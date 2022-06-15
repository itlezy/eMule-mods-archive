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
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#pragma once
#include "EncryptedStreamSocket.h"
#include "OtherFunctions.h"
#include "ThrottledSocket.h" // ZZ:UploadBandWithThrottler (UDP)

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
class CNatSocket;
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
class CAsyncProxySocketLayer;
class Packet;
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
class CReceiveData;
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --

#define ES_DISCONNECTED		0xFF
#define ES_NOTCONNECTED		0x00
#define ES_CONNECTED		0x01

#define PACKET_HEADER_SIZE	6

struct StandardPacketQueueEntry {
	uint32 actualPayloadSize;
	Packet* packet;
};

#if !defined DONT_USE_SOCKET_BUFFERING // NEO: DSB - [DynamicSocketBuffer] -- Xanatos -->
struct BufferedPacket {
	UINT	remainpacketsize;
	UINT	packetpayloadsize;
	bool	iscontrolpacket;
	bool	isforpartfile;
};
#endif // NEO: DSB - [DynamicSocketBuffer] <-- Xanatos --

class CEMSocket : public CEncryptedStreamSocket, public ThrottledFileSocket // ZZ:UploadBandWithThrottler (UDP)
{
	DECLARE_DYNAMIC(CEMSocket)
public:
	CEMSocket();
	virtual ~CEMSocket();

#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
	virtual void 	SendPacket(Packet* packet, bool delpacket = true, bool controlpacket = true, uint32 actualPayloadSize = 0);
#else
	virtual void 	SendPacket(Packet* packet, bool delpacket = true, bool controlpacket = true, uint32 actualPayloadSize = 0, bool bForceImmediateSend = false);
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
	bool	IsConnected() const {return byConnected == ES_CONNECTED;}
	uint8	GetConState() const {return byConnected;}
	virtual bool IsRawDataMode() const { return false; }
	void	SetDownloadLimit(uint32 limit);
	void	DisableDownloadLimit();
	BOOL	AsyncSelect(long lEvent);
	// NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
	void	ReceiveData();
	void	ProcessReceivedData(char *rptr,const char *rend, bool &bPacketResult);
#ifdef NEO_DBT 
	void	SetPriorityReceive(UINT is) {priorityReceive = is;}
	bool	IsPriorityReceive() {return priorityReceive == TRUE;}
	bool	IsImmediateReceive() {return priorityReceive == 2;}

	virtual int Receive(uint32 size, UINT hyperreceiving = FALSE);
	bool	ProcessData(bool ignore = false);

	virtual bool IsEmpty() const {return (pendingOnReceive == false);}
#endif // NEO_DBT 
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
 #if !defined DONT_USE_SOCKET_BUFFERING // NEO: DSB - [DynamicSocketBuffer]
	//bool	ControlPacketQueueIsEmpty() const		{return controlpacket_queue.IsEmpty() && (sendblen == 0 || !m_currentPacket_in_buffer_list.GetHead()->iscontrolpacket);}
	//bool	StandardPacketQueueIsEmpty() const		{return standartpacket_queue.IsEmpty() && (sendblen == 0 || m_currentPacket_in_buffer_list.GetHead()->iscontrolpacket);}
	bool	ControlPacketQueueIsEmpty() const		{return controlpacket_queue.IsEmpty() && (sendblen == 0 ||  (sendblenWithoutControlPacket == sendblen - sent));}
	bool	StandardPacketQueueIsEmpty() const		{return standartpacket_queue.IsEmpty() && (sendblen == 0 || (sendblenWithoutControlPacket != sendblen - sent));}
 #else
	bool	ControlPacketQueueIsEmpty() const		{return controlpacket_queue.IsEmpty() && (sendbuffer==NULL || !m_currentPacket_is_controlpacket);}
	bool	StandardPacketQueueIsEmpty() const		{return standartpacket_queue.IsEmpty() && (sendbuffer==NULL || m_currentPacket_is_controlpacket);}
 #endif // NEO: DSB

	bool	IsSending() const { return (sendbuffer != NULL);}
	void	SetPrioritySend(UINT is) {prioritySend = is;}
	bool	IsPrioritySend() {return prioritySend == TRUE;}
	bool	IsImmediateSend() {return prioritySend == 2;}
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --

	// NEO: DSB - [DynamicSocketBuffer] -- Xanatos -->
	void	SetSendBufferSize(uint32 SendBufferSize = 0);
	void	SetRecvBufferSize(uint32 RecvBufferSize = 0);
#if !defined DONT_USE_SOCKET_BUFFERING
	void	SetSocketBufferLimit(uint32 limit) { bufferlimit = limit; }
	uint32	GetSendBufferSize() { return m_uCurrentSendBufferSize; };
	uint32	GetRecvBufferSize() { return m_uCurrentRecvBufferSize; };
#endif 
	// NEO: DSB <-- Xanatos --

	// NEO: NDBT END <-- Xanatos --
	virtual bool IsBusy() const			{return m_bBusy;}
    virtual bool HasQueues() const		{return (sendbuffer || standartpacket_queue.GetCount() > 0 || controlpacket_queue.GetCount() > 0);} // not trustworthy threaded? but it's ok if we don't get the correct result now and then

	virtual UINT GetTimeOut() const;
	virtual void SetTimeOut(UINT uTimeOut);

	virtual BOOL Connect(LPCSTR lpszHostAddress, UINT nHostPort);
	virtual BOOL Connect(SOCKADDR* pSockAddr, int iSockAddrLen);

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
	CNatSocket* InitNatSupport();
	CNatSocket* GetNatLayer()	{return m_pNatLayer;}
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
	void InitProxySupport();
	virtual void RemoveAllLayers();
	const CString GetLastProxyError() const { return m_strLastProxyError; }
	bool GetProxyConnectFailed() const { return m_bProxyConnectFailed; }

	CString GetFullErrorMessage(DWORD dwError);

	DWORD GetLastCalledSend() { return lastCalledSend; }
	uint64 GetSentBytesCompleteFileSinceLastCallAndReset();
	uint64 GetSentBytesPartFileSinceLastCallAndReset();
	uint64 GetSentBytesControlPacketSinceLastCallAndReset();
	uint64 GetSentPayloadSinceLastCallAndReset();
	void TruncateQueues();

#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
    virtual SocketSentBytes Send(uint32 maxNumberOfBytesToSend, uint32 minFragSize, bool onlyAllowedToSendControlPacket = false, uint16 maxNumberOfPacketsToSend = 0xFFFF);
#else
    virtual SocketSentBytes SendControlData(uint32 maxNumberOfBytesToSend, uint32 minFragSize) { return Send(maxNumberOfBytesToSend, minFragSize, true); };
    virtual SocketSentBytes SendFileAndControlData(uint32 maxNumberOfBytesToSend, uint32 minFragSize) { return Send(maxNumberOfBytesToSend, minFragSize, false); };
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --

    uint32	GetNeededBytes();
#ifdef _DEBUG
	// Diagnostic Support
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual int	OnLayerCallback(const CAsyncSocketExLayer *pLayer, int nType, int nCode, WPARAM wParam, LPARAM lParam);
	
	virtual void	DataReceived(const BYTE* pcData, UINT uSize);
	virtual bool	PacketReceived(Packet* packet) = 0;
	virtual void	OnError(int nErrorCode) = 0;
	virtual void	OnClose(int nErrorCode);
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
	virtual void	OnConnect(int nErrorCode);
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
	virtual void	OnSend(int nErrorCode);
	virtual void	OnReceive(int nErrorCode);

	uint8	byConnected;
	UINT	m_uTimeOut;

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
	CNatSocket* m_pNatLayer;
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
	bool	m_bProxyConnectFailed;
	CAsyncProxySocketLayer* m_pProxyLayer;
	CString m_strLastProxyError;
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
	CTypedPtrList<CPtrList, CReceiveData*> downloaddata_queue;
    CCriticalSection receiveLocker;
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --

private:
#if !defined(NEO_UBT) // NEO: NUBT - [NeoUploadBandwidthThrottler] <-- Xanatos --
    virtual SocketSentBytes Send(uint32 maxNumberOfBytesToSend, uint32 minFragSize, bool onlyAllowedToSendControlPacket);
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
	void	ClearQueues();
	virtual int Receive(void* lpBuf, int nBufLen, int nFlags = 0);

    uint32 GetNextFragSize(uint32 current, uint32 minFragSize);
    bool    HasSent() { return m_hasSent; }

	// Download (pseudo) rate control
	uint32	downloadLimit;
	bool	downloadLimitEnable;
	bool	pendingOnReceive;

	// Download partial header
	char	pendingHeader[PACKET_HEADER_SIZE];	// actually, this holds only 'PACKET_HEADER_SIZE-1' bytes.
	uint32	pendingHeaderSize;

	// Download partial packet
	Packet* pendingPacket;
	uint32	pendingPacketSize;

	// Upload control
	char*	sendbuffer;
#if !defined DONT_USE_SOCKET_BUFFERING // NEO: DSB - [DynamicSocketBuffer] -- Xanatos -->
	uint32	m_uCurrentRecvBufferSize;
	uint32	m_uCurrentSendBufferSize;
	uint32 currentBufferSize;
#endif // NEO: DSB - [DynamicSocketBuffer] <-- Xanatos --
	uint32	sendblen;
	uint32	sent;

	CTypedPtrList<CPtrList, Packet*> controlpacket_queue;
	CList<StandardPacketQueueEntry> standartpacket_queue;
#if !defined DONT_USE_SOCKET_BUFFERING // NEO: DSB - [DynamicSocketBuffer] -- Xanatos -->
	CList<BufferedPacket*> m_currentPacket_in_buffer_list;
#else
	bool m_currentPacket_is_controlpacket;
#endif // NEO: DSB - [DynamicSocketBuffer] <-- Xanatos --
	CCriticalSection sendLocker;
	uint64 m_numberOfSentBytesCompleteFile;
	uint64 m_numberOfSentBytesPartFile;
	uint64 m_numberOfSentBytesControlPacket;
#if defined DONT_USE_SOCKET_BUFFERING // NEO: DSB - [DynamicSocketBuffer] <-- Xanatos --
	bool m_currentPackageIsFromPartFile;
#endif // NEO: DSB <-- Xanatos --
	bool m_bAccelerateUpload;
	DWORD lastCalledSend;
    DWORD lastSent;
	uint32 lastFinishedStandard;
#if !defined DONT_USE_SOCKET_BUFFERING // NEO: DSB - [DynamicSocketBuffer] -- Xanatos -->
	uint32 sendblenWithoutControlPacket; //Used to know if a controlpacket is already buffered
	uint32 bufferlimit;
#else
	uint32 m_actualPayloadSize;
#endif // NEO: DSB - [DynamicSocketBuffer] <-- Xanatos --
	uint32 m_actualPayloadSizeSent;
    bool m_bBusy;
    bool m_hasSent;
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
    UINT	priorityReceive;
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
    UINT	prioritySend;
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
};
