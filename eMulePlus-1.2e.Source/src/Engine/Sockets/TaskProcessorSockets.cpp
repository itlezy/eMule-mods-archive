// TaskProcessor_Main.cpp: implementation of the CTaskProcessor_Sockets class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "TaskProcessorSockets.h"
#include "TasksSockets.h"
#include "TcpEngine.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// starting
bool CTaskProcessor_Sockets::Start()
{
//	m_mapClients.InitHashTable(997);
	m_dwWaitTimeout = 500;
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// destroy all clients that are possibly left in our map
//	MOREVIT: I'm thinking that some clients may be destroyed from our side (such as we currently do with unwanted
//		sources.) If we don't communicate that here we'll run into some of the same problems we had before with
//		calls being invoked on dead clients. Client should have a backpointer to processor?
void CTaskProcessor_Sockets::Stop()
{
	for (SocketClientMap::iterator pos = m_mapClients.begin(); pos != m_mapClients.end(); pos++)
	{
		CEmClient* pClient = (*pos).second;

//		ASSERT(pClient != NULL);	<= This is unnecessary. The C++ standard says delete MUST accept NULL gracefully.
		delete pClient;
	}
	m_mapClients.clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// processing timeout
// this should replace all old Process() functions
void CTaskProcessor_Sockets::ProcessTimeout()
{
/*
//	Keep connection alive once per minute
//	Recommended by lugdunummaster
	if (g_stEngine.IsConnected())
	{
		// sending empty OFFER FILES opcode
	}
*/
	// Execute this in engine cause we use TaskProcessor_Sockets in several modules
	g_stEngine.ProcessSocketsTimeout();

	// Perform check 10 times per second
	m_dwWaitTimeout = 100;
}
//////////////////////////////////////////////////////////////////////
// creating new client object
CEmClient *CTaskProcessor_Sockets::AllocClient(T_CLIENT_TYPE eType)
{
	CEmClient* pClient;

	switch (eType)
	{
#ifdef SUPPORT_CLIENT_PEER
		case T_CLIENT_PEER:
			pClient = new CEmClient_Peer;
			break;
		case T_CLIENT_SERVER:
			pClient = new CEmClient_Server;
			break;
#endif //SUPPORT_CLIENT_PEER
		case T_CLIENT_WEB:
			pClient = new CEmClient_Web;
			break;
		case T_CLIENT_XML:
			pClient = new CEmClient_Xml;
			break;
		default:
			ASSERT(FALSE); // invalid type ???
			return NULL;
	}
	if (pClient != NULL)
	{
		pClient->m_nClientID = ++m_nLastClientID;
	}

	return pClient;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// connecting to any client (or server)
CEmClient *CTaskProcessor_Sockets::AllocTcpConnect(ULONG nAddr, USHORT nPort, T_CLIENT_TYPE eClientType,
												   CTcpCompletionTask *pOnCompletionTask )
{
	CEmClient* pClient = AllocClient(eClientType);

	if (pClient != NULL)
	{
		pClient->m_bFromOutside = false;
		pClient->m_dwAddr = nAddr;
		pClient->m_uPort = nPort;
		pClient->m_pOnCompletionTask = pOnCompletionTask;
		if(pOnCompletionTask)
			pOnCompletionTask->SetClient(pClient);

		SOCKET		hSocket = g_stEngine.GetTcpEngine()->AllocConnect(nAddr, nPort, eClientType);

		if (INVALID_SOCKET == hSocket)
		{
			delete pClient;
			pClient = NULL;
		}
		else
		{
			pClient->m_hSocket = hSocket;
			pClient->m_bIsConnected = false;

			m_mapClients[hSocket] = pClient;
		}
	} 
	else
	{
		AddLog(LOG_ERROR, _T("No memory"));
	}

	return pClient;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// disconnect existing client and destroy its object
void CTaskProcessor_Sockets::KillClient(CEmClient *pClient)
{
	ASSERT(pClient);

	SOCKET hSocket = pClient->m_hSocket;

	pClient->OnDisconnected();
	delete pClient;

	m_mapClients.erase(hSocket);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CTaskProcessor_Sockets::AddClientToMap(SOCKET hSocket, CEmClient *pClient)
{
	m_mapClients[hSocket] = pClient;
}

bool CTaskProcessor_Sockets::AllocSend(SOCKET hSocket, PCVOID pData, DWORD dwLen, EnumQueuePriority ePriority)
{
	return g_stEngine.GetTcpEngine()->AllocSend(hSocket, pData, dwLen, ePriority);
}
