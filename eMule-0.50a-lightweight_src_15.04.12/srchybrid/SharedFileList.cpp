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
#include <sys/stat.h>
#include "emule.h"
#include "SharedFileList.h"
#include "KnownFileList.h"
#include "Packets.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "kademlia/kademlia/search.h"
#include "kademlia/kademlia/SearchManager.h"
#include "kademlia/kademlia/prefs.h"
#include "kademlia/kademlia/Tag.h"
#include "DownloadQueue.h"
#include "Statistics.h"
#include "Preferences.h"
#include "OtherFunctions.h"
#include "KnownFile.h"
#include "Sockets.h"
#include "SafeFile.h"
#include "Server.h"
#include "UpDownClient.h"
#include "PartFile.h"
#include "emuledlg.h"
#include "StringConversion.h"
#include "ClientList.h"
#include "Log.h"
#include "Collection.h"
#include "kademlia/kademlia/UDPFirewallTester.h"
#include "SharedFilesCtrl.h"
#include "TransferWnd.h"
#include "VolumeInfo.h" // X: [FSFS] - [FileSystemFeaturesSupport]

//Xman advanced upload-priority
#include "UploadQueue.h"
#include <list>
#include <algorithm>
#include "XMessageBox.h"// X: [QOH] - [QueryOnHashing]

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


typedef CSimpleArray<CKnownFile*> CSimpleKnownFileArray;
#define	SHAREDFILES_FILE	_T("sharedfiles.dat")


///////////////////////////////////////////////////////////////////////////////
// CPublishKeyword

class CPublishKeyword
{
public:
	CPublishKeyword(const Kademlia::CKadTagValueString& rstrKeyword)
	{
		m_strKeyword = rstrKeyword;
		// min. keyword char is allowed to be < 3 in some cases (see also 'CSearchManager::GetWords')
		//ASSERT( rstrKeyword.GetLength() >= 3 );
		ASSERT( !rstrKeyword.IsEmpty() );
		KadGetKeywordHash(rstrKeyword, &m_nKadID);
		SetNextPublishTime(0);
		SetPublishedCount(0);
	}

	const Kademlia::CUInt128& GetKadID() const { return m_nKadID; }
	const Kademlia::CKadTagValueString& GetKeyword() const { return m_strKeyword; }
	int GetRefCount() const { return m_aFiles.GetSize(); }
	const CSimpleKnownFileArray& GetReferences() const { return m_aFiles; }

	uint64 GetNextPublishTime() const { return m_tNextPublishTime; }
	void SetNextPublishTime(uint64 tNextPublishTime) { m_tNextPublishTime = tNextPublishTime; }

	UINT GetPublishedCount() const { return m_uPublishedCount; }
	void SetPublishedCount(UINT uPublishedCount) { m_uPublishedCount = uPublishedCount; }
	void IncPublishedCount() { m_uPublishedCount++; }

	BOOL AddRef(CKnownFile* pFile)
	{
		if (m_aFiles.Find(pFile) != -1)
		{
			ASSERT(0);
			return FALSE;
		}
		return m_aFiles.Add(pFile);
	}

	int RemoveRef(CKnownFile* pFile)
	{
		m_aFiles.Remove(pFile);
		return m_aFiles.GetSize();
	}

	void RemoveAllReferences()
	{
		m_aFiles.RemoveAll();
	}

	void RotateReferences(int iRotateSize)
	{
		if (m_aFiles.GetSize() > iRotateSize)
		{
			CKnownFile** ppRotated = (CKnownFile**)malloc(m_aFiles.m_nAllocSize * sizeof(*m_aFiles.GetData()));
			if (ppRotated != NULL)
			{
				memcpy(ppRotated, m_aFiles.GetData() + iRotateSize, (m_aFiles.GetSize() - iRotateSize) * sizeof(*m_aFiles.GetData()));
				memcpy(ppRotated + m_aFiles.GetSize() - iRotateSize, m_aFiles.GetData(), iRotateSize * sizeof(*m_aFiles.GetData()));
				free(m_aFiles.GetData());
				m_aFiles.m_aT = ppRotated;
			}
		}
	}

protected:
	Kademlia::CKadTagValueString m_strKeyword;
	Kademlia::CUInt128 m_nKadID;
	uint64 m_tNextPublishTime;
	UINT m_uPublishedCount;
	CSimpleKnownFileArray m_aFiles;
};


///////////////////////////////////////////////////////////////////////////////
// CPublishKeywordList

class CPublishKeywordList
{
public:
	CPublishKeywordList();
	~CPublishKeywordList();

	void AddKeywords(CKnownFile* pFile);
	void RemoveKeywords(CKnownFile* pFile);
	void RemoveAllKeywords();

	void RemoveAllKeywordReferences();
	void PurgeUnreferencedKeywords();

	INT_PTR GetCount() const { return m_lstKeywords.GetCount(); }

	CPublishKeyword* GetNextKeyword();
	void ResetNextKeyword();

	uint64 GetNextPublishTime() const { return m_tNextPublishKeywordTime; }
	void SetNextPublishTime(uint64 tNextPublishKeywordTime) { m_tNextPublishKeywordTime = tNextPublishKeywordTime; }

#ifdef _DEBUG
	void Dump();
#endif

protected:
	// can't use a CAtlMap - too many disadvantages in processing the 'list'
	//CTypedPtrMap<CMapStringToPtr, CString, CPublishKeyword*> m_lstKeywords;
	CAtlList<CPublishKeyword*> m_lstKeywords;
	POSITION m_posNextKeyword;
	uint64 m_tNextPublishKeywordTime;

	CPublishKeyword* FindKeyword(const CStringW& rstrKeyword, POSITION* ppos = NULL) const;
};

CPublishKeywordList::CPublishKeywordList()
{
	ResetNextKeyword();
	SetNextPublishTime(0);
}

CPublishKeywordList::~CPublishKeywordList()
{
	RemoveAllKeywords();
}

CPublishKeyword* CPublishKeywordList::GetNextKeyword()
{
	if (m_posNextKeyword == NULL)
	{
		m_posNextKeyword = m_lstKeywords.GetHeadPosition();
		if (m_posNextKeyword == NULL)
			return NULL;
	}
	return m_lstKeywords.GetNext(m_posNextKeyword);
}

void CPublishKeywordList::ResetNextKeyword()
{
	m_posNextKeyword = m_lstKeywords.GetHeadPosition();
}

CPublishKeyword* CPublishKeywordList::FindKeyword(const CStringW& rstrKeyword, POSITION* ppos) const
{
	POSITION pos = m_lstKeywords.GetHeadPosition();
	while (pos)
	{
		POSITION posLast = pos;
		CPublishKeyword* pPubKw = m_lstKeywords.GetNext(pos);
		if (pPubKw->GetKeyword() == rstrKeyword)
		{
			if (ppos)
				*ppos = posLast;
			return pPubKw;
		}
	}
	return NULL;
}

void CPublishKeywordList::AddKeywords(CKnownFile* pFile)
{
	const Kademlia::WordList& wordlist = pFile->GetKadKeywords();
	//ASSERT( wordlist.size() > 0 );
	Kademlia::WordList::const_iterator it;
	for (it = wordlist.begin(); it != wordlist.end(); it++)
	{
		const CStringW& strKeyword = *it;
		CPublishKeyword* pPubKw = FindKeyword(strKeyword);
		if (pPubKw == NULL)
		{
			pPubKw = new CPublishKeyword(strKeyword);
			m_lstKeywords.AddTail(pPubKw);
			SetNextPublishTime(0);
		}
		if(pPubKw->AddRef(pFile) && pPubKw->GetNextPublishTime() > MIN2S(30))
		{
			// User may be adding and removing files, so if this is a keyword that
			// has already been published, we reduce the time, but still give the user
			// enough time to finish what they are doing.
			// If this is a hot node, the Load list will prevent from republishing.
			pPubKw->SetNextPublishTime(MIN2S(30));
		}
	}
}

void CPublishKeywordList::RemoveKeywords(CKnownFile* pFile)
{
	const Kademlia::WordList& wordlist = pFile->GetKadKeywords();
	//ASSERT( wordlist.size() > 0 );
	Kademlia::WordList::const_iterator it;
	for (it = wordlist.begin(); it != wordlist.end(); it++)
	{
		const CStringW& strKeyword = *it;
		POSITION pos;
		CPublishKeyword* pPubKw = FindKeyword(strKeyword, &pos);
		if (pPubKw != NULL)
		{
			if (pPubKw->RemoveRef(pFile) == 0)
			{
				if (pos == m_posNextKeyword)
					(void)m_lstKeywords.GetNext(m_posNextKeyword);
				m_lstKeywords.RemoveAt(pos);
				delete pPubKw;
				SetNextPublishTime(0);
			}
		}
	}
}

void CPublishKeywordList::RemoveAllKeywords()
{
	POSITION pos = m_lstKeywords.GetHeadPosition();
	while (pos)
		delete m_lstKeywords.GetNext(pos);
	m_lstKeywords.RemoveAll();
	ResetNextKeyword();
	SetNextPublishTime(0);
}

void CPublishKeywordList::RemoveAllKeywordReferences()
{
	POSITION pos = m_lstKeywords.GetHeadPosition();
	while (pos)
		m_lstKeywords.GetNext(pos)->RemoveAllReferences();
}

void CPublishKeywordList::PurgeUnreferencedKeywords()
{
	POSITION pos = m_lstKeywords.GetHeadPosition();
	while (pos)
	{
		POSITION posLast = pos;
		CPublishKeyword* pPubKw = m_lstKeywords.GetNext(pos);
		if (pPubKw->GetRefCount() == 0)
		{
			if (posLast == m_posNextKeyword)
				(void)m_lstKeywords.GetNext(m_posNextKeyword);
			m_lstKeywords.RemoveAt(posLast);
			delete pPubKw;
			SetNextPublishTime(0);
		}
	}
}

#ifdef _DEBUG
void CPublishKeywordList::Dump()
{
	int i = 0;
	POSITION pos = m_lstKeywords.GetHeadPosition();
	while (pos)
	{
		CPublishKeyword* pPubKw = m_lstKeywords.GetNext(pos);
		TRACE(_T("%3u: %-10ls  ref=%u  %s\n"), i, pPubKw->GetKeyword(), pPubKw->GetRefCount(), CastSecondsToHM(pPubKw->GetNextPublishTime()));
		i++;
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
// CAddFileThread

CAddFileThread::CAddFileThread(CSharedFileList* pOwner, LPCTSTR directory, LPCTSTR filename, LPCTSTR strSharedDir, CPartFile* partfile)
: Thread(true, PRIO_LOW), m_pOwner(pOwner), m_strDirectory(directory), m_strFilename(filename), m_partfile(partfile), m_strSharedDir(strSharedDir)
{
}

void CAddFileThread::run()
{
	DbgSetThreadName("Hashing %s", m_strFilename);
	if ( !(m_pOwner || m_partfile) || m_strFilename.IsEmpty() || !CemuleDlg::IsRunning() )
		return;
	//Xman
	// BEGIN SLUGFILLER: SafeHash
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return;
	// END SLUGFILLER: SafeHash
	InitThreadLocale();

	CoInitialize(NULL);

	// locking that hashing thread is needed because we may create a couple of those threads at startup when rehashing
	// potentially corrupted downloading part files. if all those hash threads would run concurrently, the io-system would be
	// under very heavy load and slowly progressing
	//Xman
	// SLUGFILLER: SafeHash remove - locking code removed, unnecessary
	/*
	Poco::FastMutex::SingleLock sLock1(theApp.hashing_mut); // only one filehash at a time
	sLock1.Lock();
	*/
	CString strFilePath;
	_tmakepathlimit(strFilePath.GetBuffer(MAX_PATH), NULL, m_strDirectory, m_strFilename, NULL);
	strFilePath.ReleaseBuffer();
	if (m_partfile)
		Log(GetResString(IDS_HASHINGFILE) + _T(" \"%s\" \"%s\""), m_partfile->GetFileName(), strFilePath);
	else
		Log(GetResString(IDS_HASHINGFILE) + _T(" \"%s\""), strFilePath);
	
	CKnownFile* newrecord = new CKnownFile();
	//zz_fly :: minor issue in case of shutdown while still hashing :: WiZaRd :: start
	/*
	if (newrecord->CreateFromFile(m_strDirectory, m_strFilename, m_partfile) && theApp.emuledlg && CemuleDlg::IsRunning()) // SLUGFILLER: SafeHash - in case of shutdown while still hashing
	*/
	if (newrecord->CreateFromFile(m_strDirectory, m_strFilename, m_partfile))
	//zz_fly :: end
	{
		newrecord->SetSharedDirectory(m_strSharedDir);
		if (m_partfile && m_partfile->GetFileOp() == PFOP_HASHING)
			m_partfile->SetFileOp(PFOP_NONE);
		if (theApp.emuledlg == NULL || !CemuleDlg::IsRunning() || // SLUGFILLER: SafeHash - in case of shutdown while still hashing 
			!PostMessage(theApp.emuledlg->m_hWnd, TM_FINISHEDHASHING, (m_pOwner ? 0: (WPARAM)m_partfile), (LPARAM)newrecord))
			delete newrecord;
	}
	else
	{
		//zz_fly :: minor issue in case of shutdown while still hashing :: WiZaRd :: start
		/*
		if (theApp.emuledlg && CemuleDlg::IsRunning())
		{
		*/
		//zz_fly :: end
			if (m_partfile && m_partfile->GetFileOp() == PFOP_HASHING)
				m_partfile->SetFileOp(PFOP_NONE);
		//}

		// SLUGFILLER: SafeHash - inform main program of hash failure
		if (m_pOwner && theApp.emuledlg && CemuleDlg::IsRunning())
		{
			UnknownFile_Struct* hashed = new UnknownFile_Struct;
			hashed->strDirectory = m_strDirectory;
			hashed->strName = m_strFilename;
			if (!PostMessage(theApp.emuledlg->m_hWnd, TM_HASHFAILED, 0, (LPARAM)hashed))
				delete hashed;
		}
		// SLUGFILLER: SafeHash
		delete newrecord;
	}
	//Xman
	// SLUGFILLER: SafeHash remove - locking code removed, unnecessary
	/*
	sLock1.Unlock();
	*/
	CoUninitialize();
}

///////////////////////////////////////////////////////////////////////////////
// CSharedFileList

CSharedFileList::CSharedFileList(CServerConnect* in_server)
#ifdef REPLACE_ATLMAP
: m_Files_map(DEFAULT_FILES_TABLE_SIZE), m_UnsharedFiles_map(DEFAULT_FILES_TABLE_SIZE)
#endif
{
	server = in_server;
	output = 0;
#ifndef REPLACE_ATLMAP
	m_Files_map.InitHashTable(DEFAULT_FILES_TABLE_SIZE);
	m_UnsharedFiles_map.InitHashTable(DEFAULT_FILES_TABLE_SIZE);
#endif
	m_keywords = new CPublishKeywordList;
	m_lastPublishED2K = 0;
	m_lastPublishED2KFlag = true;
	m_currFileSrc = 0;
	m_lastPublishKadSrc = 0;
	//m_currFileKey = 0;
	bHaveSingleSharedFiles = false;
	//Xman advanced upload-priority
	m_lastavgPercent = 0; 
	m_avg_virtual_sources = 0;
	m_avg_client_on_uploadqueue = 0;
	//Xman end
	// SLUGFILLER: SafeHash remove - delay load shared files
	/*
	LoadSingleSharedFilesList();
	FindSharedFiles();
	*/
	// SLUGFILLER End
}

CSharedFileList::~CSharedFileList(){
	while (!waitingforhash_list.IsEmpty())
		delete waitingforhash_list.RemoveHead();
	// SLUGFILLER: SafeHash
	while (!currentlyhashing_list.IsEmpty()){
		UnknownFile_Struct* nextfile = currentlyhashing_list.RemoveHead();
		delete nextfile;
	}
	// SLUGFILLER: SafeHash
	delete m_keywords;
}

void CSharedFileList::FindSharedFiles(bool bNotify)// X: [QOH] - [QueryOnHashing]
{
	//Xman
	// SLUGFILLER: SafeHash remove - only called after the download queue is created
	/*
	if (!m_Files_map.IsEmpty())
	*/
	{
		Poco::FastMutex::SingleLock listlock(m_mutWriteList);
		
#ifdef REPLACE_ATLMAP
		for (CKnownFilesMap::const_iterator it = m_Files_map.begin(); it != m_Files_map.end();)
		{
			CKnownFile* cur_file = it->second;
#else
		POSITION pos = m_Files_map.GetStartPosition();
		while (pos)
		{
			CCKey key;
			CKnownFile* cur_file;
			m_Files_map.GetNextAssoc(pos, key, cur_file);
#endif
			if (IsCPartFile(cur_file)/*cur_file->IsKindOf(RUNTIME_CLASS(CPartFile))*/ 
				&& !theApp.downloadqueue->IsPartFile(cur_file) 
				&& !theApp.knownfiles->IsFilePtrInList(cur_file)
				&& _taccess(cur_file->GetFilePath(), 0) == 0)
			{
#ifdef REPLACE_ATLMAP
				++it;
#endif
				continue;
			}
#ifdef REPLACE_ATLMAP
			//m_UnsharedFiles_map[CSKey(cur_file->GetFileHash())] = true;
			m_UnsharedFiles_map.insert(CSKey(cur_file->GetFileHash()));
#else
			m_UnsharedFiles_map.SetAt(CSKey(cur_file->GetFileHash()), true);
#endif
			listlock.Lock();
#ifdef REPLACE_ATLMAP
			it = m_Files_map.erase(it);
#else
			m_Files_map.RemoveKey(key);
#endif
			listlock.Unlock();
		}
		
		ASSERT( theApp.downloadqueue );
		if (theApp.downloadqueue)
			theApp.downloadqueue->AddPartFilesToShare(); // read partfiles
	}

	// khaos::kmod+ Fix: Shared files loaded multiple times.
	std::vector<LPCTSTR> l_sAdded;
	LPCTSTR tempDir = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);

	l_sAdded.push_back( g_VolumeInfo.GetRealPath(tempDir) ); // X: [FSFS] - [FileSystemFeaturesSupport]

	AddFilesFromDirectory(tempDir, bNotify);// X: [QOH] - [QueryOnHashing]

	for (size_t ix=1;ix<thePrefs.GetCatCount();ix++)
	{
		tempDir = thePrefs.GetCatPath(ix);
		LPCTSTR realDir = g_VolumeInfo.GetRealPath(tempDir); // X: [FSFS] - [FileSystemFeaturesSupport]
		if(std::find(l_sAdded.begin(), l_sAdded.end(), realDir) ==  l_sAdded.end()) {
			l_sAdded.push_back(realDir);
			AddFilesFromDirectory(tempDir, bNotify);// X: [QOH] - [QueryOnHashing]
		}
	}

	for (POSITION pos = thePrefs.shareddir_list.GetHeadPosition();pos != 0;)
	{
		tempDir = thePrefs.shareddir_list.GetNext(pos);
		LPCTSTR realDir = g_VolumeInfo.GetRealPath(tempDir); // X: [FSFS] - [FileSystemFeaturesSupport]
		if(std::find(l_sAdded.begin(), l_sAdded.end(), realDir) ==  l_sAdded.end()) {
			l_sAdded.push_back(realDir);
			AddFilesFromDirectory(tempDir, bNotify);// X: [QOH] - [QueryOnHashing]
		}
	}
	// add all single shared files
	for (POSITION pos = m_liSingleSharedFiles.GetHeadPosition(); pos != NULL; m_liSingleSharedFiles.GetNext(pos))
		CheckAndAddSingleFile(m_liSingleSharedFiles.GetAt(pos));

	// khaos::kmod-
	if (waitingforhash_list.IsEmpty())
		AddLogLine(false,GetResString(IDS_SHAREDFOUND), GetCount());
	else
		AddLogLine(false,GetResString(IDS_SHAREDFOUNDHASHING), GetCount(), waitingforhash_list.GetCount());
	
	HashNextFile();
}
//thePrefs.queryOnHashing
//0:bNotify==true	Hashing No query
//1:bNotify==false	No Hashing No query
//2:bNotify==true	Query On Hashing
void CSharedFileList::AddFilesFromDirectory(LPCTSTR rstrDirectory, bool bNotify)// X: [QOH] - [QueryOnHashing]
{
	CFileFind ff;

	CString strSearchPath(rstrDirectory);
	if (strSearchPath.Right(1)!=_T('\\'))
		strSearchPath+=_T('\\');
	//PathAddBackslash(strSearchPath.GetBuffer(strSearchPath.GetLength() + 1));
	//strSearchPath.ReleaseBuffer();
	strSearchPath += _T("*");
	bool end = !ff.FindFile(strSearchPath, 0);
	if (end) {
		DWORD dwError = GetLastError();
		if (dwError != ERROR_FILE_NOT_FOUND)
			LogWarning(GetResString(IDS_ERR_SHARED_DIR), rstrDirectory, GetErrorMessage(dwError));
		return;
	}

	while (!end)
	{
		end = !ff.FindNextFile();
		CheckAndAddSingleFile(ff, bNotify);// X: [QOH] - [QueryOnHashing]
	}
	ff.Close();
}

bool CSharedFileList::AddSingleSharedFile(const CString& rstrFilePath, bool bNoUpdate)
{
	bool bExclude = false;
	bool bShared = false;
	// first check if we are explicty exluding this file
	for (POSITION pos = m_liSingleExcludedFiles.GetHeadPosition(); pos != NULL; m_liSingleExcludedFiles.GetNext(pos) )
	{
		if (rstrFilePath.CompareNoCase(m_liSingleExcludedFiles.GetAt(pos)) == 0)
		{
			bExclude = true;
			m_liSingleExcludedFiles.RemoveAt(pos);
			break;
		}
	}

	// check if we share this file in general
	bShared = ShouldBeShared(rstrFilePath.Left(rstrFilePath.ReverseFind('\\') + 1), rstrFilePath, false);

	if (bShared && !bExclude){
		// we should share this file already
		return false;
	}
	if(bNoUpdate || CheckAndAddSingleFile(rstrFilePath)){// X: [QOH] - [QueryOnHashing]
		if (!bShared){
		// the directory is not shared, so we need a special entry
		m_liSingleSharedFiles.AddTail(rstrFilePath);
	}	
		return true;
	}
	return false;
}

bool CSharedFileList::CheckAndAddSingleFile(const CString& rstrFilePath)
{
	
	CFileFind ff;
	bool end = !ff.FindFile(rstrFilePath, 0);
	if (end) {
		DWORD dwError = GetLastError();
		if (dwError != ERROR_FILE_NOT_FOUND)
			LogWarning(GetResString(IDS_ERR_SHARED_DIR), rstrFilePath, GetErrorMessage(dwError));
		return false;
	}
	ff.FindNextFile();
	bool added = true;
	added = CheckAndAddSingleFile(ff, added);// X: [QOH] - [QueryOnHashing]
	ff.Close();
	if(!added)
		return false;
	HashNextFile();
	bHaveSingleSharedFiles = true;
	// GUI updating needs to be done by caller
	return true;
}

bool CSharedFileList::SafeAddKFile(CKnownFile* toadd, bool bOnlyAdd)
{
	bool bAdded = false;
	RemoveFromHashing(toadd);	// SLUGFILLER: SafeHash - hashed ok, remove from list, in case it was on the list
	bAdded = AddFile(toadd);
	//Xman advanced upload-priority
	if(bAdded)
	{
		toadd->CheckAUPFilestats(true);
	}
	//Xman end
	if (bOnlyAdd)
		return bAdded;
	if (bAdded && output){
		output->AddFile(toadd);
		theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Shared);
		//Xman [MoNKi: -Downloaded History-]
		if(!toadd->IsPartFile() && thePrefs.m_bHistoryShowShared && !thePrefs.IsHistoryListDisabled()){
			theApp.emuledlg->transferwnd->historylistctrl.AddFile(toadd); 
			theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2History);
		}
		//Xman end
	}
	m_lastPublishED2KFlag = true;
	return bAdded;
}

void CSharedFileList::RepublishFile(CKnownFile* pFile)
{
	CServer* pCurServer = server->GetCurrentServer();
	if (pCurServer && (pCurServer->GetTCPFlags() & SRV_TCPFLG_COMPRESSION))
	{
		m_lastPublishED2KFlag = true;
		pFile->SetPublishedED2K(false); // FIXME: this creates a wrong 'No' for the ed2k shared info in the listview until the file is shared again.
	}
}

bool CSharedFileList::AddFile(CKnownFile* pFile)
{
	ASSERT( pFile->GetFileIdentifier().HasExpectedMD4HashCount() );
	//ASSERT( !pFile->IsKindOf(RUNTIME_CLASS(CPartFile)) || !STATIC_DOWNCAST(CPartFile, pFile)->m_bMD4HashsetNeeded );
	ASSERT( !IsCPartFile(pFile) || !((CPartFile*)pFile)->m_bMD4HashsetNeeded );
	ASSERT( !pFile->IsShellLinked() || ShouldBeShared(pFile->GetSharedDirectory(), _T(""), false) );
#ifdef REPLACE_ATLMAP
	CKnownFilesMap::const_iterator it = m_Files_map.find(pFile->GetFileHash());
	if(it != m_Files_map.end())
	{
		CKnownFile* pFileInMap = it->second;
#else
	CCKey key(pFile->GetFileHash());
	CKnownFile* pFileInMap;
	if (m_Files_map.Lookup(key, pFileInMap))
	{
#endif
		TRACE(_T("%hs: File already in shared file list: %s \"%s\" \"%s\"\n"), __FUNCTION__, md4str(pFileInMap->GetFileHash()), pFileInMap->GetFileName(), pFileInMap->GetFilePath());
		TRACE(_T("%hs: File to add:                      %s \"%s\" \"%s\"\n"), __FUNCTION__, md4str(pFile->GetFileHash()), pFile->GetFileName(), pFile->GetFilePath());
		if (!IsCPartFile(pFileInMap)/*!pFileInMap->IsKindOf(RUNTIME_CLASS(CPartFile))*/ || theApp.downloadqueue->IsPartFile(pFileInMap))
			LogWarning(GetResString(IDS_ERR_DUPL_FILES), pFileInMap->GetFilePath(), pFile->GetFilePath());
		return false;
	}
#ifdef REPLACE_ATLMAP
	m_UnsharedFiles_map.erase(CSKey(pFile->GetFileHash()));
#else
	m_UnsharedFiles_map.RemoveKey(CSKey(pFile->GetFileHash()));
#endif
	
	Poco::FastMutex::SingleLock listlock(m_mutWriteList, true);
#ifdef REPLACE_ATLMAP
	m_Files_map[pFile->GetFileHash()] = pFile;
#else
	m_Files_map.SetAt(key, pFile);
#endif
	listlock.Unlock();

	bool bKeywordsNeedUpdated = true;

	if(!pFile->IsPartFile() && !pFile->m_pCollection && pFile->HasCollectionExtenesion_Xtreme() /*CCollection::HasCollectionExtention(pFile->GetFileName())*/) //Xman Code Improvement for HasCollectionExtention
	{
		pFile->m_pCollection = new CCollection();
		if(!pFile->m_pCollection->InitCollectionFromFile(pFile->GetFilePath(), pFile->GetFileName()))
		{
			delete pFile->m_pCollection;
			pFile->m_pCollection = NULL;
		}
		else if (!pFile->m_pCollection->GetCollectionAuthorKeyString().IsEmpty())
		{
			//If the collection has a key, resetting the file name will
			//cause the key to be added into the wordlist to be stored
			//into Kad.
			pFile->SetFileName(pFile->GetFileName());
			//During the initial startup, sharedfiles is not accessable
			//to SetFileName which will then not call AddKeywords..
			//But when it is accessable, we don't allow it to readd them.
			if(theApp.sharedfiles)
				bKeywordsNeedUpdated = false;
		}
	}

	if(bKeywordsNeedUpdated)
		m_keywords->AddKeywords(pFile);

	theApp.knownfiles->m_nRequestedTotal += pFile->statistic.GetAllTimeRequests();
	theApp.knownfiles->m_nAcceptedTotal += pFile->statistic.GetAllTimeAccepts();
	theApp.knownfiles->m_nTransferredTotal += pFile->statistic.GetAllTimeTransferred();

	return true;
}

void CSharedFileList::FileHashingFinished(CKnownFile* file)
{
	// File hashing finished for a shared file (none partfile)
	//	- reading shared directories at startup and hashing files which were not found in known.met
	//	- reading shared directories during runtime (user hit Reload button, added a shared directory, ...)

	ASSERT( !IsFilePtrInList(file) );
	ASSERT( !theApp.knownfiles->IsFilePtrInList(file) );

	CKnownFile* found_file = GetFileByID(file->GetFileHash());
	if (found_file == NULL)
	{
		// check if we still want to actually share this file, the user might have unshared it while hashing
		if (!ShouldBeShared(file->GetSharedDirectory(), file->GetFilePath(), false)){
			RemoveFromHashing(file);
			if (!IsFilePtrInList(file) && !theApp.knownfiles->IsFilePtrInList(file))
				delete file;
			else
				ASSERT(0);
		}
		else 
		{
			SafeAddKFile(file);
			theApp.knownfiles->SafeAddKFile(file);
		}
	}
	else
	{
		TRACE(_T("%hs: File already in shared file list: %s \"%s\"\n"), __FUNCTION__, md4str(found_file->GetFileHash()), found_file->GetFilePath());
		TRACE(_T("%hs: File to add:                      %s \"%s\"\n"), __FUNCTION__, md4str(file->GetFileHash()), file->GetFilePath());
		LogWarning(GetResString(IDS_ERR_DUPL_FILES), found_file->GetFilePath(), file->GetFilePath());

		RemoveFromHashing(file);
		if (!IsFilePtrInList(file) && !theApp.knownfiles->IsFilePtrInList(file))
			delete file;
		else
			ASSERT(0);
	}
}

bool CSharedFileList::RemoveFile(CKnownFile* pFile, bool bDeleted)
{
	Poco::FastMutex::SingleLock listlock(m_mutWriteList, true);
#ifdef REPLACE_ATLMAP
	CKnownFilesMap::const_iterator it = m_Files_map.find(pFile->GetFileHash());
	bool bResult = (it != m_Files_map.end());
	if(bResult)
		m_Files_map.erase(it);
#else
	bool bResult = (m_Files_map.RemoveKey(CCKey(pFile->GetFileHash())) != FALSE);
#endif
	listlock.Unlock();
	
	if(output)// X: [BF] - [Bug Fix]
	output->RemoveFile(pFile, bDeleted);
	m_keywords->RemoveKeywords(pFile);
	theApp.m_DiscAccessQueue.remove(pFile);
	if (bResult)
	{
		// X: remove the file from single sharedfile list
		for (POSITION pos = m_liSingleSharedFiles.GetHeadPosition(); pos != NULL; m_liSingleSharedFiles.GetNext(pos) )
		{
			if (pFile->GetFilePath().CompareNoCase(m_liSingleSharedFiles.GetAt(pos)) == 0)
			{
				m_liSingleSharedFiles.RemoveAt(pos);
				break;
			}
		}
#ifdef REPLACE_ATLMAP
		m_UnsharedFiles_map[CSKey(pFile->GetFileHash())] = true;
#else
		m_UnsharedFiles_map.SetAt(CSKey(pFile->GetFileHash()), true);
#endif
		theApp.knownfiles->m_nRequestedTotal -= pFile->statistic.GetAllTimeRequests();
		theApp.knownfiles->m_nAcceptedTotal -= pFile->statistic.GetAllTimeAccepts();
		theApp.knownfiles->m_nTransferredTotal -= pFile->statistic.GetAllTimeTransferred();
	}
	return bResult;
}

void CSharedFileList::Reload(bool bNotify)// X: [QOH] - [QueryOnHashing]
{
	//Xman
	// BEGIN SLUGFILLER: SafeHash - don't allow to be called until after the control is loaded
	if (!output)
		return;
	// END SLUGFILLER: SafeHash
	g_VolumeInfo.ClearCache(/*-1*/); // X: [FSFS] - [FileSystemFeaturesSupport]
#ifdef REPLACE_ATLMAP
	m_mapPseudoDirNames.clear();
#else
	m_mapPseudoDirNames.RemoveAll();
#endif
	m_keywords->RemoveAllKeywordReferences();
	while (!waitingforhash_list.IsEmpty()) // delete all files which are waiting to get hashed, will be readded if still shared below
		delete waitingforhash_list.RemoveHead();
	bHaveSingleSharedFiles = false;
	FindSharedFiles(bNotify);// X: [QOH] - [QueryOnHashing]
	m_keywords->PurgeUnreferencedKeywords();
	// SLUGFILLER: SafeHash remove - check moved up
	output->ReloadFileList();
	if (!thePrefs.m_bDisableHistoryList)
		theApp.emuledlg->transferwnd->historylistctrl.ReloadFileList();
	m_lastPublishED2KFlag = true; //Xman CodeFix: we need to check if this files were published to server
}

void CSharedFileList::SetOutputCtrl(CSharedFilesCtrl* in_ctrl)
{
	output = in_ctrl;
	//output->ReloadFileList();
	//Xman
	//HashNextFile();		// SLUGFILLER: SafeHash - if hashing not yet started, start it now
	LoadSingleSharedFilesList();
	Reload(thePrefs.queryOnHashing!=1);		// SLUGFILLER: SafeHash - load shared files after everything// X: [QOH] - [QueryOnHashing]
}

uint8 GetRealPrio(uint8 in)
{
	switch(in) {
		case 4 : return 0;
		case 0 : return 1;
		case 1 : return 2;
		case 2 : return 3;
		case 3 : return 4;
	}
	return 0;
}

void CSharedFileList::SendListToServer(){
#ifdef REPLACE_ATLMAP
	if (m_Files_map.empty() || !server->IsConnected())
#else
	if (m_Files_map.IsEmpty() || !server->IsConnected())
#endif
	{
		return;
	}
	
	CServer* pCurServer = server->GetCurrentServer();
	CSafeMemFile files(1024);
	POSITION pos,pos2;
	CAtlList<CKnownFile*> sortedList;
	bool added=false;

#ifdef REPLACE_ATLMAP
	for (CKnownFilesMap::const_iterator it = m_Files_map.begin(); it != m_Files_map.end(); ++it)
	{
		CKnownFile* cur_file = it->second;
#else
	CCKey bufKey;
	CKnownFile* cur_file;
	for(pos=m_Files_map.GetStartPosition(); pos!=0;)
	{
		m_Files_map.GetNextAssoc(pos, bufKey, cur_file);
#endif
		added=false;
//>>> Spike2::Publish small incomplete files fix [WiZaRd]
		//WiZaRd: there is no use to publish files below 1 chunk filesize which we do not have complete
		//it might even cause problems if clients request this file from us
		if(cur_file->GetFileSize() <= PARTSIZE && cur_file->IsPartFile()) 
			added = true;
//<<< Spike2::Publish small incomplete files fix [WiZaRd]

		//insertsort into sortedList
//>>> WiZaRd::Publish small incomplete files fix [WiZaRd]
		if(!added && !cur_file->GetPublishedED2K() && (!cur_file->IsLargeFile() || (pCurServer != NULL && pCurServer->SupportsLargeFilesTCP())))
//		if(!cur_file->GetPublishedED2K() && (!cur_file->IsLargeFile() || (pCurServer != NULL && pCurServer->SupportsLargeFilesTCP())))
//<<< WiZaRd::Publish small incomplete files fix [WiZaRd]
		{
			for (pos2 = sortedList.GetHeadPosition();pos2 != 0 && !added;sortedList.GetNext(pos2))
			{
				if (GetRealPrio(sortedList.GetAt(pos2)->GetUpPriority()) <= GetRealPrio(cur_file->GetUpPriority()) )
				{
					sortedList.InsertBefore(pos2,cur_file);
					added=true;
				}
			}
			if (!added)
			{
				sortedList.AddTail(cur_file);
			}
		}
	}

	
	// add to packet
	uint32 limit = pCurServer ? pCurServer->GetSoftFiles() : 0;
	if( limit == 0 || limit > 200 )
	{
		limit = 200;
	}
	if( sortedList.GetCount() < limit )
	{
		limit = (uint32)sortedList.GetCount();
		if (limit == 0)
		{
			m_lastPublishED2KFlag = false;
			return;
		}
	}
	files.WriteUInt32(limit);
	uint32 count=0;
	for (pos = sortedList.GetHeadPosition();pos != 0 && count<limit; )
	{
		count++;
		CKnownFile* file = sortedList.GetNext(pos);
		CreateOfferedFilePacket(file, &files, pCurServer);
		file->SetPublishedED2K(true);
	}
	sortedList.RemoveAll();
	Packet* packet = new Packet(&files, OP_EDONKEYPROT, OP_OFFERFILES);
	// compress packet
	//   - this kind of data is highly compressable (N * (1 MD4 and at least 3 string meta data tags and 1 integer meta data tag))
	//   - the min. amount of data needed for one published file is ~100 bytes
	//   - this function is called once when connecting to a server and when a file becomes shareable - so, it's called rarely.
	//   - if the compressed size is still >= the original size, we send the uncompressed packet
	// therefor we always try to compress the packet
	if (pCurServer && pCurServer->GetTCPFlags() & SRV_TCPFLG_COMPRESSION){
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
		UINT uUncomprSize = packet->size;
#endif
		packet->PackPacket();
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
		if (thePrefs.GetDebugServerTCPLevel() > 0)
			Debug(_T(">>> Sending OP__OfferFiles(compressed); uncompr size=%u  compr size=%u  files=%u\n"), uUncomprSize, packet->size, limit);
#endif
	}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	else{
		if (thePrefs.GetDebugServerTCPLevel() > 0)
			Debug(_T(">>> Sending OP__OfferFiles; size=%u  files=%u\n"), packet->size, limit);
	}
#endif
	theStats.AddUpDataOverheadServer(packet->size);
	if (thePrefs.GetVerbose())
		AddDebugLogLine(false, _T("Server, Sendlist: Packet size:%u"), packet->size);
	server->SendPacket(packet,true);
}
#ifdef REPLACE_ATLMAP
CKnownFile*	CSharedFileList::GetFileByIndex(size_t index){
	size_t count = 0;
	for (CKnownFilesMap::const_iterator it = m_Files_map.begin(); it != m_Files_map.end(); ++it)
	{
		if (index==count)
			return it->second;
		count++;
	}
	return 0;
}
#else
CKnownFile*	CSharedFileList::GetFileByIndex(INT_PTR index){
	INT_PTR count = 0;
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
#endif

void CSharedFileList::ClearED2KPublishInfo()
{
	m_lastPublishED2KFlag = true;
#ifdef REPLACE_ATLMAP
	for (CKnownFilesMap::const_iterator it = m_Files_map.begin(); it != m_Files_map.end(); ++it)
	{
		CKnownFile* cur_file = it->second;
#else
	CKnownFile* cur_file;
	CCKey bufKey;
	for (POSITION pos = m_Files_map.GetStartPosition();pos != 0;)
	{
		m_Files_map.GetNextAssoc(pos,bufKey,cur_file);
#endif
		if (cur_file->GetPublishedED2K()) // X-Ray :: Optimizations
			cur_file->SetPublishedED2K(false);
	}
}

void CSharedFileList::ClearKadSourcePublishInfo()
{
#ifdef REPLACE_ATLMAP
	for (CKnownFilesMap::const_iterator it = m_Files_map.begin(); it != m_Files_map.end(); ++it)
		it->second->SetLastPublishTimeKadSrc(0,0);
#else
	CKnownFile* cur_file;
	CCKey bufKey;
	for (POSITION pos = m_Files_map.GetStartPosition();pos != 0;)
	{
		m_Files_map.GetNextAssoc(pos,bufKey,cur_file);
		cur_file->SetLastPublishTimeKadSrc(0,0);
	}
#endif
}

void CSharedFileList::CreateOfferedFilePacket(CKnownFile* cur_file, CSafeMemFile* files, 
											  CServer* pServer, CUpDownClient* pClient)
{
	uint_ptr uEmuleVer = (pClient && pClient->IsEmuleClient()) ? pClient->GetVersion() : 0;

	// NOTE: This function is used for creating the offered file packet for Servers _and_ for Clients..
	files->WriteHash16(cur_file->GetFileHash());

	// *) This function is used for offering files to the local server and for sending
	//    shared files to some other client. In each case we send our IP+Port only, if
	//    we have a HighID.
	// *) Newer eservers also support 2 special IP+port values which are used to hold basic file status info.
	uint32 nClientID = 0;
	uint16 nClientPort = 0;
	if (pServer)
	{
		// we use the 'TCP-compression' server feature flag as indicator for a 'newer' server.
		if (pServer->GetTCPFlags() & SRV_TCPFLG_COMPRESSION)
		{
			if (cur_file->IsPartFile())
			{
				// publishing an incomplete file
				nClientID = 0xFCFCFCFC;
				nClientPort = 0xFCFC;
			}
			else
			{
				// publishing a complete file
				nClientID = 0xFBFBFBFB;
				nClientPort = 0xFBFB;
			}
		}
		else
		{
			// check eD2K ID state
			if (theApp.serverconnect->IsConnected() && !theApp.serverconnect->IsLowID())
			{
				nClientID = theApp.GetID();
				nClientPort = thePrefs.GetPort();
			}
		}
	}
	else
	{
		if (theApp.IsConnected() && !theApp.IsFirewalled())
		{
			nClientID = theApp.GetID();
			nClientPort = thePrefs.GetPort();
		}
	}
	files->WriteUInt32(nClientID);
	files->WriteUInt16(nClientPort);
	//TRACE(_T("Publishing file: Hash=%s  ClientIP=%s  ClientPort=%u\n"), md4str(cur_file->GetFileHash()), ipstr(nClientID), nClientPort);

	CSimpleArray<CTag*> tags;

	tags.Add(new CTag(FT_FILENAME, cur_file->GetFileName()));

	if (!cur_file->IsLargeFile()){
		tags.Add(new CTag(FT_FILESIZE, (uint32)(uint64)cur_file->GetFileSize()));
	}
	else{
		// we send 2*32 bit tags to servers, but a real 64 bit tag to other clients.
		if (pServer != NULL){
			if (!pServer->SupportsLargeFilesTCP()){
				ASSERT( false );
				tags.Add(new CTag(FT_FILESIZE, 0, false));
			}
			else{
				tags.Add(new CTag(FT_FILESIZE, (uint32)(uint64)cur_file->GetFileSize()));
				tags.Add(new CTag(FT_FILESIZE_HI, (uint32)((uint64)cur_file->GetFileSize() >> 32)));
			}
		}
		else{
			if (!pClient->SupportsLargeFiles()){
				ASSERT( false );
				tags.Add(new CTag(FT_FILESIZE, 0, false));
			}
			else{
				tags.Add(new CTag(FT_FILESIZE, cur_file->GetFileSize(), true));
			}
		}
	}

	// NOTE: Archives and CD-Images are published+searched with file type "Pro"
	bool bAddedFileType = false;
	if (pServer && (pServer->GetTCPFlags() & SRV_TCPFLG_TYPETAGINTEGER)) {
		// Send integer file type tags to newer servers
		EED2KFileType eFileType = GetED2KFileTypeSearchID(GetED2KFileTypeID(cur_file->GetFileName()));
		if (eFileType >= ED2KFT_AUDIO && eFileType <= ED2KFT_CDIMAGE) {
			tags.Add(new CTag(FT_FILETYPE, (UINT)eFileType));
			bAddedFileType = true;
		}
	}
	if (!bAddedFileType) {
		// Send string file type tags to:
		//	- newer servers, in case there is no integer type available for the file type (e.g. emulecollection)
		//	- older servers
		//	- all clients
		CString strED2KFileType(GetED2KFileTypeSearchTerm(GetED2KFileTypeID(cur_file->GetFileName())));
		if (!strED2KFileType.IsEmpty()) {
			tags.Add(new CTag(FT_FILETYPE, strED2KFileType));
			bAddedFileType = true;
		}
	}

	// eserver 16.4+ does not need the FT_FILEFORMAT tag at all nor does any eMule client. This tag
	// was used for older (very old) eDonkey servers only. -> We send it only to non-eMule clients.
	if (pServer == NULL && uEmuleVer == 0) {
		int iExt = cur_file->GetFileName().ReverseFind(_T('.'));
		if (iExt != -1){
			CString strExt(cur_file->GetFileName().Mid(iExt+1));// X: [CI] - [Code Improvement]
			//if (!strExt.IsEmpty()){
				//strExt = strExt.Mid(1);
				if (!strExt.IsEmpty()){
					strExt.MakeLower();
					tags.Add(new CTag(FT_FILEFORMAT, strExt)); // file extension without a "."
				}
			//}
		}
	}

	// only send verified meta data to servers/clients
	if (cur_file->GetMetaDataVer() > 0)
	{
		static const struct
		{
			bool	bSendToServer;
			uint8	nName;
			uint8	nED2KType;
			LPCSTR	pszED2KName;
		} _aMetaTags[] = 
		{
			// Artist, Album and Title are disabled because they should be already part of the filename
			// and would therefore be redundant information sent to the servers.. and the servers count the
			// amount of sent data!
			{ false, FT_MEDIA_ARTIST,	TAGTYPE_STRING, FT_ED2K_MEDIA_ARTIST },
			{ false, FT_MEDIA_ALBUM,	TAGTYPE_STRING, FT_ED2K_MEDIA_ALBUM },
			{ false, FT_MEDIA_TITLE,	TAGTYPE_STRING, FT_ED2K_MEDIA_TITLE },
			{ true,  FT_MEDIA_LENGTH,	TAGTYPE_STRING, FT_ED2K_MEDIA_LENGTH },
			{ true,  FT_MEDIA_BITRATE,	TAGTYPE_UINT32, FT_ED2K_MEDIA_BITRATE },
			{ true,  FT_MEDIA_CODEC,	TAGTYPE_STRING, FT_ED2K_MEDIA_CODEC }
		};
		for (INT_PTR i = 0; i < ARRSIZE(_aMetaTags); i++)
		{
			if (pServer!=NULL && !_aMetaTags[i].bSendToServer)
				continue;
			CTag* pTag = cur_file->GetTag(_aMetaTags[i].nName);
			if (pTag != NULL)
			{
				// skip string tags with empty string values
				if (pTag->IsStr() && pTag->GetStr()[0] == 0)
					continue;
				
				// skip integer tags with '0' values
				if (pTag->IsInt() && pTag->GetInt() == 0)
					continue;
				
				if (_aMetaTags[i].nED2KType == TAGTYPE_STRING && pTag->IsStr())
				{
					if (pServer && (pServer->GetTCPFlags() & SRV_TCPFLG_NEWTAGS))
						tags.Add(new CTag(_aMetaTags[i].nName, pTag->GetStr()));
					else
						tags.Add(new CTag(_aMetaTags[i].pszED2KName, pTag->GetStr()));
				}
				else if (_aMetaTags[i].nED2KType == TAGTYPE_UINT32 && pTag->IsInt())
				{
					if (pServer && (pServer->GetTCPFlags() & SRV_TCPFLG_NEWTAGS))
						tags.Add(new CTag(_aMetaTags[i].nName, pTag->GetInt()));
					else
						tags.Add(new CTag(_aMetaTags[i].pszED2KName, pTag->GetInt()));
				}
				else if (_aMetaTags[i].nName == FT_MEDIA_LENGTH && pTag->IsInt())
				{
					ASSERT( _aMetaTags[i].nED2KType == TAGTYPE_STRING );
					// All 'eserver' versions and eMule versions >= 0.42.4 support the media length tag with type 'integer'
					if (   pServer!=NULL && (pServer->GetTCPFlags() & SRV_TCPFLG_COMPRESSION)
						|| uEmuleVer >= MAKE_CLIENT_VERSION(0,42,4))
					{
						if (pServer && (pServer->GetTCPFlags() & SRV_TCPFLG_NEWTAGS))
							tags.Add(new CTag(_aMetaTags[i].nName, pTag->GetInt()));
						else
							tags.Add(new CTag(_aMetaTags[i].pszED2KName, pTag->GetInt()));
					}
					else
					{
						CString strValue;
						SecToTimeLength(pTag->GetInt(), strValue);
						tags.Add(new CTag(_aMetaTags[i].pszED2KName, strValue));
					}
				}
				else
					ASSERT(0);
			}
		}
	}

	EUtf8Str eStrEncode;
	if (pServer != NULL && (pServer->GetTCPFlags() & SRV_TCPFLG_UNICODE))
		eStrEncode = utf8strRaw;
	else if (pClient && !pClient->GetUnicodeSupport())
		eStrEncode = utf8strNone;
	else
		eStrEncode = utf8strRaw;

	files->WriteUInt32(tags.GetSize());
	for (int i = 0; i < tags.GetSize(); i++)
	{
		const CTag* pTag = tags[i];
		//TRACE(_T("  %s\n"), pTag->GetFullInfo(DbgGetFileMetaTagName));
		if (pServer && (pServer->GetTCPFlags() & SRV_TCPFLG_NEWTAGS) || (uEmuleVer >= MAKE_CLIENT_VERSION(0,42,7)))
			pTag->WriteNewEd2kTag(files, eStrEncode);
		else
			pTag->WriteTagToFile(files, eStrEncode);
		delete pTag;
	}
}

// -khaos--+++> New param:  pbytesLargest, pointer to uint64.
//				Various other changes to accomodate our new statistic...
//				Point of this is to find the largest file currently shared.
uint64 CSharedFileList::GetDatasize(uint64 &pbytesLargest) const
{
	pbytesLargest=0;
	// <-----khaos-
	uint64 fsize;
	fsize=0;

#ifdef REPLACE_ATLMAP
	for (CKnownFilesMap::const_iterator it = m_Files_map.begin(); it != m_Files_map.end(); ++it)
	{
		CKnownFile* cur_file = it->second;
#else
	CCKey bufKey;
	CKnownFile* cur_file;
	for (POSITION pos = m_Files_map.GetStartPosition();pos != 0;){
		m_Files_map.GetNextAssoc(pos,bufKey,cur_file);
#endif
		fsize += (uint64)cur_file->GetFileSize();
		// -khaos--+++> If this file is bigger than all the others...well duh.
		if (cur_file->GetFileSize() > pbytesLargest)
			pbytesLargest = cur_file->GetFileSize();
		// <-----khaos-
	}
	return fsize;
}

CKnownFile* CSharedFileList::GetFileByID(const uchar* hash) const
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

CKnownFile* CSharedFileList::GetFileByIdentifier(const CFileIdentifierBase& rFileIdent, bool bStrict) const
{
	CKnownFile* pResult;
#ifdef REPLACE_ATLMAP
	CKnownFilesMap::const_iterator it = m_Files_map.find(CCKey(rFileIdent.GetMD4Hash()));
	if(it != m_Files_map.end())
	{
		pResult = it->second;
#else
	if (m_Files_map.Lookup(CCKey(rFileIdent.GetMD4Hash()), pResult))
	{
#endif
		if (bStrict)
			return pResult->GetFileIdentifier().CompareStrict(rFileIdent) ? pResult : NULL;
		else
			return pResult->GetFileIdentifier().CompareRelaxed(rFileIdent) ? pResult : NULL;
	}
	else
		return NULL;
}

bool CSharedFileList::IsFilePtrInList(const CKnownFile* file) const
{
	if (file)
	{
		CKnownFile* file2 = GetFileByID(file->GetFileHash());// X: [CI] - [Code Improvement]
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

void CSharedFileList::HashNextFile(){
	// BEGIN SLUGFILLER: SafeHash
	//Xman
	if (!theApp.emuledlg || !CemuleDlg::IsRunning() || !::IsWindow(theApp.emuledlg->m_hWnd))	// wait for the dialog to open
		return;
	if (theApp.emuledlg && CemuleDlg::IsRunning())
		theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Shared);
	if (!currentlyhashing_list.IsEmpty())	// one hash at a time
		return;
	// END SLUGFILLER: SafeHash
	if (waitingforhash_list.IsEmpty())
		return;
	UnknownFile_Struct* nextfile = waitingforhash_list.RemoveHead();
	currentlyhashing_list.AddTail(nextfile);	// SLUGFILLER: SafeHash - keep track
	(new CAddFileThread(this, nextfile->strDirectory, nextfile->strName, nextfile->strSharedDirectory))->start();
	// SLUGFILLER: SafeHash - nextfile deleting handled elsewhere
	//delete nextfile;
}

// BEGIN SLUGFILLER: SafeHash
bool CSharedFileList::IsHashing(const CString& rstrDirectory, const CString& rstrName){
	for (POSITION pos = waitingforhash_list.GetHeadPosition(); pos != 0; ){
		const UnknownFile_Struct* pFile = waitingforhash_list.GetNext(pos);
		if (!pFile->strName.CompareNoCase(rstrName) && !CompareDirectories(pFile->strDirectory, rstrDirectory))
			return true;
	}
	for (POSITION pos = currentlyhashing_list.GetHeadPosition(); pos != 0; ){
		const UnknownFile_Struct* pFile = currentlyhashing_list.GetNext(pos);
		if (!pFile->strName.CompareNoCase(rstrName) && !CompareDirectories(pFile->strDirectory, rstrDirectory))
			return true;
	}
	return false;
}

void CSharedFileList::RemoveFromHashing(CKnownFile* hashed){
	for (POSITION pos = currentlyhashing_list.GetHeadPosition(); pos != 0; ){
		POSITION posLast = pos;
		const UnknownFile_Struct* pFile = currentlyhashing_list.GetNext(pos);
		if (!pFile->strName.CompareNoCase(hashed->GetFileName()) && !CompareDirectories(pFile->strDirectory, hashed->GetPath())){
			currentlyhashing_list.RemoveAt(posLast);
			delete pFile;
			HashNextFile();			// start next hash if possible, but only if a previous hash finished
			return;
		}
	}
}

void CSharedFileList::HashFailed(UnknownFile_Struct* hashed){
	for (POSITION pos = currentlyhashing_list.GetHeadPosition(); pos != 0; ){
		POSITION posLast = pos;
		const UnknownFile_Struct* pFile = currentlyhashing_list.GetNext(pos);
		if (!pFile->strName.CompareNoCase(hashed->strName) && !CompareDirectories(pFile->strDirectory, hashed->strDirectory)){
			currentlyhashing_list.RemoveAt(posLast);
			delete pFile;
			HashNextFile();			// start next hash if possible, but only if a previous hash finished
			break;
		}
	}
	delete hashed;
}
// END SLUGFILLER: SafeHash

void CSharedFileList::UpdateFile(CKnownFile* toupdate)
{
	if(output)// X: [BF] - [Bug Fix]
	output->UpdateFile(toupdate);
}

void CSharedFileList::Process()
{
	Publish();
	if( m_lastPublishED2KFlag && ( ::GetTickCount() - m_lastPublishED2K > ED2KREPUBLISHTIME ) )
	{
		SendListToServer();
		m_lastPublishED2K = ::GetTickCount();
	}
}

void CSharedFileList::Publish()
{
	// Variables to save cpu.
	uint64 tNow = time(NULL);
	bool isFirewalled = theApp.IsFirewalled();
	bool bDirectCallback = Kademlia::CKademlia::IsRunning() && !Kademlia::CUDPFirewallTester::IsFirewalledUDP(true) && Kademlia::CUDPFirewallTester::IsVerified();

	if( Kademlia::CKademlia::IsConnected() && ( !isFirewalled || ( isFirewalled && theApp.clientlist->GetBuddyStatus() == Connected) || bDirectCallback) && GetCount() && Kademlia::CKademlia::GetPublish())
	{ 
		//We are connected to Kad. We are either open or have a buddy. And Kad is ready to start publishing.
		if( Kademlia::CKademlia::GetTotalStoreKey() < KADEMLIATOTALSTOREKEY)
		{
			//We are not at the max simultaneous keyword publishes 
			if (tNow >= m_keywords->GetNextPublishTime())
			{
				//Enough time has passed since last keyword publish

				//Get the next keyword which has to be (re)-published
				CPublishKeyword* pPubKw = m_keywords->GetNextKeyword();
				if(pPubKw)
				{
					//We have the next keyword to check if it can be published

					//Debug check to make sure things are going well.
					ASSERT( pPubKw->GetRefCount() != 0 );

					if (tNow >= pPubKw->GetNextPublishTime())
					{
						//This keyword can be published.
						Kademlia::CSearch* pSearch = Kademlia::CSearchManager::PrepareLookup(Kademlia::CSearch::STOREKEYWORD, false, pPubKw->GetKadID());
						if (pSearch)
						{
							//pSearch was created. Which means no search was already being done with this HashID.
							//This also means that it was checked to see if network load wasn't a factor.

							//This sets the filename into the search object so we can show it in the gui.
							pSearch->SetGUIName(pPubKw->GetKeyword());

							//Add all file IDs which relate to the current keyword to be published
							const CSimpleKnownFileArray& aFiles = pPubKw->GetReferences();
							uint32 count = 0;
							for (int f = 0; f < aFiles.GetSize(); f++)
							{
								//Debug check to make sure things are working well.
								ASSERT_VALID( aFiles[f] );
								// JOHNTODO - Why is this happening.. I think it may have to do with downloading a file that is already
								// in the known file list..
//								ASSERT( IsFilePtrInList(aFiles[f]) );

								//Only publish complete files as someone else should have the full file to publish these keywords.
								//As a side effect, this may help reduce people finding incomplete files in the network.
								if( !aFiles[f]->IsPartFile() && IsFilePtrInList(aFiles[f]))
								{
									count++;
									pSearch->AddFileID(Kademlia::CUInt128(aFiles[f]->GetFileHash()));
									if( count > 150 )
									{
										//We only publish up to 150 files per keyword publish then rotate the list.
										pPubKw->RotateReferences(f);
										break;
									}
								}
							}

							if( count )
							{
								//Start our keyword publish
								pPubKw->SetNextPublishTime(tNow+(KADEMLIAREPUBLISHTIMEK));
								pPubKw->IncPublishedCount();
								Kademlia::CSearchManager::StartSearch(pSearch);
							}
							else
							{
								//There were no valid files to publish with this keyword.
								delete pSearch;
							}
						}
					}
				}
				m_keywords->SetNextPublishTime(KADEMLIAPUBLISHTIME+tNow);
			}
		}
		
		if( Kademlia::CKademlia::GetTotalStoreSrc() < KADEMLIATOTALSTORESRC)
		{
			if(tNow >= m_lastPublishKadSrc)
			{
				if(m_currFileSrc > GetCount())
					m_currFileSrc = 0;
				CKnownFile* pCurKnownFile = GetFileByIndex(m_currFileSrc);
				if(pCurKnownFile)
				{
					if(pCurKnownFile->PublishSrc())
					{
						//Xman Code-Improvement: show filename immediately
						Kademlia::CSearch* pSearch = Kademlia::CSearchManager::PrepareLookup(Kademlia::CSearch::STOREFILE, true, Kademlia::CUInt128(pCurKnownFile->GetFileHash()));
						if(pSearch==NULL)
						{
							pCurKnownFile->SetLastPublishTimeKadSrc(0,0);
						}
						else
							pSearch->SetGUIName(pCurKnownFile->GetFileName());
						//Xman end
					}
				}
				m_currFileSrc++;

				// even if we did not publish a source, reset the timer so that this list is processed
				// only every KADEMLIAPUBLISHTIME seconds.
				m_lastPublishKadSrc = KADEMLIAPUBLISHTIME+tNow;
			}
		}
	}
}

void CSharedFileList::AddKeywords(CKnownFile* pFile)
{
	m_keywords->AddKeywords(pFile);
}

void CSharedFileList::RemoveKeywords(CKnownFile* pFile)
{
	m_keywords->RemoveKeywords(pFile);
}

void CSharedFileList::DeletePartFileInstances() const
{
	// this is only allowed during shut down
	ASSERT( theApp.m_app_state == APP_STATE_SHUTTINGDOWN );
	ASSERT( theApp.knownfiles );

#ifdef REPLACE_ATLMAP
	for (CKnownFilesMap::const_iterator it = m_Files_map.begin(); it != m_Files_map.end(); ++it)
	{
		CKnownFile* cur_file = it->second;
#else
	POSITION pos = m_Files_map.GetStartPosition();
	while (pos)
	{
		CCKey key;
		CKnownFile* cur_file;
		m_Files_map.GetNextAssoc(pos, key, cur_file);
#endif
		if (IsCPartFile(cur_file)/*cur_file->IsKindOf(RUNTIME_CLASS(CPartFile))*/)
		{
			if (!theApp.downloadqueue->IsPartFile(cur_file) && !theApp.knownfiles->IsFilePtrInList(cur_file))
				delete cur_file; // this is only allowed during shut down
		}
	}
}

bool CSharedFileList::IsUnsharedFile(const uchar* auFileHash) const {
	if (auFileHash){
		CSKey key(auFileHash);
#ifdef REPLACE_ATLMAP
		if(m_UnsharedFiles_map.find(key) != m_UnsharedFiles_map.end())
#else
		bool bFound;
		if (m_UnsharedFiles_map.Lookup(key, bFound))
#endif
			return true;
	}
	return false;
}

bool CSharedFileList::ShouldBeShared(CString strPath, CString strFilePath, bool bMustBeShared) const
{
	// check if this file is explicit unshared
	if (!bMustBeShared && !strFilePath.IsEmpty())// X: [QOH] - [QueryOnHashing] Exclude files move up
	{
		for (POSITION pos = m_liSingleExcludedFiles.GetHeadPosition(); pos != NULL; m_liSingleExcludedFiles.GetNext(pos) )
		{
			if (strFilePath.CompareNoCase(m_liSingleExcludedFiles.GetAt(pos)) == 0)
				return false;
		}
	}
	// determines if a file should be a shared file based on out shared directories/files preferences
	if (CompareDirectories(strPath, thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR)) == 0)
		return true;
    
	for (size_t ix=1;ix<thePrefs.GetCatCount();ix++)
	{
		if (CompareDirectories(strPath, thePrefs.GetCatPath(ix)) == 0)
			return true;		
	}

	if (bMustBeShared)
		return false;

	if (!strFilePath.IsEmpty())
	{
		// check if this file is explicit shared
		for (POSITION pos = m_liSingleSharedFiles.GetHeadPosition(); pos != NULL; m_liSingleSharedFiles.GetNext(pos) )
		{
			if (strFilePath.CompareNoCase(m_liSingleSharedFiles.GetAt(pos)) == 0)
				return true;
		}
	}

	for (POSITION pos = thePrefs.shareddir_list.GetHeadPosition();pos != 0;)
	{
		if (CompareDirectories(strPath, thePrefs.shareddir_list.GetNext(pos)) == 0)
			return true;
	}
	return false;
}

bool CSharedFileList::ContainsSingleSharedFiles(CString strDirectory) const
{
	if (strDirectory.Right(1) != _T('\\'))
		strDirectory += _T('\\');
	for (POSITION pos = m_liSingleSharedFiles.GetHeadPosition(); pos != NULL; m_liSingleSharedFiles.GetNext(pos) )
	{
		if (strDirectory.CompareNoCase(m_liSingleSharedFiles.GetAt(pos).Left(strDirectory.GetLength())) == 0)
			return true;
	}
	return false;
}

bool CSharedFileList::ExcludeFile(CString strFilePath)
{
	bool bShared = false;
	// first check if we are explicty sharing this file
	for (POSITION pos = m_liSingleSharedFiles.GetHeadPosition(); pos != NULL; m_liSingleSharedFiles.GetNext(pos) )
	{
		if (strFilePath.CompareNoCase(m_liSingleSharedFiles.GetAt(pos)) == 0)
		{
			bShared = true;
			m_liSingleSharedFiles.RemoveAt(pos);
			break;
		}
	}

	//MORPH START - Added by Stulle, Only exclude file if it was not single shared
	bool bSingleShared = bShared; 
	//MORPH END   - Added by Stulle, Only exlcude file if it was not single shared

	// check if we implicity share this file
	bShared |= ShouldBeShared(strFilePath.Left(strFilePath.ReverseFind('\\') + 1), strFilePath, false);

	if (!bShared)
	{
		// we don't actually share this file, can't be excluded
		return false;
	}
	// X: [QOH] - [QueryOnHashing] Exclude files
	/*else if (ShouldBeShared(strFilePath.Left(strFilePath.ReverseFind('\\') + 1), _T("")/*strFilePath*, true))
	{
		// we cannot unshare this file (incoming directories)
		ASSERT( false ); // checks should be done earlier already
		return false;
	}*/

	// add to exclude list
	//MORPH START - Added by Stulle, Only exclude file if it was not single shared
	if(!bSingleShared) 
	//MORPH END   - Added by Stulle, Only exlcude file if it was not single shared
		m_liSingleExcludedFiles.AddTail(strFilePath);
	
	// check if the file is in the shared list (doesn't has to for example if it is hashing or not loaded yet) and remove
#ifdef REPLACE_ATLMAP
	for (CKnownFilesMap::const_iterator it = m_Files_map.begin(); it != m_Files_map.end(); ++it)
	{
		CKnownFile* cur_file = it->second;
#else
	CKnownFile* cur_file;
	CCKey bufKey;
	for (POSITION pos = m_Files_map.GetStartPosition();pos != NULL;)
	{
		m_Files_map.GetNextAssoc(pos,bufKey,cur_file);
#endif
		if (strFilePath.CompareNoCase(cur_file->GetFilePath()) == 0) 
		{
			RemoveFile(cur_file);
			break;
		}
	}	
	// updating the GUI needs to be done by the caller
	return true;
}

bool CSharedFileList::CheckAndAddSingleFile(const CFileFind& ff, bool&bNotify){// X: [QOH] - [QueryOnHashing]
	if (ff.IsDirectory() || ff.IsDots() || ff.IsSystem() || ff.IsTemporary() || ff.GetLength()==0 || ff.GetLength()>MAX_EMULE_FILE_SIZE
		||(thePrefs.dontsharext&&checkExt(ff.GetFileName(), thePrefs.shareExt)))// X: [DSE] - [DontShareExt]
		return false;

	CString strFoundFileName(ff.GetFileName());
	CString strFoundFilePath(ff.GetFilePath());
	CString strFoundDirectory(strFoundFilePath.Left(ff.GetFilePath().ReverseFind(_T('\\')) + 1));
	CString strShellLinkDir;
	ULONGLONG ullFoundFileSize = ff.GetLength();

	// check if this file is explicit unshared
	for (POSITION pos = m_liSingleExcludedFiles.GetHeadPosition(); pos != NULL; m_liSingleExcludedFiles.GetNext(pos) )
	{
		if (strFoundFilePath.CompareNoCase(m_liSingleExcludedFiles.GetAt(pos)) == 0)
			return false;
	}


	CTime tFoundFileTime;
	try{
		ff.GetLastWriteTime(tFoundFileTime);
	}
	catch(CException* ex){
		ex->Delete();
	}

	// ignore real(!) LNK files
	TCHAR szExt[_MAX_EXT];
	_tsplitpath_s(strFoundFileName, NULL, 0, NULL, 0, NULL, 0, szExt, _countof(szExt));
	if (_tcsicmp(szExt, _T(".lnk")) == 0){
		SHFILEINFO info;
		if (SHGetFileInfo(strFoundFilePath, 0, &info, sizeof(info), SHGFI_ATTRIBUTES) && (info.dwAttributes & SFGAO_LINK)){
			if (!thePrefs.GetResolveSharedShellLinks()) {
				TRACE(_T("%hs: Did not share file \"%s\" - not supported file type\n"), __FUNCTION__, strFoundFilePath);
				return false;
			}
			// Win98: Would need to implement a different code path which is using 'IShellLinkA' on Win9x.
			CComPtr<IShellLink> pShellLink;
			if (SUCCEEDED(pShellLink.CoCreateInstance(CLSID_ShellLink))){
				CComQIPtr<IPersistFile> pPersistFile = pShellLink;
				if (pPersistFile){
					if (SUCCEEDED(pPersistFile->Load(strFoundFilePath, STGM_READ))){
						TCHAR szResolvedPath[MAX_PATH];
						if (pShellLink->GetPath(szResolvedPath, _countof(szResolvedPath), (WIN32_FIND_DATA *)NULL/*DO NOT USE (read below)*/, 0) == NOERROR){
							// WIN32_FIND_DATA povided by "IShellLink::GetPath" contains the file stats which where
							// taken when the shortcut was created! Thus the file stats which are returned do *not*
							// reflect the current real file stats. So, do *not* use that data!
							// 
							// Need to do an explicit 'FindFile' to get the current WIN32_FIND_DATA file stats.
							//
							CFileFind ffResolved;
							if (!ffResolved.FindFile(szResolvedPath))
								return false;
							VERIFY( !ffResolved.FindNextFile() );
							if (ffResolved.IsDirectory() || ffResolved.IsDots() || ffResolved.IsSystem() || ffResolved.IsTemporary() || ffResolved.GetLength() == 0 || ffResolved.GetLength() > MAX_EMULE_FILE_SIZE
								||(thePrefs.dontsharext&&checkExt(ff.GetFileName(), thePrefs.shareExt)))// X: [DSE] - [DontShareExt]
								return false;
							strShellLinkDir = strFoundDirectory;
							strFoundDirectory = ffResolved.GetRoot();
							strFoundFileName = ffResolved.GetFileName();
							strFoundFilePath = ffResolved.GetFilePath();
							ullFoundFileSize = ffResolved.GetLength();
							try {
								ffResolved.GetLastWriteTime(tFoundFileTime);
							}
							catch (CException *ex) {
								ex->Delete();
								return false;
							}
							if (strFoundDirectory.Right(1) != _T("\\"))
								strFoundDirectory += _T('\\');
						}
					}
				}
			}
		}
	}

	// ignore real(!) thumbs.db files -- seems that lot of ppl have 'thumbs.db' files without the 'System' file attribute
	if (strFoundFileName.CompareNoCase(_T("thumbs.db")) == 0)
	{
		// if that's a valid 'Storage' file, we declare it as a "thumbs.db" file.
		CComPtr<IStorage> pStorage;
		if (StgOpenStorage(strFoundFilePath, NULL, STGM_READ | STGM_SHARE_DENY_WRITE, NULL, 0, &pStorage) == S_OK)
		{
			CComPtr<IEnumSTATSTG> pEnumSTATSTG;
			if (SUCCEEDED(pStorage->EnumElements(0, NULL, 0, &pEnumSTATSTG)))
			{
				STATSTG statstg = {0};
				if (pEnumSTATSTG->Next(1, &statstg, 0) == S_OK)
				{
					CoTaskMemFree(statstg.pwcsName);
					statstg.pwcsName = NULL;
					TRACE(_T("%hs: Did not share file \"%s\" - not supported file type\n"), __FUNCTION__, strFoundFilePath);
					return false;
				}
			}
		}
	}

	uint64 fdate = tFoundFileTime.GetTime();
	if (fdate == 0){
		if (thePrefs.GetVerbose())
			AddDebugLogLine(false, _T("Failed to get file date of \"%s\""), strFoundFilePath);
	}
	else
		AdjustNTFSDaylightFileTime(fdate, strFoundFilePath);

	CKnownFile* toadd = theApp.knownfiles->FindKnownFile(strFoundFileName, fdate, ullFoundFileSize);
	if (toadd)
	{
#ifdef REPLACE_ATLMAP
		CKnownFilesMap::const_iterator it = m_Files_map.find(toadd->GetFileHash());
		if(it != m_Files_map.end())
		{
			CKnownFile* pFileInMap = it->second;
#else
		CCKey key(toadd->GetFileHash());
		CKnownFile* pFileInMap;
		if (m_Files_map.Lookup(key, pFileInMap))
		{
#endif
			TRACE(_T("%hs: File already in shared file list: %s \"%s\"\n"), __FUNCTION__, md4str(pFileInMap->GetFileHash()), pFileInMap->GetFilePath());
			TRACE(_T("%hs: File to add:                      %s \"%s\"\n"), __FUNCTION__, md4str(toadd->GetFileHash()), strFoundFilePath);
			if (!IsCPartFile(pFileInMap)/*!pFileInMap->IsKindOf(RUNTIME_CLASS(CPartFile))*/ || theApp.downloadqueue->IsPartFile(pFileInMap))
			{
				if (pFileInMap->GetFilePath().CompareNoCase(toadd->GetFilePath()) != 0) /* is it actually really the same file in the same place we already share? if so don't bother too much */
					LogWarning( GetResString(IDS_ERR_DUPL_FILES) , pFileInMap->GetFilePath(), strFoundFilePath);
				else
					DebugLog( _T("File shared twice, might have been a single shared file before - %s") , pFileInMap->GetFilePath());
			}
			return false;
		}
			if (!strShellLinkDir.IsEmpty())
				DebugLog(_T("Shared link: %s from %s"), strFoundFilePath, strShellLinkDir);
				toadd->SetPath(strFoundDirectory);
				toadd->SetFilePath(strFoundFilePath);
				toadd->SetSharedDirectory(strShellLinkDir);
				//Xman advanced upload-priority
				if(AddFile(toadd))
					toadd->CheckAUPFilestats(false);
				//Xman end
		return true;
			}
	if(bNotify)
	{
		//not in knownfilelist - start adding thread to hash file if the hashing of this file isnt already waiting
		// SLUGFILLER: SafeHash - don't double hash, MY way
		//Xman
		if (!IsHashing(strFoundDirectory, strFoundFileName) && !theApp.downloadqueue->IsTempFile(strFoundDirectory, strFoundFileName)){
		//if (!IsHashing(strFoundDirectory, strFoundFileName) && !thePrefs.IsTempFile(strFoundDirectory, strFoundFileName)){
			if(thePrefs.queryOnHashing!=0){// X: [QOH] - [QueryOnHashing]
				theApp.SplashHide(); //Xman new slpash-screen arrangement
				XMSGBOXPARAMS params;
				params.nTimeoutSeconds = 5;
				CString msg;
				msg.Format(GetResString(IDS_QUERYONHASHING),strFoundFileName,strFoundDirectory);
				int nResult = XMessageBox(theApp.emuledlg->GetSafeHwnd(), msg, _T("eMule"), MB_YESNOCANCEL | MB_DONOTASKAGAIN | MB_DEFBUTTON3 | MB_ICONQUESTION, &params);
				bool dontaskagain = (nResult & MB_DONOTASKAGAIN) != 0;
				nResult &= 0xFF;
				if(nResult != IDYES){
					if(nResult == IDNO)
					{
						m_liSingleExcludedFiles.AddTail(strFoundFilePath); // X: [QOH] - [QueryOnHashing] Exclude files
						if(dontaskagain)
							thePrefs.queryOnHashing = 1;
					}
					if(dontaskagain)
						bNotify = false;
					return false;
				}
				if(dontaskagain)
					thePrefs.queryOnHashing = 0;
			}
			UnknownFile_Struct* tohash = new UnknownFile_Struct;
			tohash->strDirectory = strFoundDirectory;
			tohash->strName = strFoundFileName;
			tohash->strSharedDirectory = strShellLinkDir;
			waitingforhash_list.AddTail(tohash);
			return true;
		}
		else
			TRACE(_T("%hs: Did not share file \"%s\" - already hashing or temp. file\n"), __FUNCTION__, strFoundFilePath);
		// SLUGFILLER: SafeHash
	}
	return false;
}

void CSharedFileList::Save() const
{
	CString strFullPath = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + SHAREDFILES_FILE;
	CStdioFile sdirfile;
	if (sdirfile.Open(strFullPath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite | CFile::typeBinary))
	{
		try{
			// write Unicode byte-order mark 0xFEFF
			WORD wBOM = 0xFEFF;
			sdirfile.Write(&wBOM, sizeof(wBOM));

			for (POSITION pos = m_liSingleSharedFiles.GetHeadPosition();pos != 0;){
				sdirfile.WriteString(m_liSingleSharedFiles.GetNext(pos));
				sdirfile.Write(L"\r\n", sizeof(TCHAR)*2);
			}
			for (POSITION pos = m_liSingleExcludedFiles.GetHeadPosition();pos != 0;){
				sdirfile.WriteString(_T("-") + m_liSingleExcludedFiles.GetNext(pos)); // a '-' prefix means excluded
				sdirfile.Write(L"\r\n", sizeof(TCHAR)*2);
			}
			if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !CemuleDlg::IsRunning())){
				sdirfile.Flush(); // flush file stream buffers to disk buffers
				if (_commit(_fileno(sdirfile.m_pStream)) != 0) // commit disk buffers to disk
					AfxThrowFileException(CFileException::hardIO, GetLastError(), sdirfile.GetFileName());
			}
			sdirfile.Close();
		}
		catch(CFileException* error){
			TCHAR buffer[MAX_CFEXP_ERRORMSG];
			error->GetErrorMessage(buffer,_countof(buffer));
			DebugLogError(L"Failed to save %s - %s", strFullPath, buffer);
			error->Delete();
		}
	}
	else
		DebugLogError(L"Failed to save %s", strFullPath);
}

void CSharedFileList::LoadSingleSharedFilesList()
{
	CString strFullPath = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + SHAREDFILES_FILE;
	CStdioFile* sdirfile = new CStdioFile();
	bool bIsUnicodeFile = IsUnicodeFile(strFullPath); // check for BOM
	if (sdirfile->Open(strFullPath, CFile::modeRead | CFile::shareDenyWrite | (bIsUnicodeFile ? CFile::typeBinary : 0)))
	{
		try {
			if (bIsUnicodeFile)
				sdirfile->Seek(sizeof(WORD), SEEK_CUR); // skip BOM

			CString toadd;
			while (sdirfile->ReadString(toadd))
			{
				toadd.Trim(L" \t\r\n"); // need to trim '\r' in binary mode
				if (toadd.IsEmpty())
					continue;

				bool bExclude = false;
				if (toadd.Left(1) == '-') // a '-' prefix means excluded
				{
					bExclude = true;
					toadd = toadd.Right(toadd.GetLength() - 1);
				}

				// Skip non-existing directories from fixed disks only
				int iDrive = PathGetDriveNumber(toadd);
				if (iDrive >= 0 && iDrive <= 25) {
					WCHAR szRootPath[4] = L" :\\";
					szRootPath[0] = (WCHAR)(L'A' + iDrive);
					if (GetDriveType(szRootPath) == DRIVE_FIXED) {
						if (_taccess(toadd, 0) != 0)
							continue;
					}
				}

				if (bExclude)
					ExcludeFile(toadd);
				else
					AddSingleSharedFile(toadd, true);
				
			}
			sdirfile->Close();
		}
		catch(CFileException* error){
			TCHAR buffer[MAX_CFEXP_ERRORMSG];
			error->GetErrorMessage(buffer,_countof(buffer));
			DebugLogError(L"Failed to load %s - %s", strFullPath, buffer);
			error->Delete();
		}
	}
	else
		DebugLogError(L"Failed to load %s", strFullPath);
	delete sdirfile;
}

bool CSharedFileList::AddSingleSharedDirectory(LPCTSTR rstrFilePath, bool bNoUpdate)
{
	ASSERT( rstrFilePath[_tcslen(rstrFilePath)-1] == _T('\\') );
	// check if we share this dir already or are not allowed to
	if (ShouldBeShared(rstrFilePath, _T(""), false) 
		// SLUGFILLER: SafeHash remove - removed installation dir unsharing
		/*
		|| !thePrefs.IsShareableDirectory(rstrFilePath)
		*/
		)
		return false;
	thePrefs.shareddir_list.AddTail(rstrFilePath); // adds the new directory as shared, GUI updates need to be done by the caller
	
	if (!bNoUpdate)
	{
		AddFilesFromDirectory(rstrFilePath, thePrefs.queryOnHashing!=1);// X: [QOH] - [QueryOnHashing]
		HashNextFile();
	}
	return true;
}

CString CSharedFileList::GetPseudoDirName(const CString& strDirectoryName)
{
	// those pseudo names are sent to other clients when requestin shared files instead of the full directory names to avoid
	// giving away too many information about our local file structure, which might be sensitive data in some cases,
	// but we still want to use a descriptive name so the information of files sorted by directories is not lost
	// So, in general we use only the name of the directory, shared subdirs keep the path up to the highest shared dir,
	// this way we never reveal the name of any not directly shared directory. We then make sure its unique.
	if (!ShouldBeShared(strDirectoryName, _T(""), false))
	{
		ASSERT( false );
		return _T("");
	}
	// does the name already exists?
#ifdef REPLACE_ATLMAP
	for (CStringToStringMap::const_iterator it = m_mapPseudoDirNames.begin(); it != m_mapPseudoDirNames.end(); ++it)
	{
		if (CompareDirectories(it->second, strDirectoryName) == 0)
		{
			// already done here
			return it->first;
		}
	}
#else
	for (POSITION pos = m_mapPseudoDirNames.GetStartPosition(); pos != NULL;)
	{
		CString strTmpPseudo;
		CString strTmpPath;
		m_mapPseudoDirNames.GetNextAssoc(pos, strTmpPseudo, strTmpPath);
		if (CompareDirectories(strTmpPath, strDirectoryName) == 0)
		{
			// already done here
			return strTmpPseudo;
		}
	}
#endif

	// create a new Pseudoname
	CString strDirectoryTmp = strDirectoryName;
	if (strDirectoryTmp.Right(1) == _T('\\'))
		strDirectoryTmp.Truncate(strDirectoryTmp.GetLength() - 1);
	
	CString strPseudoName;
	int iPos;
	while ((iPos = strDirectoryTmp.ReverseFind(_T('\\'))) != (-1))
	{
		strPseudoName = strDirectoryTmp.Right(strDirectoryTmp.GetLength() - iPos) + strPseudoName;
		strDirectoryTmp.Truncate(iPos);
		if (!ShouldBeShared(strDirectoryTmp, _T(""), false))
			break;
	}
	if (!strPseudoName.IsEmpty())
	{
		// remove first backslash
		ASSERT( strPseudoName.GetAt(0) == _T('\\') );
		strPseudoName = strPseudoName.Right(strPseudoName.GetLength() - 1);
	}
	else
	{
		// must be a rootdirectory
		ASSERT( strDirectoryTmp.GetLength() == 2 );
		strPseudoName = strDirectoryTmp;
	}
	// we have the name, make sure it is unique
#ifdef REPLACE_ATLMAP
	if(m_mapPseudoDirNames.find(strPseudoName) != m_mapPseudoDirNames.end())
#else
	if (m_mapPseudoDirNames.Lookup(strPseudoName, strDirectoryTmp))
#endif
	{
		CString strUnique;
		for (iPos = 2; ; iPos++)
		{
			strUnique.Format(_T("%s_%u"), strPseudoName, iPos);
#ifdef REPLACE_ATLMAP
			if(m_mapPseudoDirNames.find(strUnique) == m_mapPseudoDirNames.end())
#else
			if (!m_mapPseudoDirNames.Lookup(strUnique, strDirectoryTmp))
#endif
			{
				DebugLog(_T("Using Pseudoname %s for directory %s"), strUnique, strDirectoryName);
#ifdef REPLACE_ATLMAP
				m_mapPseudoDirNames[strUnique] = strDirectoryName;
#else
				m_mapPseudoDirNames.SetAt(strUnique, strDirectoryName);
#endif
				return strUnique;
			}
			else if (iPos > 200)
			{
				// wth?
				ASSERT( false );
				return _T("");
			}
		}
	}
	else
	{
		DebugLog(_T("Using Pseudoname %s for directory %s"), strPseudoName, strDirectoryName);
#ifdef REPLACE_ATLMAP
		m_mapPseudoDirNames[strPseudoName] = strDirectoryName;
#else
		m_mapPseudoDirNames.SetAt(strPseudoName, strDirectoryName);
#endif
		return strPseudoName;
	}
}

CString CSharedFileList::GetDirNameByPseudo(const CString& strPseudoName) const
{
#ifdef REPLACE_ATLMAP
	CStringToStringMap::const_iterator it = m_mapPseudoDirNames.find(strPseudoName);
	if(it != m_mapPseudoDirNames.end())
		return it->second;
	return _T("");
#else
	CString strResult;
	m_mapPseudoDirNames.Lookup(strPseudoName, strResult);
	return strResult;
#endif
}
/*
bool CSharedFileList::GetPopularityRank(const CKnownFile* pFile, uint32& rnOutSession, uint32& rnOutTotal) const
{
	rnOutSession = 0;
	rnOutTotal = 0;
	if (GetFileByIdentifier(pFile->GetFileIdentifierC()) == NULL)
	{
		ASSERT( false );
		return false;
	}
	// cycle all files, each file which has more request than the given files lowers the rank
	CKnownFile* cur_file;
#ifdef REPLACE_ATLMAP
	for (CKnownFilesMap::const_iterator it = m_Files_map.begin(); it != m_Files_map.end(); ++it)
	{
		cur_file = it->second;
#else
	CCKey bufKey;
	for (POSITION pos = m_Files_map.GetStartPosition(); pos != 0; )
	{
		m_Files_map.GetNextAssoc(pos,bufKey,cur_file);
#endif
		if (cur_file == pFile)
			continue;
		if (cur_file->statistic.GetAllTimeRequests() > pFile->statistic.GetAllTimeRequests())
			rnOutTotal++;
		if (cur_file->statistic.GetRequests() > pFile->statistic.GetRequests())
			rnOutSession++;
	}
	// we start at rank #1, not 0
	rnOutSession++;
	rnOutTotal++;
	return true;
}
*/
//Xman advanced upload-priority
void CSharedFileList::CalculateUploadPriority(bool force)
{
	static uint32 lastprocess; //if used Advanced Auto Prio
	static uint32 lastprocess2; //if Advanced Auto Prio is not used

	if(!thePrefs.UseAdvancedAutoPtio())
	{
		if(::GetTickCount() - lastprocess2 > HR2MS(1))
		{
			lastprocess2=::GetTickCount();
			//the counted upload stats must be updated from time to time, user can switch to AUP
#ifdef REPLACE_ATLMAP
			for (CKnownFilesMap::const_iterator it = m_Files_map.begin(); it != m_Files_map.end(); ++it)
				it->second->statistic.UpdateCountedTransferred();
#else
			POSITION pos = m_Files_map.GetStartPosition();
			while( pos != NULL )
			{
				CKnownFile* pFile;
				CCKey key;
				m_Files_map.GetNextAssoc( pos, key, pFile );
				pFile->statistic.UpdateCountedTransferred();
			}
#endif
		}
		return; 
	}


	if(force || ::GetTickCount() - lastprocess > MIN2MS(2))
	{
		lastprocess=::GetTickCount();
		lastprocess2=lastprocess;
		

#ifdef _DEBUG
		AddDebugLogLine(false,_T("calculating auto uploadprios. mapcount: %i"), GetCount()); 
#endif


		// v2 other avg calculation
		double sum_wanted_upload=0;
		double sum_uploaded=0;
		uint_ptr all_virtual_sources=0;
		//first loop to calculate the avg
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

			pFile->statistic.UpdateCountedTransferred();

			//we only take files > 500k into account
			if((uint64)pFile->GetFileSize() > 500*1024)
			{
				//update virtual uploadsources not in realtime
				if(pFile->IsPartFile())
					pFile->UpdateVirtualUploadSources();

				sum_wanted_upload += pFile->GetWantedUpload();
				/*
				uint64 oldtransferred ;
				if (pFile->statistic.GetAllTimeTransferred() > pFile->statistic.GetTransferred())
					oldtransferred = pFile->statistic.GetAllTimeTransferred()-pFile->statistic.GetTransferred();
				else
					oldtransferred = 0;
				

				sum_uploaded += (pFile->statistic.GetTransferred() + oldtransferred/2.0);
				*/
				sum_uploaded += pFile->statistic.GetCountedTransferred();

				all_virtual_sources += pFile->GetVirtualSourceIndicator();
			}
		}
		float avgpercent;
		if (sum_wanted_upload > 0)
			avgpercent = (float)(sum_uploaded / sum_wanted_upload * 100.0);
		else
			avgpercent = 0;

		m_lastavgPercent=avgpercent;

		if(GetCount()>0)
		{
			m_avg_virtual_sources = all_virtual_sources / (UINT_PTR)GetCount();
			m_avg_client_on_uploadqueue = theApp.uploadqueue->GetWaitingUserCount() / (UINT_PTR)GetCount();
		}
		else
		{
			m_avg_virtual_sources = 0;
			m_avg_client_on_uploadqueue = 0;
		}
		
		//end v2


		//second loop to set new prios
#ifdef REPLACE_ATLMAP
		for (CKnownFilesMap::const_iterator it = m_Files_map.begin(); it != m_Files_map.end(); ++it)
			it->second->CalculateAndSetUploadPriority();
#else
		pos = m_Files_map.GetStartPosition();
		while( pos != NULL )
		{
			CKnownFile* pFile;
			CCKey key;
			m_Files_map.GetNextAssoc( pos, key, pFile );
			pFile->CalculateAndSetUploadPriority();
		}
#endif
	}
}

void CSharedFileList::CalculateUploadPriority_Standard()
{
#ifdef REPLACE_ATLMAP
	for (CKnownFilesMap::const_iterator it = m_Files_map.begin(); it != m_Files_map.end(); ++it)
		it->second->UpdateAutoUpPriority();
#else
	POSITION pos = m_Files_map.GetStartPosition();
	while( pos != NULL )
	{
		CKnownFile* pFile;
		CCKey key;
		m_Files_map.GetNextAssoc( pos, key, pFile );
		pFile->UpdateAutoUpPriority();
	}
#endif
}
//Xman end
