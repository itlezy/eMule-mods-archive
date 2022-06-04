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
#include "AsyncSocketEx.h"// X: [SUDPS] - [CAsyncSocketEx UDP Support]
#include "UploadBandwidthThrottler.h" // ZZ:UploadBandWithThrottler (UDP)
#include "EncryptedDatagramSocket.h"

class Packet;

struct UDPPack // netfinity: Rearranged for alignment reasons
{
	uchar	pachTargetClientHashORKadID[16];
	Packet* packet;
	uint32	dwTime;
	uint32  nReceiverVerifyKey;
	uint32	dwIP;
	uint16	nPort;
	bool	bEncrypt;
	bool	bKad;
	//uint16 nPriority; We could add a priority system here to force some packets.
};

class CClientUDPSocket : public CAsyncSocketEx, public CEncryptedDatagramSocket, public ThrottledControlSocket // ZZ:UploadBandWithThrottler (UDP)// X: [SUDPS] - [CAsyncSocketEx UDP Support]
{
public:
	CClientUDPSocket();
	virtual ~CClientUDPSocket();

	bool	IsSocketUploading() const {return false;} //Xman Xtreme Upload

	bool	Create();
	bool	Rebind();

	uint16	GetConnectedPort()			{ return m_port; }
	bool	SendPacket(Packet* packet, uint32 dwIP, uint16 nPort, bool bEncrypt, const uchar* pachTargetClientHashORKadID, bool bKad, uint32 nReceiverVerifyKey);
    SocketSentBytes  SendControlData(uint32 maxNumberOfBytesToSend, uint32 minFragSize); // ZZ:UploadBandWithThrottler (UDP)

protected:
	bool	ProcessPacket(const BYTE* packet, UINT size, uint8 opcode, uint32 ip, uint16 port);
	
	virtual void	OnSend(int nErrorCode);	
	virtual void	OnReceive(int nErrorCode);

private:
	int		SendTo(char* lpBuf,int nBufLen,uint32 dwIP, uint16 nPort);
    bool	IsBusy() const { return m_bWouldBlock; }
	bool	m_bWouldBlock;
	uint16	m_port;

	CAtlList<UDPPack*> controlpacket_queue;

	Poco::FastMutex sendLocker; // ZZ:UploadBandWithThrottler (UDP)
};
