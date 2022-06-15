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

class CServerConnect;
struct SServerUDPPacket;
struct SServerDNSRequest;
class CUDPSocket;
class Packet;
class CServer;
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
class CReceiveData;
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --


///////////////////////////////////////////////////////////////////////////////
// CUDPSocketWnd

class CUDPSocketWnd : public CWnd
{
// Construction
public:
	CUDPSocketWnd() {};
	CUDPSocket* m_pOwner;

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnDNSLookupDone(WPARAM wParam, LPARAM lParam);
};


///////////////////////////////////////////////////////////////////////////////
// CUDPSocket

class CUDPSocket : public CAsyncSocket, public CEncryptedDatagramSocket, public ThrottledControlSocket // ZZ:UploadBandWithThrottler (UDP)
{
	friend class CServerConnect;

public:
	CUDPSocket();
	virtual ~CUDPSocket();

	bool Create();
	bool Rebind(); // NEO: MOD - [BindToAdapter] <-- Xanatos --
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
	bool	IsEmpty() const {return m_pendingPackets == 0;}
	int		Receive(uint32 size, UINT hyperreceiving = FALSE);
	bool	ProcessData(bool ignore = false);
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
    SocketSentBytes  Send(uint32 maxNumberOfBytesToSend, uint32 minFragSize, bool onlyAllowedToSendControlPacket = false, uint16 maxNumberOfPacketsToSend = 0xFFFF);
	bool	ControlPacketQueueIsEmpty() const {return !controlpacket_queue.GetCount();}
#else
    SocketSentBytes SendControlData(uint32 maxNumberOfBytesToSend, uint32 minFragSize); // ZZ:UploadBandWithThrottler (UDP)
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
	void SendPacket(Packet* packet, CServer* pServer, uint16 nSpecialPort = 0, BYTE* pRawPacket = 0, uint32 nRawLen = 0);
	void DnsLookupDone(WPARAM wp, LPARAM lp);

protected:
	virtual void OnSend(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
	bool	isUDP() const {return true;} // NEO: MOD - [ThrottledControlSocket] <-- Xanatos --

	// NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
	void	ProcessReceivedData(BYTE* buffer, int length, SOCKADDR_IN &sockAddr, int iSockAddrLen, DWORD dwError);
#ifdef NEO_DBT 
	uint16	m_pendingPackets;
	CTypedPtrList<CPtrList, CReceiveData*> downloaddata_queue;
    CCriticalSection receiveLocker;
#endif // NEO_DBT 
	// NEO: NDBT END <-- Xanatos --

private:
	HWND m_hWndResolveMessage;
	CUDPSocketWnd m_udpwnd;
	CTypedPtrList<CPtrList, SServerDNSRequest*> m_aDNSReqs;

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
	void SendBuffer(uint32 nIP, uint16 nPort, BYTE* pPacket, UINT uSize, bool bClient);
#else
	void SendBuffer(uint32 nIP, uint16 nPort, BYTE* pPacket, UINT uSize);
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
	bool ProcessPacket(const BYTE* packet, UINT size, UINT opcode, uint32 nIP, uint16 nUDPPort);
	void ProcessPacketError(UINT size, UINT opcode, uint32 nIP, uint16 nUDPPort, LPCTSTR pszError);
	bool IsBusy() const { return m_bWouldBlock; }
	int SendTo(BYTE* lpBuf, int nBufLen, uint32 dwIP, uint16 nPort);

	bool m_bWouldBlock;
	CTypedPtrList<CPtrList, SServerUDPPacket*> controlpacket_queue;
    CCriticalSection sendLocker; // ZZ:UploadBandWithThrottler (UDP)

	uint16	m_port; // NEO: MOD - [BindToAdapter] <-- Xanatos --
};
