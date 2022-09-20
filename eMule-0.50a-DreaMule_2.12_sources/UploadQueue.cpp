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
//#include "LastCommonRouteFinder.h" //Xman
#include "DownloadQueue.h"
#include "FriendList.h"
#include "Statistics.h"
//#include "MMServer.h"
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
//Xman
#include "Bandwidthcontrol.h"
#include <math.h>
#include "IPFilter.h" //Xman dynamic IP-Filters
#include "PartFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/* Xman moved down
static uint32 counter, sec, statsave;
static UINT _uSaveStatistics = 0;
// -khaos--+++> Added iupdateconnstats...
static uint32 igraph, istats, iupdateconnstats;
// <-----khaos-
*/

CUploadQueue::CUploadQueue()
{
	VERIFY( (h_timer = SetTimer(0,0,TIMER_PERIOD,UploadTimer)) != NULL ); //Xman variable timer-period
	if (thePrefs.GetVerbose() && !h_timer)
		AddDebugLogLine(true,_T("Failed to create 'upload queue' timer - %s"),GetErrorMessage(GetLastError()));
	//datarate = 0;
	//counter=0;
	successfullupcount = 0;
	failedupcount = 0;
	totaluploadtime = 0;
	m_nLastStartUpload = 0;
	m_dwRemovedClientByScore = ::GetTickCount();
    //Xman
	//m_iHighestNumberOfFullyActivatedSlotsSinceLastCall = 0;
    //m_MaxActiveClients = 0;
    //m_MaxActiveClientsShortTime = 0;

	//Xman
    //m_lastCalculatedDataRateTick = 0;
    //m_avarage_dr_sum = 0;
    //friendDatarate = 0;

	//Xman Xtreme Upload
	waituntilnextlook=0;
	dataratestocheck=10;
	currentuploadlistsize=0; //Xman x4
	internetmaybedown=true; //Xman x4
	checkforuploadblock=true; //Xman 4.4 enable the feature the check for too many too slow clients.
	m_dwnextallowedscoreremove=0;

    m_dwLastResortedUploadSlots = 0;
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
//Xman Code Improvement
CUpDownClient* CUploadQueue::FindBestClientInQueue()
{
	POSITION toadd = 0;
	uint32	bestscore = 0;
	uint32  bestlowscore = 0;
    CUpDownClient* lowclient = NULL;
	uint32 thisTick = ::GetTickCount(); //cache the value

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
			cur_client->isupprob=false;	//Xman uploading problem client
			RemoveFromWaitingQueue(pos2,true);
			continue;
		}
        else
        {
		    // finished clearing
		    uint32 cur_score = cur_client->GetScore(false);

		    if ( cur_score > bestscore)
		    {
                // cur_client is more worthy than current best client that is ready to go (connected).
                if((!cur_client->HasLowID() && cur_client->isupprob==false) || (cur_client->socket && cur_client->socket->IsConnected())) //Xman uploading problem client
				{
                    // this client is a HighID or a lowID client that is ready to go (connected)
                    // and it is more worthy
					if ((thisTick - cur_client->GetLastUpRequest()< 1800000) //Xman accept only clients which asked the last 30 minutes:
						&& cur_client->GetLastAction()==OP_STARTUPLOADREQ) //Xman fix for startupload
					{
					bestscore = cur_score;
			        toadd = pos2;
					}
                }
				else if(!cur_client->m_bAddNextConnect)
				{
                    // this client is a lowID client that is not ready to go (not connected)

                    // now that we know this client is not ready to go, compare it to the best not ready client
                    // the best not ready client may be better than the best ready client, so we need to check
                    // against that client
			        if (cur_score > bestlowscore)
			        {
                        // it is more worthy, keep it
				        bestlowscore = cur_score;
                        lowclient = waitinglist.GetAt(pos2);
			        }
                }
            }
		}
	}

	if (bestlowscore > bestscore && lowclient)
		lowclient->m_bAddNextConnect = true;

    if (!toadd)
		return NULL;
    else
	    return waitinglist.GetAt(toadd);
}
//Xman end

void CUploadQueue::InsertInUploadingList(CUpDownClient* newclient)
{
	//Lets make sure any client that is added to the list has this flag reset!
	newclient->m_bAddNextConnect = false;



	//Xman Xtreme Upload
	//Full Power for friendslots
	if (newclient->GetFriendSlot())
	{
		theApp.uploadBandwidthThrottler->AddToStandardList(true, newclient->GetFileUploadSocket());
		theApp.uploadBandwidthThrottler->RecalculateOnNextLoop();
	}
	else
	{
		// Add it last
		theApp.uploadBandwidthThrottler->AddToStandardList(false, newclient->GetFileUploadSocket());
		if(uploadinglist.GetCount()==0)
			theApp.uploadBandwidthThrottler->RecalculateOnNextLoop();
	}
	//Xman end
	uploadinglist.AddTail(newclient);
    //Xman
	//newclient->SetSlotNumber(uploadinglist.GetCount());
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
			lastupslotHighID = true; //Xman Xtreme Upload
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
	if (!newclient->socket || !newclient->socket->IsConnected() || !newclient->CheckHandshakeFinished()) //Xman Fix Connection Collision (Sirob)
	{
		newclient->SetUploadState(US_CONNECTING);
		if (!newclient->TryToConnect(true))
			return false;
	}
	else
	{
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__AcceptUploadReq", newclient);
		Packet* packet = new Packet(OP_ACCEPTUPLOADREQ,0);
		theStats.AddUpDataOverheadFileRequest(packet->size);
		newclient->socket->SendPacket(packet,true);
		newclient->SetUploadState(US_UPLOADING);
		//Xman find best sources
		//Xman: in every case, we add this client to our downloadqueue
		CKnownFile* partfile = theApp.downloadqueue->GetFileByID(newclient->GetUploadFileID());
		if (partfile && partfile->IsPartFile())
			theApp.downloadqueue->CheckAndAddKnownSource((CPartFile*)partfile,newclient, true);
		//Xman end
	}
	newclient->SetUpStartTime();
	newclient->ResetSessionUp();

	//Xman uploading problem client
	if(newclient->isupprob)
	{
		failedupcount--;
		CUpDownClient::SubUploadSocketStopCount();
	}
	newclient->isupprob=false;
	//Xman end


    InsertInUploadingList(newclient);

    m_nLastStartUpload = ::GetTickCount();

	// statistic
	CKnownFile* reqfile = theApp.sharedfiles->GetFileByID(newclient->GetUploadFileID());
	if (reqfile)
		reqfile->statistic.AddAccepted();

	theApp.emuledlg->transferwnd->uploadlistctrl.AddClient(newclient);

	return true;
}

/* //Xman Xtreme upload
void CUploadQueue::UpdateActiveClientsInfo(DWORD curTick) {
    // Save number of active clients for statistics
    uint32 tempHighest = theApp.uploadBandwidthThrottler->GetHighestNumberOfFullyActivatedSlotsSinceLastCallAndReset();

    if(thePrefs.GetLogUlDlEvents() && theApp.uploadBandwidthThrottler->GetStandardListSize() > (uint32)uploadinglist.GetSize()) {
        // debug info, will remove this when I'm done.
        //AddDebugLogLine(false, _T("UploadQueue: Error! Throttler has more slots than UploadQueue! Throttler: %i UploadQueue: %i Tick: %i"), theApp.uploadBandwidthThrottler->GetStandardListSize(), uploadinglist.GetSize(), ::GetTickCount());

		if(tempHighest > (uint32)uploadinglist.GetSize()) {
        	tempHighest = uploadinglist.GetSize();
		}
    }

    m_iHighestNumberOfFullyActivatedSlotsSinceLastCall = tempHighest;

    // save 15 minutes of data about number of fully active clients
    uint32 tempMaxRemoved = -1;
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

        if(tempMaxRemoved > m_MaxActiveClients) {
            m_MaxActiveClients = tempMaxActiveClients;
        }

        m_MaxActiveClientsShortTime = tempMaxActiveClientsShortTime;
    } else {
        m_MaxActiveClients = m_iHighestNumberOfFullyActivatedSlotsSinceLastCall;
        m_MaxActiveClientsShortTime = m_iHighestNumberOfFullyActivatedSlotsSinceLastCall;
    }
}
*/
/**
 * Maintenance method for the uploading slots. It adds and removes clients to the
 * uploading list. It also makes sure that all the uploading slots' Sockets always have
 * enough packets in their queues, etc.
 *
 * This method is called approximately once every 100 milliseconds.
 */
void CUploadQueue::Process() {

    //DWORD curTick = ::GetTickCount();

    //UpdateActiveClientsInfo(curTick); Xman Xtreme Upload

	if (ForceNewClient()){
        // There's not enough open uploads. Open another one.
        AddUpNextClient(_T("Not enough open upload slots for current ul speed"));
	}

    // The loop that feeds the upload slots with data.
	POSITION pos = uploadinglist.GetHeadPosition();
	while(pos != NULL){
        // Get the client. Note! Also updates pos as a side effect.
		CUpDownClient* cur_client = uploadinglist.GetNext(pos);
		if (thePrefs.m_iDbgHeap >= 2)
			ASSERT_VALID(cur_client);
		//It seems chatting or friend slots can get stuck at times in upload.. This needs looked into..
		if (!cur_client->socket)
		{
			RemoveFromUploadQueue(cur_client, _T("Uploading to client without socket? (CUploadQueue::Process)"),CUpDownClient::USR_SOCKET); // Maella -Upload Stop Reason-
			if(cur_client->Disconnected(_T("CUploadQueue::Process"))){
				delete cur_client;
			}
		} else {
            cur_client->SendBlockData();
        }
	}
};

bool CUploadQueue::AcceptNewClient(bool addOnNextConnect)
{


	//Xman Xtreme Upload

	//uint16 MinSlots=(uint16)ceil((thePrefs.GetMaxUpload()-0.5f)/thePrefs.m_slotspeed +1 );
	uint16 MinSlots=(uint16)ceil(thePrefs.GetMaxUpload()/thePrefs.m_slotspeed  );
	if(MinSlots<3) MinSlots=3;
	uint16 MaxSlots=0;

	if(thePrefs.m_slotspeed>6)
		MaxSlots=max(MaxSlots,(uint16)ceil(thePrefs.GetMaxUpload()/4));
	else 	if(thePrefs.m_slotspeed>4)
		MaxSlots=max(MaxSlots,(uint16)ceil(thePrefs.GetMaxUpload()/3));
	else
		MaxSlots=max((uint16)ceil(MinSlots*1.33),(uint16)ceil(thePrefs.m_slotspeed) + MinSlots);


	uint16 curUpSlots = (uint16)uploadinglist.GetCount();

	//Xman only one slot if maybe no internetconnection
	if(internetmaybedown && curUpSlots>=1)
		return false;

	//Xman count the blocksend to remove such clients if needed
	//Xman count block/success send
	if(curUpSlots>=MaxSlots)
	{
		//there are many clients out there, which can't take a high slotspeed (often less than 1kbs)
		//in worst case out upload is full of them and because no new slots are opened
		//our over all upload decrease
		//why should we keep such bad clients ? we keep it only if we have enough slots left
		//if our slot-max is reached, we drop the most blocking client
		if(theApp.uploadBandwidthThrottler->needslot==true && checkforuploadblock)
		{
			uint32 maxblock=0;
			CUpDownClient* blockclient=NULL;
			bool allfull=true;
			for(POSITION pos=uploadinglist.GetHeadPosition();pos!=NULL;)
			{
				CUpDownClient* cur_client=uploadinglist.GetNext(pos);
				if(cur_client->GetFileUploadSocket()->IsFull()==false)
				{
					allfull=false;
					break;
				}
				else
				{
					if(cur_client->GetFileUploadSocket()->blockedsendcount_overall > maxblock)
					{
						maxblock=cur_client->GetFileUploadSocket()->blockedsendcount_overall;
						blockclient=cur_client;
					}
				}
			}
			if(allfull && blockclient!=NULL && blockclient->GetUpStartTimeDelay()>MIN2MS(15))
			{
				RemoveFromUploadQueue(blockclient,_T("client is blocking too often and max slots are reached")); //remark: uploadstopreason: other
				m_blockstoplist.AddHead(::GetTickCount()); //Xman 4.4 remember when this happend.
				//because there are some users out, which set a too high uploadlimit, this code isn't useable
				//we deactivate it and warn the user
				if(m_blockstoplist.GetCount()>=6) //5 old + one new element
				{
					uint32 oldestblocktime=m_blockstoplist.GetAt(m_blockstoplist.FindIndex(5)); // the 6th element
					if(::GetTickCount() - oldestblocktime<HR2MS(1))
					{
						//5 block-drops during 1 hour is too much->warn the user and disable the feature
						LogWarning(GetResString(IDS_UPLOADINSTABLE));
						checkforuploadblock=false;
					}
					m_blockstoplist.RemoveTail();
				}
			}
			theApp.uploadBandwidthThrottler->SetNoNeedSlot(); //this can occur after increasing slotspeed
		}
		//remark: we always return false here... also on removing blockclient
		//next loop a new client will be accepted
		return false;
	}
	else
	{
		static uint32 lastblockingcheck;
		if(thePrefs.DropBlockingSockets() && curUpSlots > MinSlots +2 && lastblockingcheck + 1000 < ::GetTickCount() && theApp.uploadBandwidthThrottler->GetAvgHealth() >= 100)
		{
			lastblockingcheck=::GetTickCount();
			float ratioreference=96.0f;
			if(thePrefs.GetMaxUpload()>=100.0f && thePrefs.m_slotspeed >=10.0f) ratioreference=97.0f;
			//search a socket we should remove
			for(POSITION pos=uploadinglist.GetHeadPosition();pos!=NULL;)
			{
				CUpDownClient* cur_client=uploadinglist.GetNext(pos);
				if(cur_client->GetUpStartTimeDelay()>MIN2MS(3) //this client is already 5 minutes uploading.
					&& cur_client->GetFileUploadSocket()->GetBlockRatio_overall() >= 95.0f //95% of all send were blocked
					&& cur_client->GetFileUploadSocket()->GetBlockRatio() >= ratioreference //96% the last 20 seconds
					)
				{
					CString buffer;
					buffer.Format(_T("client is blocking too often: avg20: %0.0f%%, all: %0.0f%%, avg-speed: %0.0f B/s"),cur_client->GetFileUploadSocket()->GetBlockRatio(),cur_client->GetFileUploadSocket()->GetBlockRatio_overall(),(float)cur_client->GetSessionUp()/cur_client->GetUpStartTimeDelay()*1000.0f );
					RemoveFromUploadQueue(cur_client,buffer,CUpDownClient::USR_BLOCKING); // Maella -Upload Stop Reason-
					break; //only one socket
				}
			}
		}

	}
	//Xman end

	//Xman 4.8: if openmoreslots isn't checkt, be more dynamic with the slots... many users have high overhead with small bandwidth.
	//better we use less trickles

	if(thePrefs.m_openmoreslots==false && curUpSlots<MinSlots)
		return true;
	else if(curUpSlots < MinSlots/2)
		return true;


	if(addOnNextConnect
		&& ((thePrefs.m_openmoreslots //if openmoreslots, then it is allowed if only one trickle
		&& (curUpSlots-theApp.uploadBandwidthThrottler->GetNumberOfFullyActivatedSlots()<=1) )
		|| (thePrefs.m_openmoreslots==false && curUpSlots<=MinSlots) //else it is allowed if only minslots
		|| lastupslotHighID == true)) //or last client was highid
		return true;


	static uint32 lastchecktime;
	const uint32 thisTick=::GetTickCount();

	if (thePrefs.m_openmoreslots &&  (thisTick - lastchecktime >=1000) &&  (thisTick - m_nLastStartUpload >= 1000) && theApp.uploadBandwidthThrottler->needslot==true)
	{
		lastchecktime=thisTick;
		bool allready=true;
		for(POSITION pos=uploadinglist.GetTailPosition();pos!=NULL;)
		{
			CUpDownClient* client=uploadinglist.GetPrev(pos);
			if(client->GetFileUploadSocket()->isready==false && client->GetUpStartTimeDelay() >7000)
			{
				//client isn't responding for >7 sec -->new slot
				break;
			}
			if(client->GetFileUploadSocket()->IsFull()==false && client->GetFileUploadSocket()->isready==false)
			{
				allready=false;
				break;
			}
		}
		return allready;
	}


	return false;


}

bool CUploadQueue::ForceNewClient(bool allowEmptyWaitingQueue) {
    //Xman x4
	currentuploadlistsize=(uint16)uploadinglist.GetCount();

	if(!allowEmptyWaitingQueue && waitinglist.GetSize() <= 0)
        return false;

	//Xman Xtreme Upload

	static uint32 lastchecktimefull;
	const uint32 thisTick=::GetTickCount();
	if (thisTick - lastchecktimefull <500) //only every 500ms, not to suck too much CPU
		return false;
	lastchecktimefull=thisTick;

    //Xman only if we have an internet connection
	//I test this, if we have a serverconnect and outgoing traffic

	// Compute all datarates elapsed for the last 1 seconds
	uint32 eMuleIn;	uint32 eMuleInOverall;
	uint32 eMuleOut; uint32 eMuleOutOverall;
	uint32 networkIn; uint32 networkOut;

	//if (theApp.serverconnect->IsConnected()==false)
	//{
		theApp.pBandWidthControl->GetDatarates(thePrefs.m_internetdownreactiontime, // 2 seconds
			eMuleIn, eMuleInOverall,
			eMuleOut, eMuleOutOverall,
			networkIn, networkOut);

		//Xman check out if eventually we don't have an internet-connection
		if(eMuleOut==0)
			internetmaybedown=true;
		else
			internetmaybedown=false;

		if(theApp.IsConnected()==false)
		{
			if(eMuleOut==0)
				return false;
		}

	//}
	//else
	//	internetmaybedown=false;


	return AcceptNewClient();

}

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

//Xman uploading problem client
void CUploadQueue::AddClientDirectToQueue(CUpDownClient* client)
{
	CKnownFile* reqfile = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
	if(reqfile)
	{
		theApp.uploadqueue->waitinglist.AddTail(client);
		client->SetUploadState(US_ONUPLOADQUEUE);
		theApp.emuledlg->transferwnd->queuelistctrl.AddClient(client,false);
		theApp.emuledlg->transferwnd->ShowQueueCount(waitinglist.GetCount());
		//Xman see OnUploadqueue
		reqfile->AddOnUploadqueue();
		client->SetOldUploadFileID();
		//Xman end
	}
}
//Xman end


/**
 * Add a client to the waiting queue for uploads.
 *
 * @param client address of the client that should be added to the waiting queue
 *
 * @param bIgnoreTimelimit don't check time limit to possibly ban the client.
 */
//Xman Code Improvement
void CUploadQueue::AddClientToQueue(CUpDownClient* client, bool bIgnoreTimelimit)
{
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
	CKnownFile* reqfile = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
	if (!reqfile){
		AddDebugLogLine(false,_T("AddClientToQueue: Client is asking for a unknown file, %s"),client->DbgGetClientInfo());
		// send file request no such file packet (0x48)
		Packet* replypacket = new Packet(OP_FILEREQANSNOFIL, 16);
		md4cpy(replypacket->pBuffer, client->GetUploadFileID());
		theStats.AddUpDataOverheadFileRequest(replypacket->size);
		client->socket->SendPacket(replypacket, true);
		return;
	}

	client->AddAskedCount();
	client->SetLastUpRequest();
	if (!bIgnoreTimelimit)
		client->AddRequestCount(client->GetUploadFileID());
	if (client->IsBanned())
		return;

	//check completed sources which want to download their "complete" file
	if(client->GetRequestFile()==reqfile && client->HasFileComplete())
	{
		AddDebugLogLine(false, _T("->client want to download a file it has already complete: %s, %s"), reqfile->GetFileName(), client->DbgGetClientInfo());
		return; //no answer... a ban would also be not to bad
	}
	//Xman end

	uint16 cSameIP = 0;
	// check for double
	POSITION pos1, pos2;
	for (pos1 = waitinglist.GetHeadPosition();( pos2 = pos1 ) != NULL;)
	{
		waitinglist.GetNext(pos1);
		CUpDownClient* cur_client= waitinglist.GetAt(pos2);
		if (cur_client == client)
		{
			//Xman see OnUploadqueue
			//look if the client is now asking for another file
			CKnownFile* newreqfile = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
			CKnownFile* oldreqfile = theApp.sharedfiles->GetFileByID(client->GetOldUploadFileID());
			if(newreqfile != oldreqfile)
			{
				if(oldreqfile) oldreqfile->RemoveOnUploadqueue();
				newreqfile->AddOnUploadqueue();
				client->SetOldUploadFileID();
				newreqfile->statistic.AddRequest(); //Xman Bugfix of official client: official client doesn't count a request when a user swapped to other file
			}
			//Xman end

			if ((client->m_bAddNextConnect || client->GetFriendSlot()) && AcceptNewClient(true))
			{
				//Special care is given to lowID clients that missed their upload slot
				//due to the saving bandwidth on callbacks.
				/*if(thePrefs.GetLogUlDlEvents())
					AddDebugLogLine(true, _T("Adding ****lowid when reconnecting. Client: %s"), client->DbgGetClientInfo());
				*/
				//client->m_bAddNextConnect = false; //done in removefromwaitingqueue
				lastupslotHighID = false; //Xman Xtreme upload
				RemoveFromWaitingQueue(client, true);
				if (client->GetFriendSlot())
				{
					AddUpNextClient(_T("Adding friend on reconnect"), client);
				}
				else if (client->isupprob) //Xman uploading problem client
				{
					AddUpNextClient(_T("Adding ~~~problematic client (second change) on reconnect"),client);
				}
				else
					AddUpNextClient(_T("Adding ****lowid on reconnecting."), client);
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
					AddLeecherLogLine(false, GetResString(IDS_SAMEUSERHASH), client->GetUserName(), cur_client->GetUserName(),client->GetUserName() );
				return;
			}
			if (client->credits != NULL && client->credits->GetCurrentIdentState(client->GetIP()) == IS_IDENTIFIED)
			{
				//client has a valid secure hash, add him remove other one
				if (thePrefs.GetVerbose())
					AddLeecherLogLine(false, GetResString(IDS_SAMEUSERHASH), client->GetUserName(), cur_client->GetUserName(),cur_client->GetUserName() );
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
					AddLeecherLogLine(false, GetResString(IDS_SAMEUSERHASH), client->GetUserName(), cur_client->GetUserName(),_T("Both") );
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

	// statistic values
	reqfile->statistic.AddRequest();

	if (client->IsDownloading())
	{
		// he's already downloading and wants probably only another file
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__AcceptUploadReq", client);

		//Xman Close Backdoor v2
		//a downloading client can simply request an other file during downloading
		//this code checks the Up-Priority of the new request
		uint8 oldUpPrio= ((CKnownFile*)theApp.sharedfiles->GetFileByID((uchar*)client->GetOldUploadFileID()))->GetUpPriorityEx();
		uint8 newUpPrio= reqfile->GetUpPriorityEx();
		if(newUpPrio  < oldUpPrio)
		{
			if(thePrefs.GetLogUlDlEvents()){
				AddDebugLogLine(false, _T("--> Upload session ended due wrong requested FileID (in AddClientToQueue) (client=%s, expected=%s, asked=%s)"),
					client->GetUserName(),((CKnownFile*)theApp.sharedfiles->GetFileByID((uchar*)client->GetOldUploadFileID()))->GetFileName(), reqfile->GetFileName());
			}
			RemoveFromUploadQueue(client, _T("wrong file"), CUpDownClient::USR_DIFFERENT_FILE,true); // Maella -Upload Stop Reason-
			client->SendOutOfPartReqsAndAddToWaitingQueue();
			client->SetWaitStartTime(); // Penality (soft punishement)
			return;
		}
		//Xman end Close Backdoor v2

		Packet* packet = new Packet(OP_ACCEPTUPLOADREQ,0);
		theStats.AddUpDataOverheadFileRequest(packet->size);
		client->socket->SendPacket(packet,true);
		//AddDebugLogLine(false,_T("-->sending ACCEPTUPLOADREQ a second time: %s"), client->DbgGetClientInfo());
		return;
	}

	// emule collection will bypass the queue
	if (reqfile != NULL && reqfile->HasCollectionExtenesion_Xtreme() /*CCollection::HasCollectionExtention(reqfile->GetFileName())*/ && reqfile->GetFileSize() < (uint64)MAXPRIORITYCOLL_SIZE //Xman Code Improvement for HasCollectionExtention
		&& !client->IsDownloading() && client->socket != NULL && client->socket->IsConnected())
	{
		client->SetCollectionUploadSlot(true);
		RemoveFromWaitingQueue(client, true);
		client->SetOldUploadFileID(); //Xman Close Backdoor v2
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
        client->IsFriend()==false  && // client is not a friend with friend slot
        client->GetCombinedFilePrioAndCredit() < GetAverageCombinedFilePrioAndCredit()) { // and client has lower credits/wants lower prio file than average client in queue
			//Xman Queueoverflow Minimumcontingent
			uint16 mincontingent=(uint16)(thePrefs.GetQueueSize()/theApp.sharedfiles->GetCount()/2);
			if (reqfile->GetOnUploadqueue()> mincontingent)
				return; // then block client from getting on queue
			//Xman end
	}
	if (waitinglist.IsEmpty() && ForceNewClient(true))
	{
		client->SetOldUploadFileID(); //Xman Close Backdoor v2
		AddUpNextClient(_T("Direct add with empty queue."), client);
	}
	else
	{
		waitinglist.AddTail(client);
		client->SetUploadState(US_ONUPLOADQUEUE);
		theApp.emuledlg->transferwnd->queuelistctrl.AddClient(client,true);
		theApp.emuledlg->transferwnd->ShowQueueCount(waitinglist.GetCount());
		client->SendRankingInfo();
		//Xman see OnUploadqueue
		reqfile->AddOnUploadqueue(); //Do this only if requfile!!! (is asked above)
		client->SetOldUploadFileID();
		//Xman end
	}
}
//Xman end

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
//Xman Code improvement
bool CUploadQueue::RemoveFromUploadQueue(CUpDownClient* client, LPCTSTR pszReason, CUpDownClient::UpStopReason reason , bool updatewindow, bool earlyabort){ // Maella -Upload Stop Reason-
    bool result = false;
    //uint32 slotCounter = 1;
	POSITION pos = uploadinglist.Find(client);
	if(pos != NULL)
	{
			if (updatewindow)
				theApp.emuledlg->transferwnd->uploadlistctrl.RemoveClient(client);

			if (thePrefs.GetLogUlDlEvents())
                AddDebugLogLine(DLP_DEFAULT, true,_T("Removing client from upload list: %s Client: %s Transferred: %s SessionUp: %s QueueSessionPayload: %s In buffer: %s Req blocks: %i File: %s"), pszReason==NULL ? _T("") : pszReason, client->DbgGetClientInfo(), CastSecondsToHM( client->GetUpStartTimeDelay()/1000), CastItoXBytes(client->GetSessionUp(), false, false), CastItoXBytes(client->GetQueueSessionPayloadUp(), false, false), CastItoXBytes(client->GetPayloadInBuffer()), client->GetNumberOfRequestedBlocksInQueue(), (theApp.sharedfiles->GetFileByID(client->GetUploadFileID())?theApp.sharedfiles->GetFileByID(client->GetUploadFileID())->GetFileName():_T("")));
            client->m_bAddNextConnect = false;
			uploadinglist.RemoveAt(pos);

			//Xman Full Chunk
			//set the flag back
			client->upendsoon=false;

            bool removed = theApp.uploadBandwidthThrottler->RemoveFromStandardList(client->socket);
            bool pcRemoved = theApp.uploadBandwidthThrottler->RemoveFromStandardList((CClientReqSocket*)client->m_pPCUpSocket);
			(void)removed;
			(void)pcRemoved;
            //if(thePrefs.GetLogUlDlEvents() && !(removed || pcRemoved)) {
            //    AddDebugLogLine(false, _T("UploadQueue: Didn't find socket to delete. Adress: 0x%x"), client->socket);
            //}

			if(client->GetSessionUp() > 0) {
				++successfullupcount;
				totaluploadtime += client->GetUpStartTimeDelay()/1000;
				CUpDownClient::AddUpStopCount(false, reason); // Maella -Upload Stop Reason-
            } else if(earlyabort == false)
			{
				++failedupcount;
				CUpDownClient::AddUpStopCount(true, reason); // Maella -Upload Stop Reason-
			}

            CKnownFile* requestedFile = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
            if(requestedFile != NULL) {
                requestedFile->UpdatePartsInfo();
            }
			theApp.clientlist->AddTrackClient(client); // Keep track of this client
			client->SetUploadState(US_NONE);
			client->ClearUploadBlockRequests();
			client->SetCollectionUploadSlot(false);

            //Xman Xtreme Upload
			m_dwnextallowedscoreremove=0;

			result = true;
	}
	return result;
}
//Xman end

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
		if (updatewindow)
			theApp.emuledlg->transferwnd->ShowQueueCount(waitinglist.GetCount());
		//Xman Code Fix: wrong place, because some removing is done with position
		//client->m_bAddNextConnect = false;
		return true;
	}
	else
		return false;
}

void CUploadQueue::RemoveFromWaitingQueue(POSITION pos, bool updatewindow){
	CUpDownClient* todelete = waitinglist.GetAt(pos);

	//Xman see OnUploadqueue
	CKnownFile* reqfile=NULL;
	reqfile = theApp.sharedfiles->GetFileByID(todelete->GetOldUploadFileID());

	if (reqfile)
		reqfile->RemoveOnUploadqueue();
	else
	{
		reqfile=theApp.knownfiles->FindKnownFileByID(todelete->GetOldUploadFileID());
		if(reqfile)
		//we unshared the file, but sub regardless of this fact, because he can share it later again
		reqfile->RemoveOnUploadqueue();
	}
	//else
		//ASSERT(false); //we aborted this file
	//Xman end

	//Xman Code Fix
	todelete->m_bAddNextConnect = false; //right place

	waitinglist.RemoveAt(pos);
	if (updatewindow)
		theApp.emuledlg->transferwnd->queuelistctrl.RemoveClient(todelete);
	todelete->SetUploadState(US_NONE);
}
//Xman Xtreme Mod
void CUploadQueue::UpdateMaxClientScore()
{
	m_imaxscore=0;
	uint32 thisTick= ::GetTickCount(); //cache value
	for(POSITION pos = waitinglist.GetHeadPosition(); pos != 0; ) {
		CUpDownClient* cur_client =	waitinglist.GetNext(pos);
		uint32 score = cur_client->GetScore(true, false);
		if((thisTick - cur_client->GetLastUpRequest()< 1800000) //Xman accept only clients which asked the last 30 minutes:
			&& ((cur_client->isupprob==false) || (cur_client->socket && cur_client->socket->IsConnected())) //Xman uploading problem client
			&& cur_client->GetLastAction()==OP_STARTUPLOADREQ && //Xman fix for startupload
			score > m_imaxscore )
			m_imaxscore=score;
	}
}
//Xman end
//Xman Full Chunk
bool CUploadQueue::CheckForTimeOver(CUpDownClient* client){
	//If we have nobody in the queue, do NOT remove the current uploads..
	//This will save some bandwidth and some unneeded swapping from upload/queue/upload..
	if ( waitinglist.IsEmpty() || client->GetFriendSlot() )
		return false;

	if(client->HasCollectionUploadSlot()){
		CKnownFile* pDownloadingFile = theApp.sharedfiles->GetFileByID(client->requpfileid);
		if(pDownloadingFile == NULL)
			return true;
		if (pDownloadingFile->HasCollectionExtenesion_Xtreme() /*CCollection::HasCollectionExtention(pDownloadingFile->GetFileName())*/ && pDownloadingFile->GetFileSize() < (uint64)MAXPRIORITYCOLL_SIZE) //Xman Code Improvement for HasCollectionExtention
			return false;
		else{
			if (thePrefs.GetLogUlDlEvents())
				AddDebugLogLine(DLP_HIGH, false, _T("%s: Upload session ended - client with Collection Slot tried to request blocks from another file"), client->GetUserName());
			return true;
		}
	}


bool returnvalue=false;

    if( client->GetUpStartTimeDelay() > SESSIONMAXTIME){ // Try to keep the clients from downloading for ever
	    if (thePrefs.GetLogUlDlEvents())
		    AddDebugLogLine(DLP_LOW, false, _T("%s: Upload session will end soon due to max time %s."), client->GetUserName(), CastSecondsToHM(SESSIONMAXTIME/1000));
	    returnvalue=true;
    }


	//Xman: we allow a min of 1.8 MB
	if (!thePrefs.TransferFullChunks() && client->GetSessionUp() >= 1887437
		&& m_dwnextallowedscoreremove < ::GetTickCount() //Xman avoid to short upload-periods
		)
	{

		// Cache current client score
		const uint32 score = client->GetScore(true, true);

		// Check if another client has a bigger score
		//Xman max allowed are 10 MB
		if ((score < GetMaxClientScore() || client->GetSessionUp() >= 10485760) && m_dwRemovedClientByScore < GetTickCount())
		{
			if (thePrefs.GetLogUlDlEvents())
				AddDebugLogLine(DLP_VERYLOW, false, _T("%s: Upload session will end soon due to score."), client->GetUserName());
			//Set timer to prevent to many uploadslot getting kick do to score.
			//Upload slots are delayed by a min of 1 sec and the maxscore is reset every 5 sec.
			//So, I choose 6 secs to make sure the maxscore it updated before doing this again.
			m_dwRemovedClientByScore = GetTickCount()+SEC2MS(6);
			m_dwnextallowedscoreremove = ::GetTickCount() + MIN2MS(1);
			//Xman remark:
			//now we have 2 values. This is to avoid one special situation:
			//one client at waitingqueue get a very high score, because long waiting-time and now he is uploading to us.
			//because the client with lowest score is only kicked after he has completed it's emblock
			//this can need too long time, and a second or third client will also be kicked
			//now we wait, until the lowest score client has finish it's emblock, but max 1 minute
			//you can see m_dwRemovedClientByScore as the minimum time and
			//m_dwnextallowedscoreremove as the maximum time

			returnvalue=true;
		}
	}

	if(thePrefs.TransferFullChunks() && (client->IsDifferentPartBlock() || client->GetQueueSessionPayloadUp() > SESSIONMAXTRANS))
	{
		// Allow the client to download a specified amount per session
			if (thePrefs.GetLogUlDlEvents() && client->GetQueueSessionPayloadUp() > SESSIONMAXTRANS)
				AddDebugLogLine(DLP_DEFAULT, false, _T("%s: Upload session will end soon due to max transferred amount. %s"), client->GetUserName(), CastItoXBytes(SESSIONMAXTRANS, false, false));
			returnvalue=true;
	}


	if(returnvalue==true)
	{
		//client->upendsoon=true;
		//if we don't have enough slots after this clients ends, accept a new one
		if((uint16)uploadinglist.GetCount()-theApp.uploadBandwidthThrottler->GetNumberOfFullyActivatedSlots()<=1)
			AddUpNextClient(_T("Accept new client, because other client Upload end soon"));
	}

	return returnvalue;


}
//Xman end

void CUploadQueue::DeleteAll(){
	waitinglist.RemoveAll();
	uploadinglist.RemoveAll();
    // PENDING: Remove from UploadBandwidthThrottler as well!
}

// Maella -One-queue-per-file- (idea bloodymad)
UINT CUploadQueue::GetWaitingPosition(CUpDownClient* client){


	//Xman
	//MORPH - Changed by SiRoB, Optimization
	/*
	if (!IsOnUploadQueue(client))
	*/
	ASSERT((client->GetUploadState() == US_ONUPLOADQUEUE) == IsOnUploadQueue(client));
	if (client->GetUploadState() != US_ONUPLOADQUEUE)
		return 0;

	UINT rank = 1;
	const UINT myscore = client->GetScore(false);

	if(thePrefs.GetEnableMultiQueue() == false){
		for(POSITION pos = waitinglist.GetHeadPosition(); pos != NULL; ){
			if(waitinglist.GetNext(pos)->GetScore(false) > myscore)
				rank++;
		}
	}
	else {
		// Compare score only with others clients waiting for the same file
		for(POSITION pos = waitinglist.GetHeadPosition(); pos != NULL; ){
			CUpDownClient* pOtherClient = waitinglist.GetNext(pos);
			if(md4cmp(client->GetUploadFileID(), pOtherClient->GetUploadFileID()) == 0 &&
				pOtherClient->GetScore(false) > myscore){
					rank++;
				}
		}
	}
	return rank;
}
// Maella end


//Xman rework: + //Xman process timer code via messages (Xanatos)
VOID CALLBACK CUploadQueue::UploadTimer(HWND /*hwnd*/, UINT /*uMsg*/, UINT_PTR /*idEvent*/, DWORD /*dwTime*/)
{
	// NOTE: Always handle all type of MFC exceptions in TimerProcs - otherwise we'll get mem leaks
	try
	{
		theApp.emuledlg->PostMessage(TM_DOTIMER, NULL, NULL); //Xman process timer code via messages (Xanatos)
	}
    CATCH_DFLT_EXCEPTIONS(_T("CUploadQueue::UploadTimer"))
}

void CUploadQueue::UploadTimer() 
{
		// Barry - Don't do anything if the app is shutting down - can cause unhandled exceptions
		if (!theApp.emuledlg->IsRunning())
			return;

		//Xman
		// BEGIN SLUGFILLER: SafeHash - let eMule start first
		if (theApp.emuledlg->status != 255)
			return;
		// END SLUGFILLER: SafeHash

		//Xman 5.1
		//Xman skip High-CPU-Load
		static uint32 lastprocesstime;
		if(::GetTickCount() - lastprocesstime <=0)
			return;
		else
			lastprocesstime=::GetTickCount();

		static uint16 counter;


        // Elandal:ThreadSafeLogging -->
        // other threads may have queued up log lines. This prints them.
        theApp.HandleDebugLogQueue();
        theApp.HandleLogQueue();
        // Elandal: ThreadSafeLogging <--

		//Xman
		// ZZ:UploadSpeedSense -->
		/*
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
		*/
		// ZZ:UploadSpeedSense <--

		//Xman final version: 100ms are enough
		counter++;
		if(counter%(100/TIMER_PERIOD)==0)
		{
			theApp.uploadqueue->Process();
		}
		theApp.downloadqueue->Process();

		// need more accuracy here. don't rely on the 'sec' and 'statsave' helpers.
		thePerfLog.LogSamples();



		// 1 second clock (=> CPU load balancing)
		if(counter == (200/TIMER_PERIOD)){

			theApp.downloadqueue->CompDownloadRate(); // Update GUI for Download Queue
			theApp.uploadqueue->CompUploadRate(); // Calcule and refresh GUI

		}
		else if(counter == (400/TIMER_PERIOD)){

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
			if (thePrefs.WatchClipboard4ED2KLinks())
				theApp.SearchClipboard();

			if (theApp.serverconnect->IsConnecting())
				theApp.serverconnect->CheckForTimeout();

		}
		else if(counter == (600/TIMER_PERIOD)){

			theApp.clientlist->Process();
			if (theApp.emuledlg->IsTrayIconToFlash())
				theApp.emuledlg->ShowTransferRate(true);
			static int showRate; showRate++;
			if(showRate >= DISPLAY_REFRESH){
				showRate = 0;
				if (!theApp.emuledlg->IsTrayIconToFlash())
					theApp.emuledlg->ShowTransferRate(); // Update GUI control bar + icon tray
				theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Uploading, -1); // Update the shown amount of clients in lists
			}

		}
		else if(counter == (800/TIMER_PERIOD)){

			theApp.clientlist->CleanUpClientList(); //Maella -Extended clean-up II-
//remove moblie mule

		}
		else if(counter == (900/TIMER_PERIOD)){

			theApp.sharedfiles->Process();
			theApp.emuledlg->statisticswnd->Process(); // Record history of bandwidth + update statistic tree + update Graphs

		}
		else if (counter >= (1000/TIMER_PERIOD)){
			counter=0;


			// 5 seconds
			static uint16 sec; sec++;
			if (sec==1) {
#ifdef _DEBUG
				if (thePrefs.m_iDbgHeap > 0 && !AfxCheckMemory())
					AfxDebugBreak();
#endif

				theApp.listensocket->Process();
				theApp.clientcredits->Process(); // 13 minutes

			}
			else if (sec==2) {
				theApp.OnlineSig(); // Added By Bouc7
				theApp.friendlist->Process(); // 19 minutes
			}
			else if (sec==3) {
				theApp.ipfilter->Process(); //Xman dynamic IP-Filters
				theApp.serverlist->Process(); // 17 minutes
			}
			else if (sec==4) {
				theApp.knownfiles->Process(); // 11 minutes
				// update cat-titles with downloadinfos only when needed
				if (thePrefs.ShowCatTabInfos() &&
					theApp.emuledlg->activewnd == theApp.emuledlg->transferwnd &&
					theApp.emuledlg->IsWindowVisible())
					theApp.emuledlg->transferwnd->UpdateCatTabTitles(false);
			}
			else if (sec==5) {
				sec=0;
				if (!thePrefs.TransferFullChunks())
					theApp.uploadqueue->UpdateMaxClientScore();
				if (thePrefs.IsSchedulerEnabled())
					theApp.scheduler->Check();
			}

			/*
			// ZZ:UploadSpeedSense -->
            theApp.emuledlg->ShowPing();

            bool gotEnoughHosts = theApp.clientlist->GiveClientsForTraceRoute();
            if(gotEnoughHosts == false) {
                theApp.serverlist->GiveServersForTraceRoute();
            }
			// ZZ:UploadSpeedSense <--
			*/

			static uint16 statsave; statsave++;
			// 60 seconds
			if (statsave>=60) {
				statsave=0;
				if (thePrefs.GetWSIsEnabled())
					theApp.webserver->UpdateSessionCount();

				theApp.serverconnect->KeepConnectionAlive();
			}

			static UINT _uSaveStatistics; _uSaveStatistics++;
			if (_uSaveStatistics >= thePrefs.GetStatsSaveInterval())
			{
				_uSaveStatistics = 0;
				thePrefs.SaveStats();
			}

		}
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

//Xman Xtreme Upload: Peercache-part
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
			bool ret=false;
            ret=theApp.uploadBandwidthThrottler->RemoveFromStandardList(cur_client->socket);
			if(ret && cur_client->HasPeerCacheState())
				DEBUG_ONLY( Debug(_T("removed normal socket from uploadbandwidththrottler")));
            ret=theApp.uploadBandwidthThrottler->RemoveFromStandardList((CClientReqSocket*)cur_client->m_pPCUpSocket);
			if(ret && cur_client->HasPeerCacheState())
				DEBUG_ONLY( Debug( _T("removed m_pPCUpSocket from uploadbandwidththrottler")));


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

			//Xman only for debug
#ifdef _DEBUG
			if(cur_client->HasPeerCacheState())
				cur_client->GetFileUploadSocket(true);
#endif

        }
		theApp.uploadBandwidthThrottler->RecalculateOnNextLoop(); //Xman x4
        theApp.uploadBandwidthThrottler->Pause(false);
    }
}
//Xman Xtreme Upload: Peercache-part
//this method replaces the resort-function
void CUploadQueue::ReplaceSlot(CUpDownClient* client)
{


	if(!theApp.uploadBandwidthThrottler->ReplaceSocket(client->socket, (CClientReqSocket*)client->m_pPCUpSocket,client->GetFileUploadSocket()))
		ReSortUploadSlots(true);


}




// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
void CUploadQueue::CompUploadRate(){
	//Xman Xtreme Upload:
	//check if one slot is over tolerance and tell the throttler
	bool isovertolerance=false;
	// Compute the upload datarate of all clients
	for(POSITION pos = uploadinglist.GetHeadPosition(); pos != NULL; ){
		uploadinglist.GetAt(pos)->CompUploadRate();
		if(uploadinglist.GetNext(pos)->CheckDatarate(dataratestocheck))
			isovertolerance=true;
	}
	if(isovertolerance && waituntilnextlook==0)
	{
		waituntilnextlook=5; //5 seconds until we redo this test //Xman up to 5
		dataratestocheck=-1; //Xman don't check the first, but the next three
		theApp.uploadBandwidthThrottler->SetNextTrickleToFull();
	}
	if(waituntilnextlook>0)
		waituntilnextlook--;
	if(dataratestocheck<15)
		dataratestocheck++;
}
// Maella end

//Xman Xtreme Upload
void CUploadQueue::ChangeSendBufferSize(int newValue)
{
	for(POSITION pos=uploadinglist.GetHeadPosition(); pos!=NULL;)
	{
		CClientReqSocket* cur_socket=uploadinglist.GetNext(pos)->GetFileUploadSocket();
		if(cur_socket)
		{
			//int newValue = thePrefs.GetSendbuffersize(); // default: 8192;
			int setValue = 0;
			int size = sizeof(newValue);
			cur_socket->SetSockOpt(SO_SNDBUF, &newValue, sizeof(newValue), SOL_SOCKET);
			cur_socket->GetSockOpt(SO_SNDBUF, &setValue, &size, SOL_SOCKET);
			//AddDebugLogLine(false,_T("new socketbuffer: %u "), setValue);
		}
	}
}