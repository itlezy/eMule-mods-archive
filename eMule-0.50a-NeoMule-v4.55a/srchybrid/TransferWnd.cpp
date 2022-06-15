//this file is part of eMule
//Copyright (C)2002-2005 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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
#include "SearchDlg.h"
#include "TransferWnd.h"
#include "OtherFunctions.h"
#include "ClientList.h"
#include "UploadQueue.h"
#include "DownloadQueue.h"
#include "emuledlg.h"
#include "MenuCmds.h"
#include "PartFile.h"
#include "CatDialog.h"
#include "InputBox.h"
#include "UserMsgs.h"
#include "SharedFileList.h"
#include "DropDownButton.h"
#include "ToolTipCtrlX.h"
#include "SharedFilesWnd.h"
#include "HelpIDs.h"
#include "KnownFileList.h" // NEO: NSC - [NeoSharedCategories] <-- Xanatos --
#include "Neo/NeoPreferences.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --
#include "Neo/CP/FilePreferencesDialog.h" // NEO: FCFG - [FileConfiguration] <-- Xanatos --
#include "Neo/Functions.h" // NEO: MOD <-- Xanatos --
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
#include "UpDownClient.h"
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	DFLT_TOOLBAR_BTN_WIDTH	24

#define	WND_SPLITTER_YOFF	10
#define	WND_SPLITTER_HEIGHT	4

#define	WND1_BUTTON_XOFF	8
#define	WND1_BUTTON_WIDTH	170
#define	WND1_BUTTON_HEIGHT	22	// don't set the height to something different than 22 unless you know exactly what you are doing!
#define	WND1_NUM_BUTTONS	6

#define	WND2_BUTTON_XOFF	8
#define	WND2_BUTTON_WIDTH	170
#define	WND2_BUTTON_HEIGHT	22	// don't set the height to something different than 22 unless you know exactly what you are doing!
#define	WND2_NUM_BUTTONS	4

// NEO: NTB - [NeoToolbarButtons] -- Xanatos -->
#define	WNDN_BUTTON_XOFF	8
#define	WNDN_BUTTON_WIDTH	((NeoPrefs.UseNeoToolbar() == TRUE) ? 22 : 44)
#define	WNDN_BUTTON_HEIGHT	22	// don't set the height do something different than 22 unless you know exactly what you are doing!
#define	NUMN_WIN_BUTTONS	6
// NEO: NTB END <-- Xanatos --

// NEO: CCF - [ColloredCategoryFlags] -- Xanatos -->
#define GetAValue(rgb) ((BYTE)(rgb>>24))
#define RGBA(r, g, b, a) ((DWORD)((BYTE)a<<24)|((BYTE)b<<16)|((BYTE)g<<8)|((BYTE)r))
// NEO: CCF END <-- Xanatos --

// CTransferWnd dialog

IMPLEMENT_DYNAMIC(CTransferWnd, CDialog)

BEGIN_MESSAGE_MAP(CTransferWnd, CResizableDialog)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CTLCOLOR()
	ON_WM_SETTINGCHANGE()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_HELPINFO()
	ON_NOTIFY(TBN_DROPDOWN, IDC_DOWNLOAD_ICO, OnWnd1BtnDropDown)
	ON_NOTIFY(TBN_DROPDOWN, IDC_UPLOAD_ICO, OnWnd2BtnDropDown)
	ON_NOTIFY(TBN_DROPDOWN, IDC_NEO_ICO, OnWndNBtnDropDown) // NEO: NTB - [NeoToolbarButtons] <-- Xanatos --
	ON_NOTIFY(UM_SPN_SIZED, IDC_SPLITTER, OnSplitterMoved)
	ON_NOTIFY(LVN_HOTTRACK, IDC_UPLOADLIST, OnHoverUploadList)
	ON_NOTIFY(LVN_HOTTRACK, IDC_QUEUELIST, OnHoverUploadList)
	ON_NOTIFY(LVN_HOTTRACK, IDC_DOWNLOADLIST, OnHoverDownloadList)
	ON_NOTIFY(LVN_HOTTRACK, IDC_CLIENTLIST , OnHoverUploadList)
	ON_NOTIFY(TCN_SELCHANGE, IDC_DLTAB, OnTcnSelchangeDltab)
	ON_NOTIFY(NM_RCLICK, IDC_DLTAB, OnNMRclickDltab)
	ON_NOTIFY(UM_TABMOVED, IDC_DLTAB, OnTabMovement)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_DOWNLOADLIST, OnLvnBegindrag)
	ON_NOTIFY(LVN_KEYDOWN, IDC_DOWNLOADLIST, OnLvnKeydownDownloadlist)
	ON_WM_MEASUREITEM() // NEO: NMX - [NeoMenuXP] <-- Xanatos --
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	ON_NOTIFY_EX_RANGE(UDM_TOOLTIP_DISPLAY, 0, 0xFFFF, OnToolTipNotify)
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
END_MESSAGE_MAP()

CTransferWnd::CTransferWnd(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CTransferWnd::IDD, pParent)
{
	m_uWnd2 = wnd2Uploading;
	m_dwShowListIDC = 0;
	m_pLastMousePoint.x = -1;
	m_pLastMousePoint.y = -1;
	m_nLastCatTT = -1;
	m_btnWnd1 = new CDropDownButton;
	m_btnWnd2 = new CDropDownButton;
	m_btnWndN = new CDropDownButton; // NEO: NTB - [NeoToolbarButtons] <-- Xanatos --
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	lists_list[UPLOAD_WND] = &uploadlistctrl;
	lists_list[QUEUE_WND] = &queuelistctrl;
	lists_list[KNOWN_WND] = &clientlistctrl;
	lists_list[TRANSF_WND] = &downloadclientsctrl;
	lists_list[UPDOWN_WND] = &downloadlistctrl;
	lists_list[DOWNLOAD_WND] = &downloadlistctrl;
#else
	m_tooltipCats = new CToolTipCtrlX;
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
}

CTransferWnd::~CTransferWnd()
{
	// NEO: CCF - [ColloredCategoryFlags] -- Xanatos -->
	DestroyIcon(m_hIconCatTab);
	DestroyIcon(m_hIconCatTabHi);
	DestroyIcon(m_hIconCatTabLo);
	// NEO: CCF END <-- Xanatos --

	delete m_btnWnd1;
	delete m_btnWnd2;
	delete m_btnWndN; // NEO: NTB - [NeoToolbarButtons] <-- Xanatos --
#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
	delete m_tooltipCats;
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

	// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
	VERIFY(m_mnuCatA4AF.DestroyMenu());
	VERIFY(m_mnuCatPriority.DestroyMenu());
	VERIFY(m_mnuCatViewFilter.DestroyMenu());
	VERIFY(m_mnuCategory.DestroyMenu());
	// NEO: NXC END <-- Xanatos --
}

BOOL CTransferWnd::OnInitDialog()
{
	CResizableDialog::OnInitDialog();
	InitWindowStyles(this);

	// NEO: CCF - [ColloredCategoryFlags] -- Xanatos -->
	m_hIconCatTab = theApp.LoadIcon(_T("CATTAB"));
	m_hIconCatTabHi = theApp.LoadIcon(_T("CATTAB_HI"));
	m_hIconCatTabLo = theApp.LoadIcon(_T("CATTAB_LO"));
	// NEO: CCF END <-- Xanatos --

	SetAllIcons(); // NEO: MOD - [SetAllIcons] <-- Xanatos --
	ResetTransToolbar(thePrefs.IsTransToolbarEnabled(), false);
	uploadlistctrl.Init();
	downloadlistctrl.Init();
	queuelistctrl.Init();
	clientlistctrl.Init();
	downloadclientsctrl.Init();

	m_uWnd2 = (EWnd2)thePrefs.GetTransferWnd2();
	ShowWnd2(m_uWnd2);

	AddAnchor(IDC_DOWNLOADLIST, TOP_LEFT, CSize(100, thePrefs.GetSplitterbarPosition()));
	AddAnchor(IDC_UPLOADLIST, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
	AddAnchor(IDC_QUEUELIST, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
	AddAnchor(IDC_CLIENTLIST, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
	AddAnchor(IDC_DOWNLOADCLIENTS, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
	AddAnchor(IDC_QUEUECOUNT, BOTTOM_LEFT);
	AddAnchor(IDC_QUEUECOUNT_LABEL, BOTTOM_LEFT);
	AddAnchor(IDC_QUEUE_REFRESH_BUTTON, BOTTOM_RIGHT);
	AddAnchor(IDC_DLTAB, CSize(50, 0), TOP_RIGHT);

	switch (thePrefs.GetTransferWnd1()) {
		default:
		case 0: {
			// splitting functionality
			CRect rc;
			GetWindowRect(rc);
			ScreenToClient(rc);
			CRect rcBtn2;
			m_btnWnd2->GetWindowRect(rcBtn2);
			ScreenToClient(rcBtn2);
			CRect rcSpl;
			rcSpl.left = rcBtn2.right + 8;
			rcSpl.right = rc.right;
			rcSpl.top = rc.bottom - 100;
			rcSpl.bottom = rcSpl.top + WND_SPLITTER_HEIGHT;
			m_wndSplitter.Create(WS_CHILD | WS_VISIBLE, rcSpl, this, IDC_SPLITTER);
			m_wndSplitter.SetDrawBorder(true);
			ShowSplitWindow();
			break;
		}
		case 1:
			ShowList(IDC_DOWNLOADLIST);
			break;
		case 2:
			ShowList(IDC_UPLOADLIST);
			break;
		case 3:
			ShowList(IDC_QUEUELIST);
			break;
		case 4:
			ShowList(IDC_DOWNLOADCLIENTS);
			break;
		case 5:
			ShowList(IDC_CLIENTLIST);
			break;
	}

	//cats
	rightclickindex=-1;

	CreateCategoryMenus(); // NEO: NXC - [NewExtendedCategories] <-- Xanatos --

	downloadlistactive=true;
	m_bIsDragging=false;

	// show & cat-tabs
	//m_dlTab.ModifyStyle(0, TCS_OWNERDRAWFIXED); // NEO: CCF - [ColloredCategoryFlags] <-- Xanatos --
	m_dlTab.SetPadding(CSize(6, 4));
	if (theApp.IsVistaThemeActive())
		m_dlTab.ModifyStyle(0, WS_CLIPCHILDREN);
	// NEO: NXC - [NewExtendedCategories] -- Xanatos --
	//thePrefs.GetCategory(0)->strTitle = GetCatTitle(thePrefs.GetCategory(0)->filter);
	//thePrefs.GetCategory(0)->strIncomingPath = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
	//thePrefs.GetCategory(0)->care4all = true;

	for (int ix=0;ix<thePrefs.GetCatCount();ix++){
		// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
		Category_Struct* curCat = thePrefs.GetCategory(ix);
		/*if (ix == 0 && curCat->viewfilters.nFromCats == 2)
			curCat->viewfilters.nFromCats = 0;
		else*/ if (curCat->viewfilters.nFromCats != 2 && ix != 0 && theApp.downloadqueue->GetCategoryFileCount(ix) != 0)
			curCat->viewfilters.nFromCats = 2;
		// NEO: NXC END <-- Xanatos --
		m_dlTab.InsertItem(ix,thePrefs.GetCategory(ix)->strTitle);
	}

	// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
	m_dlTab.SetCurSel(thePrefs.GetLastCategory());
	downloadlistctrl.ChangeCategory(thePrefs.GetLastCategory());
	// NEO: NXC END <-- Xanatos --

	// create tooltip control for download categories
#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
	m_tooltipCats->Create(this, TTS_NOPREFIX);
	m_dlTab.SetToolTips(m_tooltipCats);
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
	UpdateCatTabTitles();
	UpdateTabToolTips();
	UpdateCatTabIcons(); // NEO: CCF - [ColloredCategoryFlags] <-- Xanatos --

#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	// download tabs tooltips
	m_tabtip.Create(this, TTS_NOPREFIX);
	m_tabtip.SetDirection(CPPToolTip::PPTOOLTIP_RIGHT_BOTTOM);

	m_tabtip.AddTool(&m_dlTab, _T(""));

	// listctrls tooltips
	m_ttip.Create(this);

	m_ttip.AddTool(&downloadlistctrl, _T(""));
	m_ttip.AddTool(&uploadlistctrl, _T(""));
	m_ttip.AddTool(&queuelistctrl, _T(""));
	m_ttip.AddTool(&clientlistctrl, _T(""));
	m_ttip.AddTool(&downloadclientsctrl, _T(""));


	for(int i = 0; i < ARRSIZE(m_iOldToolTipItem); i++)
		m_iOldToolTipItem[i] = -1;

	//m_othertips.Create(this);

	SetTTDelay();
#else
	m_tooltipCats->SetMargin(CRect(4, 4, 4, 4));
	m_tooltipCats->SendMessage(TTM_SETMAXTIPWIDTH, 0, SHRT_MAX); // recognize \n chars!
	m_tooltipCats->SetDelayTime(TTDT_AUTOPOP, 20000);
	m_tooltipCats->SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1000);
	m_tooltipCats->Activate(TRUE);
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

	VerifyCatTabSize();
    Localize();

	return true;
}

void CTransferWnd::ShowQueueCount(uint32 number)
{
	TCHAR buffer[100];
	_sntprintf(buffer, _countof(buffer), _T("%u (%u ") + GetResString(IDS_BANNED).MakeLower() + _T(")"), number, theApp.clientlist->GetBannedCount());
	buffer[_countof(buffer) - 1] = _T('\0');
	GetDlgItem(IDC_QUEUECOUNT)->SetWindowText(buffer);
}

void CTransferWnd::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DOWNLOAD_ICO, *m_btnWnd1);
	DDX_Control(pDX, IDC_UPLOAD_ICO, *m_btnWnd2);
	DDX_Control(pDX, IDC_NEO_ICO, *m_btnWndN); // NEO: NTB - [NeoToolbarButtons] <-- Xanatos --
	DDX_Control(pDX, IDC_DLTAB, m_dlTab);
	DDX_Control(pDX, IDC_UPLOADLIST, uploadlistctrl);
	DDX_Control(pDX, IDC_DOWNLOADLIST, downloadlistctrl);
	DDX_Control(pDX, IDC_QUEUELIST, queuelistctrl);
	DDX_Control(pDX, IDC_CLIENTLIST, clientlistctrl);
	DDX_Control(pDX, IDC_DOWNLOADCLIENTS, downloadclientsctrl);
}

void CTransferWnd::DoResize(int delta)
{
	CSplitterControl::ChangeHeight(&downloadlistctrl, delta);
	CSplitterControl::ChangeHeight(&uploadlistctrl, -delta, CW_BOTTOMALIGN);
	CSplitterControl::ChangeHeight(&queuelistctrl, -delta, CW_BOTTOMALIGN);
	CSplitterControl::ChangeHeight(&clientlistctrl, -delta, CW_BOTTOMALIGN);
	CSplitterControl::ChangeHeight(&downloadclientsctrl, -delta, CW_BOTTOMALIGN);

	UpdateSplitterRange();

	if (m_dwShowListIDC == IDC_DOWNLOADLIST + IDC_UPLOADLIST)
	{
		downloadlistctrl.Invalidate();
		downloadlistctrl.UpdateWindow();
		switch (m_uWnd2)
		{
		case wnd2Uploading:
			uploadlistctrl.Invalidate();
			uploadlistctrl.UpdateWindow();
			break;
		case wnd2OnQueue:
			queuelistctrl.Invalidate();
			queuelistctrl.UpdateWindow();
			break;
		case wnd2Clients:
			clientlistctrl.Invalidate();
			clientlistctrl.UpdateWindow();
			break;
		case wnd2Downloading:
			downloadclientsctrl.Invalidate();
			downloadclientsctrl.UpdateWindow();
			break;
		default:
			ASSERT(0);
		}
	}
}

void CTransferWnd::UpdateSplitterRange()
{
	CRect rcWnd;
	GetWindowRect(rcWnd);
	if (rcWnd.Height() == 0){
		ASSERT( false );
		return;
	}

	CRect rcDown;
	downloadlistctrl.GetWindowRect(rcDown);
	ScreenToClient(rcDown);

	CRect rcUp;
	downloadclientsctrl.GetWindowRect(rcUp);
	ScreenToClient(rcUp);

	thePrefs.SetSplitterbarPosition((rcDown.bottom * 100) / rcWnd.Height());

	RemoveAnchor(IDC_DOWNLOADLIST);
	RemoveAnchor(IDC_UPLOADLIST);
	RemoveAnchor(IDC_QUEUELIST);
	RemoveAnchor(IDC_CLIENTLIST);
	RemoveAnchor(IDC_DOWNLOADCLIENTS);

	AddAnchor(IDC_DOWNLOADLIST, TOP_LEFT, CSize(100, thePrefs.GetSplitterbarPosition()));
	AddAnchor(IDC_UPLOADLIST, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
	AddAnchor(IDC_QUEUELIST, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
	AddAnchor(IDC_CLIENTLIST, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
	AddAnchor(IDC_DOWNLOADCLIENTS, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);

	m_wndSplitter.SetRange(rcDown.top + 50, rcUp.bottom - 40);
}

LRESULT CTransferWnd::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
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
					CRect rcDown;
					downloadlistctrl.GetWindowRect(rcDown);
					ScreenToClient(rcDown);

					CRect rcBtn2;
					m_btnWnd2->GetWindowRect(rcBtn2);
					ScreenToClient(rcBtn2);

					// splitter paint update
					CRect rcSpl;
					rcSpl.left = rcBtn2.right + 8;
					rcSpl.right = rcDown.right;
					rcSpl.top = rcDown.bottom + WND_SPLITTER_YOFF;
					rcSpl.bottom = rcSpl.top + WND_SPLITTER_HEIGHT;
					m_wndSplitter.MoveWindow(rcSpl, TRUE);
					m_btnWnd2->MoveWindow(WND2_BUTTON_XOFF, rcSpl.top - (WND_SPLITTER_YOFF - 1), rcBtn2.Width(), WND2_BUTTON_HEIGHT);
					UpdateSplitterRange();
				}
			}

			// Workaround to solve a glitch with WM_SETTINGCHANGE message
			if (m_btnWnd1 && m_btnWnd1->m_hWnd && m_btnWnd1->GetBtnWidth(IDC_DOWNLOAD_ICO) != WND1_BUTTON_WIDTH)
				m_btnWnd1->SetBtnWidth(IDC_DOWNLOAD_ICO, WND1_BUTTON_WIDTH);
			if (m_btnWnd2 && m_btnWnd2->m_hWnd && m_btnWnd2->GetBtnWidth(IDC_UPLOAD_ICO) != WND2_BUTTON_WIDTH)
				m_btnWnd2->SetBtnWidth(IDC_UPLOAD_ICO, WND2_BUTTON_WIDTH);
			// NEO: NTB - [NeoToolbarButtons] -- Xanatos -->
			if (NeoPrefs.UseNeoToolbar() && m_btnWndN && m_btnWndN->m_hWnd && m_btnWndN->GetBtnWidth(IDC_NEO_ICO) != WNDN_BUTTON_WIDTH)
				m_btnWndN->SetBtnWidth(IDC_NEO_ICO, WNDN_BUTTON_WIDTH);
			// NEO: NTB END <-- Xanatos --
			break;
	}

	return CResizableDialog::DefWindowProc(message, wParam, lParam);
}

void CTransferWnd::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	if (m_wndSplitter)
	{
		CRect rcWnd;
		GetWindowRect(rcWnd);
		if (rcWnd.Height() > 0)
			Invalidate();
	}
	CResizableDialog::OnWindowPosChanged(lpwndpos);
}

void CTransferWnd::OnSplitterMoved(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	SPC_NMHDR* pHdr = (SPC_NMHDR*)pNMHDR;
	DoResize(pHdr->delta);
}

BOOL CTransferWnd::PreTranslateMessage(MSG* pMsg)
{
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	m_ttip.RelayEvent(pMsg);
	m_tabtip.RelayEvent(pMsg);
	//m_othertips.RelayEvent(pMsg);
	m_btttp.RelayEvent(pMsg);
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

	if (pMsg->message == WM_KEYDOWN)
	{
		// Don't handle Ctrl+Tab in this window. It will be handled by main window.
		if (pMsg->wParam == VK_TAB && GetAsyncKeyState(VK_CONTROL) < 0)
			return FALSE;
	}
	else if (pMsg->message == WM_LBUTTONDBLCLK)
	{
		if (pMsg->hwnd == m_dlTab.m_hWnd)
		{
			OnDblclickDltab();
			return TRUE;
		}
	}
	else if (pMsg->message == WM_MOUSEMOVE)
	{
		POINT point;
		::GetCursorPos(&point);
		if (point.x != m_pLastMousePoint.x || point.y != m_pLastMousePoint.y)
		{
			m_pLastMousePoint = point;
			// handle tooltip updating, when mouse is moved from one item to another
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
			UpdateToolTips();
			UpdateTabToolTips();
#else
			CPoint pt(point);
			m_nDropIndex = GetTabUnderMouse(&pt);
			if (m_nDropIndex != m_nLastCatTT)
			{
				m_nLastCatTT = m_nDropIndex;
				if (m_nDropIndex != -1)
					UpdateTabToolTips(m_nDropIndex);
			}
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
		}
	}
	else if (!thePrefs.GetStraightWindowStyles() && pMsg->message == WM_MBUTTONUP)
	{
		if (downloadlistactive)
			downloadlistctrl.ShowSelectedFileDetails();
		else if (m_dwShowListIDC != IDC_DOWNLOADLIST + IDC_UPLOADLIST)
		{
			switch (m_dwShowListIDC)
			{
				case IDC_UPLOADLIST:
					uploadlistctrl.ShowSelectedUserDetails();
					break;
				case IDC_QUEUELIST:
					queuelistctrl.ShowSelectedUserDetails();
					break;
				case IDC_CLIENTLIST:
					clientlistctrl.ShowSelectedUserDetails();
					break;
				case IDC_DOWNLOADCLIENTS:
					downloadclientsctrl.ShowSelectedUserDetails();
					break;
				default:
					ASSERT(0);
			}
		}
		else
		{
			switch (m_uWnd2)
			{
				case wnd2OnQueue:
					queuelistctrl.ShowSelectedUserDetails();
					break;
				case wnd2Uploading:
					uploadlistctrl.ShowSelectedUserDetails();
					break;
				case wnd2Clients:
					clientlistctrl.ShowSelectedUserDetails();
					break;
				case wnd2Downloading:
					downloadclientsctrl.ShowSelectedUserDetails();
					break;
				default:
					ASSERT(0);
			}
		}
		return TRUE;
	}

	return CResizableDialog::PreTranslateMessage(pMsg);
}

#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos 
int CTransferWnd::GetItemUnderMouse(CListCtrl* ctrl)
{
	CPoint pt;
	::GetCursorPos(&pt);
	ctrl->ScreenToClient(&pt);
	LVHITTESTINFO hit, subhit;
	hit.pt = pt;
	subhit.pt = pt;
	ctrl->SubItemHitTest(&subhit);
	int sel = ctrl->HitTest(&hit);
	if (sel != LB_ERR && (hit.flags & LVHT_ONITEM))
	{
		if (subhit.iSubItem == 0)
			return sel;
	}
	return LB_ERR;
}
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

//void CTransferWnd::UpdateFilesCount(int iCount)
void CTransferWnd::UpdateFilesCount(int iCount, float Percentage) // NEO: MOD - [Percentage] <-- Xanatos --
{
	if (m_dwShowListIDC == IDC_DOWNLOADLIST || m_dwShowListIDC == IDC_DOWNLOADLIST + IDC_UPLOADLIST)
	{
		CString strBuffer;
		strBuffer.Format(_T("%s (%u)"), GetResString(IDS_TW_DOWNLOADS), iCount);
		// NEO: MOD - [Percentage] -- Xanatos -->
		if (Percentage != 0.0f)
			strBuffer.AppendFormat(_T(" %.1f%%"),Percentage);
		// NEO: MOD END <-- Xanatos --
		m_btnWnd1->SetWindowText(strBuffer);
	}
}

void CTransferWnd::UpdateListCount(EWnd2 listindex, int iCount /*=-1*/)
{
	switch (m_dwShowListIDC)
	{
		case IDC_DOWNLOADLIST + IDC_UPLOADLIST: {
			if (m_uWnd2 != listindex)
				return;
			CString strBuffer;
			switch (m_uWnd2)
			{
				case wnd2Uploading: {
					uint32 itemCount = iCount == -1 ? uploadlistctrl.GetItemCount() : iCount;
					uint32 activeCount = theApp.uploadqueue->GetActiveUploadsCount();
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
					uint32 lanCount = theApp.uploadqueue->GetLanUploadsCount();
					itemCount -= lanCount;
#endif //LANCAST // NEO: NLC END <-- Xanatos --
					if (activeCount >= itemCount)
						strBuffer.Format(_T(" (%i)"), itemCount);
					else
						strBuffer.Format(_T(" (%i/%i)"), activeCount, itemCount);
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
					if(lanCount)
						strBuffer.AppendFormat(_T(" [%i]"),lanCount);
#endif //LANCAST // NEO: NLC END <-- Xanatos --
					m_btnWnd2->SetWindowText(GetResString(IDS_UPLOADING) + strBuffer);
					break;
				}
				case wnd2OnQueue:
					strBuffer.Format(_T(" (%i)"), iCount == -1 ? queuelistctrl.GetItemCount() : iCount);
					m_btnWnd2->SetWindowText(GetResString(IDS_ONQUEUE) + strBuffer);
					break;
				case wnd2Clients:
					strBuffer.Format(_T(" (%i)"), iCount == -1 ? clientlistctrl.GetItemCount() : iCount);
					m_btnWnd2->SetWindowText(GetResString(IDS_CLIENTLIST) + strBuffer);
					break;
				case wnd2Downloading:
					strBuffer.Format(_T(" (%i)"), iCount == -1 ? downloadclientsctrl.GetItemCount() : iCount);
					m_btnWnd2->SetWindowText(GetResString(IDS_DOWNLOADING) + strBuffer);
					break;
				default:
					ASSERT(0);
			}
			break;
		}

		case IDC_DOWNLOADLIST:
			break;

		case IDC_UPLOADLIST:
			if (listindex == wnd2Uploading)
			{
				CString strBuffer;
				uint32 itemCount = iCount == -1 ? uploadlistctrl.GetItemCount() : iCount;
				uint32 activeCount = theApp.uploadqueue->GetActiveUploadsCount();
				if (activeCount >= itemCount)
					strBuffer.Format(_T(" (%i)"), itemCount);
				else
					strBuffer.Format(_T(" (%i/%i)"), activeCount, itemCount);
				m_btnWnd1->SetWindowText(GetResString(IDS_UPLOADING) + strBuffer);
			}
			break;
		
		case IDC_QUEUELIST:
			if (listindex == wnd2OnQueue)
			{
				CString strBuffer;
				strBuffer.Format(_T(" (%i)"), iCount == -1 ? queuelistctrl.GetItemCount() : iCount);
				m_btnWnd1->SetWindowText(GetResString(IDS_ONQUEUE) + strBuffer);
			}
			break;
		
		case IDC_CLIENTLIST:
			if (listindex == wnd2Clients)
			{
				CString strBuffer;
				strBuffer.Format(_T(" (%i)"), iCount == -1 ? clientlistctrl.GetItemCount() : iCount);
				m_btnWnd1->SetWindowText(GetResString(IDS_CLIENTLIST) + strBuffer);
			}
			break;

		case IDC_DOWNLOADCLIENTS:
			if (listindex == wnd2Downloading)
			{
				CString strBuffer;
				strBuffer.Format(_T(" (%i)"), iCount == -1 ? downloadclientsctrl.GetItemCount() : iCount);
				m_btnWnd1->SetWindowText(GetResString(IDS_DOWNLOADING) + strBuffer);
			}
			break;

		default:
			//ASSERT(0);
			;
	}
}

void CTransferWnd::SwitchUploadList()
{
	if (m_uWnd2 == wnd2Uploading)
	{
		SetWnd2(wnd2Downloading);
		queuelistctrl.Hide();
		clientlistctrl.Hide();
		uploadlistctrl.Hide();
		downloadclientsctrl.Show();
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
		m_btnWnd2->CheckButton(MP_VIEW2_DOWNLOADING);
		SetWnd2Icon(w2iDownloading);
	}
	else if (m_uWnd2 == wnd2Downloading)
	{
		SetWnd2(wnd2OnQueue);
		if (thePrefs.IsQueueListDisabled()){
			SwitchUploadList();
			return;
		}
		uploadlistctrl.Hide();
		clientlistctrl.Hide();
		downloadclientsctrl.Hide();
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_SHOW);
		queuelistctrl.Visable();
		m_btnWnd2->CheckButton(MP_VIEW2_ONQUEUE);
		SetWnd2Icon(w2iOnQueue);
	}
	else if (m_uWnd2 == wnd2OnQueue)
	{
		SetWnd2(wnd2Clients);
		if (thePrefs.IsKnownClientListDisabled()){
			SwitchUploadList();
			return;
		}
		uploadlistctrl.Hide();
		queuelistctrl.Hide();
		downloadclientsctrl.Hide();
		clientlistctrl.Visable();
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
		m_btnWnd2->CheckButton(MP_VIEW2_CLIENTS);
		SetWnd2Icon(w2iClientsKnown);
	}
	else
	{
		SetWnd2(wnd2Uploading);
		queuelistctrl.Hide();
		clientlistctrl.Hide();
		downloadclientsctrl.Hide();
		uploadlistctrl.Visable();
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
		m_btnWnd2->CheckButton(MP_VIEW2_UPLOADING);
		SetWnd2Icon(w2iUploading);
	}
	UpdateListCount(m_uWnd2);
}

void CTransferWnd::ShowWnd2(EWnd2 uWnd2)
{
	if (uWnd2 == wnd2Downloading) 
	{
		SetWnd2(uWnd2);
		queuelistctrl.Hide();
		clientlistctrl.Hide();
		uploadlistctrl.Hide();
		downloadclientsctrl.Show();
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
		m_btnWnd2->CheckButton(MP_VIEW2_DOWNLOADING);
		SetWnd2Icon(w2iDownloading);
	}
	else if (uWnd2 == wnd2OnQueue && !thePrefs.IsQueueListDisabled())
	{
		SetWnd2(uWnd2);
		uploadlistctrl.Hide();
		clientlistctrl.Hide();
		downloadclientsctrl.Hide();
		queuelistctrl.Visable();
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_SHOW);
		m_btnWnd2->CheckButton(MP_VIEW2_ONQUEUE);
		SetWnd2Icon(w2iOnQueue);
	}
	else if (uWnd2 == wnd2Clients && !thePrefs.IsKnownClientListDisabled())
	{
		SetWnd2(uWnd2);
		uploadlistctrl.Hide();
		queuelistctrl.Hide();
		downloadclientsctrl.Hide();
		clientlistctrl.Visable();
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
		m_btnWnd2->CheckButton(MP_VIEW2_CLIENTS);
		SetWnd2Icon(w2iClientsKnown);
	}
	else
	{
		SetWnd2(wnd2Uploading);
		queuelistctrl.Hide();
		clientlistctrl.Hide();
		downloadclientsctrl.Hide();
		uploadlistctrl.Visable();
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
		m_btnWnd2->CheckButton(MP_VIEW2_UPLOADING);
		SetWnd2Icon(w2iUploading);
	}
	UpdateListCount(m_uWnd2);
}

void CTransferWnd::SetWnd2(EWnd2 uWnd2)
{
	m_uWnd2 = uWnd2;
	thePrefs.SetTransferWnd2(m_uWnd2);
}

void CTransferWnd::OnSysColorChange()
{
	CResizableDialog::OnSysColorChange();
	SetAllIcons();
	m_btnWnd1->Invalidate();
	m_btnWnd2->Invalidate();
	m_btnWndN->Invalidate(); // NEO: NTB - [NeoToolbarButtons] <-- Xanatos --
}

void CTransferWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CResizableDialog::OnSettingChange(uFlags, lpszSection);
	// It does not work to reset the width of 1st button here.
	//m_btnWnd1->SetBtnWidth(IDC_DOWNLOAD_ICO, WND1_BUTTON_WIDTH);
	//m_btnWnd2->SetBtnWidth(IDC_UPLOAD_ICO, WND2_BUTTON_WIDTH);
}

#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
enum EnumDownloadListIcons
{
	DL_BLANK = 0,

	DL_SRC_DOWNLOADING,
	DL_SRC_ON_QUEUE,
	DL_SRC_CONNECTING,
	DL_SRC_NNPFQ,
	DL_SRC_UNKNOWN,

	DL_CLIENT_CDONKEY,
	DL_CLIENT_EDONKEY,
	DL_CLIENT_EMULE,
	DL_CLIENT_HYBRID,
	DL_CLIENT_MLDONKEY,
	DL_CLIENT_OLDEMULE,
	DL_CLIENT_SHAREAZA,
	DL_CLIENT_UNKNOWN,
	DL_CLIENT_XMULE,
	DL_CLIENT_AMULE,
	DL_CLIENT_LPHANT,
	DL_CLIENT_EMULEPLUS,
	DL_CLIENT_TRUSTYFILES,
	DL_CLIENT_HYDRANODE,
	DL_SERVER,

	DL_CLIENT_MOD,
	DL_CLIENT_MOD_NEO,
	DL_CLIENT_MOD_MORPH,
	DL_CLIENT_MOD_SCARANGEL,
	DL_CLIENT_MOD_STULLE,
	DL_CLIENT_MOD_MAXMOD,
	DL_CLIENT_MOD_XTREME,
	DL_CLIENT_MOD_EASTSHARE,
	DL_CLIENT_MOD_IONIX,
	DL_CLIENT_MOD_CYREX,
	DL_CLIENT_MOD_NEXTEMF,
};
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

void CTransferWnd::SetAllIcons()
{
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	m_ImageList.DeleteImageList();
	m_ImageList.Create(32,32,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	m_ImageList.SetBkColor(CLR_NONE);

	m_ImageList.Add(CTempIconLoader(_T("EMPTY")));
	m_ImageList.Add(CTempIconLoader(_T("SrcDownloading")));
	m_ImageList.Add(CTempIconLoader(_T("SrcOnQueue")));
	m_ImageList.Add(CTempIconLoader(_T("SrcConnecting")));
	m_ImageList.Add(CTempIconLoader(_T("SrcNNPQF")));
	m_ImageList.Add(CTempIconLoader(_T("SrcUnknown")));

	m_ImageList.Add(CTempIconLoader(_T("CLIENT_CDONKEY")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_EDONKEY")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_EMULE")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_HYBRID")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_MLDONKEY")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_OLDEMULE")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_SHAREAZA")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_UNKNOWN")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_XMULE")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_AMULE")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_LPHANT")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_EMULEPLUS")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_TRUSTYFILES")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_HYDRANODE")));
	m_ImageList.Add(CTempIconLoader(_T("SERVER")));

	m_ImageList.Add(CTempIconLoader(_T("CLIENT_MOD")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_NEO")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_MORPH")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_SCARANGEL")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_STULLE")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_MAXMOD")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_XTREME")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_EASTSHARE")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_IONIX")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_CYREX")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_NEXTEMF")));
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

	SetWnd1Icons();
	SetWnd2Icons();
	SetWndNIcons(); // NEO: NTB - [NeoToolbarButtons] <-- Xanatos --
}

void CTransferWnd::SetWnd1Icons()
{
	CImageList iml;
	iml.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 1, 1);
	iml.Add(CTempIconLoader(_T("SplitWindow")));
	iml.Add(CTempIconLoader(_T("DownloadFiles")));
	iml.Add(CTempIconLoader(_T("Upload")));
	iml.Add(CTempIconLoader(_T("Download")));
	iml.Add(CTempIconLoader(_T("ClientsOnQueue")));
	iml.Add(CTempIconLoader(_T("ClientsKnown")));
	CImageList* pImlOld = m_btnWnd1->SetImageList(&iml);
	iml.Detach();
	if (pImlOld)
		pImlOld->DeleteImageList();
}

void CTransferWnd::SetWnd2Icons()
{
	CImageList iml;
	iml.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 1, 1);
	iml.Add(CTempIconLoader(_T("Upload")));
	iml.Add(CTempIconLoader(_T("Download")));
	iml.Add(CTempIconLoader(_T("ClientsOnQueue")));
	iml.Add(CTempIconLoader(_T("ClientsKnown")));
	CImageList* pImlOld = m_btnWnd2->SetImageList(&iml);
	iml.Detach();
	if (pImlOld)
		pImlOld->DeleteImageList();
}

// NEO: NTB - [NeoToolbarButtons] -- Xanatos -->
CString GetNeoCmdIco(UINT Command);
void CTransferWnd::SetWndNIcons()
{
	CImageList imlN;
	imlN.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 1, 1);

	if(NeoPrefs.UseNeoToolbar() == 2)
		imlN.Add(CTempIconLoader(_T("NEO_BTN")));
	for (int i=0; i<NeoPrefs.GetNeoToolbarButtonCount(); i++)
		imlN.Add(CTempIconLoader(GetNeoCmdIco(NeoPrefs.GetNeoToolbarButton(i))));
	if(NeoPrefs.UseNeoToolbar() == 1)
		imlN.Add(CTempIconLoader(_T("NEO_BTN")));
	CImageList* pImlOld = m_btnWndN->SetImageList(&imlN);
	imlN.Detach();
	if (pImlOld)
		pImlOld->DeleteImageList();
}
// NEO: NTB END <-- Xanatos --

void CTransferWnd::Localize()
{
	LocalizeToolbars();

	GetDlgItem(IDC_QUEUECOUNT_LABEL)->SetWindowText(GetResString(IDS_TW_QUEUE));
	GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->SetWindowText(GetResString(IDS_SV_UPDATE));

	m_btnWndN->SetWindowText(GetResString(IDS_X_TW_NEO)); // NEO: NTB - [NeoToolbarButtons] <-- Xanatos --

	CreateCategoryMenus(); // NEO: NXC - [NewExtendedCategories] <-- Xanatos --

	uploadlistctrl.Localize();
	queuelistctrl.Localize();
	downloadlistctrl.Localize();
	clientlistctrl.Localize();
	downloadclientsctrl.Localize();

	if (m_dwShowListIDC == IDC_DOWNLOADLIST + IDC_UPLOADLIST)
		ShowSplitWindow();
	else
		ShowList(m_dwShowListIDC);
	UpdateListCount(m_uWnd2);
}

void CTransferWnd::LocalizeToolbars()
{
	m_btnWnd1->SetWindowText(GetResString(IDS_TW_DOWNLOADS));
	m_btnWnd1->SetBtnText(MP_VIEW1_SPLIT_WINDOW, GetResString(IDS_SPLIT_WINDOW));
	m_btnWnd1->SetBtnText(MP_VIEW1_DOWNLOADS, GetResString(IDS_TW_DOWNLOADS));
	m_btnWnd1->SetBtnText(MP_VIEW1_UPLOADING, GetResString(IDS_UPLOADING));
	m_btnWnd1->SetBtnText(MP_VIEW1_DOWNLOADING, GetResString(IDS_DOWNLOADING));
	m_btnWnd1->SetBtnText(MP_VIEW1_ONQUEUE, GetResString(IDS_ONQUEUE));
	m_btnWnd1->SetBtnText(MP_VIEW1_CLIENTS, GetResString(IDS_CLIENTLIST));
	m_btnWnd2->SetWindowText(GetResString(IDS_UPLOADING));
	m_btnWnd2->SetBtnText(MP_VIEW2_UPLOADING, GetResString(IDS_UPLOADING));
	m_btnWnd2->SetBtnText(MP_VIEW2_DOWNLOADING, GetResString(IDS_DOWNLOADING));
	m_btnWnd2->SetBtnText(MP_VIEW2_ONQUEUE, GetResString(IDS_ONQUEUE));
	m_btnWnd2->SetBtnText(MP_VIEW2_CLIENTS, GetResString(IDS_CLIENTLIST));
}

void CTransferWnd::OnBnClickedQueueRefreshButton()
{
	CUpDownClient* update = theApp.uploadqueue->GetNextClient(NULL);

	while( update ){
		theApp.emuledlg->transferwnd->queuelistctrl.RefreshClient( update);
		update = theApp.uploadqueue->GetNextClient(update);
	}
}

void CTransferWnd::OnHoverUploadList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	downloadlistactive = false;
	*pResult = 0;
}

void CTransferWnd::OnHoverDownloadList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	downloadlistactive = true;
	*pResult = 0;
}

void CTransferWnd::OnTcnSelchangeDltab(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	thePrefs.SetLastCategory(m_dlTab.GetCurSel()); // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
	downloadlistctrl.ChangeCategory(m_dlTab.GetCurSel());
	*pResult = 0;
}

// Ornis' download categories
void CTransferWnd::OnNMRclickDltab(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	POINT point;
	::GetCursorPos(&point);
	CPoint pt(point);
	rightclickindex = GetTabUnderMouse(&pt);
	if (rightclickindex == -1)
		return;

	// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
	// If the current category is '0'...  Well, we can't very well delete the default category, now can we...
	// Nor can we merge it.
	m_mnuCategory.EnableMenuItem(MP_CAT_REMOVE, rightclickindex == 0 ? MF_GRAYED : MF_ENABLED);
	//m_mnuCategory.EnableMenuItem(MP_CAT_MERGE, rightclickindex == 0 ? MF_GRAYED : MF_ENABLED);
	m_mnuCategory.EnableMenuItem(8, (NeoPrefs.AdvancedA4AFMode() ? MF_ENABLED : MF_GRAYED) | MF_BYPOSITION);

	Category_Struct* curCat = thePrefs.GetCategory(rightclickindex);
	if (curCat) {
		// Check and enable the appropriate menu items in Select View Filter
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0, (curCat->viewfilters.nFromCats == 0) ? MF_CHECKED : MF_UNCHECKED);
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+1, (curCat->viewfilters.nFromCats == 1) ? MF_CHECKED : MF_UNCHECKED); // Lit Cat Filter
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+2, (curCat->viewfilters.nFromCats == 2) ? MF_CHECKED : MF_UNCHECKED);

		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+3, (curCat->viewfilters.bComplete) ? MF_CHECKED : MF_UNCHECKED);
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+4, (curCat->viewfilters.bCompleting) ? MF_CHECKED : MF_UNCHECKED);
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+5, (curCat->viewfilters.bTransferring) ? MF_CHECKED : MF_UNCHECKED);
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+6, (curCat->viewfilters.bWaiting) ? MF_CHECKED : MF_UNCHECKED);
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+7, (curCat->viewfilters.bPaused) ? MF_CHECKED : MF_UNCHECKED);
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+8, (curCat->viewfilters.bStopped) ? MF_CHECKED : MF_UNCHECKED);
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+19, (curCat->viewfilters.bStandby) ? MF_CHECKED : MF_UNCHECKED); // NEO: SD - [StandByDL]
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+20, (curCat->viewfilters.bSuspend) ? MF_CHECKED : MF_UNCHECKED); // NEO: SC - [SuspendCollecting]
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+9, (curCat->viewfilters.bHashing) ? MF_CHECKED : MF_UNCHECKED);
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+10, (curCat->viewfilters.bErrorUnknown) ? MF_CHECKED : MF_UNCHECKED);

		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+11, (curCat->viewfilters.bVideo) ? MF_CHECKED : MF_UNCHECKED);
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+12, (curCat->viewfilters.bAudio) ? MF_CHECKED : MF_UNCHECKED);
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+13, (curCat->viewfilters.bArchives) ? MF_CHECKED : MF_UNCHECKED);
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+14, (curCat->viewfilters.bImages) ? MF_CHECKED : MF_UNCHECKED);
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+15, (curCat->viewfilters.bSuspendFilters) ? MF_CHECKED : MF_UNCHECKED);
		
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+16, (curCat->viewfilters.bSeenComplet) ? MF_CHECKED : MF_UNCHECKED);


		// Check the appropriate menu item for the Prio menu...
	    m_mnuCatPriority.CheckMenuRadioItem(MP_PRIOLOW, MP_PRIOHIGH, MP_PRIOLOW+curCat->prio,0);
		// Check the appropriate menu item for the A4AF menu...
		m_mnuCatA4AF.CheckMenuRadioItem(MP_CAT_A4AF, MP_CAT_A4AF+2, MP_CAT_A4AF+curCat->iAdvA4AFMode,0);
	    m_mnuCatA4AF.CheckMenuItem(MP_DOWNLOAD_ALPHABETICAL, curCat->downloadInAlphabeticalOrder ? MF_CHECKED : MF_UNCHECKED);
		m_mnuCategory.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	}
	// NEO: NXC END <-- Xanatos --

	/*
	CMenu PrioMenu;
	PrioMenu.CreateMenu();
    Category_Struct* category_Struct = thePrefs.GetCategory(rightclickindex);
	PrioMenu.AppendMenu(MF_STRING,MP_PRIOLOW,GetResString(IDS_PRIOLOW));
    PrioMenu.CheckMenuItem(MP_PRIOLOW, category_Struct && category_Struct->prio == PR_LOW ? MF_CHECKED : MF_UNCHECKED);
	PrioMenu.AppendMenu(MF_STRING,MP_PRIONORMAL,GetResString(IDS_PRIONORMAL));
    PrioMenu.CheckMenuItem(MP_PRIONORMAL, category_Struct && category_Struct->prio != PR_LOW && category_Struct->prio != PR_HIGH ? MF_CHECKED : MF_UNCHECKED);
	PrioMenu.AppendMenu(MF_STRING,MP_PRIOHIGH, GetResString(IDS_PRIOHIGH));
    PrioMenu.CheckMenuItem(MP_PRIOHIGH, category_Struct && category_Struct->prio == PR_HIGH ? MF_CHECKED : MF_UNCHECKED);

	CTitleMenu menu;
	menu.CreatePopupMenu();
	if (rightclickindex)
		menu.AddMenuTitle(GetResString(IDS_CAT) + _T(" (") + thePrefs.GetCategory(rightclickindex)->strTitle + _T(")"), true);
	else
		menu.AddMenuTitle(GetResString(IDS_CAT),true);

	m_isetcatmenu = rightclickindex;
	CMenu CatMenu;
	CatMenu.CreateMenu();
	CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0,GetResString(IDS_ALL) );
	UINT flag = (!thePrefs.GetCategory(rightclickindex)->care4all && rightclickindex) ? MF_GRAYED : MF_STRING;
	CatMenu.AppendMenu(flag,MP_CAT_SET0+1,GetResString(IDS_ALLOTHERS) );

	// selector for regular expression view filter
	if (rightclickindex) {
		if (thePrefs.IsExtControlsEnabled())
			CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+18, GetResString(IDS_REGEXPRESSION) );

		flag=MF_STRING;
		if (thePrefs.GetCategory(rightclickindex)->care4all)
			flag=flag|MF_CHECKED | MF_BYCOMMAND;
		if (thePrefs.IsExtControlsEnabled() )
			CatMenu.AppendMenu(flag,MP_CAT_SET0+17,GetResString(IDS_CARE4ALL) );
	}

	CatMenu.AppendMenu(MF_SEPARATOR);
	CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+2,GetResString(IDS_STATUS_NOTCOMPLETED) );
	CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+3,GetResString(IDS_DL_TRANSFCOMPL) );
	CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+4,GetResString(IDS_WAITING) );
	CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+5,GetResString(IDS_DOWNLOADING) );
	CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+6,GetResString(IDS_ERRORLIKE) );
	CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+7,GetResString(IDS_PAUSED) );
	CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+8,GetResString(IDS_SEENCOMPL) );
	CatMenu.AppendMenu(MF_SEPARATOR);
	CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+10,GetResString(IDS_VIDEO) );
	CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+11,GetResString(IDS_AUDIO) );
	CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+12,GetResString(IDS_SEARCH_ARC) );
	CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+13,GetResString(IDS_SEARCH_CDIMG) );
	CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+14,GetResString(IDS_SEARCH_DOC) );
	CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+15,GetResString(IDS_SEARCH_PICS) );
	CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+16,GetResString(IDS_SEARCH_PRG) );
	CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+20,GetResString(IDS_SEARCH_EMULECOLLECTION) );

	if (thePrefs.IsExtControlsEnabled()) {
		CatMenu.AppendMenu(MF_SEPARATOR);
		CatMenu.AppendMenu( thePrefs.GetCatFilter(rightclickindex)>0?MF_STRING:MF_GRAYED,MP_CAT_SET0+19,GetResString(IDS_NEGATEFILTER) );
		if ( thePrefs.GetCatFilterNeg(rightclickindex))
			CatMenu.CheckMenuItem( MP_CAT_SET0+19 ,MF_CHECKED | MF_BYCOMMAND);
	}
	
	CatMenu.CheckMenuItem( MP_CAT_SET0+thePrefs.GetCatFilter(rightclickindex) ,MF_CHECKED | MF_BYCOMMAND);

	menu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)CatMenu.m_hMenu, GetResString(IDS_CHANGECATVIEW),_T("SEARCHPARAMS") );
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)PrioMenu.m_hMenu, GetResString(IDS_PRIORITY), _T("FILEPRIORITY") );
	menu.AppendMenu(MF_STRING,MP_CANCEL,GetResString(IDS_MAIN_BTN_CANCEL),_T("DELETE") );
	menu.AppendMenu(MF_STRING,MP_STOP, GetResString(IDS_DL_STOP),_T("STOP"));
	menu.AppendMenu(MF_STRING,MP_PAUSE, GetResString(IDS_DL_PAUSE), _T("PAUSE"));
	menu.AppendMenu(MF_STRING,MP_RESUME, GetResString(IDS_DL_RESUME), _T("RESUME"));
	menu.AppendMenu(MF_STRING,MP_RESUMENEXT, GetResString(IDS_DL_RESUMENEXT), _T("RESUME"));
    if(rightclickindex != 0 && thePrefs.IsExtControlsEnabled()) {
        menu.AppendMenu(MF_STRING,MP_DOWNLOAD_ALPHABETICAL, GetResString(IDS_DOWNLOAD_ALPHABETICAL));	
        menu.CheckMenuItem(MP_DOWNLOAD_ALPHABETICAL, category_Struct && category_Struct->downloadInAlphabeticalOrder ? MF_CHECKED : MF_UNCHECKED);
    }
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING,MP_HM_OPENINC, GetResString(IDS_OPENINC), _T("Incoming") );

	flag=(rightclickindex==0) ? MF_GRAYED:MF_STRING;
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING,MP_CAT_ADD,GetResString(IDS_CAT_ADD),_T("CATADD"));
	menu.AppendMenu(flag,MP_CAT_EDIT,GetResString(IDS_CAT_EDIT),_T("CATEDIT"));
	menu.AppendMenu(flag,MP_TWEAKS, GetResString(IDS_X_CAT_TWEAKS), _T("CATCONFIG")); // NEO: FCFG - [FileConfiguration] <-- Xanatos --
	menu.AppendMenu(flag,MP_CAT_REMOVE, GetResString(IDS_CAT_REMOVE),_T("CATREMOVE"));

	menu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);

	VERIFY( PrioMenu.DestroyMenu() );
	VERIFY( CatMenu.DestroyMenu() );
	VERIFY( menu.DestroyMenu() );*/

	*pResult = 0;
}

// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
void CTransferWnd::CreateCategoryMenus()
{
	if (m_mnuCatPriority) VERIFY( m_mnuCatPriority.DestroyMenu() );
	if (m_mnuCatViewFilter) VERIFY( m_mnuCatViewFilter.DestroyMenu() );
	if (m_mnuCatA4AF) VERIFY( m_mnuCatA4AF.DestroyMenu() );
	if (m_mnuCategory) VERIFY( m_mnuCategory.DestroyMenu() );
	
	// Create sub-menus first...

	// Priority Menu
	m_mnuCatPriority.CreateMenu();
	m_mnuCatPriority.AddMenuTitle(GetResString(IDS_PRIORITY) + _T(" (") + GetResString(IDS_DOWNLOAD) + _T(")"));
	m_mnuCatPriority.AppendMenu(MF_STRING, MP_PRIOLOW, GetResString(IDS_PRIOLOW));
	m_mnuCatPriority.AppendMenu(MF_STRING, MP_PRIONORMAL, GetResString(IDS_PRIONORMAL));
	m_mnuCatPriority.AppendMenu(MF_STRING, MP_PRIOHIGH, GetResString(IDS_PRIOHIGH));

	// A4AF Menu
	m_mnuCatA4AF.CreateMenu();
	m_mnuCatA4AF.AddMenuTitle(GetResString(IDS_X_CAT_ADVA4AF));
	m_mnuCatA4AF.AppendMenu(MF_STRING, MP_CAT_A4AF, GetResString(IDS_DEFAULT));
	m_mnuCatA4AF.AppendMenu(MF_STRING, MP_CAT_A4AF+1, GetResString(IDS_X_A4AF_BALANCE));
	m_mnuCatA4AF.AppendMenu(MF_STRING, MP_CAT_A4AF+2, GetResString(IDS_X_A4AF_STACK));
    m_mnuCatA4AF.AppendMenu(MF_STRING,MP_DOWNLOAD_ALPHABETICAL, GetResString(IDS_DOWNLOAD_ALPHABETICAL));

	// View Filter Menu
	m_mnuCatViewFilter.CreateMenu();
	m_mnuCatViewFilter.AddMenuTitle(GetResString(IDS_CHANGECATVIEW));
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0, GetResString(IDS_ALL) );
	//m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+1, GetResString(IDS_ALLOTHERS) );
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+1, GetResString(IDS_ALL) + _T(" + ") + GetResString(IDS_X_CAT_THISCAT)); // Lit Cat Filter
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+2, GetResString(IDS_X_CAT_THISCAT) );
		
	m_mnuCatViewFilter.AppendMenu(MF_SEPARATOR);
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+3, GetResString(IDS_COMPLETE));
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+4, GetResString(IDS_COMPLETING));
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+5, GetResString(IDS_DOWNLOADING));
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+6, GetResString(IDS_WAITING));
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+7, GetResString(IDS_PAUSED));
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+8, GetResString(IDS_STOPPED));
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+19, GetResString(IDS_X_STANDBY)); // NEO: SD - [StandByDL]
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+20, GetResString(IDS_X_SUSPEND)); // NEO: SC - [SuspendCollecting]
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+9, GetResString(IDS_HASHING)); // NEO: SSH - [SlugFillerSafeHash]
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+10, GetResString(IDS_ERRORLIKE));	
	
	m_mnuCatViewFilter.AppendMenu(MF_SEPARATOR);
	CString strtemp = GetResString(IDS_LASTSEENCOMPL);
	strtemp.Remove(':');
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+16, strtemp);	

	m_mnuCatViewFilter.AppendMenu(MF_SEPARATOR);
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+11, GetResString(IDS_VIDEO));
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+12, GetResString(IDS_AUDIO));
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+13, GetResString(IDS_SEARCH_ARC));
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+14, GetResString(IDS_SEARCH_CDIMG));
	
	m_mnuCatViewFilter.AppendMenu(MF_SEPARATOR);
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+15, GetResString(IDS_X_CAT_SUSPENDFILTERS));

	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+17, GetResString(IDS_COL_MORECOLORS));

	// Create the main menu...
	m_mnuCategory.CreatePopupMenu();
	m_mnuCategory.AddMenuTitle(GetResString(IDS_CAT),true);

	m_mnuCategory.AppendMenu(MF_STRING | MF_POPUP, (UINT_PTR)m_mnuCatViewFilter.m_hMenu, GetResString(IDS_CHANGECATVIEW),_T("SEARCHPARAMS"));
	m_mnuCategory.AppendMenu(MF_SEPARATOR);
	m_mnuCategory.AppendMenu(MF_STRING, MP_CAT_ADD, GetResString(IDS_CAT_ADD),_T("CATADD"));
	m_mnuCategory.AppendMenu(MF_STRING, MP_CAT_EDIT, GetResString(IDS_CAT_EDIT),_T("CATEDIT"));
	//m_mnuCategory.AppendMenu(MF_STRING, MP_CAT_MERGE, GetResString(IDS_X_CAT_MERGE),_T("CATMERGE"));
	m_mnuCategory.AppendMenu(MF_STRING, MP_CAT_REMOVE, GetResString(IDS_CAT_REMOVE),_T("CATREMOVE"));
	m_mnuCategory.AppendMenu(MF_STRING,MP_TWEAKS, GetResString(IDS_X_CAT_TWEAKS), _T("CATCONFIG")); // NEO: FCFG - [FileConfiguration]
	m_mnuCategory.AppendMenu(MF_SEPARATOR);
	m_mnuCategory.AppendMenu(MF_STRING,MP_HM_OPENINC, GetResString(IDS_OPENINC), _T("FOLDERS") );
	m_mnuCategory.AppendMenu(MF_SEPARATOR);
	m_mnuCategory.AppendMenu(MF_STRING | MF_POPUP, (UINT_PTR)m_mnuCatPriority.m_hMenu, GetResString(IDS_PRIORITY) + _T(" (") + GetResString(IDS_DOWNLOAD) + _T(")"), _T("FILEPRIORITY"));
	m_mnuCategory.AppendMenu(MF_STRING | MF_POPUP, (UINT_PTR)m_mnuCatA4AF.m_hMenu, GetResString(IDS_X_CAT_ADVA4AF),_T("CATADVA4AF"));
	m_mnuCategory.AppendMenu(MF_SEPARATOR);
	m_mnuCategory.AppendMenu(MF_STRING, MP_CANCEL, GetResString(IDS_MAIN_BTN_CANCEL), _T("DELETE"));
	m_mnuCategory.AppendMenu(MF_STRING, MP_STOP, GetResString(IDS_DL_STOP), _T("STOP"));
	m_mnuCategory.AppendMenu(MF_STRING, MP_PAUSE, GetResString(IDS_DL_PAUSE), _T("PAUSE"));
	m_mnuCategory.AppendMenu(MF_STRING, MP_RESUME, GetResString(IDS_DL_RESUME), _T("RESUME"));
	m_mnuCategory.AppendMenu(MF_SEPARATOR);
	m_mnuCategory.AppendMenu(MF_STRING, MP_CAT_STOPLAST, GetResString(IDS_X_CAT_STOPLAST), _T("CATSTOPLAST"));	
	m_mnuCategory.AppendMenu(MF_STRING, MP_CAT_PAUSELAST, GetResString(IDS_X_CAT_PAUSELAST), _T("CATPAUSELAST"));	
	m_mnuCategory.AppendMenu(MF_STRING, MP_RESUMENEXT, GetResString(IDS_X_CAT_RESUMENEXT), _T("CATRESUMENEXT"));   
}
// NEO: NXC END <-- Xanatos --

void CTransferWnd::OnLvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult)
{
    int iSel = downloadlistctrl.GetSelectionMark();
	if (iSel==-1) return;
	if (((CtrlItem_Struct*)downloadlistctrl.GetItemData(iSel))->type != FILE_TYPE) return;
	
	m_bIsDragging = true;

	POINT pt;
	::GetCursorPos(&pt);

	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	m_nDragIndex = pNMLV->iItem;
	m_pDragImage = downloadlistctrl.CreateDragImage( downloadlistctrl.GetSelectionMark() ,&pt);
    m_pDragImage->BeginDrag( 0, CPoint(0,0) );
    m_pDragImage->DragEnter( GetDesktopWindow(), pNMLV->ptAction );
    SetCapture();
	m_nDropIndex = -1;

	*pResult = 0;
}

void CTransferWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	if( !(nFlags & MK_LBUTTON) ) m_bIsDragging = false;

	if (m_bIsDragging){
		CPoint pt(point);           //get our current mouse coordinates
		ClientToScreen(&pt);        //convert to screen coordinates

		m_nDropIndex=GetTabUnderMouse(&pt);

		// NEO: NXC - [NewExtendedCategories] -- Xanatos --
		//if (m_nDropIndex>0 && thePrefs.GetCategory(m_nDropIndex)->care4all)	// not droppable
		//	m_dlTab.SetCurSel(-1);
		//else
			m_dlTab.SetCurSel(m_nDropIndex);

		m_dlTab.Invalidate();
		
		::GetCursorPos(&pt);
		pt.y-=10;
		m_pDragImage->DragMove(pt); //move the drag image to those coordinates
	}
}

void CTransferWnd::OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/)
{
	if (m_bIsDragging)
	{
		ReleaseCapture ();
		m_bIsDragging = false;
		m_pDragImage->DragLeave (GetDesktopWindow ());
		m_pDragImage->EndDrag ();
		delete m_pDragImage;
		
		if (m_nDropIndex > -1 && (downloadlistctrl.curTab==0 ||
				(downloadlistctrl.curTab > 0 && (UINT)m_nDropIndex != downloadlistctrl.curTab) )) {

			CPartFile* file;

			// for multiselections
			CTypedPtrList <CPtrList,CPartFile*> selectedList; 
			POSITION pos = downloadlistctrl.GetFirstSelectedItemPosition();
			while(pos != NULL) 
			{ 
				int index = downloadlistctrl.GetNextSelectedItem(pos);
				if(index > -1 && (((CtrlItem_Struct*)downloadlistctrl.GetItemData(index))->type == FILE_TYPE))
					selectedList.AddTail( (CPartFile*)((CtrlItem_Struct*)downloadlistctrl.GetItemData(index))->value );
			}

			while (!selectedList.IsEmpty())
			{
				file = selectedList.GetHead();
				selectedList.RemoveHead();
				file->SetCategory(m_nDropIndex);
			}


			m_dlTab.SetCurSel(downloadlistctrl.curTab);
			//if (m_dlTab.GetCurSel()>0 || (thePrefs.GetAllcatType()==1 && m_dlTab.GetCurSel()==0) )
			downloadlistctrl.UpdateCurrentCategoryView();

			UpdateCatTabTitles();

		} else m_dlTab.SetCurSel(downloadlistctrl.curTab);
		downloadlistctrl.Invalidate();
	}
}

BOOL CTransferWnd::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	// category filter menuitems
	/*if (wParam>=MP_CAT_SET0 && wParam<=MP_CAT_SET0+99)
	{
		if (wParam==MP_CAT_SET0+17)
		{
			thePrefs.GetCategory(m_isetcatmenu)->care4all=!thePrefs.GetCategory(m_isetcatmenu)->care4all;
		}
		else if (wParam==MP_CAT_SET0+19) // negate
		{
			thePrefs.SetCatFilterNeg(m_isetcatmenu, (!thePrefs.GetCatFilterNeg(m_isetcatmenu)));
		}
		else // set the view filter
		{
			if (wParam-MP_CAT_SET0<1)	// dont negate all filter
				thePrefs.SetCatFilterNeg(m_isetcatmenu, false);
			thePrefs.SetCatFilter(m_isetcatmenu,wParam-MP_CAT_SET0);
			m_nLastCatTT=-1;
		}

		// set to regexp but none is set for that category?
		if (wParam==MP_CAT_SET0+18 && thePrefs.GetCategory(m_isetcatmenu)->regexp.IsEmpty())
		{
			m_nLastCatTT=-1;
			CCatDialog dialog(rightclickindex);
			dialog.DoModal();

			// still no regexp?
			if (thePrefs.GetCategory(m_isetcatmenu)->regexp.IsEmpty())
				thePrefs.SetCatFilter(m_isetcatmenu,0);
		}

		downloadlistctrl.UpdateCurrentCategoryView();
		EditCatTabLabel(m_isetcatmenu);
		thePrefs.SaveCats();
		return TRUE;
	}*/

	// NEO: NTB - [NeoToolbarButtons] -- Xanatos -->
	if (wParam>=MP_VIEWN_0 && wParam<=MP_VIEWN_0+99)
	{
		NeoCommand((uint8)(wParam - MP_VIEWN_0));
		return TRUE;
	}
	if(wParam == IDC_NEO_ICO)
	{
		OnWndNBtnDropDown(0,0);
		return TRUE;
	}
	// NEO: NTB END <-- Xanatos --

	Category_Struct* curCat = thePrefs.GetCategory(rightclickindex); // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
	
	switch (wParam)
	{ 
		case MP_CAT_ADD: {
			m_nLastCatTT=-1;
			int newindex=AddCategory(_T("?"),thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR),_T(""),_T(""),false);
			CCatDialog dialog(newindex);
			if (dialog.DoModal() == IDOK)
			{
				theApp.emuledlg->searchwnd->UpdateCatTabs();
				m_dlTab.InsertItem(newindex,thePrefs.GetCategory(newindex)->strTitle);
				m_dlTab.SetTabTextColor(newindex, thePrefs.GetCatColor(newindex) );
				EditCatTabLabel(newindex);
				thePrefs.SaveCats();
				VerifyCatTabSize();
				//if (CompareDirectories(thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR), thePrefs.GetCatPath(newindex)))
				//	theApp.emuledlg->sharedfileswnd->Reload();
				theApp.emuledlg->sharedfileswnd->Reload(!CompareDirectories(thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR), thePrefs.GetCatPath(newindex))); // NEO: NSC - [NeoSharedCategories] <-- Xanatos --
			}
			else
				thePrefs.RemoveCat(newindex);
			UpdateCatTabIcons(); // NEO: CCF - [ColloredCategoryFlags] <-- Xanatos --
			break;
		}
		case MP_CAT_SET0+17: // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
		case MP_CAT_EDIT: {
			m_nLastCatTT=-1;
			CString oldincpath=thePrefs.GetCatPath(rightclickindex);
			CCatDialog dialog(rightclickindex);
			if (dialog.DoModal() == IDOK)
			{
				EditCatTabLabel(rightclickindex, thePrefs.GetCategory(rightclickindex)->strTitle);
				m_dlTab.SetTabTextColor(rightclickindex, thePrefs.GetCatColor(rightclickindex) );
				UpdateCatTabIcons(); // NEO: CCF - [ColloredCategoryFlags] <-- Xanatos --
				theApp.emuledlg->searchwnd->UpdateCatTabs();				
				theApp.emuledlg->transferwnd->downloadlistctrl.UpdateCurrentCategoryView();
				thePrefs.SaveCats();
				//if (CompareDirectories(oldincpath, thePrefs.GetCatPath(rightclickindex)))
				//	theApp.emuledlg->sharedfileswnd->Reload();
				theApp.emuledlg->sharedfileswnd->Reload(!CompareDirectories(oldincpath, thePrefs.GetCatPath(rightclickindex))); // NEO: NSC - [NeoSharedCategories] <-- Xanatos --
			}
			break;
		}
		// NEO: FCFG - [FileConfiguration]
		case MP_TWEAKS:{
			m_nLastCatTT=-1;
			Category_Struct* pCatStruct = thePrefs.GetCategory(rightclickindex);
			if (pCatStruct != NULL){
				// NEO: MLD - [ModelesDialogs] 
				CFilePreferencesDialog* dlg = new CFilePreferencesDialog(pCatStruct);
				dlg->OpenDialog(); 
				// NEO: MLD END
				//CFilePreferencesDialog dialog(pCatStruct);
				//dialog.DoModal();
			}
			break;
		}
		// NEO: FCFG END
		case MP_CAT_REMOVE: {
			m_nLastCatTT=-1;
			bool toreload=( _tcsicmp(thePrefs.GetCatPath(rightclickindex), thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR))!=0);
			theApp.downloadqueue->ResetCatParts(rightclickindex);
			thePrefs.RemoveCat(rightclickindex);
			m_dlTab.DeleteItem(rightclickindex);
			m_dlTab.SetCurSel(0);
			downloadlistctrl.ChangeCategory(0);
			thePrefs.SaveCats();
			// NEO: NXC - [NewExtendedCategories] -- Xanatos --
			//if (thePrefs.GetCatCount()==1) 
			//	thePrefs.GetCategory(0)->filter=0;
			UpdateCatTabIcons(); // NEO: CCF - [ColloredCategoryFlags] <-- Xanatos --
			theApp.emuledlg->searchwnd->UpdateCatTabs();
			VerifyCatTabSize();
			//if (toreload)
			//	theApp.emuledlg->sharedfileswnd->Reload();
			theApp.emuledlg->sharedfileswnd->Reload(!toreload); // NEO: NSC - [NeoSharedCategories] <-- Xanatos --
			break;
		}

		case MP_PRIOLOW:
            thePrefs.GetCategory(rightclickindex)->prio = PR_LOW;
			thePrefs.SaveCats();
			UpdateCatTabIcons(); // NEO: CCF - [ColloredCategoryFlags] <-- Xanatos --
			break;
		case MP_PRIONORMAL:
            thePrefs.GetCategory(rightclickindex)->prio = PR_NORMAL;
			thePrefs.SaveCats();
			UpdateCatTabIcons(); // NEO: CCF - [ColloredCategoryFlags] <-- Xanatos --
			break;
		case MP_PRIOHIGH:
            thePrefs.GetCategory(rightclickindex)->prio = PR_HIGH;
			thePrefs.SaveCats();
			UpdateCatTabIcons(); // NEO: CCF - [ColloredCategoryFlags] <-- Xanatos --
			break;

		case MP_PAUSE:
			theApp.downloadqueue->SetCatStatus(rightclickindex,MP_PAUSE);
			break;
		case MP_STOP:
			theApp.downloadqueue->SetCatStatus(rightclickindex,MP_STOP);
			break;
		case MP_CANCEL:
			if (AfxMessageBox(GetResString(IDS_Q_CANCELDL),MB_ICONQUESTION|MB_YESNO) == IDYES)
				theApp.downloadqueue->SetCatStatus(rightclickindex,MP_CANCEL);
			break;
		case MP_RESUME:
			theApp.downloadqueue->SetCatStatus(rightclickindex,MP_RESUME);
			break;
		case MP_RESUMENEXT:
			theApp.downloadqueue->StartNextFile(rightclickindex,false);
			break;

		case MP_DOWNLOAD_ALPHABETICAL: {
            bool newSetting = !thePrefs.GetCategory(rightclickindex)->downloadInAlphabeticalOrder;
            thePrefs.GetCategory(rightclickindex)->downloadInAlphabeticalOrder = newSetting;
			thePrefs.SaveCats();
            if(newSetting) {
                // any auto prio files will be set to normal now.
                theApp.downloadqueue->RemoveAutoPrioInCat(rightclickindex, PR_NORMAL);
            }
            break;
		}

		// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
		case MP_CAT_STOPLAST: {
			theApp.downloadqueue->StopPauseLastFile(MP_STOP, rightclickindex);
			break;
		}
		case MP_CAT_PAUSELAST: {
			theApp.downloadqueue->StopPauseLastFile(MP_PAUSE, rightclickindex);
			break;
		}
		// NEO: NXC END <-- Xanatos --

		case IDC_UPLOAD_ICO:
			SwitchUploadList();
			break;
		case MP_VIEW2_UPLOADING:
			ShowWnd2(wnd2Uploading);
			break;
		case MP_VIEW2_DOWNLOADING:
			ShowWnd2(wnd2Downloading);
			break;
		case MP_VIEW2_ONQUEUE:
			ShowWnd2(wnd2OnQueue);
			break;
		case MP_VIEW2_CLIENTS:
			ShowWnd2(wnd2Clients);
			break;
		case IDC_QUEUE_REFRESH_BUTTON:
			OnBnClickedQueueRefreshButton();
			break;

		case IDC_DOWNLOAD_ICO:
			OnBnClickedChangeView();
			break;
		case MP_VIEW1_SPLIT_WINDOW:
			ShowSplitWindow();
			break;
		case MP_VIEW1_DOWNLOADS:
			ShowList(IDC_DOWNLOADLIST);
			break;
		case MP_VIEW1_UPLOADING:
			ShowList(IDC_UPLOADLIST);
			break;
		case MP_VIEW1_DOWNLOADING:
			ShowList(IDC_DOWNLOADCLIENTS);
			break;
		case MP_VIEW1_ONQUEUE:
			ShowList(IDC_QUEUELIST);
			break;
		case MP_VIEW1_CLIENTS:
			ShowList(IDC_CLIENTLIST);
			break;

		// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
		case MP_CAT_SET0+1:
			/*if (rightclickindex != 0 && theApp.downloadqueue->GetCategoryFileCount(rightclickindex))  // Lit Cat Filter
			{
				MessageBox(GetResString(IDS_X_CAT_FROMCATSINFO), GetResString(IDS_X_CAT_FROMCATSCAP));
				curCat->viewfilters.nFromCats = 2;
				EditCatTabLabel(rightclickindex, CString(curCat->title));
				break;
			}*/
		case MP_CAT_SET0:
		case MP_CAT_SET0+2: {
			curCat->viewfilters.nFromCats = wParam - MP_CAT_SET0;
			break;
		}
		case MP_CAT_SET0+3: {
			curCat->viewfilters.bComplete = curCat->viewfilters.bComplete ? false : true;
			break;
		}
		case MP_CAT_SET0+4: {
			curCat->viewfilters.bCompleting = curCat->viewfilters.bCompleting ? false : true;
			break;
		}
		case MP_CAT_SET0+5: {
			curCat->viewfilters.bTransferring = curCat->viewfilters.bTransferring ? false : true;
			break;
		}
		case MP_CAT_SET0+6: {
			curCat->viewfilters.bWaiting = curCat->viewfilters.bWaiting ? false : true;
			break;
		}
		case MP_CAT_SET0+7: {
			curCat->viewfilters.bPaused = curCat->viewfilters.bPaused ? false : true;
			break;
		}
		case MP_CAT_SET0+8: {
			curCat->viewfilters.bStopped = curCat->viewfilters.bStopped ? false : true;
			break;
		}
		// NEO: SD - [StandByDL]
		case MP_CAT_SET0+19: { 
			curCat->viewfilters.bStandby = curCat->viewfilters.bStandby ? false : true;
			break;
		}
		// NEO: SD END
		// NEO: SC - [SuspendCollecting]
		case MP_CAT_SET0+20: { 
			curCat->viewfilters.bSuspend = curCat->viewfilters.bSuspend ? false : true;
			break;
		}
		// NEO: SC END
		case MP_CAT_SET0+9: {
			curCat->viewfilters.bHashing = curCat->viewfilters.bHashing ? false : true;
			break;
		}
		case MP_CAT_SET0+10: {
			curCat->viewfilters.bErrorUnknown = curCat->viewfilters.bErrorUnknown ? false : true;
			break;
		}
		case MP_CAT_SET0+11: {
			curCat->viewfilters.bVideo = curCat->viewfilters.bVideo ? false : true;
			break;
		}
		case MP_CAT_SET0+12: {
			curCat->viewfilters.bAudio = curCat->viewfilters.bAudio ? false : true;
			break;
		}
		case MP_CAT_SET0+13: {
			curCat->viewfilters.bArchives = curCat->viewfilters.bArchives ? false : true;
			break;
		}
		case MP_CAT_SET0+14: {
			curCat->viewfilters.bImages = curCat->viewfilters.bImages ? false : true;
			break;
		}
		case MP_CAT_SET0+15: {
			curCat->viewfilters.bSuspendFilters = curCat->viewfilters.bSuspendFilters ? false : true;
			break;
		}

		case MP_CAT_SET0+16: {
			curCat->viewfilters.bSeenComplet = curCat->viewfilters.bSeenComplet ? false : true;
			break;
		}
		// NEO: NXC END <-- Xanatos --

		case MP_HM_OPENINC:
			//ShellExecute(NULL, _T("open"), thePrefs.GetCategory(m_isetcatmenu)->strIncomingPath,NULL, NULL, SW_SHOW);
			ShellExecute(NULL, _T("open"), thePrefs.GetCatPath(m_isetcatmenu),NULL, NULL, SW_SHOW); // NEO: NXC - [NewExtendedCategories] -- Xanatos --
			break;

	}

	// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
	if (wParam >= MP_CAT_SET0 && wParam <= MP_CAT_SET0 + 16)
		downloadlistctrl.ChangeCategory(m_dlTab.GetCurSel());
	if (wParam >= MP_CAT_A4AF && wParam <= MP_CAT_A4AF + 2)
		curCat->iAdvA4AFMode = (uint8)(wParam - MP_CAT_A4AF);
	// NEO: NXC END <-- Xanatos --

	return TRUE;
}

void CTransferWnd::UpdateCatTabTitles(bool force)
{
	CPoint pt;
	::GetCursorPos(&pt);
	if (!force && GetTabUnderMouse(&pt)!=-1)		// avoid cat tooltip jumping
		return;

	for (int i = 0; i < m_dlTab.GetItemCount(); i++){
		EditCatTabLabel(i,/*(i==0)? GetCatTitle( thePrefs.GetCategory(0)->filter ):*/thePrefs.GetCategory(i)->strTitle);
		m_dlTab.SetTabTextColor(i, thePrefs.GetCatColor(i) );
	}
}

void CTransferWnd::EditCatTabLabel(int i)
{
	EditCatTabLabel(i,/*(i==0)? GetCatTitle( thePrefs.GetAllcatType() ):*/thePrefs.GetCategory(i)->strTitle);
}

void CTransferWnd::EditCatTabLabel(int index,CString newlabel)
{
	TCITEM tabitem;
	tabitem.mask = TCIF_PARAM;
	m_dlTab.GetItem(index,&tabitem);
	tabitem.mask = TCIF_TEXT;

	newlabel.Replace(_T("&"),_T("&&"));

	// NEO: NXC - [NewExtendedCategories] -- Xanatos --
	/*if (!index)
		newlabel.Empty();

	if (!index || (index && thePrefs.GetCatFilter(index)>0)) {

		if (index)
			newlabel.Append(_T(" (")) ;
		
		if (thePrefs.GetCatFilterNeg(index))
			newlabel.Append(_T("!"));			
 
		if (thePrefs.GetCatFilter(index)==18)
			newlabel.Append( _T("\"") + thePrefs.GetCategory(index)->regexp + _T("\"") );
		else
        	newlabel.Append( GetCatTitle(thePrefs.GetCatFilter(index)));

		if (index)
			newlabel.Append( _T(")") );
	}*/

	int count,dwl;
	int catcount, canpause; // NEO: MOD - [MoreCategoryInfo] <-- Xanatos --
	if (thePrefs.ShowCatTabInfos()) 
	{
		CPartFile* cur_file;
		count=dwl=0;
		catcount=canpause=0; // NEO: MOD - [MoreCategoryInfo] <-- Xanatos --
		for (int i=0;i<theApp.downloadqueue->GetFileCount();i++) {
			cur_file=theApp.downloadqueue->GetFileByIndex(i);
			if (cur_file==0) continue;
			if (cur_file->CheckShowItemInGivenCat(index)) {
				if (cur_file->GetTransferringSrcCount()>0) ++dwl;
			}
			// NEO: MOD - [MoreCategoryInfo] -- Xanatos -->
			if(cur_file->GetCategory() == (UINT)index)
			{
				catcount++;
				if(cur_file->CanPauseFile())
					canpause++;
			}
			// NEO: MOD END <-- Xanatos --
		}
		CString title=newlabel;
		theApp.emuledlg->transferwnd->downloadlistctrl.GetCompleteDownloads(index, count);
		// NEO: MOD - [MoreCategoryInfo] -- Xanatos -->
		if(thePrefs.ShowCatTabInfos() == TRUE)
		{
			if(canpause != catcount)
			{
				if(canpause == 0)
					newlabel.Format(_T("%s %i/%i!"),title,dwl,count);
				else
					newlabel.Format(_T("%s %i/%i*"),title,dwl,count);
			}
			else
				newlabel.Format(_T("%s %i/%i"),title,dwl,count);
		}else
		// NEO: MOD END <-- Xanatos --
		newlabel.Format(_T("%s %i/%i"),title,dwl,count);
	}

	tabitem.pszText = newlabel.LockBuffer();
	m_dlTab.SetItem(index,&tabitem);
	newlabel.UnlockBuffer();

	VerifyCatTabSize();
}

//int CTransferWnd::AddCategory(CString newtitle,CString newincoming,CString newcomment, CString newautocat, bool addTab)
int CTransferWnd::AddCategory(CString newtitle,CString newincoming,CString newcomment, CString newautocat, bool addTab, bool share) // NEO: NSC - [NeoSharedCategories] <-- Xanatos --
{
	Category_Struct* newcat=new Category_Struct;
	newcat->strTitle = newtitle;
	newcat->prio=PR_NORMAL;
	newcat->strIncomingPath = _T(""); //newincoming; // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
	newcat->strTempPath = _T(""); // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
	newcat->strComment = newcomment;
	// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
	newcat->iAdvA4AFMode = 0;
	newcat->boost = PR_NORMAL;
	newcat->release = false; // NEO: SRS - [SmartReleaseSharing]
	newcat->viewfilters.bArchives = true;
	newcat->viewfilters.bAudio = true;
	newcat->viewfilters.bComplete = true;
	newcat->viewfilters.bCompleting = true;
	newcat->viewfilters.bSeenComplet = true; 
	newcat->viewfilters.bErrorUnknown = true;
	newcat->viewfilters.bHashing = true;
	newcat->viewfilters.bImages = true;
	newcat->viewfilters.bPaused = true;
	newcat->viewfilters.bStopped = true;
	newcat->viewfilters.bStandby = true; // NEO: SD - [StandByDL]
	newcat->viewfilters.bSuspend = true; // NEO: SC - [SuspendCollecting]
	newcat->viewfilters.bSuspendFilters = false;
	newcat->viewfilters.bTransferring = true;
	newcat->viewfilters.bVideo = true;
	newcat->viewfilters.bWaiting = true;
	newcat->viewfilters.nAvailSourceCountMax = 0;
	newcat->viewfilters.nAvailSourceCountMin = 0;
	newcat->viewfilters.nFromCats = 2;
	newcat->viewfilters.nFSizeMax = 0;
	newcat->viewfilters.nFSizeMin = 0;
	newcat->viewfilters.nRSizeMax = 0;
	newcat->viewfilters.nRSizeMin = 0;
	newcat->viewfilters.nSourceCountMax = 0;
	newcat->viewfilters.nSourceCountMin = 0;
	newcat->viewfilters.nTimeRemainingMax = 0;
	newcat->viewfilters.nTimeRemainingMin = 0;
	newcat->viewfilters.sAdvancedFilterMask = "";
	newcat->selectioncriteria.bAdvancedFilterMask = false;
	newcat->selectioncriteria.bFileSize = false;
	newcat->bResumeFileOnlyInSameCat = false;
	// NEO: NXC END <-- Xanatos --
	//newcat->regexp.Empty();
	//newcat->ac_regexpeval=false;
	//newcat->autocat=newautocat;
    newcat->downloadInAlphabeticalOrder = FALSE;
	//newcat->filter=0;
	//newcat->filterNeg=false;
	//newcat->care4all=false;
	newcat->color= (DWORD)-1;

	// NEO: FCFG - [FileConfiguration] -- Xanatos -->
	newcat->PartPrefs = NULL;
	newcat->KnownPrefs = NULL;
	// NEO: FCFG END <-- Xanatos --

	//int index=thePrefs.AddCat(newcat);
	int index=thePrefs.InsertCat(newcat, share); // NEO: NSC - [NeoSharedCategories] <-- Xanatos --
	if (addTab) m_dlTab.InsertItem(index,newtitle);
	UpdateCatTabIcons(); // NEO: CCF - [ColloredCategoryFlags] <-- Xanatos --
	VerifyCatTabSize();
	
	return index;
}

int CTransferWnd::GetTabUnderMouse(CPoint* point)
{
	TCHITTESTINFO hitinfo;
	CRect rect;
	m_dlTab.GetWindowRect(&rect);
	point->Offset(0-rect.left,0-rect.top);
	hitinfo.pt = *point;

	if( m_dlTab.GetItemRect( 0, &rect ) )
		if (hitinfo.pt.y< rect.top+30 && hitinfo.pt.y >rect.top-30)
			hitinfo.pt.y = rect.top;

	// Find the destination tab...
	unsigned int nTab = m_dlTab.HitTest( &hitinfo );

	if( hitinfo.flags != TCHT_NOWHERE )
		return nTab;
	else
		return -1;
}

void CTransferWnd::OnLvnKeydownDownloadlist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	int iItem = downloadlistctrl.GetSelectionMark();
	if (iItem != -1)
	{
		bool bAltKey = GetAsyncKeyState(VK_MENU) < 0;
		int iAction = EXPAND_COLLAPSE;
		if (pLVKeyDow->wVKey==VK_ADD || (bAltKey && pLVKeyDow->wVKey==VK_RIGHT))
			iAction = EXPAND_ONLY;
		else if (pLVKeyDow->wVKey==VK_SUBTRACT || (bAltKey && pLVKeyDow->wVKey==VK_LEFT))
			iAction = COLLAPSE_ONLY;
		if (iAction < EXPAND_COLLAPSE)
			downloadlistctrl.ExpandCollapseItem(iItem, iAction, true);
	}
	*pResult = 0;
}

#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
void CTransferWnd::UpdateTabToolTips() 
{
	CPoint point;
	::GetCursorPos(&point);
	int Index = GetTabUnderMouse(&point);
	if (Index != m_nLastCatTT)
		{
		m_nLastCatTT = Index;
		if (m_nDropIndex != -1)
			m_tabtip.Pop();
	}
}

#else

void CTransferWnd::UpdateTabToolTips(int tab)
{
	if (tab == -1)
	{
		for (int i = 0; i < m_tooltipCats->GetToolCount(); i++)
			m_tooltipCats->DelTool(&m_dlTab, i + 1);

		for (int i = 0; i < m_dlTab.GetItemCount(); i++)
		{
			CRect r;
			m_dlTab.GetItemRect(i, &r);
			VERIFY( m_tooltipCats->AddTool(&m_dlTab, GetTabStatistic(i), &r, i + 1) );
		}
	}
	else
	{
		CRect r;
		m_dlTab.GetItemRect(tab, &r);
		m_tooltipCats->DelTool(&m_dlTab, tab + 1);
		VERIFY( m_tooltipCats->AddTool(&m_dlTab, GetTabStatistic(tab), &r, tab + 1) );
	}
}

void CTransferWnd::SetToolTipsDelay(DWORD dwDelay)
{
	m_tooltipCats->SetDelayTime(TTDT_INITIAL, dwDelay);

	CToolTipCtrl* tooltip = downloadlistctrl.GetToolTips();
	if (tooltip)
		tooltip->SetDelayTime(TTDT_INITIAL, dwDelay);

	tooltip = uploadlistctrl.GetToolTips();
	if (tooltip)
		tooltip->SetDelayTime(TTDT_INITIAL, dwDelay);
}

CString CTransferWnd::GetTabStatistic(int tab)
{
	uint16 count, dwl, err, paus;
	count = dwl = err = paus = 0;
	float speed = 0;
	uint64 size = 0;
	uint64 trsize = 0;
	uint64 disksize = 0;
	for (int i = 0; i < theApp.downloadqueue->GetFileCount(); i++)
	{
		/*const*/ CPartFile* cur_file = theApp.downloadqueue->GetFileByIndex(i);
		if (cur_file == 0)
			continue;
		if (cur_file->CheckShowItemInGivenCat(tab))
		{
			count++;
			if (cur_file->GetTransferringSrcCount() > 0)
				dwl++;
			speed += cur_file->GetDatarate() / 1024.0F;
			size += (uint64)cur_file->GetFileSize();
			trsize += (uint64)cur_file->GetCompletedSize();
			if (!cur_file->IsAllocating())
				disksize += (uint64)cur_file->GetRealFileSize();
			if (cur_file->GetStatus() == PS_ERROR)
				err++;
			if (cur_file->GetStatus() == PS_PAUSED)
				paus++;
		}
	}

	int total;
	int compl = theApp.emuledlg->transferwnd->downloadlistctrl.GetCompleteDownloads(tab, total);

    CString prio;
    switch (thePrefs.GetCategory(tab)->prio)
	{
        case PR_LOW:
            prio = GetResString(IDS_PRIOLOW);
            break;
        case PR_HIGH:
            prio = GetResString(IDS_PRIOHIGH);
            break;
        default:
            prio = GetResString(IDS_PRIONORMAL);
            break;
    }

	CString title;
	title.Format(_T("%s: %i\n\n%s: %i\n%s: %i\n%s: %i\n%s: %i\n\n%s: %s\n\n%s: %.1f %s\n%s: %s/%s\n%s%s"),
		GetResString(IDS_FILES), count + compl,
		GetResString(IDS_DOWNLOADING), dwl,
		GetResString(IDS_PAUSED), paus,
		GetResString(IDS_ERRORLIKE), err,
		GetResString(IDS_DL_TRANSFCOMPL), compl,
        GetResString(IDS_PRIORITY), prio,
		GetResString(IDS_DL_SPEED), speed, GetResString(IDS_KBYTESPERSEC),
		GetResString(IDS_DL_SIZE), CastItoXBytes(trsize, false, false), CastItoXBytes(size, false, false),
		GetResString(IDS_ONDISK), CastItoXBytes(disksize, false, false));
	title += TOOLTIP_AUTOFORMAT_SUFFIX_CH;
	return title;
}
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

void CTransferWnd::OnDblclickDltab()
{
	POINT point;
	::GetCursorPos(&point);
	CPoint pt(point);
	int tab = GetTabUnderMouse(&pt);
	if (tab < 1)
		return;
	rightclickindex = tab;
	OnCommand(MP_CAT_EDIT, 0);
}

void CTransferWnd::OnTabMovement(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	UINT from=m_dlTab.GetLastMovementSource();
	UINT to=m_dlTab.GetLastMovementDestionation();

	if (from==0 || to==0 || from==to-1) return;

	// do the reorder
	
	// rearrange the cat-map
	if (!thePrefs.MoveCat(from,to)) return;

	// update partfile-stored assignment
	theApp.downloadqueue->MoveCat((uint8)from,(uint8)to);
	theApp.knownfiles->MoveCat((uint8)from,(uint8)to); // NEO: NSC - [NeoSharedCategories] <-- Xanatos --

	// move category of completed files
	downloadlistctrl.MoveCompletedfilesCat((uint8)from,(uint8)to);

	// of the tabcontrol itself
	m_dlTab.ReorderTab(from,to);

	UpdateCatTabTitles();
	theApp.emuledlg->searchwnd->UpdateCatTabs();
	UpdateCatTabIcons(); // NEO: CCF - [ColloredCategoryFlags] <-- Xanatos --

	if (to>from) --to;
	m_dlTab.SetCurSel(to);
	downloadlistctrl.ChangeCategory(to);
}

void CTransferWnd::VerifyCatTabSize()
{
	if (m_dwShowListIDC != IDC_DOWNLOADLIST && m_dwShowListIDC != IDC_UPLOADLIST + IDC_DOWNLOADLIST)
		return;

	int size = 0;
	for (int i = 0; i < m_dlTab.GetItemCount(); i++)
	{
		CRect rect;
		m_dlTab.GetItemRect(i, &rect);
		size += rect.Width();
	}
	size += 4;

	int right;
	WINDOWPLACEMENT wp;
	downloadlistctrl.GetWindowPlacement(&wp);
	right = wp.rcNormalPosition.right;
	m_dlTab.GetWindowPlacement(&wp);
	if (wp.rcNormalPosition.right < 0)
		return;
	wp.rcNormalPosition.right = right;

	int left = wp.rcNormalPosition.right - size;
	CRect rcBtnWnd1;
	m_btnWnd1->GetWindowRect(rcBtnWnd1);
	ScreenToClient(rcBtnWnd1);
	if (left < rcBtnWnd1.right + 10)
		left = rcBtnWnd1.right + 10;
	wp.rcNormalPosition.left = left;

	// NEO: NTB - [NeoToolbarButtons] -- Xanatos -->
	if(NeoPrefs.UseNeoToolbar()){
		CRect rcBtnWndN;
		m_btnWndN->GetWindowRect(rcBtnWndN);
		ScreenToClient(rcBtnWndN);
		if (left < rcBtnWndN.right + 10)
			left = rcBtnWndN.right + 10;
		wp.rcNormalPosition.left = left;
	}
	// NEO: NTB END <-- Xanatos --

	RemoveAnchor(m_dlTab);
	m_dlTab.SetWindowPlacement(&wp);
	AddAnchor(m_dlTab, TOP_RIGHT);
}

// NEO: NXC - [NewExtendedCategories] -- Xanatos --
/*CString CTransferWnd::GetCatTitle(int catid)
{
	switch (catid) {
		case 0 : return GetResString(IDS_ALL);
		case 1 : return GetResString(IDS_ALLOTHERS);
		case 2 : return GetResString(IDS_STATUS_NOTCOMPLETED);
		case 3 : return GetResString(IDS_DL_TRANSFCOMPL);
		case 4 : return GetResString(IDS_WAITING);
		case 5 : return GetResString(IDS_DOWNLOADING);
		case 6 : return GetResString(IDS_ERRORLIKE);
		case 7 : return GetResString(IDS_PAUSED);
		case 8 : return GetResString(IDS_SEENCOMPL);
		case 10 : return GetResString(IDS_VIDEO);
		case 11 : return GetResString(IDS_AUDIO);
		case 12 : return GetResString(IDS_SEARCH_ARC);
		case 13 : return GetResString(IDS_SEARCH_CDIMG);
		case 14 : return GetResString(IDS_SEARCH_DOC);
		case 15 : return GetResString(IDS_SEARCH_PICS);
		case 16 : return GetResString(IDS_SEARCH_PRG);
//		case 18 : return GetResString(IDS_REGEXPRESSION);
	}
	return _T("?");
}*/

void CTransferWnd::OnBnClickedChangeView()
{
	switch (m_dwShowListIDC)
	{
		case IDC_DOWNLOADLIST:
			ShowList(IDC_UPLOADLIST);
			break;
		case IDC_UPLOADLIST:
			ShowList(IDC_DOWNLOADCLIENTS);
			break;
		case IDC_DOWNLOADCLIENTS:
			if (!thePrefs.IsQueueListDisabled()){
				ShowList(IDC_QUEUELIST);
				break;
			}
		case IDC_QUEUELIST:
			if (!thePrefs.IsKnownClientListDisabled()){
				ShowList(IDC_CLIENTLIST);
				break;
			}
		case IDC_CLIENTLIST:
			ShowSplitWindow();
			break;
		case IDC_UPLOADLIST + IDC_DOWNLOADLIST:
			ShowList(IDC_DOWNLOADLIST);
			break;
	}
}

void CTransferWnd::SetWnd1Icon(EWnd1Icon iIcon)
{
	TBBUTTONINFO tbbi = {0};
	tbbi.cbSize = sizeof tbbi;
	tbbi.dwMask = TBIF_IMAGE;
	tbbi.iImage = iIcon;
	m_btnWnd1->SetButtonInfo(GetWindowLong(*m_btnWnd1, GWL_ID), &tbbi);
}

void CTransferWnd::SetWnd2Icon(EWnd2Icon iIcon)
{
	TBBUTTONINFO tbbi = {0};
	tbbi.cbSize = sizeof tbbi;
	tbbi.dwMask = TBIF_IMAGE;
	tbbi.iImage = iIcon;
	m_btnWnd2->SetButtonInfo(GetWindowLong(*m_btnWnd2, GWL_ID), &tbbi);
}

void CTransferWnd::ShowList(uint32 dwListIDC)
{
	CRect rcWnd;
	GetWindowRect(rcWnd);
	ScreenToClient(rcWnd);

	CRect rcDown;
	GetDlgItem(dwListIDC)->GetWindowRect(rcDown);
	ScreenToClient(rcDown);
	rcDown.bottom = rcWnd.bottom - 20;
	rcDown.top = 28;
	m_wndSplitter.DestroyWindow();
	RemoveAnchor(dwListIDC);
	m_btnWnd2->ShowWindow(SW_HIDE);

	m_dwShowListIDC = dwListIDC;
	uploadlistctrl.ShowWindow((m_dwShowListIDC == IDC_UPLOADLIST) ? SW_SHOW : SW_HIDE);
	queuelistctrl.ShowWindow((m_dwShowListIDC == IDC_QUEUELIST) ? SW_SHOW : SW_HIDE);
	downloadclientsctrl.ShowWindow((m_dwShowListIDC == IDC_DOWNLOADCLIENTS) ? SW_SHOW : SW_HIDE);
	clientlistctrl.ShowWindow((m_dwShowListIDC == IDC_CLIENTLIST) ? SW_SHOW : SW_HIDE);
	downloadlistctrl.ShowWindow((m_dwShowListIDC == IDC_DOWNLOADLIST) ? SW_SHOW : SW_HIDE);
	m_dlTab.ShowWindow((m_dwShowListIDC == IDC_DOWNLOADLIST) ? SW_SHOW : SW_HIDE);

	GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow((m_dwShowListIDC == IDC_QUEUELIST) ? SW_SHOW : SW_HIDE);

	switch (dwListIDC)
	{
		case IDC_DOWNLOADLIST:
			downloadlistctrl.MoveWindow(rcDown);
			downloadlistctrl.ShowFilesCount();
			m_btnWnd1->CheckButton(MP_VIEW1_DOWNLOADS);
			SetWnd1Icon(w1iDownloadFiles);
			thePrefs.SetTransferWnd1(1);
			break;
		case IDC_UPLOADLIST:
			uploadlistctrl.MoveWindow(rcDown);
			UpdateListCount(wnd2Uploading);
			m_btnWnd1->CheckButton(MP_VIEW1_UPLOADING);
			SetWnd1Icon(w1iUploading);
			thePrefs.SetTransferWnd1(2);
			break;
		case IDC_QUEUELIST:
			queuelistctrl.MoveWindow(rcDown);
			UpdateListCount(wnd2OnQueue);
			m_btnWnd1->CheckButton(MP_VIEW1_ONQUEUE);
			SetWnd1Icon(w1iOnQueue);
			thePrefs.SetTransferWnd1(3);
			break;
		case IDC_DOWNLOADCLIENTS:
			downloadclientsctrl.MoveWindow(rcDown);
			UpdateListCount(wnd2Downloading);
			m_btnWnd1->CheckButton(MP_VIEW1_DOWNLOADING);
			SetWnd1Icon(w1iDownloading);
			thePrefs.SetTransferWnd1(4);
			break;
		case IDC_CLIENTLIST:
			clientlistctrl.MoveWindow(rcDown);
			UpdateListCount(wnd2Clients);
			m_btnWnd1->CheckButton(MP_VIEW1_CLIENTS);
			SetWnd1Icon(w1iClientsKnown);
			thePrefs.SetTransferWnd1(5);
			break;
		default:
			ASSERT(0);
	}
	AddAnchor(dwListIDC, TOP_LEFT, BOTTOM_RIGHT);
}

void CTransferWnd::ShowSplitWindow(bool bReDraw) 
{
	thePrefs.SetTransferWnd1(0);
	m_dlTab.ShowWindow(SW_SHOW);
	if (!bReDraw && m_dwShowListIDC == IDC_DOWNLOADLIST + IDC_UPLOADLIST)
		return;

	m_btnWnd1->CheckButton(MP_VIEW1_SPLIT_WINDOW);
	SetWnd1Icon(w1iDownloadFiles);

	CRect rcWnd;
	GetWindowRect(rcWnd);
	ScreenToClient(rcWnd);

	LONG splitpos = (thePrefs.GetSplitterbarPosition() * rcWnd.Height()) / 100;

	// do some more magic, don't ask -- just fix it..
	if (bReDraw || m_dwShowListIDC != 0 && m_dwShowListIDC != IDC_DOWNLOADLIST + IDC_UPLOADLIST)
		splitpos += 10;

	CRect rcDown;
	downloadlistctrl.GetWindowRect(rcDown);
	ScreenToClient(rcDown);
	rcDown.bottom = splitpos - 5; // Magic constant '5'..
	downloadlistctrl.MoveWindow(rcDown);

	uploadlistctrl.GetWindowRect(rcDown);
	ScreenToClient(rcDown);
	rcDown.right = rcWnd.right - 7;
	rcDown.bottom = rcWnd.bottom - 20;
	rcDown.top = splitpos + 20;
	uploadlistctrl.MoveWindow(rcDown);

	queuelistctrl.GetWindowRect(rcDown);
	ScreenToClient(rcDown);
	rcDown.right = rcWnd.right - 7;
	rcDown.bottom = rcWnd.bottom - 20;
	rcDown.top = splitpos + 20;
	queuelistctrl.MoveWindow(rcDown);

	clientlistctrl.GetWindowRect(rcDown);
	ScreenToClient(rcDown);
	rcDown.right = rcWnd.right - 7;
	rcDown.bottom = rcWnd.bottom - 20;
	rcDown.top = splitpos + 20;
	clientlistctrl.MoveWindow(rcDown);

	downloadclientsctrl.GetWindowRect(rcDown);
	ScreenToClient(rcDown);
	rcDown.right = rcWnd.right - 7;
	rcDown.bottom = rcWnd.bottom - 20;
	rcDown.top = splitpos + 20;
	downloadclientsctrl.MoveWindow(rcDown);

	CRect rcBtn2;
	m_btnWnd2->GetWindowRect(rcBtn2);
	ScreenToClient(rcBtn2);
	CRect rcSpl;
	rcSpl.left = rcBtn2.right + 8;
	rcSpl.right = rcDown.right;
	rcSpl.top = splitpos + WND_SPLITTER_YOFF;
	rcSpl.bottom = rcSpl.top + WND_SPLITTER_HEIGHT;
	if (!m_wndSplitter)
		m_wndSplitter.Create(WS_CHILD | WS_VISIBLE, rcSpl, this, IDC_SPLITTER);
	else
		m_wndSplitter.MoveWindow(rcSpl, TRUE);
	DoResize(0);

	m_dwShowListIDC = IDC_DOWNLOADLIST + IDC_UPLOADLIST;
	downloadlistctrl.ShowFilesCount();
	m_btnWnd2->ShowWindow(SW_SHOW);

	RemoveAnchor(IDC_DOWNLOADLIST);
	RemoveAnchor(IDC_UPLOADLIST);
	RemoveAnchor(IDC_QUEUELIST);
	RemoveAnchor(IDC_DOWNLOADCLIENTS);
	RemoveAnchor(IDC_CLIENTLIST);

	AddAnchor(IDC_DOWNLOADLIST, TOP_LEFT, CSize(100, thePrefs.GetSplitterbarPosition()));
	AddAnchor(IDC_UPLOADLIST, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
	AddAnchor(IDC_QUEUELIST, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
	AddAnchor(IDC_CLIENTLIST, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
	AddAnchor(IDC_DOWNLOADCLIENTS, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);

	downloadlistctrl.ShowWindow(SW_SHOW);
	uploadlistctrl.ShowWindow((m_uWnd2 == wnd2Uploading) ? SW_SHOW : SW_HIDE);
	queuelistctrl.ShowWindow((m_uWnd2 == wnd2OnQueue) ? SW_SHOW : SW_HIDE);
	downloadclientsctrl.ShowWindow((m_uWnd2 == wnd2Downloading) ? SW_SHOW : SW_HIDE);
	clientlistctrl.ShowWindow((m_uWnd2 == wnd2Clients) ? SW_SHOW : SW_HIDE);

	GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow((m_uWnd2 == wnd2OnQueue) ? SW_SHOW : SW_HIDE);

	UpdateListCount(m_uWnd2);
}

void CTransferWnd::OnDisableList()
{
	bool bSwitchList = false;
	if (thePrefs.m_bDisableKnownClientList)
	{
		clientlistctrl.DeleteAllItems();
		if (m_uWnd2 == wnd2Clients)
			bSwitchList = true;
	}
	if (thePrefs.m_bDisableQueueList)
	{
		queuelistctrl.DeleteAllItems();
		if (m_uWnd2 == wnd2OnQueue)
			bSwitchList = true;
	}
	if (bSwitchList)
		SwitchUploadList();
}

void CTransferWnd::OnWnd1BtnDropDown(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	CTitleMenu menu;
	menu.CreatePopupMenu();
	menu.EnableIcons();

	menu.AppendMenu(MF_STRING | (m_dwShowListIDC == IDC_DOWNLOADLIST + IDC_UPLOADLIST ? MF_GRAYED : 0), MP_VIEW1_SPLIT_WINDOW, GetResString(IDS_SPLIT_WINDOW), _T("SplitWindow"));
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING | (m_dwShowListIDC == IDC_DOWNLOADLIST ? MF_GRAYED : 0), MP_VIEW1_DOWNLOADS, GetResString(IDS_TW_DOWNLOADS), _T("DownloadFiles"));
	menu.AppendMenu(MF_STRING | (m_dwShowListIDC == IDC_UPLOADLIST ? MF_GRAYED : 0), MP_VIEW1_UPLOADING, GetResString(IDS_UPLOADING), _T("Upload"));
	menu.AppendMenu(MF_STRING | (m_dwShowListIDC == IDC_DOWNLOADCLIENTS ? MF_GRAYED : 0), MP_VIEW1_DOWNLOADING, GetResString(IDS_DOWNLOADING), _T("Download"));
	if (!thePrefs.IsQueueListDisabled())
		menu.AppendMenu(MF_STRING | (m_dwShowListIDC == IDC_QUEUELIST ? MF_GRAYED : 0), MP_VIEW1_ONQUEUE, GetResString(IDS_ONQUEUE), _T("ClientsOnQueue"));
	if (!thePrefs.IsKnownClientListDisabled())
		menu.AppendMenu(MF_STRING | (m_dwShowListIDC == IDC_CLIENTLIST ? MF_GRAYED : 0), MP_VIEW1_CLIENTS, GetResString(IDS_CLIENTLIST), _T("ClientsKnown"));

	CRect rc;
	m_btnWnd1->GetWindowRect(&rc);
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rc.left, rc.bottom, this);
}

void CTransferWnd::OnWnd2BtnDropDown(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	CTitleMenu menu;
	menu.CreatePopupMenu();
	menu.EnableIcons();

	menu.AppendMenu(MF_STRING | (m_uWnd2 == wnd2Uploading ? MF_GRAYED : 0), MP_VIEW2_UPLOADING, GetResString(IDS_UPLOADING), _T("Upload"));
	menu.AppendMenu(MF_STRING | (m_uWnd2 == wnd2Downloading ? MF_GRAYED : 0), MP_VIEW2_DOWNLOADING, GetResString(IDS_DOWNLOADING), _T("Download"));
	if (!thePrefs.IsQueueListDisabled())
		menu.AppendMenu(MF_STRING | (m_uWnd2 == wnd2OnQueue ? MF_GRAYED : 0), MP_VIEW2_ONQUEUE, GetResString(IDS_ONQUEUE), _T("ClientsOnQueue"));
	if (!thePrefs.IsKnownClientListDisabled())
		menu.AppendMenu(MF_STRING | (m_uWnd2 == wnd2Clients ? MF_GRAYED : 0), MP_VIEW2_CLIENTS, GetResString(IDS_CLIENTLIST), _T("ClientsKnown"));

	CRect rc;
	m_btnWnd2->GetWindowRect(&rc);
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rc.left, rc.bottom, this);
}

// NEO: NTB - [NeoToolbarButtons] -- Xanatos -->
CString GetNeoCmdStr(UINT Command);
void CTransferWnd::OnWndNBtnDropDown(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	//CTitleMenu menu;
	CMenuXP menu; // NEO: NMX - [NeoMenuXP]
	menu.CreatePopupMenu();
	menu.AddMenuTitle(NULL, true);
	menu.EnableIcons();

	for (int i=0; i<NeoPrefs.GetNeoToolbarButtonCount(); i++)
	{
		if(NeoPrefs.GetNeoToolbarButton(i) == (INST_OTHER | (INST_SEP << 16)))
			menu.AppendMenu(MF_STRING|MF_SEPARATOR, MP_VIEWN_0+i);
		else
			menu.AppendMenu(MF_STRING, MP_VIEWN_0+i, GetNeoCmdStr(NeoPrefs.GetNeoToolbarButton(i)), GetNeoCmdIco(NeoPrefs.GetNeoToolbarButton(i)));
	}

	CRect rc;
	m_btnWndN->GetWindowRect(&rc);
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rc.left, rc.bottom, this);
	VERIFY( menu.DestroyMenu() ); // NEO: FIX - [DestroyMenu]
}
// NEO: NTB END <-- Xanatos --

void CTransferWnd::ResetTransToolbar(bool bShowToolbar, bool bResetLists)
{
	if (m_btnWnd1->m_hWnd)
		RemoveAnchor(*m_btnWnd1);
	if (m_btnWnd2->m_hWnd)
		RemoveAnchor(*m_btnWnd2);

	CRect rcBtn1;
	rcBtn1.top = 5;
	rcBtn1.left = WND1_BUTTON_XOFF;
	rcBtn1.right = rcBtn1.left + WND1_BUTTON_WIDTH + (bShowToolbar ? WND1_NUM_BUTTONS*DFLT_TOOLBAR_BTN_WIDTH : 0);
	rcBtn1.bottom = rcBtn1.top + WND1_BUTTON_HEIGHT;
	m_btnWnd1->Init(!bShowToolbar);
	m_btnWnd1->MoveWindow(&rcBtn1);
	SetWnd1Icons();

	CRect rcBtn2;
	m_btnWnd2->GetWindowRect(rcBtn2);
	ScreenToClient(rcBtn2);
	rcBtn2.top = rcBtn2.top;
	rcBtn2.left = WND2_BUTTON_XOFF;
	rcBtn2.right = rcBtn2.left + WND2_BUTTON_WIDTH + (bShowToolbar ? WND2_NUM_BUTTONS*DFLT_TOOLBAR_BTN_WIDTH : 0);
	rcBtn2.bottom = rcBtn2.top + WND2_BUTTON_HEIGHT;
	m_btnWnd2->Init(!bShowToolbar);
	m_btnWnd2->MoveWindow(&rcBtn2);
	SetWnd2Icons();

	if (bShowToolbar)
	{
		m_btnWnd1->ModifyStyle(0, TBSTYLE_TOOLTIPS);
		m_btnWnd1->SetExtendedStyle(m_btnWnd1->GetExtendedStyle() | TBSTYLE_EX_MIXEDBUTTONS);

		TBBUTTON atb1[1+WND1_NUM_BUTTONS] = {0};
		atb1[0].iBitmap = w1iDownloadFiles;
		atb1[0].idCommand = IDC_DOWNLOAD_ICO;
		atb1[0].fsState = TBSTATE_ENABLED;
		atb1[0].fsStyle = BTNS_BUTTON | BTNS_SHOWTEXT;
		atb1[0].iString = -1;

		atb1[1].iBitmap = w1iSplitWindow;
		atb1[1].idCommand = MP_VIEW1_SPLIT_WINDOW;
		atb1[1].fsState = TBSTATE_ENABLED;
		atb1[1].fsStyle = BTNS_BUTTON | BTNS_CHECKGROUP | BTNS_AUTOSIZE;
		atb1[1].iString = -1;

		atb1[2].iBitmap = w1iDownloadFiles;
		atb1[2].idCommand = MP_VIEW1_DOWNLOADS;
		atb1[2].fsState = TBSTATE_ENABLED;
		atb1[2].fsStyle = BTNS_BUTTON | BTNS_CHECKGROUP | BTNS_AUTOSIZE;
		atb1[2].iString = -1;

		atb1[3].iBitmap = w1iUploading;
		atb1[3].idCommand = MP_VIEW1_UPLOADING;
		atb1[3].fsState = TBSTATE_ENABLED;
		atb1[3].fsStyle = BTNS_BUTTON | BTNS_CHECKGROUP | BTNS_AUTOSIZE;
		atb1[3].iString = -1;

		atb1[4].iBitmap = w1iDownloading;
		atb1[4].idCommand = MP_VIEW1_DOWNLOADING;
		atb1[4].fsState = TBSTATE_ENABLED;
		atb1[4].fsStyle = BTNS_BUTTON | BTNS_CHECKGROUP | BTNS_AUTOSIZE;
		atb1[4].iString = -1;

		atb1[5].iBitmap = w1iOnQueue;
		atb1[5].idCommand = MP_VIEW1_ONQUEUE;
		atb1[5].fsState = thePrefs.IsQueueListDisabled() ? 0 : TBSTATE_ENABLED;
		atb1[5].fsStyle = BTNS_BUTTON | BTNS_CHECKGROUP | BTNS_AUTOSIZE;
		atb1[5].iString = -1;

		atb1[6].iBitmap = w1iClientsKnown;
		atb1[6].idCommand = MP_VIEW1_CLIENTS;
		atb1[6].fsState = thePrefs.IsKnownClientListDisabled() ? 0 : TBSTATE_ENABLED;
		atb1[6].fsStyle = BTNS_BUTTON | BTNS_CHECKGROUP | BTNS_AUTOSIZE;
		atb1[6].iString = -1;
		m_btnWnd1->AddButtons(_countof(atb1), atb1);

		TBBUTTONINFO tbbi = {0};
		tbbi.cbSize = sizeof tbbi;
		tbbi.dwMask = TBIF_SIZE | TBIF_BYINDEX;
		tbbi.cx = WND1_BUTTON_WIDTH;
		m_btnWnd1->SetButtonInfo(0, &tbbi);

		// 'GetMaxSize' does not work properly under:
		//	- Win98SE with COMCTL32 v5.80
		//	- Win2000 with COMCTL32 v5.81 
		// The value returned by 'GetMaxSize' is just couple of pixels too small so that the 
		// last toolbar button is nearly not visible at all.
		// So, to circumvent such problems, the toolbar control should be created right with
		// the needed size so that we do not really need to call the 'GetMaxSize' function.
		// Although it would be better to call it to adapt for system metrics basically.
		if (theApp.m_ullComCtrlVer > MAKEDLLVERULL(5,81,0,0))
		{
			CSize size;
			m_btnWnd1->GetMaxSize(&size);
			CRect rc;
			m_btnWnd1->GetWindowRect(&rc);
			ScreenToClient(&rc);
			// the with of the toolbar should already match the needed size (see comment above)
			ASSERT( size.cx == rc.Width() );
			m_btnWnd1->MoveWindow(rc.left, rc.top, size.cx, rc.Height());
		}
/*---*/
		m_btnWnd2->ModifyStyle(0, TBSTYLE_TOOLTIPS);
		m_btnWnd2->SetExtendedStyle(m_btnWnd2->GetExtendedStyle() | TBSTYLE_EX_MIXEDBUTTONS);

		TBBUTTON atb2[1+WND2_NUM_BUTTONS] = {0};
		atb2[0].iBitmap = w2iUploading;
		atb2[0].idCommand = IDC_UPLOAD_ICO;
		atb2[0].fsState = TBSTATE_ENABLED;
		atb2[0].fsStyle = BTNS_BUTTON | BTNS_SHOWTEXT;
		atb2[0].iString = -1;

		atb2[1].iBitmap = w2iUploading;
		atb2[1].idCommand = MP_VIEW2_UPLOADING;
		atb2[1].fsState = TBSTATE_ENABLED;
		atb2[1].fsStyle = BTNS_BUTTON | BTNS_CHECKGROUP | BTNS_AUTOSIZE;
		atb2[1].iString = -1;

		atb2[2].iBitmap = w2iDownloading;
		atb2[2].idCommand = MP_VIEW2_DOWNLOADING;
		atb2[2].fsState = TBSTATE_ENABLED;
		atb2[2].fsStyle = BTNS_BUTTON | BTNS_CHECKGROUP | BTNS_AUTOSIZE;
		atb2[2].iString = -1;

		atb2[3].iBitmap = w2iOnQueue;
		atb2[3].idCommand = MP_VIEW2_ONQUEUE;
		atb2[3].fsState = thePrefs.IsQueueListDisabled() ? 0 : TBSTATE_ENABLED;
		atb2[3].fsStyle = BTNS_BUTTON | BTNS_CHECKGROUP | BTNS_AUTOSIZE;
		atb2[3].iString = -1;

		atb2[4].iBitmap = w2iClientsKnown;
		atb2[4].idCommand = MP_VIEW2_CLIENTS;
		atb2[4].fsState = thePrefs.IsKnownClientListDisabled() ? 0 : TBSTATE_ENABLED;
		atb2[4].fsStyle = BTNS_BUTTON | BTNS_CHECKGROUP | BTNS_AUTOSIZE;
		atb2[4].iString = -1;
		m_btnWnd2->AddButtons(_countof(atb2), atb2);

		memset(&tbbi, 0, sizeof(tbbi));
		tbbi.cbSize = sizeof tbbi;
		tbbi.dwMask = TBIF_SIZE | TBIF_BYINDEX;
		tbbi.cx = WND2_BUTTON_WIDTH;
		m_btnWnd2->SetButtonInfo(0, &tbbi);

		// 'GetMaxSize' does not work properly under:
		//	- Win98SE with COMCTL32 v5.80
		//	- Win2000 with COMCTL32 v5.81 
		// The value returned by 'GetMaxSize' is just couple of pixels too small so that the 
		// last toolbar button is nearly not visible at all.
		// So, to circumvent such problems, the toolbar control should be created right with
		// the needed size so that we do not really need to call the 'GetMaxSize' function.
		// Although it would be better to call it to adapt for system metrics basically.
		if (theApp.m_ullComCtrlVer > MAKEDLLVERULL(5,81,0,0))
		{
			CSize size;
			m_btnWnd2->GetMaxSize(&size);
			CRect rc;
			m_btnWnd2->GetWindowRect(&rc);
			ScreenToClient(&rc);
			// the with of the toolbar should already match the needed size (see comment above)
			ASSERT( size.cx == rc.Width() );
			m_btnWnd2->MoveWindow(rc.left, rc.top, size.cx, rc.Height());
		}
	}
	else
	{
		m_btnWnd1->ModifyStyle(TBSTYLE_TOOLTIPS, 0);
		m_btnWnd1->SetExtendedStyle(m_btnWnd1->GetExtendedStyle() & ~TBSTYLE_EX_MIXEDBUTTONS);
		m_btnWnd1->RecalcLayout(true);

		m_btnWnd2->ModifyStyle(TBSTYLE_TOOLTIPS, 0);
		m_btnWnd2->SetExtendedStyle(m_btnWnd2->GetExtendedStyle() & ~TBSTYLE_EX_MIXEDBUTTONS);
		m_btnWnd2->RecalcLayout(true);
	}

	AddAnchor(*m_btnWnd1, TOP_LEFT);
	AddAnchor(*m_btnWnd2, CSize(0, thePrefs.GetSplitterbarPosition()));

	// NEO: NTB - [NeoToolbarButtons] -- Xanatos -->
	UINT uShowNeoToolbar = NeoPrefs.UseNeoToolbar(); 

	if (m_btnWndN->m_hWnd)
		RemoveAnchor(*m_btnWndN);

	if(uShowNeoToolbar)
	{
		m_btnWndN->ShowWindow(SW_SHOW);

		CRect& rc = rcBtn1;
		rc.top = 5;
		rc.left = WNDN_BUTTON_XOFF + rc.right;
		rc.right = rc.left + WNDN_BUTTON_WIDTH + (uShowNeoToolbar == TRUE ? NUMN_WIN_BUTTONS*DFLT_TOOLBAR_BTN_WIDTH : 0);
		rc.bottom = rc.top + WNDN_BUTTON_HEIGHT;
		m_btnWndN->Init(uShowNeoToolbar!=TRUE);
		m_btnWndN->MoveWindow(&rc);
		SetWndNIcons();

		const int ButtonCount = NeoPrefs.GetNeoToolbarButtonCount();
		if (uShowNeoToolbar == TRUE && ButtonCount > 0)
		{
			m_btnWndN->ModifyStyle(0, TBSTYLE_TOOLTIPS);
			m_btnWndN->SetExtendedStyle(m_btnWndN->GetExtendedStyle() | TBSTYLE_EX_MIXEDBUTTONS);

			TBBUTTON* atb = new TBBUTTON[ButtonCount];

			for (int i=0; i<ButtonCount; i++)
			{
				atb[i].iBitmap = i;
				atb[i].idCommand = MP_VIEWN_0+i;
				atb[i].fsState = TBSTATE_ENABLED;
				if(NeoPrefs.GetNeoToolbarButton(i) == (INST_OTHER | (INST_SEP << 16)))
					atb[i].fsStyle = BTNS_BUTTON | BTNS_SEP;
				else
					atb[i].fsStyle = BTNS_BUTTON | BTNS_AUTOSIZE;
				atb[i].iString = -1;
			}

			m_btnWndN->AddButtons(ButtonCount, atb);

			delete [] atb;

			TBBUTTONINFO tbbi = {0};
			tbbi.cbSize = sizeof tbbi;
			tbbi.dwMask = TBIF_SIZE | TBIF_BYINDEX;
			tbbi.cx = WNDN_BUTTON_WIDTH;
			m_btnWndN->SetButtonInfo(0, &tbbi);

			for (int i=0; i<ButtonCount; i++)
			{
				m_btnWndN->SetBtnText(MP_VIEWN_0+i,GetNeoCmdStr(NeoPrefs.GetNeoToolbarButton(i)));
				if(NeoPrefs.GetNeoToolbarButton(i) == (INST_OTHER | (INST_SEP << 16)))
					m_btnWndN->SetBtnWidth(MP_VIEWN_0+i,3);
			}

			CSize size;
			m_btnWndN->GetMaxSize(&size);
			m_btnWndN->GetWindowRect(&rc);
			ScreenToClient(&rc);
			m_btnWndN->MoveWindow(rc.left, rc.top, size.cx, rc.Height());
		}
		else
		{
			m_btnWndN->ModifyStyle(TBSTYLE_TOOLTIPS, 0);
			m_btnWndN->SetExtendedStyle(m_btnWndN->GetExtendedStyle() & ~TBSTYLE_EX_MIXEDBUTTONS);
			m_btnWndN->RecalcLayout(true);

			TBBUTTONINFO tbbi = {0};
			tbbi.cbSize = sizeof tbbi;
			tbbi.dwMask = TBIF_IMAGE;
			tbbi.iImage = 0;
			m_btnWndN->SetButtonInfo(GetWindowLong(*m_btnWndN, GWL_ID), &tbbi);
		}
	}else
		m_btnWndN->ShowWindow(SW_HIDE);

	AddAnchor(*m_btnWndN, TOP_LEFT);
	// NEO: NTB END <-- Xanatos --

	if (bResetLists)
	{
		LocalizeToolbars();
		ShowSplitWindow(true);
		VerifyCatTabSize();
		ShowWnd2(m_uWnd2);
	}
}

BOOL CTransferWnd::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	theApp.ShowHelp(eMule_FAQ_GUI_Transfers);
	return TRUE;
}

HBRUSH CTransferWnd::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = theApp.emuledlg->GetCtlColor(pDC, pWnd, nCtlColor);
	if (hbr)
		return hbr;
	return __super::OnCtlColor(pDC, pWnd, nCtlColor);
}

// NEO: CCF - [ColloredCategoryFlags] -- Xanatos -->
HICON CTransferWnd::GetColorIcon(COLORREF color, UINT prio)
{
	HBITMAP newBmp = NULL;
	ICONINFO ii;
	switch(prio)
	{
	case PR_HIGH:
		GetIconInfo(m_hIconCatTabHi, &ii);
		break;
	case PR_LOW:
		GetIconInfo(m_hIconCatTabLo, &ii);
		break;
	default:
		GetIconInfo(m_hIconCatTab, &ii);
	}


	HDC BufferDC=CreateCompatibleDC(NULL);
	if(BufferDC)
	{
		HGDIOBJ PreviousBufferObject = SelectObject(BufferDC, ii.hbmColor);

		HDC DirectDC = CreateCompatibleDC(NULL);
		if(DirectDC)
		{
			BITMAP bm;
			GetObject(ii.hbmColor, sizeof(bm), &bm);

			BITMAPINFO biinfo;
			ZeroMemory(&biinfo,sizeof(BITMAPINFO));
			biinfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
			biinfo.bmiHeader.biWidth=bm.bmWidth;
			biinfo.bmiHeader.biHeight=bm.bmHeight;
			biinfo.bmiHeader.biPlanes=1;
			biinfo.bmiHeader.biBitCount=32;

			DWORD *ptPixels;
			HBITMAP DirectBitmap = CreateDIBSection(DirectDC, &biinfo, DIB_RGB_COLORS, (void **)&ptPixels, NULL, 0);
			if(DirectBitmap)
			{
				HGDIOBJ PreviousObject=SelectObject(DirectDC, DirectBitmap);
				BitBlt(DirectDC,0,0, bm.bmWidth,bm.bmHeight, BufferDC,0,0,SRCCOPY);

				if(NeoPrefs.ShowCategoryFlags() == TRUE && color == RGB(0,0,0))
					color = RGB(255,255,255);

				int iRed = GetRValue(color);
				int iGreen = GetGValue(color);
				int iBlue = GetBValue(color);

				for(int i=((bm.bmWidth*bm.bmHeight)-1);i>=0;i--)
				{
					int g = GetGValue(ptPixels[i]);

					if(g > GetRValue(ptPixels[i]) && g > GetBValue(ptPixels[i]))
					{
						g-=170;
						ptPixels[i] = RGBA(
							max(min(iBlue+g, 255), 0),
							max(min(iGreen+g, 255), 0),
							max(min(iRed+g, 255), 0),
							GetAValue(ptPixels[i]));
					}
				}
				SelectObject(DirectDC,PreviousObject);

				newBmp=DirectBitmap;
			}
			DeleteDC(DirectDC);
		}
		SelectObject(BufferDC,PreviousBufferObject);
		DeleteDC(BufferDC);
	}

	DeleteObject(ii.hbmColor);
	ii.hbmColor = newBmp;
	HICON Icon = CreateIconIndirect(&ii);
	DeleteObject(ii.hbmColor);
	DeleteObject(ii.hbmMask);
	return Icon;
}

void CTransferWnd::UpdateCatTabIcons(bool force)
{
	if(!NeoPrefs.ShowCategoryFlags()){
		if(force)
			m_dlTab.SetImageList(NULL);
		return;
	}

	TCITEM item;
	item.mask = TCIF_IMAGE;
	m_dlTab.SetImageList(NULL);
	m_imagelistCat.DeleteImageList();
	m_imagelistCat.Create(14,16, ILC_COLOR32|ILC_MASK, 0, thePrefs.GetCatCount());
	for(int ix=0; ix < thePrefs.GetCatCount(); ix++)
	{
		HICON Icon = GetColorIcon(thePrefs.GetCategory(ix)->color, thePrefs.GetCategory(ix)->prio);
		item.iImage = m_imagelistCat.Add(Icon);
		VERIFY( DestroyIcon(Icon) );
		m_dlTab.SetItem(ix, &item);
	}
	m_dlTab.SetImageList(&m_imagelistCat);

	VerifyCatTabSize(); // X?
}
// NEO: CCF END <-- Xanatos --

// NEO: NTB - [NeoToolbarButtons] -- Xanatos -->
void CTransferWnd::NeoCommand(uint8 button)
{
	UINT Command = NeoPrefs.GetNeoToolbarButton(button);
	if(Command == 0)
		return;
	NeoCommand((uint8)Command,(uint8)(Command >> 16));
}

void CTransferWnd::NeoCommand(uint8 uNeoCmdL, uint8 uNeoCmdW)
{
	CTypedPtrList<CPtrList, CPartFile*> FileQueue;

	int cat = downloadlistctrl.curTab;
	CTypedPtrList<CPtrList, CPartFile*>* filelist = theApp.downloadqueue->GetFileList();
	POSITION pos= filelist->GetHeadPosition();
	while (pos != 0){
		CPartFile* cur_file = filelist->GetAt(pos);
		if (!cur_file)
			continue;
		if (cur_file->CheckShowItemInGivenCat(cat))
			FileQueue.AddTail(cur_file);
		filelist->GetNext(pos);
	}

	theApp.downloadqueue->ExecuteNeoCommand(FileQueue,uNeoCmdL,uNeoCmdW);
}

CString GetNeoCmdLStr(uint8 uNeoCmdL)
{
	switch(uNeoCmdL)
	{
		case INST_COLLECT: return GetResString(IDS_X_SOURCE_COLLECTING);
		case INST_DROP: return GetResString(IDS_X_DROP);
		case INST_STORAGE: return GetResString(IDS_X_SOURCE_STORAGE);
		case INST_OTHER: return GetResString(IDS_X_MIXED);
		default:
			return _T("");
	}
}

CString GetNeoCmdWStr(uint8 uNeoCmdL, uint8 uNeoCmdW)
{
	switch(uNeoCmdL)
	{
		case INST_COLLECT:
			switch(uNeoCmdW)
			{
				case INST_COLLECT_ALL_SOURCES: return GetResString(IDS_X_COLLECT_ALL_SOURCES);
				case INST_COLLECT_XS_SOURCES: return GetResString(IDS_X_COLLECT_XS_SOURCES);
				case INST_COLLECT_SVR_SOURCES: return GetResString(IDS_X_COLLECT_SVR_SOURCES);
				case INST_COLLECT_KAD_SOURCES: return GetResString(IDS_X_COLLECT_KAD_SOURCES);
				case INST_COLLECT_UDP_SOURCES: return GetResString(IDS_X_COLLECT_UDP_SOURCES);
#ifdef VOODOO // NEO: VOODOOx - [VoodooSourceExchange]
				case INST_COLLECT_VOODOO_SOURCES: return GetResString(IDS_X_COLLECT_VOODOO_SOURCES);
#endif // VOODOO // NEO: VOODOOx END
				case INST_AHL_INCREASE: return GetResString(IDS_X_AHL_INCREASE);
				case INST_AHL_DECREASE: return GetResString(IDS_X_AHL_DECREASE);
				default:
					return _T("");
			}
			break;
		case INST_DROP:
			switch(uNeoCmdW)
			{
				case INST_DROP_NNP: return GetResString(IDS_X_DROP_NNP);
				case INST_DROP_FULLQ: return GetResString(IDS_X_DROP_FULLQ);
				case INST_DROP_HIGHQ: return GetResString(IDS_X_DROP_HIGHQ);
				// NEO: TCR - [TCPConnectionRetry]
				case INST_DROP_WAITINGRETRY: return GetResString(IDS_X_DROP_WAITINGRETRY);
				// NEO: TCR END
				// NEO: XSC - [ExtremeSourceCache]
				case INST_DROP_CACHED: return GetResString(IDS_X_DROP_CACHED);
				// NEO: XSC END
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
				case INST_DROP_LOADED: return GetResString(IDS_X_DROP_LOADED);
#endif // NEO_SS // NEO: NSS END
				case INST_DROP_TOMANY: return GetResString(IDS_X_DROP_TOMANY);
				case INST_DROP_LOW2LOW: return GetResString(IDS_X_DROP_LOW2LOW);
				default:
					return _T("");
			}
			break;

		case INST_STORAGE:
			switch(uNeoCmdW)
			{
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
				case INST_STORAGE_LOAD: return GetResString(IDS_X_LOADSRCMANUALLY);
				case INST_STORAGE_SAVE: return GetResString(IDS_X_SAVESRCMANUALLY);
#endif // NEO_SS // NEO: NSS END
#ifdef NEO_CD // NEO: SFL - [SourceFileList]
				case INST_STORAGE_FIND: return GetResString(IDS_X_FINDSOURCES);
#endif // NEO_CD // NEO: SFL END
				default:
					return _T("");
			}

		case INST_OTHER:
			switch(uNeoCmdW)
			{
				// NEO: PP - [PasswordProtection]
				case INST_OTHER_PROTECT_SHOW: return GetResString(IDS_X_PWPROT_SHOW);
				case INST_OTHER_PROTECT_HIDE: return GetResString(IDS_X_PWPROT_HIDE);
				case INST_OTHER_PROTECT_SET: return GetResString(IDS_X_PWPROT_SET);
				case INST_OTHER_PROTECT_CHANGE: return GetResString(IDS_X_PWPROT_CHANGE);
				case INST_OTHER_PROTECT_UNSET: return GetResString(IDS_X_PWPROT_UNSET);
				// NEO: PP END
				// NEO: FCFG - [FileConfiguration]
				case INST_OTHER_PROPERTIES: return GetResString(IDS_DL_INFO);
				case INST_OTHER_PREFERENCES: return GetResString(IDS_X_FILE_TWEAKS);
				// NEO: FCFG END
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
				case INST_OTHER_VOODOO_LIST: return GetResString(IDS_X_VOODOO_LIST);
#endif // VOODOO // NEO: VOODOO END
				case INST_SEP: return GetResString(IDS_X_SEP);
				default:
					return _T("");
			}
		default:
			return _T("");
	}
}

CString GetNeoCmdStr(UINT Command)
{
	return StrLine(_T("%s: %s"),GetNeoCmdLStr((uint8)Command),GetNeoCmdWStr((uint8)Command,(uint8)(Command >> 16)));
}

CString GetNeoCmdIco(UINT Command)
{
	uint8 uNeoCmdL = (uint8)Command;
	uint8 uNeoCmdW = (uint8)(Command >> 16);

	switch(uNeoCmdL)
	{
		case INST_COLLECT:
			switch(uNeoCmdW)
			{
				case INST_COLLECT_ALL_SOURCES: return _T("SRCALL");
				case INST_COLLECT_XS_SOURCES: return _T("XSSRC");
				case INST_COLLECT_SVR_SOURCES: return _T("SVRSRC");
				case INST_COLLECT_KAD_SOURCES: return _T("KADSRC");
				case INST_COLLECT_UDP_SOURCES: return _T("UDPSRC");
#ifdef VOODOO // NEO: VOODOOx - [VoodooSourceExchange]
				case INST_COLLECT_VOODOO_SOURCES: return _T("SRCVOODOO");
#endif // VOODOO // NEO: VOODOOx END
				case INST_AHL_INCREASE: return _T("AHLINCREASE");
				case INST_AHL_DECREASE: return _T("AHLDECREASE");
				default:
					return _T("NEO_BTN");
			}
			break;
		case INST_DROP:
			switch(uNeoCmdW)
			{
				case INST_DROP_NNP: return _T("DROPNNP");
				case INST_DROP_FULLQ: return _T("DROPFULLQ");
				case INST_DROP_HIGHQ: return _T("DROPHIGHQ");
				// NEO: TCR - [TCPConnectionRetry]
				case INST_DROP_WAITINGRETRY: return _T("DROPWAITINGRETRY");
				// NEO: TCR END
				// NEO: XSC - [ExtremeSourceCache]
				case INST_DROP_CACHED: return _T("DROPCACHED");
				// NEO: XSC END
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
				case INST_DROP_LOADED: return _T("DROPLOADED");
#endif // NEO_SS // NEO: NSS END
				case INST_DROP_TOMANY: return _T("DROPTOMANY");
				case INST_DROP_LOW2LOW: return _T("DROPLOW2LOW");
				default:
					return _T("NEO_BTN");
			}
			break;

		case INST_STORAGE:
			switch(uNeoCmdW)
			{
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
				case INST_STORAGE_LOAD: return _T("LOADSRC");
				case INST_STORAGE_SAVE: return _T("SAVESRC");
#endif // NEO_SS // NEO: NSS END
#ifdef NEO_CD // NEO: SFL - [SourceFileList]
				case INST_STORAGE_FIND: return _T("FINDSOURCES");
#endif // NEO_CD // NEO: SFL END
				default:
					return _T("NEO_BTN");
			}
		case INST_OTHER:
			switch(uNeoCmdW)
			{
				// NEO: PP - [PasswordProtection]
				case INST_OTHER_PROTECT_SHOW: return _T("PWPROT_SHOW");
				case INST_OTHER_PROTECT_HIDE: return _T("PWPROT_HIDE");
				case INST_OTHER_PROTECT_SET: return _T("PWPROT_SET");
				case INST_OTHER_PROTECT_CHANGE: return _T("PWPROT_CHANGE");
				case INST_OTHER_PROTECT_UNSET: return _T("PWPROT_UNSET");
				// NEO: PP END
				// NEO: FCFG - [FileConfiguration]
				case INST_OTHER_PROPERTIES: return _T("FILEINFO");
				case INST_OTHER_PREFERENCES: return _T("FILECONFIG");
				// NEO: FCFG END
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
				case INST_OTHER_VOODOO_LIST: return _T("VOODOOLIST");
#endif // VOODOO // NEO: VOODOO END
				default:
					return _T("NEO_BTN");
			}
		default:
			return _T("NEO_BTN");
	}
}
// NEO: NTB END <-- Xanatos --

// NEO: NMX - [NeoMenuXP] -- Xanatos -->
void CTransferWnd::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	HMENU hMenu = AfxGetThreadState()->m_hTrackingMenu;
	if(CMenu *pMenu = CMenu::FromHandle(hMenu))
		pMenu->MeasureItem(lpMeasureItemStruct);
	
	CResizableDialog::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}
// NEO: NMX END <-- Xanatos --

#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
int CTransferWnd::GetClientImage(CUpDownClient* client)
{
	int iImage = 0;
	switch(client->GetClientSoft())
	{
		case SO_URL:		iImage = DL_SERVER;				break;
		case SO_CDONKEY:	iImage = DL_CLIENT_CDONKEY;		break;
		case SO_AMULE: 		iImage = DL_CLIENT_AMULE;		break;					
		case SO_LPHANT:		iImage = DL_CLIENT_LPHANT;		break;
		case SO_EMULEPLUS:	iImage = DL_CLIENT_EMULEPLUS;	break;
		case SO_HYDRANODE:	iImage = DL_CLIENT_HYDRANODE;	break;
		case SO_TRUSTYFILES:iImage = DL_CLIENT_TRUSTYFILES;	break;
		case SO_XMULE:		iImage = DL_CLIENT_XMULE;		break;
		case SO_SHAREAZA:	iImage = DL_CLIENT_SHAREAZA;	break;
		case SO_EDONKEYHYBRID: iImage = DL_CLIENT_HYBRID;	break;
		case SO_MLDONKEY:	iImage = DL_CLIENT_MLDONKEY;	break;
		case SO_EMULE:
			if(const EModClient Mod = client->GetMod())
				iImage = DL_CLIENT_MOD + ((int)Mod - 1);
			else
				iImage = DL_CLIENT_EMULE;
			break;
		case SO_OLDEMULE:	iImage = DL_CLIENT_OLDEMULE;	break;
		case SO_EDONKEY:	iImage = DL_CLIENT_EDONKEY;		break;
		case SO_UNKNOWN:	iImage = DL_CLIENT_UNKNOWN;		break;
		default:			iImage = DL_CLIENT_UNKNOWN;
	}
	return iImage;
}

void CTransferWnd::UpdateToolTips()
{
	bool pop = true;
	for(int i = UPDOWN_WND; i >= 0; i--)
	{
		int sel = lists_list[i]->GetItemUnderMouse();
		if (sel != -1)
		{
			pop = false;
			if (sel != m_iOldToolTipItem[i])
			{
				m_iOldToolTipItem[i] = sel;
				return;
			}
		}
	}

	if (pop)
		m_ttip.Pop();
}

BOOL CTransferWnd::OnToolTipNotify(UINT /*id*/, NMHDR *pNMH, LRESULT* /*pResult*/)
{
	NM_PPTOOLTIP_DISPLAY * pNotify = (NM_PPTOOLTIP_DISPLAY*)pNMH;
	int control_id = CWnd::FromHandle(pNotify->ti->hWnd)->GetDlgCtrlID();

	if (!control_id) return FALSE;

	switch(control_id)
	{
		case IDC_DLTAB:
		{
			uint8 index = (uint8)GetTabUnderMouse(&CPoint(*pNotify->pt));
			if(index < 0) 
				return FALSE;
			theApp.downloadqueue->GetTipInfoByCat(index, *((CString *)&pNotify->ti->sTooltip));
			return TRUE;
		}
		case IDC_DOWNLOAD_ICO:
		case IDC_UPLOAD_ICO:
		case IDC_DOWNLOADLIST:
		case IDC_DOWNLOADCLIENTS:
		case IDC_UPLOADLIST:
		case IDC_QUEUELIST:
		case IDC_CLIENTLIST:
		{
			DWORD_PTR pItem;
			try
			{
				CMuleListCtrl* list = (CMuleListCtrl*)GetDlgItem(control_id);

				if (list->GetItemCount() < 1)
					return FALSE;

				int sel = list->GetItemUnderMouse();
				if (sel < 0)
					return FALSE;

				pItem = list->GetItemData(sel);
				if (!pItem) return FALSE;
			}
			catch(...)
			{
				return FALSE;
			}

			CString info;
			switch(control_id)
			{
				case IDC_DOWNLOADLIST:
				{
					// build info text and display it
					CtrlItem_Struct* pContent = (CtrlItem_Struct*)pItem;
					if (!pContent->value) return FALSE;
					if(pContent->type == FILE_TYPE)	// for downloading files
					{
						CPartFile* partfile = (CPartFile*)pContent->value;

						partfile->GetTooltipFileInfo(info);

						SHFILEINFO shfi;
						ZeroMemory(&shfi, sizeof(shfi));
						SHGetFileInfo(partfile->GetFileName(), FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi), SHGFI_ICON|SHGFI_USEFILEATTRIBUTES);
						pNotify->ti->hIcon = shfi.hIcon;

						downloadlistctrl.SetFocus();
					}
					else if (pContent->type == AVAILABLE_SOURCE || pContent->type == UNAVAILABLE_SOURCE) // for sources
					{
						CUpDownClient* client = (CUpDownClient*)pContent->value;

						pNotify->ti->hIcon = m_ImageList.ExtractIcon(GetClientImage(client));

						client->GetTooltipDownloadInfo(info, pContent->type == UNAVAILABLE_SOURCE, pContent->owner);
					}

					break;
				}
				case IDC_DOWNLOADCLIENTS:
					{
						((CUpDownClient*)pItem)->GetTooltipDownloadInfo(info, false, NULL);
					}
					break;
				case IDC_UPLOADLIST:
					((CUpDownClient*)pItem)->GetTooltipUploadInfo(info);
					break;
				case IDC_QUEUELIST:
					((CUpDownClient*)pItem)->GetTooltipQueueInfo(info);
					break;
				case IDC_CLIENTLIST:
					((CUpDownClient*)pItem)->GetTooltipClientInfo(info);
					break;
			}

			if (control_id != IDC_DOWNLOADLIST)
				pNotify->ti->hIcon = m_ImageList.ExtractIcon(GetClientImage(((CUpDownClient*)pItem)));

			SetDlgItemFocus(control_id);
			pNotify->ti->sTooltip = info;

			return TRUE;
		}
		default:
			if(pNotify->ti->hIcon)
				pNotify->ti->hIcon = DuplicateIcon(AfxGetInstanceHandle(), pNotify->ti->hIcon);
			return TRUE;
	}
}

void CTransferWnd::SetDlgItemFocus(int nID)
{
	GetDlgItem(nID)->SetFocus();
}

void CTransferWnd::SetTTDelay()
{
	m_ttip.SetDelayTime(TTDT_AUTOPOP, 20000);
	m_ttip.SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay() * 500);
	m_tabtip.SetDelayTime(TTDT_AUTOPOP, 40000);
	m_tabtip.SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay() * 500);
	//m_othertips.SetDelayTime(TTDT_AUTOPOP, 20000);
	//m_othertips.SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay() * 500);
}
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --