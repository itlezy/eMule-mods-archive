//this file is part of NeoMule
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
#include <Mmsystem.h>
#include <math.h>
#include "emule.h"
#include "UploadBandwidthThrottler.h"
#include "opcodes.h"
#include "LastCommonRouteFinder.h"
#include "OtherFunctions.h"
#include "emuledlg.h"
#include "preferences.h"
#include "Neo/Neopreferences.h"
#include "Neo/Functions.h"
#include "updownclient.h"
#include "listensocket.h"
#include "BandwidthControl.h" 
#include "ThrottlerHelpers.h"
#include "log.h"
#include "UploadQueue.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->

/**
* The constructor starts the thread.
*/
UploadBandwidthThrottler::UploadBandwidthThrottler(void) {
	recalculate = 1;
	needslot = false;
	nextslotfull = true;

	//m_SentBytesSinceLastCall = 0; // removed // NEO: ASM - [AccurateSpeedMeasure]

	threadEndedEvent = new CEvent(0, 1);
	pauseEvent = new CEvent(TRUE, TRUE);

	doRun = true;
	pThread = AfxBeginThread(RunProc, (LPVOID)this, NeoPrefs.GetBCPriorityUp(), 0, CREATE_SUSPENDED);
}

/**
* The destructor stops the thread. If the thread has already stoppped, destructor does nothing.
*/
UploadBandwidthThrottler::~UploadBandwidthThrottler(void) {
	//EndThread();
	delete threadEndedEvent;
	delete pauseEvent;
}

/**
* @param socket the address to the socket that should be added to the list. If the address is NULL,
*               this method will do nothing.
*/
void UploadBandwidthThrottler::AddToStandardList(ThrottledFileSocket* socket, bool bFull) {
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	if(socket && socket->IsLanSocket())
		return;
#endif //LANCAST // NEO: NLC END

	sendLocker.Lock();

	needslot = false; // we got what we wanted
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
	if(!NeoPrefs.IsCheckSlotDatarate() || m_UploadQueue_list.GetCount() == 1)
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
		recalculate = 1; // ckeck do we need more

 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
	if(socket->m_eSlotType != ST_NORMAL) 
		bFull = true;
 #endif // BW_MOD // NEO: BM END

	AddToStandardListNoLock(socket, bFull);

	socket->m_SocketSlope = 0;
	// NEO: NUSM - [NeoUploadSlotManagement]
	if(socket->StandardPacketQueueIsEmpty()==false)
		socket->m_IsReady = true;
	else
		socket->m_IsReady = false;
	// NEO: NUSM END

	sendLocker.Unlock();
}

/**
* Add a socket to the list of sockets that have upload slots. NOT THREADSAFE!
* The main thread will continously call send on these sockets, to give them chance to work off their queues.
*
* @param socket the address to the socket that should be added to the list. If the address is NULL,
*               this method will do nothing.
*/

void UploadBandwidthThrottler::AddToStandardListNoLock(ThrottledFileSocket* socket, bool bFull) {
	ASSERT(!socket || socket->m_eSlotState == SS_NONE);
	if (socket && !socket->onUpQueue){
		socket->ResetProcessRate();
		socket->onUpQueue = true;
		ASSERT(!m_UploadQueue_list.Find(socket));
		if(bFull || nextslotfull){
			nextslotfull = false;
			m_UploadQueue_list.AddHead(socket);

			if(socket->m_eSlotState != SS_FULL){
				socket->m_eSlotState = SS_FULL;
				ASSERT(!m_UploadQueue_list_full.Find(socket));
				m_UploadQueue_list_full.AddHead(socket);
			}else
				ASSERT(false);
		}else{
			m_UploadQueue_list.AddTail(socket);
			socket->m_eSlotState = SS_TRICKLE;
		}
		socket->m_SocketSlope = 0;
	}
}


/**
* Look do we need more sockets for out current bandwidth
*
* @return are additional slots needed
*
*/
bool UploadBandwidthThrottler::ForceNewClient()
{
	bool _needslot;
	sendLocker.Lock();
	_needslot = needslot;
	sendLocker.Unlock();
	return _needslot;
}

/**
* Inform trotheler that currently no new slot will be given.
*
*/
void UploadBandwidthThrottler::SetNoNeedSlot()
{
	sendLocker.Lock();
	needslot=false;
	sendLocker.Unlock();
}

/**
* Find out the number of slots that are on the full list.
*
* @return the number of fully activated slots
*/
UINT UploadBandwidthThrottler::GetNumberOfFullyActivatedSlots()
{
	UINT _activatedslots;
	sendLocker.Lock();
	_activatedslots = m_UploadQueue_list_full.GetCount();
	sendLocker.Unlock();
	return _activatedslots;
}

/**
* Get the first socket on the full list
*
* @return adres to the first socket on the list
*/
ThrottledFileSocket* UploadBandwidthThrottler::GetPrioritySocket()
{
	ThrottledFileSocket* _prioritysocket = NULL;
	sendLocker.Lock();
	if(m_UploadQueue_list_full.GetCount())
		_prioritysocket = m_UploadQueue_list_full.GetHead();
	sendLocker.Unlock();
	return _prioritysocket;
}

/**
* Remove a socket from the list of sockets that have upload slots.
*
* @param socket the address of the socket that should be removed from the list. If this socket
*               does not exist in the list, this method will do nothing.
*/
bool UploadBandwidthThrottler::RemoveFromStandardList(ThrottledFileSocket* socket) {
	bool returnValue = false;
	if (doRun){
		sendLocker.Lock();

		if(socket && socket->m_eSlotState == SS_FULL) // If we droped a full socket 
			recalculate = 2; // lets check do we need a new one

		returnValue = RemoveFromStandardListNoLock(socket);

		//if(socket && returnValue && !socket->ControlPacketQueueIsEmpty() && !socket->onUpControlQueue)
		if(socket && returnValue && (!socket->ControlPacketQueueIsEmpty() || !socket->StandardPacketQueueIsEmpty()) && !socket->onUpControlQueue){ // socket removed but still packets waiting
			socket->onUpControlQueue = true;
			m_ControlQueue_list.AddHead(socket); // requeue it to complete seinding
		}

		sendLocker.Unlock();
	}
	return returnValue;
}

/**
* Remove a socket from the list of sockets that have upload slots. NOT THREADSAFE!
* This is an internal method that doesn't take the necessary lock before it removes
* the socket. This method should only be called when the current thread already owns
* the sendLocker lock!
*
* @param socket address of the socket that should be removed from the list. If this socket
*               does not exist in the list, this method will do nothing.
*/
bool UploadBandwidthThrottler::RemoveFromStandardListNoLock(ThrottledFileSocket* socket) {
	bool returnValue = false;
	if (socket && socket->onUpQueue){
		// Find the slot
		POSITION pos = m_UploadQueue_list.Find(socket);
		if (pos){
			returnValue = true;
			socket->onUpQueue = false;
			m_UploadQueue_list.RemoveAt(pos);
		}
		else
			ASSERT(false || !doRun);

		if(socket->m_eSlotState == SS_FULL){ // Find the full slot
			/*POSITION*/ pos = m_UploadQueue_list_full.Find(socket);
			if (pos){
				m_UploadQueue_list_full.RemoveAt(pos);
			}
			else
				ASSERT(false || !doRun);
		}

		socket->m_eSlotState = SS_NONE;
	}
	return returnValue;
}

/**
* Manage the socket states, full/trickle.
* The function will upgrade fit sockts from trickle to full state
* When a socket is blockung than it will be shifted to the botom of the list
* In case there are not ehough usefull sockets on the list this function will request more slots
*
* @param lock shell we get the critical section or do we already own it
*
* @param count number of socket to upgrade, default 1
*
* @param reset cleanup the current list and readd the proper amount of sockets
*
* @return succesfuly upgraded at least one socket, but must not be the demanded value
*/

bool UploadBandwidthThrottler::SetNextTrickleToFull(bool lock, UINT count, bool reset)
{
	bool returnValue = false;
	if(lock) {
		// Get critical section
		sendLocker.Lock();
	}

	if(reset){ // we are resetiung our list, cause we have to much slots
		for(;!m_UploadQueue_list_full.IsEmpty();)
			m_UploadQueue_list_full.RemoveHead()->m_eSlotState = SS_TRICKLE;
	}

	POSITION pos = m_UploadQueue_list.FindIndex(m_UploadQueue_list_full.GetCount());
	for(UINT i=m_UploadQueue_list.GetCount()-m_UploadQueue_list_full.GetCount();i>0;i--){
		POSITION removePos = pos;
		ThrottledFileSocket* socket = m_UploadQueue_list.GetNext(pos);

		if(socket->m_eSlotState == SS_BLOCKED && socket->GetAvgRatio() <= 0.20f){ // socked was blocking and havn't regain fitnes till now
			m_UploadQueue_list.RemoveAt(removePos);
			m_UploadQueue_list.AddTail(socket); // move it to the tail of our list
		}
		else{
			socket->m_eSlotState = SS_FULL;
			m_UploadQueue_list_full.AddTail(socket); 
			returnValue = true; // socces trickle found and upgraded
			count--;
			if(count == 0)
				break;
		}
	}

	if(count > 0 && !NeoPrefs.IsMinimizeOpenedSlots()){ // we don't have enough usefull sockets
		needslot = true;
		nextslotfull = true;
	}

	if(lock) {
		// End critical section
		sendLocker.Unlock();
	}
	return returnValue;
}

/**
* The function will upgrade fit sockts from trickle to full state
*
* @param socket socket to add to full list
*
* @param lock shell we get the critical section or do we already own it
*
*/
void UploadBandwidthThrottler::SetTrickleToFull(ThrottledFileSocket* socket, bool lock)
{
	if(lock) {
		// Get critical section
		sendLocker.Lock();
	}

	// Find the slot
	POSITION pos = m_UploadQueue_list.Find(socket);
	if (pos){
		m_UploadQueue_list.RemoveAt(pos);
		m_UploadQueue_list.AddHead(socket);
	}
	else
		ASSERT(false || !doRun);

	socket->m_eSlotState = SS_FULL;
	m_UploadQueue_list_full.AddTail(socket); 

	if(lock) {
		// End critical section
		sendLocker.Unlock();
	}
}

/**
* The function will remove a socket from the full list and put it on botom of the list
* When we cumulate the the bandwidth or and the bandwidth was not enough for this socket 
* Or when the socket is blocking, have a busy ratio belog 0.10 for some long time
* It will be removed form the full list to make space for fit sockets
*
* @param socket socket to remove from full list
*
* @param lock shell we get the critical section or do we already own it
*
* @param block does the socket have been removed becouse it have blocked
*
*/
void UploadBandwidthThrottler::SetFullToTrickle(ThrottledFileSocket* socket, bool lock, bool block)
{
	if(lock) {
		// Get critical section
		sendLocker.Lock();
	}

	socket->m_eSlotState = block ? SS_BLOCKED : SS_TRICKLE;

	// Find the slot
	POSITION pos = m_UploadQueue_list.Find(socket);
	if (pos){
		m_UploadQueue_list.RemoveAt(pos);
		m_UploadQueue_list.AddTail(socket);
	}
	else
		ASSERT(false || !doRun);

	// find full socket
	/*POSITION*/ pos = m_UploadQueue_list_full.Find(socket);
	if (pos){
		m_UploadQueue_list_full.RemoveAt(pos);
	}
	else
		ASSERT(false || !doRun);

	if(lock) {
		// End critical section
		sendLocker.Unlock();
	}
}

/**
* Notifies the send thread that it should try to call controlpacket send
* for the supplied socket. It is allowed to call this method several times
* for the same socket, without having controlpacket send called for the socket
* first.
*
* @param socket address to the socket that requests to have controlpacket send
*               to be called on it
*/

// The Bandwidth Trotheler does NOT have a *temp* control queue
// *NEVER* try to *insert* socket when sendLocker of the socket is *locked*, 
// or it will result in a *DEAD LOCK* !!!
bool UploadBandwidthThrottler::QueueForSendingControlPacket(ThrottledControlSocket* socket, bool hasSent) {
	if (socket){
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
		if(socket->IsLanSocket())
			return true;
#endif //LANCAST // NEO: NLC END

		// Get critical section
		sendLocker.Lock();
		if(doRun && !socket->onUpControlQueue && !socket->onUpQueue) {
			socket->onUpControlQueue = true;
			if(hasSent) {
				m_ControlQueue_list.AddHead(socket);
			} else {
				m_ControlQueue_list.AddTail(socket);
			}
		}
		bool ret = socket->onUpControlQueue || socket->onUpQueue;
		// End critical section
		sendLocker.Unlock();
		return ret;
	}
	return false;
}

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
bool UploadBandwidthThrottler::QueueForLanPacket(ThrottledFileSocket* socket) {
	if (socket){
		// Get critical section
		sendLocker.Lock();
		bool ret = socket->onUpLanQueue;
		if(doRun && socket && !socket->onUpLanQueue) {
			socket->onUpLanQueue = true;
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
			if(socket->IsVoodooSocket())
				m_LanQueue_list.AddHead(socket);
			else
#endif // VOODOO // NEO: VOODOO END
				m_LanQueue_list.AddTail(socket);
			ret = socket->onUpLanQueue;
		}

		// End critical section
		sendLocker.Unlock();
		return ret;
	}
	return false;
}

void UploadBandwidthThrottler::RemoveFromLanPacketQueueNoLock(ThrottledFileSocket* socket){
	if(socket && socket->onUpLanQueue){
		POSITION pos = m_LanQueue_list.Find(socket);
		if (pos){
			socket->onUpLanQueue = false;
			m_LanQueue_list.RemoveAt(pos);
		}
	}
}
#endif //LANCAST // NEO: NLC END

/**
* Remove the socket from all lists and queues. This will make it safe to
* erase/delete the socket. It will also cause the main thread to stop calling
* send() for the socket.
*
* @param socket address to the socket that should be removed
*/
void UploadBandwidthThrottler::RemoveFromAllQueuesNoLock(ThrottledControlSocket* socket) {
	if (socket && socket->onUpControlQueue){
		// Remove this socket from control packet queue
		POSITION pos = m_ControlQueue_list.Find(socket);
		if (pos){
			socket->onUpControlQueue = false;
			m_ControlQueue_list.RemoveAt(pos);
		}
	}

	if (socket && socket->onBlockedQueue){
		POSITION pos = m_BlockedQueue_list.Find(socket);
		if (pos){
			socket->onBlockedQueue = false;
			m_BlockedQueue_list.RemoveAt(pos);
		}
	}
}

void UploadBandwidthThrottler::RemoveFromAllQueues(ThrottledControlSocket* socket) { 
	if (doRun){
		// Get critical section
		sendLocker.Lock();

		RemoveFromAllQueuesNoLock(socket);

		// End critical section
		sendLocker.Unlock();
	}
};

void UploadBandwidthThrottler::RemoveFromAllQueues(ThrottledFileSocket* socket) { 
	if (doRun){
		// Get critical section
		sendLocker.Lock();

		if(socket && socket->m_eSlotState == SS_FULL) // If we droped a full socket 
			recalculate = 2; // lets check do we need a new one

		RemoveFromAllQueuesNoLock(socket);
		RemoveFromStandardListNoLock(socket);
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
		RemoveFromLanPacketQueueNoLock(socket);
#endif //LANCAST

		// End critical section
		sendLocker.Unlock();
	}
};

/**
* Make the thread exit. This method will not return until the thread has stopped
* looping. This guarantees that the thread will not access the CEMSockets after this
* call has exited.
*/
void UploadBandwidthThrottler::EndThread() {
	sendLocker.Lock();

	// signal the thread to stop looping and exit.
	doRun = false;

	sendLocker.Unlock();

	Pause(false);

	// wait for the thread to signal that it has stopped looping.
	threadEndedEvent->Lock();
}

void UploadBandwidthThrottler::StartThread() {
	pThread->ResumeThread();
}


void UploadBandwidthThrottler::Pause(bool paused) {
	if(paused) {
		pauseEvent->ResetEvent();
	} else {
		pauseEvent->SetEvent();
	}
}

/**
* Start the thread. Called from the constructor in this class.
*
* @param pParam
*
* @return
*/
UINT AFX_CDECL UploadBandwidthThrottler::RunProc(LPVOID pParam) {
	DbgSetThreadName("UploadBandwidthThrottler");
	InitThreadLocale();
	// NEO: STS - [SlugFillerThreadSafe] -- Xanatos -->
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return 0;
	// NEO: STS END <-- Xanatos --
	UploadBandwidthThrottler* uploadBandwidthThrottler = (UploadBandwidthThrottler*)pParam;

	return uploadBandwidthThrottler->RunInternal();
}

/**
* @return always returns 0.
*/
UINT UploadBandwidthThrottler::RunInternal() {
	DWORD lastLoopTick = timeGetTime();

	float allowedDataRate = 0;
	float allowedDataRate_old = 0;
	float wantedSlotSpeed = 0;
	float wantedTrickleSpeed = 0;
	toSend = 0;
 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
	for(int i=0; i<ST_NORMAL; i++)
		toSendModerated[i] = 0;
 #endif // BW_MOD // NEO: BM END

	DWORD lastTickBandwidthReached = timeGetTime();

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	float allowedLanDataRate = 0;
	toLanSend = 0;
#endif //LANCAST // NEO: NLC END

	// Max Uplaod estimation
	m_nEstiminatedLimit = 0;
	int nSlotsBusyLevel = 0;
	DWORD nUploadStartTime = 0;
    uint32 numberOfConsecutiveUpChanges = 0;
    uint32 numberOfConsecutiveDownChanges = 0;
    uint32 changesCount = 0;
    uint32 loopsCount = 0;

	DWORD lastReOrder = lastLoopTick;
	DWORD lastCleanUp = lastLoopTick;

	while(doRun) {
		pauseEvent->Lock();

		DWORD timeSinceLastLoop = timeGetTime() - lastLoopTick;

		UINT cumulate = NeoPrefs.IsCumulateBandwidth(); // comulate bandwidth on a few slots
		bool minimize = NeoPrefs.IsMinimizeOpenedSlots(); // minimize number of open slots
		bool includeoverhead = NeoPrefs.IsIncludeOverhead();
		UINT hypersending = NeoPrefs.UseBlockedQueue(); // acceletare sending of packets on busy sockets
		bool increasetricklespeed = NeoPrefs.IsIncreaseTrickleSpeed();
 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
		UINT moderatespeed[ST_NORMAL+1] = {NeoPrefs.IsSeparateReleaseBandwidth(),NeoPrefs.IsSeparateFriendBandwidth(),FALSE}; // false becouse ST_NORMAL is not moderated !!!
		ASSERT(moderatespeed[ST_NORMAL] == FALSE); // moderation for ST_NORMAL must be always off or you wil have a crash
 #endif // BW_MOD // NEO: BM END

		// get the slot speed
		if(wantedSlotSpeed != NeoPrefs.GetUploadPerSlots()){
			wantedSlotSpeed = NeoPrefs.GetUploadPerSlots();
			if(wantedSlotSpeed==0)
				wantedSlotSpeed=1; //prevent division by zero
			recalculate = 1;
		}

		// get the trickle speed
		if(wantedTrickleSpeed != NeoPrefs.GetIncreaseTrickleSpeed()){
			wantedTrickleSpeed = NeoPrefs.GetIncreaseTrickleSpeed();
			recalculate = 1;
		}

		// Get current speed from UploadSpeedSense
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
		if(theApp.bandwidthControl == NULL) // not ready yet :/
			continue;

		allowedDataRate = theApp.bandwidthControl->GetMaxUpload();
#else
		if (thePrefs.IsDynUpEnabled())
			allowedDataRate = theApp.lastCommonRouteFinder->GetUpload()/1024.0f;
		else
			allowedDataRate = thePrefs.GetMaxUpload();
#endif // NEO_BC // NEO: NBC END

		if(allowedDataRate_old != allowedDataRate && pow(allowedDataRate_old - allowedDataRate,2) > pow(wantedSlotSpeed,2)){
			allowedDataRate_old = allowedDataRate;
			recalculate = 1; 
		}
		
		// Max Uplaod estimation
        // When no upload limit has been set in options, try to guess a good upload limit.
		if (allowedDataRate == UNLIMITED) {

			// check busy level for all the slots (WSAEWOULDBLOCK status)
			uint32 cBusy = 0;
			uint32 nCanSend = 0;

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
			float  fCurUpload = (includeoverhead ? theApp.bandwidthControl->GetCurrUpload() : theApp.bandwidthControl->GetCurrDataUpload());
#else
			float  fCurUpload = (float)theApp.uploadqueue->GetDatarate() / 1024.0f;
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
			uint32 uSlotLimit = (cumulate || minimize) ? 3 : (uint32)(fCurUpload/wantedSlotSpeed);
			if(uSlotLimit < 3)
				uSlotLimit = 3;

			sendLocker.Lock();
			uint32 i = 0;
			for(POSITION pos = m_UploadQueue_list_full.GetHeadPosition(); pos != 0 && i < uSlotLimit;i++){
				ThrottledFileSocket* cur_socket = m_UploadQueue_list_full.GetNext(pos);
				if(cur_socket->HasQueues()){
					nCanSend++;
					if(cur_socket->IsBusy())
						cBusy++;
				}
			}
			sendLocker.Unlock();

			// if this is kept, the loop above can be a little optimized (don't count nCanSend, just use nCanSend = GetSlotLimit(theApp.uploadqueue->GetDatarate())
			if(nCanSend < uSlotLimit)
				nCanSend = uSlotLimit;

            loopsCount++;

            //DEBUG_ONLY(theApp.QueueDebugLogLine(false,_T("Throttler: busy: %i/%i nSlotsBusyLevel: %i Guessed limit: %0.5f changesCount: %i loopsCount: %i"), cBusy, nCanSend, nSlotsBusyLevel, m_nEstiminatedLimit, changesCount, loopsCount));

            if(nCanSend > 0) {
			    float fBusyPercent = ((float)cBusy/(float)nCanSend) * 100;
                if ( (cBusy > 2 && fBusyPercent > 75.00f) && nSlotsBusyLevel < 255){
				    nSlotsBusyLevel++;
                    changesCount++;
					//if(nSlotsBusyLevel%25==0){ 
					//	DEBUG_ONLY(theApp.QueueDebugLogLine(false,_T("Throttler: nSlotsBusyLevel: %i Guessed limit: %0.5f changesCount: %i loopsCount: %i"), nSlotsBusyLevel, m_nEstiminatedLimit, changesCount, loopsCount));
					//}
			    }
			    else if ( (cBusy <= 2 || fBusyPercent < 25.00f) && nSlotsBusyLevel > (-255)){
				    nSlotsBusyLevel--;
                    changesCount++;
					//if(nSlotsBusyLevel%25==0){
					//	DEBUG_ONLY(theApp.QueueDebugLogLine(false,_T("Throttler: nSlotsBusyLevel: %i Guessed limit: %0.5f changesCount %i loopsCount: %i"), nSlotsBusyLevel, m_nEstiminatedLimit, changesCount, loopsCount));
					//}
                }
			}

            if(nUploadStartTime == 0) {
		        if (m_UploadQueue_list_full.GetCount() >= 3)
			        nUploadStartTime = timeGetTime();
            }
			else if(timeGetTime()- nUploadStartTime > SEC2MS(60)){
				if (m_nEstiminatedLimit == 0){ // no autolimit was set yet
					if (nSlotsBusyLevel >= 250){ // sockets indicated that the BW limit has been reached
						m_nEstiminatedLimit = fCurUpload;
						allowedDataRate = min(m_nEstiminatedLimit, allowedDataRate);
						nSlotsBusyLevel = -200;
                        //DEBUG_ONLY(theApp.QueueDebugLogLine(false,_T("Throttler: Set inital estimated limit to %0.5f changesCount: %i loopsCount: %i"), m_nEstiminatedLimit, changesCount, loopsCount));
                        changesCount = 0;
                        loopsCount = 0;
					}
				}
				else{
                    if (nSlotsBusyLevel > 250){
                        if(changesCount > 500 || changesCount > 300 && loopsCount > 1000 || loopsCount > 2000) {
                            numberOfConsecutiveDownChanges = 0;
                        }
                        numberOfConsecutiveDownChanges++;
						float changeDelta = (numberOfConsecutiveDownChanges < 2) ? 0.050f : (numberOfConsecutiveDownChanges > 10) ? 2.0f : ((sqrt((float)numberOfConsecutiveDownChanges))-1.256f);

                        // Don't lower speed below 1 KBytes/s
                        if(m_nEstiminatedLimit < changeDelta + 1.024f) {
                            if(m_nEstiminatedLimit > 1.024f) {
                                changeDelta = m_nEstiminatedLimit - 1.024f;
                            } else {
                                changeDelta = 0;
                            }
                        }
                        ASSERT(m_nEstiminatedLimit >= changeDelta + 1.024f);
    					m_nEstiminatedLimit -= changeDelta;

                        //DEBUG_ONLY(theApp.QueueDebugLogLine(false,_T("Throttler: REDUCED limit #%i with %i bytes to: %0.5f changesCount: %i loopsCount: %i"), numberOfConsecutiveDownChanges, changeDelta, m_nEstiminatedLimit, changesCount, loopsCount));

                        numberOfConsecutiveUpChanges = 0;
						nSlotsBusyLevel = 0;
                        changesCount = 0;
                        loopsCount = 0;
					}
                    else if (nSlotsBusyLevel < (-250)){
                        if(changesCount > 500 || changesCount > 300 && loopsCount > 1000 || loopsCount > 2000) {
                            numberOfConsecutiveUpChanges = 0;
                        }
                        numberOfConsecutiveUpChanges++;
						float changeDelta = (numberOfConsecutiveUpChanges < 2) ? 0.050f : (numberOfConsecutiveUpChanges > 10) ? 2.0f : ((sqrt((float)numberOfConsecutiveUpChanges))-1.256f);

                        // Don't raise speed unless we are under current allowedDataRate
                        if(m_nEstiminatedLimit+changeDelta > allowedDataRate) {
                            if(m_nEstiminatedLimit < allowedDataRate) {
                                changeDelta = allowedDataRate - m_nEstiminatedLimit;
                            } else {
                                changeDelta = 0;
                            }
                        }
                        ASSERT(m_nEstiminatedLimit < allowedDataRate && m_nEstiminatedLimit+changeDelta <= allowedDataRate || m_nEstiminatedLimit >= allowedDataRate && changeDelta == 0);
                        m_nEstiminatedLimit += changeDelta;

                        //DEBUG_ONLY(theApp.QueueDebugLogLine(false,_T("Throttler: INCREASED limit #%i with %i bytes to: %0.5f changesCount: %i loopsCount: %i"), numberOfConsecutiveUpChanges, changeDelta, m_nEstiminatedLimit, changesCount, loopsCount));

                        numberOfConsecutiveDownChanges = 0;
						nSlotsBusyLevel = 0;
                        changesCount = 0;
                        loopsCount = 0;
					}

					allowedDataRate = min(m_nEstiminatedLimit, allowedDataRate);
				} 

            }

			if(cBusy == nCanSend && m_UploadQueue_list_full.GetCount() > 0) {
				allowedDataRate = 0;
				if(nSlotsBusyLevel < 125) {
					nSlotsBusyLevel = 125;
					//DEBUG_ONLY(theApp.QueueDebugLogLine(false,_T("Throttler: nSlotsBusyLevel: %i Guessed limit: %0.5f changesCount %i loopsCount: %i (set due to all slots busy)"), nSlotsBusyLevel, m_nEstiminatedLimit, changesCount, loopsCount));
				}
			}
		}
		// estimation end


		uint32 minFragSize = theApp.bandwidthControl->GetMSS();
		uint32 doubleSendSize = minFragSize * (NeoPrefs.UseDoubleSendSize() ? 2 : 1); // send two packages at a time so they can share an ACK
		uint32 minSendSize = minFragSize / 2;
		if(allowedDataRate < 6.0F) {
			minFragSize = 536;
            doubleSendSize = minFragSize; // don't send two packages at a time at very low speeds to give them a smoother load
			minSendSize = 128;
		}
		//else if (allowedDataRate >= 60.0){ // use larger packets on very fast connections
		//	minFragSize *= 2;
		//	doubleSendSize *= 2;
		//}

		DelayLoopTime(NeoPrefs.GetBCTimeUp(),allowedDataRate,toSend,timeSinceLastLoop,minFragSize);

		const DWORD thisLoopTick = timeGetTime();
		timeSinceLastLoop = thisLoopTick - lastLoopTick;
		lastLoopTick = thisLoopTick;

		CalcLoopAddOn(allowedDataRate,toSend,timeSinceLastLoop);

 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
		if(moderatespeed[ST_RELEASE]){
			float allowedReleaseDataRate = allowedDataRate * NeoPrefs.GetReleaseBandwidthPercentage() / 100;
			CalcLoopAddOn(allowedReleaseDataRate,toSendModerated[ST_RELEASE],timeSinceLastLoop);
		}
		if(moderatespeed[ST_FRIEND]){
			float allowedFriendDataRate = allowedDataRate * NeoPrefs.GetFriendBandwidthPercentage() / 100;
			CalcLoopAddOn(allowedFriendDataRate,toSendModerated[ST_FRIEND],timeSinceLastLoop);
		}
 #endif // BW_MOD // NEO: BM END

		//Slot Management this section regulates the slot-state (trickle, full)
		if(m_UploadQueue_list.GetCount() && recalculate && !cumulate && !minimize)
		{
			float realallowedDatarate = allowedDataRate;
 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
			realallowedDatarate -= theApp.uploadqueue->GetReservedDatarate()/1024;
 #endif // BW_MOD // NEO: BM END

			float savedbytes = 0;
			if(increasetricklespeed){
				//to be more accurate subtract the trickles
				UINT counttrickles = m_UploadQueue_list.GetCount() - m_UploadQueue_list_full.GetCount();
				float totalTrickleSpeed = counttrickles * wantedTrickleSpeed;
				if(totalTrickleSpeed < realallowedDatarate/2) // failsafe wen we reduce speed it may happen that we have to many trickles
					realallowedDatarate -= ( counttrickles * wantedTrickleSpeed);
				savedbytes = counttrickles > 1 ? wantedTrickleSpeed : 0;
			}

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
			if(includeoverhead)
				realallowedDatarate -= theApp.bandwidthControl->GetCurrUploadOverhead();
#endif // NEO_BC // NEO: NBC END <-- Xanatos --

			//calculate the wanted slots
			UINT slots=(UINT)(realallowedDatarate/wantedSlotSpeed);
 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
			slots += theApp.uploadqueue->GetReservedSlots();
 #endif // BW_MOD // NEO: BM END

			if(recalculate == 1){ // full recalculation
				if(slots > (UINT)m_UploadQueue_list_full.GetCount()){ //we don't have enough slots
					SetNextTrickleToFull(false, slots - m_UploadQueue_list_full.GetCount());
				}
				else if(slots < (UINT)m_UploadQueue_list_full.GetCount()){ //calculate the best amount of full slots
					if((realallowedDatarate-(float)slots*wantedSlotSpeed) > (((float)slots+1)*wantedSlotSpeed-realallowedDatarate) - savedbytes)
						slots++;

					SetNextTrickleToFull(false, slots, true);
				}
			}else if(recalculate == 2){ // partial recalculation (full slot droped, do wen need a new one)
				if((realallowedDatarate-(float)slots*wantedSlotSpeed) > (((float)slots+1)*wantedSlotSpeed-realallowedDatarate) - savedbytes)
					slots++;

				if(slots > (UINT)m_UploadQueue_list_full.GetCount())
					SetNextTrickleToFull(false);
			}

			recalculate = 0; // recalculate done
		}

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
		allowedLanDataRate = NeoPrefs.GetMaxLanUpload();

		bool bLanLanCast = NeoPrefs.IsLanSupportEnabled();
		if(bLanLanCast)
			CalcLoopAddOn(allowedLanDataRate,toLanSend,timeSinceLastLoop);
#endif //LANCAST // NEO: NLC END

		sendLocker.Lock();

		//When upload limit is too low don't allow overhead (control packets) eats the whole bandwidth
		bool prioritySend = false;

		//Serve blocked sockets
		POSITION blockedPos = m_BlockedQueue_list.GetHeadPosition();
		for(UINT i=m_BlockedQueue_list.GetCount();i>0;i--){
			POSITION removePos = blockedPos;
			ThrottledControlSocket* cur_socket = m_BlockedQueue_list.GetNext(blockedPos);
			if (!cur_socket->IsConnected())
				continue;

			// Note: we try to send only one packet to unblock the socket, the remainding packets will be send below
			SocketSentBytes Sent;
			if(cur_socket->onUpQueue){
				if (toSend > 0)
					Sent = cur_socket->Send(minFragSize,minFragSize,false,1);
				else
					continue; 
			}else{
				if(!includeoverhead || (cur_socket->IsPrioritySend() && !cur_socket->isUDP())) // if we dont include overhead or the socket is a priority tcp socket
					Sent = cur_socket->Send(0x7fffffff,minFragSize,true,1); 
				else if (toSend > 0 || cur_socket->IsPrioritySend()) // if we have bandwidth to use or the socjet is an priority udp UDP 
					Sent = cur_socket->Send((UINT)max(toSend,0),minFragSize,true,1);
				else
					continue; // we want break now, we will loop throu all queued sockets, there may be some priority sockets
			}

			toSend -= Sent.sentBytesStandardPackets;
			if(includeoverhead)
				toSend -= Sent.sentBytesControlPackets;

			//m_SentBytesSinceLastCall += Sent.sentBytesStandardPackets; // removed // NEO: ASM - [AccurateSpeedMeasure]

			if(cur_socket->IsBusy()){ // is sill busy
				if(cur_socket->onUpQueue)
					cur_socket->CountBusyTick();
				m_BlockedQueue_list.RemoveAt(removePos);
				m_BlockedQueue_list.AddTail(cur_socket); // move it to tail of the list and retry later
			}
			else{ // succesfuly unblocked
				if(cur_socket->onUpQueue)
					cur_socket->CountReadyTick();
				cur_socket->onBlockedQueue = false;
				m_BlockedQueue_list.RemoveAt(removePos);

				//if(!cur_socket->ControlPacketQueueIsEmpty() && !cur_socket->onUpQueue)
				if(!(cur_socket->StandardPacketQueueIsEmpty() && cur_socket->ControlPacketQueueIsEmpty()) && cur_socket->onUpQueue == false && cur_socket->onUpControlQueue == false){ // there are still packets to send, and socket is not queued as standard
					cur_socket->onUpControlQueue = true;
					m_ControlQueue_list.AddTail(cur_socket); // move it on the control queue
				}
			}
		}

		//Serve sockets that does not get anythink for a long time
		POSITION uploadPos = m_UploadQueue_list.GetHeadPosition();
		for(UINT i=m_UploadQueue_list.GetCount();i>0;i--){
			ThrottledFileSocket* cur_socket = m_UploadQueue_list.GetNext(uploadPos);
			uint32 neededBytes = cur_socket->GetNeededBytes();
			if(neededBytes > 0){
				if(toSend > 0){
					SocketSentBytes Sent = cur_socket->Send(neededBytes,minFragSize);
					
					toSend -= Sent.sentBytesStandardPackets;
					if(includeoverhead)
						toSend -= Sent.sentBytesControlPackets;

					//m_SentBytesSinceLastCall += Sent.sentBytesStandardPackets; // removed // NEO: ASM - [AccurateSpeedMeasure]

					// Evaluare the socket ratio
					if(Sent.sentStandardPackets > 0 || Sent.sentControlPackets > 0)
						cur_socket->CountReadyTick();
					else if(cur_socket->IsBusy()){
						// Note: We shell evaluate the particula socket fitnes, independant from toSend, ther for we evaluate only when a sent attempt was made
						cur_socket->CountBusyTick();
						if(hypersending && cur_socket->onBlockedQueue == false){ // this socket is busy and if we use hyper sending
							cur_socket->onBlockedQueue = true;
							m_BlockedQueue_list.AddTail(cur_socket); // queue the socket on the blocked queue for retry
						}
					}
				}
				else if(neededBytes > minFragSize * 5)
					prioritySend = true;
			}

			// Note: the busy evaluation could be also done below but for the blocked queue we make it here
			// Evaluare the socket ratio
			//if(cur_socket->IsBusy())
			//	cur_socket->CountBusyTick();
		}

		//Serve control packets
		POSITION controlPos = m_ControlQueue_list.GetHeadPosition();
		for(UINT i=m_ControlQueue_list.GetCount();i>0;i--){
			POSITION removePos = controlPos;
			ThrottledControlSocket* cur_socket = m_ControlQueue_list.GetNext(controlPos);
			if (prioritySend && includeoverhead && !cur_socket->IsPrioritySend()) // we are seems to be out of bandwidth, send serve only priority sockets
				continue;
	
			SocketSentBytes Sent;
			if(!includeoverhead || (cur_socket->IsPrioritySend() && !cur_socket->isUDP())) // if we dont include overhead or the socket is a priority tcp socket
				Sent = cur_socket->Send(0x7fffffff,minFragSize,true); // send all
			else if (toSend > 0 || cur_socket->IsPrioritySend()) // if we have bandwidth to use or the socjet is an priority udp UDP 
				Sent = cur_socket->Send((UINT)max(toSend,0),minFragSize,true); // send as much as we allow, UDP sockets will also send all priority packets
			else
				continue; // we want break now, we will lopp throu all queued sockets, there may be some priority sockets

			toSend -= Sent.sentBytesStandardPackets; // we may send a the rest of a partialy sendes stahdard packet to get it out of way
			if(includeoverhead)
				toSend -= Sent.sentBytesControlPackets;

			//m_SentBytesSinceLastCall += Sent.sentBytesStandardPackets; // removed // NEO: ASM - [AccurateSpeedMeasure]
			
			//if(cur_socket->ControlPacketQueueIsEmpty())
			if(cur_socket->ControlPacketQueueIsEmpty() && (cur_socket->StandardPacketQueueIsEmpty() || cur_socket->onUpQueue) ){ // this socket have nothing more to send
				cur_socket->onUpControlQueue = false;
				m_ControlQueue_list.RemoveAt(removePos);
			}
			else if(cur_socket->IsBusy()){ // this socket is busy
				cur_socket->onUpControlQueue = false;
				m_ControlQueue_list.RemoveAt(removePos);
				if(hypersending == TRUE && cur_socket->onBlockedQueue == false){ // if we use hyper sending
					cur_socket->onBlockedQueue = true;
					m_BlockedQueue_list.AddTail(cur_socket); // queue the socket on the blocked queue for retry
				} // in normal mode the socket will be requeued by OnSend
			}
			else if(cur_socket->isUDP()){ // we used up our bandwidth there are still packets left, when it is a UDP socket
				m_ControlQueue_list.RemoveAt(removePos); // make space for TCP sockets
				m_ControlQueue_list.AddTail(cur_socket); // and move the UDP spcket on the bottom of the list
			} 
			/*else {}*/ // we keep tcp sockets on top to complete incomplete packets
		}

		//now serve all trickles
		if(increasetricklespeed){
			POSITION tricklePos = m_UploadQueue_list.FindIndex(m_UploadQueue_list_full.GetCount());
			for(UINT i=m_UploadQueue_list.GetCount()-m_UploadQueue_list_full.GetCount();i>0;i--){
				POSITION removePos = tricklePos;
				ThrottledFileSocket* cur_socket = m_UploadQueue_list.GetNext(tricklePos);

				ASSERT(cur_socket->m_eSlotState == SS_TRICKLE || cur_socket->m_eSlotState == SS_BLOCKED);

				CalcLoopAddOn(wantedTrickleSpeed,cur_socket->m_SocketSlope,timeSinceLastLoop);

				if(cur_socket->m_SocketSlope <= 0 || toSend <= 0) // not enough to send
					continue;

				SocketSentBytes Sent = cur_socket->Send((UINT)min(toSend,cur_socket->m_SocketSlope),minFragSize);
				cur_socket->m_SocketSlope -= Sent.sentBytesStandardPackets;

				toSend -= Sent.sentBytesStandardPackets;
				if(includeoverhead)
					toSend -= Sent.sentBytesControlPackets;

				//m_SentBytesSinceLastCall += Sent.sentBytesStandardPackets; // removed // NEO: ASM - [AccurateSpeedMeasure]

				// Evaluare the socket ratio
				if(Sent.sentStandardPackets > 0 || Sent.sentControlPackets > 0)
					cur_socket->CountReadyTick();

				if(Sent.sentStandardPackets > 0){ // if we send
					m_UploadQueue_list.RemoveAt(removePos);
					m_UploadQueue_list.AddTail(cur_socket); // shift the socket to the tail of list
				}
				else // we shouldn't try unblock when we sent something?
				if(cur_socket->IsBusy() && hypersending == TRUE && cur_socket->onBlockedQueue == false){ // this socket is busy and if we use hyper sending
					cur_socket->onBlockedQueue = true;
					m_BlockedQueue_list.AddTail(cur_socket); // queue the socket on the blocked queue for retry
				}
			}
		}

		// sort sockets for cumulate bandwidth/slot focus
		// we will focus on the solts that can take the most
		// we sort only the full list for trickle sockets no cumulaion
		if(cumulate == TRUE && thisLoopTick - lastReOrder > SEC2MS(30)){
			lastReOrder = thisLoopTick;

			POSITION curPos = m_UploadQueue_list_full.GetHeadPosition();
			POSITION prevPos = NULL;
			ThrottledFileSocket* curSocket = NULL;
			ThrottledFileSocket* prevSocket = NULL;
			while (curPos){
				curSocket = m_UploadQueue_list_full.GetAt(curPos);
				if (prevSocket && prevSocket->GetAvgRatio() < curSocket->GetAvgRatio()){
					m_UploadQueue_list_full.RemoveAt(curPos);
					m_UploadQueue_list_full.InsertBefore(prevPos, curSocket);
					curPos = prevPos;
				}else if (curPos != prevPos){
					prevPos = curPos;
					prevSocket = curSocket;
				}
				m_UploadQueue_list_full.GetNext(curPos);
			}
		}

 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
		// calc slope for the slots that are moderated
		if(moderatespeed[ST_RELEASE] || moderatespeed[ST_FRIEND]){
			POSITION standardPos = m_UploadQueue_list_full.GetHeadPosition();
			for(UINT i=m_UploadQueue_list_full.GetCount();i>0;i--){
				ThrottledFileSocket* cur_socket = m_UploadQueue_list_full.GetNext(standardPos);
		
				ASSERT(cur_socket->m_eSlotType <= ST_NORMAL);
				
				if(moderatespeed[cur_socket->m_eSlotType])
					CalcLoopAddOn(cur_socket->m_ModerateSpeed,cur_socket->m_SocketSlope,timeSinceLastLoop);
			}
		}
 #endif // BW_MOD // NEO: BM END

		//Serve file packets
		int RedySlots;
		do{
			RedySlots = 0;
			POSITION standardPos = m_UploadQueue_list_full.GetHeadPosition();
			for(UINT i=m_UploadQueue_list_full.GetCount();i>0 && toSend > 0;i--){
				POSITION removePos = standardPos;
				ThrottledFileSocket* cur_socket = m_UploadQueue_list_full.GetNext(standardPos);

				ASSERT(cur_socket->m_eSlotState == SS_FULL);

 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
				ASSERT(cur_socket->m_eSlotType <= ST_NORMAL);
				
				if(moderatespeed[cur_socket->m_eSlotType]){
					if(cur_socket->m_SocketSlope <= 0 || toSendModerated[cur_socket->m_eSlotType] <= 0) // not enough to send
						continue;
				}
 #endif // BW_MOD // NEO: BM END

				SocketSentBytes Sent = cur_socket->Send(cumulate ? toSend : doubleSendSize,doubleSendSize);

				toSend -= Sent.sentBytesStandardPackets;
				if(includeoverhead)
					toSend -= Sent.sentBytesControlPackets;

				//m_SentBytesSinceLastCall += Sent.sentBytesStandardPackets; // removed // NEO: ASM - [AccurateSpeedMeasure]

 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
				if(moderatespeed[cur_socket->m_eSlotType]){
					cur_socket->m_SocketSlope -= Sent.sentBytesStandardPackets;
					toSendModerated[cur_socket->m_eSlotType] -= Sent.sentBytesStandardPackets;
					if(includeoverhead)
						toSendModerated[cur_socket->m_eSlotType] -= Sent.sentBytesControlPackets;
				}
 #endif // BW_MOD // NEO: BM END

				// Evaluare the socket ratio
				if(Sent.sentStandardPackets > 0 || Sent.sentControlPackets > 0)
					cur_socket->CountReadyTick();

				if(!cumulate && Sent.sentBytesStandardPackets >= minSendSize 
 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
				 // keep moderated slots on top thay have static alocated bandwidth, slot focus behavioure is ok here
				 && !moderatespeed[cur_socket->m_eSlotType]
 #endif // BW_MOD // NEO: BM END
				){ // If we sendt enough
					m_UploadQueue_list_full.RemoveAt(removePos);
					m_UploadQueue_list_full.AddTail(cur_socket); // shift the socket to the tail of list
				}
				else // we shouldn't try unblock when we sent already enough?
				if(cur_socket->IsBusy() && hypersending == TRUE && cur_socket->onBlockedQueue == false){ // this socket is busy and if we use hyper sending
					cur_socket->onBlockedQueue = true;
					m_BlockedQueue_list.AddTail(cur_socket); // queue the socket on the blocked queue for retry
				}

				if(!cur_socket->IsBusy() && Sent.success && !cur_socket->StandardPacketQueueIsEmpty()) // if the socket is not busy and have data
					RedySlots ++; // then it is redy to send again
			}
		}while(RedySlots > 0 && toSend > (int)doubleSendSize); // if we dont exost our bandwidth repeat the loop

		//downgrade the most blocking sockets to trickle
		if(NeoPrefs.IsTrickleBlocking() && thisLoopTick - lastCleanUp > SEC2MS(3) && m_UploadQueue_list_full.GetCount()){
			lastCleanUp = thisLoopTick;

			float avgratio = 0.0f;
			float blockratio = 0.5f;
			ThrottledFileSocket* blocksocket = NULL;
			POSITION testPos = m_UploadQueue_list_full.GetHeadPosition();
			for(UINT i=m_UploadQueue_list_full.GetCount();i>0;i--){
				ThrottledFileSocket* cur_socket = m_UploadQueue_list_full.GetNext(testPos);
				avgratio += cur_socket->GetAvgRatio();
				if(cur_socket->GetAvgRatio() < blockratio){
					blockratio = cur_socket->GetAvgRatio();
					blocksocket = cur_socket;
				}
			}
			avgratio /= m_UploadQueue_list_full.GetCount();

			//because there are some users out, which set a too high uploadlimit,
			//this code isn't useable we deactivate it
			if(blocksocket!=NULL && (blockratio/avgratio) < 0.25f && blockratio < 0.15f)
			{
				SetFullToTrickle(blocksocket, false, true);
				recalculate = 2;
			}
		}

		// Distribute remainding bandwidth to trickle slots
		POSITION tricklePos = m_UploadQueue_list.FindIndex(m_UploadQueue_list_full.GetCount());
		for(UINT i=m_UploadQueue_list.GetCount()-m_UploadQueue_list_full.GetCount();i>0 && toSend > (int)doubleSendSize;i--){
			POSITION removePos = tricklePos;
			ThrottledFileSocket* cur_socket = m_UploadQueue_list.GetNext(tricklePos);

			ASSERT(cur_socket->m_eSlotState == SS_TRICKLE || cur_socket->m_eSlotState == SS_BLOCKED);

			SocketSentBytes Sent = cur_socket->Send(min((UINT)toSend,minFragSize),minFragSize);
			toSend -= Sent.sentBytesStandardPackets;
			if(includeoverhead)
				toSend -= Sent.sentBytesControlPackets;

			//m_SentBytesSinceLastCall += Sent.sentBytesStandardPackets; // removed // NEO: ASM - [AccurateSpeedMeasure]

			// Evaluare the socket ratio
			if(Sent.sentStandardPackets > 0 || Sent.sentControlPackets > 0)
				cur_socket->CountReadyTick();

			if(Sent.sentStandardPackets > 0){ // if we sendt somethink
				m_UploadQueue_list.RemoveAt(removePos);
				m_UploadQueue_list.AddTail(cur_socket); // shift the socket to the tail of list
			}
			//else // wen don't try to unblock trickle sockets, when we go near a timeout we will try to send the needed bytes and when it also fails, than we wil ltry to unblock it
		}

		// Check do we used our whole bandwidth, do we need more slots
		if(toSend < (int)minSendSize){
			lastTickBandwidthReached=thisLoopTick;
		}
		else if(thisLoopTick - lastTickBandwidthReached  > SEC2MS(3) && (minimize || NeoPrefs.IsOpenMoreSlotsWhenNeeded())){ //since 3 seconds bandwidth couldn't be reached
			needslot = true;
			lastTickBandwidthReached=thisLoopTick;
		}

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
		// Sreve Lan queue
		if(bLanLanCast){
			POSITION lanPos = m_LanQueue_list.GetHeadPosition();
			for(UINT i=m_LanQueue_list.GetCount();i>0;i--){
				POSITION removePos = lanPos;
				ThrottledFileSocket* cur_socket = m_LanQueue_list.GetNext(lanPos);
				if(!cur_socket->IsConnected())
					continue;

				uint32 SentLoop = 0;
 #ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
				// No limit for voodoo sockets
				while((cur_socket->IsVoodooSocket() || toLanSend > 0) && cur_socket->HasQueues())
 #else
				while(toLanSend > 0 && cur_socket->HasQueues())
 #endif // VOODOO // NEO: VOODOO END
				{
					SocketSentBytes Sent = cur_socket->Send(theApp.bandwidthControl->GetMSS()*4,theApp.bandwidthControl->GetMSS()*4);

					SentLoop += Sent.sentBytesStandardPackets;
					toLanSend -= Sent.sentBytesStandardPackets + Sent.sentBytesControlPackets;

					if(Sent.success == false || cur_socket->IsBusy())
						break;
				}

 #ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
				// Keep voodoo sockets tham on top
				if(!cur_socket->IsVoodooSocket() && SentLoop >= (uint32)theApp.bandwidthControl->GetMSS()/2){ // we sendt enough shift socket to tail
 #else
				if(SentLoop >= (uint32)theApp.bandwidthControl->GetMSS()/2){ // we sendt enough shift socket to tail
 #endif // VOODOO // NEO: VOODOO END
					m_LanQueue_list.RemoveAt(removePos);
					m_LanQueue_list.AddTail(cur_socket);
				}
			}
		}
#endif //LANCAST // NEO: NLC END
		sendLocker.Unlock();

		//dataLocker.Lock();
		//dataLocker.Unlock();
	}

	threadEndedEvent->SetEvent();

	sendLocker.Lock();
	m_ControlQueue_list.RemoveAll();
	m_BlockedQueue_list.RemoveAll();

	m_UploadQueue_list.RemoveAll();
	m_UploadQueue_list_full.RemoveAll();

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	m_LanQueue_list.RemoveAll();
#endif //LANCAST // NEO: NLC END
	sendLocker.Unlock();

	return 0;
}

void UploadBandwidthThrottler::ReSortUploadSlots(CUpDownClient* client){
	if (client){
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
		if(client->IsLanClient())
			return;
#endif //LANCAST // NEO: NLC END

		sendLocker.Lock();
		// Remove the found Client from UploadBandwidthThrottler
		RemoveFromStandardListNoLock(client->socket);
		RemoveFromStandardListNoLock((CClientReqSocket*) client->m_pPCUpSocket);
		CEMSocket* socket = client->GetFileUploadSocket();
 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
		if(socket) {
			if(NeoPrefs.IsSeparateReleaseBandwidth() && client->GetReleaseSlot()) // NEO: SRS - [SmartReleaseSharing]
				socket->m_eSlotType = ST_RELEASE; 
			else if(NeoPrefs.IsSeparateFriendBandwidth() && client->GetFriendSlot()) // NEO: NMFS - [NiceMultiFriendSlots]
				socket->m_eSlotType = ST_FRIEND; 
			else
				socket->m_eSlotType = ST_NORMAL; 
		}
 #endif // BW_MOD // NEO: BM END
		AddToStandardListNoLock(socket);
		sendLocker.Unlock();
	}
}

// removed // NEO: ASM - [AccurateSpeedMeasure]
/*uint64 UploadBandwidthThrottler::GetSentBytes() {
	sendLocker.Lock();

	uint64 numberOfSentBytesSinceLastCall = m_SentBytesSinceLastCall;
	m_SentBytesSinceLastCall = 0;

	sendLocker.Unlock();

	return numberOfSentBytesSinceLastCall;
}*/

#endif // NEO_BC // NEO: NBC END <-- Xanatos --