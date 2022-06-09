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

class CUpDownClient;
class CPacket;
class CTimerWnd;

class CClientReqSocket : public CEMSocket
{
	friend class CListenSocket;

public:
	CUpDownClient	*m_pClient;

private:
	uint32			m_dwTimeoutTimer;
	uint32			m_dwDeleteTimer;
	bool			m_bDeleteThis;

public:
						CClientReqSocket(CUpDownClient *in_pClient = NULL);
	void				Disconnect();

	void				ResetTimeOutTimer();
	bool				CheckTimeOut();
	void				Safe_Delete();
	bool				IsDeleted() { return m_bDeleteThis; }
	
	bool				Create();
	virtual void		SendPacket(Packet *pPacket, bool bDeletePacket = true, bool bControlPacket = true);// controlpackets have a higher priority

protected:
	virtual void		Close();
	virtual void		OnConnect(int iErrorCode);
	void				OnClose(int nErrorCode);
	void				OnSend(int nErrorCode);
	void				OnReceive(int nErrorCode);
	void				OnError(int nErrorCode);
	void				PacketReceived(Packet* packet);

private:
	void				TimedDelete();
					   ~CClientReqSocket();

	bool				ProcessPacket(byte *pbytePacket, uint32 dwSize, EnumOpcodes eOpcode);
	bool				ProcessExtPacket(byte *pbytePacket, uint32 dwSize, EnumOpcodes eOpcode, uint32 dwRawSize);
	bool				CheckUploadStateForRequestParts();
};

// CListenSocket command target
class CListenSocket : public CAsyncSocketEx // deadlake PROXYSUPPORT - changed to AsyncSocketEx
{
private:
	bool			m_bListening;

	CTypedPtrList<CPtrList, CClientReqSocket*>		m_openSocketList;
	deque<uint32>	m_dwSocketOpenTime;

	uint32			m_iMaxConnectionsReachedCount;
	LONG			m_lNumPendingConnections;

public:
					CListenSocket();
				   ~CListenSocket();
	bool			StartListening();
	void			StopListening();
	virtual void	OnAccept(int iErrorCode);
	void			Process();
	void			RemoveSocket(CClientReqSocket *pSocket);
	void			AddSocket(CClientReqSocket *pSocket);
	bool			DeleteSocket(CClientReqSocket *pSocket);
	uint16			GetNumOpenSockets()		{return static_cast<uint16>(m_openSocketList.GetCount());}
	void			KillAllSockets();
	bool			TooManySockets(bool bIgnoreInterval = false);
	uint32			GetMaxConnectionsReachedCount()	{return m_iMaxConnectionsReachedCount;}
	bool			IsValidSocket(CClientReqSocket *pSocket);
	void			AddConnection();
	void			RestartListening();
	void			Debug_ClientDeleted(CUpDownClient *pClient);
};
