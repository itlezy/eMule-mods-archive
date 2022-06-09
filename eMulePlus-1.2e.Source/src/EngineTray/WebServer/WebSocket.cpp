// This class is temporary and will be replaced by generic sockets task processor
//

#include <stdafx.h> 
#pragma comment(lib, "ws2_32.lib") 

#include <stdlib.h> // for _tstol function

#include "WebSocket.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

HANDLE g_hTerminate = NULL;
HANDLE g_hSocketThread = NULL;

typedef struct
{
	void	*pThis;
	SOCKET	hSocket;
} SocketData;

void CWebSocket::SetParent(CWebServerImpl *pParent)
{
	m_pParent = pParent;
}

void CWebSocket::OnRequestReceived(TCHAR* pHeader, DWORD dwHeaderLen, TCHAR* pData, DWORD dwDataLen)
{
	EMULE_TRY

	CString		sHeader(pHeader, dwHeaderLen);
	CString		sURL;
	CString		sArgs;
	int			iIdx;
	bool		bFileReq = false;

	sURL = sHeader;
	sURL.TrimRight();

	if(sHeader.Left(3) == "GET")
	{
		if((iIdx = sURL.Find(_T('?'))) > -1)
		{
			sArgs = sURL.Mid(iIdx + 1);
			sURL = sURL.Left(iIdx);
		}
	}
	else if(sHeader.Left(4) == "POST")
	{
		CString		sData(pData, dwDataLen);
		sData.TrimRight();
		sData.TrimLeft();
		sArgs = sData;
	}
	// Clean sURL
	if((iIdx = sURL.Find(_T(' '))) > -1)
		sURL = sURL.Mid(iIdx + 1);
	if((iIdx = sURL.Find(_T(' '))) > -1)
		sURL = sURL.Left(iIdx);
	// Clean sArgs
	if((iIdx = sArgs.Find(_T(' '))) > -1)
		sArgs = sArgs.Left(iIdx);

	// only if its http://server[:port]/file/filename.recognized_extension
	if(sURL.GetLength() > 7 && sURL.Left(6) == _T("/file/"))
	{
		CString sExt = sURL.Right(5);
		sExt.MakeLower();
		int nPos = sExt.Find(_T("."));
		if(nPos >= 0 && nPos != (sExt.GetLength() - 1))
		{
			sExt = m_pParent->GetContentType(sExt.Mid(nPos + 1));
			if(!sExt.IsEmpty())
			{
				sURL = sURL.Right(sURL.GetLength() - 5);
				bFileReq = true;
			}
		}
	}

	ThreadData Data;

	Data.sURL = sURL;
	Data.sArgs = sArgs;
	Data.pThis = m_pParent;
	Data.pSocket = this;

	if(!bFileReq)
		m_pParent->ProcessURL(Data);
	else
		m_pParent->ProcessFileRequest(Data);

	Disconnect();

	EMULE_CATCH2
}

void CWebSocket::OnReceived(void* pData, DWORD dwSize)
{
	EMULE_TRY

	const UINT SIZE_PRESERVE = 0x1000;

	if (m_dwBufSize < dwSize + m_dwRecv)
	{
		// reallocate
		TCHAR* pNewBuf = new TCHAR[m_dwBufSize = dwSize + m_dwRecv + SIZE_PRESERVE];
		if (!pNewBuf)
		{
			m_bValid = false; // internal problem
			return;
		}

		if (m_pBuf)
		{
			memcpy(pNewBuf, m_pBuf, m_dwRecv);
			delete[] m_pBuf;
		}

		m_pBuf = pNewBuf;
	}
	memcpy(m_pBuf + m_dwRecv, pData, dwSize);
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
						PVOID pPtr = memchr(m_pBuf + dwPos, '\n', m_dwHttpHeaderLen - dwPos);
						if (!pPtr)
							break;
						DWORD dwNextPos = ((DWORD) pPtr) - ((DWORD) m_pBuf);

						// check this header
						TCHAR szMatch[] = _T("content-length");
						if (!_tcsncicmp(m_pBuf + dwPos, szMatch, sizeof(szMatch) - 1))
						{
							dwPos += sizeof(szMatch) - 1;
							pPtr = memchr(m_pBuf + dwPos, ':', m_dwHttpHeaderLen - dwPos);
							if (pPtr)
								m_dwHttpContentLen = _tstol(((TCHAR*) pPtr) + 1);

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

	if ((m_dwHttpHeaderLen != 0) && (m_dwHttpContentLen < m_dwRecv) && (!m_dwHttpContentLen || (m_dwHttpHeaderLen + m_dwHttpContentLen <= m_dwRecv)))
	{
		OnRequestReceived(m_pBuf, m_dwHttpHeaderLen, m_pBuf + m_dwHttpHeaderLen, m_dwHttpContentLen);

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
	EMULE_CATCH2
}

void CWebSocket::SendData(const void* pData, DWORD dwDataSize)
{
	EMULE_TRY

	ASSERT(pData);
	if (m_bValid && m_bCanSend)
	{
		if (!m_pHead)
		{
			// try to send it directly
			//-- remember: in "nRes" could be "-1" after "send" call	//SyruS (0.30b)
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
				if (pChunk->m_pData = new char[dwDataSize])
				{
					//-- data should be copied into "pChunk->m_pData" anyhow	//SyruS (0.30b)
					memcpy(pChunk->m_pData, pData, dwDataSize);

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
	EMULE_CATCH2
}

void CWebSocket::SendContent(LPCTSTR szStdResponse, const void* pContent, DWORD dwContentSize)
{
	EMULE_TRY

	char szBuf[0x1000];
	int nLen = wsprintfA(szBuf, "HTTP/1.1 200 OK\r\n%sContent-Length: %ld\r\n\r\n", szStdResponse, dwContentSize);
	SendData(szBuf, nLen);
	SendData(pContent, dwContentSize);

	EMULE_CATCH2
}

void CWebSocket::Disconnect()
{
	EMULE_TRY

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

	EMULE_CATCH2
}

DWORD WINAPI WebSocketAcceptedFunc(void *pD)
{
	EMULE_TRY

	SocketData *pData = (SocketData *)pD;
	SOCKET hSocket = pData->hSocket;
	CWebServerImpl *pThis = (CWebServerImpl *)pData->pThis;
	delete pData;

	ASSERT(INVALID_SOCKET != hSocket);


	HANDLE hEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (hEvent)
	{
		if (!WSAEventSelect(hSocket, hEvent, FD_READ | FD_CLOSE | FD_WRITE))
		{
			CWebSocket stWebSocket;
			stWebSocket.SetParent(pThis);
			stWebSocket.m_pHead = NULL;
			stWebSocket.m_pTail = NULL;
			stWebSocket.m_bValid = true;
			stWebSocket.m_bCanRecv = true;
			stWebSocket.m_bCanSend = true;
			stWebSocket.m_hSocket = hSocket;
			stWebSocket.m_pBuf = NULL;
			stWebSocket.m_dwRecv = 0;
			stWebSocket.m_dwBufSize = 0;
			stWebSocket.m_dwHttpHeaderLen = 0;
			stWebSocket.m_dwHttpContentLen = 0;

			HANDLE pWait[] = { hEvent, g_hTerminate };

			while (WAIT_OBJECT_0 == WaitForMultipleObjects(2, pWait, FALSE, INFINITE))
			{
				while (stWebSocket.m_bValid)
				{
					WSANETWORKEVENTS stEvents;
					if (WSAEnumNetworkEvents(hSocket, NULL, &stEvents))
						stWebSocket.m_bValid = false;
					else
					{
						if (!stEvents.lNetworkEvents)
							break; //no more events till now

						if (FD_READ & stEvents.lNetworkEvents)
							while (true)
							{
								char pBuf[0x1000];
								int nRes = recv(hSocket, pBuf, sizeof(pBuf), 0);
								if (nRes <= 0)
								{
									if (!nRes)
									{
										stWebSocket.m_bCanRecv = false;
										stWebSocket.OnReceived(NULL, 0);
									}
									else
										if (WSAEWOULDBLOCK != WSAGetLastError())
											stWebSocket.m_bValid = false;
									break;
								}
								stWebSocket.OnReceived(pBuf, nRes);
							}

						if (FD_CLOSE & stEvents.lNetworkEvents)
							stWebSocket.m_bCanRecv = false;

						if (FD_WRITE & stEvents.lNetworkEvents)
							// send what is left in our tails
							while (stWebSocket.m_pHead)
							{
								if (stWebSocket.m_pHead->m_pToSend)
								{
									int nRes = send(hSocket, stWebSocket.m_pHead->m_pToSend, stWebSocket.m_pHead->m_dwSize, 0);
									if (nRes != (signed) stWebSocket.m_pHead->m_dwSize)
									{
										if (nRes)
											if ((nRes > 0) && (nRes < (signed) stWebSocket.m_pHead->m_dwSize))
											{
												stWebSocket.m_pHead->m_pToSend += nRes;
												stWebSocket.m_pHead->m_dwSize -= nRes;

											} else
												if (WSAEWOULDBLOCK != WSAGetLastError())
													stWebSocket.m_bValid = false;
										break;
									}
								} else
									if (shutdown(hSocket, SD_SEND))
									{
										stWebSocket.m_bValid = false;
										break;
									}

								// erase this chunk
								CWebSocket::CChunk* pNext = stWebSocket.m_pHead->m_pNext;
								delete stWebSocket.m_pHead;
								if (!(stWebSocket.m_pHead = pNext))
									stWebSocket.m_pTail = NULL;
							}
					}
				}

				if (!stWebSocket.m_bValid || (!stWebSocket.m_bCanRecv && !stWebSocket.m_pHead))
					break;
			}

			while (stWebSocket.m_pHead)
			{
				CWebSocket::CChunk* pNext = stWebSocket.m_pHead->m_pNext;
				delete stWebSocket.m_pHead;
				stWebSocket.m_pHead = pNext;
			}
			if (stWebSocket.m_pBuf)
				delete[] stWebSocket.m_pBuf;
		}

		VERIFY(CloseHandle(hEvent));
	}

	VERIFY(!closesocket(hSocket));

	EMULE_CATCH2

	return 0;
}

DWORD WINAPI WebSocketListeningFunc(void *pThis)
{
	EMULE_TRY

	int		iRc;
	WSADATA	stData;

	if ((iRc = WSAStartup(MAKEWORD(2, 2), &stData)) == 0)
	{
		SOCKET hSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
		if (INVALID_SOCKET != hSocket)
		{
			SOCKADDR_IN stAddr;
			stAddr.sin_family = AF_INET;
			stAddr.sin_port = htons(WEBSERVER_PORT);
			stAddr.sin_addr.S_un.S_addr = INADDR_ANY;

			if (!bind(hSocket, (sockaddr*) &stAddr, sizeof(stAddr)) &&
				!listen(hSocket, SOMAXCONN))
			{
				HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
				if (hEvent)
				{
					if (!WSAEventSelect(hSocket, hEvent, FD_ACCEPT))
					{
						CWebServer *pWebServer = (CWebServer*)pThis;//log intruder
						HANDLE pWait[] = { hEvent, g_hTerminate };
						while (WAIT_OBJECT_0 == WaitForMultipleObjects(2, pWait, FALSE, INFINITE))
							for (;;)
							{
								struct sockaddr_in their_addr;//log intruder
								int sin_size = sizeof(struct sockaddr_in);

								SOCKET hAccepted = accept(hSocket, (LPSOCKADDR)&their_addr, &sin_size);//log intruder

								if (INVALID_SOCKET == hAccepted)
									break;

/*								pWebServer->SetIP(their_addr.sin_addr.S_un.S_addr);//log intruder
*/
								if(WEBSERVER_ENABLED)
								{
									DWORD dwThread = 0;
									SocketData *pData = new SocketData;
									pData->hSocket = hAccepted;
									pData->pThis = pThis;
									HANDLE hThread = CreateThread(NULL, 0, WebSocketAcceptedFunc, (void *)pData, 0, &dwThread);
									if (hThread)
										VERIFY(CloseHandle(hThread));
									else
										VERIFY(!closesocket(hSocket));
								}
								else
									VERIFY(!closesocket(hSocket));
							}
					}

					VERIFY(CloseHandle(hEvent));
				}
			}
			else
				iRc = WSAGetLastError();

			VERIFY(!closesocket(hSocket));
		}
		else
			iRc = WSAGetLastError();

		VERIFY(!WSACleanup());
	}
/*	if (iRc != 0)
		g_eMuleApp.m_pdlgEmule->AddLogLine(false, RGB_LOG_ERROR + GetResString(IDS_WEB_SOCK_FAILED), iRc);
*/
	EMULE_CATCH2

	return 0;
}

void StartSockets(CWebServer *pThis)
{
	StartSocketsImpl(reinterpret_cast<CWebServerImpl*>(pThis)); 
}

void StartSocketsImpl(CWebServerImpl *pThis)
{
	if (g_hTerminate = CreateEvent(NULL, TRUE, FALSE, NULL))
	{
		DWORD dwThread = 0;
		g_hSocketThread = CreateThread(NULL, 0, WebSocketListeningFunc, (void*)pThis, 0, &dwThread);
		if (!g_hSocketThread)
		{
			VERIFY(CloseHandle(g_hTerminate));
			g_hTerminate = NULL;
			g_hSocketThread = NULL;
		}
	}
}

void StopSockets()
{
	if (g_hSocketThread)
	{
		VERIFY(SetEvent(g_hTerminate));

		if (WAIT_TIMEOUT == WaitForSingleObject(g_hSocketThread, 1300))
			VERIFY(TerminateThread(g_hSocketThread, -1));
		VERIFY(CloseHandle(g_hSocketThread));
		g_hSocketThread = 0;
	}
	if (g_hTerminate)
	{
		VERIFY(CloseHandle(g_hTerminate));
		g_hTerminate = 0;
	}
}
