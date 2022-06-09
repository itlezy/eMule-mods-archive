//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "StdAfx.h"
#include "preferences.h"
#include "UPnPImplMiniLib.h"
#include "Log.h"
#include "Otherfunctions.h"
#define STATICLIB
#include "miniupnpc\miniupnpc.h"
#include "miniupnpc\upnpcommands.h"
#include "miniupnpc\upnperrors.h"
#include "Resource.h" //zz_fly :: show UPnP status

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

Poco::FastMutex CUPnPImplMiniLib::m_mutBusy;

CUPnPImplMiniLib::CUPnPImplMiniLib(){
	m_pURLs = NULL;
	m_pIGDData = NULL;
	m_bAbortDiscovery = false;
	m_bSucceededOnce = false;
	m_achLanIP[0] = 0;
}
	
CUPnPImplMiniLib::~CUPnPImplMiniLib(){
	if (m_pURLs != NULL)
		FreeUPNPUrls(m_pURLs);
	delete m_pURLs;
	m_pURLs = NULL;
	delete m_pIGDData;
	m_pIGDData = NULL;
}

bool CUPnPImplMiniLib::IsReady(){
	// the only check we need to do is if we are already busy with some async/threaded function
	Poco::FastMutex::SingleLock lockTest(m_mutBusy);
	 if (!m_bAbortDiscovery && lockTest.tryLock())
		 return true;
	 return false;
}

void CUPnPImplMiniLib::StopAsyncFind(){
	Poco::FastMutex::SingleLock lockTest(m_mutBusy);
	m_bAbortDiscovery = true; // if there is a thread, tell him to abort as soon as possible - he won't sent a Resultmessage when aborted
	if (!lockTest.Lock(7000)) // give the thread 7 seconds to exit gracefully - it should never really take that long
	{
		// that quite bad, something seems to be locked up. There isn't a good solution here, we need the thread to quit
		// or it might try to access a deleted object later, but termianting him is quite bad too. Well..
		DebugLogError(_T("Waiting for UPnP StartDiscoveryThread to quit failed, trying to terminate the thread..."));
		if (isRunning()){
			DebugLogError(terminate()?_T("...OK"):_T("...Failed"));
		}
		else
			ASSERT( false );
	}
	else {
		DebugLog(_T("Aborted any possible UPnP StartDiscoveryThread"));
		m_bAbortDiscovery = false;
	}
}

void CUPnPImplMiniLib::DeletePorts()
{
	m_bUPnPPortsForwarded = TRIS_FALSE;
	// this function itself blocking, because its called at the end of eMule and we need to wait for it to finish
	// before going on anyway. It might be caled from the non-blocking StartDiscovery() function too however
	Poco::FastMutex::SingleLock lockTest(m_mutBusy);
	if (lockTest.tryLock()){
		//ASSERT(m_pURLs && m_pURLs->controlURL && m_pIGDData);
		for(size_t i = 0; i < PI_COUNT; ++i)
			DeletePort(m_Mappings[i]);
	}
	else
		DebugLogError(_T("Unable to remove port mappings - implementation still busy"));
}

void CUPnPImplMiniLib::DeletePort(UPNPNAT_MAPPING&mapping)
{
	if(mapping.externalPort == 0) return;
	const char* achProtocol = mapping.bTCP ? "TCP" : "UDP";
	char achPort[10];
	_ultoa(mapping.externalPort, achPort, 10);

	if (UPNP_DeletePortMapping(m_pURLs->controlURL, m_pIGDData->first.servicetype, achPort, achProtocol, NULL) == UPNPCOMMAND_SUCCESS)
		DebugLog(_T("Sucessfully removed mapping for port %u (%hs)"), mapping.externalPort, achProtocol);
	else
		DebugLogWarning(_T("Failed to remove mapping for port %u (%hs)"), mapping.externalPort, achProtocol);
	mapping.externalPort = 0;
}

void CUPnPImplMiniLib::StartDiscovery(uint16 nTCPPort, uint16 nUDPPort, uint16 nUDPServerPort, uint16 nTCPWebPort){
	DebugLog(_T("Using MiniUPnPLib based implementation"));
	DebugLog(_T("miniupnpc (c) 2005-2011 Thomas Bernard - http://miniupnp.free.fr/"));
	m_Mappings[PI_UDP].bChanged = (nUDPPort != 0);
	m_Mappings[PI_UDP].internalPort = nUDPPort;
	m_Mappings[PI_ServerUDP].bChanged = (nUDPServerPort != 0);
	m_Mappings[PI_ServerUDP].internalPort = nUDPServerPort;
	m_Mappings[PI_TCP].bChanged = (nTCPPort != 0);
	m_Mappings[PI_TCP].internalPort = nTCPPort;
	m_Mappings[PI_WebTCP].bChanged = (nTCPWebPort != 0);
	m_Mappings[PI_WebTCP].internalPort = nTCPWebPort;
	m_bUPnPPortsForwarded = TRIS_UNKNOWN;
	m_bCheckAndRefresh = false;

	if (m_pURLs != NULL)
		FreeUPNPUrls(m_pURLs);
	delete m_pURLs;
	m_pURLs = NULL;
	delete m_pIGDData;
	m_pIGDData = NULL;

	if (m_bAbortDiscovery)
		return;

	Thread::start();
}

bool CUPnPImplMiniLib::CheckAndRefresh()
{
	// on a CheckAndRefresh we don't do any new time consuming discovery tries, we expect to find the same router like the first time
	// and of course we also don't delete old ports (this was done on Discovery) but only check that our current mappings still exist
	// and refresh them if not
	if (m_bAbortDiscovery || !m_bSucceededOnce|| !IsReady() || m_pURLs == NULL || m_pIGDData == NULL 
		|| m_pURLs->controlURL == NULL || m_Mappings[PI_TCP].internalPort == 0)
	{
		DebugLog(_T("Not refreshing UPnP ports because they don't seem to be forwarded in the first place"));
		return false;
	}
	else
		DebugLog(_T("Checking and refreshing UPnP ports"));

	m_bCheckAndRefresh = true;
	Thread::start();
	return true;
}

void CUPnPImplMiniLib::run()
{
	DbgSetThreadName("CUPnPImplMiniLib::run");
	InitThreadLocale();
	Poco::FastMutex::SingleLock sLock(m_mutBusy);
	if (!sLock.tryLock()){
		DebugLogWarning(_T("CUPnPImplMiniLib::run, failed to acquire Lock, another Mapping try might be running already"));
		return;
	}

	if (m_bAbortDiscovery) // requesting to abort ASAP?
		return;

	bool bSucceeded = false;
#if !(defined(_DEBUG) || defined(_BETA))
	try
#endif
	{
		if (!m_bCheckAndRefresh)
		{
			UPNPDev* structDeviceList = upnpDiscover(2000, NULL, NULL, 0, NULL);
			if (structDeviceList == NULL){
				DebugLog(_T("UPNP: No Internet Gateway Devices found, aborting"));
				m_bUPnPPortsForwarded = TRIS_FALSE;
				SetStatusString(GetResString(IDS_UPNPSTATUS_ERROR)); //zz_fly :: show UPnP status
				SendResultMessage();
				return;
			}

			if (m_bAbortDiscovery){ // requesting to abort ASAP?
				freeUPNPDevlist(structDeviceList);
				return;
			}

			DebugLog(_T("List of UPNP devices found on the network:"));
			for(UPNPDev* pDevice = structDeviceList; pDevice != NULL; pDevice = pDevice->pNext)
			{
				DebugLog(_T("Desc: %hs, st: %hs"), pDevice->descURL, pDevice->st);
			}
			m_pURLs = new UPNPUrls;
			memset(m_pURLs, 0, sizeof(UPNPUrls));
			m_pIGDData = new IGDdatas;
			memset(m_pIGDData, 0, sizeof(IGDdatas));
			
			m_achLanIP[0] = 0;
			int iResult = UPNP_GetValidIGD(structDeviceList, m_pURLs, m_pIGDData, m_achLanIP, sizeof(m_achLanIP));
			freeUPNPDevlist(structDeviceList);
			bool bNotFound = false;
			switch (iResult){
				case 1:
					DebugLog(_T("Found valid IGD : %hs"), m_pURLs->controlURL);
					break;
				case 2:
					DebugLog(_T("Found a (not connected?) IGD : %hs - Trying to continue anyway"), m_pURLs->controlURL);
					break;
				case 3:
					DebugLog(_T("UPnP device found. Is it an IGD ? : %hs - Trying to continue anyway"), m_pURLs->controlURL);
					break;
				default:
					DebugLog(_T("Found device (igd ?) : %hs - Aborting"), m_pURLs->controlURL != NULL ? m_pURLs->controlURL : "(none)");
					bNotFound = true;
					break;
			}
			if (bNotFound || m_pURLs->controlURL == NULL)
			{
				SetStatusString(GetResString(IDS_UPNPSTATUS_ERROR)); //zz_fly :: show UPnP status
				m_bUPnPPortsForwarded = TRIS_FALSE;
				SendResultMessage();
				return;
			}
			DebugLog(_T("Our LAN IP: %hs"), m_achLanIP);

			if (m_bAbortDiscovery) // requesting to abort ASAP?
				return;
		}
		ThreadLocalPtr<SFMT> p_rng(&t_rng);
		bool tryRandom = thePrefs.GetUPnPNatTryRandom();
		bSucceeded = OpenPort(m_Mappings[PI_UDP], tryRandom, m_bCheckAndRefresh, m_achLanIP);
		if (bSucceeded)
			bSucceeded = OpenPort(m_Mappings[PI_ServerUDP], tryRandom, m_bCheckAndRefresh, m_achLanIP);
		if (bSucceeded)
			bSucceeded = OpenPort(m_Mappings[PI_TCP], tryRandom, m_bCheckAndRefresh, m_achLanIP);
		if (bSucceeded)
			OpenPort(m_Mappings[PI_WebTCP], false, m_bCheckAndRefresh, m_achLanIP); // don't fail if only the webinterface port fails for some reason
	}
#if !(defined(_DEBUG) || defined(_BETA))
	catch(...)
	{
		DebugLogError(_T("Unknown Exception in CUPnPImplMiniLib::CStartDiscoveryThread::Run()"));
	}
#endif
	if (!m_bAbortDiscovery){ // dont send a result on a abort request
		m_bUPnPPortsForwarded = bSucceeded ? TRIS_TRUE : TRIS_FALSE;
		m_bSucceededOnce |= bSucceeded;
		SendResultMessage();
	}
}

bool CUPnPImplMiniLib::OpenPort(UPNPNAT_MAPPING&mapping, bool tryRandom, bool bCheckAndRefresh, char* pachLANIP)
{
	if(mapping.internalPort == 0)
		return true;

	if (m_bAbortDiscovery)
		return false;

	const char*achProtocol = mapping.bTCP ? "TCP" : "UDP";
	char achPort[10];
	char achExtPort[10];
	_ultoa(mapping.internalPort, achPort, 10);

	// if we are refreshing ports, check first if the mapping is still fine and only try to open if not
	char achOutIP[20];
	char achOutPort[10];
	achOutPort[0] = 0;
	if(mapping.bChanged)
	{
		DeletePort(mapping);
		mapping.bChanged = false;
		mapping.externalPort = mapping.internalPort;
	}
	else if (bCheckAndRefresh)
	{
		_ultoa(mapping.externalPort, achExtPort, 10);
		achOutIP[0] = 0;
		if (UPNP_GetSpecificPortMappingEntry(m_pURLs->controlURL, m_pIGDData->first.servicetype
			, achExtPort, achProtocol, achOutIP, achOutPort, NULL, NULL, NULL) == UPNPCOMMAND_SUCCESS && achOutIP[0] != 0){
			DebugLog(_T("Checking UPnP: Mapping for port %u (%hs) on local IP %hs still exists"), mapping.internalPort, achProtocol, achOutIP);
			return true;
		}
		else 
			DebugLogWarning(_T("Checking UPnP: Mapping for port %u (%hs) on local IP %hs is gone, trying to reopen port"), mapping.internalPort, achProtocol, achOutIP);
	}
	else if(mapping.externalPort == 0)
		mapping.externalPort = mapping.internalPort;

	for (int retries = 200; retries>0; retries--)
	{
		if (m_bAbortDiscovery)
		{
			mapping.externalPort = 0;
			return false;
		}

		_ultoa(mapping.externalPort, achExtPort, 10);
		int nResult = UPNP_AddPortMapping(m_pURLs->controlURL, m_pIGDData->first.servicetype
			, achPort, achExtPort, pachLANIP, mapping.description, achProtocol, NULL, NULL);
		if (nResult == UPNPCOMMAND_SUCCESS)
			break;
		DebugLog(_T("Adding PortMapping failed, Error Code %u"), nResult);

		if (!tryRandom)
		{
			mapping.externalPort = 0;
			return false;
		}

		mapping.externalPort = (WORD)(2049 + t_rng->getUInt32() % (65535 - 2049));
	}

	// make sure it really worked
	achOutIP[0] = 0;
	if (UPNP_GetSpecificPortMappingEntry(m_pURLs->controlURL, m_pIGDData->first.servicetype
		, achExtPort, achProtocol, achOutIP, achOutPort, NULL, NULL, NULL) == UPNPCOMMAND_SUCCESS && achOutIP[0] != 0){
		DebugLog(_T("Sucessfully added mapping for port %u (%hs) on local IP %hs"), mapping.internalPort, achProtocol, achOutIP);
		return true;
	}
	DebugLogWarning(_T("Failed to verfiy mapping for port %u (%hs) on local IP %hs - considering as failed"), mapping.internalPort, achProtocol, achOutIP);
	// maybe counting this as error is a bit harsh as this may lead to false negatives, however if we would risk false postives
	// this would mean that the fallback implementations are not tried because eMule thinks it worked out fine
	mapping.externalPort = 0;
	return false;
}
