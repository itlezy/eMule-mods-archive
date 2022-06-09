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
#include "server.h"
#include "packets.h"

#define WM_DNSLOOKUPDONE WM_USER+280

#ifdef OLD_SOCKETS_ENABLED

// Client to Server communication
class CServerConnect;

class CUDPSocketWnd : public CWnd
{
// Construction
public:
	CUDPSocketWnd();
	CUDPSocket* m_pOwner;
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnDNSLookupDone(WPARAM wParam,LPARAM lParam);
};


class CUDPSocket : public CAsyncSocket, CLoggable
{
	friend class CServerConnect;
public:
	CUDPSocket(CServerConnect* in_serverconnect);
	~CUDPSocket();
	bool	Create();
	void	SendPacket(Packet* packet,CServer* host);
	void	DnsLookupDone(WPARAM wp, LPARAM lp);
protected:
	void	AsyncResolveDNS(LPCTSTR lpszHostAddress, UINT nHostPort);
	HANDLE	m_hDNSTask;					// dns lookup handle
	
	virtual void OnReceive(int iErrorCode);
private:
	LPCTSTR m_lpszHostAddress;
	UINT m_nHostPort;
	HWND m_hWndResolveMessage;	// where to send WM_DNSRESOLVED
	SOCKADDR_IN m_SaveAddr;
	CUDPSocketWnd m_udpwnd;

	void	SendBuffer();
	bool	ProcessPacket(byte *pbytePacket, uint32 dwSize, byte uOpcode, uint32 dwIP, uint16 uPort);
	CServerConnect*	m_pServerConnect;
	char*	m_pcSendBuffer;
	uint32	m_dwSendBufLength;
	CServer* m_pServer;
	char	DnsHostBuffer[MAXGETHOSTSTRUCT];	// dns lookup structure
};
#endif //OLD_SOCKETS_ENABLED
