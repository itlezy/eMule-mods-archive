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
#include "DownloadQueue.h"
#include "UpDownClient.h"
#include "PartFile.h"
#include "ed2kLink.h"
#include "SearchFile.h"
#include "ClientList.h"
#include "Statistics.h"
#include "SharedFileList.h"
#include "OtherFunctions.h"
#include "SafeFile.h"
#include "Sockets.h"
#include "ServerList.h"
#include "Server.h"
#include "Packets.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "kademlia/utils/uint128.h"
#include "ipfilter.h"
#include "emuledlg.h"
#include "TransferDlg.h"
#include "TaskbarNotifier.h"
#include "MenuCmds.h"
#include "Log.h"
//Xman
#include "BandWidthControl.h"
#include "ListenSocket.h"
//Xman end

#include ".\MiniMule\SystemInfo.h" // CPU/MEM usage [$ick$/Stulle] - Max 
#include "UploadQueue.h" // Do not restrict download if no upload possible [Stulle] - Stulle
#include "FriendList.h" // Multiple friendslots [ZZ] - Mephisto

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DWORD quicktime; // Quick start [TPT] - Max

CDownloadQueue::CDownloadQueue()
{
	filesrdy = 0;
	//Xman
	/*
	datarate = 0;
	*/
	//Xman end
	cur_udpserver = 0;
	lastfile = 0;
	lastcheckdiskspacetime = 0;
	lastudpsearchtime = 0;
	lastudpstattime = 0;
	SetLastKademliaFileRequest();
	udcounter = 0;
	m_iSearchedServers = 0;
	//Xman
	/*
	m_datarateMS=0;
	*/
	//Xman end
	m_nUDPFileReasks = 0;
	m_nFailedUDPFileReasks = 0;
	m_dwNextTCPSrcReq = 0;
	m_cRequestsSentToServer = 0;
	//Xman Xtreme Mod
	m_TCPFileReask=0;
	m_FailedTCPFileReask=0;
	//Xman end

	// Maella -Overhead compensation (pseudo full download rate control)-
	m_lastProcessTime = ::GetTickCount();
	m_lastOverallReceivedBytes = 0;
	m_lastReceivedBytes = 0;
	m_nDownloadSlopeControl = 0;
	// Maella end

	//Xman askfordownload priority
	//m_toomanytimestamp=0;
	m_maxdownprio=0;
	m_maxdownprionew=0;
	//Xman end
	
	//Xman GlobalMaxHarlimit for fairness
	m_uGlobsources=0;
	m_limitstate=0;
	m_limitratio=0; // Enforce Ratio [Stulle] - Stulle

	//Xman
	/*
    m_dwLastA4AFtime = 0; // ZZ:DownloadManager
	*/
	//Xman end

	// ==> Quick start [TPT] - Max
	quickflag = 0;
	quickflags = 0;
	// <== Quick start [TPT] - Max

	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	m_iLastLinkQueuedTick = 0;
	m_bBusyPurgingLinks = false;
	m_ED2KLinkQueue.RemoveAll();
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

	// ==> File Settings [sivka/Stulle] - Stulle
	m_SaveSettingsThread = NULL;
	m_bSaveAgain = false;
	m_dwLastSave = 0;
	// <== File Settings [sivka/Stulle] - Stulle
}

void CDownloadQueue::AddPartFilesToShare()
{
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
	{
		CPartFile* cur_file = filelist.GetNext(pos);
		if (cur_file->GetStatus(true) == PS_READY)
			theApp.sharedfiles->SafeAddKFile(cur_file, true);
	}
}

void CDownloadQueue::Init(){

	GlobalHardLimitTemp = 0; // show global HL - Stulle

	// ==> Global Source Limit [Max/Stulle] - Stulle
	m_dwUpdateHL = ::GetTickCount();
	m_dwUpdateHlTime = 50000; // 50 sec on startup
	m_bPassiveMode = false;
	m_bGlobalHLSrcReqAllowed = true;
	// <== Global Source Limit [Max/Stulle] - Stulle

	m_dwResTimer = ::GetTickCount(); // CPU/MEM usage [$ick$/Stulle] - Max

	// find all part files, read & hash them if needed and store into a list
	CFileFind ff;
	int count = 0;

	for (int i=0;i<thePrefs.tempdir.GetCount();i++) {
		//Xman
		CStringList metsfound;	// SLUGFILLER: SafeHash - ensure each met is loaded once per tempdir
		bool allMetsSuccess = true; //zz_fly :: better .part.met file backup and recovery :: Enig123
		CString searchPath=thePrefs.GetTempDir(i);

		searchPath += _T("\\*.part.met");

		//check all part.met files
		bool end = !ff.FindFile(searchPath, 0);
		while (!end){
			end = !ff.FindNextFile();
			if (ff.IsDirectory())
				continue;

			// BEGIN SLUGFILLER: SafeHash - one is enough
			if (metsfound.Find(CString(ff.GetFileName()).MakeLower()))
				continue;
			//MORPH START - Moved Down, to allow checking for backup met files.
			/*
			metsfound.AddTail(CString(ff.GetFileName()).MakeLower());
			*/
			//MORPH END   - Moved Down, to allow checking for backup met files.
			// END SLUGFILLER: SafeHash
			CPartFile* toadd = new CPartFile();
			EPartFileLoadResult eResult = toadd->LoadPartFile(thePrefs.GetTempDir(i), ff.GetFileName());
			if (eResult == PLR_FAILED_METFILE_CORRUPT)
			{
				// .met file is corrupted, try to load the latest backup of this file
				delete toadd;
				toadd = new CPartFile();
				//zz_fly :: better .part.met file backup and recovery :: Enig123 :: Start
				//note: .backup is newer than .bak, if it is valid. we load .backup first. see CPartFile::SavePartFile() for details.
				/*
				eResult = toadd->LoadPartFile(thePrefs.GetTempDir(i), ff.GetFileName() + PARTMET_BAK_EXT);
				*/
				eResult = toadd->LoadPartFile(thePrefs.GetTempDir(i), ff.GetFileName() + PARTMET_TMP_EXT);
				//zz_fly :: better .part.met file backup and recovery :: Enig123 :: End
				if (eResult == PLR_LOADSUCCESS)
				{
					toadd->SavePartFile(true); // don't override our just used .bak file yet
					AddLogLine(false, GetResString(IDS_RECOVERED_PARTMET), toadd->GetFileName());
				}
			}

			if (eResult == PLR_LOADSUCCESS)
			{
				metsfound.AddTail(CString(ff.GetFileName()).MakeLower()); //MORPH - Added, fix SafeHash
				count++;
				filelist.AddTail(toadd);			// to downloadqueue
				// SLUGFILLER: SafeHash remove - part files are shared later
				/*
				if (toadd->GetStatus(true) == PS_READY)
					theApp.sharedfiles->SafeAddKFile(toadd); // part files are always shared files
				*/
				// SLUGFILLER End
				theApp.emuledlg->transferwnd->GetDownloadList()->AddFile(toadd);// show in downloadwindow
			}
			else
			{
				delete toadd;
				allMetsSuccess = false; //zz_fly :: better .part.met file backup and recovery :: Enig123
			}
		}
		ff.Close();

		//zz_fly :: better .part.met file backup and recovery :: Enig123 :: Start
		//  skip recovery step if there's no failed part file load
		if(allMetsSuccess)
			continue;
		//zz_fly :: better .part.met file backup and recovery :: Enig123 :: End

		//try recovering any part.met files
		//zz_fly :: better .part.met file backup and recovery :: Enig123 :: Start
		// .backup failed, load .bak
		/*
		searchPath += _T(".backup");
		*/
		searchPath += PARTMET_BAK_EXT;
		//zz_fly :: better .part.met file backup and recovery :: Enig123 :: Emd
		end = !ff.FindFile(searchPath, 0);
		while (!end){
			end = !ff.FindNextFile();
			if (ff.IsDirectory())
				continue;
			//Xman
			// BEGIN SLUGFILLER: SafeHash - one is enough
			if (metsfound.Find(RemoveFileExtension(CString(ff.GetFileName()).MakeLower())))
				continue;
			//MORPH START - Moved Down, to allow checking for backup met files.
			/*
			metsfound.AddTail(RemoveFileExtension(CString(ff.GetFileName()).MakeLower()));
			*/
			//MORPH END   - Moved Down, to allow checking for backup met files.
			// END SLUGFILLER: SafeHash

			CPartFile* toadd = new CPartFile();
			if (toadd->LoadPartFile(thePrefs.GetTempDir(i), ff.GetFileName()) == PLR_LOADSUCCESS){
				//MORPH START - Added, fix SafeHash
				metsfound.AddTail(RemoveFileExtension(CString(ff.GetFileName()).MakeLower()));
				//MORPH END   - Added, fix SafeHash
				toadd->SavePartFile(true); // resave backup, don't overwrite existing bak files yet
				count++;
				filelist.AddTail(toadd);			// to downloadqueue
				//Xman
				// SLUGFILLER: SafeHash remove - part files are shared later
				/*
				if (toadd->GetStatus(true) == PS_READY)
					theApp.sharedfiles->SafeAddKFile(toadd); // part files are always shared files
				*/
				// SLUGFILLER End
				theApp.emuledlg->transferwnd->GetDownloadList()->AddFile(toadd);// show in downloadwindow

				AddLogLine(false, GetResString(IDS_RECOVERED_PARTMET), toadd->GetFileName());
			}
			else {
				delete toadd;
			}
		}
		ff.Close();
	}
	if(count == 0) {
		AddLogLine(false,GetResString(IDS_NOPARTSFOUND));
	} else {
		AddLogLine(false,GetResString(IDS_FOUNDPARTS),count);
		//zz_fly :: remove useless code :: Enig123
		/*
		SortByPriority();
		*/
		//zz_fly :: remove useless code :: Enig123
		CheckDiskspace();
	}
	m_SettingsSaver.LoadSettings(); // File Settings [sivka/Stulle] - Stulle
	VERIFY( m_srcwnd.CreateEx(0, AfxRegisterWndClass(0), _T("eMule Async DNS Resolve Socket Wnd #2"), WS_OVERLAPPED, 0, 0, 0, 0, NULL, NULL));

	ExportPartMetFilesOverview();
}

CDownloadQueue::~CDownloadQueue(){
	// ==> File Settings [sivka/Stulle] - Stulle
	if (m_SaveSettingsThread) // we just saved something
	{
		m_SaveSettingsThread->EndThread();
		delete m_SaveSettingsThread;
		m_SaveSettingsThread = NULL;
	}
	else // we might have missed something
		(void)m_SettingsSaver.SaveSettings();
	// <== File Settings [sivka/Stulle] - Stulle
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;)
		delete filelist.GetNext(pos);
	m_srcwnd.DestroyWindow(); // just to avoid a MFC warning
}

// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
/*
void CDownloadQueue::AddSearchToDownload(CSearchFile* toadd, uint8 paused, int cat)
*/
// New Param: uint16 useOrder (Def: 0)
void CDownloadQueue::AddSearchToDownload(CSearchFile* toadd, uint8 paused, int cat, uint16 useOrder)
// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
{
	if (toadd->GetFileSize()== (uint64)0 || IsFileExisting(toadd->GetFileHash()))
		return;

	if (toadd->GetFileSize() > OLD_MAX_EMULE_FILE_SIZE && !thePrefs.CanFSHandleLargeFiles(cat)){
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_FSCANTHANDLEFILE));
		return;
	}

	CPartFile* newfile = new CPartFile(toadd,cat);
	if (newfile->GetStatus() == PS_ERROR){
		delete newfile;
		return;
	}

	newfile->SetCatResumeOrder(useOrder); // Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

	if (paused == 2)
		paused = (uint8)thePrefs.AddNewFilesPaused();
	AddDownload(newfile, (paused==1));

	// If the search result is from OP_GLOBSEARCHRES there may also be a source
	if (toadd->GetClientID() && toadd->GetClientPort()){
		CSafeMemFile sources(1+4+2);
		try{
			sources.WriteUInt8(1);
			sources.WriteUInt32(toadd->GetClientID());
			sources.WriteUInt16(toadd->GetClientPort());
			sources.SeekToBegin();
			newfile->AddSources(&sources, toadd->GetClientServerIP(), toadd->GetClientServerPort(), false);
		}
		catch(CFileException* error){
			ASSERT(0);
			error->Delete();
		}
	}

	// Add more sources which were found via global UDP search
	// ==> Make code VS 2005 and VS 2008 ready [MorphXT] - Stulle
	/*
	const CSimpleArray<CSearchFile::SClient>& aClients = toadd->GetClients();
	*/
	const CSimpleArray<CSearchFile::SClient,CSearchFile::CSClientEqualHelper>& aClients = toadd->GetClients();
	// <== Make code VS 2005 and VS 2008 ready [MorphXT] - Stulle
	for (int i = 0; i < aClients.GetSize(); i++){
		CSafeMemFile sources(1+4+2);
		try{
			sources.WriteUInt8(1);
			sources.WriteUInt32(aClients[i].m_nIP);
			sources.WriteUInt16(aClients[i].m_nPort);
			sources.SeekToBegin();
			newfile->AddSources(&sources,aClients[i].m_nServerIP, aClients[i].m_nServerPort, false);
		}
		catch(CFileException* error){
			ASSERT(0);
			error->Delete();
			break;
		}
	}
}

// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
/*
void CDownloadQueue::AddSearchToDownload(CString link, uint8 paused, int cat)
*/
// New Param: uint16 useOrder (Def: 0)
void CDownloadQueue::AddSearchToDownload(CString link,uint8 paused, int cat, uint16 useOrder)
// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
{
	CPartFile* newfile = new CPartFile(link, cat);
	if (newfile->GetStatus() == PS_ERROR){
		delete newfile;
		return;
	}

	newfile->SetCatResumeOrder(useOrder); // Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

	if (paused == 2)
		paused = (uint8)thePrefs.AddNewFilesPaused();
	AddDownload(newfile, (paused==1));
}

void CDownloadQueue::StartNextFileIfPrefs(int cat) {
    if (thePrefs.StartNextFile())
		// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
		/*
		StartNextFile((thePrefs.StartNextFile() > 1?cat:-1), (thePrefs.StartNextFile()!=3));
		*/
	{
		int catTemp = thePrefs.StartNextFile() > 1?cat:-1;
		Category_Struct* cur_cat = thePrefs.GetCategory(cat);
		if (cur_cat && cur_cat->bResumeFileOnlyInSameCat)
			catTemp = cat;
		
		StartNextFile(catTemp, (thePrefs.StartNextFile()!=3));
	}
		// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
}

// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
/*
void CDownloadQueue::StartNextFile(int cat, bool force){
*/
bool CDownloadQueue::StartNextFile(int cat, bool force){
// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

	CPartFile*  pfile = NULL;
	CPartFile* cur_file ;
	POSITION pos;
	
	if (cat != -1) {
		// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
		/*
        // try to find in specified category
		for (pos = filelist.GetHeadPosition();pos != 0;){
			cur_file = filelist.GetNext(pos);
			if (cur_file->GetStatus()==PS_PAUSED &&
                (
				 cur_file->GetCategory()==(UINT)cat || 
				 cat==0 && thePrefs.GetCategory(0)->filter==0 && cur_file->GetCategory()>0
                ) &&
                CPartFile::RightFileHasHigherPrio(pfile, cur_file)
		*/
		Category_Struct* cur_cat = thePrefs.GetCategory(cat);
		force &= !(cur_cat && cur_cat->bResumeFileOnlyInSameCat);
		// try to find in specified category
		for (pos = filelist.GetHeadPosition();pos != 0;){
			cur_file = filelist.GetNext(pos);
			if (cur_file->GetStatus()==PS_PAUSED &&
				cur_file->GetCategory()==(UINT)cat &&
                CPartFile::RightFileHasHigherPrio(pfile, cur_file)
		// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
			   ) {
    			pfile = cur_file;
			}
		}
		if (pfile == NULL && !force)
			// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
			/*
			return;
			*/
			return false;
			// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	}

    if(cat == -1 || pfile == NULL && force) {
	    for (pos = filelist.GetHeadPosition();pos != 0;){
		    cur_file = filelist.GetNext(pos);
		    Category_Struct* cur_cat = thePrefs.GetCategory(cur_file->GetCategory()); // Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
		    if (cur_file->GetStatus() == PS_PAUSED &&
				// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
				/*
                CPartFile::RightFileHasHigherPrio(pfile, cur_file))
				*/
				CPartFile::RightFileHasHigherPrio(pfile, cur_file) && !(cur_cat && cur_cat->bResumeFileOnlyInSameCat))
				// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
		    {
                // pick first found matching file, since they are sorted in prio order with most important file first.
			    pfile = cur_file;
		    }
	    }
    }
	if (pfile) pfile->ResumeFile();
	return pfile!=NULL; // Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
}

// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
/*
void CDownloadQueue::AddFileLinkToDownload(CED2KFileLink* pLink, int cat)
{
	CPartFile* newfile = new CPartFile(pLink, cat);
	if (newfile->GetStatus() == PS_ERROR){
		delete newfile;
		newfile=NULL;
	}
	else {
		AddDownload(newfile,thePrefs.AddNewFilesPaused());
	}

	CPartFile* partfile = newfile;
	if (partfile == NULL)
		partfile = GetFileByID(pLink->GetHashKey());
	if (partfile)
	{
		// match the fileidentifier and only if the are the same add possible sources
		CFileIdentifierSA tmpFileIdent(pLink->GetHashKey(), pLink->GetSize(), pLink->GetAICHHash(), pLink->HasValidAICHHash());
		if (partfile->GetFileIdentifier().CompareRelaxed(tmpFileIdent))
		{
			if (pLink->HasValidSources())
				partfile->AddClientSources(pLink->SourcesList, 1, false);
			if (!partfile->GetFileIdentifier().HasAICHHash() && tmpFileIdent.HasAICHHash())
			{
				partfile->GetFileIdentifier().SetAICHHash(tmpFileIdent.GetAICHHash());
				partfile->GetAICHRecoveryHashSet()->SetMasterHash(tmpFileIdent.GetAICHHash(), AICH_VERIFIED);
				partfile->GetAICHRecoveryHashSet()->FreeHashSet();

			}
			//MORPH START - Added by Stulle, Only resolve hostnames for downloads if partfile found [WiZaRd]
			if (pLink->HasHostnameSources())
			{
				POSITION pos = pLink->m_HostnameSourcesList.GetHeadPosition();
				while (pos != NULL)
				{
					const SUnresolvedHostname* pUnresHost = pLink->m_HostnameSourcesList.GetNext(pos);
					m_srcwnd.AddToResolve(pLink->GetHashKey(), pUnresHost->strHostname, pUnresHost->nPort, pUnresHost->strURL);
				}
			}
			//MORPH END   - Added by Stulle, Only resolve hostnames for downloads if partfile found [WiZaRd]
		}
		else
			DebugLogWarning(_T("FileIdentifier mismatch when trying to add ed2k link to existing download - AICH Hash or Size might differ, no sources added. File: %s"),
				partfile->GetFileName());
	}

	//MORPH START - Removed by Stulle, Only resolve hostnames for downloads if partfile found [WiZaRd]
	/*
	if (pLink->HasHostnameSources())
	{
		POSITION pos = pLink->m_HostnameSourcesList.GetHeadPosition();
		while (pos != NULL)
		{
			const SUnresolvedHostname* pUnresHost = pLink->m_HostnameSourcesList.GetNext(pos);
			m_srcwnd.AddToResolve(pLink->GetHashKey(), pUnresHost->strHostname, pUnresHost->nPort, pUnresHost->strURL);
		}
	}
	*//*
	//MORPH END   - Removed by Stulle, Only resolve hostnames for downloads if partfile found [WiZaRd]
}
*/
// This function has been modified in order
// to accomodate the category selection.
// NEW PARAM:  bool AllocatedLink = false by default
void CDownloadQueue::AddFileLinkToDownload(CED2KFileLink* pLink, int theCat, bool AllocatedLink)
{
	if (thePrefs.SelectCatForNewDL() && theCat==-1)
	{
		m_ED2KLinkQueue.AddTail(pLink);
		m_iLastLinkQueuedTick = GetTickCount();
		return;
	}

	int useCat = theCat;

	if (useCat == -1){
		if (thePrefs.UseAutoCat())
			useCat = theApp.downloadqueue->GetAutoCat(CString(pLink->GetName()), pLink->GetSize());
		else if (thePrefs.UseActiveCatForLinks())
			useCat = theApp.emuledlg->transferwnd->GetActiveCategory();
		else
			useCat = 0;
	}
	// Just in case...
	if (m_ED2KLinkQueue.GetCount() && !thePrefs.SelectCatForNewDL()) PurgeED2KLinkQueue();
	m_iLastLinkQueuedTick = 0;

	// Pass useCat instead of cat and autoset resume order.
	/*
	CPartFile* newfile = new CPartFile(pLink, cat);
	*/
	CPartFile* newfile = new CPartFile(pLink, useCat);

	if (newfile->GetStatus() == PS_ERROR){
		delete newfile;
		newfile=NULL;
	}
	else {
		// Pass useCat instead of cat and autoset resume order.
		if (thePrefs.SmallFileDLPush() && newfile->GetFileSize() < (uint64)154624)
			newfile->SetCatResumeOrder(0);
		else if (thePrefs.AutoSetResumeOrder())
			newfile->SetCatResumeOrder(GetMaxCatResumeOrder(useCat)+1);
		AddDownload(newfile,thePrefs.AddNewFilesPaused());
	}

	CPartFile* partfile = newfile;
	if (partfile == NULL)
		partfile = GetFileByID(pLink->GetHashKey());
	if (partfile)
	{
		// match the fileidentifier and only if the are the same add possible sources
		CFileIdentifierSA tmpFileIdent(pLink->GetHashKey(), pLink->GetSize(), pLink->GetAICHHash(), pLink->HasValidAICHHash());
		if (partfile->GetFileIdentifier().CompareRelaxed(tmpFileIdent))
		{
			if (pLink->HasValidSources())
				partfile->AddClientSources(pLink->SourcesList, 1, false);
			if (!partfile->GetFileIdentifier().HasAICHHash() && tmpFileIdent.HasAICHHash())
			{
				partfile->GetFileIdentifier().SetAICHHash(tmpFileIdent.GetAICHHash());
				partfile->GetAICHRecoveryHashSet()->SetMasterHash(tmpFileIdent.GetAICHHash(), AICH_VERIFIED);
				partfile->GetAICHRecoveryHashSet()->FreeHashSet();

			}
			//MORPH START - Added by Stulle, Only resolve hostnames for downloads if partfile found [WiZaRd]
			if (pLink->HasHostnameSources())
			{
				POSITION pos = pLink->m_HostnameSourcesList.GetHeadPosition();
				while (pos != NULL)
				{
					const SUnresolvedHostname* pUnresHost = pLink->m_HostnameSourcesList.GetNext(pos);
					m_srcwnd.AddToResolve(pLink->GetHashKey(), pUnresHost->strHostname, pUnresHost->nPort, pUnresHost->strURL);
				}
			}
			//MORPH END   - Added by Stulle, Only resolve hostnames for downloads if partfile found [WiZaRd]
		}
		else
			DebugLogWarning(_T("FileIdentifier mismatch when trying to add ed2k link to existing download - AICH Hash or Size might differ, no sources added. File: %s"),
				partfile->GetFileName());
	}

	//MORPH START - Removed by Stulle, Only resolve hostnames for downloads if partfile found [WiZaRd]
	/*
	if (pLink->HasHostnameSources())
	{
		POSITION pos = pLink->m_HostnameSourcesList.GetHeadPosition();
		while (pos != NULL)
		{
			const SUnresolvedHostname* pUnresHost = pLink->m_HostnameSourcesList.GetNext(pos);
			m_srcwnd.AddToResolve(pLink->GetHashKey(), pUnresHost->strHostname, pUnresHost->nPort, pUnresHost->strURL);
		}
	}
	*/
	//MORPH END   - Removed by Stulle, Only resolve hostnames for downloads if partfile found [WiZaRd]

	// Deallocate memory, because if we've gotten here,
	// this link wasn't added to the queue and therefore there's no reason to
	// not delete it.
	if (AllocatedLink) {
		delete pLink;
		pLink = NULL;
	}
}
// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

void CDownloadQueue::AddToResolved( CPartFile* pFile, SUnresolvedHostname* pUH )
{
	if( pFile && pUH )
		m_srcwnd.AddToResolve( pFile->GetFileHash(), pUH->strHostname, pUH->nPort, pUH->strURL);
}

void CDownloadQueue::AddDownload(CPartFile* newfile,bool paused) {
	// Barry - Add in paused mode if required
	if (paused)
		newfile->PauseFile();
	
	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	/*
	SetAutoCat(newfile);// HoaX_69 / Slugfiller: AutoCat
	*/
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

	filelist.AddTail(newfile);
	//zz_fly :: remove useless code :: Enig123
	/*
	SortByPriority();
	*/
	//zz_fly :: remove useless code :: Enig123
	CheckDiskspace();
	theApp.emuledlg->transferwnd->GetDownloadList()->AddFile(newfile);
	AddLogLine(true, GetResString(IDS_NEWDOWNLOAD), newfile->GetFileName());
	CString msgTemp;
	msgTemp.Format(GetResString(IDS_NEWDOWNLOAD) + _T("\n"), newfile->GetFileName());
	theApp.emuledlg->ShowNotifier(msgTemp, TBN_DOWNLOADADDED);
	ExportPartMetFilesOverview();
	SaveFileSettings(); // File Settings [sivka/Stulle] - Stulle
}

bool CDownloadQueue::IsFileExisting(const uchar* fileid, bool bLogWarnings) const
{
	const CKnownFile* file = theApp.sharedfiles->GetFileByID(fileid);
	if (file){
		if (bLogWarnings){
			if (file->IsPartFile())
				LogWarning(LOG_STATUSBAR, GetResString(IDS_ERR_ALREADY_DOWNLOADING), file->GetFileName());
			else
				LogWarning(LOG_STATUSBAR, GetResString(IDS_ERR_ALREADY_DOWNLOADED), file->GetFileName());
		}
		return true;
	}
	else if ((file = GetFileByID(fileid)) != NULL){
		if (bLogWarnings)
			LogWarning(LOG_STATUSBAR, GetResString(IDS_ERR_ALREADY_DOWNLOADING), file->GetFileName());
		return true;
	}
	return false;
}

//Xman Xtreme Mod
/*
void CDownloadQueue::Process(){
	
	ProcessLocalRequests(); // send src requests to local server

	uint32 downspeed = 0;
    uint64 maxDownload = thePrefs.GetMaxDownloadInBytesPerSec(true);
	if (maxDownload != UNLIMITED*1024 && datarate > 1500){
		downspeed = (UINT)((maxDownload*100)/(datarate+1));
		if (downspeed < 50)
			downspeed = 50;
		else if (downspeed > 200)
			downspeed = 200;
	}

	while(avarage_dr_list.GetCount()>0 && (GetTickCount() - avarage_dr_list.GetHead().timestamp > 10*1000) )
		m_datarateMS-=avarage_dr_list.RemoveHead().datalen;
	
	if (avarage_dr_list.GetCount()>1){
		datarate = (UINT)(m_datarateMS / avarage_dr_list.GetCount());
	} else {
		datarate = 0;
	}

	uint32 datarateX=0;
	udcounter++;

	theStats.m_fGlobalDone = 0;
	theStats.m_fGlobalSize = 0;
	theStats.m_dwOverallStatus=0;
	//filelist is already sorted by prio, therefore I removed all the extra loops..
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);

		// maintain global download stats
		theStats.m_fGlobalDone += (uint64)cur_file->GetCompletedSize();
		theStats.m_fGlobalSize += (uint64)cur_file->GetFileSize();
		
		if (cur_file->GetTransferringSrcCount()>0)
			theStats.m_dwOverallStatus  |= STATE_DOWNLOADING;
		if (cur_file->GetStatus()==PS_ERROR)
			theStats.m_dwOverallStatus  |= STATE_ERROROUS;


		if (cur_file->GetStatus() == PS_READY || cur_file->GetStatus() == PS_EMPTY){
			datarateX += cur_file->Process(downspeed, udcounter);
		}
		else{
			//This will make sure we don't keep old sources to paused and stoped files..
			cur_file->StopPausedFile();
		}
	}

	TransferredData newitem = {datarateX, ::GetTickCount()};
	avarage_dr_list.AddTail(newitem);
	m_datarateMS+=datarateX;

	if (udcounter == 5){
		if (theApp.serverconnect->IsUDPSocketAvailable()){
		    if((!lastudpstattime) || (::GetTickCount() - lastudpstattime) > UDPSERVERSTATTIME){
			    lastudpstattime = ::GetTickCount();
			    theApp.serverlist->ServerStats();
		    }
	    }
	}

	if (udcounter == 10){
		udcounter = 0;
		if (theApp.serverconnect->IsUDPSocketAvailable()){
			if ((!lastudpsearchtime) || (::GetTickCount() - lastudpsearchtime) > UDPSERVERREASKTIME)
				SendNextUDPPacket();
		}
	}

	CheckDiskspaceTimed();

// ZZ:DownloadManager -->
    if((!m_dwLastA4AFtime) || (::GetTickCount() - m_dwLastA4AFtime) > MIN2MS(8)) {
        theApp.clientlist->ProcessA4AFClients();
        m_dwLastA4AFtime = ::GetTickCount();
    }
// <-- ZZ:DownloadManager
}
*/
void CDownloadQueue::Process(){
	// ==> Global Source Limit [Max/Stulle] - Stulle
	if( thePrefs.IsUseGlobalHL() && (::GetTickCount() - m_dwUpdateHL) >= m_dwUpdateHlTime )
		SetHardLimits();
	// <== Global Source Limit [Max/Stulle] - Stulle
	
	ProcessLocalRequests(); // send src requests to local server

	// ==> Quick start [TPT] - Max
	static DWORD QuickStartEndTime=0;
	if(thePrefs.GetQuickStart() && theApp.IsConnected() && quickflag == 0)
	{
		if(quickflags == 0)
		{
			quicktime = ::GetTickCount();
			if (thePrefs.GetQuickStartMaxConnPerFiveBack() < thePrefs.GetQuickStartMaxConnPerFive()) // return values manual - Stulle
				thePrefs.SetMaxConsPerFive(thePrefs.GetQuickStartMaxConnPerFive());
			if (thePrefs.GetQuickStartMaxConnBack() < thePrefs.GetQuickStartMaxConn()) // return values manual - Stulle
				thePrefs.SetMaxCon(thePrefs.GetQuickStartMaxConn());
			quickflags = 1;
			AddLogLine(true, _T("***** Quick Start actived for %u min. *****"), thePrefs.GetQuickStartMaxTime());
			AddLogLine(false, _T("***** Max.Con.: %i *****"), thePrefs.GetMaxCon());
			AddLogLine(false, _T("***** Max.Con./5sec: %i *****"), thePrefs.GetMaxConperFive());

			DWORD QuickStartTime = MIN2MS(thePrefs.GetQuickStartMaxTime());
			QuickStartEndTime = quicktime + QuickStartTime;
			// this is sooooooooo useless...
			//			for(POSITION pos = theApp.downloadqueue->filelist.GetHeadPosition(); pos != NULL;) 
			//			{ 
			//					CPartFile* cur_file = theApp.downloadqueue->filelist.GetNext(pos); 
			//			}
		}
		if (QuickStartEndTime <= ::GetTickCount() || theApp.m_app_state == APP_STATE_SHUTTINGDOWN)
		{
			thePrefs.SetMaxConsPerFive(thePrefs.GetQuickStartMaxConnPerFiveBack()); // return values manual - Stulle
			thePrefs.SetMaxCon(thePrefs.GetQuickStartMaxConnBack()); // return values manual - Stulle
			quickflag = 1;
			AddLogLine(true , _T("***** Quick Start ended *****"));
			AddLogLine(false, _T("***** Max.Con.: %i *****"), thePrefs.GetMaxCon());
			AddLogLine(false, _T("***** Max.Con./5sec: %i *****"), thePrefs.GetMaxConperFive());
		}
	}
	// <== Quick start [TPT] - Max

	// Elapsed time (TIMER_PERIOD not accurate)	
	//zz_fly :: fix possible overflow :: start
	//note: i am not fix the subtraction. the subtraction is right when overflow happens. but overflowed deltaTime will let if(deltaTime>0) useless.
	/*
	uint32 deltaTime = ::GetTickCount() - m_lastProcessTime;
	*/
	uint32 deltaTime = 30000 + ::GetTickCount() - m_lastProcessTime;
	if (deltaTime > 30000)
		deltaTime -= 30000;
	else
		deltaTime = 0;
	//zz_fly :: end
	m_lastProcessTime += deltaTime;

	// - Maella -New bandwidth control-
	// Anticipate high CPU load => unregular cycle
	if(deltaTime > 0){
		/*
		const float maxDownload = (theApp.glob_prefs->GetNAFCEnable() == false && theApp.glob_prefs->GetNAFCFullControl() == false) ? 
		app_prefs->GetMaxDownload() : theApp.pBandWidthControl->GetMaxDownload();
		*/
		//Xman 1:3 Raitio
		//Xman GlobalMaxHarlimit for fairness
		// ==> Enforce Ratio [Stulle] - Stulle
		/*
		const bool limitbysources=GetGlobalSources()> thePrefs.m_uMaxGlobalSources && thePrefs.m_bAcceptsourcelimit==false;
		const float maxDownload = theApp.pBandWidthControl->GetMaxDownloadEx(limitbysources); //in [kb/s]
		if(limitbysources)
			m_limitstate=1; //at least session ratio
		else
			m_limitstate=0; //no limit
		*/
		uint8 limitbysources = 0;
		uint8 uReason = 0;
		m_limitratio = 0; // reset
		if(thePrefs.GetEnforceRatio())
			limitbysources |= 2;
		if(GetGlobalSources() > thePrefs.m_uMaxGlobalSources && thePrefs.m_bAcceptsourcelimit == false)
			limitbysources |= 1;
		// ==> Multiple friendslots [ZZ] - Mephisto
		if(theApp.friendlist && theApp.friendlist->IsFriendSlot()) // just in case...
			limitbysources |= 4;
		// <== Multiple friendslots [ZZ] - Mephisto
		// ==> Do not restrict download if no upload possible [Stulle] - Stulle
		/*
		const float maxDownload = theApp.pBandWidthControl->GetMaxDownloadEx(limitbysources); //in [kb/s]
		*/
		float maxDownload = 0; //in [kb/s]
		if(theApp.uploadqueue->GetUploadQueueLength() <= 2 && // yeah, it should be two at the least
			theApp.uploadqueue->GetWaitingUserCount() <= 0) // nobody in queue
		{
			maxDownload = thePrefs.GetMaxDownload();
			limitbysources = 0xFF;
		}
		else
			maxDownload = theApp.pBandWidthControl->GetMaxDownloadEx(limitbysources,uReason,m_limitratio);
		// <== Do not restrict download if no upload possible [Stulle] - Stulle
		m_limitstate=DLR_NONE; // reset
		if(limitbysources == 0xFF)
			m_limitstate = DLR_NOUL; // unlimited because no UL possible
		else
		{
			if(limitbysources&1)
				m_limitstate |= DLR_SESLIM; // Will be session limited by sources
			if(limitbysources&2)
				m_limitstate |= DLR_ENFLIM; // Will be session limited by enforce
			// ==> Multiple friendslots [ZZ] - Mephisto
			if(limitbysources&4)
				m_limitstate |= DLR_FRILIM; // Will be session limited by friendslots
			// <== Multiple friendslots [ZZ] - Mephisto
		}
		// <== Enforce Ratio [Stulle] - Stulle
		//Xman end
		//Xman end

		const bool isLimited = (maxDownload < UNLIMITED);

		if(isLimited == false)
		{
			m_nDownloadSlopeControl = 0;
			// ==> Enforce Ratio [Stulle] - Stulle
			if(uReason&4)
				m_limitstate |= DLR_13RATIO; // unlimited because < 1:3 ratio
			// <== Enforce Ratio [Stulle] - Stulle
		}
		else 
		{
			//Xman 
			//application can hang. compensate to 1 second:
			if(deltaTime>1000)
				deltaTime=1000;

			//Xman GlobalMaxHarlimit for fairness
			// ==> Enforce Ratio [Stulle] - Stulle
			/*
			if(limitbysources && maxDownload < theApp.pBandWidthControl->GetMaxDownloadEx(false))
				m_limitstate=2; //session ratio is reached->full limitation 
			//Xman end

			else if (thePrefs.Is13Ratio()) //downloadlimit although it should be unlimited => we have a ratio
				m_limitstate=3;
			else if (maxDownload < thePrefs.GetMaxDownload()) //can only be NAFC
				m_limitstate=4;
			*/
			if(uReason&1)
				m_limitstate |= DLR_SOURCE; // Forced 1:3 by sources
			if(uReason&2)
				m_limitstate |= DLR_ENFORCE; // Enforce
			if (uReason&8)
				m_limitstate |= DLR_NAFC; // NAFC limit
			// ==> Multiple friendslots [ZZ] - Mephisto
			if(uReason&16)
				m_limitstate |= DLR_FRIEND; // Friendslots
			// <== Multiple friendslots [ZZ] - Mephisto
			// <== Enforce Ratio [Stulle] - Stulle
			//Xman end

			//Xman changed sint32 to sint64
			// Bandwidth control
			const sint64 slopeCredit = (sint64)(maxDownload * 1.024f * (float)deltaTime); // it's 1.024 due to deltaTime is in milliseconds
			const sint64 maxSlop = 12 * slopeCredit / 10; // 120%

			// The Bandwitch control should be valid for an AVERAGE value
			m_nDownloadSlopeControl += slopeCredit; // [bytes/period]
			m_nDownloadSlopeControl -= (sint64)(theApp.pBandWidthControl->GeteMuleIn() - m_lastReceivedBytes);

			// Trunk negative value => possible when Overhead compensation activated
			if(m_nDownloadSlopeControl > maxSlop)
			{
				m_nDownloadSlopeControl = maxSlop;
			}
			//Xman 4.8
			//if we got to many data during one loop (e.g. downloads timed out and we read many filled buffers at once)
			//the download could stop for a few seconds. 
			//comensate it to max 2 seconds:
			else if(m_nDownloadSlopeControl < -1 * (sint64)(maxDownload*1024*2))
			{
				m_nDownloadSlopeControl = -1 * (sint64)(maxDownload*1024*2);
			}

		}

		// Keep current value for next processing
		//m_lastOverallReceivedBytes = theApp.pBandWidthControl->GeteMuleInOverall();
		m_lastReceivedBytes = theApp.pBandWidthControl->GeteMuleIn();

		sint64 nDownloadSlopeControl = m_nDownloadSlopeControl;  

		//Xman askfordownload priority
		if(udcounter == 0)
		{
			m_maxdownprio=m_maxdownprionew;
			m_maxdownprionew=0;
			GetTooManyConnections(true); //force the recalc
		}
		//Xman end

		GlobalHardLimitTemp = 0; // show global HL - Stulle

		theStats.m_fGlobalDone = 0;
		theStats.m_fGlobalSize = 0;
		theStats.m_dwOverallStatus=0;
		// Remark: filelist is not sorted by priority (see 'balancing' below), needed to priorize the connection (e.g. during start-up)
		for(int priority = 0; priority < 3; priority++)
		{
			POSITION next_pos = filelist.GetHeadPosition();
			bool notbalanced = true; //zz_fly :: optimized in download balance :: Enig123
			for(int i=0; i<filelist.GetCount(); i++)
			{
				POSITION cur_pos = next_pos;
				const int count = filelist.GetCount(); // Could changed => to check
				CPartFile* cur_file = filelist.GetNext(next_pos); // Already point to the next element

				//note: let all files can be maintained by global download stats. thanks DolphinX
				if(cur_file && (
					(priority == 0 && cur_file->GetDownPriority() == PR_HIGH) ||
					(priority == 1 && cur_file->GetDownPriority() == PR_NORMAL) ||
					(priority == 2 && (cur_file->GetDownPriority() == PR_LOW))))
				{			
					// maintain global download stats
					theStats.m_fGlobalDone += (uint64)cur_file->GetCompletedSize();
					theStats.m_fGlobalSize += (uint64)cur_file->GetFileSize();

					if (cur_file->GetTransferringSrcCount()>0)
						theStats.m_dwOverallStatus  |= STATE_DOWNLOADING;
					if (cur_file->GetStatus()==PS_ERROR)
						theStats.m_dwOverallStatus  |= STATE_ERROROUS;

					if(cur_file->GetStatus() == PS_READY || cur_file->GetStatus() == PS_EMPTY)
					{ 
						//Xman sourcecache
						cur_file->ProcessSourceCache();
						//Xman end

						// ==> show global HL - Stulle
						if (thePrefs.GetShowGlobalHL())
							GlobalHardLimitTemp += cur_file->GetMaxSources();
						// <== show global HL - Stulle

						// The method must be called regardless of nDownloadSlopeControl
						// The method returns the size of the received blocks if the download rate is limited (otherwise zero)
						uint32 maxAmmount = (nDownloadSlopeControl <= 0) ? 0 : (uint32)nDownloadSlopeControl;
						uint32 receivedBlock = cur_file->Process(maxAmmount, isLimited, udcounter == 0);						
						if(receivedBlock > 0)
						{
							nDownloadSlopeControl -= receivedBlock;
							// Try to 'balance' the download between sources (=> clients).
							// Move the 'uploaded' at the end of the list.
							if(isLimited == true && count == filelist.GetCount() 
							//zz_fly :: optimized in download balance :: Enig123 :: start
								//&& cur_file->GetStatus() == PS_READY //note: PS_EMPTY is in downloading too
								&& (nDownloadSlopeControl > 0 || notbalanced) ) //note: do not balance the file only recieved several bytes
							//zz_fly :: end
							{
								// To check if these line are a source of bug
								filelist.RemoveAt(cur_pos);
								filelist.AddTail(cur_file); 
								notbalanced = false; //zz_fly :: optimized in download balance :: Enig123
							}
						}
					}
				}
			}
		}
	}

	// ==> show global HL - Stulle
	if (thePrefs.GetShowGlobalHL())
	{
        if (filelist.GetCount() == 0)
			GlobalHardLimitTemp = 0;
		theApp.emuledlg->transferwnd->GetDownloadList()->GlobalHardLimit = GlobalHardLimitTemp;
	}
	// <== show global HL - Stulle

	//Xman askfordownload priority
	if(udcounter == 0 && !(theApp.listensocket->TooManySockets()) && m_maxdownprionew>0)
		m_maxdownprionew--;
	//Xman end

	udcounter++;

	// Server statistic + UDP socket    
	if (udcounter == (500/TIMER_PERIOD)) { // Maella -Small latency- every 0.5 second  
		if (theApp.serverconnect->IsUDPSocketAvailable())
		{
			if((!lastudpstattime) || (::GetTickCount() - lastudpstattime) > UDPSERVERSTATTIME)
			{
				lastudpstattime = ::GetTickCount();
				theApp.serverlist->ServerStats();
			}
		}
	}

	if (udcounter >= (1000/TIMER_PERIOD))  // Maella -Small latency- every 1 second
	{
		udcounter = 0;
		// [TPT] - Patch
		// This will avoid reordering list while filelist processing (due to Maella bandwidth)
		for (POSITION pos = filelist.GetHeadPosition(); pos != NULL;)
		{
			CPartFile* cur_file =  filelist.GetNext(pos);

			if(cur_file)
			{
				cur_file->UpdateAutoDownPriority();
				// This will make sure we don't keep old sources to paused and stoped files..
				// Remark: don't need to be processed every 50/100 ms
				cur_file->StopPausedFile();
			}
		}
		// [TPT] - Patch
		if (theApp.serverconnect->IsUDPSocketAvailable())
		{
			if ((!lastudpsearchtime) || (::GetTickCount() - lastudpsearchtime) > UDPSERVERREASKTIME)
				SendNextUDPPacket();
		}
	}
	//end - Maella -New bandwidth control-

	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	// Purge ED2K Link Queue
	if (m_iLastLinkQueuedTick && !m_bBusyPurgingLinks && (GetTickCount() - m_iLastLinkQueuedTick) > 400)
		PurgeED2KLinkQueue();
	else if (m_ED2KLinkQueue.GetCount() && !thePrefs.SelectCatForNewDL()) // This should not happen.
	{
		PurgeED2KLinkQueue();
		AddDebugLogLine(false, _T("ERROR: Links in ED2K Link Queue while SelectCatForNewDL was disabled!"));
	}
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

	// ==> CPU/MEM usage [$ick$/Stulle] - Max 
	if(::GetTickCount() > m_dwResTimer) 
		{
			theApp.emuledlg->transferwnd->ShowRessources();
			m_dwResTimer = ::GetTickCount() + 1000; // update ever 1 sec
		}
	// <== CPU/MEM usage [$ick$/Stulle] - Max 

	CheckDiskspaceTimed();
}
//Xman end

//Xman askfordownload priority
uint16 CDownloadQueue::GetTooManyConnections(bool recalc)
{
	if(recalc)
	{
		m_toomanyconnections=0;
		for (POSITION pos =filelist.GetHeadPosition();pos != 0;)
		{
			CPartFile* cur_file = filelist.GetNext(pos);
			m_toomanyconnections = m_toomanyconnections + (uint16)cur_file->GetSrcStatisticsValue(DS_TOOMANYCONNS);
		}
	}
	return m_toomanyconnections;
}
//Xman end

CPartFile* CDownloadQueue::GetFileByIndex(int index) const
{
	POSITION pos = filelist.FindIndex(index);
	if (pos)
		return filelist.GetAt(pos);
	return NULL;
}

CPartFile* CDownloadQueue::GetFileByID(const uchar* filehash) const
{
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
	{
		CPartFile* cur_file = filelist.GetNext(pos);
		if (!md4cmp(filehash, cur_file->GetFileHash()))
			return cur_file;
	}
	return NULL;
}

CPartFile* CDownloadQueue::GetFileByKadFileSearchID(uint32 id) const
{
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
	{
		CPartFile* cur_file = filelist.GetNext(pos);
		if (id == cur_file->GetKadFileSearchID())
			return cur_file;
	}
	return NULL;
}

bool CDownloadQueue::IsPartFile(const CKnownFile* file) const
{
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
	{
		if (file == filelist.GetNext(pos))
			return true;
	}
	return false;
}

bool CDownloadQueue::CheckAndAddSource(CPartFile* sender,CUpDownClient* source){
	if (sender->IsStopped()){
		delete source;
		return false;
	}

	if (source->HasValidHash())
	{
		if(!md4cmp(source->GetUserHash(), thePrefs.GetUserHash()))
		{
			if (thePrefs.GetVerbose())
				AddDebugLogLine(false, _T("Tried to add source with matching hash to your own."));
			delete source;
			return false;
		}
	}
	// filter sources which are known to be temporarily dead/useless
	if (theApp.clientlist->m_globDeadSourceList.IsDeadSource(source) || sender->m_DeadSourceList.IsDeadSource(source)){
		//if (thePrefs.GetLogFilteredIPs())
		//	AddDebugLogLine(DLP_DEFAULT, false, _T("Rejected source because it was found on the DeadSourcesList (%s) for file %s : %s")
		//	,sender->m_DeadSourceList.IsDeadSource(source)? _T("Local") : _T("Global"), sender->GetFileName(), source->DbgGetClientInfo() );
		delete source;
		return false;
	}

	// filter sources which are incompatible with our encryption setting (one requires it, and the other one doesn't supports it)
	if ( (source->RequiresCryptLayer() && (!thePrefs.IsClientCryptLayerSupported() || !source->HasValidHash())) || (thePrefs.IsClientCryptLayerRequired() && (!source->SupportsCryptLayer() || !source->HasValidHash())))
	{
#if defined(_DEBUG) || defined(_BETA)
		//if (thePrefs.GetDebugSourceExchange()) // TODO: Uncomment after testing
		AddDebugLogLine(DLP_DEFAULT, false, _T("Rejected source because CryptLayer-Setting (Obfuscation) was incompatible for file %s : %s"), sender->GetFileName(), source->DbgGetClientInfo() );
#endif
		delete source;
		return false;
	}

	// "Filter LAN IPs" and/or "IPfilter" is not required here, because it was already done in parent functions

	//srcLock.Lock();	//zz_fly :: make source add action thread safe :: Enig123

	// uses this only for temp. clients
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		for (POSITION pos2 = cur_file->srclist.GetHeadPosition();pos2 != 0; ){
			CUpDownClient* cur_client = cur_file->srclist.GetNext(pos2);
			if (cur_client->Compare(source, true) || cur_client->Compare(source, false)){
				if (cur_file == sender){ // this file has already this source
					delete source;
					//srcLock.Unlock();	//zz_fly :: make source add action thread safe :: Enig123
					return false;
				}
				// set request for this source
				if (cur_client->AddRequestForAnotherFile(sender)){
					theApp.emuledlg->transferwnd->GetDownloadList()->AddSource(sender,cur_client,true);
					delete source;
                    //Xman remark: this is done in filerequest
                    /*
					if(cur_client->GetDownloadState() != DS_CONNECTED) {
                        cur_client->SwapToAnotherFile(_T("New A4AF source found. CDownloadQueue::CheckAndAddSource()"), false, false, false, NULL, true, false); // ZZ:DownloadManager
                    }
                    */
                    //Xman end
					//srcLock.Unlock();	//zz_fly :: make source add action thread safe :: Enig123
					return false;
				}
				else{
					delete source;
					//srcLock.Unlock();	//zz_fly :: make source add action thread safe :: Enig123
					return false;
				}
			}
		}
	}
	//our new source is real new but maybe it is already uploading to us?
	//if yes the known client will be attached to the var "source"
	//and the old sourceclient will be deleted
	if (theApp.clientlist->AttachToAlreadyKnown(&source,0)){
#ifdef _DEBUG
		if (thePrefs.GetVerbose() && source->GetRequestFile()){
			// if a client sent us wrong sources (sources for some other file for which we asked but which we are also
			// downloading) we may get a little in trouble here when "moving" this source to some other partfile without
			// further checks and updates.
			if (md4cmp(source->GetRequestFile()->GetFileHash(), sender->GetFileHash()) != 0)
				AddDebugLogLine(false, _T("*** CDownloadQueue::CheckAndAddSource -- added potential wrong source (%u)(diff. filehash) to file \"%s\""), source->GetUserIDHybrid(), sender->GetFileName());
			if (source->GetRequestFile()->GetPartCount() != 0 && source->GetRequestFile()->GetPartCount() != sender->GetPartCount())
				AddDebugLogLine(false, _T("*** CDownloadQueue::CheckAndAddSource -- added potential wrong source (%u)(diff. partcount) to file \"%s\""), source->GetUserIDHybrid(), sender->GetFileName());
		}
#endif
		//Xman check if this is a recent dropped source
		if(source->droptime>0 /*it is a dropped source */ 
			&& ((::GetTickCount() - source->GetLastAskedTime())<(50*60*1000) /*last asked was shorter than 50 minutes */  
			|| (((::GetTickCount() - source->GetLastAskedTime())<(150*60*1000)) /*last asked was shorter than 150 minutes */ && (source->IsEmuleClient()==false || source->IsLeecher()))	))
		{
			//AddDebugLogLine(false, _T("-o- rejected dropped client %s, %s reentering downloadqueue after time: %u min"), source->GetClientVerString(), source->GetUserName(), (::GetTickCount()-source->droptime)/60000);
			//srcLock.Unlock();	//zz_fly :: make source add action thread safe :: Enig123
			return false;
		}
		//Xman end

		source->SetRequestFile(sender);
	}
	else{
		// here we know that the client instance 'source' is a new created client instance (see callers) 
		// which is therefor not already in the clientlist, we can avoid the check for duplicate client list entries 
		// when adding this client
		theApp.clientlist->AddClient(source,true);
	}
	
#ifdef _DEBUG
	if (thePrefs.GetVerbose() && source->GetPartCount()!=0 && source->GetPartCount()!=sender->GetPartCount()){
		DEBUG_ONLY(AddDebugLogLine(false, _T("*** CDownloadQueue::CheckAndAddSource -- New added source (%u, %s) had still value in partcount"), source->GetUserIDHybrid(), sender->GetFileName()));
	}
#endif

	//Xman Xtreme Downloadmanager
	//if(source->droptime>0)
		//AddDebugLogLine(false, _T("-o- dropped client %s, %s reentered downloadqueue after time: %u min"), source->GetClientVerString(), source->GetUserName(), (::GetTickCount()-source->droptime)/60000);
	source->droptime=0;
	source->enterqueuetime=0;
	//Xman end

	sender->srclist.AddTail(source);

	//srcLock.Unlock();	//zz_fly :: make source add action thread safe :: Enig123

	theApp.emuledlg->transferwnd->GetDownloadList()->AddSource(sender,source,false);

	//Xman GlobalMaxHarlimit for fairness
	IncGlobSources();
	//Xman end

	return true;
}

bool CDownloadQueue::CheckAndAddKnownSource(CPartFile* sender,CUpDownClient* source, bool bIgnoreGlobDeadList){
	if (sender->IsStopped())
		return false;

	//Xman check if this is a recent dropped source
	if(source->droptime>0 /*it is a dropped source */ 
		&& ((::GetTickCount() - source->GetLastAskedTime())<(50*60*1000) /*last asked was shorter than 50 minutes */  
		|| (((::GetTickCount() - source->GetLastAskedTime())<(150*60*1000)) /*last asked was shorter than 150 minutes */ && (source->IsEmuleClient()==false || source->IsLeecher()))	))
	{
		//AddDebugLogLine(false, _T("-o- rejected known dropped client %s, %s reentering downloadqueue after time: %u min"), source->GetClientVerString(), source->GetUserName(), (::GetTickCount()-source->droptime)/60000);
		return false;
	}
	//Xman end

	// filter sources which are known to be temporarily dead/useless
	//Xman
	/*
	if ( (theApp.clientlist->m_globDeadSourceList.IsDeadSource(source) && !bIgnoreGlobDeadList) || sender->m_DeadSourceList.IsDeadSource(source)){
	*/
	if ( (!bIgnoreGlobDeadList && theApp.clientlist->m_globDeadSourceList.IsDeadSource(source)) || sender->m_DeadSourceList.IsDeadSource(source)){
	//Xman end
		//if (thePrefs.GetLogFilteredIPs())
		//	AddDebugLogLine(DLP_DEFAULT, false, _T("Rejected source because it was found on the DeadSourcesList (%s) for file %s : %s")
		//	,sender->m_DeadSourceList.IsDeadSource(source)? _T("Local") : _T("Global"), sender->GetFileName(), source->DbgGetClientInfo() );
		return false;
	}

	// filter sources which are incompatible with our encryption setting (one requires it, and the other one doesn't supports it)
	if ( (source->RequiresCryptLayer() && (!thePrefs.IsClientCryptLayerSupported() || !source->HasValidHash())) || (thePrefs.IsClientCryptLayerRequired() && (!source->SupportsCryptLayer() || !source->HasValidHash())))
	{
#if defined(_DEBUG) || defined(_BETA)
		//if (thePrefs.GetDebugSourceExchange()) // TODO: Uncomment after testing
		AddDebugLogLine(DLP_DEFAULT, false, _T("Rejected source because CryptLayer-Setting (Obfuscation) was incompatible for file %s : %s"), sender->GetFileName(), source->DbgGetClientInfo() );
#endif
		return false;
	}

	// "Filter LAN IPs" -- this may be needed here in case we are connected to the internet and are also connected
	// to a LAN and some client from within the LAN connected to us. Though this situation may be supported in future
	// by adding that client to the source list and filtering that client's LAN IP when sending sources to
	// a client within the internet.
	//
	// "IPfilter" is not needed here, because that "known" client was already IPfiltered when receiving OP_HELLO.
	if (!source->HasLowID()){
		uint32 nClientIP = ntohl(source->GetUserIDHybrid());
		if (!IsGoodIP(nClientIP)){ // check for 0-IP, localhost and LAN addresses
			//if (thePrefs.GetLogFilteredIPs())
			//	AddDebugLogLine(false, _T("Ignored already known source with IP=%s"), ipstr(nClientIP));
			return false;
		}
	}

	//srcLock.Lock();	//zz_fly :: make source add action thread safe :: Enig123

	// use this for client which are already know (downloading for example)
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		if (cur_file->srclist.Find(source)){
			if (cur_file == sender)
			//zz_fly :: make source add action thread safe :: Enig123
			{
				//srcLock.Unlock();
			//zz_fly :: make source add action thread safe :: Enig123
				return false;
			} //zz_fly :: make source add action thread safe :: Enig123
			if (source->AddRequestForAnotherFile(sender))
				theApp.emuledlg->transferwnd->GetDownloadList()->AddSource(sender,source,true);
                //Xman
                /*
                if(source->GetDownloadState() != DS_CONNECTED) {
                    source->SwapToAnotherFile(_T("New A4AF source found. CDownloadQueue::CheckAndAddKnownSource()"), false, false, false, NULL, true, false); // ZZ:DownloadManager
                }
                */
                //Xman end
			//srcLock.Unlock();	//zz_fly :: make source add action thread safe :: Enig123
			return false;
		}
	}
#ifdef _DEBUG
	if (thePrefs.GetVerbose() && source->GetRequestFile()){
		// if a client sent us wrong sources (sources for some other file for which we asked but which we are also
		// downloading) we may get a little in trouble here when "moving" this source to some other partfile without
		// further checks and updates.
		if (md4cmp(source->GetRequestFile()->GetFileHash(), sender->GetFileHash()) != 0)
			AddDebugLogLine(false, _T("*** CDownloadQueue::CheckAndAddKnownSource -- added potential wrong source (%u)(diff. filehash) to file \"%s\""), source->GetUserIDHybrid(), sender->GetFileName());
		if (source->GetRequestFile()->GetPartCount() != 0 && source->GetRequestFile()->GetPartCount() != sender->GetPartCount())
			AddDebugLogLine(false, _T("*** CDownloadQueue::CheckAndAddKnownSource -- added potential wrong source (%u)(diff. partcount) to file \"%s\""), source->GetUserIDHybrid(), sender->GetFileName());
	}
#endif
	source->SetRequestFile(sender);
	sender->srclist.AddTail(source);
	source->SetSourceFrom(SF_PASSIVE);
	//Xman a TCP-connect is not only done on SX
	/*
	if (thePrefs.GetDebugSourceExchange())
		AddDebugLogLine(false, _T("SXRecv: Passively added source; %s, File=\"%s\""), source->DbgGetClientInfo(), sender->GetFileName());
	*/
#ifdef _DEBUG
	if (thePrefs.GetVerbose() && source->GetPartCount()!=0 && source->GetPartCount()!=sender->GetPartCount()){
		DEBUG_ONLY(AddDebugLogLine(false, _T("*** CDownloadQueue::CheckAndAddKnownSource -- New added source (%u, %s) had still value in partcount"), source->GetUserIDHybrid(), sender->GetFileName()));
	}
#endif
	//srcLock.Unlock();	//zz_fly :: make source add action thread safe :: Enig123
	//Xman Xtreme Downloadmanager
	//if(source->droptime>0)
		//AddDebugLogLine(false, _T("-o- known dropped client %s, %s reentered downloadqueue after time: %u min"), source->GetClientVerString(), source->GetUserName(), (::GetTickCount()-source->droptime)/60000);
	source->droptime=0;
	source->enterqueuetime=0;
	//Xman end

	theApp.emuledlg->transferwnd->GetDownloadList()->AddSource(sender,source,false);
	//UpdateDisplayedInfo();

	//Xman GlobalMaxHarlimit for fairness
	IncGlobSources();
	//Xman end

	return true;
}
//Xman end

bool CDownloadQueue::RemoveSource(CUpDownClient* toremove, bool bDoStatsUpdate)
{
	bool bRemovedSrcFromPartFile = false;
	//srcLock.Lock();	//zz_fly :: make source add action thread safe :: Enig123
	//Xman Code Improvement
	/*
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		for (POSITION pos2 = cur_file->srclist.GetHeadPosition();pos2 != 0; cur_file->srclist.GetNext(pos2)){
			if (toremove == cur_file->srclist.GetAt(pos2)){
				cur_file->srclist.RemoveAt(pos2);
				bRemovedSrcFromPartFile = true;
				if ( bDoStatsUpdate ){
					cur_file->RemoveDownloadingSource(toremove);
					cur_file->UpdatePartsInfo();
				}
				break;
			}
		}
	*/
	for (POSITION pos = filelist.GetHeadPosition();pos != NULL;){
		CPartFile* cur_file = filelist.GetNext(pos);
		POSITION pos2 = cur_file->srclist.Find(toremove);
		if (pos2 != NULL){
			cur_file->RemoveDownloadingSource(toremove); //to be sure
			cur_file->srclist.RemoveAt(pos2);
			cur_file->RemoveSourceFileName(toremove); // Follow The Majority [AndCycle/Stulle] - Stulle

			bRemovedSrcFromPartFile = true;
			if ( bDoStatsUpdate ){
				cur_file->UpdatePartsInfo();
			}
		}
	//Xman end
		if ( bDoStatsUpdate )
			cur_file->UpdateAvailablePartsCount();
	}
	//srcLock.Unlock();	//zz_fly :: make source add action thread safe :: Enig123

	// remove this source on all files in the downloadqueue who link this source
	// pretty slow but no way arround, maybe using a Map is better, but that's slower on other parts
	POSITION pos3, pos4;
	for(pos3 = toremove->m_OtherRequests_list.GetHeadPosition();(pos4=pos3)!=NULL;)
	{
		toremove->m_OtherRequests_list.GetNext(pos3);				
		POSITION pos5 = toremove->m_OtherRequests_list.GetAt(pos4)->A4AFsrclist.Find(toremove); 
		if(pos5)
		{ 
			toremove->m_OtherRequests_list.GetAt(pos4)->A4AFsrclist.RemoveAt(pos5);
			theApp.emuledlg->transferwnd->GetDownloadList()->RemoveSource(toremove,toremove->m_OtherRequests_list.GetAt(pos4));
			toremove->m_OtherRequests_list.RemoveAt(pos4);
		}
	}
	for(pos3 = toremove->m_OtherNoNeeded_list.GetHeadPosition();(pos4=pos3)!=NULL;)
	{
		toremove->m_OtherNoNeeded_list.GetNext(pos3);				
		POSITION pos5 = toremove->m_OtherNoNeeded_list.GetAt(pos4)->A4AFsrclist.Find(toremove); 
		if(pos5)
		{ 
			toremove->m_OtherNoNeeded_list.GetAt(pos4)->A4AFsrclist.RemoveAt(pos5);
			theApp.emuledlg->transferwnd->GetDownloadList()->RemoveSource(toremove,toremove->m_OtherNoNeeded_list.GetAt(pos4));
			toremove->m_OtherNoNeeded_list.RemoveAt(pos4);
		}
	}

	if (bRemovedSrcFromPartFile && (toremove->HasFileRating() || !toremove->GetFileComment().IsEmpty()))
	{
		CPartFile* pFile = toremove->GetRequestFile();
		if(pFile)
			pFile->UpdateFileRatingCommentAvail();
	}

	toremove->SetDownloadState(DS_NONE);
	//Xman fix for startupload (downloading side)
		toremove->protocolstepflag1=false; //to be sure
	//Xman end
	theApp.emuledlg->transferwnd->GetDownloadList()->RemoveSource(toremove,0);
	toremove->SetRequestFile(NULL);

	//Xman GlobalMaxHarlimit for fairness
	if(bRemovedSrcFromPartFile)
		DecGlobSources();
	//Xman end

	return bRemovedSrcFromPartFile;
}

void CDownloadQueue::RemoveFile(CPartFile* toremove)
{
	// ==> File Settings [sivka/Stulle] - Stulle
	if (m_SaveSettingsThread) // we just started saving, we better wait
	{
		m_SaveSettingsThread->EndThread();
		delete m_SaveSettingsThread;
		m_SaveSettingsThread = NULL;
	}
	// <== File Settings [sivka/Stulle] - Stulle
	RemoveLocalServerRequest(toremove);

	//Xman
	// Maella -Code Improvement-
	/*
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;filelist.GetNext(pos)){
		if (toremove == filelist.GetAt(pos)){
			filelist.RemoveAt(pos);
			break;
		}
	}
	*/
	POSITION pos = filelist.Find(toremove);
	if (pos != NULL){
		//zz_fly :: free unused memory when a download is cancelled/completed :: Enig123 :: start
		/*
		ASSERT(filelist.GetAt(pos)->srclist.IsEmpty());
		filelist.GetAt(pos)->srclist.RemoveAll(); // Security 
		*/
		CPartFile* pFile = filelist.GetAt(pos);
		if(!pFile->srclist.IsEmpty())
			pFile->srclist.RemoveAll(); // Security 
		pFile->RemoveAllRequestedBlocks();	//Enig123??
		//zz_fly :: free unused memory when a download is cancelled/completed :: Enig123 :: end
		filelist.RemoveAt(pos);
	}
	// Maella end
	//zz_fly :: remove useless code :: Enig123
	/*
	SortByPriority();
	*/
	//zz_fly :: remove useless code :: Enig123
	CheckDiskspace();
	ExportPartMetFilesOverview();
}

void CDownloadQueue::DeleteAll(){
	POSITION pos;
	for (pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		cur_file->srclist.RemoveAll();
		// Barry - Should also remove all requested blocks
		// Don't worry about deleting the blocks, that gets handled 
		// when CUpDownClient is deleted in CClientList::DeleteAll()
		cur_file->RemoveAllRequestedBlocks();
	}
}

// Max. file IDs per UDP packet
// ----------------------------
// 576 - 30 bytes of header (28 for UDP, 2 for "E3 9A" edonkey proto) = 546 bytes
// 546 / 16 = 34
#define MAX_UDP_PACKET_DATA				510
#define BYTES_PER_FILE_G1				16
#define BYTES_PER_FILE_G2				20
#define ADDITIONAL_BYTES_PER_LARGEFILE	8

#define MAX_REQUESTS_PER_SERVER		35

bool CDownloadQueue::IsMaxFilesPerUDPServerPacketReached(uint32 nFiles, uint32 nIncludedLargeFiles) const
{
	if (cur_udpserver && cur_udpserver->GetUDPFlags() & SRV_UDPFLG_EXT_GETSOURCES)
	{
		
		const int nBytesPerNormalFile = ((cur_udpserver->GetUDPFlags() & SRV_UDPFLG_EXT_GETSOURCES2) > 0)? BYTES_PER_FILE_G2 : BYTES_PER_FILE_G1; 
		const int nUsedBytes = nFiles*nBytesPerNormalFile + nIncludedLargeFiles*ADDITIONAL_BYTES_PER_LARGEFILE;
		if (nIncludedLargeFiles > 0){
			ASSERT( cur_udpserver->SupportsLargeFilesUDP() );
			ASSERT( cur_udpserver->GetUDPFlags() & SRV_UDPFLG_EXT_GETSOURCES2 );
		}
		return (m_cRequestsSentToServer >= MAX_REQUESTS_PER_SERVER) || (nUsedBytes >= MAX_UDP_PACKET_DATA);
	}
	else{
		ASSERT( nIncludedLargeFiles == 0);
		return nFiles != 0;
	}
}

bool CDownloadQueue::SendGlobGetSourcesUDPPacket(CSafeMemFile* data, bool bExt2Packet, uint32 nFiles, uint32 nIncludedLargeFiles)
{
	bool bSentPacket = false;

	if (cur_udpserver)
	{
#ifdef _DEBUG
		int iPacketSize = (int)data->GetLength();
#endif
		Packet packet(data);
		data = NULL;
		if (bExt2Packet){
			ASSERT( iPacketSize > 0 && (uint32)iPacketSize == nFiles*20 + nIncludedLargeFiles*8);
			packet.opcode = OP_GLOBGETSOURCES2;
		}
		else{
			ASSERT( iPacketSize > 0 && (uint32)iPacketSize == nFiles*16 && nIncludedLargeFiles == 0);
			packet.opcode = OP_GLOBGETSOURCES;
		}
		if (thePrefs.GetDebugServerUDPLevel() > 0)
			Debug(_T(">>> Sending %s to server %-21s (%3u of %3u); FileIDs=%u(%u large)\n"), (packet.opcode == OP_GLOBGETSOURCES2) ? _T("OP__GlobGetSources2") : _T("OP__GlobGetSources1"), ipstr(cur_udpserver->GetAddress(), cur_udpserver->GetPort()), m_iSearchedServers + 1, theApp.serverlist->GetServerCount(), nFiles, nIncludedLargeFiles);

		theStats.AddUpDataOverheadServer(packet.size);
		theApp.serverconnect->SendUDPPacket(&packet, cur_udpserver, false);

		m_cRequestsSentToServer += nFiles;
		bSentPacket = true;
	}

	return bSentPacket;
}

bool CDownloadQueue::SendNextUDPPacket()
{
	if (   filelist.IsEmpty()
		|| !theApp.serverconnect->IsUDPSocketAvailable()
		|| !theApp.serverconnect->IsConnected()
		|| thePrefs.IsClientCryptLayerRequired()) // we cannot use sources received without userhash, so dont ask
		return false;

	CServer* pConnectedServer = theApp.serverconnect->GetCurrentServer();
	if (pConnectedServer)
		pConnectedServer = theApp.serverlist->GetServerByAddress(pConnectedServer->GetAddress(), pConnectedServer->GetPort());

	if (!cur_udpserver)
	{
		while ((cur_udpserver = theApp.serverlist->GetSuccServer(cur_udpserver)) != NULL) {
			if (cur_udpserver == pConnectedServer)
				continue;
			if (cur_udpserver->GetFailedCount() >= thePrefs.GetDeadServerRetries())
				continue;
			break;
		}
		if (cur_udpserver == NULL) {
			StopUDPRequests();
			return false;
		}
		m_cRequestsSentToServer = 0;
	}

	bool bGetSources2Packet = (cur_udpserver->GetUDPFlags() & SRV_UDPFLG_EXT_GETSOURCES2) > 0;
	bool bServerSupportsLargeFiles = cur_udpserver->SupportsLargeFilesUDP();

	// loop until the packet is filled or a packet was sent
	bool bSentPacket = false;
	CSafeMemFile dataGlobGetSources(20);
	int iFiles = 0;
	int iLargeFiles = 0;
	while (!IsMaxFilesPerUDPServerPacketReached(iFiles, iLargeFiles) && !bSentPacket)
	{
		// get next file to search sources for
		CPartFile* nextfile = NULL;
		while (!bSentPacket && !(nextfile && (nextfile->GetStatus() == PS_READY || nextfile->GetStatus() == PS_EMPTY)))
		{
			if (lastfile == NULL) // we just started the global source searching or have switched the server
			{
				// get first file to search sources for
				nextfile = filelist.GetHead();
				lastfile = nextfile;
			}
			else
			{
				POSITION pos = filelist.Find(lastfile);
				if (pos == 0) // the last file is no longer in the DL-list (may have been finished or canceld)
				{
					// get first file to search sources for
					nextfile = filelist.GetHead();
					lastfile = nextfile;
				}
				else
				{
					filelist.GetNext(pos);
					if (pos == 0) // finished asking the current server for all files
					{
						// if there are pending requests for the current server, send them
						if (dataGlobGetSources.GetLength() > 0)
						{
							if (SendGlobGetSourcesUDPPacket(&dataGlobGetSources, bGetSources2Packet, iFiles, iLargeFiles))
								bSentPacket = true;
							dataGlobGetSources.SetLength(0);
							iFiles = 0;
							iLargeFiles = 0;
						}

						// get next server to ask
						while ((cur_udpserver = theApp.serverlist->GetSuccServer(cur_udpserver)) != NULL) {
							if (cur_udpserver == pConnectedServer)
								continue;
							if (cur_udpserver->GetFailedCount() >= thePrefs.GetDeadServerRetries())
								continue;
							break;
						}
						m_cRequestsSentToServer = 0;
						if (cur_udpserver == NULL) {
							// finished asking all servers for all files
							if (thePrefs.GetDebugServerUDPLevel() > 0 && thePrefs.GetDebugServerSourcesLevel() > 0)
								Debug(_T("Finished UDP search processing for all servers (%u)\n"), theApp.serverlist->GetServerCount());
							StopUDPRequests();
							return false; // finished (processed all file & all servers)
						}
						m_iSearchedServers++;

						// if we already sent a packet, switch to the next file at next function call
						if (bSentPacket){
							lastfile = NULL;
							break;
						}

						bGetSources2Packet = (cur_udpserver->GetUDPFlags() & SRV_UDPFLG_EXT_GETSOURCES2) > 0;
						bServerSupportsLargeFiles = cur_udpserver->SupportsLargeFilesUDP();

						// have selected a new server; get first file to search sources for
						nextfile = filelist.GetHead();
						lastfile = nextfile;
					}
					else
					{
						nextfile = filelist.GetAt(pos);
						lastfile = nextfile;
					}
				}
			}
		}

		if (!bSentPacket && nextfile && nextfile->GetSourceCount() < nextfile->GetMaxSourcePerFileUDP() && (bServerSupportsLargeFiles || !nextfile->IsLargeFile()) )
		{
			if (bGetSources2Packet){
				if (nextfile->IsLargeFile()){
					// GETSOURCES2 Packet Large File (<HASH_16><IND_4 = 0><SIZE_8> *)
					iLargeFiles++;
					dataGlobGetSources.WriteHash16(nextfile->GetFileHash());
					dataGlobGetSources.WriteUInt32(0);
					dataGlobGetSources.WriteUInt64(nextfile->GetFileSize());
				}
				else{
					// GETSOURCES2 Packet (<HASH_16><SIZE_4> *)
					dataGlobGetSources.WriteHash16(nextfile->GetFileHash());
					dataGlobGetSources.WriteUInt32((uint32)(uint64)nextfile->GetFileSize());
				}
			}
			else{
				// GETSOURCES Packet (<HASH_16> *)
				dataGlobGetSources.WriteHash16(nextfile->GetFileHash());
			}
			iFiles++;
			if (thePrefs.GetDebugServerUDPLevel() > 0 && thePrefs.GetDebugServerSourcesLevel() > 0)
				Debug(_T(">>> Queued  %s to server %-21s (%3u of %3u); Buff  %u(%u)=%s\n"), bGetSources2Packet ? _T("OP__GlobGetSources2") : _T("OP__GlobGetSources1"), ipstr(cur_udpserver->GetAddress(), cur_udpserver->GetPort()), m_iSearchedServers + 1, theApp.serverlist->GetServerCount(), iFiles, iLargeFiles, DbgGetFileInfo(nextfile->GetFileHash()));
		}
	}

	ASSERT( dataGlobGetSources.GetLength() == 0 || !bSentPacket );

	if (!bSentPacket && dataGlobGetSources.GetLength() > 0)
		SendGlobGetSourcesUDPPacket(&dataGlobGetSources, bGetSources2Packet, iFiles, iLargeFiles);

	// send max 35 UDP request to one server per interval
	// if we have more than 35 files, we rotate the list and use it as queue
	if (m_cRequestsSentToServer >= MAX_REQUESTS_PER_SERVER)
	{
		if (thePrefs.GetDebugServerUDPLevel() > 0 && thePrefs.GetDebugServerSourcesLevel() > 0)
			Debug(_T("Rotating file list\n"));

		// move the last 35 files to the head
		if (filelist.GetCount() >= MAX_REQUESTS_PER_SERVER) {
			for (int i = 0; i != MAX_REQUESTS_PER_SERVER; i++)
				filelist.AddHead(filelist.RemoveTail());
		}

		// and next server
		while ((cur_udpserver = theApp.serverlist->GetSuccServer(cur_udpserver)) != NULL) {
			if (cur_udpserver == pConnectedServer)
				continue;
			if (cur_udpserver->GetFailedCount() >= thePrefs.GetDeadServerRetries())
				continue;
			break;
		}
		m_cRequestsSentToServer = 0;
		if (cur_udpserver == NULL) {
			if (thePrefs.GetDebugServerUDPLevel() > 0 && thePrefs.GetDebugServerSourcesLevel() > 0)
				Debug(_T("Finished UDP search processing for all servers (%u)\n"), theApp.serverlist->GetServerCount());
			StopUDPRequests();
			return false; // finished (processed all file & all servers)
		}
		m_iSearchedServers++;
		lastfile = NULL;
	}

	return true;
}

void CDownloadQueue::StopUDPRequests()
{
	cur_udpserver = NULL;
	lastudpsearchtime = ::GetTickCount();
	lastfile = NULL;
	m_iSearchedServers = 0;
}

//zz_fly :: remove useless code :: Enig123 :: start
//note: not needed, because priority have been handled in CDownloadQueue::Process(), thanks Enig123
/*
bool CDownloadQueue::CompareParts(POSITION pos1, POSITION pos2){
	CPartFile* file1 = filelist.GetAt(pos1);
	CPartFile* file2 = filelist.GetAt(pos2);
    return CPartFile::RightFileHasHigherPrio(file1, file2);
}

void CDownloadQueue::SwapParts(POSITION pos1, POSITION pos2){
	CPartFile* file1 = filelist.GetAt(pos1);
	CPartFile* file2 = filelist.GetAt(pos2);
	filelist.SetAt(pos1, file2);
	filelist.SetAt(pos2, file1);
}

void CDownloadQueue::HeapSort(UINT first, UINT last)
{
	UINT r;
	POSITION pos1 = filelist.FindIndex(first);
	for ( r = first; !(r & (UINT)INT_MIN) && (r<<1) < last; ){
		UINT r2 = (r<<1)+1;
		POSITION pos2 = filelist.FindIndex(r2);
		if (r2 != last){
			POSITION pos3 = pos2;
			filelist.GetNext(pos3);
			if (!CompareParts(pos2, pos3)){
				pos2 = pos3;
				r2++;
			}
		}
		if (!CompareParts(pos1, pos2)) {
			SwapParts(pos1, pos2);
			r = r2;
			pos1 = pos2;
		}
		else
			break;
	}
}

void CDownloadQueue::SortByPriority()
{
	UINT n = filelist.GetCount();
	if (!n)
		return;
	UINT i;
	for ( i = n/2; i--; )
		HeapSort(i, n-1);
	for ( i = n; --i; ){
		SwapParts(filelist.FindIndex(0), filelist.FindIndex(i));
		HeapSort(0, i-1);
	}
}
*/
//zz_fly :: end

void CDownloadQueue::CheckDiskspaceTimed()
{
	if ((!lastcheckdiskspacetime) || (::GetTickCount() - lastcheckdiskspacetime) > DISKSPACERECHECKTIME)
		CheckDiskspace();
}

void CDownloadQueue::CheckDiskspace(bool bNotEnoughSpaceLeft)
{
	lastcheckdiskspacetime = ::GetTickCount();

	// sorting the list could be done here, but I prefer to "see" that function call in the calling functions.
	//SortByPriority();

	// If disabled, resume any previously paused files
	if (!thePrefs.IsCheckDiskspaceEnabled())
	{
		if (!bNotEnoughSpaceLeft) // avoid worse case, if we already had 'disk full'
		{
			for( POSITION pos1 = filelist.GetHeadPosition(); pos1 != NULL; )
			{
				CPartFile* cur_file = filelist.GetNext(pos1);
				switch(cur_file->GetStatus())
				{
				case PS_PAUSED:
				case PS_ERROR:
				case PS_COMPLETING:
				case PS_COMPLETE:
					continue;
				}
				cur_file->ResumeFileInsufficient();
			}
		}
		return;
	}

	uint64 nTotalAvailableSpaceMain = bNotEnoughSpaceLeft ? 0 : GetFreeDiskSpaceX(thePrefs.GetTempDir());

	// 'bNotEnoughSpaceLeft' - avoid worse case, if we already had 'disk full'
	if (thePrefs.GetMinFreeDiskSpace() == 0)
	{
		for( POSITION pos1 = filelist.GetHeadPosition(); pos1 != NULL; )
		{
			CPartFile* cur_file = filelist.GetNext(pos1);

			uint64 nTotalAvailableSpace = bNotEnoughSpaceLeft ? 0 : 
				((thePrefs.GetTempDirCount()==1)?nTotalAvailableSpaceMain:GetFreeDiskSpaceX(cur_file->GetTempPath()));

			switch(cur_file->GetStatus())
			{
			case PS_PAUSED:
			case PS_ERROR:
			case PS_COMPLETING:
			case PS_COMPLETE:
				continue;
			}

			// Pause the file only if it would grow in size and would exceed the currently available free space
			uint64 nSpaceToGo = cur_file->GetNeededSpace();
			if (nSpaceToGo <= nTotalAvailableSpace)
			{
				nTotalAvailableSpace -= nSpaceToGo;
				cur_file->ResumeFileInsufficient();
			}
			else
				cur_file->PauseFile(true/*bInsufficient*/);
		}
	}
	else
	{
		for( POSITION pos1 = filelist.GetHeadPosition(); pos1 != NULL; )
		{
			CPartFile* cur_file = filelist.GetNext(pos1);
			switch(cur_file->GetStatus())
			{
			case PS_PAUSED:
			case PS_ERROR:
			case PS_COMPLETING:
			case PS_COMPLETE:
				continue;
			}

			uint64 nTotalAvailableSpace = bNotEnoughSpaceLeft ? 0 : 
				((thePrefs.GetTempDirCount()==1)?nTotalAvailableSpaceMain:GetFreeDiskSpaceX(cur_file->GetTempPath()));
			if (nTotalAvailableSpace < thePrefs.GetMinFreeDiskSpace())
			{
				if (cur_file->IsNormalFile())
				{
					// Normal files: pause the file only if it would still grow
					uint64 nSpaceToGrow = cur_file->GetNeededSpace();
					if (nSpaceToGrow > 0)
						cur_file->PauseFile(true/*bInsufficient*/);
				}
				else
				{
					// Compressed/sparse files: always pause the file
					cur_file->PauseFile(true/*bInsufficient*/);
				}
			}
			else
			{
				// doesn't work this way. resuming the file without checking if there is a chance to successfully
				// flush any available buffered file data will pause the file right after it was resumed and disturb
				// the StopPausedFile function.
				//cur_file->ResumeFileInsufficient();
			}
		}
	}
}

void CDownloadQueue::GetDownloadSourcesStats(SDownloadStats& results)
{
	memset(&results, 0, sizeof results);
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
	{
		const CPartFile* cur_file = filelist.GetNext(pos);

		results.a[0]  += cur_file->GetSourceCount();
		results.a[1]  += cur_file->GetTransferringSrcCount();
		results.a[2]  += cur_file->GetSrcStatisticsValue(DS_ONQUEUE);
		results.a[3]  += cur_file->GetSrcStatisticsValue(DS_REMOTEQUEUEFULL);
		results.a[4]  += cur_file->GetSrcStatisticsValue(DS_NONEEDEDPARTS);
		results.a[5]  += cur_file->GetSrcStatisticsValue(DS_CONNECTED);
		results.a[6]  += cur_file->GetSrcStatisticsValue(DS_REQHASHSET);
		results.a[7]  += cur_file->GetSrcStatisticsValue(DS_CONNECTING);
		results.a[8]  += cur_file->GetSrcStatisticsValue(DS_WAITCALLBACK);
		results.a[8]  += cur_file->GetSrcStatisticsValue(DS_WAITCALLBACKKAD);
		results.a[9]  += cur_file->GetSrcStatisticsValue(DS_TOOMANYCONNS);
		results.a[9]  += cur_file->GetSrcStatisticsValue(DS_TOOMANYCONNSKAD);
		results.a[10] += cur_file->GetSrcStatisticsValue(DS_LOWTOLOWIP);
		results.a[11] += cur_file->GetSrcStatisticsValue(DS_NONE);
		results.a[12] += cur_file->GetSrcStatisticsValue(DS_ERROR);
		results.a[13] += cur_file->GetSrcStatisticsValue(DS_BANNED);
		results.a[14] += cur_file->src_stats[3];
		results.a[15] += cur_file->GetSrcA4AFCount();
		results.a[16] += cur_file->src_stats[0];
		results.a[17] += cur_file->src_stats[1];
		results.a[18] += cur_file->src_stats[2];
		results.a[19] += cur_file->net_stats[0];
		results.a[20] += cur_file->net_stats[1];
		results.a[21] += cur_file->net_stats[2];
		results.a[22] += cur_file->m_DeadSourceList.GetDeadSourcesCount();
	}
}

CUpDownClient* CDownloadQueue::GetDownloadClientByIP(uint32 dwIP){
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		for (POSITION pos2 = cur_file->srclist.GetHeadPosition();pos2 != 0; ){
			CUpDownClient* cur_client = cur_file->srclist.GetNext(pos2);
			if (dwIP == cur_client->GetIP()){
				return cur_client;
			}
		}
	}
	return NULL;
}

CUpDownClient* CDownloadQueue::GetDownloadClientByIP_UDP(uint32 dwIP, uint16 nUDPPort, bool bIgnorePortOnUniqueIP, bool* pbMultipleIPs){
	CUpDownClient* pMatchingIPClient = NULL;
	uint32 cMatches = 0;

	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		for (POSITION pos2 = cur_file->srclist.GetHeadPosition();pos2 != 0;){
			CUpDownClient* cur_client = cur_file->srclist.GetNext(pos2);
			if (dwIP == cur_client->GetIP() && nUDPPort == cur_client->GetUDPPort()){
				return cur_client;
			}
			else if (dwIP == cur_client->GetIP() && bIgnorePortOnUniqueIP && cur_client != pMatchingIPClient){
				pMatchingIPClient = cur_client;
				cMatches++;
			}
		}
	}
	if (pbMultipleIPs != NULL)
		*pbMultipleIPs = cMatches > 1;

	if (pMatchingIPClient != NULL && cMatches == 1)
		return pMatchingIPClient;
	else
		return NULL;
}

bool CDownloadQueue::IsInList(const CUpDownClient* client) const
{
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		for (POSITION pos2 = cur_file->srclist.GetHeadPosition();pos2 != 0;){
			if (cur_file->srclist.GetNext(pos2) == client)
				return true;
		}
	}
	return false;
}

// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
/*
void CDownloadQueue::ResetCatParts(UINT cat)
{
	CPartFile* cur_file;

	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; ){
		cur_file = filelist.GetNext(pos);

		if (cur_file->GetCategory() == cat)
			cur_file->SetCategory(0);
		else if (cur_file->GetCategory() > cat)
			cur_file->SetCategory(cur_file->GetCategory() - 1);
	}
}
*/
// We need to reset the resume order, too, so that these files don't
// screw up the order of 'All' category.  This function is modified.
void CDownloadQueue::ResetCatParts(UINT cat, UINT useCat)
{
	int useOrder = GetMaxCatResumeOrder(useCat);
	CPartFile* cur_file;
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; ){
		cur_file = filelist.GetNext(pos);

		if (cur_file->GetCategory()==cat)
		{
			useOrder++;
			cur_file->SetCategory(useCat);
			cur_file->SetCatResumeOrder(useOrder);
		}
		else if (cur_file->GetCategory() > cat)
			cur_file->SetCategory(cur_file->GetCategory() - 1);
	}
}
// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

void CDownloadQueue::SetCatPrio(UINT cat, uint8 newprio)
{
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; ){
		CPartFile* cur_file = filelist.GetNext(pos);
		if (cat==0 || cur_file->GetCategory()==cat)
			if (newprio==PR_AUTO) {
				cur_file->SetAutoDownPriority(true);
				cur_file->SetDownPriority(PR_HIGH, false);
			}
			else {
				cur_file->SetAutoDownPriority(false);
				cur_file->SetDownPriority(newprio, false);
			}
	}

    //zz_fly :: remove useless code :: Enig123
    /*
    theApp.downloadqueue->SortByPriority();
    */
    //zz_fly :: remove useless code :: Enig123
	theApp.downloadqueue->CheckDiskspaceTimed();
}

// ZZ:DownloadManager -->
void CDownloadQueue::RemoveAutoPrioInCat(UINT cat, uint8 newprio){
	CPartFile* cur_file;
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;filelist.GetNext(pos)){
		cur_file = filelist.GetAt(pos);
        if (cur_file->IsAutoDownPriority() && (cat==0 || cur_file->GetCategory()==cat)) {
			cur_file->SetAutoDownPriority(false);
			cur_file->SetDownPriority(newprio, false);
		}
	}

    //zz_fly :: remove useless code :: Enig123
    /*
    theApp.downloadqueue->SortByPriority();
    */
    //zz_fly :: remove useless code :: Enig123
	theApp.downloadqueue->CheckDiskspaceTimed();
}
// <-- ZZ:DownloadManager

void CDownloadQueue::SetCatStatus(UINT cat, int newstatus)
{
	bool reset = false;
	bool resort = false;

	POSITION pos= filelist.GetHeadPosition();
	while (pos != 0)
	{
		CPartFile* cur_file = filelist.GetAt(pos);
		if (!cur_file)
			continue;

		if (cat==-1 || 
			(cat==-2 && cur_file->GetCategory()==0) ||
			(cat==0 && cur_file->CheckShowItemInGivenCat(cat)) || 
			(cat>0 && cat==cur_file->GetCategory()))
		{
			switch (newstatus){
				case MP_CANCEL:
					cur_file->DeleteFile();
					reset = true;
					break;
				case MP_PAUSE:
					cur_file->PauseFile(false, false);
                    resort = true;
					break;
				case MP_STOP:
					cur_file->StopFile(false, false);
                    resort = true;
					break;
				case MP_RESUME: 
					if (cur_file->CanResumeFile()){
						if (cur_file->GetStatus() == PS_INSUFFICIENT)
							cur_file->ResumeFileInsufficient();
                        else {
							cur_file->ResumeFile(false);
                            resort = true;
                        }
					}
					break;
			}
		}
		filelist.GetNext(pos);
		if (reset)
		{
			reset = false;
			pos = filelist.GetHeadPosition();
		}
	}

    if(resort) {
	    //zz_fly :: remove useless code :: Enig123
	    /*
	    theApp.downloadqueue->SortByPriority();
	    */
	    //zz_fly :: remove useless code :: Enig123
	    theApp.downloadqueue->CheckDiskspace();
    }
}

void CDownloadQueue::MoveCat(UINT from, UINT to)
{
	if (from < to)
		--to;

	POSITION pos= filelist.GetHeadPosition();
	while (pos != 0)
	{
		CPartFile* cur_file = filelist.GetAt(pos);
		if (!cur_file)
			continue;

		UINT mycat = cur_file->GetCategory();
		if ((mycat>=min(from,to) && mycat<=max(from,to)))
		{
			//if ((from<to && (mycat<from || mycat>to)) || (from>to && (mycat>from || mycat<to)) )	continue; //not affected

			if (mycat == from)
				cur_file->SetCategory(to);
			else{
				if (from < to)
					cur_file->SetCategory(mycat - 1);
				else
					cur_file->SetCategory(mycat + 1);
			}
		}
		filelist.GetNext(pos);
	}
}

UINT CDownloadQueue::GetDownloadingFileCount() const
{
	UINT result = 0;
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		UINT uStatus = filelist.GetNext(pos)->GetStatus();
		if (uStatus == PS_READY || uStatus == PS_EMPTY)
			result++;
	}
	return result;
}

UINT CDownloadQueue::GetPausedFileCount() const
{
	UINT result = 0;
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		if (filelist.GetNext(pos)->GetStatus() == PS_PAUSED)
			result++;
	}
	return result;
}

// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
/*
void CDownloadQueue::SetAutoCat(CPartFile* newfile){
	if(thePrefs.GetCatCount()==1)
		return;
	CString catExt;

	for (int ix=1;ix<thePrefs.GetCatCount();ix++){	
		catExt= thePrefs.GetCategory(ix)->autocat;
		if (catExt.IsEmpty())
			continue;

		if (!thePrefs.GetCategory(ix)->ac_regexpeval) {
			// simple string comparison

			int curPos = 0;
			catExt.MakeLower();

			CString fullname = newfile->GetFileName();
			fullname.MakeLower();
			CString cmpExt = catExt.Tokenize(_T("|"), curPos);

			while (!cmpExt.IsEmpty()) {
				// HoaX_69: Allow wildcards in autocat string
				//  thanks to: bluecow, khaos and SlugFiller
				if(cmpExt.Find(_T("*")) != -1 || cmpExt.Find(_T("?")) != -1){
					// Use wildcards
					if(PathMatchSpec(fullname, cmpExt)){
						newfile->SetCategory(ix);
						return;
					}
				}else{
					if(fullname.Find(cmpExt) != -1){
						newfile->SetCategory(ix);
						return;
					}
				}
				cmpExt = catExt.Tokenize(_T("|"),curPos);
			}
		} else {
			// regular expression evaluation
			if (RegularExpressionMatch(catExt,newfile->GetFileName()))
			{
				newfile->SetCategory(ix);
				return; //zz_fly :: Avi3k: fix cat assign
			}
		}
	}
}
*/
// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

void CDownloadQueue::ResetLocalServerRequests()
{
	m_dwNextTCPSrcReq = 0;
	m_localServerReqQueue.RemoveAll();

	POSITION pos = filelist.GetHeadPosition();
	while (pos != NULL)
	{ 
		CPartFile* pFile = filelist.GetNext(pos);
		UINT uState = pFile->GetStatus();
		if (uState == PS_READY || uState == PS_EMPTY)
			pFile->ResumeFile();
		pFile->m_bLocalSrcReqQueued = false;
	}
}

void CDownloadQueue::RemoveLocalServerRequest(CPartFile* pFile)
{
	POSITION pos1, pos2;
	for( pos1 = m_localServerReqQueue.GetHeadPosition(); ( pos2 = pos1 ) != NULL; )
	{
		m_localServerReqQueue.GetNext(pos1);
		if (m_localServerReqQueue.GetAt(pos2) == pFile)
		{
			m_localServerReqQueue.RemoveAt(pos2);
			pFile->m_bLocalSrcReqQueued = false;
			// could 'break' here.. fail safe: go through entire list..
		}
	}
}

void CDownloadQueue::ProcessLocalRequests()
{
	if ( (!m_localServerReqQueue.IsEmpty()) && (m_dwNextTCPSrcReq < ::GetTickCount()) )
	{
		CSafeMemFile dataTcpFrame(22);
		const int iMaxFilesPerTcpFrame = 15;
		int iFiles = 0;
		while (!m_localServerReqQueue.IsEmpty() && iFiles < iMaxFilesPerTcpFrame)
		{
			// find the file with the longest waitingtime
			POSITION pos1, pos2;
			uint32 dwBestWaitTime = 0xFFFFFFFF;
			POSITION posNextRequest = NULL;
			CPartFile* cur_file;
			for( pos1 = m_localServerReqQueue.GetHeadPosition(); ( pos2 = pos1 ) != NULL; ){
				m_localServerReqQueue.GetNext(pos1);
				cur_file = m_localServerReqQueue.GetAt(pos2);
				if (cur_file->GetStatus() == PS_READY || cur_file->GetStatus() == PS_EMPTY)
				{
					uint8 nPriority = cur_file->GetDownPriority();
					if (nPriority > PR_HIGH){
						ASSERT(0);
						nPriority = PR_HIGH;
					}

					if (cur_file->m_LastSearchTime + (PR_HIGH-nPriority) < dwBestWaitTime ){
						dwBestWaitTime = cur_file->m_LastSearchTime + (PR_HIGH-nPriority);
						posNextRequest = pos2;
					}
				}
				else{
					m_localServerReqQueue.RemoveAt(pos2);
					cur_file->m_bLocalSrcReqQueued = false;
					if (thePrefs.GetDebugSourceExchange())
						AddDebugLogLine(false, _T("SXSend: Local server source request for file \"%s\" not sent because of status '%s'"), cur_file->GetFileName(), cur_file->getPartfileStatus());
				}
			}

			if (posNextRequest != NULL)
			{
				cur_file = m_localServerReqQueue.GetAt(posNextRequest);
				cur_file->m_bLocalSrcReqQueued = false;
				cur_file->m_LastSearchTime = ::GetTickCount();
				m_localServerReqQueue.RemoveAt(posNextRequest);

				if (cur_file->IsLargeFile() && (theApp.serverconnect->GetCurrentServer() == NULL || !theApp.serverconnect->GetCurrentServer()->SupportsLargeFilesTCP())){
					ASSERT( false );
					DebugLogError(_T("Large file (%s) on local requestqueue for server without support for large files"), cur_file->GetFileName());
					continue;
				}

				iFiles++;

				// create request packet
				CSafeMemFile smPacket;
				smPacket.WriteHash16(cur_file->GetFileHash());
				if (!cur_file->IsLargeFile()){
					smPacket.WriteUInt32((uint32)(uint64)cur_file->GetFileSize());
				}
				else{
					smPacket.WriteUInt32(0); // indicates that this is a large file and a uint64 follows
					smPacket.WriteUInt64(cur_file->GetFileSize());
				}

				uint8 byOpcode = 0;
				if (thePrefs.IsClientCryptLayerSupported() && theApp.serverconnect->GetCurrentServer() != NULL && theApp.serverconnect->GetCurrentServer()->SupportsGetSourcesObfuscation())
					byOpcode = OP_GETSOURCES_OBFU;
				else
					byOpcode = OP_GETSOURCES;

				Packet* packet = new Packet(&smPacket, OP_EDONKEYPROT, byOpcode);
				if (thePrefs.GetDebugServerTCPLevel() > 0)
					Debug(_T(">>> Sending OP__GetSources%s(%2u/%2u); %s\n"), (byOpcode == OP_GETSOURCES) ? _T("") : _T("_OBFU"), iFiles, iMaxFilesPerTcpFrame, DbgGetFileInfo(cur_file->GetFileHash()));
				dataTcpFrame.Write(packet->GetPacket(), packet->GetRealPacketSize());
				delete packet;

				if (thePrefs.GetDebugSourceExchange())
					AddDebugLogLine(false, _T("SXSend: Local server source request; File=\"%s\""), cur_file->GetFileName());
			}
		}

		int iSize = (int)dataTcpFrame.GetLength();
		if (iSize > 0)
		{
			// create one 'packet' which contains all buffered OP_GETSOURCES eD2K packets to be sent with one TCP frame
			// server credits: 16*iMaxFilesPerTcpFrame+1 = 241
			Packet* packet = new Packet(new char[iSize], (UINT)dataTcpFrame.GetLength(), true, false);
			dataTcpFrame.Seek(0, CFile::begin);
			dataTcpFrame.Read(packet->GetPacket(), iSize);
			theStats.AddUpDataOverheadServer(packet->size);
			theApp.serverconnect->SendPacket(packet, true);
		}

		// next TCP frame with up to 15 source requests is allowed to be sent in..
		m_dwNextTCPSrcReq = ::GetTickCount() + SEC2MS(iMaxFilesPerTcpFrame*(16+4));
	}
}

void CDownloadQueue::SendLocalSrcRequest(CPartFile* sender){
	ASSERT ( !m_localServerReqQueue.Find(sender) );
	m_localServerReqQueue.AddTail(sender);
}

int CDownloadQueue::GetDownloadFilesStats(uint64 &rui64TotalFileSize,
									      uint64 &rui64TotalLeftToTransfer,
									      uint64 &rui64TotalAdditionalNeededSpace)
{
	int iActiveFiles = 0;
	for (POSITION pos = filelist.GetHeadPosition();pos != 0; )
	{
		const CPartFile* cur_file = filelist.GetNext(pos);
		UINT uState = cur_file->GetStatus();
		if (uState == PS_READY || uState == PS_EMPTY)
		{
			uint64 ui64LeftToTransfer = 0;
			uint64 ui64AdditionalNeededSpace = 0;
			cur_file->GetLeftToTransferAndAdditionalNeededSpace(ui64LeftToTransfer, ui64AdditionalNeededSpace);
			rui64TotalFileSize += (uint64)cur_file->GetFileSize();
			rui64TotalLeftToTransfer += ui64LeftToTransfer;
			rui64TotalAdditionalNeededSpace += ui64AdditionalNeededSpace;
			iActiveFiles++;
		}
	}
	return iActiveFiles;
}

///////////////////////////////////////////////////////////////////////////////
// CSourceHostnameResolveWnd

#define WM_HOSTNAMERESOLVED		(WM_USER + 0x101)	// does not need to be placed in "UserMsgs.h"

BEGIN_MESSAGE_MAP(CSourceHostnameResolveWnd, CWnd)
	ON_MESSAGE(WM_HOSTNAMERESOLVED, OnHostnameResolved)
END_MESSAGE_MAP()

CSourceHostnameResolveWnd::CSourceHostnameResolveWnd()
{
}

CSourceHostnameResolveWnd::~CSourceHostnameResolveWnd()
{
	while (!m_toresolve.IsEmpty())
		delete m_toresolve.RemoveHead();
}

void CSourceHostnameResolveWnd::AddToResolve(const uchar* fileid, LPCSTR pszHostname, uint16 port, LPCTSTR pszURL)
{
	bool bResolving = !m_toresolve.IsEmpty();

	// double checking
	if (!theApp.downloadqueue->GetFileByID(fileid))
		return;

	Hostname_Entry* entry = new Hostname_Entry;
	md4cpy(entry->fileid, fileid);
	entry->strHostname = pszHostname;
	entry->port = port;
	entry->strURL = pszURL;
	m_toresolve.AddTail(entry);

	if (bResolving)
		return;

	memset(m_aucHostnameBuffer, 0, sizeof(m_aucHostnameBuffer));
	if (WSAAsyncGetHostByName(m_hWnd, WM_HOSTNAMERESOLVED, entry->strHostname, m_aucHostnameBuffer, sizeof m_aucHostnameBuffer) != 0)
		return;
	m_toresolve.RemoveHead();
	delete entry;
}

LRESULT CSourceHostnameResolveWnd::OnHostnameResolved(WPARAM /*wParam*/, LPARAM lParam)
{
	if (m_toresolve.IsEmpty())
		return TRUE;
	Hostname_Entry* resolved = m_toresolve.RemoveHead();
	if (WSAGETASYNCERROR(lParam) == 0)
	{
		int iBufLen = WSAGETASYNCBUFLEN(lParam);
		if (iBufLen >= sizeof(HOSTENT))
		{
			LPHOSTENT pHost = (LPHOSTENT)m_aucHostnameBuffer;
			if (pHost->h_length == 4 && pHost->h_addr_list && pHost->h_addr_list[0])
			{
				uint32 nIP = ((LPIN_ADDR)(pHost->h_addr_list[0]))->s_addr;

				CPartFile* file = theApp.downloadqueue->GetFileByID(resolved->fileid);
				if (file)
				{
					if (resolved->strURL.IsEmpty())
					{
					    CSafeMemFile sources(1+4+2);
					    sources.WriteUInt8(1);
					    sources.WriteUInt32(nIP);
					    sources.WriteUInt16(resolved->port);
					    sources.SeekToBegin();
					    file->AddSources(&sources,0,0, false);
				    }
					else
					{
						file->AddSource(resolved->strURL, nIP);
					}
				}
			}
		}
	}
	delete resolved;

	while (!m_toresolve.IsEmpty())
	{
		Hostname_Entry* entry = m_toresolve.GetHead();
		memset(m_aucHostnameBuffer, 0, sizeof(m_aucHostnameBuffer));
		if (WSAAsyncGetHostByName(m_hWnd, WM_HOSTNAMERESOLVED, entry->strHostname, m_aucHostnameBuffer, sizeof m_aucHostnameBuffer) != 0)
			return TRUE;
		m_toresolve.RemoveHead();
		delete entry;
	}
	return TRUE;
}

bool CDownloadQueue::DoKademliaFileRequest()
{
	return ((::GetTickCount() - lastkademliafilerequest) > KADEMLIAASKTIME);
}

void CDownloadQueue::KademliaSearchFile(uint32 searchID, const Kademlia::CUInt128* pcontactID, const Kademlia::CUInt128* pbuddyID, uint8 type, uint32 ip, uint16 tcp, uint16 udp, uint32 dwBuddyIP, uint16 dwBuddyPort, uint8 byCryptOptions)
{
	//Safty measure to make sure we are looking for these sources
	CPartFile* temp = GetFileByKadFileSearchID(searchID);
	if( !temp )
		return;
	//Do we need more sources?
	if(!(!temp->IsStopped() && temp->GetMaxSources() > temp->GetSourceCount()))
		return;

	//Xman GlobalMaxHarlimit for fairness
	if(temp->IsGlobalSourceAddAllowed()==false)
		return;
	//Xman end

	uint32 ED2Kip = ntohl(ip);
	if (theApp.ipfilter->IsFiltered(ED2Kip))
	{
		if (thePrefs.GetLogFilteredIPs())
			//Xman show filename
			/*
			AddDebugLogLine(false, _T("IPfiltered source IP=%s (%s) received from Kademlia"), ipstr(ED2Kip), theApp.ipfilter->GetLastHit());
			*/
			AddDebugLogLine(false, _T("IPfiltered source IP=%s (%s) received from Kademlia for File: %s"), ipstr(ED2Kip), theApp.ipfilter->GetLastHit(), temp->GetFileName());
			//Xman end
		return;
	}

	//zz_fly :: skip banned source
	// X-Ray :: Optimizations :: Start
	if (theApp.clientlist->IsBannedClient(ED2Kip)){
		if (thePrefs.GetLogFilteredIPs() && thePrefs.GetLogBannedClients())
			AddDebugLogLine(false, _T("Banned source IP=%s (%s) received from Kademlia"), ipstr(ED2Kip), theApp.ipfilter->GetLastHit());
		return;
	}
	// X-Ray :: Optimizations :: End

	if( (ip == Kademlia::CKademlia::GetIPAddress() || ED2Kip == theApp.serverconnect->GetClientID()) && tcp == thePrefs.GetPort())
		return;
	CUpDownClient* ctemp = NULL; 
	//DEBUG_ONLY( DebugLog(_T("Kadsource received, type %u, IP %s"), type, ipstr(ED2Kip)) );
	switch( type )
	{
		case 4:
		case 1:
		{
			//NonFirewalled users
			if(!tcp)
			{
				if (thePrefs.GetVerbose())
					AddDebugLogLine(false, _T("Ignored source (IP=%s) received from Kademlia, no tcp port received"), ipstr(ip));
				return;
			}
			ctemp = new CUpDownClient(temp,tcp,ip,0,0,false);
			ctemp->SetSourceFrom(SF_KADEMLIA);
			// not actually sent or needed for HighID sources
			//ctemp->SetServerIP(serverip);
			//ctemp->SetServerPort(serverport);
			ctemp->SetKadPort(udp);
			byte cID[16];
			pcontactID->ToByteArray(cID);
			ctemp->SetUserHash(cID);
			break;
		}
		case 2:
		{
			//Don't use this type... Some clients will process it wrong..
			break;
		}
		case 5:
		case 3:
		{
			//This will be a firewaled client connected to Kad only.
			// if we are firewalled ourself, the source is useless to us
			if (theApp.IsFirewalled())
				break;

			//We set the clientID to 1 as a Kad user only has 1 buddy.
			ctemp = new CUpDownClient(temp,tcp,1,0,0,false);
			//The only reason we set the real IP is for when we get a callback
			//from this firewalled source, the compare method will match them.
			ctemp->SetSourceFrom(SF_KADEMLIA);
			ctemp->SetKadPort(udp);
			byte cID[16];
			pcontactID->ToByteArray(cID);
			ctemp->SetUserHash(cID);
			pbuddyID->ToByteArray(cID);
			ctemp->SetBuddyID(cID);
			ctemp->SetBuddyIP(dwBuddyIP);
			ctemp->SetBuddyPort(dwBuddyPort);
			break;
		}
		case 6:
		{
			// firewalled source which supports direct udp callback
			// if we are firewalled ourself, the source is useless to us
			if (theApp.IsFirewalled())
				break;

			if ((byCryptOptions & 0x08) == 0){
				DebugLogWarning(_T("Received Kad source type 6 (direct callback) which has the direct callback flag not set (%s)"), ipstr(ED2Kip));
				break;
			}
			ctemp = new CUpDownClient(temp, tcp, 1, 0, 0, false);
			ctemp->SetSourceFrom(SF_KADEMLIA);
			ctemp->SetKadPort(udp);
			ctemp->SetIP(ED2Kip); // need to set the Ip address, which cannot be used for TCP but for UDP
			byte cID[16];
			pcontactID->ToByteArray(cID);
			ctemp->SetUserHash(cID);
		}
	}

	if (ctemp != NULL){
		// add encryption settings
		ctemp->SetConnectOptions(byCryptOptions);
		CheckAndAddSource(temp, ctemp);
	}
}

void CDownloadQueue::ExportPartMetFilesOverview() const
{
	CString strFileListPath = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("downloads.txt");
	
	CString strTmpFileListPath = strFileListPath;
	PathRenameExtension(strTmpFileListPath.GetBuffer(MAX_PATH), _T(".tmp"));
	strTmpFileListPath.ReleaseBuffer();

	CSafeBufferedFile file;
	CFileException fexp;
	if (!file.Open(strTmpFileListPath, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareDenyWrite, &fexp))
	{
		CString strError;
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(_T("Failed to create part.met file list%s"), strError);
		return;
	}

	// write Unicode byte-order mark 0xFEFF
	fputwc(0xFEFF, file.m_pStream);

	try
	{
		file.printf(_T("Date:      %s\r\n"), CTime::GetCurrentTime().Format(_T("%c")));
		if (thePrefs.GetTempDirCount()==1)
			file.printf(_T("Directory: %s\r\n"), thePrefs.GetTempDir());
		file.printf(_T("\r\n"));
		file.printf(_T("Part file\teD2K link\r\n"));
		file.printf(_T("--------------------------------------------------------------------------------\r\n"));
		for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
		{
			const CPartFile* pPartFile = filelist.GetNext(pos);
			if (pPartFile->GetStatus(true) != PS_COMPLETE)
			{
				CString strPartFilePath(pPartFile->GetFilePath());
				TCHAR szNam[_MAX_FNAME];
				TCHAR szExt[_MAX_EXT];
				_tsplitpath(strPartFilePath, NULL, NULL, szNam, szExt);
				if (thePrefs.GetTempDirCount()==1)
					file.printf(_T("%s%s\t%s\r\n"), szNam, szExt, pPartFile->GetED2kLink());
				else
					file.printf(_T("%s\t%s\r\n"), pPartFile->GetFullName(), pPartFile->GetED2kLink());
			}
		}

		if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !theApp.emuledlg->IsRunning())){
			file.Flush(); // flush file stream buffers to disk buffers
			if (_commit(_fileno(file.m_pStream)) != 0) // commit disk buffers to disk
				AfxThrowFileException(CFileException::hardIO, GetLastError(), file.GetFileName());
		}
		file.Close();

		CString strBakFileListPath = strFileListPath;
		PathRenameExtension(strBakFileListPath.GetBuffer(MAX_PATH), _T(".bak"));
		strBakFileListPath.ReleaseBuffer();

		if (_taccess(strBakFileListPath, 0) == 0)
			CFile::Remove(strBakFileListPath);
		if (_taccess(strFileListPath, 0) == 0)
			CFile::Rename(strFileListPath, strBakFileListPath);
		CFile::Rename(strTmpFileListPath, strFileListPath);
	}
	catch(CFileException* e)
	{
		CString strError;
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (e->GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(_T("Failed to write part.met file list%s"), strError);
		e->Delete();
		file.Abort();
		(void)_tremove(file.GetFilePath());
	}
}

void CDownloadQueue::OnConnectionState(bool bConnected)
{
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
	{
		CPartFile* pPartFile = filelist.GetNext(pos);
		if (pPartFile->GetStatus() == PS_READY || pPartFile->GetStatus() == PS_EMPTY)
			pPartFile->SetActive(bConnected);
	}
}

CString CDownloadQueue::GetOptimalTempDir(UINT nCat, EMFileSize nFileSize){
	// shortcut
	if (thePrefs.tempdir.GetCount() == 1)
		return thePrefs.GetTempDir();

	CMap<int, int, sint64, sint64> mapNeededSpaceOnDrive;
	CMap<int, int, sint64, sint64> mapFreeSpaceOnDrive;

	sint64 llBuffer = 0;
	sint64 llHighestFreeSpace = 0;
	int	nHighestFreeSpaceDrive = -1;
	// first collect the free space on drives
	for (int i = 0; i < thePrefs.tempdir.GetCount(); i++) {
		const int nDriveNumber = GetPathDriveNumber(thePrefs.GetTempDir(i));
		if (mapFreeSpaceOnDrive.Lookup(nDriveNumber, llBuffer))
			continue;
		llBuffer = GetFreeDiskSpaceX(thePrefs.GetTempDir(i)) - thePrefs.GetMinFreeDiskSpace();
		mapFreeSpaceOnDrive.SetAt(nDriveNumber, llBuffer);
		if (llBuffer > llHighestFreeSpace){
			nHighestFreeSpaceDrive = nDriveNumber;
			llHighestFreeSpace = llBuffer;
		}

	}

	// now get the space we would need to download all files in the current queue
	POSITION pos = filelist.GetHeadPosition();
	while (pos != NULL){
		CPartFile* pCurFile =  filelist.GetNext(pos);
		const int nDriveNumber = GetPathDriveNumber(pCurFile->GetTempPath());

		sint64 llNeededForCompletion = 0;
		switch(pCurFile->GetStatus(false)){
			case PS_READY:
			case PS_EMPTY:
			case PS_WAITINGFORHASH:
			case PS_INSUFFICIENT:
				llNeededForCompletion = pCurFile->GetFileSize() - pCurFile->GetRealFileSize();
				if (llNeededForCompletion < 0)
					llNeededForCompletion = 0;
		}
		llBuffer = 0;
		mapNeededSpaceOnDrive.Lookup(nDriveNumber, llBuffer);
		llBuffer += llNeededForCompletion;
		mapNeededSpaceOnDrive.SetAt(nDriveNumber, llBuffer);
	}

	sint64 llHighestTotalSpace = 0;
	int	nHighestTotalSpaceDir = -1;
	int	nHighestFreeSpaceDir = -1;
	int	nAnyAvailableDir = -1;
	// first round (0): on same drive as incomming and enough space for all downloading
	// second round (1): enough space for all downloading
	// third round (2): most actual free space
	for (int i = 0; i < thePrefs.tempdir.GetCount(); i++) {
		const int nDriveNumber = GetPathDriveNumber(thePrefs.GetTempDir(i));
		llBuffer = 0;

		sint64 llAvailableSpace = 0;
		mapFreeSpaceOnDrive.Lookup(nDriveNumber, llAvailableSpace);
		mapNeededSpaceOnDrive.Lookup(nDriveNumber, llBuffer);
		llAvailableSpace -= llBuffer;

		// no condition can be met for a large file on a FAT volume
		if (nFileSize <= OLD_MAX_EMULE_FILE_SIZE || !IsFileOnFATVolume(thePrefs.GetTempDir(i))){
			// condition 0
			// needs to be same drive and enough space
			if (GetPathDriveNumber(thePrefs.GetCatPath(nCat)) == nDriveNumber &&
				llAvailableSpace > (sint64)nFileSize)
			{
				//this one is perfect
				return thePrefs.GetTempDir(i);
			}
			// condition 1
			// needs to have enough space for downloading
			if (llAvailableSpace > (sint64)nFileSize && llAvailableSpace > llHighestTotalSpace){
				llHighestTotalSpace = llAvailableSpace;
				nHighestTotalSpaceDir = i;
			}
			// condition 2
			// first one which has the highest actualy free space
			if ( nDriveNumber == nHighestFreeSpaceDrive && nHighestFreeSpaceDir == (-1)){
				nHighestFreeSpaceDir = i;
			}
			// condition 3
			// any directory which can be used for this file (ak not FAT for large files)
			if ( nAnyAvailableDir == (-1)){
				nAnyAvailableDir = i;
			}
		}
	}

	if (nHighestTotalSpaceDir != (-1)){	 //condtion 0 was apperently too much, take 1
		return thePrefs.GetTempDir(nHighestTotalSpaceDir);
	}
	else if (nHighestFreeSpaceDir != (-1)){ // condtion 1 could not be met too, take 2
		return thePrefs.GetTempDir(nHighestFreeSpaceDir);
	}
	else if( nAnyAvailableDir != (-1)){
		return thePrefs.GetTempDir(nAnyAvailableDir);
	}
	else{ // so was condtion 2 and 3, take 4.. wait there is no 3 - this must be a bug
		ASSERT( false );
		return thePrefs.GetTempDir();
	}
}

void CDownloadQueue::RefilterAllComments(){
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		cur_file->RefilterFileComments();
	}
}

// BEGIN SLUGFILLER: SafeHash
bool CDownloadQueue::IsTempFile(const CString& , const CString& rstrName) const
{
	// do not share a part file from the temp directory, if there is still a corresponding entry in
	// the download queue -- because that part file is not yet complete.
	CString othername = rstrName + _T(".met");
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		if (!othername.CompareNoCase(cur_file->GetPartMetFileName()))
			return true;
	}
	return false;
}
// END SLUGFILLER: SafeHash

// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
void CDownloadQueue::CompDownloadRate(){
	// Compute the download datarate of all clients
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; ){
		filelist.GetNext(pos)->CompDownloadRate();
	}
}
// Maella end

#ifdef PRINT_STATISTIC
void CDownloadQueue::PrintStatistic()
{
	uint32 Savedsources=0;
	uint32 Gaplist=0;
	uint32 Requestedblocklist=0;
	uint32 SrcpartFrequency=0;
	uint32 BufferedData=0;
	uint32 A4AFsrclist=0;
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; ){
		CPartFile* cur_file = filelist.GetNext(pos);
		Savedsources += cur_file->GetSavedSources();
		Gaplist += cur_file->GetGapList();
		Requestedblocklist += cur_file->GetRequestedBlocklist();
		SrcpartFrequency += cur_file->GetSrcpartFrequency();
		BufferedData += cur_file->GetBufferedData();
		A4AFsrclist += cur_file->GetA4AFsrclist();
	}
	AddLogLine(false, _T("sum of all paartfiles at downloadqueue:"));
	AddLogLine(false, _T("Savedsources: %u"), Savedsources);
	AddLogLine(false, _T("Gaplist: %u"), Gaplist);
	AddLogLine(false, _T("Requestedblocklist: %u"), Requestedblocklist);
	AddLogLine(false, _T("SrcpartFrequency: %u"), SrcpartFrequency);
	AddLogLine(false, _T("BufferedData: %u"), BufferedData);
	AddLogLine(false, _T("A4AFsrclist: %u"), A4AFsrclist);
	AddLogLine(false, _T("internal global sources: %u"), GetGlobalSources());
	AddLogLine(false, _T("---------------------------------------"));
}
#endif

// ==> File Settings [sivka/Stulle] - Stulle
void CDownloadQueue::InitTempVariables(CPartFile* file)
{
	thePrefs.SetTakeOverFileSettings(false);

	thePrefs.SetMaxSourcesPerFileTemp((uint16)(file->GetPrivateMaxSources())); // hardlimit - Stulle
	thePrefs.SetEnableAutoDropNNSTemp(file->GetEnableAutoDropNNS());
	thePrefs.SetAutoNNS_TimerTemp(file->GetAutoNNS_Timer());
	thePrefs.SetMaxRemoveNNSLimitTemp(file->GetMaxRemoveNNSLimit());
	thePrefs.SetEnableAutoDropFQSTemp(file->GetEnableAutoDropFQS());
	thePrefs.SetAutoFQS_TimerTemp(file->GetAutoFQS_Timer());
	thePrefs.SetMaxRemoveFQSLimitTemp(file->GetMaxRemoveFQSLimit());
	thePrefs.SetEnableAutoDropQRSTemp(file->GetEnableAutoDropQRS());
	thePrefs.SetAutoHQRS_TimerTemp(file->GetAutoHQRS_Timer());
	thePrefs.SetMaxRemoveQRSTemp(file->GetMaxRemoveQRS());
	thePrefs.SetMaxRemoveQRSLimitTemp(file->GetMaxRemoveQRSLimit());
	thePrefs.SetHQRXmanTemp(file->GetHQRXman());
	thePrefs.SetGlobalHlTemp(file->GetGlobalHL()); // Global Source Limit (customize for files) - Stulle
}

void CDownloadQueue::UpdateFileSettings(CPartFile* file)
{
	if(thePrefs.GetMaxSourcesPerFileTakeOver()) // hardlimit - Stulle
		file->SetPrivateMaxSources(thePrefs.GetMaxSourcesPerFileTemp()); // link Sivka File settings and official HL limits - Stulle
	if(thePrefs.GetEnableAutoDropNNSTakeOver())
		file->SetEnableAutoDropNNS(thePrefs.GetEnableAutoDropNNSTemp());
	if(thePrefs.GetAutoNNS_TimerTakeOver())
		file->SetAutoNNS_Timer(thePrefs.GetAutoNNS_TimerTemp());
	if(thePrefs.GetMaxRemoveNNSLimitTakeOver())
		file->SetMaxRemoveNNSLimit(thePrefs.GetMaxRemoveNNSLimitTemp());
	if(thePrefs.GetEnableAutoDropFQSTakeOver())
		file->SetEnableAutoDropFQS(thePrefs.GetEnableAutoDropFQSTemp());
	if(thePrefs.GetAutoFQS_TimerTakeOver())
		file->SetAutoFQS_Timer(thePrefs.GetAutoFQS_TimerTemp());
	if(thePrefs.GetMaxRemoveFQSLimitTakeOver())
		file->SetMaxRemoveFQSLimit(thePrefs.GetMaxRemoveFQSLimitTemp());
	if(thePrefs.GetEnableAutoDropQRSTakeOver())
		file->SetEnableAutoDropQRS(thePrefs.GetEnableAutoDropQRSTemp());
	if(thePrefs.GetAutoHQRS_TimerTakeOver())
		file->SetAutoHQRS_Timer(thePrefs.GetAutoHQRS_TimerTemp());
	if(thePrefs.GetMaxRemoveQRSTakeOver())
		file->SetMaxRemoveQRS(thePrefs.GetMaxRemoveQRSTemp());
	if(thePrefs.GetMaxRemoveQRSLimitTakeOver())
		file->SetMaxRemoveQRSLimit(thePrefs.GetMaxRemoveQRSLimitTemp());
	// ==> Global Source Limit (customize for files) - Stulle
	if(thePrefs.GetGlobalHlTakeOver())
		file->SetGlobalHL(thePrefs.GetGlobalHlTemp());
	// <== Global Source Limit (customize for files) - Stulle
	if(thePrefs.GetHQRXmanTakeOver())
		file->SetHQRXman(thePrefs.GetHQRXmanTemp());
}

#define SAVE_WAIT_TIME 5 // time we wait until we actually save
void CDownloadQueue::SaveFileSettings(bool bStart)
{
	if(bStart)
	{
		if (m_SaveSettingsThread == NULL)
		{
			m_SaveSettingsThread = new CSaveSettingsThread();
			m_dwLastSave = ::GetTickCount();
		}
		else
		{
			if((m_dwLastSave + SEC2MS(SAVE_WAIT_TIME)) < ::GetTickCount())
				m_bSaveAgain = true;
			m_dwLastSave = ::GetTickCount();
			m_SaveSettingsThread->KeepWaiting();
		}
	}
	else
	{
		if(m_bSaveAgain)
		{
			m_bSaveAgain = false;
			m_SaveSettingsThread->Pause(false);
		}
		else if (m_SaveSettingsThread) // just in case, should always be true at this point
		{
			m_SaveSettingsThread->EndThread();
			delete m_SaveSettingsThread;
			m_SaveSettingsThread = NULL;
		}
	}
}

// Save settings thread to avoid locking GUI
CSaveSettingsThread::CSaveSettingsThread(void) {
	threadEndedEvent = new CEvent(0, 1);
	pauseEvent = new CEvent(TRUE, TRUE);
	waitEvent = new CEvent(TRUE, FALSE);

	bDoRun = true;
	bDoWait = true;
	m_dwLastWait = 0;
	AfxBeginThread(RunProc,(LPVOID)this,THREAD_PRIORITY_LOWEST);
}

CSaveSettingsThread::~CSaveSettingsThread(void) {
	EndThread();
	delete threadEndedEvent;
	delete pauseEvent;
	delete waitEvent;
}

void CSaveSettingsThread::EndThread() {
	if(!bDoRun) // we are trying to stop already
		return;

	// signal the thread to stop looping and exit.
	bDoRun = false;
	bDoWait = false;

	Pause(false);
	waitEvent->SetEvent();

	// wait for the thread to signal that it has stopped looping.
	threadEndedEvent->Lock();
}

void CSaveSettingsThread::Pause(bool paused) {
	if(paused) {
		pauseEvent->ResetEvent();
	} else {
		pauseEvent->SetEvent();
    }
}

UINT AFX_CDECL CSaveSettingsThread::RunProc(LPVOID pParam)
{
	DbgSetThreadName("CSaveSettingsThread");

	CSaveSettingsThread* savesettingsthread = (CSaveSettingsThread*)pParam;

	return savesettingsthread->RunInternal();
}

UINT CSaveSettingsThread::RunInternal()
{
	while (bDoWait)
	{
		if(m_dwLastWait == 0) // initial wait
			waitEvent->Lock(SEC2MS(SAVE_WAIT_TIME));
		else if(m_dwLastWait + SEC2MS(SAVE_WAIT_TIME) > ::GetTickCount()) // we have not waited enough since last keep message
			waitEvent->Lock(m_dwLastWait + SEC2MS(SAVE_WAIT_TIME) - ::GetTickCount()); // so wait until the time is up
		else // we waited enough, do the actual run now
		{
			bDoWait = false;
			if(bDoRun == false) // what? canceling this thread before saving
				m_SettingsSaver.SaveSettings(); // save!
		}
	}

	while(bDoRun) 
	{
		if(m_SettingsSaver.SaveSettings()) // if this fails we need to run again
		{
			Pause(true);
			PostMessage(theApp.emuledlg->m_hWnd,TM_SAVEDONE,0,0);
		}
		pauseEvent->Lock();
	}

	threadEndedEvent->SetEvent();

	return 0;
}
// <== File Settings [sivka/Stulle] - Stulle

//==> Global Source Limit [Max/Stulle] - Stulle
/****************************\
* Function written by Stulle *
* Used ideas from MaxUpload  *
\****************************/
void CDownloadQueue::SetHardLimits()
{
/********************\
* Set the variables  *
\********************/

	uint16 aCount = 0;
	UINT countsources = 0;
	UINT m_uGlobalHardlimit = 0;
	if (thePrefs.m_bAcceptsourcelimit && thePrefs.m_uMaxGlobalSources < thePrefs.GetGlobalHL())
		m_uGlobalHardlimit = thePrefs.m_uMaxGlobalSources;
	else
		m_uGlobalHardlimit = thePrefs.GetGlobalHL();
	uint16 m_uTollerance = (uint16)(m_uGlobalHardlimit*.05);
	UINT m_uSourcesDif = 0;
	bool m_bTooMuchSrc = false;
	bool m_bPassiveModeTemp = false;

/********************\
* Count Src & files  *
\********************/

	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		if (cur_file->GetStatus() == PS_READY || cur_file->GetStatus() == PS_EMPTY)
		{
			if(thePrefs.GetGlobalHlAll() || cur_file->GetGlobalHL())
				aCount++;

			 countsources += cur_file->GetSourceCount();

			 // just a safety check
			 if (cur_file->GetFileHardLimit() < cur_file->GetSourceCount())
				 cur_file->DecrHL(); // HL = SrcCount
		}
	}

	if (aCount == 0) // nothing to check here
	{
		m_bGlobalHLSrcReqAllowed = true;
		m_dwUpdateHL = ::GetTickCount();
		return;
	}

/********************\
* Get Src difference *
\********************/

	if (m_uGlobalHardlimit < countsources) // get pos result
	{
		m_uSourcesDif = countsources - m_uGlobalHardlimit;
		m_bTooMuchSrc = true;
	}
	else
		m_uSourcesDif = m_uGlobalHardlimit - countsources;

/********************\
* Use passive mode   *
\********************/

	if(m_uSourcesDif < m_uTollerance)
	{
		if(m_bPassiveMode == false)
		{
			// First time in passive mode for this round! What we need to do now is increase
			// the recheck time. Next we need to remember we had our first time passive mode.
			// If we enter passive mode we add the difference to the max range to the number
			// of current sources so we won't exceed the limit. One more var to recognize we
			// entered the passive mode in this cycle and we are finished.
			m_dwUpdateHlTime = 300000; // 300 sec = 5 min
			m_bPassiveMode = true;
			m_bPassiveModeTemp = true;
			m_bGlobalHLSrcReqAllowed = true;
			m_uSourcesDif = ((m_uGlobalHardlimit + m_uTollerance) - countsources)/aCount;
			AddDebugLogLine(true,_T("{GSL} Global source count is in the tolerance range! PassiveMode!"));
		}
		else
		{
			m_bGlobalHLSrcReqAllowed = true;
			m_dwUpdateHL = ::GetTickCount();
			return;
		}
	}

/********************\
* Calc HL changes    *
\********************/

	else
	{
		if(m_bPassiveMode == true)
		{
			m_dwUpdateHlTime = 50000; // 50 sec
			m_bPassiveMode = false;
			AddDebugLogLine(true,_T("{GSL} Global source count is not in the tolerance range! Disabled PassiveMode!"));
		}
		if(!m_bTooMuchSrc)
		{
			uint16 m_uMaxIncr = m_uTollerance/aCount;
			m_uSourcesDif /= aCount;

			if(m_uMaxIncr < m_uSourcesDif)
				m_uSourcesDif = m_uMaxIncr;

			m_bGlobalHLSrcReqAllowed = true;
		}
		else
			m_bGlobalHLSrcReqAllowed = false;
	}

/********************\
* Change Hardlimits  *
\********************/

	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		if ((cur_file->GetStatus() == PS_READY || cur_file->GetStatus() == PS_EMPTY) && (thePrefs.GetGlobalHlAll() || cur_file->GetGlobalHL()))
		{
			if(m_bPassiveModeTemp)
				cur_file->SetPassiveHL(m_uSourcesDif);
			else if(!m_bTooMuchSrc)
				cur_file->IncrHL(m_uSourcesDif);
			else
			cur_file->DecrHL();
		}
	}

	m_dwUpdateHL = ::GetTickCount();
	return;
}
// <== Global Source Limit [Max/Stulle] - Stulle

// ==> Show sources on title - Stulle
uint16 CDownloadQueue::GetGlobalSourceCount()
{
	uint16 m_uSourceCountTemp = 0;
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
	{
		const CPartFile* cur_file = filelist.GetNext(pos);

		m_uSourceCountTemp = (uint16)(m_uSourceCountTemp + cur_file->GetSourceCount());
	}
	return m_uSourceCountTemp;
}
// <== Show sources on title - Stulle

// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
// This function is used for the category commands Stop Last and Pause Last.
// This is a new function.
void CDownloadQueue::StopPauseLastFile(int Mode, int Category) {
	CPartFile*  pfile = NULL;
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;filelist.GetNext(pos)) {
		CPartFile* cur_file = filelist.GetAt(pos);
		if ((cur_file->GetStatus() < 4) && ((int)cur_file->GetCategory() == Category || Category == -1)) {
			if (!pfile)
				pfile = cur_file;
			else {
				if (cur_file->GetCatResumeOrder() > pfile->GetCatResumeOrder())
					pfile = cur_file;
				else if (cur_file->GetCatResumeOrder() == pfile->GetCatResumeOrder() && (cur_file->GetDownPriority() < pfile->GetDownPriority()))
					pfile = cur_file;
			}
		}
	}
	if (pfile) {
		Mode == MP_STOP ? pfile->StopFile() : pfile->PauseFile();
	}
}

// This function returns the highest resume order in a given category.
// It can be used to automatically assign a resume order to a partfile
// when it is added...  Or maybe it will have other uses in the future.
UINT CDownloadQueue::GetMaxCatResumeOrder(UINT iCategory /* = 0*/)
{
	UINT		max   = 0;
	
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;filelist.GetNext(pos))
	{
		CPartFile* cur_file = filelist.GetAt(pos);
		if (cur_file->GetCategory() == iCategory && cur_file->GetCatResumeOrder() > max)
			max = cur_file->GetCatResumeOrder();
	}

	return max;
}

// New function, used to add all of the
// ED2K links on the link queue to the downloads.  This is
// called when no new links have been added to the download
// list for half a second, or when there are queued links
// and the user has disabled the SelectCatForLinks feature.
bool CDownloadQueue::PurgeED2KLinkQueue()
{
	if (m_ED2KLinkQueue.IsEmpty()) return false;
	
	m_bBusyPurgingLinks = true;

	int	useCat;
	int		addedFiles = 0;
	bool	bCreatedNewCat = false;
	bool	bCanceled = false;
	if (thePrefs.SelectCatForNewDL() && thePrefs.GetCatCount()>1)
	{
		CSelCategoryDlg* getCatDlg = new CSelCategoryDlg((CWnd*)theApp.emuledlg);
		getCatDlg->DoModal();
		
		// Returns 0 on 'Cancel', otherwise it returns the selected category
		// or the index of a newly created category.  Users can opt to add the
		// links into a new category.
		useCat = getCatDlg->GetInput();
		bCreatedNewCat = getCatDlg->CreatedNewCat();
		bCanceled = getCatDlg->WasCancelled();
		delete getCatDlg;
	}
	else if (thePrefs.UseActiveCatForLinks())
		useCat = theApp.emuledlg->transferwnd->GetActiveCategory();
	else
		useCat = 0;

	for (POSITION pos = m_ED2KLinkQueue.GetHeadPosition(); pos != 0; m_ED2KLinkQueue.GetNext(pos))
	{
		CED2KFileLink*	pLink = m_ED2KLinkQueue.GetAt(pos);
		if (bCanceled) {
			delete pLink;
			pLink = NULL;
			continue;
		}
		if (!thePrefs.SelectCatForNewDL() && thePrefs.UseAutoCat())
		{
			useCat = GetAutoCat(CString(pLink->GetName()), pLink->GetSize());
			if (!useCat && thePrefs.UseActiveCatForLinks())
				useCat = theApp.emuledlg->transferwnd->GetActiveCategory();
		}
		CPartFile*		newfile =	new CPartFile(pLink, useCat);
		
		if (newfile->GetStatus() == PS_ERROR) 
		{
			delete newfile;
			newfile = NULL;
		}
		else
		{
			if (thePrefs.SmallFileDLPush() && newfile->GetFileSize() < (uint64)154624)
				newfile->SetCatResumeOrder(0);
			else if (thePrefs.AutoSetResumeOrder()) {
				newfile->SetCatResumeOrder(GetMaxCatResumeOrder(useCat)+1);
			}
			AddDownload(newfile,thePrefs.AddNewFilesPaused());
			addedFiles++;
		}

		CPartFile* partfile = newfile;
		if (partfile == NULL)
			partfile = GetFileByID(pLink->GetHashKey());
		if (partfile)
		{
			if (pLink->HasValidSources())
			partfile->AddClientSources(pLink->SourcesList,1,false);
			if (pLink->HasValidAICHHash() ){
				if ( !(partfile->GetAICHRecoveryHashSet()->HasValidMasterHash() && partfile->GetAICHRecoveryHashSet()->GetMasterHash() == pLink->GetAICHHash())){
					partfile->GetAICHRecoveryHashSet()->SetMasterHash(pLink->GetAICHHash(), AICH_VERIFIED);
					partfile->GetAICHRecoveryHashSet()->FreeHashSet();
				}
			}
		}

		if(pLink->HasHostnameSources())
		{
			POSITION pos = pLink->m_HostnameSourcesList.GetHeadPosition();
			while (pos != NULL)
			{
				const SUnresolvedHostname* pUnresHost = pLink->m_HostnameSourcesList.GetNext(pos);
				m_srcwnd.AddToResolve(pLink->GetHashKey(), pUnresHost->strHostname, pUnresHost->nPort, pUnresHost->strURL);
			}
		}
		// We're done with this link.
		delete pLink;
		pLink = NULL;
	}
	
	m_ED2KLinkQueue.RemoveAll();

	// This bit of code will resume the number of files that the user specifies in preferences (Off by default)
	if (thePrefs.StartDLInEmptyCats() > 0 && bCreatedNewCat && thePrefs.AddNewFilesPaused())
		for (int i = 0; i < thePrefs.StartDLInEmptyCats(); i++)
			if (!StartNextFile(useCat)) break;

	m_bBusyPurgingLinks = false;
	m_iLastLinkQueuedTick = 0;
	return true;
}

// Returns statistics about a category's files' states.
void CDownloadQueue::GetCategoryFileCounts(UINT uCategory, int cntFiles[])
{
	// cntFiles Array Indices:
	// 0 = Total
	// 1 = Transferring (Transferring Sources > 0)
	// 2 = Active (READY and EMPTY)
	// 3 = Unactive (PAUSED)
	// 4 = Complete (COMPLETE and COMPLETING)
	// 5 = Error (ERROR)
	// 6 = Other (Everything else...)
	for (int i = 0; i < 7; i++) cntFiles[i] = 0;

	for (POSITION pos = filelist.GetHeadPosition(); pos != NULL; filelist.GetNext(pos))
	{
		CPartFile* cur_file = filelist.GetAt(pos);
		if (cur_file->GetCategory() != uCategory) continue;

		cntFiles[0]++;
		if (cur_file->GetTransferringSrcCount() > 0) cntFiles[1]++;

		switch (cur_file->GetStatus(false))
		{
			case	PS_READY:
			case	PS_EMPTY:
				cntFiles[2]++;
				break;

			case	PS_PAUSED:
			//case	PS_INSUFFICIENT:
				cntFiles[3]++;
				break;

			case	PS_COMPLETING:
			case	PS_COMPLETE:
				cntFiles[4]++;
				break;

			case	PS_ERROR:
				cntFiles[5]++;
				break;
			
			case	PS_WAITINGFORHASH:
			case	PS_HASHING:
			case	PS_UNKNOWN:
				cntFiles[6]++;
				break;
		}
	}
}

// Returns the number of active files in a category.
UINT CDownloadQueue::GetCatActiveFileCount(UINT uCategory)
{
	UINT uCount = 0;

	for (POSITION pos = filelist.GetHeadPosition(); pos != NULL; filelist.GetNext(pos))
	{
		CPartFile* cur_file = filelist.GetAt(pos);
		if (cur_file->GetCategory() != uCategory) continue;

		switch (cur_file->GetStatus(false))
		{
			case	PS_READY:
			case	PS_EMPTY:
			case	PS_COMPLETING:
				uCount++;
				break;
			default:
				break;
		}
	}

	return uCount;
}

// Returns the number of files in a category.
UINT CDownloadQueue::GetCategoryFileCount(UINT uCategory)
{
	UINT uCount = 0;

	for (POSITION pos = filelist.GetHeadPosition(); pos != NULL; filelist.GetNext(pos))
	{
		CPartFile* cur_file = filelist.GetAt(pos);
		if (cur_file->GetCategory() == uCategory) uCount++;
	}

	return uCount;
}

// Returns the source count of the file with the highest available source count.
// nCat is optional and allows you to specify a certain category.
UINT CDownloadQueue::GetHighestAvailableSourceCount(int nCat)
{
	UINT nCount = 0;

	for (POSITION pos = filelist.GetHeadPosition(); pos != NULL; filelist.GetNext(pos))
	{
		CPartFile* curFile = filelist.GetAt(pos);
		if (nCount < curFile->GetAvailableSrcCount() && (nCat == -1 || (int)curFile->GetCategory() == nCat))
			nCount = curFile->GetAvailableSrcCount();
	}

	return nCount;
}

// GetAutoCat returns a category index of a category
// that passes the filters.
// Idea by HoaX_69.
UINT CDownloadQueue::GetAutoCat(CString sFullName, EMFileSize nFileSize)
{
	if (sFullName.IsEmpty())
		return 0;

	if (thePrefs.GetCatCount() <= 1)
		return 0;

	for (int ix = 1; ix < thePrefs.GetCatCount(); ix++)
	{ 
		Category_Struct* curCat = thePrefs.GetCategory(ix);
		if (!curCat->selectioncriteria.bAdvancedFilterMask && !curCat->selectioncriteria.bFileSize)
			continue;
		if (curCat->selectioncriteria.bAdvancedFilterMask && !ApplyFilterMask(sFullName, ix))
			continue;
		if (curCat->selectioncriteria.bFileSize && (nFileSize < curCat->viewfilters.nFSizeMin || (curCat->viewfilters.nFSizeMax == 0 || nFileSize > curCat->viewfilters.nFSizeMax)))
			continue;
		return ix;
	}

	return 0;
}

// Checks a part-file's "pretty filename" against a filter mask and returns
// true if it passes.  See read-me for details.
bool CDownloadQueue::ApplyFilterMask(CString sFullName, UINT nCat)
{
	CString sFilterMask = thePrefs.GetCategory(nCat)->viewfilters.sAdvancedFilterMask;
	sFilterMask.Trim();

	if (sFilterMask == "")
		return false;

		sFullName.MakeLower();
	sFilterMask.MakeLower();

	if (sFilterMask.Left(1) == "<")
		{
			bool bPassedGlobal[3];
			bPassedGlobal[0] = false;
			bPassedGlobal[1] = true;
			bPassedGlobal[2] = false;

			for (int i = 0; i < 3; i++)
			{
				int iStart = 0;
				switch (i)
				{
			case 0: iStart = sFilterMask.Find(_T("<all(")); break;
			case 1: iStart = sFilterMask.Find(_T("<any(")); break;
			case 2: iStart = sFilterMask.Find(_T("<none(")); break;
				}

				if (iStart == -1)
				{
					bPassedGlobal[i] = true; // We need to do this since not all criteria are needed in order to match the category.
					continue; // Skip this criteria block.
				}

				i !=2 ? (iStart += 5) : (iStart += 6);

			int iEnd = sFilterMask.Find(_T(")>"), iStart);
			int iLT = sFilterMask.Find(_T("<"), iStart);
			int iGT = sFilterMask.Find(_T(">"), iStart);

				if (iEnd == -1 || (iLT != -1 && iLT < iEnd) || iGT < iEnd)
				{
				AddDebugLogLine(false, _T("Category '%s' has invalid Category Mask String."), thePrefs.GetCategory(nCat)->strTitle);
					break; // Move on to next category.
				}
				if (iStart == iEnd)
				{
					bPassedGlobal[i] = true; // Just because this criteria block is empty doesn't mean the mask should fail.
					continue; // Skip this criteria block.
				}

			CString sSegment = sFilterMask.Mid(iStart, iEnd - iStart);

				int curPosBlock = 0;
				CString cmpSubBlock = sSegment.Tokenize(_T(":"), curPosBlock);

				while (cmpSubBlock != "")
				{
					bool bPassed = (i == 1) ? false : true;

					int curPosToken = 0;
					CString cmpSubStr = cmpSubBlock.Tokenize(_T("|"), curPosToken);

				while (cmpSubStr != "")
				{
					int cmpResult;

					if (cmpSubStr.Find(_T("*")) != -1 || cmpSubStr.Find(_T("?")) != -1)
						cmpResult = (wildcmp(cmpSubStr.GetBuffer(), sFullName.GetBuffer()) == 0) ? -1 : 1;
					else
						cmpResult = sFullName.Find(cmpSubStr);

					switch (i)
					{
							case 0:	if (cmpResult == -1) bPassed = false; break;
							case 1:	if (cmpResult != -1) bPassed = true; break;
							case 2:	if (cmpResult != -1) bPassed = false; break;
						}
						cmpSubStr = cmpSubBlock.Tokenize(_T("|"), curPosToken);
						}
					switch (i)
						{
						case 0:
						case 2: if (bPassed) bPassedGlobal[i] = true; break;
						case 1: if (!bPassed) bPassedGlobal[i] = false; break;
						}
					cmpSubBlock = sSegment.Tokenize(_T(":"), curPosBlock);
					}
				}
			for (int i = 0; i < 3; i++)
			if (!bPassedGlobal[i]) return false;
		return true;
		}
		else
		{
			int curPos = 0;
		CString cmpSubStr = sFilterMask.Tokenize(_T("|"), curPos);

			while (cmpSubStr != "")
			{
				int cmpResult;

				if (cmpSubStr.Find(_T("*")) != -1 || cmpSubStr.Find(_T("?")) != -1)
					cmpResult = (wildcmp(cmpSubStr.GetBuffer(), sFullName.GetBuffer()) == 0) ? -1 : 1;
				else
					cmpResult = sFullName.Find(cmpSubStr);

				if(cmpResult != -1)
				return true;
			cmpSubStr = sFilterMask.Tokenize(_T("|"), curPos);
			}
		}
	return false;
}
// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle