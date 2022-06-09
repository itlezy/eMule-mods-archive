// Generic WebServer class, extendable by inheritance
// All configuration and templates are XML+XSLT

#include "StdAfx.h"
#include "WebServer.h"
#include "WebSocket.h"
#include <time.h>

CWebServer::CWebServer(void)
{
	m_bServerWorking = false;

	m_spSkeleton	= NULL;
	m_spConfig		= NULL;
}

CWebServer::~CWebServer(void)
{
	Stop();
}

bool CWebServer::Start()
{
	EMULE_TRY
	if(m_bServerWorking != WEBSERVER_ENABLED)
		m_bServerWorking = WEBSERVER_ENABLED;
	else
		return false;

	if (m_bServerWorking)
	{
		if(!LoadTemplates())
			return false;
		if (m_bServerWorking)
		{
			StartSockets(this);
//			m_nIntruderDetect = 0;
//			m_nStartTempDisabledTime = 0;
//			m_bIsTempDisabled = false;
		}
	}
	else
		StopSockets();

/*	if(WEBSERVER_ENABLED && m_bServerWorking)
		AddLogLine(false, _T("%s: %s"),_GetPlainResString(IDS_PW_HTTPD), _GetPlainResString(IDS_ENABLED).MakeLower());
	else
		AddLogLine(false, _T("%s: %s"),_GetPlainResString(IDS_PW_HTTPD), _GetPlainResString(IDS_DISABLED).MakeLower());
*/
	return true;
	EMULE_CATCH
	return false;
}

void CWebServer::Stop()
{
	EMULE_TRY
	if (m_bServerWorking)
	{
		m_bServerWorking = false;
		StopSockets();
	}
	EMULE_CATCH
}

void CWebServer::Restart()
{
	EMULE_TRY
	StopSockets();
	if (m_bServerWorking)
		StartSockets(this);
	EMULE_CATCH
}

bool CWebServer::LoadTemplates()
{
	EMULE_TRY
	// Load skeleton
	CString sSkeleton = CString(WEBSERVER_DIR) + _T("skeleton.xml");
	m_spSkeleton = XmlLoadDocumentFromFile((LPCTSTR)sSkeleton);
	if(m_spSkeleton == NULL)
		return false;
	// Do we have a root template?
	XmlNodes spNodes = m_spSkeleton->selectNodes(_T(".//template[@for='/']"));
	if(spNodes->length == 0)
		return false;
	// Load configuration
	CString sConfig = CString(WEBSERVER_DIR) + _T("config.xml");
	m_spConfig = XmlLoadDocumentFromFile((LPCTSTR)sConfig);
	if(m_spConfig == NULL)
		return false;
	return true;
	EMULE_CATCH
	return false;
}

XmlDoc CWebServer::CreateFromTemplate(const vector<CString> arrParams, const CString sArgs)
{
	EMULE_TRY
	if(m_spSkeleton == NULL)
		return NULL;
	// Find element
	CString sFind;
	sFind.Format(_T(".//template[@for='%s']"), arrParams[0]);
	XmlNode spNode = m_spSkeleton->selectSingleNode(bstr_t(sFind));
	if(spNode == NULL)
		return NULL;
	// Process template
	XmlNodes spNodes = spNode->childNodes;
	if(spNodes->length == 0)
		return NULL;
	XmlDoc spDoc = XmlCreateDocument();
	XmlInsertProcessingInstruction(spDoc, _T("xml"), _T("version='1.0' encoding='utf-8'"));
	XmlInsertProcessingInstruction(spDoc, _T("xml-stylesheet"), _T("type='text/xsl' href='/file/stylesheet.xsl'"));
	XmlElement spRoot = XmlCreateElement(spDoc, NULL, _T("emule"));
	for(int i = 0; i < spNodes->length; i++)
	{
		XmlElement spElem = spNodes->item[i];
		CString sType = XmlGetAttributeStr(spElem, _T("type"));
		if(sType == _T("dynamic"))
			ProcessDynamicItem(spDoc, XmlGetAttributeStr(spElem, _T("name")), arrParams, sArgs);
		else
			XmlCreateElement(spDoc, spRoot, XmlGetAttributeStr(spElem, _T("name")));
	}
	ProcessFinalize(spDoc, arrParams, sArgs);
	return spDoc;
	EMULE_CATCH
	return NULL;
}

CString CWebServer::GetContentType(CString sExt)
{
	EMULE_TRY
	if(m_spConfig == NULL)
		return _T("");
	CString sFind; sFind.Format(_T(".//files_allowed/item[@ext='%s']"), sExt);
	XmlElement spElem = m_spConfig->selectSingleNode(bstr_t(sFind));
	if(spElem)
		return XmlGetAttributeStr(spElem, _T("type"));
	EMULE_CATCH
	return _T("");
}

// ATL/MFC CString::Tokenize, fast-converted for WTL by kuchin
CString WTLTokenize(LPCTSTR sSrc, LPCTSTR pszTokens, int& iStart )
{
	ATLASSERT( iStart >= 0 );
		
	if(iStart < 0)
		AtlThrow(E_INVALIDARG);			
		
	if( pszTokens == NULL )
		return sSrc;

	LPCTSTR pszPlace = sSrc+iStart;
	LPCTSTR pszEnd = sSrc+CString(sSrc).GetLength();
	if( pszPlace < pszEnd )
	{
		int nIncluding = (int)_mbsspn( reinterpret_cast< const unsigned char* >( pszPlace ), reinterpret_cast< const unsigned char* >( pszTokens ) );

		if( (pszPlace+nIncluding) < pszEnd )
		{
			pszPlace += nIncluding;
			int nExcluding = (int)_mbscspn( reinterpret_cast< const unsigned char* >( pszPlace ), reinterpret_cast< const unsigned char* >( pszTokens ) );

			int iFrom = iStart+nIncluding;
			int nUntil = nExcluding;
			iStart = iFrom+nUntil+1;

			return CString(sSrc).Mid(iFrom, nUntil);
		}
	}

	// return empty string, done tokenizing
	iStart = -1;

	return sSrc;
}

void CWebServer::ProcessURL(const ThreadData Data)
{
	EMULE_TRY

	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return;

//	SetThreadLocale(g_eMuleApp.m_pGlobPrefs->GetLanguageID());

	CoInitialize(NULL);

	EMULE_TRY
	bool isUseGzip = WEBSERVER_GZIP;
	CString sOut;
//	CString OutE;	// List Entry Templates
//	CString OutS;	// ServerStatus Templates
	TCHAR *gzipOut = NULL;
	long gzipLen=0;

	srand ( time(NULL) );
	DWORD dwCurTick = ::GetTickCount();

	vector<CString> arrParams;
	int nPos = 0;
	while(nPos != -1 && nPos < Data.sURL.GetLength())
		arrParams.push_back(WTLTokenize(Data.sURL, _T("/"), nPos));

	XmlDoc spDoc = CreateFromTemplate(arrParams, Data.sArgs);
	if(spDoc != NULL)
		sOut = (LPCTSTR)spDoc->xml;

/*	long lSession = 0;
	if(!_ParseURL(Data.sURL, _T("ses")).IsEmpty())
		lSession = _tstol(_ParseURL(Data.sURL, _T("ses")));

	if(pThis->GetIsTempDisabled() &&
		((dwCurTick - pThis->m_nStartTempDisabledTime) > (g_eMuleApp.m_pGlobPrefs->GetWSTempDisableLogin() * 60000)))
	{
		pThis->m_bIsTempDisabled = false;
		pThis->m_nStartTempDisabledTime = 0;
		AddLogLine(true, RGB_LOG_NOTICE + GetResString(IDS_WEB_INTRBLOCKEND));
	}

	if ((_ParseURL(Data.sURL, _T("w")) == "password") && !pThis->GetIsTempDisabled())
	{
		byte	abyteDigest[16], abyteWSPass[16];
		CString	t_ulCurIP;
		t_ulCurIP.Format(_T("%u.%u.%u.%u"),(byte)pThis->m_ulCurIP,(byte)(pThis->m_ulCurIP>>8),(byte)(pThis->m_ulCurIP>>16),(byte)(pThis->m_ulCurIP>>24));

		if (md4cmp(MD5Sum(_ParseURL(Data.sURL, _T("p")), abyteDigest), g_eMuleApp.m_pGlobPrefs->GetWSPass(abyteWSPass)) == 0)
		{
			Session ses;
			ses.admin=true;
			ses.startTime = CTime::GetCurrentTime();
			ses.lSession = lSession = rand() * 10000L + rand();
			pThis->m_Params.Sessions.Add(ses);
			AddLogLine(true, RGB_LOG_NOTICE + GetResString(IDS_WEB_ADMINLOGIN), t_ulCurIP);
			pThis->m_nIntruderDetect = 0;
		}
		else if ( g_eMuleApp.m_pGlobPrefs->GetWSIsLowUserEnabled() &&
			(md4cmp(abyteDigest, g_eMuleApp.m_pGlobPrefs->GetWSLowPass(abyteWSPass)) == 0) )
		{
			Session ses;
			ses.admin=false;
			ses.startTime = CTime::GetCurrentTime();
			ses.lSession = lSession = rand() * 10000L + rand();
			pThis->m_Params.Sessions.Add(ses);
			AddLogLine(true, RGB_LOG_NOTICE + GetResString(IDS_WEB_GUESTLOGIN), t_ulCurIP);
			pThis->m_nIntruderDetect = 0;
		}
		else if(g_eMuleApp.m_pGlobPrefs->IsWSIntruderDetectionEnabled())
		{
			pThis->m_nIntruderDetect++;
			AddLogLine(true, RGB_LOG_NOTICE + GetResString(IDS_WEB_INTRTRIESLEFT), t_ulCurIP, g_eMuleApp.m_pGlobPrefs->GetWSLoginAttemptsAllowed()-(pThis->m_nIntruderDetect));
			if (pThis->m_nIntruderDetect == g_eMuleApp.m_pGlobPrefs->GetWSLoginAttemptsAllowed())
			{
				AddLogLine(true, RGB_LOG_NOTICE + GetResString(IDS_WEB_INTRDETECTION), pThis->m_nIntruderDetect);
				if (g_eMuleApp.m_pGlobPrefs->GetWSTempDisableLogin() > 0)
				{
					pThis->m_nStartTempDisabledTime = ::GetTickCount();
					pThis->m_bIsTempDisabled = true;
					pThis->AddLogLine(true, RGB_LOG_NOTICE + GetResString(IDS_WEB_INTRBLOCKTIME), g_eMuleApp.m_pGlobPrefs->GetWSTempDisableLogin());
					pThis->m_nIntruderDetect = 0;
					CString MessageText;
					MessageText.Format(GetResString(IDS_WEB_INTRBLOCKTIME), g_eMuleApp.m_pGlobPrefs->GetWSTempDisableLogin());
					g_eMuleApp.m_pSMTPConnection->SendMail(MessageText, g_eMuleApp.m_pGlobPrefs->GetNotifierPopOnWebServerError(), g_eMuleApp.m_pGlobPrefs->IsSMTPWarningEnabled());
					g_eMuleApp.m_pdlgEmule->ShowNotifier(MessageText, TBN_WEBSERVER, false, g_eMuleApp.m_pGlobPrefs->GetNotifierPopOnWebServerError());
				}
				else
				{
					g_eMuleApp.m_pGlobPrefs->SetWSIsEnabled(false);
					pThis->m_bServerWorking = false;
					StopSockets();
					AddLogLine(true, RGB_LOG_NOTICE + GetResString(IDS_WEB_INTRWEBDISABLE));
					pThis->m_nIntruderDetect = 0;
					CString MessageText;
					MessageText.Format(GetResString(IDS_WEB_INTRWEBDISABLE));
					g_eMuleApp.m_pSMTPConnection->SendMail(MessageText, g_eMuleApp.m_pGlobPrefs->GetNotifierPopOnWebServerError(), g_eMuleApp.m_pGlobPrefs->IsSMTPWarningEnabled());
					g_eMuleApp.m_pdlgEmule->ShowNotifier(MessageText, TBN_WEBSERVER, false, g_eMuleApp.m_pGlobPrefs->GetNotifierPopOnWebServerError());
				}
			}
		}
		else
			AddLogLine(true, RGB_LOG_NOTICE + GetResString(IDS_WEB_BADLOGINATTEMPT));
		isUseGzip = false;
	}

	CString sSession; sSession.Format(_T("%ld"), lSession);

	if (_ParseURL(Data.sURL, _T("w")) == "logout")
		_RemoveSession(Data, lSession);

	if(_IsLoggedIn(Data, lSession))
	{
		if (_ParseURL(Data.sURL, _T("w")) == "close")
		{
			g_eMuleApp.m_app_state = g_eMuleApp.APP_STATE_SHUTINGDOWN;
			SendMessage(g_eMuleApp.m_pdlgEmule->m_hWnd,WM_CLOSE,0,0);
		}
		else if (_ParseURL(Data.sURL, _T("w")) == "shutdown")
		{
			HANDLE hToken;
			TOKEN_PRIVILEGES tkp;	// Get a token for this process.
			try
			{
				if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
					throw;
				LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
				tkp.PrivilegeCount = 1;  // one privilege to set
				tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;	// Get the shutdown privilege for this process.
				AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
				ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0);
			}
			catch(...)
			{
				AddLogLine(true, RGB_LOG_NOTICE + GetResString(IDS_WEB_SHUTDOWN) + _T(' ') + GetResString(IDS_FAILED));
			}
		}
		else if (_ParseURL(Data.sURL, _T("w")) == "reboot")
		{
			HANDLE hToken;
			TOKEN_PRIVILEGES tkp;	// Get a token for this process.
			try
			{
				if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
					throw;
				LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
				tkp.PrivilegeCount = 1;  // one privilege to set
				tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;	// Get the shutdown privilege for this process.
				AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
				ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0);
			}
			catch(...)
			{
				AddLogLine(true, RGB_LOG_NOTICE + GetResString(IDS_WEB_REBOOT) + _T(' ') + GetResString(IDS_FAILED));
			}
		}

		Out += _GetHeader(Data, lSession);
		CString sPage = _ParseURL(Data.sURL, _T("w"));
		if (sPage == "server")
			Out += _GetServerList(Data);
		else if (sPage == "shared")
			Out += _GetSharedFilesList(Data);
		else if (sPage == "transfer")
			Out += _GetTransferList(Data);
		else if (sPage == "search")
			Out += _GetSearch(Data);
		else if (sPage == "graphs")
			Out += _GetGraphs(Data);
		else if (sPage == "log")
			Out += _GetLog(Data);
		if (sPage == "sinfo")
			Out += _GetServerInfo(Data);
		if (sPage == "debuglog")
			Out += _GetDebugLog(Data);
		if (sPage == "stats")
			Out += _GetStats(Data);
		if (sPage == "options")
		{
			isUseGzip = false;
			Out += _GetPreferences(Data);
		}
		Out += _GetFooter(Data);

		if (sPage.IsEmpty())
			isUseGzip = false;

		if(isUseGzip)
		{
			bool bOk = false;
			try
			{
				uLongf destLen = Out.GetLength() + 1024;
				gzipOut = new TCHAR[destLen];
				if(_GzipCompress((Bytef*)gzipOut, &destLen, (const Bytef*)(TCHAR*)Out.GetBuffer(0), Out.GetLength(), Z_DEFAULT_COMPRESSION) == Z_OK)
				{
					bOk = true;
					gzipLen = destLen;
				}
			}
			catch(...)
			{
			}
			if(!bOk)
			{
				isUseGzip = false;
				if(gzipOut != NULL)
				{
					delete[] gzipOut;
					gzipOut = NULL;
				}
			}
		}
	}
	else
	{
		isUseGzip = false;
		Out += _GetLoginScreen(Data);
	}*/

	// Send answer
	CString sHTTPRes(_T(	//text/xml for dynamic xml
		"Server: eMule\r\n"
		"Expires: Mon, 26 Jul 1997 05:00:00 GMT\r\n"				// Date in the past
		"Cache-Control: no-store, no-cache, must-revalidate\r\n"	// HTTP/1.1
		"Cache-Control: post-check=0, pre-check=0\r\n"				// HTTP/1.1
		"Pragma: no-cache\r\n"										// HTTP/1.0
		"Connection: close\r\n"
		"Content-Type: text/xml\r\n"
		));
	if(!isUseGzip)
		Data.pSocket->SendContent(sHTTPRes, sOut, sOut.GetLength());
	else
		Data.pSocket->SendContent(sHTTPRes + _T("Content-Encoding: gzip\r\n"), gzipOut, gzipLen);
	if(gzipOut != NULL)
		delete[] gzipOut;

	EMULE_CATCH
	CoUninitialize();

	EMULE_CATCH
}

void CWebServer::ProcessFileRequest(const ThreadData Data)
{
	EMULE_TRY
	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return;

	CString sFilename = Data.sURL;
	CString sExt;
	if (sFilename.GetLength() > 4)
	{
		sExt = sFilename.Right(5);
		sExt.MakeLower();
		int nPos = sExt.Find(_T("."));
		if(nPos >= 0 && nPos != (sExt.GetLength() - 1))
		{
			sExt = GetContentType(sExt.Mid(nPos + 1));
			if(sExt.IsEmpty())
				return;
		}
		else
			return;
	}
	CString sContentType = CString(_T("Content-Type: ")) + sExt + _T("\r\n"); //+_T("Last-Modified: ") + pThis->m_Params.sLastModified + _T("\r\n") + _T("ETag: ") + pThis->m_Params.sETag + _T("\r\n");

	// DonGato: expire images after 30 days (can be override by browser)
/*	CString sPrevLocale(setlocale(LC_TIME, NULL));
	_tsetlocale(LC_TIME, _T("English"));
	CTime t = CTime::GetCurrentTime();
	t += 60*60*24*30;
	CString expires = t.FormatGmt("%a, %d %b %Y %H:%M:%S GMT");
	_tsetlocale(LC_TIME, sPrevLocale);
	sContentType += _T("Expires: ") + expires + _T("\r\n");
*/
	sFilename.Delete(0);
	sFilename.Replace('/','\\');
	sFilename = CString(WEBSERVER_DIR) + sFilename;
	CFile file;
	if(file.Open(sFilename))
	{
		BYTE* buffer = new BYTE[file.GetSize()];
		DWORD size = 0;
		file.Read(buffer,file.GetSize(), &size);
		file.Close();
		Data.pSocket->SendContent(sContentType, buffer, size);
		delete[] buffer;
	}
	EMULE_CATCH
}

CString CWebServer::ReadParam(const CString sData, const CString sParamName)
{
	EMULE_TRY

	CString sValue = _T("");
	CString	sParam = sParamName;
	CString	sParameter = sData;

	// Search the parameter beginning / middle and strip the rest...
	sParam += _T('=');
	int i, iFindPos = -1, iFindLength = 0;
	if (sParameter.Find(sParam) == 0)
	{
		iFindPos = 0;
		iFindLength = sParam.GetLength();
	}
	if ((i = sParameter.Find(_T('&') + sParam)) >= 0)
	{
		iFindPos = i;
		iFindLength = sParam.GetLength() + 1;
	}
	if (iFindPos >= 0)
	{
		TCHAR fromReplace[4] = { _T('%') };	// decode URL
		TCHAR toReplace[2] = _T("\0");	// decode URL

		sParameter = sParameter.Mid(iFindPos + iFindLength, sParameter.GetLength());
		if ((iFindPos = sParameter.Find(_T('&'))) >= 0)
			sParameter = sParameter.Mid(0, iFindPos);

		sValue = sParameter;

		// Decode
		sValue.Replace(_T('+'), _T(' '));
		for (i = 0; i <= 255; i++)
		{
			_stprintf(&fromReplace[1], _T("%02x"), i);
			toReplace[0] = static_cast<TCHAR>(i);
			sValue.Replace(fromReplace, toReplace);
			_stprintf(&fromReplace[1], _T("%02X"), i);
			sValue.Replace(fromReplace, toReplace);
		}
	}

	return sValue;

	EMULE_CATCH

	return _T("");
}
