//this file is part of eMule
//Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "EMSocket.h"
#include "opcodes.h"
#include "emule.h"
#include "emuleDlg.h"
#include "otherfunctions.h"

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


namespace
{
	inline void EMTrace(char* fmt, ...)
	{
#ifdef EMSOCKET_DEBUG
		va_list argptr;
		char bufferline[512];
		va_start(argptr, fmt);
		_vsnprintf(bufferline, 512, fmt, argptr);
		va_end(argptr);
		char osDate[30],osTime[30]; 
		char temp[1024]; 
		_strtime( osTime );
		_strdate( osDate );
		int len = _snprintf(temp,1021,"%s %s: %s",osDate,osTime,bufferline);
		temp[len++] = 0x0d;
		temp[len++] = 0x0a;
		temp[len+1] = 0;
		HANDLE hFile = CreateFile("c:\\EMSocket.log",           // open MYFILE.TXT 
                GENERIC_WRITE,              // open for reading 
                FILE_SHARE_READ,           // share for reading 
                NULL,                      // no security 
                OPEN_ALWAYS,               // existing file only 
                FILE_ATTRIBUTE_NORMAL,     // normal file 
                NULL);                     // no attr. template 
  
		if (hFile != INVALID_HANDLE_VALUE) 
		{ 
			DWORD nbBytesWritten = 0;
			SetFilePointer(hFile, 0, NULL, FILE_END);
			BOOL b = WriteFile(
				hFile,                    // handle to file
				temp,                // data buffer
				len,     // number of bytes to write
				&nbBytesWritten,  // number of bytes written
				NULL        // overlapped buffer
			);
			CloseHandle(hFile);
		}
#else
		NOPRM(fmt);
		//va_list argptr;
		//va_start(argptr, fmt);
		//va_end(argptr);
#endif //EMSOCKET_DEBUG
	}
}

CEMSocket::CEMSocket(void)
{
	m_eConnectionState = ES_NOTCONNECTED;

	// Download (pseudo) rate control	
	m_dwDownloadLimit = 0;
	m_bEnableDownloadLimit = false;
	m_bPendingOnReceive = false;

	// Download partial header
	m_dwPendingHeaderSize = 0;

	// Download partial packet
	m_pPendingPacket = NULL;
	m_dwPendingPacketSize = 0;

	// Upload control
	m_pcSendBuffer = NULL;
	m_dwSendBufLen = 0;
	m_dwNumBytesSent = 0;
	m_bLinkedPackets = false;

	// Proxy Support
	m_pProxyLayer = NULL;
	m_bProxyConnectFailed = false;

	m_bInPacketReceived = false;
}

CEMSocket::~CEMSocket()
{
	EMTrace("CEMSocket::~CEMSocket() on %d",(SOCKET)this);
	ClearQueues();
	RemoveAllLayers();
	AsyncSelect(0);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Connection initialization is done by class itself
BOOL CEMSocket::Connect(SOCKADDR *pSockAddr, int iSockAddrLen)
{
	InitProxySupport();
	m_eConnectionState = ES_CONNECTING;
	return CAsyncSocketEx::Connect(pSockAddr, iSockAddrLen);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Connection initialization is done by class itself
BOOL CEMSocket::Connect(LPCSTR lpszHostAddress, UINT nHostPort)
{
	InitProxySupport();
	m_eConnectionState = ES_CONNECTING;
	return CAsyncSocketEx::Connect(lpszHostAddress, nHostPort);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEMSocket::InitProxySupport()
{
//	Destroy all proxy layers (done inside Close())
	Close();

//	Proxy initialization
	const ProxySettings	&settings = g_App.m_pPrefs->GetProxySettings();

	m_bProxyConnectFailed = false;
	if (settings.m_bUseProxy && settings.m_nType != PROXYTYPE_NOPROXY)
	{
		m_pProxyLayer = new CAsyncProxySocketLayer;
		switch (settings.m_nType)
		{
			case PROXYTYPE_SOCKS4:
			case PROXYTYPE_SOCKS4A:
				m_pProxyLayer->SetProxy(settings.m_nType, settings.m_strName, settings.m_uPort);
				break;
			case PROXYTYPE_SOCKS5:
			case PROXYTYPE_HTTP11:
				if (settings.m_bEnablePassword)
					m_pProxyLayer->SetProxy(settings.m_nType, settings.m_strName, settings.m_uPort, settings.m_strUser, settings.m_strPassword);
				else
					m_pProxyLayer->SetProxy(settings.m_nType, settings.m_strName, settings.m_uPort);
				break;
			default:
				ASSERT(0);
		}
		AddLayer(m_pProxyLayer);
	}

//	Connection Initialization
	Create();
	AsyncSelect(FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEMSocket::ClearQueues()
{
	EMTrace("CEMSocket::ClearQueues on %d",(SOCKET)this);
	while(!m_controlPacketQueue.IsEmpty())
	{
		delete m_controlPacketQueue.RemoveHead();
	}
	while (!m_standardPacketQueue.IsEmpty())
	{
		delete m_standardPacketQueue.RemoveHead();
	}

	// Download (pseudo) rate control	
	m_dwDownloadLimit = 0;
	m_bEnableDownloadLimit = false;
	m_bPendingOnReceive = false;

	// Download partial header
	m_dwPendingHeaderSize = 0;

	// Download partial packet
	delete m_pPendingPacket;
	m_pPendingPacket = NULL;
	m_dwPendingPacketSize = 0;

	// Upload control
	delete[] m_pcSendBuffer;
	m_pcSendBuffer = NULL;

	m_dwSendBufLen = 0;
	m_dwNumBytesSent = 0;
	m_bLinkedPackets = false;
}

void CEMSocket::OnClose(int nErrorCode)
{
	m_eConnectionState = ES_DISCONNECTED;
	CAsyncSocketEx::OnClose(nErrorCode);
	RemoveAllLayers();
	ClearQueues();
}

BOOL CEMSocket::AsyncSelect(long lEvent)
{
#ifdef EMSOCKET_DEBUG
	if (lEvent&FD_READ)
		EMTrace("  FD_READ");
	if (lEvent&FD_CLOSE)
		EMTrace("  FD_CLOSE");
	if (lEvent&FD_WRITE)
		EMTrace("  FD_WRITE");
#endif
	// deadlake changed to AsyncSocketEx PROXYSUPPORT
	if (m_SocketData.hSocket != INVALID_SOCKET)
		return CAsyncSocketEx::AsyncSelect(lEvent);
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEMSocket::OnReceive(int iErrorCode)
{
//	The 2 meg size was taken from another place     // MOREVIT - Um, what??
	static char g_arrcReadBuffer[2102400];			// 16*TCP window (16*131400)

//	Check for an error code
	if (iErrorCode != 0)
	{
		OnError(iErrorCode);
		return;
	}
	
//	Check current connection state
	if (m_eConnectionState == ES_DISCONNECTED)
		return;
	else
		m_eConnectionState = ES_CONNECTED; // ES_DISCONNECTED, ES_NOTCONNECTED, ES_CONNECTED

//	* CPU load improvement
	if (m_bEnableDownloadLimit && m_dwDownloadLimit == 0)
	{
		EMTrace("CEMSocket::OnReceive blocked by limit");
		m_bPendingOnReceive = true;
		return;
	}

//	Determine the maximum amount of data we can read, allowing for the download limit (if any)
//	Remark: an overflow can not occur here
	uint32		dwReadMax = sizeof(g_arrcReadBuffer) - m_dwPendingHeaderSize;

	if (m_bEnableDownloadLimit && dwReadMax > m_dwDownloadLimit)
		dwReadMax = m_dwDownloadLimit;

//	We attempt to read up to 2 megs at a time (minus whatever is in our internal read buffer)
	uint32		dwNumBytesReceived = Receive(g_arrcReadBuffer + m_dwPendingHeaderSize, dwReadMax);

	if (dwNumBytesReceived == SOCKET_ERROR || dwNumBytesReceived == 0)
	{
	//	TODO: Get error information from GetLastError()?
		return;
	}

//	* Bandwidth control
	if (m_bEnableDownloadLimit)
	{
	//	Reduce the download limit by the number of bytes received.
		m_dwDownloadLimit -= dwNumBytesReceived;
	}

//	* CPU load improvement
//	Detect if the socket's buffer is empty (or the size did match...)
	m_bPendingOnReceive = (dwNumBytesReceived == dwReadMax);

//	Copy back the partial header into the global read buffer for processing
	if (m_dwPendingHeaderSize > 0)
	{
		memcpy(g_arrcReadBuffer, m_arrcPendingHeader, m_dwPendingHeaderSize);
		dwNumBytesReceived += m_dwPendingHeaderSize;
		m_dwPendingHeaderSize = 0;
	}

	char		*pcReadBuffer = g_arrcReadBuffer; // floating index initialized with begin of buffer
	const char	*pcReadBufferEnd = g_arrcReadBuffer + dwNumBytesReceived; // end of buffer

//	Loop, processing packets until we run out of them
	while ( (pcReadBufferEnd - pcReadBuffer >= sizeof(PacketHeader_Struct))
		 || ((m_pPendingPacket != NULL) && (pcReadBufferEnd - pcReadBuffer > 0)) )
	{ 
	// Two possibilities here: 
	//
	// 1. There is no pending incoming packet
	// 2. There is already a partial pending incoming packet
	//
	// It's important to remember that emule exchange two kinds of packet
	// - The control packet
	// - The data packet for the transport of the block
	// 
	// The biggest part of the traffic is done with the data packets. 
	// The default size of one block is 10240 bytes (or less if compressed), but the
	// maximal size for one packet on the network is 1300 bytes. It's the reason
	// why most of the Blocks are splitted before to be sent. 
	//
	// Conclusion: When the download limit is disabled, this method can be at least 
	// called 8 times (10240/1300) by the lower layer before a split packet is 
	// rebuild and transferred to the above layer for processing.
	//
	// The purpose of this algorithm is to limit the amount of data exchanged between buffers

		if (m_pPendingPacket == NULL)
		{
		//	Recheck current connection state as it can be changed after data reception
			if (m_eConnectionState == ES_DISCONNECTED)
				return;
		//	Check the header data
			PacketHeader_Struct	*pNewHeader = reinterpret_cast<PacketHeader_Struct*>(pcReadBuffer);

		//	Bugfix: We still need to check for a valid protocol
		//	Remark: the default eMule v0.26b had removed this test......
			switch (pNewHeader->byteEDonkeyProtocol)
			{
				case OP_EDONKEYPROT:
				case OP_PACKEDPROT:
				case OP_EMULEPROT:
					break;
				default:
					EMTrace("%s: ERROR - Wrong protocol in packet header", __FUNCTION__);
					OnError(ERR_WRONGHEADER);
					return;
			}

		//	Security: Check for buffer overflow (2MB)
			if ((pNewHeader->dwPacketLength - 1) > sizeof(g_arrcReadBuffer))
			{
				OnError(ERR_TOOBIG);
				return;
			}

		//	Create new packet container
			m_pPendingPacket = new Packet(pNewHeader);
		//	Only the header is initialized so far. Advance past it
			pcReadBuffer += sizeof(PacketHeader_Struct);

		//	Init data buffer
			m_pPendingPacket->m_pcBuffer = new char[m_pPendingPacket->m_dwSize + 1];
			m_dwPendingPacketSize = 0;
		}

	//	Bytes ready to be copied into packet's internal buffer
		ASSERT(pcReadBuffer <= pcReadBufferEnd);
		uint32 dwBytesToCopy = ((m_pPendingPacket->m_dwSize - m_dwPendingPacketSize) < static_cast<uint32>(pcReadBufferEnd - pcReadBuffer))
							 ? (m_pPendingPacket->m_dwSize - m_dwPendingPacketSize)
							 : static_cast<uint32>(pcReadBufferEnd - pcReadBuffer);

	//	Copy bytes from Global buffer to packet's internal buffer
		memcpy2(&m_pPendingPacket->m_pcBuffer[m_dwPendingPacketSize], pcReadBuffer, dwBytesToCopy);
		m_dwPendingPacketSize += dwBytesToCopy;
		pcReadBuffer += dwBytesToCopy;

	//	Check if packet is complete
		ASSERT(m_pPendingPacket->m_dwSize >= m_dwPendingPacketSize);
		if (m_pPendingPacket->m_dwSize == m_dwPendingPacketSize)
		{
#ifdef EMSOCKET_DEBUG
			EMTrace("CEMSocket::PacketReceived on %d, opcode=%X, realSize=%d", 
				    static_cast<SOCKET>(this), m_pPendingPacket->m_eOpcode, m_pPendingPacket->GetRealPacketSize());
#endif EMSOCKET_DEBUG

		//	Process packet
			m_bInPacketReceived = true;
			PacketReceived(m_pPendingPacket);
			m_bInPacketReceived = false;
			delete m_pPendingPacket;
			m_pPendingPacket = NULL;
			m_dwPendingPacketSize = 0;
		}
	}

	// Finally, if there is any data left over, save it for next time
	ASSERT(pcReadBuffer <= pcReadBufferEnd);
	ASSERT(pcReadBufferEnd - pcReadBuffer < sizeof(PacketHeader_Struct));
	if (pcReadBuffer != pcReadBufferEnd)
	{
		// Keep the partial head
		m_dwPendingHeaderSize = pcReadBufferEnd - pcReadBuffer;
		memcpy(m_arrcPendingHeader, pcReadBuffer, m_dwPendingHeaderSize);
	}	
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEMSocket::SetDownloadLimit(uint32 dwLimit)
{
	m_dwDownloadLimit = dwLimit;
	m_bEnableDownloadLimit = true;	
	
//	CPU load improvement
	if (dwLimit > 0 && m_bPendingOnReceive)
	{
		OnReceive(0);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEMSocket::DisableDownloadLimit(bool bAvoidRead)
{
	m_bEnableDownloadLimit = false;

//	There're cases when DisableDownloadLimit can be called inside OnReceive()
//	as OnReceive() uses static buffer it shouldn't be called recursively
	if (bAvoidRead && m_bInPacketReceived)
		return;

	// CPU load improvement
	if (m_bPendingOnReceive)
		OnReceive(0);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEMSocket::SendPacket(Packet *pPacket, bool bDeletePacket/*=true*/, bool bControlPacket/*=true*/)
{
//	EMTrace("CEMSocket::OnSenPacked1 linked: %i, controlcount %i, standardcount %i, isbusy: %i",m_bLinkedPackets, m_controlPacketQueue.GetCount(), m_standardPacketQueue.GetCount(), IsBusy());
	if (!bDeletePacket)
	{
		ASSERT (!pPacket->IsSplit());

		Packet		*pPacketCopy = new Packet(pPacket->m_eOpcode,pPacket->m_dwSize);

		memcpy2(pPacketCopy->m_pcBuffer,pPacket->m_pcBuffer,pPacket->m_dwSize);
		pPacket = pPacketCopy;
	}			
	if (!IsConnected() || IsBusy() || (m_bLinkedPackets && bControlPacket))
	{
		if (bControlPacket)
			m_controlPacketQueue.AddTail(pPacket);
		else
			m_standardPacketQueue.AddTail(pPacket);
		return;
	}

	bool		bCheckControlQueue = false;

	if (pPacket->IsLastSplit())
	{
		m_bLinkedPackets = false;
		bCheckControlQueue = true;
	}
	else if (pPacket->IsSplit())
	{
		m_bLinkedPackets = true;
	}
	else if (m_bLinkedPackets)
	{
		ASSERT(false);
	}
//	EMTrace("CEMSocket::OnSenPacked2 linked: %i, controlcount %i, standardcount %i, isbusy: %i",m_bLinkedPackets, m_controlPacketQueue.GetCount(), m_standardPacketQueue.GetCount(), IsBusy());
	Send(pPacket->DetachPacket(), pPacket->GetRealPacketSize());
	delete pPacket;
	if (!IsBusy() && bCheckControlQueue)
		OnSend(0);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEMSocket::OnSend(int iErrorCode)
{
	if (iErrorCode)
	{
		OnError(iErrorCode);
		return;
	}

//	EMTrace("CEMSocket::OnSend linked: %i, controlcount %i, standardcount %i, isbusy: %i",m_bLinkedPackets, m_controlPacketQueue.GetCount(), m_standardPacketQueue.GetCount(), IsBusy());

	if (m_eConnectionState == ES_DISCONNECTED)
		return;
	else
		m_eConnectionState = ES_CONNECTED;

//	If there's data left in the send buffer, continue sending
	if (IsBusy())
		Send(NULL, 0, 0);
//	If there's _still_ data left...
	if (IsBusy())
		return;
	while ((m_controlPacketQueue.GetHeadPosition() != NULL) && !IsBusy() && IsConnected() && !m_bLinkedPackets)
	{
		Packet		*pPacket = m_controlPacketQueue.GetHead();

//		EMTrace("CEMSocket::OnSend sending control packet on %d, size=%u",(SOCKET)this, pPacket->GetRealPacketSize());
		Send(pPacket->DetachPacket(),pPacket->GetRealPacketSize());
		m_controlPacketQueue.RemoveHead();
		delete pPacket;
	}

	while ((m_standardPacketQueue.GetHeadPosition() != NULL) && !IsBusy() && IsConnected())
	{
		Packet		*pPacket = m_standardPacketQueue.GetHead();

		if (pPacket->IsLastSplit())
			m_bLinkedPackets = false;
		else if (pPacket->IsSplit())
			m_bLinkedPackets = true;
		else if (m_bLinkedPackets)
		{
			ASSERT(false);
		}
//		EMTrace("CEMSocket::OnSend sending standard packet on %d, size=%u",(SOCKET)this, pPacket->GetRealPacketSize());
		Send(pPacket->DetachPacket(), pPacket->GetRealPacketSize());
		m_standardPacketQueue.RemoveHead();
		delete pPacket;
	}

	while ((m_controlPacketQueue.GetHeadPosition() != NULL) && !IsBusy() && IsConnected() && !m_bLinkedPackets)
	{
		Packet		*pPacket = m_controlPacketQueue.GetHead();

//		EMTrace("CEMSocket::OnSend sending control packet on %d, size=%u",(SOCKET)this, pPacket->GetRealPacketSize());
		Send(pPacket->DetachPacket(), pPacket->GetRealPacketSize());
		m_controlPacketQueue.RemoveHead();
		delete pPacket;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CEMSocket::Send(char *pcBuffer, int iBufLen, int iFlags)
{
	NOPRM(iFlags);
//	EMTrace("CEMSocket::Send linked: %i, controlcount %i, standardcount %i, isbusy: %i",m_bLinkedPackets, m_controlPacketQueue.GetCount(), m_standardPacketQueue.GetCount(), IsBusy());
	ASSERT(m_pcSendBuffer == NULL || pcBuffer == NULL);

	if (pcBuffer != NULL)
	{
		m_pcSendBuffer = pcBuffer;
		m_dwSendBufLen = iBufLen;
		m_dwNumBytesSent = 0;
	}
	while (true)
	{
		uint32		dwBytesToSend = m_dwSendBufLen - m_dwNumBytesSent;

		if (dwBytesToSend > MAXFRAGSIZE)
			dwBytesToSend = MAXFRAGSIZE;
		ASSERT(dwBytesToSend != 0);
		
		uint32		dwNumBytesSent = CAsyncSocketEx::Send(m_pcSendBuffer+m_dwNumBytesSent,dwBytesToSend);

		if (dwNumBytesSent == static_cast<uint32>(SOCKET_ERROR))
		{
			uint32		dwError = GetLastError();

		//	If the socket isn't ready for more data yet...
			if (dwError == WSAEWOULDBLOCK)
			{
				break;
			}
			else
			{
//				OnError(dwError);
				return -1;
			}
		}
		m_dwNumBytesSent += dwNumBytesSent;
		ASSERT(m_dwNumBytesSent <= m_dwSendBufLen);
		if (m_dwNumBytesSent == m_dwSendBufLen)
		{
			delete[] m_pcSendBuffer;
			m_pcSendBuffer = NULL;
			m_dwNumBytesSent = 0;
			m_dwSendBufLen = 0;
			break;
		}
	}

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// pach2:
// written this overriden Receive to handle transparently FIN notifications coming from calls to recv()
// This was maybe(??) the cause of a lot of socket error, notably after a brutal close from peer
// also added trace so that we can debug after the fact ...
int CEMSocket::Receive(void* lpBuf, int nBufLen, int nFlags)
{
//	EMTrace("CEMSocket::Receive on %d, maxSize=%d",(SOCKET)this,nBufLen);
	int recvRetCode = CAsyncSocketEx::Receive(lpBuf,nBufLen,nFlags);
	switch (recvRetCode)
	{
	case 0:
		//EMTrace("CEMSocket::##Received FIN on %d, maxSize=%d",(SOCKET)this,nBufLen);
		// FIN received on socket // Connection is being closed by peer
		//ASSERT (false);
		if ( 0 == AsyncSelect(FD_CLOSE|FD_WRITE) )
 		{ // no more READ notifications ...
			//int waserr = GetLastError(); // oups, AsyncSelect failed !!!
			ASSERT(false);
		}
		return 0;
	case SOCKET_ERROR:
		switch(GetLastError())
		{
		case WSANOTINITIALISED:
			ASSERT(false);
			EMTrace("CEMSocket::OnReceive:A successful AfxSocketInit must occur before using this API.");
			break;
		case WSAENETDOWN:
			ASSERT(true);
			EMTrace("CEMSocket::OnReceive:The socket %d received a net down error",(SOCKET)this);
			break;
		case WSAENOTCONN: // The socket is not connected. 
			EMTrace("CEMSocket::OnReceive:The socket %d is not connected",(SOCKET)this);
			break;
		case WSAEINPROGRESS:   // A blocking Windows Sockets operation is in progress. 
			EMTrace("CEMSocket::OnReceive:The socket %d is blocked",(SOCKET)this);
			break;
		case WSAEWOULDBLOCK:   // The socket is marked as nonblocking and the Receive operation would block. 
			EMTrace("CEMSocket::OnReceive:The socket %d would block",(SOCKET)this);
			break;
		case WSAENOTSOCK:   // The descriptor is not a socket. 
			EMTrace("CEMSocket::OnReceive:The descriptor %d is not a socket (may have been closed or never created)",(SOCKET)this);
			break;
		case WSAEOPNOTSUPP:  // MSG_OOB was specified, but the socket is not of type SOCK_STREAM. 
			break;
		case WSAESHUTDOWN:   // The socket has been shut down; it is not possible to call Receive on a socket after ShutDown has been invoked with nHow set to 0 or 2. 
			EMTrace("CEMSocket::OnReceive:The socket %d has been shut down",(SOCKET)this);
			break;
		case WSAEMSGSIZE:   // The datagram was too large to fit into the specified buffer and was truncated. 
			EMTrace("CEMSocket::OnReceive:The datagram was too large to fit and was truncated (socket %d)",(SOCKET)this);
			break;
		case WSAEINVAL:   // The socket has not been bound with Bind. 
			EMTrace("CEMSocket::OnReceive:The socket %d has not been bound",(SOCKET)this);
			break;
		case WSAECONNABORTED:   // The virtual circuit was aborted due to timeout or other failure. 
			EMTrace("CEMSocket::OnReceive:The socket %d has not been bound",(SOCKET)this);
			break;
		case WSAECONNRESET:   // The virtual circuit was reset by the remote side. 
			EMTrace("CEMSocket::OnReceive:The socket %d has not been bound",(SOCKET)this);
			break;
		default:
			EMTrace("CEMSocket::OnReceive:Unexpected socket error %x on socket %d",GetLastError(),(SOCKET)this);
			break;
		}
		break;
	default:
//		EMTrace("CEMSocket::OnReceive on %d, receivedSize=%d",(SOCKET)this,recvRetCode);
		return recvRetCode;
	}
	return SOCKET_ERROR;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEMSocket::RemoveAllLayers()
{
//	Reset proxy layer chain
	CAsyncSocketEx::RemoveAllLayers();
	
	delete m_pProxyLayer;
	m_pProxyLayer = NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CEMSocket::OnLayerCallback(const CAsyncSocketExLayer *pLayer, int nType, int nCode, WPARAM wParam, LPARAM lParam)
{
	ASSERT(pLayer);
	/*if (nType == LAYERCALLBACK_STATECHANGE)
	{
		CString 	logline;

		if (pLayer==m_pProxyLayer)
		{
			logline.Format(_T("ProxyLayer changed state from %d to %d"), nParam2, nParam1);
			AddLogLine(0, logline);
		}
		else
			logline.Format(_T("Layer @ %d changed state from %d to %d"), pLayer, nParam2, nParam1);
			AddLogLine(0, logline);
		return 1;
	}
	else */if (nType == LAYERCALLBACK_LAYERSPECIFIC)
	{
		if (pLayer == m_pProxyLayer)
		{
			CString	strError(GetProxyError(nCode));

			switch (nCode)
			{
				case PROXYERROR_NOCONN:
					// We failed to connect to the proxy
					m_bProxyConnectFailed = true;
				case PROXYERROR_REQUESTFAILED:
					if (lParam && ((LPCSTR)lParam)[0] != '\0')
					{
						strError += _T(" - ");
						strError += (LPCSTR)lParam;
					}
					if (wParam)
					{
						CString	strErrInf;

						if (GetErrorMessage(wParam, strErrInf, 1))
						{
							strError += _T(" - ");
							strError += strErrInf;
						}
					}
				default:
					AddLogLine(0, _T("Proxy error - %s"), strError);
			}
		}
	}
	return 1;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	TruncateQueues()
//		Removes all packets from the standard queue that don't have to be sent for the socket to be able to send a control packet
//
//		Before a socket can send a new packet, the current packet has to be finished. If the current packet is part of
//		a split packet, then all parts of that split packet must be sent before the socket can send a control packet
//
//		This method leaves only minimal required number of data packets to transfer full data packet (complete chain of packets)
//		Return:
//			true  - full chain of data packets is available (no need more)
//			false - more data packets are required to complete chain of packets
bool CEMSocket::TruncateQueues()
{
	POSITION	pos, pos2;
	Packet		*pPkt;

	if (m_standardPacketQueue.IsEmpty() && !m_bLinkedPackets)
		return true;

	for (pos = m_standardPacketQueue.GetHeadPosition(); pos != NULL;)
	{
		if (m_standardPacketQueue.GetNext(pos)->IsLastSplit())
		{
		//	Found last packet of data packet chain, flush the rest to save bandwidth
			for (;(pos2 = pos) != NULL;)
			{
				pPkt = m_standardPacketQueue.GetNext(pos);
				m_standardPacketQueue.RemoveAt(pos2);
				delete pPkt;
			}
			return true;
		}
	}

	return false;
}
