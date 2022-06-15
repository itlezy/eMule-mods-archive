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
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#include "stdafx.h"
#include <io.h>
#include "emule.h"
#include "SharedFileList.h"
#include "KnownFileList.h"
#include "KnownFile.h"
#include "opcodes.h"
#include "Preferences.h"
#include "SafeFile.h"
#include "OtherFunctions.h"
#include "UpDownClient.h"
#include "DownloadQueue.h"
#include "emuledlg.h"
#include "TransferWnd.h"
#include "Log.h"
#include "packets.h"
#include "MD5Sum.h"
#include "Neo/Functions.h" // NEO: MOD <-- Xanatos --
#include "Neo/NeoPreferences.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --
#include "Neo/FilePreferences.h" // NEO: FCFG - [FileConfiguration] <-- Xanatos --
#include "partfile.h" // NEO: FCFG - [FileConfiguration] <-- Xanatos --
#include "Neo/NeoOpcodes.h" // NEO: MOD <-- Xanatos --
// NEO: XCk - [KnownComments] -- Xanatos -->
#include "clientlist.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "kademlia/kademlia/search.h"
#include "kademlia/kademlia/SearchManager.h"
#include "kademlia/kademlia/prefs.h"
// NEO: XCk END <-- Xanatos --

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define KNOWN_MET_FILENAME		_T("known.met")
#define CANCELLED_MET_FILENAME	_T("cancelled.met")

#define COMMENTS_MET_FILENAME	_T("comments.met") // NEO: XCs - [SaveComments] <-- Xanatos --

#define CANCELLED_HEADER_OLD	MET_HEADER
#define CANCELLED_HEADER		MET_HEADER + 0x01
#define CANCELLED_VERSION		0x01

CKnownFileList::CKnownFileList()
{
	m_Files_map.InitHashTable(2063);
	m_mapCancelledFiles.InitHashTable(1031);
	accepted = 0;
	requested = 0;
	transferred = 0;
	m_dwCancelledFilesSeed = 0;
	m_nLastSaved = ::GetTickCount();
	m_bPartTrafficLoaded = false; // NEO: NPT - [NeoPartTraffic] <-- Xanatos --
	m_bCommentsLoaded = false; // NEO: XCs - [SaveComments] <-- Xanatos --
	m_currFileNotes = 0; // NEO: XCk - [KnownComments] <-- Xanatos --
	Init();
}

CKnownFileList::~CKnownFileList()
{
	Clear();
}

bool CKnownFileList::Init()
{
	//return LoadKnownFiles() && LoadCancelledFiles();
	return LoadKnownFiles() 
		&& LoadKnownPreferences()	// NEO: FCFG - [FileConfiguration] <-- Xanatos --
		&& LoadPartTraffic()		// NEO: NPT - [NeoPartTraffic] <-- Xanatos --
		&& (!NeoPrefs.UseSaveComments() || LoadComments()) // NEO: XCs - [SaveComments] <-- Xanatos --
		&& LoadCancelledFiles();
}

bool CKnownFileList::LoadKnownFiles()
{
	CString fullpath = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR);
	fullpath.Append(KNOWN_MET_FILENAME);
	CSafeBufferedFile file;
	CFileException fexp;
	if (!file.Open(fullpath,CFile::modeRead|CFile::osSequentialScan|CFile::typeBinary|CFile::shareDenyWrite, &fexp)){
		if (fexp.m_cause != CFileException::fileNotFound){
			CString strError(_T("Failed to load ") KNOWN_MET_FILENAME _T(" file"));
			TCHAR szError[MAX_CFEXP_ERRORMSG];
			if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
				strError += _T(" - ");
				strError += szError;
			}
			LogError(LOG_STATUSBAR, _T("%s"), strError);
		}
		return false;
	}
	setvbuf(file.m_pStream, NULL, _IOFBF, 16384);

	CKnownFile* pRecord = NULL;
	try {
		uint8 header = file.ReadUInt8();
		if (header != MET_HEADER && header != MET_HEADER_I64TAGS){
			file.Close();
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_SERVERMET_BAD));
			return false;
		}
		AddDebugLogLine(false, _T("Known.met file version is %u (%s support 64bit tags)"), header, (header == MET_HEADER) ? _T("doesn't") : _T("does")); 

		UINT RecordsNumber = file.ReadUInt32();
		for (UINT i = 0; i < RecordsNumber; i++) {
			pRecord = new CKnownFile();
			if (!pRecord->LoadFromFile(&file)){
				TRACE(_T("*** Failed to load entry %u (name=%s  hash=%s  size=%I64u  parthashs=%u expected parthashs=%u) from known.met\n"), i, 
					pRecord->GetFileName(), md4str(pRecord->GetFileHash()), pRecord->GetFileSize(), pRecord->GetHashCount(), pRecord->GetED2KPartHashCount());
				delete pRecord;
				pRecord = NULL;
				continue;
			}
			SafeAddKFile(pRecord);
			pRecord = NULL;
		}
		file.Close();
	}
	catch(CFileException* error){
		if (error->m_cause == CFileException::endOfFile)
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_SERVERMET_BAD));
		else{
			TCHAR buffer[MAX_CFEXP_ERRORMSG];
			error->GetErrorMessage(buffer, ARRSIZE(buffer));
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_SERVERMET_UNKNOWN),buffer);
		}
		error->Delete();
		delete pRecord;
		return false;
	}

	return true;
}

bool CKnownFileList::LoadCancelledFiles(){
// cancelled.met Format: <Header 1 = CANCELLED_HEADER><Version 1 = CANCELLED_VERSION><Seed 4><Count 4>[<HashHash 16><TagCount 1>[Tags TagCount] Count]
	if (!thePrefs.IsRememberingCancelledFiles())
		return true;
	CString fullpath = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR);
	fullpath.Append(CANCELLED_MET_FILENAME);
	CSafeBufferedFile file;
	CFileException fexp;
	if (!file.Open(fullpath,CFile::modeRead|CFile::osSequentialScan|CFile::typeBinary|CFile::shareDenyWrite, &fexp)){
		if (fexp.m_cause != CFileException::fileNotFound){
			CString strError(_T("Failed to load ") CANCELLED_MET_FILENAME _T(" file"));
			TCHAR szError[MAX_CFEXP_ERRORMSG];
			if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
				strError += _T(" - ");
				strError += szError;
			}
			LogError(LOG_STATUSBAR, _T("%s"), strError);
		}
		return false;
	}
	setvbuf(file.m_pStream, NULL, _IOFBF, 16384);
	uchar ucHash[16];
	try {
		bool bOldVersion = false;
		uint8 header = file.ReadUInt8();
		if (header != CANCELLED_HEADER){
			if (header == CANCELLED_HEADER_OLD){
				bOldVersion = true;
				DebugLog(_T("Deprecated version of cancelled.met found, converting to new version"));
			}
			else{
				file.Close();
				return false;
			}
		}
		uint8 byVersion = 0;
		if (!bOldVersion){
			byVersion = file.ReadUInt8();
			if (byVersion > CANCELLED_VERSION){
				file.Close();
				return false;
			}

			m_dwCancelledFilesSeed = file.ReadUInt32();
		}
		if (m_dwCancelledFilesSeed == 0) {
			ASSERT( bOldVersion || file.GetLength() <= 10 );
			m_dwCancelledFilesSeed = (GetRandomUInt32() % 0xFFFFFFFE) + 1;
		}

		UINT RecordsNumber = file.ReadUInt32();
		for (UINT i = 0; i < RecordsNumber; i++) {
			file.ReadHash16(ucHash);
			uint8 nCount = file.ReadUInt8();
			// for compatibility with future versions which may add more data than just the hash
			for (UINT j = 0; j < nCount; j++) {
				CTag tag(&file, false);
			}
			if (bOldVersion){
				// convert old real hash to new hashash
				uchar pachSeedHash[20];
				PokeUInt32(pachSeedHash, m_dwCancelledFilesSeed);
				md4cpy(pachSeedHash + 4, ucHash);
				MD5Sum md5(pachSeedHash, sizeof(pachSeedHash));
				md4cpy(ucHash, md5.GetRawHash()); 
			}
			m_mapCancelledFiles.SetAt(CSKey(ucHash), 1);
		}
		file.Close();
	}
	catch(CFileException* error){
		if (error->m_cause == CFileException::endOfFile)
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_CONFIGCORRUPT), CANCELLED_MET_FILENAME);
		else{
			TCHAR buffer[MAX_CFEXP_ERRORMSG];
			error->GetErrorMessage(buffer, ARRSIZE(buffer));
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_FAILEDTOLOAD), CANCELLED_MET_FILENAME, buffer);
		}
		error->Delete();
		return false;
	}
	return true;
}

void CKnownFileList::Save()
{
	if (thePrefs.GetLogFileSaving())
		AddDebugLogLine(false, _T("Saving known files list file \"%s\""), KNOWN_MET_FILENAME);
	m_nLastSaved = ::GetTickCount(); 
	CString fullpath = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR);
	fullpath += KNOWN_MET_FILENAME;
	CSafeBufferedFile file;
	CFileException fexp;
	if (!file.Open(fullpath, CFile::modeWrite|CFile::modeCreate|CFile::typeBinary|CFile::shareDenyWrite, &fexp)){
		CString strError(_T("Failed to save ") KNOWN_MET_FILENAME _T(" file"));
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(LOG_STATUSBAR, _T("%s"), strError);
	}
	else{
		setvbuf(file.m_pStream, NULL, _IOFBF, 16384);

		try{
			file.WriteUInt8(0); // we will write the version tag later depending if any large files are on the list
			UINT nRecordsNumber = 0;
			bool bContainsAnyLargeFiles = false;
			file.WriteUInt32(nRecordsNumber);
			POSITION pos = m_Files_map.GetStartPosition();
			while( pos != NULL )
			{
				CKnownFile* pFile;
				CCKey key;
				m_Files_map.GetNextAssoc( pos, key, pFile );
				if (!thePrefs.IsRememberingDownloadedFiles() && !theApp.sharedfiles->IsFilePtrInList(pFile)){
					continue;
				}
				else{
					pFile->WriteToFile(&file);
					nRecordsNumber++;
					if (pFile->IsLargeFile())
						bContainsAnyLargeFiles = true;
				}
			}
			file.SeekToBegin();
			file.WriteUInt8(bContainsAnyLargeFiles ? MET_HEADER_I64TAGS : MET_HEADER);
			file.WriteUInt32(nRecordsNumber);

			if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !theApp.emuledlg->IsRunning())){
				file.Flush(); // flush file stream buffers to disk buffers
				if (_commit(_fileno(file.m_pStream)) != 0) // commit disk buffers to disk
					AfxThrowFileException(CFileException::hardIO, GetLastError(), file.GetFileName());
			}
			file.Close();
		}
		catch(CFileException* error){
			CString strError(_T("Failed to save ") KNOWN_MET_FILENAME _T(" file"));
			TCHAR szError[MAX_CFEXP_ERRORMSG];
			if (error->GetErrorMessage(szError, ARRSIZE(szError))){
				strError += _T(" - ");
				strError += szError;
			}
			LogError(LOG_STATUSBAR, _T("%s"), strError);
			error->Delete();
		}
	}


	if (thePrefs.GetLogFileSaving())
		AddDebugLogLine(false, _T("Saving known files list file \"%s\""), CANCELLED_MET_FILENAME);
 	fullpath = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR);
	fullpath += CANCELLED_MET_FILENAME;
	if (!file.Open(fullpath, CFile::modeWrite|CFile::modeCreate|CFile::typeBinary|CFile::shareDenyWrite, &fexp)){
		CString strError(_T("Failed to save ") CANCELLED_MET_FILENAME _T(" file"));
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(LOG_STATUSBAR, _T("%s"), strError);
	}
	else{
		setvbuf(file.m_pStream, NULL, _IOFBF, 16384);

		try{
			file.WriteUInt8(CANCELLED_HEADER);
			file.WriteUInt8(CANCELLED_VERSION);
			file.WriteUInt32(m_dwCancelledFilesSeed);
			if (!thePrefs.IsRememberingCancelledFiles()){
				file.WriteUInt32(0);
			}
			else{
				UINT nRecordsNumber = m_mapCancelledFiles.GetCount();
				file.WriteUInt32(nRecordsNumber);
				POSITION pos = m_mapCancelledFiles.GetStartPosition();
				while( pos != NULL )
				{
					int dwDummy;
					CSKey key;
					m_mapCancelledFiles.GetNextAssoc( pos, key, dwDummy );
					file.WriteHash16(key.m_key);
					file.WriteUInt8(0);
				}
			}

			if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !theApp.emuledlg->IsRunning())){
				file.Flush(); // flush file stream buffers to disk buffers
				if (_commit(_fileno(file.m_pStream)) != 0) // commit disk buffers to disk
					AfxThrowFileException(CFileException::hardIO, GetLastError(), file.GetFileName());
			}
			file.Close();
		}
		catch(CFileException* error){
			CString strError(_T("Failed to save ") CANCELLED_MET_FILENAME _T(" file"));
			TCHAR szError[MAX_CFEXP_ERRORMSG];
			if (error->GetErrorMessage(szError, ARRSIZE(szError))){
				strError += _T(" - ");
				strError += szError;
			}
			LogError(LOG_STATUSBAR, _T("%s"), strError);
			error->Delete();
		}
	}

	// NEO: NPT - [NeoPartTraffic] -- Xanatos -->
	if (NeoPrefs.UsePartTraffic() || m_bPartTrafficLoaded)
		SavePartTraffic();
	// NEO: NPT END <-- Xanatos --

	SaveKnownPreferences(); // NEO: FCFG - [FileConfiguration] <-- Xanatos --

	// NEO: XCs - [SaveComments] -- Xanatos -->
	if(NeoPrefs.UseSaveComments() || m_bCommentsLoaded)
		SaveComments();
	// NEO: XCs END <-- Xanatos --
}

void CKnownFileList::Clear()
{
	POSITION pos = m_Files_map.GetStartPosition();
	while( pos != NULL )
	{
		CKnownFile* pFile;
		CCKey key;
		m_Files_map.GetNextAssoc( pos, key, pFile );
	    delete pFile;
	}
	m_Files_map.RemoveAll();
}

void CKnownFileList::Process()
{
	// NEO: XCk - [KnownComments] -- Xanatos -->
	if(NeoPrefs.UseKnownComments())
		Publish();
	// NEO: XCk END <-- Xanatos --

	if (::GetTickCount() - m_nLastSaved > MIN2MS(11))
		Save();
}

// NEO: XCk - [KnownComments] -- Xanatos -->
void CKnownFileList::Publish()
{
	// Variables to save cpu.
	UINT tNow = time(NULL);
	bool isFirewalled = theApp.IsFirewalled();

	if( Kademlia::CKademlia::IsConnected() && ( !isFirewalled || ( isFirewalled && theApp.clientlist->GetBuddyStatus() == Connected)) && GetCount() && Kademlia::CKademlia::GetPublish())
	{ 
		if( Kademlia::CKademlia::GetTotalStoreNotes() < KADEMLIATOTALSTORENOTES)
		{
			if(tNow >= theApp.sharedfiles->GetLastPublishKadNotes())
			{
				if(m_currFileNotes >= GetCount())// NEO: FIX
					m_currFileNotes = 0;
				CKnownFile* pCurKnownFile = GetFileByIndex(m_currFileNotes);
				if(pCurKnownFile
				&& pCurKnownFile->Publishable() // NEO: SAFS - [ShowAllFilesInShare] 
				&& !theApp.sharedfiles->IsFilePtrInList(pCurKnownFile)) // file whitch are shared are published by the normal routine
				{
					if(pCurKnownFile->PublishNotes())
					{
						if(Kademlia::CSearchManager::PrepareLookup(Kademlia::CSearch::STORENOTES, true, Kademlia::CUInt128(pCurKnownFile->GetFileHash()))==NULL)
							pCurKnownFile->SetLastPublishTimeKadNotes(0);
					}	
				}
				m_currFileNotes++;

				// even if we did not publish a source, reset the timer so that this list is processed
				// only every KADEMLIAPUBLISHTIME seconds.
				theApp.sharedfiles->SetLastPublishKadNotes(KADEMLIAPUBLISHTIME+tNow); // we cound use a separate counter but // X?
			}
		}
	}
}
CKnownFile* CKnownFileList::GetFileByIndex(int index){
	int count=0;
	CKnownFile* cur_file;
	CCKey bufKey;

	for (POSITION pos = m_Files_map.GetStartPosition();pos != 0;){
		m_Files_map.GetNextAssoc(pos,bufKey,cur_file);
		if (index==count)
			return cur_file;
		count++;
	}
	return 0;
}
// NEO: XCk END <-- Xanatos --

bool CKnownFileList::SafeAddKFile(CKnownFile* toadd)
{
	// NEO: FCFG - [FileConfiguration] -- Xanatos -->
	// In theory stucking with an invalid PartPrefs after this point isn't a problem
	// but just in case lets delete the old object is needed and set the pointer on global default
	if(toadd->IsKindOf(RUNTIME_CLASS(CPartFile))){
		if(((CPartFile*)toadd)->PartPrefs->IsFilePrefs())
			delete ((CPartFile*)toadd)->PartPrefs;
		((CPartFile*)toadd)->PartPrefs = &NeoPrefs.PartPrefs;
	}
	// NEO: FCFG END <-- Xanatos --

	bool bRemovedDuplicateSharedFile = false;
	CCKey key(toadd->GetFileHash());
	CKnownFile* pFileInMap;
	if (m_Files_map.Lookup(key, pFileInMap))
	{
		TRACE(_T("%hs: Already in known list:   %s \"%s\"\n"), __FUNCTION__, md4str(pFileInMap->GetFileHash()), pFileInMap->GetFileName());
		TRACE(_T("%hs: Old entry replaced with: %s \"%s\"\n"), __FUNCTION__, md4str(toadd->GetFileHash()), toadd->GetFileName());

		// if we hash files which are already in known file list and add them later (when the hashing thread is finished),
		// we can not delete any already available entry from known files list. that entry can already be used by the
		// shared file list -> crash.

		m_Files_map.RemoveKey(CCKey(pFileInMap->GetFileHash()));
		//This can happen in a couple situations..
		//File was renamed outside of eMule.. 
		//A user decided to redownload a file he has downloaded and unshared..
		//RemovingKeyWords I believe is not thread safe if I'm looking at this right.
		//Not sure of a good solution yet..
		if (theApp.sharedfiles)
		{
#if 0
			// This may crash the client because of dangling ptr in shared files ctrl.
			// This may happen if a file is re-shared which is also currently downloaded.
			// After the file was downloaded (again) there is a dangl. ptr in shared files 
			// ctrl.
			// Actually that's also wrong in some cases: Keywords are not always removed
			// because the wrong ptr is used to search for in keyword publish list.
			theApp.sharedfiles->RemoveKeywords(pFileInMap);
#else
			// This solves the problem with dangl. ptr in shared files ctrl,
			// but creates a new bug. It may lead to unshared files! Even 
			// worse it may lead to files which are 'shared' in GUI but 
			// which are though not shared 'logically'.
			//
			// To reduce the harm, remove the file from shared files list, 
			// only if really needed. Right now this 'harm' applies for files
			// which are re-shared and then completed (again) because they were
			// also in download queue (they were added there when the already
			// available file was not in shared file list).
			if (theApp.sharedfiles->IsFilePtrInList(pFileInMap))
				bRemovedDuplicateSharedFile = theApp.sharedfiles->RemoveFile(pFileInMap);
#endif
			ASSERT( !theApp.sharedfiles->IsFilePtrInList(pFileInMap) );
		}
		//Double check to make sure this is the same file as it's possible that a two files have the same hash.
		//Maybe in the furture we can change the client to not just use Hash as a key throughout the entire client..
		ASSERT( toadd->GetFileSize() == pFileInMap->GetFileSize() );
		ASSERT( toadd != pFileInMap );
		if (toadd->GetFileSize() == pFileInMap->GetFileSize())
			toadd->statistic.MergeFileStats(&pFileInMap->statistic);

		ASSERT( theApp.sharedfiles==NULL || !theApp.sharedfiles->IsFilePtrInList(pFileInMap) );
		ASSERT( theApp.downloadqueue==NULL || !theApp.downloadqueue->IsPartFile(pFileInMap) );

		// Quick fix: If we downloaded already downloaded files again and if those files all had the same file names
		// and were renamed during file completion, we have a pending ptr in transfer window.
		if (theApp.emuledlg && theApp.emuledlg->transferwnd && theApp.emuledlg->transferwnd->downloadlistctrl.m_hWnd)
			theApp.emuledlg->transferwnd->downloadlistctrl.RemoveFile((CPartFile*)pFileInMap);

		delete pFileInMap;
	}
	m_Files_map.SetAt(key, toadd);
	if (bRemovedDuplicateSharedFile) {
		theApp.sharedfiles->SafeAddKFile(toadd);
	}
	return true;
}

CKnownFile* CKnownFileList::FindKnownFile(LPCTSTR filename, uint32 date, uint64 size) const
{
	POSITION pos = m_Files_map.GetStartPosition();
	while (pos != NULL)
	{
		CKnownFile* cur_file;
		CCKey key;
		m_Files_map.GetNextAssoc(pos, key, cur_file);
		//if (cur_file->GetUtcFileDate() == date && cur_file->GetFileSize() == size && !_tcscmp(filename, cur_file->GetFileName()))
		if (cur_file->GetUtcFileDate() == date && cur_file->GetFileSize() == size && !_tcscmp(filename, cur_file->GetFileName(true))) // NEO: PP - [PasswordProtection] <-- Xanatos --
			return cur_file;
	}
	return NULL;
}

CKnownFile* CKnownFileList::FindKnownFileByPath(const CString& sFilePath) const
{
	POSITION pos = m_Files_map.GetStartPosition();
	while (pos != NULL)
	{
		CKnownFile* cur_file;
		CCKey key;
		m_Files_map.GetNextAssoc(pos, key, cur_file);
		if (!cur_file->GetFilePath().CompareNoCase(sFilePath))
			return cur_file;
	}
	return NULL;
}

CKnownFile* CKnownFileList::FindKnownFileByID(const uchar* hash) const
{
	if (hash)
	{
		CKnownFile* found_file;
		CCKey key(hash);
		if (m_Files_map.Lookup(key, found_file))
			return found_file;
	}
	return NULL;
}

bool CKnownFileList::IsKnownFile(const CKnownFile* file) const
{
	if (file)
		return FindKnownFileByID(file->GetFileHash()) != NULL;
	return false;
}

bool CKnownFileList::IsFilePtrInList(const CKnownFile* file) const
{
	if (file)
	{
		POSITION pos = m_Files_map.GetStartPosition();
		while (pos)
		{
			CCKey key;
			CKnownFile* cur_file;
			m_Files_map.GetNextAssoc(pos, key, cur_file);
			if (file == cur_file)
				return true;
		}
	}
	return false;
}

void CKnownFileList::AddCancelledFileID(const uchar* hash){
	if (thePrefs.IsRememberingCancelledFiles()){
		if (m_dwCancelledFilesSeed == 0) {
			m_dwCancelledFilesSeed = (GetRandomUInt32() % 0xFFFFFFFE) + 1;
		}
		uchar pachSeedHash[20];
		PokeUInt32(pachSeedHash, m_dwCancelledFilesSeed);
		md4cpy(pachSeedHash + 4, hash);
		MD5Sum md5(pachSeedHash, sizeof(pachSeedHash));
		md4cpy(pachSeedHash, md5.GetRawHash()); 
		m_mapCancelledFiles.SetAt(CSKey(pachSeedHash), 1);	
	}
}

bool CKnownFileList::IsCancelledFileByID(const uchar* hash) const
{
	if (thePrefs.IsRememberingCancelledFiles()){
		uchar pachSeedHash[20];
		PokeUInt32(pachSeedHash, m_dwCancelledFilesSeed);
		md4cpy(pachSeedHash + 4, hash);
		MD5Sum md5(pachSeedHash, sizeof(pachSeedHash));
		md4cpy(pachSeedHash, md5.GetRawHash()); 

		int dwDummy;
		if (m_mapCancelledFiles.Lookup(CSKey(pachSeedHash), dwDummy)){
			return true;
		}
	}
	return false;
}

void CKnownFileList::CopyKnownFileMap(CMap<CCKey,const CCKey&,CKnownFile*,CKnownFile*> &Files_Map)
{
	if (!m_Files_map.IsEmpty())
	{
		POSITION pos = m_Files_map.GetStartPosition();
		while (pos)
		{
			CCKey key;
			CKnownFile* cur_file;
			m_Files_map.GetNextAssoc(pos, key, cur_file);
			Files_Map.SetAt(key, cur_file);
		}
	}
}

// NEO: AKF - [AllKnownFiles] -- Xanatos -->
bool CKnownFileList::RemoveFile(CKnownFile* toRemove)
{
	BOOL ret = m_Files_map.RemoveKey(CCKey(toRemove->GetFileHash()));
	if(ret)
		delete toRemove;
	return I2B(ret);
}
// NEO: AKF END <-- Xanatos --

// NEO: NSC - [NeoSharedCategories] -- Xanatos -->
void CKnownFileList::ResetCatParts(UINT cat)
{
	POSITION pos = m_Files_map.GetStartPosition();
	while( pos != NULL )
	{
		CKnownFile* pFile;
		CCKey key;
		m_Files_map.GetNextAssoc( pos, key, pFile );

		if (pFile->GetCategory()==cat)
			pFile->SetCategory(0);
		else if (pFile->GetCategory() > cat)
			pFile->SetCategory(pFile->GetCategory() - 1);
	}
}

void CKnownFileList::ShiftCatParts(UINT cat)
{
	POSITION pos = m_Files_map.GetStartPosition();
	while( pos != NULL )
	{
		CKnownFile* pFile;
		CCKey key;
		m_Files_map.GetNextAssoc( pos, key, pFile );

		if (pFile->GetCategory()>=cat)
			pFile->SetCategory(pFile->GetCategory() + 1);			
	}
}

void CKnownFileList::MoveCat(UINT from, uint8 to)
{
	if (from < to)
		--to;

	POSITION pos = m_Files_map.GetStartPosition();
	while( pos != NULL )
	{
		CKnownFile* pFile;
		CCKey key;
		m_Files_map.GetNextAssoc( pos, key, pFile );

		if (!pFile)
			continue;

		UINT mycat = pFile->GetCategory();
		if ((mycat>=min(from,to) && mycat<=max(from,to)))
		{
			//if ((from<to && (mycat<from || mycat>to)) || (from>to && (mycat>from || mycat<to)) )	continue; //not affected

			if (mycat == from)
				pFile->SetCategory(to);
			else{
				if (from < to)
					pFile->SetCategory(mycat - 1);
				else
					pFile->SetCategory(mycat + 1);
			}
		}
	}
}
// NEO: NSC END <-- Xanatos --

// NEO: FCFG - [FileConfiguration] -- Xanatos -->
void CKnownFileList::UpdateKnownPrefs(CKnownPreferences* KnownPrefs, UINT cat)
{
	POSITION pos = m_Files_map.GetStartPosition();
	while (pos)
	{
		CCKey key;
		CKnownFile* cur_file;
		m_Files_map.GetNextAssoc(pos, key, cur_file);
		if (!cur_file)
			continue;

		CKnownFileList::UpdateKnownPrefs(cur_file, KnownPrefs, cat);
	}

	Category_Struct* Category = thePrefs.GetCategory(cat);
	ASSERT(Category);
	if(!KnownPrefs->IsEmpty() && KnownPrefs != Category->KnownPrefs){
		((CKnownPreferencesEx*)KnownPrefs)->KnownPrefs = &NeoPrefs.KnownPrefs;
		Category->KnownPrefs = KnownPrefs;
	}else if(KnownPrefs->IsEmpty()){
		delete KnownPrefs;
		Category->KnownPrefs = NULL;
	}
}

void CKnownFileList::UpdateKnownPrefs(CKnownFile* cur_file, CKnownPreferences* KnownPrefs, UINT cat)
{
	if(!KnownPrefs->IsEmpty())
	{
		if(cur_file->GetCategory() == cat && cur_file->KnownPrefs != KnownPrefs)
		{
			ASSERT(!cur_file->KnownPrefs->IsCategoryPrefs());

			if(cur_file->KnownPrefs->IsFilePrefs()) 
			{
				((CKnownPreferencesEx*)cur_file->KnownPrefs)->KnownPrefs = KnownPrefs;
			}
			else //if(cur_file->KnownPrefs->IsGlobalPrefs()) 
			{
				cur_file->KnownPrefs = KnownPrefs;
			}
		}
	}
	else if(cur_file->KnownPrefs == KnownPrefs) // && KnownPrefs->IsEmpty()
	{
		cur_file->KnownPrefs = &NeoPrefs.KnownPrefs;
	}
	else if(((CKnownPreferencesEx*)cur_file->KnownPrefs)->KnownPrefs == KnownPrefs) // && KnownPrefs->IsEmpty()
	{
		((CKnownPreferencesEx*)cur_file->KnownPrefs)->KnownPrefs = &NeoPrefs.KnownPrefs;
	}
}
// NEO: FCFG END <-- Xanatos --

// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
//Xman [MoNKi: -Check already downloaded files-]
// returns:
//		1 if a file was found
//		2 if more than 1 file found or only the name is equal.
//		0 if no file found
//		3 if cancelled file
int CKnownFileList::CheckAlreadyDownloadedFile(const uchar* hash, CString filename, CArray<CKnownFile*,CKnownFile*> *files)
{
	files->RemoveAll();
	if ( hash != NULL )
	{
		CKnownFile* curFile = FindKnownFileByID(hash);
		if ( curFile && !curFile->IsPartFile() )	
		{
			files->Add(curFile);
			return 1;
		}
		else if(curFile==NULL && IsCancelledFileByID(hash) )
		{
				return 3; 
		}
		else
			return 0;
	}
	else if ( !filename.IsEmpty() )
	{
		POSITION pos = m_Files_map.GetStartPosition();
		while ( pos )
		{
			CCKey key;
			CKnownFile* curFile;
			m_Files_map.GetNextAssoc(pos, key, curFile);
			if ( filename == curFile->GetFileName() && !curFile->IsPartFile() )
				files->Add(curFile);
		}
		if ( files->IsEmpty() )
			return 0;
		else
			return 2;
	}
	return 0;
}

//Returns:
//	true if you can download it
bool CKnownFileList::CheckAlreadyDownloadedFileQuestion(const uchar* hash, CString filename){
	CArray<CKnownFile *,CKnownFile*> filesFound;
	int ret;

	if(theApp.downloadqueue->IsFileExisting(hash)){
		return false;
	}

	ret=CheckAlreadyDownloadedFile(hash, _T(""), &filesFound);
	if(ret==0)
		ret=CheckAlreadyDownloadedFile(NULL,filename, &filesFound);

	if(ret!=0)
	{
		//CKnownFile* curFile=NULL;
		CString msg;
		if(ret==1){
			msg = GetResString(IDS_X_DOWNHISTORY_CHECK1);
		}
		else if(ret==3)
		{
			msg= GetResString(IDS_X_DOWNHISTORY_CHECK4);
		}
		else {
			msg.Format(GetResString(IDS_X_DOWNHISTORY_CHECK2), filesFound.GetCount(), filename);
		}

		for(int i=0;i<filesFound.GetCount();i++){
			CKnownFile *cur_file = filesFound.GetAt(i);
			CString sData;

			msg+=cur_file->GetFileName() + _T("\n");
			sData.Format(GetResString(IDS_DL_SIZE) + _T(": %I64u, ") + GetResString(IDS_FILEID) + _T(": %s"), cur_file->GetFileSize(), EncodeBase16(cur_file->GetFileHash(),16));
			msg+=sData;
			if(!cur_file->GetFileComment().IsEmpty()) //Add comment
				msg+=_T("\n") + GetResString(IDS_COMMENT) + _T(": \"") + cur_file->GetFileComment() + _T("\"");
			msg+="\n\n";
		}
		msg += GetResString(IDS_X_DOWNHISTORY_CHECK3);
		if(MessageBox(NULL, msg, GetResString(IDS_X_DOWNHISTORY),MB_YESNO|MB_ICONQUESTION)==IDYES)
			return true;
		else
			return false;
	}
	else
	{
		return true;
	}
}
// NEO: NXC END <-- Xanatos --

// NEO: XCs - [SaveComments] -- Xanatos -->
bool CKnownFileList::LoadComments()
{
	m_bCommentsLoaded = true;

	CString fullpath=thePrefs.GetMuleDirectory(EMULE_CONFIGDIR);
	fullpath.Append(COMMENTS_MET_FILENAME);
	CSafeBufferedFile file;
	CFileException fexp;
	if (!file.Open(fullpath,CFile::modeRead|CFile::osSequentialScan|CFile::typeBinary|CFile::shareDenyWrite, &fexp)){
		if (fexp.m_cause != CFileException::fileNotFound){
			CString strError(_T("Failed to load ") COMMENTS_MET_FILENAME _T(" file"));
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

	try {
		uint8 version = file.ReadUInt8();
		if (version != COMMENTSFILE_VERSION /*version < COMMENTSFILE_VERSION_OLD || version > COMMENTSFILE_VERSION*/){
			ModLogError(GetResString(IDS_X_ERR_COMMENTSMET_UNKNOWN_VERSION));
			file.Close();
			return false;
		}

		UINT RecordsNumber = file.ReadUInt32();
		uchar cur_hash[16];
		CKnownFile* cuf_file=NULL;
		for (UINT i = 0; i < RecordsNumber; i++) {
			file.ReadHash16(cur_hash);
			if((cuf_file = FindKnownFileByID(cur_hash)) != NULL){
				if(!cuf_file->LoadComments(&file)){
					ModLogError(GetResString(IDS_X_ERR_COMMENTSMET_ENTRY_CORRUPT), cuf_file->GetFileName());
				}
			}else{
				ModLogError(GetResString(IDS_X_ERR_COMMENTSASYNCHRONIZED), md4str(cur_hash)); 
				ClearCommentsEntry(&file);
			}
		}
		file.Close();
	}
	catch(CFileException* error){
		if (error->m_cause == CFileException::endOfFile){
			ModLogError(LOG_STATUSBAR, GetResString(IDS_X_ERR_COMMENTSMET_BAD));
		}else{
			TCHAR buffer[MAX_CFEXP_ERRORMSG];
			error->GetErrorMessage(buffer,MAX_CFEXP_ERRORMSG);
			ModLogError(LOG_STATUSBAR, GetResString(IDS_X_ERR_COMMENTSMET_UNKNOWN),buffer);
		}
		error->Delete();
		return false;
	}

	return true;
}

bool CKnownFileList::ClearCommentsEntry(CFileDataIO* file){
	uint16 count = file->ReadUInt16();

	ULONG str = 0;

	for(uint16 i = 0; i < count; i++)
	{
		file->Seek(16,CFile::current); // UH

		str=file->ReadUInt16();
		file->Seek(str,CFile::current); // UN
		str=file->ReadUInt16();
		file->Seek(str,CFile::current); // FN
		
		file->Seek(1,CFile::current); // R
		str=file->ReadUInt16();
		file->Seek(str,CFile::current); // C
	}

	return true;
}

bool CKnownFileList::SaveComments()
{
	if (thePrefs.GetLogFileSaving())
		AddDebugLogLine(false,_T("Saving Comments files list file \"%s\""), COMMENTS_MET_FILENAME);
	CString fullpath=thePrefs.GetMuleDirectory(EMULE_CONFIGDIR);
	fullpath += COMMENTS_MET_FILENAME;
	CSafeBufferedFile file;
	CFileException fexp;
	if (!file.Open(fullpath, CFile::modeWrite|CFile::modeCreate|CFile::typeBinary|CFile::shareDenyWrite, &fexp)){
		CString strError(_T("Failed to save ") COMMENTS_MET_FILENAME _T(" file"));
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		ModLogError(LOG_STATUSBAR, _T("%s"), strError);
		return false;
	}
	setvbuf(file.m_pStream, NULL, _IOFBF, 16384);

	try{
		file.WriteUInt8(COMMENTSFILE_VERSION);

		uint32 uTagCount = 0;
		ULONG uTagCountFilePos = (ULONG)file.GetPosition();
		file.WriteUInt32(uTagCount);

		POSITION pos = m_Files_map.GetStartPosition();
		while( pos != NULL )
		{
			CKnownFile* pFile;
			CCKey key;
			m_Files_map.GetNextAssoc( pos, key, pFile );
			if(pFile->HasComments()){

				file.WriteHash16(pFile->GetFileHash());
				pFile->SaveComments(&file);
				uTagCount++;
			}

		}

		file.Seek(uTagCountFilePos, CFile::begin);
		file.WriteUInt32(uTagCount);
		file.Seek(0, CFile::end);

		if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !theApp.emuledlg->IsRunning())){
			file.Flush(); // flush file stream buffers to disk buffers
			if (_commit(_fileno(file.m_pStream)) != 0) // commit disk buffers to disk
				AfxThrowFileException(CFileException::hardIO, GetLastError(), file.GetFileName());
		}
		file.Close();
	}
	catch(CFileException* error){
		CString strError(_T("Failed to save ") COMMENTS_MET_FILENAME _T(" file"));
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (error->GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		ModLogError(LOG_STATUSBAR, _T("%s"), strError);
		error->Delete();
	}

	return true;
}
// NEO: XCs END <-- Xanatos --
