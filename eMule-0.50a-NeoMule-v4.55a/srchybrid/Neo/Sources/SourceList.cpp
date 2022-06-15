//this file is part of NeoMule
//Copyright (C)2006-2008 David Xanatos ( XanatosDavid@googlemail.com / http://NeoMule.sf.net )
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

#include "stdafx.h"
#ifdef _DEBUG
#include "DebugHelpers.h"
#endif

#include <io.h>
#include "SourceList.h"
#include "Preferences.h"
#include "PartFile.h"
#include "SafeFile.h"
#include "Packets.h"
#include "emule.h"
#include "Log.h"
#include "emuledlg.h"
#include "friend.h"
#include "friendlist.h"
#include "UpDownClient.h"
#include "ClientList.h"
#include "Transferwnd.h"
#include "downloadqueue.h"
#include "Neo/Defaults.h"
#include "Neo/NeoOpCodes.h"
#include "Neo/Functions.h"
#include "Neo/NeoPreferences.h"
#include "Neo/ClientFileStatus.h"// NEO: SCFS - [SmartClientFileStatus]
#include "Neo/NeoDebug.h" // NEO: ND - [NeoDebug]

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#if defined(NEO_SA) && !defined(NEO_CD)
#error Neo Source Analyzer needs the Neo Client Database.
#endif

#if defined(NEO_SA) && !defined(NEO_SS)
#error Neo Source Analyzer have no sence without Neo Source Storage.
#endif

#if !defined(NEO_SA) && defined(NEO_CD)
#pragma message ("Neo Source Analyzer is not included, Neo Client Database is near to useles without the Neo Source Analyzer.")
#endif

#if !defined(NEO_SA) && defined(NEO_SS)
#pragma message ("Neo Source Analyzer is not included, Source saver withlut an Intelligent IP Analyzer, is far from smooth working.")
#endif


#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase] -- Xanatos -->

#ifdef _DEBUG_NEO_SS
 #define SS_DEBUG_ONLY(f)      (f)
#else
 #define SS_DEBUG_ONLY(f)      ((void)0)
#endif

void ClearTagList(CArray<CTag*,CTag*>* &taglist){
	if(taglist){
		for (int i = 0; i < taglist->GetSize(); i++)
			delete taglist->GetAt(i);
		taglist->RemoveAll();
		delete taglist;
		taglist = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Source List

#define SOURCES_MET_FILENAME	_T("sources.met")

CSourceList::CSourceList()
{

	m_mapSources.InitHashTable(8191);
	// NEO: SHM - [SourceHashMonitor]
	m_MonitoredSourceList.InitHashTable(2011);
	m_dwLastTrackedCleanUp = ::GetTickCount();
	// NEO: SHM END

	m_nLastSaved = ::GetTickCount();
	m_nLastCleanUp = ::GetTickCount();

	if (NeoPrefs.EnableSourceList())
		LoadList();
}

CSourceList::~CSourceList()
{
	if(NeoPrefs.EnableSourceList())
		SaveList();
	
	RemoveAllMonitoredSource(); // NEO: SHM - [SourceHashMonitor]
	ClearList();
}

void CSourceList::ClearList()
{
	CKnownSource* cur_source;
	CCKey tmpkey(0);
	POSITION pos = m_mapSources.GetStartPosition();
	while (pos){
		m_mapSources.GetNextAssoc(pos, tmpkey, cur_source);
		delete cur_source;
	}
	m_mapSources.RemoveAll();
}

// NEO: SHM - [SourceHashMonitor]
void CSourceList::RemoveAllMonitoredSource()
{
	MonitorSource_Struct* cur_source;
	uint32 tmpkey;
	POSITION pos = m_MonitoredSourceList.GetStartPosition();
	while (pos){
		m_MonitoredSourceList.GetNextAssoc(pos, tmpkey, cur_source);
		delete cur_source;
	}
	m_MonitoredSourceList.RemoveAll();
}
// NEO: SHM END

bool CSourceList::LoadList()
{
	DWORD startMesure = GetTickCount();

	CString strFileName = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + SOURCES_MET_FILENAME;
	const int iOpenFlags = CFile::modeRead|CFile::osSequentialScan|CFile::typeBinary|CFile::shareDenyWrite;
	CSafeBufferedFile file;
	CFileException fexp;
	if (!file.Open(strFileName, iOpenFlags, &fexp)){
		if (fexp.m_cause != CFileException::fileNotFound){
			CString strError(GetResString(IDS_X_ERR_LOADSOURCEFILE));
			TCHAR szError[MAX_CFEXP_ERRORMSG];
			if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
				strError += _T(" - ");
				strError += szError;
			}
			ModLogError(LOG_STATUSBAR, _T("%s"), strError);
			return false;
		}
		return false;
	}
	setvbuf(file.m_pStream, NULL, _IOFBF, 16384);
	
	CKnownSource* newsource = NULL;
	try{
		uint8 version = file.ReadUInt8();
		if (version != SOURCEFILE_VERSION /*version > SOURCEFILE_VERSION || version < SOURCEFILE_VERSION_OLD*/){
			ModLogError(LOG_STATUSBAR, GetResString(IDS_X_ERR_SOURCESMET_UNKNOWN_VERSION));
			file.Close();
			return false;
		}

		UINT count = file.ReadUInt32();
		m_mapSources.InitHashTable(count+5000); // TODO: should be prime number... and 20% larger

		const uint32 dwExpired = time(NULL) - NeoPrefs.GetSourceListExpirationTime();
		uint32 cDeleted = 0;
		for (UINT i = 0; i < count; i++)
		{
			newsource = new CKnownSource(&file);

#if	defined(_DEBUG) && defined(_DEBUG_NEO) 
			if(newsource->m_IPTables.GetCount())
				newsource->AnalyzeIPBehavior(); // NEO: ND - [NeoDebug]
#endif

			if (newsource->GetLastSeen() < dwExpired){
				cDeleted++;
				delete newsource;
				newsource = NULL;
				continue;
			}

			m_mapSources.SetAt(CCKey(newsource->GetKey()), newsource);
			newsource = NULL;
		}
		file.Close();

		if (cDeleted>0)
			ModLog(GetResString(IDS_X_SOURCESFILELOADED) + GetResString(IDS_X_SOURCESEXPIRED), count-cDeleted,cDeleted);
		else
			ModLog(GetResString(IDS_X_SOURCESFILELOADED), count);
	}
	catch(CFileException* error){
		if (error->m_cause == CFileException::endOfFile){
			ModLogError(LOG_STATUSBAR, GetResString(IDS_X_SOURCESFILECORRUPT));
		}else{
			TCHAR buffer[MAX_CFEXP_ERRORMSG];
			error->GetErrorMessage(buffer, ARRSIZE(buffer));
			ModLogError(LOG_STATUSBAR, GetResString(IDS_X_ERR_SOURCESFILEREAD), buffer);
		}
		error->Delete();
		if (newsource)
			delete newsource;
		return false;
	}

	if (thePrefs.GetVerbose())
		ModLog(GetResString(IDS_X_SOURCE_DATABASE_LOADED), GetTickCount()-startMesure, m_mapSources.GetCount());

	return true;
}

void CSourceList::SaveList()
{
	if (thePrefs.GetLogFileSaving())
		AddDebugLogLine(false, _T("Saving source list file \"%s\""), SOURCES_MET_FILENAME);
	m_nLastSaved = ::GetTickCount();

	CString name = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + SOURCES_MET_FILENAME;
	CSafeBufferedFile file;
	CFileException fexp;
	if (!file.Open(name, CFile::modeWrite|CFile::modeCreate|CFile::typeBinary|CFile::shareDenyWrite, &fexp)){
		CString strError(GetResString(IDS_X_ERR_FAILED_SOURCESSAVE));
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		ModLogError(LOG_STATUSBAR, _T("%s"), strError);
		return;
	}
	
	setvbuf(file.m_pStream, NULL, _IOFBF, 16384);

	CKnownSource* cur_source;
	CCKey tempkey(0);
	POSITION pos;

	try{
		
		file.WriteUInt8(SOURCEFILE_VERSION);
		
		uint32 uTagCount = 0;
		ULONG uTagCountFilePos = (ULONG)file.GetPosition();
		file.WriteUInt32(uTagCount);

		const bool bFastCleanUp = NeoPrefs.IsSourceListRunTimeCleanUp() == TRUE;

		pos = m_mapSources.GetStartPosition();
		while (pos)
		{
			m_mapSources.GetNextAssoc(pos, tempkey, cur_source);
			if (!cur_source->IsEmpty())
			{
				if(bFastCleanUp && !SourceHaveFiles(cur_source)) // fast cleanup, drop sources that we don't need immidetly
					continue; // don't save this source
				cur_source->WriteToFile(&file);
				uTagCount++;
			}
		}

		file.Seek(uTagCountFilePos, CFile::begin);
		file.WriteUInt32(uTagCount);
		file.Seek(0, CFile::end);
		
		if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !theApp.emuledlg->IsRunning()))
			file.Flush();
		file.Close();
	}
	catch(CFileException* error){
		CString strError(GetResString(IDS_X_ERR_FAILED_SOURCESSAVE));
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (error->GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		ModLogError(LOG_STATUSBAR, _T("%s"), strError);
		error->Delete();
	}

}

CKnownSource* CSourceList::GetSource(const uchar* key)
{
	CKnownSource* result;
	CCKey tkey(key);
	if (!m_mapSources.Lookup(tkey, result)){
		result = new CKnownSource(key);
		m_mapSources.SetAt(CCKey(result->GetKey()), result);
	}
	return result;
}

/*CKnownSource* CSourceList::MergeSource(CKnownSource* source)
{
	CKnownSource* result;
	CCKey tkey(source->GetKey());
	if (m_mapSources.Lookup(tkey, result)){
		if(result->GetLastSeen() < source->GetLastSeen())
			result->Merge(source);
		delete source;
		return result;
	}else
		m_mapSources.SetAt(CCKey(source->GetKey()), source);
	return source;
}*/

void CSourceList::Process()
{
	const uint32 cur_tick = ::GetTickCount();

	if(NeoPrefs.EnableSourceList()){
		if (cur_tick - m_nLastSaved > MIN2MS(26))
			SaveList();
	}

	if(NeoPrefs.IsSourceListRunTimeCleanUp() 
	 && (cur_tick - m_nLastCleanUp) > (UINT)(NeoPrefs.GetSourceListExpirationTime()/10)){ // ExpirationTime is 10 - 100 days
		// cleanup source data base
		m_nLastCleanUp = cur_tick;
		AddDebugLogLine(false, _T("Cleaning up source database, %i sources on List..."), m_mapSources.GetCount());

		CKnownSource* cur_source;
		CCKey tempkey(0);
		POSITION pos;

		const uint32 dwExpired = time(NULL) - NeoPrefs.GetSourceListExpirationTime();
		const bool bFastCleanUp = NeoPrefs.IsSourceListRunTimeCleanUp() == TRUE;

		pos = m_mapSources.GetStartPosition();
		while (pos)
		{
			m_mapSources.GetNextAssoc(pos, tempkey, cur_source);
			if(cur_source->IsUsed())
				continue;

			if (cur_source->IsEmpty() 
			|| (cur_source->GetLastSeen() < dwExpired)
			|| (bFastCleanUp && !SourceHaveFiles(cur_source)))
			{
				m_mapSources.RemoveKey(tempkey);
				delete cur_source;
			}

		}
	
		AddDebugLogLine(false, _T("...done, %i sources left on list"), m_mapSources.GetCount());
	}
	
	// NEO: SHM - [SourceHashMonitor]
	if(NeoPrefs.UseSourceHashMonitor()){
		// Cleanup tracked client list
		if (m_dwLastTrackedCleanUp + TRACKED_CLEANUP_TIME < cur_tick)
		{
			m_dwLastTrackedCleanUp = cur_tick;
			AddDebugLogLine(false, _T("Cleaning up TrackedSourcesList, %i sources on List..."), m_MonitoredSourceList.GetCount());
			POSITION pos = m_MonitoredSourceList.GetStartPosition();
			uint32 nKey;
			MonitorSource_Struct* pResult;
			while (pos != NULL)
			{
				m_MonitoredSourceList.GetNextAssoc( pos, nKey, pResult );
				if (pResult->m_dwInserted + KEEPTRACK_TIME < cur_tick ){
					m_MonitoredSourceList.RemoveKey(nKey);
					delete pResult;
				}
			}
			AddDebugLogLine(false, _T("...done, %i sources left on list"), m_MonitoredSourceList.GetCount());
		}
	}
	// NEO: SHM END
}

bool CSourceList::IsSourcePtrInList(const CKnownSource* source) const
{
	if(source)
	{
		POSITION pos = m_mapSources.GetStartPosition();
		while (pos)
		{
			CCKey key;
			CKnownSource* cur_source;
			m_mapSources.GetNextAssoc(pos, key, cur_source);
			if (source == cur_source)
				return true;
		}
	}

	return false;
}

bool CSourceList::RemoveSourceFromPtrList(CKnownSource* source)
{
	if(source->IsUsed())
		return false;// Don't delete used sources
	m_mapSources.RemoveKey(CCKey(source->GetUserHash()));
	return true;
}

// NEO: SHM - [SourceHashMonitor]
bool CSourceList::MonitorSource(CUpDownClient *toadd){
	MonitorSource_Struct* pResult = 0;
	if (m_MonitoredSourceList.Lookup(toadd->GetIP(), pResult)){
		pResult->m_dwInserted = ::GetTickCount();
		for (int i = 0; i != pResult->m_ItemsList.GetCount(); i++){
			if(pResult->m_ItemsList[i].nPort == toadd->GetUserPort()){ // port match
				bool bChanged = false;
				if(md4cmp(pResult->m_ItemsList[i].abyUserHash,toadd->GetUserHash())!=FALSE){ // hash mismatch
					md4cpy(pResult->m_ItemsList[i].abyUserHash, toadd->GetUserHash()); // Update hash
					pResult->tLastChange = ::GetTickCount();
					bChanged = true;
				}

				if(pResult->tLastChange + MIN_REQUESTTIME > ::GetTickCount()){
					if(bChanged)
						pResult->uFastChangeCount++;
					if(pResult->uFastChangeCount >= BADCLIENTBAN){
#ifdef ARGOS // NEO: NA - [NeoArgos] 
						if(NeoPrefs.IsHashChangeDetection() == 2){
							if (thePrefs.GetLogBannedClients())
								AddDebugLogLine(false, _T("Clients: %s (%s), Banreason: Userhash changed"), toadd->GetUserName(), ipstr(toadd->GetConnectIP()));
							toadd->Ban(_T("Userhash changed"));
						}
#endif //ARGOS // NEO: NA END
						return false; // Hash changed to fast, dont give an source object to this client
					}
				}else if (!bChanged && pResult->uFastChangeCount)
					pResult->uFastChangeCount--;

				return true; // Hash change Ok or no change happend, client can get his Source Object
			}
		}

		PortHash_Struct PortHash;
		PortHash.nPort = toadd->GetUserPort();
		md4cpy(PortHash.abyUserHash, toadd->GetUserHash());
		pResult->m_ItemsList.Add(PortHash);
		pResult->tLastChange = ::GetTickCount();
	}
	else
	{
		MonitorSource_Struct* pNew = new MonitorSource_Struct;
		pNew->m_dwInserted = ::GetTickCount();
		PortHash_Struct PortHash;
		PortHash.nPort = toadd->GetUserPort();
		md4cpy(PortHash.abyUserHash, toadd->GetUserHash());
		pNew->m_ItemsList.Add(PortHash);
		pNew->tLastChange = ::GetTickCount();
		pNew->uFastChangeCount = 0;
		m_MonitoredSourceList.SetAt(toadd->GetIP(), pNew);
	}

	return true; // New Entry created, client can get his Source Object
}
// NEO: SHM END

/////////////////////////////////////////////////////////////////////////////
// Source Entries
IMPLEMENT_DYNAMIC(CKnownSource, CObject)

CKnownSource::CKnownSource(const uchar* key)
{
	Init();
	md4cpy(m_achUserHash, key);
}

void CKnownSource::Init()
{
	md4clr(m_achUserHash);

	m_dwIPZone = 0;
	m_dwIPLevel = IP_LEVEL_2ND;
	m_bIPZoneInvalid = false;

	m_dwCurrentIP = 0;
	m_nCurrentPort = 0;

	m_TemporaryIPTable = NULL;
	m_CurrentIPTable = NULL;

	m_dwUserIP = 0;
	m_nUserPort = 0;

	m_nUserIDHybrid = 0;
	m_nUDPPort = 0;
	m_nKadPort = 0;

	m_uLastSeen = 0; 

	m_strUserName.Empty();
	m_strClientSoftware.Empty();
	m_strModVersion.Empty();
	m_clientSoft = SO_UNKNOWN;

	m_IPTables.RemoveAll();
	m_SeenFiles.RemoveAll(); // NEO: SFL - [SourceFileList]


#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	m_iAvalibilityProbability = 0;

	m_uStaticIP = 0;

	m_uLongestOnTime = 0;
	m_uMidleOnTime = 0;
	m_uShortestOnTime = 0;
	
	m_uLongestOffTime = 0;
	m_uMidleOffTime = 0;
	m_uShortestOffTime = 0;
	
	m_uLongestIPTime = 0;
	m_uMidleIPTime = 0;
	m_uShortestIPTime = 0;

	m_uLargestFaildCount = 0;
	m_uMidleFaildCount = 0;
	m_uSmallestFaildCount = 0;

	m_uLastSeenDuration = 0;
	m_uTotalSeenDuration = 0;
	m_uLastLinkTime = 0;

	m_uLastAnalisis = 0;
	m_iAnalisisQuality = 0;
	m_bAnalisisNeeded = false;
#endif // NEO_SA // NEO: NSA END

	m_uUsed = 0;
}

CKnownSource::~CKnownSource()
{
	ASSERT(m_uUsed == 0);
	Clear();
	ClearTags(); 
	if(m_TemporaryIPTable)
		delete m_TemporaryIPTable;
}

CKnownSource::CKnownSource(CFileDataIO* file)
{
	Init();

	file->ReadHash16(m_achUserHash);

	UINT tagcount = file->ReadUInt32();
	for (UINT j = 0; j < tagcount; j++){
		CTag* newtag = new CTag(file, false);
		switch(newtag->GetNameID()){
			case SFT_IP_ZONE:{
                ASSERT( newtag->IsInt() );
                m_dwIPZone = newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_IP_ZONE_LEVEL:{
                ASSERT( newtag->IsInt() );
				m_dwIPLevel = newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_IP:{
                ASSERT( newtag->IsInt() );
                m_dwUserIP = newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_PORT:{
                ASSERT( newtag->IsInt() );
                m_nUserPort = (uint16)newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_HYBRID_ID:{
                ASSERT( newtag->IsInt() );
                m_nUserIDHybrid = newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_UDP_PORT:{
                ASSERT( newtag->IsInt() );
                m_nUDPPort = (uint16)newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_KAD_PORT:{
                ASSERT( newtag->IsInt() );
                m_nKadPort = (uint16)newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_LAST_SEEN:{
                ASSERT( newtag->IsInt() );
                m_uLastSeen = newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_USER_NAME:{
                ASSERT( newtag->IsStr() );
				if(newtag->IsStr())
					m_strUserName = newtag->GetStr();
                delete newtag;
                break;
            }

			case SFT_SOFTWARE_VERSION:{
                ASSERT( newtag->IsStr() );
				if(newtag->IsStr())
					m_strClientSoftware = newtag->GetStr();
                delete newtag;
                break;
            }

			case SFT_CLIENT_MODIFICATION:{
                ASSERT( newtag->IsStr() );
				if(newtag->IsStr())
					m_strModVersion = newtag->GetStr();
                delete newtag;
                break;
            }

			case SFT_CLIENT_SOFTWARE:{
                ASSERT( newtag->IsInt() );
                m_clientSoft = (uint8)newtag->GetInt();
                delete newtag;
                break;
            }
#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
			case SFT_STATIC_IP:{
                ASSERT( newtag->IsInt() );
                m_uStaticIP = (uint8)newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_MAX_ON_TIME:{
                ASSERT( newtag->IsInt() );
                m_uLongestOnTime = newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_MID_ON_TIME:{
                ASSERT( newtag->IsInt() );
                m_uMidleOnTime = newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_MIN_ON_TIME:{
                ASSERT( newtag->IsInt() );
                m_uShortestOnTime = newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_MAX_OFF_TIME:{
                ASSERT( newtag->IsInt() );
                m_uLongestOffTime = newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_MID_OFF_TIME:{
                ASSERT( newtag->IsInt() );
                m_uMidleOffTime = newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_MIN_OFF_TIME:{
                ASSERT( newtag->IsInt() );
                m_uShortestOffTime = newtag->GetInt();
                delete newtag;
                break;
            }


			case SFT_MAX_IP_TIME:{
                ASSERT( newtag->IsInt() );
                m_uLongestIPTime = newtag->GetInt();
                delete newtag;
                break;
            }
	
			case SFT_MID_IP_TIME:{
                ASSERT( newtag->IsInt() );
                m_uMidleIPTime = newtag->GetInt();
                delete newtag;
                break;
            }
	
			case SFT_MIN_IP_TIME:{
                ASSERT( newtag->IsInt() );
                m_uShortestIPTime = newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_MAX_FAILD_COUNT:{
                ASSERT( newtag->IsInt() );
                m_uLargestFaildCount = newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_MID_FAILD_COUNT:{
                ASSERT( newtag->IsInt() );
                m_uMidleFaildCount = newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_MIN_FAILD_COUNT:{
                ASSERT( newtag->IsInt() );
                m_uSmallestFaildCount = newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_LAST_SEEN_DURATION:{
                ASSERT( newtag->IsInt() );
                m_uLastSeenDuration = newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_TOTAL_SEEN_DURATION:{
                ASSERT( newtag->IsInt() );
                m_uTotalSeenDuration = newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_LAST_LINK_TIME:{
                ASSERT( newtag->IsInt() );
				m_uLastLinkTime = newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_LAST_ANALYSIS:{
                ASSERT( newtag->IsInt() );
                m_uLastAnalisis = newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_ANALYSIS_QUALITY:{
                ASSERT( newtag->IsInt() );
                m_iAnalisisQuality = newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_ANALYSIS_NEEDED:{
                ASSERT( newtag->IsInt() );
				m_bAnalisisNeeded = I2B(newtag->GetInt());
                delete newtag;
                break;
            }
#endif // NEO_SA // NEO: NSA END

			default:{
				taglist.Add(newtag);
			}
		}
	}

	LoadIPTables(file);

	LoadSeenFiles(file); // NEO: SFL - [SourceFileList]
}

void CKnownSource::WriteToFile(CFileDataIO* file)
{

	if(NeoPrefs.UseIPZoneCheck() && m_IPTables.GetCount() > (NeoPrefs.GetTableAmountToStore() / 10)) // 10% is enough by default thats 5 tables
		ReConsiderIPZone(); // Reconsider the IPZone

#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	if(NeoPrefs.EnableSourceAnalizer() && m_bAnalisisNeeded)
		AnalyzeIPBehavior();
#endif // NEO_SA // NEO: NSA END

	file->WriteHash16(m_achUserHash);

	uint32 uTagCount = 0;
	ULONG uTagCountFilePos = (ULONG)file->GetPosition();
	file->WriteUInt32(uTagCount);

	if (m_dwIPZone) {
		CTag(SFT_IP_ZONE, m_dwIPZone).WriteTagToFile(file);
		uTagCount++;
	}

	if (m_dwIPLevel != IP_LEVEL_2ND) {
		CTag(SFT_IP_ZONE_LEVEL, m_dwIPLevel).WriteTagToFile(file);
		uTagCount++;
	}

	if (m_dwUserIP) {
		CTag(SFT_IP, m_dwUserIP).WriteTagToFile(file);
		uTagCount++;
	}

	if (m_nUserPort) {
		CTag(SFT_PORT, m_nUserPort).WriteTagToFile(file);
		uTagCount++;
	}

	if (m_nUserIDHybrid) {
		CTag(SFT_HYBRID_ID, m_nUserIDHybrid).WriteTagToFile(file);
		uTagCount++;
	}

	if (m_nUDPPort) {
		CTag(SFT_UDP_PORT, m_nUDPPort).WriteTagToFile(file);
		uTagCount++;
	}

	if (m_nKadPort) {
		CTag(SFT_KAD_PORT, m_nKadPort).WriteTagToFile(file);
		uTagCount++;
	}

	if (m_uLastSeen) {
		CTag(SFT_LAST_SEEN, m_uLastSeen).WriteTagToFile(file);
		uTagCount++;
	}

	if (!m_strUserName.IsEmpty()){
		CTag(SFT_USER_NAME, m_strUserName).WriteTagToFile(file);
		uTagCount++;
	}

	if (!m_strClientSoftware.IsEmpty()){
		CTag(SFT_SOFTWARE_VERSION, m_strClientSoftware).WriteTagToFile(file);
		uTagCount++;
	}

	if (!m_strModVersion.IsEmpty()){
		CTag(SFT_CLIENT_MODIFICATION, m_strModVersion).WriteTagToFile(file);
		uTagCount++;
	}

	if (m_clientSoft != SO_UNKNOWN){
		CTag(SFT_CLIENT_SOFTWARE, m_clientSoft).WriteTagToFile(file);
		uTagCount++;
	}

#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	if (m_uStaticIP){
		CTag(SFT_STATIC_IP, m_uStaticIP).WriteTagToFile(file);
		uTagCount++;
	}

	if (m_uLongestOnTime){
		CTag(SFT_MAX_ON_TIME, m_uLongestOnTime).WriteTagToFile(file);
		uTagCount++;
	}

	if (m_uMidleOnTime){
		CTag(SFT_MID_ON_TIME, m_uMidleOnTime).WriteTagToFile(file);
		uTagCount++;
	}

	if (m_uShortestOnTime){
		CTag(SFT_MIN_ON_TIME, m_uShortestOnTime).WriteTagToFile(file);
		uTagCount++;
	}

	if (m_uLongestOffTime){
		CTag(SFT_MAX_OFF_TIME, m_uLongestOffTime).WriteTagToFile(file);
		uTagCount++;
	}

	if (m_uMidleOffTime){
		CTag(SFT_MID_OFF_TIME, m_uMidleOffTime).WriteTagToFile(file);
		uTagCount++;
	}

	if (m_uShortestOffTime){
		CTag(SFT_MIN_OFF_TIME, m_uShortestOffTime).WriteTagToFile(file);
		uTagCount++;
	}
	
	if (m_uLongestIPTime){
		CTag(SFT_MAX_IP_TIME, m_uLongestIPTime).WriteTagToFile(file);
		uTagCount++;
	}

	if (m_uMidleIPTime){
		CTag(SFT_MID_IP_TIME, m_uMidleIPTime).WriteTagToFile(file);
		uTagCount++;
	}

	if (m_uShortestIPTime){
		CTag(SFT_MIN_IP_TIME, m_uShortestIPTime).WriteTagToFile(file);
		uTagCount++;
	}

	if (m_uLargestFaildCount){
		CTag(SFT_MAX_FAILD_COUNT, m_uLargestFaildCount).WriteTagToFile(file);
		uTagCount++;
	}

	if (m_uMidleFaildCount){
		CTag(SFT_MID_FAILD_COUNT, m_uMidleFaildCount).WriteTagToFile(file);
		uTagCount++;
	}

	if (m_uSmallestFaildCount){
		CTag(SFT_MIN_FAILD_COUNT, m_uSmallestFaildCount).WriteTagToFile(file);
		uTagCount++;
	}

	if (m_uLastSeenDuration) {
		CTag(SFT_LAST_SEEN_DURATION, m_uLastSeenDuration).WriteTagToFile(file);
		uTagCount++;
	}

	if (m_uTotalSeenDuration) {
		CTag(SFT_TOTAL_SEEN_DURATION, m_uTotalSeenDuration).WriteTagToFile(file);
		uTagCount++;
	}

	if (m_uLastLinkTime) {
		CTag(SFT_LAST_LINK_TIME, m_uLastLinkTime).WriteTagToFile(file);
		uTagCount++;
	}

	if (m_uLastAnalisis){
		CTag(SFT_LAST_ANALYSIS, m_uLastAnalisis).WriteTagToFile(file);
		uTagCount++;
	}

	if (m_iAnalisisQuality){
		CTag(SFT_ANALYSIS_QUALITY, m_iAnalisisQuality).WriteTagToFile(file);
		uTagCount++;
	}

	if (m_bAnalisisNeeded){
		CTag(SFT_ANALYSIS_NEEDED, m_bAnalisisNeeded).WriteTagToFile(file);
		uTagCount++;
	}
#endif // NEO_SA // NEO: NSA END

	// unidentified tags(from mods/future versions)
	for (int j = 0; j < taglist.GetCount(); j++) {
		taglist[j]->WriteTagToFile(file);
		uTagCount++;
	}

	file->Seek(uTagCountFilePos, CFile::begin);
	file->WriteUInt32(uTagCount);
	file->Seek(0, CFile::end);

	SaveIPTables(file);

	SaveSeenFiles(file); // NEO: SFL - [SourceFileList]
}

void CKnownSource::Clear()
{
	IPTableStruct* cur_iptable;
	while (!m_IPTables.IsEmpty()){
		cur_iptable = m_IPTables.RemoveHead();
		ClearTagList(cur_iptable->taglist);
		delete cur_iptable;
	}
	m_IPTables.RemoveAll();

	// NEO: SFL - [SourceFileList]
	SeenFileStruct* cur_seenfile;
	CCKey tmpkey(0);
	POSITION pos = m_SeenFiles.GetStartPosition();
	while (pos){
		m_SeenFiles.GetNextAssoc(pos, tmpkey, cur_seenfile);
		ClearTagList(cur_seenfile->taglist);
		delete cur_seenfile;
	}
	m_SeenFiles.RemoveAll();
	// NEO: SFL END
}

void CKnownSource::ClearTags()
{
	for (int i = 0; i < taglist.GetSize(); i++)
		delete taglist[i];
	taglist.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
// IP Tables

bool CKnownSource::LoadIPTables(CFileDataIO* file){
	static IPTableStruct* newIPTable = NULL; // in the case of a file exception this will prevent a memlake.
	if(newIPTable)
		delete newIPTable;

	UINT RecordsNumber = file->ReadUInt32();
	for (UINT i = 0; i < RecordsNumber; i++) {
		newIPTable = new IPTableStruct;
		memset(newIPTable,0,sizeof(IPTableStruct));

		UINT tagcount = file->ReadUInt32();
		for (UINT j = 0; j < tagcount; j++) {
			CTag* newtag = new CTag(file, false);
			switch(newtag->GetNameID()){
				case IFT_IP:{
					newIPTable->uIP = newtag->GetInt();
					delete newtag;
					break;
				}
				case IFT_PORT:{
					newIPTable->uPort = (uint16)newtag->GetInt();
					delete newtag;
					break;
				}
				case IFT_FIRSTSEEN:{
					newIPTable->tFirstSeen = newtag->GetInt();
					delete newtag;
					break;
				}
				case IFT_LASTSEEN:{
					newIPTable->tLastSeen = newtag->GetInt();
					delete newtag;
					break;
				}
				case IFT_UNREACHABLE:{
					newIPTable->uUnreachable = (uint8)newtag->GetInt();
					delete newtag;
					break;
				}
				default:{
					if(!newIPTable->taglist)
						newIPTable->taglist = new CArray<CTag*,CTag*>;
					newIPTable->taglist->Add(newtag);
				}
			}
		}

		m_IPTables.AddTail(newIPTable);
		newIPTable = NULL;
	}

	return true;
}

void CKnownSource::SaveIPTables(CFileDataIO* file){
	uint32 RecordsNumber = 0;
	ULONG RecordsNumberFilePos = (ULONG)file->GetPosition();
	file->WriteUInt32(RecordsNumber);

	uint32 uTagCount;
	ULONG uTagCountFilePos;

	POSITION pos;
	if(m_IPTables.GetCount() > NeoPrefs.GetTableAmountToStore())
		pos = m_IPTables.FindIndex(m_IPTables.GetCount() - NeoPrefs.GetTableAmountToStore()); // find the index to save
	else
		pos = m_IPTables.GetHeadPosition();
	for (;pos != NULL;)
	{
		IPTableStruct* curIPTable = m_IPTables.GetNext(pos);

		uTagCount = 0;
		uTagCountFilePos = (ULONG)file->GetPosition();
		file->WriteUInt32(uTagCount);

		CTag(IFT_IP, curIPTable->uIP).WriteTagToFile(file); uTagCount++;
		CTag(IFT_PORT, curIPTable->uPort).WriteTagToFile(file); uTagCount++;
		CTag(IFT_FIRSTSEEN, curIPTable->tFirstSeen).WriteTagToFile(file); uTagCount++;
		CTag(IFT_LASTSEEN, curIPTable->tLastSeen).WriteTagToFile(file); uTagCount++;
		if(curIPTable->uUnreachable){
			CTag(IFT_UNREACHABLE, curIPTable->uUnreachable).WriteTagToFile(file); 
			uTagCount++;
		}

		// unidentified tags(from mods/future versions)
		if(curIPTable->taglist)
			for (int j = 0; j < curIPTable->taglist->GetCount(); j++) {
				curIPTable->taglist->GetAt(j)->WriteTagToFile(file);
				uTagCount++;
			}

		file->Seek(uTagCountFilePos, CFile::begin);
		file->WriteUInt32(uTagCount);
		file->Seek(0, CFile::end);

		RecordsNumber ++;
	}

	file->Seek(RecordsNumberFilePos, CFile::begin);
	file->WriteUInt32(RecordsNumber);
	file->Seek(0, CFile::end);
}

void CKnownSource::Attach(CUpDownClient* owner) // Merge all needed informations from the CUpDownClient
{

	m_dwCurrentIP = owner->GetIP(); //GetConnectIP()
	m_nCurrentPort = owner->GetUserPort();

	if(NeoPrefs.UseIPZoneCheck())
		if(m_dwIPZone == 0){ // If this it a new Source the zone is empty = 0
			CreateIPZone(m_dwCurrentIP,IP_LEVEL_2ND); // Setup the IP zone for the new source
			SS_DEBUG_ONLY(DebugLog(_T("New IP Zone for User: %s, IP %s, assigned Zone: %s"), owner->GetUserName() ? owner->GetUserName() : _T("?"), ipstr(m_dwCurrentIP), ipstr(m_dwIPZone)));
		}
		else if(!CheckIPZone(m_dwCurrentIP)){ // If we kew already a source with this hash, check the IP zone
			SS_DEBUG_ONLY(DebugLog(_T("IP Zone violation User: %s, IP %s, assigned Zone: %s"), owner->GetUserName() ? owner->GetUserName() : _T("?"), ipstr(m_dwCurrentIP), ipstr(m_dwIPZone)));
			m_bIPZoneInvalid = true; // The IP zone is wrong propobly bad client
			return;
		}
	m_bIPZoneInvalid = false; // IpZone Valid or new

	// All OK, this is the proper client, so update the datas...
	m_dwUserIP = owner->GetIP(); //GetConnectIP()
	m_nUserPort = owner->GetUserPort();

	m_nUserIDHybrid = owner->GetUserIDHybrid();
	m_nUDPPort = owner->GetUDPPort();
	m_nKadPort = owner->GetKadPort();

	if(GetUserName())
		m_strUserName = owner->GetUserName();
	m_strClientSoftware = owner->GetClientSoftVer();
	m_strModVersion = owner->GetClientModVer();
	m_clientSoft = (uint8)owner->GetClientSoft();
}

void CKnownSource::Detach(CUpDownClient* owner){
	if(m_CurrentIPTable){
		if(m_CurrentIPTable->uIP == owner->GetIP()){
			m_CurrentIPTable = NULL;
			return;
		}
	}

	if(m_TemporaryIPTable){
		if(m_TemporaryIPTable->uIP == owner->GetIP()){
			if(m_CurrentIPTable && m_CurrentIPTable->tLastSeen > m_TemporaryIPTable->tFirstSeen){ // Is it for sure the wrong clinet
				delete m_TemporaryIPTable; // when yes then we dont have use for this data, delete them
			}else{
				m_IPTables.AddTail(m_TemporaryIPTable); // when no we keep them just in case
#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
				m_bAnalisisNeeded = true;
#endif // NEO_SA // NEO: NSA END
			}
			m_TemporaryIPTable = NULL;
		}
	}
}

// In some cases the concept pf the IP zone may fail
// so the user have still a chance to veryfie itself via SUI
void CKnownSource::SUIPassed(CUpDownClient* owner){
	if(m_bIPZoneInvalid){ // Only needed when the IP zone fails
  		SS_DEBUG_ONLY(DebugLog(_T("User: %s, IP %s Validated Zone Change by SUI."), owner->GetUserName() ? owner->GetUserName() : _T("?"), ipstr(m_dwCurrentIP)));
		ReConsiderIPZone(m_dwCurrentIP);
		m_bIPZoneInvalid = false; // The new IP zone is now valid
		Attach(owner); // Update the datas
		if(m_TemporaryIPTable){ // Validate the current IP Table
			m_CurrentIPTable = m_TemporaryIPTable;
			m_TemporaryIPTable = NULL;
			m_IPTables.AddTail(m_CurrentIPTable);
#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
			m_bAnalisisNeeded = true;
#endif // NEO_SA // NEO: NSA END
		}
	}
}

// When the default IZone fails we extend the zone, 
// but when the first ip part dont match we create an compleetly new zone
void CKnownSource::ReConsiderIPZone(uint32 dwIP, bool bAllowReset)
{
	ASSERT(m_dwIPZone && dwIP); // An old IPZone must be known, and the IP must be valid

	// Extend the zone until it becomes valid
	if(CheckIPZone(dwIP, IP_LEVEL_1ST)){ // Statis IP -> Small Subnet
		CreateIPZone(dwIP,IP_LEVEL_1ST);
	}else if(CheckIPZone(dwIP, IP_LEVEL_2ND)){ // Statis IP -> Normal ISP
		CreateIPZone(dwIP,IP_LEVEL_2ND);
	}else if(CheckIPZone(dwIP, IP_LEVEL_3RD)){ // Normal ISP, default -> Big ISP
		CreateIPZone(dwIP,IP_LEVEL_3RD);
	}else if(bAllowReset){ // All zones faild, reset IP Zone
		CreateIPZone(dwIP,IP_LEVEL_2ND);
	}
}

// When we have already some clean IP tabeles, we will reconsider the given zone
// We will find the smallest valid zone, or even set the Zone to an static IP
void CKnownSource::ReConsiderIPZone()
{
	uint32 Masks[4] = {0xFF000000,0x00FF0000,0x0000FF00,0x000000FF};
	CMap<uint8,uint8,uint16,uint16> ZoneMap[4];
	uint16 Count[4] = {0,0,0};
	uint8 Zone[4] = {0,0,0};

	POSITION pos = m_IPTables.GetHeadPosition();
	for (;pos != NULL;){
		uint32 uIP = m_IPTables.GetNext(pos)->uIP;
		for(int i = 1;i<4;i++){ // currently no use for 0 
			uint8 zone = (uint8)((uIP & Masks[i])>>(8*(3-i)));
			uint16 &count = ZoneMap[i][zone];
			count++;
			if(count > Count[i]){
				Zone[i] = zone;
				Count[i] = count;
			}
		}
	}

	if(Count[3] < m_IPTables.GetCount()/2) // Even the first IP segment is not usable
		return; // let the zone unchanged
	else{ // datas are clear enough for a Level 3 zone
		m_dwIPZone = Zone[3];
		m_dwIPLevel = IP_LEVEL_3RD;
	}

	if(Count[2] > m_IPTables.GetCount()/2){ // datas are clear enough for a Level 2 zone
		m_dwIPZone |= Zone[2]<<8;
		m_dwIPLevel = IP_LEVEL_2ND;
	}else
		return;

	if(Count[1] > m_IPTables.GetCount()/2 && m_IPTables.GetCount() > (NeoPrefs.GetTableAmountToStore() / 2)){ // datas are clear enough for a Level 1 zone
		m_dwIPZone |= Zone[1]<<16;
		m_dwIPLevel = IP_LEVEL_1ST;
	}else
		return;

	if(GetIPType() == IP_Static && GetAnalisisQuality() == 10){ // To make this step we need a top quality of our analysis
		m_dwIPZone = m_dwUserIP;
		m_dwIPLevel = IP_LEVEL_ZERO;
	}
}

/*void CKnownSource::CleanUpIPTables()
{
	uint32 uToDrop = 0;
	if(m_IPTables.GetCount() > NeoPrefs.GetTableAmountToStore())
		uToDrop = m_IPTables.GetCount() - NeoPrefs.GetTableAmountToStore();
	IPTableStruct* curIPTable;
	POSITION pos = m_IPTables.GetHeadPosition();
	for (;pos != NULL;){
		POSITION toRemove = pos;
		curIPTable =  m_IPTables.GetNext(pos);
		if(uToDrop){
			uToDrop--;
			m_IPTables.RemoveAt(toRemove);
			delete curIPTable;
		}else if(!CheckIPZone(curIPTable->uIP)){
			m_IPTables.RemoveAt(toRemove);
			delete curIPTable;
		}
	}
}*/

// Create a new current ip table, and if the IP zone is ok, validate it immidetly
IPTableStruct* CKnownSource::CreateNewIPTable()
{
	IPTableStruct* NewIPTable = new IPTableStruct;
	memset(NewIPTable,0,sizeof(IPTableStruct));

	NewIPTable->uIP = m_dwCurrentIP;
	NewIPTable->uPort = m_nCurrentPort;

	NewIPTable->tFirstSeen = time(NULL);
	NewIPTable->tLastSeen = time(NULL);

	NewIPTable->uUnreachable = FALSE;

	return NewIPTable;
}

// Handle the IP table
bool CKnownSource::HandleIPTable(IPTableStruct* IPTable){ 
	if(!IPTable) // Do we have an valid table
		return true;

	// OK we have an table
	if(IPTable->uIP == m_dwCurrentIP){ // If the IP is still the same
		if(IPTable->uUnreachable){ // The source was gone but it's back, its propobly an periodical statis Source
			if(time(NULL) - IPTable->tLastSeen > (uint32)NeoPrefs.GetIgnoreUnreachableInterval()){ // To much time pased, the source was propobly offline
				return true; // create a new table for the new session
			}else{ // Not so much time passed, the source was propobly permanent online
				IPTable->uUnreachable = FALSE; // Cancel the Unreaable flag
			}
		}
		if(time(NULL) - IPTable->tLastSeen > (uint32)NeoPrefs.GetIgnoreUndefinedIntervall()){ // To much time pased, begin a new table 
			return true;
		}else{
			IPTable->tLastSeen = time(NULL); // Update the Last Seen time to now
			return false;
		}
	}
	
	// the IP changed
	return true;
}

// A Connection was asteblisched to the client
void CKnownSource::ConnectionSuccess()
{ 
	if(m_bIPZoneInvalid){ // The IP zone is Bad, but we store and handle it, the client may leter validate him self via SUI
		if(HandleIPTable(m_TemporaryIPTable)){
			if(m_TemporaryIPTable){ // If its a temporary table
				if(m_CurrentIPTable && m_CurrentIPTable->tLastSeen > m_TemporaryIPTable->tFirstSeen){ // Is it for sure the wrong clinet
					delete m_TemporaryIPTable; // when yes then we dont have use for this data, delete them
				}else{
					m_IPTables.AddTail(m_TemporaryIPTable); // when no we keep them just in case
#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
					m_bAnalisisNeeded = true;
#endif // NEO_SA // NEO: NSA END
				}
			}
			m_TemporaryIPTable = CreateNewIPTable(); // and create a new one
		}
	}
	else{ // If the IP Zone is OK
		SetLastSeen(); // Set last seen time to Now
		if(!m_CurrentIPTable && m_IPTables.GetCount()){
			IPTableStruct* CurrentIPTable = m_IPTables.GetTail(); // get the last table
			if(CurrentIPTable->uIP == m_dwUserIP 
			 && time(NULL) - CurrentIPTable->tLastSeen < (uint32)NeoPrefs.GetIgnoreUndefinedIntervall()) // Not to much time pased
				m_CurrentIPTable = CurrentIPTable; // take the last table as cur table
		}
		if(HandleIPTable(m_CurrentIPTable)){
			m_CurrentIPTable = CreateNewIPTable(); // create a new table
			m_IPTables.AddTail(m_CurrentIPTable);
#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
			m_bAnalisisNeeded = true;
#endif // NEO_SA // NEO: NSA END
		}
#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
		else
			m_bAnalisisNeeded = true;
#endif // NEO_SA // NEO: NSA END
	}
}

void CKnownSource::ConnectionFaild()
{ 
	if(m_bIPZoneInvalid){ // The IP zone is Bad, but we store and handle it, the client may leter validate him self via SUI
		if(m_TemporaryIPTable)
			m_TemporaryIPTable->uUnreachable = 1;
	}
	else{ // If the IP Zone is OK
		if(m_CurrentIPTable){ // do we have an current table
			m_CurrentIPTable->uUnreachable = 1;
		}else{ // if we dont have a current table, then its a loaded source
			if(m_IPTables.GetCount()){
				IPTableStruct* CurrentIPTable = m_IPTables.GetTail(); // get the last table
				if(CurrentIPTable->uIP == m_dwUserIP) // the last IP must be the same as current
					CurrentIPTable->uUnreachable = 1; // fix up the table
			}
		}
	}
}

/*void CKnownSource::Merge(CKnownSource* tomerge)
{
	m_dwUserIP = tomerge->GetIP();
	m_nUserPort = tomerge->GetUserPort();

	m_nUserIDHybrid = tomerge->GetUserIDHybrid();
	m_nUDPPort = tomerge->GetUDPPort();
	m_nKadPort = tomerge->GetKadPort();

	m_strUserName = tomerge->GetUserName();
	m_strClientSoftware = tomerge->GetClientSoftVer();
	m_strModVersion = tomerge->GetClientModVer();
	m_clientSoft = (uint8)tomerge->GetClientSoft();

	while (!m_IPTables.IsEmpty()){
		IPTableStruct* tmpIPTable = m_IPTables.RemoveHead();
		if(tmpIPTable != m_CurrentIPTable){
			ClearTagList(tmpIPTable->taglist);
			delete tmpIPTable;
		}
	}
	m_IPTables.RemoveAll();
	for (POSITION pos = tomerge->m_IPTables.GetHeadPosition(); pos != NULL;){
		IPTableStruct* curIPTable = tomerge->m_IPTables.GetNext(pos);
		IPTableStruct* tmpIPTable = new IPTableStruct;
		memcpy(tmpIPTable,curIPTable,sizeof(IPTableStruct));
		curIPTable->taglist = NULL; // Move tag list to the New Structure
		m_IPTables.AddTail(tmpIPTable);
	}
	if(m_CurrentIPTable)
		m_IPTables.AddTail(m_CurrentIPTable);
#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	m_bAnalisisNeeded = true;
#endif // NEO_SA // NEO: NSA END

	// NEO: SFL - [SourceFileList]
	//SeenFileStruct* curSeenFile;
	//SeenFileStruct* foundSeenFile;
	//CCKey tempkey(0);
	//POSITION pos2 = tomerge->m_SeenFiles.GetStartPosition();
	//while (pos2)
	//{
	//	tomerge->m_SeenFiles.GetNextAssoc(pos2, tempkey, curSeenFile);
	//
	//	CCKey tkey(curSeenFile->abyFileHash);
	//	if (!m_SeenFiles.Lookup(tkey, foundSeenFile)){
	//		foundSeenFile = new SeenFileStruct;
	//		memcpy(foundSeenFile,curSeenFile,sizeof(SeenFileStruct));
	//		curSeenFile->taglist = NULL; // Move tag list to the New Structure
	//		m_SeenFiles.SetAt(CCKey(curSeenFile->abyFileHash), foundSeenFile);
	//	}else{
	//		//foundSeenFile->uFileSize = curSeenFile->uFileSize;
	//		foundSeenFile->tLastSeen = curSeenFile->tLastSeen;
	//	}
	//}
	// NEO: SFL END
}*/

/////////////////////////////////////////////////////////////////////////////
// Neo Source IP Analyzer

#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
//#pragma pack(1)
struct IPRepeatInfoStruct{
	uint16		uRepeat;
	uint16		uTotal;

	uint32		tTotalSeen;
};
//#pragma pack()

//#pragma pack(1)
struct MetaIPTableStruct{
	uint32		uIP; // the IP os the use
	uint16		uPort; // his Port

	uint32		tFirstSeen; // time we sow the IP the first time
	uint32		tLastSeen; // time we sow the IP the last time

	uint32		SeenOnLineTime; // time we saw the user for sure online
	uint32		OnLineTime; // time the user was propobly online
	uint32		OffLineTime; // time the user was propobly offline
	//uint32		AvalibilityTime; // time how long the IP was avalibly // OffLineTime + OnLineTime

	uint16		uFaildCount; // How often a connection try on a IP failed, but the source was leter found under this ip gain
	uint16		uLostCount; // How often a connection try on a IP failed, and the the IP changed
};
//#pragma pack()

//#pragma pack(1)
struct MidleStruct{
	uint32 Maximal; // Max Value
	uint32 Midle; // Midle Value
	uint32 Minimal; // Min Value

	uint32 Total; // Total summ
	UINT   Count; // Counter

	uint32 Supremum; // Max Limit
	uint32 Infimum; // Min limit

	uint32 Major; // Max known value
	uint32 Minor; // Min known value
	uint8  LimesOverwrite; // To Restrictiv Limes, increase it
};
//#pragma pack()

#define INIT_MIDLE_LIMES(Struct) \
	Struct.Supremum = 0xFFFFFFFF; \
	Struct.Infimum = 0; \
	Struct.Major = 0; \
	Struct.Minor = 0xFFFFFFFF; \
	Struct.LimesOverwrite = 0; \

#define INIT_MIDLE(Struct) \
	Struct.Maximal = 0; \
	Struct.Midle = 0; \
	Struct.Minimal = 0xFFFFFFFF; \
	Struct.Total = 0; \
	Struct.Count = 0; \

__inline void MidleAndLimes(MidleStruct &MidleValue, bool &IsClean, const float &MaxMidleHigh, const float &LimMidleHigh, const float &MaxMidleLow, const float &LimMidleLow)
{
	if(MidleValue.Count)
		MidleValue.Midle = MidleValue.Total/MidleValue.Count; 

	// This part is an fail safe procedure on the case the MaxMidleDiscrepance, is seted to restrictiv for the current datas
	if(MidleValue.Total == 0 && MidleValue.Major != 0)
	{
		if(MidleValue.LimesOverwrite == 2){
			ASSERT(0);
			return;
		}

		MidleValue.LimesOverwrite = 1;
		if(MidleValue.Supremum < 0x7FFFFFFF)
			++MidleValue.Supremum =(uint32)(MidleValue.Supremum*(1.0F + (1.0F/MaxMidleHigh)));
		if(MidleValue.Infimum > 0)
			MidleValue.Infimum = (uint32)(MidleValue.Infimum/(1.0F + (1.0F/MaxMidleLow)));
		ASSERT(MidleValue.Supremum>MidleValue.Infimum);
		if(MidleValue.Supremum >= MidleValue.Major && MidleValue.Infimum <= MidleValue.Minor) 
			MidleValue.LimesOverwrite = 2;
		IsClean = false;
		return;
	}

	ASSERT(MidleValue.Maximal>=MidleValue.Minimal);

	if(MidleValue.Major == 0 || MidleValue.LimesOverwrite)
		return;

	if((uint32)(MidleValue.Maximal/LimMidleHigh) > MidleValue.Midle){
		if(MidleValue.Supremum > 0x7FFFFFFF && MidleValue.Midle*MaxMidleHigh > 0x7FFFFFFF) // Fail Safe
			return;
		MidleValue.Supremum = (uint32)(MidleValue.Midle*MaxMidleHigh);
		IsClean = false;
	}
	if(MidleValue.Minimal < (uint32)(MidleValue.Midle/LimMidleLow)){
		if(MidleValue.Infimum < 1 && MidleValue.Midle/MaxMidleLow < 1) // Fail Safe
			return;
		MidleValue.Infimum = (uint32)(MidleValue.Midle/MaxMidleLow);
		IsClean = false;
	}
}

__inline void HandleMidle(MidleStruct &MidleValue, uint32 Value, UINT Gravity = 1){
	if(Value > MidleValue.Major)
		MidleValue.Major = Value;
	if(Value < MidleValue.Minor)
		MidleValue.Minor = Value;

	if(Value > MidleValue.Maximal && Value <= MidleValue.Supremum)
		MidleValue.Maximal = Value;
	if(Value < MidleValue.Minimal && Value >= MidleValue.Infimum)
		MidleValue.Minimal = Value;
	if(Value >= MidleValue.Infimum && Value <= MidleValue.Supremum){
		if(Value){ // filter the 0 results, othrewice this loop will be repeared at least 1, without sence
			MidleValue.Total += Value * Gravity;
			MidleValue.Count += Gravity;
		}
	}
}

#ifdef _DEBUG_NEO // NEO: ND - [NeoDebug]
int CKnownSource::ExceptionFilter(unsigned int code, struct _EXCEPTION_POINTERS *ep) {

	CString Message;
	Message.Format(_T("Ip Analyser Procedure Coased an Exception!\n"));
	try{
		CString DumpPath;
		DumpPath = DumpTable();
		Message.AppendFormat(_T("The Source Entry that Coased this had been stored in:\n%s\n"),DumpPath);
	}catch(...){
		Message.AppendFormat(_T("Unable to store the Source Entry that Coased this!!!\n"));
	}
	Message.Append(_T("Additional Informations:\n"));
	Message.Append(InterpreteErrorCode(code) + _T("\n"));
	Message.Append(InterpreteErrorInfo(ep) + _T("\n"));
	Message.Append(_T("\nYes - Raise This Exception\nNo - Catch This Exception\nCancel - Continue if possible"));
	int ret = MessageBox(NULL,Message,_T("Exception !!!"),MB_YESNOCANCEL | MB_ICONEXCLAMATION); 

	if (ret == IDNO) 
		return EXCEPTION_EXECUTE_HANDLER;
	else if (ret == IDYES) 
		return EXCEPTION_CONTINUE_SEARCH;
	else /*if (ret == IDCANCEL)*/
		return EXCEPTION_CONTINUE_EXECUTION;
}

CString CKnownSource::DumpTable(){
	CString name = NeoPrefs.GetConfigDir();
	name.AppendFormat(_T("BadTable%u.met"),time(NULL));
	CSafeBufferedFile file;
	CFileException fexp;
	if (!file.Open(name, CFile::modeWrite|CFile::modeCreate|CFile::typeBinary|CFile::shareDenyWrite, &fexp)){
		CString strError(GetResString(IDS_ERR_FAILED_SOURCESSAVE));
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		ModLogError(LOG_STATUSBAR, _T("%s"), strError);
		return _T("Fille Acces Error");
	}
	
	setvbuf(file.m_pStream, NULL, _IOFBF, 16384);

	file.WriteUInt8(SOURCEFILE_VERSION);
	file.WriteUInt32(1);

	WriteToFile(&file);

	if (NeoPrefs.GetCommitFiles() >= 2 || (NeoPrefs.GetCommitFiles() >= 1 && !theApp.emuledlg->IsRunning()))
		file.Flush();
	file.Close();

	return name;
}

void CKnownSource::AnalyzeIPBehavior(){
  __try{
   //__try{
	AnalyzeIPBehaviorProc();
   //}__finally{
	//if(AbnormalTermination())
   //}
  }__except(ExceptionFilter(GetExceptionCode(), GetExceptionInformation())){
	
  }
}

void CKnownSource::AnalyzeIPBehaviorProc()
#else
void CKnownSource::AnalyzeIPBehavior()
#endif // _DEBUG_NEO // NEO: ND END
{
	SS_DEBUG_ONLY(DebugLog(GetResString(IDS_X_ANALYSING_SOURCE), GetUserName(), ipstr(GetIP()),GetUserPort()));

	m_uLastAnalisis = time(NULL);
	m_bAnalisisNeeded = false;

	ASSERT(m_IPTables.GetCount()); // Is should be at least one entry
	if(m_IPTables.GetCount() == 0)
		return;

	// first step we create a temp tabele
	// and count possible gaps

	CList<IPTableStruct*,IPTableStruct*> TempIPTables; // Our temp tabele
	IPTableStruct* curTempIPTable = NULL;

	CMap<uint32,uint32,IPRepeatInfoStruct*,IPRepeatInfoStruct*> IPRepeatingTable; // We will find bad gap's
	IPRepeatInfoStruct* IPRepeatInfo;

	uint32 ToAnalyze = NeoPrefs.GetTableAmountToAnalyze(); // do not Analyze the whole list, some last tables are enough
	bool FilterIPZone = NeoPrefs.UseIPZoneCheck();

	POSITION pos = m_IPTables.GetTailPosition();
	//POSITION pos = m_IPTables.GetHeadPosition();
	for (;pos != NULL && ToAnalyze;){
		NEO_DEBUG_ESC_BREAK // NEO: ND - [NeoDebug]
		IPTableStruct* curIPTable = m_IPTables.GetPrev(pos);
		//IPTableStruct* curIPTable = m_IPTables.GetNext(pos);
		
		// this table propobly not belong to our analysis subject
		if(FilterIPZone && !CheckIPZone(curIPTable->uIP))
			continue;

		if(curIPTable->tLastSeen < curIPTable->tFirstSeen){
			ASSERT(0); // invalid table, very bad...
			continue;
		}

		ToAnalyze --;

		if(!IPRepeatingTable.Lookup(curIPTable->uIP,IPRepeatInfo)){ // Does we already had this IP
			IPRepeatInfo = new IPRepeatInfoStruct;
			memset(IPRepeatInfo,0,sizeof(IPRepeatInfoStruct));
			IPRepeatingTable.SetAt(curIPTable->uIP,IPRepeatInfo);
		}

		if(!curTempIPTable || curIPTable->uIP != curTempIPTable->uIP) // does the IP changed
			IPRepeatInfo->uRepeat++; // increment IP Repeat count

		IPRepeatInfo->uTotal++; // increment IP sessions count
		IPRepeatInfo->tTotalSeen += curIPTable->tLastSeen - curIPTable->tFirstSeen; // how long was the ip seen totaly

		// Create a new entry
		curTempIPTable = new IPTableStruct; 
		memcpy(curTempIPTable,curIPTable,sizeof(IPTableStruct)); // copy all data into the new Table entry
		TempIPTables.AddHead(curTempIPTable);
		//TempIPTables.AddTail(curTempIPTable);
	}

	uint32 TotalTime = 0;
	if(TempIPTables.GetCount() == 0)
		return;
	else if(ToAnalyze /*TempIPTables.GetCount() < NeoPrefs.GetTableAmountToAnalyze()*/){ // we have not enough tables
		if(TempIPTables.GetTail()->tLastSeen >= TempIPTables.GetHead()->tFirstSeen)
			TotalTime = TempIPTables.GetTail()->tLastSeen - TempIPTables.GetHead()->tFirstSeen; // Total duration of the observation
//		else
//			ASSERT(0);
		m_iAnalisisQuality = 20 * max(TempIPTables.GetCount(),(int)S2D(TotalTime)) / NeoPrefs.GetTableAmountToAnalyze(); // calculate the quality of our analisis
		if(m_iAnalisisQuality > 10)
			m_iAnalisisQuality = 10;
	}else
		m_iAnalisisQuality = 10; // top quality

	// Now lets clean up the list from bad gap's
	// Theoreticly this isn't nessesery, but It is very important to reduce the effect of distubances and bad datas

	uint16 SeenIPCount = 0;
	uint16 DropedIPCount = 0;

	IPRepeatInfoStruct* tempIPRepeatInfo; // temporal value

	if(NeoPrefs.HandleTableGaps())
	{
		uint16 IPRepeat; // temporal value
		uint32 SplitedIP;  // temporal value

		uint32 GapIP;  // temporal value
		uint32 SplitedGapIP;  // temporal value

		uint16 GapSize; // temporal value
		uint32 GapTime; // temporal value
		uint8  GapInGap; // temporal value

		UINT InitialTabeleSize = TempIPTables.GetCount();

		// when the gap is to big don't remove it
		const float PriorityGapRatio = (float)NeoPrefs.GetPriorityGapRatio()/100.0F;
		const int	MaxGapSize = NeoPrefs.GetMaxGapSize();
		const uint32 MaxGapTime = NeoPrefs.GetMaxGapTime();

		// this loop will filter all gaps that can be detected
		POSITION pos1 = IPRepeatingTable.GetStartPosition();
		while (pos1){ // Lets find the gaps
			NEO_DEBUG_ESC_BREAK // NEO: ND - [NeoDebug]
			IPRepeatingTable.GetNextAssoc(pos1, SplitedIP, IPRepeatInfo);
			SeenIPCount++;
			if(IPRepeatInfo->uRepeat < 2) // ok the IP was seen once, no gaps
				continue;

			IPRepeat = IPRepeatInfo->uRepeat; // we have an gap
			POSITION FirstTable = NULL;
			POSITION GapBegin = NULL;
			POSITION RepeatTable = NULL;
			for (POSITION pos2 = TempIPTables.GetHeadPosition(); pos2 != NULL;){ // lets bypas the gaps
				NEO_DEBUG_ESC_BREAK // NEO: ND - [NeoDebug]
				FirstTable = pos2; // Position of the first Table
				IPTableStruct* curTempIPTable = TempIPTables.GetNext(pos2);
				if(curTempIPTable->uIP != SplitedIP)
					continue;

				// in case serie n>1 entries go to the end of serie
				while(pos2 && TempIPTables.GetAt(pos2)->uIP == SplitedIP)
					TempIPTables.GetNext(pos2);

				// ok we found the first gap
				GapBegin = pos2; // Position of the gab Table
				GapSize = 0;
				GapTime = 0;
				GapInGap = 0;
				ASSERT(pos2);
				for (;pos2 != NULL;){ // lets see how big the gap is
					NEO_DEBUG_ESC_BREAK // NEO: ND - [NeoDebug]
					RepeatTable = pos2; // Position of the repeated Table
					IPTableStruct* gapTempIPTable = TempIPTables.GetNext(pos2);
					if(gapTempIPTable->uIP != SplitedIP){
						if(IPRepeatingTable.Lookup(SplitedIP,tempIPRepeatInfo))
							if(tempIPRepeatInfo->uRepeat > 1){ // do we have an Gap in an other Gap
								if((tempIPRepeatInfo->tTotalSeen*PriorityGapRatio > IPRepeatInfo->tTotalSeen) // is the secund gap noticable biger that the curren
								|| (tempIPRepeatInfo->uTotal*PriorityGapRatio > IPRepeatInfo->uTotal)){ // is the secund gap noticable biger that the curren // X?: should the ations be separated
									GapInGap = 1; // then let it unchanged
									break;
								}else
									GapInGap = 2; // Update the repeat table for this gap
							}
						GapSize ++;
						GapTime += (gapTempIPTable->tLastSeen - gapTempIPTable->tFirstSeen);
					}else // this is the end of this gap
						break;
				}

				SplitedGapIP = 0;
				if((GapInGap != 1) && (GapSize <= MaxGapSize || GapTime <= MaxGapTime) ){ // Is the gap small enough?
					ASSERT(GapBegin != RepeatTable);
					for (POSITION pos3 = GapBegin;pos3 != RepeatTable;){ // lets erase the gab
						NEO_DEBUG_ESC_BREAK // NEO: ND - [NeoDebug]
						POSITION toRemove = pos3;
						IPTableStruct* gapTempIPTable = TempIPTables.GetNext(pos3);
						GapIP = gapTempIPTable->uIP;
						if(IPRepeatingTable.Lookup(GapIP,tempIPRepeatInfo)){
							ASSERT(tempIPRepeatInfo->uTotal);
							tempIPRepeatInfo->uTotal--; // decrement IP Repeat count for the secund gap
							if(tempIPRepeatInfo->uTotal == 0){
								DropedIPCount++; // Note: Wen can not remove the empty entried from our map, this coase exceptions in combination with the GetNextAssoc, becouse the pos1 is then pointing to an removed entire
								SplitedGapIP = 0; // we must reset the last ip
							}else if(GapInGap == 2 && tempIPRepeatInfo->uRepeat > 0){ // Let's update the IP Repeat count
								if(SplitedGapIP != GapIP){ // in the case we have a segmented Gap in Gap
									tempIPRepeatInfo->uRepeat--; // decrement IP Repeat count
									SplitedGapIP = GapIP;
								}else if(SplitedGapIP == GapIP){
									SplitedGapIP = 0;
								}
							}
						}
						TempIPTables.RemoveAt(toRemove);
						delete gapTempIPTable;
					}
					IPRepeatInfo->uRepeat--; // decrement IP Repeat count
				}

				IPRepeat--;
				if(IPRepeat == 1) // was this all gabs?
					break;

				pos2 = FirstTable; // Set the Position of the first Table, to seartch for more gabs
			}
		}

		// had we lost some datas, then reduce the quality
		m_iAnalisisQuality *= TempIPTables.GetCount();
		m_iAnalisisQuality /= InitialTabeleSize;
	}

	// Ok we have a clean list.

	// lets create an meta IP table that conteins a combination of all tabeles

	CList<MetaIPTableStruct*,MetaIPTableStruct*> MetaIPTables; // Owr meta tabele
	MetaIPTableStruct* curMetaIPTable = NULL;
	ASSERT( TempIPTables.GetCount() ); // Is should be at least one entry

	bool NextIPChanged = true; // First IP is a new one so changed
	uint32 BlankTime; // temp variable

	for (pos = TempIPTables.GetHeadPosition(); pos != NULL;){
		NEO_DEBUG_ESC_BREAK // NEO: ND - [NeoDebug]
		IPTableStruct* curIPTable = TempIPTables.GetNext(pos);
		IPTableStruct* nextIPTable = pos ? TempIPTables.GetAt(pos) : NULL;

		//if(curMetaIPTable == NULL || curIPTable->uIP != curMetaIPTable->uIP){
		if(NextIPChanged){ // does the IP changed
			// Begin a new table
			curMetaIPTable = new MetaIPTableStruct;
			memset(curMetaIPTable,0,sizeof(MetaIPTableStruct));
			MetaIPTables.AddTail(curMetaIPTable);
			curMetaIPTable->uIP = curIPTable->uIP;
			curMetaIPTable->uPort = curIPTable->uPort;
			curMetaIPTable->tFirstSeen = curIPTable->tFirstSeen;
		}
		curMetaIPTable->tLastSeen = curIPTable->tLastSeen; // bypas the times we wer offline or he wer offline

		NextIPChanged = (nextIPTable && (curMetaIPTable->uIP != nextIPTable->uIP)); // has the next ip changed
		if(nextIPTable && nextIPTable->tFirstSeen >= curIPTable->tLastSeen){
			BlankTime = nextIPTable->tFirstSeen - curIPTable->tLastSeen; // blank duration is our time
		}else{
			BlankTime = 0;
			//ASSERT(!nextIPTable);
		}
			
		curMetaIPTable->SeenOnLineTime += curIPTable->tLastSeen - curIPTable->tFirstSeen; // time we seen him forsure online
		if(curIPTable->uUnreachable == 1){ // does it end with a faild connection
			if(!NextIPChanged){ // only if the new IP is still the same
				curMetaIPTable->OffLineTime += BlankTime; // blank duration is our time
				curMetaIPTable->uFaildCount++;
			}else{
				ASSERT(curMetaIPTable->uLostCount == 0); // must be 0
				curMetaIPTable->uLostCount++; // can become only 1
			}
		}
		if(curIPTable->uUnreachable != 1){ // does if end with a drop eg. dl. done
			if(!NextIPChanged){ // only if the new IP is still the same
				curMetaIPTable->OnLineTime += BlankTime; // blank duration is our time
			}
		}
	}

	// cool, we have preprocesed datas, now the easy part
	// Analize the IP behavior...

	MidleStruct MidleOnTime;	INIT_MIDLE_LIMES(MidleOnTime)
	MidleStruct MidleOffTime;	INIT_MIDLE_LIMES(MidleOffTime)
	MidleStruct MidleIPTime;	INIT_MIDLE_LIMES(MidleIPTime)
	MidleStruct MidleFaildCount;INIT_MIDLE_LIMES(MidleFaildCount)

	const float MaxMidleHigh = (float)NeoPrefs.GetMaxMidleDiscrepanceHigh()/100.0F;
	const float LimMidleHigh = MaxMidleHigh * 5 / 4; // ignore small discrepances

	const float MaxMidleLow = (float)NeoPrefs.GetMaxMidleDiscrepanceLow()/100.0F;
	const float LimMidleLow = MaxMidleLow * 5 / 4; // ignore small discrepances 

	const bool	bLinkGravity = NeoPrefs.UseDualLinkedTableGravity();
	const int	iLinkGravity = NeoPrefs.GetDualLinkedTableGravity();
	UINT uGravity;

	UINT InitialTabeleSize = MetaIPTables.GetCount();
	UINT DirtyValues = 0;

	MetaIPTableStruct* previewMetaIPTable;
	MetaIPTableStruct* nextMetaIPTable;

	int AllowedLoops = 100; // NEO: ND - [NeoDebug]

	bool IsClean = false;
	do{
		NEO_DEBUG_ESC_BREAK // NEO: ND - [NeoDebug]
		IsClean = true; // we hope the best

		INIT_MIDLE(MidleOnTime)
		INIT_MIDLE(MidleOffTime)
		INIT_MIDLE(MidleIPTime)
		INIT_MIDLE(MidleFaildCount)

		// NEO: ND - [NeoDebug]
		AllowedLoops --;
		if(!AllowedLoops){
			DebugLogError(_T("NSA: AnalyzeIPBehavior can't exit final loop, loop abborted"));
			ASSERT(0);
#ifdef _DEBUG_NEO
			RaiseException(0x00000000,NULL,NULL,NULL); // will dump the table in debug/test builds for later analysis
#endif 
			break;
		}
		// NEO: ND END

		previewMetaIPTable = NULL;
		curMetaIPTable = NULL;
		nextMetaIPTable = NULL;

		// find the min max values and count the total values
		for (pos = MetaIPTables.GetHeadPosition(); pos != NULL;){
			NEO_DEBUG_ESC_BREAK // NEO: ND - [NeoDebug]
			previewMetaIPTable = curMetaIPTable;
			curMetaIPTable = MetaIPTables.GetNext(pos);
			nextMetaIPTable = pos ? MetaIPTables.GetAt(pos) : NULL;

			// When the table is linked on booth sides it is very reliable so magnify the datas from this table
			uGravity = (bLinkGravity && previewMetaIPTable && nextMetaIPTable
			 && curMetaIPTable->tFirstSeen - previewMetaIPTable->tLastSeen < (uint32)NeoPrefs.GetLinkTimeThreshold() // time bevoure this table
			 && nextMetaIPTable->tFirstSeen - curMetaIPTable->tLastSeen < (uint32)NeoPrefs.GetLinkTimeThreshold() // time after this table
			) ? iLinkGravity : 1;

			// process the values
			HandleMidle(MidleOnTime,(curMetaIPTable->OnLineTime + curMetaIPTable->SeenOnLineTime),uGravity); // Add the time we see him, he was the ofcorse also online
			HandleMidle(MidleOffTime,(curMetaIPTable->OffLineTime),uGravity);
			HandleMidle(MidleIPTime,(curMetaIPTable->OnLineTime + curMetaIPTable->OffLineTime + curMetaIPTable->SeenOnLineTime),uGravity); /*curMetaIPTable->AvalibilityTime + curMetaIPTable->SeenOnLineTime*/
			HandleMidle(MidleFaildCount,(curMetaIPTable->uFaildCount));
			if(curMetaIPTable->uLostCount)
				MidleFaildCount.Count++;
		}

		// Now lets calculate the midle value and check for very abnormal values
		MidleAndLimes(MidleOnTime,IsClean,MaxMidleHigh,LimMidleHigh,MaxMidleLow,LimMidleLow);
		MidleAndLimes(MidleOffTime,IsClean,MaxMidleHigh,LimMidleHigh,MaxMidleLow,LimMidleLow);
		MidleAndLimes(MidleIPTime,IsClean,MaxMidleHigh,LimMidleHigh,MaxMidleLow,LimMidleLow);
		MidleAndLimes(MidleFaildCount,IsClean,MaxMidleHigh,LimMidleHigh,MaxMidleLow,LimMidleLow);

		if(!IsClean)
			DirtyValues++;
	}while(!IsClean); // do we have clean values?

	if(DirtyValues > 2){ // allow some exceptions
		m_iAnalisisQuality *= InitialTabeleSize - min(DirtyValues,InitialTabeleSize/2);
		m_iAnalisisQuality /= InitialTabeleSize;
	}

	// super we have clean values for the propability calculation

	// Check for static IP
	if(NeoPrefs.HandleTableGaps()){
		ASSERT(SeenIPCount - DropedIPCount > 0);
		m_uStaticIP = (SeenIPCount - DropedIPCount == 1); // so let see is this a static one?
	}else
		m_uStaticIP = (IPRepeatingTable.GetCount() == 1); // so let see is this the only one?

	if(m_iAnalisisQuality < 1 && TempIPTables.GetCount() > 1) // If we have at least 2 tables
		m_iAnalisisQuality = 1; // a minimal quality is granted
	else if(TempIPTables.GetCount() < 2) // with less than 2 tables a analysis is not possible
		m_iAnalisisQuality = 0; // set unsifficient quality

	curMetaIPTable = MetaIPTables.GetTail();

	// Check for valid analysis
	if((FilterIPZone && !CheckIPZone(m_dwUserIP))
	|| curMetaIPTable->uIP != m_dwUserIP 
	|| curMetaIPTable->uPort != m_nUserPort) 
		m_iAnalisisQuality = 0; // Analysis result is not valid for the current IP

	m_uLastSeenDuration = (curMetaIPTable->OnLineTime + curMetaIPTable->SeenOnLineTime); // Time span we saw the ip the last time
	m_uTotalSeenDuration = TotalTime; // Total anaylsed time span
	if(MetaIPTables.GetCount() > 1){
		pos = MetaIPTables.GetTailPosition(); MetaIPTables.GetPrev(pos); ASSERT(pos);
		if(curMetaIPTable->tFirstSeen >= MetaIPTables.GetAt(pos)->tLastSeen)
			m_uLastLinkTime = curMetaIPTable->tFirstSeen - MetaIPTables.GetAt(pos)->tLastSeen; // timespan pased on IP change
//		else{
//			m_uLastLinkTime = 0;
//			ASSERT(0);
//		}
	}else
		m_uLastLinkTime = 0; // No last link time
	
	// Save Values
	m_uLongestOnTime = MidleOnTime.Maximal;
	m_uMidleOnTime = MidleOnTime.Midle;
	m_uShortestOnTime = MidleOnTime.Minimal;
	
	m_uLongestOffTime = MidleOffTime.Maximal;
	m_uMidleOffTime = MidleOffTime.Midle;
	m_uShortestOffTime = MidleOffTime.Minimal;
	
	m_uLongestIPTime = MidleIPTime.Maximal;
	m_uMidleIPTime = MidleIPTime.Midle;
	m_uShortestIPTime = MidleIPTime.Minimal;

	m_uLargestFaildCount = MidleFaildCount.Maximal;
	m_uMidleFaildCount = MidleFaildCount.Midle;
	m_uSmallestFaildCount = MidleFaildCount.Minimal;

	// Cleanu up after done work...
	while (!TempIPTables.IsEmpty())
		delete TempIPTables.RemoveHead();
	TempIPTables.RemoveAll();

	IPRepeatInfoStruct* cur_struct;
	uint32 tmpkey;
	pos = IPRepeatingTable.GetStartPosition();
	while (pos){
		IPRepeatingTable.GetNextAssoc(pos, tmpkey, cur_struct);
		delete cur_struct;
	}
	IPRepeatingTable.RemoveAll();

	while (!MetaIPTables.IsEmpty())
		delete MetaIPTables.RemoveHead();
	MetaIPTables.RemoveAll();

	// Finaly, we reached the end, the analysion is done, ufff... 
}

EIPType	CKnownSource::GetIPType() const {
	if(!m_uLastAnalisis)
		return IP_Unknown;

	if(GetLongestIPTime() < (uint32)NeoPrefs.GetTempralIPBorderLine())
		return IP_Temporary;

	if(m_uStaticIP)
		return IP_Static;

	return IP_Dynamic;
}

// This function calculates the propability of alalibility
// aslong as the propability is above 0 the source should be avalibly
// when it is below 0 there is still a small posibility of avalibility
void CKnownSource::CalculateAvalibilityProbability()
{
	// alanyse the IP tables if needed
	if((!GetLastAnalisis() || (time(NULL) - GetLastAnalisis() > (uint32)NeoPrefs.GetAnalyzeIntervals()/(11-GetAnalisisQuality()))) && m_bAnalisisNeeded)
		AnalyzeIPBehavior();

	uint32 uTimeSinceLastSeen = (time(NULL) - GetLastSeen());

	// check if the source is fresh
	if(uTimeSinceLastSeen < (uint32)NeoPrefs.GetFreshSourceTreshold()){ // if nt more time passed we can use this source as a regular one
		m_iAvalibilityProbability = 100;
		return;
	}

	// check if the source is completly retired
	if(GetLongestIPTime() != 0 && uTimeSinceLastSeen > GetLongestIPTime()){ // after this time there is no chance to see it back under the known ip
		m_iAvalibilityProbability = -100;
		return;
	}

	float Propability = 0.0F; // Our Result

	// check is the source could be analysed
	if(GetAnalisisQuality() < 1){ // this actualy mean we had only one ip table in storage
		// if it couldn't do the analyse we attempt to calculate a heuristic propability

		//IPTableStruct* lastIPTable = m_IPTables.GetTail(); // last and only valid iptable
		float SeenTime = (float)GetLastSeenDuration(); // lastIPTable->tLastSeen - lastIPTable->tFirstSeen // How long was teh source avalibly the last time
		float LastSeen = (float)(time(NULL) - GetLastSeen()); // (time(NULL) - lastIPTable->tLastSeen) // How much time pased

		if(SeenTime > 0)
		{
			if(LastSeen > SeenTime) // to much time pased
				Propability = -1.0F * ((LastSeen-SeenTime)/SeenTime); // The propability of his un avalibility
			else if(LastSeen < SeenTime)
				Propability = 1.0F - (LastSeen / SeenTime); // The propability of his avalibility
			else
				Propability = 0.0F;
		}
	}
	else
	{
		// here we have a sufficient quality, actualy we have a quality meaning we had somethink for a minimal analyse
		// so we can do a full calculation.
		// Note: A bad quality results only in less reasks but not in more faild connection attempts.

		// Tests shown me that for clients with shorter sessions the midle value is recorded usualy much to low 
		// so it is recomended to correct it with the maximal seen value. It usualy does not increase the fail procentage!

		float ModeGravity = (float)NeoPrefs.GetEnhancedFactor()/100.0F;
		float MidleGravity = 1;
		float MaxGravity = 1;
		if(ModeGravity > 1.0F)
			MaxGravity = ModeGravity;
		else if(ModeGravity < 1.0F)
			MidleGravity = ((1.0F - ModeGravity)*10) + 1;
		
		uint32 OnTime  = (uint32)(((GetMidleOnTime() *MidleGravity) + (GetLongestOnTime() *MaxGravity)) / (MidleGravity + MaxGravity));
		uint32 OffTime = (uint32)(((GetMidleOffTime()*MidleGravity) + (GetLongestOffTime()*MaxGravity)) / (MidleGravity + MaxGravity));
		uint32 IPTime  = (uint32)(((GetMidleIPTime() *MidleGravity) + (GetLongestIPTime() *MaxGravity)) / (MidleGravity + MaxGravity));
		//uint16 FaildCount = (uint16)(((GetMidleFaildCount()*MidleGravity) + (GetLargestFaildCount()*MaxGravity)) / (MidleGravity + MaxGravity));

		float TimeSinceLastSeen = (float)(time(NULL) - GetLastSeen()); // Time since last contact
		float TimeSinceFirstSeen = TimeSinceLastSeen + GetLastSeenDuration(); // Time sinc we saw the IP the first time

		switch(GetIPType()){
			case IP_Unknown:
				m_iAvalibilityProbability = 0;
				return;
			case IP_Static:{
				if(!OnTime){ // For static IP relay on the Online time
					m_iAvalibilityProbability = 0;
					return;
				}

				float LastTime = TimeSinceLastSeen;

				if(LastTime > OffTime) // When the offline time may take place inside of the timespan 
					LastTime -= OffTime; // we may substract it from the value

				if(LastTime > OnTime) // to much time pased
					Propability = -1.0F * ((LastTime-OnTime)/OnTime); // The propability of his un avalibility
				else if(LastTime < OnTime)
					Propability = 1.0F - (LastTime / OnTime); // The propability of his avalibility
				else
					Propability = 0.0F;

				break;
			}
			case IP_Dynamic:{
				if(!IPTime){ // For dynamic IP relay on the ip validity time
					m_iAvalibilityProbability = 0;
					return;
				}

				float LastTime;
				if((uint32)(GetTotalSeenDuration()/((float)NeoPrefs.GetLastSeenDurationThreshold()/100.F)) > GetLastSeenDuration()) // Is the GetLastSeenDuration relaiable?
					LastTime = TimeSinceFirstSeen;
				else
					LastTime = TimeSinceLastSeen;

				if(TimeSinceLastSeen > OffTime) // When the offline time may take place inside of the timespan 
					LastTime -= OffTime; // we may substract it from the value

				if(LastTime > IPTime) // to much time pased
					Propability = -1.0F * ((LastTime-IPTime)/IPTime); // The propability of his un avalibility
				else if(LastTime < IPTime)
					Propability = 1.0F - (LastTime / IPTime); // The propability of his avalibility
				else
					Propability = 0.0F;

				break;
			}
			case IP_Temporary:{
				if(!OnTime){ // For temporary IP relay on the Online time
					m_iAvalibilityProbability = 0;
					return;
				}

				float LastTime = TimeSinceFirstSeen;

				if(LastTime > OnTime) // to much time pased
					Propability = -1.0F * ((LastTime-OnTime)/OnTime); // The propability of his un avalibility
				else if(LastTime < OnTime)
					Propability = 1.0F - (LastTime / OnTime); // The propability of his avalibility
				else
					Propability = 0.0F;

				break;
			}
			default:
				ASSERT(0);
		}

		// When we know that there is no noticable undefined interval bevor the user got this IP, we can rise the propability
		if(NeoPrefs.UseLinkTimePropability() && GetLastLinkTime() != 0 && GetLastLinkTime() < (uint32)NeoPrefs.GetLinkTimeThreshold()){ 
			Propability *= 10.0F;
		}
		// Note when the time is linked the magnification becomes useles
		else if(NeoPrefs.ScaleReliableTime() && TimeSinceLastSeen < NeoPrefs.GetMaxReliableTime()){
			float Magnifyer = (1.0F - (TimeSinceLastSeen/NeoPrefs.GetMaxReliableTime()));
			if(Propability > 0.0F && Magnifyer > 0.1F) // Magnify only positiv propabilities
				Propability *= 10.0F * Magnifyer;
		}
	}

	// result in Procent
		 if(Propability >  1.0F)	m_iAvalibilityProbability = 100;
	else if(Propability < -1.0F)	m_iAvalibilityProbability = -100;
	else m_iAvalibilityProbability = (int)(Propability * 100);
}

uint32 CKnownSource::GetRemindingIPTime()
{
	uint32 TimePassed = (time(NULL) - GetLastSeen());
	uint32 LongestIPTime = GetLongestIPTime();
	if(TimePassed > LongestIPTime)
		return 0;
	return (LongestIPTime - TimePassed);
}

#endif // NEO_SA // NEO: NSA END

/////////////////////////////////////////////////////////////////////////////
// Seen File List

// NEO: SFL - [SourceFileList]
bool CKnownSource::LoadSeenFiles(CFileDataIO* file){
	static SeenFileStruct* newSeenFile = NULL; // in the case of a file exception this will prevent a memlake.
	if(newSeenFile)
		delete newSeenFile;

	UINT RecordsNumber = file->ReadUInt32();
	for (UINT i = 0; i < RecordsNumber; i++) {
		newSeenFile = new SeenFileStruct;
		memset(newSeenFile,0,sizeof(SeenFileStruct));

		UINT tagcount = file->ReadUInt32();
		for (UINT j = 0; j < tagcount; j++) {
			CTag* newtag = new CTag(file, false);
			switch(newtag->GetNameID()){
				case FFT_HASH:{
					ASSERT( newtag->IsHash() );
					if(newtag->IsHash())
						md4cpy(newSeenFile->abyFileHash, newtag->GetHash());
					delete newtag;
					break;
				}
				case FFT_SIZE:{
					ASSERT( newtag->IsInt64() );
					newSeenFile->uFileSize = newtag->GetInt64();
					delete newtag;
					break;
				}
				case FFT_LASTSEEN:{
					ASSERT( newtag->IsInt() );
					newSeenFile->tLastSeen = newtag->GetInt();
					delete newtag;
					break;
				}
				// NEO: SCFS - [SmartClientFileStatus]
				case FFT_FILE_NAME:{
					ASSERT( newtag->IsStr() );
					if(newtag->IsStr()){
						if(newSeenFile->prtFileStatus == NULL)
							newSeenFile->prtFileStatus = new CClientFileStatus(newSeenFile->abyFileHash,newSeenFile->uFileSize);
						newSeenFile->prtFileStatus->SetFileName(newtag->GetStr());
					}
					delete newtag;
					break;
				}

				case FFT_PART_STATUS:
				case FFT_INC_PART_STATUS:
				case FFT_SEEN_PART_STATUS:{
					ASSERT( newtag->IsBlob() );
					if(newtag->IsBlob() && newSeenFile->prtFileStatus) // if we write status we write the filname *ALWAYS* first
						newSeenFile->prtFileStatus->ReadFileStatusTag(newtag);
					delete newtag;
					break;
				}
				
				case FFT_SCT_STATUS:{
					ASSERT( newtag->IsBlob() );
					if(newtag->IsBlob() && newSeenFile->prtFileStatus){ // if we write status we write the filname *ALWAYS* first
						CSafeMemFile data(newtag->GetBlob(),newtag->GetBlobSize());
						newSeenFile->prtFileStatus->ReadSubChunkMaps(&data);
					}
					delete newtag;
					break;
				}
				// NEO: SCFS END
				default:{
					if(!newSeenFile->taglist)
						newSeenFile->taglist = new CArray<CTag*,CTag*>;
					newSeenFile->taglist->Add(newtag);
				}
			}
		}

		m_SeenFiles.SetAt(CCKey(newSeenFile->abyFileHash), newSeenFile);
		newSeenFile = NULL;
	}

	return true;
}

void CKnownSource::SaveSeenFiles(CFileDataIO* file){
	uint32 RecordsNumber = 0;
	ULONG RecordsNumberFilePos = (ULONG)file->GetPosition();
	file->WriteUInt32(RecordsNumber);

	uint32 uTagCount;
	ULONG uTagCountFilePos;

	const uint32 dwExpired = time(NULL) - NeoPrefs.GetFileListExpirationTime();

	SeenFileStruct* curSeenFile;
	CCKey tempkey(0);
	POSITION pos = m_SeenFiles.GetStartPosition();
	while (pos)
	{
		m_SeenFiles.GetNextAssoc(pos, tempkey, curSeenFile);
		if(curSeenFile->tLastSeen < dwExpired)
			continue;

		uTagCount = 0;
		uTagCountFilePos = (ULONG)file->GetPosition();
		file->WriteUInt32(uTagCount);

		CTag(FFT_HASH, curSeenFile->abyFileHash).WriteNewEd2kTag(file); uTagCount++;
		CTag(FFT_SIZE, curSeenFile->uFileSize,(curSeenFile->uFileSize > (uint64)OLD_MAX_EMULE_FILE_SIZE)).WriteTagToFile(file); uTagCount++;
		CTag(FFT_LASTSEEN, curSeenFile->tLastSeen).WriteTagToFile(file); uTagCount++;

		// NEO: SCFS - [SmartClientFileStatus]
		if(curSeenFile->prtFileStatus)
		{
			CTag tag(FFT_FILE_NAME,curSeenFile->prtFileStatus->GetFileName());
			tag.WriteNewEd2kTag(file);
			uTagCount++;

			for(uint16 i = 0; i<CFS_COUNT; i++){
				if(curSeenFile->prtFileStatus->WriteFileStatusTag((EPartStatus)i,file))
					uTagCount++;
			}

			CSafeMemFile data(16+16);
			if(curSeenFile->prtFileStatus->WriteSubChunkMaps(&data)){
				uint32 size = (UINT)data.GetLength();
				BYTE* tmp = data.Detach();
				CTag tag(FFT_SCT_STATUS,size,tmp);
				free(tmp);
				tag.WriteNewEd2kTag(file);
			}
		}
		// NEO: SCFS END

		// unidentified tags(from mods/future versions)
		if(curSeenFile->taglist)
			for (int j = 0; j < curSeenFile->taglist->GetCount(); j++) {
				curSeenFile->taglist->GetAt(j)->WriteTagToFile(file);
				uTagCount++;
			}

		file->Seek(uTagCountFilePos, CFile::begin);
		file->WriteUInt32(uTagCount);
		file->Seek(0, CFile::end);

		RecordsNumber ++;
	}

	file->Seek(RecordsNumberFilePos, CFile::begin);
	file->WriteUInt32(RecordsNumber);
	file->Seek(0, CFile::end);
}

SeenFileStruct* CKnownSource::AddSeenFile(const uchar* hash, uint64 size){
	SeenFileStruct* result;
	CCKey tkey(hash);
	if (!m_SeenFiles.Lookup(tkey, result)){
		result = new SeenFileStruct;
		memset(result,0,sizeof(SeenFileStruct));
		md4cpy(result->abyFileHash, hash);
		result->uFileSize = size;
		m_SeenFiles.SetAt(CCKey(result->abyFileHash), result);
	}
	result->tLastSeen = time(NULL);
	return result;
}

void CKnownSource::RemoveSeenFile(const uchar* hash){
	SeenFileStruct* result;
	CCKey tkey(hash);
	if (m_SeenFiles.Lookup(tkey, result)){
		m_SeenFiles.RemoveKey(tkey);
		ClearTagList(result->taglist);
		delete result;
	}
}

SeenFileStruct* CKnownSource::GetSeenFile(const uchar* hash) const {
	SeenFileStruct* result;
	CCKey tkey(hash);
	if (m_SeenFiles.Lookup(tkey, result))
		return result;
	return NULL;
}

void CSourceList::FindSources(CPartFile* pFile)
{
	uint16 count = 0;
	CKnownSource* cur_source;
	SeenFileStruct* cur_seenfile;
	CCKey tempkey(0);
	POSITION pos = m_mapSources.GetStartPosition();
	while (pos)
	{
		m_mapSources.GetNextAssoc(pos, tempkey, cur_source);
		cur_seenfile = cur_source->GetSeenFile(pFile->GetFileHash());
		if(cur_seenfile)
		{
			if(cur_seenfile->uFileSize == pFile->GetFileSize())
			{
				CUpDownClient* newsource = new CUpDownClient(pFile,cur_source->GetUserPort(),cur_source->GetIP(),0,0,true);
				newsource->SetUserHash(cur_source->GetUserHash());
				newsource->SetSourceFrom(SF_STORAGE);
				if(!pFile->PartPrefs->UseTotalSourceRestore() || newsource->IsLongTermStorred()) // short term storred sources goes active imminetly if total restore is enabled
					newsource->SetDownloadState(DS_LOADED);
				newsource->AttachSource(cur_source);
				theApp.downloadqueue->CheckAndAddSource(pFile,newsource);
				count++;
			}
			else
				DebugLog(_T("SourceFileList: Found file with maching hast %s, but different file size: %I64u instad of %I64u"),md4str(pFile->GetFileHash()),cur_seenfile->uFileSize,pFile->GetFileSize());
		}
	}

	if(count){
		pFile->AnalizerSources();
		pFile->SortSourceList();
		ModLog(GetResString(IDS_X_SOURCESFOUND), count, pFile->GetFileName());
	}
}

bool CSourceList::SourceHaveFiles(CKnownSource* source){
	SeenFileStruct* curSeenFile;
	CCKey tmpkey(0);
	POSITION fpos;
	fpos = source->m_SeenFiles.GetStartPosition();
	while (fpos)
	{
		source->m_SeenFiles.GetNextAssoc(fpos, tmpkey, curSeenFile);
		if(theApp.downloadqueue->GetFileByID(curSeenFile->abyFileHash) != NULL)
			return true;
	}
	return false;
}

// NEO: SCFS - [SmartClientFileStatus]
SeenFileStruct::~SeenFileStruct()
{
	if(prtFileStatus){
		if(prtFileStatus->IsUsed())
			prtFileStatus->SetArcived(false);
		else
			delete prtFileStatus;
	}
}
// NEO: SCFS END
// NEO: SFL END

#endif // NEO_CD // NEO: NCD END <-- Xanatos --