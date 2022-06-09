// EmWinNt.h: interface for the CArlNt class.
//
//////////////////////////////////////////////////////////////////////
#ifdef NEW_SOCKETS
#if !defined(AFX_ARLNT_H__566F5322_6BCF_4D6D_AB47_21C3FCBBE7F7__INCLUDED_)
#define AFX_ARLNT_H__566F5322_6BCF_4D6D_AB47_21C3FCBBE7F7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CEmDynModule : public CLoggable2
{
protected:
	HMODULE m_hDll;
	void Uninit();
public:
	CEmDynModule() : m_hDll(NULL) {}
	~CEmDynModule() { Uninit(); }
	inline bool IsInitialized() { return NULL != m_hDll; }
};

class CEmWinNT : public CEmDynModule
{
public:
	CEmWinNT();

	HANDLE (WINAPI	*m_pfnCreateIoCompletionPort)(HANDLE, HANDLE, DWORD, DWORD);
	BOOL (WINAPI	*m_pfnGetQueuedCompletionStatus)(HANDLE, PDWORD, PDWORD, OVERLAPPED**, DWORD);
	BOOL (WINAPI	*m_pfnPostQueuedCompletionStatus)(HANDLE, DWORD, DWORD, OVERLAPPED*);
	BOOL (WINAPI	*m_pfnTryEnterCriticalSection)(CRITICAL_SECTION*);

	static CEmWinNT s_stWinNT;
};

class CEmMswSock : public CEmDynModule
{
public:
	CEmMswSock();

	BOOL (WINAPI *m_pfnAcceptEx)(SOCKET, SOCKET, PVOID, DWORD, DWORD, DWORD, DWORD*, OVERLAPPED*);
	void (WINAPI *m_pfnGetAcceptExSockaddrs)(PVOID, DWORD, DWORD, DWORD, SOCKADDR**, int*, SOCKADDR**, int*);
};

#endif // !defined(AFX_ARLNT_H__566F5322_6BCF_4D6D_AB47_21C3FCBBE7F7__INCLUDED_)
#endif