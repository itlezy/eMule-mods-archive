//this file is part of NeoMule
//Copyright (C)2006-2007 David Xanatos ( Xanatos@Lycos.at / http://neomule.sourceforge.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option any later version.
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

#ifdef NATTUNNELING // NEO: UTCP - [UserModeTCP] -- Xanatos -->
#include "AsyncSocketExLayer.h"

class CNatTunnel; // NEO: NATT - [NatTraversal]

#pragma pack(1)
struct sTransferBufferEntry{
	sTransferBufferEntry(uint32 size, const BYTE* data, uint32 Nr)
	{
		uSize = size;
		pBuffer = new BYTE[uSize];
		memcpy(pBuffer,data,size);
		uSequenceNr = Nr;
		uSendCount = 0;
		uSendTime = 0;
	}
	~sTransferBufferEntry() { delete [] pBuffer; }

	BYTE*	pBuffer;		// buffered data
	uint32	uSize;			// data size, <= max segment size
	uint32	uSequenceNr;	// segment sequence number
	uint8	uSendCount;		// we will not try resending the packet for ever just a few times
	uint32	uSendTime;		// time when we send the packet for the RTT and resend timeout
};
// Note to uSequenceNr: Normal TCP uses here the number if the first byte in the segment
//						This implementation uses an linear sequencing what is much easyer to handle 
//						and allows us to ACK segments out of order what is usefull for congestion control
//						An other reason is that assuming 1KB large segments we can transfer 4 TB without a wrap arund, 
//							so at all no sequence number wrap arund implementation is needed
#pragma pack()

typedef CRBMap<uint32,sTransferBufferEntry*> TransferBuffer;

///////////////////////////////////////////////////////////////////////////////
// CUDPSocketWnd

/*class CNATSocket;
class CNATSocketWnd : public CWnd
{
// Construction
public:
	CNATSocketWnd() {m_pOwner = NULL; m_timer = 0;}
	CNATSocket* m_pOwner;

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);

private:
	uint32 m_timer;
};*/


///////////////////////////////////////////////////////////////////////////////
// CNatSocket

class CNatSocket : public CAsyncSocketExLayer
{
public:
	CNatSocket();
	virtual ~CNatSocket();

	//initialisation
	void Init(uint8 uVersion, uint32 uMSS = 0);
	bool IsInit() {return m_uMaxSegmentSize > 0;}
	bool IsSet() {return m_uIP != 0;}

	//Notification event handlers

	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//!!! Note: OnEvent functions *MUST* be called form the main thread !!!
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//
	// Note: if you intend to use any of this notifications from a different thread 
	//		you need to use the TriggerEvent instead, or uncomment theredirects below:
	//
	//virtual void OnReceive(int nErrorCode)	{ TriggerEvent(FD_READ, nErrorCode, TRUE); }
	//virtual void OnSend(int nErrorCode)		{ TriggerEvent(FD_WRITE, nErrorCode, TRUE);	}
	//virtual void OnConnect(int nErrorCode)	{ TriggerEvent(FD_CONNECT, nErrorCode, TRUE); }
	///**/virtual void OnAccept(int nErrorCode)		{ ASSERT(0); /*TriggerEvent(FD_ACCEPT, nErrorCode, TRUE);*/ } // shell never be called
	//virtual void OnClose(int nErrorCode)		{ TriggerEvent(FD_CLOSE, nErrorCode, TRUE); }

	//Operations
	virtual BOOL Create(UINT /*nSocketPort*/ = 0, int /*nSocketType*/ = SOCK_STREAM,
						long /*lEvent*/ = FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE,
						LPCSTR /*lpszSocketAddress*/ = NULL );

	BOOL Sync(bool bAck = false);
/**/virtual BOOL Connect(LPCSTR /*lpszHostAddress*/, UINT /*nHostPort*/)				{ASSERT(0); return FALSE;}	// shell never be called
	virtual BOOL Connect(const SOCKADDR* lpSockAddr, int nSockAddrLen);

/**/virtual BOOL Listen(int /*nConnectionBacklog*/)										{ASSERT(0); return FALSE;}	// shell never be called
/**/virtual BOOL Accept(CAsyncSocketEx& /*rConnectedSocket*/, SOCKADDR* /*lpSockAddr*/ = NULL, int* /*lpSockAddrLen*/ = NULL) 	{ASSERT(0); return FALSE;}	// shell never be called

	virtual void Close();
	void Reset(int nErrorCode = 0);

	virtual BOOL GetPeerName(SOCKADDR* lpSockAddr, int* lpSockAddrLen);
#ifdef _AFX
/**/virtual BOOL GetPeerName(CString& /*rPeerAddress*/, UINT& /*rPeerPort*/)			{ASSERT(0); return FALSE;}	// shell never be called
#endif
	
	virtual int Receive(void* lpBuf, int nBufLen, int nFlags = 0);
	virtual int Send(const void* lpBuf, int nBufLen, int nFlags = 0);

	virtual BOOL ShutDown(int nHow = sends);

	virtual bool IsNatLayer() {return true;}

	//Attributes
	BOOL GetSockOpt(int nOptionName, void* lpOptionValue, int* lpOptionLen);
	BOOL SetSockOpt(int nOptionName, const void* lpOptionValue, int nOptionLen);

	//Obfuscation support
	bool IsObfuscatedConnectionEstablished() const;

protected:
	friend class CNatManager;
	friend class CNatTunnel; // NEO: NATT - [NatTraversal]

	CAsyncSocketEx* GetOwner()			{return m_pOwnerSocket;}

	// NEO: NATT - [NatTraversal]
	void SetTunnel(CNatTunnel* Tunnel)	{m_Tunnel = Tunnel;}
	CNatTunnel* GetTunnel()				{return m_Tunnel;}
	// NEO: NATT END

	//Packet prozessor
	void ProcessPacket(const BYTE* packet, UINT size, uint8 opcode);

	void ProcessDataPacket(const BYTE* packet, UINT size);
	void ProcessAckPacket(const BYTE* packet, UINT size);

	void CheckForTimeOut();

	//config
	uint32	GetTargetIP() const {return m_uIP;}
	uint16	GetTargetPort() const {return m_uPort;}
	void	SetIPPort(uint32 IP, uint16 Port);
	uint8	GetVersion() const {return m_uVersion;}

private:
	void TryToSend();
	void SendBufferedSegment(sTransferBufferEntry* BufferEntry);

	void SendAckPacket(uint32 uSequenceNr);
	
	void ShrinkCongestionWindow(/*uint16 uAcknowledgeSegments = 0*/);

	//config
	uint32	m_uIP;
	uint16	m_uPort;
	uint8	m_uVersion;
	uint32	m_uMaxSegmentSize;
	uint32	m_uMaxUploadBufferSize;
	uint32	m_uMaxDownloadBufferSize;

	//UpStream
	CCriticalSection m_UpLocker;

	uint32 m_uEstimatedRTT; // connection latency time (from pasket send to recive ack)
	uint32 m_uTimeOutDeviation; // diviation ouf our rtt based timeout vlaues
	uint32 m_uActualTimeOut; // currently computed timeout value
//#if defined(REGULAR_TCP)
//	uint8  m_uDuplicatedACKCount; // count dupplicated ack's if we recive 3 it means we must resend the next segment (Fast Retransmission)
//#endif

	uint32 m_uCongestionWindow; // our estimated congestion window
	uint32 m_uAdvertisedWindow; // the window size advertised in the last recived ACK
	uint32 m_uCongestionThreshold; // used for slow start and fast recovery
	uint32 m_uPendingWindowSize; // the amount of segments currently being on the network

	uint32 m_uLastSentSequenceNr; // number of last sent segment
	uint32 m_uUploadBufferSize;
	TransferBuffer m_UploadBuffer;	// Note: unfortunatly the CRBMap does not have a FindFirstKeyBefoure function 
									//	so for the Upload buffer I reverse the map direction by using mapkey = UINT_MAX-key ;)

	//DowmStream
	CCriticalSection m_DownLocker;

	uint32 m_uLastByteRcvd;
	uint32 m_uNextByteRead;

	uint32 m_uLastAcknowledgeSequenceNr; // last sequence number completly pased to the CEMsocket
//#if defined(REGULAR_TCP)
//	bool   m_bSegmentMissing;
//#endif
	uint32 m_uCurrentSegmentPosition; // the CEMsocket may recive only a part of the current segment
	uint32 m_uDownloadBufferSize;
	TransferBuffer m_DownloadBuffer;

	//Attributes
	uint8  m_ShutDown;

	CNatTunnel* m_Tunnel; // NEO: NATT - [NatTraversal]
};

//debuging
enum nat_log_event
{
	NLE_RECIVED, // incomming packet
	NLE_SENT, // outgoing packet
	NLE_READ, // data read from download buffer
	NLE_WRITE, // data wroten to upload buffer
	NLE_CONFIGURE, // changed configuration
	NLE_FUNCTION, // called function
	NLE_RESULT, // result to above line
	NLE_STATE, // internal state information
	NLE_OTHER, // other output
};
inline void NATTrace(uint32 address,nat_log_event event, char* name, char* fmt, ...);

#endif //NATTUNNELING // NEO: UTCP END <-- Xanatos --