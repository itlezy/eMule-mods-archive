// TcpEngine.h: interface for the CTcpEngine class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_SOCKETS
#if !defined(AFX_TCPENGINE_H__F572FB64_4C8F_4112_A2B3_CE1A75ACCF86__INCLUDED_)
#define AFX_TCPENGINE_H__F572FB64_4C8F_4112_A2B3_CE1A75ACCF86__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <deque>

// Constants
const UINT INTERFACE_FLAG_SHUTDOWN	= 0x01;

const UINT WM_EMSOCKET			= WM_USER + 1;
const UINT WM_EM_ADDINTERFACE	= WM_USER + 2;
const UINT WM_EM_CONNECT		= WM_USER + 3;
const UINT WM_EM_SEND			= WM_USER + 4;
const UINT WM_EM_DISCONNECT		= WM_USER + 5;

const UINT TCP_OPTIMAL_BUF		= 0x1000; // 4k
const UINT TCP_MAX_BUF_SIZE		= 200000;

const UINT TCP_RECV_FLAG_CONNECTED = 0x01;
const UINT TCP_RECV_FLAG_FIRSTTIME = 0x02;

const UINT UPLOAD_BLOCK_SIZE		= 512;

// Time in milliseconds
const UINT TIMER_CHECK_TIME			= 100;	// for limiting bandwidth on timer

const ULONG LISTEN_TO_INTERNET	= INADDR_ANY;
const ULONG LISTEN_TO_LOOPBACK	= htonl(INADDR_LOOPBACK);

class CTcpEngine : public CLoggable2 // : protected CThreadPool
{
protected:
	bool m_bWinsockInit;

	struct CInterface {
		USHORT	m_uPort;
		SOCKET	m_hSocket;
		long	m_nPendingAccepts;
		UINT	m_nFlags;
		T_CLIENT_TYPE	m_eType;
	};
	struct CState;

	std::queue<CInterface*> m_queueInterfaces;

	struct OVERLAPPED_TCP : public OVERLAPPED
	{
		union
		{
			struct
			{
				OVERLAPPED_TCP* m_pPrev;
				OVERLAPPED_TCP* m_pNext;
			};
			CTcpEngine* m_pEngine;
		};

		SOCKET m_hSocket;

		virtual void ProcessCompletion(CTcpEngine&, int nError, DWORD dwBytes);
		virtual void ProcessEvent(CState&, int nEvent, int nError);

		OVERLAPPED_TCP() :
			m_pPrev(NULL),
			m_pNext(NULL)
			{ ZeroMemory((OVERLAPPED*) this, sizeof(OVERLAPPED)); }
		virtual ~OVERLAPPED_TCP();

		static void WINAPI OverlappedCompletionFunc(DWORD, DWORD, OVERLAPPED*, DWORD);
	};
	struct OVERLAPPED_ACCEPT : public OVERLAPPED_TCP 
	{
		virtual void ProcessCompletion(CTcpEngine&, int nError, DWORD dwBytes);
		virtual void ProcessEvent(CState&, int nEvent, int nError);
		CInterface& m_stInterface;
		char m_pBuf[(sizeof(SOCKADDR_IN) + 16) * 2]; // used with AcceptEx function

		OVERLAPPED_ACCEPT(CInterface& stInterface) : m_stInterface(stInterface) { InterlockedIncrement(&m_stInterface.m_nPendingAccepts); }
		virtual ~OVERLAPPED_ACCEPT();
	};
	struct OVERLAPPED_RECV : public OVERLAPPED_TCP, public WSABUF
	{
		virtual void ProcessCompletion(CTcpEngine&, int nError, DWORD dwBytes);
		virtual void ProcessEvent(CState&, int nEvent, int nError);
		char	m_pBuf[TCP_OPTIMAL_BUF];
		DWORD	m_dwBufUsage;
		DWORD	m_dwBufSize;
		UINT	m_nFlags;

		ULONG	m_nPeerAddr;
		USHORT	m_nPeerPort;

		char*	m_pBufExtra;
		inline char* GetBuffer() { return m_pBufExtra ? m_pBufExtra : m_pBuf; }

		void DisableTcpBufs(CTcpEngine&);
		void ClearBufs();

		inline OVERLAPPED_RECV() :
			m_pBufExtra(NULL),
			m_dwBufUsage(0),
			m_dwBufSize(sizeof(m_pBuf))
			{}
		virtual ~OVERLAPPED_RECV();

		virtual DWORD ParseRecv(CTcpEngine&, bool bIsLastRecv) = NULL;
	};
	struct OVERLAPPED_RECV_WEB : public OVERLAPPED_RECV {
		virtual DWORD ParseRecv(CTcpEngine&, bool bIsLastRecv);

		DWORD m_dwHttpHeaderLen;
		DWORD m_dwHttpContentLen;

		inline OVERLAPPED_RECV_WEB() :
			m_dwHttpHeaderLen(0),
			m_dwHttpContentLen(0)
			{}
	};
	struct OVERLAPPED_RECV_XML : public OVERLAPPED_RECV {
		virtual DWORD ParseRecv(CTcpEngine&, bool bIsLastRecv);

		DWORD m_dwLength;

		inline OVERLAPPED_RECV_XML() :
			m_dwLength(0)
			{}
	};

	struct OVERLAPPED_SEND : public OVERLAPPED_TCP, public WSABUF {
		virtual void ProcessCompletion(CTcpEngine&, int nError, DWORD dwBytes);
		BYTE m_pBuf[0];
		void* operator new (size_t nSize, DWORD dwDataSize) { return malloc(nSize + dwDataSize); }
		void operator delete(void* pPtr, DWORD) { free(pPtr); }
		void operator delete(void* pPtr) { free(pPtr); }
	};

	//////////////////////////////////////////////////////////////////////
	// Asynchronous engine
	bool InitAsync();
	void UninitAsync();

	HANDLE	m_hAsyncInit; // initialization completion event
	HANDLE	m_hAsyncThread;
	HWND	m_hSocketWnd;
	long	m_nOverlappedIOs;

	static UINT WINAPI SocketWndFunc(PVOID);
	static LRESULT WINAPI SocketWndProc(HWND, UINT, WPARAM, LPARAM);

	virtual void OnSend(CState* pState, OVERLAPPED_SEND* pSend, EnumQueuePriority ePriority);
	virtual void OnTimer(CState* pState);

	struct CState 
	{


		CTcpEngine& m_stEngine;
		OVERLAPPED_TCP* m_pHashTable[997];

		CState(CTcpEngine& stEngine) : m_stEngine(stEngine)
		{
			ZeroMemory(m_pHashTable, sizeof(m_pHashTable));
		}

		void AddClient(OVERLAPPED_TCP&);
		OVERLAPPED_TCP* LookupClient(SOCKET hSocket);
		void DeleteClient(OVERLAPPED_TCP&);

		void OnAddInterface(CInterface&);

		bool SendBlock(OVERLAPPED_SEND* pSend);
	};

	virtual OVERLAPPED_RECV* AllocRecv(T_CLIENT_TYPE eType);

	friend struct CState;
	friend struct OVERLAPPED_TCP;
	friend struct OVERLAPPED_RECV;
	friend struct OVERLAPPED_ACCEPT;

	//////////////////////////////////////////////////////////////////////
	// Overlapped engine

public:

	bool Init();
	void Uninit(bool bCleanupWinsock = true);

	CTcpEngine() 
		:m_bWinsockInit(false)
		,m_hAsyncInit(NULL)
		,m_hAsyncThread(NULL)
		,m_hSocketWnd(NULL)
		,m_nOverlappedIOs(0)
	{}
	~CTcpEngine() { Uninit(); }

	bool AddInterface(USHORT nPort, T_CLIENT_TYPE, ULONG uListen = LISTEN_TO_INTERNET);

	bool AllocSend(SOCKET hSocket, PCVOID, DWORD, EnumQueuePriority);

	void AllocDisconnect(SOCKET hSocket);
	SOCKET AllocConnect(ULONG nAddr, USHORT nPort, T_CLIENT_TYPE);
};

#endif // !defined(AFX_TCPENGINE_H__F572FB64_4C8F_4112_A2B3_CE1A75ACCF86__INCLUDED_)
#endif