// Tasks.h
//
//////////////////////////////////////////////////////////////////////
#ifdef NEW_SOCKETS
#if !defined(AFX_TASKS_H__595D01B7_787E_4BAA_8F57_B70CFE759503__INCLUDED_)
#define AFX_TASKS_H__595D01B7_787E_4BAA_8F57_B70CFE759503__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../TaskProcessor.h"
#include "TaskProcessorSockets.h"

//class CServer;
//struct CEmClient;
//struct CEmClient_Server;
//struct CEmClient_Peer;


//////////////////////////////////////////////////////////////////////
// Internal socket tasks
struct CTask_Tcp : public CTask
{
	SOCKET m_hSocket;
	CTask_Tcp() : m_hSocket(INVALID_SOCKET) {}
};

struct CTask_Tcp_Accepted : public CTask_Tcp {
	virtual bool Process();
	in_addr m_nAddr;
	USHORT m_uPort;
	T_CLIENT_TYPE m_eType;
	virtual LPCTSTR TaskName(){ return _T("Tcp_Accepted"); }
};

struct CTask_Tcp_Connected : public CTask_Tcp {
	virtual bool Process();
	virtual LPCTSTR TaskName(){ return _T("Tcp_Connected"); }
};

struct CTask_Tcp_Web : public CTask_Tcp {
	virtual bool Process();
	virtual LPCTSTR TaskName(){ return _T("Tcp_Web"); }
	DWORD m_dwHeaderLen;
	DWORD m_dwContentLen;
	BYTE m_pBuf[0]; // variable size
	void* operator new (size_t nSize, DWORD dwDataSize) { return malloc(nSize + dwDataSize); }
	void operator delete (void* pPtr) { free(pPtr); }
	void operator delete (void* pPtr, DWORD) { free(pPtr); }
};

struct CTask_Tcp_Xml : public CTask_Tcp {
	virtual bool Process();
	virtual LPCTSTR TaskName(){ return _T("Tcp_Xml"); }
	DWORD m_dwLength;
	BYTE m_pBuf[0]; // variable size
	void* operator new (size_t nSize, DWORD dwDataSize) { return malloc(nSize + dwDataSize); }
	void operator delete (void* pPtr) { free(pPtr); }
	void operator delete (void* pPtr, DWORD) { free(pPtr); }
};

struct CTask_Tcp_Err : public CTask_Tcp {
	virtual bool Process();
	virtual LPCTSTR TaskName(){ return _T("Tcp_Err"); }
	int m_nError;

	static bool Post(SOCKET hSocket, int nErrorCode);
};
// End of internal socket tasks

//////////////////////////////////////////////////////////////////////
struct CTcpCompletionTask : public CTask
{
	virtual void			SetClient(CEmClient *pClient) = 0;

	int						m_iMessage; // Socket completion message
};

//////////////////////////////////////////////////////////////////////
// connect to any client (or server)
struct CTask_Connect : public CTask
{
	CTask_Connect(ULONG, USHORT, T_CLIENT_TYPE, CTcpCompletionTask *pOnCompletionTask);
	CTask_Connect(LPCTSTR, USHORT, T_CLIENT_TYPE, CTcpCompletionTask *pOnCompletionTask);
	virtual bool Process();
	virtual LPCTSTR TaskName(){ return _T("Connect"); }

	ULONG			m_dwAddr;
	USHORT			m_uPort;
	T_CLIENT_TYPE	m_eType;

	CTcpCompletionTask	   *m_pOnCompletionTask;
};

//////////////////////////////////////////////////////////////////////
// disconnect from any client
struct CTask_KillClient : public CTask {
	CTask_KillClient(CEmClient*);
	virtual bool Process();
	virtual LPCTSTR TaskName(){ return _T("KillClient"); }

	CEmClient* m_pClient;
};



#endif // !defined(AFX_TASKS_H__595D01B7_787E_4BAA_8F57_B70CFE759503__INCLUDED_)
#endif