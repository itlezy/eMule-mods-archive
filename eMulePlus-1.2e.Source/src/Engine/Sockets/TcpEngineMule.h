// TcpEngineMule.h: interface for the CTcpEngineMule class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "TcpEngine.h"

class COpCode;

// Constants
const UINT UPLOAD_MAX_STATS_RESULTS = 20;

// Time in milliseconds
const UINT TIME_RANGE_FOR_UPLOAD = 1000;	// calculate available bandwidth for next second


class CTcpEngineMule : public CTcpEngine
{
protected:
	struct OVERLAPPED_RECV_OPCODES : public OVERLAPPED_RECV
	{
		virtual DWORD ParseRecv(CTcpEngine&, bool bIsLastRecv);

		const T_CLIENT_TYPE m_eType;

		OVERLAPPED_RECV_OPCODES(T_CLIENT_TYPE eType) : m_eType(eType) {}
	};

	virtual OVERLAPPED_RECV* AllocRecv(T_CLIENT_TYPE eType);

	virtual void OnSend(CState* pState, OVERLAPPED_SEND* pSend, EnumQueuePriority ePriority);
	virtual void OnTimer(CState* pState);

	// Upload limit
	typedef deque<OVERLAPPED_SEND*> UploadQueue;
	UploadQueue m_stUploadQueue[QUEUE_SIZE];
	ULONG m_arrUploadStats[UPLOAD_MAX_STATS_RESULTS];
	DWORD m_arrUploadTicks[UPLOAD_MAX_STATS_RESULTS];

	int m_nUploadIndex;
	CPreciseTime m_tmLastCheck;
	DWORD	m_dwAvailableBandwidth;

public:
	CTcpEngineMule();
	virtual ~CTcpEngineMule();
	bool	AllocSend(SOCKET hSocket, const COpCode&, EnumQueuePriority);
	DWORD	GetAvailableBandwidth(){ return m_dwAvailableBandwidth; }
};
