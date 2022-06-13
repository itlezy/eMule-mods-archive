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
#include "TransferDlg.h"
#include "SearchDlg.h"
#include "StatisticsDlg.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "Kademlia/Kademlia/Prefs.h"
#include "Log.h"
//#include "collection.h" Anis -> non serve + 
#include "AdunanzA.h"
#include "PartFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//>>> Anis::Automatic Firewalled Retries
static	uint8	m_iFirewalledRetries = 0;
static	DWORD	m_dwFirewalledTimer = NULL;
//### Anis::Automatic Firewalled Retries


static uint32 counter, sec, statsave;
static UINT s_uSaveStatistics = 0;
static uint32 igraph, istats, i2Secs;

#define HIGHSPEED_UPLOADRATE_START 500*1024
#define HIGHSPEED_UPLOADRATE_END   300*1024


CUploadQueue::CUploadQueue()
{	
	m_uiPaybackCount = 0; //>>> Anis::Payback First
	VERIFY( (h_timer = SetTimer(0,0,100,UploadTimer)) != NULL );
	if (thePrefs.GetVerbose() && !h_timer)
		AddDebugLogLine(true,_T("Failed to create 'upload queue' timer - %s"),GetErrorMessage(GetLastError()));
	m_dwFirewalledTimer = ::GetTickCount(); //>>> Anis::Automatic Firewalled Retries
	datarate = 0;
	counter=0;
	successfullupcount = 0;
	failedupcount = 0;
	totaluploadtime = 0;
	m_nLastStartUpload = 0;
	statsave=0;
	i2Secs=0;
	m_dwRemovedClientByScore = ::GetTickCount();
    m_iHighestNumberOfFullyActivatedSlotsSinceLastCall = 0;
    m_MaxActiveClients = 0;
    m_MaxActiveClientsShortTime = 0;

    m_lastCalculatedDataRateTick = 0;
    m_avarage_dr_sum = 0;
    friendDatarate = 0;

    m_dwLastResortedUploadSlots = 0;
	m_hHighSpeedUploadTimer = NULL;
	m_bStatisticsWaitingListDirty = true;
	// mod Adu
	// Emanem
	// Inizializzo il numero di clients Adu
	m_AduClientsNum = 0;
}

//Anis -> Funzioni personalizzate gestione coda di adunanza. (trovare il primo e il secondo client con il ranking maggiore)
INT64	GetUpDownDiff(const CUpDownClient* client)
{
	if(client->Credits() == NULL)
		return _I64_MIN;
	return  client->Credits()->GetDownloadedTotal()-client->Credits()->GetUploadedTotal();
}

BOOL IsPayback(const CUpDownClient* client)
{	
	return (!client->GetRequestFile() || client->GetDownloadState() > DS_ONQUEUE) && GetUpDownDiff(client) > (SESSIONMAXTRANS/2);
}

BOOL SecondIsBetterClient(const bool bPaybackCheck, const CUpDownClient* first, const CUpDownClient* second, const UINT firstscore, const UINT secondscore)
{
	if(second == NULL)
		return FALSE;
	if(first == NULL)
		return TRUE;

	if(bPaybackCheck)
	{
		const BOOL bPay1 = IsPayback(first);
		const BOOL bPay2 = IsPayback(second);
		if(bPay1 && !bPay2)
			return FALSE;	//first is better
		if(!bPay1 && bPay2)
			return TRUE;	//second is better
		else if(bPay1 && bPay2) 
			return GetUpDownDiff(second) > GetUpDownDiff(first); //second or first?
		//flow over
	}

	return (secondscore > firstscore);
}

CUploadQueue::~CUploadQueue(){
	if (h_timer)
		KillTimer(0,h_timer);
	if (m_hHighSpeedUploadTimer)
		UseHighSpeedUploadTimer(false);
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

CUpDownClient* CUploadQueue::FindBestClientInQueue(DWORD typeClient)
{
	//>>> Anis::PowerShare
	CUpDownClient* toadd = NULL;
	CUpDownClient* toaddlow = NULL;
	UINT bestscore = 0;
	UINT bestlowscore = 0;

	CUpDownClient* friendpos = NULL;
	CUpDownClient* toaddPS = NULL;	
	CUpDownClient* toaddlowPS = NULL;
	UINT bestscorePS = 0;
	UINT bestlowscorePS = 0;
	uint8 highestPSPrio = 0;

	//>>> Anis::Payback First
	bool bPaybackCheck = false; 
	switch(thePrefs.GetPaybackFirst())
	{
		default:
		case 0: //off
			break;

		case 1: //on - one slot
			bPaybackCheck = m_lPaybackList.IsEmpty() ? true : false;
			break;

		case 2: //on - once every X slots
			bPaybackCheck = m_uiPaybackCount == 5;
			break;

		case 3: //auto - TODO?
			bPaybackCheck = m_lPaybackList.IsEmpty() || m_uiPaybackCount == 5;
			break;
	}
	//### Anis::Payback First

	// mod Adu
	// Emanem
	// Risistemo il valore temporaneo del numero di clients
	// AdunanzA
	uint32 newAduCount = 0;
	// fine mod Adu

	POSITION pos1, pos2;
	for (pos1 = waitinglist.GetHeadPosition();( pos2 = pos1 ) != NULL;)
	{
		waitinglist.GetNext(pos1);
		CUpDownClient* cur_client =	waitinglist.GetAt(pos2);
		CKnownFile* file = theApp.sharedfiles->GetFileByID(cur_client->GetUploadFileID());
		//While we are going through this list.. Lets check if a client appears to have left the network..
		ASSERT ( cur_client->GetLastUpRequest() );
		if ((::GetTickCount() - cur_client->GetLastUpRequest() > MAX_PURGEQUEUETIME) /*|| !file*/ )
		{
			//This client has either not been seen in a long time, or we no longer share the file he wanted anymore..
			cur_client->ClearWaitStartTime();
			RemoveFromWaitingQueue(pos2,true);
			continue;
		}

		if(cur_client->GetFriendSlot())
			bPaybackCheck = false; 

		if(file == NULL)
			continue;

		if ((((theStats.sessionSentBytes - theStats.stat_Adu_sessionSentBytes)/(((GetTickCount() - theStats.transferStarttime) / 1000)*1024)) > (thePrefs.m_AduValRipBanda) && (!cur_client->IsAduClient())))
			continue;

		if(cur_client->GetFriendSlot() && (!cur_client->HasLowID() || (cur_client->socket && cur_client->socket->IsConnected())))
			friendpos = cur_client;

		if(friendpos)
			continue; //no need to go on...
		
		// finished clearing
		const uint32 cur_score = cur_client->GetScore(false);

		// Codice AdunanzA
		// Emanem	18:15	4/4/2004
		//
		// In questo caso, in base al tipo di typeClient
		// in input si sceglie su che clients fare la "classifica"
		/// di priorita'

		// Modifica Adunanza
		// lupz
		// commento censurato e commentato :D
		//
		// a me tutto sommato piace!
		//
		// --------> // Stefano Picerno: Questo codice e' una merda, ma pare funzionare...
		// --------> //                  lo lascio stare cosi'
		// --------> // Anis: LoL
		// valore temporaneo per sapere se un client e' AdunanzA
		// o no

		bool bIsAduClient = cur_client->IsAduClient();
		// nel caso incremento il contatore temporaneo
		if (bIsAduClient)
			newAduCount++;

		// Se il tipo di client non e' disponibile
		// allora prendo il primo che mi capita
		if (typeClient == ADUNANZA_FASTWEB)
		{
			// Voglio un client ADU/FASTWEB
				
			if (m_AduClientsNum == 0)
			{
				// Non mi risulta che ci siano client adu/fastweb
				// ne prendo uno qualsiasi
				typeClient = ADUNANZA_ANY;
			}
		}
		if (typeClient == ADUNANZA_EXTERN)
		{
			// Voglio un client *NON* ADU/FASTWEB
			if (m_AduClientsNum >= waitinglist.GetCount()) 
			{
				// Non mi risulta che ci siano client NON adu/fastweb 
				// ne prendo uno qualsiasi
				typeClient = ADUNANZA_ANY;
			}
		}

		bool handleThisClient;
		switch(typeClient)
		{
			case ADUNANZA_EXTERN:
				// Voglio un client *NON* fastweb/adu 
				handleThisClient = !bIsAduClient;
				break;
			case ADUNANZA_FASTWEB:
				// Voglio un client fastweb/adu, 
				handleThisClient = bIsAduClient;
				break;
			default:
			case ADUNANZA_ANY:
				// Va bene un client qualsiasi
				handleThisClient = true;
				break;
		}

		if (cur_client->IsPowerShared()) // Anis -> i leechers si fottano. Non hanno il powershare.
		{
			uint8 curprio = file->GetUpPriority()+1;
			if(curprio >= 5)
				curprio = 0;
			if(handleThisClient && (curprio > highestPSPrio) || (curprio == highestPSPrio && SecondIsBetterClient(bPaybackCheck, toaddPS, cur_client, bestscorePS, cur_score)))
			{
				highestPSPrio = curprio;
				if(curprio > highestPSPrio)
				{
					bestscorePS = 0;
					bestlowscorePS = 0;
				}

				// cur_client is more worthy than current best client that is ready to go (connected).
				if(!cur_client->HasLowID() || (cur_client->socket && cur_client->socket->IsConnected())) {
					// this client is a HighID or a lowID client that is ready to go (connected)
					// and it is more worthy
					bestscorePS = cur_score;
					toaddPS = cur_client;
				} 
				else if(!cur_client->m_bAddNextConnect) {
					// this client is a lowID client that is not ready to go (not connected)

					// now that we know this client is not ready to go, compare it to the best not ready client
					// the best not ready client may be better than the best ready client, so we need to check
					// against that client
					if(SecondIsBetterClient(bPaybackCheck, toaddlowPS, cur_client, bestlowscorePS, cur_score)) {
						// it is more worthy, keep it
						bestlowscorePS = cur_score;
						toaddlowPS = cur_client;
					}
				}
			} 
		}
		else {
			if(SecondIsBetterClient(bPaybackCheck, toadd, cur_client, bestscore, cur_score)) {
				// cur_client is more worthy than current best client that is ready to go (connected).
				if(!cur_client->HasLowID() || (cur_client->socket && cur_client->socket->IsConnected())) {
					// this client is a HighID or a lowID client that is ready to go (connected)
					// and it is more worthy
					bestscore = cur_score;
					toadd = cur_client;
					//newclient = waitinglist.GetAt(toadd);
				} 
				else if(!cur_client->m_bAddNextConnect) {
					// this client is a lowID client that is not ready to go (not connected)
					// now that we know this client is not ready to go, compare it to the best not ready client
					// the best not ready client may be better than the best ready client, so we need to check
					// against that client
					if(SecondIsBetterClient(bPaybackCheck, toaddlow, cur_client, bestlowscore, cur_score)) {
						bestlowscore = cur_score;
						toaddlow = cur_client;
					}
				}
			}
		}
	}
	m_AduClientsNum = newAduCount;
	if (bestlowscore > bestscore && toaddlow)
		toaddlow->m_bAddNextConnect = true;
	else if (toaddlowPS && SecondIsBetterClient(bPaybackCheck, toaddPS, toaddlowPS, bestscorePS, bestlowscorePS))
		toaddlowPS->m_bAddNextConnect = true;
	else if (toaddlow && SecondIsBetterClient(bPaybackCheck, toadd, toaddlow, bestscore, bestlowscore))
		toaddlow->m_bAddNextConnect = true;
	if(friendpos)
		return friendpos;
	if(toaddPS)
		return toaddPS;
	return toadd;
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

void CUploadQueue::InsertInUploadingList(CUpDownClient* newclient) 
{
	newclient->m_bAddNextConnect = false;

	// Add it last
    theApp.uploadBandwidthThrottler->AddToStandardList(uploadinglist.GetCount(), newclient->GetFileUploadSocket());
	uploadinglist.AddTail(newclient);
	//Anis lupz
	if(IsPayback(newclient) && newclient->IsAduClient())
		m_lPaybackList.AddTail(newclient); 

	if (newclient->IsAduClient())
		aduuploadinglist.AddTail(newclient);
    newclient->SetSlotNumber(uploadinglist.GetCount());
	//tigerjact autoregolatore up ext
	if (thePrefs.upextfull && (!(newclient->IsAduClient()))&& (thePrefs.extfullwaittime < time(NULL))) 
	{
		int tempotest = time(NULL) - thePrefs.extfullwaittime + thePrefs.maxexttimewait;
		uint64 ExtSentPartial = (theStats.sessionSentBytes - thePrefs.ExtAdapterSentBytes)/1024;
		int partial =  (ExtSentPartial / tempotest);
		if (partial > 10)
			partial = partial - 10;
		if (partial < ADUNANZA_MIN_BW_TROLLER)
			partial = ADUNANZA_MIN_BW_TROLLER;

		if ( partial < thePrefs.m_AduValRipBanda) 
		{	
			thePrefs.m_AduValRipBanda = partial;
			CalcolaRatio(false);
		}
	
		thePrefs.checkextfull = true;
	}

	//  Anis -> per piacere ricordarsi la tabulazione del codice....
	if ((theApp.uploadqueue->GetUploadCount() == thePrefs.m_AduMaxUpSlots) && ( theApp.uploadqueue->GetAduUploadCount() == 0) && (!(newclient->IsAduClient())))
	{
		thePrefs.upextfull = true;
		if(thePrefs.checkextfull)
		{
			thePrefs.extfullwaittime = thePrefs.maxexttimewait + time(NULL);
			thePrefs.checkextfull = false;
			thePrefs.ExtAdapterSentBytes = theStats.sessionSentBytes;
		}
	}
	else 
	{	
		thePrefs.upextfull = false;
		thePrefs.checkextfull = true;
	}
}

bool CUploadQueue::AddUpNextClient(LPCTSTR pszReason, CUpDownClient* directadd)
{
	// Mod Adu
	// lupz
	// Anis
	// capisco cosa voglio

	DWORD typeClient = AduNextClient();

	if (typeClient == ADUNANZA_NONE)
		return false;

	CUpDownClient* newclient = NULL;
	// select next client or use given client
	if (!directadd)
	{
        newclient = FindBestClientInQueue(typeClient);

        if(newclient)
		{
		    RemoveFromWaitingQueue(newclient, true);
        }
	}
	else
	{
		if ((((theStats.sessionSentBytes - theStats.stat_Adu_sessionSentBytes)/(((GetTickCount() - theStats.transferStarttime) / 1000)*1024)) > (thePrefs.m_AduValRipBanda) && (!directadd->IsAduClient())))
			return false;

		newclient = directadd;
	}

    if(newclient == NULL) 
        return false;

	if (IsDownloading(newclient))
		return false;

	//>>> Anis::Optimization
	// statistic
	CKnownFile* reqfile = theApp.sharedfiles->GetFileByID((uchar*)newclient->GetUploadFileID());
	if (reqfile == NULL)
	{
		if(thePrefs.GetLogUlDlEvents())
			AddDebugLogLine(false, L"Tried to add client %s to upload list but reqfile wasn't found - prevented failed UL session!", newclient->DbgGetClientInfo());
		return false;
	}
	//### Anis::Optimization

    if(pszReason && thePrefs.GetLogUlDlEvents())
        AddDebugLogLine(false, _T("Adding client to upload list: %s Client: %s"), pszReason, newclient->DbgGetClientInfo());

	// Anis
	if (reqfile->GetFileSize() < (uint64)MAXPRIORITYCOLL_SIZE && directadd == NULL)
	{
		ASSERT( false );
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
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__AcceptUploadReq", newclient);
		Packet* packet = new Packet(OP_ACCEPTUPLOADREQ,0);
		theStats.AddUpDataOverheadFileRequest(packet->size);
		newclient->SendPacket(packet, true);
		newclient->SetUploadState(US_UPLOADING);
	}

	newclient->SetUpStartTime();
	newclient->ResetSessionUp();
	newclient->ResetRemainingUp();	// VQB: fullChunk

    InsertInUploadingList(newclient);

    m_nLastStartUpload = ::GetTickCount();

	//Anis -> Miglioria
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
    while(!activeClients_tick_list.IsEmpty() && !activeClients_list.IsEmpty() && curTick-activeClients_tick_list.GetHead() > 20*1000) {
        activeClients_tick_list.RemoveHead();
	    uint32 removed = activeClients_list.RemoveHead();

        if(removed > tempMaxRemoved) {
            tempMaxRemoved = removed;
        }
    }

	activeClients_list.AddTail(m_iHighestNumberOfFullyActivatedSlotsSinceLastCall);
    activeClients_tick_list.AddTail(curTick);

    if(activeClients_tick_list.GetSize() > 1) 
	{
        uint32 tempMaxActiveClients = m_iHighestNumberOfFullyActivatedSlotsSinceLastCall;
        uint32 tempMaxActiveClientsShortTime = m_iHighestNumberOfFullyActivatedSlotsSinceLastCall;
        POSITION activeClientsTickPos = activeClients_tick_list.GetTailPosition();
        POSITION activeClientsListPos = activeClients_list.GetTailPosition();
        while(activeClientsListPos != NULL && (tempMaxRemoved > tempMaxActiveClients && tempMaxRemoved >= m_MaxActiveClients || curTick - activeClients_tick_list.GetAt(activeClientsTickPos) < 10 * 1000))
		{
            DWORD activeClientsTickSnapshot = activeClients_tick_list.GetAt(activeClientsTickPos);
            uint32 activeClientsSnapshot = activeClients_list.GetAt(activeClientsListPos);

            if(activeClientsSnapshot > tempMaxActiveClients) 
			{
                tempMaxActiveClients = activeClientsSnapshot;
            }

            if(activeClientsSnapshot > tempMaxActiveClientsShortTime && curTick - activeClientsTickSnapshot < 10 * 1000) 
			{
                tempMaxActiveClientsShortTime = activeClientsSnapshot;
            }

            activeClients_tick_list.GetPrev(activeClientsTickPos);
            activeClients_list.GetPrev(activeClientsListPos);
        }

        if(tempMaxRemoved >= m_MaxActiveClients || tempMaxActiveClients > m_MaxActiveClients) 
		{
            m_MaxActiveClients = tempMaxActiveClients;
        }

        m_MaxActiveClientsShortTime = tempMaxActiveClientsShortTime;
    } 
	else 
	{
        m_MaxActiveClients = m_iHighestNumberOfFullyActivatedSlotsSinceLastCall;
        m_MaxActiveClientsShortTime = m_iHighestNumberOfFullyActivatedSlotsSinceLastCall;
    }
}

/**
 * Maintenance method for the uploading slots. It adds and removes clients to the
 * uploading list. It also makes sure that all the uploading slots' Sockets always have
 * enough packets in their queues, etc.
 *
 * This method is called approximately once every 100 milliseconds. (Anis) -> Porca Madonna
 */
void CUploadQueue::Process() {

    DWORD curTick = ::GetTickCount();

    UpdateActiveClientsInfo(curTick);

	if (ForceNewClient())
	{
        AddUpNextClient( _T("Not enough open upload slots for current ul speed"), NULL);
	}

    // The loop that feeds the upload slots with data.
	POSITION pos = uploadinglist.GetHeadPosition();
	while(pos != NULL)
	{
        // Get the client. Note! Also updates pos as a side effect.
		CUpDownClient* cur_client = uploadinglist.GetNext(pos);
		if (thePrefs.m_iDbgHeap >= 2)
			ASSERT_VALID(cur_client);
		//It seems chatting or friend slots can get stuck at times in upload.. This needs looked into..
		if (!cur_client->socket)
		{
			RemoveFromUploadQueue(cur_client, _T("Uploading to client without socket? (CUploadQueue::Process)"));
			if(cur_client->Disconnected(_T("CUploadQueue::Process")))
			{
				delete cur_client;
			}

		} 
		else 
		{
            cur_client->SendBlockData();
        }
	}

    // Save used bandwidth for speed calculations
	uint64 sentBytes = theApp.uploadBandwidthThrottler->GetNumberOfSentBytesSinceLastCallAndReset();
	avarage_dr_list.AddTail(sentBytes);
    m_avarage_dr_sum += sentBytes;

    (void)theApp.uploadBandwidthThrottler->GetNumberOfSentBytesOverheadSinceLastCallAndReset();

    avarage_friend_dr_list.AddTail(theStats.sessionSentBytesToFriend);

    // Save time beetween each speed snapshot
    avarage_tick_list.AddTail(curTick);

    // don't save more than 30 secs of data
    while(avarage_tick_list.GetCount() > 3 && !avarage_friend_dr_list.IsEmpty() && ::GetTickCount()-avarage_tick_list.GetHead() > 30*1000) 
	{
   	    m_avarage_dr_sum -= avarage_dr_list.RemoveHead();
        avarage_friend_dr_list.RemoveHead();
        avarage_tick_list.RemoveHead();
    }

	if (GetDatarate() > HIGHSPEED_UPLOADRATE_START && m_hHighSpeedUploadTimer == 0)
		UseHighSpeedUploadTimer(true);
	else if (GetDatarate() < HIGHSPEED_UPLOADRATE_END && m_hHighSpeedUploadTimer != 0)
		UseHighSpeedUploadTimer(false);
}


bool CUploadQueue::AcceptNewClient(bool addOnNextConnect, bool isAduClient) {
	uint32 curUploadSlots = (uint32)uploadinglist.GetCount();

	//We allow ONE extra slot to be created to accommodate lowID users.
	//This is because we skip these users when it was actually their turn
	//to get an upload slot..
	if(addOnNextConnect && curUploadSlots > 0)
		curUploadSlots--;		

    return AcceptNewClient(curUploadSlots, isAduClient);
}

bool CUploadQueue::AcceptNewClient(uint32 curUploadSlots, bool isAduClient){
	// check if we can allow a new client to start downloading from us

	// Mod Adu
	// lupz
	// capisco cosa voglio

	DWORD typeClient = AduNextClient();

	switch (typeClient) 
	{
		case ADUNANZA_NONE:
			return false;
			break;
		case ADUNANZA_FASTWEB:
			if (!isAduClient)
				return false;
			break;
		case ADUNANZA_EXTERN:
			if (isAduClient)
				return false;
			break;
	}

	// Fine mod Adu

	if (curUploadSlots < MIN_UP_CLIENTS_ALLOWED)
	{
		return true;
	}

    const uint32 cur_tick = ::GetTickCount();

	//Anis -> ottimizziamo l'upload scartando le persone alla quale spediamo pacchetti a velocità ridicole

	if (thePrefs.GetAutoDropSystem() && m_dwNextBlockingCheck < cur_tick) // we haven't checked lately 
	{
		m_dwNextBlockingCheck = cur_tick + SEC2MS(5); // only check every 5 seconds
		const uint32 uMinUpSpeed = min(1024, (uint32)(thePrefs.GetMaxUpload()*100.0f)); // one tenth of the upload, min 1 kbyte/s
		//search a socket we should remove
		for(POSITION pos = uploadinglist.GetHeadPosition(); pos != NULL;)
		{
			CUpDownClient* cur_client = uploadinglist.GetNext(pos);

			if(!cur_client->IsFriend() && cur_client->GetDatarate() < uMinUpSpeed && cur_client->GetUpStartTimeDelay() > (MIN2MS(2) + SEC2MS(30)) && cur_client->GetSlotNumber() <= theApp.uploadqueue->GetActiveUploadsCount() && waitinglist.GetCount() > 0 && (cur_client->GetFileUploadSocket()->GetBlockRatioOverall() >= 75.0f || cur_client->GetFileUploadSocket()->GetBlockRatio() >= 85.0f))
			{
				CString buffer;
				buffer.Format(_T("Client blocked too often: avg20: %0.0f%%, all: %0.0f%%, avg-speed: %0.0f B/s"), cur_client->GetFileUploadSocket()->GetBlockRatio(), cur_client->GetFileUploadSocket()->GetBlockRatioOverall(), (float)cur_client->GetSessionUp()/cur_client->GetUpStartTimeDelay()*1000.0f);
				RemoveFromUploadQueue(cur_client, buffer);
				m_dwNextBlockingCheck +=  SEC2MS(10); // drop the next client in at least 15sec to have enough time to establish a new connection...
				break; //only one socket per loop
			}
		}
	}

	uint16 MaxSpeed;
    if (thePrefs.IsDynUpEnabled())
        MaxSpeed = (uint16)(theApp.lastCommonRouteFinder->GetUpload()/1024);
    else
		MaxSpeed = thePrefs.GetMaxUpload();
	
	if (curUploadSlots >= thePrefs.m_AduMaxUpSlots || curUploadSlots >= 4 && (curUploadSlots >= (datarate/UPLOAD_CHECK_CLIENT_DR) || curUploadSlots >= ((uint32)MaxSpeed)*1024/UPLOAD_CLIENT_DATARATE || (thePrefs.GetMaxUpload() == UNLIMITED && !thePrefs.IsDynUpEnabled() && thePrefs.GetMaxGraphUploadRate(true) > 0 && curUploadSlots >= ((uint32)thePrefs.GetMaxGraphUploadRate(false))*1024/UPLOAD_CLIENT_DATARATE))) // max number of clients to allow for all circumstances
	    return false;

	return true;
}

bool CUploadQueue::ForceNewClient(bool allowEmptyWaitingQueue) {
    if(!allowEmptyWaitingQueue && waitinglist.GetSize() <= 0)
        return false;

	if (::GetTickCount() - m_nLastStartUpload < 1000 && datarate < 102400 )
		return false;

	uint32 curUploadSlots = (uint32)uploadinglist.GetCount();

	if (curUploadSlots < MIN_UP_CLIENTS_ALLOWED)
		return true;

	if(!AcceptNewClient(curUploadSlots, AduNextClient() == ADUNANZA_FASTWEB) || !theApp.lastCommonRouteFinder->AcceptNewClient()) { // UploadSpeedSense can veto a new slot if USS enabled
		return false;
    }

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

CUpDownClient* CUploadQueue::GetWaitingClientByIP(uint32 dwIP)
{
	for (POSITION pos = waitinglist.GetHeadPosition();pos != 0;)
	{
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

	uint16 cSameIP = 0;
	// check for double
	POSITION pos1, pos2;
	for (pos1 = waitinglist.GetHeadPosition();( pos2 = pos1 ) != NULL;)
	{
		waitinglist.GetNext(pos1);
		CUpDownClient* cur_client = waitinglist.GetAt(pos2);
		if (cur_client == client)
		{	
			// Mod Adu
			// lupz
			// supporto isAduClient
			if (client->m_bAddNextConnect && AcceptNewClient(client->m_bAddNextConnect, client->IsAduClient()))
			{
				//Special care is given to lowID clients that missed their upload slot
				//due to the saving bandwidth on callbacks.
				if(thePrefs.GetLogUlDlEvents())
					AddDebugLogLine(true, _T("Adding ****lowid when reconnecting. Client: %s"), client->DbgGetClientInfo());
				client->m_bAddNextConnect = false;
				RemoveFromWaitingQueue(client, true);
				// statistic values // TODO: Maybe we should change this to count each request for a file only once and ignore reasks
				CKnownFile* reqfile = theApp.sharedfiles->GetFileByID((uchar*)client->GetUploadFileID());
				if (reqfile)
					reqfile->statistic.AddRequest();
				AddUpNextClient(_T("Adding ****lowid when reconnecting."), client);
				return;
			}
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

	if(!client->IsAduClient()) { //Anis -> faccio il controllo solo agli esterni :p
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
	}

	// statistic values // TODO: Maybe we should change this to count each request for a file only once and ignore reasks
	CKnownFile* reqfile = theApp.sharedfiles->GetFileByID((uchar*)client->GetUploadFileID());
	if (reqfile)
		reqfile->statistic.AddRequest();

	// Anis -> precedenza slot parti piccole (:
	if(reqfile && reqfile->GetFileSize() < (uint64)MAXPRIORITYCOLL_SIZE) //i.e. 50kB)
	{
		RemoveFromWaitingQueue(client, true);
		AddUpNextClient(L"Small File Priority Slot", client);
		return;
	}

	if (client->IsDownloading()) 
	{
		// he's already downloading and wants probably only another file
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__AcceptUploadReq", client);
		Packet* packet = new Packet(OP_ACCEPTUPLOADREQ,0);
		theStats.AddUpDataOverheadFileRequest(packet->size);
		client->SendPacket(packet, true);
		return;
	}

	if (waitinglist.IsEmpty() && ForceNewClient(true)) 
	{
		AddUpNextClient(_T("Direct add with empty queue."), client);
	}

	else 
	{
		// mod Adu
		// Emanem
		// se metto un coda un client Adu incremento il contatore
 		if (client->IsAduClient()) 
			m_AduClientsNum++;

		m_bStatisticsWaitingListDirty = true;
		waitinglist.AddTail(client);
		client->SetUploadState(US_ONUPLOADQUEUE);
		theApp.emuledlg->transferwnd->GetQueueList()->AddClient(client,true);
		theApp.emuledlg->transferwnd->ShowQueueCount();
		client->SendRankingInfo();
	}
}

float CUploadQueue::GetAverageCombinedFilePrioAndCredit() 
{
    DWORD curTick = ::GetTickCount();

    if (curTick - m_dwLastCalculatedAverageCombinedFilePrioAndCredit > 5*1000) {
        m_dwLastCalculatedAverageCombinedFilePrioAndCredit = curTick;

        // TODO: is there a risk of overflow? I don't think so...
        double sum = 0;
	    for (POSITION pos = waitinglist.GetHeadPosition(); pos != NULL; /**/)
		{
		    CUpDownClient* cur_client =	waitinglist.GetNext(pos);
            sum += cur_client->GetCombinedFilePrioAndCredit();
        }
        m_fAverageCombinedFilePrioAndCredit = (float)(sum/waitinglist.GetSize());
    }

    return m_fAverageCombinedFilePrioAndCredit;
}

bool CUploadQueue::RemoveFromUploadQueue(CUpDownClient* client, LPCTSTR pszReason, bool updatewindow, bool earlyabort)
{
    bool result = false;
    uint32 slotCounter = 1;
	for (POSITION pos = uploadinglist.GetHeadPosition();pos != 0;)
	{
        POSITION curPos = pos;
        CUpDownClient* curClient = uploadinglist.GetNext(pos);
		if (client == curClient)
		{
			if (updatewindow)
				theApp.emuledlg->transferwnd->GetUploadList()->RemoveClient(client);

			if (thePrefs.GetLogUlDlEvents())
                AddDebugLogLine(DLP_DEFAULT, true,_T("Removing client from upload list: %s Client: %s Transferred: %s SessionUp: %s QueueSessionPayload: %s In buffer: %s Req blocks: %i File: %s"), pszReason==NULL ? _T("") : pszReason, client->DbgGetClientInfo(), CastSecondsToHM( client->GetUpStartTimeDelay()/1000), CastItoXBytes(client->GetSessionUp(), false, false), CastItoXBytes(client->GetQueueSessionPayloadUp(), false, false), CastItoXBytes(client->GetPayloadInBuffer()), client->GetNumberOfRequestedBlocksInQueue(), (theApp.sharedfiles->GetFileByID(client->GetUploadFileID())?theApp.sharedfiles->GetFileByID(client->GetUploadFileID())->GetFileName():_T("")));
            client->m_bAddNextConnect = false;
			uploadinglist.RemoveAt(curPos);

	//>>> Anis::Payback First
			POSITION findPos = m_lPaybackList.Find(client);
			if(findPos)
				m_lPaybackList.RemoveAt(findPos);
	//### Anis::Payback First
			// Mod Adu
			// lupz
			POSITION adupos=aduuploadinglist.Find(client);
			if (adupos)
				aduuploadinglist.RemoveAt(adupos);
			// Fine mod Adu


            bool removed = theApp.uploadBandwidthThrottler->RemoveFromStandardList(client->socket);
            bool pcRemoved = theApp.uploadBandwidthThrottler->RemoveFromStandardList((CClientReqSocket*)client->m_pPCUpSocket);
			(void)removed;
			(void)pcRemoved;


			if(client->GetSessionUp() > 0) 
			{
				++successfullupcount;
				totaluploadtime += client->GetUpStartTimeDelay()/1000;
            } 
			else if(earlyabort == false)
				++failedupcount;

            CKnownFile* requestedFile = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
            if(requestedFile != NULL) 
			{
                requestedFile->UpdatePartsInfo();
            }
			theApp.clientlist->AddTrackClient(client); // Keep track of this client
			client->SetUploadState(US_NONE);
			client->ClearUploadBlockRequests();
			//client->SetCollectionUploadSlot(false); Anis -> non serve +

            m_iHighestNumberOfFullyActivatedSlotsSinceLastCall = 0;

			result = true;
        } 
		else 
		{
            curClient->SetSlotNumber(slotCounter);
            slotCounter++;
        }
	}
	return result;
}

uint32 CUploadQueue::GetAverageUpTime()
{
	if( successfullupcount )
	{
		return totaluploadtime/successfullupcount;
	}
	return 0;
}

bool CUploadQueue::RemoveFromWaitingQueue(CUpDownClient* client, bool updatewindow)
{
	POSITION pos = waitinglist.Find(client);
	if (pos)
	{
		RemoveFromWaitingQueue(pos,updatewindow);
		return true;
	}
	else
		return false;
}

void CUploadQueue::RemoveFromWaitingQueue(POSITION pos, bool updatewindow)
{	
	m_bStatisticsWaitingListDirty = true;
	CUpDownClient* todelete = waitinglist.GetAt(pos);
	// mod Adu
	// Emanem
	// se il client era adu decremento il contatore
	if (todelete->IsAduClient()) 
        m_AduClientsNum--;
	// fine mod Adu

	waitinglist.RemoveAt(pos);
	if (updatewindow) 
	{
		theApp.emuledlg->transferwnd->GetQueueList()->RemoveClient(todelete);
		theApp.emuledlg->transferwnd->ShowQueueCount();
	}
	todelete->m_bAddNextConnect = false;
	todelete->SetUploadState(US_NONE);
}

void CUploadQueue::UpdateMaxClientScore()
{
	m_imaxscore=0;
	// Inizio mod Adu
	m_AduImaxscore = 0;
	m_ExtImaxscore = 0;
	for(POSITION pos = waitinglist.GetHeadPosition(); pos != 0; ) 
	{
		CUpDownClient* client = waitinglist.GetNext(pos);
		uint32 score = client->GetScore(true, false);
		if(score > m_imaxscore )
			m_imaxscore=score;
		if (client->IsAduClient())
		{
			if (score > m_AduImaxscore) 
				m_AduImaxscore = score;
		}
		else if (score > m_ExtImaxscore) 
			m_ExtImaxscore = score;
	}
	// fine mod Adu
}

bool CUploadQueue::CheckForTimeOver(CUpDownClient* client) // Funzione di Anis Hireche
{
	if(client->GetFriendSlot())
	{
		UINT remain = client->GetRemainingUp() + client->GetPayloadInBuffer();
		if(NULL == remain) // Anis -> Se la quantità di byte arriva a 0, stacca il client.
			return true;
		else
			return false;
	}
	/* Kaiser -- 08/04/2004 - 15.43
	Anis -> 29/01/2012 - 21:15
	Se è un ext lo "stacca"  (9,3mb) altrimenti cerca
	di staccarlo dopo  (18,6mb) ovvero il doppio.
	In questo modo possiamo favorire lo scambio tra aduner senza dover 
	modificare i tempi di reask e rischiando quindi meno ban! */
	// Inoltre se la prossima slot deve essere AdunanzA e non ho altri Adu in coda
	// non lo butto giu' per poi riprenderlo in coda di upload.
	// Quindi se la prossima slot e' non adunanza lo butto giu'.
	// Condizioni sufficiente per buttare giu' un client sono:
	// - Avere altri Adu in coda di attesa
	// - Che il prossimo client non sia Adu

	//Anis -> Importante per non mandare in totale timeout i client in upload!
	//Anis -> Riscritto da zero, adattato al full chunk transfer :P

	if(client->GetQueueSessionPayloadUp() > ADU_SESSIONMAXTRANS)
	{
		if (client->IsAduClient())
		{
			if ((theApp.uploadqueue->GetAdunanzAUserCount() > 0) || (ADUNANZA_FASTWEB != AduGetTypeBand()))
			{
				return true;
			}
		}
	}

	UINT remain = client->GetRemainingUp() + client->GetPayloadInBuffer();

	if(NULL == remain) // Anis -> Se la quantità di byte arriva a 0, stacca il client.
		return true;

	return false;
}

void CUploadQueue::DeleteAll()
{
	waitinglist.RemoveAll();
	uploadinglist.RemoveAll();
	aduuploadinglist.RemoveAll();
}

UINT CUploadQueue::GetWaitingPosition(CUpDownClient* client)
{
	if (!IsOnUploadQueue(client))
		return 0;
	UINT rank = 1;
	UINT myscore = client->GetScore(false);
	for (POSITION pos = waitinglist.GetHeadPosition(); pos != 0; )
	{
		if (waitinglist.GetNext(pos)->GetScore(false) > myscore)
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

		if (theApp.emuledlg->status != 255)
			return;

		theApp.uploadqueue->UpdateDatarates();

		const DWORD curTick = ::GetTickCount(); //cache it...

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


			// Elandal:ThreadSafeLogging -->
			// other threads may have queued up log lines. This prints them.
			theApp.HandleLogQueues();		
			// Elandal: ThreadSafeLogging <--

		if (thePrefs.ShowOverhead()){
			theStats.CompUpDatarateOverhead();
			theStats.CompDownDatarateOverhead();
		}
		counter++;

		// one second
		if (counter >= 10){
			counter=0;


			// try to use different time intervals here to not create any disk-IO bottle necks by saving all files at once
			theApp.clientcredits->Process();
			theApp.serverlist->Process();		// 17 minutes
			theApp.knownfiles->Process();		// 11 minutes
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

            //save rates every second
			theStats.RecordRate();
			// mobilemule sockets
			theApp.mmserver->Process();

			// ZZ:UploadSpeedSense -->
            theApp.emuledlg->ShowPing();

             if(!theApp.clientlist->GiveClientsForTraceRoute()) 
			 {
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
				theApp.OnlineSig(); // Added By Bouc7 
				if (!theApp.emuledlg->IsTrayIconToFlash())
					theApp.emuledlg->ShowTransferRate();

				thePrefs.EstimateMaxUploadCap(theApp.uploadqueue->GetDatarate()/1024);
				
//>>> Anis::Automatic Firewalled Retries
				//15 retries = 30 mins!
#define MAX_FIREWALLED_RETRIES	15
				if(curTick - m_dwFirewalledTimer > MIN2MS(2))
				{
					m_dwFirewalledTimer = curTick;
					if(Kademlia::CKademlia::IsConnected() && Kademlia::CKademlia::IsFirewalled())
					{
						if(m_iFirewalledRetries < MAX_FIREWALLED_RETRIES)
						{
							++m_iFirewalledRetries;
							theApp.QueueLogLineEx(LOG_WARNING, L"Rechecking firewalled status - try %u out of %u...", m_iFirewalledRetries, MAX_FIREWALLED_RETRIES);
							// Refresh der Portmappings ? [shadow2004]
							// some routers seems to loose the settings after a while
							Kademlia::CKademlia::RecheckFirewalled();
						}
						else if(m_iFirewalledRetries == MAX_FIREWALLED_RETRIES)
						{
							++m_iFirewalledRetries;
							theApp.QueueLogLineEx(LOG_ERROR, L"Rechecking firewalled status failed %u times - stopping checks...", MAX_FIREWALLED_RETRIES);
						}
					}
					else
						m_iFirewalledRetries = 0;
				}
//### Anis::Automatic Firewalled Retries

				// update cat-titles with downloadinfos only when needed
				if (thePrefs.ShowCatTabInfos() && 
					theApp.emuledlg->activewnd == theApp.emuledlg->transferwnd && 
					theApp.emuledlg->IsWindowVisible()) 
						theApp.emuledlg->transferwnd->UpdateCatTabTitles(false);
				//Anis
				theApp.emuledlg->transferwnd->UpdateListCount(wnd2Uploading);
			}
			
			theApp.emuledlg->ShowTransferRate();

			statsave++;
			// *** 60 seconds *********************************************
			if (statsave >= 60) {
				statsave=0;

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
	if (!pos)
	{
		TRACE("Error: CUploadQueue::GetNextClient");
		return waitinglist.GetHead();
	}
	waitinglist.GetNext(pos);
	if (!pos)
		return NULL;
	else
		return waitinglist.GetAt(pos);
}

void CUploadQueue::UpdateDatarates() 
{
    // Calculate average datarate
    if(::GetTickCount()-m_lastCalculatedDataRateTick > 500) 
	{
        m_lastCalculatedDataRateTick = ::GetTickCount();

        if(avarage_dr_list.GetSize() >= 2 && (avarage_tick_list.GetTail() > avarage_tick_list.GetHead())) {
	        datarate = (UINT)(((m_avarage_dr_sum - avarage_dr_list.GetHead())*1000) / (avarage_tick_list.GetTail() - avarage_tick_list.GetHead()));
            friendDatarate = (UINT)(((avarage_friend_dr_list.GetTail() - avarage_friend_dr_list.GetHead())*1000) / (avarage_tick_list.GetTail() - avarage_tick_list.GetHead()));
        }
    }
}

uint32 CUploadQueue::GetDatarate() 
{
    return datarate;
}

uint32 CUploadQueue::GetToNetworkDatarate() 
{
    if(datarate > friendDatarate) 
	{
        return datarate - friendDatarate;
    } 
	else 
	{
        return 0;
    }
}

void CUploadQueue::ReSortUploadSlots(bool force) 
{
//>>> Anis::PowerShare //>>> Anis::SlotFocus
    DWORD curtick = ::GetTickCount();
    if(uploadinglist.GetCount() < 2 || (!force && curtick - m_dwLastResortedUploadSlots <= 10*1000)) //>>> taz::fix activated slots
		return;

        theApp.uploadBandwidthThrottler->Pause(true);

    	CTypedPtrList<CPtrList, CUpDownClient*> tempUploadinglist;

        // Remove all clients from uploading list and store in tempList
        POSITION ulpos = uploadinglist.GetHeadPosition();
        while (ulpos != NULL) 
		{
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
        while(tempPos != NULL) 
		{
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

VOID CALLBACK CUploadQueue::HSUploadTimer(HWND, UINT, UINT_PTR, DWORD)
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
            cur_client->CreateNextBlockPackage();
	}
}

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
// VQB: fullChunk
void CUploadQueue::UpdateRemainingUp()
{
	for(POSITION pos = uploadinglist.GetHeadPosition(); pos;)
		uploadinglist.GetNext(pos)->UpdateRemainingUp();
}
// VQB: fullChunk

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