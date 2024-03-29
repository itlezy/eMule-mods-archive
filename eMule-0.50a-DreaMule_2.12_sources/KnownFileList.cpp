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
//Xman x4.1.1 
//Xman [MoNKi: -Downloaded History-]
#include "SharedFilesWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define KNOWN_MET_FILENAME		_T("known.met")
#define CANCELLED_MET_FILENAME	_T("cancelled.met")

CKnownFileList::CKnownFileList()
{
	//Xman Init-Hashtable optimization
	//m_Files_map.InitHashTable(2063); //moved down
	m_mapCancelledFiles.InitHashTable(1031);
	accepted = 0;
	requested = 0;
	transferred = 0;
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
	CString fullpath=thePrefs.GetConfigDir();
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
		//Xman Init-Hashtable optimization
		m_Files_map.InitHashTable(1031);
		//Xman end
		return false;
	}
	setvbuf(file.m_pStream, NULL, _IOFBF, 16384);

	CKnownFile* pRecord = NULL;
	try {
		uint8 header = file.ReadUInt8();
		if (header != MET_HEADER && header != MET_HEADER_I64TAGS){
			file.Close();
			//Xman Init-Hashtable optimization
			m_Files_map.InitHashTable(1031);
			//Xman end
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_SERVERMET_BAD));
			return false;
		}
		AddDebugLogLine(false, _T("Known.met file version is %u (%s support 64bit tags)"), header, (header == MET_HEADER) ? _T("doesn't") : _T("does")); 

		UINT RecordsNumber = file.ReadUInt32();

		//Xman Init-Hashtable optimization
		m_Files_map.InitHashTable(UINT(RecordsNumber*1.2f + 1031));
		//Xman end

		for (UINT i = 0; i < RecordsNumber; i++) {
			pRecord = new CKnownFile();
			if (!pRecord->LoadFromFile(&file)){
				TRACE(_T("*** Failed to load entry %u (name=%s  hash=%s  size=%I64u  parthashs=%u expected parthashs=%u) from known.met\n"), i, 
					//Xman
					pRecord->GetFileName(), md4str(pRecord->GetFileHash()), pRecord->GetFileSize(), pRecord->GetHashCount(), pRecord->GetED2KPartCount());	// SLUGFILLER: SafeHash - removed unnececery hash counter
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
		//Xman Init-Hashtable optimization
		if(m_Files_map.GetHashTableSize()==0)
			m_Files_map.InitHashTable(1031);
		//Xman end

		return false;
	}

	return true;
}

bool CKnownFileList::LoadCancelledFiles(){
	if (!thePrefs.IsRememberingCancelledFiles())
	{
		//Xman Init-Hashtable optimization
		m_mapCancelledFiles.InitHashTable(209);
		//Xman end
		return true;
	}
	CString fullpath=thePrefs.GetConfigDir();
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
		//Xman Init-Hashtable optimization
		m_mapCancelledFiles.InitHashTable(209);
		//Xman end
		return false;
	}
	setvbuf(file.m_pStream, NULL, _IOFBF, 16384);
	uchar ucHash[16];
	try {
		uint8 header = file.ReadUInt8();
		if (header != MET_HEADER){
			file.Close();
			//Xman Init-Hashtable optimization
			m_mapCancelledFiles.InitHashTable(209);
			//Xman end
			return false;
		}

		UINT RecordsNumber = file.ReadUInt32();
		//Xman Init-Hashtable optimization
		m_mapCancelledFiles.InitHashTable(UINT(RecordsNumber * 1.2f + 209));
		//Xman end
		for (UINT i = 0; i < RecordsNumber; i++) {
			file.ReadHash16(ucHash);
			uint8 nCount = file.ReadUInt8();
			// for compatibility with future versions which may add more data than just the hash
			for (UINT j = 0; j < nCount; j++) {
				CTag tag(&file, false);
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
		//Xman Init-Hashtable optimization
		if(m_mapCancelledFiles.GetHashTableSize()==0)
			m_mapCancelledFiles.InitHashTable(209);
		//Xman end
		return false;
	}
	return true;
}

void CKnownFileList::Save()
{
	if (thePrefs.GetLogFileSaving())
		AddDebugLogLine(false, _T("Saving known files list file \"%s\""), KNOWN_MET_FILENAME);
	m_nLastSaved = ::GetTickCount(); 
	CString fullpath=thePrefs.GetConfigDir();
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
	fullpath=thePrefs.GetConfigDir();
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
			file.WriteUInt8(MET_HEADER);
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
	if (::GetTickCount() - m_nLastSaved > MIN2MS(11))
		Save();
}

bool CKnownFileList::SafeAddKFile(CKnownFile* toadd)
{
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

		//Xman [MoNKi: -Downloaded History-]
		theApp.emuledlg->sharedfileswnd->historylistctrl.RemoveFileFromView(pFileInMap);
		//Xman end

		delete pFileInMap;

		//Xman todo: check out if the new official way is working
		/*
		//Xman official bugfix for redownloading already downloaded file
		//Xman 5.1 readded but modified
		//remark: official emule has a bug at this point. download a shared file and you see:
		//both files will be unshared. But this patch leads to a crash in an unknown situation
		if (theApp.sharedfiles && !theApp.sharedfiles->IsFilePtrInList(toadd))
			theApp.sharedfiles->SafeAddKFileWithOutRemoveHasing(toadd);

		
		//Xman [MoNKi: -Downloaded History-]
		theApp.emuledlg->sharedfileswnd->historylistctrl.AddFile(toadd);
		*/

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
		if (cur_file->GetUtcFileDate() == date && cur_file->GetFileSize() == size && !_tcscmp(filename, cur_file->GetFileName()))
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
		m_mapCancelledFiles.SetAt(CSKey(hash), 1);	
	}
}

bool CKnownFileList::IsCancelledFileByID(const uchar* hash) const
{
	if (thePrefs.IsRememberingCancelledFiles()){
		int dwDummy;
		if (m_mapCancelledFiles.Lookup(CSKey(hash), dwDummy)){
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
			msg = GetResString(IDS_DOWNHISTORY_CHECK1);
		}
		else if(ret==3)
		{
			msg= GetResString(IDS_DOWNHISTORY_CHECK4);
		}
		else {
			msg.Format(GetResString(IDS_DOWNHISTORY_CHECK2), filesFound.GetCount(), filename);
		}

		for(int i=0;i<filesFound.GetCount();i++){
			CKnownFile *cur_file = filesFound.GetAt(i);
			CString sData;

			msg+=cur_file->GetFileName() + _T("\n");
			sData.Format(GetResString(IDS_DL_SIZE) + _T(": %I64u, ") + GetResString(IDS_FILEID) + _T(": %s"), (uint64)cur_file->GetFileSize(), md4str(cur_file->GetFileHash()));
			msg+=sData;
			if(!cur_file->GetFileComment().IsEmpty()) //Add comment
				msg+=_T("\n") + GetResString(IDS_COMMENT) + _T(": \"") + cur_file->GetFileComment() + _T("\"");
			msg+="\n\n";
		}
		msg += GetResString(IDS_DOWNHISTORY_CHECK3);
		if(MessageBox(NULL, msg, GetResString(IDS_DOWNHISTORY),MB_YESNO|MB_ICONQUESTION)==IDYES)
			return true;
		else
			return false;
	}
	else
	{
		return true;
	}
}
//Xman end

//Xman [MoNKi: -Downloaded History-]
CKnownFilesMap* CKnownFileList::GetDownloadedFiles(){
	CKnownFilesMap *filesFound;
	filesFound = new CKnownFilesMap;

	POSITION pos = m_Files_map.GetStartPosition();					
	while(pos){
		CKnownFile* cur_file;
		CCKey key;
		m_Files_map.GetNextAssoc( pos, key, cur_file );
		if (!theApp.sharedfiles->IsFilePtrInList(cur_file)){
			CCKey key2(cur_file->GetFileHash());
			CKnownFile* pFileInMap;
			if (!filesFound->Lookup(key2, pFileInMap))
				filesFound->SetAt(key2, cur_file);
		}
	}
	return filesFound;
}

bool CKnownFileList::RemoveKnownFile(CKnownFile *toRemove){
	if (toRemove){
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
	}
	return false;
}

void CKnownFileList::ClearHistory(){
	POSITION pos = m_Files_map.GetStartPosition();					
	while(pos){
		CKnownFile* cur_file;
		CCKey key;
		m_Files_map.GetNextAssoc( pos, key, cur_file );
		if (!theApp.sharedfiles->IsFilePtrInList(cur_file)){
			m_Files_map.RemoveKey(key);
			delete cur_file;
		}
	}	
}

//Xman end