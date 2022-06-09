//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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
#include "UploadQueue.h"
#include "updownclient.h"
#include "SharedFileList.h"
#include "packets.h"
#include "emule.h"
#include "KnownFile.h"
#include "ServerList.h"
#ifdef OLD_SOCKETS_ENABLED
#include "ListenSocket.h"
#endif
#include "ini2.h"
#include "math.h"
#include "MMServer.h"
#include <share.h>
#include <io.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define MINNUMBEROFTRICKLEUPLOADS 2
#define UQ_DELAY_CORRECTION		200		//max. delay to start correction (in ms)

static uint32	g_iCounter;

VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg,UINT_PTR idEvent,DWORD dwTime);
UINT SaveSourcesThread(HANDLE lpArg);

//	TODO rewrite the whole networkcode, use overlapped sockets

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CUploadQueue::CUploadQueue() : waitinglist(100)
{
	EMULE_TRY

	m_hTimer = SetTimer(NULL, 141, 100, TimerProc);
	if (!m_hTimer)
		AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T(__FUNCTION__) _T(": Fatal Error, failed to create Timer"));
	m_dwDataRate = 0;
	m_dwBannedCount = 0;
	g_iCounter = 0;

//	Extended Upload stats
	m_iULSessionSuccessful = 0;
	m_iULSessionSuccessfulFullChunk = 0;
	for (int i=0; i<ETS_TERMINATOR; i++)
	{
		m_iULSessionSuccessfulPartChunk[i] = 0;
		m_iULSessionFailed[i] = 0;
	}
	m_iULSessionFailedNoDataForRemoteClient = 0;

	m_iTotalUploadTime = 0;
	m_dwSumUpDataOverheadInDeque = 0;
	m_nUpDataRateMSOverhead = 0;
	m_nUpDataRateOverhead = 0;
	m_nUpDataOverheadSourceExchange = 0;
	m_nUpDataOverheadFileRequest = 0;
	m_nUpDataOverheadOther = 0;
	m_nUpDataOverheadServer = 0;
	m_nUpDataOverheadSourceExchangePackets = 0;
	m_nUpDataOverheadFileRequestPackets = 0;
	m_nUpDataOverheadOtherPackets = 0;
	m_nUpDataOverheadServerPackets = 0;

	m_iLeftOverBandwidth = 0;

	m_guessedMaxLANBandwidth = 3000;

	m_lastGaveDataTick = ::GetTickCount();
	m_dwLastScheduledBackupTick = ::GetTickCount();

	m_pSaveThread = NULL;
//	Create event responsible for thread termination
	m_hQuitEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_hQuitEvent)
	{
	//	Create lower priority thread that will do the "job"
		m_pSaveThread = AfxBeginThread(SaveSourcesThread, reinterpret_cast<LPVOID>(this),
										THREAD_PRIORITY_BELOW_NORMAL + g_App.m_pPrefs->GetMainProcessPriority(),
										CREATE_SUSPENDED);
		if (m_pSaveThread != NULL)
		{
		//	Disable self termination, because we plan to wait for thread termination
			m_pSaveThread->m_bAutoDelete = FALSE;
			m_pSaveThread->ResumeThread();
		}
		else
			AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("Error creating SourcesSaveThread"));
	}
	else
	{
		AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("Error creating SourcesSaveThread"));
	}

	m_strUploadLogFilePath.Format(_T("%supload.log"), g_App.m_pPrefs->GetAppDir());
#if 1 //code left for smooth migration, delete in v1.3
	if (GetTextFileFormat(m_strUploadLogFilePath) == tffANSI)
	{
		CString strBackupFile(m_strUploadLogFilePath);

		strBackupFile += _T(".ansi");

		_tremove(strBackupFile);
		_trename(m_strUploadLogFilePath, strBackupFile);
	}
#endif

	::InitializeCriticalSection(&m_csUploadQueueList);

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUploadQueue::AddClientToUploadQueue(CUpDownClient *pClient)
{
	EMULE_TRY

	CUpDownClient	*pNewSource;

// Select next client or use given client
	if (!pClient)
	{
		POSITION		toadd = 0, pos1, pos2;
		CUpDownClient	*pSource, *pLowIdClient = NULL;
		uint32			dwScore, dwBestScore = 0, dwBestLowIdScore = 0, dwCurTick = ::GetTickCount();

		for (pos1 = waitinglist.GetHeadPosition(); (pos2 = pos1) != NULL;)
		{
			pSource = waitinglist.GetNext(pos1);

		// Clear dead clients
			ASSERT(pSource->GetLastUpRequest());

		// Remove "?" from queue
			if ( (dwCurTick - pSource->GetLastUpRequest() > MAX_PURGEQUEUETIME)
				|| !g_App.m_pSharedFilesList->GetFileByID(pSource->m_reqFileHash) )
			{
				RemoveFromWaitingQueue(pos2, true);
#ifdef OLD_SOCKETS_ENABLED
				if (!pSource->m_pRequestSocket)
				{
					if (pSource->Disconnected())
						pSource = NULL;
				}
#endif //OLD_SOCKETS_ENABLED
				continue;
			}
			if ((dwScore = pSource->GetScore()) > dwBestScore)
			{
				if (!pSource->HasLowID()
#ifdef OLD_SOCKETS_ENABLED
					|| (pSource->m_pRequestSocket && pSource->m_pRequestSocket->IsConnected())
#endif //OLD_SOCKETS_ENABLED
				)
				{
				//	Client is a HighID or a currently connected to us LowID client
					dwBestScore = dwScore;
					toadd = pos2;
				}
				else if (!pSource->IsAddNextConnect())
				{
				//	Client is a LowID client that is not ready to go (not connected),
				//	compare it with the best not ready client
					if (dwScore > dwBestLowIdScore)
					{
						dwBestLowIdScore = dwScore;
						pLowIdClient = waitinglist.GetAt(pos2);
					}
				}
			}
		}
	//	The best not ready client may be better than the best ready client,
	//	so we need to check against that client
		if ((dwBestLowIdScore > dwBestScore) && (pLowIdClient != NULL))
			pLowIdClient->SetAddNextConnect(true);

		if (!toadd)
			return;

		pNewSource = waitinglist.GetAt(toadd);
		RemoveFromWaitingQueue(toadd, true);
		g_App.m_pMDlg->m_wndTransfer.UpdateUploadHeader();
	}
	else
		pNewSource = pClient;

	if (!g_App.m_pClientList->IsValidClient(pNewSource))
		return;

// Never upload to already downloading client
	if (IsDownloading(pNewSource))
		return;

// Tell the client that we are now ready to upload
#ifdef OLD_SOCKETS_ENABLED
	if (!pNewSource->IsHandshakeFinished())
	{
		pNewSource->SetUploadState(US_CONNECTING);
	//	Exceeding number of open TCP connections (TooManySockets) here
	//	by one is fine, as we don't invite to upload queue too often
		if (!pNewSource->TryToConnect(true))
			return;
	}
	else
	{
		Packet* packet = new Packet(OP_ACCEPTUPLOADREQ, 0);
		g_App.m_pUploadQueue->AddUpDataOverheadFileRequest(packet->m_dwSize);
		pNewSource->m_pRequestSocket->SendPacket(packet, true);
		pNewSource->SetUploadState(US_UPLOADING);
		pNewSource->SetLastGotULData();
	}
#endif //OLD_SOCKETS_ENABLED

	pNewSource->SetUpStartTime();
	pNewSource->ResetSessionUp();
	pNewSource->ResetCompressionGain();
	pNewSource->SetAddNextConnect(false);
	EnterCriticalSection(&m_csUploadQueueList);
	m_UploadingList.push_back(pNewSource);
	LeaveCriticalSection(&m_csUploadQueueList);
// clear the information about active clients, if their number in upload queue was changed
	m_activeClientsDeque.clear();
	m_activeClientsSortedVector.clear();

// Statistic
	CKnownFile	*pReqPartFile = g_App.m_pSharedFilesList->GetFileByID((uchar*)pNewSource->m_reqFileHash);

	if (pReqPartFile)
		pReqPartFile->statistic.AddAccepted();

	g_App.m_pMDlg->m_wndTransfer.m_ctlUploadList.AddClient(pNewSource);

	EMULE_CATCH
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUploadQueue::Process()
{
	EMULE_TRY

	ClientList CopyUploadQueueList;
	ClientList::const_iterator cIt;
	vector<uint32>::iterator vecIt;
	DWORD curTick = ::GetTickCount();
	sint32 usedBandwidthThisCall = 0;		// Keeps track of how much bandwidth we use this call
	int timeSinceLastGaveDataMs;
	int QtyLANClients = 0;					//LANCAST - number of LAN clients in current iteration

// After 49 days tick wraps around. I think this still handles it. (Yeah right! As if we wouldn't have crashed by then :))
	timeSinceLastGaveDataMs = curTick - m_lastGaveDataTick;

// Get a copy of Upload Queue for current iteration to prevent any interference during data upload
	GetCopyUploadQueueList(&CopyUploadQueueList);

//	There are timeout delays which can be up to 25 seconds (instead of 100ms)
//	As we can't return time back as well as put more data than the physical capacity,
//	we shouldn't try to keep average upload limit in this case to avoid high upload traffic
//	peaks (as it significantly influence downloading as well as other network applications)
//	So we will try to keep the limit and avoid high peaks
	if (timeSinceLastGaveDataMs > UQ_DELAY_CORRECTION)
	{
		int	iRateFor100ms = (g_App.m_pPrefs->GetMaxUpload() * 1024) / 100;
		int	iLeftCorrection = (iRateFor100ms * (timeSinceLastGaveDataMs - UQ_DELAY_CORRECTION + 100)) / 100;

	//	Adjust left over according passed time
		if (m_iLeftOverBandwidth >= 0)
		{
			m_iLeftOverBandwidth -= iLeftCorrection;
		//	Reserve 50ms data boost to be prepared for next delays
			iLeftCorrection = -(iRateFor100ms * (UQ_DELAY_CORRECTION - 150)) / 100;

			if (m_iLeftOverBandwidth < iLeftCorrection)
				m_iLeftOverBandwidth = iLeftCorrection;
		}
		else
			m_iLeftOverBandwidth += iLeftCorrection;
	//	There will be a boost later, so no need to add something to it
		if (m_iLeftOverBandwidth > 0)
			m_iLeftOverBandwidth = 0;
	//	Behave like there was a small delay to reduce peaks
		timeSinceLastGaveDataMs = UQ_DELAY_CORRECTION;
	}

// There's some saved bandwidth that we want to try to spend this round.
// curMaxULRatems keeps track of how much data we want to spend.
	sint32 realMaxULRatems = static_cast<sint32>(
		g_App.m_pPrefs->GetMaxUpload() * 1024u / 10u * static_cast<uint32>(timeSinceLastGaveDataMs) / 1000u );
	sint32 curMaxULRatems = realMaxULRatems + m_iLeftOverBandwidth;
	sint32 allowedBandwidthThisCall = min(curMaxULRatems, realMaxULRatems+MAXFRAGSIZE);

// Which active slot are we giving data each loop
	uint32 dwSlotCounter = 0;
	EnumBlockSendResult eSendResult = BSR_BUSY;

// The loop that gives the fully activated connections data.
	cIt = CopyUploadQueueList.begin();
	while ( (cIt != CopyUploadQueueList.end()) && (allowedBandwidthThisCall > usedBandwidthThisCall)
		&& (eSendResult == BSR_BUSY || eSendResult == BSR_FAILED_CLIENT_LEFT_UQ || eSendResult == BSR_FAILED_NO_REQUESTED_BLOCKS) )
	{
	// Only allow this connection to take what has not already been taken
	// by the other connections above it in the list.
		uint32 allowedThisIteration = allowedBandwidthThisCall-usedBandwidthThisCall;

	// Get the client. Note! Also updates ulpos as a side effect.
		CUpDownClient* cur_client = *cIt;

	// Give data to the upload, if it wants it. Remember how much data we have given this call.
	// The connection wont take any data if it hasn't emptied its buffer since we last gave it data (BSR_BUSY).
	// variable 'eSendResult' is updated by this method (it is a ref EnumBlockSendResult&)

	// LANCAST - Increment LanCount
		if (cur_client->IsOnLAN())
			QtyLANClients++;
		else
			usedBandwidthThisCall += cur_client->SendBlockData(allowedThisIteration, eSendResult);

	//	Count only the non-LAN clients, who are really active
	//	1) the client is transferring (BSR_BUSY)
	//	2) the client will transfer (BSR_OK or BSR_OK_WANTS_MORE_BANDWIDTH)
		if (!cur_client->IsOnLAN() &&
			(eSendResult == BSR_BUSY || eSendResult == BSR_OK || eSendResult == BSR_OK_WANTS_MORE_BANDWIDTH))
		{
			dwSlotCounter++;
		}
		cIt++;
	}

#define MAXTIMESLOTISALLOWEDTOGOWITHOUTDATAMS 2540
// Trickle the unneeded uploads (just give them enough to not time out)
// These downloads are kept connected, in a ready-to-go state, just in case
// one of the fully activated uploads completes/timeouts/ends.
// As soon as there's a little bandwidth leftover, the first one of these
// uploads will go to fully activated state
	for (; cIt != CopyUploadQueueList.end(); cIt++)
	{
	// Get the client. Note! Also updates trickle_client_pos as a side effect.
		CUpDownClient* cur_client = *cIt;

		if (cur_client->GetLastGotULData() + MAXTIMESLOTISALLOWEDTOGOWITHOUTDATAMS < curTick)
		{
			// It's more than 3 seconds since this connection got any data.
			// Feed it a mercy package to prevent it from timing out.
			EnumBlockSendResult eTempSendResult;

			// LANCAST - Increment LanCount
			if (cur_client->IsOnLAN())
				QtyLANClients++;
			else
				usedBandwidthThisCall += cur_client->SendBlockData(MAXFRAGSIZE, eTempSendResult);
		}
	}

// Since we don't save bandwidth for the trickles above, we may
// have used to much bandwidth.
// We may also have used to little bandwidth this round, in that
// case it is saved to the next round.
	m_iLeftOverBandwidth = curMaxULRatems-usedBandwidthThisCall;

// We don't want to long peaks. Don't save to much bandwidth (about a second should be OK)
// Always make sure we can save enough to get a packet through
	sint32 limitSave = MAXFRAGSIZE; // max(realMaxULRatems*10, MAXFRAGSIZE+realMaxULRatems*4);
	if (m_iLeftOverBandwidth > limitSave)
		m_iLeftOverBandwidth = limitSave;

//	Save number of active clients for statistics into the list & sorted vector
	m_activeClientsDeque.push_back(dwSlotCounter);
	vecIt = m_activeClientsSortedVector.begin();
	while (vecIt != m_activeClientsSortedVector.end() && *(vecIt) < dwSlotCounter)
		vecIt++;
	m_activeClientsSortedVector.insert(vecIt, dwSlotCounter);

	if (m_activeClientsDeque.size() > 50)
	{
		uint32 dwRemovedValue = m_activeClientsDeque.front();

		m_activeClientsDeque.pop_front();
		vecIt = m_activeClientsSortedVector.begin();
		while (vecIt != m_activeClientsSortedVector.end() && *(vecIt) != dwRemovedValue)
			vecIt++;
		if (vecIt != m_activeClientsSortedVector.end())
			m_activeClientsSortedVector.erase(vecIt);
	}

//	Control the number of the clients in UL queue
	uint32 dwNumOfULClients = m_UploadingList.size();
	uint32 dwWantedNumberOfTotalUploads = dwNumOfULClients;

// Since the algorithm clears the info about active clients, if one of the client entered or leaved the upload queue. It's required to
// collect some information for new client set. Therefore the number of client will be not changed during 2 sec
	uint32 dwVectorSize = m_activeClientsSortedVector.size();
	if (dwVectorSize > 20)
	{
	//	Use a sorted vector of active clients to get median if enough data was collected
		dwWantedNumberOfTotalUploads = m_activeClientsSortedVector[(dwVectorSize >> 1)] + MINNUMBEROFTRICKLEUPLOADS;
	}

// Add or remove connections as needed. Mostly, this is controlled by the number of fully active uploads.
// note: the client addition placed after queue due to following reason:
// 1) incorrect counting of fresh added client as active
// 2) prevent an influence of the client addition on the upload process due to delay
	if (CanAcceptNewClient(dwNumOfULClients)
		&& waitinglist.GetCount() > 0
		&& dwWantedNumberOfTotalUploads > dwNumOfULClients)
	{
	//	There's not enough open uploads. Open more another one.
		AddClientToUploadQueue();
	}

// Save sent bytes for speed calculations
	if (m_averageTickList.empty() || curTick- m_averageTickList.front() >= UL_QUEUE_DATARATE_SAMPLE_TIME)
	{
		m_averageDataRateList.push_front(g_App.stat_sessionSentBytes);
		m_averageTickList.push_front(curTick);

	// Remove anything older than 40 seconds
		while (curTick-m_averageTickList.back() > 40000)
		{
			m_averageDataRateList.pop_back();
			m_averageTickList.pop_back();
		}

	//	recalculate the average speed after list update
		if (m_averageDataRateList.size() > 1)
			m_dwDataRate = static_cast<uint32>(static_cast<double>(m_averageDataRateList.front()-m_averageDataRateList.back())*1000.0 / (m_averageTickList.front()-m_averageTickList.back()));
		else
			m_dwDataRate = 0;
	}
// Save time to be able to calculate how long between each call
	m_lastGaveDataTick = curTick;

// LANCAST - If we have more than one LAN user were going to upload to them.
// note: an upload for LAN client is placed after speed measurements to prevent statistic disturbance
	if (QtyLANClients > 0)
	{
		bool clientWasReady = false;
		cIt = CopyUploadQueueList.begin();

		for (; cIt != CopyUploadQueueList.end(); cIt++)
		{
		// Get the client. Note! Also updates activeClientsListPos as a side effect.
			CUpDownClient* cur_client = *cIt;
			EnumBlockSendResult eTempSendResult = BSR_BUSY;

		// LANCAST - Just Count LAN Clients
			if (cur_client->IsOnLAN())
				cur_client->SendBlockData(m_guessedMaxLANBandwidth, eTempSendResult);

			if (eTempSendResult == BSR_OK_WANTS_MORE_BANDWIDTH)
				clientWasReady = true;
		}

		if (clientWasReady == false)
		{
			if (m_guessedMaxLANBandwidth > 3000)
				m_guessedMaxLANBandwidth -= 2000;
			else
				m_guessedMaxLANBandwidth = 1000;
		}
		else
		{
			m_guessedMaxLANBandwidth += 2000;
		}
	}

#ifdef OLD_SOCKETS_ENABLED
// enkeyDEV(th1) -L2HAC- lowid side, automatic callback phase
	if (g_App.m_pServerConnect->IsConnected() && g_App.m_pServerConnect->IsLowID())
	{
		for (POSITION pos = waitinglist.GetHeadPosition(); pos != NULL; )
		{
			CUpDownClient* pSource = waitinglist.GetNext(pos);

			if ( pSource->IsL2HACEnabled() && pSource->GetLastL2HACExecution() && pSource->GetL2HACTime()
				 && (curTick - pSource->GetLastL2HACExecution()) > pSource->GetL2HACTime() )
			{
				if ( g_App.m_pListenSocket->TooManySockets()
					 && !(pSource->m_pRequestSocket && pSource->m_pRequestSocket->IsConnected()) )
				{
					 pSource->SetLastL2HACExecution(curTick - pSource->GetL2HACTime() + static_cast<uint32>(static_cast<double>(rand()) / RAND_MAX * 300000.0));
				}
				else
				{
					pSource->DisableL2HAC();
					if (!pSource->HasLowID() && pSource->GetL2HACTime())
						pSource->TryToConnect();
				}
			}
		}
	}
#endif //OLD_SOCKETS_ENABLED

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUploadQueue::CanAcceptNewClient(uint32 dwNumOfUploads)
{
	EMULE_TRY

//	Check if we can allow a new client to start downloading from us
	if (dwNumOfUploads < MIN_UP_CLIENTS_ALLOWED)
		return true;
	else if (dwNumOfUploads >= MAX_UP_CLIENTS_ALLOWED)
		return false;

	if (dwNumOfUploads < (uint32)(g_App.m_pPrefs->GetMaxUpload()*1024/(UPLOAD_LOW_CLIENT_DR*10))+1)
		return true;

	EMULE_CATCH

//	Nope
	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CUploadQueue::~CUploadQueue()
{
	KillTimer(0, m_hTimer);

	if (m_pSaveThread != NULL)
	{
	//	Request thread termination
		SetEvent(m_hQuitEvent);

	//	Wait for thread termination
		if (::WaitForSingleObject(m_pSaveThread->m_hThread, 1000000) == WAIT_TIMEOUT)
		{
			TRACE("Thread known.met thread did not exited on time");
		}
	//	Delete thread object manually, because self termination was disabled
		delete m_pSaveThread;
	}

	if (m_hQuitEvent != NULL)
		::CloseHandle(m_hQuitEvent);

	::DeleteCriticalSection(&m_csUploadQueueList);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
POSITION CUploadQueue::GetWaitingClient(CUpDownClient* client)
{
	EMULE_TRY

	return waitinglist.Find(client);

	EMULE_CATCH

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CUpDownClient* CUploadQueue::GetWaitingClientByIPAndUDPPort(uint32 dwIP, uint16 dwPort)
{
	EMULE_TRY

	for (POSITION pos = waitinglist.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* pSource = waitinglist.GetNext(pos);

		if (dwIP == pSource->GetIP() && dwPort == pSource->GetUDPPort())
			return pSource;
	}

	EMULE_CATCH

	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUploadQueue::AddClientToWaitingQueue(CUpDownClient* client, bool bIgnoreTimelimit)
{
	EMULE_TRY

#ifdef OLD_SOCKETS_ENABLED
//	If we're connected to the server, and we're LOWID, and the client is on a remote server and is not downloading
//	and is not a friend and the queue is longer than 50...
	if ( g_App.m_pServerConnect->IsConnected()
		&& g_App.m_pServerConnect->IsLowID()
		&& !g_App.m_pServerConnect->IsLocalServer(client->GetServerIP(),client->GetServerPort())
		&& client->GetDownloadState() == DS_NONE
		&& !client->IsFriend()
		&& GetWaitingUserCount() > 50 )
	{
		return;
	}
#endif //OLD_SOCKETS_ENABLED

//	Filtering invalid eMule clients
	if ( client->GetMuleVersion() == 0 && client->GetVersion() == 0
		&& (client->GetClientSoft() == SO_EMULE || client->GetClientSoft() == SO_PLUS || client->GetClientSoft() == SO_OLDEMULE) )
	{
		return;
	}

//	L2HAC- lowid side
	client->SetLastL2HACExecution();
	if (!client->HasLowID() && client->GetL2HACTime()) client->EnableL2HAC();

//	LANCAST - We ignore any ban LAN clients may have
	if (!bIgnoreTimelimit && !client->IsOnLAN())
	{
		if (client->IsBanned())
		{
			if (::GetTickCount() - client->GetBanTime() > g_App.m_pPrefs->BadClientBanTime())
				client->UnBan();
			else
				return;
		}
	}

//	Try to add client to our upload queue
	POSITION			pos1, pos2;
	CUpDownClient		*pSource;
	const uint32		dwCurrentTime = ::GetTickCount();
	const uint32		dwWaitingListCount = static_cast<uint32>(waitinglist.GetCount());
	bool				bIsQueueFull = (dwWaitingListCount >= (g_App.m_pPrefs->GetQueueSize() + m_dwBannedCount));

	for (pos1 = waitinglist.GetHeadPosition(); (pos2 = pos1) != NULL;)
	{
		pSource = waitinglist.GetNext(pos1);

	//	Check if client already exists
		if (pSource == client)
		{
		//	Special care is given to LowID clients which missed their upload slot
		//	due to the saving bandwidth on callbacks
			if (client->IsAddNextConnect())
			{
				uint32	dwNumOfULClients = m_UploadingList.size();

			//	One extra slot can be created to accommodate LowID users, as such users
			//	could be skipped when it was actually their turn to get an upload slot
				if ((dwNumOfULClients == 0) || CanAcceptNewClient(--dwNumOfULClients))
				{
					client->SetAddNextConnect(false);
					RemoveFromWaitingQueue(client, true);
					AddClientToUploadQueue(client);
					return;
				}
			}
			client->SendRankingInfo();
			g_App.m_pMDlg->m_wndTransfer.m_ctlQueueList.UpdateClient(client);
			return;
		}
		else if (client->Compare(pSource) != 0) // Another client with same ip or hash
		{
			EIdentState eClientIdentState = client->m_pCredits->GetCurrentIdentState(client->GetIP());
			EIdentState eSourceIdentState = pSource->m_pCredits->GetCurrentIdentState(pSource->GetIP());

			if (eClientIdentState == IS_IDENTIFIED)
			{
				if (eSourceIdentState != IS_IDENTIFIED)
				{
					AddLogLine( LOG_FL_DBG | LOG_RGB_DIMMED, _T("New identified client %s and client in list %s have the same userhash or IP, removed '%s'"),
									client->GetClientNameWithSoftware(), pSource->GetClientNameWithSoftware(), pSource->GetUserName() );
				}
				else
				{
					AddLogLine( LOG_FL_DBG | LOG_RGB_DIMMED, _T("New identified client %s and identified client in list %s have the same Userhash or IP, removed '%s'"),
									client->GetClientNameWithSoftware(), pSource->GetClientNameWithSoftware(), pSource->GetUserName());
				}
				RemoveFromWaitingQueue(pos2, true);
			}
			else	//eClientIdentState != IS_IDENTIFIED
			{
				if (eSourceIdentState == IS_IDENTIFIED)
				{
					AddLogLine( LOG_FL_DBG | LOG_RGB_DIMMED, _T("New client %s and indentified client in list %s have the same userhash or IP, don't let '%s' enter waiting queue"),
									client->GetClientNameWithSoftware(), pSource->GetClientNameWithSoftware(), client->GetUserName() );
				}
				else
				{
					AddLogLine( LOG_FL_DBG | LOG_RGB_DIMMED, _T("New client %s and client in list %s have the same userhash or IP, removed both"),
									client->GetClientNameWithSoftware(), pSource->GetClientNameWithSoftware() );
					RemoveFromWaitingQueue(pos2, true);
				}
				return;
			}

#ifdef OLD_SOCKETS_ENABLED
			if (pSource->m_pRequestSocket == NULL)
			{
				if (!pSource->Disconnected(false))
					delete pSource;	// get rid of the source if it wasn't deleted while disconnecting
			}
#endif //OLD_SOCKETS_ENABLED
			return;
		}

	//	If client does not exist in UL queue & the queue is full we will not be able to add new client,
	//	therefore we need to check if one of the client can be already removed from the queue due to timeout
	//	(no response from client during MAX_PURGEQUEUETIME)
		if (bIsQueueFull && (dwCurrentTime - pSource->GetLastUpRequest()) > MAX_PURGEQUEUETIME)
		{
			RemoveFromWaitingQueue(pos2,true);
#ifdef OLD_SOCKETS_ENABLED
			if (!pSource->m_pRequestSocket)
			{
				if (pSource->Disconnected())
					pSource = NULL;
			}
#endif //OLD_SOCKETS_ENABLED
		//	Since one place now is free, set full status to false
			bIsQueueFull = false;
		}
	}

	if (client->IsDownloading())
	{
	//	He's already downloading and wants probably only another file
		Packet* packet = new Packet(OP_ACCEPTUPLOADREQ, 0);
		g_App.m_pUploadQueue->AddUpDataOverheadFileRequest(packet->m_dwSize);
#ifdef OLD_SOCKETS_ENABLED
		client->m_pRequestSocket->SendPacket(packet,true);
#endif //OLD_SOCKETS_ENABLED
		client->SetLastGotULData();
		return;
	}

//	If queue is not full we can add a client
	if (!bIsQueueFull)
	{
	//	In order to have proper wait time it's needed to set time here for the case
	//	when client will be added directly to the UL-queue
		client->SetWaitStartTime();

	//	Bypass the queue (to avoid sending unrequired Queue Ranking to save traffic)
	//	- for LanCast users 
	//	- when upload queue has less than minimal number of users
		if (client->IsOnLAN() ||
			(waitinglist.IsEmpty() && !client->IsBanned() && (m_UploadingList.size() < MIN_UP_CLIENTS_ALLOWED)))
		{
			AddClientToUploadQueue(client);
		}
		else
		{
			client->SetUploadState(US_ONUPLOADQUEUE);
			waitinglist.AddTail(client);
			client->SendRankingInfo();
			g_App.m_pMDlg->m_wndTransfer.m_ctlQueueList.AddClient(client);
			g_App.m_pMDlg->m_wndTransfer.UpdateUploadHeader();
		}
	}

	EMULE_CATCH
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUploadQueue::RemoveFromUploadQueue(CUpDownClient* client, EnumEndTransferSession eReason, bool updatewindow )
{
	if (client == NULL)
		return false;

	EMULE_TRY

	EnterCriticalSection(&m_csUploadQueueList);
	uint32 dwPrevClientsNum = m_UploadingList.size();
	m_UploadingList.remove(client);
	bool bClientRemoved = (dwPrevClientsNum != m_UploadingList.size());
	LeaveCriticalSection(&m_csUploadQueueList);

	if (bClientRemoved)
	{
		client->SetAddNextConnect(false);
	// clear the information about active clients, if their number in upload queue was changed
		m_activeClientsDeque.clear();
		m_activeClientsSortedVector.clear();

		if (updatewindow)
			g_App.m_pMDlg->m_wndTransfer.m_ctlUploadList.RemoveClient(client);

	// Update stats: if client got some bytes from us the session will be succesfull, otherwise failed
		if (client->GetTransferredUp())
		{
			m_iULSessionSuccessful++;

			if (client->GetTransferredUp() >= PARTSZ32)
				m_iULSessionSuccessfulFullChunk++;
			else
				m_iULSessionSuccessfulPartChunk[eReason]++;
		//	As we transfer something we need to update a upload time
			m_iTotalUploadTime += client->GetUpStartTimeDelay() / 1000;
		}
		else
		{
			if (client->IsThereDataForRemoteClient())
				m_iULSessionFailed[eReason]++;
			else
			{
				m_iULSessionFailedNoDataForRemoteClient++;
				AddLogLine(LOG_FL_DBG, _T("%u Failed upload session with the client %s"), m_iULSessionFailedNoDataForRemoteClient, client->GetClientNameWithSoftware());
			}
		}

		if (g_App.m_pPrefs->LogUploadToFile())
		{
			FILE	*pLogFile = _tfsopen(m_strUploadLogFilePath, _T("ab"), _SH_DENYWR);

			if (pLogFile != NULL)
			{
				COleDateTime	currentTime(COleDateTime::GetCurrentTime());
				CString			strLogLine;
				CKnownFile		*pKnownFile = g_App.m_pSharedFilesList->GetFileByID(client->m_reqFileHash);

				if (pKnownFile == NULL)
					pKnownFile = g_App.m_pKnownFilesList->FindKnownFileByID(client->m_reqFileHash);

				strLogLine.Format( _T("%s,\"%s\",%s,%s,\"%s\",%u,%u,%u,%s,%s,%s,%s,%s\r\n"),
									currentTime.Format(_T("%c")),
									client->GetUserName(),
									HashToString(client->GetUserHash()),
									client->GetFullSoftVersionString(),
									(pKnownFile) ? pKnownFile->GetFileName() : _T(""),
									client->GetSessionUp(),
									client->GetCurrentlyUploadingPart(),
									(client->GetUpStartTimeDelay()/1000),
									YesNoStr(client->IsFriend()),
									YesNoStr(client->IsCommunity()),
									YesNoStr(client->IsOnLAN()),
									YesNoStr(client->HasLowID()),
									(client->GetSessionUp() >= PARTSZ32) ?
									GetResString(IDS_IDENTOK).MakeLower() :
									g_App.m_pMDlg->m_dlgStatistics.GetUpEndReason(eReason) );
#ifdef _UNICODE
			//	Write the Unicode BOM in the beginning if file was created
				if (_filelength(_fileno(pLogFile)) == 0)
					fputwc(0xFEFF, pLogFile);
#endif
				_fputts(strLogLine, pLogFile);
				fclose(pLogFile);
			}
		}

		client->SetUploadState(US_NONE);
	//	Flush sent blocks -- when you stop upload or the socket won't be able to send
		client->FlushSendBlocks();
		client->ClearUploadBlockRequests();

		return true;
	}

	EMULE_CATCH

	return false;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32 CUploadQueue::GetAverageUpTime()
{
	if (m_iULSessionSuccessful)
	{
		return m_iTotalUploadTime/m_iULSessionSuccessful;
	}
	return 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUploadQueue::RemoveFromWaitingQueue(CUpDownClient* client, bool updatewindow)
{
	EMULE_TRY

	POSITION pos = waitinglist.Find(client);
	if (pos)
	{
		RemoveFromWaitingQueue(pos, updatewindow);

		if (updatewindow)
			g_App.m_pMDlg->m_wndTransfer.UpdateUploadHeader();
		client->SetAddNextConnect(false);

		return true;
	}

	EMULE_CATCH

	return false;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUploadQueue::RemoveFromWaitingQueue(POSITION pos, bool updatewindow)
{
	EMULE_TRY

	CUpDownClient* todelete = waitinglist.GetAt(pos);
	waitinglist.RemoveAt(pos);

	if (todelete == NULL)
		return;

	if (todelete->IsBanned())
		todelete->UnBan();

	if (updatewindow)
		g_App.m_pMDlg->m_wndTransfer.m_ctlQueueList.RemoveClient(todelete);

	todelete->SetUploadState(US_NONE);

	EMULE_CATCH
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUploadQueue::DeleteAll()
{
	EMULE_TRY

	waitinglist.RemoveAll();
	m_UploadingList.clear();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint16 CUploadQueue::GetWaitingPosition(CUpDownClient *client)
{
	EMULE_TRY

	if (!IsOnUploadQueue(client))
		return 0;

	unsigned	uiRank = 1;

	if (client->IsBanned())
	{
		uiRank = waitinglist.GetCount();
	}
	else
	{
		uint32 dwClientScore = client->GetScore();

		for (POSITION pos = waitinglist.GetHeadPosition(); pos != NULL;)
		{
			if (waitinglist.GetNext(pos)->GetScore() > dwClientScore)
				uiRank++;
		}
	}
	return static_cast<uint16>(uiRank);

	EMULE_CATCH

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUploadQueue::CompUpDataRateOverhead()
{
	EMULE_TRY

//	update the deque & sum
	m_UpDataOverheadDeque.push_back(m_nUpDataRateMSOverhead);
	m_dwSumUpDataOverheadInDeque += m_nUpDataRateMSOverhead;

//	reset the value
	m_nUpDataRateMSOverhead = 0;

//	check deque length
	while (m_UpDataOverheadDeque.size() > 150)
	{
		m_dwSumUpDataOverheadInDeque -= m_UpDataOverheadDeque.front();
		m_UpDataOverheadDeque.pop_front();
	}

//	calculate average value
	if (m_UpDataOverheadDeque.size() > 10)
		m_nUpDataRateOverhead = 10*m_dwSumUpDataOverheadInDeque/m_UpDataOverheadDeque.size();
	else
		m_nUpDataRateOverhead = 0;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	NOPRM(hwnd); NOPRM(uMsg); NOPRM(idEvent); NOPRM(dwTime);
	EMULE_TRY

//	Don't do anything if the app is shutting down - can cause unhandled exceptions
	if (!g_App.m_pMDlg->IsRunning())
		return;

	g_App.m_pUploadQueue->Process();
	g_App.m_pDownloadQueue->Process();
	g_App.m_pUploadQueue->CompUpDataRateOverhead();
	g_App.m_pDownloadQueue->CompDownDataRateOverhead();

	g_iCounter++;

//	Every second (be aware that every divisor of g_iCounter-modulo has to be a multiple of 10 inside)
	if (g_iCounter % 10 == 0)
	{
#ifdef OLD_SOCKETS_ENABLED
		if (g_App.m_pServerConnect->IsConnecting())
		{
			g_App.m_pServerConnect->TryAnotherConnectionRequest();
		}
#endif

		if (g_App.m_pPrefs->IsWatchClipboard4ED2KLinks())
		{
			g_App.m_pMDlg->m_dlgSearch.SearchClipBoard();
		}

		g_App.m_pMDlg->m_dlgStatistics.UpdateConnectionsStatus();

	//	Every 2 seconds
		if ((g_iCounter & 3) == 0)
		{
#ifdef OLD_SOCKETS_ENABLED
			if (g_App.m_pServerConnect->IsConnecting())
				g_App.m_pServerConnect->CheckForTimeout();
#endif

			g_App.m_pMDlg->m_dlgStatistics.UpdateConnectionStats((double)g_App.m_pUploadQueue->GetDataRate()/1024.0, static_cast<double>(g_App.m_pDownloadQueue->GetDataRate())/1024.0);
		}

	//	Display graphs
		if (g_App.m_pPrefs->GetTrafficOMeterInterval() > 0)
		{
			if (g_iCounter % ((uint32)g_App.m_pPrefs->GetTrafficOMeterInterval() * 10) == 0)
			{
				g_App.m_pMDlg->m_dlgStatistics.SetCurrentRate(g_App.m_pUploadQueue->GetDataRate(), g_App.m_pDownloadQueue->GetDataRate());
			}
		}

	//	Display stats
		if (g_App.m_pMDlg->m_pdlgActive == &g_App.m_pMDlg->m_dlgStatistics && g_App.m_pMDlg->IsWindowVisible())
		{
			if ( (g_App.m_pPrefs->GetStatsInterval() != 0) &&
				(g_iCounter % ((uint32)g_App.m_pPrefs->GetStatsInterval() * 10) == 0) )
			{
				g_App.m_pMDlg->m_dlgStatistics.ShowStatistics();
			}
		}

	//	Save rates every second
		g_App.m_pMDlg->m_dlgStatistics.RecordRate();

#ifdef OLD_SOCKETS_ENABLED
	//	Mobilemule sockets
		g_App.m_pMMServer->Process();
#endif

	//	Every 5 seconds
		if (g_iCounter % 50 == 0)
		{
#ifdef _DEBUG
			if (!AfxCheckMemory())
				g_App.AddLogLine(LOG_FL_EMBEDFMT, RGB_PINK_TXT _T("** ") RGB_LOG_ERROR_TXT _T("Memory corruption detected") RGB_PINK_TXT _T(" **"));
#endif

#ifdef OLD_SOCKETS_ENABLED
			g_App.m_pListenSocket->Process();
#endif
			g_App.m_pSharedFilesList->Process();	// files publishing

			g_App.OnlineSig();
			g_App.m_pMDlg->ShowTransferRate();

		//	Every 90 seconds
			if (g_iCounter % 900 == 0)
			{
			//	Auto Sources per File
				if (g_App.m_pPrefs->IsAutoSourcesEnabled())
				{
					g_App.m_pDownloadQueue->SetAutoSourcesPerFile();
				}
			//	Save emfriends.met (internal 25 minute interval)
				g_App.m_pFriendList->Process();
			}

		//	Every minute
			if (g_iCounter % 600 == 0)
			{
			//	Every 5 minutes
				if (g_iCounter % 3000 == 0)
				{
				//	This function does NOT update the tree!
					g_App.m_pPrefs->SaveStats();
				}

			//	Server keepalive
				g_App.m_pServerConnect->KeepConnectionAlive();

			//	Save clients.met (internal 18 minute interval)
				g_App.m_pClientCreditList->Process();

			//	Save server.met (internal 17 minute interval)
				g_App.m_pServerList->Process();

			//	Scheduled Backup
				if (g_App.m_pPrefs->IsScheduledBackup() && g_App.m_pPrefs->GetScheduledBackupInterval() != 0)
				{
					if (::GetTickCount() > (g_App.m_pUploadQueue->m_dwLastScheduledBackupTick) + g_App.m_pPrefs->GetScheduledBackupInterval() * 3600000)
					{
						g_App.m_pMDlg->RunBackupNow(true);
						g_App.m_pUploadQueue->m_dwLastScheduledBackupTick = ::GetTickCount();
						CLoggable::AddLogLine(LOG_FL_DBG, _T("Scheduled backup performed"));
					}
				}

			//	Scheduler Shift check
				if (g_App.m_pPrefs->IsSCHEnabled())
				{
					CTime curr_t = CTime::GetCurrentTime();
					uint32 secs = curr_t.GetSecond() + 60*curr_t.GetMinute() + 60*60*curr_t.GetHour();

				//	We shouldn't subtract 60 seconds because data type is unsigned
					if (secs+60 > g_App.m_pPrefs->GetSCHShift1() && secs < g_App.m_pPrefs->GetSCHShift1()+60)
					{
					//	Switching to shift1 speeds
						g_App.m_pUploadQueue->SCHShift1UploadCheck();
						g_App.m_pPrefs->SetMaxUpload(g_App.m_pPrefs->GetSCHShift1Upload());
						g_App.m_pPrefs->SetMaxDownload(g_App.m_pPrefs->GetSCHShift1Download());
						g_App.m_pPrefs->SetMaxConnections(g_App.m_pPrefs->GetSCHShift1conn());
						g_App.m_pPrefs->SetMaxDownloadConperFive(g_App.m_pPrefs->GetSCHShift15sec());
					//	Sending message for scheduler shift 1
						CString MessageText;
						MessageText.Format( _T("SCHEDULER: switching to Shift 1 (Max Upload:%.1f Max Download:%.1f Max Connections:%i Max In 5 secs:%i)"),
											static_cast<double>(g_App.m_pPrefs->GetSCHShift1Upload()) / 10.0,
											static_cast<double>(g_App.m_pPrefs->GetSCHShift1Download()) / 10.0,
											g_App.m_pPrefs->GetSCHShift1conn(),
											g_App.m_pPrefs->GetSCHShift15sec() );
						g_App.m_pMDlg->SendMail(MessageText, g_App.m_pPrefs->GetUseSchedulerNotifier(), g_App.m_pPrefs->IsSMTPInfoEnabled());
						g_App.m_pMDlg->ShowNotifier(MessageText, TBN_SCHEDULER, false, g_App.m_pPrefs->GetUseSchedulerNotifier());
						CLoggable::AddLogLine(LOG_FL_DBG, MessageText);
					}
				//	We shouldn't subtract 60 seconds because data type is unsigned
					else if (secs+60 > g_App.m_pPrefs->GetSCHShift2() && secs < g_App.m_pPrefs->GetSCHShift2()+60)
					{
						int dayOfWeek = curr_t.GetDayOfWeek();
						if ( (dayOfWeek==2 && g_App.m_pPrefs->IsSCHExceptMon())
							|| (dayOfWeek==3 && g_App.m_pPrefs->IsSCHExceptTue())
							|| (dayOfWeek==4 && g_App.m_pPrefs->IsSCHExceptWed())
							|| (dayOfWeek==5 && g_App.m_pPrefs->IsSCHExceptThu())
							|| (dayOfWeek==6 && g_App.m_pPrefs->IsSCHExceptFri())
							|| (dayOfWeek==7 && g_App.m_pPrefs->IsSCHExceptSat())
							|| (dayOfWeek==1 && g_App.m_pPrefs->IsSCHExceptSun()) )
						{
							CLoggable::AddLogLine(LOG_FL_DBG, _T("SCHEDULER: day excepted!"));
						}
						else
						{
						//	Switching to shift2 speeds
							g_App.m_pUploadQueue->SCHShift2UploadCheck();
							g_App.m_pPrefs->SetMaxUpload(g_App.m_pPrefs->GetSCHShift2Upload());
							g_App.m_pPrefs->SetMaxDownload(g_App.m_pPrefs->GetSCHShift2Download());
							g_App.m_pPrefs->SetMaxConnections(g_App.m_pPrefs->GetSCHShift2conn());
							g_App.m_pPrefs->SetMaxDownloadConperFive(g_App.m_pPrefs->GetSCHShift25sec());
						//	Sending message for scheduler shift 2
							CString MessageText;
							MessageText.Format( _T("SCHEDULER: switching to Shift 2 (Max Upload:%.1f Max Download:%.1f Max Connections:%i Max In 5 secs:%i)"),
												static_cast<double>(g_App.m_pPrefs->GetSCHShift2Upload()) / 10.0,
												static_cast<double>(g_App.m_pPrefs->GetSCHShift2Download()) / 10.0,
												g_App.m_pPrefs->GetSCHShift2conn(),
												g_App.m_pPrefs->GetSCHShift25sec() );
							g_App.m_pMDlg->SendMail(MessageText, g_App.m_pPrefs->GetUseSchedulerNotifier(), g_App.m_pPrefs->IsSMTPInfoEnabled());
							g_App.m_pMDlg->ShowNotifier(MessageText, TBN_SCHEDULER, false, g_App.m_pPrefs->GetUseSchedulerNotifier());
							CLoggable::AddLogLine(LOG_FL_DBG, MessageText);
						}
					}
				}
			}
		}
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUploadQueue::FindSourcesForFileById(CTypedPtrList<CPtrList, CUpDownClient*>* srclist, const uchar* filehash)
{
	EMULE_TRY

	for (ClientList::const_iterator cIt = m_UploadingList.begin(); cIt != m_UploadingList.end(); cIt++)
	{
		CUpDownClient	*pPotentialSource = *cIt;

		if (md4cmp(pPotentialSource->m_reqFileHash, filehash) == 0)
			srclist->AddTail(pPotentialSource);
	}

	POSITION pos = waitinglist.GetHeadPosition();

	while (pos)
	{
		CUpDownClient	   *pPotentialSource = waitinglist.GetNext(pos);

		if (md4cmp(pPotentialSource->m_reqFileHash, filehash) == 0)
			srclist->AddTail(pPotentialSource);
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetUploadFilePartsAvailability() returns availability of file parts reported by downloaders
//		Params:
//			pdwStatuses   - array to be filled (must be zero initialized by caller);
//			dwPartCnt     - number of file parts (array size);
//			pbyteFileHash - file hash to process.
void CUploadQueue::GetUploadFilePartsAvailability(uint32 *pdwStatuses, uint32 dwPartCnt, const byte *pbyteFileHash)
{
	EMULE_TRY

	CTypedPtrList<CPtrList, CUpDownClient*> srclist;
	CUpDownClient	*pClient;
	const byte		*pbyteUpStatus;

	FindSourcesForFileById(&srclist, pbyteFileHash);

	for (POSITION pos = srclist.GetHeadPosition(); pos != NULL;)
	{
		pClient = srclist.GetNext(pos);
		if ( (pClient->GetAvailUpPartCount() != 0) &&
			((pbyteUpStatus = pClient->GetUpPartStatus()) != NULL) &&
			(pClient->GetUpPartCount() == dwPartCnt) )
		{
			for (uint32 i = 0; i < dwPartCnt; i++)
				pdwStatuses[i] += pbyteUpStatus[i];
		}
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT SaveSourcesThread(HANDLE lpArg)
{
#ifdef EP_SPIDERWEB
//	Setup Structured Exception handler for the this thread
	_set_se_translator(StructuredExceptionHandler);
#endif

	g_App.m_pPrefs->InitThreadLocale();

	HANDLE	hTermEv = (reinterpret_cast<CUploadQueue*>(lpArg))->m_hQuitEvent;

//	Do the work every 30 minutes or quit by terminating event
	while (::WaitForSingleObject(hTermEv, 30*60*1000) == WAIT_TIMEOUT)
	{
	//	Check to avoid rare crash on exit as m_pKnownFilesList is deleted before m_pUploadQueue
		if (g_App.m_pKnownFilesList == NULL)
			break;
	//	Save the information to the drive
		g_App.m_pKnownFilesList->Save();
	}

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUploadQueue::SCHShift1UploadCheck()
{
	if (g_App.m_pPrefs->GetSCHShift1Upload() != 0)
	{
		g_App.m_pPrefs->SetSCHShift1Download(TieUploadDownload(g_App.m_pPrefs->GetSCHShift1Upload(), g_App.m_pPrefs->GetSCHShift1Download()));
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUploadQueue::SCHShift2UploadCheck()
{
	if (g_App.m_pPrefs->GetSCHShift2Upload() != 0)
	{
		g_App.m_pPrefs->SetSCHShift2Download(TieUploadDownload(g_App.m_pPrefs->GetSCHShift2Upload(), g_App.m_pPrefs->GetSCHShift2Download()));
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
POSITION CUploadQueue::GetHeadPosition(void)
{
	return waitinglist.GetHeadPosition();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CUpDownClient* CUploadQueue::GetNext(POSITION& pos)
{
	return waitinglist.GetNext(pos);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUploadQueue::IsDownloading(CUpDownClient* pClient)
{
	for (ClientList::const_iterator cIt = m_UploadingList.begin(); cIt != m_UploadingList.end(); cIt++)
	{
		if (pClient == *cIt)
			return true;
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Create a copy of Upload Queue for current iteration to prevent any interference during data upload
void CUploadQueue::GetCopyUploadQueueList(ClientList *pCopy)
{
	EnterCriticalSection(&m_csUploadQueueList);

	pCopy->insert(pCopy->begin(), m_UploadingList.begin(), m_UploadingList.end());

	LeaveCriticalSection(&m_csUploadQueueList);
}
