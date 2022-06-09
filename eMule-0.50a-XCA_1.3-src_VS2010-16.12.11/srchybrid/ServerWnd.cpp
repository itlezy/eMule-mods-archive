//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
//
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
#include "emule.h"
#include "ServerWnd.h"
#include "HttpDownloadDlg.h"
#include "HTRichEditCtrl.h"
#include "ED2KLink.h"
#include "kademlia/kademlia/kademlia.h"
#include "kademlia/kademlia/prefs.h"
#include "kademlia/utils/MiscUtils.h"
#include "OtherFunctions.h"
#include "emuledlg.h"
#include "WebServer.h"
#include "CustomAutoComplete.h"
#include "Server.h"
#include "ServerList.h"
#include "Sockets.h"
#include "MuleStatusBarCtrl.h"
#include "NetworkInfoDlg.h"
#include "Log.h"
#include "UserMsgs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	SVWND_SPLITTER_YOFF		6
#define	SVWND_SPLITTER_HEIGHT	4

#define	SERVERMET_STRINGS_PROFILE	_T("AC_ServerMetURLs.dat")
#define SZ_DEBUG_LOG_TITLE			_T("Verbose")

// CServerWnd dialog

IMPLEMENT_DYNAMIC(CServerWnd, CDialog)

BEGIN_MESSAGE_MAP(CServerWnd, CResizableDialog)
	ON_BN_CLICKED(IDC_ADDSERVER, OnBnClickedAddserver)
	ON_BN_CLICKED(IDC_UPDATESERVERMETFROMURL, OnBnClickedUpdateServerMetFromUrl)
	ON_BN_CLICKED(IDC_LOGRESET, OnBnClickedResetLog)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB3, OnTcnSelchangeTab3)
	ON_NOTIFY(EN_LINK, IDC_LOGBOX, OnEnLinkLogBox)
	ON_BN_CLICKED(IDC_ED2KCONNECT, OnBnConnect)
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_DD,OnDDClicked)
	ON_EN_CHANGE(IDC_IPADDRESS, OnSvrTextChange)
	ON_EN_CHANGE(IDC_SPORT, OnSvrTextChange)
	ON_EN_CHANGE(IDC_SNAME, OnSvrTextChange)
	ON_EN_CHANGE(IDC_SERVERMETURL, OnSvrTextChange)
	ON_STN_DBLCLK(IDC_SERVLST_ICO, OnStnDblclickServlstIco)
	ON_NOTIFY(UM_SPN_SIZED, IDC_SPLITTER_SERVER, OnSplitterMoved)
END_MESSAGE_MAP()

CServerWnd::CServerWnd(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CServerWnd::IDD, pParent)
{
	logbox = new CHTRichEditCtrl;
	debuglog = new CHTRichEditCtrl;
	m_pacServerMetURL=NULL;
	icon_srvlist = NULL;
	memset(&m_cfDef, 0, sizeof m_cfDef);
	memset(&m_cfBold, 0, sizeof m_cfBold);
	StatusSelector.m_bCloseable = false;
}

CServerWnd::~CServerWnd()
{
	if (icon_srvlist)
		VERIFY( DestroyIcon(icon_srvlist) );
	if (m_pacServerMetURL){
		m_pacServerMetURL->Unbind();
		m_pacServerMetURL->Release();
	}
	delete debuglog;
	delete logbox;
}

BOOL CServerWnd::OnInitDialog()
{
	CResizableDialog::OnInitDialog();
	if (theApp.m_fontLog.m_hObject == NULL)
	{
		CFont* pFont = m_ctrlNewServerFrm.GetFont();
		LOGFONT lf;
		pFont->GetObject(sizeof lf, &lf);
		theApp.m_fontLog.CreateFontIndirect(&lf);
	}

//	ReplaceRichEditCtrl(&m_MyInfo, this, m_ctrlNewServerFrm.GetFont());// X: [CI] - [Code Improvement] Don't need replace under VS2005+

//>> pP: - Set Server.met URL
	CString strText;
	GetDlgItem(IDC_SERVERMETURL)->GetWindowText(strText);

	if (strText.IsEmpty())
	{
             //strText = L"http://www.peerates.net/servers.php";
             //strText = L"http://www.gruk.org/server.met";
               strText = L"http://upd.emule-security.org/server.met";
		GetDlgItem(IDC_SERVERMETURL)->SetWindowText(L"");
		GetDlgItem(IDC_SERVERMETURL)->SetWindowText(strText);
		((CEdit*)GetDlgItem(IDC_SERVERMETURL))->SetSel(strText.GetLength(), strText.GetLength());
	}
	//<< pP: - Set Server.met URL

	// using ES_NOHIDESEL is actually not needed, but it helps to get around a tricky window update problem!
	// If that style is not specified there are troubles with right clicking into the control for the very first time!?
#define	LOG_PANE_RICHEDIT_STYLES WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_READONLY | ES_NOHIDESEL
	RECT rect;

	GetDlgItem(IDC_LOGBOX)->GetWindowRect(&rect);
	GetDlgItem(IDC_LOGBOX)->DestroyWindow();
	::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&rect, 2);
	if (logbox->Create(LOG_PANE_RICHEDIT_STYLES, rect, this, IDC_LOGBOX)){
		logbox->SetProfileSkinKey(_T("Log"));
		logbox->ModifyStyleEx(0, WS_EX_STATICEDGE, SWP_FRAMECHANGED);
		logbox->SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(3, 3));
		logbox->SetEventMask(logbox->GetEventMask() | ENM_LINK);
		if (theApp.m_fontLog.m_hObject)
			logbox->SetFont(&theApp.m_fontLog);
		logbox->ApplySkin();
		logbox->SetTitle(GetResString(IDS_SV_LOG));
		logbox->SetAutoURLDetect(FALSE);
 		logbox->AppendText(_T("eMule v") + theApp.m_strCurVersionLong + _T("\n"));
// MOD Note: Do not remove this part - Merkur
		if (thePrefs.UpdateNotify()){
		m_strClickNewVersion = GetResString(IDS_EMULEW) + _T(" ") + GetResString(IDS_EMULEW3) + _T(" ") + GetResString(IDS_EMULEW2);
		logbox->AppendHyperLink(_T(""), _T(""), m_strClickNewVersion, _T(""));
        logbox->AppendText(_T("\n"));
		}
		// MOD Note: end
       logbox->AppendText(_T("\n"));
	}

	GetDlgItem(IDC_DEBUG_LOG)->GetWindowRect(&rect);
	GetDlgItem(IDC_DEBUG_LOG)->DestroyWindow();
	::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&rect, 2);
	if (debuglog->Create(LOG_PANE_RICHEDIT_STYLES, rect, this, IDC_DEBUG_LOG)){
		debuglog->SetProfileSkinKey(_T("VerboseLog"));
		debuglog->ModifyStyleEx(0, WS_EX_STATICEDGE, SWP_FRAMECHANGED);
		debuglog->SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(3, 3));
		if (theApp.m_fontLog.m_hObject)
			debuglog->SetFont(&theApp.m_fontLog);
		debuglog->ApplySkin();
		debuglog->SetTitle(SZ_DEBUG_LOG_TITLE);
		debuglog->SetAutoURLDetect(FALSE);
	}

	SetAllIcons();
	serverlistctrl.Init();

	((CEdit*)GetDlgItem(IDC_SPORT))->SetLimitText(5);
	SetDlgItemText(IDC_SPORT,_T("4661"));

	TCITEM newitem;
	/*CString name;// X: [RUL] - [Remove Useless Localize]
	name = GetResString(IDS_SV_LOG);
	name.Replace(_T("&"), _T("&&"));*/
	newitem.mask = TCIF_TEXT | TCIF_IMAGE;
	newitem.pszText = _T("")/*const_cast<LPTSTR>((LPCTSTR)name)*/;
	newitem.iImage = 0;
	VERIFY( StatusSelector.InsertItem(StatusSelector.GetItemCount(), &newitem) == PaneLog );

	/*name = SZ_DEBUG_LOG_TITLE;
	name.Replace(_T("&"), _T("&&"));
	newitem.mask = TCIF_TEXT | TCIF_IMAGE;*/
	newitem.pszText = _T("")/*const_cast<LPTSTR>((LPCTSTR)name)*/;
	newitem.iImage = 0;
	VERIFY( StatusSelector.InsertItem(StatusSelector.GetItemCount(), &newitem) == PaneVerboseLog );

	Localize();

	AddAnchor(IDC_SERVLST_ICO, TOP_LEFT);
	AddAnchor(IDC_SERVLIST_TEXT, TOP_LEFT);
	AddAnchor(serverlistctrl, TOP_LEFT, MIDDLE_RIGHT);
	AddAnchor(m_ctrlNewServerFrm, TOP_RIGHT);
	AddAnchor(IDC_SSTATIC4, TOP_RIGHT);
	AddAnchor(IDC_SSTATIC7, TOP_RIGHT);
	AddAnchor(IDC_IPADDRESS, TOP_RIGHT);
	AddAnchor(IDC_SSTATIC3, TOP_RIGHT);
	AddAnchor(IDC_SNAME, TOP_RIGHT);
	AddAnchor(IDC_ADDSERVER, TOP_RIGHT);
	AddAnchor(IDC_SSTATIC5, TOP_RIGHT);
	AddAnchor(m_ctrlMyInfoFrm, TOP_RIGHT, BOTTOM_RIGHT);
	AddAnchor(m_MyInfo, TOP_RIGHT, BOTTOM_RIGHT);
	AddAnchor(IDC_SPORT, TOP_RIGHT);
	AddAnchor(m_ctrlUpdateServerFrm, TOP_RIGHT);
	AddAnchor(IDC_SERVERMETURL, TOP_RIGHT);
	AddAnchor(IDC_UPDATESERVERMETFROMURL, TOP_RIGHT);
	AddAnchor(StatusSelector, MIDDLE_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_LOGRESET, MIDDLE_RIGHT); // avoid resizing GUI glitches with the tab control by adding this control as the last one (Z-order)
	AddAnchor(IDC_ED2KCONNECT, TOP_RIGHT);
	AddAnchor(IDC_DD, TOP_RIGHT);
	// The resizing of those log controls (rich edit controls) works 'better' when added as last anchors (?)
	AddAnchor(*logbox, MIDDLE_LEFT, BOTTOM_RIGHT);
	AddAnchor(*debuglog, MIDDLE_LEFT, BOTTOM_RIGHT);

	// Set the tab control to the bottom of the z-order. This solves a lot of strange repainting problems with
	// the rich edit controls (the log panes).
	::SetWindowPos(StatusSelector, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOSIZE);

	debug = true;
	ToggleDebugWindow();

	debuglog->ShowWindow(SW_HIDE);
	logbox->ShowWindow(SW_SHOW);

	// optional: restore last used log pane
	if (thePrefs.GetRestoreLastLogPane())
	{
		if (thePrefs.GetLastLogPaneID() >= 0 && thePrefs.GetLastLogPaneID() < StatusSelector.GetItemCount())
		{
			int iCurSel = StatusSelector.GetCurSel();
			StatusSelector.SetCurSel(thePrefs.GetLastLogPaneID());
			if (thePrefs.GetLastLogPaneID() == StatusSelector.GetCurSel())
				UpdateLogTabSelection();
			else
				StatusSelector.SetCurSel(iCurSel);
		}
	}

	m_MyInfo.SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(3, 3));
	m_MyInfo.SetAutoURLDetect();
	m_MyInfo.SetEventMask(m_MyInfo.GetEventMask() | ENM_LINK);

	PARAFORMAT pf = {0};
	pf.cbSize = sizeof pf;
	if (m_MyInfo.GetParaFormat(pf)){
		pf.dwMask |= PFM_TABSTOPS;
		pf.cTabCount = 4;
		pf.rgxTabs[0] = 900;
		pf.rgxTabs[1] = 1000;
		pf.rgxTabs[2] = 1100;
		pf.rgxTabs[3] = 1200;
		m_MyInfo.SetParaFormat(pf);
	}

	m_cfDef.cbSize = sizeof m_cfDef;
	if (m_MyInfo.GetSelectionCharFormat(m_cfDef)){
		m_cfBold = m_cfDef;
		m_cfBold.dwMask |= CFM_BOLD;
		m_cfBold.dwEffects |= CFE_BOLD;
	}

	if (thePrefs.GetUseAutocompletion()){
		m_pacServerMetURL = new CCustomAutoComplete();
		m_pacServerMetURL->AddRef();
		if (m_pacServerMetURL->Bind(::GetDlgItem(m_hWnd, IDC_SERVERMETURL), ACO_UPDOWNKEYDROPSLIST | ACO_AUTOSUGGEST | ACO_FILTERPREFIXES ))
			m_pacServerMetURL->LoadList(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + SERVERMET_STRINGS_PROFILE);
		if (theApp.m_fontSymbol.m_hObject){
			GetDlgItem(IDC_DD)->SetFont(&theApp.m_fontSymbol);
			SetDlgItemText(IDC_DD,_T("6")); // show a down-arrow
		}
	}
	else
		GetDlgItem(IDC_DD)->ShowWindow(SW_HIDE);

	InitWindowStyles(this);

	// splitter
	CRect rcSpl;
	rcSpl.left = 55;
	rcSpl.right = 300;
	rcSpl.top = 55;
	rcSpl.bottom = rcSpl.top + SVWND_SPLITTER_HEIGHT;
	m_wndSplitter.Create(WS_CHILD | WS_VISIBLE, rcSpl, this, IDC_SPLITTER_SERVER);
	m_wndSplitter.SetDrawBorder(true);
	InitSplitter();

	return true;
}

void CServerWnd::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SERVLIST, serverlistctrl);
	DDX_Control(pDX, IDC_SSTATIC, m_ctrlNewServerFrm);
	DDX_Control(pDX, IDC_SSTATIC6, m_ctrlUpdateServerFrm);
	DDX_Control(pDX, IDC_MYINFO, m_ctrlMyInfoFrm);
	DDX_Control(pDX, IDC_TAB3, StatusSelector);
	DDX_Control(pDX, IDC_MYINFOLIST, m_MyInfo);
}

bool CServerWnd::UpdateServerMetFromURL(CString strURL)
{
	if (strURL.IsEmpty() || (strURL.Find(_T("://")) == -1)) {
		// not a valid URL
		LogError(LOG_STATUSBAR, GetResString(IDS_INVALIDURL) );
		return false;
	}

	// add entered URL to LRU list even if it's not yet known whether we can download from this URL (it's just more convenient this way)
	if (m_pacServerMetURL && m_pacServerMetURL->IsBound())
		m_pacServerMetURL->AddItem(strURL, 0);

	CString strTempFilename;
	strTempFilename.Format(_T("%stemp-%d-server.met"), thePrefs.GetMuleDirectory(EMULE_CONFIGDIR), ::GetTickCount());

	// try to download server.met
	Log(GetResString(IDS_DOWNLOADING_SERVERMET_FROM), strURL);
	CHttpDownloadDlg dlgDownload;
	dlgDownload.m_strTitle = GetResString(IDS_DOWNLOADING_SERVERMET);
	dlgDownload.m_sURLToDownload = strURL;
	dlgDownload.m_sFileToDownloadInto = strTempFilename;
	if (dlgDownload.DoModal() != IDOK) {
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_FAILEDDOWNLOADMET), strURL);
		return false;
	}

	// add content of server.met to serverlist
	serverlistctrl.ShowWindow(SW_HIDE);
	serverlistctrl.AddServerMetToList(strTempFilename);
	serverlistctrl.ShowWindow(SW_SHOW);
	(void)_tremove(strTempFilename);
	return true;
}

void CServerWnd::OnSysColorChange()
{
	CResizableDialog::OnSysColorChange();
	SetAllIcons();
}

void CServerWnd::SetAllIcons()
{
	m_ctrlNewServerFrm.SetIcon(_T("AddServer"));
	m_ctrlUpdateServerFrm.SetIcon(_T("ServerUpdateMET"));
	m_ctrlMyInfoFrm.SetIcon(_T("Info"));

	CImageList iml;
	iml.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);
	iml.Add(CTempIconLoader(_T("FILEINFO")));
	iml.Add(CTempIconLoader(_T("FILEINFO")));
	StatusSelector.SetImageList(&iml);
	m_imlLogPanes.DeleteImageList();
	m_imlLogPanes.Attach(iml.Detach());

	if (icon_srvlist)
		VERIFY( DestroyIcon(icon_srvlist) );
	icon_srvlist = theApp.LoadIcon(_T("ServerList"), 16, 16);
	((CStatic*)GetDlgItem(IDC_SERVLST_ICO))->SetIcon(icon_srvlist);
}

void CServerWnd::Localize()// X: [RUL] - [Remove Useless Localize]
{
	//SetDlgItemText(IDC_SERVLIST_TEXT,GetResString(IDS_SV_SERVERLIST));
	m_ctrlNewServerFrm.SetWindowText(GetResString(IDS_SV_NEWSERVER));
	SetDlgItemText(IDC_SSTATIC4,GetResString(IDS_SV_ADDRESS));
	SetDlgItemText(IDC_SSTATIC7,GetResString(IDS_SV_PORT));
	SetDlgItemText(IDC_SSTATIC3,GetResString(IDS_SW_NAME));
	SetDlgItemText(IDC_ADDSERVER,GetResString(IDS_SV_ADD));
	m_ctrlUpdateServerFrm.SetWindowText(GetResString(IDS_SV_MET));
	SetDlgItemText(IDC_UPDATESERVERMETFROMURL,GetResString(IDS_SV_UPDATE));
	SetDlgItemText(IDC_LOGRESET,GetResString(IDS_PW_RESET));
	m_ctrlMyInfoFrm.SetWindowText(GetResString(IDS_MYINFO));

	TCITEM item;
	CString name = GetResString(IDS_SV_LOG);
	name.Replace(_T("&"), _T("&&"));
	item.mask = TCIF_TEXT;
	item.pszText = const_cast<LPTSTR>((LPCTSTR)name);
	StatusSelector.SetItem(PaneLog, &item);

	name = SZ_DEBUG_LOG_TITLE;
	name.Replace(_T("&"), _T("&&"));
	//item.mask = TCIF_TEXT;
	item.pszText = const_cast<LPTSTR>((LPCTSTR)name);
	StatusSelector.SetItem(PaneVerboseLog, &item);

	UpdateLogTabSelection();
	UpdateControlsState();
}

void CServerWnd::OnBnClickedAddserver()
{
	CString serveraddr;
	GetDlgItemText(IDC_IPADDRESS,serveraddr);
	serveraddr.Trim();
	if (serveraddr.IsEmpty()) {
		AfxMessageBox(GetResString(IDS_SRV_ADDR));
		return;
	}

	uint16 uPort = 0;
	if (_tcsnicmp(serveraddr, _T("ed2k://"), 7) == 0){
		CED2KLink* pLink = NULL;
		try{
			pLink = CED2KLink::CreateLinkFromUrl(serveraddr);
			serveraddr.Empty();
			if (pLink && pLink->GetKind() == CED2KLink::kServer){
				//CED2KServerLink* pServerLink = pLink->GetServerLink();
				//if (pServerLink){
				CED2KServerLink* pServerLink = (CED2KServerLink*)pLink;
				serveraddr = pServerLink->GetAddress();
				uPort = pServerLink->GetPort();
				SetDlgItemText(IDC_IPADDRESS, serveraddr);
				SetDlgItemInt(IDC_SPORT, uPort, FALSE);
				//}
			}
		}
		catch(CString strError){
			AfxMessageBox(strError);
			serveraddr.Empty();
		}
		delete pLink;
	}
	else{
		// if the port is specified with the IP, ignore any possible specified port in the port control
		int iColon = serveraddr.Find(_T(':'));
		if (iColon != -1) {
			uPort = (uint16)_tstoi(serveraddr.Mid(iColon + 1));
			serveraddr = serveraddr.Left(iColon);
		}
		else {
			if (!GetDlgItem(IDC_SPORT)->GetWindowTextLength()){
				AfxMessageBox(GetResString(IDS_SRV_PORT));
				return;
			}

			BOOL bTranslated = FALSE;
			uPort = (uint16)GetDlgItemInt(IDC_SPORT, &bTranslated, FALSE);
			if (!bTranslated){
				AfxMessageBox(GetResString(IDS_SRV_PORT));
				return;
			}
		}
	}

	if (serveraddr.IsEmpty() || uPort == 0){
		AfxMessageBox(GetResString(IDS_SRV_ADDR));
		return;
	}

	CString strServerName;
	GetDlgItemText(IDC_SNAME,strServerName);

	AddServer(uPort, serveraddr, strServerName);
}

void CServerWnd::PasteServerFromClipboard()
{
	CString strServer = theApp.CopyTextFromClipboard();
	strServer.Trim();
	if (strServer.IsEmpty())
		return;

	int nPos = 0;
	CString strTok = strServer.Tokenize(_T(" \t\r\n"), nPos);
	while (!strTok.IsEmpty())
	{
		CString strAddress;
		uint16 nPort = 0;
		CED2KLink* pLink = NULL;
		try{
			pLink = CED2KLink::CreateLinkFromUrl(strTok);
			if (pLink && pLink->GetKind() == CED2KLink::kServer){
				//CED2KServerLink* pServerLink = pLink->GetServerLink();
				//if (pServerLink){
				CED2KServerLink* pServerLink = (CED2KServerLink*)pLink;
				strAddress = pServerLink->GetAddress();
				nPort = pServerLink->GetPort();
				//}
			}
		}
		catch(CString strError){
			AfxMessageBox(strError);
		}
		delete pLink;

		if (strAddress.IsEmpty() || nPort == 0)
			break;

		(void)AddServer(nPort, strAddress, _T(""), false);
		strTok = strServer.Tokenize(_T(" \t\r\n"), nPos);
	}
}

bool CServerWnd::AddServer(uint16 nPort, CString strAddress, CString strName, bool bShowErrorMB)
{
	CServer* toadd = new CServer(nPort, strAddress);

	// Barry - Default all manually added servers to high priority
	if (thePrefs.GetManualAddedServersHighPriority())
		toadd->SetPreference(SRV_PR_HIGH);

	if (strName.IsEmpty())
		strName = strAddress;
	toadd->SetListName(strName);

	if (!serverlistctrl.AddServer(toadd, true))
	{
		CServer* pFoundServer = theApp.serverlist->GetServerByAddress(toadd->GetAddress(), toadd->GetPort());
		if (pFoundServer == NULL && toadd->GetIP() != 0)
			pFoundServer = theApp.serverlist->GetServerByIPTCP(toadd->GetIP(), toadd->GetPort());
		if (pFoundServer)
		{
			static const TCHAR _aszServerPrefix[] = _T("Server");
			if (_tcsnicmp(toadd->GetListName(), _aszServerPrefix, ARRSIZE(_aszServerPrefix)-1) != 0)
			{
				pFoundServer->SetListName(toadd->GetListName());
				serverlistctrl.RefreshServer(pFoundServer);
			}
		}
		else
		{
			if (bShowErrorMB)
				AfxMessageBox(GetResString(IDS_SRV_NOTADDED));
		}
		delete toadd;
		return false;
	}
	else
	{
		AddLogLine(true, GetResString(IDS_SERVERADDED), toadd->GetListName());
		return true;
	}
}

void CServerWnd::OnBnClickedUpdateServerMetFromUrl()
{
	CString strURL;
	GetDlgItemText(IDC_SERVERMETURL,strURL);
	strURL.Trim();
	if (strURL.IsEmpty())
	{
		if (thePrefs.addresses_list.IsEmpty())
		{
			AddLogLine(true, GetResString(IDS_SRV_NOURLAV));
		}
		else
		{
			bool bDownloaded = false;
			POSITION pos = thePrefs.addresses_list.GetHeadPosition();
			while (!bDownloaded && pos != NULL)
			{
				strURL = thePrefs.addresses_list.GetNext(pos);
				bDownloaded = UpdateServerMetFromURL(strURL);
			}
		}
	}
	else
		UpdateServerMetFromURL(strURL);
}

void CServerWnd::OnBnClickedResetLog()
{
	int cur_sel = StatusSelector.GetCurSel();
	if (cur_sel == -1)
		return;
	
	if (cur_sel == PaneVerboseLog)
	{
		theApp.emuledlg->ResetDebugLog();
		theApp.emuledlg->statusbar->SetText(_T(""), SBarLog, 0);
	}
	if (cur_sel == PaneLog)
	{
		theApp.emuledlg->ResetLog();
		theApp.emuledlg->statusbar->SetText(_T(""), SBarLog, 0);
	}
}

void CServerWnd::OnTcnSelchangeTab3(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	UpdateLogTabSelection();
	*pResult = 0;
}

void CServerWnd::UpdateLogTabSelection()
{
	int cur_sel = StatusSelector.GetCurSel();
	if (cur_sel == -1)
		return;
	if (cur_sel == PaneVerboseLog)
	{
		logbox->ShowWindow(SW_HIDE);
		debuglog->ShowWindow(SW_SHOW);
		if (debuglog->IsAutoScroll() && (StatusSelector.GetItemState(cur_sel, TCIS_HIGHLIGHTED) & TCIS_HIGHLIGHTED))
			debuglog->ScrollToLastLine(true);
		debuglog->Invalidate();
		StatusSelector.HighlightItem(cur_sel, FALSE);
	}
	else if (cur_sel == PaneLog)
	{
		debuglog->ShowWindow(SW_HIDE);
		logbox->ShowWindow(SW_SHOW);
		if (logbox->IsAutoScroll() && (StatusSelector.GetItemState(cur_sel, TCIS_HIGHLIGHTED) & TCIS_HIGHLIGHTED))
			logbox->ScrollToLastLine(true);
		logbox->Invalidate();
		StatusSelector.HighlightItem(cur_sel, FALSE);
	}
}

void CServerWnd::ToggleDebugWindow()
{
	int cur_sel = StatusSelector.GetCurSel();
	if (thePrefs.GetVerbose() && !debug)
	{
		TCITEM newitem;
		CString name;
		name = SZ_DEBUG_LOG_TITLE;
		name.Replace(_T("&"), _T("&&"));
		newitem.mask = TCIF_TEXT | TCIF_IMAGE;
		newitem.pszText = const_cast<LPTSTR>((LPCTSTR)name);
		newitem.iImage = 0;
		StatusSelector.InsertItem(StatusSelector.GetItemCount(),&newitem);

		debug = true;
	}
	else if (!thePrefs.GetVerbose() && debug)
	{
		if (cur_sel == PaneVerboseLog) 
		{
			StatusSelector.SetCurSel(PaneLog);
			StatusSelector.SetFocus();
		}
		debuglog->ShowWindow(SW_HIDE);
		logbox->ShowWindow(SW_SHOW);
		StatusSelector.DeleteItem(PaneVerboseLog);
		debug = false;
	}
}

void CServerWnd::UpdateMyInfo()
{
	m_MyInfo.SetRedraw(FALSE);
	m_MyInfo.SetWindowText(_T(""));
	CreateNetworkInfo(m_MyInfo, m_cfDef, m_cfBold);
	m_MyInfo.SetRedraw(TRUE);
	m_MyInfo.Invalidate();
}

CString CServerWnd::GetMyInfoString() {
	CString buffer;
	m_MyInfo.GetWindowText(buffer);

	return buffer;
}

BOOL CServerWnd::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		// Don't handle Ctrl+Tab in this window. It will be handled by main window.
		if (pMsg->wParam == VK_TAB && GetAsyncKeyState(VK_CONTROL) < 0)
			return FALSE;
		if (pMsg->wParam == VK_ESCAPE)
			return FALSE;

		if (pMsg->wParam == VK_DELETE && m_pacServerMetURL && m_pacServerMetURL->IsBound() && pMsg->hwnd == GetDlgItem(IDC_SERVERMETURL)->m_hWnd)
		{
			if (GetAsyncKeyState(VK_MENU)<0 || GetAsyncKeyState(VK_CONTROL)<0)
				m_pacServerMetURL->Clear();
			else
				m_pacServerMetURL->RemoveSelectedItem();
		}

		if (pMsg->wParam == VK_RETURN)
		{
			if (   pMsg->hwnd == GetDlgItem(IDC_IPADDRESS)->m_hWnd
				|| pMsg->hwnd == GetDlgItem(IDC_SPORT)->m_hWnd
				|| pMsg->hwnd == GetDlgItem(IDC_SNAME)->m_hWnd)
			{
				OnBnClickedAddserver();
				return TRUE;
			}
			else if (pMsg->hwnd == GetDlgItem(IDC_SERVERMETURL)->m_hWnd)
			{
				if (m_pacServerMetURL && m_pacServerMetURL->IsBound())
				{
					CString strText;
					GetDlgItemText(IDC_SERVERMETURL,strText);
					if (!strText.IsEmpty())
					{
						SetDlgItemText(IDC_SERVERMETURL,_T("")); // this seems to be the only chance to let the dropdown list to disapear
						SetDlgItemText(IDC_SERVERMETURL,strText);
						((CEdit*)GetDlgItem(IDC_SERVERMETURL))->SetSel(strText.GetLength(), strText.GetLength());
					}
				}
				OnBnClickedUpdateServerMetFromUrl();
				return TRUE;
			}
		}
	}

	return CResizableDialog::PreTranslateMessage(pMsg);
}

BOOL CServerWnd::SaveServerMetStrings()
{
	if (m_pacServerMetURL== NULL)
		return FALSE;
	return m_pacServerMetURL->SaveList(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + SERVERMET_STRINGS_PROFILE);
}

void CServerWnd::ShowNetworkInfo()
{
	// NEO: MLD - [ModelesDialogs] -- Xanatos -->
	CNetworkInfoDlg* dialog = new CNetworkInfoDlg(); 
	dialog->OpenDialog();
	// NEO: MLD END <-- Xanatos --
	//CNetworkInfoDlg dlg;
	//dlg.DoModal();
}

void CServerWnd::OnEnLinkLogBox(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	ENLINK* pEnLink = reinterpret_cast<ENLINK *>(pNMHDR);
	if (pEnLink && pEnLink->msg == WM_LBUTTONDOWN)
	{
		CString strUrl;
		logbox->GetTextRange(pEnLink->chrg.cpMin, pEnLink->chrg.cpMax, strUrl);
		if (strUrl == m_strClickNewVersion){
			// MOD Note: Do not remove this part - Merkur
			strUrl.Format(_T("/en/version_check.php?version=%i&language=%i"),theApp.m_uCurVersionCheck,thePrefs.GetLanguageID());
			strUrl = thePrefs.GetVersionCheckBaseURL()+strUrl;
			// MOD Note: end
		}
		ShellExecute(NULL, NULL, strUrl, NULL, NULL, SW_SHOWDEFAULT);
		*pResult = 1;
	}
}

void CServerWnd::UpdateControlsState()
{
	CString strLabel;
	UINT StrID;
	if (theApp.serverconnect->IsConnected())
		StrID = IDS_MAIN_BTN_DISCONNECT;
	else if (theApp.serverconnect->IsConnecting())
		StrID = IDS_MAIN_BTN_CANCEL;
	else
		StrID = IDS_MAIN_BTN_CONNECT;
	strLabel = GetResString(StrID);
	strLabel.Remove(_T('&'));
	SetDlgItemText(IDC_ED2KCONNECT,strLabel);
}

void CServerWnd::OnBnConnect()
{
	if (theApp.serverconnect->IsConnected())
		theApp.serverconnect->Disconnect();
	else if (theApp.serverconnect->IsConnecting())
		theApp.serverconnect->StopConnectionTry();
	else
		theApp.serverconnect->ConnectToAnyServer();
}

void CServerWnd::SaveAllSettings()
{
	thePrefs.SetLastLogPaneID(StatusSelector.GetCurSel());
	SaveServerMetStrings();
}

void CServerWnd::OnDDClicked()
{
	CWnd* box = GetDlgItem(IDC_SERVERMETURL);
	box->SetFocus();
	box->SetWindowText(_T(""));
	box->SendMessage(WM_KEYDOWN, VK_DOWN, 0x00510001);
}

void CServerWnd::ResetHistory()
{
	if (m_pacServerMetURL == NULL)
		return;
	GetDlgItem(IDC_SERVERMETURL)->SendMessage(WM_KEYDOWN, VK_ESCAPE, 0x00510001);
	m_pacServerMetURL->Clear();
}

void CServerWnd::OnSvrTextChange()
{
	GetDlgItem(IDC_ADDSERVER)->EnableWindow(GetDlgItem(IDC_IPADDRESS)->GetWindowTextLength());
	GetDlgItem(IDC_UPDATESERVERMETFROMURL)->EnableWindow( GetDlgItem(IDC_SERVERMETURL)->GetWindowTextLength()>0 );
}

void CServerWnd::OnStnDblclickServlstIco()
{
	theApp.emuledlg->ShowPreferences(IDD_PPG_SERVER);
}

void CServerWnd::DoResize(int delta)
{
	//if(!delta)
		//return;
	CSplitterControl::ChangeHeight(&serverlistctrl, delta, CW_TOPALIGN);
	CSplitterControl::ChangeHeight(&StatusSelector, -delta, CW_BOTTOMALIGN);
	CSplitterControl::ChangeHeight(logbox, -delta, CW_BOTTOMALIGN);
	CSplitterControl::ChangeHeight(debuglog, -delta, CW_BOTTOMALIGN);
	UpdateSplitterRange();
}

void CServerWnd::InitSplitter()
{
	CRect rcWnd;
	GetWindowRect(rcWnd);
	ScreenToClient(rcWnd);

	m_wndSplitter.SetRange(rcWnd.top+100,rcWnd.bottom-50);
	LONG splitpos = 5+(thePrefs.GetSplitterbarPositionServer() * rcWnd.Height()) / 100;

	CRect rcDlgItem;

	serverlistctrl.GetWindowRect(rcDlgItem);
	ScreenToClient(rcDlgItem);
	rcDlgItem.bottom=splitpos-10;
	serverlistctrl.MoveWindow(rcDlgItem);

	GetDlgItem(IDC_LOGRESET)->GetWindowRect(rcDlgItem);
	ScreenToClient(rcDlgItem);
	rcDlgItem.top = splitpos + 9;
	rcDlgItem.bottom = splitpos + 30;
	GetDlgItem(IDC_LOGRESET)->MoveWindow(rcDlgItem);

	StatusSelector.GetWindowRect(rcDlgItem);
	ScreenToClient(rcDlgItem);
	rcDlgItem.top = splitpos + 10;
	rcDlgItem.bottom = rcWnd.bottom-5;
	StatusSelector.MoveWindow(rcDlgItem);

	logbox->GetWindowRect(rcDlgItem);
	ScreenToClient(rcDlgItem);
	rcDlgItem.top=splitpos+35;
	rcDlgItem.bottom = rcWnd.bottom-12;

	//logbox->GetWindowRect(rcDlgItem);
	//ScreenToClient(rcDlgItem);
	//rcDlgItem.top=splitpos+35;
	//rcDlgItem.bottom = rcWnd.bottom-12;
	logbox->MoveWindow(rcDlgItem);

	//debuglog->GetWindowRect(rcDlgItem);
	//ScreenToClient(rcDlgItem);
	//rcDlgItem.top=splitpos+35;
	//rcDlgItem.bottom = rcWnd.bottom-12;
	debuglog->MoveWindow(rcDlgItem);

	long right=rcDlgItem.right;
	GetDlgItem(IDC_SPLITTER_SERVER)->GetWindowRect(rcDlgItem);
	ScreenToClient(rcDlgItem);
	rcDlgItem.right=right;
	GetDlgItem(IDC_SPLITTER_SERVER)->MoveWindow(rcDlgItem);

	ReattachAnchors();
}

void CServerWnd::ReattachAnchors()
{
	RemoveAnchor(serverlistctrl);
	RemoveAnchor(StatusSelector);
	RemoveAnchor(IDC_LOGRESET);
	RemoveAnchor(*logbox);
	RemoveAnchor(*debuglog);

	AddAnchor(serverlistctrl, TOP_LEFT, CSize(100, thePrefs.GetSplitterbarPositionServer()));
	AddAnchor(StatusSelector, CSize(0, thePrefs.GetSplitterbarPositionServer()), BOTTOM_RIGHT);
	AddAnchor(IDC_LOGRESET, MIDDLE_RIGHT);
	AddAnchor(*logbox, CSize(0, thePrefs.GetSplitterbarPositionServer()), BOTTOM_RIGHT);
	AddAnchor(*debuglog, CSize(0, thePrefs.GetSplitterbarPositionServer()), BOTTOM_RIGHT);

	GetDlgItem(IDC_LOGRESET)->Invalidate();

	if (logbox->IsWindowVisible())
		logbox->Invalidate();
	if (debuglog->IsWindowVisible())
		debuglog->Invalidate();
}

void CServerWnd::UpdateSplitterRange()
{
	CRect rcWnd;
	GetWindowRect(rcWnd);
	ScreenToClient(rcWnd);

	CRect rcDlgItem;
	serverlistctrl.GetWindowRect(rcDlgItem);
	ScreenToClient(rcDlgItem);

	m_wndSplitter.SetRange(rcWnd.top + 100, rcWnd.bottom - 50);

	LONG splitpos = rcDlgItem.bottom + SVWND_SPLITTER_YOFF;
	thePrefs.SetSplitterbarPositionServer((splitpos  * 100) / rcWnd.Height());

	GetDlgItem(IDC_LOGRESET)->GetWindowRect(rcDlgItem);
	ScreenToClient(rcDlgItem);
	rcDlgItem.top = splitpos + 9;
	rcDlgItem.bottom = splitpos + 30;
	GetDlgItem(IDC_LOGRESET)->MoveWindow(rcDlgItem);

	ReattachAnchors();
}

LRESULT CServerWnd::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message)
	{
		// arrange transferwindow layout
		case WM_PAINT:
			if (m_wndSplitter)
			{
				CRect rcWnd;
				GetWindowRect(rcWnd);
				if (rcWnd.Height() > 0)
				{
					RECT rcDown;
					serverlistctrl.GetWindowRect(&rcDown);
					ScreenToClient(&rcDown);

					// splitter paint update
					RECT rcSpl;
					rcSpl.left = 10;
					rcSpl.right = rcDown.right;
					rcSpl.top = rcDown.bottom + SVWND_SPLITTER_YOFF;
					rcSpl.bottom = rcSpl.top + SVWND_SPLITTER_HEIGHT;
					m_wndSplitter.MoveWindow(&rcSpl, TRUE);
					UpdateSplitterRange();
				}
			}
			break;

		case WM_WINDOWPOSCHANGED:
			if (m_wndSplitter)
				m_wndSplitter.Invalidate();
			break;
	}

	return CResizableDialog::DefWindowProc(message, wParam, lParam);
}

void CServerWnd::OnSplitterMoved(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	SPC_NMHDR* pHdr = (SPC_NMHDR*)pNMHDR;
	DoResize(pHdr->delta);
}

HBRUSH CServerWnd::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = theApp.emuledlg->GetCtlColor(pDC, pWnd, nCtlColor);
	if (hbr)
		return hbr;
	return __super::OnCtlColor(pDC, pWnd, nCtlColor);
}
