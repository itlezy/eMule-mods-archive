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
#include "emule.h"
#include "UploadQueue.h"
#include "Packets.h"
#include "KnownFile.h"
#include "ListenSocket.h"
#include "Exceptions.h"
#include "Scheduler.h"
#include "PerfLog.h"
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
#include "Neo/BC/UploadBandwidthThrottler.h"
#else
#include "UploadBandwidthThrottler.h"
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
#include "ClientList.h"
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
#include "Neo/BC/BandwidthControl.h"
#else
#include "LastCommonRouteFinder.h"
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
#include "DownloadQueue.h"
#include "FriendList.h"
#include "Statistics.h"
#include "MMServer.h"
#include "OtherFunctions.h"
#include "UpDownClient.h"
#include "SharedFileList.h"
#include "KnownFileList.h"
#include "Sockets.h"
#include "ClientCredits.h"
#include "Server.h"
#include "ServerList.h"
#include "WebServer.h"
#include "emuledlg.h"
#include "ServerWnd.h"
#include "TransferWnd.h"
#include "SearchDlg.h"
#include "StatisticsDlg.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "Kademlia/Kademlia/Prefs.h"
#include "Log.h"
#include "collection.h"
#include "Neo/NeoPreferences.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
#include "Neo/VooDoo/voodoo.h"
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
#include "Neo/NeoPreferences.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --
#ifdef NATTUNNELING // NEO: UTCP - [UserModeTCP] -- Xanatos -->
#include "Neo/NatT/NatManager.h"
#endif //NATTUNNELING // NEO: UTCP END <-- Xanatos --
#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase] -- Xanatos -->
#include "Neo/Sources/SourceList.h"
#endif // NEO_CD // NEO: NCD END <-- Xanatos --

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


static uint32 counter, sec, statsave;
static UINT _uSaveStatistics = 0;
// -khaos--+++> Added iupdateconnstats...
static uint32 igraph, istats, iupdateconnstats;
// <-----khaos-


CUploadQueue::CUploadQueue()
{
	VERIFY( (h_timer = SetTimer(0,0,100,UploadTimer)) != NULL );
	if (thePrefs.GetVerbose() && !h_timer)
		AddDebugLogLine(true,_T("Failed to create 'upload queue' timer - %s"),GetErrorMessage(GetLastError()));
	datarate = 0;
	counter=0;
	successfullupcount = 0;
	failedupcount = 0;
	totaluploadtime = 0;
	m_nLastStartUpload = 0;
	statsave=0;
	// -khaos--+++>
	iupdateconnstats=0;
	// <-----khaos-
	m_dwRemovedClientByScore = ::GetTickCount();
#if !defined(NEO_UBT) // NEO: NUBT - [NeoUploadBandwidthThrottler] <-- Xanatos --
    m_iHighestNumberOfFullyActivatedSlotsSinceLastCall = 0;
    m_MaxActiveClients = 0
    m_MaxActiveClientsShortTime = 0;
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --

	// NEO: ASM - [AccurateSpeedMeasure] -- Xanatos -->
	m_datarateMS = 0;
	// NEO: ASM END <-- Xanatos --
    //m_lastCalculatedDataRateTick = 0;
    //m_avarage_dr_sum = 0;

#if !defined(NEO_UBT) // NEO: NUBT - [NeoUploadBandwidthThrottler] <-- Xanatos --
    m_dwLastResortedUploadSlots = 0;
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --

#ifdef NEO_UBT // NEO: NUSM - [NeoUploadSlotManagement] -- Xanatos -->
	lastUploadSlotCheck = 0;
	activeSlots = 0;
	waituntilnextlook = 0;
	dataratestocheck = 10;
 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
	reservedDatarate = 0;
	reservedSlots = 0;
 #endif // BW_MOD // NEO: BM END
#endif // NEO_UBT // NEO: NUSM END <-- Xanatos --
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	lanSlots = 0;
#endif //LANCAST // NEO: NLC END <-- Xanatos --

	releaseSlots = 0; // NEO: SRS - [SmartReleaseSharing] <-- Xanatos --
	friendSlots = 0; // NEO: NMFS - [NiceMultiFriendSlots] <-- Xanatos --

	m_uLastKadFirewallRecheck = ::GetTickCount(); // NEO: RKF - [RecheckKadFirewalled] <-- Xanatos --
}

/**
 * Find the highest ranking client in the waiting queue, and return it.
 *
 * Low id client are ranked as lowest possible, unless they are currently connected.
 * A low id client that is not connected, but would have been ranked highest if it
 * had been connected, gets a flag set. This flag means that the client should be
 * allowed to get an upload slot immediately once it connects.
 *
 * @return address of the highest ranking client.
 */
CUpDownClient* CUploadQueue::FindBestClientInQueue()
{
	POSITION toadd = 0;
	POSITION toaddlow = 0;
	uint32	bestscore = 0;
	uint32  bestlowscore = 0;
    CUpDownClient* newclient = NULL;
    CUpDownClient* lowclient = NULL;
	bool bHavePowerShare = false; // NEO: SRS - [SmartReleaseSharing] <-- Xanatos --
	bool bHaveFriendSlot = false; // NEO: NMFS - [NiceMultiFriendSlots] <-- Xanatos --
	bool bRandomQueue = NeoPrefs.UseRandomQueue() == TRUE; // NEO: RQ - [RandomQueue] <-- Xanatos --

	POSITION pos1, pos2;
	for (pos1 = waitinglist.GetHeadPosition();( pos2 = pos1 ) != NULL;)
	{
		waitinglist.GetNext(pos1);
		CUpDownClient* cur_client =	waitinglist.GetAt(pos2);
		//While we are going through this list.. Lets check if a client appears to have left the network..
		ASSERT ( cur_client->GetLastUpRequest() );
		if ((::GetTickCount() - cur_client->GetLastUpRequest() > MAX_PURGEQUEUETIME) || !theApp.sharedfiles->GetFileByID(cur_client->GetUploadFileID()) )
		{
			//This client has either not been seen in a long time, or we no longer share the file he wanted anymore..
			cur_client->ClearWaitStartTime();
			cur_client->m_fUpIsProblematic = 0; // NEO: UPC - [UploadingProblemClient] <-- Xanatos --
			RemoveFromWaitingQueue(pos2,true);
			continue;
		}
        else
        {
			// NEO: SRS - [SmartReleaseSharing] -- Xanatos -->
			if(!bHavePowerShare && cur_client->GetReleaseSlot(true)){ // is this the first release candidate
				bHavePowerShare = true; // we have one
				bestscore = 0; // than his score must be the best, delete the old
				// NEO: RQ - [RandomQueue]
				if(NeoPrefs.UseRandomQueue() != FALSE)
					bRandomQueue =  true;
				// NEO: RQ END
			}else if(bHavePowerShare && !cur_client->GetReleaseSlot(true)) // we have a release candidate, but this client isn't one so spik him
				continue;
			// NEO: SRS END <-- Xanatos --

		    // finished clearing
		    //uint32 cur_score = cur_client->GetScore(false);
			uint32 cur_score = cur_client->GetScore(false,false,false,bRandomQueue); // NEO: RQ - [RandomQueue] <-- Xanatos --

			// NEO: NMFS - [NiceMultiFriendSlots] -- Xanatos -->
			if(!bHaveFriendSlot && cur_client->GetFriendSlot(true)){ // is this the first friend slot candidate
				bHaveFriendSlot = true; // we have one
				bestscore = 0; // than his score must be the best, delete the old
			}else if(bHaveFriendSlot && !cur_client->GetFriendSlot(true)) // we have a friend slot candidate, but this client isn't one so spik him
				continue;
			// NEO: NMFS END <-- Xanatos --

		    if ( cur_score > bestscore)
		    {
                // cur_client is more worthy than current best client that is ready to go (connected).
                //if(!cur_client->HasLowID() || (cur_client->socket && cur_client->socket->IsConnected())) {
				if((!cur_client->HasLowID() 
					&& cur_client->m_fUpIsProblematic == 0) // NEO: UPC - [UploadingProblemClient] <-- Xanatos --
#ifdef NATTUNNELING // NEO: LUC - [LowIDUploadCallBack] -- Xanatos -->
					// if we sue LowIDUploadCallBack we handle here low id's like high ID's 
					//    the TryToConnect function will take care of the rest
					//		When the feature is only half enabled (default) we use the upload callback only for NAT-T cleints
                 || NeoPrefs.UseLowIDUploadCallBack() == TRUE || (NeoPrefs.UseLowIDUploadCallBack() && theApp.IsFirewalled())
#endif //NATTUNNELING // NEO: LUC END <-- Xanatos --
					|| (cur_client->socket && cur_client->socket->IsConnected())) {
                    // this client is a HighID or a lowID client that is ready to go (connected)
                    // and it is more worthy
			        bestscore = cur_score;
			        toadd = pos2;
                    newclient = waitinglist.GetAt(toadd);
                } 
				//else if(!cur_client->m_bAddNextConnect) 
				else if(cur_client->GetUploadState() != US_PENDING)  // NEO: MOD - [NewUploadState] <-- Xanatos --
				{
                    // this client is a lowID client that is not ready to go (not connected)
    
                    // now that we know this client is not ready to go, compare it to the best not ready client
                    // the best not ready client may be better than the best ready client, so we need to check
                    // against that client
			        if (cur_score > bestlowscore)
			        {
                        // it is more worthy, keep it
				        bestlowscore = cur_score;
				        toaddlow = pos2;
                        lowclient = waitinglist.GetAt(toaddlow);
			        }
                }
            } 
		}
	}
	
	if (bestlowscore > bestscore && lowclient)
		//lowclient->m_bAddNextConnect = true;
		lowclient->SetUploadState(US_PENDING); // NEO: MOD - [NewUploadState] <-- Xanatos --

    if (!toadd)
		return NULL;
    else
	    return waitinglist.GetAt(toadd);
}

void CUploadQueue::InsertInUploadingList(CUpDownClient* newclient) 
{
	//Lets make sure any client that is added to the list has this flag reset!
	//newclient->m_bAddNextConnect = false; // NEO: MOD - [NewUploadState] <-- Xanatos --
    // Add it last
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
 #ifdef LANCAST // NEO: NLC - [NeoLanCast]
	bool toLan = newclient->IsLanClient();
	if(toLan)
		lanSlots++;
 #endif //LANCAST // NEO: NLC END

	if(CEMSocket* socket = newclient->GetFileUploadSocket()) {
 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
		// NEO: NMFS - [NiceMultiFriendSlots]
		if(NeoPrefs.IsSeparateFriendBandwidth() && newclient->GetFriendSlot(true)){
			socket->m_eSlotType = ST_FRIEND; 
			friendSlots ++;
		}else
		// NEO: NMFS EBD
		// NEO: SRS - [SmartReleaseSharing]
		if(NeoPrefs.IsSeparateReleaseBandwidth() && newclient->GetReleaseSlot(true)){
			socket->m_eSlotType = ST_RELEASE; 
			releaseSlots ++;
		}else 
		// NEO: SRS END
			socket->m_eSlotType = ST_NORMAL; 
 #endif // BW_MOD // NEO: BM END
		theApp.uploadBandwidthThrottler->AddToStandardList(socket,NeoPrefs.IsMinimizeOpenedSlots() || newclient->GetFriendSlot()); // If we minimize the slot number than we add only full slots
	}

	uploadinglist.AddTail(newclient);
#else
    theApp.uploadBandwidthThrottler->AddToStandardList(uploadinglist.GetCount(), newclient->GetFileUploadSocket());
	uploadinglist.AddTail(newclient);
    newclient->SetSlotNumber(uploadinglist.GetCount());
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
}

bool CUploadQueue::AddUpNextClient(LPCTSTR pszReason, CUpDownClient* directadd){
	CUpDownClient* newclient = NULL;
	// select next client or use given client
	if (!directadd)
	{
        newclient = FindBestClientInQueue();

        if(newclient)
		{
		    RemoveFromWaitingQueue(newclient, true);
		    theApp.emuledlg->transferwnd->ShowQueueCount(waitinglist.GetCount());
        }
	}
	else 
		newclient = directadd;

    if(newclient == NULL) 
        return false;

	if (!thePrefs.TransferFullChunks())
		UpdateMaxClientScore(); // refresh score caching, now that the highest score is removed

	if (IsDownloading(newclient))
		return false;

    if(pszReason && thePrefs.GetLogUlDlEvents())
        AddDebugLogLine(false, _T("Adding client to upload list: %s Client: %s"), pszReason, newclient->DbgGetClientInfo());

	if (newclient->HasCollectionUploadSlot() && directadd == NULL){
		ASSERT( false );
		newclient->SetCollectionUploadSlot(false);
	}

	// tell the client that we are now ready to upload
	//if (!newclient->socket || !newclient->socket->IsConnected())
	if (!newclient->socket || !newclient->socket->IsConnected() || !newclient->CheckHandshakeFinished()) // NEO: FCC - [FixConnectionCollision] <-- Xanatos --
	{
		newclient->SetUploadState(US_CONNECTING);
		if (!newclient->TryToConnect(true))
			return false;
#ifdef NATTUNNELING // NEO: LUC - [LowIDUploadCallBack] -- Xanatos -->
		if (!newclient->socket)
		{
			//newclient->SetUploadState(US_ONUPLOADQUEUE);
			theApp.uploadqueue->waitinglist.AddTail(newclient);
			theApp.emuledlg->transferwnd->queuelistctrl.AddClient(newclient,false);
			theApp.emuledlg->transferwnd->ShowQueueCount(waitinglist.GetCount());
			//newclient->m_bAddNextConnect = true; // in case the cleint connects us
			newclient->SetUploadState(US_PENDING); // NEO: MOD - [NewUploadState]
			// aside of this wi will retry a kad callback in a secund
			return false;
		}
#endif //NATTUNNELING // NEO: LUC END <-- Xanatos --
	}
	else
	{
		newclient->SendAcceptUpload(); // NEO: MOD - [SendAcceptUpload] <-- Xanatos --
		/*if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__AcceptUploadReq", newclient);
		Packet* packet = new Packet(OP_ACCEPTUPLOADREQ,0);
		theStats.AddUpDataOverheadFileRequest(packet->size);
		newclient->socket->SendPacket(packet,true);*/
		newclient->SetUploadState(US_UPLOADING);
	}
	newclient->SetUpStartTime();
	newclient->ResetSessionUp();

	newclient->m_fUpIsProblematic = 0; // NEO: UPC - [UploadingProblemClient] <-- Xanatos --

    InsertInUploadingList(newclient);

    m_nLastStartUpload = ::GetTickCount();
	
	// statistic
	CKnownFile* reqfile = theApp.sharedfiles->GetFileByID((uchar*)newclient->GetUploadFileID());
	if (reqfile)
		reqfile->statistic.AddAccepted();
		
	theApp.emuledlg->transferwnd->uploadlistctrl.AddClient(newclient);

	return true;
}

#if !defined(NEO_UBT) // NEO: NUBT - [NeoUploadBandwidthThrottler] <-- Xanatos --
void CUploadQueue::UpdateActiveClientsInfo(DWORD curTick) {
    // Save number of active clients for statistics
    uint32 tempHighest = theApp.uploadBandwidthThrottler->GetHighestNumberOfFullyActivatedSlotsSinceLastCallAndReset();

    if(thePrefs.GetLogUlDlEvents() && theApp.uploadBandwidthThrottler->GetStandardListSize() > (uint32)uploadinglist.GetSize()) {
        // debug info, will remove this when I'm done.
        //AddDebugLogLine(false, _T("UploadQueue: Error! Throttler has more slots than UploadQueue! Throttler: %i UploadQueue: %i Tick: %i"), theApp.uploadBandwidthThrottler->GetStandardListSize(), uploadinglist.GetSize(), ::GetTickCount());

		if(tempHighest > (uint32)uploadinglist.GetSize()+1) {
        	tempHighest = uploadinglist.GetSize()+1;
		}
    }

    m_iHighestNumberOfFullyActivatedSlotsSinceLastCall = tempHighest;

    // save some data about number of fully active clients
    uint32 tempMaxRemoved = 0;
    while(!activeClients_tick_list.IsEmpty() && !activeClients_list.IsEmpty() && curTick-activeClients_tick_list.GetHead() > 20*1000) {
        activeClients_tick_list.RemoveHead();
	    uint32 removed = activeClients_list.RemoveHead();

        if(removed > tempMaxRemoved) {
            tempMaxRemoved = removed;
        }
    }

	activeClients_list.AddTail(m_iHighestNumberOfFullyActivatedSlotsSinceLastCall);
    activeClients_tick_list.AddTail(curTick);

    if(activeClients_tick_list.GetSize() > 1) {
        uint32 tempMaxActiveClients = m_iHighestNumberOfFullyActivatedSlotsSinceLastCall;
        uint32 tempMaxActiveClientsShortTime = m_iHighestNumberOfFullyActivatedSlotsSinceLastCall;
        POSITION activeClientsTickPos = activeClients_tick_list.GetTailPosition();
        POSITION activeClientsListPos = activeClients_list.GetTailPosition();
        while(activeClientsListPos != NULL && (tempMaxRemoved > tempMaxActiveClients && tempMaxRemoved >= m_MaxActiveClients || curTick - activeClients_tick_list.GetAt(activeClientsTickPos) < 10 * 1000)) {
            DWORD activeClientsTickSnapshot = activeClients_tick_list.GetAt(activeClientsTickPos);
            uint32 activeClientsSnapshot = activeClients_list.GetAt(activeClientsListPos);

            if(activeClientsSnapshot > tempMaxActiveClients) {
                tempMaxActiveClients = activeClientsSnapshot;
            }

            if(activeClientsSnapshot > tempMaxActiveClientsShortTime && curTick - activeClientsTickSnapshot < 10 * 1000) {
                tempMaxActiveClientsShortTime = activeClientsSnapshot;
            }

            activeClients_tick_list.GetPrev(activeClientsTickPos);
            activeClients_list.GetPrev(activeClientsListPos);
        }

        if(tempMaxRemoved >= m_MaxActiveClients || tempMaxActiveClients > m_MaxActiveClients) {
            m_MaxActiveClients = tempMaxActiveClients;
        }

        m_MaxActiveClientsShortTime = tempMaxActiveClientsShortTime;
    } else {
        m_MaxActiveClients = m_iHighestNumberOfFullyActivatedSlotsSinceLastCall;
        m_MaxActiveClientsShortTime = m_iHighestNumberOfFullyActivatedSlotsSinceLastCall;
    }
}
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --

/**
 * Maintenance method for the uploading slots. It adds and removes clients to the
 * uploading list. It also makes sure that all the uploading slots' Sockets always have
 * enough packets in their queues, etc.
 *
 * This method is called approximately once every 100 milliseconds.
 */
void CUploadQueue::Process() {

    //DWORD curTick = ::GetTickCount(); // NEO: ASM - [AccurateSpeedMeasure] <-- Xanatos --

#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
	//m_MaxActiveClientsShortTime = theApp.uploadBandwidthThrottler->GetNumberOfFullyActivatedSlots();
	activeSlots = theApp.uploadBandwidthThrottler->GetNumberOfFullyActivatedSlots(); // NEO: NUSM - [NeoUploadSlotManagement]
#else
    UpdateActiveClientsInfo(curTick);
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --

	if (ForceNewClient()){
        // There's not enough open uploads. Open another one.
        AddUpNextClient(_T("Not enough open upload slots for current ul speed"));
	}

#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	uint16 lanSlots_ = 0;
#endif //LANCAST // NEO: NLC END <-- Xanatos --

	uint16 releaseSlots_ = 0; // NEO: SRS - [SmartReleaseSharing] <-- Xanatos --
	uint16 friendSlots_ = 0; // NEO: NMFS - [NiceMultiFriendSlots] <-- Xanatos --

    // The loop that feeds the upload slots with data.
	POSITION pos = uploadinglist.GetHeadPosition();
	while(pos != NULL){
        // Get the client. Note! Also updates pos as a side effect.
		CUpDownClient* cur_client = uploadinglist.GetNext(pos);
		if (thePrefs.m_iDbgHeap >= 2)
			ASSERT_VALID(cur_client);

#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
		if(cur_client->IsLanClient())
			lanSlots_++;
#endif //LANCAST // NEO: NLC END <-- Xanatos --

		// NEO: SRS - [SmartReleaseSharing] -- Xanatos -->
#ifdef BW_MOD// NEO: BM - [BandwidthModeration]
		if(NeoPrefs.IsSeparateReleaseBandwidth()){
			if(cur_client->socket && cur_client->socket->m_eSlotType == ST_RELEASE)
				releaseSlots_++; 
		}
		else
#endif // BW_MOD // NEO: BM END
		if(cur_client->GetReleaseSlot())
			releaseSlots_++; 
		// NEO: SRS END <-- Xanatos --

		// NEO: NMFS - [NiceMultiFriendSlots] -- Xanatos -->
#ifdef BW_MOD// NEO: BM - [BandwidthModeration]
		if(NeoPrefs.IsSeparateFriendBandwidth()){
			if(cur_client->socket && cur_client->socket->m_eSlotType == ST_FRIEND)
				friendSlots_++; 
		}
		else
#endif // BW_MOD // NEO: BM END
		if(cur_client->GetFriendSlot())
			friendSlots_++;
		// NEO: NMFS END <-- Xanatos --

		//It seems chatting or friend slots can get stuck at times in upload.. This needs looked into..
		if (!cur_client->socket)
		{
			RemoveFromUploadQueue(cur_client, _T("Uploading to client without socket? (CUploadQueue::Process)"));
			if(cur_client->Disconnected(_T("CUploadQueue::Process"))){
				delete cur_client;
			}
		} else {
            cur_client->SendBlockData();
        }
	}

#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	lanSlots = lanSlots_;
#endif //LANCAST // NEO: NLC END <-- Xanatos --

	releaseSlots = releaseSlots_; // NEO: SRS - [SmartReleaseSharing] <-- Xanatos --
	friendSlots = friendSlots_; // NEO: NMFS - [NiceMultiFriendSlots] <-- Xanatos --

	// NEO: L2HAC - [LowID2HighIDAutoCallback] -- Xanatos -->
	if (NeoPrefs.UseLowID2HighIDAutoCallback() && theApp.IsFirewalled())
	{
		for (POSITION pos = waitinglist.GetHeadPosition();pos != 0;waitinglist.GetNext(pos)){
			CUpDownClient* cur_client = waitinglist.GetAt(pos);
			if (cur_client->SupportsL2HAC() && !cur_client->HasLowID() && cur_client->CanDoL2HAC())
			{
				if (theApp.listensocket->TooManySockets() && !(cur_client->socket && cur_client->socket->IsConnected()) ){
					cur_client->SetNextL2HAC((uint32)ROUND(((float)rand()/RAND_MAX)*300000));
				}else{
					cur_client->TryToConnect();
					cur_client->SetNextL2HAC(cur_client->GetUploadState() == US_NONEEDEDPARTS ? FILEREASKTIME*2 : FILEREASKTIME);
				}
			}
		}
	}
	// NEO: L2HAC END <-- Xanatos --

	// NEO: ASM - [AccurateSpeedMeasure] -- Xanatos --
    /*// Save used bandwidth for speed calculations
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
	uint64 sentBytes = theApp.uploadBandwidthThrottler->GetSentBytes();
#else
	uint64 sentBytes = theApp.uploadBandwidthThrottler->GetNumberOfSentBytesSinceLastCallAndReset();
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
	avarage_dr_list.AddTail(sentBytes);
    m_avarage_dr_sum += sentBytes;

#if !defined(NEO_UBT) // NEO: NUBT - [NeoUploadBandwidthThrottler] <-- Xanatos --
    (void)theApp.uploadBandwidthThrottler->GetNumberOfSentBytesOverheadSinceLastCallAndReset();
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --

    avarage_friend_dr_list.AddTail(theStats.sessionSentBytesToFriend);

    // Save time beetween each speed snapshot
    avarage_tick_list.AddTail(curTick);

    // don't save more than 30 secs of data
    while(avarage_tick_list.GetCount() > 3 && !avarage_friend_dr_list.IsEmpty() && ::GetTickCount()-avarage_tick_list.GetHead() > 30*1000) {
   	    m_avarage_dr_sum -= avarage_dr_list.RemoveHead();
        avarage_friend_dr_list.RemoveHead();
        avarage_tick_list.RemoveHead();
    }*/
};

// NEO: ASM - [AccurateSpeedMeasure] -- Xanatos -->
void CUploadQueue::CalculateUploadRate()
{
	while(avarage_dr_list.GetCount()>0 && (GetTickCount() - avarage_dr_list.GetHead().timestamp > 10*1000) )
		m_datarateMS-=avarage_dr_list.RemoveHead().datalen;
	
	if (avarage_dr_list.GetCount()>1){
		datarate = (UINT)(m_datarateMS / avarage_dr_list.GetCount());
	} else {
		datarate = 0;
	}

	uint32 datarateX=0;	

#ifdef NEO_UBT // NEO: NUSM - [NeoUploadSlotManagement]
 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
	uint32 reservedDatarate_ = 0;
	uint32 reservedSlots_ = 0;
 #endif // BW_MOD // NEO: BM END

 #ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
	uint16 MaxUpload = (uint16)theApp.bandwidthControl->GetMaxUpload();
 #else
	uint16 MaxUpload;
    if (thePrefs.IsDynUpEnabled())
        MaxUpload = (uint16)(theApp.lastCommonRouteFinder->GetUpload()/1024);
    else
		MaxUpload = thePrefs.GetMaxUpload();
 #endif // NEO_BC // NEO: NBC END

	uint32 TrickleTolerance = 0; 
	uint32 BlockedTolerance = 0; 
	if(waituntilnextlook == 0){
		if(NeoPrefs.IsCumulateBandwidth())
			TrickleTolerance = (uint32)min((NeoPrefs.GetUploadPerSlots()*1024/2), max(5.0f*1024,(NeoPrefs.GetIncreaseTrickleSpeed()*1024*4)));
		BlockedTolerance = (uint32)(NeoPrefs.GetUploadPerSlots()*1024*2/3);
	}

	uint32 cur_datarate;
	uint32 datarate_total = 0;
	uint32 datarate_count = 0;
	POSITION pos = uploadinglist.GetHeadPosition();
	while(pos != NULL){
		CUpDownClient* cur_client = uploadinglist.GetNext(pos);
		cur_datarate = cur_client->CalculateUploadRate();
 #ifdef LANCAST // NEO: NLC - [NeoLanCast]
		if(cur_client->IsLanClient())
			continue; // dont count lan speed, or do anythink else speed related with lan cleints here
 #endif // NEO_BC // NEO: NBC END
		datarateX += cur_datarate;

		CEMSocket* cur_socket = cur_client->GetFileUploadSocket();
		//check if one slot is over tolerance and tell the throttler
		//if(waituntilnextlook == 0 && cur_client->GetUpStartTimeDelay() > SEC2MS(10) && cur_socket && cur_socket->m_eSlotState != SS_NONE)
		if(waituntilnextlook == 0 && cur_client->GetUpStartTimeDelay() > SEC(10) && cur_socket && cur_socket->m_eSlotState != SS_NONE) // NEO: CUT - [CleanUploadTiming]
		{
			if(NeoPrefs.IsCumulateBandwidth()){
				if(cur_socket->m_eSlotState != SS_FULL && cur_datarate > TrickleTolerance)
					theApp.uploadBandwidthThrottler->SetTrickleToFull(cur_socket);
				else if(cur_socket->m_eSlotState == SS_FULL && cur_datarate < TrickleTolerance )
					theApp.uploadBandwidthThrottler->SetFullToTrickle(cur_socket);
 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
			}else if(NeoPrefs.IsCheckSlotDatarate() && cur_socket->m_eSlotType == ST_NORMAL){
 #else
			}else if(NeoPrefs.IsCheckSlotDatarate()){
 #endif // BW_MOD // NEO: BM END
				uint32 chk_datarate = cur_client->CheckUploadRate(dataratestocheck);
				if(cur_socket->m_eSlotState == SS_FULL){
					datarate_total += chk_datarate;
					datarate_count ++;
				}else if (cur_socket->m_eSlotState == SS_BLOCKED && chk_datarate > BlockedTolerance){
					cur_socket->ResetProcessRate(0.25f); // reset socket ratio
					theApp.uploadBandwidthThrottler->SetTrickleToFull(cur_socket);
				}
			}
		}

		uint32 SlotSpeed = (uint32)(NeoPrefs.GetUploadPerSlots()*1024);
 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
		if(cur_socket->m_eSlotType == ST_RELEASE && NeoPrefs.IsSeparateReleaseBandwidth()){
			if(NeoPrefs.IsSeparateReleaseBandwidth() == TRUE)
				SlotSpeed = (uint32)(NeoPrefs.GetReleaseSlotSpeed()*1024);
			else{
				if(releaseSlots)
					SlotSpeed = (uint32)(((MaxUpload * 1024) * NeoPrefs.GetReleaseBandwidthPercentage() / 100) / releaseSlots);
				else // aha first time processed no other slots on list
					SlotSpeed = (uint32) ((MaxUpload * 1024) * NeoPrefs.GetReleaseBandwidthPercentage() / 100);
				SlotSpeed = max((uint32)(NeoPrefs.GetReleaseSlotSpeed()*1024), SlotSpeed);
			}
		}
		else if(cur_socket->m_eSlotType == ST_FRIEND && NeoPrefs.IsSeparateFriendBandwidth()){
			if(NeoPrefs.IsSeparateFriendBandwidth() == TRUE)
				SlotSpeed = (uint32)(NeoPrefs.GetFriendSlotSpeed()*1024);
			else{
				if(friendSlots)
					SlotSpeed = (uint32)(((MaxUpload * 1024) * NeoPrefs.GetFriendBandwidthPercentage() / 100) / friendSlots);
				else
					SlotSpeed = (uint32) ((MaxUpload * 1024) * NeoPrefs.GetFriendBandwidthPercentage() / 100);
				SlotSpeed = max((uint32)(NeoPrefs.GetFriendSlotSpeed()*1024), SlotSpeed);
			}
		}

		if(cur_socket->m_eSlotType != ST_NORMAL && cur_socket->m_eSlotState == SS_FULL){
			cur_socket->m_ModerateSpeed = (float)SlotSpeed/1024.0f;
			// Note: never reserve more that actualy used or you will end up with waisted bandwidth !!!
			reservedDatarate_ += min(cur_datarate, SlotSpeed);
			reservedSlots_ ++;
		}
 #endif // BW_MOD // NEO: BM END

 #if !defined DONT_USE_SOCKET_BUFFERING // NEO: DSB - [DynamicSocketBuffer]
		if(NeoPrefs.IsSetUploadBuffer() == 2){ // dynamic selection
			uint32 bufferlimit = max(SlotSpeed,cur_datarate); // we must have buffer for at least one secund
			cur_socket->SetSocketBufferLimit(bufferlimit);
		}
 #endif // NEO: DSB
	}

	if(waituntilnextlook == 0 && NeoPrefs.IsCheckSlotDatarate()){
		uint32 NormalTolerance = (uint32)(NeoPrefs.GetUploadPerSlots() * 1024 * ((GetUploadQueueLength() > (uint16)ceil(MaxUpload/NeoPrefs.GetUploadPerSlots()) + 1 ) ? 1.3f  : 1.2f)); //we are 2 slots over MinSlots 30% else 20%

		if(datarate_count && (datarate_total/datarate_count > NormalTolerance)){
			theApp.uploadBandwidthThrottler->SetNextTrickleToFull(); // add next trickle slot
			waituntilnextlook = 5; //5 seconds until we redo this test
			dataratestocheck = -1; //Xman don't check the first, but the next three
		}
	}

	if(waituntilnextlook>0)
		waituntilnextlook--;
	if(dataratestocheck < 15) 
		dataratestocheck++;

 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
	reservedDatarate =  reservedDatarate_;
	reservedSlots = reservedSlots_;
 #endif // BW_MOD // NEO: BM END

#else
	uint32 cur_datarate;
	POSITION pos = uploadinglist.GetHeadPosition();
	while(pos != NULL){
		CUpDownClient* cur_client = uploadinglist.GetNext(pos);
		cur_datarate = cur_client->CalculateUploadRate();
 #ifdef LANCAST // NEO: NLC - [NeoLanCast]
		if(cur_client->IsLanClient())
			continue; // dont count lan speed, or do anythink else speed related with lan cleints here
 #endif // NEO_BC // NEO: NBC END
		datarateX += cur_datarate;

	}
#endif // NEO_UBT // NEO: NUSM END

	TransferredData newitem = {datarateX, ::GetTickCount()};
	avarage_dr_list.AddTail(newitem);
	m_datarateMS+=datarateX;
}
// NEO: ASM END <-- Xanatos --

#ifdef NEO_UBT // NEO: NUSM - [NeoUploadSlotManagement] -- Xanatos -->
bool CUploadQueue::AcceptNewClient(bool addOnNextConnect)
{
	int curUpSlots = GetUploadQueueLength();
	if (curUpSlots < NeoPrefs.GetMinUploadSlots())
		return true;
	else if(curUpSlots >= NeoPrefs.GetMaxUploadSlots())
		return false;

	UINT curActiveSlots = activeSlots;

	//Xman Xtreme Upload
 #ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
	float MaxUpload = theApp.bandwidthControl->GetMaxUpload();
 #else
	float MaxUpload;
	if (thePrefs.IsDynUpEnabled())
		MaxUpload = theApp.lastCommonRouteFinder->GetUpload()/1024.0f;
	else
		MaxUpload = thePrefs.GetMaxUpload();
 #endif // NEO_BC // NEO: NBC END <-- Xanatos --

	if(MaxUpload == UNLIMITED)
		MaxUpload = theApp.uploadBandwidthThrottler->GetEstiminatedLimit();

 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
	MaxUpload -= (float)GetReservedDatarate()/1024.0f;

	if(NeoPrefs.IsSeparateReleaseBandwidth()){
		curUpSlots -= releaseSlots;
		curActiveSlots -= releaseSlots;
	}
	if(NeoPrefs.IsSeparateFriendBandwidth()){
		curUpSlots -= friendSlots;
		curActiveSlots -= friendSlots;
	}
 #endif // BW_MOD // NEO: BM END

	int DefSlots = (uint16)ceil(MaxUpload/NeoPrefs.GetUploadPerSlots());
	if(DefSlots < 3)
		DefSlots = 3; 

	int MaxSlots = max((uint16)ceil(DefSlots*1.33),(uint16)ceil(NeoPrefs.GetUploadPerSlots()) + DefSlots);
	if(NeoPrefs.GetUploadPerSlots() > 6)
		MaxSlots = max(MaxSlots,(uint16)ceil(MaxUpload/4));
	else if(NeoPrefs.GetUploadPerSlots() > 4)
		MaxSlots = max(MaxSlots,(uint16)ceil(MaxUpload/3));

	// Do we reached our estimated max slot limit
	if(!addOnNextConnect && curUpSlots >= MaxSlots)
	{
		//Xman count the blocksend to remove such clients if needed
		//there are many clients out there, which can't take a high slotspeed (often less than 1kbs)
		//in worst case out upload is full of them and because no new slots are opened
		//our over all upload decrease
		//why should we keep such bad clients ? we keep it only if we have enough slots left
		//if our slot-max is reached, we drop the most blocking client
		if(NeoPrefs.IsDropBlocking() && theApp.uploadBandwidthThrottler->ForceNewClient() && uploadinglist.GetCount())
		{
			float avgratio = 0.0f;
			float blockratio = 0.5f;
			CUpDownClient* blockclient = NULL;
			POSITION pos = uploadinglist.GetHeadPosition();
			while(pos != NULL){
				CUpDownClient* cur_client = uploadinglist.GetNext(pos);
				CEMSocket* cur_socket = cur_client->GetFileUploadSocket();
				if(cur_socket == NULL || cur_socket->m_eSlotState == SS_NONE)
					continue;
				avgratio += cur_socket->GetAvgRatio();
				if(cur_socket->GetAvgRatio() < blockratio){
					blockratio = cur_socket->GetAvgRatio();
					blockclient = cur_client;
				}
			}
			avgratio /= uploadinglist.GetCount();

			//because there are some users out, which set a too high uploadlimit,
			//this code isn't useable we deactivateif
			if(blockclient!=NULL && (blockratio/avgratio) < 0.20f)
			{
				RemoveFromUploadQueue(blockclient,_T("client is blocking too often"));
			}
		}
		theApp.uploadBandwidthThrottler->SetNoNeedSlot(); //this can occur after increasing slotspeed
		//remark: we always return false here... also on removing blockclient
		//next loop a new client will be accepted
		return false;
	}

	// Low ID client adding
	if(addOnNextConnect && ((
		(NeoPrefs.IsOpenMoreSlotsWhenNeeded() && !NeoPrefs.IsMinimizeOpenedSlots()) // in what mode do we add the client
		? (curUpSlots - curActiveSlots <= (UINT)NeoPrefs.GetMaxReservedSlots()) //if less then allowed reserve then add
		: (curUpSlots <= DefSlots) //We allow an extra slot to lowID users.
	 ) /*|| lastupslotHighID == true*/)){ //or last client was highid
		 DEBUG_ONLY(DebugLog(_T("*** Adding client to upload, reason: LowID ADD")));
		return true;
	 }

	// When we minimize the slot number we only open new slots whe it is realy needed
	if(NeoPrefs.IsMinimizeOpenedSlots() && !theApp.uploadBandwidthThrottler->ForceNewClient())
		return false;

	// if not all sockets are redy dont open new slots !
	POSITION pos = uploadinglist.GetHeadPosition();
	while(pos != NULL){
		CUpDownClient* cur_client = uploadinglist.GetNext(pos);
		CEMSocket* cur_socket = cur_client->GetFileUploadSocket();
		if(cur_socket == NULL || cur_socket->m_eSlotState == SS_NONE)
			continue;
		if(cur_socket->m_IsReady == false){ 
			//client isn't responding for >7 sec -->new slot
			//if(cur_client->GetUpStartTimeDelay() <= SEC2MS(7))
			if(cur_client->GetUpStartTimeDelay() <= SEC(7)) // NEO: CUT - [CleanUploadTiming]
				return false;
			break;
		}
	}

	// Do we demand a trickle slot
	if(NeoPrefs.IsBadwolfsUpload() && curUpSlots - curActiveSlots < ((MaxUpload <= 10 || NeoPrefs.IsBadwolfsUpload() != TRUE) ? 1 : (UINT)NeoPrefs.GetMaxReservedSlots())){
		DEBUG_ONLY(DebugLog(_T("*** Adding client to upload, reason: Badwolfs Upload")));
		return true;
	}

	// can we add a slot, is there enough free space
	if(!NeoPrefs.IsOpenMoreSlotsWhenNeeded() && curUpSlots < DefSlots){
		DEBUG_ONLY(DebugLog(_T("*** Adding client to upload, reason: curUpSlots < DefSlots")));
		return true;
	}else if(curUpSlots < DefSlots/2)
		return true;

	// Do we need more slots
	if (addOnNextConnect || !NeoPrefs.IsOpenMoreSlotsWhenNeeded() || !theApp.uploadBandwidthThrottler->ForceNewClient())
		return false;

	DEBUG_ONLY(DebugLog(_T("*** Adding client to upload, reason: else")));
	return true;
}

bool CUploadQueue::ForceNewClient(bool allowEmptyWaitingQueue)
{
    if(!allowEmptyWaitingQueue && waitinglist.GetSize() <= 0)
        return false;

	if (::GetTickCount() - m_nLastStartUpload < SEC2MS(3))
		return false;

	// NEO: NCC - [NeoConnectionChecker]
	// don't add clinets when internet connection is down
	if(!theApp.GetConState(true))
		return false;
	// NEO: NCC END

	// Check the absulut min/max limits
	uint32 curUploadSlots = (uint32)GetUploadQueueLength();
	if (curUploadSlots < (uint32)NeoPrefs.GetMinUploadSlots())
		return true;
	else if(curUploadSlots >= (uint32)NeoPrefs.GetMaxUploadSlots())
		return false;

	 // UploadSpeedSense can veto a new slot if USS enabled
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
	if(!theApp.bandwidthControl->AcceptNewClient())
#else
	if(!theApp.lastCommonRouteFinder->AcceptNewClient())
#endif // NEO_BC // NEO: NBC END
		return false;

	if(::GetTickCount() - lastUploadSlotCheck > SEC2MS(1))
		lastUploadSlotCheck = ::GetTickCount();
	else
		return false;

	return AcceptNewClient();
}

#else

bool CUploadQueue::AcceptNewClient(bool addOnNextConnect)
{
	uint32 curUploadSlots = (uint32)uploadinglist.GetCount();

	//We allow ONE extra slot to be created to accommodate lowID users.
	//This is because we skip these users when it was actually their turn
	//to get an upload slot..
	if(addOnNextConnect && curUploadSlots > 0)
		curUploadSlots--;		

    return AcceptNewClient(curUploadSlots);
}

bool CUploadQueue::AcceptNewClient(uint32 curUploadSlots){
	// check if we can allow a new client to start downloading from us

	if (curUploadSlots < MIN_UP_CLIENTS_ALLOWED)
		return true;

	uint16 MaxSpeed;
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
	if(theApp.bandwidthControl->GetMaxUpload() == UNLIMITED)
		MaxUpload = UNLIMITED;
	else
		MaxUpload =  theApp.bandwidthControl->GetMaxUpload() * 1024.0f;
#else
    if (thePrefs.IsDynUpEnabled())
        MaxSpeed = (uint16)(theApp.lastCommonRouteFinder->GetUpload()/1024);
    else
		MaxSpeed = thePrefs.GetMaxUpload();
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
	
	if (curUploadSlots >= MAX_UP_CLIENTS_ALLOWED ||
        curUploadSlots >= 4 &&
        (
         curUploadSlots >= (datarate/UPLOAD_CHECK_CLIENT_DR) ||
         curUploadSlots >= ((uint32)MaxSpeed)*1024/UPLOAD_CLIENT_DATARATE ||
         (
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
          MaxUpload == UNLIMITED &&
          !thePrefs.IsUSSEnabled() &&
#else
          thePrefs.GetMaxUpload() == UNLIMITED &&
          !thePrefs.IsDynUpEnabled() &&
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
          thePrefs.GetMaxGraphUploadRate(true) > 0 &&
          curUploadSlots >= ((uint32)thePrefs.GetMaxGraphUploadRate(false))*1024/UPLOAD_CLIENT_DATARATE
         )
        )
    ) // max number of clients to allow for all circumstances
	    return false;

	return true;
}

bool CUploadQueue::ForceNewClient(bool allowEmptyWaitingQueue) {
    if(!allowEmptyWaitingQueue && waitinglist.GetSize() <= 0)
        return false;

	if (::GetTickCount() - m_nLastStartUpload < 1000 && datarate < 102400 )
		return false;

	// NEO: NCC - [NeoConnectionChecker] -- Xanatos -->
	// don't add clinets when internet connection is down
	if(!theApp.GetConState(true))
		return false;
	// NEO: NCC END <-- Xanatos --

	uint32 curUploadSlots = (uint32)uploadinglist.GetCount();

	if (curUploadSlots < MIN_UP_CLIENTS_ALLOWED)
		return true;


#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
	if(!AcceptNewClient(curUploadSlots) || !theApp.bandwidthControl->AcceptNewClient()) { // UploadSpeedSense can veto a new slot if USS enabled
#else
    if(!AcceptNewClient(curUploadSlots) || !theApp.lastCommonRouteFinder->AcceptNewClient()) { // UploadSpeedSense can veto a new slot if USS enabled
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
		return false;
    }

	uint16 MaxSpeed;
 #ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
	MaxSpeed =  theApp.bandwidthControl->GetMaxUpload();
 #else
    if (thePrefs.IsDynUpEnabled())
        MaxSpeed = (uint16)(theApp.lastCommonRouteFinder->GetUpload()/1024);
    else
		MaxSpeed = thePrefs.GetMaxUpload();
 #endif // NEO_BC // NEO: NBC END <-- Xanatos --

	uint32 upPerClient = UPLOAD_CLIENT_DATARATE;

    // if throttler doesn't require another slot, go with a slightly more restrictive method
	if( MaxSpeed > 20 || MaxSpeed == UNLIMITED)
		upPerClient += datarate/43;

	if( upPerClient > 7680 )
		upPerClient = 7680;

	//now the final check

 #ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
	if (!theApp.bandwidthControl->IsPingWorking() && !theApp.bandwidthControl->IsPingPreparing() && MaxSpeed == UNLIMITED )
 #else
	if ( MaxSpeed == UNLIMITED )
 #endif // NEO_BC // NEO: NBC END <-- Xanatos --
	{
		if (curUploadSlots < (datarate/upPerClient))
			return true;
	}
	else{
		uint16 nMaxSlots;
		if (MaxSpeed > 12)
			nMaxSlots = (uint16)(((float)(MaxSpeed*1024)) / upPerClient);
		else if (MaxSpeed > 7)
			nMaxSlots = MIN_UP_CLIENTS_ALLOWED + 2;
		else if (MaxSpeed > 3)
			nMaxSlots = MIN_UP_CLIENTS_ALLOWED + 1;
		else
			nMaxSlots = MIN_UP_CLIENTS_ALLOWED;
//		AddLogLine(true,"maxslots=%u, upPerClient=%u, datarateslot=%u|%u|%u",nMaxSlots,upPerClient,datarate/UPLOAD_CHECK_CLIENT_DR, datarate, UPLOAD_CHECK_CLIENT_DR);

		if ( curUploadSlots < nMaxSlots )
		{
			return true;
		}
	}

#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
    if(theApp.uploadBandwidthThrottler->ForceNewClient()) {
#else
    if(m_iHighestNumberOfFullyActivatedSlotsSinceLastCall > (uint32)uploadinglist.GetSize()) {
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
        // uploadThrottler requests another slot. If throttler says it needs another slot, we will allow more slots
        // than what we require ourself. Never allow more slots than to give each slot high enough average transfer speed, though (checked above).
        //if(thePrefs.GetLogUlDlEvents() && waitinglist.GetSize() > 0)
        //    AddDebugLogLine(false, _T("UploadQueue: Added new slot since throttler needs it. m_iHighestNumberOfFullyActivatedSlotsSinceLastCall: %i uploadinglist.GetSize(): %i tick: %i"), m_iHighestNumberOfFullyActivatedSlotsSinceLastCall, uploadinglist.GetSize(), ::GetTickCount());
        return true;
    }

    //nope
	return false;
}
#endif // NEO_UBT // NEO: NUSM END <-- Xanatos --
    
CUploadQueue::~CUploadQueue(){
	if (h_timer)
		KillTimer(0,h_timer);
}

CUpDownClient* CUploadQueue::GetWaitingClientByIP_UDP(uint32 dwIP, uint16 nUDPPort, bool bIgnorePortOnUniqueIP, bool* pbMultipleIPs){
	CUpDownClient* pMatchingIPClient = NULL;
	uint32 cMatches = 0;
	for (POSITION pos = waitinglist.GetHeadPosition();pos != 0;){
		CUpDownClient* cur_client = waitinglist.GetNext(pos);
		if (dwIP == cur_client->GetIP() && nUDPPort == cur_client->GetUDPPort())
			return cur_client;
		else if (dwIP == cur_client->GetIP() && bIgnorePortOnUniqueIP){
			pMatchingIPClient = cur_client;
			cMatches++;
		}
	}
	if (pbMultipleIPs != NULL)
		*pbMultipleIPs = cMatches > 1;

	if (pMatchingIPClient != NULL && cMatches == 1)
		return pMatchingIPClient;
	else
		return NULL;
}

CUpDownClient* CUploadQueue::GetWaitingClientByIP(uint32 dwIP){
	for (POSITION pos = waitinglist.GetHeadPosition();pos != 0;){
		CUpDownClient* cur_client = waitinglist.GetNext(pos);
		if (dwIP == cur_client->GetIP())
			return cur_client;
	}
	return 0;
}

/**
 * Add a client to the waiting queue for uploads.
 *
 * @param client address of the client that should be added to the waiting queue
 *
 * @param bIgnoreTimelimit don't check time limit to possibly ban the client.
 */
void CUploadQueue::AddClientToQueue(CUpDownClient* client, bool bIgnoreTimelimit)
{
	// NEO: NCC - [NeoConnectionChecker] -- Xanatos -->
	if(!theApp.GetConState())
		return;
	// NEO: NCC END <-- Xanatos --

	//This is to keep users from abusing the limits we put on lowID callbacks.
	//1)Check if we are connected to any network and that we are a lowID.
	//(Although this check shouldn't matter as they wouldn't have found us..
	// But, maybe I'm missing something, so it's best to check as a precaution.)
	//2)Check if the user is connected to Kad. We do allow all Kad Callbacks.
	//3)Check if the user is in our download list or a friend..
	//We give these users a special pass as they are helping us..
	//4)Are we connected to a server? If we are, is the user on the same server?
	//TCP lowID callbacks are also allowed..
	//5)If the queue is very short, allow anyone in as we want to make sure
	//our upload is always used.
	if (theApp.IsConnected() 
		&& theApp.IsFirewalled()
		&& !client->GetKadPort()
		&& client->GetDownloadState() == DS_NONE 
		&& !client->IsFriend()
		&& theApp.serverconnect
		&& !theApp.serverconnect->IsLocalServer(client->GetServerIP(),client->GetServerPort())
		&& GetWaitingUserCount() > 50)
		return;

	client->AddAskedCount();
	client->SetLastUpRequest();
	if (!bIgnoreTimelimit)
#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->
	  if(NeoPrefs.IsAgressionDetection())
#endif //ARGOS // NEO: NA END <-- Xanatos --
		client->AddRequestCount(client->GetUploadFileID());
	if (client->IsBanned())
		return;

	// NEO: NCC - [NeoConnectionChecker] -- Xanatos -->
	if(!theApp.GetConState())
		return;
	// NEO: NCC END <-- Xanatos --

	uint16 cSameIP = 0;
	// check for double
	POSITION pos1, pos2;
	for (pos1 = waitinglist.GetHeadPosition();( pos2 = pos1 ) != NULL;)
	{
		waitinglist.GetNext(pos1);
		CUpDownClient* cur_client= waitinglist.GetAt(pos2);
		if (cur_client == client)
		{	
			//if (client->m_bAddNextConnect && AcceptNewClient(client->m_bAddNextConnect))
			if (client->GetUploadState() == US_PENDING && AcceptNewClient(true)) // NEO: MOD - [NewUploadState] <-- Xanatos --
			{
				//Special care is given to lowID clients that missed their upload slot
				//due to the saving bandwidth on callbacks.
				if(thePrefs.GetLogUlDlEvents())
					//AddDebugLogLine(true, _T("Adding ****lowid when reconnecting. Client: %s"), client->DbgGetClientInfo());
					AddDebugLogLine(true, client->m_fUpIsProblematic ? _T("Adding ~~~problematic client (second change) on reconnect. Client: %s") : _T("Adding ****lowid when reconnecting. Client: %s"), client->DbgGetClientInfo()); // NEO: UPC - [UploadingProblemClient] <-- Xanatos --
				//client->m_bAddNextConnect = false; // NEO: MOD - [NewUploadState] <-- Xanatos --
				RemoveFromWaitingQueue(client, true);
				// statistic values // TODO: Maybe we should change this to count each request for a file only once and ignore reasks
				CKnownFile* reqfile = theApp.sharedfiles->GetFileByID((uchar*)client->GetUploadFileID());
				if (reqfile)
					reqfile->statistic.AddRequest();
				//AddUpNextClient(_T("Adding ****lowid when reconnecting."), client);
				AddUpNextClient(client->m_fUpIsProblematic ? _T("Adding ~~~problematic client (second change) on reconnect.") : _T("Adding ****lowid when reconnecting."), client); // NEO: UPC - [UploadingProblemClient] <-- Xanatos --
				return;
			}
			client->SendRankingInfo();
			theApp.emuledlg->transferwnd->queuelistctrl.RefreshClient(client);
			return;			
		}
		else if ( client->Compare(cur_client) ) 
		{
			theApp.clientlist->AddTrackClient(client); // in any case keep track of this client

			// another client with same ip:port or hash
			// this happens only in rare cases, because same userhash / ip:ports are assigned to the right client on connecting in most cases
			if (cur_client->credits != NULL && cur_client->credits->GetCurrentIdentState(cur_client->GetIP()) == IS_IDENTIFIED)
			{
				//cur_client has a valid secure hash, don't remove him
				if (thePrefs.GetVerbose())
					AddDebugLogLine(false, GetResString(IDS_SAMEUSERHASH), client->GetUserName(), cur_client->GetUserName(), client->GetUserName());
				return;
			}
			if (client->credits != NULL && client->credits->GetCurrentIdentState(client->GetIP()) == IS_IDENTIFIED)
			{
				//client has a valid secure hash, add him remove other one
				if (thePrefs.GetVerbose())
					AddDebugLogLine(false, GetResString(IDS_SAMEUSERHASH), client->GetUserName(), cur_client->GetUserName(), cur_client->GetUserName());
				RemoveFromWaitingQueue(pos2,true);
				if (!cur_client->socket)
				{
					if(cur_client->Disconnected(_T("AddClientToQueue - same userhash 1")))
						delete cur_client;
				}
			}
			else
			{
				// remove both since we do not know who the bad one is
				if (thePrefs.GetVerbose())
					AddDebugLogLine(false, GetResString(IDS_SAMEUSERHASH), client->GetUserName() ,cur_client->GetUserName(), _T("Both"));
				RemoveFromWaitingQueue(pos2,true);	
				if (!cur_client->socket)
				{
					if(cur_client->Disconnected(_T("AddClientToQueue - same userhash 2")))
						delete cur_client;
				}
				return;
			}
		}
		else if (client->GetIP() == cur_client->GetIP())
		{
			// same IP, different port, different userhash
			cSameIP++;
		}
	}
	if (cSameIP >= 3)
	{
		// do not accept more than 3 clients from the same IP
		if (thePrefs.GetVerbose())
			DEBUG_ONLY( AddDebugLogLine(false,_T("%s's (%s) request to enter the queue was rejected, because of too many clients with the same IP"), client->GetUserName(), ipstr(client->GetConnectIP())) );
		return;
	}
	else if (theApp.clientlist->GetClientsFromIP(client->GetIP()) >= 3)
	{
		if (thePrefs.GetVerbose())
			DEBUG_ONLY( AddDebugLogLine(false,_T("%s's (%s) request to enter the queue was rejected, because of too many clients with the same IP (found in TrackedClientsList)"), client->GetUserName(), ipstr(client->GetConnectIP())) );
		return;
	}
	// done

	// statistic values // TODO: Maybe we should change this to count each request for a file only once and ignore reasks
	CKnownFile* reqfile = theApp.sharedfiles->GetFileByID((uchar*)client->GetUploadFileID());
	if (reqfile)
		reqfile->statistic.AddRequest();

	// emule collection will bypass the queue
	//if (reqfile != NULL && CCollection::HasCollectionExtention(reqfile->GetFileName()) && reqfile->GetFileSize() < (uint64)MAXPRIORITYCOLL_SIZE
	if (reqfile != NULL && reqfile->IsCollection() && reqfile->GetFileSize() < (uint64)MAXPRIORITYCOLL_SIZE  // NEO: MOD - [IsCollection] <-- Xanatos --
		&& !client->IsDownloading() && client->socket != NULL && client->socket->IsConnected())
	{
		client->SetCollectionUploadSlot(true);
		RemoveFromWaitingQueue(client, true);
		AddUpNextClient(_T("Collection Priority Slot"), client);
		return;
	}
	else
		client->SetCollectionUploadSlot(false);

	// NEO: TQ - [TweakUploadQueue] -- Xanatos -->
	if(!NeoPrefs.UseInfiniteQueue()){
		uint32 softQueueLimit = thePrefs.GetQueueSize();
		if((uint32)waitinglist.GetCount() >= softQueueLimit){
			bool bReturn = true;

			if(reqfile != NULL && NeoPrefs.IsQueueOverFlowRelease() && reqfile->GetReleasePriority()){
				if(NeoPrefs.IsQueueOverFlowRelease() == 1){
					bReturn = false;
				}else{
					uint32 hardQueueLimit = softQueueLimit + ((softQueueLimit * NeoPrefs.GetQueueOverFlowRelease()) / 100);
					if((uint32)waitinglist.GetCount() < hardQueueLimit)
						bReturn = false;
				}
			}
					
			if(bReturn && NeoPrefs.IsQueueOverFlowEx()){
				uint32 QueuedCount = theApp.sharedfiles->GetCount();
				if (QueuedCount){ //Should never happens, but incase
					uint32 MinContingent = (softQueueLimit / QueuedCount / 2);
					if (reqfile != NULL && reqfile->GetQueuedCount() < MinContingent){
						if(NeoPrefs.IsQueueOverFlowEx() == 1){
							bReturn = false;
						}else{
							uint32 hardQueueLimit = softQueueLimit + ((softQueueLimit * NeoPrefs.GetQueueOverFlowEx()) / 100);
							if((uint32)waitinglist.GetCount() < hardQueueLimit)
								bReturn = false;
						}
					}
				}
			}

			if(bReturn && NeoPrefs.IsQueueOverFlowCF()){
				if(client->IsFriend()){
					if(NeoPrefs.IsQueueOverFlowCF() == 1){
						bReturn = false;
					}else{
						uint32 hardQueueLimit = softQueueLimit + ((softQueueLimit * NeoPrefs.GetQueueOverFlowCF()) / 100);
						if((uint32)waitinglist.GetCount() < hardQueueLimit)
							bReturn = false;
					}
				}
			}

			if(bReturn && NeoPrefs.IsQueueOverFlowDef()){
				if (!(
				  (uint32)waitinglist.GetCount() >= softQueueLimit && // soft queue limit is reached
				  (client->IsFriend() && client->GetFriendSlot()) == false && // client is not a friend with friend slot
				  client->GetCombinedFilePrioAndCredit() < GetAverageCombinedFilePrioAndCredit())){ // and client has lower credits/wants lower prio file than average client in queue
					if(NeoPrefs.IsQueueOverFlowDef() == 1){
						bReturn = false;
					}else{
						uint32 hardQueueLimit = softQueueLimit + ((softQueueLimit * NeoPrefs.GetQueueOverFlowDef()) / 100);
						if((uint32)waitinglist.GetCount() < hardQueueLimit)
							bReturn = false;
					}
				}
			}

			if(bReturn)
				return;
		}
	}
	// NEO: TQ END <-- Xanatos --

	/*
    // cap the list
    // the queue limit in prefs is only a soft limit. Hard limit is 25% higher, to let in powershare clients and other
    // high ranking clients after soft limit has been reached
    uint32 softQueueLimit = thePrefs.GetQueueSize();
    uint32 hardQueueLimit = thePrefs.GetQueueSize() + max(thePrefs.GetQueueSize()/4, 200);

    // if soft queue limit has been reached, only let in high ranking clients
    if ((uint32)waitinglist.GetCount() >= hardQueueLimit ||
        (uint32)waitinglist.GetCount() >= softQueueLimit && // soft queue limit is reached
        (client->IsFriend() && client->GetFriendSlot()) == false && // client is not a friend with friend slot
        client->GetCombinedFilePrioAndCredit() < GetAverageCombinedFilePrioAndCredit()) { // and client has lower credits/wants lower prio file than average client in queue

        // then block client from getting on queue
		return;
	}
	*/

	if (client->IsDownloading())
	{
#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->
		if (NeoPrefs.CloseMaellaBackdoor()){
			RemoveFromUploadQueue(client, _T("wrong file"),true);
			client->SendOutOfPartReqsAndAddToWaitingQueue();
			return;
		}
#endif // ARGOS // NEO: NA END <-- Xanatos --
		// he's already downloading and wants probably only another file
		client->SendAcceptUpload(); // NEO: MOD - [SendAcceptUpload] <-- Xanatos --
		/*if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__AcceptUploadReq", client);
		Packet* packet = new Packet(OP_ACCEPTUPLOADREQ,0);
		theStats.AddUpDataOverheadFileRequest(packet->size);
		client->socket->SendPacket(packet,true);*/
		return;
	}
	if (waitinglist.IsEmpty() && ForceNewClient(true))
	{
		AddUpNextClient(_T("Direct add with empty queue."), client);
	}
	else
	{
		waitinglist.AddTail(client);
		client->SetUploadState(US_ONUPLOADQUEUE);
		theApp.emuledlg->transferwnd->queuelistctrl.AddClient(client,true);
		theApp.emuledlg->transferwnd->ShowQueueCount(waitinglist.GetCount());
		client->SendRankingInfo();
	}
}

float CUploadQueue::GetAverageCombinedFilePrioAndCredit() {
    DWORD curTick = ::GetTickCount();

    if (curTick - m_dwLastCalculatedAverageCombinedFilePrioAndCredit > 5*1000) {
        m_dwLastCalculatedAverageCombinedFilePrioAndCredit = curTick;

        // TODO: is there a risk of overflow? I don't think so...
        double sum = 0;
	    for (POSITION pos = waitinglist.GetHeadPosition(); pos != NULL; /**/){
		    CUpDownClient* cur_client =	waitinglist.GetNext(pos);
            sum += cur_client->GetCombinedFilePrioAndCredit();
        }
        m_fAverageCombinedFilePrioAndCredit = (float)(sum/waitinglist.GetSize());
    }

    return m_fAverageCombinedFilePrioAndCredit;
}

#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
void CUploadQueue::AddLanClient(CUpDownClient* client)
{
	client->AddAskedCount();
	client->SetLastUpRequest();
	if (client->IsBanned()) // hmm... 
		return;

	// statistic values
	CKnownFile* reqfile = theApp.sharedfiles->GetFileByID((uchar*)client->GetUploadFileID());
	if (reqfile)
		reqfile->statistic.AddRequest();

	// he's already downloading and wants probably only another file
	bool bIsDownloading = client->IsDownloading();

	// To much clients are downloading
	if(lanSlots >= (uint16)NeoPrefs.GetMaxLanUploadSlots() && !bIsDownloading)
		return;

	// tell the client that we are now ready to upload
	client->SendAcceptUpload(); // NEO: PTM - [PrivatTransferManagement] <-- Xanatos --

	// statistic
	if (reqfile)
		reqfile->statistic.AddAccepted();

	if(bIsDownloading)
		return;

	client->SetUploadState(US_UPLOADING);

	client->SetUpStartTime();
	client->ResetSessionUp();

    InsertInUploadingList(client);
	theApp.emuledlg->transferwnd->uploadlistctrl.AddClient(client);
}
#endif //LANCAST // NEO: NLC END <-- Xanatos --

bool CUploadQueue::RemoveFromUploadQueue(CUpDownClient* client, LPCTSTR pszReason, bool updatewindow, bool earlyabort){
    bool result = false;
#if !defined(NEO_UBT) // NEO: NUBT - [NeoUploadBandwidthThrottler] <-- Xanatos --
    uint32 slotCounter = 1;
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
	for (POSITION pos = uploadinglist.GetHeadPosition();pos != 0;){
        POSITION curPos = pos;
        CUpDownClient* curClient = uploadinglist.GetNext(pos);
		if (client == curClient){
			if (updatewindow)
				theApp.emuledlg->transferwnd->uploadlistctrl.RemoveClient(client);

			if (thePrefs.GetLogUlDlEvents())
                //AddDebugLogLine(DLP_DEFAULT, true,_T("Removing client from upload list: %s Client: %s Transferred: %s SessionUp: %s QueueSessionPayload: %s In buffer: %s Req blocks: %i File: %s"), pszReason==NULL ? _T("") : pszReason, client->DbgGetClientInfo(), CastSecondsToHM( client->GetUpStartTimeDelay()/1000), CastItoXBytes(client->GetSessionUp(), false, false), CastItoXBytes(client->GetQueueSessionPayloadUp(), false, false), CastItoXBytes(client->GetPayloadInBuffer()), client->GetNumberOfRequestedBlocksInQueue(), (theApp.sharedfiles->GetFileByID(client->GetUploadFileID())?theApp.sharedfiles->GetFileByID(client->GetUploadFileID())->GetFileName():_T(""))); 
				AddDebugLogLine(DLP_DEFAULT, true,_T("Removing client from upload list: %s Client: %s Transferred: %s SessionUp: %s QueueSessionPayload: %s In buffer: %s Req blocks: %i File: %s"), pszReason==NULL ? _T("") : pszReason, client->DbgGetClientInfo(), CastSecondsToHM( client->GetUpStartTimeDelay()), CastItoXBytes(client->GetSessionUp(), false, false), CastItoXBytes(client->GetQueueSessionPayloadUp(), false, false), CastItoXBytes(client->GetPayloadInBuffer()), client->GetNumberOfRequestedBlocksInQueue(), (theApp.sharedfiles->GetFileByID(client->GetUploadFileID())?theApp.sharedfiles->GetFileByID(client->GetUploadFileID())->GetFileName():_T(""))); // NEO: CUT - [CleanUploadTiming]
            //client->m_bAddNextConnect = false; // NEO: MOD - [NewUploadState] <-- Xanatos --
			uploadinglist.RemoveAt(curPos);

#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
			if(client->IsLanClient())
				lanSlots--;
#endif //LANCAST // NEO: NLC END <-- Xanatos --

            bool removed = theApp.uploadBandwidthThrottler->RemoveFromStandardList(client->socket);
            bool pcRemoved = theApp.uploadBandwidthThrottler->RemoveFromStandardList((CClientReqSocket*)client->m_pPCUpSocket);
			(void)removed;
			(void)pcRemoved;
            //if(thePrefs.GetLogUlDlEvents() && !(removed || pcRemoved)) {
            //    AddDebugLogLine(false, _T("UploadQueue: Didn't find socket to delete. Adress: 0x%x"), client->socket);
            //}

			if(client->GetSessionUp() > 0) {
				++successfullupcount;
				//totaluploadtime += client->GetUpStartTimeDelay()/1000;
				totaluploadtime += client->GetUpStartTimeDelay(); // NEO: CUT - [CleanUploadTiming] <-- Xanatos --
				client->ClearQueueWaitTime(); // NEO: SQ - [SaveUploadQueue] <-- Xanatos --
            }
			// NEO: SQ - [SaveUploadQueue] -- Xanatos -->
			else {
				if(earlyabort == false)
					++failedupcount;
				client->SaveQueueWaitTime();
			}
			// NEO: SQ END <-- Xanatos --
			// else if(earlyabort == false)
			//	++failedupcount;

            CKnownFile* requestedFile = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
            if(requestedFile != NULL) {
                requestedFile->UpdatePartsInfo();
            }
			theApp.clientlist->AddTrackClient(client); // Keep track of this client
			client->SetUploadState(US_NONE);
			client->ClearUploadBlockRequests();
			client->SetCollectionUploadSlot(false);

#if !defined(NEO_UBT) // NEO: NUBT - [NeoUploadBandwidthThrottler] <-- Xanatos --
		  m_iHighestNumberOfFullyActivatedSlotsSinceLastCall = 0;
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --

			result = true;
        } 
#if !defined(NEO_UBT) // NEO: NUBT - [NeoUploadBandwidthThrottler] <-- Xanatos --
		else {
            curClient->SetSlotNumber(slotCounter);
            slotCounter++;
        }
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
	}
	return result;
}

uint32 CUploadQueue::GetAverageUpTime(){
	if( successfullupcount ){
		return totaluploadtime/successfullupcount;
	}
	return 0;
}

bool CUploadQueue::RemoveFromWaitingQueue(CUpDownClient* client, bool updatewindow){
	POSITION pos = waitinglist.Find(client);
	if (pos){
		RemoveFromWaitingQueue(pos,updatewindow);
		return true;
	}
	else
		return false;
}

void CUploadQueue::RemoveFromWaitingQueue(POSITION pos, bool updatewindow){	
	CUpDownClient* todelete = waitinglist.GetAt(pos);
	waitinglist.RemoveAt(pos);
	if (updatewindow) {
		theApp.emuledlg->transferwnd->queuelistctrl.RemoveClient(todelete);
		theApp.emuledlg->transferwnd->ShowQueueCount(waitinglist.GetCount());
	}
	todelete->SaveQueueWaitTime(); // NEO: SQ - [SaveUploadQueue] <-- Xanatos --
	//client->m_bAddNextConnect = false; // NEO: MOD - [NewUploadState] <-- Xanatos --
	todelete->SetUploadState(US_NONE);
}

void CUploadQueue::UpdateMaxClientScore()
{
	m_imaxscore=0;
	for(POSITION pos = waitinglist.GetHeadPosition(); pos != 0; ) {
		//uint32 score = waitinglist.GetNext(pos)->GetScore(true, false);
		CUpDownClient* cur_client =waitinglist.GetNext(pos);
		uint32 score = cur_client->GetScore(true, false);
		if(score > m_imaxscore 
		&& cur_client->m_fUpIsProblematic == 0 // NEO: UPC - [UploadingProblemClient] <-- Xanatos --
		)
			m_imaxscore=score;
	}
}

bool CUploadQueue::CheckForTimeOver(CUpDownClient* client){
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	if(client->IsLanClient())
		return false;
#endif //LANCAST // NEO: NLC END <-- Xanatos --

	//If we have nobody in the queue, do NOT remove the current uploads..
	//This will save some bandwidth and some unneeded swapping from upload/queue/upload..
	if ( waitinglist.IsEmpty() || client->GetFriendSlot() )
		return false;
	
	if(client->HasCollectionUploadSlot()){
		CKnownFile* pDownloadingFile = theApp.sharedfiles->GetFileByID(client->requpfileid);
		if(pDownloadingFile == NULL)
			return true;
		//if (CCollection::HasCollectionExtention(pDownloadingFile->GetFileName()) && pDownloadingFile->GetFileSize() < (uint64)MAXPRIORITYCOLL_SIZE)
		if (pDownloadingFile->IsCollection() && pDownloadingFile->GetFileSize() < (uint64)MAXPRIORITYCOLL_SIZE) // NEO: MOD - [IsCollection] <-- Xanatos --
			return false;
		else{
			if (thePrefs.GetLogUlDlEvents())
				AddDebugLogLine(DLP_HIGH, false, _T("%s: Upload session ended - client with Collection Slot tried to request blocks from another file"), client->GetUserName());
			return true;
		}
	}
	
	if (!thePrefs.TransferFullChunks()){
	    //if( client->GetUpStartTimeDelay() > SESSIONMAXTIME){ // Try to keep the clients from downloading for ever
		if( client->GetUpStartTimeDelay() > MS2SEC(SESSIONMAXTIME)){ // NEO: CUT - [CleanUploadTiming] <-- Xanatos --
		    if (thePrefs.GetLogUlDlEvents())
			    AddDebugLogLine(DLP_LOW, false, _T("%s: Upload session ended due to max time %s."), client->GetUserName(), CastSecondsToHM(SESSIONMAXTIME/1000));
		    return true;
	    }

		// Cache current client score
		const uint32 score = client->GetScore(true, true);

		// Check if another client has a bigger score
		if (score < GetMaxClientScore() && m_dwRemovedClientByScore < GetTickCount()) {
			if (thePrefs.GetLogUlDlEvents())
				AddDebugLogLine(DLP_VERYLOW, false, _T("%s: Upload session ended due to score."), client->GetUserName());
			//Set timer to prevent to many uploadslot getting kick do to score.
			//Upload slots are delayed by a min of 1 sec and the maxscore is reset every 5 sec.
			//So, I choose 6 secs to make sure the maxscore it updated before doing this again.
			m_dwRemovedClientByScore = GetTickCount()+SEC2MS(6);
			return true;
		}
	}
	else{
		// Allow the client to download a specified amount per session
		//if( client->GetQueueSessionPayloadUp() > SESSIONMAXTRANS ){
		if( client->GetQueueSessionPayloadUp() > SESSIONMAXTRANS * (client->GetReleaseSlot() ? NeoPrefs.GetReleaseChunks() : 1) ){ // NEO: SRS - [SmartReleaseSharing] <-- Xanatos --
			if (thePrefs.GetLogUlDlEvents())
				AddDebugLogLine(DLP_DEFAULT, false, _T("%s: Upload session ended due to max transferred amount. %s"), client->GetUserName(), CastItoXBytes(SESSIONMAXTRANS, false, false));
			return true;
		}
	}
	return false;
}

void CUploadQueue::DeleteAll(){
	waitinglist.RemoveAll();
	uploadinglist.RemoveAll();
    // PENDING: Remove from UploadBandwidthThrottler as well!
}

UINT CUploadQueue::GetWaitingPosition(CUpDownClient* client)
{
	if (!IsOnUploadQueue(client))
		return 0;
	UINT rank = 1;
	UINT myscore = client->GetScore(false);
	// NEO: MQ - [MultiQueue] -- Xanatos -->
	if(NeoPrefs.UseMultiQueue())
		// Compare score only with others clients waiting for the same file
		for(POSITION pos = waitinglist.GetHeadPosition(); pos != NULL; ){
			CUpDownClient* pOtherClient = waitinglist.GetNext(pos);
			if(md4cmp(client->GetUploadFileID(), pOtherClient->GetUploadFileID()) == 0 && 
			   pOtherClient->GetScore(false) > myscore){
				rank++;
			}
		}
	else
	// NEO: MQ END <-- Xanatos --
		for (POSITION pos = waitinglist.GetHeadPosition(); pos != 0; ){
			if (waitinglist.GetNext(pos)->GetScore(false) > myscore)
				rank++;
		}
	return rank;
}

VOID CALLBACK CUploadQueue::UploadTimer(HWND /*hwnd*/, UINT /*uMsg*/, UINT_PTR /*idEvent*/, DWORD /*dwTime*/)
// NEO: ND - [NeoDebug] -- Xanatos -->
{
	if(theApp.emuledlg->m_hWnd)
		theApp.emuledlg->SendMessage(TM_DOTIMER, NULL, NULL); 	
}

void CUploadQueue::UploadTimer()
// NEO: ND END <-- Xanatos --	
{
	// NOTE: Always handle all type of MFC exceptions in TimerProcs - otherwise we'll get mem leaks
#ifndef _DEBUG // NEO: ND - [NeoDebug] <-- Xanatos --
	try
	{
#endif // NEO: ND - [NeoDebug] <-- Xanatos --
		// Barry - Don't do anything if the app is shutting down - can cause unhandled exceptions
		if (!theApp.emuledlg->IsRunning())
			return;

		// BEGIN SLUGFILLER: SafeHash - let eMule start first // NEO: STS - [SlugFillerThreadSafe] -- Xanatos -->
		if (theApp.emuledlg->status != 255)
			return;
		// END SLUGFILLER: SafeHash // NEO: STS END <-- Xanatos --

        // Elandal:ThreadSafeLogging -->
        // other threads may have queued up log lines. This prints them.
        theApp.HandleDebugLogQueue();
        theApp.HandleLogQueue();
		theApp.HandleModLogQueue(); // NEO: ML - [ModLog] <-- Xanatos --
        // Elandal: ThreadSafeLogging <--

#ifndef NEO_BC // NEO: NBC - [NeoBandwidthControl] <-- Xanatos --
	// ZZ:UploadSpeedSense -->
	theApp.lastCommonRouteFinder->SetPrefs(thePrefs.IsDynUpEnabled(), 
			theApp.uploadqueue->GetDatarate(), 
			thePrefs.GetMinUpload()*1024, 
			(thePrefs.GetMaxUpload() != 0) ? thePrefs.GetMaxUpload() * 1024 : thePrefs.GetMaxGraphUploadRate(false) * 1024, 
			thePrefs.IsDynUpUseMillisecondPingTolerance(), 
			(thePrefs.GetDynUpPingTolerance() > 100) ? ((thePrefs.GetDynUpPingTolerance() - 100) / 100.0f) : 0, 
			thePrefs.GetDynUpPingToleranceMilliseconds(), 
			thePrefs.GetDynUpGoingUpDivider(), 
			thePrefs.GetDynUpGoingDownDivider(), 
			thePrefs.GetDynUpNumberOfPings(), 
			20); // PENDING: Hard coded min pLowestPingAllowed
	// ZZ:UploadSpeedSense <--
#endif // NEO_BC // NEO: NBC END <-- Xanatos --

#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
		theApp.downloadqueue->ProcessReceiving();
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
		theApp.bandwidthControl->Process();
#endif // NEO_BC // NEO: NBC END <-- Xanatos --

#ifdef NATTUNNELING // NEO: UTCP - [UserModeTCP] -- Xanatos -->
		if(NeoPrefs.IsNatTraversalEnabled())
			theApp.natmanager->Process();
#endif //NATTUNNELING // NEO: UTCP END <-- Xanatos --

        theApp.uploadqueue->Process();
		theApp.downloadqueue->Process();
		if (thePrefs.ShowOverhead()){
			theStats.CompUpDatarateOverhead();
			theStats.CompDownDatarateOverhead();
		}
		counter++;

		// NEO: MOD - [500ms] -- Xanatos -->
		// 500 ms
		if (counter%5==0){
			theApp.listensocket->SetLimitForConnections(); // NEO: SCM - [SmartConnectionManagement]
		}
		// NEO: MOD END <-- Xanatos --


		// one second
		if (counter >= 10){
			counter=0;

			// NEO: ASM - [AccurateSpeedMeasure] -- Xanatos -->
			theApp.downloadqueue->CalculateDownloadRate();
			theApp.uploadqueue->CalculateUploadRate();
			// NEO: ASM END <-- Xanatos --

			// try to use different time intervals here to not create any disk-IO bottle necks by saving all files at once
			theApp.clientcredits->Process();	// 13 minutes
			theApp.serverlist->Process();		// 17 minutes
			theApp.knownfiles->Process();		// 11 minutes
			theApp.friendlist->Process();		// 19 minutes
			theApp.clientlist->Process();
#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase] -- Xanatos -->
			theApp.sourcelist->Process();
#endif // NEO_CD // NEO: NCD END <-- Xanatos --
			theApp.sharedfiles->Process();
			if( Kademlia::CKademlia::IsRunning() )
			{
				Kademlia::CKademlia::Process();
				if(Kademlia::CKademlia::GetPrefs()->HasLostConnection())
				{
					Kademlia::CKademlia::Stop();
					theApp.emuledlg->ShowConnectionState();
				}
			}
			if( theApp.serverconnect->IsConnecting() && !theApp.serverconnect->IsSingleConnect() )
				theApp.serverconnect->TryAnotherConnectionRequest();

			theApp.listensocket->UpdateConnectionsStatus();
			if (thePrefs.WatchClipboard4ED2KLinks()) {
				// TODO: Remove this from here. This has to be done with a clipboard chain
				// and *not* with a timer!!
				theApp.SearchClipboard();
			}

			//if (theApp.serverconnect->IsConnecting())
			//	theApp.serverconnect->CheckForTimeout();
			theApp.serverconnect->Process(); // NEO: MOD - [ServerConnect] <-- Xanatos --

			// -khaos--+++> Update connection stats...
			iupdateconnstats++;
			// 2 seconds
			if (iupdateconnstats>=2) {
				iupdateconnstats=0;
				theStats.UpdateConnectionStats((float)theApp.uploadqueue->GetDatarate()/1024, (float)theApp.downloadqueue->GetDatarate()/1024);
				// NEO: MOD - [UpdateMyInfo] -- Xanatos -->
				if(theApp.emuledlg->activewnd && theApp.emuledlg->activewnd->IsKindOf(RUNTIME_CLASS(CServerWnd)))
					theApp.emuledlg->serverwnd->UpdateMyInfo();
				// NEO: MOD END <-- Xanatos --
			}
			// <-----khaos-

			// display graphs
			if (thePrefs.GetTrafficOMeterInterval()>0) {
				igraph++;

				if (igraph >= (uint32)(thePrefs.GetTrafficOMeterInterval()) ) {
					igraph=0;
					//theApp.emuledlg->statisticswnd->SetCurrentRate((float)(theApp.uploadqueue->Getavgupload()/theApp.uploadqueue->Getavg())/1024,(float)(theApp.uploadqueue->Getavgdownload()/theApp.uploadqueue->Getavg())/1024);
					theApp.emuledlg->statisticswnd->SetCurrentRate((float)(theApp.uploadqueue->GetDatarate())/1024,(float)(theApp.downloadqueue->GetDatarate())/1024);
					//theApp.uploadqueue->Zeroavg();
				}
			}
			if (theApp.emuledlg->activewnd == theApp.emuledlg->statisticswnd && theApp.emuledlg->IsWindowVisible() )  {
				// display stats
				if (thePrefs.GetStatsInterval()>0) {
					istats++;

					if (istats >= (uint32)(thePrefs.GetStatsInterval()) ) {
						istats=0;
						theApp.emuledlg->statisticswnd->ShowStatistics();
					}
				}
			}

            //theApp.uploadqueue->UpdateDatarates(); // NEO: ASM - [AccurateSpeedMeasure] <-- Xanatos --
            
            //save rates every second
			theStats.RecordRate();
			// mobilemule sockets
			theApp.mmserver->Process();

			// ZZ:UploadSpeedSense -->
            theApp.emuledlg->ShowPing();

            bool gotEnoughHosts = theApp.clientlist->GiveClientsForTraceRoute();
            if(gotEnoughHosts == false) {
                theApp.serverlist->GiveServersForTraceRoute();
            }
			// ZZ:UploadSpeedSense <--

			if (theApp.emuledlg->IsTrayIconToFlash())
				theApp.emuledlg->ShowTransferRate(true);

            sec++;
			// *** 5 seconds **********************************************
			if (sec >= 5) {
#ifdef _DEBUG
				if (thePrefs.m_iDbgHeap > 0 && !AfxCheckMemory())
					AfxDebugBreak();
#endif

				sec = 0;
				theApp.listensocket->Process();
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
				if(NeoPrefs.IsVoodooEnabled())
					theApp.voodoo->Process();
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
				theApp.downloadqueue->ProcessQuickStart(); // NEO: QS - [QuickStart] <-- Xanatos --
				theApp.OnlineSig(); // Added By Bouc7 
				if (!theApp.emuledlg->IsTrayIconToFlash())
					theApp.emuledlg->ShowTransferRate();
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
				thePrefs.EstimateMaxUploadCap((uint32)theApp.uploadBandwidthThrottler->GetEstiminatedLimit());
#else
				thePrefs.EstimateMaxUploadCap(theApp.uploadqueue->GetDatarate()/1024);
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
				
				if (!thePrefs.TransferFullChunks())
					theApp.uploadqueue->UpdateMaxClientScore();

				// update cat-titles with downloadinfos only when needed
				if (thePrefs.ShowCatTabInfos() && 
					theApp.emuledlg->activewnd == theApp.emuledlg->transferwnd && 
					theApp.emuledlg->IsWindowVisible()) 
						theApp.emuledlg->transferwnd->UpdateCatTabTitles(false);
				
				if (thePrefs.IsSchedulerEnabled())
					theApp.scheduler->Check();

                theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Uploading, -1);
			}

			statsave++;
			// *** 60 seconds *********************************************
			if (statsave >= 60) {
				// NEO: RKF - [RecheckKadFirewalled] -- Xanatos -->
				if(NeoPrefs.IsRecheckKadFirewalled() 
					&& (::GetTickCount() - theApp.uploadqueue->m_uLastKadFirewallRecheck) > NeoPrefs.GetRecheckKadFirewalledMs()){
					theApp.uploadqueue->m_uLastKadFirewallRecheck = ::GetTickCount();
					if(Kademlia::CKademlia::IsConnected() && Kademlia::CKademlia::IsFirewalled())
						Kademlia::CKademlia::RecheckFirewalled();					
				}
				// NEO: RKF END <-- Xanatos --

				statsave=0;

				if (thePrefs.GetWSIsEnabled())
					theApp.webserver->UpdateSessionCount();

				theApp.serverconnect->KeepConnectionAlive();

				if (thePrefs.GetPreventStandby())
					theApp.ResetStandByIdleTimer(); // Reset Windows idle standby timer if necessary
			}

			_uSaveStatistics++;
			if (_uSaveStatistics >= thePrefs.GetStatsSaveInterval())
			{
				_uSaveStatistics = 0;
				thePrefs.SaveStats();
			}
		}

		// need more accuracy here. don't rely on the 'sec' and 'statsave' helpers.
		thePerfLog.LogSamples();
#ifndef _DEBUG // NEO: ND - [NeoDebug] <-- Xanatos --
	}
	CATCH_DFLT_EXCEPTIONS(_T("CUploadQueue::UploadTimer"))
#endif // NEO: ND - [NeoDebug] <-- Xanatos --
}

CUpDownClient* CUploadQueue::GetNextClient(const CUpDownClient* lastclient){
	if (waitinglist.IsEmpty())
		return 0;
	if (!lastclient)
		return waitinglist.GetHead();
	POSITION pos = waitinglist.Find(const_cast<CUpDownClient*>(lastclient));
	if (!pos){
		TRACE("Error: CUploadQueue::GetNextClient");
		return waitinglist.GetHead();
	}
	waitinglist.GetNext(pos);
	if (!pos)
		return NULL;
	else
		return waitinglist.GetAt(pos);
}

// NEO: ASM - [AccurateSpeedMeasure] -- Xanatos --
//void CUploadQueue::UpdateDatarates() {
//    // Calculate average datarate
//    if(::GetTickCount()-m_lastCalculatedDataRateTick > 500) {
//        m_lastCalculatedDataRateTick = ::GetTickCount();
//
//        if(avarage_dr_list.GetSize() >= 2 && (avarage_tick_list.GetTail() > avarage_tick_list.GetHead())) {
//	        datarate = (UINT)(((m_avarage_dr_sum - avarage_dr_list.GetHead())*1000) / (avarage_tick_list.GetTail() - avarage_tick_list.GetHead()));
//            friendDatarate = (UINT)(((avarage_friend_dr_list.GetTail() - avarage_friend_dr_list.GetHead())*1000) / (avarage_tick_list.GetTail() - avarage_tick_list.GetHead()));
//        }
//    }
//}

uint32 CUploadQueue::GetDatarate() {
    return datarate;
}

// NEO: ASM - [AccurateSpeedMeasure] -- Xanatos --
//uint32 CUploadQueue::GetToNetworkDatarate() {
//    if(datarate > friendDatarate) {
//        return datarate - friendDatarate;
//    } else {
//        return 0;
//    }
//}

#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
void CUploadQueue::ReSortUploadSlots(CUpDownClient* client) {
	if (client)
		theApp.uploadBandwidthThrottler->ReSortUploadSlots(client);
}
#else
void CUploadQueue::ReSortUploadSlots(bool force) {
    DWORD curtick = ::GetTickCount();
    if(force ||  curtick - m_dwLastResortedUploadSlots >= 10*1000) {
        m_dwLastResortedUploadSlots = curtick;

        theApp.uploadBandwidthThrottler->Pause(true);

    	CTypedPtrList<CPtrList, CUpDownClient*> tempUploadinglist;

        // Remove all clients from uploading list and store in tempList
        POSITION ulpos = uploadinglist.GetHeadPosition();
        while (ulpos != NULL) {
            POSITION curpos = ulpos;
            uploadinglist.GetNext(ulpos);

            // Get and remove the client from upload list.
		    CUpDownClient* cur_client = uploadinglist.GetAt(curpos);

            uploadinglist.RemoveAt(curpos);

            // Remove the found Client from UploadBandwidthThrottler
            theApp.uploadBandwidthThrottler->RemoveFromStandardList(cur_client->socket);
            theApp.uploadBandwidthThrottler->RemoveFromStandardList((CClientReqSocket*)cur_client->m_pPCUpSocket);

            tempUploadinglist.AddTail(cur_client);
        }

        // Remove one at a time from temp list and reinsert in correct position in uploading list
        POSITION tempPos = tempUploadinglist.GetHeadPosition();
        while(tempPos != NULL) {
            POSITION curpos = tempPos;
            tempUploadinglist.GetNext(tempPos);

            // Get and remove the client from upload list.
		    CUpDownClient* cur_client = tempUploadinglist.GetAt(curpos);

            tempUploadinglist.RemoveAt(curpos);

            // This will insert in correct place
            InsertInUploadingList(cur_client);
        }

        theApp.uploadBandwidthThrottler->Pause(false);
    }
}
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --

// NEO: UPC - [UploadingProblemClient] -- Xanatos -->
void CUploadQueue::AddClientDirectToQueue(CUpDownClient* client)
{
	if(CKnownFile* reqfile = theApp.sharedfiles->GetFileByID(client->GetUploadFileID()))
	{
		client->SetUploadState(US_PENDING);
		//client->SetUploadState(US_ONUPLOADQUEUE);
		//client->m_bAddNextConnect = true;
		if(waitinglist.Find(client) == 0)
		{
			theApp.uploadqueue->waitinglist.AddTail(client);
			theApp.emuledlg->transferwnd->queuelistctrl.AddClient(client,false);
			theApp.emuledlg->transferwnd->ShowQueueCount(waitinglist.GetCount());
		}
	}
}
// NEO: UPC END <-- Xanatos --

#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
void CUploadQueue::GetTransferTipInfo(CString &info)
{
	info.Format(GetResString(IDS_X_UL_SP), theApp.bandwidthControl->GetCurrStatsDataUpload(), theApp.bandwidthControl->GetCurrStatsDataUpload());
	info.AppendFormat(GetResString(IDS_X_UL_UL), GetUploadQueueLength());
	info.AppendFormat(GetResString(IDS_X_UL_OQ), GetWaitingUserCount(), thePrefs.GetQueueSize());
	TCHAR buffer[100];
	_stprintf(buffer,_T("%u"), theApp.clientlist->GetBannedCount() );
	info.AppendFormat(GetResString(IDS_X_UL_BAN), buffer);
}
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --