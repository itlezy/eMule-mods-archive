//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "stdafx.h"
#include "IrcSocket.h"
#include "AsyncProxySocketLayer.h"
#include "packets.h"
#include "opcodes.h"
#include "IrcMain.h"
#include "emule.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CIrcSocket::CIrcSocket(CIrcMain* pIrcMain) : CAsyncSocketEx()
{
	m_pIrcMain = pIrcMain;
	m_pProxyLayer = NULL;
}

CIrcSocket::~CIrcSocket()
{
	RemoveAllLayers();
}

BOOL CIrcSocket::Create(UINT nSocketPort, int iSocketType, long lEvent, LPCSTR lpszSocketAddress)
{
	EMULE_TRY

	const ProxySettings	&settings = g_App.m_pPrefs->GetProxySettings();

	if (settings.m_bUseProxy && settings.m_nType != PROXYTYPE_NOPROXY)
	{
		m_pProxyLayer = new CAsyncProxySocketLayer;
		switch (settings.m_nType)
		{
			case PROXYTYPE_SOCKS4:
			case PROXYTYPE_SOCKS4A:
				m_pProxyLayer->SetProxy(settings.m_nType, settings.m_strName, settings.m_uPort);
				break;
			case PROXYTYPE_SOCKS5:
			case PROXYTYPE_HTTP11:
				if (settings.m_bEnablePassword)
					m_pProxyLayer->SetProxy(settings.m_nType, settings.m_strName, settings.m_uPort, settings.m_strUser, settings.m_strPassword);
				else
					m_pProxyLayer->SetProxy(settings.m_nType, settings.m_strName, settings.m_uPort);
				break;
			default:
				ASSERT(0);
		}
		AddLayer(m_pProxyLayer);
	}

	return CAsyncSocketEx::Create(nSocketPort, iSocketType, lEvent, lpszSocketAddress);

	EMULE_CATCH

	return false;
}

void CIrcSocket::Connect()
{
	CAsyncSocketEx::Connect(CStringA(g_App.m_pPrefs->GetIRCServer()), 6667);
}

void CIrcSocket::OnReceive(int iErrorCode) 
{
	if (iErrorCode)
	{
		AddLogLine(LOG_RGB_ERROR, _T("IRC socket: Failed to read - %s"), GetErrorMessage(iErrorCode, 1));
		return;
	}

	int		iLength;
	char	acBuffer[1024];

	do
	{
		iLength = Receive(acBuffer, sizeof(acBuffer) - 1);
		if (iLength < 0)
		{
			AddLogLine(LOG_RGB_ERROR, _T("IRC socket: Failed to read - %s"), GetErrorMessage(GetLastError(), 1));
			return;
		}
		if (iLength > 0)
		{
			g_App.m_pDownloadQueue->AddDownDataOverheadOther(iLength);
			m_pIrcMain->PreParseMessage(acBuffer, iLength);
		}
	}
	while (iLength > 1022);
}

void CIrcSocket::OnConnect(int iErrorCode)
{
	if (iErrorCode)
	{
		AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, _T("IRC socket: Failed to connect - %s"), GetErrorMessage(iErrorCode, 1));
		m_pIrcMain->Disconnect();		
		return;
	}
	m_pIrcMain->SetConnectStatus(true);
	m_pIrcMain->SendLogin();
}

void CIrcSocket::OnClose(int iErrorCode)
{
	if (iErrorCode)
	{
		AddLogLine(LOG_RGB_ERROR, _T("IRC socket: Failed to close - %s"), GetErrorMessage(iErrorCode, 1));
		return;
	}
	RemoveAllLayers();
	m_pIrcMain->SetConnectStatus(false);
	CAsyncSocketEx::Close();
}

int CIrcSocket::SendString(const CString &strMsg)
{
	CStringA		strMessageA;
	int				iSize;
	ECodingFormat	eCF = g_App.m_pPrefs->GetIrcMessageEncodingUTF8() ? cfUTF8 : cfLocalCodePage;

	Str2MB(eCF, &strMessageA, strMsg);
	strMessageA += "\r\n";
	iSize = strMessageA.GetLength();

	g_App.m_pUploadQueue->AddUpDataOverheadOther(iSize);
	return Send(strMessageA, iSize);
}

void CIrcSocket::RemoveAllLayers()
{
	CAsyncSocketEx::RemoveAllLayers();
	
	delete m_pProxyLayer;
	m_pProxyLayer = NULL;
}

int CIrcSocket::OnLayerCallback(const CAsyncSocketExLayer *pLayer, int nType, int nCode, WPARAM wParam, LPARAM lParam)
{
	EMULE_TRY

	if (nType == LAYERCALLBACK_LAYERSPECIFIC)
	{
		ASSERT( pLayer );
		if (pLayer == m_pProxyLayer)
		{
			CString	strError(GetProxyError(nCode));

			switch (nCode)
			{
				case PROXYERROR_NOCONN:
				case PROXYERROR_REQUESTFAILED:
					if (lParam)
					{
						strError += _T(" - ");
						strError += (LPCSTR)lParam;
					}
					if (wParam)
					{
						CString	strErrInf;

						if (GetErrorMessage(wParam, strErrInf, 1))
						{
							strError += _T(" - ");
							strError += strErrInf;
						}
					}
				default:
					AddLogLine(LOG_RGB_ERROR, _T("IRC socket: %s"), strError);
			}
		}
	}

	EMULE_CATCH

	return 1;
}
