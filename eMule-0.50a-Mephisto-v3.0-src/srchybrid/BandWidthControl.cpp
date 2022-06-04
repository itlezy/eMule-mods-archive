//this file is part of eMule Xtreme-Mod (http://www.xtreme-mod.net)
//Copyright (C)2002-2007 Xtreme-Mod (emulextreme@yahoo.de)

//emule Xtreme is a modification of eMule
//Copyright (C)2002-2007 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

//
//
//	Author: Xman / Maella
//  


#include "StdAfx.h"
#include "BandWidthControl.h"
#include "Emule.h"
#include "Log.h"
#include "Preferences.h"
#include "opcodes.h"
#include "otherfunctions.h"
// ==> Multiple friendslots [ZZ] - Mephisto
#include "Statistics.h"
#include "FriendList.h"
// <== Multiple friendslots [ZZ] - Mephisto

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CBandWidthControl::CBandWidthControl()
:   m_statisticHistory(1024) // size  ~= 1024*(4*8+3*4) = 1024*44 bytes = 44 KBytes
{
   m_statistic.eMuleOutOctets = 0;
   m_statistic.eMuleOutOctetsFriends = 0; // Multiple friendslots [ZZ] - Mephisto
   m_statistic.eMuleInOctets = 0;
   m_statistic.eMuleOutOverallOctets = 0;
   m_statistic.eMuleInOverallOctets = 0;
   m_statistic.networkOutOctets = 0;
   m_statistic.networkInOctets = 0;
   m_statistic.timeStamp = ::GetTickCount();
	m_maxDownloadLimit = 0.0f;
	m_maxUploadLimit = 0.0f;
	m_errorTraced = false;
	m_obfuscation_InOctets=0;
	m_obfuscation_OutOctets=0;

	//Xman GlobalMaxHarlimit for fairness
	m_maxforcedDownloadlimit=0;

	// Keep last result to detect an overflow
	m_networkOutOctets = 0;
	m_networkInOctets = 0;

   // Cache index value
   m_currentAdapterIndex = 0;
   m_lastAdapterIndex = 0;

   // Dynamic load library iphlpapi.dll => user of win95
   m_hIphlpapi = ::LoadLibrary(_T("iphlpapi.dll"));
   if(m_hIphlpapi != NULL){
      m_fGetIfTable = (GETIFTABLE)GetProcAddress(m_hIphlpapi, "GetIfTable");
		m_fGetIpAddrTable = (GETIPADDRTABLE)GetProcAddress(m_hIphlpapi, "GetIpAddrTable");
      m_fGetIfEntry = (GETIFENTRY)GetProcAddress(m_hIphlpapi, "GetIfEntry");
      m_fGetNumberOfInterfaces = (GETNUMBEROFINTERFACES)GetProcAddress(m_hIphlpapi, "GetNumberOfInterfaces");

		theApp.QueueDebugLogLine(false, _T("NAFC: Succeed to load library iphlpapi.dll"));
   }
   else {
      m_fGetIfTable = NULL;
      m_fGetIfEntry = NULL;
      m_fGetNumberOfInterfaces = NULL;

		theApp.QueueDebugLogLine(false, _T("NAFC: Fail to load library iphlpapi.dll"));
   }
   //Xman new adapter selection
   wasNAFCLastActive=thePrefs.GetNAFCFullControl();
   boundIP=0;

	// ==> Enforce Ratio [Stulle] - Stulle
	m_maxforcedDownloadlimitEnforced=0.0f;
	m_fMaxDownloadEqualUploadLimit=0.0f;
	m_uNAFCRatio=0;
	// <== Enforce Ratio [Stulle] - Stulle

	m_fMaxDownloadFriends=0.0f; // Multiple friendslots [ZZ] - Mephisto
}

CBandWidthControl::~CBandWidthControl(){
   // Unload library
   if(m_hIphlpapi != NULL){
      ::FreeLibrary(m_hIphlpapi);
   }
}
//Xman new adapter selection
void CBandWidthControl::checkAdapterIndex(uint32 highid)
{
	// Check if the library was successfully loaded
	if(m_fGetNumberOfInterfaces != NULL && m_fGetIfTable != NULL && m_fGetIpAddrTable != NULL){
		DWORD dwNumIf = 0;
		if(m_fGetNumberOfInterfaces(&dwNumIf) == NO_ERROR && dwNumIf > 0 ){
			PMIB_IFTABLE mibIfTable=NULL;
			DWORD dwSize = 0;
			DWORD dwRetVal = 0;
			mibIfTable = (MIB_IFTABLE*) malloc(sizeof(MIB_IFTABLE));
			if (m_fGetIfTable(mibIfTable, &dwSize, true) == ERROR_INSUFFICIENT_BUFFER) {
				free(mibIfTable);
				mibIfTable = (MIB_IFTABLE *) malloc (dwSize);
			}

			if ((dwRetVal = m_fGetIfTable(mibIfTable, &dwSize, true)) == NO_ERROR) {

				// Trace list of Adapters
				if(m_errorTraced == false){
					for(DWORD dwNumEntries = 0; dwNumEntries < mibIfTable->dwNumEntries; dwNumEntries++){
						const MIB_IFROW& mibIfRow = mibIfTable->table[dwNumEntries];
						theApp.QueueDebugLogLine(false, _T("NAFC: Adapter %u is '%s'"), mibIfRow.dwIndex, (CString)mibIfRow.bDescr);
					}
				}

				//Xman forceNAFCadapter-option
				if(thePrefs.GetForcedNAFCAdapter()!=0)
				{
					free(mibIfTable);
					mibIfTable=NULL;
					theApp.QueueDebugLogLine(false, _T("NAFC: you forced to use NAFC-Adapter with index: %u"), thePrefs.GetForcedNAFCAdapter());
					return;
				}
				//Xman end

				// Retrieve the default used IP (=> in case of multiple adapters)
				char hostName[256];
				if(gethostname(hostName, sizeof(hostName)) == 0){
					hostent* lphost = gethostbyname(hostName);
					if(lphost != NULL){
						DWORD dwAddr = ((LPIN_ADDR)lphost->h_addr)->s_addr;
						// Pick the interface matching the IP
						PMIB_IPADDRTABLE mibIPAddrTable=NULL;
						DWORD dwSize = 0;

						mibIPAddrTable = (MIB_IPADDRTABLE*) malloc( sizeof( MIB_IPADDRTABLE) );
						if (m_fGetIpAddrTable(mibIPAddrTable, &dwSize, FALSE) == ERROR_INSUFFICIENT_BUFFER) {
							free( mibIPAddrTable );
							mibIPAddrTable = (MIB_IPADDRTABLE *) malloc ( dwSize );
						}

						theApp.QueueDebugLogLine(false,_T("NAFC: your hostIP is %s"), ipstr(dwAddr));
						theApp.QueueDebugLogLine(false,_T("NAFC: emule is bound to %s"), ipstr(boundIP));
						theApp.QueueDebugLogLine(false,_T("NAFC: your public IP is %s"), ipstr(highid));

						if(m_fGetIpAddrTable(mibIPAddrTable, &dwSize, FALSE) == NO_ERROR){
							//Xman: first we seek the highid from the server
							theApp.QueueDebugLogLine(false,_T("NAFC: searching an adapter matching your public ip"));
							for(DWORD i = 0; i < mibIPAddrTable->dwNumEntries; i++){
								if(mibIPAddrTable->table[i].dwAddr == highid){
									//const MIB_IPADDRROW& row = mibIPAddrTable.table[i];
									m_errorTraced = false;
									theApp.QueueDebugLogLine(false, _T("NAFC found by IP: Select adapter with index %u"), mibIPAddrTable->table[i].dwIndex);
									m_currentAdapterIndex= mibIPAddrTable->table[i].dwIndex;
									//reactivate NAFC	
									if(wasNAFCLastActive)
										thePrefs.SetNAFCFullControl(true);
									free(mibIfTable);
									mibIfTable=NULL;
									free( mibIPAddrTable );
									mibIPAddrTable=NULL;
									return;
								}
							}
							//if not found search for bound ip
							theApp.QueueDebugLogLine(false,_T("NAFC: searching an adapter matching your bound ip"));
							for(DWORD i = 0; i < mibIPAddrTable->dwNumEntries; i++){
								if(mibIPAddrTable->table[i].dwAddr == boundIP){
									//const MIB_IPADDRROW& row = mibIPAddrTable.table[i];
									m_errorTraced = false;
									theApp.QueueDebugLogLine(false, _T("NAFC found by IP: Select adapter with index %u"), mibIPAddrTable->table[i].dwIndex);
									m_currentAdapterIndex= mibIPAddrTable->table[i].dwIndex;
									//reactivate NAFC	
									if(wasNAFCLastActive)
										thePrefs.SetNAFCFullControl(true);
									free(mibIfTable);
									mibIfTable=NULL;
									free( mibIPAddrTable );
									mibIPAddrTable=NULL;
									return;
								}
							}

							//if not found it's strange.. use the default = host ip
							theApp.QueueDebugLogLine(false,_T("NAFC: searching an adapter matching your host ip"));
							for(DWORD i = 0; i < mibIPAddrTable->dwNumEntries; i++){
								if(mibIPAddrTable->table[i].dwAddr == dwAddr){
									//const MIB_IPADDRROW& row = mibIPAddrTable.table[i];
									m_errorTraced = false;
									theApp.QueueDebugLogLine(false, _T("NAFC: Select adapter with index %u"), mibIPAddrTable->table[i].dwIndex);
									m_currentAdapterIndex= mibIPAddrTable->table[i].dwIndex;
									//reactivate NAFC	
									if(wasNAFCLastActive)
										thePrefs.SetNAFCFullControl(true);
									free(mibIfTable);
									mibIfTable=NULL;
									free( mibIPAddrTable );
									mibIPAddrTable=NULL;
									return;
								}
							}
						}
						else {
							if(m_errorTraced == false){
								m_errorTraced = true;
								theApp.QueueDebugLogLine(false, _T("NAFC: Failed to get IP tables error=0x%x"), ::GetLastError());
								free(mibIfTable);
								mibIfTable=NULL;
								free( mibIPAddrTable );
								mibIPAddrTable=NULL;
								return ;
							}
						}
						free( mibIPAddrTable );
						mibIPAddrTable=NULL;
					}
				}
			}
			free(mibIfTable);
			mibIfTable=NULL;
			if(m_errorTraced == false){
				m_errorTraced = true;
				theApp.QueueDebugLogLine(false, _T("NAFC: Failed to get tables of interface error=0x%x"), ::GetLastError());
				return ;
			}
		}
		if(m_errorTraced == false){
			m_errorTraced = true;
			theApp.QueueDebugLogLine(false, _T("NAFC: Failed to get the number of interface error=0x%x"), ::GetLastError());
			return ;
		}
	}
	return ;
}
//Xman new adapter selection end

DWORD CBandWidthControl::getAdapterIndex(){
	// Check if the library was successfully loaded
	if(m_fGetNumberOfInterfaces != NULL && m_fGetIfTable != NULL && m_fGetIpAddrTable != NULL){
		DWORD dwNumIf = 0;
		if(m_fGetNumberOfInterfaces(&dwNumIf) == NO_ERROR && dwNumIf > 0 ){
			PMIB_IFTABLE mibIfTable=NULL;
			DWORD dwSize = 0;
			DWORD dwRetVal = 0;
			mibIfTable = (MIB_IFTABLE*) malloc(sizeof(MIB_IFTABLE));
			if (m_fGetIfTable(mibIfTable, &dwSize, true) == ERROR_INSUFFICIENT_BUFFER) {
				free(mibIfTable);
				mibIfTable = (MIB_IFTABLE *) malloc (dwSize);
			}

			if ((dwRetVal = m_fGetIfTable(mibIfTable, &dwSize, true)) == NO_ERROR) {

				// Trace list of Adapters
				if(m_errorTraced == false){
					for(DWORD dwNumEntries = 0; dwNumEntries < mibIfTable->dwNumEntries; dwNumEntries++){
						const MIB_IFROW& mibIfRow = mibIfTable->table[dwNumEntries];
						theApp.QueueDebugLogLine(false, _T("NAFC: Adapter %u is '%s'"), mibIfRow.dwIndex, (CString)mibIfRow.bDescr);
					}
				}

				//Xman forceNAFCadapter-option
				if(thePrefs.GetForcedNAFCAdapter()!=0)
				{
					free(mibIfTable);
					mibIfTable=NULL;
					theApp.QueueDebugLogLine(false, _T("NAFC: you forced to use NAFC-Adapter with index: %u"), thePrefs.GetForcedNAFCAdapter());
					return thePrefs.GetForcedNAFCAdapter();
				}
				//Xman end

				// Retrieve the default used IP (=> in case of multiple adapters)
				char hostName[256];
				if(gethostname(hostName, sizeof(hostName)) == 0){
					hostent* lphost = gethostbyname(hostName);
					if(lphost != NULL){
						DWORD dwAddr = ((LPIN_ADDR)lphost->h_addr)->s_addr;
						// Pick the interface matching the IP
						PMIB_IPADDRTABLE mibIPAddrTable=NULL;
						DWORD dwSize = 0;

						mibIPAddrTable = (MIB_IPADDRTABLE*) malloc( sizeof( MIB_IPADDRTABLE) );
						if (m_fGetIpAddrTable(mibIPAddrTable, &dwSize, FALSE) == ERROR_INSUFFICIENT_BUFFER) {
							free( mibIPAddrTable );
							mibIPAddrTable = (MIB_IPADDRTABLE *) malloc ( dwSize );
						}

						if(m_fGetIpAddrTable(mibIPAddrTable, &dwSize, FALSE) == NO_ERROR){
							for(DWORD i = 0; i < mibIPAddrTable->dwNumEntries; i++){
								if(mibIPAddrTable->table[i].dwAddr == dwAddr){
									//const MIB_IPADDRROW& row = mibIPAddrTable.table[i];
									m_errorTraced = false;
									theApp.QueueDebugLogLine(false, _T("NAFC: Select adapter with index %u"), mibIPAddrTable->table[i].dwIndex);
									DWORD dwIndex=mibIPAddrTable->table[i].dwIndex;
									free(mibIfTable);
									mibIfTable=NULL;
									free( mibIPAddrTable );
									mibIPAddrTable=NULL;
									return dwIndex;
								}
							}
						}
						else {
							if(m_errorTraced == false){
								m_errorTraced = true;
								theApp.QueueDebugLogLine(false, _T("NAFC: Failed to get IP tables error=0x%x"), ::GetLastError());
								free(mibIfTable);
								mibIfTable=NULL;
								free( mibIPAddrTable );
								mibIPAddrTable=NULL;
								return 0;
							}
						}
						free( mibIPAddrTable );
						mibIPAddrTable=NULL;
					}
				}
			}
			free(mibIfTable);
			mibIfTable=NULL;
			if(m_errorTraced == false){
				m_errorTraced = true;
				theApp.QueueDebugLogLine(false, _T("NAFC: Failed to get tables of interface error=0x%x"), ::GetLastError());
				return 0;
			}
		}
		if(m_errorTraced == false){
			m_errorTraced = true;
			theApp.QueueDebugLogLine(false, _T("NAFC: Failed to get the number of interface error=0x%x"), ::GetLastError());
			return 0;
		}
	}
	return 0;
}

//                 Only the code related to the instance bandwidth is 
//                 multithread-safe (see tag /**/)
//
void CBandWidthControl::Process()
{
	bool disableNAFC = false; //zz_fly :: Avoid Deadlock
	static DWORD processtime;
	if(::GetTickCount()-processtime >= 1000){
		processtime = ::GetTickCount();

		// Try to get the Adapter Index to access to the right interface
		if(m_currentAdapterIndex == 0){
			m_currentAdapterIndex = getAdapterIndex();

			// Disable NAFC
			if(m_currentAdapterIndex == 0){
				thePrefs.SetNAFCFullControl(false);
			}
		}

		/*->*/m_statisticLocker.Lock();
		/**/ // Update the datarate directly from the network Adapter
		/**/ if(m_currentAdapterIndex != 0 && m_fGetIfEntry != NULL){
		/**/ 	static int s_Log; // Static initiate with zero
		/**/ 	MIB_IFROW ifRow;       
		/**/ 	ifRow.dwIndex = m_currentAdapterIndex;
		/**/ 	if(m_fGetIfEntry(&ifRow) == NO_ERROR){
		/**/ 		s_Log = 0;
		/**/ 
		/**/		//Xman prevent overflow on adapterchange:
		/**/		if(m_currentAdapterIndex==m_lastAdapterIndex && ifRow.dwOutOctets >= m_networkOutOctets && ifRow.dwInOctets >= m_networkInOctets)
		/**/		{
		/**/			// Add the delta, since the last measure (convert 32 to 64 bits)
		/**/			m_statistic.networkInOctets += (DWORD)(ifRow.dwInOctets - m_networkInOctets);
		/**/			m_statistic.networkOutOctets += (DWORD)(ifRow.dwOutOctets - m_networkOutOctets);
		/**/		}
		/**/
		/**/		// Keep last measure
		/**/		m_networkOutOctets = ifRow.dwOutOctets; 
		/**/		m_networkInOctets = ifRow.dwInOctets;
		/**/		m_lastAdapterIndex = m_currentAdapterIndex;
		/**/ 	}
		/**/ 	else {
		/**/ 		if(s_Log == 0){
		/**/ 			s_Log = 1;
		/**/ 			theApp.QueueDebugLogLine(false, _T("NAFC: Failed to retrieve adapter traffic, error=0x%x"), ::GetLastError());
		/**/			wasNAFCLastActive=thePrefs.GetNAFCFullControl(); //Xman  
		/**/ 			// Disable NAFC
		/**/			//zz_fly :: Avoid Deadlock
		/**/ 			//note: SetNAFCFullControl() may call RepaintMeters(). it is better to do it out side the lock.
		/**/			//thePrefs.SetNAFCFullControl(false);
		/**/			disableNAFC = true; 
		/**/ 		}
		/**/ 
		/**/ 	}
		/**/ }
		/**/ 
		/**/ // Update large history list for the history graph
		/**/ m_statisticHistory.AddHead(Statistic(m_statistic, ::GetTickCount()));
		/**/ 
		/**/
		/**/
		/**/// Trunk size of the list (The timestamp is more accurate than the period of Process())
		/**/const uint32 averageMinTime = (uint32)(60000 * thePrefs.GetStatsAverageMinutes());
		#pragma warning(disable:4127)
		/**/while(true){
		/**/	const uint32 deltaTime = m_statisticHistory.GetHead().timeStamp - m_statisticHistory.GetTail().timeStamp;
		/**/	if(deltaTime <= averageMinTime){
		/**/		break; // exit loop
		/**/	}
		/**/	m_statisticHistory.RemoveTail(); // Trunk size
		/**/}
		#pragma warning(default:4127)
		/*->*/ m_statisticLocker.Unlock();
		
		if(disableNAFC) thePrefs.SetNAFCFullControl(false); //zz_fly :: Avoid Deadlock

		// Calculate the dynamic download limit
		m_maxDownloadLimit = thePrefs.GetMaxDownload();
		m_maxUploadLimit = thePrefs.GetMaxUpload();
		
		//Xman GlobalMaxHarlimit for fairness
		//remark: we need a new downloadlimit, which is calculated by the real emule-upload
		//we have to calculate it on each loop.
		//also we need the additionally limit for NAFC, if the forced downloadlimit isn't be used
		m_maxforcedDownloadlimit=m_maxDownloadLimit; //initialize with the standard
		// ==> Enforce Ratio [Stulle] - Stulle
		/*
		if(thePrefs.m_bAcceptsourcelimit==false) //only if user doesn't accept it
		*/
		m_maxforcedDownloadlimitEnforced=m_maxDownloadLimit; //initialize with the standard
		m_fMaxDownloadEqualUploadLimit=m_maxDownloadLimit; //initialize with the standard
		m_uNAFCRatio=0; // initialize with zero
		// ==> Multiple friendslots [ZZ] - Mephisto
		/*
		if(thePrefs.m_bAcceptsourcelimit==false || thePrefs.GetEnforceRatio())
		*/
		m_fMaxDownloadFriends=m_maxDownloadLimit; //initialize with the standard
		if(thePrefs.m_bAcceptsourcelimit==false ||
			thePrefs.GetEnforceRatio() ||
			(theApp.friendlist && theApp.friendlist->IsFriendSlot()))
		// <== Multiple friendslots [ZZ] - Mephisto
		// <== Enforce Ratio [Stulle] - Stulle
		{
			if(m_statisticHistory.GetSize() > 10){
				const Statistic& newestSample = m_statisticHistory.GetHead();
				const Statistic& oldestSample = m_statisticHistory.GetAt(m_statisticHistory.FindIndex(10)); //avg over 10 seconds, should give a smoother downloadgraph
				const uint32 deltaTime = (newestSample.timeStamp - oldestSample.timeStamp); // in [ms]
				if (deltaTime == 0) 
				{
					//Xman to be sure
					return;
				}
				const uint32 eMuleOut = (1000 * (uint32)(newestSample.eMuleOutOctets - oldestSample.eMuleOutOctets) / deltaTime); // in [Bytes/s]
				//remark: this is always a 1:4 Ratio-Limit
				float maxDownloadLimit = (float)(4*eMuleOut) / 1024.0f; // [KB/s]
				if(maxDownloadLimit < m_maxforcedDownloadlimit){
					m_maxforcedDownloadlimit = maxDownloadLimit;
				}
				if(m_maxforcedDownloadlimit<1.0f)
					m_maxforcedDownloadlimit=1.0f;
				// ==> Enforce Ratio [Stulle] - Stulle
				float maxDownloadLimitEnforced = (float)(thePrefs.GetRatioValue()*eMuleOut) / 1024.0f; // [KB/s]
				if(maxDownloadLimitEnforced < m_maxforcedDownloadlimitEnforced){
					m_maxforcedDownloadlimitEnforced = maxDownloadLimitEnforced;
				}
				if(m_maxforcedDownloadlimitEnforced<1.0f)
					m_maxforcedDownloadlimitEnforced=1.0f;

				float fMaxDownloadEqualUploadLimit = (float)eMuleOut / 1024.0f; // [KB/s]
				if(fMaxDownloadEqualUploadLimit < m_fMaxDownloadEqualUploadLimit){
					m_fMaxDownloadEqualUploadLimit = fMaxDownloadEqualUploadLimit;
				}
				if(m_fMaxDownloadEqualUploadLimit<1.0f)
					m_fMaxDownloadEqualUploadLimit=1.0f;
				// <== Enforce Ratio [Stulle] - Stulle

				// ==> Multiple friendslots [ZZ] - Mephisto
				const uint32 eMuleOutFriends = (1000 * (uint32)(newestSample.eMuleOutOctetsFriends - oldestSample.eMuleOutOctetsFriends) / deltaTime); // in [Bytes/s]
				float fMaxDownloadFriends = (float)(3*(eMuleOut-eMuleOutFriends)) / 1024.0f; // [KB/s]
				if(fMaxDownloadFriends < m_fMaxDownloadFriends){
					m_fMaxDownloadFriends = fMaxDownloadFriends;
				}
				if(m_fMaxDownloadFriends<1.0f)
					m_fMaxDownloadFriends=1.0f;
				// <== Multiple friendslots [ZZ] - Mephisto
			}
		}
		//Xman end GlobalMaxHarlimit for fairness

		if(thePrefs.GetNAFCFullControl() == true ){    
			// Remark: Because there is only ONE writer thread to m_statisticHistory, there is no need to 
			//         protect its access inside this method. 
			//         => m_statisticHistory is only modified inside this method
			//         => Its access in GetDatarates() + GetFullHistoryDatarates() still must be protected

			if(m_statisticHistory.GetSize() > 10){
				const Statistic& newestSample = m_statisticHistory.GetHead();
				const Statistic& oldestSample = m_statisticHistory.GetAt(m_statisticHistory.FindIndex(10)); //avg over 10 seconds, should give a smoother downloadgraph
				const uint32 deltaTime = (newestSample.timeStamp - oldestSample.timeStamp); // in [ms]
				if (deltaTime == 0) 
				{
					//Xman shouldn't happen, but happend
					return;
				}
				const uint32 eMuleOutOverall = (1000 * (uint32)(newestSample.eMuleOutOverallOctets - oldestSample.eMuleOutOverallOctets) / deltaTime); // in [Bytes/s]
				//const uint32 networkOut = (1000 * (uint32)(newestSample.networkOutOctets - oldestSample.networkOutOctets) / deltaTime); // in [Bytes/s]
				// Dynamic limit with ratio
				if(eMuleOutOverall < 4*1024){
					// Ratio 3x
					float maxDownloadLimit = (float)(3*eMuleOutOverall) / 1024.0f; // [KB/s]
					if(maxDownloadLimit < m_maxDownloadLimit){
						m_maxDownloadLimit = maxDownloadLimit;
					}
					m_uNAFCRatio=3; // Enforce Ratio [Stulle] - Stulle
				}
				else if(eMuleOutOverall < 11*1024){ //Xman changed to 11
					// Ratio 4x
					float maxDownloadLimit = (float)(4*eMuleOutOverall) / 1024.0f; // [KB/s]
					if(maxDownloadLimit < m_maxDownloadLimit){
						m_maxDownloadLimit = maxDownloadLimit;
					}
					m_uNAFCRatio=4; // Enforce Ratio [Stulle] - Stulle
				}

				if(m_maxDownloadLimit < 1.0f){
					m_maxDownloadLimit = 1.0f;
				}
			}
		}
	}
	else if(thePrefs.GetNAFCFullControl() == true ){
		// Retrieve the network flow => necessary for the NAFC with 'Auto U/D limit'
		if(m_currentAdapterIndex != 0 && m_fGetIfEntry != NULL){
			MIB_IFROW ifRow;       
			ifRow.dwIndex = m_currentAdapterIndex;
			if(m_fGetIfEntry(&ifRow) == NO_ERROR){
				
				// Add the delta, since the last measure (convert 32 to 64 bits)
				m_statisticLocker.Lock();  
				/**/	//Xman prevent overflow on adapterchange:
				/**/	if(m_currentAdapterIndex==m_lastAdapterIndex && ifRow.dwOutOctets >= m_networkOutOctets && ifRow.dwInOctets >= m_networkInOctets)
				/**/	{
				/**/		m_statistic.networkInOctets += (DWORD)(ifRow.dwInOctets - m_networkInOctets);
				/**/		m_statistic.networkOutOctets += (DWORD)(ifRow.dwOutOctets - m_networkOutOctets);
				/**/	}
				m_statisticLocker.Unlock();

				// Keep last measure
				m_networkOutOctets = ifRow.dwOutOctets;
				m_networkInOctets = ifRow.dwInOctets;
				m_lastAdapterIndex = m_currentAdapterIndex;
			}
		}
	}
}

void CBandWidthControl::GetDatarates(UINT samples, 
									 uint32& eMuleIn, uint32& eMuleInOverall, 
									 uint32& eMuleOut, uint32& eMuleOutOverall,
									 uint32& networkIn, uint32& networkOut) const 
{
	eMuleIn = 0;
	eMuleInOverall = 0;
	eMuleOut = 0;
	eMuleOutOverall = 0;
	networkIn = 0;
	networkOut = 0;

	// Check if the list is already long enough
	/*->*/ m_statisticLocker.Lock();
	/**/ if(m_statisticHistory.GetSize() >= 2 && samples >= 1){
	/**/ 
	/**/	// Retrieve the location of the n previous sample
	/**/	POSITION pos = m_statisticHistory.FindIndex(samples);
	/**/    if(pos == NULL){
	/**/		pos = m_statisticHistory.GetTailPosition();
	/**/    }           
	/**/
	/**/    const Statistic& newestSample = m_statisticHistory.GetHead();
	/**/    const Statistic& oldestSample = m_statisticHistory.GetAt(pos);
	/**/    const uint32 deltaTime = (newestSample.timeStamp - oldestSample.timeStamp); // in [ms]
	/**/     
	/**/    if(deltaTime > 0){
	/**/		eMuleIn = (uint32)(1000 * (newestSample.eMuleInOctets - oldestSample.eMuleInOctets) / deltaTime); // in [Bytes/s]
	/**/        eMuleInOverall = (uint32)(1000 * (newestSample.eMuleInOverallOctets - oldestSample.eMuleInOverallOctets) / deltaTime); // in [Bytes/s]
	/**/        eMuleOut = (uint32)(1000 * (newestSample.eMuleOutOctets - oldestSample.eMuleOutOctets) / deltaTime); // in [Bytes/s]
	/**/        eMuleOutOverall = (uint32)(1000 * (newestSample.eMuleOutOverallOctets - oldestSample.eMuleOutOverallOctets) / deltaTime); // in [Bytes/s]
	/**/        networkIn = (uint32)(1000 * (newestSample.networkInOctets - oldestSample.networkInOctets) / deltaTime); // in [Bytes/s]
	/**/        networkOut = (uint32)(1000 * (newestSample.networkOutOctets - oldestSample.networkOutOctets) / deltaTime); // in [Bytes/s]
	/**/    }
	/**/ }
	/*->*/ m_statisticLocker.Unlock();
}

void CBandWidthControl::GetFullHistoryDatarates(uint32& eMuleInHistory, uint32& eMuleOutHistory, 
												uint32& eMuleInSession, uint32& eMuleOutSession) const 
{
	eMuleInHistory = 0;
	eMuleOutHistory = 0;
	eMuleInSession = 0;
	eMuleOutSession = 0;

	// Check if the list is already long enough
	/*->*/ m_statisticLocker.Lock();
	/**/ if(m_statisticHistory.GetSize() >= 2){
	/**/
	/**/	const Statistic& newestSample = m_statisticHistory.GetHead();
	/**/    const Statistic& oldestSample = m_statisticHistory.GetTail();
	/**/
	/**/    // Average value since the last n minutes
	/**/    uint32 deltaTime = (newestSample.timeStamp - oldestSample.timeStamp); // in [ms]
	/**/    if(deltaTime > 0){
	/**/        eMuleInHistory = (uint32)(1000 * (newestSample.eMuleInOctets - oldestSample.eMuleInOctets) / deltaTime); // in [Bytes/s]
	/**/        eMuleOutHistory = (uint32)(1000 * (newestSample.eMuleOutOctets - oldestSample.eMuleOutOctets) / deltaTime); // in [Bytes/s]
	/**/    }
	/**/ 
	/**/    // Average value since the start of the client
	/**/    deltaTime = (::GetTickCount() - GetStartTick()) / 1000; // in [s]
	/**/    if(deltaTime > 0){
	/**/        eMuleInSession = (uint32)(newestSample.eMuleInOctets / deltaTime); // in [Bytes/s]
	/**/        eMuleOutSession = (uint32)(newestSample.eMuleOutOctets / deltaTime); // in [Bytes/s]
	/**/    }
	/**/ }
	/*->*/ m_statisticLocker.Unlock();
}

//Xman 1:3 Ratio
// ==> Enforce Ratio [Stulle] - Stulle
/*
float CBandWidthControl::GetMaxDownloadEx(bool force)
{
	//Xman GlobalMaxHarlimit for fairness
	if(force && GeteMuleIn()>GeteMuleOut()*3) //session/amount based ratio
		return GetForcedDownloadlimit();
	//Xman end

   if(thePrefs.Is13Ratio() && GeteMuleIn()<=GeteMuleOut()*3) 
       return UNLIMITED;
   else
      if(thePrefs.GetNAFCFullControl()==true)
         return GetMaxDownload();
      else
         return thePrefs.GetMaxDownload();
}
*/
// 1 = Forced 1:3 by sources
// 2 = Enforce
// 4 = Unlimited under 1:3
// 8 = NAFC
// ==> Multiple friendslots [ZZ] - Mephisto
// 16 = Friendslots
// <== Multiple friendslots [ZZ] - Mephisto
float CBandWidthControl::GetMaxDownloadEx(uint8 force, uint8 &uReason, uint8 &uRatio)
{
	uint64 ueMuleOut = GeteMuleOut();
	uint64 ueMuleIn = GeteMuleIn();
	float fMaxDownload = -1.0f;

	// ==> Multiple friendslots [ZZ] - Mephisto
	/*
	if(force&1 && ueMuleIn>ueMuleOut*3) //session/amount based ratio
	{
		uReason |= 1;
		uRatio = 4;
		fMaxDownload = GetForcedDownloadlimit();
	}
	*/
	if(force&4)
	{
		// We exclude friend traffic here so it is not taken into account for the ratio calculation
		ueMuleOut -= theStats.sessionSentBytesToFriend;
		if(force&2 && thePrefs.GetRatioValue() <= 3)
			; // Note: If we enforce a ratio <= 3 anyway we skip the Friend Ratio here and do it below
		else if(ueMuleIn > (ueMuleOut*3))
		{
			uReason |= 16;
			uRatio = 1;
			fMaxDownload = GetMaxDownloadEqualUploadLimit();
		}
		else if(ueMuleIn > (ueMuleOut*2.9f))
		{
			uReason |= 16;
			uRatio = 3;
			fMaxDownload = GetMaxDownloadFriends();
		}
	}

	if(force&1 && ueMuleIn>ueMuleOut*3) //session/amount based ratio
	{
		uReason |= 1;
		if(fMaxDownload == -1.0f)
		{
			uRatio = 4;
			fMaxDownload = GetForcedDownloadlimit();
		}
	}
	// <== Multiple friendslots [ZZ] - Mephisto

	if(force&2)
	{
		if(ueMuleIn > (ueMuleOut*thePrefs.GetRatioValue()))
		{
			uReason |= 2;
			if(fMaxDownload == -1.0f || thePrefs.GetRatioValue() < 4)
			{
				uRatio = 1;
				fMaxDownload = GetMaxDownloadEqualUploadLimit();
			}
		}
		else if(ueMuleIn > (ueMuleOut*((float)(thePrefs.GetRatioValue()-0.1f))))
		{
			uReason |= 2;
			if(fMaxDownload == -1.0f || thePrefs.GetRatioValue() < 4)
			{
				uRatio = thePrefs.GetRatioValue();
				fMaxDownload = GetForcedDownloadlimitEnforced();
			}
		}
	}

	if(thePrefs.Is13Ratio() && ueMuleIn<=ueMuleOut*3 && fMaxDownload == -1.0f)
	{
		uReason |= 4;
		fMaxDownload = UNLIMITED;
	}
	else
	{
		// Note: the NAFC limit can go below the enforced limits so we check if we should rather apply NAFC limit
		if(thePrefs.GetNAFCFullControl()==true && (fMaxDownload == -1.0f || fMaxDownload > GetMaxDownload()))
		{
			uReason |= 8;
			uRatio = m_uNAFCRatio;
			fMaxDownload = GetMaxDownload();
		}
		else if(fMaxDownload == -1.0f)
			fMaxDownload = thePrefs.GetMaxDownload();
	}
	return fMaxDownload;
}
// <== Enforce Ratio [Stulle] - Stulle
//Xman end

uint64 CBandWidthControl::GeteMuleOut() const 
{
   uint64 value;
   m_statisticLocker.Lock();
   /**/ value = m_statistic.eMuleOutOctets;
   m_statisticLocker.Unlock();
   return value;
}

uint64 CBandWidthControl::GeteMuleIn() const 
{
   uint64 value;
   m_statisticLocker.Lock();
   /**/ value = m_statistic.eMuleInOctets;
   m_statisticLocker.Unlock();
   return value;
}

uint64 CBandWidthControl::GeteMuleOutOverall() const 
{
   uint64 value;
   m_statisticLocker.Lock();
   /**/ value = m_statistic.eMuleOutOverallOctets;
   m_statisticLocker.Unlock();
   return value;
}

uint64 CBandWidthControl::GeteMuleInOverall() const 
{
   uint64 value;
   m_statisticLocker.Lock();
   /**/ value = m_statistic.eMuleInOverallOctets;
   m_statisticLocker.Unlock();
   return value;
}

uint64 CBandWidthControl::GetNetworkOut() const 
{
   uint64 value;
   m_statisticLocker.Lock();
   /**/ value = m_statistic.networkOutOctets;
   m_statisticLocker.Unlock();
   return value;
}

uint64 CBandWidthControl::GetNetworkIn() const 
{
   uint64 value;
   m_statisticLocker.Lock();
   /**/ value = m_statistic.networkInOctets;
   m_statisticLocker.Unlock();
   return value;
}

//Xman show complete internettraffic
uint64 CBandWidthControl::GetSessionNetworkIn() const
{
	uint64 value;
	m_statisticLocker.Lock();
	/**/ value = m_statistic.networkInOctets;
	m_statisticLocker.Unlock();
	return value ;

}

uint64 CBandWidthControl::GetSessionNetworkOut() const
{
	uint64 value;
	m_statisticLocker.Lock();
	/**/ value = m_statistic.networkOutOctets;
	m_statisticLocker.Unlock();
	return value ;

}
//Xman end
uint32 CBandWidthControl::GetStartTick() const 
{
   uint32 value;
   m_statisticLocker.Lock();
   /**/ value = m_statistic.timeStamp;
   m_statisticLocker.Unlock();
   return value;
}

void CBandWidthControl::AddeMuleOutUDPOverall(uint32 octets)
{
   octets += (20 /* IP */ + 8 /* UDP */);
   m_statisticLocker.Lock();
   /**/ m_statistic.eMuleOutOverallOctets += octets;
   m_statisticLocker.Unlock();
}

void CBandWidthControl::AddeMuleOutTCPOverall(uint32 octets) 
{
   octets += (20 /* IP */ + 20 /* TCP */);
   m_statisticLocker.Lock();
   /**/ m_statistic.eMuleOutOverallOctets += octets;
   m_statisticLocker.Unlock();
}

void CBandWidthControl::AddeMuleOutOverallNoHeader(uint32 octets) 
{
   m_statisticLocker.Lock();
   /**/ m_statistic.eMuleOutOverallOctets += octets;
   m_statisticLocker.Unlock();
}
void CBandWidthControl::AddeMuleInUDPOverall(uint32 octets) 
{
   octets += (20 /* IP */ + 8 /* UDP */);
   m_statisticLocker.Lock();
   /**/ m_statistic.eMuleInOverallOctets += octets;
   m_statisticLocker.Unlock();
}

void CBandWidthControl::AddeMuleInTCPOverall(uint32 octets)
{
   octets += (20 /* IP */ + 20 /* TCP */);
   m_statisticLocker.Lock();
   /**/ m_statistic.eMuleInOverallOctets += octets;
   m_statisticLocker.Unlock();
}

void CBandWidthControl::AddeMuleOut(uint32 octets)
{
   m_statisticLocker.Lock();
   /**/ m_statistic.eMuleOutOctets += octets;
   m_statisticLocker.Unlock();
}

void CBandWidthControl::AddeMuleIn(uint32 octets)
{
   m_statisticLocker.Lock();
   /**/ m_statistic.eMuleInOctets += octets;
   m_statisticLocker.Unlock();
}

void CBandWidthControl::AddeMuleSYNACK()
{
	m_statisticLocker.Lock();
	/**/ m_statistic.eMuleInOverallOctets += 40; // IP + TCP
	/**/ m_statistic.eMuleOutOverallOctets += 40; // IP + TCP
	m_statisticLocker.Unlock();
}

//calculating obfuscation
//extra send -> we must add it to overhead
//threading info: called from different threads
void CBandWidthControl::AddeMuleOutObfuscationTCP(uint32 octets)
{
	
	m_statisticLocker.Lock();
	/**/ m_obfuscation_OutOctets += octets;
	/**/ octets += (20 /* IP */ + 20 /* TCP */);
	/**/ m_statistic.eMuleOutOverallOctets += octets;
	m_statisticLocker.Unlock();
}

//the header + overhead will be counted at uploadbandwidththrottler
//threading info: called from different threads
void CBandWidthControl::AddeMuleOutObfuscation(uint32 octets)
{

	m_statisticLocker.Lock();
	/**/ m_obfuscation_OutOctets += octets;
	m_statisticLocker.Unlock();
}

//overhead already calculated at UDP-send-method. Need this only for the stats
//threading info: called from uploadbandwidththrottler asynchron to main-thread
void CBandWidthControl::AddeMuleOutObfuscationUDP(uint32 octets)
{
	m_statisticLocker.Lock();
	/**/ m_obfuscation_OutOctets += octets;
	m_statisticLocker.Unlock();
}

//overhead already counted within the normal receive
//threading info: only called from the main-thread
void CBandWidthControl::AddeMuleInObfuscation(uint32 octets)
{
	m_obfuscation_InOctets += octets;
}

//threading info: Called only from the mainthread (used for statistic)
uint64 CBandWidthControl::GeteMuleOutObfuscation() const
{
	uint64 value;
	m_statisticLocker.Lock();
	/**/ value = m_obfuscation_OutOctets;
	m_statisticLocker.Unlock();
	return value;
}

//threading info: Called only from the mainthread (used for statistic)
uint64 CBandWidthControl::GeteMuleInObfuscation() const
{
	return m_obfuscation_InOctets;
}

#ifdef PRINT_STATISTIC
void CBandWidthControl::PrintStatistic()
{
	m_statisticLocker.Lock();
	AddLogLine(false, _T("Bandwidthcontrol: number of statistic-elements: %u"), m_statisticHistory.GetSize());
	m_statisticLocker.Unlock();
}
#endif

// ==> Multiple friendslots [ZZ] - Mephisto
uint64 CBandWidthControl::GeteMuleOutFriends() const 
{
   uint64 value;
   m_statisticLocker.Lock();
   /**/ value = m_statistic.eMuleOutOctetsFriends;
   m_statisticLocker.Unlock();
   return value;
}

void CBandWidthControl::AddeMuleOutFriends(uint32 octets)
{
   m_statisticLocker.Lock();
   /**/ m_statistic.eMuleOutOctetsFriends += octets;
   m_statisticLocker.Unlock();
}
// <== Multiple friendslots [ZZ] - Mephisto