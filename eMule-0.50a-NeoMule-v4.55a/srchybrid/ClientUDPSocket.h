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
#pragma once
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
#include "Neo/BC/UploadBandwidthThrottler.h" // ZZ:UploadBandWithThrottler (UDP)
#else
#include "UploadBandwidthThrottler.h" // ZZ:UploadBandWithThrottler (UDP)
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
#include "Neo/BC/DownloadBandwidthThrottler.h"
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --
#include "EncryptedDatagramSocket.h"

class Packet;
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
class CReceiveData;
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --

#pragma pack(1)
struct UDPPack
{
	Packet* packet;
	uint32	dwIP;
	uint16	nPort;
	uint32	dwTime;
	bool	bEncrypt;
	bool	bKad;
	uint32  nReceiverVerifyKey;
	uchar	pachTargetClientHashORKadID[16];
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
	bool priority;
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
	//uint16 nPriority; We could add a priority system here to force some packets.
};
#pragma pack()

class CClientUDPSocket : public CAsyncSocket, public CEncryptedDatagramSocket, public ThrottledControlSocket // ZZ:UploadBandWithThrottler (UDP)
{
public:
	friend class CClientReqSocket; // NEO: CI#5 - [CodeImprovement] <-- Xanatos --
	CClientUDPSocket();
	virtual ~CClientUDPSocket();

	bool	Create();
	bool	Rebind();
	uint16	GetConnectedPort()			{ return m_port; }
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
	bool	IsEmpty() const {return m_pendingPackets == 0;}
	int		Receive(uint32 size, UINT hyperreceiving = FALSE);
	bool	ProcessData(bool ignore = false);
	virtual bool	IsPriorityReceive() {return true;} 
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --

	bool	SendPacket(Packet* packet, uint32 dwIP, uint16 nPort, bool bEncrypt, const uchar* pachTargetClientHashORKadID, bool bKad, uint32 nReceiverVerifyKey);
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
    SocketSentBytes  Send(uint32 maxNumberOfBytesToSend, uint32 minFragSize, bool onlyAllowedToSendControlPacket = false, uint16 maxNumberOfPacketsToSend = 0xFFFF);
	bool	ControlPacketQueueIsEmpty() const {return !controlpacket_queue.GetCount();}
	bool	IsPrioritySend();
#else
    SocketSentBytes  SendControlData(uint32 maxNumberOfBytesToSend, uint32 minFragSize); // ZZ:UploadBandWithThrottler (UDP)
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --

protected:
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
	friend class CUDPSocket;

	int		SendTo(char* lpBuf,int nBufLen,uint32 dwIP, uint16 nPort);
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --

	bool	ProcessPacket(const BYTE* packet, UINT size, uint8 opcode, uint32 ip, uint16 port);
	bool	ProcessModPacket(const BYTE* packet, UINT size, uint8 opcode, uint32 ip, uint16 port); // NEO: NMP - [NeoModProt] <-- Xanatos --
	
	virtual void	OnSend(int nErrorCode);	
	virtual void	OnReceive(int nErrorCode);
	bool	isUDP() const {return true;} // NEO: MOD - [ThrottledControlSocket] <-- Xanatos --

	// NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
	void	ProcessReceivedData(BYTE* buffer, int nRealLen, SOCKADDR_IN &sockAddr, int iSockAddrLen, DWORD dwError);
#ifdef NEO_DBT 
	uint16	m_pendingPackets;
	CTypedPtrList<CPtrList, CReceiveData*> downloaddata_queue;
    CCriticalSection receiveLocker;
#endif // NEO_DBT 
	// NEO: NDBT END <-- Xanatos --
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
	POSITION lastPriorityPacket;
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --

private:
#ifndef NATTUNNELING // NEO: NATT - [NatTraversal] <-- Xanatos --
	int		SendTo(char* lpBuf,int nBufLen,uint32 dwIP, uint16 nPort);
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
    bool	IsBusy() const { return m_bWouldBlock; }
	bool	m_bWouldBlock;
	uint16	m_port;

	CTypedPtrList<CPtrList, UDPPack*> controlpacket_queue;

    CCriticalSection sendLocker; // ZZ:UploadBandWithThrottler (UDP)
};
