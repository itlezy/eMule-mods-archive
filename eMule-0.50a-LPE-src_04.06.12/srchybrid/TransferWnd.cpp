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
#include "UploadBandwidthThrottler.h" //Xman Xtreme upload
#include "SharedFilesCtrl.h"
#include "Statistics.h"// X: [SFH] - [Show IP Filter Hits]
#include "DesktopIntegration.h"	// netfinity: Better support for WINE Gnome/KDE desktops

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	DFLT_TOOLBAR_BTN_WIDTH	24

#define	WND_SPLITTER_YOFF	10
#define	WND_SPLITTER_HEIGHT	4

#define	WND1_BUTTON_XOFF	2 //8->2
#define	WND1_BUTTON_WIDTH	220 //170 Xman see all sources
#define	WND1_BUTTON6_WIDTH	130
#define	WND1_BUTTON_HEIGHT	22	// don't set the height to something different than 22 unless you know exactly what you are doing!
#define	WND1_NUM_BUTTONS	3

#define PARTSTATUSCTRLHEIGHT 16

#define	WND2_BUTTON_XOFF	2 //8->2
#define	WND2_BUTTON_WIDTH	220 //170 //Xman Uploadhealth 
#define	WND2_BUTTON_HEIGHT	22	// don't set the height to something different than 22 unless you know exactly what you are doing!
#define	WND2_NUM_BUTTONS	0 //2->0

// CTransferWnd dialog

IMPLEMENT_DYNAMIC(CTransferWnd, CDialog)

BEGIN_MESSAGE_MAP(CTransferWnd, CResizableDialog)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_DOWNLOADLIST, OnLvnBeginDragDownloadList)
	ON_NOTIFY(LVN_HOTTRACK, IDC_DOWNLOADLIST, OnHoverDownloadList)
	ON_NOTIFY(LVN_HOTTRACK, IDC_DOWNHISTORYLIST, OnHoverUploadList)
	ON_NOTIFY(LVN_HOTTRACK, IDC_UPLOADLIST, OnHoverUploadList)
	ON_NOTIFY(LVN_HOTTRACK, IDC_SFLIST , OnHoverUploadList)
	ON_MESSAGE(UM_DELAYED_EVALUATE, OnChangeFilter)
	ON_NOTIFY(NM_RCLICK, IDC_DLTAB, OnNmRClickDltab)
	ON_NOTIFY(TBN_DROPDOWN, IDC_DOWNLOAD_ICO, OnWnd1BtnDropDown)
	ON_NOTIFY(TBN_DROPDOWN, IDC_UPLOAD_ICO, OnWnd2BtnDropDown)
	ON_NOTIFY(TCN_SELCHANGE, IDC_DLTAB, OnTcnSelchangeDltab)
	ON_NOTIFY(UM_SPN_SIZED, IDC_SPLITTER, OnSplitterMoved)
	ON_NOTIFY(UM_TABMOVED, IDC_DLTAB, OnTabMovement)
	ON_WM_CTLCOLOR()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	//ON_WM_SETTINGCHANGE()
	//ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

CTransferWnd::CTransferWnd(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CTransferWnd::IDD, pParent)
	, partstatusctrl(&downloadclientsctrl)
{
	m_uWnd2 = wnd2Downloading;
	m_dwShowListIDC = 0;
	m_pLastMousePoint.x = -1;
	m_pLastMousePoint.y = -1;
	m_nLastCatTT = -1;
	m_btnWnd1 = new CDropDownButton;
	m_btnWnd2 = new CDropDownButton;
	m_tooltipCats = new CToolTipCtrlX;
	m_pDragImage = NULL;
	nAICHHashing = 0;
}

CTransferWnd::~CTransferWnd()
{
	delete m_btnWnd1;
	delete m_btnWnd2;
	delete m_tooltipCats;
	ASSERT( m_pDragImage == NULL );
	delete m_pDragImage;
}

BOOL CTransferWnd::OnInitDialog()
{
	CResizableDialog::OnInitDialog();
	InitWindowStyles(this);

	uploadlistctrl.Init();
	downloadlistctrl.Init();
	downloadclientsctrl.Init();
	sharedfilesctrl.Init();
	historylistctrl.Init();
	historylistctrl.dlg=downloadlistctrl.dlg=sharedfilesctrl.dlg=this;
	m_ctlFilter.OnInit(downloadlistctrl);

	CRect rcWnd;
	GetWindowRect(rcWnd);
	ScreenToClient(rcWnd);

	RECT rcDown;
	sharedfilesctrl.GetWindowRect(&rcDown);
	ScreenToClient(&rcDown);
	rcDown.right = rcWnd.right;
	rcDown.bottom = rcWnd.bottom - 20;
	rcDown.top = 23;
	sharedfilesctrl.MoveWindow(&rcDown);
	historylistctrl.MoveWindow(&rcDown);

	AddAnchor(downloadlistctrl, TOP_LEFT, CSize(100, thePrefs.GetSplitterbarPosition()));
	AddAnchor(sharedfilesctrl, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(historylistctrl, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(uploadlistctrl, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
	AddAnchor(downloadclientsctrl, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
	AddAnchor(partstatusctrl, CSize(0, thePrefs.GetSplitterbarPosition()), CSize(100, thePrefs.splitterbarPosition));
	AddAnchor(IDC_QUEUECOUNT, BOTTOM_LEFT);
	AddAnchor(IDC_TOTALBUFFER, BOTTOM_LEFT);// X: [GB] - [Global Buffer]
	AddAnchor(IDC_REFRESH_BUTTON, BOTTOM_RIGHT);
	AddAnchor(m_dlTab, TOP_LEFT, TOP_RIGHT);
	AddAnchor(m_ctlFilter, TOP_RIGHT);

	//cats
	rightclickindex = (size_t)-1;

	downloadlistactive=true;
	m_bIsDragging=false;

	// show & cat-tabs
	m_dlTab.ModifyStyle(0,TCS_OWNERDRAWFIXED);
	m_dlTab.SetPadding(CSize(6, 4));
	if (theApp.IsVistaThemeActive())
		m_dlTab.ModifyStyle(0, WS_CLIPCHILDREN);
	Category_Struct* cat0 = thePrefs.GetCategory(0);
	/*cat0->strTitle = GetResString(IDS_ALL);// X: [UIC] - [UIChange] change cat0 Title
	cat0->strTempPath.Empty();// X: [TD] - [TempDir]
	cat0->strIncomingPath = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
	cat0->care4all = true;*/
	downloadlistctrl.filter = cat0->filter;

	for (size_t ix=0;ix<thePrefs.GetCatCount();ix++)
		m_dlTab.InsertItem(ix,thePrefs.GetCategory(ix)->strTitle);

	if(thePrefs.lastTranWndCatID && thePrefs.lastTranWndCatID<m_dlTab.GetItemCount()) // X: [RCI] - [Remember Catalog ID]
		m_dlTab.SetCurSel(thePrefs.lastTranWndCatID);

	// create tooltip control for download categories
	m_tooltipCats->Create(this, TTS_NOPREFIX);
	m_dlTab.SetToolTips(m_tooltipCats);
	UpdateCatTabTitles();
	UpdateTabToolTips();
	m_tooltipCats->SetMargin(CRect(4, 4, 4, 4));
	m_tooltipCats->SendMessage(TTM_SETMAXTIPWIDTH, 0, SHRT_MAX); // recognize \n chars!
	m_tooltipCats->SetDelayTime(TTDT_AUTOPOP, 20000);
	m_tooltipCats->SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1000);
	m_tooltipCats->Activate(TRUE);

	m_uWnd2 = (EWnd2)thePrefs.GetTransferWnd2();

	switch (thePrefs.GetTransferWnd1()) {
		default:
		case 0:
		case 1:
			ShowList(IDC_DOWNLOADLIST);
			UpdateSplitterRange();
			break;
		case 2:
			ShowList(IDC_SFLIST);
			break;
		case 3:
			ShowList(IDC_DOWNHISTORYLIST);
			break;
	}
	ResetTransToolbar(thePrefs.IsTransToolbarEnabled());
	Localize();

	return true;
}

void CTransferWnd::ShowQueueCount()
{
	TCHAR buffer[100];
	_sntprintf(buffer, _countof(buffer) - 1, _T("%s: %u / %u  (%s: %i, ") + GetResString(IDS_STATS_FILTEREDCLIENTS) + _T(')'), GetResString(IDS_ONQUEUE), theApp.uploadqueue->GetWaitingUserCount(), (thePrefs.GetQueueSize() + max(thePrefs.GetQueueSize()/4, 200)), GetResString(IDS_BANNED), theApp.clientlist->GetBannedCount(), theStats.filteredclients);// X: [SFH] - [Show IP Filter Hits]
	buffer[_countof(buffer) - 1] = _T('\0');
	SetDlgItemText(IDC_QUEUECOUNT,buffer);
}

void CTransferWnd::ShowBufferUsage(uint64 totalsize)// X: [GB] - [Global Buffer]
{
	TCHAR buffer[100];
	_sntprintf(buffer, _countof(buffer) - 1, _T("GB: %s (%.1f%%)"), CastItoXBytes(totalsize), totalsize * 100.0f / thePrefs.m_uGlobalBufferSize);
	buffer[_countof(buffer) - 1] = _T('\0');
   if (thePrefs.ShowBufferDisplay())
	SetDlgItemText(IDC_TOTALBUFFER,buffer);
   else
       SetDlgItemText(IDC_TOTALBUFFER,_T(""));
}

void CTransferWnd::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DOWNLOAD_ICO, *m_btnWnd1);
	DDX_Control(pDX, IDC_UPLOAD_ICO, *m_btnWnd2);
	DDX_Control(pDX, IDC_DLTAB, m_dlTab);
	DDX_Control(pDX, IDC_UPLOADLIST, uploadlistctrl);
	DDX_Control(pDX, IDC_DOWNLOADLIST, downloadlistctrl);
	DDX_Control(pDX, IDC_DOWNLOADCLIENTS, downloadclientsctrl);
	DDX_Control(pDX, IDC_SFLIST, sharedfilesctrl);
	DDX_Control(pDX, IDC_DOWNHISTORYLIST, historylistctrl);
	DDX_Control(pDX, IDC_SHAREDFILES_FILTER, m_ctlFilter);
	DDX_Control(pDX, IDC_PARTSTATUS, partstatusctrl);
}

void CTransferWnd::DoResize(int delta)
{
	if(delta){
		CSplitterControl::ChangeHeight(&downloadlistctrl, delta);
		CSplitterControl::ChangeHeight(&uploadlistctrl, -delta, CW_BOTTOMALIGN);
		CSplitterControl::ChangeHeight(&downloadclientsctrl, -delta, CW_BOTTOMALIGN);
		CSplitterControl::ChangePos(m_btnWnd2, 0, delta);
		CSplitterControl::ChangePos(&partstatusctrl,0,delta);

		UpdateSplitterRange();
	}

	if (m_dwShowListIDC == IDC_DOWNLOADLIST)
	{
		if(delta <0 ){// X: [CI] - [Code Improvement]
			downloadlistctrl.Invalidate();
			downloadlistctrl.UpdateWindow();
			partstatusctrl.Refresh(false);
		}
		else if(delta > WND_SPLITTER_YOFF + 2){

			if(delta < WND_SPLITTER_YOFF + PARTSTATUSCTRLHEIGHT + 3)
				partstatusctrl.Refresh(false);
			else{
				CListCtrl* pListCtrl = NULL;
				switch (m_uWnd2)
				{
					case wnd2Downloading:
						pListCtrl = &downloadclientsctrl;
						break;
					case wnd2Uploading:
						pListCtrl = &uploadlistctrl;
						break;
					default:
						ASSERT(0);
						return;
				}
				if(delta < WND_SPLITTER_YOFF + PARTSTATUSCTRLHEIGHT + 26){
					pListCtrl->GetHeaderCtrl()->Invalidate();
					pListCtrl->GetHeaderCtrl()->UpdateWindow();
				}
				if(delta > WND_SPLITTER_YOFF + PARTSTATUSCTRLHEIGHT + 19){
					pListCtrl->Invalidate();
					pListCtrl->UpdateWindow();
				}
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
	RemoveAnchor(downloadclientsctrl);
	RemoveAnchor(partstatusctrl);

	AddAnchor(*m_btnWnd2, CSize(0, thePrefs.GetSplitterbarPosition()));
	AddAnchor(downloadlistctrl, TOP_LEFT, CSize(100, thePrefs.GetSplitterbarPosition()));
	AddAnchor(uploadlistctrl, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
	AddAnchor(downloadclientsctrl, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
	AddAnchor(partstatusctrl, CSize(0, thePrefs.splitterbarPosition), CSize(100, thePrefs.GetSplitterbarPosition()));

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
			if (m_btnWnd1 && m_btnWnd1->m_hWnd){
				if(thePrefs.IsTransToolbarEnabled()){
					if(m_btnWnd1->GetBtnWidth(IDC_DOWNLOAD_ICO) != WND1_BUTTON_WIDTH)
						m_btnWnd1->SetBtnWidth(IDC_DOWNLOAD_ICO, WND1_BUTTON_WIDTH);
				}
				else{
					if(m_btnWnd1->GetBtnWidth(IDC_DOWNLOAD_ICO) != WND1_BUTTON_WIDTH+WND1_BUTTON6_WIDTH)
						m_btnWnd1->SetBtnWidth(IDC_DOWNLOAD_ICO, WND1_BUTTON_WIDTH+WND1_BUTTON6_WIDTH);
				}
			}
			if (m_btnWnd2 && m_btnWnd2->m_hWnd && m_btnWnd2->GetBtnWidth(IDC_UPLOAD_ICO) != WND2_BUTTON_WIDTH)
				m_btnWnd2->SetBtnWidth(IDC_UPLOAD_ICO, WND2_BUTTON_WIDTH);
			break;
		
		case WM_WINDOWPOSCHANGED:
			if (m_wndSplitter)
				m_wndSplitter.Invalidate();
			break;
	}

	return CResizableDialog::DefWindowProc(message, wParam, lParam);
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
		else if (m_dwShowListIDC != IDC_DOWNLOADLIST)
		{
			switch (m_dwShowListIDC)
			{
				case IDC_SFLIST:
					sharedfilesctrl.ShowSelectedFileInfo();
					break;
				case IDC_DOWNHISTORYLIST:
					historylistctrl.ShowSelectedFileInfo();
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
				case wnd2Downloading:
					downloadclientsctrl.ShowSelectedUserDetails();
					break;
				case wnd2Uploading:
					uploadlistctrl.ShowSelectedUserDetails();
					break;
				default:
					ASSERT(0);
					break;
			}
		}
		return TRUE;
	}

	return CResizableDialog::PreTranslateMessage(pMsg);
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
void CTransferWnd::UpdateFilesCount() {
	if (m_dwShowListIDC == IDC_DOWNLOADLIST){
		CString strBuffer = GetResString(IDS_TW_DOWNLOADS);
		if(!thePrefs.IsTransToolbarEnabled())
			strBuffer.AppendFormat(_T("(%s)"), downloadlistctrl.GetFilterLabelCat());
		m_btnWnd1->SetWindowText(strBuffer + theApp.downloadqueue->GetFilesCount(downloadlistctrl.curTab));
	}
}
//Xman end

void CTransferWnd::UpdateListCount(EWnd2 listindex, size_t iCount /*=-1*/)
{
	switch (m_dwShowListIDC)
	{
		case IDC_DOWNLOADLIST: {
			if (m_uWnd2 != listindex)
				return;
			CString strBuffer;
			switch (m_uWnd2)
			{
				case wnd2Downloading:
					strBuffer = GetResString(IDS_DOWNLOADING);
					if(downloadclientsctrl.curPartfile)
						strBuffer.AppendFormat(_T(" (%i/%i)"), downloadclientsctrl.curPartfile->GetTransferringSrcCount(), iCount == (size_t)-1 ? downloadclientsctrl.GetItemCount() : iCount);
					m_btnWnd2->SetWindowText(strBuffer);
					break;
				case wnd2Uploading: {
					strBuffer.Format(_T("%s (%i/%i) %0.0f%%")
						, GetResString(IDS_UPLOADING)
						, theApp.uploadBandwidthThrottler->GetNumberOfFullyActivatedSlots()
						, iCount == (size_t)-1 ? uploadlistctrl.GetItemCount() : iCount
						, theApp.uploadBandwidthThrottler->GetAvgHealth()
						);
					if(thePrefs.ShowBlockRatio())
						strBuffer.AppendFormat(_T(" %0.0f%%"), theApp.uploadBandwidthThrottler->GetAvgBlockRatio());
					//Xman end
					m_btnWnd2->SetWindowText(strBuffer);
					break;
				}
				default:
					ASSERT(0);
					break;
			}
			break;
		}
		
		case IDC_SFLIST:
			if (listindex == wnd2Shared)
			{
				CString strBuffer = GetResString(IDS_SF_FILES);
				if(iCount == (size_t)-1)
					iCount = theApp.sharedfiles->GetCount();
				if(!thePrefs.IsTransToolbarEnabled())
					strBuffer.AppendFormat(_T("(%s)"), (m_dwShowListIDC == IDC_DOWNLOADLIST)?downloadlistctrl.GetFilterLabelCat():GetFilterLabel());
				if (theApp.sharedfiles->GetHashingCount() + nAICHHashing)
					strBuffer.AppendFormat(_T(" (%i, %s %i)"), iCount, GetResString(IDS_HASHING), theApp.sharedfiles->GetHashingCount() + nAICHHashing);
				else
					strBuffer.AppendFormat(_T(" (%i)"), iCount);
				m_btnWnd1->SetWindowText(strBuffer);
			}
			break;

		case IDC_DOWNHISTORYLIST:
			if (listindex == wnd2History)
			{
				CString strBuffer = GetResString(IDS_DOWNHISTORY);
				if(!thePrefs.IsTransToolbarEnabled())
					strBuffer.AppendFormat(_T("(%s)"), (m_dwShowListIDC == IDC_DOWNLOADLIST)?downloadlistctrl.GetFilterLabelCat():GetFilterLabel());
				strBuffer.AppendFormat(_T(" (%i)"), iCount == (size_t)-1 ? historylistctrl.GetItemCount() : iCount);
				m_btnWnd1->SetWindowText(strBuffer);
			}
			break;

		//default:
			//ASSERT(0);
	}
}

void CTransferWnd::ShowWnd2(EWnd2 uWnd2)
{
	if (uWnd2 == wnd2Downloading) 
	{
		if(m_uWnd2 != uWnd2){
			downloadclientsctrl.ShowWindow(SW_SHOW);
			uploadlistctrl.ShowWindow(SW_HIDE);
			thePrefs.m_uTransferWnd2 = m_uWnd2 = uWnd2;
			UpdateListCount(m_uWnd2);
		}
		m_btnWnd2->CheckButton(MP_VIEW2_DOWNLOADING);
		SetWnd2Icon(w2iDownloading);
	}
	else
	{
		if(m_uWnd2 != uWnd2){
			uploadlistctrl.ShowWindow(SW_SHOW);
			downloadclientsctrl.ShowWindow(SW_HIDE);
			thePrefs.m_uTransferWnd2 = m_uWnd2 = uWnd2;
			UpdateListCount(m_uWnd2);
		}
		m_btnWnd2->CheckButton(MP_VIEW2_UPLOADING);
		SetWnd2Icon(w2iUploading);
	}
}
/*
void CTransferWnd::OnSysColorChange()
{
	CResizableDialog::OnSysColorChange();
	SetAllIcons();
	m_btnWnd1->Invalidate();
	m_btnWnd2->Invalidate();
}
void CTransferWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CResizableDialog::OnSettingChange(uFlags, lpszSection);
	// It does not work to reset the width of 1st button here.
	//m_btnWnd1->SetBtnWidth(IDC_DOWNLOAD_ICO, WND1_BUTTON_WIDTH);
	//m_btnWnd2->SetBtnWidth(IDC_UPLOAD_ICO, WND2_BUTTON_WIDTH);
}

void CTransferWnd::SetAllIcons()
{
	SetWnd1Icons();
	SetWnd2Icons();
}
*/
void CTransferWnd::SetWnd1Icons()
{
	CImageList iml;
	iml.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 1, 1);
	iml.Add(CTempIconLoader(_T("SplitWindow")));
	iml.Add(CTempIconLoader(_T("SHAREDFILESLIST")));
	iml.Add(CTempIconLoader(_T("DOWNLOADHISTORY")));
	iml.Add(CTempIconLoader(_T("SEARCHEDIT")));
	CImageList* pImlOld = m_btnWnd1->SetImageList(&iml);
	iml.Detach();
	if (pImlOld)
		pImlOld->DeleteImageList();
}

void CTransferWnd::SetWnd2Icons()
{
	CImageList iml;
	iml.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 1, 1);
	iml.Add(CTempIconLoader(_T("Download")));
	iml.Add(CTempIconLoader(_T("Upload")));
	CImageList* pImlOld = m_btnWnd2->SetImageList(&iml);
	iml.Detach();
	if (pImlOld)
		pImlOld->DeleteImageList();
}

void CTransferWnd::LocalizeAll()// X: [RUL] - [Remove Useless Localize]
{
	Localize();
	LocalizeToolbars();
	uploadlistctrl.Localize();
	downloadlistctrl.Localize();
	downloadclientsctrl.Localize();
	sharedfilesctrl.Localize();
	historylistctrl.Localize();
	if(m_ctlFilter.m_strLastEvaluatedContent.GetLength() == 0 && m_ctlFilter.GetFocus()!=(CWnd*)&m_ctlFilter)
		m_ctlFilter.ShowColumnText(true,true);

	switch (m_dwShowListIDC){
		case IDC_DOWNLOADLIST:
			UpdateFilesCount();
			UpdateListCount(m_uWnd2);
			break;		
		case IDC_SFLIST:
			UpdateListCount(wnd2Shared);
			break;
		case IDC_DOWNHISTORYLIST:
			UpdateListCount(wnd2History);
			break;
	}
	thePrefs.GetCategory(0)->strTitle = GetResString(IDS_ALL);// X: [UIC] - [UIChange] change cat0 Title
	EditCatTabLabel(0);
	//UpdateCatTabTitles();
}

void CTransferWnd::Localize()
{
	SetDlgItemText(IDC_REFRESH_BUTTON,GetResString(IDS_SV_UPDATE));

	ShowQueueCount();
	ShowBufferUsage(0);
}

void CTransferWnd::LocalizeToolbars()
{
	//m_btnWnd1->SetWindowText(GetResString(IDS_TW_DOWNLOADS));
	m_btnWnd1->SetBtnText(MP_VIEW1_SPLIT_WINDOW, GetResString(IDS_SPLIT_WINDOW));
	m_btnWnd1->SetBtnText(MP_VIEW1_SHARED, GetResString(IDS_SF_FILES));
	m_btnWnd1->SetBtnText(MP_VIEW1_HISTORY, GetResString(IDS_DOWNHISTORY));
	UpdateFilterLabel();
	//m_btnWnd2->SetWindowText(GetResString(IDS_UPLOADING));
	m_btnWnd2->SetBtnText(MP_VIEW2_UPLOADING, GetResString(IDS_UPLOADING));
	m_btnWnd2->SetBtnText(MP_VIEW2_DOWNLOADING, GetResString(IDS_DOWNLOADING));
}

void CTransferWnd::OnBnClickedRefreshButton()
{
	if(m_dwShowListIDC == IDC_SFLIST)
		theApp.sharedfiles->Reload(thePrefs.queryOnHashing!=1);// X: [QOH] - [QueryOnHashing]
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

	CMenu menu;
	menu.CreatePopupMenu();

	menu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)PrioMenu.m_hMenu, GetResString(IDS_PRIORITY) );
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
	menu.AppendMenu(MF_STRING,MP_HM_OPENINC, GetResString(IDS_OPENINC) );

	UINT flag=(rightclickindex==0) ? MF_GRAYED:MF_STRING;
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING,MP_CAT_ADD,GetResString(IDS_CAT_ADD));
	menu.AppendMenu(flag,MP_CAT_EDIT,GetResString(IDS_CAT_EDIT));
	menu.AppendMenu(flag,MP_CAT_REMOVE, GetResString(IDS_CAT_REMOVE));

	menu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);

	VERIFY( PrioMenu.DestroyMenu() );
	VERIFY( menu.DestroyMenu() );

	*pResult = 0;
}

void CTransferWnd::OnLvnBeginDragDownloadList(NMHDR *pNMHDR, LRESULT *pResult)
{
    int iSel = downloadlistctrl.GetSelectionMark();
	if (iSel == -1)
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
				if(index > -1)
					selectedList.AddTail( (CPartFile*)downloadlistctrl.GetItemData(index) );
			}

			while (!selectedList.IsEmpty())
			{
				CPartFile *file = selectedList.RemoveHead();// X: [CI] - [Code Improvement]
				file->SetCategory(m_nDropIndex);
			}

			m_dlTab.SetCurSel(downloadlistctrl.curTab);
			downloadlistctrl.ReloadFileList();
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
		if(m_dwShowListIDC == IDC_DOWNLOADLIST){
			if(downloadlistctrl.SetFilterCat(downloadlistctrl.curTab, (int)wParam - MP_CAT_SET0))
				m_nLastCatTT=-1;
		}
		else if(m_dwShowListIDC == IDC_SFLIST)// X: [FI] - [FilterItem]
			sharedfilesctrl.SetFilter((int)wParam - MP_CAT_SET0);
		else
			historylistctrl.SetFilter((int)wParam - MP_CAT_SET0);
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
				EditCatTabLabel(rightclickindex);
				m_dlTab.SetTabTextColor(rightclickindex, thePrefs.GetCatColor(rightclickindex) );
				theApp.emuledlg->searchwnd->UpdateCatTabs();
				if(downloadlistctrl.curTab == rightclickindex){
					if(downloadlistctrl.filter != thePrefs.GetCatFilter(rightclickindex)){
						downloadlistctrl.filter = thePrefs.GetCatFilter(rightclickindex);
						downloadlistctrl.ReloadFileList();
					}
				}
				thePrefs.SaveCats();
				if (CompareDirectories(oldincpath, thePrefs.GetCatPath(rightclickindex)))
					theApp.sharedfiles->Reload(thePrefs.queryOnHashing!=1);// X: [QOH] - [QueryOnHashing]
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
			if (thePrefs.GetCatCount()==1){
				thePrefs.GetCategory(0)->filter=0;
				UpdateFilterLabel();
			}
			downloadlistctrl.ChangeCategory(0);
			thePrefs.SaveCats();
			downloadlistctrl.filter = thePrefs.GetCatFilter(0);
			theApp.emuledlg->searchwnd->UpdateCatTabs();
			if (toreload)
				theApp.sharedfiles->Reload(thePrefs.queryOnHashing!=1);// X: [QOH] - [QueryOnHashing]
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
			ShowWnd2((m_uWnd2 == wnd2Downloading)?wnd2Uploading:wnd2Downloading);
			break;
		case MP_VIEW2_UPLOADING:
			ShowWnd2(wnd2Uploading);
			break;
		case MP_VIEW2_DOWNLOADING:
			ShowWnd2(wnd2Downloading);
			break;
		case IDC_REFRESH_BUTTON:
			OnBnClickedRefreshButton();
			break;
		case IDC_DOWNLOAD_ICO:
			OnBnClickedChangeView();
			break;
		case MP_VIEW1_SPLIT_WINDOW:
			ShowList(IDC_DOWNLOADLIST);
			break;		
		case MP_VIEW1_SHARED:
			ShowList(IDC_SFLIST);
			break;

		case MP_VIEW1_HISTORY:
			ShowList(IDC_DOWNHISTORYLIST);
			break;

		case MP_VIEW1_FILTER:{
			if(m_dwShowListIDC == IDC_DOWNLOADLIST||m_dwShowListIDC == IDC_SFLIST||m_dwShowListIDC == IDC_DOWNHISTORYLIST){
				CMenu CatMenu;
				CatMenu.CreatePopupMenu();
				if(m_dwShowListIDC == IDC_DOWNLOADLIST)
					downloadlistctrl.CreateFilterMenuCat(CatMenu);
				else
					CreateFilterMenu(CatMenu);
				CRect rc;
				m_btnWnd1->GetItemRect(4,&rc); //morph4u was 6
				ClientToScreen(&rc);
				CatMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rc.left+8, rc.bottom, this);
				VERIFY( CatMenu.DestroyMenu() );
			}
			break;
		}
		case MP_HM_OPENINC:
			//ShellExecute(NULL, _T("open"), thePrefs.GetCategory(rightclickindex)->strIncomingPath,NULL, NULL, SW_SHOW);
			theDesktop.ShellExecute(_T("open"), thePrefs.GetCategory(rightclickindex)->strIncomingPath); // netfinity: Better support for WINE Gnome/KDE desktops
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
		EditCatTabLabel(i);
		m_dlTab.SetTabTextColor(i, thePrefs.GetCatColor(i) );
	}
}

void CTransferWnd::EditCatTabLabel(size_t index)
{
	TCITEM tabitem;
	tabitem.mask = TCIF_PARAM;
	m_dlTab.GetItem(index,&tabitem);
	tabitem.mask = TCIF_TEXT;
	CString newlabel = thePrefs.GetCategory(index)->strTitle;
	newlabel.Replace(_T("&"),_T("&&"));

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
				if (CompareDirectories(thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR), thePrefs.GetCatPath(newindex)))
					theApp.sharedfiles->Reload(thePrefs.queryOnHashing!=1);// X: [QOH] - [QueryOnHashing]
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
	title.Format(_T("%s: %i\n\n%s: %i\n%s: %i\n%s: %i\n\n%s: %s\n\n%s: %.1f KB/s\n%s: %s/%s\n%s%s"),
		GetResString(IDS_FILES), count,
		GetResString(IDS_DOWNLOADING), dwl,
		GetResString(IDS_PAUSED), paus,
		GetResString(IDS_ERRORLIKE), err,
        GetResString(IDS_PRIORITY), prio,
		GetResString(IDS_DL_SPEED) ,speed,
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

	// of the tabcontrol itself
	m_dlTab.ReorderTab(from,to);

	UpdateCatTabTitles();
	theApp.emuledlg->searchwnd->UpdateCatTabs();

	if (to>from) --to;
	m_dlTab.SetCurSel(to);
	downloadlistctrl.ChangeCategory(to);
}

void CTransferWnd::OnBnClickedChangeView()
{
	switch (m_dwShowListIDC)
	{
		case IDC_DOWNLOADLIST:
				ShowList(IDC_SFLIST);
				break;
		case IDC_SFLIST:
			if (!thePrefs.m_bDisableHistoryList){
				ShowList(IDC_DOWNHISTORYLIST);
				break;
			}
		case IDC_DOWNHISTORYLIST:
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
	if(m_dwShowListIDC != dwListIDC){
		switch (dwListIDC)
		{
			case IDC_DOWNLOADLIST:{
				CRect rcWnd;
				GetWindowRect(rcWnd);
				ScreenToClient(rcWnd);

				LONG splitpos = (thePrefs.GetSplitterbarPosition() * rcWnd.Height() + rcWnd.Height()/2) / 100;// X fix without using the Magic constant

				RECT rcDown = {
					0,
					thePrefs.isshowcatbar?46:23,
					rcWnd.right,
					splitpos
				};
				downloadlistctrl.MoveWindow(&rcDown);

				rcDown.bottom = rcWnd.bottom - 20;
				rcDown.top = splitpos + 25 + PARTSTATUSCTRLHEIGHT;
				uploadlistctrl.MoveWindow(&rcDown);
				downloadclientsctrl.MoveWindow(&rcDown);

				rcDown.top = splitpos + 25;
				rcDown.bottom = rcDown.top + PARTSTATUSCTRLHEIGHT;
				partstatusctrl.MoveWindow(&rcDown);

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

				m_dlTab.ShowWindow(thePrefs.isshowcatbar?SW_SHOW:SW_HIDE);
				downloadlistctrl.ShowWindow(SW_SHOW);
				m_btnWnd2->ShowWindow(SW_SHOW);
				partstatusctrl.ShowWindow(SW_SHOW);
				downloadclientsctrl.ShowWindow((m_uWnd2 == wnd2Downloading) ? SW_SHOW : SW_HIDE);
				uploadlistctrl.ShowWindow((m_uWnd2 == wnd2Uploading) ? SW_SHOW : SW_HIDE);
				thePrefs.SetTransferWnd1(0);
				break;
			}			
			case IDC_SFLIST:
				sharedfilesctrl.ShowWindow(SW_SHOW);
				GetDlgItem(IDC_REFRESH_BUTTON)->ShowWindow(SW_SHOW);
				thePrefs.SetTransferWnd1(2);
				break;
			case IDC_DOWNHISTORYLIST:
				historylistctrl.ShowWindow(SW_SHOW);
				thePrefs.SetTransferWnd1(3);
				break;
			default:
				ASSERT(0);
		}
		m_ctlFilter.ShowWindow(SW_SHOW); 
		switch (m_dwShowListIDC)
		{
			case 0:
				break;
			case IDC_DOWNLOADLIST:
				m_wndSplitter.DestroyWindow();
				m_btnWnd2->ShowWindow(SW_HIDE);
				downloadlistctrl.ShowWindow(SW_HIDE);
				downloadclientsctrl.ShowWindow(SW_HIDE);
				uploadlistctrl.ShowWindow(SW_HIDE);
				partstatusctrl.ShowWindow(SW_HIDE);
				m_dlTab.ShowWindow(SW_HIDE);
				m_ctlFilter.SaveTo(downloadlistctrl);// X: [FI] - [FilterItem]
				break;	
			case IDC_SFLIST:
				sharedfilesctrl.ShowWindow(SW_HIDE);
				m_ctlFilter.SaveTo(sharedfilesctrl);// X: [FI] - [FilterItem]
				GetDlgItem(IDC_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
				break;
			case IDC_DOWNHISTORYLIST:
				historylistctrl.ShowWindow(SW_HIDE);
				m_ctlFilter.SaveTo(historylistctrl);// X: [FI] - [FilterItem]
				break;
		}
		m_dwShowListIDC = dwListIDC;
	}
	switch (dwListIDC)
	{
		case IDC_DOWNLOADLIST:
			SetWnd1Icon(w1iSplitWindow);
			m_btnWnd1->CheckButton(MP_VIEW1_SPLIT_WINDOW);
			UpdateFilesCount();
			UpdateListCount(m_uWnd2);
			m_ctlFilter.UpdateFrom(downloadlistctrl);// X: [FI] - [FilterItem]
			filter = downloadlistctrl.filter;// X: [FI] - [FilterItem]
			break;	
		case IDC_SFLIST:
			m_btnWnd1->CheckButton(MP_VIEW1_SHARED);
			SetWnd1Icon(w1iShared);
			UpdateListCount(wnd2Shared);
			m_ctlFilter.UpdateFrom(sharedfilesctrl);// X: [FI] - [FilterItem]
			filter = sharedfilesctrl.filter;// X: [FI] - [FilterItem]
			break;
		case IDC_DOWNHISTORYLIST:
			m_btnWnd1->CheckButton(MP_VIEW1_HISTORY);
			SetWnd1Icon(w1iHistory);
			UpdateListCount(wnd2History);
			m_ctlFilter.UpdateFrom(historylistctrl);// X: [FI] - [FilterItem]
			filter = historylistctrl.filter;// X: [FI] - [FilterItem]
			break;
		default:
			ASSERT(0);
	}
	UpdateFilterLabel();// X: [FI] - [FilterItem]
}

void CTransferWnd::OnDisableList()
{
	bool bSwitchList = false;
	if (thePrefs.m_bDisableHistoryList)
	{
		historylistctrl.DeleteAllItems();
		if (m_dwShowListIDC == IDC_DOWNHISTORYLIST)
			bSwitchList = true;
	}
	if (bSwitchList)
		OnBnClickedChangeView();
}

void CTransferWnd::OnWnd1BtnDropDown(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	CMenu menu;
	menu.CreatePopupMenu();

	menu.AppendMenu(MF_STRING | (m_dwShowListIDC == IDC_DOWNLOADLIST ? MF_GRAYED : 0), MP_VIEW1_SPLIT_WINDOW, GetResString(IDS_SPLIT_WINDOW));
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING | (m_dwShowListIDC == IDC_SFLIST ? MF_GRAYED : 0), MP_VIEW1_SHARED, GetResString(IDS_SF_FILES));
	if (!thePrefs.IsHistoryListDisabled())
		menu.AppendMenu(MF_STRING | (m_dwShowListIDC == IDC_DOWNHISTORYLIST ? MF_GRAYED : 0), MP_VIEW1_HISTORY, GetResString(IDS_DOWNHISTORY));
	if(m_dwShowListIDC == IDC_DOWNLOADLIST||m_dwShowListIDC == IDC_SFLIST||m_dwShowListIDC == IDC_DOWNHISTORYLIST){
		CMenu CatMenu;
		CatMenu.CreateMenu();
		if(m_dwShowListIDC == IDC_DOWNLOADLIST)
			downloadlistctrl.CreateFilterMenuCat(CatMenu);
		else
			CreateFilterMenu(CatMenu);
		menu.AppendMenu(MF_POPUP, (UINT_PTR)CatMenu.m_hMenu, GetResString(IDS_CHANGECATVIEW));
	}

	RECT rc;
	m_btnWnd1->GetWindowRect(&rc);
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rc.left, rc.bottom, this);
	VERIFY( menu.DestroyMenu() );
}

void CTransferWnd::OnWnd2BtnDropDown(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	CMenu menu;
	menu.CreatePopupMenu();

	menu.AppendMenu(MF_STRING | (m_uWnd2 == wnd2Uploading ? MF_GRAYED : 0), MP_VIEW2_UPLOADING, GetResString(IDS_UPLOADING));
	menu.AppendMenu(MF_STRING | (m_uWnd2 == wnd2Downloading ? MF_GRAYED : 0), MP_VIEW2_DOWNLOADING, GetResString(IDS_DOWNLOADING));

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
	rcBtn1.top = 0;
	rcBtn1.left = WND1_BUTTON_XOFF;
	rcBtn1.right = /*rcBtn1.left*/WND1_BUTTON_XOFF + WND1_BUTTON_WIDTH + WND1_BUTTON6_WIDTH;
	if(bShowToolbar){
		rcBtn1.right += WND1_NUM_BUTTONS*DFLT_TOOLBAR_BTN_WIDTH;
		if(thePrefs.IsHistoryListDisabled()) // X: [Bug] Can't be Gray,so hide it
			rcBtn1.right -= DFLT_TOOLBAR_BTN_WIDTH;
	}
	rcBtn1.bottom = /*rcBtn1.top + */WND1_BUTTON_HEIGHT;
	m_btnWnd1->Init(!bShowToolbar);
	m_btnWnd1->MoveWindow(&rcBtn1);
	SetWnd1Icons();

	CRect rcBtn2;
	m_btnWnd2->GetWindowRect(rcBtn2);
	ScreenToClient(rcBtn2);
	//rcBtn2.top = rcBtn2.top;
	rcBtn2.left = WND2_BUTTON_XOFF;
	rcBtn2.right = /*rcBtn2.left*/WND2_BUTTON_XOFF + WND2_BUTTON_WIDTH;
	if(bShowToolbar)
		rcBtn2.right += WND2_NUM_BUTTONS*DFLT_TOOLBAR_BTN_WIDTH;
	rcBtn2.bottom = rcBtn2.top + WND2_BUTTON_HEIGHT;
	m_btnWnd2->Init(!bShowToolbar);
	m_btnWnd2->MoveWindow(&rcBtn2);
	SetWnd2Icons();

	if (bShowToolbar)
	{
		// Vista: Remove the TBSTYLE_TRANSPARENT to avoid flickering (can be done only after the toolbar was initially created with TBSTYLE_TRANSPARENT !?)
		m_btnWnd1->ModifyStyle((theApp.m_ullComCtrlVer >= MAKEDLLVERULL(6, 16, 0, 0)) ? TBSTYLE_TRANSPARENT : 0, TBSTYLE_TOOLTIPS);
		m_btnWnd1->SetExtendedStyle(m_btnWnd1->GetExtendedStyle() | TBSTYLE_EX_MIXEDBUTTONS);

		TBBUTTON atb1[2+WND1_NUM_BUTTONS] = {0};
		atb1[0].iBitmap = w1iSplitWindow;
		atb1[0].idCommand = IDC_DOWNLOAD_ICO;
		atb1[0].fsState = TBSTATE_ENABLED;
		atb1[0].fsStyle = BTNS_BUTTON | BTNS_SHOWTEXT;
		atb1[0].iString = -1;

		atb1[1].iBitmap = w1iSplitWindow;
		atb1[1].idCommand = MP_VIEW1_SPLIT_WINDOW;
		atb1[1].fsState = TBSTATE_ENABLED;
		atb1[1].fsStyle = BTNS_BUTTON | BTNS_CHECKGROUP | BTNS_AUTOSIZE;
		atb1[1].iString = -1;

		atb1[2].iBitmap = w1iShared;
		atb1[2].idCommand = MP_VIEW1_SHARED;
		atb1[2].fsState = TBSTATE_ENABLED;
		atb1[2].fsStyle = BTNS_BUTTON | BTNS_CHECKGROUP | BTNS_AUTOSIZE;
		atb1[2].iString = -1;

		atb1[3].iBitmap = w1iHistory;
		atb1[3].idCommand = MP_VIEW1_HISTORY;
		atb1[3].fsState = thePrefs.IsHistoryListDisabled() ? TBSTATE_HIDDEN : TBSTATE_ENABLED;// X: [Bug] Can't be Gray,so hide it
		atb1[3].fsStyle = BTNS_BUTTON | BTNS_CHECKGROUP | BTNS_AUTOSIZE;
		atb1[3].iString = -1;

		atb1[4].iBitmap = w1iFilter;
		atb1[4].idCommand = MP_VIEW1_FILTER;
		atb1[4].fsState = TBSTATE_ENABLED;
		atb1[4].fsStyle = BTNS_BUTTON | BTNS_SHOWTEXT;
		atb1[4].iString = -1;
		m_btnWnd1->AddButtons(_countof(atb1), atb1);

		TBBUTTONINFO tbbi = {0};
		tbbi.cbSize = sizeof tbbi;
		tbbi.dwMask = TBIF_SIZE | TBIF_BYINDEX;
		tbbi.cx = WND1_BUTTON_WIDTH;
		m_btnWnd1->SetButtonInfo(0, &tbbi);
		tbbi.cx = WND1_BUTTON6_WIDTH;
		m_btnWnd1->SetButtonInfo(4, &tbbi);

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
/*
		atb2[1].iBitmap = w2iDownloading;
		atb2[1].idCommand = MP_VIEW2_DOWNLOADING;
		atb2[1].fsState = TBSTATE_ENABLED;
		atb2[1].fsStyle = BTNS_BUTTON | BTNS_CHECKGROUP | BTNS_AUTOSIZE;
		atb2[1].iString = -1;

		atb2[2].iBitmap = w2iUploading;
		atb2[2].idCommand = MP_VIEW2_UPLOADING;
		atb2[2].fsState = TBSTATE_ENABLED;
		atb2[2].fsStyle = BTNS_BUTTON | BTNS_CHECKGROUP | BTNS_AUTOSIZE;
		atb2[2].iString = -1;
*/
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
	ShowList(m_dwShowListIDC);
	ShowWnd2(m_uWnd2);
}

LRESULT CTransferWnd::OnChangeFilter(WPARAM wParam, LPARAM lParam)
{
	CWaitCursor curWait; // this may take a while
	
	CFilterItem*fi;
	ASSERT(m_dwShowListIDC == IDC_DOWNLOADLIST||m_dwShowListIDC == IDC_SFLIST||m_dwShowListIDC == IDC_DOWNHISTORYLIST);
	if(m_dwShowListIDC == IDC_DOWNLOADLIST)
		fi=&downloadlistctrl;
	else if(m_dwShowListIDC == IDC_SFLIST)
		fi=&sharedfilesctrl;
	else
		fi=&historylistctrl;
	bool bColumnDiff = (fi->m_nFilterColumn != (uint32)wParam);
	fi->m_nFilterColumn = (uint32)wParam;

	CAtlArray<CString> astrFilter;
	CString strFullFilterExpr = (LPCTSTR)lParam;
	int iPos = 0;
	CString strFilter(strFullFilterExpr.Tokenize(_T(" "), iPos));
	while (!strFilter.IsEmpty()) {
		if (strFilter != _T("-"))
			astrFilter.Add(strFilter);
		strFilter = strFullFilterExpr.Tokenize(_T(" "), iPos);
	}

	CAtlArray<CString>& m_astrFilter=fi->m_astrFilter;
	bool bFilterDiff = (astrFilter.GetCount() != m_astrFilter.GetCount());
	if (!bFilterDiff) {
		for (size_t i = 0; i < astrFilter.GetCount(); i++) {
			if (astrFilter[i] != m_astrFilter[i]) {
				bFilterDiff = true;
				break;
			}
		}
	}

	if (!bColumnDiff && !bFilterDiff)
		return 0;
	m_astrFilter.RemoveAll();
	m_astrFilter.Append(astrFilter);
	fi->ReloadFileList();
	return 0;
}

HBRUSH CTransferWnd::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = theApp.emuledlg->GetCtlColor(pDC, pWnd, nCtlColor);
	if (hbr)
		return hbr;
	return __super::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CTransferWnd::UpdateFilterLabel()// X: [FI] - [FilterItem]
{
	m_btnWnd1->SetBtnText(MP_VIEW1_FILTER, 
		(m_dwShowListIDC == IDC_DOWNLOADLIST)?
			downloadlistctrl.GetFilterLabelCat()
		:
			(m_dwShowListIDC == IDC_SFLIST||m_dwShowListIDC == IDC_DOWNHISTORYLIST)?
				GetFilterLabel()
			:
				_T("")
	);
}

void CTransferWnd::ShowCatTab(){
	if(m_dwShowListIDC == IDC_DOWNLOADLIST){
		RECT rcDown;
		downloadlistctrl.GetWindowRect(&rcDown);
		ScreenToClient(&rcDown);

		rcDown.top = thePrefs.isshowcatbar?46:23;
		downloadlistctrl.MoveWindow(&rcDown);
		m_dlTab.ShowWindow(thePrefs.isshowcatbar?SW_SHOW:SW_HIDE);
	}
}