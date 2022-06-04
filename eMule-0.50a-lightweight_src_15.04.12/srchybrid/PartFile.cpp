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
#include "ED2KLink.h"
#include "Preview.h"
#include "SearchFile.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "kademlia/kademlia/search.h"
#include "kademlia/kademlia/SearchManager.h"
#include "kademlia/utils/MiscUtils.h"
#include "kademlia/kademlia/prefs.h"
#include "kademlia/kademlia/Entry.h"
#include "DownloadQueue.h"
#include "IPFilter.h"
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
#include "ClientList.h"
#include "Statistics.h"
#include "shahashset.h"
#include "Log.h"
#include "CollectionViewDialog.h"
#include "Collection.h"
//Xman
#include "BandWidthControl.h"
#include "ClientCredits.h"
#include "FileVerify.h"// X: [FV] - [FileVerify]
#include "Defaults.h"// X: [IP] - [Import Parts],[POFC] - [PauseOnFileComplete]
#include "VolumeInfo.h" // X: [FSFS] - [FileSystemFeaturesSupport]
#include "XMessageBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// morph4u :: PercentBar :: Start
#define MLC_BLEND(A, B, X) ((A + B * (X-1) + ((X+1)/2)) / X)
	#define MLC_RGBBLEND(A, B, X) (                   \
	RGB(MLC_BLEND(GetRValue(A), GetRValue(B), X), \
	MLC_BLEND(GetGValue(A), GetGValue(B), X),     \
	MLC_BLEND(GetBValue(A), GetBValue(B), X))     \
    )
// morph4u :: PercentBar :: End

// Barry - use this constant for both places
#define PROGRESS_HEIGHT 3

CRectShader CPartFile::s_LoadBar(PROGRESS_HEIGHT); // Barry - was 5// X: [CI] - [Code Improvement] BarShader
//CBarShader CPartFile::s_CacheBar(PROGRESS_HEIGHT);// X: [CB] - [CacheBar]
CBarShader CPartFile::s_ChunkBar(16);
CBarShader CPartFile::s_ProgressBar(15); // morph4u :: PercentBar
//IMPLEMENT_DYNAMIC(CPartFile, CKnownFile)

CPartFile::CPartFile(size_t ucat)
// Xman -New Save/load Sources- enkeyDEV(Ottavio84)
:	m_sourcesaver(this) 
// Xman end
{
	Init(ucat);
}

CPartFile::CPartFile(CSearchFile* searchresult, size_t cat)
// Xman -New Save/load Sources- enkeyDEV(Ottavio84)
:	m_sourcesaver(this) 
// Xman end
{
	Init(cat);

	m_FileIdentifier.SetMD4Hash(searchresult->GetFileHash());
	if (searchresult->GetFileIdentifierC().HasAICHHash())
	{
		m_FileIdentifier.SetAICHHash(searchresult->GetFileIdentifierC().GetAICHHash());
		m_pAICHRecoveryHashSet->SetMasterHash(searchresult->GetFileIdentifierC().GetAICHHash(), AICH_VERIFIED);
	}

	for (size_t i = 0; i < searchresult->taglist.GetCount();i++){
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
					for (size_t t = 0; t < ARRSIZE(_aMetaTags); t++)
					{
						if (pTag->GetType() == _aMetaTags[t].nType && pTag->GetNameID() == _aMetaTags[t].nName)
						{
							// skip string tags with empty string values
							if (pTag->IsStr() && pTag->GetStr()[0] == 0)
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
				break;
			}
		}
	}
	CreatePartFile();
}

CPartFile::CPartFile(CString edonkeylink, size_t cat)
// Xman -New Save/load Sources- enkeyDEV(Ottavio84)
:	m_sourcesaver(this) 
// Xman end
{
	CED2KLink* pLink = 0;
	try {
		pLink = CED2KLink::CreateLinkFromUrl(edonkeylink);
		_ASSERT( pLink != 0 );
		//CED2KFileLink* pFileLink = pLink->GetFileLink();
		//if (pFileLink==0) 
		if(pLink->GetKind() != CED2KLink::kFile)
			throw GetResString(IDS_ERR_NOTAFILELINK);
		if(((CED2KFileLink*)pLink)->GetSize() > OLD_MAX_EMULE_FILE_SIZE && !thePrefs.CanFSHandleLargeFiles(cat)) // X: [TD] - [TempDir] moved from CED2KFileLink
			throw GetResString(IDS_ERR_FSCANTHANDLEFILE);
		InitializeFromLink((CED2KFileLink*)pLink,cat);
	} catch (CString error) {
		CString strMsg;
		strMsg.Format(GetResString(IDS_ERR_INVALIDLINK), error);
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_LINKERROR), strMsg);
		SetStatus(PS_ERROR);
	}
	delete pLink;
}

void CPartFile::InitializeFromLink(CED2KFileLink* fileLink, size_t cat, LPCTSTR filename)
{
	if (!theApp.downloadqueue->IsFileExisting(m_FileIdentifier.GetMD4Hash()))
	{
		try{
			Init(cat);
			SetFileName(fileLink->GetName(), true, true);
			SetFileSize(fileLink->GetSize());
			m_FileIdentifier.SetMD4Hash(fileLink->GetHashKey());
			if (fileLink->HasValidAICHHash())
			{
				m_FileIdentifier.SetAICHHash(fileLink->GetAICHHash());
				m_pAICHRecoveryHashSet->SetMasterHash(fileLink->GetAICHHash(), AICH_VERIFIED);
			}
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
			CreatePartFile(filename);
			return;
		}
		catch(CString error){
			CString strMsg;
			strMsg.Format(GetResString(IDS_ERR_INVALIDLINK), error);
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_LINKERROR), strMsg);
			//SetStatus(PS_ERROR);
		}
	}
	SetStatus(PS_ERROR);
}

CPartFile::CPartFile(CED2KFileLink* fileLink, size_t cat, LPCTSTR filename)
// Xman -New Save/load Sources- enkeyDEV(Ottavio84)
:	m_sourcesaver(this) 
// Xman end
{
	InitializeFromLink(fileLink,cat, filename);
}

void CPartFile::Init(size_t ucat){
	m_category=ucat;
	m_pAICHRecoveryHashSet = new CAICHRecoveryHashSet(this);
	//newdate = true;
	m_LastSearchTime = 0;
	m_LastSearchTimeKad = 0;
	m_TotalSearchesKad = 0;
	lastpurgetime = ::GetTickCount();
	paused = false;
	stopped= false;
	status = PS_EMPTY;
	insufficient = false;
	bIncreasedFile = false;
	m_bCompletionError = false;
	m_uTransferred = 0;
	m_iLastPausePurge = time(NULL);
	m_AllocateThread=NULL;
	if(thePrefs.GetNewAutoDown()){
		m_iDownPriority = PR_HIGH;
		m_bAutoDownPriority = true;
	}
	else{
		m_iDownPriority = PR_NORMAL;
		m_bAutoDownPriority = false;
	}
	xState=PFS_NORMAL;
	lastAICHRequestFailed = 0;
	verifystatus = VS_NONE;// X: [FV] - [FileVerify]
	memset(m_anStates,0,sizeof(m_anStates));
	//datarate = 0;
	m_uMaxSources = 0;
	m_bMD4HashsetNeeded = true;
	m_bAICHPartHashsetNeeded = true;
	//count = 0;
	percentcompleted = 0;
	completedsize = (uint64)0;
	leftsize = m_nFileSize;
	headerSize = 0;// X: [HP] - [HeaderPercent]
	m_bPreviewing = false;
	lastseencomplete = NULL;
	availablePartsCount=0;
	m_ClientSrcAnswered = 0;
	m_LastNoNeededCheck = 0; //Xman Xtreme Downloadmanager: Auto-A4AF-check
	m_is_A4AF_auto=false; //Xman Xtreme Downloadmanager: Auto-A4AF-check
	m_nTotalBufferData = 0;
	// X: [GB] - [Global Buffer]
	datareceived = false;
	m_nNextFlushBufferTime = 0;
	/*
	m_nLastBufferFlushTime = 0;
	*/	//Enig123::correct initialization
	//m_nLastBufferFlushTime = ::GetTickCount(); // netfinity: In case of rollover!
	m_uCompressionGain = 0;
	m_uCorruptionLoss = 0;
	m_uPartsSavedDueICH = 0;
	//m_category=0;
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
	m_tLastModified = 0;
	m_tUtcLastModified = 0;
	m_tCreated = 0;
	m_eFileOp = PFOP_NONE;
	m_uFileOpProgress = 0;
    m_bpreviewprio = false;
//    m_random_update_wait = t_rng->getUInt32()/(RAND32_MAX/1000);
	//lastSwapForSourceExchangeTick = ::GetTickCount();
	m_DeadSourceList.Init(false);
	m_bPauseOnPreview = false;
	//Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	m_nDownDatarate = 0;
	m_nDownDatarate10 = 0;
	// Maella end
	// Maella -Downloading source list-
	m_sourceListChange = false;
	// Maella end
	completedHeaderX=0;// X: [FV] - [FileVerify]
	formatInfo = NULL;// X: [FV] - [FileVerify]
	//Xman Xtreme Downloadmanager
	m_avgqr=0;
	m_sumqr=0;
	m_countqr=0;
	//Xman end

	//Xman sourcecache
	m_lastSoureCacheProcesstime = ::GetTickCount();

	//Xman
	m_PartsHashing = 0;		// SLUGFILLER: SafeHash
	// SiRoB: Flush Thread
	m_FlushSetting = NULL;
	//Xman end
}

CPartFile::~CPartFile()
{
	// Barry - Ensure all buffered data is written
		if (m_AllocateThread != NULL){
			// 2 minutes to let the thread finish
		m_AllocateThread->setPriority(Poco::Thread::PRIO_NORMAL);
		if(!m_AllocateThread->join(120000))
		{
			m_AllocateThread->terminate();
			delete m_AllocateThread;
		}
		}

	try
	{
		if (m_hpartfile.m_hFile != INVALID_HANDLE_VALUE)
			FlushBuffer(true);
	}
	catch(CFileException* e){
		e->Delete();
	}
	
	if (m_hpartfile.m_hFile != INVALID_HANDLE_VALUE){
		// commit file and directory entry
		m_hpartfile.Close();
		// Update met file (with current directory entry)
		//Xman 
		if(m_FlushSetting)
			LogError(_T("Warning: Flushsetting in function: %s for file: %s"), __FUNCTION__, GetFileName());
		//Xman end	
		SavePartFile();
	}

	POSITION pos;
	for (pos = gaplist.GetHeadPosition();pos != 0;)
		delete gaplist.GetNext(pos);

	pos = m_BufferedData_list.GetHeadPosition();
	while (pos){
		PartFileBufferedData *item = m_BufferedData_list.GetNext(pos);
		delete[] item->data;
		delete item;
	}
	delete m_pAICHRecoveryHashSet;
	m_pAICHRecoveryHashSet = NULL;
	// Maella -Extended clean-up II-
	theApp.clientlist->CleanUp(this);
	// Maella end

	//Xman sourcecache
	ClearSourceCache();
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
	(void)src_stats;
	(void)net_stats;
	CHECK_BOOL(m_bPreviewing);
	CHECK_BOOL(m_bLocalSrcReqQueued);
	CHECK_BOOL(m_bMD4HashsetNeeded);
	CHECK_BOOL(m_bAICHPartHashsetNeeded);
	(void)m_iLastPausePurge;
	//(void)count;
	(void)m_anStates;
	ASSERT( completedsize <= m_nFileSize );
	ASSERT( leftsize <= m_nFileSize );
	ASSERT( headerSize <= m_nFileSize );// X: [HP] - [HeaderPercent]
	(void)m_uCorruptionLoss;
	(void)m_uCompressionGain;
	(void)m_uPartsSavedDueICH; 
	(void)m_nDownDatarate; //Xman // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	(void)m_nDownDatarate10; //Xman // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	(void)m_sourceListChange; //Xman // Maella -New bandwidth control-
	(void)verifystatus;// X: [FV] - [FileVerify]
	(void)completedHeaderX;// X: [FV] - [FileVerify]
	(void)formatInfo;// X: [FV] - [FileVerify]
	(void)xState;
	(void)lastAICHRequestFailed;
	(void)m_fullname;
	(void)m_partmetfilename;
	(void)m_uTransferred;
	CHECK_BOOL(paused);
	CHECK_BOOL(stopped);
	CHECK_BOOL(insufficient);
	CHECK_BOOL(m_bCompletionError);
	ASSERT( m_iDownPriority == PR_LOW || m_iDownPriority == PR_NORMAL || m_iDownPriority == PR_HIGH );
	CHECK_BOOL(m_bAutoDownPriority);
	ASSERT( status == PS_READY || status == PS_EMPTY/* || status == PS_WAITINGFORHASH*/ || status == PS_ERROR || status == PS_COMPLETING || status == PS_COMPLETE );
	//CHECK_BOOL(newdate);
	(void)lastpurgetime;
	(void)m_LastNoNeededCheck; //Xman Xtreme Downloadmanager: Auto-A4AF-check
	CHECK_BOOL(m_is_A4AF_auto); //Xman Xtreme Downloadmanager: Auto-A4AF-check
	gaplist.AssertValid();
	requestedblocks_list.AssertValid();
	m_SrcpartFrequency.AssertValid();
	ASSERT( percentcompleted >= 0.0F && percentcompleted <= 100.0F );
	corrupted_list.AssertValid();
	(void)availablePartsCount;
	(void)m_ClientSrcAnswered;
	(void)s_LoadBar;
	//(void)s_CacheBar;// X: [CB] - [CacheBar]
	(void)s_ChunkBar;
	(void)s_ProgressBar; // morph4u :: PercentBar
	(void)m_lastRefreshedDLDisplay;
	m_downloadingSourceList.AssertValid();
	m_downloadingDeleteList.AssertValid(); //zz_fly :: delayed deletion of downloading source :: Enig123
	m_BufferedData_list.AssertValid();
	(void)m_nTotalBufferData;
	// X: [GB] - [Global Buffer]
	(void)datareceived;
	(void)m_nNextFlushBufferTime;
	//(void)m_nLastBufferFlushTime;
	(void)m_category;
	(void)m_dwFileAttributes;
}

void CPartFile::Dump(CDumpContext& dc) const
{
	CKnownFile::Dump(dc);
}
#endif

void CPartFile::CreatePartFile(LPCTSTR fileName)
{
	if (m_nFileSize > (uint64)MAX_EMULE_FILE_SIZE){
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_CREATEPARTFILE));
		SetStatus(PS_ERROR);
		return;
	}

	SetAutoCat();// HoaX_69 / Slugfiller: AutoCat // X: [AC] - [ActionChange] use AutoCat TempDir

	CString tempdirtouse;

	if(fileName){ // X: [IP] - [Import Parts]
		for (size_t i = 0; i < thePrefs.tempdir.GetCount(); i++) {
			if(_tcsncmp(thePrefs.GetTempDir(i),fileName,3) == 0){
				tempdirtouse = thePrefs.GetTempDir(i);
				break;
			}
		}
		if(tempdirtouse.IsEmpty()){
			tempdirtouse = fileName;
			tempdirtouse = tempdirtouse.Left(tempdirtouse.ReverseFind(_T('\\')));
			MakeFoldername(tempdirtouse);
			thePrefs.tempdir.Add(tempdirtouse);
		}
	}
	else{
		// decide which tempfolder to use
		tempdirtouse = thePrefs.GetCategory(m_category)->strTempPath;// X: [TD] - [TempDir]
		if(tempdirtouse.GetLength() == 0)
			tempdirtouse = theApp.downloadqueue->GetOptimalTempDir(m_category,GetFileSize());
	}

	SetPath(tempdirtouse);
	if(tempdirtouse.Right(1) != _T('\\'))// X: [BF] - [Bug Fix] get rid of X:\\001.part
		tempdirtouse += _T('\\');
	// use lowest free partfilenumber for free file (InterCeptor)
	size_t i = 0; 
	CString filename; 
	do{
		i++; 
		filename.Format(_T("%s%03u.part"), tempdirtouse, i); 
	}
	while (PathFileExists(filename));
	m_partmetfilename.Format(_T("%03u.part.met"), i); 
	m_fullname.Format(_T("%s.met"), filename);
	//CString partfull(RemoveFileExtension(m_fullname));
	SetFilePath(filename);

	if((fileName && MoveFile(fileName, filename)==0)|| // X: [IP] - [Import Parts]
		!m_hpartfile.Open(filename,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeReadWrite|CFile::shareDenyWrite|CFile::osSequentialScan)){
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_CREATEPARTFILE));
		SetStatus(PS_ERROR);
		return;
	}

	CTag* partnametag = new CTag(FT_PARTFILENAME,RemoveFileExtension(m_partmetfilename));
	taglist.Add(partnametag);
	
	Gap_Struct* gap = new Gap_Struct;
	gap->start = 0;
	gap->end = m_nFileSize - (uint64)1;
	gaplist.AddTail(gap);

	if (thePrefs.GetSparsePartFiles()){
		DWORD dwReturnedBytes = 0;
		if (g_VolumeInfo.GetVolumeInfoByPath(filename)->IsSupportSparseFiles() && !DeviceIoControl(m_hpartfile.m_hFile, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &dwReturnedBytes, NULL)) // X: [FSFS] - [FileSystemFeaturesSupport]
		{
			ASSERT(0);
			// Errors:
			// ERROR_INVALID_FUNCTION	returned by WinXP when attempting to create a sparse file on a FAT32 partition
			DWORD dwError = GetLastError();
			if (dwError != ERROR_INVALID_FUNCTION && thePrefs.GetVerboseLogPriority() <= DLP_VERYLOW)
				DebugLogError(_T("Failed to apply NTFS sparse file attribute to file \"%s\" - %s"), filename, GetErrorMessage(dwError, 1));
		}
	}

	struct _stat64 fileinfo;
	if (_tstat64(filename, &fileinfo) == 0){
		m_tLastModified = fileinfo.st_mtime;
		m_tCreated = fileinfo.st_ctime;
	}
	else
		AddDebugLogLine(false, _T("Failed to get file date for \"%s\" - %s"), filename, _tcserror(errno));

	m_dwFileAttributes = GetFileAttributes(filename);
	if (m_dwFileAttributes == INVALID_FILE_ATTRIBUTES)
		m_dwFileAttributes = 0;

	if(fileName){ // X: [IP] - [Import Parts]
		if(m_hpartfile.GetLength() < GetFileSize())
			AllocateNeededSpace();
		else if(m_hpartfile.GetLength() > GetFileSize()){
			try
			{
				TRACE(_T("Partfile \"%s\" is too large! Truncating %I64u bytes.\n"), GetFileName(), m_hpartfile.GetLength() - m_nFileSize);
				m_hpartfile.SetLength(m_nFileSize);
			}
			catch(CFileException* error){
				CString strError = GetResString(IDS_ERR_CREATEPARTFILE);
				TCHAR szError[MAX_CFEXP_ERRORMSG];
				if (error->GetErrorMessage(szError, ARRSIZE(szError))){
					strError += _T(" - ");
					strError += szError;
				}
				LogError(LOG_STATUSBAR, _T("%s"), strError);
				SetStatus(PS_ERROR);
				error->Delete();
				return;
			}
		}

		xState = IP_WAITING_AICH;
	}

	if (m_FileIdentifier.GetTheoreticalMD4PartHashCount() == 0)
		m_bMD4HashsetNeeded = false;
	if (m_FileIdentifier.GetTheoreticalAICHPartHashCount() == 0)
		m_bAICHPartHashsetNeeded = false;

	// SLUGFILLER: SafeHash
	// the important part
	m_PartsShareable.SetCount(GetPartCount());
	for (uint16 i = 0; i < GetPartCount();i++)
		m_PartsShareable[i] = false;
	//SLUGFILLER: SafeHash

	m_SrcpartFrequency.SetCount(GetPartCount());
	for (uint16 i = 0; i < GetPartCount();i++)
		m_SrcpartFrequency[i] = 0;
	paused = false;

	if (thePrefs.AutoFilenameCleanup())
		SetFileName(CleanupFilename(GetFileName()));

	SavePartFile();
	SetActive(theApp.IsConnected());
}

EPartFileLoadResult CPartFile::LoadPartFile(LPCTSTR in_directory,LPCTSTR in_filename)
{
	bool isnewstyle;
	uint8 version;
	EPartFileFormat partmettype = PMT_UNKNOWN;

#ifdef REPLACE_ATLMAP
	unordered_map<uint_ptr, Gap_Struct*> gap_map; // Slugfiller
#else
	CAtlMap<uint_ptr, Gap_Struct*> gap_map; // Slugfiller
#endif

	// ==> Spread bars [Slugfiller/MorphXT] - Stulle
	CMap<UINT,UINT,uint64,uint64> spread_start_map;
	CMap<UINT,UINT,uint64,uint64> spread_end_map;
	CMap<UINT,UINT,uint64,uint64> spread_count_map;
	// <== Spread bars [Slugfiller/MorphXT] - Stulle
	m_uTransferred = 0;
	m_partmetfilename = in_filename;
	SetPath(in_directory);
	m_fullname.Format((GetPath().Right(1) != _T('\\'))?_T("%s\\%s"):_T("%s%s"), GetPath(), m_partmetfilename);// X: [BF] - [Bug Fix] get rid of X:\\001.part

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

	ASSERT((PARTFILE_VERSION & I64TIMEMASKPART) == 0);// X: [E64T] - [Enable64BitTime]
	ASSERT((PARTFILE_SPLITTEDVERSION & I64TIMEMASKPART) == 0);
	ASSERT((PARTFILE_VERSION_LARGEFILE & I64TIMEMASKPART) == 0);

	setvbuf(metFile.m_pStream, NULL, _IOFBF, 16384);

	try{
		version = metFile.ReadUInt8();
		bool I64Time = ((version & I64TIMEMASKPART) != 0);// X: [E64T] - [Enable64BitTime]
		version&=~I64TIMEMASKPART;

		if (version != PARTFILE_VERSION && version != PARTFILE_SPLITTEDVERSION && version != PARTFILE_VERSION_LARGEFILE){
			metFile.Close();
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_BADMETVERSION), m_partmetfilename, GetFileName());
			return PLR_FAILED_METFILE_CORRUPT;
		}

		isnewstyle = (version == PARTFILE_SPLITTEDVERSION);
		partmettype = isnewstyle ? PMT_SPLITTED : PMT_DEFAULTOLD;
		if (!isnewstyle) {
			uint32 test;
			metFile.Seek(24, CFile::begin);
			metFile.Read(&test, 4);

			metFile.Seek(1, CFile::begin);

			if (test == 0x01020000) {
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
				LoadDateFromFile(&metFile,I64Time);// X: [E64T] - [Enable64BitTime]
				metFile.Read(gethash, 16);
				m_FileIdentifier.SetMD4Hash(gethash);
			}
		}
		else {
			LoadDateFromFile(&metFile,I64Time);// X: [E64T] - [Enable64BitTime]
			m_FileIdentifier.LoadMD4HashsetFromFile(&metFile, false);
		}

		bool bHadAICHHashSetTag = false;
		size_t tagcount = metFile.ReadUInt32();
		for (size_t j = 0; j < tagcount; j++){
			CTag* newtag = new CTag(&metFile, false);
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
					ASSERT( newtag->IsInt64() );// X: [64T] - [64BitTime]
					if (newtag->IsInt64())
						lastseencomplete = newtag->GetInt64();
					delete newtag;
					break;
										 }
				case FT_FILESIZE:{
					ASSERT( newtag->IsInt64() );// X: [64T] - [64BitTime]
					if (newtag->IsInt64())// X: [64T] - [64BitTime]
						SetFileSize(newtag->GetInt64());
					delete newtag;
					break;
								 }
				case FT_TRANSFERRED:{
					ASSERT( newtag->IsInt64() );// X: [64T] - [64BitTime]
					if (newtag->IsInt64())
						m_uTransferred = newtag->GetInt64();
					delete newtag;
					break;
									}
				case FT_COMPRESSION:{
					ASSERT( newtag->IsInt64() );// X: [64T] - [64BitTime]
					if (newtag->IsInt64())
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
							sint_ptr iUpPriority = newtag->GetInt();
							if( iUpPriority == PR_AUTO ){
								SetUpPriority(PR_HIGH, false);
								SetAutoUpPriority(true);
						}
						else{
								if (iUpPriority != PR_VERYLOW && iUpPriority != PR_LOW && iUpPriority != PR_NORMAL && iUpPriority != PR_HIGH && iUpPriority != PR_VERYHIGH && iUpPriority != PR_POWER) //Xman PowerRelease
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
					ASSERT( newtag->IsInt64() );// X: [64T] - [64BitTime]
					if (newtag->IsInt64())
					{
						SetLastPublishTimeKadSrc(newtag->GetInt64(), 0);// X: [64T] - [64BitTime]
						if(GetLastPublishTimeKadSrc() > (uint64)time(NULL)+KADEMLIAREPUBLISHTIMES)
						{
							//There may be a posibility of an older client that saved a random number here.. This will check for that..
							SetLastPublishTimeKadSrc(0,0);
						}
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
					ASSERT( newtag->IsInt64() );// X: [64T] - [64BitTime]
					if (newtag->IsInt64())
						m_nDlActiveTime = (uint_ptr)newtag->GetInt64();
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
							if (_stscanf_s(strPart, _T("%u"), &uPart) == 1)
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
					if (DecodeBase32(newtag->GetStr(), hash) == HASHSIZE)
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
				{
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
				}
				//Xman advanced upload-priority
				case FT_NOTCOUNTEDTRANSFERREDLOW:
				{
					ASSERT( newtag->IsInt() );
					if (newtag->IsInt())
						statistic.m_unotcountedtransferred = newtag->GetInt();
					delete newtag;
					break;
				}
				case FT_NOTCOUNTEDTRANSFERREDHIGH:
				{
					ASSERT( newtag->IsInt() );
					if (newtag->IsInt())
						statistic.m_unotcountedtransferred = ((uint64)newtag->GetInt() << 32) | (UINT)statistic.m_unotcountedtransferred;
					delete newtag;
					break;
				}
				case FT_LASTDATAUPDATE:
				{
					ASSERT( newtag->IsInt64(true) );// X: [64T] - [64BitTime]
					if (newtag->IsInt64(true))
						statistic.m_tlastdataupdate = newtag->GetInt64();
					delete newtag;
					break;
				}
				//Xman end

				default:{
				// ==> Spread bars [Slugfiller/MorphXT] - Stulle
						if (newtag->GetNameID()==0 && (newtag->GetName()[0]==FT_SPREADSTART || newtag->GetName()[0]==FT_SPREADEND || newtag->GetName()[0]==FT_SPREADCOUNT))
						{
							ASSERT( newtag->IsInt64(true) );
							if (newtag->IsInt64(true))
							{
							UINT spreadkey = atoi(&newtag->GetName()[1]);
							if (newtag->GetName()[0] == FT_SPREADSTART)
								spread_start_map.SetAt(spreadkey, newtag->GetInt64());
							else if (newtag->GetName()[0] == FT_SPREADEND)
								spread_end_map.SetAt(spreadkey, newtag->GetInt64());
							else if (newtag->GetName()[0] == FT_SPREADCOUNT)
								spread_count_map.SetAt(spreadkey, newtag->GetInt64());
							}
							delete newtag;
							break;
						}
				// <== Spread bars [Slugfiller/MorphXT] - Stulle
					if (newtag->GetNameID()==0 && (newtag->GetName()[0]==FT_GAPSTART || newtag->GetName()[0]==FT_GAPEND))
					{
						ASSERT( newtag->IsInt64() );
						if (newtag->IsInt64())
						{
							uint_ptr gapkey = atoi(&newtag->GetName()[1]);
#ifdef REPLACE_ATLMAP
							Gap_Struct* &gap = gap_map[gapkey];
							if(gap == NULL)
							{
								gap = new Gap_Struct;
#else
							Gap_Struct* gap;
							if (!gap_map.Lookup(gapkey, gap))
							{
								gap = new Gap_Struct;
								gap_map.SetAt(gapkey, gap);
#endif
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
					break;
				}
			}
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
		if (isnewstyle && (metFile.GetPosition()<metFile.GetLength()) ) {
			uint8 temp;
			metFile.Read(&temp,1);

			size_t parts = GetPartCount();	// assuming we will get all hashsets

			for (size_t i = 0; i < parts && (metFile.GetPosition() + 16 < metFile.GetLength()); i++){
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

	// Now to flush the map into the list (Slugfiller)
#ifdef REPLACE_ATLMAP
	for(unordered_map<uint_ptr, Gap_Struct*>::const_iterator it = gap_map.begin(); it != gap_map.end(); ++it)
	{
		Gap_Struct* gap = it->second;
#else
	for (POSITION pos = gap_map.GetStartPosition(); pos != NULL; ){
		Gap_Struct* gap;
		uint_ptr gapkey;
		gap_map.GetNextAssoc(pos, gapkey, gap);
#endif
		// BEGIN SLUGFILLER: SafeHash - revised code, and extra safety
		if (gap->start != -1 && gap->end != -1 && gap->start <= gap->end && gap->start < m_nFileSize){
			if (gap->end >= m_nFileSize)
				gap->end = m_nFileSize - (uint64)1; // Clipping
			AddGap(gap->start, gap->end); // All tags accounted for, use safe adding
		}
		delete gap;
		// END SLUGFILLER: SafeHash
	}

	// ==> Spread bars [Slugfiller/MorphXT] - Stulle
	for (POSITION pos = spread_start_map.GetStartPosition(); pos != NULL; ){
		UINT spreadkey;
		uint64 spread_start;
		uint64 spread_end;
		uint64 spread_count;
		spread_start_map.GetNextAssoc(pos, spreadkey, spread_start);
		if (!spread_end_map.Lookup(spreadkey, spread_end))
			continue;
		if (!spread_count_map.Lookup(spreadkey, spread_count))
			continue;
		if (!spread_count || spread_start >= spread_end)
			continue;
		statistic.AddBlockTransferred(spread_start, spread_end, spread_count);	// All tags accounted for
	}
	// <== Spread bars [Slugfiller/MorphXT] - Stulle

	// verify corrupted parts list
	POSITION posCorruptedPart = corrupted_list.GetHeadPosition();
	while (posCorruptedPart)
	{
		POSITION posLast = posCorruptedPart;
		uint_ptr uCorruptedPart = corrupted_list.GetNext(posCorruptedPart);
		if (IsComplete((uint64)uCorruptedPart*PARTSIZE, (uint64)(uCorruptedPart+1)*PARTSIZE-1, true))
			corrupted_list.RemoveAt(posLast);
	}
	if(corrupted_list.GetCount() > 0) // X: [IPR] - [Improved Part Recovery]
		xState = PART_CORRUPTED;

	//check if this is a backup
	//Xman
	// BEGIN SLUGFILLER: SafeHash - also update the partial name
	if(_tcsicmp(_tcsrchr(m_fullname, _T('.')), PARTMET_TMP_EXT) == 0 || _tcsicmp(_tcsrchr(m_fullname, _T('.')), PARTMET_BAK_EXT) == 0)
	{
		m_fullname = RemoveFileExtension(m_fullname);
		m_partmetfilename = RemoveFileExtension(m_partmetfilename);
	}
	// END SLUGFILLER: SafeHash

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
	struct _stat64 fileinfo;
	if (_tstat64(searchpath, &fileinfo) == 0){
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

		// SLUGFILLER: SafeHash - final safety, make sure any missing part of the file is gap
		if (m_hpartfile.GetLength() < m_nFileSize)
			AddGap(m_hpartfile.GetLength(), m_nFileSize - (uint64)1);
		// Goes both ways - Partfile should never be too large
		if (m_hpartfile.GetLength() > m_nFileSize){
			TRACE(_T("Partfile \"%s\" is too large! Truncating %I64u bytes.\n"), GetFileName(), m_hpartfile.GetLength() - m_nFileSize);
			m_hpartfile.SetLength(m_nFileSize);
		}
		// SLUGFILLER: SafeHash

		// the important part
		m_PartsShareable.SetCount(GetPartCount());
		for (uint16 i = 0; i < GetPartCount();i++)
			m_PartsShareable[i] = false;
		// END SLUGFILLER: SafeHash

		m_SrcpartFrequency.SetCount(GetPartCount());
		for (uint16 i = 0; i < GetPartCount();i++)
			m_SrcpartFrequency[i] = 0;
		SetStatus(PS_EMPTY);
		// check hashcount, filesatus etc
		if (!m_FileIdentifier.HasExpectedMD4HashCount()){
			ASSERT( m_FileIdentifier.GetAvailableMD4PartHashCount() == 0 );
			m_bMD4HashsetNeeded = true;
			m_sourcesaver.LoadSourcesFromFile(); // X: [ISS] - [Improved Source Save]
			return PLR_LOADSUCCESS;
		}
		else {
			m_bMD4HashsetNeeded = false;
			for (size_t i = 0; i < m_FileIdentifier.GetAvailableMD4PartHashCount(); i++){
				if (i < GetPartCount() && IsComplete((uint64)i*PARTSIZE, (uint64)(i + 1)*PARTSIZE - 1, true)){
					SetStatus(PS_READY);
					break;
				}
			}
		}

		if (gaplist.IsEmpty()){	// is this file complete already?
			// NEO: POFC - [PauseOnFileComplete] -- Xanatos -->
			if(thePrefs.m_bPauseOnFileComplete)
				StopOnFileComplete();
			else
			// NEO: POFC END <-- Xanatos --
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
			uint64 fdate = filestatus.m_mtime.GetTime();// X: [64T] - [64BitTime]
			if (fdate == 0){
				if (thePrefs.GetVerbose())
					AddDebugLogLine(false, _T("Failed to get file date of \"%s\" (%s)"), filestatus.m_szFullName, GetFileName());
			}
			else
				AdjustNTFSDaylightFileTime(fdate, filestatus.m_szFullName);

			if (m_tUtcLastModified != fdate){
				theApp.SplashHide(); //Xman new slpash-screen arrangement
				XMSGBOXPARAMS params;
				params.nTimeoutSeconds = 5;
				CString strFileInfo;
				strFileInfo.Format(GetResString(IDS_QUERYONHASHING2), GetFileName(), GetFilePath());// X: [QOH] - [QueryOnHashing]
				if(XMessageBox(theApp.emuledlg->GetSafeHwnd(), strFileInfo, _T("eMule"), MB_YESNO | MB_ICONQUESTION, &params) == IDNO){
					m_hpartfile.Close();
					return PLR_FAILED_OTHER;
				}
				strFileInfo.Format(_T("%s (%s)"), GetFilePath(), GetFileName());
				LogError(LOG_STATUSBAR, GetResString(IDS_ERR_REHASH), strFileInfo);
				// rehash
				//Xman
				// BEGIN SLUGFILLER: SafeHash
				SetStatus(PS_EMPTY);	// no need to wait for hashes with the new system
				PerformFirstHash();
				// END SLUGFILLER: SafeHash
			}
			//Xman
			// BEGIN SiRoB, SLUGFILLER: SafeHash - update completed, even though unchecked
			else {
				for (size_t i = 0; i < GetPartCount(); i++)
					if (IsComplete((uint64)i*PARTSIZE,(uint64)(i+1)*PARTSIZE-1, false))
						m_PartsShareable[i] = true;
			}
			// END SiRoB, SLUGFILLER: SafeHash
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
	m_sourcesaver.LoadSourcesFromFile(); // X: [ISS] - [Improved Source Save]
	return PLR_LOADSUCCESS;
}

bool CPartFile::SavePartFile(bool bDontOverrideBak)
{
	//Xman
	//MORPH - Flush Thread, no need to savepartfile now will be done when flushDone complet
	if (m_FlushSetting)
		return false;
	//MORPH - Flush Thread, no need to savepartfile now will be done when flushDone complet
	/*switch (status){
		case PS_WAITINGFORHASH:
		case PS_HASHING:
			return false;
	}*/

	// search part file
	CString searchpath(RemoveFileExtension(m_fullname));
	DWORD dwAttr = GetFileAttributes(searchpath);
	if (dwAttr == INVALID_FILE_ATTRIBUTES || (dwAttr & FILE_ATTRIBUTE_DIRECTORY))
	{
		LogError(GetResString(IDS_ERR_SAVEMET) + _T(" - %s"), m_partmetfilename, GetFileName(), GetResString(IDS_ERR_PART_FNF));
		return false;
	}

	//Xman
	// BEGIN SLUGFILLER: SafeHash - don't update the file date unless all parts are hashed
	if (!m_PartsHashing){
		// get filedate
		struct _stat64 st;
		_tstat64(searchpath, &st);
		if (_tstat64(searchpath, &st) == 0)
		{
			m_tLastModified = st.st_mtime;// X: [64T] - [64BitTime]
		m_tUtcLastModified = m_tLastModified;
			AdjustNTFSDaylightFileTime(m_tUtcLastModified, searchpath);
		}
		else if (thePrefs.GetVerbose())
				AddDebugLogLine(false, _T("Failed to get file date of \"%s\" (%s)"), m_partmetfilename, GetFileName());
		}
	// END SLUGFILLER: SafeHash

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
		uint8 nVersion=IsLargeFile()? PARTFILE_VERSION_LARGEFILE : PARTFILE_VERSION;
		bool I64Time=thePrefs.m_bEnable64BitTime;// X: [E64T] - [Enable64BitTime]
		if(I64Time)// X: [E64T] - [Enable64BitTime]
			nVersion|=I64TIMEMASKPART;
		file.WriteUInt8( nVersion );

		//date
		I64Time?file.WriteUInt64(m_tUtcLastModified):file.WriteUInt32((uint32)m_tUtcLastModified);// X: [E64T] - [Enable64BitTime]

		//hash
		m_FileIdentifier.WriteMD4HashsetToFile(&file);

		UINT uTagCount = 0;
		uint_ptr uTagCountFilePos = (uint_ptr)file.GetPosition();
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
			CTag lsctag(FT_LASTSEENCOMPLETE, lastseencomplete.GetTime(),I64Time);// X: [E64T] - [Enable64BitTime]
			lsctag.WriteTagToFile(&file);
			uTagCount++;
		}

		if (m_category){
			CTag categorytag(FT_CATEGORY, m_category);
			categorytag.WriteTagToFile(&file);
			uTagCount++;
		}

		if (GetLastPublishTimeKadSrc()){
			CTag kadLastPubSrc(FT_KADLASTPUBLISHSRC, GetLastPublishTimeKadSrc(),I64Time);// X: [E64T] - [Enable64BitTime]
			kadLastPubSrc.WriteTagToFile(&file);
			uTagCount++;
		}

		if (GetDlActiveTime()){
			CTag tagDlActiveTime(FT_DL_ACTIVE_TIME, GetDlActiveTime(),I64Time);// X: [E64T] - [Enable64BitTime]
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
		// ==> Spread bars [Slugfiller/MorphXT] - Stulle
			char sbnamebuffer[10];
			char* sbnumber = &sbnamebuffer[1];
			UINT i_sbpos = 0;
			if (IsLargeFile()) {
				for (POSITION pos = statistic.spreadlist.GetHeadPosition(); pos; ){
					uint64 count = statistic.spreadlist.GetValueAt(pos);
					if (!count) {
						statistic.spreadlist.GetNext(pos);
						continue;
					}
					uint64 start = statistic.spreadlist.GetKeyAt(pos);
					statistic.spreadlist.GetNext(pos);
					ASSERT(pos != NULL);	// Last value should always be 0
					uint64 end = statistic.spreadlist.GetKeyAt(pos);
					_itoa(i_sbpos,sbnumber,10); //Fafner: avoid C4996 (as in 0.49b vanilla) - 080731
					sbnamebuffer[0] = FT_SPREADSTART;
					CTag(sbnamebuffer,start,true).WriteTagToFile(&file);
					uTagCount++;
					sbnamebuffer[0] = FT_SPREADEND;
					CTag(sbnamebuffer,end,true).WriteTagToFile(&file);
					uTagCount++;
					sbnamebuffer[0] = FT_SPREADCOUNT;
					CTag(sbnamebuffer,count,true).WriteTagToFile(&file);
					uTagCount++;
					i_sbpos++;
				}
			} else {
				for (POSITION pos = statistic.spreadlist.GetHeadPosition(); pos; ){
					uint32 count = (uint32)statistic.spreadlist.GetValueAt(pos);
					if (!count) {
						statistic.spreadlist.GetNext(pos);
						continue;
					}
					uint32 start = (uint32)statistic.spreadlist.GetKeyAt(pos);
					statistic.spreadlist.GetNext(pos);
					ASSERT(pos != NULL);	// Last value should always be 0
					if (pos == NULL) {
						// this should no happen, but abort might prevent a crash?
						AddDebugLogLine(false, _T("Error in spreadbarinfo for partfile (%s). No matching end to start = %lu"), GetFileName(), start);
						break;
					}
					uint32 end = (uint32)statistic.spreadlist.GetKeyAt(pos);
					_itoa(i_sbpos,sbnumber,10); //Fafner: avoid C4996 (as in 0.49b vanilla) - 080731
					sbnamebuffer[0] = FT_SPREADSTART;
					CTag(sbnamebuffer,start).WriteTagToFile(&file);
					uTagCount++;
					sbnamebuffer[0] = FT_SPREADEND;
					CTag(sbnamebuffer,end).WriteTagToFile(&file);
					uTagCount++;
					sbnamebuffer[0] = FT_SPREADCOUNT;
					CTag(sbnamebuffer,count).WriteTagToFile(&file);
					uTagCount++;
					i_sbpos++;
				}
			}
		// <== Spread bars [Slugfiller/MorphXT] - Stulle
		// currupt part infos
		POSITION posCorruptedPart = corrupted_list.GetHeadPosition();
		if (posCorruptedPart)
		{
			CString strCorruptedParts;
			while (posCorruptedPart)
			{
				uint_ptr uCorruptedPart = corrupted_list.GetNext(posCorruptedPart);
				if (!strCorruptedParts.IsEmpty())
					strCorruptedParts += _T(',');
				strCorruptedParts.AppendFormat(_T("%u"), uCorruptedPart);
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
				uint32 nAICHHashSetSize = (HASHSIZE * (m_FileIdentifier.GetAvailableAICHPartHashCount() + 1)) + 2;
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
		for (size_t j = 0; j < taglist.GetCount(); j++){
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
#ifdef _DEBUG
		uint32 dbgMerged = 0;
#endif
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
#ifdef _DEBUG
					dbgMerged++;
#endif
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

		//Xman advanced upload-priority
		if (statistic.m_unotcountedtransferred)
		{
			CTag stag1(FT_NOTCOUNTEDTRANSFERREDLOW, (uint32)statistic.m_unotcountedtransferred);
			stag1.WriteTagToFile(&file);
			uTagCount++;

			CTag stag2(FT_NOTCOUNTEDTRANSFERREDHIGH, (uint32)(statistic.m_unotcountedtransferred >> 32));
			stag2.WriteTagToFile(&file);
			uTagCount++;
		}
		if (statistic.m_tlastdataupdate!=0)
		{
			CTag stag1(FT_LASTDATAUPDATE, statistic.m_tlastdataupdate,I64Time);// X: [E64T] - [Enable64BitTime]
			stag1.WriteTagToFile(&file);
			uTagCount++;
		}
		//Xman end

		file.Seek(uTagCountFilePos, CFile::begin);
		file.WriteUInt32(uTagCount);
		file.SeekToEnd();

		if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !CemuleDlg::IsRunning())){
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
	/*if (_tremove(m_fullname) != 0 && errno != ENOENT){
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
	//Xman don't overwrite bak files if last sessions crashed
	if(thePrefs.eMuleCrashedLastSession()) 
		::CopyFile(m_fullname, BAKName, TRUE); //allow one copy
	else
	//Xman end
	if (!::CopyFile(m_fullname, BAKName, FALSE)){
		if (thePrefs.GetVerbose())
			DebugLogError(_T("Failed to create backup of %s (%s) - %s"), m_fullname, GetFileName(), GetErrorMessage(GetLastError()));
	}*/
	// after successfully writing the temporary part.met file...
	//Xman don't overwrite bak files if last sessions crashed
	CString BAKName(m_fullname);
	BAKName.Append(PARTMET_BAK_EXT);
	if(!PathFileExists(BAKName)){
		if (_trename(m_fullname, BAKName) != 0 && errno != ENOENT){
			if (thePrefs.GetVerbose())
				DebugLogError(_T("Failed to create backup of %s (%s) - %s"), m_fullname, GetFileName(), GetErrorMessage(GetLastError()));
		}
	}
	else if(bDontOverrideBak || thePrefs.eMuleCrashedLastSession()){
		if (_tremove(m_fullname) != 0 && errno != ENOENT){
			if (thePrefs.GetVerbose())
				DebugLogError(_T("Failed to remove \"%s\" - %s"), m_fullname, _tcserror(errno));
		}
	}
	else{
		if (_tremove(BAKName) != 0/* && errno != ENOENT*/){
			if (thePrefs.GetVerbose())
				DebugLogError(_T("Failed to remove \"%s\" - %s"), BAKName, _tcserror(errno));
		}
		if (_trename(m_fullname, BAKName) != 0){
			if (thePrefs.GetVerbose())
				DebugLogError(_T("Failed to create backup of %s (%s) - %s"), m_fullname, GetFileName(), GetErrorMessage(GetLastError()));
		}
	}
	//Xman end

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
	return true;
}

void CPartFile::PartFileHashFinished(CKnownFile* result){
	ASSERT( result->GetFileIdentifier().GetTheoreticalMD4PartHashCount() == m_FileIdentifier.GetTheoreticalMD4PartHashCount() );
	ASSERT( result->GetFileIdentifier().GetTheoreticalAICHPartHashCount() == m_FileIdentifier.GetTheoreticalAICHPartHashCount() );
	//newdate = true;
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
				if (!IsCorruptedPart(nPart)) // X: [IPR] - [Improved Part Recovery]
					corrupted_list.AddTail(nPart);
			}
			else
				m_PartsShareable[nPart] = true;
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
			AddLogLine(false, GetResString(IDS_HASHINGDONE), GetFileName());
		if (thePrefs.GetVerbose())// X: move up
			AddDebugLogLine(true, _T("Completed file-hashing for \"%s\""), GetFileName());
		SetStatus(PS_READY);
		SavePartFile();
		theApp.sharedfiles->SafeAddKFile(this);
	}
	else{
		SetStatus(PS_READY);
		if (thePrefs.GetVerbose())
			DebugLogError(LOG_STATUSBAR, _T("File-hashing failed for \"%s\""), GetFileName());
		SavePartFile();
		xState = PART_CORRUPTED; // X: [IP] - [Import Parts] & [IPR] - [Improved Part Recovery]
	}
}

void CPartFile::AddGap(uint64 start, uint64 end)
{
	ASSERT( start <= end );

	for (POSITION pos1 = gaplist.GetHeadPosition(), pos2;(pos2 = pos1)!= NULL;){// X: [CI] - [Code Improvement]
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
	//Xman Code Improvement:
	//no need to do this, because of Maella Code
	//UpdateDisplayedInfo();
	//newdate = true;
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
		for (POSITION pos = m_BufferedData_list.GetHeadPosition();pos != 0;)
		{
			const PartFileBufferedData* cur_gap = m_BufferedData_list.GetNext(pos);
			if (   (cur_gap->start >= start          && cur_gap->end   <= end)
				|| (cur_gap->start >= start          && cur_gap->start <= end)
				|| (cur_gap->end   <= end            && cur_gap->end   >= start)
				|| (start          >= cur_gap->start && end            <= cur_gap->end)
			)	// should be equal to if (start <= cur_gap->end  && end >= cur_gap->start)
			{
				return false;	
			}
		}
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

uint64 CPartFile::GetTotalGapSizeInPart(uint_ptr uPart) const
{
	uint64 uRangeStart = (uint64)uPart * PARTSIZE;
	uint64 uRangeEnd = uRangeStart + PARTSIZE - 1;
	if (uRangeEnd >= m_nFileSize)
		uRangeEnd = m_nFileSize;
	return GetTotalGapSizeInRange(uRangeStart, uRangeEnd);
}

bool CPartFile::GetNextEmptyBlockInPart(uint_ptr partNumber, Requested_Block_Struct *result, uint64 bytesToRequest) const //Xman Dynamic block request (netfinity/morph)
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
		blockLimit = partStart + (uint64)((start - partStart)/EMBLOCKSIZE + 1)*EMBLOCKSIZE - 1;
		if (end > blockLimit)
			end = blockLimit;
		if (end > partEnd)
			end = partEnd;

		//Xman Dynamic block request (netfinity/Xman)
		bytesToRequest -= bytesToRequest % 10240; 
		if (bytesToRequest < 10240) bytesToRequest = 10240;
		else if (bytesToRequest > EMBLOCKSIZE) bytesToRequest = EMBLOCKSIZE;
		if((start + bytesToRequest) <= end && (end - start) > (bytesToRequest + 3072)) // Avoid creating small fragments
			end = start + bytesToRequest - 1;
		//Xman end

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
			}
			return true;
		}
		else
		{
        	uint64 tempStart = start;
        	uint64 tempEnd = end;

            bool shrinkSucceeded = ShrinkToAvoidAlreadyRequested(tempStart, tempEnd);
            if(shrinkSucceeded) {
				//Xman 
				/*
				AddDebugLogLine(false, _T("Shrunk interval to prevent collision with already requested block: Old interval %I64u-%I64u. New interval: %I64u-%I64u. File %s."), start, end, tempStart, tempEnd, GetFileName());
				*/
				//Xman end

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

	for (POSITION pos1 = gaplist.GetHeadPosition(), pos2;(pos2 = pos1) != NULL;){// X: [CI] - [Code Improvement]
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
	//Xman Code Improvement:
	//no need to do this, because of Maella Code
	//UpdateCompletedInfos();
	//UpdateDisplayedInfo();
	//newdate = true;
}

void CPartFile::UpdateCompletedInfos()
{
	headerSize=m_nFileSize;// X: [HP] - [HeaderPercent]
   	uint64 allgaps = 0; 

	for (POSITION pos = gaplist.GetHeadPosition();pos !=  0;){ 
		const Gap_Struct* cur_gap = gaplist.GetNext(pos);
		allgaps += cur_gap->end - cur_gap->start + 1;
		if(cur_gap->start < headerSize)
			headerSize=cur_gap->start;
	}

	UpdateCompletedInfos(allgaps);
}

uint64 CPartFile::GetHeaderSizeOnDisk() const{// X: [HP] - [HeaderPercent]
//	if(IsPartFile()){
	uint64 start=headerSize;
	for (POSITION pos = m_BufferedData_list.GetHeadPosition();pos != 0;){
		const PartFileBufferedData* cur_gap = m_BufferedData_list.GetNext(pos);
		if(cur_gap->start < start)
			start=cur_gap->start;
	}
//	}
	return start;
}
void CPartFile::UpdateCompletedInfos(uint64 uTotalGaps)
{
	if (uTotalGaps > m_nFileSize){
		ASSERT(0);
		uTotalGaps = m_nFileSize;
	}

	if (gaplist.GetCount() || requestedblocks_list.GetCount()){ 
		percentcompleted = (float)((1.0 - (double)uTotalGaps/(uint64)m_nFileSize) * 100.0);
		leftsize = uTotalGaps;
	completedsize = m_nFileSize - uTotalGaps;
	} 
	else{
		percentcompleted = 100.0F;
		leftsize = 0;
		completedsize = m_nFileSize;
	}
}

// morph4u :: PercentBar :: Start
void CPartFile::DrawProgressBar(CDC* dc, LPCRECT rect, bool bFlat)
{
	COLORREF m_crWindow  = ::GetSysColor(COLOR_WINDOW);
	COLORREF crHighlight = ::GetSysColor(COLOR_HIGHLIGHT);
	COLORREF crProgressBar;
	//COLORREF crProgressBarBk;

	//crProgressBarBk = MLC_RGBBLEND(crHighlight, m_crWindow, 16); 

	//status color +
	  switch (GetPartFileStatus()){
					case PS_DOWNLOADING:
						crProgressBar = RGB(200,250,200); //green
						break;
                    case PS_COMPLETING:
						crProgressBar = MLC_RGBBLEND(crHighlight, m_crWindow, 5); //blue
						break;
					case PS_WAITINGFORSOURCE:
						crProgressBar = MLC_RGBBLEND(crHighlight, m_crWindow, 5); //blue
						break;
					case PS_PAUSED:
						crProgressBar = RGB(255,250,200); //yellow
						break;			
					case PS_STOPPED:
					default:
						crProgressBar = RGB(255,225,225); //red
						break;
				}
	//status color -

	// gaps
	    uint64 allgaps = 0;
	    for (POSITION pos = gaplist.GetHeadPosition();pos !=  0;){
		    const Gap_Struct* cur_gap = gaplist.GetNext(pos);
		    allgaps += cur_gap->end - cur_gap->start + 1;
		    bool gapdone = false;
		    uint64 gapstart = cur_gap->start;
		    uint64 gapend = cur_gap->end;
		    for (size_t i = 0; i < GetPartCount(); i++){
			    if (gapstart >= (uint64)i*PARTSIZE && gapstart <= (uint64)(i+1)*PARTSIZE - 1){ // is in this part?
				    if (gapend <= (uint64)(i+1)*PARTSIZE - 1)
					    gapdone = true;
				    else
					    gapend = (uint64)(i+1)*PARTSIZE - 1; // and next part
			    }
		    }
	    }
		
	    s_ProgressBar.Draw(dc, rect->left, rect->top, rect->right - rect->left, bFlat);// X: [CI] - [Code Improvement] BarShader

		// Draw progress (in percent)
		float blockpixel = (float)(rect->right - rect->left)/(float)m_nFileSize;
		int ondiskwidth = (int)((uint64)(m_nFileSize - allgaps - m_nTotalBufferData)*blockpixel + 0.5F);
		
			RECT gaprect =
			{
				rect->left,
				rect->top,
				rect->left + ondiskwidth,
				gaprect.top + 15
			}; 
			dc->FillSolidRect(&gaprect, crProgressBar);

			gaprect.left = gaprect.right;
			gaprect.right = rect->right;
			dc->FillSolidRect(&gaprect, m_crWindow/*crProgressBarBk*/); //Xman Code Improvement: FillSolidRect
 }
// morph4u :: PercentBar :: End

void CPartFile::DrawPartStatusBar(CDC* dc, LPCRECT rect, UINT nPart, bool bFlat){
	s_ChunkBar.Draw(dc, rect->left, rect->top, rect->right - rect->left, nPart * PARTSIZE, PARTSIZE, bFlat);
} 

//Xman Maella -Code Improvement-
void CPartFile::DrawStatusBar(CDC* dc, LPCRECT rect, bool bFlat) 
{
	COLORREF crProgress;
	COLORREF crProgressBk;
	COLORREF crHave;
	COLORREF crPending;
	COLORREF crMissing;
	//COLORREF crStartedButIncomplete; //Xman: from zz: other color for incomplete chunks
	COLORREF crInBuffer;
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
		crInBuffer = RGB(160, 160, 160);
	}
	else
	{
		if (bFlat)
			crProgress = RGB(0, 150, 0);
		else
			crProgress = RGB(0, 224, 0);
		crProgressBk = RGB(224, 224, 224);
		if (notgray) {
			crInBuffer = RGB(160, 160, 160);
			crMissing = RGB(255, 0, 0);
			if (bFlat) {
				crHave = RGB(32, 32, 32);
			} else {
				crHave = RGB(104, 104, 104);
			}
			crPending = RGB(255, 208, 0);
		} else {
			crInBuffer = RGB(140, 140, 140);
			crMissing = RGB(191, 64, 64);
			if (bFlat) {
				crHave = RGB(64, 64, 64);
			} else {
				crHave = RGB(116, 116, 116);
			}
			crPending = RGB(191, 168, 64);
		}
	}

	// Set Size
	s_ChunkBar.SetFileSize(GetFileSize());

	if (status == PS_COMPLETE || status == PS_COMPLETING)
	{
		s_ChunkBar.Fill(crProgress);
		s_ChunkBar.Draw(dc, rect->left, rect->top, rect->right - rect->left, bFlat);// X: [CI] - [Code Improvement] BarShader
		//percentcompleted = 100.0F;
		//leftsize = 0;
		//completedsize = m_nFileSize;
	}
	else if (eVirtualState == PS_INSUFFICIENT || status == PS_ERROR)
	{
		s_ChunkBar.Fill(crHave);
		int iOldBkColor = dc->SetBkColor(RGB(255, 255, 0));
		if (theApp.m_brushBackwardDiagonal.m_hObject)
			dc->FillRect(rect, &theApp.m_brushBackwardDiagonal);
		else
			dc->FillSolidRect(rect, RGB(255, 255, 0));
		dc->SetBkColor(iOldBkColor);
		//Xman Code Improvement
		//No need to update here, because we already update every sescond (Maella Code)
		//UpdateCompletedInfos();
	}
	else
	{
		s_ChunkBar.Fill(crHave);
		/*//Xman: from zz: other color for incomplete chunks
		for(uint64 haveSlice = 0; haveSlice < GetFileSize(); haveSlice+=PARTSIZE) {
			if(!IsComplete(haveSlice, min(haveSlice+(PARTSIZE-1), GetFileSize()), false))
				s_ChunkBar.FillRange(haveSlice, min(haveSlice+(PARTSIZE-1), GetFileSize()), crStartedButIncomplete);
		}
		//Xman end*/

		uint64 bufStart = 0;
		uint64 bufEnd = 0;
	    for (POSITION pos = m_BufferedData_list.GetHeadPosition();pos !=  0;){
			PartFileBufferedData *item = m_BufferedData_list.GetNext(pos);
			if(bufEnd != item->start){
				if(bufEnd)
					s_ChunkBar.FillRange(bufStart, bufEnd - 1, crInBuffer);
				bufStart = item->start;
			}
			bufEnd = item->end + 1;
	    }
		if(bufEnd)
			s_ChunkBar.FillRange(bufStart, bufEnd - 1, crInBuffer);

		// red gaps
	    uint64 allgaps = 0;
	    for (POSITION pos = gaplist.GetHeadPosition();pos !=  0;){
		    const Gap_Struct* cur_gap = gaplist.GetNext(pos);
		    allgaps += cur_gap->end - cur_gap->start + 1;
		    bool gapdone = false;
		    uint64 gapstart = cur_gap->start;
		    uint64 gapend = cur_gap->end;
		    for (size_t i = 0; i < GetPartCount(); i++){
			    if (gapstart >= (uint64)i*PARTSIZE && gapstart <= (uint64)(i+1)*PARTSIZE - 1){ // is in this part?
				    if (gapend <= (uint64)(i+1)*PARTSIZE - 1)
					    gapdone = true;
				    else
					    gapend = (uint64)(i+1)*PARTSIZE - 1; // and next part
    
				    // paint
				    COLORREF color;
				    if (m_SrcpartFrequency.GetCount() >= i && m_SrcpartFrequency[(uint16)i])
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

	    s_ChunkBar.Draw(dc, rect->left, rect->top, rect->right - rect->left, bFlat);// X: [CI] - [Code Improvement] BarShader

		// Draw green progress (in percent)
		float blockpixel = (float)(rect->right - rect->left)/(float)m_nFileSize;
		int progwidth = (int)((uint64)(m_nFileSize - allgaps)*blockpixel + 0.5F);
		int ondiskwidth = (int)((uint64)(m_nFileSize - allgaps - m_nTotalBufferData)*blockpixel + 0.5F);
		if(!bFlat) {
			s_LoadBar.DrawRect(dc, rect->left, rect->top, ondiskwidth, crProgress, false);
			//draw yellow buffer
			if(progwidth > ondiskwidth)
			s_LoadBar.DrawRect(dc, rect->left + ondiskwidth, rect->top, progwidth - ondiskwidth, crPending, false);
		} else {
			RECT gaprect =
			{
				rect->left,
				rect->top,
				rect->left + ondiskwidth,
				gaprect.top + 2
			}; 

			//Xman Code Improvement: FillSolidRect
			//dc->FillRect(&gaprect, &CBrush(crProgress));
			dc->FillSolidRect(&gaprect, crProgress);
			//draw yellow buffer
			if(progwidth > ondiskwidth){
			gaprect.left = gaprect.right;
				gaprect.right += progwidth - ondiskwidth;
			dc->FillSolidRect(&gaprect, crPending);
			}
			//draw gray progress only if flat
			gaprect.left = gaprect.right;
			gaprect.right = rect->right;
			//Xman Code Improvement: FillSolidRect
			//dc->FillRect(&gaprect, &CBrush(crProgressBk));
			dc->FillSolidRect(&gaprect, crProgressBk);
		}
		//Xman Code Improvement
		//No need to update here, because we already update every sescond (Maella Code)
	    //UpdateCompletedInfos(allgaps); 
    }

	// additionally show any file op progress (needed for PS_COMPLETING and PS_WAITINGFORHASH)
	if (GetFileOp() != PFOP_NONE)
	{
		float blockpixel = (float)(rect->right - rect->left)/100.0F;
		RECT rcFileOpProgress;
		rcFileOpProgress.top = rect->top;
		rcFileOpProgress.bottom = rcFileOpProgress.top + PROGRESS_HEIGHT;
		rcFileOpProgress.left = rect->left;
		if (!bFlat)
		{
			s_LoadBar.DrawRect(dc, rcFileOpProgress.left, rcFileOpProgress.top, (int)(GetFileOpProgress()*blockpixel + 0.5F), RGB(255,208,0), false);
		}
		else
		{
			rcFileOpProgress.right = rcFileOpProgress.left + (UINT)(GetFileOpProgress()*blockpixel + 0.5F);
			//Xman Code Improvement: FillSolidRect
			//dc->FillRect(&rcFileOpProgress, &CBrush(RGB(255,208,0)));
			dc->FillSolidRect(&rcFileOpProgress, RGB(255,208,0));
			rcFileOpProgress.left = rcFileOpProgress.right;
			rcFileOpProgress.right = rect->right;
			//Xman Code Improvement: FillSolidRect
			//dc->FillRect(&rcFileOpProgress, &CBrush(crProgressBk));
			dc->FillSolidRect(&rcFileOpProgress, crProgressBk);
		}
	}
	/*if(thePrefs.IsExtControlsEnabled() && m_nTotalBufferData != 0){// X: [CB] - [CacheBar]
		// Draw yellow buffer usage (in percent)
		float blockpixel = (float)(rect->right - rect->left)/(float)thePrefs.m_uFileBufferSize;
		if(!bFlat) {
			s_CacheBar.SetWidth((int)(m_nTotalBufferData*blockpixel + 0.5F));
			s_CacheBar.Fill(crPending);
			s_CacheBar.Draw(dc, rect->left, rect->bottom-PROGRESS_HEIGHT, false);
		} else {
			RECT cbrect =
			{
				rect->left,
				rect->bottom - 2,
				rect->left + (uint32)(m_nTotalBufferData*blockpixel + 0.5F),
				rect->bottom
			}; 
			dc->FillSolidRect(&cbrect, crPending);
			cbrect.left = cbrect.right;
			cbrect.right = rect->right;
			dc->FillSolidRect(&cbrect, crHave);
		}
	}*/
}
//Xman end

void CPartFile::WritePartStatus(CSafeMemFile* file) const
{
	size_t uED2KPartCount = GetED2KPartCount();
	file->WriteUInt16((uint16)uED2KPartCount);

	size_t uPart = 0;
	while (uPart != uED2KPartCount)
	{
		uint8 towrite = 0;
		for (size_t i = 0; i < 8; i++)
		{
			//Xman
			//if (uPart < GetPartCount() && IsComplete((uint64)uPart*PARTSIZE, (uint64)(uPart + 1)*PARTSIZE - 1, true))
			if (uPart < GetPartCount() && IsPartShareable(uPart))	// SLUGFILLER: SafeHash
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

// ==> Source Counts Are Cached derivated from Khaos [SiRoB] - Stulle
/*
UINT CPartFile::GetValidSourcesCount() const
{
	UINT counter = 0;
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
*/
UINT CPartFile::GetValidSourcesCount() const
{
	return m_anStates[DS_ONQUEUE]+m_anStates[DS_DOWNLOADING]+m_anStates[DS_CONNECTED]+m_anStates[DS_REMOTEQUEUEFULL];
}
size_t CPartFile::GetNotCurrentSourcesCount() const
{
	return srclist.GetCount() - m_anStates[DS_DOWNLOADING] - m_anStates[DS_ONQUEUE];
}
// <== Source Counts Are Cached derivated from Khaos [SiRoB] - Stulle

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
	if (paused)
		return PS_PAUSED;
	return PS_INSUFFICIENT;
}
void CPartFile::AddDownloadingSource(CUpDownClient* client){
	//zz_fly :: delayed deletion of downloading source :: Enig123 :: Start
	POSITION pos2 = m_downloadingDeleteList.Find(client); //for security, delete it from m_downloadingDeleteList first
	if(pos2)
		m_downloadingDeleteList.RemoveAt(pos2);
	//zz_fly :: delayed deletion of downloading source :: Enig123 :: End

	POSITION pos = m_downloadingSourceList.Find(client); // to be sure
	if(pos == NULL){
		m_downloadingSourceList.AddTail(client);
		//Xman
		// Maella -New bandwidth control-
		// We need to detect a change in the source list
		// It's necessary for the download 'balancing' (see ::Process)
		m_sourceListChange = true;
		//Xman end
	}
}

void CPartFile::RemoveDownloadingSource(CUpDownClient* client){
	//zz_fly :: delayed deletion of downloading source :: Enig123 :: Start
	if(m_sourceListChange && m_downloadingDeleteList.Find(client) != NULL) //already trying to delete
		return;
	//zz_fly :: delayed deletion of downloading source :: Enig123 :: End

	POSITION pos = m_downloadingSourceList.Find(client); // to be sure
	if(pos != NULL){
		//zz_fly :: delayed deletion of downloading source :: Enig123 :: Start
		/*
		m_downloadingSourceList.RemoveAt(pos);
		*/
		m_downloadingDeleteList.AddTail(client);
		//zz_fly :: delayed deletion of downloading source :: Enig123 :: End
		//Xman
		// Maella -New bandwidth control-
		// We need to detect a change in the source list
		// It's necessary for the download 'balancing' (see ::Process)
		m_sourceListChange = true;
		//Xman end
	}
}

//Xman
// Maella -New bandwidth control-
uint32 CPartFile::Process(uint32 maxammount, bool isLimited, bool fullProcess)
{
#ifdef _DEBUG// X: [RDL] - [Remove Debug Log]
	if (thePrefs.m_iDbgHeap >= 2)
		ASSERT_VALID(this);
#endif

	UINT nOldTransSourceCount = GetSrcStatisticsValue(DS_DOWNLOADING);
	const DWORD dwCurTick = ::GetTickCount();

	// If buffer size exceeds limit, or if not written within time limit, flush data
	if (/*(m_nTotalBufferData > thePrefs.GetFileBufferSize()) || */(m_nNextFlushBufferTime && dwCurTick > m_nNextFlushBufferTime ))// X: [GB] - [Global Buffer]
	{
		// Avoid flushing while copying preview file
		if (!m_bPreviewing)
		FlushBuffer();
	}


	uint32 receivedBlockTotal = 0;
	// Remark: A client could be removed from the list during the 
	//         processing or the entries swapped.
	// Check if the list has been modified during the processing
	DoDelayedDeletion(); //zz_fly :: delayed deletion of downloading source :: Enig123

	POSITION pos = m_downloadingSourceList.GetHeadPosition();
	for(size_t i = 0; i < m_downloadingSourceList.GetCount() && pos != NULL; i++){
		POSITION cur_pos = pos;
		CUpDownClient* cur_src = m_downloadingSourceList.GetNext(pos);
#ifdef _DEBUG// X: [RDL] - [Remove Debug Log]
		if (thePrefs.m_iDbgHeap >= 2)
			ASSERT_VALID( cur_src );
#endif
		//zz_fly :: delayed deletion of downloading source :: Enig123 :: Start
		if (m_sourceListChange && m_downloadingDeleteList.Find(cur_src))
			continue;
		//zz_fly :: delayed deletion of downloading source :: Enig123 :: End
		if(cur_src->GetDownloadState() != DS_DOWNLOADING){ 
			ASSERT(FALSE); // Should never happend
			RemoveDownloadingSource(cur_src); // => security
		}
		else
		{
			//Xman remark: moved the CheckDownloadTimeout to fullprocess
			//because it causes a sourceListChange... a sourceListChange should be avoided...
			//because we go over some function if m_sourceListChange and are not full accurate in this case
			//a sourceListChange can still occur on exception
			//cur_src->CheckDownloadTimeout();
			//
			if (cur_src->socket==NULL)
			{
				continue;
			}
			if(isLimited == false){
				// Always call this method to avoid a flag (enabled/disable)
					cur_src->socket->DisableDownloadLimit();
			}
			else {
				if(maxammount > 6){ // let room for header size
					// Maella -Overhead compensation (pseudo full upload rate control)-
					uint64 receivedBlock;
					//Xman avoid the silly window syndrome
					/*
					The fast sender will quickly fill the receiver's TCP window.
					The receiver then reads N bytes, N being a relatively small number
					compared to the network frame size. A naïve stack will immediately send
					an ACK to the sender to tell it that there are now N bytes available in
					its TCP window. This will cause the sender to send N bytes of data; 
					since N is smaller than the frame size, there's relatively more protocol
					overhead in the packet compared to a full frame. Because the receiver is
					slow, the TCP window stays very small, and thus hurts throughput because
					the ratio of protocol overhead to application data goes up.
					*/
					uint32 tempmaxamount;
					//zz_fly :: netfinity: Maximum Segment Size :: start
					//we can get mss from socket, so we should not use a fixed MAXFRAGSIZE here. get it from socket.
					/*
					if(maxammount % UDP_KAD_MAXFRAGMENT == 0) {
						tempmaxamount=maxammount;
					} else {
						tempmaxamount= UDP_KAD_MAXFRAGMENT*(maxammount/UDP_KAD_MAXFRAGMENT+1);
					}
					*/
					uint32 maxfragsizefromsocket;
					if(thePrefs.retrieveMTUFromSocket && (cur_src->socket->m_dwMSS!=0))
						maxfragsizefromsocket = cur_src->socket->m_dwMSS;
					else
						maxfragsizefromsocket = UDP_KAD_MAXFRAGMENT;

					if(maxammount % maxfragsizefromsocket == 0) {
						tempmaxamount=maxammount;
					} else {
						tempmaxamount= maxfragsizefromsocket*(maxammount/maxfragsizefromsocket+1);
					}
					//zz_fly :: netfinity: Maximum Segment Size :: start

					
					// Use the global statistic to measure the amount of data (cleaner than a call back)
					receivedBlock = theApp.pBandWidthControl->GeteMuleIn();
					cur_src->socket->SetDownloadLimit(tempmaxamount); // Trig OnReceive() (go-n-stop mode)							
					receivedBlock = theApp.pBandWidthControl->GeteMuleIn() - receivedBlock;
					//Xman end: avoid the silly window syndrome
					// Maella end
					if(receivedBlock > 0){
						receivedBlockTotal += (uint32)receivedBlock;
						if(maxammount > (uint32)receivedBlock){
							maxammount -= (uint32)receivedBlock;
						} else {
							// Necessary because IP + TCP overhead
							maxammount = 0;
						}

						// Try to 'balance' the download between clients.
						// Move the 'downloader' at the end of the list.
						//zz_fly :: delayed deletion of downloading source :: Enig123 :: Start
						/*
						if(m_sourceListChange == false && cur_src->GetDownloadState() == DS_DOWNLOADING){
						*/
						if((m_sourceListChange == false || m_downloadingDeleteList.Find(cur_src) == NULL) && cur_src->GetDownloadState() == DS_DOWNLOADING){
						//zz_fly :: delayed deletion of downloading source :: Enig123 :: End
							m_downloadingSourceList.RemoveAt(cur_pos);
							m_downloadingSourceList.AddTail(cur_src);
						}
					}							
				}
				//Xman avoid the silly window syndrome
				// In case of an exception, the instance of the client might have been deleted
				//zz_fly :: delayed deletion of downloading source :: Enig123 :: Start
				/*
				if(m_sourceListChange == false &&
				*/
				if((m_sourceListChange == false || m_downloadingDeleteList.Find(cur_src) == NULL) &&
				//zz_fly :: delayed deletion of downloading source :: Enig123 :: End
					cur_src->socket != NULL && 
					cur_src->GetDownloadState() == DS_DOWNLOADING){
						// Block OnReceive() (go-n-stop mode)	
						cur_src->socket->SetDownloadLimit(0);
					}
				//Xman end: avoid the silly window syndrome
			}
		}
	}


	if(fullProcess)
	{
		// Xman -New Save/load Sources- enkeyDEV(Ottavio84)
		m_sourcesaver.Process();
		// Xman end
		bool downloadingbefore=m_anStates[DS_DOWNLOADING]>0;
		// ==> Source Counts Are Cached derivated from Khaos [SiRoB] - Stulle
		UINT	m_anStatesTemp[STATES_COUNT];
		memset(m_anStatesTemp,0,sizeof(m_anStatesTemp));
		// <== Source Counts Are Cached derivated from Khaos [SiRoB] - Stulle
		// -khaos--+++> Moved this here, otherwise we were setting our permanent variables to 0 every tenth of a second...
		// ==> Source Counts Are Cached derivated from Khaos [SiRoB] - Stulle
		/*
		memset(m_anStates,0,sizeof(m_anStates));
		*/
		//memset(m_anStatesTemp,0,sizeof(m_anStatesTemp));
		// <== Source Counts Are Cached derivated from Khaos [SiRoB] - Stulle
		memset(src_stats,0,sizeof(src_stats));
		memset(net_stats,0,sizeof(net_stats));
		uint_ptr nCountForState;

		for (POSITION pos = srclist.GetHeadPosition(); pos != NULL;)
		{
			CUpDownClient* cur_src = srclist.GetNext(pos);
#ifdef _DEBUG// X: [RDL] - [Remove Debug Log]
			if (thePrefs.m_iDbgHeap >= 2)
				ASSERT_VALID( cur_src );
#endif

			// BEGIN -rewritten- refreshing statistics (no need for temp vars since it is not multithreaded)
			nCountForState = cur_src->GetDownloadState();
			//special case which is not yet set as downloadstate
			if (nCountForState == DS_ONQUEUE)
			{
				if( cur_src->IsRemoteQueueFull() )
					nCountForState = DS_REMOTEQUEUEFULL;
			}
			//Xman
			if(cur_src->GetUploadState()==US_BANNED)//faster Code
				nCountForState = DS_BANNED;
			//Xman end
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

			// ==> Source Counts Are Cached derivated from Khaos [SiRoB] - Stulle
			/*
			ASSERT( nCountForState < sizeof(m_anStates)/sizeof(m_anStates[0]) );
			m_anStates[nCountForState]++;
			*/
			ASSERT( nCountForState < _countof(m_anStatesTemp) );
			m_anStatesTemp[nCountForState]++;
			// <== Source Counts Are Cached derivated from Khaos [SiRoB] - Stulle

			switch (cur_src->GetDownloadState())
			{
				case DS_DOWNLOADING:{
					cur_src->CheckDownloadTimeout(); //Xman 5.0 moved to here
					break;
				}
				// Do nothing with this client..
				case DS_BANNED:
				case DS_ERROR:
					break;
				// [ionix] - Fix continue asking sources
				case DS_CONNECTED:
					if(cur_src->socket == NULL || cur_src->socket->IsConnected() == false)
					{
						cur_src->SetDownloadState(DS_NONE);	

						if (thePrefs.GetLogUlDlEvents ())
							AddDebugLogLine (DLP_VERYLOW,false,_T("#### %s disconnected socket while we're asking for a file"),cur_src->GetUserName ());
					}
					break;
                                // [ionix] - END
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
				case DS_NONEEDEDPARTS:{ 
					// we try to purge noneeded source, even without reaching the limit
					if( cur_src->GetDownloadState() == DS_NONEEDEDPARTS && (dwCurTick - lastpurgetime) > 35000 ){ //Xman changed to 30
						if( !cur_src->SwapToAnotherFile( true , false, false , NULL ) ){
							//however we only delete them if reaching the limit
							if (GetSourceCount() >= (GetMaxSources()*.8 )){
								if(thePrefs.GetLogDrop())
									AddDebugLogLine(false, _T("-o- NNP dropped client: %s, %s, from %s"), cur_src->DbgGetFullClientSoftVer(), cur_src->GetUserName(), this->GetFileName());
								cur_src->droptime=dwCurTick;
								theApp.downloadqueue->RemoveSource( cur_src );
								lastpurgetime = dwCurTick;
								break; //Johnny-B - nothing more to do here (good eye!)
							}			
						}
						else{
							if(thePrefs.GetLogA4AF())
								AddDebugLogLine(false, _T("-o- NNP swapped client: %s, %s, from %s to %s"), cur_src->DbgGetFullClientSoftVer(), cur_src->GetUserName(), this->GetFileName(), cur_src->GetRequestFile()->GetFileName());
							cur_src->DontSwapTo(this);
							lastpurgetime = dwCurTick;
							break;
						}
					}

					// doubled reasktime for no needed parts - save connections and traffic
					// Maella -Spread Request- (idea SlugFiller)
					if(!((cur_src->GetNextTCPAskedTime() == 0) //Xman -Reask sources after IP change- v4
						|| (dwCurTick - cur_src->GetLastAskedTime()) > 2 * cur_src->GetJitteredFileReaskTime()))
						break; 
					// Maella end
				}
				case DS_ONQUEUE:{
						//Xman Xtreme Downloadmanager
					if(GetSourceCount() >= (0.8 * GetMaxSources())  ) /*&& cur_src->Credits() && cur_src->Credits()->GetMyScoreRatio(cur_src->GetIP())<1.7f*/ 
					{ 
						//it's droptime
						//FullQueue
						if(cur_src->IsRemoteQueueFull() == true && (cur_src->GetLastFileAskedTime(this) + (8*60*1000))<dwCurTick && !(cur_src->socket && cur_src->socket->IsConnected()) && (dwCurTick - lastpurgetime) > 45000 ) //if socket, it can be, that we receive the QR later
						{
							if( !cur_src->SwapToAnotherFile( true , false, false , NULL ))
							{
								// Purge
								if(thePrefs.GetLogDrop())
									AddDebugLogLine(false, _T("-o- FQ dropped client: %s, %s, from %s"), cur_src->DbgGetFullClientSoftVer(), cur_src->GetUserName(), this->GetFileName());
								cur_src->droptime=dwCurTick;
								theApp.downloadqueue->RemoveSource(cur_src);
								lastpurgetime = dwCurTick;
								break;
							}
							else
							{
								if(thePrefs.GetLogA4AF())
									AddDebugLogLine(false, _T("-o- FQ swapped client: %s, %s, from %s to %s"), cur_src->DbgGetFullClientSoftVer(), cur_src->GetUserName(), this->GetFileName(), cur_src->GetRequestFile()->GetFileName());
								cur_src->DontSwapTo(this);
								lastpurgetime = dwCurTick;
								break;
							}
						}
						else
						/* //Xman4.5: leechers should have been deleted earlier
						//Leechers:
						if(cur_src->IsBanned() && (dwCurTick - lastpurgetime > 35000) ) 
						{
							// Purge
							if(thePrefs.GetLogDrop())
								AddDebugLogLine(false, _T("-o- banned dropped client: %s, %s, from %s"), cur_src->DbgGetFullClientSoftVer(), cur_src->GetUserName(), this->GetFileName());
							cur_src->droptime=dwCurTick;
							theApp.downloadqueue->RemoveSource(cur_src);
							lastpurgetime = dwCurTick;
							break;
						}
						else */
							//Non emules:
						if(cur_src->IsEmuleClient() == false && (dwCurTick - cur_src->enterqueuetime>(60*60*1000)) && cur_src->GetTransferredDown()<1000000 && (dwCurTick - lastpurgetime > 60000) ) //if socket, it can be, that we receive the QR later
						{
							// Purge
							if(thePrefs.GetLogDrop())
								AddDebugLogLine(false, _T("-o- non emule dropped client: %s, %s, from %s"), cur_src->DbgGetFullClientSoftVer(), cur_src->GetUserName(), this->GetFileName());
							cur_src->droptime=dwCurTick;
							theApp.downloadqueue->RemoveSource(cur_src);
							lastpurgetime = dwCurTick;
							break;
						}
						else
						//High QR:
						if(cur_src->IsEmuleClient() && cur_src->Credits() && cur_src->credits->GetMyScoreRatio(cur_src->GetIP())<1.5f && (dwCurTick - cur_src->enterqueuetime>(90*60*1000)) && cur_src->GetRemoteQueueRank()>2000 && cur_src->GetRemoteQueueRank()>(GetAvgQr()*2+400) && (dwCurTick - lastpurgetime > 65000) ) 
						{
							UINT rqr=cur_src->GetRemoteQueueRank();
							if( !cur_src->SwapToAnotherFile( true , false, false , NULL ))
							{
								// Purge
								if(thePrefs.GetLogDrop())
									AddDebugLogLine(false, _T("-o- HQR(%u) dropped client: %s, %s, from %s"), rqr, cur_src->DbgGetFullClientSoftVer(), cur_src->GetUserName(), this->GetFileName());
								cur_src->droptime=dwCurTick;
								theApp.downloadqueue->RemoveSource(cur_src);
								lastpurgetime = dwCurTick;
								break;
							}
							else
							{
								if(thePrefs.GetLogA4AF())
									AddDebugLogLine(false, _T("-o- HQR(%u) swapped client: %s, %s, from %s to %s"),rqr, cur_src->DbgGetFullClientSoftVer(), cur_src->GetUserName(), this->GetFileName(), cur_src->GetRequestFile()->GetFileName());
								cur_src->DontSwapTo(this);
								lastpurgetime = dwCurTick;
								break;
							}
						}
					}

					// Maella -Unnecessary Protocol Overload-
					// Maella -Spread Request- (idea SlugFiller)					
					if(theApp.IsConnected() == true){
						// Check if a refresh is required for the download session with a cheap UDP
						if((!IsAutoDownPriority() || thePrefs.IsAcceptUpload()) &&// X: [RU] - [RefuseUpload]
							(cur_src->GetLastAskedTime() != 0) &&
							(cur_src->GetNextTCPAskedTime() > dwCurTick + (10*60000)) &&
							// 55 seconds for two attempts to refresh the download session with UDP
							(dwCurTick - cur_src->GetLastAskedTime() < cur_src->GetJitteredFileReaskTime() &&
							(dwCurTick - cur_src->GetLastAskedTime() > cur_src->GetJitteredFileReaskTime() - 55000))){

								// Send a OP_REASKFILEPING (UDP) to refresh the download session
								// The refresh of the download session is necessary to stay in the remote queue
								// => see CUpDownClient::SetLastUpRequest() and MAX_PURGEQUEUETIME
								cur_src->UDPReaskForDownload();
							}

					}
					// Maella end
				}
				case DS_CONNECTING:
				case DS_TOOMANYCONNS:
				case DS_TOOMANYCONNSKAD:
				case DS_NONE:
				case DS_WAITCALLBACK:
				case DS_WAITCALLBACKKAD:{
					// Maella -Spread Request- (idea SlugFiller)
					// Maella -Unnecessary Protocol Overload-
					if(theApp.IsConnected() == true){ 
						// Check if a refresh is required for the download session with TCP
						if((!IsAutoDownPriority() || thePrefs.IsAcceptUpload()) &&// X: [RU] - [RefuseUpload]
							((cur_src->GetLastAskedTime() == 0) || // Never asked before
							(cur_src->GetNextTCPAskedTime() <= dwCurTick) || // Full refresh with TCP is required
							(cur_src->socket != NULL && // Take advantage of the current connection
							cur_src->socket->IsConnected() == true && 
							//nächste TCP ist in weniger als 10 minuten
							(cur_src->GetNextTCPAskedTime() - (10*60000) < dwCurTick ||
							//Xman falls socket, dann darf der TCP-request immer stattfinden, wenn nächster request in 2 Minuten wäre
							//aber nicht falls wir noch auf UDP-Antwort warten
							(cur_src->UDPPacketPending()==false && (cur_src->GetLastAskedTime() + cur_src->GetJitteredFileReaskTime() - MIN2MS(2) < dwCurTick)) ||
							//client antwortet nicht auf UDP also schau nicht auf NextTCPAskedTime
							//sondern wann mu?das nächste mal abgefragt werden - 10 Minuten
							//Xman better using of existing connections
							((cur_src->HasTooManyFailedUDP() || cur_src->HasLowID()) && (cur_src->GetLastAskedTime() + cur_src->GetJitteredFileReaskTime() - (10*60000) < dwCurTick))) && 
							//letzte mal tcp-ask nach diesem file war vor 10 minuten:
							cur_src->GetLastFileAskedTime(this) + MIN_REQUESTTIME + 60000 < dwCurTick) || 
							//29 minuten seit dem lezten fragen(überhaupt) sind vorbei: UDP failed
							(dwCurTick - cur_src->GetLastAskedTime() > cur_src->GetJitteredFileReaskTime()))){ // Time since last refresh (UDP or TCP elapsed)

								// Initialize or refresh the download session with an expensive TCP session
								// The refresh of the download session is necessary to stay in the remote queue
								// => see CUpDownClient::SetLastUpRequest() and MAX_PURGEQUEUETIME
								if(!cur_src->AskForDownload()) // NOTE: This may *delete* the client!!
									break; //I left this break here just as a reminder just in case re rearange things..
						}
					}
					// Maella end
					break;	
				}
			}
		}

		
		//Xman Xtreme Downloadmanager: Auto-A4AF-check
		if (IsA4AFAuto() && ((!m_LastNoNeededCheck) || (dwCurTick - m_LastNoNeededCheck > (PURGESOURCESWAPSTOP + SEC2MS(5)))) )
		{
			m_LastNoNeededCheck = dwCurTick;

			for (POSITION pos = A4AFsrclist.GetHeadPosition(); pos !=NULL; ) {				
				CUpDownClient *cur_source = A4AFsrclist.GetNext(pos);
				if( cur_source->GetDownloadState() != DS_DOWNLOADING
					&& cur_source->GetRequestFile() 
					&& ( (!cur_source->GetRequestFile()->IsA4AFAuto()) || cur_source->GetDownloadState() == DS_NONEEDEDPARTS)
					&& !cur_source->IsSwapSuspended(this) 
					&& GetSourceCount()<GetMaxSources())	//Xman 4.2
				{
						CPartFile* oldfile = cur_source->GetRequestFile();
						if (cur_source->SwapToAnotherFile(false, false, false, this)){
							cur_source->DontSwapTo(oldfile);
						}
					}
			}
		}
		//Xman end
		

		if (downloadingbefore!=(m_anStates[DS_DOWNLOADING]>0))
			NotifyStatusChange();
 
		memcpy(m_anStates,m_anStatesTemp,sizeof(m_anStates)); // Source Counts Are Cached derivated from Khaos [SiRoB] - Stulle			
	 //faster ??? +
			  //if (GetMaxSourcePerFileUDP() > GetSourceCount()) 
		        if (GetMaxSources() > GetSourceCount()) 
	 //faster -
                {
	 //faster ??? +
              //if (theApp.downloadqueue->DoKademliaFileRequest() && (Kademlia::CKademlia::GetTotalFile() < KADEMLIATOTALFILE) && (dwCurTick > m_LastSearchTimeKad) &&  Kademlia::CKademlia::IsConnected() && theApp.IsConnected() && !stopped){ //Once we can handle lowID users in Kad, we remove the second IsConnected
			    if (theApp.downloadqueue->DoKademliaFileRequest() && (dwCurTick > m_LastSearchTimeKad) &&  Kademlia::CKademlia::IsConnected() && theApp.IsConnected() && !stopped){
	 //faster -
	            //Kademlia
				theApp.downloadqueue->SetLastKademliaFileRequest();
				if (!GetKadFileSearchID())
				{
					Kademlia::CSearch* pSearch = Kademlia::CSearchManager::PrepareLookup(Kademlia::CSearch::FILE, true, Kademlia::CUInt128(GetFileHash()));
					if (pSearch)
					{
						if(m_TotalSearchesKad < 7)
							m_TotalSearchesKad++;
						m_LastSearchTimeKad = dwCurTick + (KADEMLIAREASKTIME*m_TotalSearchesKad);
						SetKadFileSearchID(pSearch->GetSearchID());
						//Xman Code-Improvement: show filename immediately
						pSearch->SetGUIName(GetFileName());
						//Xman end
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


		if ( GetSrcStatisticsValue(DS_DOWNLOADING) != nOldTransSourceCount ){
			if (theApp.emuledlg->transferwnd->downloadlistctrl.curTab == 0)
				theApp.emuledlg->transferwnd->downloadlistctrl.ChangeCategory(0); 
			//else
				//UpdateDisplayedInfo(true);
			if (thePrefs.ShowCatTabInfos() )
				theApp.emuledlg->transferwnd->UpdateCatTabTitles();
		}
	}

	return receivedBlockTotal;
}
//Xman end //Maella end

bool CPartFile::CanAddSource(uint32 userid, uint16 port, uint32 serverip, uint16 serverport, uint_ptr* pdebug_lowiddropped, bool Ed2kID)
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
		if (pdebug_lowiddropped)
			(*pdebug_lowiddropped)++;
		return false;
	}
	// MOD Note - end
	return true;
}

void CPartFile::AddSources(CSafeMemFile* sources, uint32 serverip, uint16 serverport, bool bWithObfuscationAndHash)
{
	size_t count = sources->ReadUInt8();

	uint_ptr debug_lowiddropped = 0;
	uint_ptr debug_possiblesources = 0;
	uchar achUserHash[16];
	//bool bSkip = false; //Xman sourcecache
	bool stopKadSearch=false; //Xman sourcecache

	for (size_t i = 0; i < count; i++)
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
		if (stopped) //Xman sourcecache
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
					AddDebugLogLine(false, _T("Ignored source (IP=%s) received from server - IP filter"), ipstr(userid));
				continue;
			}
			if (theApp.clientlist->IsBannedClient(userid)){
#ifdef _DEBUG
				if (thePrefs.GetLogBannedClients()){
					CUpDownClient* pClient = theApp.clientlist->FindClientByIP(userid);
					AddDebugLogLine(false, _T("Ignored source (IP=%s) received from server - banned client %s"), ipstr(userid), pClient ? pClient->DbgGetClientInfo() : _T("unknown")); //Xman Code Fix
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
		//Xman filter out unreachable LowID-sources
		if(IsLowID(userid) )
		{
			if(serverip==0 || serverport==0)
			{
				AddDebugLogLine(false, _T("--> wrong source received (no server given) (AddSources)"));
				continue;
			}
			else if(!theApp.serverconnect->IsLocalServer(serverip,serverport))
			{
				//see baseclient->trytoconnect() we can't connect to this sources
				AddDebugLogLine(false, _T("--> wrong source received (not our server) (AddSources)"));
				continue;
			}
		}
		//Xman end

		//Xman surcecache
		if( GetMaxSources() > this->GetSourceCount()) 
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
			//bSkip = true; //Xman sourcecache
			if ((byCryptOptions & 0x80) != 0)
				AddToSourceCache(port,userid,serverip,serverport,SF_SERVER,true,achUserHash,byCryptOptions);
			else
				AddToSourceCache(port,userid,serverip,serverport,SF_SERVER,true,NULL,byCryptOptions);
			if(stopKadSearch==false && GetKadFileSearchID())
			{
				Kademlia::CSearchManager::StopSearch(GetKadFileSearchID(), false);
				stopKadSearch=true;
			}
		}
		//Xman end
	}
	if (thePrefs.GetDebugSourceExchange())
		AddDebugLogLine(false, _T("SXRecv: Server source response; Count=%u, Dropped=%u, PossibleSources=%u, File=\"%s\""), count, debug_lowiddropped, debug_possiblesources, GetFileName());
}

void CPartFile::UpdatePartsInfo()
{
	if( !IsPartFile() )
	{
		CKnownFile::UpdatePartsInfo();
		return;
	}

	// Cache part count
	size_t partcount = GetPartCount();
	bool flag = (time(NULL) - m_nCompleteSourcesTime > 0); 

	// Reset part counters
	if (m_SrcpartFrequency.GetCount() < partcount)
		m_SrcpartFrequency.SetCount(partcount);
	for (size_t i = 0; i < partcount; i++)
		m_SrcpartFrequency[i] = 0;
	
	CAtlArray<uint16> count;
	if (flag)
		count.SetCount(0, srclist.GetCount());
	if(!stopped){ // X-Ray :: Optimizations - No Need to Refresh a stopped File
		for (POSITION pos = srclist.GetHeadPosition(); pos != 0; ){
		CUpDownClient* cur_src = srclist.GetNext(pos);

		//Xman better chunk selection
		//use different weight
		uint16 weight=2;
			if(cur_src->GetDownloadState()==DS_ONQUEUE && (cur_src->IsRemoteQueueFull() || cur_src->GetRemoteQueueRank()>4000)) 			
                             weight=1;
		//Xman end

		if( cur_src->GetPartStatus() )
		{  
				for (size_t i = 0; i < partcount; i++)
			{
				if (cur_src->IsPartAvailable(i))
					m_SrcpartFrequency[i] = m_SrcpartFrequency[i] + (uint16)weight; //Xman better chunk selection
			}
			if ( flag )
			{
				//Xman Code Improvement
				//for complete sources the value isn't up to date btw. we don't have this value
				if(cur_src->HasFileComplete()==false && cur_src->m_abyUpPartStatus)
					count.Add(cur_src->GetUpCompleteSourcesCount());
				//Xman end
			}
		}
	}
	//Xman better chunk selection
	//at this point we have double weight -->reduce to normal (division by 2)
		for(uint_ptr i = 0; i < partcount; i++)
	{
		if(m_SrcpartFrequency[i]>1)
			m_SrcpartFrequency[i] = m_SrcpartFrequency[i]>>1; //nothing else than  m_SrcpartFrequency[i]/2; 
	}
	//Xman end
	} // X-Ray :: Optimizations - No Need to Refresh a stopped File

	if (flag)
	{
		// X-Ray :: Optimizations :: Start
		/*
		m_nCompleteSourcesCount = m_nCompleteSourcesCountLo = m_nCompleteSourcesCountHi = 0;
		*/
		m_nCompleteSourcesCount = m_SrcpartFrequency[0];
		// X-Ray :: Optimizations :: End
	
		for (uint_ptr i = 1; i < partcount; i++)
		{
			// X-Ray :: Optimizations :: Start
			/*
			if (!i)
				m_nCompleteSourcesCount = m_SrcpartFrequency[i];
			else if( m_nCompleteSourcesCount > m_SrcpartFrequency[i])
			*/
			if(m_nCompleteSourcesCount > m_SrcpartFrequency[i])
			// X-Ray :: Optimizations :: End
				m_nCompleteSourcesCount = m_SrcpartFrequency[i];
		}
		//Xman show virtual sources
		m_nVirtualCompleteSourcesCount=m_nCompleteSourcesCount;
		//Xman end

		count.Add(m_nCompleteSourcesCount);
	
		size_t n = count.GetCount();
		// X-Ray :: Optimizations :: Start
		// count can never be 0 because of the 'add' right above!
		/*
		if (n > 0)
		*/
		if(n < 5)
			m_nCompleteSourcesCountLo = m_nCompleteSourcesCountHi = m_nCompleteSourcesCount;
		else
		// X-Ray :: Optimizations :: End
		{
			// SLUGFILLER: heapsortCompletesrc
			INT_PTR r;
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
			INT_PTR i = n >> 1;			// (n / 2)
			INT_PTR j = (n * 3) >> 2;	// (n * 3) / 4
			INT_PTR k = (n * 7) >> 3;	// (n * 7) / 8

			//When still a part file, adjust your guesses by 20% to what you see..

			//Not many sources, so just use what you see..
			// X-Ray :: Optimizations :: Start
			/*
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
			// X-Ray :: Optimizations :: End
			{
				if ( count.GetAt(i) < m_nCompleteSourcesCount )
					m_nCompleteSourcesCountLo = m_nCompleteSourcesCount;
				else
					m_nCompleteSourcesCountLo = (uint16)((float)(count.GetAt(i)*.8)+(float)(m_nCompleteSourcesCount*.2));

				//m_nCompleteSourcesCount= m_nCompleteSourcesCountLo; //Xman Code Fix moved down
				m_nCompleteSourcesCountHi= (uint16)((float)(count.GetAt(j)*.8)+(float)(m_nCompleteSourcesCount*.2));
				m_nCompleteSourcesCount= m_nCompleteSourcesCountLo; //Xman Code Fix
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
				m_nCompleteSourcesCountHi= (uint16)((float)(count.GetAt(k)*.8)+(float)(m_nCompleteSourcesCountLo*.2)); //Xman Code Fix: must use m_nCompleteSourcesCountLo instead of m_nCompleteSourcesCount
				if( m_nCompleteSourcesCountHi < m_nCompleteSourcesCount )
					m_nCompleteSourcesCountHi = m_nCompleteSourcesCount;
			}
		}
		m_nCompleteSourcesTime = time(NULL) + (60);
	}
	UpdateDisplayedInfo();
}	

//Enig123::Optimizations
//MORPH START - Optimization
bool  CPartFile::RemoveBlockFromList(const Requested_Block_Struct* pblock)
{
	for (POSITION pos = requestedblocks_list.GetHeadPosition(); pos != NULL; ){
		POSITION posLast = pos;
		Requested_Block_Struct* block = requestedblocks_list.GetNext(pos);
		if (block == pblock){
			requestedblocks_list.RemoveAt(posLast);
			return true;
		}
	}
	return false;
}
//MORPH END  - Optimization

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

	theApp.emuledlg->transferwnd->downloadclientsctrl.Reload(this, false);

	if (!bIsHashingDone){
		//Xman
		// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
		m_nDownDatarate = 0;
		m_nDownDatarate10 = 0;
		// Maella end

		SetStatus(PS_COMPLETING);
		//datarate = 0;
			TCHAR mytemppath[MAX_PATH];
			_tcscpy_s(mytemppath,m_fullname);
			mytemppath[ _tcslen(mytemppath)-_tcslen(m_partmetfilename)-1]=0;
		CAddFileThread* addfilethread = new CAddFileThread(NULL, mytemppath, RemoveFileExtension(m_partmetfilename), _T(""), this);
		if(addfilethread->start())
		{
			SetFileOp(PFOP_HASHING);
			SetFileOpProgress(0);
		}
		else
		{
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_FILECOMPLETIONTHREAD));
			SetStatus(PS_ERROR);
		}
		return;
	}
	StopFile();
	SetStatus(PS_COMPLETING);
	m_is_A4AF_auto=false; //Xman Xtreme Downloadmanager: Auto-A4AF-check
	CompleteThread *pThread = new CompleteThread(this); // Lord KiRon - using threads for file completion
	if(pThread->start())
	{
	SetFileOp(PFOP_COPYING);
	SetFileOpProgress(0);
	theApp.emuledlg->transferwnd->UpdateFilesCount();
	if (thePrefs.ShowCatTabInfos())
		theApp.emuledlg->transferwnd->UpdateCatTabTitles();
	if(theApp.emuledlg->activewnd == theApp.emuledlg->transferwnd){
	UpdateDisplayedInfo(true);
		theApp.emuledlg->transferwnd->partstatusctrl.Refresh(this);
           }
        } 
           else
	{
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_FILECOMPLETIONTHREAD));
		SetStatus(PS_ERROR);
	}
}

void CPartFile::CompleteThread::run() 
{ 
	DbgSetThreadName("PartFileComplete");
	//Xman
	// BEGIN SLUGFILLER: SafeHash
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return;
	// END SLUGFILLER: SafeHash
	InitThreadLocale();
	if (!pFile)
		return; 
	CoInitialize(NULL);
   	pFile->PerformFileComplete();
	CoUninitialize();
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
	if (!thePrefs.GetCheckFileOpen() || !g_VolumeInfo.GetVolumeInfoByPath(pszFilePath)->IsSupportNamedStream()) // X: [FSFS] - [FileSystemFeaturesSupport]
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
				VERIFY( SUCCEEDED(pPersistFile->Save(pszFilePath, FALSE)) );
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
	if (TotalFileSize.QuadPart && pPartFile && IsCPartFile(pPartFile)/*pPartFile->IsKindOf(RUNTIME_CLASS(CPartFile))*/)
	{
		uint_ptr uProgress = (uint_ptr)(TotalBytesTransferred.QuadPart * 100 / TotalFileSize.QuadPart);
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
	// X-Ray :: NiceMove :: Start
	if(g_VolumeInfo.GetVolumeInfoByPath(pszPartFilePath)->Name != g_VolumeInfo.GetVolumeInfoByPath(pszNewPartFilePath)->Name) // X: [FSFS] - [FileSystemFeaturesSupport]
	{
		//Open the source file
		HANDLE source = CreateFile(pszPartFilePath,  // file name 
			GENERIC_READ,				                       // open for read/write 
			0,                                         // do not share 
			NULL,                                      // default security 
			OPEN_EXISTING,			                       // open only
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, // normal file 
			NULL);                                     // no template     
		if (source == INVALID_HANDLE_VALUE) 
			return ERROR_INVALID_HANDLE;

		//Create our destination
		HANDLE destination = CreateFile(pszNewPartFilePath,  // file name 
			GENERIC_WRITE,				                             // open for read/write 
			0,                                                 // do not share 
			NULL,                                              // default security 
			CREATE_NEW,			                                   // don't overwrite existing file
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, // normal file 
			NULL);                                             // no template 
		if (destination == INVALID_HANDLE_VALUE) 
			return ERROR_INVALID_HANDLE;

#define BUFFERSIZE	(128*1024) //128kB

		//Read blocks to the buffer && write the buffer to the destination file
		DWORD	dwBytesRead = 0;
		DWORD	dwBytesWritten = 0;
		BYTE	buffer[BUFFERSIZE];
		uint64	bytesCopied = 0;

		//Xman Nice Hash
		uint32 timeStart = ::GetTickCount();  
		//Xman end

		do 
		{
			//try to read
			if (ReadFile(source, buffer, BUFFERSIZE, &dwBytesRead, NULL) == NULL)
			{
				DWORD dwMoveResult = GetLastError();

				//close both handles
				CloseHandle(destination); 
				CloseHandle(source);

				//try to remove the partial file
				DeleteFile(pszNewPartFilePath);

				return dwMoveResult;
			}

			//try to write
			if(WriteFile(destination, buffer, dwBytesRead, &dwBytesWritten, NULL) == NULL) 
			{
				DWORD dwMoveResult = GetLastError();

				//close both handles
				CloseHandle(destination); 
				CloseHandle(source);

				//try to remove the partial file
				DeleteFile(pszNewPartFilePath);

				return dwMoveResult;
			}

			bytesCopied += dwBytesWritten;
			uint_ptr uProgress = (uint_ptr)(bytesCopied * 100 / (uint64)pPartFile->GetFileSize());
			if (uProgress != pPartFile->GetFileOpProgress())
			{
				ASSERT( uProgress <= 100 );
				VERIFY( PostMessage(theApp.emuledlg->GetSafeHwnd(), TM_FILEOPPROGRESS, uProgress, (LPARAM)pPartFile) );
			}

			// X-Ray :: NiceHash :: Start
			//Stack up time and sleep if above limit to reduce HDD trashing
			if((::GetTickCount() - timeStart) >= 100){
				Sleep(10);
				timeStart = ::GetTickCount();
			}
			// X-Ray :: NiceHash :: End

		} while (dwBytesRead == BUFFERSIZE); 

		//close both handles
		CloseHandle(destination); 
		CloseHandle(source);

		//try to remove source file
		if(DeleteFile(pszPartFilePath) == NULL)
			return GetLastError();

		return ERROR_SUCCESS;
	}
	// X-Ray :: NiceMove :: End

	if (MoveFileWithProgress(pszPartFilePath, pszNewPartFilePath, CopyProgressRoutine, pPartFile, MOVEFILE_COPY_ALLOWED))
		return ERROR_SUCCESS;
	return GetLastError();
}

// Lord KiRon - using threads for file completion
// NOTE: This function is executed within a seperate thread, do *NOT* use any lists/queues of the main thread without
// synchronization. Even the access to couple of members of the CPartFile (e.g. filename) would need to be properly
// synchronization to achive full multi threading compliance.
BOOL CPartFile::PerformFileComplete() 
{
	// If that function is invoked from within the file completion thread, it's ok if we wait (and block) the thread.
	Poco::FastMutex::SingleLock sLock(m_FileCompleteMutex, true);

	CString strPartfilename(RemoveFileExtension(m_fullname));
	TCHAR* newfilename = _tcsdup(GetFileName());
	_tcscpy(newfilename, (LPCTSTR)StripInvalidFilenameChars(newfilename));

	CString indir = thePrefs.GetCategory(GetCategory())->strIncomingPath;
	if(!PathFileExists(indir) && !CreateDirectory(indir, 0)) // X: [BF] - [Bug Fix] if not exist create one
		indir = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
	CString strNewname;
	strNewname.Format(_T("%s\\%s"), indir, newfilename);

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
		uint_ptr namecount = 0;

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
			strTestName.Format(_T("%s\\%s(%u).%s"), indir, newfilename, namecount, min(ext + 1, newfilename + length));
		}
		while (PathFileExists(strTestName));
		strNewname = strTestName;
	}
	free(newfilename);
	// Xman -New Save/load Sources- enkeyDEV(Ottavio84)
	m_sourcesaver.DeleteFile();
	// Xman end
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

		//MORPH START - Added by WiZaRd, FiX!
		// explicitly unlock the file before posting something to the main thread.
		sLock.Unlock();
		//MORPH END   - Added by WiZaRd, FiX!

		if (theApp.emuledlg && CemuleDlg::IsRunning())
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
	struct _stat64 st;
	if (_tstat64(strNewname, &st) == 0)
	{
		m_tLastModified = st.st_mtime;
		m_tUtcLastModified = m_tLastModified;
		AdjustNTFSDaylightFileTime(m_tUtcLastModified, strNewname);
	}
	else if (thePrefs.GetVerbose())
		AddDebugLogLine(false, _T("Failed to get file date of \"%s\""), strNewname);

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

	if (theApp.emuledlg && CemuleDlg::IsRunning())
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
		//Xman [MoNKi: -Downloaded History-] //Xman moved here
		if(thePrefs.m_bHistoryShowShared && !thePrefs.IsHistoryListDisabled() && theApp.emuledlg && CemuleDlg::IsRunning()){
			theApp.emuledlg->transferwnd->historylistctrl.AddFile(this);
			theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2History);
		}
		//Xman end

		//Xman sourcecache
		ClearSourceCache();
		//Xman end
		theApp.knownfiles->SafeAddKFile(this);
		theApp.downloadqueue->RemoveFile(this);
		theApp.emuledlg->transferwnd->UpdateFilesCount();

		thePrefs.Add2DownCompletedFiles();
		thePrefs.Add2DownSessionCompletedFiles();
		thePrefs.SaveCompletedDownloadsStat();

		// republish that file to the ed2k-server to update the 'FT_COMPLETE_SOURCES' counter on the server.
		theApp.sharedfiles->RepublishFile(this);

		//Xman x4.1 to be sure
		// Maella -Extended clean-up II-
		theApp.clientlist->CleanUp(this);
		// Maella end

		// give visual response
		Log(LOG_SUCCESS | LOG_STATUSBAR | LOG_DONTNOTIFY, GetResString(IDS_DOWNLOADDONE), GetFileName());
		if(xState != PFS_HASHING && thePrefs.GetNotifier())// X: [POFC] - [PauseOnFileComplete]
			theApp.emuledlg->TraySetBalloonToolTip(GetResString(IDS_CL_DOWNLSTATUS), GetResString(IDS_TBN_DOWNLOADDONE) + _T('\n') + GetFileName());
		if (dwResult & FILE_COMPLETION_THREAD_RENAMED)
		{
			CString strFilePath(GetFullName());
			PathStripPath(strFilePath.GetBuffer());
			strFilePath.ReleaseBuffer();
			Log(LOG_STATUSBAR, GetResString(IDS_DOWNLOADRENAMED), strFilePath);
		}
		if(!m_pCollection && HasCollectionExtenesion_Xtreme() /*CCollection::HasCollectionExtention(GetFileName())*/) //Xman Code Improvement for HasCollectionExtention
		//if(!m_pCollection && CCollection::HasCollectionExtention(GetFileName()))
		{
			m_pCollection = new CCollection();
			if(!m_pCollection->InitCollectionFromFile(GetFilePath(), GetFileName()))
			{
				delete m_pCollection;
				m_pCollection = NULL;
			}
		}
		//Xman advanced upload-priority
		//don't wait for the next request, update now!
		m_nCompleteSourcesTime=0;
		UpdatePartsInfo(); 
		//Xman end
	}
    if(xState!=PFS_HASHING)// X: [POFC] - [PauseOnFileComplete]
		theApp.downloadqueue->StartNextFileIfPrefs(GetCategory());
	xState=PFS_NORMAL;

//morph4u shutdown +
	int CountDlFiles = 0;
	for(POSITION pos = theApp.downloadqueue->filelist.GetHeadPosition(); pos != NULL;) 
	{ 
		CPartFile* cur_file = theApp.downloadqueue->filelist.GetNext(pos); 
		if(cur_file->GetStatus() != PS_COMPLETE )
			CountDlFiles ++;
	}

	if (thePrefs.m_bShutDownPC && CountDlFiles == 0)
	{
		theApp.emuledlg->SaveSettings(true); 
		MySystemShutdown();
	}
	else if (thePrefs.m_bShutDownMule && CountDlFiles == 0)
		theApp.emuledlg->OnClose();	
//morph4u shutdown -

   }

void  CPartFile::RemoveAllSources(bool bTryToSwap){
	theApp.emuledlg->transferwnd->downloadclientsctrl.Reload(this, false);
	//Xman Xtreme Downloadmanager
	for(POSITION pos = srclist.GetHeadPosition(); pos != NULL; ){
		CUpDownClient* cur_src = srclist.GetNext(pos);
		if (!bTryToSwap|| !cur_src->SwapToAnotherFile(true, true, true, NULL, false, true) ) //Xman x4.1 allow go over hardlimit at this case// X: [CI] - [Code Improvement]
			theApp.downloadqueue->RemoveSource(cur_src, false);
	}
	//Xman end
	UpdateAvailablePartsCount();
	UpdatePartsInfo(); 

	//[enkeyDEV(Ottavio84) -A4AF-]
	// remove all links A4AF in sources to this file
	if(!A4AFsrclist.IsEmpty())
	{
		for(POSITION pos1 = A4AFsrclist.GetHeadPosition();pos1 != NULL;){// X: [CI] - [Code Improvement]
			CUpDownClient* client = A4AFsrclist.GetNext(pos1);
			
			POSITION pos3 = client->m_OtherRequests_list.Find(this); 
			if(pos3)
				client->m_OtherRequests_list.RemoveAt(pos3);
			else{
				pos3 = client->m_OtherNoNeeded_list.Find(this); 
				if(pos3)
					client->m_OtherNoNeeded_list.RemoveAt(pos3);
			}
		}
		A4AFsrclist.RemoveAll();
	}
}

void CPartFile::DeleteFile(){
	ASSERT ( !m_bPreviewing );

	// Barry - Need to tell any connected clients to stop sending the file
	StopFile(true);

	// feel free to implement a runtime handling mechanism!
	if (m_AllocateThread != NULL){
		LogWarning(LOG_STATUSBAR, GetResString(IDS_DELETEAFTERALLOC), GetFileName());
		m_bDeleteAfterAlloc=true;
		return;
	}

	theApp.sharedfiles->RemoveFile(this, true);
	theApp.downloadqueue->RemoveFile(this);
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

	// Xman -New Save/load Sources- enkeyDEV(Ottavio84)
	m_sourcesaver.DeleteFile();
	// Xman end

	//Xman sourcecache
	ClearSourceCache();
	//Xman end

	BAKName = m_fullname;
	BAKName.Append(PARTMET_TMP_EXT);
	if (_taccess(BAKName, 0) == 0 && !::DeleteFile(BAKName))
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_DELETE) + _T(" - ") + GetErrorMessage(GetLastError()), BAKName);

	delete this;
}

//Xman <5.4.1
// SLUGFILLER: SafeHash remove - removed HashSinglePart completely.
//Xman 5.4.1
//Xman readded this method
//Xman Flush Thread/Safehash: need the mono threaded hashing when shutting down
//otherwise the partfile must be rehashed on next startup
bool CPartFile::HashSinglePart(uint_ptr partnumber, bool* pbAICHReportedOK)
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
		uint_ptr length = PARTSIZE;
		if ((ULONGLONG)PARTSIZE*(uint64)(partnumber+1) > m_hpartfile.GetLength()){
			length = (uint_ptr)(m_hpartfile.GetLength() - ((ULONGLONG)PARTSIZE*(uint64)partnumber));
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
//Xman end

bool CPartFile::IsCorruptedPart(uint_ptr partnumber) const
{
	return (corrupted_list.Find((uint16)partnumber) != NULL);
}

bool CPartFile::IsPreviewableFileType() const {
    return IsMovie();
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
			//theApp.downloadqueue->SortByPriority();
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
	// Barry - Need to tell any connected clients to stop sending the file
	PauseFile(false, resort);
	m_LastSearchTimeKad = 0;
	m_TotalSearchesKad = 0;
	if(!bCancel) // X: [ISS] - [Improved Source Save]
		m_sourcesaver.SaveSources();
	// X-Ray :: Optimizations :: Start - No Need to Refresh a stopped File
	// moved down
	/*
	RemoveAllSources(true);
	*/
	// X-Ray :: Optimizations :: End - No Need to Refresh a stopped File
	paused = true;
	stopped = true;
	insufficient = false;
	//Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	m_nDownDatarate = 0;
	m_nDownDatarate10 = 0;
	// Maella end
	//datarate = 0;
	RemoveAllSources(true); // X-Ray :: Optimizations - No Need to Refresh a stopped File
	//Xman sourcecache
	ClearSourceCache(); //only to avoid holding *maybe* not usful data in memory 
	//Xman end
	memset(m_anStates,0,sizeof(m_anStates));
	memset(src_stats,0,sizeof(src_stats)); //Xman Bugfix
	memset(net_stats,0,sizeof(net_stats)); //Xman Bugfix

	if (!bCancel)
		FlushBuffer(true);
	if(xState == PART_CORRUPTED_AICH) // X: [IPR] - [Improved Part Recovery]
		xState = PART_CORRUPTED;
    if(resort) {
	    theApp.downloadqueue->SortByPriority();
	    theApp.downloadqueue->CheckDiskspace();
    }
	if(theApp.emuledlg->activewnd == theApp.emuledlg->transferwnd){
	UpdateDisplayedInfo(true);
		theApp.emuledlg->transferwnd->partstatusctrl.Refresh(this);
	}
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

	for( POSITION pos = srclist.GetHeadPosition(); pos != NULL; )
	{
		CUpDownClient* cur_src = srclist.GetNext(pos);
		if (cur_src->GetDownloadState() == DS_DOWNLOADING)
		{
			cur_src->SendCancelTransfer();
			cur_src->SetDownloadState(DS_ONQUEUE, _T("You cancelled the download. Sending OP_CANCELTRANSFER"), CUpDownClient::DSR_PAUSED); // Maella -Download Stop Reason-
		}
	}

	if (bInsufficient)
	{
		//LogError(LOG_STATUSBAR, _T("Insufficient diskspace - pausing download of \"%s\""), GetFileName());
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_OUTOFSPACE), GetFileName());
		insufficient = true;
	}
	else
	{
		paused = true;
		insufficient = false;
	}
	//Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	m_nDownDatarate = 0;
	m_nDownDatarate10 = 0;
	// Maella end

	m_anStates[DS_DOWNLOADING] = 0; // -khaos--+++> Renamed var.
	if (!bInsufficient)
	{
        if(resort) {
		    theApp.downloadqueue->SortByPriority();
		    theApp.downloadqueue->CheckDiskspace(); // SLUGFILLER: checkDiskspace
        }
		SavePartFile();
	}
	NotifyStatusChange();
}

bool CPartFile::CanResumeFile() const
{
	return (GetStatus()==PS_PAUSED || GetStatus()==PS_INSUFFICIENT || (GetStatus()==PS_ERROR && GetCompletionError()) || xState == POFC_WAITING);// X: [POFC] - [PauseOnFileComplete]
}

void CPartFile::ResumeFile(bool resort)
{
	if (status==PS_COMPLETE || status==PS_COMPLETING)
		return;
	if (status==PS_ERROR && m_bCompletionError || xState == POFC_WAITING){// X: [POFC] - [PauseOnFileComplete]
		ASSERT( gaplist.IsEmpty() );
		if (gaplist.IsEmpty() && !m_nTotalBufferData && !m_FlushSetting) { //Xman - MORPH - Changed by SiRoB, Flush Thread
		//if (gaplist.IsEmpty()){
			// rehashing the file could probably be avoided, but better be in the safe side..
			m_bCompletionError = false;
			xState = PFS_HASHING;
			CompleteFile(false);
		}
		return;
	}
	paused = false;
	if(stopped){
	stopped = false;
		m_sourcesaver.AddSourcesToDownload(); // X: [ISS] - [Improved Source Save]
	}
	SetActive(theApp.IsConnected());
	m_LastSearchTime = 0;
    if(resort) {
	    theApp.downloadqueue->SortByPriority();
	    theApp.downloadqueue->CheckDiskspace();
    }
	SavePartFile();
	NotifyStatusChange();
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
	if(theApp.emuledlg->activewnd == theApp.emuledlg->transferwnd){
	UpdateDisplayedInfo(true);
		theApp.emuledlg->transferwnd->partstatusctrl.Refresh(this);
	}
}

CString CPartFile::getPartfileStatus() const
{
	switch(GetStatus()){
		//case PS_HASHING:
		//case PS_WAITINGFORHASH:
			//return GetResString(IDS_HASHING);

		case PS_COMPLETING:{
			CString strState = GetResString(IDS_COMPLETING);
			if (GetFileOp() == PFOP_HASHING)
				strState += _T(" (") + GetResString(IDS_HASHING) + _T(')');
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
	if(xState == IP_WAITING_AICH)// X: [IP] - [Import Parts]
		return _CString(_T("Requesting hashset"));
	return GetResString(IDS_WAITING);
} 

size_t CPartFile::getPartfileStatusRang() const
{
	switch (GetStatus()) {
		//case PS_HASHING: 
		//case PS_WAITINGFORHASH:
			//return 7;

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

//Xman
// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
uint_ptr CPartFile::getTimeRemainingSimple() const
{
	if (GetDownloadDatarate10() == 0)
		return (uint_ptr)-1;
	return (uint_ptr)(leftsize / (uint64)GetDownloadDatarate10());
}

uint_ptr CPartFile::getTimeRemaining() const
{
	uint_ptr simple = (uint_ptr)-1;
	uint_ptr estimate = (uint_ptr)-1;
	if( GetDownloadDatarate10() > 0 )
	{
		simple = (uint_ptr)(leftsize / (uint64)GetDownloadDatarate10());
	}
	if( GetDlActiveTime() && completedsize >= (uint64)512000 )
		estimate = (uint_ptr)(leftsize / ((double)completedsize / (double)GetDlActiveTime()));

	if( simple == (uint_ptr)-1 )
	{
		//We are not transferring at the moment.
		if( estimate == (uint_ptr)-1 )
			//We also don't have enough data to guess
			return (uint_ptr)-1;
		else if( estimate > HR2S(24*15) )
			//The estimate is too high
			return (uint_ptr)-1;
		else
			return estimate;
	}
	else if( estimate == (uint_ptr)-1 )
	{
		//We are transferring but estimate doesn't have enough data to guess
		return simple;
	}
	if( simple < estimate )
		return simple;
	if( estimate > HR2S(24*15) )
		//The estimate is too high..
		return (uint_ptr)-1;
	return estimate;
}
//Xman end

void CPartFile::PreviewFile()
{
	if (thePreviewApps.Preview(this))
		return;

	if (!IsReadyForPreview()){
		ASSERT( false );
		return;
	}

	if (thePrefs.IsMoviePreviewBackup()){		
		m_bPreviewing = true;
		(new CPreviewThread(this, thePrefs.GetVideoPlayer(), thePrefs.GetVideoPlayerArgs()))->start();
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

	if (thePrefs.IsMoviePreviewBackup())
	{
		return !( (GetStatus() != PS_READY && GetStatus() != PS_PAUSED) 
			|| m_bPreviewing || GetPartCount() < 5 || !IsMovie() || (CVolumeInfo::_GetFreeDiskSpace(GetTempPath()) + 100000000) < GetFileSize()
			|| ( headerSize<PARTSIZE || !IsComplete(PARTSIZE*(uint64)(GetPartCount()-1),GetFileSize() - (uint64)1, false)));
	}
	else
	{
		TCHAR szVideoPlayerFileName[_MAX_FNAME];
		_tsplitpath_s(thePrefs.GetVideoPlayer(), NULL, 0, NULL, 0, szVideoPlayerFileName, _countof(szVideoPlayerFileName), NULL, 0);

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
					LPCTSTR rstrED2KFileType = GetStrTagValue(FT_FILETYPE);
					if (rstrED2KFileType[0] == 0 || !(!_tcscmp(rstrED2KFileType, _T(ED2KFTSTR_AUDIO)) || !_tcscmp(rstrED2KFileType, _T(ED2KFTSTR_VIDEO))))
						return false;
				}
			}

			// If it's an MPEG file, VLC is even capable of showing parts of the file if the beginning of the file is missing!
			/*bool bMPEG = false;
			LPCTSTR pszExt = _tcsrchr(GetFileName(), _T('.'));
			if (pszExt != NULL){
				CString strExt(pszExt);
				strExt.MakeLower();
				bMPEG = (strExt==_T(".mpg") || strExt==_T(".mpeg") || strExt==_T(".mpe") || strExt==_T(".mp3") || strExt==_T(".mp2") || strExt==_T(".mpa"));
			}

			if (bMPEG){*/
			const TCHAR* ext = _tcsrchr(GetFileName(),_T('.'));
			if (ext != NULL && extInList(_T("mpg|mpeg|mpe|mp3|mp2|mpa"),ext+1) != NULL){// X: [CI] - [Code Improvement]
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
					if (headerSize<=256*1024)
						return false;
				}
			}

			return true;
		}
		else{
			return !((GetStatus() != PS_READY && GetStatus() != PS_PAUSED) 
				|| m_bPreviewing || GetPartCount() < 2 || !IsMovie() || headerSize<PARTSIZE); 
		}
	}
}

void CPartFile::UpdateAvailablePartsCount()
{
	UINT availablecounter = 0;
	uint_ptr iPartCount = GetPartCount();
	for (uint_ptr ixPart = 0; ixPart < iPartCount; ixPart++){
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
	//Xman remark:
	//we can exchange the sources of a paused file, if they become to old, the file will be stopped automatically
	//a stopped file has no sources and we exchange the sources of our uploading list
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
	//Xman Code Improvement
	//check for empty scrlist sufficient
	/* 
	if (!(GetStatus() == PS_READY || GetStatus() == PS_EMPTY))
		return NULL;
	*/
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

	uint_ptr nCount = 0;
	data.WriteHash16(m_FileIdentifier.GetMD4Hash());
	data.WriteUInt16((uint16)nCount);

	//Xman Code Improvement
	const size_t scount=GetSourceCount();

	bool bNeeded;
	const uint8* reqstatus = forClient->GetUpPartStatus();
	for (POSITION pos = srclist.GetHeadPosition();pos != 0;){
		bNeeded = false;
		const CUpDownClient* cur_src = srclist.GetNext(pos);
		if (cur_src->HasLowID() || !cur_src->IsValidSource())
			continue;
		if (scount >=100 && cur_src->IsRemoteQueueFull() && nCount>=5) //at least 5 sources
			continue;
		//Xman end
		const uint8* srcstatus = cur_src->GetPartStatus();
		if (srcstatus){
			if (cur_src->GetPartCount() == GetPartCount()){
				if (reqstatus){
					ASSERT( forClient->GetUpPartCount() == GetPartCount() );
					// only send sources which have needed parts for this client
					for (uint_ptr x = 0; x < GetPartCount(); x++){
						if (srcstatus[x] && !reqstatus[x]){
							bNeeded = true;
							break;
						}
					}
				}
				else{
					// We know this client is valid. But don't know the part count status.. So, currently we just send them.
					for (uint_ptr x = 0; x < GetPartCount(); x++){
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
				const uint8 uRequestsCryptLayer	= cur_src->RequestsCryptLayer() ? 1 : 0;
				const uint8 uRequiresCryptLayer	= cur_src->RequiresCryptLayer() ? 1 : 0;
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

	Packet* result = new Packet(&data, OP_EMULEPROT, bIsSX2Packet ? OP_ANSWERSOURCES2 : OP_ANSWERSOURCES);
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

	size_t nCount = 0;

	if (thePrefs.GetDebugSourceExchange()) {
		CString strDbgClientInfo;
		if (pClient)
			strDbgClientInfo.Format(_T("%s, "), pClient->DbgGetClientInfo());
		AddDebugLogLine(false, _T("SXRecv: Client source response; SX2=%s, Ver=%u, %sFile=\"%s\""), bSourceExchange2 ? _T("Yes") : _T("No"), uClientSXVersion, strDbgClientInfo, GetFileName());
	}

	size_t uPacketSXVersion = 0;
	if (!bSourceExchange2){
		// for SX1 (deprecated):
		// Check if the data size matches the 'nCount' for v1 or v2 and eventually correct the source
		// exchange version while reading the packet data. Otherwise we could experience a higher
		// chance in dealing with wrong source data, userhashs and finally duplicate sources.
		nCount = sources->ReadUInt16();
		size_t uDataSize = (size_t)(sources->GetLength() - sources->GetPosition());
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
		size_t uDataSize = (size_t)(sources->GetLength() - sources->GetPosition());	
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

	for (size_t i = 0; i < nCount; i++)
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
		uint32 dwIDED2K = (uPacketSXVersion >= 3)?ntohl(dwID):dwID;// X: [CI] - [Code Improvement]
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
					AddDebugLogLine(false, _T("Ignored source (IP=%s) received via source exchange - IP filter for file: %s"), ipstr(dwIDED2K), GetFileName()); //Xman show filename
				continue;
			}
			if (theApp.clientlist->IsBannedClient(dwIDED2K)){
#ifdef _DEBUG
				if (thePrefs.GetLogBannedClients()){
					CUpDownClient* pClient = theApp.clientlist->FindClientByIP(dwIDED2K);
					AddDebugLogLine(false, _T("Ignored source (IP=%s) received via source exchange - banned client %s"), ipstr(dwIDED2K), pClient ? pClient->DbgGetClientInfo() : _T("unknown")); //Xman code fix
				}
#endif
				continue;
			}
		}

		// additionally check for LowID and own IP
		if (!CanAddSource(dwID, nPort, dwServerIP, nServerPort, NULL, uPacketSXVersion < 3))
		{
			//if (thePrefs.GetLogFilteredIPs())
			//	AddDebugLogLine(false, _T("Ignored source (IP=%s) received via source exchange"), ipstr(dwIDED2K));
			continue;
		}

		//Xman fix: filter sources we can't connect to
		if(IsLowID(dwID) && dwServerIP==0)// unuseful source
		{
			AddDebugLogLine(false, _T("--> wrong source received from %s"), pClient->DbgGetClientInfo());
			continue;
		}
		//Xman end

		if (GetMaxSources() > GetSourceCount()) 
		{
			CUpDownClient* newsource = new CUpDownClient(this, nPort, dwID, dwServerIP, nServerPort, uPacketSXVersion < 3);// X: [CI] - [Code Improvement]
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
		{
			//Xman source cache
			if(uPacketSXVersion>=4)
				AddToSourceCache(nPort,dwID,dwServerIP,nServerPort,SF_SOURCE_EXCHANGE,false,achUserHash, byCryptOptions);
			else if(uPacketSXVersion>=3)
				AddToSourceCache(nPort,dwID,dwServerIP,nServerPort,SF_SOURCE_EXCHANGE,false,achUserHash);
			else if(uPacketSXVersion>=2)
				AddToSourceCache(nPort,dwID,dwServerIP,nServerPort,SF_SOURCE_EXCHANGE,true,achUserHash);
			else
				AddToSourceCache(nPort,dwID,dwServerIP,nServerPort,SF_SOURCE_EXCHANGE,true);
			//Xman end
		}
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
	m_uTransferred += transize;

	// This is needed a few times
	uint32 lenData = (uint32)(end - start + 1);
	ASSERT( (int)lenData > 0 && (uint64)(end - start + 1) == lenData);

	if (lenData > transize) {
		m_uCompressionGain += lenData - transize;
		thePrefs.Add2SavedFromCompression(lenData - transize);
	}

	// Occasionally packets are duplicated, no point writing it twice
	/*// X: [CI] - [Code Improvement]
	if (IsComplete(start, end, false))
	{
		if (thePrefs.GetVerbose())
			AddDebugLogLine(false, _T("PrcBlkPkt: Already written block %s; File=%s; %s"), DbgGetBlockInfo(start, end), GetFileName(), client->DbgGetClientInfo());
		return 0;
	}
	*/
	// security sanitize check to make sure we do not write anything into an already hashed complete chunk
	const uint64 nStartChunk = start / PARTSIZE;
	const uint64 nEndChunk = end / PARTSIZE;
	//if (IsComplete(PARTSIZE * (uint64)nStartChunk, (PARTSIZE * (uint64)(nStartChunk + 1)) - 1, false)){
	if (IsComplete(start, (PARTSIZE * (uint64)(nStartChunk + 1)) - 1, false)){// X: [CI] - [Code Improvement]
		//DebugLogError( _T("PrcBlkPkt: Received data touches already hashed chunk - ignored (start) %s; File=%s; %s"), DbgGetBlockInfo(start, end), GetFileName(), client->DbgGetClientInfo());
		if (thePrefs.GetVerbose())
			AddDebugLogLine(false, _T("PrcBlkPkt: Already written block %s; File=%s; %s"), DbgGetBlockInfo(start, end), GetFileName(), client->DbgGetClientInfo());
		return 0;
	}
	else if (nStartChunk != nEndChunk) {
		//if (IsComplete(PARTSIZE * (uint64)nEndChunk, (PARTSIZE * (uint64)(nEndChunk + 1)) - 1, false)){
		if (IsComplete(PARTSIZE * (uint64)nEndChunk, end, false)){// X: [CI] - [Code Improvement]
			//DebugLogError( _T("PrcBlkPkt: Received data touches already hashed chunk - ignored (end) %s; File=%s; %s"), DbgGetBlockInfo(start, end), GetFileName(), client->DbgGetClientInfo());
			if (thePrefs.GetVerbose())
				AddDebugLogLine(false, _T("PrcBlkPkt: Already written block %s; File=%s; %s"), DbgGetBlockInfo(start, end), GetFileName(), client->DbgGetClientInfo());
			return 0;
		}
		else
			DEBUG_ONLY( DebugLogWarning(_T("PrcBlkPkt: Received data crosses chunk boundaries %s; File=%s; %s"), DbgGetBlockInfo(start, end), GetFileName(), client->DbgGetClientInfo()) );
	}

	//Xman
	// BEGIN SLUGFILLER: SafeHash
	Poco::FastMutex::SingleLock sLock(ICH_mut, true);	// Wait for ICH result
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

		lenData += lenDataClipped;	// calculate actual added data
	}
	// END SLUGFILLER: SafeHash
	// Increment buffer size marker
	m_nTotalBufferData += lenData;

	//Xman
	FillGap(start, end);	// SLUGFILLER: SafeHash - clean coding, removed "item->"

	datareceived = true;// X: [GB] - [Global Buffer]

	// Update the flushed mark on the requested block 
	// The loop here is unfortunate but necessary to detect deleted blocks.
	//Xman
	POSITION pos = requestedblocks_list.GetHeadPosition();	// SLUGFILLER: SafeHash
	while (pos != NULL)
	{	
		// BEGIN SLUGFILLER: SafeHash - clean coding, removed "item->"
		if (requestedblocks_list.GetNext(pos) == block){
			block->transferred += lenData;
			break; //MORPH - Optimization
		}
		// END SLUGFILLER: SafeHash
	}

	if (gaplist.IsEmpty())
		FlushBuffer(true);
	// Return the length of data written to the buffer
	return lenData;
}

//Xman
// BEGIN SiRoB: Flush Thread
void CPartFile::FlushBuffer(bool forcewait, bool bForceICH, bool bOnlyCompletedPart)// X: [GB] - [Global Buffer]
{
	//MORPH START - Added by SiRoB, Flush Thread
	if (forcewait) { //We need to wait for flush thread to terminate
		if (m_FlushSetting) { //We are flushing something to disk
			m_FlushSetting->evTerminated.wait();
			FlushDone();
		}
	} else if (m_FlushSetting != NULL) { //Some thing is going to be flushed or allready flushed wait the window call back to call FlushDone()
		return;
	}
	//MORPH END   - Added by SiRoB, Flush Thread
	
//Xman end
	if (m_BufferedData_list.IsEmpty())
		return;


	if (m_AllocateThread!=NULL) {
		// diskspace is being allocated right now.
		// so dont write and keep the data in the buffer for later.
		return;
	}
	//Xman Flush Thread improvement
	bool forcedbecauseincreasing=false;
	//Xman end

	// X: [GB] - [Global Buffer]
	uint64 partstart = (uint64)-1;
	if(bOnlyCompletedPart){
		if(datareceived){
			for(uint_ptr i = 0;i<GetPartCount(); ++i){
				if(!m_PartsShareable[i] && IsComplete((uint64)(PARTSIZE * i), (uint64)((PARTSIZE * (i + 1)) - 1), false)){
					partstart = i;
					break;
				}
			}
			if(partstart == (uint64)-1){
				datareceived = false;
				return;
			}
		}
		else if(m_anStates[DS_DOWNLOADING] > 0)
			return;
	}

	// SLUGFILLER: SafeHash
	if (forcewait) {	// Last chance to grab any ICH results
		Poco::FastMutex::SingleLock sLock(ICH_mut, true);	// ICH locks the file - otherwise it may be written to while being checked
		ParseICHResult();	// Check result from ICH
	}
	// SLUGFILLER: SafeHash

	//if (thePrefs.GetVerbose())
	//	AddDebugLogLine(false, _T("Flushing file %s - buffer size = %ld bytes (%ld queued items) transferred = %ld [time = %ld]"), GetFileName(), m_nTotalBufferData, m_BufferedData_list.GetCount(), m_uTransferred, m_nLastBufferFlushTime);

	CAtlList<PartFileBufferedData*>*pBufferedData_list;
	CAtlList<PartFileBufferedData*> tempBufferedData_list;
	if(!forcewait && (!bOnlyCompletedPart || datareceived)){
		size_t minblocksize = 0;
		pBufferedData_list = &tempBufferedData_list;
		if(!bOnlyCompletedPart){
			minblocksize = (size_t)(m_nTotalBufferData / max(8, m_anStates[DS_DOWNLOADING] + 2));
			if(minblocksize < 32*1024)// 32KB, buffer 256KB+
				minblocksize = 32*1024;
			else if(minblocksize > 256*1024) // 256KB, buffer 2MB+
				minblocksize = 256*1024;
		}
		bool breaknextloop = false;
		while(true){
			uint64 tempBufferSize = 0;
			uint64 bufStart = 0;
			uint64 bufEnd = 0;
			size_t blocks = 0;
			// Loop through queue
			POSITION posCur = m_BufferedData_list.GetHeadPosition();
			POSITION posStart = posCur;
			bool addtotemp;
			while (posCur){
				PartFileBufferedData *item1 = m_BufferedData_list.GetAt(posCur);
				if(bufEnd != item1->start){
					addtotemp = false;
					if(bufEnd){
						if(bOnlyCompletedPart){
							ASSERT(partstart != (uint64)-1);
							if(bufStart > (partstart+1)*PARTSIZE)
								break;
							if(bufEnd > partstart*PARTSIZE)
								addtotemp = true;
						}
						else if(bufEnd - bufStart >= minblocksize || IsComplete(bufStart?bufStart - 1:bufStart, bufEnd, false)){
							tempBufferSize += bufEnd - bufStart;
							addtotemp = true;
					}
					}
					if(addtotemp){
						while(posStart != posCur){
							tempBufferedData_list.AddTail(m_BufferedData_list.GetNext(posStart));
						}
					}
					else
						posStart = posCur;
					++blocks;
					bufStart = item1->start;
				}
				bufEnd = item1->end + 1;
				m_BufferedData_list.GetNext(posCur);
			}
			addtotemp = false;
			if(bOnlyCompletedPart){
				if(bufStart <= (partstart+1)*PARTSIZE && bufEnd > partstart*PARTSIZE)
					addtotemp = true;
			}
			else if(bufEnd - bufStart >= minblocksize || IsComplete(bufStart?bufStart - 1:bufStart, bufEnd, false)){
				tempBufferSize += bufEnd - bufStart;
				addtotemp = true;
			}
			if(addtotemp){
				while(posStart != posCur){
					tempBufferedData_list.AddTail(m_BufferedData_list.GetNext(posStart));
				}
			}
			if(m_nNextFlushBufferTime == 0 || breaknextloop)
				break;
			if(!tempBufferedData_list.IsEmpty()){
				 if(tempBufferSize && tempBufferSize * 2 > m_nTotalBufferData)
					break;
				 tempBufferedData_list.RemoveAll();
			}
			++blocks;
			minblocksize = (size_t)(m_nTotalBufferData / (blocks + 1));
			if(minblocksize < 32*1024){
				datareceived = false;
				pBufferedData_list = &m_BufferedData_list;
				break;
			}
			breaknextloop = true;
		}
		if(pBufferedData_list->IsEmpty())
			return;
	}
	else{
		datareceived = false;
		pBufferedData_list = &m_BufferedData_list;
	}
	bool *changedPart = NULL;

	try
	{
		bool bCheckDiskspace = thePrefs.IsCheckDiskspaceEnabled() && thePrefs.GetMinFreeDiskSpace() > 0;
		ULONGLONG uFreeDiskSpace = bCheckDiskspace ? CVolumeInfo::_GetFreeDiskSpace(GetTempPath()) : 0;

		// Check free diskspace for compressed/sparse files before possibly increasing the file size
		if (bCheckDiskspace && !IsNormalFile())
		{
			// Compressed/sparse files; regardless whether the file is increased in size, 
			// check the amount of data which will be written
			// would need to use disk cluster sizes for more accuracy
			if (m_nTotalBufferData + thePrefs.GetMinFreeDiskSpace() >= uFreeDiskSpace)
				AfxThrowFileException(CFileException::diskFull, 0, m_hpartfile.GetFileName());
		}

		//MORPH START - Flush Thread
		//WiZaRd: no need to double-parse... also, we are probably not supposed to call ParseICHResult if forcewait is false
		/*
		// SLUGFILLER: SafeHash
		Poco::FastMutex::SingleLock sLock(ICH_mut, true);	// ICH locks the file - otherwise it may be written to while being checked
		ParseICHResult();	// Check result from ICH
		// SLUGFILLER: SafeHash
		*/

		// Ensure file is big enough to write data to (the last item will be the furthest from the start)
		PartFileBufferedData *item = pBufferedData_list->GetTail();// X: [GB] - [Global Buffer]
		if (m_hpartfile.GetLength() <= item->end)
		{
			uint64 newsize = thePrefs.GetAllocCompleteMode()? GetFileSize() : (item->end + 1);
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
			{
				//Xman Flush Thread improvement
				if(forcewait==false)
					forcedbecauseincreasing=true;
				//Xman end
				forcewait=true;	// <2MB -> alloc it at once
			} //Xman

			// Allocate filesize
			if (!forcewait) {
				m_AllocateThread = new AllocateSpaceThread(this, newsize);
				if(m_AllocateThread->start())
					return;
					m_AllocateThread = NULL;
					TRACE(_T("Failed to create alloc thread! -> allocate blocking\n"));
					forcewait=true;
				}

			if (forcewait) {
				bIncreasedFile=true;
				// If this is a NTFS compressed file and the current block is the 1st one to be written and there is not 
				// enough free disk space to hold the entire *uncompressed* file, windows throws a 'diskFull'!?
				if (IsNormalFile())
					m_hpartfile.SetLength(newsize); // allocate disk space (may throw 'diskFull')
			}
		}

		m_nNextFlushBufferTime = 0;// X: [GB] - [Global Buffer]

		size_t partCount = GetPartCount();
		changedPart = new bool[partCount];
		// Remember which parts need to be checked at the end of the flush
		for (size_t partNumber = 0; partNumber < partCount; partNumber++)
			changedPart[partNumber] = false;

// X: [GB] - [Global Buffer]
#ifdef _DEBUG
		AddLogLine(false, bOnlyCompletedPart?_T("FB: %i(%i) [IF] %s"):_T("FB: %i(%i) %s"), pBufferedData_list->GetCount(), m_BufferedData_list.GetCount(), GetFileName());
#endif
		// Loop through queue
		uint64 previouspos = (uint64)-1; //MORPH - Optimization
		POSITION posCur = (pBufferedData_list == &m_BufferedData_list) ? NULL : m_BufferedData_list.GetHeadPosition();
		while (pBufferedData_list->GetCount() > 0)
		{
			// Get top item
			item = pBufferedData_list->RemoveHead();// X: [CI] - [Code Improvement]
			while(posCur){
				POSITION posRemove = posCur;
				PartFileBufferedData *item2 = m_BufferedData_list.GetNext(posCur);
				if(item == item2){
					m_BufferedData_list.RemoveAt(posRemove);
					break;
				}
			}
			// This is needed a few times
			uint32 lenData = (uint32)(item->end - item->start + 1);

			// SLUGFILLER: SafeHash - could be more than one part
			for (uint_ptr curpart = (uint_ptr)(item->start/PARTSIZE); curpart <= item->end/PARTSIZE; curpart++)
				changedPart[curpart] = true;
			// SLUGFILLER: SafeHash

			// Go to the correct position in file and write block of data

			if (previouspos != item->start) //MORPH - Optimization
			m_hpartfile.Seek(item->start, CFile::begin);
			m_hpartfile.Write(item->data, lenData);
			previouspos = item->end + 1; //MORPH - Optimization

			// Remove item from queue
			//pBufferedData_list->RemoveHead();

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

		//Creating the Thread to flush to disk
		m_FlushSetting = new FlushDone_Struct;
		m_FlushSetting->bForceICH = bForceICH;
		m_FlushSetting->changedPart = changedPart;

		if (forcewait == false || forcedbecauseincreasing==true) //Xman Flush Thread improvement
		{
			CPartFileFlushThread *flushThread = new CPartFileFlushThread(this);
			// X: queued disc-access for read/flushing-threads
			theApp.AddNewDiscAccessThread(flushThread);
				return;
		}
		m_hpartfile.Flush();
		FlushDone();
	}
	catch (CFileException* error)
	{
		FlushBuffersExceptionHandler(error);	
		delete[] changedPart;
		if (m_FlushSetting)
		{
			delete m_FlushSetting;
			m_FlushSetting = NULL;
		}
	}
#ifndef _DEBUG
	catch(...)
	{
		FlushBuffersExceptionHandler();
		delete[] changedPart;
		if (m_FlushSetting)
		{
			delete m_FlushSetting;
			m_FlushSetting = NULL;
		}
	}
#endif
}

void CPartFile::FlushDone()
{
	if (m_FlushSetting == NULL) //Already do in normal process
		return;

	// Check each part of the file
	// Only if hashlist is available
	if (m_FileIdentifier.HasExpectedMD4HashCount()){
		size_t partCount = GetPartCount();
		// Check each part of the file
		for (size_t uPartNumber = partCount; uPartNumber > 0; )
		{
			--uPartNumber;
			if (!m_FlushSetting->changedPart[uPartNumber])
				continue;
			// Any parts other than last must be full size
			if (m_FileIdentifier.GetTheoreticalMD4PartHashCount() != 0 && !m_FileIdentifier.GetMD4PartHash(uPartNumber)) {
				LogError(LOG_STATUSBAR, GetResString(IDS_ERR_INCOMPLETEHASH), GetFileName());
				m_bMD4HashsetNeeded = true;
				ASSERT(FALSE);	// If this fails, something was seriously wrong with the hashset loading or the check above
			}

			// Is this 9MB part complete
			//MORPH - Changed by SiRoB, As we are using flushed data check asynchronously we need to check if all data have been written into the file buffer
			/*
			if (IsComplete(PARTSIZE * (uint64)partNumber, (PARTSIZE * (uint64)(partNumber + 1)) - 1, false))
			*/
			if (IsComplete(PARTSIZE * (uint64)uPartNumber, (PARTSIZE * (uint64)(uPartNumber + 1)) - 1, true))
			{
				// Is part corrupt
				// Let's check in another thread
				if (CemuleDlg::IsRunning()) //Xman Flush Thread/Safehash (Morph)
				{
					CPartHashThread* parthashthread = new CPartHashThread(this, (uint16)uPartNumber);
					parthashthread->start();
				}
				//Xman Flush Thread/Safehash (Morph)
				else
				{
					//mono threaded at shutting down
					bool bAICHAgreed = false;
					if (!HashSinglePart(uPartNumber, &bAICHAgreed))
						PartHashFinished(uPartNumber, bAICHAgreed, true);
					else
						PartHashFinished(uPartNumber, bAICHAgreed, false);
				}
				//Xman end
			}
			else if (IsCorruptedPart(uPartNumber) && (thePrefs.IsICHEnabled() || m_FlushSetting->bForceICH))
			{
				if (CemuleDlg::IsRunning()) //Xman Flush Thread/Safehash (Morph)
				{
					CPartHashThread* parthashthread = new CPartHashThread(this, (uint16)uPartNumber, true);
					--m_PartsHashing;	// Special case, doesn't increment hashing parts, since part isn't really complete
					parthashthread->start();
				}
				//Xman Flush Thread/Safehash (Morph)
				else
				{
					//mono threaded at shutting down
					PartHashFinishedAICHRecover(uPartNumber, !HashSinglePart(uPartNumber));
				}
				//Xman end
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
	//SavePartFile(); //Xman MORPH - Flush Thread Moved Down

	if (CemuleDlg::IsRunning()) // may be called during shutdown!
	{
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
		if (bCheckDiskspace && ((IsNormalFile() && bIncreasedFile) || !IsNormalFile()))
		{
			bIncreasedFile = false;
			switch(GetStatus())
			{
			case PS_PAUSED:
			case PS_ERROR:
			case PS_COMPLETING:
			case PS_COMPLETE:
				break;
			default:
				if (CVolumeInfo::_GetFreeDiskSpace(GetTempPath()) < thePrefs.GetMinFreeDiskSpace())
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
				break;
			}
		}
	}
	delete[] m_FlushSetting->changedPart;
	delete m_FlushSetting;
	m_FlushSetting = NULL;
	// Update met file
	SavePartFile();
}

CPartFileFlushThread::CPartFileFlushThread(CPartFile* partfile) : Runnable(Poco::Thread::PRIO_LOW), m_partfile(partfile)
{
}

bool CPartFileFlushThread::run()
{
	/*DbgSetThreadName("Partfile-Flushing");

	// SLUGFILLER: SafeHash
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		{
		evTerminated.set();
		return;
		}
	// SLUGFILLER: SafeHash*/
		
	//InitThreadLocale(); //Performance killer

	//theApp.QueueDebugLogLine(false,_T("FLUSH:Start (%s)"),m_partfile->GetFileName()/*, CastItoXBytes(m_iAllocinfo, false, false)*/ );

	try{
		Poco::FastMutex::SingleLock sLock1(theApp.hashing_mut); //SafeHash - wait a current hashing process end before read the chunk
		if(!sLock1.tryLock())
			return false;

		// Flush to disk
		m_partfile->m_hpartfile.Flush();

		sLock1.Unlock(); //Xman SafeHash - unlock the mutex as fast as possible

		m_partfile->m_FlushSetting->evTerminated.set();
		VERIFY( PostMessage(theApp.emuledlg->m_hWnd,TM_FLUSHDONE,0,(LPARAM)m_partfile) );
		//theApp.QueueDebugLogLine(false,_T("FLUSH:End (%s)"),m_partfile->GetFileName());
	}
	catch(...)
	{
		VERIFY( PostMessage(theApp.emuledlg->m_hWnd,TM_FILEALLOCEXC,(WPARAM)m_partfile,0) );
		FlushDone_Struct* flushSetting = m_partfile->m_FlushSetting;
		m_partfile->m_FlushSetting = NULL;
		delete[] flushSetting->changedPart;
		delete flushSetting;
	}
	return true;
}
// END SiRoB: Flush Thread

void CPartFileFlushThread::setRemoved(void*file)
{
	if (m_partfile == file)
		removed = true;
}

void CPartFile::FlushBuffersExceptionHandler(CFileException* error)
{
	if (thePrefs.IsCheckDiskspaceEnabled() && error->m_cause == CFileException::diskFull)
	{
		//LogError(LOG_STATUSBAR | LOG_DONTNOTIFY, GetResString(IDS_ERR_OUTOFSPACE), GetFileName());

		// 'CFileException::diskFull' is also used for 'not enough min. free space'
		if (CemuleDlg::IsRunning())
		{
			if(thePrefs.GetNotifier()){
				CString msg;
				msg.Format(GetResString(IDS_ERR_OUTOFSPACE), GetFileName());
				theApp.emuledlg->TraySetBalloonToolTip(GetResString(IDS_ERROR), msg, NIIF_ERROR);
			}

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
			//LogError(LOG_STATUSBAR | LOG_DONTNOTIFY, GetResString(IDS_ERR_OUTOFSPACE), GetFileName());
			if (CemuleDlg::IsRunning()){
				if(thePrefs.GetNotifier()){
					CString msg;
					msg.Format(GetResString(IDS_ERR_OUTOFSPACE), GetFileName());
					theApp.emuledlg->TraySetBalloonToolTip(GetResString(IDS_ERROR), msg, NIIF_ERROR);
				}
				PauseFile(true/*bInsufficient*/);
		}
		}
		else
		{
			TCHAR buffer[MAX_CFEXP_ERRORMSG];
			error->GetErrorMessage(buffer,ARRSIZE(buffer));
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_WRITEERROR), GetFileName(), buffer);
			SetStatus(PS_ERROR);
		paused = true;
		m_iLastPausePurge = time(NULL);
		theApp.downloadqueue->RemoveLocalServerRequest(this);
		//Xman
		// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
		m_nDownDatarate = 0;
		m_nDownDatarate10 = 0;
		// Maella end
			//datarate = 0;
		m_anStates[DS_DOWNLOADING] = 0;
			//if (CemuleDlg::IsRunning()) // may be called during shutdown!
			//UpdateDisplayedInfo();
		}
	}

	error->Delete();
}

void CPartFile::FlushBuffersExceptionHandler()
{
	ASSERT(0);
	LogError(LOG_STATUSBAR, GetResString(IDS_ERR_WRITEERROR), GetFileName(), GetResString(IDS_UNKNOWN));
	SetStatus(PS_ERROR);
	paused = true;
	//Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	m_nDownDatarate = 0;
	m_nDownDatarate10 = 0;
	// Maella end
	m_iLastPausePurge = time(NULL);
	theApp.downloadqueue->RemoveLocalServerRequest(this);
	m_anStates[DS_DOWNLOADING] = 0;
	//if (CemuleDlg::IsRunning()) // may be called during shutdown!
	//UpdateDisplayedInfo();
}

void CPartFile::AllocateSpaceThread::run()
{
	DbgSetThreadName("Partfile-Allocate Space");

	/*//Xman
	// BEGIN SLUGFILLER: SafeHash
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return;
	// END SLUGFILLER: SafeHash*/
	//InitThreadLocale(); //Performance killer
	theApp.QueueDebugLogLine(false,_T("ALLOC:Start (%s) (%s)"),myfile->GetFileName(), CastItoXBytes(m_iAllocinfo, false, false) );

	try{
		// If this is a NTFS compressed file and the current block is the 1st one to be written and there is not 
		// enough free disk space to hold the entire *uncompressed* file, windows throws a 'diskFull'!?
		myfile->m_hpartfile.SetLength(m_iAllocinfo); // allocate disk space (may throw 'diskFull')

		// force the alloc, by temporary writing a non zero to the fileend
		byte x=255;
		myfile->m_hpartfile.Seek(-1,CFile::end);
		myfile->m_hpartfile.Write(&x,1);
		myfile->m_hpartfile.Flush();
		x=0;
		myfile->m_hpartfile.Seek(-1,CFile::end);
		myfile->m_hpartfile.Write(&x,1);
		myfile->m_hpartfile.Flush();
		theApp.QueueDebugLogLine(false,_T("ALLOC:End (%s)"),myfile->GetFileName());
	}
	catch (CFileException* error)
	{
		VERIFY( PostMessage(theApp.emuledlg->m_hWnd,TM_FILEALLOCEXC,(WPARAM)myfile,(LPARAM)error) );
	}
#ifndef _DEBUG
	catch(...)
	{
		VERIFY( PostMessage(theApp.emuledlg->m_hWnd,TM_FILEALLOCEXC,(WPARAM)myfile,0) );
	}
#endif
	myfile->m_AllocateThread=NULL;
}

// Barry - This will invert the gap list, up to caller to delete gaps when done
// 'Gaps' returned are really the filled areas, and guaranteed to be in order
void CPartFile::GetFilledList(CAtlList<Gap_Struct*> *filled) const
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
			}			
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

void CPartFile::UpdateDisplayedInfo(bool force)
{
	if (CemuleDlg::IsRunning()){
		DWORD curTick = ::GetTickCount();
		/*
        if(force || curTick-m_lastRefreshedDLDisplay > MINWAIT_BEFORE_DLDISPLAY_WINDOWUPDATE+m_random_update_wait) {
			theApp.emuledlg->transferwnd->downloadlistctrl.UpdateItem(this);
			m_lastRefreshedDLDisplay = curTick;
		}*/
		//Xman new improvement:
		if(force || curTick-m_lastRefreshedDLDisplay > 550){ // MINWAIT_BEFORE_DLDISPLAY_WINDOWUPDATE+m_random_update_wait) {
			theApp.emuledlg->transferwnd->downloadlistctrl.RefreshFile(this);
			m_lastRefreshedDLDisplay = curTick;
		}
	}
}

void CPartFile::UpdateAutoDownPriority(){
	//Xman Code Improvement
	size_t sourcecount = GetSourceCount();
	if( !IsAutoDownPriority()/* || status != PS_READY || stopped*/ || paused || insufficient ) //Xman Code Improvement
		return;
	if ( sourcecount > 100 ){
		SetDownPriority( PR_LOW );
		return;
	}
	if ( sourcecount > 20 ){
		SetDownPriority( PR_NORMAL );
		return;
	}
	//Xman end
	SetDownPriority( PR_HIGH );
}

size_t CPartFile::GetCategory() /*const*/
{
	if (m_category > thePrefs.GetCatCount() - 1)
		m_category = 0;
	return m_category;
}
/*
bool CPartFile::HasDefaultCategory() const // extra function for const 
{
	return m_category == 0 || m_category > (size_t)(thePrefs.GetCatCount() - 1);
}
*/
//Xman checkmark to catogory at contextmenu of downloadlist
size_t CPartFile::GetConstCategory() const
{
	return m_category > (thePrefs.GetCatCount() - 1) ? 0:m_category;
}
//Xman end

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
			for (size_t i = 0; i < GetPartCount(); i++){
				if (gapstart >= (uint64)i*PARTSIZE && gapstart <=  (uint64)(i+1)*PARTSIZE){ // is in this part?
					if (gapend <= (uint64)(i+1)*PARTSIZE)
						gapdone = true;
					else{
						gapend = (uint64)(i+1)*PARTSIZE; // and next part
					}
					// paint
					uint8 color;
					if (m_SrcpartFrequency.GetCount() >= i && m_SrcpartFrequency[(uint16)i])  // frequency?
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

void CPartFile::SetCategory(size_t cat)
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
	if (CemuleDlg::IsRunning())
	{
		NotifyStatusChange();
		UpdateCompletedInfos(); //Xman moved from Drawstatusbar, because we don't draw hidden rects
		if (thePrefs.ShowCatTabInfos())
			theApp.emuledlg->transferwnd->UpdateCatTabTitles();
	}
}

void CPartFile::NotifyStatusChange() 
{
	if (CemuleDlg::IsRunning()){
		theApp.emuledlg->transferwnd->downloadlistctrl.UpdateCurrentCategoryView(this);
		if(theApp.emuledlg->activewnd == theApp.emuledlg->transferwnd){
			UpdateDisplayedInfo(true);
			theApp.emuledlg->transferwnd->partstatusctrl.Refresh(this);
		}
	}
}

EMFileSize CPartFile::GetRealFileSize() const
{
	return ::GetDiskFileSize(GetFilePath());
}

uint8* CPartFile::MMCreatePartStatus(){
	// create partstatus + info in mobilemule protocol specs
	// result needs to be deleted[] | slow, but not timecritical
	uint8* result = new uint8[GetPartCount()+1];
	for (size_t i = 0; i < GetPartCount(); i++){
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

//Xman Dynamic block request
uint32 CPartFile::GetDownloadSpeedInPart(uint16 forpart, CUpDownClient* current_source) const
{
	ASSERT(forpart!=(uint16)-1);
	uint32 parttransferrate=0;
	for(POSITION pos = m_downloadingSourceList.GetHeadPosition(); pos != NULL;)
	{
		//CUpDownClient* cur_src = srclist.GetNext(pos); //why we iterate in srclist?
		CUpDownClient* cur_src = m_downloadingSourceList.GetNext(pos);
		//zz_fly :: delayed deletion of downloading source :: Enig123 :: Start
		if (cur_src == NULL || m_downloadingDeleteList.Find(cur_src)) 
			continue;
		//zz_fly :: delayed deletion of downloading source :: Enig123 :: End
		if(cur_src->m_lastPartAsked==forpart && cur_src!=current_source)
		{
			parttransferrate += cur_src->GetDownloadDatarate10();
		}
	}
	return parttransferrate;
}
//Xman end
//Xman Dynamic block request (netfinity/morph)
//Xman end

CString CPartFile::GetInfoSummary() const
{
	if (!IsPartFile())
		return CKnownFile::GetInfoSummary();

	CString Sbuffer, lsc, compl, buffer, lastdwl;

	lsc = CastItoXBytes(GetCompletedSize(), false, false);
	compl = CastItoXBytes(GetFileSize(), false, false);
	buffer.Format(_T("%s/%s"), lsc, compl);
	compl.Format(_T("%s: %s (%.2f%%)\n"), GetResString(IDS_DL_TRANSFCOMPL), buffer, GetPercentCompleted());

	if (lastseencomplete == NULL)
		lsc = GetResString(IDS_NEVER);
	else
		lsc = lastseencomplete.Format(thePrefs.GetDateTimeFormat());

	float availability = 0.0F;
	if (GetPartCount() != 0)
		availability = (float)(GetAvailablePartCount() * 100.0 / GetPartCount());

	CString avail;
	avail.Format(GetResString(IDS_AVAIL), GetPartCount(), GetAvailablePartCount(), availability);

	if (GetCFileDate() != NULL)
		lastdwl = GetCFileDate().Format(thePrefs.GetDateTimeFormat());
	else
		lastdwl = GetResString(IDS_NEVER);

	CString sourcesinfo;
	sourcesinfo.Format(GetResString(IDS_DL_SOURCES) + _T(": ") + GetResString(IDS_SOURCESINFO) + _T('\n'), GetSourceCount(), GetValidSourcesCount(), GetSrcStatisticsValue(DS_NONEEDEDPARTS), GetSrcA4AFCount());

	// always show space on disk
	CString sod = _T("  (") + GetResString(IDS_ONDISK) + CastItoXBytes(GetRealFileSize(), false, false) + _T(')');

	CString status;
	if (GetTransferringSrcCount() > 0)
		status.Format(GetResString(IDS_PARTINFOS2) + _T('\n'), GetTransferringSrcCount());
	else 
		status.Format(_T("%s\n"), getPartfileStatus());

	CString info;
	info.Format(_T("%s\n")
		+ GetResString(IDS_FD_HASH) + _T(" %s\n")
		+ GetResString(IDS_FD_SIZE) + _T(" %s  %s\n<br>\n")
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
bool CPartFile::CheckShowItemInGivenCat(size_t inCategory) /*const*/
{
	size_t myfilter=thePrefs.GetCatFilter(inCategory);

	// common cases
	if (inCategory>=thePrefs.GetCatCount())
		return false;
	if ((inCategory == GetCategory() && myfilter == 0))
		return true;
	if (inCategory > 0 && GetCategory() != inCategory && !thePrefs.GetCategory(inCategory)->care4all )
		return false;


	bool ret=true;
	if ( myfilter > 0)
	{
		if (myfilter>=4 && myfilter<=8 && !IsPartFile())
			ret=false;
		else switch (myfilter)
		{
			case 1 : ret=(GetCategory() == 0);break;
			case 4 : ret= ((GetStatus()==PS_READY || GetStatus()==PS_EMPTY) && GetTransferringSrcCount()==0);break;
			case 5 : ret= ((GetStatus()==PS_READY || GetStatus()==PS_EMPTY) && GetTransferringSrcCount()>0);break;
			case 6 : ret= (GetStatus()==PS_ERROR);break;
			case 7 : ret= (GetStatus()==PS_PAUSED || IsStopped() );break;
			case 8 : ret=  lastseencomplete!=NULL ;break;
			case 10 : ret= IsMovie();break;
			case 11 : ret= (ED2KFT_AUDIO == GetED2KFileTypeID(GetFileName()));break;
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
	theApp.emuledlg->transferwnd->downloadlistctrl.UpdateCurrentCategoryView(this);
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
			m_nDlActiveTime += (uint_ptr)(tNow - m_tActivated);
			m_tActivated = 0;
		}
	}
}

uint_ptr CPartFile::GetDlActiveTime() const// X: [64T] - [64BitTime]
{
	uint_ptr nDlActiveTime = m_nDlActiveTime;// X: [64T] - [64BitTime]
	if (m_tActivated != 0)
		nDlActiveTime += (uint_ptr)(time(NULL) - m_tActivated);
	return nDlActiveTime;
}

bool CPartFile::RightFileHasHigherPrio(CPartFile* left,CPartFile* right, bool allow_go_over_hardlimit) {
    if(!right)
        return false;

	//Xman Xtreme Downloadmanager
	if(allow_go_over_hardlimit==false && right->GetSourceCount() > right->GetMaxSources())
		return false;
	//Xman end

    if(!left)
        return true;
		//Xman Xtreme Downloadmanager: Auto-A4AF-check
	if(!left->IsA4AFAuto()){
		if(right->IsA4AFAuto())
			return true;
		  //Xman end
		Category_Struct* leftcat = thePrefs.GetCategory(left->GetCategory());
		Category_Struct* rightcat = thePrefs.GetCategory(right->GetCategory());
		if(	rightcat->prio > leftcat->prio || 
			rightcat->prio == leftcat->prio && (
              right->GetDownPriority() > left->GetDownPriority() ||
              right->GetDownPriority() == left->GetDownPriority() && (
                  right->GetCategory() == left->GetCategory() && right->GetCategory() != 0 &&
                  (rightcat->downloadInAlphabeticalOrder && thePrefs.IsExtControlsEnabled()) && 
                  right->GetFileName() && left->GetFileName() &&
                  right->GetFileName().CompareNoCase(left->GetFileName()) < 0
              )
          )
    ) {
        return true;
		}
	}
	//Xman Xtreme Downloadmanager: Auto-A4AF-check
		//Xman Xtreme Downloadmanager
		if(left->IsA4AFAuto()==right->IsA4AFAuto() && //Xman Xtreme Downloadmanager: Auto-A4AF-check
			thePrefs.GetCategory(right->GetCategory())->prio == thePrefs.GetCategory(left->GetCategory())->prio
			&& right->GetDownPriority() == left->GetDownPriority()
			&& (right->GetCategory() != left->GetCategory()
				|| right->GetCategory() == left->GetCategory() && (!thePrefs.GetCategory(right->GetCategory())->downloadInAlphabeticalOrder || !thePrefs.IsExtControlsEnabled()))
			&& right->GetSourceCount() < left->GetSourceCount()
			)
			return true;
		//Xman end
			return false;
    }

void CPartFile::RequestAICHRecovery(uint_ptr nPart, bool failed) // X: [IP] - [Import Parts] & [IPR] - [Improved Part Recovery]
{
	if (failed)
		lastAICHRequestFailed = GetTickCount();
	else if (GetTickCount() - lastAICHRequestFailed < 5 * 60 * 1000)
		return;

	if (!m_pAICHRecoveryHashSet->HasValidMasterHash() || ((m_pAICHRecoveryHashSet->GetStatus() != AICH_TRUSTED && m_pAICHRecoveryHashSet->GetStatus() != AICH_VERIFIED)
			&& !m_pAICHRecoveryHashSet->TrustMajority())){
		AddDebugLogLine(DLP_DEFAULT, false, _T("Unable to request AICH Recoverydata because we have no trusted Masterhash"));
		return;
	}
	if (GetFileSize() <= (uint64)EMBLOCKSIZE || GetFileSize() - PARTSIZE*(uint64)nPart <= (uint64)EMBLOCKSIZE)
		return;
	if (CAICHRecoveryHashSet::IsClientRequestPending(this, (uint16)nPart)){
		//AddDebugLogLine(DLP_DEFAULT, false, _T("RequestAICHRecovery: Already a request for this part pending"));
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
	UINT_PTR cAICHClients = 0;
	UINT_PTR cAICHLowIDClients = 0;
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
		lastAICHRequestFailed = GetTickCount() + 25 * 60 * 1000;
		AddDebugLogLine(DLP_DEFAULT, false, _T("Unable to request AICH Recoverydata because found no client who supports it and has the same hash as the trusted one"));
			return;
		}
	uint_ptr nSelectedClient;
	if (cAICHClients > 0)
		nSelectedClient = (t_rng->getUInt32() % cAICHClients) + 1;
	else
		nSelectedClient = (t_rng->getUInt32() % cAICHLowIDClients) + 1;

	CUpDownClient* pClient = NULL;
	for (POSITION pos = srclist.GetHeadPosition(); pos != NULL;){
		CUpDownClient* pCurClient = srclist.GetNext(pos);
		if (pCurClient->IsSupportingAICH() && pCurClient->GetReqFileAICHHash() != NULL && !pCurClient->IsAICHReqPending()
			&& (*pCurClient->GetReqFileAICHHash()) == m_pAICHRecoveryHashSet->GetMasterHash())
		{
			if (cAICHClients > 0){
				if (!pCurClient->HasLowID())
					nSelectedClient--;
			}
			else{
				ASSERT( pCurClient->HasLowID());
				nSelectedClient--;
			}
			if (nSelectedClient == 0){
				pClient = pCurClient;
				break;
			}
		}
	}
	if (pClient == NULL){
		ASSERT( false );
		return;
	}
	AddDebugLogLine(DLP_DEFAULT, false, _T("Requesting AICH Hash (%s) from client %s"),cAICHClients? _T("HighID"):_T("LowID"), pClient->DbgGetClientInfo());
	pClient->SendAICHRequest(this, (uint16)nPart);
}

void CPartFile::AICHRecoveryDataAvailable(uint_ptr nPart)
{
	if (GetPartCount() < nPart){
		ASSERT( false );
		return;
	}
	lastAICHRequestFailed = 0;
	FlushBuffer(true, true);
	uint_ptr length = PARTSIZE;
	if ((ULONGLONG)PARTSIZE*(uint64)(nPart+1) > m_hpartfile.GetLength()){
		length = (uint_ptr)(m_hpartfile.GetLength() - ((ULONGLONG)PARTSIZE*(uint64)nPart));
		ASSERT( length <= PARTSIZE );
	}	
	// if the part was already ok, it would now be complete
	if (IsComplete((uint64)nPart*PARTSIZE, (((uint64)nPart*PARTSIZE)+length)-1, true)){
		AddDebugLogLine(DLP_DEFAULT, false, _T("Processing AICH Recovery data: The part (%u) is already complete, canceling"),nPart);
		//AddDebugLogLine(DLP_DEFAULT, false, _T("Processing AICH Recovery data: The part (%u) is already complete, canceling"));
		return;
	}



	CAICHHashTree* pVerifiedHash = m_pAICHRecoveryHashSet->m_pHashTree.FindHash((uint64)nPart*PARTSIZE, length);
	if (pVerifiedHash == NULL || !pVerifiedHash->m_bHashValid){
		AddDebugLogLine(DLP_DEFAULT, false, _T("Processing AICH Recovery data: Unable to get verified hash from hashset (should never happen)"));
		ASSERT( false );
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
		AddDebugLogLine(DLP_DEFAULT, false, _T("Processing AICH Recovery data: Failed to retrieve AICH Hashset of corrupt part"));
		ASSERT( false );
		return;
	}

	// now compare the hash we just did, to the verified hash and readd all blocks which are ok
	uint_ptr nRecovered = 0;
	for (uint_ptr pos = 0; pos < length; pos += EMBLOCKSIZE){
		const uint_ptr nBlockSize = min(EMBLOCKSIZE, length - pos);
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
	//Xman
	m_CorruptionBlackBox.EvaluateData(nPart);
	//Xman end

	if (m_uCorruptionLoss >= nRecovered)
		m_uCorruptionLoss -= nRecovered;
	if (thePrefs.sesLostFromCorruption >= nRecovered)
		thePrefs.sesLostFromCorruption -= nRecovered;

	POSITION posCorrupted = corrupted_list.Find((uint16)nPart); // X: [IPR] - [Improved Part Recovery]
	if (posCorrupted)
		corrupted_list.RemoveAt(posCorrupted);

	// ok now some sanity checks
	if (IsComplete((uint64)nPart*PARTSIZE, (((uint64)nPart*PARTSIZE)+length)-1, true)){
		// this is a bad, but it could probably happen under some rare circumstances
		// make sure that HashSinglePart() (MD4 and possibly AICH again) agrees to this fact too, for Verified Hashes problems are handled within that functions, otherwise:
		//Xman
		// BEGIN SLUGFILLER: SafeHash - In another thread
		CPartHashThread* parthashthread = new CPartHashThread(this, (uint16)nPart, false, true);
		parthashthread->start();
		// END SLUGFILLER: SafeHash

	} // end sanity check
	// Update met file
	SavePartFile();
	// make sure the user appreciates our great recovering work :P
	AddLogLine(true, GetResString(IDS_AICH_WORKED), CastItoXBytes(nRecovered), CastItoXBytes(length), nPart, GetFileName());
	//AICH successfully recovered %s of %s from part %u for %s
}

UINT CPartFile::GetMaxSources() const
{
	//Xman Xtreme Mod
	//hardlimit of 5 for emule collections
	if(GetFileSize()< (uint64)MAXPRIORITYCOLL_SIZE && HasCollectionExtenesion_Xtreme() /*CCollection::HasCollectionExtention(GetFileName())*/) //Xman Code Improvement for HasCollectionExtention
		return 5;
	//Xman end

	// Ignore any specified 'max sources' value if not in 'extended mode' -> don't use a parameter which was once
	// specified in GUI but can not be seen/modified any longer..
	return (!thePrefs.IsExtControlsEnabled() || m_uMaxSources == 0) ? thePrefs.GetMaxSourcePerFileDefault() : m_uMaxSources;
}

UINT_PTR CPartFile::GetMaxSourcePerFileSoft() const
{

	//Xman Xtreme Downloadmanager
	//Xman sourcecache
	//because we uses our sources longer, we may allow a shorter XS
	//UINT temp;
	UINT_PTR maxsources = GetMaxSources();
	if(maxsources>150)
		//temp = (UINT)(maxsources*0.95f);
		maxsources -= maxsources / 20;
	else
		//temp = (UINT)(maxsources*0.9f);
		maxsources -= maxsources / 10;
	//UINT temp = ((UINT)GetMaxSources() * 9L) / 10;
	//Xman end
	if (maxsources > MAX_SOURCES_FILE_SOFT)
		return MAX_SOURCES_FILE_SOFT;
	return maxsources;
}

UINT_PTR CPartFile::GetMaxSourcePerFileUDP() const
{	
	UINT_PTR temp = ((UINT_PTR)GetMaxSources() * 3L) / 4;
	if (temp > MAX_SOURCES_FILE_UDP)
		return MAX_SOURCES_FILE_UDP;
	return temp;
}

CString CPartFile::GetTempPath() const
{
	return m_fullname.Left(m_fullname.ReverseFind(_T('\\'))+1);
}


void CPartFile::SetFileSize(EMFileSize nFileSize)
{
	ASSERT( m_pAICHRecoveryHashSet != NULL );
	m_pAICHRecoveryHashSet->SetFileSize(nFileSize);
	CKnownFile::SetFileSize(nFileSize);
}

//Xman
// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
void CPartFile::CompDownloadRate(){	
	// Remark: This method is called once every second

	//Xman 4.3 Code-Improvement
	m_nDownDatarate = 0;
	m_nDownDatarate10 = 0;
	EPartFileStatus tocheck=GetStatus();
	if(tocheck==PS_READY || tocheck==PS_EMPTY || tocheck==PS_COMPLETING)
	{
	
		// Process and retrieve m_nUpDatarate form clients
		for(POSITION pos = m_downloadingSourceList.GetHeadPosition(); pos != NULL; ){
			CUpDownClient* cur_client = m_downloadingSourceList.GetNext(pos);
			//zz_fly :: delayed deletion of downloading source :: Enig123 :: Start
			if (m_sourceListChange && m_downloadingDeleteList.Find(cur_client))
				continue;
			//zz_fly :: delayed deletion of downloading source :: Enig123 :: End
			if(cur_client->GetDownloadState() == DS_DOWNLOADING){ 
				cur_client->CompDownloadRate();
				m_nDownDatarate += cur_client->GetDownloadDatarate();  // [bytes/s]
				m_nDownDatarate10 += cur_client->GetDownloadDatarate10();  // [bytes/s]
			}
		}
		//every second
		UpdateCompletedInfos(); //Remark: don't update this at drawstatusbar, because we don't draw hidden rects -> the value isn't up to date -> wrong sorting!
		if(tocheck!=PS_COMPLETING){
			if(xState == PART_CORRUPTED){ // X: [IP] - [Import Parts] & [IPR] - [Improved Part Recovery]
				ASSERT(GetFileOp() != PFOP_HASHING);
				if(corrupted_list.GetCount() > 0){
					if(CemuleDlg::IsRunning() && m_pAICHRecoveryHashSet->HasValidMasterHash() && (m_anStates[DS_ONQUEUE]+m_anStates[DS_DOWNLOADING])*4>GetSourceCount()){
						xState = PFS_NORMAL;
						for (POSITION pos = srclist.GetHeadPosition(); pos != NULL;){
							CUpDownClient* pCurClient = srclist.GetNext(pos);
							if (pCurClient->IsSupportingAICH() && pCurClient->GetReqFileAICHHash() != NULL){
								xState = PART_CORRUPTED_AICH;
								break;
					}
				}
			}
				}
				else
					xState = PFS_NORMAL;
			}
			else if(xState == IP_WAITING_AICH){ // X: [IP] - [Import Parts]
				if(!m_bMD4HashsetNeeded && CemuleDlg::IsRunning() && m_anStates[DS_ONQUEUE]*4>GetSourceCount() && m_pAICHRecoveryHashSet->HasValidMasterHash() && (m_pAICHRecoveryHashSet->GetStatus() == AICH_TRUSTED || m_pAICHRecoveryHashSet->GetStatus() == AICH_VERIFIED)){
					xState = PFS_HASHING;
					while (gaplist.GetCount()>0 )
						delete gaplist.RemoveHead();
					CompleteFile(false);
				}
			}
			else if(xState == PART_CORRUPTED_AICH){ // X: [IP] - [Import Parts] & [IPR] - [Improved Part Recovery]
				if(corrupted_list.GetCount() > 0){
					int nPart = corrupted_list.RemoveHead();
					if (nPart == GetPartCount() - 1 && GetFileSize() - nPart * PARTSIZE <= EMBLOCKSIZE)
					{
					}
					else
					{
						corrupted_list.AddTail(nPart);
						if(CemuleDlg::IsRunning())
							RequestAICHRecovery(nPart, false);
					}
				}
				else
					xState = PFS_NORMAL;
			}
		}
		if(theApp.emuledlg->activewnd == theApp.emuledlg->transferwnd){
			UpdateDisplayedInfo(true);
			theApp.emuledlg->transferwnd->partstatusctrl.Refresh(this);
		}
		//UpdateCompletedInfos();
		//m_lastRefreshedDLDisplay = ::GetTickCount(); //xman improvement
	}
}
// Maella end

//Xman Xtreme Downloadmanager
void CPartFile::RemoveNoNeededPartsSources()
{
	//AddDebugLogLine(false, "GetMaxConnectionReached(%i) GetOpenSockets(%i)",theApp.listensocket->GetMaxConnectionReached(),theApp.listensocket->GetOpenSockets());
	//uint32 removed = 0;

	for(POSITION pos = srclist.GetHeadPosition(); pos != NULL; )
	{
		CUpDownClient* cur_src = srclist.GetNext(pos);
		if (cur_src != NULL && cur_src->GetDownloadState() == DS_NONEEDEDPARTS) 
		{
			if (!cur_src->SwapToAnotherFile(false, false, false, NULL))
			{
				cur_src->droptime=::GetTickCount(); 
				theApp.downloadqueue->RemoveSource(cur_src);
				//removed ++; // Count removed source
			}
			else
				cur_src->DontSwapTo(this);
		}
	}
	UpdateDisplayedInfo(true);
}

void CPartFile::RemoveQueueFullSources()
{
	//AddDebugLogLine(false, "GetMaxConnectionReached(%i) GetOpenSockets(%i)",theApp.listensocket->GetMaxConnectionReached(),theApp.listensocket->GetOpenSockets());
	//uint32 removed = 0;	

	for(POSITION pos = srclist.GetHeadPosition(); pos != NULL; )
	{
		CUpDownClient* cur_src = srclist.GetNext(pos);
		if (cur_src != NULL && cur_src->GetDownloadState() == DS_ONQUEUE && cur_src->IsRemoteQueueFull()) 
		{
			if (!cur_src->SwapToAnotherFile(false, false, false, NULL))
			{
				cur_src->droptime=::GetTickCount(); 
				theApp.downloadqueue->RemoveSource(cur_src);
				//removed ++; // Count removed source
			}
			else
				cur_src->DontSwapTo(this);
		}
	}	
	UpdateDisplayedInfo(true);
}

void CPartFile::CalcAvgQr(UINT inqr,UINT oldqr)
{
	if( oldqr!=0)
	{
		if(m_countqr==0)
			AddDebugLogLine(false, _T("Error in calculating QR-Average, shouldn't happen"));
		else
		{
			m_countqr--;
			m_sumqr -=oldqr;
		}
	}
	if(inqr!=0 )
	{
		m_countqr++;
		m_sumqr += inqr;
	}
	if(m_countqr!=0)
		m_avgqr=m_sumqr/m_countqr;
	else
		m_avgqr=0;
}
//Xman end

//Xman sourcecache
void CPartFile::ClearSourceCache()
{
	m_sourcecache.RemoveAll();
}

void CPartFile::AddToSourceCache(uint16 nPort, uint32 dwID, uint32 dwServerIP,uint16 nServerPort, ESourceFrom sourcefrom, bool ed2kIDFlag, const uchar* achUserHash, uint8 byCryptOptions)
{
	PartfileSourceCache newsource;
	newsource.nPort=nPort;
	newsource.dwID=dwID;
	newsource.dwServerIP=dwServerIP;
	newsource.nServerPort=nServerPort;
	newsource.ed2kIDFlag=ed2kIDFlag;
	newsource.sourcefrom=sourcefrom;
	newsource.expires=::GetTickCount() + SOURCECACHELIFETIME;
	newsource.byCryptOptions=byCryptOptions;
	if(achUserHash!=NULL)
	{
		md4cpy(newsource.achUserHash,achUserHash);
		newsource.withuserhash=true;
	}
	else
	{
		newsource.withuserhash=false;
		//md4clr(newsource.achUserHash); //not needed
	}

	m_sourcecache.AddTail(newsource);
}

void CPartFile::ProcessSourceCache()
{
	if(m_lastSoureCacheProcesstime + SOURCECACHEPROCESSLOOP < ::GetTickCount())
	{
		const uint32 currenttime = ::GetTickCount(); //cache value
		m_lastSoureCacheProcesstime=currenttime;

		//if file is stopped clear the cache and return
		if(stopped)
		{
			m_sourcecache.RemoveAll();
			return;
		}

		while(m_sourcecache.IsEmpty()==false && m_sourcecache.GetHead().expires<currenttime)
		{
			m_sourcecache.RemoveHead();
		}
		uint_ptr sourcesadded=0;
		while(m_sourcecache.IsEmpty()==false && GetMaxSources()  > this->GetSourceCount() +1) //let room for 1 passiv source	
		{
			PartfileSourceCache currentsource=m_sourcecache.RemoveHead();
			CUpDownClient* newsource = new CUpDownClient(this,currentsource.nPort, currentsource.dwID,currentsource.dwServerIP,currentsource.nServerPort,currentsource.ed2kIDFlag);
			newsource->SetConnectOptions(currentsource.byCryptOptions,true,false);
			newsource->SetSourceFrom(currentsource.sourcefrom);
			if(currentsource.withuserhash==true)
				newsource->SetUserHash(currentsource.achUserHash);
			if(theApp.downloadqueue->CheckAndAddSource(this,newsource))
				sourcesadded++;
		}
		if(sourcesadded>0 && thePrefs.GetDebugSourceExchange())
			AddDebugLogLine(false,_T("-->%u sources added via sourcache. file: %s"),sourcesadded,GetFileName()); 
	}
}
//Xman end

//Xman manual file allocation (Xanatos)
void CPartFile::AllocateNeededSpace()
{
	if (m_AllocateThread!=NULL)
		return;

	// Allocate filesize
	m_AllocateThread = new AllocateSpaceThread(this, GetFileSize());
	if(!m_AllocateThread->start())
	{
		m_AllocateThread = NULL;
		TRACE(_T("Failed to create alloc thread! -> allocate blocking\n"));
	}
}
//Xman end

//Xman
// BEGIN SiRoB, SLUGFILLER: SafeHash
void CPartFile::PerformFirstHash()
{
	CPartHashThread* parthashthread = new CPartHashThread(this);	// Only hashes completed parts, why hash gaps?
	parthashthread->start();
}

bool CPartFile::IsPartShareable(uint_ptr partnumber) const
{
	if (partnumber < GetPartCount())
		return m_PartsShareable[partnumber];
	else
		return false;
}

bool CPartFile::IsRangeShareable(uint64 start, uint64 end) const
{
	uint_ptr first = (uint_ptr)(start/PARTSIZE);
	uint_ptr last = (uint_ptr)(end/PARTSIZE+1);
	if (last > GetPartCount() || first >= last)
		return false;
	for (uint_ptr i = first; i < last; i++)
		if (!m_PartsShareable[i])
			return false;
	return true;
}
void CPartFile::PartHashFinished(uint_ptr partnumber, bool bAICHAgreed, bool corrupt)
{
	if (partnumber >= GetPartCount())
		return;
	m_PartsHashing--;
	uint64 partRange = (partnumber < (uint_ptr)(GetPartCount()-1))?PARTSIZE:((uint64)m_nFileSize % PARTSIZE);
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
			corrupted_list.AddTail((uint16)partnumber);

		// request AICH recovery data, except if AICH already agreed anyway
		//if (!bAICHAgreed)
			//RequestAICHRecovery(partnumber, false);
		ASSERT(xState != PFS_HASHING);
		if(xState == PFS_NORMAL) // X: [IPR] - [Improved Part Recovery]
			xState = PART_CORRUPTED;

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
		POSITION posCorrupted = corrupted_list.Find((uint16)partnumber);
		if (posCorrupted)
			corrupted_list.RemoveAt(posCorrupted);

		// Successfully completed part, make it available for sharing
		m_PartsShareable[partnumber] = true;
		if (status == PS_EMPTY)
		{
			SetStatus(PS_READY);
			if (CemuleDlg::IsRunning())	// may be called during shutdown!
				theApp.sharedfiles->SafeAddKFile(this);
		}

		if (!m_PartsHashing){
			// Update met file - file fully hashed
			SavePartFile();
		}

		if (CemuleDlg::IsRunning())	// may be called during shutdown!
		{
			// Is this file finished?
			if (!m_PartsHashing && gaplist.IsEmpty() && !m_nTotalBufferData && !m_FlushSetting){ //Xman - MORPH - Changed by SiRoB, Flush Thread
				// NEO: POFC - [PauseOnFileComplete]
				if(thePrefs.m_bPauseOnFileComplete)
					StopOnFileComplete();
				else
				// NEO: POFC END
					CompleteFile(false);	// Recheck all hashes, because loaded data is trusted based on file-date
			}
			else if (m_bPauseOnPreview && IsReadyForPreview())
			{
				m_bPauseOnPreview = false;
				PauseFile();
			}
		}
	}
}

void CPartFile::StopOnFileComplete(){// X: [POFC] - [PauseOnFileComplete]
	xState = POFC_WAITING;
	UpdateCompletedInfos();
	theApp.downloadqueue->StartNextFileIfPrefs(GetCategory());
	StopFile();
	if(thePrefs.GetNotifier())
		theApp.emuledlg->TraySetBalloonToolTip(GetResString(IDS_CL_DOWNLSTATUS), GetResString(IDS_TBN_DOWNLOADDONE) + _T('\n') + GetFileName());
}

void CPartFile::PartHashFinishedAICHRecover(uint_ptr partnumber, bool corrupt)
{
	if (partnumber >= GetPartCount())
		return;
	m_PartsHashing--;
	if (corrupt){
		uint64 partRange = (partnumber < (uint_ptr)(GetPartCount()-1))?PARTSIZE:((uint64)m_nFileSize % PARTSIZE);
		AddDebugLogLine(DLP_DEFAULT, false, _T("Processing AICH Recovery data: The part (%u) got completed while recovering - but MD4 says it corrupt! Setting hashset to error state, deleting part"), partnumber);
		// now we are fu... unhappy
		if (!m_FileIdentifier.HasAICHHash())
			m_pAICHRecoveryHashSet->SetStatus(AICH_ERROR);
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
		POSITION posCorrupted = corrupted_list.Find((uint16)partnumber);
		if (posCorrupted)
			corrupted_list.RemoveAt(posCorrupted);
		// Successfully recovered part, make it available for sharing
		m_PartsShareable[partnumber] = true;
		if (status == PS_EMPTY)
		{
			SetStatus(PS_READY);
			if (CemuleDlg::IsRunning())	// may be called during shutdown!
				theApp.sharedfiles->SafeAddKFile(this);
		}

		if (!m_PartsHashing){
			// Update met file - file fully hashed
			SavePartFile();
		}

		if (CemuleDlg::IsRunning()){
			// Is this file finished?
			if (!m_PartsHashing && gaplist.IsEmpty() && !m_nTotalBufferData && !m_FlushSetting) //Xman - MORPH - Changed by SiRoB, Flush Thread
			{
				// NEO: POFC - [PauseOnFileComplete]
				if(thePrefs.m_bPauseOnFileComplete)
					StopOnFileComplete();
				else
				// NEO: POFC END
					CompleteFile(false);	// Recheck all hashes, because loaded data is trusted based on file-date
			}
			else if (m_bPauseOnPreview && IsReadyForPreview())
			{
				m_bPauseOnPreview = false;
				PauseFile();
			}
		}
	}
}

void CPartFile::ParseICHResult()
{
	if (m_ICHPartsComplete.IsEmpty())
		return;

	while (!m_ICHPartsComplete.IsEmpty()) {
		uint16 partnumber = m_ICHPartsComplete.RemoveHead();
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
			if (CemuleDlg::IsRunning())	// may be called during shutdown!
				theApp.sharedfiles->SafeAddKFile(this);
		}
	}

	// Update met file - gaps data changed
	SavePartFile();

	if (CemuleDlg::IsRunning()){ // may be called during shutdown!
		// Is this file finished?
		if (!m_PartsHashing && gaplist.IsEmpty() && !m_nTotalBufferData && !m_FlushSetting) //Xman - MORPH - Changed by SiRoB, Flush Thread
		{
			// NEO: POFC - [PauseOnFileComplete]
			if(thePrefs.m_bPauseOnFileComplete)
				StopOnFileComplete();
			else
			// NEO: POFC END
				CompleteFile(false);	// Recheck all hashes, because loaded data is trusted based on file-date
		}
		else if (m_bPauseOnPreview && IsReadyForPreview())
		{
			m_bPauseOnPreview = false;
			PauseFile();
		}
	}
}

CPartHashThread::CPartHashThread(CPartFile* pOwner, uint16 part/* = -1*/, bool ICHused/* = false*/, bool AICHRecover/* = false*/)
: Thread(true, PRIO_LOW)
, m_pOwner(pOwner)
, m_ICHused(ICHused)
, m_AICHRecover(AICHRecover)
, directory(m_pOwner->GetTempPath())
, filename(RemoveFileExtension(m_pOwner->GetPartMetFileName()))
{
	if (!CemuleDlg::IsRunning())	// Don't start any last-minute hashing
		return;	// Hash next start

	if(part != -1) // SetSinglePartHash
				{
		if (part >= m_pOwner->GetPartCount()) {	// Out of bounds, no point in even trying
			if (AICHRecover)
				PostMessage(theApp.emuledlg->m_hWnd,TM_PARTHASHEDCORRUPTAICHRECOVER,part,(LPARAM)m_pOwner);
			else if (!ICHused)		// ICH only sends successes
				PostMessage(theApp.emuledlg->m_hWnd,TM_PARTHASHEDCORRUPT,part,(LPARAM)m_pOwner);
				}
				else
			m_PartsToHash.Add(part);
			}
	else // SetFirstHash
			{
		for (uint16 i = 0; i < m_pOwner->GetPartCount(); i++)
				{
			//MORPH - Changed by SiRoB, Need to check buffereddata otherwise we may try to hash wrong part
			/*
			if (m_pOwner->IsComplete((uint64)i*PARTSIZE,(uint64)(i+1)*PARTSIZE-1, false)){
			*/
			if (m_pOwner->IsComplete((uint64)i*PARTSIZE,(uint64)(i+1)*PARTSIZE-1, true)){
			m_PartsToHash.Add(i);
		}
		}
	}
	m_pOwner->m_PartsHashing += m_PartsToHash.GetCount();
}


void CPartHashThread::run()
{
	DbgSetThreadName("PartHashThread");

	// SLUGFILLER: SafeHash
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return;
	// SLUGFILLER: SafeHash

	//InitThreadLocale(); //Performance killer
	CFile file;
	Poco::FastMutex::SingleLock sLock(m_pOwner->ICH_mut, m_ICHused); // ICH locks the file
	if (file.Open(directory+_T('\\')+filename,CFile::modeRead|CFile::osSequentialScan|CFile::shareDenyNone)){
		for (size_t i = 0; i < m_PartsToHash.GetCount(); i++){
			uint16 partnumber = m_PartsToHash[i];
			file.Seek((LONGLONG)PARTSIZE*partnumber, CSafeFile::begin);
			uint64 length = PARTSIZE;
			if ((ULONGLONG)PARTSIZE*(partnumber+1) > file.GetLength()){
				length = (file.GetLength() - ((ULONGLONG)PARTSIZE*partnumber));
				ASSERT( length <= PARTSIZE );
	}

	//AICH
	CAICHHash cur_AICH_hash;
	CAICHHashTree* phtAICHPartHash = NULL;
			if (m_pOwner->m_FileIdentifier.HasAICHHash() && m_pOwner->m_FileIdentifier.HasExpectedAICHHashCount())
	{
				const CAICHHashTree* pPartTree = m_pOwner->m_pAICHRecoveryHashSet->FindPartHash((uint16)partnumber);
		if (pPartTree != NULL)
		{
			// use a new part tree, so we don't overwrite any existing recovery data which we might still need lateron
			phtAICHPartHash = new CAICHHashTree(pPartTree->m_nDataSize,pPartTree->m_bIsLeftBranch, pPartTree->GetBaseSize());	
		}
		else
			ASSERT( false );

				if (m_pOwner->GetPartCount() > 1)
		{
					if (m_pOwner->m_FileIdentifier.GetAvailableAICHPartHashCount() > partnumber)
						cur_AICH_hash = m_pOwner->m_FileIdentifier.GetRawAICHHashSet()[partnumber];
			else
				ASSERT( false );
		}
		else
					cur_AICH_hash = m_pOwner->m_FileIdentifier.GetAICHHash();
}

			//MORPH - Changed by SiRoB, avoid crash if the file has been canceled
				uchar hashresult[16];

				try
				{
					m_pOwner->CreateHash(&file, length, hashresult, phtAICHPartHash);
					if (!CemuleDlg::IsRunning())	// in case of shutdown while still hashing
				{
					delete phtAICHPartHash;
						break;
				}
			}
				catch(CFileException* ex)
				{
					ex->Delete();
				delete phtAICHPartHash;
					if (!CemuleDlg::IsRunning())	// in case of shutdown while still hashing
						break;
					continue;
				}

			//MD4
			const uchar* cur_hash = NULL;
			if (m_pOwner->m_FileIdentifier.HasExpectedMD4HashCount())
			{
				if (m_pOwner->GetPartCount() > 1 || m_pOwner->GetFileSize()== (uint64)PARTSIZE)
				{
					if (m_pOwner->GetFileIdentifier().GetAvailableMD4PartHashCount() > partnumber)
						cur_hash = m_pOwner->GetFileIdentifier().GetMD4PartHash(partnumber);
					else
					{
						ASSERT( false );
					}
				}
				else
					cur_hash = m_pOwner->GetFileIdentifier().GetMD4Hash();
			}

				bool bMD4Error = false;
				bool bMD4Checked = false;
			if (cur_hash != NULL)
				{
					bMD4Checked = true;
				bMD4Error = md4cmp(hashresult, cur_hash) != 0;
				}
				else
				{
					DebugLogError(_T("MD4 HashSet not present while veryfing part %u for file %s"), partnumber, m_pOwner->GetFileName());
					m_pOwner->m_bMD4HashsetNeeded = true;
				}

				//AICH
				//Note: If phtAICHPartHash is NULL it will remain NULL while hashing using CreateHash. So if we should not check by AICH it
				//      is going to be NULL and thus the below will not be executed.
			bool bAICHError = false;
			bool bAICHChecked = false;
				if (phtAICHPartHash != NULL)
				{
					ASSERT( phtAICHPartHash->m_bHashValid );
					bAICHChecked = true;
				bAICHError = cur_AICH_hash != phtAICHPartHash->m_Hash;
				delete phtAICHPartHash;
				}
				//else
				//	DebugLogWarning(_T("AICH HashSet not present while verifying part %u for file %s"), partnumber, m_pOwner->GetFileName());

			bool pbAICHReportedOK = false;
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
		if (m_ICHused)
			sLock.Unlock();
	}
// END SiRoB, SLUGFILLER: SafeHash

void CPartFile::ClearClient(){// X: [C0SC] - [Clear0SpeedClient]
	for(POSITION pos = m_downloadingSourceList.GetHeadPosition(); pos != NULL;){
		CUpDownClient* cur_src = m_downloadingSourceList.GetNext(pos);
		//zz_fly :: delayed deletion of downloading source :: Enig123 :: Start
		if (m_sourceListChange && m_downloadingDeleteList.Find(cur_src))
			continue;
		//zz_fly :: delayed deletion of downloading source :: Enig123 :: End
		if(cur_src->GetDownloadDatarate10()<=0){
			if(socket!=NULL)
				cur_src->SendCancelTransfer();
			cur_src->SetDownloadState(DS_ONQUEUE, _T("ClearClient"), CUpDownClient::DSR_PAUSED); // Maella -Download Stop Reason-
		}
	}
}

FileFmt_Struct*CPartFile::GetFileFormat() {// X: [FV] - [FileVerify]
	if (formatInfo)
		return formatInfo;

	uint64 completedHeader=GetHeaderSizeOnDisk();
	if(completedHeaderX==completedHeader||completedHeader<HEADERVERIFYSIZE)
		return NULL;
	completedHeaderX=completedHeader;
	formatInfo = theApp.fileverify->CheckFileFormat(this, (uint_ptr)completedHeader);
	return formatInfo;
}

bool CPartFile::CanDropClient(){// X: [DSC] - [Drop Slow Client]
	if(leftsize > EMBLOCKSIZE*m_anStates[DS_DOWNLOADING])
		return false;
	if(m_iDownPriority==PR_LOW){
		m_bAutoDownPriority = false;
		m_iDownPriority=PR_NORMAL;
	}
	return true;
}

bool CPartFile::DropSlowClient(CUpDownClient* client, uint_ptr DownloadDatarate10) const{// X: [DSC] - [Drop Slow Client]
	CUpDownClient*dropclient = NULL;
	for(POSITION pos = m_downloadingSourceList.GetHeadPosition(); pos != NULL;){
		CUpDownClient* cur_src = m_downloadingSourceList.GetNext(pos);
		//zz_fly :: delayed deletion of downloading source :: Enig123 :: Start
		if (m_sourceListChange && m_downloadingDeleteList.Find(cur_src))
			continue;
		//zz_fly :: delayed deletion of downloading source :: Enig123 :: End

		if(cur_src->GetRemainingBlocksToDownload() !=0 && client->IsPartAvailable(cur_src->m_lastPartAsked)){
			if(cur_src->GetDownloadDatarate10() == 0){
				cur_src->StopClient();
				if(cur_src != client)
					return true;
				continue;
			}
			if(dropclient == NULL){
				if(DownloadDatarate10==0 || cur_src->GetDownloadDatarate10()<DownloadDatarate10)
					dropclient = cur_src;
			}
			else if(cur_src->GetDownloadDatarate10()<dropclient->GetDownloadDatarate10())
				dropclient = cur_src;
		}
	}
	if(dropclient && dropclient != client && dropclient->GetDownloadDatarate10()*2<GetDownloadDatarate10()){
		dropclient->StopClient();
		dropclient->SwapToAnotherFile(false, false, false, NULL,true);
		return true;
	}
	return false;
}

bool CPartFile::GetNextRequestedBlock(CUpDownClient* sender, 
									  Requested_Block_Struct** newblocks, 
									  uint16* count) /*const*/
{
	// The purpose of this function is to return a list of blocks (~180KB) to
	// download. To avoid a prematurely stop of the downloading, all blocks that 
	// are requested from the same source must be located within the same 
	// chunk (=> part ~9MB).
	//  
	// The selection of the chunk to download is one of the CRITICAL parts of the 
	// edonkey network. The selection algorithm must insure the best spreading
	// of files.
	//Maella & zz
	// The selection is based on 4 criteria:
	//  1.  Frequency of the chunk (availability), very rare chunks must be downloaded 
	//      as quickly as possible to become a new available source.
	//  2.  Parts used for preview (first + last chunk), preview or check a 
	//      file (e.g. movie, mp3)
	//  3.  Request state (downloading in process), try to ask each source for another 
	//      chunk. Spread the requests between all sources.
	//  4.  Completion (shortest-to-complete), partially retrieved chunks should be 
	//      completed before starting to download other one.
	//  
	// The frequency criterion defines three zones: very rare (<10%), rare (<50%)
	// and common (>30%). Inside each zone, the criteria have a specific ‘weight? used 
	// to calculate the priority of chunks. The chunk(s) with the highest 
	// priority (highest=0, lowest=0xffff) is/are selected first.
	//  
	//          very rare   (preview)       rare                      common
	//    0% <---- +0 pt ----> 10% <----- +10000 pt -----> 50% <---- +20000 pt ----> 100%
	// 1.  <------- frequency: +25*frequency pt ----------->
	// 2.  <- preview: +1 pt --><-------------- preview: set to 10000 pt ------------->
	// 3.                       <------ request: download in progress +20000 pt ------>
	// 4a. <- completion: 0% +100, 25% +75 .. 100% +0 pt --><-- !req => completion --->
	// 4b.                                                  <--- req => !completion -->
	//  
	// Unrolled, the priority scale is:
	//  
	// 0..xxxx       unrequested and requested very rare chunks
	// 10000..1xxxx  unrequested rare chunks + unrequested preview chunks
	// 20000..2xxxx  unrequested common chunks (priority to the most complete)
	// 30000..3xxxx  requested rare chunks + requested preview chunks
	// 40000..4xxxx  requested common chunks (priority to the least complete)
	//
	// This algorithm usually selects first the rarest chunk(s). However, partially
	// complete chunk(s) that is/are close to completion may overtake the priority 
	// (priority inversion).
	//Maella
	//For the common chuncks, the algorithm tries to spread the dowload between
	// the sources
	//zz
	//For common chunks, it also tries to put the transferring
	// clients on the same chunk, to complete it sooner.

	// Check input parameters
	if(count == 0||sender->GetPartStatus() == NULL)
		return false;

	//AddDebugLogLine(DLP_VERYLOW, false, _T("Evaluating chunks for file: \"%s\" Client: %s"), GetFileName(), sender->DbgGetClientInfo());
	// Define and create the list of the chunks to download

	UINT cc = thePrefs.m_chunkchooser;
	uint16 tempLastPartAsked = (uint16)-1;
	if((sender->m_lastPartAsked != ((uint16)-1) && (cc==1/*Maella*/ ||sender->GetClientSoft() == SO_EMULE && sender->GetVersion() < MAKE_CLIENT_VERSION(0, 43, 1))))
		tempLastPartAsked = sender->m_lastPartAsked;
	CAtlList<Chunk> chunksList(GetPartCount());

	//Xman Dynamic block request (netfinity/Xman)
	//uint16 countin=*count; //Xman for debug
	uint64	bytesPerRequest = EMBLOCKSIZE;
	//Xman 5.4.2: removed the faster chunk completion because: 
	//e.g. downloading 2 different chunks from 2 different sources... there is no need 
	//to reduce the blocksize for the slow source. It would only make sense if we know
	//that both sources are downloading the same chunk ==>would need much more calculation
	//also reducing the blocksize means more stress for the uploader. Only do it at the end of the download!
	uint32	fileDatarate = max(GetDownloadDatarate10(), UPLOAD_CLIENT_DATARATE); // Always assume file is being downloaded at at least 3072 B/s 
	uint32	sourceDatarate = max(sender->GetDownloadDatarate10(), 22); // Always assume client is uploading at at least 25 B/s //Xman changed from 10
	uint_ptr	timeToFileCompletion = (uint_ptr) (leftsize / fileDatarate) + 1;
	if(timeToFileCompletion < 10)
		timeToFileCompletion = 10; // Always assume it will take at least 10 seconds to complete
	bool	is_slow_source=false; 

	bytesPerRequest = (sourceDatarate * timeToFileCompletion) >>1;

	//uint64 bytesPerRequest_total=bytesPerRequest; //Xman for debug

	uint_ptr blockstorequest;
	if (bytesPerRequest > EMBLOCKSIZE) {
		blockstorequest = (uint_ptr)(bytesPerRequest/EMBLOCKSIZE);
		bytesPerRequest = EMBLOCKSIZE;
	}
	else
		blockstorequest = 1;

	uint16 remainingblocks = sender->GetRemainingBlocksToDownload();
	if(remainingblocks >= blockstorequest)
		return false;

	*count = (uint16)min(blockstorequest - remainingblocks , *count);

	if (bytesPerRequest < 10240)
	{
		// Let an other client request this packet if we are close to completion and source is slow
		// Use the true file datarate here, otherwise we might get stuck in NNP state
		if (GetDownloadDatarate10()>0)
		{
			uint_ptr realtimeToFileCompletion=(uint_ptr)(leftsize/GetDownloadDatarate10());

			if (!requestedblocks_list.IsEmpty() && realtimeToFileCompletion < 30 && bytesPerRequest < 3400 /* && 5 * sourceDatarate < GetDownloadDatarate10()*/)
			{
				is_slow_source=true;
			}
		}
		bytesPerRequest = 10240;
	}

	if(sender->GetDownTimeDifference(false)< 10001 && leftsize > 1024*1024 && bytesPerRequest <= 10240)  //we need 10 seconds until a proper Speed measurement
	{
		bytesPerRequest = 5*10240; //don't let the first request become too short !!
	}


	//Xman for debug
	//AddDebugLogLine(false, _T("DBR: Trying to request %u blocks(%u) of size(%u): %u for client:%s, file: %s"), *count, countin, (uint32)bytesPerRequest,(uint32)bytesPerRequest_total,  sender->DbgGetClientInfo(), GetFileName());
	//AddDebugLogLine(false, _T("DBR+: bytesLeftToDownload:%u, timeToFileCompletion:%u, fileDatarate: %u, sourceDatarate=%u(%u), pending:%u"), (uint32)leftsize, timeToFileCompletion, fileDatarate, sourceDatarate,sender->GetDownloadDatarate(), sender->GetPendingBlockCount());

	//prevent an endless loop
	bool chunklist_initialized=false;

	//Xman end

	// Main loop
	SFMT&rng = *t_rng;
	uint16 lastAddPart = (uint16)-1;
	uint16 newBlockCount = 0;
	while(newBlockCount != *count){
		// Create a request block stucture if a chunk has been previously selected
		if(tempLastPartAsked != (uint16)-1){
			//Xman Dynamic block request
			//a slow source could be dropped when there are fast sources and the file is near to completion
			//but it could happen that we drop a slow source, although it is the only one which has the last part needed
			//now I drop only if there are no other (and faster) clients uploading the wanted chunk
			if (lastAddPart != tempLastPartAsked && is_slow_source && 5 * sourceDatarate < GetDownloadSpeedInPart(tempLastPartAsked,sender) )
			{
				DebugLog(_T("No request block given as source is slow and chunk near completion! -->trying an other chunk"));
				//return false;
			}
			else
			{
				//Xman end

				Requested_Block_Struct* pBlock = new Requested_Block_Struct;
				if(GetNextEmptyBlockInPart(tempLastPartAsked, pBlock, bytesPerRequest) == true){ //Xman Dynamic block request (netfinity/Xman)
					// Keep a track of all pending requested blocks
					requestedblocks_list.AddTail(pBlock);
					// Update list of blocks to return
					newblocks[newBlockCount++] = pBlock;
					// Skip end of loop (=> CPU load)
					lastAddPart=tempLastPartAsked;
					continue;
				} 
				// All blocks for this chunk have been already requested
				delete pBlock;
			}//Xman Dynamic block request
			tempLastPartAsked = (uint16)-1;
			// => Try to select another chunk
		}

		// Check if a new chunk must be selected (e.g. download starting, previous chunk complete)
		// Quantify all chunks (create list of chunks to download) 
		// This is done only one time and only if it is necessary (=> CPU load)
		if(chunksList.IsEmpty() == TRUE && chunklist_initialized==false){ //Xman Dynamic block request
			// Indentify the locally missing part(s) that this source has
				for(uint16 i = (uint16)(headerSize/PARTSIZE); i < GetPartCount(); i++){
					if(sender->IsPartAvailable(i) == true && GetNextEmptyBlockInPart(i, NULL) == true){
						// Create a new entry for this chunk and add it to the list
						Chunk newEntry;
						newEntry.part = i;
						newEntry.frequency = m_SrcpartFrequency[i];
						chunksList.AddTail(newEntry);
					}
				}
				chunklist_initialized = true; //Xman Dynamic block request

				// Check if any block(s) could be downloaded
				if(chunksList.IsEmpty() == TRUE)
					break; // Exit main loop while()

				if(cc==1/*Maella*/ ){
					// Define the bounds of the three zones (very rare, rare)
					// more depending on available sources
					uint8 modif=10;
					if (GetSourceCount()>800) modif=2; else if (GetSourceCount()>200) modif=5;
					//Xman better chunk selection
					uint16 limit= (uint16)ceil((float)modif*GetSourceCount()/ 100) + 1; //Xman: better if we have very low sources
					//if (limit==0) limit=1;

					const uint16 veryRareBound = limit;
				// X-Ray :: ShiftTweak :: Start
				/*
					const uint16 rareBound = 2*limit;
				*/
				const uint16 rareBound = limit << 1;
				// X-Ray :: ShiftTweak :: End

					// Cache Preview state (Criterion 2)
					const bool isPreviewEnable = (thePrefs.GetPreviewPrio() || thePrefs.IsExtControlsEnabled() && GetPreviewPrio()) && IsPreviewableFileType();
					
					// Collect and calculate criteria for all chunks
					for(POSITION pos = chunksList.GetHeadPosition(); pos != NULL; ){
						Chunk& cur_chunk = chunksList.GetNext(pos);

					if(xState != PFS_NORMAL && corrupted_list.Find(cur_chunk.part)){ // X: [IPR] - [Improved Part Recovery]
						cur_chunk.rank = 65535;
						continue;
					}
						// Offsets of chunk
						const uint64 uStart = (uint64)cur_chunk.part * PARTSIZE;
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
							else if(cur_chunk.part == GetPartCount()-1){
								critPreview = true; // Last chunk 
							}
							else if(cur_chunk.part == GetPartCount()-2){
								// Last chunk - 1 (only if last chunk is too small)
								if( (GetFileSize() - uEnd) < (uint64)PARTSIZE/3){
									critPreview = true; // Last chunk - 1
								}
							}
						}
						
						// Criterion 3. Request state (downloading in process from other source(s))
						const bool critRequested = cur_chunk.frequency > veryRareBound &&  // => CPU load
							IsAlreadyRequested(uStart, uEnd);

						// Criterion 4. Completion
						uint64 partSize = PARTSIZE;
						for(POSITION pos = gaplist.GetHeadPosition(); pos != NULL; ) {
							const Gap_Struct* cur_gap = gaplist.GetNext(pos);
							// Check if Gap is into the limit
							if(cur_gap->start < uStart) {
								if(cur_gap->end > uStart && cur_gap->end < uEnd) {
									partSize -= cur_gap->end - uStart + 1;
								}
								else if(cur_gap->end >= uEnd) {
									partSize = 0;
									break; // exit loop for()
								}
							}
							else if(cur_gap->start <= uEnd) {
								if(cur_gap->end < uEnd) {
									partSize -= cur_gap->end - cur_gap->start + 1;
								}
								else {
									partSize -= uEnd - cur_gap->start + 1;
								}
							}
						}
						const uint16 critCompletion = (uint16)(partSize/(PARTSIZE/100)); // in [%]

						// Calculate priority with all criteria
						if(cur_chunk.frequency <= veryRareBound){
							// 0..xxxx unrequested + requested very rare chunks
							cur_chunk.rank = (25 * cur_chunk.frequency) +      // Criterion 1
								((critPreview == true) ? 0 : 1) + // Criterion 2
								(100 - critCompletion);           // Criterion 4
						}
						else if(critPreview == true){
							// 10000..10100  unrequested preview chunks
							// 30000..30100  requested preview chunks
							cur_chunk.rank = ((critRequested == false) ? 10000 : 30000) + // Criterion 3
								(100 - critCompletion);           // Criterion 4
						}
						else if(cur_chunk.frequency <= rareBound){
							// 10101..1xxxx  unrequested rare chunks
							// 30101..3xxxx  requested rare chunks
							cur_chunk.rank = (25 * cur_chunk.frequency) +                 // Criterion 1 
								((critRequested == false) ? 10101 : 30101) + // Criterion 3
								(100 - critCompletion);                      // Criterion 4
						}
						else { // common chunk
							if(critRequested == false){ // Criterion 3
								// 20000..2xxxx  unrequested common chunks
								cur_chunk.rank = 20000 +                // Criterion 3
									(100 - critCompletion); // Criterion 4
							}
							else{
								// 40000..4xxxx  requested common chunks
								// Remark: The weight of the completion criterion is inversed
								//         to spead the requests over the completing chunks. 
								//         Without this, the chunk closest to completion will  
								//         received every new sources.
								cur_chunk.rank = 40000 +                // Criterion 3
									(critCompletion);       // Criterion 4				
							}
						}
					}
				}
				else{// if(cc==2/*zz*/){
					// Define the bounds of the three zones (very rare, rare)
					// more depending on available sources
					//uint16 limit = (uint16)ceil(GetSourceCount()/ 10.0);
					uint16 limit = (uint16)((GetSourceCount() + 9)/10);//Enig123
					if (limit<3) limit=3;

					const uint16 veryRareBound = limit;

				// X-Ray :: ShiftTweak :: Start
				/*
					const uint16 rareBound = 2*limit;
					const uint16 almostRareBound = 4*limit;
				*/
				const uint16 rareBound = limit << 1;
				const uint16 almostRareBound = limit << 2;
				// X-Ray :: ShiftTweak :: End

					// Cache Preview state (Criterion 2)
					const bool isPreviewEnable = (thePrefs.GetPreviewPrio() || thePrefs.IsExtControlsEnabled() && GetPreviewPrio()) && IsPreviewableFileType();
					
					// Collect and calculate criteria for all chunks
					for(POSITION pos = chunksList.GetHeadPosition(); pos != NULL; ){
						Chunk& cur_chunk = chunksList.GetNext(pos);

					if(xState != PFS_NORMAL && corrupted_list.Find(cur_chunk.part)){ // X: [IPR] - [Improved Part Recovery]
						cur_chunk.rank = 65535;
						continue;
					}
						// Offsets of chunk
						const uint64 uStart = (uint64)cur_chunk.part * PARTSIZE;
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
							else if(cur_chunk.part == GetPartCount()-1){
								critPreview = true; // Last chunk 
							}
							else if(cur_chunk.part == GetPartCount()-2){
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

						//uint16 critCompletion = (uint16)ceil((double)(partSize*100)/PARTSIZE); // in [%]. Last chunk is always counted as a full size chunk, to not give it any advantage in this comparison due to smaller size. So a 1/3 of PARTSIZE downloaded in last chunk will give 33% even if there's just one more byte do download to complete the chunk.
						uint16 critCompletion = (uint16)((partSize*100 + PARTSIZE - 1)/PARTSIZE); // X: [CI] - [Code Improvement]
						if(critCompletion > 100) critCompletion = 100;

						// Criterion 5. Prefer to continue the same chunk
						const bool sameChunk = (cur_chunk.part == tempLastPartAsked);

						// Criterion 6. The more transferring clients that has this part, the better (i.e. lower).
						//zz_fly :: delayed deletion of downloading source :: Enig123 :: Start
						/*
					uint16 transferringClientsScore = (uint16)m_downloadingSourceList.GetCount();
						*/
					uint16 transferringClientsScore = (uint16)(m_downloadingSourceList.GetCount() - m_downloadingDeleteList.GetCount());
						//zz_fly :: delayed deletion of downloading source :: Enig123 :: End

						// Criterion 7. Sooner to completion (how much of a part is completed, how fast can be transferred to this part, if all currently transferring clients with this part are put on it. Lower is better.)
						uint16 bandwidthScore = 2000;

						// Calculate criterion 6 and 7
					if(transferringClientsScore/*m_downloadingSourceList.GetCount()*/ > 1) {
							UINT totalDownloadDatarateForThisPart = 1;
							for(POSITION downloadingClientPos = m_downloadingSourceList.GetHeadPosition(); downloadingClientPos != NULL; ) {
								/*const */CUpDownClient* downloadingClient = m_downloadingSourceList.GetNext(downloadingClientPos);
								//zz_fly :: delayed deletion of downloading source :: Enig123 :: Start
								if (m_sourceListChange && m_downloadingDeleteList.Find(downloadingClient))
									continue;
								//zz_fly :: delayed deletion of downloading source :: Enig123 :: End
								if(downloadingClient->IsPartAvailable(cur_chunk.part)) {
									transferringClientsScore--;
									totalDownloadDatarateForThisPart += downloadingClient->GetDownloadDatarate10()/*Xman*/ + 500; // + 500 to make sure that a unstarted chunk available at two clients will end up just barely below 2000 (max limit)
								}
							}

						bandwidthScore = (uint16)min((uint_ptr)((PARTSIZE-partSize)/(totalDownloadDatarateForThisPart*5)), 2000);
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
						uint16 randomAdd = 1 + (uint16)((((uint32)rng.getUInt16()*(almostRareBound-rareBound))+(RAND16_MAX/2))/RAND16_MAX);
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
			}

		// Select the next chunk to download
		if(chunksList.IsEmpty() == FALSE){
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
			uint16 randomness = 1 + (uint16)((((uint32)rng.getUInt16()*(count-1))+(RAND16_MAX/2))/RAND16_MAX);
				for(POSITION pos = chunksList.GetHeadPosition(); ; ){
					POSITION cur_pos = pos;
					const Chunk& cur_chunk = chunksList.GetNext(pos);
					if(cur_chunk.rank == rank){
						randomness--; 
						if(randomness == 0){
							// Selection process is over 
							tempLastPartAsked = cur_chunk.part;
							//AddDebugLogLine(DLP_VERYLOW, false, _T("Chunk number %i selected. Rank: %u"), cur_chunk.part, cur_chunk.rank);

							// Remark: this list might be reused up to ?count?times
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
	sender->m_lastPartAsked = tempLastPartAsked;
	// Return the number of the blocks 
	*count = newBlockCount;

	// Return
	return (newBlockCount > 0);
}

void CPartFile::SetAutoCat(){
	if(thePrefs.GetCatCount()==1)
		return;
	CString catExt;

	for (size_t ix = 1;ix < thePrefs.GetCatCount();ix++){	
		catExt= thePrefs.GetCategory(ix)->autocat;
		if (catExt.IsEmpty())
			continue;

		if (!thePrefs.GetCategory(ix)->ac_regexpeval) {
			// simple string comparison

			int curPos = 0;
			catExt.MakeLower();

			CString fullname = GetFileName();
			fullname.MakeLower();
			CString cmpExt = catExt.Tokenize(_T("|"), curPos);

			while (!cmpExt.IsEmpty()) {
				// HoaX_69: Allow wildcards in autocat string
				//  thanks to: bluecow, khaos and SlugFiller
				if(cmpExt.Find(_T('*')) != -1 || cmpExt.Find(_T('?')) != -1){
					// Use wildcards
					if(PathMatchSpec(fullname, cmpExt)){
						m_category = ix;
						return;
					}
				}else{
					if(fullname.Find(cmpExt) != -1){
						m_category = ix;
						return;
					}
				}
				cmpExt = catExt.Tokenize(_T("|"),curPos);
			}
		} else {
			// regular expression evaluation
			if (RegularExpressionMatch(catExt,GetFileName())){
				m_category = ix;
				return; //Enig123:: Avi3k: fix cat assign
			}
		}
	}
}

//zz_fly :: delayed deletion of downloading source :: Enig123 :: Start
void CPartFile::DoDelayedDeletion()
{
	if(!m_sourceListChange)
		return;
	m_sourceListChange = false;
	ASSERT ( !m_downloadingSourceList.IsEmpty());

	for(POSITION pos = m_downloadingDeleteList.GetHeadPosition(); pos!=0; )
	{
		POSITION posLast = pos;
		CUpDownClient* cur_src = m_downloadingDeleteList.GetNext(pos);

		POSITION pos2 = m_downloadingSourceList.Find(cur_src);
		if (pos2) {
			m_downloadingSourceList.RemoveAt(pos2);
			m_downloadingDeleteList.RemoveAt(posLast);
		}
	}
}
//zz_fly :: delayed deletion of downloading source :: Enig123 :: End

// Status Icons :: Start
EPartFileStatus CPartFile::GetPartFileStatus()
{
	if (GetStatus() == PS_COMPLETING)
		    return PS_COMPLETING;
	else if (xState == POFC_WAITING)
		    return PS_TICK;
	else if(GetStatus() == PS_PAUSED)
	{
		if(IsStopped())
			return PS_STOPPED;
		return PS_PAUSED;
	}
	else if(GetStatus() != PS_READY && GetStatus() != PS_EMPTY && GetStatus() != PS_INSUFFICIENT && GetStatus() != PS_UNKNOWN)
		return GetStatus();
	else if(GetTransferringSrcCount() > 0)
		return PS_DOWNLOADING;
	else
		return PS_WAITINGFORSOURCE;
}
// Status Icons :: End