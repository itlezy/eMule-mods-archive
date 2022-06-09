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
#include "emule.h"
#include "updownclient.h"
#include "zlib/zlib.h"
#include "WebServer.h"
#include "WebSocket.h"
#include "PartFile.h"
#include "ED2KLink.h"
#include "MD5Sum.h"
#include "ini2.h"
#include "QArray.h"
#include "HTRichEditCtrl.h"
#include "SharedFileList.h"
#include "ServerList.h"
#include "server.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define HTTPInit "Cache-Control: no-cache, no-store, must-revalidate, max-age=0, private\r\nPragma: no-cache\r\nContent-Type: text/html\r\n"

#define WEB_SERVER_TEMPLATES_VERSION	1011

typedef struct
{
	uint32		dwResStrId;
	const TCHAR	*pcColIcon;
	const TCHAR	*pcColHdr;
	const TCHAR	*pcColMenu;
} WSListDefinition;

//SyruS CQArray-Sorting operators
bool operator > (QueueUsers & first, QueueUsers & second)
{
	return (first.sIndex.CompareNoCase(second.sIndex) > 0);
}

bool operator < (QueueUsers & first, QueueUsers & second)
{
	return (first.sIndex.CompareNoCase(second.sIndex) < 0);
}

CWebServer::CWebServer(void)
{
	m_pWSPrefs = g_App.m_pPrefs->GetWSPrefsPtr();

	m_bServerWorking = false;
	m_nIntruderDetect = 0;
	m_nStartTempDisabledTime = 0;
	m_bIsTempDisabled = false;
}

void CWebServer::ReloadTemplates()
{
	EMULE_TRY

	CString	strTmp, sFile = g_App.m_pPrefs->GetTemplate();

	if (sFile.IsEmpty() || (sFile.CompareNoCase(_T("emulexp.tmpl")) == 0))
	{
		sFile = g_App.m_pPrefs->GetAppDir();
		sFile += _T("eMuleXP.tmpl");
	}

	CStdioFile file;
	if (file.Open(sFile, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText | CFile::osSequentialScan))
	{
		CString sAll;

		setvbuf(file.m_pStream, NULL, _IOFBF, 16*1024);
		for(;;)
		{
			if(!file.ReadString(strTmp))
				break;

			sAll += strTmp;
			sAll += _T('\n');
		}
		file.Close();

		long lVersion = _tstol(_LoadTemplate(sAll, _T("TMPL_VERSION")));

		if(lVersion < WEB_SERVER_TEMPLATES_VERSION)
		{
			if(g_App.m_pPrefs->GetWSIsEnabled() || m_bServerWorking)
				AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_WS_ERR_LOADTEMPLATE, sFile);
			if (m_bServerWorking)
				StopSockets();
			m_bServerWorking = false;
			g_App.m_pPrefs->SetWSIsEnabled(false);
		}
		else
		{
			m_Templates.sHeader = _LoadTemplate(sAll,_T("TMPL_HEADER"));
			m_Templates.sHeaderStylesheet = _LoadTemplate(sAll,_T("TMPL_HEADER_STYLESHEET"));
			m_Templates.sFooter = _LoadTemplate(sAll,_T("TMPL_FOOTER"));
			m_Templates.sServerList = _LoadTemplate(sAll,_T("TMPL_SERVER_LIST"));
			m_Templates.sServerLine = _LoadTemplate(sAll,_T("TMPL_SERVER_LINE"));
			m_Templates.sTransferImages = _LoadTemplate(sAll,_T("TMPL_TRANSFER_IMAGES"));
			m_Templates.sTransferList = _LoadTemplate(sAll,_T("TMPL_TRANSFER_LIST"));
			m_Templates.sTransferDownHeader = _LoadTemplate(sAll,_T("TMPL_TRANSFER_DOWN_HEADER"));
			m_Templates.sTransferDownFooter = _LoadTemplate(sAll,_T("TMPL_TRANSFER_DOWN_FOOTER"));
			m_Templates.sTransferDownLine = _LoadTemplate(sAll,_T("TMPL_TRANSFER_DOWN_LINE"));
			m_Templates.sTransferUpHeader = _LoadTemplate(sAll,_T("TMPL_TRANSFER_UP_HEADER"));
			m_Templates.sTransferUpFooter = _LoadTemplate(sAll,_T("TMPL_TRANSFER_UP_FOOTER"));
			m_Templates.sTransferUpLine = _LoadTemplate(sAll,_T("TMPL_TRANSFER_UP_LINE"));
			m_Templates.sTransferUpQueueShow = _LoadTemplate(sAll,_T("TMPL_TRANSFER_UP_QUEUE_SHOW"));
			m_Templates.sTransferUpQueueHide = _LoadTemplate(sAll,_T("TMPL_TRANSFER_UP_QUEUE_HIDE"));
			m_Templates.sTransferUpQueueLine = _LoadTemplate(sAll,_T("TMPL_TRANSFER_UP_QUEUE_LINE"));
			m_Templates.sTransferUpQueueBannedShow = _LoadTemplate(sAll,_T("TMPL_TRANSFER_UP_QUEUE_BANNED_SHOW"));
			m_Templates.sTransferUpQueueBannedHide = _LoadTemplate(sAll,_T("TMPL_TRANSFER_UP_QUEUE_BANNED_HIDE"));
			m_Templates.sTransferUpQueueBannedLine = _LoadTemplate(sAll,_T("TMPL_TRANSFER_UP_QUEUE_BANNED_LINE"));
			m_Templates.sTransferUpQueueFriendShow = _LoadTemplate(sAll,_T("TMPL_TRANSFER_UP_QUEUE_FRIEND_SHOW"));
			m_Templates.sTransferUpQueueFriendHide = _LoadTemplate(sAll,_T("TMPL_TRANSFER_UP_QUEUE_FRIEND_HIDE"));
			m_Templates.sTransferUpQueueFriendLine = _LoadTemplate(sAll,_T("TMPL_TRANSFER_UP_QUEUE_FRIEND_LINE"));
			m_Templates.sTransferUpQueueCreditShow = _LoadTemplate(sAll,_T("TMPL_TRANSFER_UP_QUEUE_CREDIT_SHOW"));
			m_Templates.sTransferUpQueueCreditHide = _LoadTemplate(sAll,_T("TMPL_TRANSFER_UP_QUEUE_CREDIT_HIDE"));
			m_Templates.sTransferUpQueueCreditLine = _LoadTemplate(sAll,_T("TMPL_TRANSFER_UP_QUEUE_CREDIT_LINE"));
			m_Templates.sSharedList = _LoadTemplate(sAll,_T("TMPL_SHARED_LIST"));
			m_Templates.sSharedLine = _LoadTemplate(sAll,_T("TMPL_SHARED_LINE"));
			m_Templates.sGraphs = _LoadTemplate(sAll,_T("TMPL_GRAPHS"));
			m_Templates.sLog = _LoadTemplate(sAll,_T("TMPL_LOG"));
			m_Templates.sServerInfo = _LoadTemplate(sAll,_T("TMPL_SERVERINFO"));
			m_Templates.sDebugLog = _LoadTemplate(sAll,_T("TMPL_DEBUGLOG"));
			m_Templates.sStats = _LoadTemplate(sAll,_T("TMPL_STATS"));
			m_Templates.sPreferences = _LoadTemplate(sAll,_T("TMPL_PREFERENCES"));
			m_Templates.sLogin = _LoadTemplate(sAll,_T("TMPL_LOGIN"));
			m_Templates.sAddServerBox = _LoadTemplate(sAll,_T("TMPL_ADDSERVERBOX"));
			m_Templates.sSearch = _LoadTemplate(sAll,_T("TMPL_SEARCH"));
			m_Templates.dwProgressbarWidth = _tstoi(_LoadTemplate(sAll,_T("PROGRESSBARWIDTH")));
			m_Templates.sSearchHeader = _LoadTemplate(sAll,_T("TMPL_SEARCH_RESULT_HEADER"));
			m_Templates.sSearchResultLine = _LoadTemplate(sAll,_T("TMPL_SEARCH_RESULT_LINE"));
			m_Templates.sProgressbarImgs = _LoadTemplate(sAll,_T("PROGRESSBARIMGS"));
			m_Templates.sProgressbarImgsPercent = _LoadTemplate(sAll,_T("PROGRESSBARPERCENTIMG"));
			m_Templates.sCatArrow= _LoadTemplate(sAll, _T("TMPL_CATARROW"));
			m_Templates.sDownArrow= _LoadTemplate(sAll, _T("TMPL_DOWNARROW"));
			m_Templates.sUpArrow= _LoadTemplate(sAll, _T("TMPL_UPARROW"));
			m_Templates.strDownDoubleArrow = _LoadTemplate(sAll, _T("TMPL_DNDOUBLEARROW"));
			m_Templates.strUpDoubleArrow = _LoadTemplate(sAll, _T("TMPL_UPDOUBLEARROW"));

			m_Templates.sProgressbarImgsPercent.Replace(_T("[PROGRESSGIFNAME]"),_T("%s"));
			m_Templates.sProgressbarImgsPercent.Replace(_T("[PROGRESSGIFINTERNAL]"),_T("%u"));
			m_Templates.sProgressbarImgs.Replace(_T("[PROGRESSGIFNAME]"),_T("%s"));
			m_Templates.sProgressbarImgs.Replace(_T("[PROGRESSGIFINTERNAL]"),_T("%i"));
		}
	}
	else if(m_bServerWorking)
	{
		AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_WEB_ERR_CANTLOAD, sFile);
		StopSockets();
		m_bServerWorking = false;
		g_App.m_pPrefs->SetWSIsEnabled(false);
	}

	EMULE_CATCH
}

CWebServer::~CWebServer(void)
{
	if (m_bServerWorking) StopSockets();
}

CString CWebServer::_LoadTemplate(const CString &sAll, const TCHAR *pcTemplateName)
{
	EMULE_TRY

	CString	sRet, strTmp = _T("<--");

	strTmp += pcTemplateName;

	int nStart = sAll.Find(strTmp + _T("-->"));
	int nEnd = sAll.Find(strTmp + _T("_END-->"));
	if ((nStart >= 0) && (nEnd >= 0) && (nStart < nEnd))
	{
		nStart += _tcslen(pcTemplateName) + 7;
		sRet = sAll.Mid(nStart, nEnd - nStart - 1);
	}
	else
		AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("Webserver: can't load %s template"), pcTemplateName);

	return sRet;

	EMULE_CATCH

	return _T("");
}

void CWebServer::RestartServer()
{	//Cax2 - restarts the server with the new port settings
	StopSockets();
	if (m_bServerWorking)
		StartSockets(this);
}

void CWebServer::StartServer(void)
{
	EMULE_TRY

	if (m_bServerWorking == g_App.m_pPrefs->GetWSIsEnabled())
		return;
	m_bServerWorking = g_App.m_pPrefs->GetWSIsEnabled();

	if (m_bServerWorking)
	{
		ReloadTemplates();
		if (m_bServerWorking)
		{
			StartSockets(this);
			m_nIntruderDetect = 0;
			m_nStartTempDisabledTime = 0;
			m_bIsTempDisabled = false;
		}
	}
	else
		StopSockets();

	if(g_App.m_pPrefs->GetWSIsEnabled() && m_bServerWorking)
		AddLogLine(0, _T("%s: %s"), _GetPlainResString(IDS_PW_HTTPD), _GetPlainResString(IDS_ENABLED).MakeLower());
	else
		AddLogLine(0, _T("%s: %s"), _GetPlainResString(IDS_PW_HTTPD), _GetPlainResString(IDS_DISABLED).MakeLower());

	EMULE_CATCH
}

void CWebServer::_RemoveServer(const CString &strIP, uint16 uPort)
{
	EMULE_TRY

	CServer	*server = g_App.m_pServerList->GetServerByAddress(strIP, uPort);
	if (server != NULL)
		g_App.m_pMDlg->SendMessage(WEB_REMOVE_SERVER, (WPARAM)server, NULL);

	EMULE_CATCH
}

void CWebServer::_AddToStatic(const CString &strIP, uint16 uPort)
{
	EMULE_TRY

	CServer	*server = g_App.m_pServerList->GetServerByAddress(strIP, uPort);
	if (server != NULL)
		g_App.m_pMDlg->SendMessage(WEB_ADD_TO_STATIC, (WPARAM)server, NULL);

	EMULE_CATCH
}

void CWebServer::_RemoveFromStatic(const CString &strIP, uint16 uPort)
{
	EMULE_TRY

	CServer	*server = g_App.m_pServerList->GetServerByAddress(strIP, uPort);
	if (server != NULL)
		g_App.m_pMDlg->SendMessage(WEB_REMOVE_FROM_STATIC, (WPARAM)server, NULL);

	EMULE_CATCH
}

void CWebServer::AddStatsLine(UpDown *line)
{
	EMULE_TRY

	if (PointsForWeb.GetCount() >= WEB_GRAPH_WIDTH)
		PointsForWeb.RemoveAt(0);
	PointsForWeb.Add(*line);

	EMULE_CATCH
}

CString CWebServer::_SpecialChars(CString str, bool noquote /*=false*/)
{
	EMULE_TRY

	str.Replace(_T("&"), _T("&amp;"));
	str.Replace(_T("<"), _T("&lt;"));
	str.Replace(_T(">"), _T("&gt;"));
	str.Replace(_T("\""), _T("&quot;"));
	if(noquote)
	{
		str.Replace(_T("'"), _T("&#8217;"));
		str.Replace(_T("\n"), _T("\\n"));
	}
	return str;

	EMULE_CATCH

	return _T("");
}

void CWebServer::SpecialChars(CString *str)
{
	str->Replace(_T("&"), _T("&amp;"));
	str->Replace(_T("<"), _T("&lt;"));
	str->Replace(_T(">"), _T("&gt;"));
	str->Replace(_T("\""), _T("&quot;"));
}

void CWebServer::_ConnectToServer(const CString &strIP, uint16 uPort)
{
	EMULE_TRY

	CServer	*server = g_App.m_pServerList->GetServerByAddress(strIP, uPort);

	if (server != NULL)
		g_App.m_pMDlg->SendMessage(WEB_CONNECT_TO_SERVER, (WPARAM)server, NULL);

	EMULE_CATCH
}

void CWebServer::ProcessFileReq(const ThreadData &Data)
{
	EMULE_TRY

	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return;

	CString		filename = Data.sURL;
	CString		strExt = filename.Right(4).MakeLower();
	CStringA	strHdr;
	const char	*pcContType = "";
	bool		bGzip = false;

	if (strExt == ".gif")
		pcContType = "image/gif";
	else if (strExt == ".jpg" || filename.Right(5).MakeLower() == ".jpeg")
		pcContType = "image/jpg";
	else if (strExt == ".bmp")
	{
		pcContType = "image/bmp";
		bGzip = true;
	}
	else if (strExt == ".png")
		pcContType = "image/png";
	else if (strExt == ".ico")
	{
		pcContType = "image/x-icon";
		bGzip = true;
	}
	else if (strExt == ".css")
	{
		pcContType = "text/css";
		bGzip = true;
	}
	else if (filename.Right(3).MakeLower() == ".js")
	{
		pcContType = "text/javascript";
		bGzip = true;
	}
	else if (strExt == ".xml")
	{
		pcContType = "text/xml";
		bGzip = true;
	}
	else if (strExt == ".txt")
	{
		pcContType = "text/plain";
		bGzip = true;
	}

	bGzip = bGzip && (Data.strAcceptEncoding.Find(_T("gzip")) >= 0);
#if 0
	if (bGzip)
		AddLogLine(LOG_FL_DBG, _T("WebServer: File Request (name=%s) Should do gzip (AcceptEncoding=%s)"), filename, Data.strAcceptEncoding);
	else
		AddLogLine(LOG_FL_DBG, _T("WebServer: File Request (name=%s) Should NOT do gzip (AcceptEncoding=%s)"), filename, Data.strAcceptEncoding);
#endif

//	HTTP/1.x response header "Content-Type:"
	strHdr.Format("Content-Type: %s\r\n", pcContType);

// Relative path to file from the eMule application folder
	filename.Delete(0);
	filename.Replace(_T('/'), _T('\\'));
	filename = g_App.m_pPrefs->GetAppDir() + _T("WebServer\\") + filename;

	CFile file;
	if (file.Open(filename, CFile::modeRead | CFile::shareDenyWrite | CFile::typeBinary))
	{
		char		acGMT[EP_MAX_HTTPGMTSTR];
		FILETIME	ftModify;
		struct tm	stTm = {0};

	// Get the file's modification time
		if (::GetFileTime(file.m_hFile, NULL, NULL, &ftModify))
		{
			CTime		timeFileMod(ftModify);

			if (timeFileMod.GetGmtTm(&stTm) != NULL)
			{
				Gmt2HttpStr(acGMT, &stTm);
				if (!Data.strIfModifiedSince.IsEmpty() && (CStringA(Data.strIfModifiedSince) == acGMT))
				{
					file.Close();
					Data.pSocket->SendReply(304);
					return;
				}
			//	HTTP/1.x response header "Last-Modified:"
				strHdr.AppendFormat("Last-Modified: %s\r\n", acGMT);
			}
		}

	//	DonGato: expire images after 30 days (can be overwritten by browser)
	//	Controls for file caching expiry time
		const int	iExp_M = 1; //Months
		const int	iExp_D = 0; //Days
		const int	iExp_h = 0; //Hours
		const int	iExp_m = 0; //Minutes
		const int	iExp_s = 0; //Seconds
	//	Calculate expiry time in seconds
		int		iExpireDelta = 60 * (60 * (24 * (30 * iExp_M + iExp_D) + iExp_h) + iExp_m) + iExp_s;
		CTime	t = CTime::GetCurrentTime() + iExpireDelta;

	//	HTTP/1.0 response header "Expires:" and
	//	HTTP/1.1 response header "Cache-Control: max-age"
	//	Overrides "Expires:" only if the client understands it
	//	Is more accurate than "Expires:"
		if (t.GetGmtTm(&stTm) != NULL)
		{
			Gmt2HttpStr(acGMT, &stTm);
		//	HTTP/1.x response header "Last-Modified:"
			strHdr.AppendFormat("Expires: %s\r\nCache-Control: max-age=%u\r\n", acGMT, iExpireDelta);
		}

		uint32	dwFileSz = static_cast<uint32>(file.GetLength());
		char	*buffer = new char[dwFileSz];
		int		size = file.Read(buffer, dwFileSz);

		file.Close();
		Data.pSocket->SendContent(strHdr, buffer, size);
		delete[] buffer;
	}
	else
		Data.pSocket->SendReply(404);	//	File not found

	EMULE_CATCH
}

void CWebServer::ProcessGeneralReq(const ThreadData &Data)
{
	EMULE_TRY

	//(0.29b)//////////////////////////////////////////////////////////////////
	// Here we are in real trouble! We are accessing the entire emule main thread
	// data without any syncronization!! Either we use the message pump for m_pMDlg
	// or use some hundreds of critical sections... For now, an exception handler
	// shoul avoid the worse things.
	//////////////////////////////////////////////////////////////////////////
	CoInitialize(NULL);

	EMULE_TRY

	bool	isUseGzip = (Data.strAcceptEncoding.Find(_T("gzip")) >= 0);	// Enable gzip only if the browser supports it
	CString	Out;
	CStringA	*pstrOut = NULL;
	byte		*gzipOut = NULL;
	unsigned	uiGzipLen = 0;

	DWORD dwCurTick = ::GetTickCount();

	if (GetIsTempDisabled() &&
		((dwCurTick - m_nStartTempDisabledTime) > (g_App.m_pPrefs->GetWSTempDisableLogin() * 60000)))
	{
		m_bIsTempDisabled = false;
		m_nStartTempDisabledTime = 0;
		AddLogLine(LOG_FL_SBAR | LOG_RGB_NOTICE, IDS_WEB_INTRBLOCKEND);
	}

	Session	ses;
	long	lSession = DoAuthentication(Data, &ses);
	CString	strCmd(_ParseURL(Data.sURL, _T("w")));

#if 0
	AddLogLine(LOG_FL_DBG, _T("WebServer: start request processing (Cmd=%s)"), strCmd);
#endif
#if 0
	if (isUseGzip)
		AddLogLine(LOG_FL_DBG, _T("WebServer: Gzip is ON (AcceptEncoding=%s)"), Data.strAcceptEncoding);
	else
		AddLogLine(LOG_FL_DBG, _T("WebServer: Gzip is OFF (AcceptEncoding=%s)"), Data.strAcceptEncoding);
#endif
	if (strCmd == _T("logout"))
		RemoveSession(Data, lSession);

	if (IsLoggedIn(Data, lSession))
	{
		if (strCmd == _T("close"))
		{
			g_App.m_app_state = g_App.APP_STATE_SHUTTINGDOWN;
			SendMessage(g_App.m_pMDlg->m_hWnd, WM_CLOSE, 0, 0);
		}
		else if (strCmd == _T("shutdown"))
		{
			HANDLE hToken;
			TOKEN_PRIVILEGES tkp;	// Get a token for this process
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
				AddLogLine(LOG_FL_SBAR | LOG_RGB_NOTICE, GetResString(IDS_WEB_SHUTDOWN) + _T(' ') + GetResString(IDS_FAILED));
			}
		}
		else if (strCmd == _T("reboot"))
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
				AddLogLine(LOG_FL_SBAR | LOG_RGB_NOTICE, GetResString(IDS_WEB_REBOOT) + _T(' ') + GetResString(IDS_FAILED));
			}
		}

		Out += GetHeader(Data, lSession, strCmd);
		if (strCmd == _T("server"))
			Out += GetServerList(Data);
		else if (strCmd == _T("shared"))
			Out += GetSharedFilesList(Data);
		else if (strCmd == _T("transfer"))
			Out += GetTransferList(Data);
		else if (strCmd == _T("search"))
			Out += GetSearch(Data);
		else if (strCmd == _T("graphs"))
			Out += GetGraphs(Data);
		else if (strCmd == _T("log"))
			Out += GetLog(Data);
		else if (strCmd == _T("sinfo"))
			Out += GetServerInfo(Data);
		else if (strCmd == _T("debuglog"))
			Out += GetDebugLog(Data);
		else if (strCmd == _T("stats"))
			Out += _GetStats(Data);
		else if (strCmd == _T("options"))
			Out += GetPreferences(Data);
		Out += _GetFooter(Data);
	}
	else
	{
		Out += _GetLoginScreen(Data, isUseGzip);
	}

#ifdef _UNICODE
	CStringA strUTF8;

	Str2MB(cfUTF8, &strUTF8, Out);
	pstrOut = &strUTF8;
#else
	pstrOut = &Out;
#endif

	if(isUseGzip)
	{
		bool bOk = false;
		try
		{
			unsigned uiDstLen = pstrOut->GetLength() + 1024;
			gzipOut = new byte[uiDstLen];
			if (GzipCompress((byte*)gzipOut, &uiDstLen, (const byte*)pstrOut->GetBuffer(0), pstrOut->GetLength(), Z_DEFAULT_COMPRESSION) == Z_OK)
			{
				bOk = true;
				uiGzipLen = uiDstLen;
			}
		}
		catch(...)
		{
		}
		if(!bOk)
		{
			AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("WebServer: Gzip FAILED on Cmd=%s (AcceptEncoding=%s)"), strCmd, Data.strAcceptEncoding);
			isUseGzip = false;
		}
	}

#if 0
	AddLogLine(LOG_FL_DBG, _T("WebServer: request prepared (Sz=%u)"), (!isUseGzip) ? pstrOut->GetLength() : uiGzipLen);
#endif
	// send answer ...
	if(!isUseGzip)
		Data.pSocket->SendContent(HTTPInit, pstrOut->GetString(), pstrOut->GetLength());
	else
		Data.pSocket->SendContent(HTTPInit "Content-Encoding: gzip\r\n", gzipOut, uiGzipLen);

	delete[] gzipOut;

#if 0
	AddLogLine(LOG_FL_DBG, _T("WebServer: request sent"));
#endif
	EMULE_CATCH

	CoUninitialize();

	EMULE_CATCH2
}

CString CWebServer::ParseURLArray(const CString &strURL, const TCHAR *pcFieldName)
{
	EMULE_TRY

	CString strRes, strFieldName(pcFieldName);
	int		iIdx, iPos;

	strFieldName += _T('=');
	for (iIdx = 0;;)
	{
		if ((iPos = strURL.Find(strFieldName, iIdx)) < 0)
			break;
		iIdx = iPos + strFieldName.GetLength();
		if ((iPos = strURL.Find(_T('&'), iIdx)) < 0)
			iPos = strURL.GetLength();
		strRes += strURL.Mid(iIdx, iPos - iIdx);
		strRes += _T('|');
	}
	if (!strRes.IsEmpty())
	{
		strRes.Replace(_T('+'), _T(' '));
		strRes = URLDecode(strRes);
	}
	return strRes;

	EMULE_CATCH
	return _T("");
}

CString CWebServer::_ParseURL(const CString &URL, const TCHAR *pcFieldName)
{
	EMULE_TRY

	CString strVal;
	int i, iIdx, findPos, findLength = 0;

	if ((iIdx = URL.Find(_T('?'))) >= 0)
	{
		iIdx++;

		CString	fieldname(pcFieldName);

	// Search the fieldname beginning / middle and strip the rest...
		fieldname += _T('=');
		findPos = -1;
		if (URL.Find(fieldname, iIdx) == iIdx)
		{
			findPos = iIdx;
			findLength = fieldname.GetLength();
		}
		else if ((i = URL.Find(_T('&') + fieldname, iIdx)) >= 0)
		{
			findPos = i;
			findLength = fieldname.GetLength() + 1;
		}
		if (findPos >= 0)
		{
			iIdx = findPos + findLength;
			if ((findPos = URL.Find(_T('&'), iIdx)) < 0)
				findPos = URL.GetLength();
			strVal = URL.Mid(iIdx, findPos - iIdx);

		//	Decode value, CR/LF have to be preserved to allow insertion of multiple ed2k-links
			strVal.Replace(_T('+'), _T(' '));
			strVal = URLDecode(strVal, true);
		}
	}

	return strVal;

	EMULE_CATCH

	return _T("");
}

CString CWebServer::GetHeader(const ThreadData &Data, long lSession, const CString &strCommand)
{
	EMULE_TRY

	CString sSession; sSession.Format(_T("%ld"), lSession);
	CString Out(m_Templates.sHeader);

	Out.Replace(_T("[CharSet]"), GetWebCharSet());

//	Auto-refresh code
	CString sRefresh, sCat, strVersionCheck;
	CString strTmp = _ParseURL(Data.sURL, _T("cat"));
	CString strCmd(strCommand);
	bool bAdmin = IsSessionAdmin(Data,sSession);
	int cat = _tstoi(strTmp);

	if (cat != 0)
		sCat.Format(_T("&cat=%u"), cat);

	if (strCmd == _T("options") || strCmd == _T("stats") || strCmd == _T("password"))
		sRefresh = _T("0");
	else
		sRefresh.Format(_T("%d"), g_App.m_pPrefs->GetWebPageRefresh()*1000);

	strCmd.AppendFormat(_T("&amp;cat=%s&amp;dummy=%u"), strTmp, rand());
	strVersionCheck.Format(_T("http://updates.emuleplus.info/version_check.php?version=%i&language=%i"),CURRENT_PLUS_VERSION,g_App.m_pPrefs->GetLanguageID());

	Out.Replace(_T("[admin]"), (bAdmin) ? _T("admin") : _T(""));
	Out.Replace(_T("[Session]"), sSession);
	Out.Replace(_T("[RefreshVal]"), sRefresh);
	Out.Replace(_T("[wCommand]"), strCmd);

	strCmd.Replace(_T("&amp;"), _T("&"));

	Out.Replace(_T("[wCommandJ]"), strCmd);
	Out.Replace(_T("[Nick]"), _SpecialChars(g_App.m_pPrefs->GetUserNick()));
	Out.Replace(_T("[eMuleAppName]"), CLIENT_NAME);
	Out.Replace(_T("[version]"), CURRENT_VERSION_LONG);
	Out.Replace(_T("[WebControl]"), _GetPlainResString(IDS_WEB_CONTROL));
	Out.Replace(_T("[StyleSheet]"), m_Templates.sHeaderStylesheet);
	Out.Replace(_T("[Transfer]"), _GetPlainResString(IDS_CD_TRANS));
	Out.Replace(_T("[Server]"), _GetPlainResString(IDS_SERVERS));
	Out.Replace(_T("[Shared]"), _GetPlainResString(IDS_SHAREDFILES));
	Out.Replace(_T("[Graphs]"), _GetPlainResString(IDS_GRAPHS));
	Out.Replace(_T("[Log]"), _GetPlainResString(IDS_SV_LOG));
	Out.Replace(_T("[ServerInfo]"), _GetPlainResString(IDS_SV_SERVERINFO));
	Out.Replace(_T("[DebugLog]"), _GetPlainResString(IDS_SV_DEBUGLOG));
	Out.Replace(_T("[Stats]"), _GetPlainResString(IDS_STATISTICS));
	Out.Replace(_T("[Options]"), GetResString(IDS_PREFERENCES));
	Out.Replace(_T("[Ed2klink]"), _GetPlainResString(IDS_SW_LINK));
	Out.Replace(_T("[Close]"), _GetPlainResString(IDS_WEB_CLOSE, true));
	Out.Replace(_T("[Reboot]"), _GetPlainResString(IDS_WEB_REBOOT, true));
	Out.Replace(_T("[Shutdown]"), _GetPlainResString(IDS_WEB_SHUTDOWN, true));
	Out.Replace(_T("[Logout]"), _GetPlainResString(IDS_WEB_LOGOUT));
	Out.Replace(_T("[WebOptions]"), _GetPlainResString(IDS_WEB_OPTIONS));
	Out.Replace(_T("[Search]"), _GetPlainResString(IDS_SEARCH_NOUN));
	Out.Replace(_T("[Download]"), _GetPlainResString(IDS_SW_DOWNLOAD));
	Out.Replace(_T("[Start]"), _GetPlainResString(IDS_SW_START));
	Out.Replace(_T("[Version]"), _GetPlainResString(IDS_SERVER_VERSION));
	Out.Replace(_T("[VersionCheck]"), strVersionCheck);
	if (CCat::GetNumCats() > 1)
		InsertCatBox(Out, 0, m_Templates.sCatArrow, false, false, sSession, NULL);
	else
		Out.Replace(_T("[CATBOX]"), _T(""));

	Out.Replace(_T("[FileIsHashing]"), _GetPlainResString(IDS_HASHING, true));
	Out.Replace(_T("[FileIsErroneous]"), _GetPlainResString(IDS_ERRORLIKE, true));
	Out.Replace(_T("[FileIsCompleting]"), _GetPlainResString(IDS_COMPLETING, true));
	Out.Replace(_T("[FileDetails]"), _GetPlainResString(IDS_FD_TITLE, true));
	Out.Replace(_T("[ClearCompleted]"), _GetPlainResString(IDS_DL_CLEAR, true));
	Out.Replace(_T("[A4AF]"), _GetPlainResString(IDS_ALL_A4AF_TO_HERE, true));
	Out.Replace(_T("[A4AFSameCat]"), _GetPlainResString(IDS_ALL_A4AF_SAMECAT, true));
	Out.Replace(_T("[AUTOA4AF]"), _GetPlainResString(IDS_TREE_DL_A4AF_AUTO, true));
	Out.Replace(_T("[A4AFSwap]"), _GetPlainResString(IDS_ALL_A4AF_TO_OTHER, true));
	Out.Replace(_T("[Resume]"), _GetPlainResString(IDS_RESUME, true));
	Out.Replace(_T("[Stop]"), _GetPlainResString(IDS_STOP_VERB, true));
	Out.Replace(_T("[Pause]"), _GetPlainResString(IDS_PAUSE_VERB, true));
	Out.Replace(_T("[ConfirmCancel]"), _GetPlainResString(IDS_Q_CANCELDL2, true));
	Out.Replace(_T("[Cancel]"), GetResString(IDS_MAIN_BTN_CANCEL));
	Out.Replace(_T("[GetFLC]"), _GetPlainResString(IDS_MOVIE, true));
	Out.Replace(_T("[Rename]"), _GetPlainResString(IDS_RENAME, true));
	Out.Replace(_T("[Connect]"), _GetPlainResString(IDS_IRC_CONNECT, true));
	Out.Replace(_T("[ConfirmRemove]"), _GetPlainResString(IDS_WEB_CONFIRM_REMOVE_SERVER, false));	// don't change (') as a string is for message box
	Out.Replace(_T("[ConfirmClose]"), _GetPlainResString(IDS_MAIN_CLOSE, false));	// don't change (') as a string is for message box
	Out.Replace(_T("[ConfirmReboot]"), _GetPlainResString(IDS_MAIN_REBOOT, false));	// don't change (') as a string is for message box
	Out.Replace(_T("[ConfirmShutdown]"), _GetPlainResString(IDS_MAIN_SHUTDOWN, false));	// don't change (') as a string is for message box
	Out.Replace(_T("[RemoveServer]"), _GetPlainResString(IDS_REMOVETHIS, true));
	Out.Replace(_T("[StaticServer]"), _GetPlainResString(IDS_STATICSERVER, true));
	Out.Replace(_T("[ConfirmJumpstart]"), _GetPlainResString(IDS_JS_DISABLE, true) + _T("\\n"));
	Out.Replace(_T("[Friend]"), _GetPlainResString(IDS_FRIEND, true));
	Out.Replace(_T("[CatSel]"), sCat);

	Out.Replace(_T("[PriorityVeryLow]"), _GetPlainResString(IDS_PRIOVERYLOW, true));
	Out.Replace(_T("[PriorityLow]"), _GetPlainResString(IDS_PRIOLOW, true));
	Out.Replace(_T("[PriorityNormal]"), _GetPlainResString(IDS_PRIONORMAL, true));
	Out.Replace(_T("[PriorityHigh]"), _GetPlainResString(IDS_PRIOHIGH, true));
	Out.Replace(_T("[PriorityRelease]"), _GetPlainResString(IDS_PRIORELEASE, true));
	Out.Replace(_T("[PriorityAuto]"), _GetPlainResString(IDS_PRIOAUTO, true));

	Out.Replace(_T("[SetTimerOn]"), _GetPlainResString(IDS_TIMER_ON));
	Out.Replace(_T("[SetTimerOff]"), _GetPlainResString(IDS_TIMER_OFF));
	Out.Replace(_T("[SetTimerDisabled]"), _GetPlainResString(IDS_DISABLED));

	CString HTTPConText, HTTPHelp;
	CString HTTPHelpU = _T("0");
	CString HTTPHelpM = _T("0");
	CString HTTPHelpV = _T("0");
	CString HTTPHelpF = _T("0");
	TCHAR HTTPHeader[100] = _T(""), *pcHTTPConState;

	CString sCmd = _ParseURL(Data.sURL, _T("c"));

	if (sCmd.CompareNoCase(_T("togglemenulock")) == 0)
		m_pWSPrefs->bMenuLocked = !m_pWSPrefs->bMenuLocked;

	Out.Replace(_T("[MenuLocked]"), m_pWSPrefs->bMenuLocked ? _T("true") : _T("false"));
	Out.Replace(_T("[MenuLockTitle]"), _GetPlainResString(m_pWSPrefs->bMenuLocked ? IDS_MENULOCK_OFF : IDS_MENULOCK_ON));

	bool disconnectissued = (sCmd.CompareNoCase(_T("disconnect")) == 0);
	bool connectissued = (sCmd.CompareNoCase(_T("connect")) == 0);
	uint32		dwMax;

#ifdef OLD_SOCKETS_ENABLED
	if ((g_App.m_pServerConnect->IsConnecting() && !disconnectissued) || connectissued)
	{
		pcHTTPConState = _T("connecting");
		HTTPConText = _GetPlainResString(IDS_CONNECTING);
	}
	else if (g_App.m_pServerConnect->IsConnected() && !disconnectissued)
	{
		pcHTTPConState = (g_App.m_pServerConnect->IsLowID()) ? _T("low") : _T("high");

		CServer	*pCurServer = g_App.m_pServerConnect->GetCurrentServer();
		CServer	*cur_server = g_App.m_pServerList->GetServerByAddress(
			pCurServer->GetAddress(), pCurServer->GetPort() );

		if(cur_server->GetListName().GetLength() > SHORT_LENGTH)
			HTTPConText = cur_server->GetListName().Left(SHORT_LENGTH-3) + _T("...");
		else
			HTTPConText = cur_server->GetListName();
		HTTPHelpU = CastItoThousands(cur_server->GetNumUsers());
		HTTPHelpM = CastItoThousands(cur_server->GetMaxUsers());
		HTTPHelpF = CastItoThousands(cur_server->GetFiles());
		if ((dwMax = cur_server->GetMaxUsers()) != 0)
			HTTPHelpV.Format(_T("%u"), (100 * cur_server->GetNumUsers() + dwMax / 2) / dwMax);
	}
	else
#endif //OLD_SOCKETS_ENABLED
	{
		pcHTTPConState = _T("disconnected");
		HTTPConText = _GetPlainResString(IDS_DISCONNECTED);
		if (IsSessionAdmin(Data, sSession))
			HTTPConText.AppendFormat(_T(" (<a href=\"/?ses=%s&amp;w=server&amp;c=connect\">%s</a>)"), sSession, _GetPlainResString(IDS_CONNECTTOANYSERVER));
	}
	int allUsers = 0;
	int allFiles = 0;
	for (uint32 sc = 0; sc < g_App.m_pServerList->GetServerCount(); sc++)
	{
		CServer	*cur_server = g_App.m_pServerList->GetServerAt(sc);

		allUsers += cur_server->GetNumUsers();
		allFiles += cur_server->GetFiles();
	}
	Out.Replace(_T("[AllUsers]"), CastItoThousands(allUsers));
	Out.Replace(_T("[AllFiles]"), CastItoThousands(allFiles));
	Out.Replace(_T("[ConState]"), pcHTTPConState);
	Out.Replace(_T("[ConText]"), HTTPConText);
	_stprintf(HTTPHeader, _T("%.f"), static_cast<double>(100 * g_App.m_pUploadQueue->GetDataRate()) / 102.4 / g_App.m_pPrefs->GetMaxUpload());
	Out.Replace(_T("[UploadValue]"), HTTPHeader);

	if(g_App.m_pPrefs->GetMaxDownload() == UNLIMITED)
		_stprintf(HTTPHeader, _T("%.f"), static_cast<double>(100 * g_App.m_pDownloadQueue->GetDataRate()) / 102.4 / g_App.m_pPrefs->GetMaxGraphDownloadRate());
	else
		_stprintf(HTTPHeader, _T("%.f"), static_cast<double>(100 * g_App.m_pDownloadQueue->GetDataRate()) / 102.4 / g_App.m_pPrefs->GetMaxDownload());
	Out.Replace(_T("[DownloadValue]"), HTTPHeader);
#ifdef OLD_SOCKETS_ENABLED
	dwMax = g_App.m_pPrefs->GetMaxConnections();
	_stprintf(HTTPHeader, _T("%u"), (100 * g_App.m_pListenSocket->GetNumOpenSockets() + dwMax / 2) / dwMax);
	Out.Replace(_T("[ConnectionValue]"), HTTPHeader);
	_stprintf(HTTPHeader, _T("%.1f"), (static_cast<double>(g_App.m_pUploadQueue->GetDataRate())/1024.0));
	Out.Replace(_T("[CurUpload]"), HTTPHeader);
	_stprintf(HTTPHeader, _T("%.1f"), (static_cast<double>(g_App.m_pDownloadQueue->GetDataRate())/1024.0));
	Out.Replace(_T("[CurDownload]"), HTTPHeader);
	_stprintf(HTTPHeader, _T("%u"), g_App.m_pListenSocket->GetNumOpenSockets());
	Out.Replace(_T("[CurConnection]"), HTTPHeader);
#endif //OLD_SOCKETS_ENABLED

	FractionalRate2String(&HTTPHelp, g_App.m_pPrefs->GetMaxUpload());
	Out.Replace(_T("[MaxUpload]"), HTTPHelp);

	dwMax = g_App.m_pPrefs->GetMaxDownload();
	if (dwMax == UNLIMITED)
		HTTPHelp = GetResString(IDS_PW_UNLIMITED);
	else
		FractionalRate2String(&HTTPHelp, dwMax);
	Out.Replace(_T("[MaxDownload]"), HTTPHelp);

	HTTPHelp.Format(_T("%u"), g_App.m_pPrefs->GetMaxConnections());
	Out.Replace(_T("[MaxConnection]"), HTTPHelp);
	Out.Replace(_T("[UserValue]"), HTTPHelpV);
	Out.Replace(_T("[MaxUsers]"), HTTPHelpM);
	Out.Replace(_T("[CurUsers]"), HTTPHelpU);
	Out.Replace(_T("[CurFiles]"), HTTPHelpF);

	COleDateTime	CurDateTime(COleDateTime::GetCurrentTime());

	Out.Replace(_T("[CurDate]"), CurDateTime.Format(_T("%Y.%m.%d")));
	Out.Replace(_T("[CurTime]"), CurDateTime.Format(_T("%H:%M:%S")));
	Out.Replace(_T("[Connection]"), _GetPlainResString(IDS_PW_CONNECTION));
	Out.Replace(_T("[QuickStats]"), _GetPlainResString(IDS_PW_QUICKSTATS));

	Out.Replace(_T("[Users]"), _GetPlainResString(IDS_UUSERS));
	Out.Replace(_T("[Files]"), _GetPlainResString(IDS_FILES));
	Out.Replace(_T("[Con]"), _GetPlainResString(IDS_ST_ACTIVECONNECTIONS));
	Out.Replace(_T("[Up]"), _GetPlainResString(IDS_UPLOAD_NOUN));
	Out.Replace(_T("[Down]"), _GetPlainResString(IDS_DOWNLOAD_NOUN));
	return Out;

	EMULE_CATCH

	return _T("");
}

CString CWebServer::_GetFooter(const ThreadData &Data)
{
	EMULE_TRY

	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	return pThis->m_Templates.sFooter;

	EMULE_CATCH

	return _T("");
}

CString CWebServer::GetServerList(const ThreadData &Data)
{
	static const TCHAR *s_apcSrvSortTab1[WS_SRVCOL_NUMCOLUMNS] =
	{
		_T("state"),		//WS_SRVCOL_STATE
		_T("name"),			//WS_SRVCOL_NAME
		_T("ip"),			//WS_SRVCOL_IP
		_T("description"),	//WS_SRVCOL_DESCRIPTION
		_T("ping"),			//WS_SRVCOL_PING
		_T("users"),		//WS_SRVCOL_USERS
		_T("files"),		//WS_SRVCOL_FILES
		_T("priority"),		//WS_SRVCOL_PRIORITY
		_T("failed"),		//WS_SRVCOL_FAILED
		_T("limit"),		//WS_SRVCOL_LIMIT
		_T("version")		//WS_SRVCOL_VERSION
	};
	static const TCHAR *s_apcSrvSortTab2[WS_SRVCOL_NUMCOLUMNS] =
	{
		_T("[SortState]"),		//WS_SRVCOL_STATE
		_T("[SortName]"),		//WS_SRVCOL_NAME
		_T("[SortIP]"),			//WS_SRVCOL_IP
		_T("[SortDescription]"),//WS_SRVCOL_DESCRIPTION
		_T("[SortPing]"),		//WS_SRVCOL_PING
		_T("[SortUsers]"),		//WS_SRVCOL_USERS
		_T("[SortFiles]"),		//WS_SRVCOL_FILES
		_T("[SortPriority]"),	//WS_SRVCOL_PRIORITY
		_T("[SortFailed]"),		//WS_SRVCOL_FAILED
		_T("[SortLimit]"),		//WS_SRVCOL_LIMIT
		_T("[SortVersion]")		//WS_SRVCOL_VERSION
	};
	static const WSListDefinition s_aSrvColTab[ARRSIZE(m_pWSPrefs->abServerColHidden)] =
	{
		{ IDS_SL_SERVERNAME, _T("[ServernameI]"), _T("[ServernameH]"), _T("[ServernameM]") },
		{ IDS_IP, _T("[AddressI]"), _T("[AddressH]"), _T("[AddressM]") },
		{ IDS_DESCRIPTION, _T("[DescriptionI]"), _T("[DescriptionH]"), _T("[DescriptionM]") },
		{ IDS_PING, _T("[PingI]"), _T("[PingH]"), _T("[PingM]") },
		{ IDS_UUSERS, _T("[UsersI]"), _T("[UsersH]"), _T("[UsersM]") },
		{ IDS_FILES, _T("[FilesI]"), _T("[FilesH]"), _T("[FilesM]") },
		{ IDS_PRIORITY, _T("[PriorityI]"), _T("[PriorityH]"), _T("[PriorityM]") },
		{ IDS_UFAILED, _T("[FailedI]"), _T("[FailedH]"), _T("[FailedM]") },
		{ IDS_SERVER_SOFTHARDLIMIT, _T("[LimitI]"), _T("[LimitH]"), _T("[LimitM]") },
		{ IDS_SERVER_VERSION, _T("[VersionI]"), _T("[VersionH]"), _T("[VersionM]") },
	};
	EMULE_TRY

	int		iCat = _tstoi(_ParseURL(Data.sURL, _T("cat"))) & 0xFF;
	CString	sCat;

	if (iCat != 0)
		sCat.Format(_T("%u"), iCat);

	CString sSession = _ParseURL(Data.sURL, _T("ses"));
	bool bAdmin = IsSessionAdmin(Data, sSession);
	CString sAddServerBox = GetAddServerBox(Data);

	CString sCmd(_ParseURL(Data.sURL, _T("c")));
	CString sIP(_ParseURL(Data.sURL, _T("ip")));
	uint16 uPort = static_cast<uint16>(_tstoi(_ParseURL(Data.sURL, _T("port"))));
	if (bAdmin && sCmd.CompareNoCase(_T("connect")) == 0)
	{
		if(sIP.IsEmpty())
			g_App.m_pMDlg->SendMessage(WM_COMMAND, MP_CONNECT);
		else
			_ConnectToServer(sIP, uPort);
	}
	else if (bAdmin && sCmd.CompareNoCase(_T("disconnect")) == 0)
	{
#ifdef OLD_SOCKETS_ENABLED
		if (g_App.m_pServerConnect->IsConnecting())
		{
			g_App.m_pServerConnect->StopConnectionTry();
			g_App.m_pMDlg->ShowConnectionState(false);
		}
		else
#endif //OLD_SOCKETS_ENABLED
			g_App.m_pMDlg->SendMessage(WM_COMMAND, MP_DISCONNECT);
	}
	else if (bAdmin && sCmd.CompareNoCase(_T("remove")) == 0)
	{
		if(!sIP.IsEmpty())
			_RemoveServer(sIP, uPort);
	}
	else if (bAdmin && sCmd.CollateNoCase(_T("addtostatic")) == 0)
	{
		if(!sIP.IsEmpty())
			_AddToStatic(sIP, uPort);
	}
	else if (bAdmin && sCmd.CompareNoCase(_T("removefromstatic")) == 0)
	{
		if(!sIP.IsEmpty())
			_RemoveFromStatic(sIP, uPort);
	}
	else if (bAdmin && sCmd.CompareNoCase(_T("priolow")) == 0)
	{
		if(!sIP.IsEmpty())
		{
			CServer	*server = g_App.m_pServerList->GetServerByAddress(sIP, uPort);
			server->SetPreference(PR_LOW);
			if (g_App.m_pMDlg->m_wndServer)
				g_App.m_pMDlg->m_wndServer.m_ctlServerList.RefreshServer(*server);
		}
	}
	else if (bAdmin && sCmd.CompareNoCase(_T("prionormal")) == 0)
	{
		if(!sIP.IsEmpty())
		{
			CServer	*server = g_App.m_pServerList->GetServerByAddress(sIP, uPort);
			server->SetPreference(PR_NORMAL);
			if (g_App.m_pMDlg->m_wndServer)
				g_App.m_pMDlg->m_wndServer.m_ctlServerList.RefreshServer(*server);
		}
	}
	else if (bAdmin && sCmd.CompareNoCase(_T("priohigh")) == 0)
	{
		if(!sIP.IsEmpty())
		{
			CServer	*server = g_App.m_pServerList->GetServerByAddress(sIP, uPort);
			server->SetPreference(PR_HIGH);
			if (g_App.m_pMDlg->m_wndServer)
				g_App.m_pMDlg->m_wndServer.m_ctlServerList.RefreshServer(*server);
		}
	}
	else if (sCmd.CompareNoCase(_T("menu")) == 0)
		SetHiddenColumnState(Data.sURL, m_pWSPrefs->abServerColHidden, ARRSIZE(m_pWSPrefs->abServerColHidden));

	CString strTmp = _ParseURL(Data.sURL, _T("sortreverse"));
	CString strSort = _ParseURL(Data.sURL, _T("sort"));

	if (!strSort.IsEmpty())
	{
		for (unsigned ui = 0; ui < ARRSIZE(s_apcSrvSortTab1); ui++)
		{
			if (strSort.Compare(s_apcSrvSortTab1[ui]) == 0)
			{
				m_pWSPrefs->dwServerSort = ui;
				break;
			}
		}
		if (!strTmp.IsEmpty())
			m_pWSPrefs->abServerSortAsc[m_pWSPrefs->dwServerSort] = (strTmp == _T("true"));
	}

	CString Out = m_Templates.sServerList;

	Out.Replace(_T("[AddServerBox]"), sAddServerBox);
	Out.Replace(_T("[Session]"), sSession);
	Out.Replace(_T("[CatSel]"), sCat);

	const TCHAR *pcTmp = (m_pWSPrefs->abServerSortAsc[m_pWSPrefs->dwServerSort]) ? _T("&amp;sortreverse=false") : _T("&amp;sortreverse=true");

	for (unsigned ui = 0; ui < ARRSIZE(s_apcSrvSortTab2); ui++)
		Out.Replace(s_apcSrvSortTab2[ui], (m_pWSPrefs->dwServerSort == ui) ? pcTmp : _T(""));

	Out.Replace(_T("[ServerList]"), _GetPlainResString(IDS_SERVERS));

	const TCHAR *pcSortIcon = (m_pWSPrefs->abServerSortAsc[m_pWSPrefs->dwServerSort]) ? m_Templates.sUpArrow : m_Templates.sDownArrow;

	for (unsigned ui = 0; ui < ARRSIZE(s_aSrvColTab); ui++)
	{
		GetPlainResString(&strTmp, s_aSrvColTab[ui].dwResStrId, true);
		if (!m_pWSPrefs->abServerColHidden[ui])
		{
			Out.Replace( s_aSrvColTab[ui].pcColIcon,
				(m_pWSPrefs->dwServerSort == (ui + WS_SRVCOL_NAME)) ? pcSortIcon : _T("") );
			Out.Replace(s_aSrvColTab[ui].pcColHdr, strTmp);
		}
		else
		{
			Out.Replace(s_aSrvColTab[ui].pcColIcon, _T(""));
			Out.Replace(s_aSrvColTab[ui].pcColHdr, _T(""));
		}
		Out.Replace(s_aSrvColTab[ui].pcColMenu, strTmp);
	}

	Out.Replace(_T("[Actions]"), _GetPlainResString(IDS_WEB_ACTIONS));

	CArray<ServerEntry> ServerArray;

#ifdef OLD_SOCKETS_ENABLED
	CServer		*tempServer;
#endif OLD_SOCKETS_ENABLED
	// Populating array
	for (uint32 sc = 0; sc < g_App.m_pServerList->GetServerCount(); sc++)
	{
		CServer* cur_file = g_App.m_pServerList->GetServerAt(sc);
		ServerEntry Entry;
		Entry.sServerName = _SpecialChars(cur_file->GetListName());
		Entry.sServerIP = cur_file->GetAddress();
		Entry.nServerPort = cur_file->GetPort();
		Entry.sServerDescription = _SpecialChars(cur_file->GetDescription());
		Entry.nServerPing = cur_file->GetPing();
		Entry.nServerUsers = cur_file->GetNumUsers();
		Entry.nServerMaxUsers = cur_file->GetMaxUsers();
		Entry.nServerFiles = cur_file->GetFiles();
		Entry.bServerStatic = cur_file->IsStaticMember();
		if(cur_file->GetPreferences() == PR_HIGH)
		{
			Entry.sServerPriority = _GetPlainResString(IDS_PRIOHIGH);
			Entry.nServerPriority = 2;
		}
		else if(cur_file->GetPreferences() == PR_NORMAL)
		{
			Entry.sServerPriority = _GetPlainResString(IDS_PRIONORMAL);
			Entry.nServerPriority = 1;
		}
		else if(cur_file->GetPreferences() == PR_LOW)
		{
			Entry.sServerPriority = _GetPlainResString(IDS_PRIOLOW);
			Entry.nServerPriority = 0;
		}
		Entry.nServerFailed = cur_file->GetFailedCount();
		Entry.nServerSoftLimit = cur_file->GetSoftMaxFiles();
		Entry.nServerHardLimit = cur_file->GetHardMaxFiles();
		Entry.sServerVersion = cur_file->GetVersion();
		int counter=0;
		CString newip;
		for(int j = 0; j < 4; j++)
		{
			strTmp = Entry.sServerIP.Tokenize(_T("."), counter);
			if (strTmp.GetLength() == 1)
				newip += _T("00");
			else if (strTmp.GetLength() == 2)
				newip += _T("0");
			newip += strTmp;
		}
		Entry.sServerFullIP = newip;

		if (cur_file->GetFailedCount() > 0)
			Entry.sServerState = _T("failed");
		else
			Entry.sServerState = _T("disconnected");
#ifdef OLD_SOCKETS_ENABLED
		if (g_App.m_pServerConnect->IsConnecting())
		{
			tempServer = g_App.m_pServerConnect->GetConnectingServer();
			if (tempServer->GetFullIP() == cur_file->GetFullIP())
				Entry.sServerState = _T("connecting");
		}
		else if (g_App.m_pServerConnect->IsConnected())
		{
			tempServer = g_App.m_pServerConnect->GetCurrentServer();
			if (tempServer->GetFullIP() == cur_file->GetFullIP())
			{
				if (!g_App.m_pServerConnect->IsLowID())
					Entry.sServerState = _T("high");
				else
					Entry.sServerState = _T("low");
			}
		}
#endif //OLD_SOCKETS_ENABLED
		ServerArray.Add(Entry);
	}
	// Sorting (simple bubble sort, we don't have tons of data here)
	bool	bSortReverse, bSorted = true;

	bSortReverse = m_pWSPrefs->abServerSortAsc[m_pWSPrefs->dwServerSort];
	for (int nMax = 0; bSorted && nMax < ServerArray.GetCount()*2; nMax++)
	{
		bSorted = false;
		for(int i = 0; i < ServerArray.GetCount() - 1; i++)
		{
			bool bSwap = false;
			switch (m_pWSPrefs->dwServerSort)
			{
				case WS_SRVCOL_STATE:
					bSwap = ServerArray[i].sServerState.CompareNoCase(ServerArray[i+1].sServerState) > 0;
					break;
				case WS_SRVCOL_NAME:
					bSwap = ServerArray[i].sServerName.CompareNoCase(ServerArray[i+1].sServerName) < 0;
					break;
				case WS_SRVCOL_IP:
					bSwap = ServerArray[i].sServerFullIP < ServerArray[i+1].sServerFullIP;
					break;
				case WS_SRVCOL_DESCRIPTION:
					bSwap = ServerArray[i].sServerDescription.CompareNoCase(ServerArray[i+1].sServerDescription) < 0;
					break;
				case WS_SRVCOL_PING:
					bSwap = ServerArray[i].nServerPing < ServerArray[i+1].nServerPing;
					break;
				case WS_SRVCOL_USERS:
					bSwap = ServerArray[i].nServerUsers < ServerArray[i+1].nServerUsers;
					break;
				case WS_SRVCOL_FILES:
					bSwap = ServerArray[i].nServerFiles < ServerArray[i+1].nServerFiles;
					break;
				case WS_SRVCOL_PRIORITY:
					bSwap = ServerArray[i].nServerPriority < ServerArray[i+1].nServerPriority;
					break;
				case WS_SRVCOL_FAILED:
					bSwap = ServerArray[i].nServerFailed < ServerArray[i+1].nServerFailed;
					break;
				case WS_SRVCOL_LIMIT:
					bSwap = ServerArray[i].nServerSoftLimit < ServerArray[i+1].nServerSoftLimit;
					break;
				case WS_SRVCOL_VERSION:
					bSwap = ServerArray[i].sServerVersion < ServerArray[i+1].sServerVersion;
					break;
			}
			if (bSortReverse)
				bSwap = !bSwap;
			if (bSwap)
			{
				bSorted = true;
				ServerEntry TmpEntry = ServerArray[i];
				ServerArray[i] = ServerArray[i+1];
				ServerArray[i+1] = TmpEntry;
			}
		}
	}

//	Displaying
	const TCHAR	*pcTmp2;
	CString sList, HTTPProcessData, sServerPort, ed2k;
	CString OutE(m_Templates.sServerLine);

	OutE.Replace(_T("[admin]"), (bAdmin) ? _T("admin") : _T(""));
	OutE.Replace(_T("[session]"), sSession);

	for(int i = 0; i < ServerArray.GetCount(); i++)
	{
		HTTPProcessData = OutE;	// Copy Entry Line to Temp

		sServerPort.Format(_T("%u"), ServerArray[i].nServerPort);
		ed2k.Format(_T("ed2k://|server|%s|%s|/"), ServerArray[i].sServerIP, sServerPort);

		if (ServerArray[i].sServerIP == _ParseURL(Data.sURL,_T("ip")) && sServerPort == _ParseURL(Data.sURL,_T("port")))
			pcTmp = _T("checked");
		else
			pcTmp = _T("checked_no");
		HTTPProcessData.Replace(_T("[LastChangedDataset]"), pcTmp);

		if (ServerArray[i].bServerStatic)
		{
			pcTmp = _T("staticsrv");
			pcTmp2 = _T("static");
		}
		else
		{
			pcTmp = _T("");
			pcTmp2 = _T("none");
		}
		HTTPProcessData.Replace(_T("[isstatic]"), pcTmp);
		HTTPProcessData.Replace(_T("[ServerType]"), pcTmp2);

		const TCHAR *pcSrvPriority = _T("");

		switch(ServerArray[i].nServerPriority)
		{
			case 0:
				pcSrvPriority = _T("Low");
				break;
			case 1:
				pcSrvPriority = _T("Normal");
				break;
			case 2:
				pcSrvPriority = _T("High");
				break;
		}

		HTTPProcessData.Replace(_T("[ed2k]"), ed2k);
		HTTPProcessData.Replace(_T("[ip]"), ServerArray[i].sServerIP);
		HTTPProcessData.Replace(_T("[port]"), sServerPort);
		HTTPProcessData.Replace(_T("[server-priority]"), pcSrvPriority);

		// DonGato: reduced large servernames or descriptions
		if (!m_pWSPrefs->abServerColHidden[0])
		{
			if(ServerArray[i].sServerName.GetLength() > SHORT_LENGTH)
				HTTPProcessData.Replace(_T("[Servername]"), _T("<acronym title=\"") + ServerArray[i].sServerName + _T("\">") + ServerArray[i].sServerName.Left(SHORT_LENGTH-3) + _T("...") + _T("</acronym>"));
			else
				HTTPProcessData.Replace(_T("[Servername]"), ServerArray[i].sServerName);
		}
		else
			HTTPProcessData.Replace(_T("[Servername]"), _T(""));
		if (!m_pWSPrefs->abServerColHidden[1])
		{
			CString sPort; sPort.Format(_T(":%d"), ServerArray[i].nServerPort);
			HTTPProcessData.Replace(_T("[Address]"), ServerArray[i].sServerIP + sPort);
		}
		else
			HTTPProcessData.Replace(_T("[Address]"), _T(""));
		if (!m_pWSPrefs->abServerColHidden[2])
		{
			if(ServerArray[i].sServerDescription.GetLength() > SHORT_LENGTH)
				HTTPProcessData.Replace(_T("[Description]"), _T("<acronym title=\"") + ServerArray[i].sServerDescription + _T("\">") + ServerArray[i].sServerDescription.Left(SHORT_LENGTH-3) + _T("...") + _T("</acronym>"));
			else
				HTTPProcessData.Replace(_T("[Description]"), ServerArray[i].sServerDescription);
		}
		else
			HTTPProcessData.Replace(_T("[Description]"), _T(""));
		if (!m_pWSPrefs->abServerColHidden[3])
		{
			CString sPing; sPing.Format(_T("%d"), ServerArray[i].nServerPing);
			HTTPProcessData.Replace(_T("[Ping]"), sPing);
		}
		else
			HTTPProcessData.Replace(_T("[Ping]"), _T(""));
		CString sT;
		if (!m_pWSPrefs->abServerColHidden[4])
		{
			if(ServerArray[i].nServerUsers > 0)
			{
				if(ServerArray[i].nServerMaxUsers > 0)
					sT.Format(_T("%s (%s)"), CastItoThousands(ServerArray[i].nServerUsers), CastItoThousands(ServerArray[i].nServerMaxUsers));
				else
					sT.Format(_T("%s"), CastItoThousands(ServerArray[i].nServerUsers));
			}
			HTTPProcessData.Replace(_T("[Users]"), sT);
		}
		else
			HTTPProcessData.Replace(_T("[Users]"), _T(""));
		if (!m_pWSPrefs->abServerColHidden[5] && (ServerArray[i].nServerFiles > 0))
		{
			HTTPProcessData.Replace(_T("[Files]"), CastItoThousands(ServerArray[i].nServerFiles));
		}
		else
			HTTPProcessData.Replace(_T("[Files]"), _T(""));
		if (!m_pWSPrefs->abServerColHidden[6])
			HTTPProcessData.Replace(_T("[Priority]"), ServerArray[i].sServerPriority);
		else
			HTTPProcessData.Replace(_T("[Priority]"), _T(""));
		if (!m_pWSPrefs->abServerColHidden[7])
		{
			CString sFailed; sFailed.Format(_T("%d"), ServerArray[i].nServerFailed);
			HTTPProcessData.Replace(_T("[Failed]"), sFailed);
		}
		else
			HTTPProcessData.Replace(_T("[Failed]"), _T(""));
		if (!m_pWSPrefs->abServerColHidden[8])
		{
			CString		strTemp;

			strTemp.Format( _T("%s (%s)"), CastItoThousands(ServerArray[i].nServerSoftLimit),
				CastItoThousands(ServerArray[i].nServerHardLimit) );
			HTTPProcessData.Replace(_T("[Limit]"), strTemp);
		}
		else
			HTTPProcessData.Replace(_T("[Limit]"), _T(""));
		if (!m_pWSPrefs->abServerColHidden[9])
		{
			if(ServerArray[i].sServerVersion.GetLength() > SHORT_LENGTH_MIN)
				HTTPProcessData.Replace(_T("[Version]"), _T("<acronym title=\"") + ServerArray[i].sServerVersion + _T("\">") + ServerArray[i].sServerVersion.Left(SHORT_LENGTH-3) + _T("...") + _T("</acronym>"));
			else
				HTTPProcessData.Replace(_T("[Version]"), ServerArray[i].sServerVersion);
		}
		else
			HTTPProcessData.Replace(_T("[Version]"), _T(""));
		HTTPProcessData.Replace(_T("[ServerState]"), ServerArray[i].sServerState);
		sList += HTTPProcessData;
	}
	Out.Replace(_T("[ServersList]"), sList);

	return Out;

	EMULE_CATCH

	return _T("");
}

CString CWebServer::GetTransferList(const ThreadData &Data)
{
	static const TCHAR *s_apcDLSortTab1[WS_DLCOL_NUMCOLUMNS] =
	{
		_T("dstate"),		//WS_DLCOL_STATE
		_T("dtype"),		//WS_DLCOL_TYPE
		_T("dname"),		//WS_DLCOL_NAME
		_T("dsize"),		//WS_DLCOL_SIZE
		_T("dtransferred"),	//WS_DLCOL_TRANSFERRED
		_T("dprogress"),	//WS_DLCOL_PROGRESS
		_T("dspeed"),		//WS_DLCOL_SPEED
		_T("dsources"),		//WS_DLCOL_SOURCES
		_T("dpriority"),	//WS_DLCOL_PRIORITY
		_T("dcategory"),	//WS_DLCOL_CATEGORY
		_T("dfakecheck")	//WS_DLCOL_FAKECHECK
	};
	static const TCHAR *s_apcULSortTab1[WS_ULCOL_NUMCOLUMNS] =
	{
		_T("uclient"),		//WS_ULCOL_CLIENT
		_T("uuser"),		//WS_ULCOL_USER
		_T("uversion"),		//WS_ULCOL_VERSION
		_T("ufilename"),	//WS_ULCOL_FILENAME
		_T("utransferred"),	//WS_ULCOL_TRANSFERRED
		_T("uspeed")		//WS_ULCOL_SPEED
	};
	static const TCHAR *s_apcQUSortTab1[WS_QUCOL_NUMCOLUMNS] =
	{
		_T("qclient"),		//WS_QUCOL_CLIENT
		_T("quser"),		//WS_QUCOL_USER
		_T("qversion"),		//WS_QUCOL_VERSION
		_T("qfilename"),	//WS_QUCOL_FILENAME
		_T("qscore")		//WS_QUCOL_SCORE
	};
	static const TCHAR *s_apcDLSortTab2[WS_DLCOL_NUMCOLUMNS] =
	{
		_T("[SortDState]"),		//WS_DLCOL_STATE
		_T("[SortDType]"),		//WS_DLCOL_TYPE
		_T("[SortDName]"),		//WS_DLCOL_NAME
		_T("[SortDSize]"),		//WS_DLCOL_SIZE
		_T("[SortDTransferred]"),	//WS_DLCOL_TRANSFERRED
		_T("[SortDProgress]"),	//WS_DLCOL_PROGRESS
		_T("[SortDSpeed]"),		//WS_DLCOL_SPEED
		_T("[SortDSources]"),	//WS_DLCOL_SOURCES
		_T("[SortDPriority]"),	//WS_DLCOL_PRIORITY
		_T("[SortDCategory]"),	//WS_DLCOL_CATEGORY
		_T("[SortDFakeCheck]")	//WS_DLCOL_FAKECHECK
	};
	static const TCHAR *s_apcULSortTab2[WS_ULCOL_NUMCOLUMNS] =
	{
		_T("[SortUClient]"),		//WS_ULCOL_CLIENT
		_T("[SortUUser]"),			//WS_ULCOL_USER
		_T("[SortUVersion]"),		//WS_ULCOL_VERSION
		_T("[SortUFilename]"),		//WS_ULCOL_FILENAME
		_T("[SortUTransferred]"),	//WS_ULCOL_TRANSFERRED
		_T("[SortUSpeed]")			//WS_ULCOL_SPEED
	};
	static const TCHAR *s_apcQUSortTab2[WS_QUCOL_NUMCOLUMNS] =
	{
		_T("[SortQClient]"),		//WS_QUCOL_CLIENT
		_T("[SortQUser]"),			//WS_QUCOL_USER
		_T("[SortQVersion]"),		//WS_QUCOL_VERSION
		_T("[SortQFilename]"),		//WS_QUCOL_FILENAME
		_T("[SortQScore]")			//WS_QUCOL_SCORE
	};
	static const WSListDefinition s_aDLColTab[ARRSIZE(m_pWSPrefs->abDownloadColHidden)] =
	{
		{ IDS_DL_FILENAME, _T("[DFilenameI]"), _T("[DFilename]"), _T("[DFilename]") },
		{ IDS_DL_SIZE, _T("[DSizeI]"), _T("[DSize]"), _T("[DSizeM]") },
		{ IDS_SF_COMPLETED, _T("[DTransferredI]"), _T("[DTransferred]"), _T("[DTransferredM]") },
		{ IDS_DL_PROGRESS, _T("[DProgressI]"), _T("[DProgress]"), _T("[DProgressM]") },
		{ IDS_DL_SPEED, _T("[DSpeedI]"), _T("[DSpeed]"), _T("[DSpeedM]") },
		{ IDS_DL_SOURCES, _T("[DSourcesI]"), _T("[DSources]"), _T("[DSourcesM]") },
		{ IDS_PRIORITY, _T("[DPriorityI]"), _T("[DPriority]"), _T("[DPriorityM]") },
		{ IDS_CAT, _T("[DCategoryI]"), _T("[DCategory]"), _T("[DCategoryM]") },
		{ IDS_FAKE_CHECK_HEADER, _T("[DFakeCheckI]"), _T("[DFakeCheck]"), _T("[DFakeCheckM]") }
	};
	static const WSListDefinition s_aULColTab[ARRSIZE(m_pWSPrefs->abUploadColHidden)] =
	{
		{ IDS_QL_USERNAME, _T("[UUserI]"), _T("[UUser]"), _T("[UUserM]") },
		{ IDS_INFLST_USER_CLIENTVERSION, _T("[UVersionI]"), _T("[UVersion]"), _T("[UVersionM]") },
		{ IDS_DL_FILENAME, _T("[UFilenameI]"), _T("[UFilename]"), _T("[UFilenameM]") },
		{ IDS_DL_ULDL, _T("[UTransferredI]"), _T("[UTransferred]"), _T("[UTransferredM]") },
		{ IDS_DL_SPEED, _T("[USpeedI]"), _T("[USpeed]"), _T("[USpeedM]") }
	};
	static const WSListDefinition s_aQUColTab[ARRSIZE(m_pWSPrefs->abQueueColHidden)] =
	{
		{ IDS_QL_USERNAME, _T("[UserNameTitleI]"), _T("[UserNameTitle]"), _T("[UserNameTitleM]") },
		{ IDS_INFLST_USER_CLIENTVERSION, _T("[VersionI]"), _T("[Version]"), _T("[VersionM]") },
		{ IDS_DL_FILENAME, _T("[FileNameTitleI]"), _T("[FileNameTitle]"), _T("[FileNameTitleM]") },
		{ IDS_SCORE, _T("[ScoreTitleI]"), _T("[ScoreTitle]"), _T("[ScoreTitleM]") }
	};
	EMULE_TRY

	CString	strTmp, Out, sCat;
	int		iCat = _tstoi(_ParseURL(Data.sURL, _T("cat"))) & 0xFF;
	byte	abyteFileHash[16];

	if (iCat != 0)
		sCat.Format(_T("&cat=%u"), iCat);
	Out.Preallocate(50 * 1024);

	CString sSession = _ParseURL(Data.sURL, _T("ses"));

	bool bAdmin = IsSessionAdmin(Data,sSession);

	CString sClear(_ParseURL(Data.sURL, _T("clearcompleted")));
	if (bAdmin && !sClear.IsEmpty())
	{
		if (sClear.CompareNoCase(_T("all")) == 0)
		{
			EnumCategories eCatID = (iCat == 0) ? CAT_ALL : CCat::GetCatIDByUserIndex(iCat);
			_EnumCategories eCatIDforMessage = eCatID;

			g_App.m_pMDlg->SendMessage(WEB_CLEAR_COMPLETED, (WPARAM)0, static_cast<LPARAM>(eCatIDforMessage));

			iCat = (iCat == 0) ? 0 : CCat::UserCatIndexToCatIndex(iCat);
		}
		else if (md4cmp0(StringToHash(sClear, abyteFileHash)) != 0)
		{
			byte	*pabyteFileHash = new uchar[16];

			md4cpy(pabyteFileHash, abyteFileHash);
			g_App.m_pMDlg->SendMessage(WEB_CLEAR_COMPLETED, (WPARAM)1, reinterpret_cast<LPARAM>(pabyteFileHash));
		}
	}

	if (bAdmin)
	{
		strTmp = _ParseURL(Data.sURL, _T("ed2k"));
		if (!strTmp.IsEmpty())
		{
			EnumCategories	eCatID = (iCat == 0) ? CAT_NONE : CCat::GetCatIDByUserIndex(iCat);

			g_App.m_pMDlg->m_dlgSearch.AddEd2kLinksToDownload(strTmp, eCatID);
			iCat = (iCat == 0) ? 0 : CCat::UserCatIndexToCatIndex(iCat);
		}
	}

	strTmp = _ParseURL(Data.sURL, _T("c"));
	if (strTmp.Compare(_T("menudown")) == 0)
		SetHiddenColumnState(Data.sURL, m_pWSPrefs->abDownloadColHidden, ARRSIZE(m_pWSPrefs->abDownloadColHidden));
	else if (strTmp.Compare(_T("menuup")) == 0)
		SetHiddenColumnState(Data.sURL, m_pWSPrefs->abUploadColHidden, ARRSIZE(m_pWSPrefs->abUploadColHidden));
	else if (strTmp.Compare(_T("menuqueue")) == 0)
		SetHiddenColumnState(Data.sURL, m_pWSPrefs->abQueueColHidden, ARRSIZE(m_pWSPrefs->abQueueColHidden));
	else if (strTmp.Compare(_T("menuprio")) == 0)
	{
		if (bAdmin)
		{
			byte	bytePrio;
			
			strTmp = _ParseURL(Data.sURL, _T("p"));
			if(strTmp.CompareNoCase(_T("low")) == 0)
				bytePrio = PR_LOW;
			else if(strTmp.CompareNoCase(_T("high")) == 0)
				bytePrio = PR_HIGH;
			else if(strTmp.CompareNoCase(_T("auto")) == 0)
				bytePrio = PR_AUTO;
			else // _T("normal")
				bytePrio = PR_NORMAL;

			g_App.m_pDownloadQueue->SetCatPrio(iCat, bytePrio);
		}
	}

	if (bAdmin)
	{
		CString	sOp(_ParseURL(Data.sURL, _T("op")));

		if (!sOp.IsEmpty())
		{
			byte	abyteHashBuf[16];

			strTmp = _ParseURL(Data.sURL, _T("file"));
			if (!strTmp.IsEmpty())
			{
				CPartFile	*pFile;

				if ( md4cmp0(StringToHash(strTmp, abyteHashBuf)) != 0
					&& (pFile = g_App.m_pDownloadQueue->GetFileByID(abyteHashBuf)) != NULL )
				{	// All actions require a found file (removed double-check inside)
					if (sOp == _T("a4af"))
						pFile->DownloadAllA4AF();
					else if (sOp == _T("a4afsamecat"))
						pFile->DownloadAllA4AF(true);
					else if (sOp == _T("autoa4af"))
						g_App.m_pDownloadQueue->SetA4AFAutoFile(pFile);
					else if (sOp == _T("noautoa4af"))
						g_App.m_pDownloadQueue->SetA4AFAutoFile(NULL);
					else if (sOp == _T("a4afswap"))
					{
						ClientList	clientListCopy;

						pFile->GetCopySourceLists(SLM_ALLOWED_TO_A4AF_SWAP, &clientListCopy);
						for (ClientList::const_iterator cIt = clientListCopy.begin(); cIt != clientListCopy.end(); cIt++)
							(*cIt)->SwapToAnotherFile(NULL);
					}
					else if (sOp == _T("stop"))
						pFile->StopFile();
					else if (sOp == _T("pause"))
						pFile->PauseFile();
					else if (sOp == _T("resume"))
						pFile->ResumeFile();
					else if (sOp == _T("cancel"))
					{
						pFile->DeleteFile();
						g_App.m_pMDlg->m_wndTransfer.UpdateCatTabTitles();
					}
					else if (sOp == _T("getflc"))
						pFile->GetFirstLastChunk4Preview();
					else if (sOp == _T("rename"))
					{
						strTmp = _ParseURL(Data.sURL, _T("name"));
						g_App.m_pMDlg->SendMessage(WEB_FILE_RENAME, (WPARAM)pFile, (LPARAM)(LPCTSTR)strTmp);
					}
					else if (sOp == _T("priolow"))
					{
						pFile->SetAutoPriority(false);
						pFile->SetPriority(PR_LOW);
					}
					else if (sOp == _T("prionormal"))
					{
						pFile->SetAutoPriority(false);
						pFile->SetPriority(PR_NORMAL);
					}
					else if (sOp == _T("priohigh"))
					{
						pFile->SetAutoPriority(false);
						pFile->SetPriority(PR_HIGH);
					}
					else if (sOp == _T("prioauto"))
					{
						pFile->SetAutoPriority(true);
						pFile->SetPriority(PR_HIGH);
					}
					else if (sOp == _T("setcat"))
					{
						int	iMenu = _tstoi(_ParseURL(Data.sURL, _T("filecat")));

						if (iMenu != 0)
							pFile->SetCatID(CCat::GetCatIDByUserIndex(iMenu));
						else
							pFile->SetCatID(CAT_NONE);
					}
				}
			}
			else
			{
				CUpDownClient	*pClient = NULL;

				if ( (md4cmp0(StringToHash(_ParseURL(Data.sURL, _T("userhash")), abyteHashBuf)) != 0)
					&& ((pClient = g_App.m_pClientList->FindClientByUserHash(abyteHashBuf)) != NULL) )
				{
					if (sOp.CompareNoCase(_T("addfriend")) == 0)
						g_App.m_pFriendList->AddFriend(pClient);
					else if (sOp.CompareNoCase(_T("removefriend")) == 0)
						g_App.m_pFriendList->RemoveFriend(pClient);
				}
			}
		}
	}

	strTmp = _ParseURL(Data.sURL, _T("sortreverse"));

	CString strSort(_ParseURL(Data.sURL, _T("sort")));
	TCHAR	cCh = strSort.GetString()[0];

	if (!strSort.IsEmpty())
	{
		if (cCh == _T('d'))	// download list
		{
			for (unsigned ui = 0; ui < ARRSIZE(s_apcDLSortTab1); ui++)
			{
				if (strSort.Compare(s_apcDLSortTab1[ui]) == 0)
				{
					m_pWSPrefs->dwDownloadSort = ui;
					break;
				}
			}
		}
		else if (cCh == _T('u'))	// upload list
		{
			for (unsigned ui = 0; ui < ARRSIZE(s_apcULSortTab1); ui++)
			{
				if (strSort.Compare(s_apcULSortTab1[ui]) == 0)
				{
					m_pWSPrefs->dwUploadSort = ui;
					break;
				}
			}
		}
		else if (cCh == _T('q'))	// waiting queue
		{
			for (unsigned ui = 0; ui < ARRSIZE(s_apcQUSortTab1); ui++)
			{
				if (strSort.Compare(s_apcQUSortTab1[ui]) == 0)
				{
					m_pWSPrefs->dwQueueSort = ui;
					break;
				}
			}
		}
	}

	if (!strTmp.IsEmpty())
	{
		bool	bDirection = (strTmp == _T("true"));

		if (cCh == _T('d'))
			m_pWSPrefs->abDownloadSortAsc[m_pWSPrefs->dwDownloadSort] = bDirection;
		else if (cCh == _T('u'))
			m_pWSPrefs->abUploadSortAsc[m_pWSPrefs->dwUploadSort] = bDirection;
		else if (cCh == _T('q'))
			m_pWSPrefs->abQueueSortAsc[m_pWSPrefs->dwQueueSort] = bDirection;
	}

	strTmp = _ParseURL(Data.sURL, _T("showuploadqueue"));
	if (strTmp.CompareNoCase(_T("true")) == 0)
		m_pWSPrefs->bShowUploadQueue = true;
	else if (strTmp.CompareNoCase(_T("false")) == 0)
		m_pWSPrefs->bShowUploadQueue = false;

	strTmp = _ParseURL(Data.sURL, _T("showuploadqueuebanned"));
	if (strTmp.CompareNoCase(_T("true")) == 0)
		m_pWSPrefs->bShowUploadQueueBanned = true;
	else if (strTmp.CompareNoCase(_T("false")) == 0)
		m_pWSPrefs->bShowUploadQueueBanned = false;

	strTmp = _ParseURL(Data.sURL, _T("showuploadqueuefriend"));
	if (strTmp.CompareNoCase(_T("true")) == 0)
		m_pWSPrefs->bShowUploadQueueFriend = true;
	else if (strTmp.CompareNoCase(_T("false")) == 0)
		m_pWSPrefs->bShowUploadQueueFriend = false;

	strTmp = _ParseURL(Data.sURL, _T("showuploadqueuecredit"));
	if (strTmp.CompareNoCase(_T("true")) == 0)
		m_pWSPrefs->bShowUploadQueueCredit = true;
	else if (strTmp.CompareNoCase(_T("false")) == 0)
		m_pWSPrefs->bShowUploadQueueCredit = false;

	Out += m_Templates.sTransferImages;
	Out += m_Templates.sTransferList;
	Out.Replace(_T("[DownloadHeader]"), m_Templates.sTransferDownHeader);
	Out.Replace(_T("[DownloadFooter]"), m_Templates.sTransferDownFooter);
	Out.Replace(_T("[UploadHeader]"), m_Templates.sTransferUpHeader);
	Out.Replace(_T("[UploadFooter]"), m_Templates.sTransferUpFooter);
	Out.Replace(_T("[Session]"), sSession);

	InsertCatBox(Out, iCat, _T(""), true, true, sSession, NULL);

	const TCHAR	*pcTmp = (m_pWSPrefs->abDownloadSortAsc[m_pWSPrefs->dwDownloadSort]) ? _T("&amp;sortreverse=false") : _T("&amp;sortreverse=true");

	for (unsigned ui = 0; ui < ARRSIZE(s_apcDLSortTab2); ui++)
		Out.Replace(s_apcDLSortTab2[ui], (m_pWSPrefs->dwDownloadSort == ui) ? pcTmp : _T(""));

	pcTmp = (m_pWSPrefs->abUploadSortAsc[m_pWSPrefs->dwUploadSort]) ? _T("&amp;sortreverse=false") : _T("&amp;sortreverse=true");
	for (unsigned ui = 0; ui < ARRSIZE(s_apcULSortTab2); ui++)
		Out.Replace(s_apcULSortTab2[ui], (m_pWSPrefs->dwUploadSort == ui) ? pcTmp : _T(""));
	
	const TCHAR *pcSortIcon = (m_pWSPrefs->abDownloadSortAsc[m_pWSPrefs->dwDownloadSort]) ? m_Templates.sUpArrow : m_Templates.sDownArrow;

	for (unsigned ui = 0; ui < ARRSIZE(s_aDLColTab); ui++)
	{
		GetPlainResString(&strTmp, s_aDLColTab[ui].dwResStrId, true);
		if (!m_pWSPrefs->abDownloadColHidden[ui])
		{
			Out.Replace( s_aDLColTab[ui].pcColIcon,
				(m_pWSPrefs->dwDownloadSort == (ui + WS_DLCOL_NAME)) ? pcSortIcon : _T("") );
			Out.Replace(s_aDLColTab[ui].pcColHdr, strTmp);
		}
		else
		{
			Out.Replace(s_aDLColTab[ui].pcColIcon, _T(""));
			Out.Replace(s_aDLColTab[ui].pcColHdr, _T(""));
		}
		Out.Replace(s_aDLColTab[ui].pcColMenu, strTmp);
	}

	pcSortIcon = (m_pWSPrefs->abUploadSortAsc[m_pWSPrefs->dwUploadSort]) ? m_Templates.sUpArrow : m_Templates.sDownArrow;

	for (unsigned ui = 0; ui < ARRSIZE(s_aULColTab); ui++)
	{
		GetPlainResString(&strTmp, s_aULColTab[ui].dwResStrId, true);
		if (!m_pWSPrefs->abUploadColHidden[ui])
		{
			Out.Replace( s_aULColTab[ui].pcColIcon,
				(m_pWSPrefs->dwUploadSort == (ui + WS_ULCOL_USER)) ? pcSortIcon : _T("") );
			Out.Replace(s_aULColTab[ui].pcColHdr, strTmp);
		}
		else
		{
			Out.Replace(s_aULColTab[ui].pcColIcon, _T(""));
			Out.Replace(s_aULColTab[ui].pcColHdr, _T(""));
		}
		Out.Replace(s_aULColTab[ui].pcColMenu, strTmp);
	}

	Out.Replace(_T("[DownloadList]"), _GetPlainResString(IDS_ST_ACTIVEDOWNLOAD));
	Out.Replace(_T("[UploadList]"), _GetPlainResString(IDS_ST_ACTIVEUPLOAD));
	Out.Replace(_T("[Actions]"), _GetPlainResString(IDS_WEB_ACTIONS));
	Out.Replace(_T("[TotalDown]"), _GetPlainResString(IDS_INFLST_USER_TOTALDOWNLOAD));
	Out.Replace(_T("[TotalUp]"), _GetPlainResString(IDS_INFLST_USER_TOTALUPLOAD));
	Out.Replace(_T("[CatSel]"), sCat);
	Out.Replace(_T("[admin]"), (bAdmin) ? _T("admin") : _T(""));

	Out.Replace(_T("[ClearAllCompleted]"), _GetPlainResString(IDS_TREE_DL_CLEAR_ALL_COMPLETED));

	double	fTotalSpeed = 0, dTmp;
	uint64	qwTotalSize = 0, qwTotalTransferred = 0;
	CArray<DownloadFiles> FilesArray;
	bool							bLocalArray = false;
	CDownloadList::PartFileVector	*pvecPartFiles;
	CDownloadList::PartFileVector	vecPartFiles;
	if ((pvecPartFiles = g_App.m_pDownloadList->GetFiles()) == NULL)
	{
		bLocalArray = true;
		pvecPartFiles = &vecPartFiles;
		vecPartFiles.erase(vecPartFiles.begin(),vecPartFiles.end());
	//	Populating array
		for (POSITION pos=g_App.m_pDownloadQueue->m_partFileList.GetHeadPosition(); pos != NULL;)
		{
			CPartFile	*pPartFile = g_App.m_pDownloadQueue->m_partFileList.GetNext(pos);

			if (pPartFile)
				vecPartFiles.push_back(pPartFile);
		}
	}
	for (unsigned int i = 0; i < pvecPartFiles->size(); i++)
	{
		int			iFileStatus;
		CPartFile	*pPartFile = (*pvecPartFiles)[i];

		if (pPartFile != NULL)
		{
			if ( !CCat::FileBelongsToGivenCat( pPartFile, (iCat > CAT_PREDEFINED)
														 ? static_cast<_EnumCategories>(iCat)
														 : CCat::GetCatIDByIndex(iCat), true ) )
				continue;

			if(pPartFile->IsULAutoPrioritized())
				pPartFile->UpdateDownloadAutoPriority();

			DownloadFiles dFile;

			dFile.sFileName = pPartFile->GetFileName();
			SpecialChars(&dFile.sFileName);
			dFile.pcFileType = GetFileTypeForWebServer(dFile.sFileName);
			dFile.sFileNameJS = _SpecialChars(pPartFile->GetFileName(), true);	//for javascript
			dFile.m_qwFileSize = pPartFile->GetFileSize();
			dFile.m_qwFileTransferred = pPartFile->GetCompletedSize();
			dFile.m_dblCompleted = pPartFile->GetPercentCompleted();
			dFile.dwFileSpeed = pPartFile->GetDataRate();

			dFile.bIsComplete = false;
			iFileStatus = pPartFile->GetPartFileStatusID();
			switch (iFileStatus)
			{
				case PS_HASHING:
					dFile.pcFileState = _T("hashing");
					break;
				case PS_WAITINGFORHASH:
					dFile.pcFileState = _T("waitinghash");
					break;
				case PS_ERROR:
					dFile.pcFileState = _T("error");
					break;
				case PS_COMPLETING:
					dFile.bIsComplete = true;
					dFile.pcFileState = _T("completing");
					break;
				case PS_COMPLETE:
					dFile.bIsComplete = true;
					dFile.pcFileState = _T("complete");
					break;
				case PS_STOPPED:
					dFile.pcFileState = _T("stopped");
					break;
				case PS_PAUSED:
					dFile.pcFileState = _T("paused");
					break;
				case PS_DOWNLOADING:
					dFile.pcFileState = _T("downloading");
					break;
				case PS_WAITINGFORSOURCE:
					dFile.pcFileState = _T("waiting");
					break;
				case PS_STALLED:
					dFile.pcFileState = _T("stalled");
					break;
			}

			dFile.bFileAutoPrio = pPartFile->IsAutoPrioritized();
			dFile.nFilePrio = pPartFile->GetPriority();
			CCat		*pCat = CCat::GetCatByID(pPartFile->GetCatID());
			dFile.sCategory = (pCat != NULL) ? pCat->GetTitle() : GetResString(IDS_CAT_UNCATEGORIZED);
			dFile.sCategory.Replace(_T("'"), _T("\'"));
			dFile.sFileHash = HashToString(pPartFile->GetFileHash());
			dFile.lSourceCount = pPartFile->GetSourceCount();
			dFile.lNotCurrentSourceCount = pPartFile->GetNotCurrentSourcesCount();
			dFile.lTransferringSourceCount = pPartFile->GetTransferringSrcCount();
			dFile.bIsPreview = (!dFile.bIsComplete && pPartFile->AllowGet1stLast());
			dFile.bIsGetFLC = (pPartFile->GetMovieMode() != 0);

			dFile.sED2kLink = pPartFile->CreateED2kLink();
			dFile.sFileInfo = pPartFile->GetDownloadFileInfo();
			SpecialChars(&dFile.sFileInfo);
			g_App.m_pFakeCheck->GetFakeComment(HashToString(pPartFile->GetFileHash()), pPartFile->GetFileSize(), &dFile.sFakeCheck);
			dFile.cSortRank = 0;
			if (g_App.m_pPrefs->DoPausedStoppedLast())
			{
				dFile.cSortRank |= (iFileStatus == PS_PAUSED) ? 1 : 0;
				dFile.cSortRank |= (iFileStatus == PS_STOPPED) ? 2 : 0;
			}
			FilesArray.Add(dFile);
		}
	}

	if (!bLocalArray)
	{
		delete pvecPartFiles;
		pvecPartFiles = NULL;
	}

	// Sorting (simple bubble sort, we don't have tons of data here)
	bool	bSwap, bSortReverse, bSorted = true;
	int		iSortRank;
	DownloadFiles	*pFileArr = FilesArray.GetData();

	bSortReverse = m_pWSPrefs->abDownloadSortAsc[m_pWSPrefs->dwDownloadSort];
	for(int nMax = 0; bSorted && nMax < FilesArray.GetCount()*2; nMax++)
	{
		bSorted = false;
		for(int i = 0; i < FilesArray.GetCount() - 1; i++)
		{
			if ((iSortRank = (pFileArr[i].cSortRank - pFileArr[i + 1].cSortRank)) != 0)
				bSwap = (iSortRank < 0) ? false : true;
			else
			{
				switch (m_pWSPrefs->dwDownloadSort)
				{
					case WS_DLCOL_STATE:
						bSwap = _tcscmp(pFileArr[i].pcFileState, pFileArr[i+1].pcFileState) < 0;
						break;
					case WS_DLCOL_TYPE:
						bSwap = _tcscmp(pFileArr[i].pcFileType, pFileArr[i+1].pcFileType) < 0;
						break;
					case WS_DLCOL_NAME:
						bSwap = pFileArr[i].sFileName.CompareNoCase(pFileArr[i+1].sFileName) < 0;
						break;
					case WS_DLCOL_SIZE:
						bSwap = pFileArr[i].m_qwFileSize < pFileArr[i+1].m_qwFileSize;
						break;
					case WS_DLCOL_TRANSFERRED:
						bSwap = pFileArr[i].m_qwFileTransferred < pFileArr[i+1].m_qwFileTransferred;
						break;
					case WS_DLCOL_SPEED:
						bSwap = pFileArr[i].dwFileSpeed < pFileArr[i+1].dwFileSpeed;
						break;
					case WS_DLCOL_PROGRESS:
						bSwap = pFileArr[i].m_dblCompleted < pFileArr[i+1].m_dblCompleted;
						break;
					case WS_DLCOL_SOURCES:
						bSwap = pFileArr[i].lSourceCount < pFileArr[i+1].lSourceCount;
						break;
					case WS_DLCOL_PRIORITY:
						bSwap = pFileArr[i].nFilePrio < pFileArr[i+1].nFilePrio;
						break;
					case WS_DLCOL_CATEGORY:
						bSwap = pFileArr[i].sCategory.CompareNoCase(pFileArr[i+1].sCategory) < 0;
						break;
					case WS_DLCOL_FAKECHECK:
						bSwap = pFileArr[i].sFakeCheck.CompareNoCase(pFileArr[i+1].sFakeCheck) < 0;
						break;
					default:
						bSwap = false;
						break;
				}
				if (bSortReverse)
					bSwap = !bSwap;
			}
			if(bSwap)
			{
				bSorted = true;
				DownloadFiles TmpFile = pFileArr[i];
				pFileArr[i] = pFileArr[i+1];
				pFileArr[i+1] = TmpFile;
			}
		}
	}

	uint32	dwClientSoft;
	CArray<UploadUsers, UploadUsers> UploadArray;
	ClientList CopyUploadQueueList;

	g_App.m_pUploadQueue->GetCopyUploadQueueList(&CopyUploadQueueList);
	for (ClientList::const_iterator cIt = CopyUploadQueueList.begin(); cIt != CopyUploadQueueList.end(); cIt++)
	{
		CUpDownClient* cur_client = *cIt;
		UploadUsers dUser;

		dUser.sUserHash = HashToString(cur_client->GetUserHash());
		if (cur_client->GetUpDataRate() != 0)
		{
			dUser.pcActive = _T("downloading");
			dUser.pcClientState = _T("uploading");
		}
		else
		{
			dUser.pcActive = _T("waiting");
			dUser.pcClientState = _T("connecting");
		}
		dUser.sFileInfo = cur_client->GetUploadFileInfo();
		SpecialChars(&dUser.sFileInfo);
		dUser.sFileInfo.Replace(_T("\n"), _T("<br />"));
		dUser.sFileInfo.Replace(_T("'"), _T("&#8217;"));

		dwClientSoft = cur_client->GetClientSoft();
		if ( ((dwClientSoft == SO_EMULE) || (dwClientSoft == SO_PLUS)) &&
			(cur_client->m_pCredits != NULL) &&
			(cur_client->m_pCredits->GetCurrentIdentState(cur_client->GetIP()) != IS_IDENTIFIED) )
		{
			dwClientSoft = SO_OLDEMULE;
		}
		dUser.sClientSoft.Format(_T("%u"), dwClientSoft);

		if (cur_client->IsBanned())
			dUser.pcClientExtra = _T("banned");
		else if (cur_client->IsFriend())
			dUser.pcClientExtra = _T("friend");
		else if (cur_client->m_pCredits->HasHigherScoreRatio(cur_client->GetIP()))
			dUser.pcClientExtra = _T("credit");
		else
			dUser.pcClientExtra = _T("none");

		dUser.sUserName = cur_client->GetUserName();
		if(dUser.sUserName.GetLength() > SHORT_LENGTH_MIN)
		{
			dUser.sUserName.Truncate(SHORT_LENGTH_MIN - 3);
			SpecialChars(&dUser.sUserName);
			dUser.sUserName += _T("...");
		}
		else
			SpecialChars(&dUser.sUserName);

		CKnownFile* file = g_App.m_pSharedFilesList->GetFileByID(cur_client->m_reqFileHash);
		if (file != NULL)
		{
			dUser.sFileName = file->GetFileName();
			SpecialChars(&dUser.sFileName);
		}
		else
			dUser.sFileName = _GetPlainResString(IDS_REQ_UNKNOWNFILE);
		dUser.nTransferredDown = cur_client->GetTransferredDown();
		dUser.nTransferredUp = cur_client->GetTransferredUp();
		dUser.dwDataRate = cur_client->GetUpDataRate();
		dUser.sClientNameVersion = cur_client->GetFullSoftVersionString();
		UploadArray.Add(dUser);
	}

	UploadUsers	*pUploadArr = UploadArray.GetData();

	// Sorting (simple bubble sort, we don't have tons of data here)
	bSorted = true;
	bSortReverse = m_pWSPrefs->abUploadSortAsc[m_pWSPrefs->dwUploadSort];
	for(int nMax = 0; bSorted && nMax < UploadArray.GetCount()*2; nMax++)
	{
		bSorted = false;
		for(int i = 0; i < UploadArray.GetCount() - 1; i++)
		{
			bool bSwap = false;
			switch (m_pWSPrefs->dwUploadSort)
			{
				case WS_ULCOL_CLIENT:
					bSwap = pUploadArr[i].sClientSoft.Compare(pUploadArr[i+1].sClientSoft) < 0;
					break;
				case WS_ULCOL_USER:
					bSwap = pUploadArr[i].sUserName.CompareNoCase(pUploadArr[i+1].sUserName) < 0;
					break;
				case WS_ULCOL_VERSION:
					bSwap = pUploadArr[i].sClientNameVersion.CompareNoCase(pUploadArr[i+1].sClientNameVersion) < 0;
					break;
				case WS_ULCOL_FILENAME:
					bSwap = pUploadArr[i].sFileName.CompareNoCase(pUploadArr[i+1].sFileName) < 0;
					break;
				case WS_ULCOL_TRANSFERRED:
					bSwap = pUploadArr[i].nTransferredUp < pUploadArr[i+1].nTransferredUp;
					break;
				case WS_ULCOL_SPEED:
					bSwap = pUploadArr[i].dwDataRate < pUploadArr[i+1].dwDataRate;
					break;
			}
			if (bSortReverse)
				bSwap = !bSwap;
			if(bSwap)
			{
				bSorted = true;
				UploadUsers TmpUser = pUploadArr[i];
				pUploadArr[i] = pUploadArr[i+1];
				pUploadArr[i+1] = TmpUser;
			}
		}
	}

	int nCountQueue = 0;
	int nCountQueueBanned = 0;
	int nCountQueueFriend = 0;
	int nCountQueueCredit = 0;
	int nCountQueueSecure = 0;
	int nCountQueueBannedSecure = 0;
	int nCountQueueFriendSecure = 0;
	int nCountQueueCreditSecure = 0;
	int nNextPos = 0;	// position in queue of the user with the highest score -> next upload user
	uint32 dwNextScore = 0;	// highest score -> next upload user

	CQArray<QueueUsers, QueueUsers> QueueArray;
	for (POSITION pos = g_App.m_pUploadQueue->waitinglist.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* cur_client = g_App.m_pUploadQueue->waitinglist.GetNext(pos);
		QueueUsers dUser;

		bool bSecure = (cur_client->m_pCredits != NULL) &&
			(cur_client->m_pCredits->GetCurrentIdentState(cur_client->GetIP()) == IS_IDENTIFIED);
		if (cur_client->IsBanned())
		{
			dUser.pcClientState = _T("banned");
			dUser.iClientExtra = WS_WQUETYPE_BANNED;
			nCountQueueBanned++;
			if (bSecure) nCountQueueBannedSecure++;
		}
		else if (cur_client->IsFriend())
		{
			dUser.pcClientState = _T("friend");
			dUser.iClientExtra = WS_WQUETYPE_FRIEND;
			nCountQueueFriend++;
			if (bSecure) nCountQueueFriendSecure++;
		}
		else if (cur_client->m_pCredits->HasHigherScoreRatio(cur_client->GetIP()))
		{
			dUser.pcClientState = _T("credit");
			dUser.iClientExtra = WS_WQUETYPE_CREDIT;
			nCountQueueCredit++;
			if (bSecure) nCountQueueCreditSecure++;
		}
		else
		{
			dUser.pcClientState = _T("none");
			dUser.iClientExtra = WS_WQUETYPE_NONE;
			nCountQueue++;
			if (bSecure) nCountQueueSecure++;
		}

		dUser.sUserName = cur_client->GetUserName();
		if (dUser.sUserName.GetLength() > SHORT_LENGTH_MIN)
		{
			dUser.sUserName.Truncate(SHORT_LENGTH_MIN - 3);
			SpecialChars(&dUser.sUserName);
			dUser.sUserName += _T("...");
		}
		else
			SpecialChars(&dUser.sUserName);
		dUser.sClientNameVersion = cur_client->GetFullSoftVersionString();

		CKnownFile* file = g_App.m_pSharedFilesList->GetFileByID(cur_client->m_reqFileHash);
		if (file != NULL)
		{
			dUser.sFileName = file->GetFileName();
			SpecialChars(&dUser.sFileName);
		}
		else
			dUser.sFileName = _GetPlainResString(IDS_REQ_UNKNOWNFILE);
		dUser.pcClientStateSpecial = _T("connecting");
		if ((dUser.dwScore = cur_client->GetScore()) > dwNextScore)
		{
			nNextPos = QueueArray.GetSize();
			dwNextScore = dUser.dwScore;
		}

		dwClientSoft = cur_client->GetClientSoft();
		if ( ((dwClientSoft == SO_EMULE) || (dwClientSoft == SO_PLUS)) &&
			(cur_client->m_pCredits->GetCurrentIdentState(cur_client->GetIP()) != IS_IDENTIFIED) )
		{
			dwClientSoft = SO_OLDEMULE;
		}
		dUser.sClientSoft.Format(_T("%u"), dwClientSoft);
		dUser.sUserHash = HashToString(cur_client->GetUserHash());
		//SyruS CQArray-Sorting setting sIndex according to param
		switch (m_pWSPrefs->dwQueueSort)
		{
			case WS_QUCOL_CLIENT:
				dUser.sIndex = dUser.sClientSoft;
				break;
			case WS_QUCOL_USER:
				dUser.sIndex = dUser.sUserName;
				break;
			case WS_QUCOL_VERSION:
				dUser.sIndex = dUser.sClientNameVersion;
				break;
			case WS_QUCOL_FILENAME:
				dUser.sIndex = dUser.sFileName;
				break;
			case WS_QUCOL_SCORE:
				dUser.sIndex.Format(_T("%09u"), dUser.dwScore);
				break;
			default:
				dUser.sIndex.Empty();
		}
		QueueArray.Add(dUser);
	}

	QueueUsers	*pQueueArr = QueueArray.GetData();

	if (g_App.m_pUploadQueue->waitinglist.GetHeadPosition() != 0)
	{
		pQueueArr[nNextPos].pcClientStateSpecial = pQueueArr[nNextPos].pcClientState = _T("next");
	}

	if ((nCountQueue > 0 &&	m_pWSPrefs->bShowUploadQueue) ||
		(nCountQueueBanned > 0 && m_pWSPrefs->bShowUploadQueueBanned) ||
		(nCountQueueFriend > 0 && m_pWSPrefs->bShowUploadQueueFriend) ||
		(nCountQueueCredit > 0 && m_pWSPrefs->bShowUploadQueueCredit))
	{
#ifdef _DEBUG
		DWORD dwStart = ::GetTickCount();
#endif
		QueueArray.QuickSort(m_pWSPrefs->abQueueSortAsc[m_pWSPrefs->dwQueueSort]);
#ifdef _DEBUG
		AddLogLine(LOG_FL_DBG, _T("WebServer: Waitingqueue with %u elements sorted in %u ms"), QueueArray.GetSize(), ::GetTickCount()-dwStart);
#endif
	}

	// Displaying
	CString	sDownList, HTTPProcessData;
	CString	OutE(m_Templates.sTransferDownLine);
	CString	strFInfo;
	CString	ed2k;
	const TCHAR	*pcDownPrio, *pcIsGetFLC;
	bool	bIsA4AFAutoFile;
	byte	abyteUrlFHash[16];

	StringToHash(_ParseURL(Data.sURL, _T("file")), abyteUrlFHash);
	for (int i = 0; i < FilesArray.GetCount(); i++)
	{
		HTTPProcessData = OutE;

		StringToHash(pFileArr[i].sFileHash, abyteFileHash);
		if (md4cmp(abyteFileHash, abyteUrlFHash) == 0)
			HTTPProcessData.Replace(_T("[LastChangedDataset]"), _T("checked"));
		else
			HTTPProcessData.Replace(_T("[LastChangedDataset]"), _T("checked_no"));

		bIsA4AFAutoFile = ( g_App.m_pDownloadQueue->GetA4AFAutoFile() != NULL
			&& md4cmp0(abyteFileHash) != 0
			&& (g_App.m_pDownloadQueue->GetA4AFAutoFile() == g_App.m_pDownloadQueue->GetFileByID(abyteFileHash)) );

		strFInfo = pFileArr[i].sFileInfo;
		strFInfo.Replace(_T("\\"),_T("\\\\"));
		strFInfo.Replace(_T("\n"),_T("\\n"));
		strFInfo.Replace(_T("'"),_T("&#8217;"));

		ed2k = pFileArr[i].sED2kLink;
		ed2k.Replace(_T("'"), _T("&#8217;"));

		if (!pFileArr[i].bIsPreview)
			pcIsGetFLC = _T("");
		else if(pFileArr[i].bIsGetFLC)
			pcIsGetFLC = _T("enabled");
		else
			pcIsGetFLC = _T("disabled");

		if(pFileArr[i].bFileAutoPrio)
			pcDownPrio = _T("Auto");
		else
		{
			switch(pFileArr[i].nFilePrio)
			{
				case PR_LOW:
					pcDownPrio = _T("Low");
					break;
				case PR_NORMAL:
				default:
					pcDownPrio = _T("Normal");
					break;
				case PR_HIGH:
					pcDownPrio = _T("High");
					break;
			}
		}
		HTTPProcessData.Replace(_T("[admin]"), (bAdmin) ? _T("admin") : _T(""));
		HTTPProcessData.Replace(_T("[finfo]"), strFInfo);
		HTTPProcessData.Replace(_T("[ed2k]"), ed2k);
		HTTPProcessData.Replace(_T("[DownState]"), pFileArr[i].pcFileState);
		HTTPProcessData.Replace( _T("[autoa4af]"),
			(bIsA4AFAutoFile && !pFileArr[i].bIsComplete) ? _T("autoa4af") : _T("") );
		HTTPProcessData.Replace(_T("[isgetflc]"), pcIsGetFLC);
		HTTPProcessData.Replace(_T("[fname]"), pFileArr[i].sFileNameJS);
		HTTPProcessData.Replace(_T("[session]"), sSession);
		HTTPProcessData.Replace(_T("[filehash]"), pFileArr[i].sFileHash);
		HTTPProcessData.Replace(_T("[CatSel]"), sCat);
		HTTPProcessData.Replace(_T("[down-priority]"), pcDownPrio);
		HTTPProcessData.Replace(_T("[FileType]"), pFileArr[i].pcFileType);

		if (bIsA4AFAutoFile && !pFileArr[i].bIsComplete)
			HTTPProcessData.Replace(_T("[FileIsA4AF]"), _T("a4af"));
		else
			HTTPProcessData.Replace(_T("[FileIsA4AF]"), _T("halfnone"));

		if (pFileArr[i].bIsPreview && pFileArr[i].bIsGetFLC)
			HTTPProcessData.Replace(_T("[FileIsGetFLC]"), _T("getflc"));
		else
			HTTPProcessData.Replace(_T("[FileIsGetFLC]"), _T("halfnone"));

		if (!m_pWSPrefs->abDownloadColHidden[0])
		{
			if(pFileArr[i].sFileName.GetLength() > SHORT_LENGTH_MAX)
				HTTPProcessData.Replace(_T("[ShortFileName]"), pFileArr[i].sFileName.Left(SHORT_LENGTH_MAX-3) + _T("..."));
			else
				HTTPProcessData.Replace(_T("[ShortFileName]"), pFileArr[i].sFileName);
		}
		else
			HTTPProcessData.Replace(_T("[ShortFileName]"), _T(""));

		CString sTooltip = pFileArr[i].sFileInfo;
		sTooltip.Replace(_T("\n"), _T("<br />"));
		sTooltip.Replace(_T("'"), _T("&#8217;"));
		HTTPProcessData.Replace(_T("[FileInfo]"), sTooltip);

		qwTotalSize += pFileArr[i].m_qwFileSize;

		if (!m_pWSPrefs->abDownloadColHidden[1])
			HTTPProcessData.Replace(_T("[2]"), CastItoXBytes(pFileArr[i].m_qwFileSize));
		else
			HTTPProcessData.Replace(_T("[2]"), _T(""));

		if (!m_pWSPrefs->abDownloadColHidden[2])
		{
			if(pFileArr[i].m_qwFileTransferred > 0)
			{
				qwTotalTransferred += pFileArr[i].m_qwFileTransferred;
				HTTPProcessData.Replace(_T("[3]"), CastItoXBytes(pFileArr[i].m_qwFileTransferred));
			}
			else
				HTTPProcessData.Replace(_T("[3]"), _T("-"));
		}
		else
			HTTPProcessData.Replace(_T("[3]"), _T(""));

		if (!m_pWSPrefs->abDownloadColHidden[3])
			HTTPProcessData.Replace(_T("[DownloadBar]"), GetDownloadGraph(abyteFileHash));
		else
			HTTPProcessData.Replace(_T("[DownloadBar]"), _T(""));

		if (!m_pWSPrefs->abDownloadColHidden[4])
		{
			if (pFileArr[i].dwFileSpeed != 0)
			{
				dTmp = static_cast<double>(pFileArr[i].dwFileSpeed) / 1024.0;
				fTotalSpeed += dTmp;
				strTmp.Format(_T("%.2f"), dTmp);
				HTTPProcessData.Replace(_T("[4]"), strTmp);
			}
			else
				HTTPProcessData.Replace(_T("[4]"), _T("-"));
		}
		else
			HTTPProcessData.Replace(_T("[4]"), _T(""));

		if (!m_pWSPrefs->abDownloadColHidden[5])
		{
			if(pFileArr[i].lSourceCount > 0)
			{
				if (pFileArr[i].lNotCurrentSourceCount == 0)
				{
					strTmp.Format(_T("%u&nbsp;(%i)"),
						pFileArr[i].lSourceCount,
						pFileArr[i].lTransferringSourceCount);
				}
				else
				{
					strTmp.Format(_T("%u/%u&nbsp;(%i)"),
						pFileArr[i].lSourceCount - pFileArr[i].lNotCurrentSourceCount,
						pFileArr[i].lSourceCount,
						pFileArr[i].lTransferringSourceCount);
				}
				HTTPProcessData.Replace(_T("[5]"), strTmp);
			}
			else
				HTTPProcessData.Replace(_T("[5]"), _T("-"));
		}
		else
			HTTPProcessData.Replace(_T("[5]"), _T(""));

		if (!m_pWSPrefs->abDownloadColHidden[6])
		{
			UINT	dwResStrId;

			if(pFileArr[i].bFileAutoPrio)
			{
				switch(pFileArr[i].nFilePrio)
				{
					case PR_LOW:
						dwResStrId = IDS_PRIOAUTOLOW;
						break;
					case PR_NORMAL:
					default:
						dwResStrId = IDS_PRIOAUTONORMAL;
						break;
					case PR_HIGH:
						dwResStrId = IDS_PRIOAUTOHIGH;
						break;
				}
			}
			else
			{
				switch(pFileArr[i].nFilePrio)
				{
					case PR_LOW:
						dwResStrId = IDS_PRIOLOW;
						break;
					case PR_NORMAL:
					default:
						dwResStrId = IDS_PRIONORMAL;
						break;
					case PR_HIGH:
						dwResStrId = IDS_PRIOHIGH;
						break;
				}
			}
			HTTPProcessData.Replace(_T("[PrioVal]"), GetResString(dwResStrId));
		}
		else
			HTTPProcessData.Replace(_T("[PrioVal]"), _T(""));

		if (!m_pWSPrefs->abDownloadColHidden[7])
			HTTPProcessData.Replace(_T("[Category]"), pFileArr[i].sCategory);
		else
			HTTPProcessData.Replace(_T("[Category]"), _T(""));

		if (!m_pWSPrefs->abDownloadColHidden[8])
			HTTPProcessData.Replace(_T("[FakeCheck]"), pFileArr[i].sFakeCheck);
		else
			HTTPProcessData.Replace(_T("[FakeCheck]"), _T(""));

		InsertCatBox(HTTPProcessData, 0, _T(""), false, false, sSession, abyteFileHash);

		sDownList += HTTPProcessData;
	}

	Out.Replace(_T("[DownloadFilesList]"), sDownList);
	Out.Replace(_T("[TotalDownSize]"), CastItoXBytes(qwTotalSize));
	Out.Replace(_T("[TotalDownTransferred]"), CastItoXBytes(qwTotalTransferred));

	strTmp.Format(_T("%.2f"), fTotalSpeed);
	Out.Replace(_T("[TotalDownSpeed]"), strTmp);

	strTmp.Format( _T("%s: %u, %s: %u"), GetResString(IDS_SF_FILE), g_App.m_pDownloadQueue->GetFileCount(),
					 GetResString(IDS_ST_ACTIVE), g_App.m_pDownloadQueue->GetActiveFileCount());
	Out.Replace(_T("[TotalFiles]"), strTmp);

	strTmp.Format(_T("%u"), m_Templates.dwProgressbarWidth);
	Out.Replace(_T("[PROGRESSBARWIDTHVAL]"), strTmp);

	qwTotalSize = 0;
	qwTotalTransferred = 0;
	fTotalSpeed = 0;

	CString sUpList;

	OutE = m_Templates.sTransferUpLine;
	OutE.Replace(_T("[admin]"), (bAdmin) ? _T("admin") : _T(""));

	for(int i = 0; i < UploadArray.GetCount(); i++)
	{
		HTTPProcessData = OutE;

		HTTPProcessData.Replace(_T("[UserHash]"), pUploadArr[i].sUserHash);
		HTTPProcessData.Replace(_T("[UpState]"), pUploadArr[i].pcActive);
		HTTPProcessData.Replace(_T("[FileInfo]"), pUploadArr[i].sFileInfo);
		HTTPProcessData.Replace(_T("[ClientState]"), pUploadArr[i].pcClientState);
		HTTPProcessData.Replace(_T("[ClientSoft]"), pUploadArr[i].sClientSoft);
		HTTPProcessData.Replace(_T("[ClientExtra]"), pUploadArr[i].pcClientExtra);

		pcTmp = (!m_pWSPrefs->abUploadColHidden[0]) ? pUploadArr[i].sUserName.GetString() : _T("");
		HTTPProcessData.Replace(_T("[1]"), pcTmp);

		pcTmp = (!m_pWSPrefs->abUploadColHidden[1]) ? pUploadArr[i].sClientNameVersion.GetString() : _T("");
		HTTPProcessData.Replace(_T("[ClientSoftV]"), pcTmp);

		pcTmp = (!m_pWSPrefs->abUploadColHidden[2]) ? pUploadArr[i].sFileName.GetString() : _T("");
		HTTPProcessData.Replace(_T("[2]"), pcTmp);

		pcTmp = _T("");
		if (!m_pWSPrefs->abUploadColHidden[3])
		{
			qwTotalSize += pUploadArr[i].nTransferredDown;
			qwTotalTransferred += pUploadArr[i].nTransferredUp;
			strTmp.Format(_T("%s / %s"), CastItoXBytes(pUploadArr[i].nTransferredDown),CastItoXBytes(pUploadArr[i].nTransferredUp));
			pcTmp = strTmp;
		}
		HTTPProcessData.Replace(_T("[3]"), pcTmp);

		pcTmp = _T("");
		if (!m_pWSPrefs->abUploadColHidden[4])
		{
			dTmp = static_cast<double>(pUploadArr[i].dwDataRate) / 1024.0;
			fTotalSpeed += dTmp;
			strTmp.Format(_T("%.2f"), dTmp);
			pcTmp = strTmp;
		}
		HTTPProcessData.Replace(_T("[4]"), pcTmp);

		sUpList += HTTPProcessData;
	}
	Out.Replace(_T("[UploadFilesList]"), sUpList);
	strTmp.Format(_T("%s / %s"), CastItoXBytes(qwTotalSize), CastItoXBytes(qwTotalTransferred));
	Out.Replace(_T("[TotalUpTransferred]"), strTmp);
	strTmp.Format(_T("%.2f"), fTotalSpeed);
	Out.Replace(_T("[TotalUpSpeed]"), strTmp);

	if (m_pWSPrefs->bShowUploadQueue)
	{
		Out.Replace(_T("[UploadQueue]"), m_Templates.sTransferUpQueueShow);
		Out.Replace(_T("[UploadQueueList]"), _GetPlainResString(IDS_ONQUEUE));

		CString sQueue;

		OutE = m_Templates.sTransferUpQueueLine;
		OutE.Replace(_T("[admin]"), (bAdmin) ? _T("admin") : _T(""));

		for(int i = 0; i < QueueArray.GetCount(); i++)
		{
			TCHAR HTTPTempC[100] = _T("");

			if (pQueueArr[i].iClientExtra == WS_WQUETYPE_NONE)
			{
				HTTPProcessData = OutE;
				pcTmp = (!m_pWSPrefs->abQueueColHidden[0]) ? pQueueArr[i].sUserName.GetString() : _T("");
				HTTPProcessData.Replace(_T("[UserName]"), pcTmp);

				pcTmp = (!m_pWSPrefs->abQueueColHidden[1]) ? pQueueArr[i].sClientNameVersion.GetString() : _T("");
				HTTPProcessData.Replace(_T("[ClientSoftV]"), pcTmp);

				pcTmp = (!m_pWSPrefs->abQueueColHidden[2]) ? pQueueArr[i].sFileName.GetString() : _T("");
				HTTPProcessData.Replace(_T("[FileName]"), pcTmp);

				pcTmp = _T("");
				if (!m_pWSPrefs->abQueueColHidden[3])
				{
					_stprintf(HTTPTempC, _T("%u"), pQueueArr[i].dwScore);
					pcTmp = HTTPTempC;
				}
				HTTPProcessData.Replace(_T("[Score]"), pcTmp);
				HTTPProcessData.Replace(_T("[ClientState]"), pQueueArr[i].pcClientState);
				HTTPProcessData.Replace(_T("[ClientStateSpecial]"), pQueueArr[i].pcClientStateSpecial);
				HTTPProcessData.Replace(_T("[ClientSoft]"), pQueueArr[i].sClientSoft);
				HTTPProcessData.Replace(_T("[ClientExtra]"), _T("none"));
				HTTPProcessData.Replace(_T("[UserHash]"), pQueueArr[i].sUserHash);

				sQueue += HTTPProcessData;
			}
		}
		Out.Replace(_T("[QueueList]"), sQueue);
	}
	else
		Out.Replace(_T("[UploadQueue]"), m_Templates.sTransferUpQueueHide);

	if (m_pWSPrefs->bShowUploadQueueBanned)
	{
		Out.Replace(_T("[UploadQueueBanned]"), m_Templates.sTransferUpQueueBannedShow);
		Out.Replace(_T("[UploadQueueBannedList]"), _GetPlainResString(IDS_ONQUEUEBANNED));

		CString sQueueBanned;

		OutE = m_Templates.sTransferUpQueueBannedLine;
		OutE.Replace(_T("[admin]"), (bAdmin) ? _T("admin") : _T(""));

		for(int i = 0; i < QueueArray.GetCount(); i++)
		{
			TCHAR HTTPTempC[100] = _T("");

			if (pQueueArr[i].iClientExtra == WS_WQUETYPE_BANNED)
			{
				HTTPProcessData = OutE;
				pcTmp = (!m_pWSPrefs->abQueueColHidden[0]) ? pQueueArr[i].sUserName.GetString() : _T("");
				HTTPProcessData.Replace(_T("[UserName]"), pcTmp);

				pcTmp = (!m_pWSPrefs->abQueueColHidden[1]) ? pQueueArr[i].sClientNameVersion.GetString() : _T("");
				HTTPProcessData.Replace(_T("[ClientSoftV]"), pcTmp);

				pcTmp = (!m_pWSPrefs->abQueueColHidden[2]) ? pQueueArr[i].sFileName.GetString() : _T("");
				HTTPProcessData.Replace(_T("[FileName]"), pcTmp);

				pcTmp = _T("");
				if (!m_pWSPrefs->abQueueColHidden[3])
				{
					_stprintf(HTTPTempC, _T("%u"), pQueueArr[i].dwScore);
					pcTmp = HTTPTempC;
				}
				HTTPProcessData.Replace(_T("[Score]"), pcTmp);

				HTTPProcessData.Replace(_T("[ClientState]"), pQueueArr[i].pcClientState);
				HTTPProcessData.Replace(_T("[ClientStateSpecial]"), pQueueArr[i].pcClientStateSpecial);
				HTTPProcessData.Replace(_T("[ClientSoft]"), pQueueArr[i].sClientSoft);
				HTTPProcessData.Replace(_T("[ClientExtra]"), _T("banned"));
				HTTPProcessData.Replace(_T("[UserHash]"), pQueueArr[i].sUserHash);

				sQueueBanned += HTTPProcessData;
			}
		}
		Out.Replace(_T("[QueueListBanned]"), sQueueBanned);
	}
	else
		Out.Replace(_T("[UploadQueueBanned]"), m_Templates.sTransferUpQueueBannedHide);

	if (m_pWSPrefs->bShowUploadQueueFriend)
	{
		Out.Replace(_T("[UploadQueueFriend]"), m_Templates.sTransferUpQueueFriendShow);
		Out.Replace(_T("[UploadQueueFriendList]"), _GetPlainResString(IDS_ONQUEUEFRIEND));

		CString sQueueFriend;

		OutE = m_Templates.sTransferUpQueueFriendLine;
		OutE.Replace(_T("[admin]"), (bAdmin) ? _T("admin") : _T(""));

		for(int i = 0; i < QueueArray.GetCount(); i++)
		{
			TCHAR HTTPTempC[100] = _T("");

			if (pQueueArr[i].iClientExtra == WS_WQUETYPE_FRIEND)
			{
				HTTPProcessData = OutE;
				pcTmp = (!m_pWSPrefs->abQueueColHidden[0]) ? pQueueArr[i].sUserName.GetString() : _T("");
				HTTPProcessData.Replace(_T("[UserName]"), pcTmp);

				pcTmp = (!m_pWSPrefs->abQueueColHidden[1]) ? pQueueArr[i].sClientNameVersion.GetString() : _T("");
				HTTPProcessData.Replace(_T("[ClientSoftV]"), pcTmp);

				pcTmp = (!m_pWSPrefs->abQueueColHidden[2]) ? pQueueArr[i].sFileName.GetString() : _T("");
				HTTPProcessData.Replace(_T("[FileName]"), pcTmp);

				pcTmp = _T("");
				if (!m_pWSPrefs->abQueueColHidden[3])
				{
					_stprintf(HTTPTempC, _T("%u"), pQueueArr[i].dwScore);
					pcTmp = HTTPTempC;
				}
				HTTPProcessData.Replace(_T("[Score]"), pcTmp);

				HTTPProcessData.Replace(_T("[ClientState]"), pQueueArr[i].pcClientState);
				HTTPProcessData.Replace(_T("[ClientStateSpecial]"), pQueueArr[i].pcClientStateSpecial);
				HTTPProcessData.Replace(_T("[ClientSoft]"), pQueueArr[i].sClientSoft);
				HTTPProcessData.Replace(_T("[ClientExtra]"), _T("friend"));
				HTTPProcessData.Replace(_T("[UserHash]"), pQueueArr[i].sUserHash);

				sQueueFriend += HTTPProcessData;
			}
		}
		Out.Replace(_T("[QueueListFriend]"), sQueueFriend);
	}
	else
		Out.Replace(_T("[UploadQueueFriend]"), m_Templates.sTransferUpQueueFriendHide);

	if (m_pWSPrefs->bShowUploadQueueCredit)
	{
		Out.Replace(_T("[UploadQueueCredit]"), m_Templates.sTransferUpQueueCreditShow);
		Out.Replace(_T("[UploadQueueCreditList]"), _GetPlainResString(IDS_ONQUEUECREDIT));

		CString sQueueCredit;

		OutE = m_Templates.sTransferUpQueueCreditLine;
		OutE.Replace(_T("[admin]"), (bAdmin) ? _T("admin") : _T(""));

		for(int i = 0; i < QueueArray.GetCount(); i++)
		{
			TCHAR HTTPTempC[100] = _T("");

			if (pQueueArr[i].iClientExtra == WS_WQUETYPE_CREDIT)
			{
				HTTPProcessData = OutE;
				pcTmp = (!m_pWSPrefs->abQueueColHidden[0]) ? pQueueArr[i].sUserName.GetString() : _T("");
				HTTPProcessData.Replace(_T("[UserName]"), pcTmp);

				pcTmp = (!m_pWSPrefs->abQueueColHidden[1]) ? pQueueArr[i].sClientNameVersion.GetString() : _T("");
				HTTPProcessData.Replace(_T("[ClientSoftV]"), pcTmp);

				pcTmp = (!m_pWSPrefs->abQueueColHidden[2]) ? pQueueArr[i].sFileName.GetString() : _T("");
				HTTPProcessData.Replace(_T("[FileName]"), pcTmp);

				pcTmp = _T("");
				if (!m_pWSPrefs->abQueueColHidden[3])
				{
					_stprintf(HTTPTempC, _T("%u"), pQueueArr[i].dwScore);
					pcTmp =  HTTPTempC;
				}
				HTTPProcessData.Replace(_T("[Score]"), pcTmp);

				HTTPProcessData.Replace(_T("[ClientState]"), pQueueArr[i].pcClientState);
				HTTPProcessData.Replace(_T("[ClientStateSpecial]"), pQueueArr[i].pcClientStateSpecial);
				HTTPProcessData.Replace(_T("[ClientSoft]"), pQueueArr[i].sClientSoft);
				HTTPProcessData.Replace(_T("[ClientExtra]"), _T("credit"));
				HTTPProcessData.Replace(_T("[UserHash]"), pQueueArr[i].sUserHash);

				sQueueCredit += HTTPProcessData;
			}
		}
		Out.Replace(_T("[QueueListCredit]"), sQueueCredit);
	}
	else
		Out.Replace(_T("[UploadQueueCredit]"), m_Templates.sTransferUpQueueCreditHide);

	strTmp.Format(_T("%i"), nCountQueue);
	Out.Replace(_T("[CounterQueue]"), strTmp);
	strTmp.Format(_T("%i"), nCountQueueBanned);
	Out.Replace(_T("[CounterQueueBanned]"), strTmp);
	strTmp.Format(_T("%i"), nCountQueueFriend);
	Out.Replace(_T("[CounterQueueFriend]"), strTmp);
	strTmp.Format(_T("%i"), nCountQueueCredit);
	Out.Replace(_T("[CounterQueueCredit]"), strTmp);
	strTmp.Format(_T("%i"), nCountQueueSecure);
	Out.Replace(_T("[CounterQueueSecure]"), strTmp);
	strTmp.Format(_T("%i"), nCountQueueBannedSecure);
	Out.Replace(_T("[CounterQueueBannedSecure]"), strTmp);
	strTmp.Format(_T("%i"), nCountQueueFriendSecure);
	Out.Replace(_T("[CounterQueueFriendSecure]"), strTmp);
	strTmp.Format(_T("%i"), nCountQueueCreditSecure);
	Out.Replace(_T("[CounterQueueCreditSecure]"), strTmp);
	strTmp.Format(_T("%i"), nCountQueue+nCountQueueBanned+nCountQueueFriend+nCountQueueCredit);
	Out.Replace(_T("[CounterAll]"), strTmp);
	strTmp.Format(_T("%i"), nCountQueueSecure+nCountQueueBannedSecure+nCountQueueFriendSecure+nCountQueueCreditSecure);
	Out.Replace(_T("[CounterAllSecure]"), strTmp);
	Out.Replace(_T("[CatSel]"), sCat);
	Out.Replace(_T("[Session]"), sSession);
	Out.Replace(_T("[ShowUploadQueue]"), _GetPlainResString(IDS_VIEWQUEUE));
	Out.Replace(_T("[ShowUploadQueueList]"), _GetPlainResString(IDS_WEB_SHOW_UPLOAD_QUEUE, true));
	Out.Replace(_T("[ShowUploadQueueListBanned]"), _GetPlainResString(IDS_WEB_SHOW_UPLOAD_QUEUE_BANNED, true));
	Out.Replace(_T("[ShowUploadQueueListFriend]"), _GetPlainResString(IDS_WEB_SHOW_UPLOAD_QUEUE_FRIEND, true));
	Out.Replace(_T("[ShowUploadQueueListCredit]"), _GetPlainResString(IDS_WEB_SHOW_UPLOAD_QUEUE_CREDIT, true));

	pcTmp = (m_pWSPrefs->abQueueSortAsc[m_pWSPrefs->dwQueueSort]) ? _T("&amp;sortreverse=false") : _T("&amp;sortreverse=true");

	for (unsigned ui = 0; ui < ARRSIZE(s_apcQUSortTab2); ui++)
		Out.Replace(s_apcQUSortTab2[ui], (m_pWSPrefs->dwQueueSort == ui) ? pcTmp : _T(""));

	pcSortIcon = (m_pWSPrefs->abQueueSortAsc[m_pWSPrefs->dwQueueSort]) ? m_Templates.sUpArrow : m_Templates.sDownArrow;

	for (unsigned ui = 0; ui < ARRSIZE(s_aQUColTab); ui++)
	{
		GetPlainResString(&strTmp, s_aQUColTab[ui].dwResStrId, true);
		if (!m_pWSPrefs->abQueueColHidden[ui])
		{
			Out.Replace( s_aQUColTab[ui].pcColIcon,
				(m_pWSPrefs->dwQueueSort == (ui + WS_QUCOL_USER)) ? pcSortIcon : _T("") );
			Out.Replace(s_aQUColTab[ui].pcColHdr, strTmp);
		}
		else
		{
			Out.Replace(s_aQUColTab[ui].pcColIcon, _T(""));
			Out.Replace(s_aQUColTab[ui].pcColHdr, _T(""));
		}
		Out.Replace(s_aQUColTab[ui].pcColMenu, strTmp);
	}
	return Out;

	EMULE_CATCH

	return _T("");
}

CString CWebServer::GetSharedFilesList(const ThreadData &Data)
{
	static const TCHAR *s_apcSFLSortTab1[WS_SFLCOL_NUMCOLUMNS] =
	{
		_T("state"),		//WS_SFLCOL_STATE
		_T("type"),			//WS_SFLCOL_TYPE
		_T("name"),			//WS_SFLCOL_NAME
		_T("transferred"),	//WS_SFLCOL_TRANSFERRED
		_T("requests"),		//WS_SFLCOL_REQUESTS
		_T("accepts"),		//WS_SFLCOL_ACCEPTS
		_T("size"),			//WS_SFLCOL_SIZE
		_T("completes"),	//WS_SFLCOL_COMPLETES
		_T("priority")		//WS_SFLCOL_PRIORITY
	};
	static const TCHAR *s_apcSFLSortTab2[WS_SFLCOL_NUMCOLUMNS] =
	{
		_T("[SortState]"),		//WS_SFLCOL_STATE
		_T("[SortType]"),		//WS_SFLCOL_TYPE
		_T("[SortName]"),		//WS_SFLCOL_NAME
		_T("[SortTransferred]"),//WS_SFLCOL_TRANSFERRED
		_T("[SortRequests]"),	//WS_SFLCOL_REQUESTS
		_T("[SortAccepts]"),	//WS_SFLCOL_ACCEPTS
		_T("[SortSize]"),		//WS_SFLCOL_SIZE
		_T("[SortCompletes]"),	//WS_SFLCOL_COMPLETES
		_T("[SortPriority]")	//WS_SFLCOL_PRIORITY
	};
	static const WSListDefinition s_aSFLColTab[ARRSIZE(m_pWSPrefs->abSharedColHidden)] =
	{
		{ IDS_DL_FILENAME, _T("[FilenameI]"), _T("[Filename]"), _T("[FilenameM]") },
		{ IDS_SF_TRANSFERRED, _T("[FileTransferredI]"), _T("[FileTransferred]"), _T("[FileTransferredM]") },
		{ IDS_SF_REQUESTS, _T("[FileRequestsI]"), _T("[FileRequests]"), _T("[FileRequestsM]") },
		{ IDS_SF_ACCEPTS, _T("[FileAcceptsI]"), _T("[FileAccepts]"), _T("[FileAcceptsM]") },
		{ IDS_DL_SIZE, _T("[SizeI]"), _T("[Size]"), _T("[SizeM]") },
		{ IDS_SF_COMPLETESRC, _T("[CompletesI]"), _T("[Completes]"), _T("[CompletesM]") },
		{ IDS_PRIORITY, _T("[PriorityI]"), _T("[Priority]"), _T("[PriorityM]") }
	};
	EMULE_TRY

	int		iMsk, iSortMode, iCat = _tstoi(_ParseURL(Data.sURL, _T("cat"))) & 0xFF;
	CString	strTmp, sCat;

	if (iCat != 0)
		sCat.Format(_T("%u"), iCat);

	CString	sSession = _ParseURL(Data.sURL, _T("ses"));
	bool	bReload, bUpdate, bAdmin = IsSessionAdmin(Data, sSession);
	CString	strSort = _ParseURL(Data.sURL, _T("sort"));

	if (!strSort.IsEmpty())
	{
		for (unsigned ui = 0; ui < ARRSIZE(s_apcSFLSortTab1); ui++)
		{
			if (strSort.Compare(s_apcSFLSortTab1[ui]) == 0)
			{
				m_pWSPrefs->dwSharedSort = ui;
				break;
			}
		}
		strTmp = _ParseURL(Data.sURL, _T("sortAsc"));
		if (!strTmp.IsEmpty())
			m_pWSPrefs->aiSharedSortAsc[m_pWSPrefs->dwSharedSort] = (_tstoi(strTmp) & 3);
	}

	if (bAdmin)
	{
		strTmp = _ParseURL(Data.sURL, _T("hash"));
		if (!strTmp.IsEmpty())
		{
			byte	abyteFileHash[16];

			StringToHash(strTmp, abyteFileHash);
			if (md4cmp0(abyteFileHash) != 0)
			{
				CKnownFile	*pCurFile;

				if ((pCurFile = g_App.m_pSharedFilesList->GetFileByID(abyteFileHash)) != NULL)
				{
					bUpdate = false;

					strTmp = _ParseURL(Data.sURL, _T("prio"));
					if (!strTmp.IsEmpty())
					{
						pCurFile->SetAutoULPriority(false);
						if (strTmp == _T("verylow"))
							pCurFile->SetULPriority(PR_VERYLOW);
						else if (strTmp == _T("low"))
							pCurFile->SetULPriority(PR_LOW);
						else if (strTmp == _T("normal"))
							pCurFile->SetULPriority(PR_NORMAL);
						else if (strTmp == _T("high"))
							pCurFile->SetULPriority(PR_HIGH);
						else if (strTmp == _T("release"))
							pCurFile->SetULPriority(PR_RELEASE);
						else if (strTmp == _T("auto"))
						{
							pCurFile->SetAutoULPriority(true);
							pCurFile->UpdateUploadAutoPriority();
						}
						bUpdate = true;
					}

					strTmp = _ParseURL(Data.sURL, _T("jumpstart"));
					if (strTmp == _T("true"))
					{
#ifdef OLD_SOCKETS_ENABLED
					//	Don't enable JumpStart for small files
						if (pCurFile->GetFileSize() > PARTSIZE)
							pCurFile->SetJumpstartEnabled(true);
#endif
						bUpdate = true;
					}
					else if (strTmp == _T("false"))
					{
#ifdef OLD_SOCKETS_ENABLED
						pCurFile->SetJumpstartEnabled(false);
#endif
						bUpdate = true;
					}

					if (bUpdate)
						g_App.m_pSharedFilesList->UpdateItem(pCurFile);	// Refresh GUI
				}
			}
		}
	}

	strTmp = _ParseURL(Data.sURL, _T("c"));
	if (strTmp.CompareNoCase(_T("menu")) == 0)
		SetHiddenColumnState(Data.sURL, m_pWSPrefs->abSharedColHidden, ARRSIZE(m_pWSPrefs->abSharedColHidden));

	if ((bReload = (_ParseURL(Data.sURL, _T("reload")) == _T("true"))) == true)
		g_App.m_pMDlg->SendMessage(WEB_SHARED_FILES_RELOAD);

	CString		Out(m_Templates.sSharedList);
	const TCHAR	*pcTmp;

	iSortMode = m_pWSPrefs->dwSharedSort;
	iMsk = ( (iSortMode == WS_SFLCOL_TRANSFERRED) ||
		(iSortMode == WS_SFLCOL_REQUESTS) || (iSortMode == WS_SFLCOL_ACCEPTS) ) ? 3 : 1;
	iSortMode = m_pWSPrefs->aiSharedSortAsc[iSortMode];
	strTmp.Format(_T("&amp;sortAsc=%u"), (iSortMode + 1) & iMsk);
	for (unsigned ui = 0; ui < ARRSIZE(s_apcSFLSortTab2); ui++)
		Out.Replace(s_apcSFLSortTab2[ui], (m_pWSPrefs->dwSharedSort == ui) ? strTmp.GetString() : _T(""));

	pcTmp = _T("");
	if (bReload)
	{
		g_App.m_pMDlg->m_wndServer.m_pctlLogBox->GetLastLogEntry(&strTmp);	//Pick-up last line of the log
		SpecialChars(&strTmp);
		pcTmp = strTmp.GetString();
	}
	Out.Replace(_T("[Message]"), pcTmp);

	const TCHAR *pcSortIcon = (iSortMode & 2) ?
		((iSortMode & 1) ? m_Templates.strUpDoubleArrow : m_Templates.strDownDoubleArrow) :
		((iSortMode & 1) ? m_Templates.sUpArrow : m_Templates.sDownArrow);

	for (unsigned ui = 0; ui < ARRSIZE(s_aSFLColTab); ui++)
	{
		GetPlainResString(&strTmp, s_aSFLColTab[ui].dwResStrId, true);
		if (!m_pWSPrefs->abSharedColHidden[ui])
		{
			Out.Replace( s_aSFLColTab[ui].pcColIcon,
				(m_pWSPrefs->dwSharedSort == (ui + WS_SFLCOL_NAME)) ? pcSortIcon : _T("") );
			Out.Replace(s_aSFLColTab[ui].pcColHdr, strTmp);
		}
		else
		{
			Out.Replace(s_aSFLColTab[ui].pcColIcon, _T(""));
			Out.Replace(s_aSFLColTab[ui].pcColHdr, _T(""));
		}
		Out.Replace(s_aSFLColTab[ui].pcColMenu, strTmp);
	}

	Out.Replace(_T("[Actions]"), _GetPlainResString(IDS_WEB_ACTIONS));
	Out.Replace(_T("[Reload]"), _GetPlainResString(IDS_SF_RELOAD));
	Out.Replace(_T("[Session]"), sSession);
	Out.Replace(_T("[SharedList]"), _GetPlainResString(IDS_SHAREDFILES));
	Out.Replace(_T("[CatSel]"), sCat);

	CString OutE(m_Templates.sSharedLine);
	CArray<SharedFiles, SharedFiles> SharedArray;

	// Populating array
	for (POSITION pos = g_App.m_pSharedFilesList->m_mapSharedFiles.GetStartPosition(); pos != NULL;)
	{
		CCKey bufKey;
		CKnownFile* cur_file;
		uint16 nCountLo, nCountHi;

		g_App.m_pSharedFilesList->m_mapSharedFiles.GetNextAssoc(pos,bufKey,cur_file);

		bool bPartFile = cur_file->IsPartFile();

		SharedFiles dFile;

		dFile.bIsPartFile = cur_file->IsPartFile();
		dFile.sFileName = cur_file->GetFileName();
		if (bPartFile)
			dFile.pcFileState = _T("filedown");
		else
			dFile.pcFileState = _T("file");
		dFile.pcFileType = GetFileTypeForWebServer(dFile.sFileName);
		dFile.m_qwFileSize = cur_file->GetFileSize();
		dFile.sED2kLink = cur_file->CreateED2kLink();
		dFile.nFileTransferred = cur_file->statistic.GetTransferred();
		dFile.nFileAllTimeTransferred = cur_file->statistic.GetAllTimeTransferred();
		dFile.nFileRequests = cur_file->statistic.GetRequests();
		dFile.nFileAllTimeRequests = cur_file->statistic.GetAllTimeRequests();
		dFile.nFileAccepts = cur_file->statistic.GetAccepts();
		dFile.nFileAllTimeAccepts = cur_file->statistic.GetAllTimeAccepts();
		dFile.sFileHash = HashToString(cur_file->GetFileHash());

		if (bPartFile)
		{
			((CPartFile*)cur_file)->GetCompleteSourcesRange(&nCountLo, &nCountHi);
			dFile.sFileCompletes.Format(_T("%u"), ((CPartFile*)cur_file)->GetCompleteSourcesCount());
		}
		else
		{
			cur_file->GetCompleteSourcesRange(&nCountLo, &nCountHi);
			if (nCountLo == 0)
			{
				if (nCountHi == 0)
					dFile.sFileCompletes.Empty();
				else
					dFile.sFileCompletes.Format(_T("< %u"), nCountHi);
			}
			else if (nCountLo == nCountHi)
				dFile.sFileCompletes.Format(_T("%u"), nCountLo);
			else
				dFile.sFileCompletes.Format(_T("%u - %u"), nCountLo, nCountHi);
		}
		dFile.dblFileCompletes = ((nCountLo == 0) ? ((nCountHi == 0) ? ((bPartFile) ? 2.0 : 0.0) : (2.0 - (1.0 / static_cast<double>(nCountHi)))) : static_cast<double>(nCountLo) + 3.0 - (1.0 / static_cast<double>(nCountHi)));
		dFile.sFilePriority = cur_file->GetKnownFilePriorityString();
		dFile.nFilePriority = cur_file->GetULPriority();
		dFile.bFileAutoPriority = cur_file->IsULAutoPrioritized();
		SharedArray.Add(dFile);
	}

	// Sorting (simple bubble sort, we don't have tons of data here)
	bool	bSorted = true;

	iSortMode = m_pWSPrefs->aiSharedSortAsc[m_pWSPrefs->dwSharedSort];
	for (int nMax = 0; bSorted && nMax < SharedArray.GetCount()*2; nMax++)
	{
		bSorted = false;
		for(int i = 0; i < SharedArray.GetCount() - 1; i++)
		{
			bool bSwap = false;
			switch (m_pWSPrefs->dwSharedSort)
			{
				case WS_SFLCOL_STATE:
					bSwap = _tcscmp(SharedArray[i].pcFileState, SharedArray[i+1].pcFileState) > 0;
					break;
				case WS_SFLCOL_TYPE:
					bSwap = _tcscmp(SharedArray[i].pcFileType, SharedArray[i+1].pcFileType) > 0;
					break;
				case WS_SFLCOL_NAME:
					bSwap = SharedArray[i].sFileName.CompareNoCase(SharedArray[i+1].sFileName) < 0;
					break;
				case WS_SFLCOL_SIZE:
					bSwap = SharedArray[i].m_qwFileSize < SharedArray[i+1].m_qwFileSize;
					break;
				case WS_SFLCOL_TRANSFERRED:
					if (iSortMode & 2)
						bSwap = SharedArray[i].nFileAllTimeTransferred < SharedArray[i+1].nFileAllTimeTransferred;
					else
						bSwap = SharedArray[i].nFileTransferred < SharedArray[i+1].nFileTransferred;
					break;
				case WS_SFLCOL_REQUESTS:
					if (iSortMode & 2)
						bSwap = SharedArray[i].nFileAllTimeRequests < SharedArray[i+1].nFileAllTimeRequests;
					else
						bSwap = SharedArray[i].nFileRequests < SharedArray[i+1].nFileRequests;
					break;
				case WS_SFLCOL_ACCEPTS:
					if (iSortMode & 2)
						bSwap = SharedArray[i].nFileAllTimeAccepts < SharedArray[i+1].nFileAllTimeAccepts;
					else
						bSwap = SharedArray[i].nFileAccepts < SharedArray[i+1].nFileAccepts;
					break;
				case WS_SFLCOL_COMPLETES:
					bSwap = SharedArray[i].dblFileCompletes < SharedArray[i+1].dblFileCompletes;
					break;
				case WS_SFLCOL_PRIORITY:
					//	Very low priority is define equal to 4! Must adapt sorting code
					if (SharedArray[i].nFilePriority == 4)
						bSwap = (SharedArray[i+1].nFilePriority != 4);
					else if (SharedArray[i+1].nFilePriority == 4)
						bSwap = false;
					else
						bSwap = SharedArray[i].nFilePriority < SharedArray[i+1].nFilePriority;
					break;
			}
			if (iSortMode & 1)
				bSwap = !bSwap;
			if (bSwap)
			{
				bSorted = true;
				SharedFiles TmpFile = SharedArray[i];
				SharedArray[i] = SharedArray[i+1];
				SharedArray[i+1] = TmpFile;
			}
		}
	}
	// Displaying
	CString ed2k, fname, sSharedList, HTTPProcessData;

	for(int i = 0; i < SharedArray.GetCount(); i++)
	{
		TCHAR HTTPTempC[100] = _T("");
		HTTPProcessData = OutE;

		if (SharedArray[i].sFileHash == _ParseURL(Data.sURL, _T("hash")))
			HTTPProcessData.Replace(_T("[LastChangedDataset]"), _T("checked"));
		else
			HTTPProcessData.Replace(_T("[LastChangedDataset]"), _T("checked_no"));

		const TCHAR	*pcSharedPrio, *pcIsJumpStart = _T("");

		if (SharedArray[i].bFileAutoPriority)
			pcSharedPrio = _T("Auto");
		else
		{
			switch (SharedArray[i].nFilePriority)
			{
				case PR_VERYLOW:
					pcSharedPrio = _T("VeryLow");
					break;
				case PR_LOW:
					pcSharedPrio = _T("Low");
					break;
				default:
				case PR_NORMAL:
					pcSharedPrio = _T("Normal");
					break;
				case PR_HIGH:
					pcSharedPrio = _T("High");
					break;
				case PR_RELEASE:
					pcSharedPrio = _T("Release");
					break;
			}
		}

		ed2k = SharedArray[i].sED2kLink;
		ed2k.Replace(_T("'"),_T("&#8217;"));
		fname = SharedArray[i].sFileName;
		fname.Replace(_T("'"),_T("&#8217;"));

#ifdef OLD_SOCKETS_ENABLED
		byte	abyteFileHash[16];

		if (md4cmp0(StringToHash(SharedArray[i].sFileHash, abyteFileHash)) != 0)
		{
			CKnownFile	*pCurFile;

			HTTPProcessData.Replace(_T("[hash]"), SharedArray[i].sFileHash);
			if ((pCurFile = g_App.m_pSharedFilesList->GetFileByID(abyteFileHash)) != NULL)
			{
 				if (pCurFile->GetJumpstartEnabled())
				{
					pcIsJumpStart = _T("jumpstart");
					HTTPProcessData.Replace(_T("[FileIsPriority]"), _T("jumpstart"));
				}
				else if (pCurFile->GetULPriority() == PR_RELEASE)
					HTTPProcessData.Replace(_T("[FileIsPriority]"), _T("release"));
				else
					HTTPProcessData.Replace(_T("[FileIsPriority]"), _T("none"));
			}
		}
#endif //OLD_SOCKETS_ENABLED

		HTTPProcessData.Replace(_T("[admin]"), (bAdmin) ? _T("admin") : _T(""));
		HTTPProcessData.Replace(_T("[ed2k]"), ed2k);
		HTTPProcessData.Replace(_T("[fname]"), fname);
		HTTPProcessData.Replace(_T("[session]"), sSession);
		HTTPProcessData.Replace(_T("[shared-priority]"), pcSharedPrio);
		HTTPProcessData.Replace(_T("[isjumpstart]"), pcIsJumpStart);

		HTTPProcessData.Replace(_T("[FileName]"), _SpecialChars(SharedArray[i].sFileName));
		HTTPProcessData.Replace(_T("[FileType]"), SharedArray[i].pcFileType);
		HTTPProcessData.Replace(_T("[FileState]"), SharedArray[i].pcFileState);
		if (!m_pWSPrefs->abSharedColHidden[0])
		{
			if(SharedArray[i].sFileName.GetLength() > SHORT_LENGTH)
				HTTPProcessData.Replace(_T("[ShortFileName]"), _SpecialChars(SharedArray[i].sFileName.Left(SHORT_LENGTH-3)) + _T("..."));
			else
				HTTPProcessData.Replace(_T("[ShortFileName]"), _SpecialChars(SharedArray[i].sFileName));
		}
		else
			HTTPProcessData.Replace(_T("[ShortFileName]"), _T(""));
		if (!m_pWSPrefs->abSharedColHidden[1])
		{
			HTTPProcessData.Replace(_T("[FileTransferred]"), CastItoXBytes(SharedArray[i].nFileTransferred));
			_stprintf(HTTPTempC, _T(" (%s)"), CastItoXBytes(SharedArray[i].nFileAllTimeTransferred));
			HTTPProcessData.Replace(_T("[FileAllTimeTransferred]"), HTTPTempC);
		}
		else
		{
			HTTPProcessData.Replace(_T("[FileTransferred]"), _T(""));
			HTTPProcessData.Replace(_T("[FileAllTimeTransferred]"), _T(""));
		}
		if (!m_pWSPrefs->abSharedColHidden[2])
		{
			_stprintf(HTTPTempC, _T("%i"), SharedArray[i].nFileRequests);
			HTTPProcessData.Replace(_T("[FileRequests]"), HTTPTempC);
			_stprintf(HTTPTempC, _T(" (%i)"), SharedArray[i].nFileAllTimeRequests);
			HTTPProcessData.Replace(_T("[FileAllTimeRequests]"), HTTPTempC);
		}
		else
		{
			HTTPProcessData.Replace(_T("[FileRequests]"), _T(""));
			HTTPProcessData.Replace(_T("[FileAllTimeRequests]"), _T(""));
		}
		if (!m_pWSPrefs->abSharedColHidden[3])
		{
			_stprintf(HTTPTempC, _T("%i"), SharedArray[i].nFileAccepts);
			HTTPProcessData.Replace(_T("[FileAccepts]"), HTTPTempC);
			_stprintf(HTTPTempC, _T(" (%i)"), SharedArray[i].nFileAllTimeAccepts);
			HTTPProcessData.Replace(_T("[FileAllTimeAccepts]"), HTTPTempC);
		}
		else
		{
			HTTPProcessData.Replace(_T("[FileAccepts]"), _T(""));
			HTTPProcessData.Replace(_T("[FileAllTimeAccepts]"), _T(""));
		}
		if (!m_pWSPrefs->abSharedColHidden[4])
			HTTPProcessData.Replace(_T("[FileSize]"), CastItoXBytes(SharedArray[i].m_qwFileSize));
		else
			HTTPProcessData.Replace(_T("[FileSize]"), _T(""));
		if (!m_pWSPrefs->abSharedColHidden[5])
			HTTPProcessData.Replace(_T("[Completes]"), SharedArray[i].sFileCompletes);
		else
			HTTPProcessData.Replace(_T("[Completes]"), _T(""));
		if (!m_pWSPrefs->abSharedColHidden[6])
			HTTPProcessData.Replace(_T("[Priority]"), SharedArray[i].sFilePriority);
		else
			HTTPProcessData.Replace(_T("[Priority]"), _T(""));

		HTTPProcessData.Replace(_T("[FileHash]"), SharedArray[i].sFileHash);

		sSharedList += HTTPProcessData;
	}
	Out.Replace(_T("[SharedFilesList]"), sSharedList);
	Out.Replace(_T("[Session]"), sSession);

	return Out;

	EMULE_CATCH

	return _T("");
}

CString CWebServer::GetGraphs(const ThreadData &Data)
{
	NOPRM(Data);
	EMULE_TRY

	CString Out(m_Templates.sGraphs);
	CString strGraphDownload, strGraphUpload, strGraphCons;

	unsigned	uiLmt = (PointsForWeb.GetCount() < WEB_GRAPH_WIDTH) ? PointsForWeb.GetCount() : WEB_GRAPH_WIDTH;
	
	if (uiLmt != 0)
	{
		strGraphDownload.Format(_T("%u"), PointsForWeb[0].dwDownRate);
		strGraphUpload.Format(_T("%u"), PointsForWeb[0].dwUpRate);
		strGraphCons.Format(_T("%u"), PointsForWeb[0].dwConnections);
		for (unsigned ui = 1; ui < uiLmt; ui++)
		{
			strGraphDownload.AppendFormat(_T(",%u"), PointsForWeb[ui].dwDownRate);
			strGraphUpload.AppendFormat(_T(",%u"), PointsForWeb[ui].dwUpRate);
			strGraphCons.AppendFormat(_T(",%u"), PointsForWeb[ui].dwConnections);
		}
	}

	Out.Replace(_T("[GraphDownload]"), strGraphDownload);
	Out.Replace(_T("[GraphUpload]"), strGraphUpload);
	Out.Replace(_T("[GraphConnections]"), strGraphCons);

	Out.Replace(_T("[TxtDownload]"), _GetPlainResString(IDS_ST_ACTIVEDOWNLOAD));
	Out.Replace(_T("[TxtUpload]"), _GetPlainResString(IDS_ST_ACTIVEUPLOAD));
	Out.Replace(_T("[TxtTime]"), _GetPlainResString(IDS_TIME));
	Out.Replace(_T("[KByteSec]"), _GetPlainResString(IDS_KBYTESEC));
	Out.Replace(_T("[TxtConnections]"), _GetPlainResString(IDS_ST_ACTIVECONNECTIONS));

	Out.Replace(_T("[ScaleTime]"), CastSecondsToHM(g_App.m_pPrefs->GetTrafficOMeterInterval() * WEB_GRAPH_WIDTH));

	CString s1;
	s1.Format(_T("%u"), g_App.m_pPrefs->GetMaxGraphDownloadRate() / 10);
	Out.Replace(_T("[MaxDownload]"), s1);
	s1.Format(_T("%u"), g_App.m_pPrefs->GetMaxGraphUploadRate() / 10);
	Out.Replace(_T("[MaxUpload]"), s1);
	s1.Format(_T("%u"), g_App.m_pPrefs->GetMaxConnections());
	Out.Replace(_T("[MaxConnections]"), s1);

	return Out;

	EMULE_CATCH

	return _T("");
}

CString CWebServer::GetAddServerBox(const ThreadData &Data)
{
	EMULE_TRY

	int		iCat = _tstoi(_ParseURL(Data.sURL, _T("cat"))) & 0xFF;
	CString	sCat;

	if (iCat != 0)
		sCat.Format(_T("%u"), iCat);

	CString sSession = _ParseURL(Data.sURL, _T("ses"));
	const TCHAR	*pcTmp;

	if (!IsSessionAdmin(Data, sSession))
		return _T("");

	CString	strTmp, Out(m_Templates.sAddServerBox);

	if (_ParseURL(Data.sURL, _T("addserver")) == _T("true"))
	{
		CString	strSrvAddr(_ParseURL(Data.sURL, _T("serveraddr")));
		CString	strSrvPort(_ParseURL(Data.sURL, _T("serverport")));

		if (!strSrvAddr.IsEmpty() && !strSrvPort.IsEmpty())
		{
			g_App.m_pMDlg->m_wndServer.AddServer(strSrvAddr, strSrvPort, _ParseURL(Data.sURL, _T("servername")));

			CString	strResLog;
			uint16	uSrvPort = static_cast<uint16>(_tstoi(strSrvPort));
			CServer	*server = g_App.m_pServerList->GetServerByAddress(strSrvAddr, uSrvPort);

			g_App.m_pMDlg->m_wndServer.m_pctlLogBox->GetLastLogEntry(&strResLog); //Pick-up last line of the log
			SpecialChars(&strResLog);

			strTmp = _ParseURL(Data.sURL, _T("priority"));
			if (strTmp == _T("low"))
				server->SetPreference(PR_LOW);
			else if (strTmp == _T("normal"))
				server->SetPreference(PR_NORMAL);
			else if (strTmp == _T("high"))
				server->SetPreference(PR_HIGH);
			if (g_App.m_pMDlg->m_wndServer)
				g_App.m_pMDlg->m_wndServer.m_ctlServerList.RefreshServer(*server);

			if(_ParseURL(Data.sURL, _T("addtostatic")) == _T("true"))
			{
				_AddToStatic(strSrvAddr, uSrvPort);
				strResLog += _T("<br />");
				g_App.m_pMDlg->m_wndServer.m_pctlLogBox->GetLastLogEntry(&strTmp); //Pick-up last line of the log
				SpecialChars(&strTmp);
				strResLog += strTmp;
			}
			Out.Replace(_T("[Message]"), strResLog);
			if (_ParseURL(Data.sURL, _T("connectnow")) == _T("true"))
				_ConnectToServer(strSrvAddr, uSrvPort);
		}
		else
			Out.Replace(_T("[Message]"), _GetPlainResString(IDS_INVALIDSA));
	}
	else
	{
		pcTmp = _T("");
		if(_ParseURL(Data.sURL, _T("updateservermetfromurl")) == _T("true"))
		{
			g_App.m_pMDlg->m_wndServer.UpdateServerMetFromURL(_ParseURL(Data.sURL, _T("servermeturl")));

			g_App.m_pMDlg->m_wndServer.m_pctlLogBox->GetLastLogEntry(&strTmp);
			SpecialChars(&strTmp);
			pcTmp = strTmp.GetString();
		}
		Out.Replace(_T("[Message]"), pcTmp);
	}
	Out.Replace(_T("[AddServer]"), _GetPlainResString(IDS_SV_NEWSERVER));
	Out.Replace(_T("[IP]"), _GetPlainResString(IDS_SV_ADDRESS));
	Out.Replace(_T("[Port]"), _GetPlainResString(IDS_PORT));
	Out.Replace(_T("[Name]"), _GetPlainResString(IDS_SW_NAME));
	Out.Replace(_T("[Static]"), _GetPlainResString(IDS_STATICSERVER));
	Out.Replace(_T("[ConnectNow]"), _GetPlainResString(IDS_CONNECTNOW));
	Out.Replace(_T("[Priority]"), _GetPlainResString(IDS_PRIORITY));
	Out.Replace(_T("[Low]"), _GetPlainResString(IDS_PRIOLOW));
	Out.Replace(_T("[Normal]"), _GetPlainResString(IDS_PRIONORMAL));
	Out.Replace(_T("[High]"), _GetPlainResString(IDS_PRIOHIGH));
	Out.Replace(_T("[Add]"), _GetPlainResString(IDS_SV_ADD));
	Out.Replace(_T("[UpdateServerMetFromURL]"), _GetPlainResString(IDS_SV_MET));
	Out.Replace(_T("[URL]"), _GetPlainResString(IDS_SV_URL));
	Out.Replace(_T("[Apply]"), _GetPlainResString(IDS_PW_APPLY));

	strTmp.Format(_T("/?ses=%s&amp;w=server&amp;c=disconnect&amp;cat=%s"), sSession, sCat);
	Out.Replace(_T("[URL_Disconnect]"), strTmp);
	strTmp.Format(_T("/?ses=%s&amp;w=server&amp;c=connect&amp;cat=%s"), sSession, sCat);
	Out.Replace(_T("[URL_Connect]"), strTmp);

	Out.Replace(_T("[Disconnect]"), _GetPlainResString(IDS_IRC_DISCONNECT));
	Out.Replace(_T("[Connect]"), _GetPlainResString(IDS_CONNECTTOANYSERVER));
	Out.Replace(_T("[ServerOptions]"), _GetPlainResString(IDS_FSTAT_CONNECTION));
	Out.Replace(_T("[Execute]"), _GetPlainResString(IDS_IRC_PERFORM));
	Out.Replace(_T("[CatSel]"), sCat);

	return Out;

	EMULE_CATCH

	return _T("");
}

CString CWebServer::GetLog(const ThreadData &Data)
{
	EMULE_TRY

	int		iCat = _tstoi(_ParseURL(Data.sURL, _T("cat"))) & 0xFF;
	CString	sCat;

	if (iCat != 0)
		sCat.Format(_T("%u"), iCat);

	CString sSession = _ParseURL(Data.sURL, _T("ses"));
	CString Out(m_Templates.sLog);

	if ((_ParseURL(Data.sURL, _T("clear")) == _T("yes")) && IsSessionAdmin(Data,sSession))
	{
		g_App.m_pMDlg->m_wndServer.m_pctlLogBox->Reset();
		LRESULT pDummy;
		g_App.m_pMDlg->m_wndServer.OnTcnSelchangeTab1(NULL, &pDummy);
	}
	Out.Replace(_T("[Clear]"), _GetPlainResString(IDS_PW_RESET));
	Out.Replace(_T("[Log]"), g_App.m_pMDlg->m_wndServer.m_pctlLogBox->GetHtml() + _T("<br /><a name=\"end\"></a>"));
	Out.Replace(_T("[Session]"), sSession);
	Out.Replace(_T("[CatSel]"), sCat);

	return Out;

	EMULE_CATCH

	return _T("");
}

CString CWebServer::GetServerInfo(const ThreadData &Data)
{
	EMULE_TRY

	int		iCat = _tstoi(_ParseURL(Data.sURL, _T("cat"))) & 0xFF;
	CString	sCat;

	if (iCat != 0)
		sCat.Format(_T("%u"), iCat);

	CString sSession = _ParseURL(Data.sURL, _T("ses"));
	CString Out(m_Templates.sServerInfo);

	if ((_ParseURL(Data.sURL, _T("clear")) == _T("yes")) && IsSessionAdmin(Data,sSession))
	{
		g_App.m_pMDlg->m_wndServer.m_pctlServerMsgBox->Reset();
		LRESULT pDummy;
		g_App.m_pMDlg->m_wndServer.OnTcnSelchangeTab1(NULL, &pDummy);
	}
	Out.Replace(_T("[Clear]"), _GetPlainResString(IDS_PW_RESET));
	Out.Replace(_T("[ServerInfo]"), g_App.m_pMDlg->m_wndServer.m_pctlServerMsgBox->GetHtml() + _T("<br /><a name=\"end\"></a>"));
	Out.Replace(_T("[Session]"), sSession);
	Out.Replace(_T("[CatSel]"), sCat);

	return Out;

	EMULE_CATCH

	return _T("");
}

CString CWebServer::GetDebugLog(const ThreadData &Data)
{
	EMULE_TRY

	int		iCat = _tstoi(_ParseURL(Data.sURL, _T("cat"))) & 0xFF;
	CString	sCat;

	if (iCat != 0)
		sCat.Format(_T("%u"), iCat);

	CString sSession = _ParseURL(Data.sURL, _T("ses"));
	CString Out(m_Templates.sDebugLog);

	if ((_ParseURL(Data.sURL, _T("clear")) == _T("yes")) && IsSessionAdmin(Data,sSession))
	{
		g_App.m_pMDlg->m_wndServer.m_pctlDebugBox->Reset();
		LRESULT pDummy;
		g_App.m_pMDlg->m_wndServer.OnTcnSelchangeTab1(NULL, &pDummy);
	}
	Out.Replace(_T("[Clear]"), _GetPlainResString(IDS_PW_RESET));
	Out.Replace(_T("[DebugLog]"), g_App.m_pMDlg->m_wndServer.m_pctlDebugBox->GetHtml() + _T("<br /><a name=\"end\"></a>"));
	Out.Replace(_T("[Session]"), sSession);
	Out.Replace(_T("[CatSel]"), sCat);

	return Out;

	EMULE_CATCH

	return _T("");
}

CString CWebServer::_GetStats(const ThreadData &Data)
{
	EMULE_TRY

	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString sSession = _ParseURL(Data.sURL, _T("ses"));

	// refresh statistics
	g_App.m_pMDlg->m_dlgStatistics.ShowStatistics(true);

	CString Out = pThis->m_Templates.sStats;
	// eklmn: new stats
	Out.Replace(_T("[Stats]"), g_App.m_pMDlg->m_dlgStatistics.stattree.GetHTMLForExport());

	return Out;

	EMULE_CATCH

	return _T("");
}

CString CWebServer::GetPreferences(const ThreadData &Data)
{
	EMULE_TRY

	int		iCat = _tstoi(_ParseURL(Data.sURL, _T("cat"))) & 0xFF;
	CString	sCat, strTmp;

	if (iCat != 0)
		sCat.Format(_T("%u"), iCat);

	CString sSession = _ParseURL(Data.sURL, _T("ses"));
	CString Out(m_Templates.sPreferences);

	Out.Replace(_T("[Session]"), sSession);

	if ((_ParseURL(Data.sURL, _T("saveprefs")) == _T("true")) && IsSessionAdmin(Data, sSession))
	{
		strTmp = _ParseURL(Data.sURL, _T("showuploadqueue"));
		m_pWSPrefs->bShowUploadQueue = ((strTmp == _T("true")) || (strTmp.CompareNoCase(_T("on")) == 0));

		strTmp = _ParseURL(Data.sURL, _T("showuploadqueuebanned"));
		m_pWSPrefs->bShowUploadQueueBanned = ((strTmp == _T("true")) || (strTmp.CompareNoCase(_T("on")) == 0));

		strTmp = _ParseURL(Data.sURL, _T("showuploadqueuefriend"));
		m_pWSPrefs->bShowUploadQueueFriend = ((strTmp == _T("true")) || (strTmp.CompareNoCase(_T("on")) == 0));

		strTmp = _ParseURL(Data.sURL, _T("showuploadqueuecredit"));
		m_pWSPrefs->bShowUploadQueueCredit = ((strTmp == _T("true")) || (strTmp.CompareNoCase(_T("on")) == 0));

		strTmp = _ParseURL(Data.sURL, _T("refresh"));
		if (!strTmp.IsEmpty())
			g_App.m_pPrefs->SetWebPageRefresh(_tstoi(strTmp));

		strTmp = _ParseURL(Data.sURL, _T("maxcapdown"));
		if (!strTmp.IsEmpty())
			g_App.m_pPrefs->SetMaxGraphDownloadRate(ValidateDownCapability(10 * _tstoi(strTmp)));
		strTmp = _ParseURL(Data.sURL, _T("maxcapup"));
		if (!strTmp.IsEmpty())
			g_App.m_pPrefs->SetMaxGraphUploadRate(ValidateUpCapability(10 * _tstoi(strTmp)));

		uint32	dwSpeed;

		strTmp = _ParseURL(Data.sURL, _T("maxdown"));
		if (!strTmp.IsEmpty())
		{
			dwSpeed = String2FranctionalRate(strTmp);
			if (dwSpeed != 0)
			{
				g_App.m_pPrefs->SetMaxDownloadWithCheck(dwSpeed);
				g_App.m_pPrefs->SetLimitlessDownload(false);	//SyruS disable limitless on setting a specific value
			}
			else
				g_App.m_pPrefs->SetLimitlessDownload(true);	//SyruS enable limitless on setting to 0
		}
		strTmp = _ParseURL(Data.sURL, _T("maxup"));
		if (!strTmp.IsEmpty())
		{
			dwSpeed = String2FranctionalRate(strTmp);
			if (dwSpeed != 0)
				g_App.m_pPrefs->SetMaxUploadWithCheck(dwSpeed);
		}

		if(!_ParseURL(Data.sURL, _T("maxsources")).IsEmpty())
			g_App.m_pPrefs->SetMaxSourcePerFile(_tstoi(_ParseURL(Data.sURL, _T("maxsources"))));
		if(!_ParseURL(Data.sURL, _T("maxconnections")).IsEmpty())
			g_App.m_pPrefs->SetMaxConnections(static_cast<uint16>(_tstoi(_ParseURL(Data.sURL, _T("maxconnections")))));
		if(!_ParseURL(Data.sURL, _T("maxconnectionsperfive")).IsEmpty())
			g_App.m_pPrefs->SetMaxDownloadConperFive(_tstoi(_ParseURL(Data.sURL, _T("maxconnectionsperfive"))));
	}

	// Fill form
	Out.Replace(_T("[ShowUploadQueueVal]"), (m_pWSPrefs->bShowUploadQueue) ? _T("checked") : _T(""));
	Out.Replace(_T("[ShowUploadQueueBannedVal]"), (m_pWSPrefs->bShowUploadQueueBanned) ? _T("checked") : _T(""));
	Out.Replace(_T("[ShowUploadQueueFriendVal]"), (m_pWSPrefs->bShowUploadQueueFriend) ? _T("checked") : _T(""));
	Out.Replace(_T("[ShowUploadQueueCreditVal]"), (m_pWSPrefs->bShowUploadQueueCredit) ? _T("checked") : _T(""));

	strTmp.Format(_T("%d"), g_App.m_pPrefs->GetWebPageRefresh());
	Out.Replace(_T("[RefreshVal]"), strTmp);

	strTmp.Format(_T("%d"), g_App.m_pPrefs->GetMaxSourcePerFile());
	Out.Replace(_T("[MaxSourcesVal]"), strTmp);

	strTmp.Format(_T("%d"), g_App.m_pPrefs->GetMaxConnections());
	Out.Replace(_T("[MaxConnectionsVal]"), strTmp);

	strTmp.Format(_T("%d"), g_App.m_pPrefs->GetMaxConPerFive());
	Out.Replace(_T("[MaxConnectionsPer5Val]"), strTmp);

	Out.Replace(_T("[KBS]"), _GetPlainResString(IDS_KBYTESEC)+_T(":"));
	Out.Replace(_T("[LimitForm]"), _GetPlainResString(IDS_WEB_CONLIMITS)+_T(":"));
	Out.Replace(_T("[MaxSources]"), _GetPlainResString(IDS_PW_MAXSOURCES)+_T(":"));
	Out.Replace(_T("[MaxConnections]"), _GetPlainResString(IDS_PW_MAXC)+_T(":"));
	Out.Replace(_T("[MaxConnectionsPer5]"), _GetPlainResString(IDS_MAXCON5_TEXT)+_T(":"));
	Out.Replace(_T("[UseGzipForm]"), _GetPlainResString(IDS_WEB_GZIP_COMPRESSION));
	Out.Replace(_T("[ShowUploadQueueForm]"), _GetPlainResString(IDS_WEB_SHOW_UPLOAD_QUEUE));
	Out.Replace(_T("[ShowUploadQueueBannedForm]"), _GetPlainResString(IDS_WEB_SHOW_UPLOAD_QUEUE_BANNED));
	Out.Replace(_T("[ShowUploadQueueFriendForm]"), _GetPlainResString(IDS_WEB_SHOW_UPLOAD_QUEUE_FRIEND));
	Out.Replace(_T("[ShowUploadQueueCreditForm]"), _GetPlainResString(IDS_WEB_SHOW_UPLOAD_QUEUE_CREDIT));
	Out.Replace(_T("[ShowUploadQueueComment]"), _GetPlainResString(IDS_WEB_UPLOAD_QUEUE_COMMENT));
	Out.Replace(_T("[ShowUploadQueueBannedComment]"), _GetPlainResString(IDS_WEB_UPLOAD_QUEUE_BANNED_COMMENT));
	Out.Replace(_T("[ShowUploadQueueFriendComment]"), _GetPlainResString(IDS_WEB_UPLOAD_QUEUE_FRIEND_COMMENT));
	Out.Replace(_T("[ShowUploadQueueCreditComment]"), _GetPlainResString(IDS_WEB_UPLOAD_QUEUE_CREDIT_COMMENT));
	Out.Replace(_T("[ShowQueue]"), _GetPlainResString(IDS_SHOW));
	Out.Replace(_T("[HideQueue]"), _GetPlainResString(IDS_HIDE));
	Out.Replace(_T("[ShowQueueBanned]"), _GetPlainResString(IDS_SHOW));
	Out.Replace(_T("[HideQueueBanned]"), _GetPlainResString(IDS_HIDE));
	Out.Replace(_T("[ShowQueueFriend]"), _GetPlainResString(IDS_SHOW));
	Out.Replace(_T("[HideQueueFriend]"), _GetPlainResString(IDS_HIDE));
	Out.Replace(_T("[ShowQueueCredit]"), _GetPlainResString(IDS_SHOW));
	Out.Replace(_T("[HideQueueCredit]"), _GetPlainResString(IDS_HIDE));
	Out.Replace(_T("[RefreshTimeForm]"), _GetPlainResString(IDS_WEB_REFRESH_TIME));
	Out.Replace(_T("[RefreshTimeComment]"), _GetPlainResString(IDS_WEB_REFRESH_COMMENT));
	Out.Replace(_T("[SpeedForm]"), _GetPlainResString(IDS_SPEED_LIMITS));
	Out.Replace(_T("[MaxDown]"), _GetPlainResString(IDS_ST_ACTIVEDOWNLOAD));
	Out.Replace(_T("[MaxUp]"), _GetPlainResString(IDS_ST_ACTIVEUPLOAD));
	Out.Replace(_T("[SpeedCapForm]"), _GetPlainResString(IDS_CAPACITY_LIMITS));
	Out.Replace(_T("[MaxCapDown]"), _GetPlainResString(IDS_ST_ACTIVEDOWNLOAD));
	Out.Replace(_T("[MaxCapUp]"), _GetPlainResString(IDS_ST_ACTIVEUPLOAD));
	Out.Replace(_T("[WebControl]"), _GetPlainResString(IDS_WEB_CONTROL));
	Out.Replace(_T("[eMuleAppName]"), CLIENT_NAME);
	Out.Replace(_T("[Apply]"), _GetPlainResString(IDS_PW_APPLY));

//	Show always 0 when limitless is enabled otherwise it can be disabled on apply
	uint32	dwNum = (g_App.m_pPrefs->LimitlessDownload()) ? UNLIMITED : g_App.m_pPrefs->GetMaxDownload();

	FractionalRate2String(&strTmp, (dwNum == UNLIMITED) ? 0 : dwNum);
	Out.Replace(_T("[MaxDownVal]"), strTmp);
	FractionalRate2String(&strTmp, g_App.m_pPrefs->GetMaxUpload());
	Out.Replace(_T("[MaxUpVal]"), strTmp);
	strTmp.Format(_T("%u"), g_App.m_pPrefs->GetMaxGraphDownloadRate() / 10);
	Out.Replace(_T("[MaxCapDownVal]"), strTmp);
	strTmp.Format(_T("%u"), g_App.m_pPrefs->GetMaxGraphUploadRate() / 10);
	Out.Replace(_T("[MaxCapUpVal]"), strTmp);

	Out.Replace(_T("[CatSel]"), sCat);

	return Out;

	EMULE_CATCH

	return _T("");
}

CString CWebServer::_GetLoginScreen(const ThreadData &Data, bool bIsUseGzip)
{
	EMULE_TRY

	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString Out(pThis->m_Templates.sLogin);
	CString strTmp, strTmp2(_SpecialChars(_GetPlainResString(IDS_WEB_VALID), true));

	Out.Replace(_T("[CharSet]"), GetWebCharSet());
	Out.Replace(_T("[CapsErrorText]"), _SpecialChars(_GetPlainResString(IDS_WEB_CAPSERROR), true));

	strTmp.Format(strTmp2, _T("HTML"));
	Out.Replace(_T("[ValidHTML]"), strTmp);
	strTmp.Format(strTmp2, _T("CSS"));
	Out.Replace(_T("[ValidCSS]"), strTmp);
	strTmp2 = _SpecialChars(_GetPlainResString(IDS_WEB_CHECK), true);
	strTmp.Format(strTmp2, _T("HTML"));
	Out.Replace(_T("[CheckHTML]"), strTmp);
	strTmp.Format(strTmp2, _T("CSS"));
	Out.Replace(_T("[CheckCSS]"), strTmp);

	Out.Replace(_T("[Goto]"), _SpecialChars(_GetPlainResString(IDS_WEB_GOTO), true));
	Out.Replace(_T("[Nick]"), _SpecialChars(g_App.m_pPrefs->GetUserNick()));
	Out.Replace(_T("[eMuleAppName]"), CLIENT_NAME);
	Out.Replace(_T("[version]"), CURRENT_VERSION_LONG);
	Out.Replace(_T("[Login]"), _GetPlainResString(IDS_WEB_LOGIN));
	Out.Replace(_T("[EnterPassword]"), _GetPlainResString(IDS_WEB_ENTER_PASSWORD));
	Out.Replace(_T("[LoginNow]"), _GetPlainResString(IDS_WEB_LOGIN_NOW));
	Out.Replace(_T("[WebControl]"), _GetPlainResString(IDS_WEB_CONTROL));

	if(pThis->m_nIntruderDetect >= 1)
		Out.Replace(_T("[FailedLogin]"), _T("<p class=\"failed\">") + _GetPlainResString(IDS_WEB_BADLOGINATTEMPT) + _T("</p>"));
	else
		Out.Replace(_T("[FailedLogin]"), _T("&nbsp;"));

	Out.Replace(_T("[isGziped]"), GetResString((bIsUseGzip) ? IDS_ENABLED : IDS_DISABLED));

	return Out;

	EMULE_CATCH

	return _T("");
}

// We have to add gz-header and some other stuff
// to standard zlib functions
// in order to use gzip in web pages
int CWebServer::GzipCompress(byte *pbyteDst, unsigned *puiDstLen, const byte *pbyteSrc, unsigned uiSrcLen, int level)
{
	EMULE_TRY

	const static int gz_magic[2] = {0x1f, 0x8b}; // gzip magic header
	int err;
	uLong crc;
	z_stream stream = {0};
	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;
	stream.opaque = (voidpf)0;
	crc = crc32(0L, Z_NULL, 0);
	// init Zlib stream
	// NOTE windowBits is passed < 0 to suppress zlib header
	err = deflateInit2(&stream, level, Z_DEFLATED, -MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);
	if (err != Z_OK)
		return err;

	sprintf((char*)pbyteDst, "%c%c%c%c%c%c%c%c%c%c", gz_magic[0], gz_magic[1],
		Z_DEFLATED, 0 /*flags*/, 0,0,0,0 /*time*/, 0 /*xflags*/, 255);
	// wire buffers
	stream.next_in = (Bytef*)pbyteSrc;
	stream.avail_in = (uInt)uiSrcLen;
	stream.next_out = (Bytef*)(pbyteDst + 10);
	stream.avail_out = *puiDstLen - 18;
	// doit
	err = deflate(&stream, Z_FINISH);
	if (err != Z_STREAM_END)
	{
		deflateEnd(&stream);
		return err;
	}
	err = deflateEnd(&stream);
	crc = crc32(crc, (const Bytef*)pbyteSrc, uiSrcLen);
	//CRC
	*(pbyteDst + 10 + stream.total_out + 0) = (byte)(crc & 0xFF);
	*(pbyteDst + 10 + stream.total_out + 1) = (byte)((crc >> 8) & 0xFF);
	*(pbyteDst + 10 + stream.total_out + 2) = (byte)((crc >> 16) & 0xFF);
	*(pbyteDst + 10 + stream.total_out + 3) = (byte)((crc >> 24) & 0xFF);
	// Length
	*(pbyteDst + 10 + stream.total_out + 4) = (byte)(uiSrcLen & 0xFF);
	*(pbyteDst + 10 + stream.total_out + 5) = (byte)((uiSrcLen >> 8) & 0xFF);
	*(pbyteDst + 10 + stream.total_out + 6) = (byte)((uiSrcLen >> 16) & 0xFF);
	*(pbyteDst + 10 + stream.total_out + 7) = (byte)((uiSrcLen >> 24) & 0xFF);

	*puiDstLen = 10 + stream.total_out + 8;
	return err;

	EMULE_CATCH

	return -1;
}

bool CWebServer::IsLoggedIn(const ThreadData &Data, long lSession)
{
	NOPRM(Data);
	EMULE_TRY

	UpdateSessionCount();

	if (lSession != 0)
	{
	//	Find current session
		for (int i = 0; i < Sessions.GetSize(); i++)
		{
			if (Sessions[i].lSession == lSession)
				return true;
		}
	}

	EMULE_CATCH

	return false;
}

bool CWebServer::RemoveSession(const ThreadData &Data, long lSession)
{
	NOPRM(Data);

	if (lSession == 0)
		return false;

	EMULE_TRY

//	Find current session
	for (int i = 0; i < Sessions.GetSize(); i++)
	{
		if (Sessions[i].lSession == lSession)
		{
			Sessions.RemoveAt(i);
			CString t_ulCurIP;
			t_ulCurIP.Format( _T("%u.%u.%u.%u"),
				(byte)m_ulCurIP, (byte)(m_ulCurIP>>8), (byte)(m_ulCurIP>>16), (byte)(m_ulCurIP>>24) );
			g_App.m_pMDlg->AddLogLine(LOG_FL_SBAR | LOG_RGB_NOTICE, IDS_WEB_SESSIONEND, t_ulCurIP);
			return true;
		}
	}

	EMULE_CATCH

	return false;
}

bool CWebServer::GetSessionByID(Session *ses, long lSessionID, bool bUpdateTime)
{
	EMULE_TRY

	if (lSessionID != 0)
	{
		for (int i = 0; i < Sessions.GetSize(); i++)
		{
			if (Sessions[i].lSession == lSessionID)
			{
				if (bUpdateTime)	//	Reset expiration time if required
					Sessions[i].startTime = CTime::GetCurrentTime();
				*ses = Sessions.GetAt(i);
				return true;
			}
		}
	}

	EMULE_CATCH
	return false;
}

bool CWebServer::IsSessionAdmin(const ThreadData &Data, const CString &strSsessionID)
{
	NOPRM(Data);
	EMULE_TRY

	long sessionID = _tstol(strSsessionID);

	for (int i = 0; i < Sessions.GetSize(); i++)
	{
		if (Sessions[i].lSession == sessionID && sessionID != 0)
			return Sessions[i].admin;
	}

	EMULE_CATCH
	return false;
}

CString CWebServer::_GetPlainResString(UINT nID, bool noquote)
{
	EMULE_TRY

	CString sRet = GetResString(nID);
	sRet.Remove(_T('&'));
	if(noquote)
	{
		sRet.Replace(_T("'"), _T("&#8217;"));
		sRet.Replace(_T("\n"), _T("\\n"));
	}
	return sRet;

	EMULE_CATCH

	return _T("");
}

void CWebServer::GetPlainResString(CString *pstrOut, UINT nID, bool bNoQuote/*=false*/)
{
	GetResString(pstrOut, nID);
	pstrOut->Remove(_T('&'));
	if (bNoQuote)
	{
		pstrOut->Replace(_T("'"), _T("&#8217;"));
		pstrOut->Replace(_T("\n"), _T("\\n"));
	}
}

const TCHAR* CWebServer::GetWebCharSet()
{
#ifdef _UNICODE
	return _T("utf-8");
#else
	switch (g_App.m_pPrefs->GetLanguageID())
	{
		case MAKELANGID(LANG_POLISH,SUBLANG_DEFAULT):				return _T("windows-1250");
		case MAKELANGID(LANG_RUSSIAN,SUBLANG_DEFAULT):				return _T("windows-1251");
		case MAKELANGID(LANG_GREEK,SUBLANG_DEFAULT):				return _T("ISO-8859-7");
		case MAKELANGID(LANG_HEBREW,SUBLANG_DEFAULT):				return _T("ISO-8859-8-i");
		case MAKELANGID(LANG_KOREAN,SUBLANG_DEFAULT):				return _T("EUC-KR");
		case MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_SIMPLIFIED):	return _T("GB2312");
		case MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_TRADITIONAL):	return _T("Big5");
		case MAKELANGID(LANG_LITHUANIAN,SUBLANG_DEFAULT):			return _T("windows-1257");
		case MAKELANGID(LANG_ROMANIAN,SUBLANG_DEFAULT):				return _T("windows-1250");
		case MAKELANGID(LANG_TURKISH,SUBLANG_DEFAULT):				return _T("windows-1254");
	}

	// Western (Latin) includes Catalan, Danish, Dutch, English, Faeroese, Finnish, French,
	// German, Galician, Irish, Icelandic, Italian, Norwegian, Portuguese, Spanish and Swedish
	return _T("ISO-8859-1");
#endif
}

// Ornis: creating the progress bar
CString CWebServer::GetDownloadGraph(const byte *pbyteHash)
{
	EMULE_TRY

	static const TCHAR *const apcColors[2][12] =
	{
		{ _T("p_black.gif"), _T("p_blue6.gif"), _T("p_blue5.gif"),
		  _T("p_blue4.gif"), _T("p_blue3.gif"), _T("p_blue2.gif"), _T("p_blue1.gif"),
			_T("p_red.gif"), _T("p_yellow.gif"), _T("p_green.gif"), _T("p_greenpercent.gif"), _T("transparent.gif") },
		{ _T("black.gif"), _T("blue6.gif"), _T("blue5.gif"),
		  _T("blue4.gif"), _T("blue3.gif"), _T("blue2.gif"), _T("blue1.gif"),
		  _T("red.gif"), _T("yellow.gif"), _T("green.gif"), _T("greenpercent.gif"), _T("transparent.gif") }
	};
	CPartFile	*pPartFile = g_App.m_pDownloadQueue->GetFileByID(pbyteHash);
	CString		Out;
	const TCHAR	*const *progresscolor;
	EnumPartFileStatuses	eFileStatus;

	if ( pPartFile != NULL &&
		((eFileStatus = pPartFile->GetStatus()) == PS_PAUSED || eFileStatus == PS_STOPPED) )
	{
	//	Color style (paused files)
		progresscolor = &apcColors[0][0];
	}
	else
	{
	//	Color style (active files)
		progresscolor = &apcColors[1][0];
	}

	if (pPartFile == NULL || !pPartFile->IsPartFile())
	{
		Out.AppendFormat(m_Templates.sProgressbarImgsPercent, progresscolor[10], m_Templates.dwProgressbarWidth);
		Out.AppendFormat(m_Templates.sProgressbarImgs, progresscolor[9], m_Templates.dwProgressbarWidth);
	}
	else
	{
		CString	strChunkBar;
		uint32	dwLastColor = 1, dwColor, dwLastIdx = 0;
		int		compl = static_cast<int>((m_Templates.dwProgressbarWidth / 100.0) * pPartFile->GetPercentCompleted());

		strChunkBar.Preallocate(m_Templates.dwProgressbarWidth);
		pPartFile->GetProgressString(&strChunkBar, m_Templates.dwProgressbarWidth);
	//	Make a graph out of the array - need to be in a progressive way
		Out.AppendFormat(m_Templates.sProgressbarImgsPercent,
			progresscolor[(compl > 0) ? 10 : 11], (compl > 0) ? compl : 5);

		for (uint32 i = 0; i < m_Templates.dwProgressbarWidth; i++)
		{
			dwColor = strChunkBar.GetAt(i) - _T('0');
			if (dwLastColor != dwColor)
			{
				if (i > dwLastIdx)
					Out.AppendFormat(m_Templates.sProgressbarImgs, progresscolor[dwLastColor], i - dwLastIdx);

				dwLastColor = dwColor;
				dwLastIdx = i;
			}
		}
		Out.AppendFormat(m_Templates.sProgressbarImgs, progresscolor[dwLastColor], m_Templates.dwProgressbarWidth - dwLastIdx);
	}
	return Out;

	EMULE_CATCH

	return _T("");
}

CString	CWebServer::GetSearch(const ThreadData &Data)
{
	static const WSListDefinition s_aSLColTab[ARRSIZE(m_pWSPrefs->abSearchColHidden)] =
	{
		{ IDS_DL_FILENAME, _T("[FilenameI]"), _T("[FilenameH]"), _T("[FilenameM]") },
		{ IDS_DL_SIZE, _T("[FilesizeI]"), _T("[FilesizeH]"), _T("[FilesizeM]") },
		{ IDS_FILEHASH, _T("[FilehashI]"), _T("[FilehashH]"), _T("[FilehashM]") },
		{ IDS_DL_SOURCES, _T("[SourcesI]"), _T("[SourcesH]"), _T("[SourcesM]") },
		{ IDS_FAKE_CHECK_HEADER, _T("[FakeCheckI]"), _T("[FakeCheckH]"), _T("[FakeCheckM]") }
	};
	EMULE_TRY

	int		iCat = _tstoi(_ParseURL(Data.sURL, _T("cat"))) & 0xFF;
	CString	sSession = _ParseURL(Data.sURL, _T("ses"));
	CString	strTmp, Out(m_Templates.sSearch);
	bool	bAdmin = IsSessionAdmin(Data, sSession);

	if (bAdmin)
	{
		strTmp = ParseURLArray(Data.sURL, _T("downloads"));
		if (!strTmp.IsEmpty())
		{
			EnumCategories	eCatID = (iCat == 0) ? CAT_NONE : CCat::GetCatIDByUserIndex(iCat);
			int		iCurPos = 0;
			CString	strResToken;
			byte	abyteFileHash[16];

			for (;;)
			{
				strResToken = strTmp.Tokenize(_T("|"), iCurPos);
				if (strResToken.IsEmpty())
					break;
				if (md4cmp0(StringToHash(strResToken, abyteFileHash)) != 0)
					g_App.m_pSearchList->AddFileToDownloadByHash(abyteFileHash, eCatID);
			}

			iCat = (iCat == 0) ? 0 : CCat::UserCatIndexToCatIndex(iCat);
		}
	}

	CString sCat;

	if (iCat != 0)
		sCat.Format(_T("%u"), iCat);

	CString sCmd = _ParseURL(Data.sURL, _T("c"));

	if (bAdmin && (sCmd.CompareNoCase(_T("update")) == 0))
	{
		g_App.m_pPrefs->SetFakeListURL(_ParseURL(Data.sURL, _T("url")));
		g_App.m_pFakeCheck->UpdateFakeList();
	}

	strTmp = _ParseURL(Data.sURL, _T("tosearch"));
	if (!strTmp.IsEmpty() && bAdmin)
	{
		// perform search
		g_App.m_pMDlg->m_dlgSearch.DeleteAllSearches();
		g_App.m_pMDlg->m_dlgSearch.DoNewEd2kSearch(strTmp,
			_ParseURL(Data.sURL, _T("type")),
			static_cast<uint64>(_tstol(_ParseURL(Data.sURL, _T("min")))) * 1048576ui64,
			static_cast<uint64>(_tstol(_ParseURL(Data.sURL, _T("max")))) * 1048576ui64,
			_tstoi(_ParseURL(Data.sURL, _T("avail"))),
			_ParseURL(Data.sURL, _T("ext")),
			(_ParseURL(Data.sURL, _T("global")) == _T("on")),	_T(""));
		Out.Replace(_T("[Message]"),_GetPlainResString(IDS_SW_SEARCHINGINFO));
	}
	else if (!strTmp.IsEmpty() && !bAdmin)
		Out.Replace(_T("[Message]"), _GetPlainResString(IDS_ACCESSDENIED));
	else
		Out.Replace(_T("[Message]"), _GetPlainResString(IDS_SW_REFETCHRES));

	strTmp = _ParseURL(Data.sURL, _T("sort"));
	if (!strTmp.IsEmpty())
	{
		uint32	dwNewSort = _tstoi(strTmp);

		if (dwNewSort < WS_SLCOL_NUMCOLUMNS)
		{
			m_pWSPrefs->dwSearchSort = dwNewSort;
			strTmp = _ParseURL(Data.sURL, _T("sortAsc"));
			if (!strTmp.IsEmpty())
				m_pWSPrefs->abSearchSortAsc[dwNewSort] = (_tstoi(strTmp) != 0);
		}
	}

	if (sCmd.CompareNoCase(_T("menu")) == 0)
		SetHiddenColumnState(Data.sURL, m_pWSPrefs->abSearchColHidden, ARRSIZE(m_pWSPrefs->abSearchColHidden));

	CString strResultList = m_Templates.sSearchHeader;

	strResultList += g_App.m_pSearchList->GetWebList(m_Templates.sSearchResultLine,
		m_pWSPrefs->dwSearchSort, m_pWSPrefs->abSearchSortAsc[m_pWSPrefs->dwSearchSort],
		!m_pWSPrefs->abSearchColHidden[0], !m_pWSPrefs->abSearchColHidden[1],
		!m_pWSPrefs->abSearchColHidden[2], !m_pWSPrefs->abSearchColHidden[3], !m_pWSPrefs->abSearchColHidden[4]);
	if (CCat::GetNumCats() > 1)
		InsertCatBox(Out, 0, m_Templates.sCatArrow, false, false, sSession, NULL);
	else
		Out.Replace(_T("[CATBOX]"), _T(""));

	Out.Replace(_T("[SEARCHINFOMSG]"),_T(""));
	Out.Replace(_T("[RESULTLIST]"), strResultList);
	Out.Replace(_T("[Result]"), GetResString(IDS_SW_RESULT));
	Out.Replace(_T("[Session]"), sSession);
	Out.Replace(_T("[Name]"), _GetPlainResString(IDS_SW_NAME));
	Out.Replace(_T("[Type]"), _GetPlainResString(IDS_TYPE));
	Out.Replace(_T("[Any]"), _GetPlainResString(IDS_SEARCH_ANY));
	Out.Replace(_T("[Audio]"), _GetPlainResString(IDS_SEARCH_AUDIO));
	Out.Replace(_T("[Image]"), _GetPlainResString(IDS_SEARCH_PICS));
	Out.Replace(_T("[Video]"), _GetPlainResString(IDS_SEARCH_VIDEO));
	Out.Replace(_T("[Document]"), _GetPlainResString(IDS_SEARCH_DOC));
	Out.Replace(_T("[CDImage]"), _GetPlainResString(IDS_SEARCH_CDIMG));
	Out.Replace(_T("[Program]"), _GetPlainResString(IDS_SEARCH_PRG));
	Out.Replace(_T("[Archive]"), _GetPlainResString(IDS_SEARCH_ARC));
	Out.Replace(_T("[Search]"), _GetPlainResString(IDS_SEARCH_NOUN));
	Out.Replace(_T("[Show]"), _GetPlainResString(IDS_SHOW));
	Out.Replace(_T("[Size]"), _GetPlainResString(IDS_DL_SIZE));
	Out.Replace(_T("[Sort]"), _GetPlainResString(IDS_PW_SORTING));
	Out.Replace(_T("[SizeMin]"), _GetPlainResString(IDS_SEARCHMINSIZE));
	Out.Replace(_T("[SizeMax]"), _GetPlainResString(IDS_SEARCHMAXSIZE));
	Out.Replace(_T("[Availabl]"), _GetPlainResString(IDS_SEARCHAVAIL));
	Out.Replace(_T("[Extention]"), _GetPlainResString(IDS_SEARCHEXTENSION_LBL));
	Out.Replace(_T("[Global]"), _GetPlainResString(IDS_SW_GLOBAL));
	Out.Replace(_T("[MB]"), _GetPlainResString(IDS_MBYTES));
	Out.Replace(_T("[FakeListHeader]"), _GetPlainResString(IDS_FAKE_UPDATE));
	Out.Replace(_T("[FakeListText]"), _GetPlainResString(IDS_SV_ADDRESS));
	Out.Replace(_T("[FakeListUrl]"), g_App.m_pPrefs->GetFakeListURL());
	Out.Replace(_T("[Apply]"), _GetPlainResString(IDS_PW_APPLY));
	Out.Replace(_T("[CatSel]"), sCat);
	Out.Replace(_T("[checked]"),
		(g_App.m_pPrefs->GetSearchMethod() == EP_SEARCHMETHOD_GLOB) ? _T("checked") : _T(""));

	const TCHAR *pcSortIcon = (m_pWSPrefs->abSearchSortAsc[m_pWSPrefs->dwSearchSort]) ? m_Templates.sUpArrow : m_Templates.sDownArrow;

	for (unsigned ui = 0; ui < ARRSIZE(s_aSLColTab); ui++)
	{
		GetPlainResString(&strTmp, s_aSLColTab[ui].dwResStrId);
		if (!m_pWSPrefs->abSearchColHidden[ui])
		{
			Out.Replace( s_aSLColTab[ui].pcColIcon,
				(m_pWSPrefs->dwSearchSort == (ui + WS_SLCOL_FILENAME)) ? pcSortIcon : _T("") );
			Out.Replace(s_aSLColTab[ui].pcColHdr, strTmp);
		}
		else
		{
			Out.Replace(s_aSLColTab[ui].pcColIcon, _T(""));
			Out.Replace(s_aSLColTab[ui].pcColHdr, _T(""));
		}
		Out.Replace(s_aSLColTab[ui].pcColMenu, strTmp);
	}

	Out.Replace(_T("[Download]"), _GetPlainResString(IDS_DOWNLOAD_VERB));

	for (unsigned ui = 0; ui < WS_SLCOL_NUMCOLUMNS; ui++)
	{
		strTmp.Format(_T("[SORTASCVALUE%u]"), ui);
		Out.Replace( strTmp, (m_pWSPrefs->dwSearchSort == ui) ?
			((m_pWSPrefs->abSearchSortAsc[ui]) ? _T("&amp;sortAsc=0") : _T("&amp;sortAsc=1")) : _T("") );
	}

	return Out;

	EMULE_CATCH
	return _T("");
}

int CWebServer::UpdateSessionCount()
{
	EMULE_TRY

	for (int i = 0; i < Sessions.GetSize();)
	{
		CTimeSpan ts = CTime::GetCurrentTime() - Sessions[i].startTime;
		if(ts.GetTotalSeconds() > SESSION_TIMEOUT_SECS)
			Sessions.RemoveAt(i);
		else
			i++;
	}
	return Sessions.GetCount();

	EMULE_CATCH
	return 0;
}

void CWebServer::InsertCatBox(CString &Out, int preselect, const CString &strBoxLbl, bool jump, bool extraCats, const CString &strSession, const byte *pbyteHash)
{
	EMULE_TRY

	CString	strCategory, strTmp(strBoxLbl);

	strTmp += (jump) ?
		_T("<select name=\"cat\" size=\"1\"")
		_T(" onchange=GotoCat(this.form.cat.options[this.form.cat.selectedIndex].value)>") :
		_T("<select name=\"cat\" size=\"1\">");

	for (int i = 0; i-1 < CCat::GetNumUserCats(); i++)
	{
		strCategory = (i == 0) ? _GetPlainResString(IDS_CAT_UNCATEGORIZED) : CCat::GetCatByUserIndex(i)->GetTitle();
		strCategory.Replace(_T("'"),_T("\'"));
		strTmp.AppendFormat(_T("<option%s value=\"%i\">%s</option>"), (i == preselect) ? _T(" selected") : _T(""), i, strCategory);
	}
	if (extraCats)
	{
		if (CCat::GetNumCats() > 1)
			strTmp += _T("<option>------------</option>");

		for (int i = ((CCat::GetNumCats() > 1) ? 1 : 2); i <= 14; i++)
		{
			strTmp.AppendFormat(_T("<option%s value=\"%i\">%s</option>"), (i == preselect) ? _T(" selected") : _T(""), i+100, GetSubCatLabel(i));
		}
	}
	strTmp += _T("</select>");
	Out.Replace(_T("[CATBOX]"), strTmp);

	CString	tempBuff4;
	TCHAR	acHashStr[MAX_HASHSTR_SIZE], *pcTmp, *pcHashStr;

	strTmp = _T("");

	for (int i = 0; i < CCat::GetNumCats(); i++)
	{
		if (i == preselect)
		{
			pcTmp = _T("checked.gif");
			if (i < CCat::GetNumPredefinedCats())
				tempBuff4 = CCat::GetPredefinedCatTitle(CCat::GetCatIDByIndex(i));
			else
				tempBuff4 = CCat::GetCatByIndex(i)->GetTitle();
		}
		else
			pcTmp = _T("checked_no.gif");

		strCategory = (i < CCat::GetNumPredefinedCats()) ? CCat::GetPredefinedCatTitle(CCat::GetCatIDByIndex(i)) : CCat::GetCatByIndex(i)->GetTitle();
		strCategory.Replace(_T("'"), _T("\\'"));

		strTmp.AppendFormat(_T("<a href=&quot;/?ses=%s&amp;w=transfer&amp;cat=%d&quot;>")
			_T("<div class=menuitems><img class=menuchecked src=%s>%s&nbsp;</div></a>"),
			strSession, i, pcTmp, strCategory);
	}
	if (extraCats)
	{
		for (int i = ((CCat::GetNumCats() > 1) ? 1 : 2); i <= 14; i++)
		{
			if ((i + CAT_PREDEFINED) == preselect)
			{
				pcTmp = _T("checked.gif");
				tempBuff4 = GetSubCatLabel(i);
			}
			else
				pcTmp = _T("checked_no.gif");

			strTmp.AppendFormat(_T("<a href=&quot;/?ses=%s&amp;w=transfer&amp;cat=%d&quot;>")
				_T("<div class=menuitems><img class=menuchecked src=%s>%s&nbsp;</div></a>"),
				strSession, i + CAT_PREDEFINED, pcTmp, GetSubCatLabel(i));
		}
	}
	Out.Replace(_T("[CatBox]"), strTmp);
	Out.Replace(_T("[Category]"), tempBuff4);

	strTmp = _T("");

	CPartFile	*pPartFile;

	if (pbyteHash != NULL)
	{
		pPartFile = g_App.m_pDownloadQueue->GetFileByID(pbyteHash);
		pcHashStr = md4str(pbyteHash, acHashStr);
	}
	else
	{
		pPartFile = NULL;
		pcHashStr = _T("");
	}

//	For each user category index...
	for (int i = 0; i <= CCat::GetNumUserCats(); i++)
	{
	//	Get the user category index of 'pPartFile' in 'preselect'.
		if (pPartFile != NULL)
			preselect = CCat::GetUserCatIndexByID(pPartFile->GetCatID());

		strCategory = (i == 0)? GetResString(IDS_CAT_UNASSIGN) : CCat::GetCatByUserIndex(i)->GetTitle();
		strCategory.Replace(_T("'"),_T("\\'"));

		strTmp.AppendFormat(_T("<a href=&quot;/?ses=%s&amp;w=transfer[CatSel]&amp;op=setcat&amp;file=%s&amp;filecat=%u&quot;>")
			_T("<div class=menuitems><img class=menuchecked src=%s>%s&nbsp;</div></a>"),
			strSession, pcHashStr, i,
			(i == preselect) ? _T("checked.gif") : _T("checked_no.gif"), strCategory);
	}
	Out.Replace(_T("[SetCatBox]"), strTmp);

	EMULE_CATCH
}

CString CWebServer::GetSubCatLabel(int iCat)
{
	EMULE_TRY

	CString sPlain = CCat::GetPredefinedCatTitle(static_cast<_EnumCategories>(iCat+CAT_PREDEFINED));

	sPlain.Remove(_T('&'));
	sPlain.Replace(_T("'"), _T("&#8217;"));

	return sPlain;

	EMULE_CATCH
	return _T("");
}

long CWebServer::DoAuthentication(const ThreadData &Data, Session *ses)
{
	EMULE_TRY

	ses->lSession = 0;
	ses->startTime = 0;
	ses->admin = false;
	
	CString	strTmp(_ParseURL(Data.sURL, _T("ses")));

	if (!strTmp.IsEmpty())
		GetSessionByID(ses, _tstol(strTmp), true);
	else
	{
		if ( (_ParseURL(Data.sURL, _T("w")) == _T("password")) &&
			!GetIsTempDisabled() && _ParseURL(Data.sURL, _T("c")).IsEmpty() )
		{
			byte	abyteDigest[16];

			strTmp.Format(_T("%u.%u.%u.%u"),(byte)m_ulCurIP,(byte)(m_ulCurIP>>8),(byte)(m_ulCurIP>>16),(byte)(m_ulCurIP>>24));

			if (md4cmp(MD5Sum(_ParseURL(Data.sURL, _T("p")), abyteDigest), g_App.m_pPrefs->GetWSPass()) == 0)
			{
				ses->admin = true;
				ses->startTime = CTime::GetCurrentTime();
				ses->lSession = ((rand() << 15) | rand()) + 1;
				Sessions.Add(*ses);
				AddLogLine(LOG_FL_SBAR | LOG_RGB_NOTICE, IDS_WEB_ADMINLOGIN, strTmp);
				m_nIntruderDetect = 0;
			}
			else if ( g_App.m_pPrefs->GetWSIsLowUserEnabled() &&
				(md4cmp(abyteDigest, g_App.m_pPrefs->GetWSLowPass()) == 0) )
			{
				ses->startTime = CTime::GetCurrentTime();
				ses->lSession = ((rand() << 15) | rand()) + 1;
				Sessions.Add(*ses);
				AddLogLine(LOG_FL_SBAR | LOG_RGB_NOTICE, IDS_WEB_GUESTLOGIN, strTmp);
				m_nIntruderDetect = 0;
			}
			else if (g_App.m_pPrefs->IsWSIntruderDetectionEnabled())
			{
				m_nIntruderDetect++;
				AddLogLine(LOG_FL_SBAR | LOG_RGB_NOTICE, IDS_WEB_INTRTRIESLEFT, strTmp, g_App.m_pPrefs->GetWSLoginAttemptsAllowed() - m_nIntruderDetect);
				if (m_nIntruderDetect == g_App.m_pPrefs->GetWSLoginAttemptsAllowed())
				{
					CString MessageText;

					AddLogLine(LOG_FL_SBAR | LOG_RGB_NOTICE, IDS_WEB_INTRDETECTION, m_nIntruderDetect);
					if (g_App.m_pPrefs->GetWSTempDisableLogin() > 0)
					{
						m_nStartTempDisabledTime = ::GetTickCount();
						m_bIsTempDisabled = true;
						AddLogLine(LOG_FL_SBAR | LOG_RGB_NOTICE, IDS_WEB_INTRBLOCKTIME, g_App.m_pPrefs->GetWSTempDisableLogin());
						m_nIntruderDetect = 0;
						MessageText.Format(GetResString(IDS_WEB_INTRBLOCKTIME), g_App.m_pPrefs->GetWSTempDisableLogin());
					}
					else
					{
						g_App.m_pPrefs->SetWSIsEnabled(false);
						m_bServerWorking = false;
						StopSockets();
						AddLogLine(LOG_FL_SBAR | LOG_RGB_NOTICE, IDS_WEB_INTRWEBDISABLE);
						m_nIntruderDetect = 0;
						GetResString(&MessageText, IDS_WEB_INTRWEBDISABLE);
					}
					g_App.m_pMDlg->SendMail(MessageText, g_App.m_pPrefs->GetNotifierPopOnWebServerError(), g_App.m_pPrefs->IsSMTPWarningEnabled());
					g_App.m_pMDlg->ShowNotifier(MessageText, TBN_WEBSERVER, false, g_App.m_pPrefs->GetNotifierPopOnWebServerError());
				}
			}
			else
				AddLogLine(LOG_FL_SBAR | LOG_RGB_NOTICE, IDS_WEB_BADLOGINATTEMPT);
		}
	}

	return ses->lSession;

	EMULE_CATCH

	ses->lSession = 0;
	return 0;
}

void CWebServer::SetHiddenColumnState(const CString &strURL, bool *pbCols, unsigned uiArrSz)
{
	unsigned	uiMenu = _tstoi(_ParseURL(strURL, _T("m")));
	bool		bValue = (_tstoi(_ParseURL(strURL, _T("v"))) != 0);
	
	if (uiMenu >= uiArrSz)	//outside array boundary
		return;
	pbCols[uiMenu] = bValue;
}
