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

class UploadBandwidthThrottler :
    public CWinThread 
{
public:
    UploadBandwidthThrottler(void);
    ~UploadBandwidthThrottler(void);

    //Xman Xtreme Upload unused
	//uint64 GetNumberOfSentBytesSinceLastCallAndReset();
    //uint64 GetNumberOfSentBytesOverheadSinceLastCallAndReset();
	//uint32 GetHighestNumberOfFullyActivatedSlotsSinceLastCallAndReset();

    uint32 GetStandardListSize() { return m_StandardOrder_list.GetSize(); };

	//void ReplaceSocket(ThrottledFileSocket* oldsocket, ThrottledFileSocket* newsocket); //Xman Xtreme Upload: Peercache-part
	bool ReplaceSocket(ThrottledFileSocket* normalsocket, ThrottledFileSocket* pcsocket, ThrottledFileSocket* newsocket); //Xman Xtreme Upload: Peercache-part
	void AddToStandardList(bool first, ThrottledFileSocket* socket); //Xman bugfix: sometimes a socket was placed on wrong position
    bool RemoveFromStandardList(ThrottledFileSocket* socket);

    void QueueForSendingControlPacket(ThrottledControlSocket* socket, bool hasSent = false); // ZZ:UploadBandWithThrottler (UDP)
    void RemoveFromAllQueues(ThrottledControlSocket* socket) { RemoveFromAllQueues(socket, true); }; // ZZ:UploadBandWithThrottler (UDP)
    void RemoveFromAllQueues(ThrottledFileSocket* socket);

    void EndThread();

    void Pause(bool paused);

	//Xman Xtreme Upload
	void	SetNoNeedSlot();
	uint16	GetNumberOfFullyActivatedSlots()	{return m_highestNumberOfFullyActivatedSlots_out;}
	void	SetNumberOfFullyActivatedSlots()	{m_highestNumberOfFullyActivatedSlots_out=m_highestNumberOfFullyActivatedSlots;}
	void	SetNextTrickleToFull();
	void	RecalculateOnNextLoop();
	bool	needslot;

	//Xman count block/success send
	float	GetAvgBlockRatio() const				{return avgBlockRatio;}
	//Xman upload health
	float	GetAvgHealth() const					{return avg_health;}

#ifdef PRINT_STATISTIC
	void	PrintStatistic();
#endif

private:
    static UINT RunProc(LPVOID pParam);
    UINT RunInternal();

    void RemoveFromAllQueues(ThrottledControlSocket* socket, bool lock); // ZZ:UploadBandWithThrottler (UDP)
    bool RemoveFromStandardListNoLock(ThrottledFileSocket* socket);

    CTypedPtrList<CPtrList, ThrottledControlSocket*> m_ControlQueue_list; // a queue for all the sockets that want to have Send() called on them. // ZZ:UploadBandWithThrottler (UDP)
    CTypedPtrList<CPtrList, ThrottledControlSocket*> m_ControlQueueFirst_list; // a queue for all the sockets that want to have Send() called on them. // ZZ:UploadBandWithThrottler (UDP)
    CTypedPtrList<CPtrList, ThrottledControlSocket*> m_TempControlQueue_list; // sockets that wants to enter m_ControlQueue_list // ZZ:UploadBandWithThrottler (UDP)
    CTypedPtrList<CPtrList, ThrottledControlSocket*> m_TempControlQueueFirst_list; // sockets that wants to enter m_ControlQueue_list and has been able to send before // ZZ:UploadBandWithThrottler (UDP)

    CArray<ThrottledFileSocket*, ThrottledFileSocket*> m_StandardOrder_list; // sockets that have upload slots. Ordered so the most prioritized socket is first

	CTypedPtrList<CPtrList, ThrottledFileSocket*> m_StandardOrder_list_full; //Xman Xtreme Upload

    CCriticalSection sendLocker;
    CCriticalSection tempQueueLocker;

    CEvent* threadEndedEvent;
    CEvent* pauseEvent;

	//Xman Xtreme Upload unused
    //uint64 m_SentBytesSinceLastCall;
    //uint64 m_SentBytesSinceLastCallOverhead;
    uint16 m_highestNumberOfFullyActivatedSlots; //used inside
    volatile uint16 m_highestNumberOfFullyActivatedSlots_out; //used outside
	bool doRun;

	//Xman Xtreme Upload
	bool	recalculate;
	bool	nexttrickletofull;

	//Xman count block/success send
	float	avgBlockRatio;
	//Xman upload health

	struct ratio_struct{
		float ratio; // % successful upload loops
		uint32 timestamp; // time in 1024 ms units
	};
	//Xman end

	typedef CList<ratio_struct> HealthHistory;
	HealthHistory m_healthhistory;
	float avg_health; //the average health of last 10 seconds
	float sum_healthhistory; //the sum of all stored ratio samples
	uint16 m_countsend; //count the sends during ~one second
	uint16 m_countsendsuccessful; // " successful
};
