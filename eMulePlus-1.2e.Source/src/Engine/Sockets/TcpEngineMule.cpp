// TcpEngineMule.cpp: implementation of the CTcpEngineMule class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TcpEngineMule.h"

#include "OpCode.h"
#include "../Data/Prefs.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTcpEngineMule::CTcpEngineMule()
{
	ZeroMemory(m_arrUploadStats, sizeof(m_arrUploadStats));
	ZeroMemory(m_arrUploadTicks, sizeof(m_arrUploadTicks));
	m_nUploadIndex = 0;
	m_tmLastCheck = CPreciseTime::GetCurrentTime();
	m_dwAvailableBandwidth = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTcpEngineMule::~CTcpEngineMule()
{
	// Destroy all unsent packets
	for(int i = 0; i < QUEUE_SIZE; i++)
	{
		while(!m_stUploadQueue[i].empty())
		{
			OVERLAPPED_SEND* pSend = m_stUploadQueue[i].front();
			delete pSend;
			m_stUploadQueue[i].pop_front();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTcpEngine::OVERLAPPED_RECV *CTcpEngineMule::AllocRecv(T_CLIENT_TYPE eType)
{
	switch (eType)
	{
	case T_CLIENT_PEER:
	case T_CLIENT_SERVER:
		return new OVERLAPPED_RECV_OPCODES(eType);
	default:
		return CTcpEngine::AllocRecv(eType);
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
bool CTcpEngineMule::AllocSend(SOCKET hSocket, const COpCode &stOpCode, EnumQueuePriority ePriority)
{
	BYTE			pBuf[TCP_OPTIMAL_BUF];
	CStream_MemEx	stStream;

	stStream.m_pPtr = pBuf;
	stStream.m_dwSize = sizeof(pBuf);

	VERIFY(stOpCode.Write(stStream, OP_TRANSPORT_TCP, stOpCode.m_bSupportNewTags));
	if (!stStream.m_dwSizeExtra)
		return CTcpEngine::AllocSend(hSocket, pBuf, sizeof(pBuf) - stStream.m_dwSize, ePriority);

	//	This opcode seems to be huge.
	DWORD dwSizeHuge = sizeof(pBuf) + stStream.m_dwSizeExtra;
	PBYTE pBufDyn = new BYTE[dwSizeHuge];

	if (!pBufDyn)
	{
		AddLog(LOG_ERROR, _T("No memory"));
		CTask_Tcp_Err::Post(hSocket, -1);
		return false;
	}

	stStream.m_pPtr = pBufDyn;
	stStream.m_dwSize = dwSizeHuge;
	stStream.m_dwSizeExtra = 0;
	VERIFY(stOpCode.Write(stStream, OP_TRANSPORT_TCP, stOpCode.m_bSupportNewTags));

	bool bSend = CTcpEngine::AllocSend(hSocket, pBufDyn, dwSizeHuge, ePriority);
	delete[] pBufDyn;

	return bSend;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CTcpEngineMule::OVERLAPPED_RECV_OPCODES::ParseRecv(CTcpEngine &stEngine, bool)
{
	char* pData = GetBuffer();

	ASSERT(pData);

	for (DWORD dwParsed = 0; ;)
	{
		//	Over tcp transport, opcodes always have a header of 6 bytes
		if (m_dwBufUsage < 6 + dwParsed)
		{
			break;
		}

		DWORD dwOpCodeLen = *((DWORD*) (pData + 1));

		if (dwOpCodeLen == 0)
		{
			AddLog(LOG_ERROR, _T("Invalid recv data"));
			return m_dwBufUsage; // will discard all the buffer
		}

		if (dwOpCodeLen + 5 + dwParsed > m_dwBufUsage)
			break;

		CStream_Mem stStream;

		stStream.m_pPtr = (PBYTE) pData + 6;
		stStream.m_dwSize = dwOpCodeLen - 1;

		TCHAR strOpCode[100];
		strOpCode[0] = NULL;
		COpCode* pOpCode = COpCode::Read(stStream, ((PBYTE)pData)[5], ((PBYTE)pData)[0], m_eType, strOpCode);

		if (pOpCode != NULL)
		{
			if (stStream.m_dwSize)
			{
				//				ASSERT(FALSE);
				AddLog(LOG_WARNING, _T("%u bytes left from OpCode initialization, id=%s (%x)"), stStream.m_dwSize, pOpCode->TaskName(), pOpCode->GetID());
			}

			pOpCode->m_hSocket = m_hSocket;
			g_stEngine.Sockets.Push(pOpCode);
			//	g_stEngine.PushToLogger(pOpCode);
		}
		else
		{
			AddLog(LOG_WARNING, _T("Can't receive opcode %s (%x) protocol=%x"), strOpCode, ((PBYTE)pData)[5], ((PBYTE)pData)[0]);
			//			ASSERT(FALSE);
		}

		dwOpCodeLen += 5;
		pData += dwOpCodeLen;
		dwParsed += dwOpCodeLen;
	}
	return dwParsed;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CTcpEngineMule::OnSend(CState* pState, OVERLAPPED_SEND* pSend, EnumQueuePriority ePriority)
{
	m_stUploadQueue[ePriority].push_back(pSend);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CTcpEngineMule::OnTimer(CState* pState)
{
	// 1st of all - approximate how much data we can send now.
	CPreciseTime tmNow = CPreciseTime::GetCurrentTime();

	DWORD dwPeriod = 0, dwSizeSent = 0;
	for (int i = 0; i < UPLOAD_MAX_STATS_RESULTS; i++)
	{
		dwPeriod += m_arrUploadTicks[i];
		dwSizeSent += m_arrUploadStats[i];
	}

	// If we're only starting, let's send 100 bytes per cycle
	DWORD dwBytesToSend = 100;

	if (dwPeriod)
	{
		// GetMaxUpload() returns 15 for 1.5 kb/sec, 
		// Therefore we multiply it by 1kb and divide by 10
		// (another 1000 for convert time from milliseconds to seconds)
		dwBytesToSend = g_stEngine.Prefs.GetMaxUpload() * 1024 * (dwPeriod + TIMER_CHECK_TIME) / 10000;
		if (dwBytesToSend > dwSizeSent)
			dwBytesToSend -= dwSizeSent;
		else
			dwBytesToSend = 0;
	}

	dwSizeSent = 0;

	for(int i = 0; i < QUEUE_SIZE; i++)
	{
		while(!m_stUploadQueue[i].empty())
		{
			OVERLAPPED_SEND* pSend = m_stUploadQueue[i].front();
			ASSERT(pSend);

			// If this block's size is too much, we stop until next check
			if (pSend->len > dwBytesToSend)
				break;

			m_stUploadQueue[i].pop_front();

			DWORD dwLen = pSend->len;

			if(!pState->SendBlock(pSend))
			{
				// In case of error we don't count this packet
				dwLen = 0;
			}

			dwBytesToSend -= dwLen;
			dwSizeSent += dwLen;
		}
	}

	// Fill in values for bandwidth approximation
	m_arrUploadStats[m_nUploadIndex] = dwSizeSent;
	m_arrUploadTicks[m_nUploadIndex] = tmNow - m_tmLastCheck;

	m_tmLastCheck = tmNow;
	m_nUploadIndex = (m_nUploadIndex + 1) % UPLOAD_MAX_STATS_RESULTS;


	// Calculate occupied bandwidth
	DWORD dwSizeToSend = 0;
	for(int i = 0; i < QUEUE_SIZE; i++)
	{
		for(UploadQueue::iterator it = m_stUploadQueue[i].begin(); it != m_stUploadQueue[i].end(); it++)
		{
			OVERLAPPED_SEND* pSend = *it;
			ASSERT(pSend);
			dwSizeToSend += pSend->len;
		}
	}

	// GetMaxUpload() returns 15 for 1.5 kb/sec, 
	// Therefore we multiply it by 1kb and divide by 10
	// (another 1000 for convert time from milliseconds to seconds)
	DWORD dwUpperLimit = g_stEngine.Prefs.GetMaxUpload() * 1024 * TIME_RANGE_FOR_UPLOAD / 10000;
	if (dwSizeToSend < dwUpperLimit)
		m_dwAvailableBandwidth = dwUpperLimit - dwSizeToSend;
	else
		m_dwAvailableBandwidth = 0;	// all bandwidth for next second is already taken out
}
