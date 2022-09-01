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
#include <Mmsystem.h>
#include "emule.h"
#include "UploadBandwidthThrottler.h"
#include "EMSocket.h"
#include "opcodes.h"
#include "LastCommonRouteFinder.h"
#include "OtherFunctions.h"
#include "emuledlg.h"
#include "uploadqueue.h"
#include "preferences.h"
#include "Statistics.h" // add download ack


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

/**
 * The constructor starts the thread.
 */
UploadBandwidthThrottler::UploadBandwidthThrottler(void) {
    //MORPH - Changed by SiRoB, Upload Splitting Class
	/*
	m_SentBytesSinceLastCall = 0;
	m_SentBytesSinceLastCallOverhead = 0;
    m_highestNumberOfFullyActivatedSlots = 0;
	*/
	memset(m_SentBytesSinceLastCallClass,0,sizeof(m_SentBytesSinceLastCallClass));
	memset(m_SentBytesSinceLastCallOverheadClass,0,sizeof(m_SentBytesSinceLastCallOverheadClass));
	memset(m_highestNumberOfFullyActivatedSlotsClass,0,sizeof(m_highestNumberOfFullyActivatedSlotsClass));
	memset(slotCounterClass,0,sizeof(slotCounterClass));
	m_nUpDataOverheadFromDownload=0;
		
	threadEndedEvent = new CEvent(0, 1);
    pauseEvent = new CEvent(TRUE, TRUE);

    doRun = true;
    AfxBeginThread(RunProc, (LPVOID)this);
}

/**
 * The destructor stops the thread. If the thread has already stoppped, destructor does nothing.
 */
UploadBandwidthThrottler::~UploadBandwidthThrottler(void) {
    EndThread();
    delete threadEndedEvent;
	delete pauseEvent;
}

void UploadBandwidthThrottler::GetStats(uint64* SentBytes, uint64* SentBytesOverhead, uint32* HighestNumberOfFullyActivatedSlotsSinceLastCallClass) {
    sendBytesLocker.Lock();

	for (uint8 i = 0; i < NB_SPLITTING_CLASS; i++) {
		*(SentBytes++) = m_SentBytesSinceLastCallClass[i];
		m_SentBytesSinceLastCallClass[i] = 0;
		*(SentBytesOverhead++) = m_SentBytesSinceLastCallOverheadClass[i];
		m_SentBytesSinceLastCallOverheadClass[i] = 0;
		*(HighestNumberOfFullyActivatedSlotsSinceLastCallClass++) = m_highestNumberOfFullyActivatedSlotsClass[i];
		//m_highestNumberOfFullyActivatedSlotsClass[i] = (UINT)0;
	}

	sendBytesLocker.Unlock();
}

/**
 * Find out how many bytes that has been put on the sockets since the last call to this
 * method. Excludes overhead of control packets.
 *
 * @return the number of bytes that has been put on the sockets since the last call
 */
//MORPH - Removed by SiRoB, See above
/*
uint64 UploadBandwidthThrottler::GetNumberOfSentBytesOverheadSinceLastCallAndReset() {
    sendBytesLocker.Lock();
    
	uint64 numberOfSentBytesSinceLastCall = m_SentBytesSinceLastCallOverhead;
	m_SentBytesSinceLastCallOverhead = 0;

    sendBytesLocker.Unlock();

    return numberOfSentBytesSinceLastCall;
}
*/

/**
 * Find out the highest number of slots that has been fed data in the normal standard loop
 * of the thread since the last call of this method. This means all slots that haven't
 * been in the trickle state during the entire time since the last call.
 *
 * @return the highest number of fully activated slots during any loop since last call
 */
/*void UploadBandwidthThrottler::GetHighestNumberOfFullyActivatedSlotsSinceLastCallAndReset(uint32 * HighestNumberOfFullyActivatedSlotsSinceLastCallClass) {
    sendLocker.Lock();
    
    //if(m_highestNumberOfFullyActivatedSlots > (uint32)m_StandardOrder_list.GetSize()) {
    //    theApp.QueueDebugLogLine(true, _T("UploadBandwidthThrottler: Throttler wants new slot when get-method called. m_highestNumberOfFullyActivatedSlots: %i m_StandardOrder_list.GetSize(): %i tick: %i"), m_highestNumberOfFullyActivatedSlots, m_StandardOrder_list.GetSize(), ::GetTickCount());
    //}
GetStats(
    	for (uint8 i = 0; i < NB_SPLITTING_CLASS; i++) {
		*(HighestNumberOfFullyActivatedSlotsSinceLastCallClass++) = m_highestNumberOfFullyActivatedSlotsClass[i];
		m_highestNumberOfFullyActivatedSlotsClass[i] = (UINT)0;
	}
	sendLocker.Unlock();
}
*/
/**
 * Add a socket to the list of sockets that have upload slots. The main thread will
 * continously call send on these sockets, to give them chance to work off their queues.
 * The sockets are called in the order they exist in the list, so the top socket (index 0)
 * will be given a chance first to use bandwidth, and then the next socket (index 1) etc.
 *
 * It is possible to add a socket several times to the list without removing it inbetween,
 * but that should be avoided.
 *
 * @param index insert the socket at this place in the list. An index that is higher than the
 *              current number of sockets in the list will mean that the socket should be inserted
 *              last in the list.
 *
 * @param socket the address to the socket that should be added to the list. If the address is NULL,
 *               this method will do nothing.
 */
void UploadBandwidthThrottler::AddToStandardList(uint32 index, ThrottledFileSocket* socket, uint32 classID, bool scheduled) {
    if(socket != NULL) {
        sendLocker.Lock();

		RemoveFromStandardListNoLock(socket);
		if(index > (uint32)m_StandardOrder_list.GetSize()) {
			index = m_StandardOrder_list.GetSize();
		}
		m_StandardOrder_list.InsertAt(index, socket);
		//MORPH START - Added by SiRoB & AndCycle, Upload Splitting Class
		Socket_stat* cur_socket_stat = NULL;
		if (!m_stat_list.Lookup(socket,cur_socket_stat)){
			cur_socket_stat = new Socket_stat;
			m_stat_list.SetAt(socket,cur_socket_stat);
			cur_socket_stat->realBytesToSpend = _I64_MAX;
			cur_socket_stat->dwLastBusySince = 0;
			cur_socket_stat->dwLastBytesSent = timeGetTime();
		}
		cur_socket_stat->scheduled = scheduled;
		cur_socket_stat->classID = classID;
		++slotCounterClass[classID];
		//MORPH END - Added by SiRoB & AndCycle, Upload Splitting Class

        sendLocker.Unlock();
//	} else {
//		if (thePrefs.GetVerbose())
//		DebugLogError(LOG_STATUSBAR,_T("Tried to add NULL socket to UploadBandwidthThrottler Standard list! Prevented."));
    }
}

/**
 * Remove a socket from the list of sockets that have upload slots.
 *
 * If the socket has mistakenly been added several times to the list, this method
 * will return all of the entries for the socket.
 *
 * @param socket the address of the socket that should be removed from the list. If this socket
 *               does not exist in the list, this method will do nothing.
 */
bool UploadBandwidthThrottler::RemoveFromStandardList(ThrottledFileSocket* socket, bool resort) {
	bool returnValue;
    sendLocker.Lock();

	returnValue = RemoveFromStandardListNoLock(socket, resort);
	if (returnValue && !resort)
		QueueForSendingControlPacket(socket);
	
    sendLocker.Unlock();

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
bool UploadBandwidthThrottler::RemoveFromStandardListNoLock(ThrottledFileSocket* socket, bool resort) {
	// Find the slot
    int slotCounter = 0;
    bool foundSocket = false;
	uint32 classID = NULL;
	while(slotCounter < m_StandardOrder_list.GetSize() && foundSocket == false) {
        if(m_StandardOrder_list.GetAt(slotCounter) == socket) {
			// Remove the slot
            m_StandardOrder_list.RemoveAt(slotCounter);
			// Remove the slot
			Socket_stat* stat = NULL;
			m_stat_list.Lookup(socket,stat);
			classID = stat->classID;
			--slotCounterClass[classID];
			foundSocket = true;
        } else {
            slotCounter++;
        }
    }
	//MORPH START - Added by SiRoB, Upload Splitting Class
	if(foundSocket && !resort) {
		sendBytesLocker.Lock();
		if (m_highestNumberOfFullyActivatedSlotsClass[classID] > slotCounterClass[classID])
			m_highestNumberOfFullyActivatedSlotsClass[classID] = slotCounterClass[classID];
		sendBytesLocker.Unlock();
	}
	//MORPH END  - Added by SiRoB, Upload Splitting Class

	return foundSocket;
}

void UploadBandwidthThrottler::SignalNoLongerBusy() {
    busyEvent.SetEvent();
}

/**
* Notifies the send thread that it should try to call controlpacket send
* for the supplied socket. It is allowed to call this method several times
* for the same socket, without having controlpacket send called for the socket
* first. The doublette entries are never filtered, since it is incurs less cpu
* overhead to simply call Send() in the socket for each double. Send() will
* already have done its work when the second Send() is called, and will just
* return with little cpu overhead.
*
* @param socket address to the socket that requests to have controlpacket send
*               to be called on it
*/
void UploadBandwidthThrottler::QueueForSendingControlPacket(ThrottledControlSocket* socket, bool hasSent) {
	// Get critical section
    tempQueueLocker.Lock();

    if(doRun) {
        if(hasSent) {
            m_TempControlQueueFirst_list.AddTail(socket);
        } else {
        	m_TempControlQueue_list.AddTail(socket);
    	}
    }

	// End critical section
    tempQueueLocker.Unlock();
}

/**
 * Remove the socket from all lists and queues. This will make it safe to
 * erase/delete the socket. It will also cause the main thread to stop calling
 * send() for the socket.
 *
 * @param socket address to the socket that should be removed
 */
void UploadBandwidthThrottler::RemoveFromAllQueues(ThrottledControlSocket* socket, bool lock) {
	if(doRun) {
        // Remove this socket from control packet queue
		tempQueueLocker.Lock();
        {
            POSITION pos1, pos2;
	        for (pos1 = m_TempControlQueue_list.GetHeadPosition();( pos2 = pos1 ) != NULL;) {
		        m_TempControlQueue_list.GetNext(pos1);
		        ThrottledControlSocket* socketinQueue = m_TempControlQueue_list.GetAt(pos2);

                if(socketinQueue == socket) {
                    m_TempControlQueue_list.RemoveAt(pos2);
                }
            }
        }

        {
            POSITION pos1, pos2;
	        for (pos1 = m_TempControlQueueFirst_list.GetHeadPosition();( pos2 = pos1 ) != NULL;) {
		        m_TempControlQueueFirst_list.GetNext(pos1);
		        ThrottledControlSocket* socketinQueue = m_TempControlQueueFirst_list.GetAt(pos2);

                if(socketinQueue == socket) {
                    m_TempControlQueueFirst_list.RemoveAt(pos2);
                }
            }
        }
		tempQueueLocker.Unlock();

	    if(lock) {
		    // Get critical section
		    sendLocker.Lock();
        }

        {
            POSITION pos1, pos2;
	        for (pos1 = m_ControlQueue_list.GetHeadPosition();( pos2 = pos1 ) != NULL;) {
		        m_ControlQueue_list.GetNext(pos1);
		        ThrottledControlSocket* socketinQueue = m_ControlQueue_list.GetAt(pos2);

                if(socketinQueue == socket) {
                    m_ControlQueue_list.RemoveAt(pos2);
                }
            }
        }
        
        {
            POSITION pos1, pos2;
	        for (pos1 = m_ControlQueueFirst_list.GetHeadPosition();( pos2 = pos1 ) != NULL;) {
		        m_ControlQueueFirst_list.GetNext(pos1);
		        ThrottledControlSocket* socketinQueue = m_ControlQueueFirst_list.GetAt(pos2);

                if(socketinQueue == socket) {
                    m_ControlQueueFirst_list.RemoveAt(pos2);
                }
            }
        }

        if(lock) {
		    // End critical section
		    sendLocker.Unlock();
        }
    }
}

void UploadBandwidthThrottler::RemoveFromAllQueues(ThrottledFileSocket* socket) {
    if(doRun) {
		// Get critical section
    	sendLocker.Lock();
        RemoveFromAllQueues(socket, false);

		// And remove it from upload slots
        RemoveFromStandardListNoLock(socket);

		//MORPH START - Added by SiRoB, Upload Splitting Class
		Socket_stat* stat = NULL;
		if (m_stat_list.Lookup(socket,stat)) {
			delete stat;
			m_stat_list.RemoveKey(socket);
		}
		//MORPH END   - Added by SiRoB, Upload Splitting Class

		// End critical section
	    sendLocker.Unlock();
    }
}

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

void UploadBandwidthThrottler::Pause(bool paused) {
    if(paused) {
        pauseEvent->ResetEvent();
    } else {
        pauseEvent->SetEvent();
    }
}

// morph start
void UploadBandwidthThrottler::SetDownDataOverheadOtherPackets(long bytes)	{
	sendLocker.Lock();
	m_nUpDataOverheadFromDownload+=bytes;
	if(thePrefs.m_bUseDownloadOverhead)
		theStats.AddUpDataOverheadOther((uint32)bytes); // add to overhead calculations
	sendLocker.Unlock();
}


uint64	UploadBandwidthThrottler::GetDownDataOverheadOtherPackets_andreset()	{
	sendLocker.Lock();
	uint64	 ret=m_nUpDataOverheadFromDownload;
    m_nUpDataOverheadFromDownload=0;
	sendLocker.Unlock();

	return ret;
}


uint32 UploadBandwidthThrottler::GetSlotLimit(uint32 currentUpSpeed) {
    uint32 upPerClient = UPLOAD_CLIENT_DATARATE;

    // if throttler doesn't require another slot, go with a slightly more restrictive method
	if( currentUpSpeed > 20*1024 )
		upPerClient += currentUpSpeed/43;

	if( upPerClient > 7680 )
		upPerClient = 7680;

	//now the final check

	uint16 nMaxSlots;
	if (currentUpSpeed > 12*1024)
		nMaxSlots = (uint16)(((float)currentUpSpeed) / upPerClient);
	else if (currentUpSpeed > 7*1024)
		nMaxSlots = MIN_UP_CLIENTS_ALLOWED + 2;
	else if (currentUpSpeed > 3*1024)
		nMaxSlots = MIN_UP_CLIENTS_ALLOWED + 1;
	else
		nMaxSlots = MIN_UP_CLIENTS_ALLOWED;

    return max(nMaxSlots, MIN_UP_CLIENTS_ALLOWED);
}

uint32 UploadBandwidthThrottler::CalculateChangeDelta(uint32 numberOfConsecutiveChanges) const {
    switch(numberOfConsecutiveChanges) {
        case 0: return 50;
        case 1: return 50;
        case 2: return 128;
        case 3: return 256;
        case 4: return 512;
        case 5: return 512+256;
        case 6: return 1*1024;
        case 7: return 1*1024+256;
        default: return 1*1024+512;
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

	// SLUGFILLER: SafeHash
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return 0;
	// SLUGFILLER: SafeHash

	InitThreadLocale();

    UploadBandwidthThrottler* uploadBandwidthThrottler = (UploadBandwidthThrottler*)pParam;

    return uploadBandwidthThrottler->RunInternal();
}

/**
 * The thread method that handles calling send for the individual sockets.
 *
 * Control packets will always be tried to be sent first. If there is any bandwidth leftover
 * after that, send() for the upload slot sockets will be called in priority order until we have run
 * out of available bandwidth for this loop. Upload slots will not be allowed to go without having sent
 * called for more than a defined amount of time (i.e. two seconds).
 *
 * @return always returns 0.
 */
UINT UploadBandwidthThrottler::RunInternal() {
	DWORD lastLoopTick = timeGetTime();
	//MORPH START - Added by SiRoB, Upload Splitting Class
	DWORD lastLoopTickTryTosend = lastLoopTick;
	uint32 allowedDataRateClass[NB_SPLITTING_CLASS];
	uint32 ClientDataRate[NB_SPLITTING_CLASS];
	uint32 rememberSlotCounter[NB_SPLITTING_CLASS];
	sint64 realBytesToSpendClass[NB_SPLITTING_CLASS];
	memset(realBytesToSpendClass,0,sizeof(realBytesToSpendClass));
	DWORD dwLastFullBandwidthSpent[NB_SPLITTING_CLASS];
	memset(dwLastFullBandwidthSpent,0,sizeof(dwLastFullBandwidthSpent));
	//MORPH END   - Added by SiRoB, Upload Splitting Class
	uint32 nEstiminatedLimit = 0;
	int nSlotsBusyLevel = 0;
	DWORD nUploadStartTime = 0;
    uint32 numberOfConsecutiveUpChanges = 0;
    uint32 numberOfConsecutiveDownChanges = 0;
    uint32 changesCount = 0;
    uint32 loopsCount = 0;

    //bool estimateChangedLog = false;
	bool estimateChangedLog = true;
    bool lotsOfLog = false; 

    while(doRun) {
        pauseEvent->Lock();

		theApp.lastCommonRouteFinder->GetClassByteToSend(allowedDataRateClass,ClientDataRate);

		// check busy level for all the slots (WSAEWOULDBLOCK status)
		uint32 cBusy = 0;
		uint32 nCanSend = 0;
		DWORD  cBusyTime = 0;
		DWORD timeSinceLastLoop = timeGetTime() - lastLoopTick;
		sendLocker.Lock();
		/*
		for (int i = 0; i < m_StandardOrder_list.GetSize() && (i < 3 || (UINT)i < GetSlotLimit(theApp.uploadqueue->GetDatarate())); i++){
		*/
		for (int i = 0; i < m_StandardOrder_list.GetSize(); i++){
			Socket_stat* stat = NULL;
			if (m_stat_list.Lookup(m_StandardOrder_list[i], stat)) {
				if (m_StandardOrder_list[i] != NULL && m_StandardOrder_list[i]->HasQueues() && stat->scheduled == false && m_StandardOrder_list[i]->GetBusyRatioTime()) {
					nCanSend++;
				
					DWORD newBusySince = m_StandardOrder_list[i]->GetBusyTimeSince();
					if(newBusySince > 0) {
						cBusy++;
						if (newBusySince == stat->dwLastBusySince)
							cBusyTime += timeSinceLastLoop;
						else
							cBusyTime += timeGetTime() - newBusySince;
						stat->dwLastBusySince = newBusySince;
					}
				}
			}
		}
		sendLocker.Unlock();

/*
		// if this is kept, the loop above can be a little optimized (don't count nCanSend, just use nCanSend = GetSlotLimit(theApp.uploadqueue->GetDatarate())
		if(theApp.uploadqueue) {
			nCanSend = min(nCanSend, GetSlotLimit(theApp.uploadqueue->GetDatarate()));
			//nCanSend = min(max(GetSlotLimit(theApp.uploadqueue->GetDatarate()),1), (UINT)m_StandardOrder_list.GetSize());
		}
*/
		bool bUploadUnlimited = (thePrefs.GetMaxUpload() == UNLIMITED);
		// When no upload limit has been set in options, try to guess a good upload limit.
		if (bUploadUnlimited) {
			if (timeSinceLastLoop > 0) {
				loopsCount++;

				//if(lotsOfLog) theApp.QueueDebugLogLine(false,_T("Throttler: busy: %i/%i nSlotsBusyLevel: %i Guessed limit: %0.5f changesCount: %i loopsCount: %i"), cBusy, nCanSend, nSlotsBusyLevel, (float)nEstiminatedLimit/1024.00f, changesCount, loopsCount);

				if(nCanSend > 0) {
					float fBusyPercent = ((float)cBusyTime/(float)nCanSend/(float)timeSinceLastLoop) * 100;
					if (cBusy > 2 && fBusyPercent > 75.00f && nSlotsBusyLevel < 255){
						nSlotsBusyLevel+=cBusyTime;
						changesCount+=cBusyTime;
						if(thePrefs.GetVerbose() && lotsOfLog && nSlotsBusyLevel%25==0) theApp.QueueDebugLogLine(false,_T("Throttler: nSlotsBusyLevel: %i Guessed limit: %0.5f changesCount: %i loopsCount: %i"), nSlotsBusyLevel, (float)nEstiminatedLimit/1024.00f, changesCount, loopsCount);
					}
					else if ((cBusy <= 2 || fBusyPercent < 25.00f) && nSlotsBusyLevel > (-255)){
						nSlotsBusyLevel-=timeSinceLastLoop;
						changesCount+=timeSinceLastLoop;
						if(thePrefs.GetVerbose() && lotsOfLog && nSlotsBusyLevel%25==0) theApp.QueueDebugLogLine(false,_T("Throttler: nSlotsBusyLevel: %i Guessed limit: %0.5f changesCount %i loopsCount: %i"), nSlotsBusyLevel, (float)nEstiminatedLimit/1024.00f, changesCount, loopsCount);
					}
				}

				if(nUploadStartTime == 0) {
					if (m_StandardOrder_list.GetSize() >= 3)
						nUploadStartTime = timeGetTime();
				} else if(timeGetTime()- nUploadStartTime > SEC2MS(60)) {
					if (theApp.uploadqueue){
						if (nEstiminatedLimit == 0){ // no autolimit was set yet
							if (nSlotsBusyLevel >= 250){ // sockets indicated that the BW limit has been reached
								nEstiminatedLimit = theApp.uploadqueue->GetDatarate();
								allowedDataRateClass[LAST_CLASS] = min(nEstiminatedLimit, allowedDataRateClass[LAST_CLASS]);
						    nSlotsBusyLevel = -200;
								if(thePrefs.GetVerbose() && estimateChangedLog) theApp.QueueDebugLogLine(false,_T("Throttler: Set inital estimated limit to %0.5f changesCount: %i loopsCount: %i"), (float)nEstiminatedLimit/1024.00f, changesCount, loopsCount);
								changesCount = 0;
								loopsCount = 0;
							}
						}
						else{
							if (nSlotsBusyLevel > 250){
								if(changesCount > 500 || changesCount > 300 && loopsCount > 1000 || loopsCount > 2000) {
									numberOfConsecutiveDownChanges = 0;
								}
								numberOfConsecutiveDownChanges+=timeSinceLastLoop;
								uint32 changeDelta = CalculateChangeDelta(numberOfConsecutiveDownChanges);

								// Don't lower speed below 1 KBytes/s
								// leuk_he don't move below minupload
								if(nEstiminatedLimit < changeDelta + 1024*thePrefs.minupload) {
									if(nEstiminatedLimit > ((uint32)1024)*thePrefs.minupload) {
										changeDelta = nEstiminatedLimit - 1024*thePrefs.minupload;
									} else {
										changeDelta = 0;
									}
								}
								ASSERT(nEstiminatedLimit >= changeDelta + 1024*thePrefs.minupload);
    							nEstiminatedLimit -= changeDelta;

								if(thePrefs.GetVerbose() && estimateChangedLog) theApp.QueueDebugLogLine(false,_T("Throttler: REDUCED limit #%i with %i bytes to: %0.5f changesCount: %i loopsCount: %i"), numberOfConsecutiveDownChanges, changeDelta, (float)nEstiminatedLimit/1024.00f, changesCount, loopsCount);

								numberOfConsecutiveUpChanges = 0;
								nSlotsBusyLevel = 0;
								changesCount = 0;
								loopsCount = 0;
							}
							else if (nSlotsBusyLevel < (-250)){
								if(changesCount > 500 || changesCount > 300 && loopsCount > 1000 || loopsCount > 2000) {
									numberOfConsecutiveUpChanges = 0;
								}
								numberOfConsecutiveUpChanges+=timeSinceLastLoop;
								uint32 changeDelta = CalculateChangeDelta(numberOfConsecutiveUpChanges);

								// Don't raise speed unless we are under current allowedDataRate
								if(nEstiminatedLimit+changeDelta > allowedDataRateClass[LAST_CLASS]) {
									if(nEstiminatedLimit < allowedDataRateClass[LAST_CLASS]) {
										changeDelta = allowedDataRateClass[LAST_CLASS] - nEstiminatedLimit;
									} else {
										changeDelta = 0;
									}
								}
								ASSERT(nEstiminatedLimit < allowedDataRateClass[LAST_CLASS] && nEstiminatedLimit+changeDelta <= allowedDataRateClass[LAST_CLASS] || nEstiminatedLimit >= allowedDataRateClass[LAST_CLASS] && changeDelta == 0);
								nEstiminatedLimit += changeDelta;

								if(thePrefs.GetVerbose() && estimateChangedLog) theApp.QueueDebugLogLine(false,_T("Throttler: INCREASED limit #%i with %i bytes to: %0.5f changesCount: %i loopsCount: %i"), numberOfConsecutiveUpChanges, changeDelta, (float)nEstiminatedLimit/1024.00f, changesCount, loopsCount);

								numberOfConsecutiveDownChanges = 0;
								nSlotsBusyLevel = 0;
								changesCount = 0;
								loopsCount = 0;
							}

							allowedDataRateClass[LAST_CLASS] = min(nEstiminatedLimit, allowedDataRateClass[LAST_CLASS]);
						} 
					}
				}
			} else if (nEstiminatedLimit) {
				allowedDataRateClass[LAST_CLASS] = min(nEstiminatedLimit, allowedDataRateClass[LAST_CLASS]);
			}
		} else {
			nEstiminatedLimit = 0;
		}
		uint32 minFragSize = 1300;
		uint32 doubleSendSize = minFragSize*2; // send two packages at a time so they can share an ACK
		if(allowedDataRateClass[LAST_CLASS] < 6*1024) {
			minFragSize = 536;
			doubleSendSize = minFragSize; // don't send two packages at a time at very low speeds to give them a smoother load
		}

       	if(cBusy >= nCanSend && realBytesToSpendClass[LAST_CLASS] > 999 /*&& m_StandardOrder_list.GetSize() > 0*/) {
			//allowedDataRate = 0;
            //realBytesToSpend = min(realBytesToSpend, 1024*1000);
            if(nSlotsBusyLevel < 125 && bUploadUnlimited) {
				nSlotsBusyLevel = 125;
				if(thePrefs.GetVerbose() && lotsOfLog) theApp.QueueDebugLogLine(false,_T("Throttler: nSlotsBusyLevel: %i Guessed limit: %0.5f changesCount %i loopsCount: %i (set due to all slots busy)"), nSlotsBusyLevel, (float)nEstiminatedLimit/1024.00f, changesCount, loopsCount);
			}

			busyEvent.Lock(1);
		}
		
		#define TIME_BETWEEN_UPLOAD_LOOPS 0
        /*
		uint32 TIME_BETWEEN_UPLOAD_LOOPS = 0;
        if(cBusy >= nCanSend) {
            TIME_BETWEEN_UPLOAD_LOOPS = 1;
        }
		*/

        uint32 sleepTime;
        if(allowedDataRateClass[LAST_CLASS] == _UI32_MAX || realBytesToSpendClass[LAST_CLASS] >= 1000 || allowedDataRateClass[LAST_CLASS] == 0 && nEstiminatedLimit == 0) {
            // we can send at once
            sleepTime = 1;
        } else if(allowedDataRateClass[LAST_CLASS] == 0) {
            sleepTime = max((uint32)ceil(((double)doubleSendSize*1000)/nEstiminatedLimit), TIME_BETWEEN_UPLOAD_LOOPS);
        } else {
            // sleep for just as long as we need to get back to having one byte to send
            sleepTime = max((uint32)ceil((double)(-realBytesToSpendClass[LAST_CLASS] + 1000)/allowedDataRateClass[LAST_CLASS]), TIME_BETWEEN_UPLOAD_LOOPS);
        }

		timeSinceLastLoop = timeGetTime() - lastLoopTick;
		
        if(timeSinceLastLoop < sleepTime) {
            //DWORD tickBeforeSleep = ::GetTickCount();
            //DWORD tickBeforeSleep2 = timeGetTime();
            Sleep(sleepTime-timeSinceLastLoop);
            //DWORD tickAfterSleep = ::GetTickCount();
            //DWORD tickAfterSleep2 = timeGetTime();

            //theApp.QueueDebugLogLine(false,_T("UploadBandwidthThrottler: Requested sleep time = %i ms. Actual sleep time %i/%i ms."), sleepTime-timeSinceLastLoop, tickAfterSleep-tickBeforeSleep, tickAfterSleep2-tickBeforeSleep2);
        }

		const DWORD thisLoopTick = timeGetTime();
        timeSinceLastLoop = min(1000, thisLoopTick - lastLoopTick);

		//Update realBytesToSpend (the unit is milliBytes mean 1Byte correspond to 1000) per class 
		for (uint32 classID = 0; classID < NB_SPLITTING_CLASS; classID++) {
			uint32 allowedDataRate = allowedDataRateClass[classID];
			if(allowedDataRate > 0 && allowedDataRate != _UI32_MAX) {
				if (timeSinceLastLoop > 0) {
					sint64 limit = -((sint64)2000*allowedDataRate);
					if (realBytesToSpendClass[classID] < limit)
						realBytesToSpendClass[classID] = limit;
					else if (realBytesToSpendClass[classID] > 999)
						realBytesToSpendClass[classID] = 999;
					if (_I64_MAX/timeSinceLastLoop > allowedDataRate && _I64_MAX-allowedDataRate*timeSinceLastLoop > realBytesToSpendClass[classID]) {
						realBytesToSpendClass[classID] += allowedDataRate*timeSinceLastLoop;
					} else {
						realBytesToSpendClass[classID] = _I64_MAX;
					}
				}
			} else {
				realBytesToSpendClass[classID] = _I64_MAX;
			}
		}
			
		lastLoopTick = thisLoopTick;

		sint64 BytesToSpend = realBytesToSpendClass[LAST_CLASS] / 1000;
		if(BytesToSpend >= 1 || allowedDataRateClass[LAST_CLASS] == 0) {
			timeSinceLastLoop = min(1000,thisLoopTick - lastLoopTickTryTosend);

			sendLocker.Lock();
        	tempQueueLocker.Lock();
			
			// are there any sockets in m_TempControlQueue_list? Move them to normal m_ControlQueue_list;
			while(!m_TempControlQueueFirst_list.IsEmpty()) {
				ThrottledControlSocket* moveSocket = m_TempControlQueueFirst_list.RemoveHead();
				m_ControlQueueFirst_list.AddTail(moveSocket);
			}
			while(!m_TempControlQueue_list.IsEmpty()) {
				ThrottledControlSocket* moveSocket = m_TempControlQueue_list.RemoveHead();
       			m_ControlQueue_list.AddTail(moveSocket);
			}
   			tempQueueLocker.Unlock();

	    
			uint64 ControlspentBytes = 0;
			uint64 ControlspentOverhead = 0;

			// Send any queued up control packets first
			while((BytesToSpend > 0 && ControlspentBytes < (uint64)BytesToSpend|| allowedDataRateClass[LAST_CLASS] == 0 && ControlspentBytes < 500) && (!m_ControlQueueFirst_list.IsEmpty() || !m_ControlQueue_list.IsEmpty())) {
				ThrottledControlSocket* socket = NULL;
					
				if(!m_ControlQueueFirst_list.IsEmpty()) {
					socket = m_ControlQueueFirst_list.RemoveHead();
				} else if(!m_ControlQueue_list.IsEmpty()) {
					socket = m_ControlQueue_list.RemoveHead();
				}

				if (socket != NULL) {
					/*
					// try to keep half of data for upload, or sockets will starve and sessions will timeout. 
					SocketSentBytes socketSentBytes = socket->SendControlData(allowedDataRateClass[LAST_CLASS] > 0?(UINT)(BytesToSpend - ControlspentBytes)/2:1, minFragSize);
					*/
					SocketSentBytes socketSentBytes = socket->SendControlData(1, minFragSize);
					uint32 lastSpentBytes = socketSentBytes.sentBytesControlPackets + socketSentBytes.sentBytesStandardPackets;
					if (lastSpentBytes) {
						Socket_stat* stat = NULL;
						if (m_stat_list.Lookup(socket, stat)) {
							stat->realBytesToSpend -= 1000*lastSpentBytes;
							if (stat->classID < LAST_CLASS) {
								realBytesToSpendClass[stat->classID] -= 1000*socketSentBytes.sentBytesStandardPackets;
								sendBytesLocker.Lock();
								m_SentBytesSinceLastCallClass[stat->classID] += lastSpentBytes;
								m_SentBytesSinceLastCallOverheadClass[stat->classID] += socketSentBytes.sentBytesControlPackets;
								sendBytesLocker.Unlock();
							}
						}
						ControlspentBytes += lastSpentBytes;
						ControlspentOverhead += socketSentBytes.sentBytesControlPackets;
					}
       			}
			}
			uint32 numberofclientinhigherclass = 0;
			//loop 3
			for (uint32 classID = 0; classID < NB_SPLITTING_CLASS; classID++) {
				if (slotCounterClass[classID]) {
					uint32 allowedclientdatarate = _UI32_MAX;
					if (allowedDataRateClass[LAST_CLASS])	
						allowedclientdatarate = allowedDataRateClass[LAST_CLASS]; 
   					if (allowedDataRateClass[classID])
						allowedclientdatarate = min(allowedclientdatarate,allowedDataRateClass[classID]); 
					if (ClientDataRate[classID])
						allowedclientdatarate = min(allowedclientdatarate,ClientDataRate[classID]);
					for(uint32 slotCounter = 0; slotCounter < slotCounterClass[classID]; slotCounter++) {
						ThrottledFileSocket* socket = m_StandardOrder_list.GetAt(numberofclientinhigherclass+slotCounter);
						if(socket != NULL) {
							Socket_stat* stat = NULL;
							if (m_stat_list.Lookup(socket, stat)) {
								ASSERT(stat->classID == classID);
								if (timeSinceLastLoop > 0) {
									//Update Bytes To Spend for a socket
									sint64 limit = -((sint64)2000*((allowedclientdatarate == _UI32_MAX || bUploadUnlimited)?doubleSendSize:allowedclientdatarate));
									if (stat->realBytesToSpend < limit)
										stat->realBytesToSpend = limit;
									else if (stat->realBytesToSpend > 999)
										stat->realBytesToSpend = 999;
									
									if (_I64_MAX/timeSinceLastLoop > stat->realBytesToSpend && _I64_MAX-allowedclientdatarate*timeSinceLastLoop > stat->realBytesToSpend)
										stat->realBytesToSpend += allowedclientdatarate*timeSinceLastLoop;
									else
										stat->realBytesToSpend = _I64_MAX;
								}
								if(BytesToSpend > 0 && ControlspentBytes < (uint64)BytesToSpend && stat->realBytesToSpend > 999) {
									SocketSentBytes socketSentBytes = socket->SendFileAndControlData(0, minFragSize);
									uint32 lastSpentBytes = socketSentBytes.sentBytesControlPackets + socketSentBytes.sentBytesStandardPackets;
									if (lastSpentBytes) {
										stat->dwLastBytesSent = thisLoopTick;
										stat->realBytesToSpend -= 1000*lastSpentBytes;
										sendBytesLocker.Lock();
										if(slotCounter < m_highestNumberOfFullyActivatedSlotsClass[classID])
											m_highestNumberOfFullyActivatedSlotsClass[classID] = slotCounter;
										if (classID<LAST_CLASS) {
											realBytesToSpendClass[classID] -= 1000*socketSentBytes.sentBytesStandardPackets;
											m_SentBytesSinceLastCallClass[classID] += lastSpentBytes;
											m_SentBytesSinceLastCallOverheadClass[classID] += socketSentBytes.sentBytesControlPackets;
										}
										sendBytesLocker.Unlock();
										ControlspentBytes += lastSpentBytes;
										ControlspentOverhead += socketSentBytes.sentBytesControlPackets;
									}
								}
							}
						}
					}
					numberofclientinhigherclass += slotCounterClass[classID];
				}
			}
			//
			if (ControlspentBytes) {
				// Comsume overhead byte in higher class too
				for (uint32 i = 0; i < LAST_CLASS; i++) {
					realBytesToSpendClass[i] -= 1000*ControlspentOverhead;
				}
				realBytesToSpendClass[LAST_CLASS] -= 1000*ControlspentBytes;
				sendBytesLocker.Lock();
				m_SentBytesSinceLastCallClass[LAST_CLASS] += ControlspentBytes;
				m_SentBytesSinceLastCallOverheadClass[LAST_CLASS] += ControlspentOverhead;
				sendBytesLocker.Unlock();
			}

			// add in any tcp overhead we got from ack download frames
			if(thePrefs.m_bUseDownloadOverhead) //MORPH leuk_he include download overhead in upload stats
			{
				uint64 downloadoverhead =GetDownDataOverheadOtherPackets_andreset()	;
				//ControlspentOverhead += downloadoverhead;
				realBytesToSpendClass[LAST_CLASS] -= 1000*downloadoverhead;
				sendBytesLocker.Lock();
				m_SentBytesSinceLastCallClass[LAST_CLASS] += downloadoverhead;
				m_SentBytesSinceLastCallOverheadClass[LAST_CLASS] += downloadoverhead;
				sendBytesLocker.Unlock();
				//m_SentBytesSinceLastCallOverheadClass[LAST_CLASS] +=downloadoverhead ; // so it correctly displayed in statistics
			}
			else 
				GetDownDataOverheadOtherPackets_andreset(); // just reset stats (when enabled later in session)


			//loop 4
			numberofclientinhigherclass = 0;
			//if (realBytesToSpendClass[LAST_CLASS]/1000 >= allowedDataRateClass[LAST_CLASS] || allowedDataRateClass[LAST_CLASS] == _UI32_MAX || bUploadUnlimited) {
			//if (  (thePrefs.IsSUCEnabled() || thePrefs.IsDynUpEnabled()) ||
//				(thisLoopTick - dwLastFullBandwidthSpent[LAST_CLASS]) >= 1000/max(1,m_highestNumberOfFullyActivatedSlotsClass[0]+m_highestNumberOfFullyActivatedSlotsClass[1]+m_highestNumberOfFullyActivatedSlotsClass[LAST_CLASS])) {
				for (uint32 classID = 0; classID < NB_SPLITTING_CLASS; classID++) {
					BytesToSpend = realBytesToSpendClass[classID] / 1000;
					if (realBytesToSpendClass[classID] > realBytesToSpendClass[LAST_CLASS])
						BytesToSpend = realBytesToSpendClass[LAST_CLASS] / 1000;
					if (BytesToSpend > 0) {
						if(slotCounterClass[classID]) {						
							uint64 spentBytes = 0;
							uint64 spentOverhead = 0;
							if (ClientDataRate[classID] == 0 && thePrefs.DoRemoveSpareTrickleSlot()) //we are not limiting client data rate AND we remove spare trickle slot
								rememberSlotCounter[classID] = 0; // so activate slot focus
							// loop 4.1
							for(uint32 maxCounter = 0; maxCounter < slotCounterClass[classID] && BytesToSpend > 0 && spentBytes < (uint64)BytesToSpend; maxCounter++) {
								if (rememberSlotCounter[classID] >= slotCounterClass[classID])
									rememberSlotCounter[classID] = 0;
								ThrottledFileSocket* socket = m_StandardOrder_list.GetAt(numberofclientinhigherclass+rememberSlotCounter[classID]);
								if(socket != NULL) {
									Socket_stat* stat = NULL;
									if (m_stat_list.Lookup(socket,stat)) {
										//Try to send client allowed data for a client but not more than class allowed data
										if (stat->realBytesToSpend > 999 && stat->scheduled == false) {
	#if !defined DONT_USE_SOCKET_BUFFERING
											uint32 BytesToSend = (uint32) min((BytesToSpend - (sint64) spentBytes),stat->realBytesToSpend/1000);
											SocketSentBytes socketSentBytes = socket->SendFileAndControlData(BytesToSend, doubleSendSize);
	#else
											SocketSentBytes socketSentBytes = socket->SendFileAndControlData(doubleSendSize, doubleSendSize);
	#endif
											uint32 lastSpentBytes = socketSentBytes.sentBytesControlPackets + socketSentBytes.sentBytesStandardPackets;
											if (lastSpentBytes) {
												stat->dwLastBytesSent = thisLoopTick;
												stat->realBytesToSpend -= 1000*lastSpentBytes;
												spentBytes += lastSpentBytes;
												spentOverhead += socketSentBytes.sentBytesControlPackets;
											}
											sendBytesLocker.Lock();
											if(rememberSlotCounter[classID]+1 > m_highestNumberOfFullyActivatedSlotsClass[classID])
												m_highestNumberOfFullyActivatedSlotsClass[classID] = rememberSlotCounter[classID]+1;
											sendBytesLocker.Unlock();
										}
									}
								}
								++rememberSlotCounter[classID];
							}
							//send remain data to scheduled slot
							for(uint32 slotCounter = 0; slotCounter < slotCounterClass[classID] && BytesToSpend > 0 && spentBytes < (uint64)BytesToSpend; slotCounter++) {
								ThrottledFileSocket* socket = m_StandardOrder_list.GetAt(numberofclientinhigherclass +slotCounter);
								if(socket != NULL) {
									Socket_stat* stat = NULL;
									if (m_stat_list.Lookup(socket,stat)) {
										if (stat->realBytesToSpend > 999) {
	#if !defined DONT_USE_SOCKET_BUFFERING
											uint32 BytesToSend = (uint32)min(((sint64)BytesToSpend - (sint64)spentBytes),stat->realBytesToSpend/1000);
											SocketSentBytes socketSentBytes = socket->SendFileAndControlData(BytesToSend, doubleSendSize);
	#else
											SocketSentBytes socketSentBytes = socket->SendFileAndControlData((UINT)(BytesToSpend - spentBytes), doubleSendSize);
	#endif
											uint32 lastSpentBytes = socketSentBytes.sentBytesControlPackets + socketSentBytes.sentBytesStandardPackets;
											if (lastSpentBytes) {
												stat->dwLastBytesSent = thisLoopTick;
												stat->realBytesToSpend -= 1000*lastSpentBytes;
												spentBytes += lastSpentBytes;
												spentOverhead += socketSentBytes.sentBytesControlPackets;
											}
											sendBytesLocker.Lock();
											if(slotCounter+1 > m_highestNumberOfFullyActivatedSlotsClass[classID])
												m_highestNumberOfFullyActivatedSlotsClass[classID] = slotCounter+1;
											sendBytesLocker.Unlock();
										}
									}
								}
							}

							if (spentBytes) {
								sendBytesLocker.Lock();
								if (classID < LAST_CLASS) {
									for (uint32 i = 0; i < LAST_CLASS; i++) {
										if (i != classID)
										realBytesToSpendClass[i] -= 1000*spentOverhead;
									}
									realBytesToSpendClass[classID] -= spentBytes*1000;
									m_SentBytesSinceLastCallClass[classID] += spentBytes;
									m_SentBytesSinceLastCallOverheadClass[classID] += spentOverhead;
								}
								realBytesToSpendClass[LAST_CLASS] -= spentBytes*1000;
								m_SentBytesSinceLastCallClass[LAST_CLASS] += spentBytes;
								m_SentBytesSinceLastCallOverheadClass[LAST_CLASS] += spentOverhead;
								sendBytesLocker.Unlock();
							}
							
							if ((uint64)BytesToSpend > spentBytes) {
								if (spentBytes+ControlspentBytes > 0) { 
                                    sendBytesLocker.Lock();
									m_highestNumberOfFullyActivatedSlotsClass[classID] = slotCounterClass[classID]+1;
									sendBytesLocker.Unlock();
							    }
							} else {
								dwLastFullBandwidthSpent[classID] = thisLoopTick; 
							}
						} else {
							sendBytesLocker.Lock();
							m_highestNumberOfFullyActivatedSlotsClass[classID] = 1;
							sendBytesLocker.Unlock();
						}
					} else {
							dwLastFullBandwidthSpent[classID] = thisLoopTick;
					}
					numberofclientinhigherclass += slotCounterClass[classID];
				}
	//		} // if suc or uss
			sendLocker.Unlock();
			lastLoopTickTryTosend = thisLoopTick;
		}
	}
													 
    threadEndedEvent->SetEvent();

    tempQueueLocker.Lock();
    m_TempControlQueue_list.RemoveAll();
	m_TempControlQueueFirst_list.RemoveAll();
    tempQueueLocker.Unlock();

    sendLocker.Lock();

    m_ControlQueue_list.RemoveAll();
    POSITION			pos = m_stat_list.GetStartPosition();
	ThrottledControlSocket* socket;
	Socket_stat* stat;
	while (pos)
	{
		m_stat_list.GetNextAssoc(pos, socket, stat);
		delete stat;
	}
	m_stat_list.RemoveAll();
	m_StandardOrder_list.RemoveAll();
    sendLocker.Unlock();

    return 0;
}

