//	This file is part of eMule Plus
//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//	as published by the Free Software Foundation; either
//	version 2 of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "stdafx.h"
#pragma comment(lib, "ws2_32.lib") 
#include "zlib/zlib.h"
#include "emule.h"
#include "WebSocket.h"
#include "WebServer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static HANDLE s_hTerminate = NULL;
static CWinThread* s_pSocketThread = NULL;

typedef struct
{
	void	*pThis;
	SOCKET	hSocket;
} SocketData;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Gmt2HttpStr() formats HTTP time string accordoing to RFC 1123.
//		Note: output example "Sun, 06 Nov 1994 08:49:37 GMT".
//		Format:
//			rfc1123-date = wkday "," SP date1 SP time SP "GMT"
//			date1        = 2DIGIT SP month SP 4DIGIT
//			               ; day month year (e.g. 02 Jun 1982)
//			time         = 2DIGIT ":" 2DIGIT ":" 2DIGIT
//			               ; 00:00:00 - 23:59:59
//			wkday        = "Mon" | "Tue" | "Wed" | "Thu" | "Fri" | "Sat" | "Sun"
//			month        = "Jan" | "Feb" | "Mar" | "Apr" | "May" | "Jun" | "Jul"
//			             | "Aug" | "Sep" | "Oct" | "Nov" | "Dec"
//		Params:
//			pcOut - buffer receiving formatted string (size of EP_MAX_HTTPGMTSTR).
//			pTm   - time to format
void Gmt2HttpStr(char *pcOut, const struct tm *pTm)
{
	static const char acWkday[7][4] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	static const char acMonth[12][4] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

	wsprintfA( pcOut, "%s, %02u %s %04u %02u:%02u:%02u GMT",
		acWkday[pTm->tm_wday], pTm->tm_mday, acMonth[pTm->tm_mon], pTm->tm_year + 1900,
		pTm->tm_hour, pTm->tm_min, pTm->tm_sec );
}

void CWebSocket::SetParent(CWebServer *pParent)
{
	m_pParent = pParent;
}

void CWebSocket::OnRequestReceived(char *pHeader, DWORD dwHeaderLen, char *pData, DWORD dwDataLen)
{
	EMULE_TRY

	CStringA	sHeader(pHeader, dwHeaderLen);
	CStringA	sURL;
	int			iIdx;
	bool		bFileReq = false;

	if (sHeader.Left(3) == "GET")
	{
		sURL = sHeader.TrimRight();
	}
	else if (sHeader.Left(4) == "POST")
	{
		CStringA		sData(pData, dwDataLen);

		sURL = '?';
		sURL += sData.Trim();	// '?' to imitate GET syntax for ParseURL
	}
	iIdx = sURL.Find(' ');
	if (iIdx >= 0)
		sURL = sURL.Mid(iIdx + 1);
	iIdx = sURL.Find(' ');
	if (iIdx >= 0)
		sURL = sURL.Left(iIdx);

	if (sURL.GetLength() > 4)
	{
		CStringA	strExt4 = sURL.Right(4).MakeLower();

		if (( (strExt4 == ".gif") || (strExt4 == ".jpg") || (strExt4 == ".png") ||
			(strExt4 == ".ico") || (strExt4 == ".css") || (sURL.Right(3).MakeLower() == ".js") ||
			(strExt4 == ".bmp") || (sURL.Right(5).MakeLower() == ".jpeg") || (strExt4 == ".xml") || (strExt4 == ".txt") )
			&& sURL.Find("..") < 0 )	// don't allow leaving the emule-webserver-folder for accessing files
		{
			bFileReq = true;
		}
	}

// HTTP header AcceptEncoding
	CStringA	strAcceptEncoding;
	iIdx = sHeader.Find("Accept-Encoding: ");
	if (iIdx >= 0)
	{
		int	iIdx2 = sHeader.Find("\r\n", iIdx += 17);

		strAcceptEncoding = sHeader.Mid(iIdx, iIdx2 - iIdx);
	}
// End AcceptEncoding

// HTTP header IfModifiedSince
	CStringA	strIfModifiedSince;
	iIdx = sHeader.Find("If-Modified-Since: ");
	if (iIdx >= 0)
	{
		int	iIdx2 = sHeader.Find("\r\n", iIdx += 19);

		strIfModifiedSince = sHeader.Mid(iIdx, iIdx2 - iIdx);
	}
// End IfModifiedSince

	ThreadData Data;

	Data.sURL = sURL;
	Data.pThis = m_pParent;
	Data.pSocket = this;
	Data.strAcceptEncoding = strAcceptEncoding;
	Data.strIfModifiedSince = strIfModifiedSince;

	if (!bFileReq)
		m_pParent->ProcessGeneralReq(Data);
	else
		m_pParent->ProcessFileReq(Data);

	Disconnect();

	EMULE_CATCH2
}

void CWebSocket::OnReceived(void* pData, DWORD dwSize)
{
	EMULE_TRY

	const UINT SIZE_PRESERVE = 0x1000;

	if (m_dwBufSize < dwSize + m_dwRecv)
	{
	//	Reallocate
		char	*pNewBuf = new char[m_dwBufSize = dwSize + m_dwRecv + SIZE_PRESERVE];

		if (pNewBuf == NULL)
		{
			m_bValid = false; //	Internal problem
			return;
		}

		if (m_pBuf != NULL)
		{
			memcpy2(pNewBuf, m_pBuf, m_dwRecv);
			delete[] m_pBuf;
		}

		m_pBuf = pNewBuf;
	}
	memcpy2(m_pBuf + m_dwRecv, pData, dwSize);
	m_dwRecv += dwSize;

//	Check if we have all that we want
	if (m_dwHttpHeaderLen == 0)
	{
	//	Try to find it
		bool	bPrevEndl = false;

		for (DWORD dwPos = 0; dwPos < m_dwRecv; dwPos++)
		{
			if ('\n' == m_pBuf[dwPos])
			{
				if (bPrevEndl)
				{
				//	We just found the end of the http header
				//	Now write the message's position into two first DWORDs of the buffer
					m_dwHttpHeaderLen = dwPos + 1;

				//	Try to find now the 'Content-Length' header
					for (dwPos = 0; dwPos < m_dwHttpHeaderLen;)
					{
						char	*pPtr = reinterpret_cast<char*>(memchr(m_pBuf + dwPos, '\n', m_dwHttpHeaderLen - dwPos));

						if (pPtr == NULL)
							break;

						DWORD	dwNextPos = pPtr - m_pBuf;

					//	Check this header
						static const char	acMatch[] = "content-length";

						if (strnicmp(m_pBuf + dwPos, acMatch, sizeof(acMatch) - 1) == 0)
						{
							dwPos += sizeof(acMatch) - 1;
							pPtr = reinterpret_cast<char *>(memchr(m_pBuf + dwPos, ':', m_dwHttpHeaderLen - dwPos));
							if (pPtr != NULL)
								m_dwHttpContentLen = atol(pPtr + 1);

							break;
						}
						dwPos = dwNextPos + 1;
					}
					break;
				}
				else
					bPrevEndl = true;
			}
			else if ('\r' != m_pBuf[dwPos])
				bPrevEndl = false;
		}
	}

	if ((m_dwHttpHeaderLen != 0) && !m_bCanRecv && (m_dwHttpContentLen == 0))
		m_dwHttpContentLen = m_dwRecv - m_dwHttpHeaderLen; // of course

	if ((m_dwHttpHeaderLen != 0) && (m_dwHttpContentLen < m_dwRecv) && ((m_dwHttpContentLen == 0) || (m_dwHttpHeaderLen + m_dwHttpContentLen <= m_dwRecv)))
	{
		OnRequestReceived(m_pBuf, m_dwHttpHeaderLen, m_pBuf + m_dwHttpHeaderLen, m_dwHttpContentLen);

		if (m_bCanRecv && (m_dwRecv > m_dwHttpHeaderLen + m_dwHttpContentLen))
		{
		//	Move our data
			m_dwRecv -= m_dwHttpHeaderLen + m_dwHttpContentLen;
			MoveMemory(m_pBuf, m_pBuf + m_dwHttpHeaderLen + m_dwHttpContentLen, m_dwRecv);
		}
		else
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
			if (pChunk != NULL)
			{
				pChunk->m_pNext = NULL;
				pChunk->m_dwSize = dwDataSize;
				if ((pChunk->m_pData = new char[dwDataSize]) != NULL)
				{
					//-- data should be copied into "pChunk->m_pData" anyhow
					memcpy2(pChunk->m_pData, pData, dwDataSize);

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

void CWebSocket::SendReply(int iHTTPRespCode)
{
	char		acBuf[256], acGMT[EP_MAX_HTTPGMTSTR];
	const char	*pcHTTPStatus;
	CTime		timeCurrent = CTime::GetCurrentTime();
	struct tm	stTm = {0};

	acGMT[0] = '\0';
	if (timeCurrent.GetGmtTm(&stTm) != NULL)
		Gmt2HttpStr(acGMT, &stTm);

//	Convert HTTP status code into a HTTP status message
//	If an unknown status code is sent send "500 Internal Server Error"
	switch (iHTTPRespCode)
	{
		case 304:
			pcHTTPStatus = "304 Not Modified";
			break;
		case 404:
			pcHTTPStatus = "404 Not Found";
			break;
		default:
			pcHTTPStatus = "500 Internal Server Error";
			break;
	}

	int		iLen = wsprintfA( acBuf,
		"HTTP/1.1 %s\r\n"
		"Date: %s\r\n"						//	HTTP/1.x response header "Date:"
		"Server: " WS_SERVER_TOKEN "\r\n"	//	HTTP/1.x response header "Server:"
		"Connection: close\r\n\r\n",
		pcHTTPStatus,
		acGMT );
	ASSERT(iLen < ARRSIZE(acBuf));

	SendData(acBuf, iLen);
}

void CWebSocket::SendContent(LPCSTR szStdResponse, const void *pContent, DWORD dwContentSize)
{
	EMULE_TRY

	char		szBuf[2*1024], acGMT[EP_MAX_HTTPGMTSTR];
	CTime		timeCurrent = CTime::GetCurrentTime();
	struct tm	stTm = {0};

	acGMT[0] = '\0';
	if (timeCurrent.GetGmtTm(&stTm) != NULL)
		Gmt2HttpStr(acGMT, &stTm);

	int iLen = wsprintfA( szBuf,
		"HTTP/1.1 200 OK\r\n"
		"Date: %s\r\n"						//	HTTP/1.x response header "Date:"
		"Server: " WS_SERVER_TOKEN "\r\n"	//	HTTP/1.x response header "Server:"
		"%s"
		"Content-Length: %u\r\n"
		"Connection: close\r\n\r\n",
		acGMT,
		szStdResponse,
		dwContentSize );
	SendData(szBuf, iLen);
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

UINT AFX_CDECL WebSocketAcceptedFunc(LPVOID pD)
{
#ifdef EP_SPIDERWEB
//	Setup Structured Exception handler for the this thread
	_set_se_translator(StructuredExceptionHandler);
#endif

	EMULE_TRY

	g_App.m_pPrefs->InitThreadLocale();

	SocketData *pData = (SocketData *)pD;
	SOCKET hSocket = pData->hSocket;
	CWebServer *pThis = (CWebServer *)pData->pThis;
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

		//	In multithread environment every thread has its own random number generator
			srand(time(NULL));

			HANDLE pWait[] = { hEvent, s_hTerminate };

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
							for (;;)
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
								if ((stWebSocket.m_pHead = pNext) == NULL)
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
			delete[] stWebSocket.m_pBuf;
		}

		VERIFY(CloseHandle(hEvent));
	}

	VERIFY(!closesocket(hSocket));

	EMULE_CATCH2

	return 0;
}

UINT AFX_CDECL WebSocketListeningFunc(LPVOID pThis)
{
	EMULE_TRY

	g_App.m_pPrefs->InitThreadLocale();

	int		iRc = 0;

	SOCKET hSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
	if (INVALID_SOCKET != hSocket)
	{
		SOCKADDR_IN stAddr;
		stAddr.sin_family = AF_INET;
		stAddr.sin_port = fast_htons(g_App.m_pPrefs->GetWSPort());
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
					HANDLE pWait[] = { hEvent, s_hTerminate };
					while (WAIT_OBJECT_0 == WaitForMultipleObjects(2, pWait, FALSE, INFINITE))
						for (;;)
						{
							struct sockaddr_in their_addr;//log intruder
							int sin_size = sizeof(struct sockaddr_in);

							SOCKET hAccepted = accept(hSocket, (LPSOCKADDR)&their_addr, &sin_size);//log intruder

							if (INVALID_SOCKET == hAccepted)
								break;

							pWebServer->SetIP(their_addr.sin_addr.S_un.S_addr);//log intruder

							if(g_App.m_pPrefs->GetWSIsEnabled())
							{
								SocketData *pData = new SocketData;
								pData->hSocket = hAccepted;
								pData->pThis = pThis;
							//	Do NOT use Windows API 'CreateThread' to create a thread which uses MFC/CRT -> lots of mem leaks!
							//	'AfxBeginThread' could be used here, but creates a little too much overhead for our needs.
								CWinThread* pAcceptThread = new CWinThread(WebSocketAcceptedFunc, (LPVOID)pData);
								if (!pAcceptThread->CreateThread())
								{
									delete pAcceptThread;
									pAcceptThread = NULL;
									VERIFY( !closesocket(hSocket) );
								}
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
	if (iRc != 0)
		g_App.m_pMDlg->AddLogLine(LOG_RGB_ERROR, IDS_WEB_SOCK_FAILED, iRc);

	EMULE_CATCH2

	return 0;
}


void StartSockets(CWebServer *pThis)
{
	ASSERT( s_hTerminate == NULL );
	ASSERT( s_pSocketThread == NULL );
	if ((s_hTerminate = CreateEvent(NULL, TRUE, FALSE, NULL)) != NULL)
	{
	//	Do NOT use Windows API 'CreateThread' to create a thread which uses MFC/CRT -> lot of mem leaks!
	//	because we want to wait on the thread handle we have to disable 'CWinThread::m_AutoDelete'
	//	-> can't use 'AfxBeginThread'
		s_pSocketThread = new CWinThread(WebSocketListeningFunc, (LPVOID)pThis);
		s_pSocketThread->m_bAutoDelete = FALSE;
		if (!s_pSocketThread->CreateThread())
		{
			CloseHandle(s_hTerminate);
			s_hTerminate = NULL;
			delete s_pSocketThread;
			s_pSocketThread = NULL;
		}
	}
}

void StopSockets()
{
	if (s_pSocketThread)
	{
		VERIFY( SetEvent(s_hTerminate) );

		if (s_pSocketThread->m_hThread)
		{
		//	Because we want to wait on the thread handle we must not use 'CWinThread::m_AutoDelete'.
		//	otherwise we may run into the situation that the CWinThread was already auto-deleted and
		//	the CWinThread::m_hThread is invalid.
			ASSERT( !s_pSocketThread->m_bAutoDelete );

			DWORD dwWaitRes = WaitForSingleObject(s_pSocketThread->m_hThread, 1300);
			if (dwWaitRes == WAIT_TIMEOUT)
			{
				TRACE("*** Failed to wait for websocket thread termination - Timeout\n");
				VERIFY( TerminateThread(s_pSocketThread->m_hThread, ~0ul) );
				VERIFY( CloseHandle(s_pSocketThread->m_hThread) );
			}
			else if (dwWaitRes == WAIT_FAILED)
			{
				TRACE("*** Failed to wait for websocket thread termination - Error %u\n", GetLastError());
				ASSERT(0); //	Probably invalid thread handle
			}
		}
		delete s_pSocketThread;
		s_pSocketThread = NULL;
	}
	if (s_hTerminate)
	{
		VERIFY( CloseHandle(s_hTerminate) );
		s_hTerminate = NULL;
	}
}
