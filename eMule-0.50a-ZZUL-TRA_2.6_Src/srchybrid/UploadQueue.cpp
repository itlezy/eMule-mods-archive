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
#include "UploadBandwidthThrottler.h"
#include "ClientList.h"
#include "LastCommonRouteFinder.h"
#include "DownloadQueue.h"
#include "FriendList.h"
#include "Statistics.h"
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
#include "TransferDlg.h"
#include "SearchDlg.h"
#include "StatisticsDlg.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "Kademlia/Kademlia/Prefs.h"
#include "Log.h"
#include "collection.h"
#include "Addons/SpeedGraph/SpeedGraphWnd.h" // X: [SGW] - [SpeedGraphWnd]
#ifdef CLIENTANALYZER
#include "Addons/AntiLeech/ClientAnalyzer.h" //>>> WiZaRd::ClientAnalyzer
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


static uint32 counter, sec, statsave;
static UINT s_uSaveStatistics = 0;
static uint32 igraph, istats, i2Secs;

#ifdef HIGHRES
#define HIGHSPEED_UPLOADRATE_START 500*1024
#define HIGHSPEED_UPLOADRATE_END   300*1024
#endif

CUploadQueue::CUploadQueue()
{
	VERIFY( (h_timer = SetTimer(0,0,100,UploadTimer)) != NULL );
	if (thePrefs.GetVerbose() && !h_timer)
		AddDebugLogLine(true,_T("Failed to create 'upload queue' timer - %s"),GetErrorMessage(GetLastError()));
	datarate = 0;
//ZZUL +
	powershareDatarate = 0; //powershare graph
//ZZUL -
	counter=0;
	successfullupcount = 0;
	failedupcount = 0;
	totaluploadtime = 0;
	m_nLastStartUpload = 0;
	statsave=0;
	i2Secs=0;
    m_iHighestNumberOfFullyActivatedSlotsSinceLastCall = 0;
    m_MaxActiveClients = 0;
    m_MaxActiveClientsShortTime = 0;

    m_lastCalculatedDataRateTick = 0;
    m_avarage_dr_sum = 0;
    friendDatarate = 0;

    m_dwLastResortedUploadSlots = 0;
#ifdef HIGHRES
	m_hHighSpeedUploadTimer = NULL;
#endif
	m_bStatisticsWaitingListDirty = true;
//ZZUL +
    m_dwLastCheckedForHighPrioClient = 0;
    m_dwLastCalculatedAverageCombinedFilePrioAndCredit = 0;
    m_fAverageCombinedFilePrioAndCredit = 0;
//ZZUL -
}

CUploadQueue::~CUploadQueue(){
	if (h_timer)
		KillTimer(0,h_timer);
#ifdef HIGHRES
	if (m_hHighSpeedUploadTimer)
		UseHighSpeedUploadTimer(false);
#endif
}

//ZZUL +
/**
 * Remove the client from upload socket if there's another client with same/higher
 * class that wants to get an upload socket. If there's not another matching client
 * move this client down in the upload list so that it is after all other of it's class.
 *
 * @param client address of the client that should be removed or moved down
 *
 * @return true if the client was removed. false if it is still in upload list
 */
bool CUploadQueue::RemoveOrMoveDown(CUpDownClient* client, bool onlyCheckForRemove) {
    //if(onlyCheckForRemove == false) {
    //    CheckForHighPrioClient();
    //}
    //CUpDownClient* newclient = FindBestClientInQueue(true, client);

//-
        CUpDownClient* newclient = FindBestScheduledForRemovalClientInUploadListThatCanBeReinstated();

        CUpDownClient* queueNewclient = FindBestClientInQueue(false);

        if(queueNewclient &&
           (
            !newclient ||
            !newclient->GetScheduledUploadShouldKeepWaitingTime() && (newclient->IsFriend() && newclient->GetFriendSlot()) == false && newclient->GetPowerShared() == false ||
            (
			 (queueNewclient->IsFriend() && queueNewclient->GetFriendSlot()) == true && (newclient->IsFriend() && newclient->GetFriendSlot()) == false ||
             (queueNewclient->IsFriend() && queueNewclient->GetFriendSlot()) == (newclient->IsFriend() && newclient->GetFriendSlot()) &&
             (
              queueNewclient->GetPowerShared() == true && newclient->GetPowerShared() == false ||
              queueNewclient->GetPowerShared() == true && newclient->GetPowerShared() == true &&
              (
               queueNewclient->GetFilePrioAsNumber() > newclient->GetFilePrioAsNumber() ||
               queueNewclient->GetFilePrioAsNumber() == newclient->GetFilePrioAsNumber() && !newclient->GetScheduledUploadShouldKeepWaitingTime()
              )
             )
            )
           )
          ) {
            // didn't find a scheduled client, or the one we found
            // wasn't pre-empted, and is not special class client, so shouldn't be unscheduled from removal
            newclient = queueNewclient;
        }

//-

    if(newclient != NULL && // Only remove the client if there's someone to replace it
       (
        (client->IsFriend() && client->GetFriendSlot()) == false && // if it is not in a class that gives it a right
        client->GetPowerShared() == false ||                        // to have a check performed to see if it can stay, we remove at once
        (
         (
          (
           newclient->GetPowerShared() == true && client->GetPowerShared() == false || // new client wants powershare file, but old client don't
           newclient->GetPowerShared() == true && client->GetPowerShared() == true && newclient->GetFilePrioAsNumber() >= client->GetFilePrioAsNumber() // both want powersharedfile, and newer wants higher/same prio file
          ) &&
          (client->IsFriend() && client->GetFriendSlot()) == false
         ) || // old client don't have friend slot
         (newclient->IsFriend() && newclient->GetFriendSlot()) == true // new friend has friend slot, this means it is of highest prio, and will always get this slot
        )
       )
      ){

        // Remove client from ul list to make room for higher/same prio client
	    theApp.uploadqueue->ScheduleRemovalFromUploadQueue(client, _T("Successful completion of upload."), GetResString(IDS_COMPLETING));

        return true;
    } else if(onlyCheckForRemove == false) {
		MoveDownInUploadQueue(client);

        return false;
    } else {
        return false;
    }
}

void CUploadQueue::MoveDownInUploadQueue(CUpDownClient* client) {
    // first find the client in the uploadinglist
    uint32 posCounter = 1;
    POSITION foundPos = NULL;
    POSITION pos = uploadinglist.GetHeadPosition();
	while(pos != NULL) {
        CUpDownClient* curClient = uploadinglist.GetAt(pos);
		if (curClient == client){
            foundPos = pos;
        } else {
            curClient->SetSlotNumber(posCounter);
            posCounter++;
        }

        uploadinglist.GetNext(pos);
	}

    if(foundPos != NULL) {
        // Remove the found Client
		uploadinglist.RemoveAt(foundPos);
        theApp.uploadBandwidthThrottler->RemoveFromStandardList(client->socket);

        // then add it last in it's class
        InsertInUploadingList(client);
    }
}

/**
 * Compares two clients, considering requested file and score (waitingtime, credits, requested file prio), and decides if the right
 * client is better than the left clien. If so, it returns true.
 *
 * Clients are ranked in the following classes:
 *    1: Friends (friends are internally ranked by which file they want; if it is powershared; upload priority of file)
 *    2: Clients that wants powershared files of prio release
 *    3: Clients that wants powershared files of prio high
 *    4: Clients that wants powershared files of prio normal
 *    5: Clients that wants powershared files of prio low
 *    6: Clients that wants powershared files of prio lowest
 *    7: Other clients
 *
 * Clients are then ranked inside their classes by their credits and waiting time (== score).
 *
 * Another description of the above ranking:
 *
 * First sortorder is if the client is a friend with a friend slot. Any client that is a friend with a friend slot,
 * is ranked higher than any client that does not have a friend slot.
 *
 * Second sortorder is if the requested file if a powershared file. All clients wanting powershared files are ranked higher
 * than any client wanting a not powershared filed.
 *
 * If the file is powershared, then second sortorder is file priority. For instance. Any client wanting a powershared file with
 * upload priority high, is ranked higher than any client wanting a powershared file with upload file priority normal.
 *
 * If both clients wants powershared files, and of the same upload priority, then the score is used to decide which client is better.
 * The score, as usual, weighs in the client's waiting time, credits, and requested file's upload priority.
 *
 * If both clients wants files that are not powershared, then scores are used to compare the clients, as in official eMule.
 *
 * @param leftClient a pointer to the left client
 *
 * @param leftScore the precalculated score for leftClient, which is calculated with leftClient->GetSCore()
 *
 * @param rightClient a pointer to the right client
 *
 * @param rightScore the precalculated score for rightClient, which is calculated with rightClient->GetSCore()
 *
 * @return true if right client is better, false if clients are equal. False if left client is better.
 */
// <CB Mod : Optimization : RightClientIsBetter>
bool CUploadQueue::RightClientIsBetter(CUpDownClient* leftClient, uint32 leftScore, CUpDownClient* rightClient, uint32 rightScore) {
    if(!rightClient) {
        return false;
    }

	// General denying conditions
	if (rightClient->IsBanned() || IsDownloading(rightClient)) return false;
    
	// Easy check
	if (leftClient == NULL) return true;

	int  rPrio = rightClient->GetFilePrioAsNumber();
	int  lPrio = leftClient->GetFilePrioAsNumber();

	// Condition priority 0
	// Friend slot
	bool rFS = rightClient->IsFriend() && rightClient->GetFriendSlot();
	bool lFS = leftClient->IsFriend() && leftClient->GetFriendSlot();

	if (!rFS && lFS) return false;
	if (rFS && (!lFS || (rPrio > lPrio))) return true;

	// Condition priority 1
	// Power share
	bool rPS = rightClient->GetPowerShared();
	bool lPS = leftClient->GetPowerShared();

	if (!rPS && lPS) return false;
	if (rPS && (!lPS || (rPrio > lPrio))) return true;


	// Condition priority 2
	// LowID slot mising
	bool leftLowIdMissed = leftClient->HasLowID() && leftClient->socket && leftClient->socket->IsConnected() && leftClient->m_dwWouldHaveGottenUploadSlotIfNotLowIdTick && (leftClient->GetQueueSessionPayloadUp() < SESSIONMAXTRANS);
	bool rightLowIdMissed = rightClient->HasLowID() && rightClient->socket && rightClient->socket->IsConnected() && rightClient->m_dwWouldHaveGottenUploadSlotIfNotLowIdTick && (rightClient->GetQueueSessionPayloadUp() < SESSIONMAXTRANS);
	if (!rightLowIdMissed && leftLowIdMissed) return false;
	if (rightLowIdMissed && (!leftLowIdMissed || (leftClient->m_dwWouldHaveGottenUploadSlotIfNotLowIdTick > rightClient->m_dwWouldHaveGottenUploadSlotIfNotLowIdTick)))
        return true;

	// Lowest condition priority
	// Score
	if (rightScore > leftScore) return true;

	// Right client has really nothing to make it better than the left one
        return false;
      }
// </CB Mod : Optimization : RightClientIsBetter>
//ZZUL -

/**
 * Find the highest ranking client in the waiting queue, and return it.
 * Clients are ranked in the following classes:
 *    1: Friends (friends are internally ranked by which file they want; if it is powershared; upload priority of file)
 *    2: Clients that wants powershared files of prio release
 *    3: Clients that wants powershared files of prio high
 *    4: Clients that wants powershared files of prio normal
 *    5: Clients that wants powershared files of prio low
 *    6: Clients that wants powershared files of prio lowest
 *    7: Other clients
 *
 * Clients are then ranked inside their classes by their credits and waiting time (== score).
 *
 * Low id client are ranked as lowest possible, unless they are currently connected.
 * A low id client that is not connected, but would have been ranked highest if it
 * had been connected, gets a flag set. This flag means that the client should be
 * allowed to get an upload slot immediately once it connects.
 *
 * @return address of the highest ranking client.
 */
//ZZUL +
CUpDownClient* CUploadQueue::FindBestClientInQueue(bool allowLowIdAddNextConnectToBeSet, CUpDownClient* lowIdClientMustBeInSameOrBetterClassAsThisClient)
//ZZUL -
{
	POSITION toadd = 0;
	POSITION toaddlow = 0;
	uint32	bestscore = 0;
	uint32  bestlowscore = 0;
    CUpDownClient* newclient = NULL;
    CUpDownClient* lowclient = NULL;

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
			RemoveFromWaitingQueue(pos2,true);
			continue;
		}
        else
        {
		    // finished clearing
		    uint32 cur_score = cur_client->GetScore(false);

//ZZUL +
		    if (RightClientIsBetter(newclient, bestscore, cur_client, cur_score))
//ZZUL -
		    {
                // cur_client is more worthy than current best client that is ready to go (connected).
#ifdef CLIENTANALYZER
				//if our upload tries failed at least 3 times in a row we will only grant a slot while being connected already
				//that way, we hopefully ensure a successful upload session
				if(cur_client->GetAntiLeechData()
					&& cur_client->GetAntiLeechData()->ShouldntUploadForBadSessions())
				{
					if(cur_client->socket && cur_client->socket->IsConnected())
					{
						bestscore = cur_score;
						toadd = pos2;
						newclient = waitinglist.GetAt(toadd);
					}
					continue;
				}
#endif
                if(!cur_client->HasLowID() || (cur_client->socket && cur_client->socket->IsConnected())) {
                    // this client is a HighID or a lowID client that is ready to go (connected)
                    // and it is more worthy
			        bestscore = cur_score;
			        toadd = pos2;
                    newclient = waitinglist.GetAt(toadd);
                } 
//ZZUL +
				else if(allowLowIdAddNextConnectToBeSet && !cur_client->m_dwWouldHaveGottenUploadSlotIfNotLowIdTick)
//ZZUL -
				{
                    // this client is a lowID client that is not ready to go (not connected)
    
                    // now that we know this client is not ready to go, compare it to the best not ready client
                    // the best not ready client may be better than the best ready client, so we need to check
                    // against that client
//ZZUL +
                   if (RightClientIsBetter(lowclient, bestlowscore, cur_client, cur_score))
//ZZUL -
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
//ZZUL +	
	if (bestlowscore > bestscore && lowclient && allowLowIdAddNextConnectToBeSet)
	{
        if(lowIdClientMustBeInSameOrBetterClassAsThisClient == NULL ||
           (lowIdClientMustBeInSameOrBetterClassAsThisClient->IsFriend() && lowIdClientMustBeInSameOrBetterClassAsThisClient->GetFriendSlot()) == false &&
           (lowclient->IsFriend() && lowclient->GetFriendSlot()) == true || // lowclient has friend slot, but lowIdClientMustBeInSameOrBetterClassAsThisClient not. lowclient is better
           (lowIdClientMustBeInSameOrBetterClassAsThisClient->IsFriend() && lowIdClientMustBeInSameOrBetterClassAsThisClient->GetFriendSlot()) == (lowclient->IsFriend() && lowclient->GetFriendSlot()) && // both, or neither has friend slots, let powershared and file prio decide
           (
            lowIdClientMustBeInSameOrBetterClassAsThisClient->GetPowerShared() == false && lowclient->GetPowerShared() == true ||
            lowIdClientMustBeInSameOrBetterClassAsThisClient->GetPowerShared() == true && lowclient->GetPowerShared() == true && // Both want powershared
            lowIdClientMustBeInSameOrBetterClassAsThisClient->GetFilePrioAsNumber() <= lowclient->GetFilePrioAsNumber() || // and lowIdClientMustBeInSameOrBetterClassAsThisClient wants same or lower prio, it's ok
            lowIdClientMustBeInSameOrBetterClassAsThisClient->GetPowerShared() == false && lowclient->GetPowerShared() == false // neither wants powershared file, it's ok
           )
          ){
              DWORD connectTick = ::GetTickCount();
              if(connectTick == 0) connectTick = 1;
		      lowclient->m_dwWouldHaveGottenUploadSlotIfNotLowIdTick = connectTick;
         }
	}
//ZZUL -
    if (!toadd)
		return NULL;
    else
	    return waitinglist.GetAt(toadd);
}

//ZZUL +
/**
 * Insert the client at the correct place in the uploading list.
 * The client should be inserted after all of its class, but before any
 * client of a lower ranking class.
 *
 * Clients are ranked in the following classes:
 *    1: Friends (friends are internally ranked by which file they want; if it is powershared; upload priority of file)
 *    2: Clients that wants powershared files of prio release
 *    3: Clients that wants powershared files of prio high
 *    4: Clients that wants powershared files of prio normal
 *    5: Clients that wants powershared files of prio low
 *    6: Clients that wants powershared files of prio lowest
 *    7: Other clients
 *
 * Since low ID clients are only put in an upload slot when they call us, it means they will
 * have to wait about 10-30 minutes longer to be put in an upload slot than a high id client.
 * In that time, the low ID client could possibly have gone from being a trickle slot, into
 * being a fully activated slot. At the time when the low ID client would have been put into an
 * upload slot, if it had been a high id slot, a boolean flag is set to true (AddNextConnect = true).
 *
 * A client that has AddNextConnect set when it calls back, will immiediately be given an upload slot.
 * When it is added to the upload list with this method, it will also get the time when it entered the
 * queue taken into consideration. It will be added so that it is before all clients (within in its class) 
 * that entered queue later than it. This way it will be able to possibly skip being a trickle slot,
 * since it has already been forced to wait extra time to be put in a upload slot. This makes the
 * low ID clients have almost exactly the same bandwidth from us (proportionally to the number of low ID
 * clients compared to the number of high ID clients) as high ID clients. This is a definitely a further
 * improvement of VQB's excellent low ID handling.
 *
 * @param newclient address of the client that should be inserted in the uploading list
 */
void CUploadQueue::InsertInUploadingList(CUpDownClient* newclient) {
    POSITION insertPosition = NULL;
    uint32 posCounter = uploadinglist.GetCount();

    bool foundposition = false;

    POSITION pos = uploadinglist.GetTailPosition();
    while(pos != NULL && foundposition == false) {
        CUpDownClient* uploadingClient = uploadinglist.GetAt(pos);

		if(uploadingClient->IsScheduledForRemoval() == false && newclient->IsScheduledForRemoval() == true ||
		   uploadingClient->IsScheduledForRemoval() == newclient->IsScheduledForRemoval() &&
           (
            uploadingClient->HasCollectionUploadSlot() && !newclient->HasCollectionUploadSlot() ||
            uploadingClient->HasCollectionUploadSlot() == newclient->HasCollectionUploadSlot() &&
            (
	         (uploadingClient->IsFriend() && uploadingClient->GetFriendSlot()) == true && (newclient->IsFriend() && newclient->GetFriendSlot()) == false ||
             (uploadingClient->IsFriend() && uploadingClient->GetFriendSlot()) == (newclient->IsFriend() && newclient->GetFriendSlot()) &&
             (
              uploadingClient->GetPowerShared() == true && newclient->GetPowerShared() == false ||
              uploadingClient->GetPowerShared() == true && newclient->GetPowerShared() == true && uploadingClient->GetFilePrioAsNumber() > newclient->GetFilePrioAsNumber() ||
              (
                uploadingClient->GetPowerShared() == true && newclient->GetPowerShared() == true && uploadingClient->GetFilePrioAsNumber() == newclient->GetFilePrioAsNumber() ||
                uploadingClient->GetPowerShared() == false && newclient->GetPowerShared() == false
              ) &&
              (
               //uploadingClient->GetDatarate() > newclient->GetDatarate() ||
               //uploadingClient->GetDatarate() == newclient->GetDatarate() &&
               !newclient->IsScheduledForRemoval() &&
               (
                 !newclient->HasLowID() ||
                 !newclient->m_dwWouldHaveGottenUploadSlotIfNotLowIdTick ||
                 newclient->GetQueueSessionPayloadUp() >= SESSIONMAXTRANS ||
                 newclient->HasLowID() && newclient->m_dwWouldHaveGottenUploadSlotIfNotLowIdTick &&
                 newclient->GetUpStartTimeDelay() + (::GetTickCount()-newclient->m_dwWouldHaveGottenUploadSlotIfNotLowIdTick) <= uploadingClient->GetUpStartTimeDelay() + ((uploadingClient->HasLowID() && uploadingClient->m_dwWouldHaveGottenUploadSlotIfNotLowIdTick && uploadingClient->GetQueueSessionPayloadUp() < SESSIONMAXTRANS) ? (::GetTickCount()-uploadingClient->m_dwWouldHaveGottenUploadSlotIfNotLowIdTick) : 0)
               ) ||
               newclient->IsScheduledForRemoval() &&
               uploadingClient->IsScheduledForRemoval() &&
               uploadingClient->GetScheduledUploadShouldKeepWaitingTime() &&
               !newclient->GetScheduledUploadShouldKeepWaitingTime()
              )
             )
            )
           )
          ) {
            foundposition = true;
        } else {
            insertPosition = pos;
            uploadinglist.GetPrev(pos);
            posCounter--;
        }
    }

    if(insertPosition != NULL) {
        POSITION renumberPosition = insertPosition;
        uint32 renumberSlotNumber = posCounter+1;
        
        while(renumberPosition != NULL) {
            CUpDownClient* renumberClient = uploadinglist.GetAt(renumberPosition);

            renumberSlotNumber++;

            renumberClient->SetSlotNumber(renumberSlotNumber);
            renumberClient->UpdateDisplayedInfo(true);

            theApp.emuledlg->transferwnd->GetUploadList()->RefreshClient(renumberClient);

            uploadinglist.GetNext(renumberPosition);
        }

        // add it at found pos
        newclient->SetSlotNumber(posCounter+1);
		uploadinglist.InsertBefore(insertPosition, newclient);
        newclient->UpdateDisplayedInfo(true);
        theApp.uploadBandwidthThrottler->AddToStandardList(posCounter, newclient->GetFileUploadSocket());
    } else {
//ZZUL -
    // Add it last
    theApp.uploadBandwidthThrottler->AddToStandardList(uploadinglist.GetCount(), newclient->GetFileUploadSocket());
	uploadinglist.AddTail(newclient);
    newclient->SetSlotNumber(uploadinglist.GetCount());
}
//ZZUL +
}

CUpDownClient* CUploadQueue::FindLastUnScheduledForRemovalClientInUploadList() {
	POSITION pos = uploadinglist.GetTailPosition();
	while(pos != NULL){
        // Get the client. Note! Also updates pos as a side effect.
		CUpDownClient* cur_client = uploadinglist.GetPrev(pos);

		if(!cur_client->IsScheduledForRemoval()) {
			return cur_client;
		}
	}

    return NULL;
}

CUpDownClient* CUploadQueue::FindBestScheduledForRemovalClientInUploadListThatCanBeReinstated() {
    POSITION pos = uploadinglist.GetHeadPosition();
	while(pos != NULL){
        // Get the client. Note! Also updates pos as a side effect.
		CUpDownClient* cur_client = uploadinglist.GetNext(pos);

        if(cur_client->IsScheduledForRemoval() /*&& cur_client->GetScheduledUploadShouldKeepWaitingTime()*/) {
            return cur_client;
		}
	}

    return NULL;
}

uint32 CUploadQueue::GetEffectiveUploadListCount() {
    uint32 count = 0;

	POSITION pos = uploadinglist.GetTailPosition();
	while(pos != NULL){
        // Get the client. Note! Also updates pos as a side effect.
		CUpDownClient* cur_client = uploadinglist.GetPrev(pos);

		if(!cur_client->IsScheduledForRemoval()) {
			pos = NULL;
        } else {
            count++;
        }
	}

    return uploadinglist.GetCount()-count;
}

bool CUploadQueue::AddUpNextClient(LPCTSTR pszReason, CUpDownClient* directadd, bool highPrioCheck) {
	CUpDownClient* newclient = NULL;
	// select next client or use given client
	if (!directadd)
	{
        if(!highPrioCheck) {
            newclient = FindBestScheduledForRemovalClientInUploadListThatCanBeReinstated();
        }

        CUpDownClient* queueNewclient = FindBestClientInQueue(highPrioCheck == false, newclient);

        if(queueNewclient &&
           (
            !newclient ||
            !newclient->GetScheduledUploadShouldKeepWaitingTime() && (newclient->IsFriend() && newclient->GetFriendSlot()) == false && newclient->GetPowerShared() == false ||
            (
			 (queueNewclient->IsFriend() && queueNewclient->GetFriendSlot()) == true && (newclient->IsFriend() && newclient->GetFriendSlot()) == false ||
             (queueNewclient->IsFriend() && queueNewclient->GetFriendSlot()) == (newclient->IsFriend() && newclient->GetFriendSlot()) &&
             (
              queueNewclient->GetPowerShared() == true && newclient->GetPowerShared() == false ||
              queueNewclient->GetPowerShared() == true && newclient->GetPowerShared() == true &&
              (
               queueNewclient->GetFilePrioAsNumber() > newclient->GetFilePrioAsNumber() ||
               queueNewclient->GetFilePrioAsNumber() == newclient->GetFilePrioAsNumber() && !newclient->GetScheduledUploadShouldKeepWaitingTime()
              )
             )
            )
           )
          ) {
            // didn't find a scheduled client, or the one we found
            // wasn't pre-empted, and is not special class client, so shouldn't be unscheduled from removal
            newclient = queueNewclient;
        }

        if(newclient) {
            if(highPrioCheck == true) {
                if((newclient->IsFriend() && newclient->GetFriendSlot()) || newclient->GetPowerShared()) {
                    CUpDownClient* lastClient = FindLastUnScheduledForRemovalClientInUploadList();

                    if(lastClient != NULL) {
                        if (
                             (newclient->IsFriend() && newclient->GetFriendSlot()) == true && (lastClient->IsFriend() && lastClient->GetFriendSlot()) == false ||
                             (newclient->IsFriend() && newclient->GetFriendSlot()) == (lastClient->IsFriend() && lastClient->GetFriendSlot()) &&
                             (
                               newclient->GetPowerShared() == true && lastClient->GetPowerShared() == false ||
                               newclient->GetPowerShared() == true && lastClient->GetPowerShared() == true && newclient->GetFilePrioAsNumber() > lastClient->GetFilePrioAsNumber()
                             )
                        ) {
                            // Remove last client from ul list to make room for higher prio client
		                    ScheduleRemovalFromUploadQueue(lastClient, _T("Ended upload to make room for higher prio client."), GetResString(IDS_UPLOAD_PREEMPTED), true);
                        } else {
                            return false;
                        }
                    }
                } else {
                    return false;
                }
            }

            if(!IsDownloading(newclient) && !newclient->IsScheduledForRemoval()) {
		    RemoveFromWaitingQueue(newclient, true);
			//theApp.emuledlg->transferwnd->ShowQueueCount(waitinglist.GetCount());
			theApp.emuledlg->transferwnd->ShowQueueCount(); // X: [SFH] - [Show IP Filter Hits]
            //} else {
            //    newclient->UnscheduleForRemoval();
            //    MoveDownInUploadQueue(newclient);
            }
 //ZZUL -
       }
	}
	else 
		newclient = directadd;

    if(newclient == NULL) 
        return false;
//ZZUL +
	if (IsDownloading(newclient))
	{
        if(newclient->IsScheduledForRemoval()) {
            newclient->UnscheduleForRemoval();
            m_nLastStartUpload = ::GetTickCount();
    
            MoveDownInUploadQueue(newclient);

            if(pszReason && thePrefs.GetLogUlDlEvents())
                AddDebugLogLine(false, _T("Unscheduling client from being removed from upload list: %s Client: %s File: %s"), pszReason, newclient->DbgGetClientInfo(), (theApp.sharedfiles->GetFileByID(newclient->GetUploadFileID())?theApp.sharedfiles->GetFileByID(newclient->GetUploadFileID())->GetFileName():_T("")));
            return true;
        }

		return false;
	}

    if(pszReason && thePrefs.GetLogUlDlEvents())
        AddDebugLogLine(false, _T("Adding client to upload list: %s Client: %s File: %s"), pszReason, newclient->DbgGetClientInfo(), (theApp.sharedfiles->GetFileByID(newclient->GetUploadFileID())?theApp.sharedfiles->GetFileByID(newclient->GetUploadFileID())->GetFileName():_T("")));
//ZZUL -
	if (newclient->HasCollectionUploadSlot() && directadd == NULL){
		ASSERT( false );
		newclient->SetCollectionUploadSlot(false);
	}

	// tell the client that we are now ready to upload
	if (!newclient->socket || !newclient->socket->IsConnected())
	{
		newclient->SetUploadState(US_CONNECTING);
		if (!newclient->TryToConnect(true))
			return false;
	}
	else
	{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__AcceptUploadReq", newclient);
#endif
		Packet* packet = new Packet(OP_ACCEPTUPLOADREQ,0);
		theStats.AddUpDataOverheadFileRequest(packet->size);
		newclient->SendPacket(packet, true);
		newclient->SetUploadState(US_UPLOADING);
	}
	newclient->SetUpStartTime();
	newclient->ResetSessionUp();
	newclient->ResetCompressionGain(); // ZZUL-TRA :: ShowCompression

   InsertInUploadingList(newclient);

    m_nLastStartUpload = ::GetTickCount();
//ZZUL +	
    if(newclient->GetQueueSessionUp() > 0) {
        // This client has already gotten a successfullupcount++ when it was early removed.
        // Negate that successfullupcount++ so we can give it a new one when this session ends
        // this prevents a client that gets put back first on queue, being counted twice in the
        // stats.
        successfullupcount--;
        theStats.DecTotalCompletedBytes(newclient->GetQueueSessionUp());
    }
//ZZUL -
	// statistic
	CKnownFile* reqfile = theApp.sharedfiles->GetFileByID((uchar*)newclient->GetUploadFileID());
	if (reqfile)
		reqfile->statistic.AddAccepted();
		
	theApp.emuledlg->transferwnd->GetUploadList()->AddClient(newclient);

	return true;
}

void CUploadQueue::UpdateActiveClientsInfo(DWORD curTick) {
    // Save number of active clients for statistics
    uint32 tempHighest = theApp.uploadBandwidthThrottler->GetHighestNumberOfFullyActivatedSlotsSinceLastCallAndReset();

    if(thePrefs.GetLogUlDlEvents() && theApp.uploadBandwidthThrottler->GetStandardListSize() > (uint32)uploadinglist.GetSize()) {
        // debug info, will remove this when I'm done.
        //AddDebugLogLine(false, _T("UploadQueue: Error! Throttler has more slots than UploadQueue! Throttler: %i UploadQueue: %i Tick: %i"), theApp.uploadBandwidthThrottler->GetStandardListSize(), uploadinglist.GetSize(), ::GetTickCount());
    }
	
	if(tempHighest > (uint32)uploadinglist.GetSize()+1) {
        tempHighest = uploadinglist.GetSize()+1;
	}

    m_iHighestNumberOfFullyActivatedSlotsSinceLastCall = tempHighest;

    // save some data about number of fully active clients
    uint32 tempMaxRemoved = 0;
//ZZUL +
    while(!activeClients_tick_list.IsEmpty() && !activeClients_list.IsEmpty() && curTick-activeClients_tick_list.GetHead() > 2*60*1000) {
//ZZUL -
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

/**
 * Maintenance method for the uploading slots. It adds and removes clients to the
 * uploading list. It also makes sure that all the uploading slots' Sockets always have
 * enough packets in their queues, etc.
 *
 * This method is called approximately once every 100 milliseconds.
 */
void CUploadQueue::Process() {

    DWORD curTick = ::GetTickCount();

    UpdateActiveClientsInfo(curTick);
//ZZUL +
	  CheckForHighPrioClient();

    if(::GetTickCount()-m_nLastStartUpload > SEC2MS(20) && GetEffectiveUploadListCount() > 0 && GetEffectiveUploadListCount() > m_MaxActiveClientsShortTime+GetWantedNumberOfTrickleUploads() && AcceptNewClient(GetEffectiveUploadListCount()-1) == false) {
        // we need to close a trickle slot and put it back first on the queue

        POSITION lastpos = uploadinglist.GetTailPosition();

        CUpDownClient* lastClient = NULL;
        if(lastpos != NULL) {
            lastClient = uploadinglist.GetAt(lastpos);
        }

        if(lastClient != NULL && !lastClient->IsScheduledForRemoval() /*lastClient->GetUpStartTimeDelay() > 3*1000*/) {
            // There's too many open uploads (propably due to the user changing
            // the upload limit to a lower value). Remove the last opened upload and put
            // it back on the waitinglist. When it is put back, it get
            // to keep its waiting time. This means it is likely to soon be
            // choosen for upload again.

            // Remove from upload list.
            ScheduleRemovalFromUploadQueue(lastClient, _T("Too many upload slots opened for current ul speed"), GetResString(IDS_UPLOAD_TOO_MANY_SLOTS), true /*, true*/);

		    // add to queue again.
            // the client is allowed to keep its waiting position in the queue, since it was pre-empted
            //AddClientToQueue(lastClient,true, true);

            m_nLastStartUpload = ::GetTickCount();
        }
	} else if (ForceNewClient()){
//ZZUL -
       // There's not enough open uploads. Open another one.
       AddUpNextClient(_T("Not enough open upload slots for current ul speed"));
	}

    // The loop that feeds the upload slots with data.
	POSITION pos = uploadinglist.GetHeadPosition();
	while(pos != NULL){
        // Get the client. Note! Also updates pos as a side effect.
		CUpDownClient* cur_client = uploadinglist.GetNext(pos);
#ifdef _DEBUG// X: [RDL] - [Remove Debug Log]
		if (thePrefs.m_iDbgHeap >= 2)
			ASSERT_VALID(cur_client);
#endif
		//It seems chatting or friend slots can get stuck at times in upload.. This needs looked into..
		if (!cur_client->socket)
		{
			RemoveFromUploadQueue(cur_client, _T("Uploading to client without socket? (CUploadQueue::Process)"));
			if(cur_client->Disconnected(_T("CUploadQueue::Process"))){
				delete cur_client;
			}
		} else {
//ZZUL +
			if(!cur_client->IsScheduledForRemoval() || ::GetTickCount()-m_nLastStartUpload <= SEC2MS(11) && cur_client->GetSlotNumber() <= GetActiveUploadsCount()+ 2 || ::GetTickCount()-m_nLastStartUpload <= SEC2MS(1) && cur_client->GetSlotNumber() <= GetActiveUploadsCount()+ 10 || ::GetTickCount()-m_nLastStartUpload <= 150 || !cur_client->GetScheduledRemovalLimboComplete() || pos != NULL || cur_client->GetSlotNumber() <= GetActiveUploadsCount() || ForceNewClient(true)) {
//ZZUL -
            cur_client->SendBlockData();
//ZZUL +
			} else {
				bool keepWaitingTime = cur_client->GetScheduledUploadShouldKeepWaitingTime();
				RemoveFromUploadQueue(cur_client, (CString)_T("Scheduled for removal: ") + cur_client->GetScheduledRemovalDebugReason(), true, keepWaitingTime);
				AddClientToQueue(cur_client, true, keepWaitingTime);
                m_nLastStartUpload = ::GetTickCount()-SEC2MS(9);
			}
//ZZUL -
        }
	}

    // Save used bandwidth for speed calculations
	uint64 sentBytes = theApp.uploadBandwidthThrottler->GetNumberOfSentBytesSinceLastCallAndReset();
	avarage_dr_list.AddTail(sentBytes);
    m_avarage_dr_sum += sentBytes;

    (void)theApp.uploadBandwidthThrottler->GetNumberOfSentBytesOverheadSinceLastCallAndReset();

    avarage_friend_dr_list.AddTail(theStats.sessionSentBytesToFriend);
    avarage_powershare_dr_list.AddTail(thePrefs.GetUploadedPowershareTotal());//powershare graph
    // Save time beetween each speed snapshot
    avarage_tick_list.AddTail(curTick);

    // don't save more than 30 secs of data
    //while(avarage_tick_list.GetCount() > 3 && !avarage_friend_dr_list.IsEmpty() && ::GetTickCount()-avarage_tick_list.GetHead() > 30*1000) {
     while(avarage_tick_list.GetCount() > 3 && !avarage_powershare_dr_list.IsEmpty() && !avarage_friend_dr_list.IsEmpty() && ::GetTickCount()-avarage_tick_list.GetHead() > 30*1000) {//powershare graph
   	    m_avarage_dr_sum -= avarage_dr_list.RemoveHead();
         avarage_powershare_dr_list.RemoveHead();//powershare graph
        avarage_friend_dr_list.RemoveHead();
        avarage_tick_list.RemoveHead();
    }
#ifdef HIGHRES
	if (GetDatarate() > HIGHSPEED_UPLOADRATE_START && m_hHighSpeedUploadTimer == 0)
		UseHighSpeedUploadTimer(true);
	else if (GetDatarate() < HIGHSPEED_UPLOADRATE_END && m_hHighSpeedUploadTimer != 0)
		UseHighSpeedUploadTimer(false);
#endif
};

//ZZUL +
bool CUploadQueue::AcceptNewClient(){
	uint32 curUploadSlots = (uint32)GetEffectiveUploadListCount();
 //ZZUL -
   return AcceptNewClient(curUploadSlots);
}

bool CUploadQueue::AcceptNewClient(uint32 curUploadSlots){
	// check if we can allow a new client to start downloading from us

	if (curUploadSlots < MIN_UP_CLIENTS_ALLOWED)
		return true;
//ZZUL +
    uint32 wantedNumberOfTrickles = GetWantedNumberOfTrickleUploads();
    if(curUploadSlots > m_MaxActiveClients+wantedNumberOfTrickles) {
        return false;
    }
//ZZUL -

	// ZZUL-TRA :: BlockRatio Drop Blocking Clients
    const uint32 cur_tick = ::GetTickCount();

	if (thePrefs.GetAutoDropSystem() && m_dwNextBlockingCheck < cur_tick) // we haven't checked lately 
	{
		m_dwNextBlockingCheck = cur_tick + SEC2MS(5); // only check every 5 seconds
		const uint32 uMinUpSpeed = min(1024, (uint32)(thePrefs.GetMaxUpload()*100.0f)); // one tenth of the upload, min 1 kbyte/s
		//search a socket we should remove
		for(POSITION pos = uploadinglist.GetHeadPosition(); pos != NULL;)
		{
			CUpDownClient* cur_client = uploadinglist.GetNext(pos);
			if(!cur_client->IsFriend() && cur_client->GetDatarate() < uMinUpSpeed && // less than a min of upload
				cur_client->GetUpStartTimeDelay() > (MIN2MS(2)) && // this client is already 2 minutes uploading
				cur_client->GetSlotNumber() <= theApp.uploadqueue->GetActiveUploadsCount() && // don't drop trickle slots //morph4u changed
				waitinglist.GetCount() > 0 && // it makes no sense to kick a blocker and can't add anyone
				(
					cur_client->GetFileUploadSocket()->GetBlockRatioOverall() >= 75.0f || //75% of all sends were blocked //morph4u 80->75
					cur_client->GetFileUploadSocket()->GetBlockRatio() >= 85.0f //85% in the last 20 seconds //morph4u 90->85
				)
			  )
			{
				CString buffer;
				buffer.Format(_T("Client blocked too often: avg20: %0.0f%%, all: %0.0f%%, avg-speed: %0.0f B/s"), cur_client->GetFileUploadSocket()->GetBlockRatio(), cur_client->GetFileUploadSocket()->GetBlockRatioOverall(), (float)cur_client->GetSessionUp()/cur_client->GetUpStartTimeDelay()*1000.0f);
				RemoveFromUploadQueue(cur_client, buffer);
				m_dwNextBlockingCheck +=  SEC2MS(10); // drop the next client in at least 15sec to have enough time to establish a new connection...
				break; //only one socket per loop
			}
		}
	}
// ZZUL-TRA :: BlockRatio

	uint16 MaxSpeed;
    if (thePrefs.IsDynUpEnabled())
        MaxSpeed = (uint16)(theApp.lastCommonRouteFinder->GetUpload()/1024);
    else
		MaxSpeed = thePrefs.GetMaxUpload();
//ZZUL +	
	if (curUploadSlots >= thePrefs.GetSlots()+1/*4*/ && // ZZUL-TRA :: SlotControl
        curUploadSlots >= (datarate/UPLOAD_CHECK_CLIENT_DR)
        ) { // max number of clients to allow for all circumstances
            if(curUploadSlots < 20) {
                int numberOfOkClients = 0;
                const int neededOkClients = (int)ceil(curUploadSlots/4.0f);

                POSITION ulpos = uploadinglist.GetHeadPosition();
                while (numberOfOkClients < neededOkClients && ulpos != NULL) {
                    POSITION curpos = ulpos;
                    uploadinglist.GetNext(ulpos);

                    CUpDownClient* checkingClient = uploadinglist.GetAt(curpos);
                    if(checkingClient->GetDatarate() > UPLOAD_CLIENT_DATARATE) {
                        numberOfOkClients++;
                    }
                }

                if(numberOfOkClients >= neededOkClients) {
                    return true;
                } else {
	                return false;
                }
            } else {
	    return false;
            }
    }
//ZZUL -
	return true;
}
//ZZUL +
bool CUploadQueue::ForceNewClient(bool simulateScheduledClosingOfSlot) {
	if (::GetTickCount() - m_nLastStartUpload < SEC2MS(1) && datarate < 102400 )
        return false;

	uint32 curUploadSlots = (uint32)GetEffectiveUploadListCount();
    uint32 curUploadSlotsReal = (uint32)uploadinglist.GetCount();

    if(simulateScheduledClosingOfSlot) {
        if(curUploadSlotsReal < 1) {
            return true;
        } else {
            curUploadSlotsReal--;
        }
    }

	if (curUploadSlots < MIN_UP_CLIENTS_ALLOWED)
		return true;

    if(!AcceptNewClient(curUploadSlots) || !theApp.lastCommonRouteFinder->AcceptNewClient()) { // UploadSpeedSense can veto a new slot if USS enabled
		return false;
    }

    //if(CanForceClient(curUploadSlots)) {
        uint32 activeSlots = m_iHighestNumberOfFullyActivatedSlotsSinceLastCall;
    
        if(simulateScheduledClosingOfSlot) {
            activeSlots = m_MaxActiveClientsShortTime;
        }
    
        if(curUploadSlotsReal < m_iHighestNumberOfFullyActivatedSlotsSinceLastCall && AcceptNewClient(curUploadSlots*2) /*+1*/ ||
           curUploadSlotsReal < m_iHighestNumberOfFullyActivatedSlotsSinceLastCall && ::GetTickCount() - m_nLastStartUpload > SEC2MS(1) ||
            curUploadSlots < m_iHighestNumberOfFullyActivatedSlotsSinceLastCall+1 && ::GetTickCount() - m_nLastStartUpload > SEC2MS(10)) {
            return true;
        }
	return false;
}

bool CUploadQueue::CanForceClient(uint32 curUploadSlots) {
//ZZUL -

	uint16 MaxSpeed;
    if (thePrefs.IsDynUpEnabled())
        MaxSpeed = (uint16)(theApp.lastCommonRouteFinder->GetUpload()/1024);
    else
		MaxSpeed = thePrefs.GetMaxUpload();

	uint32 upPerClient = UPLOAD_CLIENT_DATARATE;

    // if throttler doesn't require another slot, go with a slightly more restrictive method
	if( MaxSpeed > 20 || MaxSpeed == UNLIMITED)
		upPerClient += datarate/43;

	if( upPerClient > 7680 )
		upPerClient = 7680;

	//now the final check

	if ( MaxSpeed == UNLIMITED )
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

    if(m_iHighestNumberOfFullyActivatedSlotsSinceLastCall > (uint32)uploadinglist.GetSize()) {
        // uploadThrottler requests another slot. If throttler says it needs another slot, we will allow more slots
        // than what we require ourself. Never allow more slots than to give each slot high enough average transfer speed, though (checked above).
        //if(thePrefs.GetLogUlDlEvents() && waitinglist.GetSize() > 0)
        //    AddDebugLogLine(false, _T("UploadQueue: Added new slot since throttler needs it. m_iHighestNumberOfFullyActivatedSlotsSinceLastCall: %i uploadinglist.GetSize(): %i tick: %i"), m_iHighestNumberOfFullyActivatedSlotsSinceLastCall, uploadinglist.GetSize(), ::GetTickCount());
        return true;
    }

    //nope
	return false;
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
//ZZUL +
void CUploadQueue::AddClientToQueue(CUpDownClient* client, bool bIgnoreTimelimit, bool addInFirstPlace)
{
    if(addInFirstPlace == false) {
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
		client->AddRequestCount(client->GetUploadFileID());
	if (client->IsBanned())
		return;

#ifdef CLIENTANALYZER
//>>> WiZaRd::ClientAnalyzer
	CKnownFile* reqfile = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
	if(reqfile == NULL)
		return; //should never happen, just in case, though...
	if(client->GetRequestFile() == reqfile && client->IsCompleteSource())
	{
		if(client->GetAntiLeechData())
			client->GetAntiLeechData()->SetBadForThisSession(AT_FILEFAKER);
		return; //nope - come back later...
	}
//<<< WiZaRd::ClientAnalyzer
#endif

	// ZZUL-TRA :: PaybackFirst :: Start
	if (client->Credits() != NULL)
		client->Credits()->InitPayBackFirstStatus();
	// ZZUL-TRA :: PaybackFirst :: End
	}
	uint16 cSameIP = 0;
	// check for double
	POSITION pos1, pos2;
	for (pos1 = waitinglist.GetHeadPosition();( pos2 = pos1 ) != NULL;)
	{
		waitinglist.GetNext(pos1);
		CUpDownClient* cur_client= waitinglist.GetAt(pos2);
		if (cur_client == client)
		{
                        // ZZUL-TRA :: SeeOnQueue :: Start
			// look if the client is now asking for another file
			CKnownFile* newreqfile = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
			CKnownFile* oldreqfile = theApp.sharedfiles->GetFileByID(client->GetOldUploadFileID());
			if(newreqfile != oldreqfile){
				if(oldreqfile)
					oldreqfile->RemoveOnUploadqueue();
				newreqfile->AddOnUploadqueue();
				client->SetOldUploadFileID();
				newreqfile->statistic.AddRequest(); //Bugfix of official client: official client doesn't count a request when a user swapped to other file
			}
			// ZZUL-TRA :: SeeOnQueue :: End
	
			//already on queue
            // VQB LowID Slot Patch, enhanced in ZZUL
            if (addInFirstPlace == false && client->HasLowID() &&
                client->m_dwWouldHaveGottenUploadSlotIfNotLowIdTick && AcceptNewClient())
			{
				//Special care is given to lowID clients that missed their upload slot
				//due to the saving bandwidth on callbacks.
                CUpDownClient* bestQueuedClient = FindBestClientInQueue(false);
                if(bestQueuedClient == client) {
				RemoveFromWaitingQueue(client, true);
				// statistic values // TODO: Maybe we should change this to count each request for a file only once and ignore reasks
				CKnownFile* reqfile = theApp.sharedfiles->GetFileByID((uchar*)client->GetUploadFileID());
#ifndef CLIENTANALYZER
				if (reqfile)
#endif
					reqfile->statistic.AddRequest();
				AddUpNextClient(_T("Adding ****lowid when reconnecting."), client);
				return;
                }
			}
//ZZUL -
			client->SendRankingInfo();
			theApp.emuledlg->transferwnd->GetQueueList()->RefreshClient(client);
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

//ZZUL +
    if(addInFirstPlace == false) {
//ZZUL -
	// statistic values // TODO: Maybe we should change this to count each request for a file only once and ignore reasks
	CKnownFile* reqfile = theApp.sharedfiles->GetFileByID((uchar*)client->GetUploadFileID());
#ifndef CLIENTANALYZER
	if (reqfile)
#endif
		reqfile->statistic.AddRequest();

	// emule collection will bypass the queue
	if (reqfile != NULL && CCollection::HasCollectionExtention(reqfile->GetFileName()) && reqfile->GetFileSize() < (uint64)MAXPRIORITYCOLL_SIZE
		&& !client->IsDownloading() && client->socket != NULL && client->socket->IsConnected())
	{
		client->SetCollectionUploadSlot(true);
		RemoveFromWaitingQueue(client, true);
		client->SetOldUploadFileID(); // ZZUL-TRA :: SeeOnQueue
		AddUpNextClient(_T("Collection Priority Slot"), client);
		return;
	}
	else
		client->SetCollectionUploadSlot(false);

   // cap the list
    // the queue limit in prefs is only a soft limit. Hard limit is 25% higher, to let in powershare clients and other
    // high ranking clients after soft limit has been reached
    uint32 softQueueLimit = thePrefs.GetQueueSize();
    uint32 hardQueueLimit = thePrefs.GetQueueSize() + max(thePrefs.GetQueueSize()/4, 200);

    // if soft queue limit has been reached, only let in high ranking clients
    if ((uint32)waitinglist.GetCount() >= hardQueueLimit ||
        (uint32)waitinglist.GetCount() >= softQueueLimit && // soft queue limit is reached
        (client->IsFriend() && client->GetFriendSlot()) == false && // client is not a friend with friend slot
//ZZUL +
           !client->GetPowerShared() &&
//ZZUL -
       client->GetCombinedFilePrioAndCredit() < GetAverageCombinedFilePrioAndCredit()) { // and client has lower credits/wants lower prio file than average client in queue

        // then block client from getting on queue
		return;
	}
	if (client->IsDownloading())
	{
		// he's already downloading and wants probably only another file
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__AcceptUploadReq", client);
#endif
		Packet* packet = new Packet(OP_ACCEPTUPLOADREQ,0);
		theStats.AddUpDataOverheadFileRequest(packet->size);
		client->SendPacket(packet, true);
		return;
	}
//ZZUL +
        client->ResetQueueSessionUp();
	}
		m_bStatisticsWaitingListDirty = true; //morph4u fix missing code from 0.49 -> 0.50 merge
		waitinglist.AddTail(client);
		client->SetUploadState(US_ONUPLOADQUEUE);

    // Add client to waiting list. If addInFirstPlace is set, client should not have its waiting time resetted
		theApp.emuledlg->transferwnd->GetQueueList()->AddClient(client,true);
		//theApp.emuledlg->transferwnd->ShowQueueCount(waitinglist.GetCount());
	theApp.emuledlg->transferwnd->ShowQueueCount(); // X: [SFH] - [Show IP Filter Hits]
		client->SendRankingInfo();

		// ZZUL-TRA :: SeeOnQueue :: Start
		CKnownFile* reqfile = theApp.sharedfiles->GetFileByID((uchar*)client->GetUploadFileID());
		reqfile->AddOnUploadqueue(); //Do this only if requfile exists!!! (is asked above)	
		client->SetOldUploadFileID();
		// ZZUL-TRA :: SeeOnQueue :: End
	}
//ZZUL -

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
//ZZUL +
void CUploadQueue::ScheduleRemovalFromUploadQueue(CUpDownClient* client, LPCTSTR pszDebugReason, CString strDisplayReason, bool earlyabort) {
	if (thePrefs.GetLogUlDlEvents())
        AddDebugLogLine(DLP_VERYLOW, true,_T("Scheduling to remove client from upload list: %s Client: %s Transfered: %s SessionUp: %s QueueSessionUp: %s QueueSessionPayload: %s File: %s"), pszDebugReason==NULL ? _T("") : pszDebugReason, client->DbgGetClientInfo(), CastSecondsToHM( client->GetUpStartTimeDelay()/1000), CastItoXBytes(client->GetSessionUp(), false, false), CastItoXBytes(client->GetQueueSessionUp(), false, false), CastItoXBytes(client->GetQueueSessionPayloadUp(), false, false), (theApp.sharedfiles->GetFileByID(client->GetUploadFileID())?theApp.sharedfiles->GetFileByID(client->GetUploadFileID())->GetFileName():_T("")));

    client->ScheduleRemovalFromUploadQueue(pszDebugReason, strDisplayReason, earlyabort);
	MoveDownInUploadQueue(client);

    m_nLastStartUpload = ::GetTickCount();
}
//ZZUL -

bool CUploadQueue::RemoveFromUploadQueue(CUpDownClient* client, LPCTSTR pszReason, bool updatewindow, bool earlyabort){
    bool result = false;
    uint32 slotCounter = 1;
	for (POSITION pos = uploadinglist.GetHeadPosition();pos != 0;){
        POSITION curPos = pos;
        CUpDownClient* curClient = uploadinglist.GetNext(pos);
		if (client == curClient){
//ZZUL +
            if(client->socket) {
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
			    if (thePrefs.GetDebugClientTCPLevel() > 0)
				    DebugSend("OP__OutOfPartReqs", client);
#endif
			    Packet* pCancelTransferPacket = new Packet(OP_OUTOFPARTREQS, 0);
			    theStats.AddUpDataOverheadFileRequest(pCancelTransferPacket->size);
			    client->socket->SendPacket(pCancelTransferPacket,true,true);
            }
//ZZUL -
			if (updatewindow)
				theApp.emuledlg->transferwnd->GetUploadList()->RemoveClient(client);

			if (thePrefs.GetLogUlDlEvents())
//ZZUL +
                AddDebugLogLine(DLP_DEFAULT, true,_T("Removing client from upload list: %s Client: %s Transferred: %s SessionUp: %s QueueSessionUp: %s QueueSessionPayload: %s In buffer: %s Req blocks: %i File: %s"), pszReason==NULL ? _T("") : pszReason, client->DbgGetClientInfo(), CastSecondsToHM( client->GetUpStartTimeDelay()/1000), CastItoXBytes(client->GetSessionUp(), false, false), CastItoXBytes(client->GetQueueSessionUp(), false, false), CastItoXBytes(client->GetQueueSessionPayloadUp(), false, false), CastItoXBytes(client->GetPayloadInBuffer()), client->GetNumberOfRequestedBlocksInQueue(), (theApp.sharedfiles->GetFileByID(client->GetUploadFileID())?theApp.sharedfiles->GetFileByID(client->GetUploadFileID())->GetFileName():_T("")));
            client->m_dwWouldHaveGottenUploadSlotIfNotLowIdTick = 0;
			client->UnscheduleForRemoval();
//ZZUL -
			uploadinglist.RemoveAt(curPos);
//ZZUL +
            if(!earlyabort)
                client->SetWaitStartTime();
//ZZUL -
            bool removed = theApp.uploadBandwidthThrottler->RemoveFromStandardList(client->socket);
            //bool pcRemoved = theApp.uploadBandwidthThrottler->RemoveFromStandardList((CClientReqSocket*)client->m_pPCUpSocket);// X: [RPC] - [Remove PeerCache]
			(void)removed;
			//(void)pcRemoved; // X: [RPC] - [Remove PeerCache]
            //if(thePrefs.GetLogUlDlEvents() && !(removed || pcRemoved)) {
            //    AddDebugLogLine(false, _T("UploadQueue: Didn't find socket to delete. Adress: 0x%x"), client->socket);
            //}

            // ZZUL-TRA :: PaybackFirst :: Start
			//client normal leave the upload queue, check does client still satisfy requirement
			if(!earlyabort && client->Credits() != NULL)
				client->Credits()->InitPayBackFirstStatus();
			// ZZUL-TRA :: PaybackFirst :: End
//ZZUL +
			if(client->GetQueueSessionUp() > 0){
				++successfullupcount;
                theStats.IncTotalCompletedBytes(client->GetQueueSessionUp());
//ZZUL -
			    if(client->GetSessionUp() > 0) {
#ifdef CLIENTANALYZER
				if(client->pAntiLeechData)
					client->pAntiLeechData->AddULSession(false);
				++successfullupcount;
#endif
				totaluploadtime += client->GetUpStartTimeDelay()/1000;
             }
          } else if(earlyabort == false)
#ifdef CLIENTANALYZER
                      {
				if(client->pAntiLeechData)
					client->pAntiLeechData->AddULSession(true);
#endif
				++failedupcount;
#ifdef CLIENTANALYZER
			}
#endif

            CKnownFile* requestedFile = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
            if(requestedFile != NULL) {
                requestedFile->UpdatePartsInfo();
            }
			theApp.clientlist->AddTrackClient(client); // Keep track of this client
			client->SetUploadState(US_NONE);
			client->ClearUploadBlockRequests();
			client->SetCollectionUploadSlot(false);

            m_iHighestNumberOfFullyActivatedSlotsSinceLastCall = 0;

			result = true;
        } else {
            curClient->SetSlotNumber(slotCounter);
            slotCounter++;
        }
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
	m_bStatisticsWaitingListDirty = true;
	CUpDownClient* todelete = waitinglist.GetAt(pos);

	// ZZUL-TRA :: SeeOnQueue :: Start
	CKnownFile* reqfile = theApp.sharedfiles->GetFileByID(todelete->GetOldUploadFileID());
	if(reqfile)
		reqfile->RemoveOnUploadqueue();
	else {
		reqfile = theApp.knownfiles->FindKnownFileByID(todelete->GetOldUploadFileID());
		if(reqfile)
			reqfile->RemoveOnUploadqueue(); //we unshared the file, but sub regardless of this fact, because he can share it later again
	}
	// ZZUL-TRA :: SeeOnQueue :: End

	waitinglist.RemoveAt(pos);
	if (updatewindow) {
		theApp.emuledlg->transferwnd->GetQueueList()->RemoveClient(todelete);
		//theApp.emuledlg->transferwnd->ShowQueueCount(waitinglist.GetCount());
		theApp.emuledlg->transferwnd->ShowQueueCount(); // X: [SFH] - [Show IP Filter Hits]
	}
	todelete->SetUploadState(US_NONE);
	todelete->m_dwWouldHaveGottenUploadSlotIfNotLowIdTick = false; //netfinity: Moved here to be sure it's always cleared!
}

void CUploadQueue::UpdateMaxClientScore()
{
	m_imaxscore=0;
	for(POSITION pos = waitinglist.GetHeadPosition(); pos != 0; ) {
		uint32 score = waitinglist.GetNext(pos)->GetScore(true, false);
		if(score > m_imaxscore )
			m_imaxscore=score;
	}
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
	for (POSITION pos = waitinglist.GetHeadPosition(); pos != 0; ){
//ZZUL +
       CUpDownClient* compareClient = waitinglist.GetNext(pos);
		if (RightClientIsBetter(client, myscore, compareClient, compareClient->GetScore(false)))
//ZZUL -
			rank++;
	}
	return rank;
}

VOID CALLBACK CUploadQueue::UploadTimer(HWND /*hwnd*/, UINT /*uMsg*/, UINT_PTR /*idEvent*/, DWORD /*dwTime*/)
{
	// NOTE: Always handle all type of MFC exceptions in TimerProcs - otherwise we'll get mem leaks
	try
	{
		// Barry - Don't do anything if the app is shutting down - can cause unhandled exceptions
		if (!theApp.emuledlg->IsRunning())
			return;

		const DWORD curTick = ::GetTickCount(); // ZZUL-TRA :: SysInfo

        // Elandal:ThreadSafeLogging -->
        // other threads may have queued up log lines. This prints them.
        theApp.HandleDebugLogQueue();
        theApp.HandleLogQueue();
        // Elandal: ThreadSafeLogging <--

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

        theApp.uploadqueue->Process();
		theApp.downloadqueue->Process();
		if (thePrefs.ShowOverhead()){
			theStats.CompUpDatarateOverhead();
			theStats.CompDownDatarateOverhead();
		}
		counter++;

		// one second
		if (counter >= 10){
			counter=0;

// ZZUL-TRA :: SysInfo :: Start
			//update every second... this saves the timers and we can still handle different update intervals 
			//by using the passed "curTick" - if necessary, that is :)
			theApp.emuledlg->transferwnd->ShowRessources(curTick); 
// ZZUL-TRA :: SysInfo :: End

			// try to use different time intervals here to not create any disk-IO bottle necks by saving all files at once
			theApp.clientcredits->Process();	// 13 minutes
			theApp.serverlist->Process();		// 17 minutes
			theApp.knownfiles->Process();		// 11 minutes
#ifdef CLIENTANALYZER
			theApp.antileechlist->Process();	// 18 minutes  //>>> WiZaRd::ClientAnalyzer
#endif
			theApp.friendlist->Process();		// 19 minutes
			theApp.clientlist->Process();
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

			if (theApp.serverconnect->IsConnecting())
				theApp.serverconnect->CheckForTimeout();

			// 2 seconds
			i2Secs++;
			if (i2Secs>=2) {
				i2Secs=0;
				
				// Update connection stats...
				theStats.UpdateConnectionStats((float)theApp.uploadqueue->GetDatarate()/1024, (float)theApp.downloadqueue->GetDatarate()/1024);

#ifdef HAVE_WIN7_SDK_H
				if (thePrefs.IsWin7TaskbarGoodiesEnabled())
					theApp.emuledlg->UpdateStatusBarProgress();
#endif

			}

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

            theApp.uploadqueue->UpdateDatarates();
            
            //save rates every second
			theStats.RecordRate();

			// ZZ:UploadSpeedSense -->
            theApp.emuledlg->ShowPing();

            bool gotEnoughHosts = theApp.clientlist->GiveClientsForTraceRoute();
            if(gotEnoughHosts == false) {
                theApp.serverlist->GiveServersForTraceRoute();
            }
			// ZZ:UploadSpeedSense <--

			if (theApp.emuledlg->IsTrayIconToFlash())
				theApp.emuledlg->ShowTransferRate(true);

              if (theApp.m_pSpeedGraphWnd && theApp.m_pSpeedGraphWnd->IsWindowVisible()) // X: [SGW] - [SpeedGraphWnd]
			theApp.m_pSpeedGraphWnd->Update_TrafficGraph();

            sec++;
			// *** 5 seconds **********************************************
			if (sec >= 5) {
#ifdef _DEBUG
				if (thePrefs.m_iDbgHeap > 0 && !AfxCheckMemory())
					AfxDebugBreak();
#endif

				sec = 0;
				theApp.listensocket->Process();
				theApp.OnlineSig(); // Added By Bouc7 
				if (!theApp.emuledlg->IsTrayIconToFlash())
					theApp.emuledlg->ShowTransferRate();

				thePrefs.EstimateMaxUploadCap(theApp.uploadqueue->GetDatarate()/1024);
				// ZZUL-TRA :: FullChunk :: Start
				/*
				if (!thePrefs.TransferFullChunks())
					theApp.uploadqueue->UpdateMaxClientScore();
				*/
				// ZZUL-TRA :: FullChunk :: End

				// update cat-titles with downloadinfos only when needed
				if (thePrefs.ShowCatTabInfos() && 
					theApp.emuledlg->activewnd == theApp.emuledlg->transferwnd && 
					theApp.emuledlg->IsWindowVisible()) 
						theApp.emuledlg->transferwnd->UpdateCatTabTitles(false);
					theApp.downloadqueue->ProcessQuickStart(); // NEO: QS - [QuickStart] <-- Xanatos --
				
				if (thePrefs.IsSchedulerEnabled())
					theApp.scheduler->Check();

                theApp.emuledlg->transferwnd->UpdateListCount(CTransferDlg::wnd2Uploading, -1);
			}

			statsave++;
			// *** 60 seconds *********************************************
			if (statsave >= 60) {
				statsave=0;

				if (thePrefs.GetWSIsEnabled())
					theApp.webserver->UpdateSessionCount();

				theApp.serverconnect->KeepConnectionAlive();

				if (thePrefs.GetPreventStandby())
					theApp.ResetStandByIdleTimer(); // Reset Windows idle standby timer if necessary
			}

			s_uSaveStatistics++;
			if (s_uSaveStatistics >= thePrefs.GetStatsSaveInterval())
			{
				s_uSaveStatistics = 0;
				thePrefs.SaveStats();
			}
		}

		// need more accuracy here. don't rely on the 'sec' and 'statsave' helpers.
		thePerfLog.LogSamples();
	}
	CATCH_DFLT_EXCEPTIONS(_T("CUploadQueue::UploadTimer"))
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

void CUploadQueue::UpdateDatarates() {
    // Calculate average datarate
    if(::GetTickCount()-m_lastCalculatedDataRateTick > 500) {
        m_lastCalculatedDataRateTick = ::GetTickCount();

        if(avarage_dr_list.GetSize() >= 2 && (avarage_tick_list.GetTail() > avarage_tick_list.GetHead())) {
	        datarate = (UINT)(((m_avarage_dr_sum - avarage_dr_list.GetHead())*1000) / (avarage_tick_list.GetTail() - avarage_tick_list.GetHead()));
            friendDatarate = (UINT)(((avarage_friend_dr_list.GetTail() - avarage_friend_dr_list.GetHead())*1000) / (avarage_tick_list.GetTail() - avarage_tick_list.GetHead()));
//powershare graph +
     powershareDatarate = (UINT)(((avarage_powershare_dr_list.GetTail() - avarage_powershare_dr_list.GetHead())*1000) / (avarage_tick_list.GetTail() - avarage_tick_list.GetHead()));
 //powershare graoh -
        }
    }
}

uint32 CUploadQueue::GetDatarate() {
    return datarate;
}

//powershare graph +
uint32 CUploadQueue::GetDatarateExcludingPowershare() {
	if(datarate > powershareDatarate) {
		return datarate - powershareDatarate;
	} else {
		return 0;
	}
}
//powershare graph -

uint32 CUploadQueue::GetToNetworkDatarate() {
    if(datarate > friendDatarate) {
        return datarate - friendDatarate;
    } else {
        return 0;
    }
}
//ZZUL +
uint32 CUploadQueue::GetWantedNumberOfTrickleUploads() {
    uint32 minNumber = MINNUMBEROFTRICKLEUPLOADS;

    if(minNumber < 1 && GetDatarate() >= 2*1024) {
        minNumber = 1;
    }

    return max((uint32)(GetEffectiveUploadListCount()*0.1), minNumber);
}
//ZZUL -

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
            //theApp.uploadBandwidthThrottler->RemoveFromStandardList((CClientReqSocket*)cur_client->m_pPCUpSocket);// X: [RPC] - [Remove PeerCache]

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
//ZZUL +
void CUploadQueue::CheckForHighPrioClient() {
    // PENDING: Each 3 seconds
    DWORD curTick = ::GetTickCount();
    if(curTick - m_dwLastCheckedForHighPrioClient >= 3*1000) {
        m_dwLastCheckedForHighPrioClient = curTick;

        bool added = true;
        while(added) {
            added = AddUpNextClient(_T("High prio client (i.e. friend/powershare)."), NULL, true);
        }
	}
}
//ZZUL -

#ifdef HIGHRES
void  CUploadQueue::UseHighSpeedUploadTimer(bool bEnable)
{
	if (!bEnable)
	{
		if (m_hHighSpeedUploadTimer != 0)
		{
			KillTimer(0, m_hHighSpeedUploadTimer);
			m_hHighSpeedUploadTimer = 0;
		}
	}
	else
	{
		if (m_hHighSpeedUploadTimer == 0)
			VERIFY( (m_hHighSpeedUploadTimer = SetTimer(0 ,0 , 1, HSUploadTimer)) != 0 );
	}
	DebugLog(_T("%s HighSpeedUploadTimer"), bEnable ? _T("Enabled") : _T("Disabled"));
}

VOID CALLBACK CUploadQueue::HSUploadTimer(HWND /*hwnd*/, UINT /*uMsg*/, UINT_PTR /*idEvent*/, DWORD /*dwTime*/)
{
	// this timer is called every millisecond
	// all we do is feed the uploadslots with data, which is normally done only every 100ms with the big timer
	// the counting, checks etc etc are all done on the normal timer
	// the biggest effect comes actually from the BigBuffer parameter on CreateNextBlockPackage, 
	// but beeing able to fetch a request packet up to 1/10 sec earlier gives also a slight speedbump
	for (POSITION pos = theApp.uploadqueue->uploadinglist.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* cur_client = theApp.uploadqueue->uploadinglist.GetNext(pos);
		if (cur_client->socket != NULL)
            cur_client->CreateNextBlockPackage(true);
	}
}
#endif

uint32 CUploadQueue::GetWaitingUserForFileCount(const CSimpleArray<CObject*>& raFiles, bool bOnlyIfChanged)
{
	if (bOnlyIfChanged && !m_bStatisticsWaitingListDirty)
		return (uint32)-1;

	m_bStatisticsWaitingListDirty = false;
	uint32 nResult = 0;
	for (POSITION pos = waitinglist.GetHeadPosition(); pos != 0; )
	{
		const CUpDownClient* cur_client = waitinglist.GetNext(pos);
		for (int i = 0; i < raFiles.GetSize(); i++)
		{
			if (md4cmp(((CKnownFile*)raFiles[i])->GetFileHash(), cur_client->GetUploadFileID()) == 0)
				nResult++;
		}
	}
	return nResult;
}

uint32 CUploadQueue::GetDatarateForFile(const CSimpleArray<CObject*>& raFiles) const
{
	uint32 nResult = 0;
	for (POSITION pos = uploadinglist.GetHeadPosition(); pos != 0; )
	{
		const CUpDownClient* cur_client = uploadinglist.GetNext(pos);
		for (int i = 0; i < raFiles.GetSize(); i++)
		{
			if (md4cmp(((CKnownFile*)raFiles[i])->GetFileHash(), cur_client->GetUploadFileID()) == 0)
				nResult += cur_client->GetDatarate();
		}
	}
	return nResult;
}