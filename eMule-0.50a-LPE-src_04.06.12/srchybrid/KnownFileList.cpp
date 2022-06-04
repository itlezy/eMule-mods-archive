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
//Xman x4.1.1 
//Xman [MoNKi: -Downloaded History-]
#include "SharedFilesCtrl.h"
#include "PartFile.h" //to be able to remove it from transferwindow if necessary

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define KNOWN_MET_FILENAME		_T("known.met")
#define CANCELLED_MET_FILENAME	_T("cancelled.met")

#define CANCELLED_HEADER_OLD	MET_HEADER
#define CANCELLED_HEADER		MET_HEADER + 0x01
#define CANCELLED_VERSION		0x01

CKnownFileList::CKnownFileList()
// ==> Threaded Known Files Saving [Stulle] - Stulle
 : Thread(false, PRIO_LOWEST)
// <== Threaded Known Files Saving [Stulle] - Stulle
{
#ifndef REPLACE_ATLMAP
	//Xman Init-Hashtable optimization
	//m_Files_map.InitHashTable(2063); //moved down
	//m_mapCancelledFiles.InitHashTable(DEFAULT_FILES_TABLE_SIZE);
#endif
	accepted = 0;
	requested = 0;
	transferred = 0;
	m_nRequestedTotal = 0;
	m_nAcceptedTotal = 0;
	m_nTransferredTotal = 0;
	m_dwCancelledFilesSeed = 0;
	m_nLastSaved = ::GetTickCount();
	Init();
}

CKnownFileList::~CKnownFileList()
{
	Clear();
}

bool CKnownFileList::Init()
{
	return LoadKnownFiles() && LoadCancelledFiles();
}

bool CKnownFileList::LoadKnownFiles()
{
#ifdef REPLACE_ATLMAP
	KnonwFilesByAICHMap(DEFAULT_FILES_TABLE_SIZE).swap(m_mapKnownFilesByAICH);
#else
	m_mapKnownFilesByAICH.InitHashTable(DEFAULT_FILES_TABLE_SIZE);
#endif
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
#ifdef REPLACE_ATLMAP
		CKnownFilesMap(DEFAULT_FILES_TABLE_SIZE).swap(m_Files_map);
#else
		//Xman Init-Hashtable optimization
		m_Files_map.InitHashTable(DEFAULT_FILES_TABLE_SIZE);
		//Xman end
#endif
		return false;
	}
	ASSERT((MET_HEADER & I64TIMEMASK) == 0);// X: [E64T] - [Enable64BitTime]
	ASSERT((MET_HEADER_I64TAGS & I64TIMEMASK) == 0);

	setvbuf(file.m_pStream, NULL, _IOFBF, 16384);

	CKnownFile* pRecord = NULL;
	try {
		uint8 header = file.ReadUInt8();
		bool I64Time = ((header & I64TIMEMASK) != 0);// X: [E64T] - [Enable64BitTime]
		header&=~I64TIMEMASK;
		if (header != MET_HEADER && header != MET_HEADER_I64TAGS){
			file.Close();
#ifdef REPLACE_ATLMAP
			CKnownFilesMap(DEFAULT_FILES_TABLE_SIZE).swap(m_Files_map);
#else
			//Xman Init-Hashtable optimization
			m_Files_map.InitHashTable(DEFAULT_FILES_TABLE_SIZE);
			//Xman end
#endif

			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_SERVERMET_BAD));
			return false;
		}
		AddDebugLogLine(false, _T("Known.met file version is %u (%s support 64bit tags)"), header, (header == MET_HEADER) ? _T("doesn't") : _T("does")); 

		UINT RecordsNumber = file.ReadUInt32();

#ifdef REPLACE_ATLMAP
		CKnownFilesMap(size_t(RecordsNumber*1.2f + DEFAULT_FILES_TABLE_SIZE)).swap(m_Files_map);
#else
		//Xman Init-Hashtable optimization
		m_Files_map.InitHashTable(UINT(RecordsNumber*1.2f + DEFAULT_FILES_TABLE_SIZE));
		//Xman end
#endif

		for (UINT i = 0; i < RecordsNumber; i++) {
			pRecord = new CKnownFile();
			if (!pRecord->LoadFromFile(&file,I64Time)){// X: [E64T] - [Enable64BitTime]
				TRACE(_T("*** Failed to load entry %u (name=%s  hash=%s  size=%I64u  parthashs=%u expected parthashs=%u) from known.met\n"), i, 
					pRecord->GetFileName(), md4str(pRecord->GetFileHash()), pRecord->GetFileSize()
					, pRecord->GetFileIdentifier().GetAvailableMD4PartHashCount(), pRecord->GetFileIdentifier().GetTheoreticalMD4PartHashCount());
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
#ifdef REPLACE_ATLMAP
		if(m_Files_map.bucket_count()<DEFAULT_FILES_TABLE_SIZE)
			CKnownFilesMap(DEFAULT_FILES_TABLE_SIZE).swap(m_Files_map);
#else
		//Xman Init-Hashtable optimization
		if(m_Files_map.GetHashTableSize()<DEFAULT_FILES_TABLE_SIZE)
			m_Files_map.InitHashTable(DEFAULT_FILES_TABLE_SIZE);
		//Xman end
#endif

		return false;
	}

	return true;
}

bool CKnownFileList::LoadCancelledFiles(){
	// cancelled.met Format: <Header 1 = CANCELLED_HEADER><Version 1 = CANCELLED_VERSION><Seed 4><Count 4>[<HashHash 16><TagCount 1>[Tags TagCount] Count]
	if (!thePrefs.IsRememberingCancelledFiles())
	{
#ifdef REPLACE_ATLMAP
		CancelledFilesMap(DEFAULT_CANCELLED_TABLE_SIZE).swap(m_mapCancelledFiles);
#else
		//Xman Init-Hashtable optimization
		m_mapCancelledFiles.InitHashTable(DEFAULT_CANCELLED_TABLE_SIZE);
		//Xman end
#endif
		return true;
	}
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
#ifdef REPLACE_ATLMAP
		CancelledFilesMap(DEFAULT_CANCELLED_TABLE_SIZE).swap(m_mapCancelledFiles);
#else
		//Xman Init-Hashtable optimization
		m_mapCancelledFiles.InitHashTable(DEFAULT_CANCELLED_TABLE_SIZE);
		//Xman end
#endif
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
#ifdef REPLACE_ATLMAP
				CancelledFilesMap(DEFAULT_CANCELLED_TABLE_SIZE).swap(m_mapCancelledFiles);
#else
				//Xman Init-Hashtable optimization
				m_mapCancelledFiles.InitHashTable(DEFAULT_CANCELLED_TABLE_SIZE);
				//Xman end
#endif
				return false;
			}
		}
		uint8 byVersion = 0;
		if (!bOldVersion){
			byVersion = file.ReadUInt8();
			if (byVersion > CANCELLED_VERSION){
				file.Close();
#ifdef REPLACE_ATLMAP
				CancelledFilesMap(DEFAULT_CANCELLED_TABLE_SIZE).swap(m_mapCancelledFiles);
#else
				//Xman Init-Hashtable optimization
				m_mapCancelledFiles.InitHashTable(DEFAULT_CANCELLED_TABLE_SIZE);
				//Xman end
#endif
				return false;
			}

			m_dwCancelledFilesSeed = file.ReadUInt32();
		}
		while (m_dwCancelledFilesSeed == 0) {
			ASSERT( bOldVersion || file.GetLength() <= 10 );
			m_dwCancelledFilesSeed = t_rng->getUInt32();
		}

		UINT RecordsNumber = file.ReadUInt32();
#ifdef REPLACE_ATLMAP
		CancelledFilesMap(size_t(RecordsNumber * 1.2f + DEFAULT_CANCELLED_TABLE_SIZE)).swap(m_mapCancelledFiles);
#else
		//Xman Init-Hashtable optimization
		m_mapCancelledFiles.InitHashTable(UINT(RecordsNumber * 1.2f + DEFAULT_CANCELLED_TABLE_SIZE));
		//Xman end
#endif
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
#ifdef REPLACE_ATLMAP
			m_mapCancelledFiles[ucHash] = 1;
#else
			m_mapCancelledFiles.SetAt(CSKey(ucHash), 1);
#endif
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
#ifdef REPLACE_ATLMAP
		if(m_mapCancelledFiles.bucket_count()<DEFAULT_CANCELLED_TABLE_SIZE)
			CancelledFilesMap(DEFAULT_CANCELLED_TABLE_SIZE).swap(m_mapCancelledFiles);
#else
		//Xman Init-Hashtable optimization
		if(m_mapCancelledFiles.GetHashTableSize()<DEFAULT_CANCELLED_TABLE_SIZE)
			m_mapCancelledFiles.InitHashTable(DEFAULT_CANCELLED_TABLE_SIZE);
		//Xman end
#endif
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
			file.Seek(5,CFile::begin); // we will write the version tag later depending if any large files are on the list
			UINT nRecordsNumber = 0;
			bool bContainsAnyLargeFiles = false;
			bool I64Time=thePrefs.m_bEnable64BitTime;// X: [E64T] - [Enable64BitTime]
#ifdef REPLACE_ATLMAP
			for (CKnownFilesMap::const_iterator it = m_Files_map.begin(); it != m_Files_map.end(); ++it)
			{
				CKnownFile* pFile = it->second;
#else
			POSITION pos = m_Files_map.GetStartPosition();
			while( pos != NULL )
			{
				CKnownFile* pFile;
				CCKey key;
				m_Files_map.GetNextAssoc( pos, key, pFile );
#endif
				if (!thePrefs.IsRememberingDownloadedFiles() && !theApp.sharedfiles->IsFilePtrInList(pFile)){
					continue;
				}
				else{
					pFile->WriteToFile(&file,I64Time);// X: [E64T] - [Enable64BitTime]
					nRecordsNumber++;
					if (pFile->IsLargeFile())
						bContainsAnyLargeFiles = true;
				}
			}
			file.SeekToBegin();
			uint8 uVersion=bContainsAnyLargeFiles ? MET_HEADER_I64TAGS : MET_HEADER;// X: [E64T] - [Enable64BitTime]
			if(I64Time)
				uVersion|=I64TIMEMASK;
			file.WriteUInt8(uVersion);
			file.WriteUInt32(nRecordsNumber);

			if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !CemuleDlg::IsRunning())){
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
#ifdef REPLACE_ATLMAP
				UINT nRecordsNumber = (UINT)m_mapCancelledFiles.size();
				file.WriteUInt32(nRecordsNumber);
				for(CancelledFilesMap::const_iterator it = m_mapCancelledFiles.begin(); it != m_mapCancelledFiles.end(); ++it)
				{
					file.WriteHash16(it->first.m_key);
					file.WriteUInt8(0);
				}
#else
				UINT nRecordsNumber = (UINT)m_mapCancelledFiles.GetCount();
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
#endif
			}

			if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !CemuleDlg::IsRunning())){
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
}

void CKnownFileList::Clear()
{
#ifdef REPLACE_ATLMAP
	m_mapKnownFilesByAICH.clear();
	for (CKnownFilesMap::const_iterator it = m_Files_map.begin(); it != m_Files_map.end(); ++it)
		delete it->second;
	m_Files_map.clear();
#else
	m_mapKnownFilesByAICH.RemoveAll();
	POSITION pos = m_Files_map.GetStartPosition();
	while( pos != NULL )
	{
		CKnownFile* pFile;
		CCKey key;
		m_Files_map.GetNextAssoc( pos, key, pFile );
	    delete pFile;
	}
	m_Files_map.RemoveAll();
#endif
}

void CKnownFileList::Process()
{
	if (::GetTickCount() - m_nLastSaved > MIN2MS(11))
		// ==> Threaded Known Files Saving [Stulle] - Stulle
		/*
		Save();
		*/
		Thread::start();
		// <== Threaded Known Files Saving [Stulle] - Stulle
}

bool CKnownFileList::SafeAddKFile(CKnownFile* toadd)
{
	bool bRemovedDuplicateSharedFile = false;
	CKnownFile* pFileInMap;
#ifdef REPLACE_ATLMAP
	CKnownFilesMap::const_iterator it = m_Files_map.find(toadd->GetFileHash());
	if(it != m_Files_map.end())
	{
		pFileInMap = it->second;
#else
	CCKey key(toadd->GetFileHash());
	if (m_Files_map.Lookup(key, pFileInMap))
	{
#endif
		TRACE(_T("%hs: Already in known list:   %s %I64u \"%s\"\n"), __FUNCTION__, md4str(pFileInMap->GetFileHash()), pFileInMap->GetFileSize(), pFileInMap->GetFileName());
		TRACE(_T("%hs: Old entry replaced with: %s %I64u \"%s\"\n"), __FUNCTION__, md4str(toadd->GetFileHash()), toadd->GetFileSize(), toadd->GetFileName());

		// if we hash files which are already in known file list and add them later (when the hashing thread is finished),
		// we can not delete any already available entry from known files list. that entry can already be used by the
		// shared file list -> crash.
#ifdef REPLACE_ATLMAP
		m_Files_map.erase(it);
		m_mapKnownFilesByAICH.erase(pFileInMap->GetFileIdentifier().GetAICHHash());
#else
		m_Files_map.RemoveKey(CCKey(pFileInMap->GetFileHash()));
		m_mapKnownFilesByAICH.RemoveKey(pFileInMap->GetFileIdentifier().GetAICHHash());
#endif
		//This can happen in a couple situations..
		//File was renamed outside of eMule.. 
		//A user decided to redownload a file he has downloaded and unshared..
		if (theApp.sharedfiles)
		{
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
			ASSERT( !theApp.sharedfiles->IsFilePtrInList(pFileInMap) );
		}
		//Double check to make sure this is the same file as it's possible that a two files have the same hash.
		//Maybe in the furture we can change the client to not just use Hash as a key throughout the entire client..
		ASSERT( toadd->GetFileSize() == pFileInMap->GetFileSize() );
		ASSERT( toadd != pFileInMap );
		if (toadd->GetFileSize() == pFileInMap->GetFileSize())
		{ //Xman
			pFileInMap->CheckAUPFilestats(false); //Xman advanced upload-priority
			toadd->statistic.MergeFileStats(&pFileInMap->statistic);
		} //Xman

		ASSERT( theApp.sharedfiles==NULL || !theApp.sharedfiles->IsFilePtrInList(pFileInMap) );
		ASSERT( theApp.downloadqueue==NULL || !theApp.downloadqueue->IsPartFile(pFileInMap) );

		// Quick fix: If we downloaded already downloaded files again and if those files all had the same file names
		// and were renamed during file completion, we have a pending ptr in transfer window.
		if (theApp.emuledlg && theApp.emuledlg->transferwnd)
		{
			if(theApp.emuledlg->transferwnd->downloadlistctrl.m_hWnd)
			theApp.emuledlg->transferwnd->downloadlistctrl.RemoveFile((CPartFile*)pFileInMap);
			// Make sure the file is not used in out sharedfilesctrl anymore
			if (theApp.emuledlg->transferwnd->sharedfilesctrl.m_hWnd)
				theApp.emuledlg->transferwnd->sharedfilesctrl.RemoveFile(pFileInMap, true);

		//Xman [MoNKi: -Downloaded History-]
		theApp.emuledlg->transferwnd->historylistctrl.RemoveFileFromView(pFileInMap);
		//Xman end
		}

		delete pFileInMap;
	}
#ifdef REPLACE_ATLMAP
	m_Files_map[toadd->GetFileHash()] = toadd;
#else
	m_Files_map.SetAt(key, toadd);
#endif
	if (bRemovedDuplicateSharedFile) {
		theApp.sharedfiles->SafeAddKFile(toadd);
	}
	if (toadd->GetFileIdentifier().HasAICHHash())
#ifdef REPLACE_ATLMAP
		m_mapKnownFilesByAICH[toadd->GetFileIdentifier().GetAICHHash()] = toadd;
#else
		m_mapKnownFilesByAICH.SetAt(toadd->GetFileIdentifier().GetAICHHash(), toadd);
#endif
	return true;
}

CKnownFile* CKnownFileList::FindKnownFile(LPCTSTR filename, uint64 date, uint64 size) const// X: [E64T] - [Enable64BitTime]
{
#ifdef REPLACE_ATLMAP
	for (CKnownFilesMap::const_iterator it = m_Files_map.begin(); it != m_Files_map.end(); ++it)
	{
		CKnownFile* cur_file = it->second;
#else
	POSITION pos = m_Files_map.GetStartPosition();
	while (pos != NULL)
	{
		CKnownFile* cur_file;
		CCKey key;
		m_Files_map.GetNextAssoc(pos, key, cur_file);
#endif
		if (cur_file->GetUtcFileDate() == date && cur_file->GetFileSize() == size && !_tcscmp(filename, cur_file->GetFileName()))
			return cur_file;
	}
	return NULL;
}

CKnownFile* CKnownFileList::FindKnownFileByPath(const CString& sFilePath) const
{
#ifdef REPLACE_ATLMAP
	for (CKnownFilesMap::const_iterator it = m_Files_map.begin(); it != m_Files_map.end(); ++it)
	{
		CKnownFile* cur_file = it->second;
#else
	POSITION pos = m_Files_map.GetStartPosition();
	while (pos != NULL)
	{
		CKnownFile* cur_file;
		CCKey key;
		m_Files_map.GetNextAssoc(pos, key, cur_file);
#endif
		if (!cur_file->GetFilePath().CompareNoCase(sFilePath))
			return cur_file;
	}
	return NULL;
}

CKnownFile* CKnownFileList::FindKnownFileByID(const uchar* hash) const
{
	if (hash)
	{
#ifdef REPLACE_ATLMAP
		CKnownFilesMap::const_iterator it = m_Files_map.find(hash);
		if(it != m_Files_map.end())
			return it->second;
#else
		CKnownFile* found_file;
		CCKey key(hash);
		if (m_Files_map.Lookup(key, found_file))
			return found_file;
#endif
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
		CKnownFile* file2 = FindKnownFileByID(file->GetFileHash());// X: [CI] - [Code Improvement]
		if(file2 == NULL)
			return false;
		if(file2 == file)
			return true;

#ifdef REPLACE_ATLMAP
		for (CKnownFilesMap::const_iterator it = m_Files_map.begin(); it != m_Files_map.end(); ++it)
		{
			if (file == it->second)
				return true;
		}
#else
		POSITION pos = m_Files_map.GetStartPosition();
		while (pos)
		{
			CCKey key;
			CKnownFile* cur_file;
			m_Files_map.GetNextAssoc(pos, key, cur_file);
			if (file == cur_file)
				return true;
		}
#endif
	}
	return false;
}

void CKnownFileList::AddCancelledFileID(const uchar* hash){
	if (thePrefs.IsRememberingCancelledFiles()){
		while (m_dwCancelledFilesSeed == 0) {
			m_dwCancelledFilesSeed = t_rng->getUInt32();
		}
		uchar pachSeedHash[20];
		PokeUInt32(pachSeedHash, m_dwCancelledFilesSeed);
		md4cpy(pachSeedHash + 4, hash);
		MD5Sum md5(pachSeedHash, sizeof(pachSeedHash));
		//md4cpy(pachSeedHash, md5.GetRawHash()); 
#ifdef REPLACE_ATLMAP
		m_mapCancelledFiles[CSKey(md5.GetRawHash())] = 1;	
#else
		m_mapCancelledFiles.SetAt(CSKey(md5.GetRawHash()), 1);	
#endif
	}
}

bool CKnownFileList::IsCancelledFileByID(const uchar* hash) const
{
	if (thePrefs.IsRememberingCancelledFiles()){
		uchar pachSeedHash[20];
		PokeUInt32(pachSeedHash, m_dwCancelledFilesSeed);
		md4cpy(pachSeedHash + 4, hash);
		MD5Sum md5(pachSeedHash, sizeof(pachSeedHash));
		//md4cpy(pachSeedHash, md5.GetRawHash()); 

#ifdef REPLACE_ATLMAP
		if(m_mapCancelledFiles.find(CSKey(md5.GetRawHash())) != m_mapCancelledFiles.end()){
			return true;
		}
#else
		int dwDummy;
		if (m_mapCancelledFiles.Lookup(CSKey(md5.GetRawHash()), dwDummy)){
			return true;
		}
#endif
	}
	return false;
}

//Xman [MoNKi: -Check already downloaded files-]
// returns:
//		1 if a file was found
//		2 if more than 1 file found or only the name is equal.
//		0 if no file found
//		3 if cancelled file
sint_ptr CKnownFileList::CheckAlreadyDownloadedFile(const uchar* hash, CString filename, CAtlArray<CKnownFile*> *files) const
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
#ifdef REPLACE_ATLMAP
		for (CKnownFilesMap::const_iterator it = m_Files_map.begin(); it != m_Files_map.end(); ++it)
		{
			CKnownFile* curFile = it->second;
#else
		POSITION pos = m_Files_map.GetStartPosition();
		while ( pos )
		{
			CCKey key;
			CKnownFile* curFile;
			m_Files_map.GetNextAssoc(pos, key, curFile);
#endif
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
bool CKnownFileList::CheckAlreadyDownloadedFileQuestion(const uchar* hash, CString filename) const{
	if(theApp.downloadqueue->IsFileExisting(hash))
		return false;

	CAtlArray<CKnownFile *> filesFound;
	sint_ptr ret = CheckAlreadyDownloadedFile(hash, _T(""), &filesFound);
	if(ret==0){
		ret=CheckAlreadyDownloadedFile(NULL,filename, &filesFound);
	if(ret==0)
			return true;
	}
	//CKnownFile* curFile=NULL;
	CString msg;
	if(ret==1)
		msg = GetResString(IDS_DOWNHISTORY_CHECK1);
	else if(ret==3)
		msg = GetResString(IDS_DOWNHISTORY_CHECK4);
	else
		msg.Format(GetResString(IDS_DOWNHISTORY_CHECK2), filesFound.GetCount(), filename);

	for(size_t i=0;i<filesFound.GetCount();i++){
		CKnownFile *cur_file = filesFound.GetAt(i);
		CString sData;

		msg+=cur_file->GetFileName() + _T('\n');
		sData.Format(GetResString(IDS_DL_SIZE) + _T(": %I64u, ") + GetResString(IDS_FILEID) + _T(": %s\n\n"), (uint64)cur_file->GetFileSize(), md4str(cur_file->GetFileHash()));
		msg+=sData;
	}
	msg += GetResString(IDS_DOWNHISTORY_CHECK3);
	return (MessageBox(NULL, msg, GetResString(IDS_DOWNHISTORY),MB_YESNO|MB_ICONQUESTION)==IDYES);
}
//Xman end

bool CKnownFileList::RemoveKnownFile(CKnownFile *toRemove){
	if (toRemove){
	// ==> Threaded Known Files Saving [Stulle] - Stulle
		join();
	// <== Threaded Known Files Saving [Stulle] - Stulle
#ifdef REPLACE_ATLMAP
		for (CKnownFilesMap::const_iterator it = m_Files_map.begin(); it != m_Files_map.end(); ++it)
		{
			CKnownFile* cur_file = it->second;
			if (toRemove == cur_file){
				m_Files_map.erase(it);
				delete cur_file;
				return true;
			}
		}
#else
		POSITION pos = m_Files_map.GetStartPosition();
		while (pos){
			CCKey key;
			CKnownFile* cur_file;
			m_Files_map.GetNextAssoc(pos, key, cur_file);
			if (toRemove == cur_file){
				m_Files_map.RemoveKey(key);
				delete cur_file;
				return true;
			}
		}
#endif
	}
	return false;
}

void CKnownFileList::ClearHistory(){
	// ==> Threaded Known Files Saving [Stulle] - Stulle
	join();
	// <== Threaded Known Files Saving [Stulle] - Stulle
#ifdef REPLACE_ATLMAP
	for (CKnownFilesMap::const_iterator it = m_Files_map.begin(); it != m_Files_map.end();)
	{
		CKnownFile* cur_file = it->second;
		if (theApp.sharedfiles->IsFilePtrInList(cur_file))
			++it;
		else{
			it = m_Files_map.erase(it);
#else
	POSITION pos = m_Files_map.GetStartPosition();					
	while(pos){
		CKnownFile* cur_file;
		CCKey key;
		m_Files_map.GetNextAssoc( pos, key, cur_file );
		if (!theApp.sharedfiles->IsFilePtrInList(cur_file)){
			m_Files_map.RemoveKey(key);
#endif
			delete cur_file;
		}
	}	
}

//Xman end

// Save known thread to avoid locking GUI
void CKnownFileList::run()
{
	DbgSetThreadName("CSaveKnownThread");
	Save();
}
// <== Threaded Known Files Saving [Stulle] - Stulle
// X
// 0: hashset to keep
// 1: PartiallyPurge Old hashset
// 2: Purge unref hashset
uint_ptr CKnownFileList::ShouldPurgeAICHHashset(const CAICHHash& rAICHHash) const
{
#ifdef REPLACE_ATLMAP
	KnonwFilesByAICHMap::const_iterator it = m_mapKnownFilesByAICH.find(rAICHHash);
	if(it != m_mapKnownFilesByAICH.end())
		return (it->second->ShouldPartiallyPurgeFile())?1:0;
#else
	const CKnownFile* pFile = NULL;
	if (m_mapKnownFilesByAICH.Lookup(rAICHHash, pFile))
		return (pFile->ShouldPartiallyPurgeFile())?1:0;
#endif
	return 2;
}

void CKnownFileList::AICHHashChanged(const CAICHHash* pOldAICHHash, const CAICHHash& rNewAICHHash, CKnownFile* pFile)
{
#ifdef REPLACE_ATLMAP
	if (pOldAICHHash != NULL)
		m_mapKnownFilesByAICH.erase(*pOldAICHHash);
	m_mapKnownFilesByAICH[rNewAICHHash] = pFile;
#else
	if (pOldAICHHash != NULL)
		m_mapKnownFilesByAICH.RemoveKey(*pOldAICHHash);
	m_mapKnownFilesByAICH.SetAt(rNewAICHHash, pFile);
#endif
}