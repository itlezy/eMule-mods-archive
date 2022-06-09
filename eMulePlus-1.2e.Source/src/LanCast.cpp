// This file is part of eMule Plus
//
// LANCAST
//
// Written by moosetea, enjoy
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// This allows lan clients to add each other as peers, it could be
// very buggy and needs alot more work.
//
// Thanks to codeproject for help with the multicast code,
// which works but is a bit of a shambles.

#include "stdafx.h"
#include "emule.h"
#include "updownclient.h"
#include "KnownFile.h"
#include "LanCast.h"
#include "packets.h"
#include "SharedFileList.h"
#include "otherfunctions.h"
#include "SafeFile.h"

CLanCast::CLanCast()
{
	bStarted = false;
	if (GetLancastEnabled())
	{
		udp_timer = SetTimer(0, 4329, 10000, CLanCast::UDPTimerProc);
		Start();
	}
}

bool CLanCast::SendPacket(Packet* packet)
{
	char	*pcSendBuffer = new char[packet->m_dwSize + 2];
	int		iRc;

//	Standard UDP Header
	memcpy(pcSendBuffer, packet->GetUDPHeader(), 2);
//	The packet Data
	memcpy2(pcSendBuffer + 2, packet->m_pcBuffer, packet->m_dwSize);

//	Send the LanCast packet
	iRc = m_SendSocket.SendTo(pcSendBuffer, packet->m_dwSize + 2, (SOCKADDR*)&m_saHostGroup, sizeof(SOCKADDR), 0);
	delete[] pcSendBuffer;
	return (iRc != SOCKET_ERROR);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CLanCast::BroadcastHash(CKnownFile *pKnownFile)
{
	if (GetLancastEnabled())
	{
		Start();

		CMemFile	packetStream(4 + 2 + 16);

		uint32		dwIP = GetLancastIP();
		uint16		uPort = g_App.m_pPrefs->GetPort();

		packetStream.Write(&dwIP, 4);						// <lancastip 4>
		packetStream.Write(&uPort, 2);						// <emuletcpport 2>
		packetStream.Write(pKnownFile->GetFileHash(), 16);	// <filehash 16>

		Packet		*pPacket = new Packet(&packetStream, OP_LANCASTPROT);
		pPacket->m_eOpcode = OP_HASH;

		SendPacket(pPacket);
		delete pPacket;
	}
	else
		Stop();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CLanCast::ReceiveHash(byte *pbytePacketBuf, uint32 dwSize)
{
	CSafeMemFile		packetStream(pbytePacketBuf, dwSize);

	uint32		dwIP = 0;
	uint16		uPort = 0;
	uchar		fileHash[16];

//	Read Hash Data
	packetStream.Read(&dwIP, 4);
	packetStream.Read(&uPort, 2);
	packetStream.Read(&fileHash, 16);

#ifdef _DEBUG
	CString		buffer = HashToString(fileHash);
	AddLogLine(LOG_FL_DBG, _T("Lancast: Just Received %s"), buffer);
#endif

	CPartFile		*pPartFile = g_App.m_pDownloadQueue->GetFileByID(fileHash);

	if (pPartFile != NULL)
	{
#ifdef _DEBUG
		AddLogLine(LOG_FL_DBG, _T("Lancast: Adding source for %s"), buffer);
#endif

	//	Create a new Updown client for this source (with no server info), userid is the LAN ip address
	//	Add this LAN uldl client to the download queue, this also will check for duplicate sockets and "merge" them
		CUpDownClient *pNewSource = g_App.m_pDownloadQueue->CheckAndAddSource(pPartFile, ntohl(dwIP), uPort, 0, 0, NULL);

		if (pNewSource != NULL)
		{
			pNewSource->SetUserName(GetResString(IDS_LANCAST_SOURCE));
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CLanCast::~CLanCast()
{
	Stop();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CLanCast::OnReceive(int iErrorCode)
{
	byte		abyteBuffer[5000];
	SOCKADDR_IN	sockAddr = {0};
	int			iSockAddrLen = sizeof(sockAddr);
	int			iLength = ReceiveFrom(abyteBuffer, sizeof(abyteBuffer), (SOCKADDR*)&sockAddr, &iSockAddrLen);
	NOPRM(iErrorCode);

	if (iLength >= 2)	//SOCKET_ERROR = -1
	{
	//	If Lancast is enabled and this isn't a packet from ourselves
		if (GetLancastEnabled() && (sockAddr.sin_addr.s_addr != GetLancastIP()))
		{
			if (abyteBuffer[0] == OP_LANCASTPROT)
				ProcessPacket(abyteBuffer + 2, iLength - 2, abyteBuffer[1]);
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CLanCast::ProcessPacket(byte *pbytePacketBuf, uint32 dwSize, byte byteOpcode)
{
	try
	{
		switch (byteOpcode)
		{
			case OP_HASH:
				ReceiveHash(pbytePacketBuf, dwSize);
				break;

			default:
				AddLogLine(LOG_RGB_WARNING, IDS_LANCAST_UNK_OPCODE);
				return false;
		}

		return true;
	}
	catch(...)
	{
		AddLogLine(LOG_RGB_ERROR, IDS_LANCAST_ERR_MULITC);
		return false;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CALLBACK CLanCast::UDPTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	NOPRM(hwnd); NOPRM(uMsg); NOPRM(idEvent); NOPRM(dwTime);
	g_App.m_pSharedFilesList->NextLANBroadcast();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CLanCast Private member functions - MultiCast Network Code
bool CLanCast::CreateReceivingSocket(const char *pcGroupIP, uint32 dwGroupPort)
{
//	Create socket for receiving packets from multicast group
	if(!Create(dwGroupPort, SOCK_DGRAM, FD_READ))
		return false;

//	Allow reuse of local port if needed
	BOOL bMultipleApps = TRUE;
	SetSockOpt(SO_REUSEADDR, (void*)&bMultipleApps, sizeof(BOOL), SOL_SOCKET);

//	Fill m_saHostGroup_in for sending datagrams
	memzero(&m_saHostGroup, sizeof(m_saHostGroup));
	m_saHostGroup.sin_family = AF_INET;
	m_saHostGroup.sin_addr.s_addr = inet_addr(pcGroupIP);
	m_saHostGroup.sin_port = fast_htons((USHORT)dwGroupPort);

//	Join the multicast group
	m_mrMReq.imr_multiaddr.s_addr = inet_addr(pcGroupIP);	// group addr
	m_mrMReq.imr_interface.s_addr = fast_htonl(INADDR_ANY);		// use default
	if(setsockopt(m_hSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char FAR *)&m_mrMReq, sizeof(m_mrMReq)) < 0)
		return false;

	return true;
}

bool CLanCast::CreateSendingSocket(uint32 dwTTL, bool bLoopBack)
{
//	Create an unconnected UDP socket
	if(!m_SendSocket.Create(0, SOCK_DGRAM, 0))
		return false;

//	Set Time to Live as specified by user
	if(!SetTTL(dwTTL))
		AddLogLine(LOG_RGB_ERROR, IDS_ERR_TTL);

	SetLoopBack(bLoopBack);

	return true;
}

void CLanCast::Start()
{
	if (!bStarted)
	{
		in_addr host;
		host.s_addr = GetLancastIP();

	//	Try and join the multicast group - 224.0.0.1, using the prefs port and no loopback
		if(!JoinGroup("224.0.0.1", GetLancastPort(), 50, false))
			AddLogLine(LOG_RGB_WARNING, IDS_JOIN_FAILED, host.s_net, host.s_host, host.s_lh, host.s_impno, GetLancastPort());
		else
			AddLogLine(0, IDS_JOIN_SUCCEEDED, host.s_net, host.s_host, host.s_lh, host.s_impno, GetLancastPort());
	}
}

void CLanCast::Stop()
{
	if (bStarted)
	{
		if(!LeaveGroup())
		{
			if (g_App.m_pMDlg)
				AddLogLine(LOG_RGB_WARNING, IDS_LEAVE_FAILED, GetLancastPort());
		}
		else if (g_App.m_pMDlg)
			AddLogLine(0, IDS_LEAVE_SUCCEEDED, GetLancastPort());
	}
}

bool CLanCast::JoinGroup(const char *pcGroupIP, uint32 dwGroupPort, uint32 dwTTL, bool bLoopback)
{
	bStarted = false;

//	Create Socket for receiving and join the host group
	if(!CreateReceivingSocket(pcGroupIP, dwGroupPort))
		return false;
//	Create Socket for sending
	if(!CreateSendingSocket(dwTTL, bLoopback))
		return false;

	bStarted = true;

	return true;
}

bool CLanCast::LeaveGroup()
{
	bStarted = false;

	if (setsockopt(m_hSocket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char FAR *)&m_mrMReq, sizeof(m_mrMReq)) < 0)
		return false;

	m_SendSocket.Close();		// Close sending socket
	Close();					// Close receving socket
	return true;
}

bool CLanCast::SetTTL(uint32 dwTTL)
{
//	Set Time to Live to parameter TTL
	if(m_SendSocket.SetSockOpt(IP_MULTICAST_TTL, &dwTTL, sizeof(dwTTL), IPPROTO_IP) == 0)
		return false;		// Error Setting TTL
	else
		return true;		// else TTL set successfully
}

void CLanCast::SetLoopBack(bool bLoop)
{
//	Set LOOPBACK option to TRUE OR FALSE according to IsLoop parameter
	int nLoopBack = (int)bLoop;
//	Try to manually set the loopback, ie tell dozer to ignore packets from itself (if setloopback is false)
	m_SendSocket.SetSockOpt(IP_MULTICAST_LOOP, &nLoopBack, sizeof(nLoopBack), IPPROTO_IP);
}

bool CLanCast::GetLancastEnabled()
{
	return g_App.m_pPrefs->GetLancastEnabled();
}

uint32 CLanCast::GetLancastIP()
{
	return g_App.m_pPrefs->GetLancastIP();
}

uint16 CLanCast::GetLancastPort()
{
	return g_App.m_pPrefs->GetLancastPort();
}
