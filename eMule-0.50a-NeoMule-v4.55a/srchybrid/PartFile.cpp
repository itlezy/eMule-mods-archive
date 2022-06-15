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
#include "TransferWnd.h"
#include "TaskbarNotifier.h"
#include "ClientList.h"
#include "Statistics.h"
#include "shahashset.h"
#include "PeerCacheSocket.h"
#include "Log.h"
#include "CollectionViewDialog.h"
#include "Collection.h"
#include "Neo/ClientFileStatus.h"// NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
#include "Neo/NeoOpcodes.h" // NEO: NMP - [NeoModProt] <-- Xanatos --
#include "Neo/NeoPreferences.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --
#include "Neo/FilePreferences.h" // NEO: FCFG - [FileConfiguration] <-- Xanatos --
#include "Neo/Defaults.h" // NEO: FCFG - [FileConfiguration] <-- Xanatos --
#include "Neo/Functions.h" // NEO: MOD <-- Xanatos --
#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
#include "Neo/Sources/SourceList.h"
#endif // NEO_SA // NEO: NSA END
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
#include "Neo/NatT/NatManager.h"
#include "Neo/NatT/NatTunnel.h"
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
#include "Neo/BC/DownloadBandwidthThrottler.h"
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
#include "Neo/LanCast/Lancast.h"
#endif //LANCAST // NEO: NLC END <-- Xanatos --
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
#include "Neo/VooDoo/voodoo.h"
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
#define VOODOO_BLOCK_TIMEOUT MIN2MS(5);
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// Barry - use this constant for both places
#define PROGRESS_HEIGHT 3

CBarShader CPartFile::s_LoadBar(PROGRESS_HEIGHT); // Barry - was 5
CBarShader CPartFile::s_ChunkBar(16); 

IMPLEMENT_DYNAMIC(CPartFile, CKnownFile)

CPartFile::CPartFile(UINT ucat)
{
	Init();
	//m_category=ucat;
	SetCategory(ucat,1); // NEO: MOD - [SetCategory] <-- Xanatos --
}

CPartFile::CPartFile(CSearchFile* searchresult, UINT cat)
{
	Init();

	const CTypedPtrList<CPtrList, Kademlia::CEntry*>& list = searchresult->getNotes();
	for(POSITION pos = list.GetHeadPosition(); pos != NULL; )
	{
			Kademlia::CEntry* entry = list.GetNext(pos);
			m_kadNotes.AddTail(entry->Copy());
	}
	UpdateFileRatingCommentAvail();

	md4cpy(m_abyFileHash, searchresult->GetFileHash());
	for (int i = 0; i < searchresult->taglist.GetCount();i++){
		const CTag* pTag = searchresult->taglist[i];
		switch (pTag->GetNameID()){
			case FT_FILENAME:{
				ASSERT( pTag->IsStr() );
				if (pTag->IsStr()){
					//if (GetFileName().IsEmpty())
					if (GetFileName(true).IsEmpty()) // NEO: PP - [PasswordProtection] <-- Xanatos --
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
	SetCategory(cat,1); // NEO: MOD - [SetCategory] <-- Xanatos --
	CreatePartFile(cat);
	//m_category=cat;
}

CPartFile::CPartFile(CString edonkeylink, UINT cat)
{
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
		md4cpy(m_abyFileHash, fileLink->GetHashKey());
		if (!theApp.downloadqueue->IsFileExisting(m_abyFileHash))
		{
			if (fileLink->m_hashset && fileLink->m_hashset->GetLength() > 0)
			{
				try
				{
					if (!LoadHashsetFromFile(fileLink->m_hashset, true))
					{
						ASSERT( hashlist.GetCount() == 0 );
						AddDebugLogLine(false, _T("eD2K link \"%s\" specified with invalid hashset"), fileLink->GetName());
					}
					else
						hashsetneeded = false;
				}
				catch (CFileException* e)
				{
					TCHAR szError[MAX_CFEXP_ERRORMSG];
					e->GetErrorMessage(szError, ARRSIZE(szError));
					AddDebugLogLine(false, _T("Error: Failed to process hashset for eD2K link \"%s\" - %s"), fileLink->GetName(), szError);
					e->Delete();
				}
			}
			SetCategory(cat,1); // NEO: MOD - [SetCategory] <-- Xanatos --
			CreatePartFile(cat);
			//m_category=cat;
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
	InitializeFromLink(fileLink,cat);
}

void CPartFile::Init(){
	newdate = true;
	m_LastSearchTime = 0;
	m_LastSearchTimeUdp = 0; // NEO: SRT - [SourceRequestTweaks] <-- Xanatos --
	m_LastSearchTimeKad = 0;
	m_TotalSearchesKad = 0;
	lastpurgetime = ::GetTickCount();
	paused = false;
	stopped= false;
	forced=false; // NEO: OCF - [OnlyCompleetFiles] <-- Xanatos --
	standby=false; // NEO: SD - [StandByDL] <-- Xanatos --
	suspend=false; // NEO: SC - [SuspendCollecting] <-- Xanatos --
	status = PS_EMPTY;
	insufficient = false;
	m_bCompletionError = false;
	m_bCompletionBreak = false; // NEO: POFC - [PauseOnFileComplete] <-- Xanatos --
	m_uTransferred = 0;
	m_uTransferredSession = 0; // MOD - [SessionDL] <-- Xanatos --
	m_iLastPausePurge = time(NULL);
	m_AllocateThread=NULL;
	m_iAllocinfo = 0;
	if(thePrefs.GetNewAutoDown()){
		m_iDownPriority = PR_HIGH;
		m_bAutoDownPriority = true;
	}
	else{
		m_iDownPriority = PR_NORMAL;
		m_bAutoDownPriority = false;
	}
	srcarevisible = false;
	memset(m_anStates,0,sizeof(m_anStates));
	datarate = 0;
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	landatarate = 0;
#endif //LANCAST // NEO: NLC END <-- Xanatos --
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	voodoodatarate = 0;
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
	//m_uMaxSources = 0; // NEO: SRT - [SourceRequestTweaks] <-- Xanatos --
	hashsetneeded = true;
	count = 0;
	percentcompleted = 0;
	percentcompletedinitial = 0; // NEO: MOD - [Percentage] <-- Xanatos --
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
	//m_category=0;  // NEO: NSC - [NeoSharedCategories] <-- Xanatos --
	m_lastRefreshedDLDisplay = 0;
	m_bLocalSrcReqQueued = false;
	memset(src_stats,0,sizeof(src_stats));
	memset(net_stats,0,sizeof(net_stats));
	m_nCompleteSourcesTime = time(NULL);
	m_nCompleteSourcesSize = 0; // NEO: MOD - [RelativeChunkDisplay] <-- Xanatos --
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
    m_bpreviewprio = false;
    m_random_update_wait = (uint32)(rand()/(RAND_MAX/1000));
    lastSwapForSourceExchangeTick = ::GetTickCount();
	//m_DeadSourceList.Init(false);
	m_DeadSourceList.Init(this); // NEO: SDT - [SourcesDropTweaks] <-- Xanatos --

	//MORPH START - Added by SiRoB, Flush Thread // NEO: FFT - [FileFlushThread] -- Xanatos -->
	m_FlushThread = NULL;
	m_FlushSetting = NULL;
	//MORPH END - Added by SiRoB, Flush Thread // NEO: FFT END <-- Xanatos --

	m_HashThread = NULL; // NEO: SSH - [SlugFillerSafeHash] <-- Xanatos --
	// NEO: SCV - [SubChunkVerification] -- Xanatos -->
	m_AICHHashThread = NULL;
	m_uAICHVeirifcationPending = 0; 
	// NEO: SCV END <-- Xanatos --

#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	m_LastLanSearchTime = 0;
#endif //LANCAST // NEO: NLC END <-- Xanatos --

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	m_isVoodooFile = false;
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

	PartPrefs = &NeoPrefs.PartPrefs; // NEO: FCFG - [FileConfiguration] <-- Xanatos --

	// NEO: SDT - [SourcesDropTweaks] -- Xanatos -->
	lastBadSourcePurgeTime = ::GetTickCount(); 
	lastNNPSourcePurgeTime = ::GetTickCount(); 
	lastFullQSourcePurgeTime = ::GetTickCount(); 
	lastHighQSourcePurgeTime = ::GetTickCount(); 
	m_uAverageQueueRank = 0;
	// NEO: SDT END <-- Xanatos --
	// NEO: ASL - [AutoSoftLock] -- Xanatos -->
	m_bCollectingHalted = false;
	m_bSoftLocked = false;
	// NEO: ASL END <-- Xanatos --
	// NEO: AHL - [AutoHardLimit] -- Xanatos -->
	m_uAutoHardLimit = 0;
	m_uLastAutoHardLimit = 0;
	// NEO: AHL END <-- Xanatos --
	// NEO: GSL - [GlobalSourceLimit] -- Xanatos -->
	m_uFileHardLimit = 0;
	//InitHL(); // do this when the part prefs are realy set
	if(NeoPrefs.PartPrefs.UseGlobalSourceLimit() &&
		theApp.downloadqueue->GetPassiveMode())
	{
		theApp.downloadqueue->SetPassiveMode(false);
		AddDebugLogLine(true,_T("{GSL} New file added! Disabled PassiveMode!"));
	}
	// NEO: GSL END <-- Xanatos --
	// NEO: CSL - [CategorySourceLimit] -- Xanatos -->
	m_uFileCategoryLimit = 0;
	m_uLastCategoryLimit = 0;
	// NEO: CSL END <-- Xanatos --

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
	m_uLastSaveSource = ::GetTickCount();
	m_uLastLoadSource = 0;

	m_uLastReaskedGroupeTime = 0;
#endif // NEO_SS // NEO: NSS END <-- Xanatos --

	m_ics_filemode = 0; // NEO: ICS - [InteligentChunkSelection] <-- Xanatos --

	m_iCollectingXSSources = 0;	// NEO: MSR - [ManualSourceRequest] <-- Xanatos --

	m_catResumeOrder = 0; // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
}

CPartFile::~CPartFile()
{
	// Barry - Ensure all buffered data is written
	try{
		if (m_AllocateThread != NULL){
			HANDLE hThread = m_AllocateThread->m_hThread;
			// 2 minutes to let the thread finish
			m_AllocateThread->SetThreadPriority(THREAD_PRIORITY_NORMAL);  // NEO: MOD - [SpaceAllocate] <-- Xanatos --
			if (WaitForSingleObject(hThread, 120000) == WAIT_TIMEOUT)
				TerminateThread(hThread, 100);
		}

		if (m_hpartfile.m_hFile != INVALID_HANDLE_VALUE)
			FlushBuffer(true);
	}
	catch(CFileException* e){
		e->Delete();
	}
	ASSERT(m_FlushSetting == NULL); //MORPH - Added by SiRoB, Flush Thread // NEO: FFT - [FileFlushThread] <-- Xanatos -- // flush was reported done but thread not properly ended?
	
	if (m_hpartfile.m_hFile != INVALID_HANDLE_VALUE){
		// commit file and directory entry
		m_hpartfile.Close();
		// Update met file (with current directory entry)
		SavePartFile();
	}

	POSITION pos;
	for (pos = gaplist.GetHeadPosition();pos != 0;)
		delete gaplist.GetNext(pos);
	m_BlockMaps.RemoveAll(); // NEO: SCV - [SubChunkVerification] <-- Xanatos --

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	// cleanup voodoo blocks
	for (pos = requestedblocks_list.GetHeadPosition(); pos != NULL; ){
		Requested_Block_Struct* block = requestedblocks_list.GetNext(pos);
		if(block->timeout != 0)
			delete block;
	}
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

	pos = m_BufferedData_list.GetHeadPosition();
	while (pos){
		PartFileBufferedData *item = m_BufferedData_list.GetNext(pos);
		delete[] item->data;
		delete item;
	}

	// NEO: FCFG - [FileConfiguration] -- Xanatos -->
	if(PartPrefs->IsFilePrefs())
		delete PartPrefs;
	// NEO: FCFG END <-- Xanatos --
}

#ifdef _DEBUG
void CPartFile::AssertValid() const
{
	CKnownFile::AssertValid();

	(void)m_LastSearchTime;
	(void)m_LastSearchTimeUdp; // NEO: SRT - [SourceRequestTweaks] <-- Xanatos --
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
	CHECK_BOOL(hashsetneeded);
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
	(void)m_uTransferredSession; // MOD - [SessionDL] <-- Xanatos --
	CHECK_BOOL(paused);
	CHECK_BOOL(stopped);
	CHECK_BOOL(forced); // NEO: OCF - [OnlyCompleetFiles] <-- Xanatos --
	CHECK_BOOL(standby); // NEO: SD - [StandByDL] <-- Xanatos --
	CHECK_BOOL(suspend); // NEO: SC - [SuspendCollecting] <-- Xanatos --
	CHECK_BOOL(insufficient);
	CHECK_BOOL(m_bCompletionError);
	CHECK_BOOL(m_bCompletionBreak); // NEO: POFC - [PauseOnFileComplete] <-- Xanatos --
	ASSERT( m_iDownPriority == PR_LOW || m_iDownPriority == PR_NORMAL || m_iDownPriority == PR_HIGH );
	CHECK_BOOL(m_bAutoDownPriority);
	ASSERT( status == PS_READY || status == PS_EMPTY /*|| status == PS_WAITINGFORHASH*/ || status == PS_ERROR || status == PS_COMPLETING || status == PS_COMPLETE || status == PS_MOVING || status == PS_IMPORTING); // NEO: SSH - [SlugFillerSafeHash]  // NEO: MTD - [MultiTempDirectories] // NEO: PIX - [PartImportExport] <-- Xanatos --
	CHECK_BOOL(newdate);
	(void)lastpurgetime;
	(void)m_LastNoNeededCheck;
	gaplist.AssertValid();
	requestedblocks_list.AssertValid();
	m_SrcpartFrequency.AssertValid();
	m_SrcincpartFrequency.AssertValid(); // NEO: ICS - [InteligentChunkSelection] <-- Xanatos --
	ASSERT( percentcompleted >= 0.0F && percentcompleted <= 100.0F );
	ASSERT( percentcompletedinitial >= 0.0F && percentcompletedinitial <= 100.0F ); // NEO: MOD - [Percentage] <-- Xanatos --
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
	(void) m_iCollectingXSSources; // NEO: MSR - [ManualSourceRequest] <-- Xanatos --
}

void CPartFile::Dump(CDumpContext& dc) const
{
	CKnownFile::Dump(dc);
}
#endif


void CPartFile::CreatePartFile(UINT /*cat*/) // NEO: MOD - [SetCategory] <-- Xanatos --
{
	if (m_nFileSize > (uint64)MAX_EMULE_FILE_SIZE){
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_CREATEPARTFILE));
		SetStatus(PS_ERROR);
		return;
	}

	// decide which tempfolder to use
	// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
	CString tempdirtouse;

	if(thePrefs.GetCategory(m_category) && PathFileExists(thePrefs.GetCategory(m_category)->strTempPath))
		tempdirtouse = thePrefs.GetCategory(m_category)->strTempPath;
	else if(NeoPrefs.GetUsedTempDir() == AUTO_TEMPDIR)
		tempdirtouse = theApp.downloadqueue->GetOptimalTempDir(m_category,GetFileSize());
	else
		tempdirtouse = thePrefs.GetTempDir(NeoPrefs.GetUsedTempDir());

	static int i = 0; // static makes ading of very many new downloads much faster
	bool found = false;
	CString tmpfilename;
	do{
		i++; 
		found = true;
		for (int j = 0; j < thePrefs.tempdir.GetCount(); j++) {
			tmpfilename.Format(_T("%s\\%03i.part"), thePrefs.GetTempDir(j), i); 
			if (PathFileExists(tmpfilename))
				found = false;
		}
	}
	while (!found); 
	// NEO: MTD END <-- Xanatos --

	/*CString tempdirtouse=theApp.downloadqueue->GetOptimalTempDir(cat,GetFileSize());

	// use lowest free partfilenumber for free file (InterCeptor)
	int i = 0; 
	CString filename; 
	do{
		i++; 
		filename.Format(_T("%s\\%03i.part"), tempdirtouse, i); 
	}
	while (PathFileExists(filename));*/

	m_partmetfilename.Format(_T("%03i.part.met"), i);
	SetPath(tempdirtouse);
	m_fullname.Format(_T("%s\\%s"), tempdirtouse, m_partmetfilename);

	CTag* partnametag = new CTag(FT_PARTFILENAME,RemoveFileExtension(m_partmetfilename));
	taglist.Add(partnametag);
	
	Gap_Struct* gap = new Gap_Struct;
	gap->start = 0;
	gap->end = m_nFileSize - (uint64)1;
	gaplist.AddTail(gap);

	CString partfull(RemoveFileExtension(m_fullname));
	SetFilePath(partfull);
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
  if(!IsVoodooFile()){
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
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
			m_tLastModified = fileinfo.st_mtime;
			m_tCreated = fileinfo.st_ctime;
		}
		else
			AddDebugLogLine(false, _T("Failed to get file date for \"%s\" - %s"), partfull, _tcserror(errno));
	}
	m_dwFileAttributes = GetFileAttributes(partfull);
	if (m_dwFileAttributes == INVALID_FILE_ATTRIBUTES)
		m_dwFileAttributes = 0;
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
  }
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

	if (GetED2KPartHashCount() == 0)
		hashsetneeded = false;

	// NEO: SSH - [SlugFillerSafeHash] -- Xanatos -->
	m_PartsShareable.SetSize(GetPartCount());
	for (uint32 i = 0; i < GetPartCount();i++)
		m_PartsShareable[i] = false;
	// NEO: SSH END <-- Xanatos --

	m_SrcpartFrequency.SetSize(GetPartCount());
	m_SrcincpartFrequency.SetSize(GetPartCount()); // NEO: ICS - [InteligentChunkSelection] <-- Xanatos --
	for (UINT i = 0; i < GetPartCount();i++){
		m_SrcpartFrequency[i] = 0;
		m_SrcincpartFrequency[i] = 0; // NEO: ICS - [InteligentChunkSelection] <-- Xanatos --
	}
	paused = false;

	if (thePrefs.AutoFilenameCleanup())
		//SetFileName(CleanupFilename(GetFileName()));
		SetFileName(CleanupFilename(GetFileName(true))); // NEO: PP - [PasswordProtection] <-- Xanatos --

	SavePartFile();
	//SetActive(theApp.IsConnected());
	SetActive(theApp.GetConState()); // NEO: NCC - [NeoConnectionChecker] <-- Xanatos --
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
uint8 CPartFile::ImportShareazaTempfile(LPCTSTR in_directory,LPCTSTR in_filename , bool getsizeonly) 
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
		return false;
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
			return PMT_UNKNOWN;
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
			md4cpy(m_abyFileHash, pED2K);
		}else{
			Log(LOG_ERROR,GetResString(IDS_X_SHAREAZA_IMPORT_NO_HASH),in_filename);
			sdFile.Close();
			return false;
		}

		if (getsizeonly){
			sdFile.Close();
			return PMT_SHAREAZA;
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
		if(gotostring(sdFile,m_abyFileHash,16)) // search the hashset
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
				hashlist.Add(curhash);
			}

			uchar* checkhash= new uchar[16];
			if (!hashlist.IsEmpty()){
				uchar* buffer = new uchar[hashlist.GetCount()*16];
				for (int i = 0; i < hashlist.GetCount(); i++)
					md4cpy(buffer+(i*16), hashlist[i]);
				CreateHash(buffer, hashlist.GetCount()*16, checkhash);
				delete[] buffer;
			}
			if (md4cmp(pMD4, checkhash)){
				for (int i = 0; i < hashlist.GetSize(); i++)
					delete[] hashlist[i];
				hashlist.RemoveAll();
				Log(LOG_WARNING,GetResString(IDS_X_SHAREAZA_IMPORT_HASH_SET_CORRUPT),in_filename);
			}
			delete[] checkhash;

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
		return false;
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
		return false;
	}
#ifndef _DEBUG
	catch(...){
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_METCORRUPT), in_filename, GetFileName());
		ASSERT(0);
		return false;
	}
#endif

	// The part below would be a copy of the CPartFile::LoadPartFile, 
	// so it is smarter to save and reload the file insta dof dougling the whole stuff
	if(!SavePartFile())
		return false;

	for (int i = 0; i < hashlist.GetSize(); i++)
		delete[] hashlist[i];
	hashlist.RemoveAll();
	while (gaplist.GetCount()>0 ) {
		delete gaplist.GetAt(gaplist.GetHeadPosition());
		gaplist.RemoveAt(gaplist.GetHeadPosition());
	}

	return LoadPartFile(in_directory, in_filename);
}

uint8 CPartFile::LoadPartFile(LPCTSTR in_directory,LPCTSTR in_filename, bool getsizeonly)
{
	bool isnewstyle;
	uint8 version;
	EPartFileFormat partmettype = PMT_UNKNOWN;

	//CMap<UINT, UINT, Gap_Struct*, Gap_Struct*> gap_map; // Slugfiller // NEO: VOODOO - [UniversalPartfileInterface] <-- Xanatos --
	m_uTransferred = 0;
	m_uTransferredSession = 0; // MOD - [SessionDL] <-- Xanatos --
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
		return false;
	}
	setvbuf(metFile.m_pStream, NULL, _IOFBF, 16384);

	try{
		version = metFile.ReadUInt8();
		
		if (version != PARTFILE_VERSION && version != PARTFILE_SPLITTEDVERSION && version != PARTFILE_VERSION_LARGEFILE){
			metFile.Close();
			if (version==83) {				
				return ImportShareazaTempfile(in_directory, in_filename,getsizeonly);
			}
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_BADMETVERSION), m_partmetfilename, GetFileName());
			return false;
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
				LoadHashsetFromFile(&metFile, false);
			}
			else {
				uchar gethash[16];
				metFile.Seek(2, CFile::begin);
				LoadDateFromFile(&metFile);
				metFile.Read(gethash, 16);
				md4cpy(m_abyFileHash, gethash);
			}
		}
		else {
			LoadDateFromFile(&metFile);
			LoadHashsetFromFile(&metFile, false);
		}

		LoadTagsFromTempFile(&metFile, getsizeonly, isnewstyle); // NEO: VOODOO - [UniversalPartfileInterface] <-- Xanatos --

		// load the hashsets from the hybridstylepartmet
		if (isnewstyle && !getsizeonly && (metFile.GetPosition()<metFile.GetLength()) ) {
			uint8 temp;
			metFile.Read(&temp,1);
			
			UINT parts = GetPartCount();	// assuming we will get all hashsets
			
			for (UINT i = 0; i < parts && (metFile.GetPosition() + 16 < metFile.GetLength()); i++){
				uchar* cur_hash = new uchar[16];
				metFile.Read(cur_hash, 16);
				hashlist.Add(cur_hash);
			}

			uchar* checkhash= new uchar[16];
			if (!hashlist.IsEmpty()){
				uchar* buffer = new uchar[hashlist.GetCount()*16];
				for (int i = 0; i < hashlist.GetCount(); i++)
					md4cpy(buffer+(i*16), hashlist[i]);
				CreateHash(buffer, hashlist.GetCount()*16, checkhash);
				delete[] buffer;
			}
			bool flag = false;
			if (!md4cmp(m_abyFileHash, checkhash))
				flag = true;
			else{
				for (int i = 0; i < hashlist.GetSize(); i++)
					delete[] hashlist[i];
				hashlist.RemoveAll();
				flag = false;
			}
			delete[] checkhash;
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
		return false;
	}
#ifndef _DEBUG
	catch(...){
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_METCORRUPT), m_partmetfilename, GetFileName());
		ASSERT(0);
		return false;
	}
#endif

	if (m_nFileSize > (uint64)MAX_EMULE_FILE_SIZE) {
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_FILEERROR), m_partmetfilename, GetFileName(), _T("File size exceeds supported limit"));
		return false;
	}

	if (getsizeonly) {
		// AAARGGGHH!!!....
		return (uint8)partmettype;
	}

	// NEO: VOODOO - [UniversalPartfileInterface] <-- Xanatos --
	// Now to flush the map into the list (Slugfiller)
	/*for (POSITION pos = gap_map.GetStartPosition(); pos != NULL; ){
		Gap_Struct* gap;
		UINT gapkey;
		gap_map.GetNextAssoc(pos, gapkey, gap);
		// SLUGFILLER: SafeHash - revised code, and extra safety
		if (gap->start != -1 && gap->end != -1 && gap->start <= gap->end && gap->start < m_nFileSize){
			if (gap->end >= m_nFileSize)
				gap->end = m_nFileSize - (uint64)1; // Clipping
			AddGap(gap->start, gap->end); // All tags accounted for, use safe adding
		}
		delete gap;
		// SLUGFILLER: SafeHash
	}*/

	// verify corrupted parts list
	POSITION posCorruptedPart = corrupted_list.GetHeadPosition();
	while (posCorruptedPart)
	{
		POSITION posLast = posCorruptedPart;
		UINT uCorruptedPart = corrupted_list.GetNext(posCorruptedPart);
		if (IsComplete((uint64)uCorruptedPart*PARTSIZE, (uint64)(uCorruptedPart+1)*PARTSIZE-1, true))
			corrupted_list.RemoveAt(posLast);
	}

	LoadNeoFile(); // NEO: FCFG - [FileConfiguration] <-- Xanatos --

	InitHL(); // NEO: GSL - [GlobalSourceLimit] <-- Xanatos --

	//check if this is a backup
	if(_tcsicmp(_tcsrchr(m_fullname, _T('.')), PARTMET_TMP_EXT) == 0){
		m_fullname = RemoveFileExtension(m_fullname);
		m_partmetfilename = RemoveFileExtension(m_partmetfilename); // BEGIN SLUGFILLER: SafeHash - also update the partial name // NEO: SSH - [SlugFillerSafeHash] <-- Xanatos --
	}

	// open permanent handle
	CString searchpath(RemoveFileExtension(m_fullname));
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
  if(!IsVoodooFile()){
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
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
		return false;
	}

	// read part file creation time
	struct _stat fileinfo;
	if (_tstat(searchpath, &fileinfo) == 0){
		m_tLastModified = fileinfo.st_mtime;
		m_tCreated = fileinfo.st_ctime;
	}
	else
		AddDebugLogLine(false, _T("Failed to get file date for \"%s\" - %s"), searchpath, _tcserror(errno));
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
  }
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

	try{
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
      if(!IsVoodooFile()){
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
		SetFilePath(searchpath);
		m_dwFileAttributes = GetFileAttributes(GetFilePath());
		if (m_dwFileAttributes == INVALID_FILE_ATTRIBUTES)
			m_dwFileAttributes = 0;

		// SLUGFILLER: SafeHash - final safety, make sure any missing part of the file is gap
		if (m_hpartfile.GetLength() < m_nFileSize)
			AddGap(m_hpartfile.GetLength(), m_nFileSize - (uint64)1);
		// Goes both ways - Partfile should never be too large
		if (m_hpartfile.GetLength() > m_nFileSize){
			TRACE(_T("Partfile \"%s\" is too large! Truncating %I64u bytes.\n"), GetFileName(), m_hpartfile.GetLength() - m_nFileSize);
			m_hpartfile.SetLength(m_nFileSize);
		}
		// SLUGFILLER: SafeHash
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	  }
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

		// NEO: SSH - [SlugFillerSafeHash] -- Xanatos -->
		// the important part
		m_PartsShareable.SetSize(GetPartCount());
		for (UINT i = 0; i < GetPartCount();i++)
			m_PartsShareable[i] = false;
		// NEO: SSH END <-- Xanatos --

		m_SrcpartFrequency.SetSize(GetPartCount());
		m_SrcincpartFrequency.SetSize(GetPartCount()); // NEO: ICS - [InteligentChunkSelection] <-- Xanatos --
		for (UINT i = 0; i < GetPartCount();i++){
			m_SrcpartFrequency[i] = 0;
			m_SrcincpartFrequency[i] = 0; // NEO: ICS - [InteligentChunkSelection] <-- Xanatos --
		}

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
      if(IsVoodooFile()){
		SetStatus(PS_PAUSED);
		hashsetneeded = (GetHashCount() != GetED2KPartHashCount());
	  }else{
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
		SetStatus(PS_EMPTY);
		// check hashcount, filesatus etc
		if (GetHashCount() != GetED2KPartHashCount()){
			ASSERT( hashlist.GetSize() == 0 );
			hashsetneeded = true;
			return true;
		}
		else {
			hashsetneeded = false;
			for (UINT i = 0; i < (UINT)hashlist.GetSize(); i++){
				if (i < GetPartCount() && IsComplete((uint64)i*PARTSIZE, (uint64)(i + 1)*PARTSIZE - 1, true)){
					SetStatus(PS_READY);
					break;
				}
			}
		}

		VerifyIncompleteParts(true); // NEO: SCV - [SubChunkVerification] <-- Xanatos --
		// NEO: SCV - [SubChunkVerification] -- Xanatos -->
		// Note: for obtimal compatybility we need at least one complete part
		/*if (VerifyIncompleteParts(true) && status == PS_EMPTY){
			SetStatus(PS_READY);
			// NEO: SAFS - [ShowAllFilesInShare]
			//if (theApp.emuledlg->IsRunning())	// may be called during shutdown!
			//	theApp.sharedfiles->SafeAddKFile(this);
		}*/
		// NEO: SCV END <-- Xanatos --

		if (gaplist.IsEmpty()){	// is this file complete already?
			// NEO: POFC - [PauseOnFileComplete] -- Xanatos -->
			if(NeoPrefs.IsPauseOnFileComplete())
				m_bCompletionBreak = true;
			else
			// NEO: POFC END <-- Xanatos --
				CompleteFile(false);
			return true;
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
				/*SetStatus(PS_WAITINGFORHASH);
				CAddFileThread* addfilethread = (CAddFileThread*) AfxBeginThread(RUNTIME_CLASS(CAddFileThread), THREAD_PRIORITY_NORMAL,0, CREATE_SUSPENDED);
				if (addfilethread){
					SetFileOp(PFOP_HASHING);
					SetFileOpProgress(0);
					addfilethread->SetValues(0, GetPath(), m_hpartfile.GetFileName(), this);
					addfilethread->ResumeThread();
				}
				else
					SetStatus(PS_ERROR);*/
				SetSinglePartHash((uint16)-1); // NEO: SSH - [SlugFillerSafeHash] <-- Xanatos --
		    }
			// NEO: SSH - [SlugFillerSafeHash] -- Xanatos -->
			else {
				for (UINT i = 0; i < GetPartCount(); i++)
					if (IsComplete((uint64)i*PARTSIZE,(uint64)(i+1)*PARTSIZE-1, false))
						m_PartsShareable[i] = true;
			}
			// NEO: SSH END <-- Xanatos --
		}
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	  }
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
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
		return false;
	}

	UpdateCompletedInfos();
	return true;
}

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
bool CPartFile::LoadFromTempFile(CFileDataIO* file){
	// SLUGFILLER: SafeHash - load first, verify later
	bool ret1 = LoadDateFromFile(file);
	bool ret2 = LoadHashsetFromFile(file,false);
	bool ret3 = LoadTagsFromTempFile(file);

	// BEGIN SLUGFILLER: SafeHash // NEO: SSH - [SlugFillerSafeHash]
	// David: Note the current implementation of SlugFiller's SafeHash is causing an incompatybility for < 1 chunk files, 
	// this part will return us the compatybility
	if (GetED2KPartHashCount() == 0 && hashlist.GetSize() != 0) {
		ASSERT(hashlist.GetSize() == 1);
		for (int i = 0; i < hashlist.GetSize(); i++)
			delete[] hashlist[i];
		hashlist.RemoveAll();
	} 
	// END SLUGFILLER: SafeHash // NEO: SSH END

	//UpdatePartsInfo();
	if(!ret1 || !ret2 || !ret3)
		return false;
	// SLUGFILLER: SafeHash


	// NEO: SSH - [SlugFillerSafeHash]
	// the important part
	m_PartsShareable.SetSize(GetPartCount());
	for (UINT i = 0; i < GetPartCount();i++)
		m_PartsShareable[i] = false;
	// NEO: SSH END

	m_SrcpartFrequency.SetSize(GetPartCount());
	m_SrcincpartFrequency.SetSize(GetPartCount()); // NEO: ICS - [InteligentChunkSelection]
	for (UINT i = 0; i < GetPartCount();i++){
		m_SrcpartFrequency[i] = 0;
		m_SrcincpartFrequency[i] = 0; // NEO: ICS - [InteligentChunkSelection]
	}

	SetStatus(PS_EMPTY);
	// check hashcount, filesatus etc
	if (GetHashCount() != GetED2KPartHashCount()){
		ASSERT( hashlist.GetSize() == 0 );
		hashsetneeded = true;
		return true;
	}

	/*Gap_Struct* gap = new Gap_Struct;
	gap->start = 0;
	gap->end = m_nFileSize - (uint64)1;
	gaplist.AddTail(gap);*/

	hashsetneeded = false;
	if (status == PS_EMPTY){
		for (int i = 0; i < hashlist.GetSize(); i++){
			if (i < GetPartCount() && IsComplete(i*PARTSIZE, (i + 1)*PARTSIZE - 1, true)){
				SetStatus(PS_READY);
				break;
			}
		}
	}

	return true;
}
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

// NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
bool CPartFile::LoadTagsFromTempFile(CFileDataIO* fileptr, bool getsizeonly, bool isnewstyle)
{
	CFileDataIO &metFile = *fileptr;

	CMap<UINT, UINT, Gap_Struct*, Gap_Struct*> gap_map;
// NEO: VOODOO END <-- Xanatos --

		UINT tagcount = metFile.ReadUInt32();
		for (UINT j = 0; j < tagcount; j++){
			CTag* newtag = new CTag(&metFile, false);
			if (!getsizeonly || (getsizeonly && (newtag->GetNameID()==FT_FILESIZE || newtag->GetNameID()==FT_FILENAME))){
			    switch (newtag->GetNameID()){
				    case FT_FILENAME:{
					    if (!newtag->IsStr()) {
						    LogError(LOG_STATUSBAR, GetResString(IDS_ERR_METCORRUPT), m_partmetfilename, GetFileName());
						    delete newtag;
						    return false;
					    }
						//if (GetFileName().IsEmpty())
						if (GetFileName(true).IsEmpty()) // NEO: PP - [PasswordProtection] <-- Xanatos --
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
							SetCategory(newtag->GetInt(),2); // NEO: MOD - [SetCategory] <-- Xanatos --
							//m_category = newtag->GetInt();
					    delete newtag;
					    break;
				    }
					case FT_MAXSOURCES: {
						ASSERT( newtag->IsInt() );
						// NEO: SRT - [SourceRequestTweaks] -- Xanatos --
						//if (newtag->IsInt())
						//	m_uMaxSources = newtag->GetInt();
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
					// NEO: MOD - [FT_STATUS_EX] -- Xanatos -->
				    case FT_STATUS_EX:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt()){
							uint32 status = newtag->GetInt(); 
							// NEO: OCF - [OnlyCompleetFiles]
							if (status == 0)
								forced = true;
							// NEO: OCF END
							// NEO: SC - [SuspendCollecting]
							if (status & 1)
								suspend = true;
							// NEO: SC END
							// NEO: SD - [StandByDL]
							if (status & 2)
								standby = true;
							// NEO: SD END
						}
					    delete newtag;
					    break;
				    }
					// NEO: MOD END <-- Xanatos --
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
					// NEO: SRS - [SmartReleaseSharing] -- Xanatos -->
					case FT_RELEASE:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt())
							SetReleasePriority((uint8)newtag->GetInt(),false);
						delete newtag;
						break;
					}
					// NEO: SRS END <-- Xanatos --
				    case FT_KADLASTPUBLISHSRC:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt())
						{
						    SetLastPublishTimeKadSrc(newtag->GetInt(), 0);
							if(GetLastPublishTimeKadSrc() > (uint32)time(NULL)+KADEMLIAREPUBLISHTIMES)
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
                        if(newtag->GetInt() == 1) {
                            SetPreviewPrio(true);
                        } else {
                            SetPreviewPrio(false);
                        }
                        delete newtag;
                        break;
                    }
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
 					case FT_VOODOO_FILE:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt() && (newtag->GetInt() != FALSE))
							m_isVoodooFile = true;
						delete newtag;
						break;
					}
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

				   // statistics
					case FT_ATTRANSFERRED:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt())
							statistic.alltimetransferred = newtag->GetInt();
						delete newtag;
						break;
					}
					case FT_ATTRANSFERREDHI:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt())
						{
							uint32 hi,low;
							low = (UINT)statistic.alltimetransferred;
							hi = newtag->GetInt();
							uint64 hi2;
							hi2=hi;
							hi2=hi2<<32;
							statistic.alltimetransferred=low+hi2;
						}
						delete newtag;
						break;
					}
					case FT_ATREQUESTED:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt())
							statistic.alltimerequested = newtag->GetInt();
						delete newtag;
						break;
					}
 					case FT_ATACCEPTED:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt())
							statistic.alltimeaccepted = newtag->GetInt();
						delete newtag;
						break;
					}

					// NEO: SSP - [ShowSharePermissions] -- Xanatos -->
					case FT_PERMISSIONS:
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt()){
							int iPermissions = newtag->GetInt();
							if (iPermissions != PERM_ALL && iPermissions != PERM_FRIENDS && iPermissions != PERM_NONE && iPermissions != PERM_DEFAULT)
								iPermissions = PR_NORMAL;
							SetPermissions((uint8)iPermissions);
						}
						delete newtag;
						break;
					// NEO: SSP END <-- Xanatos --
					// old tags: as long as they are not needed, take the chance to purge them
					//case FT_PERMISSIONS:
					//	ASSERT( newtag->IsInt() );
					//	delete newtag;
					//	break;
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
							m_pAICHHashSet->SetMasterHash(hash, AICH_VERIFIED);
						else
							ASSERT( false );
						delete newtag;
						break;
					}
					// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
					case FT_CATRESUMEORDER:{
						ASSERT( newtag->IsInt() );
						m_catResumeOrder = newtag->GetInt();
						delete newtag;
						break;
					}
					// NEO: NXC END <-- Xanatos --
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

// NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->

	// Now to flush the map into the list (Slugfiller)
	for (POSITION pos = gap_map.GetStartPosition(); pos != NULL; ){
		Gap_Struct* gap;
		UINT gapkey;
		gap_map.GetNextAssoc(pos, gapkey, gap);
		// SLUGFILLER: SafeHash - revised code, and extra safety
		if (gap->start != -1 && gap->end != -1 && gap->start <= gap->end && gap->start < m_nFileSize){
			if (gap->end >= m_nFileSize)
				gap->end = m_nFileSize - (uint64)1; // Clipping
			AddGap(gap->start, gap->end); // All tags accounted for, use safe adding
		}
		delete gap;
		// SLUGFILLER: SafeHash
	}

	return true;
}
// NEO: VOODOO END<-- Xanatos --

bool CPartFile::SavePartFile()
{

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	if(m_fullname.IsEmpty()) // keep voodoo files completly virtual
		return true;
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

	//MORPH START - Added by SiRoB, Flush Thread // NEO: FFT - [FileFlushThread] -- Xanatos -->
	if (m_FlushSetting)
		return false;	
	//MORPH END - Added by SiRoB, Flush Thread // NEO: FFT END <-- Xanatos --

	// NEO: SSH - [SlugFillerSafeHash] -- Xanatos --
	//switch (status){
	//	case PS_WAITINGFORHASH:
	//	case PS_HASHING:
	//		return false;
	//}

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
  if(!IsVoodooFile()){
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
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

	if (!GetPartsHashing()){ // BEGIN SLUGFILLER: SafeHash - don't update the file date unless all parts are hashed // NEO: SSH - [SlugFillerSafeHash] <-- Xanatos --
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
	}

	ff.Close();
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
  }
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

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

		WriteToTempFile(&file); // NEO: VOODOO - [UniversalPartfileInterface] <-- Xanatos --

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
	if (!::CopyFile(m_fullname, BAKName, FALSE)){
		if (thePrefs.GetVerbose())
			DebugLogError(_T("Failed to create backup of %s (%s) - %s"), m_fullname, GetFileName(), GetErrorMessage(GetLastError()));
	}

	SaveNeoFile(); // NEO: FCFG - [FileConfiguration] <-- Xanatos --

	return true;
}

// NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
void CPartFile::WriteToTempFile(CFileDataIO* fileptr)
{
	CFileDataIO &file = *fileptr;
// NEO: VOODOO END <-- Xanatos --

		//date
		file.WriteUInt32(m_tUtcLastModified);

		//hash
		file.WriteHash16(m_abyFileHash);
		UINT parts = hashlist.GetCount();
		file.WriteUInt16((uint16)parts);
		for (UINT x = 0; x < parts; x++)
			file.WriteHash16(hashlist[x]);

		UINT uTagCount = 0;
		ULONG uTagCountFilePos = (ULONG)file.GetPosition();
		file.WriteUInt32(uTagCount);

		//if (WriteOptED2KUTF8Tag(&file, GetFileName(), FT_FILENAME))
		if (WriteOptED2KUTF8Tag(&file, GetFileName(true), FT_FILENAME)) // NEO: PP - [PasswordProtection] <-- Xanatos --
			uTagCount++;
		//CTag nametag(FT_FILENAME, GetFileName());
		CTag nametag(FT_FILENAME, GetFileName(true)); // NEO: PP - [PasswordProtection] <-- Xanatos --
		nametag.WriteTagToFile(&file);
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

		// NEO: MOD - [FT_STATUS_EX] -- Xanatos -->
		if(forced || suspend || standby){
			uint32 status = 0;
			if(suspend) // NEO: SC - [SuspendCollecting]
				status |= 1; 
			if(standby) // NEO: SD - [StandByDL]
				status |= 2;
			CTag statustag(FT_STATUS_EX, status);
			statustag.WriteTagToFile(&file);
			uTagCount++;
		}
		// NEO: MOD END <-- Xanatos --

		CTag prioritytag(FT_DLPRIORITY, IsAutoDownPriority() ? PR_AUTO : m_iDownPriority);
		prioritytag.WriteTagToFile(&file);
		uTagCount++;

		CTag ulprioritytag(FT_ULPRIORITY, IsAutoUpPriority() ? PR_AUTO : GetUpPriority());
		ulprioritytag.WriteTagToFile(&file);
		uTagCount++;

		// NEO: SRS - [SmartReleaseSharing] -- Xanatos -->
		CTag releasetag(FT_RELEASE, IsReleasePriority());
		releasetag.WriteTagToFile(&file);
		uTagCount++;
		// NEO: SRS END <-- Xanatos --

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

		// NEO: SSP - [ShowSharePermissions] -- Xanatos -->
		CTag permtag(FT_PERMISSIONS, GetPermissions());
		permtag.WriteTagToFile(&file);
		uTagCount++;
		// NEO: SSP END <-- Xanatos --

		if (GetDlActiveTime()){
			CTag tagDlActiveTime(FT_DL_ACTIVE_TIME, GetDlActiveTime());
			tagDlActiveTime.WriteTagToFile(&file);
			uTagCount++;
		}

        if (GetPreviewPrio()){
            CTag tagDlPreview(FT_DL_PREVIEW, GetPreviewPrio() ? 1 : 0);
			tagDlPreview.WriteTagToFile(&file);
			uTagCount++;
		}

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
		if (m_isVoodooFile){
			CTag voodootag(FT_VOODOO_FILE, TRUE);
			voodootag.WriteTagToFile(&file);
			uTagCount++;
		}
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

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

		// NEO: SRT - [SourceRequestTweaks] -- Xanatos --
		//if (m_uMaxSources){
		//	CTag attag3(FT_MAXSOURCES, m_uMaxSources);
		//	attag3.WriteTagToFile(&file);
		//	uTagCount++;
		//}

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
		CSingleLock sLockA(&SCV_mut, TRUE); // NEO: SCV - [SubChunkVerification] <-- Xanatos --
		if (m_pAICHHashSet->HasValidMasterHash() && (m_pAICHHashSet->GetStatus() == AICH_VERIFIED)){
			CTag aichtag(FT_AICH_HASH, m_pAICHHashSet->GetMasterHash().GetString() );
			aichtag.WriteTagToFile(&file);
			uTagCount++;
		}
		sLockA.Unlock(); // NEO: SCV - [SubChunkVerification] <-- Xanatos --

		// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
		CTag catresumetag(FT_CATRESUMEORDER, m_catResumeOrder );
		catresumetag.WriteTagToFile(&file);
		uTagCount++;
		// NEO: NXC END <-- Xanatos --

		for (int j = 0; j < taglist.GetCount(); j++){
			if (taglist[j]->IsStr() || taglist[j]->IsInt()){
				taglist[j]->WriteTagToFile(&file);
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

		file.Seek(uTagCountFilePos, CFile::begin);
		file.WriteUInt32(uTagCount);
		//file.SeekToEnd();
		file.Seek(0, CFile::end); // NEO: VOODOO - [UniversalPartfileInterface] <-- Xanatos --

} // NEO: VOODOO - [UniversalPartfileInterface] <-- Xanatos --

void CPartFile::PartFileHashFinished(CKnownFile* result){
	newdate = true;
	bool errorfound = false;
	if (GetED2KPartHashCount()==0 || GetHashCount()==0){
		ASSERT( IsComplete(0, m_nFileSize - (uint64)1, true) == IsComplete(0, m_nFileSize - (uint64)1, false) );
		if (IsComplete(0, m_nFileSize - (uint64)1, false)){
			if (md4cmp(result->GetFileHash(), GetFileHash())){
				LogWarning(GetResString(IDS_ERR_FOUNDCORRUPTION), 1, GetFileName());
				AddGap(0, m_nFileSize - (uint64)1);
				errorfound = true;
			}
			else{
				if (GetED2KPartHashCount() != GetHashCount()){
					ASSERT( result->GetED2KPartHashCount() == GetED2KPartHashCount() );
					if (SetHashset(result->GetHashset()))
						hashsetneeded = false;
				}
			}
		}
	}
	else{
		for (UINT i = 0; i < (UINT)hashlist.GetSize(); i++){
			ASSERT( IsComplete((uint64)i*PARTSIZE, (uint64)(i + 1)*PARTSIZE - 1, true) == IsComplete((uint64)i*PARTSIZE, (uint64)(i + 1)*PARTSIZE - 1, false) );
			if (i < GetPartCount() && IsComplete((uint64)i*PARTSIZE, (uint64)(i + 1)*PARTSIZE - 1, false)){
				if (!(result->GetPartHash(i) && !md4cmp(result->GetPartHash(i), GetPartHash(i)))){
					LogWarning(GetResString(IDS_ERR_FOUNDCORRUPTION), i+1, GetFileName());
					AddGap((uint64)i*PARTSIZE, ((uint64)((uint64)(i + 1)*PARTSIZE - 1) >= m_nFileSize) ? ((uint64)m_nFileSize - 1) : ((uint64)(i + 1)*PARTSIZE - 1) );
					errorfound = true;
				}
			}
		}
	}
	if (!errorfound && result->GetAICHHashset()->GetStatus() == AICH_HASHSETCOMPLETE && status == PS_COMPLETING){
		// NEO: SCV - [SubChunkVerification] -- Xanatos -->
		// David: lets verify our our file with the AICH hash we have for it
		if(m_pAICHHashSet->GetStatus() != AICH_ERROR && m_pAICHHashSet->GetStatus() != AICH_EMPTY)
			if(result->GetAICHHashset()->GetMasterHash() != m_pAICHHashSet->GetMasterHash()){
				switch(m_pAICHHashSet->GetStatus()){
					case AICH_VERIFIED: 
						ModLog(LOG_ERROR,GetResString(IDS_X_AICH_HASH_MISSMATCH),GetFileName(), GetResString(IDS_X_AICH_HASH_VERIFYED), m_pAICHHashSet->GetMasterHash().GetString(), result->GetAICHHashset()->GetMasterHash().GetString()); 
						break;
					case AICH_TRUSTED: 	
						ModLog(LOG_WARNING,GetResString(IDS_X_AICH_HASH_MISSMATCH),GetFileName(), GetResString(IDS_X_AICH_HASH_TRUSTED), m_pAICHHashSet->GetMasterHash().GetString(), result->GetAICHHashset()->GetMasterHash().GetString()); 
						break;
					case AICH_UNTRUSTED: 
						ModLog(LOG_INFO, GetResString(IDS_X_AICH_HASH_MISSMATCH),GetFileName(), GetResString(IDS_X_AICH_HASH_UNTRUSTED), m_pAICHHashSet->GetMasterHash().GetString(), result->GetAICHHashset()->GetMasterHash().GetString()); 
						break;
				}	
			}else
				ModLog(LOG_SUCCESS, GetResString(IDS_X_AICH_HASH_MATCH),GetFileName(),m_pAICHHashSet->GetMasterHash().GetString()); 
		// NEO: SCV END <-- Xanatos --
		delete m_pAICHHashSet;
		m_pAICHHashSet = result->GetAICHHashset();
		result->SetAICHHashset(NULL);
		m_pAICHHashSet->SetOwner(this);
	}
	else if (status == PS_COMPLETING){
		AddDebugLogLine(false, _T("Failed to store new AICH Hashset for completed file %s"), GetFileName());
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
			gaplist.RemoveAt(pos2);
			delete cur_gap;
		}
		else if (cur_gap->start >= start && cur_gap->start <= end){// a part of this gap is in the new gap - extend limit and delete
			end = cur_gap->end;
			gaplist.RemoveAt(pos2);
			delete cur_gap;
		}
		else if (cur_gap->end <= end && cur_gap->end >= start){// a part of this gap is in the new gap - extend limit and delete
			start = cur_gap->start;
			gaplist.RemoveAt(pos2);
			delete cur_gap;
		}
		else if (start >= cur_gap->start && end <= cur_gap->end){// new gap is already inside this gap - return
			return;
		}
	}
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
		((CPartFile*)this)->m_BufferedData_list_Locker.Lock(); //MORPH - Added by SiRoB, Flush Thread // NEO: FFT - [FileFlushThread] <-- Xanatos --
		for (POSITION pos = m_BufferedData_list.GetHeadPosition();pos != 0;)
		{
			const PartFileBufferedData* cur_gap = m_BufferedData_list.GetNext(pos);
			if (cur_gap->data) //MORPH - Added by SiRoB, Flush Thread // NEO: FFT - [FileFlushThread] <-- Xanatos --
			if (   (cur_gap->start >= start          && cur_gap->end   <= end)
				|| (cur_gap->start >= start          && cur_gap->start <= end)
				|| (cur_gap->end   <= end            && cur_gap->end   >= start)
				|| (start          >= cur_gap->start && end            <= cur_gap->end)
			)	// should be equal to if (start <= cur_gap->end  && end >= cur_gap->start)
			{
				((CPartFile*)this)->m_BufferedData_list_Locker.Unlock(); //MORPH - Added by SiRoB, Flush Thread // NEO: FFT - [FileFlushThread] <-- Xanatos --
				return false;	
			}
		}
		((CPartFile*)this)->m_BufferedData_list_Locker.Unlock(); //MORPH - Added by SiRoB, Flush Thread // NEO: FFT - [FileFlushThread] <-- Xanatos --
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
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
		POSITION posLast = pos;
		Requested_Block_Struct* cur_block = requestedblocks_list.GetNext(pos);
		if(cur_block->timeout && cur_block->timeout < ::GetTickCount()) // if its a voodoo block and its timed out remove it
		{
			const_cast<CPartFile*>(this)->requestedblocks_list.RemoveAt(posLast);
			delete cur_block;
			continue;
		}
#else
		const Requested_Block_Struct* cur_block = requestedblocks_list.GetNext(pos);
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
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
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
		POSITION posLast = pos;
		Requested_Block_Struct* cur_block = requestedblocks_list.GetNext(pos);
		if(cur_block->timeout && cur_block->timeout < ::GetTickCount()) // if its a voodoo block and its timed out remove it
		{
			const_cast<CPartFile*>(this)->requestedblocks_list.RemoveAt(posLast);
			delete cur_block;
			continue;
		}
#else
		const Requested_Block_Struct* cur_block = requestedblocks_list.GetNext(pos);
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
        if ((start <= cur_block->EndOffset) && (end >= cur_block->StartOffset)) {
            if(start < cur_block->StartOffset) {
                end = cur_block->StartOffset - 1;
                if(start == end)
                    return false;
            }
			else if(end > cur_block->EndOffset) {
                start = cur_block->EndOffset + 1;
                if(start == end) {
                    return false;
                }
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
                if(start == end)
                    return false;
            }
			else if(end > cur_block->end) {
                start = cur_block->end + 1;
                if(start == end) {
                    return false;
                }
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

//bool CPartFile::GetNextEmptyBlockInPart(UINT partNumber, Requested_Block_Struct *result) const
bool CPartFile::GetNextEmptyBlockInPart(UINT partNumber, tBlockMap* blockmap, Requested_Block_Struct *result, uint32 blocksize) const // NEO: SCT - [SubChunkTransfer] // NEO: DBR - [DynamicBlockRequest] <-- Xanatos --
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
				//	firstGap = currentGap;
				// NEO: SCT - [SubChunkTransfer] -- Xanatos -->
				{
					if(blockmap){ // is this an incomplete chunk
						uint64 tmpEnd = 0; 
						uint64 tmpStart = 0;
						uint8 firstBlock = (uint8)((max(currentGap->start,partStart) - partStart)/EMBLOCKSIZE);
						uint8 lastBlock = (uint8)((min(currentGap->end,partEnd) - partStart)/EMBLOCKSIZE)+1;
						ASSERT(firstBlock < 53 && lastBlock <= 53);
						for(uint8 currentBlock = firstBlock; currentBlock < lastBlock; currentBlock++)
						{
							uint64 tmpBlock = (partNumber*PARTSIZE + ((uint64)currentBlock * EMBLOCKSIZE));
							if(blockmap->IsBlockDone(currentBlock) && tmpBlock >= start){
								if(tmpEnd == 0) // first complete block the client have
									tmpStart = tmpBlock;
								tmpEnd = tmpBlock + EMBLOCKSIZE;
							}
							else if(tmpEnd != 0) // we found some available blocks but the next one is not available, stop here
								break;
						}
						if(tmpEnd != 0){
							if(firstGap == NULL)
								firstGap = new Gap_Struct;
							firstGap->start = tmpStart;
							firstGap->end = tmpEnd;
							if(firstGap->end > currentGap->end)
								firstGap->start = currentGap->end;
						}
					}
					else{
						if(firstGap == NULL)
							firstGap = new Gap_Struct;
						firstGap->start = currentGap->start;
						firstGap->end = currentGap->end;
					}
				}
				// NEO: SCT END <-- Xanatos --
			}
		}

		// If no gaps after start, exit
		if (firstGap == NULL)
			return false;

		// Update start position if gap starts after current pos
		if (start < firstGap->start)
			start = firstGap->start;
		end = firstGap->end; // NEO: SCT - [SubChunkTransfer] <-- Xanatos -- // moved from below

		delete firstGap; // NEO: SCT - [SubChunkTransfer] <-- Xanatos --

		// If this is not within part, exit
		if (start > partEnd)
			return false;

		// Find end, keeping within the max block size and the part limit
		//end = firstGap->end;
		//blockLimit = partStart + (uint64)((UINT)(start - partStart)/EMBLOCKSIZE + 1)*EMBLOCKSIZE - 1;
		blockLimit = partStart + (uint64)((UINT)(start - partStart)/blocksize + 1)*blocksize - 1; // NEO: DBR - [DynamicBlockRequest] <-- Xanatos --
		if (end > blockLimit)
			end = blockLimit;
		if (end > partEnd)
			end = partEnd;
    
		// If this gap has not already been requested, we have found a valid entry
		if (!IsAlreadyRequested(start, end, true))
		{
			// Was this block to be returned
			if (result != NULL)
			{
				result->StartOffset = start;
				result->EndOffset = end;
				md4cpy(result->FileID, GetFileHash());
				result->transferred = 0;
				result->filedata = NULL; // NEO: RBT  - [ReadBlockThread] <-- Xanatos --
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
				result->timeout = 0;
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
			}
			return true;
		}
		else
		{
        	uint64 tempStart = start;
        	uint64 tempEnd = end;

            bool shrinkSucceeded = ShrinkToAvoidAlreadyRequested(tempStart, tempEnd);
            if(shrinkSucceeded) {
                AddDebugLogLine(false, _T("Shrunk interval to prevent collision with already requested block: Old interval %I64u-%I64u. New interval: %I64u-%I64u. File %s."), start, end, tempStart, tempEnd, GetFileName());

                // Was this block to be returned
			    if (result != NULL)
			    {
				    result->StartOffset = tempStart;
				    result->EndOffset = tempEnd;
				    md4cpy(result->FileID, GetFileHash());
				    result->transferred = 0;
					result->filedata = NULL; // NEO: RBT  - [ReadBlockThread] <-- Xanatos --
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
					result->timeout = 0;
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
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
			gaplist.RemoveAt(pos2);
			delete cur_gap;
		}
		else if (cur_gap->start >= start && cur_gap->start <= end){// a part of this gap is in the part - set limit
			cur_gap->start = end+1;
		}
		else if (cur_gap->end <= end && cur_gap->end >= start){// a part of this gap is in the part - set limit
			cur_gap->end = start-1;
		}
		else if (start >= cur_gap->start && end <= cur_gap->end){
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
   	uint64 allgaps = 0; 

	for (POSITION pos = gaplist.GetHeadPosition();pos !=  0;){ 
		const Gap_Struct* cur_gap = gaplist.GetNext(pos);
		allgaps += cur_gap->end - cur_gap->start + 1;
	}

	UpdateCompletedInfos(allgaps);
}

void CPartFile::UpdateCompletedInfos(uint64 uTotalGaps)
{
	if (uTotalGaps > m_nFileSize){
		ASSERT(0);
		uTotalGaps = m_nFileSize;
	}

	if (gaplist.GetCount() || requestedblocks_list.GetCount()){ 
		// 'percentcompleted' is only used in GUI, round down to avoid showing "100%" in case 
		// we actually have only "99.9%"
		percentcompleted = (float)(floor((1.0 - (double)uTotalGaps/(uint64)m_nFileSize) * 1000.0) / 10.0);
		if(percentcompletedinitial == 0) percentcompletedinitial = percentcompleted; // NEO: MOD - [Percentage] <-- Xanatos --
		completedsize = m_nFileSize - uTotalGaps;
	} 
	else{
		percentcompleted = 100.0F;
		completedsize = m_nFileSize;
	}
}

// NEO: MOD - [ClassicShareStatusBar]  -- Xanatos -->
void CPartFile::DrawClassicShareStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool bFlat) const
{
	if( !IsPartFile() )
	{
		CKnownFile::DrawShareStatusBar( dc, rect, onlygreyrect, bFlat );
		return;
	}

	const COLORREF crMissing = RGB(255, 0, 0);
	s_ChunkBar.SetFileSize(GetFileSize());
	s_ChunkBar.SetHeight(rect->bottom - rect->top);
	s_ChunkBar.SetWidth(rect->right - rect->left);
	s_ChunkBar.Fill(crMissing);

	if (!onlygreyrect && !m_SrcpartFrequency.IsEmpty()){
		COLORREF crProgress;
		COLORREF crHave;
		COLORREF crPending;
		if(bFlat) { 
			crProgress = RGB(0, 150, 0);
			crHave = RGB(0, 0, 0);
			crPending = RGB(255,208,0);
		} else { 
			crProgress = RGB(0, 224, 0);
			crHave = RGB(104, 104, 104);
			crPending = RGB(255, 208, 0);
		} 
		for (int i = 0; i < GetPartCount(); i++){
			if(m_SrcpartFrequency[i] > 0 ){
				COLORREF color = RGB(0, (210-(22*(m_SrcpartFrequency[i]-1)) < 0) ? 0 : 210-(22*(m_SrcpartFrequency[i]-1)), 255);
				s_ChunkBar.FillRange(PARTSIZE*(i),PARTSIZE*(i+1),color);
			}
		}
	}
   	s_ChunkBar.Draw(dc, rect->left, rect->top, bFlat); 
} 
// NEO: MOD END <-- Xanatos --

void CPartFile::DrawShareStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool bFlat) const
{
	if( !IsPartFile() )
	{
		CKnownFile::DrawShareStatusBar( dc, rect, onlygreyrect, bFlat );
		return;
	}

    const COLORREF crNotShared = RGB(224, 224, 224);
	s_ChunkBar.SetFileSize(GetFileSize());
	s_ChunkBar.SetHeight(rect->bottom - rect->top);
	s_ChunkBar.SetWidth(rect->right - rect->left);
	s_ChunkBar.Fill(crNotShared);

	if (!onlygreyrect){
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
		for (UINT i = 0; i < GetPartCount(); i++){
            if(IsComplete((uint64)i*PARTSIZE,((uint64)(i+1)*PARTSIZE)-1, true)) {
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
   	s_ChunkBar.Draw(dc, rect->left, rect->top, bFlat); 
} 

void CPartFile::DrawStatusBar(CDC* dc, LPCRECT rect, bool bFlat) /*const*/
{
	COLORREF crProgress;
	COLORREF crProgressBk;
	COLORREF crHave;
	COLORREF crPending;
	COLORREF crMissing;
	COLORREF crIncomplete; // NEO: SSH - [SlugFillerSafeHash] <-- Xanatos --
	COLORREF crHaveBlock; // NEO: SCV - [SubChunkVerification] <-- Xanatos --
	COLORREF crUnconfirmed; // NEO: MOD - [ConfirmedDownload] <-- Xanatos --
	COLORREF crDot; // NEO: MOD - [ChunkDots] <-- Xanatos --
	EPartFileStatus eVirtualState = GetStatus();
	bool notgray = eVirtualState == PS_EMPTY || eVirtualState == PS_READY;

	if (g_bLowColorDesktop)
	{
		bFlat = true;
		// use straight Windows colors
		crProgress = RGB(0, 255, 0);
		crProgressBk = RGB(192, 192, 192);
		if (notgray) {
			crIncomplete = RGB(196, 196, 196); // NEO: SSH - [SlugFillerSafeHash] <-- Xanatos --
			crHaveBlock = RGB(128, 128, 128); // NEO: SCV - [SubChunkVerification] <-- Xanatos --
			crUnconfirmed = RGB(255, 210, 0); // NEO: MOD - [ConfirmedDownload] <-- Xanatos --
			crDot = RGB(255, 255, 255); // NEO: MOD - [ChunkDots] <-- Xanatos --
			crMissing = RGB(255, 0, 0);
			crHave = RGB(0, 0, 0);
			crPending = RGB(255, 255, 0);
		} else {
			crIncomplete = RGB(196, 196, 196); // NEO: SSH - [SlugFillerSafeHash] <-- Xanatos --
			crHaveBlock = RGB(128, 128, 128); // NEO: SCV - [SubChunkVerification] <-- Xanatos --
			crUnconfirmed = RGB(255, 210, 0); // NEO: MOD - [ConfirmedDownload] <-- Xanatos --
			crDot = RGB(255, 255, 255); // NEO: MOD - [ChunkDots] <-- Xanatos --
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
			crIncomplete = RGB(196, 196, 196); // NEO: SSH - [SlugFillerSafeHash] <-- Xanatos --
			crHaveBlock = RGB(128, 128, 128); // NEO: SCV - [SubChunkVerification] <-- Xanatos --
			crUnconfirmed = RGB(255, 210, 0); // NEO: MOD - [ConfirmedDownload] <-- Xanatos --
			crDot = RGB(255, 255, 255); // NEO: MOD - [ChunkDots] <-- Xanatos --
			crMissing = RGB(255, 0, 0);
			if (bFlat) {
				crHave = RGB(0, 0, 0);
				crPending = RGB(255, 208, 0);
			} else {
				crHave = RGB(104, 104, 104);
				crPending = RGB(255, 208, 0);
			}
		} else {
			crIncomplete = RGB(140, 140, 140); // NEO: SSH - [SlugFillerSafeHash] <-- Xanatos --
			crHaveBlock = RGB(160, 160, 160); // NEO: SCV - [SubChunkVerification] <-- Xanatos --
			crUnconfirmed = RGB(255, 210, 0); // NEO: MOD - [ConfirmedDownload] <-- Xanatos --
			crDot = RGB(255, 255, 255); // NEO: MOD - [ChunkDots] <-- Xanatos --
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

	if (status == PS_COMPLETE || status == PS_COMPLETING)
	{
		s_ChunkBar.FillRange(0, m_nFileSize, crProgress);
		s_ChunkBar.Draw(dc, rect->left, rect->top, bFlat);
		percentcompleted = 100.0F;
		completedsize = m_nFileSize;
	}
	/*else if (eVirtualState == PS_INSUFFICIENT || status == PS_ERROR)
	{
		int iOldBkColor = dc->SetBkColor(RGB(255, 255, 0));
		if (theApp.m_brushBackwardDiagonal.m_hObject)
			dc->FillRect(rect, &theApp.m_brushBackwardDiagonal);
		else
			dc->FillSolidRect(rect, RGB(255, 255, 0));
		dc->SetBkColor(iOldBkColor);

		UpdateCompletedInfos();
	}*/
	// NEO: MOD - [EventCollors] -- Xanatos -->
	else if(eVirtualState == PS_INSUFFICIENT)
	{
		int iOldBkColor = dc->SetBkColor(RGB(255, 255, 0));
		if(theApp.m_brushBackwardDiagonal.m_hObject)
			dc->FillRect(rect, &theApp.m_brushBackwardDiagonal);
		else
			dc->FillSolidRect(rect, dc->GetBkColor());
		dc->SetBkColor(iOldBkColor);

		UpdateCompletedInfos();
	}
	else if(status == PS_ERROR)
	{
		int iOldBkColor = dc->SetBkColor(RGB(255, 170, 0));
		if(theApp.m_brushBackwardDiagonal.m_hObject)
			dc->FillRect(rect, &theApp.m_brushBackwardDiagonal);
		else
			dc->FillSolidRect(rect, dc->GetBkColor());
		dc->SetBkColor(iOldBkColor);

		UpdateCompletedInfos();
	}
	// NEO: MOD END <-- Xanatos --
	else
	{
		// NEO: SSH - [SlugFillerSafeHash] -- Xanatos -->
		for(UINT i = 0; i < GetPartCount(); i++){
			if(!IsPartShareable(i)){
				s_ChunkBar.FillRange((uint64)i*PARTSIZE, min((uint64)(i+1)*PARTSIZE - 1, GetFileSize()), crIncomplete);

				// NEO: SCV - [SubChunkVerification]
				tBlockMap* blockMap;
				if(GetBlockMap((uint16)i,&blockMap)){
					for (uint8 j = 0; j < 53; j++){
						if(blockMap->IsBlockDone(j))
							s_ChunkBar.FillRange((uint64)i*PARTSIZE+(uint64)j*EMBLOCKSIZE, min((uint64)i*PARTSIZE+(uint64)(j+1)*EMBLOCKSIZE - 1, GetFileSize()), crHaveBlock);
					}
				}
				// NEO: SCV END
			}
		}
		// NEO: SSH END <-- Xanatos --

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
							// NEO: MOD - [RelativeChunkDisplay] -- Xanatos -->
							uint16 uAvail = m_SrcpartFrequency[(uint16)i];
							if(NeoPrefs.UseRelativeChunkDisplay()){
								if(uAvail > m_nCompleteSourcesCount)
									uAvail = uAvail - m_nCompleteSourcesCount;
								else
									uAvail = 1;
							}
							// NEO: MOD END <-- Xanatso --
							if (notgray)
								color = RGB(0,
											(210 - 22*(uAvail - 1) <  0) ?  0 : 210 - 22*(uAvail - 1), // NEO: MOD - [RelativeChunkDisplay] <-- Xanatos --
											//(210 - 22*(m_SrcpartFrequency[(uint16)i] - 1) <  0) ?  0 : 210 - 22*(m_SrcpartFrequency[(uint16)i] - 1),
											255);
							else
								color = RGB(64,
											(169 - 11*(uAvail - 1) < 64) ? 64 : 169 - 11*(uAvail - 1), // NEO: MOD - [RelativeChunkDisplay] <-- Xanatos --
											//(169 - 11*(m_SrcpartFrequency[(uint16)i] - 1) < 64) ? 64 : 169 - 11*(m_SrcpartFrequency[(uint16)i] - 1),
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
	    //float blockpixel = (float)(rect->right - rect->left)/(float)m_nFileSize;
	    RECT gaprect;
	    gaprect.top = rect->top;
	    gaprect.bottom = gaprect.top + PROGRESS_HEIGHT;
	    gaprect.left = rect->left;

		// NEO: MOD - [ConfirmedDownload] -- Xanatos -->
		float	percentconfirmed;
		uint64  confirmedsize;
		if (gaplist.GetCount())	{
			// all this here should be done in the Process, not in the drawing!!! its a waste of cpu time, or maybe not :)
			UINT	completedParts=0;
			confirmedsize=0;
			for(UINT i=0; i<GetPartCount(); i++){
				uint64	end=(uint64)(i+1)*PARTSIZE-1;
				bool	lastChunk=false;
				if(end>m_nFileSize)	{
					end=m_nFileSize;
					lastChunk=true;
				}

				if(IsComplete((uint64)i*PARTSIZE, end, true))	{
					completedParts++;
					if(lastChunk==false)
						confirmedsize += (uint64)PARTSIZE;
					else
						confirmedsize += (uint64)m_nFileSize % PARTSIZE;
				}
			}
			percentconfirmed = (float)(floor(((double)confirmedsize/(uint64)m_nFileSize) * 1000.0) / 10.0);
		}else{
			percentconfirmed = 100.0F;
			confirmedsize = (uint64)m_nFileSize;
		}

		uint32	w=rect->right-rect->left+1;
		uint32	wc=(uint32)(percentconfirmed/100*w+0.5f);
		uint32	wp=(uint32)(percentcompleted/100*w+0.5f);
	    if(!bFlat) {
			// NEO: MOD - [ChunkDots] -- Xanatos -->
			if(NeoPrefs.UseChunkDots()){
				s_LoadBar.SetWidth(1);
				s_LoadBar.SetFileSize((uint64)1);
				s_LoadBar.Fill(crDot);
				for(ULONGLONG i=completedsize+PARTSIZE-((uint64)completedsize % PARTSIZE); i<m_nFileSize; i+=PARTSIZE)
					s_LoadBar.Draw(dc, gaprect.left+(int)((double)i*w/(uint64)m_nFileSize), gaprect.top, false);
			}
			// NEO: MOD END <-- Xanatos --
			s_LoadBar.SetWidth(wp);
			s_LoadBar.SetFileSize(completedsize);
			s_LoadBar.Fill(crUnconfirmed);
			s_LoadBar.FillRange(0, confirmedsize, crProgress);
		    s_LoadBar.Draw(dc, gaprect.left, gaprect.top, false);
	    } else {
			gaprect.right = rect->left+wc;
			dc->FillRect(&gaprect, &CBrush(crProgress));
			gaprect.left = gaprect.right;
			gaprect.right = rect->left+wp;
			dc->FillRect(&gaprect, &CBrush(crUnconfirmed));
		    //draw gray progress only if flat
		    gaprect.left = gaprect.right;
		    gaprect.right = rect->right;
		    dc->FillRect(&gaprect, &CBrush(crProgressBk));
			// NEO: MOD - [ChunkDots] -- Xanatos -->
			if(NeoPrefs.UseChunkDots()){
				for(uint64 i=completedsize+PARTSIZE-((uint64)completedsize % PARTSIZE); i<(uint64)m_nFileSize; i+=PARTSIZE){
					gaprect.left = gaprect.right = (LONG)(rect->left+(uint64)((float)i*w/(uint64)m_nFileSize));
					gaprect.right++;
					dc->FillRect(&gaprect, &CBrush(RGB(128,128,128)));
				}

			}
			// NEO: MOD END <-- Xanatos --
	    }
		// NEO: MOD END <-- Xanatos --
    
	    /*if (!bFlat) {
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
	    }*/

	    UpdateCompletedInfos(allgaps);
    }

	// additionally show any file op progress (needed for PS_COMPLETING and PS_WAITINGFORHASH)
	if (GetFileOp() != PFOP_NONE)
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

//void CPartFile::WritePartStatus(CSafeMemFile* file) const
void CPartFile::WritePartStatus(CSafeMemFile* file, CUpDownClient* client) const // NEO: IPS - [InteligentPartSharing] <-- Xanatos --
{
	// NEO: IPS - [InteligentPartSharing] -- Xanatos -->
	if (KnownPrefs->UseInteligentPartSharing())
		ReCalculateIPS();

	CClientFileStatus* status = client->GetFileStatus(this); // NEO: SCFS - [SmartClientFileStatus]

	CMap<UINT, UINT, BOOL, BOOL> HideMap;
	GetHideMap(status, HideMap);
	// NEO: IPS END <-- Xanatos --

	UINT uED2KPartCount = GetED2KPartCount();
	file->WriteUInt16((uint16)uED2KPartCount);
	
	UINT uPart = 0;
	while (uPart != uED2KPartCount)
	{
		uint8 towrite = 0;
		for (UINT i = 0; i < 8; i++)
		{
			if(IsPartShareable(uPart)) // NEO: SSH - [SlugFillerSafeHash] <-- Xanatos --
			//if (uPart < GetPartCount() && IsComplete((uint64)uPart*PARTSIZE, (uint64)(uPart + 1)*PARTSIZE - 1, true))
				if (HideMap[uPart] == FALSE) // NEO: IPS - [InteligentPartSharing] <-- Xanatos --
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
	/*int counter = 0;
	for (POSITION pos = srclist.GetHeadPosition(); pos != NULL;){
		EDownloadState nDLState = srclist.GetNext(pos)->GetDownloadState();
		if (nDLState==DS_ONQUEUE || nDLState==DS_DOWNLOADING || nDLState==DS_CONNECTED || nDLState==DS_REMOTEQUEUEFULL)
			++counter;
	}
	return counter;*/
	return m_anStates[DS_ONQUEUE]+m_anStates[DS_DOWNLOADING]+m_anStates[DS_CONNECTED]+m_anStates[DS_REMOTEQUEUEFULL]; // NEO: FIX - [SourceCount] <-- Xanatos --
}

UINT CPartFile::GetNotCurrentSourcesCount() const
{
	/*UINT counter = 0;
	for (POSITION pos = srclist.GetHeadPosition(); pos != NULL;){
		EDownloadState nDLState = srclist.GetNext(pos)->GetDownloadState();
		if (nDLState!=DS_ONQUEUE && nDLState!=DS_DOWNLOADING)
			counter++;
	}
	return counter;*/
	return GetSourceCount() - m_anStates[DS_DOWNLOADING] - m_anStates[DS_ONQUEUE] - m_anStates[DS_CACHED];  // NEO: FIX - [SourceCount] <-- Xanatos --
}

// NEO: FIX - [SourceCount] -- Xanatos -->
UINT CPartFile::GetInactiveSourceCount() const
{
	return m_anStates[DS_CACHED] // NEO: XSC - [ExtremeSourceCache]
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
	     + m_anStates[DS_LOADED]
#endif // NEO_SS // NEO: NSS END
								;
}

UINT CPartFile::GetAvailableSrcCount() const
{
	return m_anStates[DS_ONQUEUE]+m_anStates[DS_DOWNLOADING];
}
// NEO: FIX END <-- Xanatos --

uint64 CPartFile::GetNeededSpace() const
{
	if (m_hpartfile.GetLength() > GetFileSize())
		return 0;	// Shouldn't happen, but just in case
	return GetFileSize() - m_hpartfile.GetLength();
}

EPartFileStatus CPartFile::GetStatus(bool ignorepause) const
{
	if ((!paused && !insufficient) || status == PS_ERROR || status == PS_COMPLETING || status == PS_COMPLETE || status == PS_MOVING || ignorepause) // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
		return status;
	else if (paused)
		return PS_PAUSED;
	else
		return PS_INSUFFICIENT;
}

void CPartFile::AddDownloadingSource(CUpDownClient* client){
	POSITION pos = m_downloadingSourceList.Find(client); // to be sure
	if(pos == NULL){
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
		CEMSocket* socket = client->GetFileDownloadSocket();
		if (socket != NULL) {
			if(NeoPrefs.UseDownloadBandwidthThrottler())
			{
				theApp.downloadBandwidthThrottler->AddToStandardList(socket);
			}
 #ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
			socket->m_bDownloadSocket = true;
 #endif // NEO_BC // NEO: NBC END
		}
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
		//theApp.downloadqueue->IncreaseDownloadQueueLength();
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
		m_downloadingSourceList.AddTail(client);
		theApp.emuledlg->transferwnd->downloadclientsctrl.AddClient(client);
	}
}

void CPartFile::RemoveDownloadingSource(CUpDownClient* client){
	POSITION pos = m_downloadingSourceList.Find(client); // to be sure
	if(pos != NULL){
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
		CEMSocket* PCDownSocket = client->m_pPCDownSocket;
		if (PCDownSocket != NULL) {
			theApp.downloadBandwidthThrottler->RemoveFromStandardList(PCDownSocket);
 #ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
			PCDownSocket->m_bDownloadSocket = false;
 #endif // NEO_BC // NEO: NBC END
		}
		CEMSocket* socket = client->socket;
		if (socket != NULL) {
			theApp.downloadBandwidthThrottler->RemoveFromStandardList(socket);
 #ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
			socket->m_bDownloadSocket = false;
 #endif // NEO_BC // NEO: NBC END
		}
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
		theApp.downloadqueue->DecreaseDownloadQueueLength();
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
		m_downloadingSourceList.RemoveAt(pos);
		theApp.emuledlg->transferwnd->downloadclientsctrl.RemoveClient(client);
	}
}

uint32 CPartFile::Process(uint32 reducedownload, UINT icounter/*in percent*/)
{
	if (thePrefs.m_iDbgHeap >= 2)
		ASSERT_VALID(this);

	UINT nOldTransSourceCount = GetSrcStatisticsValue(DS_DOWNLOADING);
	DWORD dwCurTick = ::GetTickCount();

	// If buffer size exceeds limit, or if not written within time limit, flush data
	//if ((m_nTotalBufferData > thePrefs.GetFileBufferSize()) || (dwCurTick > (m_nLastBufferFlushTime + BUFFER_TIME_LIMIT)))
	if ((m_nTotalBufferData > thePrefs.GetFileBufferSize()) || (dwCurTick > (m_nLastBufferFlushTime + SEC2MS(thePrefs.GetFileBufferTime())))) // NEO: MOD - [BufferCustomisation] <-- Xanatos --
	{
		// Avoid flushing while copying preview file
		if (!m_bPreviewing)
			FlushBuffer();
	}

	// NEO: ASM - [AccurateSpeedMeasure] -- Xanatos --
//	datarate = 0;
//#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
//	landatarate = 0;
//#endif //LANCAST // NEO: NLC END <-- Xanatos --

	// calculate datarate, set limit etc.
	//if(icounter < 10) // NEO: MOD - [PartFileProcessObtimisation] <-- Xanatos --
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
					cur_src->CheckDownloadTimeout();
					cur_datarate = cur_src->CheckDownloadRate(15); // NEO: ASM - [AccurateSpeedMeasure] <-- Xanatos -- // look on the 15 last secunds
					//cur_datarate = cur_src->CalculateDownloadRate();
					//datarate+=cur_datarate;
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
					if(cur_src->IsLanClient()){ // dont count lan downlaod
//						landatarate+=cur_datarate;
						continue;
					}
#endif //LANCAST // NEO: NLC END <-- Xanatos --
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
					if(!NeoPrefs.UseDownloadBandwidthThrottler())
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --
					if(reducedownload)
					{
						uint32 limit = reducedownload*cur_datarate/1000;
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
						cur_src->socket->SetDownloadLimit(limit);
						if (cur_src->IsDownloadingFromPeerCache() && cur_src->m_pPCDownSocket && cur_src->m_pPCDownSocket->IsConnected())
							cur_src->m_pPCDownSocket->SetDownloadLimit(limit);
					}
					// NEO: MOD - [PartFileProcessObtimisation] -- Xanatos -->
					else{
						cur_src->socket->DisableDownloadLimit();
						if (cur_src->IsDownloadingFromPeerCache() && cur_src->m_pPCDownSocket && cur_src->m_pPCDownSocket->IsConnected())
							cur_src->m_pPCDownSocket->DisableDownloadLimit();
					}
					// NEO: MOD END <-- Xanatos --
				}
			}
		}
	}
	//else
	if(icounter == 10) // NEO: MOD - [PartFileProcessObtimisation] <-- Xanatos --
	{
		// NEO: FIX - [SourceCount] -- Xanatos --
		//bool downloadingbefore=m_anStates[DS_DOWNLOADING]>0;
		// -khaos--+++> Moved this here, otherwise we were setting our permanent variables to 0 every tenth of a second...
		//memset(m_anStates,0,sizeof(m_anStates));
		memset(src_stats,0,sizeof(src_stats));
		memset(net_stats,0,sizeof(net_stats));
		//UINT nCountForState;

		// NEO: SDT - [SourcesDropTweaks] -- Xanatos -->
		UINT uAverageQueueRank = 0; 
		UINT uOnQueueQueueCount = 0; 
		// NEO: SDT END <-- Xanatos --

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
		UINT uReaskedGroupeSize = 0;
#endif // NEO_SS // NEO: NSS END <-- Xanatos --

		for (POSITION pos = srclist.GetHeadPosition(); pos != NULL;)
		{
			CUpDownClient* cur_src = srclist.GetNext(pos);
			if (thePrefs.m_iDbgHeap >= 2)
				ASSERT_VALID( cur_src );

			// BEGIN -rewritten- refreshing statistics (no need for temp vars since it is not multithreaded)
			//nCountForState = cur_src->GetDownloadState();
			//special case which is not yet set as downloadstate
			//if (nCountForState == DS_ONQUEUE)
			//{
			//	if( cur_src->IsRemoteQueueFull() )
			//		nCountForState = DS_REMOTEQUEUEFULL;
			//}


			// this is a performance killer -> avoid calling 'IsBanned' for gathering stats
			//if (cur_src->IsBanned())
			//	nCountForState = DS_BANNED;
			//if (cur_src->GetUploadState() == US_BANNED) // not as accurate as 'IsBanned', but way faster and good enough for stats.
			//	nCountForState = DS_BANNED;

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

			// NEO: FIX - [SourceCount] -- Xanatos --
			//ASSERT( nCountForState < sizeof(m_anStates)/sizeof(m_anStates[0]) );
			//m_anStates[nCountForState]++;

			switch (cur_src->GetDownloadState())
			{
				case DS_DOWNLOADING:{
					ASSERT( cur_src->socket );
					if (cur_src->socket)
					{
						cur_src->CheckDownloadTimeout();
						// NEO: MOD - [PartFileProcessObtimisation] -- Xanatos --
						//uint32 cur_datarate = cur_src->CalculateDownloadRate();
						//datarate += cur_datarate;
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
						if(NeoPrefs.UseDownloadBandwidthThrottler())
						{
							// Should be only needed in case we dissable direct downlaod on runtime
							if(cur_src->socket && cur_src->socket->onDownQueue == false && (!cur_src->m_pPCDownSocket || cur_src->m_pPCDownSocket->onDownQueue == false))
							{
								CEMSocket* socket = cur_src->GetFileDownloadSocket();
								if (socket != NULL){
 #ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
									ASSERT(socket->m_bDownloadSocket);
 #endif // NEO_BC // NEO: NBC END
									theApp.downloadBandwidthThrottler->AddToStandardList(socket);
								}
							}
						}
						//else
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --
						// NEO: MOD - [PartFileProcessObtimisation] -- Xanatos --
						//if (reducedownload && cur_src->GetDownloadState() == DS_DOWNLOADING)
						//{
						//	uint32 limit = reducedownload*cur_datarate/1000; //(uint32)(((float)reducedownload/100)*cur_datarate)/10;		
						//	if (limit < 1000 && reducedownload == 200)
						//		limit += 1000;
						//	else if(limit<200 && cur_datarate == 0 && reducedownload >= 100)
						//		limit = 200;
						//	else if(limit<60 && cur_datarate < 600 && reducedownload >= 97)
						//		limit = 60;
						//	else if(limit<20 && cur_datarate < 200 && reducedownload >= 93)
						//		limit = 20;
						//	else if (limit < 1)
						//		limit = 1;
						//	cur_src->socket->SetDownloadLimit(limit);
						//	if (cur_src->IsDownloadingFromPeerCache() && cur_src->m_pPCDownSocket && cur_src->m_pPCDownSocket->IsConnected())
						//		cur_src->m_pPCDownSocket->SetDownloadLimit(limit);
						//}
						//else{
						//	cur_src->socket->DisableDownloadLimit();
						//	if (cur_src->IsDownloadingFromPeerCache() && cur_src->m_pPCDownSocket && cur_src->m_pPCDownSocket->IsConnected())
						//		cur_src->m_pPCDownSocket->DisableDownloadLimit();
						//}
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
							//if( ((dwCurTick - lastpurgetime) > SEC2MS(30)) && (this->GetSourceCount() >= (GetMaxSources()*.8 )) )
							//{
							//	theApp.downloadqueue->RemoveSource( cur_src );
							//	lastpurgetime = dwCurTick;
							//}

							// NEO: SDT - [SourcesDropTweaks] -- Xanatos -->
							if(PartPrefs->UseBadSourceDrop())
								TestAndDropSource(cur_src,PartPrefs->GetBadSourceDropMode(),PartPrefs->GetBadSourceDropTimeMs(),PartPrefs->GetBadSourceLimitMode(),GetBadSourceDropLimit(),1,lastBadSourcePurgeTime);
							// NEO: SDT END <-- Xanatos --
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
					//if( (dwCurTick - lastpurgetime) > SEC2MS(40) ){
					//	lastpurgetime = dwCurTick;
					//	// we only delete them if reaching the limit
					//	if (GetSourceCount() >= (GetMaxSources()*.8 )){
					//		theApp.downloadqueue->RemoveSource( cur_src );
					//		break;
					//	}			
					//}

					// NEO: SDT - [SourcesDropTweaks] -- Xanatos -->
					if(PartPrefs->UseNNPSourceDrop())
						if(TestAndDropSource(cur_src,PartPrefs->GetNNPSourceDropMode(),PartPrefs->GetNNPSourceDropTimeMs(),PartPrefs->GetNNPSourceLimitMode(),GetNNPSourceDropLimit(),PartPrefs->UseNNPSourceDrop(),lastNNPSourcePurgeTime))
						{
							break;
						}
					// NEO: SDT END <-- Xanatos --

					// doubled reasktime for no needed parts - save connections and traffic
                    if (cur_src->GetTimeUntilReask() > 0)
						break; 

                    cur_src->SwapToAnotherFile(_T("A4AF for NNP file. CPartFile::Process()"), true, false, false, NULL, true, true); // ZZ:DownloadManager
					// Recheck this client to see if still NNP.. Set to DS_NONE so that we force a TCP reask next time..
    				cur_src->SetDownloadState(DS_NONE);
					break;
				}
				// NEO: XSC - [ExtremeSourceCache] -- Xanatos -->
				case DS_CACHED:
				{
					if (GetMaxSources() > GetSourceCount()){
						if((time(NULL) - cur_src->GetLastSeen()) > (unsigned)PartPrefs->GetSourceCacheTime())
							theApp.downloadqueue->RemoveSource( cur_src );
						break;
					}
					cur_src->SetDownloadState(DS_NONE);
					break;
				}
				// NEO: XSC END <-- Xanatos --
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
				case DS_LOADED:
					if(dwCurTick - m_uLastLoadSource > PartPrefs->GetLoadedSourceCleanUpTimeMs()){ // if the source is not longer needed
						theApp.downloadqueue->RemoveSource( cur_src );
						break;
					}
					
					// Never reask low ID sources, the propobility for the same server/kad buddy to stay constant is near to 0
					if(cur_src->HasLowID()) 
						break;

					// check if we are allowed to reask more sources from storage
					if(GetSourceCount() > GetSourceStorageReaskSourceLimit()) // Note: sources in DS_LOADED are not counted in GetSourceCount or else ware
						break;

 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
					// When we are here the Neo source analyser already did his job and calculated propability of availibility and cached it.
					// He sorted the list by the calculated value and also removed all sources with propability 0 or below.
					// So the only think we have to do here is to manage the reask and the cleanup.

					if(NeoPrefs.EnableSourceAnalizer())
					{
						if(cur_src->Source() == NULL) // should not happen but in case
							break;

						// check if the source could be analysed
						if(cur_src->Source()->GetAnalisisQuality() > 0)
						{
							// check if the propability is high enough
							if(cur_src->Source()->GetAvalibilityProbability() < PartPrefs->GetReaskPropability())
								break;
						}
						else if(PartPrefs->UseUnpredictedPropability())
						{
							// check if the unpredicted propability is high enough
							if(cur_src->Source()->GetAvalibilityProbability() < PartPrefs->GetUnpredictedReaskPropability())
								break;
						}
						else
							break;
					}
 #endif // NEO_SA // NEO: NSA END

					// Delay the loading, wait for the network to give us enough sources by it self
					if(PartPrefs->GetAutoReaskStoredSourcesDelay() != 0 && dwCurTick - m_uLastLoadSource < PartPrefs->GetAutoReaskStoredSourcesDelayMs()) 
						break;

					// Don't set for reask to much sources at once
					// we should wait a bit for The network to get sources.
					if(PartPrefs->GroupStoredSourceReask()){
						if(dwCurTick - m_uLastReaskedGroupeTime > PartPrefs->GetStoredSourceGroupIntervalsMs()){
							if(uReaskedGroupeSize > (UINT)PartPrefs->GetStoredSourceGroupSize()){ // Do we reasked enough sources for this time frame
								uReaskedGroupeSize = 0;
								m_uLastReaskedGroupeTime = dwCurTick;
								break;
							}
							uReaskedGroupeSize++;
						}else
							break;
					}

					// set this source to be reasked during the next loop
					cur_src->SetDownloadState(DS_NONE);
					break;
#endif // NEO_SS // NEO: NSS END <-- Xanatos --
				case DS_REMOTEQUEUEFULL: // NEO: FIX - [SourceCount] <-- Xanatos --
				{
					// To Mods, please stop instantly removing these sources..
					// This causes sources to pop in and out creating extra overhead!
					//if( cur_src->IsRemoteQueueFull() ) 
					//{
					//	if( ((dwCurTick - lastpurgetime) > MIN2MS(1)) && (GetSourceCount() >= (GetMaxSources()*.8 )) )
					//	{
					//		theApp.downloadqueue->RemoveSource( cur_src );
					//		lastpurgetime = dwCurTick;
					//		break;
					//	}
					//}

					// NEO: SDT - [SourcesDropTweaks] -- Xanatos -->
					if(PartPrefs->UseFullQSourceDrop())
						if(TestAndDropSource(cur_src,PartPrefs->GetFullQSourceDropMode(),PartPrefs->GetFullQSourceDropTimeMs(),PartPrefs->GetFullQSourceLimitMode(),GetFullQSourceDropLimit(),PartPrefs->UseFullQSourceDrop(),lastFullQSourcePurgeTime))
						{
							break;
						}
					// NEO: SDT END <-- Xanatos --
				}
				case DS_ONQUEUE:
				{
					// NEO: SDT - [SourcesDropTweaks] -- Xanatos -->
					if(cur_src->GetRemoteQueueRank()){
						uAverageQueueRank += cur_src->GetRemoteQueueRank();
						uOnQueueQueueCount++;
					}

					if(PartPrefs->UseHighQSourceDrop() && IsHighQState(cur_src))
					{
						if(TestAndDropSource(cur_src,PartPrefs->GetHighQSourceDropMode(),PartPrefs->GetHighQSourceDropTimeMs(),PartPrefs->GetHighQSourceLimitMode(),GetHighQSourceDropLimit(),PartPrefs->UseHighQSourceDrop(),lastHighQSourcePurgeTime))
						{
							break;
						}
					}
					// NEO: SDT END <-- Xanatos --

					//Give up to 1 min for UDP to respond.. If we are within one min of TCP reask, do not try..
					//if (theApp.IsConnected() && cur_src->GetTimeUntilReask() < MIN2MS(2) && cur_src->GetTimeUntilReask() > SEC2MS(1) && ::GetTickCount()-cur_src->getLastTriedToConnectTime() > 20*60*1000) // ZZ:DownloadManager (one resk timestamp for each file)
					if(theApp.GetConState() // NEO: NCC - [NeoConnectionChecker] <-- Xanatos --
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
					  && !cur_src->IsLanClient() // don't use UDP reasl for lan
#endif //LANCAST // NEO: NLC END <-- Xanatos --
					 && cur_src->GetTimeUntilReask() < MIN2MS(2) && cur_src->GetTimeUntilReask() > SEC2MS(1) 
					 //&& ::GetTickCount()-cur_src->getLastTriedToConnectTime() > 20*60*1000 // ZZ:DownloadManager (one resk timestamp for each file)
					 && cur_src->IsLastTriedToConnectTimeOk(PartPrefs->GetSourceReaskTimeZZMs())) // NEO: DRT - [DownloadReaskTweaks] <-- Xanatos --
						cur_src->UDPReaskForDownload();

					// NEO: L2HAC - [LowID2HighIDAutoCallback] -- Xanatos -->
					if(cur_src->SupportsL2HAC() && cur_src->HasLowID() && (::GetTickCount()-cur_src->getLastTriedToConnectTime() < 59*60*1000))
						break; // we just wait till he connects us
					// NEO: L2HAC END <-- Xanatos --
				}
				case DS_CONNECTING:
				case DS_TOOMANYCONNS:
				case DS_TOOMANYCONNSKAD:
				case DS_NONE:
				case DS_CONNECTIONRETRY: // NEO: TCR - [TCPConnectionRetry] <-- Xanatos --
				case DS_HALTED: // NEO: SD - [StandByDL] <-- Xanatos --
				case DS_WAITCALLBACK:
				case DS_WAITCALLBACKKAD:
#ifdef NATTUNNELING // NEO: XSB - [XSBuddy] -- Xanatos -->
				case DS_WAITCALLBACKXS:
#endif //NATTUNNELING // NEO: XSB END <-- Xanatos --
				{
					//if (theApp.IsConnected() && cur_src->GetTimeUntilReask() == 0 && ::GetTickCount()-cur_src->getLastTriedToConnectTime() > 20*60*1000) // ZZ:DownloadManager (one resk timestamp for each file)
					if(theApp.GetConState() // NEO: NCC - [NeoConnectionChecker] <-- Xanatos --
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
					  && !cur_src->IsLanClient() // don't use UDP reasl for lan
#endif //LANCAST // NEO: NLC END <-- Xanatos --
					 && cur_src->GetTimeUntilReask() == 0 
					 //&& ::GetTickCount()-cur_src->getLastTriedToConnectTime() > 20*60*1000 // ZZ:DownloadManager (one resk timestamp for each file)
					 && cur_src->IsLastTriedToConnectTimeOk(PartPrefs->GetSourceReaskTimeZZMs())) // NEO: DRT - [DownloadReaskTweaks] <-- Xanatos --
					{
						if(!cur_src->AskForDownload()) // NOTE: This may *delete* the client!!
							break; //I left this break here just as a reminder just in case re rearange things..
					}
					break;
				}
			}
		}


		//memcpy(m_anStates,anStates,sizeof(anStates)); // NEO: FIX - [SourceCount] <-- Xanatos --

		// NEO: SDT - [SourcesDropTweaks] -- Xanatos -->
		// if the time had come and we do the dropping reset the time
		if(lastBadSourcePurgeTime == 0)
			lastBadSourcePurgeTime = dwCurTick;
		if(lastNNPSourcePurgeTime == 0)
			lastNNPSourcePurgeTime = dwCurTick;
		if(lastFullQSourcePurgeTime == 0)
			lastFullQSourcePurgeTime = dwCurTick;
		if(lastHighQSourcePurgeTime == 0)
			lastHighQSourcePurgeTime = dwCurTick;

		// average queue rank
		if(uOnQueueQueueCount)
			m_uAverageQueueRank = uAverageQueueRank/uOnQueueQueueCount;
		else
			m_uAverageQueueRank = 0;
		// NEO: SDT END <-- Xanatos --

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
		if(PartPrefs->AutoSaveSources() == TRUE  && dwCurTick - m_uLastSaveSource > PartPrefs->GetAutoSaveSourcesIntervalsMs())
			SaveSources();
#endif // NEO_SS // NEO: NSS END <-- Xanatos --

		m_bCollectingHalted = ( (PartPrefs->UseAutoSoftLock() && CheckSoftLock()) // NEO: ASL - [AutoSoftLock] <-- Xanatos --
			|| (NeoPrefs.UseConnectionControl() && theApp.listensocket->TooManySocketsOCC()) ); // NEO: OCC - [ObelixConnectionControl] <-- Xanatos --

		// NEO: AHL - [AutoHardLimit] -- Xanatos -->
		if(PartPrefs->UseAutoHardLimit() &&  dwCurTick - m_uLastAutoHardLimit > PartPrefs->GetAutoHardLimitTimeMs()){
			m_uLastAutoHardLimit = dwCurTick;
			CalculateAutoHardLimit();
		}
		// NEO: AHL END <-- Xanatos --

		// NEO: CSL - [CategorySourceLimit] -- Xanatos -->
		if(PartPrefs->UseCategorySourceLimit() &&  dwCurTick - m_uLastCategoryLimit > PartPrefs->GetGlobalSourceLimitTimeMs()){
			m_uLastCategoryLimit = dwCurTick;
			CalculateCategoryLimit();
		}
		// NEO: CSL END <-- Xanatos --

		// NEO: FIX - [SourceCount] -- Xanatos --
		//if (downloadingbefore!=(m_anStates[DS_DOWNLOADING]>0))
		//	NotifyStatusChange();
 
		//if( GetMaxSourcePerFileUDP() > GetSourceCount()){
		//	if (theApp.downloadqueue->DoKademliaFileRequest() && (Kademlia::CKademlia::GetTotalFile() < KADEMLIATOTALFILE) && (dwCurTick > m_LastSearchTimeKad) &&  Kademlia::CKademlia::IsConnected() && theApp.IsConnected() && !stopped){ //Once we can handle lowID users in Kad, we remove the second IsConnected
		// NEO: SRT - [SourceRequestTweaks] -- Xanatos -->
		if(GetKadSourceLimit() > GetSourceCount()){
			if (theApp.downloadqueue->DoKademliaFileRequest() 
			 && (Kademlia::CKademlia::GetTotalFile() < (unsigned)PartPrefs->GetKadMaxFiles()) 
			 && (dwCurTick > m_LastSearchTimeKad) 
			 &&  Kademlia::CKademlia::IsConnected() 
			 && PartPrefs->IsKadEnable() && !IsCollectingHalted() // NEO: XSC - [ExtremeSourceCache]
			 && !stopped){ //Once we can handle lowID users in Kad, we remove the second IsConnected
		// NEO: SRT END <-- Xanatos --
				//Kademlia
				theApp.downloadqueue->SetLastKademliaFileRequest();
				if (!GetKadFileSearchID())
				{
					Kademlia::CSearch* pSearch = Kademlia::CSearchManager::PrepareLookup(Kademlia::CSearch::FILE, true, Kademlia::CUInt128(GetFileHash()));
					if (pSearch)
					{
						pSearch->SetFileName(GetFileName()); // NEO: KII - [KadInterfaceImprovement] <-- Xanatos --

						//if(m_TotalSearchesKad < 7)
						if(m_TotalSearchesKad < PartPrefs->GetKadRepeatDelay()) // NEO: SRT - [SourceRequestTweaks] <-- Xanatos --
							m_TotalSearchesKad++;

						m_LastSearchTimeKad = dwCurTick + (KADEMLIAREASKTIME*m_TotalSearchesKad);
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

#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
		if (NeoPrefs.IsLancastEnabled()
		&& ((!m_LastLanSearchTime) || (dwCurTick - m_LastLanSearchTime) > /*LANSEARCHTIME*/ PartPrefs->GetLcIntervalsMs()) 
		 && !stopped 
		 && KnownPrefs->IsEnableLanCast()
		 && theApp.lancast->IsConnected()
		) // NEO: NST END <-- Xanatos --
		{
			m_LastLanSearchTime = ::GetTickCount();
			theApp.lancast->GetSources(this);
		}
#endif //LANCAST // NEO: NLC END <-- Xanatos --

		// check if we want new sources from server
		//if ( !m_bLocalSrcReqQueued && ((!m_LastSearchTime) || (dwCurTick - m_LastSearchTime) > SERVERREASKTIME) && theApp.serverconnect->IsConnected()
		//	&& GetMaxSourcePerFileSoft() > GetSourceCount() && !stopped
		//	&& (!IsLargeFile() || (theApp.serverconnect->GetCurrentServer() != NULL && theApp.serverconnect->GetCurrentServer()->SupportsLargeFilesTCP())))
		// NEO: SRT - [SourceRequestTweaks] -- Xanatos -->
		if ( !m_bLocalSrcReqQueued 
			&& ((!m_LastSearchTime) || (dwCurTick - m_LastSearchTime) > PartPrefs->GetSvrIntervalsMs()) 
			&& theApp.serverconnect->IsConnected()
			&& GetSvrSourceLimit() > GetSourceCount() 
			&& !stopped 
			&& PartPrefs->IsSvrEnable() && !IsCollectingHalted() // NEO: XSC - [ExtremeSourceCache]
			&& (!IsLargeFile() || (theApp.serverconnect->GetCurrentServer() != NULL && theApp.serverconnect->GetCurrentServer()->SupportsLargeFilesTCP()))
			)
		// NEO: SRT END <-- Xanatos --
		{
			m_bLocalSrcReqQueued = true;
			theApp.downloadqueue->SendLocalSrcRequest(this);
		}

		// NEO: SCV - [SubChunkVerification] -- Xanatos -->
		CSingleLock sLockA(&SCV_mut, TRUE); 
		if(m_uAICHVeirifcationPending  // not all blocks are verifyed yet we have to verify them ASAP 
		 && theApp.GetConState() // NEO: NCC - [NeoConnectionChecker]
		 && (m_pAICHHashSet->HasValidMasterHash() && (m_pAICHHashSet->GetStatus() == AICH_TRUSTED || m_pAICHHashSet->GetStatus() == AICH_VERIFIED)))
			VerifyIncompleteParts(); 
		sLockA.Unlock();
		// NEO: SCV END <-- Xanatos --

		count++;
		if (count == 3){
			count = 0;
			UpdateAutoDownPriority();
			UpdateDisplayedInfo();
			UpdateCompletedInfos();
		}
	}

	if ( GetSrcStatisticsValue(DS_DOWNLOADING) != nOldTransSourceCount ){
		// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
		int curselcat = theApp.emuledlg->transferwnd->downloadlistctrl.curTab;
		Category_Struct* cat = thePrefs.GetCategory(curselcat);
		if (cat && cat->viewfilters.nFromCats == 0)
			theApp.emuledlg->transferwnd->downloadlistctrl.ChangeCategory(curselcat);
		// NEO: NXC END <-- Xanatos --
		//if (theApp.emuledlg->transferwnd->downloadlistctrl.curTab == 0)
		//	theApp.emuledlg->transferwnd->downloadlistctrl.ChangeCategory(0); 
		else
			UpdateDisplayedInfo(true);
		if (thePrefs.ShowCatTabInfos() )
			theApp.emuledlg->transferwnd->UpdateCatTabTitles();
	}

	return datarate;
}

// NEO: ASM - [AccurateSpeedMeasure] -- Xanatos -->
uint32 CPartFile::CalculateDownloadRate()
{
	datarate = 0;
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	landatarate = 0;
#endif //LANCAST // NEO: NLC END <-- Xanatos --

	if(GetStatus() != PS_READY && GetStatus() != PS_EMPTY)
		return 0;

	uint32 cur_datarate;
	for(POSITION pos = m_downloadingSourceList.GetHeadPosition(); pos != NULL; ){
		CUpDownClient* cur_client = m_downloadingSourceList.GetNext(pos);
		ASSERT(cur_client->GetDownloadState() == DS_DOWNLOADING);
		cur_datarate = cur_client->CalculateDownloadRate();
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
		if(cur_client->IsLanClient()) // count lan datarate separatly
			landatarate+=cur_datarate;
		else
#endif //LANCAST // NEO: NLC END <-- Xanatos --
			datarate += cur_datarate;
	}

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	voodoodatarate = theApp.voodoo->GetDownDatarate(this);
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

	return datarate;
}
// NEO: ASM END <-- Xanatos --

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
bool CPartFile::CanAddSource(uint32 userid, uint16 port, uint32 serverip, uint16 serverport, UINT* pdebug_lowiddropped, bool Ed2kID, bool Low2Low)
#else
bool CPartFile::CanAddSource(uint32 userid, uint16 port, uint32 serverip, uint16 serverport, UINT* pdebug_lowiddropped, bool Ed2kID)
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
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
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
		// does this cleint support lowID 2 lowID
		if( Low2Low )
			return true;
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
		if (pdebug_lowiddropped)
			(*pdebug_lowiddropped)++;
		return false;
	}
	// MOD Note - end
	return true;
}

// NEO: CI#6 - [CodeImprovement] <-- Xanatos -->
bool CPartFile::CheckSourceID(uint32 dwID)
{
	// check the HighID(IP) - "Filter LAN IPs" and "IPfilter" the received sources IP addresses
	if (!IsLowID(dwID))
	{
		if (!IsGoodIP(dwID))
		{ 
			// check for 0-IP, localhost and optionally for LAN addresses
			if (thePrefs.GetLogFilteredIPs())
				AddDebugLogLine(false, _T("Ignored source (IP=%s) received via source exchange - bad IP"), ipstr(dwID));
			return false;
		}
		if (theApp.ipfilter->IsFiltered(dwID))
		{
			if (thePrefs.GetLogFilteredIPs())
				AddDebugLogLine(false, _T("Ignored source (IP=%s) received via source exchange - IP filter (%s)"), ipstr(dwID), theApp.ipfilter->GetLastHit());
			return false;
		}
		if (theApp.clientlist->IsBannedClient(dwID)){
#ifdef _DEBUG
			if (thePrefs.GetLogBannedClients()){
				CUpDownClient* pClient = theApp.clientlist->FindClientByIP(dwID);
				AddDebugLogLine(false, _T("Ignored source (IP=%s) received via source exchange - banned client %s"), ipstr(dwID), pClient->DbgGetClientInfo());
			}
#endif
			return false;
		}
	}

	return true;
}
// NEO: CI#6 END <-- Xanatos --

void CPartFile::AddSources(CSafeMemFile* sources, uint32 serverip, uint16 serverport, bool bWithObfuscationAndHash)
{
	UINT count = sources->ReadUInt8();

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
	bool Low2Low = theApp.serverconnect->CanLow2Low();
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --

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

		// NEO: CI#6 - [CodeImprovement] -- Xanatos -->
		if(!CheckSourceID(userid))
		{
			//if (thePrefs.GetLogFilteredIPs())
			//	AddDebugLogLine(false, _T("Ignored source (IP=%s) received from server"), ipstr(userid));
			continue;
		}
		// NEO: CI#6 END <-- Xanatos --

		// additionally check for LowID and own IP
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
		if (!CanAddSource(userid, port, serverip, serverport, &debug_lowiddropped, true, Low2Low))
#else
		if (!CanAddSource(userid, port, serverip, serverport, &debug_lowiddropped))
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
		{
			//if (thePrefs.GetLogFilteredIPs())
			//	AddDebugLogLine(false, _T("Ignored source (IP=%s) received from server"), ipstr(userid));
			continue;
		}

		//if( GetMaxSources() > this->GetSourceCount() )
		// NEO: XSC - [ExtremeSourceCache] -- Xanatos -->
		bool bCache = false;
		if ( (bCache = !(this->GetMaxSources() > this->GetSourceCount())) == false // NEO: SRT - [SourceRequestTweaks] <-- Xanatos --
			|| this->PartPrefs->UseSourceCache() && this->GetSourceCacheSourceLimit() > this->GetSrcStatisticsValue(DS_CACHED))
		// NEO: XSC END <-- Xanatos --
		{
			debug_possiblesources++;
			CUpDownClient* newsource = new CUpDownClient(this,port,userid,serverip,serverport,true);
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
			newsource->SetNatTraversalSupport(Low2Low, false, true);
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
			newsource->SetConnectOptions(byCryptOptions, true, false);
			if ((byCryptOptions & 0x80) != 0)
				newsource->SetUserHash(achUserHash);
			// NEO: XSC - [ExtremeSourceCache] -- Xanatos -->
			if(bCache)
				newsource->SetDownloadState(DS_CACHED);
			// NEO: XSC END <-- Xanatos --
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

// SLUGFILLER: heapsortCompletesrc
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
// SLUGFILLER: heapsortCompletesrc

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
	// NEO: MOD - [RelativeChunkDisplay] -- Xanatos -->
	if(m_nCompleteSourcesSize * 5 / 4 < GetAvailableSrcCount()) // if teh source amount havly changed force a recalc, also for the release prio
		flag = true;
	// NEO: MOD END <-- Xanatos --

	// Reset part counters
	if ((UINT)m_SrcpartFrequency.GetSize() < partcount){
		m_SrcpartFrequency.SetSize(partcount);
		m_SrcincpartFrequency.SetSize(partcount); // NEO: ICS - [InteligentChunkSelection] <-- Xanatos --
	}
	for (UINT i = 0; i < partcount; i++){
		m_SrcpartFrequency[i] = 0;
		m_SrcincpartFrequency[i] = 0; // NEO: ICS - [InteligentChunkSelection] <-- Xanatos --
	}

	CArray<uint16, uint16> count;
	if (flag)
		count.SetSize(0, srclist.GetSize());
	for (POSITION pos = srclist.GetHeadPosition(); pos != 0; )
	{
		CUpDownClient* cur_src = srclist.GetNext(pos);
		// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
		CClientFileStatus* status = cur_src->GetFileStatus(this);
		if(status == NULL || status->GetPartCount() != partcount)
			continue;
		uint8* abyUpPartStatus = status->GetPartStatus();
		if(abyUpPartStatus == NULL)
			continue;
		// NEO: SCFS END <-- Xanatos --
		//if( cur_src->GetPartStatus() )
		{		
			for (UINT i = 0; i < partcount; i++)
			{
				//if (cur_src->IsPartAvailable(i))
				if(abyUpPartStatus[i]) // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
					m_SrcpartFrequency[i] += 1;
			}
			if ( flag )
			{
				//count.Add(cur_src->GetUpCompleteSourcesCount());
				count.Add(status->GetCompleteSourcesCount()); // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
			}
		}
	}


	// NEO: MFSB - [MultiFileStatusBars] -- Xanatos -->
	for (POSITION pos = A4AFsrclist.GetHeadPosition(); pos != 0; )
	{
		CUpDownClient* cur_src = A4AFsrclist.GetNext(pos);
		// NEO: SCFS - [SmartClientFileStatus]
		CClientFileStatus* status = cur_src->GetFileStatus(this);
		if(status == NULL || status->GetPartCount() != partcount)
			continue;
		uint8* abyUpPartStatus = status->GetPartStatus();
		if(abyUpPartStatus == NULL)
			continue;
		// NEO: SCFS END
		//if( cur_src->GetPartStatus() )
		{		
			for (UINT i = 0; i < partcount; i++)
			{
				//if (cur_src->IsPartAvailable(i))
				if(abyUpPartStatus[i]) // NEO: SCFS - [SmartClientFileStatus]
					m_SrcpartFrequency[i] += 1;
			}
			if ( flag )
			{
				//count.Add(cur_src->GetUpCompleteSourcesCount());
				count.Add(status->GetCompleteSourcesCount()); // NEO: SCFS - [SmartClientFileStatus]
			}
		}
	}
	// NEO: MFSB END <-- Xanatos --

	if (flag)
	{
		m_nCompleteSourcesCount = m_nCompleteSourcesCountLo = m_nCompleteSourcesCountHi = 0;
	
		for (UINT i = 0; i < partcount; i++)
		{
			if (!i)
				m_nCompleteSourcesCount = m_SrcpartFrequency[i];
			else if( m_nCompleteSourcesCount > m_SrcpartFrequency[i])
				m_nCompleteSourcesCount = m_SrcpartFrequency[i];
		}
	
		count.Add(m_nCompleteSourcesCount);
	
		int n = count.GetSize();
		if (n > 0)
		{
			// SLUGFILLER: heapsortCompletesrc
			int r;
			for (r = n/2; r--; )
				HeapSort(count, r, n-1);
			for (r = n; --r; ){
				uint16 t = count[r];
				count[r] = count[0];
				count[0] = t;
				HeapSort(count, 0, r-1);
			}
			// SLUGFILLER: heapsortCompletesrc

			// calculate range
			int i = n >> 1;			// (n / 2)
			int j = (n * 3) >> 2;	// (n * 3) / 4
			int k = (n * 7) >> 3;	// (n * 7) / 8

			//When still a part file, adjust your guesses by 20% to what you see..

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
		m_nCompleteSourcesSize = GetAvailableSrcCount(); // NEO: MOD - [RelativeChunkDisplay] <-- Xanatos --
	}

	CalcRelease(flag); // NEO: SRS - [SmartReleaseSharing] <-- Xanatos --

	UpdateDisplayedInfo();
}	

// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
void CPartFile::UpdatePartsInfoEx(EPartStatus type)
{
	if( !IsPartFile() )
	{
		CKnownFile::UpdatePartsInfoEx(type);
		return;
	}

	CArray<uint16, uint16>* PartFrequency;
	switch(type)
	{
	case CFS_Incomplete: PartFrequency = &m_SrcincpartFrequency; break; // NEO: ICS - [InteligentChunkSelection]
	//case CFS_Hiden: PartFrequency = &m_SrchidenpartFrequency; break; // NEO: RPS - [RealPartStatus]
	//case CFS_Blocked: PartFrequency = &m_SrcblockedpartFrequency; break; // NEO: RPS - [RealPartStatus]
	default:
		ASSERT(0);
		return;
	}

	// Cache part count
	UINT partcount = GetPartCount();

	// Reset part counters
	if ((UINT)PartFrequency->GetSize() < partcount)
		PartFrequency->SetSize(partcount);
	for(UINT i = 0; i < partcount; i++)
		PartFrequency->GetAt(i) = 0;

	for (POSITION pos = srclist.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* cur_src = srclist.GetNext(pos);
		CClientFileStatus* status = cur_src->GetFileStatus(this);
		if(status == NULL || status->GetPartCount() != partcount)
			continue;
		uint8* abyUpPartStatus = status->GetPartStatus(type);
		if(abyUpPartStatus == NULL)
			continue;
		for (UINT i = 0; i < partcount; i++){
			if (abyUpPartStatus[i])
				PartFrequency->GetAt(i)+=1;
		}
	}

	// NEO: MFSB - [MultiFileStatusBars]
	for (POSITION pos = A4AFsrclist.GetHeadPosition(); pos != 0; )
	{
		CUpDownClient* cur_src = A4AFsrclist.GetNext(pos);
		CClientFileStatus* status = cur_src->GetFileStatus(this);
		if(status == NULL || status->GetPartCount() != partcount)
			continue;
		uint8* abyUpPartStatus = status->GetPartStatus(type);
		if(abyUpPartStatus == NULL)
			continue;
		for (UINT i = 0; i < partcount; i++){
			if (abyUpPartStatus[i])
				PartFrequency->GetAt(i)+=1;
		}
	}
	// NEO: MFSB END

	//UpdateDisplayedInfo(); // Not displayed
}
// NEO: SCFS END <-- Xanatos --

bool CPartFile::RemoveBlockFromList(uint64 start, uint64 end)
{
	ASSERT( start <= end );

	bool bResult = false;
	for (POSITION pos = requestedblocks_list.GetHeadPosition(); pos != NULL; ){
		POSITION posLast = pos;
		Requested_Block_Struct* block = requestedblocks_list.GetNext(pos);
		if (block->StartOffset <= start && block->EndOffset >= end){
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
			if(block->timeout != 0) // its a voodo block, not what we ware looking for, don't touch it!!!
				continue;

			// tell all voodoo cleints that we are not longer on this block
			if(!IsVoodooFile() && KnownPrefs->IsEnableVoodoo())
				theApp.voodoo->ManifestThrottleBlock(this,block->StartOffset,block->EndOffset,true);
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
			requestedblocks_list.RemoveAt(posLast);
			bResult = true;
		}
	}
	return bResult;
}

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
		theApp.emuledlg->transferwnd->downloadlistctrl.HideSources(this);
	
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	if(IsVoodooFile()){
		SetStatus(PS_COMPLETING);
		datarate = 0;
		landatarate = 0; // NEO: NLC - [NeoLanCast]
		voodoodatarate = 0;
		return;
	}
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

	if (!bIsHashingDone){
		SetStatus(PS_COMPLETING);
		datarate = 0;
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
		landatarate = 0;
#endif //LANCAST // NEO: NLC END <-- Xanatos --
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
		voodoodatarate = 0;
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
		CAddFileThread* addfilethread = (CAddFileThread*) AfxBeginThread(RUNTIME_CLASS(CAddFileThread), THREAD_PRIORITY_BELOW_NORMAL,0, CREATE_SUSPENDED);
		if (addfilethread){
			SetFileOp(PFOP_HASHING);
			SetFileOpProgress(0);
			TCHAR mytemppath[MAX_PATH];
			_tcscpy(mytemppath,m_fullname);
			mytemppath[ _tcslen(mytemppath)-_tcslen(m_partmetfilename)-1]=0;
			addfilethread->SetValues(0,mytemppath,RemoveFileExtension(m_partmetfilename),this);
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
	theApp.emuledlg->transferwnd->downloadlistctrl.ShowFilesCount();
	if (thePrefs.ShowCatTabInfos())
		theApp.emuledlg->transferwnd->UpdateCatTabTitles();
	UpdateDisplayedInfo(true);
}

UINT CPartFile::CompleteThreadProc(LPVOID pvParams) 
{ 
	DbgSetThreadName("PartFileComplete");
	InitThreadLocale();
	// SLUGFILLER: SafeHash // NEO: STS - [SlugFillerThreadSafe] -- Xanatos -->
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return 0;
	// SLUGFILLER: SafeHash // NEO: STS END <-- Xanatos --
	CPartFile* pFile = (CPartFile*)pvParams;
	if (!pFile)
		return (UINT)-1; 
	CSingleLock sLock(&theApp.hashing_mut, TRUE);	// SLUGFILLER: SafeHash - only file operation at a time // NEO: SSH - [SlugFillerSafeHash] <-- Xanatos --
   	pFile->PerformFileComplete(); 
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
	HMODULE hLib = LoadLibrary(_T("KERNEL32.DLL"));
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
		FreeLibrary(hLib);
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
	//TCHAR* newfilename = _tcsdup(GetFileName());
	TCHAR* newfilename = _tcsdup(GetFileName(true)); // NEO: PP - [PasswordProtection] <-- Xanatos --
	_tcscpy(newfilename, (LPCTSTR)StripInvalidFilenameChars(newfilename));

	CString strNewname;
	CString indir;

	//if (PathFileExists(thePrefs.GetCategory(GetCategory())->strIncomingPath)){
	if (PathFileExists(thePrefs.GetCatPath(GetCategory()))){ // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
		//indir = thePrefs.GetCategory(GetCategory())->strIncomingPath;
		indir = thePrefs.GetCatPath(GetCategory()); // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
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

	// NEO: GSL - [GlobalSourceLimit] -- Xanatos -->
	if(NeoPrefs.PartPrefs.UseGlobalSourceLimit() && 
		theApp.downloadqueue->GetPassiveMode())
	{
		theApp.downloadqueue->SetPassiveMode(false);
		AddDebugLogLine(true,_T("{GSL} File completed! Disabled PassiveMode!"));
	}
	// NEO: GSL END <-- Xanatos --

	CleanUpFiles(); // NEO: MOD - [CleanUpFiles] <-- Xanatos --

	// initialize 'this' part file for being a 'complete' file, this is to be done *before* releasing the file mutex.
	m_fullname = strNewname;
	SetPath(indir);
	SetFilePath(m_fullname);
	_SetStatus(PS_COMPLETE); // set status of CPartFile object, but do not update GUI (to avoid multi-thread problems)
	paused = false;
	SetFileOp(PFOP_NONE);

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	if(KnownPrefs->IsEnableVoodoo())
		theApp.voodoo->ManifestDownloadInstruction(this,INST_COMPLETE);
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

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
			theApp.emuledlg->transferwnd->downloadlistctrl.RemoveFile(this);
		else
			UpdateDisplayedInfo(true);

		theApp.emuledlg->transferwnd->downloadlistctrl.ShowFilesCount();

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
		//if(!m_pCollection && CCollection::HasCollectionExtention(GetFileName()))
		if(!m_pCollection && IsCollection()) // NEO: MOD - [IsCollection] <-- Xanatos --
		{
			m_pCollection = new CCollection();
			//if(!m_pCollection->InitCollectionFromFile(GetFilePath(), GetFileName()))
			if(!m_pCollection->InitCollectionFromFile(GetFilePath(), GetFileName(true))) // NEO: PP - [PasswordProtection] <-- Xanatos --
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
				theApp.emuledlg->transferwnd->downloadlistctrl.RemoveSource(this->A4AFsrclist.GetAt(pos2),this);
			}
			else{
				pos3 = A4AFsrclist.GetAt(pos2)->m_OtherNoNeeded_list.Find(this); 
				if(pos3)
				{ 
					A4AFsrclist.GetAt(pos2)->m_OtherNoNeeded_list.RemoveAt(pos3);
					theApp.emuledlg->transferwnd->downloadlistctrl.RemoveSource(A4AFsrclist.GetAt(pos2),this);
				}
			}
		}
		A4AFsrclist.RemoveAll();
	}
	
	UpdateFileRatingCommentAvail();
}

//void CPartFile::DeleteFile(){
void CPartFile::DeleteFile(bool bUnloadOnly, bool resort){ // NEO: MTD - [MultiTempDirectories] // NEO: VOODOO - [UniversalPartfileInterface] <-- Xanatos --
	ASSERT ( !m_bPreviewing );

	// Barry - Need to tell any connected clients to stop sending the file
	StopFile(true, resort); // NEO: VOODOO - [UniversalPartfileInterface] <-- Xanatos --

	// NEO: MOD - [SpaceAllocate] -- Xanatos -->
	if(m_AllocateThread != NULL){
		TerminateThread(m_AllocateThread->m_hThread, 100);
		m_AllocateThread = NULL;
	}
	// NEO: MOD END <-- Xanatos --
	// NEO: FFT - [FileFlushThread] -- Xanatos -->
	if(m_FlushThread != NULL){
		TerminateThread(m_FlushThread->m_hThread, 100);
		m_FlushThread = NULL;
	}
	// NEO: FFT END <-- Xanatos --
	// NEO: SSH - [SlugFillerSafeHash] -- Xanatos -->
	if(m_HashThread != NULL){
		m_PartsToHashLocker.Lock();
		if(((CPartHashThread*)m_HashThread)->file.m_hFile != INVALID_HANDLE_VALUE)
			((CPartHashThread*)m_HashThread)->file.Close();
		TerminateThread(m_HashThread->m_hThread, 100);
		m_HashThread = NULL;
		m_PartsToHashLocker.Unlock();
	}
	// NEO: SSH END <-- Xanatos --
	// NEO: SCV - [SubChunkVerification] -- Xanatos -->
	if(m_AICHHashThread != NULL){
		m_BlocksToHashLocker.Lock();
		if(((CBlockHashThread*)m_AICHHashThread)->file.m_hFile != INVALID_HANDLE_VALUE)
			((CBlockHashThread*)m_AICHHashThread)->file.Close();
		TerminateThread(m_AICHHashThread->m_hThread, 100);
		m_AICHHashThread = NULL;
		m_BlocksToHashLocker.Unlock();
	}
	// NEO: SCV END <-- Xanatos --

	// feel free to implement a runtime handling mechanism!
	if (m_AllocateThread != NULL){
		LogWarning(LOG_STATUSBAR, GetResString(IDS_DELETEAFTERALLOC), GetFileName());
		m_bDeleteAfterAlloc=true;
		return;
	}

	theApp.sharedfiles->RemoveFile(this);
	theApp.downloadqueue->RemoveFile(this);
	theApp.emuledlg->transferwnd->downloadlistctrl.RemoveFile(this);
	theApp.knownfiles->AddCancelledFileID(GetFileHash());

	if (m_hpartfile.m_hFile != INVALID_HANDLE_VALUE)
		m_hpartfile.Close();

	// NEO: GSL - [GlobalSourceLimit] -- Xanatos -->
	if(NeoPrefs.PartPrefs.UseGlobalSourceLimit() && 
		theApp.downloadqueue->GetPassiveMode())
	{
		theApp.downloadqueue->SetPassiveMode(false);
		AddDebugLogLine(true,_T("{GSL} File deleted! Disabled PassiveMode!"));
	}
	// NEO: GSL END <-- Xanatos --

	// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
	if(!bUnloadOnly 
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	|| !m_fullname.IsEmpty()
#endif // VOODOO // NEO: VOODOO END
	){
	// NEO: MTD END <-- Xanatos --

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	  if(!IsVoodooFile()){
#endif // VOODOO // NEO: VOODOO END
		CString partfilename(RemoveFileExtension(m_fullname));
		if (_tremove(partfilename))
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_DELETE) + _T(" - ") + CString(_tcserror(errno)), partfilename);
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	  }
#endif // VOODOO // NEO: VOODOO END

		CleanUpFiles(); // NEO: MOD - [CleanUpFiles] <-- Xanatos --

	} // NEO: MTD - [MultiTempDirectories] <-- Xanatos --

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatso -->
	if(!IsVoodooFile() && KnownPrefs->IsEnableVoodoo())
		theApp.voodoo->ManifestDownloadInstruction(this,INST_DELETE);
#endif // VOODOO // NEO: VOODOO END <-- Xanatso --

	delete this;
}

void CPartFile::CleanUpFiles(){ // NEO: MOD - [CleanUpFiles] <-- Xanatos --

	// remove part.met file
	if (_tremove(m_fullname))
		theApp.QueueLogLine(true, GetResString(IDS_ERR_DELETEFAILED) + _T(" - ") + CString(_tcserror(errno)), m_fullname);

	// remove backup files
	CString BAKName(m_fullname);
	BAKName.Append(PARTMET_BAK_EXT);
	if (_taccess(BAKName, 0) == 0 && !::DeleteFile(BAKName))
		theApp.QueueLogLine(true,GetResString(IDS_ERR_DELETE) + _T(" - ") + GetErrorMessage(GetLastError()), BAKName);

	BAKName = m_fullname;
	BAKName.Append(PARTMET_TMP_EXT);
	if (_taccess(BAKName, 0) == 0 && !::DeleteFile(BAKName))
		theApp.QueueLogLine(true,GetResString(IDS_ERR_DELETE) + _T(" - ") + GetErrorMessage(GetLastError()), BAKName);
	
	// NEO: FCFG - [FileConfiguration] -- Xanatos -->
	BAKName = m_fullname;
	BAKName.Append(PARTNEO_EXT);
	if (_tremove(BAKName))
		theApp.QueueModLogLine(true,GetResString(IDS_ERR_DELETE) + _T(" - ") + GetErrorMessage(GetLastError()), BAKName);

	BAKName = m_fullname;
	BAKName.Append(PARTNEO_EXT);
	BAKName.Append(PARTNEO_BAK_EXT);
	if (_taccess(BAKName, 0) == 0 && !::DeleteFile(BAKName))
		theApp.QueueModLogLine(true,GetResString(IDS_ERR_DELETE) + _T(" - ") + GetErrorMessage(GetLastError()), BAKName);

	BAKName = m_fullname;
	BAKName.Append(PARTNEO_EXT);
	BAKName.Append(PARTNEO_TMP_EXT);
	if (_taccess(BAKName, 0) == 0 && !::DeleteFile(BAKName))
		theApp.QueueModLogLine(true,GetResString(IDS_ERR_DELETE) + _T(" - ") + GetErrorMessage(GetLastError()), BAKName);
	// NEO: FCFG END <-- Xanatos --

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
	// move part.met.src file
	BAKName.Format(PARTMET_EXT);
	BAKName.Append(PARTSRC_EXT);
	if (_taccess(BAKName, 0) == 0 && !::DeleteFile(BAKName))
		theApp.QueueModLogLine(true,GetResString(IDS_ERR_DELETE) + _T(" - ") + GetErrorMessage(GetLastError()), BAKName);

	// Move backup files
	BAKName.Format(PARTMET_EXT);
	BAKName.Append(PARTSRC_EXT);
	BAKName.Append(PARTNEO_BAK_EXT);
	if (_taccess(BAKName, 0) == 0 && !::DeleteFile(BAKName))
		theApp.QueueModLogLine(true,GetResString(IDS_ERR_DELETE) + _T(" - ") + GetErrorMessage(GetLastError()), BAKName);

	BAKName.Format(PARTMET_EXT);
	BAKName.Append(PARTSRC_EXT);
	BAKName.Append(PARTNEO_TMP_EXT);
	if (_taccess(BAKName, 0) == 0 && !::DeleteFile(BAKName))
		theApp.QueueModLogLine(true,GetResString(IDS_ERR_DELETE) + _T(" - ") + GetErrorMessage(GetLastError()), BAKName);
#endif // NEO_SS // NEO: NSS END
}

bool CPartFile::HashSinglePart(UINT partnumber)
{
	if ((GetHashCount() <= partnumber) && (GetPartCount() > 1)){
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_HASHERRORWARNING), GetFileName());
		hashsetneeded = true;
		return true;
	}
	else if (!GetPartHash(partnumber) && GetPartCount() != 1){
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_INCOMPLETEHASH), GetFileName());
		hashsetneeded = true;
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
		CreateHash(&m_hpartfile, length, hashresult, NULL);

		if (GetPartCount()>1 || GetFileSize()== (uint64)PARTSIZE){
			if (md4cmp(hashresult,GetPartHash(partnumber)))
				return false;
			else
				return true;
		}
		else{
			if (md4cmp(hashresult,m_abyFileHash))
				return false;
			else
				return true;
		}
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
		// NEO: MLD - [ModelesDialogs] -- Xanatos -->
		CCollectionViewDialog* dialog = new CCollectionViewDialog(); 
		dialog->SetCollection(m_pCollection);
		dialog->OpenDialog();
		// NEO: MLD END <-- Xanatos --
		//CCollectionViewDialog dialog;
		//dialog.SetCollection(m_pCollection);
		//dialog.DoModal();
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
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
	if(PartPrefs->AutoSaveSources() && !stopped)
		SaveSources();
#endif // NEO_SS // NEO: NSS END <-- Xanatos --

	// Barry - Need to tell any connected clients to stop sending the file
	PauseFile(false, resort);
	m_LastSearchTimeKad = 0;
	m_TotalSearchesKad = 0;
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	m_LastLanSearchTime = 0;
#endif //LANCAST // NEO: NLC END <-- Xanatos --
	RemoveAllSources(true);
	paused = true;
	stopped = true;
	insufficient = false;
	datarate = 0;
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	landatarate = 0;
#endif //LANCAST // NEO: NLC END <-- Xanatos --
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	voodoodatarate = 0;
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
	memset(m_anStates,0,sizeof(m_anStates));
	memset(src_stats,0,sizeof(src_stats));	//Xman Bugfix
	memset(net_stats,0,sizeof(net_stats));	//Xman Bugfix

	if (!bCancel)
		FlushBuffer(true);

	// NEO: GSL - [GlobalSourceLimit] -- Xanatos -->
	if(NeoPrefs.PartPrefs.UseGlobalSourceLimit() && 
		theApp.downloadqueue->GetPassiveMode())
	{
		theApp.downloadqueue->SetPassiveMode(false);
		AddDebugLogLine(true,_T("{GSL} File stopped! Disabled PassiveMode!"));
	}
	// NEO: GSL END <-- Xanatos --

    if(resort) {
	    theApp.downloadqueue->SortByPriority();
	    theApp.downloadqueue->CheckDiskspace();
    }
	UpdateDisplayedInfo(true);

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	if(!IsVoodooFile() && KnownPrefs->IsEnableVoodoo())
		theApp.voodoo->ManifestDownloadInstruction(this,INST_STOP);
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
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
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	landatarate = 0;
#endif //LANCAST // NEO: NLC END <-- Xanatos --
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	voodoodatarate = 0;
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
	//m_anStates[DS_DOWNLOADING] = 0; // -khaos--+++> Renamed var. // NEO: FIX - [SourceCount] <-- Xanatos --
	if (!bInsufficient)
	{
        if(resort) {
		    theApp.downloadqueue->SortByPriority();
		    theApp.downloadqueue->CheckDiskspace();
        }
		SavePartFile();
	}
	UpdateDisplayedInfo(true);

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	if(!IsVoodooFile() && KnownPrefs->IsEnableVoodoo())
		theApp.voodoo->ManifestDownloadInstruction(this,INST_PAUSE);
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
}

bool CPartFile::CanResumeFile() const
{
	//return (GetStatus()==PS_PAUSED || GetStatus()==PS_INSUFFICIENT || (GetStatus()==PS_ERROR && GetCompletionError()));
	return (GetStatus()==PS_PAUSED || GetStatus()==PS_INSUFFICIENT || (GetStatus()==PS_ERROR && GetCompletionError()) || GetCompletionBreak()); // NEO: POFC - [PauseOnFileComplete] <-- Xanatos --
}

void CPartFile::ResumeFile(bool resort)
{
	if (status==PS_COMPLETE || status==PS_COMPLETING || status==PS_MOVING) // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
		return;
	//if (status==PS_ERROR && m_bCompletionError){
	if (status==PS_ERROR && m_bCompletionError || m_bCompletionBreak){ // NEO: POFC - [PauseOnFileComplete] <-- Xanatos --
		ASSERT( gaplist.IsEmpty() );
		if (gaplist.IsEmpty() && !m_nTotalBufferData && !m_FlushThread) { //MORPH - Added by SiRoB, Flush Thread // NEO: FFT - [FileFlushThread] <-- Xanatos --
			// rehashing the file could probably be avoided, but better be in the safe side..
			m_bCompletionError = false;
			m_bCompletionBreak = false; // NEO: POFC - [PauseOnFileComplete] <-- Xanatos --
			CompleteFile(false);
		}
		return;
	}

	bool bWasPaused = paused; // NEO: MOD - [WasPaused] <-- Xanatos --
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
	bool bWasStopped = stopped;
#endif // NEO_SS // NEO: NSS END <-- Xanatos --

	paused = false;
	stopped = false;

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
	if(PartPrefs->AutoLoadSources() == TRUE && bWasStopped) // Load only when we resuming from stoppen state, not from pause.
		LoadSources();
#endif // NEO_SS // NEO: NSS END <-- Xanatos --

	// NEO: GSL - [GlobalSourceLimit] -- Xanatos -->
	InitHL();
	if(NeoPrefs.PartPrefs.UseGlobalSourceLimit() && 
		theApp.downloadqueue->GetPassiveMode())
	{
		theApp.downloadqueue->SetPassiveMode(false);
		AddDebugLogLine(true,_T("{GSL} New file resumed! Disabled PassiveMode!"));
	}
	// NEO: GSL END <-- Xanatos --

	//SetActive(theApp.IsConnected());
	SetActive(theApp.GetConState()); // NEO: NCC - [NeoConnectionChecker] <-- Xanatos --
	m_LastSearchTime = 0;
	m_LastSearchTimeUdp = 0; // NEO: SRT - [SourceRequestTweaks] <-- Xanatos --
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	m_LastLanSearchTime = 0;
#endif //LANCAST // NEO: NLC END <-- Xanatos --
    if(resort) {
	    theApp.downloadqueue->SortByPriority();
	    theApp.downloadqueue->CheckDiskspace();
    }
	if(bWasPaused){ // NEO: MOD - [WasPaused] <-- Xanatos --
		SavePartFile();
		NotifyStatusChange();

		UpdateDisplayedInfo(true);

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
		if(!IsVoodooFile() && KnownPrefs->IsEnableVoodoo())
			theApp.voodoo->ManifestDownloadInstruction(this,INST_RESUME);
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
	}
}

void CPartFile::ResumeFileInsufficient()
{
	if (status==PS_COMPLETE || status==PS_COMPLETING)
		return;
	if (!insufficient)
		return;
	AddLogLine(false, _T("Resuming download of \"%s\""), GetFileName());
	insufficient = false;
	//SetActive(theApp.IsConnected());
	SetActive(theApp.GetConState()); // NEO: NCC - [NeoConnectionChecker] <-- Xanatos --
	m_LastSearchTime = 0;
	m_LastSearchTimeUdp = 0; // NEO: SRT - [SourceRequestTweaks] <-- Xanatos --
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	m_LastLanSearchTime = 0;
#endif //LANCAST // NEO: NLC END <-- Xanatos --
	UpdateDisplayedInfo(true);
}

CString CPartFile::getPartfileStatus() const
{
	switch(GetStatus()){
		// NEO: SSH - [SlugFillerSafeHash] -- Xanatos --
		//case PS_HASHING:
		//case PS_WAITINGFORHASH:
		//	return GetResString(IDS_HASHING);

		// NEO: PIX - [PartImportExport] -- Xanatos -->
		case PS_IMPORTING:
			return GetResString(IDS_X_IMPORTING);
		// NEO: MTD END <-- Xanatos --

		// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
		case PS_MOVING:
			return GetResString(IDS_X_MOVING);
		// NEO: MTD END <-- Xanatos --

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
			return GetResString(IDS_COMPLETE);

		case PS_PAUSED:
			if (stopped)
				return GetResString(IDS_STOPPED);
			return GetResString(IDS_PAUSED);

		case PS_INSUFFICIENT:
			return GetResString(IDS_INSUFFICIENT);

		case PS_ERROR:
			if (m_bCompletionError)
				return GetResString(IDS_INSUFFICIENT);
			return GetResString(IDS_ERRORLIKE);
	}

	if (GetSrcStatisticsValue(DS_DOWNLOADING) > 0)
		return GetResString(IDS_DOWNLOADING);
	// NEO: SC - [SuspendCollecting] -- Xanatos -->
	else if(IsCollectingHalted() == 1)
		return GetResString(IDS_X_SUSPEND);
	else if(IsCollectingHalted() == 3)
		return GetResString(IDS_X_SUSPEND_SHORT);
	// NEO: SC END  <-- Xanatos --
	// NEO: SD - [StandByDL] -- Xanatos -->
	else if(IsStandBy() == 1)
		return GetResString(IDS_X_STANDBY);
	else if(IsStandBy() == 2)
		return GetResString(IDS_X_STANDBY_STALLED);
	// NEO: SD END  <-- Xanatos --
	// NEO: OCF - [OnlyCompleetFiles] -- Xanatos -->
	else if(NotSeenCompleteSource())
		return GetResString(IDS_X_STALLED);
	// NEO: OCF END <-- Xanatos --
	else
		return GetResString(IDS_WAITING);
} 

int CPartFile::getPartfileStatusRang() const
{
	/*switch (GetStatus()) {
		// NEO: SSH - [SlugFillerSafeHash] -- Xanatos --
		//case PS_HASHING: 
		//case PS_WAITINGFORHASH:
		case PS_IMPORTING: // NEO: PIX - [PartImportExport] <-- Xanatos --
		case PS_MOVING: // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
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
	return 2; // downloading?*/

	// NEO: MOD - [NewSorting] -- Xanatos -->
	switch (GetStatus())
	{
	case PS_ERROR:
		return 0;
	case PS_INSUFFICIENT:
		return 1;
	case PS_COMPLETE:
		return 2;
	case PS_COMPLETING:
		return 3;
	//case PS_HASHING:
	//	return 4;
	//case PS_WAITINGFORHASH: // does not exist with Sefa hash
	//	return 5;
	case PS_MOVING:
		return 6;
	case PS_IMPORTING:
		return 7;
	case PS_PAUSED:
		if (IsStopped())
			return 14;
		else
			return 13;
	}
	if (GetSrcStatisticsValue(DS_DOWNLOADING) == 0)
	{
		if(const_cast <CPartFile*> (this)->GetPartsHashing())
			return 4;
		if(IsStandBy())
			return 11;
		if(IsCollectingHalted())
			return 12;
		if(NotSeenCompleteSource())
			return 10;
		return 9; // waiting
	}
	return 8;
	// NEO: MOD END <-- Xanatos --
} 

time_t CPartFile::getTimeRemainingSimple() const
{
	if (GetDatarate() == 0)
		return -1;
	return (time_t)((uint64)(GetFileSize() - GetCompletedSize()) / (uint64)GetDatarate());
}

time_t CPartFile::getTimeRemaining() const
{
	EMFileSize completesize = GetCompletedSize();
	time_t simple = -1;
	time_t estimate = -1;
	if( GetDatarate() > 0 )
	{
		simple = (time_t)((uint64)(GetFileSize() - completesize) / (uint64)GetDatarate());
	}
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
		m_bPreviewing = true;
		CPreviewThread* pThread = (CPreviewThread*) AfxBeginThread(RUNTIME_CLASS(CPreviewThread), THREAD_PRIORITY_NORMAL,0, CREATE_SUSPENDED);
		pThread->SetValues(this, thePrefs.GetVideoPlayer(), thePrefs.GetVideoPlayerArgs());
		pThread->ResumeThread();
	}
	else{
		if (!thePrefs.GetVideoPlayer().IsEmpty())
			ExecutePartFile(this, thePrefs.GetVideoPlayer(), thePrefs.GetVideoPlayerArgs());
		else {
			CString strPartFilePath = GetFullName();

			// strip available ".met" extension to get the part file name.
			if (strPartFilePath.GetLength()>4 && strPartFilePath.Right(4)==_T(".met"))
				strPartFilePath.Delete(strPartFilePath.GetLength()-4,4);

			// if the path contains spaces, quote the entire path
			if (strPartFilePath.Find(_T(' ')) != -1)
				strPartFilePath = _T('\"') + strPartFilePath + _T('\"');

			ShellExecute(NULL, NULL, strPartFilePath, NULL, NULL, SW_SHOWNORMAL);
		}
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
		//if (uState == PS_COMPLETE || uState == PS_COMPLETING)
		if (uState == PS_COMPLETE || uState == PS_COMPLETING || uState == PS_MOVING) // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
			return false;

		// check part file size(s)
		if (GetFileSize() < (uint64)1024 || GetCompletedSize() < (uint64)1024)
			return false;

		// check if we already trying to recover an archive file from this part file
		if (m_bRecoveringArchive)
			return false;

		// check free disk space
		uint64 uMinFreeDiskSpace = (thePrefs.IsCheckDiskspaceEnabled() && thePrefs.GetMinFreeDiskSpace() > 0)
									? thePrefs.GetMinFreeDiskSpace()
									: 20*1024*1024;
		if (thePrefs.GetPreviewCopiedArchives())
			uMinFreeDiskSpace += (uint64)(GetFileSize() * (uint64)2);
		else
			uMinFreeDiskSpace += (uint64)(GetCompletedSize() + (uint64)16*1024);
		if (GetFreeDiskSpaceX(GetTempPath()) < uMinFreeDiskSpace)
			return false;
		return true; 
	}

	if (thePrefs.IsMoviePreviewBackup())
	{
		return !( (GetStatus() != PS_READY && GetStatus() != PS_PAUSED) 
				|| m_bPreviewing || GetPartCount() < 5 || !IsMovie() || (GetFreeDiskSpaceX(GetTempPath()) + 100000000) < GetFileSize()
				|| ( !IsComplete(0,PARTSIZE-1, false) || !IsComplete(PARTSIZE*(uint64)(GetPartCount()-1),GetFileSize() - (uint64)1, false)));
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
				//EED2KFileType eFileType = GetED2KFileTypeID(GetFileName());
				EED2KFileType eFileType = GetED2KFileTypeID(GetFileName(true)); // NEO: PP - [PasswordProtection] <-- Xanatos --
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
		    //LPCTSTR pszExt = _tcsrchr(GetFileName(), _T('.'));
			LPCTSTR pszExt = _tcsrchr(GetFileName(true), _T('.')); // NEO: PP - [PasswordProtection] <-- Xanatos --
		    if (pszExt != NULL){
			    CString strExt(pszExt);
			    strExt.MakeLower();
			    bMPEG = (strExt==_T(".mpg") || strExt==_T(".mpeg") || strExt==_T(".mpe") || strExt==_T(".mp3") || strExt==_T(".mp2") || strExt==_T(".mpa"));
		    }

		    if (bMPEG){
			    // ToDo: search a block which is at least 16K (Audio) or 256K (Video)
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
		bool bFound = false; // NEO: MFSB - [MultiFileStatusBars] <-- Xanatos --
		for (POSITION pos = srclist.GetHeadPosition(); pos; ){
			// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
			CClientFileStatus* status = srclist.GetNext(pos)->GetFileStatus(this);
			if(status == NULL || status->GetPartCount() != iPartCount)
				continue;
			uint8* abyUpPartStatus = status->GetPartStatus();
			if(abyUpPartStatus && abyUpPartStatus[ixPart]){
			// NEO: SCFS END <-- Xanatos --
			//if (srclist.GetNext(pos)->IsPartAvailable(ixPart)){
				availablecounter++; 
				bFound = true; // NEO: MFSB - [MultiFileStatusBars] <-- Xanatos --
				break;
			}
		}
		// NEO: MFSB - [MultiFileStatusBars] -- Xanatos -->
		if(bFound == false){
			for (POSITION pos = A4AFsrclist.GetHeadPosition(); pos; ){
				// NEO: SCFS - [SmartClientFileStatus]
				CClientFileStatus* status = A4AFsrclist.GetNext(pos)->GetFileStatus(this);
				if(status == NULL || status->GetPartCount() != iPartCount)
					continue;
				uint8* abyUpPartStatus = status->GetPartStatus();
				if(abyUpPartStatus && abyUpPartStatus[ixPart]){
				// NEO: SCFS END
				//if (A4AFsrclist.GetNext(pos)->IsPartAvailable(ixPart)){
					availablecounter++; 
					break;
				}
			}
		}
		// NEO: MFSB END <-- Xanatos --
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

	// NEO: SCFS - [SmartClientFileStatus] -- Xanatos --
	// check whether client has either no download status at all or a download status which is valid for this file
	/*if (!(forClient->GetUpPartCount() == 0 && forClient->GetUpPartStatus() == NULL)
		&& !(forClient->GetUpPartCount() == GetPartCount() && forClient->GetUpPartStatus() != NULL))
	{
		// should never happen
		DEBUG_ONLY( DebugLogError(_T("*** %hs - part count (%u) of client (%s) does not match part count (%u) of file \"%s\""), __FUNCTION__, forClient->GetUpPartCount(), forClient->DbgGetClientInfo(), GetPartCount(), GetFileName()) );
		ASSERT(0);
		return NULL;
	}*/

	if (!(GetStatus() == PS_READY || GetStatus() == PS_EMPTY))
		return NULL;

	CSafeMemFile data(1024);
	
	uint8 byUsedVersion;
	bool bIsSX2Packet;
	// NEO: NXS - [NeoXS] -- Xanatos -->
	if (forClient->SupportsNeoXS()){
		byUsedVersion = 3;
		bIsSX2Packet = false;
	}
	else
	// NEO: NXS END <-- Xanatos --
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
	data.WriteHash16(m_abyFileHash);
	data.WriteUInt16((uint16)nCount);
	
	bool bNeeded;
	//const uint8* reqstatus = forClient->GetUpPartStatus();
	// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
	CClientFileStatus* reqFileStatus = forClient->GetFileStatus(this);
	const uint8* reqstatus = reqFileStatus ? reqFileStatus->GetPartStatus() : NULL;
	// NEO: SCFS END <-- Xanatos --
	for (POSITION pos = srclist.GetHeadPosition();pos != 0;){
		bNeeded = false;
		const CUpDownClient* cur_src = srclist.GetNext(pos);
		//if (cur_src->HasLowID() || !cur_src->IsValidSource())
		if ((cur_src->HasLowID() && !forClient->SupportsNeoXS()) || !cur_src->IsValidSource()) // NEO: NXS - [NeoXS] <-- Xanatos --
			continue;
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
		if (cur_src->IsLanClient() && !forClient->IsLanClient())
			continue;
#endif //LANCAST // NEO: NLC END <-- Xanatos --
		//const uint8* srcstatus = cur_src->GetPartStatus();
		// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
		CClientFileStatus* srcFileStatus = cur_src->GetFileStatus(this);
		const uint8* srcstatus = srcFileStatus ? srcFileStatus->GetPartStatus() : NULL;
		// NEO: SCFS END <-- Xanatos --
		if (srcstatus){
			if (reqstatus){
				//if (cur_src->GetPartCount() == GetPartCount()){
				if (srcFileStatus->GetPartCount() == reqFileStatus->GetPartCount()){ // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
					//ASSERT( forClient->GetUpPartCount() == GetPartCount() ); // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
					// only send sources which have needed parts for this client
					for (UINT x = 0; x < GetPartCount(); x++){
						if (srcstatus[x] && !reqstatus[x]){
							bNeeded = true;
							break;
						}
					}
				}
				else{
					// should never happen
					if (thePrefs.GetVerbose())
						//DEBUG_ONLY(DebugLogError(_T("*** %hs - found source (%s) with wrong partcount (%u) attached to partfile \"%s\" (partcount=%u)"), __FUNCTION__, cur_src->DbgGetClientInfo(), cur_src->GetPartCount(), GetFileName(), GetPartCount()));
						DEBUG_ONLY(DebugLogError(_T("*** %hs - found source (%s) with wrong partcount (%u) attached to partfile \"%s\" (partcount=%u)"), __FUNCTION__, cur_src->DbgGetClientInfo(), srcFileStatus->GetPartCount(), GetFileName(), GetPartCount())); // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
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

		if (bNeeded){
			nCount++;
			uint32 dwID;
			if (byUsedVersion >= 3)
				dwID = cur_src->GetUserIDHybrid();
			else
				dwID = ntohl(cur_src->GetUserIDHybrid());
			data.WriteUInt32(dwID);
			data.WriteUInt16(cur_src->GetUserPort());
			// NEO: NXS - [NeoXS] -- Xanatos -->
			if(forClient->SupportsNeoXS()){
				data.WriteHash16(cur_src->GetUserHash());
				cur_src->WriteNeoXSTags(&data);
			}else
			// NEO: NXS END <-- Xanatos --
			{
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
					const uint8 uRequestsCryptLayer	= cur_src->RequestsCryptLayer() ? 1 : 0;
					const uint8 uRequiresCryptLayer	= cur_src->RequiresCryptLayer() ? 1 : 0;
					//const uint8 uDirectUDPCallback	= cur_src->SupportsDirectUDPCallback() ? 1 : 0;
					const uint8 byCryptOptions = /*(uDirectUDPCallback << 3) |*/ (uRequiresCryptLayer << 2) | (uRequestsCryptLayer << 1) | (uSupportsCryptLayer << 0);
					data.WriteUInt8(byCryptOptions);
				}
			}
			if (nCount > 500)
				break;
		}
	}
	if (!nCount)
		return 0;
	data.Seek(bIsSX2Packet ? 17 : 16, SEEK_SET);
	data.WriteUInt16((uint16)nCount);

	//Packet* result = new Packet(&data, OP_EMULEPROT);
	//result->opcode = bIsSX2Packet ? OP_ANSWERSOURCES2 : OP_ANSWERSOURCES;
	// NEO: NXS - [NeoXS] -- Xanatos -->
	Packet* result = new Packet(&data, forClient->SupportsNeoXS() ? OP_MODPROT : OP_EMULEPROT);
	result->opcode = forClient->SupportsNeoXS() ? OP_NEO_ANSWERSOURCES : (bIsSX2Packet ? OP_ANSWERSOURCES2 : OP_ANSWERSOURCES);
	// NEO: NXS END <-- Xanatos --
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
	// NEO: NXS - [NeoXS] -- Xanatos -->
	CUpDownClient::tNeoXSTags* NeoXSTags = NULL; // we first read the tags and create the cleint later so we have to buffer them
	if(pClient && pClient->SupportsNeoXS()){ 
		uPacketSXVersion = 3; //4 // only needed for the proper ip
		NeoXSTags = new CUpDownClient::tNeoXSTags;

		nCount = sources->ReadUInt16();
	}
	else // Don't do this check for Neo XS it has an dynamic size
	// NEO: NXS END <-- Xanatos --
	{
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
						DebugLogWarning(_T("Received invalid SX packet (v%u, count=%u, size=%u), %sFile=\"%s\""), uClientSXVersion, nCount, uDataSize, strDbgClientInfo, GetFileName());
					}
					return;
				}
				uPacketSXVersion = 1;
			}
			// Checks if version 2&3 packet is correct size
			else if (nCount*(4+2+4+2+16) == uDataSize)
			{
				// Received v2,v3 packet: Check if remote client supports at least v2
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
	}

	for (UINT i = 0; i < nCount; i++)
	{
		uint32 dwID = sources->ReadUInt32();
		uint16 nPort = sources->ReadUInt16();
		// NEO: NXS - [NeoXS] -- Xanatos -->
		uint32 dwServerIP = 0;
		uint16 nServerPort = 0;
		uchar achUserHash[16];
		uint8 byCryptOptions = 0;

		if(NeoXSTags){
			sources->ReadHash16(achUserHash);
			CUpDownClient::ReadNeoXSTags(sources, NeoXSTags);
		}else
		// NEO: NXS END <-- Xanatos --
		{
			/*uint32*/ dwServerIP = sources->ReadUInt32();
			/*uint16*/ nServerPort = sources->ReadUInt16();

			//uchar achUserHash[16];
			if (uPacketSXVersion >= 2)
				sources->ReadHash16(achUserHash);

			//uint8 byCryptOptions = 0;
			if (uPacketSXVersion >= 4)
				byCryptOptions = sources->ReadUInt8();
		}

		// Clients send ID's in the Hyrbid format so highID clients with *.*.*.0 won't be falsely switched to a lowID..
		uint32 dwIDED2K = (uPacketSXVersion >= 3) ? ntohl(dwID) : dwID; // NEO: CI#6 - [CodeImprovement] <-- Xanatos --

		// NEO: CI#6 - [CodeImprovement] -- Xanatos -->
		if(!CheckSourceID(dwIDED2K)) 
		{
			//if (thePrefs.GetLogFilteredIPs())
			//	AddDebugLogLine(false, _T("Ignored source (IP=%s) received via source exchange"), ipstr(dwIDED2K));
			continue;
		}
		// NEO: CI#6 END <-- Xanatos --

		// additionally check for LowID and own IP
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
		if (!CanAddSource(dwID, nPort, NeoXSTags ? NeoXSTags->dwServerIP : dwServerIP, NeoXSTags ? NeoXSTags->nServerPort : nServerPort, NULL, uPacketSXVersion < 3, NeoXSTags && NeoXSTags->uSupportsNatTraversal)) // NEO: NXS - [NeoXS] <-- Xanatos --
#else
		if (!CanAddSource(dwID, nPort, NeoXSTags ? NeoXSTags->dwServerIP : dwServerIP, NeoXSTags ? NeoXSTags->nServerPort : nServerPort, NULL, uPacketSXVersion < 3)) // NEO: NXS - [NeoXS] <-- Xanatos --
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
		//if (!CanAddSource(dwID, nPort, dwServerIP, nServerPort, NULL, uPacketSXVersion < 3)) // NEO: CI#6 - [CodeImprovement] <-- Xanatos --
		{
			//if (thePrefs.GetLogFilteredIPs())
			//	AddDebugLogLine(false, _T("Ignored source (IP=%s) received via source exchange"), ipstr(dwIDED2K));
			continue;
		}
	

		//if (GetMaxSources() > GetSourceCount())
		// NEO: XSC - [ExtremeSourceCache] -- Xanatos -->
		bool bCache = false;
		if ((bCache = !(GetMaxSources() > GetSourceCount())) == false // NEO: SRT - [SourceRequestTweaks] <-- Xanatos --
		 || PartPrefs->UseSourceCache() && GetSourceCacheSourceLimit() > this->GetSrcStatisticsValue(DS_CACHED)) 
		// NEO: XSC END <-- Xanatos --
		{
			CUpDownClient* newsource;
			if (uPacketSXVersion >= 3)
				newsource = new CUpDownClient(this, nPort, dwID, dwServerIP, nServerPort, false);
			else
				newsource = new CUpDownClient(this, nPort, dwID, dwServerIP, nServerPort, true);
			if (uPacketSXVersion >= 2)
				newsource->SetUserHash(achUserHash);
			// NEO: NXS - [NeoXS] -- Xanatos -->
			if(NeoXSTags)
				NeoXSTags->Attach(newsource);
			else
			// NEO: NXS END <-- Xanatos --
			if (uPacketSXVersion >= 4) {
				newsource->SetConnectOptions(byCryptOptions, true, false);
				//if (thePrefs.GetDebugSourceExchange()) // remove this log later
				//	AddDebugLogLine(false, _T("Received CryptLayer aware (%u) source from V4 Sourceexchange (%s)"), byCryptOptions, newsource->DbgGetClientInfo());
			}
			newsource->SetSourceFrom(SF_SOURCE_EXCHANGE);
			// NEO: XSC - [ExtremeSourceCache] -- Xanatos -->
			if(bCache)
				newsource->SetDownloadState(DS_CACHED);
			// NEO: XSC END <-- Xanatos --
			theApp.downloadqueue->CheckAndAddSource(this, newsource);
		} 
		else
			break;
	}

	// NEO: NXS - [NeoXS] -- Xanatos -->
	if(NeoXSTags)
		delete NeoXSTags;
	// NEO: NXS END <-- Xanatos --
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
								uint32 clientIP) // NEO: VOODOO - [UniversalPartfileInterface] <-- Xanatos --
								//const CUpDownClient* client)
{
	ASSERT( (sint64)transize > 0 );
	ASSERT( start <= end );

	// Increment transferred bytes counter for this file
	m_uTransferred += transize;
	m_uTransferredSession += transize; // MOD - [SessionDL] <-- Xanatos --

	// This is needed a few times
	uint32 lenData = (uint32)(end - start + 1);
	ASSERT( (int)lenData > 0 && (uint64)(end - start + 1) == lenData);

	if (lenData > transize) {
		m_uCompressionGain += lenData - transize;
		thePrefs.Add2SavedFromCompression(lenData - transize);
	}

	// Occasionally packets are duplicated, no point writing it twice
	//MORPH - Optimization, don't check this twice only use the loop to write into gap 
	// Occasionally packets are duplicated, no point writing it twice
#ifdef _DEBUG // NEO: PI#2 -[PerformanceImprovement] <-- Xanatos --
	if (IsComplete(start, end, false))
	{
		if (thePrefs.GetVerbose()){
			//AddDebugLogLine(false, _T("PrcBlkPkt: Already written block %s; File=%s; %s"), DbgGetBlockInfo(start, end), GetFileName(), client->DbgGetClientInfo());
			// NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
			CUpDownClient* client = theApp.clientlist->FindClientByIP(clientIP);
			AddDebugLogLine(false, _T("PrcBlkPkt: Already written block %s; File=%s; %s"), DbgGetBlockInfo(start, end), GetFileName(), client ? client->DbgGetClientInfo() : _T("N/A"));
			// NEO: VOODOO END <-- Xanatos --
		}
		return 0;
	}
#endif	

	// security sanitize check to make sure we do not write anything into an already hashed complete chunk
	const uint64 nStartChunk = start / PARTSIZE;
	const uint64 nEndChunk = end / PARTSIZE;
	if (IsComplete(PARTSIZE * (uint64)nStartChunk, (PARTSIZE * (uint64)(nStartChunk + 1)) - 1, false)){
		// NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
		CUpDownClient* client = theApp.clientlist->FindClientByIP(clientIP); 
		DebugLogError( _T("PrcBlkPkt: Received data touches already hashed chunk - ignored (start) %s; File=%s; %s"), DbgGetBlockInfo(start, end), GetFileName(), client ? client->DbgGetClientInfo() : _T("N/A"));
		// NEO: VOODOO END <-- Xanatos --
		return 0;
	}
	else if (nStartChunk != nEndChunk) {
		if (IsComplete(PARTSIZE * (uint64)nEndChunk, (PARTSIZE * (uint64)(nEndChunk + 1)) - 1, false)){
			// NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
			CUpDownClient* client = theApp.clientlist->FindClientByIP(clientIP);
			DebugLogError( _T("PrcBlkPkt: Received data touches already hashed chunk - ignored (end) %s; File=%s; %s"), DbgGetBlockInfo(start, end), GetFileName(), client ? client->DbgGetClientInfo() : _T("N/A"));
			// NEO: VOODOO END <-- Xanatos --
			return 0;
		}
		else
		// NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
		{
#ifdef _DEBUG
			CUpDownClient* client = theApp.clientlist->FindClientByIP(clientIP);
			DebugLogWarning(_T("PrcBlkPkt: Received data crosses chunk boundaries %s; File=%s; %s"), DbgGetBlockInfo(start, end), GetFileName(), client ? client->DbgGetClientInfo() : _T("N/A"));
#endif
		}
		// NEO: VOODOO END <-- Xanatos --
			//DEBUG_ONLY( DebugLogWarning(_T("PrcBlkPkt: Received data crosses chunk boundaries %s; File=%s; %s"), DbgGetBlockInfo(start, end), GetFileName(), client->DbgGetClientInfo()) );
	}

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
  if(!IsVoodooFile()){
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

	// BEGIN SLUGFILLER: SafeHash // NEO: SSH - [SlugFillerSafeHash] -- Xanatos -->
	//CSingleLock sLock(&ICH_mut,true);	// Wait for ICH result
	//ParseICHResult();	// Check result to prevent post-complete writing
	//sLock.Unlock();

	lenData = 0; // this one is an effective counter

	// only write to gaps
	for (POSITION pos1 = gaplist.GetHeadPosition();pos1 != NULL;){
		Gap_Struct* cur_gap = gaplist.GetNext(pos1);

		if (start > cur_gap->end || end < cur_gap->start)
			continue;

		// Create a new buffered queue entry
		PartFileBufferedData *item = new PartFileBufferedData;
		item->start = (start > cur_gap->start)?start:cur_gap->start;
		item->end = (end < cur_gap->end)?end:cur_gap->end;
		//item->block = block;

		uint32 lenDataClipped = (uint32)(item->end - item->start + 1);
		ASSERT(lenDataClipped <= end - start + 1);

		// log transferinformation in our "blackbox"
		//m_CorruptionBlackBox.TransferredData(start, end, client);
		m_CorruptionBlackBox.TransferredData(start, end, clientIP); // NEO: VOODOO - [UniversalPartfileInterface] <-- Xanatos --

		// Create copy of data as new buffer
		BYTE *buffer = new BYTE[lenDataClipped];
		memcpy(buffer, data+(item->start-start), lenDataClipped);
		item->data = buffer;

		// Official code
		// Add to the queue in the correct position (most likely the end)
		PartFileBufferedData *queueItem;
		bool added = false;
		m_BufferedData_list_Locker.Lock(); //MORPH - Flush Thread // NEO: FFT - [FileFlushThread] <-- Xanatos --
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
		// Official code END
		// NEO: FFT - [FileFlushThread] -- Xanatos -->
		m_nTotalBufferData += lenDataClipped;
		m_BufferedData_list_Locker.Unlock(); //MORPH - Flush Thread 
		// NEO: FFT END <-- Xanatos --
		lenData += lenDataClipped;	// calculate actual added data
	}
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
  }else{
	CMasterDatas* Datas;
	CVoodooSocket* Master;
	POSITION pos = m_MasterMap.GetStartPosition();
	while (pos){
		m_MasterMap.GetNextAssoc(pos, Master, Datas);
		ASSERT(theApp.voodoo->IsValidSocket(Master));
		if(Datas->IsComplete(start, end)) // this master already have the datas
			continue;

		// Create a new buffered queue entry
		PartFileBufferedData* item = new PartFileBufferedData;
		item->start = start;
		item->end = end;
		BYTE *buffer = new BYTE[lenData];
		memcpy(buffer, data, lenData);
		item->data = buffer;


		if(lenData > MB2B(1)){ // Fail safe, CEMSocket buffer can't handle packets larger than 2 MB !
			ASSERT(0); // Do NOT import parts to a voodoo file, import the file on the Master Client !
			Master->SplitPart(this, item);
		}else
			Master->TransferFileData(this, item, clientIP);
	}
  }
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

	// Increment buffer size marker
	//m_nTotalBufferData += lenData;   //MORPH - Flush Thread, Moved above // NEO: FFT - [FileFlushThread] <-- Xanatos --

	// Mark this small section of the file as filled
	//FillGap(item->start, item->end);
	FillGap(start, end); // NEO: CI#7 - [CodeImprovement] <-- Xanatos --

	// Update the flushed mark on the requested block 
	// The loop here is unfortunate but necessary to detect deleted blocks.
	POSITION pos = requestedblocks_list.GetHeadPosition();
	while (pos != NULL)
	{	
		//if (requestedblocks_list.GetNext(pos) == item->block)
		//	item->block->transferred += lenData;
		// NEO: CI#7 - [CodeImprovement] -- Xanatos -->
		if (requestedblocks_list.GetNext(pos) == block)
			block->transferred += lenData;
		// NEO: CI#7 END <-- Xanatos -->
	}
	// END SLUGFILLER: SafeHash // NEO: SSH END <-- Xanatos --

/*// log transferinformation in our "blackbox"
	m_CorruptionBlackBox.TransferredData(start, end, client);

	// Create copy of data as new buffer
	BYTE *buffer = new BYTE[lenData];
	memcpy(buffer, data, lenData);

	// Create a new buffered queue entry
	PartFileBufferedData *item = new PartFileBufferedData;
	item->data = buffer;
	item->start = start;
	item->end = end;
	item->block = block;

	// Add to the queue in the correct position (most likely the end)
	PartFileBufferedData *queueItem;
	bool added = false;
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

	// Increment buffer size marker
	m_nTotalBufferData += lenData; 

	// Mark this small section of the file as filled
	FillGap(item->start, item->end);

	// Update the flushed mark on the requested block 
	// The loop here is unfortunate but necessary to detect deleted blocks.
	pos = requestedblocks_list.GetHeadPosition();
	while (pos != NULL)
	{	
		if (requestedblocks_list.GetNext(pos) == item->block)
			item->block->transferred += lenData;
	}
	*/

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	if(!IsVoodooFile() && KnownPrefs->IsEnableVoodoo())
		theApp.voodoo->ManifestGapList(this);

	if (gaplist.IsEmpty()){
		if(IsVoodooFile())
			CompleteFile(false);
		else
			FlushBuffer(true);
	}
#else
	if (gaplist.IsEmpty())
		FlushBuffer(true);
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

	// Return the length of data written to the buffer
	return lenData;
}

void CPartFile::FlushBuffer(bool forcewait, bool bForceICH, bool /*bNoAICH*/) // NEO: SSH - [SlugFillerSafeHash] <-- Xanatos --
{
	//MORPH START - Added by SiRoB, Flush Thread // NEO: FFT - [FileFlushThread] -- Xanatos -->
	if (forcewait) { //We need to wait for flush thread to terminate
		CWinThread* pThread = m_FlushThread;
		if (pThread != NULL) { //We are flushing something to disk
			HANDLE hThread = pThread->m_hThread;
			// 2 minutes to let the thread finish
			pThread->SetThreadPriority(THREAD_PRIORITY_NORMAL); 
			if (WaitForSingleObject(hThread, 120000) == WAIT_TIMEOUT) {
				AddDebugLogLine(true, _T("Flushing (force=true) failed.(%s)"), GetFileName(), m_nTotalBufferData, m_BufferedData_list.GetCount(), m_uTransferred, m_nLastBufferFlushTime);
				TerminateThread(hThread, 100); // Should never happen
				ASSERT(0);
			}
		}
		if (m_FlushSetting != NULL) //We noramly flushed something to disk
			FlushDone();
	} else 
	if (m_FlushSetting != NULL) { //Some thing is going to be flushed or already flushed 
		                          //wait the window call back to call FlushDone()
		return;
	}
	//MORPH END - Added by SiRoB, Flush Thread // NEO: FFT END <-- Xanatos --

	bool bIncreasedFile=false;

	m_nLastBufferFlushTime = GetTickCount();
	//if (m_BufferedData_list.IsEmpty())
	if (m_nTotalBufferData==0) //MORPH - Added by SiRoB, Flush Thread // NEO: FFT - [FileFlushThread] <-- Xanatos --
		return;

	if (m_AllocateThread!=NULL) {
		// diskspace is being allocated right now.
		// so dont write and keep the data in the buffer for later.
		return;
	}else if (m_iAllocinfo>0) {
		bIncreasedFile=true;
		m_iAllocinfo=0;
	}

	// SLUGFILLER: SafeHash // NEO: SSH - [SlugFillerSafeHash] -- Xanatos -->
	if (forcewait) {	// Last chance to grab any ICH results
		CSingleLock sLock(&ICH_mut,true);	// ICH locks the file - otherwise it may be written to while being checked
		ParseICHResult();	// Check result from ICH
		sLock.Unlock();
	}
	// SLUGFILLER: SafeHash // NEO: SSH END <-- Xanatos --

	//if (thePrefs.GetVerbose())
	//	AddDebugLogLine(false, _T("Flushing file %s - buffer size = %ld bytes (%ld queued items) transferred = %ld [time = %ld]\n"), GetFileName(), m_nTotalBufferData, m_BufferedData_list.GetCount(), m_uTransferred, m_nLastBufferFlushTime);

	UINT partCount = GetPartCount();
	bool *changedPart = new bool[partCount];
	// Remember which parts need to be checked at the end of the flush
	for (UINT partNumber = 0; partNumber < partCount; partNumber++)
		changedPart[partNumber] = false;

	try
	{
		//MORPH START - Added by SiRoB, Flush Thread // NEO: FFT - [FileFlushThread] -- Xanatos -->
		//Creating the Thread to flush to disk
		m_FlushSetting = new FlushDone_Struct;
		m_FlushSetting->bIncreasedFile = bIncreasedFile;
		m_FlushSetting->bForceICH = bForceICH;
		//m_FlushSetting->bNoAICH = bNoAICH;  // SLUGFILLER: SafeHash - removed
		m_FlushSetting->changedPart = changedPart;
		if (forcewait == false) {
			m_FlushThread = AfxBeginThread(RUNTIME_CLASS(CPartFileFlushThread), THREAD_PRIORITY_BELOW_NORMAL,0, CREATE_SUSPENDED);
			if (m_FlushThread) {
				((CPartFileFlushThread*) m_FlushThread)->SetPartFile(this);
				((CPartFileFlushThread*) m_FlushThread)->ResumeThread();
				return;
			}
		}
		//MORPH END - Added by SiRoB, Flush Thread // NEO: FFT END <-- Xanatos --

		bool bCheckDiskspace = thePrefs.IsCheckDiskspaceEnabled() && thePrefs.GetMinFreeDiskSpace() > 0;
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
		uint64 previouspos = (uint64)-1; //MORPH - Optimization // NEO: FFT - [FileFlushThread] <-- Xanatos --
		for (int i = m_BufferedData_list.GetCount(); i>0; i--)
		{
			// Get top item
			item = m_BufferedData_list.GetHead();

			//MORPH START - Added by SiRoB, Flush Thread // NEO: FFT - [FileFlushThread] -- Xanatos -->
			if (item->data == NULL) {
				m_BufferedData_list.RemoveHead();
				delete item;
				continue;
			}
			//MORPH END - Added by SiRoB, Flush Thread // NEO: FFT END <-- Xanatos --

			// This is needed a few times
			uint32 lenData = (uint32)(item->end - item->start + 1);

			// SLUGFILLER: SafeHash - could be more than one part
			for (uint32 curpart = (uint32)(item->start/PARTSIZE); curpart <= item->end/PARTSIZE; curpart++)
				changedPart[curpart] = true;
			// SLUGFILLER: SafeHash

			// Go to the correct position in file and write block of data
			if (previouspos != item->start) //MORPH - Optimization // NEO: FFT - [FileFlushThread] <-- Xanatos --
			m_hpartfile.Seek(item->start, CFile::begin);
			m_hpartfile.Write(item->data, lenData);
			previouspos = item->end + 1; //MORPH - Optimization // NEO: FFT - [FileFlushThread] <-- Xanatos --

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

		//MORPH START - Added by SiRoB, Flush Thread // NEO: FFT - [FileFlushThread] -- Xanatos -->
		m_FlushSetting->bIncreasedFile = bIncreasedFile;
		FlushDone();
		//MORPH END - Added by SiRoB, Flush Thread // NEO: FFT END <-- Xanatos --
/*
		// Check each part of the file
		uint32 partRange = (UINT)((m_hpartfile.GetLength() % PARTSIZE > 0) ? ((m_hpartfile.GetLength() % PARTSIZE) - 1) : (PARTSIZE - 1));
		for (int iPartNumber = partCount-1; iPartNumber >= 0; iPartNumber--)
		{
			UINT uPartNumber = iPartNumber; // help VC71...
			if (changedPart[uPartNumber] == false)
			{
				// Any parts other than last must be full size
				partRange = PARTSIZE - 1;
				continue;
			}

			// Is this 9MB part complete
			if (IsComplete(PARTSIZE * (uint64)uPartNumber, (PARTSIZE * (uint64)(uPartNumber + 1)) - 1, false))
			{
				// Is part corrupt
				if (!HashSinglePart(uPartNumber))
				{
					LogWarning(LOG_STATUSBAR, GetResString(IDS_ERR_PARTCORRUPT), uPartNumber, GetFileName());
					AddGap(PARTSIZE*(uint64)uPartNumber, PARTSIZE*(uint64)uPartNumber + partRange);

					// add part to corrupted list, if not already there
					if (!IsCorruptedPart(uPartNumber))
						corrupted_list.AddTail((uint16)uPartNumber);

					// request AICH recovery data
					if (!bNoAICH)
						RequestAICHRecovery((uint16)uPartNumber);

					// update stats
					m_uCorruptionLoss += (partRange + 1);
					thePrefs.Add2LostFromCorruption(partRange + 1);
				}
				else
				{
					if (!hashsetneeded){
						if (thePrefs.GetVerbose())
							AddDebugLogLine(DLP_VERYLOW, false, _T("Finished part %u of \"%s\""), uPartNumber, GetFileName());
					}

					// tell the blackbox about the verified data
					m_CorruptionBlackBox.VerifiedData(PARTSIZE*(uint64)uPartNumber, PARTSIZE*(uint64)uPartNumber + partRange);

					// if this part was successfully completed (although ICH is active), remove from corrupted list
					POSITION posCorrupted = corrupted_list.Find((uint16)uPartNumber);
					if (posCorrupted)
						corrupted_list.RemoveAt(posCorrupted);

					if (status == PS_EMPTY)
					{
						if (theApp.emuledlg->IsRunning()) // may be called during shutdown!
						{
							if (GetHashCount() == GetED2KPartHashCount() && !hashsetneeded)
							{
								// Successfully completed part, make it available for sharing
								SetStatus(PS_READY);
								//theApp.sharedfiles->SafeAddKFile(this); // NEO: SAFS - [ShowAllFilesInShare] <-- Xanatos --
							}
						}
					}
				}
			}
			else if (IsCorruptedPart(uPartNumber) && (thePrefs.IsICHEnabled() || bForceICH))
			{
				// Try to recover with minimal loss
				if (HashSinglePart(uPartNumber))
				{
					m_uPartsSavedDueICH++;
					thePrefs.Add2SessionPartsSavedByICH(1);

					uint32 uRecovered = (uint32)GetTotalGapSizeInPart(uPartNumber);
					FillGap(PARTSIZE*(uint64)uPartNumber, PARTSIZE*(uint64)uPartNumber + partRange);
					RemoveBlockFromList(PARTSIZE*(uint64)uPartNumber, PARTSIZE*(uint64)uPartNumber + partRange);

					// tell the blackbox about the verified data
					m_CorruptionBlackBox.VerifiedData(PARTSIZE*(uint64)uPartNumber, PARTSIZE*(uint64)uPartNumber + partRange);

					// remove from corrupted list
					POSITION posCorrupted = corrupted_list.Find((uint16)uPartNumber);
					if (posCorrupted)
						corrupted_list.RemoveAt(posCorrupted);

					AddLogLine(true, GetResString(IDS_ICHWORKED), uPartNumber, GetFileName(), CastItoXBytes(uRecovered, false, false));

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

					if (status == PS_EMPTY)
					{
						if (theApp.emuledlg->IsRunning()) // may be called during shutdown!
						{
							if (GetHashCount() == GetED2KPartHashCount() && !hashsetneeded)
							{
								// Successfully recovered part, make it available for sharing
								SetStatus(PS_READY);
								//theApp.sharedfiles->SafeAddKFile(this); // NEO: SAFS - [ShowAllFilesInShare] <-- Xanatos --
							}
						}
					}
				}
			}
			// NEO: SCV - [SubChunkVerification] -- Xanatos -->
			else if(NeoPrefs.UseSubChunkTransfer() && !IsPureGap((uint64)uPartNumber*PARTSIZE, (uint64)(uPartNumber + 1)*PARTSIZE - 1))
			{
				VerifyIncompletePart((uint16)uPartNumber);
			}
			// NEO: SCV END <-- Xanatos --

			// Any parts other than last must be full size
			partRange = PARTSIZE - 1;
		}

		// Update met file
		SavePartFile();

		if (theApp.emuledlg->IsRunning()) // may be called during shutdown!
		{
			// Is this file finished?
			if (gaplist.IsEmpty())
				CompleteFile(false);

			// Check free diskspace
			//
			// Checking the free disk space again after the file was written could most likely be avoided, but because
			// we do not use real physical disk allocation units for the free disk computations, it should be more safe
			// and accurate to check the free disk space again, after file was written and buffers were flushed to disk.
			//
			// If useing a normal file, we could avoid the check disk space if the file was not increased.
			// If useing a compressed or sparse file, we always have to check the space 
			// regardless whether the file was increased in size or not.
			if (bCheckDiskspace && ((IsNormalFile() && bIncreasedFile) || !IsNormalFile()))
			{
				switch(GetStatus())
				{
				case PS_PAUSED:
				case PS_ERROR:
				case PS_COMPLETING:
				case PS_COMPLETE:
				case PS_MOVING: // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
					break;
				default:
					if (GetFreeDiskSpaceX(GetTempPath()) < thePrefs.GetMinFreeDiskSpace())
					{
						if (IsNormalFile())
						{
							// Normal files: pause the file only if it would still grow
							if (GetNeededSpace() > 0)
								PauseFile(true/bInsufficient/);
						}
						else
						{
							// Compressed/sparse files: always pause the file
							PauseFile(true/bInsufficient/);
						}
					}
				}
			}
		}
*/
	}
	catch (CFileException* error)
	{
		FlushBuffersExceptionHandler(error);	
		//MORPH START - Added by SiRoB, Flush Thread // NEO: FFT - [FileFlushThread] -- Xanatos -->
		delete[] changedPart;
		if (m_FlushSetting) {
			delete m_FlushSetting;
			m_FlushSetting = NULL;
		}
		//MORPH END - Added by SiRoB, Flush Thread // NEO: FFT END <-- Xanatos --
	}
#ifndef _DEBUG
	catch(...)
	{
		FlushBuffersExceptionHandler();
		//MORPH START - Added by SiRoB, Flush Thread // NEO: FFT - [FileFlushThread] -- Xanatos -->
		delete[] changedPart;
		if (m_FlushSetting) {
			delete m_FlushSetting;
			m_FlushSetting = NULL;
		}
		//MORPH END - Added by SiRoB, Flush Thread // NEO: FFT END <-- Xanatos --
	}
#endif
	//delete[] changedPart; //MORPH - removed by SiRoB, Flush Thread // NEO: FFT - [FileFlushThread] <-- Xanatos --
}

//MORPH START - Added by SiRoB, Flush Thread // NEO: FFT - [FileFlushThread] -- Xanatos -->
void CPartFile::WriteToDisk() { //Called by Flush Thread
	bool bCheckDiskspace = thePrefs.IsCheckDiskspaceEnabled() && thePrefs.GetMinFreeDiskSpace() > 0;
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

		// SLUGFILLER: SafeHash - could be more than one part
		for (uint32 curpart = (uint32)(item->start/PARTSIZE); curpart <= item->end/PARTSIZE; curpart++)
			changedPart[curpart] = true;
		// SLUGFILLER: SafeHash

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
	if (hashlist.GetCount() == GetED2KPartHashCount()){ // SLUGFILLER: SafeHash // NEO: SSH - [SlugFillerSafeHash]
		UINT partCount = GetPartCount();
		// Check each part of the file
		//uint32 partRange = (UINT)((m_hpartfile.GetLength() % PARTSIZE > 0) ? ((m_hpartfile.GetLength() % PARTSIZE) - 1) : (PARTSIZE - 1)); // SLUGFILLER: SafeHash - removed
		for (int partNumber = partCount-1; partNumber >= 0; partNumber--)
		{
			if (!m_FlushSetting->changedPart[partNumber])
			{
				// Any parts other than last must be full size
				//partRange = PARTSIZE - 1; // SLUGFILLER: SafeHash - removed // NEO: SSH - [SlugFillerSafeHash]
				continue;
			}
			// Any parts other than last must be full size
			if (!GetPartHash(partNumber)) {
				LogError(LOG_STATUSBAR, GetResString(IDS_ERR_INCOMPLETEHASH), GetFileName());
				hashsetneeded = true;
				ASSERT(FALSE);	// If this fails, something was seriously wrong with the hashset loading or the check above
			}

			// Is this 9MB part complete
			//MORPH - Changed by SiRoB, As we are using flushed data check asynchronously we need to check if all data have been written into the file buffer
			/*
			if (IsComplete(PARTSIZE * (uint64)partNumber, (PARTSIZE * (uint64)(partNumber + 1)) - 1, false))
			*/
			if (IsComplete(PARTSIZE * (uint64)partNumber, (PARTSIZE * (uint64)(partNumber + 1)) - 1, true))
			{
				// Is part corrupt
				// Let's check in another thread
				//m_PartsHashing++;
				if (theApp.emuledlg->IsRunning()) { //MORPH - Flush Thread
					//m_BlockMaps.RemoveKey((uint16)uPartNumber); // NEO: SCV - [SubChunkVerification] // David: we have already hashed many blocks with AICH lets hash the last one to
					// NEO: SCV - [SubChunkVerification]
					if (NeoPrefs.UseSubChunkTransfer())
						SetBlockPartHash((uint16)partNumber,false,false/*true*/);
					// NEO: SCV - END

					// Note: we could order the MD4 hash when we are done with the AICH hashing
					//  excepted the AICH data are not available than we would Hash MD4 first and AICH when its available
					//  Howeever as we do always booth hash's I decided to order them at once

					SetSinglePartHash((uint16)partNumber); // SLUGFILLER: SafeHash // NEO: SSH - [SlugFillerSafeHash]
				//MORPH START - Flush Thread
				} else { 
					if (!HashSinglePart(partNumber))
						PartHashFinished(partNumber, true);
					else
						PartHashFinished(partNumber, false);
				}
				//MORPH END   - Flush Thread

				/*// Is part corrupt
				if (!HashSinglePart(partNumber))
				{
					LogWarning(LOG_STATUSBAR, GetResString(IDS_ERR_PARTCORRUPT), partNumber, GetFileName());
					AddGap(PARTSIZE*(uint64)partNumber, PARTSIZE*(uint64)partNumber + partRange);

					// add part to corrupted list, if not already there
					if (!IsCorruptedPart(partNumber))
						corrupted_list.AddTail((uint16)partNumber);

					// request AICH recovery data
					if (!m_FlushSetting->bNoAICH)
						RequestAICHRecovery(partNumber);

					// update stats
					m_uCorruptionLoss += (partRange + 1);
					thePrefs.Add2LostFromCorruption(partRange + 1);
				}
				else
				{
					if (!hashsetneeded){
						if (thePrefs.GetVerbose())
							AddDebugLogLine(DLP_VERYLOW, false, _T("Finished part %u of \"%s\""), partNumber, GetFileName());
					}

					// tell the blackbox about the verified data
					m_CorruptionBlackBox.VerifiedData(PARTSIZE*(uint64)partNumber, PARTSIZE*(uint64)partNumber + partRange);

					// if this part was successfully completed (although ICH is active), remove from corrupted list
					POSITION posCorrupted = corrupted_list.Find((uint16)partNumber);
					if (posCorrupted)
						corrupted_list.RemoveAt(posCorrupted);

					if (status == PS_EMPTY)
					{
						if (theApp.emuledlg->IsRunning()) // may be called during shutdown!
						{
							if (GetHashCount() == GetED2KPartHashCount() && !hashsetneeded)
							{
								// Successfully completed part, make it available for sharing
								SetStatus(PS_READY);
								//theApp.sharedfiles->SafeAddKFile(this); // NEO: SAFS - [ShowAllFilesInShare] <-- Xanatos --
							}
						}
					}
				}*/
			}
			else if (IsCorruptedPart(partNumber) && (thePrefs.IsICHEnabled() || m_FlushSetting->bForceICH))
			{
				if (theApp.emuledlg->IsRunning()) {
					SetSinglePartHash((uint16)partNumber, true); // SLUGFILLER: SafeHash  // NEO: SSH - [SlugFillerSafeHash]
				//MORPH START - Flush Thread
				} else { 
					if (!HashSinglePart(partNumber))
						PartHashFinished(partNumber, true, true);
					else
						PartHashFinished(partNumber, false, true);
				}
				//MORPH END   - Flush Thread


				/*// Try to recover with minimal loss
				if (HashSinglePart(partNumber))
				{
					m_uPartsSavedDueICH++;
					thePrefs.Add2SessionPartsSavedByICH(1);

					uint32 uRecovered = (uint32)GetTotalGapSizeInPart(partNumber);
					FillGap(PARTSIZE*(uint64)partNumber, PARTSIZE*(uint64)partNumber + partRange);
					RemoveBlockFromList(PARTSIZE*(uint64)partNumber, PARTSIZE*(uint64)partNumber + partRange);

					// tell the blackbox about the verified data
					m_CorruptionBlackBox.VerifiedData(PARTSIZE*(uint64)partNumber, PARTSIZE*(uint64)partNumber + partRange);

					// remove from corrupted list
					POSITION posCorrupted = corrupted_list.Find((uint16)partNumber);
					if (posCorrupted)
						corrupted_list.RemoveAt(posCorrupted);

					AddLogLine(true, GetResString(IDS_ICHWORKED), partNumber, GetFileName(), CastItoXBytes(uRecovered, false, false));

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

					if (status == PS_EMPTY)
					{
						if (theApp.emuledlg->IsRunning()) // may be called during shutdown!
						{
							if (GetHashCount() == GetED2KPartHashCount() && !hashsetneeded)
							{
								// Successfully recovered part, make it available for sharing
								SetStatus(PS_READY);
								//theApp.sharedfiles->SafeAddKFile(this); // NEO: SAFS - [ShowAllFilesInShare] <-- Xanatos --
							}
						}
					}
				}*/
			}
			// NEO: SCV - [SubChunkVerification]
			else if(NeoPrefs.UseSubChunkTransfer() && !IsPureGap((uint64)partNumber*PARTSIZE, (uint64)(partNumber + 1)*PARTSIZE - 1))
			{
				SetBlockPartHash((uint16)partNumber);
			}
			// NEO: SCV END

			// Any parts other than last must be full size
			//partRange = PARTSIZE - 1; // NEO: SSH - [SlugFillerSafeHash]
		}
	}
	else {
		//ASSERT(GetED2KPartCount() > 1);	// Files with only 1 chunk should have a forced hashset
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_HASHERRORWARNING), GetFileName());
		hashsetneeded = true;
	}
	// SLUGFILLER: SafeHash
	
	// Update met file
	//SavePartFile(); //MORPH - Flush Thread Moved Down
	
	if (theApp.emuledlg->IsRunning()) // may be called during shutdown!
	{
		// SLUGFILLER: SafeHash remove - Don't perform file completion here
		//if (gaplist.IsEmpty())
		//if ( gaplist.IsEmpty() && !m_nTotalBufferData && !m_FlushThread) //MORPH - Changed by SiRoB, Flush Thread
		//		CompleteFile(false);
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
		bool bCheckDiskspace = thePrefs.IsCheckDiskspaceEnabled() && thePrefs.GetMinFreeDiskSpace() > 0;
		if (bCheckDiskspace && ((IsNormalFile() && m_FlushSetting->bIncreasedFile) || !IsNormalFile()))
		{
			switch(GetStatus())
			{
			case PS_PAUSED:
			case PS_ERROR:
			case PS_COMPLETING:
			case PS_COMPLETE:
			case PS_MOVING: // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
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
}	

int CPartFileFlushThread::Run()
{
	DbgSetThreadName("Partfile-Flushing");
	InitThreadLocale(); //Performance killer

	// SLUGFILLER: SafeHash // NEO: STS - [SlugFillerThreadSafe]
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return 0;
	// SLUGFILLER: SafeHash // NEO: STS END

	//theApp.QueueDebugLogLine(false,_T("FLUSH:Start (%s)"),m_partfile->GetFileName()/*, CastItoXBytes(myfile->m_iAllocinfo, false, false)*/ );

	try{
		CSingleLock sLock(&(theApp.hashing_mut), TRUE); //MORPH - Wait any Read/Write access (hashing or download stuff) before flushing
		m_partfile->WriteToDisk();
	}
	catch (CFileException* error)
	{
		VERIFY( PostMessage(theApp.emuledlg->m_hWnd,TM_FILEALLOCEXC,(WPARAM)m_partfile,(LPARAM)error) );
		delete[] m_partfile->m_FlushSetting->changedPart;
		delete m_partfile->m_FlushSetting;
		m_partfile->m_FlushSetting = NULL;
		m_partfile->m_FlushThread = NULL;
		return 1;
	}
#ifndef _DEBUG
	catch(...)
	{
		VERIFY( PostMessage(theApp.emuledlg->m_hWnd,TM_FILEALLOCEXC,(WPARAM)m_partfile,0) );
		delete[] m_partfile->m_FlushSetting->changedPart;
		delete m_partfile->m_FlushSetting;
		m_partfile->m_FlushSetting = NULL;
		m_partfile->m_FlushThread = NULL;
		return 2;
	}
#endif
	m_partfile->m_FlushThread = NULL;
	VERIFY( PostMessage(theApp.emuledlg->m_hWnd,TM_FLUSHDONE,0,(LPARAM)m_partfile) );
	//theApp.QueueDebugLogLine(false,_T("FLUSH:End (%s)"),m_partfile->GetFileName());
	return 0;
}
//MORPH END - Added by SiRoB, Flush Thread // NEO: FFT END <-- Xanatos --

void CPartFile::FlushBuffersExceptionHandler(CFileException* error)
{
	if (thePrefs.IsCheckDiskspaceEnabled() && error->m_cause == CFileException::diskFull)
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
			if (thePrefs.IsCheckDiskspaceEnabled() && thePrefs.GetMinFreeDiskSpace()==0)
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
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
		landatarate = 0;
#endif //LANCAST // NEO: NLC END <-- Xanatos --
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
		voodoodatarate = 0;
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
		//m_anStates[DS_DOWNLOADING] = 0; // NEO: FIX - [SourceCount] <-- Xanatos --
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
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	landatarate = 0;
#endif //LANCAST // NEO: NLC END <-- Xanatos --
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	voodoodatarate = 0;
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
	//m_anStates[DS_DOWNLOADING] = 0; // NEO: FIX - [SourceCount] <-- Xanatos --
	if (theApp.emuledlg->IsRunning()) // may be called during shutdown!
		UpdateDisplayedInfo();
}

UINT AFX_CDECL CPartFile::AllocateSpaceThread(LPVOID lpParam)
{
	DbgSetThreadName("Partfile-Allocate Space");
	InitThreadLocale();

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
	if (gaplist.GetHeadPosition() == NULL )
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

		// ToDo: here we have a problem - it occured that eMule crashed because of "best==NULL" while
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

	CKnownFile::UpdateFileRatingCommentAvail(bForceUpdate);	// NEO: XC - [ExtendedComments] <-- Xanatos --

	/*m_bHasComment = false;
	UINT uRatings = 0;
	UINT uUserRatings = 0;

	for (POSITION pos = srclist.GetHeadPosition(); pos != NULL;)
	{
		const CUpDownClient* cur_src = srclist.GetNext(pos);
		// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
		CClientFileStatus* status = cur_src->GetFileStatus(this);
		if (status && !m_bHasComment && status->HasFileComment())
			m_bHasComment = true;
		if (status && status->HasFileRating())
		{
			uRatings++;
			uUserRatings += status->GetFileRating();
		}
		// NEO: SCFS END <-- Xanatos --
		//if (!m_bHasComment && cur_src->HasFileComment())
		//	m_bHasComment = true;
		//if (cur_src->HasFileRating())
		//{
		//	uRatings++;
		//	uUserRatings += cur_src->GetFileRating();
		//}
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
		m_uUserRating = 0;*/

	if (bOldHasComment != m_bHasComment || uOldUserRatings != m_uUserRating || bForceUpdate)
		UpdateDisplayedInfo(true);
}

void CPartFile::UpdateDisplayedInfo(bool force)
{
	if (theApp.emuledlg->IsRunning()){
		DWORD curTick = ::GetTickCount();

        if(force || curTick-m_lastRefreshedDLDisplay > MINWAIT_BEFORE_DLDISPLAY_WINDOWUPDATE+m_random_update_wait) {
			theApp.emuledlg->transferwnd->downloadlistctrl.UpdateItem(this);
			m_lastRefreshedDLDisplay = curTick;
		}
	}
}

void CPartFile::UpdateAutoDownPriority(){
	// Small optimization: Do not bother with all this if file is paused or stopped.
	if( !IsAutoDownPriority() || GetStatus()==PS_PAUSED || IsStopped() ) // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
	//if( !IsAutoDownPriority() )
		return;
	UINT nHighestSC = theApp.downloadqueue->GetHighestAvailableSourceCount(); // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
	//if ( GetSourceCount() > 100 ){
	if ( GetAvailableSrcCount() > (nHighestSC * .40) ) { // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
		SetDownPriority( PR_LOW );
		return;
	}
	//if ( GetSourceCount() > 20 ){
	if ( GetAvailableSrcCount() > (nHighestSC * .20) ) { // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
		SetDownPriority( PR_NORMAL );
		return;
	}
	SetDownPriority( PR_HIGH );
}

// NEO: NSC - [NeoSharedCategories] -- Xanatos --
//UINT CPartFile::GetCategory() /*const*/
//{
//	if (m_category > (UINT)(thePrefs.GetCatCount() - 1))
//		m_category = 0;
//	return m_category;
//}

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

//void CPartFile::SetCategory(UINT cat)
void CPartFile::SetCategory(UINT cat, uint8 init) // NEO: MOD - [SetCategory]
{
	// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
	if(init == 1) // we are initialising category preferences for a new download
	{
		// set the right category
		if((int) cat == -1)
		{
			cat = 0;
			if (NeoPrefs.UseAutoCat())
				cat = theApp.downloadqueue->GetAutoCat(GetFileName(), GetFileSize());
			if(!cat && NeoPrefs.UseActiveCatForLinks()) // no auto cateory selected use default or active
				cat = theApp.emuledlg->transferwnd->GetActiveCategory();
		}

		if (NeoPrefs.SmallFileDLPush() && GetFileSize() < NeoPrefs.SmallFileDLPushSizeB()){
			m_catResumeOrder = 0;
		}else{ 
			m_catResumeOrder = theApp.downloadqueue->GetMaxCatResumeOrder(cat);
			if (NeoPrefs.AutoSetResumeOrder())
				m_catResumeOrder++;
		}
	}
	// NEO: NXC END <-- Xanatos --

	// NEO: NSC - [NeoSharedCategories] -- Xanatos -->
	if(IsPartFile() && (int)cat > thePrefs.GetCatCount() -1) // partfiles can not be stored in shared categories
		return;

	CKnownFile::SetCategory(cat, init);
	// NEO: NSC END <-- Xanatos --
	//m_category=cat;
	
	// NEO: FCFG - [FileConfiguration] -- Xanatos -->
	Category_Struct* Category = thePrefs.GetCategory(cat);
	CPartPreferences* newPartPrefs = Category ? Category->PartPrefs : NULL;
	if(newPartPrefs){
		if(PartPrefs->IsGlobalPrefs() || PartPrefs->IsCategoryPrefs())
			PartPrefs = newPartPrefs;
		else
			((CPartPreferencesEx*)PartPrefs)->PartPrefs = newPartPrefs;
	}else{
		if(PartPrefs->IsCategoryPrefs())
			PartPrefs = &NeoPrefs.PartPrefs;
		else if(PartPrefs->IsFilePrefs())
			((CPartPreferencesEx*)PartPrefs)->PartPrefs = &NeoPrefs.PartPrefs;
	}
	// NEO: FCFG END <-- Xanatos --

// ZZ:DownloadManager -->
	// set new prio
	if (IsPartFile() && !init){ // NEO: FCFG - [FileConfiguration] <-- Xanatos --
		SavePartFile();
	}
// <-- ZZ:DownloadManager
}

// NEO: FCFG - [FileConfiguration] -- Xanatos -->
void CPartFile::UpdatePartPrefs(CPartPreferences* cfgPartPrefs)
{
	if(cfgPartPrefs->IsEmpty())
	{
		if(cfgPartPrefs != PartPrefs)
		{
			ASSERT(!PartPrefs->IsFilePrefs());

			delete cfgPartPrefs;
		}
		else if(cfgPartPrefs == PartPrefs)
		{
			Category_Struct* Category = thePrefs.GetCategory(GetCategory());
			CPartPreferences* newPartPrefs = Category ? Category->PartPrefs : NULL;
			if(newPartPrefs)
				PartPrefs = newPartPrefs;
			else
				PartPrefs = &NeoPrefs.PartPrefs;
			delete cfgPartPrefs;
		}
	}
	else //if(!cfgPartPrefs->IsEmpty())
	{
		if(cfgPartPrefs != PartPrefs)
		{
			ASSERT(!PartPrefs->IsFilePrefs());

			((CPartPreferencesEx*)cfgPartPrefs)->PartFile = this;
			PartPrefs = cfgPartPrefs;

			Category_Struct* Category = thePrefs.GetCategory(GetCategory());
			CPartPreferences* newPartPrefs = Category ? Category->PartPrefs : NULL;
			if(newPartPrefs)
				((CPartPreferencesEx*)cfgPartPrefs)->PartPrefs = newPartPrefs;
			else
				((CPartPreferencesEx*)cfgPartPrefs)->PartPrefs = &NeoPrefs.PartPrefs;
		}
		else // if(cfgPartPrefs == PartPrefs)
		{
			ASSERT(PartPrefs->IsFilePrefs());
		}
	}
}

bool CPartFile::HasPreferences(){
	if(!IsPartFile())
		return CKnownFile::HasPreferences();
	return PartPrefs->IsFilePrefs() || KnownPrefs->IsFilePrefs();
}
// NEO: FCFG END <-- Xanatos --

void CPartFile::_SetStatus(EPartFileStatus eStatus)
{
	// NOTE: This function is meant to be used from *different* threads -> Do *NOT* call
	// any GUI functions from within here!!
	//ASSERT( eStatus != PS_PAUSED && eStatus != PS_INSUFFICIENT );
	ASSERT( eStatus != PS_INSUFFICIENT ); // NEO: VOODOO - [UniversalPartfileInterface] <-- Xanatso --
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
		theApp.emuledlg->transferwnd->downloadlistctrl.UpdateCurrentCategoryView(this);
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

// NEO: FIX - [SourceCount] -- Xanatos -->
void CPartFile::IncrSrcStatisticsValue(EDownloadState nDLState)
{
	if(stopped)
		return;
	ASSERT( nDLState < ARRSIZE(m_anStates) );
	m_anStates[nDLState]++;
}
void CPartFile::DecrSrcStatisticsValue(EDownloadState nDLState)
{
	if(stopped)
		return;
	ASSERT( nDLState < ARRSIZE(m_anStates) );
	ASSERT(m_anStates[nDLState]);
	if(m_anStates[nDLState])
		m_anStates[nDLState]--;
}
// NEO: FIX END <-- Xanatos --

UINT CPartFile::GetTransferringSrcCount() const
{
	// NEO: FIX - [SourceCount] -- Xanatos -->
	if(insufficient == true || paused == true)
		return 0;
	// NEO: FIX END <-- Xanatos --
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

bool CPartFile::GetNextRequestedBlock(CUpDownClient* sender, 
                                      Requested_Block_Struct** newblocks, 
									  uint16* count,
									  uint32 blocksize) /*const*/ // NEO: DBR - [DynamicBlockRequest] <-- Xanatos --
{
	// The purpose of this function is to return a list of blocks (~180KB) to
	// download. To avoid a prematurely stop of the downloading, all blocks that 
	// are requested from the same source must be located within the same 
	// chunk (=> part ~9MB).
	//  
	// The selection of the chunk to download is one of the CRITICAL parts of the 
	// edonkey network. The selection algorithm must insure the best spreading
	// of files.
	//  
	// The selection is based on several criteria:
	//  -   Frequency of the chunk (availability), very rare chunks must be downloaded 
	//      as quickly as possible to become a new available source.
	//  -   Parts used for preview (first + last chunk), preview or check a 
	//      file (e.g. movie, mp3)
	//  -   Completion (shortest-to-complete), partially retrieved chunks should be 
	//      completed before starting to download other one.
	//  
	// The frequency criterion defines several zones: very rare, rare, almost rare,
	// and common. Inside each zone, the criteria have a specific ‘weight’, used 
	// to calculate the priority of chunks. The chunk(s) with the highest 
	// priority (highest=0, lowest=0xffff) is/are selected first.
	//  
	// This algorithm usually selects first the rarest chunk(s). However, partially
	// complete chunk(s) that is/are close to completion may overtake the priority 
	// (priority inversion). For common chunks, it also tries to put the transferring
    // clients on the same chunk, to complete it sooner.
	//

	// Check input parameters
	if(count == 0)
		return false;
	//if(sender->GetPartStatus() == NULL)
	//	return false;
	// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
	CClientFileStatus* srcFileStatus = sender->GetFileStatus(this); 
	if(srcFileStatus == NULL || srcFileStatus->GetPartStatus() == NULL)
		return false;
	// NEO: SCFS END <-- Xanatos --

    //AddDebugLogLine(DLP_VERYLOW, false, _T("Evaluating chunks for file: \"%s\" Client: %s"), GetFileName(), sender->DbgGetClientInfo());
    
	// Define and create the list of the chunks to download
	const uint16 partCount = GetPartCount();
	CList<Chunk> chunksList(partCount);

    uint16 tempLastPartAsked = (uint16)-1;
    if(sender->m_lastPartAsked != ((uint16)-1) && sender->GetClientSoft() == SO_EMULE && sender->GetVersion() < MAKE_CLIENT_VERSION(0, 43, 1)){
        tempLastPartAsked = sender->m_lastPartAsked;
    }

	// NEO: SCT - [SubChunkTransfer] -- Xanatos -->
	const bool bSCT = NeoPrefs.UseSubChunkTransfer() == TRUE;
	tBlockMap* blockMap = NULL;
	// NEO: SCT END <-- Xanatos --

	// Main loop
	uint16 newBlockCount = 0;
	while(newBlockCount != *count){
		// Create a request block stucture if a chunk has been previously selected
		if(tempLastPartAsked != (uint16)-1){
			Requested_Block_Struct* pBlock = new Requested_Block_Struct;
			// NEO: SCT - [SubChunkTransfer] -- Xanatos -->
			if(bSCT) 
				srcFileStatus->GetBlockMap(tempLastPartAsked,&blockMap);
			if(GetNextEmptyBlockInPart(tempLastPartAsked, blockMap, pBlock, blocksize) == true){ // NEO: DBR - [DynamicBlockRequest]
			// NEO: SCT END <-- Xanatos --
			//if(GetNextEmptyBlockInPart(tempLastPartAsked, pBlock) == true){
                //AddDebugLogLine(false, _T("Got request block. Interval %i-%i. File %s. Client: %s"), pBlock->StartOffset, pBlock->EndOffset, GetFileName(), sender->DbgGetClientInfo());
				// Keep a track of all pending requested blocks
				requestedblocks_list.AddTail(pBlock);
				// Update list of blocks to return
				newblocks[newBlockCount++] = pBlock;
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatso -->
				// tell all voodoo cleints that we are downloading thich block
				if(!IsVoodooFile() && KnownPrefs->IsEnableVoodoo())
					theApp.voodoo->ManifestThrottleBlock(this,pBlock->StartOffset,pBlock->EndOffset);
#endif // VOODOO // NEO: VOODOO END <-- Xanatso --
				// Skip end of loop (=> CPU load)
				continue;
			} 
			else {
				// All blocks for this chunk have been already requested
				delete pBlock;
				// => Try to select another chunk
				sender->m_lastPartAsked = tempLastPartAsked = (uint16)-1;
			}
		}

		// Check if a new chunk must be selected (e.g. download starting, previous chunk complete)
		if(tempLastPartAsked == (uint16)-1){

			// Quantify all chunks (create list of chunks to download) 
			// This is done only one time and only if it is necessary (=> CPU load)
			if(chunksList.IsEmpty() == TRUE){
				// Indentify the locally missing part(s) that this source has
				for(uint16 i = 0; i < partCount; i++){
					if( (srcFileStatus->IsPartAvailable(i) // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
					// NEO: SCT - [SubChunkTransfer] -- Xanatos -->
					 || bSCT && srcFileStatus->GetBlockMap(i,&blockMap)) 
					 && GetNextEmptyBlockInPart(i, blockMap, NULL, blocksize) == true){  // NEO: DBR - [DynamicBlockRequest]
					// NEO: SCT END <-- Xanatos --
					//if(sender->IsPartAvailable(i) == true && GetNextEmptyBlockInPart(i, NULL) == true){
						// Create a new entry for this chunk and add it to the list
						Chunk newEntry;
						newEntry.part = i;
						newEntry.frequency = m_SrcpartFrequency[i];
						chunksList.AddTail(newEntry);
					}
				}

				// Check if any block(s) could be downloaded
				if(chunksList.IsEmpty() == TRUE){
					break; // Exit main loop while()
				}

                // Define the bounds of the zones (very rare, rare etc)
				// more depending on available sources
				uint16 limit = (uint16)ceil(GetSourceCount()/ 10.0);
				if (limit<3) limit=3;

				const uint16 veryRareBound = limit;
				const uint16 rareBound = 2*limit;
				const uint16 almostRareBound = 4*limit;

				// Cache Preview state (Criterion 2)
                const bool isPreviewEnable = (thePrefs.GetPreviewPrio() || thePrefs.IsExtControlsEnabled() && GetPreviewPrio()) && IsPreviewableFileType();

				// Collect and calculate criteria for all chunks
				for(POSITION pos = chunksList.GetHeadPosition(); pos != NULL; ){
					Chunk& cur_chunk = chunksList.GetNext(pos);

					// Offsets of chunk
					UINT uCurChunkPart = cur_chunk.part; // help VC71...
					const uint64 uStart = (uint64)uCurChunkPart * PARTSIZE;
					const uint64 uEnd  = ((GetFileSize() - (uint64)1) < (uStart + PARTSIZE - 1)) ? 
										  (GetFileSize() - (uint64)1) : (uStart + PARTSIZE - 1);
					ASSERT( uStart <= uEnd );

					// Criterion 2. Parts used for preview
					// Remark: - We need to download the first part and the last part(s).
					//        - When the last part is very small, it's necessary to 
					//          download the two last parts.
					bool critPreview = false;
					if(isPreviewEnable == true){
						if(cur_chunk.part == 0){
							critPreview = true; // First chunk
						}
						else if(cur_chunk.part == partCount-1){
							critPreview = true; // Last chunk 
						}
						else if(cur_chunk.part == partCount-2){
							// Last chunk - 1 (only if last chunk is too small)
							if( (GetFileSize() - uEnd) < (uint64)PARTSIZE/3){
								critPreview = true; // Last chunk - 1
							}
						}
					}

					// Criterion 3. Request state (downloading in process from other source(s))
					//const bool critRequested = IsAlreadyRequested(uStart, uEnd);
                    bool critRequested = false; // <--- This is set as a part of the second critCompletion loop below

					// Criterion 4. Completion
					uint64 partSize = uEnd - uStart + 1; //If all is covered by gaps, we have downloaded PARTSIZE, or possibly less for the last chunk;
                    ASSERT(partSize <= PARTSIZE);
					for(POSITION pos = gaplist.GetHeadPosition(); pos != NULL; ) {
						const Gap_Struct* cur_gap = gaplist.GetNext(pos);
						// Check if Gap is into the limit
						if(cur_gap->start < uStart) {
							if(cur_gap->end > uStart && cur_gap->end < uEnd) {
                                ASSERT(partSize >= (cur_gap->end - uStart + 1));
								partSize -= cur_gap->end - uStart + 1;
							}
							else if(cur_gap->end >= uEnd) {
								partSize = 0;
								break; // exit loop for()
							}
						}
						else if(cur_gap->start <= uEnd) {
							if(cur_gap->end < uEnd) {
                                ASSERT(partSize >= (cur_gap->end - cur_gap->start + 1));
								partSize -= cur_gap->end - cur_gap->start + 1;
							}
							else {
                                ASSERT(partSize >= (uEnd - cur_gap->start + 1));
								partSize -= uEnd - cur_gap->start + 1;
							}
						}
					}
                    //ASSERT(partSize <= PARTSIZE && partSize <= (uEnd - uStart + 1));

                    // requested blocks from sources we are currently downloading from is counted as if already downloaded
                    // this code will cause bytes that has been requested AND transferred to be counted twice, so we can end
                    // up with a completion number > PARTSIZE. That's ok, since it's just a relative number to compare chunks.
                    for(POSITION reqPos = requestedblocks_list.GetHeadPosition(); reqPos != NULL; ) {
                        const Requested_Block_Struct* reqBlock = requestedblocks_list.GetNext(reqPos);
                        if(reqBlock->StartOffset < uStart) {
                            if(reqBlock->EndOffset > uStart) {
                                if(reqBlock->EndOffset < uEnd) {
                                    //ASSERT(partSize + (reqBlock->EndOffset - uStart + 1) <= (uEnd - uStart + 1));
								    partSize += reqBlock->EndOffset - uStart + 1;
                                    critRequested = true;
                                } else if(reqBlock->EndOffset >= uEnd) {
                                    //ASSERT(partSize + (uEnd - uStart + 1) <= uEnd - uStart);
                                    partSize += uEnd - uStart + 1;
                                    critRequested = true;
                                }
							}
                        } else if(reqBlock->StartOffset <= uEnd) {
							if(reqBlock->EndOffset < uEnd) {
                                //ASSERT(partSize + (reqBlock->EndOffset - reqBlock->StartOffset + 1) <= (uEnd - uStart + 1));
								partSize += reqBlock->EndOffset - reqBlock->StartOffset + 1;
                                critRequested = true;
							} else {
                                //ASSERT(partSize +  (uEnd - reqBlock->StartOffset + 1) <= (uEnd - uStart + 1));
								partSize += uEnd - reqBlock->StartOffset + 1;
                                critRequested = true;
							}
						}
                    }
                    //Don't check this (see comment above for explanation): ASSERT(partSize <= PARTSIZE && partSize <= (uEnd - uStart + 1));

                    if(partSize > PARTSIZE) partSize = PARTSIZE;

                    uint16 critCompletion = (uint16)ceil((double)(partSize*100)/PARTSIZE); // in [%]. Last chunk is always counted as a full size chunk, to not give it any advantage in this comparison due to smaller size. So a 1/3 of PARTSIZE downloaded in last chunk will give 33% even if there's just one more byte do download to complete the chunk.
                    if(critCompletion > 100) critCompletion = 100;

                    // Criterion 5. Prefer to continue the same chunk
                    const bool sameChunk = (cur_chunk.part == sender->m_lastPartAsked);

                    // Criterion 6. The more transferring clients that has this part, the better (i.e. lower).
                    uint16 transferringClientsScore = (uint16)m_downloadingSourceList.GetSize();

                    // Criterion 7. Sooner to completion (how much of a part is completed, how fast can be transferred to this part, if all currently transferring clients with this part are put on it. Lower is better.)
                    uint16 bandwidthScore = 2000;

                    // Calculate criterion 6 and 7
                    if(m_downloadingSourceList.GetSize() > 1) {
                        UINT totalDownloadDatarateForThisPart = 1;
                        for(POSITION downloadingClientPos = m_downloadingSourceList.GetHeadPosition(); downloadingClientPos != NULL; ) {
                            const CUpDownClient* downloadingClient = m_downloadingSourceList.GetNext(downloadingClientPos);
							if(srcFileStatus->IsPartAvailable(cur_chunk.part)) { // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
                            //if(downloadingClient->IsPartAvailable(cur_chunk.part)) {
                                transferringClientsScore--;
                                totalDownloadDatarateForThisPart += downloadingClient->GetDownloadDatarate() + 500; // + 500 to make sure that a unstarted chunk available at two clients will end up just barely below 2000 (max limit)
                            }
                        }

                        bandwidthScore = (uint16)min((UINT)((PARTSIZE-partSize)/(totalDownloadDatarateForThisPart*5)), 2000);
                        //AddDebugLogLine(DLP_VERYLOW, false,
                        //    _T("BandwidthScore for chunk %i: bandwidthScore = %u = min((PARTSIZE-partSize)/(totalDownloadDatarateForThisChunk*5), 2000) = min((PARTSIZE-%I64u)/(%u*5), 2000)"),
                        //    cur_chunk.part, bandwidthScore, partSize, totalDownloadDatarateForThisChunk);
                    }

                    //AddDebugLogLine(DLP_VERYLOW, false, _T("Evaluating chunk number: %i, SourceCount: %u/%i, critPreview: %s, critRequested: %s, critCompletion: %i%%, sameChunk: %s"), cur_chunk.part, cur_chunk.frequency, GetSourceCount(), ((critPreview == true) ? _T("true") : _T("false")), ((critRequested == true) ? _T("true") : _T("false")), critCompletion, ((sameChunk == true) ? _T("true") : _T("false")));

					// Calculate priority with all criteria
                    if(partSize > 0 && GetSourceCount() <= GetSrcA4AFCount()) {
						// If there are too many a4af sources, the completion of blocks have very high prio
						cur_chunk.rank = (cur_chunk.frequency) +                      // Criterion 1
							             ((critPreview == true) ? 0 : 200) +          // Criterion 2
										 ((critRequested == true) ? 0 : 1) +          // Criterion 3
										 (100 - critCompletion) +                     // Criterion 4
                                         ((sameChunk == true) ? 0 : 1) +              // Criterion 5
                                         bandwidthScore;                              // Criterion 7
                    } else if(cur_chunk.frequency <= veryRareBound){
						// 3000..xxxx unrequested + requested very rare chunks
						cur_chunk.rank = (75 * cur_chunk.frequency) +                 // Criterion 1
							             ((critPreview == true) ? 0 : 1) +            // Criterion 2
										 ((critRequested == true) ? 3000 : 3001) +    // Criterion 3
										 (100 - critCompletion) +                     // Criterion 4
                                         ((sameChunk == true) ? 0 : 1) +              // Criterion 5
                                         transferringClientsScore;                    // Criterion 6
					}
					else if(critPreview == true){
						// 10000..10100  unrequested preview chunks
						// 20000..20100  requested preview chunks
						cur_chunk.rank = ((critRequested == true &&
                                           sameChunk == false) ? 20000 : 10000) +     // Criterion 3
										 (100 - critCompletion);                      // Criterion 4
					}
					else if(cur_chunk.frequency <= rareBound){
						// 10101..1xxxx  requested rare chunks
						// 10102..1xxxx  unrequested rare chunks
                        //ASSERT(cur_chunk.frequency >= veryRareBound);

                        cur_chunk.rank = (25 * cur_chunk.frequency) +                 // Criterion 1 
										 ((critRequested == true) ? 10101 : 10102) +  // Criterion 3
										 (100 - critCompletion) +                     // Criterion 4
                                         ((sameChunk == true) ? 0 : 1) +              // Criterion 5
                                         transferringClientsScore;                    // Criterion 6
					}
					else if(cur_chunk.frequency <= almostRareBound){
						// 20101..1xxxx  requested almost rare chunks
						// 20150..1xxxx  unrequested almost rare chunks
                        //ASSERT(cur_chunk.frequency >= rareBound);

                        // used to slightly lessen the imporance of frequency
                        uint16 randomAdd = 1 + (uint16)((((uint32)rand()*(almostRareBound-rareBound))+(RAND_MAX/2))/RAND_MAX);
                        //AddDebugLogLine(DLP_VERYLOW, false, _T("RandomAdd: %i, (%i-%i=%i)"), randomAdd, rareBound, almostRareBound, almostRareBound-rareBound);

                        cur_chunk.rank = (cur_chunk.frequency) +                      // Criterion 1
										 ((critRequested == true) ? 20101 : (20201+almostRareBound-rareBound)) +  // Criterion 3
                                         ((partSize > 0) ? 0 : 500) +                 // Criterion 4
										 (5*100 - (5*critCompletion)) +               // Criterion 4
                                         ((sameChunk == true) ? (uint16)0 : randomAdd) +  // Criterion 5
                                         bandwidthScore;                              // Criterion 7
					}
					else { // common chunk
						// 30000..30100  requested common chunks
						// 30001..30101  unrequested common chunks
						cur_chunk.rank = ((critRequested == true) ? 30000 : 30001) +  // Criterion 3
										 (100 - critCompletion) +                     // Criterion 4
                                         ((sameChunk == true) ? 0 : 1) +              // Criterion 5
                                         bandwidthScore;                              // Criterion 7
					}

                    //AddDebugLogLine(DLP_VERYLOW, false, _T("Rank: %u"), cur_chunk.rank);
				}
			}

			// Select the next chunk to download
			if(chunksList.IsEmpty() == FALSE){

				// NEO: MCS - [ManualChunkSelection] -- Xanatos -->
				if(PartPrefs->HasWantedParts())
				{
					for(POSITION pos = chunksList.GetHeadPosition(); pos != NULL; ){
						POSITION cur_pos = pos;
						const Chunk& cur_chunk = chunksList.GetNext(pos);
						if(PartPrefs->GetWantedPart(cur_chunk.part))
						{
                            sender->m_lastPartAsked = tempLastPartAsked = cur_chunk.part;
                            //AddDebugLogLine(DLP_VERYLOW, false, _T("Chunk number %i selected as wanted by user"), cur_chunk.part);

							// Remark: this list might be reused up to ‘*count’ times
							chunksList.RemoveAt(cur_pos);
							break; // exit loop for()
						}
					}
					if(tempLastPartAsked != (uint16)-1)
						continue; // abort the chunk selection and got on top of the while loop
				}
				// NEO: MCS END <-- Xanatos --

				// Find and count the chunck(s) with the highest priority
				uint16 count = 0; // Number of found chunks with same priority
				uint16 rank = 0xffff; // Highest priority found
				for(POSITION pos = chunksList.GetHeadPosition(); pos != NULL; ){
					const Chunk& cur_chunk = chunksList.GetNext(pos);
					if(cur_chunk.rank < rank){
						count = 1;
						rank = cur_chunk.rank;
					}
					else if(cur_chunk.rank == rank){
						count++;
					}
				}

				// Use a random access to avoid that everybody tries to download the 
				// same chunks at the same time (=> spread the selected chunk among clients)
				uint16 randomness = 1 + (uint16)((((uint32)rand()*(count-1))+(RAND_MAX/2))/RAND_MAX);
				for(POSITION pos = chunksList.GetHeadPosition(); ; ){
					POSITION cur_pos = pos;
					const Chunk& cur_chunk = chunksList.GetNext(pos);
					if(cur_chunk.rank == rank){
						randomness--; 
						if(randomness == 0){
							// Selection process is over 
                            sender->m_lastPartAsked = tempLastPartAsked = cur_chunk.part;
                            //AddDebugLogLine(DLP_VERYLOW, false, _T("Chunk number %i selected. Rank: %u"), cur_chunk.part, cur_chunk.rank);

							// Remark: this list might be reused up to ‘*count’ times
							chunksList.RemoveAt(cur_pos);
							break; // exit loop for()
						}  
					}
				}
			}
			else {
				// There is no remaining chunk to download
				break; // Exit main loop while()
			}
		}
	}
	// Return the number of the blocks 
	*count = newBlockCount;
	
	// Return
	return (newBlockCount > 0);
}
// Maella end

// NEO: ICS - [InteligentChunkSelection] -- Xanatos -->
uint16* CPartFile::CalcDownloadingParts(CUpDownClient* client){
	if (!client)
		return NULL;

	uint16  partsCount = GetPartCount();
	if (!partsCount)
		return NULL;

	uint16* partsDownloading = new uint16[partsCount];
	ZeroMemory(partsDownloading, partsCount * sizeof(uint16));

	CUpDownClient* cur_client;
	POSITION pos = m_downloadingSourceList.GetHeadPosition();
	while (pos){
		cur_client = m_downloadingSourceList.GetNext(pos);
		uint16 clientPart = cur_client->m_lastPartAsked;
		if (cur_client != client && cur_client->GetRequestFile() && 
			!md4cmp(cur_client->GetRequestFile()->GetFileHash(), GetFileHash()) && 
			clientPart < partsCount && cur_client->GetDownloadDatarate() > 150)
			partsDownloading[clientPart]++;
	}
	return partsDownloading;
}

uint64 CPartFile::GetPartSizeToDownload(uint16 partNumber)
{
	Gap_Struct *currentGap;
	uint64 total, gap_start, gap_end, partStart, partEnd;

	total = 0;
	partStart = (uint64)(PARTSIZE * partNumber);
	partEnd = min(partStart + PARTSIZE, GetFileSize());

	for (POSITION pos = gaplist.GetHeadPosition(); pos != 0; gaplist.GetNext(pos))
	{
		currentGap = gaplist.GetAt(pos);

		if ((currentGap->start < partEnd) && (currentGap->end >= partStart))
		{
			gap_start = max(currentGap->start, partStart);
			gap_end = min(currentGap->end + 1, partEnd);
			total += (gap_end - gap_start); // I'm not sure this works: do gaps overlap?
		}
	}

	return min(total, partEnd - partStart); // This to limit errors: see note above
}

#define	CM_RELEASE_MODE			1
#define	CM_SPREAD_MODE			2
#define	CM_SHARE_MODE			3

#define	CM_SPREAD_MINSRC		10
#define	CM_SHARE_MINSRC			25
#define CM_MAX_SRC_CHUNK		3

bool CPartFile::GetNextRequestedBlockICS(CUpDownClient* sender, Requested_Block_Struct** newblocks, uint16* count, uint32 blocksize) // NEO: DBR - [DynamicBlockRequest]
{
	if (!(*count)) 
		return false;

	// NEO: SCFS - [SmartClientFileStatus]
	CClientFileStatus* srcFileStatus = sender->GetFileStatus(this); 
	if(srcFileStatus == NULL || srcFileStatus->GetPartStatus() == NULL)
		return false;
	// NEO: SCFS END

	// Select mode: RELEASE, SPREAD or SHARE

	uint16	part_idx;
	uint16	min_src = (uint16)-1;

	if (m_SrcpartFrequency.GetCount() < GetPartCount())
		min_src = 0;
	else
		for (part_idx = 0; part_idx < GetPartCount(); ++part_idx)
			if (m_SrcpartFrequency[part_idx] < min_src)
				min_src = m_SrcpartFrequency[part_idx];

	if (min_src <= CM_SPREAD_MINSRC)		m_ics_filemode = CM_RELEASE_MODE;
	else if (min_src <= CM_SHARE_MINSRC)	m_ics_filemode = CM_SPREAD_MODE;
	else									m_ics_filemode = CM_SHARE_MODE;

	// Chunk list ordered by preference

	CList<uint16,uint16> chunk_list;
	CList<uint32,uint32> chunk_pref;
	uint32 c_pref;
	uint32 complete_src;
	uint32 incomplete_src;
	uint32 first_last_mod;
	uint32 size2transfer;
	uint16* partsDownloading = CalcDownloadingParts(sender);

	const bool isPreviewEnable = (thePrefs.GetPreviewPrio() || thePrefs.IsExtControlsEnabled() && GetPreviewPrio()) && IsPreviewableFileType();
	const bool isWantedEnable = PartPrefs->HasWantedParts(); // NEO: MCS - [ManualChunkSelection]
	// NEO: SCT - [SubChunkTransfer]
	const bool bSCT = NeoPrefs.UseSubChunkTransfer() == TRUE;
	tBlockMap* blockMap = NULL;
	// NEO: SCT END

	if (isPreviewEnable || isWantedEnable) m_ics_filemode = CM_SHARE_MODE; // NEO: MCS - [ManualChunkSelection]

	for (part_idx = 0; part_idx < GetPartCount(); ++part_idx)
	{
		//if (sender->IsPartAvailable(part_idx) && GetNextEmptyBlockInPart(part_idx, 0, blocksize)) // NEO: DBR - [DynamicBlockRequest]
		if ( (srcFileStatus->IsPartAvailable(part_idx) // NEO: SCFS - [SmartClientFileStatus]
		// NEO: SCT - [SubChunkTransfer]
		 || bSCT && srcFileStatus->GetBlockMap(part_idx,&blockMap))
		 && GetNextEmptyBlockInPart(part_idx, blockMap, NULL, blocksize)) // NEO: DBR - [DynamicBlockRequest]
		// NEO: SCT END
		{
				complete_src = 0;
				incomplete_src = 0;
				first_last_mod = 0;

				// Chunk priority modifiers
				if (m_ics_filemode != CM_SHARE_MODE)
				{
					complete_src = m_SrcpartFrequency.GetCount() > part_idx ? m_SrcpartFrequency[part_idx] : 0;
					incomplete_src = m_SrcincpartFrequency.GetCount() > part_idx ? m_SrcincpartFrequency[part_idx] : 0;
				}
				if (m_ics_filemode != CM_RELEASE_MODE)
				{
					if (isPreviewEnable)
						if (part_idx == 0 || part_idx == (GetPartCount() - 1))		first_last_mod = 2;
						else if (part_idx == 1 || part_idx == (GetPartCount() - 2))	first_last_mod = 1;
						else														first_last_mod = 0;
					if (isWantedEnable && PartPrefs->GetWantedPart(part_idx)) 		first_last_mod = 3; // NEO: MCS - [ManualChunkSelection]
				}

				size2transfer = (uint32)GetPartSizeToDownload(part_idx);
				size2transfer = min(((size2transfer + (partsDownloading ? (uint64)PARTSIZE * partsDownloading[part_idx] / CM_MAX_SRC_CHUNK : (uint64)0) + 0xff) >> 8), 0xFFFF);

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

			} // END if part downloadable
		}

	if (partsDownloading)
		delete[] partsDownloading;

	//if(sender->m_lastPartAsked != 0xffff && sender->IsPartAvailable(sender->m_lastPartAsked) && GetNextEmptyBlockInPart(sender->m_lastPartAsked, 0, blocksize)){ // NEO: DBR - [DynamicBlockRequest]
	if(sender->m_lastPartAsked != 0xffff && 
	 (srcFileStatus->IsPartAvailable(sender->m_lastPartAsked) // NEO: SCFS - [SmartClientFileStatus]
	// NEO: SCT - [SubChunkTransfer]
	 || bSCT && srcFileStatus->GetBlockMap(part_idx,&blockMap))
	  && GetNextEmptyBlockInPart(sender->m_lastPartAsked, blockMap, NULL, blocksize)){  // NEO: DBR - [DynamicBlockRequest]
	// NEO: SCT - [SubChunkTransfer]
		chunk_list.AddHead(sender->m_lastPartAsked);
		chunk_pref.AddHead((uint32) 0);
	} 
	else {
		sender->m_lastPartAsked = 0xffff;
	}

	uint16 requestedCount = *count;
	uint16 newblockcount = 0;
	*count = 0;

	if (chunk_list.IsEmpty()) return false;

	Requested_Block_Struct* block = new Requested_Block_Struct;
	for (POSITION scan_chunks = chunk_list.GetHeadPosition(); scan_chunks; chunk_list.GetNext(scan_chunks))
	{
		sender->m_lastPartAsked = chunk_list.GetAt(scan_chunks);
		//while(GetNextEmptyBlockInPart(chunk_list.GetAt(scan_chunks), block, blocksize)) // NEO: DBR - [DynamicBlockRequest]
		// NEO: SCT - [SubChunkTransfer]
		if(bSCT) 
			srcFileStatus->GetBlockMap(chunk_list.GetAt(scan_chunks),&blockMap);
		while(GetNextEmptyBlockInPart(chunk_list.GetAt(scan_chunks),blockMap,block,blocksize)) // NEO: DBR - [DynamicBlockRequest]
		// NEO: SCT END
		{
			requestedblocks_list.AddTail(block);
			newblocks[newblockcount] = block;
			newblockcount++;
			*count = newblockcount;
			if (newblockcount == requestedCount)
				return true;
			block = new Requested_Block_Struct;
		}
	}
	delete block;

	if (!(*count)) return false; // useless, just to be sure
	return true;
}
// NEO: ICS END <-- Xanatos --

CString CPartFile::GetInfoSummary() const
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

	CString info;
	info.Format(_T("%s\n")
		+ GetResString(IDS_FD_HASH) + _T(" %s\n")
		+ GetResString(IDS_FD_SIZE) + _T(" %s  %s\n<br_head>\n")
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
		return CKnownFile::GrabImage(GetPath() + CString(_T("\\")) + GetFileName(true),nFramesToGrab, dStartTime, bReduceColor, nMaxWidth, pSender); // NEO: PP - [PasswordProtection] <-- Xanatos --
		//return CKnownFile::GrabImage(GetPath() + CString(_T("\\")) + GetFileName(),nFramesToGrab, dStartTime, bReduceColor, nMaxWidth, pSender);
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
		uint64 uGapSize = cur_gap->end - cur_gap->start;
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
	//m_ClientSrcAnswered = 2 * CONNECTION_LATENCY + ::GetTickCount() - SOURCECLIENTREASKS;
	m_ClientSrcAnswered = 2 * CONNECTION_LATENCY + ::GetTickCount() - PartPrefs->GetXsClientIntervalsMs(); // NEO: SRT - [SourceRequestTweaks] <-- Xanatos --
}

// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
bool CPartFile::CheckShowItemInGivenCat(int inCategory) /*const*/
{
	Category_Struct* curCat = thePrefs.GetCategory(inCategory);
	if (curCat == NULL)
		return false;

	//if (curCat->viewfilters.bSuspendFilters && (GetCategory() == inCategory || curCat->viewfilters.nFromCats == 0))
	if ((curCat->viewfilters.bSuspendFilters || curCat->viewfilters.nFromCats == 1) && (GetCategory() == (UINT)inCategory || curCat->viewfilters.nFromCats == 0)) // Lit Cat Filter
		return true;

	if (curCat->viewfilters.nFromCats == 2 && GetCategory() != (UINT)inCategory)
		return false;

	if (!curCat->viewfilters.bVideo && IsMovie())
		return false;
	//if (!curCat->viewfilters.bAudio && ED2KFT_AUDIO == GetED2KFileTypeID(GetFileName()))
	if (!curCat->viewfilters.bAudio && ED2KFT_AUDIO == GetED2KFileTypeID(GetFileName(true))) // NEO: PP - [PasswordProtection]
		return false;
	if (!curCat->viewfilters.bArchives && IsArchive())
		return false;
	//if (!curCat->viewfilters.bImages && ED2KFT_CDIMAGE == GetED2KFileTypeID(GetFileName()))
	if (!curCat->viewfilters.bImages && ED2KFT_CDIMAGE == GetED2KFileTypeID(GetFileName(true))) // NEO: PP - [PasswordProtection]
		return false;
	if (!curCat->viewfilters.bWaiting && GetStatus()!=PS_PAUSED && !IsStopped() && ((GetStatus()==PS_READY|| GetStatus()==PS_EMPTY) && GetTransferringSrcCount()==0))
		return false;
	if (!curCat->viewfilters.bTransferring && ((GetStatus()==PS_READY|| GetStatus()==PS_EMPTY) && GetTransferringSrcCount()>0))
		return false;
	if (!curCat->viewfilters.bComplete && GetStatus() == PS_COMPLETE)
		return false;
	if (!curCat->viewfilters.bCompleting && GetStatus() == PS_COMPLETING)
		return false;
	if (!curCat->viewfilters.bHashing && GetPartsHashing()) // NEO: SSH - [SlugFillerSafeHash]
	//if (!curCat->viewfilters.bHashing && (GetStatus() == PS_WAITINGFORHASH || GetStatus() == PS_HASHING))
		return false;
	if (!curCat->viewfilters.bPaused && GetStatus()==PS_PAUSED && !IsStopped())
		return false;
	if (!curCat->viewfilters.bStopped && IsStopped() && IsPartFile())
		return false;
	if (!curCat->viewfilters.bStandby && IsStandBy() && IsPartFile()) // NEO: SD - [StandByDL]
		return false;
	if (!curCat->viewfilters.bSuspend && IsCollectingHalted() && IsPartFile()) // NEO: SC - [SuspendCollecting]
		return false;
	if (!curCat->viewfilters.bErrorUnknown && (GetStatus() == PS_ERROR || GetStatus() == PS_UNKNOWN))
		return false;
	if (GetFileSize() < curCat->viewfilters.nFSizeMin || (curCat->viewfilters.nFSizeMax != 0 && GetFileSize() > curCat->viewfilters.nFSizeMax))
		return false;
	uint64 nTemp = GetFileSize() - GetCompletedSize();
	if (nTemp < curCat->viewfilters.nRSizeMin || (curCat->viewfilters.nRSizeMax != 0 && nTemp > curCat->viewfilters.nRSizeMax))
		return false;
	if (curCat->viewfilters.nTimeRemainingMin > 0 || curCat->viewfilters.nTimeRemainingMax > 0)
	{
		sint32 nTemp2 = getTimeRemaining();
		if (nTemp2 < (sint32)curCat->viewfilters.nTimeRemainingMin || (curCat->viewfilters.nTimeRemainingMax != 0 && nTemp2 > (sint32)curCat->viewfilters.nTimeRemainingMax))
			return false;
	}
	nTemp = GetSourceCount();
	if (nTemp < curCat->viewfilters.nSourceCountMin || (curCat->viewfilters.nSourceCountMax != 0 && nTemp > curCat->viewfilters.nSourceCountMax))
		return false;
	nTemp = GetAvailableSrcCount();
	if (nTemp < curCat->viewfilters.nAvailSourceCountMin || (curCat->viewfilters.nAvailSourceCountMax != 0 && nTemp > curCat->viewfilters.nAvailSourceCountMax))
		return false;
	//if (!curCat->viewfilters.sAdvancedFilterMask.IsEmpty() && !theApp.downloadqueue->ApplyFilterMask(GetFileName(), inCategory))
	if (!curCat->viewfilters.sAdvancedFilterMask.IsEmpty() && !theApp.downloadqueue->ApplyFilterMask(GetFileName(true), inCategory)) // NEO: PP - [PasswordProtection]
		return false;
	if (!curCat->viewfilters.bSeenComplet && lastseencomplete!=NULL)
		return false;
	return true;
}
// NEO: NXC END <-- Xanatos --

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
//bool CPartFile::CheckShowItemInGivenCat(int inCategory) /*const*/
/*{
	int myfilter=thePrefs.GetCatFilter(inCategory);

	// common cases
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
}*/



void CPartFile::SetFileName(LPCTSTR pszFileName, bool bReplaceInvalidFileSystemChars, bool bRemoveControlChars)
{
	CKnownFile::SetFileName(pszFileName, bReplaceInvalidFileSystemChars, bRemoveControlChars);

	UpdateDisplayedInfo(true);
	theApp.emuledlg->transferwnd->downloadlistctrl.UpdateCurrentCategoryView(this);
}

void CPartFile::SetActive(bool bActive)
{
	time_t tNow = time(NULL);
	if (bActive)
	{
		//if (theApp.IsConnected())
		if(theApp.GetConState()) // NEO: NCC - [NeoConnectionChecker] <-- Xanatos --
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


// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
bool CPartFile::NextRightFileHasHigherPrio(const CPartFile* left, const CPartFile* right)
{
	if(NeoPrefs.IsStartNextFileByPriority())
	{
		if(!right) {
			return false;
		}
		if (!left) {
			return true;
		}

		if(right->GetCatResumeOrder() < left->GetCatResumeOrder())
			return true;
		else if(right->GetCatResumeOrder() > left->GetCatResumeOrder())
			return false;
	}

	return RightFileHasHigherPrio(left, right);
}

bool CPartFile::RightFileHasHigherPrio(const CPartFile* left, const CPartFile* right)
{
    if(!right) {
        return false;
    }
	if (!left) {
		return true;
	}
	if (NeoPrefs.UseSmartA4AFSwapping())
	{
		if (right == theApp.downloadqueue->forcea4af_file)
			return true;
		else if (right->PartPrefs->ForceA4AF() == 1)
			return true;
	}
	int right_iA4AFMode = NeoPrefs.AdvancedA4AFMode();
	if (right_iA4AFMode && thePrefs.GetCategory(right->GetCategory())->iAdvA4AFMode)
		right_iA4AFMode = thePrefs.GetCategory(right->GetCategory())->iAdvA4AFMode;
	int left_iA4AFMode = NeoPrefs.AdvancedA4AFMode();
	if (left_iA4AFMode && thePrefs.GetCategory(left->GetCategory())->iAdvA4AFMode)
		left_iA4AFMode = thePrefs.GetCategory(left->GetCategory())->iAdvA4AFMode;
			
	if(
		(
			NeoPrefs.UseSmartA4AFSwapping() && left != theApp.downloadqueue->forcea4af_file
			||
			!NeoPrefs.UseSmartA4AFSwapping()
		)
		&&
		(
			!(NeoPrefs.UseSmartA4AFSwapping() && (right->PartPrefs->ForceA4AF() == 2 || left->PartPrefs->ForceA4AF() == 1)) &&
			(
				left_iA4AFMode == 2 &&
				right->GetCatResumeOrder() < left->GetCatResumeOrder() ||
				(
					right->GetCatResumeOrder() == left->GetCatResumeOrder() &&
					left_iA4AFMode == 2 &&
					left_iA4AFMode == right_iA4AFMode
					||
					left_iA4AFMode != 2 &&
					left_iA4AFMode == right_iA4AFMode
				) &&
				(
					thePrefs.GetCategory(right->GetCategory())->prio > thePrefs.GetCategory(left->GetCategory())->prio ||
					thePrefs.GetCategory(right->GetCategory())->prio == thePrefs.GetCategory(left->GetCategory())->prio &&
					(
						right->GetDownPriority() > left->GetDownPriority() ||
						right->GetDownPriority() == left->GetDownPriority() &&
						(
							right->GetCategory() == left->GetCategory() && right->GetCategory() != 0 &&
							(thePrefs.GetCategory(right->GetCategory())->downloadInAlphabeticalOrder && thePrefs.IsExtControlsEnabled()) && 
							//right->GetFileName() && left->GetFileName() &&
							right->GetFileName(true) && left->GetFileName(true) && // NEO: PP - [PasswordProtection]
							//right->GetFileName().CompareNoCase(left->GetFileName()) < 0
							right->GetFileName(true).CompareNoCase(left->GetFileName(true)) < 0 // NEO: PP - [PasswordProtection]
							||
							left_iA4AFMode != 0 &&
							right->GetAvailableSrcCount() < left->GetAvailableSrcCount()
						)
					)
				)
			)
		)
    ) {
        return true;
    } else {
        return false;
    }
}
// NEO: NXC END <-- Xanatos --

/*bool CPartFile::RightFileHasHigherPrio(CPartFile* left, CPartFile* right)
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
}*/

//void CPartFile::RequestAICHRecovery(UINT nPart)
void CPartFile::RequestAICHRecovery(UINT nPart, bool* pFaild) // NEO: SCV - [SubChunkVerification] <-- Xanatos --
{
	if (!m_pAICHHashSet->HasValidMasterHash() || (m_pAICHHashSet->GetStatus() != AICH_TRUSTED && m_pAICHHashSet->GetStatus() != AICH_VERIFIED)){
		AddDebugLogLine(DLP_DEFAULT, false, _T("Unable to request AICH Recoverydata because we have no trusted Masterhash"));
		return;
	}
	if (GetFileSize() <= (uint64)EMBLOCKSIZE || GetFileSize() - PARTSIZE*(uint64)nPart <= (uint64)EMBLOCKSIZE)
		return;
	if (CAICHHashSet::IsClientRequestPending(this, (uint16)nPart)){
		AddDebugLogLine(DLP_DEFAULT, false, _T("RequestAICHRecovery: Already a request for this part pending"));
		return;
	}

	// first check if we have already the recoverydata, no need to rerequest it then
	if (m_pAICHHashSet->IsPartDataAvailable((uint64)nPart*PARTSIZE)){
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
			&& (*pCurClient->GetReqFileAICHHash()) == m_pAICHHashSet->GetMasterHash())
		{
			if (pCurClient->HasLowID())
				cAICHLowIDClients++;
			else
				cAICHClients++;
		}
	}
	if ((cAICHClients | cAICHLowIDClients) == 0){
		AddDebugLogLine(DLP_DEFAULT, false, _T("Unable to request AICH Recoverydata because found no client who supports it and has the same hash as the trusted one"));
		if(pFaild) *pFaild = true; // NEO: SCV - [SubChunkVerification] <-- Xanatos --
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
			&& (*pCurClient->GetReqFileAICHHash()) == m_pAICHHashSet->GetMasterHash())
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
		//ASSERT( false );
		if(pFaild) *pFaild = true; // NEO: SCV - [SubChunkVerification] <-- Xanatos --
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
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	if(IsVoodooFile()){
		ASSERT(0);
		return;
	}
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

	// NEO: SCV - [SubChunkVerification] -- Xanatos -->
	if(!IsCorruptedPart(nPart) && NeoPrefs.UseSubChunkTransfer()){
		//VerifyIncompletePart((uint16)nPart);
		SetBlockPartHash((uint16)nPart);
		return;
	}
	// NEO: SCV END <-- Xanatos --

	FlushBuffer(true, true, true);

	if(!NeoPrefs.UseSubChunkTransfer()) // NEO: SCV - [SubChunkVerification] <-- Xanatos -- // David: With SCV the below condition wil never be true, a chunk can not be complete and corrupted at the same time
	{
		uint32 length = PARTSIZE;
		if ((ULONGLONG)PARTSIZE*(uint64)(nPart+1) > m_hpartfile.GetLength()){
			length = (UINT)(m_hpartfile.GetLength() - ((ULONGLONG)PARTSIZE*(uint64)nPart));
			ASSERT( length <= PARTSIZE );
		}	
		// if the part was already ok, it would now be complete
		if (IsComplete((uint64)nPart*PARTSIZE, (((uint64)nPart*PARTSIZE)+length)-1, true)){
			AddDebugLogLine(DLP_DEFAULT, false, _T("Processing AICH Recovery data: The part (%u) is already complete, canceling"));
			return;
		}
	}

	SetBlockPartHash((uint16)nPart, true, true); // NEO: SCV - [SubChunkVerification] <-- Xanatos --

	/*CAICHHashTree* pVerifiedHash = m_pAICHHashSet->m_pHashTree.FindHash((uint64)nPart*PARTSIZE, length);
	if (pVerifiedHash == NULL || !pVerifiedHash->m_bHashValid){
		AddDebugLogLine(DLP_DEFAULT, false, _T("Processing AICH Recovery data: Unable to get verified hash from hashset (should never happen)"));
		ASSERT( false );
		return;
	}
	CAICHHashTree htOurHash(pVerifiedHash->m_nDataSize, pVerifiedHash->m_bIsLeftBranch, pVerifiedHash->m_nBaseSize);
	try{
		m_hpartfile.Seek((LONGLONG)PARTSIZE*(uint64)nPart,0);
		CreateHash(&m_hpartfile,length, NULL, &htOurHash);
	}
	catch(...){
		ASSERT( false );
		return;
	}

	if (!htOurHash.m_bHashValid){
		AddDebugLogLine(DLP_DEFAULT, false, _T("Processing AICH Recovery data: Failed to retrieve AICH Hashset of corrupt part"));
		ASSERT( false );
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

	m_CorruptionBlackBox.EvaluateData(nPart); // NEO: CBBF - [CorruptionBlackBoxFix] <-- Xanatos --

	// ok now some sanity checks
	if (IsComplete((uint64)nPart*PARTSIZE, (((uint64)nPart*PARTSIZE)+length)-1, true)){
		// this is a bad, but it could probably happen under some rare circumstances
		// make sure that MD4 agrres to this fact too
		//if (!HashSinglePart(nPart)){
			AddDebugLogLine(DLP_DEFAULT, false, _T("Processing AICH Recovery data: The part (%u) got completed while recovering - but MD4 says it corrupt! Setting hashset to error state, deleting part"));
			// now we are fu... unhappy
			m_pAICHHashSet->SetStatus(AICH_ERROR);
			AddGap(PARTSIZE*(uint64)nPart, (((uint64)nPart*PARTSIZE)+length)-1);
			ASSERT( false );
			return;
		}
		else{
			AddDebugLogLine(DLP_DEFAULT, false, _T("Processing AICH Recovery data: The part (%u) got completed while recovering and MD4 agrees"));
			// alrighty not so bad
			POSITION posCorrupted = corrupted_list.Find((uint16)nPart);
			if (posCorrupted)
				corrupted_list.RemoveAt(posCorrupted);
			if (status == PS_EMPTY && theApp.emuledlg->IsRunning()){
				if (GetHashCount() == GetED2KPartHashCount() && !hashsetneeded){
					// Successfully recovered part, make it available for sharing
					SetStatus(PS_READY);
					//theApp.sharedfiles->SafeAddKFile(this); // NEO: SAFS - [ShowAllFilesInShare] <-- Xanatos --
				}
			}

			if (theApp.emuledlg->IsRunning()){
				// Is this file finished?
				//if (gaplist.IsEmpty())
				if ( gaplist.IsEmpty() && !m_nTotalBufferData && !m_FlushThread) //MORPH - Changed by SiRoB, Flush Thread // NEO: FFT - [FileFlushThread] <-- Xanatos --
					CompleteFile(false);
			}
		}//
		SetSinglePartHash((uint16)nPart, false, true, true); // SLUGFILLER: SafeHash  // NEO: SSH - [SlugFillerSafeHash] <-- Xanatos --
	} // end sanity check
	// Update met file
	SavePartFile();
	// make sure the user appreciates our great recovering work :P
	AddLogLine(true, GetResString(IDS_AICH_WORKED), CastItoXBytes(nRecovered), CastItoXBytes(length), nPart, GetFileName());
	//AICH successfully recovered %s of %s from part %u for %s*/
}

// BEGIN SLUGFILLER: SafeHash // NEO: SSH - [SlugFillerSafeHash] -- Xanatos -->
uint16 CPartFile::GetPartsHashing(bool bNoBlocks)
{
	uint16 ret;
	m_PartsToHashLocker.Lock();
	ret = (uint16)m_PartsToHash.GetCount();
	m_PartsToHashLocker.Unlock();

	if(!bNoBlocks){
		m_BlocksToHashLocker.Lock();
		ret = ret + (uint16)m_BlocksToHash.GetCount();
		m_BlocksToHashLocker.Unlock();
	}

	return ret;
}

void CPartFile::PartHashFinished(UINT partnumber, bool corrupt, bool AICHRecover, bool AICHok)
{
	theApp.emuledlg->transferwnd->downloadlistctrl.UpdateItem(this);
	if (partnumber >= GetPartCount())
		return;
	uint64 partRange = (partnumber < (UINT)(GetPartCount()-1))?PARTSIZE:((uint64)m_nFileSize % PARTSIZE);
	if (corrupt){
		if(AICHok){
			AddDebugLogLine(DLP_DEFAULT, false, _T("Processing AICH Recovery data: The part (%u) got completed while recovering - but MD4 says it corrupt! Setting hashset to error state, deleting part"), partnumber);
			// now we are fu... unhappy
			CSingleLock sLockA(&SCV_mut, TRUE); // NEO: SCV - [SubChunkVerification] <-- Xanatos --
			m_pAICHHashSet->SetStatus(AICH_ERROR);
		}
		if(!AICHRecover)
			LogWarning(LOG_STATUSBAR, GetResString(IDS_ERR_PARTCORRUPT), partnumber, GetFileName());

		//MORPH START - Changed by SiRoB, SafeHash Fix  if (partRange > 0) {partRange--;AddGap((uint64)PARTSIZE*partnumber, (uint64)PARTSIZE*partnumber + partRange);}
		if (partRange > 0)
			partRange--;
		else
			partRange = PARTSIZE - 1;
		AddGap((uint64)PARTSIZE*partnumber, (uint64)PARTSIZE*partnumber + partRange);

		if(AICHRecover == false){
			// add part to corrupted list, if not already there
			if (!IsCorruptedPart(partnumber))
				corrupted_list.AddTail((uint16)partnumber);

			// request AICH recovery data
			CSingleLock sLockA(&SCV_mut, TRUE); // NEO: SCV - [SubChunkVerification] <-- Xanatos --
			RequestAICHRecovery(partnumber);
			sLockA.Unlock(); // NEO: SCV - [SubChunkVerification] <-- Xanatos --

			// update stats
			m_uCorruptionLoss += (partRange + 1);
			thePrefs.Add2LostFromCorruption(partRange + 1);
		}else
			ASSERT( false );

		// Update met file - gaps data changed
		SavePartFile();
	} else {
		if(AICHRecover == false){
			if (thePrefs.GetVerbose())
				AddDebugLogLine(DLP_VERYLOW, false, _T("Finished part %u of \"%s\""), partnumber, GetFileName());

			//MORPH START - Changed by SiRoB, SafeHash Fix	if (partRange > 0) {partRange--;m_CorruptionBlackBox.VerifiedData((uint64)PARTSIZE * partnumber, (uint64)PARTSIZE*partnumber + partRange);}
			if (partRange > 0)
				partRange--;
			else
				partRange = PARTSIZE - 1;
			// tell the blackbox about the verified data
			m_CorruptionBlackBox.VerifiedData((uint64)PARTSIZE * partnumber, (uint64)PARTSIZE*partnumber + partRange);
		}else
			AddDebugLogLine(DLP_DEFAULT, false, _T("Processing AICH Recovery data: The part (%u) got completed while recovering and MD4 agrees"), partnumber);
			// alrighty not so bad

		// if this part was successfully completed (although ICH is active), remove from corrupted list
		POSITION posCorrupted = corrupted_list.Find((uint16)partnumber);
		if (posCorrupted)
			corrupted_list.RemoveAt(posCorrupted);

		// Successfully completed part, make it available for sharing
		m_PartsShareable[partnumber] = true;
		if (status == PS_EMPTY)
		{
			SetStatus(PS_READY);
			// NEO: SAFS - [ShowAllFilesInShare] -- Xanatos --
			//if (theApp.emuledlg->IsRunning())	// may be called during shutdown!
			//		theApp.sharedfiles->SafeAddKFile(this);
		}

		if (!GetPartsHashing()){
			// Update met file - file fully hashed
			SavePartFile();
		}

		if (theApp.emuledlg->IsRunning())	// may be called during shutdown!
		{
			// Is this file finished?
			//if (!GetPartsHashing() && gaplist.IsEmpty())
			if (!GetPartsHashing(true) && gaplist.IsEmpty() && GetStatus() == PS_READY && m_nTotalBufferData == 0 && m_FlushThread == NULL) // NEO: FFT - [FileFlushThread] // NEO: SCV - [SubChunkVerification]
			{
				// NEO: POFC - [PauseOnFileComplete]
				if(NeoPrefs.IsPauseOnFileComplete())
					m_bCompletionBreak = true;
				else
				// NEO: POFC END
					CompleteFile(false);	// Recheck all hashes, because loaded data is trusted based on file-date
			}
		}
	}
}

void CPartFile::ParseICHResult()
{
	if (m_ICHPartsComplete.IsEmpty())
		return;
	//GetPartsHashing()--;

	while (!m_ICHPartsComplete.IsEmpty()) {
		uint16 partnumber = m_ICHPartsComplete.RemoveHead();
		uint64 partRange = (partnumber < GetPartCount()-1)?PARTSIZE:((uint64)m_nFileSize % PARTSIZE);

		m_uPartsSavedDueICH++;
		thePrefs.Add2SessionPartsSavedByICH(1);

		uint64 uRecovered = GetTotalGapSizeInPart(partnumber);
		//MORPH START - Changed by SiRoB, Fix  if (partRange > 0) {partRange--;FillGap((uint64)PARTSIZE*partnumber, (uint64)PARTSIZE*partnumber + partRange);RemoveBlockFromList((uint32)PARTSIZE*partnumber, (uint32)PARTSIZE*partnumber + partRange);}
		if (partRange > 0)
			partRange--;
		else
			partRange = PARTSIZE - 1;
		FillGap((uint64)PARTSIZE*partnumber, (uint64)PARTSIZE*partnumber + partRange);
		RemoveBlockFromList((uint64)PARTSIZE*partnumber, (uint64)PARTSIZE*partnumber + partRange);
		
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
			// NEO: SAFS - [ShowAllFilesInShare] -- Xanatos --
			//if (theApp.emuledlg->IsRunning())	// may be called during shutdown!
			//		theApp.sharedfiles->SafeAddKFile(this);
		}
	}

	// Update met file - gaps data changed
	SavePartFile();

	if (theApp.emuledlg->IsRunning()){ // may be called during shutdown!
		// Is this file finished?
		//if (!GetPartsHashing() && gaplist.IsEmpty())
		if (!GetPartsHashing(true) && m_BufferedData_list.IsEmpty() && gaplist.IsEmpty()  // netfinity: Ensure all data has been written // NEO: SCV - [SubChunkVerification]
		 && GetStatus() == PS_READY && m_nTotalBufferData == 0 && m_FlushThread == NULL) // NEO: FFT - [FileFlushThread] 
		{
			// NEO: POFC - [PauseOnFileComplete]
			if(NeoPrefs.IsPauseOnFileComplete())
				m_bCompletionBreak = true;
			else
			// NEO: POFC END
				CompleteFile(false);	// Recheck all hashes, because loaded data is trusted based on file-date
		}
	}
}

bool CPartFile::SetSinglePartHash(uint16 part, bool ICHused, bool AICHRecover, bool AICHok)
{
	if (!theApp.emuledlg->IsRunning() // Don't start any last-minute hashing
	|| (part >= GetPartCount() && part != (uint16)-1)) // Out of bounds, no point in even trying
		return false;

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	if(IsVoodooFile())
		return true;
#endif // VOODOO // NEO: VOODOO END

	m_PartsToHashLocker.Lock();
	if(m_HashThread == NULL){ // we have no activ hash thread start one
		m_HashThread = AfxBeginThread(RUNTIME_CLASS(CPartHashThread), THREAD_PRIORITY_BELOW_NORMAL,0, CREATE_SUSPENDED);
		if(m_HashThread == NULL){
			ASSERT(0);
			return false;
		}
		((CPartHashThread*)m_HashThread)->SetPartFile(this);
		m_HashThread->ResumeThread();
	}

	if(part == (uint16)-1)
	{
		/*PartHashOrder* temp;
		uint16* key;
		POSITION pos = m_PartsToHash.GetStartPosition();
		while (pos){
			m_PartsToHash.GetNextAssoc(pos, key, temp);
			delete temp;
		}
		m_PartsToHash.RemoveAll();*/

		for (uint16 i = 0; i < GetPartCount(); i++){
			const uchar* hash = GetPartHash(i);
			if(hash)
			{
				if (IsComplete((uint64)i*PARTSIZE,(uint64)(i+1)*PARTSIZE-1, true)){
					PartHashOrder* temp;
					if(!m_PartsToHash.Lookup(i,temp))
						m_PartsToHash.SetAt(i,new PartHashOrder(i, hash));
					else
						ASSERT(0);
				}
			}else
				ASSERT(0);
		}
	}
	else
	{
		const uchar* hash = GetPartHash(part);
		if(hash)
		{
			PartHashOrder* temp;
			if(!m_PartsToHash.Lookup(part,temp))
				m_PartsToHash.SetAt(part,new PartHashOrder(part, hash, ICHused, AICHRecover,AICHok)); // order our part to be hashed
			else
				ASSERT(0);
		}else
			ASSERT(0);
	}

	m_PartsToHashLocker.Unlock();

	return true;
}

IMPLEMENT_DYNCREATE(CPartHashThread, CWinThread)
void CPartHashThread::SetPartFile(CPartFile* pOwner)
{
	m_pOwner = pOwner;
	fullname = RemoveFileExtension(pOwner->GetFullName());
}

int CPartHashThread::Run()
{
	DbgSetThreadName("CPartHashThread");

	InitThreadLocale();

	// NEO: STS - [SlugFillerThreadSafe]
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return 0;
	// NEO: STS END

	if(m_pOwner == NULL)
		return 0;

	CSingleLock sLock(&(m_pOwner->ICH_mut)); // ICH locks the file
	uchar hashresult[16];

	if (!file.Open(fullname,CFile::modeRead|CFile::osSequentialScan|CFile::shareDenyNone))
		return 0;

	m_pOwner->m_PartsToHashLocker.Lock();
	while(CMap<uint16,uint16,PartHashOrder*,PartHashOrder*>::CPair* pOrder = m_pOwner->m_PartsToHash.PGetFirstAssoc())
	{
		PartHashOrder* hashOrder = pOrder->value;
		m_pOwner->m_PartsToHash.RemoveKey(pOrder->key);

		if (!theApp.emuledlg->IsRunning())	// in case of shutdown while still hashing
			break;

		m_pOwner->m_PartsToHashLocker.Unlock();

		if (hashOrder->ICHused)
			sLock.Lock();

		file.Seek((LONGLONG)PARTSIZE*hashOrder->uPart,0);
		uint64 length = PARTSIZE;
		if ((ULONGLONG)PARTSIZE*(hashOrder->uPart+1) > file.GetLength()){
			length = (file.GetLength() - ((ULONGLONG)PARTSIZE*hashOrder->uPart));
			ASSERT( length <= PARTSIZE );
		}

		//MORPH - Changed by SiRoB, avoid crash if the file has been canceled
		try
		{
			m_pOwner->CreateHash(&file, length, hashresult, NULL);
		}
		catch(CFileException* ex)
		{
			ex->Delete();
			if (hashOrder->ICHused)
				sLock.Unlock();
			delete hashOrder;
			m_pOwner->m_PartsToHashLocker.Lock();
			continue;
		}
		
		bool corrupted = I2B(md4cmp(hashresult,hashOrder->Hash));
		if (!hashOrder->ICHused)	// ICH only sends successes
			PostMessage(theApp.emuledlg->m_hWnd,TM_PARTHASHED,(WPARAM) new PartHashResult(hashOrder->uPart,corrupted,hashOrder->AICHRecover,hashOrder->AICHok),(LPARAM)m_pOwner);
		else{ 
			if(!corrupted)
				m_pOwner->m_ICHPartsComplete.AddTail(hashOrder->uPart);	// Time critical, don't use message callback
			if (hashOrder->ICHused)
				sLock.Unlock();
		}

		delete hashOrder;
		m_pOwner->m_PartsToHashLocker.Lock();
	}

	if(m_pOwner->m_HashThread == this) 
		m_pOwner->m_HashThread = NULL;
	m_pOwner->m_PartsToHashLocker.Unlock();

	file.Close();
	return 0;
}

bool CPartFile::IsPartShareable(UINT partnumber) const
{
	if (partnumber < GetPartCount())
		return m_PartsShareable[partnumber];
	else
		return false;
}

//bool CPartFile::IsRangeShareable(uint64 start, uint64 end) const
//{
//	UINT first = (UINT)(start/PARTSIZE);
//	UINT last = (UINT)(end/PARTSIZE+1);
//	if (last > GetPartCount() || first >= last)
//		return false;
//	for (UINT i = first; i < last; i++)
//		if (!m_PartsShareable[i])
//			return false;
//	return true;
//}
// END SLUGFILLER: SafeHash // NEO: SSH END <-- Xanatos --


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

	if (swaped > 0)
		ModLog(GetResString(IDS_X_SWAPED_TO_SOURCE),swaped,GetFileName());

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

	if (swaped > 0)
		ModLog(GetResString(IDS_X_SWAPED_FROM_SOURCE),swaped,GetFileName());

	return swaped;
}
// NEO: MCM END <-- Xanatos --

// NEO: SCV - [SubChunkVerification] -- Xanatos -->
//void CPartFile::VerifyIncompletePart(uint16 nPart)
//{
//	// first check if we have already the recoverydata, no need to rerequest it then
//	if (!m_pAICHHashSet->IsPartDataAvailable((uint64)nPart*PARTSIZE)){
//		// we request a recovery as usual, when we get the data we expect to not find the current part on the corrupted list 
//		// and whan this heppens we call this function again
//		RequestAICHRecovery(nPart);
//		return;
//	}
//
//	// get the length of our part
//	uint32 length = PARTSIZE;
//	if ((ULONGLONG)PARTSIZE*(uint64)(nPart+1) > m_hpartfile.GetLength()){
//		length = (UINT)(m_hpartfile.GetLength() - ((ULONGLONG)PARTSIZE*(uint64)nPart));
//		ASSERT( length <= PARTSIZE );
//	}
//
//	// the part may become complete, unpropobly but can occure
//	if (IsComplete((uint64)nPart*PARTSIZE, (((uint64)nPart*PARTSIZE)+length)-1, true)){
//		AddDebugLogLine(DLP_DEFAULT, false, _T("Verify Incomplete Part: The part (%u) is already complete, canceling"), nPart);
//		return;
//	}
//
//	// get a block list for this part
//	tBlockMap* blockMap = NULL;
//	if(!GetBlockMap(nPart,&blockMap)){
//		blockMap = &m_BlockMaps[nPart]; // Add new ellement and get a pointer on it
//		blockMap->Reset();
//	}
//
//	for(uint8 nBlock = 0; nBlock != 53; nBlock++)
//	{
//		if (!blockMap->IsBlockDone(nBlock)
//			&& IsComplete((uint64)nPart*PARTSIZE + (uint64)nBlock*EMBLOCKSIZE, (uint64)nPart*PARTSIZE + (uint64)(nBlock + 1)*EMBLOCKSIZE - 1, true))
//		{
//			const uint64 nBlockStart = (uint64)nPart*PARTSIZE + (uint64)nBlock*EMBLOCKSIZE;
//			const uint64 nBlockSize = min(EMBLOCKSIZE, length - (uint64)nBlock*EMBLOCKSIZE);
//			// prepere the good hash
//			CAICHHashTree* pVerifiedHash = m_pAICHHashSet->m_pHashTree.FindHash(nBlockStart, nBlockSize);
//			if (pVerifiedHash == NULL || !pVerifiedHash->m_bHashValid){
//				AddDebugLogLine(DLP_DEFAULT, false, _T("Verify Incomplete Part: Unable to get verified hash from hashset (should never happen)"));
//				m_pAICHHashSet->SetStatus(AICH_UNTRUSTED);
//				ASSERT( false );
//				break;
//			}
//
//			// calculate the current hash
//			CAICHHashTree htOurHash(pVerifiedHash->m_nDataSize, pVerifiedHash->m_bIsLeftBranch, pVerifiedHash->m_nBaseSize);
//			try{
//				m_hpartfile.Seek((LONGLONG)nBlockStart,0);
//				CreateHash(&m_hpartfile,nBlockSize, NULL, &htOurHash);
//			}catch(...){
//				ASSERT( false );
//				continue;
//			}
//			if (!htOurHash.m_bHashValid){
//				AddDebugLogLine(DLP_DEFAULT, false, _T("Verify Incomplete Part: Failed to retrieve AICH Hashset of corrupt part"));
//				ASSERT( false );
//				continue;
//			}
//
//			// now verify our block hash
//			CAICHHashTree* pVerifiedBlock = pVerifiedHash->FindHash(0, nBlockSize);
//			CAICHHashTree* pOurBlock = htOurHash.FindHash(0, nBlockSize);
//			if ( pVerifiedBlock == NULL || pOurBlock == NULL || !pVerifiedBlock->m_bHashValid || !pOurBlock->m_bHashValid){
//				ASSERT( false );
//				continue;
//			}
//			if (pOurBlock->m_Hash == pVerifiedBlock->m_Hash){
//				blockMap->SetBlockDone(nBlock);
//				// tell the blackbox about the verified data
//				m_CorruptionBlackBox.VerifiedData(PARTSIZE*(uint64)nPart+EMBLOCKSIZE*(uint64)nBlock, PARTSIZE*(uint64)nPart + EMBLOCKSIZE*(uint64)nBlock + (nBlockSize-1));
//			}
//			else
//			{
//				blockMap->ClearBlockDone(nBlock);
//				AddGap(PARTSIZE*(uint64)nPart+EMBLOCKSIZE*(uint64)nBlock, PARTSIZE*(uint64)nPart + EMBLOCKSIZE*(uint64)nBlock + (nBlockSize-1));
//				// inform our "blackbox" about the corrupted block which may ban clients who sent it
//				m_CorruptionBlackBox.CorruptedData(PARTSIZE*(uint64)nPart+EMBLOCKSIZE*(uint64)nBlock, PARTSIZE*(uint64)nPart + EMBLOCKSIZE*(uint64)nBlock + (nBlockSize-1));
//			}
//
//			//m_PartsShareable[partnumber] = true;
//			/*if (status == PS_EMPTY && blockMap->IsBlockDone(nBlock))
//			{
//				SetStatus(PS_READY);
//				// NEO: SAFS - [ShowAllFilesInShare]
//				//if (theApp.emuledlg->IsRunning())	// may be called during shutdown!
//				//	theApp.sharedfiles->SafeAddKFile(this);
//			}*/
//		}
//	}
//
//	m_CorruptionBlackBox.EvaluateData(nPart); // NEO: CBBF - [CorruptionBlackBoxFix]
//}

void CPartFile::BlockHashFinished(UINT partnumber, CMap<uint8,uint8,BOOL,BOOL>* resultMap, bool AICHRecover, bool ForceMD4)
{
	if (partnumber >= GetPartCount())
		return;

	// get a block list for this part
	tBlockMap* blockMap = NULL;
	if(!AICHRecover && !GetBlockMap((uint16)partnumber,&blockMap)){
		delete resultMap; // this means this part is already completed and will be hashed regulary
		return;
	}

	uint32 length = PARTSIZE;
	if ((ULONGLONG)PARTSIZE*(uint64)(partnumber+1) > m_hpartfile.GetLength()){
		length = (UINT)(m_hpartfile.GetLength() - ((ULONGLONG)PARTSIZE*(uint64)partnumber));
		ASSERT( length <= PARTSIZE );
	}

	uint8 uBlock;
	BOOL Result;
	POSITION pos = resultMap->GetStartPosition();
	while (pos){
		resultMap->GetNextAssoc(pos, uBlock, Result);
		const uint64 nBlockSize = min(EMBLOCKSIZE, length - (uint64)uBlock*EMBLOCKSIZE);
		if(Result){
			if(AICHRecover){
				FillGap(PARTSIZE*(uint64)partnumber+EMBLOCKSIZE*(uint64)uBlock, PARTSIZE*(uint64)partnumber + EMBLOCKSIZE*(uint64)uBlock + (nBlockSize-1));
				RemoveBlockFromList(PARTSIZE*(uint64)partnumber+EMBLOCKSIZE*(uint64)uBlock, PARTSIZE*(uint64)partnumber + EMBLOCKSIZE*(uint64)uBlock + (nBlockSize-1));
			}

			if(blockMap){
				blockMap->SetBlockDone(uBlock);

				// Note: for obtimal compatybility we need at least one complete part
				/*if (status == PS_EMPTY && blockMap->IsBlockDone(uBlock))
				{
					SetStatus(PS_READY);
					// NEO: SAFS - [ShowAllFilesInShare] -- Xanatos --
					//if (theApp.emuledlg->IsRunning())	// may be called during shutdown!
					//		theApp.sharedfiles->SafeAddKFile(this);
				}*/
			}

			// tell the blackbox about the verified data
			m_CorruptionBlackBox.VerifiedData(PARTSIZE*(uint64)partnumber+EMBLOCKSIZE*(uint64)uBlock, PARTSIZE*(uint64)partnumber + EMBLOCKSIZE*(uint64)uBlock + (nBlockSize-1));
		}else{
			if(!AICHRecover)
				AddGap(PARTSIZE*(uint64)partnumber+EMBLOCKSIZE*(uint64)uBlock, PARTSIZE*(uint64)partnumber + EMBLOCKSIZE*(uint64)uBlock + (nBlockSize-1));

			if(blockMap)
				blockMap->ClearBlockDone(uBlock);

			// inform our "blackbox" about the corrupted block which may ban clients who sent it
			m_CorruptionBlackBox.CorruptedData(PARTSIZE*(uint64)partnumber+EMBLOCKSIZE*(uint64)uBlock, PARTSIZE*(uint64)partnumber + EMBLOCKSIZE*(uint64)uBlock + (nBlockSize-1));
		}
	}

	m_CorruptionBlackBox.EvaluateData((uint16)partnumber); // NEO: CBBF - [CorruptionBlackBoxFix]

	uint8 uMax = GetBlocksInPart((uint16)partnumber);
	bool AICHok = false;
	if(blockMap){
		bool bComplete = true;
		for(uint8 i=0; i<uMax; i++){
			if(blockMap->IsBlockDone(i) == false){
				bComplete = false;
				break;
			}
		}
		if(bComplete){ // if all blocks are complete remove the map
			AICHok = true;
			m_BlockMaps.RemoveKey((uint16)partnumber);
			blockMap = NULL;
		}
	}

	if(ForceMD4){
		// ok now some sanity checks
		if (IsComplete((uint64)partnumber*PARTSIZE, (((uint64)partnumber*PARTSIZE)+length)-1, true)){
			// this is a bad, but it could probably happen under some rare circumstances
			// make sure that MD4 agrres to this fact too
			SetSinglePartHash((uint16)partnumber, false, AICHRecover, AICHok); // NEO: SSH - [SlugFillerSafeHash]
		} // end sanity check
	}

	if(AICHRecover){
		// Update met file
		SavePartFile();
	}

	// just in case the md4 wil be done earlier
	if (theApp.emuledlg->IsRunning())	// may be called during shutdown!
	{
		// Is this file finished?
		//if (!GetPartsHashing() && gaplist.IsEmpty())
		if (!GetPartsHashing(true) && gaplist.IsEmpty() && GetStatus() == PS_READY && m_nTotalBufferData == 0 && m_FlushThread == NULL) // NEO: FFT - [FileFlushThread] // NEO: SCV - [SubChunkVerification]
		{
			// NEO: POFC - [PauseOnFileComplete]
			if(NeoPrefs.IsPauseOnFileComplete())
				m_bCompletionBreak = true;
			else
			// NEO: POFC END
				CompleteFile(false);	// Recheck all hashes, because loaded data is trusted based on file-date
		}
	}

	delete resultMap;
}

uint8 CPartFile::GetBlocksInPart(uint16 part)
{
	uint32 length = PARTSIZE;
	if ((ULONGLONG)PARTSIZE*(uint64)(part+1) > m_hpartfile.GetLength()){
		length = (UINT)(m_hpartfile.GetLength() - ((ULONGLONG)PARTSIZE*(uint64)part));
		ASSERT( length <= PARTSIZE );
	}
	return (uint8)((length/EMBLOCKSIZE)+((length%EMBLOCKSIZE)?1:0));
}

bool CPartFile::SetBlockPartHash(uint16 part, bool AICHRecover, bool ForceMD4)
{
	if (!theApp.emuledlg->IsRunning() // Don't start any last-minute hashing
	|| part >= GetPartCount()) // Out of bounds, no point in even trying
		return false;

	CSingleLock sLockA(&SCV_mut, TRUE); 
	if(!m_pAICHHashSet->IsPartDataAvailable((uint64)part*PARTSIZE)){
		RequestAICHRecovery(part);
		return false;
	}
	sLockA.Unlock();

	m_BlocksToHashLocker.Lock();
	if(m_AICHHashThread == NULL){ // we have no activ hash thread start one
		m_AICHHashThread = AfxBeginThread(RUNTIME_CLASS(CBlockHashThread), THREAD_PRIORITY_BELOW_NORMAL,0, CREATE_SUSPENDED);
		if(m_AICHHashThread == NULL){
			ASSERT(0);
			return false;
		}
		((CBlockHashThread*)m_AICHHashThread)->SetPartFile(this);
		m_AICHHashThread->ResumeThread();
	}

	BlockHashOrder* partOrder;
	if(!m_BlocksToHash.Lookup(part,partOrder)){
		partOrder = new BlockHashOrder(part, AICHRecover, ForceMD4);
		m_BlocksToHash.SetAt(part,partOrder); // add a new order for this part
	}else if(AICHRecover){
		ASSERT(0); // should not happen
		if(!partOrder->AICHRecover)
			partOrder->AICHRecover = true;
		else
			return false;
	}

	partOrder->BlocksToHash.RemoveAll();
	if(AICHRecover){
		// list all blocks
		uint8 uMax = GetBlocksInPart(part);
		for(uint8 nBlock = 0; nBlock != uMax; nBlock++)
			partOrder->BlocksToHash.AddTail(nBlock);
	}
	else
	{
		// get a block list for this part
		tBlockMap* blockMap = NULL;
		if(!GetBlockMap(part,&blockMap)){
			blockMap = &m_BlockMaps[part]; // Add new ellement and get a pointer on it
			blockMap->Reset();
		}

		//uint32 length = PARTSIZE;
		//if ((ULONGLONG)PARTSIZE*(uint64)(part+1) > m_hpartfile.GetLength()){
		//	length = (UINT)(m_hpartfile.GetLength() - ((ULONGLONG)PARTSIZE*(uint64)part));
		//	ASSERT( length <= PARTSIZE );
		//}

		// list blocks to check
		for(uint8 nBlock = 0; nBlock != 53; nBlock++){
			//const uint64 nBlockStart = (uint64)part*PARTSIZE + (uint64)nBlock*EMBLOCKSIZE;
			//const uint64 nBlockSize = min(EMBLOCKSIZE, length - (uint64)nBlock*EMBLOCKSIZE);

			if (!blockMap->IsBlockDone(nBlock)
			 && IsComplete((uint64)part*PARTSIZE + (uint64)nBlock*EMBLOCKSIZE, (uint64)part*PARTSIZE + (uint64)(nBlock + 1)*EMBLOCKSIZE - 1, true))
			// && IsComplete(nBlockStart, nBlockStart + nBlockSize - 1, true))
				partOrder->BlocksToHash.AddTail(nBlock);
		}
	}

	m_BlocksToHashLocker.Unlock();

	return true;
}

IMPLEMENT_DYNCREATE(CBlockHashThread, CWinThread)
void CBlockHashThread::SetPartFile(CPartFile* pOwner)
{
	m_pOwner = pOwner;
	fullname = RemoveFileExtension(pOwner->GetFullName());
}
int CBlockHashThread::Run()
{
	DbgSetThreadName("CBlockHashThread");

	InitThreadLocale();

	// NEO: STS - [SlugFillerThreadSafe]
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return 0;
	// NEO: STS END

	if(m_pOwner == NULL)
		return 0;

	//CSingleLock sLock(&(m_pOwner->ICH_mut)); // ICH locks the file

	if (!file.Open(fullname,CFile::modeRead|CFile::osSequentialScan|CFile::shareDenyNone))
		return 0;

	m_pOwner->m_BlocksToHashLocker.Lock();
	while(CMap<uint16,uint16,BlockHashOrder*,BlockHashOrder*>::CPair* pOrder = m_pOwner->m_BlocksToHash.PGetFirstAssoc())
	{
		BlockHashOrder* hashOrder = pOrder->value;
		m_pOwner->m_BlocksToHash.RemoveKey(pOrder->key);

		if (!theApp.emuledlg->IsRunning())	// in case of shutdown while still hashing
			break;

		m_pOwner->m_BlocksToHashLocker.Unlock();

		// get the length of our part
		uint32 length = PARTSIZE;
		if ((ULONGLONG)PARTSIZE*(uint64)(hashOrder->uPart+1) > m_pOwner->m_hpartfile.GetLength()){
			length = (UINT)(m_pOwner->m_hpartfile.GetLength() - ((ULONGLONG)PARTSIZE*(uint64)hashOrder->uPart));
			ASSERT( length <= PARTSIZE );
		}

		//if (hashOrder->AICHRecover)
		//	sLock.Lock();

		CMap<uint8,uint8,BOOL,BOOL>* resultMap = new CMap<uint8,uint8,BOOL,BOOL>;

		uint32 nRecovered = 0;
		while(!hashOrder->BlocksToHash.IsEmpty())
		{
			uint8 nBlock = hashOrder->BlocksToHash.RemoveHead();

			BOOL oldResult;
			if(resultMap->Lookup(nBlock,oldResult)){
				ASSERT(0); //This block we already have hashed recently
				if(!hashOrder->AICHRecover)
					continue;
			}
			
			const uint64 nBlockStart = (uint64)hashOrder->uPart*PARTSIZE + (uint64)nBlock*EMBLOCKSIZE;
			const uint64 nBlockSize = min(EMBLOCKSIZE, length - (uint64)nBlock*EMBLOCKSIZE);

			CSingleLock sLockA(&(m_pOwner->SCV_mut),TRUE); // SCV locks the AICH Hashset

			// prepere the good hash
			CAICHHashTree* pVerifiedHash = m_pOwner->m_pAICHHashSet->m_pHashTree.FindHash(nBlockStart, nBlockSize);
			if (pVerifiedHash == NULL || !pVerifiedHash->m_bHashValid){
				AddDebugLogLine(DLP_DEFAULT, false, _T("Verify Incomplete Part: Unable to get verified hash from hashset (should never happen)"));
				m_pOwner->m_pAICHHashSet->SetStatus(AICH_UNTRUSTED);
				sLockA.Unlock();
				ASSERT( false );
				break;
			}

			CAICHHashTree* pVerifiedBlock = pVerifiedHash->FindHash(0, nBlockSize);
			if ( pVerifiedBlock == NULL || !pVerifiedBlock->m_bHashValid){
				sLockA.Unlock();
				ASSERT( false );
				continue;
			}
			CAICHHash Hash = pVerifiedBlock->m_Hash;
			uint64 nDataSize = pVerifiedHash->m_nDataSize;
			bool bIsLeftBranch = pVerifiedHash->m_bIsLeftBranch;
			uint64 nBaseSize = pVerifiedHash->m_nBaseSize;

			sLockA.Unlock();

			// calculate the current hash
			CAICHHashTree htOurHash(nDataSize, bIsLeftBranch, nBaseSize);
			try
			{
				file.Seek((LONGLONG)nBlockStart,0);
				m_pOwner->CreateHash(&file,nBlockSize, NULL, &htOurHash);
			}
			catch(CFileException* ex)
			{
				ex->Delete();
				if (!theApp.emuledlg->IsRunning())	// in case of shutdown while still hashing
					break;
				continue;
			}
			catch(...)
			{
				ASSERT( false );
				continue;
			}

			if (!htOurHash.m_bHashValid){
				AddDebugLogLine(DLP_DEFAULT, false, _T("Verify Incomplete Part: Failed to retrieve AICH Hashset of corrupt part"));
				ASSERT( false );
				continue;
			}

			// now verify our block hash
			CAICHHashTree* pOurBlock = htOurHash.FindHash(0, nBlockSize);
			if ( pOurBlock == NULL || !pOurBlock->m_bHashValid){
				ASSERT( false );
				continue;
			}

			BOOL Result = pOurBlock->m_Hash == Hash;
			resultMap->SetAt(nBlock,Result);

			if(Result && hashOrder->AICHRecover)
				nRecovered += (uint32)nBlockSize;
		}

		if(hashOrder->AICHRecover){
			//sLock.Unlock();
			if (m_pOwner->m_uCorruptionLoss >= nRecovered)
				m_pOwner->m_uCorruptionLoss -= nRecovered;
			if (thePrefs.sesLostFromCorruption >= nRecovered)
				thePrefs.sesLostFromCorruption -= nRecovered;

			theApp.QueueLogLine(true, GetResString(IDS_AICH_WORKED), CastItoXBytes(nRecovered), CastItoXBytes(length), hashOrder->uPart, m_pOwner->GetFileName());
		}

		PostMessage(theApp.emuledlg->m_hWnd,TM_BLOCKHASHED,(WPARAM) new BlockHashResult(hashOrder->uPart,resultMap,hashOrder->AICHRecover,hashOrder->ForceMD4),(LPARAM)m_pOwner);

		delete hashOrder;
		m_pOwner->m_BlocksToHashLocker.Lock();
	}

	if(m_pOwner->m_AICHHashThread == this) 
		m_pOwner->m_AICHHashThread = NULL;
	m_pOwner->m_BlocksToHashLocker.Unlock();

	file.Close();
	return 0;
}

bool CPartFile::VerifyIncompleteParts(bool bCheck)
{
	bool bFound = false;
	uint16 nPart = 0;
	while (nPart != GetPartCount()){
		if( !IsPureGap((uint64)nPart*PARTSIZE, (uint64)(nPart + 1)*PARTSIZE - 1) 
		 && !IsComplete((uint64)nPart*PARTSIZE, (uint64)(nPart + 1)*PARTSIZE - 1, true) ){
			// get a block list for this part
			tBlockMap* blockMap = NULL;
			if(GetBlockMap(nPart,&blockMap)){
				if(!blockMap->IsEmpty())
					bFound = true;
			}

			for(uint8 nBlock = 0; nBlock != 53; nBlock++){
				if ((!blockMap || !blockMap->IsBlockDone(nBlock))
				&& IsComplete((uint64)nPart*PARTSIZE + (uint64)nBlock*EMBLOCKSIZE, (uint64)nPart*PARTSIZE + (uint64)(nBlock + 1)*EMBLOCKSIZE - 1, true)){
					if(bCheck)
						m_uAICHVeirifcationPending++;
					else{
						bool pFaild = false;
						CSingleLock sLockA(&SCV_mut, TRUE);
						RequestAICHRecovery(nPart,&pFaild);
						sLockA.Unlock();
						if(!pFaild && m_uAICHVeirifcationPending != 0)
							m_uAICHVeirifcationPending--;
						else if(m_uAICHVeirifcationPending == 0) // it may happen that more blocks got finished in the mean time and the m_uAICHVeirifcationPending is not relaialable
							m_uAICHVeirifcationPending++;
					}
					break;
				}
			}
		}
		nPart++;
	}
	return bFound;
}

void CPartFile::SaveAICHMap(CFileDataIO* file)
{
	uint8 uMapCount = 0;
	ULONG uMapCountPos = (ULONG)file->GetPosition();
	file->WriteUInt16(uMapCount);

	uint16 uPart = 0;
	while (uPart != GetPartCount()){
		tBlockMap* blockMap = NULL;
		if(GetBlockMap(uPart,&blockMap)){
			file->WriteUInt16(uPart);
			file->Write(&blockMap->map,7);
			uMapCount++;
		}
		uPart++;
	}

	file->Seek(uMapCountPos, CFile::begin);
	file->WriteUInt8(uMapCount);
	file->Seek(0, CFile::end);
}

bool CPartFile::LoadAICHMap(CFileDataIO* file)
{
	uint16 count = file->ReadUInt16();

	uint16 part;
	tBlockMap blockmap;
	for (uint16 i = 0; i < count; i++){
		part = file->ReadUInt16();
		file->Read(&blockmap.map,7);
		m_BlockMaps.SetAt(part,blockmap); // copy the map into our global map
	}

	return true;
}

bool CPartFile::GetBlockMap(uint16 part, tBlockMap** map)
{
	*map = NULL; // it is important to reset the map pointer, otherwice GetNextEmptyBlockInPart will fail
	CBlockMaps::CPair *pCurVal;
	pCurVal = m_BlockMaps.PLookup(part);
	if(pCurVal)
		*map = &pCurVal->value; // return reference to object storred in our global map
	return *map != NULL;
}
// NEO: SCV END <-- Xanatos --

// NEO: MOD - [SpaceAllocate] -- Xanatos -->
void CPartFile::AllocateNeededSpace()
{
	if (m_AllocateThread!=NULL)
		return;

	// Allocate filesize
	m_AllocateThread = AfxBeginThread(AllocateSpaceThread, this, THREAD_PRIORITY_LOWEST, 0, CREATE_SUSPENDED);
	if (m_AllocateThread == NULL){
		TRACE(_T("Failed to create alloc thread! -> allocate blocking\n"));
	}else{
		m_iAllocinfo = GetFileSize();
		m_AllocateThread->ResumeThread();
	}
}
// NEO: MOD END <-- Xanatos --

// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
void CPartFile::MovePartFile(CString newTempDir)
{
	//PauseFile();

	m_strNewDirectory = newTempDir;
	CWinThread *pThread = AfxBeginThread(MoveThreadProc, this, THREAD_PRIORITY_BELOW_NORMAL, 0, CREATE_SUSPENDED); // Lord KiRon - using threads for file completion
	if (pThread){
		SetFileOp(PFOP_COPYING);
		SetFileOpProgress(0);
		pThread->ResumeThread();
	}else
		LogError(LOG_STATUSBAR, GetResString(IDS_X_ERR_FILEMOVETHREAD));

}

UINT CPartFile::MoveThreadProc(LPVOID pvParams) 
{ 
	DbgSetThreadName("PartFileComplete");
	InitThreadLocale();

	// SLUGFILLER: SafeHash // NEO: STS - [SlugFillerThreadSafe]
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return 0;
	// SLUGFILLER: SafeHash // NEO: STS END

	CPartFile* pFile = (CPartFile*)pvParams;
	if (!pFile)
		return (UINT)-1; 

	CSingleLock sLock(&(theApp.hashing_mut), TRUE); // only one file operation at a time
   	pFile->PerformFileMove(); 

   	return 0; 
}

// Lord KiRon - using threads for file completion
// NOTE: This function is executed within a seperate thread, do *NOT* use any lists/queues of the main thread without
// synchronization. Even the access to couple of members of the CPartFile (e.g. filename) would need to be properly
// synchronization to achive full multi threading compliance.
BOOL CPartFile::PerformFileMove() 
{
	CString strOldname(RemoveFileExtension(m_fullname));
	CString strNewname;
	strNewname.Format(_T("%s\\%s"), m_strNewDirectory, RemoveFileExtension(m_partmetfilename));

	if (!PathFileExists(m_strNewDirectory)){
		theApp.QueueLogLine(true,GetResString(IDS_X_ERR_BADTEMPDIR),m_strNewDirectory, GetFileName());
		return FALSE;
	}

	if(GetFileSize() > GetFreeDiskSpaceX(m_strNewDirectory)){
		theApp.QueueLogLine(true,GetResString(IDS_X_ERR_NOTEMPDIRSPACE),m_strNewDirectory, GetFileName());
		return FALSE;
	}

	SetStatus(PS_MOVING);

	// If that function is invoked from within the file completion thread, it's ok if we wait (and block) the thread.
	CSingleLock sLock(&m_FileCompleteMutex, TRUE);
	
	bool bFastMove = false;

	// close permanent handle
	try{
		if (m_hpartfile.m_hFile != INVALID_HANDLE_VALUE){
			bFastMove = (m_hpartfile.GetLength() < KB2B(250));
			m_hpartfile.Close();
		}
	}
	catch(CFileException* error){
		TCHAR buffer[MAX_CFEXP_ERRORMSG];
		error->GetErrorMessage(buffer, ARRSIZE(buffer));
		theApp.QueueLogLine(true, GetResString(IDS_ERR_FILEERROR), m_partmetfilename, GetFileName(), buffer);
		error->Delete();
		return false;
	}

	if(PathFileExists(strNewname))
	{
		ASSERT(0); // This Should not happen...

		int i = 0; 
		bool found = false;
		CString tmpfilename;
		do{
			i++; 
			found = true;
			for (int j = 0; j < thePrefs.tempdir.GetCount(); j++) {
				tmpfilename.Format(_T("%s\\%03i.part"), thePrefs.GetTempDir(j), i); 
				if (PathFileExists(tmpfilename))
					found = false;
			}
		}
		while (!found); 

		tmpfilename.Format(_T("%03i.part"), i); 
		strNewname.Format(_T("%s\\%s"), m_strNewDirectory, tmpfilename);
	}

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
  if(!IsVoodooFile()){
#endif // VOODOO // NEO: VOODOO END

	if(!bFastMove){
		DWORD dwMoveResult;
		if ((dwMoveResult = MoveCompletedPartFile(strOldname, strNewname, this)) != ERROR_SUCCESS)
		{
			theApp.QueueLogLine(true,GetResString(IDS_X_ERR_MOVEINGFAILED) + _T(" - \"%s\": ") + GetErrorMessage(dwMoveResult), GetFileName(), strNewname);
			// If the destination file path is too long, the default system error message may not be helpful for user to know what failed.
			if (strNewname.GetLength() >= MAX_PATH)
					theApp.QueueLogLine(true,GetResString(IDS_X_ERR_MOVEINGFAILED) + _T(" - \"%s\": Path too long"),GetFileName(), strNewname);

			SetStatus(PS_ERROR);
			SetFileOp(PFOP_NONE);
			if (theApp.emuledlg && theApp.emuledlg->IsRunning())
				VERIFY( PostMessage(theApp.emuledlg->m_hWnd, TM_FILEMOVED, FILE_MOVE_THREAD_FAILED, (LPARAM)this) );
			return FALSE;
		}
	}
	else if (!::MoveFile(strOldname,strNewname))
		theApp.QueueLogLine(true,GetResString(IDS_X_ERR_MOVE) + _T(" - ") + CString(strerror(errno)),strOldname,strNewname);

	//UncompressFile(strNewname, this);

	// to have the accurate date stored in known.met we have to update the 'date' of a just completed file.
	// if we don't update the file date here (after commiting the file and before adding the record to known.met), 
	// that file will be rehashed at next startup and there would also be a duplicate entry (hash+size) in known.met
	// because of different file date!
	/*ASSERT( m_hpartfile.m_hFile == INVALID_HANDLE_VALUE ); // the file must be closed/commited!
	struct _stat st;
	if (_tstat(strNewname, &st) == 0)
	{
		m_tLastModified = st.st_mtime;
		m_tUtcLastModified = m_tLastModified;
		AdjustNTFSDaylightFileTime(m_tUtcLastModified, strNewname);
	}*/

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
  }
#endif // VOODOO // NEO: VOODOO END

	CString strExt;

	// move part.met file
	strExt.Format(PARTMET_EXT);
	if (!::MoveFile(strOldname + strExt,strNewname + strExt))
		theApp.QueueLogLine(true,GetResString(IDS_X_ERR_MOVE) + _T(" - ") + CString(strerror(errno)),strOldname + strExt,strNewname + strExt);

	// Move backup files
	strExt.Format(PARTMET_EXT);
	strExt.Append(PARTMET_BAK_EXT);
	if (_taccess(strOldname + strExt, 0) == 0 && !::MoveFile(strOldname + strExt,strNewname + strExt))
		theApp.QueueLogLine(true,GetResString(IDS_X_ERR_MOVE) + _T(" - ") + GetErrorMessage(GetLastError()), strOldname + strExt,strNewname + strExt);

	strExt.Format(PARTMET_EXT);
	strExt.Append(PARTMET_TMP_EXT);
	if (_taccess(strOldname + strExt, 0) == 0 && !::MoveFile(strOldname + strExt,strNewname + strExt))
		theApp.QueueLogLine(true,GetResString(IDS_X_ERR_MOVE) + _T(" - ") + GetErrorMessage(GetLastError()), strOldname + strExt,strNewname + strExt);

	// NEO: FCFG - [FileConfiguration]
	// move part.met.neo file
	strExt.Format(PARTMET_EXT);
	strExt.Append(PARTNEO_EXT);
	if (MoveFile(strOldname + strExt,strNewname + strExt))
		theApp.QueueModLogLine(true,GetResString(IDS_X_ERR_MOVE) + _T(" - ") + GetErrorMessage(GetLastError()), strOldname + strExt,strNewname + strExt);

	// Move backup files
	strExt.Format(PARTMET_EXT);
	strExt.Append(PARTNEO_EXT);
	strExt.Append(PARTNEO_BAK_EXT);
	if (_taccess(strOldname + strExt, 0) == 0 && !::MoveFile(strOldname + strExt,strNewname + strExt))
		theApp.QueueLogLine(true,GetResString(IDS_X_ERR_MOVE) + _T(" - ") + GetErrorMessage(GetLastError()), strOldname + strExt,strNewname + strExt);

	strExt.Format(PARTMET_EXT);
	strExt.Append(PARTNEO_EXT);
	strExt.Append(PARTNEO_TMP_EXT);
	if (_taccess(strOldname + strExt, 0) == 0 && !::MoveFile(strOldname + strExt,strNewname + strExt))
		theApp.QueueLogLine(true,GetResString(IDS_X_ERR_MOVE) + _T(" - ") + GetErrorMessage(GetLastError()), strOldname + strExt,strNewname + strExt);
	// NEO: FCFG END

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
	// move part.met.src file
	strExt.Format(PARTMET_EXT);
	strExt.Append(PARTSRC_EXT);
	if (_taccess(strOldname + strExt, 0) == 0 && MoveFile(strOldname + strExt,strNewname + strExt))
		theApp.QueueModLogLine(true,GetResString(IDS_X_ERR_MOVE) + _T(" - ") + GetErrorMessage(GetLastError()), strOldname + strExt,strNewname + strExt);

	// Move backup files
	strExt.Format(PARTMET_EXT);
	strExt.Append(PARTSRC_EXT);
	strExt.Append(PARTNEO_BAK_EXT);
	if (_taccess(strOldname + strExt, 0) == 0 && !::MoveFile(strOldname + strExt,strNewname + strExt))
		theApp.QueueLogLine(true,GetResString(IDS_X_ERR_MOVE) + _T(" - ") + GetErrorMessage(GetLastError()), strOldname + strExt,strNewname + strExt);

	strExt.Format(PARTMET_EXT);
	strExt.Append(PARTSRC_EXT);
	strExt.Append(PARTNEO_TMP_EXT);
	if (_taccess(strOldname + strExt, 0) == 0 && !::MoveFile(strOldname + strExt,strNewname + strExt))
		theApp.QueueLogLine(true,GetResString(IDS_X_ERR_MOVE) + _T(" - ") + GetErrorMessage(GetLastError()), strOldname + strExt,strNewname + strExt);
#endif // NEO_SS // NEO: NSS END

	// initialize 'this' part file for being a 'complete' file, this is to be done *before* releasing the file mutex.
	m_fullname = strNewname + PARTMET_EXT;
	SetPath(m_strNewDirectory);

	// open permanent handle
	CString searchpath(RemoveFileExtension(m_fullname));

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
  if(!IsVoodooFile()){
#endif // VOODOO // NEO: VOODOO END
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
		SetStatus(PS_ERROR);
		return false;
	}
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
  }
#endif // VOODOO // NEO: VOODOO END

	SetFilePath(searchpath);
	_SetStatus(PS_READY); // set status of CPartFile object, but do not update GUI (to avoid multi-thread problems) // X? is this nessesery?
	SetFileOp(PFOP_NONE);

	// clear the blackbox to free up memory
	m_CorruptionBlackBox.Free();

	// explicitly unlock the file before posting something to the main thread.
	sLock.Unlock();

	if (theApp.emuledlg && theApp.emuledlg->IsRunning())
		VERIFY( PostMessage(theApp.emuledlg->m_hWnd, TM_FILEMOVED, FILE_MOVE_THREAD_SUCCESS, (LPARAM)this) );
	return TRUE;
}
// NEO: MTD END <-- Xanatos --

// NEO: SAFS - [ShowAllFilesInShare] -- Xanatos -->
// We put all files we have into the share window, but we won't change the bahavioure of emule towards the public therefor we use this
bool CPartFile::Publishable()
{
	if(GetFileSize() <= PARTSIZE && IsPartFile())
		return false;
	if(hashsetneeded || GetStatus(true) == PS_EMPTY)
		return false;
	return CKnownFile::Publishable();
}
// NEO: SAFS END <-- Xanatos --

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
bool CPartFile::IsVoodooFile() const 
{
	if(!IsPartFile())
		return CKnownFile::IsVoodooFile();
	return m_isVoodooFile;
}

void CPartFile::AddMaster(CVoodooSocket* Master)
{
	if(!IsPartFile()){
		CKnownFile::AddMaster(Master);
		return;
	}

	CMasterDatas* Datas = GetMasterDatas(Master);
	if(Datas) // master is already on list
		return;

	Datas = new CMasterDatas;
	Datas->gaplist = new CTypedPtrList<CPtrList, Gap_Struct*>; // Individual Master gap list

	Gap_Struct* gap = new Gap_Struct;
	gap->start = 0;
	gap->end = m_nFileSize - (uint64)1;
	Datas->gaplist->AddTail(gap);

	if(m_MasterMap.IsEmpty()){ // first master
		SetStatus(PS_EMPTY); 
		//theApp.sharedfiles->SafeAddKFile(this); // don't share the file untill we get teh gap list
	}

	m_MasterMap.SetAt(Master,Datas);
}

void CPartFile::ReCombinateGapList()
{
	while (!gaplist.IsEmpty())
		delete gaplist.RemoveHead();

	CMasterDatas* Datas;
	CVoodooSocket* Master;
	POSITION pos = m_MasterMap.GetStartPosition();

	if(m_MasterMap.GetCount() > 1){
		Gap_Struct* gap = new Gap_Struct;
		gap->start = 0;
		gap->end = m_nFileSize - (uint64)1;
		gaplist.AddTail(gap);

		// If we have more than one master we compile a gap list that only contains the gaps all masters have
		// thay should use lancast to exchange internaly the parts thay already have

		while (pos){
			m_MasterMap.GetNextAssoc(pos, Master, Datas);
			if(!Datas->HaveGapList())
				continue;

			uint64 prev = 0;
			for (pos = Datas->gaplist->GetHeadPosition();pos != NULL;){
				Gap_Struct* cur_gap = Datas->gaplist->GetNext(pos);
				prev = cur_gap->end;
				if(cur_gap->start != 0)
					FillGap(prev, cur_gap->start);
			}
			if(prev && prev != m_nFileSize - (uint64)1)
				FillGap(prev, m_nFileSize - (uint64)1);
				
		}

		// in case the masters don't use lancast and all chaunks are present internaly 
		// we create a gap list that contains all gaps form all masters
		if(gaplist.IsEmpty())
			pos = m_MasterMap.GetStartPosition();
	}

	while (pos){
		m_MasterMap.GetNextAssoc(pos, Master, Datas);
		if(!Datas->HaveGapList())
			continue;
		for (pos = Datas->gaplist->GetHeadPosition();pos != NULL;){
			Gap_Struct* cur_gap = Datas->gaplist->GetNext(pos);
			AddGap(cur_gap->start,cur_gap->end);
		}
	}

	// NEO: SSH - [SlugFillerSafeHash]
	for (UINT i = 0; i < GetPartCount(); i++)
		if (IsComplete((uint64)i*PARTSIZE,(uint64)(i+1)*PARTSIZE-1, false))
			m_PartsShareable[i] = true;
	// NEO: SSH END

	if(gaplist.IsEmpty() && GetStatus() == PS_READY)
		VoodooComplete(); // Complete the File
	else if(GetStatus() == PS_COMPLETING && !gaplist.IsEmpty())
		SetStatus(PS_READY); // Resume downloading, master droped data, propobly currupted
	else if(GetStatus() == PS_EMPTY){
		for (int i = 0; i < hashlist.GetSize(); i++){
			if (i < GetPartCount() && IsComplete(i*PARTSIZE, (i + 1)*PARTSIZE - 1, true)){
				SetStatus(PS_READY);
				break;
			}
		}

		// NEO: SAFS - [ShowAllFilesInShare]
		//if (!hashsetneeded && (GetStatus(true) == PS_READY))
		//	theApp.sharedfiles->SafeAddKFile(this);

	}
}

void CPartFile::RemoveMaster(CVoodooSocket* Master)
{
	ASSERT(IsVoodooFile());
	if(!IsPartFile()){
		CKnownFile::RemoveMaster(Master);
		return;
	}

	CMasterDatas* Datas = GetMasterDatas(Master);
	if(Datas == NULL) // master is not on list nothing to do
		return;

	m_MasterMap.RemoveKey(Master);
	delete Datas;

	if(m_MasterMap.IsEmpty() && !IsVoodooFile()){
		SetStatus(PS_PAUSED);
		SetActive(false);
		theApp.sharedfiles->RemoveFile(this); // unshare it
	}
}

void CPartFile::VoodooComplete(bool bFinal)
{
	if(!bFinal){
		CompleteFile(true);
		return;
	}

	if(!m_fullname.IsEmpty()){
		if(!IsVoodooFile()){
			CString partfilename(RemoveFileExtension(m_fullname));
			if (_tremove(partfilename))
				LogError(LOG_STATUSBAR, GetResString(IDS_ERR_DELETE) + _T(" - ") + CString(_tcserror(errno)), partfilename);
		}

		CleanUpFiles(); // NEO: MOD - [CleanUpFiles]
	}

	PerformFileCompleteEnd(FILE_COMPLETION_THREAD_SUCCESS);

	if(GetED2KPartHashCount()==0 || GetHashCount()==0) // if this file have only one part we have to share it here
		theApp.sharedfiles->SafeAddKFile(this);

	if(NeoPrefs.UseVirtualVoodooFiles() == TRUE)
		theApp.emuledlg->transferwnd->downloadlistctrl.RemoveFile(this);
}

void CPartFile::AddThrottleBlock(uint64 start, uint64 end)
{
	RemoveThrottleBlock(start, end); // remove the old entry if present

	Requested_Block_Struct* pBlock = new Requested_Block_Struct;
	pBlock->StartOffset = start;
	pBlock->EndOffset = end;
	md4cpy(pBlock->FileID, GetFileHash());
	pBlock->transferred = 0;
	pBlock->timeout = ::GetTickCount() + VOODOO_BLOCK_TIMEOUT;
	requestedblocks_list.AddTail(pBlock);
}

void CPartFile::RemoveThrottleBlock(uint64 start, uint64 end)
{
	ASSERT( start <= end );

	for (POSITION pos = requestedblocks_list.GetHeadPosition(); pos != NULL; ){
		POSITION posLast = pos;
		Requested_Block_Struct* block = requestedblocks_list.GetNext(pos);
		if (block->StartOffset <= start && block->EndOffset >= end){
			if(block->timeout == 0) // its not a voodoo block don't touch it!
				continue;
			requestedblocks_list.RemoveAt(posLast);
			delete block;
		}
	}
}

// NEO: VOODOOx - [VoodooSourceExchange]
bool CPartFile::IsVoodooXS(bool bBoot) const
{
	if(!IsVoodooFile() && !KnownPrefs->IsEnableVoodoo())
		return false;
	return (PartPrefs->IsVoodooXS() == 1 || (bBoot && PartPrefs->IsVoodooXS() == 2));
}
// NEO: VOODOOx END

#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

// NEO: SCT - [SubChunkTransfer] -- Xanatos -->
void CPartFile::WriteSubChunkMaps(CSafeMemFile* file, CClientFileStatus* status) const
{
	uint8 uDiv = 1; // one bit represents one block //1 // 2 //3 // 4 //7
	if ((uint64)GetFileSize() > 1*1024*1024*1024)
		uDiv = 7;
	else if ((uint64)GetFileSize() > 512*1024*1024)
		uDiv = 4;
	else if ((uint64)GetFileSize() > 256*1024*1024)
		uDiv = 3;
	else if ((uint64)GetFileSize() > 128*1024*1024)
		uDiv = 2;
	else
		uDiv = 1;
	uint8 uOpts =   //((			& 0x1F) << 3) | // reserved
					((uDiv			& 0x07) << 0);
	file->WriteUInt8(uOpts);

	uint8 uMapCount = 0;
	ULONG uMapCountPos = (ULONG)file->GetPosition();
	file->WriteUInt8(uMapCount);
	
	UINT BitMapLen = (53/(8*uDiv)) + ((53%(8*uDiv)) ? 1 : 0);
	uint8 BitMap[7];

	// Note: We can not send the entier list for all parts in file, 
	// even if we limit us only to the one the remote clinet is missing, it may be to much
	// so we send only a limited amount, at least 5 never more than 15 by default 10% of the file
	// We want pass with the time the whole list to the 
	UINT uLimit = min(15,max(5,GetPartCount()/10));
	UINT uPart = status->m_uSCTpos; // 0; // we are sending the status map progressiv untill al is sent
	while (uPart != GetPartCount())
	{
		tBlockMap* blockMap = NULL;
		if(!status->IsPartAvailable(uPart) // don't send maps for parts the cleint have complete
		 && const_cast<CPartFile*>(this)->GetBlockMap((uint16)uPart,&blockMap) && !blockMap->IsEmpty() // get the map and see are they ready blocks inside
		 && IsComplete((uint64)uPart*PARTSIZE, (uint64)(uPart + 1)*PARTSIZE - 1, true) // the part may already be completet just not fully checked by SCV
		 && GetPartState(uPart) == PR_PART_ON // NEO: IPS - [InteligentPartSharing] // Note: we dont use GetHideMap becouse it dont handle NNP situations
		){
			file->WriteUInt16((uint16)uPart);
			if(uDiv == 1)
				file->Write(&blockMap->map,7);
			else
			{
				memset(&BitMap[0],0,BitMapLen);
				blockMap->Write(BitMap,uDiv);
				file->Write(&BitMap[0],BitMapLen);
			}
			uMapCount ++;

			if(uMapCount >= uLimit)
				break;
		}
		uPart++;
	}
	status->m_uSCTpos = uPart;
	if(status->m_uSCTpos >= GetPartCount())
		status->m_uSCTpos = 0;

	file->Seek(uMapCountPos, CFile::begin);
	file->WriteUInt8(uMapCount);
	file->SeekToEnd();
}
// NEO: SCT END <-- Xanatos --

// NEO: ICS - [InteligentChunkSelection] -- Xanatos -->
void CPartFile::WriteIncPartStatus(CSafeMemFile* file) const
{
	UINT uED2KPartCount = GetED2KPartCount();
	file->WriteUInt16((uint16)uED2KPartCount);
	UINT uPart = 0;
	while (uPart != uED2KPartCount){
		uint8 towrite = 0;
		for (UINT i = 0;i < 8;i++){
			if (uPart < GetPartCount() && !IsPureGap((uint64)uPart*PARTSIZE, (uint64)(uPart + 1)*PARTSIZE - 1))
				towrite |= (1<<i);
			uPart++;
			if (uPart == uED2KPartCount)
				break;
		}
		file->WriteUInt8(towrite);
	}
}
// NEO: ICS END <-- Xanatos --

// NEO: SRT - [SourceRequestTweaks] -- Xanatos --
/*UINT CPartFile::GetMaxSources() const
{
	// Ignore any specified 'max sources' value if not in 'extended mode' -> don't use a parameter which was once
	// specified in GUI but can not be seen/modified any longer..
	return (!thePrefs.IsExtControlsEnabled() || m_uMaxSources == 0) ? thePrefs.GetMaxSourcePerFileDefault() : m_uMaxSources;
}

UINT CPartFile::GetMaxSourcePerFileSoft() const
{
	UINT temp = ((UINT)GetMaxSources() * 9L) / 10;
	if (temp > MAX_SOURCES_FILE_SOFT)
		return MAX_SOURCES_FILE_SOFT;
	return temp;
}

UINT CPartFile::GetMaxSourcePerFileUDP() const
{	
	UINT temp = ((UINT)GetMaxSources() * 3L) / 4;
	if (temp > MAX_SOURCES_FILE_UDP)
		return MAX_SOURCES_FILE_UDP;
	return temp;
}*/

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
		//	if (cur_src->HasFileComment())
		// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
		CClientFileStatus* status = cur_src->GetFileStatus(this);
		if (status && status->HasFileComment())
		// NEO: SCFS END <-- Xanatos --
		{
			//CString strCommentLower(cur_src->GetFileComment());
			CString strCommentLower(status->GetFileComment()); // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
			strCommentLower.MakeLower();

			int iPos = 0;
			CString strFilter(thePrefs.GetCommentFilter().Tokenize(_T("|"), iPos));
			while (!strFilter.IsEmpty())
			{
				// comment filters are already in lowercase, compare with temp. lowercased received comment
				if (strCommentLower.Find(strFilter) >= 0)
				{
					// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
					status->SetFileComment(_T(""));
					status->SetFileRating(0);
					// NEO: SCFS END <-- Xanatos --
					//cur_src->SetFileComment(_T(""));
					//cur_src->SetFileRating(0);
					break;
				}
				strFilter = thePrefs.GetCommentFilter().Tokenize(_T("|"), iPos);
			}		
		}
	}
	RefilterKadNotes();
	UpdateFileRatingCommentAvail();
}

// NEO: SDT - [SourcesDropTweaks] -- Xanatos -->
bool CPartFile::TestAndDropSource(CUpDownClient* src, int DropMode, UINT Time, int LimitMode, UINT Limit, int Mode, UINT &LastTime)
{
	switch(LimitMode)
	{
		case SDT_LIMIT_MODE_TOTAL:
			if(GetSourceCount() <= Limit)
				return false;
			break;
		case SDT_LIMIT_MODE_RELATIV:
			if(!GetSourceCount() || (GetSrcStatisticsValue(src->GetDownloadState()) * 100 / GetSourceCount()) <= Limit)
				return false;
			break;
		case SDT_LIMIT_MODE_SPECIFIC:
			if(GetSrcStatisticsValue(src->GetDownloadState()) <= Limit)
				return false;
			break;
		default: 
			ASSERT(0); 
			return false;
	}

	switch(DropMode)
	{
		case SDT_TIME_MODE_PROGRESSIV:
			if((::GetTickCount() - lastpurgetime) <= Time)
				return false;
			lastpurgetime = ::GetTickCount();
			break;
		case SDT_TIME_MODE_DISTRIBUTIV:
			if(::GetTickCount() - src->GetLastUsableDownloadState() <= Time)
				return false;
			break;
		case SDT_TIME_MODE_CUMMULATIV:
			if(LastTime && (::GetTickCount() - LastTime) <= Time)
				return false;
			LastTime = 0; // this means the time had come and dropping is in progress will be set to ::GetTickCount() after all sources have been prozessed
			break;
		default: 
			ASSERT(0); 
			return false;
	}

	if(Mode == 1 || !src->SwapToAnotherFile(_T("Drop Special"),false, true, true, NULL)){
		// Note: dropping without preventing the source from comming back is pointles, more its even bad as it created additional overhead,
		//			therefor we sue our dead source list to prevent the user from comming back to early.
		m_DeadSourceList.AddDeadSource(src, true);
		theApp.downloadqueue->RemoveSource( src );
	}
	return true;
}

bool CPartFile::IsHighQState(CUpDownClient* src)
{
	if(src->GetDownloadState() != DS_ONQUEUE)
		return false;
	if(PartPrefs->GetHighQSourceRankMode() == SDT_HIGHQ_MODE_NORMAL){
		if(src->GetRemoteQueueRank() > (unsigned)PartPrefs->GetHighQSourceMaxRank())
			return true;
	}else if(PartPrefs->GetHighQSourceRankMode() == SDT_HIGHQ_MODE_AVERAGE){
		if(GetAverageQueueRank() && (src->GetRemoteQueueRank() * 100 / GetAverageQueueRank()) <= (unsigned)PartPrefs->GetHighQSourceMaxRank())
			return true;
	}
	return false;
}
// NEO: SDT END <-- Xanatos --

// NEO: XSC - [ExtremeSourceCache] -- Xanatos -->
UINT CPartFile::IsCollectingHalted() const 
{
	if(suspend)	// NEO: SC - [SuspendCollecting]
		return 1;
	if(GetSrcStatisticsValue(DS_CACHED) > 0)
		return 2;
	if(m_bCollectingHalted) // NEO: ASL - [AutoSoftLock]
		return 2;
	if(PartPrefs->UseGlobalSourceLimit() && !theApp.downloadqueue->GetGlobalHLSrcReqAllowed()) // NEO: GSL - [GlobalSourceLimit]
		return 3;

	return 0;
} 
// NEO: XSC END <-- Xanatos --

// NEO: ASL - [AutoSoftLock] -- Xanatos -->
bool CPartFile::CheckSoftLock(){
	UINT uWaitingSourceCount = m_anStates[DS_TOOMANYCONNS]+m_anStates[DS_TOOMANYCONNSKAD];
	if(uWaitingSourceCount > (UINT)PartPrefs->GetAutoSoftLockLimit()){
		m_bSoftLocked = true;
		return true;
	}else if(m_bSoftLocked){
		if(uWaitingSourceCount*4 > (UINT)PartPrefs->GetAutoSoftLockLimit()*3)
			return true;
		m_bSoftLocked = false;
	}
	return false;
}
// NEO: ASL END <-- Xanatos --

// NEO: AHL - [AutoHardLimit] -- Xanatos -->
void CPartFile::CalculateAutoHardLimit()
{
	const int iMaxHardLimit = GetMaxSources(true);
	if(m_uAutoHardLimit == 0)
		m_uAutoHardLimit = iMaxHardLimit;
	float fAutoHardLimit = (float)m_uAutoHardLimit;
	const UINT uAvailableSrcCount = m_anStates[DS_ONQUEUE] + m_anStates[DS_DOWNLOADING];
	const uint32 tempCount = GetSourceCount();

	if(GetDownPriority() == PR_HIGH) // VS = 10...20...40...260
	{
		// AutoPR,   AutoHL = 50...125, VS = 10...20
		// ManualPR, AutoHL = 50...125, VS = 10...20
		if(uAvailableSrcCount <= 20
			&& uAvailableSrcCount >= fAutoHardLimit*0.20)
			fAutoHardLimit*=1.25F;
		else
			// AutoPR,   AutoHL = 84...144, VS = 21...30
			// ManualPR, AutoHL = 84...144, VS = 21...30
			if(uAvailableSrcCount <= 30
				&& uAvailableSrcCount >= fAutoHardLimit*0.25)
				fAutoHardLimit*=1.20F;
			else
				// AutoPR,   AutoHL = 103...154,  VS = 31...40
				// ManualPR, AutoHL = 103...1000, VS = 31...260
				if(uAvailableSrcCount >= fAutoHardLimit*0.30
					&&(fAutoHardLimit*1.15) <= iMaxHardLimit)
					fAutoHardLimit*=1.15F;
				else
					if(fAutoHardLimit*0.99 >= 50
						&& fAutoHardLimit >= tempCount)
						fAutoHardLimit*=0.99F;
	}
	else if(GetDownPriority() == PR_NORMAL) // VS = 17...41...160...519
	{
		// AutoPR,   AutoHL = 118...172, VS = 41...55
		// ManualPR, AutoHL = 50....172, VS = 17...55
		if(uAvailableSrcCount <= 55
			&& uAvailableSrcCount >= fAutoHardLimit*0.35)
			fAutoHardLimit*=1.10F;
		else
			// AutoPR,   AutoHL = 140...203, VS = 56...75
			// ManualPR, AutoHL = 140...203, VS = 56...75
			if(uAvailableSrcCount <= 75
				&& uAvailableSrcCount >= fAutoHardLimit*0.40)
				fAutoHardLimit*=1.09F;
			else
				// AutoPR,   AutoHL = 169...239, VS = 76...100
				// ManualPR, AutoHL = 169...239, VS = 76...100
				if(uAvailableSrcCount <= 100
					&& uAvailableSrcCount >= fAutoHardLimit*0.45)
					fAutoHardLimit*=1.08F;
				else
					// AutoPR,   AutoHL = 202...278, VS = 101...130
					// ManualPR, AutoHL = 202...278, VS = 101...130
					if(uAvailableSrcCount <= 130
						&& uAvailableSrcCount >= fAutoHardLimit*0.50)
						fAutoHardLimit*=1.07F;
					else
						// AutoPR,   AutoHL = 238...307,  VS = 131...160
						// ManualPR, AutoHL = 238...iMaxHardLimit, VS = 131...519
						if(uAvailableSrcCount >= fAutoHardLimit*0.55
							&& (fAutoHardLimit*1.06) <= iMaxHardLimit)
							fAutoHardLimit*=1.06F;
						else
							if(fAutoHardLimit*0.99 >= 50
								&& fAutoHardLimit >= tempCount)
								fAutoHardLimit*=0.99F;
	}
	else if(GetDownPriority() == PR_LOW) // VS = 30...161...792...792
	{
		// AutoPR,   AutoHL = 268...341, VS = 161...195
		// ManualPR, AutoHL = 50....341, VS = 30....195
		if(uAvailableSrcCount <= 195
			&& uAvailableSrcCount >= fAutoHardLimit*0.60)
			fAutoHardLimit*=1.05F;
		else
			// AutoPR,   AutoHL = 301...375, VS = 196...235
			// ManualPR, AutoHL = 301...375, VS = 196...235
			if(uAvailableSrcCount <= 235
				&& uAvailableSrcCount >= fAutoHardLimit*0.65)
				fAutoHardLimit*=1.04F;
			else
				// AutoPR,   AutoHL = 337...412, VS = 236...280
				// ManualPR, AutoHL = 337...412, VS = 236...280
				if(uAvailableSrcCount <= 280
					&& uAvailableSrcCount >= fAutoHardLimit*0.70)
					fAutoHardLimit*=1.03F;
				else
					// AutoPR,   AutoHL = 374...448, VS = 281...330
					// ManualPR, AutoHL = 374...448, VS = 281...330
					if(uAvailableSrcCount <= 330
						&& uAvailableSrcCount >= fAutoHardLimit*0.75)
						fAutoHardLimit*=1.02F;
					else
						// AutoPR,   AutoHL = 413...iMaxHardLimit, VS = 331...792
						// ManualPR, AutoHL = 413...iMaxHardLimit, VS = 331...792
						if(uAvailableSrcCount >= fAutoHardLimit*0.80
							&& (fAutoHardLimit*1.01) <= iMaxHardLimit)
							fAutoHardLimit*=1.01F;
						else
							if(fAutoHardLimit*0.99 >= 50
								&& fAutoHardLimit >= tempCount)
								fAutoHardLimit*=0.99F;
	}

	m_uAutoHardLimit = (UINT)fAutoHardLimit;

	if(m_uAutoHardLimit < (UINT)PartPrefs->GetMinSourcePerFile())
		m_uAutoHardLimit = (UINT)PartPrefs->GetMinSourcePerFile();
}
// NEO: AHL END <-- Xanatos --

// NEO: CSL - [CategorySourceLimit] -- Xanatos -->
void CPartFile::CalculateCategoryLimit()
{
	const UINT usrc = GetSourceCount();
	UINT iAHL = (UINT)max(usrc*1.15f, usrc+15);

	//Keep minimum...
	iAHL = max(iAHL, (UINT)PartPrefs->GetMinSourcePerFile());
	const UINT m_uiHLCount = theApp.downloadqueue->GetGlobalSourceCount(GetCategory()) - m_uFileCategoryLimit;
	const UINT m_uiAllowedHL = (UINT)PartPrefs->GetCategorySourceLimitLimit();
	//the max HL is *either* the min HL if we are always above the limit *or* the remaining src 
	const UINT m_uiMaxHL = 
		(m_uiHLCount > m_uiAllowedHL)
		 ? (UINT)PartPrefs->GetMinSourcePerFile() 
		 : (
		   ((m_uiAllowedHL-m_uiHLCount) > GetMaxSources(true) 
		    ? GetMaxSources(true) 
		    : (m_uiAllowedHL-m_uiHLCount))
		   );
	//Here, set HL to at least as many srcs as we have actually in queue...	
	//which is only to keep graphs looking ok - but stay below maximum
	m_uFileCategoryLimit = (max(min(m_uiMaxHL, iAHL), usrc));;

}
// NEO: CSL END <-- Xanatos --

// NEO: GSL - [GlobalSourceLimit] -- Xanatos -->
void CPartFile::InitHL()
{
	if((UINT)(NeoPrefs.PartPrefs.GetGlobalSourceLimitLimit()*.95) - theApp.downloadqueue->GetGlobalSourceCount() > 0)
		m_uFileHardLimit = max(100,GetMaxSources(true)/10);
	else
		m_uFileHardLimit = max(10,GetMaxSources(true)/100);
}

void CPartFile::IncrHL(UINT m_uSourcesDif)
{
	m_uFileHardLimit += m_uSourcesDif;

	if(m_uFileHardLimit > GetMaxSources(true))
		m_uFileHardLimit = GetMaxSources(true);
}

void CPartFile::SetPassiveHL(UINT m_uSourcesDif)
{
	m_uFileHardLimit = GetSourceCount() + m_uSourcesDif;

	if(m_uFileHardLimit < (UINT)PartPrefs->GetMinSourcePerFile())
		m_uFileHardLimit = (UINT)PartPrefs->GetMinSourcePerFile();
}
// NEO: GSL END <-- Xanatos --


// NEO: SRT - [SourceRequestTweaks] -- Xanatos -->
void CPartFile::ManIncrHL()
{
	UINT SourceLimit = (GetMaxSources() * 11L) / 10L;
	// NEO: AHL - [AutoHardLimit]
	if(m_uAutoHardLimit < SourceLimit)
		m_uAutoHardLimit = SourceLimit;
	// NEO: AHL END
	// NEO: CSL - [CategorySourceLimit]
	if(m_uFileCategoryLimit < SourceLimit)
		m_uFileCategoryLimit = SourceLimit;
	// NEO: CSL END
	// NEO: GSL - [GlobalSourceLimit]
	if(m_uFileHardLimit < SourceLimit)
		m_uFileHardLimit = SourceLimit;
	// NEO: GSL END
}

void CPartFile::ManDecrHL()
{
	UINT SourceLimit = (GetMaxSources() * 9L) / 10L;
	// NEO: AHL - [AutoHardLimit]
	if(m_uAutoHardLimit > SourceLimit)
		m_uAutoHardLimit = SourceLimit;
	// NEO: AHL END
	// NEO: CSL - [CategorySourceLimit]
	if(m_uFileCategoryLimit > SourceLimit)
		m_uFileCategoryLimit = SourceLimit;
	// NEO: CSL END
	// NEO: GSL - [GlobalSourceLimit]
	if(m_uFileHardLimit > SourceLimit)
		m_uFileHardLimit = SourceLimit;
	// NEO: GSL END
}
// NEO: SRT END <-- Xanatos --

// NEO: FCFG - [FileConfiguration] -- Xanatos -->
// NEO: SRT - [SourceRequestTweaks]
UINT CPartFile::GetMaxSources(bool bReal) const
{
	UINT SourceLimit = (UINT)PartPrefs->GetSourceLimit();
	ASSERT(SourceLimit < FCFG_BASE);
	if(!bReal)
	{
		// NEO: AHL - [AutoHardLimit]
		if(PartPrefs->UseAutoHardLimit() && m_uAutoHardLimit){
			if(m_uAutoHardLimit < SourceLimit)
				SourceLimit = m_uAutoHardLimit;
		}
		// NEO: AHL END
		// NEO: CSL - [CategorySourceLimit]
		if(PartPrefs->UseCategorySourceLimit() && m_uFileCategoryLimit){
			if(m_uFileCategoryLimit < SourceLimit)
				SourceLimit = m_uFileCategoryLimit;
		}
		// NEO: CSL END
		// NEO: GSL - [GlobalSourceLimit]
		if(PartPrefs->UseGlobalSourceLimit() && m_uFileHardLimit){
			if(m_uFileHardLimit < SourceLimit)
				SourceLimit = m_uFileHardLimit;
		}
		// NEO: GSL END
	}
	return SourceLimit;
}

UINT CPartFile::GetSwapSourceLimit() const
{
	int SwapLimit = PartPrefs->GetSwapLimit();
	ASSERT_VAL(SwapLimit);
	if(SwapLimit == FCFG_AUT)
		return (GetMaxSources() * 8L) / 10;
	return (UINT) SwapLimit;
}

#define IMPLEMENT_SOURCE_LIMIT(type, TYPE, factor) \
UINT CPartFile::Get##type##SourceLimit() const \
{ \
	int SourceLimit = PartPrefs->Get##type##Limit(); \
	ASSERT_VAL(SourceLimit); \
	if(SourceLimit == FCFG_AUT){ \
		UINT limit = (GetMaxSources() * factor) / 10; \
		if (limit > MAX_##TYPE##_LIMIT) \
			return MAX_##TYPE##_LIMIT; \
		return limit; \
	} \
	else if((UINT)SourceLimit > GetMaxSources()) \
		return GetMaxSources(); \
	return (UINT)SourceLimit; \
} 

IMPLEMENT_SOURCE_LIMIT(Xs,XS,9)
IMPLEMENT_SOURCE_LIMIT(Svr,SVR,9)
IMPLEMENT_SOURCE_LIMIT(Udp,UDP,7)
IMPLEMENT_SOURCE_LIMIT(Kad,KAD,7)
// NEO: SRT END

// NEO: XSC - [ExtremeSourceCache]
UINT CPartFile::GetSourceCacheSourceLimit() const
{
	int CacheLimit = PartPrefs->GetSourceCacheLimit();
	ASSERT_VAL(CacheLimit);
	if(CacheLimit == FCFG_AUT){
		UINT limit = (GetMaxSources() * 1) / 10; // cache only 10 %
		MinMax(&limit, (UINT)MIN_SOURCE_CACHE_LIMIT, (UINT)MAX_SOURCE_CACHE_LIMIT);
		return limit;
	}
	return (UINT) CacheLimit;
}
// NEO: XSC END

// NEO: SDT - [SourcesDropTweaks]
#define IMPLEMENT_DROP_LIMIT(type, TYPE, factor1,factor3) \
UINT CPartFile::Get##type##SourceDropLimit() const \
{ \
	int DropLimit = PartPrefs->Get##type##SourceLimit(); \
	ASSERT_VAL(DropLimit); \
	if(DropLimit == FCFG_AUT){ \
		ASSERT(PartPrefs->Get##type##SourceLimitMode() != 1); \
		if(PartPrefs->Get##type##SourceLimitMode() == 0) \
		{ \
			UINT limit = (GetMaxSources() * factor1) / 10; \
			if (limit < MIN_##TYPE##_SOURCE_LIMIT_1) \
				return MIN_##TYPE##_SOURCE_LIMIT_1; \
			return limit; \
		} \
		else /*if(PartPrefs->Get##type##SourceLimitMode() == 2) */ \
		{ \
			UINT limit = (GetMaxSources() * factor3) / 10; \
			if (limit < MIN_##TYPE##_SOURCE_LIMIT_3) \
				return MIN_##TYPE##_SOURCE_LIMIT_3; \
			return limit; \
		} \
	} \
	return (UINT) DropLimit; \
} 

IMPLEMENT_DROP_LIMIT(Bad,BAD,8,2)
IMPLEMENT_DROP_LIMIT(NNP,NNP,8,2)
IMPLEMENT_DROP_LIMIT(FullQ,FULLQ,8,2)
IMPLEMENT_DROP_LIMIT(HighQ,HIGHQ,9,1)
// NEO: SDT END

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
UINT CPartFile::GetSourceStorageSourceLimit() const
{
	int StorageLimit = PartPrefs->GetSourceStorageLimit();
	ASSERT_VAL(StorageLimit);
	if(StorageLimit == FCFG_AUT){
		UINT limit = GetMaxSources(); // stor everything
		//MinMax(&limit, (UINT)MIN_SOURCE_STORAGE_LIMIT, (UINT)MAX_SOURCE_STORAGE_LIMIT);
		return limit;
	}
	return (UINT) StorageLimit;
}

UINT CPartFile::GetSourceStorageReaskSourceLimit() const
{
	int StorageLimit = PartPrefs->GetSourceStorageReaskLimit();
	ASSERT_VAL(StorageLimit);
	if(StorageLimit == FCFG_AUT){
		UINT limit = (GetMaxSources() / 4);
		MinMax(&limit, (UINT)MIN_SOURCE_STORAGE_REASK_LIMIT, (UINT)MAX_SOURCE_STORAGE_REASK_LIMIT);
 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
		if(!NeoPrefs.EnableSourceAnalizer())
 #endif // NEO_SA
		// without source analyser we are allowed o reask not more than 10% of the normal amount of sources !!!!
			Maximal(&limit, (UINT)(MAX_SOURCE_STORAGE_REASK_LIMIT/10)); 
		// NEO: NSA END
		return limit;
	}
	return (UINT) StorageLimit;
}
#endif // NEO_SS // NEO: NSS END
// NEO: FCFG END <-- Xanatos --

// NEO: MSR - [ManualSourceRequest] -- Xanatos -->
void CPartFile::CollectKADSources()
{
	if ((Kademlia::CKademlia::GetTotalFile() > /*(UINT)thePrefs.GetKadMaxFiles()*/ KADEMLIATOTALFILE) 
	 || (::GetTickCount() < m_LastSearchTimeKad))
		return;

	if (IsStopped())
		return;

	if(Kademlia::CKademlia::IsConnected())
	{
		//Kademlia
		theApp.downloadqueue->SetLastKademliaFileRequest();
		if (!GetKadFileSearchID())
		{
			Kademlia::CUInt128 kadFileID;
			kadFileID.SetValue(GetFileHash());
			Kademlia::CSearch* pSearch = Kademlia::CSearchManager::PrepareLookup(Kademlia::CSearch::FILE, true, kadFileID);
			if (pSearch)
			{
				pSearch->SetFileName(GetFileName()); // NEO: KII - [KadInterfaceImprovement]

				if(m_TotalSearchesKad < 7 /*PartPrefs->GetKadRepeatDelay()*/)
					m_TotalSearchesKad++;
				
				m_LastSearchTimeKad = ::GetTickCount() + (KADEMLIAREASKTIME /*PartPrefs->GetKadIntervalsMs()*/ * m_TotalSearchesKad);

				SetKadFileSearchID(pSearch->GetSearchID());
				ModLog(GetResString(IDS_X_COLLECT_KAD_DONE), GetFileName());
			}
			else
				SetKadFileSearchID(0);
		}
	}
}
// NEO: MSR END <-- Xanatos --

// NEO: MSD - [ManualSourcesDrop] -- Xanatos -->
void CPartFile::DropSources(EDownloadState nState)
{
	UINT removed = 0;
	for (POSITION pos = srclist.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* cur_src = srclist.GetNext(pos);
		if (cur_src->GetDownloadState() == nState && (nState!= DS_ONQUEUE || IsHighQState(cur_src)))
		{
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
			if(cur_src->IsLanClient()) // Don't drop lan sources
				continue;
#endif //LANCAST // NEO: NLC END
			m_DeadSourceList.AddDeadSource(cur_src, true);
			theApp.downloadqueue->RemoveSource(cur_src);
			removed ++; // Count removed source
		}
	}

	if (removed > 0)
		ModLog(GetResString(IDS_X_REMOVE_SOURCE),removed,GetFileName());
}
// NEO: MSD END <-- Xanatos --

// NEO: OCF - [OnlyCompleetFiles] -- Xanatos -->
bool CPartFile::NotSeenCompleteSource() const
{
	if(lastseencomplete == NULL)
		return true;
	else if(CTime::GetCurrentTime() - lastseencomplete > (UINT)D2S(NeoPrefs.GetToOldComplete()))
		return true;
	return false;
}
// NEO: OCF END <-- Xanatos --

// NEO: SD - [StandByDL] -- Xanatos -->
UINT CPartFile::IsStandBy() const
{
	if (standby)
		return 1;
	if (NeoPrefs.OnlyCompleteFiles() && NotSeenCompleteSource() && !forced)
		return 2;
	return 0;
}
// NEO: SD END <-- Xanatos --

#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
void CPartFile::GetTooltipFileInfo(CString &info)
{
	CKnownFile::GetTooltipFileInfo(info);
	info.Append(GetResString(IDS_X_PARTINFO));

	info.AppendFormat(GetResString(IDS_X_PARTINFOS), GetPartMetFileName());

	if (IsPartFile())
	{
		CString sourcesinfo;
		float availability = 0;
		if (IsPartFile())
		{
			sourcesinfo.Format(GetResString(IDS_SOURCESINFO), GetSourceCount(), GetValidSourcesCount(), GetSrcStatisticsValue(DS_NONEEDEDPARTS), GetSrcA4AFCount());
			if(GetPartCount() == 0)
				availability = 0;
			else
				availability = (float)GetAvailablePartCount() * 100 / GetPartCount();
		}
		info.AppendFormat(GetResString(IDS_X_STATUS), getPartfileStatus());
		info.AppendFormat(GetResString(IDS_X_PARTINFOS4), GetPartCount(), GetAvailablePartCount(), availability);
		info.AppendFormat(GetResString(IDS_X_SOURCES), sourcesinfo);
	}

	CString lsc;
	if (lastseencomplete == NULL)
		lsc.LoadString(IDS_NEVER);
	else
		lsc = lastseencomplete.Format(thePrefs.GetDateTimeFormat());
	info.AppendFormat(GetResString(IDS_X_LASTSEENCOMPL), lsc);

	CString lastdwl;
	if (GetCFileDate() == NULL)
		lastdwl = GetResString(IDS_NEVER);
	else
		lastdwl.Format(_T("%s"), GetCFileDate().Format( thePrefs.GetDateTimeFormat()));
	info.AppendFormat(GetResString(IDS_X_LASTCHANGE), lastdwl);

	CString dwlstart;
	if (GetCrFileDate()!=NULL) 
  		dwlstart.Format(_T("%s"),GetCrCFileDate().Format( thePrefs.GetDateTimeFormat()));
	else dwlstart=GetResString(IDS_UNKNOWN);
	info.AppendFormat(GetResString(IDS_X_DL_ADD), dwlstart);
}
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
