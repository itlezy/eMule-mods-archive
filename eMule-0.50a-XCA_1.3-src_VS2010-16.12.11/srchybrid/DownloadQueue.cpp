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
#include "Defaults.h"// X: [POFC] - [PauseOnFileComplete]
#include "VolumeInfo.h" // X: [FSFS] - [FileSystemFeaturesSupport]
#include "KnownFileList.h" //Xman [MoNKi: -Check already downloaded files-]// X-Ray :: SearchCatch

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#define	MS2SEC(ms)	((ms)/1000)
#define	MS2MIN(ms)	MS2SEC((ms)/60)

CDownloadQueue::CDownloadQueue()
{
	filesrdy = 0;
	//datarate = 0;
	cur_udpserver = 0;
	lastfile = 0;
	lastcheckdiskspacetime = 0;	// SLUGFILLER: checkDiskspace
	lastudpsearchtime = 0;
	lastudpstattime = 0;
	SetLastKademliaFileRequest();
	udcounter = 0;
	m_iSearchedServers = 0;
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
	// NEO: QS - [QuickStart] -- Xanatos -->
	m_bQuickStartDone = false; 
	m_dwQuickStartEndTime = 0;
	// NEO: QS END <-- Xanatos --

    //m_dwLastA4AFtime = 0; // ZZ:DownloadManager
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
	// find all part files, read & hash them if needed and store into a list
	CFileFind ff;
	int count = 0;

	for (size_t i=0;i<thePrefs.tempdir.GetCount();i++) {
		//Xman
		CAtlList<CString> metsfound;	// SLUGFILLER: SafeHash - ensure each met is loaded once per tempdir

		CString searchPath=thePrefs.GetTempDir(i);

		searchPath += _T("\\*.part.met");

		//check all part.met files
		bool end = !ff.FindFile(searchPath, 0);
		while (!end){
			end = !ff.FindNextFile();
			if (ff.IsDirectory())
				continue;
			//Xman
			// BEGIN SLUGFILLER: SafeHash - one is enough
			if (metsfound.Find(CString(ff.GetFileName()).MakeLower()))
				continue;
			//metsfound.AddTail(CString(ff.GetFileName()).MakeLower()); //MORPH - Moved Down, to allow checking for backup met files.
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
				//Xman
				// SLUGFILLER: SafeHash remove - part files are shared later
				/*
				if (toadd->GetStatus(true) == PS_READY)
					theApp.sharedfiles->SafeAddKFile(toadd); // part files are always shared files
				*/
				theApp.emuledlg->transferwnd->GetDownloadList()->AddFile(toadd);// show in downloadwindow
				if(!toadd->IsStopped())
					toadd->m_sourcesaver.AddSourcesToDownload(); // X: [ISS] - [Improved Source Save]
			}
			else
				delete toadd;
		}
		ff.Close();

		//try recovering any part.met files
		//zz_fly :: better .part.met file backup and recovery :: Enig123 :: Start
		// .backup failed, load .bak
		/*
		searchPath += PARTMET_TMP_EXT;
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
			//metsfound.AddTail(RemoveFileExtension(CString(ff.GetFileName()).MakeLower())); //MORPH - Moved Down, to allow checking for backup met files.
			// END SLUGFILLER: SafeHash
			CPartFile* toadd = new CPartFile();
			if (toadd->LoadPartFile(thePrefs.GetTempDir(i), ff.GetFileName()) == PLR_LOADSUCCESS){
				metsfound.AddTail(RemoveFileExtension(CString(ff.GetFileName()).MakeLower())); //MORPH - Added, fix SafeHash
				toadd->SavePartFile(true); // resave backup, don't overwrite existing bak files yet
				count++;
				filelist.AddTail(toadd);			// to downloadqueue
				//Xman
				// SLUGFILLER: SafeHash remove - part files are shared later
				/*
				if (toadd->GetStatus(true) == PS_READY)
					theApp.sharedfiles->SafeAddKFile(toadd); // part files are always shared files
				*/
				theApp.emuledlg->transferwnd->GetDownloadList()->AddFile(toadd);// show in downloadwindow
				if(!toadd->IsStopped())
					toadd->m_sourcesaver.AddSourcesToDownload(); // X: [ISS] - [Improved Source Save]

				AddLogLine(false, GetResString(IDS_RECOVERED_PARTMET), toadd->GetFileName());
			}
			else {
				delete toadd;
			}
		}
		ff.Close();
	}
	theApp.SplashHide(SW_SHOW); //Xman new slpash-screen arrangement
	if(count == 0) {
		AddLogLine(false,GetResString(IDS_NOPARTSFOUND));
	} else {
		AddLogLine(false,GetResString(IDS_FOUNDPARTS),count);
		SortByPriority();
		CheckDiskspace();
	}
	VERIFY( m_srcwnd.CreateEx(0, AfxRegisterWndClass(0), _T("eMule Async DNS Resolve Socket Wnd #2"), WS_OVERLAPPED, 0, 0, 0, 0, NULL, NULL));

	ExportPartMetFilesOverview();
}

CDownloadQueue::~CDownloadQueue(){
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;)
		delete filelist.GetNext(pos);
	m_srcwnd.DestroyWindow(); // just to avoid a MFC warning
}

void CDownloadQueue::AddSearchToDownload(CSearchFile* toadd, uint8 paused, size_t cat)
{
	// X-Ray :: SearchCatch :: Start
	/*
	if (toadd->GetFileSize()== (uint64)0 || IsFileExisting(toadd->GetFileHash()))
		return;

	if (toadd->GetFileSize() > OLD_MAX_EMULE_FILE_SIZE && !thePrefs.CanFSHandleLargeFiles(cat)){ // X: [TD] - [TempDir] moved from CED2KFileLink
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_FSCANTHANDLEFILE));
		return;
	}

	CPartFile* newfile = new CPartFile(toadd,cat);
	if (newfile->GetStatus() == PS_ERROR){
		delete newfile;
		return;
	}

	if (paused == 2)
		paused = (uint8)thePrefs.AddNewFilesPaused();
	AddDownload(newfile, (paused==1));
	*/
	if (toadd->GetFileSize() == (uint64)0)
		return;

	if (toadd->GetFileSize() > OLD_MAX_EMULE_FILE_SIZE && !thePrefs.CanFSHandleLargeFiles(cat)){
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_FSCANTHANDLEFILE));
		return;
	}

	CPartFile* newfile;
	if (!IsFileExisting(toadd->GetFileHash())) 
	{
		if ( !theApp.knownfiles->CheckAlreadyDownloadedFileQuestion(toadd->GetFileHash(), toadd->GetFileName()) )// X-Ray :: SearchCatch
			return;

		newfile = new CPartFile(toadd,cat);
		if (newfile->GetStatus() == PS_ERROR){
			delete newfile;
			return;
		}

		if (paused == 2)
			paused = (uint8)thePrefs.AddNewFilesPaused();
		AddDownload(newfile, (paused==1));
	}
	else if ((newfile = GetFileByID(toadd->GetFileHash())) == NULL)
		return;
	// X-Ray :: SearchCatch :: End

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

void CDownloadQueue::AddSearchToDownload(CString link, uint8 paused, size_t cat)
{
	CPartFile* newfile = new CPartFile(link, cat);
	if (newfile->GetStatus() == PS_ERROR){
		delete newfile;
		return;
	}

	if (paused == 2)
		paused = (uint8)thePrefs.AddNewFilesPaused();
	AddDownload(newfile, (paused==1));
}

void CDownloadQueue::StartNextFileIfPrefs(size_t cat) {
    if (thePrefs.StartNextFile())
		StartNextFile((thePrefs.StartNextFile() > 1?cat:(size_t)-1), (thePrefs.StartNextFile()!=3));
}

void CDownloadQueue::StartNextFile(size_t cat, bool force){

	CPartFile*  pfile = NULL;
	CPartFile* cur_file ;
	POSITION pos;

	if (cat != (size_t)-1) {
        // try to find in specified category
		for (pos = filelist.GetHeadPosition();pos != 0;){
			cur_file = filelist.GetNext(pos);
			if (cur_file->GetStatus()==PS_PAUSED &&
                (
				 cur_file->GetCategory()==(UINT)cat || 
				 cat==0 && thePrefs.GetCategory(0)->filter==0 && cur_file->GetCategory()>0
                ) &&
                CPartFile::RightFileHasHigherPrio(pfile, cur_file)
				&& (!thePrefs.m_bPauseOnFileComplete || cur_file->xState == PFS_NORMAL)// X: [POFC] - [PauseOnFileComplete]
			   ) {
    			pfile = cur_file;
			}
		}
		if (pfile == NULL && !force)
			return;
	}

    if(cat == (size_t)-1 || pfile == NULL && force) {
	    for (pos = filelist.GetHeadPosition();pos != 0;){
		    cur_file = filelist.GetNext(pos);
		    if (cur_file->GetStatus() == PS_PAUSED &&
                CPartFile::RightFileHasHigherPrio(pfile, cur_file)
				&& (!thePrefs.m_bPauseOnFileComplete || cur_file->xState == PFS_NORMAL))// X: [POFC] - [PauseOnFileComplete]
		    {
                // pick first found matching file, since they are sorted in prio order with most important file first.
			    pfile = cur_file;
		    }
	    }
    }
	if (pfile) pfile->ResumeFile();
}

void CDownloadQueue::AddFileLinkToDownload(CED2KFileLink* pLink, size_t cat, LPCTSTR filename)// X: [IP] - [Import Parts]
{
	if (pLink->GetSize() > OLD_MAX_EMULE_FILE_SIZE && !thePrefs.CanFSHandleLargeFiles(cat)){ // X: [TD] - [TempDir] moved from CED2KFileLink
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_FSCANTHANDLEFILE));
		return;
	}
	CPartFile* partfile = new CPartFile(pLink, cat, filename);
	if (partfile->GetStatus() == PS_ERROR){
		delete partfile;
		partfile = GetFileByID(pLink->GetHashKey());
	}
	else {
		AddDownload(partfile,thePrefs.AddNewFilesPaused());
	}

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
		else if(thePrefs.GetLogPartmismatch())//Xman Log part/size-mismatch
			DebugLogWarning(_T("FileIdentifier mismatch when trying to add ed2k link to existing download - AICH Hash or Size might differ, no sources added. File: %s"),
				partfile->GetFileName());
	}

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
	

	filelist.AddTail(newfile);
	SortByPriority();
	CheckDiskspace();	// SLUGFILLER: checkDiskspace
	theApp.emuledlg->transferwnd->GetDownloadList()->AddFile(newfile);
	Log(LOG_STATUSBAR | LOG_DONTNOTIFY, GetResString(IDS_NEWDOWNLOAD), newfile->GetFileName());
	CString msgTemp;
	msgTemp.Format(GetResString(IDS_NEWDOWNLOAD) + _T('\n'), newfile->GetFileName());
	theApp.emuledlg->ShowNotifier(msgTemp, TBN_DOWNLOADADDED);
	ExportPartMetFilesOverview();
}

bool CDownloadQueue::IsFileExisting(const uchar* fileid, bool bLogWarnings) const
{
	const CKnownFile* file = theApp.sharedfiles->GetFileByID(fileid);
	if (file){
		if (bLogWarnings){
			LogWarning(LOG_STATUSBAR, GetResString(file->IsPartFile()?IDS_ERR_ALREADY_DOWNLOADING:IDS_ERR_ALREADY_DOWNLOADED), file->GetFileName());
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
void CDownloadQueue::Process(){
	ProcessLocalRequests(); // send src requests to local server

	// Elapsed time (TIMER_PERIOD not accurate)	
	uint32 deltaTime = ::GetTickCount() - m_lastProcessTime;
	m_lastProcessTime += deltaTime;

	// - Maella -New bandwidth control-
	// Anticipate high CPU load => unregular cycle
	if(deltaTime > 0){
		ASSERT(deltaTime<=0x80000000);// X: possible overflow
		/*
		const float maxDownload = (theApp.glob_prefs->GetNAFCEnable() == false && theApp.glob_prefs->GetNAFCFullControl() == false) ? 
		app_prefs->GetMaxDownload() : theApp.pBandWidthControl->GetMaxDownload();
		*/
		//Xman 1:3 Raitio
		//Xman GlobalMaxHarlimit for fairness
		const bool limitbysources=GetGlobalSources()> thePrefs.m_uMaxGlobalSources && thePrefs.m_bAcceptsourcelimit==false;
		const float maxDownload = theApp.pBandWidthControl->GetMaxDownloadEx(limitbysources); //in [kb/s]
		if(limitbysources)
			m_limitstate=1; //at least session ratio
		else
			m_limitstate=0; //no limit
		//Xman end
		//Xman end

		const bool isLimited = (maxDownload < UNLIMITED);

		if(isLimited == false)
		{
			m_nDownloadSlopeControl = 0;
		}
		else 
		{
			//Xman 
			//application can hang. compensate to 1 second:
			if(deltaTime>1000)
				deltaTime=1000;

			//Xman GlobalMaxHarlimit for fairness
			if(limitbysources && maxDownload < theApp.pBandWidthControl->GetMaxDownloadEx(false))
				m_limitstate=2; //session ratio is reached->full limitation 
			//Xman end

			else if (thePrefs.Is13Ratio()) //downloadlimit although it should be unlimited => we have a ratio
				m_limitstate=3;
			else if (maxDownload < thePrefs.GetMaxDownload()) //can only be NAFC
				m_limitstate=4;

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

		theStats.m_fGlobalDone = 0;
		theStats.m_fGlobalSize = 0;
		theStats.m_dwOverallStatus=0;
		// Remark: filelist is not sorted by priority (see 'balancing' below), needed to priorize the connection (e.g. during start-up)
		static const uint8 prioritys[] = {PR_HIGH, PR_NORMAL, PR_LOW};
		for(int p = 0; p < _countof(prioritys); p++)
		{
			uint8 priority = prioritys[p];
			POSITION next_pos = filelist.GetHeadPosition();
			for(size_t i=0; i<filelist.GetCount(); i++)
			{
				POSITION cur_pos = next_pos;
				const size_t count = filelist.GetCount(); // Could changed => to check
				CPartFile* cur_file = filelist.GetNext(next_pos); // Already point to the next element
				
				if(cur_file && cur_file->GetDownPriority() == priority)
				{ 
					// maintain global download stats
					theStats.m_fGlobalDone += (uint64)cur_file->GetCompletedSize();
					theStats.m_fGlobalSize += (uint64)cur_file->GetFileSize();

					if (cur_file->GetTransferringSrcCount()>0)
						theStats.m_dwOverallStatus  |= STATE_DOWNLOADING;
					if (cur_file->GetStatus() == PS_ERROR)
						theStats.m_dwOverallStatus  |= STATE_ERROROUS;
					else if(cur_file->GetStatus() == PS_READY || cur_file->GetStatus() == PS_EMPTY)
					{
						//Xman sourcecache
						cur_file->ProcessSourceCache();
						//Xman end

						// The method must be called regardless of nDownloadSlopeControl
						// The method returns the size of the received blocks if the download rate is limited (otherwise zero)
						uint32 maxAmmount = (nDownloadSlopeControl <= 0) ? 0 : (uint32)nDownloadSlopeControl;
						uint32 receivedBlock = cur_file->Process(maxAmmount, isLimited, udcounter == 0);						
						if(receivedBlock > 0)
						{
							nDownloadSlopeControl -= receivedBlock;
							// Try to 'balance' the download between sources (=> clients).
							// Move the 'uploaded' at the end of the list.
							if(isLimited == true && count == filelist.GetCount() && cur_file->GetStatus() == PS_READY)
							{
								// To check if these line are a source of bug
								filelist.RemoveAt(cur_pos);
								filelist.AddTail(cur_file); 
							}
						}
					}
				}
			}
		}
	}
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
		uint64 TotalBuffer = 0;// X: [GB] - [Global Buffer]
		for (POSITION pos = filelist.GetHeadPosition(); pos != NULL;)
		{
			CPartFile* cur_file =  filelist.GetNext(pos);

			if(cur_file)
			{
				cur_file->UpdateAutoDownPriority();
				// This will make sure we don't keep old sources to paused and stoped files..
				// Remark: don't need to be processed every 50/100 ms
				cur_file->StopPausedFile();
				TotalBuffer += cur_file->m_nTotalBufferData;
			}
		}
		// X: [GB] - [Global Buffer]
		if(TotalBuffer > thePrefs.m_uGlobalBufferSize){
			UINT downloading = GetDownloadingFileCount();
			const UINT_PTR avgBufferSize = thePrefs.m_uGlobalBufferSize / max(downloading, 2);
			const DWORD curTick = ::GetTickCount();
			CPartFile* maxbufferfile = NULL;			
			POSITION pos = filelist.GetHeadPosition();
			while(pos){// flush the files which buffer size >= avgBufferSize
				CPartFile* cur_file = filelist.GetNext(pos);
				if(cur_file
					&& cur_file->m_nNextFlushBufferTime == 0
					&& (cur_file->GetDownloadDatarate10()*4<theApp.emuledlg->GetDownloadDatarate()?
					cur_file->m_nTotalBufferData*2 >= avgBufferSize:cur_file->m_nTotalBufferData >= avgBufferSize)
					&& cur_file->GetStatus() != PS_INSUFFICIENT
					&& cur_file->GetStatus() != PS_ERROR){
						// Avoid flushing while copying preview file
						if(!cur_file->m_bPreviewing && (!maxbufferfile || cur_file->m_nTotalBufferData > maxbufferfile->m_nTotalBufferData))
							maxbufferfile = cur_file;
						cur_file->m_nNextFlushBufferTime = curTick;
				}
			}
			if(maxbufferfile){
				TotalBuffer -= maxbufferfile->m_nTotalBufferData;
				maxbufferfile->FlushBuffer();
				TotalBuffer += maxbufferfile->m_nTotalBufferData;
			}
		}
		else{
			static size_t flushcounter = 0;
			if(flushcounter > 15){ // 15 secs, if file is not downloading, flush buffers, or check whether there is one part finished and flush this part
				flushcounter = 0;
				POSITION pos = filelist.GetHeadPosition();
				while(pos){
					CPartFile* cur_file = filelist.GetNext(pos);
					if(cur_file
						&& cur_file->m_nNextFlushBufferTime == 0
						&& cur_file->m_nTotalBufferData
						&& cur_file->GetStatus() != PS_INSUFFICIENT
						&& cur_file->GetStatus() != PS_ERROR)
						cur_file->FlushBuffer(false, false, true);
				}
			}
			else
				++flushcounter;
		}
		theApp.emuledlg->transferwnd->ShowBufferUsage(TotalBuffer);

		// [TPT] - Patch
		if (theApp.serverconnect->IsUDPSocketAvailable())
		{
			if ((!lastudpsearchtime) || (::GetTickCount() - lastudpsearchtime) > UDPSERVERREASKTIME)
				SendNextUDPPacket();
		}
	}
	//end - Maella -New bandwidth control-
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

CPartFile* CDownloadQueue::GetFileByIndex(size_t index) const
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

//Xman Xtreme Downloadmanager
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
				/*
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
					//if(cur_client->GetDownloadState() != DS_CONNECTED) {
                    //    cur_client->SwapToAnotherFile(_T("New A4AF source found. CDownloadQueue::CheckAndAddSource()"), false, false, false, NULL, true, false); // ZZ:DownloadManager
                    //}
					//srcLock.Unlock();	//zz_fly :: make source add action thread safe :: Enig123
					return false;
				}
				else{
					delete source;
					//srcLock.Unlock();	//zz_fly :: make source add action thread safe :: Enig123
					return false;
				}*/
				if (cur_file != sender && cur_client->AddRequestForAnotherFile(sender))// X: [CI] - [Code Improvement]
					theApp.emuledlg->transferwnd->GetDownloadList()->AddSource(sender,cur_client,true);// set request for this source
				delete source;
				//srcLock.Unlock();	//zz_fly :: make source add action thread safe :: Enig123
				return false;
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
#ifdef CLIENTANALYZER
			|| (((::GetTickCount() - source->GetLastAskedTime())<(150*60*1000)) /*last asked was shorter than 150 minutes */ && (source->IsEmuleClient()==false || source->IsBadGuy()))	))
#else
			|| (((::GetTickCount() - source->GetLastAskedTime())<(150*60*1000)) /*last asked was shorter than 150 minutes */ && (source->IsEmuleClient()==false)) ))
#endif
		{
			//AddDebugLogLine(false, _T("-o- rejected dropped client %s, %s reentering downloadqueue after time: %u min"), source->GetClientVerString(), source->GetUserName(), (::GetTickCount()-source->droptime)/60000);
			//srcLock.Unlock();	//zz_fly :: make source add action thread safe :: Enig123
			return false;
		}
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
	source->droptime=0;
	source->enterqueuetime=0;
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
#ifdef CLIENTANALYZER
		|| (((::GetTickCount() - source->GetLastAskedTime())<(150*60*1000)) /*last asked was shorter than 150 minutes */ && (source->IsEmuleClient()==false || source->IsBadGuy()))	))
#else
		|| (((::GetTickCount() - source->GetLastAskedTime())<(150*60*1000)) /*last asked was shorter than 150 minutes */ && (source->IsEmuleClient()==false)) ))
#endif
	{
		//AddDebugLogLine(false, _T("-o- rejected known dropped client %s, %s reentering downloadqueue after time: %u min"), source->GetClientVerString(), source->GetUserName(), (::GetTickCount()-source->droptime)/60000);
		return false;
	}

	// filter sources which are known to be temporarily dead/useless
	if ( (!bIgnoreGlobDeadList && theApp.clientlist->m_globDeadSourceList.IsDeadSource(source)) || sender->m_DeadSourceList.IsDeadSource(source)){
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
			/*if (cur_file == sender){
				//srcLock.Unlock();	//zz_fly :: make source add action thread safe :: Enig123
				return false;
			}
			if (source->AddRequestForAnotherFile(sender))
				theApp.emuledlg->transferwnd->GetDownloadList()->AddSource(sender,source,true);
                //if(source->GetDownloadState() != DS_CONNECTED) {
                //    source->SwapToAnotherFile(_T("New A4AF source found. CDownloadQueue::CheckAndAddKnownSource()"), false, false, false, NULL, true, false); // ZZ:DownloadManager
                //}
			//srcLock.Unlock();	//zz_fly :: make source add action thread safe :: Enig123
			return false;*/
			if (cur_file != sender && source->AddRequestForAnotherFile(sender))// X: [CI] - [Code Improvement]
				theApp.emuledlg->transferwnd->GetDownloadList()->AddSource(sender,source,true);
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
	source->droptime=0;
	source->enterqueuetime=0;
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
	//Xman Code Improvement
	bool bRemovedSrcFromPartFile = false;
	//srcLock.Lock();	//zz_fly :: make source add action thread safe :: Enig123
	for (POSITION pos = filelist.GetHeadPosition();pos != NULL;){
		CPartFile* cur_file = filelist.GetNext(pos);
		POSITION pos2 = cur_file->srclist.Find(toremove);
		if (pos2 != NULL){
			cur_file->RemoveDownloadingSource(toremove); //to be sure
			cur_file->srclist.RemoveAt(pos2);

			bRemovedSrcFromPartFile = true;
			if ( bDoStatsUpdate ){
				cur_file->UpdateAvailablePartsCount();
				cur_file->UpdatePartsInfo();
			}
		}
	}
	//srcLock.Unlock();	//zz_fly :: make source add action thread safe :: Enig123

	// remove this source on all files in the downloadqueue who link this source
	// pretty slow but no way arround, maybe using a Map is better, but that's slower on other parts
	POSITION pos3, pos4;
	for(pos3 = toremove->m_OtherRequests_list.GetHeadPosition();(pos4=pos3)!=NULL;)
	{
		CPartFile*file = toremove->m_OtherRequests_list.GetNext(pos3);// X: [CI] - [Code Improvement]			
		POSITION pos5 = file->A4AFsrclist.Find(toremove);
		if(pos5)
		{ 
			file->A4AFsrclist.RemoveAt(pos5);
			theApp.emuledlg->transferwnd->GetDownloadList()->RemoveSource(toremove,file);
			toremove->m_OtherRequests_list.RemoveAt(pos4);
		}
	}
	for(pos3 = toremove->m_OtherNoNeeded_list.GetHeadPosition();(pos4=pos3)!=NULL;)
	{
		CPartFile*file = toremove->m_OtherNoNeeded_list.GetNext(pos3);// X: [CI] - [Code Improvement]		
		POSITION pos5 = file->A4AFsrclist.Find(toremove);
		if(pos5)
		{ 
			file->A4AFsrclist.RemoveAt(pos5);
			theApp.emuledlg->transferwnd->GetDownloadList()->RemoveSource(toremove,file);
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
	RemoveLocalServerRequest(toremove);
	//Xman
	// Maella -Code Improvement-
	POSITION pos = filelist.Find(toremove);
	if (pos != NULL){
		CPartFile* pFile = filelist.GetAt(pos);
		ASSERT(pFile->srclist.IsEmpty());
		pFile->srclist.RemoveAll(); // Security 
		//zz_fly :: free unused memory when a download is cancelled/completed :: Enig123 :: start
		pFile->RemoveAllRequestedBlocks();	//Enig123??
		//zz_fly :: free unused memory when a download is cancelled/completed :: Enig123 :: end
		filelist.RemoveAt(pos);
	}
	// Maella end
	SortByPriority();
	CheckDiskspace();	// SLUGFILLER: checkDiskspace
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
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
		if (thePrefs.GetDebugServerUDPLevel() > 0)
			Debug(_T(">>> Sending %s to server %-21s (%3u of %3u); FileIDs=%u(%u large)\n"), (packet.opcode == OP_GLOBGETSOURCES2) ? _T("OP__GlobGetSources2") : _T("OP__GlobGetSources1"), ipstr(cur_udpserver->GetAddress(), cur_udpserver->GetPort()), m_iSearchedServers + 1, theApp.serverlist->GetServerCount(), nFiles, nIncludedLargeFiles);
#endif

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
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
							if (thePrefs.GetDebugServerUDPLevel() > 0 && thePrefs.GetDebugServerSourcesLevel() > 0)
								Debug(_T("Finished UDP search processing for all servers (%u)\n"), theApp.serverlist->GetServerCount());
#endif
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
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
			if (thePrefs.GetDebugServerUDPLevel() > 0 && thePrefs.GetDebugServerSourcesLevel() > 0)
				Debug(_T(">>> Queued  %s to server %-21s (%3u of %3u); Buff  %u(%u)=%s\n"), bGetSources2Packet ? _T("OP__GlobGetSources2") : _T("OP__GlobGetSources1"), ipstr(cur_udpserver->GetAddress(), cur_udpserver->GetPort()), m_iSearchedServers + 1, theApp.serverlist->GetServerCount(), iFiles, iLargeFiles, DbgGetFileInfo(nextfile->GetFileHash()));
#endif
		}
	}

	ASSERT( dataGlobGetSources.GetLength() == 0 || !bSentPacket );

	if (!bSentPacket && dataGlobGetSources.GetLength() > 0)
		SendGlobGetSourcesUDPPacket(&dataGlobGetSources, bGetSources2Packet, iFiles, iLargeFiles);

	// send max 35 UDP request to one server per interval
	// if we have more than 35 files, we rotate the list and use it as queue
	if (m_cRequestsSentToServer >= MAX_REQUESTS_PER_SERVER)
	{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
		if (thePrefs.GetDebugServerUDPLevel() > 0 && thePrefs.GetDebugServerSourcesLevel() > 0)
			Debug(_T("Rotating file list\n"));
#endif

		// move the last 35 files to the head
		if (filelist.GetCount() >= MAX_REQUESTS_PER_SERVER) {
			for (size_t i = 0; i != MAX_REQUESTS_PER_SERVER; i++)
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
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
			if (thePrefs.GetDebugServerUDPLevel() > 0 && thePrefs.GetDebugServerSourcesLevel() > 0)
				Debug(_T("Finished UDP search processing for all servers (%u)\n"), theApp.serverlist->GetServerCount());
#endif
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

// SLUGFILLER: checkDiskspace
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

void CDownloadQueue::HeapSort(size_t first, size_t last)
{
	size_t r;
	POSITION pos1 = filelist.FindIndex(first);
	for ( r = first; !(r & (size_t)INT_PTR_MIN) && (r<<1) < last; ){
		size_t r2 = (r<<1)+1;
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
	size_t n = filelist.GetCount();
	if (!n)
		return;
	size_t i;
	for ( i = n/2; i--; )
		HeapSort(i, n-1);
	for ( i = n; --i; ){
		SwapParts(filelist.FindIndex(0), filelist.FindIndex(i));
		HeapSort(0, i-1);
	}
}

void CDownloadQueue::CheckDiskspaceTimed()
{
	if ((!lastcheckdiskspacetime) || (::GetTickCount() - lastcheckdiskspacetime) > DISKSPACERECHECKTIME){
		SortByPriority();
		CheckDiskspace();
	}
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

	g_VolumeInfo.ResetFreeDiskSpace();
	uint64 nTotalAvailableSpaceMain = bNotEnoughSpaceLeft ? 0 : g_VolumeInfo.GetFreeDiskSpace(thePrefs.GetTempDir())->FreeSpace;

	// 'bNotEnoughSpaceLeft' - avoid worse case, if we already had 'disk full'
	if (thePrefs.GetMinFreeDiskSpace() == 0)
	{
		for( POSITION pos1 = filelist.GetHeadPosition(); pos1 != NULL; )
		{
			CPartFile* cur_file = filelist.GetNext(pos1);

			uint64 nTotalAvailableSpace = bNotEnoughSpaceLeft ? 0 : 
				((thePrefs.GetTempDirCount()==1)?nTotalAvailableSpaceMain:g_VolumeInfo.GetFreeDiskSpace(cur_file->GetTempPath())->FreeSpace);

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
				((thePrefs.GetTempDirCount()==1)?nTotalAvailableSpaceMain:g_VolumeInfo.GetFreeDiskSpace(cur_file->GetTempPath())->FreeSpace);
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
// SLUGFILLER: checkDiskspace

void CDownloadQueue::GetDownloadSourcesStats(SDownloadStats& results)
{
	memset(&results, 0, sizeof results);
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
	{
		const CPartFile* cur_file = filelist.GetNext(pos);

		results.a[0]  += (UINT)cur_file->GetSourceCount();
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
		results.a[15] += (UINT)cur_file->GetSrcA4AFCount();
		results.a[16] += cur_file->src_stats[0];
		results.a[17] += cur_file->src_stats[1];
		results.a[18] += cur_file->src_stats[2];
		results.a[19] += cur_file->net_stats[0];
		results.a[20] += cur_file->net_stats[1];
		results.a[21] += cur_file->net_stats[2];
		results.a[22] += (UINT)cur_file->m_DeadSourceList.GetDeadSourcesCount();
	}
}

UINT CDownloadQueue::GetTransferingSources()
{
	UINT count = 0;
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
	{
		const CPartFile* cur_file = filelist.GetNext(pos);
		count += cur_file->GetTransferringSrcCount();
	}
	return count;
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

void CDownloadQueue::ResetCatParts(size_t cat)
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

void CDownloadQueue::SetCatPrio(size_t cat, uint8 newprio)
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

    //SortByPriority();
	CheckDiskspaceTimed();
}

// ZZ:DownloadManager -->
void CDownloadQueue::RemoveAutoPrioInCat(size_t cat, uint8 newprio){
	CPartFile* cur_file;
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){// X: [CI] - [Code Improvement]
		cur_file = filelist.GetNext(pos);
        if (cur_file->IsAutoDownPriority() && (cat==0 || cur_file->GetCategory()==cat)) {
			cur_file->SetAutoDownPriority(false);
			cur_file->SetDownPriority(newprio, false);
		}
	}

    //SortByPriority();
	CheckDiskspaceTimed(); // SLUGFILLER: checkDiskspace
}
// <-- ZZ:DownloadManager

void CDownloadQueue::SetCatStatus(size_t cat, int newstatus)
{
	bool reset = false;
    bool resort = false;

	POSITION pos= filelist.GetHeadPosition();
	while (pos != 0)
	{
		CPartFile* cur_file = filelist.GetNext(pos);// X: [CI] - [Code Improvement]
		if (!cur_file)
			continue;

		if (cat == (size_t)-1 || 
			(cat == (size_t)-2 && cur_file->GetCategory()==0) ||
			(cat == 0 && cur_file->CheckShowItemInGivenCat(cat)) || 
			(cat > 0 && cat==cur_file->GetCategory()))
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
		if (reset)
		{
			reset = false;
			pos = filelist.GetHeadPosition();
		}
	}

    if(resort) {
	    SortByPriority();
	    CheckDiskspace(); // SLUGFILLER: checkDiskspace
    }
}

void CDownloadQueue::MoveCat(size_t from, size_t to)
{
	if (from < to)
		--to;

	POSITION pos= filelist.GetHeadPosition();
	while (pos != 0)
	{
		CPartFile* cur_file = filelist.GetAt(pos);
		if (!cur_file)
			continue;

		size_t mycat = cur_file->GetCategory();
		if((from>to && mycat>=to && mycat<=from)// X: [CI] - [Code Improvement]
			|| (from<to && mycat>=from && mycat<=to))
		{
			//if ((from<to && (mycat<from || mycat>to)) || (from>to && (mycat>from || mycat<to)) )	continue; //not affected

			cur_file->SetCategory(mycat == from?to:((from < to)?mycat - 1:mycat + 1));
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
	for(POSITION pos1 = m_localServerReqQueue.GetHeadPosition(), pos2; ( pos2 = pos1 ) != NULL; ){// X: [CI] - [Code Improvement]
		if (m_localServerReqQueue.GetNext(pos1) == pFile){
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
		CSafeMemFile dataTcpFrame(6+(16+4+8));
		CSafeMemFile smPacket(16+4+8);
		const int iMaxFilesPerTcpFrame = 15;
		int iFiles = 0;
		while (!m_localServerReqQueue.IsEmpty() && iFiles < iMaxFilesPerTcpFrame)
		{
			// find the file with the longest waitingtime
			uint32 dwBestWaitTime = 0xFFFFFFFF;
			POSITION posNextRequest = NULL;
			CPartFile* cur_file;
			for(POSITION pos1 = m_localServerReqQueue.GetHeadPosition(), pos2; ( pos2 = pos1 ) != NULL; ){// X: [CI] - [Code Improvement]
				cur_file = m_localServerReqQueue.GetNext(pos1);
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
				smPacket.SetLength(0);
				smPacket.WriteHash16(cur_file->GetFileHash());
				if (!cur_file->IsLargeFile()){
					smPacket.WriteUInt32((uint32)(uint64)cur_file->GetFileSize());
				}
				else{
					smPacket.WriteUInt32(0); // indicates that this is a large file and a uint64 follows
					smPacket.WriteUInt64(cur_file->GetFileSize());
				}

				uint8 byOpcode = 
					(thePrefs.IsClientCryptLayerSupported() && theApp.serverconnect->GetCurrentServer() != NULL && theApp.serverconnect->GetCurrentServer()->SupportsGetSourcesObfuscation())?
						OP_GETSOURCES_OBFU
					:
						OP_GETSOURCES;

				Packet packet(&smPacket, OP_EDONKEYPROT, byOpcode);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
				if (thePrefs.GetDebugServerTCPLevel() > 0)
					Debug(_T(">>> Sending OP__GetSources%s(%2u/%2u); %s\n"), (byOpcode == OP_GETSOURCES) ? _T("") : _T("_OBFU"), iFiles, iMaxFilesPerTcpFrame, DbgGetFileInfo(cur_file->GetFileHash()));
#endif
				dataTcpFrame.Write(packet.GetPacket(), packet.GetRealPacketSize());

				if (thePrefs.GetDebugSourceExchange())
					AddDebugLogLine(false, _T("SXSend: Local server source request; File=\"%s\""), cur_file->GetFileName());
			}
		}

		UINT iSize = (UINT)dataTcpFrame.GetLength();
		if (iSize > 0)
		{
			// create one 'packet' which contains all buffered OP_GETSOURCES eD2K packets to be sent with one TCP frame
			// server credits: 16*iMaxFilesPerTcpFrame+1 = 241
			Packet* packet = new Packet(new char[iSize], iSize/*(UINT)dataTcpFrame.GetLength()*/, true, false);
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
			AddDebugLogLine(false, _T("IPfiltered source IP=%s (%s) received from Kademlia for File: %s"), ipstr(ED2Kip), theApp.ipfilter->GetLastHit(), temp->GetFileName()); //Xman show filename
		return;
	}

	//zz_fly :: skip banned source
	// X-Ray :: Optimizations :: Start
	if (theApp.clientlist->IsBannedClient(ED2Kip)){
		if (thePrefs.GetLogBannedClients())
			AddDebugLogLine(false, _T("Banned source IP=%s received from Kademlia"), ipstr(ED2Kip));
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
			//ctemp->SetServerIP(dwBuddyIP);
			//ctemp->SetServerPort(dwBuddyPort);
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
			break;
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
				_tsplitpath_s(strPartFilePath, NULL, 0, NULL, 0, szNam, _countof(szNam), szExt, _countof(szExt));
				if (thePrefs.GetTempDirCount()==1)
					file.printf(_T("%s%s\t%s\r\n"), szNam, szExt, pPartFile->GetED2kLink());
				else
					file.printf(_T("%s\t%s\r\n"), pPartFile->GetFullName(), pPartFile->GetED2kLink());
			}
		}

		if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !CemuleDlg::IsRunning())){
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

CString CDownloadQueue::GetOptimalTempDir(size_t nCat, EMFileSize nFileSize){
	// shortcut
	if (thePrefs.tempdir.GetCount() == 1)
		return thePrefs.GetTempDir();

	g_VolumeInfo.ResetFreeDiskSpace();

	size_t	nHighestFreeSpaceDir = (size_t)-1;
	uint64 llHighestTotalSpace = 0;
	// first collect the free space on drives
	for (size_t i = 0; i < thePrefs.tempdir.GetCount(); i++) {
		VolumeInfo*	viTemp = g_VolumeInfo.GetFreeDiskSpace(thePrefs.GetTempDir(i));
		if ((nFileSize <= OLD_MAX_EMULE_FILE_SIZE || !viTemp->IsFAT())
			&& (viTemp->FreeSpace > llHighestTotalSpace)){
			llHighestTotalSpace = viTemp->FreeSpace;
			nHighestFreeSpaceDir = i;
		}
	}

	// now get the space we would need to download all files in the current queue
	POSITION pos = filelist.GetHeadPosition();
	while (pos != NULL){
		CPartFile* pCurFile =  filelist.GetNext(pos);

		uint64 llNeededForCompletion = 0;
		switch(pCurFile->GetStatus(false)){
			case PS_READY:
			case PS_EMPTY:
			//case PS_WAITINGFORHASH:
			case PS_INSUFFICIENT:
				llNeededForCompletion = pCurFile->GetRealFileSize();
				if(pCurFile->GetFileSize() <= llNeededForCompletion)
					continue;//llNeededForCompletion = 0;
				llNeededForCompletion = pCurFile->GetFileSize() - llNeededForCompletion;
				break;
		}
		VolumeInfo*viTemp = g_VolumeInfo.GetVolumeInfoByPath(pCurFile->GetTempPath());
		if(viTemp->FreeSpace > llNeededForCompletion)
			viTemp->FreeSpace -= llNeededForCompletion;
		else
			viTemp->FreeSpace = 0;
	}

	llHighestTotalSpace = 0;
	size_t	nHighestTotalSpaceDir = (size_t)-1;
	size_t	nAnyAvailableDir = (size_t)-1;
	// first round (0): on same drive as incomming and enough space for all downloading
	// second round (1): enough space for all downloading
	// third round (2): most actual free space
	for (size_t i = 0; i < thePrefs.tempdir.GetCount(); i++) {
		VolumeInfo*viTemp = g_VolumeInfo.GetVolumeInfoByPath(thePrefs.GetTempDir(i));

		// no condition can be met for a large file on a FAT volume
		if (nFileSize <= OLD_MAX_EMULE_FILE_SIZE || !viTemp->IsFAT()){ // X: [FSFS] - [FileSystemFeaturesSupport]
			if(viTemp->FreeSpace - thePrefs.GetMinFreeDiskSpace() > nFileSize){
				// condition 0
				// needs to be same drive and enough space
				if (g_VolumeInfo.GetVolumeInfoByPath(thePrefs.GetCatPath(nCat))->Name == viTemp->Name)
				{
					//this one is perfect
					return thePrefs.GetTempDir(i);
				}
				// condition 1
				// needs to have enough space for downloading
				if (viTemp->FreeSpace > llHighestTotalSpace){
					llHighestTotalSpace = viTemp->FreeSpace;
					nHighestTotalSpaceDir = i;
				}
			}
			// condition 2
			// first one which has the highest actualy free space
			/*if ( viTemp->Name == viHighestFreeSpace->Name && nHighestFreeSpaceDir == (-1)){
			nHighestFreeSpaceDir = i;
			}*/
			// condition 3
			// any directory which can be used for this file (ak not FAT for large files)
			if ( nAnyAvailableDir == (size_t)-1){
				nAnyAvailableDir = i;
			}
		}
	}
	if (nHighestTotalSpaceDir != (size_t)-1)	 //condtion 0 was apperently too much, take 1
		return thePrefs.GetTempDir(nHighestTotalSpaceDir);
	if (nHighestFreeSpaceDir != (size_t)-1) // condtion 1 could not be met too, take 2
		return thePrefs.GetTempDir(nHighestFreeSpaceDir);
	if (nAnyAvailableDir != (size_t)-1)
		return thePrefs.GetTempDir(nAnyAvailableDir);
	// so was condtion 2 and 3, take 4.. wait there is no 3 - this must be a bug
	ASSERT( false );
	return thePrefs.GetTempDir();
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

// NEO: QS - [QuickStart] -- Xanatos -->
void CDownloadQueue::ProcessQuickStart()
{
	if (m_bQuickStartDone)
		return;

	if(thePrefs.m_uQuickStart || thePrefs.m_bOnQuickStart)
	{
		if(theApp.IsConnected())
		{
			if(!thePrefs.m_bOnQuickStart)
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

	DWORD QuickStartTime = min((CountFiles * SEC2MS(thePrefs.m_iQuickStartTimePerFile)), MIN2MS(thePrefs.m_iQuickStartTime));

	if (QuickStartTime > 0){
		m_bQuickStartDone = false;
		thePrefs.m_bOnQuickStart = true;
		m_dwQuickStartEndTime = ::GetTickCount() + QuickStartTime;
		Log(LOG_STATUSBAR, GetResString(IDS_X_QUICK_START_BEGIN), MS2MIN((float)QuickStartTime));
	}else
		m_bQuickStartDone = true;
}

void CDownloadQueue::StopQuickStart()
{
	thePrefs.m_bOnQuickStart = false;
	m_bQuickStartDone = true;
	Log(LOG_STATUSBAR, GetResString(IDS_X_QUICK_START_END));
}
// NEO: QS END <-- Xanatos --
#ifdef _DEBUG
void CDownloadQueue::PrintStatistic()
{
	size_t Savedsources=0;
	size_t Gaplist=0;
	size_t Requestedblocklist=0;
	size_t SrcpartFrequency=0;
	size_t BufferedData=0;
	size_t A4AFsrclist=0;
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

//Xman see all sources
CString CDownloadQueue::GetFilesCount(size_t inCategory)
{
	size_t iCount = 0;
	size_t countsources=0; 
	size_t countreadyfiles=0;

	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){// X: [CI] - [Code Improvement]
		CPartFile* cur_file = filelist.GetNext(pos);
		if (cur_file->CheckShowItemInGivenCat(inCategory)){
			++iCount;
			countsources += cur_file->GetSourceCount();
			EPartFileStatus status=cur_file->GetStatus();
			if(status!=PS_COMPLETE && status!=PS_PAUSED/* && status!=PS_HASHING && status!=PS_WAITINGFORHASH*/ && status!=PS_COMPLETING && status!=PS_INSUFFICIENT && status!=PS_ERROR)
				countreadyfiles++;
		}
	}
	CString strBuffer;
	strBuffer.Format(_T("%s (%u/%u)/%s (%u)"), GetResString(IDS_TW_DOWNLOADS), countreadyfiles, iCount, GetResString(IDS_DL_SOURCES), countsources);
	return strBuffer;
}
//Xman end see all sources

void CDownloadQueue::UpdateCompressible()// X: [DCE] - [DontCompressExt]
{
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
		filelist.GetNext(pos)->UpdateCompressible();
}
