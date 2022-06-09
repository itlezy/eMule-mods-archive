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
#pragma once

#include "Loggable.h"

class Packet;
struct UDPPack;

class CClientUDPSocket : public CAsyncSocket, public CLoggable
{
public:
	CClientUDPSocket();
	virtual ~CClientUDPSocket();
	bool	SendPacket(Packet* packet, uint32 dwIP, uint16 nPort);
	bool	IsBusy()	{return m_bWouldBlock;}
	bool	Create();

protected:
	bool	ProcessPacket(byte *pbytePacket, uint32 dwSize, byte opcode, uint32 dwIP, uint16 port);
	
	virtual void	OnSend(int nErrorCode);	
	virtual void	OnReceive(int nErrorCode);

private:
	void	ClearQueues();	
	int		SendTo(const char *pcBuf, int nBufLen, uint32 dwIP, uint16 nPort);
	bool	m_bWouldBlock;

	CTypedPtrList<CPtrList, UDPPack*> m_controlPacketQueue;
};
