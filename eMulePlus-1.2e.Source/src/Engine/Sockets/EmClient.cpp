// EmClient.cpp
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OpCode.h"
#include "TaskProcessorSockets.h"
#ifdef SUPPORT_CLIENT_PEER
	#include "../Data/ClientList.h"
#endif //SUPPORT_CLIENT_PEER

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CEmClient::~CEmClient()
{
	if(closesocket(m_hSocket))
		AddLog(LOG_ERROR, _T("closesocket failed"));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef SUPPORT_CLIENT_PEER
T_CLIENT_TYPE CEmClient_Peer::GetType() const
{
	return T_CLIENT_PEER;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
T_CLIENT_TYPE CEmClient_Server::GetType() const
{
	return T_CLIENT_SERVER;
}
#endif //SUPPORT_CLIENT_PEER
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
T_CLIENT_TYPE CEmClient_Web::GetType() const
{
	return T_CLIENT_WEB;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
T_CLIENT_TYPE CEmClient_Xml::GetType() const
{
	return T_CLIENT_XML;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmClient::OnConnected()
{
	if (m_pOnCompletionTask != NULL)
	{
		m_pOnCompletionTask->m_iMessage = FD_CONNECT;
		g_stEngine.Sockets.Push(m_pOnCompletionTask);
		m_pOnCompletionTask = NULL;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmClient::OnAccepted()
{
	if (m_pOnCompletionTask != NULL)
	{
		m_pOnCompletionTask->m_iMessage = FD_ACCEPT;
		g_stEngine.Sockets.Push(m_pOnCompletionTask);
		m_pOnCompletionTask = NULL;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmClient::OnDisconnected()
{
	if (m_pOnCompletionTask != NULL)
	{
		m_pOnCompletionTask->m_iMessage = FD_CLOSE;
		g_stEngine.Sockets.Push(m_pOnCompletionTask);
		m_pOnCompletionTask = NULL;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef SUPPORT_CLIENT_PEER
CEmClient_Peer::CEmClient_Peer()
{
	m_pMule = NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmClient_Peer::OnDisconnected()
{ 
	if(m_pMule)
	{
		m_pMule->OnDisconnected();
		// Do not delete the object, it remains in the client list (until I decide other :)
		m_pMule = NULL; 
	}
	CEmClient::OnDisconnected(); 
}

CClientMule* CEmClient_Peer::_GetClientMule()
{
	if(m_pMule != NULL && g_stEngine.ClientList.IsValidClient(m_pMule))
		return m_pMule;
	else
	{
		// Removed from list...
		if(m_pMule != NULL)
		{
			AddLog(LOG_DEBUG, _T("Client mule has been removed? client (%s:%d)"), inet_ntoa(*reinterpret_cast<in_addr*>(&m_dwAddr)), m_uPort);
			m_pMule->OnDisconnected();
			m_pMule = NULL;
		}
		return NULL;
	}
}
#endif //SUPPORT_CLIENT_PEER

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// in the future - move webserver to this engine
//void CEmClient_Web::OnConnected()
//{
//	// say something...
//	char szSend[] = "GET / HTTP 1.1\r\n\r\n";
//	stEngine.m_stTcp.AllocSend(m_hSocket, szSend, sizeof(szSend) - 1);
//}
