#pragma once

#include "WebServerImpl.h"

void StartSocketsImpl(CWebServerImpl *pThis);
void StartSockets(CWebServer *pThis);
void StopSockets();

class CWebSocket 
{
public:
	void SetParent(CWebServerImpl *);
	CWebServerImpl* m_pParent;

	class CChunk 
	{
	public:
		char* m_pData;
		char* m_pToSend;
		DWORD m_dwSize;
		CChunk* m_pNext;

		~CChunk() { if (m_pData) delete[] m_pData; }
	};

	CChunk* m_pHead; // tails of what has to be sent
	CChunk* m_pTail;

	TCHAR* m_pBuf;
	DWORD m_dwRecv;
	DWORD m_dwBufSize;
	DWORD m_dwHttpHeaderLen;
	DWORD m_dwHttpContentLen;

	bool m_bCanRecv;
	bool m_bCanSend;
	bool m_bValid;
	SOCKET m_hSocket;

	void OnReceived(void* pData, DWORD dwDataSize); // must be implemented
	void SendData(const void* pData, DWORD dwDataSize);
	void SendData(LPCTSTR szText) { SendData(szText, lstrlen(szText)); }
	void SendContent(LPCTSTR szStdResponse, const void* pContent, DWORD dwContentSize);
	void SendTextContent(LPCTSTR szText) { SendContent(_T(""), szText, lstrlen(szText)); }
	void Disconnect();

	void OnRequestReceived(TCHAR* pHeader, DWORD dwHeaderLen, TCHAR* pData, DWORD dwDataLen);
};

