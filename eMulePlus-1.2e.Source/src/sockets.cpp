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

#include "stdafx.h"
#include "emule.h"
#include "packets.h"
#include "ServerSocket.h"
#include "server.h"
#include "SharedFileList.h"
#include "ServerListCtrl.h"
#include "ServerList.h"
#include "sockets.h"
#include "emuleDlg.h"
#include "opcodes.h"
#include "SearchList.h"
#include <afxinet.h>
#include "UDPSocket.h"
#include "HTRichEditCtrl.h"
#include "SafeFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifdef OLD_SOCKETS_ENABLED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CServerConnect::CServerConnect()
{
	EMULE_TRY

	m_pConnectedSocket = NULL;
	m_bConnecting = false;
	m_bConnected = false;
	m_bTryObfuscated = false;
	m_dwClientID = 0;
	m_pUDPSocket = new CUDPSocket(this);
	m_pUDPSocket->Create();
	m_iRetryTimerID = 0;
	m_iLastServerListStartPos = 0;
	m_iCheckTimerID = 0;
	m_byteNumConnAttempts = 0;
	m_pConnectedServerSocket = NULL;
	m_hICCThread = 0;

	InitLocalIP();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CServerConnect::~CServerConnect()
{
	EMULE_TRY

//	Exit ICC Thread if exist
	if (m_hICCThread != 0)
		TerminateThread(m_hICCThread, 0);
//	Stop all connections
	StopConnectionTry();
//	Close connected socket, if any
	DestroySocket(m_pConnectedSocket);
	m_pConnectedSocket = NULL;
//	Close UDP socket
	m_pUDPSocket->Close();
	delete m_pUDPSocket;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerConnect::TryAnotherConnectionRequest()
{
	EMULE_TRY

//	Skip server connection if ICC thread is working, wait for thread selftermination
//	to avoid rough thread killing inside ConnectToServer(), as it will leave
//	DLL attached to the thread (wininet) in unpredictable state
	if ( (m_mapConnectionAttempts.GetCount() == 0) &&
		((m_hICCThread == 0) || (WaitForSingleObject(m_hICCThread, 0) != WAIT_TIMEOUT)) )
	{
		CServer	*pNextServer = g_App.m_pServerList->GetNextServer(m_bTryObfuscated);

		if (pNextServer == NULL)
		{
			if (m_mapConnectionAttempts.GetCount() == 0)
			{
				if (m_bTryObfuscated && !g_App.m_pPrefs->IsClientCryptLayerRequired())
				{
				//	Try all servers on the non-obfuscated port next
					m_bTryObfuscated = false;
					ConnectToAnyServer(~0u, true, true, true);
				}
				else
				{
					AddLogLine(LOG_FL_SBAR, IDS_OUTOFSERVERS);
					ConnectToAnyServer(m_iLastServerListStartPos, false);
				}
			}
		}
	//	Auto-connect to static servers only option
		else if (!g_App.m_pPrefs->AutoConnectStaticOnly() || pNextServer->IsStaticMember())
			ConnectToServer(pNextServer, !m_bTryObfuscated);
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerConnect::ConnectToAnyServer(uint32 iStartIndex/*=0xFFFFFFFF*/,
	bool bPrioSort/*=true*/, bool bIsAuto/*=true*/, bool bNoCrypt/*=false*/)
{
	EMULE_TRY

	CServer		*pNextServer;

//	If no start position is specified start with the next unused server in the list
	if (iStartIndex == 0xFFFFFFFF)
		iStartIndex = g_App.m_pServerList->GetServerPosition();
	m_iLastServerListStartPos = iStartIndex;
//	Cancel any ongoing connection attempts
	StopConnectionTry();
//	If we're currently connected to a server, disconnect from it.
	Disconnect();
//	If the server list is empty, output an error message and return
	if (g_App.m_pServerList->GetServerCount() == 0)
	{
		AddLogLine(LOG_FL_SBAR | LOG_RGB_WARNING, IDS_ERR_NOVALIDSERVERSFOUND);
		return;
	}
	if (g_App.m_pPrefs->GetUseServerPriorities() && bPrioSort)
		g_App.m_pServerList->Sort();

	g_App.m_pServerList->SetServerPosition(iStartIndex);
//	If we're auto-connecting and the "only auto-connect to static server" preference is set...
	if (g_App.m_pPrefs->AutoConnectStaticOnly() && bIsAuto)
	{
		bool	bAnyStatic = false;

	//	Scan all servers in the list to find a static one
		for (uint32 i = 0; i < g_App.m_pServerList->GetServerCount(); i++)
		{
			pNextServer = g_App.m_pServerList->GetNextServer(false);

		//	If we are at the end of the list, 'pNextServer' will be NULL.
		//	Since the list has at least one server, we just need to repeat operation.
			if (pNextServer == NULL)
				pNextServer = g_App.m_pServerList->GetNextServer(false);

			if (pNextServer->IsStaticMember())
			{
				bAnyStatic = true;
				break;
			}
		}
	//	If we couldn't find a static server, output an error message and return
		if (!bAnyStatic)
		{
			AddLogLine(LOG_FL_SBAR | LOG_RGB_WARNING, IDS_ERR_NOVALIDSERVERSFOUND);
			return;
		}
	}
//	If we're not auto-connecting or the "only auto-connect to static server" preference is not set...
	else
	{
		pNextServer = g_App.m_pServerList->GetNextServer(false);
	//	If we are at the end of the list, 'pNextServer' will be NULL.
	//	Since the list has at least one server, we just need to repeat operation.
		if (pNextServer == NULL)
			pNextServer = g_App.m_pServerList->GetNextServer(false);
	}
//	Get ready for new upload client connections.
	g_App.m_pListenSocket->Process();

//	At this point we have a server object -- connect to it
	if (pNextServer != NULL)
		ConnectToServer(pNextServer, !m_bTryObfuscated);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerConnect::ConnectToServer(CServer *pSrv, bool bNoCrypt)
{
	EMULE_TRY

	StopConnectionTry();
	Disconnect();

	CServerSocket		*pNewSocket = new CServerSocket(this);

	if (pNewSocket != NULL)
	{
		m_bConnecting = true;
		m_pConnectedServerSocket = pSrv;
		m_openSocketList.AddTail(pNewSocket);
		pNewSocket->Create(0, SOCK_STREAM, FD_READ|FD_WRITE|FD_CLOSE|FD_CONNECT, NULL);
		pNewSocket->ConnectTo(pSrv, bNoCrypt);
		m_mapConnectionAttempts.SetAt(::GetTickCount(), pNewSocket);
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	StopConnectionTry() cancels any ongoing server connection attempts and closes all but the currently connected socket.
void CServerConnect::StopConnectionTry()
{
	EMULE_TRY

	m_mapConnectionAttempts.RemoveAll();
	m_bConnecting = false;

	if (m_iRetryTimerID)
	{
		KillTimer(NULL, m_iRetryTimerID);
		m_iRetryTimerID = 0;
	}

//	Close all currently opened sockets except the one which is connected to our current server
	POSITION		pos1, pos2;

	for (pos1 = m_openSocketList.GetHeadPosition(); ( pos2 = pos1 ) != NULL; )
	{
		CServerSocket		*pSck = m_openSocketList.GetNext(pos1);
	//	Don't destroy socket which is connected to server
	//	Don't destroy socket if it is going to destroy itself later on
		if (pSck != m_pConnectedSocket)
			DestroySocket(pSck);
	}
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerConnect::ConnectionEstablished(CServerSocket* pServerSocket)
{
	EMULE_TRY

//	Reset the retry code - Don't make any further attempts to connect.
	m_byteNumConnAttempts = 0;
	if (m_iCheckTimerID)
		KillTimer(NULL, m_iCheckTimerID);
	m_iCheckTimerID = 0;

//	We are already connected to another server, so just destroy this socket
	if (!m_bConnecting)
	{
		if (pServerSocket != m_pConnectedSocket)
			DestroySocket(pServerSocket);
		return;
	}
//	Get the IP of the local computer
	InitLocalIP();
//	If we're connected and waiting for the server to log us on...
	if (pServerSocket->GetConnectionState() == CS_WAITFORLOGIN)
	{
	//	If there's an entry for the connected socket in the set of connection
	//	attempts, update the attempt time.
		DWORD			dwAttemptTime;
		CServerSocket  *pTempSocket;
		POSITION		pos = m_mapConnectionAttempts.GetStartPosition();

	//	Until we've run out of connecting sockets...
		while (pos)
		{
		//	Retrieve the next key/value entry from the map
			m_mapConnectionAttempts.GetNextAssoc(pos,dwAttemptTime,pTempSocket);
		//	If the entry is for the connected socket...
			if (pTempSocket == pServerSocket)
			{
			//	Remove the entry
				m_mapConnectionAttempts.RemoveKey(dwAttemptTime);
			//	Re-add the entry with the current time
				dwAttemptTime = GetTickCount();
				m_mapConnectionAttempts.SetAt(dwAttemptTime,pServerSocket);
				break;
			}
		}

		AddLogLine( 0, IDS_CONNECTEDTOREQ, pServerSocket->m_pServer->GetListName(),
											   pServerSocket->m_pServer->GetFullIP(),
											   pServerSocket->m_pServer->GetPort() );

	//	Send the login packet
		CServer		*pServer = g_App.m_pServerList->GetServerByAddress( pServerSocket->m_pServer->GetAddress(),
																			 pServerSocket->m_pServer->GetPort() );
	//	If the server we're connecting to is in the server list...
		if (pServer)
		{
			pServer->ResetFailedCount();
		//	Update its information in the Server Window.
			g_App.m_pMDlg->m_wndServer.m_ctlServerList.RefreshServer(*pServer);
		}

	//	Construct the LOGINREQUEST packet byte stream
		CMemFile	packetStream(128);
		CWrTag		tagWr;

		packetStream.Write(g_App.m_pPrefs->GetUserHash(), 16);	// <HASH> Our userhash

		uint32		dwClientID = GetClientID();				// <ID:DWORD> Our client ID
		uint16		nPort = g_App.m_pPrefs->GetPort();			// <PORT:WORD> Our TCP port

		packetStream.Write(&dwClientID, sizeof(uint32));
		packetStream.Write(&nPort, sizeof(uint16));

		uint32		dwTagCount = 4;

		packetStream.Write(&dwTagCount, sizeof(uint32));				// <Tag_set> <TAGCOUNT:DWORD>

		g_App.m_pPrefs->WritePreparedNameTag(packetStream);	// (CT_NAME:string) Our nick
		tagWr.WriteToFile(CT_VERSION, EDONKEYVERSION, packetStream);	// (CT_VERSION:int) Our software version #
		tagWr.WriteToFile( CT_SERVER_FLAGS, SRVCAP_ZLIB | SRVCAP_AUXPORT |
#ifdef _UNICODE
			SRVCAP_UNICODE |
#endif
			SRVCAP_NEWTAGS | SRVCAP_LARGEFILES, packetStream );

	//	eMule Version (14-Mar-2004: requested by lugdunummaster (need for LowID clients which have no chance
	//	to send Hello packet to the server during the callback test))
		tagWr.WriteToFile( CT_EMULE_VERSION, (PLUS_COMPATIBLECLIENTID << 24) | ((CURRENT_PLUS_VERSION & 0x7F00) << 9) |
			((CURRENT_PLUS_VERSION & 0xF0) << 6) | ((CURRENT_PLUS_VERSION & 0x7) << 7), packetStream );

		Packet			*pPacket = new Packet(&packetStream);

		pPacket->m_eOpcode = OP_LOGINREQUEST;
		g_App.m_pUploadQueue->AddUpDataOverheadServer(pPacket->m_dwSize);
		this->SendPacket(pPacket, true, pServerSocket);
	}
//	If we thought we already were connected...
	else if (pServerSocket->GetConnectionState() == CS_CONNECTED)
	{
		g_App.stat_reconnects++;
		g_App.stat_serverConnectTime = GetTickCount();
		m_bConnected = true;
		AddLogLine(LOG_FL_SBAR, IDS_CONNECTEDTO, pServerSocket->m_pServer->GetListName());
		g_App.m_pMDlg->ShowConnectionState(true, pServerSocket->m_pServer->GetListName());
		m_pConnectedSocket = pServerSocket;
		StopConnectionTry();
		g_App.m_pSharedFilesList->ClearED2KPublishInfo();
		g_App.m_pSharedFilesList->SendListToServer();
		g_App.m_pMDlg->m_wndServer.m_ctlServerList.RemoveDeadServer();

		if (g_App.m_pPrefs->GetAddServersFromServer())
		{
			Packet		*pPacket = new Packet(OP_GETSERVERLIST, 0);

			g_App.m_pUploadQueue->AddUpDataOverheadServer(pPacket->m_dwSize);
			SendPacket(pPacket,true);
		}
		if(g_App.m_pPrefs->RestartWaiting())
			g_App.m_pMDlg->m_wndTransfer.m_ctlDownloadList.RestartWaitingDownloads();

		g_App.m_pMDlg->m_wndServer.m_pctlServerMsgBox->AppendText(_T("\n"), CSTRLEN(_T("\n")), CLR_DEFAULT, CLR_DEFAULT, HTC_HAVENOLINK);

	//	Reset a request timer after reconnection
		if (pServerSocket->m_pServer != g_App.m_pDownloadQueue->GetLastTCPSrcReqServer())
		{
			g_App.m_pDownloadQueue->ResetLastTCPRequestTime();
			g_App.m_pDownloadQueue->SetLastTCPSrcReqServer(pServerSocket->m_pServer);
		}

	// update the download states of the client
		g_App.m_pDownloadQueue->UpdateSourceStatesAfterServerChange();
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CServerConnect::SendPacket(Packet *pPacket,bool bDelPkt, CServerSocket *pSrvSocket/*=NULL*/)
{
	EMULE_TRY

	if (pSrvSocket == NULL)	//	If there's no server socket spec'd...
	{
		if (m_bConnected)	//	... and we're connected...
		{
			m_pConnectedSocket->SendPacket(pPacket, bDelPkt, true);
			m_pConnectedSocket->m_dwLastTransmission = GetTickCount();
		}
		else
		{
			if (bDelPkt)
				delete pPacket;

			return false;
		}
	}
	else
	{
		pSrvSocket->SendPacket(pPacket, bDelPkt, true);
		pSrvSocket->m_dwLastTransmission = GetTickCount();
	}

	return true;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CServerConnect::SendUDPPacket(Packet *pPacket, CServer *pHostSrv, bool bDelPkt,
	uint16 uSpecPort/*=0*/, byte *pbyteRawPkt/*=NULL*/, uint32 dwLen/*=0*/)
{
	EMULE_TRY

	if (m_bConnected && (pHostSrv != NULL))
#ifdef _CRYPT_READY
		m_pUDPSocket->SendPacket(pPacket, pHostSrv, uSpecPort, pbyteRawPkt, dwLen);
#else
		m_pUDPSocket->SendPacket(pPacket, pHostSrv);
#endif
	if (bDelPkt)
	{
		delete pPacket;
		delete[] pbyteRawPkt;
	}

	return true;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerConnect::ConnectionFailed(CServerSocket *pSrvSocket)
{
	EMULE_TRY

	if (!m_bConnecting && (pSrvSocket != m_pConnectedSocket))
		return;	//	Just return, cleanup is done by the socket itself

	CServer		*pUpdateServer;
	CServer		*pServer = g_App.m_pServerList->GetServerByAddress( pSrvSocket->m_pServer->GetAddress(),
																		 pSrvSocket->m_pServer->GetPort() );

	switch (pSrvSocket->GetConnectionState())
	{
		case CS_FATALERROR:
		{
		//	Sending message when internet connection down
			CString		strMessageText = GetResString(IDS_ERR_FATAL);

			g_App.m_pMDlg->SendMail( strMessageText,
													g_App.m_pPrefs->GetNotifierPopOnServerError(),
													g_App.m_pPrefs->IsSMTPWarningEnabled() );
			g_App.m_pMDlg->ShowNotifier( strMessageText, TBN_SERVER, false,
												  g_App.m_pPrefs->GetNotifierPopOnServerError() );
			AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, strMessageText);
			m_byteNumConnAttempts++;
			break;
		}
		case CS_DISCONNECTED:
		{
		//	Sending message when connection lost to server
			CString		strMessageText;

			g_App.m_pSharedFilesList->ClearED2KPublishInfo();

			strMessageText.Format( GetResString(IDS_ERR_LOSTC),
								   pSrvSocket->m_pServer->GetListName(),
								   pSrvSocket->m_pServer->GetFullIP(),
								   pSrvSocket->m_pServer->GetPort() );
			g_App.m_pMDlg->SendMail( strMessageText,
													g_App.m_pPrefs->GetNotifierPopOnServerError(),
													g_App.m_pPrefs->IsSMTPInfoEnabled() );
			g_App.m_pMDlg->ShowNotifier( strMessageText, TBN_SERVER, false,
												  g_App.m_pPrefs->GetNotifierPopOnServerError() );
			AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, _T("%s"), strMessageText);
			break;
		}
		case CS_SERVERDEAD:
		{
			AddLogLine(0, IDS_ERR_DEAD, pSrvSocket->m_pServer->GetListName(),
						pSrvSocket->m_pServer->GetFullIP(), pSrvSocket->m_pServer->GetPort() );
			pUpdateServer = g_App.m_pServerList->GetServerByAddress( pSrvSocket->m_pServer->GetAddress(),
																		  pSrvSocket->m_pServer->GetPort() );
			if (pUpdateServer)
			{
				pUpdateServer->AddFailedCount();
				g_App.m_pMDlg->m_wndServer.m_ctlServerList.RefreshServer(*pUpdateServer);
			}
			m_byteNumConnAttempts++;
			break;
		}
		case CS_ERROR:
			m_byteNumConnAttempts++;
			break;

		case CS_SERVERFULL:
			AddLogLine( 0, IDS_ERR_FULL, pSrvSocket->m_pServer->GetListName(),
				pSrvSocket->m_pServer->GetFullIP(), pSrvSocket->m_pServer->GetPort() );
			break;

		case CS_NOTCONNECTED:
			m_byteNumConnAttempts++;
			break;
	}

	if (m_byteNumConnAttempts >= 3)
	{
		if (!m_iCheckTimerID)
		{
			CString		strUrl;

			strUrl.Format(_T("http://%s"), g_App.m_ICCURLs[rand() % g_App.m_ICCURLs.GetSize()]);
			AddLogLine(0, IDS_NET_CHECKING, strUrl);

			if (IsAliveURL(strUrl))
				m_byteNumConnAttempts = 0;
			else if (m_bConnecting)
			{
				StopConnectionTry();
				m_iCheckTimerID = SetTimer(NULL, 0, 1000*CS_RETRYCONNECTTIME, static_cast<TIMERPROC>(CheckInternetCallback));

			//	Refreshing server in list when disconnection occurs
				if (pServer)
					g_App.m_pMDlg->m_wndServer.m_ctlServerList.RefreshServer(*pServer);
				return;
			}
			else
				return;
		}
	}

	switch (pSrvSocket->GetConnectionState())
	{
		case CS_FATALERROR:
		{
			StopConnectionTry();
			if ((g_App.m_pPrefs->Reconnect()) && (!m_iRetryTimerID))
			{
			//	Sending message when reconnecting after 30 seconds
				CString		strMessage;

				strMessage.Format(GetResString(IDS_RECONNECT), CS_RETRYCONNECTTIME);
				g_App.m_pMDlg->SendMail( strMessage,
														g_App.m_pPrefs->GetNotifierPopOnServerError(),
														g_App.m_pPrefs->IsSMTPWarningEnabled() );
				g_App.m_pMDlg->ShowNotifier( strMessage,
													  TBN_SERVER,
													  false,
													  g_App.m_pPrefs->GetNotifierPopOnServerError() );
				AddLogLine(0, IDS_RECONNECT, CS_RETRYCONNECTTIME);
				m_iRetryTimerID = SetTimer(NULL, 0, 1000*CS_RETRYCONNECTTIME, static_cast<TIMERPROC>(RetryConnectCallback));
			}
			break;
		}
		case CS_DISCONNECTED:
		{
			g_App.m_pSharedFilesList->ClearED2KPublishInfo();
			m_bConnected = false;
			if (m_pConnectedSocket)
				DestroySocket(m_pConnectedSocket);
			m_pConnectedSocket = NULL;
			g_App.m_pMDlg->m_dlgSearch.OnBnClickedCancels();

			g_App.stat_serverConnectTime = 0;
			g_App.m_pMDlg->m_dlgStatistics.Add2TotalServerDuration();

			if (g_App.m_pPrefs->Reconnect() && !m_bConnecting)
			{
				ConnectToAnyServer();
			}
		//	Sending message when connection lost
			CString		strMessage = GetResString(IDS_CONNECTIONLOST);

			g_App.m_pMDlg->SendMail( strMessage,
													g_App.m_pPrefs->GetNotifierPopOnServerError(),
													g_App.m_pPrefs->IsSMTPWarningEnabled() );
			g_App.m_pMDlg->ShowNotifier( strMessage,
												  TBN_SERVER,
												  false,
												  g_App.m_pPrefs->GetNotifierPopOnServerError() );
			break;
		}
		case CS_ERROR:
		case CS_NOTCONNECTED:
			if (!m_bConnecting)
				break;
			AddLogLine( 0, IDS_ERR_CONFAILED, pSrvSocket->m_pServer->GetListName(),
				pSrvSocket->m_pServer->GetFullIP(), pSrvSocket->m_pServer->GetPort() );
		case CS_SERVERDEAD:
		case CS_SERVERFULL:
		{
			if (!m_bConnecting)
				break;

#if _CRYPT_READY
			if ( (pServer != NULL) && pSrvSocket->IsServerCryptEnabledConnection() &&
				!g_App.m_pPrefs->IsClientCryptLayerRequired() )
			{
			//	Try reconnecting without obfuscation
				ConnectToServer(pServer, false, true);
				break;
			}
#endif

			DWORD				dwTempKey;
			CServerSocket	   *pTempSocket;
			POSITION			pos = m_mapConnectionAttempts.GetStartPosition();

			while (pos)
			{
				m_mapConnectionAttempts.GetNextAssoc(pos,dwTempKey,pTempSocket);
				if (pTempSocket == pSrvSocket)
				{
					m_mapConnectionAttempts.RemoveKey(dwTempKey);
					break;
				}
			}
			TryAnotherConnectionRequest();
		}
	}
//	Refreshing server in list when disconnection occurs
	if (pServer)
		g_App.m_pMDlg->m_wndServer.m_ctlServerList.RefreshServer(*pServer);

	g_App.m_pMDlg->ShowConnectionState(false);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID CALLBACK CServerConnect::RetryConnectCallback(HWND hWnd, UINT nMsg, UINT nId, DWORD dwTime)
{
	NOPRM(hWnd); NOPRM(nMsg); NOPRM(nId); NOPRM(dwTime);
	EMULE_TRY

//	Always handle all type of MFC exceptions in TimerProcs - otherwise we'll get mem leaks
	CServerConnect		*_this = g_App.m_pServerConnect;

	ASSERT(_this);

//	Check if the thread is already running
	if (_this->m_hICCThread && WaitForSingleObject(_this->m_hICCThread, 0) == WAIT_TIMEOUT)
		return;

	_this->StopConnectionTry();

	if (_this->IsConnected())
		return;

	_this->ConnectToAnyServer();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID CALLBACK CServerConnect::CheckInternetCallback(HWND hWnd, UINT nMsg, UINT nId, DWORD dwTime)
{
	NOPRM(hWnd); NOPRM(nMsg); NOPRM(nId); NOPRM(dwTime);
	EMULE_TRY

	CServerConnect		*_this = g_App.m_pServerConnect;

	ASSERT(_this);

//	Check if the thread is already running
	if (_this->m_hICCThread && WaitForSingleObject(_this->m_hICCThread, 0) == WAIT_TIMEOUT)
		return;

	KillTimer(NULL, _this->m_iCheckTimerID);
	_this->m_iCheckTimerID = 0;

	if (_this->IsConnected())
		return;

	CString		strURL;

	strURL.Format(_T("http://%s"), g_App.m_ICCURLs[rand() % g_App.m_ICCURLs.GetSize()]);
	AddLogLine(0, IDS_NET_CHECKING, strURL);

	if(_this->IsAliveURL(strURL))
	{
		_this->m_byteNumConnAttempts = 0;
		if(!_this->IsConnecting())
			_this->ConnectToAnyServer();
	}
	else
	{
		_this->m_iCheckTimerID = SetTimer(NULL, 0, 1000*CS_RETRYCONNECTTIME, static_cast<TIMERPROC>(CheckInternetCallback));
		if(_this->IsConnecting())
			_this->StopConnectionTry();
	}
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct StructIsAliveURL
{
	StructIsAliveURL(const CString &strUrl, int iRet) : strURL(strUrl), iRC(iRet)	{}
	CString	strURL;
	int		iRC;
};

bool CServerConnect::IsAliveURL(const CString &strURL)
{
	StructIsAliveURL stData(strURL, -1);
	bool	bAppClose = false;

	EMULE_TRY

	m_hICCThread = (HANDLE)_beginthread(IsAliveURLThread, 0, &stData);

	MSG	msg;
	for(bool bDone = false; !bDone; WaitMessage())
	{
		while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
			{
				bDone = true;
				bAppClose = true;
				m_hICCThread = 0;
				PostMessage(NULL, WM_QUIT, 0, 0);
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if ( (WaitForSingleObject(m_hICCThread, 0) != WAIT_TIMEOUT) ||
				((bAppClose = !g_App.m_pMDlg->IsRunning()) == true) )
			{
				bDone = true;
				m_hICCThread = 0;
				break;
			}
		}
	}

	EMULE_CATCH

	if (!bAppClose)
	{
		if (stData.iRC == HTTP_STATUS_OK)
			AddLogLine(0, IDS_NET_WORKING);
		else
			AddLogLine(LOG_RGB_ERROR, IDS_ICC_FAILED, stData.iRC);
	}

	return (stData.iRC == HTTP_STATUS_OK);
}

void CServerConnect::IsAliveURLThread(void *pCtx)
{
	EMULE_TRY

	StructIsAliveURL *stData = (StructIsAliveURL *)pCtx;
	stData->iRC = -2;

//	Ping method is used for fast connectivity check
	if (InternetCheckConnection(stData->strURL, FLAG_ICC_FORCE_CONNECTION, 0))
		stData->iRC = HTTP_STATUS_OK;
	else
		stData->iRC = GetLastError();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerConnect::CheckForTimeout()
{
	EMULE_TRY

	DWORD			dwTmpKey, dwCurTick = GetTickCount(), dwSrvConnectTimeout = CONSERVTIMEOUT;
	CServerSocket  *pTmpSrvSocket;
	POSITION		pos = m_mapConnectionAttempts.GetStartPosition();

// In case of proxy, limit minimum connection timeout with default connection timeout
	if (g_App.m_pPrefs->GetProxySettings().m_bUseProxy)
		dwSrvConnectTimeout = max(dwSrvConnectTimeout, CONNECTION_TIMEOUT);

	while (pos)
	{
		m_mapConnectionAttempts.GetNextAssoc(pos, dwTmpKey, pTmpSrvSocket);

		if (pTmpSrvSocket == NULL)
		{
			AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T(__FUNCTION__) _T(": Invalid socket during timeout check"));
			m_mapConnectionAttempts.RemoveKey(dwTmpKey);
			return;
		}

		if (dwCurTick - dwTmpKey > dwSrvConnectTimeout)
		{
			AddLogLine( 0, IDS_ERR_CONTIMEOUT, pTmpSrvSocket->m_pServer->GetListName(),
												   pTmpSrvSocket->m_pServer->GetFullIP(),
												   pTmpSrvSocket->m_pServer->GetPort() );
			m_mapConnectionAttempts.RemoveKey(dwTmpKey);
			TryAnotherConnectionRequest();
		}
	}
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CServerConnect::Disconnect()
{
	EMULE_TRY

//	We clear any timer that was left set
	if (m_iCheckTimerID)
	{
		KillTimer(NULL, m_iCheckTimerID);
		m_iCheckTimerID = 0;
	}

	if (m_hICCThread != 0)	//	If the ICC thread is running, we terminate it
	{
	//	Such rough thread killing can leave DLL
	//	attached to the thread (wininet) in unpredictable state
		if (!TerminateThread(m_hICCThread, 0))
			AddLogLine(LOG_FL_DBG | LOG_RGB_WARNING, _T("Can't terminate IsAliveURL Thread!"));
	}

//	Socket won't be destroyed if server is changed during connecting
	if ((m_bConnected || m_bConnecting) && m_pConnectedSocket)
	{
		CServer		*pServer = g_App.m_pServerList->GetServerByAddress(m_pConnectedSocket->m_pServer->GetAddress(), m_pConnectedSocket->m_pServer->GetPort());

		g_App.m_pSharedFilesList->ClearED2KPublishInfo();
#ifdef _CRYPT_READY
		g_App.SetPublicIP(0);
#endif
		DestroySocket(m_pConnectedSocket);

		m_pConnectedSocket = NULL;
		m_bConnected = false;
	//	Socket won't be destroyed if server is changed during connecting
		m_bConnecting = false;
		g_App.m_pMDlg->ShowConnectionState(false, _T(""));
		g_App.stat_serverConnectTime = 0;
	//	Tell our total server duration to update...
		g_App.m_pMDlg->m_dlgStatistics.Add2TotalServerDuration();
		if (pServer != NULL)
			g_App.m_pMDlg->m_wndServer.m_ctlServerList.RefreshServer(*pServer);

		return true;
	}

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CServer *CServerConnect::GetCurrentServer()
{
	EMULE_TRY

	if (IsConnected() && m_pConnectedSocket != NULL)
	{
		return m_pConnectedSocket->m_pServer;
	}

	EMULE_CATCH

	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CServer* CServerConnect::GetConnectingServer()
{
	return (m_bConnecting) ? m_pConnectedServerSocket : NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerConnect::SetClientID(uint32 newid)
{
	EMULE_TRY

	m_dwClientID = newid;
	if (!::IsLowID(newid))
		g_App.SetPublicIP(newid);
	g_App.m_pMDlg->ShowConnectionState(IsConnected(), GetCurrentServer()->GetListName());

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	DestroySocket() closes and destroys socket 'pSck' and removes it from the open socket list.
void CServerConnect::DestroySocket(CServerSocket *pSck)
{
	EMULE_TRY

	if (pSck == NULL)
		return;

//	Remove the socket from the open socket list
	POSITION	posDel = m_openSocketList.Find(pSck);

	if (posDel != NULL)
	{
		m_openSocketList.RemoveAt(posDel);
	}

	if (pSck->m_SocketData.hSocket != INVALID_SOCKET)
	{
	//	Indicate that we don't want notifications for this socket any more.
		pSck->AsyncSelect(0);
	//	Close the socket and release the socket descriptor.
		pSck->Close();
	}

	delete pSck;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CServerConnect::IsLocalServer(uint32 dwIP, uint16 nPort)
{
	EMULE_TRY

	if (IsConnected())
	{
		if (m_pConnectedSocket->m_pServer->GetIP() == dwIP && m_pConnectedSocket->m_pServer->GetPort() == nPort)
			return true;
	}

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	InitLocalIP() gets the IP address of the local computer and stores it in 'm_dwLocalIP'.
void CServerConnect::InitLocalIP()
{
	EMULE_TRY

	m_dwLocalIP = 0;

//	Don't use 'gethostbyname(NULL)'. The winsock DLL may be replaced by a DLL from a third party
//	which is not fully compatible to the original winsock DLL. ppl reported crash with SCORSOCK.DLL
//	when using 'gethostbyname(NULL)'.
	char		szHost[256];

//	Try to get the standard host name for the local computer. If successful...
	if (gethostname(szHost, sizeof szHost) == 0)
	{
		hostent		*pHostEnt = gethostbyname(szHost);

	//	If we received a 4-byte address...
		if (pHostEnt != NULL && pHostEnt->h_length == 4 && pHostEnt->h_addr_list[0] != NULL)
			m_dwLocalIP = *(reinterpret_cast<uint32*>(pHostEnt->h_addr_list[0]));
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerConnect::KeepConnectionAlive()
{
	EMULE_TRY

	DWORD dwServerKeepAliveTimeout = g_App.m_pPrefs->GetServerKeepAliveTimeout();

	if (dwServerKeepAliveTimeout && m_bConnected && m_pConnectedSocket && m_pConnectedSocket->GetConnectionState() == CS_CONNECTED &&
		GetTickCount() - m_pConnectedSocket->m_dwLastTransmission >= dwServerKeepAliveTimeout)
	{
	//	"Ping" the server if the TCP connection was not used for the specified interval with
	//	an empty publish files packet -> recommended by lugdunummaster himself!
		CSafeMemFile files(4);
		uint32 nFiles = 0;
		files.Write(&nFiles,4);
		Packet *pPacket = new Packet(&files);
		pPacket->m_eOpcode = OP_OFFERFILES;
		g_App.m_pUploadQueue->AddUpDataOverheadServer(pPacket->m_dwSize);
		AddLogLine(LOG_FL_DBG, _T("Refreshing server connection for %s"), m_pConnectedSocket->m_pServer->GetListName());
		m_pConnectedSocket->SendPacket(pPacket,true);
		m_pConnectedSocket->m_dwLastTransmission = GetTickCount();
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //OLD_SOCKETS_ENABLED
