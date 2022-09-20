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

#include "StdAfx.h"
#ifdef _DEBUG
#include "DebugHelpers.h"
#endif
#include "emule.h"
#include "Preferences.h"
#include "emuledlg.h"
#include "Log.h"
#include "AsyncSocketEx.h"
#include "AsyncProxySocketLayer.h"
#include "NatSocket.h"
#include "NatManager.h"
#include "Packets.h"
#include "Functions.h"
#include "otherFunctions.h"
#include "NeoOpcodes.h"
#include "NatTunnel.h" // NEO: NATT - [NatTraversal]

#ifdef NATTUNNELING // NEO: UTCP - [UserModeTCP] -- Xanatos -->

inline void NATTrace(uint32 address,nat_log_event event, char* name, char* fmt, ...) { // function does not accept unicode text use CStringA()
 #ifdef NATSOCKET_DEBUG
	va_list argptr;
	char bufferline[1024];
	va_start(argptr, fmt);
	_vsnprintf(bufferline, 512, fmt, argptr);
	va_end(argptr);

	char Events[9][30] = {
		"Recived Packet",
		"Sent Packet   ",
		"Read Data     ",
		"Write Data    ",
		"Configure     ",
		"Enter Function",
		"        Result",
		"Socket Status ",
		"Other Info    "};

	char temp[1024+256]; 
	_snprintf(temp,1021,"%u: %s: %s: %s \x0d\x0a\x00", address, Events[event], name, bufferline);

	TRACE(temp); // log to CRT log or debuger output window

	// Log to file
	/*
	char osDate[30],osTime[30]; 
	_strtime( osTime );
	_strdate( osDate );

	char tempf[1024+512]; 
	int len = _snprintf(tempf,1021,"%s %s: %s", osDate, osTime, temp);

	HANDLE hFile = CreateFile("c:\\NATSocket.log",  // open MYFILE.TXT 
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
			tempf,                // data buffer
			len,     // number of bytes to write
			&nbBytesWritten,  // number of bytes written
			NULL        // overlapped buffer
		);
		CloseHandle(hFile);
	}*/
 #else 
	//va_list argptr;
	//va_start(argptr, fmt);
	//va_end(argptr);
	UNREFERENCED_PARAMETER(address);
	UNREFERENCED_PARAMETER(event);
	UNREFERENCED_PARAMETER(name);
	UNREFERENCED_PARAMETER(fmt);
 #endif //NATSOCKET_DEBUG
}

///////////////////////////////////////////////////////////////////////////////
// CUDPSocketWnd

/*#define NAT_TIMER_INTERVALS 100 // Ms

BEGIN_MESSAGE_MAP(CNATSocketWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
END_MESSAGE_MAP()

int CNATSocketWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	VERIFY( (m_timer = SetTimer(301, NAT_TIMER_INTERVALS, 0)) != NULL );
	return 0;
}

void CNATSocketWnd::OnDestroy()
{
	if (m_timer){
		KillTimer(m_timer);
		m_timer = 0;
	}
}

void CNATSocketWnd::OnTimer(UINT /nIDEvent/)
{
	m_pOwner->CheckForTimeOut();
}

*/

///////////////////////////////////////////////////////////////////////////////
// CNatSocket

/**
* The constructor creates the socket.
*/
CNatSocket::CNatSocket()
{
	NATTrace((uint32)this,NLE_OTHER,__FUNCTION__,"Create new NAT Socket");

	//VERIFY( m_natwnd.CreateEx(0, AfxRegisterWndClass(0), StrLine(_T("eMule NAT Socket Wnd #%u"),(uint32)this), WS_OVERLAPPED, 0, 0, 0, 0, NULL, NULL));
	//m_natwnd.m_pOwner = this;

	//AsyncLayer
	SetNatLayerState(unconnected); // we have a socket but it's not connected yet

	//config
	m_uIP = 0;
	m_uPort = 0;
	m_uVersion = 0;
	m_uMaxSegmentSize = 0;
	// default buffer sizes
	m_uMaxDownloadBufferSize = 16*1024;
	m_uMaxUploadBufferSize = 8*1024;

	// UpStream
	m_uEstimatedRTT = 0;
	m_uTimeOutDeviation = 0;
	m_uActualTimeOut = 0;
//#if defined(REGULAR_TCP)
//	m_uDuplicatedACKCount = 0;
//#endif

	m_uCongestionWindow = 0;
	m_uAdvertisedWindow = 0;
	m_uPendingWindowSize = 0;

	m_uLastSentSequenceNr = 0;
	m_uUploadBufferSize = 0;

	// DownStream
	m_uLastAcknowledgeSequenceNr = 0;
//#if defined(REGULAR_TCP)
//	m_bSegmentMissing = false;
//#endif
	m_uCurrentSegmentPosition = 0;
	m_uDownloadBufferSize = 0;

	// other
	m_ShutDown = 0;

	m_Tunnel = NULL; // NEO: NATT - [NatTraversal]
}

/**
* The destructor deletes the socket.
*/
CNatSocket::~CNatSocket() 
{
	NATTrace((uint32)this,NLE_OTHER,__FUNCTION__,"Destroy NAT Socket");

	theApp.natmanager->RemoveSocket(this);

	//m_natwnd.DestroyWindow();

	m_UpLocker.Lock();
	for(POSITION pos = m_UploadBuffer.GetTailPosition();pos;)
		delete m_UploadBuffer.GetPrev(pos)->m_value;
	m_UpLocker.Unlock();

	m_DownLocker.Lock();
	for(POSITION pos = m_DownloadBuffer.GetTailPosition();pos;)
		delete m_DownloadBuffer.GetPrev(pos)->m_value;
	m_DownLocker.Unlock();
}

/**
* Init Initialise the socket with default values and makes it ready to operate.
*
* @param Version: version of the remote clients UserModeTCP.
*
* @param MSS: rempte cleints max segment size. (obtional)
*/
void CNatSocket::Init(uint8 uVersion, uint32 uMSS)
{
	m_uVersion = uVersion;
	m_uMaxSegmentSize = uMSS;

	ASSERT(m_uVersion && m_uIP && m_uPort);

	if( m_uMaxSegmentSize == 0 // this information is obtional it musn't be send by the remote cleint
	 || m_uMaxSegmentSize > MAXFRAGSIZE) // we always take the smaller one
		m_uMaxSegmentSize = MAXFRAGSIZE;
	// Note: we have 8 bytes UDP overhead 2 byted emule overhead and 4 byted nat, a normal TCP have 20 bytes as long ;)

	// Note: always allow to send one segment at startup
	m_uAdvertisedWindow = m_uMaxSegmentSize; 
	m_uCongestionWindow = m_uAdvertisedWindow;

	m_uCongestionThreshold = m_uMaxUploadBufferSize; // slow start ;)

	NATTrace((uint32)this,NLE_CONFIGURE,__FUNCTION__, "IP %s, MSS %u", CStringA(ipstr(m_uIP,m_uPort)), m_uMaxSegmentSize);
}

/**
* SetIPPort makes the socket connectable and adds it on the natmanager's socket list.
*
* @param IP: ip of the socket.
*
* @param Port: udp port of the socket.
*/
void CNatSocket::SetIPPort(uint32 IP, uint16 Port) 
{
	m_uIP = IP; 
	m_uPort = Port;
	theApp.natmanager->AddSocket(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Upload implementation //
///////////////////////////

/**
* Send is calles from CEMSocket like for a usual TCP socket,
*   The function copyes the data to the socket intern upload buffer and splits tham in segments,
*   every segment gets an uniqie segment number.
*
* Synchronisation note: This function locks the Upload Buffer Mutex.
*
* @param lpBuf: pointer on the data buffer to read.
*
* @param nBufLen: buffer length.
*
* @param nFlags: unused yet.
*/
int CNatSocket::Send(const void* lpBuf, int nBufLen, int /*nFlags*/)
{
	// NOTE: *** This function is invoked from a *different* thread!
	
	NATTrace((uint32)this,NLE_FUNCTION,__FUNCTION__,"nBufLen: %i", nBufLen);

	ASSERT(IsInit());

	if(!IsInit()){
		WSASetLastError(WSAENOTCONN);
		NATTrace((uint32)this,NLE_RESULT,__FUNCTION__, "socket is *not* configured!");
		return SOCKET_ERROR;
	}

	if(m_uUploadBufferSize + min((unsigned)nBufLen,m_uMaxSegmentSize) > m_uMaxUploadBufferSize){
		WSASetLastError(WSAEWOULDBLOCK);
		return SOCKET_ERROR;
	}

	uint32 toSend = min((unsigned)nBufLen, m_uMaxUploadBufferSize - m_uUploadBufferSize);
	
	m_UpLocker.Lock();

	uint32 Sent = 0;
	while(toSend > Sent) 
	{
		uint32 uSequenceNr;
		sTransferBufferEntry* BufferEntry;
		POSITION pos = m_UploadBuffer.GetHeadPosition();	// Note: on a CRBMap this points always on the object with the lowest key, 
															//		 bu becouse we rever the map keys (mapkey = UINT_MAX-key) 
		if(pos){ 											//		 it points always on the entry with the highest Sequence Number
			BufferEntry = m_UploadBuffer.GetValueAt(pos);
			if(BufferEntry->uSize < m_uMaxSegmentSize && BufferEntry->uSendCount == 0){ // if this segment is not full and wsn't already sent, fill it up
				uSequenceNr = 0;
			}else{ // create a new segment
				uSequenceNr = max(BufferEntry->uSequenceNr,m_uLastSentSequenceNr)+1; // the buffered segments may already been have sended and are out ou ofrder now
				BufferEntry = NULL;
			}
		}else{ // no pending segments
			uSequenceNr = m_uLastSentSequenceNr+1;
			BufferEntry = NULL;
		}

		//--------------------------------
		// We Split the data into segments
		//--------------------------------

		uint32 Size = min((toSend - Sent),m_uMaxSegmentSize);
		if(BufferEntry == NULL){
			NATTrace((uint32)this,NLE_WRITE,__FUNCTION__, "Buffer new segment; Size: %u, uSequenceNr: %u", Size, uSequenceNr);
			BufferEntry = new sTransferBufferEntry(Size, (const BYTE*)lpBuf+Sent, uSequenceNr);
			m_UploadBuffer.SetAt(UINT_MAX-uSequenceNr, BufferEntry);
		}else{ // to save overhead we try to send only full segments when there are some not full bufferes
			Size = min(Size, m_uMaxSegmentSize - BufferEntry->uSize);
			NATTrace((uint32)this,NLE_WRITE,__FUNCTION__, "Append to exiting segment; Old Size: %u, New Size: %u, uSequenceNr: %u",
													BufferEntry->uSize, BufferEntry->uSize + Size, BufferEntry->uSequenceNr);
			uint32 tmpSize = BufferEntry->uSize;
			BYTE* tmpBuffer = BufferEntry->pBuffer;
			BufferEntry->uSize += Size;
			BufferEntry->pBuffer = new BYTE[BufferEntry->uSize];
			memcpy(BufferEntry->pBuffer,tmpBuffer,tmpSize);
			memcpy((BYTE*)BufferEntry->pBuffer+tmpSize,(const BYTE*)lpBuf+Sent,Size);
			delete [] tmpBuffer;
		}
		
		Sent += Size;

		// increment our send buffer size
		m_uUploadBufferSize += Size;
	}

	NATTrace((uint32)this,NLE_RESULT,__FUNCTION__, "Sent: %u", Sent);

	//--------------
	// Send Segments
	//--------------

	ShrinkCongestionWindow();
	TryToSend();

	m_UpLocker.Unlock();

	return Sent; // return the number if bytes we actualy tooken
}

/**
* TryToSend is called from the Send (BT thread) and from ProcessAckPacket(main thread).
*   It tryes to send as much segments as fit in the uEffectiveWindow whitch is the minumum of m_uCongestionWindow and m_uAdvertisedWindow.
*   The function returns when the window is full or no unsent more segments are storren in the buffer,
*
* Synchronisation note: This function calles SendBufferedSegment, look on Sync note of this function for furder informations
*
*/
void CNatSocket::TryToSend()
{
	// NOTE: This function is invoked from a *different* thread! But all places where it is called lock already

	NATTrace((uint32)this,NLE_FUNCTION,__FUNCTION__,"m_uLastSentSequenceNr: %u, m_uCongestionWindow: %u m_uAdvertisedWindow: %u, m_uPendingWindowSize: %u", 
									m_uLastSentSequenceNr, m_uCongestionWindow, m_uAdvertisedWindow, m_uPendingWindowSize);

	//suint32 uMaxWindow = min(m_uCongestionWindow, m_uAdvertisedWindow);
	//suint32 uEffectiveWindow = uMaxWindow - m_uPendingWindowSize;
	sTransferBufferEntry* BufferEntry;
	// Note: when we have an EffectiveWindow we can already send an entier segment
	while(min(m_uCongestionWindow, m_uAdvertisedWindow) > m_uPendingWindowSize) 
	{
		if(m_UploadBuffer.Lookup(UINT_MAX-(m_uLastSentSequenceNr+1),BufferEntry)) // get the next segment to send
		{
			m_uLastSentSequenceNr++;
			m_uPendingWindowSize += BufferEntry->uSize; // m_uMaxSegmentSize;
			SendBufferedSegment(BufferEntry);
		}
		else
			break; // no more unsent datas in buffer
	}

	NATTrace((uint32)this,NLE_RESULT,__FUNCTION__,"m_uLastSentSequenceNr: %u, uEffectiveWindow: %u, m_uPendingWindowSize: %u", 
									m_uLastSentSequenceNr, min(m_uCongestionWindow, m_uAdvertisedWindow),m_uPendingWindowSize);
}

/**
* SendBufferedSegment punts a single segment (BufferEntry) on the UDP socket's send queue.
*
* Synchronisation note: This function unlockes the Upload Buffer Mutex and after the packet was put on the udp queue locks it again.
*                                This means we must enter this function always with locked mutex and we exit it always with locked mutex.
*                                If one of the calling functions (CheckForTimeOut) was looping throu the Upload buffer by using POSITION,
*                                it must abbort the loop after this function was executed, as the main thread may have freed some buffer entries.
*
* @param BufferEntry: pointer to the segment whitch is to be sent.
*/
void CNatSocket::SendBufferedSegment(sTransferBufferEntry* BufferEntry)
{
	// NOTE: This function is invoked from a *different* thread! But all places where it is called are (and must be) locked already

	NATTrace((uint32)this,NLE_FUNCTION,__FUNCTION__,"Segment Size: %u, uSequenceNr: %u", 
									BufferEntry ? BufferEntry->uSize : 0, BufferEntry ? BufferEntry->uSequenceNr : 0);

	if(m_ShutDown & 0x02)
		return; // socket is shuting down
	if(GetLayerState() != connected) // socket is not connected
		return;

	char* pBuffer;
	uint32 uSize;

	if(BufferEntry)
	{
		BufferEntry->uSendCount++;
		BufferEntry->uSendTime = ::GetTickCount();

		uSize = BufferEntry->uSize + 4;// segment size + 4 bytes sequence number, there are also 2 bytes ed2k opcodes thiere are irrleevant here
		pBuffer = new char[uSize];

		PokeUInt32(pBuffer, BufferEntry->uSequenceNr); // segment sequence number
		// Note: we don't need to write the length as the UDP protocol already take care about the size
		memcpy(pBuffer+4,BufferEntry->pBuffer,BufferEntry->uSize);
	}
	else // if the last recived Advertised Window size is 0 and we have datas in buffer we ping with empty segments to get a new window size
			// Note: regular TCP sendy a minimal data segment != 0 expecting it to be dropped is window is still 0
	{
		uSize = 4;
		pBuffer = new char[uSize];

		PokeUInt32(pBuffer, 0);
	}

	NATTrace((uint32)this,NLE_SENT,__FUNCTION__, "uSequenceNr: %u", BufferEntry ? BufferEntry->uSequenceNr : 0);

	// send packet
	m_UpLocker.Unlock(); // *** We must unlock so that SendPacket does not block our UploadBandwidth Trothler (with the official BW trothler this is not needed)
	theApp.natmanager->SendUmTCPPacket(this,pBuffer,uSize,OP_NAT_DATA);
	m_UpLocker.Lock(); // *** when we entered this function we already was locked so relock

	NATTrace((uint32)this,NLE_RESULT,__FUNCTION__, "done");
}

/**
* ProcessAckPacket is colled by the udp socket (main thread) when an NAT ACK packet was recived.
*   The function free's the acknowledged buffer space and calls TryToSend to put new segments on the network.
*    In the old TCP implementation it also handles the fast Retransmission
*   The ACK may have the uSequenceNr = 0 this means it's only an answer on a ping whitch privides us a new m_uAdvertisedWindow > 0
*
* @param packet: pointer to the recived packet buffer.
*
* @param size: recived packet buffer size.
*/
void CNatSocket::ProcessAckPacket(const BYTE* packet, UINT size)
{
	ASSERT(IsInit());

	ASSERT(size == 8);

	m_UpLocker.Lock();

	// read the sequence number of the last acknowledge segment
	uint32 uSequenceNr = PeekUInt32(packet); 

	// read the actual Advertised Window size
	m_uAdvertisedWindow = PeekUInt32(packet+4);

	NATTrace((uint32)this,NLE_FUNCTION,__FUNCTION__,"uSequenceNr: %u, m_uAdvertisedWindow: %u", uSequenceNr, m_uAdvertisedWindow);

	sTransferBufferEntry* BufferEntry;
	if(uSequenceNr == 0)
	{
		// Note: ACK's with sequence number 0 indicates us that the m_uAdvertisedWindow of the remote side is open again
		// the remote side may sendit without being asked or when it recives a data segment with 0 data size
		ASSERT(m_uAdvertisedWindow > 0);
	}
	else if(m_UploadBuffer.Lookup(UINT_MAX-uSequenceNr,BufferEntry))
	{
		// --------------------------------
		// Congestion Window Incrementation
		// --------------------------------
		uint32 WindowIncrement = m_uMaxSegmentSize; // exponenial increase
		if(m_uCongestionWindow > m_uCongestionThreshold)
			WindowIncrement = WindowIncrement * WindowIncrement / m_uCongestionWindow; // additiv increase
		m_uCongestionWindow = min(m_uCongestionWindow + WindowIncrement, m_uMaxUploadBufferSize);

//#if defined(REGULAR_TCP)
//		m_uDuplicatedACKCount = 0;
//		uint16 uAcknowledgeSegments = 0;
//#endif

		// --------------------------
		// Packet Timing Calculations
		// --------------------------
		ASSERT(BufferEntry->uSendCount);
		if(BufferEntry->uSendCount == 1) // w ignore resent packets
		{
			ASSERT(::GetTickCount() >= BufferEntry->uSendTime);

			int uSampleRTT = ::GetTickCount() - BufferEntry->uSendTime; // Must be a signed value !
			if(m_uEstimatedRTT == 0) // its the verry first ACK we got
			{
				m_uEstimatedRTT = uSampleRTT;
				m_uTimeOutDeviation = 0;
				m_uActualTimeOut = uSampleRTT * 2; // better a to log timeout than a to short one
			}
			else // calculate the proper TimeOut and average RTT
			{
				// RTT and TimeOut calculation
				uSampleRTT -= /*(*/m_uEstimatedRTT /*>> 3)*/;
				m_uEstimatedRTT = ((m_uEstimatedRTT << 3) + uSampleRTT) >> 3; //m_uEstimatedRTT += uSampleRTT;
				if (uSampleRTT < 0)
					uSampleRTT = -uSampleRTT;
				uSampleRTT -= (m_uTimeOutDeviation >> 3);
				m_uTimeOutDeviation += uSampleRTT;
				m_uActualTimeOut = /*(*/m_uEstimatedRTT /*>> 3)*/ + (m_uTimeOutDeviation >> 1);
			}
			NATTrace((uint32)this,NLE_STATE,"Timing Calculations","uSampleRTT: %u, m_uEstimatedRTT: %u, m_uActualTimeOut: %u, m_uTimeOutDeviation: %u,"
							, ::GetTickCount() - BufferEntry->uSendTime, m_uEstimatedRTT, m_uActualTimeOut, m_uTimeOutDeviation);
		}

		// -----------------------------------
		// Clear buffer from acknowledned data
		// -----------------------------------

//#if !defined(REGULAR_TCP)
		m_UploadBuffer.RemoveKey(UINT_MAX-uSequenceNr);

		// data have left the network
		m_uPendingWindowSize -= BufferEntry->uSize; //m_uMaxSegmentSize;

		// remove packet from buffer
		m_uUploadBufferSize -= BufferEntry->uSize;
		delete BufferEntry;
//#endif

//#if defined(REGULAR_TCP)
//		for (uint32 i=UINT_MAX-uSequenceNr; i< UINT_MAX; i++){
//			POSITION pos = m_UploadBuffer.FindFirstKeyAfter(i);
//			if(pos == NULL)
//				break; // no more data in buffer
//
//			uAcknowledgeSegments++;
//
//			BufferEntry = m_UploadBuffer.GetValueAt(pos);
//			m_UploadBuffer.RemoveAt(pos);
//
//			// data have left the network
//			m_uPendingWindowSize -= BufferEntry->uSize; //m_uMaxSegmentSize;
//
//			// remove packet from buffer
//			m_uUploadBufferSize -= BufferEntry->uSize;
//			delete BufferEntry;
//		}
//
//		NATTrace((uint32)this,NLE_OTHER,__FUNCTION__, "uAcknowledgeSegments %u", uAcknowledgeSegments);
//#endif
		// we have removed at least one segment from our Upload buffer
		m_UpLocker.Unlock();
		OnSend(0); // so we notify the CEMsocket that he can give use new data
		m_UpLocker.Lock();
	}
//#if defined(REGULAR_TCP)
//	else // this may happen in 2 cases; 
//		 // 1 the ack's just arrived out of order and the data's are already confirmed
//		 // 2 its the 2nd or 3rd ack with the same sequence number indicating that the reciver miss the next segment
//	if(m_UploadBuffer.Lookup(UINT_MAX-(uSequenceNr+1),BufferEntry)) // If the next segment this ack is pointing on exists than it is not an out of order ack
//	{
//		// -------------------
//		// Fast Retransmission
//		// -------------------
//		m_uDuplicatedACKCount++;
//		if(m_uDuplicatedACKCount >= 3)
//		{
//			// Half the Congestion Window
//			m_uCongestionWindow = max(m_uMaxSegmentSize, m_uCongestionWindow/2); // multiplicative decrease
//			m_uCongestionThreshold = m_uCongestionWindow; // fast recovery
//
//			// Note: we don't increase m_uPendingWindowSize yet as we assume the other packet is vanished
//			SendBufferedSegment(BufferEntry);
//
//			NATTrace((uint32)this,NLE_OTHER,__FUNCTION__, "Resent Segment %u", BufferEntry->uSequenceNr);
//			NATTrace((uint32)this,NLE_RESULT,__FUNCTION__, "done");
//
//			m_UpLocker.Unlock();
//			return;
//		}
//
//		NATTrace((uint32)this,NLE_OTHER,__FUNCTION__, "m_uDuplicatedACKCount %u", m_uDuplicatedACKCount);
//	}
//
//	ShrinkCongestionWindow(/*uAcknowledgeSegments*/); // dont allow the window to grow rapidly to large
//#endif

	// ---------------------------------
	// Self clocking - send next Segment
	// ---------------------------------

//#if defined(REGULAR_TCP)
//	if(m_uDuplicatedACKCount == 0)
//#endif
		TryToSend();

	NATTrace((uint32)this,NLE_RESULT,__FUNCTION__, "done");

	m_UpLocker.Unlock();
}

/**
* ShrinkCongestionWindow should prevent larget bursts of packet when due to a transmission pause the Effective Window groes to large
*
* @param uAcknowledgeSegments: unused yet.
*/
void CNatSocket::ShrinkCongestionWindow(/*uint16 uAcknowledgeSegments*/)
{
	// We use this function to Shrink the current Congestion Window 
	//  in cases when we are afraid of causing an larger segment outburst.
	//    //When we got a cumulated ACK for more than 2 segments <- this was solved with the new TCP fast restransmission
	//    And in case we havn't sent any data and the Pending Window goes empty
	//  in such cases we allow to send one segment 
	// If we fall below the Congestion Threshold 
	//   we will use exponenial increase to restore our previuse state
	if(/*uAcknowledgeSegments >= 3 ||*/ m_uCongestionWindow / 2 > max(m_uPendingWindowSize,m_uMaxSegmentSize * 2)) 
		m_uCongestionWindow = m_uPendingWindowSize + m_uMaxSegmentSize; 
}

/**
* The CheckForTimeOut function take care about resending lost segments.
*   In the new implementation this function is used also for fast retransmission.
* CheckForTimeOut is called form the main thread 10 times per secund.
* The function also take care of pinging with 0 size segments when the advertised window becomes 0.
*
* @param bTimed: is = true once per secund
*/
void CNatSocket::CheckForTimeOut()
{
	if(!IsInit())
		return; // socket is not redy yet

	if(m_UploadBuffer.GetTailPosition())
	NATTrace((uint32)this,NLE_FUNCTION,__FUNCTION__, "");

	m_UpLocker.Lock();

	sTransferBufferEntry* BufferEntry;
//#if !defined(REGULAR_TCP)
	uint32 uConfirmedSegments = 0;
	//uint32 uMissingACKCount = 0;
	uint32 uLastSequenceNr = 0;
//#endif

	// -------------------------
	// Check for Segment Timeout
	// -------------------------

	for(POSITION pos = m_UploadBuffer.GetTailPosition(); pos;)
	{
		BufferEntry = m_UploadBuffer.GetPrev(pos)->m_value;
		if(BufferEntry->uSendCount == 0) // was this segment already sent
			break; // since here all segments are 

//#if !defined(REGULAR_TCP)
		// In the new way by David Xanatos, we ack every segment we get, also when we miss the previus one.
		// We don't longer send duplicated ACK's
		// So the sender must detect in an other way that we miss a segment
		// Theoreticly its easy, but in practic it's a bit tricky
		if(uLastSequenceNr && uLastSequenceNr+1 != BufferEntry->uSequenceNr){
			uConfirmedSegments += BufferEntry->uSequenceNr - (uLastSequenceNr+1);
			//uMissingACKCount++;
		}
		uLastSequenceNr = BufferEntry->uSequenceNr;
//#endif

		// increas eth eeffective timeout for every resent packet
		if(BufferEntry->uSendTime + (m_uActualTimeOut*BufferEntry->uSendCount) < ::GetTickCount())
		{
			if(BufferEntry->uSendCount > max(3, m_uCongestionWindow/m_uMaxSegmentSize)) // to much fails
			{
				m_UpLocker.Unlock();
				if(::GetTickCount() - BufferEntry->uSendTime > SEC2MS(10)) // lets wait a while
				{
					NATTrace((uint32)this,NLE_RESULT,__FUNCTION__, "Terminating connection, 3 packet resend's failed");
					// we call this function yet only over a timer form the main thread so we can excute Reset() directiy here
					Reset(WSAETIMEDOUT);
				}
				else{
					NATTrace((uint32)this,NLE_RESULT,__FUNCTION__, "3 packet resend's failed, waiting");
				}
				return;
			}

			// -----------------------
			// Resend the Lost Segment
			// -----------------------

			// half the Congestion window
			m_uCongestionThreshold = max(m_uMaxSegmentSize, m_uCongestionWindow/2); // multiplicative decrease
			m_uCongestionWindow = m_uMaxSegmentSize; // reset the Congestion Window to one to avoid a sgment outburst

			NATTrace((uint32)this,NLE_OTHER,__FUNCTION__, "Segment ACK TimeOut: %u; resending segment %u", ::GetTickCount() - BufferEntry->uSendTime, BufferEntry->uSequenceNr);

			SendBufferedSegment(BufferEntry); // *** we unlock and relock the buffer mutex here inside, so we must exit this loop !!!

			m_UpLocker.Unlock();
			NATTrace((uint32)this,NLE_RESULT,__FUNCTION__, "done");
			return;
		}
		else if(BufferEntry->uSendCount > 1) // are we in progres of resending a apcket
			break; // don't resend more than this one (old implementation: when it arrives we may get a ACK for many following packts)
	}

//#if !defined(REGULAR_TCP)
	if(uConfirmedSegments >= 3) // we already have 3 or more confirmed segments but the first is still not confirmed, fast resend it
	{
		// -------------------
		// Fast Retransmission
		// -------------------

		BufferEntry = m_UploadBuffer.GetAt(m_UploadBuffer.GetTailPosition())->m_value; // if we are here we know that there is somethink in the buffer

		if(BufferEntry->uSendCount == 1) // we make fast resend only once
		{
			// half the Congestion window
			m_uCongestionWindow = max(m_uMaxSegmentSize, m_uCongestionWindow/2); // multiplicative decrease
			m_uCongestionThreshold = m_uCongestionWindow; // fast recovery

			// Note: we don't increase m_uPendingWindowSize yet as we assume the other packet is vanished
			SendBufferedSegment(BufferEntry);

			NATTrace((uint32)this,NLE_OTHER,__FUNCTION__, "Resent Segment %u", BufferEntry->uSequenceNr);
		}
	 }
//#endif

	if(m_UploadBuffer.GetTailPosition())
	NATTrace((uint32)this,NLE_RESULT,__FUNCTION__, "uConfirmedSegments %u, m_uAdvertisedWindow %u", uConfirmedSegments, m_uAdvertisedWindow);

	// -----------
	// Window Ping
	// -----------

	// if needed ping to get a new window size
	if(m_uAdvertisedWindow == 0 && m_uUploadBufferSize != 0) // and only if we have something to send
		SendBufferedSegment(NULL); // send the ping

	m_UpLocker.Unlock();
}

	
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Download implementation //
/////////////////////////////

/**
* ProcessDataPacket is colled by the udp socket (main thread) when an NAT DATA packet was recived.
* The function puts the new data into the download buffer and notifyes the CEMSocket that there are data to recive
* In case we get a empty data packet and we have a Recieving window != 0 we send an ACK with the number 0 and the new window size
*
* @param packet: pointer to the recived packet buffer.
*
* @param size: recived packet buffer size.
*/
void CNatSocket::ProcessDataPacket(const BYTE* packet, UINT size)
{
	ASSERT(IsInit());

	if(m_ShutDown & 0x01)
		return; // socket is shuting down
	if(GetLayerState() != connected) // socket is not connected
		return;

	uint32 uSequenceNr = PeekUInt32(packet);

	NATTrace((uint32)this,NLE_FUNCTION,__FUNCTION__,"uSequenceNr: %u, size: %u", uSequenceNr, size-4);

	if(uSequenceNr) // no Sequence Nrumber means this is a ping to get a new > 0 window size
	{
		//-------------------
		// Buffer new Segment
		//-------------------

		m_DownLocker.Lock();

		sTransferBufferEntry* BufferEntry;
		// Check is this Sequence Number valid, it may be a duplicated packet
		if(uSequenceNr <= m_uLastAcknowledgeSequenceNr || m_DownloadBuffer.Lookup(uSequenceNr,BufferEntry)){
			m_DownLocker.Unlock();
//#if !defined(REGULAR_TCP)
			// that's the down side of the new implementation, we must send an ACK for every segment
			SendAckPacket(uSequenceNr); // this may happen when our last ACK got lost, so resend it
//#endif
			return; // it's a duplicat just drop it
		}

		if(m_uDownloadBufferSize + (size-4) > m_uMaxDownloadBufferSize + m_uMaxSegmentSize // does it fit in our buffer, we allow one segment large overflow
		 && uSequenceNr != m_uLastAcknowledgeSequenceNr+1){ // if it is the next required segment we always have space for it
			m_DownLocker.Unlock();
			return; // Damn it! our buffer it full, we must dropp it :'(
		}

		// add the new packet to our download buffer
		m_DownloadBuffer.SetAt(uSequenceNr, new sTransferBufferEntry(size-4, packet+4, uSequenceNr));
		m_uDownloadBufferSize += (size-4);

		m_DownLocker.Unlock();
	}
	// its a ping to get a new window size
	else if(m_uMaxDownloadBufferSize <= m_uDownloadBufferSize) // answre only when the window size is not longer 0
		return; // windows ist still full just return

//#if !defined(REGULAR_TCP)
	// Note: in countrary to the reqular TCP we confirm every segment immidetly, this give's the sender a better overview over the situation
	SendAckPacket(uSequenceNr); // send a acknowledge packet
//#endif

	NATTrace((uint32)this,NLE_RESULT,__FUNCTION__,"done");

	// we have new data in buffer
	OnReceive(0); // notify the CEMsocket that it can resive data

//#if defined(REGULAR_TCP)
////	m_DownLocker.Lock();
//
//	// fast retransmission, we repeat the last ACK
//	if(m_bSegmentMissing && uSequenceNr != (m_uLastAcknowledgeSequenceNr+1))
//		SendAckPacket(m_uLastAcknowledgeSequenceNr); // send acknowledge packet
//
////	m_DownLocker.Unlock();
//#endif
}

/**
* Receive is calles from CEMSocket like for a usual TCP socket,
*   The function copyes the data segments from the socket intern download buffer into the, provided buffer.
*   The segments are reasembeld according to the sequence number if a number is missing function stops
*
* Synchronisation note: This function locks the Download Buffer Mutex.
*
* @param lpBuf: pointer on the data buffer to read.
*
* @param nBufLen: buffer length.
*
* @param nFlags: unused yet.
*/
int CNatSocket::Receive(void* lpBuf, int nBufLen, int /*nFlags*/)
{ 
	// NOTE: *** This function is invoked from a *different* thread!

	NATTrace((uint32)this,NLE_FUNCTION,__FUNCTION__,"nBufLen: %i", nBufLen);

	ASSERT(IsInit());

	if(!IsInit()){
		WSASetLastError(WSAENOTCONN);
		NATTrace((uint32)this,NLE_RESULT,__FUNCTION__, "socket is *not* configured!");
		return SOCKET_ERROR;
	}

	if(m_uDownloadBufferSize == 0){
		WSASetLastError(WSAEWOULDBLOCK);
		return SOCKET_ERROR;
	}

//#if defined(REGULAR_TCP)
//	m_bSegmentMissing = false; // reset the flag when the segment is still missing it will be set to true again
//#endif

	uint32 toRecive = min((unsigned)nBufLen, m_uDownloadBufferSize);

	m_DownLocker.Lock();

	uint32 Recived = 0;
	for(POSITION pos = m_DownloadBuffer.GetHeadPosition(); pos && toRecive > Recived; pos = m_DownloadBuffer.GetHeadPosition()) // the buffer is ordered by swquence numbers
	{
		sTransferBufferEntry* BufferEntry = m_DownloadBuffer.GetValueAt(pos);

		// ups! we miss a segment
		if(BufferEntry->uSequenceNr != (m_uLastAcknowledgeSequenceNr+1)) 
		{
//#if defined(REGULAR_TCP)
//			m_bSegmentMissing = true; // set the missing flag
//#endif
			break; // and break loop
		}

		//--------------------------------------
		// We may read the segment only partialy
		//--------------------------------------

		uint32 Size = min((toRecive - Recived),(BufferEntry->uSize - m_uCurrentSegmentPosition));
		memcpy((BYTE*)lpBuf+Recived, BufferEntry->pBuffer + m_uCurrentSegmentPosition, Size);
		Recived += Size;

		m_uCurrentSegmentPosition += Size;

		// If we ware able to read the netier segment
		if(m_uCurrentSegmentPosition == BufferEntry->uSize)
		{
			NATTrace((uint32)this,NLE_READ,__FUNCTION__, "Read segment; Size: %u, uSequenceNr: %u", BufferEntry->uSize, BufferEntry->uSequenceNr);

			m_uLastAcknowledgeSequenceNr++; // increment our counter

			m_uCurrentSegmentPosition = 0; // reset position

			// free our download buffer
			m_uDownloadBufferSize -= BufferEntry->uSize;
			m_DownloadBuffer.RemoveAt(pos); 
			delete BufferEntry;
		}else{
			NATTrace((uint32)this,NLE_READ,__FUNCTION__, "Read partialy segment; Size: %u, uSequenceNr: %u, Read %u", BufferEntry->uSize, BufferEntry->uSequenceNr, Size);
		}
	}

	m_DownLocker.Unlock();

	NATTrace((uint32)this,NLE_RESULT,__FUNCTION__, "Recived: %u; left segments %u", Recived, m_DownloadBuffer.GetCount());

//#if defined(REGULAR_TCP)
//	// if we have recived a segment
//	if(Recived && m_uCurrentSegmentPosition == 0) // and only when we recived it completly
//		SendAckPacket(m_uLastAcknowledgeSequenceNr); // we send a acknowledge packet for the last segment
//	// Note: like in the original tcp it may be a cumulated ACK for more than one segment
//	// in this way we avoid packet bursrs when our side blocked for a while
//#endif

	//m_DownLocker.Unlock();

	return Recived; // return the number if bytes we gave
}

/**
* SendAckPacket send's segment ACK packets, and calculates the advertised recieving window size.
*
* Synchronisation note: This function does nut use critical parameters so it must't be locked.
*
* @param uSequenceNr: number of the to acknowledge segment
*/
void CNatSocket::SendAckPacket(uint32 uSequenceNr)
{
	// NOTE: This function is called from a *different* thread! 

	NATTrace((uint32)this,NLE_FUNCTION,__FUNCTION__,"uSequenceNr: %u", uSequenceNr);

	const uint32 uSize = 8; // 4 bytes sequence number and 4 bytes window size
	char* pBuffer = new char[uSize];

	PokeUInt32(pBuffer, uSequenceNr); // ack sequence number

	//uint32 uNewWindowSize = m_uMaxDownloadBufferSize - m_uDownloadBufferSize; 
	if(m_uMaxDownloadBufferSize > m_uDownloadBufferSize) // Note: we may have decreased the buffer size so that we actualy buffer mor than the max size
		PokeUInt32(pBuffer+4, m_uMaxDownloadBufferSize - m_uDownloadBufferSize); // new reciving window size
	else
		PokeUInt32(pBuffer+4, 0); // or reciving window is full
	
	NATTrace((uint32)this,NLE_SENT,__FUNCTION__, "uSequenceNr: %u", uSequenceNr);
	// send packet
//	m_DownLocker.Unlock(); // we enter this function always unlocked
	theApp.natmanager->SendUmTCPPacket(this,pBuffer,uSize,OP_NAT_DATA_ACK);
//	m_DownLocker.Lock();

	NATTrace((uint32)this,NLE_RESULT,__FUNCTION__,"done");
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Other implementations //
///////////////////////////

/**
* Create is called by the CASyncSocketEx and must do some formal tasks.
*
* @param's: 
*/
BOOL CNatSocket::Create(UINT /*nSocketPort*/, int /*nSocketType*/, long lEvent, LPCSTR /*lpszSocketAddress*/)
{
	m_pOwnerSocket->AsyncSelect(lEvent);
	return TRUE;
}

/**
* Sync by the NatManager when a tunel have been established
*   The function sends out UserModeTCP version as well as our MAXFRAGSIZE to the remote client,
*   And requests a UserModeTCP connetion
*
* @param bAck: specyfyed if this is a connection attempt or a connection acknowledgement
*/
BOOL CNatSocket::Sync(bool bAck)
{
	ASSERT(IsSet());

	NATTrace((uint32)this,NLE_SENT,__FUNCTION__, "Nat Sync");

	if(GetLayerState() == (bAck ? unconnected : connecting))
	{
		const uint32 uSize = 5; // 1 byte version 4 bytes mss
		char* pBuffer = new char[uSize];
		PokeUInt8(pBuffer, 1); // version
		PokeUInt32(pBuffer+1, MAXFRAGSIZE); // max segment size (obtional)
		theApp.natmanager->SendUmTCPPacket(this,pBuffer,uSize,bAck ? OP_NAT_SYN_ACK : OP_NAT_SYN);

		NATTrace((uint32)this,NLE_RESULT,__FUNCTION__,"*already* connected!");
		return FALSE;
	}

	NATTrace((uint32)this,NLE_RESULT,__FUNCTION__,"done");

	return FALSE; 
}

/**
* Connect is called to establish a User Mode TCP connection when a nat tunnel is already working.
*
* @param's: not used
*/
BOOL CNatSocket::Connect(const SOCKADDR* /*lpSockAddr*/, int /*nSockAddrLen*/)
{
	NATTrace((uint32)this,NLE_FUNCTION,__FUNCTION__,"");

	ASSERT(IsSet());

	if(!IsSet()){
		NATTrace((uint32)this,NLE_RESULT,__FUNCTION__, "socket is *not* configured!");
		WSASetLastError(WSAEISCONN);
		return FALSE;
	}

	if(GetLayerState() == unconnected)
	{
		SetNatLayerState(connecting);

		Sync();

		NATTrace((uint32)this,NLE_RESULT,__FUNCTION__,"done");

		return TRUE;
	}
	
	NATTrace((uint32)this,NLE_RESULT,__FUNCTION__,"*already* connecting!");

	return FALSE;
}

/**
* Close is called to regulary close a connection.
*/
void CNatSocket::Close() 
{ 
	NATTrace((uint32)this,NLE_FUNCTION,__FUNCTION__,"");

	ASSERT(IsInit());

	if(!IsInit()){
		NATTrace((uint32)this,NLE_RESULT,__FUNCTION__, "socket is *not* configured!");
		return;
	}

	if(GetLayerState() == connected)
	{
		NATTrace((uint32)this,NLE_SENT,__FUNCTION__, "Nat Finish");
		theApp.natmanager->SendUmTCPPacket(this,NULL,0,OP_NAT_FIN);

		SetNatLayerState(closed);
	}

	NATTrace((uint32)this,NLE_RESULT,__FUNCTION__,"done");
}

/**
* Reset a problematic connection.
*
* @param nErrorCode: obtional error code
*/
void CNatSocket::Reset(int nErrorCode) 
{ 
	ASSERT(IsSet());

	if(!IsSet()){
		NATTrace((uint32)this,NLE_RESULT,__FUNCTION__, "socket is *not* configured!");
		return;
	}

	NATTrace((uint32)this,NLE_FUNCTION,__FUNCTION__,"");

	if(GetLayerState() != aborted)
	{
		NATTrace((uint32)this,NLE_SENT,__FUNCTION__, "Nat Reset");

		const uint32 uSize = 4;
		char* pBuffer = new char[uSize];
		PokeUInt32(pBuffer, nErrorCode);
		theApp.natmanager->SendUmTCPPacket(this,pBuffer,uSize,OP_NAT_RST);
		// Note: this packet may be empty

		SetNatLayerState(aborted);
		OnClose(nErrorCode);
		// CEMSocket::OnClose will call RemoveAllLayers what will result in the net socket being deleted and removed
		//theApp.natmanager->RemoveSocket(this);
	}

	NATTrace((uint32)this,NLE_RESULT,__FUNCTION__,"done");
}

//////////////////////////////////////////////////////
//		Regular TCP Connection Scheme
//
//	Client					Server
//		SYN		--->					// Establish Connection
//					<---	SYN+ACK		// Connection Established
//		ACK		--->
//				[......]
//		DATA	--->
//					<---	ACK
//				[......]
//					<--		DATA
//		ACK		-->
//				[......]
//		FIN		--->					// Close Connection
//					<---	FIN+ACK		// Connection Closed
//		ACK		--->
//
//				 [ OR ]
//					<---	RST			// Reset (Abort) Connection
//
//	We don't use the ack for the syn/fin ack.
//  On a syn we answer with syn_ack an thats all,
//  the same with fin.
//

/**
* ProcessPacket handless all incomming UserModeTCP packets
*
* @param packet: pointer to the recived packee
*
* @param size: size of the recived packet
*
* @param opcode: recived opcode
*/
void CNatSocket::ProcessPacket(const BYTE* packet, UINT size, uint8 opcode)
{
	NATTrace((uint32)this,NLE_FUNCTION,__FUNCTION__,"");

	switch(opcode)
	{
		case OP_NAT_SYN: // establish connection
		case OP_NAT_SYN_ACK:
		{
			NATTrace(0,NLE_RECIVED,__FUNCTION__,opcode == OP_NAT_SYN ? "Nat SYN Packet" : "Nat SYN_ACK Packet");

			if(size < 1) // version
				throw CString(_T("Nat SYN or SYN_ACK packet to short!!!"));

			uint8 uVersion = PeekUInt8(packet);
			uint32 uMSS = 0;
			if(size >= 5) // MSS (obtional)
				uMSS = PeekUInt32(packet+1);

			Init(uVersion,uMSS);

			if( opcode == OP_NAT_SYN )
				Sync(true);

			SetNatLayerState(connected);
			OnConnect(0);
			//if( opcode == OP_NAT_SYN_ACK ) // Async Socket Ex triggers OnSend also for incomming Connections
			OnSend(0); 
			break;
		}
		case OP_NAT_DATA:
		{
			NATTrace(0,NLE_RECIVED,__FUNCTION__,"Nat DATA Packet");

			if(size < 4) // SeqNr (+ DATA)
				throw CString(_T("Nat DATA packet to short!!!"));

			ProcessDataPacket(packet, size);
			break;
		}
		case OP_NAT_DATA_ACK:
		{
			NATTrace(0,NLE_RECIVED,__FUNCTION__,"Nat DATA_ACK Packet");

			if(size < 4+4) // SeqNr + Window
				throw CString(_T("Nat DATA_ACK packet to short!!!"));

			ProcessAckPacket(packet, size);
			break;
		}
		case OP_NAT_FIN: // close connection
		case OP_NAT_FIN_ACK:
		{
			NATTrace(0,NLE_RECIVED,__FUNCTION__,opcode == OP_NAT_FIN ? "Nat FIN Packet" : "Nat FIN_ACK Packet");

			// Note: if we recive FIN and the state is "closed" this means boot sides have tryed at the same time 
			//			to close the connection, that's ok we handle this situation correctly, 
			//			we don't send FIN_ACK becouse we already sent a FIN whitch will do all the tasks of FIN_ACK.
			//			If this FIN arrived and the state is not "connected", we ignore it and formaly trigger OnClose();
			if( opcode == OP_NAT_FIN && GetLayerState() == connected)
			{
				NATTrace((uint32)this,NLE_SENT,__FUNCTION__, "Nat Finish Acknowledged");
				theApp.natmanager->SendUmTCPPacket(this,NULL,0,OP_NAT_FIN_ACK);

				SetNatLayerState(closed);
			}
			OnClose(0);
			// CEMSocket::OnClose will call RemoveAllLayers what will result in the net socket being deleted and removed
			//theApp.natmanager->RemoveSocket(this);
			break;
		}
		case OP_NAT_RST: // abort connection
		{
			NATTrace(0,NLE_RECIVED,__FUNCTION__,"NAT connection reset");

			int nErrorCode = 0;
			if(size >= 4) // in debug versions a error code may be sent here
				nErrorCode = PeekUInt32(packet);

			SetNatLayerState(aborted);
			OnClose(nErrorCode);
			// CEMSocket::OnClose will call RemoveAllLayers what will result in the net socket being deleted and removed
			//theApp.natmanager->RemoveSocket(this);
			break;
		}
		default: 
			ASSERT(0); 
	}

	NATTrace((uint32)this,NLE_RESULT,__FUNCTION__,"done");
}

/**
* Shutdown the socket
*
* @param nHow: shutdown type
*/
BOOL CNatSocket::ShutDown(int nHow)
{
	m_ShutDown |= (uint8)(nHow+1);
		// receives = 0 -> 1 = 10
		// sends = 1    -> 2 = 01
		// both = 2	    -> 3 = 11
	return TRUE; 
}

/**
* Get remote peer address
*
* @param lpSockAddr: pointer to a SOCKADDR ctructure
*
* @param lpSockAddrLen: pointer to a integer size
*/
BOOL CNatSocket::GetPeerName(SOCKADDR* lpSockAddr, int* lpSockAddrLen)
{
	ASSERT(IsSet());

	ZeroMemory(lpSockAddr,*lpSockAddrLen);
	if(*lpSockAddrLen == sizeof(SOCKADDR_IN))
	{
		SOCKADDR_IN* sockAddr = (SOCKADDR_IN*)lpSockAddr;
		sockAddr->sin_addr.S_un.S_addr = m_uIP;
		sockAddr->sin_port = ntohs(m_uPort);
		return TRUE;
	}
	return FALSE;
}

/////////////
//Attributes

BOOL CNatSocket::GetSockOpt(int nOptionName, void* lpOptionValue, int* lpOptionLen)
{
	switch(nOptionName)
	{
	case TCP_NODELAY:
		if(lpOptionLen)
			*((char*)lpOptionValue) = 0; // This function is yet not implemented, and besides we don't need it anyway
		return TRUE;
	case SO_SNDBUF:
		if(*lpOptionLen < sizeof(uint32))
			return FALSE;
		memcpy(lpOptionValue,&m_uMaxUploadBufferSize,sizeof(uint32));
		return TRUE;
	case SO_RCVBUF:
		if(*lpOptionLen < sizeof(uint32))
			return FALSE;
		memcpy(lpOptionValue,&m_uMaxUploadBufferSize,sizeof(uint32));
		return TRUE;
	default:
		return FALSE;
	}
}

BOOL CNatSocket::SetSockOpt(int nOptionName, const void* lpOptionValue, int nOptionLen)
{
	switch(nOptionName)
	{
	case TCP_NODELAY:
		// I havn't implement the nagle Alghorytm becouse we don't need it, we usualy send's larger packets.
		// If you think we need this funtion anyway, fill free to implement it, it isn't so complicated.
		return FALSE;
	case SO_SNDBUF:
		memset(&m_uMaxUploadBufferSize,0,sizeof(uint32));
		memcpy(&m_uMaxUploadBufferSize,lpOptionValue,min(nOptionLen,sizeof(uint32)));
		return TRUE;
	case SO_RCVBUF:
		memset(&m_uMaxUploadBufferSize,0,sizeof(uint32));
		memcpy(&m_uMaxUploadBufferSize,lpOptionValue,min(nOptionLen,sizeof(uint32)));
		return TRUE;
	default:
		return FALSE;
	}
}

//Obfuscation support
bool CNatSocket::IsObfuscatedConnectionEstablished() const 
{ 
	return (m_Tunnel && m_Tunnel->ShouldReceiveCryptUDPPackets()); 
}

#endif //NATTUNNELING // NEO: UTCP END <-- Xanatos --
