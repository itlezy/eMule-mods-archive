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
//#include "EMSocket.h"
#include "ListenSocket.h" //Xman I use ClientRequestsockets 
#include "opcodes.h"
//#include "LastCommonRouteFinder.h" //Xman
#include "OtherFunctions.h"
#include "emuledlg.h"
#include "Preferences.h" //Xman Xtreme Upload
#include "BandWidthControl.h" // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/**
 * The constructor starts the thread.
 */
UploadBandwidthThrottler::UploadBandwidthThrottler(void) {
	//Xman Xtreme Upload unused
	/*
	m_SentBytesSinceLastCall = 0;
	m_SentBytesSinceLastCallOverhead = 0;
	*/
	// ==> Mephisto Upload - Mephisto
	/*
    m_highestNumberOfFullyActivatedSlots = 0;
	m_highestNumberOfFullyActivatedSlots_out =0;
	//Xman Xtreme Upload
	needslot=false;
	recalculate=true;
	nexttrickletofull=true;
	*/
	needslot=0;
	// <== Mephisto Upload - Mephisto
	//Xman end

	//Xman count block/success send
	avgBlockRatio=0;
	//Xman upload health
	avg_health=100;
	sum_healthhistory=0;
	m_countsend=0;
	m_countsendsuccessful=0;
	//Xman end

	// ==> Mephisto Upload - Mephisto
	m_uActiveSlotCount=0;
	m_bBandwithReached=false;
	// <== Mephisto Upload - Mephisto

	threadEndedEvent = new CEvent(0, 1);
	pauseEvent = new CEvent(TRUE, TRUE);

	doRun = true;
	AfxBeginThread(RunProc,(LPVOID)this, THREAD_PRIORITY_ABOVE_NORMAL); //Xman  
}

/**
 * The destructor stops the thread. If the thread has already stoppped, destructor does nothing.
 */
UploadBandwidthThrottler::~UploadBandwidthThrottler(void) {
	EndThread();
	delete threadEndedEvent;
	delete pauseEvent;
}

/**
 * Find out how many bytes that has been put on the sockets since the last call to this
 * method. Includes overhead of control packets.
 *
 * @return the number of bytes that has been put on the sockets since the last call
 */
//Xman Xtreme Upload unused
/*
uint64 UploadBandwidthThrottler::GetNumberOfSentBytesSinceLastCallAndReset() {
	sendLocker.Lock();

	uint64 numberOfSentBytesSinceLastCall = m_SentBytesSinceLastCall;
	m_SentBytesSinceLastCall = 0;

	sendLocker.Unlock();

	return numberOfSentBytesSinceLastCall;
}
*/
/**
 * Find out how many bytes that has been put on the sockets since the last call to this
 * method. Excludes overhead of control packets.
 *
 * @return the number of bytes that has been put on the sockets since the last call
 */
/*
uint64 UploadBandwidthThrottler::GetNumberOfSentBytesOverheadSinceLastCallAndReset() {
	sendLocker.Lock();

	uint64 numberOfSentBytesSinceLastCall = m_SentBytesSinceLastCallOverhead;
	m_SentBytesSinceLastCallOverhead = 0;

	sendLocker.Unlock();

	return numberOfSentBytesSinceLastCall;
}
*/

//Xman Xtreme Upload //method is currently not used
//void UploadBandwidthThrottler::ReplaceSocket(ThrottledFileSocket* oldsocket, ThrottledFileSocket* newsocket)
//{
//	sendLocker.Lock();
//	int slotnumber=-1;
//	bool isfull=false;
//	//bool isready=false;
//
//	if(oldsocket==NULL)theApp.QueueLogLine(false,_T("oldsocket NULL"));
//	if(newsocket==NULL) theApp.QueueLogLine(false,_T("newsocket NULL"));
//
//	if(oldsocket != NULL && newsocket != NULL)
//	{
//		for(slotnumber=0;slotnumber<m_StandardOrder_list.GetSize();slotnumber++)
//			if(m_StandardOrder_list.GetAt(slotnumber) == oldsocket)
//			{
//				//remember the values
//				isfull=oldsocket->IsFull();
//				//isready=oldsocket->isready;
//				RemoveFromStandardListNoLock(oldsocket);
//				break;
//			}
//	}
//	if(slotnumber>=0 && slotnumber<m_StandardOrder_list.GetSize()) //found
//	{
//		m_StandardOrder_list.InsertAt(slotnumber, newsocket);
//		if(isfull)
//		{
//			newsocket->SetFull();
//			m_StandardOrder_list_full.AddTail(newsocket);
//			m_highestNumberOfFullyActivatedSlots++;
//			SetNumberOfFullyActivatedSlots();
//		}
//		else
//			newsocket->SetTrickle();
//		if(newsocket->StandardPacketQueueIsEmpty()==false)
//			newsocket->isready=true;
//		else
//			newsocket->isready=false;
//		theApp.QueueLogLine(false,_T("->replaced socket on pos: %u isfull: %u "),slotnumber, isfull);
//	}
//	else
//	{
//		/*
//		m_StandardOrder_list.InsertAt(m_StandardOrder_list.GetSize(),newsocket);
//		newsocket->SetTrickle();
//		newsocket->isready=false;
//		recalculate=true;
//		*/
//		theApp.QueueLogLine(false,_T("-->tried to replace a not existing socket"));
//	}
//
//	sendLocker.Unlock();
//}

//Xman Xtreme Upload: Peercache-part
//threadsafe for Main-thread (only caller) and upbloadbandwidthThrottler (sendLocker)
bool UploadBandwidthThrottler::ReplaceSocket(ThrottledFileSocket* normalsocket, ThrottledFileSocket* pcsocket, ThrottledFileSocket* newsocket)
{
	sendLocker.Lock();
	int slotnumber=-1;
	// ==> Mephisto Upload - Mephisto
	/*
	bool isfull=false;
	*/
	// <== Mephisto Upload - Mephisto


	if(newsocket==NULL)
	{
		theApp.QueueDebugLogLine(false, _T("ReplaceSocket-> NULL socket!"));
		sendLocker.Unlock();
		return false;
	}

	if(pcsocket!=NULL)
	{
		for(slotnumber=0;slotnumber<m_StandardOrder_list.GetSize();slotnumber++)
			if(m_StandardOrder_list.GetAt(slotnumber) == pcsocket)
			{
				//remember the values
				// ==> Mephisto Upload - Mephisto
				/*
				isfull=pcsocket->IsFull();
				*/
				// <== Mephisto Upload - Mephisto
				RemoveFromStandardListNoLock(pcsocket);
				break;
			}
	}
	if(slotnumber>=0 && slotnumber<m_StandardOrder_list.GetSize())
	{
		DEBUG_ONLY( Debug(_T("ReplaceSocket-> found peercachesocket!\n")));
	}
	else
	{
		if(normalsocket==NULL)
			DEBUG_ONLY( Debug(_T("ReplaceSocket-> error: no peercachesocket and normal is NULL-socket!\n")));
		else
		{
			for(slotnumber=0;slotnumber<m_StandardOrder_list.GetSize();slotnumber++)
			{
				if(m_StandardOrder_list.GetAt(slotnumber) == normalsocket)
				{
					//remember the values
					// ==> Mephisto Upload - Mephisto
					/*
					isfull=normalsocket->IsFull();
					*/
					// <== Mephisto Upload - Mephisto
					RemoveFromStandardListNoLock(normalsocket);
					break;
				}
			}
			if(slotnumber>=0 && slotnumber<m_StandardOrder_list.GetSize())
			{
				DEBUG_ONLY( Debug(_T("ReplaceSocket-> found normalsocket!")));
			}
		}
	}

	if(slotnumber>=0 && slotnumber<m_StandardOrder_list.GetSize())
	{
		m_StandardOrder_list.InsertAt(slotnumber, newsocket);
		// ==> Mephisto Upload - Mephisto
		/*
		if(isfull)
		{
			newsocket->SetFull();
			m_StandardOrder_list_full.AddTail(newsocket);
			m_highestNumberOfFullyActivatedSlots++;
			SetNumberOfFullyActivatedSlots();
		}
		else
			newsocket->SetTrickle();
		if(newsocket->StandardPacketQueueIsEmpty()==false)
			newsocket->isready=true;
		else
			newsocket->isready=false;
		DEBUG_ONLY( Debug(_T("->replaced socket on pos: %u isfull: %u \n"),slotnumber, isfull));
		*/
		newsocket->SetUploading();
		if(newsocket->StandardPacketQueueIsEmpty()==false)
			newsocket->isready=true;
		else
			newsocket->isready=false;
		DEBUG_ONLY( Debug(_T("->replaced socket on pos: %u \n"),slotnumber));
		// <== Mephisto Upload - Mephisto
	}
	else
	{
		DEBUG_ONLY( Debug(_T("ReplaceSocket-> found no socket to replace!\n")));
		sendLocker.Unlock();
		return false;
	}
	sendLocker.Unlock();
	return true;

}

//Xman Xtreme Upload
//threadsafe for Main-thread (only caller) and upbloadbandwidthThrottler (sendLocker)
// ==> Mephisto Upload - Mephisto
/*
void UploadBandwidthThrottler::AddToStandardList(bool first, ThrottledFileSocket* socket) {
	if(socket != NULL) {
		sendLocker.Lock();

		RemoveFromStandardListNoLock(socket);
		if(first)
		{
			m_StandardOrder_list.InsertAt(0, socket);
			socket->SetFull();
			m_StandardOrder_list_full.AddTail(socket);
			m_highestNumberOfFullyActivatedSlots++;
			SetNumberOfFullyActivatedSlots();
		}
		else
		{
			m_StandardOrder_list.InsertAt(m_StandardOrder_list.GetSize(),socket);
			socket->SetTrickle();
		}
*/
void UploadBandwidthThrottler::AddToStandardList(int posCounter, ThrottledFileSocket* socket, bool bFriend) {
	if(socket != NULL) {
		sendLocker.Lock();

		RemoveFromStandardListNoLock(socket);
		if(posCounter>=0) // add somewhere before the tail
		{
			m_StandardOrder_list.InsertAt(posCounter, socket);
			socket->SetUploading();
		}
		else // add at tail
		{
			m_StandardOrder_list.InsertAt(m_StandardOrder_list.GetSize(),socket);
			socket->SetUploading();
		}
		socket->SetFriendSlot(bFriend);
// <== Mephisto Upload - Mephisto
		//Xman x4
		if(socket->StandardPacketQueueIsEmpty()==false)
			socket->isready=true;
		else
			socket->isready=false;

		//Xman end

		sendLocker.Unlock();
	}
}

//Xman Xtreme Upload
void UploadBandwidthThrottler::SetNoNeedSlot()
{
	sendLocker.Lock();
			needslot=false;
	sendLocker.Unlock();
}

// ==> Mephisto Upload - Mephisto
/*
void UploadBandwidthThrottler::SetNextTrickleToFull()
{
	sendLocker.Lock();
		nexttrickletofull=true;
	sendLocker.Unlock();
}

void UploadBandwidthThrottler::RecalculateOnNextLoop()
{
	sendLocker.Lock();
	recalculate=true;
	needslot=false;
	sendLocker.Unlock();
}
*/
// <== Mephisto Upload - Mephisto
//Xman end

/**
 * Remove a socket from the list of sockets that have upload slots.
 *
 * If the socket has mistakenly been added several times to the list, this method
 * will return all of the entries for the socket.
 *
 * @param socket the address of the socket that should be removed from the list. If this socket
 *               does not exist in the list, this method will do nothing.
 */
bool UploadBandwidthThrottler::RemoveFromStandardList(ThrottledFileSocket* socket) {
    bool returnValue;
	sendLocker.Lock();

	returnValue = RemoveFromStandardListNoLock(socket);

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
bool UploadBandwidthThrottler::RemoveFromStandardListNoLock(ThrottledFileSocket* socket) {
	// Find the slot
	int slotCounter = 0;
	bool foundSocket = false;
	while(slotCounter < m_StandardOrder_list.GetSize() && foundSocket == false) {
		if(m_StandardOrder_list.GetAt(slotCounter) == socket) {
			// Remove the slot
			// ==> Mephisto Upload - Mephisto
			/*
			//Xman Xtreme Upload
			if(m_StandardOrder_list.GetAt(slotCounter)->IsFull()) 
			{
				m_highestNumberOfFullyActivatedSlots--;	
				SetNumberOfFullyActivatedSlots();
			}
			//Xman end
			*/
			// <== Mephisto Upload - Mephisto
			m_StandardOrder_list.RemoveAt(slotCounter);
			//Xman improved socket queuing: re queue the socket for sending control packets 
			socket->SetNoUploading();
			QueueForSendingControlPacket(socket,true);
			//Xman end
			foundSocket = true;
		} else {
			slotCounter++;
        }
	}

	
	// ==> Mephisto Upload - Mephisto
	/*
	POSITION pos;
	pos=m_StandardOrder_list_full.Find(socket);
	if(pos!=NULL)
	{
		m_StandardOrder_list_full.RemoveAt(pos);
		//Xman final version:
		if(m_StandardOrder_list.GetSize())
		{
			//calculate overhead of last 20 seconds
			uint32 notused,m_currentAvgEmuleOut,m_currentAvgOverallSentBytes ,m_currentAvgNetworkOut,m_AvgOverhead;
			theApp.pBandWidthControl->GetDatarates(20,
				notused, notused,
				m_currentAvgEmuleOut, m_currentAvgOverallSentBytes,
				notused,m_currentAvgNetworkOut);
			uint32 slotspeed=(uint32)(thePrefs.m_slotspeed*1024);
			if(thePrefs.GetNAFCFullControl()==true)
			{
				m_AvgOverhead=m_currentAvgNetworkOut-m_currentAvgEmuleOut;
			}
			else
			{
				m_AvgOverhead=m_currentAvgOverallSentBytes-m_currentAvgEmuleOut;
			}
			uint32 realallowedDatarate = (uint32)(theApp.pBandWidthControl->GetMaxUpload()*1024 - m_AvgOverhead);
			//Xman 4.6
			//to be more accurate subtract the trickles
			//(tricklespeed is 500 Bytes per seconds)
			const uint16 counttrickles=(uint16)m_StandardOrder_list.GetSize() - m_highestNumberOfFullyActivatedSlots;
			realallowedDatarate -= ( counttrickles * 500);
			const uint16 savedbytes = counttrickles > 1 ? 500 : 0;
			//calculate the wanted slots
			uint16 slots=(uint16)max(realallowedDatarate/slotspeed,1);
			if((realallowedDatarate-slots*slotspeed) > ((slots+1)*slotspeed-realallowedDatarate) - savedbytes)
			{
				slots++;
			}
			if(slots > m_highestNumberOfFullyActivatedSlots)
				nexttrickletofull=true;
		}
	}
	//Xman end
	*/
	// <== Mephisto Upload - Mephisto

	//Xman unused
	/*
    if(foundSocket && m_highestNumberOfFullyActivatedSlots > (uint32)m_StandardOrder_list.GetSize()) {
        m_highestNumberOfFullyActivatedSlots = m_StandardOrder_list.GetSize();
    }
	*/
	//Xman end
    return foundSocket;
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
	if(lock) {
		// Get critical section
		sendLocker.Lock();
    }

	if(doRun) {
        // Remove this socket from control packet queue
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
	}

	if(lock) {
		// End critical section
		sendLocker.Unlock();
    }
}

void UploadBandwidthThrottler::RemoveFromAllQueues(ThrottledFileSocket* socket) {
	// Get critical section
	sendLocker.Lock();

	if(doRun) {
		// And remove it from upload slots
		RemoveFromStandardListNoLock(socket);
		RemoveFromAllQueues(socket, false); //Xman 
	}

	// End critical section
	sendLocker.Unlock();
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

//Xman upload unused
/*
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
*/
//Xman end

/**
 * Start the thread. Called from the constructor in this class.
 *
 * @param pParam
 *
 * @return
 */
UINT AFX_CDECL UploadBandwidthThrottler::RunProc(LPVOID pParam) {
	DbgSetThreadName("UploadBandwidthThrottler");
	//Xman
	// BEGIN SLUGFILLER: SafeHash
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return 0;
	// END SLUGFILLER: SafeHash

	InitThreadLocale();

	UploadBandwidthThrottler* uploadBandwidthThrottler = (UploadBandwidthThrottler*)pParam;

	return uploadBandwidthThrottler->RunInternal();
}

#ifdef PRINT_STATISTIC
void UploadBandwidthThrottler::PrintStatistic()
{
	sendLocker.Lock();
	uint32 amount=m_ControlQueue_list.GetCount();
	amount += m_ControlQueueFirst_list.GetCount();
	AddLogLine(false, _T("Queued Control-Sockets: %u"), amount);
	sendLocker.Unlock();
}
#endif

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
	//uint16 rememberedSlotCounterMain = 0;
	// ==> Mephisto Upload - Mephisto
	/*
	uint16 rememberedSlotCounterTrickle = 0;
	*/
	// <== Mephisto Upload - Mephisto
	uint32 allowedDataRate = 1000;
	sint64 uSlope=0;
	sint64 uSlopehelp=0;
	// ==> Mephisto Upload - Mephisto
	/*
	uint32 slotspeed=0; 
	*/
	// <== Mephisto Upload - Mephisto
	const uint32 TRICKLESPEED=500; //~0.5kbs
	const uint16 MAXSLOPEBUFFERTIME=1; //1 sec
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	uint64  m_lastOverallSentBytes = theApp.pBandWidthControl->GeteMuleOutOverall();
    uint64  m_lastNetworkOut = theApp.pBandWidthControl->GetNetworkOut();
	uint64  m_currentOverallSentBytes=m_lastOverallSentBytes;
	uint64  m_currentNetworkOut=m_lastNetworkOut;
	// ==> Mephisto Upload - Mephisto
	/*
	uint32	m_currentAvgOverallSentBytes=0;
	uint32  m_currentAvgNetworkOut=0;
	uint32  m_currentAvgEmuleOut=0;
	uint32  m_AvgOverhead=0; 
	*/
	// <== Mephisto Upload - Mephisto
	
    uint32 spentBytes = 0; //during one loop
	uint32 spentOverhead=0; // " 
    uint32 spentBytesFriends = 0; // Multiple friendslots [ZZ] - Mephisto

	//Xman 4.4 Code-Improvement: reserve 1/3 of your uploadlimit for emule
	//reason: if you have a big/long ftp-transfer, all bandwidth goes to the ftp-client
	//and emule times out
	sint64 uSlopehelp_minUpload=0; 
	sint64 uSlopehelp_tinyUpload=0; // Do not reserve 1/3 of your uploadlimit for emule [Stulle] - Stulle

	DWORD lastTickReachedBandwidth = timeGetTime();

	// ==> Mephisto Upload - Mephisto
	/*
	uint16 minslots=0;
	*/
	uint16 minslots=thePrefs.GetMinSlots();
	DWORD lastTickHadTrickle = timeGetTime();
	bool bHadTrickle = false;
	// <== Mephisto Upload - Mephisto
	//Xman count block/success send
	uint32 last_block_process = timeGetTime() >> 10;
	// ==> Dynamic Socket Buffering [SiRoB] - Mephisto
#ifdef DONT_USE_SOCKET_BUFFERING
	bool bAlwaysEnableBigSocketBuffers = false;
#endif
	// <== Dynamic Socket Buffering [SiRoB] - Mephisto

	while(doRun) {
        pauseEvent->Lock();

		DWORD timeSinceLastLoop = timeGetTime() - lastLoopTick;

#define TIME_BETWEEN_UPLOAD_LOOPS_MIN 4 //Xman 4(was 8) are enough -> using higher prio for this thread
#define TIME_BETWEEN_UPLOAD_LOOPS_MAX 50 //25
        uint32 sleeptime=TIME_BETWEEN_UPLOAD_LOOPS_MIN;
		///*
		// ==> Mephisto Upload - Mephisto
		/*
		if(uSlope<0)
		*/
		minslots=thePrefs.GetMinSlots(); // this could have changed
		if(uSlope<500)
		// <== Mephisto Upload - Mephisto
		{
			//Xman 5.01 new High-Res-Timer oftenly sleep to short... it's probably too accurate (normal timer sleep always too long)
			//to not do unnecessary loops sleep until we have 500 Bytes to spend.
			//zz_fly :: Optimized :: start
			//note: use integer+if is faster than float+marco.
			/*
			sleeptime=min((uint32)ceil((float)((-uSlope+500)*1000)/allowedDataRate),TIME_BETWEEN_UPLOAD_LOOPS_MAX); //was 1 byte to spend
			*/
			sleeptime = (uint32)(((-uSlope+500)*1000 + allowedDataRate-1) / allowedDataRate);
			if(sleeptime > TIME_BETWEEN_UPLOAD_LOOPS_MAX)
				sleeptime = TIME_BETWEEN_UPLOAD_LOOPS_MAX;
			//zz_fly :: Optimized :: end
		}
		if(sleeptime<TIME_BETWEEN_UPLOAD_LOOPS_MIN)
			sleeptime=TIME_BETWEEN_UPLOAD_LOOPS_MIN;
		//*/	
	   if(timeSinceLastLoop < sleeptime) 
	   {
            Sleep(sleeptime-timeSinceLastLoop);
       }

		const DWORD thisLoopTick = timeGetTime();
		timeSinceLastLoop = thisLoopTick - lastLoopTick;
		lastLoopTick = thisLoopTick;
		if(timeSinceLastLoop==0)
		{	continue; //shouldn't happen
			//theApp.QueueDebugLogLine(false,_T("UploadBandwidthThrottler: Application to fast"));
		}
		if(timeSinceLastLoop>1000)
		{
			timeSinceLastLoop=1000;	//compensate to 1 sec
			//theApp.QueueDebugLogLine(false,_T("UploadBandwidthThrottler: Application hang"));
		}

		// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
		theApp.pBandWidthControl->Process(); 

		// Get current speed from prefs
		// ==> Mephisto Upload - Mephisto
		/*
		uint32 old_value;
		old_value=allowedDataRate;
		allowedDataRate = (uint32)(theApp.pBandWidthControl->GetMaxUpload()*1024); 
		if(allowedDataRate<old_value)
		{
			if(old_value -  allowedDataRate >= 1024)
				recalculate=true; //readjust socket: trickle or full
		}
		old_value=slotspeed;
		slotspeed= (uint32)(thePrefs.m_slotspeed*1024);
		if(slotspeed==0)
			slotspeed=1024; //prevent division by zero
		if(old_value != slotspeed)
			recalculate=true;
		*/
		allowedDataRate = (uint32)(theApp.pBandWidthControl->GetMaxUpload()*1024); 
		// <== Mephisto Upload - Mephisto


		uint32 minFragSize = thePrefs.GetMTU() - (20 /*IP*/ + 20 /*TCP*/); // Maella -MTU Configuration- //Xman Bandwidthtest
		// ==> Mephisto Upload - Mephisto
		/*
		uint32 doubleSendSize = thePrefs.usedoublesendsize ? minFragSize*2 : minFragSize; // send two packages at a time so they can share an ACK
		if(allowedDataRate < 6*1024) 
		{
			minFragSize = 536;
			doubleSendSize = minFragSize; // don't send two packages at a time at very low speeds to give them a smoother load
		}
		else if(allowedDataRate < 16*1024)
		{
			doubleSendSize = minFragSize;
		}
		*/
		if(allowedDataRate < 6*1024) 
			minFragSize = 536;
		// <== Mephisto Upload - Mephisto

		m_currentOverallSentBytes=theApp.pBandWidthControl->GeteMuleOutOverall();
		m_currentNetworkOut=theApp.pBandWidthControl->GetNetworkOut();
		
		///*
		//fetch the last data + overhead / networkinterface
		if(thePrefs.GetNAFCFullControl()==true)
		{
			//uSlope will hold the amount of data, we allow to send this loop
			uSlopehelp -= (m_currentNetworkOut - m_lastNetworkOut);
		}
		else
		{
			uSlopehelp -= (m_currentOverallSentBytes - m_lastOverallSentBytes);
		}
        //*/
		uSlopehelp_minUpload -= (uint32)(m_currentOverallSentBytes - m_lastOverallSentBytes); //Xman 4.4 Code-Improvement: reserve 1/3 of your uploadlimit for emule
		uSlopehelp_tinyUpload -= (uint32)(m_currentOverallSentBytes - m_lastOverallSentBytes); // Do not reserve 1/3 of your uploadlimit for emule [Stulle] - Stulle

		//don't go over limit during the start-phase
		// ==> Mephisto Upload - Mephisto
		/*
		minslots=(uint16)(allowedDataRate/slotspeed/2);
		*/
		// <== Mephisto Upload - Mephisto

		if(uSlopehelp>0 && minslots > (uint16)m_StandardOrder_list.GetSize())
			uSlopehelp=0;

		const uint32 toadd=(uint32)(allowedDataRate*(float)timeSinceLastLoop/1000);
		uSlopehelp +=toadd;
		uSlopehelp_minUpload += (uint32)(toadd*0.33f); //Xman 4.4 Code-Improvement: reserve 1/3 of your uploadlimit for emule
		// ==> Do not reserve 1/3 of your uploadlimit for emule [Stulle] - Stulle
		uSlopehelp_tinyUpload += (uint32)(2048*(float)timeSinceLastLoop/1000);
		// <== Do not reserve 1/3 of your uploadlimit for emule [Stulle] - Stulle


		// Keep current value for next processing    
        m_lastOverallSentBytes = m_currentOverallSentBytes;
        m_lastNetworkOut = m_currentNetworkOut;

		//compensate:
		if(uSlopehelp > allowedDataRate*MAXSLOPEBUFFERTIME*0.25f) //max 250ms //Xman 
			uSlopehelp=(sint64)(allowedDataRate*MAXSLOPEBUFFERTIME*0.25f); 
		else if(uSlopehelp < -(sint64)(allowedDataRate*MAXSLOPEBUFFERTIME*0.25f)) //max 250ms
			uSlopehelp=-((sint64)(allowedDataRate*MAXSLOPEBUFFERTIME*0.25f));

		//Xman 4.4 Code-Improvement: reserve 1/3 of your uploadlimit for emule
		if(uSlopehelp_minUpload > allowedDataRate*MAXSLOPEBUFFERTIME*0.25f) 
			uSlopehelp_minUpload=(sint64)(allowedDataRate*MAXSLOPEBUFFERTIME*0.25f); 
		else if(uSlopehelp_minUpload < -(sint64)(allowedDataRate*MAXSLOPEBUFFERTIME*0.25f)) 
			uSlopehelp_minUpload=-((sint64)(allowedDataRate*MAXSLOPEBUFFERTIME*0.25f));

		
		// ==> Do not reserve 1/3 of your uploadlimit for emule [Stulle] - Stulle
		/*
		if(thePrefs.GetNAFCFullControl()==true && uSlopehelp_minUpload>uSlopehelp)
			uSlope=uSlopehelp_minUpload;
		*/
		if(uSlopehelp_tinyUpload > 2048*MAXSLOPEBUFFERTIME*0.25f) 
			uSlopehelp_tinyUpload=(sint64)(2048*MAXSLOPEBUFFERTIME*0.25f); 
		else if(uSlopehelp_tinyUpload < -(sint64)(2048*MAXSLOPEBUFFERTIME*0.25f)) 
			uSlopehelp_tinyUpload=-((sint64)(2048*MAXSLOPEBUFFERTIME*0.25f));

		if(thePrefs.GetNAFCFullControl()==true && // NAFC enabled
			thePrefs.GetIgnoreThird() && // ignore the third
			theApp.pBandWidthControl->GeteMuleIn() <= (theApp.pBandWidthControl->GeteMuleOut()*2) && // ratio max 1:2
			uSlopehelp_tinyUpload>uSlopehelp)
			uSlope=uSlopehelp_tinyUpload;
		else if(thePrefs.GetNAFCFullControl()==true && uSlopehelp_minUpload>uSlopehelp)
			uSlope=uSlopehelp_minUpload;
		// <== Do not reserve 1/3 of your uploadlimit for emule [Stulle] - Stulle
		else
			uSlope=uSlopehelp;


		sendLocker.Lock();

	    spentBytes = 0;
		spentOverhead=0;

		// ==> Mephisto Upload - Mephisto
		/*
		//slot control:
		//this section regulates the slot-state (trickle, full)
		if(m_StandardOrder_list.GetSize() && recalculate)
		{
			recalculate=false;
			//calculate overhead of last 20 seconds
			uint32 notused;
			theApp.pBandWidthControl->GetDatarates(20,
										notused, notused,
										m_currentAvgEmuleOut, m_currentAvgOverallSentBytes,
										notused,m_currentAvgNetworkOut);

			if(thePrefs.GetNAFCFullControl()==true)
			{
				m_AvgOverhead=m_currentAvgNetworkOut-m_currentAvgEmuleOut;
			}
			else
			{
				m_AvgOverhead=m_currentAvgOverallSentBytes-m_currentAvgEmuleOut;
			}
			uint32 realallowedDatarate = allowedDataRate-m_AvgOverhead;
			//Xman 4.6
			//to be more accurate subtract the trickles
			//(tricklespeed is 500 Bytes per seconds)
			const uint16 counttrickles=(uint16)m_StandardOrder_list.GetSize() - m_highestNumberOfFullyActivatedSlots;
			realallowedDatarate -= ( counttrickles * 500);
			const uint16 savedbytes = counttrickles > 1 ? 500 : 0;

			//calculate the wanted slots
			uint16 slots=(uint16)(max(realallowedDatarate/slotspeed,1));
			if(slots>=m_StandardOrder_list.GetSize())
			{	//we don't have enough slots
				needslot=true;
				m_highestNumberOfFullyActivatedSlots=0;
				m_StandardOrder_list_full.RemoveAll(); 
				for(uint16 i=0;i<m_StandardOrder_list.GetSize();i++)
				{
					ThrottledFileSocket* socket = m_StandardOrder_list.GetAt(i);
					socket->SetFull();
					m_StandardOrder_list_full.AddTail(socket); 
					m_highestNumberOfFullyActivatedSlots++;
				}
				SetNumberOfFullyActivatedSlots();
			}
			else
			{	//calculate the best amount of full slots
				if((realallowedDatarate-slots*slotspeed) > ((slots+1)*slotspeed-realallowedDatarate) - savedbytes)
				{
					slots++;
				}
				m_highestNumberOfFullyActivatedSlots=0;
				m_StandardOrder_list_full.RemoveAll(); 
				for(uint16 i=0;i<m_StandardOrder_list.GetSize();i++)
				{
					ThrottledFileSocket* socket = m_StandardOrder_list.GetAt(i);
					if(i<slots)
					{
						socket->SetFull();
						m_StandardOrder_list_full.AddTail(socket); 
						m_highestNumberOfFullyActivatedSlots++;
					}
					else
						socket->SetTrickle();
				}
				SetNumberOfFullyActivatedSlots();
			}
			// if we are uploading fast, increase the sockets sendbuffers in order to be able to archive faster
			// speeds
			// NOTE: We do not call this an awful lot so it's fine to use a more constant approach here.
			//bool bUseBigBuffers = bAlwaysEnableBigSocketBuffers;
			if (allowedDataRate/slots > 100 * 1024 && realallowedDatarate > 300 * 1024)
				bAlwaysEnableBigSocketBuffers = true;
			else
				bAlwaysEnableBigSocketBuffers = false;
		}
		else if(nexttrickletofull)		
		{
			if(m_StandardOrder_list.GetSize()==0 || m_StandardOrder_list.GetSize()<=m_highestNumberOfFullyActivatedSlots)
			{
				needslot=true;
				nexttrickletofull=false; 
			}
			else
			{
				nexttrickletofull=false;
				ThrottledFileSocket* socket = m_StandardOrder_list.GetAt(m_highestNumberOfFullyActivatedSlots);
				socket->SetFull();
				m_StandardOrder_list_full.AddTail(socket); 
				m_highestNumberOfFullyActivatedSlots++;
				SetNumberOfFullyActivatedSlots();
			}
		}
		*/
		spentBytesFriends = 0;
		// ==> Dynamic Socket Buffering [SiRoB] - Mephisto
#ifdef DONT_USE_SOCKET_BUFFERING
		// if we are uploading fast, increase the sockets sendbuffers in order to be able to archive faster
		// speeds
		// NOTE: Since we use slot focus we start using bigger sockets ealier
		if (allowedDataRate > 150 * 1024)
			bAlwaysEnableBigSocketBuffers = true;
		else
			bAlwaysEnableBigSocketBuffers = false;
#endif
		// <== Dynamic Socket Buffering [SiRoB] - Mephisto
		// <== Mephisto Upload - Mephisto


		//Send Control Data:
        if(uSlope >0) 
		{
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
    
		    // Send any queued up control packets first
		    while(uSlope > 0 && spentBytes < uSlope && (!m_ControlQueueFirst_list.IsEmpty() || !m_ControlQueue_list.IsEmpty())) {
			    ThrottledControlSocket* socket = NULL;
    
                if(!m_ControlQueueFirst_list.IsEmpty()) {
                    socket = m_ControlQueueFirst_list.RemoveHead();
                } else if(!m_ControlQueue_list.IsEmpty()) {
                    socket = m_ControlQueue_list.RemoveHead();
                }
    
			    if(socket != NULL) 
				{
					SocketSentBytes socketSentBytes = socket->SendControlData(/*uSlope-spentBytes*/  minFragSize, minFragSize); //Xman4.5
				    uint32 lastSpentBytes = socketSentBytes.sentBytesControlPackets + socketSentBytes.sentBytesStandardPackets;
				    spentBytes += lastSpentBytes;
				    spentOverhead += socketSentBytes.sentBytesControlPackets;
			    }
		    }
			uSlope -=spentBytes;
		}

		// ==> Mephisto Upload - Mephisto
		/*
		//Xman 3.1
		if(m_highestNumberOfFullyActivatedSlots!=m_StandardOrder_list_full.GetSize())
		{
			theApp.QueueDebugLogLine(false, _T("Warning m_highestNumberOfFullyActivatedSlots not equal list"));
			recalculate=true;
		}

		//First all the trickles:
		if(m_StandardOrder_list.GetSize()==0 || (m_highestNumberOfFullyActivatedSlots >= (uint32)m_StandardOrder_list.GetSize()) )
			needslot=true;
		else
		{
			bool foundtrickle=false; //looks if we have a ready trickle
			for(uint16 i=m_highestNumberOfFullyActivatedSlots;i<m_StandardOrder_list.GetSize();i++)
			{
				ThrottledFileSocket* socket = m_StandardOrder_list.GetAt(i);
				if(socket!=NULL && socket->isready==true) 
				{
					if(socket->IsTrickle()==false)
					{
						theApp.QueueDebugLogLine(false, _T("Warning trickle on wrong possition"));
						recalculate=true;
					}
					
					foundtrickle=true;
					if(timeSinceLastLoop==1000)
					{
						//application hang
						socket->CSlope=1;
					}
					else
					{
						socket->CSlope+=(TRICKLESPEED*timeSinceLastLoop/1000);
					}
					//compensate:
					if(socket->CSlope>(sint32)TRICKLESPEED*MAXSLOPEBUFFERTIME)
						socket->CSlope=TRICKLESPEED*MAXSLOPEBUFFERTIME;

					//now we can send:
					if(uSlope >0 && socket->CSlope>0) 
					{
						    SocketSentBytes socketSentBytes = socket->SendFileAndControlData(minFragSize, minFragSize); 
						    uint32 lastSpentBytes = socketSentBytes.sentBytesControlPackets + socketSentBytes.sentBytesStandardPackets;
						    spentBytes += lastSpentBytes;
						    spentOverhead += socketSentBytes.sentBytesControlPackets;
							uSlope-=lastSpentBytes;
							socket->CSlope-=socketSentBytes.sentBytesStandardPackets;
					}
				}
			}
			if(foundtrickle==false) //no ready trickle found, we need a new slot
				needslot=true;
		}
		*/
		// <== Mephisto Upload - Mephisto
		
		/*
		//Xman x4 just test slotfocus test to not timeout:
		if(uSlope>0)
		{
			POSITION pos=m_StandardOrder_list_full.GetHeadPosition();
			for(uint16 i=0;i<m_StandardOrder_list_full.GetCount();i++)
			{
				POSITION cur_pos = pos;
				ThrottledFileSocket* socket = m_StandardOrder_list_full.GetNext(pos);
				if(socket!=NULL  && socket->isready==true) 
				{
					if(uSlope>0 && thisLoopTick-socket->GetLastCalledSend() > SEC2MS(2))
					{
						SocketSentBytes socketSentBytes = socket->SendFileAndControlData(minFragSize,minFragSize); //Xman4.5  //Xman only one package  
						uint32 lastSpentBytes = socketSentBytes.sentBytesControlPackets + socketSentBytes.sentBytesStandardPackets;
						spentBytes += lastSpentBytes;
						spentOverhead += socketSentBytes.sentBytesControlPackets;
						uSlope-=lastSpentBytes;
					}
				}
			}
		}
		*/


		// ==> Mephisto Upload - Mephisto
		/*
		//Xman Full upload new version
		if(uSlope>0)
		{
			uint16 k=0;
			POSITION pos=m_StandardOrder_list_full.GetHeadPosition();
			POSITION cur_pos = pos;
			ThrottledFileSocket* socket=NULL;
			while(uSlope>0 && k < m_StandardOrder_list_full.GetCount())
			{
				if(pos!=NULL)//it's the last socket
				{
					cur_pos = pos;
					socket = m_StandardOrder_list_full.GetNext(pos);
				}
				if(socket!=NULL  && socket->isready==true) 
				{
					if(socket->IsFull()==false)
					{
						theApp.QueueDebugLogLine(false, _T("Warning full on wrong possition"));
						recalculate=true;
					}
					if (bAlwaysEnableBigSocketBuffers)
						socket->UseBigSendBuffer();

					// netfinity: Maximum Segment Size (MSS - Vista only) //added by zz_fly
					SocketSentBytes socketSentBytes;
					if(thePrefs.retrieveMTUFromSocket && (socket->m_dwMSS!=0) && (allowedDataRate >= 6*1024)){ //let MTU=536 at very low speed
						uint32 socketMSS = (thePrefs.usedoublesendsize && (allowedDataRate >= 16*1024)) ? ((socket->m_dwMSS - 40) * 2) : (socket->m_dwMSS - 40);
						socketSentBytes = socket->SendFileAndControlData(socketMSS,socketMSS); 
					}
					else {
						socketSentBytes = socket->SendFileAndControlData(doubleSendSize,doubleSendSize); 
					}
					// netfinity: end
					uint32 lastSpentBytes = socketSentBytes.sentBytesControlPackets + socketSentBytes.sentBytesStandardPackets;
					spentBytes += lastSpentBytes;
					spentOverhead += socketSentBytes.sentBytesControlPackets;
					uSlope-=lastSpentBytes;
					if(socketSentBytes.sentBytesStandardPackets>450 /*|| socketSentBytes.sentBytesControlPackets >0*//*)
					{
						if(pos!=NULL) //it's the last socket
						{
							m_StandardOrder_list_full.RemoveAt(cur_pos);
							m_StandardOrder_list_full.AddTail(socket);
						}
					}	
					else
						k++;
				}
				else
					k++;
			}
		}
		*/

		// first we give all slots trickle speed
		if(uSlope>0)
		{
			for(uint32 i = 0; i < (uint32)m_StandardOrder_list.GetSize(); i++)
			{
				ThrottledFileSocket* socket = m_StandardOrder_list.GetAt(i);
				if(socket!=NULL && socket->isready==true) 
				{
					if(timeSinceLastLoop==1000)
					{
						//application hang
						socket->CSlope=1;
					}
					else
					{
						socket->CSlope+=(TRICKLESPEED*timeSinceLastLoop/1000);
					}
					//compensate:
					if(socket->CSlope>(sint32)TRICKLESPEED*MAXSLOPEBUFFERTIME)
						socket->CSlope=TRICKLESPEED*MAXSLOPEBUFFERTIME;

					//theApp.QueueDebugLogLine(false,_T("UBT: CSlope: %i"),socket->CSlope);
					if(uSlope >0 && socket->CSlope>0)
					{
						//now we can send:
						SocketSentBytes socketSentBytes = socket->SendFileAndControlData(minFragSize, minFragSize); 
						uint32 lastSpentBytes = socketSentBytes.sentBytesControlPackets + socketSentBytes.sentBytesStandardPackets;
						spentBytes += lastSpentBytes;
						spentOverhead += socketSentBytes.sentBytesControlPackets;
						// ==> Multiple friendslots [ZZ] - Mephisto
						if(socket->GetFriendSlot())
							spentBytesFriends += socketSentBytes.sentBytesStandardPackets;
						// <== Multiple friendslots [ZZ] - Mephisto
						uSlope-=lastSpentBytes;
						socket->CSlope-=socketSentBytes.sentBytesStandardPackets;
						//theApp.QueueDebugLogLine(false,_T("UBT: Trickle spent: %u"),lastSpentBytes);
					}
				}
			}
		}

		//next we give maximum upload to any slot that can take it
		if(uSlope>0)
		{
			bHadTrickle = false; // reset the value
			uint32 uActiveSlotCountTemp=0;
			for(uint32 i = 0; i < (uint32)m_StandardOrder_list.GetSize(); i++)
			{
				ThrottledFileSocket* socket = m_StandardOrder_list.GetAt(i);

				if(socket==NULL) // no socket? we can do nothing here!
					continue;

				// count full slots
				if(socket->IsFull())
					uActiveSlotCountTemp++;

				FullTimes_struct FullTimes;
				FullTimes.dwTimeStamp = thisLoopTick;
				bool bFull = false;
				while(socket->m_FullTimes.IsEmpty() == false && // this shall prevent crashes
					(thisLoopTick-socket->m_FullTimes.GetTail().dwTimeStamp)>(DWORD)(thePrefs.GetMonitorLoops()*1000))
				{
					if(socket->m_FullTimes.GetTail().bWasFull)
						socket->m_uFullTimesCounter--;
					socket->m_FullTimes.RemoveTail();
				}
				if(socket->m_FullTimes.IsEmpty() == false)
				{
					if((((double)socket->m_uFullTimesCounter)/((double)socket->m_FullTimes.GetCount()))>=(((double)thePrefs.GetFullLoops())/100.0))
						bFull = true;
				}

				// set trickles, skip this loop
				if(uSlope<=0) // nothing we could give
				{
					if(socket->IsFull())
					{
						if(!bFull)
						{
							socket->SetTrickle(); // this is a trickle
							bHadTrickle = true;
							uActiveSlotCountTemp--; // we made a new trickle here
							// ==> Upload Debuging [Stulle] - Mephisto
#ifdef UPLOAD_DEBUGING
							AddDebugLogLine(true,_T("Slot %i: Set to trickle."),i);
#endif
							// <== Upload Debuging [Stulle] - Mephisto
						}
					}
					else
						bHadTrickle = true;

					FullTimes.bWasFull = false;
					socket->m_FullTimes.AddHead(FullTimes);
					continue; // too little data to give
				}

				// ==> Dynamic Socket Buffering [SiRoB] - Mephisto
#ifdef DONT_USE_SOCKET_BUFFERING
				// check if SendBuffer should be increased
				if (bAlwaysEnableBigSocketBuffers)
					socket->UseBigSendBuffer();
#endif
				// <== Dynamic Socket Buffering [SiRoB] - Mephisto

				// set the maximum to give
				//theApp.QueueDebugLogLine(false,_T("UBT: uSlope: %I64u"),uSlope);
				uint32 uTempSendSize = minFragSize;
				// netfinity: Maximum Segment Size (MSS - Vista only) //added by zz_fly
				/*
				if(uSlope>minFragSize)
					uTempSendSize = (uint32)(minFragSize*(uSlope/minFragSize+1);
				*/
				uint32 socketMSS = minFragSize;
				if(thePrefs.retrieveMTUFromSocket && (socket->m_dwMSS!=0) && (allowedDataRate >= 6*1024)) //let MTU=536 at very low speed
					socketMSS = socket->m_dwMSS - 40;
				if(uSlope>minFragSize)
					uTempSendSize = socketMSS*(uSlope/socketMSS+1);
				// netfinity: end
				//theApp.QueueDebugLogLine(false,_T("UBT: SlotFocus needed: %u"),uTempSendSize);

				// spend bandwith
				if(socket->isready==true) 
				{
					// we don't get here if the slots don't get a thing (uSlope<0)
					if(socket->IsTrickle() && bFull)
					{
						socket->SetFull(); // change state
						uActiveSlotCountTemp++; // new full slot
						// ==> Upload Debuging [Stulle] - Mephisto
#ifdef UPLOAD_DEBUGING
						AddDebugLogLine(true,_T("Slot %i: Set to full."),i);
#endif
						// <== Upload Debuging [Stulle] - Mephisto
					}
					FullTimes.bWasFull = true;
					socket->m_FullTimes.AddHead(FullTimes);
					socket->m_uFullTimesCounter++;

					SocketSentBytes socketSentBytes = socket->SendFileAndControlData(uTempSendSize,minFragSize); 
					uint32 lastSpentBytes = socketSentBytes.sentBytesControlPackets + socketSentBytes.sentBytesStandardPackets;
					spentBytes += lastSpentBytes;
					spentOverhead += socketSentBytes.sentBytesControlPackets;
					// ==> Multiple friendslots [ZZ] - Mephisto
					if(socket->GetFriendSlot())
						spentBytesFriends += socketSentBytes.sentBytesStandardPackets;
					// <== Multiple friendslots [ZZ] - Mephisto

					// ==> Upload Debuging [Stulle] - Mephisto
#ifdef UPLOAD_DEBUGING
					socket->m_uFullBytesCount += lastSpentBytes;
					socket->m_uAllFullTimesCounter++;
#endif
					// <== Upload Debuging [Stulle] - Mephisto

					uSlope-=lastSpentBytes;
					//theApp.QueueDebugLogLine(false,_T("UBT: SlotFocus spent: %u"),lastSpentBytes);
				}
			}
			m_uActiveSlotCount=uActiveSlotCountTemp;
		}
		// <== Mephisto Upload - Mephisto

		//Xman count block/success send
		if(thisLoopTick >> 10 > last_block_process)
		{
			last_block_process = thisLoopTick >> 10;
			float tmpavgblockratio=0;
			for(int i=0; i < m_StandardOrder_list.GetSize(); i++)
			{
				ThrottledFileSocket* socket = m_StandardOrder_list.GetAt(i);
				if(socket!=NULL ) 
				{
					// ==> Mephisto Upload - Mephisto
					/*
					if(socket->IsFull())
						tmpavgblockratio +=socket->GetandStepBlockRatio();
					else
						socket->GetandStepBlockRatio();
					*/
					tmpavgblockratio +=socket->GetandStepBlockRatio();
					// <== Mephisto Upload - Mephisto
				}
			}
			// ==> Mephisto Upload - Mephisto
			/*
			avgBlockRatio = m_StandardOrder_list_full.GetCount() > 0 ? tmpavgblockratio/m_StandardOrder_list_full.GetCount() : 0;
			*/
			avgBlockRatio = m_StandardOrder_list.GetCount() > 0 ? tmpavgblockratio/m_StandardOrder_list.GetCount() : 0;
			// <== Mephisto Upload - Mephisto
		}

		//Xman upload health
		if(minslots <= (uint16)m_StandardOrder_list.GetSize())
		{
			m_countsend++;
			if(uSlope <= 0)
				m_countsendsuccessful++;
			if(m_healthhistory.GetSize()==0 || thisLoopTick >> 10 > m_healthhistory.GetHead().timestamp)
			{
				ratio_struct newsample;
				newsample.ratio = 100.0f*m_countsendsuccessful/m_countsend; //m_countsend is always greater 0
				newsample.timestamp = thisLoopTick >> 10; // 1024 ms
				m_healthhistory.AddHead(newsample);
				sum_healthhistory += newsample.ratio;
				if(m_healthhistory.GetSize()>HISTORY_SIZE) // ~ 20 seconds
				{
					const ratio_struct& substract = m_healthhistory.RemoveTail(); //susbtract the old element
					sum_healthhistory -= substract.ratio;
					if(sum_healthhistory<0) //fix rounding errors
						sum_healthhistory=0;
				}
				avg_health = sum_healthhistory / m_healthhistory.GetSize();
				m_countsend=0;
				m_countsendsuccessful=0;
			}
		}
		//Xman end

		// ==> Mephisto Upload - Mephisto
		/*
		uint16 countTrickles=(uint16)m_StandardOrder_list.GetSize()-m_highestNumberOfFullyActivatedSlots;

		//any data left ? this data is given to the trickles
		//if(uSlope>allowedDataRate*MAXSLOPEBUFFERTIME/6 && countTrickles>0) 
		if(uSlope>0 && countTrickles>0) 
		{
			uint16 i=0;
			for(i=m_highestNumberOfFullyActivatedSlots;i<m_StandardOrder_list.GetSize();i++)
			{
		        if(rememberedSlotCounterTrickle >= m_StandardOrder_list.GetSize()) 
			    {
				    rememberedSlotCounterTrickle = m_highestNumberOfFullyActivatedSlots;
				}
				ThrottledFileSocket* socket = m_StandardOrder_list.GetAt(rememberedSlotCounterTrickle);
				if(socket!=NULL  && socket->isready==true) 
				{
					if(uSlope>0)
					{
							SocketSentBytes socketSentBytes = socket->SendFileAndControlData(minFragSize, minFragSize); 
						    uint32 lastSpentBytes = socketSentBytes.sentBytesControlPackets + socketSentBytes.sentBytesStandardPackets;
						    spentBytes += lastSpentBytes;
						    spentOverhead += socketSentBytes.sentBytesControlPackets;
							uSlope-=lastSpentBytes;
							socket->CSlope-=socketSentBytes.sentBytesStandardPackets;
							if (lastSpentBytes>0 && socket->CSlope<-(sint32)minFragSize)
							{
								socket->CSlope=-(sint32)minFragSize; //otherwise we send to much and trickle could timeout
							}
					}
					else
						break;
				}
				rememberedSlotCounterTrickle++;
			}
		}
	
  
		//Xman x4 
		if(uSlope<TRICKLESPEED)
			lastTickReachedBandwidth=thisLoopTick;
		else
			if(thePrefs.m_bandwidthnotreachedslots==true && thisLoopTick - lastTickReachedBandwidth  > 3000) //since 3 seconds bandwidth couldn't be reached
			{
				needslot=true;
				lastTickReachedBandwidth=thisLoopTick;
			}
		*/
		//Xman x4 
		if(uSlope<500)
		{
			lastTickReachedBandwidth=thisLoopTick;
			m_bBandwithReached=true;
		}
		else
		{
			if((int)(thisLoopTick - lastTickReachedBandwidth)  > (int)SEC2MS(thePrefs.GetNotReachedBW())) //since x seconds bandwidth couldn't be reached
			{
				needslot|=1;
				lastTickReachedBandwidth=thisLoopTick;
			}
			m_bBandwithReached=false;
		}

		if(bHadTrickle)
			lastTickHadTrickle=thisLoopTick;
		// we had no trickle within a configured timespan
		// thisLoopTick - lastTickHadTrickle will result in >=0
		// thus will a setting 0 prevent a new slot from being opened
		else
		{
			uint8 uNoTrickleTimer = thePrefs.GetNoTrickleTimer();
			if(uNoTrickleTimer > 0 && (int)(thisLoopTick - lastTickHadTrickle)  > (int)SEC2MS(uNoTrickleTimer))
			{
				needslot|=2;
				lastTickHadTrickle=thisLoopTick;
			}
		}
		// <== Mephisto Upload - Mephisto

		//Xman upload health
		//remark: we do this calculation above if we have enough sockets
		//if we have less sockets we do it here
		if(minslots > (uint16)m_StandardOrder_list.GetSize())
		{
			m_countsend++;
			if(uSlope <= 0)
				m_countsendsuccessful++;
			if(m_healthhistory.GetSize()==0 || thisLoopTick >> 10 > m_healthhistory.GetHead().timestamp)
			{
				ratio_struct newsample;
				newsample.ratio = 100.0f*m_countsendsuccessful/m_countsend; //m_countsend is always greater 0
				newsample.timestamp = thisLoopTick >> 10; // 1024 ms
				m_healthhistory.AddHead(newsample);
				sum_healthhistory += newsample.ratio;
				if(m_healthhistory.GetSize()>HISTORY_SIZE) // ~ 20 seconds
				{
					const ratio_struct& substract = m_healthhistory.RemoveTail(); //susbtract the old element
					sum_healthhistory -= substract.ratio;
				}
				avg_health = sum_healthhistory / m_healthhistory.GetSize();
				m_countsend=0;
				m_countsendsuccessful=0;
			}
		}
		//Xman end



		// - Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
		theApp.pBandWidthControl->AddeMuleOutOverallNoHeader(spentBytes);
		theApp.pBandWidthControl->AddeMuleOut(spentBytes-spentOverhead);
		// ==> Multiple friendslots [ZZ] - Mephisto
		theApp.pBandWidthControl->AddeMuleOutFriends(spentBytesFriends);
		// <== Multiple friendslots [ZZ] - Mephisto


        sendLocker.Unlock();

	//end of the main loop
	}

	threadEndedEvent->SetEvent();

	tempQueueLocker.Lock();
	m_TempControlQueue_list.RemoveAll();
	m_TempControlQueueFirst_list.RemoveAll();
	tempQueueLocker.Unlock();

	sendLocker.Lock();

	m_ControlQueue_list.RemoveAll();
	m_StandardOrder_list.RemoveAll();
	sendLocker.Unlock();

	return 0;
}
