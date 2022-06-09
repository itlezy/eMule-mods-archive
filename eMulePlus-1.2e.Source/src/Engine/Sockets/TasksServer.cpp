// TasksServer.cpp: implementation of server opcodes.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OpCode.h"
#include "TasksOpcodes.h"
#include "../Data/Prefs.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SERVER MESSAGE
bool COpCode_SERVERMESSAGE::ProcessForClient(CEmClient_Peer* pClient)
{
	ASSERT(pClient); // this opcode is processed only in the main context

	CEmClient_Server* pSrv = reinterpret_cast<CEmClient_Server*>(pClient);

	// Increase message count
	m_nMsgCount = pSrv->m_nMsgCount++;

	AddLog(LOG_DEBUG, _T("Message from server: %s"), _Msg);

	//	m_Server._Addr = pSrv->m_dwAddr;
	//	m_Server._Port = pSrv->m_uPort;

	//	stEngine.PushToUI(this);

	return false; // Always return false when going to GUI, otherwise this object will be destroyed!
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SERVER MESSAGE in GUI
/*bool COpCode_SERVERMESSAGE::ProcessForUI()
{
// Print message
g_eMuleApp.m_pdlgEmule->AddServerMessageLine(_Msg);

// Check for server version
if ((m_nMsgCount < 2) && (_Msg.Left(14).CompareNoCase(_T("server version")) == 0))
{
CString strVer = _Msg.Mid(14);
strVer.Trim();
strVer = strVer.Left(64); // Truncate string to avoid misuse by servers in showing ads
int n = strVer.FindOneOf("\x09\x0A\x0D");
if (n >= 0)
strVer = strVer.Left(n);
CServer* pServer = g_eMuleApp.m_pServerList->GetServerByIP(m_Server._Addr, m_Server._Port);
if (pServer)
pServer->SetVersion(strVer);
}

// Check for dynamic IP
int nDynIP = _Msg.Find(_T("[emDynIP: "));
int nCloseBracket = _Msg.Find(_T("]"));
if (nDynIP != -1 && nCloseBracket != -1 && nDynIP < nCloseBracket)
{
CString sDynIP = _Msg.Mid(nDynIP + 10, nCloseBracket - (nDynIP + 10));
sDynIP.Trim(_T(" "));
if (sDynIP.GetLength() && sDynIP.GetLength() < 51)
{
CServer* pServer = g_eMuleApp.m_pServerList->GetServerByIP(m_Server._Addr, m_Server._Port);
if (pServer)
{
pServer->SetDynIP(sDynIP.GetBuffer());
g_eMuleApp.m_pdlgEmule->m_wndServer.m_ctlServerList.RefreshServer(*pServer);
}
}
}

return true;
}*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ID CHANGE
bool COpCode_IDCHANGE::ProcessForClient(CEmClient_Peer* pClient)
{
	ASSERT(pClient); // this opcode is processed only in the main context

	// TODO: Smart ID checks, notifiers

	// Disconnect from previous connected server, if any
	if(g_stEngine.GetServerSocket() != pClient->m_hSocket &&
		g_stEngine.GetServerSocket())
		g_stEngine.DisconnectFromServer(g_stEngine.GetServerSocket());

	// Set server connection info
	g_stEngine.ConnectedTo(reinterpret_cast<CEmClient_Server*>(pClient));
	g_stEngine.SetServerSupportedFeatures(_SupportedFeatures);
	g_stEngine.SetClientID(_NewClientID);
	g_stEngine.SetConnectionState(SERVER_CONNECTED);

	/*
	if (g_eMuleApp.m_pGlobPrefs.GetAddServersFromConn())
	{
	Packet* packet = new Packet(OP_GETSERVERLIST);
	g_eMuleApp.m_pUploadQueue->AddUpDataOverheadServer(packet->size);
	SendPacket(packet,true);
	}
	if(g_eMuleApp.m_pGlobPrefs.RestartWaiting())
	g_eMuleApp.m_pdlgEmule->m_wndTransfer.m_ctlDownloadList.RestartWaitingDownloads();
	*/

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SEARCH RESULT
bool COpCode_SEARCHRESULT::ProcessForClient(CEmClient_Peer* pClient)
{
	ASSERT(pClient); // this opcode is processed only in the main context

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FOUND SOURCES
bool COpCode_FOUNDSOURCES::ProcessForClient(CEmClient_Peer* pClient)
{
	ASSERT(pClient); // this opcode is processed only in the main context

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SERVER STATUS
bool COpCode_SERVERSTATUS::ProcessForClient(CEmClient_Peer* pClient)
{
	ASSERT(pClient); // this opcode is processed only in the main context

	// Set address and port
	//	m_Server._Addr = pClient->m_dwAddr;
	//	m_Server._Port = pClient->m_uPort;

	//	stEngine.PushToUI(this);

	AddLog(LOG_DEBUG, _T("Server status: %u users, %u files"), _NumberOfUsers, _NumberOfFiles);

	return false; // Always return false when going to GUI, otherwise this object will be destroyed!
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SERVER STATUS in GUI
/*bool COpCode_SERVERSTATUS::ProcessForUI()
{
CServer* pServer = g_eMuleApp.m_pServerList->GetServerByIP(m_Server._Addr, m_Server._Port);
if (pServer)
{
pServer->SetUserCount(_NumberOfUsers); 
pServer->SetFileCount(_NumberOfFiles);
g_eMuleApp.m_pdlgEmule->ShowUserCount(_NumberOfUsers, _NumberOfFiles);
g_eMuleApp.m_pdlgEmule->m_wndServer.m_ctlServerList.RefreshServer(*pServer);
}

return true;
}*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SERVER IDENTification
bool COpCode_SERVERIDENT::ProcessForClient(CEmClient_Peer* pClient)
{
	ASSERT(pClient); // this opcode is processed only in the main context

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SERVER LIST
bool COpCode_SERVERLIST::ProcessForClient(CEmClient_Peer* pClient)
{
	ASSERT(pClient); // this opcode is processed only in the main context

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	CALLBACKREQUEST is a Client => Server message and should never be
//		received by a client.
bool COpCode_CALLBACKREQUEST::ProcessForClient(CEmClient_Peer *pClient)
{
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	CALLBACKFAIL (if it is ever received by a client) is ignored by
//		eMulePlus.
bool COpCode_CALLBACKFAIL::ProcessForClient(CEmClient_Peer *pClient)
{
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CALLBACK REQUESTED
bool COpCode_CALLBACKREQUESTED::ProcessForClient(CEmClient_Peer* )
{
	CTask_SayHelloToPeer* pSayHelloTask = new CTask_SayHelloToPeer();
	CTask_Connect* pTask = new CTask_Connect(_ClientAddr.Addr, _ClientAddr.Port, T_CLIENT_PEER, pSayHelloTask);

	g_stEngine.Sockets.Push(pTask);

	AddLog(LOG_DEBUG, _T("Callback requested"));

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LOGIN REQUEST is a Client => Server message and should never be
//		received by a client.
bool COpCode_LOGINREQUEST::ProcessForClient(CEmClient_Peer* )
{
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CTask_LoginToServer::Process()
{
	//	Reset message count
	//	m_pServer->m_nMsgCount = 0;
	if (m_pServer == NULL)
	{
		//	Report error...
		return true;
	}
	if (m_iMessage == FD_CONNECT)
	{
		//
		//	We've just connected. Make sure we're prepared to handle disconnect.
		CTask_DisconnectServer* pOnDisconnectTask = new CTask_DisconnectServer();

		pOnDisconnectTask->m_pServer = m_pServer;
		m_pServer->m_pOnCompletionTask = pOnDisconnectTask;

		//	Disconnect from previous connected server, if any
		if ( g_stEngine.GetServerSocket() != m_pServer->m_hSocket
			&& g_stEngine.GetServerSocket() != 0 )
		{
			g_stEngine.DisconnectFromServer(g_stEngine.GetServerSocket());
		}

		//	Set connection info
		g_stEngine.ConnectedTo(m_pServer);
		g_stEngine.SetConnectionState(SERVER_WAITFORLOGIN);

		// Ask for login
		COpCode_LOGINREQUEST stMsg;

		// Don't work for new 2.0 version...
		const UINT CURRENT_PLUS_VERSION_FAKE = 0x0115;

		md4cpy(&stMsg._UserHash, g_stEngine.Prefs.GetUserHash());
		stMsg._ClientAddr.Addr = g_stEngine.GetClientID();
		stMsg._ClientAddr.Port = g_stEngine.Prefs.GetPort();
		stMsg._UserName = g_stEngine.Prefs.GetUserNick();
		stMsg._ClientSoftVersion = EDONKEYVERSION;
		stMsg._UserPort = g_stEngine.Prefs.GetPort();
		stMsg._Flags = SRVCAP_ZLIB | SRVCAP_AUXPORT | SRVCAP_NEWTAGS;
		stMsg._eMuleVersion = (PLUS_COMPATIBLECLIENTID << 24) | ((CURRENT_PLUS_VERSION_FAKE & 0x7F00) << 9) |
			((CURRENT_PLUS_VERSION_FAKE & 0xF0) << 6) | ((CURRENT_PLUS_VERSION_FAKE & 0x7) << 7);

		g_stEngine.SendOpCode(m_pServer->m_hSocket, stMsg, m_pServer, QUE_HIGH);
	}
	else
	{
		//	We're not expecting this.
	}

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CTask_DisconnectServer::Process()
{
	if (m_pServer == NULL)
		return true;

	// (!) at any reason, can be even if we tried to connect and failed

	if (m_iMessage == FD_CLOSE)
	{
		// Set connection info
		if (g_stEngine.GetServerSocket() == m_pServer->m_hSocket)
		{
			g_stEngine.SetConnectionState(SERVER_DISCONNECTED);
		}
		else
		{
			// removed if, because we can get 'disconnected' even if we didn't
			// succeeded to connect yet, thus GetConnectingServer() won't return
			// correct value
			// if(rEngine.GetConnectingServer() == m_hSocket)
			g_stEngine.SetConnectionState(SERVER_DISCONNECTED);
			// TODO: mark somewhere that server is dead
		}
	}
	else
	{
		//	We're not expecting this.
		ASSERT(FALSE);
	}

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
