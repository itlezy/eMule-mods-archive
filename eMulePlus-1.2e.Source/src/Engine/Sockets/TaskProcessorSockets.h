// TaskProcessor_Main.h: interface for the CTaskProcessor_Sockets class.
//
//////////////////////////////////////////////////////////////////////
#ifdef NEW_SOCKETS
#if !defined(AFX_TASKPROCESSOR_MAIN_H__E3110A38_4DB8_41A3_B7B4_F4481D9DDCB1__INCLUDED_)
#define AFX_TASKPROCESSOR_MAIN_H__E3110A38_4DB8_41A3_B7B4_F4481D9DDCB1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../TaskProcessor.h"
#ifdef SUPPORT_CLIENT_PEER
	#include "../../OtherFunctions.h"
#endif //SUPPORT_CLIENT_PEER


struct CTcpCompletionTask;
class CClientMule;

struct CEmClient : public CLoggable2
{
	SOCKET	m_hSocket;
	bool	m_bIsConnected;
	bool	m_bFromOutside;

	ULONG	m_dwAddr;
	USHORT	m_uPort;
	ULONG	m_nClientID; // must be generated automatically

	CTcpCompletionTask	*m_pOnCompletionTask;

	CEmClient()
		: m_hSocket(0), m_bIsConnected(false), m_bFromOutside(false),
			m_dwAddr(0), m_uPort(0), m_nClientID(0),
			m_pOnCompletionTask(NULL) {}
	virtual ~CEmClient();
	virtual T_CLIENT_TYPE GetType() const = 0;

	virtual void OnConnected();
	virtual void OnAccepted();
	virtual void OnDisconnected();
};

#ifdef SUPPORT_CLIENT_PEER
struct CEmClient_Peer : public CEmClient
{
	CEmClient_Peer();
	virtual T_CLIENT_TYPE GetType() const;
//	virtual void OnConnected();
	virtual void OnDisconnected();

	__declspec(property(get=_GetClientMule, put=_PutClientMule)) CClientMule* Mule;

	CClientMule* _GetClientMule();
	void _PutClientMule(CClientMule* pMule){ m_pMule = pMule; }

protected:
	CClientMule* m_pMule;
};

struct CEmClient_Server : public CEmClient_Peer
{
	virtual T_CLIENT_TYPE GetType() const;
//	virtual void OnConnected();
//	virtual void OnDisconnected();

	int	m_nCredits; // Will use later to check for credits left
	int	m_nMsgCount; // To check that server version appears in first message
};
#endif //SUPPORT_CLIENT_PEER

struct CEmClient_Web : public CEmClient {

	virtual T_CLIENT_TYPE GetType() const;
	// Possibly some user's stuff.
//	virtual void OnConnected();
};

struct CEmClient_Xml : public CEmClient {

	virtual T_CLIENT_TYPE GetType() const;
};

typedef map<SOCKET, CEmClient*> SocketClientMap;

class CTaskProcessor_Sockets : public CTaskProcessor
{
	virtual bool Start();
	virtual void Stop();
	virtual void ProcessTimeout();

	ULONG m_nLastClientID;

public:
	CTaskProcessor_Sockets() :
		m_nLastClientID(0)
		{}

	SocketClientMap	m_mapClients;

public:
	CEmClient* AllocClient(T_CLIENT_TYPE);
	CEmClient* CTaskProcessor_Sockets::Lookup(SOCKET hSocket)
	{
		SocketClientMap::iterator pos = m_mapClients.find(hSocket);
		return (pos != m_mapClients.end() ? (*pos).second : NULL);
	}

	CEmClient* AllocTcpConnect(ULONG nAddr, USHORT nPort, T_CLIENT_TYPE, CTcpCompletionTask *pOnCompletionTask);
	void AddClientToMap(SOCKET hSocket, CEmClient *pClient);
	bool AllocSend(SOCKET hSocket, PCVOID pData, DWORD dwLen, EnumQueuePriority ePriority);

	void KillClient(CEmClient*);
};
#endif // !defined(AFX_TASKPROCESSOR_MAIN_H__E3110A38_4DB8_41A3_B7B4_F4481D9DDCB1__INCLUDED_)
#endif