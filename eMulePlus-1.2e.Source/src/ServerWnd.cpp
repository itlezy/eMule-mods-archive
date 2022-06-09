//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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
#include "ServerList.h"
#include "ServerWnd.h"
#include "server.h"
#include "HttpDownloadDlg.h"
#include "HTRichEditCtrl.h"
#include "NewServerDlg.h"
#include "UpdateServerMetDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// CServerWnd dialog

IMPLEMENT_DYNAMIC(CServerWnd, CDialog)
CServerWnd::CServerWnd(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CServerWnd::IDD, pParent)
{
	m_pctlServerMsgBox = new CHTRichEditCtrl;
	m_pctlLogBox = new CHTRichEditCtrl;
	m_pctlDebugBox = new CHTRichEditCtrl;
}

CServerWnd::~CServerWnd()
{
	delete m_pctlServerMsgBox;
	delete m_pctlLogBox;
	delete m_pctlDebugBox;
}

BOOL CServerWnd::OnInitDialog()
{
	static const uint16 s_auIconResID[] =
	{
		IDI_LOG,
		IDI_SERVERINFO
	};

	CResizableDialog::OnInitDialog();
	m_ctlServerList.Init();

	m_ctrlBoxSwitcher.ModifyStyle(NULL, TCS_OWNERDRAWFIXED);

	m_imageList.Create(16, 16, g_App.m_iDfltImageListColorFlags | ILC_MASK, ARRSIZE(s_auIconResID), 0);
	m_imageList.SetBkColor(RGB(255, 0, 255));
	FillImgLstWith16x16Icons(&m_imageList, s_auIconResID, ARRSIZE(s_auIconResID));

	CImageList		imageList;
	CBitmap			Bitmap, *pOldBitmap;
	CDC				*pCtrlDC = GetDC(), TempDC;

	TempDC.CreateCompatibleDC(pCtrlDC);
	Bitmap.CreateCompatibleBitmap(pCtrlDC, 16 * 2, 16);
	pOldBitmap = TempDC.SelectObject(&Bitmap);
	m_imageList.Draw(&TempDC, 0, CPoint(0, 0), ILD_NORMAL);
	m_imageList.Draw(&TempDC, 1, CPoint(16, 0), ILD_NORMAL);
	TempDC.SelectObject(pOldBitmap);
	imageList.Create(16, 16, g_App.m_iDfltImageListColorFlags | ILC_MASK, 0, 10);
	imageList.Add(&Bitmap, RGB(255, 0, 255));
	m_ctrlBoxSwitcher.SetImageList(&imageList);
	imageList.Detach();
	//Bitmap.Detach();
	Bitmap.DeleteObject();
	ReleaseDC(pCtrlDC);
///////////////////////////////////////////////////////

	m_ctrlBoxSwitcher.InsertItem(TCIF_IMAGE | TCIF_TEXT, 0, GetResString(IDS_SV_LOG), 0, 0, 0, 0);
	m_ctrlBoxSwitcher.InsertItem(TCIF_IMAGE | TCIF_TEXT, 1, GetResString(IDS_SV_SERVERINFO), 1, 0, 0, 0);

	CRect		rBox;

	GetDlgItem(IDC_LOG)->GetWindowRect(rBox);
	::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&rBox, 2);
	m_pctlServerMsgBox->Create(WS_VISIBLE | WS_CHILD | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_NOHIDESEL, rBox, this, IDC_SERVERMSGBOX);
	m_pctlServerMsgBox->ModifyStyleEx(0, WS_EX_STATICEDGE, SWP_FRAMECHANGED);
	m_pctlLogBox->Create(WS_VISIBLE | WS_CHILD | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_NOHIDESEL, rBox, this, IDC_LOGBOX);
	m_pctlLogBox->ModifyStyleEx(0, WS_EX_STATICEDGE, SWP_FRAMECHANGED);
	m_pctlDebugBox->Create(WS_VISIBLE | WS_CHILD | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_NOHIDESEL, rBox, this, IDC_DEBUGBOX);
	m_pctlDebugBox->ModifyStyleEx(0, WS_EX_STATICEDGE, SWP_FRAMECHANGED);

	InitFont();
	oldServerListIcon = GetServerLstStatic()->SetIcon(GetServerLstIcon());
	m_pctlServerMsgBox->AppendText(CLIENT_NAME_WITH_VER _T(" (http://emuleplus.info)\n"), CSTRLEN(CLIENT_NAME_WITH_VER _T(" (http://emuleplus.info)\n")), RGB(153,51,102));
	m_pctlServerMsgBox->AppendText(GetResString(IDS_EMULEW) + _T(' '), RGB(153,51,102), CLR_DEFAULT, HTC_HAVENOLINK);
	m_pctlServerMsgBox->AppendText(GetResString(IDS_EMULEW3), RGB(153,51,102), CLR_DEFAULT, HTC_LINK);
	m_pctlServerMsgBox->AppendText(_T(' ') + GetResString(IDS_EMULEW2) + _T("\n\n"), RGB(153,51,102), CLR_DEFAULT, HTC_HAVENOLINK);

	m_ctrlNewServerBtn.SetThemeHelper(&g_App.m_pMDlg->m_themeHelper);
	m_ctrlNewServerBtn.SetIcon(IDI_NEWSERVER);
	m_ctrlUpdateServerMetBtn.SetThemeHelper(&g_App.m_pMDlg->m_themeHelper);
	m_ctrlUpdateServerMetBtn.SetIcon(IDI_UPDATESERVERS);
	m_ctrlResetLogBtn.SetThemeHelper(&g_App.m_pMDlg->m_themeHelper);
	m_ctrlResetLogBtn.SetIcon(IDI_RESETLOG);

	AddAnchor(IDC_SERVLST_ICO, TOP_LEFT);
	AddAnchor(IDC_SERVLIST_TEXT, TOP_LEFT);
	AddAnchor(IDC_SERVLIST, TOP_LEFT, MIDDLE_RIGHT);
	AddAnchor(IDC_TAB1, MIDDLE_LEFT, BOTTOM_RIGHT);
	AddAnchor(*m_pctlServerMsgBox, MIDDLE_LEFT, BOTTOM_RIGHT);
	AddAnchor(*m_pctlLogBox, MIDDLE_LEFT, BOTTOM_RIGHT);
	AddAnchor(*m_pctlDebugBox, MIDDLE_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_NEWSERVER, TOP_RIGHT);
	AddAnchor(IDC_UPDATESERVERMET, TOP_RIGHT);
	AddAnchor(IDC_LOGRESET, MIDDLE_RIGHT);
	bDebug = false;
	ToggleDebugWindow();
	m_pctlServerMsgBox->ShowWindow(SW_HIDE);
	m_pctlDebugBox->ShowWindow(SW_HIDE);
	m_pctlLogBox->ShowWindow(SW_HIDE);
	if (m_pctlLogBox->m_hWnd)
		m_pctlLogBox->ShowWindow(SW_SHOW);

	Localize();

	m_ttip.Create(this);
	m_ttip.SetDelayTime(TTDT_AUTOPOP, 15000);
	m_ttip.SetDelayTime(TTDT_INITIAL, g_App.m_pPrefs->GetToolTipDelay()*1000);
	m_ttip.SendMessage(TTM_SETMAXTIPWIDTH, 0, SHRT_MAX);
	m_ttip.SetBehaviour(PPTOOLTIP_MULTIPLE_SHOW);
	m_ttip.SetNotify(m_hWnd);
	m_ttip.AddTool(&m_ctlServerList, _T(""));

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerWnd::InitFont()
{
	m_pctlLogBox->SetFont(&g_App.m_pMDlg->m_fontDefault);
	m_pctlServerMsgBox->SetFont(&g_App.m_pMDlg->m_fontDefault);
	m_pctlDebugBox->SetFont(&g_App.m_pMDlg->m_fontDefault);
}

void CServerWnd::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SERVLIST, m_ctlServerList);
	DDX_Control(pDX, IDC_NEWSERVER, m_ctrlNewServerBtn);
	DDX_Control(pDX, IDC_UPDATESERVERMET, m_ctrlUpdateServerMetBtn);
	DDX_Control(pDX, IDC_TAB1, m_ctrlBoxSwitcher);
	DDX_Control(pDX, IDC_LOGRESET, m_ctrlResetLogBtn);
}

void CServerWnd::Localize()
{
	if (m_hWnd != NULL)
	{
		TCHAR	acBuf[256];
		TCITEM	tci;

		tci.mask = TCIF_TEXT;
		tci.pszText = acBuf;
		tci.cchTextMax = ARRSIZE(acBuf);

		m_ctrlBoxSwitcher.GetItem(0, &tci);
		_stprintf(acBuf, _T("%s"), GetResString(IDS_SV_LOG));
		m_ctrlBoxSwitcher.SetItem(0, &tci);
		m_ctrlBoxSwitcher.GetItem(1, &tci);
		_stprintf(acBuf, _T("%s"), GetResString(IDS_SV_SERVERINFO));
		m_ctrlBoxSwitcher.SetItem(1, &tci);
		if (g_App.m_pPrefs->GetVerbose() && bDebug)
		{
			m_ctrlBoxSwitcher.GetItem(2, &tci);
			_stprintf(acBuf, _T("%s"), GetResString(IDS_SV_DEBUGLOG));
			m_ctrlBoxSwitcher.SetItem(2, &tci);
		}
		m_pctlLogBox->SetTitle(GetResString(IDS_SV_LOG));
		m_pctlServerMsgBox->SetTitle(GetResString(IDS_SV_SERVERINFO));
		m_pctlDebugBox->SetTitle(GetResString(IDS_SV_DEBUGLOG));
		m_ctrlBoxSwitcher.Invalidate();
		m_pctlLogBox->Invalidate();
		m_pctlDebugBox->Invalidate();
		m_pctlServerMsgBox->Invalidate();
		m_ctrlBoxSwitcher.SetWindowPos(&CWnd::wndBottom,0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);
		m_ctrlNewServerBtn.SetWindowText(GetResString(IDS_SV_NEWSERVER));
		m_ctrlUpdateServerMetBtn.SetWindowText(GetResString(IDS_SV_MET));
		m_ctrlResetLogBtn.SetWindowText(GetResString(IDS_PW_RESET));

		m_ctlServerList.Localize();
	}
}

BEGIN_MESSAGE_MAP(CServerWnd, CResizableDialog)
	ON_BN_CLICKED(IDC_LOGRESET, OnBnClickedResetLog)
	ON_BN_CLICKED(IDC_UPDATESERVERMET, OnBnClickedUpdateservermet)
	ON_BN_CLICKED(IDC_NEWSERVER, OnBnClickedNewserver)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, OnTcnSelchangeTab1)
	ON_NOTIFY(EN_LINK, IDC_SERVERMSGBOX, OnEnLinkServerBox)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CServerWnd message handlers
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CServerWnd::PreTranslateMessage(MSG* pMsg)
{
	if (g_App.m_pPrefs->GetToolTipDelay() != 0)
		m_ttip.RelayEvent(pMsg);

	return CResizableDialog::PreTranslateMessage(pMsg);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CServerWnd::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR* pNMHDR = (NMHDR*)lParam;

	switch(pNMHDR->code)
	{
		case UDM_TOOLTIP_DISPLAY:
		{
			NM_PPTOOLTIP_DISPLAY *pNotify = (NM_PPTOOLTIP_DISPLAY*)lParam;

			GetInfo4ToolTip(pNotify);
			return TRUE;
		}
		case UDM_TOOLTIP_POP:
		{
			m_ttip.Pop();
			return TRUE;
		}
	}

	return CResizableDialog::OnNotify(wParam, lParam, pResult);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerWnd::GetInfo4ToolTip(NM_PPTOOLTIP_DISPLAY *pNotify)
{
	int						iControlId = CWnd::FromHandle(pNotify->ti->hWnd)->GetDlgCtrlID();

	if (iControlId == IDC_SERVLIST)
	{
		if (m_ctlServerList.GetItemCount() < 1)
			return;

		int			iSel = GetItemUnderMouse(&m_ctlServerList);

		if (iSel < 0 || iSel == 65535)
			return;

		CServer		*pServer = (CServer*)m_ctlServerList.GetItemData(iSel);

		if (pServer == NULL)
			return;

		CString					strInfo;

		pNotify->ti->hIcon = pServer->GetServerInfo4Tooltips(strInfo);
		pNotify->ti->sTooltip = strInfo;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerWnd::OnBnClickedResetLog()
{
	switch (m_ctrlBoxSwitcher.GetCurSel())
	{
		case 0:
			m_pctlLogBox->Reset();
			g_App.m_pMDlg->m_ctlStatusBar.SetText(_T(""), SB_MESSAGETEXT, 0);
			break;
		case 1:
			m_pctlServerMsgBox->Reset();
			break;
		case 2:
			m_pctlDebugBox->Reset();
			break;
		default:
			break;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerWnd::OnBnClickedNewserver()
{
	CNewServerDlg	dlgNewServer;
	CRect			rBtn;

	dlgNewServer.SetParent(this);
	GetDlgItem(IDC_NEWSERVER)->GetWindowRect(&rBtn);
	dlgNewServer.SetInitialPos(CPoint(rBtn.right,rBtn.bottom), 1);
	dlgNewServer.SetServerEditMode(false); // We are adding a server, not editing
	dlgNewServer.DoModal();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerWnd::OnBnClickedUpdateservermet()
{
	CUpdateServerMetDlg	dlgUpdateServerMet;
	CRect				rBtn;

	dlgUpdateServerMet.SetParent(this);
	GetDlgItem(IDC_UPDATESERVERMET)->GetWindowRect(&rBtn);
	dlgUpdateServerMet.SetInitialPos(CPoint(rBtn.right,rBtn.bottom), 1);
	dlgUpdateServerMet.DoModal();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerWnd::AddServer(const CString &strAddress, const CString &strPort, const CString &strName, const TCHAR *pcAuxPort/*=_T("")*/, bool bChangeServerInfo/*=false*/)
{
	if (strAddress.IsEmpty())
	{
		AddLogLine(LOG_FL_SBAR, IDS_SRV_ADDR);
		return;
	}

	if (strPort.IsEmpty())
	{
		AddLogLine(LOG_FL_SBAR, IDS_SRV_PORT);
		return;
	}

	CServer			*pNewServer = new CServer(static_cast<uint16>(_tstoi(strPort)), strAddress);

	// Default all manually added servers to high priority
	if (g_App.m_pPrefs->GetManuallyAddedServerHighPrio())
		pNewServer->SetPreference(PR_HIGH);

	if (!strName.IsEmpty())
		pNewServer->SetListName(strName);
	else
		pNewServer->SetListName(strAddress);

	if (*pcAuxPort != _T('\0'))
		pNewServer->SetAuxPort(static_cast<uint16>(_tstoi(pcAuxPort)));

	if (!m_ctlServerList.AddServer(pNewServer, true, false, bChangeServerInfo))
	{
		CServer	   *pServerFound = g_App.m_pServerList->GetServerByAddress(pNewServer->GetAddress(), pNewServer->GetPort());

		if (pServerFound != NULL)
		{
			pServerFound->SetListName(pNewServer->GetListName());
			m_ctlServerList.RefreshServer(*pServerFound);
		}

		AddLogLine(LOG_FL_SBAR, IDS_SRV_NOTADDED);
		delete pNewServer;
	}
	else if (bChangeServerInfo)	// server exists in the list, Edit operation was requested
		delete pNewServer;
	else
		AddLogLine(LOG_FL_SBAR, IDS_SERVERADDED, pNewServer->GetListName());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerWnd::UpdateServerMetFromURL(CString strURL)
{
	m_ctlServerList.SetRedraw(false);

	if (strURL.IsEmpty())
	{
	//	Update from known addresses
		g_App.m_pServerList->AutoUpdate();

	//	Load Metfile (merge existing servers)
		CString		strPath = g_App.m_pPrefs->GetConfigDir() + _T("server.met");

		g_App.m_pServerList->AddServerMetToList(strPath, true);
	}
	else
	{
	//	Is not a valid URL
		if (!strURL.IsEmpty() && strURL.Find(_T("://")) == -1)
		{
			AddLogLine(LOG_FL_SBAR | LOG_RGB_WARNING, IDS_INVALIDURL);
			m_ctlServerList.SetRedraw(true);
			return;
		}

		CString			strTempFilename;

		strTempFilename.Format(_T("%stemp-%d-server.met"), g_App.m_pPrefs->GetConfigDir(), ::GetTickCount());

		CHttpDownloadDlg		dlgDownload;

		dlgDownload.m_strInitializingTitle = GetResString(IDS_HTTP_CAPTION);
		dlgDownload.m_nIDPercentage = IDS_HTTPDOWNLOAD_PERCENTAGE;
		dlgDownload.m_sURLToDownload = strURL;
		dlgDownload.m_sFileToDownloadInto = strTempFilename;

		if (dlgDownload.DoModal() != IDOK)
		{
			AddLogLine(LOG_FL_SBAR | LOG_RGB_WARNING, IDS_ERR_FAILEDDOWNLOADMET, strURL);
			m_ctlServerList.SetRedraw(true);
			return;
		}

	//	Load Metfile (merge existing servers)
		g_App.m_pServerList->AddServerMetToList(strTempFilename, true);

		_tremove(strTempFilename);
	}

//	Insert static servers from textfile
	CString			strPath(g_App.m_pPrefs->GetConfigDir());

	strPath += CFGFILE_STATICSERVERS;
	g_App.m_pServerList->LoadServersFromTextFile(strPath);

	m_ctlServerList.RemoveDeadServer();
	m_ctlServerList.ShowFilesCount();

	m_ctlServerList.SetRedraw(true);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerWnd::OnStnClickedServinfIco()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerWnd::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	NOPRM(pNMHDR);
	switch(m_ctrlBoxSwitcher.GetCurSel())
	{
		case 0:
			m_pctlDebugBox->ShowWindow(SW_HIDE);
			m_pctlServerMsgBox->ShowWindow(SW_HIDE);
			m_pctlLogBox->SetRedraw(false);
			m_pctlLogBox->ShowWindow(SW_SHOW);
			if (((m_pctlLogBox->m_dwFlags & HTC_ISAUTOSCROLL) != 0) && (m_ctrlBoxSwitcher.GetItemState(0, TCIS_HIGHLIGHTED) & TCIS_HIGHLIGHTED))
				m_pctlLogBox->ScrollToLastLine();
			m_pctlLogBox->SetRedraw(true);
			m_ctrlBoxSwitcher.SetItemState(0, TCIS_HIGHLIGHTED, NULL);
			break;
		case 1:
			m_pctlDebugBox->ShowWindow(SW_HIDE);
			m_pctlLogBox->ShowWindow(SW_HIDE);
			m_pctlServerMsgBox->SetRedraw(false);
			m_pctlServerMsgBox->ShowWindow(SW_SHOW);
			if (((m_pctlServerMsgBox->m_dwFlags & HTC_ISAUTOSCROLL) != 0) && (m_ctrlBoxSwitcher.GetItemState(1, TCIS_HIGHLIGHTED) & TCIS_HIGHLIGHTED))
				m_pctlServerMsgBox->ScrollToLastLine();
			m_pctlServerMsgBox->SetRedraw(true);
			m_ctrlBoxSwitcher.SetItemState(1, TCIS_HIGHLIGHTED, NULL);
			break;
		case 2:
			m_pctlServerMsgBox->ShowWindow(SW_HIDE);
			m_pctlLogBox->ShowWindow(SW_HIDE);
			m_pctlDebugBox->SetRedraw(false);
			m_pctlDebugBox->ShowWindow(SW_SHOW);
			if (((m_pctlDebugBox->m_dwFlags & HTC_ISAUTOSCROLL) != 0) && (m_ctrlBoxSwitcher.GetItemState(2, TCIS_HIGHLIGHTED) & TCIS_HIGHLIGHTED))
				m_pctlDebugBox->ScrollToLastLine();
			m_pctlDebugBox->SetRedraw(true);
			m_ctrlBoxSwitcher.SetItemState(2, TCIS_HIGHLIGHTED, NULL);
			break;
		default:
			break;
	}

	*pResult = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerWnd::ToggleDebugWindow()
{
	int			iCurSel = m_ctrlBoxSwitcher.GetCurSel();

	if (g_App.m_pPrefs->GetVerbose() && !bDebug)
	{
		m_ctrlBoxSwitcher.InsertItem(TCIF_IMAGE | TCIF_TEXT, 2, GetResString(IDS_SV_DEBUGLOG), 0, 0, 0, 0);
		bDebug = true;
	}
	else if (!g_App.m_pPrefs->GetVerbose() && bDebug)
	{
		if (iCurSel == 2)
		{
			m_ctrlBoxSwitcher.SetCurSel(0);
			m_ctrlBoxSwitcher.SetFocus();
		}
		m_pctlDebugBox->ShowWindow(SW_HIDE);
		m_pctlServerMsgBox->ShowWindow(SW_HIDE);
		m_pctlLogBox->ShowWindow(SW_SHOW);
		m_ctrlBoxSwitcher.DeleteItem(2);
		bDebug = false;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerWnd::OnCancel()
{
	//CResizableDialog::OnCancel();	// pressing ESC when the focus is on the logbox	would hide the server-wnd [FoRcHa]
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerWnd::OnEnLinkServerBox(NMHDR *pNMHDR, LRESULT *pResult)
{
	ENLINK		*pEnLink = reinterpret_cast<ENLINK *>(pNMHDR);

	if (pEnLink && pEnLink->msg == WM_LBUTTONDOWN)
	{
		CString			strUrl, strLangUrl;

		m_pctlServerMsgBox->GetTextRange(pEnLink->chrg.cpMin, pEnLink->chrg.cpMax, strUrl);
		strLangUrl = strUrl; // GetTextRange returns '?' for non-english chars :(
		strLangUrl.Remove(_T('?'));
		strLangUrl.Remove(_T(' '));
		if (strUrl == GetResString(IDS_EMULEW3) || strLangUrl.IsEmpty())
			strUrl.Format(_T("http://updates.emuleplus.info/version_check.php?version=%i&language=%i"),CURRENT_PLUS_VERSION, g_App.m_pPrefs->GetLanguageID());
		ShellExecute(NULL, NULL, strUrl, NULL, NULL, SW_SHOWDEFAULT);
		*pResult = 1;
	}
	else
		*pResult = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerWnd::OnDestroy()
{
	CResizableDialog::OnDestroy();

	DestroyIcon(GetServerLstStatic()->SetIcon(oldServerListIcon));
	m_imageList.DeleteImageList();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CNewTabCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	EMULE_TRY

	CDC			dc;

	dc.Attach(lpDrawItemStruct->hDC);

	TCHAR	acBuf[255];
	int		iCurItem = lpDrawItemStruct->itemID;
	TCITEM	tcitem;

	tcitem.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_STATE;
	tcitem.pszText = acBuf;
	tcitem.cchTextMax = ARRSIZE(acBuf);
	tcitem.dwStateMask = TCIS_HIGHLIGHTED;
	GetItem(iCurItem, &tcitem);

	CRect		rect = lpDrawItemStruct->rcItem;

	rect.top += ::GetSystemMetrics(SM_CYEDGE);
	rect.left += ::GetSystemMetrics(SM_CXEDGE);
	rect.right -= ::GetSystemMetrics(SM_CXEDGE);

	if (!g_App.m_pMDlg->m_themeHelper.IsAppThemed())
		dc.FillSolidRect(&rect, GetSysColor(COLOR_BTNFACE));
	dc.SetBkMode(TRANSPARENT);

	CImageList	*pImageList = GetImageList();

	if (pImageList != NULL)
		pImageList->Draw(&dc, tcitem.iImage, CPoint((iCurItem == GetCurSel()) ? rect.left + 4 : rect.left + 2, rect.top + 1), ILD_TRANSPARENT);

	dc.SetTextColor((tcitem.dwState & TCIS_HIGHLIGHTED) ? RGB(196, 0, 0) : GetSysColor(COLOR_BTNTEXT));

	rect.OffsetRect(10, 2);
	dc.DrawText(acBuf, -1, rect, DT_CENTER);
	dc.Detach();

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
