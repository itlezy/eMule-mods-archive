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
#include "EMSocket.h"
#include "updownclient.h" //Xman

class CUpDownClient;
class CPacket;
class CTimerWnd;
enum EDebugLogPriority;

enum SocketState 
{
	SS_Other,		//These are sockets we created that may or may not be used.. Or incoming connections.
	SS_Half,		//These are sockets that we called ->connect(..) and waiting for some kind of response.
	SS_Complete	//These are sockets that have responded with either a connection or error.
};

class CClientReqSocket : public CEMSocket
{
	friend class CListenSocket;
	DECLARE_DYNCREATE(CClientReqSocket)

public:
	CClientReqSocket(CUpDownClient* in_client = NULL);

	void	SetClient(CUpDownClient* pClient);
	// Maella -Upload Stop Reason-
	/*
	void	Disconnect(LPCTSTR pszReason);
	*/
	void	Disconnect(LPCTSTR pszReason, CUpDownClient::UpStopReason reason = CUpDownClient::USR_NONE);
	//Xman end
	void	WaitForOnConnect();
	void	ResetTimeOutTimer();
	bool	CheckTimeOut();
	virtual UINT GetTimeOut();
	virtual void Safe_Delete();
	
	bool	Create();
	virtual void SendPacket(Packet* packet, bool delpacket = true, bool controlpacket = true, uint32 actualPayloadSize = 0, bool bForceImmediateSend = false);
    virtual SocketSentBytes SendControlData(uint32 maxNumberOfBytesToSend, uint32 overchargeMaxBytesToSend);
    virtual SocketSentBytes SendFileAndControlData(uint32 maxNumberOfBytesToSend, uint32 overchargeMaxBytesToSend);

	void	DbgAppendClientInfo(CString& str);
	CString DbgGetClientInfo();

	//Xman Xtreme Upload
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	//declare this in emsocket
	/*
	CUpDownClient*	client;
	*/
	//Xman improved socket closing
	void	CloseSocket();
	//Xman end
	uint32	GetTimeOutTimer()	const	{return timeout_timer;} //zz_fly :: Drop stalled downloads :: netfinity

	void		 OnReceive(int nErrorCode);

protected:
	virtual ~CClientReqSocket();
	virtual void Close() { CAsyncSocketEx::Close(); }
	void	Delete_Timed();

	virtual void OnConnect(int nErrorCode);
	void		 OnClose(int nErrorCode);
	void		 OnSend(int nErrorCode);

	void		 OnError(int nErrorCode);

	virtual bool PacketReceived(Packet* packet);
	int			 PacketReceivedSEH(Packet* packet);
	bool		 PacketReceivedCppEH(Packet* packet);

	bool	ProcessPacket(const BYTE* packet, uint32 size,UINT opcode);
	bool	ProcessExtPacket(const BYTE* packet, uint32 size, UINT opcode, UINT uRawSize);
	//Xman
	// Maella -Dump information of unknown packet in debug tab-
	/*
	void	PacketToDebugLogLine(LPCTSTR protocol, const uchar* packet, uint32 size, UINT opcode);
	*/
	void    PacketToDebugLogLine(bool isOpcodeKnown, const uchar* packet, uint32 size, UINT opcode);
	// Maella end

	void	SetConState(SocketState val);

	uint32	timeout_timer;
	bool	deletethis;
	uint32	deltimer;
	bool	m_bPortTestCon;
	uint32	m_nOnConnect;
};


class CListenSocket : public CAsyncSocketEx
{
	friend class CClientReqSocket;

public:
	CListenSocket();
	virtual ~CListenSocket();

	bool	StartListening();
	void	StopListening();
	virtual void OnAccept(int nErrorCode);
	void	Process();
	void	RemoveSocket(CClientReqSocket* todel);
	void	AddSocket(CClientReqSocket* toadd);
	UINT	GetOpenSockets()		{return socket_list.GetCount();}
	void	KillAllSockets();
	bool	TooManySockets(bool bIgnoreInterval = false);
	uint32	GetMaxConnectionReached()	{return maxconnectionreached;}
	bool    IsValidSocket(CClientReqSocket* totest);
	void	AddConnection();
	void	RecalculateStats();
	void	ReStartListening();
	void	Debug_ClientDeleted(CUpDownClient* deleted);
	bool	Rebind();

#ifdef DUAL_UPNP //zz_fly :: dual upnp
	bool	RebindUPnP(); //ACAT UPnP :: Rebind UPnP on IP-change
#endif //zz_fly :: dual upnp

	bool	SendPortTestReply(char result,bool disconnect=false);

	void	UpdateConnectionsStatus();
	float	GetMaxConperFiveModifier();
	uint32	GetPeakConnections()		{ return peakconnections; }
	uint32	GetTotalConnectionChecks()	{ return totalconnectionchecks; }
	float	GetAverageConnections()		{ return averageconnections; }
	uint32	GetActiveConnections()		{ return activeconnections; }
	uint16	GetConnectedPort()			{ return m_port; }
	uint32	GetTotalHalfCon()			{ return m_nHalfOpen; }
	uint32	GetTotalComp()				{ return m_nComp; }

	//Xman NAFC
	bool boundcheck;
	//Xman end

private:
	bool bListening;
	CTypedPtrList<CPtrList, CClientReqSocket*> socket_list;
	//Enig123 :: ACAT style socket handling //see details in CListenSocket::TooManySockets()
#ifndef _DEBUG
	uint16	m_OpenSocketsInterval;
#else
	CList<uint32>	SocketCreateTimeList;
	bool	m_bTimeListChanged;
	uint32	m_nextTriggerTick;
#endif
	//Enig123 :: end
	uint32	maxconnectionreached;
	uint16	m_ConnectionStates[3];
	int		m_nPendingConnections;
	uint32	peakconnections;
	uint32	totalconnectionchecks;
	float	averageconnections;
	uint32	activeconnections;
	uint16  m_port;
	uint32	m_nHalfOpen;
	uint32	m_nComp;
};
