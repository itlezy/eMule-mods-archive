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
#include "HelpIDs.h"
#include "MenuCmds.h"
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
#include "SearchFile.h"
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	SPLITTER_RANGE_MIN		100
#define	SPLITTER_RANGE_MAX		350

#define	SPLITTER_MARGIN			0
#define	SPLITTER_WIDTH			4

// NEO: AKF - [AllKnownFiles] -- Xanatos -->
#define	DFLT_TOOLBAR_BTN_WIDTH	27

#define	WNDS_BUTTON_XOFF	8
#define	WNDS_BUTTON_WIDTH	220
#define	WNDS_BUTTON_HEIGHT	22	// don't set the height do something different than 22 unless you know exactly what you are doing!
#define	NUMS_WINA_BUTTONS	2
// NEO: AKF END <-- Xanatos --

// CSharedFilesWnd dialog

IMPLEMENT_DYNAMIC(CSharedFilesWnd, CDialog)

BEGIN_MESSAGE_MAP(CSharedFilesWnd, CResizableDialog)
	//ON_BN_CLICKED(IDC_RELOADSHAREDFILES, OnBnClickedReloadSharedFiles)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_SFLIST, OnLvnItemActivateSharedFiles)
	ON_NOTIFY(NM_CLICK, IDC_SFLIST, OnNMClickSharedFiles)
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CTLCOLOR()
	//ON_STN_DBLCLK(IDC_FILES_ICO, OnStnDblClickFilesIco)
	ON_NOTIFY(TBN_DROPDOWN, IDC_SHARE_ICO, OnWndSBtnDropDown) // NEO: AKF - [AllKnownFiles] <-- Xanatos --
	ON_NOTIFY(TVN_SELCHANGED, IDC_SHAREDDIRSTREE, OnTvnSelChangedSharedDirsTree)
	ON_WM_SIZE()
	ON_MESSAGE(UM_DELAYED_EVALUATE, OnChangeFilter)
	ON_WM_HELPINFO()
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	ON_NOTIFY_EX_RANGE(UDM_TOOLTIP_DISPLAY, 0, 0xFFFF, OnToolTipNotify)
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
END_MESSAGE_MAP()

CSharedFilesWnd::CSharedFilesWnd(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CSharedFilesWnd::IDD, pParent)
{
	//icon_files = NULL;
	m_btnWndS = new CDropDownButton; // NEO: AKF - [AllKnownFiles] <-- Xanatos --
	m_nFilterColumn = 0;
}

CSharedFilesWnd::~CSharedFilesWnd()
{
	delete m_btnWndS;  // NEO: AKF - [AllKnownFiles] <-- Xanatos --
	m_ctlSharedListHeader.Detach();
	//if (icon_files)
	//	VERIFY( DestroyIcon(icon_files) );
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
	DDX_Control(pDX, IDC_SHARE_ICO, *m_btnWndS); // NEO: AKF - [AllKnownFiles] <-- Xanatos --
	DDX_Control(pDX, IDC_SHAREDFILES_FILTER, m_ctlFilter);
}

BOOL CSharedFilesWnd::OnInitDialog()
{
	CResizableDialog::OnInitDialog();
	InitWindowStyles(this);
	SetAllIcons();

	// NEO: AKF - [AllKnownFiles] -- Xanatos -->
	ResetShareToolbar(thePrefs.IsTransToolbarEnabled()); 
	m_btnWndS->CheckButton(MP_VIEWS_SHARED); 
	// NEO: AKF END <-- Xanatos --

	sharedfilesctrl.Init();
	m_ctlSharedDirTree.Initalize(&sharedfilesctrl);
	if (thePrefs.GetUseSystemFontForMainControls())
		m_ctlSharedDirTree.SendMessage(WM_SETFONT, NULL, FALSE);

	m_ctlSharedListHeader.Attach(sharedfilesctrl.GetHeaderCtrl()->Detach());
	CArray<int, int> aIgnore; // ignored no-text columns for filter edit
	aIgnore.Add(8); // shared parts
	aIgnore.Add(11); // shared ed2k/kad
	m_ctlFilter.OnInit(&m_ctlSharedListHeader, &aIgnore);

	pop_bar.SetGradientColors(RGB(255,255,240),RGB(255,255,0));
	pop_bar.SetTextColor(RGB(20,70,255));
	pop_baraccept.SetGradientColors(RGB(255,255,240),RGB(255,255,0));
	pop_baraccept.SetTextColor(RGB(20,70,255));
	pop_bartrans.SetGradientColors(RGB(255,255,240),RGB(255,255,0));
	pop_bartrans.SetTextColor(RGB(20,70,255));

	CRect rcSpl;
	m_ctlSharedDirTree.GetWindowRect(rcSpl);
	ScreenToClient(rcSpl);
	rcSpl.left = rcSpl.right + SPLITTER_MARGIN;
	rcSpl.right = rcSpl.left + SPLITTER_WIDTH;
	m_wndSplitter.Create(WS_CHILD | WS_VISIBLE, rcSpl, this, IDC_SPLITTER_SHAREDFILES);

	AddAnchor(m_wndSplitter, TOP_LEFT);
	AddAnchor(sharedfilesctrl, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(m_ctrlStatisticsFrm, BOTTOM_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_CURSESSION_LBL, BOTTOM_LEFT);
	AddAnchor(IDC_FSTATIC4, BOTTOM_LEFT);
	AddAnchor(IDC_SREQUESTED, BOTTOM_LEFT);
	AddAnchor(IDC_FSTATIC5, BOTTOM_LEFT);
	AddAnchor(IDC_SACCEPTED, BOTTOM_LEFT);
	AddAnchor(IDC_FSTATIC6, BOTTOM_LEFT);
	AddAnchor(IDC_STRANSFERRED, BOTTOM_LEFT);
	AddAnchor(m_ctlSharedDirTree, TOP_LEFT, BOTTOM_LEFT);
	AddAnchor(pop_bar, BOTTOM_LEFT, BOTTOM_RIGHT);
	AddAnchor(pop_baraccept, BOTTOM_LEFT, BOTTOM_RIGHT);
	AddAnchor(pop_bartrans, BOTTOM_LEFT, BOTTOM_RIGHT);
	AddAnchor(m_ctlFilter, TOP_LEFT);
	//AddAnchor(IDC_FILES_ICO, TOP_LEFT); // NEO: AKF - [AllKnownFiles] <-- Xanatos --
	AddAnchor(IDC_RELOADSHAREDFILES, TOP_RIGHT);
	AddAnchor(IDC_TOTAL_LBL, BOTTOM_RIGHT);
	AddAnchor(IDC_SREQUESTED2,BOTTOM_RIGHT);
	AddAnchor(IDC_FSTATIC7,BOTTOM_RIGHT);
	AddAnchor(IDC_FSTATIC8,BOTTOM_RIGHT);
	AddAnchor(IDC_FSTATIC9,BOTTOM_RIGHT);
	AddAnchor(IDC_STRANSFERRED2,BOTTOM_RIGHT);
	AddAnchor(IDC_SACCEPTED2,BOTTOM_RIGHT);
	//AddAnchor(IDC_TRAFFIC_TEXT, TOP_LEFT); // NEO: AKF - [AllKnownFiles] <-- Xanatos --

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
		m_wndSplitter.MoveWindow(rcSpl);
		DoResize(iPosStatNew - iPosStatInit);
	}

	Localize();

	GetDlgItem(IDC_CURSESSION_LBL)->SetFont(&theApp.m_fontDefaultBold);
	GetDlgItem(IDC_TOTAL_LBL)->SetFont(&theApp.m_fontDefaultBold);

#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	m_ttip.Create(this);
	m_ttip.AddTool(&sharedfilesctrl, _T(""));
	SetTTDelay();
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

	return TRUE;
}

void CSharedFilesWnd::DoResize(int iDelta)
{
	CSplitterControl::ChangeWidth(&m_ctlSharedDirTree, iDelta);
	CSplitterControl::ChangeWidth(&m_ctlFilter, iDelta);
	CSplitterControl::ChangePos(GetDlgItem(IDC_CURSESSION_LBL), -iDelta, 0);
	CSplitterControl::ChangePos(GetDlgItem(IDC_FSTATIC4), -iDelta, 0);
	CSplitterControl::ChangePos(GetDlgItem(IDC_SREQUESTED), -iDelta, 0);
	CSplitterControl::ChangePos(GetDlgItem(IDC_FSTATIC5), -iDelta, 0);
	CSplitterControl::ChangePos(GetDlgItem(IDC_SACCEPTED), -iDelta, 0);
	CSplitterControl::ChangePos(GetDlgItem(IDC_FSTATIC6), -iDelta, 0);
	CSplitterControl::ChangePos(GetDlgItem(IDC_STRANSFERRED), -iDelta, 0);
	CSplitterControl::ChangePos(&m_ctrlStatisticsFrm, -iDelta, 0);
	CSplitterControl::ChangePos(&sharedfilesctrl, -iDelta, 0);
	CSplitterControl::ChangeWidth(&m_ctrlStatisticsFrm, -iDelta);
	CSplitterControl::ChangeWidth(&sharedfilesctrl, -iDelta);
	CSplitterControl::ChangePos(&pop_bar, -iDelta, 0);
	CSplitterControl::ChangePos(&pop_baraccept, -iDelta, 0);
	CSplitterControl::ChangePos(&pop_bartrans, -iDelta, 0);
	CSplitterControl::ChangeWidth(&pop_bar, -iDelta);
	CSplitterControl::ChangeWidth(&pop_baraccept, -iDelta);
	CSplitterControl::ChangeWidth(&pop_bartrans, -iDelta);

	CRect rcSpl;
	m_wndSplitter.GetWindowRect(rcSpl);
	ScreenToClient(rcSpl);
	thePrefs.SetSplitterbarPositionShared(rcSpl.left);

	RemoveAnchor(m_wndSplitter);
	AddAnchor(m_wndSplitter, TOP_LEFT);
	RemoveAnchor(sharedfilesctrl);
	RemoveAnchor(m_ctrlStatisticsFrm);
	RemoveAnchor(IDC_CURSESSION_LBL);
	RemoveAnchor(IDC_FSTATIC4);
	RemoveAnchor(IDC_SREQUESTED);
	RemoveAnchor(pop_bar);
	RemoveAnchor(IDC_FSTATIC5);
	RemoveAnchor(IDC_SACCEPTED);
	RemoveAnchor(pop_baraccept);
	RemoveAnchor(IDC_FSTATIC6);
	RemoveAnchor(IDC_STRANSFERRED);
	RemoveAnchor(pop_bartrans);
	RemoveAnchor(m_ctlSharedDirTree);
	RemoveAnchor(m_ctlFilter);

	AddAnchor(sharedfilesctrl, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(m_ctrlStatisticsFrm, BOTTOM_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_CURSESSION_LBL, BOTTOM_LEFT);
	AddAnchor(IDC_FSTATIC4, BOTTOM_LEFT);
	AddAnchor(IDC_SREQUESTED, BOTTOM_LEFT);
	AddAnchor(IDC_FSTATIC5, BOTTOM_LEFT);
	AddAnchor(IDC_SACCEPTED, BOTTOM_LEFT);
	AddAnchor(IDC_FSTATIC6, BOTTOM_LEFT);
	AddAnchor(IDC_STRANSFERRED, BOTTOM_LEFT);
	AddAnchor(m_ctlSharedDirTree, TOP_LEFT, BOTTOM_LEFT);
	AddAnchor(pop_bar, BOTTOM_LEFT, BOTTOM_RIGHT);
	AddAnchor(pop_baraccept, BOTTOM_LEFT, BOTTOM_RIGHT);
	AddAnchor(pop_bartrans, BOTTOM_LEFT, BOTTOM_RIGHT);
	AddAnchor(m_ctlFilter, TOP_LEFT);

	CRect rcWnd;
	GetWindowRect(rcWnd);
	ScreenToClient(rcWnd);
	m_wndSplitter.SetRange(rcWnd.left + SPLITTER_RANGE_MIN, rcWnd.left + SPLITTER_RANGE_MAX);

	Invalidate();
	UpdateWindow();
}


//void CSharedFilesWnd::Reload()
void CSharedFilesWnd::Reload(bool bTreeOnly) // NEO: NSC - [NeoSharedCategories] <-- Xanatos --
{	
	sharedfilesctrl.SetDirectoryFilter(NULL, false);
	m_ctlSharedDirTree.Reload();
	sharedfilesctrl.SetDirectoryFilter(m_ctlSharedDirTree.GetSelectedFilter(), false);
	if(bTreeOnly == false) // NEO: NSC - [NeoSharedCategories] <-- Xanatos --
		theApp.sharedfiles->Reload();

	ShowSelectedFilesSummary();
}

// NEO: AKF - [AllKnownFiles] -- Xanatos --
//void CSharedFilesWnd::OnStnDblclickFilesIco()
//{
//	theApp.emuledlg->ShowPreferences(IDD_PPG_DIRECTORIES);
//}

//void CSharedFilesWnd::OnBnClickedReloadsharedfiles()
//{
//	CWaitCursor curWait;
//	Reload();
//}

void CSharedFilesWnd::OnLvnItemActivateSharedFiles(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	ShowSelectedFilesSummary();
}

void CSharedFilesWnd::ShowSelectedFilesSummary()
{
	const CKnownFile* pTheFile = NULL;
	int iFiles = 0;
	uint64 uTransferred = 0;
	UINT uRequests = 0;
	UINT uAccepted = 0;
	uint64 uAllTimeTransferred = 0;
	UINT uAllTimeRequests = 0;
	UINT uAllTimeAccepted = 0;
	POSITION pos = sharedfilesctrl.GetFirstSelectedItemPosition();
	while (pos)
	{
		int iItem = sharedfilesctrl.GetNextSelectedItem(pos);
		const CKnownFile* pFile = ((CSharedItem*)sharedfilesctrl.GetItemData(iItem))->KnownFile; // NEO: SP - [SharedParts] <-- Xanatos --
		//const CKnownFile* pFile = (CKnownFile*)sharedfilesctrl.GetItemData(iItem);
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

void CSharedFilesWnd::OnNMClickSharedFiles(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnLvnItemActivateSharedFiles(pNMHDR, pResult);
	*pResult = 0;
}

BOOL CSharedFilesWnd::PreTranslateMessage(MSG* pMsg)
{
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	m_ttip.RelayEvent(pMsg); 
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

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
		if (pMsg->hwnd == sharedfilesctrl.m_hWnd)
			OnLvnItemActivateSharedFiles(0, 0);
	}
	else if (!thePrefs.GetStraightWindowStyles() && pMsg->message == WM_MBUTTONUP)
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
		//sharedfilesctrl.ShowComments((CKnownFile*)sharedfilesctrl.GetItemData(it));
		sharedfilesctrl.ShowComments(((CSharedItem*)sharedfilesctrl.GetItemData(it))->KnownFile); // NEO: SP - [SharedParts] <-- Xanatos --
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
	m_btnWndS->Invalidate(); // NEO: AKF - [AllKnownFiles] <-- Xanatos --
}

void CSharedFilesWnd::SetAllIcons()
{
	m_ctrlStatisticsFrm.SetIcon(_T("StatsDetail"));

	//if (icon_files)
	//	VERIFY( DestroyIcon(icon_files) );
	//icon_files = theApp.LoadIcon(_T("SharedFilesList"), 16, 16);
	//((CStatic*)GetDlgItem(IDC_FILES_ICO))->SetIcon(icon_files);
	SetWndSIcons(); // NEO: AKF - [AllKnownFiles] <-- Xanatos --
}

// NEO: AKF - [AllKnownFiles] -- Xanatos -->
void CSharedFilesWnd::SetWndSIcons()
{
	CImageList iml;
	iml.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 1, 1);
	iml.Add(CTempIconLoader(_T("Sharedfiles")));
	iml.Add(CTempIconLoader(_T("SHAREDFILESLIST")));
	CImageList* pImlOld = m_btnWndS->SetImageList(&iml);
	iml.Detach();
	if (pImlOld)
		pImlOld->DeleteImageList();
}
// NEO: AKF END <-- Xanatos --

void CSharedFilesWnd::Localize()
{
	sharedfilesctrl.Localize();
	m_ctlSharedDirTree.Localize();
	sharedfilesctrl.SetDirectoryFilter(NULL,true);

	// NEO: AKF - [AllKnownFiles] -- Xanatos -->
	m_btnWndS->SetWindowText(GetResString(IDS_SF_FILES));
	m_btnWndS->SetBtnText(MP_VIEWS_SHARED, GetResString(IDS_X_SHARED));
	m_btnWndS->SetBtnText(MP_VIEWS_KNOWN, GetResString(IDS_X_KNOWN));
	// NEO: AKF END <-- Xanatos --

	//GetDlgItem(IDC_TRAFFIC_TEXT)->SetWindowText(GetResString(IDS_SF_FILES));
	sharedfilesctrl.ShowFilesCount(); // NEO: AKF - [AllKnownFiles] <-- Xanatos --
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
}

void CSharedFilesWnd::OnTvnSelChangedSharedDirsTree(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
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
					CRect rcSpl;
					m_ctlSharedDirTree.GetWindowRect(rcSpl);
					ScreenToClient(rcSpl);
					rcSpl.left = rcSpl.right + SPLITTER_MARGIN;
					rcSpl.right = rcSpl.left + SPLITTER_WIDTH;

					CRect rcFilter;
					m_ctlFilter.GetWindowRect(rcFilter);
					ScreenToClient(rcFilter);
					rcSpl.top = rcFilter.top;
					m_wndSplitter.MoveWindow(rcSpl, TRUE);
				}
			}

			// NEO: AKF - [AllKnownFiles] -- Xanatos -->
			// Workaround to solve a glitch with WM_SETTINGCHANGE message
			if (m_btnWndS->m_hWnd && m_btnWndS->GetBtnWidth(IDC_SHARE_ICO) != WNDS_BUTTON_WIDTH)
				m_btnWndS->SetBtnWidth(IDC_SHARE_ICO, WNDS_BUTTON_WIDTH);
			// NEO: AKF END <-- Xanatos --

			break;

		case WM_NOTIFY:
			if (wParam == IDC_SPLITTER_SHAREDFILES)
			{ 
				SPC_NMHDR* pHdr = (SPC_NMHDR*)lParam;
				DoResize(pHdr->delta);
			}
			break;

		case WM_WINDOWPOSCHANGED: {
			CRect rcWnd;
			GetWindowRect(rcWnd);
			if (m_wndSplitter && rcWnd.Width() > 0)
				Invalidate();
			break;
		}
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

void CSharedFilesWnd::OnSize(UINT nType, int cx, int cy){
	CResizableDialog::OnSize(nType, cx, cy);
}

LRESULT CSharedFilesWnd::OnChangeFilter(WPARAM wParam, LPARAM lParam)
{
	CWaitCursor curWait; // this may take a while

	bool bColumnDiff = (m_nFilterColumn != (uint32)wParam);
	m_nFilterColumn = (uint32)wParam;

	CStringArray astrFilter;
	CString strFullFilterExpr = (LPCTSTR)lParam;
	int iPos = 0;
	CString strFilter(strFullFilterExpr.Tokenize(_T(" "), iPos));
	while (!strFilter.IsEmpty()) {
		if (strFilter != _T("-"))
			astrFilter.Add(strFilter);
		strFilter = strFullFilterExpr.Tokenize(_T(" "), iPos);
	}

	bool bFilterDiff = (astrFilter.GetSize() != m_astrFilter.GetSize());
	if (!bFilterDiff) {
		for (int i = 0; i < astrFilter.GetSize(); i++) {
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

	sharedfilesctrl.ReloadFileList();
	return 0;
}

BOOL CSharedFilesWnd::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	theApp.ShowHelp(eMule_FAQ_GUI_SharedFiles);
	return TRUE;
}

HBRUSH CSharedFilesWnd::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = theApp.emuledlg->GetCtlColor(pDC, pWnd, nCtlColor);
	if (hbr)
		return hbr;
	return __super::OnCtlColor(pDC, pWnd, nCtlColor);
}

// NEO: AKF - [AllKnownFiles] -- Xanatos -->
BOOL CSharedFilesWnd::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	switch (wParam)
	{ 
		case IDC_RELOADSHAREDFILES:{
			CWaitCursor curWait;
			Reload();
			break;
		}

		case IDC_SHARE_ICO:{
			sharedfilesctrl.ChangeView();
			ShowSelectedFilesSummary();
			break;
		}
		case MP_VIEWS_SHARED: {
			sharedfilesctrl.ChangeView(1);
			ShowSelectedFilesSummary();
			break;
		}
		case MP_VIEWS_KNOWN: {
			sharedfilesctrl.ChangeView(2);
			ShowSelectedFilesSummary();
			break;
		}
	}

	return TRUE;
}

void CSharedFilesWnd::ChangeSIcon(int iIcon)
{
	TBBUTTONINFO tbbi = {0};
	tbbi.cbSize = sizeof tbbi;
	tbbi.dwMask = TBIF_IMAGE;
	tbbi.iImage = iIcon;
	m_btnWndS->SetButtonInfo(GetWindowLong(*m_btnWndS, GWL_ID), &tbbi);
}

void CSharedFilesWnd::OnWndSBtnDropDown(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	CTitleMenu menu;
	menu.CreatePopupMenu();
	menu.AddMenuTitle(NULL); // NEO: NMX - [NeoMenuXP]
	menu.EnableIcons();

	menu.AppendMenu(MF_STRING | (!sharedfilesctrl.IsKnownList() ? MF_GRAYED : 0), MP_VIEWS_SHARED, GetResString(IDS_X_SHARED), _T("SHAREDFILESLIST"));
	menu.AppendMenu(MF_STRING | (sharedfilesctrl.IsKnownList() ? MF_GRAYED : 0), MP_VIEWS_KNOWN, GetResString(IDS_X_KNOWN), _T("Sharedfiles"));

	CRect rc;
	m_btnWndS->GetWindowRect(&rc);
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rc.left, rc.bottom, this);
	VERIFY( menu.DestroyMenu() );
}

void CSharedFilesWnd::ResetShareToolbar(bool bShowToolbar)
{
	if (m_btnWndS->m_hWnd)
		RemoveAnchor(*m_btnWndS);

	CRect rc;
	rc.top = 5;
	rc.left = WNDS_BUTTON_XOFF;
	rc.right = rc.left + WNDS_BUTTON_WIDTH + (bShowToolbar ? NUMS_WINA_BUTTONS*DFLT_TOOLBAR_BTN_WIDTH : 0);
	rc.bottom = rc.top + WNDS_BUTTON_HEIGHT;
	m_btnWndS->Init(!bShowToolbar);
	m_btnWndS->MoveWindow(&rc);
	SetWndSIcons();

	if (bShowToolbar)
	{
		m_btnWndS->ModifyStyle(0, TBSTYLE_TOOLTIPS);
		m_btnWndS->SetExtendedStyle(m_btnWndS->GetExtendedStyle() | TBSTYLE_EX_MIXEDBUTTONS);

		TBBUTTON atb[1+NUMS_WINA_BUTTONS] = {0};
		atb[0].iBitmap = 1;
		atb[0].idCommand = IDC_SHARE_ICO;
		atb[0].fsState = TBSTATE_ENABLED;
		atb[0].fsStyle = BTNS_BUTTON | BTNS_SHOWTEXT;
		atb[0].iString = -1;

		atb[1].iBitmap = 1;
		atb[1].idCommand = MP_VIEWS_SHARED;
		atb[1].fsState = TBSTATE_ENABLED;
		atb[1].fsStyle = BTNS_BUTTON | BTNS_CHECKGROUP | BTNS_AUTOSIZE;
		atb[1].iString = -1;

		atb[2].iBitmap = 0;
		atb[2].idCommand = MP_VIEWS_KNOWN;
		atb[2].fsState = TBSTATE_ENABLED;
		atb[2].fsStyle = BTNS_BUTTON | BTNS_CHECKGROUP | BTNS_AUTOSIZE;
		atb[2].iString = -1;

		m_btnWndS->AddButtons(ARRSIZE(atb), atb);

		//CSize size;
		//m_btnWndS->GetMaxSize(&size);
		m_btnWndS->GetWindowRect(&rc);
		ScreenToClient(&rc);
		m_btnWndS->MoveWindow(rc.left, rc.top, rc.Width(), rc.Height());
	}
	else
	{
		m_btnWndS->ModifyStyle(TBSTYLE_TOOLTIPS, 0);
		m_btnWndS->SetExtendedStyle(m_btnWndS->GetExtendedStyle() & ~TBSTYLE_EX_MIXEDBUTTONS);
		m_btnWndS->RecalcLayout(true);
	}

	AddAnchor(*m_btnWndS, TOP_LEFT);

	sharedfilesctrl.ShowFilesCount();
}
// NEO: AKF END <-- Xanatos --

#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
BOOL CSharedFilesWnd::OnToolTipNotify(UINT /*id*/, NMHDR *pNMH, LRESULT* /*pResult*/)
{
	NM_PPTOOLTIP_DISPLAY * pNotify = (NM_PPTOOLTIP_DISPLAY*)pNMH;
	int control_id = CWnd::FromHandle(pNotify->ti->hWnd)->GetDlgCtrlID();

	switch(control_id)
	{
		case IDC_SFLIST:
		{
			if (sharedfilesctrl.GetItemCount() < 1)
				return FALSE;

			int sel = sharedfilesctrl.GetItemUnderMouse();
			if (sel < 0)
				return FALSE;

			CSharedItem* item = (CSharedItem*)sharedfilesctrl.GetItemData(sel); // NEO: SP - [SharedParts]
			if (!item)
				return FALSE;
			CKnownFile* file = item->KnownFile; // NEO: SP - [SharedParts]

			SHFILEINFO shfi;
			ZeroMemory(&shfi, sizeof(shfi));
			SHGetFileInfo(file->GetFileName(), FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi), SHGFI_ICON|SHGFI_USEFILEATTRIBUTES);

			CString info;
			file->GetTooltipFileInfo(info);

			pNotify->ti->hIcon = shfi.hIcon;
			pNotify->ti->sTooltip = info;

			SetFocus();

			return TRUE;
		}
		default:
			if(pNotify->ti->hIcon)
				pNotify->ti->hIcon = DuplicateIcon(AfxGetInstanceHandle(), pNotify->ti->hIcon);
			return TRUE;
	}
}

void CSharedFilesWnd::SetTTDelay()
{
	m_ttip.SetDelayTime(TTDT_AUTOPOP, 20000);
	m_ttip.SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay() * 500);
}
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --