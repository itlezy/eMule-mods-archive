//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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

// Client to Server communication
#pragma once

#include "types.h"
#include "EMSocket.h"
#include "AsyncProxySocketLayer.h"

enum EnumServerConnectionStates
{
	CS_FATALERROR		= -5,
	CS_DISCONNECTED		= -4,
	CS_SERVERDEAD		= -3,
	CS_ERROR			= -2,
	CS_SERVERFULL		= -1,
	CS_NOTCONNECTED		= 0,
	CS_CONNECTING		= 1,
	CS_CONNECTED		= 2,
	CS_WAITFORLOGIN		= 3,
	CS_WAITFORPROXYLISTENING = 4 // PROXYSUPPORT
};

class CServer;

class CServerSocket : public CEMSocket
{
	friend class CServerConnect;

public:
					CServerSocket(CServerConnect *in_serverconnect);
				   ~CServerSocket();

	void			ConnectTo(CServer *pSrv, bool bNoCrypt);
	int				GetConnectionState() const	{return m_iConnectionState;}

	uint32			m_dwOldID;
	DWORD			m_dwLastTransmission;

protected:
	void			OnClose(int nErrorCode);
	void			OnConnect(int nErrorCode);
	void			OnReceive(int nErrorCode);
	void			OnError(int nErrorCode);
	void			PacketReceived(Packet* packet);

private:
	bool			ProcessPacket(char* packet, uint32 size,EnumOpcodes eOpcode);
	void			SetConnectionState(int iNewState);

	CServerConnect	*m_pServerConnect;
	CServer			*m_pServer;
	int				m_iConnectionState;
};
