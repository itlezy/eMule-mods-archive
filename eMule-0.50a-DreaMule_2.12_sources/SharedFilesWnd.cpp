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
#include "emuleDlg.h"
#include "SharedFilesWnd.h"
#include "OtherFunctions.h"
#include "SharedFileList.h"
#include "KnownFileList.h"
#include "KnownFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	SPLITTER_RANGE_MIN		100
#define	SPLITTER_RANGE_MAX		350

#define	SPLITTER_MARGIN			1
#define	SPLITTER_WIDTH			4


// CSharedFilesWnd dialog

IMPLEMENT_DYNAMIC(CSharedFilesWnd, CDialog)

BEGIN_MESSAGE_MAP(CSharedFilesWnd, CResizableDialog)
	ON_BN_CLICKED(IDC_RELOADSHAREDFILES, OnBnClickedReloadsharedfiles)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_SFLIST, OnLvnItemActivateSflist)
	ON_NOTIFY(NM_CLICK, IDC_SFLIST, OnNMClickSflist)
	ON_WM_SYSCOLORCHANGE()
	ON_STN_DBLCLK(IDC_FILES_ICO, OnStnDblclickFilesIco)
	ON_NOTIFY(TVN_SELCHANGED, IDC_SHAREDDIRSTREE, OnTvnSelchangedShareddirstree)
	ON_WM_SIZE()
	//Xman [MoNKi: -Downloaded History-]
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_DOWNHISTORYLIST, OnLvnItemActivateHistorylist)
	ON_NOTIFY(NM_CLICK, IDC_DOWNHISTORYLIST, OnNMClickHistorylist)
	ON_WM_SHOWWINDOW() 
	//Xman end
END_MESSAGE_MAP()

CSharedFilesWnd::CSharedFilesWnd(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CSharedFilesWnd::IDD, pParent)
{
	icon_files = NULL;
}

CSharedFilesWnd::~CSharedFilesWnd()
{
	if (icon_files)
		VERIFY( DestroyIcon(icon_files) );
}

void CSharedFilesWnd::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SFLIST, sharedfilesctrl);
	DDX_Control(pDX, IDC_POPBAR, pop_bar);
	DDX_Control(pDX, IDC_POPBAR2, pop_baraccept);
	DDX_Control(pDX, IDC_POPBAR3, pop_bartrans);
	DDX_Control(pDX, IDC_STATISTICS, m_ctrlStatisticsFrm);
	DDX_Control(pDX, IDC_SHAREDDIRSTREE, m_ctlSharedDirTree);
	//Xman [MoNKi: -Downloaded History-]
	DDX_Control(pDX, IDC_DOWNHISTORYLIST, historylistctrl);
	//Xman end
}

BOOL CSharedFilesWnd::OnInitDialog()
{
	CResizableDialog::OnInitDialog();
	InitWindowStyles(this);
	SetAllIcons();
	sharedfilesctrl.Init();
	m_ctlSharedDirTree.Initalize(&sharedfilesctrl);
	
	pop_bar.SetGradientColors(RGB(255,255,240),RGB(255,255,0));
	pop_bar.SetTextColor(RGB(20,70,255));
	pop_baraccept.SetGradientColors(RGB(255,255,240),RGB(255,255,0));
	pop_baraccept.SetTextColor(RGB(20,70,255));
	pop_bartrans.SetGradientColors(RGB(255,255,240),RGB(255,255,0));
	pop_bartrans.SetTextColor(RGB(20,70,255));

	LOGFONT lf;
	GetFont()->GetLogFont(&lf);
	lf.lfWeight = FW_BOLD;
	bold.CreateFontIndirect(&lf);

	CRect rc;
	GetDlgItem(IDC_SHAREDDIRSTREE)->GetWindowRect(rc);
	ScreenToClient(rc);

	CRect rcSpl;
	GetDlgItem(IDC_SHAREDDIRSTREE)->GetWindowRect(rcSpl);
	ScreenToClient(rcSpl);

	rcSpl.left = rcSpl.right + SPLITTER_MARGIN;
	rcSpl.right = rcSpl.left + SPLITTER_WIDTH;
	m_wndSplitter.Create(WS_CHILD | WS_VISIBLE, rcSpl, this, IDC_SPLITTER_SHAREDFILES);

	int PosStatVinit = rcSpl.left;
	int PosStatVnew = rc.left + thePrefs.GetSplitterbarPositionShared() + 2;

	if (thePrefs.GetSplitterbarPositionShared() > SPLITTER_RANGE_MAX)
		PosStatVnew = SPLITTER_RANGE_MAX;
	else if (thePrefs.GetSplitterbarPositionShared() < SPLITTER_RANGE_MIN)
		PosStatVnew = SPLITTER_RANGE_MIN;
	rcSpl.left = PosStatVnew;
	rcSpl.right = PosStatVnew + SPLITTER_WIDTH;
	m_wndSplitter.MoveWindow(rcSpl);

	//Xman [MoNKi: -Downloaded History-]
	{
	CRect tmpRect;
	sharedfilesctrl.GetWindowRect(tmpRect);
	ScreenToClient(tmpRect);
	historylistctrl.MoveWindow(tmpRect,true);
	historylistctrl.ShowWindow(false);
	//historylistctrl.Init(); //Xman SLUGFILLER: SafeHash - moved
	AddAnchor(IDC_DOWNHISTORYLIST,TOP_LEFT,BOTTOM_RIGHT);
	}
	//Xman end

	DoResize(PosStatVnew - PosStatVinit);

    
	Localize();

	GetDlgItem(IDC_CURSESSION_LBL)->SetFont(&bold);
	GetDlgItem(IDC_TOTAL_LBL)->SetFont(&bold);
	
	AddAnchor(IDC_FILES_ICO, TOP_LEFT);
	AddAnchor(IDC_RELOADSHAREDFILES, TOP_RIGHT);
	AddAnchor(IDC_TOTAL_LBL, BOTTOM_RIGHT);
	AddAnchor(IDC_SREQUESTED2,BOTTOM_RIGHT);
	AddAnchor(IDC_FSTATIC7,BOTTOM_RIGHT);
	AddAnchor(IDC_FSTATIC8,BOTTOM_RIGHT);
	AddAnchor(IDC_FSTATIC9,BOTTOM_RIGHT);
	AddAnchor(IDC_STRANSFERRED2,BOTTOM_RIGHT);
	AddAnchor(IDC_SACCEPTED2,BOTTOM_RIGHT);
	AddAnchor(IDC_TRAFFIC_TEXT, TOP_LEFT);

	
	
	return TRUE;
}

void CSharedFilesWnd::DoResize(int delta)
{


	CSplitterControl::ChangeWidth(GetDlgItem(IDC_SHAREDDIRSTREE), delta);

	CSplitterControl::ChangePos(GetDlgItem(IDC_CURSESSION_LBL), -delta, 0);
	CSplitterControl::ChangePos(GetDlgItem(IDC_FSTATIC4), -delta, 0);
	CSplitterControl::ChangePos(GetDlgItem(IDC_SREQUESTED), -delta, 0);
	CSplitterControl::ChangePos(GetDlgItem(IDC_FSTATIC5), -delta, 0);
	CSplitterControl::ChangePos(GetDlgItem(IDC_SACCEPTED), -delta, 0);

	CSplitterControl::ChangePos(GetDlgItem(IDC_FSTATIC6), -delta, 0);
	CSplitterControl::ChangePos(GetDlgItem(IDC_STRANSFERRED), -delta, 0);

	CSplitterControl::ChangePos(GetDlgItem(IDC_STATISTICS), -delta, 0);
	CSplitterControl::ChangePos(GetDlgItem(IDC_SFLIST), -delta, 0);
	CSplitterControl::ChangeWidth(GetDlgItem(IDC_STATISTICS), -delta);
	CSplitterControl::ChangeWidth(GetDlgItem(IDC_SFLIST), -delta);
	
	//Xman [MoNKi: -Downloaded History-]
	CSplitterControl::ChangePos(GetDlgItem(IDC_DOWNHISTORYLIST), -delta, 0);
	CSplitterControl::ChangeWidth(GetDlgItem(IDC_DOWNHISTORYLIST), -delta);
	//Xman end

	CSplitterControl::ChangePos(GetDlgItem(IDC_POPBAR), -delta, 0);
	CSplitterControl::ChangePos(GetDlgItem(IDC_POPBAR2), -delta, 0);
	CSplitterControl::ChangePos(GetDlgItem(IDC_POPBAR3), -delta, 0);
	CSplitterControl::ChangeWidth(GetDlgItem(IDC_POPBAR), -delta);
	CSplitterControl::ChangeWidth(GetDlgItem(IDC_POPBAR2), -delta);
	CSplitterControl::ChangeWidth(GetDlgItem(IDC_POPBAR3), -delta);

	CRect rcW;
	GetWindowRect(rcW);
	ScreenToClient(rcW);

	CRect rcspl;
	GetDlgItem(IDC_SHAREDDIRSTREE)->GetClientRect(rcspl);

	thePrefs.SetSplitterbarPositionShared(rcspl.right + SPLITTER_MARGIN);

	RemoveAnchor(m_wndSplitter);
	AddAnchor(m_wndSplitter, TOP_LEFT);

	//Xman [MoNKi: -Downloaded History-]
	RemoveAnchor(IDC_DOWNHISTORYLIST);
	//Xman end
	RemoveAnchor(IDC_SFLIST);
	RemoveAnchor(IDC_STATISTICS);
	RemoveAnchor(IDC_CURSESSION_LBL);
	RemoveAnchor(IDC_FSTATIC4);
	RemoveAnchor(IDC_SREQUESTED);
	RemoveAnchor(IDC_POPBAR);
	RemoveAnchor(IDC_FSTATIC5);
	RemoveAnchor(IDC_SACCEPTED);
	RemoveAnchor(IDC_POPBAR2);
	RemoveAnchor(IDC_FSTATIC6);
	RemoveAnchor(IDC_STRANSFERRED);
	RemoveAnchor(IDC_POPBAR3);
	RemoveAnchor(IDC_SHAREDDIRSTREE);

	//Xman [MoNKi: -Downloaded History-]
	AddAnchor(IDC_DOWNHISTORYLIST,TOP_LEFT,BOTTOM_RIGHT);
	//Xman end
	AddAnchor(IDC_SFLIST,TOP_LEFT,BOTTOM_RIGHT);
	AddAnchor(IDC_STATISTICS,BOTTOM_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_CURSESSION_LBL, BOTTOM_LEFT);
	AddAnchor(IDC_FSTATIC4, BOTTOM_LEFT);
	AddAnchor(IDC_SREQUESTED,BOTTOM_LEFT);
	AddAnchor(IDC_FSTATIC5,BOTTOM_LEFT);
	AddAnchor(IDC_SACCEPTED,BOTTOM_LEFT);
	AddAnchor(IDC_FSTATIC6,BOTTOM_LEFT);
	AddAnchor(IDC_STRANSFERRED,BOTTOM_LEFT);
	AddAnchor(IDC_SHAREDDIRSTREE,TOP_LEFT, BOTTOM_LEFT);
	AddAnchor(IDC_POPBAR,BOTTOM_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_POPBAR2,BOTTOM_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_POPBAR3,BOTTOM_LEFT, BOTTOM_RIGHT);

	m_wndSplitter.SetRange(rcW.left+SPLITTER_RANGE_MIN, rcW.left+SPLITTER_RANGE_MAX);

	Invalidate();
	UpdateWindow();
}


void CSharedFilesWnd::Reload()
{	
	sharedfilesctrl.SetDirectoryFilter(NULL, false);
	m_ctlSharedDirTree.Reload();
	sharedfilesctrl.SetDirectoryFilter(m_ctlSharedDirTree.GetSelectedFilter(), false);
	theApp.sharedfiles->Reload();

	ShowSelectedFilesSummary();
}

void CSharedFilesWnd::OnStnDblclickFilesIco()
{
	theApp.emuledlg->ShowPreferences(IDD_PPG_DIRECTORIES);
}

void CSharedFilesWnd::OnBnClickedReloadsharedfiles()
{
	CWaitCursor curWait;
	Reload();
}

void CSharedFilesWnd::OnLvnItemActivateSflist(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	ShowSelectedFilesSummary();
}

void CSharedFilesWnd::ShowSelectedFilesSummary(bool bHistory /*=false*/) //Xman [MoNKi: -Downloaded History-]
{
	const CKnownFile* pTheFile = NULL;
	int iFiles = 0;
	uint64 uTransferred = 0;
	UINT uRequests = 0;
	UINT uAccepted = 0;
	uint64 uAllTimeTransferred = 0;
	UINT uAllTimeRequests = 0;
	UINT uAllTimeAccepted = 0;
	//Xman [MoNKi: -Downloaded History-]
	POSITION pos;
	if(bHistory){
		pos = historylistctrl.GetFirstSelectedItemPosition();
	}
	else{
		pos = sharedfilesctrl.GetFirstSelectedItemPosition();
	}
	//Xman end
	while (pos)
	{
		//Xman [MoNKi: -Downloaded History-]
		/* int iItem = sharedfilesctrl.GetNextSelectedItem(pos);
		const CKnownFile* pFile = (CKnownFile*)sharedfilesctrl.GetItemData(iItem); */
		int iItem;
		const CKnownFile* pFile;
		if(bHistory){
			iItem = historylistctrl.GetNextSelectedItem(pos);
			pFile = (CKnownFile*)historylistctrl.GetItemData(iItem);
		}
		else{
			iItem = sharedfilesctrl.GetNextSelectedItem(pos);
			pFile = (CKnownFile*)sharedfilesctrl.GetItemData(iItem);
		}
		//Xman end
		iFiles++;
		if (iFiles == 1)
			pTheFile = pFile;

		uTransferred += pFile->statistic.GetTransferred();
		uRequests += pFile->statistic.GetRequests();
		uAccepted += pFile->statistic.GetAccepts();

		uAllTimeTransferred += pFile->statistic.GetAllTimeTransferred();
		uAllTimeRequests += pFile->statistic.GetAllTimeRequests();
		uAllTimeAccepted += pFile->statistic.GetAllTimeAccepts();
	}

	if (iFiles != 0)
	{
		pop_bartrans.SetRange32(0, (int)(theApp.knownfiles->transferred/1024));
		pop_bartrans.SetPos((int)(uTransferred/1024));
		pop_bartrans.SetShowPercent();
		SetDlgItemText(IDC_STRANSFERRED, CastItoXBytes(uTransferred, false, false));

		pop_bar.SetRange32(0, theApp.knownfiles->requested);
		pop_bar.SetPos(uRequests);
		pop_bar.SetShowPercent();
		SetDlgItemInt(IDC_SREQUESTED, uRequests, FALSE);

		pop_baraccept.SetRange32(0, theApp.knownfiles->accepted);
		pop_baraccept.SetPos(uAccepted);
		pop_baraccept.SetShowPercent();
		SetDlgItemInt(IDC_SACCEPTED, uAccepted, FALSE);

		SetDlgItemText(IDC_STRANSFERRED2, CastItoXBytes(uAllTimeTransferred, false, false));
		SetDlgItemInt(IDC_SREQUESTED2, uAllTimeRequests, FALSE);
		SetDlgItemInt(IDC_SACCEPTED2, uAllTimeAccepted, FALSE);

		CString str(GetResString(IDS_SF_STATISTICS));
		if (iFiles == 1 && pTheFile != NULL)
			str += _T(" (") + MakeStringEscaped(pTheFile->GetFileName()) +_T(")");
		m_ctrlStatisticsFrm.SetWindowText(str);
	}
	else
	{
		pop_bartrans.SetRange32(0, 100);
		pop_bartrans.SetPos(0);
		pop_bartrans.SetTextFormat(_T(""));
		SetDlgItemText(IDC_STRANSFERRED, _T("-"));

		pop_bar.SetRange32(0, 100);
		pop_bar.SetPos(0);
		pop_bar.SetTextFormat(_T(""));
		SetDlgItemText(IDC_SREQUESTED, _T("-"));

		pop_baraccept.SetRange32(0, 100);
		pop_baraccept.SetPos(0);
		pop_baraccept.SetTextFormat(_T(""));
		SetDlgItemText(IDC_SACCEPTED, _T("-"));

		SetDlgItemText(IDC_STRANSFERRED2, _T("-"));
		SetDlgItemText(IDC_SREQUESTED2, _T("-"));
		SetDlgItemText(IDC_SACCEPTED2, _T("-"));

		m_ctrlStatisticsFrm.SetWindowText(GetResString(IDS_SF_STATISTICS));
	}
}

void CSharedFilesWnd::OnNMClickSflist(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnLvnItemActivateSflist(pNMHDR,pResult);
	*pResult = 0;
}

BOOL CSharedFilesWnd::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN)
	{
		// Don't handle Ctrl+Tab in this window. It will be handled by main window.
		if (pMsg->wParam == VK_TAB && GetAsyncKeyState(VK_CONTROL) < 0)
			return FALSE;
	}
	else if (pMsg->message == WM_KEYUP)
	{

	   if (pMsg->hwnd == GetDlgItem(IDC_SFLIST)->m_hWnd)
			OnLvnItemActivateSflist(0, 0);
   }
	else if (pMsg->message == WM_MBUTTONUP)
	{
		POINT point;
		::GetCursorPos(&point);
		CPoint p = point; 
		sharedfilesctrl.ScreenToClient(&p); 
		int it = sharedfilesctrl.HitTest(p); 
		if (it == -1)
			return FALSE;

		sharedfilesctrl.SetItemState(-1, 0, LVIS_SELECTED);
		sharedfilesctrl.SetItemState(it, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		sharedfilesctrl.SetSelectionMark(it);   // display selection mark correctly! 
		sharedfilesctrl.ShowComments((CKnownFile*)sharedfilesctrl.GetItemData(it));
		return TRUE;
   }

   return CResizableDialog::PreTranslateMessage(pMsg);
}

void CSharedFilesWnd::OnSysColorChange()
{
	pop_bar.SetBkColor(GetSysColor(COLOR_3DFACE));
	pop_baraccept.SetBkColor(GetSysColor(COLOR_3DFACE));
	pop_bartrans.SetBkColor(GetSysColor(COLOR_3DFACE));
	CResizableDialog::OnSysColorChange();
	SetAllIcons();
}

void CSharedFilesWnd::SetAllIcons()
{
	m_ctrlStatisticsFrm.SetIcon(_T("StatsDetail"));
	if (icon_files)
		VERIFY( DestroyIcon(icon_files) );
	//Xman [MoNKi: -Downloaded History-]
	if(!historylistctrl.IsWindowVisible())
	{
		icon_files = theApp.LoadIcon(_T("SharedFilesList"), 16, 16);
		((CStatic*)GetDlgItem(IDC_FILES_ICO))->SetIcon(icon_files);
	}
	else
	{
		icon_files = theApp.LoadIcon(_T("DOWNLOAD"), 16, 16);
		((CStatic*)GetDlgItem(IDC_FILES_ICO))->SetIcon(icon_files);
	}
	//Xman end
}

void CSharedFilesWnd::Localize()
{
	sharedfilesctrl.Localize();
	historylistctrl.Localize(); //Xman [MoNKi: -Downloaded History-]
	m_ctlSharedDirTree.Localize();
	sharedfilesctrl.SetDirectoryFilter(NULL,true);

	//GetDlgItem(IDC_TRAFFIC_TEXT)->SetWindowText(GetResString(IDS_SF_FILES)); //Xman [MoNKi: -Downloaded History-]
	GetDlgItem(IDC_RELOADSHAREDFILES)->SetWindowText(GetResString(IDS_SF_RELOAD));
	m_ctrlStatisticsFrm.SetWindowText(GetResString(IDS_SF_STATISTICS));
	GetDlgItem(IDC_CURSESSION_LBL)->SetWindowText(GetResString(IDS_SF_CURRENT));
	GetDlgItem(IDC_TOTAL_LBL)->SetWindowText(GetResString(IDS_SF_TOTAL));
	GetDlgItem(IDC_FSTATIC6)->SetWindowText(GetResString(IDS_SF_TRANS));
	GetDlgItem(IDC_FSTATIC5)->SetWindowText(GetResString(IDS_SF_ACCEPTED));
	GetDlgItem(IDC_FSTATIC4)->SetWindowText(GetResString(IDS_SF_REQUESTS)+_T(":"));
	GetDlgItem(IDC_FSTATIC9)->SetWindowText(GetResString(IDS_SF_TRANS));
	GetDlgItem(IDC_FSTATIC8)->SetWindowText(GetResString(IDS_SF_ACCEPTED));
	GetDlgItem(IDC_FSTATIC7)->SetWindowText(GetResString(IDS_SF_REQUESTS)+_T(":"));

	/*
	//Xman [MoNKi: -Downloaded History-]
	if(!historylistctrl.IsWindowVisible()){
		sharedfilesctrl.ShowFilesCount();
	}
	else
	{
		CString str;
		str.Format(_T(" (%i)"),historylistctrl.GetItemCount());
		GetDlgItem(IDC_TRAFFIC_TEXT)->SetWindowText(GetResString(IDS_DOWNHISTORY) + str);
	}
	//Xman end
	*/
}

void CSharedFilesWnd::OnTvnSelchangedShareddirstree(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	//Xman [MoNKi: -Downloaded History-]
	if(m_ctlSharedDirTree.GetSelectedFilter() == m_ctlSharedDirTree.pHistory)
	{
		if(!historylistctrl.IsWindowVisible())
		{
			sharedfilesctrl.ShowWindow(false);
			historylistctrl.ShowWindow(true);
			SetAllIcons();
			CString str;
			str.Format(_T(" (%i)"),historylistctrl.GetItemCount());
			GetDlgItem(IDC_TRAFFIC_TEXT)->SetWindowText(GetResString(IDS_DOWNHISTORY) + str);
			GetDlgItem(IDC_RELOADSHAREDFILES)->ShowWindow(false);
		}
	}
	else
	{
		if(!sharedfilesctrl.IsWindowVisible()) 
		{
			sharedfilesctrl.ShowWindow(true);
			historylistctrl.ShowWindow(false);
			SetAllIcons();
			GetDlgItem(IDC_RELOADSHAREDFILES)->ShowWindow(true);
			sharedfilesctrl.ShowFilesCount(); //Xman Code Improvement for ShowFilesCount
		}
		sharedfilesctrl.SetDirectoryFilter(m_ctlSharedDirTree.GetSelectedFilter(), !m_ctlSharedDirTree.IsCreatingTree());
	}
	//Xman end
	*pResult = 0;
}

LRESULT CSharedFilesWnd::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message)
	{
	case WM_PAINT:
		if (m_wndSplitter)
		{
			CRect rcW;
			GetWindowRect(rcW);
			ScreenToClient(rcW);
			if (rcW.Width() > 0)
			{
				CRect rctree;
				GetDlgItem(IDC_SHAREDDIRSTREE)->GetWindowRect(rctree);
				ScreenToClient(rctree);
				CRect rcSpl;
				rcSpl.left = rctree.right + SPLITTER_MARGIN;
				rcSpl.right = rcSpl.left + SPLITTER_WIDTH;
				rcSpl.top = rctree.top;
				rcSpl.bottom = rctree.bottom;
				m_wndSplitter.MoveWindow(rcSpl, TRUE);

			}
		}
		break;

	case WM_NOTIFY:
		if (wParam == IDC_SPLITTER_SHAREDFILES)
		{ 
			SPC_NMHDR* pHdr = (SPC_NMHDR*)lParam;
			DoResize(pHdr->delta);
		}
		break;

	case WM_WINDOWPOSCHANGED:
		{
			CRect rcW;
			GetWindowRect(rcW);
			ScreenToClient(rcW);
			if (m_wndSplitter && rcW.Width()>0)
				Invalidate();
			break;
		}
	case WM_SIZE:
		if (m_wndSplitter)
		{
			CRect rc;
			GetWindowRect(rc);
			ScreenToClient(rc);
			m_wndSplitter.SetRange(rc.left+SPLITTER_RANGE_MIN, rc.left+SPLITTER_RANGE_MAX);
		}
		break;
	}
	return CResizableDialog::DefWindowProc(message, wParam, lParam);
}

void CSharedFilesWnd::OnSize(UINT nType, int cx, int cy){
	CResizableDialog::OnSize(nType, cx, cy);
}

//Xman [MoNKi: -Downloaded History-]
void CSharedFilesWnd::OnNMClickHistorylist(NMHDR *pNMHDR, LRESULT *pResult){
	OnLvnItemActivateHistorylist(pNMHDR,pResult);
	*pResult = 0;
}

void CSharedFilesWnd::OnLvnItemActivateHistorylist(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	ShowSelectedFilesSummary(true);
}

void CSharedFilesWnd::OnShowWindow( BOOL bShow,UINT /*nStatus*/ )
{
	if(bShow && m_ctlSharedDirTree.GetSelectedFilter() == m_ctlSharedDirTree.pHistory)
	{
		CString str;
		str.Format(_T(" (%i)"),historylistctrl.GetItemCount());
		GetDlgItem(IDC_TRAFFIC_TEXT)->SetWindowText(GetResString(IDS_DOWNHISTORY) + str);
	}
	//Xman Code Improvement for ShowFilesCount
	else if(bShow)
		sharedfilesctrl.ShowFilesCount();
	//Xman end
}
//Xman end
