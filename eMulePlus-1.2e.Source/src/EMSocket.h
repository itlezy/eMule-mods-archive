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

#include "afxsock.h"
#include "types.h"
#include "packets.h"
#include "AsyncSocketEx.h" // deadlake PROXYSUPPORT Socketfiles
#include "AsyncProxySocketLayer.h" // deadlake PROXYSUPPORT Socketfiles

#define ERR_WRONGHEADER		0x01
#define ERR_TOOBIG			0x02

enum EnumConnectionStates
{
	ES_NOTCONNECTED	= 0x00,
	ES_CONNECTED	= 0x01,
	ES_CONNECTING	= 0x02,
	ES_DISCONNECTED	= 0xFF
};

class CEMSocket : public CAsyncSocketEx // deadlake PROXYSUPPORT - changed to AsyncSocketEx
{
public:
			CEMSocket(void);
	virtual	~CEMSocket(void);
	virtual void	SendPacket(Packet *pPacket, bool bDeletePacket = true, bool bControlPacket = true);// controlpackets have a higher priority
	bool	IsBusy() const { return m_pcSendBuffer != NULL; }
	bool	IsConnected() const { return m_eConnectionState == ES_CONNECTED; }
	bool	IsConnecting() const { return m_eConnectionState == ES_CONNECTING; }
	void	SetDownloadLimit(uint32 dwLimit);
	void	DisableDownloadLimit(bool bAvoidRead = false);
	BOOL	AsyncSelect(long lEvent);

	virtual BOOL Connect(LPCSTR lpszHostAddress, UINT nHostPort);
	virtual BOOL Connect(SOCKADDR *pSockAddr, int iSockAddrLen);
	virtual void RemoveAllLayers();	//	Reset Layer Chain
	bool	TruncateQueues();

protected:
	virtual int		OnLayerCallback(const CAsyncSocketExLayer *pLayer, int nType, int nCode, WPARAM wParam, LPARAM lParam);
	virtual void	PacketReceived(Packet *pPacket) = 0;
	virtual void	OnError(int iErrorCode) = 0;
	virtual void	OnClose(int iErrorCode);
	virtual void	OnSend(int iErrorCode);
	virtual void	OnReceive(int iErrorCode);

	EnumConnectionStates	m_eConnectionState;
	bool	m_bProxyConnectFailed;
	CAsyncProxySocketLayer	*m_pProxyLayer;

private:
	void		ClearQueues();
	int			Send(char* lpBuf,int nBufLen,int nFlags = 0);
	virtual int	Receive(void* lpBuf, int nBufLen, int nFlags = 0);
	void		InitProxySupport();

//	Download (pseudo) rate control
	uint32		m_dwDownloadLimit;
	bool		m_bEnableDownloadLimit;
	bool		m_bPendingOnReceive;

//	Download partial header
	char			m_arrcPendingHeader[sizeof(PacketHeader_Struct)];
	uint32		m_dwPendingHeaderSize;

//	Download partial packet
	Packet	   *m_pPendingPacket;
	uint32		m_dwPendingPacketSize;

//	Upload control
	char	   *m_pcSendBuffer;
	uint32		m_dwSendBufLen;
	uint32		m_dwNumBytesSent;
	bool		m_bLinkedPackets;

	CTypedPtrList<CPtrList, Packet*> m_controlPacketQueue;
	CTypedPtrList<CPtrList, Packet*> m_standardPacketQueue;

	bool		m_bInPacketReceived;
};
