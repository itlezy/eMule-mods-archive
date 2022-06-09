// TcpEngine.cpp: implementation of the CTcpEngine class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TcpEngine.h"

#include "TasksSockets.h"

#include <process.h>



//////////////////////////////////////////////////////////////////////
// Start
bool CTcpEngine::InitAsync()
{
	if (m_hAsyncInit = CreateEvent(NULL, TRUE, FALSE, NULL))
	{
		// Create a thread for handling standard sockets events
		UINT nThread = 0;
		if (m_hAsyncThread = (HANDLE) _beginthreadex(NULL, 0, SocketWndFunc, this, 0, &nThread))
		{
			// Very well. Wait now until it finishes its initialization
			HANDLE pWait[] = { m_hAsyncInit, m_hAsyncThread };
			if (WAIT_OBJECT_0 == WaitForMultipleObjects(_countof(pWait), pWait, FALSE, 2000))
				return true;	// Everything is ok
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////
// stop
void CTcpEngine::UninitAsync()
{
	if (m_hAsyncInit)
	{
		if (m_hAsyncThread)
		{
			if (m_hSocketWnd)
				VERIFY(PostMessage(m_hSocketWnd, WM_QUIT, 0, 0));

			if (WAIT_OBJECT_0 != WaitForSingleObject(m_hAsyncThread, 2000))
			{
				AddLog(LOG_ERROR, _T("Thread has not terminated."));
				if (!TerminateThread(m_hAsyncThread, -1))
					AddLog(LOG_ERROR, _T("TerminateThread failed"));
			}
			VERIFY(CloseHandle(m_hAsyncThread));
			m_hAsyncThread = NULL;
		}

		VERIFY(CloseHandle(m_hAsyncInit));
		m_hAsyncInit = NULL;
	}
	m_nOverlappedIOs = 0;
}

//////////////////////////////////////////////////////////////////////
UINT WINAPI CTcpEngine::SocketWndFunc(PVOID pPtr)
{
	CTcpEngine* pThis = (CTcpEngine*) pPtr;
	ASSERT(pThis);
	if (pThis)
	{
		ASSERT(!pThis->m_hSocketWnd);
		HINSTANCE hInstance = GetModuleHandle(NULL);

		WNDCLASS stWC;
		ZeroMemory(&stWC, sizeof(stWC));
		stWC.hInstance = hInstance;
		stWC.lpfnWndProc = SocketWndProc;
		stWC.lpszClassName = _T("EM_SockWnd");
		stWC.cbWndExtra = sizeof(ULONG_PTR);

		ATOM aClass = RegisterClass(&stWC);
		ASSERT(aClass);

		if (aClass)
		{
			if (pThis->m_hSocketWnd = CreateWindow((LPCTSTR) aClass, _T(""), 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL))
			{
				CState stState(*pThis);

				SetWindowLong(pThis->m_hSocketWnd, GWL_USERDATA, (long) &stState);

				VERIFY(SetEvent(pThis->m_hAsyncInit));

				SetTimer(pThis->m_hSocketWnd, 1, TIMER_CHECK_TIME, NULL);

				// From now on - run the message loop, so that this thread most of the time
				// is in alertable state.
				for (bool bRun = true; bRun; )
				{
					MsgWaitForMultipleObjectsEx(0, NULL, INFINITE, QS_ALLEVENTS, MWMO_ALERTABLE);

					MSG stMsg;
					while (PeekMessage(&stMsg, NULL, 0, 0, PM_REMOVE))
						if (WM_QUIT == stMsg.message)
							bRun = false;
						else
							DispatchMessage(&stMsg);
				}

				if (IsWindow(pThis->m_hSocketWnd))
					VERIFY(DestroyWindow(pThis->m_hSocketWnd));

				// destroy all our possibly left clients
				for (UINT nIndex = 0; nIndex < _countof(stState.m_pHashTable); nIndex++)
				{
					OVERLAPPED_TCP* pClient = stState.m_pHashTable[nIndex];
					while (pClient)
					{
						OVERLAPPED_TCP* pNext = pClient->m_pNext;
						delete pClient;
						pClient = pNext;
					}
				}

				// Wait until all pending IO operations return with error
				const DWORD PENDING_WAIT_MAX = 1000;
				DWORD dwTicks = GetTickCount();
				while (true)
				{
					if (pThis->m_nOverlappedIOs <= 0)
						break;

					DWORD dwWait = GetTickCount() - dwTicks;
					if (dwWait >= PENDING_WAIT_MAX)
						break;

					dwWait = PENDING_WAIT_MAX - dwWait;
					MsgWaitForMultipleObjectsEx(0, NULL, dwWait, QS_ALLEVENTS, MWMO_ALERTABLE);
				}

				if (pThis->m_nOverlappedIOs)
					AddLog(LOG_ERROR, _T("Overlapped IOs counter is %d"), pThis->m_nOverlappedIOs);
			} else
				ASSERT(FALSE);

			VERIFY(UnregisterClass((LPCTSTR) aClass, hInstance));
		}
	}
	
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT WINAPI CTcpEngine::SocketWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_EMSOCKET:
		case WM_EM_ADDINTERFACE:
		case WM_EM_CONNECT:
		case WM_EM_SEND:
		case WM_EM_DISCONNECT:
		case WM_TIMER:
		{
			break;
		}
		default:
		{
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}

	CState* pState = (CState*) GetWindowLong(hWnd, GWL_USERDATA);

	ASSERT(pState);

	if (pState)
	{
		switch (uMsg)
		{
			case WM_EMSOCKET:
			{
				OVERLAPPED_TCP* pClient = pState->LookupClient(wParam);

				if (pClient)
				{
					pClient->ProcessEvent(*pState, LOWORD(lParam), HIWORD(lParam));
				}
				break;
			}
			case WM_EM_ADDINTERFACE:
			{
				CInterface* pInterface = (CInterface*) wParam;
				ASSERT(pInterface);
				if (pInterface)
				{
					// associate now a 'client' with this interface
					OVERLAPPED_ACCEPT* pClient = new OVERLAPPED_ACCEPT(*pInterface);
					ASSERT(pClient);
					if (pClient)
					{
						pClient->m_hSocket = pInterface->m_hSocket;
						pState->AddClient(*pClient);

						// just in case there are already pending accept operations
						pClient->ProcessEvent(*pState, FD_ACCEPT, 0);

					} 
					else
					{
						AddLog(LOG_ERROR, _T("No memory"));
						InterlockedDecrement(&pInterface->m_nPendingAccepts);
					}
				}

				InterlockedDecrement(&pInterface->m_nPendingAccepts);

				if (lParam && !SetEvent((HANDLE) lParam))
					AddLog(LOG_ERROR, _T("SetEvent failed"));
				break;
			}
			case WM_EM_CONNECT:
			{
				OVERLAPPED_RECV* pClient = (OVERLAPPED_RECV*)wParam;

				ASSERT(pClient);

				if (pClient != NULL)
				{
					pState->AddClient(*pClient);

					SOCKADDR_IN stAddr;

					stAddr.sin_family = AF_INET;
					stAddr.sin_addr.S_un.S_addr = pClient->m_nPeerAddr;
					stAddr.sin_port = pClient->m_nPeerPort;

					if (connect(pClient->m_hSocket, (sockaddr*)&stAddr, sizeof(stAddr)))
					{
						int		nError = WSAGetLastError();

						if (nError != WSAEWOULDBLOCK)
						{
							pClient->ProcessEvent(*pState, FD_CONNECT, nError);
						}
					}
					else
					{
						pClient->ProcessEvent(*pState, FD_CONNECT, 0);
					}
				}
				break;
			}
			case WM_EM_SEND:
			{
				pState->m_stEngine.OnSend(pState, (OVERLAPPED_SEND*)lParam, (EnumQueuePriority)wParam);
				break;
			}
			case WM_TIMER:
			{
				pState->m_stEngine.OnTimer(pState);
				break;
			}
			case WM_EM_DISCONNECT:
			{
				ASSERT(INVALID_SOCKET != lParam);
				if ((INVALID_SOCKET != lParam) && shutdown((SOCKET) lParam, SD_SEND))
					CTask_Tcp_Err::Post((SOCKET) lParam, WSAGetLastError());

				break;
			}
			default:
			{
				ASSERT(FALSE);
			}
		}
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CTcpEngine::OnSend(CState* pState, OVERLAPPED_SEND* pSend, EnumQueuePriority ePriority)
{
	pState->SendBlock(pSend);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CTcpEngine::OnTimer(CState* pState)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CTcpEngine::CState::SendBlock(OVERLAPPED_SEND* pSend)
{
	DWORD dwSize = 0;

	InterlockedIncrement(&m_stEngine.m_nOverlappedIOs);

	if (pSend && WSASend(pSend->m_hSocket, pSend, 1, &dwSize, 0, pSend, OVERLAPPED_TCP::OverlappedCompletionFunc))
	{
		int nError = WSAGetLastError();
		// If error happened...
		if (WSA_IO_PENDING != nError)
		{
			InterlockedDecrement(&m_stEngine.m_nOverlappedIOs);

			// Notify the EmEngine about this.
			CTask_Tcp_Err::Post(pSend->m_hSocket, nError);
			delete pSend;

			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CTcpEngine::OVERLAPPED_TCP::OverlappedCompletionFunc(DWORD dwError, DWORD dwBytes, OVERLAPPED* pOverlapped, DWORD dwFlags)
{
	OVERLAPPED_TCP* pOverlappedTcp = (OVERLAPPED_TCP*) pOverlapped;
	ASSERT(pOverlappedTcp && pOverlappedTcp->m_pEngine);
	if (pOverlappedTcp && pOverlappedTcp->m_pEngine)
	{
		InterlockedDecrement(&pOverlappedTcp->m_pEngine->m_nOverlappedIOs);
		pOverlappedTcp->ProcessCompletion(*pOverlappedTcp->m_pEngine, dwError, dwBytes);
	}
}

//////////////////////////////////////////////////////////////////////
void CTcpEngine::OVERLAPPED_TCP::ProcessEvent(CTcpEngine::CState&, int, int)
{
	ASSERT(FALSE);
}

//////////////////////////////////////////////////////////////////////
void CTcpEngine::OVERLAPPED_TCP::ProcessCompletion(CTcpEngine&, int, DWORD)
{
	ASSERT(FALSE);
}

//////////////////////////////////////////////////////////////////////
CTcpEngine::OVERLAPPED_TCP::~OVERLAPPED_TCP()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTcpEngine::OVERLAPPED_RECV *CTcpEngine::AllocRecv(T_CLIENT_TYPE eType)
{
	switch (eType)
	{
	case T_CLIENT_WEB:
		return new OVERLAPPED_RECV_WEB;
	case T_CLIENT_XML:
		return new OVERLAPPED_RECV_XML;
	default:
		ASSERT(FALSE); // invalid type
	}

	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CTcpEngine::OVERLAPPED_ACCEPT::ProcessEvent(CTcpEngine::CState& stState, int nEvent, int nError)
{
	if (nError)
	{
		if (INTERFACE_FLAG_SHUTDOWN & m_stInterface.m_nFlags)
		{
			// self destruct
			stState.DeleteClient(*this);
			delete this;
		}
		else
			AddLog(LOG_ERROR, _T("Accept operation has failed"));
	}
	else
	{
		while (true)
		{
			SOCKADDR_IN		stAddr;
			int				nLen = sizeof(stAddr);
			SOCKET			hSocket = accept(m_hSocket, (sockaddr*) &stAddr, &nLen);

			if (INVALID_SOCKET == hSocket)
			{
				if (WSAEWOULDBLOCK != (nError = WSAGetLastError()))
					AddLog(LOG_ERROR, _T("accept failed"));
				break;
			}

		//	Create now a 'receiver' of relevant type
			OVERLAPPED_RECV* pOverlapped = stState.m_stEngine.AllocRecv(m_stInterface.m_eType);

			if (pOverlapped != NULL)
			{
				pOverlapped->m_hSocket = hSocket;
				pOverlapped->m_pEngine = &stState.m_stEngine;
				pOverlapped->m_nFlags = TCP_RECV_FLAG_CONNECTED | TCP_RECV_FLAG_FIRSTTIME;
				pOverlapped->m_nPeerAddr = stAddr.sin_addr.S_un.S_addr;
				pOverlapped->m_nPeerPort = stAddr.sin_port;

			//	Notify the EmEngine about new connection accepted
				CTask_Tcp_Accepted* pTask = new CTask_Tcp_Accepted;

				if (pTask != NULL)
				{
					pTask->m_hSocket = hSocket;
					pTask->m_nAddr.S_un.S_addr = stAddr.sin_addr.S_un.S_addr;
					pTask->m_uPort = stAddr.sin_port;
					pTask->m_eType = m_stInterface.m_eType;
					g_stEngine.Sockets.Push(pTask);
				} 
				else
					AddLog(LOG_ERROR, _T("No memory"));

				pOverlapped->DisableTcpBufs(stState.m_stEngine);
				pOverlapped->ProcessCompletion(stState.m_stEngine, 0, 0); // will initiate the 1st read operation

			} 
			else
			{
				AddLog(LOG_ERROR, _T("No memory"));
				if(closesocket(hSocket))
					AddLog(LOG_ERROR, _T("closesocket failed"));
			}
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CTcpEngine::OVERLAPPED_ACCEPT::ProcessCompletion(CTcpEngine& stEngine, int nError, DWORD dwBytes)
{
	// completion of AcceptEx operation
}

//////////////////////////////////////////////////////////////////////
CTcpEngine::OVERLAPPED_ACCEPT::~OVERLAPPED_ACCEPT()
{
	InterlockedDecrement(&m_stInterface.m_nPendingAccepts);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CTcpEngine::OVERLAPPED_RECV::ProcessEvent(CTcpEngine::CState& stState, int nEvent, int nError)
{
	stState.DeleteClient(*this);
	m_pEngine = &stState.m_stEngine;

//	The only event we expect is FD_CONNECT
	if (FD_CONNECT == nEvent)
	{
		if (nError)
		{
		//	Notify the EMEngine about unsuccessful connection attempt
			CTask_Tcp_Err::Post(m_hSocket, nError);
			
			delete this;
		}
		else
		{
			CTask_Tcp_Connected		*pTask = new CTask_Tcp_Connected;

			if (pTask != NULL)
			{
				pTask->m_hSocket = m_hSocket;
				g_stEngine.Sockets.Push(pTask);
			} 
			else
				AddLog(LOG_ERROR, _T("No memory"));

			DisableTcpBufs(stState.m_stEngine);

			m_nFlags |= TCP_RECV_FLAG_CONNECTED | TCP_RECV_FLAG_FIRSTTIME;
			ProcessCompletion(stState.m_stEngine, 0, 0); // initiate read operation
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CTcpEngine::OVERLAPPED_RECV::ProcessCompletion(CTcpEngine &stEngine, int nError, DWORD dwBytes)
{
	if (nError)
		dwBytes = 0; // of course

	if (TCP_RECV_FLAG_FIRSTTIME & m_nFlags)
	{
		m_nFlags &= ~TCP_RECV_FLAG_FIRSTTIME;
	}
	else
	{
	//	If the packet contains data and it's not too much to fit in the buffer...
		if (dwBytes != 0 && (dwBytes + m_dwBufUsage <= m_dwBufSize))
		{
			m_dwBufUsage += dwBytes;

			DWORD		dwParsed = ParseRecv(stEngine, false);

			if (dwParsed != 0)
			{
				ASSERT(dwParsed <= m_dwBufUsage);

				if (!(m_dwBufUsage -= dwParsed))
				{
					ClearBufs();
				}
			}
			else
			{
				if (m_dwBufSize == m_dwBufUsage)
				{
					if (m_dwBufSize >= TCP_MAX_BUF_SIZE)
					{
						AddLog(LOG_WARNING, _T("The limit has been reached. Discarding."));
						ClearBufs();
					}
					else
					{
					//	Extend the buffer
						DWORD		dwSizeNew = m_dwBufUsage + TCP_OPTIMAL_BUF;
						char	   *pBufNew = new char[dwSizeNew];

						if (pBufNew != NULL)
						{
							CopyMemory(pBufNew, GetBuffer(), m_dwBufUsage);
							if (m_pBufExtra != NULL)
							{
								delete[] m_pBufExtra;
							}
							m_pBufExtra = pBufNew;
							m_dwBufSize = dwSizeNew;

						}
						else
						{
							AddLog(LOG_ERROR, _T("No memory"));
							ClearBufs();
						}
					}
				}
			}
		}
		else
		{
			// the peer has called shutdown
			ParseRecv(stEngine, true);

			CTask_Tcp_Err::Post(m_hSocket, 0);

			delete this;
			return;
		}
	}

	// initiate another read operation
	buf = GetBuffer() + m_dwBufUsage;
	VERIFY(len = m_dwBufSize - m_dwBufUsage);

	ZeroMemory((OVERLAPPED*) this, sizeof(OVERLAPPED)); // for more safety

	InterlockedIncrement(&stEngine.m_nOverlappedIOs);

	DWORD dwFlag = 0;
	if (WSARecv(m_hSocket, this, 1, &dwBytes, &dwFlag, this, OverlappedCompletionFunc))
	{
		int nError = WSAGetLastError();
		if (WSA_IO_PENDING != nError)
		{
			// oops
			InterlockedDecrement(&stEngine.m_nOverlappedIOs);
			// notify EMEngine about this error
			CTask_Tcp_Err::Post(m_hSocket, nError);
			delete this;
		}
	}
}

//////////////////////////////////////////////////////////////////////
void CTcpEngine::OVERLAPPED_RECV::DisableTcpBufs(CTcpEngine& stEngine)
{
	// From now on we are not interested in futher notifications
	if (WSAAsyncSelect(m_hSocket, stEngine.m_hSocketWnd, 0, 0))
		AddLog(LOG_ERROR, _T("WSAAsyncSelect failed"));

	// disable both read and write built-in socket buffers
	DWORD dwZero = 0;
	if (setsockopt(m_hSocket, SOL_SOCKET, SO_RCVBUF, (char*) &dwZero, sizeof(dwZero)) ||
		setsockopt(m_hSocket, SOL_SOCKET, SO_SNDBUF, (char*) &dwZero, sizeof(dwZero)))
	{
		AddLog(LOG_ERROR, _T("setsockopt failed"));
	}
}

//////////////////////////////////////////////////////////////////////
void CTcpEngine::OVERLAPPED_RECV::ClearBufs()
{
	if (m_pBufExtra)
	{
		delete[] m_pBufExtra;
		m_pBufExtra = NULL;
	}
	m_dwBufUsage = 0;
	m_dwBufSize = sizeof(m_pBuf);
}

//////////////////////////////////////////////////////////////////////
CTcpEngine::OVERLAPPED_RECV::~OVERLAPPED_RECV()
{
	ClearBufs();
}

//////////////////////////////////////////////////////////////////////
void CTcpEngine::OVERLAPPED_SEND::ProcessCompletion(CTcpEngine& stEngine, int nError, DWORD dwBytes)
{
	if (nError)
		// notify EMEngine about this error
		CTask_Tcp_Err::Post(m_hSocket, nError);
	delete this;
}

//////////////////////////////////////////////////////////////////////
void CTcpEngine::CState::AddClient(OVERLAPPED_TCP& stClient)
{
	OVERLAPPED_TCP*& pFirst = m_pHashTable[stClient.m_hSocket % _countof(m_pHashTable)];
	if (pFirst)
	{
		ASSERT(!pFirst->m_pPrev);
		pFirst->m_pPrev = &stClient;
	}
	stClient.m_pNext = pFirst;
	pFirst = &stClient;
}

//////////////////////////////////////////////////////////////////////
CTcpEngine::OVERLAPPED_TCP* CTcpEngine::CState::LookupClient(SOCKET hSocket)
{
	for (OVERLAPPED_TCP* pClient = m_pHashTable[hSocket % _countof(m_pHashTable)]; pClient; pClient = pClient->m_pNext)
		if (pClient->m_hSocket == hSocket)
			break;

	return pClient;
}

//////////////////////////////////////////////////////////////////////
void CTcpEngine::CState::DeleteClient(CTcpEngine::OVERLAPPED_TCP& stClient)
{
	if (stClient.m_pNext)
	{
		ASSERT(stClient.m_pNext->m_pPrev == &stClient);
		stClient.m_pNext->m_pPrev = stClient.m_pPrev;
	}
	if (stClient.m_pPrev)
	{
		ASSERT(stClient.m_pPrev->m_pNext == &stClient);
		stClient.m_pPrev->m_pNext = stClient.m_pNext;
	} else
	{
		OVERLAPPED_TCP*& pFirst = m_pHashTable[stClient.m_hSocket % _countof(m_pHashTable)];
		if (pFirst == &stClient)
			pFirst = stClient.m_pNext;
		else
			ASSERT(FALSE); // must not happen !!!
	}
}

//////////////////////////////////////////////////////////////////////
bool CTcpEngine::Init()
{
	WSADATA stData;
	if (WSAStartup(MAKEWORD(2, 2), &stData))
		AddLog(LOG_ERROR, _T("WSAStartup failed"));
	else
	{
		m_bWinsockInit = true;
		return InitAsync();
	}
	return false;
}

//////////////////////////////////////////////////////////////////////
bool CTcpEngine::AddInterface(USHORT nPort, T_CLIENT_TYPE eType, ULONG uListen)
{
	SOCKET hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET != hSocket)
	{
		SOCKADDR_IN stAddr;
		stAddr.sin_family = AF_INET;
		stAddr.sin_addr.S_un.S_addr = uListen;	// INADDR_ANY
		stAddr.sin_port = htons(nPort);

		if (!bind(hSocket, (sockaddr*) &stAddr, sizeof(stAddr)) &&
			!WSAAsyncSelect(hSocket, m_hSocketWnd, WM_EMSOCKET, FD_ACCEPT) &&
			!listen(hSocket, SOMAXCONN))
		{
			// very well.
			CInterface* pInterface = new CInterface;
			ASSERT(pInterface);
			if (pInterface)
			{
				pInterface->m_hSocket = hSocket;
				pInterface->m_uPort = nPort;
				pInterface->m_eType = eType;
				pInterface->m_nPendingAccepts = 1;

				if (PostMessage(m_hSocketWnd, WM_EM_ADDINTERFACE, (WPARAM) pInterface, 0))
				{
					AddLog(LOG_DEBUG, _T("Added interface for port %u."), nPort);
					m_queueInterfaces.push(pInterface);
					return true; // success
				}

				AddLog(LOG_ERROR, _T("PostMessage failed"));
				delete pInterface;
			} 
			else
				AddLog(LOG_ERROR, _T("No memory"));
		}
		else
			AddLog(LOG_ERROR, _T("Failed to bind to port %u. Error %d (already in use ?)"), nPort, WSAGetLastError());

		if (closesocket(hSocket))
			AddLog(LOG_ERROR, _T("closesocket failed"));
	}
	return false; // failed
}

//////////////////////////////////////////////////////////////////////
void CTcpEngine::Uninit(bool bCleanupWinsock /* = true */)
{
	if (m_bWinsockInit)
	{
		// purge all interfaces
		for (; !m_queueInterfaces.empty(); m_queueInterfaces.pop())
		{
			CInterface* pInterface = m_queueInterfaces.front();
			ASSERT(pInterface);

			pInterface->m_nFlags |= INTERFACE_FLAG_SHUTDOWN;

			if (closesocket(pInterface->m_hSocket))
				AddLog(LOG_ERROR, _T("closesocket failed"));

			if (m_hSocketWnd && !PostMessage(m_hSocketWnd, WM_EMSOCKET, pInterface->m_hSocket, MAKELONG(FD_ACCEPT, -1)))
				AddLog(LOG_ERROR, _T("PostMessage failed"));

			// wait now until all accept operations return with an error
			for (DWORD dwTicks = GetTickCount(); pInterface->m_nPendingAccepts; Sleep(0))
				if (GetTickCount() - dwTicks > 1000)
				{
					AddLog(LOG_ERROR, _T("Not all accept operations returned"));
					break;
				}

			delete pInterface;
		}

		if (bCleanupWinsock)
		{
			UninitAsync();

			if (WSACleanup())
				AddLog(LOG_ERROR, _T("WSACleanup failed"));
			m_bWinsockInit = false;
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SOCKET CTcpEngine::AllocConnect(ULONG nAddr, USHORT nPort, T_CLIENT_TYPE eType)
{
	OVERLAPPED_RECV* pClient = AllocRecv(eType);

	ASSERT(pClient);

	if (pClient != NULL)
	{
		SOCKET		hSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

		if (INVALID_SOCKET == (pClient->m_hSocket = hSocket))
			AddLog(LOG_ERROR, _T("WSASocket failed"));
		else
		{
			if (WSAAsyncSelect(pClient->m_hSocket, m_hSocketWnd, WM_EMSOCKET, FD_CONNECT))
				AddLog(LOG_ERROR, _T("WSAAsyncSelect failed"));
			else
			{
				pClient->m_nFlags = 0;
				pClient->m_nPeerAddr = nAddr;
				pClient->m_nPeerPort = htons(nPort);

				if (PostMessage(m_hSocketWnd, WM_EM_CONNECT, (WPARAM) pClient, 0))
					return hSocket;

			}
			AddLog(LOG_ERROR, _T("PostMessage failed"));
			if (closesocket(pClient->m_hSocket))
				AddLog(LOG_ERROR, _T("closesocket failed"));
		}
		delete pClient;
	} 
	else
		AddLog(LOG_ERROR, _T("No memory"));

	return INVALID_SOCKET;
}

//////////////////////////////////////////////////////////////////////
bool CTcpEngine::AllocSend(SOCKET hSocket, PCVOID pData, DWORD dwSize, EnumQueuePriority ePriority)
{
	while(dwSize)
	{
		DWORD dwPortion = min(UPLOAD_BLOCK_SIZE, dwSize);
		OVERLAPPED_SEND* pOverlapped = new(dwPortion) OVERLAPPED_SEND;
		if (pOverlapped)
		{
			CopyMemory(pOverlapped->m_pBuf, pData, dwPortion);
			pOverlapped->buf = (char*) pOverlapped->m_pBuf;
			pOverlapped->len = dwPortion;
			pOverlapped->m_pEngine = this;
			pOverlapped->m_hSocket = hSocket;

			if(!PostMessage(m_hSocketWnd, WM_EM_SEND, (WPARAM)ePriority, (LPARAM)pOverlapped))
			{
				AddLog(LOG_ERROR, _T("PostMessage failed"));
				CTask_Tcp_Err::Post(hSocket, -1);
				return false;
			}
		}
		else
			AddLog(LOG_ERROR, _T("No memory"));

		dwSize -= dwPortion;
		((PBYTE&) pData) += dwPortion;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
void CTcpEngine::AllocDisconnect(SOCKET hSocket)
{
	if (!PostMessage(m_hSocketWnd, WM_EM_DISCONNECT, 0, (LPARAM) hSocket))
	{
		AddLog(LOG_ERROR, _T("PostMessage failed"));
		CTask_Tcp_Err::Post(hSocket, -1);
	}
}

//////////////////////////////////////////////////////////////////////
DWORD CTcpEngine::OVERLAPPED_RECV_WEB::ParseRecv(CTcpEngine& stEngine, bool bIsLastRecv)
{
	char* pData = GetBuffer();
	ASSERT(pData);

	if (!m_dwHttpHeaderLen)
	{
		// try to find it
		bool bPrevEndl = false;
		for (DWORD dwPos = 0; dwPos < m_dwBufUsage; dwPos++)
			if ('\n' == pData[dwPos])
				if (bPrevEndl)
				{
					// We just found the end of the http header
					// Now write the message's position into two first DWORDs of the buffer
					m_dwHttpHeaderLen = dwPos + 1;

					// try to find now the 'Content-Length' header
					for (dwPos = 0; dwPos < m_dwHttpHeaderLen; )
					{
						PVOID pPtr = memchr(pData + dwPos, '\n', m_dwHttpHeaderLen - dwPos);
						if (!pPtr)
							break;
						DWORD dwNextPos = ((DWORD) pPtr) - ((DWORD) pData);

						// check this header
						char szMatch[] = "content-length";
						if (!strnicmp(pData + dwPos, szMatch, sizeof(szMatch) - 1))
						{
							dwPos += sizeof(szMatch) - 1;
							pPtr = memchr(pData + dwPos, ':', m_dwHttpHeaderLen - dwPos);
							if (pPtr)
								m_dwHttpContentLen = atol(((char*) pPtr) + 1);

							break;
						}
						dwPos = dwNextPos + 1;
					}

					break;
				}
				else
				{
					bPrevEndl = true;
				}
			else
				if ('\r' != pData[dwPos])
					bPrevEndl = false;
	}

	DWORD dwParsed = 0;
	if (m_dwHttpHeaderLen)
	{
		if (bIsLastRecv && !m_dwHttpContentLen)
			m_dwHttpContentLen = m_dwBufUsage - m_dwHttpHeaderLen;

		if (m_dwHttpHeaderLen + m_dwHttpContentLen <= m_dwBufUsage)
		{
			dwParsed = m_dwHttpHeaderLen + m_dwHttpContentLen;

			CTask_Tcp_Web* pTask = new (dwParsed) CTask_Tcp_Web;
			if (pTask)
			{
				CopyMemory(pTask->m_pBuf, pData, dwParsed);
				pTask->m_hSocket = m_hSocket;
				pTask->m_dwHeaderLen = m_dwHttpHeaderLen;
				pTask->m_dwContentLen = m_dwHttpContentLen;

				g_stEngine.Sockets.Push(pTask);

			} 
			else
				AddLog(LOG_ERROR, _T("No memory"));

			m_dwHttpHeaderLen = m_dwHttpContentLen = 0;
		}
	}
	return dwParsed;
}
//////////////////////////////////////////////////////////////////////
DWORD CTcpEngine::OVERLAPPED_RECV_XML::ParseRecv(CTcpEngine& stEngine, bool bIsLastRecv)
{
	char* pData = GetBuffer();
	ASSERT(pData);

	if (!m_dwLength && m_dwBufUsage >= sizeof(DWORD))
	{
		DWORD *pdwLen = (DWORD*)pData;
		m_dwLength = *pdwLen;
	}

	if (bIsLastRecv && m_dwBufUsage)
		m_dwLength = m_dwBufUsage - sizeof(DWORD);
	DWORD dwParsed = 0;
	if (m_dwLength)
	{
		if (m_dwLength <= (m_dwBufUsage - sizeof(DWORD)))
		{
			dwParsed = m_dwLength + sizeof(DWORD);

			CTask_Tcp_Xml* pTask = new (m_dwLength + 1) CTask_Tcp_Xml;
			if (pTask)
			{
				CopyMemory(pTask->m_pBuf, pData + sizeof(DWORD), m_dwLength);
				pTask->m_pBuf[m_dwLength] = '\0';
				pTask->m_hSocket = m_hSocket;
				pTask->m_dwLength = m_dwLength;

				g_stEngine.Sockets.Push(pTask);

			} 
			else
				AddLog(LOG_ERROR, _T("No memory"));

			m_dwLength = 0;
		}
	}
	return dwParsed;
}
