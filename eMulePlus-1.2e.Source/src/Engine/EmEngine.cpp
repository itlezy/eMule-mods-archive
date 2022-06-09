// EmEngine.cpp: implementation of the CEmEngine class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Files/TaskProcessorFiles.h"
#include "Database/TaskProcessorDB.h"
#include "Other/TaskProcessorLogger.h"

#include "Other/EmWinNT.h"
#include "Other/EmMt.h"
#include "Other/ThreadPool.h"
#include "TaskProcessor.h"
#include "EmEngine.h"
#include "Sockets/TasksSockets.h"
#include "Sockets/TasksOpcodes.h"
#include "Sockets/TcpEngineMule.h"
#include "Data/ClientList.h"
#include "Data/Prefs.h"
#include "XML/XMLEvents.h"

#include "Data/Server.h"
//#include "Files/KnownFile.h"
//#include "Other/ed2k_filetype.h" included indirect over KnownFile.h

#include "XML/XML.h"

//////////////////////////////////////////////////////////////////////
// Constructor
CEmEngine::CEmEngine()
	:m_pTcpEngineMule(NULL)
	,m_pMainProcessor(NULL)
	,m_pFilesProcessor(NULL)
	,m_pLoggerProcessor(NULL)
	,m_pDbProcessor(NULL)
	,m_pClientList(NULL)
	,m_pPreferences(NULL)
	,m_pXmlEvents(NULL)
{
	ServerState.nConnState = SERVER_DISCONNECTED;
	ServerState.hSocket = NULL;
	ServerState.dwAddr = 0;
	ServerState.uPort = 0;
	ServerState.nClientID = 0;

	m_dwThreadId = GetCurrentThreadId();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Start
bool CEmEngine::Init()
{
/*	CServer stSrv;
	stSrv.ImportFromServerMet(*this, _T("./Db/server.met"));

	if(stSrv.ListStartByStatic(*this))
	{
		while(stSrv.ListGetNext())
		{
			int a = 1;
		}
		stSrv.ListFinish();
	}*/

	// Clients list
	m_pClientList = new CClientList;

	// Preferences
	m_pPreferences = new CPrefs;

	// Xml events
	m_pXmlEvents = new CXMLEvents;

	m_pTcpEngineMule = new CTcpEngineMule;

	// Base class init
	if(!CEmEngineBase::Init())
		return false;

	//	Try to initialize the Logger
	m_pLoggerProcessor = new CTaskProcessor_Logger;
	if (!m_pLoggerProcessor->Init())
	{
		AddLog(LOG_ERROR, _T("Failed to initialize logger processor"));
		return false;
	}

	//	Try to initialize the DB TaskProcessor
	m_pDbProcessor = new CTaskProcessor_DB;
	if (!m_pDbProcessor->Init())
	{
		AddLog(LOG_ERROR, _T("Failed to initialize database processor"));
		return false;
	}
	//	Try to initialize the Files TaskProcessor
	m_pFilesProcessor = new CTaskProcessor_Files;
	if (!m_pFilesProcessor->Init())
	{
		AddLog(LOG_ERROR, _T("Failed to initialize files processor"));
		return false;
	}
	m_pMainProcessor = new CTaskProcessor_Main;
	if (!m_pMainProcessor->Init())
	{
		AddLog(LOG_ERROR, _T("Failed to initialize main processor"));
		return false;
	}
	//initial sort file types array
/*	if (!InitFileTypeArray())
	{
		ASSERT(FALSE);
		AddDebugLogLine("Failed to initialize filetype array");
		return false;
	}*/

	// add relevant interfaces
	GetTcpEngine()->AddInterface(Prefs.PortXml,		T_CLIENT_XML,	Prefs.XmlLocalBind ? LISTEN_TO_LOOPBACK : LISTEN_TO_INTERNET);
	GetTcpEngine()->AddInterface(Prefs.GetPort(),	T_CLIENT_PEER);

	//TcpEngine.AddInterface(80, T_CLIENT_WEB);

	return true;
}

void CEmEngine::UninitMiddle()
{
	m_pMainProcessor->Uninit();
	delete m_pMainProcessor;
}

void CEmEngine::UninitFinal()
{
	EMULE_TRY

	m_pLoggerProcessor->Uninit();
	delete m_pLoggerProcessor;

	m_pFilesProcessor->Uninit();
	delete m_pFilesProcessor;

	m_pDbProcessor->Uninit();
	delete m_pDbProcessor;

	delete m_pTcpEngineMule;

	delete m_pXmlEvents;

	delete m_pPreferences;

	delete m_pClientList;

	EMULE_CATCH2
}

bool CEmEngine::SendOpCode(SOCKET hSocket, const COpCode &stOpCode, CEmClient* pClient, EnumQueuePriority ePriority)
{
	if(pClient && Prefs.SaveLogsIO)
		m_pLoggerProcessor->Post(stOpCode, pClient->GetType(), pClient->m_nClientID, TRUE);
	return TcpEngineMule.AllocSend(hSocket, stOpCode, ePriority);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Connecting to given server addr/port
void CEmEngine::ConnectToServer(CString sAddr/* = "" */, ULONG ulPort/* = 0 */)
{
	EMULE_TRY

	if (GetServerSocket())
	{
		DisconnectFromServer(GetServerSocket());
	}

	AddLog(LOG_DEBUG, _T("Connecting to server (%s:%d)"), sAddr, ulPort);

	CTask_LoginToServer	*pCompletionTask = new CTask_LoginToServer();
	CTask_Connect		*pTask = new CTask_Connect(sAddr, (USHORT) ulPort, T_CLIENT_SERVER, pCompletionTask);

	SetConnectionState(SERVER_CONNECTING);
	m_pSocketsProcessor->Push(pTask);

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Connect to any server
void CEmEngine::ConnectToAnyServer()
{
	ConnectToServer();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Disconnect from connected server
void CEmEngine::DisconnectFromServer(SOCKET hPrevServer /* = NULL */)
{
	if(!hPrevServer)
	{
		if(GetServerSocket())
			DisconnectFromServer(GetServerSocket());
		else
			SetConnectionState(SERVER_DISCONNECTED);
		return;
	}

	CEmClient* pServer = m_pSocketsProcessor->Lookup(hPrevServer);
	if(!pServer)
	{
		AddLog(LOG_ERROR, _T("lookup for server failed"));
		return;
	}
	AddLog(LOG_DEBUG, _T("Disconnecting from server (%s:%d)"), inet_ntoa(*reinterpret_cast<in_addr*>(&pServer->m_dwAddr)), pServer->m_uPort);
	CTask_KillClient* pTask = new CTask_KillClient(pServer);
	m_pSocketsProcessor->Push(pTask);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmEngine::SetConnectionState(short nConnectionState)
{
	ServerState.nConnState = nConnectionState;

//	Export connection state
	switch (nConnectionState)
	{
		case SERVER_DISCONNECTED:
		{
			ServerState.hSocket = NULL;
			ServerState.dwAddr = 0;
			ServerState.uPort = 0;
			ServerState.nClientID = 0;
			AddLog(LOG_NOTICE, "Disconnected from server");

			XmlEvents.Fire_OnDisconnectedFromServer();
			break;
		}
		case SERVER_CONNECTING:
		{
			AddLog(LOG_DEBUG, "Connection state: connecting");
			break;
		}
		case SERVER_WAITFORLOGIN:
		{
			AddLog(LOG_DEBUG, "Connection state: waiting for login");
			break;
		}
		case SERVER_CONNECTED:
		{
			AddLog(LOG_DEBUG, "Connection state: connected");
			// send list of shared files
			CTask_SendSharedList* pTask = new CTask_SendSharedList();
			if(pTask)
				Sockets.Push(pTask);
			break;
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmEngine::SetClientID(long cID)
{
	ServerState.nClientID = cID; 
	AddLog(LOG_NOTICE, _T("New client ID: %ld"), cID);

	XmlEvents.Fire_OnConnectedToServer(ServerState.dwAddr, ServerState.uPort, ServerState.nClientID);
}

//////////////////////////////////////////////////////////////////////
void CEmEngine::ConnectedTo(CEmClient_Server* pClient)
{
	ServerState.hSocket = pClient->m_hSocket;
	ServerState.dwAddr = pClient->m_dwAddr;
	ServerState.uPort = pClient->m_uPort;
}

//////////////////////////////////////////////////////////////////////
bool CEmEngine::IsLocalServer(ULONG nAddr, USHORT uPort)
{
	if(!ServerState.IsConnected())
		return false;
	/*	if(m_stConnected.pServer)
	{
	if (m_stConnected.pServer->GetIP() == nAddr &&
	m_stConnected.pServer->GetPort() == uPort)
	return true;
	}*/
	return false;
}

//////////////////////////////////////////////////////////////////////
void CEmEngine::ProcessSocketsTimeout()
{
	ClientList.PurgeBadClients();
	ClientList.CheckAcceptNewClient();
	ClientList.ProcessUpload();
}

bool CEmEngine::AlertOnErrors() const	
{ 
	return Prefs.AlertOnErrors; 
}
