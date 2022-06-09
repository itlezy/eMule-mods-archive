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

#include "Loggable.h"

#define CS_RETRYCONNECTTIME  30 // seconds

#ifdef OLD_SOCKETS_ENABLED

class CServerList;
class CUDPSocket;
class CServerSocket;
class CServer;
class Packet;

class CServerConnect : public CLoggable
{
public:
					CServerConnect();
					~CServerConnect();
	void			ConnectionFailed(CServerSocket* sender);
	void			ConnectionEstablished(CServerSocket* sender);

	void			ConnectToAnyServer(uint32 iStartIndex = 0xFFFFFFFF, bool bPrioSort = true, bool bIsAuto = true, bool bNoCrypt = false);
	void			ConnectToServer(CServer *pSrv, bool bNoCrypt);
	void			StopConnectionTry();
	static			VOID CALLBACK RetryConnectCallback(HWND hWnd, UINT nMsg, UINT nId, DWORD dwTime);
	static			VOID CALLBACK CheckInternetCallback(HWND hWnd, UINT nMsg, UINT nId, DWORD dwTime);

	void			CheckForTimeout();
	void			DestroySocket(CServerSocket* pSck);	// safe socket closure and destruction
	bool			SendPacket(Packet *pPacket, bool bDelPkt = true, CServerSocket *pSrvSocket = NULL);
	bool			SendUDPPacket(Packet *pPacket, CServer *pHostSrv, bool bDelPkt, uint16 uSpecPort = 0, byte *pbyteRawPkt = NULL, uint32 dwLen = 0);
	bool			Disconnect();
	bool			IsConnecting() const		{ return m_bConnecting; }
	bool			IsConnected() const			{ return m_bConnected; }
	bool			IsICCActive() const			{ return ((m_iCheckTimerID != 0) || (m_hICCThread != 0)); }
	uint32			GetClientID() const			{ return m_dwClientID; }
	CServer*		GetCurrentServer();
	CServer*		GetConnectingServer();	// Server status in WebServer
	bool			IsLowID()					{ return (m_dwClientID < 0x1000000); }
	void			SetClientID(uint32 newid);
	bool			IsLocalServer(uint32 dwIP, uint16 uPort);
	void			TryAnotherConnectionRequest();
	void			InitLocalIP();
	uint32			GetLocalIP() const			{ return m_dwLocalIP; }
	void			KeepConnectionAlive();
#ifdef _CRYPT_READY
	bool			IsConnectedObfuscated() const	{ return (m_pConnectedSocket != NULL) && (m_pConnectedSocket->IsObfusicating()); }
#else
	bool			IsConnectedObfuscated() const	{ return false; }
#endif

private:
	bool			IsAliveURL(const CString &strURL);
	static void		IsAliveURLThread(void *pCtx);

private:
	bool			m_bConnecting;
	bool			m_bConnected;
	bool			m_bTryObfuscated;
	byte			m_byteNumConnAttempts;
	CServer			*m_pConnectedServerSocket;
	uint32			m_iLastServerListStartPos;
	CServerSocket	*m_pConnectedSocket;
	CUDPSocket		*m_pUDPSocket;
	UINT			m_iRetryTimerID;
	UINT			m_iCheckTimerID;
	uint32			m_dwLocalIP;
	uint32			m_dwClientID;
	HANDLE			m_hICCThread;

	CTypedPtrList<CPtrList, CServerSocket *> m_openSocketList;	// list of currently opened sockets
	CMap<ULONG, ULONG, CServerSocket*, CServerSocket*> m_mapConnectionAttempts;	// Map of connect times to server sockets
};
#endif //OLD_SOCKETS_ENABLED
