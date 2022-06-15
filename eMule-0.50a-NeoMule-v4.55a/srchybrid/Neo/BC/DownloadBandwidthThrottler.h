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

#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->

#include "ThrottledSocket.h"

class CPartFile;

class DownloadBandwidthThrottler :
    public CWinThread 
{
public:
    DownloadBandwidthThrottler(void);
    ~DownloadBandwidthThrottler(void);

    bool QueueForReceivingPacket(ThrottledControlSocket* socket);
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	bool QueueForLanPacket(ThrottledFileSocket* socket);
#endif //LANCAST // NEO: NLC END
    void RemoveFromAllQueues(ThrottledControlSocket* socket);
	void RemoveFromAllQueues(ThrottledFileSocket* socket);

	void StartThread();
    void EndThread();

    void Pause(bool paused);

	void AddToStandardList(ThrottledFileSocket* socket);
	void AddToStandardListNoLock(ThrottledFileSocket* socket);
    void RemoveFromStandardList(ThrottledFileSocket* socket);

	void ClearQueues();
	void DecreaseToReceive(const int& bytes) {toReceive -= bytes;}
	void ReSortDownloadSlots(CUpDownClient* client);

protected:
	friend class CPPgBandwidth;

	CWinThread* pThread;

private:
    static UINT RunProc(LPVOID pParam);
    UINT RunInternal();

    void RemoveFromAllQueuesNoLock(ThrottledControlSocket* socket);
    void RemoveFromStandardListNoLock(ThrottledFileSocket* socket);
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	void RemoveFromLanPacketQueueNoLock(ThrottledFileSocket* socket);
#endif //LANCAST // NEO: NLC END

    CCriticalSection receiveLocker;

    CEvent* threadEndedEvent;
    CEvent* pauseEvent;

    bool doRun;

    CTypedPtrList<CPtrList, ThrottledControlSocket*> m_ControlQueue_list; // a queue for all the sockets that want to have Receive() called on them.
	CTypedPtrList<CPtrList, ThrottledFileSocket*> m_DownloadQueue_list;
	int     toReceive;

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	CTypedPtrList<CPtrList, ThrottledFileSocket*> m_LanQueue_list;
	int     toLanReceive;
#endif //LANCAST // NEO: NLC END
};

#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --