/*CAsyncSocketEx by Tim Kosse (Tim.Kosse@gmx.de)
            Version 1.3 (2003-04-26)
--------------------------------------------------------

Introduction:
-------------

CAsyncSocketEx is a replacement for the MFC class CAsyncSocket.
This class was written because CAsyncSocket is not the fastest WinSock
wrapper and it's very hard to add new functionality to CAsyncSocket
derived classes. This class offers the same functionality as CAsyncSocket.
Also, CAsyncSocketEx offers some enhancements which were not possible with
CAsyncSocket without some tricks.

How do I use it?
----------------
Basically exactly like CAsyncSocket.
To use CAsyncSocketEx, just replace all occurrences of CAsyncSocket in your
code with CAsyncSocketEx, if you did not enhance CAsyncSocket yourself in
any way, you won't have to change anything else in your code.

Why is CAsyncSocketEx faster?
-----------------------------

CAsyncSocketEx is slightly faster when dispatching notification event messages.
First have a look at the way CAsyncSocket works. For each thread that uses
CAsyncSocket, a window is created. CAsyncSocket calls WSAAsyncSelect with
the handle of that window. Until here, CAsyncSocketEx works the same way.
But CAsyncSocket uses only one window message (WM_SOCKET_NOTIFY) for all
sockets within one thread. When the window receive WM_SOCKET_NOTIFY, wParam
contains the socket handle and the window looks up an CAsyncSocket instance
using a map. CAsyncSocketEx works differently. It's helper window uses a
wide range of different window messages (WM_USER through 0xBFFF) and passes
a different message to WSAAsyncSelect for each socket. When a message in
the specified range is received, CAsyncSocketEx looks up the pointer to a
CAsyncSocketEx instance in an Array using the index of message - WM_USER.
As you can see, CAsyncSocketEx uses the helper window in a more efficient
way, as it don't have to use the slow maps to lookup it's own instance.
Still, speed increase is not very much, but it may be noticeable when using
a lot of sockets at the same time.
Please note that the changes do not affect the raw data throughput rate,
CAsyncSocketEx only dispatches the notification messages faster.

What else does CAsyncSocketEx offer?
------------------------------------

CAsyncSocketEx offers a flexible layer system. One example is the proxy layer.
Just create an instance of the proxy layer, configure it and add it to the layer
chain of your CAsyncSocketEx instance. After that, you can connect through
proxies.
Benefit: You don't have to change much to use the layer system.
Another layer that is currently in development is the SSL layer to establish
SSL encrypted connections.

License
-------

Feel free to use this class, as long as you don't claim that you wrote it
and this copyright notice stays intact in the source files.
If you use this class in commercial applications, please send a short message
to tim.kosse@gmx.de
*/

#include "stdafx.h"
#include "DebugHelpers.h"
#include "AsyncSocketEx.h"

#ifndef NOLAYERS
#include "AsyncSocketExLayer.h"
#endif //NOLAYERS

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
/*
#ifndef CCRITICALSECTIONWRAPPERINCLUDED
class CCriticalSectionWrapper
{
public:
	CCriticalSectionWrapper()
	{
		m_bInitialized = TRUE;
		//InitializeCriticalSection(&m_criticalSection);
		InitializeCriticalSectionAndSpinCount(&m_criticalSection, 4000);
	}

	~CCriticalSectionWrapper()
	{
		if (m_bInitialized){
			DeleteCriticalSection(&m_criticalSection);
			m_bInitialized = FALSE;
		}
	}

	void Lock()
	{
		if (m_bInitialized)
			EnterCriticalSection(&m_criticalSection);
	}
	void Unlock()
	{
		if (m_bInitialized)
			LeaveCriticalSection(&m_criticalSection);
	}
protected:
	CRITICAL_SECTION m_criticalSection;
	BOOL m_bInitialized;
};
#define CCRITICALSECTIONWRAPPERINCLUDED
#endif
*/
Poco::FastMutex CAsyncSocketEx::m_sGlobalCriticalSection;
CAsyncSocketEx::t_AsyncSocketExThreadDataList *CAsyncSocketEx::m_spAsyncSocketExThreadDataList = 0;
HMODULE CAsyncSocketEx::m_hDll = 0;
t_getaddrinfo CAsyncSocketEx::p_getaddrinfo = 0;
t_freeaddrinfo CAsyncSocketEx::p_freeaddrinfo = 0;


#ifndef _AFX
#ifndef VERIFY
#define VERIFY(x) (void(x))
#endif //VERIFY
#ifndef ASSERT
#define ASSERT(x)
#endif //ASSERT
#endif //_AFX

/////////////////////////////
//Helper Window class

class CAsyncSocketExHelperWindow
{
public:
	CAsyncSocketExHelperWindow(CAsyncSocketEx::t_AsyncSocketExThreadData* pThreadData)
	{
		//Initialize data
		m_pAsyncSocketExWindowData = new t_AsyncSocketExWindowData[512]; //Reserve space for 512 active sockets
		memset(m_pAsyncSocketExWindowData, 0, 512*sizeof(t_AsyncSocketExWindowData));
		m_nWindowDataSize = 512;
		m_nSocketCount = 0;
		m_nWindowDataPos = 0;
		m_pThreadData = pThreadData;

		//Create window
		WNDCLASSEX wndclass;
		wndclass.cbSize = sizeof wndclass;
		wndclass.style = 0;
		wndclass.lpfnWndProc = WindowProc;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = GetModuleHandle(0);
		wndclass.hIcon = 0;
		wndclass.hCursor = 0;
		wndclass.hbrBackground = 0;
		wndclass.lpszMenuName = 0;
		wndclass.lpszClassName = _T("CAsyncSocketEx Helper Window");
		wndclass.hIconSm = 0;
		RegisterClassEx(&wndclass);

		/*
		m_hWnd = CreateWindow(_T("CAsyncSocketEx Helper Window"), _T("CAsyncSocketEx Helper Window"), 0, 0, 0, 0, 0, 0, 0, 0, GetModuleHandle(0));
		*/

	//	Starting from Win2000 system supports the message-only window that is not visible,
	//	has no z-order, cannot be enumerated, and does not receive broadcast messages.
	//	Enable this window type for Win2000 & later versions to reduce sockets handling overhead
		//HWND hParent = 0;

		//if ((thePrefs.GetWindowsVersion() & 0x00FF) >= 0x0005)// X: Support Win2000 or Higher Only
			//hParent = HWND_MESSAGE;
		m_hWnd = CreateWindow(_T("CAsyncSocketEx Helper Window"), _T("CAsyncSocketEx Helper Window"), 0, 0, 0, 0, 0, HWND_MESSAGE, 0, 0, GetModuleHandle(0));

		ASSERT( m_hWnd );
		SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR) this);
	};

	virtual ~CAsyncSocketExHelperWindow()
	{
		//Clean up socket storage
		delete[] m_pAsyncSocketExWindowData;
		m_pAsyncSocketExWindowData = 0;
		m_nWindowDataSize = 0;
		m_nSocketCount = 0;

		//Destroy window
		if (m_hWnd)
		{
			DestroyWindow(m_hWnd);
			m_hWnd = 0;
		}
	}

	//Adds a socket to the list of attached sockets
	BOOL AddSocket(CAsyncSocketEx *pSocket, int &nSocketIndex)
	{
		ASSERT(CAsyncSocketEx::IsValidAsyncSocketEx(pSocket)); // netfinity: Ensure socket object really exist
		//ASSERT(pSocket);
		if (!m_nWindowDataSize)
		{
			ASSERT( !m_nSocketCount );
			m_nWindowDataSize = 512;
			m_pAsyncSocketExWindowData = new t_AsyncSocketExWindowData[512]; //Reserve space for 512 active sockets
			memset(m_pAsyncSocketExWindowData, 0, 512 * sizeof(t_AsyncSocketExWindowData));
		}

		if (nSocketIndex != -1)
		{
			ASSERT( m_pAsyncSocketExWindowData );
			ASSERT( m_nWindowDataSize>nSocketIndex );
			ASSERT( m_pAsyncSocketExWindowData[nSocketIndex].m_pSocket == pSocket );
			ASSERT( m_nSocketCount );
			return TRUE;
		}

		//Increase socket storage if too small
		if (m_nSocketCount >= m_nWindowDataSize - 10)
		{
			int nOldWindowDataSize = m_nWindowDataSize;
			ASSERT( m_nWindowDataSize < MAX_SOCKETS );
			m_nWindowDataSize += 512;
			if (m_nWindowDataSize > MAX_SOCKETS)
				m_nWindowDataSize = MAX_SOCKETS;

			t_AsyncSocketExWindowData* tmp = m_pAsyncSocketExWindowData;
			m_pAsyncSocketExWindowData = new t_AsyncSocketExWindowData[m_nWindowDataSize];
			memcpy(m_pAsyncSocketExWindowData, tmp, nOldWindowDataSize * sizeof(t_AsyncSocketExWindowData));
			memset(m_pAsyncSocketExWindowData + nOldWindowDataSize, 0, (m_nWindowDataSize - nOldWindowDataSize) * sizeof(t_AsyncSocketExWindowData));
			delete[] tmp;
		}

		//Search for free slot
		for (int i = m_nWindowDataPos; i < m_nWindowDataSize + m_nWindowDataPos; i++)
		{
			if (!m_pAsyncSocketExWindowData[i%m_nWindowDataSize].m_pSocket)
			{
				m_pAsyncSocketExWindowData[i % m_nWindowDataSize].m_pSocket = pSocket;
				nSocketIndex = i % m_nWindowDataSize;
				m_nWindowDataPos = (i + 1) % m_nWindowDataSize;
				m_nSocketCount++;
				return TRUE;
			}
		}

		//No slot found, maybe there are too much sockets!
		return FALSE;
	}

	//Removes a socket from the socket storage
	BOOL RemoveSocket(CAsyncSocketEx *pSocket, int &nSocketIndex)
	{
		ASSERT(CAsyncSocketEx::IsValidAsyncSocketEx(pSocket)); // netfinity: Ensure socket object really exist
		//ASSERT(pSocket);
		if (nSocketIndex==-1)
			return TRUE;

		// Remove additional messages from queue
		MSG msg;
		while (PeekMessage(&msg, m_hWnd, WM_SOCKETEX_NOTIFY + nSocketIndex, WM_SOCKETEX_NOTIFY + nSocketIndex, PM_REMOVE));

		ASSERT( m_pAsyncSocketExWindowData );
		ASSERT( m_nWindowDataSize > 0 );
		ASSERT( m_nSocketCount > 0 );
		ASSERT( m_pAsyncSocketExWindowData[nSocketIndex].m_pSocket == pSocket );
		m_pAsyncSocketExWindowData[nSocketIndex].m_pSocket = 0;
		nSocketIndex = -1;
		m_nSocketCount--;

		return TRUE;
	}

	void RemoveLayers(CAsyncSocketEx *pOrigSocket)
	{
		// Remove all layer messages from old socket
		std::list<MSG> msgList;
		MSG msg;
		while (PeekMessage(&msg, m_hWnd, WM_SOCKETEX_TRIGGER, WM_SOCKETEX_TRIGGER, PM_REMOVE))
		{
			//Verify parameters, lookup socket and notification message
			//Verify parameters
			if (msg.wParam >= static_cast<UINT>(m_nWindowDataSize)) //Index is within socket storage
				continue;

			CAsyncSocketEx *pSocket = m_pAsyncSocketExWindowData[msg.wParam].m_pSocket;
			CAsyncSocketExLayer::t_LayerNotifyMsg *pMsg=(CAsyncSocketExLayer::t_LayerNotifyMsg *)msg.lParam;
			if (!pMsg || !pSocket || pSocket == pOrigSocket || pSocket->m_SocketData.hSocket != pMsg->hSocket)
			{
				delete pMsg;
				continue;
			}

			msgList.push_back(msg);
		}

		for (std::list<MSG>::const_iterator iter = msgList.begin(); iter != msgList.end(); iter++)
			PostMessage(m_hWnd, iter->message, iter->wParam, iter->lParam);
	}

	//Processes event notifications sent by the sockets or the layers
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (message>=WM_SOCKETEX_NOTIFY)
		{
			//Verify parameters
			ASSERT(hWnd);
			CAsyncSocketExHelperWindow *pWnd=(CAsyncSocketExHelperWindow *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			ASSERT(pWnd);
			
			if (message < static_cast<UINT>(WM_SOCKETEX_NOTIFY+pWnd->m_nWindowDataSize)) //Index is within socket storage
			{
				//Lookup socket and verify if it's valid
				CAsyncSocketEx *pSocket=pWnd->m_pAsyncSocketExWindowData[message-WM_SOCKETEX_NOTIFY].m_pSocket;
				SOCKET hSocket=wParam;
				if (!pSocket)
					return 0;
				if (hSocket==INVALID_SOCKET)
					return 0;
				if (pSocket->m_SocketData.hSocket != hSocket)
					return 0;

				ASSERT(CAsyncSocketEx::IsValidAsyncSocketEx(pSocket)); // netfinity: Ensure socket object really exist
				
				int nEvent=lParam&0xFFFF;
				int nErrorCode=lParam>>16;

				//Dispatch notification
#ifndef NOLAYERS
				if (!pSocket->m_pFirstLayer)
				{
#endif //NOLAYERS
					//Dispatch to CAsyncSocketEx instance
					switch (nEvent)
					{
					case FD_READ:
#ifndef NOSOCKETSTATES
						if(pSocket->GetState() != udpsock){// X: [SUDPS] - [CAsyncSocketEx UDP Support]
							if (pSocket->GetState() == connecting && !nErrorCode)
							{
								pSocket->m_nPendingEvents |= FD_READ;
								break;
							}
							else if (pSocket->GetState() == attached)
								pSocket->SetState(connected);
							if (pSocket->GetState() != connected)
								break;
						}

						// Ignore further FD_READ events after FD_CLOSE has been received
						if (pSocket->m_SocketData.onCloseCalled)
							break;
#endif //NOSOCKETSTATES
						
						if (pSocket->m_lEvent & FD_READ)
						{
							DWORD nBytes = 0;
							if (!nErrorCode)
								if (!pSocket->IOCtl(FIONREAD, &nBytes))
									nErrorCode = WSAGetLastError();
#ifndef NOSOCKETSTATES
							if (nErrorCode)
								pSocket->SetState(aborted);
#endif //NOSOCKETSTATES
							if (nBytes != 0 || nErrorCode != 0)
								pSocket->OnReceive(nErrorCode);
						}
						break;
					case FD_FORCEREAD: //Forceread does not check if there's data waiting
#ifndef NOSOCKETSTATES
						if(pSocket->GetState() != udpsock){// X: [SUDPS] - [CAsyncSocketEx UDP Support]
							if (pSocket->GetState() == connecting && !nErrorCode)
							{
								pSocket->m_nPendingEvents |= FD_FORCEREAD;
								break;
							}
							else if (pSocket->GetState() == attached)
								pSocket->SetState(connected);
							if (pSocket->GetState() != connected)
								break;
						}
#endif //NOSOCKETSTATES
						if (pSocket->m_lEvent & FD_READ)
						{
#ifndef NOSOCKETSTATES
							if (nErrorCode)
								pSocket->SetState(aborted);
#endif //NOSOCKETSTATES
							pSocket->OnReceive(nErrorCode);
						}
						break;
					case FD_WRITE:
#ifndef NOSOCKETSTATES
						if(pSocket->GetState() != udpsock){// X: [SUDPS] - [CAsyncSocketEx UDP Support]
							if (pSocket->GetState() == connecting && !nErrorCode)
							{
								pSocket->m_nPendingEvents |= FD_WRITE;
								break;
							}
							else if (pSocket->GetState() == attached && !nErrorCode)
								pSocket->SetState(connected);
							if (pSocket->GetState() != connected)
								break;
						}
#endif //NOSOCKETSTATES
						if (pSocket->m_lEvent & FD_WRITE)
						{
#ifndef NOSOCKETSTATES
							if (nErrorCode)
								pSocket->SetState(aborted);
#endif //NOSOCKETSTATES
							pSocket->OnSend(nErrorCode);
						}
						break;
					case FD_CONNECT:
#ifndef NOSOCKETSTATES
						if (pSocket->GetState() == connecting)
						{
							if (nErrorCode && pSocket->m_SocketData.nextAddr)
							{
								if (pSocket->TryNextProtocol())
									break;
							}
							pSocket->SetState(connected);
						}
						else if (pSocket->GetState() == attached && !nErrorCode)
							pSocket->SetState(connected);
#endif //NOSOCKETSTATES
						if (pSocket->m_lEvent & FD_CONNECT)
							pSocket->OnConnect(nErrorCode);

#ifndef NOSOCKETSTATES
						// netfinity: Check that socket is still valid. It might have got deleted.
						if (!pWnd->m_pAsyncSocketExWindowData || pSocket != pWnd->m_pAsyncSocketExWindowData[message-WM_SOCKETEX_NOTIFY].m_pSocket)
							break;
						if (!nErrorCode)
						{
							if ((pSocket->m_nPendingEvents&FD_READ) && pSocket->GetState() == connected)
								pSocket->OnReceive(0);
							if ((pSocket->m_nPendingEvents&FD_FORCEREAD) && pSocket->GetState() == connected)
								pSocket->OnReceive(0);
							if ((pSocket->m_nPendingEvents&FD_WRITE) && pSocket->GetState() == connected)
								pSocket->OnSend(0);
						}
						pSocket->m_nPendingEvents = 0;
#endif
						break;
					case FD_ACCEPT:
#ifndef NOSOCKETSTATES
						if (pSocket->GetState() != listening && pSocket->GetState() != attached)
							break;
#endif //NOSOCKETSTATES
						if (pSocket->m_lEvent & FD_ACCEPT)
							pSocket->OnAccept(nErrorCode);
						break;
					case FD_CLOSE:
#ifndef NOSOCKETSTATES
						if (pSocket->GetState() != connected && pSocket->GetState() != attached)
							break;

						// If there are still bytes left to read, call OnReceive instead of 
						// OnClose and trigger a new OnClose
						DWORD nBytes = 0;
						if (!nErrorCode && pSocket->IOCtl(FIONREAD, &nBytes))
						{
							if (nBytes > 0)
							{
								pSocket->m_SocketData.onCloseCalled = true;								
								pSocket->OnReceive(WSAESHUTDOWN);
								// netfinity: OnReceive may fail and then we could get into a endless loop
								DWORD nBytesRemaining = 0;
								if (pSocket->IOCtl(FIONREAD, &nBytesRemaining) && nBytesRemaining < nBytes)
								{
									// Just repeat message.
									PostMessage(hWnd, message, wParam, lParam);
									break;
								}
							}
						}

						pSocket->SetState(nErrorCode?aborted:closed);
#endif //NOSOCKETSTATES
						pSocket->OnClose(nErrorCode);
						break;
					}
				}
#ifndef NOLAYERS
				else //Dispatch notification to the lowest layer
				{
					if (nEvent == FD_READ)
					{
						// Ignore further FD_READ events after FD_CLOSE has been received
						if (pSocket->m_SocketData.onCloseCalled)
							return 0;

						DWORD nBytes;
						if (!pSocket->IOCtl(FIONREAD, &nBytes))
							nErrorCode = WSAGetLastError();
						if (nBytes != 0 || nErrorCode != 0)
							pSocket->m_pLastLayer->CallEvent(nEvent, nErrorCode);
					}
					else if (nEvent == FD_CLOSE)
					{
						// If there are still bytes left to read, call OnReceive instead of 
						// OnClose and trigger a new OnClose
						DWORD nBytes = 0;
						if (!nErrorCode && pSocket->IOCtl(FIONREAD, &nBytes))
						{
							if (nBytes > 0)
							{
								// Just repeat message.
								pSocket->ResendCloseNotify();
								pSocket->m_pLastLayer->CallEvent(FD_READ, 0);
								return 0;
							}
						}
						pSocket->m_SocketData.onCloseCalled = true;
						pSocket->m_pLastLayer->CallEvent(nEvent, nErrorCode);
					}
					else
						pSocket->m_pLastLayer->CallEvent(nEvent, nErrorCode);
				}
			}
#endif //NOLAYERS
			return 0;
		}
#ifndef NOLAYERS
		else if (message == WM_SOCKETEX_TRIGGER) //Notification event sent by a layer
		{
			//Verify parameters, lookup socket and notification message
			//Verify parameters
			ASSERT(hWnd);
			CAsyncSocketExHelperWindow *pWnd=(CAsyncSocketExHelperWindow *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			ASSERT(pWnd);
			
			if (wParam >= static_cast<UINT>(pWnd->m_nWindowDataSize)) //Index is within socket storage
			{
				return 0;
			}
			
			CAsyncSocketEx *pSocket=pWnd->m_pAsyncSocketExWindowData[wParam].m_pSocket;
			CAsyncSocketExLayer::t_LayerNotifyMsg *pMsg=(CAsyncSocketExLayer::t_LayerNotifyMsg *)lParam;
			if (!pMsg || !pSocket || pSocket->m_SocketData.hSocket != pMsg->hSocket)
			{
				delete pMsg;
				return 0;
			}
			int nEvent=pMsg->lEvent&0xFFFF;
			int nErrorCode=pMsg->lEvent>>16;
			
			ASSERT(CAsyncSocketEx::IsValidAsyncSocketEx(pSocket)); // netfinity: Ensure socket object really exist

			//Dispatch to layer
			if (pMsg->pLayer)
				pMsg->pLayer->CallEvent(nEvent, nErrorCode);
			else
			{
				//Dispatch to CAsyncSocketEx instance
				switch (nEvent)
				{
				case FD_READ:
#ifndef NOSOCKETSTATES
					if(pSocket->GetState() != udpsock){// X: [SUDPS] - [CAsyncSocketEx UDP Support]
						if (pSocket->GetState() == connecting && !nErrorCode)
						{
							pSocket->m_nPendingEvents |= FD_READ;
							break;
						}
						else if (pSocket->GetState() == attached && !nErrorCode)
							pSocket->SetState(connected);
						if (pSocket->GetState() != connected)
							break;
					}
#endif //NOSOCKETSTATES
					if (pSocket->m_lEvent & FD_READ)
					{
#ifndef NOSOCKETSTATES
						if (nErrorCode)
							pSocket->SetState(aborted);
#endif //NOSOCKETSTATES
						pSocket->OnReceive(nErrorCode);
					}
					break;
				case FD_FORCEREAD: //Forceread does not check if there's data waiting
#ifndef NOSOCKETSTATES
					if(pSocket->GetState() != udpsock){// X: [SUDPS] - [CAsyncSocketEx UDP Support]
						if (pSocket->GetState() == connecting && !nErrorCode)
						{
							pSocket->m_nPendingEvents |= FD_FORCEREAD;
							break;
						}
						else if (pSocket->GetState() == attached && !nErrorCode)
							pSocket->SetState(connected);
						if (pSocket->GetState() != connected)
							break;
					}
#endif //NOSOCKETSTATES
					if (pSocket->m_lEvent & FD_READ)
					{
#ifndef NOSOCKETSTATES
						if (nErrorCode)
							pSocket->SetState(aborted);
#endif //NOSOCKETSTATES
						pSocket->OnReceive(nErrorCode);
					}
					break;
				case FD_WRITE:
#ifndef NOSOCKETSTATES
					if(pSocket->GetState() != udpsock){// X: [SUDPS] - [CAsyncSocketEx UDP Support]
						if (pSocket->GetState() == connecting && !nErrorCode)
						{
							pSocket->m_nPendingEvents |= FD_WRITE;
							break;
						}
						else if (pSocket->GetState() == attached && !nErrorCode)
							pSocket->SetState(connected);
						if (pSocket->GetState() != connected)
							break;
					}
#endif //NOSOCKETSTATES
					if (pSocket->m_lEvent & FD_WRITE)
					{
#ifndef NOSOCKETSTATES
						if (nErrorCode)
							pSocket->SetState(aborted);
#endif //NOSOCKETSTATES
						pSocket->OnSend(nErrorCode);
					}
					break;
				case FD_CONNECT:
#ifndef NOSOCKETSTATES
					if (pSocket->GetState() == connecting)
						pSocket->SetState(connected);
					else if (pSocket->GetState() == attached && !nErrorCode)
						pSocket->SetState(connected);
#endif //NOSOCKETSTATES
					if (pSocket->m_lEvent & FD_CONNECT)
						pSocket->OnConnect(nErrorCode);

#ifndef NOSOCKETSTATES
					// netfinity: Check that socket is still valid. It might have got deleted.
					if (!pWnd->m_pAsyncSocketExWindowData || pSocket != pWnd->m_pAsyncSocketExWindowData[message-WM_SOCKETEX_NOTIFY].m_pSocket)
						break;
					if (!nErrorCode)
					{
						if (((pSocket->m_nPendingEvents&FD_READ) && pSocket->GetState() == connected) && (pSocket->m_lEvent & FD_READ))
							pSocket->OnReceive(0);
						if (((pSocket->m_nPendingEvents&FD_FORCEREAD) && pSocket->GetState() == connected) && (pSocket->m_lEvent & FD_READ))
							pSocket->OnReceive(0);
						if (((pSocket->m_nPendingEvents&FD_WRITE) && pSocket->GetState() == connected) && (pSocket->m_lEvent & FD_WRITE))
							pSocket->OnSend(0);
					}
					pSocket->m_nPendingEvents = 0;
#endif //NOSOCKETSTATES
					break;
				case FD_ACCEPT:
#ifndef NOSOCKETSTATES
					if ((pSocket->GetState() == listening || pSocket->GetState() == attached) && (pSocket->m_lEvent & FD_ACCEPT))
#endif //NOSOCKETSTATES
					{
						pSocket->OnAccept(nErrorCode);
					}
					break;
				case FD_CLOSE:
#ifndef NOSOCKETSTATES
					if ((pSocket->GetState() == connected || pSocket->GetState() == attached) && (pSocket->m_lEvent & FD_CLOSE))
					{
						pSocket->SetState(nErrorCode?aborted:closed);
#else
					{
#endif //NOSOCKETSTATES
						pSocket->OnClose(nErrorCode);
					}
					break;
				}
			}
			delete pMsg;
			return 0;
		}
#endif //NOLAYERS
		else if (message == WM_SOCKETEX_GETHOST)
		{
			// WSAAsyncGetHostByName reply

			// Verify parameters
			ASSERT(hWnd);
			CAsyncSocketExHelperWindow *pWnd = (CAsyncSocketExHelperWindow *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			ASSERT(pWnd);

			CAsyncSocketEx *pSocket = NULL;
			for (int i = 0; i < pWnd->m_nWindowDataSize; i++)
			{
				CAsyncSocketEx *pTempSocket = pWnd->m_pAsyncSocketExWindowData[i].m_pSocket;
				//ASSERT(CAsyncSocketEx::IsValidAsyncSocketEx(pTempSocket)); // netfinity: Ensure socket object really exist

				if (pTempSocket && pTempSocket->m_hAsyncGetHostByNameHandle &&
					pTempSocket->m_hAsyncGetHostByNameHandle == (HANDLE)wParam)
				{
					pSocket = pTempSocket;
					break;
				}
			}
			if (!pSocket)
				return 0;

			int nErrorCode = lParam >> 16;
			if (nErrorCode)
			{
				pSocket->OnConnect(nErrorCode);
				return 0;
			}

			SOCKADDR_IN sockAddr;
			memset(&sockAddr,0,sizeof(sockAddr));
			sockAddr.sin_family=AF_INET;
			sockAddr.sin_addr.s_addr = ((LPIN_ADDR)((LPHOSTENT)pSocket->m_pAsyncGetHostByNameBuffer)->h_addr)->s_addr;

			sockAddr.sin_port = htons(pSocket->m_nAsyncGetHostByNamePort);

			BOOL res = pSocket->Connect((SOCKADDR*)&sockAddr, sizeof(sockAddr));
			delete [] pSocket->m_pAsyncGetHostByNameBuffer;
			pSocket->m_pAsyncGetHostByNameBuffer=0;
			pSocket->m_hAsyncGetHostByNameHandle=0;

			if (!res && GetLastError() != WSAEWOULDBLOCK)
					pSocket->OnConnect(GetLastError());
			return 0;
		}
		else if (message == WM_SOCKETEX_CALLBACK)
		{
			//Verify parameters, lookup socket and notification message
			//Verify parameters
			if (!hWnd)
				return 0;

			CAsyncSocketExHelperWindow *pWnd=(CAsyncSocketExHelperWindow *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			if (!pWnd)
				return 0;

			if (wParam >= static_cast<UINT>(pWnd->m_nWindowDataSize)) //Index is within socket storage
				return 0;
			
			CAsyncSocketEx *pSocket = pWnd->m_pAsyncSocketExWindowData[wParam].m_pSocket;
			if (!pSocket)
				return 0;

			ASSERT(CAsyncSocketEx::IsValidAsyncSocketEx(pSocket)); // netfinity: Ensure socket object really exist

			// Process pending callbacks
			std::list<t_callbackMsg> tmp;
			tmp.swap(pSocket->m_pendingCallbacks);
			pSocket->OnLayerCallback(tmp);
		}
		else if (message == WM_TIMER)
		{
			if (wParam != 1)
				return 0;
			
			ASSERT(hWnd);
			CAsyncSocketExHelperWindow *pWnd=(CAsyncSocketExHelperWindow *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			ASSERT(pWnd);

			if (pWnd->m_pThreadData->layerCloseNotify.empty())
			{
				KillTimer(hWnd, 1);
				return 0;
			}
			CAsyncSocketEx* socket = pWnd->m_pThreadData->layerCloseNotify.front();

			ASSERT(CAsyncSocketEx::IsValidAsyncSocketEx(socket)); // netfinity: Ensure socket object really exist

			pWnd->m_pThreadData->layerCloseNotify.pop_front();
			if (pWnd->m_pThreadData->layerCloseNotify.empty())
				KillTimer(hWnd, 1);

			PostMessage(hWnd, socket->m_SocketData.nSocketIndex + WM_SOCKETEX_NOTIFY, socket->m_SocketData.hSocket, FD_CLOSE);
			return 0;
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	HWND CAsyncSocketExHelperWindow::GetHwnd()
	{
		return m_hWnd;
	}

private:
	HWND m_hWnd;
	struct t_AsyncSocketExWindowData
	{
		CAsyncSocketEx *m_pSocket;
	} *m_pAsyncSocketExWindowData;
	int m_nWindowDataSize;
	int m_nWindowDataPos;
	int m_nSocketCount;
	CAsyncSocketEx::t_AsyncSocketExThreadData* m_pThreadData;
};

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

//IMPLEMENT_DYNAMIC(CAsyncSocketEx, CObject)

CAsyncSocketEx::CAsyncSocketEx()
{
#ifdef _DEBUG
	m_uMagic = ASYNCSOCKETEXMAGIC;	// netfinity: Magic value indicating this socket is for real
#endif

	m_SocketData.hSocket = INVALID_SOCKET;
	m_SocketData.nSocketIndex = -1;
	m_SocketData.nFamily = AF_UNSPEC;
	m_SocketData.onCloseCalled = false;
	m_pLocalAsyncSocketExThreadData = 0;

#ifndef NOSOCKETSTATES
	m_nPendingEvents = 0;
	m_nState = notsock;
#endif //NOSOCKETSTATES

#ifndef NOLAYERS
	m_pFirstLayer = 0;
	m_pLastLayer = 0;
#endif //NOLAYERS
	m_pAsyncGetHostByNameBuffer = NULL;
	m_hAsyncGetHostByNameHandle = NULL;

	m_nSocketPort = 0;
	m_lpszSocketAddress = 0;

	m_SocketData.addrInfo = 0;
	m_SocketData.nextAddr = 0;
}

CAsyncSocketEx::~CAsyncSocketEx()
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	Close();
	FreeAsyncSocketExInstance();

#ifdef _DEBUG
	m_uMagic = 0;	// netfinity: Socket doesn't exist anymore so just in case anyone tries to use the pointer
#endif
}

BOOL CAsyncSocketEx::Create(UINT nSocketPort /*=0*/, int nSocketType /*=SOCK_STREAM*/, long lEvent /*=FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE*/, LPCTSTR lpszSocketAddress /*=NULL*/, int nFamily /*=AF_INET*/, bool reusable /*=false*/)
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	ASSERT(GetSocketHandle() == INVALID_SOCKET);

	//Close the socket, although this should not happen
	if (GetSocketHandle() != INVALID_SOCKET)
	{
		WSASetLastError(WSAEALREADY);
		return FALSE;
	}

	if (!InitAsyncSocketExInstance()) {
		ASSERT(0);
		WSASetLastError(WSANOTINITIALISED);
		return FALSE;
	}

	m_SocketData.nFamily = nFamily;

#ifndef NOLAYERS
	if (m_pFirstLayer)
	{
		BOOL res = m_pFirstLayer->Create(nSocketPort, nSocketType, lEvent, lpszSocketAddress, nFamily, reusable);
#ifndef NOSOCKETSTATES
		if (res)
			SetState(nSocketType == SOCK_STREAM ? unconnected : udpsock);// X: [SUDPS] - [CAsyncSocketEx UDP Support]
#endif //NOSOCKETSTATES
		return res;
	}
	else
#endif //NOLAYERS
	{
		if (m_SocketData.nFamily == AF_UNSPEC)
		{
#ifndef NOSOCKETSTATES
			SetState(nSocketType == SOCK_STREAM ? unconnected : udpsock);// X: [SUDPS] - [CAsyncSocketEx UDP Support]
#endif //NOSOCKETSTATES
			m_lEvent = lEvent;

			m_nSocketPort = nSocketPort;
		
			delete [] m_lpszSocketAddress;
			if (lpszSocketAddress && *lpszSocketAddress)
			{
				size_t nLen1 = _tcslen(lpszSocketAddress) + 1;
				m_lpszSocketAddress = new TCHAR[nLen1];
				_tcscpy_s(m_lpszSocketAddress, nLen1, lpszSocketAddress);
			}
			else
				m_lpszSocketAddress = 0;

			return TRUE;
		}
		else
		{
			SOCKET hSocket = socket(m_SocketData.nFamily, nSocketType, 0);
			if (hSocket == INVALID_SOCKET)
				return FALSE;
			m_SocketData.hSocket = hSocket;
			AttachHandle(hSocket);
		
#ifndef NOLAYERS
			if (m_pFirstLayer)
			{
				m_lEvent = lEvent;
				if (WSAAsyncSelect(m_SocketData.hSocket, GetHelperWindowHandle(), m_SocketData.nSocketIndex+WM_SOCKETEX_NOTIFY, FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE) )
				{
					Close();
					return FALSE;
				}
			}
			else
#endif //NOLAYERS
			{
				if (!AsyncSelect(lEvent))
				{
					Close();
					return FALSE;
				}
			}

			if (reusable && nSocketPort != 0)
			{
				BOOL value = TRUE;
				SetSockOpt(SO_REUSEADDR, reinterpret_cast<const void*>(&value), sizeof(value));
			}

			if (!Bind(nSocketPort, lpszSocketAddress))
			{
				Close();
				return FALSE;
			}

#ifndef NOSOCKETSTATES
			SetState(nSocketType == SOCK_STREAM ? unconnected : udpsock);// X: [SUDPS] - [CAsyncSocketEx UDP Support]
#endif //NOSOCKETSTATES

			return TRUE;
		}
	}
}

void CAsyncSocketEx::OnReceive(int /*nErrorCode*/)
{
}

void CAsyncSocketEx::OnSend(int /*nErrorCode*/)
{
}

void CAsyncSocketEx::OnConnect(int /*nErrorCode*/)
{
}

void CAsyncSocketEx::OnAccept(int /*nErrorCode*/)
{
}

void CAsyncSocketEx::OnClose(int /*nErrorCode*/)
{
}

BOOL CAsyncSocketEx::Bind(UINT nSocketPort, LPCTSTR lpszSocketAddress)
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	if(m_lpszSocketAddress != lpszSocketAddress){
		delete [] m_lpszSocketAddress;
		if (lpszSocketAddress && *lpszSocketAddress)
		{
			size_t nLen1 = _tcslen(lpszSocketAddress) + 1;
			m_lpszSocketAddress = new TCHAR[nLen1];
			_tcscpy_s(m_lpszSocketAddress, nLen1, lpszSocketAddress);
		}
		else
			m_lpszSocketAddress = 0;
	}
	m_nSocketPort = nSocketPort;

	if (m_SocketData.nFamily == AF_UNSPEC)
		return TRUE;
	
	//LPSTR lpszAscii = (lpszSocketAddress && *lpszSocketAddress) ? CT2A((LPTSTR)lpszSocketAddress) : 0; // X: [BF] - [Bug Fix] not work
	LPSTR lpszAscii = 0;
	CT2A tmpt2a(lpszSocketAddress);
	lpszAscii =  tmpt2a;
	
	if ((
#ifdef ENABLE_IPV6
		m_SocketData.nFamily == AF_INET6 || 
#endif
		m_SocketData.nFamily == AF_INET) && lpszAscii)
	{
		if (!p_getaddrinfo)
		{
			if (m_SocketData.nFamily != AF_INET)
			{
				WSASetLastError(WSAEPROTONOSUPPORT);
				return FALSE;
			}
			else
			{
				unsigned long ip = inet_addr(lpszAscii);
				if (!ip)
				{
					WSASetLastError(WSAEINVAL);
					return FALSE;
				}

				SOCKADDR_IN sockAddr;
				memset(&sockAddr, 0, sizeof(sockAddr));
				sockAddr.sin_family = m_SocketData.nFamily;
				sockAddr.sin_addr.s_addr = ip;
				sockAddr.sin_port = htons((u_short)nSocketPort);
				return Bind((SOCKADDR*)&sockAddr, sizeof(sockAddr));
			}
		}
		addrinfo hints, *res0, *res;
		int error;
		char port[10];
		BOOL ret = FALSE;

		memset(&hints, 0, sizeof(addrinfo));
		hints.ai_family = m_SocketData.nFamily;
		hints.ai_socktype = SOCK_STREAM;
		_snprintf_s(port, 9, "%lu", nSocketPort);
		error = p_getaddrinfo(lpszAscii, port, &hints, &res0);
		if (error)
			return FALSE;

		for (res = res0; res; res = res->ai_next)
			if (Bind(res->ai_addr, (int)res->ai_addrlen))
			{
				ret = TRUE;
				break;
			}
			else
				continue ;

			p_freeaddrinfo(res0);

			return ret ;
	}
#ifdef ENABLE_IPV6
	else if (!lpszAscii && m_SocketData.nFamily == AF_INET6)
	{
		SOCKADDR_IN6 sockAddr6;

		memset(&sockAddr6, 0, sizeof(sockAddr6));
		sockAddr6.sin6_family = AF_INET6 ;
		sockAddr6.sin6_addr = in6addr_any ;
		sockAddr6.sin6_port = htons((u_short)nSocketPort);

		return Bind((SOCKADDR*)&sockAddr6, sizeof(sockAddr6));
	}
#endif
	else if (!lpszAscii && m_SocketData.nFamily == AF_INET)
	{
		SOCKADDR_IN sockAddr;

		memset(&sockAddr, 0, sizeof(sockAddr));
		sockAddr.sin_family = AF_INET ;
		sockAddr.sin_addr.s_addr = INADDR_ANY ;
		sockAddr.sin_port = htons((u_short)nSocketPort);

		return Bind((SOCKADDR*)&sockAddr, sizeof(sockAddr));
	}
	else
		return FALSE ;
}

BOOL CAsyncSocketEx::Bind(const SOCKADDR* lpSockAddr, int nSockAddrLen)
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	if (!::bind(m_SocketData.hSocket, lpSockAddr, nSockAddrLen))
		return TRUE;
	else
		return FALSE;
}

void CAsyncSocketEx::AttachHandle(SOCKET /*hSocket*/)
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	ASSERT( m_pLocalAsyncSocketExThreadData );
	VERIFY( m_pLocalAsyncSocketExThreadData->m_pHelperWindow->AddSocket(this, m_SocketData.nSocketIndex) );
#ifndef NOSOCKETSTATES
	SetState(attached);
#endif //NOSOCKETSTATES
}

void CAsyncSocketEx::DetachHandle(SOCKET /*hSocket*/)
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	ASSERT( m_pLocalAsyncSocketExThreadData );
	if (!m_pLocalAsyncSocketExThreadData)
		return;
	
	ASSERT( m_pLocalAsyncSocketExThreadData->m_pHelperWindow );
	if (!m_pLocalAsyncSocketExThreadData->m_pHelperWindow)
		return;
	
	VERIFY( m_pLocalAsyncSocketExThreadData->m_pHelperWindow->RemoveSocket(this, m_SocketData.nSocketIndex) );
#ifndef NOSOCKETSTATES
	SetState(notsock);
#endif //NOSOCKETSTATES
}

void CAsyncSocketEx::Close()
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

#ifndef NOSOCKETSTATES
	m_nPendingEvents = 0;
#endif //NOSOCKETSTATES
#ifndef NOLAYERS
	if (m_pFirstLayer)
		m_pFirstLayer->Close();
#endif //NOLAYERS
	if (m_SocketData.hSocket != INVALID_SOCKET)
	{
		VERIFY(closesocket(m_SocketData.hSocket) != SOCKET_ERROR);
		DetachHandle(m_SocketData.hSocket);
		m_SocketData.hSocket = INVALID_SOCKET;
	}
	if (m_SocketData.addrInfo)
	{
		p_freeaddrinfo(m_SocketData.addrInfo);
		m_SocketData.addrInfo = 0;
		m_SocketData.nextAddr = 0;
	}
	m_SocketData.nFamily = AF_UNSPEC;
	delete [] m_lpszSocketAddress;
	m_lpszSocketAddress = 0;
	m_nSocketPort = 0;
#ifndef NOLAYERS
	RemoveAllLayers();
#endif //NOLAYERS
	delete [] m_pAsyncGetHostByNameBuffer;
	m_pAsyncGetHostByNameBuffer = NULL;
	if (m_hAsyncGetHostByNameHandle) {
		WSACancelAsyncRequest(m_hAsyncGetHostByNameHandle);
	m_hAsyncGetHostByNameHandle = NULL;
	}
	m_SocketData.onCloseCalled = false;
}

BOOL CAsyncSocketEx::InitAsyncSocketExInstance()
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	//Check if already initialized
	if (m_pLocalAsyncSocketExThreadData)
		return TRUE;

	DWORD id = GetCurrentThreadId();

	m_sGlobalCriticalSection.Lock();

	//Get thread specific data
	if (m_spAsyncSocketExThreadDataList)
	{
		t_AsyncSocketExThreadDataList *pList = m_spAsyncSocketExThreadDataList;
		while (pList)
		{
			ASSERT( pList->pThreadData );
			ASSERT( pList->pThreadData->nInstanceCount > 0 );
	
			if (pList->pThreadData->nThreadId == id)
			{
				m_pLocalAsyncSocketExThreadData = pList->pThreadData;
				m_pLocalAsyncSocketExThreadData->nInstanceCount++;
				break;
			}
			pList = pList->pNext;
		}
		//Current thread yet has no sockets
		if (!pList)
		{
			//Initialize data for current thread
			pList = new t_AsyncSocketExThreadDataList;
			pList->pNext = m_spAsyncSocketExThreadDataList;
			m_spAsyncSocketExThreadDataList = pList;
			m_pLocalAsyncSocketExThreadData = new t_AsyncSocketExThreadData;
			m_pLocalAsyncSocketExThreadData->nInstanceCount = 1;
			m_pLocalAsyncSocketExThreadData->nThreadId = id;
			m_pLocalAsyncSocketExThreadData->m_pHelperWindow=new CAsyncSocketExHelperWindow(m_pLocalAsyncSocketExThreadData);
			m_spAsyncSocketExThreadDataList->pThreadData = m_pLocalAsyncSocketExThreadData;
		}
	}
	else
	{	//No thread has instances of CAsyncSocketEx; Initialize data
		m_spAsyncSocketExThreadDataList = new t_AsyncSocketExThreadDataList;
		m_spAsyncSocketExThreadDataList->pNext = 0;
		m_pLocalAsyncSocketExThreadData = new t_AsyncSocketExThreadData;
		m_pLocalAsyncSocketExThreadData->nInstanceCount = 1;
		m_pLocalAsyncSocketExThreadData->nThreadId = id;
		m_pLocalAsyncSocketExThreadData->m_pHelperWindow=new CAsyncSocketExHelperWindow(m_pLocalAsyncSocketExThreadData);
		m_spAsyncSocketExThreadDataList->pThreadData = m_pLocalAsyncSocketExThreadData;

		m_hDll = LoadLibrary(_T("WS2_32.dll"));
		if (m_hDll)
		{
			p_getaddrinfo = (t_getaddrinfo)GetProcAddress(m_hDll, "getaddrinfo");
			p_freeaddrinfo = (t_freeaddrinfo)GetProcAddress(m_hDll, "freeaddrinfo");

			if (!p_getaddrinfo || !p_freeaddrinfo)
			{
				p_getaddrinfo = 0;
				p_freeaddrinfo = 0;
				FreeLibrary(m_hDll);
				m_hDll = 0;
			}
		}
	}
	m_sGlobalCriticalSection.Unlock();
	return TRUE;
}

void CAsyncSocketEx::FreeAsyncSocketExInstance()
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	//Check if already freed
	if (!m_pLocalAsyncSocketExThreadData)
		return;

	for (std::list<CAsyncSocketEx*>::iterator iter = m_pLocalAsyncSocketExThreadData->layerCloseNotify.begin(); iter != m_pLocalAsyncSocketExThreadData->layerCloseNotify.end(); iter++)
	{
		if (*iter != this)
			continue;

		m_pLocalAsyncSocketExThreadData->layerCloseNotify.erase(iter);
		if (m_pLocalAsyncSocketExThreadData->layerCloseNotify.empty())
			KillTimer(m_pLocalAsyncSocketExThreadData->m_pHelperWindow->GetHwnd(), 1);
		break;
	}

	DWORD id = m_pLocalAsyncSocketExThreadData->nThreadId;
	m_sGlobalCriticalSection.Lock();

	ASSERT( m_spAsyncSocketExThreadDataList );
	t_AsyncSocketExThreadDataList *pList = m_spAsyncSocketExThreadDataList;
	t_AsyncSocketExThreadDataList *pPrev = 0;
	
	//Search for data for current thread and decrease instance count
	while (pList)
	{
		ASSERT( pList->pThreadData );
		ASSERT( pList->pThreadData->nInstanceCount > 0 );

		if (pList->pThreadData->nThreadId == id)
		{
			ASSERT( m_pLocalAsyncSocketExThreadData == pList->pThreadData );
			m_pLocalAsyncSocketExThreadData->nInstanceCount--;
	
			//Freeing last instance?
			//If so, destroy helper window
			if (!m_pLocalAsyncSocketExThreadData->nInstanceCount)
			{
				delete m_pLocalAsyncSocketExThreadData->m_pHelperWindow;
				delete m_pLocalAsyncSocketExThreadData;
				if (pPrev)
					pPrev->pNext = pList->pNext;
				else
					m_spAsyncSocketExThreadDataList = pList->pNext;
				delete pList;

				// Last thread closed, free dll
				if (!m_spAsyncSocketExThreadDataList)
				{
					if (m_hDll)
					{
						p_getaddrinfo = 0;
						p_freeaddrinfo = 0;
						FreeLibrary(m_hDll);
						m_hDll = 0;
					}
				}
				break;
			}
			break;
		}
		pPrev = pList;
		pList = pList->pNext;
		ASSERT( pList );
	}

	m_sGlobalCriticalSection.Unlock();
}

int CAsyncSocketEx::Receive(void* lpBuf, int nBufLen, int nFlags /*=0*/)
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

#ifndef NOLAYERS
	if (m_pFirstLayer)
		return m_pFirstLayer->Receive(lpBuf, nBufLen, nFlags);
	else
#endif //NOLAYERS
		return recv(m_SocketData.hSocket, (LPSTR)lpBuf, nBufLen, nFlags);
}
// X: [SUDPS] - [CAsyncSocketEx UDP Support]
int CAsyncSocketEx::ReceiveFrom(void* lpBuf, int nBufLen, SOCKADDR* lpSockAddr, int* nSockAddrLen, int nFlags /*=0*/)
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

#ifndef NOLAYERS
	if (m_pFirstLayer)
		return m_pFirstLayer->ReceiveFrom(lpBuf, nBufLen, lpSockAddr, nSockAddrLen, nFlags);
	else
#endif //NOLAYERS
		return recvfrom(m_SocketData.hSocket, (LPSTR)lpBuf, nBufLen, nFlags, lpSockAddr, nSockAddrLen);
}

int CAsyncSocketEx::Send(const void* lpBuf, int nBufLen, int nFlags /*=0*/)
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

#ifndef NOLAYERS
	if (m_pFirstLayer)
		return m_pFirstLayer->Send(lpBuf, nBufLen, nFlags);
	else
#endif //NOLAYERS
		return send(m_SocketData.hSocket, (LPSTR)lpBuf, nBufLen, nFlags);
}
// X: [SUDPS] - [CAsyncSocketEx UDP Support]
int CAsyncSocketEx::SendTo(const void* lpBuf, int nBufLen, const SOCKADDR* lpSockAddr, int nSockAddrLen, int nFlags /*=0*/)
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

#ifndef NOLAYERS
	if (m_pFirstLayer)
		return m_pFirstLayer->SendTo(lpBuf, nBufLen, lpSockAddr, nSockAddrLen, nFlags);
	else
#endif //NOLAYERS
		return sendto(m_SocketData.hSocket, (LPSTR)lpBuf, nBufLen, nFlags, lpSockAddr, nSockAddrLen);
}

BOOL CAsyncSocketEx::Connect(LPCTSTR lpszHostAddress, UINT nHostPort)
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

#ifndef NOLAYERS
	if (m_pFirstLayer)
	{
		BOOL res = m_pFirstLayer->Connect(lpszHostAddress, nHostPort);
#ifndef NOSOCKETSTATES
		if (res || GetLastError()==WSAEWOULDBLOCK)
			SetState(connecting);
#endif //NOSOCKETSTATES
		return res;
	} else
#endif //NOLAYERS
	if (m_SocketData.nFamily == AF_INET)
	{

		ASSERT(lpszHostAddress != NULL);

		SOCKADDR_IN sockAddr;
		memset(&sockAddr,0,sizeof(sockAddr));

		//LPSTR lpszAscii = CT2A((LPTSTR)lpszHostAddress); // X: [BF] - [Bug Fix] not work
		CT2A tmpt2a(lpszHostAddress);
		LPSTR lpszAscii = tmpt2a;
		sockAddr.sin_family = AF_INET;
		sockAddr.sin_addr.s_addr = inet_addr(lpszAscii);

		if (sockAddr.sin_addr.s_addr == INADDR_NONE)
		{
			if (m_pAsyncGetHostByNameBuffer)
				delete [] m_pAsyncGetHostByNameBuffer;
			m_pAsyncGetHostByNameBuffer=new char[MAXGETHOSTSTRUCT];

			m_nAsyncGetHostByNamePort=nHostPort;

			m_hAsyncGetHostByNameHandle=WSAAsyncGetHostByName(GetHelperWindowHandle(), WM_SOCKETEX_GETHOST, lpszAscii, m_pAsyncGetHostByNameBuffer, MAXGETHOSTSTRUCT);
			if (!m_hAsyncGetHostByNameHandle)
				return FALSE;

			WSASetLastError(WSAEWOULDBLOCK);
#ifndef NOSOCKETSTATES
			SetState(connecting);
#endif //NOSOCKETSTATES
			return FALSE;
		}

		sockAddr.sin_port = htons((u_short)nHostPort);
		return CAsyncSocketEx::Connect((SOCKADDR*)&sockAddr, sizeof(sockAddr));
	}
	else
	{
		if (!p_getaddrinfo)
		{
			WSASetLastError(WSAEPROTONOSUPPORT);
			return FALSE;
		}


		ASSERT( lpszHostAddress != NULL );

		if (m_SocketData.addrInfo)
		{
			p_freeaddrinfo(m_SocketData.addrInfo);
			m_SocketData.addrInfo = 0;
			m_SocketData.nextAddr = 0;
		}

		addrinfo hints;
		int error;
		BOOL ret;
		char port[10];

		memset(&hints, 0, sizeof(addrinfo));
		hints.ai_family = m_SocketData.nFamily;
		hints.ai_socktype = SOCK_STREAM;
		_snprintf_s(port, 9, "%lu", nHostPort);
		error = p_getaddrinfo(CT2CA(lpszHostAddress), port, &hints, &m_SocketData.addrInfo);
		if (error)
			return FALSE;

		for (m_SocketData.nextAddr = m_SocketData.addrInfo; m_SocketData.nextAddr; m_SocketData.nextAddr = m_SocketData.nextAddr->ai_next)
		{
			bool newSocket = false;
			if (m_SocketData.nFamily == AF_UNSPEC)
			{
				newSocket = true;
				m_SocketData.hSocket = socket(m_SocketData.nextAddr->ai_family, m_SocketData.nextAddr->ai_socktype, m_SocketData.nextAddr->ai_protocol);

				if (m_SocketData.hSocket == INVALID_SOCKET)
					continue;

				m_SocketData.nFamily = m_SocketData.nextAddr->ai_family;
				AttachHandle(m_SocketData.hSocket);
				if (!AsyncSelect(m_lEvent))
				{
					if (newSocket)
					{
						DetachHandle(m_SocketData.hSocket);
						closesocket(m_SocketData.hSocket);
						m_SocketData.hSocket = INVALID_SOCKET;
					}
					continue;
				}
			}
			else if (m_SocketData.hSocket == INVALID_SOCKET)
				continue;

#ifndef NOLAYERS
			if (m_pFirstLayer)
			{
				if (WSAAsyncSelect(m_SocketData.hSocket, GetHelperWindowHandle(), m_SocketData.nSocketIndex+WM_SOCKETEX_NOTIFY, FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE))
				{
					if (newSocket)
					{
						m_SocketData.nFamily = AF_UNSPEC;
						DetachHandle(m_SocketData.hSocket);
						closesocket(m_SocketData.hSocket);
						m_SocketData.hSocket = INVALID_SOCKET;
					}
					continue;
				}
			}
#endif //NOLAYERS

			if (newSocket)
			{
				m_SocketData.nFamily = m_SocketData.nextAddr->ai_family;
				if (!Bind(m_nSocketPort, m_lpszSocketAddress))
				{ 
					m_SocketData.nFamily = AF_UNSPEC;
					DetachHandle(m_SocketData.hSocket);
					closesocket(m_SocketData.hSocket);
					m_SocketData.hSocket = INVALID_SOCKET;
					continue; 
				}
			}

			if (!(ret = CAsyncSocketEx::Connect(m_SocketData.nextAddr->ai_addr, (int)m_SocketData.nextAddr->ai_addrlen)) && GetLastError() != WSAEWOULDBLOCK)
			{
				if (newSocket)
				{
					m_SocketData.nFamily = AF_UNSPEC;
					DetachHandle(m_SocketData.hSocket);
					closesocket(m_SocketData.hSocket);
					m_SocketData.hSocket = INVALID_SOCKET;
				}
				continue;
			}

			break;
		}

		if (m_SocketData.nextAddr)
			m_SocketData.nextAddr = m_SocketData.nextAddr->ai_next;

		if (!m_SocketData.nextAddr)
		{
			p_freeaddrinfo(m_SocketData.addrInfo);
			m_SocketData.nextAddr = 0;
			m_SocketData.addrInfo = 0;
		}

		if (m_SocketData.hSocket == INVALID_SOCKET || !ret)
			return FALSE;
		else
			return TRUE;
	}
}

BOOL CAsyncSocketEx::Connect(const SOCKADDR* lpSockAddr, int nSockAddrLen)
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	BOOL res;
#ifndef NOLAYERS
	if (m_pFirstLayer)
		res = SOCKET_ERROR!=m_pFirstLayer->Connect(lpSockAddr, nSockAddrLen);
	else
#endif //NOLAYERS
		res = SOCKET_ERROR!=connect(m_SocketData.hSocket, lpSockAddr, nSockAddrLen);

#ifndef NOSOCKETSTATES
	if (res || GetLastError()==WSAEWOULDBLOCK)
		SetState(connecting);
#endif //NOSOCKETSTATES
	return res;
}

#ifdef _AFX
BOOL CAsyncSocketEx::GetPeerName(CString& rPeerAddress, UINT& rPeerPort)
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

#ifndef NOLAYERS
	if (m_pFirstLayer)
		return m_pFirstLayer->GetPeerName(rPeerAddress, rPeerPort);
#endif NOLAYERS

	SOCKADDR* sockAddr;
	int nSockAddrLen;

#ifdef ENABLE_IPV6
	if (m_SocketData.nFamily == AF_INET6)
	{
		sockAddr = (SOCKADDR*)new SOCKADDR_IN6;
		nSockAddrLen = sizeof(SOCKADDR_IN6);
	} 
	else 
#endif
	if (m_SocketData.nFamily == AF_INET)
	{
		sockAddr = (SOCKADDR*)new SOCKADDR_IN;
		nSockAddrLen = sizeof(SOCKADDR_IN);
	}

	memset(sockAddr, 0, nSockAddrLen);

	BOOL bResult = GetPeerName(sockAddr, &nSockAddrLen);

	if (bResult)
	{
#ifdef ENABLE_IPV6
		if (m_SocketData.nFamily == AF_INET6)
		{
			rPeerPort = ntohs(((SOCKADDR_IN6*)sockAddr)->sin6_port);
			LPTSTR buf = Inet6AddrToString(((SOCKADDR_IN6*)sockAddr)->sin6_addr);
			rPeerAddress = buf;
			delete [] buf;
		}
		else 
#endif
		if (m_SocketData.nFamily == AF_INET)
		{
			rPeerPort = ntohs(((SOCKADDR_IN*)sockAddr)->sin_port);
			rPeerAddress = inet_ntoa(((SOCKADDR_IN*)sockAddr)->sin_addr);
		}
		else
		{
			delete sockAddr;
			return FALSE;
		}
	}
	delete sockAddr;

	return bResult;
}
#endif //_AFX

BOOL CAsyncSocketEx::GetPeerName( SOCKADDR* lpSockAddr, int* lpSockAddrLen )
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

#ifndef NOLAYERS
	if (m_pFirstLayer)
		return m_pFirstLayer->GetPeerName(lpSockAddr, lpSockAddrLen);
#endif //NOLAYERS

	if (!getpeername(m_SocketData.hSocket, lpSockAddr, lpSockAddrLen))
		return TRUE;
	else
		return FALSE;
}

#ifdef _AFX
BOOL CAsyncSocketEx::GetSockName(CString& rSocketAddress, UINT& rSocketPort)
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	SOCKADDR* sockAddr;
	int nSockAddrLen;

#ifdef ENABLE_IPV6
	if (m_SocketData.nFamily == AF_INET6)
	{
		sockAddr = (SOCKADDR*)new SOCKADDR_IN6;
		nSockAddrLen = sizeof(SOCKADDR_IN6);
	}
	else 
#endif
	if (m_SocketData.nFamily == AF_INET)
	{
		sockAddr = (SOCKADDR*)new SOCKADDR_IN;
		nSockAddrLen = sizeof(SOCKADDR_IN);
	}

	memset(sockAddr, 0, nSockAddrLen);

	BOOL bResult = GetSockName(sockAddr, &nSockAddrLen);

	if (bResult)
	{
#ifdef ENABLE_IPV6
		if (m_SocketData.nFamily == AF_INET6)
		{
			rSocketPort = ntohs(((SOCKADDR_IN6*)sockAddr)->sin6_port);
			LPTSTR buf = Inet6AddrToString(((SOCKADDR_IN6*)sockAddr)->sin6_addr);
			rSocketAddress = buf;
			delete [] buf;
		}
		else 
#endif
		if (m_SocketData.nFamily == AF_INET)
		{
			rSocketPort = ntohs(((SOCKADDR_IN*)sockAddr)->sin_port);
			rSocketAddress = inet_ntoa(((SOCKADDR_IN*)sockAddr)->sin_addr);
		}
		else
		{
			delete sockAddr;
			return FALSE;
		}
	}
	delete sockAddr;

	return bResult;
}
#endif //_AFX

BOOL CAsyncSocketEx::GetSockName(SOCKADDR* lpSockAddr, int* lpSockAddrLen)
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	if (!getsockname(m_SocketData.hSocket, lpSockAddr, lpSockAddrLen))
		return TRUE;
	else
		return FALSE;
}

BOOL CAsyncSocketEx::ShutDown(int nHow /*=sends*/)
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

#ifndef NOLAYERS
	if (m_pFirstLayer)
	{
		return m_pFirstLayer->ShutDown();
	}
	else
#endif //NOLAYERS
	{
		if (!shutdown(m_SocketData.hSocket, nHow))
			return TRUE;
		else
			return FALSE;
	}
}

SOCKET CAsyncSocketEx::Detach()
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	SOCKET socket = m_SocketData.hSocket;
	DetachHandle(socket);
	m_SocketData.hSocket = INVALID_SOCKET;
	m_SocketData.nFamily = AF_UNSPEC;
	return socket;
}

BOOL CAsyncSocketEx::Attach(SOCKET hSocket, long lEvent /*= FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE*/)
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	if (hSocket == INVALID_SOCKET || !hSocket)
		return FALSE;
	VERIFY( InitAsyncSocketExInstance() );
	m_SocketData.hSocket = hSocket;
	AttachHandle(hSocket);

#ifndef NOLAYERS
	if (m_pFirstLayer)
	{
		m_lEvent = lEvent;
		return !WSAAsyncSelect(m_SocketData.hSocket, GetHelperWindowHandle(), m_SocketData.nSocketIndex+WM_SOCKETEX_NOTIFY, FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE);
	}
	else
#endif //NOLAYERS
	{
		return AsyncSelect(lEvent);
	}		
}

BOOL CAsyncSocketEx::AsyncSelect(long lEvent /*= FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE*/)
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	ASSERT( m_pLocalAsyncSocketExThreadData );
	m_lEvent = lEvent;
#ifndef NOLAYERS
	if (m_pFirstLayer)
		return TRUE;
	else
#endif //NOLAYERS
	{
		if (m_SocketData.hSocket == INVALID_SOCKET && m_SocketData.nFamily == AF_UNSPEC)
			return true;

		if (!WSAAsyncSelect(m_SocketData.hSocket, GetHelperWindowHandle(), m_SocketData.nSocketIndex + WM_SOCKETEX_NOTIFY, lEvent))
			return TRUE;
		else
			return FALSE;
	}
	return TRUE;
}

BOOL CAsyncSocketEx::Listen( int nConnectionBacklog /*=5*/ )
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

#ifndef NOLAYERS
	if (m_pFirstLayer)
		return m_pFirstLayer->Listen(nConnectionBacklog);
#endif //NOLAYERS

	if (!listen(m_SocketData.hSocket, nConnectionBacklog))
	{
#ifndef NOSOCKETSTATES
		SetState(listening);
#endif //NOSOCKETSTATES
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CAsyncSocketEx::Accept( CAsyncSocketEx& rConnectedSocket, SOCKADDR* lpSockAddr /*=NULL*/, int* lpSockAddrLen /*=NULL*/ )
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	ASSERT( rConnectedSocket.m_SocketData.hSocket == INVALID_SOCKET );
#ifndef NOLAYERS
	if (m_pFirstLayer)
	{
		return m_pFirstLayer->Accept(rConnectedSocket, lpSockAddr, lpSockAddrLen);
	}
	else
#endif //NOLAYERS
	{
		SOCKET hTemp = accept(m_SocketData.hSocket, lpSockAddr, lpSockAddrLen);
		if (hTemp == INVALID_SOCKET)
			return FALSE;
		VERIFY(rConnectedSocket.InitAsyncSocketExInstance());
		rConnectedSocket.m_SocketData.hSocket = hTemp;
		rConnectedSocket.AttachHandle(hTemp);
		rConnectedSocket.SetFamily(GetFamily());
#ifndef NOSOCKETSTATES
		rConnectedSocket.SetState(connected);
#endif //NOSOCKETSTATES
	}
	return TRUE;
}

BOOL CAsyncSocketEx::IOCtl( long lCommand, DWORD* lpArgument )
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	return ioctlsocket(m_SocketData.hSocket, lCommand, lpArgument) != SOCKET_ERROR;
}

int CAsyncSocketEx::GetLastError()
{
	return WSAGetLastError();
}

BOOL CAsyncSocketEx::TriggerEvent(long lEvent)
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	if (m_SocketData.hSocket == INVALID_SOCKET)
		return FALSE;

	ASSERT( m_pLocalAsyncSocketExThreadData );
	ASSERT( m_pLocalAsyncSocketExThreadData->m_pHelperWindow );
	ASSERT( m_SocketData.nSocketIndex != -1 );

#ifndef NOLAYERS
	if (m_pFirstLayer)
	{
		CAsyncSocketExLayer::t_LayerNotifyMsg *pMsg = new CAsyncSocketExLayer::t_LayerNotifyMsg;
		pMsg->hSocket = m_SocketData.hSocket;
		pMsg->lEvent = lEvent & 0xFFFF;
		pMsg->pLayer = 0;
		BOOL res = PostMessage(GetHelperWindowHandle(), WM_SOCKETEX_TRIGGER, (WPARAM)m_SocketData.nSocketIndex, (LPARAM)pMsg);
		if (!res)
			delete pMsg;
		return res;
	}
	else
#endif //NOLAYERS
		return PostMessage(GetHelperWindowHandle(), m_SocketData.nSocketIndex+WM_SOCKETEX_NOTIFY, m_SocketData.hSocket, lEvent%0xFFFF);

}

SOCKET CAsyncSocketEx::GetSocketHandle()
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	return m_SocketData.hSocket;
}

HWND CAsyncSocketEx::GetHelperWindowHandle()
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	if (!m_pLocalAsyncSocketExThreadData)
		return 0;
	if (!m_pLocalAsyncSocketExThreadData->m_pHelperWindow)
		return 0;
	return m_pLocalAsyncSocketExThreadData->m_pHelperWindow->GetHwnd();
}

#ifndef NOLAYERS
BOOL CAsyncSocketEx::AddLayer(CAsyncSocketExLayer *pLayer)
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	ASSERT( pLayer );
	if (m_SocketData.hSocket != INVALID_SOCKET)
		return FALSE;
	if (m_pFirstLayer)
	{
		ASSERT( m_pLastLayer );
		m_pLastLayer = m_pLastLayer->AddLayer(pLayer, this);
		return m_pLastLayer ? TRUE : FALSE;
	}
	else
	{
		ASSERT( !m_pLastLayer );
		pLayer->Init(0, this);
		m_pFirstLayer = pLayer;
		m_pLastLayer = m_pFirstLayer;
		if (m_SocketData.hSocket != INVALID_SOCKET)
			if (WSAAsyncSelect(m_SocketData.hSocket, GetHelperWindowHandle(), m_SocketData.nSocketIndex+WM_SOCKETEX_NOTIFY, FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE))
				return FALSE;
	}

	return TRUE;
}

void CAsyncSocketEx::RemoveAllLayers()
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	for (std::list<t_callbackMsg>::const_iterator iter = m_pendingCallbacks.begin(); iter != m_pendingCallbacks.end(); iter++)
		delete [] iter->str;
	m_pendingCallbacks.clear();

	m_pFirstLayer = 0;
	m_pLastLayer = 0;

	if (!m_pLocalAsyncSocketExThreadData)
		return;
	if (!m_pLocalAsyncSocketExThreadData->m_pHelperWindow)
		return;
	m_pLocalAsyncSocketExThreadData->m_pHelperWindow->RemoveLayers(this);
}

int CAsyncSocketEx::OnLayerCallback(std::list<t_callbackMsg>& callbacks)
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	for (std::list<t_callbackMsg>::const_iterator iter = callbacks.begin(); iter != callbacks.end(); iter++)
	{
		delete [] iter->str;
	}
	return 0;
}

BOOL CAsyncSocketEx::IsLayerAttached() const
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	return m_pFirstLayer ? TRUE : FALSE;
}
#endif //NOLAYERS

BOOL CAsyncSocketEx::GetSockOpt(int nOptionName, void* lpOptionValue, int* lpOptionLen, int nLevel /*=SOL_SOCKET*/)
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	return (SOCKET_ERROR != getsockopt(m_SocketData.hSocket, nLevel, nOptionName, (LPSTR)lpOptionValue, lpOptionLen));
}

BOOL CAsyncSocketEx::SetSockOpt(int nOptionName, const void* lpOptionValue, int nOptionLen, int nLevel /*=SOL_SOCKET*/)
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	return (SOCKET_ERROR != setsockopt(m_SocketData.hSocket, nLevel, nOptionName, (LPSTR)lpOptionValue, nOptionLen));
}

#ifndef NOSOCKETSTATES

int CAsyncSocketEx::GetState() const
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	return m_nState;
}

void CAsyncSocketEx::SetState(int nState)
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	m_nState = nState;
}

#endif //NOSOCKETSTATES

int CAsyncSocketEx::GetFamily() const
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	return m_SocketData.nFamily;
}

bool CAsyncSocketEx::SetFamily(int nFamily)
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	if (m_SocketData.nFamily != AF_UNSPEC)
		return false;

	m_SocketData.nFamily = nFamily;
	return true;	
}

bool CAsyncSocketEx::TryNextProtocol()
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	DetachHandle(m_SocketData.hSocket);
	closesocket(m_SocketData.hSocket);
	m_SocketData.hSocket = INVALID_SOCKET;

	BOOL ret = FALSE;
	for (; m_SocketData.nextAddr; m_SocketData.nextAddr = m_SocketData.nextAddr->ai_next)
	{
		m_SocketData.hSocket = socket(m_SocketData.nextAddr->ai_family, m_SocketData.nextAddr->ai_socktype, m_SocketData.nextAddr->ai_protocol);

		if (m_SocketData.hSocket == INVALID_SOCKET)
			continue;

		AttachHandle(m_SocketData.hSocket);
		m_SocketData.nFamily = m_SocketData.nextAddr->ai_family;
		if (!AsyncSelect(m_lEvent))
		{
			DetachHandle(m_SocketData.hSocket);
			closesocket(m_SocketData.hSocket);
			m_SocketData.hSocket = INVALID_SOCKET;
			continue;
		}

#ifndef NOLAYERS
		if (m_pFirstLayer)
		{
			if (WSAAsyncSelect(m_SocketData.hSocket, GetHelperWindowHandle(), m_SocketData.nSocketIndex+WM_SOCKETEX_NOTIFY, FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE))
			{
				DetachHandle(m_SocketData.hSocket);
				closesocket(m_SocketData.hSocket);
				m_SocketData.hSocket = INVALID_SOCKET;
				continue;
			}
		}
#endif //NOLAYERS

		if (!Bind(m_nSocketPort, m_lpszSocketAddress))
		{ 
			DetachHandle(m_SocketData.hSocket);
			closesocket(m_SocketData.hSocket);
			m_SocketData.hSocket = INVALID_SOCKET;
			continue; 
		}

		ret = CAsyncSocketEx::Connect(m_SocketData.nextAddr->ai_addr, (int)m_SocketData.nextAddr->ai_addrlen);
		if (!ret && GetLastError() != WSAEWOULDBLOCK)
		{
			DetachHandle(m_SocketData.hSocket);
			closesocket(m_SocketData.hSocket);
			m_SocketData.hSocket = INVALID_SOCKET;
			continue;
		}

		ret = true;
		break;
	}

	if (m_SocketData.nextAddr)
		m_SocketData.nextAddr = m_SocketData.nextAddr->ai_next;

	if (!m_SocketData.nextAddr)
	{
		p_freeaddrinfo(m_SocketData.addrInfo);
		m_SocketData.nextAddr = 0;
		m_SocketData.addrInfo = 0;
	}

	if (m_SocketData.hSocket == INVALID_SOCKET || !ret)
		return FALSE;
	else
		return TRUE;
}

#ifndef NOLAYERS

void CAsyncSocketEx::AddCallbackNotification(const t_callbackMsg& msg)
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	m_pendingCallbacks.push_back(msg);

	if(m_pendingCallbacks.size() == 1 && m_SocketData.nSocketIndex != -1)
		PostMessage(GetHelperWindowHandle(), WM_SOCKETEX_CALLBACK, (WPARAM)m_SocketData.nSocketIndex, 0);
}

#endif //NOLAYERS

void CAsyncSocketEx::ResendCloseNotify()
{
	ASSERT(IsValidAsyncSocketEx(this)); // netfinity: Ensure socket object really exist

	for (std::list<CAsyncSocketEx*>::const_iterator iter = m_pLocalAsyncSocketExThreadData->layerCloseNotify.begin(); iter != m_pLocalAsyncSocketExThreadData->layerCloseNotify.end(); iter++)
	{
		if (*iter == this)
			return;
	}
	m_pLocalAsyncSocketExThreadData->layerCloseNotify.push_back(this);
	if (m_pLocalAsyncSocketExThreadData->layerCloseNotify.size() == 1)
	{
		SetTimer(m_pLocalAsyncSocketExThreadData->m_pHelperWindow->GetHwnd(), 1, 10, 0);
	}
}
#ifdef _DEBUG
void CAsyncSocketEx::AssertValid() const
{
	CObject::AssertValid();

	(void)m_SocketData;
	(void)m_lEvent;
	(void)m_pAsyncGetHostByNameBuffer;
	(void)m_hAsyncGetHostByNameHandle;
	(void)m_uMagic;
	(void)m_nAsyncGetHostByNamePort;
	(void)m_nSocketPort;
	(void)m_lpszSocketAddress;
	(void)m_hDll;
	(void)p_getaddrinfo;
	(void)p_freeaddrinfo;
#ifndef NOLAYERS
	(void)m_pendingCallbacks;
#endif // NOLAYERS

	//Pointer to the data of the local thread
//	struct t_AsyncSocketExThreadData
//	{
//		CAsyncSocketExHelperWindow *m_pHelperWindow;
//		int nInstanceCount;
//		DWORD nThreadId;
//	} *m_pLocalAsyncSocketExThreadData;

	//List of the data structures for all threads
//	static struct t_AsyncSocketExThreadDataList
//	{
//		t_AsyncSocketExThreadDataList *pNext;
//		t_AsyncSocketExThreadData *pThreadData;
//	} *m_spAsyncSocketExThreadDataList;

#ifndef NOLAYERS
	CHECK_PTR(m_pFirstLayer);
	CHECK_PTR(m_pLastLayer);
#endif //NOLAYERS
}
#endif

#ifdef _DEBUG
void CAsyncSocketEx::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif
