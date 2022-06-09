#include "StdAfx.h"
#include "sourcesaver.h"
#include "PartFile.h"
#include "emule.h"
#include "Log.h"
#include "updownclient.h"
#include "urlclient.h" // X: [SUS] - [Save URL Source]
#include "preferences.h"
#include "downloadqueue.h"
#include "log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define RELOADTIME	3600000 //60 minutes	
#define RESAVETIME	 600000 //10 minutes

CSourceSaver::CSourceSaver(CPartFile* file)
{
	// X: [ISS] - [Improved Source Save]
	//m_dwLastTimeLoaded = 0;
	//m_dwLastTimeSaved = 0;
	m_dwLastTimeSaved = ::GetTickCount() + (t_rng->getUInt32() / (RAND32_MAX / 30000)) - 15000; // Don't save all files at the same time
	m_pFile = file;
}

CSourceSaver::~CSourceSaver(void)
{
}

CSourceSaver::CSourceData::CSourceData(CUpDownClient* client, const CString& expiration90mins, const CString& expiration3days) 
:	sourceID(client->GetUserIDHybrid()), 
	sourcePort(client->GetUserPort()),
	partsavailable(client->GetAvailablePartCount()),
	expiration90mins(expiration90mins),
    expiration3days(expiration3days)
{
	if(IsCUrlClient(client)/*client->IsKindOf(RUNTIME_CLASS(CUrlClient))*/) // X: [SUS] - [Save URL Source]
		URL = client->GetUserName();
}

bool CSourceSaver::Process() // return false if sources not saved
{
	// X: [ISS] - [Improved Source Save]
	// Load only one time the list and keep it in memory (=> reduce CPU load)
	/*if (m_dwLastTimeLoaded == 0){
		m_dwLastTimeLoaded = ::GetTickCount();
		m_dwLastTimeSaved = m_dwLastTimeLoaded + (t_rng->getUInt32() * 30000 / RAND32_MAX) - 15000; // Don't save all files at the same time

		//Xman 6.0.1 skip loading if obfuscation only
		if(!thePrefs.IsClientCryptLayerRequired())
		{
			// Load sources from the file
			LoadSourcesFromFile();
	
			// Try to add the sources
			AddSourcesToDownload();
		}
	}
	// Save the list every n minutes (default 10 minutes)
	else */if ((int)(::GetTickCount() - m_dwLastTimeSaved) > RESAVETIME) {
		SaveSources();

		// Try to reload the unsuccessfull source
		// if ((int)(::GetTickCount() - m_dwLastTimeLoaded) > RELOADTIME) {
		//	m_dwLastTimeLoaded = ::GetTickCount() + (t_rng->getUInt32() / (RAND32_MAX / 30000)) - 15000;
		//	 AddSourcesToDownload(false);
		// }

		return true;
	}
	return false;
}

void CSourceSaver::DeleteFile()
{
	m_sourceList.RemoveAll(); //Xman x4.1
	CString slsfilepath;
	slsfilepath.Format(_T("%s\\%s.txtsrc"), m_pFile->GetTempPath(), m_pFile->GetPartMetFileName());
	if (_tremove(slsfilepath)) if (errno != ENOENT)
		AddLogLine(true, _T("Failed to delete %s, you will need to do this by hand"), slsfilepath);    
}

void CSourceSaver::LoadSourcesFromFile() // X: [ISS] - [Improved Source Save]
{
	CString slsfile;
	slsfile.Format(_T("%s\\%s.txtsrc"), m_pFile->GetTempPath(), m_pFile->GetPartMetFileName());
	CString strLine;
	CStdioFile f;
	if (!f.Open(slsfile, CFile::modeRead | CFile::typeText))
		return;
	uint_ptr count = 0;
	while(f.ReadString(strLine)) {
		// Skip comment (e.g. title)
		if (strLine.GetAt(0) == '#')
			continue;

		// Load IP
		int pos = strLine.Find(':');
		if (pos == -1)
			continue;
		CStringA strIP(strLine.Left(pos));
		// X: [SUS] - [Save URL Source]
		uint32 dwID;
		uint16 wPort = 0;
		if(strIP.CompareNoCase("http")){
			strLine = strLine.Mid(pos+1);
			//Xman 6.0.1 skip loading if obfuscation only
			if(thePrefs.IsClientCryptLayerRequired())
				continue;
			dwID = inet_addr(strIP);
			if (dwID == INADDR_NONE)
				continue;

			// Load Port
			pos = strLine.Find(',');
			if (pos == -1)
				continue;
			CString strPort = strLine.Left(pos);
			wPort = (uint16)_tstoi(strPort);
			if (!wPort)
				continue;
		}
		else{
			pos = strLine.Find(' ');
			if (pos == -1)
				continue;
			strIP = strLine.Left(pos);
		}

		strLine = strLine.Mid(pos+1);
		// Load expiration time (short version => usualy for 3 days)
		pos = strLine.Find(';');
		if (pos == -1)
			continue;
		CString expiration3days = strLine.Left(pos);
		strLine = strLine.Mid(pos+1);

		// Load expiration time (short version => usualy for 1.5 hours)
		pos = strLine.Find(';');
        CString expiration90mins;
		if (pos != -1){
		    expiration90mins = strLine.Left(pos);
		    strLine = strLine.Mid(pos+1);
        }

		if (IsExpired(expiration90mins) == true){
			if(IsExpired(expiration3days) == true)
				continue;
            expiration90mins.Empty(); // Erase
		}

		// Add source to list
		if(wPort)
			m_sourceList.AddTail(CSourceData(dwID, wPort, expiration90mins, expiration3days));
		else
			m_sourceList.AddTail(CSourceData(strIP, expiration90mins, expiration3days));
		++count;
	}
    f.Close();

	AddDebugLogLine(false, _T("%u %s loaded for the file '%s'"), count, (count>1) ?  _T("sources") : _T("source"), m_pFile->GetFileName());
}

void CSourceSaver::AddSourcesToDownload(){
	uint_ptr count = 0;
	for(POSITION pos = m_sourceList.GetHeadPosition(); pos != NULL && m_pFile->GetMaxSources()  > m_pFile->GetSourceCount() && m_pFile->IsGlobalSourceAddAllowed(); ){
		// Check if the limit of allowed source was reached
		if(m_pFile->GetMaxSources() <= m_pFile->GetSourceCount())
			break;

		// Try to add new sources
        // within 3 days => load only 10
        // within 1.5 hours => load all		
	    const CSourceData& cur_src = m_sourceList.GetNext(pos);
        if(count < 10 || IsExpired(cur_src.expiration90mins) == false){
            ++count;
			if(cur_src.URL.IsEmpty()){ // X: [SUS] - [Save URL Source]
				CUpDownClient* newclient = new CUpDownClient(m_pFile, cur_src.sourcePort, cur_src.sourceID, 0, 0, false);
				newclient->SetSourceFrom(SF_SLS);
				theApp.downloadqueue->CheckAndAddSource(m_pFile, newclient);
			}
			else{
				CUrlClient* newclient = new CUrlClient();
				if (!newclient->SetUrl(cur_src.URL, 0))
				{
					LogError(LOG_STATUSBAR, _T("Failed to process URL source \"%s\""), cur_src.URL);
					delete newclient;
					continue;
				}
				newclient->SetRequestFile(m_pFile);
				newclient->SetSourceFrom(SF_SLS);
				theApp.downloadqueue->CheckAndAddSource(m_pFile, newclient);
			}

        }
	}
	AddDebugLogLine(false, _T("add %u saved %s to the file '%s'"), count, (count>1) ?  _T("sources") : _T("source"), m_pFile->GetFileName());
}

void CSourceSaver::SaveSources() // X: [ISS] - [Improved Source Save]
{
	m_dwLastTimeSaved = ::GetTickCount() + (t_rng->getUInt32() / (RAND32_MAX / 30000)) - 15000; // Don't save all files at the same time

	const CString expiration90mins = CalcExpiration();
	const CString expiration3days = CalcExpirationLong();

	//Xman keep old sources only for rar files
	//the old version of the sourcesaver had a big bug if we have many sources per file
	UINT validsources=m_pFile->GetValidSourcesCount();
	if (validsources>25) 
	{
		//keep only current sources, remove the old one
		m_sourceList.RemoveAll();
	}	

	//Xman x4.1
	//remove expired for rare files
	POSITION pos=m_sourceList.GetTailPosition();
	while(pos!=NULL && m_sourceList.GetCount()>10 )
	{
		POSITION cur_pos = pos;
		m_sourceList.GetPrev(pos);
		if(IsExpired(m_sourceList.GetAt(cur_pos).expiration90mins))
			m_sourceList.RemoveAt(cur_pos);
	}
	//Xman end

	// Update sources list for the file
	for(POSITION pos1 = m_pFile->srclist.GetHeadPosition(); pos1 != NULL; ){
		CUpDownClient* cur_src = m_pFile->srclist.GetNext(pos1);

		// Skip leecher sources if we have lots of sources or lowID sources
		if (cur_src->HasLowID() 
#ifdef CLIENTANALYZER				
			|| cur_src->IsBadGuy()
#endif		
			)
			continue;

		// Skip also Required Obfuscation, because we don't save the userhash (and we don't know if all settings are still valid on next restart)
		if (cur_src->RequiresCryptLayer())
			continue;

		CSourceData sourceData(cur_src, expiration90mins, expiration3days);

		// Update or add a source
		if (validsources<=25) 
			for(POSITION pos2 = m_sourceList.GetHeadPosition(); pos2 != NULL;) {
				POSITION cur_pos = pos2;
				const CSourceData& cur_sourcedata = m_sourceList.GetNext(pos2);
				if(cur_sourcedata.Compare(sourceData) == true){
					m_sourceList.RemoveAt(cur_pos);
					break; // exit loop for()
				}
			}

		// Add source to the list
        if(m_sourceList.IsEmpty() == TRUE){
            m_sourceList.AddHead(sourceData);
        }
        else{
	        for(POSITION pos2 = m_sourceList.GetHeadPosition(); pos2 != NULL;) {
		        POSITION cur_pos = pos2;
		        const CSourceData& cur_sourcedata = m_sourceList.GetNext(pos2);
		        if((cur_src->GetDownloadState() == DS_ONQUEUE || cur_src->GetDownloadState() == DS_DOWNLOADING) && 
                    (sourceData.partsavailable >= cur_sourcedata.partsavailable)){
                    // Use the state and the number of available part to sort the list
			        m_sourceList.InsertBefore(cur_pos, sourceData);
			        break; // Exit loop
		        } 
                else if(cur_sourcedata.partsavailable == 0){
                    // Use the number of available part to sort the list
			        m_sourceList.InsertBefore(cur_pos, sourceData);
			        break; // Exit loop
                }
		        else if(pos2 == NULL){						
			        m_sourceList.AddTail(sourceData);
			        break; // Exit loop
		        }
	        }
        }
	}
	
	// Save sources to the file
	CString slsfile;
	slsfile.Format(_T("%s\\%s.txtsrc"), m_pFile->GetTempPath(), m_pFile->GetPartMetFileName());
	CString strLine;
	CStdioFile f;
	if (!f.Open(slsfile, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
		return;
	f.WriteString(_T("#format: a.b.c.d:port,expirationdate-3day(yymmdd);expirationdate-1.5hour(yymmddhhmm);\r\n"));
	uint_ptr counter = 0;
	for(POSITION pos = m_sourceList.GetHeadPosition(); pos != NULL; ){
        //POSITION cur_pos = pos;
		const CSourceData& cur_src = m_sourceList.GetNext(pos);
        //if(cur_src.partsavailable > 0){
		if(cur_src.URL.IsEmpty()){ // X: [SUS] - [Save URL Source]
	        strLine.Format(_T("%i.%i.%i.%i:%i,%s;%s;\r\n"), 
				        (uint8)cur_src.sourceID, (uint8)(cur_src.sourceID>>8), (uint8)(cur_src.sourceID>>16), (uint8)(cur_src.sourceID>>24),
				        cur_src.sourcePort, 
						(counter < 10)?cur_src.expiration3days:_T("000101"),
				        cur_src.expiration90mins);
		}
		else{
			strLine.Format(_T("%s %s;%s;\r\n"), 
				cur_src.URL,
				(counter < 10)?cur_src.expiration3days:_T("000101"),
				cur_src.expiration90mins);
		}
        ++counter;
	    f.WriteString(strLine);
        /*
		}
        else if(counter < 10){
		    strLine.Format(_T("%i.%i.%i.%i:%i,%s;%s;\r\n"), 
					    (uint8)cur_src.sourceID, (uint8)(cur_src.sourceID>>8), (uint8)(cur_src.sourceID>>16), (uint8)(cur_src.sourceID>>24),
					    cur_src.sourcePort, 
                        cur_src.expiration3days,
					    _T(""));
            ++counter;
		    f.WriteString(strLine);
        }
        else
        {
            m_sourceList.RemoveAt(cur_pos);
        }*/
	}
	f.Close();
	if (thePrefs.GetLogFileSaving())
		AddDebugLogLine(false, _T("%u %s saved for the file '%s'"), counter, (counter>1) ?  _T("sources") : _T("source"), m_pFile->GetFileName());
}

CString CSourceSaver::CalcExpiration()
{
	CTime expiration90mins = CTime::GetCurrentTime();
	CTimeSpan timediff(0, 1, 0, 0); //Xman 1 hour
	expiration90mins += timediff;
    
	CString strExpiration;
	strExpiration.Format(_T("%02i%02i%02i%02i%02i"), 
						 (expiration90mins.GetYear() % 100), 
						 expiration90mins.GetMonth(), 
						 expiration90mins.GetDay(),
						 expiration90mins.GetHour(),
						 expiration90mins.GetMinute());

	return strExpiration;
}

CString CSourceSaver::CalcExpirationLong()
{
	CTime expiration3days = CTime::GetCurrentTime();
	CTimeSpan timediff(2, 0, 0, 0); //Xman 2 days
	expiration3days += timediff;
    
	CString strExpiration;
	strExpiration.Format(_T("%02i%02i%02i"), 
						 (expiration3days.GetYear() % 100), 
						  expiration3days.GetMonth(), 
						  expiration3days.GetDay());

	return strExpiration;
}

bool CSourceSaver::IsExpired(const CString& expiration) const
{
	// example: "yymmddhhmm"
	if(expiration.GetLength() == 10 || expiration.GetLength() == 6){
		int year = _tstoi(expiration.Mid(0, 2)) + 2000;
		int month = _tstoi(expiration.Mid(2, 2));
		int day = _tstoi(expiration.Mid(4, 2));
		int hour = (expiration.GetLength() == 10) ? _tstoi(expiration.Mid(6, 2)) : 0;
		int minute = (expiration.GetLength() == 10) ? _tstoi(expiration.Mid(8, 2)) : 0;

		CTime date(year, month, day, hour, minute, 0);
		return (date < CTime::GetCurrentTime());
	}

	return true;
}
