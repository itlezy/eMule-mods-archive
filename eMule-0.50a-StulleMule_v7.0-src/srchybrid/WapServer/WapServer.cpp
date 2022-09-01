//emulEspaña - Copied from WebServer.cpp and then modified by MoNKi [MoNKi: -Wap Server-]
#include "StdAfx.h"
#include "emule.h"
#include "OtherFunctions.h"
#include "SearchDlg.h"
#include "SearchParams.h"
#include "WapServer.h"
#include "ED2KLink.h"
#include "MD5Sum.h"
#include <stdlib.h>
#include "SearchList.h"
#include <locale.h>
#include "HTRichEditCtrl.h"
#include "KademliaWnd.h"
#include "KadContactListCtrl.h"
#include "KadSearchListCtrl.h"
#include "UploadQueue.h"
#include "DownloadQueue.h"
#include "WebSocket.h"
#include "ServerList.h"
#include "SharedFileList.h"
#include "emuledlg.h"
#include "ServerWnd.h"
#include "Sockets.h"
#include "Server.h"
#include "TransferDlg.h"
#include "PartFile.h"
#include "UpDownClient.h"
#include "StatisticsDlg.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "Kademlia/Net/KademliaUDPListener.h"
#include "Exceptions.h"
#include "Log.h"
#include "UserMsgs.h"
// emulEspaña: added by Announ [Maella: -Allow Bandwidth Settings in <1KB Incremements-]
#include "opcodes.h"
// End emulEspaña
#include "Scheduler.h" //MORPH - Added by Stulle, Don't reset Connection Settings without reason

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define WMLInit _T("Server: eMule\r\nConnection: close\r\nContent-Type: text/vnd.wap.wml\r\n")

#define WAP_SERVER_TEMPLATES_VERSION	2

CWapServer::CWapServer(void)
{
	m_Params.bShowUploadQueue = false;

	m_Params.DownloadSort = DOWN_SORT_NAME;
	m_Params.bDownloadSortReverse = false;
	m_Params.ServerSort = SERVER_SORT_NAME;
	m_Params.bServerSortReverse = false;
	m_Params.SharedSort = SHARED_SORT_NAME;
	m_Params.bSharedSortReverse = false;

	m_Params.sLastModified=_T("");
	m_Params.sETag=_T("");
	m_iSearchSortby=3;
	m_bSearchAsc=0;

	m_bServerWorking = false;
}

CWapServer::~CWapServer(void)
{
	if (m_bServerWorking) StopWapSockets();
}

void CWapServer::RestartServer() {	//Cax2 - restarts the server with the new port settings
	StopWapSockets();
	if (m_bServerWorking)	
		StartWapSockets(this);
}

CString CWapServer::_LoadTemplate(CString sAll, CString sTemplateName)
{
	CString sRet = _T("");
	int nStart = sAll.Find(_T("<--") + sTemplateName + _T("-->"));
	int nEnd = sAll.Find(_T("<--") + sTemplateName + _T("_END-->"));
	if(nStart != -1 && nEnd != -1 && nStart<nEnd)
	{
		nStart += sTemplateName.GetLength() + 7;
		sRet = sAll.Mid(nStart, nEnd - nStart - 1);
	} else{
		if (sTemplateName==_T("TMPL_VERSION"))
			AddLogLine(true,GetResString(IDS_WS_ERR_LOADTEMPLATE),sTemplateName);
		if (nStart==-1) AddDebugLogLine(false,GetResString(IDS_WEB_ERR_CANTLOAD),sTemplateName);
	}

	return sRet;
}

void CWapServer::ReloadTemplates()
{
	CString sPrevLocale(setlocale(LC_TIME,NULL));
	_tsetlocale(LC_TIME, _T("English"));
	CTime t = GetCurrentTime();
	m_Params.sLastModified = t.FormatGmt(_T("%a, %d %b %Y %H:%M:%S GMT"));
	m_Params.sETag = MD5Sum(m_Params.sLastModified).GetHash();
	_tsetlocale(LC_TIME, sPrevLocale);

	CString sFile;
	if (thePrefs.GetWapTemplate()==_T("") || thePrefs.GetWapTemplate().MakeLower()==_T("emule_wap.tmpl"))
		sFile= thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR) + CString(_T("eMule_Wap.tmpl"));
	else sFile=thePrefs.GetWapTemplate();

	CStdioFile file;
	if(file.Open(sFile, CFile::modeRead|CFile::typeText))
	{
		CString sAll;
		for(;;)
		{
			CString sLine;
			if(!file.ReadString(sLine))
				break;

			sAll += sLine + _T("\n");
		}
		file.Close();

		CString sVersion = _LoadTemplate(sAll,_T("TMPL_VERSION"));
		long lVersion = _ttol(sVersion);
		if(lVersion < WAP_SERVER_TEMPLATES_VERSION)
		{
			if(m_bServerWorking)
				AddLogLine(true,GetResString(IDS_WS_ERR_LOADTEMPLATE),sFile);
		}
		else
		{
			m_Templates.sScriptFile = _LoadTemplate(sAll,_T("TMPL_WMLSCRIPT"));
			m_Templates.sMain = _LoadTemplate(sAll,_T("TMPL_MAIN"));
			m_Templates.sTransferDownLineLite = _LoadTemplate(sAll,_T("TMPL_TRANSFER_DOWN_LINE_LITE"));
			m_Templates.sTransferDownLineLiteGood = _LoadTemplate(sAll,_T("TMPL_TRANSFER_DOWN_LINE_LITE_GOOD"));
			m_Templates.sTransferDownFileDetails = _LoadTemplate(sAll,_T("TMPL_TRANSFER_DOWN_FILE_DETAILS"));
			m_Templates.sServerList = _LoadTemplate(sAll,_T("TMPL_SERVER_LIST"));
			m_Templates.sServerMain = _LoadTemplate(sAll,_T("TMPL_SERVER_MAIN"));
			m_Templates.sServerLine = _LoadTemplate(sAll,_T("TMPL_SERVER_LINE"));
			m_Templates.sTransferDownList = _LoadTemplate(sAll,_T("TMPL_TRANSFER_LIST"));
			m_Templates.sTransferUpList = _LoadTemplate(sAll,_T("TMPL_TRANSFER_UP_LIST"));
			m_Templates.sTransferUpLine = _LoadTemplate(sAll,_T("TMPL_TRANSFER_UP_LINE"));
			m_Templates.sTransferUpQueueList = _LoadTemplate(sAll,_T("TMPL_TRANSFER_UP_QUEUE_LIST"));
			m_Templates.sTransferUpQueueLine = _LoadTemplate(sAll,_T("TMPL_TRANSFER_UP_QUEUE_LINE"));
			m_Templates.sDownloadLink = _LoadTemplate(sAll,_T("TMPL_DOWNLOAD_LINK"));
			m_Templates.sSharedList = _LoadTemplate(sAll,_T("TMPL_SHARED_LIST"));
			m_Templates.sSharedLine = _LoadTemplate(sAll,_T("TMPL_SHARED_LINE"));
			m_Templates.sSharedLineChanged = _LoadTemplate(sAll,_T("TMPL_SHARED_LINE_CHANGED"));
			m_Templates.sGraphs = _LoadTemplate(sAll,_T("TMPL_GRAPHS"));
			m_Templates.sLog = _LoadTemplate(sAll,_T("TMPL_LOG"));
			m_Templates.sServerInfo = _LoadTemplate(sAll,_T("TMPL_SERVERINFO"));
			m_Templates.sDebugLog = _LoadTemplate(sAll,_T("TMPL_DEBUGLOG"));
			m_Templates.sStats = _LoadTemplate(sAll,_T("TMPL_STATS"));
			m_Templates.sPreferences = _LoadTemplate(sAll,_T("TMPL_PREFERENCES"));
			m_Templates.sLogin = _LoadTemplate(sAll,_T("TMPL_LOGIN"));
			m_Templates.sConnectedServer = _LoadTemplate(sAll,_T("TMPL_CONNECTED_SERVER"));
			m_Templates.sServerOptions = _LoadTemplate(sAll,_T("TMPL_SERVER_OPTIONS"));
			m_Templates.sSearchMain = _LoadTemplate(sAll,_T("TMPL_SEARCH_MAIN"));
			m_Templates.sSearchList = _LoadTemplate(sAll,_T("TMPL_SEARCH_LIST"));
			m_Templates.sSearchResultLine = _LoadTemplate(sAll,_T("TMPL_SEARCH_RESULT_LINE"));			
			m_Templates.sClearCompleted = _LoadTemplate(sAll,_T("TMPL_TRANSFER_DOWN_CLEARBUTTON"));
			m_Templates.sBootstrapLine= _LoadTemplate(sAll,_T("TMPL_BOOTSTRAPLINE"));
			m_Templates.sKad= _LoadTemplate(sAll,_T("TMPL_KADDLG"));
			
			CString tmpColor;
			tmpColor.Format(_T("0x00%s"),_LoadTemplate(sAll,_T("TMPL_DOWNLOAD_GRAPHCOLOR")));
			m_Templates.cDownColor = _tcstol(tmpColor,NULL,16);
			tmpColor.Format(_T("0x00%s"),_LoadTemplate(sAll,_T("TMPL_UPLOAD_GRAPHCOLOR")));
			m_Templates.cUpColor = _tcstol(tmpColor,NULL,16);
			tmpColor.Format(_T("0x00%s"),_LoadTemplate(sAll,_T("TMPL_CONNECTIONS_GRAPHCOLOR")));
			m_Templates.cConnectionsColor = _tcstol(tmpColor,NULL,16);
		}
	}
	else
        if(m_bServerWorking)
			AddLogLine(true,GetResString(IDS_WEB_ERR_CANTLOAD), sFile);

}

void CWapServer::StartServer(void)
{
	if(m_bServerWorking != thePrefs.GetWapServerEnabled())
		m_bServerWorking = thePrefs.GetWapServerEnabled();
	else
		return;

	if (m_bServerWorking) {
		ReloadTemplates();
		StartWapSockets(this);
	} else StopWapSockets();

	if(thePrefs.GetWapServerEnabled())
		AddLogLine(false,_T("%s: %s"),_GetPlainResString(IDS_PW_WAP), _GetPlainResString(IDS_ENABLED));
	else
		AddLogLine(false,_T("%s: %s"),_GetPlainResString(IDS_PW_WAP), _GetPlainResString(IDS_DISABLED));
}

void CWapServer::_RemoveServer(CString sIP, int nPort)
{
	CServer* server=theApp.serverlist->GetServerByAddress(sIP, (uint16)nPort);
	if (server!=NULL)
		SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION, WEBGUIIA_SERVER_REMOVE, (LPARAM)server);
}

void CWapServer::_SetSharedFilePriority(CString hash, uint8 priority)
{	
	CKnownFile* cur_file;
	uchar fileid[16];
	if (hash.GetLength()!=32 || !DecodeBase16(hash, hash.GetLength(), fileid,ARRSIZE(fileid)))
		return;

	cur_file=theApp.sharedfiles->GetFileByID(fileid);
	
	if (cur_file==0) return;

	if(priority >= 0 && priority < 5)
	{
		cur_file->SetAutoUpPriority(false);
		cur_file->SetUpPriority(priority);
	}
	else if(priority == 5)// && cur_file->IsPartFile())
	{
		//MORPH START - Added by SiRoB, force savepart to update auto up flag since i removed the update in UpdateAutoUpPriority optimization
		if(cur_file->IsPartFile() &&  !cur_file->IsAutoUpPriority()){
			cur_file->SetAutoUpPriority(true);
			((CPartFile*)cur_file)->SavePartFile();
		}else
			cur_file->SetAutoUpPriority(true);
		//MORPH END   - Added by SiRoB, force savepart to update auto up flag since i removed the update in UpdateAutoUpPriority optimization
		cur_file->UpdateAutoUpPriority(); 
	}
}

void CWapServer::AddStatsLine(UpDown line)
{
	m_Params.PointsForWeb.Add(line);
	if(m_Params.PointsForWeb.GetCount() > thePrefs.GetWapGraphWidth())
		m_Params.PointsForWeb.RemoveAt(0);
}

CString CWapServer::_SpecialChars(CString str) 
{
	str.Replace(_T("&"),_T("&amp;"));
	str.Replace(_T("<"),_T("&lt;"));
	str.Replace(_T(">"),_T("&gt;"));
	str.Replace(_T("\""),_T("&quot;"));
	str.Replace(_T("'"),_T("&#39;"));
	str.Replace(_T("$"),_T("$$"));

	return str;
}

void CWapServer::_ConnectToServer(CString sIP, int nPort)
{
	CServer* server=NULL;
	if (!sIP.IsEmpty()) server=theApp.serverlist->GetServerByAddress(sIP, (uint16)nPort);
	SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION, WEBGUIIA_CONNECTTOSERVER, (LPARAM)server);
}

void CWapServer::ProcessFileReq(WapThreadData Data) {
	CWapServer *pThis = (CWapServer *)Data.pThis;
	if (pThis == NULL) return;

	CString filename=Data.sURL;
	CString contenttype;

	filename.Replace(_T('/'),_T('\\'));

	if (filename.GetAt(0)==_T('\\')) filename.Delete(0);

	filename=thePrefs.GetMuleDirectory(EMULE_WAPSERVERDIR)+filename;

	if(filename.Right(11).CompareNoCase(_T("script.wmls"))==0){
		SendScriptFile(Data);
		return;
	}

	if(filename.Right(5).MakeLower()==_T(".wbmp") ||
	   filename.Right(4).MakeLower()==_T(".png") ||
	   filename.Right(4).MakeLower()==_T(".gif")){
		SendImageFile(Data,filename);
		return;
	}

	if (filename.Right(5).MakeLower()==_T(".wbmp")) contenttype=_T("Content-Type: image/vnd.wap.wbmp\r\n");
	  else if (filename.Right(5).MakeLower()==_T(".wmls")) contenttype=_T("Content-Type: text/vnd.wap.wmlscript\r\n");

	contenttype += _T("Last-Modified: ") + pThis->m_Params.sLastModified + _T("\r\n") + _T("ETag: ") + pThis->m_Params.sETag + _T("\r\n");

	if(!SendFile(Data,filename,contenttype))
		Data.pSocket->SendError404(true);
}

void CWapServer::ProcessURL(WapThreadData Data)
{
	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis == NULL)
		return;

	//SetThreadLocale(thePrefs.GetLanguageID());

	//////////////////////////////////////////////////////////////////////////
	// Here we are in real trouble! We are accessing the entire emule main thread
	// data without any syncronization!! Either we use the message pump for emuledlg
	// or use some hundreds of critical sections... For now, an exception handler
	// shoul avoid the worse things.
	//////////////////////////////////////////////////////////////////////////
	CoInitialize(NULL);

	try{
		CString Out = _T("");
		CString OutE = _T("");	// List Entry Templates
		CString OutE2 = _T("");

		CString HTTPProcessData = _T("");
		CString HTTPTemp = _T("");
		//char	HTTPTempC[100] = "";
		srand ( time(NULL) );

		long lSession = 0;

		if(_ParseURL(Data.sURL, _T("ses")) != _T(""))
			lSession = _tstol(_ParseURL(Data.sURL, _T("ses")));

		if (_ParseURL(Data.sURL, _T("w")) == _T("password"))
		{
			CString test=MD5Sum(_ParseURL(Data.sURL, _T("p"))).GetHash();
			bool login=false;
			CString ip=ipstr( Data.inadr );

			if(MD5Sum(_ParseURL(Data.sURL, _T("p"))).GetHash() == thePrefs.GetWapPass())
			{
				Session ses;
				ses.admin=true;
				ses.startTime = CTime::GetCurrentTime();
				ses.lSession = lSession = rand() * 10000L + rand();
				pThis->m_Params.Sessions.Add(ses);
				theApp.emuledlg->serverwnd->UpdateMyInfo();
				AddLogLine(true,GetResString(IDS_WAP_ADMINLOGIN)+_T(" (%s)"),ip);
				login=true;
			}
			else if(thePrefs.GetWapIsLowUserEnabled() && thePrefs.GetWapLowPass()!=_T("") && MD5Sum(_ParseURL(Data.sURL, _T("p"))).GetHash() == thePrefs.GetWapLowPass())
			{
				Session ses;
				ses.admin=false;
				ses.startTime = CTime::GetCurrentTime();
				ses.lSession = lSession = rand() * 10000L + rand();
				pThis->m_Params.Sessions.Add(ses);
				theApp.emuledlg->serverwnd->UpdateMyInfo();
				AddLogLine(true,GetResString(IDS_WAP_GUESTLOGIN)+_T(" (%s)"),ip);
				login=true;
			} else {
				AddLogLine(true,GetResString(IDS_WAP_BADLOGINATTEMPT)+_T(" (%s)"),ip);
				// TODO need to change htis. BadLogin newban={inet_addr(T2CA(ip)), ::GetTickCount()};	// save failed attempt (ip,time)
				// TODO pThis->m_Params.badlogins.Add(newban);
				login=false;
			}

			if (login) {
				uint32 ipn=inet_addr(CT2CA(ip)) ;
				for(int i = 0; i < pThis->m_Params.badlogins.GetSize();)
				{
					if (ipn == pThis->m_Params.badlogins[i].datalen) {
						pThis->m_Params.badlogins.RemoveAt(i);
					}
					else
						i++;
				}
			}
		}

		CString sSession; sSession.Format(_T("%ld"), lSession);

		if (_ParseURL(Data.sURL, _T("w")) == _T("logout")) 
			_RemoveSession(Data, lSession);

		if(_IsLoggedIn(Data, lSession))
		{
			// Check if graph image request
			CString filename = _ParseURL(Data.sURL, _T("graphimage"));
			if (filename.CompareNoCase(_T("download"))==0){
				SendGraphFile(Data, 1);
				return;
			}else if (filename.CompareNoCase(_T("upload"))==0){
				SendGraphFile(Data, 2);
				return;
			}else if (filename.CompareNoCase(_T("connections"))==0){
				SendGraphFile(Data, 3);
				return;
			}

			// Check if progressbar image request
			if(_ParseURL(Data.sURL, _T("progressbar"))!=_T("")){
				CString strFileHash = _ParseURL(Data.sURL, _T("progressbar"));
				SendProgressBar(Data, strFileHash);
				return;
			}

			CString sPage = _ParseURL(Data.sURL, _T("w"));
			if (sPage == _T("server")) 
			{
				Out += _GetServerList(Data);
			}
			else
			if (sPage == _T("download")) 
			{
				Out += _GetDownloadLink(Data);
			}
			else
			if (sPage == _T("kad")) 
			{
				Out += _GetKadPage(Data);
			}
			else
			if (sPage == _T("shared")) 
			{ 
				Out += _GetSharedFilesList(Data);
			}
			else
			if (sPage == _T("transferdown")) 
			{
				Out += _GetTransferDownList(Data);
			}
			else
			if (sPage == _T("transferup")) 
			{
				Out += _GetTransferUpList(Data);
			}
			else
			if (sPage == _T("transferqueue")) 
			{
				Out += _GetTransferQueueList(Data);
			}
			else
			/*if (sPage == "websearch") 
			{
				Out += _GetWebSearch(Data);
			}
			else*/
			if (sPage == _T("search")) 
			{
				Out += _GetSearch(Data);
			}
			else
			if (sPage == _T("graphs"))
			{
				Out += _GetGraphs(Data);
			}
			else
			if (sPage == _T("log")) 
			{
				Out += _GetLog(Data);
			}else
			if (sPage == _T("sinfo")) 
			{
				Out += _GetServerInfo(Data);
			}else
			if (sPage == _T("debuglog")) 
			{
				Out += _GetDebugLog(Data);
			}else
			if (sPage == _T("stats")) 
			{
				Out += _GetStats(Data);
			}else
			if (sPage == _T("options")) 
			{
				Out += _GetPreferences(Data);
			}else
				Out += _GetMain(Data, lSession);
		}
		else 
		{
			uint32 ip= inet_addr(CT2CA(ipstr( Data.inadr )));
			uint32 faults=0;

			// check for bans
			for(int i = 0; i < pThis->m_Params.badlogins.GetSize();i++)
				if ( pThis->m_Params.badlogins[i].datalen==ip ) faults++;

			if (faults>4) {
				Out +=_T("<?xml version=\"1.0\" encoding=\"")+ _GetWebCharSet() +_T("\"?>");
				Out +=_T("<!DOCTYPE wml PUBLIC \"-//WAPFORUM//DTD WML 1.1//EN\" \"http://www.wapforum.org/DTD/wml_1.1.xml\">");
				Out +=_T("<wml><card><p>");
				Out += _GetPlainResString(IDS_ACCESSDENIED);
				Out +=_T("</p></card></wml>");
				
				// set 15 mins ban by using the badlist
				/* TODO need to change thie
				BadLogin preventive={ip, ::GetTickCount() + (15*60*1000) };
				for (int i=0;i<=5;i++)
					pThis->m_Params.badlogins.Add(preventive);
				*/

			}
			else
				Out += _GetLoginScreen(Data);
		}
		
		Out.Replace(_T("[CharSet]"), _GetWebCharSet());
		// send answer ...
		Data.pSocket->SendContent(CT2CA(WMLInit), Out);
	}
	catch(...){
		TRACE(_T("*** Unknown exception in CWapServer::ProcessURL\n"));
		ASSERT(0);
	}

	CoUninitialize();
}

CString CWapServer::_ParseURLArray(CString URL, CString fieldname) {
	CString res,temp;

	while (URL.GetLength()>0) {
		int pos=URL.MakeLower().Find(fieldname.MakeLower() +_T("="));
		if (pos>-1) {
			temp=_ParseURL(URL,fieldname);
			if (temp==_T("")) break;
			res.Append(temp+_T("|"));
			URL.Delete(pos,10);
		} else break;
	}
	return res;
}

CString CWapServer::_ParseURL(CString URL, CString fieldname)
{

	CString value = _T("");
	CString Parameter = _T("");
	TCHAR fromReplace[4] = _T("");	// decode URL
	TCHAR toReplace[2] = _T("");		// decode URL
	int i = 0;
	int findPos = -1;
	int findLength = 0;

	if (URL.Find(_T("?")) > -1) {
		Parameter = URL.Mid(URL.Find(_T("?"))+1, URL.GetLength()-URL.Find(_T("?"))-1);

		// search the fieldname beginning / middle and strip the rest...
		if (Parameter.Find(fieldname + _T("=")) == 0) {
			findPos = 0;
			findLength = fieldname.GetLength() + 1;
		}
		if (Parameter.Find(_T("&") + fieldname + _T("=")) > -1) {
			findPos = Parameter.Find(_T("&") + fieldname + _T("="));
			findLength = fieldname.GetLength() + 2;
		}
		if (findPos > -1) {
			Parameter = Parameter.Mid(findPos + findLength, Parameter.GetLength());
			if (Parameter.Find(_T("&")) > -1) {
				Parameter = Parameter.Mid(0, Parameter.Find(_T("&")));
			}
	
			value = Parameter;

			// decode value ...
			value.Replace(_T("+"), _T(" "));
			for (i = 0 ; i <= 255 ; i++) {
				_stprintf(fromReplace, _T("%%%02x"), i);
				toReplace[0] = (char)i;
				toReplace[1] = NULL;
				value.Replace(fromReplace, toReplace);
				_stprintf(fromReplace, _T("%%%02X"), i);
				value.Replace(fromReplace, toReplace);
			}
		}
	}

#ifdef _UNICODE
	CStringA strValueA;
	LPSTR pszA = strValueA.GetBuffer(value.GetLength());
	for (int i = 0; i < value.GetLength(); i++)
		*pszA++ = (CHAR)value[i];
	strValueA.ReleaseBuffer(value.GetLength());
	value = strValueA;
#endif

	return value;
}

CString CWapServer::_GetMain(WapThreadData Data, long lSession)
{
	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString sSession; sSession.Format(_T("%ld"), lSession);

	CString	Out = pThis->m_Templates.sMain;

	Out.Replace(_T("[Session]"), sSession);
	Out.Replace(_T("[eMuleAppName]"), _T("eMule"));
	//MORPH START - Changed by SiRoB, [itsonlyme: -modname-]
	/*
	Out.Replace(_T("[version]"), _SpecialChars(theApp.m_strCurVersionLong));
	*/
	Out.Replace(_T("[version]"), _SpecialChars(theApp.m_strCurVersionLong + _T(" [") + theApp.m_strModLongVersion + _T("]")));
	//MORPH END   - Changed by SiRoB, [itsonlyme: -modname-]
	Out.Replace(_T("[WapControl]"), _GetPlainResString(IDS_WAP_CONTROL));
	Out.Replace(_T("[Transfer]"), _GetPlainResString(IDS_CD_TRANS));
	Out.Replace(_T("[Server]"), _GetPlainResString(IDS_SV_SERVERLIST));
	Out.Replace(_T("[Shared]"), _GetPlainResString(IDS_SHAREDFILES));
	Out.Replace(_T("[Download]"), _GetPlainResString(IDS_SW_LINK));
	Out.Replace(_T("[Graphs]"), _GetPlainResString(IDS_GRAPHS));
	Out.Replace(_T("[Log]"), _GetPlainResString(IDS_SV_LOG));
	Out.Replace(_T("[ServerInfo]"), _GetPlainResString(IDS_SV_SERVERINFO));
	Out.Replace(_T("[DebugLog]"), _GetPlainResString(IDS_SV_DEBUGLOG));
	Out.Replace(_T("[Stats]"), _GetPlainResString(IDS_SF_STATISTICS));
	Out.Replace(_T("[Options]"), _GetPlainResString(IDS_EM_PREFS));
	Out.Replace(_T("[Logout]"), _GetPlainResString(IDS_WEB_LOGOUT));
	Out.Replace(_T("[Search]"), _GetPlainResString(IDS_SW_SEARCHBOX));
	Out.Replace(_T("[TransferDown]"), _GetPlainResString(IDS_TW_DOWNLOADS));
	Out.Replace(_T("[TransferUp]"), _GetPlainResString(IDS_PW_CON_UPLBL));
	Out.Replace(_T("[TransferQueue]"), _GetPlainResString(IDS_ONQUEUE));
	Out.Replace(_T("[BasicData]"), _GetPlainResString(IDS_WAP_BASICDATA));
	Out.Replace(_T("[Home]"), _GetPlainResString(IDS_WAP_HOME));
	Out.Replace(_T("[Back]"), _GetPlainResString(IDS_WAP_BACK));
	Out.Replace(_T("[SearchResults]"), _GetPlainResString(IDS_SW_RESULT));
	Out.Replace(_T("[Kad]"), _GetPlainResString(IDS_KADEMLIA));

	TCHAR HTTPTempC[100] = _T("");
	CString sConnected = _T("");
	if (theApp.IsConnected()) 
	{
		if(!theApp.IsFirewalled())
			sConnected = _GetPlainResString(IDS_CONNECTED);
		else
			sConnected = _GetPlainResString(IDS_CONNECTED) + _T(" (") + _GetPlainResString(IDS_IDLOW) + _T(")");

		if( theApp.serverconnect->IsConnected()){
			CServer* cur_server = theApp.serverconnect->GetCurrentServer();
			sConnected += _T(": ") + _SpecialChars(cur_server->GetListName());
	
			_stprintf(HTTPTempC, _T("%i "), cur_server->GetUsers());
			sConnected += _T(" [") + CString(HTTPTempC) + _GetPlainResString(IDS_UUSERS) + _T("]");
		}
	} 
	else if (theApp.serverconnect->IsConnecting())
	{
		sConnected = _GetPlainResString(IDS_CONNECTING);
	}
	else
	{
		sConnected = _GetPlainResString(IDS_DISCONNECTED);
		
		if (IsSessionAdmin(Data,sSession)){
			sConnected+=_T(" (<a href=\"./?ses=") + sSession + _T("&amp;w=server&amp;c=connect\">") + _GetPlainResString(IDS_CONNECTTOANYSERVER)+_T("</a>)");
		}
	}
	Out.Replace(_T("[Connected]"), _T("<b>")+_GetPlainResString(IDS_PW_CONNECTION)+_T(":</b> ")+sConnected);
    _stprintf(HTTPTempC, _GetPlainResString(IDS_UPDOWNSMALL),(float)theApp.uploadqueue->GetDatarate()/1024,(float)theApp.downloadqueue->GetDatarate()/1024);
	CString sLimits;
	// EC 25-12-2003
	CString MaxUpload;
	CString MaxDownload;

	// emulEspaña: modified by MoNKi [Maella: -Allow Bandwidth Settings in <1KB Incremements-]
	/**/ //MORPH - Don't use <1KB Increment just change this /**/ into /* to toggle :)
	MaxUpload.Format(_T("%i"),thePrefs.GetMaxUpload());
	MaxDownload.Format(_T("%i"),thePrefs.GetMaxDownload());
	if (MaxUpload == _T("65535"))  MaxUpload = GetResString(IDS_PW_UNLIMITED);
	if (MaxDownload == _T("65535")) MaxDownload = GetResString(IDS_PW_UNLIMITED);
	/*/
	if ( CPreferences::GetMaxUpload() < UNLIMITED )
		MaxUpload.Format(_T("%.1f"), CPreferences::GetMaxUpload());
	else
		MaxUpload = GetResString(IDS_PW_UNLIMITED);
	if ( CPreferences::GetMaxDownload() < UNLIMITED )
		MaxDownload.Format(_T("%.1f"), CPreferences::GetMaxDownload());
	else
		MaxDownload = GetResString(IDS_PW_UNLIMITED);
	/**/
	// End emulEspaña
	
	sLimits.Format(_T("%s/%s"), MaxUpload, MaxDownload);
	// EC Ends

	Out.Replace(_T("[Speed]"), _T("<b>")+_GetPlainResString(IDS_DL_SPEED)+_T(":</b> ")+_SpecialChars(CString(HTTPTempC)) + _T("(") + _GetPlainResString(IDS_PW_CON_LIMITFRM) + _T(": ") + _SpecialChars(sLimits) + _T(")"));

	CString buffer;
	buffer=_T("<b>")+GetResString(IDS_KADEMLIA)+_T(":</b> ");

	if (!thePrefs.GetNetworkKademlia()) {
		buffer.Append(_GetPlainResString(IDS_DISABLED));
	}
	else if ( !Kademlia::CKademlia::IsRunning() ) {

		buffer.Append(_GetPlainResString(IDS_DISCONNECTED));

		if (IsSessionAdmin(Data,sSession)) 
			buffer+=_T(" (<a href=\"?ses=") + sSession + _T("&amp;w=kad&amp;c=connect\">")+_GetPlainResString(IDS_MAIN_BTN_CONNECT)+_T("</a>)");
	}
	else if ( Kademlia::CKademlia::IsRunning() && !Kademlia::CKademlia::IsConnected() ) {

		buffer.Append(_GetPlainResString(IDS_CONNECTING));

		if (IsSessionAdmin(Data,sSession)) 
			buffer+=_T(" (<a href=\"?ses=") + sSession + _T("&amp;w=kad&amp;c=disconnect\">")+_GetPlainResString(IDS_MAIN_BTN_DISCONNECT)+_T("</a>)");
	}
	else if (Kademlia::CKademlia::IsFirewalled()) {
		buffer.Append(_GetPlainResString(IDS_FIREWALLED));
		if (IsSessionAdmin(Data,sSession)) 
			buffer+=_T(" (<a href=\"?ses=") + sSession + _T("&amp;w=kad&amp;c=disconnect\">")+_GetPlainResString(IDS_IRC_DISCONNECT)+_T("</a>)");
	}
	else if (Kademlia::CKademlia::IsConnected()) {
		buffer.Append(_GetPlainResString(IDS_CONNECTED));
		if (IsSessionAdmin(Data,sSession)) 
			buffer+=_T(" (<a href=\"?ses=") + sSession + _T("&amp;w=kad&amp;c=disconnect\">")+_GetPlainResString(IDS_IRC_DISCONNECT)+_T("</a>)");
	}

	Out.Replace(_T("[KademliaInfo]"),buffer);

	return Out;
}


CString CWapServer::_GetServerList(WapThreadData Data)
{
	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString sSession = _ParseURL(Data.sURL, _T("ses"));

	CString sCmd = _ParseURL(Data.sURL, _T("c"));

	if (sCmd == _T("options")){
		CString sAddServerBox = _GetServerOptions(Data);
		sAddServerBox.Replace(_T("[Session]"), sSession);
		return sAddServerBox;
	}
	else{
		if (sCmd == _T("connect") && IsSessionAdmin(Data,sSession) )
		{
			_ConnectToServer(_ParseURL(Data.sURL, _T("ip")),_ttoi(_ParseURL(Data.sURL, _T("port"))));
		}	
		else if (sCmd == _T("disconnect") && IsSessionAdmin(Data,sSession)) 
		{
			if (theApp.serverconnect->IsConnecting())
				SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION, WEBGUIIA_STOPCONNECTING,NULL);
			else
				SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION,WEBGUIIA_DISCONNECT,1);
		}	
		
		if (_ParseURL(Data.sURL, _T("showlist"))==_T("true")){
			if (sCmd == _T("remove") && IsSessionAdmin(Data,sSession)) 
			{
				CString sIP = _ParseURL(Data.sURL, _T("ip"));
				int nPort = _ttoi(_ParseURL(Data.sURL, _T("port")));
				if(!sIP.IsEmpty())
					_RemoveServer(sIP,nPort);
			}	
			CString sSort = _ParseURL(Data.sURL, _T("sort"));
			if (sSort != _T("")) 
			{
				if(sSort == _T("name"))
					pThis->m_Params.ServerSort = SERVER_SORT_NAME;
				else
				if(sSort == _T("description"))
					pThis->m_Params.ServerSort = SERVER_SORT_DESCRIPTION;
				else
				if(sSort == _T("ip"))
					pThis->m_Params.ServerSort = SERVER_SORT_IP;
				else
				if(sSort == _T("users"))
					pThis->m_Params.ServerSort = SERVER_SORT_USERS;
				else
				if(sSort == _T("files"))
					pThis->m_Params.ServerSort = SERVER_SORT_FILES;

				if(_ParseURL(Data.sURL, _T("sortreverse")) == _T(""))
					pThis->m_Params.bServerSortReverse = false;
			}
			if (_ParseURL(Data.sURL, _T("sortreverse")) != _T("")) 
			{
				pThis->m_Params.bServerSortReverse = (_ParseURL(Data.sURL, _T("sortreverse")) == _T("true"));
			}
			CString sServerSortRev;
			if(pThis->m_Params.bServerSortReverse)
				sServerSortRev = _T("false");
			else
				sServerSortRev = _T("true");

			CString Out = pThis->m_Templates.sServerList;

			Out.Replace(_T("[ConnectedServerData]"), _GetConnectedServer(Data));
			//Out.Replace("[AddServerBox]", sAddServerBox);
			Out.Replace(_T("[Session]"), sSession);
			int sortpos=0;
			if(pThis->m_Params.ServerSort == SERVER_SORT_NAME){
				Out.Replace(_T("[SortName]"), _T("./?ses=[Session]&amp;w=server&amp;sort=name&amp;showlist=true&amp;sortreverse=") + sServerSortRev);
				sortpos=1;

				CString strSort=_GetPlainResString(IDS_SL_SERVERNAME);
				if (pThis->m_Params.bServerSortReverse)
					strSort+=_T(" (-)");
				else
					strSort+=_T(" (+)");
				Out.Replace(_T("[Servername]"), strSort);
			}else
				Out.Replace(_T("[SortName]"), _T("./?ses=[Session]&amp;w=server&amp;sort=name&amp;showlist=true"));
			if(pThis->m_Params.ServerSort == SERVER_SORT_DESCRIPTION){
				Out.Replace(_T("[SortDescription]"), _T("./?ses=[Session]&amp;w=server&amp;sort=description&amp;showlist=true&amp;sortreverse=") + sServerSortRev);
				sortpos=2;

				CString strSort=_GetPlainResString(IDS_DESCRIPTION);
				if (pThis->m_Params.bServerSortReverse)
					strSort+=_T(" (-)");
				else
					strSort+=_T(" (+)");
				Out.Replace(_T("[Description]"), strSort);
			}else
				Out.Replace(_T("[SortDescription]"), _T("./?ses=[Session]&amp;w=server&amp;sort=description&amp;showlist=true"));
			if(pThis->m_Params.ServerSort == SERVER_SORT_IP){
				Out.Replace(_T("[SortIP]"), _T("./?ses=[Session]&amp;w=server&amp;sort=ip&amp;showlist=true&amp;sortreverse=") + sServerSortRev);
				sortpos=3;

				CString strSort=_GetPlainResString(IDS_IP);
				if (pThis->m_Params.bServerSortReverse)
					strSort+=_T(" (-)");
				else
					strSort+=_T(" (+)");
				Out.Replace(_T("[Address]"), strSort);
			}else
				Out.Replace(_T("[SortIP]"), _T("./?ses=[Session]&amp;w=server&amp;sort=ip&amp;showlist=true"));
			if(pThis->m_Params.ServerSort == SERVER_SORT_USERS){
				Out.Replace(_T("[SortUsers]"), _T("./?ses=[Session]&amp;w=server&amp;sort=users&amp;showlist=true&amp;sortreverse=") + sServerSortRev);
				sortpos=4;

				CString strSort=_GetPlainResString(IDS_UUSERS);
				if (pThis->m_Params.bServerSortReverse)
					strSort+=_T(" (-)");
				else
					strSort+=_T(" (+)");
				Out.Replace(_T("[Users]"), strSort);
			}else
				Out.Replace(_T("[SortUsers]"), _T("./?ses=[Session]&amp;w=server&amp;sort=users&amp;showlist=true"));
			if(pThis->m_Params.ServerSort == SERVER_SORT_FILES){
				Out.Replace(_T("[SortFiles]"), _T("./?ses=[Session]&amp;w=server&amp;sort=files&amp;showlist=true&amp;sortreverse=") + sServerSortRev);
				sortpos=5;

				CString strSort=_GetPlainResString(IDS_PW_FILES);
				if (pThis->m_Params.bServerSortReverse)
					strSort+=_T(" (-)");
				else
					strSort+=_T(" (+)");
				Out.Replace(_T("[Files]"), strSort);
			}else
				Out.Replace(_T("[SortFiles]"), _T("./?ses=[Session]&amp;w=server&amp;sort=files&amp;showlist=true"));

			//[SortByVal:x1,x2,x3,...]
			int sortbypos=Out.Find(_T("[SortByVal:"),0);
			if(sortbypos!=-1){
				int sortbypos2=Out.Find(_T("]"),sortbypos+1);
				if(sortbypos2!=-1){
					CString strSortByArray=Out.Mid(sortbypos+11,sortbypos2-sortbypos-11);
					CString resToken;
					int curPos = 0,curPos2=0;
					bool posfound=false;
					resToken= strSortByArray.Tokenize(_T(","),curPos);
					while (resToken != _T("") && !posfound)
					{
						curPos2++;
						if(sortpos==_ttoi(resToken)){
							CString strTemp;
							strTemp.Format(_T("%i"),curPos2);
							Out.Replace(Out.Mid(sortbypos,sortbypos2-sortbypos+1),strTemp);
							posfound=true;
						}
						resToken= strSortByArray.Tokenize(_T(","),curPos);
					};
					if(!posfound)
						Out.Replace(Out.Mid(sortbypos,sortbypos2-sortbypos+1),_T("1"));
				}
			}

			CString OutE = pThis->m_Templates.sServerLine;

			OutE.Replace(_T("[Connect]"), _GetPlainResString(IDS_IRC_CONNECT));
			OutE.Replace(_T("[RemoveServer]"), _GetPlainResString(IDS_REMOVETHIS));
			OutE.Replace(_T("[ConfirmRemove]"), _GetPlainResString(IDS_WEB_CONFIRM_REMOVE_SERVER));

			CArray<ServerEntry, ServerEntry> ServerArray;

			// Populating array
			for (uint32 sc=0;sc<theApp.serverlist->GetServerCount();sc++)
			{
				CServer* cur_file = theApp.serverlist->GetServerAt(sc);
				ServerEntry Entry;
				Entry.sServerName = _SpecialChars(cur_file->GetListName());
				Entry.sServerDescription = _SpecialChars(cur_file->GetDescription());
				Entry.nServerPort = cur_file->GetPort();
				Entry.sServerIP = cur_file->GetAddress();
				Entry.nServerUsers = cur_file->GetUsers();
				Entry.nServerMaxUsers = cur_file->GetMaxUsers();
				Entry.nServerFiles = cur_file->GetFiles();

				ServerArray.Add(Entry);
			}
			// Sorting (simple bubble sort, we don't have tons of data here)
			bool bSorted = true;
			for(int nMax = 0;bSorted && nMax < ServerArray.GetCount()*2; nMax++)
			{
				bSorted = false;
				for(int i = 0; i < ServerArray.GetCount() - 1; i++)
				{
					bool bSwap = false;
					switch(pThis->m_Params.ServerSort)
					{
					case SERVER_SORT_NAME:
						bSwap = ServerArray[i].sServerName.CompareNoCase(ServerArray[i+1].sServerName) > 0;
						break;
					case SERVER_SORT_DESCRIPTION:
						bSwap = ServerArray[i].sServerDescription.CompareNoCase(ServerArray[i+1].sServerDescription) < 0;
						break;
					case SERVER_SORT_IP:
						bSwap = ServerArray[i].sServerIP.CompareNoCase(ServerArray[i+1].sServerIP) > 0;
						break;
					case SERVER_SORT_USERS:
						bSwap = ServerArray[i].nServerUsers < ServerArray[i+1].nServerUsers;
						break;
					case SERVER_SORT_FILES:
						bSwap = ServerArray[i].nServerFiles < ServerArray[i+1].nServerFiles;
						break;
					}
					if(pThis->m_Params.bServerSortReverse)
					{
						bSwap = !bSwap;
					}
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
			CString sList = _T("");
			int startpos=0,endpos;

			if(_ParseURL(Data.sURL, _T("startpos"))!=_T(""))
				startpos=_ttoi(_ParseURL(Data.sURL, _T("startpos")));

			endpos = startpos + thePrefs.GetWapMaxItemsInPages();

			if (endpos>ServerArray.GetCount())
				endpos=ServerArray.GetCount();

			if (startpos>ServerArray.GetCount()){
				startpos=endpos-thePrefs.GetWapMaxItemsInPages();
				if (startpos<0) startpos=0;
			}

			for(int i = startpos; i < endpos; i++)
			{
				CString HTTPProcessData = OutE;	// Copy Entry Line to Temp
				HTTPProcessData.Replace(_T("[1]"), ServerArray[i].sServerName);
				HTTPProcessData.Replace(_T("[2]"), ServerArray[i].sServerDescription);
				CString sPort; sPort.Format(_T(":%d"), ServerArray[i].nServerPort);
				HTTPProcessData.Replace(_T("[3]"), ServerArray[i].sServerIP + sPort);

				CString sT;
				if(ServerArray[i].nServerUsers > 0)
				{
					if(ServerArray[i].nServerMaxUsers > 0)
						sT.Format(_T("%d (%d)"), ServerArray[i].nServerUsers, ServerArray[i].nServerMaxUsers);
					else
						sT.Format(_T("%d"), ServerArray[i].nServerUsers);
				}
				HTTPProcessData.Replace(_T("[4]"), sT);
				sT = _T("");
				if(ServerArray[i].nServerFiles > 0)
					sT.Format(_T("%d"), ServerArray[i].nServerFiles);
				HTTPProcessData.Replace(_T("[5]"), sT);

				CString sServerPort; sServerPort.Format(_T("%d"), ServerArray[i].nServerPort);

				HTTPProcessData.Replace(_T("[6]"), IsSessionAdmin(Data,sSession)? CString(_T("./?ses=") + sSession + _T("&amp;w=server&amp;c=connect&amp;ip=") + ServerArray[i].sServerIP+_T("&amp;port=")+sServerPort):GetPermissionDenied());
				HTTPProcessData.Replace(_T("[LinkRemove]"), IsSessionAdmin(Data,sSession)?CString(_T("./?ses=") + sSession + _T("&amp;w=server&amp;c=remove&amp;ip=") + ServerArray[i].sServerIP+_T("&amp;port=")+sServerPort+_T("&amp;showlist=true")):GetPermissionDenied());

				sList += HTTPProcessData;
			}
			
			//Navigation Line
			CString strNavLine,strNavUrl;
			int pos,pos2;
			strNavUrl = Data.sURL;
			if((pos=strNavUrl.Find(_T("startpos="),0))>1){
				if (strNavUrl.Mid(pos-1,1)==_T("&")) pos--;
				pos2=strNavUrl.Find(_T("&"),pos+1);
				if (pos2==-1) pos2 = strNavUrl.GetLength();
				strNavUrl = strNavUrl.Left(pos) + strNavUrl.Right(strNavUrl.GetLength()-pos2);
			}	
			strNavUrl=_SpecialChars(strNavUrl);

			if(startpos>0){
				startpos-=thePrefs.GetWapMaxItemsInPages();
				if (startpos<0) startpos = 0;
				strNavLine.Format(_T("<a href=\".") + strNavUrl + _T("&amp;startpos=%d\">") + _GetPlainResString(IDS_WAP_PREV) + _T("</a>&nbsp;"), startpos);
			}
			if(endpos<ServerArray.GetCount()){
				CString strTemp;
				strTemp.Format(_T("<a href=\".") + strNavUrl + _T("&amp;startpos=%d\">") + _GetPlainResString(IDS_WAP_NEXT) + _T("</a> "), endpos);
				strNavLine += strTemp;
			}
			Out.Replace(_T("[NavigationLine]"), strNavLine);
			Out.Replace(_T("[ServersList]"), sList);

			Out.Replace(_T("[ServerList]"), _GetPlainResString(IDS_SV_SERVERLIST));
			Out.Replace(_T("[Servername]"), _GetPlainResString(IDS_SL_SERVERNAME));
			Out.Replace(_T("[Description]"), _GetPlainResString(IDS_DESCRIPTION));
			Out.Replace(_T("[Address]"), _GetPlainResString(IDS_IP));
			Out.Replace(_T("[Connect]"), _GetPlainResString(IDS_IRC_CONNECT));
			Out.Replace(_T("[Users]"), _GetPlainResString(IDS_UUSERS));
			Out.Replace(_T("[Files]"), _GetPlainResString(IDS_PW_FILES));
			Out.Replace(_T("[Actions]"), _GetPlainResString(IDS_WEB_ACTIONS));
			Out.Replace(_T("[Session]"), sSession);
			Out.Replace(_T("[Home]"), _GetPlainResString(IDS_WAP_HOME));
			Out.Replace(_T("[Back]"), _GetPlainResString(IDS_WAP_BACK));
			Out.Replace(_T("[SortBy]"), _GetPlainResString(IDS_WAP_SORTBY));

			return Out;
		} else {
			CString Out = pThis->m_Templates.sServerMain;
			Out.Replace(_T("[ServerList]"), _GetPlainResString(IDS_SV_SERVERLIST));
			Out.Replace(_T("[ConnectedServerData]"), _GetConnectedServer(Data));
			Out.Replace(_T("[Session]"), sSession);
			Out.Replace(_T("[Home]"), _GetPlainResString(IDS_WAP_HOME));
			Out.Replace(_T("[Back]"), _GetPlainResString(IDS_WAP_BACK));
			Out.Replace(_T("[Refresh]"), _GetPlainResString(IDS_WAP_REFRESH));

			return Out;
		}
	}
}

CString CWapServer::_GetTransferDownList(WapThreadData Data)
{
	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString sSession = _ParseURL(Data.sURL, _T("ses"));
	int cat=_ttoi(_ParseURL(Data.sURL,_T("cat")));

	bool clcompl=(_ParseURL(Data.sURL,_T("ClCompl"))==_T("yes"));
	CString sCat; (cat!=0)?sCat.Format(_T("&amp;cat=%i"),cat):sCat=_T("");

	CString Out = _T("");

	if (clcompl && IsSessionAdmin(Data,sSession)) 
		theApp.emuledlg->transferwnd->GetDownloadList()->ClearCompleted(true);

	if (_ParseURL(Data.sURL, _T("c")) != _T("") && IsSessionAdmin(Data,sSession)) 
	{
		CString HTTPTemp = _ParseURL(Data.sURL, _T("c"));
		theApp.AddEd2kLinksToDownload(HTTPTemp,(uint8)cat);
	}

	if (_ParseURL(Data.sURL, _T("op")) != _T("") &&
		_ParseURL(Data.sURL, _T("file")) != _T(""))
	{
		uchar FileHash[16];
		_GetFileHash(_ParseURL(Data.sURL, _T("file")), FileHash);

		CPartFile* found_file = NULL;

		for (int fx=0;fx<theApp.downloadqueue->GetFileCount();fx++)
		{
			CPartFile* cur_file =  theApp.downloadqueue->GetFileByIndex(fx);

			bool bGood = true;
			for(int i = 0; i < 16; i++)
			{
				if(cur_file->GetFileHash()[i] != FileHash[i])
				{
					bGood = false;
					break;
				}
			}
			if(bGood)
			{
				found_file = cur_file;
				break;
			}
		}

		if(_ParseURL(Data.sURL, _T("op")) == _T("pause") && IsSessionAdmin(Data,sSession))
		{
			if(found_file)
				found_file->PauseFile();
		}
		else if(_ParseURL(Data.sURL, _T("op")) == _T("resume") && IsSessionAdmin(Data,sSession))
		{
			if(found_file)
				found_file->ResumeFile();
		}
		else if(_ParseURL(Data.sURL, _T("op")) == _T("cancel") && IsSessionAdmin(Data,sSession))
		{
			if(found_file)
				found_file->DeleteFile();
		}
		else if(_ParseURL(Data.sURL, _T("op")) == _T("prioup") && IsSessionAdmin(Data,sSession))
		{
			if(found_file) {
			  if (!found_file->IsAutoDownPriority())
				switch (found_file->GetDownPriority()) {
					case PR_LOW: found_file->SetAutoDownPriority(false); found_file->SetDownPriority(PR_NORMAL);break;
					case PR_NORMAL: found_file->SetAutoDownPriority(false); found_file->SetDownPriority(PR_HIGH);break;
					case PR_HIGH: found_file->SetAutoDownPriority(true); found_file->SetDownPriority(PR_HIGH);break;
				}
			  else {found_file->SetAutoDownPriority(false); found_file->SetDownPriority(PR_LOW);}
			}
		}
		else if(_ParseURL(Data.sURL, _T("op")) == _T("priodown") && IsSessionAdmin(Data,sSession))
		{
			if(found_file) {
			  if (!found_file->IsAutoDownPriority())
				switch (found_file->GetDownPriority()) {
					case PR_LOW: found_file->SetAutoDownPriority(true); found_file->SetDownPriority(PR_HIGH);break;
					case PR_NORMAL: found_file->SetAutoDownPriority(false); found_file->SetDownPriority(PR_LOW);break;
					case PR_HIGH: found_file->SetAutoDownPriority(false); found_file->SetDownPriority(PR_NORMAL);break;
				}
			  else {found_file->SetAutoDownPriority(false); found_file->SetDownPriority(PR_HIGH);}
			}
		}
	}
	if (_ParseURL(Data.sURL, _T("sort")) != _T("")) 
	{
		if(_ParseURL(Data.sURL, _T("sort")) == _T("name"))
			pThis->m_Params.DownloadSort = DOWN_SORT_NAME;
		else
		if(_ParseURL(Data.sURL, _T("sort")) == _T("size"))
			pThis->m_Params.DownloadSort = DOWN_SORT_SIZE;
		else
		if(_ParseURL(Data.sURL, _T("sort")) == _T("transferred"))
			pThis->m_Params.DownloadSort = DOWN_SORT_TRANSFERRED;
		else
		if(_ParseURL(Data.sURL, _T("sort")) == _T("speed"))
			pThis->m_Params.DownloadSort = DOWN_SORT_SPEED;
		else
		if(_ParseURL(Data.sURL, _T("sort")) == _T("progress"))
			pThis->m_Params.DownloadSort = DOWN_SORT_PROGRESS;

		if(_ParseURL(Data.sURL, _T("sortreverse")) == _T(""))
			pThis->m_Params.bDownloadSortReverse = false;
	}
	if (_ParseURL(Data.sURL, _T("sortreverse")) != _T("")) 
	{
		pThis->m_Params.bDownloadSortReverse = (_ParseURL(Data.sURL, _T("sortreverse")) == _T("true"));
	}
	CString sDownloadSortRev;
	if(pThis->m_Params.bDownloadSortReverse)
		sDownloadSortRev = _T("false");
	else
		sDownloadSortRev = _T("true");

	CPartFile* found_file = NULL;

	if(_ParseURL(Data.sURL, _T("showfile"))!=_T("")){
		uchar FileHash[16];
		_GetFileHash(_ParseURL(Data.sURL, _T("showfile")), FileHash);

		CArray<CPartFile*,CPartFile*> partlist;
		theApp.emuledlg->transferwnd->GetDownloadList()->GetDisplayedFiles(&partlist);
		
		for (int fx=0;fx<partlist.GetCount();fx++)
		{
			CPartFile* cur_file =  partlist.GetAt(fx);

			bool bGood = true;
			for(int i = 0; i < 16; i++)
			{
				if(cur_file->GetFileHash()[i] != FileHash[i])
				{
					bGood = false;
					break;
				}
			}
			if(bGood)
			{
				found_file = cur_file;
				break;
			}
		}

		if(found_file){
			Out += pThis->m_Templates.sTransferDownFileDetails;

			CString JSfileinfo=_SpecialChars(found_file->GetInfoSummary());
			JSfileinfo.Replace(_T("\n"),_T("\\n"));
			CString sActions = found_file->getPartfileStatus() + _T("<br/>");
			CString sED2kLink;
			CString strFileSize;
			CString strCleanFileName;
			CString strFileHash;
			CString strFileName;
			CString HTTPTemp;

			strFileName = _SpecialChars(found_file->GetFileName());
			strFileHash = EncodeBase16(found_file->GetFileHash(), 16);
			strFileSize.Format(_T("%I64ul"),(uint64)found_file->GetFileSize());
			
			strCleanFileName = RemoveWMLScriptInvalidChars(found_file->GetFileName());
			for(int pos=0; pos<strCleanFileName.GetLength(); pos++){
				unsigned char c;
				c = (unsigned char)strCleanFileName.GetAt(pos);
				if(!((c >= _T('0') && c <= _T('9')) || (c >= _T('a') && c <= _T('z')) || (c >= _T('A') && c <= _T('Z')))){
					c = _T('.');
				}
				strCleanFileName.SetAt(pos, c);
			}

			sED2kLink.Format(_T("<anchor>[Ed2klink]<go href=\"./script.wmls#showed2k('")+ strCleanFileName  + _T("','") + strFileSize + _T("','") + strFileHash + _T("')\"/></anchor><br/>"));
			sED2kLink.Replace(_T("[Ed2klink]"), _GetPlainResString(IDS_SW_LINK));
			sActions += sED2kLink;

			bool bCanBeDeleted = true;
			switch(found_file->GetStatus())
			{
				case PS_COMPLETING:
				case PS_COMPLETE:
					bCanBeDeleted = false;
					break;
				case PS_HASHING: 
				case PS_WAITINGFORHASH:
				case PS_ERROR:
					break;
				case PS_PAUSED:
				{
					if (IsSessionAdmin(Data,sSession)) {
						CString sResume=_T("");
						sResume.Format(_T("<a href=\"[Link]\">[Resume]</a><br />"));
						sResume.Replace(_T("[Link]"), CString(_T("./?ses=") + sSession + _T("&amp;w=transferdown&amp;op=resume&amp;file=") + strFileHash + sCat + _T("&amp;showfile=") + strFileHash )) ;
						sActions += sResume;
					}
				}
					break; 
				default: // waiting or downloading
				{
					if (IsSessionAdmin(Data,sSession)) {
						CString sPause;
						sPause.Format(_T("<a href=\"[Link]\">[Pause]</a><br />"));
						sPause.Replace(_T("[Link]"), _T("./?ses=") + sSession + _T("&amp;w=transferdown&amp;op=pause&amp;file=") + strFileHash + sCat + _T("&amp;showfile=") + strFileHash);
						sActions += sPause;
					}
				}
				break;
			}
			if(bCanBeDeleted)
			{
				if (IsSessionAdmin(Data,sSession)) {
					CString sCancel;
					sCancel.Format(_T("<a href=\"./script.wmls#confirmcancel('[Link]')\">[Cancel]</a><br />"));
					sCancel.Replace(_T("[Link]"), _T("./?ses=") + sSession + _T("&amp;w=transferdown&amp;op=cancel&amp;file=") + strFileHash + sCat);
					sActions += sCancel;
				}
			}

			if (IsSessionAdmin(Data,sSession)) {
				sActions.Replace(_T("[Resume]"), _GetPlainResString(IDS_DL_RESUME));
				sActions.Replace(_T("[Pause]"), _GetPlainResString(IDS_DL_PAUSE));
				sActions.Replace(_T("[Cancel]"), _GetPlainResString(IDS_MAIN_BTN_CANCEL));
				sActions.Replace(_T("[ConfirmCancel]"), _GetPlainResStringNoQuote(IDS_Q_CANCELDL2));
				
				if (found_file->GetStatus()!=PS_COMPLETE && found_file->GetStatus()!=PS_COMPLETING) {
					sActions.Append(_T("<a href=\"./?ses=[Session]&amp;w=transferdown&amp;op=prioup&amp;file=") + strFileHash + sCat + _T("&amp;showfile=") + strFileHash + _T("\">[PriorityUp]</a><br/>"));
					sActions.Append(_T("<a href=\"./?ses=[Session]&amp;w=transferdown&amp;op=priodown&amp;file=") + strFileHash + sCat + _T("&amp;showfile=") + strFileHash + _T("\">[PriorityDown]</a><br/>"));
				}
			}

			Out.Replace(_T("[FileName]"), strFileName);

			if(strFileName.GetLength() > SHORT_FILENAME_LENGTH){
				Out.Replace(_T("[ShortFileName]"), strFileName.Left(SHORT_FILENAME_LENGTH) + _T("..."));
			}else{
				Out.Replace(_T("[ShortFileName]"), strFileName);
			}
			Out.Replace(_T("[FileInfo]"), _SpecialChars(found_file->GetInfoSummary()));

			Out.Replace(_T("[2]"), CastItoXBytes(found_file->GetFileSize()));

			if(found_file->GetCompletedSize() > (uint64)0)
			{
				Out.Replace(_T("[3]"), CastItoXBytes(found_file->GetCompletedSize()));
			}
			else{
				Out.Replace(_T("[3]"), _T("-"));
			}

			Out.Replace(_T("[DownloadBar]"), _GetDownloadGraph(Data,strFileHash));

			if(found_file->GetDatarate() > 0.0f)
			{
				HTTPTemp.Format(_T("%8.2f %s"), found_file->GetDatarate()/1024.0 ,_GetPlainResString(IDS_KBYTESPERSEC) );
				Out.Replace(_T("[4]"), HTTPTemp);
			}
			else{
				Out.Replace(_T("[4]"), _T("-"));
			}
			if(found_file->GetSourceCount() > 0)
			{
				HTTPTemp.Format(_T("%i&nbsp;/&nbsp;%8i&nbsp;(%i)"),
					found_file->GetSourceCount()-found_file->GetNotCurrentSourcesCount(),
					found_file->GetSourceCount(),
					found_file->GetTransferringSrcCount());

				Out.Replace(_T("[5]"), HTTPTemp);
			}
			else{
				Out.Replace(_T("[5]"), _T("-"));
			}

			switch(found_file->GetDownPriority()) {
				case 0: HTTPTemp=GetResString(IDS_PRIOLOW);break;
				case 10: HTTPTemp=GetResString(IDS_PRIOAUTOLOW);break;

				case 1: HTTPTemp=GetResString(IDS_PRIONORMAL);break;
				case 11: HTTPTemp=GetResString(IDS_PRIOAUTONORMAL);break;

				case 2: HTTPTemp=GetResString(IDS_PRIOHIGH);break;
				case 12: HTTPTemp=GetResString(IDS_PRIOAUTOHIGH);break;
			}

			Out.Replace(_T("[PrioVal]"), HTTPTemp);
			Out.Replace(_T("[6]"), sActions);

			HTTPTemp.Format(_T("%.1f%%"),found_file->GetPercentCompleted());
			Out.Replace(_T("[FileProgress]"), HTTPTemp);

			Out.Replace(_T("[FileHash]"),strFileHash);
		}
	}

	if (!found_file){
		Out += pThis->m_Templates.sTransferDownList;

		InsertCatBox(Out,cat,_GetPlainResString(IDS_SELECTCAT)+_T(":&nbsp;<br/>"),true,true);
		Out.Replace(_T("[URL]"), _SpecialChars(_T(".") + Data.sURL));

		int sortpos=0;
		if(pThis->m_Params.DownloadSort == DOWN_SORT_NAME){
			Out.Replace(_T("[SortName]"), _T("./?ses=[Session]&amp;w=transferdown&amp;sort=name&amp;sortreverse=") + sDownloadSortRev);
			sortpos=1;
			CString strSort=_GetPlainResString(IDS_DL_FILENAME);
			if (pThis->m_Params.bDownloadSortReverse)
				strSort+=_T(" (-)");
			else
				strSort+=_T(" (+)");
			Out.Replace(_T("[Filename]"), strSort);
		}else
			Out.Replace(_T("[SortName]"), _T("./?ses=[Session]&amp;w=transferdown&amp;sort=name"));
		if(pThis->m_Params.DownloadSort == DOWN_SORT_SIZE){
			Out.Replace(_T("[SortSize]"), _T("./?ses=[Session]&amp;w=transferdown&amp;sort=size&amp;sortreverse=") + sDownloadSortRev);
			sortpos=2;
			CString strSort=_GetPlainResString(IDS_DL_SIZE);
			if (pThis->m_Params.bDownloadSortReverse)
				strSort+=_T(" (-)");
			else
				strSort+=_T(" (+)");
			Out.Replace(_T("[Size]"), strSort);
		}else
			Out.Replace(_T("[SortSize]"), _T("./?ses=[Session]&amp;w=transferdown&amp;sort=size"));
		if(pThis->m_Params.DownloadSort == DOWN_SORT_TRANSFERRED){
			Out.Replace(_T("[SortTransferred]"), _T("./?ses=[Session]&amp;w=transferdown&amp;sort=transferred&amp;sortreverse=") + sDownloadSortRev);
			sortpos=3;
			CString strSort=_GetPlainResString(IDS_COMPLETE);
			if (pThis->m_Params.bDownloadSortReverse)
				strSort+=_T(" (-)");
			else
				strSort+=_T(" (+)");
			Out.Replace(_T("[Transferred]"), strSort);
		}else
			Out.Replace(_T("[SortTransferred]"), _T("./?ses=[Session]&amp;w=transferdown&amp;sort=transferred"));
		if(pThis->m_Params.DownloadSort == DOWN_SORT_SPEED){
			Out.Replace(_T("[SortSpeed]"), _T("./?ses=[Session]&amp;w=transferdown&amp;sort=speed&amp;sortreverse=") + sDownloadSortRev);
			sortpos=4;
			CString strSort=_GetPlainResString(IDS_DL_SPEED);
			if (pThis->m_Params.bDownloadSortReverse)
				strSort+=_T(" (-)");
			else
				strSort+=_T(" (+)");
			Out.Replace(_T("[Speed]"), strSort);
		}else
			Out.Replace(_T("[SortSpeed]"), _T("./?ses=[Session]&amp;w=transferdown&amp;sort=speed"));
		if(pThis->m_Params.DownloadSort == DOWN_SORT_PROGRESS){
			Out.Replace(_T("[SortProgress]"), _T("./?ses=[Session]&amp;w=transferdown&amp;sort=progress&amp;sortreverse=") + sDownloadSortRev);
			sortpos=5;
			CString strSort=_GetPlainResString(IDS_DL_PROGRESS);
			if (pThis->m_Params.bDownloadSortReverse)
				strSort+=_T(" (-)");
			else
				strSort+=_T(" (+)");
			Out.Replace(_T("[Progress]"), strSort);
		}else
			Out.Replace(_T("[SortProgress]"), _T("./?ses=[Session]&amp;w=transferdown&amp;sort=progress"));

		//[SortByVal:x1,x2,x3,...]
		int sortbypos=Out.Find(_T("[SortByVal:"),0);
		if(sortbypos!=-1){
			int sortbypos2=Out.Find(_T("]"),sortbypos+1);
			if(sortbypos2!=-1){
				CString strSortByArray=Out.Mid(sortbypos+11,sortbypos2-sortbypos-11);
				CString resToken;
				int curPos = 0,curPos2=0;
				bool posfound=false;
				resToken= strSortByArray.Tokenize(_T(","),curPos);
				while (resToken != _T("") && !posfound)
				{
					curPos2++;
					if(sortpos==_ttoi(resToken)){
						CString strTemp;
						strTemp.Format(_T("%i"),curPos2);
						Out.Replace(Out.Mid(sortbypos,sortbypos2-sortbypos+1),strTemp);
						posfound=true;
					}
					resToken= strSortByArray.Tokenize(_T(","),curPos);
				};
				if(!posfound)
					Out.Replace(Out.Mid(sortbypos,sortbypos2-sortbypos+1),_T("1"));
			}
		}

		CString OutE = pThis->m_Templates.sTransferDownLineLite;
		CString OutE2 = pThis->m_Templates.sTransferDownLineLiteGood;

		float fTotalSize = 0, fTotalTransferred = 0, fTotalSpeed = 0;
		CArray<DownloadFiles, DownloadFiles> FilesArray;

		CArray<CPartFile*,CPartFile*> partlist;
		theApp.emuledlg->transferwnd->GetDownloadList()->GetDisplayedFiles(&partlist);

		// Populating array
		bool completedAv=false;
		for (int i=0;i<partlist.GetCount();i++) {
			
			CPartFile* cur_file=partlist.GetAt(i);
			if (cur_file) {

				DownloadFiles dFile;
				dFile.sFileName = _SpecialChars(cur_file->GetFileName());
				dFile.m_qwFileSize = cur_file->GetFileSize();
				dFile.m_qwFileTransferred = cur_file->GetCompletedSize();
				dFile.m_dblCompleted = cur_file->GetPercentCompleted();
				dFile.lFileSpeed = cur_file->GetDatarate();
				dFile.nFileStatus = cur_file->GetStatus();
				dFile.sFileState = cur_file->getPartfileStatus();
				dFile.nFilePrio = cur_file->GetDownPriority();
				if (cur_file->IsAutoDownPriority()) dFile.nFilePrio+=10;
				dFile.sFileHash = EncodeBase16(cur_file->GetFileHash(), 16);
				dFile.lSourceCount = cur_file->GetSourceCount();
				dFile.lNotCurrentSourceCount = cur_file->GetNotCurrentSourcesCount();
				dFile.lTransferringSourceCount = cur_file->GetTransferringSrcCount();
				if (theApp.GetPublicIP() != 0 && !theApp.IsFirewalled())
					dFile.sED2kLink = cur_file->GetED2kLink(false, false, false, true, theApp.GetPublicIP());
				else
					dFile.sED2kLink = cur_file->GetED2kLink();
				dFile.sFileInfo = _SpecialChars(cur_file->GetInfoSummary(true));

				if (cat>0 && cur_file->GetCategory() != (UINT)cat) continue;
				if (cat<0) {
					switch (cat) {
						case -1 : if (cur_file->GetCategory()!=0) continue; break;
						case -2 : if (!cur_file->IsPartFile()) continue; break;
						case -3 : if (cur_file->IsPartFile()) continue; break;
						case -4 : if (!((cur_file->GetStatus()==PS_READY|| cur_file->GetStatus()==PS_EMPTY) && cur_file->GetTransferringSrcCount()==0)) continue; break;
						case -5 : if (!((cur_file->GetStatus()==PS_READY|| cur_file->GetStatus()==PS_EMPTY) && cur_file->GetTransferringSrcCount()>0)) continue; break;
						case -6 : if (cur_file->GetStatus()!=PS_ERROR) continue; break;
						case -7 : if (cur_file->GetStatus()!=PS_PAUSED) continue; break;
						case -8 : if (!cur_file->IsStopped()) continue; break;
						case -9 : if (!cur_file->IsMovie()) continue; break;
						case -10 : if (ED2KFT_AUDIO != GetED2KFileTypeID(cur_file->GetFileName())) continue; break;
						case -11 : if (!cur_file->IsArchive()) continue; break;
						case -12 : if (ED2KFT_CDIMAGE != GetED2KFileTypeID(cur_file->GetFileName())) continue; break;
					}
				}
				FilesArray.Add(dFile);
				if (!cur_file->IsPartFile()) completedAv=true;
			}
		}
		// Sorting (simple bubble sort, we don't have tons of data here)
		bool bSorted = true;
		for(int nMax = 0;bSorted && nMax < FilesArray.GetCount()*2; nMax++)
		{
			bSorted = false;
			for(int i = 0; i < FilesArray.GetCount() - 1; i++)
			{
				bool bSwap = false;
				switch(pThis->m_Params.DownloadSort)
				{
					case DOWN_SORT_NAME:
						bSwap = FilesArray[i].sFileName.CompareNoCase(FilesArray[i+1].sFileName) > 0;
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
				}
				if(pThis->m_Params.bDownloadSortReverse)
				{
					bSwap = !bSwap;
				}
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
		CString sDownList = _T("");
		CString HTTPTemp;
		int startpos=0,endpos;

		if(_ParseURL(Data.sURL, _T("startpos"))!=_T(""))
			startpos=_ttoi(_ParseURL(Data.sURL, _T("startpos")));

		endpos = startpos + thePrefs.GetWapMaxItemsInPages();

		if (endpos>FilesArray.GetCount())
			endpos=FilesArray.GetCount();

		if (startpos>FilesArray.GetCount()){
			startpos=endpos-thePrefs.GetWapMaxItemsInPages();
			if (startpos<0) startpos=0;
		}

		for(int i = startpos; i < endpos; i++)
		{
			CString JSfileinfo=FilesArray[i].sFileInfo;
			JSfileinfo.Replace(_T("\n"),_T("\\n"));
			CString sActions = FilesArray[i].sFileState + _T("<br /> ");
			CString sED2kLink;
			CString strFileSize;
			CString strCleanFileName;

			strFileSize.Format(_T("%ul"),FilesArray[i].m_qwFileSize);
			strCleanFileName = _SpecialChars(FilesArray[i].sFileName);
			strCleanFileName.Replace(_T(" "),_T("."));
			sED2kLink.Format(_T("<anchor>[Ed2klink]<go href=\"./script.wmls#showed2k('")+ strCleanFileName  + _T("','") + strFileSize + _T("','") + FilesArray[i].sFileHash + _T("')\"/></anchor><br />"));
			sED2kLink.Replace(_T("[Ed2klink]"), _GetPlainResString(IDS_SW_LINK));
			sActions += sED2kLink;

			bool bCanBeDeleted = true;
			switch(FilesArray[i].nFileStatus)
			{
				case PS_COMPLETING:
				case PS_COMPLETE:
					bCanBeDeleted = false;
					break;
				case PS_HASHING: 
				case PS_WAITINGFORHASH:
				case PS_ERROR:
					break;
				case PS_PAUSED:
				{
					if (IsSessionAdmin(Data,sSession)) {
						CString sResume=_T("");
						sResume.Format(_T("<a href=\"[Link]\">[Resume]</a><br />"));
						sResume.Replace(_T("[Link]"), _T("./?ses=") + sSession + _T("&amp;w=transferdown&amp;op=resume&amp;file=") + FilesArray[i].sFileHash +sCat ) ;
						sActions += sResume;
					}
				}
					break; 
				default: // waiting or downloading
				{
					if (IsSessionAdmin(Data,sSession)) {
						CString sPause;
						sPause.Format(_T("<a href=\"[Link]\">[Pause]</a><br />"));
						sPause.Replace(_T("[Link]"), _T("./?ses=") + sSession + _T("&amp;w=transferdown&amp;op=pause&amp;file=") + FilesArray[i].sFileHash+sCat);
						sActions += sPause;
					}
				}
				break;
			}
			if(bCanBeDeleted)
			{
				if (IsSessionAdmin(Data,sSession)) {
					CString sCancel;
					sCancel.Format(_T("<a href=\"./script.wmls#confirmcancel('[Link]')\">[Cancel]</a><br />"));
					sCancel.Replace(_T("[Link]"), _T("./?ses=") + sSession + _T("&amp;w=transferdown&amp;op=cancel&amp;file=") + FilesArray[i].sFileHash+sCat);
					sActions += sCancel;
				}
			}

			if (IsSessionAdmin(Data,sSession)) {
				sActions.Replace(_T("[Resume]"), _GetPlainResString(IDS_DL_RESUME));
				sActions.Replace(_T("[Pause]"), _GetPlainResString(IDS_DL_PAUSE));
				sActions.Replace(_T("[Cancel]"), _GetPlainResString(IDS_MAIN_BTN_CANCEL));
				sActions.Replace(_T("[ConfirmCancel]"), _GetPlainResStringNoQuote(IDS_Q_CANCELDL2));
				
				if (FilesArray[i].nFileStatus!=PS_COMPLETE && FilesArray[i].nFileStatus!=PS_COMPLETING) {
					sActions.Append(_T("<a href=\"./?ses=[Session]&amp;w=transferdown&amp;op=prioup&amp;file=") + FilesArray[i].sFileHash+sCat+_T("\">[PriorityUp]</a><br />"));
					sActions.Append(_T("<a href=\"./?ses=[Session]&amp;w=transferdown&amp;op=priodown&amp;file=") + FilesArray[i].sFileHash+sCat+_T("\">[PriorityDown]</a><br />"));
				}
			}

			CString HTTPProcessData;
			// if downloading, draw in other color
			if(FilesArray[i].lFileSpeed > 0)
				HTTPProcessData = OutE2;
			else
				HTTPProcessData = OutE;

			CString sFileNumber; sFileNumber.Format(_T("%d"),i+1);
			HTTPProcessData.Replace(_T("[FileNumber]"), sFileNumber);

			HTTPProcessData.Replace(_T("[FileName]"), FilesArray[i].sFileName);

			if(FilesArray[i].sFileName.GetLength() > SHORT_FILENAME_LENGTH){
				HTTPProcessData.Replace(_T("[ShortFileName]"), FilesArray[i].sFileName.Left(SHORT_FILENAME_LENGTH) + _T("..."));
			}else{
				HTTPProcessData.Replace(_T("[ShortFileName]"), FilesArray[i].sFileName);
			}
			HTTPProcessData.Replace(_T("[FileInfo]"), FilesArray[i].sFileInfo);

			fTotalSize += FilesArray[i].m_qwFileSize;

			HTTPProcessData.Replace(_T("[2]"), CastItoXBytes(FilesArray[i].m_qwFileSize));

			if(FilesArray[i].m_qwFileTransferred > 0)
			{
				fTotalTransferred += FilesArray[i].m_qwFileTransferred;

				HTTPProcessData.Replace(_T("[3]"), CastItoXBytes(FilesArray[i].m_qwFileTransferred));
			}
			else{
				HTTPProcessData.Replace(_T("[3]"), _T("-"));
			}

			HTTPProcessData.Replace(_T("[DownloadBar]"), _GetDownloadGraph(Data,FilesArray[i].sFileHash));

			if(FilesArray[i].lFileSpeed > 0.0f)
			{
				fTotalSpeed += FilesArray[i].lFileSpeed;

				HTTPTemp.Format(_T("%8.2f %s"), FilesArray[i].lFileSpeed/1024.0 ,_GetPlainResString(IDS_KBYTESPERSEC) );
				HTTPProcessData.Replace(_T("[4]"), HTTPTemp);
			}
			else{
				HTTPProcessData.Replace(_T("[4]"), _T("-"));
			}
			if(FilesArray[i].lSourceCount > 0)
			{
				HTTPTemp.Format(_T("%i&nbsp;/&nbsp;%8i&nbsp;(%i)"),
					FilesArray[i].lSourceCount-FilesArray[i].lNotCurrentSourceCount,
					FilesArray[i].lSourceCount,
					FilesArray[i].lTransferringSourceCount);

				HTTPProcessData.Replace(_T("[5]"), HTTPTemp);
			}
			else{
				HTTPProcessData.Replace(_T("[5]"), _T("-"));
			}

			switch(FilesArray[i].nFilePrio) {
				case 0: HTTPTemp=GetResString(IDS_PRIOLOW);break;
				case 10: HTTPTemp=GetResString(IDS_PRIOAUTOLOW);break;

				case 1: HTTPTemp=GetResString(IDS_PRIONORMAL);break;
				case 11: HTTPTemp=GetResString(IDS_PRIOAUTONORMAL);break;

				case 2: HTTPTemp=GetResString(IDS_PRIOHIGH);break;
				case 12: HTTPTemp=GetResString(IDS_PRIOAUTOHIGH);break;
			}

			HTTPProcessData.Replace(_T("[PrioVal]"), HTTPTemp);
			HTTPProcessData.Replace(_T("[6]"), sActions);

			HTTPTemp.Format(_T("%.1f%%"),FilesArray[i].m_dblCompleted);
			HTTPProcessData.Replace(_T("[FileProgress]"), HTTPTemp);

			HTTPProcessData.Replace(_T("[FileHash]"),FilesArray[i].sFileHash);

			sDownList += HTTPProcessData;
		}

		//Navigation Line
		CString strNavLine,strNavUrl;
		int pos,pos2;
		strNavUrl = Data.sURL;
		if((pos=strNavUrl.Find(_T("startpos="),0))>1){
			if (strNavUrl.Mid(pos-1,1)==_T("&")) pos--;
			pos2=strNavUrl.Find(_T("&"),pos+1);
			if (pos2==-1) pos2 = strNavUrl.GetLength();
			strNavUrl = strNavUrl.Left(pos) + strNavUrl.Right(strNavUrl.GetLength()-pos2);
		}	
		strNavUrl=_SpecialChars(strNavUrl);

		if(startpos>0){
			startpos-=thePrefs.GetWapMaxItemsInPages();
			if (startpos<0) startpos = 0;
			strNavLine.Format(_T("<a href=\".") + strNavUrl + _T("&amp;startpos=%d\">") + _GetPlainResString(IDS_WAP_PREV) + _T("</a>&nbsp;"), startpos);
		}
		if(endpos<FilesArray.GetCount()){
			CString strTemp;
			strTemp.Format(_T("<a href=\".") + strNavUrl + _T("&amp;startpos=%d\">") + _GetPlainResString(IDS_WAP_NEXT) + _T("</a> "), endpos);
			strNavLine += strTemp;
		}
		Out.Replace(_T("[NavigationLine]"), strNavLine);

		Out.Replace(_T("[DownloadFilesListLite]"), sDownList);

		// Elandal: cast from float to integral type always drops fractions.
		// avoids implicit cast warning
		Out.Replace(_T("[TotalDownSize]"), CastItoXBytes((uint64)fTotalSize));
		Out.Replace(_T("[TotalDownTransferred]"), CastItoXBytes((uint64)fTotalTransferred));

		Out.Replace(_T("[ClearCompletedButton]"),(completedAv && IsSessionAdmin(Data,sSession))?pThis->m_Templates.sClearCompleted :_T(""));

		HTTPTemp.Format(_T("%8.2f %s"), fTotalSpeed/1024.0,_GetPlainResString(IDS_KBYTESPERSEC));
		Out.Replace(_T("[TotalDownSpeed]"), HTTPTemp);
		
		Out.Replace(_T("[CLEARCOMPLETED]"),_GetPlainResString(IDS_DL_CLEAR));

		CString buffer;
		buffer.Format(_T("%s (%i)"), _GetPlainResString(IDS_TW_DOWNLOADS),FilesArray.GetCount());
		Out.Replace(_T("[DownloadList]"),buffer);
		Out.Replace(_T("[CatSel]"),sCat);
		
	}

	
	//MORPH START - Changed by SiRoB, [itsonlyme: -modname-]
	/*
	Out.Replace(_T("[version]"), _SpecialChars(theApp.m_strCurVersionLong));
	*/
	Out.Replace(_T("[version]"), _SpecialChars(theApp.m_strCurVersionLong + _T(" [") + theApp.m_strModLongVersion + _T("]")));
	//MORPH END   - Changed by SiRoB, [itsonlyme: -modname-]
	
	
	Out.Replace(_T("[eMuleAppName]"), _T("eMule"));
	Out.Replace(_T("[Home]"), _GetPlainResString(IDS_WAP_HOME));
	Out.Replace(_T("[Back]"), _GetPlainResString(IDS_WAP_BACK));
	/* Commented until we put addon feature
	Out.Replace(_T("[PriorityUp]"), _GetPlainResString(IDS_PRIORITY_UP));
	Out.Replace(_T("[PriorityDown]"), _GetPlainResString(IDS_PRIORITY_DOWN));
	*/
	Out.Replace(_T("[Filename]"), _GetPlainResString(IDS_DL_FILENAME));
	Out.Replace(_T("[Size]"), _GetPlainResString(IDS_DL_SIZE));
	Out.Replace(_T("[Transferred]"), _GetPlainResString(IDS_COMPLETE));
	Out.Replace(_T("[Progress]"), _GetPlainResString(IDS_DL_PROGRESS));
	Out.Replace(_T("[Speed]"), _GetPlainResString(IDS_DL_SPEED));
	Out.Replace(_T("[Sources]"), _GetPlainResString(IDS_DL_SOURCES));
	Out.Replace(_T("[Actions]"), _GetPlainResString(IDS_WEB_ACTIONS));
	Out.Replace(_T("[User]"), _GetPlainResString(IDS_QL_USERNAME));
	Out.Replace(_T("[TotalDown]"), _GetPlainResString(IDS_INFLST_USER_TOTALDOWNLOAD));
	Out.Replace(_T("[Prio]"), _GetPlainResString(IDS_PRIORITY));
	Out.Replace(_T("[CatSel]"),sCat);
	Out.Replace(_T("[Session]"), sSession);
	Out.Replace(_T("[Section]"), _GetPlainResString(IDS_TW_DOWNLOADS));
	Out.Replace(_T("[SortBy]"), _GetPlainResString(IDS_WAP_SORTBY));

	return Out;
}

CString CWapServer::_GetTransferUpList(WapThreadData Data)
{
	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString sSession = _ParseURL(Data.sURL, _T("ses"));

	CString Out = _T(""), OutE = _T("");

	Out += pThis->m_Templates.sTransferUpList;

	//MORPH START - Changed by SiRoB, [itsonlyme: -modname-]
	/*
	Out.Replace(_T("[version]"), _SpecialChars(theApp.m_strCurVersionLong));
	*/
	Out.Replace(_T("[version]"), _SpecialChars(theApp.m_strCurVersionLong + _T(" [") + theApp.m_strModLongVersion + _T("]")));
	//MORPH END   - Changed by SiRoB, [itsonlyme: -modname-]
	
	
	Out.Replace(_T("[eMuleAppName]"), _T("eMule"));
	Out.Replace(_T("[Section]"), _GetPlainResString(IDS_PW_CON_UPLBL));
	Out.Replace(_T("[Home]"), _GetPlainResString(IDS_WAP_HOME));
	Out.Replace(_T("[Back]"), _GetPlainResString(IDS_WAP_BACK));
	
	OutE = pThis->m_Templates.sTransferUpLine;

	float fTotalSize = 0, fTotalTransferred = 0, fTotalSpeed = 0;

	CString sUpList = _T("");
	int startpos=0,endpos;

	if(_ParseURL(Data.sURL, _T("startpos"))!=_T(""))
		startpos=_ttoi(_ParseURL(Data.sURL, _T("startpos")));

	endpos = startpos + thePrefs.GetWapMaxItemsInPages();

	if (endpos>theApp.uploadqueue->GetUploadQueueLength())
		endpos=theApp.uploadqueue->GetUploadQueueLength();

	if (startpos>theApp.uploadqueue->GetUploadQueueLength()){
		startpos=endpos-thePrefs.GetWapMaxItemsInPages();
		if (startpos<0) startpos=0;
	}

	int fCount = 0;
	for (POSITION pos = theApp.uploadqueue->GetFirstFromUploadList();
		pos != 0 && fCount < endpos;theApp.uploadqueue->GetNextFromUploadList(pos))
	{
		if(fCount >= startpos){
			CUpDownClient* cur_client = theApp.uploadqueue->GetQueueClientAt(pos);
			CString HTTPProcessData = OutE;
			HTTPProcessData.Replace(_T("[1]"), _SpecialChars(cur_client->GetUserName()));
			HTTPProcessData.Replace(_T("[FileInfo]"), _SpecialChars(GetUploadFileInfo(cur_client)));

			CKnownFile* file = theApp.sharedfiles->GetFileByID(cur_client->GetUploadFileID());
			if (file)
				HTTPProcessData.Replace(_T("[2]"), _SpecialChars(CString(file->GetFileName())));
			else
				HTTPProcessData.Replace(_T("[2]"), _GetPlainResString(IDS_REQ_UNKNOWNFILE));

			fTotalSize += cur_client->GetTransferredDown();
			fTotalTransferred += cur_client->GetTransferredUp();
			CString HTTPTemp;
			HTTPTemp.Format(_T("%s / %s"), CastItoXBytes(cur_client->GetTransferredDown()),CastItoXBytes(cur_client->GetTransferredUp()));
			HTTPProcessData.Replace(_T("[3]"), HTTPTemp);

			fTotalSpeed += cur_client->GetDatarate();
			HTTPTemp.Format(_T("%8.2f ") + _GetPlainResString(IDS_KBYTESPERSEC), cur_client->GetDatarate()/1024.0);
			HTTPProcessData.Replace(_T("[4]"), HTTPTemp);

			sUpList += HTTPProcessData;
		}
		fCount++;
	}

	//Navigation Line
	CString strNavLine,strNavUrl;
	int position,position2;
	strNavUrl = Data.sURL;
	if((position=strNavUrl.Find(_T("startpos="),0))>1){
		if (strNavUrl.Mid(position-1,1)==_T("&")) position--;
		position2=strNavUrl.Find(_T("&"),position+1);
		if (position2==-1) position2 = strNavUrl.GetLength();
		strNavUrl = strNavUrl.Left(position) + strNavUrl.Right(strNavUrl.GetLength()-position2);
	}	

	strNavUrl=_SpecialChars(strNavUrl);

	if(startpos>0){
		startpos-=thePrefs.GetWapMaxItemsInPages();
		if (startpos<0) startpos = 0;
		strNavLine.Format(_T("<a href=\".") + strNavUrl + _T("&amp;startpos=%d\">") + _GetPlainResString(IDS_WAP_PREV) + _T("</a>&nbsp;"), startpos);
	}
	if(endpos<theApp.uploadqueue->GetUploadQueueLength()){
		CString strTemp;
		strTemp.Format(_T("<a href=\".") + strNavUrl + _T("&amp;startpos=%d\">") + _GetPlainResString(IDS_WAP_NEXT) + _T("</a> "), endpos);
		strNavLine += strTemp;
	}
	Out.Replace(_T("[NavigationLine]"), strNavLine);

	Out.Replace(_T("[UploadFilesList]"), sUpList);
	// Elandal: cast from float to integral type always drops fractions.
	// avoids implicit cast warning
	CString HTTPTemp;
	HTTPTemp.Format(_T("%s / %s"), CastItoXBytes((uint64)fTotalSize), CastItoXBytes((uint64)fTotalTransferred));
	Out.Replace(_T("[TotalUpTransferred]"), HTTPTemp);
	HTTPTemp.Format(_T("%8.2f ") + _GetPlainResString(IDS_KBYTESPERSEC), fTotalSpeed/1024.0);
	Out.Replace(_T("[TotalUpSpeed]"), HTTPTemp);

	Out.Replace(_T("[Session]"), sSession);

	CString buffer;
	buffer.Format(_T("%s (%i)"),_GetPlainResString(IDS_PW_CON_UPLBL),theApp.uploadqueue->GetUploadQueueLength());
	Out.Replace(_T("[UploadList]"), buffer );
	
	Out.Replace(_T("[Filename]"), _GetPlainResString(IDS_DL_FILENAME));
	Out.Replace(_T("[Transferred]"), _GetPlainResString(IDS_COMPLETE));
	Out.Replace(_T("[Speed]"), _GetPlainResString(IDS_DL_SPEED));
	Out.Replace(_T("[User]"), _GetPlainResString(IDS_QL_USERNAME));
	Out.Replace(_T("[TotalUp]"), _GetPlainResString(IDS_INFLST_USER_TOTALUPLOAD));

	return Out;
}

CString CWapServer::_GetTransferQueueList(WapThreadData Data)
{
	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString sSession = _ParseURL(Data.sURL, _T("ses"));

	CString Out = _T(""), OutE = _T("");

	Out=pThis->m_Templates.sTransferUpQueueList;

	
	//MORPH START - Changed by SiRoB, [itsonlyme: -modname-]
	/*
	Out.Replace(_T("[version]"), _SpecialChars(theApp.m_strCurVersionLong));
	*/
	Out.Replace(_T("[version]"), _SpecialChars(theApp.m_strCurVersionLong + _T(" [") + theApp.m_strModLongVersion + _T("]")));
	//MORPH END   - Changed by SiRoB, [itsonlyme: -modname-]
	
	
	Out.Replace(_T("[eMuleAppName]"), _T("eMule"));
	Out.Replace(_T("[Section]"), _GetPlainResString(IDS_ONQUEUE));
	Out.Replace(_T("[Home]"), _GetPlainResString(IDS_WAP_HOME));
	Out.Replace(_T("[Back]"), _GetPlainResString(IDS_WAP_BACK));

	CString buffer;
	buffer.Format(_T("%s (%i)"),_GetPlainResString(IDS_ONQUEUE),theApp.uploadqueue->GetWaitingUserCount());
	Out.Replace(_T("[UploadQueueList]"), buffer );

	OutE = pThis->m_Templates.sTransferUpQueueLine;
	// Replace [xx]
	CString sQueue = _T("");

	int startpos=0,endpos;

	if(_ParseURL(Data.sURL, _T("startpos"))!=_T(""))
		startpos=_ttoi(_ParseURL(Data.sURL, _T("startpos")));

	endpos = startpos + thePrefs.GetWapMaxItemsInPages();

	if (endpos>theApp.uploadqueue->GetWaitingUserCount())
		endpos=theApp.uploadqueue->GetWaitingUserCount();

	if (startpos>theApp.uploadqueue->GetWaitingUserCount()){
		startpos=endpos-thePrefs.GetWapMaxItemsInPages();
		if (startpos<0) startpos=0;
	}

	int fCount = 0;
	for (POSITION pos = theApp.uploadqueue->GetFirstFromWaitingList(); pos != 0 && fCount<endpos ;theApp.uploadqueue->GetNextFromWaitingList(pos)){
		if(fCount>=startpos){
			CUpDownClient* cur_client = theApp.uploadqueue->GetWaitClientAt(pos);
			CString HTTPProcessData;
			TCHAR HTTPTempC[100] = _T("");
			HTTPProcessData = OutE;
			HTTPProcessData.Replace(_T("[UserName]"), _SpecialChars(cur_client->GetUserName()));
			CKnownFile* file = theApp.sharedfiles->GetFileByID(cur_client->GetUploadFileID());
			if (file)
				HTTPProcessData.Replace(_T("[FileName]"), _SpecialChars(file->GetFileName()));
			else
				HTTPProcessData.Replace(_T("[FileName]"), _T("?"));
			_stprintf(HTTPTempC, _T("%i") , cur_client->GetScore(false));
			CString HTTPTemp = HTTPTempC;
			HTTPProcessData.Replace(_T("[Score]"), HTTPTemp);
			//MORPH - Changed by SiRoB, Code Optimization
			/*		
			if (cur_client->IsBanned())
			*/
			if (cur_client->GetUploadState() == US_BANNED)
				HTTPProcessData.Replace(_T("[Banned]"), _GetPlainResString(IDS_YES));
			else
				HTTPProcessData.Replace(_T("[Banned]"), _GetPlainResString(IDS_NO));
			sQueue += HTTPProcessData;
		}
		fCount++;
	}
	//Navigation Line
	CString strNavLine,strNavUrl;
	int position,position2;
	strNavUrl = Data.sURL;
	if((position=strNavUrl.Find(_T("startpos="),0))>1){
		if (strNavUrl.Mid(position-1,1)==_T("&")) position--;
		position2=strNavUrl.Find(_T("&"),position+1);
		if (position2==-1) position2 = strNavUrl.GetLength();
		strNavUrl = strNavUrl.Left(position) + strNavUrl.Right(strNavUrl.GetLength()-position2);
	}	
	strNavUrl=_SpecialChars(strNavUrl);

	if(startpos>0){
		startpos-=thePrefs.GetWapMaxItemsInPages();
		if (startpos<0) startpos = 0;
		strNavLine.Format(_T("<a href=\".") + strNavUrl + _T("&amp;startpos=%d\">") + _GetPlainResString(IDS_WAP_PREV) + _T("</a>&nbsp;"), startpos);
	}
	if(endpos<theApp.uploadqueue->GetWaitingUserCount()){
		CString strTemp;
		strTemp.Format(_T("<a href=\".") + strNavUrl + _T("&amp;startpos=%d\">") + _GetPlainResString(IDS_WAP_NEXT) + _T("</a> "), endpos);
		strNavLine += strTemp;
	}
	Out.Replace(_T("[NavigationLine]"), strNavLine);

	Out.Replace(_T("[QueueList]"), sQueue);
	Out.Replace(_T("[Session]"), sSession);
	Out.Replace(_T("[UserNameTitle]"), _GetPlainResString(IDS_QL_USERNAME));
	Out.Replace(_T("[FileNameTitle]"), _GetPlainResString(IDS_DL_FILENAME));
	Out.Replace(_T("[ScoreTitle]"), _GetPlainResString(IDS_SCORE));
	Out.Replace(_T("[BannedTitle]"), _GetPlainResString(IDS_BANNED));

	return Out;
}

CString CWapServer::_GetDownloadLink(WapThreadData Data)
{
	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString sSession = _ParseURL(Data.sURL, _T("ses"));

	if (!IsSessionAdmin(Data,sSession)) {
		CString ad=_T("<br/><br/>[Message]");
		ad.Replace(_T("[Message]"),_GetPlainResString(IDS_ACCESSDENIED));
		return ad;
	}

	CString Out = pThis->m_Templates.sDownloadLink;

	Out.Replace(_T("[Download]"), _GetPlainResString(IDS_SW_DOWNLOAD));
	Out.Replace(_T("[Ed2klink]"), _GetPlainResString(IDS_SW_LINK));
	Out.Replace(_T("[Start]"), _GetPlainResString(IDS_SW_START));
	Out.Replace(_T("[Session]"), sSession);
	Out.Replace(_T("[Section]"), _GetPlainResString(IDS_SW_LINK));
	Out.Replace(_T("[Home]"), _GetPlainResString(IDS_WAP_HOME));
	Out.Replace(_T("[Back]"), _GetPlainResString(IDS_WAP_BACK));

	if (thePrefs.GetCatCount()>1){
		InsertCatBox(Out,0,_GetPlainResString(IDS_TOCAT)+_T(":&nbsp;<br/>"));
		Out.Replace(_T("[URL]"), _SpecialChars(_T(".") + Data.sURL));
	}else Out.Replace(_T("[CATBOX]"),_T(""));

	return Out;
}

CString CWapServer::_GetSharedFilesList(WapThreadData Data)
{
	CWapServer *pThis = (CWapServer *)Data.pThis;
        CString sSession = _ParseURL(Data.sURL, _T("ses"));
	if(pThis == NULL)
		return _T("");
	if (_ParseURL(Data.sURL, _T("sort")) != _T("")) 
	{
		if(_ParseURL(Data.sURL, _T("sort")) == _T("name"))
			pThis->m_Params.SharedSort = SHARED_SORT_NAME;
		else
		if(_ParseURL(Data.sURL, _T("sort")) == _T("size"))
			pThis->m_Params.SharedSort = SHARED_SORT_SIZE;
		else
		if(_ParseURL(Data.sURL, _T("sort")) == _T("transferred"))
			pThis->m_Params.SharedSort = SHARED_SORT_TRANSFERRED;
		else
		if(_ParseURL(Data.sURL, _T("sort")) == _T("alltimetransferred"))
			pThis->m_Params.SharedSort = SHARED_SORT_ALL_TIME_TRANSFERRED;
		else
		if(_ParseURL(Data.sURL, _T("sort")) == _T("requests"))
			pThis->m_Params.SharedSort = SHARED_SORT_REQUESTS;
		else
		if(_ParseURL(Data.sURL, _T("sort")) == _T("alltimerequests"))
			pThis->m_Params.SharedSort = SHARED_SORT_ALL_TIME_REQUESTS;
		else
		if(_ParseURL(Data.sURL, _T("sort")) == _T("accepts"))
			pThis->m_Params.SharedSort = SHARED_SORT_ACCEPTS;
		else
		if(_ParseURL(Data.sURL, _T("sort")) == _T("alltimeaccepts"))
			pThis->m_Params.SharedSort = SHARED_SORT_ALL_TIME_ACCEPTS;
		else
		if(_ParseURL(Data.sURL, _T("sort")) == _T("priority"))
			pThis->m_Params.SharedSort = SHARED_SORT_PRIORITY;

		if(_ParseURL(Data.sURL, _T("sortreverse")) == _T(""))
			pThis->m_Params.bSharedSortReverse = false;
	}
	if (_ParseURL(Data.sURL, _T("sortreverse")) != "") 
		pThis->m_Params.bSharedSortReverse = (_ParseURL(Data.sURL, _T("sortreverse")) == "true");

	if(_ParseURL(Data.sURL, _T("hash")) != "" && _ParseURL(Data.sURL, _T("setpriority")) != "" && IsSessionAdmin(Data,sSession)) 
		_SetSharedFilePriority(_ParseURL(Data.sURL, _T("hash")),(uint8)_ttoi(_ParseURL(Data.sURL, _T("setpriority"))));

	if(_ParseURL(Data.sURL, _T("reload")) == "true")
	{
		SendMessage(theApp.emuledlg->m_hWnd,WEB_GUI_INTERACTION,WEBGUIIA_SHARED_FILES_RELOAD,0);
	}

	CString sSharedSortRev;
	if(pThis->m_Params.bSharedSortReverse)
		sSharedSortRev = _T("false");
	else
		sSharedSortRev = _T("true");
    
	CString Out = pThis->m_Templates.sSharedList;
	int sortpos=0;
	//Name sorting link	
	if(pThis->m_Params.SharedSort == SHARED_SORT_NAME){
		Out.Replace(_T("[SortName]"), _T("./?ses=[Session]&amp;w=shared&amp;sort=name&amp;sortreverse=") + sSharedSortRev);
		sortpos=1;
		CString strSort=_GetPlainResString(IDS_DL_FILENAME);
		if (pThis->m_Params.bSharedSortReverse)
			strSort+=_T(" (-)");
		else
			strSort+=_T(" (+)");
		Out.Replace(_T("[FilenameTitle]"), strSort);
	}else
		Out.Replace(_T("[SortName]"), _T("./?ses=[Session]&amp;w=shared&amp;sort=name"));
	//Size sorting Link
	if(pThis->m_Params.SharedSort == SHARED_SORT_SIZE){
		Out.Replace(_T("[SortSize]"), _T("./?ses=[Session]&amp;w=shared&amp;sort=size&amp;sortreverse=") + sSharedSortRev);
		sortpos=2;
		CString strSort=_GetPlainResString(IDS_DL_SIZE);
		if (pThis->m_Params.bSharedSortReverse)
			strSort+=_T(" (-)");
		else
			strSort+=_T(" (+)");
		Out.Replace(_T("[SizeTitle]"), strSort);
	}else
		Out.Replace(_T("[SortSize]"), _T("./?ses=[Session]&amp;w=shared&amp;sort=size"));
	//Priority sorting Link
	if(pThis->m_Params.SharedSort == SHARED_SORT_PRIORITY){
		Out.Replace(_T("[SortPriority]"), _T("./?ses=[Session]&amp;w=shared&amp;sort=priority&amp;sortreverse=") + sSharedSortRev);
		sortpos=3;
		CString strSort=_GetPlainResString(IDS_PRIORITY);
		if (pThis->m_Params.bSharedSortReverse)
			strSort+=_T(" (-)");
		else
			strSort+=_T(" (+)");
		Out.Replace(_T("[PriorityTitle]"), strSort);
	}else
		Out.Replace(_T("[SortPriority]"), _T("./?ses=[Session]&amp;w=shared&amp;sort=priority"));
    //Transferred sorting link
	if(pThis->m_Params.SharedSort == SHARED_SORT_TRANSFERRED)
	{
		sortpos=4;
		if(pThis->m_Params.bSharedSortReverse)
            Out.Replace(_T("[SortTransferred]"), _T("./?ses=[Session]&amp;w=shared&amp;sort=alltimetransferred&amp;sortreverse=") + sSharedSortRev);
		else
			Out.Replace(_T("[SortTransferred]"), _T("./?ses=[Session]&amp;w=shared&amp;sort=transferred&amp;sortreverse=") + sSharedSortRev);

		CString strSort=_GetPlainResString(IDS_SF_TRANSFERRED);
		if (pThis->m_Params.bSharedSortReverse)
			strSort+=_T(" (-)");
		else
			strSort+=_T(" (+)");
		Out.Replace(_T("[FileTransferredTitle]"), strSort);
	}
	else
	if(pThis->m_Params.SharedSort == SHARED_SORT_ALL_TIME_TRANSFERRED)
	{
		sortpos=4;
		if(pThis->m_Params.bSharedSortReverse)
            Out.Replace(_T("[SortTransferred]"), _T("./?ses=[Session]&amp;w=shared&amp;sort=transferred&amp;sortreverse=") + sSharedSortRev);
		else
			Out.Replace(_T("[SortTransferred]"), _T("./?ses=[Session]&amp;w=shared&amp;sort=alltimetransferred&amp;sortreverse=") + sSharedSortRev);

		CString strSort=_GetPlainResString(IDS_SF_TRANSFERRED);
		if (pThis->m_Params.bSharedSortReverse)
			strSort+=_T(" (-)");
		else
			strSort+=_T(" (+)");
		Out.Replace(_T("[FileTransferredTitle]"), strSort);
	}
	else
        Out.Replace(_T("[SortTransferred]"), _T("./?ses=[Session]&amp;w=shared&amp;sort=transferred&amp;sortreverse=false"));
    //Request sorting link
	if(pThis->m_Params.SharedSort == SHARED_SORT_REQUESTS)
	{
		sortpos=5;
		if(pThis->m_Params.bSharedSortReverse)
            Out.Replace(_T("[SortRequests]"), _T("./?ses=[Session]&amp;w=shared&amp;sort=alltimerequests&amp;sortreverse=") + sSharedSortRev);
		else
			Out.Replace(_T("[SortRequests]"), _T("./?ses=[Session]&amp;w=shared&amp;sort=requests&amp;sortreverse=") + sSharedSortRev);

		CString strSort=_GetPlainResString(IDS_SF_REQUESTS);
		if (pThis->m_Params.bSharedSortReverse)
			strSort+=_T(" (-)");
		else
			strSort+=_T(" (+)");
		Out.Replace(_T("[FileRequestsTitle]"), strSort);
	}
	else
	if(pThis->m_Params.SharedSort == SHARED_SORT_ALL_TIME_REQUESTS)
	{
		sortpos=5;
		if(pThis->m_Params.bSharedSortReverse)
            Out.Replace(_T("[SortRequests]"), _T("./?ses=[Session]&amp;w=shared&amp;sort=requests&amp;sortreverse=") + sSharedSortRev);
		else
			Out.Replace(_T("[SortRequests]"), _T("./?ses=[Session]&amp;w=shared&amp;sort=alltimerequests&amp;sortreverse=") + sSharedSortRev);

		CString strSort=_GetPlainResString(IDS_SF_REQUESTS);
		if (pThis->m_Params.bSharedSortReverse)
			strSort+=_T(" (-)");
		else
			strSort+=_T(" (+)");
		Out.Replace(_T("[FileRequestsTitle]"), strSort);
	}
	else
        Out.Replace(_T("[SortRequests]"), _T("./?ses=[Session]&amp;w=shared&amp;sort=requests&amp;sortreverse=false"));
    //Accepts sorting link
	if(pThis->m_Params.SharedSort == SHARED_SORT_ACCEPTS)
	{
		sortpos=6;
		if(pThis->m_Params.bSharedSortReverse)
            Out.Replace(_T("[SortAccepts]"), _T("./?ses=[Session]&amp;w=shared&amp;sort=alltimeaccepts&amp;sortreverse=") + sSharedSortRev);
		else
			Out.Replace(_T("[SortAccepts]"), _T("./?ses=[Session]&amp;w=shared&amp;sort=accepts&amp;sortreverse=") + sSharedSortRev);

		CString strSort=_GetPlainResString(IDS_SF_ACCEPTS);
		if (pThis->m_Params.bSharedSortReverse)
			strSort+=_T(" (-)");
		else
			strSort+=_T(" (+)");
		Out.Replace(_T("[FileAcceptsTitle]"), strSort);
	}
	else
	if(pThis->m_Params.SharedSort == SHARED_SORT_ALL_TIME_ACCEPTS)
	{
		sortpos=6;
		if(pThis->m_Params.bSharedSortReverse)
            Out.Replace(_T("[SortAccepts]"), _T("./?ses=[Session]&amp;w=shared&amp;sort=accepts&amp;sortreverse=") + sSharedSortRev);
		else
			Out.Replace(_T("[SortAccepts]"), _T("./?ses=[Session]&amp;w=shared&amp;sort=alltimeaccepts&amp;sortreverse=") + sSharedSortRev);

		CString strSort=_GetPlainResString(IDS_SF_ACCEPTS);
		if (pThis->m_Params.bSharedSortReverse)
			strSort+=_T(" (-)");
		else
			strSort+=_T(" (+)");
		Out.Replace(_T("[FileAcceptsTitle]"), strSort);
	}
	else
        Out.Replace(_T("[SortAccepts]"), _T("./?ses=[Session]&amp;w=shared&amp;sort=accepts&amp;sortreverse=false"));

	//[SortByVal:x1,x2,x3,...]
	int sortbypos=Out.Find(_T("[SortByVal:"),0);
	if(sortbypos!=-1){
		int sortbypos2=Out.Find(_T("]"),sortbypos+1);
		if(sortbypos2!=-1){
			CString strSortByArray=Out.Mid(sortbypos+11,sortbypos2-sortbypos-11);
			CString resToken;
			int curPos = 0,curPos2=0;
			bool posfound=false;
			resToken= strSortByArray.Tokenize(_T(","),curPos);
			while (resToken != "" && !posfound)
			{
				curPos2++;
				if(sortpos==_ttoi(resToken)){
					CString strTemp;
					strTemp.Format(_T("%i"),curPos2);
					Out.Replace(Out.Mid(sortbypos,sortbypos2-sortbypos+1),strTemp);
					posfound=true;
				}
				resToken= strSortByArray.Tokenize(_T(","),curPos);
			};
			if(!posfound)
				Out.Replace(Out.Mid(sortbypos,sortbypos2-sortbypos+1),_T("1"));
		}
	}

	if(_ParseURL(Data.sURL, _T("reload")) == "true")
	{
		//CString resultlog = _SpecialChars(theApp.emuledlg->logtext);	//Pick-up last line of the log
		//resultlog = resultlog.TrimRight('\n');
		//resultlog = resultlog.Mid(resultlog.ReverseFind('\n'));
		CString resultlog = _SpecialChars(theApp.emuledlg->GetLastLogEntry());
		Out.Replace(_T("[Message]"),resultlog+_T("<br/><br/>"));
	}
	else
        Out.Replace(_T("[Message]"),_T(""));

	Out.Replace(_T("[Ed2klink]"), _GetPlainResString(IDS_SW_LINK));
	Out.Replace(_T("[Reload]"), _GetPlainResString(IDS_SF_RELOAD));
	Out.Replace(_T("[Session]"), sSession);

	CString OutE = pThis->m_Templates.sSharedLine; 

	OutE.Replace(_T("[Ed2klink]"), _GetPlainResString(IDS_SW_LINK));
    /* Commented until we put addon feature
	OutE.Replace(_T("[PriorityUp]"), _GetPlainResString(IDS_PRIORITY_UP));
    OutE.Replace(_T("[PriorityDown]"), _GetPlainResString(IDS_PRIORITY_DOWN));
	*/
	CString OutE2 = pThis->m_Templates.sSharedLineChanged; 

	OutE2.Replace(_T("[Ed2klink]"), _GetPlainResString(IDS_SW_LINK));
    /* Commented until we put addon feature
	OutE2.Replace(_T("[PriorityUp]"), _GetPlainResString(IDS_PRIORITY_UP));
    OutE2.Replace(_T("[PriorityDown]"), _GetPlainResString(IDS_PRIORITY_DOWN));
	*/

	CArray<SharedFiles, SharedFiles> SharedArray;
	// Populating array
	for (int ix=0;ix<theApp.sharedfiles->GetCount();ix++)
	{
		CCKey bufKey;
		CKnownFile* cur_file;
		cur_file=theApp.sharedfiles->GetFileByIndex(ix);// m_Files_map.GetNextAssoc(pos,bufKey,cur_file);

		SharedFiles dFile;
		dFile.sFileName = _SpecialChars(cur_file->GetFileName());
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

		dFile.sFileHash = EncodeBase16(cur_file->GetFileHash(), 16);
		if (cur_file->IsAutoUpPriority())
        {
            if (cur_file->GetUpPriority() == PR_LOW)
                dFile.sFilePriority = _GetPlainResString(IDS_PRIOAUTOLOW);
            else if (cur_file->GetUpPriority() == PR_NORMAL)
				dFile.sFilePriority = _GetPlainResString(IDS_PRIOAUTONORMAL);
			else if (cur_file->GetUpPriority() == PR_HIGH)
				dFile.sFilePriority = _GetPlainResString(IDS_PRIOAUTOHIGH);
			else if (cur_file->GetUpPriority() == PR_VERYHIGH)
				dFile.sFilePriority = _GetPlainResString(IDS_PRIOAUTORELEASE);
		}
		else
		{
			if (cur_file->GetUpPriority() == PR_VERYLOW)
				dFile.sFilePriority = _GetPlainResString(IDS_PRIOVERYLOW);
			else if (cur_file->GetUpPriority() == PR_LOW)
				dFile.sFilePriority = _GetPlainResString(IDS_PRIOLOW);
			else if (cur_file->GetUpPriority() == PR_NORMAL)
				dFile.sFilePriority = _GetPlainResString(IDS_PRIONORMAL);
			else if (cur_file->GetUpPriority() == PR_HIGH)
				dFile.sFilePriority = _GetPlainResString(IDS_PRIOHIGH);
			else if (cur_file->GetUpPriority() == PR_VERYHIGH)
				dFile.sFilePriority = _GetPlainResString(IDS_PRIORELEASE);
		}
		dFile.nFilePriority = cur_file->GetUpPriority();
		dFile.bFileAutoPriority = cur_file->IsAutoUpPriority();
		SharedArray.Add(dFile);
	}

	// Sorting (simple bubble sort, we don't have tons of data here)
	bool bSorted = true;
	
	for(int nMax = 0;bSorted && nMax < SharedArray.GetCount()*2; nMax++)
	{
		bSorted = false;
		for(int i = 0; i < SharedArray.GetCount() - 1; i++)
		{
			bool bSwap = false;
			switch(pThis->m_Params.SharedSort)
			{
			case SHARED_SORT_NAME:
				bSwap = SharedArray[i].sFileName.CompareNoCase(SharedArray[i+1].sFileName) > 0;
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
			case SHARED_SORT_PRIORITY:
				//Very low priority is define equal to 4 ! Must adapte sorting code
				if(SharedArray[i].nFilePriority == 4)
				{
					if(SharedArray[i+1].nFilePriority == 4)
						bSwap = false;
					else
						bSwap = true;
				}        
				else
					if(SharedArray[i+1].nFilePriority == 4)
					{
						if(SharedArray[i].nFilePriority == 4)
							bSwap = true;
						else
							bSwap = false;
					}
					else
						bSwap = SharedArray[i].nFilePriority < SharedArray[i+1].nFilePriority;
				break;
			}
			if(pThis->m_Params.bSharedSortReverse)
			{
				bSwap = !bSwap;
			}
			if(bSwap)
			{
				bSorted = true;
				SharedFiles TmpFile = SharedArray[i];
				SharedArray[i] = SharedArray[i+1];
				SharedArray[i+1] = TmpFile;
			}
		}
	}
	// Displaying
	CString sSharedList = _T("");
	int startpos=0,endpos;

	if(_ParseURL(Data.sURL, _T("startpos"))!="")
		startpos=_ttoi(_ParseURL(Data.sURL, _T("startpos")));

	endpos = startpos + thePrefs.GetWapMaxItemsInPages();

	if (endpos>SharedArray.GetCount())
		endpos=SharedArray.GetCount();

	if (startpos>SharedArray.GetCount()){
		startpos=endpos-thePrefs.GetWapMaxItemsInPages();
		if (startpos<0) startpos=0;
	}

	for(int i = startpos; i < endpos; i++)
	{
		TCHAR HTTPTempC[100] = _T("");
		CString HTTPProcessData;
		if (SharedArray[i].sFileHash == _ParseURL(Data.sURL,_T("hash")) )
            HTTPProcessData = OutE2;
		else
            HTTPProcessData = OutE;

		HTTPProcessData.Replace(_T("[FileName]"), _SpecialChars(SharedArray[i].sFileName));
		if(SharedArray[i].sFileName.GetLength() > SHORT_FILENAME_LENGTH)
            HTTPProcessData.Replace(_T("[ShortFileName]"), _SpecialChars(SharedArray[i].sFileName.Left(SHORT_FILENAME_LENGTH)) + _T("..."));
		else
			HTTPProcessData.Replace(_T("[ShortFileName]"), _SpecialChars(SharedArray[i].sFileName));

		_stprintf(HTTPTempC, _T("%s"),CastItoXBytes(SharedArray[i].m_qwFileSize));
		HTTPProcessData.Replace(_T("[FileSize]"), CString(HTTPTempC));
		HTTPProcessData.Replace(_T("[FileLink]"), SharedArray[i].sED2kLink);

		_stprintf(HTTPTempC, _T("%s"),CastItoXBytes(SharedArray[i].nFileTransferred));
		HTTPProcessData.Replace(_T("[FileTransferred]"), CString(HTTPTempC));

		_stprintf(HTTPTempC, _T("%s"),CastItoXBytes(SharedArray[i].nFileAllTimeTransferred));
		HTTPProcessData.Replace(_T("[FileAllTimeTransferred]"), CString(HTTPTempC));

		_stprintf(HTTPTempC, _T("%i"), SharedArray[i].nFileRequests);
		HTTPProcessData.Replace(_T("[FileRequests]"), CString(HTTPTempC));

		_stprintf(HTTPTempC, _T("%i"), SharedArray[i].nFileAllTimeRequests);
		HTTPProcessData.Replace(_T("[FileAllTimeRequests]"), CString(HTTPTempC));

		_stprintf(HTTPTempC, _T("%i"), SharedArray[i].nFileAccepts);
		HTTPProcessData.Replace(_T("[FileAccepts]"), CString(HTTPTempC));

		_stprintf(HTTPTempC, _T("%i"), SharedArray[i].nFileAllTimeAccepts);
		HTTPProcessData.Replace(_T("[FileAllTimeAccepts]"), CString(HTTPTempC));

		HTTPProcessData.Replace(_T("[Priority]"), SharedArray[i].sFilePriority);

		HTTPProcessData.Replace(_T("[FileHash]"), SharedArray[i].sFileHash);

		uint8 upperpriority=0, lesserpriority=0;
		if(SharedArray[i].nFilePriority == 4)
		{
			upperpriority = 0;	lesserpriority = 4;
		}
		else
		if(SharedArray[i].nFilePriority == 0)
		{
			upperpriority = 1;	lesserpriority = 4;
		}
		else
		if(SharedArray[i].nFilePriority == 1)
		{
			upperpriority = 2;	lesserpriority = 0;
		}
		else
		if(SharedArray[i].nFilePriority == 2)
		{
			upperpriority = 3;	lesserpriority = 1;
		}
		else
		if(SharedArray[i].nFilePriority == 3)
		{
			upperpriority = 5;	lesserpriority = 2;
		}
		else
		if(SharedArray[i].nFilePriority == 5)
		{
			upperpriority = 5;	lesserpriority = 3;
		}
		if(SharedArray[i].bFileAutoPriority)
		{
			upperpriority = 5;	lesserpriority = 3;
		}
        _stprintf(HTTPTempC, _T("%i"), upperpriority);
		HTTPProcessData.Replace(_T("[PriorityUpLink]"), _T("hash=") + SharedArray[i].sFileHash +_T("&amp;setpriority=") + CString(HTTPTempC));
        _stprintf(HTTPTempC, _T("%i"), lesserpriority);
		HTTPProcessData.Replace(_T("[PriorityDownLink]"), _T("hash=") + SharedArray[i].sFileHash +_T("&amp;setpriority=") + CString(HTTPTempC));

		sSharedList += HTTPProcessData;
	}
	Out.Replace(_T("[SharedFilesList]"), sSharedList);
	Out.Replace(_T("[Session]"), sSession);

	//Navigation Line
	CString strNavLine,strNavUrl;
	int position,position2;
	strNavUrl = Data.sURL;
	if((position=strNavUrl.Find(_T("startpos="),0))>1){
		if (strNavUrl.Mid(position-1,1)=="&") position--;
		position2=strNavUrl.Find(_T("&"),position+1);
		if (position2==-1) position2 = strNavUrl.GetLength();
		strNavUrl = strNavUrl.Left(position) + strNavUrl.Right(strNavUrl.GetLength()-position2);
	}	
	strNavUrl=_SpecialChars(strNavUrl);

	if(startpos>0){
		startpos-=thePrefs.GetWapMaxItemsInPages();
		if (startpos<0) startpos = 0;
		strNavLine.Format(_T("<a href=\".") + strNavUrl + _T("&amp;startpos=%d\">") + _GetPlainResString(IDS_WAP_PREV) + _T("</a>&nbsp;"), startpos);
	}
	if(endpos<SharedArray.GetCount()){
		CString strTemp;
		strTemp.Format(_T("<a href=\".") + strNavUrl + _T("&amp;startpos=%d\">") + _GetPlainResString(IDS_WAP_NEXT) + _T("</a> "), endpos);
		strNavLine += strTemp;
	}
	Out.Replace(_T("[NavigationLine]"), strNavLine);

	Out.Replace(_T("[FileHashTitle]"), _GetPlainResString(IDS_FD_HASH));
	Out.Replace(_T("[FilenameTitle]"), _GetPlainResString(IDS_DL_FILENAME));
	Out.Replace(_T("[PriorityTitle]"),  _GetPlainResString(IDS_PRIORITY));
    Out.Replace(_T("[FileTransferredTitle]"),  _GetPlainResString(IDS_SF_TRANSFERRED));
    Out.Replace(_T("[FileRequestsTitle]"),  _GetPlainResString(IDS_SF_REQUESTS));
    Out.Replace(_T("[FileAcceptsTitle]"),  _GetPlainResString(IDS_SF_ACCEPTS));
	Out.Replace(_T("[SizeTitle]"), _GetPlainResString(IDS_DL_SIZE));
	Out.Replace(_T("[Section]"), _GetPlainResString(IDS_SHAREDFILES));
	Out.Replace(_T("[Home]"), _GetPlainResString(IDS_WAP_HOME));
	Out.Replace(_T("[Back]"), _GetPlainResString(IDS_WAP_BACK));
	Out.Replace(_T("[SortBy]"), _GetPlainResString(IDS_WAP_SORTBY));

	return Out;
}

CString CWapServer::_GetGraphs(WapThreadData Data)
{
	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString Out = pThis->m_Templates.sGraphs;
	
	CString sSession = _ParseURL(Data.sURL, _T("ses"));
	Out.Replace(_T("[Session]"), sSession);
	
	Out.Replace(_T("[TxtDownload]"), _GetPlainResString(IDS_DOWNLOAD));
	Out.Replace(_T("[TxtUpload]"), _GetPlainResString(IDS_PW_CON_UPLBL));
	Out.Replace(_T("[TxtTime]"), _GetPlainResString(IDS_TIME));
	Out.Replace(_T("[TxtConnections]"), _GetPlainResString(IDS_SP_ACTCON));
	Out.Replace(_T("[KByteSec]"), _GetPlainResString(IDS_KBYTESPERSEC));
	Out.Replace(_T("[TxtTime]"), _GetPlainResString(IDS_TIME));
	Out.Replace(_T("[Section]"), _GetPlainResString(IDS_GRAPHS));
	Out.Replace(_T("[Home]"), _GetPlainResString(IDS_WAP_HOME));
	Out.Replace(_T("[Back]"), _GetPlainResString(IDS_WAP_BACK));

	CString sScale;
	sScale.Format(_T("%s"), CastSecondsToHM(thePrefs.GetTrafficOMeterInterval() * thePrefs.GetWapGraphWidth()) );

	
	// emulEspaña: modified by MoNKi [Maella: -Allow Bandwidth Settings in <1KB Incremements-]
	/**/ //MORPH - Don't use <1KB Increment just change this /**/ into /* to toggle :)
	CString s1, s2,s3;
	s1.Format(_T("%d"), thePrefs.GetMaxGraphDownloadRate() + 4);
	s2.Format(_T("%d"), thePrefs.GetMaxGraphUploadRate(true) + 4);
	s3.Format(_T("%d"), thePrefs.GetMaxConnections()+20);

	Out.Replace(_T("[ScaleTime]"), sScale);
	Out.Replace(_T("[MaxDownload]"), s1);
	Out.Replace(_T("[MaxUpload]"), s2);
	Out.Replace(_T("[MaxConnections]"), s3);
	/*/
	CString s1, s2, s3;
	s1.Format(_T("%d"), (int)thePrefs.GetMaxGraphDownloadRate() + 4);
	s2.Format(_T("%d"), (int)thePrefs.GetMaxGraphUploadRate() + 4);
	s3.Format(_T("%d"), thePrefs.GetMaxConnections()+20);
	
	Out.Replace("[ScaleTime]", sScale);
	Out.Replace("[MaxDownload]", s1);
	Out.Replace("[MaxUpload]", s2);
	Out.Replace("[MaxConnections]", s3);
	/**/
	// End emulEspaña

	return Out;
}

CString CWapServer::_GetServerOptions(WapThreadData Data)
{

	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString sSession = _ParseURL(Data.sURL, _T("ses"));

	if (!IsSessionAdmin(Data,sSession)) return _T("");

	CString Out = pThis->m_Templates.sServerOptions;

	if(_ParseURL(Data.sURL, _T("addserver")) == "true")
	{
		CServer* nsrv = new CServer((uint16)_ttoi(_ParseURL(Data.sURL, _T("serverport"))), _ParseURL(Data.sURL, _T("serveraddr")));
		nsrv->SetListName(_ParseURL(Data.sURL, _T("servername")));
		theApp.emuledlg->serverwnd->serverlistctrl.AddServer(nsrv,true);
		CString resultlog = _SpecialChars(theApp.emuledlg->GetLastLogEntry());
		Out.Replace(_T("[Message]"),resultlog);
	}
	else
		if(_ParseURL(Data.sURL, _T("updateservermetfromurl")) == "true")
		{
				theApp.emuledlg->serverwnd->UpdateServerMetFromURL(_ParseURL(Data.sURL, _T("servermeturl")));
				//CString resultlog = _SpecialChars(theApp.emuledlg->logtext);
				//resultlog = resultlog.TrimRight('\n');
				//resultlog = resultlog.Mid(resultlog.ReverseFind('\n'));
				CString resultlog = _SpecialChars(theApp.emuledlg->GetLastLogEntry());
				Out.Replace(_T("[Message]"),resultlog);
		}
		else
		Out.Replace(_T("[Message]"), _T(""));
    Out.Replace(_T("[AddServer]"), _GetPlainResString(IDS_SV_NEWSERVER));
	Out.Replace(_T("[IP]"), _GetPlainResString(IDS_SV_ADDRESS));
	Out.Replace(_T("[Port]"), _GetPlainResString(IDS_SV_PORT));
	Out.Replace(_T("[Name]"), _GetPlainResString(IDS_SW_NAME));
	Out.Replace(_T("[Add]"), _GetPlainResString(IDS_SV_ADD));
	Out.Replace(_T("[UpdateServerMetFromURL]"), _GetPlainResString(IDS_SV_MET));
	Out.Replace(_T("[URL]"), _GetPlainResString(IDS_SV_URL));
	Out.Replace(_T("[Apply]"), _GetPlainResString(IDS_PW_APPLY));
	Out.Replace(_T("[Home]"), _GetPlainResString(IDS_WAP_HOME));
	Out.Replace(_T("[Back]"), _GetPlainResString(IDS_WAP_BACK));
	Out.Replace(_T("[Section]"), _GetPlainResString(IDS_SERVER)+ _T(" ") + _GetPlainResString(IDS_EM_PREFS));
	return Out;
}

/*
CString	CWapServer::_GetWebSearch(WapThreadData Data)
{
	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis == NULL)
		return "";

	CString sSession = _ParseURL(Data.sURL, "ses");
    
	CString Out = pThis->m_Templates.sWebSearch;

	if(_ParseURL(Data.sURL, "tosearch") != "")
	{
		CString query;
		CString tosearch = _ParseURL(Data.sURL, "tosearch");
		query = "http://www.filedonkey.com/fdsearch/index.php?media=";
		query += _ParseURL(Data.sURL, "media");
		tosearch = URLEncode(tosearch);
		tosearch.Replace("%20","+");
		query += "&amp;pattern=";
		query += _ParseURL(Data.sURL, "tosearch");
		query += "&amp;action=search&amp;name=FD-Search&amp;op=modload&amp;file=index&amp;requestby=emule";
		Out += "\n<script language=\"javascript\">";
		Out += "\n searchwindow=window.open('"+ query + "','searchwindow');";
		Out += "\n</script>";
	}
	Out.Replace("[Session]", sSession);
	Out.Replace("[Name]", _GetPlainResString(IDS_SW_NAME));
	Out.Replace("[Type]", _GetPlainResString(IDS_TYPE));
	Out.Replace("[Any]", _GetPlainResString(IDS_SEARCH_ANY));
	Out.Replace("[Audio]", _GetPlainResString(IDS_SEARCH_AUDIO));
	Out.Replace("[Video]", _GetPlainResString(IDS_SEARCH_VIDEO));
	Out.Replace("[Other]", "Other");
	Out.Replace("[Search]", _GetPlainResString(IDS_SW_SEARCHBOX));
	Out.Replace("[WebSearch]", _GetPlainResString(IDS_SW_WEBBASED));
	
	return Out;
}*/

CString CWapServer::_GetLog(WapThreadData Data)
{
	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString sSession = _ParseURL(Data.sURL, _T("ses"));

	CString Out = pThis->m_Templates.sLog;

	if (_ParseURL(Data.sURL, _T("clear")) == "yes" && IsSessionAdmin(Data,sSession))
	{
		theApp.emuledlg->ResetLog();
	}
	Out.Replace(_T("[Clear]"), _GetPlainResString(IDS_PW_RESET));

	CString strServerInfo, strServerInfo2, strServerInfo3, strToken;
	CList<CString,CString> logRows;
	int pos=0;
	strServerInfo = _SpecialChars(theApp.emuledlg->GetAllLogEntries());
	strToken=strServerInfo.Tokenize(_T("\r\n"),pos);
	while (strToken != "")
	{
		logRows.AddTail(strToken);
		strToken = strServerInfo.Tokenize(_T("\r\n"),pos);
	};
	
	POSITION pos2 = logRows.GetTailPosition();
	while (pos2 != NULL)
	{
		strToken = logRows.GetPrev(pos2);
		strServerInfo3=strServerInfo2+_T("<br/>")+strToken;
		if((UINT)strServerInfo3.GetLength() > thePrefs.GetWapLogsSize()){
			if(strServerInfo2=="")
				strServerInfo2=strServerInfo3;
			break;
		}
		else{
			strServerInfo2=_T("<br/>")+strToken+strServerInfo2;
		}
	}
	logRows.RemoveAll();
	Out.Replace(_T("[Log]"), strServerInfo2);
	Out.Replace(_T("[Session]"), sSession);
	Out.Replace(_T("[Home]"), _GetPlainResString(IDS_WAP_HOME));
	Out.Replace(_T("[Back]"), _GetPlainResString(IDS_WAP_BACK));
	Out.Replace(_T("[Section]"), _GetPlainResString(IDS_SV_LOG));

	return Out;
}

CString CWapServer::_GetServerInfo(WapThreadData Data)
{
	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString sSession = _ParseURL(Data.sURL, _T("ses"));

	CString Out = pThis->m_Templates.sServerInfo;
	
	if (_ParseURL(Data.sURL, _T("clear")) == "yes")
	{
		theApp.emuledlg->serverwnd->servermsgbox->SetWindowText(_T(""));
	}

	CString strServerInfo, strServerInfo2, strServerInfo3, strToken;
	CList<CString,CString> logRows;
	int pos=0;
	strServerInfo = _SpecialChars(theApp.emuledlg->serverwnd->servermsgbox->GetText());
	strToken=strServerInfo.Tokenize(_T("\r\n"),pos);
	while (strToken != "")
	{
		logRows.AddTail(strToken);
		strToken = strServerInfo.Tokenize(_T("\r\n"),pos);
	};
	
	POSITION pos2 = logRows.GetTailPosition();
	while (pos2 != NULL)
	{
		strToken = logRows.GetPrev(pos2);
		strServerInfo3=strServerInfo2+_T("<br/>")+strToken;
		if((UINT)strServerInfo3.GetLength() > thePrefs.GetWapLogsSize()){
			if(strServerInfo2=="")
				strServerInfo2=strServerInfo3;
			break;
		}
		else{
			strServerInfo2=_T("<br/>")+strToken+strServerInfo2;
		}
	}
	logRows.RemoveAll();
	Out.Replace(_T("[ServerInfo]"), strServerInfo2);
	Out.Replace(_T("[Clear]"), _GetPlainResString(IDS_PW_RESET));
	Out.Replace(_T("[Session]"), sSession);
	Out.Replace(_T("[Home]"), _GetPlainResString(IDS_WAP_HOME));
	Out.Replace(_T("[Back]"), _GetPlainResString(IDS_WAP_BACK));
	Out.Replace(_T("[Section]"), _GetPlainResString(IDS_SV_SERVERINFO));

	return Out;
}

CString CWapServer::_GetDebugLog(WapThreadData Data)
{
	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString sSession = _ParseURL(Data.sURL, _T("ses"));

	CString Out = pThis->m_Templates.sDebugLog;

	if (_ParseURL(Data.sURL, _T("clear")) == "yes" && IsSessionAdmin(Data,sSession))
	{
		theApp.emuledlg->ResetDebugLog();
	}
	Out.Replace(_T("[Clear]"), _GetPlainResString(IDS_PW_RESET));

	CString strServerInfo, strServerInfo2, strServerInfo3, strToken;
	CList<CString,CString> logRows;
	int pos=0;
	strServerInfo = _SpecialChars(theApp.emuledlg->GetAllDebugLogEntries());
	strToken=strServerInfo.Tokenize(_T("\r\n"),pos);
	while (strToken != "")
	{
		logRows.AddTail(strToken);
		strToken = strServerInfo.Tokenize(_T("\r\n"),pos);
	};
	
	POSITION pos2 = logRows.GetTailPosition();
	while (pos2 != NULL)
	{
		strToken = logRows.GetPrev(pos2);
		strServerInfo3=strServerInfo2+_T("<br/>")+strToken;
		if((UINT)strServerInfo3.GetLength() > thePrefs.GetWapLogsSize()){
			if(strServerInfo2=="")
				strServerInfo2=strServerInfo3;
			break;
		}
		else{
			strServerInfo2=_T("<br/>")+strToken+strServerInfo2;
		}
	}
	logRows.RemoveAll();
	Out.Replace(_T("[DebugLog]"), strServerInfo2);
	Out.Replace(_T("[Session]"), sSession);
	Out.Replace(_T("[Home]"), _GetPlainResString(IDS_WAP_HOME));
	Out.Replace(_T("[Back]"), _GetPlainResString(IDS_WAP_BACK));
	Out.Replace(_T("[Section]"), _GetPlainResString(IDS_SV_DEBUGLOG));

	return Out;

}

CString CWapServer::_GetStats(WapThreadData Data)
{
	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString sSession = _ParseURL(Data.sURL, _T("ses"));

	// refresh statistics
	theApp.emuledlg->statisticswnd->ShowStatistics(true);

	CString Out = pThis->m_Templates.sStats;

	if(_ParseURL(Data.sURL, _T("show"))!=""){
		HTREEITEM item;
		item = (HTREEITEM)_tcstoul(_ParseURL(Data.sURL, _T("show")),NULL,16);
		if(theApp.emuledlg->statisticswnd->stattree.ItemExist(item) &&
			theApp.emuledlg->statisticswnd->stattree.ItemHasChildren(item))
		{
			CString strTemp;
			strTemp = _T("<b>") + theApp.emuledlg->statisticswnd->stattree.GetItemText(item) + _T("</b><br/>");
			strTemp += theApp.emuledlg->statisticswnd->stattree.GetWML(false, false, true,theApp.emuledlg->statisticswnd->stattree.GetChildItem(item),1,false);
			Out.Replace(_T("[Stats]"), strTemp);
		}
		else
		{
			Out.Replace(_T("[Stats]"), theApp.emuledlg->statisticswnd->stattree.GetWML(false, true));
		}
	}
	else
	{
		Out.Replace(_T("[Stats]"), theApp.emuledlg->statisticswnd->stattree.GetWML(false, true));
	}

	Out.Replace(_T("[Session]"),sSession);
	Out.Replace(_T("[Section]"), _GetPlainResString(IDS_SF_STATISTICS));
	Out.Replace(_T("[Home]"), _GetPlainResString(IDS_WAP_HOME));
	Out.Replace(_T("[Back]"), _GetPlainResString(IDS_WAP_BACK));

	return Out;

}

CString CWapServer::_GetPreferences(WapThreadData Data)
{
	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString sSession = _ParseURL(Data.sURL, _T("ses"));

	CString Out = pThis->m_Templates.sPreferences;

	Out.Replace(_T("[Session]"), sSession);

	if ((_ParseURL(Data.sURL, _T("saveprefs")) == "true") && IsSessionAdmin(Data,sSession) ) {
		if(_ParseURL(Data.sURL, _T("logssize")) != "")
		{
			UINT logssize = _ttoi(_ParseURL(Data.sURL, _T("logssize")));
			thePrefs.SetWapLogsSize(logssize);
		}
		if(_ParseURL(Data.sURL, _T("itemsperpage"))!="")
		{
			thePrefs.SetWapMaxItemsInPages(_ttoi(_ParseURL(Data.sURL, _T("itemsperpage"))));
		}
		if(_ParseURL(Data.sURL, _T("graphswidth"))!="")
		{
			thePrefs.SetWapGraphWidth(_ttoi(_ParseURL(Data.sURL, _T("graphswidth"))));
		}
		if(_ParseURL(Data.sURL, _T("graphsheight"))!="")
		{
			thePrefs.SetWapGraphHeight(_ttoi(_ParseURL(Data.sURL, _T("graphsheight"))));
		}
		if(_ParseURL(Data.sURL, _T("sendbwimages"))!="")
		{
			thePrefs.SetWapAllwaysSendBWImages(_ParseURL(Data.sURL, _T("sendbwimages")).MakeLower() == "on");
		}
		if(_ParseURL(Data.sURL, _T("sendimages"))!="")
		{
			thePrefs.SetWapSendImages(_ParseURL(Data.sURL, _T("sendimages")).MakeLower() == "on");
		}
		if(_ParseURL(Data.sURL, _T("sendgraphs"))!="")
		{
			thePrefs.SetWapSendGraphs(_ParseURL(Data.sURL, _T("sendgraphs")).MakeLower() == "on");
		}
		if(_ParseURL(Data.sURL, _T("sendprogressbars"))!="")
		{
			thePrefs.SetWapSendProgressBars(_ParseURL(Data.sURL, _T("sendprogressbars")).MakeLower() == "on");
		}

		if(_ParseURL(Data.sURL, _T("filledgraphs")).MakeLower() == "on")
		{
			thePrefs.SetWapGraphsFilled(true);
		}
		if(_ParseURL(Data.sURL, _T("filledgraphs")).MakeLower() == "off")
		{
			thePrefs.SetWapGraphsFilled(false);
		}
		if(_ParseURL(Data.sURL, _T("refresh")) != "")
		{
			thePrefs.SetWebPageRefresh(_ttoi(_ParseURL(Data.sURL, _T("refresh"))));
		}

		
		// emulEspaña: modified by MoNKi [Maella: -Allow Bandwidth Settings in <1KB Incremements-]
		/**/ //MORPH - Don't use <1KB Increment just change this /**/ into /* to toggle :)
		if(_ParseURL(Data.sURL, _T("maxdown")) != "")
		{
			thePrefs.SetMaxDownload(_ttoi(_ParseURL(Data.sURL, _T("maxdown"))));
		}
		if(_ParseURL(Data.sURL, _T("maxup")) != "")
		{
			thePrefs.SetMaxUpload(_ttoi(_ParseURL(Data.sURL, _T("maxup"))));
		}
		int lastmaxgu=thePrefs.GetMaxGraphUploadRate(false);
		int lastmaxgd=thePrefs.GetMaxGraphDownloadRate();

		if(_ParseURL(Data.sURL, _T("maxcapdown")) != "")
		{
			thePrefs.SetMaxGraphDownloadRate(_ttoi(_ParseURL(Data.sURL, _T("maxcapdown"))));
		}
		if(_ParseURL(Data.sURL, _T("maxcapup")) != "")
		{
			thePrefs.SetMaxGraphUploadRate(_ttoi(_ParseURL(Data.sURL, _T("maxcapup"))));
		}

		if(lastmaxgu != thePrefs.GetMaxGraphUploadRate(false)) 
			theApp.emuledlg->statisticswnd->SetARange(false,thePrefs.GetMaxGraphUploadRate(false));
		if(lastmaxgd!=thePrefs.GetMaxGraphDownloadRate())
			theApp.emuledlg->statisticswnd->SetARange(true,thePrefs.GetMaxGraphDownloadRate());
		/*/
		if(_ParseURL(Data.sURL, _T("maxdown")) != "")
		{
			float maxdown = _ttof(_ParseURL(Data.sURL, _T("maxdown")));
			if(maxdown > 0.0f)
				thePrefs.SetMaxDownload(maxdown);
			else
				thePrefs.SetMaxDownload(UNLIMITED);
		}
		if(_ParseURL(Data.sURL, _T("maxup")) != "")
		{
			float maxup = _ttof(_ParseURL(Data.sURL, _T("maxup")));
			if(maxup > 0.0f)
				thePrefs.SetMaxUpload(maxup);
			else
				thePrefs.SetMaxUpload(UNLIMITED);
		}
		if(_ParseURL(Data.sURL, _T("maxcapdown")) != "")
		{
			thePrefs.SetMaxGraphDownloadRate(_ttof(_ParseURL(Data.sURL, _T("maxcapdown"))));
		}
		if(_ParseURL(Data.sURL, _T("maxcapup")) != "")
		{
			thePrefs.SetMaxGraphUploadRate(_ttof(_ParseURL(Data.sURL, _T("maxcapup"))));
		}
		*/
		// End emulEspaña
		
		if(_ParseURL(Data.sURL, _T("maxsources")) != _T(""))
		{
			thePrefs.SetMaxSourcesPerFile(_ttoi(_ParseURL(Data.sURL, _T("maxsources"))));
		}
		if(_ParseURL(Data.sURL, _T("maxconnections")) != _T(""))
		{
			thePrefs.SetMaxConnections(_ttoi(_ParseURL(Data.sURL, _T("maxconnections"))));
		}
		if(_ParseURL(Data.sURL, _T("maxconnectionsperfive")) != _T(""))
		{
			thePrefs.SetMaxConsPerFive(_ttoi(_ParseURL(Data.sURL, _T("maxconnectionsperfive"))));
		}
		thePrefs.SetPreviewPrio((_ParseURL(Data.sURL, _T("firstandlast")).MakeLower() == _T("on")));

		thePrefs.SetNetworkED2K((_ParseURL(Data.sURL, _T("neted2k")).MakeLower() == _T("on")));
		thePrefs.SetNetworkKademlia((_ParseURL(Data.sURL, _T("netkad")).MakeLower() == _T("on")));

		theApp.scheduler->SaveOriginals(); //MORPH - Added by Stulle, Don't reset Connection Settings without reason
	}

	// Fill form
	Out.Replace(_T("[WapGraphFilledVal]"),thePrefs.GetWapGraphsFilled()?_T("1"):_T("2"));
	Out.Replace(_T("[SendGraphsVal]"),thePrefs.GetWapSendGraphs()?_T("1"):_T("2"));
	Out.Replace(_T("[SendBWImagesVal]"),thePrefs.GetWapAllwaysSendBWImages()?_T("1"):_T("2"));
	Out.Replace(_T("[SendImagesVal]"),thePrefs.GetWapSendImages()?_T("1"):_T("2"));
	Out.Replace(_T("[SendProgressBarsVal]"),thePrefs.GetWapSendProgressBars()?_T("1"):_T("2"));
	Out.Replace(_T("[FirstAndLastVal]"),thePrefs.GetPreviewPrio()?_T("1"):_T("2"));

	CString sRefresh;

	sRefresh.Format(_T("%d"), thePrefs.GetWapLogsSize());
	Out.Replace(_T("[LogsSizeVal]"), sRefresh);

	sRefresh.Format(_T("%d"), thePrefs.GetWapGraphHeight());
	Out.Replace(_T("[GraphsHeightVal]"), sRefresh);

	sRefresh.Format(_T("%d"), thePrefs.GetWapGraphWidth());
	Out.Replace(_T("[GraphsWidthVal]"), sRefresh);

	sRefresh.Format(_T("%d"), thePrefs.GetWapMaxItemsInPages());
	Out.Replace(_T("[ItemsPerPageVal]"), sRefresh);

	sRefresh.Format(_T("%d"), thePrefs.GetWebPageRefresh());
	Out.Replace(_T("[RefreshVal]"), sRefresh);

	sRefresh.Format(_T("%d"), thePrefs.GetMaxSourcePerFileDefault());
	Out.Replace(_T("[MaxSourcesVal]"), sRefresh);

	sRefresh.Format(_T("%d"), thePrefs.GetMaxConnections());
	Out.Replace(_T("[MaxConnectionsVal]"), sRefresh);

	sRefresh.Format(_T("%d"), thePrefs.GetMaxConperFive());
	Out.Replace(_T("[MaxConnectionsPer5Val]"), sRefresh);

	Out.Replace(_T("[ED2KVAL]"), (thePrefs.GetNetworkED2K())?_T("1"):_T("2") );
	Out.Replace(_T("[KADVAL]"), (thePrefs.GetNetworkKademlia())?_T("1"):_T("2") );

	Out.Replace(_T("[KBS]"), _GetPlainResString(IDS_KBYTESPERSEC)+_T(":"));
	Out.Replace(_T("[LimitForm]"), _GetPlainResString(IDS_WEB_CONLIMITS)+_T(":"));
	Out.Replace(_T("[MaxSources]"), _GetPlainResString(IDS_PW_MAXSOURCES)+_T(":"));
	Out.Replace(_T("[MaxConnections]"), _GetPlainResString(IDS_PW_MAXC)+_T(":"));
	Out.Replace(_T("[MaxConnectionsPer5]"), _GetPlainResString(IDS_MAXCON5SECLABEL)+_T(":"));
	Out.Replace(_T("[RefreshTimeForm]"), _GetPlainResString(IDS_WEB_REFRESH_TIME));
	Out.Replace(_T("[RefreshTimeComment]"), _GetPlainResString(IDS_WEB_REFRESH_COMMENT));
	Out.Replace(_T("[SpeedForm]"), _GetPlainResString(IDS_SPEED_LIMITS));
	Out.Replace(_T("[MaxDown]"), _GetPlainResString(IDS_DOWNLOAD));
	Out.Replace(_T("[MaxUp]"), _GetPlainResString(IDS_PW_CON_UPLBL));
	Out.Replace(_T("[SpeedCapForm]"), _GetPlainResString(IDS_CAPACITY_LIMITS));
	Out.Replace(_T("[MaxCapDown]"), _GetPlainResString(IDS_DOWNLOAD));
	Out.Replace(_T("[MaxCapUp]"), _GetPlainResString(IDS_PW_CON_UPLBL));
	Out.Replace(_T("[FirstAndLast]"), _GetPlainResString(IDS_DOWNLOADMOVIECHUNKS));
	Out.Replace(_T("[WapControl]"), _GetPlainResString(IDS_WAP_CONTROL));
	Out.Replace(_T("[eMuleAppName]"), _T("eMule"));
	Out.Replace(_T("[Apply]"), _GetPlainResString(IDS_PW_APPLY));

	Out.Replace(_T("[Yes]"), _GetPlainResString(IDS_YES));
	Out.Replace(_T("[No]"), _GetPlainResString(IDS_NO));	
	Out.Replace(_T("[On]"), _GetPlainResString(IDS_WAP_ON));
	Out.Replace(_T("[Off]"), _GetPlainResString(IDS_WAP_OFF));
	Out.Replace(_T("[Section]"), _GetPlainResString(IDS_EM_PREFS));
	Out.Replace(_T("[SendImages]"), _GetPlainResString(IDS_WAP_SENDIMAGES));
	Out.Replace(_T("[SendGraphs]"), _GetPlainResString(IDS_WAP_SENDGRAPHS));
	Out.Replace(_T("[SendProgressBars]"), _GetPlainResString(IDS_WAP_SENDPROGRESSBARS));
	Out.Replace(_T("[SendBWImages]"), _GetPlainResString(IDS_WAP_SENDBWIMAGES));
	Out.Replace(_T("[GraphsWidth]"), _GetPlainResString(IDS_WAP_GRAPHSWIDTH));
	Out.Replace(_T("[GraphsHeight]"), _GetPlainResString(IDS_WAP_GRAPHSHEIGHT));
	Out.Replace(_T("[FilledGraphs]"), _GetPlainResString(IDS_WAP_FILLEDGRAPHS));
	Out.Replace(_T("[LogsSize]"), _GetPlainResString(IDS_WAP_LOGSSIZE));
	Out.Replace(_T("[ItemsPerPage]"), _GetPlainResString(IDS_WAP_ITEMSPERPAGE));
	Out.Replace(_T("[Bytes]"), _GetPlainResString(IDS_BYTES));
	Out.Replace(_T("[Home]"), _GetPlainResString(IDS_WAP_HOME));
	Out.Replace(_T("[Back]"), _GetPlainResString(IDS_WAP_BACK));
	Out.Replace(_T("[NETWORKS]"), _GetPlainResString(IDS_NETWORK));
	Out.Replace(_T("[BOOTSTRAP]"), _GetPlainResString(IDS_BOOTSTRAP));
	Out.Replace(_T("[BS_IP]"), _GetPlainResString(IDS_IP));
	Out.Replace(_T("[BS_PORT]"), _GetPlainResString(IDS_PORT));
	Out.Replace(_T("[KADEMLIA]"), GetResString(IDS_KADEMLIA) );

	CString sT;
	int n = (int)thePrefs.GetMaxDownload();
	if(n < 0 || n == 65535) n = 0;
	sT.Format(_T("%d"), n);
	Out.Replace(_T("[MaxDownVal]"), sT);
	n = (int)thePrefs.GetMaxUpload();
	if(n < 0 || n == 65535) n = 0;
	sT.Format(_T("%d"), n);
	Out.Replace(_T("[MaxUpVal]"), sT);
	sT.Format(_T("%d"), thePrefs.GetMaxGraphDownloadRate());
	Out.Replace(_T("[MaxCapDownVal]"), sT);
	sT.Format(_T("%d"), thePrefs.GetMaxGraphUploadRate(false));
	Out.Replace(_T("[MaxCapUpVal]"), sT);
	return Out;
}

CString CWapServer::_GetLoginScreen(WapThreadData Data)
{

	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString sSession = _ParseURL(Data.sURL, _T("ses"));

	CString Out = _T("");

	Out += pThis->m_Templates.sLogin;

	Out.Replace(_T("[eMulePlus]"), _T("eMule"));
	Out.Replace(_T("[eMuleAppName]"), _T("eMule"));
	//MORPH START - Changed by SiRoB, [itsonlyme: -modname-]
	/*
	Out.Replace(_T("[version]"), _SpecialChars(theApp.m_strCurVersionLong));
	*/
	Out.Replace(_T("[version]"), _SpecialChars(theApp.m_strCurVersionLong + _T(" [") + theApp.m_strModLongVersion + _T("]")));
	//MORPH END   - Changed by SiRoB, [itsonlyme: -modname-]
	Out.Replace(_T("[Login]"), _GetPlainResString(IDS_WEB_LOGIN));
	Out.Replace(_T("[EnterPassword]"), _GetPlainResString(IDS_WEB_ENTER_PASSWORD));
	Out.Replace(_T("[LoginNow]"), _GetPlainResString(IDS_WEB_LOGIN_NOW));
	Out.Replace(_T("[WapControl]"), _GetPlainResString(IDS_WAP_CONTROL));
	Out.Replace(_T("[Home]"), _GetPlainResString(IDS_WAP_HOME));
	Out.Replace(_T("[Back]"), _GetPlainResString(IDS_WAP_BACK));

	return Out;
}

CString CWapServer::_GetConnectedServer(WapThreadData Data)
{
	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString sSession = _ParseURL(Data.sURL, _T("ses"));

	CString HTTPTemp = _T("");
	TCHAR	HTTPTempC[100] = _T("");

	CString OutS = pThis->m_Templates.sConnectedServer;

	OutS.Replace(_T("[ConnectedServer]"), _GetPlainResString(IDS_PW_SERVER));
	OutS.Replace(_T("[Servername]"), _GetPlainResString(IDS_SL_SERVERNAME));
	OutS.Replace(_T("[Status]"), _GetPlainResString(IDS_STATUS));
	OutS.Replace(_T("[Usercount]"), _GetPlainResString(IDS_UUSERS));
	OutS.Replace(_T("[Action]"), _GetPlainResString(IDS_CONNECTING));
	OutS.Replace(_T("[URL_Disconnect]"), IsSessionAdmin(Data,sSession)?_T("./?ses=") + sSession + _T("&amp;w=server&amp;c=disconnect"):GetPermissionDenied());
	OutS.Replace(_T("[URL_Connect]"), IsSessionAdmin(Data,sSession)?_T("./?ses=") + sSession + _T("&amp;w=server&amp;c=connect"):GetPermissionDenied());
	OutS.Replace(_T("[Disconnect]"), _GetPlainResString(IDS_IRC_DISCONNECT));
	OutS.Replace(_T("[Connect]"), _GetPlainResString(IDS_CONNECTTOANYSERVER));
	OutS.Replace(_T("[URL_ServerOptions]"), IsSessionAdmin(Data,sSession)?_T("./?ses=") + sSession + _T("&amp;w=server&amp;c=options"):GetPermissionDenied());
	OutS.Replace(_T("[ServerOptions]"), _GetPlainResString(IDS_SERVER)+ _T(" ") + _GetPlainResString(IDS_EM_PREFS));
	OutS.Replace(_T("[Home]"), _GetPlainResString(IDS_WAP_HOME));
	OutS.Replace(_T("[Back]"), _GetPlainResString(IDS_WAP_BACK));

	if (theApp.serverconnect->IsConnected()) {
		if(!theApp.serverconnect->IsLowID())
			OutS.Replace(_T("[1]"), _GetPlainResString(IDS_CONNECTED));
		else
			OutS.Replace(_T("[1]"), _GetPlainResString(IDS_CONNECTED) + _T(" (") + _GetPlainResString(IDS_IDLOW) + _T(")"));

		CServer* cur_server = theApp.serverconnect->GetCurrentServer();
		OutS.Replace(_T("[2]"), _SpecialChars(cur_server->GetListName()));

		_stprintf(HTTPTempC, _T("%10i"), cur_server->GetUsers());
		HTTPTemp = HTTPTempC;												
		OutS.Replace(_T("[3]"), HTTPTemp);

	} else if (theApp.serverconnect->IsConnecting()) {
		OutS.Replace(_T("[1]"), _GetPlainResString(IDS_CONNECTING));
		OutS.Replace(_T("[2]"), _T(""));
		OutS.Replace(_T("[3]"), _T(""));
	} else {
		OutS.Replace(_T("[1]"), _GetPlainResString(IDS_DISCONNECTED));
		OutS.Replace(_T("[2]"), _T(""));
		OutS.Replace(_T("[3]"), _T(""));
	}
	return OutS;
}

bool CWapServer::_IsLoggedIn(WapThreadData Data, long lSession)
{
	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis == NULL)
		return false;

	_RemoveTimeOuts(Data);

	// find our session
	// i should have used CMap there, but i like CArray more ;-)
	for(int i = 0; i < pThis->m_Params.Sessions.GetSize(); i++)
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

void CWapServer::_RemoveTimeOuts(WapThreadData Data) {
	// remove expired sessions
	CWapServer *pThis = (CWapServer *)Data.pThis;
	pThis->UpdateSessionCount();
}

bool CWapServer::_RemoveSession(WapThreadData Data, long lSession)
{
	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis == NULL)
		return false;

	// find our session
	for(int i = 0; i < pThis->m_Params.Sessions.GetSize(); i++)
	{
		if(pThis->m_Params.Sessions[i].lSession == lSession && lSession != 0)
		{
			pThis->m_Params.Sessions.RemoveAt(i);
			theApp.emuledlg->serverwnd->UpdateMyInfo();
			AddLogLine(true,GetResString(IDS_WAP_SESSIONEND));
			return true;
		}
	}
	return false;
}

Session CWapServer::GetSessionByID(WapThreadData Data,long sessionID) {
	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis != NULL) {
		for(int i = 0; i < pThis->m_Params.Sessions.GetSize(); i++)
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

bool CWapServer::IsSessionAdmin(WapThreadData Data,CString SsessionID){

	uint64 sessionID=_ttoi64(SsessionID);
	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis != NULL) {
		for(int i = 0; i < pThis->m_Params.Sessions.GetSize(); i++)
		{
			if(pThis->m_Params.Sessions[i].lSession == sessionID && sessionID != 0)
				return pThis->m_Params.Sessions[i].admin>1;	// MORPH: ionix multiuser 
		}
	}
	return false;
}

CString CWapServer::GetPermissionDenied() {
	return _T("./script.wmls#accessdenied()");
}

bool CWapServer::_GetFileHash(CString sHash, uchar *FileHash)
{
	TCHAR hex_byte[3];
	int byte;
	hex_byte[2] = '\0';
	for (int i = 0; i < 16; i++) 
	{
		hex_byte[0] = sHash.GetAt(i*2);
		hex_byte[1] = sHash.GetAt((i*2 + 1));
		_stscanf(hex_byte, _T("%02x"), &byte);
		FileHash[i] = (uchar)byte;
	}
	return true;
}

void CWapServer::PlainString(CString& rstr, bool noquot)
{
	rstr.Replace(_T("&"), _T(""));
	//rstr.Replace(_T("&"), "&amp;");
	rstr.Replace(_T("<"), _T("&lt;"));
	rstr.Replace(_T(">"), _T("&gt;"));
	rstr.Replace(_T("\""), _T("&quot;"));

	if(noquot)
	{
        rstr.Replace(_T("'"), _T("\\'"));
		rstr.Replace(_T("\n"), _T("\\n"));
	}
	else
	{
        rstr.Replace(_T("'"), _T("&apos;"));
	}
}

CString	CWapServer::_GetPlainResString(RESSTRIDTYPE nID, bool noquot)
{
	CString sRet = _GetResString(nID);
	PlainString(sRet, noquot);
	return sRet;
}

// EC + kuchin
CString	CWapServer::_GetWebCharSet()
{
	/*
	switch (thePrefs.GetLanguageID())
	{
		case MAKELANGID(LANG_POLISH,SUBLANG_DEFAULT):				return _T("windows-1250");
		case MAKELANGID(LANG_RUSSIAN,SUBLANG_DEFAULT):				return _T("windows-1251");
		case MAKELANGID(LANG_GREEK,SUBLANG_DEFAULT):				return _T("ISO-8859-7");
		case MAKELANGID(LANG_HEBREW,SUBLANG_DEFAULT):				return _T("windows-1255");
		case MAKELANGID(LANG_KOREAN,SUBLANG_DEFAULT):				return _T("EUC-KR");
		case MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_SIMPLIFIED):	return _T("GB2312");
		case MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_TRADITIONAL):	return _T("Big5");
		case MAKELANGID(LANG_LITHUANIAN,SUBLANG_DEFAULT):			return _T("windows-1257");
		case MAKELANGID(LANG_TURKISH,SUBLANG_DEFAULT):				return _T("windows-1254");
	}

	// Western (Latin) includes Catalan, Danish, Dutch, English, Faeroese, Finnish, French,
	// German, Galician, Irish, Icelandic, Italian, Norwegian, Portuguese, Spanish and Swedish
	return _T("ISO-8859-1");
	*/

	return _T("utf-8");
}

// Ornis: creating the progressbar. colored if ressources are given/available

CString CWapServer::_GetDownloadGraph(WapThreadData Data,CString filehash)
{
	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	if(thePrefs.GetWapSendProgressBars()){
		CString sSession = _ParseURL(Data.sURL, _T("ses"));
		CString Out;
		Out=_T("<img src=\"./?ses=") + sSession + _T("&amp;progressbar=") + filehash + _T("\" alt=\"ProgressBar\"/>");
		return Out;
	}
	else return _T("");
}


CString	CWapServer::_GetSearch(WapThreadData Data)
{
	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	CString sSession = _ParseURL(Data.sURL, _T("ses"));
	CString Out = pThis->m_Templates.sSearchMain;
	CString OutList = pThis->m_Templates.sSearchList;

	if (_ParseURL(Data.sURL, _T("downloads")) != "" && IsSessionAdmin(Data,sSession) ) {
		CString downloads=_ParseURLArray(Data.sURL,_T("downloads"));
		int cat=_ttoi(_ParseURL(Data.sURL, _T("cat")));

		CString resToken;
		int curPos=0;
		resToken= downloads.Tokenize(_T("|"),curPos);
		while (resToken != "")
		{
			uchar fileid[16];
			if (resToken.GetLength()==32 && DecodeBase16(resToken.GetBuffer(),resToken.GetLength(),fileid,ARRSIZE(fileid)))
			theApp.searchlist->AddFileToDownloadByHash(fileid,cat);
			resToken= downloads.Tokenize(_T("|"),curPos);
		}
	}

	if(_ParseURL(Data.sURL, _T("tosearch")) != _T("") && IsSessionAdmin(Data,sSession) )
		{
			// perform search
			theApp.emuledlg->searchwnd->DeleteAllSearches();

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
			&& pParams->strFileType != ED2KFTSTR_VIDEO){
			ASSERT(0);
			pParams->strFileType.Empty();
		}
		pParams->ullMinSize = _tstoi64(_ParseURL(Data.sURL, _T("min")))*1048576ui64;
		pParams->ullMaxSize = _tstoi64(_ParseURL(Data.sURL, _T("max")))*1048576ui64;
		if (pParams->ullMaxSize < pParams->ullMinSize)
			pParams->ullMaxSize = 0;
 
		pParams->uAvailability = (_ParseURL(Data.sURL, _T("avail"))==_T(""))?0:_tstoi(_ParseURL(Data.sURL, _T("avail")));
		if (pParams->uAvailability > 1000000)
			pParams->uAvailability = 1000000;

		pParams->strExtension = _ParseURL(Data.sURL, _T("ext"));
		if (method == _T("kademlia"))
			 pParams->eType = SearchTypeKademlia;
		else if (method == _T("global"))
			 pParams->eType = SearchTypeEd2kGlobal;
		else
			 pParams->eType = SearchTypeEd2kServer;

	 CString strResponse = _GetPlainResString(IDS_SW_SEARCHINGINFO);
	 try
		{
			if (pParams->eType != SearchTypeKademlia){
				if (!theApp.emuledlg->searchwnd->DoNewEd2kSearch(pParams)){
					delete pParams;
					 strResponse = _GetPlainResString(IDS_ERR_NOTCONNECTED);
				 }
			}
			else{
				 if (!theApp.emuledlg->searchwnd->DoNewKadSearch(pParams)){
					delete pParams;
					strResponse = _GetPlainResString(IDS_ERR_NOTCONNECTEDKAD);
				}
			}
		}
	 catch (CMsgBoxException* ex)
	 {
		strResponse = ex->m_strMsg;
		PlainString(strResponse, false);
		ex->Delete();
	    delete pParams;
	 }
	 Out.Replace(_T("[Message]"),strResponse);
	}

	else if(_ParseURL(Data.sURL, _T("tosearch")) != _T("") && !IsSessionAdmin(Data,sSession) ) {
		 OutList.Replace(_T("[Message]"),_GetPlainResString(IDS_ACCESSDENIED));
	}	
	else OutList.Replace(_T("[Message]"),_T(""));
	
	if(_ParseURL(Data.sURL, _T("showlist")) == "true"){
		CString sSort = _ParseURL(Data.sURL, _T("sort"));	if (sSort.GetLength()>0) pThis->m_iSearchSortby=_ttoi(sSort);
		sSort = _ParseURL(Data.sURL, _T("sortAsc"));		if (sSort.GetLength()>0) pThis->m_bSearchAsc=_ttoi(sSort)!=0;

		int startpos = 0;
		bool more;

		if(_ParseURL(Data.sURL, _T("startpos"))!="")
			startpos = _ttoi(_ParseURL(Data.sURL, _T("startpos")));

		int sortpos=0;
		if(pThis->m_iSearchSortby==0){
			sortpos=1;
			CString strSortBy;
			if(pThis->m_bSearchAsc==0)
				strSortBy=_T(" (+)");
			else
				strSortBy=_T(" (-)");
			OutList.Replace(_T("[Filename]"), _GetPlainResString(IDS_DL_FILENAME) + strSortBy);
		}
		else if(pThis->m_iSearchSortby==1){
			sortpos=2;
			CString strSortBy;
			if(pThis->m_bSearchAsc==0)
				strSortBy=_T(" (+)");
			else
				strSortBy=_T(" (-)");
			OutList.Replace(_T("[Filesize]"), _GetPlainResString(IDS_DL_SIZE) + strSortBy);
		}
		else if(pThis->m_iSearchSortby==2){
			sortpos=3;
			CString strSortBy;
			if(pThis->m_bSearchAsc==0)
				strSortBy=_T(" (+)");
			else
				strSortBy=_T(" (-)");
			OutList.Replace(_T("[Filehash]"), _GetPlainResString(IDS_FILEHASH) + strSortBy);
		}
		else if(pThis->m_iSearchSortby==3){
			sortpos=4;
			CString strSortBy;
			if(pThis->m_bSearchAsc==0)
				strSortBy=_T(" (+)");
			else
				strSortBy=_T(" (-)");
			OutList.Replace(_T("[Sources]"), _GetPlainResString(IDS_DL_SOURCES) + strSortBy);
		}
		//[SortByVal:x1,x2,x3,...]
		int sortbypos=OutList.Find(_T("[SortByVal:"),0);
		if(sortbypos!=-1){
			int sortbypos2=OutList.Find(_T("]"),sortbypos+1);
			if(sortbypos2!=-1){
				CString strSortByArray=OutList.Mid(sortbypos+11,sortbypos2-sortbypos-11);
				CString resToken;
				int curPos = 0,curPos2=0;
				bool posfound=false;
				resToken= strSortByArray.Tokenize(_T(","),curPos);
				while (resToken != "" && !posfound)
				{
					curPos2++;
					if(sortpos==_ttoi(resToken)){
						CString strTemp;
						strTemp.Format(_T("%i"),curPos2);
						OutList.Replace(OutList.Mid(sortbypos,sortbypos2-sortbypos+1),strTemp);
						posfound=true;
					}
					resToken= strSortByArray.Tokenize(_T(","),curPos);
				};
				if(!posfound)
					OutList.Replace(Out.Mid(sortbypos,sortbypos2-sortbypos+1),_T("1"));
			}
		}

		CString val;
		val.Format(_T("%i"),(pThis->m_iSearchSortby!=0 || (pThis->m_iSearchSortby==0 && pThis->m_bSearchAsc==0 ))?1:0 );
		OutList.Replace(_T("[SORTASCVALUE0]"), val);
		val.Format(_T("%i"),(pThis->m_iSearchSortby!=1 || (pThis->m_iSearchSortby==1 && pThis->m_bSearchAsc==0 ))?1:0 );
		OutList.Replace(_T("[SORTASCVALUE1]"), val);
		val.Format(_T("%i"),(pThis->m_iSearchSortby!=2 || (pThis->m_iSearchSortby==2 && pThis->m_bSearchAsc==0 ))?1:0 );
		OutList.Replace(_T("[SORTASCVALUE2]"), val);
		val.Format(_T("%i"),(pThis->m_iSearchSortby!=3 || (pThis->m_iSearchSortby==3 && pThis->m_bSearchAsc==0 ))?1:0 );
		OutList.Replace(_T("[SORTASCVALUE3]"), val);

		CString result = theApp.searchlist->GetWapList(pThis->m_Templates.sSearchResultLine,pThis->m_iSearchSortby,pThis->m_bSearchAsc,startpos,thePrefs.GetWapMaxItemsInPages(),more);

		//Navigation Line
		CString strNavLine,strNavUrl;
		int pos,pos2;
		strNavUrl = Data.sURL;
		if((pos=strNavUrl.Find(_T("startpos="),0))>1){
			if (strNavUrl.Mid(pos-1,1)=="&") pos--;
			pos2=strNavUrl.Find(_T("&"),pos+1);
			if (pos2==-1) pos2 = strNavUrl.GetLength();
			strNavUrl = strNavUrl.Left(pos) + strNavUrl.Right(strNavUrl.GetLength()-pos2);
		}	
		strNavUrl=_SpecialChars(strNavUrl);

		if(more){
			strNavLine.Format(_T("<a href=\".") + strNavUrl + _T("&amp;startpos=%d\">") + _GetPlainResString(IDS_WAP_NEXT) + _T("</a> "), startpos+thePrefs.GetWapMaxItemsInPages());
		}
		if(startpos>0){
			CString strTemp;
			startpos-=thePrefs.GetWapMaxItemsInPages();
			if (startpos<0) startpos = 0;
			strTemp.Format(_T("<a href=\".") + strNavUrl + _T("&amp;startpos=%d\">") + _GetPlainResString(IDS_WAP_PREV) + _T("</a>&nbsp;"), startpos);
			strNavLine.Insert(0,strTemp);
		}
		OutList.Replace(_T("[NavigationLine]"), strNavLine);

		if (thePrefs.GetCatCount()>1) InsertCatBox(OutList,0,_GetPlainResString(IDS_TOCAT)+_T(":&nbsp;<br/>")); else OutList.Replace(_T("[CATBOX]"),_T(""));
		OutList.Replace(_T("[URL]"), _SpecialChars(_T(".") + Data.sURL));
		
		OutList.Replace(_T("[RESULTLIST]"), result);
		OutList.Replace(_T("[Result]"), _GetPlainResString(IDS_SW_RESULT) );
		OutList.Replace(_T("[Session]"), sSession);
		OutList.Replace(_T("[RefetchResults]"), _GetPlainResString(IDS_SW_REFETCHRES));
		OutList.Replace(_T("[Download]"), _GetPlainResString(IDS_DOWNLOAD));
		OutList.Replace(_T("[Filename]"), _GetPlainResString(IDS_DL_FILENAME));
		OutList.Replace(_T("[Filesize]"), _GetPlainResString(IDS_DL_SIZE));
		OutList.Replace(_T("[Filehash]"), _GetPlainResString(IDS_FILEHASH));
		OutList.Replace(_T("[Sources]"), _GetPlainResString(IDS_DL_SOURCES));
		OutList.Replace(_T("[Section]"), _GetPlainResString(IDS_SW_SEARCHBOX));
		OutList.Replace(_T("[Home]"), _GetPlainResString(IDS_WAP_HOME));
		OutList.Replace(_T("[Back]"), _GetPlainResString(IDS_WAP_BACK));
		OutList.Replace(_T("[SortBy]"), _GetPlainResString(IDS_WAP_SORTBY));
		return OutList;

	}
	else
		{ // No showlist
		Out.Replace(_T("[Name]"), _GetPlainResString(IDS_SW_NAME));
		Out.Replace(_T("[Type]"), _GetPlainResString(IDS_TYPE));

	 	// 'file type' display strings
	 	Out.Replace(_T("[FileTypeAny]"), _GetPlainResString(IDS_SEARCH_ANY));
	 	Out.Replace(_T("[FileTypeArc]"), _GetPlainResString(IDS_SEARCH_ARC));
	 	Out.Replace(_T("[FileTypeAud]"), _GetPlainResString(IDS_SEARCH_AUDIO));
	 	Out.Replace(_T("[FileTypeIso]"), _GetPlainResString(IDS_SEARCH_CDIMG));
	 	Out.Replace(_T("[FileTypeDoc]"), _GetPlainResString(IDS_SEARCH_DOC));
	 	Out.Replace(_T("[FileTypeImg]"), _GetPlainResString(IDS_SEARCH_PICS));
	 	Out.Replace(_T("[FileTypePro]"), _GetPlainResString(IDS_SEARCH_PRG));
	 	Out.Replace(_T("[FileTypeVid]"), _GetPlainResString(IDS_SEARCH_VIDEO));

	 	// 'file type' ID strings
	 	Out.Replace(_T("[FileTypeAnyId]"), _T(""));
	 	Out.Replace(_T("[FileTypeArcId]"), _T(ED2KFTSTR_ARCHIVE));
	 	Out.Replace(_T("[FileTypeAudId]"), _T(ED2KFTSTR_AUDIO));
	 	Out.Replace(_T("[FileTypeIsoId]"), _T(ED2KFTSTR_CDIMAGE));
	 	Out.Replace(_T("[FileTypeDocId]"), _T(ED2KFTSTR_DOCUMENT));
	 	Out.Replace(_T("[FileTypeImgId]"), _T(ED2KFTSTR_IMAGE));
	 	Out.Replace(_T("[FileTypeProId]"), _T(ED2KFTSTR_PROGRAM));
	 	Out.Replace(_T("[FileTypeVidId]"), _T(ED2KFTSTR_VIDEO));

		Out.Replace(_T("[Search]"), _GetPlainResString(IDS_SW_SEARCHBOX));
 		Out.Replace(_T("[Session]"), sSession);

		Out.Replace(_T("[SizeMin]"), _GetPlainResString(IDS_SEARCHMINSIZE));
		Out.Replace(_T("[SizeMax]"), _GetPlainResString(IDS_SEARCHMAXSIZE));
		Out.Replace(_T("[Availabl]"), _GetPlainResString(IDS_SEARCHAVAIL));
		Out.Replace(_T("[Extention]"), _GetPlainResString(IDS_SEARCHEXTENTION));
		//Out.Replace("[Global]", _GetPlainResString(IDS_GLOBALSEARCH2));
		Out.Replace(_T("[MB]"), _GetPlainResString(IDS_MBYTES));
		Out.Replace(_T("[Section]"), _GetPlainResString(IDS_SW_SEARCHBOX));
		Out.Replace(_T("[Yes]"), _GetPlainResString(IDS_YES));
		Out.Replace(_T("[No]"), _GetPlainResString(IDS_NO));
		Out.Replace(_T("[Home]"), _GetPlainResString(IDS_WAP_HOME));
		Out.Replace(_T("[Back]"), _GetPlainResString(IDS_WAP_BACK));

		Out.Replace(_T("[METHOD]"), _GetPlainResString(IDS_METHOD));
		Out.Replace(_T("[USESSERVER]"), _GetPlainResString(IDS_SERVER));
		Out.Replace(_T("[Global]"), _GetPlainResString(IDS_GLOBALSEARCH));
		Out.Replace(_T("[KADEMLIA]"), _GetPlainResString(IDS_KADEMLIA) );
		return Out;
	}
}

int CWapServer::UpdateSessionCount() {

	// remove old bans
	for(int i = 0; i < m_Params.badlogins.GetSize();)
	{
		uint32 diff= ::GetTickCount() - m_Params.badlogins[i].timestamp ;
		if ( diff >1000*60*15 && (::GetTickCount() > m_Params.badlogins[i].timestamp ) ) {
			m_Params.badlogins.RemoveAt(i);
		}
			else i++;
	}

	// count & remove old session
	int oldvalue=m_Params.Sessions.GetSize();
	for(int i = 0; i < m_Params.Sessions.GetSize();)
	{
		CTimeSpan ts = CTime::GetCurrentTime() - m_Params.Sessions[i].startTime;
		if(ts.GetTotalSeconds() > SESSION_TIMEOUT_SECS) {
			m_Params.Sessions.RemoveAt(i);
		}
		else
			i++;
	}

	if (oldvalue!=m_Params.Sessions.GetSize()) theApp.emuledlg->serverwnd->UpdateMyInfo();
		
	return m_Params.Sessions.GetCount();
}

void CWapServer::InsertCatBox(CString &Out,int preselect,CString boxlabel,bool jump,bool extraCats) {
	CString tempBuf2,tempBuf3;
	CString tempBuf;
	int pos=0;
	tempBuf3 = _T("<select name=\"cat\">");

	for (int i=0;i< thePrefs.GetCatCount();i++) {
		pos++;
		if(preselect==i)
			tempBuf3.Format(_T("<select name=\"cat\" ivalue=\"%i\">"),pos);
		if(jump)
			tempBuf2.Format(_T("<option value=\"%i\" onpick=\"./script.wmls#GotoCat('%i','[URL]')\">%s</option>"),i,i, (i==0)?_GetPlainResString(IDS_ALL):thePrefs.GetCategory(i)->strTitle );
		else
			tempBuf2.Format(_T("<option value=\"%i\">%s</option>"),i, (i==0)?_GetPlainResString(IDS_ALL):thePrefs.GetCategory(i)->strTitle );
		tempBuf.Append(tempBuf2);
	}
	if (extraCats) {
		if (thePrefs.GetCatCount()>1){
			pos++;
			tempBuf2.Format(_T("<option>------------</option>"));
			tempBuf.Append(tempBuf2);
		}
	
		for (int i=(thePrefs.GetCatCount()>1)?1:2;i<=12;i++) {
			pos++;
			if(preselect==-i)
				tempBuf3.Format(_T("<select name=\"cat\" ivalue=\"%i\">"),pos);
			if(jump)
				tempBuf2.Format(_T("<option value=\"%i\" onpick=\"./script.wmls#GotoCat('%i','[URL]')\">%s</option>"),-i,-i, GetSubCatLabel(-i) );
			else
				tempBuf2.Format(_T("<option value=\"%i\">%s</option>"),-i, GetSubCatLabel(-i) );
			tempBuf.Append(tempBuf2);
		}
	}
	tempBuf.Insert(0,tempBuf3);
	tempBuf.Append(_T("</select>"));
	//if(jump) tempBuf.Append("<a href=\"./script.wmls#GotoCat($(cat),'[URL]')\">Go</a>");
	Out.Replace(_T("[CATBOX]"),boxlabel+tempBuf);
}

CString CWapServer::GetSubCatLabel(int cat) {
	switch (cat) {
		case -1: return _GetPlainResString(IDS_ALLOTHERS);
		case -2: return _GetPlainResString(IDS_STATUS_NOTCOMPLETED);
		case -3: return _GetPlainResString(IDS_DL_TRANSFCOMPL);
		case -4: return _GetPlainResString(IDS_WAITING);
		case -5: return _GetPlainResString(IDS_DOWNLOADING);
		case -6: return _GetPlainResString(IDS_ERRORLIKE);
		case -7: return _GetPlainResString(IDS_PAUSED);
		case -8: return _GetPlainResString(IDS_STOPPED);
		case -9: return _GetPlainResString(IDS_VIDEO);
		case -10: return _GetPlainResString(IDS_AUDIO);
		case -11: return _GetPlainResString(IDS_SEARCH_ARC);
		case -12: return _GetPlainResString(IDS_SEARCH_CDIMG);
	}
	return _T("?");
}

// Elandal: moved from CUpDownClient
// Webserber [kuchin]
CString CWapServer::GetUploadFileInfo(CUpDownClient* client)
{
	if(client == NULL) return _T("");
	CString sRet;

	// build info text and display it
	sRet.Format(GetResString(IDS_USERINFO), client->GetUserName());
	if (client->GetRequestFile())
	{
		sRet += GetResString(IDS_SF_REQUESTED) + client->GetRequestFile()->GetFileName() + _T("\n");
		CString stat;
		stat.Format(GetResString(IDS_FILESTATS_SESSION)+GetResString(IDS_FILESTATS_TOTAL),
			client->GetRequestFile()->statistic.GetAccepts(),
			client->GetRequestFile()->statistic.GetRequests(),
			CastItoXBytes(client->GetRequestFile()->statistic.GetTransferred()),
			client->GetRequestFile()->statistic.GetAllTimeAccepts(),
			client->GetRequestFile()->statistic.GetAllTimeRequests(),
			CastItoXBytes(client->GetRequestFile()->statistic.GetAllTimeTransferred()) );
		sRet += stat;
	}
	else
	{
		sRet += GetResString(IDS_REQ_UNKNOWNFILE);
	}
	return sRet;
}

void CWapServer::DrawLineInCxImage(CxImage *image,int x1, int y1, int x2, int y2, COLORREF color){
	int dx, dy, incr_x, incr_y, const1, const2, x, y, p, k;

	if (x1==x2 && y1==y2)
		image->SetPixelColor(x1,y1,color);
	else {
		// Bresenham Algorithm
		dx = x2 - x1;
		if (dx < 0){
			dx = -dx;
			incr_x = -1;
		}
		else incr_x = 1;

		dy = y2 - y1;
		if (dy < 0){
			dy = -dy;
			incr_y = -1;
		}
		else incr_y = 1;

		x = x1;
		y = y1;
		image->SetPixelColor(x,y,color);

		if (dx > dy) {
			p = 2*dy - dx;
			const1 = 2*dy;
			const2 = 2*(dy - dx);
			for (k=1; k <= dx; k++){
				x = x + incr_x;
				if (p < 0)
					p = p + const1;
				else {
					y = y + incr_y;
					p = p + const2;
				}
					image->SetPixelColor(x,y,color);
			}
		} else {
			p = 2*dx - dy;
			const1 = 2*dx;
			const2 = 2*(dx - dy);
			for (k=1; k <= dy; k++){
				y = y+ incr_y;
				if (p < 0)
					p = p + const1;
				else {
					x = x + incr_x;
					p = p + const2;
				}
					image->SetPixelColor(x,y,color);
			}
		}
	}
}

void CWapServer::SendGraphFile(WapThreadData Data, int file_val){
	CxImage* cImage;
	int pos, width, height;
	long curval = NULL;
	float scalefactor = NULL;
	COLORREF color1,color2,color3,curcolor = NULL;
	bool png=false, gif=false;

	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis == NULL)
		return;

	if(!thePrefs.GetWapSendGraphs()){
		Data.pSocket->SendError404();
		return;
	}

	png=BrowserAccept(Data,_T("image/png"));
	gif=BrowserAccept(Data,_T("image/gif"));
	if(!(png || gif) && BrowserAccept(Data,_T("image/*"))){
		png=true;
		gif=true;
	}

	width=thePrefs.GetWapGraphWidth();
	height=thePrefs.GetWapGraphHeight();
	switch(file_val){
		case 1:
			//(height-2) because the border are 2 pixels (1 top, 1 bottom)
			scalefactor=(float)(height-2)/(thePrefs.GetMaxGraphDownloadRate() + 4);
			break;
		case 2:
			scalefactor=(float)(height-2)/(thePrefs.GetMaxGraphUploadRate(true) + 4);
			break;
		case 3:
			scalefactor=(float)(height-2)/(thePrefs.GetMaxConnections()+20);
			break;
	}

	cImage = new CxImage(width, height, 24);
	if(!cImage){
		Data.pSocket->SendError404();
		return;
	}
	cImage->Clear(0xff);

	if((png || gif) && !thePrefs.GetWapAllwaysSendBWImages()){
		color1=pThis->m_Templates.cDownColor; 
		color2=pThis->m_Templates.cUpColor; 
		color3=pThis->m_Templates.cConnectionsColor; 

		RGBQUAD colors[16];
		colors[0]=cImage->RGBtoRGBQUAD(0x00000000);
		colors[1]=cImage->RGBtoRGBQUAD(color1);
		colors[2]=cImage->RGBtoRGBQUAD(color2);
		colors[3]=cImage->RGBtoRGBQUAD(color3);
		for(int i=4; i<16; i++)
			colors[i]=cImage->RGBtoRGBQUAD(0x00FFFFFF);

		cImage->DecreaseBpp(4,false,colors);
	} else {
		color1=color2=color3=0x00000000;
		cImage->DecreaseBpp(1,false);
	}

	pos = pThis->m_Params.PointsForWeb.GetCount()-1;

	if(thePrefs.GetWapGraphsFilled()){
		for(int x = width-2; x > 0 && pos >= 0; x--, pos--)
		{
			switch(file_val){
				case 1:
					curval=(long)(pThis->m_Params.PointsForWeb[pos].download * scalefactor);
					curcolor=color1;
					break;
				case 2:
					curval=(long)(pThis->m_Params.PointsForWeb[pos].upload * scalefactor);
					curcolor=color2;
					break;
				case 3:
					curval=(long)(pThis->m_Params.PointsForWeb[pos].connections * scalefactor);
					curcolor=color3;
					break;
			}
			for(int y = 0; y <= curval ;y++)
				cImage->SetPixelColor(x,y,curcolor);
		}
	} else {
		int oldval=0;
		for(int x = width-1; x > 0 && pos >= 0; x--, pos--)
		{
			switch(file_val){
				case 1:
					curval=(long)(pThis->m_Params.PointsForWeb[pos].download * scalefactor);
					curcolor=color1;
					break;
				case 2:
					curval=(long)(pThis->m_Params.PointsForWeb[pos].upload * scalefactor);
					curcolor=color2;
					break;
				case 3:
					curval=(long)(pThis->m_Params.PointsForWeb[pos].connections * scalefactor);
					curcolor=color3;
					break;
			}
			DrawLineInCxImage(cImage,x,curval,x+1,oldval,curcolor);
			oldval=curval;
		}
	}

	//Creates a border
	for(int x = 0; x < width; x++)
	{
		cImage->SetPixelColor(x,0,0);
		cImage->SetPixelColor(x,height-1,0);
	}
	for(int y = 0; y < height; y++)
	{
		cImage->SetPixelColor(0,y,0);
		cImage->SetPixelColor(width-1,y,0);
	}

	CString contenttype;

	cImage->SetTransIndex(-1);
	SendSmallestCxImage(Data, cImage);

	delete cImage;
}

void CWapServer::SendScriptFile(WapThreadData Data){
	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis == NULL)
		return;

	if(!BrowserAccept(Data,_T("text/vnd.wap.wmlscript"))){
		CString Out;
		Out =_T("<?xml version=\"1.0\" encoding=\"")+ _GetWebCharSet() +_T("\"?>");
		Out +=_T("<!DOCTYPE wml PUBLIC \"-//WAPFORUM//DTD WML 1.1//EN\" \"http://www.wapforum.org/DTD/wml_1.1.xml\">");
		Out +=_T("<wml><card><p>");
		Out +=_T("Error: This browser do not support WMLScript");
		Out +=_T("</p></card></wml>");
		Data.pSocket->SendContent(CT2CA(WMLInit), Out);
		return;
	}

	CString wmlsStr, question;
	
	wmlsStr = pThis->m_Templates.sScriptFile;
	wmlsStr.Replace(_T("[Yes]"),RemoveWMLScriptInvalidChars(_GetPlainResString(IDS_YES)));
	wmlsStr.Replace(_T("[No]"),RemoveWMLScriptInvalidChars(_GetPlainResString(IDS_NO)));
	wmlsStr.Replace(_T("[ConfirmRemoveServer]"), RemoveWMLScriptInvalidChars(_GetPlainResString(IDS_WEB_CONFIRM_REMOVE_SERVER)));

	if(!wmlsStr.IsEmpty()) wmlsStr+="\n\n";

	//Confirm cancel
	question=RemoveWMLScriptInvalidChars(_GetPlainResStringNoQuote(IDS_Q_CANCELDL2));
	question.Replace(_T("\\n"),_T(""));
	wmlsStr += _T("extern function confirmcancel(curl) {\n")
			_T("var a = Dialogs.confirm(\"") + 
			question + _T("\", ")
			_T("\"") + RemoveWMLScriptInvalidChars(_GetPlainResString(IDS_YES)) + _T("\", ") +
			_T("\"") + RemoveWMLScriptInvalidChars(_GetPlainResString(IDS_NO)) + _T("\");\n")
			_T("if(a) WMLBrowser.go(curl);\n")
			_T("}\n\n");

	//Access denied
	wmlsStr += _T("extern function accessdenied() {\n")
			_T("var a = Dialogs.alert(\"") +
			RemoveWMLScriptInvalidChars(_GetPlainResString(IDS_ACCESSDENIED)) + _T("\");\n")
			_T("}\n\n");

	//Shows an ed2klink
	wmlsStr += _T("extern function showed2k(fname,hash,size) {\n")
			_T("var ed2k = \"ed2k://|file|\" + fname + \"|\" + size + \"|\" + hash + \"|/\";\n")
			_T("var a = Dialogs.alert(ed2k);\n")
			_T("}");

	Data.pSocket->SendContent("Server: eMule\r\nConnection: close\r\nContent-Type: text/vnd.wap.wmlscript\r\n", wmlsStr, wmlsStr.GetLength());
}

bool CWapServer::BrowserAccept(WapThreadData Data, CString sAccept){
	int curPos= 0;
	
	CString header(Data.pSocket->m_pBuf,Data.pSocket->m_dwHttpHeaderLen);
	header.Replace(_T("\r\n"),_T("\n"));

	CString resToken;
	resToken=header.Tokenize(_T("\n"),curPos);
	while (resToken != "")
	{
		if(resToken.Left(7).MakeLower()=="accept:")
			if(CString(resToken).MakeLower().Find(sAccept.MakeLower())!=-1)
				return true;

		resToken = header.Tokenize(_T("\n"),curPos);
	};
	return false;		
}

void CWapServer::SendImageFile(WapThreadData Data, CString filename){
	CWapServer *pThis = (CWapServer *)Data.pThis;
	if (pThis == NULL) return;

	if(!thePrefs.GetWapSendImages()){
		Data.pSocket->SendError404();
		return;
	}

	bool png=false, gif=false;

	png=BrowserAccept(Data,_T("image/png"));
	gif=BrowserAccept(Data,_T("image/gif"));
	if(!(png || gif) && BrowserAccept(Data,_T("image/*"))){
		png=true;
		gif=true;
	}

	TCHAR pngFile[_MAX_PATH];
	_stprintf(pngFile, filename.Left(filename.ReverseFind(_T('.'))) + _T(".png"));
	TCHAR gifFile[_MAX_PATH];
	_stprintf(gifFile, filename.Left(filename.ReverseFind(_T('.'))) + _T(".png"));
	TCHAR wbmpFile[_MAX_PATH];
	_stprintf(wbmpFile, filename.Left(filename.ReverseFind(_T('.'))) + _T(".png"));

	BOOL pngExist,gifExist,wbmpExist;
	pngExist=PathFileExists(pngFile);
	gifExist=PathFileExists(gifFile);
	wbmpExist=PathFileExists(wbmpFile);

	if(thePrefs.GetWapAllwaysSendBWImages()){
		filename=wbmpFile;
	}

	CxImage *cImage;
	CString moreContentType = _T("Last-Modified: ") + pThis->m_Params.sLastModified + _T("\r\n") + _T("ETag: ") + pThis->m_Params.sETag + _T("\r\n");

	if(!(png || gif) || thePrefs.GetWapAllwaysSendBWImages()){
		if(wbmpExist){
			cImage = new CxImage;
			if(cImage && cImage->Load(wbmpFile,CXIMAGE_FORMAT_WBMP)){
				BYTE * bufferPNG = NULL;
				BYTE * bufferGIF = NULL;
				long sizePNG = 0, sizeGIF = 0, sizeWBMP = 0;

				if(png) cImage->Encode(bufferPNG, sizePNG, CXIMAGE_FORMAT_PNG);
				
				if(gif){
					cImage->SetCodecOption(2); //LZW
					cImage->Encode(bufferGIF, sizeGIF, CXIMAGE_FORMAT_GIF);
				}

				sizeWBMP=PathFileExists(wbmpFile);
				if(bufferPNG && bufferGIF && (sizePNG<=sizeGIF) && (sizePNG<=sizeWBMP)){
					Data.pSocket->SendContent(CT2CA(_T("Server: eMule\r\nCache-Control: public\r\nContent-Type: image/png\r\n") + moreContentType), bufferPNG, sizePNG);
				}
				else if(bufferPNG && bufferGIF && (sizeGIF<=sizePNG) && (sizeGIF<=sizeWBMP)){
					Data.pSocket->SendContent(CT2CA(_T("Server: eMule\r\nCache-Control: public\r\nContent-Type: image/gif\r\n") + moreContentType), bufferGIF, sizeGIF);
				}
				else if(bufferPNG && (sizePNG<=sizeWBMP)){
					Data.pSocket->SendContent(CT2CA(_T("Server: eMule\r\nCache-Control: public\r\nContent-Type: image/png\r\n") + moreContentType), bufferPNG, sizePNG);
				}
				else if(bufferGIF && (sizeGIF<=sizeWBMP)){
					Data.pSocket->SendContent(CT2CA(_T("Server: eMule\r\nCache-Control: public\r\nContent-Type: image/gif\r\n") + moreContentType), bufferGIF, sizeGIF);
				}
				else {
					SendFile(Data,wbmpFile,_T("Server: eMule\r\nCache-Control: public\r\nContent-Type: image/vnd.wap.wbmp\r\n") + moreContentType);
				}

				delete[] bufferPNG;
				delete[] bufferGIF;
				delete cImage;
			}
		}
		else if(pngExist){
			cImage = new CxImage;
			if(cImage && cImage->Load(pngFile,CXIMAGE_FORMAT_PNG)){
				SendSmallestCxImage(Data,cImage);
				delete cImage;
			}
		}
		else if(gifExist){
			cImage = new CxImage;
			if(cImage && cImage->Load(gifFile,CXIMAGE_FORMAT_GIF)){
				SendSmallestCxImage(Data,cImage);
				delete cImage;
			}
		}
	}
	else {
		if(png && gif){
			if(pngExist && gifExist){
				if(FileSize(pngFile)<FileSize(gifFile))
					SendFile(Data,pngFile,_T("Server: eMule\r\nCache-Control: public\r\nContent-Type: image/png\r\n") + moreContentType);
				else
					SendFile(Data,gifFile,_T("Server: eMule\r\nCache-Control: public\r\nContent-Type: image/gif\r\n") + moreContentType);
			}
			else if(pngExist)
				SendFile(Data,pngFile,_T("Server: eMule\r\nCache-Control: public\r\nContent-Type: image/png\r\n") + moreContentType);
			else if(gifExist)
				SendFile(Data,gifFile,_T("Server: eMule\r\nCache-Control: public\r\nContent-Type: image/gif\r\n") + moreContentType);
			else if(wbmpExist)
				SendFile(Data,wbmpFile,_T("Server: eMule\r\nCache-Control: public\r\nContent-Type: image/vnd.wap.wbmp\r\n") + moreContentType);
			else
				Data.pSocket->SendError404();
		}
		else if(png || gif){
			if(png && pngExist)
				SendFile(Data,pngFile,_T("Server: eMule\r\nCache-Control: public\r\nContent-Type: image/png\r\n") + moreContentType);
			else if(gif && gifExist)
				SendFile(Data,gifFile,_T("Server: eMule\r\nCache-Control: public\r\nContent-Type: image/gif\r\n") + moreContentType);
			else if(wbmpExist)
				SendFile(Data,wbmpFile,_T("Server: eMule\r\nCache-Control: public\r\nContent-Type: image/vnd.wap.wbmp\r\n") + moreContentType);
			else{
				if(pngExist){
					cImage = new CxImage;
					if(cImage && cImage->Load(pngFile,CXIMAGE_FORMAT_PNG)){
						cImage->DecreaseBpp(1,true);
						SendSmallestCxImage(Data, cImage);
						delete cImage;
					}
				}
				if(gifExist){
					cImage = new CxImage;
					if(cImage && cImage->Load(gifFile,CXIMAGE_FORMAT_GIF)){
						cImage->DecreaseBpp(1,true);
						SendSmallestCxImage(Data, cImage);
						delete cImage;
					}
				}
				else{
					Data.pSocket->SendError404();
				}
			}
		}
		else{
			if(wbmpExist)
				SendFile(Data,wbmpFile,_T("Server: eMule\r\nCache-Control: public\r\nContent-Type: image/vnd.wap.wbmp\r\n") + moreContentType);
			else {
				if(pngExist){
					cImage = new CxImage;
					if(cImage && cImage->Load(pngFile,CXIMAGE_FORMAT_PNG)){
						cImage->DecreaseBpp(1,true);
						SendSmallestCxImage(Data, cImage);
						delete cImage;
					}
				}
				if(gifExist){
					cImage = new CxImage;
					if(cImage && cImage->Load(gifFile,CXIMAGE_FORMAT_GIF)){
						cImage->DecreaseBpp(1,true);
						SendSmallestCxImage(Data, cImage);
						delete cImage;
					}
				}
				else{
					Data.pSocket->SendError404();
				}
			}
		}
	}
}

void CWapServer::SendProgressBar(WapThreadData Data, CString filehash)
{
	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis == NULL)
		return;

	CPartFile* cur_file;

	uchar fileid[16];
	if(filehash.GetLength()!=32 || !DecodeBase16(filehash, filehash.GetLength(), fileid, ARRSIZE(fileid)))
		return;
	
	CxImage progressImage;
	bool png=false, gif=false;

	png=BrowserAccept(Data, _T("image/png"));
	gif=BrowserAccept(Data, _T("image/gif"));
	if(!(png || gif) && BrowserAccept(Data,_T("image/*"))){
		png=true;
		gif=true;
	}

	// cool style
	RGBQUAD progresscolor[16];
	progresscolor[0]=progressImage.RGBtoRGBQUAD(0x0000c000);		//green
	progresscolor[1]=progressImage.RGBtoRGBQUAD(0x00000000);		//black
	progresscolor[2]=progressImage.RGBtoRGBQUAD(0x0000ffff);		//yellow
	progresscolor[3]=progressImage.RGBtoRGBQUAD(0x000000ff);		//red
	progresscolor[4]=progressImage.RGBtoRGBQUAD(0x00ffd200);		//blue1
	progresscolor[5]=progressImage.RGBtoRGBQUAD(0x00ffb100);		//blue2
	progresscolor[6]=progressImage.RGBtoRGBQUAD(0x00ff8500);		//blue3
	progresscolor[7]=progressImage.RGBtoRGBQUAD(0x00ff5900);		//blue4
	progresscolor[8]=progressImage.RGBtoRGBQUAD(0x00ff2d00);		//blue5
	progresscolor[9]=progressImage.RGBtoRGBQUAD(0x00ff0000);		//blue6
	for(int i=10; i<16; i++)
		progresscolor[i]=progressImage.RGBtoRGBQUAD(0x00FFFFFF);	//white

	CString s_ChunkBar;
	cur_file=theApp.downloadqueue->GetFileByID(fileid);
	if (cur_file==0 || !cur_file->IsPartFile()) {
		for(int i=0;i<thePrefs.GetWapGraphWidth()-2;i++)
			s_ChunkBar+="0";
	}
	else
	{
		s_ChunkBar=cur_file->GetProgressString((uint16)thePrefs.GetWapGraphWidth()-1);
	}

	if((png || gif) && !thePrefs.GetWapAllwaysSendBWImages()){
		//Single Color Bar (13px = 8px (progress) + 2px (percent) + 3px (border))
		progressImage.Create(thePrefs.GetWapGraphWidth(),13,4,0);
		progressImage.SetPalette(progresscolor,16);
	}else{
		//Double B/N Bar (22px = 8px (progress1) + 8px (progress2) + 2px (percent) + 2px (ext_border) + 2px (int_borders))
		progressImage.Create(thePrefs.GetWapGraphWidth(),22,24,0);
	}

	progressImage.Clear(0xFF);

	// and now make a graph out of the array - need to be in a progressive way
	int curcolor;
	for (uint16 i=1;i<progressImage.GetWidth()-1;i++) {
		curcolor = int(s_ChunkBar.GetAt(i-1)) - int('0');
		if(curcolor>9) MessageBox(NULL,_T("Ups"),_T("Ups"),MB_OK);
		if((png || gif) && !thePrefs.GetWapAllwaysSendBWImages())
		{
			for(int y=0;y<10;y++)
		        progressImage.SetPixelIndex(i,y,(BYTE)curcolor);
		}
		else
		{
			if(curcolor==0 || curcolor==1){ //DOWNLOADED
				for(int y=10;y<18;y++)
			        progressImage.SetPixelColor(i,y,0);
			}

			if(curcolor!=3){ //!RED
				for(int y=0;y<9;y++)
			        progressImage.SetPixelColor(i,y,0);
			}
		}
	}

	int compl;
	if(cur_file)
		compl=(int)(((thePrefs.GetWapGraphWidth()-2)/100.0)*cur_file->GetPercentCompleted());
	else
		compl=thePrefs.GetWapGraphWidth()-2;

	if((png || gif) && !thePrefs.GetWapAllwaysSendBWImages())
	{
		for(int x=1; x <= compl; x++){
		    progressImage.SetPixelColor(x,progressImage.GetHeight()-2,progresscolor[0]);
			progressImage.SetPixelColor(x,progressImage.GetHeight()-3,progresscolor[0]);
		}
	}
	else
	{
		for(int x=1; x <= compl; x++){
		    progressImage.SetPixelColor(x,progressImage.GetHeight()-2,0);
			progressImage.SetPixelColor(x,progressImage.GetHeight()-3,0);
		}
	}

	//Creates a border
	for(unsigned int x = 0; x < progressImage.GetWidth(); x++)
	{
		progressImage.SetPixelColor(x,0,0);
		progressImage.SetPixelColor(x,progressImage.GetHeight()-1,0);
	}
	for(unsigned int y = 0; y < progressImage.GetHeight(); y++)
	{
		progressImage.SetPixelColor(0,y,0);
		progressImage.SetPixelColor(progressImage.GetWidth()-1,y,0);
	}
	
	//Draws internals separators.
	if((png || gif) && !thePrefs.GetWapAllwaysSendBWImages()){
		for(unsigned int x = 0; x < progressImage.GetWidth(); x++)
		{
			progressImage.SetPixelColor(x,progressImage.GetHeight()-4,0);
		}
	} else {
		for(unsigned int x = 0; x < progressImage.GetWidth(); x+=2)
		{
			progressImage.SetPixelColor(x,progressImage.GetHeight()-4,0);
			progressImage.SetPixelColor(x,progressImage.GetHeight()-13,0);
		}
	}

	//Select the smallest file
	progressImage.SetTransIndex(-1);
	SendSmallestCxImage(Data, &progressImage);
}

bool CWapServer::SendFile(WapThreadData Data, LPCTSTR szfileName, CString contentType){
	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis == NULL)
		return false;

	CFile file;
	if(file.Open(szfileName, CFile::modeRead|CFile::shareDenyWrite|CFile::typeBinary))
	{
		char* buffer=new char[(UINT)(file.GetLength())];
		int size=file.Read(buffer,(UINT)(file.GetLength()));
		file.Close();
		Data.pSocket->SendContent(CT2CA(contentType), buffer, size);
		delete[] buffer;
		return true;
	}
	return false;
}

void CWapServer::SendSmallestCxImage(WapThreadData Data, CxImage *cImage){
	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis == NULL)
		return;

	bool png=false, gif=false, bwImage;
	ENUM_WAP_IMAGE_FORMAT imageToSend = WITHOUT_IMAGE;

	png=BrowserAccept(Data, _T("image/png"));
	gif=BrowserAccept(Data, _T("image/gif"));
	if(!(png || gif) && BrowserAccept(Data,_T("image/*"))){
		png=true;
		gif=true;
	}
	bwImage=(thePrefs.GetWapAllwaysSendBWImages() || !(png || gif));

	if(bwImage && (cImage->GetBpp()>1)){
		cImage->DecreaseBpp(1,true);
	}

	if(!bwImage && (cImage->GetBpp()>4)){
		cImage->DecreaseBpp(4,true);
	}
	
	long sizePNG = 0, sizeGIF = 0, sizeWBMP = 0;
	BYTE * bufferPNG = NULL;
	BYTE * bufferGIF = NULL;
	BYTE * bufferWBMP = NULL;
	
	if(png) cImage->Encode(bufferPNG, sizePNG, CXIMAGE_FORMAT_PNG);
	
	if(gif){
		cImage->SetCodecOption(2); //LZW
		cImage->Encode(bufferGIF, sizeGIF, CXIMAGE_FORMAT_GIF);
	}

	cImage->DecreaseBpp(1,true);
	cImage->Encode(bufferWBMP, sizeWBMP, CXIMAGE_FORMAT_WBMP);

	if(bwImage){
		if(bufferGIF && bufferPNG && bufferWBMP && (sizeGIF<=sizePNG) && (sizeGIF<=sizeWBMP)){
			imageToSend = GIF_IMAGE;
		}else if(bufferGIF && bufferPNG && bufferWBMP && (sizePNG<=sizeGIF) && (sizePNG<=sizeWBMP)){
			imageToSend = PNG_IMAGE;
		}else if(bufferGIF && bufferWBMP && (sizeGIF<=sizeWBMP)){
			imageToSend = GIF_IMAGE;
		}else if(bufferPNG && bufferWBMP && (sizePNG<=sizeWBMP)){
			imageToSend = PNG_IMAGE;
		}else if(bufferWBMP){
			imageToSend = WBMP_IMAGE;
		}else if(!bufferWBMP){ //???
			if(bufferPNG && bufferGIF){
				if (sizeGIF<=sizePNG){
					imageToSend = GIF_IMAGE;
				}else{
					imageToSend = PNG_IMAGE;
				}
			} else if(bufferPNG){
				imageToSend = PNG_IMAGE;
			} else if(bufferGIF){
				imageToSend = GIF_IMAGE;
			} else {
				imageToSend = WITHOUT_IMAGE;
			}
		}
	} else {
		//!bwImage
		if(bufferGIF && bufferPNG && (sizeGIF<=sizePNG)){
			imageToSend = GIF_IMAGE;
		}else if(bufferGIF && bufferPNG && (sizePNG<=sizeGIF)){
			imageToSend = PNG_IMAGE;
		}else if(bufferGIF){
			imageToSend = GIF_IMAGE;
		}else if(bufferPNG){
			imageToSend = PNG_IMAGE;
		}else if(bufferWBMP){
			imageToSend = WBMP_IMAGE;
		}else{
			imageToSend = WITHOUT_IMAGE;
		}
	}

	switch(imageToSend){
		case GIF_IMAGE:
			//AddDebugLogLine(false, "Wap Server: Send GIF CxImage");
			Data.pSocket->SendContent("Server: eMule\r\nConnection: close\r\nCache-Control: no-cache\r\nContent-Type: image/gif\r\n", bufferGIF, sizeGIF);
			break;
		case PNG_IMAGE:
			//AddDebugLogLine(false, "Wap Server: Send PNG CxImage");
			Data.pSocket->SendContent("Server: eMule\r\nConnection: close\r\nCache-Control: no-cache\r\nContent-Type: image/png\r\n", bufferPNG, sizePNG);
			break;
		case WBMP_IMAGE:
			//AddDebugLogLine(false, "Wap Server: Send WBMP CxImage");
			Data.pSocket->SendContent("Server: eMule\r\nConnection: close\r\nCache-Control: no-cache\r\nContent-Type: image/vnd.wap.wbmp\r\n", bufferWBMP, sizeWBMP);
			break;
		default:
			AddDebugLogLine(false, _T("Wap Server: Error sending CxImage -> No image send"));
			Data.pSocket->SendError404();
			break;
	}

	delete[] bufferPNG;
	delete[] bufferGIF;
	delete[] bufferWBMP;
}

CString CWapServer::RemoveWMLScriptInvalidChars(CString input)
{
	//This function removes accents, and chars >= 128 (Latin-4/ISO 8859-4) http://www.free-definition.com/ISO-8859-4.html
	//because some mobiles has problems with those chars in wmlscripts.
	CString tmpStr;
	tmpStr = input;
	for(int i=0; i<input.GetLength(); i++){ 
		TCHAR c;
		c = input.GetAt(i);
		switch (c){
			//Lower Case:
			case _T('\xE0'):
			case _T('\xE1'):
			case _T('\xE2'):
			case _T('\xE4'):
				c = _T('a');
				break;
			case _T('\xE8'):
			case _T('\xE9'):
			case _T('\xEA'):
			case _T('\xEB'):
				c = _T('e');
				break;
			case _T('\xEC'):
			case _T('\xED'):
			case _T('\xEE'):
			case _T('\xEF'):
				c = _T('i');
				break;
			case _T('\xF2'):
			case _T('\xF3'):
			case _T('\xF4'):
			case _T('\xF6'):
				c = _T('o');
				break;
			case _T('\xF9'):
			case _T('\xFA'):
			case _T('\xFB'):
			case _T('\xFC'):
				c = _T('u');
				break;
			case _T('\xFD'):
			case _T('\xFF'):
				c = _T('y');
				break;
			case _T('\xF1'):
				c = _T('n');
				break;

			//Upper Case:
			case _T('\xC0'):
			case _T('\xC1'):
			case _T('\xC2'):
			case _T('\xC4'):
				c = _T('A');
				break;
			case _T('\xC8'):
			case _T('\xC9'):
			case _T('\xCA'):
			case _T('\xCB'):
				c = _T('E');
				break;
			case _T('\xCC'):
			case _T('\xCD'):
			case _T('\xCE'):
			case _T('\xCF'):
				c = _T('I');
				break;
			case _T('\xD2'):
			case _T('\xD3'):
			case _T('\xD4'):
			case _T('\xD6'):
				c = _T('O');
				break;
			case _T('\xD9'):
			case _T('\xDA'):
			case _T('\xDB'):
			case _T('\xDC'):
				c = _T('U');
				break;
			case _T('\xDD'):
				c = _T('Y');
				break;
			case _T('\xD1'):
				c = _T('N');
				break;

			//Default Chars
			default:
				if((unsigned char)c >= 128)
					c = _T('_');
				break;
		}
		tmpStr.SetAt(i,c);
	}
	return tmpStr;
}

CString	CWapServer::_GetKadPage(WapThreadData Data)
{
	CWapServer *pThis = (CWapServer *)Data.pThis;
	if(pThis == NULL)
		return _T("");

	if (!thePrefs.GetNetworkKademlia()) return _T("");

	CString sSession = _ParseURL(Data.sURL, _T("ses"));
	CString Out = pThis->m_Templates.sKad;
	Out.Replace(_T("[Session]"), sSession);

	if (_ParseURL(Data.sURL, _T("bootstrap")) != "" && IsSessionAdmin(Data,sSession) ) {
		CString dest=_ParseURL(Data.sURL, _T("ipport"));
		int pos=dest.Find(_T(':'));
		if (pos!=-1) {
			uint16 port=(uint16)_ttoi(dest.Right( dest.GetLength()-pos-1));
			CString ip=dest.Left(pos);
			Kademlia::CKademlia::Bootstrap(ip,port);
		}
	}

	if (_ParseURL(Data.sURL, _T("c")) == "connect" && IsSessionAdmin(Data,sSession) ) {
		Kademlia::CKademlia::Start();
	}

	if (_ParseURL(Data.sURL, _T("c")) == "disconnect" && IsSessionAdmin(Data,sSession) ) {
		Kademlia::CKademlia::Stop();
	}

	// check the condition if bootstrap is possible
	if ( Kademlia::CKademlia::IsRunning() &&  !Kademlia::CKademlia::IsConnected()) {

		Out.Replace(_T("[BOOTSTRAPLINE]"), pThis->m_Templates.sBootstrapLine );

		// Bootstrap
		CString bsip=_ParseURL(Data.sURL, _T("bsip"));
		uint16 bsport=(uint16)_ttoi(_ParseURL(Data.sURL, _T("bsport")));

		if (!bsip.IsEmpty() && bsport>0)
			Kademlia::CKademlia::Bootstrap(bsip,bsport);
	} else Out.Replace(_T("[BOOTSTRAPLINE]"), _T("") );

	// Infos
	CString info;
	if (thePrefs.GetNetworkKademlia()) {
		CString buffer;
			
			buffer.Format(_T("%s: %i<br/>"), _GetPlainResString(IDS_KADCONTACTLAB), theApp.emuledlg->kademliawnd->GetContactCount());
			info.Append(buffer);

			buffer.Format(_T("%s: %i<br/>"), _GetPlainResString(IDS_KADSEARCHLAB), theApp.emuledlg->kademliawnd->searchList->GetItemCount());
			info.Append(buffer);

		
	} else info=_T("");
	Out.Replace(_T("[KADSTATSDATA]"),info);

	Out.Replace(_T("[BS_IP]"),_GetPlainResString(IDS_IP));
	Out.Replace(_T("[BS_PORT]"),_GetPlainResString(IDS_PORT));
	Out.Replace(_T("[BOOTSTRAP]"),_GetPlainResString(IDS_BOOTSTRAP));
	Out.Replace(_T("[KADSTAT]"),_GetPlainResString(IDS_STATSSETUPINFO));
	Out.Replace(_T("[Section]"), _GetPlainResString(IDS_KADEMLIA));
	Out.Replace(_T("[Home]"), _GetPlainResString(IDS_WAP_HOME));
	Out.Replace(_T("[Back]"), _GetPlainResString(IDS_WAP_BACK));

	return Out;
}