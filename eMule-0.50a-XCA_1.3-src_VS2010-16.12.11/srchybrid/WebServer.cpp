#include "stdafx.h"
#include "emule.h"
#include "StringConversion.h"
#include "WebServer.h"
#include "ClientCredits.h"
#include "ClientList.h"
#include "DownloadQueue.h"
#include "ED2KLink.h"
#include "emuledlg.h"
#include "FriendList.h"
#include "MD5Sum.h"
#include "ini2.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "KademliaWnd.h"
#include "KadSearchListCtrl.h"
#include "kademlia/kademlia/Entry.h"
#include "KnownFileList.h"
#include "ListenSocket.h"
#include "Log.h"
#include "MenuCmds.h"
#include "OtherFunctions.h"
#include "Preferences.h"
#include "Server.h"
#include "ServerList.h"
#include "ServerWnd.h"
#include "SearchList.h"
#include "SearchDlg.h"
#include "SearchParams.h"
#include "SharedFileList.h"
#include "Sockets.h"
#include "StatisticsDlg.h"
#include "Opcodes.h"
#include "TransferDlg.h"
#include "UploadQueue.h"
#include "UpDownClient.h"
#include "UserMsgs.h"
//Xman
// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
#include "BandWidthControl.h"
#include "Scheduler.h" // Don't reset Connection Settings for Webserver/CML/MM [Stulle] - Stulle
#include "Addons/Modname/Modname.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define HTTPENCODING _T("utf-8")
#define WEB_SERVER_TEMPLATES_VERSION	1

struct CmpQueueUsersByStr
{
	CmpQueueUsersByStr(bool _asc, size_t _index): asc(_asc), index(_index){}
	bool operator()(const QueueUsers& first, const QueueUsers& second) const{
		int ret = first.sortIndex[index].CompareNoCase(second.sortIndex[index]);
		return asc?(ret < 0):(ret > 0);
	}
	size_t index;
	bool asc;
};
struct CmpQueueUsersByScore
{
	CmpQueueUsersByScore(bool _asc): asc(_asc){}
	bool operator()(const QueueUsers& first, const QueueUsers& second) const{
		return asc?(first.nScore < second.nScore):(first.nScore > second.nScore);
	}
	bool asc;
};

struct CmpSearchFileByStr
{
	CmpSearchFileByStr(bool _asc, size_t _index): asc(_asc), index(_index){}
	bool operator()(const SearchFileStruct& first, const SearchFileStruct& second) const{
		int ret = first.sortIndex[index].CompareNoCase(second.sortIndex[index]);
		return asc?(ret < 0):(ret > 0);
	}
	size_t index;
	bool asc;
};
struct CmpSearchFileByFileSize
{
	CmpSearchFileByFileSize(bool _asc): asc(_asc){}
	bool operator()(const SearchFileStruct& first, const SearchFileStruct& second) const{
		return asc?(first.m_uFileSize < second.m_uFileSize):(first.m_uFileSize > second.m_uFileSize);
	}
	bool asc;
};
struct CmpSearchFileBySourceCount
{
	CmpSearchFileBySourceCount(bool _asc): asc(_asc){}
	bool operator()(const SearchFileStruct& first, const SearchFileStruct& second) const{
		return asc?(first.m_uSourceCount < second.m_uSourceCount):(first.m_uSourceCount > second.m_uSourceCount);
	}
	bool asc;
};

static BOOL	WSdownloadColumnHidden[8];
static BOOL	WSuploadColumnHidden[5];
static BOOL	WSqueueColumnHidden[4];
static BOOL	WSsharedColumnHidden[7];
static BOOL	WSserverColumnHidden[10];
static BOOL	WSsearchColumnHidden[4];

CWebServer::CWebServer(void)
{
	m_iSearchSortby=3;
	m_bSearchAsc = false;

	m_bServerWorking = false;

	m_nIntruderDetect = 0;
	m_nStartTempDisabledTime = 0;
	m_bIsTempDisabled = false;

	CIni ini( thePrefs.GetConfigFile(),_T("WebServer"));

	ini.SerGet(true, WSdownloadColumnHidden, _countof(WSdownloadColumnHidden), _T("downloadColumnHidden"));
	ini.SerGet(true, WSuploadColumnHidden, _countof(WSuploadColumnHidden), _T("uploadColumnHidden"));
	ini.SerGet(true, WSqueueColumnHidden, _countof(WSqueueColumnHidden), _T("queueColumnHidden"));
	ini.SerGet(true, WSsearchColumnHidden, _countof(WSsearchColumnHidden), _T("searchColumnHidden"));
	ini.SerGet(true, WSsharedColumnHidden, _countof(WSsharedColumnHidden), _T("sharedColumnHidden"));
	ini.SerGet(true, WSserverColumnHidden, _countof(WSserverColumnHidden), _T("serverColumnHidden"));

	m_Params.bShowUpload =			ini.GetBool(_T("ShowUpload"),false);
	m_Params.bShowUploadQueue =			ini.GetBool(_T("ShowUploadQueue"),false);

	m_Params.bDownloadSortReverse =	ini.GetBool(_T("DownloadSortReverse"),true);
	m_Params.bUploadSortReverse =	ini.GetBool(_T("UploadSortReverse"),true);
	m_Params.bQueueSortReverse =	ini.GetBool(_T("QueueSortReverse"),true);
	m_Params.bServerSortReverse =	ini.GetBool(_T("ServerSortReverse"),true);
	m_Params.bSharedSortReverse =	ini.GetBool(_T("SharedSortReverse"),true);

	m_Params.DownloadSort =	(DownloadSort)ini.GetInt(_T("DownloadSort"),DOWN_SORT_NAME);
	m_Params.UploadSort =	(UploadSort)ini.GetInt(_T("UploadSort"),UP_SORT_FILENAME);
	m_Params.QueueSort =	(QueueSort)ini.GetInt(_T("QueueSort"),QU_SORT_FILENAME);
	m_Params.ServerSort =	(ServerSort)ini.GetInt(_T("ServerSort"),SERVER_SORT_NAME);
	m_Params.SharedSort =	(SharedSort)ini.GetInt(_T("SharedSort"),SHARED_SORT_NAME);
}

CWebServer::~CWebServer(void)
{
	// save layout settings
	CIni ini( thePrefs.GetConfigFile(), _T("WebServer"));

	ini.WriteBool( _T("bShowUpload"), m_Params.bShowUpload);
	ini.WriteBool( _T("ShowUploadQueue"), m_Params.bShowUploadQueue );

	ini.WriteBool( _T("DownloadSortReverse"), m_Params.bDownloadSortReverse );
	ini.WriteBool( _T("UploadSortReverse"), m_Params.bUploadSortReverse );
	ini.WriteBool( _T("QueueSortReverse"), m_Params.bQueueSortReverse );
	ini.WriteBool( _T("ServerSortReverse"), m_Params.bServerSortReverse );
	ini.WriteBool( _T("SharedSortReverse"), m_Params.bSharedSortReverse );

	ini.WriteInt( _T("DownloadSort"), m_Params.DownloadSort);
	ini.WriteInt( _T("UploadSort"), m_Params.UploadSort);
	ini.WriteInt( _T("QueueSort"), m_Params.QueueSort);
	ini.WriteInt( _T("ServerSort"), m_Params.ServerSort);
	ini.WriteInt( _T("SharedSort"), m_Params.SharedSort);

	if (m_bServerWorking) StopSockets();
}

void CWebServer::SaveWIConfigArray(BOOL array[], int size, LPCTSTR key) {

	CIni ini(thePrefs.GetConfigFile(), _T("WebServer"));
	ini.SerGet(false, array, size, key);
}


void CWebServer::ReloadTemplates()
{
	CString sFile = thePrefs.GetTemplate();

	CStdioFile file;
	if (file.Open(sFile, CFile::modeRead|CFile::shareDenyWrite|CFile::typeText))
	{
		CString sAll, sLine;
		for(;;)
		{
			if(!file.ReadString(sLine))
				break;

			sAll += sLine;
			sAll += _T('\n');
		}
		file.Close();

		CString sVersion = _LoadTemplate(sAll,_T("TMPL_VERSIONX"));
		long lVersion = _tstol(sVersion);
		if(lVersion < WEB_SERVER_TEMPLATES_VERSION)
		{
			if(!theApp.DidWeAutoStart() && (thePrefs.GetWSIsEnabled() || m_bServerWorking))  {
				CString buffer;
				buffer.Format(GetResString(IDS_WS_ERR_LOADTEMPLATE), sFile);
				AddLogLine(true, buffer);
				theApp.SplashHide(SW_HIDE); //Xman new slpash-screen arrangement
				AfxMessageBox(buffer ,MB_OK);
				theApp.SplashHide(SW_SHOW); //Xman new slpash-screen arrangement
			}
			if (m_bServerWorking)
				StopSockets();
			m_bServerWorking = false;
			thePrefs.SetWSIsEnabled(false);
		}
		else
		{
			m_Templates.sMainWnd = _LoadTemplate(sAll,_T("TMPL_MAIN_WND"));
			m_Templates.sED2K = _LoadTemplate(sAll,_T("TMPL_ED2K"));
			m_Templates.sStatusBar = _LoadTemplate(sAll,_T("TMPL_STATUSBAR"));
			m_Templates.sServerWnd = _LoadTemplate(sAll,_T("TMPL_SERVER_WND"));
			m_Templates.sServerListFooter = _LoadTemplate(sAll,_T("TMPL_SERVER_LIST_FOOTER"));
			m_Templates.sServerLine = _LoadTemplate(sAll,_T("TMPL_SERVER_LINE"));
			m_Templates.sTransferWnd = _LoadTemplate(sAll,_T("TMPL_TRANSFER_WND"));
			m_Templates.sTransferDownFooter = _LoadTemplate(sAll,_T("TMPL_TRANSFER_DOWN_FOOTER"));
			m_Templates.sTransferDownLine = _LoadTemplate(sAll,_T("TMPL_TRANSFER_DOWN_LINE"));
			m_Templates.sTransferUpFooter = _LoadTemplate(sAll,_T("TMPL_TRANSFER_UP_FOOTER"));
			m_Templates.sTransferUpLine = _LoadTemplate(sAll,_T("TMPL_TRANSFER_UP_LINE"));
			m_Templates.sTransferUpQueueFooter = _LoadTemplate(sAll,_T("TMPL_TRANSFER_UP_QUEUE_FOOTER"));
			m_Templates.sTransferUpQueueLine = _LoadTemplate(sAll,_T("TMPL_TRANSFER_UP_QUEUE_LINE"));
			m_Templates.sSharedList = _LoadTemplate(sAll,_T("TMPL_SHARED_LIST"));
			m_Templates.sSharedListFooter = _LoadTemplate(sAll,_T("TMPL_SHARED_LIST_FOOTER"));
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
			m_Templates.iProgressbarWidth = (uint16)_tstoi(_LoadTemplate(sAll,_T("PROGRESSBARWIDTH")));
			m_Templates.sSearchResult = _LoadTemplate(sAll,_T("TMPL_SEARCH_RESULT"));
			m_Templates.sSearchResultFooter = _LoadTemplate(sAll,_T("TMPL_SEARCH_RESULT_FOOTER"));
			m_Templates.sSearchResultLine = _LoadTemplate(sAll,_T("TMPL_SEARCH_RESULT_LINE"));
			m_Templates.sProgressbarImgs = _LoadTemplate(sAll,_T("PROGRESSBARIMGS"));
			m_Templates.sProgressbarImgsPercent = _LoadTemplate(sAll,_T("PROGRESSBARPERCENTIMG"));
			m_Templates.sKad = _LoadTemplate(sAll,_T("TMPL_KADDLG"));
			m_Templates.sBootstrapLine= _LoadTemplate(sAll,_T("TMPL_BOOTSTRAPLINE"));
			m_Templates.sMyInfoLog= _LoadTemplate(sAll,_T("TMPL_MYINFO"));
			m_Templates.sCommentList= _LoadTemplate(sAll,_T("TMPL_COMMENTLIST"));
			m_Templates.sCommentListLine= _LoadTemplate(sAll,_T("TMPL_COMMENTLIST_LINE"));

			m_Templates.sProgressbarImgsPercent.Replace(_T("[PROGRESSGIFNAME]"),_T("%s"));
			m_Templates.sProgressbarImgsPercent.Replace(_T("[PROGRESSGIFINTERNAL]"),_T("%i"));
			m_Templates.sProgressbarImgs.Replace(_T("[PROGRESSGIFNAME]"),_T("%s"));
			m_Templates.sProgressbarImgs.Replace(_T("[PROGRESSGIFINTERNAL]"),_T("%i"));
		}
	}
	else if(m_bServerWorking)
	{
		AddLogLine(true, GetResString(IDS_WEB_ERR_CANTLOAD), sFile);
		StopSockets();
		m_bServerWorking = false;
		thePrefs.SetWSIsEnabled(false);
	}
}

CString CWebServer::_LoadTemplate(CString sAll, CString sTemplateName)
{
	CString sRet;
	int nStart = sAll.Find(_T("<--") + sTemplateName + _T("-->"));
	int nEnd = sAll.Find(_T("<--") + sTemplateName + _T("_END-->"));
	if(nStart != -1 && nEnd != -1 && nStart<nEnd)
	{
		nStart += sTemplateName.GetLength() + 7;
		sRet = sAll.Mid(nStart, nEnd - nStart - 1);
	}
	else
	{
		if (sTemplateName==_T("TMPL_VERSIONX"))
			AddLogLine(true, GetResString(IDS_WS_ERR_LOADTEMPLATE), sTemplateName);
		if (nStart==-1)
			AddLogLine(false,  GetResString(IDS_WEB_ERR_CANTLOAD), sTemplateName);
	}
	return sRet;
}

void CWebServer::RestartServer()
{	//Cax2 - restarts the server with the new port settings
	StopSockets();
	if (m_bServerWorking){
		StartSockets(this);
	}
}

void CWebServer::StartServer(void)
{
	
	if(m_bServerWorking == thePrefs.GetWSIsEnabled())
		return;
	m_bServerWorking = thePrefs.m_bWebEnabled;

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

	AddLogLine(false, _T("%s: %s"),_GetPlainResString(IDS_PW_WS), _GetPlainResString((thePrefs.GetWSIsEnabled() && m_bServerWorking)?IDS_ENABLED:IDS_DISABLED).MakeLower());
	
}

void CWebServer::_AddToStatic(CString sIP, int nPort)
{
	CServer* server=theApp.serverlist->GetServerByAddress(sIP, (uint16)nPort);
	if (server!=NULL) 
		SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION, WEBGUIIA_ADD_TO_STATIC, (LPARAM)server);
}

void CWebServer::AddStatsLine(UpDown line)
{
	m_Params.PointsForWeb.Add(line);
	if(m_Params.PointsForWeb.GetCount() > WEB_GRAPH_WIDTH)
		m_Params.PointsForWeb.RemoveAt(0);
}

CString CWebServer::_SpecialChars(CString str, bool noquote /*=false*/)
{
	str.Replace(_T("&"),_T("&amp;"));
	str.Replace(_T("<"),_T("&lt;"));
	str.Replace(_T(">"),_T("&gt;"));
	str.Replace(_T("\""),_T("&quot;"));
	if(noquote)
	{
		str.Replace(_T("'"), _T("&#8217;"));
		str.Replace(_T("\n"), _T("\\n"));
	}
	return str;
}

void CWebServer::_ConnectToServer(CString sIP, int nPort)
{
	CServer* server=theApp.serverlist->GetServerByAddress(sIP, (uint16)nPort);
	if (server!=NULL) 
		SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION,WEBGUIIA_CONNECTTOSERVER,(LPARAM)server);
}

void CWebServer::ProcessURL(ThreadData Data)
{
	if (theApp.m_app_state!=APP_STATE_RUNNING)
		return;

	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return;

	SetThreadLocale(thePrefs.GetLanguageID());

	//(0.29b)//////////////////////////////////////////////////////////////////
	// Here we are in real trouble! We are accessing the entire emule main thread
	// data without any syncronization!! Either we use the message pump for m_pdlgEmule
	// or use some hundreds of critical sections... For now, an exception handler
	// shoul avoid the worse things.
	//////////////////////////////////////////////////////////////////////////
	CoInitialize(NULL);

#ifndef _DEBUG
	try{
#endif	

		
	uint32 myip= inet_addr(ipstrA(Data.inadr));
	DWORD now=::GetTickCount();

	// check for being banned
	sint_ptr myfaults = 0;
	size_t i=0;
	while (i<pThis->m_Params.badlogins.GetCount() ) {
		if ( pThis->m_Params.badlogins[i].timestamp < now-MIN2MS(15) ) {
			pThis->m_Params.badlogins.RemoveAt(i);	// remove outdated entries
			continue;
		}

		if ( pThis->m_Params.badlogins[i].datalen==myip) 
			myfaults++;
		i++;
	}
	if (myfaults>4) {
		Data.pSocket->SendHtml(_GetPlainResString(IDS_ACCESSDENIED), false);
		CoUninitialize();
		return;
	}

	bool login=false;

	CString Out;
	bool justAddLink = false;
	long lSession = 0;
	CString sSession = _ParseURL(Data.sURL, _T("ses"));// X: [CI] - [Code Improvement]
	if(!sSession.IsEmpty())
		lSession = _tstol(sSession);

	CString sPage = _ParseURL(Data.sURL, _T("w"));// X: [CI] - [Code Improvement]
	if (sPage == _T("password"))
	{
		CString test=MD5Sum(_ParseURL(Data.sURL, _T("p"))).GetHash();
		CString ip=ipstr(Data.inadr);

        if (!_ParseURL(Data.sURL, _T("c")).IsEmpty()) {
            // just sent password to add link remotely. Don't start a session.
            justAddLink = true;
        }

		if(test == thePrefs.GetWSPass())// X: [CI] - [Code Improvement]
		{
	        if (!justAddLink) 
	        {
                // user wants to login
				Session ses;
				ses.admin=true;
				ses.startTime = CTime::GetCurrentTime();
				ses.lSession = lSession = t_rng->getUInt32();
				ses.lastcat = 0;
				ses.lastfilter = 0 - thePrefs.GetCatFilter(0);
				pThis->m_Params.Sessions.Add(ses);
				sSession.Format(_T("%ld"), lSession);
            }
			
			SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION,WEBGUIIA_UPDATEMYINFO,0);

			AddLogLine(true,GetResString(IDS_WEB_ADMINLOGIN)+_T(" (%s)"),ip);
			login=true;
		}
		else if(thePrefs.GetWSIsLowUserEnabled() && !thePrefs.GetWSLowPass().IsEmpty() && test == thePrefs.GetWSLowPass())// X: [CI] - [Code Improvement]
		{
			Session ses;
			ses.admin=false;
			ses.startTime = CTime::GetCurrentTime();
			ses.lSession = lSession = t_rng->getUInt32();
			pThis->m_Params.Sessions.Add(ses);
			sSession.Format(_T("%ld"), lSession);

			SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION,WEBGUIIA_UPDATEMYINFO,0);

			AddLogLine(true,GetResString(IDS_WEB_GUESTLOGIN)+_T(" (%s)"),ip);
			login=true;
		} else {
			LogWarning(LOG_STATUSBAR,GetResString(IDS_WEB_BADLOGINATTEMPT)+_T(" (%s)"),ip);

			BadLogin newban={myip, now};	// save failed attempt (ip,time)
			pThis->m_Params.badlogins.Add(newban);
			login=false;
			myfaults++;
			if (myfaults>4) {
				Data.pSocket->SendHtml(_GetPlainResString(IDS_ACCESSDENIED), false);
				CoUninitialize();
				return;
			}
		}
		if (login) {	// on login, forget previous failed attempts
			i=0;
			while (i<pThis->m_Params.badlogins.GetCount() ) {
				if ( pThis->m_Params.badlogins[i].datalen==myip ) {
					pThis->m_Params.badlogins.RemoveAt(i);
					continue;
				}
				i++;
			}
		}
	}
	else if (sPage == _T("logout"))
		_RemoveSession(Data, lSession);

	if(_IsLoggedIn(Data, lSession)){
		if(IsSessionAdmin(Data,sSession)){
			if(thePrefs.GetWebAdminAllowedHiLevFunc()){
				if (sPage == _T("close"))
				{
					theApp.m_app_state = APP_STATE_SHUTTINGDOWN;
					_RemoveSession(Data, lSession);

					// send answer ...
					Out = _T("<script>GoTo('/');</script>");
					Data.pSocket->SendHtml(Out, false);

					SendMessage(theApp.emuledlg->m_hWnd,WM_CLOSE,0,0);

					CoUninitialize();
					return;
				}
				if (sPage == _T("shutdown"))
				{
					_RemoveSession(Data, lSession);
					// send answer ...
					Out = _T("<script>GoTo('/');</script>");
					Data.pSocket->SendHtml(Out, false);

					SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION, WEBGUIIA_WINFUNC,1);

					CoUninitialize();
					return;
				}
				if (sPage == _T("reboot"))
				{
					_RemoveSession(Data, lSession);

					// send answer ...
					Out = _T("<script>GoTo('/');</script>");
					Data.pSocket->SendHtml(Out, false);

					SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION, WEBGUIIA_WINFUNC,2);

					CoUninitialize();
					return;
				}
			}
			if (sPage == _T("getfile")) {
				uchar FileHash[16];
				CKnownFile* kf=theApp.sharedfiles->GetFileByID(_GetFileHash(_ParseURL(Data.sURL, _T("filehash")),FileHash) );

				if (kf) {
					if (thePrefs.GetMaxWebUploadFileSizeMB() != 0 && kf->GetFileSize() > (uint64)thePrefs.GetMaxWebUploadFileSizeMB()*1024*1024) {
						Data.pSocket->SendReply( "HTTP/1.1 403 Forbidden\r\n" );

						CoUninitialize();
						return;
					}

					CFile file;
					if(file.Open(kf->GetFilePath(), CFile::modeRead|CFile::shareDenyWrite|CFile::typeBinary))
					{
						EMFileSize filesize= kf->GetFileSize();

#define SENDFILEBUFSIZE 2048
//>>> WiZaRd::Memleak FiX
						//char* buffer=(char*)malloc(SENDFILEBUFSIZE);
						char* buffer= new char[SENDFILEBUFSIZE];
//<<< WiZaRd::Memleak FiX
						if (!buffer) {
							Data.pSocket->SendReply( "HTTP/1.1 500 Internal Server Error\r\n" );
							CoUninitialize();
							return;
						}

						char szBuf[1024];
						int nLen = _snprintf(szBuf, _countof(szBuf), "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Description: \"%s\"\r\nContent-Disposition: attachment; filename=\"%s\";\r\nContent-Transfer-Encoding: binary\r\nContent-Length: %I64u\r\n\r\n", 
							(LPCSTR)CT2CA(kf->GetFileName()),
							(LPCSTR)CT2CA(kf->GetFileName()),
							(uint64)filesize);
						Data.pSocket->SendData(szBuf, nLen);

						DWORD r=1;
						while (filesize > (uint64)0 && r) {
							r=file.Read(buffer,SENDFILEBUFSIZE);
							filesize -= (uint64)r;
							Data.pSocket->SendData(buffer, r);
						}
						file.Close();

//>>> WiZaRd::Memleak FiX
//malloc --> free
//new --> delete/delete[]
						//free(buffer);
						delete[] buffer;
//<<< WiZaRd::Memleak FiX

						CoUninitialize();
						return;
					}

					Data.pSocket->SendReply( "HTTP/1.1 404 File not found\r\n" );
					CoUninitialize();
					return;
				}
			}
		}
		if (sPage == _T("commentlist"))
		{
			Out = _GetCommentlist(Data);
			if (!Out.IsEmpty()) {
				Data.pSocket->SendHtml(Out, thePrefs.GetWebUseGzip());
				CoUninitialize();
				return;
			}
		}
		//CString sPage = _ParseURL(Data.sURL, _T("w"));
		if (sPage == _T("status"))
			Out = _GetStatus(Data, lSession);
		else if (sPage == _T("transfer"))
			Out = _GetTransferWnd(Data);
		else if (sPage == _T("server"))
			Out = _GetServerWnd(Data);
		else if (sPage == _T("shared"))
			Out = _GetSharedFilesWnd(Data);
		else if (sPage == _T("search"))
			Out = _GetSearchWnd(Data);
		else if (sPage == _T("graphs"))
			Out = _GetGraphs(Data);
		else if (sPage == _T("log"))
			Out = _GetLog(Data);
		else if (sPage == _T("debuglog"))
			Out = _GetDebugLog(Data);
		else if (sPage == _T("myinfo"))
			Out = _GetMyInfo(Data);
		else if (sPage == _T("stats"))
			Out = _GetStats(Data);
		else if (sPage == _T("kad"))
			Out = _GetKadDlg(Data);
		else if (sPage == _T("options"))
			Out = _GetPreferences(Data);
		else
			Out = _GetMainWnd(Data, lSession);
	}
	else if(justAddLink && login)
        Out = _GetRemoteLinkAddedOk(Data);
	else if(lSession != 0)
			Out = _T("<script>GoTo('/');</script>");
		else if(justAddLink)
            Out = _GetRemoteLinkAddedFailed(Data);
		else
			Out = _GetLoginScreen(Data);

	// send answer ...
	Data.pSocket->SendHtml(Out, thePrefs.GetWebUseGzip());

#ifndef _DEBUG
	}
	catch(...){
		AddDebugLogLine( DLP_VERYHIGH, false, _T("*** Unknown exception in CWebServer::ProcessURL") );
		ASSERT(0);
	}	
#endif

	CoUninitialize();
}
/*
CString CWebServer::_ParseURLArray(CString URL, CString fieldname) {
	CString res,temp;

	while (URL.GetLength()>0) {
		int pos=URL.MakeLower().Find(fieldname.MakeLower() +_T('='));
		if (pos>-1) {
			temp=_ParseURL(URL,fieldname);
			if (temp.IsEmpty()) break;
			res.Append(temp+_T('|'));
			URL.Delete(pos,10);
		} else break;
	}
	return res;
}
*/
CString CWebServer::_ParseURL(CString URL, CString fieldname)
{
	CString value;

	if (URL.Find(_T('?')) > -1) {
		int findPos = -1;
		int findLength = 0;
		CString Parameter = URL.Mid(URL.Find(_T('?'))+1, URL.GetLength()-URL.Find(_T('?'))-1);

		// search the fieldname beginning / middle and strip the rest...
		if (Parameter.Find(fieldname + _T('=')) == 0) {
			findPos = 0;
			findLength = fieldname.GetLength() + 1;
		}
		if (Parameter.Find(_T('&') + fieldname + _T('=')) > -1) {
			findPos = Parameter.Find(_T('&') + fieldname + _T('='));
			findLength = fieldname.GetLength() + 2;
		}
		if (findPos > -1) {
			Parameter = Parameter.Mid(findPos + findLength, Parameter.GetLength());
			if (Parameter.Find(_T('&')) > -1) {
				Parameter = Parameter.Mid(0, Parameter.Find(_T('&')));
			}
	
			value = Parameter;

			// decode value ...
			value.Replace(_T('+'), _T(' '));
			value=URLDecode(value, true);
		}
	}

	value = OptUtf8ToStr(value);

	return value;
}

CString CWebServer::_GetMainWnd(ThreadData&Data, long lSession)
{
	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");


	CString strVersionCheck;
	strVersionCheck.Format(_T("/en/version_check.php?version=%i&language=%i"),theApp.m_uCurVersionCheck,thePrefs.GetLanguageID());
	strVersionCheck = thePrefs.GetVersionCheckBaseURL()+strVersionCheck;

	CString sSession; sSession.Format(_T("%ld"), lSession);
	bool bAdmin = IsSessionAdmin(Data,sSession);
	CString Out = pThis->m_Templates.sMainWnd;
	Out.Replace(_T("[CharSet]"), HTTPENCODING );
	Out.Replace(_T("[hiLevel]"), (bAdmin && thePrefs.GetWebAdminAllowedHiLevFunc() ) ? _T("true") : _T("false"));
	Out.Replace(_T("[admin]"), (bAdmin) ? _T("true") : _T("false"));
	Out.Replace(_T("[Session]"), sSession);
	Out.Replace(_T("[eMuleAppName]"), MOD_VERSION);
	Out.Replace(_T("[VersionCheck]"), strVersionCheck);
	CString sTemp;
	sTemp.Format(_T("%d"), thePrefs.GetWebPageRefresh()*1000);
	Out.Replace(_T("[RefreshVal]"), sTemp);
	Out.Replace(_T("[Lang]"), thePrefs.GetLangNameByID(thePrefs.GetLanguageID()));
	/*Out.Replace(_T("[WebControl]"), _GetPlainResString(IDS_PW_WS));
	Out.Replace(_T("[Transfer]"), _GetPlainResString(IDS_CD_TRANS));
	Out.Replace(_T("[Server]"), _GetPlainResString(IDS_SV_SERVERLIST));
	Out.Replace(_T("[Shared]"), _GetPlainResString(IDS_SHAREDFILES));
	Out.Replace(_T("[Graphs]"), _GetPlainResString(IDS_GRAPHS));
	Out.Replace(_T("[Log]"), _GetPlainResString(IDS_SV_LOG));
	Out.Replace(_T("[DebugLog]"), _GetPlainResString(IDS_SV_DEBUGLOG));
	Out.Replace(_T("[MyInfo]"), _GetPlainResString(IDS_MYINFO));
	Out.Replace(_T("[Stats]"), _GetPlainResString(IDS_SF_STATISTICS));
	Out.Replace(_T("[Options]"), _GetPlainResString(IDS_EM_PREFS));*/
	/*Out.Replace(_T("[Close]"), _GetPlainResString(IDS_WEB_SHUTDOWN));
	Out.Replace(_T("[Reboot]"), _GetPlainResString(IDS_WEB_REBOOT));
	Out.Replace(_T("[Shutdown]"), _GetPlainResString(IDS_WEB_SHUTDOWNSYSTEM));
	Out.Replace(_T("[WebOptions]"), _GetPlainResString(IDS_WEB_ADMINMENU));
	Out.Replace(_T("[Logout]"), _GetPlainResString(IDS_WEB_LOGOUT));
	Out.Replace(_T("[Search]"), _GetPlainResString(IDS_EM_SEARCH));
	Out.Replace(_T("[Kad]"), _GetPlainResString(IDS_KADEMLIA));

	Out.Replace(_T("[Refresh]"), _GetPlainResString(IDS_SV_UPDATE));
	Out.Replace(_T("[FileIsHashing]"), _GetPlainResString(IDS_HASHING));
	Out.Replace(_T("[FileIsErroneous]"), _GetPlainResString(IDS_ERRORLIKE));
	Out.Replace(_T("[FileIsCompleting]"), _GetPlainResString(IDS_COMPLETING));
	Out.Replace(_T("[FileDetails]"), _GetPlainResString(IDS_FD_TITLE));
	Out.Replace(_T("[FileComments]"), _GetPlainResString(IDS_CMT_SHOWALL));
	Out.Replace(_T("[ClearCompleted]"), _GetPlainResString(IDS_DL_CLEAR));
	Out.Replace(_T("[RunFile]"), _GetPlainResString(IDS_DOWNLOAD));
	Out.Replace(_T("[Resume]"), _GetPlainResString(IDS_DL_RESUME));
	Out.Replace(_T("[Stop]"), _GetPlainResString(IDS_DL_STOP));
	Out.Replace(_T("[Pause]"), _GetPlainResString(IDS_DL_PAUSE));
	Out.Replace(_T("[ConfirmCancel]"), _GetPlainResString(IDS_Q_CANCELDL2));
	Out.Replace(_T("[Cancel]"), _GetPlainResString(IDS_MAIN_BTN_CANCEL));
	Out.Replace(_T("[GetFLC]"), _GetPlainResString(IDS_DOWNLOADMOVIECHUNKS));
	Out.Replace(_T("[Rename]"), _GetPlainResString(IDS_RENAME));
	Out.Replace(_T("[ConnectAny]"), _GetPlainResString(IDS_CONNECTTOANYSERVER));
	Out.Replace(_T("[Connect]"), _GetPlainResString(IDS_IRC_CONNECT));
	Out.Replace(_T("[ConfirmRemove]"), _GetPlainResString(IDS_WEB_CONFIRM_REMOVE_SERVER));
	Out.Replace(_T("[ConfirmClose]"), _GetPlainResString(IDS_WEB_MAIN_CLOSE));
	Out.Replace(_T("[ConfirmReboot]"), _GetPlainResString(IDS_WEB_MAIN_REBOOT));
	Out.Replace(_T("[ConfirmShutdown]"), _GetPlainResString(IDS_WEB_MAIN_SHUTDOWN));
	Out.Replace(_T("[RemoveServer]"), _GetPlainResString(IDS_REMOVETHIS));
	Out.Replace(_T("[StaticServer]"), _GetPlainResString(IDS_STATICSERVER));
	Out.Replace(_T("[Friend]"), _GetPlainResString(IDS_PW_FRIENDS));

	Out.Replace(_T("[PriorityVeryLow]"), _GetPlainResString(IDS_PRIOVERYLOW));
	Out.Replace(_T("[PriorityLow]"), _GetPlainResString(IDS_PRIOLOW));
	Out.Replace(_T("[PriorityNormal]"), _GetPlainResString(IDS_PRIONORMAL));
	Out.Replace(_T("[PriorityHigh]"), _GetPlainResString(IDS_PRIOHIGH));
	Out.Replace(_T("[PriorityRelease]"), _GetPlainResString(IDS_PRIORELEASE));
	Out.Replace(_T("[PriorityPower-Release]"), _GetPlainResString(IDS_POWERRELEASE)); //Xman PowerRelease
	Out.Replace(_T("[PriorityAuto]"), _GetPlainResString(IDS_PRIOAUTO));*/

	if(bAdmin){
		CString sED2K = pThis->m_Templates.sED2K;
		sED2K.Replace(_T("[Ed2klink]"), _GetPlainResString(IDS_SW_LINK));
		sED2K.Replace(_T("[Download]"), _GetPlainResString(IDS_SW_DOWNLOAD));
		sED2K.Replace(_T("[Start]"), _GetPlainResString(IDS_SW_START));
		if (thePrefs.GetCatCount()>1) 
			InsertCatBox(sED2K,true);
		else 
			sED2K.Replace(_T("[CATBOXED2K]"),thePrefs.GetCategory(0)->strTitle);
		Out.Replace(_T("[ED2K]"), sED2K);
	}
	else
		Out.Replace(_T("[ED2K]"), _T(""));

	Out.Replace(_T("[StatusBar]"), _GetStatusBar(Data, bAdmin));

	return Out;
}

CString CWebServer::_GetStatusBar(ThreadData&Data, bool bAdmin)
{
	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString sStatusBar = pThis->m_Templates.sStatusBar;
	CString HTTPConState,HTTPConText;
	CString HTTPHelpU = _T("0");
	CString HTTPHelpF = _T("0");
	if ((theApp.serverconnect->IsConnecting()))
	{
		HTTPConState = _T("connecting");
		HTTPConText = _GetPlainResString(IDS_CONNECTING);
	}
	else if (theApp.serverconnect->IsConnected())
	{
		if(!theApp.serverconnect->IsLowID())
			HTTPConState = _T("high");
		else
			HTTPConState = _T("low");
		CServer* cur_server = theApp.serverlist->GetServerByAddress(
			theApp.serverconnect->GetCurrentServer()->GetAddress(),
			theApp.serverconnect->GetCurrentServer()->GetPort() );

		if (cur_server) {
			if(cur_server->GetListName().GetLength() > SHORT_LENGTH)
				HTTPConText = cur_server->GetListName().Left(SHORT_LENGTH-3) + _T("...");
			else
				HTTPConText = cur_server->GetListName();

			if (bAdmin) HTTPConText+=_T(" (<span onclick=\"servercmd('disconnect')\">")+_GetPlainResString(IDS_IRC_DISCONNECT)+_T("</span>)");

			HTTPHelpU = CastItoIShort(cur_server->GetUsers());
			HTTPHelpF = CastItoIShort(cur_server->GetFiles());
		}

	}
	else
	{
		HTTPConState = _T("disconnected");
		HTTPConText = _GetPlainResString(IDS_DISCONNECTED);
		if (bAdmin) HTTPConText+=_T(" (<span onclick=\"servercmd('connect')\">")+_GetPlainResString(IDS_CONNECTTOANYSERVER)+_T("</span>)");
	}
	sStatusBar.Replace(_T("[ConState]"), HTTPConState);
	sStatusBar.Replace(_T("[ConText]"), HTTPConText);

	CString HTTPHelpKU = _T("0");
	CString HTTPHelpKF = _T("0");
	// kad status
	if (Kademlia::CKademlia::IsConnected()) {
		HTTPHelpKU = CastItoIShort(Kademlia::CKademlia::GetKademliaUsers());
		HTTPHelpKF = CastItoIShort(Kademlia::CKademlia::GetKademliaFiles());
		if (Kademlia::CKademlia::IsFirewalled()) {
			HTTPConText=GetResString(IDS_FIREWALLED);
			HTTPConText.AppendFormat(_T("(<span onclick=\"kadcmd('rcfirewall')\">%s</span>"), GetResString(IDS_KAD_RECHECKFW) );
			HTTPConText.AppendFormat(_T(",<span onclick=\"kadcmd('disconnect')\">%s</span>)"), GetResString(IDS_IRC_DISCONNECT) );
		} else {
			HTTPConText=GetResString(IDS_CONNECTED);
			HTTPConText.AppendFormat(_T("(<span onclick=\"kadcmd('disconnect')\">%s</span>)"),  GetResString(IDS_IRC_DISCONNECT) );
		}
	}
	else {
		if (Kademlia::CKademlia::IsRunning()) {
			HTTPConText=GetResString(IDS_CONNECTING);
		} else {
			HTTPConText=GetResString(IDS_DISCONNECTED);
			HTTPConText.AppendFormat(_T("(<span onclick=\"kadcmd('connect')\">%s</span>)"),  GetResString(IDS_IRC_CONNECT) );
		}
	}
	sStatusBar.Replace(_T("[KadConText]"), HTTPConText);

	TCHAR HTTPHeader[100] = _T("");
	_sntprintf(HTTPHeader, _countof(HTTPHeader) - 1, _T("%s:%s(%s)|%s:%s(%s)"),
		_GetPlainResString(IDS_UUSERS), HTTPHelpU, HTTPHelpKU,
		_GetPlainResString(IDS_FILES), HTTPHelpF, HTTPHelpKF);
	sStatusBar.Replace(_T("[Panel1]"), HTTPHeader);

	//Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	uint32 eMuleIn;	uint32 eMuleInOverall;
	uint32 eMuleOut; uint32 eMuleOutOverall;
	uint32 notUsed;
	theApp.pBandWidthControl->GetDatarates(thePrefs.GetDatarateSamples(),
		eMuleIn, eMuleInOverall,
		eMuleOut, eMuleOutOverall,
		notUsed, notUsed);

	LPCTSTR strTemp;
	if (eMuleOut && eMuleIn)
		strTemp = _T("u1d1");
	else if (eMuleOut)
		strTemp = _T("u1d0");
	else if (eMuleIn)
		strTemp = _T("u0d1");
	else
		strTemp = _T("u0d0");

	sStatusBar.Replace(_T("[UpDownState]"), strTemp);

	_sntprintf(HTTPHeader, _countof(HTTPHeader) - 1, _GetPlainResString(IDS_UPDOWN),
		(float)eMuleOut/1024, (float)(eMuleOutOverall-eMuleOut)/1024,
		(float)eMuleIn/1024, (float)(eMuleInOverall-eMuleIn)/1024);
	sStatusBar.Replace(_T("[Panel2]"), HTTPHeader);
	//Xman end
	return sStatusBar;
}

CString CWebServer::_GetStatus(ThreadData&Data, long lSession)
{
	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString sSession; sSession.Format(_T("%ld"), lSession);

	bool bAdmin = IsSessionAdmin(Data,sSession);
	CString sStatus, sTemp, otherdata;
	sTemp.Append(_GetStatusBar(Data, bAdmin));
	sStatus.AppendFormat(_T("{html:{statusbar:%i"), sTemp.GetLength());
	CString sWnd = _ParseURL(Data.sURL, _T("wnd"));
	if(sWnd == _T("transfer")){
		size_t cat = 0;
		sint_ptr catfilter = 0;
		_GetLastUserCat(Data,lSession, cat, catfilter);
		sTemp.Append(_GetDownloadList(Data, cat, catfilter, bAdmin));
		sStatus.AppendFormat(_T(",down:%i"), sTemp.GetLength());
		if(pThis->m_Params.bShowUpload){
			sTemp.Append(_GetUploadList(Data));
			sStatus.AppendFormat(_T(",up:%i"), sTemp.GetLength());
		}
		if(pThis->m_Params.bShowUploadQueue){
			sTemp.Append(_GetQueueList(Data));
			sStatus.AppendFormat(_T(",queue:%i"), sTemp.GetLength());
		}
		otherdata.AppendFormat(_T(",queuecount:%i"), theApp.uploadqueue->waitinglist.GetCount());
	}
	else if(sWnd == _T("shared")){
		CString sEmpty;
		sTemp.Append(_GetSharedFilesList(Data, sEmpty));
		sStatus.AppendFormat(_T(",shared:%i"), sTemp.GetLength());
	}
	else if(sWnd == _T("server")){
		sTemp.Append(_GetServerList(Data));
		sStatus.AppendFormat(_T(",server:%i"), sTemp.GetLength());
	}
	sStatus.AppendChar(_T('}'));
	sStatus.Append(otherdata);
	sStatus.Append(_T("};"));
	sStatus += sTemp;
	return sStatus;
}

CString CWebServer::_GetServerWnd(ThreadData&Data)
{
	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString sSession = _ParseURL(Data.sURL, _T("ses"));
	CString sCmd = _ParseURL(Data.sURL, _T("c"));
	if(!sCmd.IsEmpty()){
		if(IsSessionAdmin(Data,sSession)){
			CString sIP = _ParseURL(Data.sURL, _T("ip"));
			int nPort = _tstoi(_ParseURL(Data.sURL, _T("port")));
			if (sCmd == _T("connect")){
				if(sIP.IsEmpty())
					SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION,WEBGUIIA_CONNECTTOSERVER,0);
				else
					_ConnectToServer(sIP, nPort);
				return _T("");
			}
			if (sCmd == _T("disconnect")){
				if (theApp.serverconnect->IsConnecting())
					SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION, WEBGUIIA_STOPCONNECTING,NULL);
				else
					SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION,WEBGUIIA_DISCONNECT,1);
				return _T("");
			}
			if (sCmd == _T("remove")){
				if(!sIP.IsEmpty()){
					CServer* server=theApp.serverlist->GetServerByAddress(sIP, (uint16)nPort);
					if (server!=NULL) 
						SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION, WEBGUIIA_SERVER_REMOVE, (LPARAM)server);
				}
			}
			else if (sCmd == _T("addtostatic")){
				if(!sIP.IsEmpty())
					_AddToStatic(sIP, nPort);
			}
			else if (sCmd == _T("removefromstatic")){
				if(!sIP.IsEmpty()){
					CServer* server=theApp.serverlist->GetServerByAddress(sIP, (uint16)nPort);
					if (server!=NULL) 
						SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION,WEBGUIIA_REMOVE_FROM_STATIC, (LPARAM)server);
				}
			}
			else if (sCmd == _T("priolow")){
				if(!sIP.IsEmpty())
				{
					CServer* server=theApp.serverlist->GetServerByAddress(sIP, (uint16)nPort);
					if (server) {
						server->SetPreference(SRV_PR_LOW);
						SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION,WEBGUIIA_UPDATESERVER,(LPARAM)server);
					}
				}
			}
			else if (sCmd == _T("prionormal")){
				if(!sIP.IsEmpty())
				{
					CServer* server=theApp.serverlist->GetServerByAddress(sIP, (uint16)nPort);
					if (server) {
						server->SetPreference(SRV_PR_NORMAL);
						SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION,WEBGUIIA_UPDATESERVER,(LPARAM)server);
					}
				}
			}
			else if (sCmd == _T("priohigh"))
			{
				if(!sIP.IsEmpty())
				{
					CServer* server=theApp.serverlist->GetServerByAddress(sIP, (uint16)nPort);
					if (server) {
						server->SetPreference(SRV_PR_HIGH);
						SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION,WEBGUIIA_UPDATESERVER,(LPARAM)server);
					}
				}
			}
			else if (sCmd == _T("addserver"))
			{
				CString strServerAddress = _ParseURL(Data.sURL, _T("serveraddr")).Trim();
				CString strServerPort = _ParseURL(Data.sURL, _T("serverport")).Trim();
				if (!strServerAddress.IsEmpty() && !strServerPort.IsEmpty())
				{
					CString strServerName = _ParseURL(Data.sURL, _T("servername")).Trim();
					if (strServerName.IsEmpty())
						strServerName = strServerAddress;
					CServer* nsrv = new CServer((uint16)_tstoi(strServerPort), strServerAddress);
					nsrv->SetListName(strServerName);
					if (!theApp.emuledlg->serverwnd->serverlistctrl.AddServer(nsrv,true))
						delete nsrv;
					else{
						CString sPriority(_ParseURL(Data.sURL, _T("priority")));// X: [CI] - [Code Improvement]
						if (sPriority == _T("low"))
							nsrv->SetPreference(PR_LOW);
						else if (sPriority == _T("normal"))
							nsrv->SetPreference(PR_NORMAL);
						else if (sPriority == _T("high"))
							nsrv->SetPreference(PR_HIGH);

						SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION,WEBGUIIA_UPDATESERVER,(LPARAM)nsrv);

						if(_ParseURL(Data.sURL, _T("addtostatic")) == _T("true"))
						{
							_AddToStatic(_ParseURL(Data.sURL, _T("serveraddr")),_tstoi(_ParseURL(Data.sURL, _T("serverport"))));
						}
						if(_ParseURL(Data.sURL, _T("connectnow")) == _T("true"))
							_ConnectToServer(_ParseURL(Data.sURL, _T("serveraddr")),_tstoi(_ParseURL(Data.sURL, _T("serverport"))));
					}
				}
			}
			else if (sCmd == _T("updateservermetfromurl"))
			{
				CString url=_ParseURL(Data.sURL, _T("servermeturl"));
				const TCHAR* urlbuf=url;
				SendMessage(theApp.emuledlg->m_hWnd, WEB_GUI_INTERACTION, WEBGUIIA_UPDATESERVERMETFROMURL, (LPARAM)urlbuf);
			}
		}
	}
	if (sCmd == _T("menu")){
		int iMenu = _tstol( _ParseURL(Data.sURL, _T("m")) );
		if(iMenu<_countof(WSserverColumnHidden)){
			WSserverColumnHidden[iMenu] = !WSserverColumnHidden[iMenu];
			SaveWIConfigArray(WSserverColumnHidden, _countof(WSserverColumnHidden), _T("serverColumnHidden"));
		}
		return _T("");
	}
	sCmd = _ParseURL(Data.sURL, _T("l"));
	if (sCmd == _T("sort"))
	{
		CString sSortReverse = _ParseURL(Data.sURL, _T("sortAsc"));
		CString sSort = _ParseURL(Data.sURL, _T("sort"));
		if (!sSort.IsEmpty()){
			pThis->m_Params.ServerSort = (ServerSort)_tstol(sSort);

			if (sSortReverse.IsEmpty())
				pThis->m_Params.bServerSortReverse = (pThis->m_Params.ServerSort == SERVER_SORT_NAME||pThis->m_Params.ServerSort == SERVER_SORT_DESCRIPTION);
		}
		if (!sSortReverse.IsEmpty())
			pThis->m_Params.bServerSortReverse = _tstol(sSortReverse)!=0;
		return _GetServerList(Data);
	}

	CString Out = pThis->m_Templates.sServerWnd;

	// ListText
	Out.Replace(_T("[ServerList]"), _GetPlainResString(IDS_SV_SERVERLIST));
	CString strTemp;
	for(uint_ptr i = 0, added = 0;i < _countof(WSserverColumnHidden);++i){
		if(!WSserverColumnHidden[i])
				continue;
		if(added>0)
			strTemp.AppendChar(_T(','));
		strTemp.AppendFormat(_T("%i"), i);
		++added;
	}
	Out.Replace(_T("[ColumnHidden]"), strTemp);
	strTemp.Format(_T("%i"), pThis->m_Params.ServerSort);
	Out.Replace(_T("[SortColumn]"), strTemp);

	// Column Header
	Out.Replace(_T("[Servername]"),		_GetPlainResString(IDS_SL_SERVERNAME));
	Out.Replace(_T("[Address]"),		_GetPlainResString(IDS_IP));
	Out.Replace(_T("[Description]"),	_GetPlainResString(IDS_DESCRIPTION));
	Out.Replace(_T("[Ping]"),			_GetPlainResString(IDS_PING));
	Out.Replace(_T("[Users]"),			_GetPlainResString(IDS_UUSERS));
	Out.Replace(_T("[Files]"),			_GetPlainResString(IDS_FILES));
	Out.Replace(_T("[Priority]"),		_GetPlainResString(IDS_PRIORITY));
	Out.Replace(_T("[Failed]"),			_GetPlainResString(IDS_UFAILED));
	Out.Replace(_T("[Limit]"),			_GetPlainResString(IDS_SERVER_LIMITS));


	if(IsSessionAdmin(Data,sSession)){
		CString sAddServerBox = pThis->m_Templates.sAddServerBox;

		sAddServerBox.Replace(_T("[AddServer]"), _GetPlainResString(IDS_SV_NEWSERVER));
		sAddServerBox.Replace(_T("[IP]"), _GetPlainResString(IDS_SV_ADDRESS));
		sAddServerBox.Replace(_T("[Port]"), _GetPlainResString(IDS_PORT));
		sAddServerBox.Replace(_T("[Name]"), _GetPlainResString(IDS_SW_NAME));
		sAddServerBox.Replace(_T("[Static]"), _GetPlainResString(IDS_STATICSERVER));
		sAddServerBox.Replace(_T("[ConnectNow]"), _GetPlainResString(IDS_IRC_CONNECT));
		sAddServerBox.Replace(_T("[Priority]"), _GetPlainResString(IDS_PRIORITY));
		sAddServerBox.Replace(_T("[Low]"), _GetPlainResString(IDS_PRIOLOW));
		sAddServerBox.Replace(_T("[Normal]"), _GetPlainResString(IDS_PRIONORMAL));
		sAddServerBox.Replace(_T("[High]"), _GetPlainResString(IDS_PRIOHIGH));
		sAddServerBox.Replace(_T("[Add]"), _GetPlainResString(IDS_SV_ADD));
		sAddServerBox.Replace(_T("[UpdateServerMetFromURL]"), _GetPlainResString(IDS_SV_MET));
		sAddServerBox.Replace(_T("[URL]"), _GetPlainResString(IDS_SV_URL));
		sAddServerBox.Replace(_T("[Apply]"), _GetPlainResString(IDS_PW_APPLY));
		sAddServerBox.Replace(_T("[Execute]"), _GetPlainResString(IDS_IRC_PERFORM));
		Out.Replace(_T("[AddServerBox]"), sAddServerBox);
	}
	else
		Out.Replace(_T("[AddServerBox]"), _T(""));

	// List Content
	Out.Replace(_T("[ServersList]"), _GetServerList(Data));
	return Out;
}
CString CWebServer::_GetServerList(ThreadData&Data)
{
	CWebServer *pThis = (CWebServer *)Data.pThis;
	//if(pThis == NULL)
		//return _T("");

	CAtlArray<ServerEntry> ServerArray;

	CServer		*tempServer;
	// Populating array
	for (size_t sc=0;sc<theApp.serverlist->GetServerCount();sc++)
	{
		CServer* cur_file = theApp.serverlist->GetServerAt(sc);
		ServerEntry Entry;
		Entry.sServerName = _SpecialChars(cur_file->GetListName());
		Entry.sServerIP = cur_file->GetAddress();
		Entry.nServerPort = cur_file->GetPort();
		Entry.sServerDescription = _SpecialChars(cur_file->GetDescription());
		Entry.nServerPing = cur_file->GetPing();
		Entry.nServerUsers = cur_file->GetUsers();
		Entry.nServerMaxUsers = cur_file->GetMaxUsers();
		Entry.nServerFiles = cur_file->GetFiles();
		Entry.bServerStatic = cur_file->IsStaticMember();
		if(cur_file->GetPreference()== SRV_PR_HIGH)
		{
			Entry.sServerPriority = _GetPlainResString(IDS_PRIOHIGH);
			Entry.nServerPriority = 2;
		}
		else if(cur_file->GetPreference()== SRV_PR_NORMAL)
		{
			Entry.sServerPriority = _GetPlainResString(IDS_PRIONORMAL);
			Entry.nServerPriority = 1;
		}
		else if(cur_file->GetPreference()== SRV_PR_LOW)
		{
			Entry.sServerPriority = _GetPlainResString(IDS_PRIOLOW);
			Entry.nServerPriority = 0;
		}
		Entry.nServerFailed = cur_file->GetFailedCount();
		Entry.nServerSoftLimit = cur_file->GetSoftFiles();
		Entry.nServerHardLimit = cur_file->GetHardFiles();
		Entry.sServerVersion = cur_file->GetVersion();
		if (inet_addr(CStringA(Entry.sServerIP)) != INADDR_NONE)
		{
			int counter=0;
			CString temp,newip;
			for(int j=0; j<4; j++)
			{
				temp = Entry.sServerIP.Tokenize(_T("."),counter);
				if (temp.GetLength() == 1)
					newip.Append(_T("00") + temp);// X: [CI] - [Code Improvement]
				else if (temp.GetLength() == 2)
					newip.Append(_T('0') + temp);
				else if (temp.GetLength() == 3)
					newip.Append(temp);
			}
			Entry.sServerFullIP = newip;
		}
		else
			Entry.sServerFullIP = Entry.sServerIP;

		Entry.sServerState = (cur_file->GetFailedCount() > 0)?_T("failed"):_T("disconnected");
		if (theApp.serverconnect->IsConnecting())
		{
			Entry.sServerState = _T("connecting");
		}
		else if (theApp.serverconnect->IsConnected())
		{
			tempServer = theApp.serverconnect->GetCurrentServer();
			if (tempServer->GetAddress() == cur_file->GetAddress())
				Entry.sServerState = (!theApp.serverconnect->IsLowID())?_T("high"):_T("low");
		}
		ServerArray.Add(Entry);
	}
	// Sorting (simple bubble sort, we don't have tons of data here)
	bool bSorted = true;
	for(size_t nMax = 0;bSorted && nMax < ServerArray.GetCount()*2; nMax++)
	{
		bSorted = false;
		for(size_t i = 0; i < ServerArray.GetCount() - 1; i++)
		{
			bool bSwap = false;
			switch(pThis->m_Params.ServerSort)
			{
			case SERVER_SORT_STATE:
				bSwap = ServerArray[i].sServerState.CompareNoCase(ServerArray[i+1].sServerState) > 0;
				break;
			case SERVER_SORT_NAME:
				bSwap = ServerArray[i].sServerName.CompareNoCase(ServerArray[i+1].sServerName) < 0;
				break;
			case SERVER_SORT_IP:
				bSwap = ServerArray[i].sServerFullIP < ServerArray[i+1].sServerFullIP;
				break;
			case SERVER_SORT_DESCRIPTION:
				bSwap = ServerArray[i].sServerDescription.CompareNoCase(ServerArray[i+1].sServerDescription) < 0;
				break;
			case SERVER_SORT_PING:
				bSwap = ServerArray[i].nServerPing < ServerArray[i+1].nServerPing;
				break;
			case SERVER_SORT_USERS:
				bSwap = ServerArray[i].nServerUsers < ServerArray[i+1].nServerUsers;
				break;
			case SERVER_SORT_FILES:
				bSwap = ServerArray[i].nServerFiles < ServerArray[i+1].nServerFiles;
				break;
			case SERVER_SORT_PRIORITY:
				bSwap = ServerArray[i].nServerPriority < ServerArray[i+1].nServerPriority;
				break;
			case SERVER_SORT_FAILED:
				bSwap = ServerArray[i].nServerFailed < ServerArray[i+1].nServerFailed;
				break;
			case SERVER_SORT_LIMIT:
				bSwap = ServerArray[i].nServerSoftLimit < ServerArray[i+1].nServerSoftLimit;
				break;
			case SERVER_SORT_VERSION:
				bSwap = ServerArray[i].sServerVersion < ServerArray[i+1].sServerVersion;
				break;
			}
			if(pThis->m_Params.bServerSortReverse)
				bSwap = !bSwap;
			if(bSwap)
			{
				bSorted = true;
				ServerEntry TmpEntry = ServerArray[i];
				ServerArray[i] = ServerArray[i+1];
				ServerArray[i+1] = TmpEntry;
			}
		}
	}

	// Displaying
	const TCHAR	*pcTmp, *pcTmp2;
	CString sList, HTTPProcessData, sServerPort, ed2k;
	CString OutE = pThis->m_Templates.sServerLine;

	CString sPort = _ParseURL(Data.sURL, _T("port"));// X: [CI] - [Code Improvement]
	for(size_t i = 0; i < ServerArray.GetCount(); i++)
	{
		HTTPProcessData = OutE;	// Copy Entry Line to Temp

		sServerPort.Format(_T("%u"), ServerArray[i].nServerPort);
		ed2k.Format(_T("ed2k://|server|%s|%s|/"), ServerArray[i].sServerIP, sServerPort);

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
		if (!WSserverColumnHidden[0])
		{
			if(ServerArray[i].sServerName.GetLength() > SHORT_LENGTH)
				HTTPProcessData.Replace(_T("[Servername]"), _T("<span title=\"") + ServerArray[i].sServerName + _T("\">") + ServerArray[i].sServerName.Left(SHORT_LENGTH-3) + _T("...") + _T("</span>"));
			else
				HTTPProcessData.Replace(_T("[Servername]"), ServerArray[i].sServerName);
		}
		else
			HTTPProcessData.Replace(_T("[Servername]"), _T(""));
		if (!WSserverColumnHidden[1])
		{
			CString sPort; sPort.Format(_T(":%d"), ServerArray[i].nServerPort);
			HTTPProcessData.Replace(_T("[Address]"), ServerArray[i].sServerIP + sPort);
		}
		else
			HTTPProcessData.Replace(_T("[Address]"), _T(""));
		if (!WSserverColumnHidden[2])
		{
			if(ServerArray[i].sServerDescription.GetLength() > SHORT_LENGTH)
				HTTPProcessData.Replace(_T("[Description]"), _T("<span title=\"") + ServerArray[i].sServerDescription + _T("\">") + ServerArray[i].sServerDescription.Left(SHORT_LENGTH-3) + _T("...") + _T("</span>"));
			else
				HTTPProcessData.Replace(_T("[Description]"), ServerArray[i].sServerDescription);
		}
		else
			HTTPProcessData.Replace(_T("[Description]"), _T(""));
		if (!WSserverColumnHidden[3])
		{
			CString sPing; sPing.Format(_T("%d"), ServerArray[i].nServerPing);
			HTTPProcessData.Replace(_T("[Ping]"), sPing);
		}
		else
			HTTPProcessData.Replace(_T("[Ping]"), _T(""));
		CString sT;
		if (!WSserverColumnHidden[4])
		{
			if(ServerArray[i].nServerUsers > 0)
			{
				if(ServerArray[i].nServerMaxUsers > 0)
					sT.Format(_T("%s (%s)"), CastItoIShort(ServerArray[i].nServerUsers), CastItoIShort(ServerArray[i].nServerMaxUsers));
				else
					sT = CastItoIShort(ServerArray[i].nServerUsers);
			}
			HTTPProcessData.Replace(_T("[Users]"), sT);
		}
		else
			HTTPProcessData.Replace(_T("[Users]"), _T(""));
		if (!WSserverColumnHidden[5] && (ServerArray[i].nServerFiles > 0))
		{
			HTTPProcessData.Replace(_T("[Files]"), CastItoIShort(ServerArray[i].nServerFiles));
		}
		else
			HTTPProcessData.Replace(_T("[Files]"), _T(""));
		if (!WSserverColumnHidden[6])
			HTTPProcessData.Replace(_T("[Priority]"), ServerArray[i].sServerPriority);
		else
			HTTPProcessData.Replace(_T("[Priority]"), _T(""));
		if (!WSserverColumnHidden[7])
		{
			CString sFailed; sFailed.Format(_T("%d"), ServerArray[i].nServerFailed);
			HTTPProcessData.Replace(_T("[Failed]"), sFailed);
		}
		else
			HTTPProcessData.Replace(_T("[Failed]"), _T(""));
		if (!WSserverColumnHidden[8])
		{
			CString		strTemp;

			strTemp.Format( _T("%s (%s)"), CastItoIShort(ServerArray[i].nServerSoftLimit),
				CastItoIShort(ServerArray[i].nServerHardLimit) );
			HTTPProcessData.Replace(_T("[Limit]"), strTemp);
		}
		else
			HTTPProcessData.Replace(_T("[Limit]"), _T(""));
		if (!WSserverColumnHidden[9])
		{
			if(ServerArray[i].sServerVersion.GetLength() > SHORT_LENGTH_MIN)
				HTTPProcessData.Replace(_T("[Version]"), _T("<span title=\"") + ServerArray[i].sServerVersion + _T("\">") + ServerArray[i].sServerVersion.Left(SHORT_LENGTH-3) + _T("...") + _T("</span>"));
			else
				HTTPProcessData.Replace(_T("[Version]"), ServerArray[i].sServerVersion);
		}
		else
			HTTPProcessData.Replace(_T("[Version]"), _T(""));
		HTTPProcessData.Replace(_T("[ServerState]"), ServerArray[i].sServerState);
		sList += HTTPProcessData;
	}
	CString Out = pThis->m_Templates.sServerListFooter;
	Out.Replace(_T("[SortReverse]"), pThis->m_Params.bServerSortReverse?_T("1"):_T("0"));
	sList += Out;
	return sList;
}

CString CWebServer::_GetTransferWnd(ThreadData&Data)
{
	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString HTTPTemp = _ParseURL(Data.sURL, _T("showup"));
	if (HTTPTemp == _T("true"))
		pThis->m_Params.bShowUpload = true;
	else if (HTTPTemp == _T("false")){
		pThis->m_Params.bShowUpload = false;
		return _T("");
	}
	HTTPTemp = _ParseURL(Data.sURL, _T("showqueue"));
	if (HTTPTemp == _T("true"))
		pThis->m_Params.bShowUploadQueue = true;
	else if (HTTPTemp == _T("false")){
		pThis->m_Params.bShowUploadQueue = false;
		return _T("");
	}
	CString sSession = _ParseURL(Data.sURL, _T("ses"));
	long lSession = _tstol(sSession);// X: [CI] - [Code Improvement]
	bool bAdmin = IsSessionAdmin(Data,sSession);

	// cat
	size_t cat = 0;
	sint_ptr catfilter = 0;
	CString catp=_ParseURL(Data.sURL,_T("cat"));
	if (catp.IsEmpty())
		_GetLastUserCat(Data, lSession, cat, catfilter);
	else
	{
		cat=_tstoi(catp);
		_SetLastUserCat(Data, lSession, cat);
		if(cat<0)
			_GetLastUserCat(Data, lSession, cat, catfilter);
	}
	CString sCmd = _ParseURL(Data.sURL, _T("c"));
	if(bAdmin){
		// commands
		HTTPTemp = _ParseURL(Data.sURL, _T("ed2k"));
		if (!HTTPTemp.IsEmpty()){
			theApp.emuledlg->SendMessage(WEB_ADDDOWNLOADS, (WPARAM)(LPCTSTR)HTTPTemp, cat);
			HTTPTemp = _ParseURL(Data.sURL, _T("l"));
			if (HTTPTemp != _T("sort"))
				return _T("");
		}
		else if (sCmd == _T("clearcompleted"))
			theApp.emuledlg->SendMessage(WEB_CLEAR_COMPLETED, (WPARAM)0, (LPARAM)cat );
		else if (sCmd == _T("menuprio")){
			CString sPrio(_ParseURL(Data.sURL, _T("p")));
			int prio = PR_NORMAL;

			if(sPrio == _T("low"))
				prio = PR_LOW;
			else if(sPrio == _T("normal"))
				prio = PR_NORMAL;
			else if(sPrio == _T("high"))
				prio = PR_HIGH;
			else if(sPrio == _T("auto"))
				prio = PR_AUTO;

			SendMessage(theApp.emuledlg->m_hWnd,WEB_CATPRIO, (WPARAM)cat,(LPARAM)prio);
			return _T("");
		}

		CString sOp(_ParseURL(Data.sURL, _T("op")));
		if (!sOp.IsEmpty())
		{
			CString sFile(_ParseURL(Data.sURL, _T("file")));
			uchar FileHash[16], UserHash[16];

			if (!sFile.IsEmpty())
			{
				CPartFile *found_file = theApp.downloadqueue->GetFileByID(_GetFileHash(sFile, FileHash));
				if(found_file)
				{	// SyruS all actions require a found file (removed double-check inside)
					if (sOp == _T("stop"))
						found_file->StopFile();
					else if (sOp == _T("pause"))
						found_file->PauseFile();
					else if (sOp == _T("resume"))
						found_file->ResumeFile();
					else if (sOp == _T("cancel"))
					{
						found_file->DeleteFile();
						SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION,WEBGUIIA_UPD_CATTABS,0);
					}
					else if (sOp == _T("getflc"))
						found_file->SetPreviewPrio(!found_file->GetPreviewPrio());
					else if (sOp == _T("rename"))
					{
						CString sNewName(_ParseURL(Data.sURL, _T("name")));
						theApp.emuledlg->SendMessage(WEB_FILE_RENAME, (WPARAM)found_file, (LPARAM)(LPCTSTR)sNewName);
					}
					else if (sOp == _T("priolow"))
					{
						found_file->SetAutoDownPriority(false);
						found_file->SetDownPriority(PR_LOW);
					}
					else if (sOp == _T("prionormal"))
					{
						found_file->SetAutoDownPriority(false);
						found_file->SetDownPriority(PR_NORMAL);
					}
					else if (sOp == _T("priohigh"))
					{
						found_file->SetAutoDownPriority(false);
						found_file->SetDownPriority(PR_HIGH);
					}
					else if (sOp == _T("prioauto"))
					{
						found_file->SetAutoDownPriority(true);
						found_file->SetDownPriority(PR_HIGH);
					}
					else if (sOp == _T("setcat"))
					{
						CString newcat=_ParseURL(Data.sURL, _T("filecat"));
						if (!newcat.IsEmpty())
							found_file->SetCategory(_tstol(newcat));
					}
				}
			}
			else
			{
				CString sUser(_ParseURL(Data.sURL, _T("userhash")));

				if (_GetFileHash(sUser, UserHash) != 0)
				{
					CUpDownClient* cur_client = theApp.clientlist->FindClientByUserHash(UserHash);
					if(cur_client)
					{
						if (sOp == _T("addfriend"))
							SendMessage(theApp.emuledlg->m_hWnd,WEB_ADDREMOVEFRIEND,(WPARAM)cur_client,(LPARAM)1);
						else if (sOp == _T("removefriend")) {
							CFriend* f=theApp.friendlist->SearchFriend(UserHash,0,0);
							if (f)
								SendMessage(theApp.emuledlg->m_hWnd,WEB_ADDREMOVEFRIEND,(WPARAM)f,(LPARAM)0);
						}
					}
				}
			}
		}
	}
	if (sCmd == _T("menu")){
		CString sList = _ParseURL(Data.sURL, _T("list"));
		int iMenu = _tstol( _ParseURL(Data.sURL, _T("m")) );
		if (sList == _T("down")){
			if(iMenu<_countof(WSserverColumnHidden)){
				WSdownloadColumnHidden[iMenu] = !WSdownloadColumnHidden[iMenu];
				SaveWIConfigArray(WSdownloadColumnHidden, _countof(WSdownloadColumnHidden), _T("downloadColumnHidden"));
			}
			return _T("");
		}
		if (sList == _T("up")){
			if(iMenu<_countof(WSuploadColumnHidden)){
				WSuploadColumnHidden[iMenu] = !WSuploadColumnHidden[iMenu];
				SaveWIConfigArray(WSuploadColumnHidden, _countof(WSuploadColumnHidden), _T("uploadColumnHidden"));
			}
			return _T("");
		}
		if (sList == _T("queue")){
			if(iMenu<_countof(WSqueueColumnHidden)){
				WSqueueColumnHidden[iMenu] = !WSqueueColumnHidden[iMenu];
				SaveWIConfigArray(WSqueueColumnHidden, _countof(WSqueueColumnHidden), _T("queueColumnHidden"));
			}
			//return _T("");
		}
		return _T("");
	}
	sCmd = _ParseURL(Data.sURL, _T("l"));
	if (sCmd == _T("sort")){
		CString sList = _ParseURL(Data.sURL, _T("list"));
		CString sSortReverse = _ParseURL(Data.sURL, _T("sortAsc"));
		CString sSort = _ParseURL(Data.sURL, _T("sort"));
		if (sList == _T("down")){
			if (!sSort.IsEmpty()){
				pThis->m_Params.DownloadSort = (DownloadSort)_tstol(sSort);
				if (sSortReverse.IsEmpty())
					pThis->m_Params.bDownloadSortReverse = (pThis->m_Params.DownloadSort == DOWN_SORT_NAME||pThis->m_Params.DownloadSort == DOWN_SORT_CATEGORY);
			}
			if (!sSortReverse.IsEmpty())
				pThis->m_Params.bDownloadSortReverse = _tstol(sSortReverse)!=0;
			return _GetDownloadList(Data, cat, catfilter, bAdmin);
		}
		if (sList == _T("up")){
			if (!sSort.IsEmpty()){
				pThis->m_Params.UploadSort = (UploadSort)_tstol(sSort);
				if (sSortReverse.IsEmpty())
					pThis->m_Params.bUploadSortReverse = (pThis->m_Params.UploadSort == UP_SORT_USER||pThis->m_Params.UploadSort == UP_SORT_FILENAME);
			}
			if (!sSortReverse.IsEmpty())
				pThis->m_Params.bUploadSortReverse = _tstol(sSortReverse)!=0;
			return _GetUploadList(Data);
		}
		if (sList == _T("queue")){
			if (!sSort.IsEmpty()){
				pThis->m_Params.QueueSort = (QueueSort)_tstol(sSort);
				if (sSortReverse.IsEmpty())
					pThis->m_Params.bQueueSortReverse = (pThis->m_Params.QueueSort == QU_SORT_USER||pThis->m_Params.QueueSort == QU_SORT_FILENAME);
			}
			if (!sSortReverse.IsEmpty())
				pThis->m_Params.bQueueSortReverse = _tstol(sSortReverse)!=0;
			return _GetQueueList(Data);
		}
		return _T("");
	}
	CString Out = pThis->m_Templates.sTransferWnd;
	// ListText
	Out.Replace(_T("[DownloadList]"), _GetPlainResString(IDS_TW_DOWNLOADS));
	Out.Replace(_T("[UploadList]"), _GetPlainResString(IDS_TW_UPLOADS));
	Out.Replace(_T("[UploadQueueList]"), _GetPlainResString(IDS_ONQUEUE));
	HTTPTemp.Format(_T("%i"), theApp.uploadqueue->waitinglist.GetCount());
	Out.Replace(_T("[CounterQueue]"),	HTTPTemp);
	HTTPTemp.Format(_T("%i"), pThis->m_Params.DownloadSort);
	Out.Replace(_T("[DSortColumn]"), HTTPTemp);
	HTTPTemp.Empty();
	for(uint_ptr i = 0, added = 0;i < _countof(WSdownloadColumnHidden);++i){
		if(!WSdownloadColumnHidden[i])
			continue;
		if(added>0)
			HTTPTemp.AppendChar(_T(','));
		HTTPTemp.AppendFormat(_T("%i"), i);
		++added;
	}
	Out.Replace(_T("[DColumnHidden]"), HTTPTemp);
	HTTPTemp.Format(_T("%i"), pThis->m_Params.UploadSort);
	Out.Replace(_T("[USortColumn]"), HTTPTemp);
	HTTPTemp.Empty();
	for(uint_ptr i = 0, added = 0;i < _countof(WSuploadColumnHidden);++i){
		if(!WSuploadColumnHidden[i])
			continue;
		if(added>0)
			HTTPTemp.AppendChar(_T(','));
		HTTPTemp.AppendFormat(_T("%i"), i);
		++added;
	}
	Out.Replace(_T("[UColumnHidden]"), HTTPTemp);
	HTTPTemp.Format(_T("%i"), pThis->m_Params.QueueSort);
	Out.Replace(_T("[QSortColumn]"), HTTPTemp);
	HTTPTemp.Empty();
	for(uint_ptr i = 0, added = 0;i < _countof(WSqueueColumnHidden);++i){
		if(!WSqueueColumnHidden[i])
			continue;
		if(added>0)
			HTTPTemp.AppendChar(_T(','));
		HTTPTemp.AppendFormat(_T("%i"), i);
		++added;
	}
	Out.Replace(_T("[QColumnHidden]"), HTTPTemp);

	// Column Header
	HTTPTemp.Format(_T("%i"),pThis->m_Templates.iProgressbarWidth);
	Out.Replace(_T("[PROGRESSBARWIDTHVAL]"),HTTPTemp);
	Out.Replace(_T("[Filename]"),		_GetPlainResString(IDS_DL_FILENAME));
	Out.Replace(_T("[Size]"),			_GetPlainResString(IDS_DL_SIZE));
	Out.Replace(_T("[Transferred]"),	_GetPlainResString(IDS_DL_TRANSFCOMPL));
	Out.Replace(_T("[Progress]"),		_GetPlainResString(IDS_DL_PROGRESS));
	Out.Replace(_T("[Speed]"),			_GetPlainResString(IDS_DL_SPEED));
	Out.Replace(_T("[Sources]"),		_GetPlainResString(IDS_DL_SOURCES));
	Out.Replace(_T("[Priority]"),		_GetPlainResString(IDS_PRIORITY));
	Out.Replace(_T("[Category]"),		_GetPlainResString(IDS_CAT));
	Out.Replace(_T("[UserName]"),		_GetPlainResString(IDS_QL_USERNAME));
	Out.Replace(_T("[RATIO]"),			_GetPlainResString(IDS_STATS_SRATIO));
	Out.Replace(_T("[Version]"),		_GetPlainResString(IDS_CD_CSOFT));
	Out.Replace(_T("[Score]"),			_GetPlainResString(IDS_SCORE));
	HTTPTemp.Format(_T("-%i"),	thePrefs.GetCatFilter(0));
	Out.Replace(_T("[Filter]"),			HTTPTemp);


	HTTPTemp.Empty();
	CString	strCategory;
	for (size_t i = 1;i < 16;i++)
	{
		if(i != 1)
			HTTPTemp += _T(',');
		strCategory = GetSubCatLabel(0-i);
		HTTPTemp.AppendFormat(_T("'%s'"), strCategory);
	}
	Out.Replace(_T("[CatFilter]"), HTTPTemp);

	HTTPTemp.Empty();
	for (size_t i = 0; i <thePrefs.GetCatCount(); i++)
	{
		if(i != 0)
			HTTPTemp += _T(',');
		strCategory = thePrefs.GetCategory(i)->strTitle;
		strCategory.Replace(_T("'"),_T("\\'"));

		HTTPTemp.AppendFormat(_T("'%s'"), strCategory);
	}
	Out.Replace(_T("[Cats]"), HTTPTemp);

	// List Content
	Out.Replace(_T("[QueueList]"), pThis->m_Params.bShowUploadQueue?_GetQueueList(Data):_T(""));
	Out.Replace(_T("[UploadFilesList]"), pThis->m_Params.bShowUpload?_GetUploadList(Data):_T(""));
	Out.Replace(_T("[DownloadFilesList]"), _GetDownloadList(Data, cat, catfilter, bAdmin));

	return Out;
}

CString CWebServer::_GetDownloadList(ThreadData&Data, size_t cat, sint_ptr catfilter, bool bAdmin)
{
	CWebServer *pThis = (CWebServer *)Data.pThis;
	//if(pThis == NULL)
		//return _T("");
	CAtlArray<DownloadFiles> FilesArray;
	CAtlArray<CPartFile*> partlist;

	theApp.emuledlg->transferwnd->GetDownloadList()->GetDisplayedFiles(&partlist);

	// Populating array
	for (size_t i = 0;i < partlist.GetCount();i++) {
		
		CPartFile* pPartFile=partlist.GetAt(i);

		if (pPartFile)
		{
			if (cat>0 && pPartFile->GetCategory() != cat)
				continue;
			if (catfilter<0) {
				switch (catfilter) {
					case -1 : if (pPartFile->GetCategory()!=0) continue; break;
					case -2 : if (!pPartFile->IsPartFile()) continue; break;
					case -3 : if (pPartFile->IsPartFile()) continue; break;
					case -4 : if (!((pPartFile->GetStatus()==PS_READY|| pPartFile->GetStatus()==PS_EMPTY) && pPartFile->GetTransferringSrcCount()==0)) continue; break;
					case -5 : if (!((pPartFile->GetStatus()==PS_READY|| pPartFile->GetStatus()==PS_EMPTY) && pPartFile->GetTransferringSrcCount()>0)) continue; break;
					case -6 : if (pPartFile->GetStatus()!=PS_ERROR) continue; break;
					case -7 : if (pPartFile->GetStatus()!=PS_PAUSED && !pPartFile->IsStopped()) continue; break;
					case -8 : if (pPartFile->lastseencomplete==NULL) continue; break;
					case -9 : if (!pPartFile->IsMovie()) continue; break;
					case -10 : if (ED2KFT_AUDIO != GetED2KFileTypeID(pPartFile->GetFileName())) continue; break;
					case -11 : if (!pPartFile->IsArchive()) continue; break;
					case -12 : if (ED2KFT_CDIMAGE != GetED2KFileTypeID(pPartFile->GetFileName())) continue; break;
					case -13 : if (ED2KFT_DOCUMENT != GetED2KFileTypeID(pPartFile->GetFileName())) continue; break;
					case -14 : if (ED2KFT_IMAGE != GetED2KFileTypeID(pPartFile->GetFileName())) continue; break;
					case -15 : if (ED2KFT_PROGRAM != GetED2KFileTypeID(pPartFile->GetFileName())) continue; break;
					//JOHNTODO: Not too sure here.. I was going to add Collections but noticed something strange.. Are these supposed to match the list in PartFile around line 5132? Because they do not..
				}
			}

			DownloadFiles dFile;
			dFile.sFileName = _SpecialChars(pPartFile->GetFileName());
			dFile.sFileType = GetWebImageNameForFileType(dFile.sFileName);
			dFile.sFileNameJS = _SpecialChars(pPartFile->GetFileName());	//for javascript
			dFile.m_qwFileSize = pPartFile->GetFileSize();
			dFile.m_qwFileTransferred = pPartFile->GetCompletedSize();
			dFile.m_dblCompleted = pPartFile->GetPercentCompleted();
			dFile.lFileSpeed = pPartFile->GetDownloadDatarate(); //Xman // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
			
			if ( pPartFile->HasComment() || pPartFile->HasRating()) {
				dFile.iComment= pPartFile->HasBadRating()?2:1;
			} else 
				dFile.iComment = 0;

			dFile.iFileState=pPartFile->getPartfileStatusRang();

			dFile.sFileState = (dFile.lFileSpeed > 0)?_T("downloading"):_T("waiting");//Xman // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-

			switch(pPartFile->GetStatus())
			{
				//case PS_HASHING:
					dFile.sFileState = _T("hashing");
					break;
				//case PS_WAITINGFORHASH:
					//dFile.sFileState = _T("waitinghash");
					//break;
				case PS_ERROR:
					dFile.sFileState = _T("error");
					break;
				case PS_COMPLETING:
					dFile.sFileState = _T("completing");
					break;
				case PS_COMPLETE:
					dFile.sFileState = _T("complete");
					break;
				case PS_PAUSED:
					if (!pPartFile->IsStopped())
						dFile.sFileState = _T("paused");
					else
						dFile.sFileState = _T("stopped");
					break;
				default:
					break;
			}

			dFile.bFileAutoPrio = pPartFile->IsAutoDownPriority();
			dFile.nFilePrio		= pPartFile->GetDownPriority();
			size_t		pCat = pPartFile->GetCategory();

			CString	strCategory = thePrefs.GetCategory(pCat)->strTitle;
			strCategory.Replace(_T("'"),_T("\'"));

			dFile.sCategory = strCategory;
		
			dFile.sFileHash = md4str(pPartFile->GetFileHash());
			dFile.lSourceCount = pPartFile->GetSourceCount();
			dFile.lNotCurrentSourceCount = pPartFile->GetNotCurrentSourcesCount();
			dFile.lTransferringSourceCount = pPartFile->GetTransferringSrcCount();
			dFile.bIsComplete = !pPartFile->IsPartFile();
			dFile.bIsPreview = pPartFile->IsReadyForPreview();
			dFile.bIsGetFLC =  pPartFile->GetPreviewPrio();
			dFile.iCatIndex =  pPartFile->GetCategory();

			if (theApp.GetPublicIP() != 0 && !theApp.IsFirewalled())
				dFile.sED2kLink = pPartFile->GetED2kLink(false, false, false, true, theApp.GetPublicIP());
			else
				dFile.sED2kLink = pPartFile->GetED2kLink();
			
			dFile.sFileInfo = _SpecialChars(pPartFile->GetInfoSummary(true),false);

			FilesArray.Add(dFile);
		}
	}


	// Sorting (simple bubble sort, we don't have tons of data here)
	bool bSorted = true;
	for(size_t nMax = 0;bSorted && nMax < FilesArray.GetCount()*2; nMax++)
	{
		bSorted = false;
		for(size_t i = 0; i < FilesArray.GetCount() - 1; i++)
		{
			bool bSwap = false;
			switch(pThis->m_Params.DownloadSort)
			{
			case DOWN_SORT_STATE:
				bSwap = FilesArray[i].iFileState<FilesArray[i+1].iFileState;
				break;
			case DOWN_SORT_TYPE:
				bSwap = FilesArray[i].sFileType.CompareNoCase(FilesArray[i+1].sFileType) < 0;
				break;
			case DOWN_SORT_NAME:
				bSwap = FilesArray[i].sFileName.CompareNoCase(FilesArray[i+1].sFileName) < 0;
				break;
			case DOWN_SORT_SIZE:
				bSwap = FilesArray[i].m_qwFileSize < FilesArray[i+1].m_qwFileSize;
				break;
			case DOWN_SORT_TRANSFERRED:
				bSwap = FilesArray[i].m_qwFileTransferred < FilesArray[i+1].m_qwFileTransferred;
				break;
			case DOWN_SORT_SPEED:
				bSwap = FilesArray[i].lFileSpeed < FilesArray[i+1].lFileSpeed;
				break;
			case DOWN_SORT_PROGRESS:
				bSwap = FilesArray[i].m_dblCompleted  < FilesArray[i+1].m_dblCompleted ;
				break;
			case DOWN_SORT_SOURCES:
				bSwap = FilesArray[i].lSourceCount  < FilesArray[i+1].lSourceCount ;
				break;
			case DOWN_SORT_PRIORITY:
				bSwap = FilesArray[i].nFilePrio < FilesArray[i+1].nFilePrio ;
				break;
			case DOWN_SORT_CATEGORY:
				bSwap = FilesArray[i].sCategory.CompareNoCase(FilesArray[i+1].sCategory) < 0;
				break;
			}
			if(pThis->m_Params.bDownloadSortReverse)
				bSwap = !bSwap;
			if(bSwap)
			{
				bSorted = true;
				DownloadFiles TmpFile = FilesArray[i];
				FilesArray[i] = FilesArray[i+1];
				FilesArray[i+1] = TmpFile;
			}
		}
	}

	// Displaying
	CString sDownList, HTTPProcessData;
	CString OutE = pThis->m_Templates.sTransferDownLine;
	CString HTTPTemp;

	for(size_t i = 0; i < FilesArray.GetCount(); i++)
	{
		HTTPProcessData = OutE;

		DownloadFiles dwnlf=FilesArray.GetAt(i);


		CString JSED2kLink=dwnlf.sED2kLink;
		JSED2kLink.Replace(_T("'"),_T("&#8217;"));
		CString ed2k = JSED2kLink;;			//ed2klink
		CString isgetflc;		//getflc
		CString fsize;			//filesize
		CString downpriority;	//priority
		CString catindex;	//category

		fsize.Format(_T("%I64u"),dwnlf.m_qwFileSize);
		catindex.Format(_T("%u"),dwnlf.iCatIndex);

		//CPartFile *found_file = theApp.downloadqueue->GetFileByID(_GetFileHash(dwnlf.sFileHash, FileHashA4AF));

		dwnlf.sFileInfo.Replace(_T("\\"),_T("\\\\"));
		dwnlf.sFileInfo.Replace(_T("&lt;br_head&gt;"),_T(""));// X: [BF] - [Bug Fix]
		CString strFInfo = dwnlf.sFileInfo;
		strFInfo.Replace(_T("'"),_T("&#8217;"));
		strFInfo.Replace(_T("\n"),_T("\\n"));

		dwnlf.sFileInfo.Replace(_T("\n"), _T("<br>"));
		if (!dwnlf.bIsPreview)
			isgetflc = (dwnlf.bIsGetFLC)?_T("enabled"):_T("disabled");

		if(dwnlf.bFileAutoPrio)
			downpriority = _T("Auto");
		else
		{
			switch(dwnlf.nFilePrio)
			{
				case 0:
					downpriority = _T("Low");
					break;
				case 1:
					downpriority = _T("Normal");
					break;
				case 2:
					downpriority = _T("High");
					break;
			}
		}

		HTTPProcessData.Replace(_T("[finfo]"), strFInfo);
		HTTPProcessData.Replace(_T("[fcomments]"), (dwnlf.iComment==0)?_T(""):_T("yes") );
		HTTPProcessData.Replace(_T("[ed2k]"), ed2k);
		HTTPProcessData.Replace(_T("[DownState]"), dwnlf.sFileState);
		HTTPProcessData.Replace(_T("[isgetflc]"), isgetflc);
		HTTPProcessData.Replace(_T("[fname]"), dwnlf.sFileNameJS);
		HTTPProcessData.Replace(_T("[fsize]"), fsize);
		HTTPProcessData.Replace(_T("[filehash]"), dwnlf.sFileHash);
		HTTPProcessData.Replace(_T("[down-priority]"), downpriority);
		HTTPProcessData.Replace(_T("[FileType]"), dwnlf.sFileType);
		HTTPProcessData.Replace(_T("[downloadable]"), (bAdmin && (thePrefs.GetMaxWebUploadFileSizeMB()==0 ||dwnlf.m_qwFileSize<thePrefs.GetMaxWebUploadFileSizeMB()*1024*1024))?_T("yes"):_T("no")  );
		HTTPProcessData.Replace(_T("[CatIndex]"), catindex);
		
		// comment icon
		if (!dwnlf.iComment)
			HTTPProcessData.Replace(_T("[FileCommentIcon]"), _T("none") );
		else if (dwnlf.iComment==1)
			HTTPProcessData.Replace(_T("[FileCommentIcon]"), _T("cmtgood") );
		else if (dwnlf.iComment==2)
			HTTPProcessData.Replace(_T("[FileCommentIcon]"), _T("cmtbad") );

		if (!WSdownloadColumnHidden[0]){
			if(dwnlf.sFileName.GetLength() > (SHORT_LENGTH_MAX))
				HTTPProcessData.Replace(_T("[ShortFileName]"), dwnlf.sFileName.Left(SHORT_LENGTH_MAX-3) + _T("..."));
			else
				HTTPProcessData.Replace(_T("[ShortFileName]"), dwnlf.sFileName);
		}
		else
			HTTPProcessData.Replace(_T("[ShortFileName]"), _T(""));

		HTTPProcessData.Replace(_T("[FileInfo]"), dwnlf.sFileInfo);

		HTTPProcessData.Replace(_T("[2]"), !WSdownloadColumnHidden[1]?CastItoXBytes(dwnlf.m_qwFileSize):_T(""));
		HTTPProcessData.Replace(_T("[3]"), (!WSdownloadColumnHidden[2] && dwnlf.m_qwFileTransferred > 0)?CastItoXBytes(dwnlf.m_qwFileTransferred):_T(""));

		if(!WSdownloadColumnHidden[3]){
			HTTPProcessData.Replace(_T("[DownloadBar]"), _GetDownloadGraph(Data,dwnlf.sFileHash));
			HTTPTemp.Format(_T("%.2f%%"),dwnlf.m_dblCompleted);
			HTTPProcessData.Replace(_T("[Progress]"), HTTPTemp);
		}
		else{
			HTTPProcessData.Replace(_T("[DownloadBar]"), _T(""));
			HTTPProcessData.Replace(_T("[Progress]"), _T(""));
		}
		if (!WSdownloadColumnHidden[4] && dwnlf.lFileSpeed > 0.0f){
			HTTPTemp.Format(_T("%8.2f"), dwnlf.lFileSpeed/1024.0);
			HTTPProcessData.Replace(_T("[4]"), HTTPTemp);
		}
		else
			HTTPProcessData.Replace(_T("[4]"), _T(""));

		if (!WSdownloadColumnHidden[5] && dwnlf.lSourceCount > 0){
			HTTPTemp.Format(_T("%i/%i(%i)"),
		    dwnlf.lSourceCount-dwnlf.lNotCurrentSourceCount,
		    dwnlf.lSourceCount,
		    dwnlf.lTransferringSourceCount);
			HTTPProcessData.Replace(_T("[5]"), HTTPTemp);
		}
		else
			HTTPProcessData.Replace(_T("[5]"), _T(""));

		if (!WSdownloadColumnHidden[6]){
			if(dwnlf.bFileAutoPrio)
			{
				switch(dwnlf.nFilePrio)
				{
					case 0: HTTPTemp=GetResString(IDS_PRIOAUTOLOW); break;
					case 1: HTTPTemp=GetResString(IDS_PRIOAUTONORMAL); break;
					case 2: HTTPTemp=GetResString(IDS_PRIOAUTOHIGH); break;
				}
			}
			else
			{
				switch(dwnlf.nFilePrio)
				{
					case  0: HTTPTemp=GetResString(IDS_PRIOLOW); break;
					case  1: HTTPTemp=GetResString(IDS_PRIONORMAL); break;
					case  2: HTTPTemp=GetResString(IDS_PRIOHIGH); break;
				}
			}
			HTTPProcessData.Replace(_T("[PrioVal]"), HTTPTemp);
		}
		else
			HTTPProcessData.Replace(_T("[PrioVal]"), _T(""));

		HTTPProcessData.Replace(_T("[Category]"), !WSdownloadColumnHidden[7]?dwnlf.sCategory:_T(""));

		sDownList += HTTPProcessData;
	}

	CString Out = pThis->m_Templates.sTransferDownFooter;
	Out.Replace(_T("[DownloadFilesList]"), sDownList);
	HTTPTemp.Format(_T("%i"),cat);
	Out.Replace(_T("[CatIndex]"), HTTPTemp);

	Out.Replace(_T("[SortReverse]"), pThis->m_Params.bDownloadSortReverse?_T("1"):_T("0"));
	sDownList += Out;
	return sDownList;
}

CString CWebServer::_GetUploadList(ThreadData&Data)
{
	CWebServer *pThis = (CWebServer *)Data.pThis;
	//if(pThis == NULL)
		//return _T("");
//	uint32	dwClientSoft;
	CAtlArray<UploadUsers> UploadArray;

	CUpDownClient* cur_client;

	for (POSITION pos = theApp.uploadqueue->GetFirstFromUploadList();
		pos != 0;theApp.uploadqueue->GetNextFromUploadList(pos))
	{
		cur_client = theApp.uploadqueue->GetQueueClientAt(pos);

		UploadUsers dUser;
		dUser.sUserHash = md4str(cur_client->GetUserHash());
		dUser.sClientState = (cur_client->GetUploadDatarate() > 0)?_T("uploading"):_T("connecting");//Xman

		dUser.sFileInfo = _SpecialChars(GetClientSummary(cur_client),false);
		dUser.sFileInfo.Replace(_T("\\"),_T("\\\\"));
		dUser.sFileInfo.Replace(_T("\n"), _T("<br>"));
		dUser.sFileInfo.Replace(_T("'"), _T("&#8217;"));

		dUser.sClientSoft = GetClientversionImage(cur_client);
		
		if (cur_client->IsFriend())
			dUser.sClientExtra = _T("friend");
		else if (cur_client->Credits()->GetScoreRatio(cur_client->GetIP()) > 1)
			dUser.sClientExtra = _T("credit");
		else
			dUser.sClientExtra = _T("none");
		
		dUser.sUserName = _SpecialChars(cur_client->GetUserName());

		CString cun(cur_client->GetUserName());
		if(cun.GetLength() > SHORT_LENGTH_MIN)
			dUser.sUserName = _SpecialChars(cun.Left(SHORT_LENGTH_MIN-3)) + _T("...");
		
		CKnownFile* file = theApp.sharedfiles->GetFileByID(cur_client->GetUploadFileID() );
		if (file)
			dUser.sFileName = _SpecialChars(file->GetFileName());
		else
			dUser.sFileName = _GetPlainResString(IDS_REQ_UNKNOWNFILE);
		dUser.nTransferredDown = cur_client->GetTransferredDown();
		dUser.nTransferredUp = cur_client->GetTransferredUp();
		dUser.nDataRate = cur_client->GetUploadDatarate();
		dUser.sClientNameVersion = cur_client->DbgGetFullClientSoftVer();
		UploadArray.Add(dUser);
	}

	// Sorting (simple bubble sort, we don't have tons of data here)
	bool bSorted = true;
	for(size_t nMax = 0;bSorted && nMax < UploadArray.GetCount()*2; nMax++)
	{
		bSorted = false;
		for(size_t i = 0; i < UploadArray.GetCount() - 1; i++)
		{
			bool bSwap = false;
			switch(pThis->m_Params.UploadSort)
			{
			case UP_SORT_CLIENT:
				bSwap = UploadArray[i].sClientSoft.CompareNoCase(UploadArray[i+1].sClientSoft) < 0;
				break;
			case UP_SORT_USER:
				bSwap = UploadArray[i].sUserName.CompareNoCase(UploadArray[i+1].sUserName) < 0;
				break;
			case UP_SORT_VERSION:
				bSwap = UploadArray[i].sClientNameVersion.CompareNoCase(UploadArray[i+1].sClientNameVersion) < 0;
				break;
			case UP_SORT_FILENAME:
				bSwap = UploadArray[i].sFileName.CompareNoCase(UploadArray[i+1].sFileName) < 0;
				break;
			case UP_SORT_TRANSFERRED:
				bSwap = UploadArray[i].nTransferredUp < UploadArray[i+1].nTransferredUp;
				break;
			case UP_SORT_SPEED:
				bSwap = UploadArray[i].nDataRate < UploadArray[i+1].nDataRate;
				break;
			}
			if(pThis->m_Params.bUploadSortReverse)
				bSwap = !bSwap;
			if(bSwap)
			{
				bSorted = true;
				UploadUsers TmpUser = UploadArray[i];
				UploadArray[i] = UploadArray[i+1];
				UploadArray[i+1] = TmpUser;
			}
		}
	}

	CString sUpList, HTTPProcessData;

	CString OutE = pThis->m_Templates.sTransferUpLine;

	CString HTTPTemp;
	for(size_t i = 0; i < UploadArray.GetCount(); i++)
	{
		UploadUsers ulu=UploadArray.GetAt(i);
		HTTPProcessData = OutE;

		HTTPProcessData.Replace(_T("[UserHash]"), ulu.sUserHash);
		HTTPProcessData.Replace(_T("[FileInfo]"), ulu.sFileInfo);
		HTTPProcessData.Replace(_T("[ClientState]"), ulu.sClientState);
		HTTPProcessData.Replace(_T("[ClientSoft]"), ulu.sClientSoft);
		HTTPProcessData.Replace(_T("[ClientExtra]"), ulu.sClientExtra);

		HTTPProcessData.Replace(_T("[1]"), (!WSuploadColumnHidden[0]) ? ulu.sUserName : _T(""));
		HTTPProcessData.Replace(_T("[ClientSoftV]"), (!WSuploadColumnHidden[1]) ? ulu.sClientNameVersion : _T(""));
		HTTPProcessData.Replace(_T("[2]"), (!WSuploadColumnHidden[2]) ? ulu.sFileName : _T(""));

		if (!WSuploadColumnHidden[3])
		{
			HTTPTemp.Format(_T("%s/%s"),CastItoXBytes(ulu.nTransferredUp), CastItoXBytes(ulu.nTransferredDown));
			HTTPProcessData.Replace(_T("[3]"), HTTPTemp);
		}
		else
			HTTPProcessData.Replace(_T("[3]"), _T(""));

		if (!WSuploadColumnHidden[4])
		{
			HTTPTemp.Format(_T("%8.2f"), max((double)ulu.nDataRate/1024, 0.0));
			HTTPProcessData.Replace(_T("[4]"), HTTPTemp);
		}
		else
			HTTPProcessData.Replace(_T("[4]"), _T(""));

		sUpList += HTTPProcessData;
	}
	CString Out = pThis->m_Templates.sTransferUpFooter;
	Out.Replace(_T("[SortReverse]"), pThis->m_Params.bUploadSortReverse?_T("1"):_T("0"));
	sUpList += Out;
	return sUpList;
}

CString CWebServer::_GetQueueList(ThreadData&Data)
{
	CWebServer *pThis = (CWebServer *)Data.pThis;
	//if(pThis == NULL)
		//return _T("");

	size_t nCountQueueSecure = 0;
	double fTotalSize = 0, fTotalTransferred = 0, fTotalSpeed = 0;

	std::vector<QueueUsers> QueueArray;

	for (POSITION pos = theApp.uploadqueue->waitinglist.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* cur_client = theApp.uploadqueue->waitinglist.GetNext(pos);
		QueueUsers dUser;

		bool bSecure = (cur_client->Credits()->GetCurrentIdentState(cur_client->GetIP()) == IS_IDENTIFIED);
		if (cur_client->IsFriend())
		{
			dUser.sClientExtra = _T("friend");
		}	
		else if (cur_client->Credits()->GetScoreRatio(cur_client->GetIP()) > 1)
			dUser.sClientExtra = _T("credit");
		else
			dUser.sClientExtra = _T("none");
		if (bSecure) nCountQueueSecure++;

		CString usn(cur_client->GetUserName());
		dUser.sortIndex[0] = GetClientversionImage(cur_client);
		dUser.sortIndex[1] = _SpecialChars((usn.GetLength() > SHORT_LENGTH_MIN)?(usn.Left((SHORT_LENGTH_MIN-3)) + _T("...")):usn);
		dUser.sortIndex[2] = cur_client->DbgGetFullClientSoftVer();
		CKnownFile* file = theApp.sharedfiles->GetFileByID(cur_client->GetUploadFileID() );
		dUser.sortIndex[3] = file?_SpecialChars(file->GetFileName()):_GetPlainResString(IDS_REQ_UNKNOWNFILE);
		dUser.sClientState = dUser.sClientExtra;
		dUser.sClientStateSpecial = _T("connecting");
		dUser.nScore = cur_client->GetScore(false);
		dUser.sUserHash = md4str(cur_client->GetUserHash());
		QueueArray.push_back(dUser);
	}

	size_t nNextPos = 0;	// position in queue of the user with the highest score -> next upload user
	uint_ptr nNextScore = 0;	// highest score -> next upload user
	for(size_t i = 0; i < QueueArray.size(); i++)
	{
		if (QueueArray[i].nScore > nNextScore)
		{
			nNextPos = i;
			nNextScore = QueueArray[i].nScore;
		}
	}
	if (theApp.uploadqueue->waitinglist.GetHeadPosition() != 0)
	{
		QueueArray[nNextPos].sClientState = _T("next");
		QueueArray[nNextPos].sClientStateSpecial = QueueArray[nNextPos].sClientState;
	}

	size_t nCountQueue = theApp.uploadqueue->waitinglist.GetCount();
	if (nCountQueue > 0)
	{
#ifdef _DEBUG
	DWORD dwStart = ::GetTickCount();
#endif
	switch(pThis->m_Params.QueueSort)
	{
		case QU_SORT_CLIENT:
		case QU_SORT_USER:
		case QU_SORT_VERSION:
		case QU_SORT_FILENAME:{
			std::sort(QueueArray.begin(), QueueArray.end(), CmpQueueUsersByStr(pThis->m_Params.bQueueSortReverse, pThis->m_Params.QueueSort));
			break;
		}
		case QU_SORT_SCORE:
			std::sort(QueueArray.begin(), QueueArray.end(), CmpQueueUsersByScore(pThis->m_Params.bQueueSortReverse));
			break;
	}
#ifdef _DEBUG
	AddDebugLogLine(false, _T("WebServer: Waitingqueue with %u elements sorted in %u ms"), QueueArray.size(), ::GetTickCount()-dwStart);
#endif
	}
	CString sQueue, HTTPProcessData;

	CString OutE = pThis->m_Templates.sTransferUpQueueLine;

	CString HTTPTemp;

	for(size_t i = 0; i < QueueArray.size(); i++){
        TCHAR HTTPTempC[100] = _T("");
		if (QueueArray[i].sClientExtra == _T("none")){
			HTTPProcessData = OutE;
			HTTPProcessData.Replace(_T("[UserName]"), (!WSqueueColumnHidden[0]) ? QueueArray[i].sortIndex[1] : _T(""));
			HTTPProcessData.Replace(_T("[ClientSoftV]"), (!WSqueueColumnHidden[1]) ? QueueArray[i].sortIndex[2] : _T(""));
			HTTPProcessData.Replace(_T("[FileName]"), (!WSqueueColumnHidden[2]) ? QueueArray[i].sortIndex[3] : _T(""));

			if (!WSqueueColumnHidden[3])
			{
				_ultot_s_ptr(QueueArray[i].nScore, HTTPTempC, _countof(HTTPTempC), 10);// X: [CI] - [Code Improvement]
				HTTPProcessData.Replace(_T("[Score]"), HTTPTempC);
			}
			else
				HTTPProcessData.Replace(_T("[Score]"), _T(""));
			HTTPProcessData.Replace(_T("[ClientState]"), QueueArray[i].sClientState);
			HTTPProcessData.Replace(_T("[ClientStateSpecial]"), QueueArray[i].sClientStateSpecial);
			HTTPProcessData.Replace(_T("[ClientSoft]"), QueueArray[i].sortIndex[0]);
			HTTPProcessData.Replace(_T("[ClientExtra]"), QueueArray[i].sClientExtra);
			HTTPProcessData.Replace(_T("[UserHash]"), QueueArray[i].sUserHash);

			sQueue += HTTPProcessData;
		}
	}
	CString Out = pThis->m_Templates.sTransferUpQueueFooter;
	HTTPTemp.Format(_T("%i (%i)"),nCountQueue,nCountQueueSecure);
	Out.Replace(_T("[CounterQueue]"), HTTPTemp);

	Out.Replace(_T("[SortReverse]"), pThis->m_Params.bQueueSortReverse?_T("1"):_T("0"));
	sQueue += Out;
	return sQueue;
}

CString CWebServer::_GetSharedFilesWnd(ThreadData&Data)
{
	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString	sSession = _ParseURL(Data.sURL, _T("ses"));
	CString hash = _ParseURL(Data.sURL, _T("hash"));
	if(!hash.IsEmpty() && !_ParseURL(Data.sURL, _T("prio")).IsEmpty() && IsSessionAdmin(Data,sSession))
	{
		CKnownFile* cur_file;
		uchar fileid[16];
		if (hash.GetLength()==32 && DecodeBase16(hash, hash.GetLength(), fileid, _countof(fileid)))
		{
			cur_file = theApp.sharedfiles->GetFileByID(fileid);

			if (cur_file != 0)
			{
				cur_file->SetAutoUpPriority(false);
				CString strTmp = _ParseURL(Data.sURL, _T("prio"));
				if (strTmp == _T("verylow"))
					cur_file->SetUpPriority(PR_VERYLOW);
				else if (strTmp == _T("low"))
					cur_file->SetUpPriority(PR_LOW);
				else if (strTmp == _T("normal"))
					cur_file->SetUpPriority(PR_NORMAL);
				else if (strTmp == _T("high"))
					cur_file->SetUpPriority(PR_HIGH);
				else if (strTmp == _T("release"))
					cur_file->SetUpPriority(PR_VERYHIGH);
				//Xman PowerRelease
				else if (strTmp == _T("PowerRelease"))
				{
					if(cur_file->IsPartFile())			
						cur_file->SetUpPriority(PR_VERYHIGH);
					else
						cur_file->SetUpPriority(PR_POWER);
				}
				//Xman end
				else if (strTmp == _T("auto"))
				{
					cur_file->SetAutoUpPriority(true);
					//Xman advanced upload-priority
					if (thePrefs.UseAdvancedAutoPtio())
						cur_file->CalculateAndSetUploadPriority();
					else
						cur_file->UpdateAutoUpPriority();
					//Xman end
				}

				SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION, WEBGUIIA_UPD_SFUPDATE, (LPARAM)cur_file);
			}
		}
	}

	CString sMessage;
	if(_ParseURL(Data.sURL, _T("reload")) == _T("true"))
	{
		SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION,WEBGUIIA_SHARED_FILES_RELOAD,0);
		CString strResultLog = _SpecialChars(theApp.emuledlg->GetLastLogEntry() );	//Pick-up last line of the log
		strResultLog = strResultLog.TrimRight(_T("\r\n"));
		int iStringIndex = strResultLog.ReverseFind(_T('\n'));
		if (iStringIndex != -1)
			strResultLog = strResultLog.Mid(iStringIndex);
		sMessage = strResultLog;
	}

	CString sCmd = _ParseURL(Data.sURL, _T("l"));
	if (sCmd == _T("sort"))
	{
		CString sSortReverse = _ParseURL(Data.sURL, _T("sortAsc"));
		CString sSort = _ParseURL(Data.sURL, _T("sort"));
		if (!sSort.IsEmpty())
		{
			pThis->m_Params.SharedSort = (SharedSort)_tstol(sSort);

			if (sSortReverse.IsEmpty())
				pThis->m_Params.bSharedSortReverse = (pThis->m_Params.SharedSort == SHARED_SORT_NAME);
		}
		if (!sSortReverse.IsEmpty())
			pThis->m_Params.bSharedSortReverse = _tstol(sSortReverse)!=0;
		return _GetSharedFilesList(Data, sMessage);
	}
	sCmd = _ParseURL(Data.sURL, _T("c"));
	if (sCmd == _T("menu")){
		int iMenu = _tstoi(_ParseURL(Data.sURL, _T("m")));
		if(iMenu<_countof(WSsharedColumnHidden)){
			WSsharedColumnHidden[iMenu] = !WSsharedColumnHidden[iMenu];
			SaveWIConfigArray(WSsharedColumnHidden, _countof(WSsharedColumnHidden), _T("sharedColumnHidden"));
		}
		return _T("");
	}

	CString Out = pThis->m_Templates.sSharedList;

	// ListText
	Out.Replace(_T("[SharedList]"), _GetPlainResString(IDS_SHAREDFILES));
	CString strTemp;
	for(uint_ptr i = 0, added = 0;i < _countof(WSsharedColumnHidden);++i){
		if(!WSsharedColumnHidden[i])
			continue;
		if(added>0)
			strTemp.AppendChar(_T(','));
		strTemp.AppendFormat(_T("%i"), i);
		++added;
	}
	Out.Replace(_T("[ColumnHidden]"), strTemp);
	strTemp.Format(_T("%i"), pThis->m_Params.SharedSort);
	Out.Replace(_T("[SortColumn]"), strTemp);
	Out.Replace(_T("[Refresh]"), _GetPlainResString(IDS_SV_UPDATE));

	// Column Header
	Out.Replace(_T("[Filename]"),			_GetPlainResString(IDS_DL_FILENAME));
	Out.Replace(_T("[FileTransferred]"),	_GetPlainResString(IDS_SF_TRANSFERRED));
	Out.Replace(_T("[FileRequests]"),		_GetPlainResString(IDS_SF_REQUESTS));
	Out.Replace(_T("[FileAccepts]"),		_GetPlainResString(IDS_SF_ACCEPTS));
	Out.Replace(_T("[Size]"),				_GetPlainResString(IDS_DL_SIZE));
	Out.Replace(_T("[Completes]"),			_GetPlainResString(IDS_COMPLSOURCES));
	Out.Replace(_T("[Priority]"),			_GetPlainResString(IDS_PRIORITY));

	Out.Replace(_T("[Reload]"), _GetPlainResString(IDS_SF_RELOAD));
	// List Content
	Out.Replace(_T("[SharedFilesList]"), _GetSharedFilesList(Data, sMessage));

	return Out;
}

CString CWebServer::_GetSharedFilesList(ThreadData&Data, CString&sMessage)
{
	CWebServer *pThis = (CWebServer *)Data.pThis;
	//if(pThis == NULL)
		//return _T("");

	CAtlArray<SharedFiles> SharedArray;

	// Populating array
#ifdef REPLACE_ATLMAP
	const CKnownFilesMap& file_map = theApp.sharedfiles->GetSharedFiles();
	for (CKnownFilesMap::const_iterator it = file_map.begin(); it != file_map.end(); ++it)
	{
		CKnownFile* cur_file = it->second;
#else
	for (INT_PTR ix=0;ix<theApp.sharedfiles->GetCount();ix++){
		CCKey bufKey;
		CKnownFile* cur_file = theApp.sharedfiles->GetFileByIndex(ix);
#endif
		//		uint16 nCountLo, nCountHi;
		uint32	dwResId;

		bool bPartFile = cur_file->IsPartFile();

		SharedFiles dFile;
		//dFile.sFileName = _SpecialChars(cur_file->GetFileName());
		dFile.bIsPartFile = cur_file->IsPartFile();
		dFile.sFileName = cur_file->GetFileName();
		dFile.sFileState = (bPartFile)?_T("filedown"):_T("file");
		dFile.sFileType = GetWebImageNameForFileType(dFile.sFileName);

		dFile.m_qwFileSize = cur_file->GetFileSize();
		
		if (theApp.GetPublicIP() != 0 && !theApp.IsFirewalled())
			dFile.sED2kLink = cur_file->GetED2kLink(false, false, false, true, theApp.GetPublicIP());
		else
			dFile.sED2kLink = cur_file->GetED2kLink();

		dFile.nFileTransferred = cur_file->statistic.GetTransferred();
		dFile.nFileAllTimeTransferred = cur_file->statistic.GetAllTimeTransferred();
		dFile.nFileRequests = cur_file->statistic.GetRequests();
		dFile.nFileAllTimeRequests = cur_file->statistic.GetAllTimeRequests();
		dFile.nFileAccepts = cur_file->statistic.GetAccepts();
		dFile.nFileAllTimeAccepts = cur_file->statistic.GetAllTimeAccepts();
		dFile.sFileHash = md4str(cur_file->GetFileHash());

		if (cur_file->m_nCompleteSourcesCountLo == 0)
			dFile.sFileCompletes.Format(_T("< %u"), cur_file->m_nCompleteSourcesCountHi);
		else if (cur_file->m_nCompleteSourcesCountLo == cur_file->m_nCompleteSourcesCountHi)
			dFile.sFileCompletes.Format(_T("%u"), cur_file->m_nCompleteSourcesCountLo);
		else
			dFile.sFileCompletes.Format(_T("%u - %u"), cur_file->m_nCompleteSourcesCountLo, cur_file->m_nCompleteSourcesCountHi);

		if (cur_file->IsAutoUpPriority())
		{
			if (cur_file->GetUpPriority() == PR_NORMAL)
				dwResId = IDS_PRIOAUTONORMAL;
			else if (cur_file->GetUpPriority() == PR_HIGH)
				dwResId = IDS_PRIOAUTOHIGH;
			else if (cur_file->GetUpPriority() == PR_VERYHIGH)
				dwResId = IDS_PRIOAUTORELEASE;
			else	//PR_LOW
				dwResId = IDS_PRIOAUTOLOW;
		}
		else
		{
			if (cur_file->GetUpPriority() == PR_LOW)
				dwResId = IDS_PRIOLOW;
			else if (cur_file->GetUpPriority() == PR_NORMAL)
				dwResId = IDS_PRIONORMAL;
			else if (cur_file->GetUpPriority() == PR_HIGH)
				dwResId = IDS_PRIOHIGH;
			else if (cur_file->GetUpPriority() == PR_VERYHIGH)
				dwResId = IDS_PRIORELEASE;
			//Xman PowerRelease
			else if (cur_file->GetUpPriority() == PR_POWER)
				dwResId = IDS_POWERRELEASE;
			//Xman end
			else	//PR_VERYLOW
				dwResId = IDS_PRIOVERYLOW;
		}
		dFile.sFilePriority=GetResString(dwResId);

		dFile.nFilePriority = cur_file->GetUpPriority();
		dFile.bFileAutoPriority = cur_file->IsAutoUpPriority();
		SharedArray.Add(dFile);
	}

	// Sorting (simple bubble sort, we don't have tons of data here)
	bool bSorted = true;

	for(size_t nMax = 0;bSorted && nMax < SharedArray.GetCount()*2; nMax++)
	{
		bSorted = false;
		for(size_t i = 0; i < SharedArray.GetCount() - 1; i++)
		{
			bool bSwap = false;
			switch(pThis->m_Params.SharedSort)
			{
			case SHARED_SORT_STATE:
				bSwap = SharedArray[i].sFileState.CompareNoCase(SharedArray[i+1].sFileState) > 0;
				break;
			case SHARED_SORT_TYPE:
				bSwap = SharedArray[i].sFileType.CompareNoCase(SharedArray[i+1].sFileType) > 0;
				break;
			case SHARED_SORT_NAME:
				bSwap = SharedArray[i].sFileName.CompareNoCase(SharedArray[i+1].sFileName) < 0;
				break;
			case SHARED_SORT_SIZE:
				bSwap = SharedArray[i].m_qwFileSize < SharedArray[i+1].m_qwFileSize;
				break;
			case SHARED_SORT_TRANSFERRED:
				bSwap = SharedArray[i].nFileTransferred < SharedArray[i+1].nFileTransferred;
				break;
			case SHARED_SORT_ALL_TIME_TRANSFERRED:
				bSwap = SharedArray[i].nFileAllTimeTransferred < SharedArray[i+1].nFileAllTimeTransferred;
				break;
			case SHARED_SORT_REQUESTS:
				bSwap = SharedArray[i].nFileRequests < SharedArray[i+1].nFileRequests;
				break;
			case SHARED_SORT_ALL_TIME_REQUESTS:
				bSwap = SharedArray[i].nFileAllTimeRequests < SharedArray[i+1].nFileAllTimeRequests;
				break;
			case SHARED_SORT_ACCEPTS:
				bSwap = SharedArray[i].nFileAccepts < SharedArray[i+1].nFileAccepts;
				break;
			case SHARED_SORT_ALL_TIME_ACCEPTS:
				bSwap = SharedArray[i].nFileAllTimeAccepts < SharedArray[i+1].nFileAllTimeAccepts;
				break;
			case SHARED_SORT_COMPLETES:
				bSwap = SharedArray[i].dblFileCompletes < SharedArray[i+1].dblFileCompletes;
				break;
			case SHARED_SORT_PRIORITY:
				//Very low priority is define equal to 4 ! Must adapte sorting code
				if(SharedArray[i].nFilePriority == 4)
					bSwap = (SharedArray[i+1].nFilePriority != 4);
				else if(SharedArray[i+1].nFilePriority == 4)
					bSwap = (SharedArray[i].nFilePriority == 4);
				else
					bSwap = SharedArray[i].nFilePriority < SharedArray[i+1].nFilePriority;
				break;
			}
			if(pThis->m_Params.bSharedSortReverse)
				bSwap = !bSwap;
			if(bSwap)
			{
				bSorted = true;
				SharedFiles TmpFile = SharedArray[i];
				SharedArray[i] = SharedArray[i+1];
				SharedArray[i+1] = TmpFile;
			}
		}
	}


	CString OutE = pThis->m_Templates.sSharedLine;
	// Displaying
	CString sSharedList, HTTPProcessData;
	for(size_t i = 0; i < SharedArray.GetCount(); i++)
	{
		TCHAR HTTPTempC[100] = _T("");
		HTTPProcessData = OutE;

		CString ed2k;				//ed2klink
		CString hash;				//hash
		CString fname;				//filename
		CString sharedpriority;		//priority

		switch(SharedArray[i].nFilePriority)
		{
		case PR_VERYLOW:
			sharedpriority = _T("VeryLow");
			break;
		case PR_LOW:
			sharedpriority = _T("Low");
			break;
		case PR_NORMAL:
			sharedpriority = _T("Normal");
			break;
		case PR_HIGH:
			sharedpriority = _T("High");
			break;
		case PR_VERYHIGH:
			sharedpriority = _T("Release");
			break;
			//Xman PowerRelease
		case PR_POWER:
			sharedpriority = _T("PowerRelease");
			break;
			//Xman end
		}
		if (SharedArray[i].bFileAutoPriority)
			sharedpriority = _T("Auto");

		CString JSED2kLink=SharedArray[i].sED2kLink;
		JSED2kLink.Replace(_T("'"),_T("&#8217;"));

		ed2k = JSED2kLink;
		hash = SharedArray[i].sFileHash;
		fname = SharedArray[i].sFileName;
		fname.Replace(_T("'"),_T("&#8217;"));

		bool downloadable = false;
		uchar fileid[16] = {0};
		if (hash.GetLength()==32 && DecodeBase16(hash, hash.GetLength(), fileid, _countof(fileid)))
		{
			HTTPProcessData.Replace(_T("[hash]"), hash);
			CKnownFile* cur_file = theApp.sharedfiles->GetFileByID(fileid);
			if (cur_file != NULL)
			{
				HTTPProcessData.Replace(_T("[FileIsPriority]"), (cur_file->GetUpPriority() == PR_VERYHIGH|| cur_file->GetUpPriority() == PR_POWER)?_T("release"):_T("none"));
				downloadable = !cur_file->IsPartFile() && (thePrefs.GetMaxWebUploadFileSizeMB() == 0 || SharedArray[i].m_qwFileSize < thePrefs.GetMaxWebUploadFileSizeMB()*1024*1024);
			}
		}

		HTTPProcessData.Replace(_T("[ed2k]"), ed2k);
		HTTPProcessData.Replace(_T("[fname]"), fname);
		HTTPProcessData.Replace(_T("[shared-priority]"), sharedpriority); //DonGato: priority change

		HTTPProcessData.Replace(_T("[FileName]"), _SpecialChars(SharedArray[i].sFileName));
		HTTPProcessData.Replace(_T("[FileType]"), SharedArray[i].sFileType);
		HTTPProcessData.Replace(_T("[FileState]"), SharedArray[i].sFileState);


		HTTPProcessData.Replace(_T("[Downloadable]"), downloadable?_T("yes"):_T("no") );

		HTTPProcessData.Replace(_T("[ShortFileName]"), (!WSsharedColumnHidden[0])?
			((SharedArray[i].sFileName.GetLength() > SHORT_LENGTH)?_SpecialChars(SharedArray[i].sFileName.Left(SHORT_LENGTH-3)) + _T("..."):_SpecialChars(SharedArray[i].sFileName))
			:_T(""));
		if (!WSsharedColumnHidden[1])
		{
			HTTPProcessData.Replace(_T("[FileTransferred]"), CastItoXBytes(SharedArray[i].nFileTransferred));
			HTTPProcessData.Replace(_T("[FileAllTimeTransferred]"), _T(" (") +CastItoXBytes(SharedArray[i].nFileAllTimeTransferred)+_T(')') );
		}
		else
		{
			HTTPProcessData.Replace(_T("[FileTransferred]"), _T(""));
			HTTPProcessData.Replace(_T("[FileAllTimeTransferred]"), _T(""));
		}
		if (!WSsharedColumnHidden[2])
		{
			_ultot_s(SharedArray[i].nFileRequests, HTTPTempC, 10);// X: [CI] - [Code Improvement]
			HTTPProcessData.Replace(_T("[FileRequests]"), HTTPTempC);
			_ultot_s(SharedArray[i].nFileAllTimeRequests, HTTPTempC, 10);// X: [CI] - [Code Improvement]
			HTTPProcessData.Replace(_T("[FileAllTimeRequests]"), _T(" (") + CString(HTTPTempC) + _T(')'));
		}
		else
		{
			HTTPProcessData.Replace(_T("[FileRequests]"), _T(""));
			HTTPProcessData.Replace(_T("[FileAllTimeRequests]"), _T(""));
		}
		if (!WSsharedColumnHidden[3])
		{
			_ultot_s(SharedArray[i].nFileAccepts, HTTPTempC, 10);// X: [CI] - [Code Improvement]
			HTTPProcessData.Replace(_T("[FileAccepts]"), HTTPTempC);
			_ultot_s(SharedArray[i].nFileAllTimeAccepts, HTTPTempC, 10);// X: [CI] - [Code Improvement]
			HTTPProcessData.Replace(_T("[FileAllTimeAccepts]"), _T(" (") + CString(HTTPTempC) + _T(')'));
		}
		else
		{
			HTTPProcessData.Replace(_T("[FileAccepts]"), _T(""));
			HTTPProcessData.Replace(_T("[FileAllTimeAccepts]"), _T(""));
		}
		HTTPProcessData.Replace(_T("[FileSize]"), (!WSsharedColumnHidden[4])?CastItoXBytes(SharedArray[i].m_qwFileSize):_T(""));
		HTTPProcessData.Replace(_T("[Completes]"), (!WSsharedColumnHidden[5])?SharedArray[i].sFileCompletes:_T(""));
		HTTPProcessData.Replace(_T("[Priority]"), (!WSsharedColumnHidden[6])?SharedArray[i].sFilePriority:_T(""));
		HTTPProcessData.Replace(_T("[FileHash]"), SharedArray[i].sFileHash);

		sSharedList += HTTPProcessData;
	}
	CString Out = pThis->m_Templates.sSharedListFooter;
	Out.Replace(_T("[Message]"), sMessage);
	Out.Replace(_T("[SortReverse]"), pThis->m_Params.bSharedSortReverse?_T("1"):_T("0"));
	sSharedList += Out;
	return sSharedList;
}

CString CWebServer::_GetGraphs(ThreadData&Data)
{
	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString Out = pThis->m_Templates.sGraphs;

	Out.Replace(_T("[TxtDownload]"), _GetPlainResString(IDS_TW_DOWNLOADS));
	Out.Replace(_T("[TxtUpload]"), _GetPlainResString(IDS_TW_UPLOADS));
	Out.Replace(_T("[TxtTime]"), _GetPlainResString(IDS_TIME));
	Out.Replace(_T("[KByteSec]"), _GetPlainResString(IDS_KBYTESPERSEC));
	Out.Replace(_T("[TxtConnections]"), _GetPlainResString(IDS_SP_ACTCON));

	Out.Replace(_T("[ScaleTime]"), CastSecondsToHM(thePrefs.GetTrafficOMeterInterval() * WEB_GRAPH_WIDTH));
	Out.Replace(_T("[Refresh]"), _GetPlainResString(IDS_SV_UPDATE));

	CString s1;
	//Xman
	// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
	s1.Format(_T("%u"), (int)thePrefs.GetMaxGraphDownloadRate()+4 );
	Out.Replace(_T("[MaxDownload]"), s1);
	s1.Format(_T("%u"), (int)thePrefs.GetMaxGraphUploadRate()+4 );
	Out.Replace(_T("[MaxUpload]"), s1);
	//Xman end
	s1.Format(_T("%u"), thePrefs.GetMaxConnections()+20);
	Out.Replace(_T("[MaxConnections]"), s1);

	CString strGraphDownload, strGraphUpload, strGraphCons;
	for(size_t i = 0; i < WEB_GRAPH_WIDTH; i++){
		if(i < pThis->m_Params.PointsForWeb.GetCount()){
			if(i != 0){
				strGraphDownload += _T(',');
				strGraphUpload += _T(',');
				strGraphCons += _T(',');
			}

			// download
			strGraphDownload.AppendFormat(_T("%u"), pThis->m_Params.PointsForWeb[i].download);
			// upload
			strGraphUpload.AppendFormat(_T("%u"), pThis->m_Params.PointsForWeb[i].upload);
			// connections
			strGraphCons.AppendFormat(_T("%u"), pThis->m_Params.PointsForWeb[i].connections);
		}
	}

	Out.Replace(_T("[GraphDownload]"), strGraphDownload);
	Out.Replace(_T("[GraphUpload]"), strGraphUpload);
	Out.Replace(_T("[GraphConnections]"), strGraphCons);

	return Out;
}

CString CWebServer::_GetLog(ThreadData&Data)
{
	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	
	CString sSession = _ParseURL(Data.sURL, _T("ses"));

	if (_ParseURL(Data.sURL, _T("clear")) == _T("yes") && IsSessionAdmin(Data,sSession)){
		theApp.emuledlg->ResetLog();
		return _T("");
	}

	CString Out = pThis->m_Templates.sLog;

	Out.Replace(_T("[Clear]"), _GetPlainResString(IDS_PW_RESET));
	Out.Replace(_T("[Log]"), _SpecialChars(theApp.emuledlg->GetAllLogEntries(),false));

	return Out;

}

CString CWebServer::_GetDebugLog(ThreadData&Data)
{
	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString sSession = _ParseURL(Data.sURL, _T("ses"));

	if (_ParseURL(Data.sURL, _T("clear")) == _T("yes") && IsSessionAdmin(Data,sSession)){
		theApp.emuledlg->ResetDebugLog();
		return _T("");
	}

	CString Out = pThis->m_Templates.sDebugLog;

	Out.Replace(_T("[Clear]"), _GetPlainResString(IDS_PW_RESET));
	Out.Replace(_T("[DebugLog]"), _SpecialChars(theApp.emuledlg->GetAllDebugLogEntries() ,false));

	return Out;
}

CString CWebServer::_GetMyInfo(ThreadData&Data)
{
	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString Out = pThis->m_Templates.sMyInfoLog;
	Out.Replace(_T("[MYINFOLOG]"), theApp.emuledlg->serverwnd->GetMyInfoString() );

	return Out;
}

CString CWebServer::_GetKadDlg(ThreadData&Data)
{
	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString sSession = _ParseURL(Data.sURL, _T("ses"));

	CString sCmd = _ParseURL(Data.sURL, _T("c"));// X: [CI] - [Code Improvement]
	if(!sCmd.IsEmpty() && IsSessionAdmin(Data,sSession)){// X: [CI] - [Code Improvement]
		if (sCmd == _T("bootstrap")) {
			CString dest=_ParseURL(Data.sURL, _T("ip"));
			CString ip=_ParseURL(Data.sURL, _T("port"));
			dest.Append(_T(':')+ip);
			const TCHAR* ipbuf=dest;
			SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION,WEBGUIIA_KAD_BOOTSTRAP, (LPARAM)ipbuf );
			return _T("");
		}
		if (sCmd == _T("connect") )
			SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION,WEBGUIIA_KAD_START, 0 );
		else if (sCmd == _T("disconnect") )
			SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION,WEBGUIIA_KAD_STOP, 0 );
		else if (sCmd == _T("rcfirewall") )
			SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION,WEBGUIIA_KAD_RCFW, 0 );
		return _T("");
	}

	CString Out = pThis->m_Templates.sKad;
	// check the condition if bootstrap is possible
	if ( /*Kademlia::CKademlia::IsRunning() && */ !Kademlia::CKademlia::IsConnected())
		Out.Replace(_T("[BOOTSTRAPLINE]"), pThis->m_Templates.sBootstrapLine );
	else 
		Out.Replace(_T("[BOOTSTRAPLINE]"), _T("") );

	// kadstats	
	// labels
	CString buffer;
	buffer.Format(_T("%s<br>%s"),GetResString(IDS_KADCONTACTLAB), GetResString(IDS_KADSEARCHLAB));
	Out.Replace(_T("[KADSTATSLABELS]"),buffer);

	// numbers
	buffer.Format(_T("%i<br>%i"),	theApp.emuledlg->kademliawnd->GetContactCount(), 
		theApp.emuledlg->kademliawnd->searchList->GetItemCount());
	Out.Replace(_T("[KADSTATSDATA]"),buffer);

	Out.Replace(_T("[BS_IP]"),GetResString(IDS_IP));
	Out.Replace(_T("[BS_PORT]"),GetResString(IDS_PORT));
	Out.Replace(_T("[BOOTSTRAP]"),GetResString(IDS_BOOTSTRAP));
	Out.Replace(_T("[KADSTAT]"),GetResString(IDS_STATSSETUPINFO));
	Out.Replace(_T("[STATUS]"),GetResString(IDS_STATUS));
	//Out.Replace(_T("[KAD]"),GetResString(IDS_KADEMLIA));

	// Infos
	if (Kademlia::CKademlia::IsConnected()) {
		if (Kademlia::CKademlia::IsFirewalled()) {
			Out.Replace(_T("[KADSTATUS]"), GetResString(IDS_FIREWALLED));
			buffer.Format(_T("<span onclick=\"kadcmd(\'rcfirewall\')\">%s</span> <span onclick=\"kadcmd(\'disconnect\')\">%s</span>"), GetResString(IDS_KAD_RECHECKFW), GetResString(IDS_IRC_DISCONNECT) );
		} else {
			Out.Replace(_T("[KADSTATUS]"), GetResString(IDS_CONNECTED));
			buffer.Format(_T("<span onclick=\"kadcmd(\'disconnect\')\">%s</span>"),  GetResString(IDS_IRC_DISCONNECT) );
		}
	}
	else {
		if (Kademlia::CKademlia::IsRunning()) {
			Out.Replace(_T("[KADSTATUS]"), GetResString(IDS_CONNECTING));
			buffer.Format(_T("<span onclick=\"kadcmd(\'disconnect\')\">%s</span>"),  GetResString(IDS_IRC_DISCONNECT) );
		} else {
			Out.Replace(_T("[KADSTATUS]"), GetResString(IDS_DISCONNECTED));
			buffer.Format(_T("<span onclick=\"kadcmd(\'connect\')\">%s</span>"),  GetResString(IDS_IRC_CONNECT) );
		}
	}
	Out.Replace(_T("[KADACTION]"), buffer);
	return Out;
}

CString CWebServer::_GetStats(ThreadData&Data)
{
	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	// refresh statistics
	SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION, WEBGUIIA_SHOWSTATISTICS, 1);

	CString Out = pThis->m_Templates.sStats;
	Out.Replace(_T("[Refresh]"), _GetPlainResString(IDS_SV_UPDATE));
	// eklmn: new stats
	Out.Replace(_T("[Stats]"), theApp.emuledlg->statisticswnd->stattree.GetHTMLForExport());

	return Out;
}

CString CWebServer::_GetPreferences(ThreadData&Data)
{
	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString sSession = _ParseURL(Data.sURL, _T("ses"));

	if ((_ParseURL(Data.sURL, _T("saveprefs")) == _T("true")) && IsSessionAdmin(Data,sSession) )
	{
		CString strTmp(_ParseURL(Data.sURL, _T("gzip")));// X: [CI] - [Code Improvement]
		if(strTmp == _T("true") || strTmp.MakeLower() == _T("on"))
			thePrefs.SetWebUseGzip(true);
		if(strTmp == _T("false") || strTmp.IsEmpty())
			thePrefs.SetWebUseGzip(false);

		strTmp = _ParseURL(Data.sURL, _T("refresh"));
		if(!strTmp.IsEmpty())
			thePrefs.SetWebPageRefresh(_tstoi(strTmp));

		//Xman // Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
		strTmp = _ParseURL(Data.sURL, _T("maxcapdown"));
		if(!strTmp.IsEmpty())
			thePrefs.SetMaxGraphDownloadRate((float)_tstof(strTmp));
		strTmp = _ParseURL(Data.sURL, _T("maxcapup"));
		if(!strTmp.IsEmpty())
			thePrefs.SetMaxGraphUploadRate((float)_tstof(strTmp));

		float	dwSpeed;

		strTmp = _ParseURL(Data.sURL, _T("maxdown"));
		if (!strTmp.IsEmpty())
		{
			dwSpeed = (float)_tstof(strTmp);
			thePrefs.SetMaxDownload(dwSpeed>0?dwSpeed:UNLIMITED);
		}
		strTmp = _ParseURL(Data.sURL, _T("maxup"));
		if (!strTmp.IsEmpty())
		{
			dwSpeed = (float)_tstof(strTmp);
			//Xman Xtreme Upload
			//unlimited is not allowed
			if (dwSpeed==0 || dwSpeed==UNLIMITED)
				dwSpeed=11;
			thePrefs.SetMaxUpload(dwSpeed);
			//thePrefs.CheckSlotSpeed(); //Xman Xtreme Upload
}
		//Xman end

		strTmp = _ParseURL(Data.sURL, _T("maxsources"));
		if (!strTmp.IsEmpty())
			thePrefs.SetMaxSourcesPerFile(_tstoi(strTmp));
		strTmp = _ParseURL(Data.sURL, _T("maxconnections"));
		if (!strTmp.IsEmpty())
			thePrefs.SetMaxConnections(_tstoi(strTmp));
		strTmp = _ParseURL(Data.sURL, _T("maxconnectionsperfive"));
		if (!strTmp.IsEmpty())
			thePrefs.SetMaxConsPerFive(_tstoi(strTmp));
		theApp.scheduler->SaveOriginals(); // Don't reset Connection Settings for Webserver/CML/MM [Stulle] - Stulle
		return _T("");
	}

	CString Out = pThis->m_Templates.sPreferences;
	// Fill form
	Out.Replace(_T("[UseGzipVal]"), thePrefs.GetWebUseGzip()?_T("checked"):_T(""));

	CString sRefresh;

	sRefresh.Format(_T("%d"), thePrefs.GetWebPageRefresh());
	Out.Replace(_T("[RefreshVal]"), sRefresh);

	sRefresh.Format(_T("%d"), thePrefs.GetMaxSourcePerFileDefault());
	Out.Replace(_T("[MaxSourcesVal]"), sRefresh);

	sRefresh.Format(_T("%d"), thePrefs.GetMaxConnections());
	Out.Replace(_T("[MaxConnectionsVal]"), sRefresh);

	sRefresh.Format(_T("%d"), thePrefs.GetMaxConperFive());
	Out.Replace(_T("[MaxConnectionsPer5Val]"), sRefresh);

	Out.Replace(_T("[KBS]"), _GetPlainResString(IDS_KBYTESPERSEC)+_T(':'));
	Out.Replace(_T("[LimitForm]"), _GetPlainResString(IDS_WEB_CONLIMITS)+_T(':'));
	Out.Replace(_T("[MaxSources]"), _GetPlainResString(IDS_PW_MAXSOURCES)+_T(':'));
	Out.Replace(_T("[MaxConnections]"), _GetPlainResString(IDS_PW_MAXC)+_T(':'));
	Out.Replace(_T("[MaxConnectionsPer5]"), _GetPlainResString(IDS_MAXCON5SECLABEL)+_T(':'));
	Out.Replace(_T("[UseGzipForm]"), _GetPlainResString(IDS_WEB_GZIP_COMPRESSION));
	Out.Replace(_T("[UseGzipComment]"), _GetPlainResString(IDS_WEB_GZIP_COMMENT));

	Out.Replace(_T("[RefreshTimeForm]"), _GetPlainResString(IDS_WEB_REFRESH_TIME));
	Out.Replace(_T("[RefreshTimeComment]"), _GetPlainResString(IDS_WEB_REFRESH_COMMENT));
	Out.Replace(_T("[SpeedForm]"), _GetPlainResString(IDS_SPEED_LIMITS));
	Out.Replace(_T("[SpeedCapForm]"), _GetPlainResString(IDS_CAPACITY_LIMITS));

	Out.Replace(_T("[MaxCapDown]"), _GetPlainResString(IDS_PW_CON_DOWNLBL));
	Out.Replace(_T("[MaxCapUp]"), _GetPlainResString(IDS_PW_CON_UPLBL));
	Out.Replace(_T("[MaxDown]"), _GetPlainResString(IDS_PW_CON_DOWNLBL));
	Out.Replace(_T("[MaxUp]"), _GetPlainResString(IDS_PW_CON_UPLBL));
	Out.Replace(_T("[WebControl]"), _GetPlainResString(IDS_PW_WS));
	//Out.Replace(_T("[Options]"), _GetPlainResString(IDS_EM_PREFS));
	Out.Replace(_T("[eMuleAppName]"), MOD_VERSION);
	Out.Replace(_T("[Apply]"), _GetPlainResString(IDS_PW_APPLY));

	CString m_sTestURL;
	m_sTestURL.Format(PORTTESTURL, thePrefs.GetPort(),thePrefs.GetUDPPort(), thePrefs.GetLanguageID() );
	
	// the portcheck will need to do an obfuscated callback too if obfuscation is requested, so we have to provide our userhash so it can create the key
	if (thePrefs.IsClientCryptLayerRequested())
		m_sTestURL += _T("&obfuscated_test=") + md4str(thePrefs.GetUserHash());

	Out.Replace(_T("[CONNECTIONTESTLINK]"), m_sTestURL);
	Out.Replace(_T("[CONNECTIONTESTLABEL]"), GetResString(IDS_CONNECTIONTEST)); 


	CString	sT;
	//Xman
	// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
	float n = thePrefs.GetMaxDownload();
	if(n < 0.0f || n >= UNLIMITED) n = 0.0f;
	sT.Format(_T("%0.1f"), n);
	Out.Replace(_T("[MaxDownVal]"), sT);
	n = thePrefs.GetMaxUpload();
	if(n < 0.0f || n >= UNLIMITED) n = 0.0f;
	sT.Format(_T("%.1f"), n);
	Out.Replace(_T("[MaxUpVal]"), sT);
	sT.Format(_T("%.1f"), thePrefs.GetMaxGraphDownloadRate());
	Out.Replace(_T("[MaxCapDownVal]"), sT);
	sT.Format(_T("%.1f"), thePrefs.GetMaxGraphUploadRate());
	Out.Replace(_T("[MaxCapUpVal]"), sT);
	// Maella end

	return Out;
}

CString CWebServer::_GetLoginScreen(ThreadData&Data)
{
	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString Out = pThis->m_Templates.sLogin;

	Out.Replace(_T("[CharSet]"), HTTPENCODING );
	Out.Replace(_T("[eMuleAppName]"), MOD_VERSION);
	Out.Replace(_T("[Login]"), _GetPlainResString(IDS_WEB_LOGIN));
	Out.Replace(_T("[EnterPassword]"), _GetPlainResString(IDS_WEB_ENTER_PASSWORD));
	Out.Replace(_T("[LoginNow]"), _GetPlainResString(IDS_WEB_LOGIN_NOW));
	Out.Replace(_T("[WebControl]"), _GetPlainResString(IDS_PW_WS));
	Out.Replace(_T("[FailedLogin]"), (pThis->m_nIntruderDetect >= 1)?_T("<p class=\"failed\">") + _GetPlainResString(IDS_WEB_BADLOGINATTEMPT) + _T("</p>"):_T("&nbsp;") );

	return Out;
}

bool CWebServer::_IsLoggedIn(ThreadData&Data, long lSession)
{
	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return false;

	_RemoveTimeOuts(Data);

	// find our session
	// i should have used CAtlMap there, but i like CAtlArray more ;-)
	for(size_t i = 0; i < pThis->m_Params.Sessions.GetCount(); i++)
	{
		if(pThis->m_Params.Sessions[i].lSession == lSession && lSession != 0)
		{
			// if found, also reset expiration time
			pThis->m_Params.Sessions[i].startTime = CTime::GetCurrentTime();
			return true;
		}
	}

	return false;
}

void CWebServer::_RemoveTimeOuts(ThreadData&Data)
{
	// remove expired sessions
	CWebServer *pThis = (CWebServer *)Data.pThis;
	pThis->UpdateSessionCount();
}

bool CWebServer::_RemoveSession(ThreadData&Data, long lSession)
{
	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return false;

	// find our session
	for(size_t i = 0; i < pThis->m_Params.Sessions.GetCount(); i++)
	{
		if(pThis->m_Params.Sessions[i].lSession == lSession && lSession != 0)
		{
			pThis->m_Params.Sessions.RemoveAt(i);
			AddLogLine(true, GetResString(IDS_WEB_SESSIONEND), ipstr(pThis->m_ulCurIP));
			SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION,WEBGUIIA_UPDATEMYINFO,0);
			return true;
		}
	}

	return false;
}

Session CWebServer::GetSessionByID(ThreadData&Data,long sessionID)
{
	
	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis != NULL) {
		for(size_t i = 0; i < pThis->m_Params.Sessions.GetCount(); i++)
		{
			if(pThis->m_Params.Sessions[i].lSession == sessionID && sessionID != 0)
				return pThis->m_Params.Sessions.GetAt(i);
		}
	}

	Session ses;
	ses.admin=false;
	ses.startTime = 0;

	return ses;
}

bool CWebServer::IsSessionAdmin(ThreadData&Data, const CString &strSsessionID)
{
	long sessionID = _tstol(strSsessionID);
	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis != NULL)
	{
		for(size_t i = 0; i < pThis->m_Params.Sessions.GetCount(); i++)
		{
			if(pThis->m_Params.Sessions[i].lSession == sessionID && sessionID != 0)
				return pThis->m_Params.Sessions[i].admin;
		}
	}
	
	return false;
}

CString CWebServer::_GetPlainResString(UINT nID, bool noquote)
{
	CString sRet = GetResString(nID);
	sRet.Remove(_T('&'));
	if(noquote)
	{
		sRet.Replace(_T("'"), _T("&#8217;"));
		sRet.Replace(_T("\n"), _T("\\n"));
	}
	return sRet;
}

void CWebServer::_GetPlainResString(CString *pstrOut, UINT nID, bool noquote)
{
	*pstrOut=_GetPlainResString(nID,noquote);
}

// Ornis: creating the progressbar. colored if ressources are given/available
CString CWebServer::_GetDownloadGraph(ThreadData&Data, CString filehash)
{
	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CPartFile* pPartFile;
	uchar fileid[16];
	if (filehash.GetLength()!=32 || !DecodeBase16(filehash, filehash.GetLength(), fileid, _countof(fileid)))
		return _T("");

	CString Out;
	CString progresscolor[12];

	pPartFile = theApp.downloadqueue->GetFileByID(fileid);

	if (pPartFile != NULL && (pPartFile->GetStatus() == PS_PAUSED ) )
	{
	//	Color style (paused files)
		progresscolor[0]=_T("p_green.gif");
		progresscolor[1]=_T("p_black.gif");
		progresscolor[2]=_T("p_yellow.gif");
		progresscolor[3]=_T("p_red.gif");
		progresscolor[4]=_T("p_blue1.gif");
		progresscolor[5]=_T("p_blue2.gif");
		progresscolor[6]=_T("p_blue3.gif");
		progresscolor[7]=_T("p_blue4.gif");
		progresscolor[8]=_T("p_blue5.gif");
		progresscolor[9]=_T("p_blue6.gif");
	}
	else
	{
	//	Color style (active files)
		progresscolor[0]=_T("green.gif");
		progresscolor[1]=_T("black.gif");
		progresscolor[2]=_T("yellow.gif");
		progresscolor[3]=_T("red.gif");
		progresscolor[4]=_T("blue1.gif");
		progresscolor[5]=_T("blue2.gif");
		progresscolor[6]=_T("blue3.gif");
		progresscolor[7]=_T("blue4.gif");
		progresscolor[8]=_T("blue5.gif");
		progresscolor[9]=_T("blue6.gif");
	}
	progresscolor[10]=_T("greenpercent.gif");
	progresscolor[11]=_T("transparent.gif");

	if (pPartFile == NULL || !pPartFile->IsPartFile())
	{
		Out.AppendFormat(pThis->m_Templates.sProgressbarImgsPercent, progresscolor[10], pThis->m_Templates.iProgressbarWidth);
		Out.AppendFormat(pThis->m_Templates.sProgressbarImgs,progresscolor[0], pThis->m_Templates.iProgressbarWidth);
	}
	else
	{
		CString s_ChunkBar=pPartFile->GetProgressString(pThis->m_Templates.iProgressbarWidth);

		// and now make a graph out of the array - need to be in a progressive way
		BYTE lastcolor=1;
		uint16 lastindex=0;

		int		compl = static_cast<int>((pThis->m_Templates.iProgressbarWidth / 100.0) * pPartFile->GetPercentCompleted());

		Out.AppendFormat(pThis->m_Templates.sProgressbarImgsPercent,
			progresscolor[(compl > 0) ? 10 : 11], (compl > 0) ? compl : 5);

		for (uint16 i=0;i<pThis->m_Templates.iProgressbarWidth;i++)
		{
			if (lastcolor!= charhexval(s_ChunkBar.GetAt(i)))
			{
				if (i>lastindex)
				{
					if (lastcolor < _countof(progresscolor))
						Out.AppendFormat(pThis->m_Templates.sProgressbarImgs, progresscolor[lastcolor], i-lastindex);
				}
				lastcolor=(BYTE)charhexval(s_ChunkBar.GetAt(i));
				lastindex=i;
			}
		}
		Out.AppendFormat(pThis->m_Templates.sProgressbarImgs, progresscolor[lastcolor], pThis->m_Templates.iProgressbarWidth-lastindex);
	}
	return Out;
}

CString	CWebServer::_GetSearchWnd(ThreadData&Data)
{
	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString sSession = _ParseURL(Data.sURL, _T("ses"));

	CString sMessage;
	if(IsSessionAdmin(Data,sSession)){
		CString downloads = _ParseURL(Data.sURL, _T("downloads"));
		if (!downloads.IsEmpty())
		{
			CString resToken;
			int curPos=0;
			resToken= downloads.Tokenize(_T("|"),curPos);

			size_t cat = _tstoi(_ParseURL(Data.sURL, _T("cat")));
			while (!resToken.IsEmpty())
			{
				if (resToken.GetLength()==32){
					SendMessage(theApp.emuledlg->m_hWnd,WEB_ADDDOWNLOADS, (LPARAM)(LPCTSTR)resToken, cat);
				}
				resToken= downloads.Tokenize(_T("|"),curPos);
			}
			return _T("");
		}
		if(!_ParseURL(Data.sURL, _T("tosearch")).IsEmpty()){
			// perform search
			SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION, WEBGUIIA_DELETEALLSEARCHES, 0);
			// get method
			CString method=(_ParseURL(Data.sURL, _T("method")));

			SSearchParams* pParams = new SSearchParams;
			pParams->strExpression = _ParseURL(Data.sURL, _T("tosearch"));
			pParams->strFileType = _ParseURL(Data.sURL, _T("type"));
			// for safety: this string is sent to servers and/or kad nodes, validate it!
			if (!pParams->strFileType.IsEmpty()
				&& pParams->strFileType != ED2KFTSTR_ARCHIVE
				&& pParams->strFileType != ED2KFTSTR_AUDIO
				&& pParams->strFileType != ED2KFTSTR_CDIMAGE
				&& pParams->strFileType != ED2KFTSTR_DOCUMENT
				&& pParams->strFileType != ED2KFTSTR_IMAGE
				&& pParams->strFileType != ED2KFTSTR_PROGRAM
				&& pParams->strFileType != ED2KFTSTR_VIDEO
				&& pParams->strFileType != ED2KFTSTR_EMULECOLLECTION){
				ASSERT(0);
				pParams->strFileType.Empty();
			}
			pParams->ullMinSize = _tstoi64(_ParseURL(Data.sURL, _T("min")))*1048576ui64;
			pParams->ullMaxSize = _tstoi64(_ParseURL(Data.sURL, _T("max")))*1048576ui64;
			if (pParams->ullMaxSize < pParams->ullMinSize)
				pParams->ullMaxSize = 0;
			
			pParams->uAvailability = (_ParseURL(Data.sURL, _T("avail")).IsEmpty())?0:_tstoi(_ParseURL(Data.sURL, _T("avail")));
			if (pParams->uAvailability > 1000000)
				pParams->uAvailability = 1000000;

			pParams->strExtension = _ParseURL(Data.sURL, _T("ext"));
			if (method == _T("kademlia"))
				pParams->eType = SearchTypeKademlia;
			else if (method == _T("global"))
				pParams->eType = SearchTypeEd2kGlobal;
			else
				pParams->eType = SearchTypeEd2kServer;

			LRESULT ret = SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION, WEBGUIIA_SEARCH, (LPARAM)pParams);
			if(ret != IDS_SW_SEARCHINGINFO)
				delete pParams;
			sMessage = _GetPlainResString((UINT)ret);
		}
		else
			sMessage = _GetPlainResString(IDS_SW_REFETCHRES);
	}
	else
		sMessage = _GetPlainResString(IDS_ACCESSDENIED);

	CString sCmd = _ParseURL(Data.sURL, _T("l"));
	if (sCmd == _T("sort"))
	{
		CString sSort = _ParseURL(Data.sURL, _T("sort"));
		if (sSort.GetLength()>0) pThis->m_iSearchSortby=_tstol(sSort);
		sSort = _ParseURL(Data.sURL, _T("sortAsc"));
		if (sSort.GetLength()>0) pThis->m_bSearchAsc=_tstol(sSort)!=0;
		return _GetSearchList(Data, sMessage);
	}
	sCmd = _ParseURL(Data.sURL, _T("c"));
	if (sCmd == _T("menu")){
		int iMenu = _tstoi(_ParseURL(Data.sURL, _T("m")));
		if(iMenu<_countof(WSsearchColumnHidden)){
			WSsearchColumnHidden[iMenu] = !WSsearchColumnHidden[iMenu];
			SaveWIConfigArray(WSsearchColumnHidden,_countof(WSsearchColumnHidden) ,_T("searchColumnHidden"));
		}
		return _T("");
	}

	CString Out = pThis->m_Templates.sSearch;
	
	// Search Params
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
	Out.Replace(_T("[Search]"), _GetPlainResString(IDS_EM_SEARCH));
	Out.Replace(_T("[Size]"), _GetPlainResString(IDS_DL_SIZE));
	Out.Replace(_T("[Start]"), _GetPlainResString(IDS_SW_START));

	Out.Replace(_T("[USESSERVER]"), _GetPlainResString(IDS_SERVER));
	Out.Replace(_T("[USEKADEMLIA]"), _GetPlainResString(IDS_KADEMLIA));
	Out.Replace(_T("[METHOD]"), _GetPlainResString(IDS_METHOD));

	Out.Replace(_T("[SizeMin]"), _GetPlainResString(IDS_SEARCHMINSIZE));
	Out.Replace(_T("[SizeMax]"), _GetPlainResString(IDS_SEARCHMAXSIZE));
	Out.Replace(_T("[Availabl]"), _GetPlainResString(IDS_SEARCHAVAIL));
	Out.Replace(_T("[Extention]"), _GetPlainResString(IDS_SEARCHEXTENTION));
	Out.Replace(_T("[Global]"), _GetPlainResString(IDS_SW_GLOBAL));
	Out.Replace(_T("[MB]"), _GetPlainResString(IDS_MBYTES));
	Out.Replace(_T("[Apply]"), _GetPlainResString(IDS_PW_APPLY));
	CString sCat = _ParseURL(Data.sURL, _T("cat"));
	size_t cat = _tstoi(sCat);
	if (cat != 0)
		sCat.Format(_T("%i"), cat);
	Out.Replace(_T("[CatSel]"), sCat);
	Out.Replace(_T("[Ed2klink]"), _GetPlainResString(IDS_SW_LINK));

	Out.Replace(_T("[Result]"), GetResString(IDS_SW_RESULT));
	Out.Replace(_T("[Download]"), _GetPlainResString(IDS_DOWNLOAD));

	if (thePrefs.GetCatCount()>1) 
		InsertCatBox(Out);
	else
		Out.Replace(_T("[CATBOX]"),thePrefs.GetCategory(0)->strTitle);

	CString sResult = pThis->m_Templates.sSearchResult;

	CString strTemp;
	for(uint_ptr i = 0, added = 0;i < _countof(WSsearchColumnHidden);++i){
		if(!WSsearchColumnHidden[i])
			continue;
		if(added>0)
			strTemp.AppendChar(_T(','));
		strTemp.AppendFormat(_T("%i"), i);
		++added;
	}
	sResult.Replace(_T("[ColumnHidden]"), strTemp);
	strTemp.Format(_T("%i"), pThis->m_iSearchSortby);
	sResult.Replace(_T("[SortColumn]"), strTemp);

	// Column Header
	sResult.Replace(_T("[Filename]"),		_GetPlainResString(IDS_DL_FILENAME));
	sResult.Replace(_T("[Filesize]"),		_GetPlainResString(IDS_DL_SIZE));
	sResult.Replace(_T("[Filehash]"),		_GetPlainResString(IDS_FILEHASH));
	sResult.Replace(_T("[Sources]"),		_GetPlainResString(IDS_DL_SOURCES));

	// List Content
	sResult.Replace(_T("[RESULTLIST]"), _GetSearchList(Data, sMessage));

	Out.Replace(_T("[RESULTLISTS]"), sResult);
	return Out;
}

CString	CWebServer::_GetSearchList(ThreadData&Data, CString&sMessage){
	CWebServer *pThis = (CWebServer *)Data.pThis;
	//if(pThis == NULL)
		//return _T("");

	std::vector<SearchFileStruct> SearchFileArray;
	theApp.searchlist->GetWebList(SearchFileArray);
	switch(pThis->m_iSearchSortby){
		case 0:
		case 2:
		case 4:
			std::sort(SearchFileArray.begin(), SearchFileArray.end(), CmpSearchFileByStr(pThis->m_bSearchAsc, pThis->m_iSearchSortby/2));
			break;
		case 1:
			std::sort(SearchFileArray.begin(), SearchFileArray.end(), CmpSearchFileByFileSize(pThis->m_bSearchAsc));
			break;
		case 3:
			std::sort(SearchFileArray.begin(), SearchFileArray.end(), CmpSearchFileBySourceCount(pThis->m_bSearchAsc));
			break;
	}

	uchar aFileHash[16];
	uchar nRed, nGreen, nBlue;
	CKnownFile* sameFile;
	CString result, strSourcesImage;
	CString strFontColor;
	CString strSources;
	CString strFilename;
	CString strTemp, strTemp2;

	for (size_t i = 0; i < SearchFileArray.size(); ++i){
		SearchFileStruct&structFile = SearchFileArray[i];
		nRed = nGreen = nBlue = 0;
		DecodeBase16(structFile.sortIndex[2], 32, aFileHash, _countof(aFileHash));

		if (theApp.downloadqueue->GetFileByID(aFileHash)!=NULL) {
			nBlue  = 128;
			nGreen = 128;
		} else {
			sameFile = theApp.sharedfiles->GetFileByID(aFileHash);
			if (sameFile == NULL)
				sameFile = theApp.knownfiles->FindKnownFileByID(aFileHash);

			if (sameFile != NULL)
			{
				nBlue  = 128;
				nRed = 128;
			}
		}
		strFontColor.Format(_T("%02x%02x%02x>"), nRed, nGreen, nBlue);

		if (structFile.m_uSourceCount < 5)
			strSourcesImage = _T('0');
		else if (structFile.m_uSourceCount > 4 && structFile.m_uSourceCount < 10)
			strSourcesImage = _T('5');
		else if (structFile.m_uSourceCount > 9 && structFile.m_uSourceCount < 25)
			strSourcesImage = _T("10");
		else if (structFile.m_uSourceCount > 24 && structFile.m_uSourceCount < 50)
			strSourcesImage = _T("25");
		else
			strSourcesImage = _T("50");

		strSources.Format(_T("%u(%u)"), structFile.m_uSourceCount, structFile.m_dwCompleteSourceCount);
		strFilename = structFile.sortIndex[0];
		strFilename.Replace(_T("'"),_T("\\'"));

		strTemp2.Format(_T("ed2k://|file|%s|%I64u|%s|/"),
			strFilename, structFile.m_uFileSize, structFile.sortIndex[2]);

		strTemp.Format(pThis->m_Templates.sSearchResultLine,
			strTemp2, strFontColor, strSourcesImage, GetWebImageNameForFileType(structFile.sortIndex[0]),
			(!WSsearchColumnHidden[0]) ? StringLimit(structFile.sortIndex[0],70) : _T(""),
			(!WSsearchColumnHidden[1]) ? CastItoXBytes(structFile.m_uFileSize) : _T(""),
			(!WSsearchColumnHidden[2]) ? structFile.sortIndex[2] : _T(""),
			(!WSsearchColumnHidden[3]) ? strSources : _T(""));
		result.Append(strTemp);
	}
	CString Out = pThis->m_Templates.sSearchResultFooter;
	Out.Replace(_T("[Message]"),sMessage);

	Out.Replace(_T("[SortReverse]"), pThis->m_bSearchAsc?_T("1"):_T("0"));
	result += Out;
	return result;
}

void CWebServer::UpdateSessionCount()
{
	size_t oldvalue = m_Params.Sessions.GetCount();
	for(size_t i = 0; i < m_Params.Sessions.GetCount();)
	{
		CTimeSpan ts = CTime::GetCurrentTime() - m_Params.Sessions[i].startTime;
		if(thePrefs.GetWebTimeoutMins()>0 && ts.GetTotalSeconds() > thePrefs.GetWebTimeoutMins()*60 )
			m_Params.Sessions.RemoveAt(i);
		else
			i++;
	}

	if (oldvalue != m_Params.Sessions.GetCount())
		SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION,WEBGUIIA_UPDATEMYINFO,0);

	//return m_Params.Sessions.GetCount();
}

void CWebServer::InsertCatBox(CString &Out, bool ed2kbox)
{
	CString tempBuf = _T("<select name=\"cat\">");

	for (size_t i = 0; i < thePrefs.GetCatCount(); i++)
	{
		CString strCategory = thePrefs.GetCategory(i)->strTitle;
		strCategory.Replace(_T("'"),_T("\'"));
		tempBuf.AppendFormat( _T("<option%s value=\"%i\">%s</option>\n"), (i == 0) ? _T(" selected") : _T(""), i, strCategory);
	}
	tempBuf.Append(_T("</select>") );
	Out.Replace( ed2kbox?_T("[CATBOXED2K]") : _T("[CATBOX]"), tempBuf);
}

CString CWebServer::GetSubCatLabel(size_t cat) {
	switch (cat) {
		case -1: return _GetPlainResString(IDS_ALLOTHERS);
		case -2: return _GetPlainResString(IDS_STATUS_NOTCOMPLETED);
		case -3: return _GetPlainResString(IDS_DL_TRANSFCOMPL);
		case -4: return _GetPlainResString(IDS_WAITING);
		case -5: return _GetPlainResString(IDS_DOWNLOADING);
		case -6: return _GetPlainResString(IDS_ERRORLIKE);
		case -7: return _GetPlainResString(IDS_PAUSED);
		case -8: return _GetPlainResString(IDS_SEENCOMPL);
		case -9: return _GetPlainResString(IDS_VIDEO);
		case -10: return _GetPlainResString(IDS_AUDIO);
		case -11: return _GetPlainResString(IDS_SEARCH_ARC);
		case -12: return _GetPlainResString(IDS_SEARCH_CDIMG);
		case -13: return _GetPlainResString(IDS_SEARCH_DOC);
		case -14: return _GetPlainResString(IDS_SEARCH_PICS);
		case -15: return _GetPlainResString(IDS_SEARCH_PRG);
	}
	return _CString(_T("?"));
}

CString CWebServer::_GetRemoteLinkAddedOk(ThreadData&Data)
{

	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");


    size_t cat = _tstoi(_ParseURL(Data.sURL,_T("cat")));
	CString HTTPTemp = _ParseURL(Data.sURL, _T("c"));

	const TCHAR* buf=HTTPTemp;
	theApp.emuledlg->SendMessage(WEB_ADDDOWNLOADS, (WPARAM)buf, cat);

	CString Out = _T("<status result=\"OK\">");
    Out += _T("<description>") + GetResString(IDS_WEB_REMOTE_LINK_ADDED) + _T("</description>");
    Out += _T("<filename>") + HTTPTemp + _T("</filename>");
    Out += _T("</status>");

	return Out;
}
CString CWebServer::_GetRemoteLinkAddedFailed(ThreadData&Data)
{

	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString Out = _T("<status result=\"FAILED\" reason=\"WRONG_PASSWORD\">");
    Out += _T("<description>") + GetResString(IDS_WEB_REMOTE_LINK_NOT_ADDED) + _T("</description>");
    Out += _T("</status>");

	return Out;
}

void CWebServer::_SetLastUserCat(ThreadData&Data, long lSession, size_t cat){
	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return;

	_RemoveTimeOuts(Data);

	// find our session
	for(size_t i = 0; i < pThis->m_Params.Sessions.GetCount(); i++)
	{
		if(pThis->m_Params.Sessions[i].lSession == lSession && lSession != 0)
		{
			// if found, also reset expiration time
			pThis->m_Params.Sessions[i].startTime = CTime::GetCurrentTime();
			if(cat<0)
				pThis->m_Params.Sessions[i].lastfilter=cat;
			else{
				pThis->m_Params.Sessions[i].lastcat=cat;
				pThis->m_Params.Sessions[i].lastfilter=0;
			}
			return;
		}
	}
}

void CWebServer::_GetLastUserCat(ThreadData&Data, long lSession, size_t&lastcat, sint_ptr&lastfilter)
{
	CWebServer *pThis = (CWebServer *)Data.pThis;
	if(pThis == NULL)
		return;

	_RemoveTimeOuts(Data);

	// find our session
	for(size_t i = 0; i < pThis->m_Params.Sessions.GetCount(); i++)
	{
		if(pThis->m_Params.Sessions[i].lSession == lSession && lSession != 0)
		{
			// if found, also reset expiration time
			pThis->m_Params.Sessions[i].startTime = CTime::GetCurrentTime();
			lastcat = pThis->m_Params.Sessions[i].lastcat;
			lastfilter = pThis->m_Params.Sessions[i].lastfilter;
			return;
		}
	}

	return;
}

uchar* CWebServer::_GetFileHash(CString sHash, uchar *FileHash)
{
	wcsmd4(sHash, FileHash);
	return FileHash;
}

CString CWebServer::GetWebImageNameForFileType(CString filename)
{
	switch (GetED2KFileTypeID(filename)) {
		case ED2KFT_AUDIO:		return _CString(_T("audio"));
		case ED2KFT_VIDEO:		return _CString(_T("video"));
		case ED2KFT_IMAGE:		return _CString(_T("picture"));
		case ED2KFT_PROGRAM:	return _CString(_T("program"));
		case ED2KFT_DOCUMENT:	return _CString(_T("document"));
		case ED2KFT_ARCHIVE:	return _CString(_T("archive"));
		case ED2KFT_CDIMAGE:	return _CString(_T("cdimage"));
		case ED2KFT_EMULECOLLECTION: return _CString(_T("emulecollection"));

		default: /*ED2KFT_ANY:*/ return _CString(_T("other"));
	}
}

CString CWebServer::GetClientSummary(CUpDownClient* client) {

	// name
	CString buffer=	GetResString(IDS_CD_UNAME) + _T(' ') + client->GetUserName() + _T('\n');
	// client version
	buffer+= GetResString(IDS_CD_CSOFT)+ _T(": ") + client->GetClientSoftVer() + _T('\n');
	
	// uploading file
	buffer+= GetResString(IDS_CD_UPLOADREQ) + _T(' ');
	CKnownFile* file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID() );
	ASSERT(file);
	if (file) {
		buffer += file->GetFileName();
	}
	buffer+= _T("\n\n");
	

	// transfering time
	buffer+= GetResString(IDS_UPLOADTIME) + _T(": ") + CastSecondsToHM(client->GetUpStartTimeDelay()/1000) + _T('\n');

	// transfered data (up,down,global,session)
	buffer+= GetResString(IDS_FD_TRANS) + _T(" (") +GetResString(IDS_STATS_SESSION) + _T("):\n");
	buffer+= _T(".....") + GetResString(IDS_PW_CON_UPLBL) +  _T(": ")+ CastItoXBytes(client->GetTransferredUp()) + _T(" (") + CastItoXBytes(client->GetSessionUp()) + _T(" )\n");
	buffer+= _T(".....") + GetResString(IDS_DOWNLOAD) +  _T(": ") +    CastItoXBytes(client->GetTransferredDown()) + _T(" (") + CastItoXBytes(client->GetSessionDown()) + _T(" )\n");

	return buffer;
}

CString CWebServer::GetClientversionImage(CUpDownClient* client)
{
	switch(client->GetClientSoft()) {
		case SO_EMULE:
		case SO_OLDEMULE:		return _CString(_T("1"));
		case SO_EDONKEY:		return _CString(_T("0"));
		case SO_EDONKEYHYBRID:	return _CString(_T("h"));
		case SO_AMULE:			return _CString(_T("a"));
		case SO_SHAREAZA:		return _CString(_T("s"));
		case SO_MLDONKEY:		return _CString(_T("m"));
		case SO_LPHANT:			return _CString(_T("l"));
		case SO_URL:			return _CString(_T("u"));
	}

	return _CString(_T("0"));
}

CString CWebServer::_GetCommentlist(ThreadData&Data)
{
	CWebServer *pThis = (CWebServer *)Data.pThis;

	uchar FileHash[16];
	CPartFile* pPartFile=theApp.downloadqueue->GetFileByID(_GetFileHash(_ParseURL(Data.sURL, _T("filehash")),FileHash) );
	if (!pPartFile)
		return _T("");

	CString Out = pThis->m_Templates.sCommentList;
	Out.Replace(_T("[COMMENTS]"), GetResString(IDS_COMMENT) + _T(": ") + pPartFile->GetFileName() );

	CString commentlines;
	// prepare commentsinfo-string
	for (POSITION pos = pPartFile->srclist.GetHeadPosition(); pos != NULL; )
	{ 
		CUpDownClient* cur_src = pPartFile->srclist.GetNext(pos);
		if (cur_src->HasFileRating() || !cur_src->GetFileComment().IsEmpty())
		{
			commentlines.AppendFormat( pThis->m_Templates.sCommentListLine,
				_SpecialChars(cur_src->GetUserName()),
				_SpecialChars(cur_src->GetClientFilename()),
				_SpecialChars(cur_src->GetFileComment()),
				_SpecialChars(GetRateString(cur_src->GetFileRating()))
				);
		} 
	} 

	const CAtlList<Kademlia::CEntry*>& list = pPartFile->getNotes();
	for(POSITION pos = list.GetHeadPosition(); pos != NULL; )
	{
		Kademlia::CEntry* entry = list.GetNext(pos);

		commentlines.AppendFormat( pThis->m_Templates.sCommentListLine,
			_T(""),
			_SpecialChars(entry->GetCommonFileName()),
			_SpecialChars(entry->GetStrTagValue(TAG_DESCRIPTION)),
			_SpecialChars(GetRateString((UINT)entry->GetIntTagValue(TAG_FILERATING)) )
			);
	}
	
	Out.Replace(_T("[COMMENTLINES]"), commentlines);
	Out.Replace(_T("[USERNAME]"), GetResString(IDS_QL_USERNAME));
	Out.Replace(_T("[FILENAME]"), GetResString(IDS_DL_FILENAME));
	Out.Replace(_T("[COMMENT]"), GetResString(IDS_COMMENT));
	Out.Replace(_T("[RATING]"), GetResString(IDS_QL_RATING));
	Out.Replace(_T("[CLOSE]"), GetResString(IDS_CW_CLOSE));
	Out.Replace(_T("[CharSet]"), HTTPENCODING);
	return Out;
}
