//>>> JvA::SLS [enkeyDEV]
//this file is part of eMule
//Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "SourceSaver.h"
#include "PartFile.h"
#include "emule.h"
#include "updownclient.h"
#include "Preferences.h" // for thePrefs
#include "emuleDlg.h"
#include "DownloadQueue.h" // for theApp.downloadqueue
#include "OtherFunctions.h" // for ipstr()
#include "Log.h"
#include "Sockets.h" //>>> Spike2::SLS - needed to know if we have a LowID

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#define RELOADTIME					3600000 //60 minutes
#define RESAVETIME					1200000 //20 minutes ###!!!>>>
#define ACTIVATIONLIMIT          (	thePrefs.GetActivationLimitSLS()) //>>> Spike2::SLS - FiX from Anis


CSourceSaver::CSourceSaver(void)
{
	m_dwLastTimeLoaded = ::GetTickCount() - RELOADTIME;
	m_dwLastTimeSaved = ::GetTickCount() + (rand() * 30000 / RAND_MAX) - 15000 - RESAVETIME;
}

//>>> Anis - it's more effective to use the USEFULL part count
//CSourceSaver::CSourceData::CSourceData(CUpDownClient* client, const TCHAR* exp) 
CSourceSaver::CSourceData::CSourceData(CUpDownClient* client, const uint16 partsavail, const TCHAR* exp)
//### Anis - it's more effective to use the USEFULL part count
{
	// khaos::kmod+ Modified to Save Source Exchange Version
	nSrcExchangeVer = client->GetSourceExchange1Version();
	// khaos::kmod-
	if(nSrcExchangeVer > 2)
		sourceID = client->GetUserIDHybrid();
	else
		sourceID = client->GetIP();
	sourcePort = client->GetUserPort();
	serverip = client->GetServerIP();
	serverport = client->GetServerPort();
//>>> Anis - it's more effective to use the USEFULL part count
//	partsavailable = client->GetAvailablePartCount();
	partsavailable = partsavail;
//### Anis - it's more effective to use the USEFULL part count	
	memcpy(expiration, exp, 11*sizeof(TCHAR));
	expiration[10] = 0;
}

CSourceSaver::~CSourceSaver(void)
{
}

void CSourceSaver::Process(CPartFile* file) // return false if sources not saved
{
	if (!thePrefs.UseSaveLoadSources()) 
		return;

	SourceList srcs;
	TCHAR szslsfilepath[_MAX_PATH];
	_tmakepath(szslsfilepath,NULL,(CString)file->GetTempPath()+_T("\\Saved Sources"), file->GetPartMetFileName(),_T(".txtsrc"));
	if ((int)(::GetTickCount() - m_dwLastTimeLoaded) >= RELOADTIME)
	{	
		m_dwLastTimeLoaded = ::GetTickCount() + (rand() * 30000 / RAND_MAX) - 15000;
		if (file->GetAvailableSrcCount() < ACTIVATIONLIMIT) //>>> Spike2:: SLS - added ActivationLimit
		{
			LoadSourcesFromFile(file, &srcs, szslsfilepath);
			AddSourcesToDownload(file, &srcs);
		}
	}
	if ((int)(::GetTickCount() - m_dwLastTimeSaved) > RESAVETIME)
	{
		m_dwLastTimeSaved = ::GetTickCount() + (rand() * 30000 / RAND_MAX) - 15000;
		if (file->GetAvailableSrcCount() > ACTIVATIONLIMIT) //>>> Spike2:: SLS - added ActivationLimit
			if (PathFileExists(szslsfilepath)) //>>> taz::Just to be sure
				_tremove(szslsfilepath);
		else	
			SaveSources(file, &srcs, szslsfilepath);
	}
	while (!srcs.IsEmpty()) 
		delete srcs.RemoveHead();
}

void CSourceSaver::DeleteFile(CPartFile* file)
{
	TCHAR szslsfilepath[_MAX_PATH];
	// khaos::kmod+ Source Lists directory
	_tmakepath(szslsfilepath,NULL,(CString)file->GetTempPath()+_T("\\Saved Sources"), file->GetPartMetFileName(),_T(".txtsrc"));
	if (_tremove(szslsfilepath) && errno != ENOENT)
		theApp.QueueLogLineEx(LOG_ERROR, _T("Failed to delete %s, you will need to do this manually."), szslsfilepath);
}

void CSourceSaver::LoadSourcesFromFile(CPartFile* , SourceList* sources, LPCTSTR slsfile)
{
	CStdioFile f;
	if (!f.Open(slsfile, CFile::modeRead | CFile::typeText))
		return;

	CString strLine;
//>>> Spike2::SLS - changed LowID-saving
	//Anis: the proper way to fix this is by ignoring those sources on LOAD
	//Comment: I don't see any reason, why we shouldn't save LowID-sources. Especially these can be great uploaders!!!
	//There's only one circumstance, where saving LowID-sources should be avoided: if we are a LowID ourself....!
	const bool bSelfLow = theApp.IsFirewalled();
//### Spike2::SLS - changed LowID-saving
	while(f.ReadString(strLine)) 
	{
		if (strLine.GetAt(0) == L'#')
			continue;
		int pos = strLine.Find(L':');
		if (pos == -1)
			continue;

		const CString strIP = strLine.Left(pos);
		strLine = strLine.Mid(pos+1);
		UINT dwID = 0;
		if(strIP.Find(L'.') != -1) //"old" version
			dwID = inet_addr(CT2CA(strIP));
		else 
			dwID = _tstoi(strIP);
		if (dwID == INADDR_NONE || dwID == 0) //FiX
			continue;
//>>> Spike2::SLS - changed LowID-saving
		if (::IsLowID(dwID) && bSelfLow)
		{
//			DebugLog(LOG_WARNING, _T("SLS: Ignored LowID-source %s, because we're firewalled!"), cur_src->DbgGetFullClientSoftVer());
			continue;
		}
//### Spike2::SLS - changed LowID-saving
		pos = strLine.Find(L',');
		if (pos == -1)
			continue;

		const CString strPort = strLine.Left(pos);
		strLine = strLine.Mid(pos+1);
		const uint16 wPort = (uint16)_tstoi(strPort);
		if (!wPort)
			continue;
		
		pos = strLine.Find(L',');
		if (pos == -1)
			continue;
		CString strExpiration = strLine.Left(pos);
 // sivka [-bugfix-] //
		for(int i = strExpiration.GetLength(); i < 10;)
			 i = strExpiration.Insert(i, L"0");
 // sivka [-bugfix-] //
		if (IsExpired(strExpiration))
			continue;
		strLine = strLine.Mid(pos+1);
		pos = strLine.Find(',');
		if (pos == -1)
			continue;

		// khaos::kmod+ Src Ex Ver
		uint8 nSrcExchangeVer = (uint8)_tstoi(strLine.Left(pos));
		strLine = strLine.Mid(pos+1);
		pos = strLine.Find(L':');
		if (pos == -1)
			continue;
		// khaos::kmod-

		const CString strserverip = strLine.Left(pos);
		strLine = strLine.Mid(pos+1);
		UINT dwserverip = 0;
		if(strIP.Find(L'.') != -1) //"old" version
			dwserverip = inet_addr(CT2CA(strserverip));
		else
			dwserverip = _tstoi(strserverip);
		if (dwserverip == INADDR_NONE || dwserverip == 0) //FiX
			continue;

		pos = strLine.Find(L';');
		if (pos == -1 || strLine.GetLength() < 2)
			continue;
		const CString strserverport = strLine.Left(pos);
		const uint16 wserverport = (uint16)_tstoi(strserverport);
		if (!wserverport)
			continue;
		CSourceData* newsource = new CSourceData(dwID, wPort, dwserverip, wserverport, strExpiration, nSrcExchangeVer);		
		sources->AddTail(newsource);
	}
	f.Close();
}

void CSourceSaver::AddSourcesToDownload(CPartFile* file, SourceList* sources) 
{
	UINT count = 0;
	for (POSITION pos = sources->GetHeadPosition(); pos && count < thePrefs.GetSourcesToSaveSLS();)
	{
		if (file->GetMaxSources() <= file->GetSourceCount())
			break; //no need to return, we might want the log ;)

		CSourceData* cur_src = sources->GetNext(pos);
//==>List Of Dont Ask This IPs [cyrex2001]
// JvA: should be readded if Drops are implemented
/*
		if(theApp.clientlist->DontAskThisIP(cur_src->sourceID))
			continue;
*/
//<==List Of Dont Ask This IPs [cyrex2001]
		CUpDownClient* newclient = new CUpDownClient(file, cur_src->sourcePort, cur_src->sourceID, cur_src->serverip, cur_src->serverport, cur_src->nSrcExchangeVer != 3);
		if(theApp.downloadqueue->CheckAndAddSource(file, newclient))
		{
			++count;
			newclient->SetSourceFrom(SF_SLS);
		}
	}
	if(count)
		DebugLog(LOG_SUCCESS, _T("SLS: Loaded %u sources for file %s"), count, file->GetFileName());
}

#define EXPIREIN	1 //days

void CSourceSaver::SaveSources(CPartFile* file, SourceList* prevsources, LPCTSTR slsfile)
{
	//moved here - if we fail to open the file, then there's no need to parse the srclists
	CStdioFile f;
	if (!f.Open(slsfile, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
		return;

	SourceList srcstosave;
	CSourceData* sourcedata;

	POSITION pos, pos2;
	CUpDownClient* cur_src;
	const UINT maxSourcesToSave = thePrefs.GetSourcesToSaveSLS();
	// Choose best sources for the file
	for (pos = file->srclist.GetHeadPosition(); pos!=NULL;)
	{
		cur_src = file->srclist.GetNext(pos);

		if(!cur_src->IsAduClient())	  //Anis
			continue;

		switch(cur_src->GetDownloadState())
		{
			case DS_BANNED:	//bad clients
			case DS_ERROR:	//erroneous clients
				continue;
			default:
				break;
		}

		// Skip also Required Obfuscation, because we don't save the userhash (and we don't know if all settings are still valid on next restart)
		if(cur_src->RequiresCryptLayer() || thePrefs.IsClientCryptLayerRequired())
			continue;

		uint16 availParts = 0;
		for (uint16 i = 0; i < cur_src->GetPartCount(); ++i)
		{
			const uint64 uStart = PARTSIZE*i;
			const uint64 uEnd = uStart+PARTSIZE-1;
			if(cur_src->IsPartAvailable(i)
				&& !file->IsComplete(uStart, uEnd, true))
				++availParts;
		}
		if(availParts == 0)
			continue; //no use
//### Anis - it's more effective to use the USEFULL part count
		if (srcstosave.IsEmpty()) 
		{
			sourcedata = new CSourceData(cur_src, availParts, CalcExpiration(EXPIREIN));
			srcstosave.AddHead(sourcedata);
			continue;
		}		
		if ((UINT)srcstosave.GetCount() < maxSourcesToSave || (availParts > srcstosave.GetTail()->partsavailable) || (cur_src->GetSourceExchange1Version() > srcstosave.GetTail()->nSrcExchangeVer))
		{
			if ((UINT)srcstosave.GetCount() == maxSourcesToSave)
				delete srcstosave.RemoveTail();
			ASSERT((UINT)srcstosave.GetCount() < maxSourcesToSave);
			bool bInserted = false;
			for (pos2 = srcstosave.GetTailPosition(); pos2 && !bInserted; srcstosave.GetPrev(pos2))
			{
				CSourceData* cur_srctosave = srcstosave.GetAt(pos2);
				// khaos::kmod+ Source Exchange Version
				if (file->GetAvailableSrcCount() > (maxSourcesToSave*2) && cur_srctosave->nSrcExchangeVer > cur_src->GetSourceExchange1Version())
					bInserted = true;
				else if (file->GetAvailableSrcCount() > (maxSourcesToSave*2) && cur_srctosave->nSrcExchangeVer == cur_src->GetSourceExchange1Version() && cur_srctosave->partsavailable > availParts)
					bInserted = true;
				else if (file->GetAvailableSrcCount() <= (maxSourcesToSave*2) && cur_srctosave->partsavailable > availParts)
					bInserted = true;
			}
			if (!bInserted)
			{
				sourcedata = new CSourceData(cur_src, availParts, CalcExpiration(EXPIREIN));
				srcstosave.AddHead(sourcedata);
			}
		}
	}
	
	// Add previously saved sources if found sources does not reach the limit
	for (pos = prevsources->GetHeadPosition(); pos && (UINT)srcstosave.GetCount() < maxSourcesToSave;)
	{
		CSourceData* cur_sourcedata = prevsources->GetNext(pos);
		bool bFound = false;
		for (pos2 = srcstosave.GetHeadPosition(); pos2 && !bFound;)
		{
			if (srcstosave.GetNext(pos2)->Compare(cur_sourcedata)) 
				bFound = true;
		}
		if (!bFound)
			srcstosave.AddTail(new CSourceData(cur_sourcedata));
	}

	CString strLine;
	f.WriteString(_T("#format: ip:port,expirationdate(yymmddhhmm);\r\n"));
	f.WriteString(_T("#") + file->GetED2kLink() + _T("\r\n")); //MORPH - Added by IceCream, Storing ED2K link in Save Source files, To recover corrupted met by skynetman
	UINT count = 0;
	while (!srcstosave.IsEmpty())
	{
		CSourceData* cur_src = srcstosave.RemoveHead();
		UINT dwID = cur_src->sourceID;		
		if(dwID != 0) //FRTK(kts)+ fix NULL src
		{
			++count;
			uint16 wPort = cur_src->sourcePort;
			UINT dwserverip = cur_src->serverip;
			uint16 wserverport = cur_src->serverport;
			strLine.Format(_T("%u:%u,%s,%u,%u:%u;\r\n"), dwID, wPort, cur_src->expiration, cur_src->nSrcExchangeVer, dwserverip, wserverport);
			f.WriteString(strLine);
		}
		delete cur_src;
	}
	f.Close();
	if(count)
		DebugLog(LOG_SUCCESS, _T("SLS: Saved %i sources for file %s"), count, file->GetFileName());
	else
		_tremove(slsfile); //no sources saved, no need to keep the file ;)
}

CString CSourceSaver::CalcExpiration(const int nDays) const
{
	CTime expiration = CTime::GetCurrentTime()+CTimeSpan(nDays, 0, 0, 0);

	CString strExpiration;
	strExpiration.Format(L"%02i%02i%02i%02i%02i", (expiration.GetYear() % 100), expiration.GetMonth(), expiration.GetDay(), expiration.GetHour(), expiration.GetMinute());

	return strExpiration;
}

bool CSourceSaver::IsExpired(const CString& expirationdate) const
{
	CTime expiration(_tstoi(expirationdate.Mid(0, 2)) + 2000, _tstoi(expirationdate.Mid(2, 2)), _tstoi(expirationdate.Mid(4, 2)), _tstoi(expirationdate.Mid(6, 2)), _tstoi(expirationdate.Mid(8, 2)), 0);
	return (expiration < CTime::GetCurrentTime());
}
//### JvA::SLS [enkeyDEV]