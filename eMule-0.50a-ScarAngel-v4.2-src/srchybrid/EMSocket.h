//this file is part of eMule
//Copyright (C)2002-2010 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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

class CAsyncProxySocketLayer;
class Packet;

#define ES_DISCONNECTED		0xFF
#define ES_NOTCONNECTED		0x00
#define ES_CONNECTED		0x01

#define PACKET_HEADER_SIZE	6

//Xman Xtreme Upload
// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
class CUpDownClient;
// Maella end

struct StandardPacketQueueEntry {
	uint32 actualPayloadSize;
	Packet* packet;
};

class CEMSocket : public CEncryptedStreamSocket, public ThrottledFileSocket // ZZ:UploadBandWithThrottler (UDP)
{
	DECLARE_DYNAMIC(CEMSocket)
public:
	CEMSocket();
	virtual ~CEMSocket();

	virtual void 	SendPacket(Packet* packet, bool delpacket = true, bool controlpacket = true, uint32 actualPayloadSize = 0, bool bForceImmediateSend = false);
	bool	IsConnected() const {return byConnected == ES_CONNECTED;}
	uint8	GetConState() const {return byConnected;}
	virtual bool IsRawDataMode() const { return false; }
	void	SetDownloadLimit(uint32 limit);
	void	DisableDownloadLimit();
	BOOL	AsyncSelect(long lEvent);
	virtual bool IsBusy() const			{return m_bBusy;}
    virtual bool HasQueues() const		{return (sendbuffer || standartpacket_queue.GetCount() > 0 || controlpacket_queue.GetCount() > 0);} // not trustworthy threaded? but it's ok if we don't get the correct result now and then
	virtual bool UseBigSendBuffer();

	virtual UINT GetTimeOut() const;
	virtual void SetTimeOut(UINT uTimeOut);

	virtual BOOL Connect(LPCSTR lpszHostAddress, UINT nHostPort);
	virtual BOOL Connect(SOCKADDR* pSockAddr, int iSockAddrLen);

	void InitProxySupport();
	virtual void RemoveAllLayers();
	const CString GetLastProxyError() const { return m_strLastProxyError; }
	bool GetProxyConnectFailed() const { return m_bProxyConnectFailed; }

	CString GetFullErrorMessage(DWORD dwError);

	DWORD GetLastCalledSend() { return lastCalledSend; }
	uint64 GetSentBytesCompleteFileSinceLastCallAndReset();
	uint64 GetSentBytesPartFileSinceLastCallAndReset();
	//Xman unused
	/*
	uint64 GetSentBytesControlPacketSinceLastCallAndReset();
	*/
	//Xman end
	uint64 GetSentPayloadSinceLastCallAndReset();
	void TruncateQueues();

    virtual SocketSentBytes SendControlData(uint32 maxNumberOfBytesToSend, uint32 minFragSize) { return Send(maxNumberOfBytesToSend, minFragSize, true); };
    virtual SocketSentBytes SendFileAndControlData(uint32 maxNumberOfBytesToSend, uint32 minFragSize) { return Send(maxNumberOfBytesToSend, minFragSize, false); };


	//Xman Xtreme Upload
	/*
    uint32	GetNeededBytes();
	*/
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	CUpDownClient*	client; // Quick and dirty
	// Maella end

	//Xman Full Chunk
	//bool StandardPacketQueueIsEmpty() const {return standartpacket_queue.IsEmpty()!=0;}
	bool StandardPacketQueueIsEmpty() const {return standartpacket_queue.IsEmpty()!=0 && (sendbuffer==NULL || sendbuffer!=NULL && m_currentPacket_is_controlpacket==true);} //Xman 4.3
	//remark: this method isn't threadsave at uploadclient... but there is no need for
	// this method is threadsave at uploadbandwidththrottler!

	//Xman include ACK
	//zz_fly
	//netfinity: Special case when socket is closing but data still in buffer, need to empty buffer or deadlock forever
	/*
	void ProcessReceiveData();
	*/
	void ProcessReceiveData(int nErrorCode = 0);
	//zz_fly end

	//Xman count block/success send
	typedef CList<float> BlockHistory;
	BlockHistory m_blockhistory;
	float avg_block_ratio; //the average block of last 20 seconds
	float sum_blockhistory; //the sum of all stored ratio samples

	uint32 blockedsendcount;
	uint32 sendcount;
	uint32 blockedsendcount_overall;
	uint32 sendcount_overall;

	float GetBlockRatio_overall() const {return sendcount_overall>0 ? 100.0f*blockedsendcount_overall/sendcount_overall : 0.0f;}

	float GetBlockRatio() const {return avg_block_ratio;}
	float GetandStepBlockRatio() {
			float newsample  = sendcount>0 ? 100.0f*blockedsendcount/sendcount : 0.0f;
			m_blockhistory.AddHead(newsample);
			sum_blockhistory += newsample;
			if(m_blockhistory.GetSize()>HISTORY_SIZE) // ~ 20 seconds
			{
				const float& substract = m_blockhistory.RemoveTail(); //susbtract the old element
				sum_blockhistory -= substract;
				if(sum_blockhistory<0)
					sum_blockhistory=0; //fix possible rounding error
			}
			blockedsendcount=0;
			sendcount=0;
			avg_block_ratio = sum_blockhistory / m_blockhistory.GetSize();
			return avg_block_ratio;
	}
	//Xman end count block/success send

	//Xman 
	//Threadsafe Statechange
	void			SetConnectedState(const uint8 state);

	// netfinity: Maximum Segment Size (MSS - Vista only) //added by zz_fly
	void		SetMSSFromSocket(SOCKET socket);

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
	virtual void	OnSend(int nErrorCode);
	virtual void	OnReceive(int nErrorCode);
	//Xman
	virtual void	OnConnect(int nErrorCode); // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	//Xman end

	uint8	byConnected;
	UINT	m_uTimeOut;
	bool	m_bProxyConnectFailed;
	CAsyncProxySocketLayer* m_pProxyLayer;
	CString m_strLastProxyError;

private:
    virtual SocketSentBytes Send(uint32 maxNumberOfBytesToSend, uint32 minFragSize, bool onlyAllowedToSendControlPacket);
	void	ClearQueues();
	virtual int Receive(void* lpBuf, int nBufLen, int nFlags = 0);

    uint32 GetNextFragSize(uint32 current, uint32 minFragSize);
    bool    HasSent() { return m_hasSent; }

	//Xman Code Improvement
	bool	isreadyforsending;

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
	uint32	sendblen;
	uint32	sent;

	CTypedPtrList<CPtrList, Packet*> controlpacket_queue;
	CList<StandardPacketQueueEntry> standartpacket_queue;
	bool m_currentPacket_is_controlpacket;
	CCriticalSection sendLocker;
	uint64 m_numberOfSentBytesCompleteFile;
	uint64 m_numberOfSentBytesPartFile;
	//Xman unused
	/*
	uint64 m_numberOfSentBytesControlPacket;
	*/
	//Xman end
	bool m_currentPackageIsFromPartFile;
	//Xman unused
	/*
	bool m_bAccelerateUpload;
	*/
	//Xman end
	DWORD lastCalledSend;
    DWORD lastSent;
	//Xman unused
	/*
	uint32 lastFinishedStandard;
	*/
	//Xman end
	uint32 m_actualPayloadSize;
	uint32 m_actualPayloadSizeSent;
    bool m_bBusy;
    bool m_hasSent;
	bool m_bUsesBigSendBuffers;
};
