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
#include "stdafx.h"
#ifdef _DEBUG
#include "DebugHelpers.h"
#endif
#include "emule.h"
#include "emsocket.h"
#include "AsyncProxySocketLayer.h"
#include "Packets.h"
#include "OtherFunctions.h"
#include "UploadBandwidthThrottler.h"
#include "Preferences.h"
#include "emuleDlg.h"
#include "Log.h"
//Xman
#include "updownclient.h" //Xman Xtreme Upload
#include "BandWidthControl.h" // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
#include "ListenSocket.h" //Xman 
// netfinity: Maximum Segment Size (MSS - Vista only) //added by zz_fly
#ifdef HAVE_VISTA_SDK
#include "NetF.h"
#endif
// netfinity: end

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


namespace {
	inline void EMTrace(char* fmt, ...) {
#ifdef EMSOCKET_DEBUG
		va_list argptr;
		char bufferline[512];
		va_start(argptr, fmt);
		_vsnprintf(bufferline, 512, fmt, argptr);
		va_end(argptr);
		//(Ornis+)
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
		//va_list argptr;
		//va_start(argptr, fmt);
		//va_end(argptr);
		UNREFERENCED_PARAMETER(fmt);
#endif //EMSOCKET_DEBUG
	}
}

IMPLEMENT_DYNAMIC(CEMSocket, CEncryptedStreamSocket)

CEMSocket::CEMSocket(void){
	byConnected = ES_NOTCONNECTED;
	m_uTimeOut = CONNECTION_TIMEOUT; // default timeout for ed2k sockets

	// Download (pseudo) rate control	
	downloadLimit = 0;
	downloadLimitEnable = false;
	pendingOnReceive = false;

	// Download partial header
	pendingHeaderSize = 0;

	// Download partial packet
	pendingPacket = NULL;
	pendingPacketSize = 0;

	// Upload control
	sendbuffer = NULL;
	sendblen = 0;
	sent = 0;
	//m_bLinkedPackets = false;

	// deadlake PROXYSUPPORT
	m_pProxyLayer = NULL;
	m_bProxyConnectFailed = false;

    //m_startSendTick = 0;
    //m_lastSendLatency = 0;
    //m_latency_sum = 0;
    //m_wasBlocked = false;

	// ==> Dynamic Socket Buffering [SiRoB] - Mephisto
#ifdef DONT_USE_SOCKET_BUFFERING
    m_currentPacket_is_controlpacket = false;
	m_currentPackageIsFromPartFile = false;
#endif
	// <== Dynamic Socket Buffering [SiRoB] - Mephisto

    m_numberOfSentBytesCompleteFile = 0;
    m_numberOfSentBytesPartFile = 0;
    //Xman unused
    /*
    m_numberOfSentBytesControlPacket = 0;
    */
    //Xman end

    lastCalledSend = ::GetTickCount();
    lastSent = ::GetTickCount()-1000;

	//Xman unused
	/*
	m_bAccelerateUpload = false;
	*/
	//Xman end

	// ==> Dynamic Socket Buffering [SiRoB] - Mephisto
#ifdef DONT_USE_SOCKET_BUFFERING
    m_actualPayloadSize = 0;
#endif
	// <== Dynamic Socket Buffering [SiRoB] - Mephisto
    m_actualPayloadSizeSent = 0;

    m_bBusy = false;
    m_hasSent = false;
	// ==> Dynamic Socket Buffering [SiRoB] - Mephisto
#ifdef DONT_USE_SOCKET_BUFFERING
	m_bUsesBigSendBuffers = false;
#endif
	// <== Dynamic Socket Buffering [SiRoB] - Mephisto

	//Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	client = NULL; // Quick and dirty

	//Xman Code Improvement
	isreadyforsending=false;

	//Xman count block/success send
	blockedsendcount=0;
	sendcount=0;
	blockedsendcount_overall=0;
	sendcount_overall=0;
	avg_block_ratio=0;
	sum_blockhistory=0;
	//Xman end

	// netfinity: Maximum Segment Size (MSS - Vista only) //added by zz_fly
	m_dwMSS = 0;

	// ==> Dynamic Socket Buffering [SiRoB] - Mephisto
#ifndef DONT_USE_SOCKET_BUFFERING
	sendblenWithoutControlPacket = 0;
	currentBufferSize = 0;

	m_uCurrentSendBufferSize = 0;
	m_uCurrentRecvBufferSize = 0;
#endif
	// <== Dynamic Socket Buffering [SiRoB] - Mephisto
}

CEMSocket::~CEMSocket(){
	EMTrace("CEMSocket::~CEMSocket() on %d",(SOCKET)this);

    // need to be locked here to know that the other methods
    // won't be in the middle of things
    sendLocker.Lock();
	byConnected = ES_DISCONNECTED;
    sendLocker.Unlock();

    // now that we know no other method will keep adding to the queue
    // we can remove ourself from the queue
    theApp.uploadBandwidthThrottler->RemoveFromAllQueues(this);

    ClearQueues();
	RemoveAllLayers(); // deadlake PROXYSUPPORT
	AsyncSelect(0);
	//Xman
	// Maella -Accurate download rate measurement directly at socket-
	if(client != NULL && (CEMSocket*)client->socket == this){ 
		// They might have an error in the cross link somewhere
		//Xman only for test
		AddDebugLogLine(false,_T("emsocket destructor exception"));
		client->socket = NULL;		
	}
	client = NULL;
	// Maella end
}

// deadlake PROXYSUPPORT
// By Maverick: Connection initialisition is done by class itself
BOOL CEMSocket::Connect(LPCSTR lpszHostAddress, UINT nHostPort)
{
	InitProxySupport();
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	theApp.pBandWidthControl->AddeMuleOutTCPOverall(0); // SYN
	// Maella end	
	return CEncryptedStreamSocket::Connect(lpszHostAddress, nHostPort);
}
// end deadlake

// deadlake PROXYSUPPORT
// By Maverick: Connection initialisition is done by class itself
//BOOL CEMSocket::Connect(LPCTSTR lpszHostAddress, UINT nHostPort)
BOOL CEMSocket::Connect(SOCKADDR* pSockAddr, int iSockAddrLen)
{
	InitProxySupport();
	return CEncryptedStreamSocket::Connect(pSockAddr, iSockAddrLen);
}
// end deadlake

void CEMSocket::InitProxySupport()
{
	m_bProxyConnectFailed = false;

	// ProxyInitialisation
	const ProxySettings& settings = thePrefs.GetProxySettings();
	if (settings.UseProxy && settings.type != PROXYTYPE_NOPROXY)
	{
		Close();

		m_pProxyLayer = new CAsyncProxySocketLayer;
		switch (settings.type)
		{
			case PROXYTYPE_SOCKS4:
			case PROXYTYPE_SOCKS4A:
				m_pProxyLayer->SetProxy(settings.type, settings.name, settings.port);
				break;
			case PROXYTYPE_SOCKS5:
			case PROXYTYPE_HTTP10:
			case PROXYTYPE_HTTP11:
				if (settings.EnablePassword)
					m_pProxyLayer->SetProxy(settings.type, settings.name, settings.port, settings.user, settings.password);
				else
					m_pProxyLayer->SetProxy(settings.type, settings.name, settings.port);
				break;
			default:
				ASSERT(0);
		}
		AddLayer(m_pProxyLayer);

		// Connection Initialisation
		Create(0, SOCK_STREAM, FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE, thePrefs.GetBindAddrA());
		AsyncSelect(FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE);
	}
}

void CEMSocket::ClearQueues(){
	EMTrace("CEMSocket::ClearQueues on %d",(SOCKET)this);

	sendLocker.Lock();
	for(POSITION pos = controlpacket_queue.GetHeadPosition(); pos != NULL; )
		delete controlpacket_queue.GetNext(pos);
	controlpacket_queue.RemoveAll();

	for(POSITION pos = standartpacket_queue.GetHeadPosition(); pos != NULL; )
		delete standartpacket_queue.GetNext(pos).packet;
	standartpacket_queue.RemoveAll();

	// ==> Dynamic Socket Buffering [SiRoB] - Mephisto
#ifndef DONT_USE_SOCKET_BUFFERING
	sendblenWithoutControlPacket = 0;
	for (POSITION pos = m_currentPacket_in_buffer_list.GetHeadPosition(); pos != NULL;) {
		delete m_currentPacket_in_buffer_list.GetNext(pos);
	}
	m_currentPacket_in_buffer_list.RemoveAll();

	sendblenWithoutControlPacket = 0;
#endif
	// <== Dynamic Socket Buffering [SiRoB] - Mephisto

    //Xman
    /*
    sendLocker.Unlock();
    */
    //Xman end

	// Download (pseudo) rate control	
	downloadLimit = 0;
	downloadLimitEnable = false;
	pendingOnReceive = false;

	// Download partial header
	pendingHeaderSize = 0;

	// Download partial packet
	delete pendingPacket;
	pendingPacket = NULL;
	pendingPacketSize = 0;

	// Upload control
	delete[] sendbuffer;
	sendbuffer = NULL;

	sendblen = 0;
	sent = 0;

	sendLocker.Unlock(); //Xman
}

//Xman
void CEMSocket::OnConnect(int nErrorCode){
	if(nErrorCode == 0){
		// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
		theApp.pBandWidthControl->AddeMuleSYNACK();
		// Maella end

	}
	CAsyncSocketEx::OnConnect(nErrorCode); // deadlake PROXYSUPPORT - changed to AsyncSocketEx
}
//Xman end

void CEMSocket::OnClose(int nErrorCode){
    // need to be locked here to know that the other methods
    // won't be in the middle of things
    sendLocker.Lock();
	byConnected = ES_DISCONNECTED;
    sendLocker.Unlock();

    // now that we know no other method will keep adding to the queue
    // we can remove ourself from the queue
    theApp.uploadBandwidthThrottler->RemoveFromAllQueues(this);

	CEncryptedStreamSocket::OnClose(nErrorCode); // deadlake changed socket to PROXYSUPPORT ( AsyncSocketEx )
	RemoveAllLayers(); // deadlake PROXYSUPPORT
	ClearQueues();
	//Xman
	if(nErrorCode == 0){
		// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
		theApp.pBandWidthControl->AddeMuleSYNACK();
		// Maella end
	}
	//Xman end
}

BOOL CEMSocket::AsyncSelect(long lEvent){
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
		return CEncryptedStreamSocket::AsyncSelect(lEvent);
	return true;
}

void CEMSocket::OnReceive(int nErrorCode){
	// the 2 meg size was taken from another place
	//Xman include ACK
	/*
	static char GlobalReadBuffer[2000000];
	*/
	//Xman end

	// Check for an error code
	//zz_fly
	//netfinity: Special case when socket is closing but data still in buffer, need to empty buffer or deadlock forever
	/*
	if(nErrorCode != 0){
	*/
	if(nErrorCode != 0 && nErrorCode != WSAESHUTDOWN){ 
	//zz_fly end
		OnError(nErrorCode);
		return;
	}

	//Xman -Reask sources after IP change- v4
	if(pendingOnReceive==false)
		theApp.last_traffic_reception=::GetTickCount(); //Threading Info: synchronized with the main thread
	//Xman end
	
	// Check current connection state
	sendLocker.Lock(); //Xman threadsafe!
	if(byConnected == ES_DISCONNECTED){
		sendLocker.Unlock(); //Xman threadsafe!
		return;
	}
	else {	
		byConnected = ES_CONNECTED; // ES_DISCONNECTED, ES_NOTCONNECTED, ES_CONNECTED
	}
	sendLocker.Unlock(); //Xman threadsafe!

	//Xman include ACK
	theApp.pBandWidthControl->AddeMuleOutTCPOverall(0); //ACK

	//zz_fly
	//netfinity: Special case when socket is closing but data still in buffer, need to empty buffer or deadlock forever
	/*
	ProcessReceiveData(); //Xman include ACK
	*/
	ProcessReceiveData(nErrorCode); //Xman include ACK
	//zz_fly end
}

//Xman include ACK
//zz_fly
//netfinity: Special case when socket is closing but data still in buffer, need to empty buffer or deadlock forever
/*
void CEMSocket::ProcessReceiveData()
*/
void CEMSocket::ProcessReceiveData(int nErrorCode)
//zz_fly end
//Xman include ACK end
{
	// the 2 meg size was taken from another place
	static char GlobalReadBuffer[256*1024];

	//Xman end include ACK
    // CPU load improvement
	//zz_fly
	//netfinity: Special case when socket is closing but data still in buffer, need to empty buffer or deadlock forever
    /*
    if(downloadLimitEnable == true && downloadLimit == 0){
	*/
	if(downloadLimitEnable == true && downloadLimit == 0 && nErrorCode != WSAESHUTDOWN){ 
	//zz_fly end
        EMTrace("CEMSocket::OnReceive blocked by limit");
        pendingOnReceive = true;

        //Receive(GlobalReadBuffer + pendingHeaderSize, 0);
        return;
    }

	// Remark: an overflow can not occur here
	uint32 readMax = sizeof(GlobalReadBuffer) - pendingHeaderSize; 
	//zz_fly
	//netfinity: Special case when socket is closing but data still in buffer, need to empty buffer or deadlock forever
	/*
	if(downloadLimitEnable == true && readMax > downloadLimit) {
	*/
	if(downloadLimitEnable == true && readMax > downloadLimit && nErrorCode != WSAESHUTDOWN) {
	//zz_fly end
		readMax = downloadLimit;
	}

	// We attempt to read up to 2 megs at a time (minus whatever is in our internal read buffer)
	uint32 ret = Receive(GlobalReadBuffer + pendingHeaderSize, readMax);
	if(ret == SOCKET_ERROR || byConnected == ES_DISCONNECTED){
		return;
	}

	// ==> Dynamic Socket Buffering [SiRoB] - Mephisto
#ifndef DONT_USE_SOCKET_BUFFERING
	uint32 recvbufferlimit = 2*ret;
	if (recvbufferlimit > (10*1024*1024)) {
		recvbufferlimit = (10*1024*1024);
	} else if (recvbufferlimit < 8192) {
		recvbufferlimit = 8192;
	}

	if (recvbufferlimit > m_uCurrentRecvBufferSize) {
		SetSockOpt(SO_RCVBUF, &recvbufferlimit, sizeof(recvbufferlimit), SOL_SOCKET);
	}
	int ilen = sizeof(int);
	GetSockOpt(SO_RCVBUF, &recvbufferlimit, &ilen, SOL_SOCKET);
	m_uCurrentRecvBufferSize = recvbufferlimit;
#endif
	// <== Dynamic Socket Buffering [SiRoB] - Mephisto

	//Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	//cache the value
	uint32 realreceivedbytes=GetRealReceivedBytes();
	//Xman end

	// Bandwidth control
	if(downloadLimitEnable == true){
		// Update limit
	//Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	/*
		downloadLimit -= GetRealReceivedBytes();
	}
	*/
		downloadLimit -= realreceivedbytes;
	}
	if(realreceivedbytes >0)
	{
		theApp.pBandWidthControl->AddeMuleInTCPOverall(realreceivedbytes);
		theApp.pBandWidthControl->AddeMuleInObfuscation(realreceivedbytes-ret);
	}
	//Xman end

	// CPU load improvement
	// Detect if the socket's buffer is empty (or the size did match...)
	//Xman Code Improvement: what is if the sizes match per accident ?
	/*
	pendingOnReceive = m_bFullReceive;
	*/
	//more accurate
	DWORD nBytes=0;
	if(downloadLimitEnable == false || !IOCtl(FIONREAD, &nBytes))
		pendingOnReceive=false;
	else if (nBytes != 0)
		pendingOnReceive=true;
	else	
		pendingOnReceive=false;
	//Xman end

	if (ret == 0)
		return;

	// Copy back the partial header into the global read buffer for processing
	if(pendingHeaderSize > 0) {
  		memcpy(GlobalReadBuffer, pendingHeader, pendingHeaderSize);
		ret += pendingHeaderSize;
		pendingHeaderSize = 0;
	}

	if (IsRawDataMode())
	{
		DataReceived((BYTE*)GlobalReadBuffer, ret);
		return;
	}

	char *rptr = GlobalReadBuffer; // floating index initialized with begin of buffer
	const char *rend = GlobalReadBuffer + ret; // end of buffer

	// Loop, processing packets until we run out of them
	while ((rend - rptr >= PACKET_HEADER_SIZE) || ((pendingPacket != NULL) && (rend - rptr > 0)))
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
		// called 8 times (10240/1300) by the lower layer before a splitted packet is 
		// rebuild and transferred to the above layer for processing.
		//
		// The purpose of this algorithm is to limit the amount of data exchanged between buffers

		if(pendingPacket == NULL){
			pendingPacket = new Packet(rptr); // Create new packet container. 
			rptr += 6;                        // Only the header is initialized so far

			// Bugfix We still need to check for a valid protocol
			// Remark: the default eMule v0.26b had removed this test......
			switch (pendingPacket->prot){
				case OP_EDONKEYPROT:
				case OP_PACKEDPROT:
				case OP_EMULEPROT:
					break;
				default:
					EMTrace("CEMSocket::OnReceive ERROR Wrong header");
					delete pendingPacket;
					pendingPacket = NULL;
					OnError(ERR_WRONGHEADER);
					return;
			}

			// Security: Check for buffer overflow (2MB)
			if(pendingPacket->size > sizeof(GlobalReadBuffer)) {
				delete pendingPacket;
				pendingPacket = NULL;
				OnError(ERR_TOOBIG);
				return;
			}

			// Init data buffer
			pendingPacket->pBuffer = new char[pendingPacket->size + 1];
			pendingPacketSize = 0;
		}

		// Bytes ready to be copied into packet's internal buffer
		ASSERT(rptr <= rend);
		uint32 toCopy = ((pendingPacket->size - pendingPacketSize) < (uint32)(rend - rptr)) ? 
			             (pendingPacket->size - pendingPacketSize) : (uint32)(rend - rptr);

		//Xman
		// - Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
		switch(pendingPacket->opcode){
			case OP_SENDINGPART:
			case OP_COMPRESSEDPART:
			case OP_SENDINGPART_I64:
			case OP_COMPRESSEDPART_I64:
				{
					// Don't wait to have recieved the full block (10k) to account it 
					if(client != NULL){						
						if(pendingPacketSize == 0){
							// Statistic, the control (FileId+StartPos+EndPos) header should not be included
							//FileID -> 16 Byte
							//OP_SENDINGPART: 4 + 4 ->24
							//OP_COMPRESSEDPART: 4 + 4 ->24
							//OP_SENDINGPART_I64: 8 + 8 -> 32
							//OP_COMPRESSEDPART_I64: 8 + 4 -> 28
							uint8 headersize;
							if(pendingPacket->opcode == OP_SENDINGPART || pendingPacket->opcode == OP_COMPRESSEDPART)
								headersize=24;
							else if(pendingPacket->opcode == OP_COMPRESSEDPART_I64)
								headersize=28;
							else
								headersize=32;
							uint32 receivedBytes = (toCopy > headersize) ? (toCopy - headersize) : 0;
							client->AddDownloadRate(receivedBytes); 
							theApp.pBandWidthControl->AddeMuleIn(receivedBytes);
						}
						else {
							client->AddDownloadRate(toCopy); 
							theApp.pBandWidthControl->AddeMuleIn(toCopy);
						}
					}
				}
				break;
		}
		// Maella end

		// Copy Bytes from Global buffer to packet's internal buffer
		memcpy(&pendingPacket->pBuffer[pendingPacketSize], rptr, toCopy);
		pendingPacketSize += toCopy;
		rptr += toCopy;

		// Check if packet is complet
		ASSERT(pendingPacket->size >= pendingPacketSize);
		if(pendingPacket->size == pendingPacketSize){
#ifdef EMSOCKET_DEBUG
			EMTrace("CEMSocket::PacketReceived on %d, opcode=%X, realSize=%d", 
				    (SOCKET)this, pendingPacket->opcode, pendingPacket->GetRealPacketSize());
#endif

			// Process packet
			bool bPacketResult = PacketReceived(pendingPacket);
			delete pendingPacket;	
			pendingPacket = NULL;
			pendingPacketSize = 0;

			if (!bPacketResult)
				return;
		}
	}

	// Finally, if there is any data left over, save it for next time
	ASSERT(rptr <= rend);
	ASSERT(rend - rptr < PACKET_HEADER_SIZE);
	if(rptr != rend) {
		// Keep the partial head
		pendingHeaderSize = rend - rptr;
		memcpy(pendingHeader, rptr, pendingHeaderSize);
	}	
}

void CEMSocket::SetDownloadLimit(uint32 limit){	
	downloadLimit = limit;
	downloadLimitEnable = true;	
	
	// CPU load improvement
	if(limit > 0 && pendingOnReceive == true){
		//Xman include ACK
		/*
		OnReceive(0);
		*/
		ProcessReceiveData();
		//Xman end
	}
}

void CEMSocket::DisableDownloadLimit(){
	downloadLimitEnable = false;

	// CPU load improvement
	if(pendingOnReceive == true){
		//Xman include ACK
		/*
		OnReceive(0);
		*/
		ProcessReceiveData();
		//Xman end
	}
}

/**
 * Queues up the packet to be sent. Another thread will actually send the packet.
 *
 * If the packet is not a control packet, and if the socket decides that its queue is
 * full and forceAdd is false, then the socket is allowed to refuse to add the packet
 * to its queue. It will then return false and it is up to the calling thread to try
 * to call SendPacket for that packet again at a later time.
 *
 * @param packet address to the packet that should be added to the queue
 *
 * @param delpacket if true, the responsibility for deleting the packet after it has been sent
 *                  has been transferred to this object. If false, don't delete the packet after it
 *                  has been sent.
 *
 * @param controlpacket the packet is a controlpacket
 *
 * @param forceAdd this packet must be added to the queue, even if it is full. If this flag is true
 *                 then the method can not refuse to add the packet, and therefore not return false.
 *
 * @return true if the packet was added to the queue, false otherwise
 */
void CEMSocket::SendPacket(Packet* packet, bool delpacket, bool controlpacket, uint32 actualPayloadSize, bool bForceImmediateSend){
	//EMTrace("CEMSocket::OnSenPacked1 linked: %i, controlcount %i, standartcount %i, isbusy: %i",m_bLinkedPackets, controlpacket_queue.GetCount(), standartpacket_queue.GetCount(), IsBusy());

    sendLocker.Lock();

    if (byConnected == ES_DISCONNECTED) {
        sendLocker.Unlock();
        if(delpacket) {
			delete packet;
        }
		return;
    } else {
        if (!delpacket){
            //ASSERT ( !packet->IsSplitted() );
            Packet* copy = new Packet(packet->opcode,packet->size);
		    memcpy(copy->pBuffer,packet->pBuffer,packet->size);
		    packet = copy;
	    }

        //if(m_startSendTick > 0) {
        //    m_lastSendLatency = ::GetTickCount() - m_startSendTick;
        //}

        if (controlpacket) {
	        controlpacket_queue.AddTail(packet);

            // queue up for controlpacket
            //Xman Code Improvement
            if(isreadyforsending)
            {
	        if(!IsSocketUploading()) //Xman improved socket queuing
                        theApp.uploadBandwidthThrottler->QueueForSendingControlPacket(this, HasSent());
            }
        } else {
            //Xman unused
            /*
            bool first = !((sendbuffer && !m_currentPacket_is_controlpacket) || !standartpacket_queue.IsEmpty());
            */
            //Xman end
            StandardPacketQueueEntry queueEntry = { actualPayloadSize, packet };
            standartpacket_queue.AddTail(queueEntry);

            // reset timeout for the first time
            //Xman unused
            /*
            if (first) {
                lastFinishedStandard = ::GetTickCount();
                m_bAccelerateUpload = true;	// Always accelerate first packet in a block
            }
            */
            //Xman end
        }
    }

    sendLocker.Unlock();
	if (bForceImmediateSend){
		ASSERT( controlpacket_queue.GetSize() == 1 );
		//Xman // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
		/*
		Send(1024, 0, true);
		*/
		SocketSentBytes socketSentBytes = Send(1024, 0, true); 
		if(socketSentBytes.sentBytesControlPackets > 0)
		{		
			theApp.pBandWidthControl->AddeMuleOutOverallNoHeader(socketSentBytes.sentBytesControlPackets);
		}
		//Xman end
	}
}

// ==> Send Array Packet [SiRoB] - Mephisto
#ifndef DONT_USE_SEND_ARRAY_PACKET
void CEMSocket::SendPacket(Packet* packet[], uint32 npacket, bool delpacket, bool controlpacket, uint32 actualPayloadSize, bool bForceImmediateSend){
	//EMTrace("CEMSocket::OnSenPacked1 linked: %i, controlcount %i, standartcount %i, isbusy: %i",m_bLinkedPackets, controlpacket_queue.GetCount(), standartpacket_queue.GetCount(), IsBusy());

    sendLocker.Lock();

    if (byConnected == ES_DISCONNECTED) {
        sendLocker.Unlock();
        for (uint32 i = 0; i < npacket; i++) {
			if(delpacket) {
				delete packet[i];
			}
        }
		return;
    } else {
        if (!delpacket){
            for (uint32 i = 0; i < npacket; i++) {
				//ASSERT ( !packet[i]->IsSplitted() );
				Packet* copy = new Packet(packet[i]->opcode,packet[i]->size);
				memcpy(copy->pBuffer,packet[i]->pBuffer,packet[i]->size);
				packet[i] = copy;
			}
	    }

        //if(m_startSendTick > 0) {
        //    m_lastSendLatency = ::GetTickCount() - m_startSendTick;
        //}

        if (controlpacket) {
	        for (uint32 i = 0; i < npacket; i++) {
				controlpacket_queue.AddTail(packet[i]);
			}

            // queue up for controlpacket
            //Xman Code Improvement
            if(isreadyforsending)
            {
	        if(!IsSocketUploading()) //Xman improved socket queuing
                        theApp.uploadBandwidthThrottler->QueueForSendingControlPacket(this, HasSent());
            }
        } else {
            //Xman unused
            /*
            bool first = !((sendbuffer && !m_currentPacket_is_controlpacket) || !standartpacket_queue.IsEmpty());
            */
            //Xman end
            uint32 payloadSize = actualPayloadSize/npacket;
			while (payloadSize <= actualPayloadSize) {
				actualPayloadSize -= payloadSize;
				if(actualPayloadSize < payloadSize) {
					payloadSize += actualPayloadSize;
				}
				StandardPacketQueueEntry queueEntry = { payloadSize, *packet++ };
				standartpacket_queue.AddTail(queueEntry);
			}

            // reset timeout for the first time
            //Xman unused
            /*
            if (first) {
                lastFinishedStandard = ::GetTickCount();
                m_bAccelerateUpload = true;	// Always accelerate first packet in a block
            }
            */
            //Xman end
        }
    }

    sendLocker.Unlock();
	if (bForceImmediateSend){
		ASSERT( controlpacket_queue.GetSize() == 1 );
		//Xman // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
		/*
		Send(1024, 0, true);
		*/
		SocketSentBytes socketSentBytes = Send(1024, 0, true); 
		if(socketSentBytes.sentBytesControlPackets > 0)
		{		
			theApp.pBandWidthControl->AddeMuleOutOverallNoHeader(socketSentBytes.sentBytesControlPackets);
		}
		//Xman end
	}
}
#endif
// <== Send Array Packet [SiRoB] - Mephisto

uint64 CEMSocket::GetSentBytesCompleteFileSinceLastCallAndReset() {
    sendLocker.Lock();

    uint64 sentBytes = m_numberOfSentBytesCompleteFile;
    m_numberOfSentBytesCompleteFile = 0;

    sendLocker.Unlock();

    return sentBytes;
}

uint64 CEMSocket::GetSentBytesPartFileSinceLastCallAndReset() {
    sendLocker.Lock();

    uint64 sentBytes = m_numberOfSentBytesPartFile;
    m_numberOfSentBytesPartFile = 0;

    sendLocker.Unlock();

    return sentBytes;
}

//Xman: this mothod seems not to be used
/*
uint64 CEMSocket::GetSentBytesControlPacketSinceLastCallAndReset() {
    sendLocker.Lock();

    uint64 sentBytes = m_numberOfSentBytesControlPacket;
    m_numberOfSentBytesControlPacket = 0;

    sendLocker.Unlock();

    return sentBytes;
}
*/
//Xman end

uint64 CEMSocket::GetSentPayloadSinceLastCallAndReset() {
    sendLocker.Lock();

    uint64 sentBytes = m_actualPayloadSizeSent;
    m_actualPayloadSizeSent = 0;

    sendLocker.Unlock();

    return sentBytes;
}

void CEMSocket::OnSend(int nErrorCode){
    //onSendWillBeCalledOuter = false;

    if (nErrorCode){
		OnError(nErrorCode);
		return;
	}

	//EMTrace("CEMSocket::OnSend linked: %i, controlcount %i, standartcount %i, isbusy: %i",m_bLinkedPackets, controlpacket_queue.GetCount(), standartpacket_queue.GetCount(), IsBusy());
	CEncryptedStreamSocket::OnSend(0);

    sendLocker.Lock();

	m_bBusy = false;

    // stopped sending here.
    //StoppedSendSoUpdateStats();

    if (byConnected == ES_DISCONNECTED) {
        sendLocker.Unlock();
		return;
    } else
		byConnected = ES_CONNECTED;

	//Xman Code Improvement
	/*
    if(m_currentPacket_is_controlpacket) {
        // queue up for control packet
		theApp.uploadBandwidthThrottler->QueueForSendingControlPacket(this, HasSent());
    }
	*/
	//Xman improved socket queuing
	// ==> Dynamic Socket Buffering [SiRoB] - Mephisto
#ifndef DONT_USE_SOCKET_BUFFERING
	if(sendblenWithoutControlPacket != sendblen - sent/*m_currentPacket_is_controlpacket == true*/ || !controlpacket_queue.IsEmpty()) {
#else
	if((m_currentPacket_is_controlpacket && sendbuffer!=NULL) || !controlpacket_queue.IsEmpty()) {
#endif
	// <== Dynamic Socket Buffering [SiRoB] - Mephisto
		// queue up for control packet
		if(!IsSocketUploading())
			theApp.uploadBandwidthThrottler->QueueForSendingControlPacket(this, HasSent());
	}
	isreadyforsending=true;
	//Xman end

    sendLocker.Unlock();
}

//void CEMSocket::StoppedSendSoUpdateStats() {
//    if(m_startSendTick > 0) {
//        m_lastSendLatency = ::GetTickCount()-m_startSendTick;
//        
//        if(m_lastSendLatency > 0) {
//            if(m_wasBlocked == true) {
//                SocketTransferStats newLatencyStat = { m_lastSendLatency, ::GetTickCount() };
//                m_Average_sendlatency_list.AddTail(newLatencyStat);
//                m_latency_sum += m_lastSendLatency;
//            }
//
//            m_startSendTick = 0;
//            m_wasBlocked = false;
//
//            CleanSendLatencyList();
//        }
//    }
//}
//
//void CEMSocket::CleanSendLatencyList() {
//    while(m_Average_sendlatency_list.GetCount() > 0 && ::GetTickCount() - m_Average_sendlatency_list.GetHead().timestamp > 3*1000) {
//        SocketTransferStats removedLatencyStat = m_Average_sendlatency_list.RemoveHead();
//        m_latency_sum -= removedLatencyStat.latency;
//    }
//}

/**
 * Try to put queued up data on the socket.
 *
 * Control packets have higher priority, and will be sent first, if possible.
 * Standard packets can be split up in several package containers. In that case
 * all the parts of a split package must be sent in a row, without any control packet
 * in between.
 *
 * @param maxNumberOfBytesToSend This is the maximum number of bytes that is allowed to be put on the socket
 *                               this call. The actual number of sent bytes will be returned from the method.
 *
 * @param onlyAllowedToSendControlPacket This call we only try to put control packets on the sockets.
 *                                       If there's a standard packet "in the way", and we think that this socket
 *                                       is no longer an upload slot, then it is ok to send the standard packet to
 *                                       get it out of the way. But it is not allowed to pick a new standard packet
 *                                       from the queue during this call. Several split packets are counted as one
 *                                       standard packet though, so it is ok to finish them all off if necessary.
 *
 * @return the actual number of bytes that were put on the socket.
 */
//Xman
/*
SocketSentBytes CEMSocket::Send(uint32 maxNumberOfBytesToSend, uint32 minFragSize, bool onlyAllowedToSendControlPacket) {
	//EMTrace("CEMSocket::Send linked: %i, controlcount %i, standartcount %i, isbusy: %i",m_bLinkedPackets, controlpacket_queue.GetCount(), standartpacket_queue.GetCount(), IsBusy());
    sendLocker.Lock();

    if (byConnected == ES_DISCONNECTED) {
        sendLocker.Unlock();
        SocketSentBytes returnVal = { false, 0, 0 };
        return returnVal;
    }

    bool anErrorHasOccured = false;
    uint32 sentStandardPacketBytesThisCall = 0;
    uint32 sentControlPacketBytesThisCall = 0;

    if(byConnected == ES_CONNECTED && IsEncryptionLayerReady() && !(m_bBusy && onlyAllowedToSendControlPacket)) {
        if(minFragSize < 1) {
            minFragSize = 1;
        }

        maxNumberOfBytesToSend = GetNextFragSize(maxNumberOfBytesToSend, minFragSize);

        bool bWasLongTimeSinceSend = (::GetTickCount() - lastSent) > 1000;

        lastCalledSend = ::GetTickCount();

        while(sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall < maxNumberOfBytesToSend && anErrorHasOccured == false && // don't send more than allowed. Also, there should have been no error in earlier loop
              (sendbuffer != NULL || !controlpacket_queue.IsEmpty() || !standartpacket_queue.IsEmpty()) && // there must exist something to send
               (onlyAllowedToSendControlPacket == false || // this means we are allowed to send both types of packets, so proceed
                sendbuffer != NULL && m_currentPacket_is_controlpacket == true || // We are in the progress of sending a control packet. We are always allowed to send those
                sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall > 0 && (sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall) % minFragSize != 0 || // Once we've started, continue to send until an even minFragsize to minimize packet overhead
                sendbuffer == NULL && !controlpacket_queue.IsEmpty() || // There's a control packet in queue, and we are not currently sending anything, so we will handle the control packet next
                sendbuffer != NULL && m_currentPacket_is_controlpacket == false && bWasLongTimeSinceSend && !controlpacket_queue.IsEmpty() && (sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall) < minFragSize // We have waited to long to clean the current packet (which may be a standard packet that is in the way). Proceed no matter what the value of onlyAllowedToSendControlPacket.
               )
             )
		{

            // If we are currently not in the progress of sending a packet, we will need to find the next one to send
            if(sendbuffer == NULL) {
                Packet* curPacket = NULL;
                if(!controlpacket_queue.IsEmpty()) {
                    // There's a control packet to send
                    m_currentPacket_is_controlpacket = true;
                    curPacket = controlpacket_queue.RemoveHead();
                } else if(!standartpacket_queue.IsEmpty()) {
                    // There's a standard packet to send
                    m_currentPacket_is_controlpacket = false;
                    StandardPacketQueueEntry queueEntry = standartpacket_queue.RemoveHead();
                    curPacket = queueEntry.packet;
                    m_actualPayloadSize = queueEntry.actualPayloadSize;

                    // remember this for statistics purposes.
                    m_currentPackageIsFromPartFile = curPacket->IsFromPF();
                } else {
                    // Just to be safe. Shouldn't happen?
                    sendLocker.Unlock();

                    // if we reach this point, then there's something wrong with the while condition above!
                    ASSERT(0);
                    theApp.QueueDebugLogLine(true,_T("EMSocket: Couldn't get a new packet! There's an error in the first while condition in EMSocket::Send()"));

                    SocketSentBytes returnVal = { true, sentStandardPacketBytesThisCall, sentControlPacketBytesThisCall };
                    return returnVal;
                }

                // We found a package to send. Get the data to send from the
                // package container and dispose of the container.
                sendblen = curPacket->GetRealPacketSize();
                sendbuffer = curPacket->DetachPacket();
                sent = 0;
                delete curPacket;

				// encrypting which cannot be done transparent by base class
				CryptPrepareSendData((uchar*)sendbuffer, sendblen);
            }

            // At this point we've got a packet to send in sendbuffer. Try to send it. Loop until entire packet
            // is sent, or until we reach maximum bytes to send for this call, or until we get an error.
            // NOTE! If send would block (returns WSAEWOULDBLOCK), we will return from this method INSIDE this loop.
            while (sent < sendblen &&
                   sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall < maxNumberOfBytesToSend &&
                   (
                    onlyAllowedToSendControlPacket == false || // this means we are allowed to send both types of packets, so proceed
                    m_currentPacket_is_controlpacket ||
                    bWasLongTimeSinceSend && (sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall) < minFragSize ||
                    (sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall) % minFragSize != 0
                   ) &&
                   anErrorHasOccured == false)
			{
		        uint32 tosend = sendblen-sent;
                if(!onlyAllowedToSendControlPacket || m_currentPacket_is_controlpacket) {
    		        if (maxNumberOfBytesToSend >= sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall && tosend > maxNumberOfBytesToSend-(sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall))
                        tosend = maxNumberOfBytesToSend-(sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall);
                } else if(bWasLongTimeSinceSend && (sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall) < minFragSize) {
    		        if (minFragSize >= sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall && tosend > minFragSize-(sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall))
                        tosend = minFragSize-(sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall);
                } else {
                    uint32 nextFragMaxBytesToSent = GetNextFragSize(sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall, minFragSize);
    		        if (nextFragMaxBytesToSent >= sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall && tosend > nextFragMaxBytesToSent-(sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall))
                        tosend = nextFragMaxBytesToSent-(sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall);
                }
		        ASSERT (tosend != 0 && tosend <= sendblen-sent);
        		
                //DWORD tempStartSendTick = ::GetTickCount();

                lastSent = ::GetTickCount();

		        uint32 result = CEncryptedStreamSocket::Send(sendbuffer+sent,tosend); // deadlake PROXYSUPPORT - changed to AsyncSocketEx
		        if (result == (uint32)SOCKET_ERROR){
			        uint32 error = GetLastError();
			        if (error == WSAEWOULDBLOCK){
                        m_bBusy = true;

                        //m_wasBlocked = true;
                        sendLocker.Unlock();

                        SocketSentBytes returnVal = { true, sentStandardPacketBytesThisCall, sentControlPacketBytesThisCall };
                        return returnVal; // Send() blocked, onsend will be called when ready to send again
			        } else{
                        // Send() gave an error
                        anErrorHasOccured = true;
                        //DEBUG_ONLY( AddDebugLogLine(true,"EMSocket: An error has occured: %i", error) );
                    }
                } else {
                    // we managed to send some bytes. Perform bookkeeping.
                    m_bBusy = false;
                    m_hasSent = true;

                    sent += result;

                    // Log send bytes in correct class
                    if(m_currentPacket_is_controlpacket == false) {
                        sentStandardPacketBytesThisCall += result;

                        if(m_currentPackageIsFromPartFile == true) {
                            m_numberOfSentBytesPartFile += result;
                        } else {
                            m_numberOfSentBytesCompleteFile += result;
                        }
                    } else {
                        sentControlPacketBytesThisCall += result;
                        m_numberOfSentBytesControlPacket += result;
                    }
                }
	        }

            if (sent == sendblen){
                // we are done sending the current package. Delete it and set
                // sendbuffer to NULL so a new packet can be fetched.
		        delete[] sendbuffer;
		        sendbuffer = NULL;
			    sendblen = 0;

                if(!m_currentPacket_is_controlpacket) {
                    m_actualPayloadSizeSent += m_actualPayloadSize;
                    m_actualPayloadSize = 0;

                    lastFinishedStandard = ::GetTickCount(); // reset timeout
                    m_bAccelerateUpload = false; // Safe until told otherwise
                }

                sent = 0;
            }
        }
    }

    if(onlyAllowedToSendControlPacket && (!controlpacket_queue.IsEmpty() || sendbuffer != NULL && m_currentPacket_is_controlpacket)) {
        // enter control packet send queue
        // we might enter control packet queue several times for the same package,
        // but that costs very little overhead. Less overhead than trying to make sure
        // that we only enter the queue once.
        theApp.uploadBandwidthThrottler->QueueForSendingControlPacket(this, HasSent());
    }

    //CleanSendLatencyList();

    sendLocker.Unlock();

    SocketSentBytes returnVal = { !anErrorHasOccured, sentStandardPacketBytesThisCall, sentControlPacketBytesThisCall };
    return returnVal;
}
*/
//SocketSentBytes CEMSocket::Send(uint32 maxNumberOfBytesToSend, uint32 minFragSize, bool onlyAllowedToSendControlPacket) {
//	//EMTrace("CEMSocket::Send linked: %i, controlcount %i, standartcount %i, isbusy: %i",m_bLinkedPackets, controlpacket_queue.GetCount(), standartpacket_queue.GetCount(), IsBusy());
//
//	//Xman  Code Improvement
//	//there is no need to lock when testing m_bBusy, because we have only one caller (uploadbandwidththrottler)
//	//the worse case is ONSend is triggered at the same time we are at this point.. but then we only have to wait until next uploadbandwidththrottler-loop
//	if (m_bBusy && onlyAllowedToSendControlPacket) 
//	{
//			SocketSentBytes returnVal = { true, 0, 0 };
//			return returnVal;
//	}
//	//Xman Code Improvement end
//	
//
//
//	sendLocker.Lock();
//
//	//Xman 5.1.1: //Xman improved socket queuing
//	//due to the new socket-queueing this should only happen in very very rare cases
//	//no need for an extra check.. let it happen
//	/*
//	//Xman uploadbandwidththrottler: a uploading client shouldn't get data on the control-packet-sending-loop (see uploadbandwidththrottler)
//	//this little improvement allow a bit better upload-control
//	if(IsSocketUploading() && onlyAllowedToSendControlPacket)
//	{
//		sendLocker.Unlock();
//		theApp.QueueDebugLogLine(false, _T("-=-> uploading socket tries to send control packets"));
//		SocketSentBytes returnVal = { true, 0, 0 };
//		return returnVal;
//	}
//	*/
//	//Xman end
//
//
//    //if (byConnected == ES_DISCONNECTED) {
//	if (byConnected != ES_CONNECTED) { //Xman changed 5.1.1
//        sendLocker.Unlock();
//        SocketSentBytes returnVal = { false, 0, 0 };
//        return returnVal;
//    }
//
//
//    if(minFragSize < 1) {
//        minFragSize = 1;
//    }
//
//	//Xman Xtreme Upload
//	//don't add the header of standardpackage:
//	bool newdatapacket=false;
//	uint8 sendingdata_opcode=0;
//	uint32	IPHeaderThisCall=0;
//	//Xman end
//
//    maxNumberOfBytesToSend = GetNextFragSize(maxNumberOfBytesToSend, minFragSize);
//
//    bool bWasLongTimeSinceSend = (::GetTickCount() - lastSent) > 1000;
//
//    lastCalledSend = ::GetTickCount();
//
//    bool anErrorHasOccured = false;
//    uint32 sentStandardPacketBytesThisCall = 0;
//    uint32 sentControlPacketBytesThisCall = 0;
//
//
//    while(sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall < maxNumberOfBytesToSend && anErrorHasOccured == false && // don't send more than allowed. Also, there should have been no error in earlier loop
//          (!controlpacket_queue.IsEmpty() || !standartpacket_queue.IsEmpty() || sendbuffer != NULL) && // there must exist something to send
//          (onlyAllowedToSendControlPacket == false || // this means we are allowed to send both types of packets, so proceed
//           sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall > 0 && (sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall) % minFragSize != 0 ||
//           sendbuffer == NULL && !controlpacket_queue.IsEmpty() || // There's a control packet in queue, and we are not currently sending anything, so we will handle the control packet next
//           sendbuffer != NULL && m_currentPacket_is_controlpacket == true || // We are in the progress of sending a control packet. We are always allowed to send those
//           sendbuffer != NULL && m_currentPacket_is_controlpacket == false && bWasLongTimeSinceSend && !controlpacket_queue.IsEmpty() && standartpacket_queue.IsEmpty() && (sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall) < minFragSize // We have waited to long to clean the current packet (which may be a standard packet that is in the way). Proceed no matter what the value of onlyAllowedToSendControlPacket.
//          )
//         ) {
//
//        // If we are currently not in the progress of sending a packet, we will need to find the next one to send
//        if(sendbuffer == NULL) {
//            Packet* curPacket = NULL;
//            if(!controlpacket_queue.IsEmpty()) {
//                // There's a control packet to send
//                m_currentPacket_is_controlpacket = true;
//                curPacket = controlpacket_queue.RemoveHead();
//            } else if(!standartpacket_queue.IsEmpty() 
//				&& onlyAllowedToSendControlPacket == false) { //Xman look for 4.2: I use this code, although it is redundant, but there were a few very suspicious crashes at this point
//                // There's a standard packet to send
//                m_currentPacket_is_controlpacket = false;
//                StandardPacketQueueEntry queueEntry = standartpacket_queue.RemoveHead();
//                curPacket = queueEntry.packet;
//                m_actualPayloadSize = queueEntry.actualPayloadSize;
//				//Xman Xtreme Upload
//				// we have to remember, that a data package has begone
//				// after sending (particular), subtract the header
//                // remember this for statistics purposes.
//				newdatapacket=true;
//				sendingdata_opcode=curPacket->opcode;
//                m_currentPackageIsFromPartFile = curPacket->IsFromPF();
//            } else {
//                // Just to be safe. Shouldn't happen?
//                sendLocker.Unlock();
//
//                // if we reach this point, then there's something wrong with the while condition above!
//                ASSERT(0);
//                theApp.QueueDebugLogLine(true,_T("EMSocket: Couldn't get a new packet! There's an error in the first while condition in EMSocket::Send()"));
//
//                SocketSentBytes returnVal = { true, sentStandardPacketBytesThisCall, sentControlPacketBytesThisCall };
//                return returnVal;
//            }
//
//            // We found a package to send. Get the data to send from the
//            // package container and dispose of the container.
//            sendblen = curPacket->GetRealPacketSize();
//            sendbuffer = curPacket->DetachPacket();
//            sent = 0;
//            delete curPacket;
//        }
//
//        // At this point we've got a packet to send in sendbuffer. Try to send it. Loop until entire packet
//        // is sent, or until we reach maximum bytes to send for this call, or until we get an error.
//        // NOTE! If send would block (returns WSAEWOULDBLOCK), we will return from this method INSIDE this loop.
//        while (sent < sendblen &&
//               sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall < maxNumberOfBytesToSend &&
//               (
//                onlyAllowedToSendControlPacket == false || // this means we are allowed to send both types of packets, so proceed
//                m_currentPacket_is_controlpacket ||
//                bWasLongTimeSinceSend && (sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall) < minFragSize ||
//                (sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall) % minFragSize != 0
//               ) &&
//               anErrorHasOccured == false) {
//		    uint32 tosend = sendblen-sent;
//            if(!onlyAllowedToSendControlPacket || m_currentPacket_is_controlpacket) {
//    		    if (maxNumberOfBytesToSend >= sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall && tosend > maxNumberOfBytesToSend-(sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall))
//                    tosend = maxNumberOfBytesToSend-(sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall);
//            } else if(bWasLongTimeSinceSend && (sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall) < minFragSize) {
//    		    if (minFragSize >= sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall && tosend > minFragSize-(sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall))
//                    tosend = minFragSize-(sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall);
//            } else {
//                uint32 nextFragMaxBytesToSent = GetNextFragSize(sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall, minFragSize);
//    		    if (nextFragMaxBytesToSent >= sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall && tosend > nextFragMaxBytesToSent-(sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall))
//                    tosend = nextFragMaxBytesToSent-(sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall);
//            }
//		    ASSERT (tosend != 0 && tosend <= sendblen-sent);
//    		
//            //DWORD tempStartSendTick = ::GetTickCount();
//
//            lastSent = ::GetTickCount();
//
//			//Xman count block/success send
//			if(!onlyAllowedToSendControlPacket)
//				sendcount++;
//
//		    uint32 result = CAsyncSocketEx::Send(sendbuffer+sent,tosend); // deadlake PROXYSUPPORT - changed to AsyncSocketEx
//		    if (result == (uint32)SOCKET_ERROR){
//			    uint32 error = GetLastError();
//			    if (error == WSAEWOULDBLOCK){
//                    m_bBusy = true;
//
//
//					//Xman 4.8.2 moved here
//					//Xman count the blocksend to remove such clients if needed
//					if(!onlyAllowedToSendControlPacket)
//						blockedsendcount++;
//					//Xman end
//
//                    //m_wasBlocked = true;
//                    sendLocker.Unlock();
//
//                    SocketSentBytes returnVal = { true, sentStandardPacketBytesThisCall, sentControlPacketBytesThisCall };
//                    return returnVal; // Send() blocked, onsend will be called when ready to send again
//			    } else{
//                    // Send() gave an error
//                    anErrorHasOccured = true;
//                    //DEBUG_ONLY( AddDebugLogLine(true,"EMSocket: An error has occured: %i", error) );
//                }
//            } else {
//				// we managed to send some bytes. Perform bookkeeping.
//                m_bBusy = false;
//                m_hasSent = true;
//
//                sent += result;
//
//
//                // Log send bytes in correct class
//                //Xman Xtreme Upload
//				//after sending a complete package we have to remove the header size
//				// Remove: header+FileId+StartPos+EndPos 
//				if(m_currentPacket_is_controlpacket == false) {
//					uint32 packetheadersize=0;
//					if(newdatapacket)
//					{
//						//Header -> 6 Bytes
//						//FileID -> 16 Byte
//						//OP_SENDINGPART: 4 + 4 ->30
//						//OP_COMPRESSEDPART: 4 + 4 ->30
//						//OP_SENDINGPART_I64: 8 + 8 -> 36
//						//OP_COMPRESSEDPART_I64: 8 + 4 -> 32
//						switch(sendingdata_opcode)
//						{
//							case OP_SENDINGPART:
//							case OP_COMPRESSEDPART:
//							{
//								packetheadersize=  (result > 30) ? 30 : result;
//								break;
//							}
//							case OP_COMPRESSEDPART_I64:
//							{
//								packetheadersize=  (result > 32) ? 32 : result;
//								break;
//							}
//							case OP_SENDINGPART_I64:
//							{
//								packetheadersize=  (result > 36) ? 36 : result;
//								break;
//							}
//							default:
//								ASSERT(0);
//						}
//					}
//                    if(m_currentPackageIsFromPartFile == true) {
//                        m_numberOfSentBytesPartFile += (result-packetheadersize);
//                    } else {
//                        m_numberOfSentBytesCompleteFile += (result-packetheadersize);
//                    }
//					sentStandardPacketBytesThisCall += (result-packetheadersize);
//					sentControlPacketBytesThisCall += packetheadersize;
//                } else {
//                    sentControlPacketBytesThisCall += result;
//                    //m_numberOfSentBytesControlPacket += result; //Xman unused
//                }
//				newdatapacket=false; //to be sure if sending two (mini)data packets
//				// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
//				IPHeaderThisCall+= (20+20); //Header
//				//Xman end
//            }
//	    }
//
//        if (sent == sendblen){
//            // we are done sending the current package. Delete it and set
//            // sendbuffer to NULL so a new packet can be fetched.
//		    delete[] sendbuffer;
//		    sendbuffer = NULL;
//			sendblen = 0;
//			
//
//            if(!m_currentPacket_is_controlpacket) {
//                m_actualPayloadSizeSent += m_actualPayloadSize;
//                m_actualPayloadSize = 0;
//
//				//Xman unused
//                //lastFinishedStandard = ::GetTickCount(); // reset timeout
//                //m_bAccelerateUpload = false; // Safe until told otherwise
//            }
//
//            sent = 0;
//        }
//    }
//
//	if(onlyAllowedToSendControlPacket && (!controlpacket_queue.IsEmpty() || sendbuffer != NULL && m_currentPacket_is_controlpacket)) {
//        // enter control packet send queue
//        // we might enter control packet queue several times for the same package,
//        // but that costs very little overhead. Less overhead than trying to make sure
//        // that we only enter the queue once.
//		theApp.uploadBandwidthThrottler->QueueForSendingControlPacket(this, HasSent());
//    }
//
//    //CleanSendLatencyList();
//
//	// - Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
//	sentControlPacketBytesThisCall+=IPHeaderThisCall;
//	//Xman end
//
//    sendLocker.Unlock();
//
//    SocketSentBytes returnVal = { !anErrorHasOccured, sentStandardPacketBytesThisCall, sentControlPacketBytesThisCall };
//    return returnVal;
//}


//Xman 5.2 new version 
//remark: minFragSize is now unused. maxNumberOfBytesToSend must be the size of MSS or MSS *2
// ==> Dynamic Socket Buffering [SiRoB] - Mephisto
#ifndef DONT_USE_SOCKET_BUFFERING
// Note: we need minFragSize to determine the number amount of IP headers that were sent
SocketSentBytes CEMSocket::Send(uint32 maxNumberOfBytesToSend, uint32 minFragSize, bool onlyAllowedToSendControlPacket) {
#else
SocketSentBytes CEMSocket::Send(uint32 maxNumberOfBytesToSend, uint32 /*minFragSize*/, bool onlyAllowedToSendControlPacket) {
#endif
// <== Dynamic Socket Buffering [SiRoB] - Mephisto
	//EMTrace("CEMSocket::Send linked: %i, controlcount %i, standartcount %i, isbusy: %i",m_bLinkedPackets, controlpacket_queue.GetCount(), standartpacket_queue.GetCount(), IsBusy());



	sendLocker.Lock();

	//Can happen after resort of sockets (peercache)
	//let uploading sockets only send when the uploadbandwidththrottler allow it
	if(onlyAllowedToSendControlPacket && IsSocketUploading())
	{
		sendLocker.Unlock();
		SocketSentBytes returnVal = { true, 0, 0 };
		return returnVal;
	}


	if (byConnected != ES_CONNECTED) 
	{ 
		sendLocker.Unlock();
		SocketSentBytes returnVal = { false, 0, 0 };
		return returnVal;
	}


	//minFragsize should always be valid: see uploadbandwiththrottler
	//if(minFragSize < 1) {
	//	minFragSize = 1;
	//}

	//Xman Xtreme Upload
	//don't add the header of standardpackage:
	// ==> Dynamic Socket Buffering [SiRoB] - Mephisto
#ifdef DONT_USE_SOCKET_BUFFERING
	bool newdatapacket=false;
	uint8 sendingdata_opcode=0;
#endif
	// <== Dynamic Socket Buffering [SiRoB] - Mephisto
	uint32	IPHeaderThisCall=0;
	//Xman end

	bool anErrorHasOccured = false;
	uint32 sentStandardPacketBytesThisCall = 0;
	uint32 sentControlPacketBytesThisCall = 0;

	if(IsEncryptionLayerReady())
	{
		lastCalledSend = ::GetTickCount();

		// ==> Dynamic Socket Buffering [SiRoB] - Mephisto
#ifndef DONT_USE_SOCKET_BUFFERING
		if(sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall < maxNumberOfBytesToSend && anErrorHasOccured == false && // don't send more than allowed. Also, there should have been no error in earlier loop
			(!controlpacket_queue.IsEmpty() || !standartpacket_queue.IsEmpty() || sendblen/*sendbuffer*/ != NULL)  // there must exist something to send
			) 
#else
		while(sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall < maxNumberOfBytesToSend && anErrorHasOccured == false && // don't send more than allowed. Also, there should have been no error in earlier loop
			(!controlpacket_queue.IsEmpty() || !standartpacket_queue.IsEmpty() || sendbuffer != NULL)  // there must exist something to send
			) 
#endif
		// <== Dynamic Socket Buffering [SiRoB] - Mephisto
		{

				// If we are currently not in the progress of sending a packet, we will need to find the next one to send
				// ==> Dynamic Socket Buffering [SiRoB] - Mephisto
#ifndef DONT_USE_SOCKET_BUFFERING
			ASSERT(sendblen>=sent);
			while ((!controlpacket_queue.IsEmpty() || !standartpacket_queue.IsEmpty()) && sendblen-sent+sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall < maxNumberOfBytesToSend) {
				bool bcontrolpacket;
				bool bnewdatapacket = false;
				uint32 ipacketpayloadsize = 0;
#else
				if(sendbuffer == NULL) {
#endif
				// <== Dynamic Socket Buffering [SiRoB] - Mephisto
					Packet* curPacket = NULL;
					if(!controlpacket_queue.IsEmpty()) {
						// There's a control packet to send
						// ==> Dynamic Socket Buffering [SiRoB] - Mephisto
#ifndef DONT_USE_SOCKET_BUFFERING
						bcontrolpacket = true;
#else
						m_currentPacket_is_controlpacket = true;
#endif
						// <== Dynamic Socket Buffering [SiRoB] - Mephisto
						curPacket = controlpacket_queue.RemoveHead();
					} else if(!standartpacket_queue.IsEmpty() 
						&& onlyAllowedToSendControlPacket == false) { //Xman look for 4.2: I use this code, although it is redundant, but there were a few very suspicious crashes at this point
							// There's a standard packet to send
							// ==> Dynamic Socket Buffering [SiRoB] - Mephisto
#ifndef DONT_USE_SOCKET_BUFFERING
							bcontrolpacket = false;
							bnewdatapacket = true;
#else
							m_currentPacket_is_controlpacket = false;
#endif
							// <== Dynamic Socket Buffering [SiRoB] - Mephisto
							StandardPacketQueueEntry queueEntry = standartpacket_queue.RemoveHead();
							curPacket = queueEntry.packet;
							// ==> Dynamic Socket Buffering [SiRoB] - Mephisto
#ifndef DONT_USE_SOCKET_BUFFERING
							ipacketpayloadsize = queueEntry.actualPayloadSize;
#else
							m_actualPayloadSize = queueEntry.actualPayloadSize;
							//Xman Xtreme Upload
							// we have to remember, that a data package has begone
							// after sending (particular), subtract the header
							// remember this for statistics purposes.
							newdatapacket=true;
							sendingdata_opcode=curPacket->opcode;
							m_currentPackageIsFromPartFile = curPacket->IsFromPF();
#endif
							// <== Dynamic Socket Buffering [SiRoB] - Mephisto
						} else {
							// Just to be safe. Shouldn't happen?
							sendLocker.Unlock();

							// if we reach this point, then there's something wrong with the while condition above!
							ASSERT(0);
							theApp.QueueDebugLogLine(true,_T("EMSocket: Couldn't get a new packet! There's an error in the first while condition in EMSocket::Send()"));

							SocketSentBytes returnVal = { true, sentStandardPacketBytesThisCall, sentControlPacketBytesThisCall };
							return returnVal;
						}

						// We found a package to send. Get the data to send from the
						// package container and dispose of the container.
						// ==> Dynamic Socket Buffering [SiRoB] - Mephisto
#if !defined DONT_USE_SOCKET_BUFFERING
						uint32 packetsize = curPacket->GetRealPacketSize();
						uint32 sendbufferlimit = packetsize;
						if (sendbufferlimit > 10*1024*1024)
							sendbufferlimit = 10*1024*1024;
						else if (sendbufferlimit < 8192)
							sendbufferlimit = 8192;
						if (m_uCurrentSendBufferSize != sendbufferlimit) {
							SetSockOpt(SO_SNDBUF, &sendbufferlimit, sizeof(sendbufferlimit), SOL_SOCKET);
						}
						int ilen = sizeof(int);
						GetSockOpt(SO_SNDBUF, &sendbufferlimit, &ilen, SOL_SOCKET);
						m_uCurrentSendBufferSize = sendbufferlimit;
						if (sendbuffer) {
							if (sent > (currentBufferSize>>1) || currentBufferSize < sendblen+packetsize){
								ASSERT(sendblen>=sent);
								sendblen-=sent;
								if (currentBufferSize < sendblen+packetsize) {
									currentBufferSize = max(sendblen+packetsize, 2*m_uCurrentSendBufferSize);
									char* newsendbuffer = new char[currentBufferSize];
									memcpy(newsendbuffer, sendbuffer+sent, sendblen);
									delete[] sendbuffer;
									sendbuffer = newsendbuffer;
								} else {
									memmove(sendbuffer, sendbuffer+sent, sendblen);
								}
								sent = 0;
							}
							char* packetcore = curPacket->DetachPacket();
							// encrypting which cannot be done transparent by base class
							CryptPrepareSendData((uchar*)packetcore, packetsize);
							memcpy(sendbuffer+sendblen, packetcore, packetsize);
							delete[] packetcore;
							sendblen+=packetsize;
						} else {
							ASSERT (sent == 0);
							sendbuffer = curPacket->DetachPacket();
							sendblen = packetsize;
							// encrypting which cannot be done transparent by base class
							CryptPrepareSendData((uchar*)sendbuffer, packetsize);
							currentBufferSize = packetsize;
						}
						if (bcontrolpacket == false)
							sendblenWithoutControlPacket+=packetsize;
						BufferedPacket* newitem = new BufferedPacket;
						newitem->remainpacketsize = packetsize;
						newitem->isforpartfile = curPacket->IsFromPF();
						newitem->iscontrolpacket = bcontrolpacket;
						newitem->packetpayloadsize = ipacketpayloadsize;
						newitem->newdatapacket = bnewdatapacket;
						newitem->sendingdata_opcode = curPacket->opcode;
						m_currentPacket_in_buffer_list.AddTail(newitem);
						delete curPacket;
#else
						sendblen = curPacket->GetRealPacketSize();
						sendbuffer = curPacket->DetachPacket();
						sent = 0;
						delete curPacket;

						// encrypting which cannot be done transparent by base class
						CryptPrepareSendData((uchar*)sendbuffer, sendblen);
#endif
						// <== Dynamic Socket Buffering [SiRoB] - Mephisto
				}

				// At this point we've got a packet to send in sendbuffer. Try to send it. Loop until entire packet
				// is sent, or until we reach maximum bytes to send for this call, or until we get an error.
				// NOTE! If send would block (returns WSAEWOULDBLOCK), we will return from this method INSIDE this loop.
				while (sent < sendblen &&
					sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall < maxNumberOfBytesToSend &&
					anErrorHasOccured == false) 
				{
						uint32 tosend = sendblen-sent;
						if (maxNumberOfBytesToSend >= sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall && tosend > maxNumberOfBytesToSend-(sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall))
							tosend = maxNumberOfBytesToSend-(sentStandardPacketBytesThisCall + sentControlPacketBytesThisCall);
						ASSERT (tosend != 0 && tosend <= sendblen-sent);


						//Xman count block/success send
						if(!onlyAllowedToSendControlPacket)
						{
							sendcount++;
							sendcount_overall++;
						}

						uint32 result = CEncryptedStreamSocket::Send(sendbuffer+sent,tosend); // deadlake PROXYSUPPORT - changed to AsyncSocketEx

						if (result == (uint32)SOCKET_ERROR){
							uint32 error = GetLastError();
							if (error == WSAEWOULDBLOCK){
								m_bBusy = true;


								//Xman 4.8.2 moved here
								//Xman count block/success send
								if(!onlyAllowedToSendControlPacket)
								{
									blockedsendcount++;
									blockedsendcount_overall++;
								}
								//Xman end

								//m_wasBlocked = true;
								sendLocker.Unlock();

								SocketSentBytes returnVal = { true, sentStandardPacketBytesThisCall, sentControlPacketBytesThisCall };
								return returnVal; // Send() blocked, onsend will be called when ready to send again
							} else{
								// Send() gave an error
								anErrorHasOccured = true;
								//DEBUG_ONLY( AddDebugLogLine(true,"EMSocket: An error has occured: %i", error) );
							}
						} else {
							// we managed to send some bytes. Perform bookkeeping.
							m_bBusy = false;
							m_hasSent = true;

							if(result>0) 
							{
								sent += result;


								// Log send bytes in correct class
								//Xman Xtreme Upload
								//after sending a complete package we have to remove the header size
								// Remove: header+FileId+StartPos+EndPos 
								// ==> Dynamic Socket Buffering [SiRoB] - Mephisto
#ifndef DONT_USE_SOCKET_BUFFERING
								uint32 sumofpacketsizesent = 0;
								uint32 sumofnocontrolpacketsizesent = 0;
								uint32 sumofpacketpartfilesizesent = 0;
								while (result > sumofpacketsizesent && result-sumofpacketsizesent >= m_currentPacket_in_buffer_list.GetHead()->remainpacketsize) {
									BufferedPacket* pPacket = m_currentPacket_in_buffer_list.RemoveHead();
									if(pPacket->iscontrolpacket == false) {
										uint32 packetheadersize=0;
										if(pPacket->newdatapacket)
										{
											//Header -> 6 Bytes
											//FileID -> 16 Byte
											//OP_SENDINGPART: 4 + 4 ->30
											//OP_COMPRESSEDPART: 4 + 4 ->30
											//OP_SENDINGPART_I64: 8 + 8 -> 36
											//OP_COMPRESSEDPART_I64: 8 + 4 -> 32
											switch(pPacket->sendingdata_opcode)
											{
											case OP_SENDINGPART:
											case OP_COMPRESSEDPART:
												{
													packetheadersize=  (result > 30) ? 30 : result;
													break;
												}
											case OP_COMPRESSEDPART_I64:
												{
													packetheadersize=  (result > 32) ? 32 : result;
													break;
												}
											case OP_SENDINGPART_I64:
												{
													packetheadersize=  (result > 36) ? 36 : result;
													break;
												}
											default:
												ASSERT(0);
											}
										}
										sumofnocontrolpacketsizesent += pPacket->remainpacketsize-packetheadersize;
										if(pPacket->isforpartfile == true)
											sumofpacketpartfilesizesent += pPacket->remainpacketsize-packetheadersize;

										if(0 < pPacket->packetpayloadsize) {
											//statsLocker.Lock();
											m_actualPayloadSizeSent += pPacket->packetpayloadsize;
											//statsLocker.Unlock();
										} else {
											ASSERT(0);
										}
										sendblenWithoutControlPacket -= pPacket->remainpacketsize-packetheadersize;
									} else {
										sentControlPacketBytesThisCall += result;
										//m_numberOfSentBytesControlPacket += result; //Xman unused
									}
									pPacket->newdatapacket=false; //to be sure if sending two (mini)data packets
									sumofpacketsizesent += pPacket->remainpacketsize;
									delete pPacket;
								}

								if (result > sumofpacketsizesent) {
									BufferedPacket* pPacket = m_currentPacket_in_buffer_list.GetHead();
									uint32 partialpacketsizesent = result-sumofpacketsizesent;
									if (pPacket->iscontrolpacket == false) {
										uint32 packetheadersize=0;
										if(pPacket->newdatapacket)
										{
											//Header -> 6 Bytes
											//FileID -> 16 Byte
											//OP_SENDINGPART: 4 + 4 ->30
											//OP_COMPRESSEDPART: 4 + 4 ->30
											//OP_SENDINGPART_I64: 8 + 8 -> 36
											//OP_COMPRESSEDPART_I64: 8 + 4 -> 32
											switch(pPacket->sendingdata_opcode)
											{
											case OP_SENDINGPART:
											case OP_COMPRESSEDPART:
												{
													packetheadersize=  (result > 30) ? 30 : result;
													break;
												}
											case OP_COMPRESSEDPART_I64:
												{
													packetheadersize=  (result > 32) ? 32 : result;
													break;
												}
											case OP_SENDINGPART_I64:
												{
													packetheadersize=  (result > 36) ? 36 : result;
													break;
												}
											default:
												ASSERT(0);
											}
										}
										sumofnocontrolpacketsizesent += partialpacketsizesent-packetheadersize;
										if (pPacket->isforpartfile)
											sumofpacketpartfilesizesent += partialpacketsizesent-packetheadersize;
										uint32 partialpayloadSentWithThisCall = (uint32)(((double)partialpacketsizesent/(double)(pPacket->remainpacketsize))*pPacket->packetpayloadsize);
										if(partialpayloadSentWithThisCall <= pPacket->packetpayloadsize) {
											//statsLocker.Lock();
											m_actualPayloadSizeSent += partialpayloadSentWithThisCall;
											//statsLocker.Unlock();
										} else {
											ASSERT(0);
										}
										pPacket->packetpayloadsize -= partialpayloadSentWithThisCall;
										sendblenWithoutControlPacket -= partialpacketsizesent-packetheadersize;
									}
									pPacket->newdatapacket=false; //to be sure if sending two (mini)data packets
									pPacket->remainpacketsize -= partialpacketsizesent;
								}

								// Log send bytes in correct class
								if(sumofnocontrolpacketsizesent/*m_currentPacket_is_controlpacket == false*/) {
									sentStandardPacketBytesThisCall += sumofnocontrolpacketsizesent/*result*/;
									if(sumofpacketpartfilesizesent) {
										m_numberOfSentBytesPartFile += sumofpacketpartfilesizesent/*result*/;
										m_numberOfSentBytesCompleteFile += sumofnocontrolpacketsizesent-sumofpacketpartfilesizesent/*result*/;
									} else {
										m_numberOfSentBytesCompleteFile += sumofnocontrolpacketsizesent/*result*/;
									}
								}
								sentControlPacketBytesThisCall += result-sumofnocontrolpacketsizesent;
								// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
								IPHeaderThisCall+= (((sentControlPacketBytesThisCall+sentStandardPacketBytesThisCall)/minFragSize)+(((sentControlPacketBytesThisCall+sentStandardPacketBytesThisCall)%minFragSize)?1:0))*(20+20); //Header
								//Xman end
#else
								if(m_currentPacket_is_controlpacket == false) {
									uint32 packetheadersize=0;
									if(newdatapacket)
									{
										//Header -> 6 Bytes
										//FileID -> 16 Byte
										//OP_SENDINGPART: 4 + 4 ->30
										//OP_COMPRESSEDPART: 4 + 4 ->30
										//OP_SENDINGPART_I64: 8 + 8 -> 36
										//OP_COMPRESSEDPART_I64: 8 + 4 -> 32
										switch(sendingdata_opcode)
										{
										case OP_SENDINGPART:
										case OP_COMPRESSEDPART:
											{
												packetheadersize=  (result > 30) ? 30 : result;
												break;
											}
										case OP_COMPRESSEDPART_I64:
											{
												packetheadersize=  (result > 32) ? 32 : result;
												break;
											}
										case OP_SENDINGPART_I64:
											{
												packetheadersize=  (result > 36) ? 36 : result;
												break;
											}
										default:
											ASSERT(0);
										}
									}
									if(m_currentPackageIsFromPartFile == true) {
										m_numberOfSentBytesPartFile += (result-packetheadersize);
									} else {
										m_numberOfSentBytesCompleteFile += (result-packetheadersize);
									}
									sentStandardPacketBytesThisCall += (result-packetheadersize);
									sentControlPacketBytesThisCall += packetheadersize;
								} else {
									sentControlPacketBytesThisCall += result;
									//m_numberOfSentBytesControlPacket += result; //Xman unused
								}
								newdatapacket=false; //to be sure if sending two (mini)data packets
								// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
								IPHeaderThisCall+= (20+20); //Header
								//Xman end
#endif
								// <== Dynamic Socket Buffering [SiRoB] - Mephisto
							}//end if(result>0)
						}
					}

					if (sent == sendblen){
						// we are done sending the current package. Delete it and set
						// sendbuffer to NULL so a new packet can be fetched.
						delete[] sendbuffer;
						sendbuffer = NULL;
						sendblen = 0;


						// ==> Dynamic Socket Buffering [SiRoB] - Mephisto
#ifdef DONT_USE_SOCKET_BUFFERING
						if(!m_currentPacket_is_controlpacket) {
							m_actualPayloadSizeSent += m_actualPayloadSize;
							m_actualPayloadSize = 0;
						}
#endif
						// <== Dynamic Socket Buffering [SiRoB] - Mephisto

						sent = 0;
					}
			}
		}

		// ==> Dynamic Socket Buffering [SiRoB] - Mephisto
#ifndef DONT_USE_SOCKET_BUFFERING
		if(onlyAllowedToSendControlPacket && (!controlpacket_queue.IsEmpty() || sendblenWithoutControlPacket != sendblen - sent /*m_currentPacket_is_controlpacket == true*/)) {
#else
		if(onlyAllowedToSendControlPacket && (!controlpacket_queue.IsEmpty() || sendbuffer != NULL && m_currentPacket_is_controlpacket)) {
#endif
		// <== Dynamic Socket Buffering [SiRoB] - Mephisto
		    // enter control packet send queue
		    // we might enter control packet queue several times for the same package,
		    // but that costs very little overhead. Less overhead than trying to make sure
		    // that we only enter the queue once.
			theApp.uploadBandwidthThrottler->QueueForSendingControlPacket(this, HasSent());
		}

		// - Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
		sentControlPacketBytesThisCall+=IPHeaderThisCall;
		//Xman end

		sendLocker.Unlock();

		SocketSentBytes returnVal = { !anErrorHasOccured, sentStandardPacketBytesThisCall, sentControlPacketBytesThisCall };
		return returnVal;
}

uint32 CEMSocket::GetNextFragSize(uint32 current, uint32 minFragSize) {
    if(current % minFragSize == 0) {
        return current;
    } else {
        return minFragSize*(current/minFragSize+1);
    }
}

/**
 * Decides the (minimum) amount the socket needs to send to prevent timeout.
 * 
 * @author SlugFiller
 */
/* //Xman Xtreme Upload
uint32 CEMSocket::GetNeededBytes() {
	sendLocker.Lock();
	if (byConnected == ES_DISCONNECTED) {
		sendLocker.Unlock();
		return 0;
	}

    if (!((sendbuffer && !m_currentPacket_is_controlpacket) || !standartpacket_queue.IsEmpty())) {
    	// No standard packet to send. Even if data needs to be sent to prevent timout, there's nothing to send.
        sendLocker.Unlock();
		return 0;
	}

	if (((sendbuffer && !m_currentPacket_is_controlpacket)) && !controlpacket_queue.IsEmpty())
		m_bAccelerateUpload = true;	// We might be trying to send a block request, accelerate packet

	uint32 sendgap = ::GetTickCount() - lastCalledSend;

	uint64 timetotal = m_bAccelerateUpload?45000:90000;
	uint64 timeleft = ::GetTickCount() - lastFinishedStandard;
	uint64 sizeleft, sizetotal;
	if (sendbuffer && !m_currentPacket_is_controlpacket) {
		sizeleft = sendblen-sent;
		sizetotal = sendblen;
	}
	else {
		sizeleft = sizetotal = standartpacket_queue.GetHead().packet->GetRealPacketSize();
	}
	sendLocker.Unlock();

	if (timeleft >= timetotal)
		return (UINT)sizeleft;
	timeleft = timetotal-timeleft;
	if (timeleft*sizetotal >= timetotal*sizeleft) {
		// don't use 'GetTimeOut' here in case the timeout value is high,
		if (sendgap > SEC2MS(20))
			return 1;	// Don't let the socket itself time out - Might happen when switching from spread(non-focus) slot to trickle slot
		return 0;
	}
	uint64 decval = timeleft*sizetotal/timetotal;
	if (!decval)
		return (UINT)sizeleft;
	if (decval < sizeleft)
		return (UINT)(sizeleft-decval+1);	// Round up
	else
		return 1;
}
*/

// pach2:
// written this overriden Receive to handle transparently FIN notifications coming from calls to recv()
// This was maybe(??) the cause of a lot of socket error, notably after a brutal close from peer
// also added trace so that we can debug after the fact ...
int CEMSocket::Receive(void* lpBuf, int nBufLen, int nFlags)
{
//	EMTrace("CEMSocket::Receive on %d, maxSize=%d",(SOCKET)this,nBufLen);
	int recvRetCode = CEncryptedStreamSocket::Receive(lpBuf,nBufLen,nFlags); // deadlake PROXYSUPPORT - changed to AsyncSocketEx
	switch (recvRetCode) {
	case 0:
		if (GetRealReceivedBytes() > 0) // we received data but it was for the underlying encryption layer - all fine
			return 0;
		//EMTrace("CEMSocket::##Received FIN on %d, maxSize=%d",(SOCKET)this,nBufLen);
		// FIN received on socket // Connection is being closed by peer
		//ASSERT (false);
		//Xman improved socket closing
		//removed this old patch. At least the FD-WRITE is wrong here
		/*
		if ( 0 == AsyncSelect(FD_CLOSE|FD_WRITE) ) { // no more READ notifications ...
			//int waserr = GetLastError(); // oups, AsyncSelect failed !!!
			ASSERT(false);
		}
		*/
		//Xman end
		return 0;
	case SOCKET_ERROR:
		switch(GetLastError()) {
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

void CEMSocket::RemoveAllLayers()
{
	CEncryptedStreamSocket::RemoveAllLayers();
	delete m_pProxyLayer;
	m_pProxyLayer = NULL;
}

int CEMSocket::OnLayerCallback(const CAsyncSocketExLayer *pLayer, int nType, int nCode, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	ASSERT( pLayer );
	if (nType == LAYERCALLBACK_STATECHANGE)
	{
		/*CString logline;
		if (pLayer==m_pProxyLayer)
		{
		//logline.Format(_T("ProxyLayer changed state from %d to %d"), wParam, nCode);
		//AddLogLine(false,logline);
		}else
		//logline.Format(_T("Layer @ %d changed state from %d to %d"), pLayer, wParam, nCode);
		//AddLogLine(false,logline);*/
		return 1;
	}
	else if (nType == LAYERCALLBACK_LAYERSPECIFIC)
	{
		if (pLayer == m_pProxyLayer)
		{
			switch (nCode)
			{
			case PROXYERROR_NOCONN:
				// We failed to connect to the proxy.
				m_bProxyConnectFailed = true;
				/* fall through */
			case PROXYERROR_REQUESTFAILED:
				// We are connected to the proxy but it failed to connect to the peer.
				if (thePrefs.GetVerbose()) {
					m_strLastProxyError = GetProxyError(nCode);
					if (lParam && ((LPCSTR)lParam)[0] != '\0') {
						m_strLastProxyError += _T(" - ");
						m_strLastProxyError += (LPCSTR)lParam;
					}
					// Appending the Winsock error code is actually not needed because that error code
					// gets reported by to the original caller anyway and will get reported eventually
					// by calling 'GetFullErrorMessage',
					/*if (wParam) {
					CString strErrInf;
					if (GetErrorMessage(wParam, strErrInf, 1))
					m_strLastProxyError += _T(" - ") + strErrInf;
					}*/
				}
				break;
			default:
				m_strLastProxyError = GetProxyError(nCode);
				LogWarning(false, _T("Proxy-Error: %s"), m_strLastProxyError);
			}
		}
	}
	return 1;
}

/**
 * Removes all packets from the standard queue that don't have to be sent for the socket to be able to send a control packet.
 *
 * Before a socket can send a new packet, the current packet has to be finished. If the current packet is part of
 * a split packet, then all parts of that split packet must be sent before the socket can send a control packet.
 *
 * This method keeps in standard queue only those packets that must be sent (rest of split packet), and removes everything
 * after it. The method doesn't touch the control packet queue.
 */
void CEMSocket::TruncateQueues() {
    sendLocker.Lock();

    // Clear the standard queue totally
    // Please note! There may still be a standardpacket in the sendbuffer variable!
	for(POSITION pos = standartpacket_queue.GetHeadPosition(); pos != NULL; )
		delete standartpacket_queue.GetNext(pos).packet;
	standartpacket_queue.RemoveAll();

    sendLocker.Unlock();
}

#ifdef _DEBUG
void CEMSocket::AssertValid() const
{
	CEncryptedStreamSocket::AssertValid();

	const_cast<CEMSocket*>(this)->sendLocker.Lock();

	ASSERT( byConnected==ES_DISCONNECTED || byConnected==ES_NOTCONNECTED || byConnected==ES_CONNECTED );
	CHECK_BOOL(m_bProxyConnectFailed);
	CHECK_PTR(m_pProxyLayer);
	(void)downloadLimit;
	CHECK_BOOL(downloadLimitEnable);
	CHECK_BOOL(pendingOnReceive);
	//char pendingHeader[PACKET_HEADER_SIZE];
	pendingHeaderSize;
	CHECK_PTR(pendingPacket);
	(void)pendingPacketSize;
	CHECK_ARR(sendbuffer, sendblen);
	(void)sent;
	controlpacket_queue.AssertValid();
	standartpacket_queue.AssertValid();
	// ==> Dynamic Socket Buffering [SiRoB] - Mephisto
#ifndef DONT_USE_SOCKET_BUFFERING
	m_currentPacket_in_buffer_list.AssertValid();
#else
	CHECK_BOOL(m_currentPacket_is_controlpacket);
#endif
	// <== Dynamic Socket Buffering [SiRoB] - Mephisto
    //(void)sendLocker;
    (void)m_numberOfSentBytesCompleteFile;
    (void)m_numberOfSentBytesPartFile;
    //Xman unused
    /*
    (void)m_numberOfSentBytesControlPacket;
    */
    //Xman end
	// ==> Dynamic Socket Buffering [SiRoB] - Mephisto
#ifndef DONT_USE_SOCKET_BUFFERING
    (void)sendblenWithoutControlPacket;
#else
    CHECK_BOOL(m_currentPackageIsFromPartFile);
#endif
	// <== Dynamic Socket Buffering [SiRoB] - Mephisto
    (void)lastCalledSend;
	// ==> Dynamic Socket Buffering [SiRoB] - Mephisto
#ifdef DONT_USE_SOCKET_BUFFERING
    (void)m_actualPayloadSize;
#endif
	// <== Dynamic Socket Buffering [SiRoB] - Mephisto
    (void)m_actualPayloadSizeSent;

	const_cast<CEMSocket*>(this)->sendLocker.Unlock();
}
#endif

#ifdef _DEBUG
void CEMSocket::Dump(CDumpContext& dc) const
{
	CEncryptedStreamSocket::Dump(dc);
}
#endif

void CEMSocket::DataReceived(const BYTE*, UINT)
{
	ASSERT(0);
}

UINT CEMSocket::GetTimeOut() const
{
	return m_uTimeOut;
}

void CEMSocket::SetTimeOut(UINT uTimeOut)
{
	m_uTimeOut = uTimeOut;
}

CString CEMSocket::GetFullErrorMessage(DWORD nErrorCode)
{
	CString strError;

	// Proxy error
	if (!GetLastProxyError().IsEmpty())
	{
		strError = GetLastProxyError();
		// If we had a proxy error and the socket error is WSAECONNABORTED, we just 'aborted'
		// the TCP connection ourself - no need to show that self-created error too.
		if (nErrorCode == WSAECONNABORTED)
			return strError;
	}

	// Winsock error
	if (nErrorCode)
	{
		if (!strError.IsEmpty())
			strError += _T(": ");
		strError += GetErrorMessage(nErrorCode, 1);
	}

	return strError;
}

// ==> Dynamic Socket Buffering [SiRoB] - Mephisto
#ifdef DONT_USE_SOCKET_BUFFERING
// increases the send buffer to a bigger size
bool CEMSocket::UseBigSendBuffer()
{
#define BIGSIZE 128 * 1024
	if (m_bUsesBigSendBuffers)
		return true;
	m_bUsesBigSendBuffers = true;
    int val = BIGSIZE;
    int vallen = sizeof(int);
	int oldval = 0;
	GetSockOpt(SO_SNDBUF, &oldval, &vallen);
	if (val > oldval)
		SetSockOpt(SO_SNDBUF, &val, sizeof(int));
	val = 0;
	vallen = sizeof(int);
	GetSockOpt(SO_SNDBUF, &val, &vallen);
#if defined(_DEBUG) || defined(_BETA)
	if (val == BIGSIZE)
		theApp.QueueDebugLogLine(false, _T("Increased Sendbuffer for uploading socket from %uKB to %uKB"), oldval/1024, val/1024);
	else
		theApp.QueueDebugLogLine(false, _T("Failed to increase Sendbuffer for uploading socket, stays at %uKB"), oldval/1024);
#endif
	return val == BIGSIZE;
}
#endif
// <== Dynamic Socket Buffering [SiRoB] - Mephisto

//Xman 4.8.2
//Threadsafe Statechange
void CEMSocket::SetConnectedState(const uint8 state)
{
	if(byConnected == state)
		return;

	sendLocker.Lock();
	byConnected = state;
	sendLocker.Unlock();
}
//Xman end
// netfinity: Maximum Segment Size (MSS - Vista only) //added by zz_fly
void CEMSocket::SetMSSFromSocket(SOCKET socket){
#ifdef HAVE_VISTA_SDK
	m_dwMSS = theNetF.GetMSSFromSocket(socket);
#else
	m_dwMSS = 0;
#endif
}
// netfinity: end
