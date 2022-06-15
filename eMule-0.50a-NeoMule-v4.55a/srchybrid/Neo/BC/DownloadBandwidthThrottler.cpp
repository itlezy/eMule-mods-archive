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
#include "emule.h"
#include "DownloadBandwidthThrottler.h"
#include "preferences.h"
#include "updownclient.h"
#include "OtherFunctions.h"
#include "Listensocket.h"
#include "downloadqueue.h"
#include "PartFile.h"
#include "BandwidthControl.h"
#include "ThrottlerHelpers.h"
#include "Log.h"
#include "Neo/NeoPreferences.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->

/**
* The constructor starts the thread.
*/
DownloadBandwidthThrottler::DownloadBandwidthThrottler(void) {

	threadEndedEvent = new CEvent(0, 1);
	pauseEvent = new CEvent(TRUE, TRUE);

	doRun = true;
	pThread = AfxBeginThread(RunProc, (LPVOID)this, NeoPrefs.GetBCPriorityDown(), 0, CREATE_SUSPENDED);
}

/**
* The destructor stops the thread. If the thread has already stoppped, destructor does nothing.
*/
DownloadBandwidthThrottler::~DownloadBandwidthThrottler(void) {
	//EndThread();
	delete threadEndedEvent;
	delete pauseEvent;
}

void DownloadBandwidthThrottler::AddToStandardList(ThrottledFileSocket* socket) {
	if (socket) {
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
		if(socket->IsLanSocket())
			return;
#endif //LANCAST // NEO: NLC END
		receiveLocker.Lock();
		AddToStandardListNoLock(socket);
		receiveLocker.Unlock();
	}
}

void DownloadBandwidthThrottler::AddToStandardListNoLock(ThrottledFileSocket* socket) {
	if (socket && !socket->onDownQueue){
		m_DownloadQueue_list.AddTail(socket);
		socket->onDownQueue = true;
	}
}

void DownloadBandwidthThrottler::RemoveFromStandardList(ThrottledFileSocket* socket) {
	if (doRun){
		receiveLocker.Lock();

		RemoveFromStandardListNoLock(socket);

		if(socket && !socket->IsEmpty() && !socket->onDownControlQueue && NeoPrefs.UseDownloadBandwidthThrottler()){ // socket removed but still packets waiting
			socket->onDownControlQueue = true;
			m_ControlQueue_list.AddHead(socket);
		}

		receiveLocker.Unlock();
	}
}

void DownloadBandwidthThrottler::RemoveFromStandardListNoLock(ThrottledFileSocket* socket) {
	if (socket && socket->onDownQueue) {
		// Find the slot
		POSITION pos = m_DownloadQueue_list.Find(socket);
		if (pos){
			socket->onDownQueue = false;
			m_DownloadQueue_list.RemoveAt(pos);
		}
	}
}

bool DownloadBandwidthThrottler::QueueForReceivingPacket(ThrottledControlSocket* socket) {
	if (socket){
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
		if(socket->IsLanSocket())
			return true;
#endif //LANCAST // NEO: NLC END

		// Get critical section
		receiveLocker.Lock();
		if(doRun && socket && !socket->onDownControlQueue && !socket->onDownQueue) {
			socket->onDownControlQueue = true;
			m_ControlQueue_list.AddTail(socket);
		}

		bool ret = socket->onDownControlQueue || socket->onDownQueue;
		// End critical section
		receiveLocker.Unlock();
		return ret;
	}
	return false;
}

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
bool DownloadBandwidthThrottler::QueueForLanPacket(ThrottledFileSocket* socket) {
	if (socket){
		// Get critical section
		receiveLocker.Lock();
		if (doRun && socket && !socket->onDownLanQueue){
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
			if(socket->IsVoodooSocket())
				m_LanQueue_list.AddHead(socket);
			else
#endif // VOODOO // NEO: VOODOO END
				m_LanQueue_list.AddTail(socket);
			socket->onDownLanQueue = true;
		}

		bool ret = socket->onDownLanQueue;
		// End critical section
		receiveLocker.Unlock();
		return ret;
	}
	return false;
}

void DownloadBandwidthThrottler::RemoveFromLanPacketQueueNoLock(ThrottledFileSocket* socket){
	if(socket && socket->onDownLanQueue){
		POSITION pos = m_LanQueue_list.Find(socket);
		if (pos){
			socket->onDownControlQueue = false;
			m_LanQueue_list.RemoveAt(pos);
		}
	}
}
#endif //LANCAST // NEO: NLC END

void DownloadBandwidthThrottler::RemoveFromAllQueuesNoLock(ThrottledControlSocket* socket) {
	if (socket && socket->onDownControlQueue){
		// Remove this socket from control packet queue
		POSITION pos = m_ControlQueue_list.Find(socket);
		if (pos){
			socket->onDownControlQueue = false;
			m_ControlQueue_list.RemoveAt(pos);
		}
	}
}

void DownloadBandwidthThrottler::RemoveFromAllQueues(ThrottledFileSocket* socket) {
	if (doRun){
		// Get critical section
		receiveLocker.Lock();

		RemoveFromAllQueuesNoLock(socket);
		RemoveFromStandardListNoLock(socket);
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
		RemoveFromLanPacketQueueNoLock(socket);
#endif //LANCAST

		// End critical section
		receiveLocker.Unlock();
	}
}

void DownloadBandwidthThrottler::RemoveFromAllQueues(ThrottledControlSocket* socket) {
	if (socket){
		// Get critical section
		receiveLocker.Lock();

		if(doRun) {
			RemoveFromAllQueuesNoLock(socket);
		}

		// End critical section
		receiveLocker.Unlock();
	}
}

/**
* Make the thread exit. This method will not return until the thread has stopped
* looping. This guarantees that the thread will not access the CEMSockets after this
* call has exited.
*/
void DownloadBandwidthThrottler::EndThread() {
	receiveLocker.Lock();

	// signal the thread to stop looping and exit.
	doRun = false;

	receiveLocker.Unlock();

	Pause(false);

	// wait for the thread to signal that it has stopped looping.
	threadEndedEvent->Lock();
}

void DownloadBandwidthThrottler::StartThread() {
	pThread->ResumeThread();
}

void DownloadBandwidthThrottler::Pause(bool paused) {
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
UINT AFX_CDECL DownloadBandwidthThrottler::RunProc(LPVOID pParam) {
	DbgSetThreadName("DownloadBandwidthThrottler");
	InitThreadLocale();
	// NEO: STS - [SlugFillerThreadSafe] -- Xanatos -->
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return 0;
	// NEO: STS END <-- Xanatos --
	DownloadBandwidthThrottler* downloadBandwidthThrottler = (DownloadBandwidthThrottler*)pParam;

	return downloadBandwidthThrottler->RunInternal();
}

/**
* @return always returns 0.
*/
UINT DownloadBandwidthThrottler::RunInternal() {
	DWORD lastLoopTick = timeGetTime();
	float allowedDataRate = 0;
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	float allowedLanDataRate = 0;
#endif //LANCAST // NEO: NLC END

	toReceive = 0;
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	toLanReceive = 0;
#endif //LANCAST // NEO: NLC END

	while(doRun) {
		pauseEvent->Lock();

		if(!NeoPrefs.UseDownloadBandwidthThrottler()){
			Sleep(1000);
			continue;
		}

		DWORD timeSinceLastLoop = timeGetTime() - lastLoopTick;

		// Get current speed from UploadSpeedSense
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
		if(theApp.bandwidthControl == NULL) // not ready yet :/
			continue;

		allowedDataRate = theApp.bandwidthControl->GetMaxDownload();
#else
		allowedDataRate = thePrefs.GetMaxDownload();
#endif // NEO_BC // NEO: NBC END

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
		allowedLanDataRate = NeoPrefs.GetMaxLanDownload();
#endif //LANCAST // NEO: NLC END

		//uint32 minFragSize // the serding sinde take care abotu this

		DelayLoopTime(NeoPrefs.GetBCTimeDown(),allowedDataRate,toReceive,timeSinceLastLoop,theApp.bandwidthControl->GetMSS());

		const DWORD thisLoopTick = timeGetTime();
		timeSinceLastLoop = thisLoopTick - lastLoopTick;
		lastLoopTick = thisLoopTick;

		CalcLoopAddOn(allowedDataRate,toReceive,timeSinceLastLoop);

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
		bool bLanLanCast = NeoPrefs.IsLanSupportEnabled();
		if(bLanLanCast)
			CalcLoopAddOn(allowedLanDataRate,toLanReceive,timeSinceLastLoop);
#endif //LANCAST // NEO: NLC END

		bool includeoverhead = NeoPrefs.IsIncludeOverhead();
		UINT hyperreceiving = NeoPrefs.UseHyperDownload();

		receiveLocker.Lock();

		//Serve control packets
		POSITION controlPos = m_ControlQueue_list.GetHeadPosition();
		for(UINT i=m_ControlQueue_list.GetCount();i>0;i--){
			POSITION prev_pos = controlPos;
			ThrottledControlSocket* cur_socket = m_ControlQueue_list.GetNext(controlPos);

			if(cur_socket->onDownQueue){ // This is empty or is not longer a control socket, remove it
				cur_socket->onDownControlQueue = false;
				m_ControlQueue_list.RemoveAt(prev_pos);
				continue;
			}

			int Received;
			if (cur_socket->IsPriorityReceive() || !includeoverhead)
				Received = cur_socket->Receive(0x7fffffff,hyperreceiving);
			else if (toReceive >= (int) theApp.bandwidthControl->GetMSS()/2)
				Received = cur_socket->Receive(toReceive,hyperreceiving);
			else
				continue; // we wont break now, we will lopp throu all queued sockets, there may be some priority sockets

			if(includeoverhead)
				toReceive -= Received;

			if(cur_socket->IsEmpty() && hyperreceiving != TRUE){ // This is empty remove it
				cur_socket->onDownControlQueue = false;
				m_ControlQueue_list.RemoveAt(prev_pos);
			}
			else if(cur_socket->isUDP()){ // we used up our bandwidth there are still packets left, when it is a UDP socket
				m_ControlQueue_list.RemoveAt(prev_pos); // make space for TCP sockets
				m_ControlQueue_list.AddTail(cur_socket); // and move the UDP spcket on the bottom of the list
			} 
			/*else {}*/ // we keep tcp sockets on top to complete incomplete packets
		}

		//Serve file packets
		POSITION downloadPos = m_DownloadQueue_list.GetHeadPosition();
		for(UINT i=m_DownloadQueue_list.GetCount();i>0 && toReceive > 0;i--){
			POSITION prev_pos = downloadPos;
			ThrottledFileSocket* cur_socket = m_DownloadQueue_list.GetNext(downloadPos);
			if(cur_socket->IsEmpty() && hyperreceiving == FALSE)
				continue;

			int Received = cur_socket->Receive(toReceive,hyperreceiving);
 
			if(Received >= (int) theApp.bandwidthControl->GetMSS()/3){ // we resived enough shift socket to tail
				m_DownloadQueue_list.RemoveAt(prev_pos);
				m_DownloadQueue_list.AddTail(cur_socket);
			}

			toReceive -= Received;
		}


#ifdef LANCAST // NEO: NLC - [NeoLanCast]
		//Serve the LAN packets
		if(bLanLanCast){
			POSITION lanPos = m_LanQueue_list.GetHeadPosition();
			for(UINT i=m_LanQueue_list.GetCount();i>0;i--){
				POSITION prev_pos = lanPos;
				ThrottledFileSocket* cur_socket = m_LanQueue_list.GetNext(lanPos);
				if(cur_socket->IsEmpty() && hyperreceiving == FALSE)
					continue;

				int Received;
 #ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
				if(cur_socket->IsVoodooSocket()){ // No limit for voodoo sockets, and keep them on top
					Received = cur_socket->Receive(0x7fffffff,hyperreceiving);
				}else
 #endif // VOODOO // NEO: VOODOO END
				if(toLanReceive > 0) {
					Received = cur_socket->Receive(toLanReceive,hyperreceiving);

					if(Received >= (int) theApp.bandwidthControl->GetMSS()/2){ // we resived enough shift socket to tail
						m_LanQueue_list.RemoveAt(prev_pos);
						m_LanQueue_list.AddTail(cur_socket);
					}
				}else
					break;

				toLanReceive -= Received;
			}
		}
#endif //LANCAST // NEO: NLC END
		receiveLocker.Unlock();
	}

	threadEndedEvent->SetEvent();

	receiveLocker.Lock();

	m_ControlQueue_list.RemoveAll();
	m_DownloadQueue_list.RemoveAll();
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	m_LanQueue_list.RemoveAll();
#endif //LANCAST // NEO: NLC END

	receiveLocker.Unlock();

	return 0;
}

void DownloadBandwidthThrottler::ReSortDownloadSlots(CUpDownClient* client){
	if (doRun && client){
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
		if(client->IsLanClient())
			return;
#endif //LANCAST // NEO: NLC END

		receiveLocker.Lock();
		// Remove the found Client from UploadBandwidthThrottler
		RemoveFromStandardListNoLock(client->socket);
		RemoveFromStandardListNoLock((CClientReqSocket*) client->m_pPCDownSocket);
		AddToStandardListNoLock(client->GetFileDownloadSocket());
		receiveLocker.Unlock();
	}
}

// when we dissable DBT on runtime we have to recive all remainding data and clear the queues
// this function *_must_* be calles from the !_main_! thread !!!
void DownloadBandwidthThrottler::ClearQueues()
{
	receiveLocker.Lock();

	POSITION controlPos = m_ControlQueue_list.GetHeadPosition();
	for(UINT i=m_ControlQueue_list.GetCount();i>0;i--){
		POSITION prev_pos = controlPos;
		ThrottledControlSocket* cur_socket = m_ControlQueue_list.GetNext(controlPos);
		m_ControlQueue_list.RemoveAt(prev_pos);

		cur_socket->Receive(0x7fffffff);
		cur_socket->ProcessData(true);
	}

	POSITION downloadPos = m_DownloadQueue_list.GetHeadPosition();
	for(UINT i=m_DownloadQueue_list.GetCount();i>0 && toReceive > 0;i--){
		POSITION prev_pos = downloadPos;
		ThrottledFileSocket* cur_socket = m_DownloadQueue_list.GetNext(downloadPos);
		m_DownloadQueue_list.RemoveAt(prev_pos);
		
		cur_socket->Receive(0x7fffffff);
		cur_socket->ProcessData(true);
	}

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	POSITION lanPos = m_LanQueue_list.GetHeadPosition();
	for(UINT i=m_LanQueue_list.GetCount();i>0;i--){
		POSITION prev_pos = lanPos;
		ThrottledFileSocket* cur_socket = m_LanQueue_list.GetNext(lanPos);
		m_LanQueue_list.RemoveAt(prev_pos);

		cur_socket->Receive(0x7fffffff);
		cur_socket->ProcessData(true);
	}
#endif //LANCAST // NEO: NLC END
	receiveLocker.Unlock();
}

#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --