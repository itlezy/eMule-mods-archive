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
#include "TransferWnd.h"
#include "TaskbarNotifier.h"
#include "MenuCmds.h"
#include "Log.h"
#include "Neo/ClientFileStatus.h"// NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
#include "sharedfileswnd.h"// NEO: NSC - [NeoSharedCategories] <-- Xanatos --
#include "Neo/Functions.h" // NEO: MOD <-- Xanatos --
#include "Neo/NeoPreferences.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --
// NEO: MOD - [NeoDownloadCommands] -- Xanatos -->
#include "Neo/Sources/SourceList.h"
#include "Neo/CP/FilePreferencesDialog.h"
#include "FileDetailDialog.h"
// NEO: MOD END <-- Xanatos --
#include "Neo/GUI/SelCategoryDlg.h" // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
#include "Neo/BC/BandwidthControl.h"
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
#include "Neo/BC/DownloadBandwidthThrottler.h"
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
#include "Neo/VooDoo/voodoo.h"
#include "Neo/VooDoo/VoodooListDlg.h"
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
// NEO: FCFG - [FileConfiguration] -- Xanatos -->
#include "Neo/FilePreferences.h"
#include "KnownFileList.h"
// NEO: FCFG END <-- Xanatos --
// NEO: PP - [PasswordProtection] -- Xanatos -->
#include "InputBox.h"
#include "MD5Sum.h" 
// NEO: PP END <-- Xanatos --

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CDownloadQueue::CDownloadQueue()
{
	filesrdy = 0;
	datarate = 0;
	cur_udpserver = 0;
	lastfile = 0;
	lastcheckdiskspacetime = 0;
	lastudpsearchtime = 0;
	lastudpstattime = 0;
	SetLastKademliaFileRequest();
	udcounter = 0;
	m_iSearchedServers = 0;
	m_datarateMS=0;
	m_nUDPFileReasks = 0;
	m_nFailedUDPFileReasks = 0;
	m_dwNextTCPSrcReq = 0;
	m_cRequestsSentToServer = 0;
    m_dwLastA4AFtime = 0; // ZZ:DownloadManager

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
	numberOfDownloading = 0;
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
	// NEO: QS - [QuickStart] -- Xanatos -->
	m_bQuickStartDone = false; 
	m_dwQuickStartEndTime = 0;
	// NEO: QS END <-- Xanatos --

	// NEO: GSL - [GlobalSourceLimit] -- Xanatos -->
	m_uLastGlobalSourceLimit = ::GetTickCount();
	m_bPassiveMode = false;
	m_bGlobalHLSrcReqAllowed = true;
	// NEO: GSL END <-- Xanatos --

	// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
	m_iNextLinkQueuedTick = 0;
	forcea4af_file = NULL;
	// NEO: NXC END <-- Xanatos --
}

void CDownloadQueue::AddPartFilesToShare()
{
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
	{
		CPartFile* cur_file = filelist.GetNext(pos);
		//if (cur_file->GetStatus(true) == PS_READY) // NEO: SAFS - [ShowAllFilesInShare] <-- Xanatos --
			theApp.sharedfiles->SafeAddKFile(cur_file, true);
	}
}

void CDownloadQueue::Init(){
	// find all part files, read & hash them if needed and store into a list
	//CFileFind ff;
	//int count = 0;

	for (int i=0;i<thePrefs.tempdir.GetCount();i++) {
		Init(thePrefs.GetTempDir(i)); // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
		//CString searchPath=thePrefs.GetTempDir(i);
	}
	// NEO: MTD - [MultiTempDirectories] -- Xanatos --
	//if(count == 0) {
	//	AddLogLine(false,GetResString(IDS_NOPARTSFOUND));
	//} else {
	//	AddLogLine(false,GetResString(IDS_FOUNDPARTS),count);
	//	SortByPriority();
	//	CheckDiskspace();
	//}
	VERIFY( m_srcwnd.CreateEx(0, AfxRegisterWndClass(0), _T("eMule Async DNS Resolve Socket Wnd #2"), WS_OVERLAPPED, 0, 0, 0, 0, NULL, NULL));

	ExportPartMetFilesOverview();
}

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
void CDownloadQueue::LoadSources(){
	// Load sources
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		if(cur_file->PartPrefs->AutoLoadSources() && !cur_file->IsStopped()){
			cur_file->LoadSources();
		}
	}
}
#endif // NEO_SS // NEO: NSS END <-- Xanatos --

// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
void CDownloadQueue::Init(CString tempPath){

	CFileFind ff;
	int count = 0;

	CString searchPath = tempPath;
// NEO: MTD END <-- Xanatos --

	searchPath += _T("\\*.part.met");

	//check all part.met files
	bool end = !ff.FindFile(searchPath, 0);
	while (!end){
		end = !ff.FindNextFile();
		if (ff.IsDirectory())
			continue;
		CPartFile* toadd = new CPartFile();
		if (toadd->LoadPartFile(tempPath, ff.GetFileName())){ // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
			count++;
			filelist.AddTail(toadd);			// to downloadqueue
			// NEO: SSH - [SlugFillerSafeHash] -- Xanatos -->
			// SLUGFILLER: SafeHash remove - part files are shared later
			//if (toadd->GetStatus(true) == PS_READY) // NEO: SAFS - [ShowAllFilesInShare] <-- Xanatos --
			//	theApp.sharedfiles->SafeAddKFile(toadd); // part files are always shared files
			// NEO: SSH END <- Xanatos --
			theApp.emuledlg->transferwnd->downloadlistctrl.AddFile(toadd);// show in downloadwindow
			// NEO: PP - [PasswordProtection] -- Xanatos -->
			if (toadd->IsPWProtHidden()) 
				theApp.emuledlg->transferwnd->downloadlistctrl.HideFile(toadd);
			// NEO: PP END <- Xanatos --
		}
		else
			delete toadd;
	}
	ff.Close();

	//try recovering any part.met files
	searchPath += _T(".backup");
	end = !ff.FindFile(searchPath, 0);
	while (!end){
		end = !ff.FindNextFile();
		if (ff.IsDirectory())
			continue;
		CPartFile* toadd = new CPartFile();
		if (toadd->LoadPartFile(tempPath, ff.GetFileName())){ // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
			toadd->SavePartFile(); // resave backup
			count++;
			filelist.AddTail(toadd);			// to downloadqueue
			// NEO: SSH - [SlugFillerSafeHash] -- Xanatos -->
			// SLUGFILLER: SafeHash remove - part files are shared later
			//if (toadd->GetStatus(true) == PS_READY) // NEO: SAFS - [ShowAllFilesInShare] <-- Xanatos --
			//	theApp.sharedfiles->SafeAddKFile(toadd); // part files are always shared files
			// NEO: SSH END <- Xanatos --
			theApp.emuledlg->transferwnd->downloadlistctrl.AddFile(toadd);// show in downloadwindow
			// NEO: PP - [PasswordProtection] -- Xanatos -->
			if (toadd->IsPWProtHidden()) 
				theApp.emuledlg->transferwnd->downloadlistctrl.HideFile(toadd);
			// NEO: PP END <- Xanatos --
			AddLogLine(false, GetResString(IDS_RECOVERED_PARTMET), toadd->GetFileName());
		}
		else {
			delete toadd;
		}
	}
	ff.Close();

// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
	if(count == 0) {
		AddLogLine(false,GetResString(IDS_NOPARTSFOUND) + _T(" (%s)"),searchPath); 
	} else {
		AddLogLine(false,GetResString(IDS_FOUNDPARTS) + _T(" (%s)"),count,searchPath); 
		SortByPriority();
		CheckDiskspace();
	}
}
// NEO: MTD END <-- Xanatos --

CDownloadQueue::~CDownloadQueue(){
	// NEO: XUC - [ExtendedUdpCache] -- Xanatos -->
	CSKey Key;
	CSearchFile::SServer* Server;
	POSITION Pos = m_CachedServers.GetStartPosition();
	while( Pos != NULL ){
		m_CachedServers.GetNextAssoc( Pos, Key, (void*&)Server );
	    delete Server;
	}
	m_CachedServers.RemoveAll();
	// NEO: XUC END <-- Xanatos --

	for (POSITION pos = filelist.GetHeadPosition();pos != 0;)
		delete filelist.GetNext(pos);
	m_srcwnd.DestroyWindow(); // just to avoid a MFC warning
}

void CDownloadQueue::AddSearchToDownload(CSearchFile* toadd, uint8 paused, int cat)
{
	if (toadd->GetFileSize()== (uint64)0 || IsFileExisting(toadd->GetFileHash()))
		return;

	if (toadd->GetFileSize() > OLD_MAX_EMULE_FILE_SIZE && !thePrefs.CanFSHandleLargeFiles()){
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_FSCANTHANDLEFILE));
		return;
	}

	// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
	if(NeoPrefs.CheckAlreadyDownloaded()){
		if(theApp.knownfiles->CheckAlreadyDownloadedFileQuestion(toadd->GetFileHash(),toadd->GetFileName()) == false){
			return;
		}
	}
	// NEO: NXC END <-- Xanatos --

	CPartFile* newfile = new CPartFile(toadd,cat);
	if (newfile->GetStatus() == PS_ERROR){
		delete newfile;
		return;
	}

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
	const CSimpleArray<CSearchFile::SClient>& aClients = toadd->GetClients();
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

	CahceUDPServer(toadd); // NEO: XUC - [ExtendedUdpCache] <-- Xanatos --
}

void CDownloadQueue::AddSearchToDownload(CString link, uint8 paused, int cat)
{
	CPartFile* newfile = new CPartFile(link, cat);
	if (newfile->GetStatus() == PS_ERROR){
		delete newfile;
		return;
	}

	// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
	if(NeoPrefs.CheckAlreadyDownloaded()){
		if(theApp.knownfiles->CheckAlreadyDownloadedFileQuestion(newfile->GetFileHash(),newfile->GetFileName()) == false){
			delete newfile;
			return;
		}
	}
	// NEO: NXC END <-- Xanatos --

	if (paused == 2)
		paused = (uint8)thePrefs.AddNewFilesPaused();
	AddDownload(newfile, (paused==1));
}

void CDownloadQueue::StartNextFileIfPrefs(int cat) {
    if (thePrefs.StartNextFile())
		//StartNextFile((thePrefs.StartNextFile() > 1?cat:-1), (thePrefs.StartNextFile()!=3));
	// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
	{
		int catTemp = thePrefs.StartNextFile() > 1 ? cat: -1;
		Category_Struct* cur_cat = thePrefs.GetCategory(cat);
		if (cur_cat && cur_cat->bResumeFileOnlyInSameCat)
			catTemp = cat;
		StartNextFile(catTemp, (thePrefs.StartNextFile()!=3));
	}
	// NEO: NXC END <-- Xanatos --
}

//void CDownloadQueue::StartNextFile(int cat, bool force){ 
bool CDownloadQueue::StartNextFile(int cat, bool force){ // NEO: NXC - [NewExtendedCategories] <-- Xanatos --

	CPartFile*  pfile = NULL;
	CPartFile* cur_file ;
	POSITION pos;
	
	if (cat != -1) {
		// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
		if(thePrefs.GetResumeOnlyInCat(cat))
			force = false;
		// NEO: NXC END <-- Xanatos --
        // try to find in specified category
		for (pos = filelist.GetHeadPosition();pos != 0;){
			cur_file = filelist.GetNext(pos);
			if (cur_file->GetStatus()==PS_PAUSED &&
                (
				 cur_file->GetCategory()==(UINT)cat //|| 
				 //cat==0 && thePrefs.GetCategory(0)->filter==0 && cur_file->GetCategory()>0 // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
                ) &&
                //CPartFile::RightFileHasHigherPrio(pfile, cur_file)
				CPartFile::NextRightFileHasHigherPrio(pfile, cur_file) // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
			   ) {
    			pfile = cur_file;
			}
		}
		if (pfile == NULL && !force)
			//return;
			return false; // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
	}

    if(cat == -1 || pfile == NULL && force) {
	    for (pos = filelist.GetHeadPosition();pos != 0;){
		    cur_file = filelist.GetNext(pos);
		    if (cur_file->GetStatus() == PS_PAUSED &&
				CPartFile::NextRightFileHasHigherPrio(pfile, cur_file) && !thePrefs.GetResumeOnlyInCat(cur_file->GetCategory())) // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
                //CPartFile::RightFileHasHigherPrio(pfile, cur_file))
		    {
                // pick first found matching file, since they are sorted in prio order with most important file first.
			    pfile = cur_file;
		    }
	    }
    }
	if (pfile) pfile->ResumeFile();

	return (pfile != NULL); // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
}

void CDownloadQueue::AddFileLinkToDownload(CED2KFileLink* pLink, int cat)
{
	// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
	if(NeoPrefs.CheckAlreadyDownloaded()){
		if(theApp.knownfiles->CheckAlreadyDownloadedFileQuestion(pLink->GetHashKey(),pLink->GetName()) == false){
			delete pLink;
			return;
		}
	}
	
	if (NeoPrefs.SelectCatForNewDL() && cat == -1){
		m_ED2KLinkQueue.AddTail(pLink);
		if(m_iNextLinkQueuedTick != 0xFFFFFFFF)
			m_iNextLinkQueuedTick = ::GetTickCount() + 500;
		return;
	}
	// NEO: NXC END <-- Xanatos --

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
		if (pLink->HasValidSources())
			partfile->AddClientSources(pLink->SourcesList, 1, false);
		if (pLink->HasValidAICHHash() ){
			if ( !(partfile->GetAICHHashset()->HasValidMasterHash() && partfile->GetAICHHashset()->GetMasterHash() == pLink->GetAICHHash())){
				partfile->GetAICHHashset()->SetMasterHash(pLink->GetAICHHash(), AICH_VERIFIED);
				partfile->GetAICHHashset()->FreeHashSet();
			}
		}
	}

	if (pLink->HasHostnameSources())
	{
		POSITION pos = pLink->m_HostnameSourcesList.GetHeadPosition();
		while (pos != NULL)
		{
			const SUnresolvedHostname* pUnresHost = pLink->m_HostnameSourcesList.GetNext(pos);
			m_srcwnd.AddToResolve(pLink->GetHashKey(), pUnresHost->strHostname, pUnresHost->nPort, pUnresHost->strURL);
		}
	}

	delete pLink; // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
}

void CDownloadQueue::AddToResolved( CPartFile* pFile, SUnresolvedHostname* pUH )
{
	if( pFile && pUH )
		m_srcwnd.AddToResolve( pFile->GetFileHash(), pUH->strHostname, pUH->nPort, pUH->strURL);
}

void CDownloadQueue::AddDownload(CPartFile* newfile,bool paused) {
	// Barry - Add in paused mode if required
	if (paused)
		newfile->PauseFile();
	
	//SetAutoCat(newfile);// HoaX_69 / Slugfiller: AutoCat // NEO: NXC - [NewExtendedCategories] <-- Xanatos --

	filelist.AddTail(newfile);
	SortByPriority();
	CheckDiskspace();
	theApp.emuledlg->transferwnd->downloadlistctrl.AddFile(newfile);
	theApp.sharedfiles->SafeAddKFile(newfile); // NEO: SAFS - [ShowAllFilesInShare] <-- Xanatos --
	AddLogLine(true, GetResString(IDS_NEWDOWNLOAD), newfile->GetFileName());
	CString msgTemp;
	msgTemp.Format(GetResString(IDS_NEWDOWNLOAD) + _T("\n"), newfile->GetFileName());
	theApp.emuledlg->ShowNotifier(msgTemp, TBN_DOWNLOADADDED);
	ExportPartMetFilesOverview();

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	if(!newfile->IsVoodooFile() && newfile->KnownPrefs->IsEnableVoodoo())
		theApp.voodoo->ManifestDownloadOrder(newfile);
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
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

void CDownloadQueue::Process(){
	// NEO: GSL - [GlobalSourceLimit] -- Xanatos -->
	if( NeoPrefs.PartPrefs.UseGlobalSourceLimit() && (::GetTickCount() - m_uLastGlobalSourceLimit) >= (NeoPrefs.PartPrefs.GetGlobalSourceLimitTimeMs() * (m_bPassiveMode ? 1 : 5)) )
		SetGlobalSourceLimits();
	// NEO: GSL END <-- Xanatos --

	ProcessLocalRequests(); // send src requests to local server

	uint32 downspeed = 0;
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
	uint64 maxDownload = (uint64)(theApp.bandwidthControl->GetMaxDownload() * 1024);
#else
    uint64 maxDownload = thePrefs.GetMaxDownloadInBytesPerSec(true);
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
	if(!NeoPrefs.UseDownloadBandwidthThrottler())
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --
	if (maxDownload != UNLIMITED*1024 && datarate > 1500){
		downspeed = (UINT)((maxDownload*100)/(datarate+1));
		if (downspeed < 50)
			downspeed = 50;
		else if (downspeed > 200)
			downspeed = 200;
	}

	// NEO: ASM - [AccurateSpeedMeasure] -- Xanatos --
	//while(avarage_dr_list.GetCount()>0 && (GetTickCount() - avarage_dr_list.GetHead().timestamp > 10*1000) )
	//	m_datarateMS-=avarage_dr_list.RemoveHead().datalen;
	
	//if (avarage_dr_list.GetCount()>1){
	//	datarate = (UINT)(m_datarateMS / avarage_dr_list.GetCount());
	//} else {
	//	datarate = 0;
	//}

	//uint32 datarateX=0; // NEO: ASM - [AccurateSpeedMeasure] <-- Xanatos --
	udcounter++;

	//filelist is already sorted by prio, therefore I removed all the extra loops..
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		if (cur_file->GetStatus() == PS_READY || cur_file->GetStatus() == PS_EMPTY){
			cur_file->Process(downspeed, udcounter); // NEO: ASM - [AccurateSpeedMeasure] <-- Xanatos --
			//datarateX += cur_file->Process(downspeed, udcounter); 
		}
		else{
			//This will make sure we don't keep old sources to paused and stoped files..
			cur_file->StopPausedFile();
		}
	}

	// NEO: ASM - [AccurateSpeedMeasure] -- Xanatos --
	//TransferredData newitem = {datarateX, ::GetTickCount()};
	//avarage_dr_list.AddTail(newitem);
	//m_datarateMS+=datarateX;

	if (udcounter == 5){
		if (theApp.serverconnect->IsUDPSocketAvailable()){
		    if((!lastudpstattime) || (::GetTickCount() - lastudpstattime) > UDPSERVERSTATTIME){
			    lastudpstattime = ::GetTickCount();
			    theApp.serverlist->ServerStats();
		    }
	    }
	}

	//if (udcounter == 10){
	ASSERT(udcounter <= 10); // NEO: ND <-- Xanatos -- 
	if (udcounter >= 10){ // NEO: FIX <-- Xanatos -- 
		udcounter = 0;
		if (theApp.serverconnect->IsUDPSocketAvailable()){
			//if ((!lastudpsearchtime) || (::GetTickCount() - lastudpsearchtime) > UDPSERVERREASKTIME)
			if ((!lastudpsearchtime) || (::GetTickCount() - lastudpsearchtime) > NeoPrefs.PartPrefs.GetUdpGlobalIntervalsMs()) // NEO: SRT - [SourceRequestTweaks] <-- Xanatos --
				SendNextUDPPacket();
		}
	}

	// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
	if (m_iNextLinkQueuedTick && m_iNextLinkQueuedTick < ::GetTickCount())
		PurgeED2KLinkQueue();
	// NEO: NXC END <-- Xanatos --

	CheckDiskspaceTimed();

// ZZ:DownloadManager -->
    if((!m_dwLastA4AFtime) || (::GetTickCount() - m_dwLastA4AFtime) > MIN2MS(8)) {
        theApp.clientlist->ProcessA4AFClients();
        m_dwLastA4AFtime = ::GetTickCount();
    }
// <-- ZZ:DownloadManager
}

// NEO: ASM - [AccurateSpeedMeasure] -- Xanatos -->
void CDownloadQueue::CalculateDownloadRate()
{
	while(avarage_dr_list.GetCount()>0 && (GetTickCount() - avarage_dr_list.GetHead().timestamp > 10*1000) )
		m_datarateMS-=avarage_dr_list.RemoveHead().datalen;
	
	if (avarage_dr_list.GetCount()>1){
		datarate = (UINT)(m_datarateMS / avarage_dr_list.GetCount());
	} else {
		datarate = 0;
	}

	uint32 datarateX=0;

	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; ){
		datarateX += filelist.GetNext(pos)->CalculateDownloadRate();
	}

	TransferredData newitem = {datarateX, ::GetTickCount()};
	avarage_dr_list.AddTail(newitem);
	m_datarateMS+=datarateX;
}
// NEO: ASM END <-- Xanatos --

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

#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	const bool bMultiTransfer = source->IsLanClient() && NeoPrefs.UseLanMultiTransfer();
#endif //LANCAST // NEO: NLC END <-- Xanatos --

	// uses this only for temp. clients
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
		if(bMultiTransfer && cur_file != sender)
			continue;
#endif //LANCAST // NEO: NLC END <-- Xanatos --
		for (POSITION pos2 = cur_file->srclist.GetHeadPosition();pos2 != 0; ){
			CUpDownClient* cur_client = cur_file->srclist.GetNext(pos2);
			if (cur_client->Compare(source, true) || cur_client->Compare(source, false)){
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
				// If we find a matching clinet, that is in loaded stare,
				// it means he should be available again, 
				// so we set him to DS_NONE to force an TCP reask.
				// And if needed update the ip of our source.
				if(cur_client->GetDownloadState() == DS_LOADED){
					if(cur_client->GetIP() != source->GetConnectIP() || cur_client->GetUserPort() != source->GetUserPort()){ // UserHash match, but IP is new
						cur_client->SetConnectIP(source->GetConnectIP());
						cur_client->SetUserPort(source->GetUserPort());
						cur_client->SetSourceFrom(source->GetSourceFrom());
					}
					cur_client->SetDownloadState(DS_NONE);
				}
#endif // NEO_SS // NEO: NSS END <-- Xanatos --

				if (cur_file == sender){ // this file has already this source
					delete source;
					return false;
				}
				// set request for this source
				if (cur_client->AddRequestForAnotherFile(sender)){
					theApp.emuledlg->transferwnd->downloadlistctrl.AddSource(sender,cur_client,true);
#ifdef VOODOO // NEO: VOODOOx - [VoodooSourceExchange] -- Xanatos -->
					if(NeoPrefs.IsVoodooEnabled() && sender->IsVoodooXS())
						theApp.voodoo->ManifestSingleSource(sender,cur_client);
#endif // VOODOO // NEO: VOODOOx END <-- Xanatos --
					delete source;
                    if(cur_client->GetDownloadState() != DS_CONNECTED) {
                        cur_client->SwapToAnotherFile(_T("New A4AF source found. CDownloadQueue::CheckAndAddSource()"), false, false, false, NULL, true, false); // ZZ:DownloadManager
                    }
					return false;
				}
				else{
					delete source;
					return false;
				}
			}
		}
	}
	//our new source is real new but maybe it is already uploading to us?
	//if yes the known client will be attached to the var "source"
	//and the old sourceclient will be deleted
	//if (theApp.clientlist->AttachToAlreadyKnown(&source,0)){
	if (
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	 !bMultiTransfer &&
#endif //LANCAST // NEO: NLC END <-- Xanatos --
	 theApp.clientlist->AttachToAlreadyKnown(&source,0)){
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
		source->SetRequestFile(sender);
	}
	else{
		// here we know that the client instance 'source' is a new created client instance (see callers) 
		// which is therefor not already in the clientlist, we can avoid the check for duplicate client list entries 
		// when adding this client
		theApp.clientlist->AddClient(source,true);
	}

	// NEO: SCFS - [SmartClientFileStatus] -- Xanatos --
//#ifdef _DEBUG
//	if (thePrefs.GetVerbose() && source->GetPartCount()!=0 && source->GetPartCount()!=sender->GetPartCount()){
//		DEBUG_ONLY(AddDebugLogLine(false, _T("*** CDownloadQueue::CheckAndAddSource -- New added source (%u, %s) had still value in partcount"), source->GetUserIDHybrid(), sender->GetFileName()));
//	}
//#endif

	sender->srclist.AddTail(source);
	theApp.emuledlg->transferwnd->downloadlistctrl.AddSource(sender,source,false);
#ifdef VOODOO // NEO: VOODOOx - [VoodooSourceExchange] -- Xanatos -->
	if(NeoPrefs.IsVoodooEnabled() && sender->IsVoodooXS())
		theApp.voodoo->ManifestSingleSource(sender,source);
#endif // VOODOO // NEO: VOODOOx END <-- Xanatos --
	return true;
}

bool CDownloadQueue::CheckAndAddKnownSource(CPartFile* sender,CUpDownClient* source, bool bIgnoreGlobDeadList){
	if (sender->IsStopped())
		return false;
	
	// filter sources which are known to be temporarily dead/useless
	if ( (theApp.clientlist->m_globDeadSourceList.IsDeadSource(source) && !bIgnoreGlobDeadList) || sender->m_DeadSourceList.IsDeadSource(source)){
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
	//if (!source->HasLowID()){
	if (!source->HasLowID()
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	 && !source->IsLanClient()	// LAN clients would always be filtered, so don't filter identified LanCast users
#endif //LANCAST // NEO: NLC END <-- Xanatos --
	 ){
		uint32 nClientIP = ntohl(source->GetUserIDHybrid());
		if (!IsGoodIP(nClientIP)){ // check for 0-IP, localhost and LAN addresses
			//if (thePrefs.GetLogFilteredIPs())
			//	AddDebugLogLine(false, _T("Ignored already known source with IP=%s"), ipstr(nClientIP));
			return false;
		}
	}

	// use this for client which are already know (downloading for example)
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		if (cur_file->srclist.Find(source)){
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
				// If we find a matching clinet, that is in loaded stare,
				// it means he should be available again, 
				// so we set him to DS_NONE to force an TCP reask.
				if(source->GetDownloadState() == DS_LOADED)
					source->SetDownloadState(DS_NONE);
#endif // NEO_SS // NEO: NSS END <-- Xanatos --
			if (cur_file == sender)
				return false;
			if (source->AddRequestForAnotherFile(sender))
				theApp.emuledlg->transferwnd->downloadlistctrl.AddSource(sender,source,true);
#ifdef VOODOO // NEO: VOODOOx - [VoodooSourceExchange] -- Xanatos -->
					if(NeoPrefs.IsVoodooEnabled() && sender->IsVoodooXS())
						theApp.voodoo->ManifestSingleSource(sender,source);
#endif // VOODOO // NEO: VOODOOx END <-- Xanatos --
                if(source->GetDownloadState() != DS_CONNECTED) {
                    source->SwapToAnotherFile(_T("New A4AF source found. CDownloadQueue::CheckAndAddKnownSource()"), false, false, false, NULL, true, false); // ZZ:DownloadManager
                }
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
	if (thePrefs.GetDebugSourceExchange())
		AddDebugLogLine(false, _T("SXRecv: Passively added source; %s, File=\"%s\""), source->DbgGetClientInfo(), sender->GetFileName());

	// NEO: SCFS - [SmartClientFileStatus] -- Xanatos --
//#ifdef _DEBUG
//	if (thePrefs.GetVerbose() && source->GetPartCount()!=0 && source->GetPartCount()!=sender->GetPartCount()){
//		DEBUG_ONLY(AddDebugLogLine(false, _T("*** CDownloadQueue::CheckAndAddKnownSource -- New added source (%u, %s) had still value in partcount"), source->GetUserIDHybrid(), sender->GetFileName()));
//	}
//#endif

	theApp.emuledlg->transferwnd->downloadlistctrl.AddSource(sender,source,false);
#ifdef VOODOO // NEO: VOODOOx - [VoodooSourceExchange] -- Xanatos -->
	if(NeoPrefs.IsVoodooEnabled() && sender->IsVoodooXS())
		theApp.voodoo->ManifestSingleSource(sender,source);
#endif // VOODOO // NEO: VOODOOx END <-- Xanatos --
	//UpdateDisplayedInfo();
	return true;
}

bool CDownloadQueue::RemoveSource(CUpDownClient* toremove, bool bDoStatsUpdate)
{
	bool bRemovedSrcFromPartFile = false;
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
		if ( bDoStatsUpdate )
			cur_file->UpdateAvailablePartsCount();
	}
	
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
			theApp.emuledlg->transferwnd->downloadlistctrl.RemoveSource(toremove,toremove->m_OtherRequests_list.GetAt(pos4));
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
			theApp.emuledlg->transferwnd->downloadlistctrl.RemoveSource(toremove,toremove->m_OtherNoNeeded_list.GetAt(pos4));
			toremove->m_OtherNoNeeded_list.RemoveAt(pos4);
		}
	}

	// NEO: XC - [ExtendedComments] -- Xanatos --
	// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
	//CClientFileStatus* status = toremove->GetFileStatus(toremove->GetRequestFile());
	//if (bRemovedSrcFromPartFile && status && (status->HasFileRating() || !status->GetFileComment().IsEmpty()))
	// NEO: SCFS END <-- Xanatos --
	////if (bRemovedSrcFromPartFile && (toremove->HasFileRating() || !toremove->GetFileComment().IsEmpty()))
	//{
	//	CPartFile* pFile = toremove->GetRequestFile();
	//	if(pFile)
	//		pFile->UpdateFileRatingCommentAvail();
	//}

	toremove->SetDownloadState(DS_NONE);
	theApp.emuledlg->transferwnd->downloadlistctrl.RemoveSource(toremove,0);
	toremove->SetRequestFile(NULL);
	return bRemovedSrcFromPartFile;
}

void CDownloadQueue::RemoveFile(CPartFile* toremove)
{
	RemoveLocalServerRequest(toremove);

	for (POSITION pos = filelist.GetHeadPosition();pos != 0;filelist.GetNext(pos)){
		if (toremove == filelist.GetAt(pos)){
			filelist.RemoveAt(pos);
			break;
		}
	}
	SortByPriority();
	CheckDiskspace();
	ExportPartMetFilesOverview();
}

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
void CDownloadQueue::SaveSources(){
	POSITION pos;
	// Save source in a separat loop to get all A4AF sources
	for (pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		if(cur_file->PartPrefs->AutoSaveSources() && !cur_file->IsStopped())
			cur_file->SaveSources();
	}
}
#endif // NEO_SS // NEO: NSS END <-- Xanatos --

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

#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
	ProcessQueue.RemoveAll();
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --
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
		//return (m_cRequestsSentToServer >= MAX_REQUESTS_PER_SERVER) || (nUsedBytes >= MAX_UDP_PACKET_DATA);
		return (m_cRequestsSentToServer >= (UINT)NeoPrefs.PartPrefs.GetUdpFilesPerServer()) || (nUsedBytes >= MAX_UDP_PACKET_DATA); // NEO: SRT - [SourceRequestTweaks] <-- Xanatos --
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

		//if (!bSentPacket && nextfile && nextfile->GetSourceCount() < nextfile->GetMaxSourcePerFileUDP() && (bServerSupportsLargeFiles || !nextfile->IsLargeFile()) )
		if (!bSentPacket && nextfile 
			// NEO: SRT - [SourceRequestTweaks] -- Xanatos -->
			&& nextfile->PartPrefs->IsUdpEnable() && !nextfile->IsCollectingHalted() // NEO: XSC - [ExtremeSourceCache]
			&& nextfile->GetSourceCount() < nextfile->GetUdpSourceLimit()
			&& (!nextfile->m_LastSearchTimeUdp || (::GetTickCount() - nextfile->m_LastSearchTimeUdp) > nextfile->PartPrefs->GetUdpIntervalsMs())
			// NEO: SRT END <-- Xanatos --
			&& (bServerSupportsLargeFiles || !nextfile->IsLargeFile()) )
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
	//if (m_cRequestsSentToServer >= MAX_REQUESTS_PER_SERVER)
	if (m_cRequestsSentToServer >= (UINT)NeoPrefs.PartPrefs.GetUdpFilesPerServer()) // NEO: SRT - [SourceRequestTweaks] <-- Xanatos --
	{
		if (thePrefs.GetDebugServerUDPLevel() > 0 && thePrefs.GetDebugServerSourcesLevel() > 0)
			Debug(_T("Rotating file list\n"));

		// move the last 35 files to the head
		//if (filelist.GetCount() >= MAX_REQUESTS_PER_SERVER) {
		if (filelist.GetCount() >= NeoPrefs.PartPrefs.GetUdpFilesPerServer()) { // NEO: SRT - [SourceRequestTweaks] <-- Xanatos --
			//for (int i = 0; i != MAX_REQUESTS_PER_SERVER; i++)
			for (int i = 0; i != NeoPrefs.PartPrefs.GetUdpFilesPerServer(); i++) // NEO: SRT - [SourceRequestTweaks] <-- Xanatos --
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
	// NEO: SRT - [SourceRequestTweaks] -- Xanatos -->
	for( POSITION pos1 = filelist.GetHeadPosition(); pos1 != NULL; )
		filelist.GetNext(pos1)->m_LastSearchTimeUdp = ::GetTickCount();
	// NEO: SRT END <-- Xanatos --
	lastfile = NULL;
	m_iSearchedServers = 0;
}

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
				case PS_MOVING: // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
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
			case PS_MOVING: // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
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
			case PS_MOVING: // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
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
#ifdef NATTUNNELING // NEO: XSB - [XSBuddy] -- Xanatos -->
		results.a[30]  += cur_file->GetSrcStatisticsValue(DS_WAITCALLBACKXS);
#endif //NATTUNNELING // NEO: XSB END <-- Xanatos --
		results.a[9]  += cur_file->GetSrcStatisticsValue(DS_TOOMANYCONNS);
		results.a[9]  += cur_file->GetSrcStatisticsValue(DS_TOOMANYCONNSKAD);
		results.a[10] += cur_file->GetSrcStatisticsValue(DS_LOWTOLOWIP);
		results.a[11] += cur_file->GetSrcStatisticsValue(DS_NONE);
		results.a[12] += cur_file->GetSrcStatisticsValue(DS_ERROR);
		results.a[13] += cur_file->GetSrcStatisticsValue(DS_BANNED);
		results.a[14] += cur_file->src_stats[SF_PASSIVE]; // NEO: MOD - [srcStats] <-- Xanatos --
		results.a[15] += cur_file->GetSrcA4AFCount();
		results.a[16] += cur_file->src_stats[SF_SERVER]; // NEO: MOD - [srcStats] <-- Xanatos --
		results.a[17] += cur_file->src_stats[SF_KADEMLIA]; // NEO: MOD - [srcStats] <-- Xanatos --
		results.a[18] += cur_file->src_stats[SF_SOURCE_EXCHANGE]; // NEO: MOD - [srcStats] <-- Xanatos --
		results.a[19] += cur_file->net_stats[0];
		results.a[20] += cur_file->net_stats[1];
		results.a[21] += cur_file->net_stats[2];
		results.a[22] += cur_file->m_DeadSourceList.GetDeadSourcesCount();
		results.a[23] += cur_file->GetSrcStatisticsValue(DS_CONNECTIONRETRY); // NEO: TCR - [TCPConnectionRetry] <-- Xanatos --
		results.a[24] += cur_file->GetSrcStatisticsValue(DS_CACHED); // NEO: XSC - [ExtremeSourceCache] <-- Xanatos --
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
		results.a[25] += cur_file->src_stats[SF_STORAGE]; // src from storage
		results.a[26] += cur_file->GetSrcStatisticsValue(DS_LOADED);
#endif // NEO_SS // NEO: NSS END <-- Xanatos --
		results.a[27]  += cur_file->GetSrcStatisticsValue(DS_HALTED); // NEO: SD - [StandByDL] <-- Xanatos --
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
		results.a[28] += cur_file->src_stats[SF_LANCAST];
#endif //LANCAST // NEO: NLC END <-- Xanatos --
#ifdef VOODOO // NEO: VOODOOx - [VoodooSourceExchange] -- Xanatos -->
		results.a[29] += cur_file->src_stats[SF_VOODOO];
#endif // VOODOO // NEO: VOODOOx END <-- Xanatos --
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

    theApp.downloadqueue->SortByPriority();
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

    theApp.downloadqueue->SortByPriority();
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
	    theApp.downloadqueue->SortByPriority();
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

	theApp.emuledlg->sharedfileswnd->Reload(true); // NEO: NSC - [NeoSharedCategories] <-- Xanatos --
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

// NEO: NXC - [NewExtendedCategories] -- Xanatos --
/*void CDownloadQueue::SetAutoCat(CPartFile* newfile){
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
				newfile->SetCategory(ix);
		}
	}
}*/

void CDownloadQueue::ResetLocalServerRequests()
{
	m_dwNextTCPSrcReq = 0;
	m_localServerReqQueue.RemoveAll();

	POSITION pos = filelist.GetHeadPosition();
	while (pos != NULL)
	{ 
		CPartFile* pFile = filelist.GetNext(pos);
		// NEO: POFC - [PauseOnFileComplete] -- Xanatos -->
		if(pFile->GetCompletionBreak()) 
			continue; 
		// NEO: POFC END <-- Xanatos --
		UINT uState = pFile->GetStatus();
		if (uState == PS_READY || uState == PS_EMPTY)
			pFile->ResumeFile(false); // NEO: CI#12 - [CodeImprovement] <-- Xanatos --
			//pFile->ResumeFile();
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
		//const int iMaxFilesPerTcpFrame = 15;
		const int iMaxFilesPerTcpFrame = theApp.serverconnect->GetCurrentServer() ? theApp.serverconnect->GetCurrentServer()->GetMaxAsksFromCredits(15) : 15; // NEO: KLC - [KhaosLugdunumCredits] <-- Xanatos --
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
			// NEO: KLC - [KhaosLugdunumCredits] -- Xanatos -->
			if(theApp.serverconnect->GetCurrentServer() != NULL)
				theApp.serverconnect->GetCurrentServer()->FileAsksSent(iFiles); 
			// NEO: KLC END <-- Xanatos --
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
	//if(!(!temp->IsStopped() && temp->GetMaxSources() > temp->GetSourceCount()))
	// NEO: XSC - [ExtremeSourceCache] -- Xanatos -->
	bool bCache = false;
	if(!(!temp->IsStopped() && 
		(bCache = !(temp->GetMaxSources() > temp->GetSourceCount())) == false)
	 && !(temp->PartPrefs->UseSourceCache() && temp->GetSourceCacheSourceLimit() > temp->GetSrcStatisticsValue(DS_CACHED)))
	 // NEO: XSC END <-- Xanatos --
		return;

	uint32 ED2Kip = ntohl(ip);
	if (theApp.ipfilter->IsFiltered(ED2Kip))
	{
		if (thePrefs.GetLogFilteredIPs())
			AddDebugLogLine(false, _T("IPfiltered source IP=%s (%s) received from Kademlia"), ipstr(ED2Kip), theApp.ipfilter->GetLastHit());
		return;
	}
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
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
			if(tcp == (uint16)-1)
			{
				ctemp->SetConnectIP(ip);
				ctemp->SetNatTraversalSupport(true, true);
			}
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
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
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
			if(tcp == (uint16)-1)
			{
				ctemp->SetConnectIP(ip);
				ctemp->SetNatTraversalSupport(true, true);
			}
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
		}
	}

	if (ctemp != NULL){
		// add encryption settings
		ctemp->SetConnectOptions(byCryptOptions);
		// NEO: XSC - [ExtremeSourceCache] -- Xanatos -->
		if(bCache)
			ctemp->SetDownloadState(DS_CACHED);
		// NEO: XSC END <-- Xanatos --
		CheckAndAddSource(temp, ctemp);
	}
}

void CDownloadQueue::ExportPartMetFilesOverview() const
{
	CString strFileListPath = thePrefs.GetMuleDirectory(EMULE_DATABASEDIR) + _T("downloads.txt");
	
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
					file.printf(_T("%s%s\t%s\r\n"), szNam, szExt, CreateED2kLink(pPartFile));
				else
					file.printf(_T("%s\t%s\r\n"), pPartFile->GetFullName(), CreateED2kLink(pPartFile));
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
			//case PS_WAITINGFORHASH: // NEO: SSH - [SlugFillerSafeHash] -- Xanatos --
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

// BEGIN SLUGFILLER: SafeHash // NEO: SSH - [SlugFillerSafeHash] -- Xanatos -->
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
// END SLUGFILLER: SafeHash // NEO: SSH END <-- Xanatos --

#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
void CDownloadQueue::ReSortDownloadSlots(CUpDownClient* client) {
	if(NeoPrefs.UseDownloadBandwidthThrottler() && client)
		theApp.downloadBandwidthThrottler->ReSortDownloadSlots(client);
}

void CDownloadQueue::ProcessReceiving(){
	POSITION posSock = ProcessQueue.GetHeadPosition();
	ThrottledControlSocket* socket = NULL;
	while (posSock){
		socket = ProcessQueue.GetNext(posSock);
		if(socket->ProcessData() == false) // not longer on reciving queue, all datas processed
			RemoveFromProcessQueue(socket);
	}
}

void CDownloadQueue::AddToProcessQueue(ThrottledControlSocket* sock){
	if (sock && !sock->onDownProcessQueue){
		ProcessQueue.AddTail(sock);
		sock->onDownProcessQueue = true;
	}
}

void CDownloadQueue::RemoveFromProcessQueue(ThrottledControlSocket* sock){
	if (sock && sock->onDownProcessQueue){
		sock->onDownProcessQueue = false;
		POSITION pos = ProcessQueue.Find(sock);
		if (pos){
			//if (pos == posSock)
			//	ProcessQueue.GetNext(posSock);
			ProcessQueue.RemoveAt(pos);
		}
	}
}
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --

// NEO: QS - [QuickStart] -- Xanatos -->
void CDownloadQueue::ProcessQuickStart()
{
	if (m_bQuickStartDone)
		return;

	if(NeoPrefs.UseQuickStart() || NeoPrefs.OnQuickStart())
	{
		if(theApp.GetConState()) // NEO: NCC - [NeoConnectionChecker]
		{
			if(!NeoPrefs.OnQuickStart())
				DoQuickStart();
			else if (m_dwQuickStartEndTime <= ::GetTickCount())
				StopQuickStart();
		}
	}
	else
		m_bQuickStartDone = true;
}

void CDownloadQueue::DoQuickStart()
{
	int CountFiles = 0;
	for(POSITION pos = filelist.GetHeadPosition(); pos != NULL;) 
	{ 
		CPartFile* cur_file = filelist.GetNext(pos); 
		if(cur_file->GetStatus() == PS_READY || cur_file->GetStatus() == PS_EMPTY )
			CountFiles ++;
	}
	if(CountFiles == 0)
		return;

	DWORD QuickStartTime = min((CountFiles * NeoPrefs.GetQuickStartTimePerFileMs()), NeoPrefs.GetQuickStartMaxTimeMs());

	if (QuickStartTime > 0){
		m_bQuickStartDone = false;
		NeoPrefs.SetOnQuickStart(true);
		m_dwQuickStartEndTime = ::GetTickCount() + QuickStartTime;
		ModLog(LOG_STATUSBAR, GetResString(IDS_X_QUICK_START_BEGIN), MS2MIN((float)QuickStartTime));
	}else
		m_bQuickStartDone = true;
}

void CDownloadQueue::StopQuickStart()
{
	NeoPrefs.SetOnQuickStart(false);
	m_bQuickStartDone = true;
	ModLog(LOG_STATUSBAR, GetResString(IDS_X_QUICK_START_END));
}
// NEO: QS END <-- Xanatos --

// NEO: FCFG - [FileConfiguration] -- Xanatos -->
void CDownloadQueue::UpdatePartPrefs(CPartPreferences* PartPrefs, CKnownPreferences* KnownPrefs, UINT cat)
{
	POSITION pos= filelist.GetHeadPosition();
	while (pos != 0)
	{
		CPartFile* cur_file = filelist.GetNext(pos);
		if (!cur_file)
			continue;

		CDownloadQueue::UpdatePartPrefs(cur_file, PartPrefs, cat);
		if(KnownPrefs)
			CKnownFileList::UpdateKnownPrefs(cur_file, KnownPrefs, cat);
	}

	Category_Struct* Category = thePrefs.GetCategory(cat);
	ASSERT(Category);
	if(!PartPrefs->IsEmpty() && PartPrefs != Category->PartPrefs){
		ASSERT(Category->PartPrefs == NULL);
		((CPartPreferencesEx*)PartPrefs)->PartPrefs = &NeoPrefs.PartPrefs;
		Category->PartPrefs = PartPrefs;
	}else if(PartPrefs->IsEmpty()){
		delete PartPrefs;
		Category->PartPrefs = NULL;
	}
}

void CDownloadQueue::UpdatePartPrefs(CPartFile* cur_file, CPartPreferences* PartPrefs, UINT cat)
{
	if(!PartPrefs->IsEmpty())
	{
		if(cur_file->GetCategory() == cat && cur_file->PartPrefs != PartPrefs)
		{
			ASSERT(!cur_file->PartPrefs->IsCategoryPrefs());

			if(cur_file->PartPrefs->IsFilePrefs()) 
			{
				((CPartPreferencesEx*)cur_file->PartPrefs)->PartPrefs = PartPrefs;
			}
			else //if(cur_file->PartPrefs->IsGlobalPrefs()) 
			{
				cur_file->PartPrefs = PartPrefs;
			}
		}
	}
	else if(cur_file->PartPrefs == PartPrefs) // && PartPrefs->IsEmpty()
	{
		cur_file->PartPrefs = &NeoPrefs.PartPrefs;
	}
	else if(((CPartPreferencesEx*)cur_file->PartPrefs)->PartPrefs == PartPrefs) // && PartPrefs->IsEmpty()
	{
		((CPartPreferencesEx*)cur_file->PartPrefs)->PartPrefs = &NeoPrefs.PartPrefs;
	}
}
// NEO: FCFG END <-- Xanatos --

// NEO: GSL - [GlobalSourceLimit] -- Xanatos -->
/****************************\
* Function written by Stulle *
* Used ideas from MaxUpload  *
\****************************/
void CDownloadQueue::SetGlobalSourceLimits()
{
/********************\
* Set the variables  *
\********************/

	UINT aCount = 0;
	UINT countsources = 0;
	UINT m_uTollerance = (unsigned)(NeoPrefs.PartPrefs.GetGlobalSourceLimitLimit()*.05);
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
			if(cur_file->PartPrefs->UseGlobalSourceLimit())
				aCount++;

			countsources += cur_file->GetSourceCount() - cur_file->GetSrcStatisticsValue(DS_CACHED); // NEO: XSC - [ExtremeSourceCache]

			 // just a safety check
			 if (cur_file->GetFileHardLimit() < cur_file->GetSourceCount()-  cur_file->GetSrcStatisticsValue(DS_CACHED)) // NEO: XSC - [ExtremeSourceCache]
				 cur_file->DecrHL(); // HL = SrcCount
		}
	}

	if (aCount == 0) // nothing to check here
	{
		m_bGlobalHLSrcReqAllowed = true;
		m_uLastGlobalSourceLimit = ::GetTickCount();
		return;
	}

/********************\
* Get Src difference *
\********************/

	if ((UINT)NeoPrefs.PartPrefs.GetGlobalSourceLimitLimit() < countsources) // get pos result
	{
		m_uSourcesDif = countsources - NeoPrefs.PartPrefs.GetGlobalSourceLimitLimit();
		m_bTooMuchSrc = true;
	}
	else
		m_uSourcesDif = NeoPrefs.PartPrefs.GetGlobalSourceLimitLimit() - countsources;

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
			m_bPassiveMode = true;
			m_bPassiveModeTemp = true;
			m_bGlobalHLSrcReqAllowed = true;
			m_uSourcesDif = ((NeoPrefs.PartPrefs.GetGlobalSourceLimitLimit() + m_uTollerance) - countsources)/aCount;
			AddDebugLogLine(true,_T("{GSL} Global source count is in the tolerance range! PassiveMode!"));
		}
		else
		{
			m_bGlobalHLSrcReqAllowed = true;
			m_uLastGlobalSourceLimit = ::GetTickCount();
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
			m_bPassiveMode = false;
			AddDebugLogLine(true,_T("{GSL} Global source count is not in the tolerance range! Disabled PassiveMode!"));
		}
		if(!m_bTooMuchSrc)
		{
			UINT m_uMaxIncr = m_uTollerance/aCount;
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
		if ((cur_file->GetStatus() == PS_READY || cur_file->GetStatus() == PS_EMPTY) && cur_file->PartPrefs->UseGlobalSourceLimit())
		{
			if(m_bPassiveModeTemp)
				cur_file->SetPassiveHL(m_uSourcesDif);
			else if(!m_bTooMuchSrc)
				cur_file->IncrHL(m_uSourcesDif);
			else
				cur_file->DecrHL();
		}
	}

	m_uLastGlobalSourceLimit = ::GetTickCount();
}

UINT CDownloadQueue::GetGlobalSourceCount(UINT cat) // NEO: CSL - [CategorySourceLimit]
{
	UINT m_uSourceCountTemp = 0;
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
	{
		const CPartFile* cur_file = filelist.GetNext(pos);

		if(cur_file->GetCategory() == cat || cat == (UINT)-1)
			m_uSourceCountTemp = m_uSourceCountTemp + (cur_file->GetSourceCount() - cur_file->GetSrcStatisticsValue(DS_CACHED)); // NEO: XSC - [ExtremeSourceCache]
	}
	return m_uSourceCountTemp;
}
// NEO: GSL END <-- Xanatos --

// NEO: MSR - [ManualSourceRequest] -- Xanatos -->
void CDownloadQueue::ManualLocalRequests(CTypedPtrList<CPtrList, CPartFile*>& ServerReqQueue)
{
	if(!theApp.serverconnect->IsConnected())
		return;

	POSITION pos = ServerReqQueue.GetHeadPosition();
	while(pos != NULL)
	{
		CSafeMemFile dataTcpFrame(22);
		const int iMaxFilesPerTcpFrame = theApp.serverconnect->GetCurrentServer() ? theApp.serverconnect->GetCurrentServer()->GetMaxAsksFromCredits(15) : 15; // NEO: KLC - [KhaosLugdunumCredits] <-- Xanatos --
		int iFiles = 0;

		// NEO: KLC - [KhaosLugdunumCredits] -- Xanatos -->
		// Note: when we exided our credits break, whe the manual reask will be called for the next time, 
		// files we have asked now will have a recent last ask date and will be skilled, so we eill ask to the next files we need.
		// Instad, we may sort the ServerReqQueue list everytime by last ask, but the current sollution is easyer
		if(iMaxFilesPerTcpFrame == 0)
			break;
		// NEO: KLC END <-- Xanatos --

		CPartFile* cur_file;
		while ((pos != NULL) && (iFiles < iMaxFilesPerTcpFrame)){
			cur_file = ServerReqQueue.GetNext(pos);

			//cur_file->m_bLocalSrcReqQueued = false;
			if(cur_file->IsKindOf(RUNTIME_CLASS(CPartFile))) // NEO: ASP - [ActiveSpreading]
			{
				if(cur_file->IsStopped())
					continue;
				if(cur_file->m_LastSearchTime && (::GetTickCount() - cur_file->m_LastSearchTime) < /*PartPrefs.GetSVRIntervalsMs()*/ SERVERREASKTIME)
					continue;
			}

			if (cur_file->IsLargeFile() && (theApp.serverconnect->GetCurrentServer() == NULL || !theApp.serverconnect->GetCurrentServer()->SupportsLargeFilesTCP())){
				ASSERT( false );
				DebugLogError(_T("Large file (%s) on local requestqueue for server without support for large files"), cur_file->GetFileName());
				continue;
			}

			cur_file->m_LastSearchTime = ::GetTickCount();
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

			ModLog(GetResString(IDS_X_COLLECT_SVR_DONE), cur_file->GetFileName());
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
			// NEO: KLC - [KhaosLugdunumCredits] -- Xanatos -->
			if(theApp.serverconnect->GetCurrentServer() != NULL)
				theApp.serverconnect->GetCurrentServer()->FileAsksSent(iFiles); 
			// NEO: KLC END <-- Xanatos --
		}

		// next TCP frame with up to 15 source requests is allowed to be sent in..
		m_dwNextTCPSrcReq = ::GetTickCount() + SEC2MS(iMaxFilesPerTcpFrame*(16+4));
		// X?: should this by alpyed also to manual requests above 15 files???
	}
}

// NEO: XUC - [ExtendedUdpCache]
void CDownloadQueue::CahceUDPServer(CSearchFile* toadd)
{
	const CSimpleArray<CSearchFile::SServer>& aServers = toadd->GetServers();
	for (int i = 0; i < aServers.GetSize(); i++)
	{
		CSKey key(toadd->GetFileHash());
		CSearchFile::SServer* Server = NULL;
		if(m_CachedServers.Lookup(key,(void*&)Server)){
			if(Server->m_uAvail >= aServers[i].m_uAvail)
				continue;
		}else
			Server = new CSearchFile::SServer();

		Server->m_nIP = aServers[i].m_nIP;
		Server->m_nPort = aServers[i].m_nPort;
		Server->m_uAvail = aServers[i].m_uAvail;
		m_CachedServers.SetAt(key, Server);
	}
}
// NEO: XUC END

void CDownloadQueue::ManualGlobalRequests(CTypedPtrList<CPtrList, CPartFile*>& GlobalReqQueue)
{
	if (!theApp.serverconnect->IsConnected())
		return;
	if (!theApp.serverconnect->IsUDPSocketAvailable())
		return;

	// backup the old status
	CServer* old_udpserver = cur_udpserver;

	CServer* pConnectedServer = theApp.serverconnect->GetCurrentServer();
	if (pConnectedServer)
		pConnectedServer = theApp.serverlist->GetServerByAddress(pConnectedServer->GetAddress(), pConnectedServer->GetPort());

	// Ask all servers for all files
	POSITION pos = GlobalReqQueue.GetHeadPosition();
	while(pos != NULL)
	{
		POSITION nextpos = pos;

		CPartFile* cur_file = GlobalReqQueue.GetAt(pos);

		cur_udpserver = NULL;
		//while((cur_udpserver = theApp.serverlist->GetSuccServer(cur_udpserver)) != NULL)
		//{
		//	if (cur_udpserver == pConnectedServer)
		//		continue;

		// NEO: XUC - [ExtendedUdpCache]
		CSKey key(cur_file->GetFileHash());
		CSearchFile::SServer* Server = NULL;
		if(m_CachedServers.Lookup(key,(void*&)Server))
		{
			cur_udpserver = theApp.serverlist->GetServerByIPTCP(Server->m_nIP, Server->m_nPort);
			m_CachedServers.RemoveKey(key);
			delete Server;
		}
		// NEO: XUC END

		if(cur_udpserver == NULL)
			cur_udpserver = theApp.serverlist->GetSuccServer(cur_udpserver);
		if(cur_udpserver == pConnectedServer)
			cur_udpserver = theApp.serverlist->GetSuccServer(cur_udpserver);

		if(cur_udpserver)
		{

			bool bGetSources2Packet = ((cur_udpserver->GetUDPFlags() & SRV_UDPFLG_EXT_GETSOURCES2) > 0);
			bool bServerSupportsLargeFiles = cur_udpserver->SupportsLargeFilesUDP();

			// loop until the packet is filled or a packet was sent
			CSafeMemFile dataGlobGetSources(20);
			int iFiles = 0;
			int iLargeFiles = 0;

			nextpos = pos;

			while (!IsMaxFilesPerUDPServerPacketReached(iFiles,iLargeFiles) && (nextpos != NULL) ){
				cur_file = GlobalReqQueue.GetNext(nextpos);

				if(cur_file->IsStopped())
					continue;
				if(cur_file->m_LastSearchTimeUdp && (::GetTickCount() - cur_file->m_LastSearchTimeUdp) < /*NeoPrefs.PartPrefs.GetUdpGlobalIntervalsMs()*/ UDPSERVERREASKTIME)
					continue;

				if(!bServerSupportsLargeFiles && cur_file->IsLargeFile()) // this server does not support large files :'(
					continue;

				iFiles++;
				if (bGetSources2Packet){
					if (cur_file->IsLargeFile()){
						// GETSOURCES2 Packet Large File (<HASH_16><IND_4 = 0><SIZE_8> *)
						iLargeFiles++;
						dataGlobGetSources.WriteHash16(cur_file->GetFileHash());
						dataGlobGetSources.WriteUInt32(0);
						dataGlobGetSources.WriteUInt64(cur_file->GetFileSize());
					}
					else{
						// GETSOURCES2 Packet (<HASH_16><SIZE_4> *)
						dataGlobGetSources.WriteHash16(cur_file->GetFileHash());
						dataGlobGetSources.WriteUInt32((uint32)(uint64)cur_file->GetFileSize());
					}
				}
				else{
					// GETSOURCES Packet (<HASH_16> *)
					dataGlobGetSources.WriteHash16(cur_file->GetFileHash());
				}

				if (thePrefs.GetDebugServerUDPLevel() > 0 && thePrefs.GetDebugServerSourcesLevel() > 0)
					Debug(_T(">>> Queued  %s to server %-21s (%3u of %3u); Buff  %u(%u)=%s\n"), bGetSources2Packet ? _T("OP__GlobGetSources2") : _T("OP__GlobGetSources1"), ipstr(cur_udpserver->GetAddress(), cur_udpserver->GetPort()), m_iSearchedServers + 1, theApp.serverlist->GetServerCount(), iFiles, iLargeFiles, DbgGetFileInfo(cur_file->GetFileHash()));
			}

			if (dataGlobGetSources.GetLength() == 0)
				break; // No files where as is alowed or that are resumed, exit loop
			
			SendGlobGetSourcesUDPPacket(&dataGlobGetSources, bGetSources2Packet, iFiles, iLargeFiles);
		}

		if(pos == nextpos)
			break; // no usefull servers found, exit main loop

		// shift to the right place and set the last search time
		while ((pos != NULL) && (pos != nextpos)){
			cur_file = GlobalReqQueue.GetNext(pos);

			if(cur_file->IsStopped())
				continue;
			if(cur_file->m_LastSearchTimeUdp && (::GetTickCount() - cur_file->m_LastSearchTimeUdp) < /*NeoPrefs.PartPrefs.GetUdpGlobalIntervalsMs()*/ UDPSERVERREASKTIME)
				continue;

			cur_file->m_LastSearchTimeUdp = ::GetTickCount();

			ModLog(GetResString(IDS_X_COLLECT_UDP_DONE), cur_file->GetFileName());
		}
	}

	// restore the old status the old status
	cur_udpserver = old_udpserver;
}
// NEO: MSR END <-- Xanatos --

// NEO: MOD - [NeoDownloadCommands] -- Xanatos -->
void CDownloadQueue::ExecuteNeoCommand(CTypedPtrList<CPtrList, CPartFile*>& selectedList, uint8 uNeoCmdL, uint8 uNeoCmdW)
{
	switch(uNeoCmdL)
	{
		case INST_COLLECT:
			switch(uNeoCmdW)
			{
				// NEO: MSR - [ManualSourceRequest]
				case INST_COLLECT_ALL_SOURCES: 
					theApp.downloadqueue->ManualLocalRequests(selectedList);	// Get Server Sources
					theApp.downloadqueue->ManualGlobalRequests(selectedList);	// Get UDP Sources
					while (!selectedList.IsEmpty()){
						selectedList.GetHead()->SetXSSourcesCollecting();		// Get X-Change Sources
						selectedList.GetHead()->CollectKADSources();			// Get KAD Source
						selectedList.RemoveHead();
					}
					break;

				case INST_COLLECT_XS_SOURCES: 
					while (!selectedList.IsEmpty()){
						selectedList.GetHead()->SetXSSourcesCollecting();
						selectedList.RemoveHead();
					}
					break;
				case INST_COLLECT_SVR_SOURCES: 
					theApp.downloadqueue->ManualLocalRequests(selectedList);
					break;
				case INST_COLLECT_KAD_SOURCES: 
					while (!selectedList.IsEmpty()){
						selectedList.GetHead()->CollectKADSources();
						selectedList.RemoveHead();
					}
					break;
				case INST_COLLECT_UDP_SOURCES: 
					theApp.downloadqueue->ManualGlobalRequests(selectedList);
					break;
#ifdef VOODOO // NEO: VOODOOx - [VoodooSourceExchange]
				case INST_COLLECT_VOODOO_SOURCES:{
					while (!selectedList.IsEmpty()){
						if(selectedList.GetHead()->KnownPrefs->IsEnableVoodoo())
							theApp.voodoo->ManifestSourceListRequest(selectedList.GetHead());
						selectedList.RemoveHead();
					}
					break;
				}
#endif // VOODOOx // NEO: VOODOOn END

				case INST_AHL_INCREASE: 
					while (!selectedList.IsEmpty()){
						selectedList.GetHead()->ManIncrHL();
						selectedList.RemoveHead();
					}
					break;
				case INST_AHL_DECREASE: 
					while (!selectedList.IsEmpty()){
						selectedList.GetHead()->ManDecrHL();
						selectedList.RemoveHead();
					}
					break;
				// NEO: MSR END
				default:
					DebugLogError(_T("Unknown Extended Instruction: INST_COLLECT; %u"),(uint32)uNeoCmdW);
			}
			break;
		case INST_DROP:
			switch(uNeoCmdW)
			{
				// NEO: MSD - [ManualSourcesDrop]
				case INST_DROP_NNP:
					while (!selectedList.IsEmpty()){
						selectedList.GetHead()->DropSources(DS_NONEEDEDPARTS);
						selectedList.RemoveHead();
					}
					break;
				case INST_DROP_FULLQ:
					while (!selectedList.IsEmpty()){
						selectedList.GetHead()->DropSources(DS_REMOTEQUEUEFULL);
						selectedList.RemoveHead();
					}
					break;
				case INST_DROP_HIGHQ:
					while (!selectedList.IsEmpty()){
						selectedList.GetHead()->DropSources(DS_ONQUEUE);
						selectedList.RemoveHead();
					}
					break;
				// NEO: TCR - [TCPConnectionRetry]
				case INST_DROP_WAITINGRETRY:
					while (!selectedList.IsEmpty()){
						selectedList.GetHead()->DropSources(DS_CONNECTIONRETRY);
						selectedList.RemoveHead();
					}
					break;
				// NEO: TCR END
				// NEO: XSC - [ExtremeSourceCache]
				case INST_DROP_CACHED:
					while (!selectedList.IsEmpty()){
						selectedList.GetHead()->DropSources(DS_CACHED);
						selectedList.RemoveHead();
					}
					break;
				// NEO: XSC END
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
				case INST_DROP_LOADED:
					while (!selectedList.IsEmpty()){
						selectedList.GetHead()->DropSources(DS_LOADED);
						selectedList.RemoveHead();
					}
					break;
#endif // NEO_SS // NEO: NSS END
				case INST_DROP_TOMANY:
					while (!selectedList.IsEmpty()){
						selectedList.GetHead()->DropSources(DS_TOOMANYCONNS);
						selectedList.RemoveHead();
					}
					break;
				case INST_DROP_LOW2LOW:
					while (!selectedList.IsEmpty()){
						selectedList.GetHead()->DropSources(DS_LOWTOLOWIP);
						selectedList.RemoveHead();
					}
					break;
				// NEO: MSD END
				default:
					DebugLogError(_T("Unknown Extended Instruction: INST_DROP; %u/%u"),(uint32)uNeoCmdW);
			}
			break;
		case INST_REASK:
			break;

		case INST_STORAGE:
			switch(uNeoCmdW)
			{
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
				case INST_STORAGE_LOAD:
					while (!selectedList.IsEmpty()){
						selectedList.GetHead()->LoadSources();
						selectedList.RemoveHead();
					}
					break;
				case INST_STORAGE_SAVE:
					while (!selectedList.IsEmpty()){
						selectedList.GetHead()->SaveSources();
						selectedList.RemoveHead();
					}
					break;
 #ifdef NEO_CD // NEO: SFL - [SourceFileList]
				case INST_STORAGE_FIND:
					while (!selectedList.IsEmpty()){
						theApp.sourcelist->FindSources(selectedList.GetHead());
						selectedList.RemoveHead();
					}
					break;
 #endif // NEO_CD // NEO: SFL END
#endif // NEO_SS // NEO: NSS END
				default:
					DebugLogError(_T("Unknown Extended Instruction: INST_REASK; %u"),(uint32)uNeoCmdW);
			}
			break;

		case INST_OTHER:
			switch(uNeoCmdW)
			{
				// NEO: PP - [PasswordProtection]
				case INST_OTHER_PROTECT_SHOW:{
					InputBox inputbox;
					inputbox.SetLabels(GetResString(IDS_X_PWPROT_SHOWIBTITLE),GetResString(IDS_X_PWPROT_SHOWIBDESC), _T(""));
					inputbox.SetPassword(true);	
					inputbox.DoModal();
					CString pass=inputbox.GetInput();
					if (!inputbox.WasCancelled() /*&& pass.GetLength()>0*/){ // for voodoo 
						MD5Sum makeMD5Sum( pass );
						pass = makeMD5Sum.GetHash();

						while (!selectedList.IsEmpty()){
							if ( selectedList.GetHead()->GetPWProt() == pass){
								selectedList.GetHead()->SetPWProtShow(true);
								theApp.emuledlg->transferwnd->downloadlistctrl.ShowFile(selectedList.GetHead());
								//if (selectedList.GetHead()->GetStatus(true) == PS_READY) // NEO: SEF - [ShareAlsoEmptyFiles] // || (thePrefs.IsShareAlsoEmptyFiles() && selectedList.GetHead()->GetStatus(true) == PS_EMPTY)) 
								theApp.emuledlg->sharedfileswnd->sharedfilesctrl.AddFile((CKnownFile*)selectedList.GetHead());
							}
							selectedList.RemoveHead();
						}

					}
					break;
				}
				case INST_OTHER_PROTECT_HIDE:
					while (!selectedList.IsEmpty()){
						if ( selectedList.GetHead()->IsPWProt()){
							selectedList.GetHead()->SetPWProtShow(false);
							theApp.emuledlg->transferwnd->downloadlistctrl.HideFile(selectedList.GetHead());
							theApp.emuledlg->sharedfileswnd->sharedfilesctrl.RemoveFile((CKnownFile*)selectedList.GetHead());
						}
						selectedList.RemoveHead();
					}
					break;
				case INST_OTHER_PROTECT_SET:{
					InputBox inputbox1, inputbox2;
					CString pass1 = _T(""), pass2 = _T("");
					MD5Sum makeMD5Sum;

					inputbox1.SetLabels(GetResString(IDS_X_PWPROT_SETIBTITLE),GetResString(IDS_X_PWPROT_SETIBDESC), _T(""));
					//inputbox1.SetPassword(true);
					if (inputbox1.DoModal() == IDOK)
						pass1=inputbox1.GetInput();
					else 
						break;

					while(!selectedList.IsEmpty()) { 
						CPartFile *file = selectedList.GetHead();
						if ( !file->IsPWProt() ) {
							file->SetPWProtShow(true);
							file->SetPWProt( (pass1.GetLength() == 0) ? _T("") : makeMD5Sum.Calculate( pass1 ) );
						}
						selectedList.RemoveHead();
					}
					break;
				}
				case INST_OTHER_PROTECT_CHANGE:{
					InputBox inputbox1, inputbox2;
					CString pass1 = _T(""), pass2 = _T("");
					MD5Sum makeMD5Sum;

					inputbox1.SetLabels(GetResString(IDS_X_PWPROT_OLDPWIBTITLE),GetResString(IDS_X_PWPROT_OLDPWIBDESC), _T(""));
					inputbox1.SetPassword(true);
					if (inputbox1.DoModal() == IDOK) {
						pass1=inputbox1.GetInput();
						if (pass1.GetLength() == 0)
							break;
					} else 
						break;

					inputbox2.SetLabels(GetResString(IDS_X_PWPROT_SETIBTITLE),GetResString(IDS_X_PWPROT_SETIBDESC), _T(""));
					//inputbox2.SetPassword(true);
					if (inputbox2.DoModal() == IDOK){
						pass2=inputbox2.GetInput();
						if (pass2.GetLength() == 0)
							if(AfxMessageBox(GetResString(IDS_X_PWPROT_CLEARPW), MB_YESNO|MB_ICONQUESTION) == IDNO)
								break;
					}

					while(!selectedList.IsEmpty()) { 
						CPartFile *file = selectedList.GetHead();
						if ( file->IsPWProt() ) {
							if ( makeMD5Sum.Calculate( pass1 ) == file->GetPWProt() ) {
								file->SetPWProtShow(true);
								file->SetPWProt( (pass2.GetLength() == 0) ? _T("") : makeMD5Sum.Calculate( pass2 ) );
							}
						}
						selectedList.RemoveHead();
					}
					break;
				}
				case INST_OTHER_PROTECT_UNSET:{
					InputBox inputbox1;
					CString pass1 = _T("");
					MD5Sum makeMD5Sum;

					inputbox1.SetLabels(GetResString(IDS_X_PWPROT_SHOWIBTITLE),GetResString(IDS_X_PWPROT_UNSETIBDESC), _T(""));
					inputbox1.SetPassword(true);
					if (inputbox1.DoModal() == IDOK) {
						pass1=inputbox1.GetInput();
						if (pass1.GetLength() == 0)
							break;	
					} else 
						break;

					while(!selectedList.IsEmpty()) { 
						CPartFile *file = selectedList.GetHead();
						if ( file->IsPWProt() )
							if ( makeMD5Sum.Calculate( pass1 ) == file->GetPWProt()) {
								file->SetPWProt( _T("") );
							}
						selectedList.RemoveHead();
					}
					break;
				}
				// NEO: FCFG - [FileConfiguration]
				case INST_OTHER_PROPERTIES:{
					CSimpleArray<CAbstractFile*> paFiles;
					while(!selectedList.IsEmpty()){
						paFiles.Add(selectedList.GetHead());
						selectedList.RemoveHead();
					}
					// NEO: MLD - [ModelesDialogs] 
					CFileDetailDialog* dlg = new CFileDetailDialog(&paFiles, 0, &theApp.emuledlg->transferwnd->downloadlistctrl);
					dlg->OpenDialog(); 
					// NEO: MLD END
					//CFileDetailDialog dialog(&paFiles, 0, &theApp.emuledlg->transferwnd->downloadlistctrl);
					//dialog.DoModal();
					break;
				}
				case INST_OTHER_PREFERENCES:{
					CSimpleArray<CKnownFile*> paFiles;
					while(!selectedList.IsEmpty()){
						paFiles.Add(selectedList.GetHead());
						selectedList.RemoveHead();
					}
					// NEO: MLD - [ModelesDialogs] 
					CFilePreferencesDialog* dlg = new CFilePreferencesDialog(&paFiles, 0, &theApp.emuledlg->transferwnd->downloadlistctrl);
					dlg->OpenDialog(); 
					// NEO: MLD END
					//CFilePreferencesDialog dialog(&paFiles, 0, &theApp.emuledlg->transferwnd->downloadlistctrl);
					//dialog.DoModal();
					break;
				}
				// NEO: FCFG END
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
				case INST_OTHER_VOODOO_LIST:{
					// NEO: MLD - [ModelesDialogs] 
					CVoodooListDlg* dlg = new CVoodooListDlg();
					dlg->OpenDialog(); 
					// NEO: MLD END
					//CVoodooListDlg dlg;
					//dlg.DoModal();
					break;
				}
#endif // VOODOO // NEO: VOODOO END
				default:
					DebugLogError(_T("Unknown Extended Instruction: INST_REASK; %u"),(uint32)uNeoCmdW);
			}
			break;

		default:
			DebugLogError(_T("Unknown Download Command: %u %u"),(uint32)uNeoCmdL,(uint32)uNeoCmdW);
	}

#ifdef VOODOO // NEO: VOODOOn - [VoodooExtensionForNeo]
	if(NeoPrefs.UseVoodooTransfer() && uNeoCmdL != INST_OTHER) // don't send local commands!
		theApp.voodoo->ManifestDownloadCommand(selectedList,uNeoCmdL,uNeoCmdW);
#endif // VOODOO // NEO: VOODOOn END
}
// NEO: MOD END <-- Xanatos --

// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
void CDownloadQueue::StopPauseLastFile(int Mode, int Category) {
	CPartFile*  pfile = NULL;
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;filelist.GetNext(pos)) {
		CPartFile* cur_file = filelist.GetAt(pos);
		if ((cur_file->GetStatus() < 4) && (cur_file->GetCategory() == (UINT)Category || Category == -1)) {
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

int CDownloadQueue::GetMaxCatResumeOrder(int iCategory /* = 0*/)
{
	int	max = 0;
	
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;filelist.GetNext(pos))
	{
		CPartFile* cur_file = filelist.GetAt(pos);
		if (cur_file->GetCategory() == (UINT)iCategory && cur_file->GetCatResumeOrder() > max)
			max = cur_file->GetCatResumeOrder();
	}

	return max;
}

void CDownloadQueue::PurgeED2KLinkQueue()
{
	if (m_ED2KLinkQueue.IsEmpty()) 
		return;

	int		useCat = 0;
	bool	bCanceled = false; 
	bool	bCreatedNewCat = false;

	m_iNextLinkQueuedTick = 0xFFFFFFFF; // we are busy right now
	if (NeoPrefs.SelectCatForNewDL()) // may have been dissabled
	{
		CSelCategoryDlg catDlg;
		catDlg.DoModal();
		
		useCat = catDlg.GetInput();
		bCreatedNewCat = catDlg.CreatedNewCat();
		bCanceled = useCat == -1;
	}
	m_iNextLinkQueuedTick = 0; // wen can "unlock" this flag now its all in one thread

	for (POSITION pos = m_ED2KLinkQueue.GetHeadPosition(); pos != 0; m_ED2KLinkQueue.GetNext(pos))
	{
		CED2KFileLink*	pLink = m_ED2KLinkQueue.GetAt(pos);
		if (bCanceled) {
			delete pLink;
			continue;
		}

		ASSERT(useCat != -1);
		AddFileLinkToDownload(pLink, useCat);
	}
	
	m_ED2KLinkQueue.RemoveAll();

	if(bCreatedNewCat)
		StartDLInEmptyCats(useCat);
}

void CDownloadQueue::StartDLInEmptyCats(int useCat)
{
	if (NeoPrefs.StartDLInEmptyCats() && thePrefs.AddNewFilesPaused())
		for (int i = 0; i < NeoPrefs.StartDLInEmptyCatsAmount(); i++)
			if (!StartNextFile(useCat)) break;
}

// khaos::categorymod+ GetAutoCat returns a category index of a category
int CDownloadQueue::GetAutoCat(CString sFullName, EMFileSize nFileSize)
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
bool CDownloadQueue::ApplyFilterMask(CString sFullName, int nCat)
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

// Returns the number of files in a category.
UINT CDownloadQueue::GetCategoryFileCount(int iCategory)
{
	UINT uCount = 0;

	for (POSITION pos = filelist.GetHeadPosition(); pos != NULL; filelist.GetNext(pos))
	{
		CPartFile* cur_file = filelist.GetAt(pos);
		if (cur_file->GetCategory() == (UINT)iCategory) uCount++;
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
		if (nCount < curFile->GetAvailableSrcCount() && (nCat == -1 || curFile->GetCategory() == (UINT)nCat))
			nCount = curFile->GetAvailableSrcCount();
	}

	return nCount;
}

// NEO: NXC END <-- Xanatos --

#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
void CDownloadQueue::GetTipInfoByCat(uint8 cat, CString &info)
{
	int count,dwl,err,compl,paus;
	count=dwl=err=compl=paus=0;
	float speed = 0;
	uint64 size = 0;
	uint64 trsize = 0;
	uint64 disksize = 0;

	for (POSITION pos = filelist.GetHeadPosition(); pos; )
	{
		CPartFile* cur_file = filelist.GetNext(pos);
		//if (CheckShowItemInGivenCat(cur_file,cat)) // modified by rayita
		if (cur_file && cur_file->CheckShowItemInGivenCat(cat))
		{
			count++;
			if (cur_file->GetTransferringSrcCount()>0) ++dwl;
			speed += cur_file->GetDatarate()/1024.0f;
			size += (uint64)cur_file->GetFileSize();
			trsize += (uint64)cur_file->GetCompletedSize();
			disksize += (uint64)cur_file->GetRealFileSize();
			if (cur_file->GetStatus()==PS_ERROR) ++err;
			if (cur_file->GetStatus()==PS_PAUSED) ++paus;
		}
	}

	int total;
	compl=theApp.emuledlg->transferwnd->downloadlistctrl.GetCompleteDownloads(cat,total);

	// ZZ:DownloadManager -->
	CString prio = GetResString(IDS_PRIONORMAL);
	if (thePrefs.GetCategory(cat) != NULL)
	{
		switch(thePrefs.GetCategory(cat)->prio) {
			case PR_LOW:
				prio = GetResString(IDS_PRIOLOW);
				break;

			case PR_HIGH:
				prio = GetResString(IDS_PRIOHIGH);
				break;		
		}
	}
// ZZ:DownloadManager <--

	info.Format(GetResString(IDS_X_DLTAB_TL), thePrefs.GetCategory(cat) ? thePrefs.GetCategory(cat)->strTitle : _T(""));
	info.AppendFormat(GetResString(IDS_X_FILES), count);
	info.AppendFormat(GetResString(IDS_X_DOWNLOADING), dwl);
	info.AppendFormat(GetResString(IDS_X_PAUSED), paus);
	info.AppendFormat(GetResString(IDS_X_ERRONEOUS), err);
	info.AppendFormat(GetResString(IDS_X_COMPLETED), compl);
	info.AppendFormat(GetResString(IDS_X_DLTAB_SP), speed);
	info.AppendFormat(GetResString(IDS_X_DLTAB_SZ), CastItoXBytes(trsize, false, false),CastItoXBytes(size, false, false));
	info.AppendFormat(GetResString(IDS_X_DLTAB_DS), CastItoXBytes(disksize, false, false));
	info.AppendFormat(GetResString(IDS_X_DLTAB_PR), prio); // ZZ:DownloadManager
}

void CDownloadQueue::GetTransferTipInfo(CString &info)
{
	int count = 0;
	int dwl = 0;	
	uint64 size = 0;
	uint64 trsize = 0;
	uint64 disksize = 0;

	for (POSITION pos = filelist.GetHeadPosition(); pos; )
	{
		CPartFile* cur_file = filelist.GetNext(pos);
		count++;
		if (cur_file->GetTransferringSrcCount() > 0) ++dwl;		
		size += (uint64)cur_file->GetFileSize();
		trsize += (uint64)cur_file->GetCompletedSize();
		disksize += (uint64)cur_file->GetRealFileSize();
	}

	info.Format(GetResString(IDS_X_DL_SP), theApp.bandwidthControl->GetCurrStatsDataDownload(), theApp.bandwidthControl->GetCurrStatsDataDownload());
	info.AppendFormat(GetResString(IDS_X_DLTAB_DL), dwl, count);
	info.AppendFormat(GetResString(IDS_X_DLTAB_SZ), CastItoXBytes(trsize, false, false), CastItoXBytes(size, false, false));
	info.AppendFormat(GetResString(IDS_X_DLTAB_DS), CastItoXBytes(disksize, false, false));

#ifdef NEO_BC // NEO: NMFS - [NiceMultiFriendSlots]
	UINT ActivatedRatioReason = theApp.bandwidthControl->IsSessionRatio();
	info.AppendFormat(GetResString(IDS_X_SESSION_RATIO2),ActivatedRatioReason ? (ActivatedRatioReason > 1 ? GetResString(IDS_X_SESSION_RATIO_ENABLED_ZZ) : GetResString(IDS_X_SESSION_RATIO_ENABLED)):GetResString(IDS_X_SESSION_RATIO_DISABLED));
	info.AppendFormat(GetResString(IDS_X_ZZRATIO_CHECK02),(theApp.bandwidthControl->IsSessionRatioWorking())?GetResString(IDS_YES):GetResString(IDS_NO));
	info.AppendFormat(GetResString(IDS_X_ZZRATIO_CHECK12),(ActivatedRatioReason & 1)?GetResString(IDS_YES):GetResString(IDS_NO));
	info.AppendFormat(GetResString(IDS_X_ZZRATIO_CHECK22),(ActivatedRatioReason & 2)?GetResString(IDS_YES):GetResString(IDS_NO));
#endif // NEO_BC // NEO: NMFS END

}
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --