/*CAsyncSocketEx by Tim Kosse (Tim.Kosse@gmx.de)
            Version 1.1 (2002-11-01)
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
#include "AsyncSocketExLayer.h"
#include "AsyncSocketEx.h"
#include "otherfunctions.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CAsyncSocketExLayer::CAsyncSocketExLayer()
{
	m_pOwnerSocket = NULL;
	m_pNextLayer = NULL;
	m_pPrevLayer = NULL;

	m_nLayerState = LAYERSTATE_NOTSOCK;
	m_nCriticalError = 0;
}

CAsyncSocketExLayer::~CAsyncSocketExLayer()
{
}

CAsyncSocketExLayer *CAsyncSocketExLayer::AddLayer(CAsyncSocketExLayer *pLayer, CAsyncSocketEx *pOwnerSocket)
{
	ASSERT(pLayer);
	ASSERT(pOwnerSocket);
	if (m_pNextLayer)
	{
		return m_pNextLayer->AddLayer(pLayer, pOwnerSocket);
	}
	else
	{
		ASSERT(m_pOwnerSocket==pOwnerSocket);
		pLayer->Init(this, m_pOwnerSocket);
		m_pNextLayer=pLayer;
	}
	return m_pNextLayer;
}

int CAsyncSocketExLayer::Receive(void* lpBuf, int nBufLen, int nFlags /*=0*/)
{
	return ReceiveNext(lpBuf, nBufLen, nFlags);
}

int CAsyncSocketExLayer::Send(const void* lpBuf, int nBufLen, int nFlags /*=0*/)
{
	return SendNext(lpBuf, nBufLen, nFlags);
}

void CAsyncSocketExLayer::OnReceive(int nErrorCode)
{
	if (m_pPrevLayer)
		m_pPrevLayer->OnReceive(nErrorCode);
	else
		if (m_pOwnerSocket->m_lEvent&FD_READ)
			m_pOwnerSocket->OnReceive(nErrorCode);
}

void CAsyncSocketExLayer::OnSend(int nErrorCode)
{
	if (m_pPrevLayer)
		m_pPrevLayer->OnSend(nErrorCode);
	else
		if (m_pOwnerSocket->m_lEvent&FD_WRITE)
			m_pOwnerSocket->OnSend(nErrorCode);
}

void CAsyncSocketExLayer::OnConnect(int nErrorCode)
{
	if (m_pPrevLayer)
		m_pPrevLayer->OnConnect(nErrorCode);
	else
		if (m_pOwnerSocket->m_lEvent&FD_CONNECT)
			m_pOwnerSocket->OnConnect(nErrorCode);
}

void CAsyncSocketExLayer::OnAccept(int nErrorCode)
{
	if (m_pPrevLayer)
		m_pPrevLayer->OnAccept(nErrorCode);
	else
		if (m_pOwnerSocket->m_lEvent&FD_ACCEPT)
			m_pOwnerSocket->OnAccept(nErrorCode);
}

void CAsyncSocketExLayer::OnClose(int nErrorCode)
{
	if (m_pPrevLayer)
		m_pPrevLayer->OnClose(nErrorCode);
	else
		if (m_pOwnerSocket->m_lEvent&FD_CLOSE)
			m_pOwnerSocket->OnClose(nErrorCode);
}

BOOL CAsyncSocketExLayer::TriggerEvent(long lEvent, int nErrorCode, BOOL bPassThrough /*=FALSE*/ )
{
	ASSERT(m_pOwnerSocket);
	if (m_pOwnerSocket->m_SocketData.hSocket == INVALID_SOCKET)
		return FALSE;

	if (lEvent & FD_CONNECT)
	{
		ASSERT(bPassThrough);
		if (nErrorCode == 0)
			ASSERT(bPassThrough && GetLayerState() == LAYERSTATE_CONNECTED);
		else
		{
			SetLayerState(LAYERSTATE_ABORTED);
			m_nCriticalError = nErrorCode;
		}
	}
	else if (lEvent & FD_CLOSE && nErrorCode == 0)
	{
		SetLayerState(LAYERSTATE_CLOSED);
	}
	else if (lEvent & FD_CLOSE && nErrorCode != 0)
	{
		SetLayerState(LAYERSTATE_ABORTED);
		m_nCriticalError = nErrorCode;
	}
	ASSERT(m_pOwnerSocket->m_pLocalAsyncSocketExThreadData);
	ASSERT(m_pOwnerSocket->m_pLocalAsyncSocketExThreadData->m_pHelperWindow);
	ASSERT(m_pOwnerSocket->m_SocketData.nSocketIndex != -1);
	t_LayerNotifyMsg *pMsg = new t_LayerNotifyMsg;
	pMsg->lEvent = (lEvent & 0xFFFF) + (nErrorCode << 16);
	pMsg->pLayer = bPassThrough ? m_pPrevLayer : this;
	BOOL res = PostMessage(m_pOwnerSocket->GetHelperWindowHandle(), WM_SOCKETEX_TRIGGER, (WPARAM)m_pOwnerSocket, (LPARAM)pMsg);
	if (!res)
		delete pMsg;
	return res;
}

void CAsyncSocketExLayer::Close()
{
	CloseNext();
}

void CAsyncSocketExLayer::CloseNext()
{
	SetLayerState(LAYERSTATE_NOTSOCK);
	if (m_pNextLayer)
		m_pNextLayer->Close();
}

BOOL CAsyncSocketExLayer::Connect(LPCSTR lpszHostAddress, UINT nHostPort)
{
	return ConnectNext(lpszHostAddress, nHostPort);
}

BOOL CAsyncSocketExLayer::Connect(const SOCKADDR* lpSockAddr, int nSockAddrLen)
{
	return ConnectNext(lpSockAddr, nSockAddrLen);
}

int CAsyncSocketExLayer::SendNext(const void *lpBuf, int nBufLen, int nFlags /*=0*/)
{
	if (m_nCriticalError)
	{
		WSASetLastError(m_nCriticalError);
		return SOCKET_ERROR;
	}
	else if (GetLayerState()==LAYERSTATE_NOTSOCK)
	{
		WSASetLastError(WSAENOTSOCK);
		return SOCKET_ERROR;
	}
	else if (GetLayerState()==LAYERSTATE_UNCONNECTED || GetLayerState()==LAYERSTATE_CONNECTING || GetLayerState()==LAYERSTATE_LISTENING)
	{
		WSASetLastError(WSAENOTCONN);
		return SOCKET_ERROR;
	}

	if (!m_pNextLayer)
	{
		ASSERT(m_pOwnerSocket);
		return send(m_pOwnerSocket->GetSocketHandle(), (LPSTR)lpBuf, nBufLen, nFlags);
	}
	else
		return m_pNextLayer->Send(lpBuf, nBufLen, nFlags);
}

int CAsyncSocketExLayer::ReceiveNext(void *lpBuf, int nBufLen, int nFlags /*=0*/)
{
	if (m_nCriticalError)
	{
		WSASetLastError(m_nCriticalError);
		return SOCKET_ERROR;
	}
	else if (GetLayerState()==LAYERSTATE_NOTSOCK)
	{
		WSASetLastError(WSAENOTSOCK);
		return SOCKET_ERROR;
	}
	else if (GetLayerState()==LAYERSTATE_UNCONNECTED || GetLayerState()==LAYERSTATE_CONNECTING || GetLayerState()==LAYERSTATE_LISTENING)
	{
		WSASetLastError(WSAENOTCONN);
		return SOCKET_ERROR;
	}

	if (!m_pNextLayer)
	{
		ASSERT(m_pOwnerSocket);
		return recv(m_pOwnerSocket->GetSocketHandle(), (LPSTR)lpBuf, nBufLen, nFlags);
	}
	else
		return m_pNextLayer->Receive(lpBuf, nBufLen, nFlags);
}

BOOL CAsyncSocketExLayer::ConnectNext(LPCSTR lpszHostAddress, UINT nHostPort)
{
	ASSERT(lpszHostAddress != NULL);
	ASSERT(GetLayerState() == LAYERSTATE_UNCONNECTED);
	ASSERT(m_pOwnerSocket);

	BOOL res;
	if (m_pNextLayer)
		res = m_pNextLayer->Connect(lpszHostAddress, nHostPort);
	else
	{
		SOCKADDR_IN sockAddr = {0};

		sockAddr.sin_addr.s_addr = inet_addr(lpszHostAddress);
		if (sockAddr.sin_addr.s_addr == INADDR_NONE)
		{
			LPHOSTENT lphost = gethostbyname(lpszHostAddress);

			if (lphost == NULL)
			{
				WSASetLastError(WSAEINVAL);
				return FALSE;
			}
			sockAddr.sin_addr.s_addr = ((LPIN_ADDR)lphost->h_addr)->s_addr;
		}
		sockAddr.sin_family = AF_INET;
		sockAddr.sin_port = fast_htons((u_short)nHostPort);
		res = (connect(m_pOwnerSocket->GetSocketHandle(), (SOCKADDR*)&sockAddr, sizeof(sockAddr)) != SOCKET_ERROR);
	}

	if (res || WSAGetLastError() == WSAEWOULDBLOCK)
		SetLayerState(LAYERSTATE_CONNECTING);

	return res;
}

BOOL CAsyncSocketExLayer::ConnectNext(const SOCKADDR* lpSockAddr, int nSockAddrLen)
{
	ASSERT(GetLayerState() == LAYERSTATE_UNCONNECTED);
	ASSERT(m_pOwnerSocket);

	BOOL res;
	if (m_pNextLayer)
		res = m_pNextLayer->Connect(lpSockAddr, nSockAddrLen);
	else
		res = (connect(m_pOwnerSocket->GetSocketHandle(), lpSockAddr, nSockAddrLen) != SOCKET_ERROR);

	if (res || WSAGetLastError() == WSAEWOULDBLOCK)
		SetLayerState(LAYERSTATE_CONNECTING);

	return res;
}

//Gets the address of the peer socket to which the socket is connected
#ifdef _AFX

BOOL CAsyncSocketExLayer::GetPeerName(CString& rPeerAddress, UINT& rPeerPort)
{
	return GetPeerNameNext(rPeerAddress, rPeerPort);
}

BOOL CAsyncSocketExLayer::GetPeerNameNext(CString& rPeerAddress, UINT& rPeerPort)
{
	if (m_pNextLayer)
		return m_pNextLayer->GetPeerName(rPeerAddress, rPeerPort);
	else
	{
		ASSERT(m_pOwnerSocket);
		SOCKADDR_IN sockAddr = {0};

		int nSockAddrLen = sizeof(sockAddr);

		if (!getpeername(m_pOwnerSocket->GetSocketHandle(), (SOCKADDR*)&sockAddr, &nSockAddrLen))
		{
			rPeerPort = ntohs(sockAddr.sin_port);
			ipstr(&rPeerAddress, sockAddr.sin_addr);
			return TRUE;
		}
		else
			return FALSE;
	}
}

#endif //_AFX

BOOL CAsyncSocketExLayer::GetPeerName( SOCKADDR* lpSockAddr, int* lpSockAddrLen )
{
	return GetPeerNameNext(lpSockAddr, lpSockAddrLen);
}

BOOL CAsyncSocketExLayer::GetPeerNameNext( SOCKADDR* lpSockAddr, int* lpSockAddrLen )
{
	if (m_pNextLayer)
		return m_pNextLayer->GetPeerName(lpSockAddr, lpSockAddrLen);
	else
	{
		ASSERT(m_pOwnerSocket);
		if ( !getpeername(m_pOwnerSocket->GetSocketHandle(), lpSockAddr, lpSockAddrLen) )
			return TRUE;
		else
			return FALSE;
	}
}

void CAsyncSocketExLayer::Init(CAsyncSocketExLayer *pPrevLayer, CAsyncSocketEx *pOwnerSocket)
{
	ASSERT(pOwnerSocket);
	m_pPrevLayer=pPrevLayer;
	m_pOwnerSocket=pOwnerSocket;
	m_pNextLayer=0;
}

int CAsyncSocketExLayer::GetLayerState()
{
	return m_nLayerState;
}

void CAsyncSocketExLayer::CallEvent(int nEvent, int nErrorCode)
{
	if (m_nCriticalError)
		return;
	m_nCriticalError=nErrorCode;
	switch (nEvent)
	{
	case FD_READ:
	case FD_FORCEREAD:
		if (GetLayerState()==LAYERSTATE_CONNECTED)
		{
			if (nErrorCode)
				SetLayerState(LAYERSTATE_ABORTED);
			OnReceive(nErrorCode);
		}
		break;
	case FD_WRITE:
		if (GetLayerState()==LAYERSTATE_CONNECTED)
		{
			if (nErrorCode)
				SetLayerState(LAYERSTATE_ABORTED);
			OnSend(nErrorCode);
		}
		break;
	case FD_CONNECT:
		if (GetLayerState()==LAYERSTATE_CONNECTING)
		{
			if (!nErrorCode)
				SetLayerState(LAYERSTATE_CONNECTED);
			else
				SetLayerState(LAYERSTATE_ABORTED);
			OnConnect(nErrorCode);
		}
		break;
	case FD_ACCEPT:
		if (GetLayerState()==LAYERSTATE_LISTENING)
		{
			if (!nErrorCode)
				SetLayerState(LAYERSTATE_CONNECTED);
			else
				SetLayerState(LAYERSTATE_ABORTED);
			OnAccept(nErrorCode);
		}
		break;
	case FD_CLOSE:
		if (GetLayerState()==LAYERSTATE_CONNECTED)
		{
			if (nErrorCode)
				SetLayerState(LAYERSTATE_ABORTED);
			else
				SetLayerState(LAYERSTATE_CLOSED);
			OnClose(nErrorCode);
		}
		break;
	}
}

BOOL CAsyncSocketExLayer::Create(UINT nSocketPort, int nSocketType, long lEvent, LPCSTR lpszSocketAddress)
{
	return CreateNext(nSocketPort, nSocketType, lEvent, lpszSocketAddress);
}

BOOL CAsyncSocketExLayer::CreateNext(UINT nSocketPort, int nSocketType, long lEvent, LPCSTR lpszSocketAddress)
{
	ASSERT(GetLayerState() == LAYERSTATE_NOTSOCK);
	BOOL res = FALSE;
	if (m_pNextLayer)
		res = m_pNextLayer->Create(nSocketPort, nSocketType, lEvent, lpszSocketAddress);
	else
	{
		SOCKET hSocket=socket(AF_INET, nSocketType, 0);
		if (hSocket==INVALID_SOCKET)
			res=FALSE;
		m_pOwnerSocket->m_SocketData.hSocket=hSocket;
		m_pOwnerSocket->AttachHandle(hSocket);
		if (!m_pOwnerSocket->AsyncSelect(lEvent))
		{
			m_pOwnerSocket->Close();
			res=FALSE;
		}
		if (m_pOwnerSocket->m_pFirstLayer)
		{
			if (WSAAsyncSelect(m_pOwnerSocket->m_SocketData.hSocket, m_pOwnerSocket->GetHelperWindowHandle(), m_pOwnerSocket->m_SocketData.nSocketIndex+WM_SOCKETEX_NOTIFY, FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE) )
			{
				m_pOwnerSocket->Close();
				res=FALSE;
			}
		}
		if (!m_pOwnerSocket->Bind(nSocketPort, lpszSocketAddress))
		{
			m_pOwnerSocket->Close();
			res=FALSE;
		}
		res=TRUE;
	}
	if (res)
		SetLayerState(LAYERSTATE_UNCONNECTED);
	return res;
}

void CAsyncSocketExLayer::SetLayerState(int nLayerState)
{
	ASSERT(m_pOwnerSocket);
	int nOldLayerState = GetLayerState();
	m_nLayerState = nLayerState;
	if (nOldLayerState != nLayerState)
		DoLayerCallback(LAYERCALLBACK_STATECHANGE, GetLayerState(), nOldLayerState);
}

int CAsyncSocketExLayer::DoLayerCallback(int nType, int nCode, WPARAM wParam, LPARAM lParam)
{
	ASSERT(m_pOwnerSocket);

	int nError = WSAGetLastError();
	int res = m_pOwnerSocket->OnLayerCallback(this, nType, nCode, wParam, lParam);

	WSASetLastError(nError);

	return res;
}

BOOL CAsyncSocketExLayer::Listen(int nConnectionBacklog)
{
	return ListenNext( nConnectionBacklog);
}

BOOL CAsyncSocketExLayer::ListenNext(int nConnectionBacklog)
{
	ASSERT(GetLayerState() == LAYERSTATE_UNCONNECTED);
	BOOL res;
	if (m_pNextLayer)
		res=m_pNextLayer->Listen(nConnectionBacklog);
	else
		res=listen(m_pOwnerSocket->GetSocketHandle(), nConnectionBacklog);
	if (res)
	{
		SetLayerState(LAYERSTATE_LISTENING);
	}
	return res;
}

BOOL CAsyncSocketExLayer::Accept( CAsyncSocketEx& rConnectedSocket, SOCKADDR* lpSockAddr /*=NULL*/, int* lpSockAddrLen /*=NULL*/ )
{
	return AcceptNext(rConnectedSocket, lpSockAddr, lpSockAddrLen);
}

BOOL CAsyncSocketExLayer::AcceptNext( CAsyncSocketEx& rConnectedSocket, SOCKADDR* lpSockAddr /*=NULL*/, int* lpSockAddrLen /*=NULL*/ )
{
	ASSERT(GetLayerState()==LAYERSTATE_LISTENING);
	BOOL res;
	if (m_pNextLayer)
		res=m_pNextLayer->Accept(rConnectedSocket, lpSockAddr, lpSockAddrLen);
	else
	{
		SOCKET hTemp = accept(m_pOwnerSocket->m_SocketData.hSocket, lpSockAddr, lpSockAddrLen);

		if (hTemp == INVALID_SOCKET)
			return FALSE;
		VERIFY(rConnectedSocket.InitAsyncSocketExInstance());
		rConnectedSocket.m_SocketData.hSocket=hTemp;
		rConnectedSocket.AttachHandle(hTemp);
	}
	return TRUE;
}

BOOL CAsyncSocketExLayer::ShutDown(int nHow /*=sends*/)
{
	return ShutDownNext(nHow);
}

BOOL CAsyncSocketExLayer::ShutDownNext(int nHow /*=sends*/)
{
	if (m_nCriticalError)
	{
		WSASetLastError(m_nCriticalError);
		return FALSE;
	}
	else if (GetLayerState()==LAYERSTATE_NOTSOCK)
	{
		WSASetLastError(WSAENOTSOCK);
		return FALSE;
	}
	else if (GetLayerState()==LAYERSTATE_UNCONNECTED || GetLayerState()==LAYERSTATE_CONNECTING || GetLayerState()==LAYERSTATE_LISTENING)
	{
		WSASetLastError(WSAENOTCONN);
		return FALSE;
	}

	if (!m_pNextLayer)
	{
		ASSERT(m_pOwnerSocket);
		return shutdown(m_pOwnerSocket->GetSocketHandle(), nHow);
	}
	else
		return m_pNextLayer->ShutDownNext(nHow);
}