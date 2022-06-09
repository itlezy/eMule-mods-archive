/*CAsyncProxySocketLayer by Tim Kosse (Tim.Kosse@gmx.de)
                 Version 1.6 (2003-03-26)
--------------------------------------------------------

Introduction:
-------------

This class is layer class for CAsyncSocketEx. With this class you
can connect through SOCKS4/5 and HTTP 1.1 proxies. This class works
as semi-transparent layer between CAsyncSocketEx and the actual socket.
This class is used in FileZilla, a powerful open-source FTP client.
It can be found under http://sourceforge.net/projects/filezilla
For more information about SOCKS4/5 goto
http://www.socks.nec.com/socksprot.html
For more information about HTTP 1.1 goto http://www.rfc-editor.org
and search for RFC2616

How to use?
-----------

You don't have to change much in you already existing code to use
CAsyncProxySocketLayer.
To use it, create an instance of CAsyncProxySocketLayer, call SetProxy
and attach it to a CAsyncSocketEx instance.
You have to process OnLayerCallback in you CAsyncSocketEx instance as it will
receive all layer nofications.
The following notifications are sent:

//Error codes
PROXYERROR_NOERROR 0
PROXYERROR_NOCONN 1 //Can't connect to proxy server, use GetLastError for more information
PROXYERROR_REQUESTFAILED 2 //Request failed, can't send data
PROXYERROR_AUTHREQUIRED 3 //Authentication required
PROXYERROR_AUTHTYPEUNKNOWN 4 //Authtype unknown or not supported
PROXYERROR_AUTHFAILED 5  //Authentication failed
PROXYERROR_AUTHNOLOGON 6
PROXYERROR_CANTRESOLVEHOST 7

//Status messages
PROXYSTATUS_LISTENSOCKETCREATED 8 //Called when a listen socket was created successfully. Unlike the normal listen function,
								//a socksified socket has to connect to the proxy to negotiate the details with the server
								//on which the listen socket will be created
								//The two parameters will contain the ip and port of the listen socket on the server.

If you want to use CAsyncProxySocketLayer to create a listen socket, you
have to use this overloaded function:
BOOL PrepareListen(unsigned long serverIp);
serverIP is the IP of the server you are already connected
through the SOCKS proxy. You can't use listen sockets over a
SOCKS proxy without a primary connection. Listen sockets are only
supported by SOCKS proxies, this won't work with HTTP proxies.
When the listen socket is created successfully, the PROXYSTATUS_LISTENSOCKETCREATED
notification is sent. The parameters  will tell you the ip and the port of the listen socket.
After it you have to handle the OnAccept message and accept the
connection.
Be carful when calling Accept: rConnected socket will NOT be filled! Instead use the instance which created the
listen socket, it will handle the data connection.
If you want to accept more than one connection, you have to create a listing socket for each of them!

Description of important functions and their parameters:
--------------------------------------------------------

void SetProxy(int nProxyType);
void SetProxy(int nProxyType, const char * pProxyHost, int nProxyPort);
void SetProxy(int nProxyType, const char * pProxyHost, int nProxyPort, const char *pProxyUser, const char * pProxyPass);

Call one of this functions to set the proxy type.
Parametes:
- nProxyType specifies the Proxy Type.
- ProxyHost and nProxyPort specify the address of the proxy
- ProxyUser and ProxyPass are only available for SOCKS5 proxies.

supported proxy types:
PROXYTYPE_NOPROXY
PROXYTYPE_SOCKS4
PROXYTYPE_SOCKS4A
PROXYTYPE_SOCKS5
PROXYTYPE_HTTP11

There are also some other functions:

GetProxyPeerName
Like GetPeerName of CAsyncSocket, but returns the address of the
server connected through the proxy.	If using proxies, GetPeerName
only returns the address of the proxy.

int GetProxyType();
Returns the used proxy

const int GetLastProxyError() const;
Returns the last proxy error

License
-------

Feel free to use this class, as long as you don't claim that you wrote it
and this copyright notice stays intact in the source files.
If you use this class in commercial applications, please send a short message
to tim.kosse@gmx.de

Version history
---------------

- 1.6 got rid of MFC
- 1.5 released CAsyncSocketExLayer version
- 1.4 added UNICODE support
- 1.3 added basic HTTP1.1 authentication
      fixed memory leak in SOCKS5 code
	  OnSocksOperationFailed will be called after Socket has been closed
      fixed some minor bugs
- 1.2 renamed into CAsyncProxySocketLayer
      added HTTP1.1 proxy support
- 1.1 fixes all known bugs, mostly with SOCKS5 authentication
- 1.0 initial release
*/

#include "stdafx.h"
#include "AsyncProxySocketLayer.h"
#include "CBase64coding.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CAsyncProxySocketLayer::CAsyncProxySocketLayer()
{
	m_nProxyOpID=0;
	m_nProxyOpState=0;
	m_pRecvBuffer=0;
	m_nRecvBufferPos=0;
	m_ProxyData.nProxyType=0;
	m_nProxyPeerIp=0;
	m_nProxyPeerPort=0;
	m_pProxyPeerHost = NULL;
	m_pStrBuffer = NULL;
	m_ProxyData.pProxyHost = NULL;
	m_ProxyData.pProxyUser = NULL;
	m_ProxyData.pProxyPass = NULL;
	m_pProxyPeerHost = NULL;
}

CAsyncProxySocketLayer::~CAsyncProxySocketLayer()
{
	delete [] m_ProxyData.pProxyHost;
	delete [] m_ProxyData.pProxyUser;
	delete [] m_ProxyData.pProxyPass;
	delete [] m_pProxyPeerHost;
	ClearBuffer();
}

/////////////////////////////////////////////////////////////////////////////
// Member-Funktion CAsyncProxySocketLayer

void CAsyncProxySocketLayer::SetProxy(int nProxyType)
{
	//Validate the parameters
	ASSERT(nProxyType==PROXYTYPE_NOPROXY);
	m_ProxyData.nProxyType=nProxyType;
}

void CAsyncProxySocketLayer::SetProxy(int nProxyType, const char * pProxyHost, int ProxyPort)
{
	//Validate the parameters
	ASSERT(nProxyType==PROXYTYPE_SOCKS4  ||
		   nProxyType==PROXYTYPE_SOCKS4A ||
		   nProxyType==PROXYTYPE_SOCKS5  ||
		   nProxyType==PROXYTYPE_HTTP11);
	ASSERT(!m_nProxyOpID);
	ASSERT(pProxyHost && *pProxyHost);
	ASSERT(ProxyPort>0);
	ASSERT(ProxyPort<=65535);

	delete m_ProxyData.pProxyHost;
	delete m_ProxyData.pProxyUser;
	delete m_ProxyData.pProxyPass;
	m_ProxyData.pProxyUser = NULL;
	m_ProxyData.pProxyPass = NULL;

	m_ProxyData.nProxyType = nProxyType;
	size_t nlen1 = strlen(pProxyHost) + 1;
	m_ProxyData.pProxyHost = new char[nlen1];
	strcpy_s(m_ProxyData.pProxyHost, nlen1, pProxyHost);
	m_ProxyData.nProxyPort = ProxyPort;
	m_ProxyData.bUseLogon = FALSE;
}

void CAsyncProxySocketLayer::SetProxy(int nProxyType, const char * pProxyHost, int ProxyPort, const char * pProxyUser, const char * pProxyPass)
{
	//Validate the parameters
	ASSERT(nProxyType==PROXYTYPE_SOCKS5 || nProxyType==PROXYTYPE_HTTP11);
	ASSERT(!m_nProxyOpID);
	ASSERT(pProxyHost && *pProxyHost);
	ASSERT(ProxyPort>0);
	ASSERT(ProxyPort<=65535);

	delete m_ProxyData.pProxyHost;
	delete m_ProxyData.pProxyUser;
	delete m_ProxyData.pProxyPass;
	m_ProxyData.pProxyUser = NULL;
	m_ProxyData.pProxyPass = NULL;

	m_ProxyData.nProxyType = nProxyType;
	size_t nlen1 = strlen(pProxyHost) + 1;
	m_ProxyData.pProxyHost = new char[nlen1];
	strcpy_s(m_ProxyData.pProxyHost, nlen1, pProxyHost);
	m_ProxyData.nProxyPort=ProxyPort;
	if (pProxyUser)
	{
		nlen1 = strlen(pProxyUser) + 1;
		m_ProxyData.pProxyUser = new char[nlen1];
		strcpy_s(m_ProxyData.pProxyUser, nlen1, pProxyUser);
	}
	if (pProxyPass)
	{
		nlen1 = strlen(pProxyPass) + 1;
		m_ProxyData.pProxyPass = new char[nlen1];
		strcpy_s(m_ProxyData.pProxyPass, nlen1, pProxyPass);
	}
	m_ProxyData.bUseLogon = TRUE;
}

void CAsyncProxySocketLayer::OnReceive(int nErrorCode)
{
	//Here we handle the responses from the SOCKS proxy
	if (!m_nProxyOpID)
	{
		TriggerEvent(FD_READ, nErrorCode, TRUE);
		return;
	}
	if (nErrorCode)
		TriggerEvent(FD_READ, nErrorCode, TRUE);

	if (!m_nProxyOpState) //We should not receive a response yet!
		return;

	if (m_ProxyData.nProxyType == PROXYTYPE_SOCKS4 || m_ProxyData.nProxyType == PROXYTYPE_SOCKS4A)
	{
		if (m_nProxyOpState==1) //Both for PROXYOP_CONNECT and PROXYOP_BIND
		{
			if (!m_pRecvBuffer)
				m_pRecvBuffer = new char[8];
			int numread = ReceiveNext(m_pRecvBuffer + m_nRecvBufferPos, 8 - m_nRecvBufferPos);
			if (numread==SOCKET_ERROR)
			{
				if (WSAGetLastError()!=WSAEWOULDBLOCK)
				{
					DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_REQUESTFAILED, 0);
					TriggerEvent((m_nProxyOpID == PROXYOP_CONNECT)?FD_CONNECT:FD_ACCEPT, WSAGetLastError(), TRUE);// X: [CI] - [Code Improvement]
					Reset();
					ClearBuffer();
				}
				return;
			}
			m_nRecvBufferPos+=numread;
			if (m_nRecvBufferPos==8)
			{
				if (m_pRecvBuffer[1]!=90 || m_pRecvBuffer[0]!=0)
				{
					DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_REQUESTFAILED, 0);
					TriggerEvent((m_nProxyOpID == PROXYOP_CONNECT)?FD_CONNECT:FD_ACCEPT, WSAECONNABORTED, TRUE);// X: [CI] - [Code Improvement]
					Reset();
					ClearBuffer();
					return;
				}
				if (m_nProxyOpID==PROXYOP_CONNECT)
				{
					//OK, we are connected with the remote server
					ClearBuffer();
					Reset();
					TriggerEvent(FD_CONNECT, 0, TRUE);
					TriggerEvent(FD_READ, 0, TRUE);
					TriggerEvent(FD_WRITE, 0, TRUE);
					return;
				}
				else
				{
					//Listen socket created
					m_nProxyOpState++;
					unsigned long ip;
					int port;
					memcpy(&ip,&m_pRecvBuffer[4],4);
					if (!ip)
					{ //No IP return, use the IP of the proxy server
						SOCKADDR SockAddr;
						memset(&SockAddr,0,sizeof(SockAddr));
						int SockAddrLen=sizeof(SockAddr);
						if (GetPeerName(&SockAddr, &SockAddrLen ))
						{
							ip=((LPSOCKADDR_IN)&SockAddr)->sin_addr.S_un.S_addr;
						}
						else
						{
							DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_REQUESTFAILED, 0);
							TriggerEvent((m_nProxyOpID == PROXYOP_CONNECT)?FD_CONNECT:FD_ACCEPT, WSAECONNABORTED, TRUE);// X: [CI] - [Code Improvement]
							Reset();
							ClearBuffer();
							return;
						}
					}
					memcpy(&port,&m_pRecvBuffer[2],2);
					t_ListenSocketCreatedStruct data;
					data.ip=ip;
					data.nPort=port;
					DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYSTATUS_LISTENSOCKETCREATED, (INT_PTR) &data);
				}
				ClearBuffer();
			}
		}
		else if (m_nProxyOpID==2)
		{
			if (!m_pRecvBuffer)
				m_pRecvBuffer=new char[8];
			int numread=ReceiveNext(m_pRecvBuffer+m_nRecvBufferPos,8-m_nRecvBufferPos);
			if (numread==SOCKET_ERROR)
			{
				if (WSAGetLastError()!=WSAEWOULDBLOCK)
				{
					DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_REQUESTFAILED, 0);
					TriggerEvent((m_nProxyOpID == PROXYOP_CONNECT)?FD_CONNECT:FD_ACCEPT, WSAGetLastError(), TRUE);// X: [CI] - [Code Improvement]
					Reset();
					ClearBuffer();
				}
				return;
			}
			m_nRecvBufferPos+=numread;
			if (m_nRecvBufferPos==8)
			{
				if (m_pRecvBuffer[1]!=90 || m_pRecvBuffer[0]!=0)
				{
					DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_REQUESTFAILED, 0);
					TriggerEvent((m_nProxyOpID == PROXYOP_CONNECT)?FD_CONNECT:FD_ACCEPT, WSAECONNABORTED, TRUE);// X: [CI] - [Code Improvement]
					Reset();
					ClearBuffer();
					return;
				}
				//Connection to remote server established
				ClearBuffer();
				Reset();
				TriggerEvent(FD_ACCEPT, 0, TRUE);
				TriggerEvent(FD_READ, 0, TRUE);
				TriggerEvent(FD_WRITE, 0, TRUE);
			}
		}
	}
	else if (m_ProxyData.nProxyType==PROXYTYPE_SOCKS5)
	{
		if (m_nProxyOpState==1) //Get respone to initialization message
		{
			if (!m_pRecvBuffer)
				m_pRecvBuffer=new char[2];
			int numread=ReceiveNext(m_pRecvBuffer+m_nRecvBufferPos,2-m_nRecvBufferPos);
			if (numread==SOCKET_ERROR)
			{
				if (WSAGetLastError()!=WSAEWOULDBLOCK)
				{
					DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_REQUESTFAILED, 0);
					TriggerEvent((m_nProxyOpID == PROXYOP_CONNECT)?FD_CONNECT:FD_ACCEPT, WSAGetLastError(), TRUE);// X: [CI] - [Code Improvement]
					Reset();
				}
				return;
			}
			m_nRecvBufferPos+=numread;
			if (m_nRecvBufferPos==2)
			{

				if (m_pRecvBuffer[0]!=5)
				{
					DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_REQUESTFAILED, 0);
					TriggerEvent((m_nProxyOpID == PROXYOP_CONNECT)?FD_CONNECT:FD_ACCEPT, WSAECONNABORTED, TRUE);// X: [CI] - [Code Improvement]
					Reset();
					ClearBuffer();
					return;
				}
				if (m_pRecvBuffer[1])
				{ //Auth needed
					if (m_pRecvBuffer[1]!=2)
					{ //Unknown auth type
						DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_AUTHTYPEUNKNOWN, 0);
						TriggerEvent((m_nProxyOpID == PROXYOP_CONNECT)?FD_CONNECT:FD_ACCEPT, WSAECONNABORTED, TRUE);// X: [CI] - [Code Improvement]
						Reset();
						ClearBuffer();
						return;
					}

					if (!m_ProxyData.bUseLogon)
					{
						DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_AUTHNOLOGON, 0);
						TriggerEvent((m_nProxyOpID == PROXYOP_CONNECT)?FD_CONNECT:FD_ACCEPT, WSAECONNABORTED, TRUE);// X: [CI] - [Code Improvement]
						Reset();
						ClearBuffer();
						return;
					}
					//Send authentication
					LPCSTR lpszAsciiUser = m_ProxyData.pProxyUser;
					LPCSTR lpszAsciiPass = m_ProxyData.pProxyPass;
					size_t nLenUser = lpszAsciiUser ? strlen(lpszAsciiUser) : 0;
					size_t nLenPass = lpszAsciiPass ? strlen(lpszAsciiPass) : 0;
					ASSERT(nLenUser<=255);
					ASSERT(nLenPass<=255);
					unsigned char *buffer = new unsigned char[3 + nLenUser + nLenPass];
					buffer[0]=1;
					buffer[1]=static_cast<unsigned char>(nLenUser);
					if (nLenUser)
						strncpy((char*)buffer + 2, lpszAsciiUser, nLenUser);
					buffer[2+nLenUser]=static_cast<unsigned char>(nLenPass);
					if (nLenPass)
						strncpy((char*)buffer + 3 + nLenUser, lpszAsciiPass, nLenPass);
					int nBufLen = 3 + static_cast<int>(nLenUser + nLenPass);
					int res=SendNext(buffer, nBufLen);
					delete [] buffer;
					if (res==SOCKET_ERROR || res<nBufLen)
					{
						if ((WSAGetLastError()!=WSAEWOULDBLOCK) || res<nBufLen)
						{
							DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_REQUESTFAILED, 0);
							TriggerEvent((m_nProxyOpID == PROXYOP_CONNECT)?FD_CONNECT:FD_ACCEPT, WSAGetLastError(), TRUE);// X: [CI] - [Code Improvement]
							Reset();
							return;
						}
					}
					ClearBuffer();
					m_nProxyOpState++;
					return;
				}
			}
			//No auth needed
			//Send connection request
			char *lpszAsciiHost = m_pProxyPeerHost?m_pProxyPeerHost:"";
			size_t nlen = strlen(lpszAsciiHost);
			char *command=new char[10+nlen+1];
			memset(command,0,10+nlen+1);
			command[0]=5;
			command[1]=(m_nProxyOpID==PROXYOP_CONNECT)?1:2;
			//command[2]=0;
			command[3]=m_nProxyPeerIp?1:3;
			int nBufLen=4;
			if (m_nProxyPeerIp)
			{
				memcpy(&command[nBufLen],&m_nProxyPeerIp,4);
				nBufLen+=4;
			}
			else
			{
				command[nBufLen]=static_cast<char>(nlen);
				strncpy(&command[++nBufLen], lpszAsciiHost, nlen);
				nBufLen += (int) nlen;
			}
			memcpy(&command[nBufLen], &m_nProxyPeerPort, 2);
			nBufLen+=2;
			int res=SendNext(command,nBufLen);
			delete [] command;
			if (res==SOCKET_ERROR || res<nBufLen)
			{
				if ( ( WSAGetLastError()!=WSAEWOULDBLOCK) || res<nBufLen)
				{
					DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_REQUESTFAILED, 0);
					TriggerEvent((m_nProxyOpID == PROXYOP_CONNECT)?FD_CONNECT:FD_ACCEPT, WSAGetLastError(), TRUE);// X: [CI] - [Code Improvement]
					Reset();
					return;
				}
			}
			m_nProxyOpState+=2;
			ClearBuffer();
			return;
		}
		else if (m_nProxyOpState==2)
		{//Response to the auth request
			if (!m_pRecvBuffer)
				m_pRecvBuffer=new char[2];
			int numread=ReceiveNext(m_pRecvBuffer+m_nRecvBufferPos, 2-m_nRecvBufferPos);
			if (numread==SOCKET_ERROR)
			{
				if (WSAGetLastError()!=WSAEWOULDBLOCK)
				{
					DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_REQUESTFAILED, 0);
					TriggerEvent((m_nProxyOpID == PROXYOP_CONNECT)?FD_CONNECT:FD_ACCEPT, WSAGetLastError(), TRUE);// X: [CI] - [Code Improvement]
					Reset();
				}
				return;
			}
			m_nRecvBufferPos+=numread;
			if (m_nRecvBufferPos==2)
			{
				if (m_pRecvBuffer[1]!=0)
				{
					DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_AUTHFAILED, 0);
					TriggerEvent((m_nProxyOpID == PROXYOP_CONNECT)?FD_CONNECT:FD_ACCEPT, WSAECONNABORTED, TRUE);// X: [CI] - [Code Improvement]
					Reset();
					ClearBuffer();
					return;
				}
				char * lpszAsciiHost = m_pProxyPeerHost?m_pProxyPeerHost:"";
				size_t nlen = strlen(lpszAsciiHost);
				char *command = new char[10+nlen+1];
				memset(command,0,10+nlen+1);
				command[0]=5;
				command[1]=(m_nProxyOpID==PROXYOP_CONNECT)?1:2;
				//command[2]=0;
				command[3]=m_nProxyPeerIp?1:3;
				int nBufLen=4;
				if (m_nProxyPeerIp)
				{
					memcpy(&command[nBufLen],&m_nProxyPeerIp,4);
					nBufLen+=4;
				}
				else
				{
					command[nBufLen]=static_cast<char>(nlen);
					strncpy(&command[++nBufLen], lpszAsciiHost, nlen);
					nBufLen+= (int) nlen;
				}
				memcpy(&command[nBufLen],&m_nProxyPeerPort,2);
				nBufLen+=2;
				int res=SendNext(command,nBufLen);
				delete [] command;
				if (res==SOCKET_ERROR || res<nBufLen)
				{
					if ((WSAGetLastError()!=WSAEWOULDBLOCK) || res<nBufLen)
					{
						DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_REQUESTFAILED, 0);
						TriggerEvent((m_nProxyOpID == PROXYOP_CONNECT)?FD_CONNECT:FD_ACCEPT, WSAGetLastError(), TRUE);// X: [CI] - [Code Improvement]
						Reset();
						return;
					}
				}
				m_nProxyOpState++;
				ClearBuffer();
				return;
			}
		}
		else if (m_nProxyOpState==3)
		{//Response to the connection request
			if (!m_pRecvBuffer)
			{
				m_pRecvBuffer=new char[10];
				m_nRecvBufferLen=5;
			}
			int numread=ReceiveNext(m_pRecvBuffer+m_nRecvBufferPos,m_nRecvBufferLen-m_nRecvBufferPos);
			if (numread==SOCKET_ERROR)
			{
				if (WSAGetLastError()!=WSAEWOULDBLOCK)
				{
					DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_REQUESTFAILED, 0);
					TriggerEvent((m_nProxyOpID == PROXYOP_CONNECT)?FD_CONNECT:FD_ACCEPT, WSAGetLastError(), TRUE);// X: [CI] - [Code Improvement]
					Reset();
				}
				return;
			}
			m_nRecvBufferPos+=numread;
			if (m_nRecvBufferPos==m_nRecvBufferLen)
			{
				//Check for errors
				if (m_pRecvBuffer[1]!=0 || m_pRecvBuffer[0]!=5)
				{
					DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_REQUESTFAILED, 0);
					TriggerEvent((m_nProxyOpID == PROXYOP_CONNECT)?FD_CONNECT:FD_ACCEPT, WSAECONNABORTED, TRUE);// X: [CI] - [Code Improvement]
					Reset();
					ClearBuffer();
					return;
				}
				if (m_nRecvBufferLen==5)
				{ //Check which kind of address the response contains
					if (m_pRecvBuffer[3]==1)
						m_nRecvBufferLen=10;
					else
					{
						char *tmp=new char[m_nRecvBufferLen+=m_pRecvBuffer[4]+2];
						memcpy(tmp,m_pRecvBuffer,5);
						delete [] m_pRecvBuffer;
						m_pRecvBuffer=tmp;
						m_nRecvBufferLen+=m_pRecvBuffer[4]+2;
					}
					return;
				}

				if (m_nProxyOpID==PROXYOP_CONNECT)
				{
					//OK, we are connected with the remote server
					Reset();
					ClearBuffer();
					TriggerEvent(FD_CONNECT, 0, TRUE);
					TriggerEvent(FD_READ, 0, TRUE);
					TriggerEvent(FD_WRITE, 0, TRUE);
				}
				else
				{
					//Listen socket created
					m_nProxyOpState++;
					unsigned long ip;
					unsigned short port;
					ASSERT(m_pRecvBuffer[3]==1);
					memcpy(&ip, &m_pRecvBuffer[4], 4);
					memcpy(&port, &m_pRecvBuffer[8], 2);
					t_ListenSocketCreatedStruct data;
					data.ip=ip;
					data.nPort=port;
					DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYSTATUS_LISTENSOCKETCREATED, (INT_PTR) &data);
				}
				ClearBuffer();
			}
		}
		else if (m_nProxyOpState==4)
		{
			if (!m_pRecvBuffer)
				m_pRecvBuffer=new char[10];
			int numread=ReceiveNext(m_pRecvBuffer+m_nRecvBufferPos,10-m_nRecvBufferPos);
			if (numread==SOCKET_ERROR)
			{
				if (WSAGetLastError()!=WSAEWOULDBLOCK)
				{
					DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_REQUESTFAILED, 0);
					TriggerEvent((m_nProxyOpID == PROXYOP_CONNECT)?FD_CONNECT:FD_ACCEPT, WSAGetLastError(), TRUE);// X: [CI] - [Code Improvement]
					Reset();
				}
				return;
			}
			m_nRecvBufferPos+=numread;
			if (m_nRecvBufferPos==10)
			{
				if (m_pRecvBuffer[1]!=0)
				{
					DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_REQUESTFAILED, 0);
					if (m_nProxyOpID==PROXYOP_CONNECT)
						TriggerEvent(FD_CONNECT, WSAECONNABORTED, TRUE);
					else
					{
						VERIFY(m_nProxyOpID==PROXYOP_LISTEN);
						TriggerEvent(FD_ACCEPT, WSAECONNABORTED, TRUE);
					}
					Reset();
					ClearBuffer();
					return;
				}
				//Connection to remote server established
				ClearBuffer();
				Reset();
				TriggerEvent(FD_ACCEPT, 0, TRUE);
				TriggerEvent(FD_READ, 0, TRUE);
				TriggerEvent(FD_WRITE, 0, TRUE);
			}
		}
	}
	if (m_ProxyData.nProxyType==PROXYTYPE_HTTP11)
	{
		ASSERT (m_nProxyOpID==PROXYOP_CONNECT);
		char buffer[9];
		for(;;)
		{
			memset(buffer, 0, 9);
			int numread = ReceiveNext(buffer, m_pStrBuffer?1:8);
			if (numread==SOCKET_ERROR)
			{
				int nError=WSAGetLastError();
				if (nError!=WSAEWOULDBLOCK)
				{
					DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_REQUESTFAILED, 0);
					Reset();
					ClearBuffer();
					TriggerEvent(FD_CONNECT, nError, TRUE );
				}
				return;
			}
			//Response begins with HTTP/
			size_t nLen1 = strlen(buffer) + 1;
			if (!m_pStrBuffer)
			{
				m_pStrBuffer = new char[nLen1];
				strcpy_s(m_pStrBuffer, nLen1, buffer);
			}
			else
			{
				char *tmp = m_pStrBuffer;
				size_t nBufLen = strlen(tmp) + nLen1;
				m_pStrBuffer = new char[nBufLen];
				strcpy_s(m_pStrBuffer, nBufLen, tmp);
				strcpy_s(m_pStrBuffer + nBufLen - nLen1, nLen1, buffer);
				delete [] tmp;
			}
			const char start[] = "HTTP/";
			if (memcmp(start, m_pStrBuffer, (strlen(start)>strlen(m_pStrBuffer)) ? strlen(m_pStrBuffer) : strlen(start)))
			{
				char* str = new char[_countof("No valid HTTP reponse") + 1];
				strcpy_s(str, _countof("No valid HTTP reponse") + 1, "No valid HTTP reponse");
				DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_REQUESTFAILED, 0, str);
				Reset();
				ClearBuffer();
				TriggerEvent(FD_CONNECT, WSAECONNABORTED, TRUE);
				return;
			}
			char *pos = strstr(m_pStrBuffer, "\r\n");
			if (pos)
			{
				char *pos2 = strstr(m_pStrBuffer, " ");
				if (!pos2 || *(pos2+1)!='2' || pos2>pos)
				{
					char *tmp = new char[pos-m_pStrBuffer + 1];
					strncpy(tmp, m_pStrBuffer, pos-m_pStrBuffer);
					tmp[pos-m_pStrBuffer] = 0;
					DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_REQUESTFAILED, 0, tmp);
					Reset();
					ClearBuffer();
					TriggerEvent(FD_CONNECT, WSAECONNABORTED, TRUE );
					return;
				}
			}
			if (strlen(m_pStrBuffer)>3 && !memcmp(m_pStrBuffer+strlen(m_pStrBuffer)-4, "\r\n\r\n", 4)) //End of the HTTP header
			{
				Reset();
				ClearBuffer();
				TriggerEvent(FD_CONNECT, 0, TRUE);
				TriggerEvent(FD_READ, 0, TRUE);
				TriggerEvent(FD_WRITE, 0, TRUE);
				return;
			}
		}
	}
}

BOOL CAsyncProxySocketLayer::Connect( LPCTSTR lpszHostAddress, UINT nHostPort )
{
	if (!m_ProxyData.nProxyType)
		//Connect normally because there is no proxy
		return ConnectNext(lpszHostAddress, nHostPort);

	//Translate the host address
	ASSERT(lpszHostAddress != NULL);

	if (m_ProxyData.nProxyType != PROXYTYPE_SOCKS4)
	{
		// We can send hostname to proxy, no need to resolve it
		
		//Connect to proxy server
		BOOL res = ConnectNext(CA2CT(m_ProxyData.pProxyHost), m_ProxyData.nProxyPort);
		if (!res)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_NOCONN, 0);
				return FALSE;
			}
		}
		m_nProxyPeerPort = htons((u_short)nHostPort);
		m_nProxyPeerIp = 0;
		delete [] m_pProxyPeerHost;
		size_t nLen1 = _tcslen(lpszHostAddress)+1;
		m_pProxyPeerHost = new char[nLen1];
		strcpy_s(m_pProxyPeerHost, nLen1, CT2CA(lpszHostAddress));
		m_nProxyOpID=PROXYOP_CONNECT;
		return TRUE;
	}

	SOCKADDR_IN sockAddr;
	memset(&sockAddr,0,sizeof(sockAddr));

	//LPCSTR lpszAscii = CT2A((LPTSTR)lpszHostAddress); // X: [BF] - [Bug Fix] not work
	CT2A tmpt2a(lpszHostAddress);
	LPSTR lpszAscii = tmpt2a;
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = inet_addr(lpszAscii);

	if (sockAddr.sin_addr.s_addr == INADDR_NONE)
	{
		LPHOSTENT lphost;
		lphost = gethostbyname(lpszAscii);
		if (lphost != NULL)
			sockAddr.sin_addr.s_addr = ((LPIN_ADDR)lphost->h_addr)->s_addr;
		else
		{
			//Can't resolve hostname
			DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_CANTRESOLVEHOST, 0);
			WSASetLastError(WSAEINVAL);
			return FALSE;
		}
	}

	sockAddr.sin_port = htons((u_short)nHostPort);

	BOOL res = CAsyncProxySocketLayer::Connect((SOCKADDR*)&sockAddr, sizeof(sockAddr));
	if (res || WSAGetLastError() == WSAEWOULDBLOCK)
	{
		delete[] m_pProxyPeerHost;
		//m_pProxyPeerHost = new char[strlen(CT2CA(lpszHostAddress))+1]; // X: [BF] - [Bug Fix] not work
		//strcpy_s(m_pProxyPeerHost, strlen(CT2CA(lpszHostAddress))+1, CT2CA(lpszHostAddress));
		CT2CA tmpt2a(lpszHostAddress);
		size_t nlen1 = strlen(tmpt2a) + 1;
		m_pProxyPeerHost = new char[nlen1];
		strcpy_s(m_pProxyPeerHost, nlen1, tmpt2a);
	}
	return res;
}

BOOL CAsyncProxySocketLayer::Connect(const SOCKADDR* lpSockAddr, int nSockAddrLen)
{
	if (!m_ProxyData.nProxyType)
		//Connect normally because there is no proxy
		return ConnectNext(lpSockAddr, nSockAddrLen);

	LPSOCKADDR_IN sockAddr = (LPSOCKADDR_IN)lpSockAddr;

	//Save server details
	m_nProxyPeerIp = sockAddr->sin_addr.S_un.S_addr;
	m_nProxyPeerPort = sockAddr->sin_port;
	delete[] m_pProxyPeerHost;
	m_pProxyPeerHost = NULL;

	m_nProxyOpID = PROXYOP_CONNECT;

	BOOL res = ConnectNext(CA2T(m_ProxyData.pProxyHost), m_ProxyData.nProxyPort);
	if (!res)
	{
		if (WSAGetLastError()!=WSAEWOULDBLOCK)
		{
			DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_NOCONN, 0);
			return FALSE;
		}
	}

	return res;
}

void CAsyncProxySocketLayer::OnConnect(int nErrorCode)
{
	if (m_ProxyData.nProxyType==PROXYTYPE_NOPROXY)
	{
		TriggerEvent(FD_CONNECT, nErrorCode, TRUE);
		return;
	}
	ASSERT(m_nProxyOpID);
	if (!m_nProxyOpID)
	{
		//This should not happen
		return;
	}

	if (nErrorCode)
	{ //Can't connect to proxy
		DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_NOCONN, 0);
		TriggerEvent((m_nProxyOpID == PROXYOP_CONNECT)?FD_CONNECT:FD_ACCEPT, nErrorCode, TRUE);// X: [CI] - [Code Improvement]
		Reset();
		ClearBuffer();
		return;
	}
	if (m_nProxyOpID==PROXYOP_CONNECT || m_nProxyOpID==PROXYOP_LISTEN)
	{
		if (m_nProxyOpState)
			//Somehow OnConnect has been called more than once
			return;
		ASSERT(m_ProxyData.nProxyType!=PROXYTYPE_NOPROXY);
		ClearBuffer();
		//Send the initial request
		if (m_ProxyData.nProxyType==PROXYTYPE_SOCKS4 || m_ProxyData.nProxyType==PROXYTYPE_SOCKS4A)
		{ //SOCKS4 proxy
			//Send request
			LPCSTR lpszAscii = m_pProxyPeerHost?m_pProxyPeerHost:"";
			size_t nLen1 = strlen(lpszAscii) + 1;
			char *command=new char [9+nLen1];
			memset(command,0,9+nLen1);
			int nBufLen=9;
			command[0]=4;
			command[1]=(m_nProxyOpID==PROXYOP_CONNECT)?1:2; //CONNECT or BIND request
			memcpy(&command[2],&m_nProxyPeerPort,2); //Copy target address
			if (!m_nProxyPeerIp || m_ProxyData.nProxyType==PROXYTYPE_SOCKS4A)
			{
				ASSERT(m_ProxyData.nProxyType==PROXYTYPE_SOCKS4A);
				ASSERT(strcmp(lpszAscii, ""));
				//Set the IP to 0.0.0.x (x is nonzero)
				//command[4]=0;
				//command[5]=0;
				//command[6]=0;
				command[7]=1;
				//command[8]=0;	// Terminating NUL-byte for USERID
				//Add host as URL
				strcpy_s(&command[9], nLen1,lpszAscii);
				nBufLen+= (int) nLen1;
			}
			else
				memcpy(&command[4],&m_nProxyPeerIp,4);
			int res=SendNext(command,nBufLen); //Send command
			delete [] command;
				int nErrorCode = WSAGetLastError();
			if (res==SOCKET_ERROR)//nErrorCode!=WSAEWOULDBLOCK)
			{
				DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_REQUESTFAILED, 0);
				if (m_nProxyOpID == PROXYOP_CONNECT)
					TriggerEvent(FD_CONNECT, (nErrorCode == WSAEWOULDBLOCK) ? WSAECONNABORTED : nErrorCode, TRUE);
				else
					TriggerEvent(FD_ACCEPT, nErrorCode, TRUE);
				Reset();
				ClearBuffer();
				return;
			}
			else if (res<nBufLen)
			{
				DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_REQUESTFAILED, 0);
				TriggerEvent((m_nProxyOpID == PROXYOP_CONNECT)?FD_CONNECT:FD_ACCEPT, WSAECONNABORTED, TRUE);// X: [CI] - [Code Improvement]
				Reset();
				ClearBuffer();
				return;
			}
		}
		else if (m_ProxyData.nProxyType==PROXYTYPE_SOCKS5)
		{ //SOCKS5 proxy
			//Send initialization request
			unsigned char command[10];
			memset(command,0,10);
			command[0]=5;
			//CAsyncProxySocketLayer supports to logon types: No logon and
			//cleartext username/password (if set) logon
			command[1]=m_ProxyData.bUseLogon?2:1; //Number of logon types
			command[2]=m_ProxyData.bUseLogon?2:0; //2=user/pass, 0=no logon
			int nBufLen=m_ProxyData.bUseLogon?4:3; //length of request
			int res=SendNext(command,nBufLen);

			int nErrorCode=WSAGetLastError();
			if (res==SOCKET_ERROR)//nErrorCode!=WSAEWOULDBLOCK)
			{
				DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_REQUESTFAILED, 0);
				if (m_nProxyOpID==PROXYOP_CONNECT)
					TriggerEvent(FD_CONNECT, (nErrorCode==WSAEWOULDBLOCK)?WSAECONNABORTED:nErrorCode, TRUE);
				else
					TriggerEvent(FD_ACCEPT, nErrorCode, TRUE);
				Reset();
				ClearBuffer();
				return;
			}
			else if (res<nBufLen)
			{
				DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_REQUESTFAILED, 0);
				TriggerEvent((m_nProxyOpID == PROXYOP_CONNECT)?FD_CONNECT:FD_ACCEPT, WSAECONNABORTED, TRUE);// X: [CI] - [Code Improvement]
				Reset();
				ClearBuffer();
				return;
			}
		}
		else if (m_ProxyData.nProxyType==PROXYTYPE_HTTP11)
		{
			char str[4096]; //This should be large enough

			char * pHost = NULL;
			if (m_pProxyPeerHost && *m_pProxyPeerHost)
			{
				size_t nLen1 = strlen(m_pProxyPeerHost) + 1;
				pHost = new char[nLen1];
				strcpy_s(pHost, nLen1, m_pProxyPeerHost);
			}
			else
			{
				pHost = new char[16];
				sprintf_s(pHost, 16, "%d.%d.%d.%d", m_nProxyPeerIp & 0xff, (m_nProxyPeerIp>>8) & 0xff, (m_nProxyPeerIp>>16) & 0xff, m_nProxyPeerIp>>24);
			}
			if (!m_ProxyData.bUseLogon)
				sprintf_s(str, _countof(str), "CONNECT %s:%d HTTP/1.1\r\nHost: %s:%d\r\n\r\n", pHost, ntohs(m_nProxyPeerPort),
					pHost, ntohs(m_nProxyPeerPort));
			else
			{
				sprintf_s(str, _countof(str), "CONNECT %s:%d HTTP/1.1\r\nHost: %s:%d\r\n", pHost, ntohs(m_nProxyPeerPort),
					pHost, ntohs(m_nProxyPeerPort));

				char userpass[4096];
				sprintf_s(userpass, _countof(userpass), "%s:%s", m_ProxyData.pProxyUser?m_ProxyData.pProxyUser:"", m_ProxyData.pProxyPass?m_ProxyData.pProxyPass:"");
				char base64str[4096];

				CBase64Coding base64coding;
				if (!base64coding.Encode(userpass, strlen(userpass), base64str))
				{
					DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_REQUESTFAILED, 0);
					TriggerEvent((m_nProxyOpID == PROXYOP_CONNECT)?FD_CONNECT:FD_ACCEPT, WSAECONNABORTED, TRUE);// X: [CI] - [Code Improvement]
					Reset();
					ClearBuffer();
					delete [] pHost;
					return;
				}
				strcat_s(str, _countof(str), "Authorization: Basic ");
				strcat_s(str, _countof(str), base64str);
				strcat_s(str, _countof(str), "\r\nProxy-Authorization: Basic ");
				strcat_s(str, _countof(str), base64str);
				strcat_s(str, _countof(str), "\r\n\r\n");
			}
			delete [] pHost;

			int numsent=SendNext(str, (int) strlen(str) );
			int nErrorCode=WSAGetLastError();
			if (numsent==SOCKET_ERROR)//nErrorCode!=WSAEWOULDBLOCK)
			{
				DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_REQUESTFAILED, 0);
				if (m_nProxyOpID == PROXYOP_CONNECT)
					TriggerEvent(FD_CONNECT, (nErrorCode == WSAEWOULDBLOCK) ? WSAECONNABORTED : nErrorCode, TRUE);
				else
					TriggerEvent(FD_ACCEPT, nErrorCode, TRUE);
				Reset();
				ClearBuffer();
				return;
			}
			else if (  numsent < static_cast<int>( strlen(str) )  )
			{
				DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_REQUESTFAILED, 0);
				if (m_nProxyOpID == PROXYOP_CONNECT)
					TriggerEvent(FD_CONNECT, WSAECONNABORTED, TRUE);
				else
					TriggerEvent(FD_ACCEPT, WSAECONNABORTED, TRUE);
				Reset();
				ClearBuffer();
				return;
			}
			m_nProxyOpState++;
			return;
		}
		else
			ASSERT(FALSE);
		//Now we'll wait for the response, handled in OnReceive
		m_nProxyOpState++;
	}
}

void CAsyncProxySocketLayer::ClearBuffer()
{
	delete [] m_pStrBuffer;
	m_pStrBuffer = NULL;
	if (m_pRecvBuffer)
	{
		delete[] m_pRecvBuffer;
		m_pRecvBuffer = 0;
	}
	m_nRecvBufferLen = 0;
	m_nRecvBufferPos = 0;
}

BOOL CAsyncProxySocketLayer::Listen(int nConnectionBacklog)
{
	if (GetProxyType() == PROXYTYPE_NOPROXY)
		return ListenNext(nConnectionBacklog);

	//Connect to proxy server
	BOOL res = ConnectNext(CA2T(m_ProxyData.pProxyHost), m_ProxyData.nProxyPort);
	if (!res)
	{
		if (WSAGetLastError()!=WSAEWOULDBLOCK)
		{
			DoLayerCallback(LAYERCALLBACK_LAYERSPECIFIC, PROXYERROR_NOCONN, 0);
			return FALSE;
		}
	}
	m_nProxyPeerPort=0;
	m_nProxyPeerIp=(unsigned int)nConnectionBacklog;

	m_nProxyOpID=PROXYOP_LISTEN;
	return TRUE;
}

#ifdef _AFX
BOOL CAsyncProxySocketLayer::GetPeerName(CString &rPeerAddress, UINT &rPeerPort)
{
	if (m_ProxyData.nProxyType == PROXYTYPE_NOPROXY)
		return GetPeerNameNext(rPeerAddress, rPeerPort);

	if (GetLayerState() == notsock)
	{
		WSASetLastError(WSAENOTSOCK);
		return FALSE;
	}
	else if (GetLayerState() != connected)
	{
		WSASetLastError(WSAENOTCONN);
		return FALSE;
	}
	else if (!m_nProxyPeerIp || !m_nProxyPeerPort)
	{
		WSASetLastError(WSAENOTCONN);
		return FALSE;
	}

	ASSERT( m_ProxyData.nProxyType );
	BOOL res = GetPeerNameNext(rPeerAddress, rPeerPort);
	if (res)
	{
		rPeerPort=ntohs(m_nProxyPeerPort);
		rPeerAddress.Format(_T("%u.%u.%u.%u"), m_nProxyPeerIp&0xff, (m_nProxyPeerIp>>8)&0xff, (m_nProxyPeerIp>>16)&0xff, m_nProxyPeerIp>>24);
	}
	return res;
}
#endif

BOOL CAsyncProxySocketLayer::GetPeerName(SOCKADDR* lpSockAddr, int* lpSockAddrLen)
{
	if (m_ProxyData.nProxyType == PROXYTYPE_NOPROXY)
		return GetPeerNameNext(lpSockAddr, lpSockAddrLen);

	if (GetLayerState() == notsock)
	{
		WSASetLastError(WSAENOTSOCK);
		return FALSE;
	}
	else if (GetLayerState() != connected)
	{
		WSASetLastError(WSAENOTCONN);
		return FALSE;
	}
	else if (!m_nProxyPeerIp || !m_nProxyPeerPort)
	{
		WSASetLastError(WSAENOTCONN);
		return FALSE;
	}

	ASSERT( m_ProxyData.nProxyType );
	BOOL res = GetPeerNameNext(lpSockAddr, lpSockAddrLen);
	if (res)
	{
		LPSOCKADDR_IN addr = (LPSOCKADDR_IN)lpSockAddr;
		addr->sin_port = m_nProxyPeerPort;
		addr->sin_addr.S_un.S_addr = m_nProxyPeerIp;
	}
	return res;
}

int CAsyncProxySocketLayer::GetProxyType() const
{
	return m_ProxyData.nProxyType;
}

void CAsyncProxySocketLayer::Close()
{
	delete [] m_ProxyData.pProxyHost;
	delete [] m_ProxyData.pProxyUser;
	delete [] m_ProxyData.pProxyPass;
	delete [] m_pProxyPeerHost;
	m_ProxyData.pProxyHost = NULL;
	m_ProxyData.pProxyUser = NULL;
	m_ProxyData.pProxyPass = NULL;
	m_pProxyPeerHost = NULL;
	ClearBuffer();
	Reset();
	CloseNext();
}

void CAsyncProxySocketLayer::Reset()
{
	m_nProxyOpState = 0;
	m_nProxyOpID = 0;
}

int CAsyncProxySocketLayer::Send(const void* lpBuf, int nBufLen, int nFlags)
{
	if (m_nProxyOpID)
	{
		WSASetLastError(WSAEWOULDBLOCK);
		return SOCKET_ERROR;
	}

	return SendNext(lpBuf, nBufLen, nFlags);
}
// X: [SUDPS] - [CAsyncSocketEx UDP Support]
int CAsyncProxySocketLayer::SendTo(const void* lpBuf, int nBufLen, const SOCKADDR* lpSockAddr, int nSockAddrLen, int nFlags)
{
	if (m_nProxyOpID)
	{
		WSASetLastError(WSAEWOULDBLOCK);
		return SOCKET_ERROR;
	}

	return SendToNext(lpBuf, nBufLen, lpSockAddr, nSockAddrLen, nFlags);
}

int CAsyncProxySocketLayer::Receive(void* lpBuf, int nBufLen, int nFlags)
{
	if (m_nProxyOpID)
	{
		WSASetLastError(WSAEWOULDBLOCK);
		return SOCKET_ERROR;
	}

	return ReceiveNext(lpBuf, nBufLen, nFlags);
}
// X: [SUDPS] - [CAsyncSocketEx UDP Support]
int CAsyncProxySocketLayer::ReceiveFrom(void* lpBuf, int nBufLen, SOCKADDR* lpSockAddr, int* nSockAddrLen, int nFlags)
{
	if (m_nProxyOpID)
	{
		WSASetLastError(WSAEWOULDBLOCK);
		return SOCKET_ERROR;
	}

	return ReceiveFromNext(lpBuf, nBufLen, lpSockAddr, nSockAddrLen, nFlags);
}

BOOL CAsyncProxySocketLayer::PrepareListen(unsigned long ip)
{
	if (GetLayerState()!=notsock && GetLayerState()!=unconnected)
		return FALSE;
	m_nProxyPeerIp=ip;
	return TRUE;
}

BOOL CAsyncProxySocketLayer::Accept( CAsyncSocketEx& rConnectedSocket, SOCKADDR* lpSockAddr /*=NULL*/, int* lpSockAddrLen /*=NULL*/ )
{
	if (!m_ProxyData.nProxyType)
		return AcceptNext(rConnectedSocket, lpSockAddr, lpSockAddrLen);

	GetPeerName(lpSockAddr, lpSockAddrLen);
	return TRUE;
}

CString GetProxyError(UINT nError)
{
	switch(nError)// X: [CI] - [Code Improvement]
	{
		case PROXYERROR_NOERROR:
			return _CString(_T("No proxy error"));
		case PROXYERROR_NOCONN:
			return _CString(_T("Proxy connection failed"));
		case PROXYERROR_REQUESTFAILED:
			return _CString(_T("Proxy request failed"));
		case PROXYERROR_AUTHREQUIRED:
			return _CString(_T("Proxy authentication required"));
		case PROXYERROR_AUTHTYPEUNKNOWN:
			return _CString(_T("Proxy authentication not supported"));
		case PROXYERROR_AUTHFAILED:
			return _CString(_T("Proxy authentication failed"));
		case PROXYERROR_AUTHNOLOGON:
			return _CString(_T("Proxy authentication required"));
		case PROXYERROR_CANTRESOLVEHOST:
			return _CString(_T("Proxy hostname not resolved"));
		case PROXYSTATUS_LISTENSOCKETCREATED:
			return _CString(_T("Proxy listen socket created"));
		default:{
			CString strError;
			strError.Format(_T("Proxy-Error: %u"), nError);
			return strError;
		}
	}
}
