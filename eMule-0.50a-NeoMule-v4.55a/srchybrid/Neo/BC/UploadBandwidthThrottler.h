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

#pragma once

#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->

#include "ThrottledSocket.h"

class UploadBandwidthThrottler :
    public CWinThread 
{
public:
    UploadBandwidthThrottler(void);
    ~UploadBandwidthThrottler(void);

    bool QueueForSendingControlPacket(ThrottledControlSocket* socket, bool hasSent = false);
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	bool QueueForLanPacket(ThrottledFileSocket* socket);
#endif //LANCAST // NEO: NLC END
    void RemoveFromAllQueues(ThrottledControlSocket* socket);
	void RemoveFromAllQueues(ThrottledFileSocket* socket);

	//uint64 GetSentBytes(); // removed // NEO: ASM - [AccurateSpeedMeasure]

    void StartThread();
	void EndThread();

    void Pause(bool paused);

	void AddToStandardList(ThrottledFileSocket* socket, bool bFull = false);
    bool RemoveFromStandardList(ThrottledFileSocket* socket);

	bool SetNextTrickleToFull(bool lock = true, UINT count = 1, bool reset = false);
	void SetTrickleToFull(ThrottledFileSocket* socket, bool lock = true);
	void SetFullToTrickle(ThrottledFileSocket* socket, bool lock = true, bool block = false);

	//bool AddToBlockedQueue(ThrottledControlSocket* socket);
	void DecreaseToSend(const int& bytes) {toSend -= bytes;}
	void ReSortUploadSlots(CUpDownClient* client);

	bool ForceNewClient();
	void SetNoNeedSlot();

	UINT GetNumberOfFullyActivatedSlots();

	float GetEstiminatedLimit() { return m_nEstiminatedLimit; }

	ThrottledFileSocket* GetPrioritySocket();

protected:
	friend class CPPgBandwidth;

	CWinThread* pThread;

private:
    static UINT RunProc(LPVOID pParam);
    UINT RunInternal();

	void AddToStandardListNoLock(ThrottledFileSocket* socket, bool bFull = false);

    void RemoveFromAllQueuesNoLock(ThrottledControlSocket* socket);
    bool RemoveFromStandardListNoLock(ThrottledFileSocket* socket);
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	void RemoveFromLanPacketQueueNoLock(ThrottledFileSocket* socket);
#endif //LANCAST // NEO: NLC END

    CCriticalSection sendLocker; 
    //CCriticalSection dataLocker;

    CEvent* threadEndedEvent;
    CEvent* pauseEvent;

    bool doRun;

	int     toSend;
 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
	int     toSendModerated[ST_NORMAL];
 #endif // BW_MOD // NEO: BM END
    CTypedPtrList<CPtrList, ThrottledControlSocket*> m_ControlQueue_list;
    CTypedPtrList<CPtrList, ThrottledControlSocket*> m_BlockedQueue_list;
    CTypedPtrList<CPtrList, ThrottledFileSocket*> m_UploadQueue_list;
	CTypedPtrList<CPtrList, ThrottledFileSocket*> m_UploadQueue_list_full;

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	int     toLanSend;
    CTypedPtrList<CPtrList, ThrottledFileSocket*> m_LanQueue_list;
#endif //LANCAST // NEO: NLC END

	uint8	recalculate; // we have 2 different operation modis
	bool	needslot;
	bool	nextslotfull;

	//uint64	m_SentBytesSinceLastCall; // removed // NEO: ASM - [AccurateSpeedMeasure]

	float	m_nEstiminatedLimit;
};

#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
