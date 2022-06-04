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
#pragma once

#include "ThrottledSocket.h" // ZZ:UploadBandWithThrottler (UDP)

//MORPH START - Added by SiRoB & AndCycle, Upload Splitting Class
#define NB_SPLITTING_CLASS 3
#define LAST_CLASS NB_SPLITTING_CLASS-1
#define NB_SCHED_CLASS NB_SPLITTING_CLASS

struct Socket_stat{
	uint32	classID;
	sint64	realBytesToSpend;
	DWORD	dwLastBusySince;
	DWORD	dwLastBytesSent;
	bool	scheduled;
};
//MORPH END - Added by SiRoB & AndCycle, Upload Splitting Class


class UploadBandwidthThrottler :
    public CWinThread 
{
public:
    UploadBandwidthThrottler(void);
    ~UploadBandwidthThrottler(void);

    void GetStats(uint64* SentBytes, uint64* SentBytesOverhead, uint32* HighestNumberOfFullyActivatedSlotsSinceLastCallClass);

	// MORPH count tcp overhead from download
	void SetDownDataOverheadOtherPackets(long bytes)	;
	uint64	GetDownDataOverheadOtherPackets_andreset()	;
	// END count


    //MORPH - Removed by SiRoB, See above (avoid lock delay by splitting call)
	/*
	uint64 GetNumberOfSentBytesOverheadSinceLastCallAndReset(uint64* sentBytesOverhead);
    uint32 GetHighestNumberOfFullyActivatedSlotsSinceLastCallAndReset();
	*/
	//MORPH END   - Changed by SiRoB, Upload Splitting Class
    uint32 GetStandardListSize() { return m_StandardOrder_list.GetSize(); };

	//MORPH START - Changed by SiRoB, Upload Splitting Class
    /*
	void AddToStandardList(uint32 index, ThrottledFileSocket* socket);
	*/
	void AddToStandardList(uint32 index, ThrottledFileSocket* socket, uint32 classID, bool scheduled);
	//MORPH END   - Changed by SiRoB, Upload Splitting Class
	bool RemoveFromStandardList(ThrottledFileSocket* socket, bool resort = false); //MORPH - Changed by SiRoB & AndCycle, Upload Splitting Class

    void QueueForSendingControlPacket(ThrottledControlSocket* socket, bool hasSent = false); // ZZ:UploadBandWithThrottler (UDP)
    void RemoveFromAllQueues(ThrottledControlSocket* socket) { RemoveFromAllQueues(socket, true); }; // ZZ:UploadBandWithThrottler (UDP)
    void RemoveFromAllQueues(ThrottledFileSocket* socket);

    void EndThread();

    void Pause(bool paused);

	static uint32 UploadBandwidthThrottler::GetSlotLimit(uint32 currentUpSpeed);

    void SignalNoLongerBusy();
private:
    static UINT RunProc(LPVOID pParam);
    UINT RunInternal();

    void RemoveFromAllQueues(ThrottledControlSocket* socket, bool lock); // ZZ:UploadBandWithThrottler (UDP)
	bool RemoveFromStandardListNoLock(ThrottledFileSocket* socket, bool resort = false); //MORPH - Changed by SiRoB & AndCycle, Upload Splitting Class
    
    uint32 CalculateChangeDelta(uint32 numberOfConsecutiveChanges) const;

	CTypedPtrList<CPtrList, ThrottledControlSocket*> m_ControlQueue_list; // a queue for all the sockets that want to have Send() called on them. // ZZ:UploadBandWithThrottler (UDP)
    CTypedPtrList<CPtrList, ThrottledControlSocket*> m_ControlQueueFirst_list; // a queue for all the sockets that want to have Send() called on them. // ZZ:UploadBandWithThrottler (UDP)
    CTypedPtrList<CPtrList, ThrottledControlSocket*> m_TempControlQueue_list; // sockets that wants to enter m_ControlQueue_list // ZZ:UploadBandWithThrottler (UDP)
    CTypedPtrList<CPtrList, ThrottledControlSocket*> m_TempControlQueueFirst_list; // sockets that wants to enter m_ControlQueue_list and has been able to send before // ZZ:UploadBandWithThrottler (UDP)

    CArray<ThrottledFileSocket*, ThrottledFileSocket*> m_StandardOrder_list; // sockets that have upload slots. Ordered so the most prioritized socket is first
    //MORPH START - Added by SiRoB & AndCycle, Upload Splitting Class
	CMap<ThrottledControlSocket*, ThrottledControlSocket*, Socket_stat*, Socket_stat*> m_stat_list;
	//MORPH END - Added by SiRoB & AndCycle, Upload Splitting Class

	
	CCriticalSection sendLocker;
    CCriticalSection tempQueueLocker;
    CCriticalSection sendBytesLocker;

    CEvent* threadEndedEvent;
    CEvent* pauseEvent;

    uint64 m_SentBytesSinceLastCallClass[NB_SPLITTING_CLASS];
    uint64 m_SentBytesSinceLastCallOverheadClass[NB_SPLITTING_CLASS];
    uint32 m_highestNumberOfFullyActivatedSlotsClass[NB_SPLITTING_CLASS];
	uint32 slotCounterClass[NB_SPLITTING_CLASS];
	bool doRun;
	// MORPH START
	uint64	m_nUpDataOverheadFromDownload;
	// MOPRH END


    CEvent busyEvent;
};
