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
#include "Preferences.h"
#include "emuleDlg.h"
#include "DownloadQueue.h"
#include "OtherFunctions.h"
#include "Log.h"
#include "Sockets.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#define RELOADTIME	3600000 //60 minutes
#define RESAVETIME	1200000 //20 minutes <<<!!!>>>
#define ACTIVATIONLIMIT (thePrefs.GetActivationLimitSLS())


CSourceSaver::CSourceSaver(void)
{
	m_dwLastTimeLoaded = ::GetTickCount() - RELOADTIME;
	m_dwLastTimeSaved = ::GetTickCount() + (rand() * 30000 / RAND_MAX) - 15000 - RESAVETIME;
}

CSourceSaver::CSourceData::CSourceData(CUpDownClient* client, const TCHAR* exp) 
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
	partsavailable = client->GetAvailablePartCount();
	memcpy(expiration, exp, 11*sizeof(TCHAR));
	expiration[10] = 0;
}

CSourceSaver::~CSourceSaver(void)
{
}

bool CSourceSaver::Process(CPartFile* file) // return false if sources not saved
{
	if (!thePrefs.UseSaveLoadSources()) 
		return false;

	if ((int)(::GetTickCount() - m_dwLastTimeSaved) > RESAVETIME){
		TCHAR szslsfilepath[_MAX_PATH];
		_tmakepath(szslsfilepath,NULL,(CString)file->GetTempPath()+_T("\\Saved Sources"), file->GetPartMetFileName(),_T(".txtsrc"));
	
		if (file->GetAvailableSrcCount() > ACTIVATIONLIMIT){
			if (PathFileExists(szslsfilepath))
				_tremove(szslsfilepath);
			return false;
		}
		m_dwLastTimeSaved = ::GetTickCount() + (rand() * 30000 / RAND_MAX) - 15000;
		SourceList srcs;
		if (!thePrefs.IsClientCryptLayerRequired())
			LoadSourcesFromFile(file, &srcs, szslsfilepath);
		SaveSources(file, &srcs, szslsfilepath, thePrefs.GetSourcesToSaveSLS());
		
		if ((int)(::GetTickCount() - m_dwLastTimeLoaded) >= RELOADTIME){	
			m_dwLastTimeLoaded = ::GetTickCount() + (rand() * 30000 / RAND_MAX) - 15000;
			if (!thePrefs.IsClientCryptLayerRequired())
				AddSourcesToDownload(file, &srcs);
		}

		while (!srcs.IsEmpty()) 
			delete srcs.RemoveHead();
		
		return true;
	}
	return false;
}

void CSourceSaver::DeleteFile(CPartFile* file)
{
	TCHAR szslsfilepath[_MAX_PATH];
	// khaos::kmod+ Source Lists directory
	_tmakepath(szslsfilepath,NULL,(CString)file->GetTempPath()+_T("\\Saved Sources"), file->GetPartMetFileName(),_T(".txtsrc"));
	if (_tremove(szslsfilepath)) if (errno != ENOENT)
		if (thePrefs.GetLogSLSEvents())	// Tux: Improvement: log SLS events
			DebugLog(LOG_ERROR | LOG_STATUSBAR, _T("Failed to delete 'Temp\\Saved Sources\\%s.txtsrc', you will need to do this by hand."), file->GetPartMetFileName());    
}

void CSourceSaver::LoadSourcesFromFile(CPartFile* , SourceList* sources, LPCTSTR slsfile)
{
	CString strLine;
	CStdioFile f;
	if (!f.Open(slsfile, CFile::modeRead | CFile::typeText))
		return;
	while(f.ReadString(strLine)) {
		if (strLine.GetAt(0) == '#')
			continue;
		int pos = strLine.Find(':');
		if (pos == -1)
			continue;
		CString strIP = strLine.Left(pos);
		strLine = strLine.Mid(pos+1);
		uint32 dwID = inet_addr(CT2CA(strIP));
		if (dwID == INADDR_NONE || dwID == 0) //Fix
			continue;
		pos = strLine.Find(',');
		if (pos == -1)
			continue;
		CString strPort = strLine.Left(pos);
		strLine = strLine.Mid(pos+1);
		uint16 wPort = (uint16)_tstoi(strPort);
		if (!wPort)
			continue;
		// khaos::kmod+ Src Ex Ver
		pos = strLine.Find(',');
		if (pos == -1)
			continue;
		CString strExpiration = strLine.Left(pos);
 // sivka [-bugfix-] //
		for(int i = strExpiration.GetLength(); i < 10; )
			 i = strExpiration.Insert(i, _T("0"));
 // sivka [-bugfix-] //
		if (IsExpired(strExpiration))
			continue;
		strLine = strLine.Mid(pos+1);
		pos = strLine.Find(',');
		if (pos == -1)
			continue;
		uint8 nSrcExchangeVer = (uint8)_tstoi(strLine.Left(pos));
		strLine = strLine.Mid(pos+1);
		pos = strLine.Find(L':');
		if (pos == -1)
			continue;
		const CString strserverip = strLine.Left(pos);
		strLine = strLine.Mid(pos+1);
		uint32 dwserverip = inet_addr(CT2CA(strserverip));
		if (dwserverip == INADDR_NONE) 
			continue;

		pos = strLine.Find(L';');
		if (pos == -1 || strLine.GetLength() < 2)
			continue;
		const CString strserverport = strLine.Left(pos);
		const uint16 wserverport = (uint16)_tstoi(strserverport);
		if (!wserverport)
			continue;
		CSourceData* newsource = new CSourceData(dwID, wPort, dwserverip, wserverport, strExpiration, nSrcExchangeVer);
		// khaos::kmod-
		sources->AddTail(newsource);
	}
	f.Close();
}

void CSourceSaver::AddSourcesToDownload(CPartFile* file, SourceList* sources) 
{
	for (POSITION pos = sources->GetHeadPosition(); pos; sources->GetNext(pos))
	{
		if (file->GetMaxSources() <= file->GetSourceCount())
			break; //no need to return, we might want the log ;)

		CSourceData* cur_src = sources->GetAt(pos);
		CUpDownClient* newclient; 

		if( cur_src->nSrcExchangeVer == 3 )
			newclient = new CUpDownClient(file, cur_src->sourcePort, cur_src->sourceID, cur_src->serverip, cur_src->serverport, false);
		else
			newclient = new CUpDownClient(file, cur_src->sourcePort, cur_src->sourceID, cur_src->serverip, cur_src->serverport, true);
		newclient->SetSourceFrom(SF_SLS);
		theApp.downloadqueue->CheckAndAddSource(file, newclient);
	}
	if(sources->GetCount() && thePrefs.GetLogSLSEvents())	// Tux: Improvement: log SLS events: added check
		DebugLog(LOG_SUCCESS, _T("SLS: Loaded %i sources for file %s"), sources->GetCount(), file->GetFileName());
}

#define EXPIREIN	1 //days

void CSourceSaver::SaveSources(CPartFile* file, SourceList* prevsources, LPCTSTR slsfile, UINT maxSourcesToSave)
{
	//moved here - if we fail to open the file, then there's no need to parse the srclists
	CStdioFile f;
	if (!f.Open(slsfile, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
		return;

	SourceList srcstosave;
	CSourceData* sourcedata;

	POSITION pos, pos2;
	CUpDownClient* cur_src;
	// Choose best sources for the file
	for (pos = file->srclist.GetHeadPosition();pos!=NULL;){
		cur_src = file->srclist.GetNext(pos);
		if (cur_src->GetDownloadState() != DS_ONQUEUE && cur_src->GetDownloadState() != DS_DOWNLOADING && cur_src->GetDownloadState() != DS_NONEEDEDPARTS)
			continue;

		// Comment: I don't see any reason, why we shouldn't save LowID-sources. Especially these can be
		// great Uploaders !!! There's only one circumstance, were saving LowID-sources should be avoided:
		// if we are a LowID ourself....!
		// Original code:
		// if (cur_src->HasLowID())
		//		continue;
		if (cur_src->HasLowID() && theApp.IsFirewalled()){ // now it should work correctly
			if (thePrefs.GetLogSLSEvents())	// Tux: Improvement: log SLS events
				DebugLog(LOG_WARNING, _T("SLS: Ignored LowID-source %s, because we're on a LowID ourself!"), cur_src->DbgGetFullClientSoftVer());
			continue;
		}
		
		if (srcstosave.IsEmpty()) {
			sourcedata = new CSourceData(cur_src, CalcExpiration(EXPIREIN));
			srcstosave.AddHead(sourcedata);
			continue;
		}
		if ((UINT)srcstosave.GetCount() < maxSourcesToSave || (cur_src->GetAvailablePartCount() > srcstosave.GetTail()->partsavailable) || (cur_src->GetSourceExchange1Version() > srcstosave.GetTail()->nSrcExchangeVer)){
			if ((UINT)srcstosave.GetCount() == maxSourcesToSave)
				delete srcstosave.RemoveTail();
			ASSERT((UINT)srcstosave.GetCount() < maxSourcesToSave);
			bool bInserted = false;
			for (pos2 = srcstosave.GetTailPosition();pos2 != NULL;srcstosave.GetPrev(pos2)){
				CSourceData* cur_srctosave = srcstosave.GetAt(pos2);
				// khaos::kmod+ Source Exchange Version
				if (file->GetAvailableSrcCount() > (maxSourcesToSave*2) && cur_srctosave->nSrcExchangeVer > cur_src->GetSourceExchange1Version())
					bInserted = true;
				else if (file->GetAvailableSrcCount() > (maxSourcesToSave*2) && cur_srctosave->nSrcExchangeVer == cur_src->GetSourceExchange1Version() && cur_srctosave->partsavailable > cur_src->GetAvailablePartCount())
					bInserted = true;
				else if (file->GetAvailableSrcCount() <= (maxSourcesToSave*2) && cur_srctosave->partsavailable > cur_src->GetAvailablePartCount())
					bInserted = true;
				uint8* srcstatus = cur_src->GetPartStatus();
				if (srcstatus){
					if (cur_src->GetPartCount() == file->GetPartCount()){
						// only save sources which have needed parts
						for (int x = 0; x < file->GetPartCount(); x++){
							if (srcstatus[x]){
								bInserted = true;
								break;
							}
						}
					}
				}
				if (bInserted){
					sourcedata = new CSourceData(cur_src, CalcExpiration(EXPIREIN));
					srcstosave.InsertAfter(pos2, sourcedata);
					break;
				}
				// khaos::kmod-
			}
			if (!bInserted){
				sourcedata = new CSourceData(cur_src, CalcExpiration(EXPIREIN));
				srcstosave.AddHead(sourcedata);
			}
		}
	}
	
	// Add previously saved sources if found sources does not reach the limit
	for (pos = prevsources->GetHeadPosition(); pos; prevsources->GetNext(pos)){
		CSourceData* cur_sourcedata = prevsources->GetAt(pos);
		if ((UINT)srcstosave.GetCount() == maxSourcesToSave)
			break;
		ASSERT((UINT)srcstosave.GetCount() <= maxSourcesToSave);

		bool bFound = false;
		for (pos2 = srcstosave.GetHeadPosition(); pos2; srcstosave.GetNext(pos2)){
			if (srcstosave.GetAt(pos2)->Compare(cur_sourcedata)){
				bFound = true;
				break;
			}
		}
		if (!bFound)
			srcstosave.AddTail(new CSourceData(cur_sourcedata));
	}

	if(srcstosave.GetCount() && thePrefs.GetLogSLSEvents())	// Tux: Improvement: log SLS events: added check
		DebugLog(LOG_SUCCESS, _T("SLS: Saved %i sources for file %s"), srcstosave.GetCount(), file->GetFileName());

	CString strLine;
	f.WriteString(_T("#format: a.b.c.d:port,expirationdate(yymmddhhmm);\r\n"));
	f.WriteString(_T("#") + file->GetED2kLink() + _T("\r\n"));
	while (!srcstosave.IsEmpty()) {
		CSourceData* cur_src = srcstosave.RemoveHead();
		uint32 dwID = cur_src->sourceID;
		uint16 wPort = cur_src->sourcePort;
		uint32 dwserverip = cur_src->serverip;
		uint16 wserverport = cur_src->serverport;
		strLine.Format(_T("%s:%i,%s,%i,%s:%i;\r\n"), ipstr(dwID), wPort, cur_src->expiration, cur_src->nSrcExchangeVer, ipstr(dwserverip), wserverport);
		delete cur_src;
		if(dwID != 0)
			f.WriteString(strLine);
	}
	f.Close();
}

CString CSourceSaver::CalcExpiration(int nDays)
{
	CTime expiration = CTime::GetCurrentTime();
	CTimeSpan timediff(nDays, 0, 0, 0);
	expiration += timediff;
    
	CString strExpiration;
	strExpiration.Format(_T("%02i%02i%02i%02i%02i"), (expiration.GetYear() % 100), expiration.GetMonth(), expiration.GetDay(), expiration.GetHour(),expiration.GetMinute());

	return strExpiration;
}

bool CSourceSaver::IsExpired(CString expirationdate)
{
	int year = _tstoi(expirationdate.Mid(0, 2)) + 2000;
	int month = _tstoi(expirationdate.Mid(2, 2));
	int day = _tstoi(expirationdate.Mid(4, 2));
	int hour = _tstoi(expirationdate.Mid(6, 2));
	int minute = _tstoi(expirationdate.Mid(8, 2));

	CTime expiration(year, month, day, hour, minute, 0);
	return (expiration < CTime::GetCurrentTime());
}
