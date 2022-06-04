//emulEspaña - Copied from WebSocket.cpp and then modified by MoNKi [MoNKi: -Wap Server-]
// ws2.cpp : Defines the entry point for the application.
//

#include <stdafx.h> 

#include "emule.h"
#include "OtherFunctions.h"
#include "WapSocket.h"
#include "WapServer.h"
#include "Preferences.h"
#include "StringConversion.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


static HANDLE s_hWapTerminate = NULL;
static CWinThread* s_pWapSocketThread = NULL;

typedef struct
{
	void	*pThis;
	SOCKET	hSocket;
	in_addr incomingaddr;
} SocketData;

void CWapSocket::SetParent(CWapServer *pParent)
{
	m_pParent = pParent;
}

void CWapSocket::OnRequestReceived(char* pHeader, DWORD dwHeaderLen, char* pData, DWORD dwDataLen, in_addr inad)
{
	CString sHeader(pHeader, dwHeaderLen);
	CString sData(pData, dwDataLen);
	CString sURL;
	bool filereq=false;

	if(sHeader.Left(3) == "GET")
		sURL = sHeader.Trim();

	else if(sHeader.Left(4) == "POST")
		sURL = _T("?") + sData.Trim();	// '?' to imitate GET syntax for ParseURL

	if(sURL.Find(_T(" ")) > -1)
		sURL = sURL.Mid(sURL.Find(_T(" "))+1, sURL.GetLength());
	if(sURL.Find(_T(" ")) > -1)
		sURL = sURL.Left(sURL.Find(_T(" ")));

	if (sURL.GetLength()>5 && sURL.Right(5).MakeLower()==".wbmp" || sURL.Right(5).MakeLower()==".wmls"
		|| sURL.Right(4).MakeLower()==".png" || sURL.Right(4).MakeLower()==".gif")
		filereq=true;

	WapThreadData Data;
	Data.sURL = URLDecode(sURL);
	Data.pThis = m_pParent;
	Data.inadr = inad;
	Data.pSocket = this;
	
	if (!filereq)
		m_pParent->ProcessURL(Data);
	else
		m_pParent->ProcessFileReq(Data);

	Disconnect();
}

void CWapSocket::OnReceived(void* pData, DWORD dwSize, in_addr inad)
{
	const UINT SIZE_PRESERVE = 0x1000;

	if (m_dwBufSize < dwSize + m_dwRecv)
	{
		// reallocate
		char* pNewBuf = new char[m_dwBufSize = dwSize + m_dwRecv + SIZE_PRESERVE];
		if (!pNewBuf)
		{
			m_bValid = false; // internal problem
			return;
		}

		if (m_pBuf)
		{
			CopyMemory(pNewBuf, m_pBuf, m_dwRecv);
			delete[] m_pBuf;
		}

		m_pBuf = pNewBuf;
	}
	CopyMemory(m_pBuf + m_dwRecv, pData, dwSize);
	m_dwRecv += dwSize;

	// check if we have all that we want
	if (!m_dwHttpHeaderLen)
	{
		// try to find it
		bool bPrevEndl = false;
		for (DWORD dwPos = 0; dwPos < m_dwRecv; dwPos++)
			if ('\n' == m_pBuf[dwPos])
				if (bPrevEndl)
				{
					// We just found the end of the http header
					// Now write the message's position into two first DWORDs of the buffer
					m_dwHttpHeaderLen = dwPos + 1;

					// try to find now the 'Content-Length' header
					for (dwPos = 0; dwPos < m_dwHttpHeaderLen; )
					{
						// Elandal: pPtr is actually a char*, not a void*
						char* pPtr = (char*)memchr(m_pBuf + dwPos, '\n', m_dwHttpHeaderLen - dwPos);
						if (!pPtr)
							break;
						// Elandal: And thus now the pointer substraction works as it should
						DWORD dwNextPos = pPtr - m_pBuf;

						// check this header
						char szMatch[] = "content-length";
						if (!strnicmp(m_pBuf + dwPos, szMatch, sizeof(szMatch) - 1))
						{
							dwPos += sizeof(szMatch) - 1;
							pPtr = (char*)memchr(m_pBuf + dwPos, ':', m_dwHttpHeaderLen - dwPos);
							if (pPtr)
								m_dwHttpContentLen = atol((pPtr) + 1);

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
				if ('\r' != m_pBuf[dwPos])
					bPrevEndl = false;

	}

	if (m_dwHttpHeaderLen && !m_bCanRecv && !m_dwHttpContentLen)
		m_dwHttpContentLen = m_dwRecv - m_dwHttpHeaderLen; // of course

	if (m_dwHttpHeaderLen && (!m_dwHttpContentLen || (m_dwHttpHeaderLen + m_dwHttpContentLen <= m_dwRecv)))
	{
		OnRequestReceived(m_pBuf, m_dwHttpHeaderLen, m_pBuf + m_dwHttpHeaderLen, m_dwHttpContentLen, inad);

		if (m_bCanRecv && (m_dwRecv > m_dwHttpHeaderLen + m_dwHttpContentLen))
		{
			// move our data
			m_dwRecv -= m_dwHttpHeaderLen + m_dwHttpContentLen;
			MoveMemory(m_pBuf, m_pBuf + m_dwHttpHeaderLen + m_dwHttpContentLen, m_dwRecv);
		} else
			m_dwRecv = 0;

		m_dwHttpHeaderLen = 0;
		m_dwHttpContentLen = 0;
	}

}

void CWapSocket::SendData(const void* pData, DWORD dwDataSize)
{
	ASSERT(pData);
	if (m_bValid && m_bCanSend)
	{
		if (!m_pHead)
		{
			// try to send it directly
			//-- remember: in "nRes" could be "-1" after "send" call
			int nRes = send(m_hSocket, (const char*) pData, dwDataSize, 0);

			if (((nRes < 0) || (nRes > (signed) dwDataSize)) && (WSAEWOULDBLOCK != WSAGetLastError()))
			{
				m_bValid = false;
			}
			else
			{                
				//-- in nRes still could be "-1" (if WSAEWOULDBLOCK occured)
				//-- next to line should be like this:

				((const char*&) pData) += (nRes == -1 ? 0 : nRes);
				dwDataSize -= (nRes == -1 ? 0 : nRes);

				//-- ... and not like this:
				//-- ((const char*&) pData) += nRes;
				//-- dwDataSize -= nRes;

			}
		}

		if (dwDataSize && m_bValid)
		{
			// push it to our tails
			CChunk* pChunk = new CChunk;
			if (pChunk)
			{
				pChunk->m_pNext = NULL;
				pChunk->m_dwSize = dwDataSize;
				if ((pChunk->m_pData = new char[dwDataSize]) != NULL)
				{
					//-- data should be copied into "pChunk->m_pData" anyhow
					//-- possible solution is simple:

					CopyMemory(pChunk->m_pData, pData, dwDataSize);

					// push it to the end of our queue
					pChunk->m_pToSend = pChunk->m_pData;
					if (m_pTail)
						m_pTail->m_pNext = pChunk;
					else
						m_pHead = pChunk;
					m_pTail = pChunk;

				} else
					delete pChunk; // oops, no memory (???)
			}
		}
	}

}

void CWapSocket::SendContent(LPCSTR szStdResponse, const void* pContent, DWORD dwContentSize)
{
	char szBuf[0x1000];
	int nLen = wsprintfA(szBuf, "HTTP/1.1 200 OK\r\n%sContent-Length: %ld\r\n\r\n", szStdResponse, dwContentSize);
	SendData(szBuf, nLen);
	SendData(pContent, dwContentSize);
}

void CWapSocket::SendContent(LPCSTR szStdResponse, const CString& rstr)
{
	CStringA strA(wc2utf8(rstr));
	SendContent(szStdResponse, strA, strA.GetLength());
}

void CWapSocket::SendError404(bool sendInfo)
{
	char szBuf[0x1000];
	CString Out;
	if (sendInfo){
		Out="<?xml version=\"1.0\"?>"
			"<!DOCTYPE wml PUBLIC \"-//WAPFORUM//DTD WML 1.1//EN\" \"http://www.wapforum.org/DTD/wml_1.1.xml\">"
			"<wml><card id=\"error\" title=\"HTTP Error 404\"><p>"
			"The eMule Wap Server cannot find the file or script you asked for. Please check the URL to ensure that the path is correct."
			"</p></card></wml>";
	}
	int nLen = wsprintfA(szBuf, "HTTP/1.1 404 Error\r\nServer: eMule\r\nConnection: close\r\nContent-Type: text/vnd.wap.wml\r\nContent-Length: %ld\r\n\r\n", Out.GetLength());
	SendData(szBuf, nLen);
	SendData(Out, Out.GetLength());
}

void CWapSocket::Disconnect()
{
	if (m_bValid && m_bCanSend)
	{
		m_bCanSend = false;
		if (m_pTail)
		{
			// push it as a tail
			CChunk* pChunk = new CChunk;
			if (pChunk)
			{
				pChunk->m_dwSize = 0;
				pChunk->m_pData = NULL;
				pChunk->m_pToSend = NULL;
				pChunk->m_pNext = NULL;

				m_pTail->m_pNext = pChunk;
			}

		} else
			if (shutdown(m_hSocket, SD_SEND))
				m_bValid = false;
	}
}	

UINT AFX_CDECL WapSocketAcceptedFunc(LPVOID pD)
{
	DbgSetThreadName("WapSocketAccepted");
	SocketData *pData = (SocketData *)pD;
	SOCKET hSocket = pData->hSocket;
	CWapServer *pThis = (CWapServer *)pData->pThis;
	in_addr ad=pData->incomingaddr;
	
	delete pData;

	ASSERT(INVALID_SOCKET != hSocket);

	HANDLE hEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (hEvent)
	{
		if (!WSAEventSelect(hSocket, hEvent, FD_READ | FD_CLOSE | FD_WRITE))
		{
			CWapSocket stWapSocket;
			stWapSocket.SetParent(pThis);
			stWapSocket.m_pHead = NULL;
			stWapSocket.m_pTail = NULL;
			stWapSocket.m_bValid = true;
			stWapSocket.m_bCanRecv = true;
			stWapSocket.m_bCanSend = true;
			stWapSocket.m_hSocket = hSocket;
			stWapSocket.m_pBuf = NULL;
			stWapSocket.m_dwRecv = 0;
			stWapSocket.m_dwBufSize = 0;
			stWapSocket.m_dwHttpHeaderLen = 0;
			stWapSocket.m_dwHttpContentLen = 0;

			HANDLE pWait[] = { hEvent, s_hWapTerminate };

			while (WAIT_OBJECT_0 == WaitForMultipleObjects(2, pWait, FALSE, INFINITE))
			{
				while (stWapSocket.m_bValid)
				{
					WSANETWORKEVENTS stEvents;
					if (WSAEnumNetworkEvents(hSocket, NULL, &stEvents))
						stWapSocket.m_bValid = false;
					else
					{
						if (!stEvents.lNetworkEvents)
							break; //no more events till now

						if (FD_READ & stEvents.lNetworkEvents)
							for (;;)
							{
								char pBuf[0x1000];
								int nRes = recv(hSocket, pBuf, sizeof(pBuf), 0);
								if (nRes <= 0)
								{
									if (!nRes)
									{
										stWapSocket.m_bCanRecv = false;
										stWapSocket.OnReceived(NULL, 0, ad);
									}
									else
										if (WSAEWOULDBLOCK != WSAGetLastError())
											stWapSocket.m_bValid = false;
									break;
								}
								stWapSocket.OnReceived(pBuf, nRes,ad);
							}

						if (FD_CLOSE & stEvents.lNetworkEvents)
							stWapSocket.m_bCanRecv = false;

						if (FD_WRITE & stEvents.lNetworkEvents)
							// send what is left in our tails
							while (stWapSocket.m_pHead)
							{
								if (stWapSocket.m_pHead->m_pToSend)
								{
									int nRes = send(hSocket, stWapSocket.m_pHead->m_pToSend, stWapSocket.m_pHead->m_dwSize, 0);
									if (nRes != (signed) stWapSocket.m_pHead->m_dwSize)
									{
										if (nRes)
											if ((nRes > 0) && (nRes < (signed) stWapSocket.m_pHead->m_dwSize))
											{
												stWapSocket.m_pHead->m_pToSend += nRes;
												stWapSocket.m_pHead->m_dwSize -= nRes;

											} else
												if (WSAEWOULDBLOCK != WSAGetLastError())
													stWapSocket.m_bValid = false;
										break;
									}
								} else
									if (shutdown(hSocket, SD_SEND))
									{
										stWapSocket.m_bValid = false;
										break;
									}

								// erase this chunk
								CWapSocket::CChunk* pNext = stWapSocket.m_pHead->m_pNext;
								delete stWapSocket.m_pHead;
								stWapSocket.m_pHead = pNext;
								if (stWapSocket.m_pHead == NULL)
									stWapSocket.m_pTail = NULL;								
							}
					}
				}

				if (!stWapSocket.m_bValid || (!stWapSocket.m_bCanRecv && !stWapSocket.m_pHead))
					break;
			}

			while (stWapSocket.m_pHead)
			{
				CWapSocket::CChunk* pNext = stWapSocket.m_pHead->m_pNext;
				delete stWapSocket.m_pHead;
				stWapSocket.m_pHead = pNext;
			}
			if (stWapSocket.m_pBuf)
				delete[] stWapSocket.m_pBuf;
		}
		VERIFY( CloseHandle(hEvent) );
	}
	VERIFY( !closesocket(hSocket) );

	return 0;
}

UINT AFX_CDECL WapSocketListeningFunc(LPVOID pThis)
{
	DbgSetThreadName("WapSocketListening");
//	WSADATA stData;
//	if (!WSAStartup(MAKEWORD(2, 2), &stData))
	{
		SOCKET hSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
		if (INVALID_SOCKET != hSocket)
		{
			SOCKADDR_IN stAddr;
			stAddr.sin_family = AF_INET;
			stAddr.sin_port = htons(thePrefs.GetWapPort());
			stAddr.sin_addr.S_un.S_addr = INADDR_ANY;

			if (!bind(hSocket, (sockaddr*)&stAddr, sizeof(stAddr)) && !listen(hSocket, 5)) //SOMAXCONN is too much with winsock2
			{
				HANDLE hEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
				if (hEvent)
				{
					if (!WSAEventSelect(hSocket, hEvent, FD_ACCEPT))
					{
						// emulEspaña: Added by MoNKi [MoNKi: -UPnPNAT Support-]
						CUPnP_IGDControlPoint::UPNPNAT_MAPPING mapping;
						BOOL UPnP = false;

						mapping.internalPort = mapping.externalPort = ntohs(stAddr.sin_port);
						mapping.protocol = CUPnP_IGDControlPoint::UNAT_TCP;
						mapping.description = "Wap Interface";
						
						if(thePrefs.IsUPnPNat())
							UPnP = theApp.m_UPnP_IGDControlPoint->AddPortMapping(&mapping);
						// End emulEspaña

						HANDLE pWait[] = { hEvent, s_hWapTerminate };
						while (WAIT_OBJECT_0 == WaitForMultipleObjects(2, pWait, FALSE, INFINITE))
						{
							for (;;)
							{
								struct sockaddr_in their_addr;
                                int sin_size = sizeof(struct sockaddr_in);

								SOCKET hAccepted = accept(hSocket,(struct sockaddr *)&their_addr, &sin_size);
								if (INVALID_SOCKET == hAccepted)
									break;

								if(thePrefs.GetWapServerEnabled())
								{
									SocketData *pData = new SocketData;
									pData->hSocket = hAccepted;
									pData->pThis = pThis;
									pData->incomingaddr=their_addr.sin_addr;
									
									// - do NOT use Windows API 'CreateThread' to create a thread which uses MFC/CRT -> lot of mem leaks!
									// - 'AfxBeginThread' could be used here, but creates a little too much overhead for our needs.
									CWinThread* pAcceptThread = new CWinThread(WapSocketAcceptedFunc, (LPVOID)pData);
									if (!pAcceptThread->CreateThread()){
										delete pAcceptThread;
										pAcceptThread = NULL;
										VERIFY( !closesocket(hSocket) );
									}
								}
								else
									VERIFY( !closesocket(hSocket) );
							}
						}
						
						// emulEspaña: Added by MoNKi [MoNKi: -UPnPNAT Support-]
						if(UPnP)
							theApp.m_UPnP_IGDControlPoint->DeletePortMapping(mapping);
						// End emulEspaña
					}
					VERIFY( CloseHandle(hEvent) );
				}
			}
			VERIFY( !closesocket(hSocket) );
		}
//		VERIFY( !WSACleanup() );
	}

	return 0;
}

void StartWapSockets(CWapServer *pThis)
{
	ASSERT( s_hWapTerminate == NULL );
	ASSERT( s_pWapSocketThread == NULL );
	if ((s_hWapTerminate = CreateEvent(NULL, TRUE, FALSE, NULL)) != NULL)
	{
		// - do NOT use Windows API 'CreateThread' to create a thread which uses MFC/CRT -> lot of mem leaks!
		// - because we want to wait on the thread handle we have to disable 'CWinThread::m_AutoDelete' -> can't 
		//   use 'AfxBeginThread'
		s_pWapSocketThread = new CWinThread(WapSocketListeningFunc, (LPVOID)pThis);
		s_pWapSocketThread->m_bAutoDelete = FALSE;
		if (!s_pWapSocketThread->CreateThread())
		{
			CloseHandle(s_hWapTerminate);
			s_hWapTerminate = NULL;
			delete s_pWapSocketThread;
			s_pWapSocketThread = NULL;
		}
	}
}

void StopWapSockets()
{
	if (s_pWapSocketThread)
	{
		VERIFY( SetEvent(s_hWapTerminate) );

		if (s_pWapSocketThread->m_hThread)
		{
			// because we want to wait on the thread handle we must not use 'CWinThread::m_AutoDelete'.
			// otherwise we may run into the situation that the CWinThread was already auto-deleted and
			// the CWinThread::m_hThread is invalid.
			ASSERT( !s_pWapSocketThread->m_bAutoDelete );

			DWORD dwWaitRes = WaitForSingleObject(s_pWapSocketThread->m_hThread, 1300);
			if (dwWaitRes == WAIT_TIMEOUT)
			{
				TRACE("*** Failed to wait for websocket thread termination - Timeout\n");
				VERIFY( TerminateThread(s_pWapSocketThread->m_hThread, (DWORD)-1) );
				VERIFY( CloseHandle(s_pWapSocketThread->m_hThread) );
			}
			else if (dwWaitRes == -1)
			{
				TRACE("*** Failed to wait for websocket thread termination - Error %u\n", GetLastError());
				ASSERT(0); // probable invalid thread handle
			}
		}
		delete s_pWapSocketThread;
		s_pWapSocketThread = NULL;
	}
	if (s_hWapTerminate){
		VERIFY( CloseHandle(s_hWapTerminate) );
		s_hWapTerminate = NULL;
	}
}
