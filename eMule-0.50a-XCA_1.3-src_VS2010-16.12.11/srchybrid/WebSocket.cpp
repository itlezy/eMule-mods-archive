#include <stdafx.h>
#include "emule.h"
#include "OtherFunctions.h"
#include "WebSocket.h"
#include "WebServer.h"
#include "Preferences.h"
#include "StringConversion.h"
#include "Log.h"
#include "MD5Sum.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static HANDLE s_hTerminate = NULL;
static CWinThread* s_pSocketThread = NULL;

typedef struct
{
	void	*pThis;
	SOCKET	hSocket;
	in_addr incomingaddr;
} SocketData;

#define HTTPInit "Server: eMule\r\nConnection: close\r\nContent-Type: text/html\r\n"
#define USEGZ "Content-Encoding: gzip\r\n"
#define MIN_COMPRESS_SIZE	30

struct SWebFileType
{
	LPCTSTR pszExt;
	LPCSTR pszContentType;
	bool bCompressed;
	bool bAllowAccess;
} g_aWebFileTypes[] = 
{
	{ _T("bmp"), "image/bmp", false, true },
	{ _T("css"), "text/css", false, true },
	{ _T("gif"), "image/gif", true, true },
	{ _T("ico"), "image/x-icon", false, true },
	{ _T("jpeg"), "image/jpg", true, true },
	{ _T("jpg"), "image/jpg", true, true },
	{ _T("js"), "text/javascript", false, true },
	{ _T("png"), "image/png", true, true }
};
SWebFileType DefaultType = { NULL, "application/octet-stream", true, false };

int __cdecl CompareWebFileType(const void* p1, const void* p2)
{
	return _tcscmp( ((const SWebFileType*)p1)->pszExt, ((const SWebFileType*)p2)->pszExt );
}

SWebFileType* GetWebFileType(LPCTSTR pszFileName)
{
	LPCTSTR pszExt = _tcsrchr(pszFileName, _T('.'));
	if (pszExt == NULL)
		return &DefaultType;
	CString strExt(pszExt + 1);
	strExt.MakeLower();

	SWebFileType ft;
	ft.pszExt = strExt;
	SWebFileType *pFound = (SWebFileType*)bsearch(&ft, g_aWebFileTypes, _countof(g_aWebFileTypes), sizeof g_aWebFileTypes[0], CompareWebFileType);
	return pFound != NULL ? pFound : &DefaultType;
}

void CWebSocket::SetParent(CWebServer *pParent)
{
	m_pParent = pParent;
}

void CWebSocket::OnRequestReceived(char* pHeader, DWORD dwHeaderLen, char* pData, DWORD dwDataLen, in_addr inad)
{
	CStringA sHeader(pHeader, dwHeaderLen);
	CStringA sURL;
	bool filereq=false;

	if(strncmp(pHeader, "GET", 3) == 0)
	{
		sURL = sHeader.TrimRight();
	if(sURL.Find(' ') > -1)
		sURL = sURL.Mid(sURL.Find(' ')+1, sURL.GetLength());
	if(sURL.Find(' ') > -1)
		sURL = sURL.Left(sURL.Find(' '));
	if (sURL.GetLength()>4){	// min length (for valid extentions)
			CString filename(sURL);
			SWebFileType* fileType = GetWebFileType(filename);
			if (fileType->bAllowAccess
		&& sURL.Find("..")==-1	// dont allow leaving the emule-webserver-folder for accessing files
		)
			filereq=true;
	}
	}
	else if(strncmp(pHeader, "POST", 4) == 0)
	{
		CStringA sData(pData, dwDataLen);
		sURL = "?" + sData.Trim();	// '?' to imitate GET syntax for ParseURL
	}

	ThreadData Data;
	Data.sURL = sURL;
	Data.pThis = m_pParent;
	Data.inadr = inad;
	Data.pSocket = this;

	if (!filereq)
		m_pParent->ProcessURL(Data);
	else{ // X: [AEWI] - [AJAX Enabled Web Interface] support ETag
		CStringA ETag;
		int tagstart = sHeader.Find("If-None-Match: ");
		if(tagstart != -1)
			ETag = sHeader.Mid(tagstart+_countof("If-None-Match: ")-1, 32);
		m_pParent->ProcessFileReq(Data, ETag);
	}

	Disconnect();
}

void CWebSocket::OnReceived(void* pData, DWORD dwSize, in_addr inad)
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
		for (size_t dwPos = 0; dwPos < m_dwRecv; dwPos++)
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
						size_t dwNextPos = pPtr - m_pBuf;

						// check this header
						char szMatch[] = "content-length";
						if (!_strnicmp(m_pBuf + dwPos, szMatch, _countof(szMatch) - 1))
						{
							dwPos += _countof(szMatch) - 1;
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

	if (m_dwHttpHeaderLen && m_dwHttpContentLen < m_dwRecv && (!m_dwHttpContentLen || (m_dwHttpHeaderLen + m_dwHttpContentLen <= m_dwRecv)))
	{
		OnRequestReceived(m_pBuf, m_dwHttpHeaderLen, m_pBuf + m_dwHttpHeaderLen, m_dwHttpContentLen, inad);

		if (m_bCanRecv && (m_dwRecv > m_dwHttpHeaderLen + m_dwHttpContentLen))
		{
			// move our data
			m_dwRecv -= m_dwHttpHeaderLen + m_dwHttpContentLen;
			memmove(m_pBuf, m_pBuf + m_dwHttpHeaderLen + m_dwHttpContentLen, m_dwRecv);
		} else
			m_dwRecv = 0;

		m_dwHttpHeaderLen = 0;
		m_dwHttpContentLen = 0;
	}

}

void CWebSocket::SendData(const void* pData, DWORD dwDataSize)
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
}

void CWebSocket::SendReply(LPCSTR szReply)
{
	char szBuf[256];
	int nLen = _snprintf(szBuf, _countof(szBuf), "%s\r\n", szReply);
	if (nLen > 0)
		SendData(szBuf, nLen);
}

void CWebSocket::SendContent(LPCSTR szStdResponse, const void* pContent, DWORD dwContentSize)
{
	char szBuf[0x1000];
	int nLen = _snprintf(szBuf, _countof(szBuf), "HTTP/1.1 200 OK\r\n%sContent-Length: %ld\r\n\r\n", szStdResponse, dwContentSize);
	if (nLen > 0) {
		SendData(szBuf, nLen);
		if(dwContentSize > 0) // X: [CI] - [Code Improvement]
			SendData(pContent, dwContentSize);
	}
}

void CWebSocket::SendHtml(const CString& rstr, bool useGzip)
{
	CStringA strA(wc2utf8(rstr));
	unsigned long contentSize = strA.GetLength();
	if(useGzip && contentSize > MIN_COMPRESS_SIZE)
	{
		char*gzipOut = NULL;
		try
		{
			unsigned long gzipLen = contentSize + 1024;
			gzipOut = new char[gzipLen];
			int ret = GZipCompress((Bytef*)gzipOut, &gzipLen, (const Bytef*)(LPCSTR)strA, contentSize, Z_DEFAULT_COMPRESSION);
			if(ret == Z_OK && gzipLen < contentSize)
			{
				SendContent(HTTPInit USEGZ, gzipOut, gzipLen);
				return;
			}
		}
		catch(...)
		{
			ASSERT(0);
		}
		delete[] gzipOut;
	}
	char*strContent = new char[contentSize];
	memcpy(strContent, strA, contentSize);
	SendContent(HTTPInit, strContent, contentSize);
}

void CWebServer::ProcessFileReq(ThreadData&Data, CStringA reqETag) {// X: [AEWI] - [AJAX Enabled Web Interface] support ETag
	CWebServer *pThis = (CWebServer *)Data.pThis;
	if (pThis == NULL) return;

	CString filename = Data.sURL;
	filename.Replace(_T('/'),_T('\\'));
	if (filename.GetAt(0)==_T('\\')) filename.Delete(0);
	filename = thePrefs.GetMuleDirectory(EMULE_WEBSERVERDIR) + filename;

	CString filename_gz(filename);
	filename_gz.Append(_T(".gz"));
	bool fileGzipped = false;
	struct _stat fileinfo;
	if (_tstat(filename_gz, &fileinfo) == 0)
		fileGzipped = true;
	else if (_tstat(filename, &fileinfo) == 0)
		filename_gz = filename;
	else
	{
		Data.pSocket->SendReply("HTTP/1.1 404 File not found\r\n");
		return;
	}

	if (thePrefs.GetMaxWebUploadFileSizeMB() != 0 && fileinfo.st_size > (long)thePrefs.GetMaxWebUploadFileSizeMB()*1024*1024)
	{
		Data.pSocket->SendReply("HTTP/1.1 403 Forbidden\r\n");
		return;
	}

	char szEtag[33];
	md4strA(MD5Sum((unsigned char *)&fileinfo.st_mtime, sizeof(fileinfo.st_mtime)).GetRawHash(), szEtag);
	if(reqETag == szEtag)
	{
		Data.pSocket->SendReply("HTTP/1.1 304 Not Modified\r\n");
		return;
	}

	CFile file;
	if(file.Open(filename_gz, CFile::modeRead|CFile::shareDenyWrite|CFile::typeBinary))
	{
		char* buffer = new char[fileinfo.st_size];
		file.Read(buffer, fileinfo.st_size);
		file.Close();

		CStringA contenttype;
		SWebFileType* fileType = GetWebFileType(filename);
		contenttype.Format("Content-Type: %s\r\nETag: %s\r\n", fileType->pszContentType, szEtag);
		if(thePrefs.GetWebUseGzip() && (fileGzipped || (fileinfo.st_size > MIN_COMPRESS_SIZE) && !fileType->bCompressed))
		{
			if(!fileGzipped)
			{
				char* gzipOut = NULL;
				try
				{
					uLongf destLen = fileinfo.st_size + 1024;
					gzipOut = new char[destLen];
					int ret = GZipCompress((Bytef*)gzipOut, &destLen, (const Bytef*)buffer, fileinfo.st_size, Z_DEFAULT_COMPRESSION);
#ifdef _DEBUG
					if(ret == Z_OK)
						AddLogLine(false, _T("File Request:%s, size:%s, compress ratio:%.1f%%"), filename, CastItoXBytes((uint32)destLen), destLen*100.0/fileinfo.st_size);
#endif
					if (ret == Z_OK && (long)destLen < fileinfo.st_size)
					{
						delete [] buffer;
						Data.pSocket->SendContent(contenttype + USEGZ, gzipOut, destLen);
						delete[] gzipOut;
						return;
					}
				}
				catch(...)
				{
					ASSERT(0);
				}
				delete[] gzipOut;
			}
			else
				contenttype += USEGZ;
		}
		else if(fileGzipped)
		{
			unsigned char* gzipOut = NULL;
			uLongf destLen;
			try
			{
				if(GZipUncompress(&gzipOut, &destLen, (unsigned char* )buffer, fileinfo.st_size) == 0)
				{
					delete [] buffer;
					Data.pSocket->SendContent(contenttype, gzipOut, destLen);
					delete[] gzipOut;
					return;
				}
			}
			catch(...)
			{
				ASSERT(0);
				delete[] gzipOut;
			}
			delete[] buffer;
			Data.pSocket->SendReply("HTTP/1.1 404 File not found\r\n");
			return;
		}
		Data.pSocket->SendContent(contenttype, buffer, fileinfo.st_size);
		delete[] buffer;
	}
	else
		Data.pSocket->SendReply("HTTP/1.1 404 File not found\r\n");
}

void CWebSocket::Disconnect()
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

UINT AFX_CDECL WebSocketAcceptedFunc(LPVOID pD)
{
	DbgSetThreadName("WebSocketAccepted");

	ThreadLocalPtr<SFMT> p_rng(&t_rng);
	InitThreadLocale();

	SocketData *pData = (SocketData *)pD;
	SOCKET hSocket = pData->hSocket;
	CWebServer *pThis = (CWebServer *)pData->pThis;
	in_addr ad=pData->incomingaddr;
	
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
										stWebSocket.OnReceived(NULL, 0, ad);
									}
									else
										if (WSAEWOULDBLOCK != WSAGetLastError())
											stWebSocket.m_bValid = false;
									break;
								}
								stWebSocket.OnReceived(pBuf, nRes,ad);
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
								stWebSocket.m_pHead = pNext;
								if (stWebSocket.m_pHead == NULL)
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
		VERIFY( CloseHandle(hEvent) );
	}
	VERIFY( !closesocket(hSocket) );

	return 0;
}

UINT AFX_CDECL WebSocketListeningFunc(LPVOID pThis)
{
	DbgSetThreadName("WebSocketListening");

	ThreadLocalPtr<SFMT> p_rng(&t_rng);
	InitThreadLocale();

	SOCKET hSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
	if (INVALID_SOCKET != hSocket)
	{
		SOCKADDR_IN stAddr;
		stAddr.sin_family = AF_INET;
		stAddr.sin_port = htons(thePrefs.GetWSPort());
		if (thePrefs.GetBindAddrA())
			stAddr.sin_addr.S_un.S_addr = inet_addr(thePrefs.GetBindAddrA());
		else
			stAddr.sin_addr.S_un.S_addr = INADDR_ANY;

		if (!::bind(hSocket, (sockaddr*)&stAddr, sizeof(stAddr)) && !listen(hSocket, SOMAXCONN))
		{
			HANDLE hEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
			if (hEvent)
			{
				if (!WSAEventSelect(hSocket, hEvent, FD_ACCEPT))
				{
					HANDLE pWait[] = { hEvent, s_hTerminate };
					while (WAIT_OBJECT_0 == WaitForMultipleObjects(2, pWait, FALSE, INFINITE))
					{
						for (;;)
						{
							struct sockaddr_in their_addr;
                            int sin_size = sizeof(struct sockaddr_in);

							SOCKET hAccepted = accept(hSocket,(struct sockaddr *)&their_addr, &sin_size);
							if (INVALID_SOCKET == hAccepted)
								break;

							if (thePrefs.GetAllowedRemoteAccessIPs().GetCount() > 0)
							{
								bool bAllowedIP = false;
								for (size_t i = 0; i < thePrefs.GetAllowedRemoteAccessIPs().GetCount(); i++)
								{
									if (their_addr.sin_addr.S_un.S_addr == thePrefs.GetAllowedRemoteAccessIPs()[i])
									{
										bAllowedIP = true;
										break;
									}
								}
								if (!bAllowedIP) {
									LogWarning(_T("Web Interface: Rejected connection attempt from %s"), ipstr(their_addr.sin_addr.S_un.S_addr));
									VERIFY( !closesocket(hAccepted) );
									break;
								}
							}

							if(thePrefs.GetWSIsEnabled())
							{
								SocketData *pData = new SocketData;
								pData->hSocket = hAccepted;
								pData->pThis = pThis;
								pData->incomingaddr=their_addr.sin_addr;
								
								// - do NOT use Windows API 'CreateThread' to create a thread which uses MFC/CRT -> lot of mem leaks!
								// - 'AfxBeginThread' could be used here, but creates a little too much overhead for our needs.
								CWinThread* pAcceptThread = new CWinThread(WebSocketAcceptedFunc, (LPVOID)pData);
								if (!pAcceptThread->CreateThread())
								{
									delete pAcceptThread;
									pAcceptThread = NULL;
									VERIFY( !closesocket(hAccepted) );
									hAccepted = NULL;
								}
							}
							else
							{
								VERIFY( !closesocket(hAccepted) );
								hAccepted = NULL;
							}
						}
					}
				}
				VERIFY( CloseHandle(hEvent) );
				hEvent = NULL;
			}
		}
		VERIFY( !closesocket(hSocket) );
		hSocket = NULL;
	}

	return 0;
}

void StartSockets(CWebServer *pThis)
{
	ASSERT( s_hTerminate == NULL );
	ASSERT( s_pSocketThread == NULL );
	if ((s_hTerminate = CreateEvent(NULL, TRUE, FALSE, NULL)) != NULL)
	{
		// - do NOT use Windows API 'CreateThread' to create a thread which uses MFC/CRT -> lot of mem leaks!
		// - because we want to wait on the thread handle we have to disable 'CWinThread::m_AutoDelete' -> can't 
		//   use 'AfxBeginThread'
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
			// because we want to wait on the thread handle we must not use 'CWinThread::m_AutoDelete'.
			// otherwise we may run into the situation that the CWinThread was already auto-deleted and
			// the CWinThread::m_hThread is invalid.
			ASSERT( !s_pSocketThread->m_bAutoDelete );

			DWORD dwWaitRes = WaitForSingleObject(s_pSocketThread->m_hThread, 1300);
			if (dwWaitRes == WAIT_TIMEOUT)
			{
				TRACE("*** Failed to wait for websocket thread termination - Timeout\n");
				VERIFY( TerminateThread(s_pSocketThread->m_hThread, (DWORD)-1) );
				VERIFY( CloseHandle(s_pSocketThread->m_hThread) );
			}
			else if (dwWaitRes == -1)
			{
				TRACE("*** Failed to wait for websocket thread termination - Error %u\n", GetLastError());
				ASSERT(0); // probable invalid thread handle
			}
		}
		delete s_pSocketThread;
		s_pSocketThread = NULL;
	}
	if (s_hTerminate){
		VERIFY( CloseHandle(s_hTerminate) );
		s_hTerminate = NULL;
	}
}