// Tasks.cpp: implementation of the CTasks class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TasksSockets.h"
#include "TaskProcessorSockets.h"
#include "../XML/XML.h"

//////////////////////////////////////////////////////////////////////
// global and sockets task functions
CTask::~CTask()
{
}

bool CTask::OnException()
{
	// delete this buggy task
	return true;
}

bool CTask_Tcp_Err::Post(SOCKET hSocket, int nErrorCode)
{
	CTask_Tcp_Err* pTask = new CTask_Tcp_Err;
	if (pTask)
	{
		pTask->m_hSocket = hSocket;
		pTask->m_nError = nErrorCode;
		g_stEngine.Sockets.Push(pTask);
	} 
	else
	{
		AddLog(LOG_ERROR, _T("No memory"));
	}

	return NULL != pTask;
}

bool CTask_Tcp_Err::Process()
{
	// check if we still have this client
	CEmClient* pClient = g_stEngine.Sockets.Lookup(m_hSocket);
	if (pClient)
	{
		ASSERT(pClient->m_hSocket == m_hSocket);
		// here we can write to the log that this client has disconnected from us

		g_stEngine.Sockets.KillClient(pClient);
	}

	return true;
}

bool CTask_Tcp_Accepted::Process()
{
	CEmClient* pClient = g_stEngine.Sockets.AllocClient(m_eType);
	ASSERT(pClient);
	if (pClient)
	{
		pClient->m_bFromOutside = true;
		pClient->m_hSocket = m_hSocket;
		pClient->m_bIsConnected = true;
		g_stEngine.Sockets.AddClientToMap(m_hSocket, pClient);

		pClient->OnAccepted();

	} else
	{
		AddLog(LOG_ERROR, _T("No memory"));
		if(closesocket(m_hSocket))
			AddLog(LOG_ERROR, _T("closesocket failed"));
	}

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CTask_Tcp_Connected::Process()
{
	CEmClient *pClient = g_stEngine.Sockets.Lookup(m_hSocket);

	if (pClient != NULL)
	{
		ASSERT(!pClient->m_bIsConnected);
		pClient->m_bIsConnected = true;

		pClient->OnConnected();
	}
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CTask_Tcp_Web::Process()
{
	CEmClient_Web* pClient = (CEmClient_Web*) g_stEngine.Sockets.Lookup(m_hSocket);
	if (pClient)
	{
		ASSERT(T_CLIENT_WEB == pClient->GetType());

/*		  CString strTxt;
		  strTxt.Format("Currently I have %d clients", g_stEngine.m_stSocketsTP.m_mapClients.size());
		  g_stEngine.m_stTcp.AllocSend(m_hSocket, (LPCTSTR) strTxt, strTxt.GetLength());
		  g_stEngine.m_stTcp.AllocDisconnect(m_hSocket);*/
	}

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CTask_Tcp_Xml::Process()
{
	CEmClient_Xml* pClient = (CEmClient_Xml*) g_stEngine.Sockets.Lookup(m_hSocket);
	if (pClient)
	{
		ASSERT(T_CLIENT_XML == pClient->GetType());

		//AddDebugLogLine(_T("Received: %s"), m_pBuf);
		CString sXml((LPCTSTR)m_pBuf);
		CXmlTask* pTask = CXmlTask::ParseXml(sXml);
		if(pTask)
		{
			pTask->SetClient(pClient);
			g_stEngine.Sockets.Push(pTask);
		}
/*		CString strTxt;
		strTxt.Format("Currently I have %d clients", g_stEngine.m_stSocketsTP.m_mapClients.size());
		g_stEngine.m_stTcp.AllocSend(m_hSocket, (LPCTSTR) strTxt, strTxt.GetLength());
		g_stEngine.m_stTcp.AllocDisconnect(m_hSocket);*/
	}

	return true;
}
// end of global and socket task functions
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// connect
CTask_Connect::CTask_Connect(ULONG nAddr, USHORT nPort, T_CLIENT_TYPE eType, CTcpCompletionTask *pOnCompletionTask/*= NULL*/)
	: m_dwAddr(nAddr), m_uPort(nPort), m_eType(eType), m_pOnCompletionTask(pOnCompletionTask)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTask_Connect::CTask_Connect(LPCTSTR sAddr, USHORT nPort, T_CLIENT_TYPE eType, CTcpCompletionTask *pOnCompletionTask/*= NULL*/) 
	: m_dwAddr(inet_addr(sAddr)), m_uPort(nPort), m_eType(eType), m_pOnCompletionTask(pOnCompletionTask)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CTask_Connect::Process()
{
	if (m_dwAddr != 0 && m_uPort != 0)
	{
		g_stEngine.Sockets.AllocTcpConnect(m_dwAddr, m_uPort, m_eType, m_pOnCompletionTask);
	}
	else
	{
	//	Connect to any server
	//	For now, we'll hardcode Razorback say?
		m_dwAddr = inet_addr(_T("195.245.244.243"));
		m_uPort = 4661;
		g_stEngine.Sockets.AllocTcpConnect(m_dwAddr, m_uPort, m_eType, m_pOnCompletionTask);
	}

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// disconnect
CTask_KillClient::CTask_KillClient(CEmClient *pClient)
	: m_pClient(pClient)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CTask_KillClient::Process()
{
	g_stEngine.Sockets.KillClient(m_pClient);

	return true;
}

