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
#include <math.h>
#include <sys/stat.h>
#include <io.h>
#include <winioctl.h>
#ifndef FSCTL_SET_SPARSE
#define FSCTL_SET_SPARSE                CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 49, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#endif
#ifdef _DEBUG
#include "DebugHelpers.h"
#endif
#include "emule.h"
#include "PartFile.h"
#include "UpDownClient.h"
#include "UrlClient.h"
#include "ED2KLink.h"
#include "Preview.h"
#include "ArchiveRecovery.h"
#include "SearchFile.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "kademlia/kademlia/search.h"
#include "kademlia/kademlia/SearchManager.h"
#include "kademlia/utils/MiscUtils.h"
#include "kademlia/kademlia/prefs.h"
#include "kademlia/kademlia/Entry.h"
#include "DownloadQueue.h"
#include "IPFilter.h"
#include "MMServer.h"
#include "OtherFunctions.h"
#include "Packets.h"
#include "Preferences.h"
#include "SafeFile.h"
#include "SharedFileList.h"
#include "ListenSocket.h"
#include "Sockets.h"
#include "Server.h"
#include "KnownFileList.h"
#include "emuledlg.h"
#include "TransferDlg.h"
#include "TaskbarNotifier.h"
#include "ClientList.h"
#include "Statistics.h"
#include "shahashset.h"
#include "PeerCacheSocket.h"
#include "Log.h"
#include "CollectionViewDialog.h"
#include "Collection.h"
#include "RemoteSettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// Barry - use this constant for both places
#define PROGRESS_HEIGHT 3
// Anis
bool controlstream = false;

CBarShader CPartFile::s_LoadBar(PROGRESS_HEIGHT); // Barry - was 5
CBarShader CPartFile::s_ChunkBar(16); 

IMPLEMENT_DYNAMIC(CPartFile, CKnownFile)

CPartFile::CPartFile(UINT ucat)
{
	Init();
	m_category=ucat;
	isStreamingStopped = false;
	isStreamingPaused = false;
}

CPartFile::CPartFile(CSearchFile* searchresult, UINT cat)
{
	Init();
	isStreamingStopped = false;
	isStreamingPaused = false;
	const CTypedPtrList<CPtrList, Kademlia::CEntry*>& list = searchresult->getNotes();
	for(POSITION pos = list.GetHeadPosition(); pos != NULL; )
	{
			Kademlia::CEntry* entry = list.GetNext(pos);
			m_kadNotes.AddTail(entry->Copy());
	}
	UpdateFileRatingCommentAvail();
	
	m_FileIdentifier.SetMD4Hash(searchresult->GetFileHash());
	if (searchresult->GetFileIdentifierC().HasAICHHash())
	{
		m_FileIdentifier.SetAICHHash(searchresult->GetFileIdentifierC().GetAICHHash());
		m_pAICHRecoveryHashSet->SetMasterHash(searchresult->GetFileIdentifierC().GetAICHHash(), AICH_VERIFIED);
	}

	for (int i = 0; i < searchresult->taglist.GetCount();i++){
		const CTag* pTag = searchresult->taglist[i];
		switch (pTag->GetNameID()){
			case FT_FILENAME:{
				ASSERT( pTag->IsStr() );
				if (pTag->IsStr()){
					if (GetFileName().IsEmpty())
						SetFileName(pTag->GetStr(), true, true);
				}
				break;
			}
			case FT_FILESIZE:{
				ASSERT( pTag->IsInt64(true) );
				if (pTag->IsInt64(true))
					SetFileSize(pTag->GetInt64());
				break;
			}
			default:{
				bool bTagAdded = false;
				if (pTag->GetNameID() != 0 && pTag->GetName() == NULL && (pTag->IsStr() || pTag->IsInt()))
				{
					static const struct
					{
						uint8	nName;
						uint8	nType;
					} _aMetaTags[] = 
					{
						{ FT_MEDIA_ARTIST,  2 },
						{ FT_MEDIA_ALBUM,   2 },
						{ FT_MEDIA_TITLE,   2 },
						{ FT_MEDIA_LENGTH,  3 },
						{ FT_MEDIA_BITRATE, 3 },
						{ FT_MEDIA_CODEC,   2 },
						{ FT_FILETYPE,		2 },
						{ FT_FILEFORMAT,	2 }
					};
					for (int t = 0; t < ARRSIZE(_aMetaTags); t++)
					{
						if (pTag->GetType() == _aMetaTags[t].nType && pTag->GetNameID() == _aMetaTags[t].nName)
						{
							// skip string tags with empty string values
							if (pTag->IsStr() && pTag->GetStr().IsEmpty())
								break;

							// skip integer tags with '0' values
							if (pTag->IsInt() && pTag->GetInt() == 0)
								break;

							TRACE(_T("CPartFile::CPartFile(CSearchFile*): added tag %s\n"), pTag->GetFullInfo(DbgGetFileMetaTagName));
							CTag* newtag = new CTag(*pTag);
							taglist.Add(newtag);
							bTagAdded = true;
							break;
						}
					}
				}

				if (!bTagAdded)
					TRACE(_T("CPartFile::CPartFile(CSearchFile*): ignored tag %s\n"), pTag->GetFullInfo(DbgGetFileMetaTagName));
			}
		}
	}
	CreatePartFile(cat);
	m_category=cat;
}

CPartFile::CPartFile(CString edonkeylink, UINT cat)
{
	isStreamingStopped = false;
	isStreamingPaused = false;
	CED2KLink* pLink = 0;
	try {
		pLink = CED2KLink::CreateLinkFromUrl(edonkeylink);
		_ASSERT( pLink != 0 );
		CED2KFileLink* pFileLink = pLink->GetFileLink();
		if (pFileLink==0) 
			throw GetResString(IDS_ERR_NOTAFILELINK);
		InitializeFromLink(pFileLink,cat);
	} catch (CString error) {
		CString strMsg;
		strMsg.Format(GetResString(IDS_ERR_INVALIDLINK), error);
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_LINKERROR), strMsg);
		SetStatus(PS_ERROR);
	}
	delete pLink;
}

void CPartFile::InitializeFromLink(CED2KFileLink* fileLink, UINT cat)
{
	Init();
	try{
		SetFileName(fileLink->GetName(), true, true);
		SetFileSize(fileLink->GetSize());
		m_FileIdentifier.SetMD4Hash(fileLink->GetHashKey());
		if (fileLink->HasValidAICHHash())
		{
			m_FileIdentifier.SetAICHHash(fileLink->GetAICHHash());
			m_pAICHRecoveryHashSet->SetMasterHash(fileLink->GetAICHHash(), AICH_VERIFIED);
		}
		if (!theApp.downloadqueue->IsFileExisting(m_FileIdentifier.GetMD4Hash()))
		{
			if (fileLink->m_hashset && fileLink->m_hashset->GetLength() > 0)
			{
				try
				{
					if (!m_FileIdentifier.LoadMD4HashsetFromFile(fileLink->m_hashset, true))
					{
						ASSERT( m_FileIdentifier.GetRawMD4HashSet().GetCount() == 0 );
						AddDebugLogLine(false, _T("eD2K link \"%s\" specified with invalid hashset"), fileLink->GetName());
					}
					else
						m_bMD4HashsetNeeded = false;
				}
				catch (CFileException* e)
				{
					TCHAR szError[MAX_CFEXP_ERRORMSG];
					e->GetErrorMessage(szError, ARRSIZE(szError));
					AddDebugLogLine(false, _T("Error: Failed to process hashset for eD2K link \"%s\" - %s"), fileLink->GetName(), szError);
					e->Delete();
				}
			}
			CreatePartFile(cat);
			m_category=cat;
		}
		else
			SetStatus(PS_ERROR);
	}
	catch(CString error){
		CString strMsg;
		strMsg.Format(GetResString(IDS_ERR_INVALIDLINK), error);
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_LINKERROR), strMsg);
		SetStatus(PS_ERROR);
	}
}

CPartFile::CPartFile(CED2KFileLink* fileLink, UINT cat)
{
	isStreamingStopped = false;
	isStreamingPaused = false;
	InitializeFromLink(fileLink,cat);
}

void CPartFile::Init(){
	m_pAICHRecoveryHashSet = new CAICHRecoveryHashSet(this);
	newdate = true;
	m_LastSearchTime = 0;
	m_LastSearchTimeKad = 0;
	m_TotalSearchesKad = 0;
	lastpurgetime = ::GetTickCount();
	paused = false;
	stopped= false;
	status = PS_EMPTY;
	insufficient = false;
	m_bCompletionError = false;
	m_uTransferred = 0;
	m_iLastPausePurge = time(NULL);
	m_AllocateThread=NULL;
	m_iAllocinfo = 0;
	if(thePrefs.GetNewAutoDown())
	{
		m_iDownPriority = PR_HIGH;
		m_bAutoDownPriority = true;
	}
	else
	{
		m_iDownPriority = PR_NORMAL;
		m_bAutoDownPriority = false;
	}
	srcarevisible = false;
	memset(m_anStates,0,sizeof(m_anStates));
	datarate = 0;
	m_uMaxSources = 0;
	m_bMD4HashsetNeeded = true;
	m_bAICHPartHashsetNeeded = true;
	count = 0;
	percentcompleted = 0;
	completedsize = (uint64)0;
	m_bPreviewing = false;
	lastseencomplete = NULL;
	availablePartsCount=0;
	m_ClientSrcAnswered = 0;
	m_LastNoNeededCheck = 0;
	m_uRating = 0;
	(void)m_strComment;
	m_nTotalBufferData = 0;
	m_nLastBufferFlushTime = 0;
	m_bRecoveringArchive = false;
	m_uCompressionGain = 0;
	m_uCorruptionLoss = 0;
	m_uPartsSavedDueICH = 0;
	m_category=0;
	m_lastRefreshedDLDisplay = 0;
	m_bLocalSrcReqQueued = false;
	memset(src_stats,0,sizeof(src_stats));
	memset(net_stats,0,sizeof(net_stats));
	m_nCompleteSourcesTime = time(NULL);
	m_nCompleteSourcesCount = 0;
	m_nCompleteSourcesCountLo = 0;
	m_nCompleteSourcesCountHi = 0;
	m_dwFileAttributes = 0;
	m_bDeleteAfterAlloc=false;
	m_tActivated = 0;
	m_nDlActiveTime = 0;
	m_tLastModified = (UINT)-1;
	m_tUtcLastModified = (UINT)-1;
	m_tCreated = 0;
	m_eFileOp = PFOP_NONE;
	m_uFileOpProgress = 0;
	m_PartsHashing = 0;
    m_bpreviewprio = false;
    m_random_update_wait = (uint32)(rand()/(RAND_MAX/1000));
    lastSwapForSourceExchangeTick = ::GetTickCount();
	m_DeadSourceList.Init(false);
	m_bPauseOnPreview = false;
	//Anis -> ottimizzazione
	m_FlushThread = NULL;
	m_FlushSetting = NULL;
	m_dwAutoNNSTimer = 0;
	m_dwAutoFQSTimer = 0;
	m_dwAutoHQSTimer = 0;
	m_ShowDroppedSrc = 0;
	InChangedSharedStatusBar = false;
	m_uTotalGaps = (uint64)0;
}

CPartFile::~CPartFile()
{
	// Barry - Ensure all buffered data is written
	try{
		if (m_AllocateThread != NULL){
			HANDLE hThread = m_AllocateThread->m_hThread;
			// 2 minutes to let the thread finish
			m_AllocateThread->SetThreadPriority(THREAD_PRIORITY_NORMAL);  // MORPH like flushthread
			if (WaitForSingleObject(hThread, 120000) == WAIT_TIMEOUT){
				TerminateThread(hThread, 100);
				ASSERT(0); // did this happen why? 
			}
       	}

		//MORPH START - Changed by Anis, Flush Thread
		/*
		if (m_hpartfile.m_hFile != INVALID_HANDLE_VALUE)
			FlushBuffer(true, false, true);
		*/
		//MORPH END   - Changed by Anis, Flush Thread
	}
	catch(CFileException* e){
		e->Delete();
	}
	//MORPH START - Added by Anis, Flush Thread
	bool bNeedFlush = true;
	//moved to a separate try-catch-block to avoid skipping flushing due to an exception in the allocate-thread
	try{
		if (m_hpartfile.m_hFile != INVALID_HANDLE_VALUE)
		{
			FlushBuffer(true, false, true);
			bNeedFlush = false;
		}
	}
	catch(CFileException* e){
		e->Delete();
	}
	//this is usually done in FlushBuffer above - if not, wo do it now
	if(bNeedFlush)
	{
		try
		{
			if (m_FlushThread != NULL)
			{
				HANDLE hThread = m_FlushThread->m_hThread;
				// 2 minutes to let the thread finish - will never need that much time, just to be sure
				m_FlushThread->SetThreadPriority(THREAD_PRIORITY_NORMAL); 
				((CPartFileFlushThread*) m_FlushThread)->StopFlush();
				if (WaitForSingleObject(hThread, 120000) == WAIT_TIMEOUT)
				{
					AddDebugLogLine(true, L"Flushing (force=true) failed2.(%s)", GetFileName()/*, m_nTotalBufferData, m_BufferedData_list.GetCount(), m_uTransferred, m_nLastBufferFlushTime*/);
					TerminateThread(hThread, 100);
					ASSERT(0); // did this happen why? 
				}
				m_FlushThread = NULL;
			} 
			if (m_FlushSetting != NULL) //We normally flushed something to disk
				FlushDone();
		}
		catch(CFileException* e){
			e->Delete();
		} 
	}
	//MORPH END   - Added by Anis, Flush Thread
	ASSERT(m_FlushSetting == NULL); // flush was reported done but thread not properly ended?
	
	if (m_hpartfile.m_hFile != INVALID_HANDLE_VALUE){
		// commit file and directory entry
		m_hpartfile.Close();
		// Update met file (with current directory entry)
		SavePartFile();
	}

	POSITION pos;
	for (pos = gaplist.GetHeadPosition();pos != 0;)
		delete gaplist.GetNext(pos);

	pos = m_BufferedData_list.GetHeadPosition();
	while (pos){
		PartFileBufferedData *item = m_BufferedData_list.GetNext(pos);
		if (item->data) //MORPH - Flush Thread
		delete[] item->data;
		delete item;
	}
	delete m_pAICHRecoveryHashSet;
	m_pAICHRecoveryHashSet = NULL;

}

#ifdef _DEBUG
void CPartFile::AssertValid() const
{
	CKnownFile::AssertValid();

	(void)m_LastSearchTime;
	(void)m_LastSearchTimeKad;
	(void)m_TotalSearchesKad;
	srclist.AssertValid();
	A4AFsrclist.AssertValid();
	(void)lastseencomplete;
	m_hpartfile.AssertValid();
	m_FileCompleteMutex.AssertValid();
	(void)src_stats;
	(void)net_stats;
	CHECK_BOOL(m_bPreviewing);
	CHECK_BOOL(m_bRecoveringArchive);
	CHECK_BOOL(m_bLocalSrcReqQueued);
	CHECK_BOOL(srcarevisible);
	CHECK_BOOL(m_bMD4HashsetNeeded);
	CHECK_BOOL(m_bAICHPartHashsetNeeded);
	(void)m_iLastPausePurge;
	(void)count;
	(void)m_anStates;
	ASSERT( completedsize <= m_nFileSize );
	(void)m_uCorruptionLoss;
	(void)m_uCompressionGain;
	(void)m_uPartsSavedDueICH; 
	(void)datarate;
	(void)m_fullname;
	(void)m_partmetfilename;
	(void)m_uTransferred;
	CHECK_BOOL(paused);
	CHECK_BOOL(stopped);
	CHECK_BOOL(insufficient);
	CHECK_BOOL(m_bCompletionError);
	ASSERT( m_iDownPriority == PR_LOW || m_iDownPriority == PR_NORMAL || m_iDownPriority == PR_HIGH );
	CHECK_BOOL(m_bAutoDownPriority);
	ASSERT( status == PS_READY || status == PS_EMPTY || status == PS_WAITINGFORHASH || status == PS_ERROR || status == PS_COMPLETING || status == PS_COMPLETE );
	CHECK_BOOL(newdate);
	(void)lastpurgetime;
	(void)m_LastNoNeededCheck;
	gaplist.AssertValid();
	requestedblocks_list.AssertValid();
	m_SrcpartFrequency.AssertValid();
	ASSERT( percentcompleted >= 0.0F && percentcompleted <= 100.0F );
	corrupted_list.AssertValid();
	(void)availablePartsCount;
	(void)m_ClientSrcAnswered;
	(void)s_LoadBar;
	(void)s_ChunkBar;
	(void)m_lastRefreshedDLDisplay;
	m_downloadingSourceList.AssertValid();
	m_BufferedData_list.AssertValid();
	(void)m_nTotalBufferData;
	(void)m_nLastBufferFlushTime;
	(void)m_category;
	(void)m_dwFileAttributes;
	(void)m_dwAutoNNSTimer;
	(void)m_dwAutoFQSTimer;
	(void)m_dwAutoHQSTimer;
	(void)m_ShowDroppedSrc;
}

void CPartFile::Dump(CDumpContext& dc) const
{
	CKnownFile::Dump(dc);
}
#endif


void CPartFile::CreatePartFile(UINT cat)
{
	if (m_nFileSize > (uint64)MAX_EMULE_FILE_SIZE){
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_CREATEPARTFILE));
		SetStatus(PS_ERROR);
		return;
	}

	// decide which tempfolder to use
	CString tempdirtouse=theApp.downloadqueue->GetOptimalTempDir(cat,GetFileSize());

	// use lowest free partfilenumber for free file (InterCeptor)
	int i = 0; 
	CString filename; 
	do{
		i++; 
		filename.Format(_T("%s\\%03i.part"), tempdirtouse, i); 
	}
	while (PathFileExists(filename));
	m_partmetfilename.Format(_T("%03i.part.met"), i); 
	SetPath(tempdirtouse);
	m_fullname.Format(_T("%s\\%s"), tempdirtouse, m_partmetfilename);

	CTag* partnametag = new CTag(FT_PARTFILENAME,RemoveFileExtension(m_partmetfilename));
	taglist.Add(partnametag);
	
	Gap_Struct* gap = new Gap_Struct;
	gap->start = 0;
	gap->end = m_nFileSize - (uint64)1;
	m_uTotalGaps = m_nFileSize; //MORPH - Optimization, completedsize
	gaplist.AddTail(gap);

	CString partfull(RemoveFileExtension(m_fullname));
	SetFilePath(partfull);
	
	//Fafner: note: CFile::osRandomAccess inflates XP's file cache beyond all limits and renders systems tight on mem virtually unusable - 080229
	if (!m_hpartfile.Open(partfull,CFile::modeCreate|CFile::modeReadWrite|CFile::shareDenyWrite|CFile::osSequentialScan)){
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_CREATEPARTFILE));
		SetStatus(PS_ERROR);
	}
	else{
		if (thePrefs.GetSparsePartFiles()){
			DWORD dwReturnedBytes = 0;
			if (!DeviceIoControl(m_hpartfile.m_hFile, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &dwReturnedBytes, NULL))
			{
				// Errors:
				// ERROR_INVALID_FUNCTION	returned by WinXP when attempting to create a sparse file on a FAT32 partition
				DWORD dwError = GetLastError();
				if (dwError != ERROR_INVALID_FUNCTION && thePrefs.GetVerboseLogPriority() <= DLP_VERYLOW)
					DebugLogError(_T("Failed to apply NTFS sparse file attribute to file \"%s\" - %s"), partfull, GetErrorMessage(dwError, 1));
			}
		}

		struct _stat fileinfo;
		if (_tstat(partfull, &fileinfo) == 0){
			m_tLastModified = (uint32)fileinfo.st_mtime; //vs2005
			m_tCreated = (uint32)fileinfo.st_ctime; //vs2005
		}
		else
			AddDebugLogLine(false, _T("Failed to get file date for \"%s\" - %s"), partfull, _tcserror(errno));
	}
	m_dwFileAttributes = GetFileAttributes(partfull);
	if (m_dwFileAttributes == INVALID_FILE_ATTRIBUTES)
		m_dwFileAttributes = 0;

	if (m_FileIdentifier.GetTheoreticalMD4PartHashCount() == 0)
		m_bMD4HashsetNeeded = false;
	if (m_FileIdentifier.GetTheoreticalAICHPartHashCount() == 0)
		m_bAICHPartHashsetNeeded = false;

	// Anis: SafeHash
	// the important part
	m_PartsShareable.SetSize(GetPartCount());
	for (uint32 i = 0; i < GetPartCount();i++)
		m_PartsShareable[i] = false;
	//Anis: SafeHash

	m_SrcpartFrequency.SetSize(GetPartCount());
	for (UINT i = 0; i < GetPartCount();i++)
		m_SrcpartFrequency[i] = 0;
	//Morph Start - added by AndCycle, ICS
	// enkeyDEV: ICS
	m_SrcIncPartFrequency.SetSize(GetPartCount());
	for (UINT i = 0; i < GetPartCount();i++)
		m_SrcIncPartFrequency[i] = 0;
	// enkeyDEV: ICS
	//Morph End - added by AndCycle, ICS
	paused = false;

	if (thePrefs.AutoFilenameCleanup())
		SetFileName(CleanupFilename(GetFileName()));

	SavePartFile();
	SetActive(theApp.IsConnected());
}

/* 
* David: Lets try to import a Shareaza download ...
*
* The first part to get filename size and hash is easy 
* the secund part to get the hashset and the gap List
* is much more complicated.
*
* We could parse the whole *.sd file but I chose a other tricky way:
* To find the hashset we will search for the ed2k hash, 
* it is repeated on the begin of the hashset
* To get the gap list we will process analog 
* but now we will search for the file size.
*
*
* The *.sd file format for version 32
* [S][D][L] <-- File ID
* [20][0][0][0] <-- Version
* [FF][FE][FF][BYTE]NAME <-- len;Name 
* [QWORD] <-- Size
* [BYTE][0][0][0]SHA(20)[BYTE][0][0][0] <-- SHA Hash
* [BYTE][0][0][0]TIGER(24)[BYTE][0][0][0] <-- TIGER Hash
* [BYTE][0][0][0]MD5(16)[BYTE][0][0][0] <-- MD4 Hash
* [BYTE][0][0][0]ED2K(16)[BYTE][0][0][0] <-- ED2K Hash
* [...] <-- Saved Sources
* [QWORD][QWORD][DWORD]GAP(QWORD:QWORD)<-- Gap List: Total;Left;count;gap1(begin:length),gap2,Gap3,...
* [...] <-- Bittorent Info
* [...] <-- Tiger Tree
* [DWORD]ED2K(16)HASH1(16)HASH2(16)... <-- ED2K Hash Set: count;ed2k hash;hash1,hash2,hash3,...
* [...] <-- Comments
*/
EPartFileLoadResult CPartFile::ImportShareazaTempfile(LPCTSTR in_directory,LPCTSTR in_filename, EPartFileFormat* pOutCheckFileFormat) 
{
	CString fullname;
	fullname.Format(_T("%s\\%s"), in_directory, in_filename);

	// open the file
	CFile sdFile;
	CFileException fexpMet;
	if (!sdFile.Open(fullname, CFile::modeRead|CFile::osSequentialScan|CFile::typeBinary|CFile::shareDenyWrite, &fexpMet)){
		CString strError;
		strError.Format(GetResString(IDS_ERR_OPENMET), in_filename, _T(""));
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (fexpMet.GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(LOG_STATUSBAR, _T("%s"), strError);
		return PLR_FAILED_METFILE_NOACCESS;
	}
	//	setvbuf(sdFile.m_pStream, NULL, _IOFBF, 16384);

	try{
		CArchive ar( &sdFile, CArchive::load );

		// Is it a valid Shareaza temp file?
		CHAR szID[3];
		ar.Read( szID, 3 );
		if ( strncmp( szID, "SDL", 3 ) ){ 
			ar.Close();
			sdFile.Close();
			if (pOutCheckFileFormat != NULL)
				*pOutCheckFileFormat = PMT_UNKNOWN;
			return PLR_FAILED_OTHER;
		}

		// Get the version
		int nVersion;
		ar >> nVersion;

		// Get the File Name
		CString sRemoteName;
		ar >> sRemoteName;
		SetFileName(sRemoteName);

		// Get the File Size
		unsigned __int64 lSize;
		EMFileSize nSize;
		/*if ( nVersion >= 29 ){
			ar >> lSize;
			nSize = lSize;
		}else
			ar >> nSize;*/
		ar >> lSize;
		nSize = lSize;
		SetFileSize(nSize);

		// Get the ed2k hash
		BOOL bSHA1, bTiger, bMD5, bED2K, Trusted; bMD5 = false; bED2K = false;
		BYTE pSHA1[20];
		BYTE pTiger[24];
		BYTE pMD5[16];
		BYTE pED2K[16];

		ar >> bSHA1;
		if ( bSHA1 ) ar.Read( pSHA1, sizeof(pSHA1) );
		if ( nVersion >= 31 ) ar >> Trusted;

		ar >> bTiger;
		if ( bTiger ) ar.Read( pTiger, sizeof(pTiger) );
		if ( nVersion >= 31 ) ar >> Trusted;

		if ( nVersion >= 22 ) ar >> bMD5;
		if ( bMD5 ) ar.Read( pMD5, sizeof(pMD5) );
		if ( nVersion >= 31 ) ar >> Trusted;

		if ( nVersion >= 13 ) ar >> bED2K;
		if ( bED2K ) ar.Read( pED2K, sizeof(pED2K) );
		if ( nVersion >= 31 ) ar >> Trusted;

		ar.Close();

		if(bED2K){
			m_FileIdentifier.SetMD4Hash(pED2K);
		}else{
			Log(LOG_ERROR,GetResString(IDS_X_SHAREAZA_IMPORT_NO_HASH),in_filename);
			sdFile.Close();
			return PLR_FAILED_OTHER;
		}

		if (pOutCheckFileFormat != NULL){
			*pOutCheckFileFormat = PMT_SHAREAZA;
			return PLR_CHECKSUCCESS;
		}

		// Now the tricky part
		LONGLONG basePos = sdFile.GetPosition();

		// Try to to get the gap list
		if(gotostring(sdFile,nVersion >= 29 ? (uchar*)&lSize : (uchar*)&nSize,nVersion >= 29 ? 8 : 4)) // search the gap list
		{
			sdFile.Seek(sdFile.GetPosition()-(nVersion >= 29 ? 8 : 4),CFile::begin); // - file size
			CArchive ar( &sdFile, CArchive::load );

			bool badGapList = false;

			if( nVersion >= 29 )
			{
				__int64 nTotal, nRemaining;
				DWORD nFragments;
				ar >> nTotal >> nRemaining >> nFragments;

				if(nTotal >= nRemaining){
					__int64 begin, length;
					for (; nFragments--; ){
						ar >> begin >> length;
						if(begin + length > nTotal){
							badGapList = true;
							break;
						}
						AddGap((uint32)begin, (uint32)(begin+length-1));
					}
				}else
					badGapList = true;
			}
			else
			{
				DWORD nTotal, nRemaining;
				DWORD nFragments;
				ar >> nTotal >> nRemaining >> nFragments;

				if(nTotal >= nRemaining){
					DWORD begin, length;
					for (; nFragments--; ){
						ar >> begin >> length;
						if(begin + length > nTotal){
							badGapList = true;
							break;
						}
						AddGap(begin,begin+length-1);
					}
				}else
					badGapList = true;
			}

			if(badGapList){
				while (gaplist.GetCount()>0 ) {
					delete gaplist.GetAt(gaplist.GetHeadPosition());
					gaplist.RemoveAt(gaplist.GetHeadPosition());
				}
				Log(LOG_WARNING,GetResString(IDS_X_SHAREAZA_IMPORT_GAP_LIST_CORRUPT),in_filename);
			}

			ar.Close();
		}
		else{
			Log(LOG_WARNING,GetResString(IDS_X_SHAREAZA_IMPORT_NO_GAP_LIST),in_filename);
			sdFile.Seek(basePos,CFile::begin); // not found, reset start position
		}

		// Try to get the complete hashset
		if(gotostring(sdFile,m_FileIdentifier.GetMD4Hash(),16)) // search the hashset
		{
			sdFile.Seek(sdFile.GetPosition()-16-4,CFile::begin); // - list size - hash length
			CArchive ar( &sdFile, CArchive::load );

			DWORD nCount;
			ar >> nCount;

			BYTE pMD4[16];
			ar.Read( pMD4, sizeof(pMD4) ); // read the hash again

			// read the hashset
			for (DWORD i = 0; i < nCount; i++){
				uchar* curhash = new uchar[16];
				ar.Read( curhash, 16 );
				m_FileIdentifier.GetRawMD4HashSet().Add(curhash);
			}

			if (m_FileIdentifier.GetAvailableMD4PartHashCount() > 1)
			{
				if (!m_FileIdentifier.CalculateMD4HashByHashSet(true, true))
					Log(LOG_WARNING,GetResString(IDS_X_SHAREAZA_IMPORT_HASH_SET_CORRUPT),in_filename);
			}
			else if (m_FileIdentifier.GetTheoreticalMD4PartHashCount() != m_FileIdentifier.GetAvailableMD4PartHashCount())
			{
				Log(LOG_WARNING,GetResString(IDS_X_SHAREAZA_IMPORT_HASH_SET_CORRUPT),in_filename);
				m_FileIdentifier.DeleteMD4Hashset();
			}

			ar.Close();
		}
		else{
			Log(LOG_WARNING,GetResString(IDS_X_SHAREAZA_IMPORT_NO_HASH_SET),in_filename);
			//sdFile.Seek(basePos,CFile::begin); // not found, reset start position
		}

		// Close the file
		sdFile.Close();
	}
	catch(CArchiveException* error){
		TCHAR buffer[MAX_CFEXP_ERRORMSG];
		error->GetErrorMessage(buffer,ARRSIZE(buffer));
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_FILEERROR), in_filename, GetFileName(), buffer);
		error->Delete();
		return PLR_FAILED_OTHER;
	}
	catch(CFileException* error){
		if (error->m_cause == CFileException::endOfFile){
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_METCORRUPT), in_filename, GetFileName());
		}else{
			TCHAR buffer[MAX_CFEXP_ERRORMSG];
			error->GetErrorMessage(buffer,ARRSIZE(buffer));
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_FILEERROR), in_filename, GetFileName(), buffer);
		}
		error->Delete();
		return PLR_FAILED_OTHER;
	}
#ifndef _DEBUG
	catch(...){
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_METCORRUPT), in_filename, GetFileName());
		ASSERT(0);
		return PLR_FAILED_OTHER;
	}
#endif

	// The part below would be a copy of the CPartFile::LoadPartFile, 
	// so it is smarter to save and reload the file insta dof dougling the whole stuff
	if(!SavePartFile())
		return PLR_FAILED_OTHER;

	m_FileIdentifier.DeleteMD4Hashset();
	while (gaplist.GetCount()>0 ) {
		delete gaplist.GetAt(gaplist.GetHeadPosition());
		gaplist.RemoveAt(gaplist.GetHeadPosition());
	}

	return LoadPartFile(in_directory, in_filename);
}

EPartFileLoadResult CPartFile::LoadPartFile(LPCTSTR in_directory,LPCTSTR in_filename, EPartFileFormat* pOutCheckFileFormat)
{
	bool isnewstyle;
	uint8 version;
	EPartFileFormat partmettype = PMT_UNKNOWN;

	CMap<UINT, UINT, Gap_Struct*, Gap_Struct*> gap_map; // Anis
	m_uTransferred = 0;
	m_partmetfilename = in_filename;
	SetPath(in_directory);
	m_fullname.Format(_T("%s\\%s"), GetPath(), m_partmetfilename);
	
	// readfile data form part.met file
	CSafeBufferedFile metFile;
	CFileException fexpMet;
	if (!metFile.Open(m_fullname, CFile::modeRead|CFile::osSequentialScan|CFile::typeBinary|CFile::shareDenyWrite, &fexpMet)){
		CString strError;
		strError.Format(GetResString(IDS_ERR_OPENMET), m_partmetfilename, _T(""));
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (fexpMet.GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(LOG_STATUSBAR, _T("%s"), strError);
		return PLR_FAILED_METFILE_NOACCESS;
	}
	setvbuf(metFile.m_pStream, NULL, _IOFBF, 16384);

	try{
		version = metFile.ReadUInt8();
		
		if (version != PARTFILE_VERSION && version != PARTFILE_SPLITTEDVERSION && version != PARTFILE_VERSION_LARGEFILE){
			metFile.Close();
			if (version==83) {				
				return ImportShareazaTempfile(in_directory, in_filename, pOutCheckFileFormat);
			}
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_BADMETVERSION), m_partmetfilename, GetFileName());
			return PLR_FAILED_METFILE_CORRUPT;
		}
		
		isnewstyle = (version == PARTFILE_SPLITTEDVERSION);
		partmettype = isnewstyle ? PMT_SPLITTED : PMT_DEFAULTOLD;
		if (!isnewstyle) {
			uint8 test[4];
			metFile.Seek(24, CFile::begin);
			metFile.Read(&test[0], 1);
			metFile.Read(&test[1], 1);
			metFile.Read(&test[2], 1);
			metFile.Read(&test[3], 1);

			metFile.Seek(1, CFile::begin);

			if (test[0]==0 && test[1]==0 && test[2]==2 && test[3]==1) {
				isnewstyle = true;	// edonkeys so called "old part style"
				partmettype = PMT_NEWOLD;
			}
		}

		if (isnewstyle) {
			uint32 temp;
			metFile.Read(&temp,4);

			if (temp == 0) {	// 0.48 partmets - different again
				m_FileIdentifier.LoadMD4HashsetFromFile(&metFile, false);
			}
			else {
				uchar gethash[16];
				metFile.Seek(2, CFile::begin);
				LoadDateFromFile(&metFile);
				metFile.Read(gethash, 16);
				m_FileIdentifier.SetMD4Hash(gethash);
			}
		}
		else {
			LoadDateFromFile(&metFile);
			m_FileIdentifier.LoadMD4HashsetFromFile(&metFile, false);
		}

		bool bHadAICHHashSetTag = false;
		UINT tagcount = metFile.ReadUInt32();
		for (UINT j = 0; j < tagcount; j++){
			CTag* newtag = new CTag(&metFile, false);
			if (pOutCheckFileFormat == NULL || (pOutCheckFileFormat != NULL && (newtag->GetNameID()==FT_FILESIZE || newtag->GetNameID()==FT_FILENAME))){
			    switch (newtag->GetNameID()){
				    case FT_FILENAME:{
					    if (!newtag->IsStr()) {
						    LogError(LOG_STATUSBAR, GetResString(IDS_ERR_METCORRUPT), m_partmetfilename, GetFileName());
						    delete newtag;
						    return PLR_FAILED_METFILE_CORRUPT;
					    }
						if (GetFileName().IsEmpty())
							SetFileName(newtag->GetStr());
					    delete newtag;
					    break;
				    }
				    case FT_LASTSEENCOMPLETE:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt())
						    lastseencomplete = newtag->GetInt();
					    delete newtag;
					    break;
				    }
				    case FT_FILESIZE:{
						ASSERT( newtag->IsInt64(true) );
						if (newtag->IsInt64(true))
						    SetFileSize(newtag->GetInt64());
					    delete newtag;
					    break;
				    }
				    case FT_TRANSFERRED:{
						ASSERT( newtag->IsInt64(true) );
						if (newtag->IsInt64(true))
						    m_uTransferred = newtag->GetInt64();
					    delete newtag;
					    break;
				    }
				    case FT_COMPRESSION:{
						ASSERT( newtag->IsInt64(true) );
						if (newtag->IsInt64(true))
							m_uCompressionGain = newtag->GetInt64();
					    delete newtag;
					    break;
				    }
				    case FT_CORRUPTED:{
						ASSERT( newtag->IsInt64() );
						if (newtag->IsInt64())
							m_uCorruptionLoss = newtag->GetInt64();
					    delete newtag;
					    break;
				    }
				    case FT_FILETYPE:{
						ASSERT( newtag->IsStr() );
						if (newtag->IsStr())
						    SetFileType(newtag->GetStr());
					    delete newtag;
					    break;
				    }
				    case FT_CATEGORY:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt())
							m_category = newtag->GetInt();
					    delete newtag;
					    break;
				    }
					case FT_MAXSOURCES: {
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt())
							m_uMaxSources = newtag->GetInt();
					    delete newtag;
					    break;
				    }
				    case FT_DLPRIORITY:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt()){
							if (!isnewstyle){
								m_iDownPriority = (uint8)newtag->GetInt();
								if( m_iDownPriority == PR_AUTO ){
									m_iDownPriority = PR_HIGH;
									SetAutoDownPriority(true);
								}
								else{
									if (m_iDownPriority != PR_LOW && m_iDownPriority != PR_NORMAL && m_iDownPriority != PR_HIGH)
										m_iDownPriority = PR_NORMAL;
									SetAutoDownPriority(false);
								}
							}
						}
						delete newtag;
						break;
				    }
				    case FT_STATUS:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt()){
						    paused = newtag->GetInt()!=0;
						    stopped = paused;
						}
					    delete newtag;
					    break;
				    }
				    case FT_ULPRIORITY:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt()){
							if (!isnewstyle){
								int iUpPriority = newtag->GetInt();
								if( iUpPriority == PR_AUTO ){
									SetUpPriority(PR_HIGH, false);
									SetAutoUpPriority(true);
								}
								else{
									if (iUpPriority != PR_VERYLOW && iUpPriority != PR_LOW && iUpPriority != PR_NORMAL && iUpPriority != PR_HIGH && iUpPriority != PR_VERYHIGH)
										iUpPriority = PR_NORMAL;
									SetUpPriority((uint8)iUpPriority, false);
									SetAutoUpPriority(false);
								}
							}
						}
						delete newtag;
					    break;
				    }
				    case FT_KADLASTPUBLISHSRC:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt())
						{
						    SetLastPublishTimeKadSrc(newtag->GetInt(), 0);
							if(GetLastPublishTimeKadSrc() > (uint32)time(NULL)+theApp.rm->kadRepublishTimeS)
							{
								//There may be a posibility of an older client that saved a random number here.. This will check for that..
								SetLastPublishTimeKadSrc(0,0);
							}
						}
					    delete newtag;
					    break;
				    }
				    case FT_KADLASTPUBLISHNOTES:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt())
						{
						    SetLastPublishTimeKadNotes(newtag->GetInt());
						}
					    delete newtag;
					    break;
				    }
                    case FT_DL_PREVIEW:{
                        ASSERT( newtag->IsInt() );
						SetPreviewPrio(((newtag->GetInt() >>  0) & 0x01) == 1);
						SetPauseOnPreview(((newtag->GetInt() >>  1) & 0x01) == 1);
                        delete newtag;
                        break;
                    }

				   // statistics
					case FT_ATTRANSFERRED:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt())
							statistic.SetAllTimeTransferred(newtag->GetInt());
						delete newtag;
						break;
					}
					case FT_ATTRANSFERREDHI:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt())
						{
							uint32 hi,low;
							low = (UINT)statistic.GetAllTimeTransferred();
							hi = newtag->GetInt();
							uint64 hi2;
							hi2=hi;
							hi2=hi2<<32;
							statistic.SetAllTimeTransferred(low+hi2);
						}
						delete newtag;
						break;
					}
					case FT_ATREQUESTED:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt())
							statistic.SetAllTimeRequests(newtag->GetInt());
						delete newtag;
						break;
					}
 					case FT_ATACCEPTED:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt())
							statistic.SetAllTimeAccepts(newtag->GetInt());
						delete newtag;
						break;
					}

					// old tags: as long as they are not needed, take the chance to purge them
					case FT_PERMISSIONS:
						ASSERT( newtag->IsInt() );
						delete newtag;
						break;
					case FT_KADLASTPUBLISHKEY:
						ASSERT( newtag->IsInt() );
						delete newtag;
						break;
					case FT_DL_ACTIVE_TIME:
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt())
							m_nDlActiveTime = newtag->GetInt();
						delete newtag;
						break;
					case FT_CORRUPTEDPARTS:
						ASSERT( newtag->IsStr() );
						if (newtag->IsStr())
						{
							ASSERT( corrupted_list.GetHeadPosition() == NULL );
							CString strCorruptedParts(newtag->GetStr());
							int iPos = 0;
							CString strPart = strCorruptedParts.Tokenize(_T(","), iPos);
							while (!strPart.IsEmpty())
							{
								UINT uPart;
								if (_stscanf(strPart, _T("%u"), &uPart) == 1)
								{
									if (uPart < GetPartCount() && !IsCorruptedPart(uPart))
										corrupted_list.AddTail((uint16)uPart);
								}
								strPart = strCorruptedParts.Tokenize(_T(","), iPos);
							}
						}
						delete newtag;
						break;
					case FT_AICH_HASH:{
						ASSERT( newtag->IsStr() );
						CAICHHash hash;
						if (DecodeBase32(newtag->GetStr(), hash) == (UINT)CAICHHash::GetHashSize())
						{
							m_FileIdentifier.SetAICHHash(hash);
							m_pAICHRecoveryHashSet->SetMasterHash(hash, AICH_VERIFIED);
						}
						else
							ASSERT( false );
						delete newtag;
						break;
					}
					case FT_AICHHASHSET:
						if (newtag->IsBlob())
						{
							CSafeMemFile aichHashSetFile(newtag->GetBlob(), newtag->GetBlobSize());
							m_FileIdentifier.LoadAICHHashsetFromFile(&aichHashSetFile, false);
							aichHashSetFile.Detach();
							bHadAICHHashSetTag = true;
						}
						else
							ASSERT( false );
						delete newtag;
						break;
				    default:{
					    if (newtag->GetNameID()==0 && (newtag->GetName()[0]==FT_GAPSTART || newtag->GetName()[0]==FT_GAPEND))
						{
							ASSERT( newtag->IsInt64(true) );
							if (newtag->IsInt64(true))
							{
								Gap_Struct* gap;
								UINT gapkey = atoi(&newtag->GetName()[1]);
								if (!gap_map.Lookup(gapkey, gap))
								{
									gap = new Gap_Struct;
									gap_map.SetAt(gapkey, gap);
									gap->start = (uint64)-1;
									gap->end = (uint64)-1;
								}
								if (newtag->GetName()[0] == FT_GAPSTART)
									gap->start = newtag->GetInt64();
								if (newtag->GetName()[0] == FT_GAPEND)
									gap->end = newtag->GetInt64() - 1;
							}
						    delete newtag;
					    }
					    else
						    taglist.Add(newtag);
				    }
				}
			}
			else
				delete newtag;
		}

		//m_bAICHPartHashsetNeeded = m_FileIdentifier.GetTheoreticalAICHPartHashCount() > 0;
		if (bHadAICHHashSetTag)
		{
			if (!m_FileIdentifier.VerifyAICHHashSet())
				DebugLogError(_T("Failed to load AICH Part HashSet for part file %s"), GetFileName());
			else
			{
			//	DebugLog(_T("Succeeded to load AICH Part HashSet for file %s"), GetFileName());
				m_bAICHPartHashsetNeeded = false;
			}
		}

		// load the hashsets from the hybridstylepartmet
		if (isnewstyle && pOutCheckFileFormat == NULL && (metFile.GetPosition()<metFile.GetLength()) ) {
			uint8 temp;
			metFile.Read(&temp,1);
			
			UINT parts = GetPartCount();	// assuming we will get all hashsets
			
			for (UINT i = 0; i < parts && (metFile.GetPosition() + 16 < metFile.GetLength()); i++){
				uchar* cur_hash = new uchar[16];
				metFile.Read(cur_hash, 16);
				m_FileIdentifier.GetRawMD4HashSet().Add(cur_hash);
			}
			m_FileIdentifier.CalculateMD4HashByHashSet(true, true);
		}

		metFile.Close();
	}
	catch(CFileException* error){
		if (error->m_cause == CFileException::endOfFile)
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_METCORRUPT), m_partmetfilename, GetFileName());
		else{
			TCHAR buffer[MAX_CFEXP_ERRORMSG];
			error->GetErrorMessage(buffer,ARRSIZE(buffer));
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_FILEERROR), m_partmetfilename, GetFileName(), buffer);
		}
		error->Delete();
		return PLR_FAILED_METFILE_CORRUPT;
	}
#ifndef _DEBUG
	catch(...){
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_METCORRUPT), m_partmetfilename, GetFileName());
		ASSERT(0);
		return PLR_FAILED_METFILE_CORRUPT;
	}
#endif

	if (m_nFileSize > (uint64)MAX_EMULE_FILE_SIZE) {
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_FILEERROR), m_partmetfilename, GetFileName(), _T("File size exceeds supported limit"));
		return PLR_FAILED_OTHER;
	}

	if (pOutCheckFileFormat != NULL)
	{
		// AAARGGGHH!!!....
		*pOutCheckFileFormat = partmettype;
		return PLR_CHECKSUCCESS;
	}

	m_uTotalGaps =0; //MORPH - Optimization, completedsize

	// Now to flush the map into the list (Anis)
	for (POSITION pos = gap_map.GetStartPosition(); pos != NULL; ){
		Gap_Struct* gap;
		UINT gapkey;
		gap_map.GetNextAssoc(pos, gapkey, gap);
		// Anis: SafeHash - revised code, and extra safety
		if (gap->start != -1 && gap->end != -1 && gap->start <= gap->end && gap->start < m_nFileSize){
			if (gap->end >= m_nFileSize)
				gap->end = m_nFileSize - (uint64)1; // Clipping
			AddGap(gap->start, gap->end); // All tags accounted for, use safe adding
		}
		delete gap;
		// Anis: SafeHash
	}

	// verify corrupted parts list
	POSITION posCorruptedPart = corrupted_list.GetHeadPosition();
	while (posCorruptedPart)
	{
		POSITION posLast = posCorruptedPart;
		UINT uCorruptedPart = corrupted_list.GetNext(posCorruptedPart);
		if (IsComplete((uint64)uCorruptedPart*PARTSIZE, (uint64)(uCorruptedPart+1)*PARTSIZE-1, true))
			corrupted_list.RemoveAt(posLast);
	}

	//check if this is a backup
	if(_tcsicmp(_tcsrchr(m_fullname, _T('.')), PARTMET_TMP_EXT) == 0 || _tcsicmp(_tcsrchr(m_fullname, _T('.')), PARTMET_BAK_EXT) == 0)
	{
		m_fullname = RemoveFileExtension(m_fullname);
		m_partmetfilename = RemoveFileExtension(m_partmetfilename);
	}

	// open permanent handle
	CString searchpath(RemoveFileExtension(m_fullname));
	ASSERT( searchpath.Right(5) == _T(".part") );
	CFileException fexpPart;
	if (!m_hpartfile.Open(searchpath, CFile::modeReadWrite|CFile::shareDenyWrite|CFile::osSequentialScan, &fexpPart)){
		CString strError;
		strError.Format(GetResString(IDS_ERR_FILEOPEN), searchpath, GetFileName());
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (fexpPart.GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(LOG_STATUSBAR, _T("%s"), strError);
		return PLR_FAILED_OTHER;
	}

	// read part file creation time
	struct _stat fileinfo;
	if (_tstat(searchpath, &fileinfo) == 0){
		m_tLastModified = fileinfo.st_mtime;
		m_tCreated = fileinfo.st_ctime;
	}
	else
		AddDebugLogLine(false, _T("Failed to get file date for \"%s\" - %s"), searchpath, _tcserror(errno));

	try{
		SetFilePath(searchpath);
		m_dwFileAttributes = GetFileAttributes(GetFilePath());
		if (m_dwFileAttributes == INVALID_FILE_ATTRIBUTES)
			m_dwFileAttributes = 0;

		// Anis: SafeHash - final safety, make sure any missing part of the file is gap
		if (m_hpartfile.GetLength() < m_nFileSize)
			AddGap(m_hpartfile.GetLength(), m_nFileSize - (uint64)1);
		// Goes both ways - Partfile should never be too large
		if (m_hpartfile.GetLength() > m_nFileSize){
			TRACE(_T("Partfile \"%s\" is too large! Truncating %I64u bytes.\n"), GetFileName(), m_hpartfile.GetLength() - m_nFileSize);
			m_hpartfile.SetLength(m_nFileSize);
		}
		// Anis: SafeHash

		m_PartsShareable.SetSize(GetPartCount());
		for (UINT i = 0; i < GetPartCount();i++)
			m_PartsShareable[i] = false;

		m_SrcpartFrequency.SetSize(GetPartCount());
		m_SrcIncPartFrequency.SetSize(GetPartCount()); //>>> Anis::ICS [enkeyDEV]
		for (UINT i = 0; i < GetPartCount();i++)
		{
			m_SrcpartFrequency[i] = 0;
			m_SrcIncPartFrequency[i] = 0; //>>> Anis::ICS [enkeyDEV]
		}
		SetStatus(PS_EMPTY);
		// check hashcount, filesatus etc
		if (!m_FileIdentifier.HasExpectedMD4HashCount()){
			ASSERT( m_FileIdentifier.GetRawMD4HashSet().GetSize() == 0 );
			m_bMD4HashsetNeeded = true;
			return PLR_LOADSUCCESS;
		}
		else {
			m_bMD4HashsetNeeded = false;
			for (UINT i = 0; i < (UINT)m_FileIdentifier.GetAvailableMD4PartHashCount(); i++){
				if (i < GetPartCount() && IsComplete((uint64)i*PARTSIZE, (uint64)(i + 1)*PARTSIZE - 1, true)){
					SetStatus(PS_READY);
					break;
				}
			}
		}

		if (gaplist.IsEmpty()){	// is this file complete already?
			CompleteFile(false);
			return PLR_LOADSUCCESS;
		}

		if (!isnewstyle) // not for importing
		{
			// check date of .part file - if its wrong, rehash file
			CFileStatus filestatus;
			try{
				m_hpartfile.GetStatus(filestatus); // this; "...returns m_attribute without high-order flags" indicates a known MFC bug, wonder how many unknown there are... :)
			}
			catch(CException* ex){
				ex->Delete();
			}
			uint32 fdate = (UINT)filestatus.m_mtime.GetTime();
			if (fdate == 0)
				fdate = (UINT)-1;
			if (fdate == -1){
				if (thePrefs.GetVerbose())
					AddDebugLogLine(false, _T("Failed to get file date of \"%s\" (%s)"), filestatus.m_szFullName, GetFileName());
			}
			else
				AdjustNTFSDaylightFileTime(fdate, filestatus.m_szFullName);

			if (m_tUtcLastModified != fdate){
				CString strFileInfo;
				strFileInfo.Format(_T("%s (%s)"), GetFilePath(), GetFileName());
				LogError(LOG_STATUSBAR, GetResString(IDS_ERR_REHASH), strFileInfo);
				// rehash
				SetStatus(PS_EMPTY);	// no need to wait for hashes with the new system
				CPartHashThread* parthashthread = (CPartHashThread*) AfxBeginThread(RUNTIME_CLASS(CPartHashThread), THREAD_PRIORITY_BELOW_NORMAL,0, CREATE_SUSPENDED);
				m_PartsHashing += parthashthread->SetFirstHash(this);	// Only hashes completed parts, why hash gaps?
				parthashthread->ResumeThread();
			}
			//Added by SiRoB, Anis: SafeHash - update completed, even though unchecked
			else {
				for (UINT i = 0; i < GetPartCount(); i++)
					if (IsComplete((uint64)i*PARTSIZE,(uint64)(i+1)*PARTSIZE-1, false))
						m_PartsShareable[i] = true;
			}
		}
	}
	catch(CFileException* error){
		CString strError;
		strError.Format(_T("Failed to initialize part file \"%s\" (%s)"), m_hpartfile.GetFilePath(), GetFileName());
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (error->GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(LOG_STATUSBAR, _T("%s"), strError);
		error->Delete();
		return PLR_FAILED_OTHER;
	}

	UpdateCompletedInfos();
	return PLR_LOADSUCCESS;
}

bool CPartFile::SavePartFile(bool bDontOverrideBak)
{
	if (m_FlushSetting)
		return false;

	switch (status){
		case PS_WAITINGFORHASH:
		case PS_HASHING:
			return false;
	}

	// search part file
	CFileFind ff;
	CString searchpath(RemoveFileExtension(m_fullname));
	bool end = !ff.FindFile(searchpath,0);
	if (!end)
		ff.FindNextFile();
	if (end || ff.IsDirectory()){
		LogError(GetResString(IDS_ERR_SAVEMET) + _T(" - %s"), m_partmetfilename, GetFileName(), GetResString(IDS_ERR_PART_FNF));
		return false;
	}

	if (!m_PartsHashing){	// Anis: SafeHash - don't update the file date unless all parts are hashed
	  // get filedate
	  CTime lwtime;
	try{
		ff.GetLastWriteTime(lwtime);
	}
	catch(CException* ex){
		ex->Delete();
	  }
	m_tLastModified = (UINT)lwtime.GetTime();
	if (m_tLastModified == 0)
		m_tLastModified = (UINT)-1;
	  m_tUtcLastModified = m_tLastModified;
	  if (m_tUtcLastModified == -1){
		  if (thePrefs.GetVerbose())
			AddDebugLogLine(false, _T("Failed to get file date of \"%s\" (%s)"), m_partmetfilename, GetFileName());
	  }
	  else
		  AdjustNTFSDaylightFileTime(m_tUtcLastModified, ff.GetFilePath());
	 }	// Anis: SafeHash
	ff.Close();

	CString strTmpFile(m_fullname);
	strTmpFile += PARTMET_TMP_EXT;

	// save file data to part.met file
	CSafeBufferedFile file;
	CFileException fexp;
	if (!file.Open(strTmpFile, CFile::modeWrite|CFile::modeCreate|CFile::typeBinary|CFile::shareDenyWrite, &fexp)){
		CString strError;
		strError.Format(GetResString(IDS_ERR_SAVEMET), m_partmetfilename, GetFileName());
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(_T("%s"), strError);
		return false;
	}
	setvbuf(file.m_pStream, NULL, _IOFBF, 16384);

	try{
		//version
		// only use 64 bit tags, when PARTFILE_VERSION_LARGEFILE is set!
		file.WriteUInt8( IsLargeFile()? PARTFILE_VERSION_LARGEFILE : PARTFILE_VERSION);

		//date
		file.WriteUInt32(m_tUtcLastModified);

		//hash
		m_FileIdentifier.WriteMD4HashsetToFile(&file);

		UINT uTagCount = 0;
		ULONG uTagCountFilePos = (ULONG)file.GetPosition();
		file.WriteUInt32(uTagCount);

		CTag nametag(FT_FILENAME, GetFileName());
		nametag.WriteTagToFile(&file, utf8strOptBOM);
		uTagCount++;

		CTag sizetag(FT_FILESIZE, m_nFileSize, IsLargeFile());
		sizetag.WriteTagToFile(&file);
		uTagCount++;

		if (m_uTransferred){
			CTag transtag(FT_TRANSFERRED, m_uTransferred, IsLargeFile());
			transtag.WriteTagToFile(&file);
			uTagCount++;
		}
		if (m_uCompressionGain){
			CTag transtag(FT_COMPRESSION, m_uCompressionGain, IsLargeFile());
			transtag.WriteTagToFile(&file);
			uTagCount++;
		}
		if (m_uCorruptionLoss){
			CTag transtag(FT_CORRUPTED, m_uCorruptionLoss, IsLargeFile());
			transtag.WriteTagToFile(&file);
			uTagCount++;
		}

		if (paused){
			CTag statustag(FT_STATUS, 1);
			statustag.WriteTagToFile(&file);
			uTagCount++;
		}

		CTag prioritytag(FT_DLPRIORITY, IsAutoDownPriority() ? PR_AUTO : m_iDownPriority);
		prioritytag.WriteTagToFile(&file);
		uTagCount++;

		CTag ulprioritytag(FT_ULPRIORITY, IsAutoUpPriority() ? PR_AUTO : GetUpPriority());
		ulprioritytag.WriteTagToFile(&file);
		uTagCount++;

		if (lastseencomplete.GetTime()){
			CTag lsctag(FT_LASTSEENCOMPLETE, (UINT)lastseencomplete.GetTime());
			lsctag.WriteTagToFile(&file);
			uTagCount++;
		}

		if (m_category){
			CTag categorytag(FT_CATEGORY, m_category);
			categorytag.WriteTagToFile(&file);
			uTagCount++;
		}

		if (GetLastPublishTimeKadSrc()){
			CTag kadLastPubSrc(FT_KADLASTPUBLISHSRC, GetLastPublishTimeKadSrc());
			kadLastPubSrc.WriteTagToFile(&file);
			uTagCount++;
		}

		if (GetLastPublishTimeKadNotes()){
			CTag kadLastPubNotes(FT_KADLASTPUBLISHNOTES, GetLastPublishTimeKadNotes());
			kadLastPubNotes.WriteTagToFile(&file);
			uTagCount++;
		}

		if (GetDlActiveTime()){
			CTag tagDlActiveTime(FT_DL_ACTIVE_TIME, GetDlActiveTime());
			tagDlActiveTime.WriteTagToFile(&file);
			uTagCount++;
		}

        if (GetPreviewPrio() || IsPausingOnPreview()){
			UINT uTagValue = ((IsPausingOnPreview() ? 1 : 0) <<  1) | ((GetPreviewPrio() ? 1 : 0) <<  0);
            CTag tagDlPreview(FT_DL_PREVIEW, uTagValue);
			tagDlPreview.WriteTagToFile(&file);
			uTagCount++;
		}

		// statistics
		if (statistic.GetAllTimeTransferred()){
			CTag attag1(FT_ATTRANSFERRED, (uint32)statistic.GetAllTimeTransferred());
			attag1.WriteTagToFile(&file);
			uTagCount++;
			
			CTag attag4(FT_ATTRANSFERREDHI, (uint32)(statistic.GetAllTimeTransferred() >> 32));
			attag4.WriteTagToFile(&file);
			uTagCount++;
		}

		if (statistic.GetAllTimeRequests()){
			CTag attag2(FT_ATREQUESTED, statistic.GetAllTimeRequests());
			attag2.WriteTagToFile(&file);
			uTagCount++;
		}
		
		if (statistic.GetAllTimeAccepts()){
			CTag attag3(FT_ATACCEPTED, statistic.GetAllTimeAccepts());
			attag3.WriteTagToFile(&file);
			uTagCount++;
		}

		if (m_uMaxSources){
			CTag attag3(FT_MAXSOURCES, m_uMaxSources);
			attag3.WriteTagToFile(&file);
			uTagCount++;
		}

		// currupt part infos
        POSITION posCorruptedPart = corrupted_list.GetHeadPosition();
		if (posCorruptedPart)
		{
			CString strCorruptedParts;
			while (posCorruptedPart)
			{
				UINT uCorruptedPart = corrupted_list.GetNext(posCorruptedPart);
				if (!strCorruptedParts.IsEmpty())
					strCorruptedParts += _T(",");
				strCorruptedParts.AppendFormat(_T("%u"), (UINT)uCorruptedPart);
			}
			ASSERT( !strCorruptedParts.IsEmpty() );
			CTag tagCorruptedParts(FT_CORRUPTEDPARTS, strCorruptedParts);
			tagCorruptedParts.WriteTagToFile(&file);
			uTagCount++;
		}

		//AICH Filehash
		if (m_FileIdentifier.HasAICHHash()){
			CTag aichtag(FT_AICH_HASH, m_FileIdentifier.GetAICHHash().GetString() );
			aichtag.WriteTagToFile(&file);
			uTagCount++;

			// AICH Part HashSet
			// no point in permanently storing the AICH part hashset if we need to rehash the file anyway to fetch the full recovery hashset
			// the tag will make the known.met incompatible with emule version prior 0.44a - but that one is nearly 6 years old 
			if (m_FileIdentifier.HasExpectedAICHHashCount())
			{
				uint32 nAICHHashSetSize = (CAICHHash::GetHashSize() * (m_FileIdentifier.GetAvailableAICHPartHashCount() + 1)) + 2;
				BYTE* pHashBuffer = new BYTE[nAICHHashSetSize];
				CSafeMemFile hashSetFile(pHashBuffer, nAICHHashSetSize);
				bool bWriteHashSet = true;
				try
				{
					m_FileIdentifier.WriteAICHHashsetToFile(&hashSetFile);
				}
				catch (CFileException* pError)
				{
					ASSERT( false );
					DebugLogError(_T("Memfile Error while storing AICH Part HashSet"));
					bWriteHashSet = false;
					delete[] hashSetFile.Detach();
					pError->Delete();
				}
				if (bWriteHashSet)
				{
					CTag tagAICHHashSet(FT_AICHHASHSET, hashSetFile.Detach(), nAICHHashSetSize);
					tagAICHHashSet.WriteTagToFile(&file);
					uTagCount++;
				}
			}
		}

		for (int j = 0; j < taglist.GetCount(); j++){
			if (taglist[j]->IsStr() || taglist[j]->IsInt()){
				taglist[j]->WriteTagToFile(&file, utf8strOptBOM);
				uTagCount++;
			}
		}

		//gaps
		char namebuffer[10];
		char* number = &namebuffer[1];
		UINT i_pos = 0;
		for (POSITION pos = gaplist.GetHeadPosition(); pos != 0; )
		{
			Gap_Struct* gap = gaplist.GetNext(pos);
			_itoa(i_pos, number, 10);
			namebuffer[0] = FT_GAPSTART;
			CTag gapstarttag(namebuffer,gap->start, IsLargeFile());
			gapstarttag.WriteTagToFile(&file);
			uTagCount++;

			// gap start = first missing byte but gap ends = first non-missing byte in edonkey
			// but I think its easier to user the real limits
			namebuffer[0] = FT_GAPEND;
			CTag gapendtag(namebuffer,gap->end+1, IsLargeFile());
			gapendtag.WriteTagToFile(&file);
			uTagCount++;
			
			i_pos++;
		}
		// Add buffered data as gap too - at the time of writing this file, this data does not exists on
		// the disk, so not addding it as gaps leads to inconsistencies which causes problems in case of
		// failing to write the buffered data (for example on disk full errors)
		// don't bother with best merging too much, we do this on the next loading
		uint32 dbgMerged = 0;
		for (POSITION pos = m_BufferedData_list.GetHeadPosition(); pos != 0; )
		{
			PartFileBufferedData* gap = m_BufferedData_list.GetNext(pos);
			const uint64 nStart = gap->start;
			uint64 nEnd = gap->end;
			while (pos != 0) // merge if obvious
			{
				gap = m_BufferedData_list.GetAt(pos);
				if (gap->start == (nEnd + 1))
				{
					dbgMerged++;
					nEnd = gap->end;
					m_BufferedData_list.GetNext(pos);
				}
				else
					break;
			}

			_itoa(i_pos, number, 10);
			namebuffer[0] = FT_GAPSTART;
			CTag gapstarttag(namebuffer,nStart, IsLargeFile());
			gapstarttag.WriteTagToFile(&file);
			uTagCount++;

			// gap start = first missing byte but gap ends = first non-missing byte in edonkey
			// but I think its easier to user the real limits
			namebuffer[0] = FT_GAPEND;
			CTag gapendtag(namebuffer,nEnd+1, IsLargeFile());
			gapendtag.WriteTagToFile(&file);
			uTagCount++;
			i_pos++;
		}
		//DEBUG_ONLY( DebugLog(_T("Wrote %u buffered gaps (%u merged) for file %s"), m_BufferedData_list.GetCount(), dbgMerged, GetFileName()) );

		file.Seek(uTagCountFilePos, CFile::begin);
		file.WriteUInt32(uTagCount);
		file.SeekToEnd();

		if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !theApp.emuledlg->IsRunning())){
			file.Flush(); // flush file stream buffers to disk buffers
			if (_commit(_fileno(file.m_pStream)) != 0) // commit disk buffers to disk
				AfxThrowFileException(CFileException::hardIO, GetLastError(), file.GetFileName());
		}
		file.Close();
	}
	catch(CFileException* error){
		CString strError;
		strError.Format(GetResString(IDS_ERR_SAVEMET), m_partmetfilename, GetFileName());
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (error->GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(_T("%s"), strError);
		error->Delete();

		// remove the partially written or otherwise damaged temporary file
		file.Abort(); // need to close the file before removing it. call 'Abort' instead of 'Close', just to avoid an ASSERT.
		(void)_tremove(strTmpFile);
		return false;
	}

	// after successfully writing the temporary part.met file...
	if (_tremove(m_fullname) != 0 && errno != ENOENT){
		if (thePrefs.GetVerbose())
			DebugLogError(_T("Failed to remove \"%s\" - %s"), m_fullname, _tcserror(errno));
	}

	if (_trename(strTmpFile, m_fullname) != 0){
		int iErrno = errno;
		if (thePrefs.GetVerbose())
			DebugLogError(_T("Failed to move temporary part.met file \"%s\" to \"%s\" - %s"), strTmpFile, m_fullname, _tcserror(iErrno));

		CString strError;
		strError.Format(GetResString(IDS_ERR_SAVEMET), m_partmetfilename, GetFileName());
		strError += _T(" - ");
		strError += _tcserror(iErrno);
		LogError(_T("%s"), strError);
		return false;
	}

	// create a backup of the successfully written part.met file
	CString BAKName(m_fullname);
	BAKName.Append(PARTMET_BAK_EXT);
	if (!::CopyFile(m_fullname, BAKName, bDontOverrideBak ? TRUE : FALSE)){
		if (!bDontOverrideBak)
			DebugLogError(_T("Failed to create backup of %s (%s) - %s"), m_fullname, GetFileName(), GetErrorMessage(GetLastError()));
	}

	return true;
}

void CPartFile::PartFileHashFinished(CKnownFile* result){
	ASSERT( result->GetFileIdentifier().GetTheoreticalMD4PartHashCount() == m_FileIdentifier.GetTheoreticalMD4PartHashCount() );
	ASSERT( result->GetFileIdentifier().GetTheoreticalAICHPartHashCount() == m_FileIdentifier.GetTheoreticalAICHPartHashCount() );
	newdate = true;
	bool errorfound = false;
	// check each part
	for (uint16 nPart = 0; nPart < GetPartCount(); nPart++)
	{
		const uint64 nPartStartPos = (uint64)nPart *  PARTSIZE;
		const uint64 nPartEndPos = min(((uint64)(nPart + 1) *  PARTSIZE) - 1, GetFileSize() - (uint64)1);
		ASSERT( IsComplete(nPartStartPos, nPartEndPos, true) == IsComplete(nPartStartPos, nPartEndPos, false) );
		if (IsComplete(nPartStartPos, nPartEndPos, false))
		{
			bool bMD4Error = false;
			bool bMD4Checked = false; 
			bool bAICHError = false;
			bool bAICHChecked = false;
			// MD4
			if (nPart == 0 && m_FileIdentifier.GetTheoreticalMD4PartHashCount() == 0)
			{
				bMD4Checked = true;
				bMD4Error = md4cmp(result->GetFileIdentifier().GetMD4Hash(), GetFileIdentifier().GetMD4Hash()) != 0;
			}
			else if (m_FileIdentifier.HasExpectedMD4HashCount())
			{
				bMD4Checked = true;
				if (result->GetFileIdentifier().GetMD4PartHash(nPart) && GetFileIdentifier().GetMD4PartHash(nPart))
					bMD4Error = md4cmp(result->GetFileIdentifier().GetMD4PartHash(nPart), m_FileIdentifier.GetMD4PartHash(nPart)) != 0;
				else
					ASSERT( false );
			}
			// AICH
			if (GetFileIdentifier().HasAICHHash())
			{
				if (nPart == 0 && m_FileIdentifier.GetTheoreticalAICHPartHashCount() == 0)
				{
					bAICHChecked = true;
					bAICHError = result->GetFileIdentifier().GetAICHHash() != GetFileIdentifier().GetAICHHash();
				}
				else if (m_FileIdentifier.HasExpectedAICHHashCount())
				{
					bAICHChecked = true;
					if (result->GetFileIdentifier().GetAvailableAICHPartHashCount() > nPart && GetFileIdentifier().GetAvailableAICHPartHashCount() > nPart)
						bAICHError = result->GetFileIdentifier().GetRawAICHHashSet()[nPart] != GetFileIdentifier().GetRawAICHHashSet()[nPart];
					else
						ASSERT( false );
				}
			}
			if (bMD4Error || bAICHError)
			{
				errorfound = true;
				LogWarning(GetResString(IDS_ERR_FOUNDCORRUPTION), nPart + 1, GetFileName());
				AddGap(nPartStartPos, nPartEndPos);
				if (bMD4Checked && bAICHChecked && bMD4Error != bAICHError)
					DebugLogError(_T("AICH and MD4 HashSet disagree on verifying part %u for file %s. MD4: %s - AICH: %s"), nPart
					, GetFileName(), bMD4Error ? _T("Corrupt") : _T("OK"), bAICHError ? _T("Corrupt") : _T("OK"));
			}
		}
	}
	// missing md4 hashset?
	if (!m_FileIdentifier.HasExpectedMD4HashCount())
	{
		DebugLogError(_T("Final hashing/rehashing without valid MD4 HashSet for file %s"), GetFileName());
		// if finished we can copy over the hashset from our hashresult
		if (IsComplete(0, m_nFileSize - (uint64)1, false) &&  md4cmp(result->GetFileIdentifier().GetMD4Hash(), GetFileIdentifier().GetMD4Hash()) == 0)
		{
			if (m_FileIdentifier.SetMD4HashSet(result->GetFileIdentifier().GetRawMD4HashSet()))
				m_bMD4HashsetNeeded = false;
		}
	}

	if (!errorfound && status == PS_COMPLETING)
	{
		if (!result->GetFileIdentifier().HasAICHHash())
			AddDebugLogLine(false, _T("Failed to store new AICH Recovery and Part Hashset for completed file %s"), GetFileName());
		else
		{
			m_FileIdentifier.SetAICHHash(result->GetFileIdentifier().GetAICHHash());
			m_FileIdentifier.SetAICHHashSet(result->GetFileIdentifier());
			SetAICHRecoverHashSetAvailable(true);
		}
		m_pAICHRecoveryHashSet->FreeHashSet();
	}

	delete result;
	if (!errorfound){
		if (status == PS_COMPLETING){
			if (thePrefs.GetVerbose())
				AddDebugLogLine(true, _T("Completed file-hashing for \"%s\""), GetFileName());
			if (theApp.sharedfiles->GetFileByID(GetFileHash()) == NULL)
				theApp.sharedfiles->SafeAddKFile(this);
			CompleteFile(true);
			return;
		}
		else
			AddLogLine(false, GetResString(IDS_HASHINGDONE), GetFileName());
	}
	else{
		SetStatus(PS_READY);
		if (thePrefs.GetVerbose())
			DebugLogError(LOG_STATUSBAR, _T("File-hashing failed for \"%s\""), GetFileName());
		SavePartFile();
		return;
	}
	if (thePrefs.GetVerbose())
		AddDebugLogLine(true, _T("Completed file-hashing for \"%s\""), GetFileName());
	SetStatus(PS_READY);
	SavePartFile();
	theApp.sharedfiles->SafeAddKFile(this);
}

void CPartFile::AddGap(uint64 start, uint64 end)
{
	ASSERT( start <= end );

	POSITION pos1, pos2;
	for (pos1 = gaplist.GetHeadPosition();(pos2 = pos1) != NULL;){
		Gap_Struct* cur_gap = gaplist.GetNext(pos1);
		if (cur_gap->start >= start && cur_gap->end <= end){ // this gap is inside the new gap - delete
			m_uTotalGaps -= cur_gap->end - cur_gap->start + 1; //MORPH - Optimization, completedsize
			gaplist.RemoveAt(pos2);
			delete cur_gap;
		}
		else if (cur_gap->start >= start && cur_gap->start <= end){// a part of this gap is in the new gap - extend limit and delete
			m_uTotalGaps -= cur_gap->end - cur_gap->start + 1; //MORPH - Optimization, completedsize
			end = cur_gap->end;
			gaplist.RemoveAt(pos2);
			delete cur_gap;
		}
		else if (cur_gap->end <= end && cur_gap->end >= start){// a part of this gap is in the new gap - extend limit and delete
			m_uTotalGaps -= cur_gap->end - cur_gap->start + 1; //MORPH - Optimization, completedsize
			start = cur_gap->start;
			gaplist.RemoveAt(pos2);
			delete cur_gap;
		}
		else if (start >= cur_gap->start && end <= cur_gap->end){// new gap is already inside this gap - return
			return;
		}
	}
	m_uTotalGaps += end - start + 1; //MORPH - Optimization, completedsize
	Gap_Struct* new_gap = new Gap_Struct;
	new_gap->start = start;
	new_gap->end = end;
	gaplist.AddTail(new_gap);
	UpdateDisplayedInfo();
	newdate = true;
}

bool CPartFile::IsComplete(uint64 start, uint64 end, bool bIgnoreBufferedData) const
{
	ASSERT( start <= end );

	if (end >= m_nFileSize)
		end = m_nFileSize-(uint64)1;
	for (POSITION pos = gaplist.GetHeadPosition();pos != 0;)
	{
		const Gap_Struct* cur_gap = gaplist.GetNext(pos);
		if (   (cur_gap->start >= start          && cur_gap->end   <= end)
			|| (cur_gap->start >= start          && cur_gap->start <= end)
			|| (cur_gap->end   <= end            && cur_gap->end   >= start)
			|| (start          >= cur_gap->start && end            <= cur_gap->end)
		   )
		{
			return false;	
		}
	}

	if (bIgnoreBufferedData){
		((CPartFile*)this)->m_BufferedData_list_Locker.Lock(); //MORPH - Flush Thread
		for (POSITION pos = m_BufferedData_list.GetHeadPosition();pos != 0;)
		{
			const PartFileBufferedData* cur_gap = m_BufferedData_list.GetNext(pos);
			if (cur_gap->data) //MORPH - Flush Thread
			if (   (cur_gap->start >= start          && cur_gap->end   <= end)
				|| (cur_gap->start >= start          && cur_gap->start <= end)
				|| (cur_gap->end   <= end            && cur_gap->end   >= start)
				|| (start          >= cur_gap->start && end            <= cur_gap->end)
			)	// should be equal to if (start <= cur_gap->end  && end >= cur_gap->start)
			{
				((CPartFile*)this)->m_BufferedData_list_Locker.Unlock();//MORPH - Flush Thread
				return false;	
			}
		}
		((CPartFile*)this)->m_BufferedData_list_Locker.Unlock();//MORPH - Flush Thread
	}
	return true;
}

bool CPartFile::IsPureGap(uint64 start, uint64 end) const
{
	ASSERT( start <= end );

	if (end >= m_nFileSize)
		end = m_nFileSize-(uint64)1;
	for (POSITION pos = gaplist.GetHeadPosition();pos != 0;){
		const Gap_Struct* cur_gap = gaplist.GetNext(pos);
		if (start >= cur_gap->start  && end <= cur_gap->end ){
			return true;
		}
	}
	return false;
}

bool CPartFile::IsAlreadyRequested(uint64 start, uint64 end, bool bCheckBuffers) const
{
	ASSERT( start <= end );
	// check our requestlist
	for (POSITION pos =  requestedblocks_list.GetHeadPosition();pos != 0; ){
		const Requested_Block_Struct* cur_block = requestedblocks_list.GetNext(pos);
		if ((start <= cur_block->EndOffset) && (end >= cur_block->StartOffset))
			return true;
	}
	// check our buffers
	if (bCheckBuffers){
		for (POSITION pos =  m_BufferedData_list.GetHeadPosition();pos != 0; ){
			const PartFileBufferedData* cur_block =  m_BufferedData_list.GetNext(pos);
			if ((start <= cur_block->end) && (end >= cur_block->start)){
				DebugLogWarning(_T("CPartFile::IsAlreadyRequested, collision with buffered data found"));
				return true;
			}
		}
	}
	return false;
}

bool CPartFile::ShrinkToAvoidAlreadyRequested(uint64& start, uint64& end) const
{
	ASSERT( start <= end );
#ifdef _DEBUG
    uint64 startOrig = start;
    uint64 endOrig = end;
#endif
	for (POSITION pos =  requestedblocks_list.GetHeadPosition();pos != 0; ){
		const Requested_Block_Struct* cur_block = requestedblocks_list.GetNext(pos);
        if ((start <= cur_block->EndOffset) && (end >= cur_block->StartOffset)) {
            if(start < cur_block->StartOffset) {
                end = cur_block->StartOffset - 1;
//>>> NetFinity::DynamicBlockRequests
// Removed as it is actually a one byte request
//                if(start == end)
//                    return false;
//### NetFinity::DynamicBlockRequests
            }
			else if(end > cur_block->EndOffset) {
                start = cur_block->EndOffset + 1;
//>>> NetFinity::DynamicBlockRequests
// Removed as it is actually a one byte request
//                if(start == end) {
//                    return false;
//                }
//### NetFinity::DynamicBlockRequests
            }
			else 
                return false;
        }
	}

	// has been shrunk to fit requested, if needed shrink it further to not collidate with buffered data
	// check our buffers
	for (POSITION pos =  m_BufferedData_list.GetHeadPosition();pos != 0; ){
		const PartFileBufferedData* cur_block =  m_BufferedData_list.GetNext(pos);
		if ((start <= cur_block->end) && (end >= cur_block->start)) {
            if(start < cur_block->start) {
                end = cur_block->end - 1;
//>>> NetFinity::DynamicBlockRequests
// Removed as it is actually a one byte request
//                if(start == end)
//                    return false;
//### NetFinity::DynamicBlockRequests
            }
			else if(end > cur_block->end) {
                start = cur_block->end + 1;
//>>> NetFinity::DynamicBlockRequests
// Removed as it is actually a one byte request
//                if(start == end) {
//                    return false;
//                }
//### NetFinity::DynamicBlockRequests
            }
			else 
                return false;
        }
	}

    ASSERT(start >= startOrig && start <= endOrig);
    ASSERT(end >= startOrig && end <= endOrig);

	return true;
}

uint64 CPartFile::GetTotalGapSizeInRange(uint64 uRangeStart, uint64 uRangeEnd) const
{
	ASSERT( uRangeStart <= uRangeEnd );

	uint64 uTotalGapSize = 0;

	if (uRangeEnd >= m_nFileSize)
		uRangeEnd = m_nFileSize - (uint64)1;

	POSITION pos = gaplist.GetHeadPosition();
	while (pos)
	{
		const Gap_Struct* pGap = gaplist.GetNext(pos);

		if (pGap->start < uRangeStart && pGap->end > uRangeEnd)
		{
			uTotalGapSize += uRangeEnd - uRangeStart + 1;
			break;
		}

		if (pGap->start >= uRangeStart && pGap->start <= uRangeEnd)
		{
			uint64 uEnd = (pGap->end > uRangeEnd) ? uRangeEnd : pGap->end;
			uTotalGapSize += uEnd - pGap->start + 1;
		}
		else if (pGap->end >= uRangeStart && pGap->end <= uRangeEnd)
		{
			uTotalGapSize += pGap->end - uRangeStart + 1;
		}
	}

	ASSERT( uTotalGapSize <= uRangeEnd - uRangeStart + 1 );

	return uTotalGapSize;
}

uint64 CPartFile::GetTotalGapSizeInPart(UINT uPart) const
{
	uint64 uRangeStart = (uint64)uPart * PARTSIZE;
	uint64 uRangeEnd = uRangeStart + PARTSIZE - 1;
	if (uRangeEnd >= m_nFileSize)
		uRangeEnd = m_nFileSize;
	return GetTotalGapSizeInRange(uRangeStart, uRangeEnd);
}

//>>> NetFinity::DynamicBlockRequests -> Anis Adaptation
bool CPartFile::GetNextEmptyBlockInPart(UINT partNumber, Requested_Block_Struct *result, uint64 bytesToRequest) const 
{
	Gap_Struct *firstGap;
	Gap_Struct *currentGap;
	uint64 end;
	uint64 blockLimit;

	// Find start of this part
	uint64 partStart = PARTSIZE * (uint64)partNumber;
	uint64 start = partStart;

	// What is the end limit of this block, i.e. can't go outside part (or filesize)
	uint64 partEnd = PARTSIZE * (uint64)(partNumber + 1) - 1;
	if (partEnd >= GetFileSize())
		partEnd = GetFileSize() - (uint64)1;
	ASSERT( partStart <= partEnd );

	// Loop until find a suitable gap and return true, or no more gaps and return false
	for (;;)
	{
		firstGap = NULL;

		// Find the first gap from the start position
		for (POSITION pos = gaplist.GetHeadPosition(); pos != 0; )
		{
			currentGap = gaplist.GetNext(pos);
			// Want gaps that overlap start<->partEnd
			if ((currentGap->start <= partEnd) && (currentGap->end >= start))
			{
				// Is this the first gap?
				if ((firstGap == NULL) || (currentGap->start < firstGap->start))
					firstGap = currentGap;
			}
		}

		// If no gaps after start, exit
		if (firstGap == NULL)
			return false;

		// Update start position if gap starts after current pos
		if (start < firstGap->start)
			start = firstGap->start;

		// If this is not within part, exit
		if (start > partEnd)
			return false;

		// Find end, keeping within the max block size and the part limit
		end = firstGap->end;
		blockLimit = partStart + (uint64)((UINT)(start - partStart)/EMBLOCKSIZE + 1)*EMBLOCKSIZE - 1;
		if (end > blockLimit)
			end = blockLimit;
		if (end > partEnd)
			end = partEnd;
    
//>>> NetFinity::DynamicBlockRequests
		//Reduce bytes to request
		bytesToRequest -= bytesToRequest % 10240;
		if((start + bytesToRequest) <= end && (end - start) > (bytesToRequest + 3072)) // Avoid creating small fragments
			end = start + bytesToRequest - 1;
//### NetFinity::DynamicBlockRequests

		// If this gap has not already been requested, we have found a valid entry
		if (!IsAlreadyRequested(start, end, true))
		{
			try{
			// Was this block to be returned
			if (result != NULL)
			{
				result->StartOffset = start;
				result->EndOffset = end;
				md4cpy(result->FileID, GetFileHash());
				result->transferred = 0;
			}
			return true;
			}catch(...)
			{
				DebugLogError(_T("Error in %s (AlreadyReq)"), CString(__FUNCTION__));
			}
		}
		else
		{
        	uint64 tempStart = start;
        	uint64 tempEnd = end;

            bool shrinkSucceeded = ShrinkToAvoidAlreadyRequested(tempStart, tempEnd);
            if(shrinkSucceeded) {
//>>> Anis::FiX
		const bool realSucceeded = (start != tempStart) || (end != tempEnd);
		if(realSucceeded && thePrefs.GetLogUlDlEvents())
//### Anis::FiX
                AddDebugLogLine(false, _T("Shrunk interval to prevent collision with already requested block: Old interval %I64u-%I64u. New interval: %I64u-%I64u. File %s."), start, end, tempStart, tempEnd, GetFileName());

                // Was this block to be returned
			    if (result != NULL)
			    {
				    result->StartOffset = tempStart;
				    result->EndOffset = tempEnd;
				    md4cpy(result->FileID, GetFileHash());
				    result->transferred = 0;
			    }
			    return true;
            } else {
			    // Reposition to end of that gap
			    start = end + 1;
		    }
		}

		// If tried all gaps then break out of the loop
		if (end == partEnd)
			break;
	}

	// No suitable gap found
	return false;
}

void CPartFile::FillGap(uint64 start, uint64 end)
{
	ASSERT( start <= end );

	POSITION pos1, pos2;
	for (pos1 = gaplist.GetHeadPosition();(pos2 = pos1) != NULL;){
		Gap_Struct* cur_gap = gaplist.GetNext(pos1);
		if (cur_gap->start >= start && cur_gap->end <= end){ // our part fills this gap completly
			m_uTotalGaps -= cur_gap->end - cur_gap->start + 1; //MORPH - Optimization, completedsize
			gaplist.RemoveAt(pos2);
			delete cur_gap;
		}
		else if (cur_gap->start >= start && cur_gap->start <= end){// a part of this gap is in the part - set limit
			m_uTotalGaps -= end - cur_gap->start + 1; //MORPH - Optimization, completedsize
			cur_gap->start = end+1;
		}
		else if (cur_gap->end <= end && cur_gap->end >= start){// a part of this gap is in the part - set limit
			m_uTotalGaps -= cur_gap->end - start + 1; //MORPH - Optimization, completedsize
			cur_gap->end = start-1;
		}
		else if (start >= cur_gap->start && end <= cur_gap->end){
			m_uTotalGaps -= end - start + 1; //MORPH - Optimization, completedsize
			uint64 buffer = cur_gap->end;
			cur_gap->end = start-1;
			cur_gap = new Gap_Struct;
			cur_gap->start = end+1;
			cur_gap->end = buffer;
			gaplist.InsertAfter(pos1,cur_gap);
			break; // [Lord KiRon]
		}
	}

	UpdateCompletedInfos();
	UpdateDisplayedInfo();
	newdate = true;
}

void CPartFile::UpdateCompletedInfos()
{
#ifdef _DEBUG
	uint64 uTotalGaps = 0; 

	for (POSITION pos = gaplist.GetHeadPosition();pos !=  0;){ 
		const Gap_Struct* cur_gap = gaplist.GetNext(pos);
		uTotalGaps += cur_gap->end - cur_gap->start + 1;
	}
	if (uTotalGaps > m_nFileSize){
		ASSERT(0);
		uTotalGaps = m_nFileSize;
	}
	ASSERT(uTotalGaps == m_uTotalGaps);
#endif

	if (gaplist.GetCount() || requestedblocks_list.GetCount()){ 
		// 'percentcompleted' is only used in GUI, round down to avoid showing "100%" in case 
		// we actually have only "99.9%"
		percentcompleted = (float)(floor((1.0 - (double)m_uTotalGaps/(uint64)m_nFileSize) * 1000.0) / 10.0);
		completedsize = m_nFileSize - m_uTotalGaps;
	} 
	else{
		percentcompleted = 100.0F;
		completedsize = m_nFileSize;
	}
}

void CPartFile::DrawShareStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool bFlat) /*const*/
{ 
	if( !IsPartFile() )
	{
		CKnownFile::DrawShareStatusBar( dc, rect, onlygreyrect, bFlat );
		return;
	}
	int iWidth=rect->right - rect->left;
	if (iWidth <= 0)	return;
	int iHeight=rect->bottom - rect->top;
	if (m_bitmapSharedStatusBar == (HBITMAP)NULL)
		VERIFY(m_bitmapSharedStatusBar.CreateBitmap(1, 1, 1, 8, NULL)); 
	CDC cdcStatus;
	HGDIOBJ hOldBitmap;
	cdcStatus.CreateCompatibleDC(dc);
	if(!InChangedSharedStatusBar || lastSize!=iWidth || lastonlygreyrect!=onlygreyrect || lastbFlat!=bFlat){
		InChangedSharedStatusBar = true;
		lastSize=iWidth;
		lastonlygreyrect=onlygreyrect;
		lastbFlat=bFlat;
		m_bitmapSharedStatusBar.DeleteObject(); 
		m_bitmapSharedStatusBar.CreateCompatibleBitmap(dc,  iWidth, iHeight); 
		m_bitmapSharedStatusBar.SetBitmapDimension(iWidth,  iHeight); 
		hOldBitmap = cdcStatus.SelectObject(m_bitmapSharedStatusBar);

		const COLORREF crNotShared = RGB(224, 224, 224);
		s_ChunkBar.SetFileSize(GetFileSize());
		s_ChunkBar.SetHeight(iHeight);
		s_ChunkBar.SetWidth(iWidth);
		s_ChunkBar.Fill(crNotShared); 

		if (!onlygreyrect && !m_SrcpartFrequency.IsEmpty()) { 
			const COLORREF crMissing = RGB(255, 0, 0);
			COLORREF crProgress;
			COLORREF crHave;
			COLORREF crPending;
        	COLORREF crNooneAsked;
			if(bFlat) { 
				crProgress = RGB(0, 150, 0);
				crHave = RGB(0, 0, 0);
				crPending = RGB(255,208,0);
			    crNooneAsked = RGB(0, 0, 0);
			} else { 
				crProgress = RGB(0, 224, 0);
				crHave = RGB(104, 104, 104);
				crPending = RGB(255, 208, 0);
				crNooneAsked = RGB(104, 104, 104);
			} 
			for (uint16 i = 0; i < GetPartCount(); i++){
            //MORPH - Changed by SiRoB, SafeHash
			/*
			if(IsComplete((uint64)i*PARTSIZE,((uint64)(i+1)*PARTSIZE)-1, true)) {
			*/
			if(IsPartShareable(i)) {
    	            if(GetStatus() != PS_PAUSED || m_ClientUploadList.GetSize() > 0 || m_nCompleteSourcesCountHi > 0) {
    	                uint32 frequency;
    	                if(GetStatus() != PS_PAUSED && !m_SrcpartFrequency.IsEmpty()) {
    	                    frequency = m_SrcpartFrequency[i];
    	                } else if(!m_AvailPartFrequency.IsEmpty()) {
    	                    frequency = max(m_AvailPartFrequency[i], m_nCompleteSourcesCountLo);
    	                } else {
    	                    frequency = m_nCompleteSourcesCountLo;
    	                }

    				    if(frequency > 0 ){
				        COLORREF color = RGB(0, (22*(frequency-1) >= 210) ? 0 : 210-(22*(frequency-1)), 255);
				        s_ChunkBar.FillRange(PARTSIZE*(uint64)(i),PARTSIZE*(uint64)(i+1),color);
	                    } else {
			            s_ChunkBar.FillRange(PARTSIZE*(uint64)(i),PARTSIZE*(uint64)(i+1),crMissing);
	                    }
	                } else {
					        s_ChunkBar.FillRange(PARTSIZE*(uint64)(i),PARTSIZE*(uint64)(i+1),crNooneAsked);
					}
				}
			}
		}
   		s_ChunkBar.Draw(&cdcStatus, 0, 0, bFlat);
	}
	else
		hOldBitmap = cdcStatus.SelectObject(m_bitmapSharedStatusBar);
	dc->BitBlt(rect->left,rect->top,iWidth,iHeight,&cdcStatus,0,0,SRCCOPY);
	cdcStatus.SelectObject(hOldBitmap);
} 

void CPartFile::DrawStatusBar(CDC* dc, LPCRECT rect, bool bFlat) /*const*/
{
	COLORREF crProgress;
	COLORREF crProgressBk;
	COLORREF crHave;
	COLORREF crPending;
	COLORREF crMissing;
	EPartFileStatus eVirtualState = GetStatus();
	bool notgray = eVirtualState == PS_EMPTY || eVirtualState == PS_READY;

	if (g_bLowColorDesktop)
	{
		bFlat = true;
		// use straight Windows colors
		crProgress = RGB(0, 255, 0);
		crProgressBk = RGB(192, 192, 192);
		if (notgray) {
			crMissing = RGB(255, 0, 0);
			crHave = RGB(0, 0, 0);
			crPending = RGB(255, 255, 0);
		} else {
			crMissing = RGB(128, 0, 0);
			crHave = RGB(128, 128, 128);
			crPending = RGB(128, 128, 0);
		}
	}
	else
	{
		if (bFlat)
			crProgress = RGB(0, 150, 0);
		else
			crProgress = RGB(0, 224, 0);
		crProgressBk = RGB(224, 224, 224);
		if (notgray) {
			crMissing = RGB(255, 0, 0);
			if (bFlat) {
				crHave = RGB(0, 0, 0);
				crPending = RGB(255, 208, 0);
			} else {
				crHave = RGB(104, 104, 104);
				crPending = RGB(255, 208, 0);
			}
		} else {
			crMissing = RGB(191, 64, 64);
			if (bFlat) {
				crHave = RGB(64, 64, 64);
				crPending = RGB(191, 168, 64);
			} else {
				crHave = RGB(116, 116, 116);
				crPending = RGB(191, 168, 64);
			}
		}
	}

	s_ChunkBar.SetHeight(rect->bottom - rect->top);
	s_ChunkBar.SetWidth(rect->right - rect->left);
	s_ChunkBar.SetFileSize(m_nFileSize);
	s_ChunkBar.Fill(crHave);

	uint64 allgaps = 0; //>>> Anis Optimization
	bool bDraw = false; //>>> For Enig123

	if (status == PS_COMPLETE || status == PS_COMPLETING)
	{
		s_ChunkBar.FillRange(0, m_nFileSize, crProgress);
		s_ChunkBar.Draw(dc, rect->left, rect->top, bFlat);
		percentcompleted = 100.0F;
		completedsize = m_nFileSize;
	}
	else if (eVirtualState == PS_INSUFFICIENT || status == PS_ERROR)
	{
		int iOldBkColor = dc->SetBkColor(RGB(255, 255, 0));
		if (theApp.m_brushBackwardDiagonal.m_hObject)
			dc->FillRect(rect, &theApp.m_brushBackwardDiagonal);
		else
			dc->FillSolidRect(rect, RGB(255, 255, 0));
		dc->SetBkColor(iOldBkColor);

		UpdateCompletedInfos();
	}
	else
	{
	    // red gaps
	    uint64 allgaps = 0;
	    for (POSITION pos = gaplist.GetHeadPosition();pos !=  0;){
		    const Gap_Struct* cur_gap = gaplist.GetNext(pos);
		    allgaps += cur_gap->end - cur_gap->start + 1;
		    bool gapdone = false;
		    uint64 gapstart = cur_gap->start;
		    uint64 gapend = cur_gap->end;
		    for (UINT i = 0; i < GetPartCount(); i++){
			    if (gapstart >= (uint64)i*PARTSIZE && gapstart <= (uint64)(i+1)*PARTSIZE - 1){ // is in this part?
				    if (gapend <= (uint64)(i+1)*PARTSIZE - 1)
					    gapdone = true;
				    else
					    gapend = (uint64)(i+1)*PARTSIZE - 1; // and next part
    
				    // paint
				    COLORREF color;
				    if (m_SrcpartFrequency.GetCount() >= (INT_PTR)i && m_SrcpartFrequency[(uint16)i])
				    {
						if (g_bLowColorDesktop)
						{
							if (notgray) {
								if (m_SrcpartFrequency[(uint16)i] <= 5)
									color = RGB(0, 255, 255);
								else
									color = RGB(0, 0, 255);
							}
							else {
								color = RGB(0, 128, 128);
							}
						}
						else
						{
							if (notgray)
								color = RGB(0,
											(210 - 22*(m_SrcpartFrequency[(uint16)i] - 1) <  0) ?  0 : 210 - 22*(m_SrcpartFrequency[(uint16)i] - 1),
											255);
							else
								color = RGB(64,
											(169 - 11*(m_SrcpartFrequency[(uint16)i] - 1) < 64) ? 64 : 169 - 11*(m_SrcpartFrequency[(uint16)i] - 1),
											191);
						}
				    }
				    else
					    color = crMissing;
				    s_ChunkBar.FillRange(gapstart, gapend + 1, color);
    
				    if (gapdone) // finished?
					    break;
				    else{
					    gapstart = gapend + 1;
					    gapend = cur_gap->end;
				    }
			    }
		    }
	    }
    
	    // yellow pending parts
	    for (POSITION pos = requestedblocks_list.GetHeadPosition();pos !=  0;){
		    const Requested_Block_Struct* block = requestedblocks_list.GetNext(pos);
		    s_ChunkBar.FillRange(block->StartOffset + block->transferred, block->EndOffset + 1, crPending);
	    }
    
	    s_ChunkBar.Draw(dc, rect->left, rect->top, bFlat);
    
	    // green progress
//>>> Anis::Optimization
//	    float blockpixel = (float)(rect->right - rect->left)/(float)m_nFileSize;
//	    RECT gaprect;
//	    gaprect.top = rect->top;
//	    gaprect.bottom = gaprect.top + PROGRESS_HEIGHT;
//	    gaprect.left = rect->left;
    
//	    if (!bFlat) {
//		    s_LoadBar.SetWidth((int)( (uint64)(m_nFileSize - allgaps)*blockpixel + 0.5F));
//		    s_LoadBar.Fill(crProgress);
//		    s_LoadBar.Draw(dc, gaprect.left, gaprect.top, false);
//	    } else {
//		    gaprect.right = rect->left + (uint32)((uint64)(m_nFileSize - allgaps)*blockpixel + 0.5F);
//		    dc->FillRect(&gaprect, &CBrush(crProgress));
//		    //draw gray progress only if flat
//		    gaprect.left = gaprect.right;
//		    gaprect.right = rect->right;
//		    dc->FillRect(&gaprect, &CBrush(crProgressBk));
//	    }
//### Anis::Optimization

    }

	// additionally show any file op progress (needed for PS_COMPLETING and PS_WAITINGFORHASH)
//>>> Anis::Optimization
//	if (GetFileOp() != PFOP_NONE)
	if (bDraw) //>>> For Enig123
	{
		float blockpixel = (float)(rect->right - rect->left)/(float)m_nFileSize;
		RECT gaprect;
		gaprect.top = rect->top;
		gaprect.bottom = gaprect.top + PROGRESS_HEIGHT;
		gaprect.left = rect->left;

		if (!bFlat) {
			s_LoadBar.SetWidth((int)( (uint64)(m_nFileSize - allgaps)*blockpixel + 0.5F));
			s_LoadBar.Fill(crProgress);
			s_LoadBar.Draw(dc, gaprect.left, gaprect.top, false);
		} else {
			gaprect.right = rect->left + (uint32)((uint64)(m_nFileSize - allgaps)*blockpixel + 0.5F);
			dc->FillRect(&gaprect, &CBrush(crProgress));
			//draw gray progress only if flat
			gaprect.left = gaprect.right;
			gaprect.right = rect->right;
			dc->FillRect(&gaprect, &CBrush(crProgressBk));
		}
	}
//	else if (GetFileOp() != PFOP_NONE)
//### Anis::Optimization	
	{
		float blockpixel = (float)(rect->right - rect->left)/100.0F;
		CRect rcFileOpProgress;
		rcFileOpProgress.top = rect->top;
		rcFileOpProgress.bottom = rcFileOpProgress.top + PROGRESS_HEIGHT;
		rcFileOpProgress.left = rect->left;
		if (!bFlat)
		{
			s_LoadBar.SetWidth((int)(GetFileOpProgress()*blockpixel + 0.5F));
			s_LoadBar.Fill(RGB(255,208,0));
			s_LoadBar.Draw(dc, rcFileOpProgress.left, rcFileOpProgress.top, false);
		}
		else
		{
			rcFileOpProgress.right = rcFileOpProgress.left + (UINT)(GetFileOpProgress()*blockpixel + 0.5F);
			dc->FillRect(&rcFileOpProgress, &CBrush(RGB(255,208,0)));
			rcFileOpProgress.left = rcFileOpProgress.right;
			rcFileOpProgress.right = rect->right;
			dc->FillRect(&rcFileOpProgress, &CBrush(crProgressBk));
		}
	}
}

void CPartFile::WritePartStatus(CSafeMemFile* file) const
{
	UINT uED2KPartCount = GetED2KPartCount();
	file->WriteUInt16((uint16)uED2KPartCount);
	
	UINT uPart = 0;
	while (uPart != uED2KPartCount)
	{
		uint8 towrite = 0;
		for (UINT i = 0; i < 8; i++)
		{
			if (uPart < GetPartCount() && IsComplete((uint64)uPart*PARTSIZE, (uint64)(uPart + 1)*PARTSIZE - 1, true))
				towrite |= (1 << i);
			uPart++;
			if (uPart == uED2KPartCount)
				break;
		}
		file->WriteUInt8(towrite);
	}
}

void CPartFile::WriteCompleteSourcesCount(CSafeMemFile* file) const
{
	file->WriteUInt16(m_nCompleteSourcesCount);
}

int CPartFile::GetValidSourcesCount() const
{
	int counter = 0;
	for (POSITION pos = srclist.GetHeadPosition(); pos != NULL;){
		EDownloadState nDLState = srclist.GetNext(pos)->GetDownloadState();
		if (nDLState==DS_ONQUEUE || nDLState==DS_DOWNLOADING || nDLState==DS_CONNECTED || nDLState==DS_REMOTEQUEUEFULL)
			++counter;
	}
	return counter;
}

UINT CPartFile::GetNotCurrentSourcesCount() const
{
	UINT counter = 0;
	for (POSITION pos = srclist.GetHeadPosition(); pos != NULL;){
		EDownloadState nDLState = srclist.GetNext(pos)->GetDownloadState();
		if (nDLState!=DS_ONQUEUE && nDLState!=DS_DOWNLOADING)
			counter++;
	}
	return counter;
}

uint64 CPartFile::GetNeededSpace() const
{
	if (m_hpartfile.GetLength() > GetFileSize())
		return 0;	// Shouldn't happen, but just in case
	return GetFileSize() - m_hpartfile.GetLength();
}

EPartFileStatus CPartFile::GetStatus(bool ignorepause) const
{
	if ((!paused && !insufficient) || status == PS_ERROR || status == PS_COMPLETING || status == PS_COMPLETE || ignorepause)
		return status;
	else if (paused)
		return PS_PAUSED;
	else
		return PS_INSUFFICIENT;
}

void CPartFile::AddDownloadingSource(CUpDownClient* client){
	POSITION pos = m_downloadingSourceList.Find(client); // to be sure
	if(pos == NULL){
		m_downloadingSourceList.AddTail(client);
		theApp.emuledlg->transferwnd->GetDownloadClientsList()->AddClient(client);
	}
}

void CPartFile::RemoveDownloadingSource(CUpDownClient* client){
	POSITION pos = m_downloadingSourceList.Find(client); // to be sure
	if(pos != NULL){
		m_downloadingSourceList.RemoveAt(pos);
		theApp.emuledlg->transferwnd->GetDownloadClientsList()->RemoveClient(client);
	}
}

takelimit CPartFile::Process(uint32 reducedownload, uint32 reducedownloadext,uint32 reducedownloadkadu, UINT icounter/*in percent*/)
{
	if (thePrefs.m_iDbgHeap >= 2)
		ASSERT_VALID(this);

	UINT nOldTransSourceCount = GetSrcStatisticsValue(DS_DOWNLOADING);
	DWORD dwCurTick = ::GetTickCount();
	if (dwCurTick < m_nLastBufferFlushTime)
	{
		ASSERT( false );
		m_nLastBufferFlushTime = dwCurTick;
	}

	// If buffer size exceeds limit, or if not written within time limit, flush data
	if ((m_nTotalBufferData > thePrefs.GetFileBufferSize()) || (dwCurTick > (m_nLastBufferFlushTime + thePrefs.GetFileBufferTimeLimit())))
	{
		// Avoid flushing while copying preview file
		if (!m_bPreviewing)
			FlushBuffer();
	}

	datarate = 0;
	datarateext = 0;
	dataratekadu = 0;

	// calculate datarate, set limit etc.
	if(icounter < 10)
	{
		uint32 cur_datarate;
		for(POSITION pos = m_downloadingSourceList.GetHeadPosition();pos!=0;)
		{
			CUpDownClient* cur_src = m_downloadingSourceList.GetNext(pos);
			if (thePrefs.m_iDbgHeap >= 2)
				ASSERT_VALID( cur_src );
			if(cur_src && cur_src->GetDownloadState() == DS_DOWNLOADING)
			{
				ASSERT( cur_src->socket );
				if (cur_src->socket)
				{
					if(!cur_src->IsAduClient())
						cur_src->socket->SetDownloadLimit(thePrefs.m_AduValRipBanda);

					cur_src->CheckDownloadTimeout();
					cur_datarate = cur_src->CalculateDownloadRate();
					
					//tigerjact
					datarate+=cur_datarate;
					if(!cur_src->IsAduClient())
					{
					     datarateext+=cur_datarate;
					}
					else
					{
						dataratekadu+=cur_datarate;
					}
					
					//tigerjact
					uint32 limit = reducedownload*cur_datarate/1000;
					uint32 limitext = reducedownloadext*cur_datarate/1000;
					uint32 limitkadu = reducedownloadkadu*cur_datarate/1000;
					
					//imposto il vincolo di banda totale al socket
					if(reducedownload)
					{   limit = CalculateSocketLimit(reducedownload,limit,cur_datarate);
					    if((!reducedownloadkadu)&&(!reducedownloadext))
						{	
							cur_src->socket->SetDownloadLimit(limit);
							if (cur_src->IsDownloadingFromPeerCache() && cur_src->m_pPCDownSocket && cur_src->m_pPCDownSocket->IsConnected())
								cur_src->m_pPCDownSocket->SetDownloadLimit(limit);
						}
					}
					
					//imposto il limite di banda kadu al socket kadu
					if((reducedownloadkadu)&&(cur_src->IsAduClient()))
					{  
						limitkadu = CalculateSocketLimit(reducedownloadkadu,limitkadu,cur_datarate);

						if ((limitkadu > limit)&&(reducedownload)) limitkadu=limit;

						cur_src->socket->SetDownloadLimit(limitkadu);
						if (cur_src->IsDownloadingFromPeerCache() && cur_src->m_pPCDownSocket && cur_src->m_pPCDownSocket->IsConnected())
							cur_src->m_pPCDownSocket->SetDownloadLimit(limitkadu);
					}
				
					//imposto il limite di banda ext al socket ext

					if((reducedownloadext)&&(!cur_src->IsAduClient()))
					{  
						limitext = CalculateSocketLimit(reducedownloadext,limitext,cur_datarate);

						if ((limitext > limit)&&(reducedownload)) limitext=limit;

						cur_src->socket->SetDownloadLimit(limitext);
						if (cur_src->IsDownloadingFromPeerCache() && cur_src->m_pPCDownSocket && cur_src->m_pPCDownSocket->IsConnected())
							cur_src->m_pPCDownSocket->SetDownloadLimit(limitext);
					}

				}
			}
		}
	}
	else
	{
		bool downloadingbefore=m_anStates[DS_DOWNLOADING]>0;
		// -khaos--+++> Moved this here, otherwise we were setting our permanent variables to 0 every tenth of a second...
		memset(m_anStates,0,sizeof(m_anStates));
		memset(src_stats,0,sizeof(src_stats));
		memset(net_stats,0,sizeof(net_stats));
		UINT nCountForState;

		uint16 m_ShowDroppedSrc_Temp = 0; //Anis - Stulle -> SHOW OF DROPPED SOURCES

		for (POSITION pos = srclist.GetHeadPosition(); pos != NULL;)
		{
			CUpDownClient* cur_src = srclist.GetNext(pos);
			// Mod Adu
			// lupz
			// inserisco la fonte nella lista fonti adunanza
			if (cur_src->IsAduClient()) 
			{
				POSITION adupos=adusrclist.Find(cur_src);
				if (!adupos)
					adusrclist.AddTail(cur_src);
			}
			// Fine mod Adu

			if (thePrefs.m_iDbgHeap >= 2)
				ASSERT_VALID( cur_src );

			// BEGIN -rewritten- refreshing statistics (no need for temp vars since it is not multithreaded)
			nCountForState = cur_src->GetDownloadState();
			//special case which is not yet set as downloadstate
			if (nCountForState == DS_ONQUEUE)
			{
				if( cur_src->IsRemoteQueueFull() )
					nCountForState = DS_REMOTEQUEUEFULL;
			}

			// this is a performance killer -> avoid calling 'IsBanned' for gathering stats
			//if (cur_src->IsBanned())
			//	nCountForState = DS_BANNED;
			if (cur_src->GetUploadState() == US_BANNED) // not as accurate as 'IsBanned', but way faster and good enough for stats.
				nCountForState = DS_BANNED;

			if (cur_src->GetSourceFrom() >= SF_SERVER && cur_src->GetSourceFrom() <= SF_PASSIVE)
				++src_stats[cur_src->GetSourceFrom()];

			if (cur_src->GetServerIP() && cur_src->GetServerPort())
			{
				net_stats[0]++;
				if(cur_src->GetKadPort())
					net_stats[2]++;
			}
			if (cur_src->GetKadPort())
				net_stats[1]++;

			ASSERT( nCountForState < sizeof(m_anStates)/sizeof(m_anStates[0]) );
			m_anStates[nCountForState]++;

			switch (cur_src->GetDownloadState())
			{
				case DS_DOWNLOADING:
				{
					ASSERT( cur_src->socket );
					if (cur_src->socket)
					{
						cur_src->CheckDownloadTimeout();
						uint32 cur_datarate = cur_src->CalculateDownloadRate();

						//tigerjact
						datarate+=cur_datarate;
						if(!cur_src->IsAduClient())
						{
							datarateext+=cur_datarate;
						}
						else
						{
							dataratekadu+=cur_datarate;
						}
					
						//tigerjact
						uint32 limit = reducedownload*cur_datarate/1000;
						uint32 limitext = reducedownloadext*cur_datarate/1000;
						uint32 limitkadu = reducedownloadkadu*cur_datarate/1000;
						bool islimit = true;
					
						//imposto il vincolo di banda totale al socket
						if((reducedownload)&& (cur_src->GetDownloadState() == DS_DOWNLOADING))
						{  
							islimit=false;
							limit = CalculateSocketLimit(reducedownload,limit,cur_datarate);

							if((!reducedownloadkadu)&&(!reducedownloadext))
							{
								cur_src->socket->SetDownloadLimit(limit);
								if (cur_src->IsDownloadingFromPeerCache() && cur_src->m_pPCDownSocket && cur_src->m_pPCDownSocket->IsConnected())
									cur_src->m_pPCDownSocket->SetDownloadLimit(limit);
							}
						}
						else
						{
							if((!reducedownloadkadu)&&(!reducedownloadext))
							{
								cur_src->socket->DisableDownloadLimit();
								if (cur_src->IsDownloadingFromPeerCache() && cur_src->m_pPCDownSocket && cur_src->m_pPCDownSocket->IsConnected())
									cur_src->m_pPCDownSocket->DisableDownloadLimit();
					
							}
						}

						//imposto il limite di banda kadu al socket kadu
						if (((reducedownloadkadu) &&(cur_src->IsAduClient())) && (cur_src->GetDownloadState() == DS_DOWNLOADING))
						{
							limitkadu = CalculateSocketLimit(reducedownloadkadu,limitkadu,cur_datarate);

							if ((limitkadu > limit)&&(islimit)) limitkadu=limit;

							cur_src->socket->SetDownloadLimit(limitkadu);
							if (cur_src->IsDownloadingFromPeerCache() && cur_src->m_pPCDownSocket && cur_src->m_pPCDownSocket->IsConnected())
								cur_src->m_pPCDownSocket->SetDownloadLimit(limitkadu);

						}
						else
						{
							if((cur_src->IsAduClient())&&(islimit))
							{
								cur_src->socket->DisableDownloadLimit();
								if (cur_src->IsDownloadingFromPeerCache() && cur_src->m_pPCDownSocket && cur_src->m_pPCDownSocket->IsConnected())
									cur_src->m_pPCDownSocket->DisableDownloadLimit();
							}
						}

						//imposto il limite di banda ext al socket ext

						if (((reducedownloadext) &&(!cur_src->IsAduClient())) && (cur_src->GetDownloadState() == DS_DOWNLOADING))
						{
							limitext = CalculateSocketLimit(reducedownloadext,limitext,cur_datarate);
		
							if ((limitext > limit)&&(islimit)) 
								limitext=limit;

							cur_src->socket->SetDownloadLimit(limitext);

							if (cur_src->IsDownloadingFromPeerCache() && cur_src->m_pPCDownSocket && cur_src->m_pPCDownSocket->IsConnected())
								cur_src->m_pPCDownSocket->SetDownloadLimit(limitext);

						}
						else
						{
							if((!cur_src->IsAduClient())&&(islimit))
							{
								cur_src->socket->DisableDownloadLimit();
								if (cur_src->IsDownloadingFromPeerCache() && cur_src->m_pPCDownSocket && cur_src->m_pPCDownSocket->IsConnected())
									cur_src->m_pPCDownSocket->DisableDownloadLimit();
							}
						}
					}
					break;
				}
				// Do nothing with this client..
				case DS_BANNED:
					break;
				// Check if something has changed with our or their ID state..
				case DS_LOWTOLOWIP:
				{
					// To Mods, please stop instantly removing these sources..
					// This causes sources to pop in and out creating extra overhead!
					//Make sure this source is still a LowID Client..
					if( cur_src->HasLowID() )
					{
						//Make sure we still cannot callback to this Client..
						if( !theApp.CanDoCallback( cur_src ) )
						{
							//If we are almost maxed on sources, slowly remove these client to see if we can find a better source.
							if( ((dwCurTick - lastpurgetime) > SEC2MS(30)) && (this->GetSourceCount() >= (GetMaxSources()*.8 )) )
							{
								theApp.downloadqueue->RemoveSource( cur_src );
								lastpurgetime = dwCurTick;
							}
							break;
						}
					}
					// This should no longer be a LOWTOLOWIP..
					cur_src->SetDownloadState(DS_ONQUEUE);
					break;
				}
				case DS_NONEEDEDPARTS:
				{ 
					// To Mods, please stop instantly removing these sources..
					// This causes sources to pop in and out creating extra overhead!

					if(thePrefs.GetAutoDropSystem() && theApp.GetReAskTick() < dwCurTick){ // ZZUL-TRA :: AutoDropImmunity
						if(dwCurTick > (m_dwAutoNNSTimer + SEC2MS(45))
							&& GetSourceCount() > ((GetMaxSources() *70)/100))
						{
							if(!cur_src->SwapToAnotherFile( _T("Removed: NoNeededSources"), true, true, true , NULL ))
								theApp.downloadqueue->RemoveSourceAndDontAsk(cur_src, true, 1); // variable BlockTime
							m_dwAutoNNSTimer = dwCurTick;
							m_ShowDroppedSrc_Temp++; // show # of dropped sources - Stulle
							break;
						}
					}

					// doubled reasktime for no needed parts - save connections and traffic
                    if (cur_src->GetTimeUntilReask() > 0)
						break; 

                    cur_src->SwapToAnotherFile(_T("A4AF for NNP file. CPartFile::Process()"), true, false, false, NULL, true, true); // ZZ:DownloadManager
					// Recheck this client to see if still NNP.. Set to DS_NONE so that we force a TCP reask next time..
    				cur_src->SetDownloadState(DS_NONE);
					break;
				}
				case DS_ONQUEUE:
				{
					// To Mods, please stop instantly removing these sources..
					// This causes sources to pop in and out creating extra overhead!
					if( cur_src->IsRemoteQueueFull() ) 
					{
						if(thePrefs.GetAutoDropSystem() && theApp.GetReAskTick() < dwCurTick){ // ZZUL-TRA :: AutoDropImmunity
							if(dwCurTick > (m_dwAutoFQSTimer + SEC2MS(55))
								&& GetSourceCount() > ((GetMaxSources() *75)/100))
							{
								theApp.downloadqueue->RemoveSourceAndDontAsk(cur_src, true, 2); // variable BlockTime
								m_dwAutoFQSTimer = dwCurTick;
							    m_ShowDroppedSrc_Temp++; // show # of dropped sources - Stulle
								break;
					        }
                          }
					}
					else if(cur_src->GetRemoteQueueRank() > 3000){
						if(thePrefs.GetAutoDropSystem() && theApp.GetReAskTick() < dwCurTick){ // ZZUL-TRA :: AutoDropImmunity
							if( dwCurTick > (m_dwAutoHQSTimer + SEC2MS(70))
								&& GetSourceCount() > ((GetMaxSources() *90)/100))
							{
								theApp.downloadqueue->RemoveSourceAndDontAsk(cur_src, true, 3); // variable BlockTime
								m_dwAutoHQSTimer = dwCurTick;
								m_ShowDroppedSrc_Temp++; // show # of dropped sources - Stulle	
				                break;
							}
						}
					}
					//Give up to 1 min for UDP to respond.. If we are within one min of TCP reask, do not try..
					if (theApp.IsConnected() && cur_src->GetTimeUntilReask() < MIN2MS(2) && cur_src->GetTimeUntilReask() > SEC2MS(1) && ::GetTickCount()-cur_src->getLastTriedToConnectTime() > 20*60*1000) // ZZ:DownloadManager (one resk timestamp for each file)
						cur_src->UDPReaskForDownload();

				}
				case DS_CONNECTING:
				case DS_TOOMANYCONNS:
				case DS_TOOMANYCONNSKAD:
				case DS_NONE:
				case DS_WAITCALLBACK:
				case DS_WAITCALLBACKKAD:
				{
					/* Kaiser 10/04/2004 -- 15.19
					Cerchiamo di distinguere tra file request agli aduner ed agli esterni. 
					Gli aduner avranno un file request quando viene superato il tempo ADU_FILEREASKTIME mentre
					gli esterni riceveranno richieste regolarmente, ovvero secondo tempo 20*60*1000 */

					// Stefano Picerno: (Rimosso codice duplicato)
					//                  L'unica differenza fra client adu e client non adu e' 
					//                  il valore di dwMaxInterval
					DWORD dwMaxInterval = 20*60*1000;

					if (cur_src->IsAduClient())
					{
						dwMaxInterval = ADU_FILEREASKTIME;
					}

					if (theApp.IsConnected() && cur_src->GetTimeUntilReask() == 0 && ::GetTickCount()-cur_src->getLastTriedToConnectTime() > dwMaxInterval) // ZZ:DownloadManager (one resk timestamp for each file)
					{
						if(!cur_src->AskForDownload()) // NOTE: This may *delete* the client!!
							break; //I left this break here just as a reminder just in case re rearange things..
					}
					break;
				}
			}
		}
		if (downloadingbefore!=(m_anStates[DS_DOWNLOADING]>0))
			NotifyStatusChange();
 		// Mod Adu
 		// Emanem/lupz
 		// cerchiamo su kadu se non abbiamo sufficenti fonti adunanza
		
		m_ShowDroppedSrc = (uint16)(m_ShowDroppedSrc+ m_ShowDroppedSrc_Temp);

		if( GetMaxSourcePerFileUDP() > GetAduSourceCount()){
			if (theApp.downloadqueue->DoKademliaFileRequest() && (Kademlia::CKademlia::GetTotalFile() < KADEMLIATOTALFILE) && (dwCurTick > m_LastSearchTimeKad) &&  Kademlia::CKademlia::IsConnected() && theApp.IsConnected() && !stopped){ //Once we can handle lowID users in Kad, we remove the second IsConnected
				//Kademlia
				theApp.downloadqueue->SetLastKademliaFileRequest();
				if (!GetKadFileSearchID())
				{
					Kademlia::CSearch* pSearch = Kademlia::CSearchManager::PrepareLookup(Kademlia::CSearch::FILE, true, Kademlia::CUInt128(GetFileHash()));
					if (pSearch)
					{
						if(m_TotalSearchesKad < theApp.rm->kadReaskIncs)
							m_TotalSearchesKad++;
						m_LastSearchTimeKad = dwCurTick + (theApp.rm->kadReaskTime*m_TotalSearchesKad);
						pSearch->SetGUIName(GetFileName());
						SetKadFileSearchID(pSearch->GetSearchID());
					}
					else
						SetKadFileSearchID(0);
				}
			}
		}
		else{
			if(GetKadFileSearchID())
			{
				Kademlia::CSearchManager::StopSearch(GetKadFileSearchID(), true);
			}
		}


		// check if we want new sources from server
		if ( !m_bLocalSrcReqQueued && ((!m_LastSearchTime) || (dwCurTick - m_LastSearchTime) > SERVERREASKTIME) && theApp.serverconnect->IsConnected()
			&& GetMaxSourcePerFileSoft() > GetSourceCount() && !stopped
			&& (!IsLargeFile() || (theApp.serverconnect->GetCurrentServer() != NULL && theApp.serverconnect->GetCurrentServer()->SupportsLargeFilesTCP())))
		{
			m_bLocalSrcReqQueued = true;
			theApp.downloadqueue->SendLocalSrcRequest(this);
		}

		count++;
		if (count == 3)
		{
			count = 0;
			m_sourcesaver.Process(this); //Anis Salvataggio fonti
			UpdateAutoDownPriority();
			UpdateDisplayedInfo();
			UpdateCompletedInfos();
		}
	}

	if ( GetSrcStatisticsValue(DS_DOWNLOADING) != nOldTransSourceCount )
	{
		if (theApp.emuledlg->transferwnd->GetDownloadList()->curTab == 0)
			theApp.emuledlg->transferwnd->GetDownloadList()->ChangeCategory(0); 
		else
			UpdateDisplayedInfo(true);
		if (thePrefs.ShowCatTabInfos())
			theApp.emuledlg->transferwnd->UpdateCatTabTitles();
	}


	takelimit returndata;
	returndata.limit = datarate;
	returndata.limitext = datarateext;
	returndata.limitkad = dataratekadu;
	return returndata;
}
bool CPartFile::CanAddSource(uint32 userid, uint16 port, uint32 serverip, uint16 serverport, UINT* pdebug_lowiddropped, bool Ed2kID)
{
	//The incoming ID could have the userid in the Hybrid format.. 
	uint32 hybridID = 0;
	if( Ed2kID )
	{
		if(IsLowID(userid))
			hybridID = userid;
		else
			hybridID = ntohl(userid);
	}
	else
	{
		hybridID = userid;
		if(!IsLowID(userid))
			userid = ntohl(userid);
	}

	if(theApp.clientlist->DontAskThisIP(userid))
		return false;

	// MOD Note: Do not change this part - Merkur
	if (theApp.serverconnect->IsConnected())
	{
		if(theApp.serverconnect->IsLowID())
		{
			if(theApp.serverconnect->GetClientID() == userid && theApp.serverconnect->GetCurrentServer()->GetIP() == serverip && theApp.serverconnect->GetCurrentServer()->GetPort() == serverport )
				return false;
			if(theApp.serverconnect->GetLocalIP() == userid)
				return false;
		}
		else
		{
			if(theApp.serverconnect->GetClientID() == userid && thePrefs.GetPort() == port)
				return false;
		}
	}
	if (Kademlia::CKademlia::IsConnected())
	{
		if(!Kademlia::CKademlia::IsFirewalled())
			if(Kademlia::CKademlia::GetIPAddress() == hybridID && thePrefs.GetPort() == port)
				return false;
	}

	//This allows *.*.*.0 clients to not be removed if Ed2kID == false
	if ( IsLowID(hybridID) && theApp.IsFirewalled())
	{
		if (pdebug_lowiddropped)
			(*pdebug_lowiddropped)++;
		return false;
	}
	// Stefano Picerno: Che senso ha questa modifica?
	//                  Se la fonte che sto cercando di aggiungere è un lowid e NON è fastweb, la ignoro
	if ( IsLowID(hybridID) && !AduIsFastWebIP( userid ) )
	{
		if (pdebug_lowiddropped)
			(*pdebug_lowiddropped)++;
		AddDebugLogLine(false, _T("Ignored source (IP=%s), because it's LOWID and not a fastweb ip"), ipstr(userid));
		return false;
	}
	// Fine modifica
	// MOD Note - end
	return true;
}

void CPartFile::AddSources(CSafeMemFile* sources, uint32 serverip, uint16 serverport, bool bWithObfuscationAndHash)
{
	UINT count = sources->ReadUInt8();

	UINT debug_lowiddropped = 0;
	UINT debug_possiblesources = 0;
	uchar achUserHash[16];
	bool bSkip = false;
	for (UINT i = 0; i < count; i++)
	{
		uint32 userid = sources->ReadUInt32();
		uint16 port = sources->ReadUInt16();
		uint8 byCryptOptions = 0;
		if (bWithObfuscationAndHash){
			byCryptOptions = sources->ReadUInt8();
			if ((byCryptOptions & 0x80) > 0)
				sources->ReadHash16(achUserHash);

			if ((thePrefs.IsClientCryptLayerRequested() && (byCryptOptions & 0x01/*supported*/) > 0 && (byCryptOptions & 0x80) == 0)
				|| (thePrefs.IsClientCryptLayerSupported() && (byCryptOptions & 0x02/*requested*/) > 0 && (byCryptOptions & 0x80) == 0))
				DebugLogWarning(_T("Server didn't provide UserHash for source %u, even if it was expected to (or local obfuscationsettings changed during serverconnect"), userid);
			else if (!thePrefs.IsClientCryptLayerRequested() && (byCryptOptions & 0x02/*requested*/) == 0 && (byCryptOptions & 0x80) != 0)
				DebugLogWarning(_T("Server provided UserHash for source %u, even if it wasn't expected to (or local obfuscationsettings changed during serverconnect"), userid);
		}
		
		// since we may received multiple search source UDP results we have to "consume" all data of that packet
		if (stopped || bSkip)
			continue;

		// check the HighID(IP) - "Filter LAN IPs" and "IPfilter" the received sources IP addresses
		if (!IsLowID(userid))
		{
			if (!IsGoodIP(userid))
			{ 
				// check for 0-IP, localhost and optionally for LAN addresses
				//if (thePrefs.GetLogFilteredIPs())
				//	AddDebugLogLine(false, _T("Ignored source (IP=%s) received from server - bad IP"), ipstr(userid));
				continue;
			}
			if (theApp.ipfilter->IsFiltered(userid))
			{
				if (thePrefs.GetLogFilteredIPs())
					AddDebugLogLine(false, _T("Ignored source (IP=%s) received from server - IP filter (%s)"), ipstr(userid), theApp.ipfilter->GetLastHit());
				continue;
			}
			if (theApp.clientlist->IsBannedClient(userid)){
#ifdef _DEBUG
				if (thePrefs.GetLogBannedClients()){
					CUpDownClient* pClient = theApp.clientlist->FindClientByIP(userid);
					AddDebugLogLine(false, _T("Ignored source (IP=%s) received from server - banned client %s"), ipstr(userid), pClient->DbgGetClientInfo());
				}
#endif
				continue;
			}
		}

		// additionally check for LowID and own IP
		if (!CanAddSource(userid, port, serverip, serverport, &debug_lowiddropped))
		{
			//if (thePrefs.GetLogFilteredIPs())
			//	AddDebugLogLine(false, _T("Ignored source (IP=%s) received from server"), ipstr(userid));
			continue;
		}

		if( GetMaxSources() > this->GetSourceCount() )
		{
			debug_possiblesources++;
			CUpDownClient* newsource = new CUpDownClient(this,port,userid,serverip,serverport,true);
			newsource->SetConnectOptions(byCryptOptions, true, false);

			if ((byCryptOptions & 0x80) != 0)
				newsource->SetUserHash(achUserHash);
			theApp.downloadqueue->CheckAndAddSource(this,newsource);
		}
		else
		{
			// since we may received multiple search source UDP results we have to "consume" all data of that packet
			bSkip = true;
			if(GetKadFileSearchID())
				Kademlia::CSearchManager::StopSearch(GetKadFileSearchID(), false);
			continue;
		}
	}
	if (thePrefs.GetDebugSourceExchange())
		AddDebugLogLine(false, _T("SXRecv: Server source response; Count=%u, Dropped=%u, PossibleSources=%u, File=\"%s\""), count, debug_lowiddropped, debug_possiblesources, GetFileName());
}

void CPartFile::AddSource(LPCTSTR pszURL, uint32 nIP)
{
	if (stopped)
		return;

	if (!IsGoodIP(nIP))
	{ 
		// check for 0-IP, localhost and optionally for LAN addresses
		//if (thePrefs.GetLogFilteredIPs())
		//	AddDebugLogLine(false, _T("Ignored URL source (IP=%s) \"%s\" - bad IP"), ipstr(nIP), pszURL);
		return;
	}
	if (theApp.ipfilter->IsFiltered(nIP))
	{
		if (thePrefs.GetLogFilteredIPs())
			AddDebugLogLine(false, _T("Ignored URL source (IP=%s) \"%s\" - IP filter (%s)"), ipstr(nIP), pszURL, theApp.ipfilter->GetLastHit());
		return;
	}

	CUrlClient* client = new CUrlClient;
	if (!client->SetUrl(pszURL, nIP))
	{
		LogError(LOG_STATUSBAR, _T("Failed to process URL source \"%s\""), pszURL);
		delete client;
		return;
	}
	client->SetRequestFile(this);
	client->SetSourceFrom(SF_LINK);
	if (theApp.downloadqueue->CheckAndAddSource(this, client))
		UpdatePartsInfo();
}

// Anis: heapsortCompletesrc
static void HeapSort(CArray<uint16, uint16>& count, UINT first, UINT last){
	UINT r;
	for ( r = first; !(r & (UINT)INT_MIN) && (r<<1) < last; ){
		UINT r2 = (r<<1)+1;
		if (r2 != last)
			if (count[r2] < count[r2+1])
				r2++;
		if (count[r] < count[r2]){
			uint16 t = count[r2];
			count[r2] = count[r];
			count[r] = t;
			r = r2;
		}
		else
			break;
	}
}
// Anis: heapsortCompletesrc

void CPartFile::UpdatePartsInfo()
{
	if( !IsPartFile() )
	{
		CKnownFile::UpdatePartsInfo();
		return;
	}

	// Cache part count
	UINT partcount = GetPartCount();
	bool flag = (time(NULL) - m_nCompleteSourcesTime > 0); 

	// Reset part counters
	if ((UINT)m_SrcpartFrequency.GetSize() < partcount)
		m_SrcpartFrequency.SetSize(partcount);
	//>>> Anis::ICS [enkeyDEV]
	if ((uint16)m_SrcIncPartFrequency.GetSize() < partcount)
		m_SrcIncPartFrequency.SetSize(partcount);
	//### Anis::ICS [enkeyDEV]

	for (UINT i = 0; i < partcount; i++) {
		m_SrcpartFrequency[i] = 0;
		m_SrcIncPartFrequency[i] = 0; //>>> Anis::ICS [enkeyDEV]
	}

	CArray<uint16, uint16> count;
	if (flag)
		count.SetSize(0, srclist.GetSize());
	for (POSITION pos = srclist.GetHeadPosition(); pos != 0; )
	{
		CUpDownClient* cur_src = srclist.GetNext(pos);
		// Mod Adu
		// lupz
		// Anis revision
		// SECONDO STIMATORE
		// consideriamo solo le fonti adunanza per effettuare la stima
		if (!cur_src->IsAduClient())
			continue;
		// Fine mod Adu
		if( cur_src->GetPartStatus() )
		{		
#define DEFAULT_WEIGHT	0x02	//official was 1
//a seen part gets lower priority because we *know* it exists but can't request it
#define SEEN_WEIGHT		0x01
//### Anis::AntiHideOS/Anis::ICS
//>>> Anis::AntiHideOS [netfinity]
			if(!cur_src->m_abySeenPartStatus)
			{
				cur_src->m_abySeenPartStatus = new uint8[partcount];
				memset(cur_src->m_abySeenPartStatus, 0, partcount);				
			}
//### Anis::AntiHideOS [netfinity]
			for (uint16 i = 0; i < partcount; i++)
			{
				if (cur_src->IsPartAvailable(i))
				{
					m_SrcpartFrequency[i] += DEFAULT_WEIGHT;
					cur_src->m_abySeenPartStatus[i] = 1; //>>> Anis::AntiHideOS [netfinity]
				}
//>>> Anis::AntiHideOS [netfinity]
				else if((cur_src->m_abySeenPartStatus && cur_src->m_abySeenPartStatus[i] == 1)) 
					m_SrcpartFrequency[i] += SEEN_WEIGHT;
//### Anis::AntiHideOS [netfinity]
//>>> Anis::ICS [enkeyDEV]
				else if (cur_src->IsIncPartAvailable(i))
					m_SrcIncPartFrequency[i] += 1;
//### Anis::ICS [enkeyDEV]
			}
			if ( flag )
			{
				count.Add(cur_src->GetUpCompleteSourcesCount());
			}
		}
	}

	if (flag)
	{
//>>> taz::Optimization
		m_nCompleteSourcesCount = m_SrcpartFrequency[0];

		for (UINT i = 1; i < partcount; i++)
		{
			if( m_nCompleteSourcesCount > m_SrcpartFrequency[i])
				m_nCompleteSourcesCount = m_SrcpartFrequency[i];
		}
//### taz::Optimization
	
		count.Add(m_nCompleteSourcesCount);
	
		int n = count.GetSize();
//>>> Anis::Optimization
// moved from below
		//Not many sources, so just use what you see..
		if (n < 5)
		{
			m_nCompleteSourcesCountLo= m_nCompleteSourcesCount;
			m_nCompleteSourcesCountHi= m_nCompleteSourcesCount;
		}
		else
//		if (n > 0) // Always trus
//### Anis::Optimization
		{
			// Anis: heapsortCompletesrc
			int r;
			for (r = n/2; r--; )
				HeapSort(count, r, n-1);
			for (r = n; --r; ){
				uint16 t = count[r];
				count[r] = count[0];
				count[0] = t;
				HeapSort(count, 0, r-1);
			}
			// Anis: heapsortCompletesrc

			// calculate range
			int i = n >> 1;			// (n / 2)
			int j = (n * 3) >> 2;	// (n * 3) / 4
			int k = (n * 7) >> 3;	// (n * 7) / 8

			//When still a part file, adjust your guesses by 20% to what you see..

//>>> Anis::Optimization
/* moved up
			//Not many sources, so just use what you see..
			if (n < 5)
			{
//				m_nCompleteSourcesCount;
				m_nCompleteSourcesCountLo= m_nCompleteSourcesCount;
				m_nCompleteSourcesCountHi= m_nCompleteSourcesCount;
			}
			//For low guess and normal guess count
			//	If we see more sources then the guessed low and normal, use what we see.
			//	If we see less sources then the guessed low, adjust network accounts for 80%, we account for 20% with what we see and make sure we are still above the normal.
			//For high guess
			//  Adjust 80% network and 20% what we see.
			else if (n < 20)
*/
			if (n < 20)
//### Anis::Optimization
			{
				if ( count.GetAt(i) < m_nCompleteSourcesCount )
					m_nCompleteSourcesCountLo = m_nCompleteSourcesCount;
				else
					m_nCompleteSourcesCountLo = (uint16)((float)(count.GetAt(i)*.8)+(float)(m_nCompleteSourcesCount*.2));
				m_nCompleteSourcesCount= m_nCompleteSourcesCountLo;
				m_nCompleteSourcesCountHi= (uint16)((float)(count.GetAt(j)*.8)+(float)(m_nCompleteSourcesCount*.2));
				if( m_nCompleteSourcesCountHi < m_nCompleteSourcesCount )
					m_nCompleteSourcesCountHi = m_nCompleteSourcesCount;
			}
			else
			//Many sources..
			//For low guess
			//	Use what we see.
			//For normal guess
			//	Adjust network accounts for 80%, we account for 20% with what we see and make sure we are still above the low.
			//For high guess
			//  Adjust network accounts for 80%, we account for 20% with what we see and make sure we are still above the normal.
			{
				m_nCompleteSourcesCountLo= m_nCompleteSourcesCount;
				m_nCompleteSourcesCount= (uint16)((float)(count.GetAt(j)*.8)+(float)(m_nCompleteSourcesCount*.2));
				if( m_nCompleteSourcesCount < m_nCompleteSourcesCountLo )
					m_nCompleteSourcesCount = m_nCompleteSourcesCountLo;
				m_nCompleteSourcesCountHi= (uint16)((float)(count.GetAt(k)*.8)+(float)(m_nCompleteSourcesCount*.2));
				if( m_nCompleteSourcesCountHi < m_nCompleteSourcesCount )
					m_nCompleteSourcesCountHi = m_nCompleteSourcesCount;
			}
		}
		m_nCompleteSourcesTime = time(NULL) + (60);
	}
	NewSrcIncPartsInfo(); //>>> Anis::ICS [enkeyDEV]
	UpdateDisplayedInfo();
	InChangedSharedStatusBar = false;
}		

bool CPartFile::RemoveBlockFromList(uint64 start, uint64 end)
{
	ASSERT( start <= end );

	bool bResult = false;
	for (POSITION pos = requestedblocks_list.GetHeadPosition(); pos != NULL; ){
		POSITION posLast = pos;
		Requested_Block_Struct* block = requestedblocks_list.GetNext(pos);
		if (block->StartOffset <= start && block->EndOffset >= end){
			requestedblocks_list.RemoveAt(posLast);
			bResult = true;
		}
	}
	return bResult;
}

//>>> Anis::ICS [enkeyDEV]
void CPartFile::WriteIncPartStatus(CSafeMemFile* file)
{
	const uint16 parts = GetED2KPartCount();       
	const uint16 partcount = GetPartCount();
	file->WriteUInt16(parts);

	uint16 done = 0;
	while (done != parts)
	{
		uint8 towrite = 0;
		for (uint8 i = 0; i != 8; ++i)
		{
			if(done < partcount && !IsPureGap(done*PARTSIZE, (done + 1)*PARTSIZE - 1))
				towrite |= (1<<i);
			++done;
			if (done == parts)
				break;
		}
		file->WriteUInt8(towrite);
	}
}

void CPartFile::NewSrcIncPartsInfo()
{
	const uint16 partcount = GetPartCount();

	if (m_SrcIncPartFrequency.GetSize() < partcount)
		m_SrcIncPartFrequency.SetSize(partcount);

	for(uint16 i = 0; i < partcount; ++i)
		m_SrcIncPartFrequency[i] = 0;

	for (POSITION pos = srclist.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* cur_src = srclist.GetNext(pos);
		for (uint16 i = 0; i < partcount; ++i)
			if (cur_src->IsIncPartAvailable(i))
				++m_SrcIncPartFrequency[i];
	}
	//UpdateDisplayedInfo(); // Not displayed
}

uint16* CPartFile::CalcDownloadingParts(const CUpDownClient* client){	//<<-- Pawcio for enkeyDEV -ICS-
	if (!client)
		return NULL;

	uint16  partsCount = GetPartCount();
	if (!partsCount)
		return NULL;

	uint16* partsDownloading = new uint16[partsCount];
	memset(partsDownloading, 0, partsCount * sizeof(uint16));

	CUpDownClient* cur_client;
	POSITION pos = m_downloadingSourceList.GetHeadPosition();
	while (pos){
		cur_client = m_downloadingSourceList.GetNext(pos);
		uint16 clientPart = cur_client->m_lastPartAsked;
		if (cur_client != client && cur_client->GetRequestFile() && !md4cmp(cur_client->GetRequestFile()->GetFileHash(), GetFileHash()) && clientPart < partsCount && cur_client->GetDownloadDatarate() > 150)
			partsDownloading[clientPart]++;
	}
	return partsDownloading;
}

//returns the number of bytes that are left to be downloaded
uint64 CPartFile::GetPartSizeToDownload(const uint16 partNumber) const
{
	return GetTotalGapSizeInPart(partNumber);
}

//Anis MOD -> AdunanzA
bool CPartFile::GetNextRequestedBlock(CUpDownClient* sender, Requested_Block_Struct** newblocks, uint16* count) //Anis -> Aggiunto il dinamic block request.
{
	//ANIS ////////////////////////////////MOD ADU -> STREAMING CORE////////////////////////////////////////////////////////////////////////
	if (!(*count))
		return false;
	if(sender->GetPartStatus() == NULL)
		return false;
	
	extern bool AduStream;
	//>>> NetFinity::DynamicBlockRequests
	uint64	bytesPerRequest = EMBLOCKSIZE;
	uint64	bytesLeftToDownload = GetFileSize() - GetCompletedSize();
	UINT	fileDatarate = max(GetDatarate(), 3072); // Always assume file is being downloaded at atleast 3 kB/s
	UINT	sourceDatarate = max(sender->GetDownloadDatarate(), 10); // Always assume client is uploading at atleast 10 B/s
	UINT	timeToFileCompletion = max((UINT) (bytesLeftToDownload / (uint64) fileDatarate) + 1, 10); // Always assume it will take atleast 10 seconds to complete
	
	bytesPerRequest = (sourceDatarate * timeToFileCompletion) / 2;

	if (bytesPerRequest > EMBLOCKSIZE)
		bytesPerRequest = EMBLOCKSIZE;
	if (bytesPerRequest < 10240)
	{
		// Let an other client request this packet if we are close to completion and source is slow
		// Use the true file datarate here, otherwise we might get stuck in NNP state
		if (!requestedblocks_list.IsEmpty() && timeToFileCompletion < 30 && bytesPerRequest < 3400 && 5 * sourceDatarate < GetDatarate())
		{
			DebugLogWarning(_T("DBG: (%s) No request block given as source is slow and file near completion!"), sender->DbgGetClientInfo());
			return false;
		}
		bytesPerRequest = 10240;
	}
	//### NetFinity::DynamicBlockRequests

	// Select mode: RELEASE, SPREAD or SHARE
	uint16	min_src = _UI16_MAX;
	if (m_SrcpartFrequency.GetCount() < GetPartCount())
		min_src = 0;
	else
	{
		for (uint16 part_idx = 0; part_idx < GetPartCount(); ++part_idx)
			if (m_SrcpartFrequency[part_idx] < min_src && sender->IsPartAvailable(part_idx)) //>>> taz::Fix chunks that can not be downloaded from sender shouldn't affect selection mode
				min_src = m_SrcpartFrequency[part_idx];
	}

	uint8 m_ics_filemode = CM_SHARE_MODE;
	if (min_src <= CM_SPREAD_MINSRC)		
		m_ics_filemode = CM_RELEASE_MODE;
	else if (min_src <= CM_SHARE_MINSRC)	
		m_ics_filemode = CM_SPREAD_MODE;

	// Chunk list ordered by preference
	CList<uint16> chunk_list;
	CList<uint64> chunk_pref;
	uint16* partsDownloading = CalcDownloadingParts(sender); 

	bool isPreviewEnable = (thePrefs.GetPreviewPrio() || thePrefs.IsExtControlsEnabled() && GetPreviewPrio()) && IsPreviewableFileType(); //>>> taz::Optimization

	for (uint16 part_idx = 0; part_idx < GetPartCount(); ++part_idx)
	{
		if(sender->IsPartAvailable(part_idx) && GetNextEmptyBlockInPart(part_idx, 0, bytesPerRequest))
		{
			UINT complete_src = 0;
			UINT incomplete_src = 0;
			UINT first_last_mod = 2; //>>> taz::Fix was 0
			uint64 c_pref = 0;
			uint64 size2transfer = 0;
			
			if(!AduStream) {
			// Chunk priority modifiers
				complete_src = m_SrcpartFrequency.GetCount() > part_idx ? m_SrcpartFrequency[part_idx] : 0;
				incomplete_src = m_SrcIncPartFrequency.GetCount() > part_idx ? m_SrcIncPartFrequency[part_idx] : 0;
			}

			else 
			{
				if (m_ics_filemode != CM_SHARE_MODE)
				{
					complete_src = m_SrcpartFrequency.GetCount() > part_idx ? m_SrcpartFrequency[part_idx] : 0;
					incomplete_src = m_SrcIncPartFrequency.GetCount() > part_idx ? m_SrcIncPartFrequency[part_idx] : 0;
				}
			}
			
			if(!AduStream) 
			{

				if (isPreviewEnable) //>>> taz::Optimization
				{
					if (part_idx == 0 || part_idx == (GetPartCount() - 1))		
						first_last_mod = 0; 

					else if (part_idx == 1 || part_idx == (GetPartCount() - 2))	
						first_last_mod = 1;
				}

			}

			else 
			{

				if (m_ics_filemode != CM_RELEASE_MODE && isPreviewEnable) //>>> taz::Optimization
				{
					if (part_idx == 0 || part_idx == (GetPartCount() - 1))		
						first_last_mod = 0; 

					else if (part_idx == 1 || part_idx == (GetPartCount() - 2))	
						first_last_mod = 1;
				}

			}

			size2transfer = GetPartSizeToDownload(part_idx);
			size2transfer = min(((size2transfer + (partsDownloading ? PARTSIZE * partsDownloading[part_idx] / CM_MAX_SRC_CHUNK : 0) + 0xff) >> 8), 0xFFFF);

			switch (m_ics_filemode)
			{
				case CM_RELEASE_MODE:
					complete_src = min(complete_src, 0xFF);
					incomplete_src = min(incomplete_src, 0xFF);
					c_pref = size2transfer | (incomplete_src << 16) | (complete_src << 24);
					break;
				case CM_SPREAD_MODE:
					complete_src = min(complete_src, 0xFF);
					incomplete_src = min(incomplete_src, 0x3F);
					c_pref = first_last_mod | (incomplete_src << 2) | (complete_src << 8) | (size2transfer << 16);
					break;
				case CM_SHARE_MODE:
					c_pref = first_last_mod | (size2transfer << 16);
					break;
			}

			if (partsDownloading && partsDownloading[part_idx] >= ceil((float)size2transfer * CM_MAX_SRC_CHUNK / (float)PARTSIZE)) 
				c_pref |= 0xFF000000;

			// Ordered insertion
			POSITION c_ins_point = chunk_list.GetHeadPosition();
			POSITION p_ins_point = chunk_pref.GetHeadPosition();

		if(!AduStream) {

			while (c_ins_point)
				chunk_list.GetNext(c_ins_point);

			if (c_ins_point)
			{
				int eq_count = 0;
				POSITION p_eq_point = c_ins_point;

					while (p_eq_point != 0 )
						++eq_count;
				

				if (eq_count) // insert in random position
				{
					uint16 randomness = (uint16)floor(((float)rand()/RAND_MAX)*eq_count);
					while (randomness)
					{
						chunk_list.GetNext(c_ins_point);
						--randomness;
					}
				}

			} // END if c_ins_point
			
			if (c_ins_point) // null ptr would add to head, I need to add to tail
				chunk_list.InsertBefore(c_ins_point, part_idx);
			else
				chunk_list.AddTail(part_idx);
		}
		else {
			while (c_ins_point && p_ins_point && chunk_pref.GetAt(p_ins_point) < c_pref)
			{
				chunk_list.GetNext(c_ins_point);
				chunk_pref.GetNext(p_ins_point);
			}

			if (c_ins_point)
			{
				int eq_count = 0;
				POSITION p_eq_point = p_ins_point;
				while (p_eq_point != 0 && chunk_pref.GetAt(p_eq_point) == c_pref)
				{
					++eq_count;
					chunk_pref.GetNext(p_eq_point);
				}
				if (eq_count) // insert in random position
				{
					uint16 randomness = (uint16)floor(((float)rand()/RAND_MAX)*eq_count);
					while (randomness)
					{
						chunk_list.GetNext(c_ins_point);
						chunk_pref.GetNext(p_ins_point);
						--randomness;
					}
				}

			} // END if c_ins_point

			if (c_ins_point) // null ptr would add to head, I need to add to tail
			{
				chunk_list.InsertBefore(c_ins_point, part_idx);
				chunk_pref.InsertBefore(p_ins_point, c_pref);
			}
			else
			{
				chunk_list.AddTail(part_idx);
				chunk_pref.AddTail(c_pref);
			}
			}
		}
	} 

	delete[] partsDownloading;
	partsDownloading = NULL;
	
	if(!AduStream) {
	if(sender->m_lastPartAsked != _UI16_MAX && sender->IsPartAvailable(sender->m_lastPartAsked) && GetNextEmptyBlockInPart(sender->m_lastPartAsked, 0, bytesPerRequest))
		chunk_list.AddHead(sender->m_lastPartAsked);
	else 
		sender->m_lastPartAsked = _UI16_MAX;
	}

	else {

	if(sender->m_lastPartAsked != _UI16_MAX && sender->IsPartAvailable(sender->m_lastPartAsked) && GetNextEmptyBlockInPart(sender->m_lastPartAsked, 0, bytesPerRequest)) {
		chunk_list.AddHead(sender->m_lastPartAsked);
		chunk_pref.AddHead((uint64) 0);
	} 
	else 
		sender->m_lastPartAsked = _UI16_MAX;

	}

	uint16 requestedCount = *count;
	uint16 newblockcount = 0;
	*count = 0;

	if (chunk_list.IsEmpty()) 
		return false;

	Requested_Block_Struct* block = new Requested_Block_Struct;
	for (POSITION scan_chunks = chunk_list.GetHeadPosition(); scan_chunks; chunk_list.GetNext(scan_chunks))
	{
		sender->m_lastPartAsked = chunk_list.GetAt(scan_chunks);
		while(GetNextEmptyBlockInPart(chunk_list.GetAt(scan_chunks), block, bytesPerRequest))
		{
			requestedblocks_list.AddTail(block);
			newblocks[newblockcount++] = block;
			*count = newblockcount;
			if (newblockcount == requestedCount)
				return true;			
			block = new Requested_Block_Struct;
		}
	}
	delete block;
	block = NULL;

	if (!(*count)) 
		return false; // useless, just to be sure
	return true;
}
//Anis MOD -> AdunanzA

bool CPartFile::IsInRequestedBlockList(const Requested_Block_Struct* block) const
{
	return requestedblocks_list.Find(const_cast<Requested_Block_Struct*>(block)) != NULL;
}

void CPartFile::RemoveAllRequestedBlocks(void)
{
	requestedblocks_list.RemoveAll();
}

void CPartFile::CompleteFile(bool bIsHashingDone)
{
	theApp.downloadqueue->RemoveLocalServerRequest(this);
	if(GetKadFileSearchID())
		Kademlia::CSearchManager::StopSearch(GetKadFileSearchID(), false);

	if (srcarevisible)
		theApp.emuledlg->transferwnd->GetDownloadList()->HideSources(this);
	
	if (!bIsHashingDone){
		SetStatus(PS_COMPLETING);
		datarate = 0;
		CAddFileThread* addfilethread = (CAddFileThread*) AfxBeginThread(RUNTIME_CLASS(CAddFileThread), THREAD_PRIORITY_BELOW_NORMAL,0, CREATE_SUSPENDED);
		if (addfilethread){
			SetFileOp(PFOP_HASHING);
			SetFileOpProgress(0);
			TCHAR mytemppath[MAX_PATH];
			_tcscpy(mytemppath,m_fullname);
			mytemppath[ _tcslen(mytemppath)-_tcslen(m_partmetfilename)-1]=0;
			addfilethread->SetValues(NULL, mytemppath, RemoveFileExtension(m_partmetfilename), _T(""), this);
			addfilethread->ResumeThread();	
		}
		else{
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_FILECOMPLETIONTHREAD));
			SetStatus(PS_ERROR);
		}
		return;
	}
	else{
		StopFile();
		SetStatus(PS_COMPLETING);
		CWinThread *pThread = AfxBeginThread(CompleteThreadProc, this, THREAD_PRIORITY_BELOW_NORMAL, 0, CREATE_SUSPENDED); // Lord KiRon - using threads for file completion
		if (pThread){
			SetFileOp(PFOP_COPYING);
			SetFileOpProgress(0);
			pThread->ResumeThread();
		}
		else{
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_FILECOMPLETIONTHREAD));
			SetStatus(PS_ERROR);
			return;
		}
	}
	theApp.emuledlg->transferwnd->GetDownloadList()->ShowFilesCount();
	if (thePrefs.ShowCatTabInfos())
		theApp.emuledlg->transferwnd->UpdateCatTabTitles();
	UpdateDisplayedInfo(true);
}

UINT CPartFile::CompleteThreadProc(LPVOID pvParams) 
{ 
	DbgSetThreadName("PartFileComplete");
	InitThreadLocale();
	// Anis: SafeHash
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return 0;
	// Anis: SafeHash
	CPartFile* pFile = (CPartFile*)pvParams;
	if (!pFile)
		return (UINT)-1; 
	CoInitialize(NULL);
   	pFile->PerformFileComplete();
	CoUninitialize();
   	return 0; 
}


void UncompressFile(LPCTSTR pszFilePath, CPartFile* pPartFile)
{
	// check, if it's a compressed file
	DWORD dwAttr = GetFileAttributes(pszFilePath);
	if (dwAttr == INVALID_FILE_ATTRIBUTES || (dwAttr & FILE_ATTRIBUTE_COMPRESSED) == 0)
		return;

	CString strDir = pszFilePath;
	PathRemoveFileSpec(strDir.GetBuffer());
	strDir.ReleaseBuffer();

	// If the directory of the file has the 'Compress' attribute, do not uncomress the file
	dwAttr = GetFileAttributes(strDir);
	if (dwAttr == INVALID_FILE_ATTRIBUTES || (dwAttr & FILE_ATTRIBUTE_COMPRESSED) != 0)
		return;

	HANDLE hFile = CreateFile(pszFilePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE){
		if (thePrefs.GetVerbose())
			theApp.QueueDebugLogLine(true, _T("Failed to open file \"%s\" for decompressing - %s"), pszFilePath, GetErrorMessage(GetLastError(), 1));
		return;
	}
	
	if (pPartFile)
		pPartFile->SetFileOp(PFOP_UNCOMPRESSING);

	USHORT usInData = COMPRESSION_FORMAT_NONE;
	DWORD dwReturned = 0;
	if (!DeviceIoControl(hFile, FSCTL_SET_COMPRESSION, &usInData, sizeof usInData, NULL, 0, &dwReturned, NULL)){
		if (thePrefs.GetVerbose())
			theApp.QueueDebugLogLine(true, _T("Failed to decompress file \"%s\" - %s"), pszFilePath, GetErrorMessage(GetLastError(), 1));
	}
	CloseHandle(hFile);
}

#ifndef __IZoneIdentifier_INTERFACE_DEFINED__
MIDL_INTERFACE("cd45f185-1b21-48e2-967b-ead743a8914e")
IZoneIdentifier : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetId(DWORD *pdwZone) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetId(DWORD dwZone) = 0;
    virtual HRESULT STDMETHODCALLTYPE Remove(void) = 0;
};
#endif //__IZoneIdentifier_INTERFACE_DEFINED__

#ifdef CLSID_PersistentZoneIdentifier
EXTERN_C const IID CLSID_PersistentZoneIdentifier;
#else
const GUID CLSID_PersistentZoneIdentifier = { 0x0968E258, 0x16C7, 0x4DBA, { 0xAA, 0x86, 0x46, 0x2D, 0xD6, 0x1E, 0x31, 0xA3 } };
#endif

void SetZoneIdentifier(LPCTSTR pszFilePath)
{
	if (!thePrefs.GetCheckFileOpen())
		return;
	CComPtr<IZoneIdentifier> pZoneIdentifier;
	HRESULT hr = pZoneIdentifier.CoCreateInstance(CLSID_PersistentZoneIdentifier, NULL, CLSCTX_INPROC_SERVER);
	if (SUCCEEDED(hr))
	{
		CComQIPtr<IPersistFile> pPersistFile = pZoneIdentifier;
		if (pPersistFile)
		{
			// Specify the 'zone identifier' which has to be commited with 'IPersistFile::Save'
			hr = pZoneIdentifier->SetId(URLZONE_INTERNET);
			if (SUCCEEDED(hr))
			{
				// Save the 'zone identifier'
				// NOTE: This does not modify the file content in any way, 
				// *but* it modifies the "Last Modified" file time!
				VERIFY( SUCCEEDED(hr = pPersistFile->Save(pszFilePath, FALSE)) );
			}
		}
	}
}


DWORD CALLBACK CopyProgressRoutine(LARGE_INTEGER TotalFileSize, LARGE_INTEGER TotalBytesTransferred,
								   LARGE_INTEGER /*StreamSize*/, LARGE_INTEGER /*StreamBytesTransferred*/, DWORD /*dwStreamNumber*/,
								   DWORD /*dwCallbackReason*/, HANDLE /*hSourceFile*/, HANDLE /*hDestinationFile*/, 
								   LPVOID lpData)
{
	CPartFile* pPartFile = (CPartFile*)lpData;
	if (TotalFileSize.QuadPart && pPartFile && pPartFile->IsKindOf(RUNTIME_CLASS(CPartFile)))
	{
		UINT uProgress = (UINT)(TotalBytesTransferred.QuadPart * 100 / TotalFileSize.QuadPart);
		if (uProgress != pPartFile->GetFileOpProgress())
		{
			ASSERT( uProgress <= 100 );
			VERIFY( PostMessage(theApp.emuledlg->GetSafeHwnd(), TM_FILEOPPROGRESS, uProgress, (LPARAM)pPartFile) );
		}
	}
	else
		ASSERT(0);

	return PROGRESS_CONTINUE;
}

DWORD MoveCompletedPartFile(LPCTSTR pszPartFilePath, LPCTSTR pszNewPartFilePath, CPartFile* pPartFile)
{
	DWORD dwMoveResult = ERROR_INVALID_FUNCTION;

	bool bUseDefaultMove = true;
	HMODULE hLib = GetModuleHandle(_T("kernel32"));
	if (hLib)
	{
		BOOL (WINAPI *pfnMoveFileWithProgress)(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, DWORD dwFlags);
		(FARPROC&)pfnMoveFileWithProgress = GetProcAddress(hLib, _TWINAPI("MoveFileWithProgress"));
		if (pfnMoveFileWithProgress)
		{
			bUseDefaultMove = false;
			if ((*pfnMoveFileWithProgress)(pszPartFilePath, pszNewPartFilePath, CopyProgressRoutine, pPartFile, MOVEFILE_COPY_ALLOWED))
				dwMoveResult = ERROR_SUCCESS;
			else
				dwMoveResult = GetLastError();
		}
	}

	if (bUseDefaultMove)
	{
		if (MoveFile(pszPartFilePath, pszNewPartFilePath))
			dwMoveResult = ERROR_SUCCESS;
		else
			dwMoveResult = GetLastError();
	}

	return dwMoveResult;
}

// Lord KiRon - using threads for file completion
// NOTE: This function is executed within a seperate thread, do *NOT* use any lists/queues of the main thread without
// synchronization. Even the access to couple of members of the CPartFile (e.g. filename) would need to be properly
// synchronization to achive full multi threading compliance.
BOOL CPartFile::PerformFileComplete() 
{
	// If that function is invoked from within the file completion thread, it's ok if we wait (and block) the thread.
	CSingleLock sLock(&m_FileCompleteMutex, TRUE);

	CString strPartfilename(RemoveFileExtension(m_fullname));
	TCHAR* newfilename = _tcsdup(GetFileName());
	_tcscpy(newfilename, (LPCTSTR)StripInvalidFilenameChars(newfilename));

	CString strNewname;
	CString indir;

	if (PathFileExists(thePrefs.GetCategory(GetCategory())->strIncomingPath)){
		indir = thePrefs.GetCategory(GetCategory())->strIncomingPath;
		strNewname.Format(_T("%s\\%s"), indir, newfilename);
	}
	else{
		indir = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
		strNewname.Format(_T("%s\\%s"), indir, newfilename);
	}

	// close permanent handle
	try{
		if (m_hpartfile.m_hFile != INVALID_HANDLE_VALUE)
			m_hpartfile.Close();
	}
	catch(CFileException* error){
		TCHAR buffer[MAX_CFEXP_ERRORMSG];
		error->GetErrorMessage(buffer, ARRSIZE(buffer));
		theApp.QueueLogLine(true, GetResString(IDS_ERR_FILEERROR), m_partmetfilename, GetFileName(), buffer);
		error->Delete();
		//return false;
	}

	bool renamed = false;
	if(PathFileExists(strNewname))
	{
		renamed = true;
		int namecount = 0;

		size_t length = _tcslen(newfilename);
		ASSERT(length != 0); //name should never be 0

		//the file extension
		TCHAR *ext = _tcsrchr(newfilename, _T('.'));
		if(ext == NULL)
			ext = newfilename + length;

		TCHAR *last = ext;  //new end is the file name before extension
		last[0] = 0;  //truncate file name

		//search for matching ()s and check if it contains a number
		if((ext != newfilename) && (_tcsrchr(newfilename, _T(')')) + 1 == last)) {
			TCHAR *first = _tcsrchr(newfilename, _T('('));
			if(first != NULL) {
				first++;
				bool found = true;
				for(TCHAR *step = first; step < last - 1; step++)
					if(*step < _T('0') || *step > _T('9')) {
						found = false;
						break;
					}
				if(found) {
					namecount = _tstoi(first);
					last = first - 1;
					last[0] = 0;  //truncate again
				}
			}
		}

		CString strTestName;
		do {
			namecount++;
			strTestName.Format(_T("%s\\%s(%d).%s"), indir, newfilename, namecount, min(ext + 1, newfilename + length));
		}
		while (PathFileExists(strTestName));
		strNewname = strTestName;
	}
	free(newfilename);

	DWORD dwMoveResult;
	if ((dwMoveResult = MoveCompletedPartFile(strPartfilename, strNewname, this)) != ERROR_SUCCESS)
	{
		theApp.QueueLogLine(true,GetResString(IDS_ERR_COMPLETIONFAILED) + _T(" - \"%s\": ") + GetErrorMessage(dwMoveResult), GetFileName(), strNewname);
		// If the destination file path is too long, the default system error message may not be helpful for user to know what failed.
		if (strNewname.GetLength() >= MAX_PATH)
			theApp.QueueLogLine(true,GetResString(IDS_ERR_COMPLETIONFAILED) + _T(" - \"%s\": Path too long"),GetFileName(), strNewname);

		paused = true;
		stopped = true;
		SetStatus(PS_ERROR);
		m_bCompletionError = true;
		SetFileOp(PFOP_NONE);
		if (theApp.emuledlg && theApp.emuledlg->IsRunning())
			VERIFY( PostMessage(theApp.emuledlg->m_hWnd, TM_FILECOMPLETED, FILE_COMPLETION_THREAD_FAILED, (LPARAM)this) );
		return FALSE;
	}

	UncompressFile(strNewname, this);
	SetZoneIdentifier(strNewname);		// may modify the file's "Last Modified" time

	// to have the accurate date stored in known.met we have to update the 'date' of a just completed file.
	// if we don't update the file date here (after commiting the file and before adding the record to known.met), 
	// that file will be rehashed at next startup and there would also be a duplicate entry (hash+size) in known.met
	// because of different file date!
	ASSERT( m_hpartfile.m_hFile == INVALID_HANDLE_VALUE ); // the file must be closed/commited!
	struct _stat st;
	if (_tstat(strNewname, &st) == 0)
	{
		m_tLastModified = st.st_mtime;
		m_tUtcLastModified = m_tLastModified;
		AdjustNTFSDaylightFileTime(m_tUtcLastModified, strNewname);
	}

	// remove part.met file
	if (_tremove(m_fullname))
		theApp.QueueLogLine(true, GetResString(IDS_ERR_DELETEFAILED) + _T(" - ") + CString(_tcserror(errno)), m_fullname);
	
	m_sourcesaver.DeleteFile(this); // salvataggio fonti Anis

	// remove backup files
	CString BAKName(m_fullname);
	BAKName.Append(PARTMET_BAK_EXT);
	if (_taccess(BAKName, 0) == 0 && !::DeleteFile(BAKName))
		theApp.QueueLogLine(true,GetResString(IDS_ERR_DELETE) + _T(" - ") + GetErrorMessage(GetLastError()), BAKName);

	BAKName = m_fullname;
	BAKName.Append(PARTMET_TMP_EXT);
	if (_taccess(BAKName, 0) == 0 && !::DeleteFile(BAKName))
		theApp.QueueLogLine(true,GetResString(IDS_ERR_DELETE) + _T(" - ") + GetErrorMessage(GetLastError()), BAKName);
	
	// initialize 'this' part file for being a 'complete' file, this is to be done *before* releasing the file mutex.
	m_fullname = strNewname;
	SetPath(indir);
	SetFilePath(m_fullname);
	_SetStatus(PS_COMPLETE); // set status of CPartFile object, but do not update GUI (to avoid multi-thread problems)
	paused = false;
	SetFileOp(PFOP_NONE);

	// clear the blackbox to free up memory
	m_CorruptionBlackBox.Free();

	// explicitly unlock the file before posting something to the main thread.
	sLock.Unlock();

	if (theApp.emuledlg && theApp.emuledlg->IsRunning())
		VERIFY( PostMessage(theApp.emuledlg->m_hWnd, TM_FILECOMPLETED, FILE_COMPLETION_THREAD_SUCCESS | (renamed ? FILE_COMPLETION_THREAD_RENAMED : 0), (LPARAM)this) );
	return TRUE;
}

// 'End' of file completion, to avoid multi threading synchronization problems, this is to be invoked from within the
// main thread!
void CPartFile::PerformFileCompleteEnd(DWORD dwResult)
{
	if (dwResult & FILE_COMPLETION_THREAD_SUCCESS)
	{
		SetStatus(PS_COMPLETE); // (set status and) update status-modification related GUI elements
		theApp.knownfiles->SafeAddKFile(this);
		theApp.downloadqueue->RemoveFile(this);
		theApp.mmserver->AddFinishedFile(this);
		if (thePrefs.GetRemoveFinishedDownloads())
			theApp.emuledlg->transferwnd->GetDownloadList()->RemoveFile(this);
		else
			UpdateDisplayedInfo(true);

		theApp.emuledlg->transferwnd->GetDownloadList()->ShowFilesCount();

		thePrefs.Add2DownCompletedFiles();
		thePrefs.Add2DownSessionCompletedFiles();
		thePrefs.SaveCompletedDownloadsStat();

		// 05-Jän-2004 [bc]: ed2k and Kad are already full of totally wrong and/or not properly attached meta data. Take
		// the chance to clean any available meta data tags and provide only tags which were determined by us.
		UpdateMetaDataTags();

		// republish that file to the ed2k-server to update the 'FT_COMPLETE_SOURCES' counter on the server.
		theApp.sharedfiles->RepublishFile(this);

		// give visual response
		Log(LOG_SUCCESS | LOG_STATUSBAR, GetResString(IDS_DOWNLOADDONE), GetFileName());
		theApp.emuledlg->ShowNotifier(GetResString(IDS_TBN_DOWNLOADDONE) + _T('\n') + GetFileName(), TBN_DOWNLOADFINISHED, GetFilePath());
		if (dwResult & FILE_COMPLETION_THREAD_RENAMED)
		{
			CString strFilePath(GetFullName());
			PathStripPath(strFilePath.GetBuffer());
			strFilePath.ReleaseBuffer();
			Log(LOG_STATUSBAR, GetResString(IDS_DOWNLOADRENAMED), strFilePath);
		}
		if(!m_pCollection && CCollection::HasCollectionExtention(GetFileName()))
		{
			m_pCollection = new CCollection();
			if(!m_pCollection->InitCollectionFromFile(GetFilePath(), GetFileName()))
			{
				delete m_pCollection;
				m_pCollection = NULL;
			}
		}
	}

	theApp.downloadqueue->StartNextFileIfPrefs(GetCategory());
}

void  CPartFile::RemoveAllSources(bool bTryToSwap){
	POSITION pos1,pos2;
	for( pos1 = srclist.GetHeadPosition(); ( pos2 = pos1 ) != NULL; ){
		srclist.GetNext(pos1);
		// NEO: MCM - [ManualClientManagement] -- Xanatos -->
		if(srclist.GetAt(pos2)->IsSwapingDisabled())
			srclist.GetAt(pos2)->DisableSwaping(false);
		// NEO: MCM END <-- Xanatos -
		if (bTryToSwap){
			if (!srclist.GetAt(pos2)->SwapToAnotherFile(_T("Removing source. CPartFile::RemoveAllSources()"), true, true, true, NULL, false, false) ) // ZZ:DownloadManager
				theApp.downloadqueue->RemoveSource(srclist.GetAt(pos2), false);
		}
		else
			theApp.downloadqueue->RemoveSource(srclist.GetAt(pos2), false);
	}
	UpdatePartsInfo(); 
	UpdateAvailablePartsCount();

	//[enkeyDEV(Ottavio84) -A4AF-]
	// remove all links A4AF in sources to this file
	if(!A4AFsrclist.IsEmpty())
	{
		POSITION pos1, pos2;
		for(pos1 = A4AFsrclist.GetHeadPosition();(pos2=pos1)!=NULL;)
		{
			A4AFsrclist.GetNext(pos1);
			
			POSITION pos3 = A4AFsrclist.GetAt(pos2)->m_OtherRequests_list.Find(this); 
			if(pos3)
			{ 
				A4AFsrclist.GetAt(pos2)->m_OtherRequests_list.RemoveAt(pos3);
				theApp.emuledlg->transferwnd->GetDownloadList()->RemoveSource(this->A4AFsrclist.GetAt(pos2),this);
			}
			else{
				pos3 = A4AFsrclist.GetAt(pos2)->m_OtherNoNeeded_list.Find(this); 
				if(pos3)
				{ 
					A4AFsrclist.GetAt(pos2)->m_OtherNoNeeded_list.RemoveAt(pos3);
					theApp.emuledlg->transferwnd->GetDownloadList()->RemoveSource(A4AFsrclist.GetAt(pos2),this);
				}
			}
		}
		A4AFsrclist.RemoveAll();
	}

		if (!adusrclist.IsEmpty())
		adusrclist.RemoveAll();
	
	UpdateFileRatingCommentAvail();
}

void CPartFile::DeleteFile(){
	ASSERT ( !m_bPreviewing );

	// Barry - Need to tell any connected clients to stop sending the file
	StopFile(true);

	// feel free to implement a runtime handling mechAnism!
	if (m_AllocateThread != NULL){
		LogWarning(LOG_STATUSBAR, GetResString(IDS_DELETEAFTERALLOC), GetFileName());
		m_bDeleteAfterAlloc=true;
		return;
	}

	theApp.sharedfiles->RemoveFile(this, true);
	theApp.downloadqueue->RemoveFile(this);
	theApp.emuledlg->transferwnd->GetDownloadList()->RemoveFile(this);
	theApp.knownfiles->AddCancelledFileID(GetFileHash());

	if (m_hpartfile.m_hFile != INVALID_HANDLE_VALUE)
		m_hpartfile.Close();

	if (_tremove(m_fullname))
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_DELETE) + _T(" - ") + CString(_tcserror(errno)), m_fullname);
	CString partfilename(RemoveFileExtension(m_fullname));
	if (_tremove(partfilename))
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_DELETE) + _T(" - ") + CString(_tcserror(errno)), partfilename);

	CString BAKName(m_fullname);
	BAKName.Append(PARTMET_BAK_EXT);
	if (_taccess(BAKName, 0) == 0 && !::DeleteFile(BAKName))
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_DELETE) + _T(" - ") + GetErrorMessage(GetLastError()), BAKName);
	
	m_sourcesaver.DeleteFile(this);

	BAKName = m_fullname;
	BAKName.Append(PARTMET_TMP_EXT);
	if (_taccess(BAKName, 0) == 0 && !::DeleteFile(BAKName))
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_DELETE) + _T(" - ") + GetErrorMessage(GetLastError()), BAKName);

	delete this;
}

bool CPartFile::HashSinglePart(UINT partnumber, bool* pbAICHReportedOK)
{
	// Right now we demand that AICH (if we have one) and MD4 agree on a parthash, no matter what
	// This is the most secure way in order to make sure eMule will never deliver a corrupt file,
	// even if one of the hashalgorithms is completely or both somewhat broken
	// This however doesn't means that eMule is guaranteed to be able to finish a file in case
	// one of the algorithms is completely broken, but we will bother about that if it becomes an
	// issue, with the current implementation at least nothing can go horribly wrong (from a security PoV)
	if (pbAICHReportedOK != NULL)
		*pbAICHReportedOK = false;
	if (!m_FileIdentifier.HasExpectedMD4HashCount() && !(m_FileIdentifier.HasAICHHash() && m_FileIdentifier.HasExpectedAICHHashCount()))
	{
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_HASHERRORWARNING), GetFileName());
		m_bMD4HashsetNeeded = true;
		m_bAICHPartHashsetNeeded = true;
		return true;		
	}
	else{
		uchar hashresult[16];
		m_hpartfile.Seek((LONGLONG)PARTSIZE*(uint64)partnumber,0);
		uint32 length = PARTSIZE;
		if ((ULONGLONG)PARTSIZE*(uint64)(partnumber+1) > m_hpartfile.GetLength()){
			length = (UINT)(m_hpartfile.GetLength() - ((ULONGLONG)PARTSIZE*(uint64)partnumber));
			ASSERT( length <= PARTSIZE );
		}
		
		CAICHHashTree* phtAICHPartHash = NULL;
		if (m_FileIdentifier.HasAICHHash() && m_FileIdentifier.HasExpectedAICHHashCount())
		{
			const CAICHHashTree* pPartTree = m_pAICHRecoveryHashSet->FindPartHash((uint16)partnumber);
			if (pPartTree != NULL)
			{
				// use a new part tree, so we don't overwrite any existing recovery data which we might still need lateron
				phtAICHPartHash = new CAICHHashTree(pPartTree->m_nDataSize,pPartTree->m_bIsLeftBranch, pPartTree->GetBaseSize());	
			}
			else
				ASSERT( false );
		}
		CreateHash(&m_hpartfile, length, hashresult, phtAICHPartHash);

		bool bMD4Error = false;
		bool bMD4Checked = false;
		bool bAICHError = false;
		bool bAICHChecked = false;

		if (m_FileIdentifier.HasExpectedMD4HashCount())
		{
			bMD4Checked = true;
			if (GetPartCount() > 1 || GetFileSize()== (uint64)PARTSIZE)
			{
				if (m_FileIdentifier.GetAvailableMD4PartHashCount() > partnumber)
					bMD4Error = md4cmp(hashresult, m_FileIdentifier.GetMD4PartHash(partnumber)) != 0;
				else
				{
					ASSERT( false );
					m_bMD4HashsetNeeded = true;
				}
			}
			else
				bMD4Error = md4cmp(hashresult, m_FileIdentifier.GetMD4Hash()) != 0;
		}
		else
		{
			DebugLogError(_T("MD4 HashSet not present while veryfing part %u for file %s"), partnumber, GetFileName());
			m_bMD4HashsetNeeded = true;
		}

		if (m_FileIdentifier.HasAICHHash() && m_FileIdentifier.HasExpectedAICHHashCount() && phtAICHPartHash != NULL)
		{
			ASSERT( phtAICHPartHash->m_bHashValid );
			bAICHChecked = true;
			if (GetPartCount() > 1)
			{
				if (m_FileIdentifier.GetAvailableAICHPartHashCount() > partnumber)
					bAICHError = m_FileIdentifier.GetRawAICHHashSet()[partnumber] != phtAICHPartHash->m_Hash;
				else
					ASSERT( false );
			}
			else
				bAICHError = m_FileIdentifier.GetAICHHash() != phtAICHPartHash->m_Hash;
		}
		//else
		//	DebugLogWarning(_T("AICH HashSet not present while verifying part %u for file %s"), partnumber, GetFileName());

		delete phtAICHPartHash;
		phtAICHPartHash = NULL;
		if (pbAICHReportedOK != NULL && bAICHChecked)
			*pbAICHReportedOK = !bAICHError;
		if (bMD4Checked && bAICHChecked && bMD4Error != bAICHError)
			DebugLogError(_T("AICH and MD4 HashSet disagree on verifying part %u for file %s. MD4: %s - AICH: %s"), partnumber
			, GetFileName(), bMD4Error ? _T("Corrupt") : _T("OK"), bAICHError ? _T("Corrupt") : _T("OK"));
#ifdef _DEBUG
		else
			DebugLog(_T("Verifying part %u for file %s. MD4: %s - AICH: %s"), partnumber , GetFileName()
			, bMD4Checked ? (bMD4Error ? _T("Corrupt") : _T("OK")) : _T("Unavailable"), bAICHChecked ? (bAICHError ? _T("Corrupt") : _T("OK")) : _T("Unavailable"));	
#endif
		return !bMD4Error && !bAICHError;
	}
}

bool CPartFile::IsCorruptedPart(UINT partnumber) const
{
	return (corrupted_list.Find((uint16)partnumber) != NULL);
}

// Barry - Also want to preview zip/rar files
bool CPartFile::IsArchive(bool onlyPreviewable) const
{
	if (onlyPreviewable) {
		EFileType ftype=GetFileTypeEx((CKnownFile*)this);
		return (ftype==ARCHIVE_RAR || ftype==ARCHIVE_ZIP || ftype==ARCHIVE_ACE);
	}

	return (ED2KFT_ARCHIVE == GetED2KFileTypeID(GetFileName()));
}

bool CPartFile::IsPreviewableFileType() const {
    return IsArchive(true) || IsMovie();
}

void CPartFile::SetDownPriority(uint8 np, bool resort)
{
	//Changed the default resort to true. As it is was, we almost never sorted the download list when a priority changed.
	//If we don't keep the download list sorted, priority means nothing in downloadqueue.cpp->process().
	//Also, if we call this method with the same priotiry, don't do anything to help use less CPU cycles.
	if ( m_iDownPriority != np )
	{
		//We have a new priotiry
		if (np != PR_LOW && np != PR_NORMAL && np != PR_HIGH){
			//This should never happen.. Default to Normal.
			ASSERT(0);
			np = PR_NORMAL;
		}
	
		m_iDownPriority = np;
		//Some methods will change a batch of priorites then call these methods. 
	    if(resort) {
			//Sort the downloadqueue so contacting sources work correctly.
			theApp.downloadqueue->SortByPriority();
			theApp.downloadqueue->CheckDiskspaceTimed();
	    }
		//Update our display to show the new info based on our new priority.
		UpdateDisplayedInfo(true);
		//Save the partfile. We do this so that if we restart eMule before this files does
		//any transfers, it will remember the new priority.
		SavePartFile();
	}
}

bool CPartFile::CanOpenFile() const
{
	return (GetStatus()==PS_COMPLETE);
}

void CPartFile::OpenFile() const
{
	if(m_pCollection)
	{
		CCollectionViewDialog dialog;
		dialog.SetCollection(m_pCollection);
		dialog.DoModal();
	}
	else
		ShellOpenFile(GetFullName(), NULL);
}

bool CPartFile::CanStopFile() const
{
	bool bFileDone = (GetStatus()==PS_COMPLETE || GetStatus()==PS_COMPLETING);
	return (!IsStopped() && GetStatus()!=PS_ERROR && !bFileDone);
}

void CPartFile::StopFile(bool bCancel, bool resort)
{
	// Barry - Need to tell any connected clients to stop sending the file
	PauseFile(false, resort);
	m_LastSearchTimeKad = 0;
	m_TotalSearchesKad = 0;
	RemoveAllSources(true);
	paused = true;
	stopped = true;
	insufficient = false;
	datarate = 0;
	memset(m_anStates,0,sizeof(m_anStates));
	memset(src_stats,0,sizeof(src_stats));	//Bugfix
	memset(net_stats,0,sizeof(net_stats));	//Bugfix

	if (!bCancel)
		FlushBuffer(true);
    if(resort) {
	    theApp.downloadqueue->SortByPriority();
	    theApp.downloadqueue->CheckDiskspace();
    }
	UpdateDisplayedInfo(true);
}

void CPartFile::StopPausedFile()
{
	//Once an hour, remove any sources for files which are no longer active downloads
	EPartFileStatus uState = GetStatus();
	if( (uState==PS_PAUSED || uState==PS_INSUFFICIENT || uState==PS_ERROR) && !stopped && time(NULL) - m_iLastPausePurge > (60*60) )
	{
		StopFile();
	}
	else
	{
		if (m_bDeleteAfterAlloc && m_AllocateThread==NULL)
		{
			DeleteFile();
			return;
		}
	}
}

bool CPartFile::CanPauseFile() const
{
	bool bFileDone = (GetStatus()==PS_COMPLETE || GetStatus()==PS_COMPLETING);
	return (GetStatus()!=PS_PAUSED && GetStatus()!=PS_ERROR && !bFileDone);
}

void CPartFile::PauseFile(bool bInsufficient, bool resort)
{
	// if file is already in 'insufficient' state, don't set it again to insufficient. this may happen if a disk full
	// condition is thrown before the automatically and periodically check free diskspace was done.
	if (bInsufficient && insufficient)
		return;

	// if file is already in 'paused' or 'insufficient' state, do not refresh the purge time
	if (!paused && !insufficient)
		m_iLastPausePurge = time(NULL);
	theApp.downloadqueue->RemoveLocalServerRequest(this);

	if(GetKadFileSearchID())
	{
		Kademlia::CSearchManager::StopSearch(GetKadFileSearchID(), true);
		m_LastSearchTimeKad = 0; //If we were in the middle of searching, reset timer so they can resume searching.
	}

	SetActive(false);

	if (status==PS_COMPLETE || status==PS_COMPLETING)
		return;

	Packet* packet = new Packet(OP_CANCELTRANSFER,0);
	for( POSITION pos = srclist.GetHeadPosition(); pos != NULL; )
	{
		CUpDownClient* cur_src = srclist.GetNext(pos);
		if (cur_src->GetDownloadState() == DS_DOWNLOADING)
		{
			cur_src->SendCancelTransfer(packet);
			cur_src->SetDownloadState(DS_ONQUEUE, _T("You cancelled the download. Sending OP_CANCELTRANSFER"));
		}
	}
	delete packet;

	if (bInsufficient)
	{
		LogError(LOG_STATUSBAR, _T("Insufficient diskspace - pausing download of \"%s\""), GetFileName());
		insufficient = true;
	}
	else
	{
		paused = true;
		insufficient = false;
	}
	NotifyStatusChange();
	datarate = 0;
	m_anStates[DS_DOWNLOADING] = 0; // -khaos--+++> Renamed var.
	if (!bInsufficient)
	{
        if(resort) {
		    theApp.downloadqueue->SortByPriority();
		    theApp.downloadqueue->CheckDiskspace();
        }
		SavePartFile();
	}
	UpdateDisplayedInfo(true);
}

bool CPartFile::CanResumeFile() const
{
	return (GetStatus()==PS_PAUSED || GetStatus()==PS_INSUFFICIENT || (GetStatus()==PS_ERROR && GetCompletionError()));
}

void CPartFile::ResumeFile(bool resort)
{
	if (status==PS_COMPLETE || status==PS_COMPLETING)
		return;
	if (status==PS_ERROR && m_bCompletionError){
		ASSERT( gaplist.IsEmpty() );
		if (gaplist.IsEmpty() && !m_nTotalBufferData && !m_FlushSetting) { //MORPH - Changed by SiRoB, Flush Thread
			// rehashing the file could probably be avoided, but better be in the safe side..
			m_bCompletionError = false;
			CompleteFile(false);
		}
		return;
	}
	paused = false;
	stopped = false;
	SetActive(theApp.IsConnected());
	m_LastSearchTime = 0;
    if(resort) {
	    theApp.downloadqueue->SortByPriority();
	    theApp.downloadqueue->CheckDiskspace();
    }
	SavePartFile();
	NotifyStatusChange();

	UpdateDisplayedInfo(true);
}

void CPartFile::ResumeFileInsufficient()
{
	if (status==PS_COMPLETE || status==PS_COMPLETING)
		return;
	if (!insufficient)
		return;
	AddLogLine(false, _T("Resuming download of \"%s\""), GetFileName());
	insufficient = false;
	SetActive(theApp.IsConnected());
	m_LastSearchTime = 0;
	UpdateDisplayedInfo(true);
}

CString CPartFile::getPartfileStatus() const  //Anis Hireche's AduStreaming mod
{
	switch(GetStatus()){
		case PS_HASHING:
		case PS_WAITINGFORHASH:
			return GetResString(IDS_HASHING);

		case PS_COMPLETING:{
			CString strState = GetResString(IDS_COMPLETING);
			if (GetFileOp() == PFOP_HASHING)
				strState += _T(" (") + GetResString(IDS_HASHING) + _T(")");
			else if (GetFileOp() == PFOP_COPYING)
				strState += _T(" (Copying)");
			else if (GetFileOp() == PFOP_UNCOMPRESSING)
				strState += _T(" (Uncompressing)");
			return strState;
		}

		case PS_COMPLETE:
		{
			return GetResString(IDS_COMPLETE);
		}

		case PS_PAUSED:
			if (stopped)
				return GetResString(IDS_STOPPED);
			return GetResString(IDS_PAUSED);

		case PS_INSUFFICIENT: 
		{
			extern bool AduStream;
			if(!AduStream)
				return _T("Streaming terminato!");
			else
				return GetResString(IDS_INSUFFICIENT);
		}
		case PS_ERROR:
			extern bool AduStream;
			if (!AduStream)
				return _T("Streaming terminato!");
			else {
			if (m_bCompletionError)
				return GetResString(IDS_INSUFFICIENT);
			}
			return GetResString(IDS_ERRORLIKE);
	}

	if (GetSrcStatisticsValue(DS_DOWNLOADING) > 0) {
		extern bool AduStream;
		if(!AduStream) { //Anis -> AduStreaming
			CString str;
			str.Format(_T("Streaming: pronto tra %s"),CastSecondsToHM(getTimeRemainingSimple_Preview()));
			if(IsReadyForPreview() && GetPercentCompleted() > thePrefs.m_BufferStreaming) {
				if(controlstream) {
					controlstream = false;
					theApp.emuledlg->ShowNotifier(_T("Streaming: pronto al lancio!"), TBN_IMPORTANTEVENT);
				}
				return _T("Streaming: pronto al lancio!");
			}
			else
				return str;
		}
		return GetResString(IDS_DOWNLOADING);
	}
	else {
		extern bool AduStream;
		if(!AduStream) {
			if(!this->GetAduSourceCount())
				return _T("Streaming: tempo rimanente indefinito.");
			else
				return _T("Streaming: calcolo tempo rimanente...");
		}
		else
		return GetResString(IDS_WAITING);
	}
} 

int CPartFile::getPartfileStatusRang() const
{
	switch (GetStatus()) {
		case PS_HASHING: 
		case PS_WAITINGFORHASH:
			return 7;

		case PS_COMPLETING:
			return 1;

		case PS_COMPLETE:
			return 0;

		case PS_PAUSED:
			if (IsStopped())
				return 6;
			else
				return 5;
		case PS_INSUFFICIENT:
			return 4;

		case PS_ERROR:
			return 8;
	}
	if (GetSrcStatisticsValue(DS_DOWNLOADING) == 0)
		return 3; // waiting?
	return 2; // downloading?
} 

time_t CPartFile::getTimeRemainingSimple() const
{
	if (GetDatarate() == 0)
		return -1;
	return (time_t)((uint64)(GetFileSize() - GetCompletedSize()) / (uint64)GetDatarate());
}

time_t CPartFile::getTimeRemainingSimple_Preview() const //Anis
{
	if (GetDatarate() == 0)
		return -1;
		time_t bufferpart = ((GetFileSize() / 100) * thePrefs.m_BufferStreaming) - GetCompletedSize();
		if (bufferpart <= 0)
			return 0;
		else
			return bufferpart/(uint64)dataratekadu;
}

time_t CPartFile::getTimeRemaining() const
{
	EMFileSize completesize = GetCompletedSize();
	time_t simple = -1;
	time_t estimate = -1;
	if( GetDatarate() > 0 )
		simple = (time_t)((uint64)(GetFileSize() - completesize) / (uint64)GetDatarate());

	if( GetDlActiveTime() && completesize >= (uint64)512000 )
		estimate = (time_t)((uint64)(GetFileSize() - completesize) / ((double)completesize / (double)GetDlActiveTime()));

	if( simple == -1 )
	{
		//We are not transferring at the moment.
		if( estimate == -1 )
			//We also don't have enough data to guess
			return -1;
		else if( estimate > HR2S(24*15) )
			//The estimate is too high
			return -1;
		else
			return estimate;
	}
	else if( estimate == -1 )
	{
		//We are transferring but estimate doesn't have enough data to guess
		return simple;
	}
	if( simple < estimate )
		return simple;
	if( estimate > HR2S(24*15) )
		//The estimate is too high..
		return -1;
	return estimate;
}

void CPartFile::PreviewFile()
{
	if (thePreviewApps.Preview(this))
		return;

	if (IsArchive(true)){
		if (!m_bRecoveringArchive && !m_bPreviewing)
			CArchiveRecovery::recover(this, true, thePrefs.GetPreviewCopiedArchives());
		return;
	}
	
	if (!IsReadyForPreview()){
		ASSERT( false );
		return;
	}

	if (thePrefs.IsMoviePreviewBackup()){
		if (!CheckFileOpen(GetFilePath(), GetFileName()))
			return;
		m_bPreviewing = true;
		CPreviewThread* pThread = (CPreviewThread*) AfxBeginThread(RUNTIME_CLASS(CPreviewThread), THREAD_PRIORITY_NORMAL,0, CREATE_SUSPENDED);
		pThread->SetValues(this, thePrefs.GetVideoPlayer(), thePrefs.GetVideoPlayerArgs());
		pThread->ResumeThread();
	}
	else{
		ExecutePartFile(this, thePrefs.GetVideoPlayer(), thePrefs.GetVideoPlayerArgs());
	}
}

bool CPartFile::IsReadyForPreview() const
{
	CPreviewApps::ECanPreviewRes ePreviewAppsRes = thePreviewApps.CanPreview(this);
	if (ePreviewAppsRes != CPreviewApps::NotHandled)
		return (ePreviewAppsRes == CPreviewApps::Yes);

	// Barry - Allow preview of archives of any length > 1k
	if (IsArchive(true))
	{
		//if (GetStatus() != PS_COMPLETE && GetStatus() != PS_COMPLETING 
		//	&& GetFileSize()>1024 && GetCompletedSize()>1024 
		//	&& !m_bRecoveringArchive 
		//	&& GetFreeDiskSpaceX(thePrefs.GetTempDir())+100000000 > 2*GetFileSize())
		//	return true;

		// check part file state
	    EPartFileStatus uState = GetStatus();
		if (uState == PS_COMPLETE || uState == PS_COMPLETING)
			return false;

		// check part file size(s)
		if (GetFileSize() < (uint64)1024 || GetCompletedSize() < (uint64)1024)
			return false;

		// check if we already trying to recover an archive file from this part file
		if (m_bRecoveringArchive)
			return false;

		// check free disk space
		uint64 uMinFreeDiskSpace = (thePrefs.GetMinFreeDiskSpace() > 0) ? thePrefs.GetMinFreeDiskSpace() : 20*1024*1024;
		if (thePrefs.GetPreviewCopiedArchives())
			uMinFreeDiskSpace += (uint64)(GetFileSize() * (uint64)2);
		else
			uMinFreeDiskSpace += (uint64)(GetCompletedSize() + (uint64)16*1024);
		if (GetFreeDiskSpaceX(GetTempPath()) < uMinFreeDiskSpace)
			return false;
		return true; 
	}

	if (thePrefs.IsMoviePreviewBackup()) //Anis
	{
		return !( GetStatus(true) != PS_READY || m_bPreviewing || GetPartCount() < 2 || !IsMovie() || (GetFreeDiskSpaceX(GetTempPath()) + 100000000) < GetFileSize() || ( !IsComplete(0,PARTSIZE-1, false) || !IsComplete(PARTSIZE*(uint64)(GetPartCount()-1),GetFileSize()-(uint64)1, false)));
	}
	else
	{
		TCHAR szVideoPlayerFileName[_MAX_FNAME];
		_tsplitpath(thePrefs.GetVideoPlayer(), NULL, NULL, szVideoPlayerFileName, NULL);

		// enable the preview command if the according option is specified 'PreviewSmallBlocks' 
		// or if VideoLAN client is specified
		if (thePrefs.GetPreviewSmallBlocks() || !_tcsicmp(szVideoPlayerFileName, _T("vlc")))
		{
		    if (m_bPreviewing)
			    return false;

		    EPartFileStatus uState = GetStatus();
			if (!(uState == PS_READY || uState == PS_EMPTY || uState == PS_PAUSED || uState == PS_INSUFFICIENT))
			    return false;

			// default: check the ED2K file format to be of type audio, video or CD image. 
			// but because this could disable the preview command for some file types which eMule does not know,
			// this test can be avoided by specifying 'PreviewSmallBlocks=2'
			if (thePrefs.GetPreviewSmallBlocks() <= 1)
			{
				// check the file extension
				EED2KFileType eFileType = GetED2KFileTypeID(GetFileName());
				if (!(eFileType == ED2KFT_VIDEO || eFileType == ED2KFT_AUDIO || eFileType == ED2KFT_CDIMAGE))
				{
					// check the ED2K file type
					const CString& rstrED2KFileType = GetStrTagValue(FT_FILETYPE);
					if (rstrED2KFileType.IsEmpty() || !(!_tcscmp(rstrED2KFileType, _T(ED2KFTSTR_AUDIO)) || !_tcscmp(rstrED2KFileType, _T(ED2KFTSTR_VIDEO))))
						return false;
				}
			}

		    // If it's an MPEG file, VLC is even capable of showing parts of the file if the beginning of the file is missing!
		    bool bMPEG = false;
		    LPCTSTR pszExt = _tcsrchr(GetFileName(), _T('.'));
		    if (pszExt != NULL){
			    CString strExt(pszExt);
			    strExt.MakeLower();
			    bMPEG = (strExt==_T(".mpg") || strExt==_T(".mpeg") || strExt==_T(".mpe") || strExt==_T(".mp3") || strExt==_T(".mp2") || strExt==_T(".mpa"));
		    }

		    if (bMPEG){
			    // TODO: search a block which is at least 16K (Audio) or 256K (Video)
			    if (GetCompletedSize() < (uint64)16*1024)
				    return false;
		    }
		    else{
			    // For AVI files it depends on the used codec..
				if (thePrefs.GetPreviewSmallBlocks() >= 2){
					if (GetCompletedSize() < (uint64)256*1024)
						return false;
				}
				else{
				    if (!IsComplete(0, 256*1024, false))
					    return false;
			    }
			}
    
		    return true;
		}
		else{
		    return !((GetStatus() != PS_READY && GetStatus() != PS_PAUSED) 
				    || m_bPreviewing || GetPartCount() < 2 || !IsMovie() || !IsComplete(0,PARTSIZE-1, false)); 
		}
	}
}

void CPartFile::UpdateAvailablePartsCount()
{
	UINT availablecounter = 0;
	UINT iPartCount = GetPartCount();
	for (UINT ixPart = 0; ixPart < iPartCount; ixPart++){
		for (POSITION pos = srclist.GetHeadPosition(); pos; ){
			if (srclist.GetNext(pos)->IsPartAvailable(ixPart)){
				availablecounter++; 
				break;
			}
		}
	}
	if (iPartCount == availablecounter && availablePartsCount < iPartCount)
		lastseencomplete = CTime::GetCurrentTime();
	availablePartsCount = availablecounter;
}

Packet* CPartFile::CreateSrcInfoPacket(const CUpDownClient* forClient, uint8 byRequestedVersion, uint16 nRequestedOptions) const
{
	if (!IsPartFile() || srclist.IsEmpty())
		return CKnownFile::CreateSrcInfoPacket(forClient, byRequestedVersion, nRequestedOptions);

	if (md4cmp(forClient->GetUploadFileID(), GetFileHash()) != 0) {
		// should never happen
		DEBUG_ONLY( DebugLogError(_T("*** %hs - client (%s) upload file \"%s\" does not match file \"%s\""), __FUNCTION__, forClient->DbgGetClientInfo(), DbgGetFileInfo(forClient->GetUploadFileID()), GetFileName()) );
		ASSERT(0);
		return NULL;
	}

	// check whether client has either no download status at all or a download status which is valid for this file
	if (!(forClient->GetUpPartCount() == 0 && forClient->GetUpPartStatus() == NULL)
		&& !(forClient->GetUpPartCount() == GetPartCount() && forClient->GetUpPartStatus() != NULL))
	{
		// should never happen
		DEBUG_ONLY( DebugLogError(_T("*** %hs - part count (%u) of client (%s) does not match part count (%u) of file \"%s\""), __FUNCTION__, forClient->GetUpPartCount(), forClient->DbgGetClientInfo(), GetPartCount(), GetFileName()) );
		ASSERT(0);
		return NULL;
	}

	if (!(GetStatus() == PS_READY || GetStatus() == PS_EMPTY))
		return NULL;

	CSafeMemFile data(1024);
	
	uint8 byUsedVersion;
	bool bIsSX2Packet;
	if (forClient->SupportsSourceExchange2() && byRequestedVersion > 0){
		// the client uses SourceExchange2 and requested the highest version he knows
		// and we send the highest version we know, but of course not higher than his request
		byUsedVersion = min(byRequestedVersion, (uint8)SOURCEEXCHANGE2_VERSION);
		bIsSX2Packet = true;
		data.WriteUInt8(byUsedVersion);

		// we don't support any special SX2 options yet, reserved for later use
		if (nRequestedOptions != 0)
			DebugLogWarning(_T("Client requested unknown options for SourceExchange2: %u (%s)"), nRequestedOptions, forClient->DbgGetClientInfo());
	}
	else{
		byUsedVersion = forClient->GetSourceExchange1Version();
		bIsSX2Packet = false;
		if (forClient->SupportsSourceExchange2())
			DebugLogWarning(_T("Client which announced to support SX2 sent SX1 packet instead (%s)"), forClient->DbgGetClientInfo());
	}

	UINT nCount = 0;
	data.WriteHash16(m_FileIdentifier.GetMD4Hash());
	data.WriteUInt16((uint16)nCount);
	
	bool bNeeded;
	const uint8* reqstatus = forClient->GetUpPartStatus();
	//Mod Adu
	//Ded
	//scambiamo prima le fonti FastWeb
	if (forClient->HasFastwebIP())
		for (POSITION pos = srclist.GetHeadPosition();pos != 0;){
			bNeeded = false;
			const CUpDownClient* cur_src = srclist.GetNext(pos);
			if (!cur_src->HasFastwebIP())
				continue;
			if (cur_src->HasLowID() || !cur_src->IsValidSource())
				continue;
			const uint8* srcstatus = cur_src->GetPartStatus();
			if (srcstatus){
				if (cur_src->GetPartCount() == GetPartCount()){
					const uint8* reqstatus = forClient->GetPartStatus();
					if (reqstatus){
						ASSERT( forClient->GetPartCount() == GetPartCount() );
						// only send sources which have needed parts for this client
						for (UINT x = 0; x < GetPartCount(); x++){
							if (srcstatus[x] && !reqstatus[x]){
								bNeeded = true;
								break;
							}
						}
					}
					else{
						// We know this client is valid. But don't know the part count status.. So, currently we just send them.
						for (UINT x = 0; x < GetPartCount(); x++){
							if (srcstatus[x]){
								bNeeded = true;
								break;
							}
						}
					}
				}
				else{
					// should never happen
					if (thePrefs.GetVerbose())
						DEBUG_ONLY(DebugLogError(_T("*** %hs - found source (%s) with wrong partcount (%u) attached to partfile \"%s\" (partcount=%u)"), __FUNCTION__, cur_src->DbgGetClientInfo(), cur_src->GetPartCount(), GetFileName(), GetPartCount()));
				}
			}

			if (bNeeded){
				nCount++;
				uint32 dwID;
				if (byUsedVersion >= 3)
					dwID = cur_src->GetUserIDHybrid();
				else
					dwID = ntohl(cur_src->GetUserIDHybrid());
				data.WriteUInt32(dwID);
				data.WriteUInt16(cur_src->GetUserPort());
				data.WriteUInt32(cur_src->GetServerIP());
				data.WriteUInt16(cur_src->GetServerPort());
				if (byUsedVersion >= 2)
					data.WriteHash16(cur_src->GetUserHash());
				if (byUsedVersion >= 4){
					// CryptSettings - SourceExchange V4
					// 5 Reserved (!)
					// 1 CryptLayer Required
					// 1 CryptLayer Requested
					// 1 CryptLayer Supported
					const uint8 uSupportsCryptLayer	= forClient->SupportsCryptLayer() ? 1 : 0;
					const uint8 uRequestsCryptLayer	= thePrefs.IsAduClientCryptLayerRequested() ? 1 : 0;
					const uint8 uRequiresCryptLayer	= thePrefs.IsAduClientCryptLayerRequired() ? 1 : 0;
					const uint8 byCryptOptions = (uRequiresCryptLayer << 2) | (uRequestsCryptLayer << 1) | (uSupportsCryptLayer << 0);
					data.WriteUInt8(byCryptOptions);
				}
				if (nCount > 500)
					break;
			}
		}
	
	if (nCount<=500)
	//Mod Adu
	//Fine
	for (POSITION pos = srclist.GetHeadPosition();pos != 0;){
		bNeeded = false;
		const CUpDownClient* cur_src = srclist.GetNext(pos);
		//Mod Adu
		//Ded
		//scambiamo prima le fonti FastWeb
		if (cur_src->HasFastwebIP())
			continue;
		//Mod Adu
		//Fine
		if (cur_src->HasLowID() || !cur_src->IsValidSource())
			continue;
		const uint8* srcstatus = cur_src->GetPartStatus();
		if (srcstatus){
			if (cur_src->GetPartCount() == GetPartCount()){
				if (reqstatus){
					ASSERT( forClient->GetUpPartCount() == GetPartCount() );
					// only send sources which have needed parts for this client
					for (UINT x = 0; x < GetPartCount(); x++){
						if (srcstatus[x] && !reqstatus[x]){
							bNeeded = true;
							break;
						}
					}
				}
				else{
					// We know this client is valid. But don't know the part count status.. So, currently we just send them.
					for (UINT x = 0; x < GetPartCount(); x++){
						if (srcstatus[x]){
							bNeeded = true;
							break;
						}
					}
				}
			}
			else{
				// should never happen
				if (thePrefs.GetVerbose())
					DEBUG_ONLY(DebugLogError(_T("*** %hs - found source (%s) with wrong partcount (%u) attached to partfile \"%s\" (partcount=%u)"), __FUNCTION__, cur_src->DbgGetClientInfo(), cur_src->GetPartCount(), GetFileName(), GetPartCount()));
			}
		}

		if (bNeeded){
			nCount++;
			uint32 dwID;
			if (byUsedVersion >= 3)
				dwID = cur_src->GetUserIDHybrid();
			else
				dwID = ntohl(cur_src->GetUserIDHybrid());
			data.WriteUInt32(dwID);
			data.WriteUInt16(cur_src->GetUserPort());
			data.WriteUInt32(cur_src->GetServerIP());
			data.WriteUInt16(cur_src->GetServerPort());
			if (byUsedVersion >= 2)
				data.WriteHash16(cur_src->GetUserHash());
			if (byUsedVersion >= 4){
				// ConnectSettings - SourceExchange V4
				// 4 Reserved (!)
				// 1 DirectCallback Supported/Available 
				// 1 CryptLayer Required
				// 1 CryptLayer Requested
				// 1 CryptLayer Supported
				const uint8 uSupportsCryptLayer	= cur_src->SupportsCryptLayer() ? 1 : 0;
				const uint8 uRequestsCryptLayer	= thePrefs.IsAduClientCryptLayerRequested() ? 1 : 0;
				const uint8 uRequiresCryptLayer	= thePrefs.IsAduClientCryptLayerRequired() ? 1 : 0;
				//const uint8 uDirectUDPCallback	= cur_src->SupportsDirectUDPCallback() ? 1 : 0;
				const uint8 byCryptOptions = /*(uDirectUDPCallback << 3) |*/ (uRequiresCryptLayer << 2) | (uRequestsCryptLayer << 1) | (uSupportsCryptLayer << 0);
				data.WriteUInt8(byCryptOptions);
			}
			if (nCount > 500)
				break;
		}
	}
	if (!nCount)
		return 0;
	data.Seek(bIsSX2Packet ? 17 : 16, SEEK_SET);
	data.WriteUInt16((uint16)nCount);

	Packet* result = new Packet(&data, OP_EMULEPROT);
	result->opcode = bIsSX2Packet ? OP_ANSWERSOURCES2 : OP_ANSWERSOURCES;
	// (1+)16+2+501*(4+2+4+2+16+1) = 14547 (14548) bytes max.
	if (result->size > 354)
		result->PackPacket();
	if (thePrefs.GetDebugSourceExchange())
		AddDebugLogLine(false, _T("SXSend: Client source response SX2=%s, Version=%u; Count=%u, %s, File=\"%s\""), bIsSX2Packet ? _T("Yes") : _T("No"), byUsedVersion, nCount, forClient->DbgGetClientInfo(), GetFileName());
	return result;
}

void CPartFile::AddClientSources(CSafeMemFile* sources, uint8 uClientSXVersion, bool bSourceExchange2, const CUpDownClient* pClient)
{
	if (stopped)
		return;

	UINT nCount = 0;

	if (thePrefs.GetDebugSourceExchange()) {
		CString strDbgClientInfo;
		if (pClient)
			strDbgClientInfo.Format(_T("%s, "), pClient->DbgGetClientInfo());
		AddDebugLogLine(false, _T("SXRecv: Client source response; SX2=%s, Ver=%u, %sFile=\"%s\""), bSourceExchange2 ? _T("Yes") : _T("No"), uClientSXVersion, strDbgClientInfo, GetFileName());
	}

	UINT uPacketSXVersion = 0;
	if (!bSourceExchange2){
		// for SX1 (deprecated):
		// Check if the data size matches the 'nCount' for v1 or v2 and eventually correct the source
		// exchange version while reading the packet data. Otherwise we could experience a higher
		// chance in dealing with wrong source data, userhashs and finally duplicate sources.
		nCount = sources->ReadUInt16();
		UINT uDataSize = (UINT)(sources->GetLength() - sources->GetPosition());
		// Checks if version 1 packet is correct size
		if (nCount*(4+2+4+2) == uDataSize)
		{
			// Received v1 packet: Check if remote client supports at least v1
			if (uClientSXVersion < 1) {
				if (thePrefs.GetVerbose()) {
					CString strDbgClientInfo;
					if (pClient)
						strDbgClientInfo.Format(_T("%s, "), pClient->DbgGetClientInfo());
					//DebugLogWarning(_T("Received invalid SX packet (v%u, count=%u, size=%u), %sFile=\"%s\""), uClientSXVersion, nCount, uDataSize, strDbgClientInfo, GetFileName());
		}
				return;


			}
				uPacketSXVersion = 1;
		}
		// v4 packets
		else if (nCount*(4+2+4+2+16+1) == uDataSize)
		{
			// Received v4 packet: Check if remote client supports at least v4
			if (uClientSXVersion < 2) {
				if (thePrefs.GetVerbose()) {
					CString strDbgClientInfo;
					if (pClient)
						strDbgClientInfo.Format(_T("%s, "), pClient->DbgGetClientInfo());
					DebugLogWarning(_T("Received invalid SX packet (v%u, count=%u, size=%u), %sFile=\"%s\""), uClientSXVersion, nCount, uDataSize, strDbgClientInfo, GetFileName());
				}
				return;
			}
		if (uClientSXVersion == 2)
			uPacketSXVersion = 2;
		else
			uPacketSXVersion = 3;
	}
	// v4 packets
	else if (nCount*(4+2+4+2+16+1) == uDataSize)
	{
		// Received v4 packet: Check if remote client supports at least v4
		if (uClientSXVersion < 4) {
	if (thePrefs.GetVerbose()) {
				CString strDbgClientInfo;
				if (pClient)
					strDbgClientInfo.Format(_T("%s, "), pClient->DbgGetClientInfo());
				DebugLogWarning(_T("Received invalid SX packet (v%u, count=%u, size=%u), %sFile=\"%s\""), uClientSXVersion, nCount, uDataSize, strDbgClientInfo, GetFileName());
			}
			return;
		}
		uPacketSXVersion = 4;
	}
	else
	{
		// If v5+ inserts additional data (like v2), the above code will correctly filter those packets.
		// If v5+ appends additional data after <count>(<Sources>)[count], we are in trouble with the 
		// above code. Though a client which does not understand v5+ should never receive such a packet.
		if (thePrefs.GetVerbose()) {
			CString strDbgClientInfo;
			if (pClient)
				strDbgClientInfo.Format(_T("%s, "), pClient->DbgGetClientInfo());
			DebugLogWarning(_T("Received invalid SX packet (v%u, count=%u, size=%u), %sFile=\"%s\""), uClientSXVersion, nCount, uDataSize, strDbgClientInfo, GetFileName());
		}
		return;
	}
		ASSERT( uPacketSXVersion != 0 );
	}
	else{
		// for SX2:
		// We only check if the version is known by us and do a quick sanitize check on known version
		// other then SX1, the packet will be ignored if any error appears, sicne it can't be a "misunderstanding" anymore
		if (uClientSXVersion > SOURCEEXCHANGE2_VERSION || uClientSXVersion == 0){
			if (thePrefs.GetVerbose()) {
				CString strDbgClientInfo;
				if (pClient)
					strDbgClientInfo.Format(_T("%s, "), pClient->DbgGetClientInfo());

				DebugLogWarning(_T("Received invalid SX2 packet - Version unknown (v%u), %sFile=\"%s\""), uClientSXVersion, strDbgClientInfo, GetFileName());
			}
			return;
		}
		// all known versions use the first 2 bytes as count and unknown version are already filtered above
		nCount = sources->ReadUInt16();
		UINT uDataSize = (UINT)(sources->GetLength() - sources->GetPosition());	
		bool bError = false;
		switch (uClientSXVersion){
			case 1:
				bError = nCount*(4+2+4+2) != uDataSize;
				break;
			case 2:
			case 3:
				bError = nCount*(4+2+4+2+16) != uDataSize;
				break;
			case 4:
				bError = nCount*(4+2+4+2+16+1) != uDataSize;
				break;
			default:
				ASSERT( false );
		}

		if (bError){
			ASSERT( false );
			if (thePrefs.GetVerbose()) {
				CString strDbgClientInfo;
				if (pClient)
					strDbgClientInfo.Format(_T("%s, "), pClient->DbgGetClientInfo());
				DebugLogWarning(_T("Received invalid/corrupt SX2 packet (v%u, count=%u, size=%u), %sFile=\"%s\""), uClientSXVersion, nCount, uDataSize, strDbgClientInfo, GetFileName());
			}
			return;
		}
		uPacketSXVersion = uClientSXVersion;
	}

	for (UINT i = 0; i < nCount; i++)
	{
		uint32 dwID = sources->ReadUInt32();
		uint16 nPort = sources->ReadUInt16();
		uint32 dwServerIP = sources->ReadUInt32();
		uint16 nServerPort = sources->ReadUInt16();

		uchar achUserHash[16];
		if (uPacketSXVersion >= 2)
			sources->ReadHash16(achUserHash);

		uint8 byCryptOptions = 0;
		if (uPacketSXVersion >= 4)
			byCryptOptions = sources->ReadUInt8();

		// Clients send ID's in the Hyrbid format so highID clients with *.*.*.0 won't be falsely switched to a lowID..
		if (uPacketSXVersion >= 3)
		{
			uint32 dwIDED2K = ntohl(dwID);

			// check the HighID(IP) - "Filter LAN IPs" and "IPfilter" the received sources IP addresses
			if (!IsLowID(dwID))
			{
				if (!IsGoodIP(dwIDED2K))
				{
					// check for 0-IP, localhost and optionally for LAN addresses
					//if (thePrefs.GetLogFilteredIPs())
					//	AddDebugLogLine(false, _T("Ignored source (IP=%s) received via source exchange - bad IP"), ipstr(dwIDED2K));
					continue;
				}
				if (theApp.ipfilter->IsFiltered(dwIDED2K))
				{
					if (thePrefs.GetLogFilteredIPs())
						AddDebugLogLine(false, _T("Ignored source (IP=%s) received via source exchange - IP filter (%s)"), ipstr(dwIDED2K), theApp.ipfilter->GetLastHit());
					continue;
				}
				if (theApp.clientlist->IsBannedClient(dwIDED2K)){
#ifdef _DEBUG
					if (thePrefs.GetLogBannedClients()){
						CUpDownClient* pClient = theApp.clientlist->FindClientByIP(dwIDED2K);
						AddDebugLogLine(false, _T("Ignored source (IP=%s) received via source exchange - banned client %s"), ipstr(dwIDED2K), pClient->DbgGetClientInfo());
					}
#endif
					continue;
				}
			}

			// additionally check for LowID and own IP
			if (!CanAddSource(dwID, nPort, dwServerIP, nServerPort, NULL, false))
			{
				//if (thePrefs.GetLogFilteredIPs())
				//	AddDebugLogLine(false, _T("Ignored source (IP=%s) received via source exchange"), ipstr(dwIDED2K));
				continue;
			}
		}
		else
		{
			// check the HighID(IP) - "Filter LAN IPs" and "IPfilter" the received sources IP addresses
			if (!IsLowID(dwID))
			{
				if (!IsGoodIP(dwID))
				{ 
					// check for 0-IP, localhost and optionally for LAN addresses
					//if (thePrefs.GetLogFilteredIPs())
					//	AddDebugLogLine(false, _T("Ignored source (IP=%s) received via source exchange - bad IP"), ipstr(dwID));
					continue;
				}
				if (theApp.ipfilter->IsFiltered(dwID))
				{
					if (thePrefs.GetLogFilteredIPs())
						AddDebugLogLine(false, _T("Ignored source (IP=%s) received via source exchange - IP filter (%s)"), ipstr(dwID), theApp.ipfilter->GetLastHit());
					continue;
				}
				if (theApp.clientlist->IsBannedClient(dwID)){
#ifdef _DEBUG
					if (thePrefs.GetLogBannedClients()){
						CUpDownClient* pClient = theApp.clientlist->FindClientByIP(dwID);
						AddDebugLogLine(false, _T("Ignored source (IP=%s) received via source exchange - banned client %s"), ipstr(dwID), pClient->DbgGetClientInfo());
					}
#endif
					continue;
				}
			}

			// additionally check for LowID and own IP
			if (!CanAddSource(dwID, nPort, dwServerIP, nServerPort))
			{
				//if (thePrefs.GetLogFilteredIPs())
				//	AddDebugLogLine(false, _T("Ignored source (IP=%s) received via source exchange"), ipstr(dwID));
				continue;
			}
		}

		if (GetMaxSources() > GetSourceCount())
		{
			CUpDownClient* newsource;
			if (uPacketSXVersion >= 3)
				newsource = new CUpDownClient(this, nPort, dwID, dwServerIP, nServerPort, false);
			else
				newsource = new CUpDownClient(this, nPort, dwID, dwServerIP, nServerPort, true);
			if (uPacketSXVersion >= 2)
				newsource->SetUserHash(achUserHash);
			if (uPacketSXVersion >= 4) {
				newsource->SetConnectOptions(byCryptOptions, true, false);
				//if (thePrefs.GetDebugSourceExchange()) // remove this log later
				//	AddDebugLogLine(false, _T("Received CryptLayer aware (%u) source from V4 Sourceexchange (%s)"), byCryptOptions, newsource->DbgGetClientInfo());
			}
			newsource->SetSourceFrom(SF_SOURCE_EXCHANGE);
			theApp.downloadqueue->CheckAndAddSource(this, newsource);
		} 
		else
			break;
	}
}

// making this function return a higher when more sources have the extended
// protocol will force you to ask a larger variety of people for sources
/*int CPartFile::GetCommonFilePenalty() const
{
	//TODO: implement, but never return less than MINCOMMONPENALTY!
	return MINCOMMONPENALTY;
}
*/
/* Barry - Replaces BlockReceived() 

           Originally this only wrote to disk when a full 180k block 
           had been received from a client, and only asked for data in 
		   180k blocks.

		   This meant that on average 90k was lost for every connection
		   to a client data source. That is a lot of wasted data.

		   To reduce the lost data, packets are now written to a buffer
		   and flushed to disk regularly regardless of size downloaded.
		   This includes compressed packets.

		   Data is also requested only where gaps are, not in 180k blocks.
		   The requests will still not exceed 180k, but may be smaller to
		   fill a gap.
*/
uint32 CPartFile::WriteToBuffer(uint64 transize, const BYTE *data, uint64 start, uint64 end, Requested_Block_Struct *block, 
								const CUpDownClient* client)
{
	ASSERT( (sint64)transize > 0 );
	ASSERT( start <= end );

	// Increment transferred bytes counter for this file
	if (client) //MORPH - Added by SiRoB, Import Part don't count as transfered
		m_uTransferred += transize;

	// This is needed a few times
	uint32 lenData = (uint32)(end - start + 1);
	ASSERT( (int)lenData > 0 && (uint64)(end - start + 1) == lenData);

	if (lenData > transize) {
		m_uCompressionGain += lenData - transize;
		thePrefs.Add2SavedFromCompression(lenData - transize);
	}

	//MORPH - Optimization, don't check this twice only use the loop to write into gap 
	// Occasionally packets are duplicated, no point writing it twice
#ifdef _DEBUG
	if (IsComplete(start, end, false))
	{
		if (thePrefs.GetVerbose())
			AddDebugLogLine(false, _T("PrcBlkPkt: Already written block %s; File=%s; %s"), DbgGetBlockInfo(start, end), GetFileName(), client->DbgGetClientInfo());
		return 0;
	}
#endif
	// security sanitize check to make sure we do not write anything into an already hashed complete chunk
	const uint64 nStartChunk = start / PARTSIZE;
	const uint64 nEndChunk = end / PARTSIZE;
	if (IsComplete(PARTSIZE * (uint64)nStartChunk, (PARTSIZE * (uint64)(nStartChunk + 1)) - 1, false)){
		DebugLogError( _T("PrcBlkPkt: Received data touches already hashed chunk - ignored (start) %s; File=%s; %s"), DbgGetBlockInfo(start, end), GetFileName(), client->DbgGetClientInfo());
		return 0;
	}
	else if (nStartChunk != nEndChunk) {
		if (IsComplete(PARTSIZE * (uint64)nEndChunk, (PARTSIZE * (uint64)(nEndChunk + 1)) - 1, false)){
			DebugLogError( _T("PrcBlkPkt: Received data touches already hashed chunk - ignored (end) %s; File=%s; %s"), DbgGetBlockInfo(start, end), GetFileName(), client->DbgGetClientInfo());
			return 0;
		}
		else
			DEBUG_ONLY( DebugLogWarning(_T("PrcBlkPkt: Received data crosses chunk boundaries %s; File=%s; %s"), DbgGetBlockInfo(start, end), GetFileName(), client->DbgGetClientInfo()) );
	}

	// Anis: SafeHash
	CSingleLock sLock(&ICH_mut,true);	// Wait for ICH result
	ParseICHResult();	// Check result to prevent post-complete writing

	lenData = 0;	// this one is an effective counter

	// only write to gaps
	for (POSITION pos1 = gaplist.GetHeadPosition();pos1 != NULL;){
		Gap_Struct* cur_gap = gaplist.GetNext(pos1);

		if (start > cur_gap->end || end < cur_gap->start)
			continue;

		// Create a new buffered queue entry
		PartFileBufferedData *item = new PartFileBufferedData;
		item->start = (start > cur_gap->start)?start:cur_gap->start;
		item->end = (end < cur_gap->end)?end:cur_gap->end;
		item->block = block;

		uint32 lenDataClipped = (uint32)(item->end - item->start + 1);
		ASSERT(lenDataClipped <= end - start + 1);
		// log transferinformation in our "blackbox"
		m_CorruptionBlackBox.TransferredData(item->start, item->end, client);

		// Create copy of data as new buffer
		BYTE *buffer = new BYTE[lenDataClipped];
		memcpy(buffer, data+(item->start-start), lenDataClipped);
		item->data = buffer;
	// Anis: SafeHash

		// Add to the queue in the correct position (most likely the end)
		PartFileBufferedData *queueItem;
		bool added = false;
		m_BufferedData_list_Locker.Lock(); //MORPH - Flush Thread
		POSITION pos = m_BufferedData_list.GetTailPosition();
		while (pos != NULL)
		{	
			POSITION posLast = pos;
			queueItem = m_BufferedData_list.GetPrev(pos);
			if (item->end > queueItem->end)
			{
				added = true;
				m_BufferedData_list.InsertAfter(posLast, item);
				break;
			}

		}
		if (!added)
			m_BufferedData_list.AddHead(item);
		m_nTotalBufferData += lenDataClipped;
		m_BufferedData_list_Locker.Unlock(); //MORPH - Flush Thread
	// Anis: SafeHash
		lenData += lenDataClipped;	// calculate actual added data
		
	}
	// Anis: SafeHash

	// Increment buffer size marker
//	m_nTotalBufferData += lenData; //MORPH - Flush Thread, Moved above

	// Mark this small section of the file as filled
	FillGap(start, end);	// Anis: SafeHash - clean coding, removed "item->"

	// Update the flushed mark on the requested block 
	// The loop here is unfortunate but necessary to detect deleted blocks.
	POSITION pos = requestedblocks_list.GetHeadPosition();	// Anis: SafeHash
	while (pos != NULL)
	{	
		// Anis: SafeHash - clean coding, removed "item->"
		if (requestedblocks_list.GetNext(pos) == block) {
			block->transferred += lenData;
			break; //MORPH - Optimization
		}
		// Anis: SafeHash
	}

	if (gaplist.IsEmpty())
		FlushBuffer(true);
	//MORPH START - Added by SiRoB, Import Parts
	else if (GetStatus()!=PS_READY && GetStatus()!=PS_EMPTY)
		FlushBuffer();
	//MORPH END   - Added by SiRoB, Import Parts

	// Return the length of data written to the buffer
	return lenData;
}

void CPartFile::FlushBuffer(bool forcewait, bool bForceICH, bool /*bNoAICH*/)
{
	//MORPH START - Flush Thread
	if (forcewait) { //We need to wait for flush thread to terminate
		CWinThread* pThread = m_FlushThread;
		if (pThread != NULL) { //We are flushing something to disk
			HANDLE hThread = pThread->m_hThread;
			// 2 minutes to let the thread finish
			pThread->SetThreadPriority(THREAD_PRIORITY_NORMAL); 
			((CPartFileFlushThread*) m_FlushThread)->StopFlush();
			if (WaitForSingleObject(hThread, 120000) == WAIT_TIMEOUT) {
				AddDebugLogLine(true, _T("Flushing (force=true) failed.(%s)"), GetFileName()/*, m_nTotalBufferData, m_BufferedData_list.GetCount(), m_uTransferred, m_nLastBufferFlushTime*/);
				TerminateThread(hThread, 100); // Should never happen
				ASSERT(0);
			}
			m_FlushThread = NULL;
		}
		if (m_FlushSetting != NULL) //We noramly flushed something to disk
			FlushDone();
	} else 
	if (m_FlushSetting != NULL) { //Some thing is going to be flushed or already flushed 
		                          //wait the window call back to call FlushDone()
		return;
	}
	//MORPH END   - Flush Thread
	bool bIncreasedFile=false;

	m_nLastBufferFlushTime = GetTickCount();
	if (m_nTotalBufferData==0)
		return;

	if (m_AllocateThread!=NULL) {
		// diskspace is being allocated right now.
		// so dont write and keep the data in the buffer for later.
		return;
	}else if (m_iAllocinfo>0) {
		bIncreasedFile=true;
		m_iAllocinfo=0;
	}

	// SLUGFILLER: SafeHash
	if (forcewait) {	// Last chance to grab any ICH results
		CSingleLock sLock(&ICH_mut,true);	// ICH locks the file - otherwise it may be written to while being checked
		ParseICHResult();	// Check result from ICH
	}
	// SLUGFILLER: SafeHash
	
	//if (thePrefs.GetVerbose())
	//	AddDebugLogLine(false, _T("Flushing file %s - buffer size = %ld bytes (%ld queued items) transferred = %ld [time = %ld]"), GetFileName(), m_nTotalBufferData, m_BufferedData_list.GetCount(), m_uTransferred, m_nLastBufferFlushTime);

	UINT partCount = GetPartCount();
	bool *changedPart = new bool[partCount];
	// Remember which parts need to be checked at the end of the flush
	for (UINT partNumber = 0; partNumber < partCount; partNumber++)
		changedPart[partNumber] = false;

	try
	{
		//MORPH START - Flush Thread
		//WiZaRd: no need to double-parse... also, we are probably not supposed to call ParseICHResult if forcewait is false
		/*
		// SLUGFILLER: SafeHash
		CSingleLock sLock(&ICH_mut,true);	// ICH locks the file - otherwise it may be written to while being checked
		ParseICHResult();	// Check result from ICH
		// SLUGFILLER: SafeHash
		*/

		//Creating the Thread to flush to disk
		m_FlushSetting = new FlushDone_Struct;
		m_FlushSetting->bIncreasedFile = bIncreasedFile;
		m_FlushSetting->bForceICH = bForceICH;
		m_FlushSetting->changedPart = changedPart;
		if (forcewait == false) {
			if (m_FlushThread == NULL) {
				m_FlushThread = AfxBeginThread(RUNTIME_CLASS(CPartFileFlushThread), THREAD_PRIORITY_BELOW_NORMAL,0, CREATE_SUSPENDED);
				((CPartFileFlushThread*) m_FlushThread)->ResumeThread();
			}
			if (m_FlushThread) {
				((CPartFileFlushThread*) m_FlushThread)->SetPartFile(this);
				return;
			}
		}
	
		//MORPH END   - Flush Thread
		bool bCheckDiskspace = thePrefs.GetMinFreeDiskSpace() > 0;
		ULONGLONG uFreeDiskSpace = bCheckDiskspace ? GetFreeDiskSpaceX(GetTempPath()) : 0;

		// Check free diskspace for compressed/sparse files before possibly increasing the file size
		if (bCheckDiskspace && !IsNormalFile())
		{
			// Compressed/sparse files; regardless whether the file is increased in size, 
			// check the amount of data which will be written
			// would need to use disk cluster sizes for more accuracy
			if (m_nTotalBufferData + thePrefs.GetMinFreeDiskSpace() >= uFreeDiskSpace)
				AfxThrowFileException(CFileException::diskFull, 0, m_hpartfile.GetFileName());
		}

		// Ensure file is big enough to write data to (the last item will be the furthest from the start)
		PartFileBufferedData *item = m_BufferedData_list.GetTail();
		if (m_hpartfile.GetLength() <= item->end)
		{
			uint64 newsize = thePrefs.GetAllocCompleteMode() ? GetFileSize() : (item->end + 1);
			ULONGLONG uIncrease = newsize - m_hpartfile.GetLength();

			// Check free diskspace for normal files before increasing the file size
			if (bCheckDiskspace && IsNormalFile())
			{
				// Normal files; check if increasing the file would reduce the amount of min. free space beyond the limit
				// would need to use disk cluster sizes for more accuracy
				if (uIncrease + thePrefs.GetMinFreeDiskSpace() >= uFreeDiskSpace)
					AfxThrowFileException(CFileException::diskFull, 0, m_hpartfile.GetFileName());
			}

			if (!IsNormalFile() || uIncrease<2097152) 
				forcewait=true;	// <2MB -> alloc it at once

			// Allocate filesize
			if (!forcewait) {
				m_AllocateThread= AfxBeginThread(AllocateSpaceThread, this, THREAD_PRIORITY_LOWEST, 0, CREATE_SUSPENDED);
				if (m_AllocateThread == NULL)
				{
					TRACE(_T("Failed to create alloc thread! -> allocate blocking\n"));
					forcewait=true;
				} else {
					m_iAllocinfo = newsize;
					m_AllocateThread->ResumeThread();
					delete[] changedPart;
					return;
				}
			}
			
			if (forcewait) {
				bIncreasedFile=true;
				// If this is a NTFS compressed file and the current block is the 1st one to be written and there is not 
				// enough free disk space to hold the entire *uncompressed* file, windows throws a 'diskFull'!?
				if (IsNormalFile())
					m_hpartfile.SetLength(newsize); // allocate disk space (may throw 'diskFull')
			}
		}

		// Loop through queue
		uint64 previouspos = (uint64)-1; //MORPH - Optimization
		for (int i = m_BufferedData_list.GetCount(); i>0; i--)
		{
			// Get top item
			item = m_BufferedData_list.GetHead();
			//MORPH - Flush Thread
			if (item->data == NULL) {
				m_BufferedData_list.RemoveHead();
				delete item;
				continue;
			}
			//MORPH - Flush Thread
			// This is needed a few times
			uint32 lenData = (uint32)(item->end - item->start + 1);

			// SLUGFILLER: SafeHash - could be more than one part
			for (uint32 curpart = (uint32)(item->start/PARTSIZE); curpart <= item->end/PARTSIZE; curpart++)
				changedPart[curpart] = true;
			// SLUGFILLER: SafeHash

			// Go to the correct position in file and write block of data
			
			if (previouspos != item->start) //MORPH - Optimization
				m_hpartfile.Seek(item->start, CFile::begin);
			m_hpartfile.Write(item->data, lenData);
			previouspos = item->end + 1; //MORPH - Optimization
			
			// Remove item from queue
			m_BufferedData_list.RemoveHead();

			// Decrease buffer size
			m_nTotalBufferData -= lenData;

			// Release memory used by this item
			delete [] item->data;
			delete item;
		}

		// Partfile should never be too large
 		if (m_hpartfile.GetLength() > m_nFileSize){
			// it's "last chance" correction. the real bugfix has to be applied 'somewhere' else
			TRACE(_T("Partfile \"%s\" is too large! Truncating %I64u bytes.\n"), GetFileName(), m_hpartfile.GetLength() - m_nFileSize);
			m_hpartfile.SetLength(m_nFileSize);
		}

		// Flush to disk
		m_hpartfile.Flush();

		m_FlushSetting->bIncreasedFile = bIncreasedFile;
		FlushDone();
	}
	catch (CFileException* error)
	{
		FlushBuffersExceptionHandler(error);	
		delete[] changedPart;
		if (m_FlushSetting) {
			delete m_FlushSetting;
			m_FlushSetting = NULL;
		}
	}
	catch(...)
	{
		FlushBuffersExceptionHandler();
		delete[] changedPart;
		if (m_FlushSetting) {
			delete m_FlushSetting;
			m_FlushSetting = NULL;
		}
	}
}

void CPartFile::FlushBuffersExceptionHandler(CFileException* error)
{
	if (error->m_cause == CFileException::diskFull)
	{
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_OUTOFSPACE), GetFileName());
		if (theApp.emuledlg->IsRunning() && thePrefs.GetNotifierOnImportantError()){
			CString msg;
			msg.Format(GetResString(IDS_ERR_OUTOFSPACE), GetFileName());
			theApp.emuledlg->ShowNotifier(msg, TBN_IMPORTANTEVENT);
		}

		// 'CFileException::diskFull' is also used for 'not enough min. free space'
		if (theApp.emuledlg->IsRunning())
		{
			if (thePrefs.GetMinFreeDiskSpace()==0)
				theApp.downloadqueue->CheckDiskspace(true);
			else
				PauseFile(true/*bInsufficient*/);
		}
	}
	else
	{
		if (thePrefs.IsErrorBeepEnabled())
			Beep(800,200);

		if (error->m_cause == CFileException::diskFull) 
		{
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_OUTOFSPACE), GetFileName());
			// may be called during shutdown!
			if (theApp.emuledlg->IsRunning() && thePrefs.GetNotifierOnImportantError()){
				CString msg;
				msg.Format(GetResString(IDS_ERR_OUTOFSPACE), GetFileName());
				theApp.emuledlg->ShowNotifier(msg, TBN_IMPORTANTEVENT);
			}
		}
		else
		{
			TCHAR buffer[MAX_CFEXP_ERRORMSG];
			error->GetErrorMessage(buffer,ARRSIZE(buffer));
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_WRITEERROR), GetFileName(), buffer);
			SetStatus(PS_ERROR);
		}
		paused = true;
		m_iLastPausePurge = time(NULL);
		theApp.downloadqueue->RemoveLocalServerRequest(this);
		datarate = 0;
		m_anStates[DS_DOWNLOADING] = 0;
	}

	if (theApp.emuledlg->IsRunning()) // may be called during shutdown!
		UpdateDisplayedInfo();

	error->Delete();
}

void CPartFile::FlushBuffersExceptionHandler()
{
	ASSERT(0);
	LogError(LOG_STATUSBAR, GetResString(IDS_ERR_WRITEERROR), GetFileName(), GetResString(IDS_UNKNOWN));
	SetStatus(PS_ERROR);
	paused = true;
	m_iLastPausePurge = time(NULL);
	theApp.downloadqueue->RemoveLocalServerRequest(this);
	datarate = 0;
	m_anStates[DS_DOWNLOADING] = 0;
	if (theApp.emuledlg->IsRunning()) // may be called during shutdown!
		UpdateDisplayedInfo();
}

UINT AFX_CDECL CPartFile::AllocateSpaceThread(LPVOID lpParam)
{
	DbgSetThreadName("Partfile-Allocate Space");
	//InitThreadLocale(); //Performance killer

	// Anis: SafeHash
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return 0;
	// Anis: SafeHash

	CPartFile* myfile=(CPartFile*)lpParam;
	theApp.QueueDebugLogLine(false,_T("ALLOC:Start (%s) (%s)"),myfile->GetFileName(), CastItoXBytes(myfile->m_iAllocinfo, false, false) );

	try{
		// If this is a NTFS compressed file and the current block is the 1st one to be written and there is not 
		// enough free disk space to hold the entire *uncompressed* file, windows throws a 'diskFull'!?
		myfile->m_hpartfile.SetLength(myfile->m_iAllocinfo); // allocate disk space (may throw 'diskFull')

		// force the alloc, by temporary writing a non zero to the fileend
		byte x=255;
		myfile->m_hpartfile.Seek(-1,CFile::end);
		myfile->m_hpartfile.Write(&x,1);
		myfile->m_hpartfile.Flush();
		x=0;
		myfile->m_hpartfile.Seek(-1,CFile::end);
		myfile->m_hpartfile.Write(&x,1);
		myfile->m_hpartfile.Flush();
	}
	catch (CFileException* error)
	{
		VERIFY( PostMessage(theApp.emuledlg->m_hWnd,TM_FILEALLOCEXC,(WPARAM)myfile,(LPARAM)error) );
		myfile->m_AllocateThread=NULL;

		return 1;
	}
#ifndef _DEBUG
	catch(...)
	{

		VERIFY( PostMessage(theApp.emuledlg->m_hWnd,TM_FILEALLOCEXC,(WPARAM)myfile,0) );
		myfile->m_AllocateThread=NULL;
		return 2;
	}
#endif

	myfile->m_AllocateThread=NULL;
	theApp.QueueDebugLogLine(false,_T("ALLOC:End (%s)"),myfile->GetFileName());
	return 0;
}

// Barry - This will invert the gap list, up to caller to delete gaps when done
// 'Gaps' returned are really the filled areas, and guaranteed to be in order
void CPartFile::GetFilledList(CTypedPtrList<CPtrList, Gap_Struct*> *filled) const
{
	if (gaplist.GetHeadPosition() == NULL)
		return;

	Gap_Struct *gap=NULL;
	Gap_Struct *best=NULL;
	POSITION pos;
	uint64 start = 0;
	uint64 bestEnd = 0;

	// Loop until done
	bool finished = false;
	while (!finished)
	{
		finished = true;
		// Find first gap after current start pos
		bestEnd = m_nFileSize;
		pos = gaplist.GetHeadPosition();
		while (pos != NULL)
		{
			gap = gaplist.GetNext(pos);
			if ( (gap->start >= start) && (gap->end < bestEnd))
			{
				best = gap;
				bestEnd = best->end;
				finished = false;
			}
		}

		// TODO: here we have a problem - it occured that eMule crashed because of "best==NULL" while
		// recovering an archive which was currently in "completing" state...
		if (best==NULL){
			ASSERT(0);
			return;
		}

		if (!finished)
		{
			if (best->start>0) {
			// Invert this gap
			gap = new Gap_Struct;
			gap->start = start;
			gap->end = best->start - 1;
				filled->AddTail(gap);
			start = best->end + 1;
			} else 				
				start = best->end + 1;

		}
		else if (best->end+1 < m_nFileSize)
		{
			gap = new Gap_Struct;
			gap->start = best->end + 1;
			gap->end = m_nFileSize;
			filled->AddTail(gap);
		}
	}
}

void CPartFile::UpdateFileRatingCommentAvail(bool bForceUpdate)
{
	bool bOldHasComment = m_bHasComment;
	UINT uOldUserRatings = m_uUserRating;

	m_bHasComment = false;
	UINT uRatings = 0;
	UINT uUserRatings = 0;

	for (POSITION pos = srclist.GetHeadPosition(); pos != NULL;)
	{
		const CUpDownClient* cur_src = srclist.GetNext(pos);
		if (!m_bHasComment && cur_src->HasFileComment())
			m_bHasComment = true;
		if (cur_src->HasFileRating())
		{
			uRatings++;
			uUserRatings += cur_src->GetFileRating();
		}
	}
	for(POSITION pos = m_kadNotes.GetHeadPosition(); pos != NULL; )
	{
		Kademlia::CEntry* entry = m_kadNotes.GetNext(pos);
		if (!m_bHasComment && !entry->GetStrTagValue(TAG_DESCRIPTION).IsEmpty())
			m_bHasComment = true;
		UINT rating = (UINT)entry->GetIntTagValue(TAG_FILERATING);
		if (rating != 0)
		{
			uRatings++;
			uUserRatings += rating;
		}
	}

	if (uRatings)
		m_uUserRating = (uint32)ROUND((float)uUserRatings / uRatings);
	else
		m_uUserRating = 0;

	if (bOldHasComment != m_bHasComment || uOldUserRatings != m_uUserRating || bForceUpdate)
		UpdateDisplayedInfo(true);
}

void CPartFile::UpdateDisplayedInfo(bool /*force*/)
{   //Anis -> ottimizzazione
	if (theApp.emuledlg->IsRunning())
		theApp.emuledlg->transferwnd->GetDownloadList()->UpdateItem(this);
}

void CPartFile::UpdateAutoDownPriority()
{
	if( !IsAutoDownPriority() || GetStatus()==PS_PAUSED || IsStopped() )
		return;
	UINT nHighestSC = theApp.downloadqueue->GetHighestAvailableSourceCount();
	if ( GetAvailableSrcCount() > (nHighestSC * .40) ) {
		SetDownPriority( PR_LOW );
		return;
	}
	if ( GetAvailableSrcCount() > (nHighestSC * .20) ) {
		SetDownPriority( PR_NORMAL );
		return;
	}
	SetDownPriority( PR_HIGH );
}

UINT CPartFile::GetCategory() /*const*/
{
	return m_category > (UINT)(thePrefs.GetCatCount() - 1)?0:m_category;
}

bool CPartFile::HasDefaultCategory() const // extra function for const 
{
	return m_category == 0 || m_category > (UINT)(thePrefs.GetCatCount() - 1);
}

// Ornis: Creating progressive presentation of the partfilestatuses - for webdisplay
CString CPartFile::GetProgressString(uint16 size) const
{
	char crProgress = '0';//green
	char crHave = '1';	// black
	char crPending='2';	// yellow
	char crMissing='3';  // red
	
	char crWaiting[6];
	crWaiting[0]='4'; // blue few source
	crWaiting[1]='5';
	crWaiting[2]='6';
	crWaiting[3]='7';
	crWaiting[4]='8';
	crWaiting[5]='9'; // full sources

	CString my_ChunkBar;
	for (uint16 i=0;i<=size+1;i++) my_ChunkBar.AppendChar(crHave);	// one more for safety

	float unit= (float)size/(float)m_nFileSize;

	if(GetStatus() == PS_COMPLETE || GetStatus() == PS_COMPLETING) {
		CharFillRange(&my_ChunkBar,0,(uint32)((uint64)m_nFileSize*unit), crProgress);
	} else
	    // red gaps
	    for (POSITION pos = gaplist.GetHeadPosition();pos !=  0;){
		    Gap_Struct* cur_gap = gaplist.GetNext(pos);
		    bool gapdone = false;
		    uint64 gapstart = cur_gap->start;
		    uint64 gapend = cur_gap->end;
		    for (UINT i = 0; i < GetPartCount(); i++){
			    if (gapstart >= (uint64)i*PARTSIZE && gapstart <=  (uint64)(i+1)*PARTSIZE){ // is in this part?
				    if (gapend <= (uint64)(i+1)*PARTSIZE)
					    gapdone = true;
				    else{
					    gapend = (uint64)(i+1)*PARTSIZE; // and next part
				    }
				    // paint
				    uint8 color;
				    if (m_SrcpartFrequency.GetCount() >= (INT_PTR)i && m_SrcpartFrequency[(uint16)i])  // frequency?
					    //color = crWaiting;
					    color = m_SrcpartFrequency[(uint16)i] <  10 ? crWaiting[m_SrcpartFrequency[(uint16)i]/2]:crWaiting[5];
				    else
					    color = crMissing;
    
				    CharFillRange(&my_ChunkBar,(uint32)(gapstart*unit), (uint32)(gapend*unit + 1),  color);
    
				    if (gapdone) // finished?
					    break;
				    else{
					    gapstart = gapend;
					    gapend = cur_gap->end;
				    }
			    }
		    }
	    }

	// yellow pending parts
	for (POSITION pos = requestedblocks_list.GetHeadPosition();pos !=  0;){
		Requested_Block_Struct* block =  requestedblocks_list.GetNext(pos);
		CharFillRange(&my_ChunkBar, (uint32)((block->StartOffset + block->transferred)*unit), (uint32)(block->EndOffset*unit),  crPending);
	}

	return my_ChunkBar;
}

void CPartFile::CharFillRange(CString* buffer, uint32 start, uint32 end, char color) const
{
	for (uint32 i = start; i <= end;i++)
		buffer->SetAt(i, color);
}

void CPartFile::SetCategory(UINT cat)
{
	m_category=cat;
	
// ZZ:DownloadManager -->
	// set new prio
	if (IsPartFile()){
		SavePartFile();
	}
// <-- ZZ:DownloadManager
}

void CPartFile::_SetStatus(EPartFileStatus eStatus)
{
	// NOTE: This function is meant to be used from *different* threads -> Do *NOT* call
	// any GUI functions from within here!!
	ASSERT( eStatus != PS_PAUSED && eStatus != PS_INSUFFICIENT );
	status = eStatus;
}

void CPartFile::SetStatus(EPartFileStatus eStatus)
{
	_SetStatus(eStatus);
	if (theApp.emuledlg->IsRunning())
	{
		NotifyStatusChange();
		UpdateDisplayedInfo(true);
		if (thePrefs.ShowCatTabInfos())
			theApp.emuledlg->transferwnd->UpdateCatTabTitles();
	}
}

void CPartFile::NotifyStatusChange()
{
	if (theApp.emuledlg->IsRunning())
		theApp.emuledlg->transferwnd->GetDownloadList()->UpdateCurrentCategoryView(this);
}

EMFileSize CPartFile::GetRealFileSize() const
{
	return ::GetDiskFileSize(GetFilePath());
}

uint8* CPartFile::MMCreatePartStatus(){
	// create partstatus + info in mobilemule protocol specs
	// result needs to be deleted[] | slow, but not timecritical
	uint8* result = new uint8[GetPartCount()+1];
	for (UINT i = 0; i < GetPartCount(); i++){
		result[i] = 0;
		if (IsComplete((uint64)i*PARTSIZE,((uint64)(i+1)*PARTSIZE)-1, false)){
			result[i] = 1;
			continue;
		}
		else{
			if (IsComplete((uint64)i*PARTSIZE + (0*(PARTSIZE/3)), (((uint64)i*PARTSIZE)+(1*(PARTSIZE/3)))-1, false))
				result[i] += 2;
			if (IsComplete((uint64)i*PARTSIZE+ (1*(PARTSIZE/3)), (((uint64)i*PARTSIZE)+(2*(PARTSIZE/3)))-1, false))
				result[i] += 4;
			if (IsComplete((uint64)i*PARTSIZE+ (2*(PARTSIZE/3)), (((uint64)i*PARTSIZE)+(3*(PARTSIZE/3)))-1, false))
				result[i] += 8;
			uint8 freq;
			if (m_SrcpartFrequency.GetCount() > (signed)i)
				freq = (uint8)m_SrcpartFrequency[i];
			else
				freq = 0;

			if (freq > 44)
				freq = 44;
			freq = (uint8)ceilf((float)freq/3);
			freq = (uint8)(freq << 4);
			result[i] = (uint8)(result[i] + freq);
		}

	}
	return result;
};

UINT CPartFile::GetSrcStatisticsValue(EDownloadState nDLState) const
{
	ASSERT( nDLState < ARRSIZE(m_anStates) );
	return m_anStates[nDLState];
}

UINT CPartFile::GetTransferringSrcCount() const
{
	return GetSrcStatisticsValue(DS_DOWNLOADING);
}

// [Maella -Enhanced Chunk Selection- (based on jicxicmic)]

#pragma pack(1)
struct Chunk {
	uint16 part;			// Index of the chunk
	union {
		uint16 frequency;	// Availability of the chunk
		uint16 rank;		// Download priority factor (highest = 0, lowest = 0xffff)
	};
};
#pragma pack()

CString CPartFile::GetInfoSummary(bool bNoFormatCommands) const
{
	if (!IsPartFile())
		return CKnownFile::GetInfoSummary();

	CString Sbuffer, lsc, compl, buffer, lastdwl;

	lsc.Format(_T("%s"), CastItoXBytes(GetCompletedSize(), false, false));
	compl.Format(_T("%s"), CastItoXBytes(GetFileSize(), false, false));
	buffer.Format(_T("%s/%s"), lsc, compl);
	compl.Format(_T("%s: %s (%.1f%%)\n"), GetResString(IDS_DL_TRANSFCOMPL), buffer, GetPercentCompleted());

	if (lastseencomplete == NULL)
		lsc.Format(_T("%s"), GetResString(IDS_NEVER));
	else
		lsc.Format(_T("%s"), lastseencomplete.Format(thePrefs.GetDateTimeFormat()));

	float availability = 0.0F;
	if (GetPartCount() != 0)
		availability = (float)(GetAvailablePartCount() * 100.0 / GetPartCount());
	
	CString avail;
	avail.Format(GetResString(IDS_AVAIL), GetPartCount(), GetAvailablePartCount(), availability);

	if (GetCFileDate() != NULL)
		lastdwl.Format(_T("%s"), GetCFileDate().Format(thePrefs.GetDateTimeFormat()));
	else
		lastdwl = GetResString(IDS_NEVER);
	
	CString sourcesinfo;
	sourcesinfo.Format(GetResString(IDS_DL_SOURCES) + _T(": ") + GetResString(IDS_SOURCESINFO) + _T('\n'), GetSourceCount(), GetValidSourcesCount(), GetSrcStatisticsValue(DS_NONEEDEDPARTS), GetSrcA4AFCount());
		
	// always show space on disk
	CString sod = _T("  (") + GetResString(IDS_ONDISK) + CastItoXBytes(GetRealFileSize(), false, false) + _T(")");

	CString status;
	if (GetTransferringSrcCount() > 0)
		status.Format(GetResString(IDS_PARTINFOS2) + _T("\n"), GetTransferringSrcCount());
	else 
		status.Format(_T("%s\n"), getPartfileStatus());

	CString strHeadFormatCommand = bNoFormatCommands ? _T("") : _T("<br_head>");
	CString info;
	info.Format(_T("%s\n")
		+ GetResString(IDS_FD_HASH) + _T(" %s\n")
		+ GetResString(IDS_FD_SIZE) + _T(" %s  %s\n") + strHeadFormatCommand + _T("\n")
		+ GetResString(IDS_FD_MET)+ _T(" %s\n")
		+ GetResString(IDS_STATUS) + _T(": ") + status
		+ _T("%s")
		+ sourcesinfo
		+ _T("%s")
		+ GetResString(IDS_LASTSEENCOMPL) + _T(' ') + lsc + _T('\n')
		+ GetResString(IDS_FD_LASTCHANGE) + _T(' ') + lastdwl,
		GetFileName(),
		md4str(GetFileHash()),
		CastItoXBytes(GetFileSize(), false, false),	sod,
		GetPartMetFileName(),
		compl,
		avail);
	return info;
}

bool CPartFile::GrabImage(uint8 nFramesToGrab, double dStartTime, bool bReduceColor, uint16 nMaxWidth, void* pSender)
{
	if (!IsPartFile()){
		return CKnownFile::GrabImage(GetPath() + CString(_T("\\")) + GetFileName(),nFramesToGrab, dStartTime, bReduceColor, nMaxWidth, pSender);
	}
	else{
		if ( ((GetStatus() != PS_READY && GetStatus() != PS_PAUSED) || m_bPreviewing || GetPartCount() < 2 || !IsComplete(0,PARTSIZE-1, true))  )
			return false;
		CString strFileName = RemoveFileExtension(GetFullName());
		if (m_FileCompleteMutex.Lock(100)){
			m_bPreviewing = true; 
			try{
				if (m_hpartfile.m_hFile != INVALID_HANDLE_VALUE){
					m_hpartfile.Close();
				}
			}
			catch(CFileException* exception){
				exception->Delete();
				m_FileCompleteMutex.Unlock();
				m_bPreviewing = false; 
				return false;
			}
		}
		else
			return false;

		return CKnownFile::GrabImage(strFileName,nFramesToGrab, dStartTime, bReduceColor, nMaxWidth, pSender);
	}
}

void CPartFile::GrabbingFinished(CxImage** imgResults, uint8 nFramesGrabbed, void* pSender)
{
	// unlock and reopen the file
	if (IsPartFile()){
		CString strFileName = RemoveFileExtension(GetFullName());
		if (!m_hpartfile.Open(strFileName, CFile::modeReadWrite|CFile::shareDenyWrite|CFile::osSequentialScan)){
			// uhuh, that's really bad
			LogError(LOG_STATUSBAR, GetResString(IDS_FAILEDREOPEN), RemoveFileExtension(GetPartMetFileName()), GetFileName());
			SetStatus(PS_ERROR);
			StopFile();
		}
		m_bPreviewing = false;
		m_FileCompleteMutex.Unlock();
		// continue processing
	}
	CKnownFile::GrabbingFinished(imgResults, nFramesGrabbed, pSender);
}

void CPartFile::GetLeftToTransferAndAdditionalNeededSpace(uint64 &rui64LeftToTransfer, 
														  uint64 &rui64AdditionalNeededSpace) const
{
	uint64 uSizeLastGap = 0;
	for (POSITION pos = gaplist.GetHeadPosition(); pos != 0; )
	{
		const Gap_Struct* cur_gap = gaplist.GetNext(pos);
		uint64 uGapSize = cur_gap->end - cur_gap->start + 1;
		rui64LeftToTransfer += uGapSize;
		if (cur_gap->end == GetFileSize() - (uint64)1)
			uSizeLastGap = uGapSize;
	}

	if (IsNormalFile())
	{
		// File is not NTFS-Compressed nor NTFS-Sparse
		if (GetFileSize() == GetRealFileSize()) // already fully allocated?
			rui64AdditionalNeededSpace = 0;
		else
			rui64AdditionalNeededSpace = uSizeLastGap;
	}
	else
	{
		// File is NTFS-Compressed or NTFS-Sparse
		rui64AdditionalNeededSpace = rui64LeftToTransfer;
	}
}

void CPartFile::SetLastAnsweredTimeTimeout()
{
	m_ClientSrcAnswered = 2 * CONNECTION_LATENCY + ::GetTickCount() - SOURCECLIENTREASKS;
}

/*Checks, if a given item should be shown in a given category
AllcatTypes:
	0	all
	1	all not assigned
	2	not completed
	3	completed
	4	waiting
	5	transferring
	6	errorous
	7	paused
	8	stopped
	10	Video
	11	Audio
	12	Archive
	13	CDImage
	14  Doc
	15  Pic
	16  Program
*/
bool CPartFile::CheckShowItemInGivenCat(int inCategory) /*const*/
{
	int myfilter=thePrefs.GetCatFilter(inCategory);

	// common cases
	if (inCategory>=thePrefs.GetCatCount())
		return false;
	if (((UINT)inCategory == GetCategory() && myfilter == 0))
		return true;
	if (inCategory>0 && GetCategory()!=(UINT)inCategory && !thePrefs.GetCategory(inCategory)->care4all )
		return false;


	bool ret=true;
	if ( myfilter > 0)
	{
		if (myfilter>=4 && myfilter<=8 && !IsPartFile())
			ret=false;
		else switch (myfilter)
		{
			case 1 : ret=(GetCategory() == 0);break;
			case 2 : ret= (IsPartFile());break;
			case 3 : ret= (!IsPartFile());break;
			case 4 : ret= ((GetStatus()==PS_READY || GetStatus()==PS_EMPTY) && GetTransferringSrcCount()==0);break;
			case 5 : ret= ((GetStatus()==PS_READY || GetStatus()==PS_EMPTY) && GetTransferringSrcCount()>0);break;
			case 6 : ret= (GetStatus()==PS_ERROR);break;
			case 7 : ret= (GetStatus()==PS_PAUSED || IsStopped() );break;
			case 8 : ret=  lastseencomplete!=NULL ;break;
			case 10 : ret= IsMovie();break;
			case 11 : ret= (ED2KFT_AUDIO == GetED2KFileTypeID(GetFileName()));break;
			case 12 : ret= IsArchive();break;
			case 13 : ret= (ED2KFT_CDIMAGE == GetED2KFileTypeID(GetFileName()));break;
			case 14 : ret= (ED2KFT_DOCUMENT == GetED2KFileTypeID(GetFileName()));break;
			case 15 : ret= (ED2KFT_IMAGE == GetED2KFileTypeID(GetFileName()));break;
			case 16 : ret= (ED2KFT_PROGRAM == GetED2KFileTypeID(GetFileName()));break;
			case 18 : ret= RegularExpressionMatch(thePrefs.GetCategory(inCategory)->regexp ,GetFileName());break;
			case 20 : ret= (ED2KFT_EMULECOLLECTION == GetED2KFileTypeID(GetFileName()));break;
		}
	}

	return (thePrefs.GetCatFilterNeg(inCategory))?!ret:ret;
}



void CPartFile::SetFileName(LPCTSTR pszFileName, bool bReplaceInvalidFileSystemChars, bool bRemoveControlChars)
{
	CKnownFile::SetFileName(pszFileName, bReplaceInvalidFileSystemChars, bRemoveControlChars);

	UpdateDisplayedInfo(true);
	theApp.emuledlg->transferwnd->GetDownloadList()->UpdateCurrentCategoryView(this);
}

void CPartFile::SetActive(bool bActive)
{
	time_t tNow = time(NULL);
	if (bActive)
	{
		if (theApp.IsConnected())
		{
			if (m_tActivated == 0)
				m_tActivated = tNow;
		}
	}
	else
	{
		if (m_tActivated != 0)
		{
			m_nDlActiveTime += tNow - m_tActivated;
			m_tActivated = 0;
		}
	}
}

uint32 CPartFile::GetDlActiveTime() const
{
	uint32 nDlActiveTime = m_nDlActiveTime;
	if (m_tActivated != 0)
		nDlActiveTime += time(NULL) - m_tActivated;
	return nDlActiveTime;
}

void CPartFile::SetFileOp(EPartFileOp eFileOp)
{
	m_eFileOp = eFileOp;
}

void CPartFile::SetFileOpProgress(UINT uProgress)
{
	ASSERT( uProgress <= 100 );
	m_uFileOpProgress = uProgress;
}

bool CPartFile::RightFileHasHigherPrio(CPartFile* left, CPartFile* right)
{
    if(!right) {
        return false;
    }

    if(!left ||
       thePrefs.GetCategory(right->GetCategory())->prio > thePrefs.GetCategory(left->GetCategory())->prio ||
       thePrefs.GetCategory(right->GetCategory())->prio == thePrefs.GetCategory(left->GetCategory())->prio &&
       (
           right->GetDownPriority() > left->GetDownPriority() ||
           right->GetDownPriority() == left->GetDownPriority() &&
           (
               right->GetCategory() == left->GetCategory() && right->GetCategory() != 0 &&
               (thePrefs.GetCategory(right->GetCategory())->downloadInAlphabeticalOrder && thePrefs.IsExtControlsEnabled()) && 
               right->GetFileName() && left->GetFileName() &&
               right->GetFileName().CompareNoCase(left->GetFileName()) < 0
           )
       )
    ) {
        return true;
    } else {
        return false;
    }
}

void CPartFile::RequestAICHRecovery(UINT nPart)
{
	if (!m_pAICHRecoveryHashSet->HasValidMasterHash() || (m_pAICHRecoveryHashSet->GetStatus() != AICH_TRUSTED && m_pAICHRecoveryHashSet->GetStatus() != AICH_VERIFIED)){
		AddDebugLogLine(DLP_DEFAULT, false, _T("Unable to request AICH Recoverydata because we have no trusted Masterhash"));
		return;
	}
	if (GetFileSize() <= (uint64)EMBLOCKSIZE || GetFileSize() - PARTSIZE*(uint64)nPart <= (uint64)EMBLOCKSIZE)
		return;
	if (CAICHRecoveryHashSet::IsClientRequestPending(this, (uint16)nPart)){
		AddDebugLogLine(DLP_DEFAULT, false, _T("RequestAICHRecovery: Already a request for this part pending"));
		return;
	}

	// first check if we have already the recoverydata, no need to rerequest it then
	if (m_pAICHRecoveryHashSet->IsPartDataAvailable((uint64)nPart*PARTSIZE)){
		AddDebugLogLine(DLP_DEFAULT, false, _T("Found PartRecoveryData in memory"));
		AICHRecoveryDataAvailable(nPart);
		return;
	}

	ASSERT( nPart < GetPartCount() );
	// find some random client which support AICH to ask for the blocks
	// first lets see how many we have at all, we prefer high id very much
	uint32 cAICHClients = 0;
	uint32 cAICHLowIDClients = 0;
	for (POSITION pos = srclist.GetHeadPosition(); pos != NULL;){
		CUpDownClient* pCurClient = srclist.GetNext(pos);
		if (pCurClient->IsSupportingAICH() && pCurClient->GetReqFileAICHHash() != NULL && !pCurClient->IsAICHReqPending()
			&& (*pCurClient->GetReqFileAICHHash()) == m_pAICHRecoveryHashSet->GetMasterHash())
		{
			if (pCurClient->HasLowID())
				cAICHLowIDClients++;
			else
				cAICHClients++;
		}
	}
	if ((cAICHClients | cAICHLowIDClients) == 0){
		AddDebugLogLine(DLP_DEFAULT, false, _T("Unable to request AICH Recoverydata because found no client who supports it and has the same hash as the trusted one"));
		return;
	}
	uint32 nSeclectedClient;
	if (cAICHClients > 0)
		nSeclectedClient = (rand() % cAICHClients) + 1;
	else
		nSeclectedClient = (rand() % cAICHLowIDClients) + 1;
	
	CUpDownClient* pClient = NULL;
	for (POSITION pos = srclist.GetHeadPosition(); pos != NULL;){
		CUpDownClient* pCurClient = srclist.GetNext(pos);
		if (pCurClient->IsSupportingAICH() && pCurClient->GetReqFileAICHHash() != NULL && !pCurClient->IsAICHReqPending()
			&& (*pCurClient->GetReqFileAICHHash()) == m_pAICHRecoveryHashSet->GetMasterHash())
		{
			if (cAICHClients > 0){
				if (!pCurClient->HasLowID())
					nSeclectedClient--;
			}
			else{
				ASSERT( pCurClient->HasLowID());
				nSeclectedClient--;
			}
			if (nSeclectedClient == 0){
				pClient = pCurClient;
				break;
			}
		}
	}
	if (pClient == NULL){
		ASSERT( false );
		return;
	}
	AddDebugLogLine(DLP_DEFAULT, false, _T("Requesting AICH Hash (%s) from client %s"),cAICHClients? _T("HighId"):_T("LowID"), pClient->DbgGetClientInfo());
	pClient->SendAICHRequest(this, (uint16)nPart);
}

void CPartFile::AICHRecoveryDataAvailable(UINT nPart)
{
	if (GetPartCount() < nPart){
		ASSERT( false );
		return;
	}
	FlushBuffer(true, true, true);
	uint32 length = PARTSIZE;
	if ((ULONGLONG)PARTSIZE*(uint64)(nPart+1) > m_hpartfile.GetLength()){
		length = (UINT)(m_hpartfile.GetLength() - ((ULONGLONG)PARTSIZE*(uint64)nPart));
		ASSERT( length <= PARTSIZE );
	}	
	// if the part was already ok, it would now be complete
	if (IsComplete((uint64)nPart*PARTSIZE, (((uint64)nPart*PARTSIZE)+length)-1, true)){
		return;
	}
	


	CAICHHashTree* pVerifiedHash = m_pAICHRecoveryHashSet->m_pHashTree.FindHash((uint64)nPart*PARTSIZE, length);
	if (pVerifiedHash == NULL || !pVerifiedHash->m_bHashValid){
		return;
	}
	CAICHHashTree htOurHash(pVerifiedHash->m_nDataSize, pVerifiedHash->m_bIsLeftBranch, pVerifiedHash->GetBaseSize());
	try{
		m_hpartfile.Seek((LONGLONG)PARTSIZE*(uint64)nPart,0);
		CreateHash(&m_hpartfile,length, NULL, &htOurHash);
	}
	catch(...){
		ASSERT( false );
		return;
	}

	if (!htOurHash.m_bHashValid){
		return;
	}

	// now compare the hash we just did, to the verified hash and readd all blocks which are ok
	uint32 nRecovered = 0;
	for (uint32 pos = 0; pos < length; pos += EMBLOCKSIZE){
		const uint32 nBlockSize = min(EMBLOCKSIZE, length - pos);
		CAICHHashTree* pVerifiedBlock = pVerifiedHash->FindHash(pos, nBlockSize);
		CAICHHashTree* pOurBlock = htOurHash.FindHash(pos, nBlockSize);
		if ( pVerifiedBlock == NULL || pOurBlock == NULL || !pVerifiedBlock->m_bHashValid || !pOurBlock->m_bHashValid){
			ASSERT( false );
			continue;
		}
		if (pOurBlock->m_Hash == pVerifiedBlock->m_Hash){
			FillGap(PARTSIZE*(uint64)nPart+pos, PARTSIZE*(uint64)nPart + pos + (nBlockSize-1));
			RemoveBlockFromList(PARTSIZE*(uint64)nPart+pos, PARTSIZE*(uint64)nPart + pos + (nBlockSize-1));
			nRecovered += nBlockSize;
			// tell the blackbox about the verified data
			m_CorruptionBlackBox.VerifiedData(PARTSIZE*(uint64)nPart+pos, PARTSIZE*(uint64)nPart + pos + (nBlockSize-1));
		}
		else{
			// inform our "blackbox" about the corrupted block which may ban clients who sent it
			m_CorruptionBlackBox.CorruptedData(PARTSIZE*(uint64)nPart+pos, PARTSIZE*(uint64)nPart + pos + (nBlockSize-1));
		}
	}
	m_CorruptionBlackBox.EvaluateData((uint16)nPart);
	
	if (m_uCorruptionLoss >= nRecovered)
		m_uCorruptionLoss -= nRecovered;
	if (thePrefs.sesLostFromCorruption >= nRecovered)
		thePrefs.sesLostFromCorruption -= nRecovered;


	// ok now some sanity checks
	if (IsComplete((uint64)nPart*PARTSIZE, (((uint64)nPart*PARTSIZE)+length)-1, true)){
		// this is a bad, but it could probably happen under some rare circumstances
		// make sure that HashSinglePart() (MD4 and possibly AICH again) agrees to this fact too, for Verified Hashes problems are handled within that functions, otherwise:
		// Anis: SafeHash - In another thread
		m_PartsHashing++;
		CPartHashThread* parthashthread = (CPartHashThread*) AfxBeginThread(RUNTIME_CLASS(CPartHashThread), THREAD_PRIORITY_BELOW_NORMAL,0, CREATE_SUSPENDED);
		parthashthread->SetSinglePartHash(this, nPart, false, true);
		parthashthread->ResumeThread();
		// Anis: SafeHash
	} // end sanity check
	// Update met file
	SavePartFile();
	// make sure the user appreciates our great recovering work :P
	AddLogLine(true, GetResString(IDS_AICH_WORKED), CastItoXBytes(nRecovered), CastItoXBytes(length), nPart, GetFileName());
	//AICH successfully recovered %s of %s from part %u for %s
}

UINT CPartFile::GetMaxSources() const
{
	// Ignore any specified 'max sources' value if not in 'extended mode' -> don't use a parameter which was once
	// specified in GUI but can not be seen/modified any longer..
	return (!thePrefs.IsExtControlsEnabled() || m_uMaxSources == 0) ? thePrefs.GetMaxSourcePerFileDefault() : m_uMaxSources;
}

UINT CPartFile::GetMaxSourcePerFileSoft() const
{
	UINT temp = ((UINT)GetMaxSources() * 9L) / 10;
 	if (temp > theApp.rm->maxSrc)
 		return theApp.rm->maxSrc;
	return temp;
}

UINT CPartFile::GetMaxSourcePerFileUDP() const
{	
	UINT temp = ((UINT)GetMaxSources() * 3L) / 4;
 	if (temp > theApp.rm->maxSrcUdp)
 		return theApp.rm->maxSrcUdp;
	return temp;
}

CString CPartFile::GetTempPath() const
{
	return m_fullname.Left(m_fullname.ReverseFind(_T('\\'))+1);
}

void CPartFile::RefilterFileComments(){
	// check all availabe comments against our filter again
	if (thePrefs.GetCommentFilter().IsEmpty())
		return;
	for (POSITION pos = srclist.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* cur_src = srclist.GetNext(pos);
		if (cur_src->HasFileComment())
		{
			CString strCommentLower(cur_src->GetFileComment());
			strCommentLower.MakeLower();

			int iPos = 0;
			CString strFilter(thePrefs.GetCommentFilter().Tokenize(_T("|"), iPos));
			while (!strFilter.IsEmpty())
			{
				// comment filters are already in lowercase, compare with temp. lowercased received comment
				if (strCommentLower.Find(strFilter) >= 0)
				{
					cur_src->SetFileComment(_T(""));
					cur_src->SetFileRating(0);
					break;
				}
				strFilter = thePrefs.GetCommentFilter().Tokenize(_T("|"), iPos);
			}		
		}
	}
	RefilterKadNotes();
	UpdateFileRatingCommentAvail();
}

void CPartFile::SetFileSize(EMFileSize nFileSize)
{
	ASSERT( m_pAICHRecoveryHashSet != NULL );
	m_pAICHRecoveryHashSet->SetFileSize(nFileSize);
	CKnownFile::SetFileSize(nFileSize);
}
boolean CPartFile::DropSlowestSource(CUpDownClient* calling_source) {
	
	//if(GetFileSize()-GetCompletedSize()> SESSIONMAXTRANS)

//xpm : + uint64
	if( GetFileSize() - GetCompletedSize() > (uint64)SESSIONMAXTRANS )
//end

		return false;
	uint32 lowestspeed = 2000000000;
	CUpDownClient* slowest_source= NULL;
	
	//xpm : end-game edit
	//for (POSITION pos = m_downloadingSourceList.GetHeadPosition(); pos != NULL;)
	for (POSITION pos = m_downloadingSourceList.GetHeadPosition(); pos != 0;)
	//end

	{
		CUpDownClient* cur_src = srclist.GetNext(pos);
		if (cur_src->GetDownloadDatarate() < lowestspeed) {
			lowestspeed = cur_src->GetDownloadDatarate();
			slowest_source = cur_src;
		}
	}
	// only remove source if it isn't the last one
	if (m_downloadingSourceList.GetCount() > 1 && slowest_source && slowest_source!=calling_source ){
			ASSERT( slowest_source->socket != NULL );
			if (slowest_source->socket != NULL)
			{
				ASSERT( !slowest_source->socket->IsRawDataMode() );
				if (!slowest_source->socket->IsRawDataMode())
					slowest_source->SendCancelTransfer();
			}
			slowest_source->SetDownloadState(DS_ONQUEUE, _T("DaZZle: replacing client for a faster one"));
		return true;
	}
	else
		return false;	
}

uint32 CPartFile::CalculateSocketLimit(uint32 reducedownload, uint32 limit, uint32 cur_datarate)
{
	if(limit<1000 && reducedownload == 200)
		limit +=1000;
	else if(limit<200 && cur_datarate == 0 && reducedownload >= 100)
		limit = 200;
	else if(limit<60 && cur_datarate < 600 && reducedownload >= 97)
		limit = 60;
	else if(limit<20 && cur_datarate < 200 && reducedownload >= 93)
		limit = 20;
	else if(limit<1)
		limit = 1;
	return limit;
}

// NEO: MCM - [ManualClientManagement] -- Xanatos -->
uint16 CPartFile::CollectAllA4AF()
{
	uint16 swaped = 0;
	for (POSITION pos = A4AFsrclist.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* cur_src = A4AFsrclist.GetNext(pos);
		if(cur_src->GetDownloadState() == DS_DOWNLOADING)
			continue;

		cur_src->DoSwap(this, false, _T("Special Swap"),true);
			swaped++;
	}

	return swaped;
}

uint16 CPartFile::ReleaseAllA4AF()
{
	uint16 swaped = 0;
	for (POSITION pos = srclist.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* cur_src = srclist.GetNext(pos);
		if(cur_src->GetDownloadState() == DS_DOWNLOADING)
			continue;

		if(cur_src->SwapToAnotherFile(_T("Swap Special"),false, true, false, NULL))
			swaped++;
	}

	return swaped;
}// NEO: MCM END <-- Xanatos --

UINT CPartFile::GetAvailableSrcCount() const
{
	return m_anStates[DS_ONQUEUE]+m_anStates[DS_DOWNLOADING];
}

void CPartFile::WriteToDisk() { //Called by Flush Thread
	bool bCheckDiskspace = thePrefs.GetMinFreeDiskSpace() > 0;
	ULONGLONG uFreeDiskSpace = bCheckDiskspace ? GetFreeDiskSpaceX(GetTempPath()) : 0;

	// Check free diskspace for compressed/sparse files before possibly increasing the file size
	if (bCheckDiskspace && !IsNormalFile())
	{
		// Compressed/sparse files; regardless whether the file is increased in size, 
		// check the amount of data which will be written
		// would need to use disk cluster sizes for more accuracy	
			if (m_nTotalBufferData + thePrefs.GetMinFreeDiskSpace() >= uFreeDiskSpace)
			AfxThrowFileException(CFileException::diskFull, 0, m_hpartfile.GetFileName());
	}

	// Ensure file is big enough to write data to (the last item will be the furthest from the start)
	m_BufferedData_list_Locker.Lock();
	PartFileBufferedData *item = m_BufferedData_list.GetTail();
	m_BufferedData_list_Locker.Unlock();
	
	if (m_hpartfile.GetLength() <= item->end)
	{
		uint64 newsize=thePrefs.GetAllocCompleteMode()? GetFileSize() : (item->end+1);
		ULONGLONG uIncrease = newsize - m_hpartfile.GetLength();

		// Check free diskspace for normal files before increasing the file size
		if (bCheckDiskspace && IsNormalFile())
		{
			// Normal files; check if increasing the file would reduce the amount of min. free space beyond the limit
			// would need to use disk cluster sizes for more accuracy
			if (uIncrease + thePrefs.GetMinFreeDiskSpace() >= uFreeDiskSpace)
				AfxThrowFileException(CFileException::diskFull, 0, m_hpartfile.GetFileName());
		}

		m_FlushSetting->bIncreasedFile = true;
		// If this is a NTFS compressed file and the current block is the 1st one to be written and there is not 
		// enough free disk space to hold the entire *uncompressed* file, windows throws a 'diskFull'!?
		if (IsNormalFile())
			m_hpartfile.SetLength(newsize); // allocate disk space (may throw 'diskFull')
	}

	// Loop through queue
	uint64 previouspos = (uint64)-1;
	bool *changedPart = m_FlushSetting->changedPart;
	uint64 uWrotetodisk = 0;
	m_BufferedData_list_Locker.Lock();
	for (int i = m_BufferedData_list.GetCount(); i>0 && uWrotetodisk < thePrefs.GetFileBufferSize(); i--)
	{
		item = m_BufferedData_list.GetHead();
		if (item->data == NULL) {
			m_BufferedData_list.RemoveHead();
			delete item;
			continue;
		}
		m_BufferedData_list_Locker.Unlock();
		// This is needed a few times
		uint32 lenData = (uint32)(item->end - item->start + 1);

		// Anis: SafeHash - could be more than one part
		for (uint32 curpart = (uint32)(item->start/PARTSIZE); curpart <= item->end/PARTSIZE; curpart++)
			changedPart[curpart] = true;
		// Anis: SafeHash

		// Go to the correct position in file and write block of data
		
		if (previouspos != item->start) //MORPH - Optimization
			m_hpartfile.Seek(item->start, CFile::begin);
		m_hpartfile.Write(item->data, lenData);
		previouspos = item->end + 1; //MORPH - Optimization
		
		// Release memory used by this item
		delete [] item->data;
		item->data = NULL;
		m_BufferedData_list_Locker.Lock();
		// Decrease buffer size
		m_nTotalBufferData -= lenData;
		uWrotetodisk+=lenData;
	}
	m_BufferedData_list_Locker.Unlock();

	// Partfile should never be too large
 	if (m_hpartfile.GetLength() > m_nFileSize){
		// it's "last chance" correction. the real bugfix has to be applied 'somewhere' else
		TRACE(_T("Partfile \"%s\" is too large! Truncating %I64u bytes.\n"), GetFileName(), m_hpartfile.GetLength() - m_nFileSize);
		m_hpartfile.SetLength(m_nFileSize);
	}
	// Flush to disk
	m_hpartfile.Flush();
}

void CPartFile::FlushDone()
{
	if (m_FlushSetting == NULL) //Already do in normal process
		return;
	// Check each part of the file
	// Only if hashlist is available
	if (m_FileIdentifier.HasExpectedMD4HashCount()){
		UINT partCount = GetPartCount();
		// Check each part of the file
		for (int partNumber = partCount-1; partNumber >= 0; partNumber--)
		{
			if (!m_FlushSetting->changedPart[partNumber])
				continue;
			/*
			// Any parts other than last must be full size
			if (!GetFileIdentifier().GetMD4PartHash(partNumber)) {
				LogError(LOG_STATUSBAR, GetResString(IDS_ERR_INCOMPLETEHASH), GetFileName());
				m_bMD4HashsetNeeded = true;
				ASSERT(FALSE);	// If this fails, something was seriously wrong with the hashset loading or the check above
			}
			*/

			// Is this 9MB part complete
			//MORPH - Changed by SiRoB, As we are using flushed data check asynchronously we need to check if all data have been written into the file buffer
			/*
			if (IsComplete(PARTSIZE * (uint64)partNumber, (PARTSIZE * (uint64)(partNumber + 1)) - 1, false))
			*/
			if (IsComplete(PARTSIZE * (uint64)partNumber, (PARTSIZE * (uint64)(partNumber + 1)) - 1, true))
			{
				// Is part corrupt
				// Let's check in another thread
				m_PartsHashing++;
				if (theApp.emuledlg->IsRunning()) { //MORPH - Flush Thread
					CPartHashThread* parthashthread = (CPartHashThread*) AfxBeginThread(RUNTIME_CLASS(CPartHashThread), THREAD_PRIORITY_BELOW_NORMAL,0, CREATE_SUSPENDED);
					parthashthread->SetSinglePartHash(this, (UINT)partNumber);
					parthashthread->ResumeThread();
				//MORPH START - Flush Thread
				} else { 
					bool bAICHAgreed = false;
					if (!HashSinglePart(partNumber, &bAICHAgreed))
						PartHashFinished(partNumber, bAICHAgreed, true);
					else
						PartHashFinished(partNumber, bAICHAgreed, false);
				}
				//MORPH END   - Flush Thread
			}
			else if (IsCorruptedPart(partNumber) && (thePrefs.IsICHEnabled() || m_FlushSetting->bForceICH))
			{
				if (theApp.emuledlg->IsRunning()) { //MORPH - Flush Thread
					CPartHashThread* parthashthread = (CPartHashThread*) AfxBeginThread(RUNTIME_CLASS(CPartHashThread), THREAD_PRIORITY_BELOW_NORMAL,0, CREATE_SUSPENDED);
					parthashthread->SetSinglePartHash(this, (UINT)partNumber, true);	// Special case, doesn't increment hashing parts, since part isn't really complete
					parthashthread->ResumeThread();
				//MORPH START - Flush Thread
				} else { 
					if (!HashSinglePart(partNumber))
						PartHashFinishedAICHRecover(partNumber, true);
					else
						PartHashFinishedAICHRecover(partNumber, false);
				}
				//MORPH END   - Flush Thread
			}
		}
	}
	else {
		ASSERT(GetED2KPartCount() > 1);	// Files with only 1 chunk should have a forced hashset
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_HASHERRORWARNING), GetFileName());
		m_bMD4HashsetNeeded = true;
		m_bAICHPartHashsetNeeded = true;
	}
	// SLUGFILLER: SafeHash
	
	// Update met file
	//SavePartFile(); //MORPH - Flush Thread Moved Down
	
	if (theApp.emuledlg->IsRunning()) // may be called during shutdown!
	{

		if (m_bPauseOnPreview && IsReadyForPreview())
		{
			m_bPauseOnPreview = false;
			PauseFile();
		}
		// SLUGFILLER: SafeHash remove - Don't perform file completion here

		// Check free diskspace
		//
		// Checking the free disk space again after the file was written could most likely be avoided, but because
		// we do not use real physical disk allocation units for the free disk computations, it should be more safe
		// and accurate to check the free disk space again, after file was written and buffers were flushed to disk.
		//
		// If useing a normal file, we could avoid the check disk space if the file was not increased.
		// If useing a compressed or sparse file, we always have to check the space 
		// regardless whether the file was increased in size or not.
		bool bCheckDiskspace = thePrefs.GetMinFreeDiskSpace() > 0;
		if (bCheckDiskspace && ((IsNormalFile() && m_FlushSetting->bIncreasedFile) || !IsNormalFile()))
		{
			switch(GetStatus())
			{
			case PS_PAUSED:
			case PS_ERROR:
			case PS_COMPLETING:
			case PS_COMPLETE:
				break;
			default:
				if (GetFreeDiskSpaceX(GetTempPath()) < thePrefs.GetMinFreeDiskSpace())
				{
					if (IsNormalFile())
					{
						// Normal files: pause the file only if it would still grow
						if (GetNeededSpace() > 0)
							PauseFile(true/*bInsufficient*/);
					}
					else
					{
						// Compressed/sparse files: always pause the file
						PauseFile(true/*bInsufficient*/);
					}
				}
			}
		}
	}
	delete[] m_FlushSetting->changedPart;
	delete	m_FlushSetting;
	m_FlushSetting = NULL;
	// Update met file
	SavePartFile();
}

IMPLEMENT_DYNCREATE(CPartFileFlushThread, CWinThread)
void CPartFileFlushThread::SetPartFile(CPartFile* partfile)
{
	m_partfile = partfile;
	pauseEvent.SetEvent();
}	
void CPartFileFlushThread::StopFlush() {
	doRun = false;
	pauseEvent.SetEvent();
}	

int CPartFileFlushThread::Run()
{
	DbgSetThreadName("Partfile-Flushing");
	//InitThreadLocale(); //Performance killer

	// Anis: SafeHash
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return 0;
	// Anis: SafeHash

	doRun = true;
	pauseEvent.Lock();
	
	//theApp.QueueDebugLogLine(false,_T("FLUSH:Start (%s)"),m_partfile->GetFileName()/*, CastItoXBytes(myfile->m_iAllocinfo, false, false)*/ );
	while(doRun) {
	try{
		CSingleLock sLock1(&(theApp.hashing_mut), TRUE); //MORPH - Wait any Read/Write access (hashing or download stuff) before flushing
		m_partfile->WriteToDisk();
	}
	catch (CFileException* error)
	{
		VERIFY( PostMessage(theApp.emuledlg->m_hWnd,TM_FILEALLOCEXC,(WPARAM)m_partfile,(LPARAM)error) );
		return 1;
	}
#ifndef _DEBUG
	catch(...)
	{
		VERIFY( PostMessage(theApp.emuledlg->m_hWnd,TM_FILEALLOCEXC,(WPARAM)m_partfile,0) );
		return 2;
	}
#endif
	VERIFY( PostMessage(theApp.emuledlg->m_hWnd,TM_FLUSHDONE,0,(LPARAM)m_partfile) );
	//theApp.QueueDebugLogLine(false,_T("FLUSH:End (%s)"),m_partfile->GetFileName());
		pauseEvent.Lock();
	}
	return 0;
}

void CPartFile::ParseICHResult()
{
	if (m_ICHPartsComplete.IsEmpty())
		return;

	while (!m_ICHPartsComplete.IsEmpty()) {
		UINT partnumber = m_ICHPartsComplete.RemoveHead();
		uint64 partRange = (partnumber < GetPartCount()-1)?PARTSIZE:((uint64)m_nFileSize % PARTSIZE);

		m_uPartsSavedDueICH++;
		thePrefs.Add2SessionPartsSavedByICH(1);

		uint64 uRecovered = GetTotalGapSizeInPart(partnumber);
		//MORPH START - Changed by SiRoB, Fix
		/*
		if (partRange > 0) {
			partRange--;
			FillGap((uint64)PARTSIZE*partnumber, (uint64)PARTSIZE*partnumber + partRange);
			RemoveBlockFromList((uint32)PARTSIZE*partnumber, (uint32)PARTSIZE*partnumber + partRange);
		}
		*/
		if (partRange > 0)
			partRange--;
		else
			partRange = PARTSIZE - 1;
		FillGap((uint64)PARTSIZE*partnumber, (uint64)PARTSIZE*partnumber + partRange);
		RemoveBlockFromList((uint64)PARTSIZE*partnumber, (uint64)PARTSIZE*partnumber + partRange);
		//MORPH END   - Changed by SiRoB, Fix
		
		// tell the blackbox about the verified data
		m_CorruptionBlackBox.VerifiedData((uint64)PARTSIZE * partnumber, (uint64)PARTSIZE*partnumber + partRange);

		// remove from corrupted list
		POSITION posCorrupted = corrupted_list.Find(partnumber);
		if (posCorrupted)
			corrupted_list.RemoveAt(posCorrupted);

		AddLogLine(true, GetResString(IDS_ICHWORKED), partnumber, GetFileName(), CastItoXBytes(uRecovered, false, false));

		// correct file stats
		if (m_uCorruptionLoss >= uRecovered) // check, in case the tag was not present in part.met
			m_uCorruptionLoss -= uRecovered;
		// here we can't know if we have to subtract the amount of recovered data from the session stats
		// or the cumulative stats, so we subtract from where we can which leads eventuall to correct 
		// total stats
		if (thePrefs.sesLostFromCorruption >= uRecovered)
			thePrefs.sesLostFromCorruption -= uRecovered;
		else if (thePrefs.cumLostFromCorruption >= uRecovered)
			thePrefs.cumLostFromCorruption -= uRecovered;

		// Successfully recovered part, make it available for sharing
		m_PartsShareable[partnumber] = true;
		if (status == PS_EMPTY)
		{
			SetStatus(PS_READY);
			if (theApp.emuledlg->IsRunning())	// may be called during shutdown!
				theApp.sharedfiles->SafeAddKFile(this);
		}
	}

	// Update met file - gaps data changed
	SavePartFile();

	if (theApp.emuledlg->IsRunning()){ // may be called during shutdown!
		// Is this file finished?
		if (!m_PartsHashing && !m_nTotalBufferData && !m_FlushSetting) //MORPH - Changed by SiRoB, Flush Thread
		{
			if(gaplist.IsEmpty())
				CompleteFile(false);	// Recheck all hashes, because loaded data is trusted based on file-date
			else if (m_bPauseOnPreview && IsReadyForPreview())
			{
				m_bPauseOnPreview = false;
				PauseFile();
			}
		}
	}
}

IMPLEMENT_DYNCREATE(CPartHashThread, CWinThread)

int CPartHashThread::SetFirstHash(CPartFile* pOwner)
{
	m_pOwner = pOwner;
	m_ICHused = false;
	m_AICHRecover = false;
	directory = pOwner->GetTempPath();
	filename = RemoveFileExtension(pOwner->GetPartMetFileName());

	if (!theApp.emuledlg->IsRunning())	// Don't start any last-minute hashing
		return 1;	// Hash next start

	for (UINT i = 0; i < pOwner->GetPartCount(); i++)
		//MORPH - Changed by SiRoB, Need to check buffereddata otherwise we may try to hash wrong part
		/*
		if (pOwner->IsComplete((uint64)i*PARTSIZE,(uint64)(i+1)*PARTSIZE-1, false)){
		*/
		if (pOwner->IsComplete((uint64)i*PARTSIZE,(uint64)(i+1)*PARTSIZE-1, true)){
			uchar* cur_hash = new uchar[16];
			/*
			md4cpy(cur_hash, pOwner->GetFileIdentifier().GetMD4PartHash(i));
			*/
			//MD4
			if (pOwner->m_FileIdentifier.HasExpectedMD4HashCount())
			{
				if (pOwner->GetPartCount() > 1 || pOwner->GetFileSize()== (uint64)PARTSIZE)
				{
					if (pOwner->GetFileIdentifier().GetAvailableMD4PartHashCount() > i)
						md4cpy(cur_hash, pOwner->GetFileIdentifier().GetMD4PartHash(i));
					else
						ASSERT( false );
				}
				else
					md4cpy(cur_hash, pOwner->GetFileIdentifier().GetMD4Hash());
			}
			else
			{
				DebugLogError(_T("MD4 HashSet not present while veryfing part %u for file %s"), i, m_pOwner->GetFileName());
				m_pOwner->m_bMD4HashsetNeeded = true;
			}

			//AICH
			CAICHHash cur_AICH_hash;
			CAICHHashTree* phtAICHPartHash = NULL;
			if (pOwner->m_FileIdentifier.HasAICHHash() && pOwner->m_FileIdentifier.HasExpectedAICHHashCount())
			{
				const CAICHHashTree* pPartTree = pOwner->m_pAICHRecoveryHashSet->FindPartHash((uint16)i);
				if (pPartTree != NULL)
				{
					// use a new part tree, so we don't overwrite any existing recovery data which we might still need lateron
					phtAICHPartHash = new CAICHHashTree(pPartTree->m_nDataSize,pPartTree->m_bIsLeftBranch, pPartTree->GetBaseSize());	
				}
				else
					ASSERT( false );

				if (pOwner->GetPartCount() > 1)
				{
					if (pOwner->m_FileIdentifier.GetAvailableAICHPartHashCount() > i)
						cur_AICH_hash = pOwner->m_FileIdentifier.GetRawAICHHashSet()[i];
					else
						ASSERT( false );
				}
				else
					cur_AICH_hash = pOwner->m_FileIdentifier.GetAICHHash();
			}

			m_PartsToHash.Add(i);
			m_DesiredHashes.Add(cur_hash);
			m_phtAICHPartHash.Add(phtAICHPartHash);
			m_DesiredAICHHashes.Add(cur_AICH_hash);
		}
	return m_PartsToHash.GetSize();
}

void CPartFile::PerformFirstHash()
{
	CPartHashThread* parthashthread = (CPartHashThread*) AfxBeginThread(RUNTIME_CLASS(CPartHashThread), THREAD_PRIORITY_BELOW_NORMAL,0, CREATE_SUSPENDED);
	m_PartsHashing += parthashthread->SetFirstHash(this);	// Only hashes completed parts, why hash gaps?
	parthashthread->ResumeThread();
}

void CPartHashThread::SetSinglePartHash(CPartFile* pOwner, UINT part, bool ICHused, bool AICHRecover)
{
	m_pOwner = pOwner;
	m_ICHused = ICHused;
	m_AICHRecover = AICHRecover;
	directory = pOwner->GetTempPath();
	filename = RemoveFileExtension(pOwner->GetPartMetFileName());

	if (!theApp.emuledlg->IsRunning())	// Don't start any last-minute hashing
		return;

	if (part >= pOwner->GetPartCount()) {	// Out of bounds, no point in even trying
		if (AICHRecover)
			PostMessage(theApp.emuledlg->m_hWnd,TM_PARTHASHEDCORRUPTAICHRECOVER,part,(LPARAM)m_pOwner);
		else if (!ICHused)		// ICH only sends successes
			PostMessage(theApp.emuledlg->m_hWnd,TM_PARTHASHEDCORRUPT,part,(LPARAM)m_pOwner);
		return;
	}

	//MD4
	uchar* cur_hash = NULL;
	if (pOwner->m_FileIdentifier.HasExpectedMD4HashCount())
	{
		cur_hash = new uchar[16];
		if (pOwner->GetPartCount() > 1 || pOwner->GetFileSize()== (uint64)PARTSIZE)
		{
			if (pOwner->GetFileIdentifier().GetAvailableMD4PartHashCount() > part)
				md4cpy(cur_hash, pOwner->GetFileIdentifier().GetMD4PartHash(part));
			else
			{
				ASSERT( false );
				pOwner->m_bMD4HashsetNeeded = true;
			}
		}
		else
			md4cpy(cur_hash, pOwner->GetFileIdentifier().GetMD4Hash());
	}
	else
	{
		DebugLogError(_T("MD4 HashSet not present while veryfing part %u for file %s"), part, m_pOwner->GetFileName());
		m_pOwner->m_bMD4HashsetNeeded = true;
	}

	//AICH
	CAICHHash cur_AICH_hash;
	CAICHHashTree* phtAICHPartHash = NULL;
	if (pOwner->m_FileIdentifier.HasAICHHash() && pOwner->m_FileIdentifier.HasExpectedAICHHashCount())
	{
		const CAICHHashTree* pPartTree = pOwner->m_pAICHRecoveryHashSet->FindPartHash((uint16)part);
		if (pPartTree != NULL)
		{
			// use a new part tree, so we don't overwrite any existing recovery data which we might still need lateron
			phtAICHPartHash = new CAICHHashTree(pPartTree->m_nDataSize,pPartTree->m_bIsLeftBranch, pPartTree->GetBaseSize());	
		}
		else
			ASSERT( false );

		if (pOwner->GetPartCount() > 1)
		{
			if (pOwner->m_FileIdentifier.GetAvailableAICHPartHashCount() > part)
				cur_AICH_hash = pOwner->m_FileIdentifier.GetRawAICHHashSet()[part];
			else
				ASSERT( false );
		}
		else
			cur_AICH_hash = pOwner->m_FileIdentifier.GetAICHHash();
	}

	m_PartsToHash.Add(part);
	m_DesiredHashes.Add(cur_hash);
	m_phtAICHPartHash.Add(phtAICHPartHash);
	m_DesiredAICHHashes.Add(cur_AICH_hash);
}

void CPartFile::PartHashFinished(UINT  partnumber, bool bAICHAgreed, bool corrupt)
{
	if (partnumber >= GetPartCount())
		return;
	m_PartsHashing--;
	uint64 partRange = (partnumber < (UINT)(GetPartCount()-1))?PARTSIZE:((uint64)m_nFileSize % PARTSIZE);
	if (corrupt){
		LogWarning(LOG_STATUSBAR, GetResString(IDS_ERR_PARTCORRUPT), partnumber, GetFileName());
		//MORPH START - Changed by SiRoB, SafeHash Fix
		/*
		if (partRange > 0) {
			partRange--;
			AddGap((uint64)PARTSIZE*partnumber, (uint64)PARTSIZE*partnumber + partRange);
		}
		*/
		if (partRange > 0)
			partRange--;
		else
			partRange = PARTSIZE - 1;
		AddGap((uint64)PARTSIZE*partnumber, (uint64)PARTSIZE*partnumber + partRange);
		//MORPH END   - Changed by SiRoB, SafeHash Fix

		// add part to corrupted list, if not already there
		if (!IsCorruptedPart(partnumber))
			corrupted_list.AddTail(partnumber);

		// request AICH recovery data, except if AICH already agreed anyway
		if (!bAICHAgreed)
			RequestAICHRecovery(partnumber);

		// update stats
		m_uCorruptionLoss += (partRange + 1);
		thePrefs.Add2LostFromCorruption(partRange + 1);

		// Update met file - gaps data changed
		SavePartFile();
	} else {
		if (thePrefs.GetVerbose())
			AddDebugLogLine(DLP_VERYLOW, false, _T("Finished part %u of \"%s\""), partnumber, GetFileName());

		//MORPH START - Changed by SiRoB, SafeHash Fix
		/*
		if (partRange > 0) {
			partRange--;

			// tell the blackbox about the verified data
			m_CorruptionBlackBox.VerifiedData((uint64)PARTSIZE * partnumber, (uint64)PARTSIZE*partnumber + partRange);
		}
		*/
		if (partRange > 0)
			partRange--;
		else
			partRange = PARTSIZE - 1;
		m_CorruptionBlackBox.VerifiedData((uint64)PARTSIZE * partnumber, (uint64)PARTSIZE*partnumber + partRange);
		//MORPH END   - Added by SiRoB, SafeHash -Fix-
		// if this part was successfully completed (although ICH is active), remove from corrupted list
		POSITION posCorrupted = corrupted_list.Find(partnumber);
		if (posCorrupted)
			corrupted_list.RemoveAt(posCorrupted);

		// Successfully completed part, make it available for sharing
		m_PartsShareable[partnumber] = true;
		if (status == PS_EMPTY)
		{
			SetStatus(PS_READY);
			if (theApp.emuledlg->IsRunning())	// may be called during shutdown!
				theApp.sharedfiles->SafeAddKFile(this);
		}

		if (!m_PartsHashing){
			// Update met file - file fully hashed
			SavePartFile();
		}

		if (theApp.emuledlg->IsRunning())	// may be called during shutdown!
		{
			// Is this file finished?
			if (!m_PartsHashing && !m_nTotalBufferData && !m_FlushSetting) //MORPH - Changed by SiRoB, Flush Thread
			{
				if(gaplist.IsEmpty())
					CompleteFile(false);	// Recheck all hashes, because loaded data is trusted based on file-date
				else if (m_bPauseOnPreview && IsReadyForPreview())
				{
					m_bPauseOnPreview = false;
					PauseFile();
				}
			}
		}
	}
}

bool CPartFile::IsPartShareable(UINT partnumber) const
{
	if (partnumber < GetPartCount())
		return m_PartsShareable[partnumber];
	else
		return false;
}

void CPartFile::PartHashFinishedAICHRecover(UINT partnumber, bool corrupt)
{
	if (partnumber >= GetPartCount())
		return;
	m_PartsHashing--;
	if (corrupt){
		uint64 partRange = (partnumber < (UINT)(GetPartCount()-1))?PARTSIZE:((uint64)m_nFileSize % PARTSIZE);
		AddDebugLogLine(DLP_DEFAULT, false, _T("Processing AICH Recovery data: The part (%u) got completed while recovering - but MD4 says it corrupt! Setting hashset to error state, deleting part"), partnumber);
		// now we are fu... unhappy
		if (!m_FileIdentifier.HasAICHHash())
			m_pAICHRecoveryHashSet->SetStatus(AICH_ERROR); // set it to error on unverified hashs
		//MORPH START - Changed by SiRoB, SafeHash Fix
		/*
		if (partRange > 0) {
			partRange--;
			AddGap((uint64)PARTSIZE*partnumber, (uint64)PARTSIZE*partnumber + partRange);
		}
		*/
		if (partRange > 0)
			partRange--;
		else
			partRange = PARTSIZE - 1;
		AddGap((uint64)PARTSIZE*partnumber, (uint64)PARTSIZE*partnumber + partRange);
		//MORPH END   - Changed by SiRoB, SafeHash Fix
		ASSERT( false );

		// Update met file - gaps data changed
		SavePartFile();
	}
	else{
		AddDebugLogLine(DLP_DEFAULT, false, _T("Processing AICH Recovery data: The part (%u) got completed while recovering and MD4 agrees"), partnumber);
		// alrighty not so bad
		POSITION posCorrupted = corrupted_list.Find(partnumber);
		if (posCorrupted)
			corrupted_list.RemoveAt(posCorrupted);
		// Successfully recovered part, make it available for sharing
		m_PartsShareable[partnumber] = true;
		if (status == PS_EMPTY)
		{
			SetStatus(PS_READY);
			if (theApp.emuledlg->IsRunning())	// may be called during shutdown!
				theApp.sharedfiles->SafeAddKFile(this);
		}

		if (!m_PartsHashing){
			// Update met file - file fully hashed
			SavePartFile();
		}

		if (theApp.emuledlg->IsRunning()){
			// Is this file finished?
			if (!m_PartsHashing && !m_nTotalBufferData && !m_FlushSetting) //MORPH - Changed by SiRoB, Flush Thread
			{
				if(gaplist.IsEmpty())
					CompleteFile(false);	// Recheck all hashes, because loaded data is trusted based on file-date
				else if (m_bPauseOnPreview && IsReadyForPreview())
				{
					m_bPauseOnPreview = false;
					PauseFile();
				}
			}
		}
	}
}

bool CPartFile::IsRangeShareable(uint64 start, uint64 end) const
{
	UINT first = (UINT)(start/PARTSIZE);
	UINT last = (UINT)(end/PARTSIZE+1);
	if (last > GetPartCount() || first >= last)
		return false;
	for (UINT i = first; i < last; i++)
		if (!m_PartsShareable[i])
			return false;
	return true;
}

int CPartHashThread::Run()
{
	DbgSetThreadName("PartHashThread");
	//InitThreadLocale(); //Performance killer

	// Anis: SafeHash
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return 0;
	// Anis: SafeHash

	CFile file;
	CSingleLock sLock(&(m_pOwner->ICH_mut)); // ICH locks the file
	if (m_ICHused)
		sLock.Lock();
	
	if (file.Open(directory+_T("\\")+filename,CFile::modeRead|CFile::osSequentialScan|CFile::shareDenyNone)){
		for (UINT i = 0; i < (UINT)m_PartsToHash.GetSize(); i++){
			bool pbAICHReportedOK = false;
			UINT partnumber = m_PartsToHash[i];
			uchar hashresult[16];
			file.Seek((LONGLONG)PARTSIZE*partnumber,0);
			uint64 length = PARTSIZE;
			if ((ULONGLONG)PARTSIZE*(partnumber+1) > file.GetLength()){
				length = (file.GetLength() - ((ULONGLONG)PARTSIZE*partnumber));
				ASSERT( length <= PARTSIZE );
			}

			CAICHHashTree* phtAICHPartHash = m_phtAICHPartHash[i];

			//MORPH - Changed by SiRoB, avoid crash if the file has been canceled
			try
			{
				m_pOwner->CreateHash(&file, length, hashresult, phtAICHPartHash);
				if (!theApp.emuledlg->IsRunning())	// in case of shutdown while still hashing
					break;
			}
			catch(CFileException* ex)
			{
				ex->Delete();
				if (!theApp.emuledlg->IsRunning())	// in case of shutdown while still hashing
					break;
				continue;
			}

			bool bMD4Error = false;
			bool bMD4Checked = false;
			bool bAICHError = false;
			bool bAICHChecked = false;

			//MD4
			if (m_DesiredHashes[i] != NULL)
			{
				bMD4Checked = true;
				bMD4Error = md4cmp(hashresult,m_DesiredHashes[i]) != 0;
			}

			//AICH
			//Note: If phtAICHPartHash is NULL it will remain NULL while hashing using CreateHash. So if we should not check by AICH it
			//      is going to be NULL and thus the below will not be executed.
			if (phtAICHPartHash != NULL)
			{
				ASSERT( phtAICHPartHash->m_bHashValid );
				bAICHChecked = true;
				bAICHError = m_DesiredAICHHashes[i] != phtAICHPartHash->m_Hash;
			}
			//else
			//	DebugLogWarning(_T("AICH HashSet not present while verifying part %u for file %s"), partnumber, m_pOwner->GetFileName());
			phtAICHPartHash = NULL;

			if (bAICHChecked)
				pbAICHReportedOK = !bAICHError;
			if (bMD4Checked && bAICHChecked && bMD4Error != bAICHError)
				DebugLogError(_T("AICH and MD4 HashSet disagree on verifying part %u for file %s. MD4: %s - AICH: %s"), partnumber
				, m_pOwner->GetFileName(), bMD4Error ? _T("Corrupt") : _T("OK"), bAICHError ? _T("Corrupt") : _T("OK"));
#ifdef _DEBUG
			else
				DebugLog(_T("Verifying part %u for file %s. MD4: %s - AICH: %s"), partnumber , m_pOwner->GetFileName()
				, bMD4Checked ? (bMD4Error ? _T("Corrupt") : _T("OK")) : _T("Unavailable"), bAICHChecked ? (bAICHError ? _T("Corrupt") : _T("OK")) : _T("Unavailable"));	
#endif

			if (bMD4Error || bAICHError){
				if (m_AICHRecover)
					PostMessage(theApp.emuledlg->m_hWnd,TM_PARTHASHEDCORRUPTAICHRECOVER,partnumber,(LPARAM)m_pOwner);
				else if (!m_ICHused)		// ICH only sends successes
				{
					if(pbAICHReportedOK)
						PostMessage(theApp.emuledlg->m_hWnd,TM_PARTHASHEDCORRUPT,partnumber,(LPARAM)m_pOwner);
					else
						PostMessage(theApp.emuledlg->m_hWnd,TM_PARTHASHEDCORRUPTNOAICH,partnumber,(LPARAM)m_pOwner);
				}
			} else {
				if (m_ICHused)
					m_pOwner->m_ICHPartsComplete.AddTail(partnumber);	// Time critical, don't use message callback
				else if (m_AICHRecover)
					PostMessage(theApp.emuledlg->m_hWnd,TM_PARTHASHEDOKAICHRECOVER,partnumber,(LPARAM)m_pOwner);
				else if(pbAICHReportedOK)
					PostMessage(theApp.emuledlg->m_hWnd,TM_PARTHASHEDOK,partnumber,(LPARAM)m_pOwner);
				else
					PostMessage(theApp.emuledlg->m_hWnd,TM_PARTHASHEDOKNOAICH,partnumber,(LPARAM)m_pOwner);
			}
		}
		file.Close();
	}
	for (UINT i = 0; i < (UINT)m_DesiredHashes.GetSize(); i++)
		delete[] m_DesiredHashes[i];
	for (UINT i = 0; i < (UINT)m_phtAICHPartHash.GetSize(); i++)
		delete m_phtAICHPartHash[i];
	m_DesiredAICHHashes.RemoveAll(); // I just hope this does not create a mem leak...
	if (m_ICHused)
		sLock.Unlock();
	return 0;
}