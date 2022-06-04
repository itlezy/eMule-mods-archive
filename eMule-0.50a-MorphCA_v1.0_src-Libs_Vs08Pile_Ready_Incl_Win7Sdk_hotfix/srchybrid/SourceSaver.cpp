#include "StdAfx.h"
#include "sourcesaver.h"
#include "PartFile.h"
#include "emule.h"
#include "emuledlg.h"
#include "OtherFunctions.h"
#include "DownloadQueue.h"
#include "updownclient.h"
#include "Preferences.h"
#include "Log.h"

#define RELOADTIME	3600000 //60 minutes	
#define RESAVETIME	 600000 //10 minutes


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
	//MORPH - Changed by SiRoB, SLS keep only for rar files, reduce Saved Source and life time
	//memcpy(expiration, exp, 7);
	memcpy(expiration, exp, 11*sizeof(TCHAR));
	//MORPH - Changed by SiRoB, SLS keep only for rar files, reduce Saved Source and life time
	//expiration[6] = 0;
	expiration[10] = 0;
}


CSourceSaver::~CSourceSaver(void)
{
}

bool CSourceSaver::Process(CPartFile* file, UINT maxSourcesToSave) // return false if sources not saved
{
	if ((int)(::GetTickCount() - m_dwLastTimeSaved) > RESAVETIME) {
		TCHAR szslsfilepath[_MAX_PATH];
		_tmakepath(szslsfilepath,NULL,(CString)file->GetTempPath()+_T("\\Source Lists"), file->GetPartMetFileName(),_T(".txtsrc"));
	
		//MORPH - Changed by SiRoB, SLS keep only for rare files, reduce Saved Source and life time
		//if (file->GetAvailableSrcCount() > 100 && file->GetDownPriority() < PR_HIGH)
		if (file->GetAvailableSrcCount() > 25)
		{
			if (PathFileExists(szslsfilepath))
				_tremove(szslsfilepath);
			return false;
		}
		m_dwLastTimeSaved = ::GetTickCount() + (rand() * 30000 / RAND_MAX) - 15000;
		SourceList srcs;
		LoadSourcesFromFile(file, &srcs, szslsfilepath);
		SaveSources(file, &srcs, szslsfilepath, maxSourcesToSave);
		
		if ((int)(::GetTickCount() - m_dwLastTimeLoaded) > RELOADTIME) {
			m_dwLastTimeLoaded = ::GetTickCount() + (rand() * 30000 / RAND_MAX) - 15000;
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
	_tmakepath(szslsfilepath,NULL,(CString)file->GetTempPath()+_T("\\Source Lists"), file->GetPartMetFileName(),_T(".txtsrc"));
	if (_tremove(szslsfilepath)) if (errno != ENOENT)
		AddLogLine(true, _T("Failed to delete 'Temp\\Source Lists\\%s.txtsrc', you will need to do this by hand."), file->GetPartMetFileName());    
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
		if (dwID == INADDR_NONE) 
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
		if (IsExpired(strExpiration))
			continue;
		strLine = strLine.Mid(pos+1);
		pos = strLine.Find(',');
		if (pos == -1)
			continue;
		uint8 nSrcExchangeVer = (uint8)_tstoi(strLine.Left(pos));
		strLine = strLine.Mid(pos+1);
		pos = strLine.Find(':');
		if (pos == -1)
			continue;
		CString strserverip = strLine.Left(pos);
		strLine = strLine.Mid(pos+1);
		uint32 dwserverip = inet_addr(CT2CA(strserverip));
		if (dwserverip == INADDR_NONE) 
			continue;
		pos = strLine.Find(';');
		if (pos == -1 || strLine.GetLength() < 2)
			continue;
		CString strserverport = strLine.Left(pos);
		uint16 wserverport = (uint16)_tstoi(strserverport);
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
	for (POSITION pos = sources->GetHeadPosition(); pos; sources->GetNext(pos)) {
		if (file->GetMaxSources() <= file->GetSourceCount())
			return;
    
		CSourceData* cur_src = sources->GetAt(pos);
		CUpDownClient* newclient; 
		//MORPH START - Changed by SiRoB, SLS keep only for rar files, reduce Saved Source and life time
		//newclient = new CUpDownClient(file, cur_src->sourcePort, cur_src->sourceID, 0, 0);
		if( cur_src->nSrcExchangeVer >= 3 )
				newclient = new CUpDownClient(file, cur_src->sourcePort, cur_src->sourceID, cur_src->serverip, cur_src->serverport, false);
		else
				newclient = new CUpDownClient(file, cur_src->sourcePort, cur_src->sourceID, cur_src->serverip, cur_src->serverport, true);
		newclient->SetSourceFrom(SF_SLS);
		//MORPH END   - Changed by SiRoB, SLS keep only for rar files, reduce Saved Source and life time
		theApp.downloadqueue->CheckAndAddSource(file, newclient);
        
	}
	//AddLogLine(/*TBN_NONOTIFY, */false, "Loaded %i sources for file %s", sources->GetCount(), file->GetFileName());	

}

//#define SOURCESTOSAVE	10
//MORPH - Changed by SiRoB, SLS keep only for rar files, reduce Saved Source and life time
//#define EXPIREIN		3 //Day
#define EXPIREIN		30 //Minute

void CSourceSaver::SaveSources(CPartFile* file, SourceList* prevsources, LPCTSTR slsfile, UINT maxSourcesToSave)
{
	SourceList srcstosave;
	CSourceData* sourcedata;

	ASSERT(srcstosave.IsEmpty());

	POSITION pos2,pos;								
	CUpDownClient* cur_src;
	// Choose best sources for the file
	for(pos = file->srclist.GetHeadPosition();pos!=0;){
		cur_src = file->srclist.GetNext(pos);
		if (cur_src->GetDownloadState() != DS_ONQUEUE &&
			cur_src->GetDownloadState() != DS_DOWNLOADING &&
			cur_src->GetDownloadState() != DS_NONEEDEDPARTS ||
			cur_src->IsEd2kClient() == false)
			continue;
		if (srcstosave.IsEmpty()) {
			sourcedata = new CSourceData(cur_src, CalcExpiration(EXPIREIN));
			srcstosave.AddHead(sourcedata);
			continue;
		}
		// Skip also Required Obfuscation, because we don't save the userhash (and we don't know if all settings are still valid on next restart)
		if (cur_src->RequiresCryptLayer() ||thePrefs.IsClientCryptLayerRequired())
			continue;
		if ((UINT)srcstosave.GetCount() < maxSourcesToSave || (cur_src->GetAvailablePartCount() > srcstosave.GetTail()->partsavailable) || (cur_src->GetSourceExchange1Version() > srcstosave.GetTail()->nSrcExchangeVer)) {
			if ((UINT)srcstosave.GetCount() == maxSourcesToSave)
				delete srcstosave.RemoveTail();
			ASSERT((UINT)srcstosave.GetCount() < maxSourcesToSave);
			bool bInserted = false;
			for (pos2 = srcstosave.GetTailPosition();pos2 != 0;srcstosave.GetPrev(pos2)){
				CSourceData* cur_srctosave = srcstosave.GetAt(pos2);
				// khaos::kmod+ Source Exchange Version
				if (file->GetAvailableSrcCount() > (maxSourcesToSave*2) &&
					cur_srctosave->nSrcExchangeVer > cur_src->GetSourceExchange1Version())
				{
					bInserted = true;
				}
				else if (file->GetAvailableSrcCount() > (maxSourcesToSave*2) && 
							cur_srctosave->nSrcExchangeVer == cur_src->GetSourceExchange1Version() &&
							cur_srctosave->partsavailable > cur_src->GetAvailablePartCount())
				{
					bInserted = true;
				}
				else if (file->GetAvailableSrcCount() <= (maxSourcesToSave*2) &&
							cur_srctosave->partsavailable > cur_src->GetAvailablePartCount())
				{
					bInserted = true;
				}
				const uint8* srcstatus = cur_src->GetPartStatus();
				if (srcstatus){
					if (cur_src->GetPartCount() == file->GetPartCount()){
						// only save sources which have needed parts
						for (uint16 x = 0; x < file->GetPartCount(); x++){
							//MORPH - Changed by SiRoB, ICS merged into partstatus
							/*
							if (srcstatus[x] && !file->IsPartShareable(x)){
							*/
							if ((srcstatus[x]&SC_AVAILABLE) && !file->IsPartShareable(x)){
								bInserted = true;
								break;
							}
						}
					}
				}
				if (bInserted)
				{
					sourcedata = new CSourceData(cur_src, CalcExpiration(EXPIREIN));
					srcstosave.InsertAfter(pos2, sourcedata);
					break;
				}
				// khaos::kmod-
			}
			if (!bInserted) {
				sourcedata = new CSourceData(cur_src, CalcExpiration(EXPIREIN));
				srcstosave.AddHead(sourcedata);
			}
		}
	}
	
	// Add previously saved sources if found sources does not reach the limit
	for (pos = prevsources->GetHeadPosition(); pos; prevsources->GetNext(pos)) {
		CSourceData* cur_sourcedata = prevsources->GetAt(pos);
		if ((UINT)srcstosave.GetCount() == maxSourcesToSave)
			break;
		ASSERT((UINT)srcstosave.GetCount() <= maxSourcesToSave);

		bool bFound = false;
		for (pos2 = srcstosave.GetHeadPosition(); pos2; srcstosave.GetNext(pos2)) {
			if (srcstosave.GetAt(pos2)->Compare(cur_sourcedata)) {
				bFound = true;
				break;
			}
		}
		if (!bFound) {
			srcstosave.AddTail(new CSourceData(cur_sourcedata));
		}
			
	}

	//DEBUG_ONLY(AddLogLine(/*TBN_NONOTIFY, */false, "Saving %i sources for file %s", srcstosave.GetCount(), file->GetFileName()));	

	CString strLine;
	CStdioFile f;
	if (!f.Open(slsfile, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
		return;
	f.WriteString(_T("#format: a.b.c.d:port,expirationdate(yymmddhhmm);\r\n"));
	f.WriteString(_T("#") + file->GetED2kLink() + _T("\r\n")); //MORPH - Added by IceCream, Storing ED2K link in Save Source files, To recover corrupted met by skynetman
	while (!srcstosave.IsEmpty()) {
		CSourceData* cur_src = srcstosave.RemoveHead();
		uint32 dwID = cur_src->sourceID;
		uint16 wPort = cur_src->sourcePort;
		uint32 dwserverip = cur_src->serverip;
		uint16 wserverport = cur_src->serverport;
		strLine.Format(_T("%s:%i,%s,%i,%s:%i;\r\n"), ipstr(dwID), wPort, cur_src->expiration, cur_src->nSrcExchangeVer, ipstr(dwserverip), wserverport);
		delete cur_src;
		f.WriteString(strLine);
	}
	f.Close();
}

//MORPH - Changed by SiRoB, SLS keep only for rar files, reduce saved source and life time
//CString CSourceSaver::CalcExpiration(int Days)
CString CSourceSaver::CalcExpiration(int Minutes)
{
	CTime expiration = CTime::GetCurrentTime();
	//MORPH - Changed by SiRoB, SLS keep only for rare files, reduce Saved Source and life time
	//CTimeSpan timediff(Days, 0, 0, 0);
	CTimeSpan timediff(Minutes/1440, (Minutes/60) % 24, Minutes % 60, 0);
	expiration += timediff;
    
	CString strExpiration;
	//MORPH - Changed by SiRoB, SLS keep only for rare files, reduce Saved Source and life time
	//strExpiration.Format("%02i%02i%02i", (expiration.GetYear() % 100), expiration.GetMonth(), expiration.GetDay());
	strExpiration.Format(_T("%02i%02i%02i%02i%02i"), (expiration.GetYear() % 100), expiration.GetMonth(), expiration.GetDay(), expiration.GetHour(),expiration.GetMinute());

	return strExpiration;
}

bool CSourceSaver::IsExpired(CString expirationdate)
{
	int year = _tstoi(expirationdate.Mid(0, 2)) + 2000;
	int month = _tstoi(expirationdate.Mid(2, 2));
	int day = _tstoi(expirationdate.Mid(4, 2));
	//MORPH - Added by SiRoB, SLS keep only for rare files, reduce Saved Source and life time
	int hour = _tstoi(expirationdate.Mid(6, 2));
	int minute = _tstoi(expirationdate.Mid(8, 2));
	
	//MORPH - Changed by SiRoB, SLS keep only for rare files, reduce Saved Source and life time
	//CTime expiration(year, month, day, 0, 0, 0);
	CTime expiration(year, month, day, hour, minute, 0);
	return (expiration < CTime::GetCurrentTime());
}
