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
#include "TransferDlg.h"
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
#include "UploadBandwidthThrottler.h" //Xman Xtreme upload
#include "Statistics.h"// X: [SFH] - [Show IP Filter Hits]

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	DFLT_TOOLBAR_BTN_WIDTH	24

#define	WND_SPLITTER_YOFF	10
#define	WND_SPLITTER_HEIGHT	4

#define	WND1_BUTTON_XOFF	2
#define	WND1_BUTTON_WIDTH	220 //170 Xman see all sources
#define	WND1_BUTTON_HEIGHT	22	// don't set the height to something different than 22 unless you know exactly what you are doing!
#define	WND1_NUM_BUTTONS	6

#define	WND2_BUTTON_XOFF	2
#define	WND2_BUTTON_WIDTH	220 //170 //Xman Uploadhealth
#define	WND2_BUTTON_HEIGHT	22	// don't set the height to something different than 22 unless you know exactly what you are doing!
#define	WND2_NUM_BUTTONS	4

// CTransferWnd dialog

IMPLEMENT_DYNCREATE(CTransferWnd, CResizableFormView)

BEGIN_MESSAGE_MAP(CTransferWnd, CResizableFormView)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_DOWNLOADLIST, OnLvnBeginDragDownloadList)
	ON_NOTIFY(LVN_HOTTRACK, IDC_CLIENTLIST , OnHoverUploadList)
	ON_NOTIFY(LVN_HOTTRACK, IDC_DOWNLOADLIST, OnHoverDownloadList)
	ON_NOTIFY(LVN_HOTTRACK, IDC_QUEUELIST, OnHoverUploadList)
	ON_NOTIFY(LVN_HOTTRACK, IDC_UPLOADLIST, OnHoverUploadList)
	ON_NOTIFY(LVN_KEYDOWN, IDC_DOWNLOADLIST, OnLvnKeydownDownloadList)
	ON_NOTIFY(NM_RCLICK, IDC_DLTAB, OnNmRClickDltab)
	ON_NOTIFY(TBN_DROPDOWN, IDC_DOWNLOAD_ICO, OnWnd1BtnDropDown)
	ON_NOTIFY(TBN_DROPDOWN, IDC_UPLOAD_ICO, OnWnd2BtnDropDown)
	ON_NOTIFY(TCN_SELCHANGE, IDC_DLTAB, OnTcnSelchangeDltab)
	ON_NOTIFY(UM_SPN_SIZED, IDC_SPLITTER, OnSplitterMoved)
	ON_NOTIFY(UM_TABMOVED, IDC_DLTAB, OnTabMovement)
	ON_WM_CTLCOLOR()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SETTINGCHANGE()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_PAINT()
	ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()

CTransferWnd::CTransferWnd(CWnd* /*pParent =NULL*/)
	: CResizableFormView(CTransferWnd::IDD)
{
	m_uWnd2 = wnd2Uploading;
	m_dwShowListIDC = 0;
	m_pLastMousePoint.x = -1;
	m_pLastMousePoint.y = -1;
	m_nLastCatTT = -1;
	m_btnWnd1 = new CDropDownButton;
	m_btnWnd2 = new CDropDownButton;
	m_tooltipCats = new CToolTipCtrlX;
	m_pDragImage = NULL;
	m_bLayoutInited = false;
}

CTransferWnd::~CTransferWnd()
{
	delete m_btnWnd1;
	delete m_btnWnd2;
	delete m_tooltipCats;
	ASSERT( m_pDragImage == NULL );
	delete m_pDragImage;
}

void CTransferWnd::OnInitialUpdate()
{
	CResizableFormView::OnInitialUpdate();
	InitWindowStyles(this);

	uploadlistctrl.Init();
	downloadlistctrl.Init();
	queuelistctrl.Init();
	clientlistctrl.Init();
	downloadclientsctrl.Init();

	AddAnchor(downloadlistctrl, TOP_LEFT, CSize(100, thePrefs.GetSplitterbarPosition()));
	AddAnchor(uploadlistctrl, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
	AddAnchor(queuelistctrl, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
	AddAnchor(clientlistctrl, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
	AddAnchor(downloadclientsctrl, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
	AddAnchor(IDC_QUEUECOUNT, BOTTOM_LEFT);
	AddAnchor(IDC_QUEUECOUNT_LABEL, BOTTOM_LEFT);
	// X: [GB] - [Global Buffer]
	AddAnchor(IDC_TOTALBUFFER, BOTTOM_LEFT);
	AddAnchor(IDC_TOTALBUFFER_LABEL, BOTTOM_LEFT);
	AddAnchor(IDC_QUEUE_REFRESH_BUTTON, BOTTOM_RIGHT);
	AddAnchor(m_dlTab, TOP_CENTER, TOP_RIGHT);

	//cats
	rightclickindex = (size_t)-1;

	downloadlistactive=true;
	m_bIsDragging=false;

	// show & cat-tabs
	m_dlTab.ModifyStyle(0,TCS_OWNERDRAWFIXED);
	m_dlTab.SetPadding(CSize(6, 4));
	if (theApp.IsVistaThemeActive())
		m_dlTab.ModifyStyle(0, WS_CLIPCHILDREN);
	/*Category_Struct* cat0 = thePrefs.GetCategory(0); // move to void CPreferences::LoadCats()
	cat0->strTitle = GetResString(IDS_ALL);// X: [UIC] - [UIChange] change cat0 Title
	cat0->strTempPath.Empty();// X: [TD] - [TempDir]
	cat0->strIncomingPath = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
	cat0->care4all = true;*/

	for (size_t ix=0;ix<thePrefs.GetCatCount();ix++)
		m_dlTab.InsertItem(ix,thePrefs.GetCategory(ix)->strTitle);

	if(thePrefs.lastTranWndCatID && thePrefs.lastTranWndCatID<m_dlTab.GetItemCount()) // X: [RCI] - [Remember Catalog ID]
		m_dlTab.SetCurSel(thePrefs.lastTranWndCatID);

	// create tooltip control for download categories
	m_tooltipCats->Create(this, TTS_NOPREFIX);
	m_dlTab.SetToolTips(m_tooltipCats);
	//UpdateCatTabTitles();
	UpdateTabToolTips();
	m_tooltipCats->SetMargin(CRect(4, 4, 4, 4));
	m_tooltipCats->SendMessage(TTM_SETMAXTIPWIDTH, 0, SHRT_MAX); // recognize \n chars!
	m_tooltipCats->SetDelayTime(TTDT_AUTOPOP, 20000);
	m_tooltipCats->SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1000);
	m_tooltipCats->Activate(TRUE);

	static const uint_ptr IDC[]={IDC_DOWNLOADLIST + IDC_UPLOADLIST,IDC_DOWNLOADLIST,IDC_UPLOADLIST,IDC_QUEUELIST,IDC_DOWNLOADCLIENTS,IDC_CLIENTLIST};
	if(thePrefs.GetTransferWnd1()<_countof(IDC)){
		if(thePrefs.GetTransferWnd1() > 1){
			m_dwShowListIDC = IDC_DOWNLOADLIST + IDC_UPLOADLIST;//fix wrong CatTab position when list isn't IDC_DOWNLOADLIST and IDC_UPLOADLIST + IDC_DOWNLOADLIST
			VerifyCatTabSize();
			m_dlTab.ShowWindow(SW_HIDE);
		}
		m_dwShowListIDC = IDC[thePrefs.GetTransferWnd1()];
	}
	else
		m_dwShowListIDC = IDC_DOWNLOADLIST + IDC_UPLOADLIST;
	m_uWnd2 = (EWnd2)thePrefs.GetTransferWnd2();
	//ResetTransToolbar(thePrefs.IsTransToolbarEnabled());

	Localize();
}

void CTransferWnd::ShowQueueCount()
{
	TCHAR buffer[100];
	_sntprintf(buffer, _countof(buffer) - 1, _T("%u / %u  (%s: %i, ") + GetResString(IDS_STATS_FILTEREDCLIENTS) + _T(')'), theApp.uploadqueue->GetWaitingUserCount(), (thePrefs.GetQueueSize() + max(thePrefs.GetQueueSize()/4, 200)), GetResString(IDS_BANNED), theApp.clientlist->GetBannedCount(), theStats.filteredclients);// X: [SFH] - [Show IP Filter Hits]
	buffer[_countof(buffer) - 1] = _T('\0');
	SetDlgItemText(IDC_QUEUECOUNT,buffer);
}

void CTransferWnd::ShowBufferUsage(uint64 totalsize)// X: [GB] - [Global Buffer]
{
	TCHAR buffer[100];
	_sntprintf(buffer, _countof(buffer) - 1, _T("%s (%.1f%%)"), CastItoXBytes(totalsize), totalsize * 100.0f / thePrefs.m_uGlobalBufferSize);
	buffer[_countof(buffer) - 1] = _T('\0');
	SetDlgItemText(IDC_TOTALBUFFER,buffer);
}

void CTransferWnd::DoDataExchange(CDataExchange* pDX)
{
	CResizableFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DOWNLOAD_ICO, *m_btnWnd1);
	DDX_Control(pDX, IDC_UPLOAD_ICO, *m_btnWnd2);
	DDX_Control(pDX, IDC_DLTAB, m_dlTab);
	DDX_Control(pDX, IDC_UPLOADLIST, uploadlistctrl);
	DDX_Control(pDX, IDC_DOWNLOADLIST, downloadlistctrl);
	DDX_Control(pDX, IDC_QUEUELIST, queuelistctrl);
	DDX_Control(pDX, IDC_CLIENTLIST, clientlistctrl);
	DDX_Control(pDX, IDC_DOWNLOADCLIENTS, downloadclientsctrl);
}

void CTransferWnd::DoResize(int delta)
{
	if(delta){
		CSplitterControl::ChangeHeight(&downloadlistctrl, delta);
		CSplitterControl::ChangeHeight(&uploadlistctrl, -delta, CW_BOTTOMALIGN);
		CSplitterControl::ChangeHeight(&queuelistctrl, -delta, CW_BOTTOMALIGN);
		CSplitterControl::ChangeHeight(&clientlistctrl, -delta, CW_BOTTOMALIGN);
		CSplitterControl::ChangeHeight(&downloadclientsctrl, -delta, CW_BOTTOMALIGN);
		CSplitterControl::ChangePos(m_btnWnd2, 0, delta);

		UpdateSplitterRange();
	}

	if (m_dwShowListIDC == IDC_DOWNLOADLIST + IDC_UPLOADLIST)
	{
		if(delta <0 ){// X: [CI] - [Code Improvement]
			downloadlistctrl.Invalidate();
			downloadlistctrl.UpdateWindow();
		}
		else if(delta > WND_SPLITTER_YOFF + 2){
			CListCtrl* pListCtrl = NULL;
			switch (m_uWnd2)
			{
				case wnd2Uploading:
					pListCtrl = &uploadlistctrl;
					break;
				case wnd2OnQueue:
					pListCtrl = &queuelistctrl;
					break;
				case wnd2Clients:
					pListCtrl = &clientlistctrl;
					break;
				case wnd2Downloading:
					pListCtrl = &downloadclientsctrl;
					break;
				default:
					ASSERT(0);
					return;
			}
			if(delta < WND_SPLITTER_YOFF + 26){
				pListCtrl->GetHeaderCtrl()->Invalidate();
				pListCtrl->GetHeaderCtrl()->UpdateWindow();
			}
			if(delta > WND_SPLITTER_YOFF + 19){
				pListCtrl->Invalidate();
				pListCtrl->UpdateWindow();
			}
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

	RECT rcDown;
	downloadlistctrl.GetWindowRect(&rcDown);
	ScreenToClient(&rcDown);

	RECT rcUp;
	downloadclientsctrl.GetWindowRect(&rcUp);
	ScreenToClient(&rcUp);

	thePrefs.SetSplitterbarPosition(rcDown.bottom * 100 / rcWnd.Height());

	RemoveAnchor(*m_btnWnd2);
	RemoveAnchor(downloadlistctrl);
	RemoveAnchor(uploadlistctrl);
	RemoveAnchor(queuelistctrl);
	RemoveAnchor(clientlistctrl);
	RemoveAnchor(downloadclientsctrl);

	AddAnchor(*m_btnWnd2, CSize(0, thePrefs.GetSplitterbarPosition()));
	AddAnchor(downloadlistctrl, TOP_LEFT, CSize(100, thePrefs.GetSplitterbarPosition()));
	AddAnchor(uploadlistctrl, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
	AddAnchor(queuelistctrl, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
	AddAnchor(clientlistctrl, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
	AddAnchor(downloadclientsctrl, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);

	m_wndSplitter.SetRange(rcDown.top + 50, rcUp.bottom - 40);
}

LRESULT CTransferWnd::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message)
	{
		case WM_WINDOWPOSCHANGED:
			if (m_wndSplitter)
				m_wndSplitter.Invalidate();
			break;
	}

	return CResizableFormView::DefWindowProc(message, wParam, lParam);
}

void CTransferWnd::OnSplitterMoved(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	SPC_NMHDR* pHdr = (SPC_NMHDR*)pNMHDR;
	DoResize(pHdr->delta);
}

BOOL CTransferWnd::PreTranslateMessage(MSG* pMsg)
{
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
			OnDblClickDltab();
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
			CPoint pt(point);
			m_nDropIndex = GetTabUnderMouse(&pt);
			if (m_nDropIndex != m_nLastCatTT)
			{
				m_nLastCatTT = m_nDropIndex;
				if (m_nDropIndex != -1)
					UpdateTabToolTips(m_nDropIndex);
			}
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
					break;
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
					break;
			}
		}
		return TRUE;
	}

	return CResizableFormView::PreTranslateMessage(pMsg);
}

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

//Xman see all sources
void CTransferWnd::UpdateFilesCount() 
{
	if (m_dwShowListIDC == IDC_DOWNLOADLIST || m_dwShowListIDC == IDC_DOWNLOADLIST + IDC_UPLOADLIST)
		m_btnWnd1->SetWindowText(theApp.downloadqueue->GetFilesCount(downloadlistctrl.curTab));
}
//Xman end

void CTransferWnd::UpdateListCount(EWnd2 listindex, size_t iCount /*=-1*/)
{
	static const uint_ptr LISTIDS[] = {IDC_DOWNLOADCLIENTS, IDC_UPLOADLIST, IDC_QUEUELIST, IDC_CLIENTLIST};
	if(m_dwShowListIDC == IDC_DOWNLOADLIST + IDC_UPLOADLIST){
		if(	m_uWnd2 != listindex)
			return;
	}
	else if(m_dwShowListIDC == IDC_DOWNLOADLIST || m_dwShowListIDC != LISTIDS[listindex])
		return;

	CString strBuffer;
	uint_ptr ListID;
	CDropDownButton* btnWnd;
	if(m_dwShowListIDC == IDC_DOWNLOADLIST + IDC_UPLOADLIST){
		btnWnd = m_btnWnd2;
		ListID = LISTIDS[m_uWnd2];
	}
	else{
		btnWnd = m_btnWnd1;
		ListID = m_dwShowListIDC;
	}

	switch (ListID)
	{
		case IDC_UPLOADLIST:{
			if(iCount == (size_t)-1)
				iCount =  uploadlistctrl.GetItemCount();
			//Xman Xtreme Upload
			uint32 activeCount = theApp.uploadBandwidthThrottler->GetNumberOfFullyActivatedSlots();
			//Xman upload health
			//Xman count block/success send
			float health = theApp.uploadBandwidthThrottler->GetAvgHealth();
			float avgblocks = theApp.uploadBandwidthThrottler->GetAvgBlockRatio();
			if(thePrefs.ShowBlockRatio())
				strBuffer.Format(_T(" (%i/%i) %0.0f%% %0.0f%%"), activeCount, iCount, health, avgblocks);
			else
				strBuffer.Format(_T(" (%i/%i) %0.0f%%"), activeCount, iCount, health);
			//Xman end
			btnWnd->SetWindowText(GetResString(IDS_UPLOADING) + strBuffer);
			break;
		}

		case IDC_QUEUELIST:
			strBuffer.Format(_T(" (%i)"), iCount == (size_t)-1 ? queuelistctrl.GetItemCount() : iCount);
			btnWnd->SetWindowText(GetResString(IDS_ONQUEUE) + strBuffer);
			break;

		case IDC_CLIENTLIST:
			strBuffer.Format(_T(" (%i)"), iCount == (size_t)-1 ? clientlistctrl.GetItemCount() : iCount);
			btnWnd->SetWindowText(GetResString(IDS_CLIENTLIST) + strBuffer);
			break;

		case IDC_DOWNLOADCLIENTS:
			strBuffer.Format(_T(" (%i)"), iCount == (size_t)-1 ? downloadclientsctrl.GetItemCount() : iCount);
			btnWnd->SetWindowText(GetResString(IDS_DOWNLOADING) + strBuffer);
			break;
		default:
			ASSERT(0);
			break;
	}
}

//Xman Code Improvement
void CTransferWnd::SwitchUploadList()
{
	switch(m_uWnd2){
		case wnd2Uploading:
			ShowWnd2(wnd2Downloading);
			break;
		case wnd2Downloading:
			if (!thePrefs.IsQueueListDisabled()){
				ShowWnd2(wnd2OnQueue);
				break;
			}
		case wnd2OnQueue:
			if (!thePrefs.IsKnownClientListDisabled()){
				ShowWnd2(wnd2Clients);
				break;
			}
		case wnd2Clients:
			ShowWnd2(wnd2Uploading);
			break;
	}
}
//Xman end

void CTransferWnd::ShowWnd2(EWnd2 uWnd2)
{
	EWnd2 olduWnd2 = m_uWnd2;
	if (uWnd2 == wnd2Downloading) 
	{
		thePrefs.SetTransferWnd2(m_uWnd2 = uWnd2);
		if(olduWnd2 != uWnd2)
			downloadclientsctrl.ShowWindow(SW_SHOW);
		m_btnWnd2->CheckButton(MP_VIEW2_DOWNLOADING);
		SetWnd2Icon(w2iDownloading);
	}
	else if (uWnd2 == wnd2OnQueue && !thePrefs.IsQueueListDisabled())
	{
		thePrefs.SetTransferWnd2(m_uWnd2 = uWnd2);
		if(olduWnd2 != uWnd2){
			queuelistctrl.ShowWindow(SW_SHOW);
			OnBnClickedQueueRefreshButton(); //Xman update this list
			GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_SHOW);
		}
		m_btnWnd2->CheckButton(MP_VIEW2_ONQUEUE);
		SetWnd2Icon(w2iOnQueue);
	}
	else if (uWnd2 == wnd2Clients && !thePrefs.IsKnownClientListDisabled())
	{
		thePrefs.SetTransferWnd2(m_uWnd2 = uWnd2);
		if(olduWnd2 != uWnd2){
			clientlistctrl.ShowWindow(SW_SHOW);
			//Xman SortingFix for Morph-Code-Improvement Don't Refresh item if not needed
			clientlistctrl.UpdateAll();
			//Xman end
		}
		m_btnWnd2->CheckButton(MP_VIEW2_CLIENTS);
		SetWnd2Icon(w2iClientsKnown);
	}
	else
	{
		thePrefs.SetTransferWnd2(m_uWnd2 = wnd2Uploading);
		if(olduWnd2 != uWnd2)
			uploadlistctrl.ShowWindow(SW_SHOW);
		m_btnWnd2->CheckButton(MP_VIEW2_UPLOADING);
		SetWnd2Icon(w2iUploading);
	}
	if(olduWnd2 != uWnd2){
		UpdateListCount(m_uWnd2);
		if (olduWnd2 == wnd2Downloading) 
			downloadclientsctrl.ShowWindow(SW_HIDE);
		else if (olduWnd2 == wnd2Uploading)
			uploadlistctrl.ShowWindow(SW_HIDE);
		else if (olduWnd2 == wnd2OnQueue){
			queuelistctrl.ShowWindow(SW_HIDE);
			GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
		}
		else
			clientlistctrl.ShowWindow(SW_HIDE);
	}
}

void CTransferWnd::OnSysColorChange()
{
	CResizableFormView::OnSysColorChange();
	SetAllIcons();
	m_btnWnd1->Invalidate();
	m_btnWnd2->Invalidate();
}

void CTransferWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CResizableFormView::OnSettingChange(uFlags, lpszSection);
	// It does not work to reset the width of 1st button here.
	//m_btnWnd1->SetBtnWidth(IDC_DOWNLOAD_ICO, WND1_BUTTON_WIDTH);
	//m_btnWnd2->SetBtnWidth(IDC_UPLOAD_ICO, WND2_BUTTON_WIDTH);
}

void CTransferWnd::SetAllIcons()
{
	SetWnd1Icons();
	SetWnd2Icons();
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

void CTransferWnd::LocalizeAll()// X: [RUL] - [Remove Useless Localize]
{
	thePrefs.GetCategory(0)->strTitle = GetResString(IDS_ALL);// X: [UIC] - [UIChange] change cat0 Title
	Localize();
	LocalizeToolbars();
	uploadlistctrl.Localize();
	queuelistctrl.Localize();
	downloadlistctrl.Localize();
	clientlistctrl.Localize();
	downloadclientsctrl.Localize();

/*	if (m_dwShowListIDC == IDC_DOWNLOADLIST + IDC_UPLOADLIST)
		ShowSplitWindow();
	else
		ShowList(m_dwShowListIDC);*/
	switch (m_dwShowListIDC){
		case IDC_UPLOADLIST:
			UpdateListCount(wnd2Uploading);
			break;
		case IDC_QUEUELIST:
			UpdateListCount(wnd2OnQueue);
			break;
		case IDC_CLIENTLIST:
			UpdateListCount(wnd2Clients);
			break;
		case IDC_DOWNLOADCLIENTS:
			UpdateListCount(wnd2Downloading);
			break;
		default:
			UpdateFilesCount();
			UpdateListCount(m_uWnd2);
			break;
	}
}
void CTransferWnd::Localize()// X: [RUL] - [Remove Useless Localize]
{
	SetDlgItemText(IDC_QUEUECOUNT_LABEL,GetResString(IDS_TW_QUEUE));
	SetDlgItemText(IDC_TOTALBUFFER_LABEL,GetResString(IDS_GLOBALBUFFERSIZE));// X: [GB] - [Global Buffer]
	SetDlgItemText(IDC_QUEUE_REFRESH_BUTTON,GetResString(IDS_SV_UPDATE));

	ShowQueueCount();
	UpdateCatTabTitles();
}

void CTransferWnd::LocalizeToolbars()
{
	//m_btnWnd1->SetWindowText(GetResString(IDS_TW_DOWNLOADS));
	m_btnWnd1->SetBtnText(MP_VIEW1_SPLIT_WINDOW, GetResString(IDS_SPLIT_WINDOW));
	m_btnWnd1->SetBtnText(MP_VIEW1_DOWNLOADS, GetResString(IDS_TW_DOWNLOADS));
	m_btnWnd1->SetBtnText(MP_VIEW1_UPLOADING, GetResString(IDS_UPLOADING));
	m_btnWnd1->SetBtnText(MP_VIEW1_DOWNLOADING, GetResString(IDS_DOWNLOADING));
	m_btnWnd1->SetBtnText(MP_VIEW1_ONQUEUE, GetResString(IDS_ONQUEUE));
	m_btnWnd1->SetBtnText(MP_VIEW1_CLIENTS, GetResString(IDS_CLIENTLIST));
	//m_btnWnd2->SetWindowText(GetResString(IDS_UPLOADING));
	m_btnWnd2->SetBtnText(MP_VIEW2_UPLOADING, GetResString(IDS_UPLOADING));
	m_btnWnd2->SetBtnText(MP_VIEW2_DOWNLOADING, GetResString(IDS_DOWNLOADING));
	m_btnWnd2->SetBtnText(MP_VIEW2_ONQUEUE, GetResString(IDS_ONQUEUE));
	m_btnWnd2->SetBtnText(MP_VIEW2_CLIENTS, GetResString(IDS_CLIENTLIST));
}

void CTransferWnd::OnBnClickedQueueRefreshButton()
{
	//Xman faster Updating of Queuelist
	if (queuelistctrl.GetItemCount()>1)
	{
		queuelistctrl.UpdateAll();
	}
	//Xman end
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
	downloadlistctrl.ChangeCategory(m_dlTab.GetCurSel());
	*pResult = 0;
}

// Ornis' download categories
void CTransferWnd::OnNmRClickDltab(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	POINT point;
	::GetCursorPos(&point);
	CPoint pt(point);
	rightclickindex = GetTabUnderMouse(&pt);
	if (rightclickindex == (size_t)-1)
		return;

	Category_Struct* category_Struct = thePrefs.GetCategory(rightclickindex);
	CMenu PrioMenu;
	PrioMenu.CreateMenu();
	PrioMenu.AppendMenu((category_Struct && category_Struct->prio == PR_LOW ? MF_CHECKED : MF_UNCHECKED),MP_PRIOLOW,GetResString(IDS_PRIOLOW));
	PrioMenu.AppendMenu((category_Struct && category_Struct->prio != PR_LOW && category_Struct->prio != PR_HIGH ? MF_CHECKED : MF_UNCHECKED),MP_PRIONORMAL,GetResString(IDS_PRIONORMAL));
	PrioMenu.AppendMenu((category_Struct && category_Struct->prio == PR_HIGH ? MF_CHECKED : MF_UNCHECKED),MP_PRIOHIGH, GetResString(IDS_PRIOHIGH));

	CTitleMenu menu;
	menu.CreatePopupMenu();
	if (rightclickindex)
		menu.AddMenuTitle(GetResString(IDS_CAT) + _T(" (") + thePrefs.GetCategory(rightclickindex)->strTitle + _T(')'), true);
	else
		menu.AddMenuTitle(GetResString(IDS_CAT),true);

	m_isetcatmenu = rightclickindex;
	CMenu CatMenu;
	CatMenu.CreateMenu();
	CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0,GetResString(IDS_ALL) );
	UINT flag = (!thePrefs.GetCategory(rightclickindex)->care4all && rightclickindex) ? MF_GRAYED : MF_STRING;
	CatMenu.AppendMenu(flag,MP_CAT_SET0+1,GetResString(IDS_ALLOTHERS) );

	// selector for regular expression view filter
	if (rightclickindex && thePrefs.IsExtControlsEnabled()) {
		CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+18, GetResString(IDS_REGEXPRESSION) );
		CatMenu.AppendMenu((thePrefs.GetCategory(rightclickindex)->care4all)?MF_STRING:MF_STRING|MF_CHECKED | MF_BYCOMMAND ,MP_CAT_SET0+17,GetResString(IDS_CARE4ALL) );
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

	menu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)CatMenu.m_hMenu, GetResString(IDS_CHANGECATVIEW));
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)PrioMenu.m_hMenu, GetResString(IDS_PRIORITY));
	menu.AppendMenu(MF_STRING,MP_CANCEL,GetResString(IDS_MAIN_BTN_CANCEL));
	menu.AppendMenu(MF_STRING,MP_STOP, GetResString(IDS_DL_STOP));
	menu.AppendMenu(MF_STRING,MP_PAUSE, GetResString(IDS_DL_PAUSE));
	menu.AppendMenu(MF_STRING,MP_RESUME, GetResString(IDS_DL_RESUME));
	menu.AppendMenu(MF_STRING,MP_RESUMENEXT, GetResString(IDS_DL_RESUMENEXT));
	menu.AppendMenu(theApp.IsEd2kFileLinkInClipboard() ? MF_STRING : MF_GRAYED,MP_PASTE, GetResString(IDS_SW_DIRECTDOWNLOAD));
//>>> WiZaRd - Unnecessary user limitation!
//    if(rightclickindex != 0 && thePrefs.IsExtControlsEnabled()) {
	if(thePrefs.IsExtControlsEnabled()) {
//<<< WiZaRd - Unnecessary user limitation!
        menu.AppendMenu((category_Struct && category_Struct->downloadInAlphabeticalOrder ? MF_CHECKED : MF_UNCHECKED),MP_DOWNLOAD_ALPHABETICAL, GetResString(IDS_DOWNLOAD_ALPHABETICAL));
    }
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING,MP_HM_OPENINC, GetResString(IDS_OPENINC));

	flag=(rightclickindex==0) ? MF_GRAYED:MF_STRING;
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING,MP_CAT_ADD,GetResString(IDS_CAT_ADD));
	menu.AppendMenu(flag,MP_CAT_EDIT,GetResString(IDS_CAT_EDIT));
	menu.AppendMenu(flag,MP_CAT_REMOVE, GetResString(IDS_CAT_REMOVE));

	menu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);

	VERIFY( PrioMenu.DestroyMenu() );
	VERIFY( CatMenu.DestroyMenu() );
	VERIFY( menu.DestroyMenu() );

	*pResult = 0;
}

void CTransferWnd::OnLvnBeginDragDownloadList(NMHDR *pNMHDR, LRESULT *pResult)
{
    int iSel = downloadlistctrl.GetSelectionMark();
	if (iSel == -1)
		return;
	if (((CtrlItem_Struct *)downloadlistctrl.GetItemData(iSel))->type != FILE_TYPE)
		return;

	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	m_nDragIndex = pNMLV->iItem;

	ASSERT( m_pDragImage == NULL );
	delete m_pDragImage;
	// It is actually more user friendly to attach the drag image (which could become
	// quite large) somewhat below the mouse cursor, instead of using the exact drag 
	// position. When moving the drag image over the category tab control it is hard
	// to 'see' the tabs of the category control when the mouse cursor is in the middle
	// of the drag image.
	const bool bUseDragHotSpot = false;
	CPoint pt(0, -10); // default drag hot spot
	m_pDragImage = downloadlistctrl.CreateDragImage(m_nDragIndex, bUseDragHotSpot ? &pt : NULL);
	if (m_pDragImage == NULL)
	{
		// fall back code
		m_pDragImage = new CImageList();
		m_pDragImage->Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 0);
		m_pDragImage->Add(CTempIconLoader(_T("AllFiles")));
	}

    m_pDragImage->BeginDrag(0, pt);
    m_pDragImage->DragEnter(GetDesktopWindow(), pNMLV->ptAction);

	m_bIsDragging = true;
	m_nDropIndex = -1;
	SetCapture();

	*pResult = 0;
}

void CTransferWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!(nFlags & MK_LBUTTON))
		m_bIsDragging = false;

	if (m_bIsDragging)
	{
		CPoint pt(point);
		ClientToScreen(&pt);
		CPoint ptScreen(pt);
		m_nDropIndex = GetTabUnderMouse(&pt);
		if (m_nDropIndex > 0 && thePrefs.GetCategory(m_nDropIndex)->care4all)	// not droppable
			m_dlTab.SetCurSel(-1);
		else
			m_dlTab.SetCurSel(m_nDropIndex);

		m_pDragImage->DragMove(ptScreen); //move the drag image to those coordinates
	}
}

void CTransferWnd::OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/)
{
	if (m_bIsDragging)
	{
		ReleaseCapture();
		m_bIsDragging = false;
		m_pDragImage->DragLeave(GetDesktopWindow());
		m_pDragImage->EndDrag();
		delete m_pDragImage;
		m_pDragImage = NULL;
		
		if (   m_nDropIndex > -1
			&& (   downloadlistctrl.curTab == 0 
			    || (downloadlistctrl.curTab > 0 && (UINT)m_nDropIndex != downloadlistctrl.curTab)))
		{
			// for multiselections
			CAtlList<CPartFile *> selectedList;
			POSITION pos = downloadlistctrl.GetFirstSelectedItemPosition();
			while (pos != NULL)
			{ 
				int index = downloadlistctrl.GetNextSelectedItem(pos);
				if (index > -1 && (((CtrlItem_Struct *)downloadlistctrl.GetItemData(index))->type == FILE_TYPE))
					selectedList.AddTail((CPartFile *)((CtrlItem_Struct *)downloadlistctrl.GetItemData(index))->value);
			}

			while (!selectedList.IsEmpty())
			{
				CPartFile *file = selectedList.RemoveHead();// X: [CI] - [Code Improvement]
				file->SetCategory(m_nDropIndex);
			}

			m_dlTab.SetCurSel(downloadlistctrl.curTab);
			downloadlistctrl.UpdateCurrentCategoryView();
			UpdateCatTabTitles();
		}
		else
			m_dlTab.SetCurSel(downloadlistctrl.curTab);
		downloadlistctrl.Invalidate();
	}
}

BOOL CTransferWnd::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	// category filter menuitems
	if (wParam>=MP_CAT_SET0 && wParam<=MP_CAT_SET0+99)
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
			thePrefs.SetCatFilter(m_isetcatmenu, wParam-MP_CAT_SET0);
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
	}
	
	switch (wParam)
	{ 
		case MP_CAT_ADD: {
			AddCategoryInteractive();
			break;
		}
		case MP_CAT_EDIT: {
			m_nLastCatTT=-1;
			CString oldincpath=thePrefs.GetCatPath(rightclickindex);
			CCatDialog dialog(rightclickindex);
			if (dialog.DoModal() == IDOK)
			{
				EditCatTabLabel(rightclickindex, thePrefs.GetCategory(rightclickindex)->strTitle);
				m_dlTab.SetTabTextColor(rightclickindex, thePrefs.GetCatColor(rightclickindex) );
				theApp.emuledlg->searchwnd->UpdateCatTabs();
				if(downloadlistctrl.curTab == rightclickindex)// X: [CI] - [Code Improvement]
					downloadlistctrl.UpdateCurrentCategoryView();
				thePrefs.SaveCats();
				if (CompareDirectories(oldincpath, thePrefs.GetCatPath(rightclickindex)))
					theApp.emuledlg->sharedfileswnd->Reload();
			}
			break;
		}
		case MP_CAT_REMOVE: {
			m_nLastCatTT=-1;
			bool toreload=( _tcsicmp(thePrefs.GetCatPath(rightclickindex), thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR))!=0);
			theApp.downloadqueue->ResetCatParts(rightclickindex);
			thePrefs.RemoveCat(rightclickindex);
			m_dlTab.DeleteItem(rightclickindex);
			m_dlTab.SetCurSel(0);
			if (thePrefs.GetCatCount()==1){// X: [BF] - [Bug Fix]
				thePrefs.GetCategory(0)->filter=0;
				EditCatTabLabel(0);
			}
			else
				VerifyCatTabSize();
			downloadlistctrl.ChangeCategory(0);
			thePrefs.SaveCats();
			theApp.emuledlg->searchwnd->UpdateCatTabs();
			if (toreload)
				theApp.emuledlg->sharedfileswnd->Reload();
			break;
		}

		case MP_PRIOLOW:
            thePrefs.GetCategory(rightclickindex)->prio = PR_LOW;
			thePrefs.SaveCats();
			break;
		case MP_PRIONORMAL:
            thePrefs.GetCategory(rightclickindex)->prio = PR_NORMAL;
			thePrefs.SaveCats();
			break;
		case MP_PRIOHIGH:
            thePrefs.GetCategory(rightclickindex)->prio = PR_HIGH;
			thePrefs.SaveCats();
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
		case MP_PASTE:
			if (theApp.IsEd2kFileLinkInClipboard())
				theApp.PasteClipboard(rightclickindex);
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

		case MP_HM_OPENINC:
			ShellExecute(NULL, _T("open"), thePrefs.GetCategory(m_isetcatmenu)->strIncomingPath,NULL, NULL, SW_SHOW);
			break;

	}
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

void CTransferWnd::EditCatTabLabel(size_t i)
{
	EditCatTabLabel(i,/*(i==0)? GetCatTitle( thePrefs.GetAllcatType() ):*/thePrefs.GetCategory(i)->strTitle);
}

void CTransferWnd::EditCatTabLabel(size_t index,CString newlabel)
{
	TCITEM tabitem;
	tabitem.mask = TCIF_PARAM;
	m_dlTab.GetItem(index,&tabitem);
	tabitem.mask = TCIF_TEXT;

	newlabel.Replace(_T("&"),_T("&&"));

	//if (!index)
		//newlabel.Empty();

	if (/*!index || (index && */thePrefs.GetCatFilter(index)>0)/*)*/ {

		//if (index)
		newlabel.Append(_T(" (")) ;
		
		if (thePrefs.GetCatFilterNeg(index))
			newlabel.AppendChar(_T('!'));			
 
		if (thePrefs.GetCatFilter(index)==18)
			newlabel.Append( _T('\"') + thePrefs.GetCategory(index)->regexp + _T('\"') );
		else
        	newlabel.Append( GetCatTitle(thePrefs.GetCatFilter(index)));

		//if (index)
		newlabel.AppendChar( _T(')') );
	}

	if (thePrefs.ShowCatTabInfos()) {
		uint_ptr total = 0, dwl=0;
		for (POSITION pos = theApp.downloadqueue->filelist.GetHeadPosition();pos != 0;){// X: [CI] - [Code Improvement]
			CPartFile* file = theApp.downloadqueue->filelist.GetNext(pos);
			if (file->CheckShowItemInGivenCat(index)){
				total++;
				if (file->GetTransferringSrcCount()>0) ++dwl;
			}
		}
		CString title=newlabel;
		newlabel.Format(_T("%s %i/%i"), title, dwl, total);
	}

	tabitem.pszText = newlabel.LockBuffer();
	m_dlTab.SetItem(index,&tabitem);
	newlabel.UnlockBuffer();

	VerifyCatTabSize();
}

size_t CTransferWnd::AddCategory(CString newtitle,CString newincoming,CString newtemp,CString newcomment, CString newautocat, bool addTab)// X: [TD] - [TempDir]
{
	Category_Struct* newcat=new Category_Struct;
	newcat->strTitle = newtitle;
	newcat->prio=PR_NORMAL;
	newcat->strTempPath=newtemp;// X: [TD] - [TempDir]
	newcat->strIncomingPath = newincoming;
	newcat->strComment = newcomment;
	newcat->regexp.Empty();
	newcat->ac_regexpeval=false;
	newcat->autocat=newautocat;
    newcat->downloadInAlphabeticalOrder = FALSE;
	newcat->filter=0;
	newcat->filterNeg=false;
	newcat->care4all=false;
	newcat->color= (DWORD)-1;

	size_t index = thePrefs.AddCat(newcat);
	if (addTab) m_dlTab.InsertItem(index,newtitle);
	VerifyCatTabSize();
	
	return index;
}

size_t CTransferWnd::AddCategoryInteractive()
{
	m_nLastCatTT=-1;
	size_t newindex = AddCategory(_T("?"),thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR),_T(""),_T(""),_T(""),false);
	CCatDialog dialog(newindex);
	if (dialog.DoModal() == IDOK)
	{
		theApp.emuledlg->searchwnd->UpdateCatTabs();
		m_dlTab.InsertItem(newindex,thePrefs.GetCategory(newindex)->strTitle);
		m_dlTab.SetTabTextColor(newindex, thePrefs.GetCatColor(newindex) );
		EditCatTabLabel(newindex);
		thePrefs.SaveCats();
		//VerifyCatTabSize();
		if (CompareDirectories(thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR), thePrefs.GetCatPath(newindex)))
			theApp.emuledlg->sharedfileswnd->Reload();
		return newindex;
	}
	else
	{
		thePrefs.RemoveCat(newindex);
		return 0;
	}
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
	int nTab = m_dlTab.HitTest( &hitinfo );

	if( hitinfo.flags != TCHT_NOWHERE )
		return nTab;
	else
		return -1;
}

void CTransferWnd::OnLvnKeydownDownloadList(NMHDR *pNMHDR, LRESULT *pResult)
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
	uint_ptr count, dwl, err, paus;
	count = dwl = err = paus = 0;
	float speed = 0;
	uint64 size = 0;
	uint64 trsize = 0;
	uint64 disksize = 0;
	for (POSITION pos = theApp.downloadqueue->filelist.GetHeadPosition();pos != 0;){// X: [CI] - [Code Improvement]
		CPartFile* cur_file = theApp.downloadqueue->filelist.GetNext(pos);
		if (cur_file->CheckShowItemInGivenCat(tab))
		{
			count++;
			if (cur_file->GetTransferringSrcCount() > 0)
				dwl++;
			speed += cur_file->GetDownloadDatarate() / 1024.0F; //Xman // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
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

	size_t compl = downloadlistctrl.GetCompleteDownloads(tab/*, total*/);// X: [CI] - [Code Improvement]

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

void CTransferWnd::OnDblClickDltab()
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
	size_t from = m_dlTab.GetLastMovementSource();
	size_t to = m_dlTab.GetLastMovementDestionation();

	if (from==0 || to==0 || from==to-1) return;

	// do the reorder
	
	// rearrange the cat-map
	if (!thePrefs.MoveCat(from,to)) return;

	// update partfile-stored assignment
	theApp.downloadqueue->MoveCat(from,to);

	// move category of completed files
	downloadlistctrl.MoveCompletedfilesCat(from, to);

	// of the tabcontrol itself
	m_dlTab.ReorderTab(from,to);

	UpdateCatTabTitles();
	theApp.emuledlg->searchwnd->UpdateCatTabs();

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

	RemoveAnchor(m_dlTab);
	m_dlTab.SetWindowPlacement(&wp);
	AddAnchor(m_dlTab, TOP_RIGHT);
}

CString CTransferWnd::GetCatTitle(size_t catid)
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
		case 20 : return GetResString(IDS_SEARCH_EMULECOLLECTION);
	}
	return _CString(_T("?"));
}

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
	m_btnWnd1->SetButtonInfo(GetWindowLongPtr(*m_btnWnd1, GWL_ID), &tbbi);
}

void CTransferWnd::SetWnd2Icon(EWnd2Icon iIcon)
{
	TBBUTTONINFO tbbi = {0};
	tbbi.cbSize = sizeof tbbi;
	tbbi.dwMask = TBIF_IMAGE;
	tbbi.iImage = iIcon;
	m_btnWnd2->SetButtonInfo(GetWindowLongPtr(*m_btnWnd2, GWL_ID), &tbbi);
}

void CTransferWnd::ShowList(uint_ptr dwListIDC)
{
	CRect rcWnd;
	GetWindowRect(rcWnd);
	ScreenToClient(rcWnd);

	RECT rcDown;
	GetDlgItem(dwListIDC)->GetWindowRect(&rcDown);
	ScreenToClient(&rcDown);
	rcDown.bottom = rcWnd.bottom - 20;
	rcDown.top = 28;
	m_wndSplitter.DestroyWindow();
	RemoveAnchor(dwListIDC);

	uint_ptr oldListIDC = m_dwShowListIDC;
	if(oldListIDC == IDC_DOWNLOADLIST + IDC_UPLOADLIST){
		m_btnWnd2->ShowWindow(SW_HIDE);
		downloadlistctrl.ShowWindow(SW_HIDE);
		m_dlTab.ShowWindow(SW_HIDE);
		uploadlistctrl.ShowWindow(SW_HIDE);
		queuelistctrl.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
		downloadclientsctrl.ShowWindow(SW_HIDE);
		clientlistctrl.ShowWindow(SW_HIDE);
	}

	m_dwShowListIDC = dwListIDC;

	switch (dwListIDC)
	{
		case IDC_DOWNLOADLIST:
			downloadlistctrl.MoveWindow(&rcDown);
			downloadlistctrl.ShowWindow(SW_SHOW);
			m_dlTab.ShowWindow(SW_SHOW);
			UpdateFilesCount();
			m_btnWnd1->CheckButton(MP_VIEW1_DOWNLOADS);
			SetWnd1Icon(w1iDownloadFiles);
			thePrefs.SetTransferWnd1(1);
			break;
		case IDC_UPLOADLIST:
			uploadlistctrl.MoveWindow(&rcDown);
			uploadlistctrl.ShowWindow(SW_SHOW);
			UpdateListCount(wnd2Uploading);
			m_btnWnd1->CheckButton(MP_VIEW1_UPLOADING);
			SetWnd1Icon(w1iUploading);
			thePrefs.SetTransferWnd1(2);
			break;
		case IDC_QUEUELIST:
			queuelistctrl.MoveWindow(&rcDown);
			queuelistctrl.ShowWindow(SW_SHOW);
			GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_SHOW);
			UpdateListCount(wnd2OnQueue);
			m_btnWnd1->CheckButton(MP_VIEW1_ONQUEUE);
			SetWnd1Icon(w1iOnQueue);
			thePrefs.SetTransferWnd1(3);
			OnBnClickedQueueRefreshButton(); //Xman update this list
			break;
		case IDC_DOWNLOADCLIENTS:
			downloadclientsctrl.MoveWindow(&rcDown);
			downloadclientsctrl.ShowWindow(SW_SHOW);
			UpdateListCount(wnd2Downloading);
			m_btnWnd1->CheckButton(MP_VIEW1_DOWNLOADING);
			SetWnd1Icon(w1iDownloading);
			thePrefs.SetTransferWnd1(4);
			break;
		case IDC_CLIENTLIST:
			clientlistctrl.MoveWindow(&rcDown);
			clientlistctrl.ShowWindow(SW_SHOW);
			UpdateListCount(wnd2Clients);
			m_btnWnd1->CheckButton(MP_VIEW1_CLIENTS);
			SetWnd1Icon(w1iClientsKnown);
			thePrefs.SetTransferWnd1(5);
			//Xman SortingFix for Morph-Code-Improvement Don't Refresh item if not needed
			clientlistctrl.UpdateAll();
			//Xman end
			break;
		default:
			ASSERT(0);
			break;
	}
	AddAnchor(dwListIDC, TOP_LEFT, BOTTOM_RIGHT);
	theApp.emuledlg->transferwnd->ShowToolbar(dwListIDC == IDC_DOWNLOADLIST);

	if(oldListIDC != IDC_DOWNLOADLIST + IDC_UPLOADLIST && oldListIDC != dwListIDC){
		switch (oldListIDC)
		{
			case IDC_DOWNLOADLIST:
				downloadlistctrl.ShowWindow(SW_HIDE);
				m_dlTab.ShowWindow(SW_HIDE);
				break;
			case IDC_UPLOADLIST:
				uploadlistctrl.ShowWindow(SW_HIDE);
				break;
			case IDC_QUEUELIST:
				queuelistctrl.ShowWindow(SW_HIDE);
				GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
				break;
			case IDC_DOWNLOADCLIENTS:
				downloadclientsctrl.ShowWindow(SW_HIDE);
				break;
			case IDC_CLIENTLIST:
				clientlistctrl.ShowWindow(SW_HIDE);
				break;
			default:
				ASSERT(0);
				break;
		}
	}
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

	LONG splitpos = (thePrefs.GetSplitterbarPosition() * rcWnd.Height() + rcWnd.Height()/2) / 100;// X fix without using the Magic constant

	RECT rcDown;
	downloadlistctrl.GetWindowRect(&rcDown);
	ScreenToClient(&rcDown);
	rcDown.bottom = splitpos;
	downloadlistctrl.MoveWindow(&rcDown);

	uploadlistctrl.GetWindowRect(&rcDown);
	ScreenToClient(&rcDown);
	rcDown.right = rcWnd.right;
	rcDown.bottom = rcWnd.bottom - 20;
	rcDown.top = splitpos + 25;
	uploadlistctrl.MoveWindow(&rcDown);
	queuelistctrl.MoveWindow(&rcDown);
	clientlistctrl.MoveWindow(&rcDown);
	downloadclientsctrl.MoveWindow(&rcDown);

	CRect rcBtn2;
	m_btnWnd2->GetWindowRect(rcBtn2);
	ScreenToClient(rcBtn2);
	CRect rcSpl;
	rcSpl.left = rcBtn2.right + 8;
	rcSpl.right = rcDown.right;
	rcSpl.top = splitpos + WND_SPLITTER_YOFF;
	rcSpl.bottom = rcSpl.top + WND_SPLITTER_HEIGHT;
	if (!m_wndSplitter) {
		m_wndSplitter.Create(WS_CHILD | WS_VISIBLE, rcSpl, this, IDC_SPLITTER);
		m_wndSplitter.SetDrawBorder(true);
	}
	else
		m_wndSplitter.MoveWindow(rcSpl, TRUE);
	m_btnWnd2->MoveWindow(WND2_BUTTON_XOFF, rcSpl.top - (WND_SPLITTER_YOFF - 1), rcBtn2.Width(), WND2_BUTTON_HEIGHT);
	DoResize(0);

	m_dwShowListIDC = IDC_DOWNLOADLIST + IDC_UPLOADLIST;
	UpdateFilesCount();
	m_btnWnd2->ShowWindow(SW_SHOW);
	theApp.emuledlg->transferwnd->ShowToolbar(true);

	RemoveAnchor(*m_btnWnd2);
	RemoveAnchor(downloadlistctrl);
	RemoveAnchor(uploadlistctrl);
	RemoveAnchor(queuelistctrl);
	RemoveAnchor(clientlistctrl);
	RemoveAnchor(downloadclientsctrl);

	AddAnchor(*m_btnWnd2, CSize(0, thePrefs.GetSplitterbarPosition()));
	AddAnchor(downloadlistctrl, TOP_LEFT, CSize(100, thePrefs.GetSplitterbarPosition()));
	AddAnchor(uploadlistctrl, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
	AddAnchor(queuelistctrl, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
	AddAnchor(clientlistctrl, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
	AddAnchor(downloadclientsctrl, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);

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
	bool bSwitchList1 = false;
	bool bSwitchList2 = false;
	if (thePrefs.m_bDisableKnownClientList)
	{
		clientlistctrl.DeleteAllItems();
		if (m_dwShowListIDC == IDC_CLIENTLIST)
			bSwitchList1 = true;
		if (m_uWnd2 == wnd2Clients)
			bSwitchList2 = true;
	}
	if (thePrefs.m_bDisableQueueList)
	{
		queuelistctrl.DeleteAllItems();
		if (m_dwShowListIDC == IDC_QUEUELIST)
			bSwitchList1 = true;
		if (m_uWnd2 == wnd2OnQueue)
			bSwitchList2 = true;
	}
	if (bSwitchList1)
		OnBnClickedChangeView();
	if (bSwitchList2)
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

	RECT rc;
	m_btnWnd1->GetWindowRect(&rc);
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rc.left, rc.bottom, this);
	VERIFY( menu.DestroyMenu() );
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

	RECT rc;
	m_btnWnd2->GetWindowRect(&rc);
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rc.left, rc.bottom, this);
	VERIFY( menu.DestroyMenu() );
}

void CTransferWnd::ResetTransToolbar(bool bShowToolbar)
{
	if (m_btnWnd1->m_hWnd)
		RemoveAnchor(*m_btnWnd1);
	if (m_btnWnd2->m_hWnd)
		RemoveAnchor(*m_btnWnd2);

	CRect rcBtn1;
	rcBtn1.top = 5;
	rcBtn1.left = WND1_BUTTON_XOFF;
	rcBtn1.right = rcBtn1.left + WND1_BUTTON_WIDTH;
	if(bShowToolbar){
		rcBtn1.right += WND1_NUM_BUTTONS*DFLT_TOOLBAR_BTN_WIDTH;
		if(thePrefs.IsQueueListDisabled()) // X: [Bug] Can't be Gray,so hide it
			rcBtn1.right -= DFLT_TOOLBAR_BTN_WIDTH;
		if(thePrefs.IsKnownClientListDisabled()) // X: [Bug] Can't be Gray,so hide it
			rcBtn1.right -= DFLT_TOOLBAR_BTN_WIDTH;
	}
	rcBtn1.bottom = rcBtn1.top + WND1_BUTTON_HEIGHT;
	m_btnWnd1->Init(!bShowToolbar);
	m_btnWnd1->MoveWindow(&rcBtn1);
	SetWnd1Icons();

	CRect rcBtn2;
	m_btnWnd2->GetWindowRect(rcBtn2);
	ScreenToClient(rcBtn2);
	//rcBtn2.top = rcBtn2.top;
	rcBtn2.left = WND2_BUTTON_XOFF;
	rcBtn2.right = rcBtn2.left + WND2_BUTTON_WIDTH;
	if(bShowToolbar){
		rcBtn2.right += WND2_NUM_BUTTONS*DFLT_TOOLBAR_BTN_WIDTH;
		if(thePrefs.IsQueueListDisabled()) // X: [Bug] Can't be Gray,so hide it
			rcBtn2.right -= DFLT_TOOLBAR_BTN_WIDTH;
		if(thePrefs.IsKnownClientListDisabled()) // X: [Bug] Can't be Gray,so hide it
			rcBtn2.right -= DFLT_TOOLBAR_BTN_WIDTH;
	}
	rcBtn2.bottom = rcBtn2.top + WND2_BUTTON_HEIGHT;
	m_btnWnd2->Init(!bShowToolbar);
	m_btnWnd2->MoveWindow(&rcBtn2);
	SetWnd2Icons();

	if (bShowToolbar)
	{
		// Vista: Remove the TBSTYLE_TRANSPARENT to avoid flickering (can be done only after the toolbar was initially created with TBSTYLE_TRANSPARENT !?)
		m_btnWnd1->ModifyStyle((theApp.m_ullComCtrlVer >= MAKEDLLVERULL(6, 16, 0, 0)) ? TBSTYLE_TRANSPARENT : 0, TBSTYLE_TOOLTIPS);
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
		atb1[5].fsState = thePrefs.IsQueueListDisabled() ? TBSTATE_HIDDEN : TBSTATE_ENABLED;// X: [Bug] Can't be Gray,so hide it
		atb1[5].fsStyle = BTNS_BUTTON | BTNS_CHECKGROUP | BTNS_AUTOSIZE;
		atb1[5].iString = -1;

		atb1[6].iBitmap = w1iClientsKnown;
		atb1[6].idCommand = MP_VIEW1_CLIENTS;
		atb1[6].fsState = thePrefs.IsKnownClientListDisabled() ? TBSTATE_HIDDEN : TBSTATE_ENABLED;
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
		// Vista: Remove the TBSTYLE_TRANSPARENT to avoid flickering (can be done only after the toolbar was initially created with TBSTYLE_TRANSPARENT !?)
		m_btnWnd2->ModifyStyle((theApp.m_ullComCtrlVer >= MAKEDLLVERULL(6, 16, 0, 0)) ? TBSTYLE_TRANSPARENT : 0, TBSTYLE_TOOLTIPS);
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
		atb2[3].fsState = thePrefs.IsQueueListDisabled() ? TBSTATE_HIDDEN : TBSTATE_ENABLED;
		atb2[3].fsStyle = BTNS_BUTTON | BTNS_CHECKGROUP | BTNS_AUTOSIZE;
		atb2[3].iString = -1;

		atb2[4].iBitmap = w2iClientsKnown;
		atb2[4].idCommand = MP_VIEW2_CLIENTS;
		atb2[4].fsState = thePrefs.IsKnownClientListDisabled() ? TBSTATE_HIDDEN : TBSTATE_ENABLED;
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
		// Vista: Remove the TBSTYLE_TRANSPARENT to avoid flickering (can be done only after the toolbar was initially created with TBSTYLE_TRANSPARENT !?)
		m_btnWnd1->ModifyStyle(TBSTYLE_TOOLTIPS | ((theApp.m_ullComCtrlVer >= MAKEDLLVERULL(6, 16, 0, 0)) ? TBSTYLE_TRANSPARENT : 0), 0);
		m_btnWnd1->SetExtendedStyle(m_btnWnd1->GetExtendedStyle() & ~TBSTYLE_EX_MIXEDBUTTONS);
		m_btnWnd1->RecalcLayout(true);

		// Vista: Remove the TBSTYLE_TRANSPARENT to avoid flickering (can be done only after the toolbar was initially created with TBSTYLE_TRANSPARENT !?)
		m_btnWnd2->ModifyStyle(TBSTYLE_TOOLTIPS | ((theApp.m_ullComCtrlVer >= MAKEDLLVERULL(6, 16, 0, 0)) ? TBSTYLE_TRANSPARENT : 0), 0);
		m_btnWnd2->SetExtendedStyle(m_btnWnd2->GetExtendedStyle() & ~TBSTYLE_EX_MIXEDBUTTONS);
		m_btnWnd2->RecalcLayout(true);
	}

	AddAnchor(*m_btnWnd1, TOP_LEFT);
	AddAnchor(*m_btnWnd2, CSize(0, thePrefs.GetSplitterbarPosition()));

	LocalizeToolbars();
	if (m_dwShowListIDC == IDC_DOWNLOADLIST + IDC_UPLOADLIST)
		ShowSplitWindow(true);
	else
		ShowList(m_dwShowListIDC);
	VerifyCatTabSize();
	ShowWnd2(m_uWnd2);
}

HBRUSH CTransferWnd::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = theApp.emuledlg->GetCtlColor(pDC, pWnd, nCtlColor);
	if (hbr)
		return hbr;
	return __super::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CTransferWnd::OnPaint()
{
	CResizableFormView::OnPaint();
	CRect rcWnd;
	GetWindowRect(rcWnd);

	// Another small work arround: Init/Redraw the layout as soon as we have our real windows size
	// as the inital size is far below the minimum and will mess things up which expect this size
	if (!m_bLayoutInited && rcWnd.Height() > 400)
	{
		m_bLayoutInited = true;
		ResetTransToolbar(thePrefs.IsTransToolbarEnabled());
		/*if (m_dwShowListIDC == IDC_DOWNLOADLIST + IDC_UPLOADLIST)
			ShowSplitWindow(true);
		else
			ShowList(m_dwShowListIDC);*/
	}

	if (m_wndSplitter)
	{
		if (rcWnd.Height() > 0)
		{
			RECT rcDown;
			downloadlistctrl.GetWindowRect(&rcDown);
			ScreenToClient(&rcDown);

			RECT rcBtn2;
			m_btnWnd2->GetWindowRect(&rcBtn2);
			ScreenToClient(&rcBtn2);

			// splitter paint update
			RECT rcSpl = {
				rcBtn2.right + 8,
				rcDown.bottom + WND_SPLITTER_YOFF,
				rcDown.right,
				rcDown.bottom + WND_SPLITTER_YOFF + WND_SPLITTER_HEIGHT
			};
			m_wndSplitter.MoveWindow(&rcSpl, TRUE);
			UpdateSplitterRange();
		}
	}

	// Workaround to solve a glitch with WM_SETTINGCHANGE message
	if (m_btnWnd1 && m_btnWnd1->m_hWnd && m_btnWnd1->GetBtnWidth(IDC_DOWNLOAD_ICO) != WND1_BUTTON_WIDTH)
		m_btnWnd1->SetBtnWidth(IDC_DOWNLOAD_ICO, WND1_BUTTON_WIDTH);
	if (m_btnWnd2 && m_btnWnd2->m_hWnd && m_btnWnd2->GetBtnWidth(IDC_UPLOAD_ICO) != WND2_BUTTON_WIDTH)
		m_btnWnd2->SetBtnWidth(IDC_UPLOAD_ICO, WND2_BUTTON_WIDTH);
}

void CTransferWnd::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (nID == SC_KEYMENU)
	{
		if (lParam == EMULE_HOTMENU_ACCEL)
			theApp.emuledlg->SendMessage(WM_COMMAND, IDC_HOTMENU);
		else
			theApp.emuledlg->SendMessage(WM_SYSCOMMAND, nID, lParam);
		return;
	}
	__super::OnSysCommand(nID, lParam);
}