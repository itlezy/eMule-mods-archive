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
#include "UserMsgs.h"
#include "HighColorTab.hpp"
#include "filedetaildlgstatistics.h"
#include "ED2kLinkDlg.h"
#include "FileInfoDialog.h"
#include "ArchivePreviewDlg.h"
#include "MetaDataDlg.h"
#include "DropDownButton.h"
#include "preferences.h"
#include "MenuCmds.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define SPLITTER_RANGE_MIN 10//100->10
#define	SPLITTER_RANGE_MAX		350

#define	SPLITTER_MARGIN			0
#define	SPLITTER_WIDTH			4
// NEO: AKF - [AllKnownFiles] -- Xanatos -->
#define	DFLT_TOOLBAR_BTN_WIDTH	27

#define	WNDS_BUTTON_XOFF	8
#define	WNDS_BUTTON_WIDTH	210
#define	WNDS_BUTTON3_WIDTH	130
#define	WNDS_BUTTON_HEIGHT	22	// don't set the height do something different than 22 unless you know exactly what you are doing!
#define	NUMS_WINA_BUTTONS	2
// NEO: AKF END <-- Xanatos --

// CSharedFilesWnd dialog

IMPLEMENT_DYNAMIC(CSharedFilesWnd, CDialog)

BEGIN_MESSAGE_MAP(CSharedFilesWnd, CResizableDialog)
	//ON_BN_CLICKED(IDC_RELOADSHAREDFILES, OnBnClickedReloadSharedFiles)// NEO: AKF - [AllKnownFiles] -- Xanatos -->
	ON_MESSAGE(UM_DELAYED_EVALUATE, OnChangeFilter)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_SFLIST, OnLvnItemActivateFiles)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_DOWNHISTORYLIST, OnLvnItemActivateFiles)
	ON_NOTIFY(NM_CLICK, IDC_SFLIST, OnLvnItemActivateFiles)
	ON_NOTIFY(TVN_SELCHANGED, IDC_SHAREDDIRSTREE, OnTvnSelChangedSharedDirsTree)
	ON_NOTIFY(NM_CLICK, IDC_DOWNHISTORYLIST, OnLvnItemActivateFiles)
	ON_WM_CTLCOLOR()
//	ON_STN_DBLCLK(IDC_FILES_ICO, OnStnDblClickFilesIco)
	ON_NOTIFY(TBN_DROPDOWN, IDC_FILES_ICO, OnWndSBtnDropDown) // NEO: AKF - [AllKnownFiles] <-- Xanatos --
	ON_WM_SYSCOLORCHANGE()
	//ON_BN_CLICKED(IDC_SF_HIDESHOWDETAILS, OnBnClickedSfHideshowdetails)// NEO: AKF - [AllKnownFiles] -- Xanatos -->
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_SFLIST, OnLvnItemchangedSflist)
	//Xman [MoNKi: -Downloaded History-]
	ON_WM_SHOWWINDOW() 
	//Xman end
END_MESSAGE_MAP()

CSharedFilesWnd::CSharedFilesWnd(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CSharedFilesWnd::IDD, pParent)
{
//	icon_files = NULL;
	m_btnWndS = new CDropDownButton; // NEO: AKF - [AllKnownFiles] <-- Xanatos --
	nAICHHashing = 0;
	m_ShowAllKnow = false; // NEO: AKF - [AllKnownFiles] <-- Xanatos --
	m_bDetailsVisible = true;
}

CSharedFilesWnd::~CSharedFilesWnd()
{
	delete m_btnWndS;  // NEO: AKF - [AllKnownFiles] <-- Xanatos --
//	if (icon_files)
//		VERIFY( DestroyIcon(icon_files) );
}

void CSharedFilesWnd::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SFLIST, sharedfilesctrl);
	DDX_Control(pDX, IDC_SHAREDDIRSTREE, m_ctlSharedDirTree);
	DDX_Control(pDX, IDC_SHAREDFILES_FILTER, m_ctlFilter);
	DDX_Control(pDX, IDC_FILES_ICO, *m_btnWndS); // NEO: AKF - [AllKnownFiles] <-- Xanatos --
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
	historylistctrl.Init();
	historylistctrl.dlg = sharedfilesctrl.dlg = this;
	m_ctlFilter.OnInit(sharedfilesctrl);
	m_ctlSharedDirTree.Initalize(&sharedfilesctrl);
	if (thePrefs.GetUseSystemFontForMainControls())
		m_ctlSharedDirTree.SendMessage(WM_SETFONT, NULL, FALSE);


	RECT rcSpl;
	m_ctlSharedDirTree.GetWindowRect(&rcSpl);
	ScreenToClient(&rcSpl);

	CRect rcFiles;
	sharedfilesctrl.GetWindowRect(rcFiles);
	ScreenToClient(rcFiles);
	VERIFY( m_dlgDetails.Create(this, DS_CONTROL | DS_SETFONT | WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, WS_EX_CONTROLPARENT) );
	m_dlgDetails.SetWindowPos(NULL, rcFiles.left - 6, rcFiles.bottom - 2, rcFiles.Width() + 12, rcSpl.bottom + 7 - (rcFiles.bottom - 2), 0);
	AddAnchor(m_dlgDetails, BOTTOM_LEFT, BOTTOM_RIGHT);

	rcSpl.left = rcSpl.right + SPLITTER_MARGIN;
	rcSpl.right = rcSpl.left + SPLITTER_WIDTH;
	m_wndSplitter.Create(WS_CHILD | WS_VISIBLE, rcSpl, this, IDC_SPLITTER_SHAREDFILES);
	
	AddAnchor(IDC_SF_HIDESHOWDETAILS, BOTTOM_RIGHT);

	//Xman [MoNKi: -Downloaded History-]
	historylistctrl.MoveWindow(rcFiles,true);
	historylistctrl.ShowWindow(false);
	//Xman end
	ResetShareToolbar(thePrefs.m_bWinaTransToolbar); 
	//ChangeView(false);


	AddAnchor(m_wndSplitter, TOP_LEFT);
	AddAnchor(sharedfilesctrl, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(historylistctrl,TOP_LEFT,BOTTOM_RIGHT);
	AddAnchor(m_ctlSharedDirTree, TOP_LEFT, BOTTOM_LEFT);
	AddAnchor(m_ctlFilter, TOP_LEFT);
//	AddAnchor(IDC_FILES_ICO, TOP_LEFT);
	AddAnchor(IDC_RELOADSHAREDFILES, TOP_RIGHT);
//	AddAnchor(IDC_TRAFFIC_TEXT, TOP_LEFT);

	int iPosStatInit = rcSpl.left;
	int iPosStatNew = thePrefs.GetSplitterbarPositionShared();
	if (iPosStatNew > SPLITTER_RANGE_MAX)
		iPosStatNew = SPLITTER_RANGE_MAX;
	else if (iPosStatNew < SPLITTER_RANGE_MIN)
		iPosStatNew = SPLITTER_RANGE_MIN;
	rcSpl.left = iPosStatNew;
	rcSpl.right = iPosStatNew + SPLITTER_WIDTH;
	if (iPosStatNew != iPosStatInit)
	{
		m_wndSplitter.MoveWindow(&rcSpl);
		DoResize(iPosStatNew - iPosStatInit);
	}

	GetDlgItem(IDC_SF_HIDESHOWDETAILS)->SetFont(&theApp.m_fontSymbol);
	GetDlgItem(IDC_SF_HIDESHOWDETAILS)->SetWindowText(_T("6"));
	GetDlgItem(IDC_SF_HIDESHOWDETAILS)->BringWindowToTop();
	ShowDetailsPanel(thePrefs.GetShowSharedFilesDetails());
	Localize();
	return TRUE;
}

void CSharedFilesWnd::DoResize(int iDelta)
{
	if(!iDelta)
		return;

	CSplitterControl::ChangeWidth(&m_ctlSharedDirTree, iDelta);
	CSplitterControl::ChangeWidth(&m_ctlFilter, iDelta);
	CSplitterControl::ChangePos(&sharedfilesctrl, -iDelta, 0);
	CSplitterControl::ChangeWidth(&sharedfilesctrl, -iDelta);
	bool bAntiFlicker = m_dlgDetails.IsWindowVisible() == TRUE;
	if (bAntiFlicker)
		m_dlgDetails.SetRedraw(FALSE);
	CSplitterControl::ChangePos(&m_dlgDetails, -iDelta, 0);
	CSplitterControl::ChangeWidth(&m_dlgDetails, -iDelta);
	if (bAntiFlicker)
		m_dlgDetails.SetRedraw(TRUE);
	//Xman [MoNKi: -Downloaded History-]
	CSplitterControl::ChangePos(&historylistctrl, -iDelta, 0);
	CSplitterControl::ChangeWidth(&historylistctrl, -iDelta);
	//Xman end


	RECT rcSpl;
	m_wndSplitter.GetWindowRect(&rcSpl);
	ScreenToClient(&rcSpl);
	thePrefs.SetSplitterbarPositionShared(rcSpl.left);

	RemoveAnchor(m_wndSplitter);
	AddAnchor(m_wndSplitter, TOP_LEFT);

	//Xman [MoNKi: -Downloaded History-]
	RemoveAnchor(historylistctrl);
	//Xman end
	RemoveAnchor(sharedfilesctrl);
	RemoveAnchor(m_ctlSharedDirTree);
	RemoveAnchor(m_ctlFilter);
	RemoveAnchor(m_dlgDetails);

	//Xman [MoNKi: -Downloaded History-]
	AddAnchor(historylistctrl,TOP_LEFT,BOTTOM_RIGHT);
	//Xman end
	AddAnchor(sharedfilesctrl, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(m_ctlSharedDirTree, TOP_LEFT, BOTTOM_LEFT);
	AddAnchor(m_ctlFilter, TOP_LEFT);
	AddAnchor(m_dlgDetails, BOTTOM_LEFT, BOTTOM_RIGHT);

	CRect rcWnd;
	GetWindowRect(rcWnd);
	ScreenToClient(rcWnd);
	m_wndSplitter.SetRange(rcWnd.left + SPLITTER_RANGE_MIN, rcWnd.left + SPLITTER_RANGE_MAX);

	Invalidate();
	UpdateWindow();
}


void CSharedFilesWnd::Reload(bool bForceTreeReload)
{	
	sharedfilesctrl.SetDirectoryFilter(NULL, false);
	m_ctlSharedDirTree.Reload(bForceTreeReload); // force a reload of the tree to update the 'accessible' state of each directory
	sharedfilesctrl.SetDirectoryFilter(m_ctlSharedDirTree.GetSelectedFilter(), false);
	theApp.sharedfiles->Reload(thePrefs.queryOnHashing!=1);// X: [QOH] - [QueryOnHashing]

	ShowSelectedFilesDetails();
}
// NEO: AKF - [AllKnownFiles] -- Xanatos -->
/*
void CSharedFilesWnd::OnStnDblClickFilesIco()
{
	theApp.emuledlg->ShowPreferences(IDD_PPG_DIRECTORIES);
}

void CSharedFilesWnd::OnBnClickedReloadSharedFiles()
{
	CWaitCursor curWait;
#ifdef _DEBUG
	if (GetAsyncKeyState(VK_CONTROL) < 0) {
		theApp.sharedfiles->RebuildMetaData();
		sharedfilesctrl.Invalidate();
		sharedfilesctrl.UpdateWindow();
		return;
	}
#endif
	Reload(true);
}
*/
void CSharedFilesWnd::OnLvnItemActivateFiles(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	ShowSelectedFilesDetails();
}

BOOL CSharedFilesWnd::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		// Don't handle Ctrl+Tab in this window. It will be handled by main window.
		if (pMsg->wParam == VK_TAB && GetAsyncKeyState(VK_CONTROL) < 0)
			return FALSE;
		if (pMsg->wParam == VK_ESCAPE)
			return FALSE;
	}
	else if (pMsg->message == WM_KEYUP)
	{
	   ShowSelectedFilesDetails();
   }
	else if (!thePrefs.GetStraightWindowStyles() && pMsg->message == WM_MBUTTONUP)
	{
		if(m_ShowAllKnow)
			historylistctrl.ShowSelectedFileComments();
		else
			sharedfilesctrl.ShowSelectedFileComments();
		return TRUE;
	}

	return CResizableDialog::PreTranslateMessage(pMsg);
}

void CSharedFilesWnd::OnSysColorChange()
{
	CResizableDialog::OnSysColorChange();
	SetAllIcons();
	m_btnWndS->Invalidate(); // NEO: AKF - [AllKnownFiles] <-- Xanatos --
}

void CSharedFilesWnd::SetAllIcons()
{
//	if (icon_files)
//		VERIFY( DestroyIcon(icon_files) );
	//Xman [MoNKi: -Downloaded History-]
	//if(!historylistctrl.IsWindowVisible())
	//{
	//	icon_files = theApp.LoadIcon(_T("SharedFilesList"), 16, 16);
	//	((CStatic*)GetDlgItem(IDC_FILES_ICO))->SetIcon(icon_files);
	//}
	//else
	//{
	//	icon_files = theApp.LoadIcon(_T("DOWNLOAD"), 16, 16);
	//	((CStatic*)GetDlgItem(IDC_FILES_ICO))->SetIcon(icon_files);
	//}
	//Xman end
	SetWndIcons(); // NEO: AKF - [AllKnownFiles] <-- Xanatos --
}
// NEO: AKF - [AllKnownFiles] -- Xanatos -->
void CSharedFilesWnd::SetWndIcons()
{
	CImageList iml;
	iml.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 1, 1);
	iml.Add(CTempIconLoader(_T("SharedFilesList")));
	iml.Add(CTempIconLoader(_T("DOWNLOAD")));
	iml.Add(CTempIconLoader(_T("SEARCHEDIT")));
	CImageList* pImlOld = m_btnWndS->SetImageList(&iml);
	iml.Detach();
	if (pImlOld)
		pImlOld->DeleteImageList();
}
// NEO: AKF END <-- Xanatos --
void CSharedFilesWnd::LocalizeToolbars()
{
	// NEO: AKF - [AllKnownFiles] -- Xanatos -->
	ShowFilesCount(m_ShowAllKnow);
	//m_btnWndS->SetWindowText(GetResString(IDS_SF_FILES));
	m_btnWndS->SetBtnText(MP_VIEWS_SHARED, GetResString(IDS_SF_FILES));
	m_btnWndS->SetBtnText(MP_VIEWS_HISTORY, GetResString(IDS_DOWNHISTORY));
	UpdateFilterLabel();
	// NEO: AKF END <-- Xanatos --
}

void CSharedFilesWnd::Localize()// X: [RUL] - [Remove Useless Localize]
{
	m_ctlSharedDirTree.Localize();
	sharedfilesctrl.SetDirectoryFilter(NULL,true);

	//SetDlgItemText(IDC_TRAFFIC_TEXT,GetResString(IDS_SF_FILES)); //Xman [MoNKi: -Downloaded History-]
	SetDlgItemText(IDC_RELOADSHAREDFILES,GetResString(IDS_SF_RELOAD));
}

void CSharedFilesWnd::LocalizeAll()// X: [RUL] - [Remove Useless Localize]
{
	Localize();
	LocalizeToolbars();
	sharedfilesctrl.Localize();
	historylistctrl.Localize(); //Xman [MoNKi: -Downloaded History-]
	m_dlgDetails.LocalizeAll();
	if(m_ctlFilter.m_strLastEvaluatedContent.GetLength() == 0 && m_ctlFilter.GetFocus()!=(CWnd*)&m_ctlFilter)
		m_ctlFilter.ShowColumnText(true,true);
}

//>>> WiZaRd::FiX
void CSharedFilesWnd::UpdateMetaDataPage()
{
	m_dlgDetails.UpdateMetaDataPage();
}
//<<< WiZaRd::FiX

void CSharedFilesWnd::OnTvnSelChangedSharedDirsTree(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	if(!m_ShowAllKnow)
		sharedfilesctrl.SetDirectoryFilter(m_ctlSharedDirTree.GetSelectedFilter(), !m_ctlSharedDirTree.IsCreatingTree());
	*pResult = 0;
}

LRESULT CSharedFilesWnd::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message)
	{
		case WM_PAINT:
			if (m_wndSplitter)
			{
				CRect rcWnd;
				GetWindowRect(rcWnd);
				if (rcWnd.Width() > 0)
				{
					RECT rcSpl;
					m_ctlSharedDirTree.GetWindowRect(&rcSpl);
					ScreenToClient(&rcSpl);
					rcSpl.left = rcSpl.right + SPLITTER_MARGIN;
					rcSpl.right = rcSpl.left + SPLITTER_WIDTH;

					RECT rcFilter;
					m_ctlFilter.GetWindowRect(&rcFilter);
					ScreenToClient(&rcFilter);
					rcSpl.top = rcFilter.top;
					m_wndSplitter.MoveWindow(&rcSpl, TRUE);
				}
			}
			// NEO: AKF - [AllKnownFiles] -- Xanatos -->
			// Workaround to solve a glitch with WM_SETTINGCHANGE message
			if (m_btnWndS && m_btnWndS->m_hWnd){
				if(thePrefs.IsTransToolbarEnabled()){
					if(m_btnWndS->GetBtnWidth(IDC_FILES_ICO) != WNDS_BUTTON_WIDTH)
						m_btnWndS->SetBtnWidth(IDC_FILES_ICO, WNDS_BUTTON_WIDTH);
				}
				else{
					if(m_btnWndS->GetBtnWidth(IDC_FILES_ICO) != WNDS_BUTTON_WIDTH+WNDS_BUTTON3_WIDTH)
						m_btnWndS->SetBtnWidth(IDC_FILES_ICO, WNDS_BUTTON_WIDTH+WNDS_BUTTON3_WIDTH);
				}
			}
			// NEO: AKF END <-- Xanatos --
			break;

		case WM_NOTIFY:
			if (wParam == IDC_SPLITTER_SHAREDFILES)
			{ 
				SPC_NMHDR* pHdr = (SPC_NMHDR*)lParam;
				DoResize(pHdr->delta);
			}
			break;

		case WM_SIZE:
			if (m_wndSplitter)
			{
				CRect rcWnd;
				GetWindowRect(rcWnd);
				ScreenToClient(rcWnd);
				m_wndSplitter.SetRange(rcWnd.left + SPLITTER_RANGE_MIN, rcWnd.left + SPLITTER_RANGE_MAX);
			}
			break;
	}
	return CResizableDialog::DefWindowProc(message, wParam, lParam);
}

LRESULT CSharedFilesWnd::OnChangeFilter(WPARAM wParam, LPARAM lParam)
{
	CWaitCursor curWait; // this may take a while

	CFilterItem*fi;
	if(m_ShowAllKnow)
		fi=&historylistctrl;
	else
		fi=&sharedfilesctrl;
	bool bColumnDiff = (fi->m_nFilterColumn != (uint32)wParam);
	fi->m_nFilterColumn = wParam;
	
	CAtlArray<CString> astrFilter;
	CString strFullFilterExpr = (LPCTSTR)lParam;
	int iPos = 0;
	CString strFilter(strFullFilterExpr.Tokenize(_T(" "), iPos));
	while (!strFilter.IsEmpty()) {
		if (strFilter != _T('-'))
			astrFilter.Add(strFilter);
		strFilter = strFullFilterExpr.Tokenize(_T(" "), iPos);
	}
	
	CAtlArray<CString>& m_astrFilter=fi->m_astrFilter; // X: AKF - [AllKnownFiles] <-- Xanatos --
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
	fi->ReloadFileList(); // X: AKF - [AllKnownFiles] <-- Xanatos --
	return 0;
}

HBRUSH CSharedFilesWnd::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = theApp.emuledlg->GetCtlColor(pDC, pWnd, nCtlColor);
	if (hbr)
		return hbr;
	return __super::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CSharedFilesWnd::SetToolTipsDelay(DWORD dwDelay)
{
	sharedfilesctrl.SetToolTipsDelay(dwDelay);
}

void CSharedFilesWnd::ShowSelectedFilesDetails(bool bForce) //Xman [MoNKi: -Downloaded History-]
{
	CAtlList<CShareableFile*> selectedList;
	bool m_bChanged = false;
	if (m_bDetailsVisible)
	{
		CMuleListCtrl*listctrl = m_ShowAllKnow?(CMuleListCtrl*)&historylistctrl:(CMuleListCtrl*)&sharedfilesctrl; // X: AKF - [AllKnownFiles] <-- Xanatos --
		POSITION pos = listctrl->GetFirstSelectedItemPosition();
		int i = 0;
		while (pos != NULL)
		{
			INT_PTR index = listctrl->GetNextSelectedItem(pos);
			if (index >= 0)
			{
				CShareableFile* file = (CShareableFile*)listctrl->GetItemData(index);
				if (file != NULL)
				{
					selectedList.AddTail(file);
					m_bChanged |= m_dlgDetails.GetItems().GetSize() <= i || m_dlgDetails.GetItems()[i] != file;
					i++;
				}
			}
		}
	}
	m_bChanged |= m_dlgDetails.GetItems().GetSize() != selectedList.GetCount();

	if (m_bChanged || bForce)
		m_dlgDetails.SetFiles(selectedList);
}
void CSharedFilesWnd::ShowDetailsPanel(bool bShow)
{
	if (bShow == m_bDetailsVisible)
		return;
	m_bDetailsVisible = bShow;
	thePrefs.SetShowSharedFilesDetails(bShow);
	RemoveAnchor(sharedfilesctrl);
	RemoveAnchor(historylistctrl);
	RemoveAnchor(IDC_SF_HIDESHOWDETAILS);
	
	CRect rcFile, rcDetailDlg, rcButton;
	sharedfilesctrl.GetWindowRect(rcFile);
	m_dlgDetails.GetWindowRect(rcDetailDlg);
	GetDlgItem(IDC_SF_HIDESHOWDETAILS)->GetWindowRect(rcButton);
	ScreenToClient(rcButton);
	const int nOffset = 29;
	if (!bShow)
	{
		sharedfilesctrl.SetWindowPos(NULL, 0, 0, rcFile.Width(), rcFile.Height() + (rcDetailDlg.Height() - nOffset), SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
		GetDlgItem(IDC_SF_HIDESHOWDETAILS)->SetWindowPos(NULL, rcButton.left, rcButton.top + (rcDetailDlg.Height() - nOffset), 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
		m_dlgDetails.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_SF_HIDESHOWDETAILS)->SetWindowText(_T("5"));
		//Xman [MoNKi: -Downloaded History-]
		historylistctrl.SetWindowPos(NULL, 0, 0, rcFile.Width(), rcFile.Height() + (rcDetailDlg.Height() - nOffset), SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
		//Xman end
	}
	else
	{
		sharedfilesctrl.SetWindowPos(NULL, 0, 0, rcFile.Width(), rcFile.Height() - (rcDetailDlg.Height() - nOffset), SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
		GetDlgItem(IDC_SF_HIDESHOWDETAILS)->SetWindowPos(NULL, rcButton.left, rcButton.top - (rcDetailDlg.Height() - nOffset), 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
		m_dlgDetails.ShowWindow(SW_SHOW);
		GetDlgItem(IDC_SF_HIDESHOWDETAILS)->SetWindowText(_T("6"));
		//Xman [MoNKi: -Downloaded History-]
		historylistctrl.SetWindowPos(NULL, 0, 0, rcFile.Width(), rcFile.Height() - (rcDetailDlg.Height() - nOffset), SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
		//Xman end
	}
	if(m_ShowAllKnow)
		historylistctrl.SetFocus();
	else
		sharedfilesctrl.SetFocus();
	AddAnchor(sharedfilesctrl, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(historylistctrl,TOP_LEFT,BOTTOM_RIGHT);
	AddAnchor(IDC_SF_HIDESHOWDETAILS, BOTTOM_RIGHT);
	ShowSelectedFilesDetails();
}
/*
void CSharedFilesWnd::OnBnClickedSfHideshowdetails()
{
	ShowDetailsPanel(!m_bDetailsVisible);
}
*/
void CSharedFilesWnd::OnLvnItemchangedSflist(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	ShowSelectedFilesDetails();
	*pResult = 0;
}
/*
//Xman [MoNKi: -Downloaded History-]
void CSharedFilesWnd::OnNMClickHistorylist(NMHDR *pNMHDR, LRESULT *pResult){
	OnLvnItemActivateHistorylist(pNMHDR,pResult);
	*pResult = 0;
}

void CSharedFilesWnd::OnLvnItemActivateHistorylist(NMHDR* /*pNMHDR, LRESULT* /*pResult)
{
	ShowSelectedFilesDetails(true);
}

void CSharedFilesWnd::OnShowWindow( BOOL bShow,UINT /*nStatus )
{
	if(m_ShowAllKnow)
		historylistctrl.ShowFilesCount();
	else if(bShow)
		sharedfilesctrl.ShowFilesCount();
}*/
//Xman end

void CSharedFilesWnd::ShowFilesCount(bool history)
{
	if(m_ShowAllKnow == history){
		CString str;
		if(history){
			str = GetResString(IDS_DOWNHISTORY);
			if(!thePrefs.IsTransToolbarEnabled())
				str.AppendFormat(_T("(%s)"), GetFilterLabel());
			str.AppendFormat(_T(" (%i)"), historylistctrl.GetItemCount());
		}
		else{
			str = GetResString(IDS_SF_FILES);
			if(!thePrefs.IsTransToolbarEnabled())
				str.AppendFormat(_T("(%s)"), GetFilterLabel());
			if (theApp.sharedfiles->GetHashingCount() + nAICHHashing)
				str.AppendFormat(_T(" (%i, %s %i)"), theApp.sharedfiles->GetCount(), GetResString(IDS_HASHING), theApp.sharedfiles->GetHashingCount() + nAICHHashing);
			else
				str.AppendFormat(_T(" (%i)"), theApp.sharedfiles->GetCount());
		}
		m_btnWndS->SetWindowText(str);
	}

}

void CSharedFilesWnd::UpdateFilterLabel(){
	m_btnWndS->SetBtnText(MP_VIEWS_FILTER, GetFilterLabel());
}

// NEO: AKF - [AllKnownFiles] -- Xanatos -->
BOOL CSharedFilesWnd::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	if (wParam>=MP_CAT_SET0 && wParam<=MP_CAT_SET0+99){
		CFilterItem*fi;
		if(m_ShowAllKnow)
			fi=&historylistctrl;
		else
			fi=&sharedfilesctrl;
		fi->SetFilter(wParam - MP_CAT_SET0);
		return TRUE;
	}

	switch (wParam)
	{
		case IDC_RELOADSHAREDFILES:
		{
			CWaitCursor curWait;
#ifdef _DEBUG
			if (GetAsyncKeyState(VK_CONTROL) < 0) {
				theApp.sharedfiles->RebuildMetaData();
				sharedfilesctrl.Invalidate();
				sharedfilesctrl.UpdateWindow();
				return TRUE;
			}
#endif
			Reload(true);
			break;
		}
		case IDC_SF_HIDESHOWDETAILS:
			ShowDetailsPanel(!m_bDetailsVisible);
			break;
		default:
		{
			bool oldShowAllKnow=m_ShowAllKnow;
			switch (wParam)
			{ 
				case IDC_FILES_ICO:{
					m_ShowAllKnow = !m_ShowAllKnow;
					break;
				}
				case MP_VIEWS_SHARED: {
					m_ShowAllKnow = false;
					break;
				}
				case MP_VIEWS_HISTORY: {
					m_ShowAllKnow = true;
					break;
				}
				case MP_VIEWS_FILTER:{
					CTitleMenu CatMenu;
					CatMenu.CreatePopupMenu();
					CatMenu.EnableIcons();
					CreateFilterMenu(CatMenu);
					RECT rc;
					m_btnWndS->GetItemRect(3,&rc);
					ClientToScreen(&rc);
					CatMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rc.left+8, rc.bottom+4, this);
					VERIFY( CatMenu.DestroyMenu() );
				}
					//break;
				default:
					return  TRUE;
			}
			ChangeView(oldShowAllKnow!=m_ShowAllKnow);
			ShowFilesCount(m_ShowAllKnow);
			ShowSelectedFilesDetails();
			break;
		}
	}

	return TRUE;
}

void CSharedFilesWnd::SetWndIcon()
{
	TBBUTTONINFO tbbi = {0};
	tbbi.cbSize = sizeof tbbi;
	tbbi.dwMask = TBIF_IMAGE;
	tbbi.iImage = m_ShowAllKnow?1:0;
	m_btnWndS->SetButtonInfo(GetWindowLongPtr(*m_btnWndS, GWL_ID), &tbbi);
}

void CSharedFilesWnd::ChangeView(bool changFilter) // X: AKF - [AllKnownFiles] <-- Xanatos --
{
	SetWndIcon();

	//Xman [MoNKi: -Downloaded History-]
	if(m_ShowAllKnow)
	{
		historylistctrl.ShowWindow(true);
		sharedfilesctrl.ShowWindow(false);
		m_btnWndS->CheckButton(MP_VIEWS_HISTORY);
		GetDlgItem(IDC_RELOADSHAREDFILES)->ShowWindow(false);

		if(changFilter){
			m_ctlFilter.SaveTo(sharedfilesctrl);// X: [FI] - [FilterItem]
			m_ctlFilter.UpdateFrom(historylistctrl);// X: [FI] - [FilterItem]
			filter = historylistctrl.filter;// X: [FI] - [FilterItem]
		}
	}
	else
	{
		sharedfilesctrl.ShowWindow(true);
		historylistctrl.ShowWindow(false);
		m_btnWndS->CheckButton(MP_VIEWS_SHARED);
		GetDlgItem(IDC_RELOADSHAREDFILES)->ShowWindow(true);

		if(changFilter){
			m_ctlFilter.SaveTo(historylistctrl);// X: [FI] - [FilterItem]
			m_ctlFilter.UpdateFrom(sharedfilesctrl);// X: [FI] - [FilterItem]
			filter = sharedfilesctrl.filter;// X: [FI] - [FilterItem]
			sharedfilesctrl.SetDirectoryFilter(m_ctlSharedDirTree.GetSelectedFilter(), !m_ctlSharedDirTree.IsCreatingTree());
		}
	}
	UpdateFilterLabel();
	//Xman end
}

void CSharedFilesWnd::OnWndSBtnDropDown(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/) // X: AKF - [AllKnownFiles] <-- Xanatos --
{
	CTitleMenu menu;
	menu.CreatePopupMenu();
	//menu.AddMenuTitle(NULL); // NEO: NMX - [NeoMenuXP]
	menu.EnableIcons();

	menu.AppendMenu(MF_STRING | (!m_ShowAllKnow ? MF_GRAYED : 0), MP_VIEWS_SHARED, GetResString(IDS_SF_FILES), _T("SharedFilesList"));
	menu.AppendMenu(MF_STRING | (m_ShowAllKnow ? MF_GRAYED : 0), MP_VIEWS_HISTORY, GetResString(IDS_DOWNHISTORY), _T("DOWNLOAD"));
	menu.AppendMenu(MF_SEPARATOR);
	CTitleMenu CatMenu;
	CatMenu.CreateMenu();
	CatMenu.EnableIcons();
	CreateFilterMenu(CatMenu);
	menu.AppendMenu(MF_POPUP, (UINT_PTR)CatMenu.m_hMenu, GetResString(IDS_CHANGECATVIEW));

	RECT rc;
	m_btnWndS->GetWindowRect(&rc);
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rc.left, rc.bottom, this);
	VERIFY( CatMenu.DestroyMenu() );
	VERIFY( menu.DestroyMenu() );
}

void CSharedFilesWnd::ResetShareToolbar(bool bShowToolbar) // X: AKF - [AllKnownFiles] <-- Xanatos --
{
	if (m_btnWndS->m_hWnd)
		RemoveAnchor(*m_btnWndS);

	CRect rc;
	rc.top = 5;
	rc.left = WNDS_BUTTON_XOFF;
	rc.right = rc.left + WNDS_BUTTON_WIDTH + WNDS_BUTTON3_WIDTH;
	if(bShowToolbar)
		rc.right += NUMS_WINA_BUTTONS*DFLT_TOOLBAR_BTN_WIDTH;
	rc.bottom = rc.top + WNDS_BUTTON_HEIGHT;
	m_btnWndS->Init(!bShowToolbar);
	m_btnWndS->MoveWindow(&rc);
	SetWndIcons();

	if (bShowToolbar)
	{
		m_btnWndS->ModifyStyle(0, TBSTYLE_TOOLTIPS);
		m_btnWndS->SetExtendedStyle(m_btnWndS->GetExtendedStyle() | TBSTYLE_EX_MIXEDBUTTONS);

		TBBUTTON atb[2+NUMS_WINA_BUTTONS] = {0};
		atb[0].iBitmap = m_ShowAllKnow?1:0;
		atb[0].idCommand = IDC_FILES_ICO;
		atb[0].fsState = TBSTATE_ENABLED;
		atb[0].fsStyle = BTNS_BUTTON | BTNS_SHOWTEXT;
		atb[0].iString = -1;

		atb[1].iBitmap = 0;
		atb[1].idCommand = MP_VIEWS_SHARED;
		atb[1].fsState = TBSTATE_ENABLED;
		atb[1].fsStyle = BTNS_BUTTON | BTNS_CHECKGROUP | BTNS_AUTOSIZE;
		atb[1].iString = -1;

		atb[2].iBitmap = 1;
		atb[2].idCommand = MP_VIEWS_HISTORY;
		atb[2].fsState = TBSTATE_ENABLED;
		atb[2].fsStyle = BTNS_BUTTON | BTNS_CHECKGROUP | BTNS_AUTOSIZE;
		atb[2].iString = -1;

		atb[3].iBitmap = 2;
		atb[3].idCommand = MP_VIEWS_FILTER;
		atb[3].fsState = TBSTATE_ENABLED;
		atb[3].fsStyle = BTNS_BUTTON | BTNS_SHOWTEXT;
		atb[3].iString = -1;

		m_btnWndS->AddButtons(ARRSIZE(atb), atb);

		m_btnWndS->GetWindowRect(&rc);
		ScreenToClient(&rc);
		m_btnWndS->MoveWindow(rc.left, rc.top, rc.Width(), rc.Height());
		m_btnWndS->CheckButton(m_ShowAllKnow?MP_VIEWS_HISTORY:MP_VIEWS_SHARED);
	}
	else
	{
		m_btnWndS->ModifyStyle(TBSTYLE_TOOLTIPS, 0);
		m_btnWndS->SetExtendedStyle(m_btnWndS->GetExtendedStyle() & ~TBSTYLE_EX_MIXEDBUTTONS);
		m_btnWndS->RecalcLayout(true);
		SetWndIcon();
	}

	AddAnchor(*m_btnWndS, TOP_LEFT);
	LocalizeToolbars();
}
// NEO: AKF END <-- Xanatos --

/////////////////////////////////////////////////////////////////////////////////////////////
// CSharedFileDetailsModelessSheet
IMPLEMENT_DYNAMIC(CSharedFileDetailsModelessSheet, CListViewPropertySheet)

BEGIN_MESSAGE_MAP(CSharedFileDetailsModelessSheet, CListViewPropertySheet)
	ON_MESSAGE(UM_DATA_CHANGED, OnDataChanged)
	ON_WM_CREATE()
END_MESSAGE_MAP()

int CSharedFileDetailsModelessSheet::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	// skip CResizableSheet::OnCreate because we don't the styles and stuff which are set there
	CreateSizeGrip(FALSE); // create grip but dont show it
	return CPropertySheet::OnCreate(lpCreateStruct);
}

bool NeedArchiveInfoPage(const CSimpleArray<void*>* paItems);
void UpdateFileDetailsPages(CListViewPropertySheet *pSheet,
							CResizablePage *pArchiveInfo, CResizablePage *pMediaInfo);

CSharedFileDetailsModelessSheet::CSharedFileDetailsModelessSheet()
{
	m_psh.dwFlags &= ~PSH_HASHELP;
	m_psh.dwFlags |= PSH_MODELESS;
	m_wndStatistics = new CFileDetailDlgStatistics();
	m_wndFileLink = new CED2kLinkDlg();
	m_wndArchiveInfo = new CArchivePreviewDlg();
	m_wndMediaInfo = new CFileInfoDialog();
	m_wndMetaData = NULL;

	m_wndStatistics->m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndStatistics->m_psp.dwFlags |= PSP_USEICONID;
	m_wndStatistics->m_psp.pszIcon = _T("StatsDetail");
	m_wndStatistics->SetFiles(&m_aItems);
	AddPage(m_wndStatistics);

	m_wndArchiveInfo->m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndArchiveInfo->m_psp.dwFlags |= PSP_USEICONID;
	m_wndArchiveInfo->m_psp.pszIcon = _T("ARCHIVE_PREVIEW");
	m_wndArchiveInfo->SetReducedDialog();
	m_wndArchiveInfo->SetFiles(&m_aItems);
	m_wndMediaInfo->m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndMediaInfo->m_psp.dwFlags |= PSP_USEICONID;
	m_wndMediaInfo->m_psp.pszIcon = _T("MEDIAINFO");
	m_wndMediaInfo->SetReducedDialog();
	m_wndMediaInfo->SetFiles(&m_aItems);
	if (NeedArchiveInfoPage(&m_aItems))
		AddPage(m_wndArchiveInfo);
	else
		AddPage(m_wndMediaInfo);

	m_wndFileLink->m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndFileLink->m_psp.dwFlags |= PSP_USEICONID;
	m_wndFileLink->m_psp.pszIcon = _T("ED2KLINK");
	m_wndFileLink->SetReducedDialog();
	m_wndFileLink->SetFiles(&m_aItems);
	AddPage(m_wndFileLink);

//>>> WiZaRd::FiX
	UpdateMetaDataPage();
// 	if (thePrefs.IsExtControlsEnabled())
// 	{
// 		m_wndMetaData = new CMetaDataDlg();
// 		m_wndMetaData->m_psp.dwFlags &= ~PSP_HASHELP;
// 		m_wndMetaData->m_psp.dwFlags |= PSP_USEICONID;
// 		m_wndMetaData->m_psp.pszIcon = _T("METADATA");
// 		m_wndMetaData->SetFiles(&m_aItems);
// 		AddPage(m_wndMetaData);
// 	}
//<<< WiZaRd::FiX

	/*LPCTSTR pPshStartPage = m_pPshStartPage;
	if (m_uPshInvokePage != 0)
		pPshStartPage = MAKEINTRESOURCE(m_uPshInvokePage);
	for (int i = 0; i < m_pages.GetSize(); i++)
	{
		CPropertyPage* pPage = GetPage(i);
		if (pPage->m_psp.pszTemplate == pPshStartPage)
		{
			m_psh.nStartPage = i;
			break;
		}
	}*/
}

//>>> WiZaRd::FiX
void CSharedFileDetailsModelessSheet::UpdateMetaDataPage()
{
	if (thePrefs.IsExtControlsEnabled())
	{
		if(m_wndMetaData)
			return;

		m_wndMetaData = new CMetaDataDlg();
		m_wndMetaData->m_psp.dwFlags &= ~PSP_HASHELP;
		m_wndMetaData->m_psp.dwFlags |= PSP_USEICONID;
		m_wndMetaData->m_psp.pszIcon = _T("METADATA");
		m_wndMetaData->SetFiles(&m_aItems);
		AddPage(m_wndMetaData);
	}
	else
	{
		if(m_wndMetaData == NULL)
			return;

		RemovePage(m_wndMetaData);
		delete m_wndMetaData;
		m_wndMetaData = NULL;
	}
}

void  CSharedFileDetailsModelessSheet::LocalizeAll()
{
	// X: [BF] - [Bug Fix]
	static const UINT titleids[] = {IDS_SF_STATISTICS, IDS_SW_LINK, IDS_CONTENT_INFO, IDS_META_DATA};
	CTabCtrl*tab = GetTabControl();
	TCITEM tabitem;
	tabitem.mask = TCIF_TEXT;
	for (int index = 0; index < tab->GetItemCount(); ++index)
	{
		CString strCaption(GetResString(titleids[index]));
		tabitem.pszText = const_cast<LPTSTR>((LPCTSTR)strCaption);
		tab->SetItem(index,&tabitem);
	}
	

	if(m_wndStatistics && m_wndStatistics->GetSafeHwnd())
		m_wndStatistics->Localize();
	if(m_wndFileLink && m_wndFileLink->GetSafeHwnd())
		m_wndFileLink->Localize();
	if(m_wndArchiveInfo && m_wndArchiveInfo->GetSafeHwnd())
		m_wndArchiveInfo->Localize();
	if(m_wndMediaInfo && m_wndMediaInfo->GetSafeHwnd())
		m_wndMediaInfo->Localize();
	if(m_wndMetaData && m_wndMetaData->GetSafeHwnd())
		m_wndMetaData->Localize();
}
//<<< WiZaRd::FiX

CSharedFileDetailsModelessSheet::~CSharedFileDetailsModelessSheet()
{
	delete m_wndStatistics;
	delete m_wndFileLink;
	delete m_wndArchiveInfo;
	delete m_wndMediaInfo;
	delete m_wndMetaData;
}

BOOL CSharedFileDetailsModelessSheet::OnInitDialog()
{
	EnableStackedTabs(FALSE);
	BOOL bResult = CListViewPropertySheet::OnInitDialog();
	HighColorTab::UpdateImageList(*this);
	InitWindowStyles(this);
	return bResult;
}

void  CSharedFileDetailsModelessSheet::SetFiles(CAtlList<CShareableFile*>& aFiles)
{
	m_aItems.RemoveAll();
	POSITION pos = aFiles.GetHeadPosition();
	while (pos)
		m_aItems.Add(aFiles.GetNext(pos));
	ChangedData();
}

LRESULT CSharedFileDetailsModelessSheet::OnDataChanged(WPARAM, LPARAM)
{
	UpdateFileDetailsPages(this, m_wndArchiveInfo, m_wndMediaInfo);
	return 1;
}