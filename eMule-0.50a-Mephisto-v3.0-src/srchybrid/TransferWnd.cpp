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
#include "HelpIDs.h"
#include "UploadBandwidthThrottler.h" //Xman Xtreme upload

#include ".\MiniMule\SystemInfo.h" // CPU/MEM usage [$ick$/Stulle] - Max
#include "XMessageBox.h" // Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	DFLT_TOOLBAR_BTN_WIDTH	24

#define	WND_SPLITTER_YOFF	10
#define	WND_SPLITTER_HEIGHT	4

#define	WND1_BUTTON_XOFF	8
//Xman see all sources
/*
#define	WND1_BUTTON_WIDTH	170
*/
#define	WND1_BUTTON_WIDTH	230
//Xman end
#define	WND1_BUTTON_HEIGHT	22	// don't set the height to something different than 22 unless you know exactly what you are doing!
#define	WND1_NUM_BUTTONS	6

#define	WND2_BUTTON_XOFF	8
//Xman see all sources
/*
#define	WND2_BUTTON_WIDTH	170
*/
#define	WND2_BUTTON_WIDTH	230
//Xman end
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
	ON_WM_HELPINFO()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SETTINGCHANGE()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_PAINT()
	ON_WM_SYSCOMMAND()
	// ==> XP Style Menu [Xanatos] - Stulle
	ON_WM_MEASUREITEM()
	ON_WM_MENUCHAR()
	// <== XP Style Menu [Xanatos] - Stulle
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
	m_dwTopListIDC = 0; // Advanced Transfer Window Layout [Stulle] - Stulle
}

CTransferWnd::~CTransferWnd()
{
	delete m_btnWnd1;
	delete m_btnWnd2;
	delete m_tooltipCats;
	ASSERT( m_pDragImage == NULL );
	delete m_pDragImage;
	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	VERIFY(m_mnuCatPriority.DestroyMenu());
	VERIFY(m_mnuCatDlMode.DestroyMenu());
	VERIFY(m_mnuCatViewFilter.DestroyMenu());
	VERIFY(m_mnuCategory.DestroyMenu());
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
}

void CTransferWnd::OnInitialUpdate()
{
	CResizableFormView::OnInitialUpdate();
	InitWindowStyles(this);

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
	// ==> Client queue progress bar [Commander] - Stulle
	AddAnchor(IDC_QUEUE, BOTTOM_LEFT, CSize(73,100));
	AddAnchor(IDC_QUEUE2, CSize(73,100), BOTTOM_RIGHT);
	// <== Client queue progress bar [Commander] - Stulle
	AddAnchor(IDC_QUEUECOUNT_LABEL, BOTTOM_LEFT);
	AddAnchor(IDC_QUEUE_REFRESH_BUTTON, BOTTOM_RIGHT);
	AddAnchor(IDC_DLTAB, TOP_CENTER, TOP_RIGHT);

	// ==> CPU/MEM usage [$ick$/Stulle] - Max 
	AddAnchor(IDC_MEMCOUNT,BOTTOM_LEFT); // contador
	AddAnchor(IDC_MEM,BOTTOM_LEFT); // texto
	AddAnchor(IDC_CPUCOUNT,BOTTOM_LEFT);  
	AddAnchor(IDC_CPU,BOTTOM_LEFT); 
	bool bEnable = thePrefs.GetSysInfo();
	GetDlgItem(IDC_CPU)->ShowWindow((bEnable ? SW_SHOW : SW_HIDE));
	GetDlgItem(IDC_CPUCOUNT)->ShowWindow((bEnable ? SW_SHOW : SW_HIDE));
	GetDlgItem(IDC_MEM)->ShowWindow((bEnable ? SW_SHOW : SW_HIDE));
	GetDlgItem(IDC_MEMCOUNT)->ShowWindow((bEnable ? SW_SHOW : SW_HIDE));
	// <== CPU/MEM usage [$ick$/Stulle] - Max 

	// ==> Advanced Transfer Window Layout [Stulle] - Stulle
	/*
	switch (thePrefs.GetTransferWnd1()) {
	*/
	UINT uTransferWnd1 = thePrefs.GetTransferWnd1();
//	ShowList(IDC_DOWNLOADLIST);
	if (thePrefs.GetSplitWindow())
	{
		m_dwShowListIDC = IDC_DOWNLOADLIST + IDC_UPLOADLIST;
		switch (uTransferWnd1)
		{
			default:
			case 1:
				m_dwTopListIDC = IDC_DOWNLOADLIST;
				break;
			case 2:
				m_dwTopListIDC = IDC_UPLOADLIST;
				break;
			case 3:
				m_dwTopListIDC = IDC_QUEUELIST;
				break;
			case 4:
				m_dwTopListIDC = IDC_DOWNLOADCLIENTS;
				break;
			case 5:
				m_dwTopListIDC = IDC_CLIENTLIST;
				break;
		}
	}
	else
	switch (uTransferWnd1) {
	// <== Advanced Transfer Window Layout [Stulle] - Stulle
		default:
		// ==> Advanced Transfer Window Layout [Stulle] - Stulle
		/*
		case 0: {
			m_dwShowListIDC = IDC_DOWNLOADLIST + IDC_UPLOADLIST;
			break;
		}
		*/
		// <== Advanced Transfer Window Layout [Stulle] - Stulle
		case 1:
			m_dwShowListIDC = IDC_DOWNLOADLIST;
			break;
		case 2:
			m_dwShowListIDC = IDC_UPLOADLIST;
			break;
		case 3:
			m_dwShowListIDC = IDC_QUEUELIST;
			break;
		case 4:
			m_dwShowListIDC = IDC_DOWNLOADCLIENTS;
			break;
		case 5:
			m_dwShowListIDC = IDC_CLIENTLIST;
			break;
	}

	//cats
	rightclickindex=-1;

	downloadlistactive=true;
	m_bIsDragging=false;

	// show & cat-tabs
	m_dlTab.ModifyStyle(0, TCS_OWNERDRAWFIXED);
	m_dlTab.SetPadding(CSize(6, 4));
	if (theApp.IsVistaThemeActive())
		m_dlTab.ModifyStyle(0, WS_CLIPCHILDREN);
	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	/*
	thePrefs.GetCategory(0)->strTitle = GetCatTitle(thePrefs.GetCategory(0)->filter);
	thePrefs.GetCategory(0)->strIncomingPath = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
	thePrefs.GetCategory(0)->care4all = true;

	for (int ix=0;ix<thePrefs.GetCatCount();ix++)
		m_dlTab.InsertItem(ix,thePrefs.GetCategory(ix)->strTitle);
	*/
	CreateCategoryMenus();

	for (int ix=0; ix < thePrefs.GetCatCount(); ix++)
	{
		Category_Struct* curCat = thePrefs.GetCategory(ix);
		// TO WATCH FOR CATEGORY STATUS
		/*if (ix == 0 && curCat->viewfilters.nFromCats == 2)
			curCat->viewfilters.nFromCats = 0;
		else*/ if (curCat->viewfilters.nFromCats != 2 && ix != 0 && theApp.downloadqueue->GetCategoryFileCount(ix) != 0)
			curCat->viewfilters.nFromCats = 2;
		m_dlTab.InsertItem(ix,thePrefs.GetCategory(ix)->strTitle );
	}
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

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

	OnBackcolor(); // Design Settings [eWombat/Stulle] - Max

	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	/*
	VerifyCatTabSize();
	*/
	VerifyCatTabSize(true);
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
    Localize();

	// ==> Visual Studio 2010 Compatibility [Stulle/Avi-3k/ied] - Stulle
#if _MSC_VER>=1600
	m_Refresh.ModifyStyle(0,BS_OWNERDRAW,0);
#endif
	// <== Visual Studio 2010 Compatibility [Stulle/Avi-3k/ied] - Stulle

	// ==> Client queue progress bar [Commander] - Stulle
	bold.CreateFont(10,0,0,1,FW_BOLD,0,0,0,0,3,2,1,34,_T("Small Fonts"));

	queueBar.SetFont(&bold);
	queueBar.SetBkColor(GetSysColor(COLOR_WINDOW));
	queueBar.SetShowPercent();
	queueBar.SetGradientColors(RGB(0, 224, 0), RGB(224, 224, 0));
	queueBar2.SetFont(&bold);
	queueBar2.SetBkColor(GetSysColor(COLOR_WINDOW));
	queueBar2.SetShowPercent();
	queueBar2.SetGradientColors(RGB(224, 224, 0), RGB(224, 0, 0));
	if(!thePrefs.ShowClientQueueProgressBar()){
		GetDlgItem(IDC_QUEUE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_QUEUE2)->ShowWindow(SW_HIDE);
	}
	// <== Client queue progress bar [Commander] - Stulle
}

void CTransferWnd::ShowQueueCount(uint32 number)
{
	TCHAR buffer[100];
	_sntprintf(buffer, _countof(buffer), _T("%u (%u ") + GetResString(IDS_BANNED).MakeLower() + _T(")"), number, theApp.clientlist->GetBannedCount());
	buffer[_countof(buffer) - 1] = _T('\0');
	GetDlgItem(IDC_QUEUECOUNT)->SetWindowText(buffer);

	// ==> Client queue progress bar [Commander] - Stulle
	if(!thePrefs.ShowClientQueueProgressBar()){
		GetDlgItem(IDC_QUEUE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_QUEUE2)->ShowWindow(SW_HIDE);
	}
	else{
		GetDlgItem(IDC_QUEUE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_QUEUE2)->ShowWindow(SW_SHOW);
		UINT iMaxQueueSize = thePrefs.GetQueueSize() + max(thePrefs.GetQueueSize()/4, 200);
		queueBar.SetRange32(0, thePrefs.GetQueueSize()); //Softlimit -> GetQueueSize | Hardlimit -> (GetQueueSize + (GetQueueSize/4))
		queueBar2.SetRange32(thePrefs.GetQueueSize()+1, iMaxQueueSize);
		if (number<=thePrefs.GetQueueSize()){
			queueBar.SetPos(number);
			queueBar2.SetPos(thePrefs.GetQueueSize()+1);
		}else {
			queueBar.SetPos(thePrefs.GetQueueSize());
			queueBar2.SetPos(number);
		}
	}
	// <== Client queue progress bar [Commander] - Stulle
}

void CTransferWnd::DoDataExchange(CDataExchange* pDX)
{
	CResizableFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DOWNLOAD_ICO, *m_btnWnd1);
	DDX_Control(pDX, IDC_UPLOAD_ICO, *m_btnWnd2);
	DDX_Control(pDX, IDC_DLTAB, m_dlTab);
	// ==> Visual Studio 2010 Compatibility [Stulle/Avi-3k/ied] - Stulle
#if _MSC_VER>=1600
	DDX_Control(pDX, IDC_QUEUE_REFRESH_BUTTON, m_Refresh);
#endif
	// <== Visual Studio 2010 Compatibility [Stulle/Avi-3k/ied] - Stulle
	// ==> Client queue progress bar [Commander] - Stulle
	DDX_Control(pDX, IDC_QUEUE, queueBar);
	DDX_Control(pDX, IDC_QUEUE2, queueBar2);
	// <== Client queue progress bar [Commander] - Stulle
	DDX_Control(pDX, IDC_UPLOADLIST, uploadlistctrl);
	DDX_Control(pDX, IDC_DOWNLOADLIST, downloadlistctrl);
	DDX_Control(pDX, IDC_QUEUELIST, queuelistctrl);
	DDX_Control(pDX, IDC_CLIENTLIST, clientlistctrl);
	DDX_Control(pDX, IDC_DOWNLOADCLIENTS, downloadclientsctrl);
}

void CTransferWnd::DoResize(int delta)
{
	CSplitterControl::ChangeHeight(&downloadlistctrl, delta);
	// ==> Advanced Transfer Window Layout [Stulle] - Stulle
	/*
	CSplitterControl::ChangeHeight(&uploadlistctrl, -delta, CW_BOTTOMALIGN);
	CSplitterControl::ChangeHeight(&queuelistctrl, -delta, CW_BOTTOMALIGN);
	CSplitterControl::ChangeHeight(&clientlistctrl, -delta, CW_BOTTOMALIGN);
	CSplitterControl::ChangeHeight(&downloadclientsctrl, -delta, CW_BOTTOMALIGN);
	CSplitterControl::ChangePos(m_btnWnd2, 0, delta);

	UpdateSplitterRange();

	if (m_dwShowListIDC == IDC_DOWNLOADLIST + IDC_UPLOADLIST)
	{
		downloadlistctrl.Invalidate();
		downloadlistctrl.UpdateWindow();
		CHeaderCtrl* pHeaderUpdate = NULL;
		switch (m_uWnd2)
		{
		case wnd2Uploading:
			pHeaderUpdate = uploadlistctrl.GetHeaderCtrl();
			uploadlistctrl.Invalidate();
			uploadlistctrl.UpdateWindow();
			break;
		case wnd2OnQueue:
			pHeaderUpdate = queuelistctrl.GetHeaderCtrl();
			queuelistctrl.Invalidate();
			queuelistctrl.UpdateWindow();
			break;
		case wnd2Clients:
			pHeaderUpdate = clientlistctrl.GetHeaderCtrl();
			clientlistctrl.Invalidate();
			clientlistctrl.UpdateWindow();
			break;
		case wnd2Downloading:
			pHeaderUpdate = downloadclientsctrl.GetHeaderCtrl();
			downloadclientsctrl.Invalidate();
			downloadclientsctrl.UpdateWindow();
			break;
		default:
			ASSERT(0);
		}
		if (pHeaderUpdate != NULL)
		{
			pHeaderUpdate->Invalidate();
			pHeaderUpdate->UpdateWindow();
		}
	}
	*/
	switch (m_dwTopListIDC)
	{
		case IDC_UPLOADLIST:
			//CSplitterControl::ChangeHeight(&downloadlistctrl, -delta, CW_BOTTOMALIGN);
			CSplitterControl::ChangeHeight(&uploadlistctrl, delta);
			CSplitterControl::ChangeHeight(&queuelistctrl, -delta, CW_BOTTOMALIGN);
			CSplitterControl::ChangeHeight(&clientlistctrl, -delta, CW_BOTTOMALIGN);
			CSplitterControl::ChangeHeight(&downloadclientsctrl, -delta, CW_BOTTOMALIGN);
			CSplitterControl::ChangePos(m_btnWnd2, 0, delta);
			break;
		case IDC_QUEUELIST:
			//CSplitterControl::ChangeHeight(&downloadlistctrl, -delta, CW_BOTTOMALIGN);
			CSplitterControl::ChangeHeight(&uploadlistctrl, -delta, CW_BOTTOMALIGN);
			CSplitterControl::ChangeHeight(&queuelistctrl, delta);
			CSplitterControl::ChangeHeight(&clientlistctrl, -delta, CW_BOTTOMALIGN);
			CSplitterControl::ChangeHeight(&downloadclientsctrl, -delta, CW_BOTTOMALIGN);
			CSplitterControl::ChangePos(m_btnWnd2, 0, delta);
			break;
		case IDC_DOWNLOADCLIENTS:
			//CSplitterControl::ChangeHeight(&downloadlistctrl, -delta, CW_BOTTOMALIGN);
			CSplitterControl::ChangeHeight(&uploadlistctrl, -delta, CW_BOTTOMALIGN);
			CSplitterControl::ChangeHeight(&queuelistctrl, -delta, CW_BOTTOMALIGN);
			CSplitterControl::ChangeHeight(&clientlistctrl, -delta, CW_BOTTOMALIGN);
			CSplitterControl::ChangeHeight(&downloadclientsctrl, delta);
			CSplitterControl::ChangePos(m_btnWnd2, 0, delta);
			break;
		case IDC_CLIENTLIST:
			//CSplitterControl::ChangeHeight(&downloadlistctrl, -delta, CW_BOTTOMALIGN);
			CSplitterControl::ChangeHeight(&uploadlistctrl, -delta, CW_BOTTOMALIGN);
			CSplitterControl::ChangeHeight(&queuelistctrl, -delta, CW_BOTTOMALIGN);
			CSplitterControl::ChangeHeight(&clientlistctrl, delta);
			CSplitterControl::ChangeHeight(&downloadclientsctrl, -delta, CW_BOTTOMALIGN);
			CSplitterControl::ChangePos(m_btnWnd2, 0, delta);
			break;
		default:
		case IDC_DOWNLOADLIST:
			//CSplitterControl::ChangeHeight(&downloadlistctrl, delta);
			CSplitterControl::ChangeHeight(&uploadlistctrl, -delta, CW_BOTTOMALIGN);
			CSplitterControl::ChangeHeight(&queuelistctrl, -delta, CW_BOTTOMALIGN);
			CSplitterControl::ChangeHeight(&clientlistctrl, -delta, CW_BOTTOMALIGN);
			CSplitterControl::ChangeHeight(&downloadclientsctrl, -delta, CW_BOTTOMALIGN);
			CSplitterControl::ChangePos(m_btnWnd2, 0, delta);
			break;
	}

	UpdateSplitterRange();

	if (m_dwShowListIDC == IDC_DOWNLOADLIST + IDC_UPLOADLIST)
	{
		switch (m_dwTopListIDC)
		{
			case IDC_UPLOADLIST:
				uploadlistctrl.Invalidate();
				uploadlistctrl.UpdateWindow();
				break;
			case IDC_QUEUELIST:
				queuelistctrl.Invalidate();
				queuelistctrl.UpdateWindow();
				break;
			case IDC_DOWNLOADCLIENTS:
				downloadclientsctrl.Invalidate();
				downloadclientsctrl.UpdateWindow();
				break;
			case IDC_CLIENTLIST:
				clientlistctrl.Invalidate();
				clientlistctrl.UpdateWindow();
				break;
			default:
			case IDC_DOWNLOADLIST:
				downloadlistctrl.Invalidate();
				downloadlistctrl.UpdateWindow();
				break;
		}

		CHeaderCtrl* pHeaderUpdate = NULL;
		switch (m_uWnd2)
		{
			case wnd2Uploading:
				uploadlistctrl.Invalidate();
				uploadlistctrl.UpdateWindow();
				pHeaderUpdate = uploadlistctrl.GetHeaderCtrl();
				break;
			case wnd2OnQueue:
				queuelistctrl.Invalidate();
				queuelistctrl.UpdateWindow();
				pHeaderUpdate = queuelistctrl.GetHeaderCtrl();
				break;
			case wnd2Clients:
				clientlistctrl.Invalidate();
				clientlistctrl.UpdateWindow();
				pHeaderUpdate = clientlistctrl.GetHeaderCtrl();
				break;
			case wnd2Downloading:
				downloadclientsctrl.Invalidate();
				downloadclientsctrl.UpdateWindow();
				pHeaderUpdate = downloadclientsctrl.GetHeaderCtrl();
				break;
			default:
				ASSERT(0);
		}
		if (pHeaderUpdate != NULL)
		{
			pHeaderUpdate->Invalidate();
			pHeaderUpdate->UpdateWindow();
		}
	}
	// <== Advanced Transfer Window Layout [Stulle] - Stulle
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
	// ==> Advanced Transfer Window Layout [Stulle] - Stulle
	/*
	downloadlistctrl.GetWindowRect(rcDown);
	*/
	switch (m_dwTopListIDC)
	{
		case IDC_UPLOADLIST:
			uploadlistctrl.GetWindowRect(rcDown);
			break;
		case IDC_QUEUELIST:
			queuelistctrl.GetWindowRect(rcDown);
			break;
		case IDC_DOWNLOADCLIENTS:
			downloadclientsctrl.GetWindowRect(rcDown);
			break;
		case IDC_CLIENTLIST:
			clientlistctrl.GetWindowRect(rcDown);
			break;
		default:
		case IDC_DOWNLOADLIST:
			downloadlistctrl.GetWindowRect(rcDown);
			break;
	}
	// <== Advanced Transfer Window Layout [Stulle] - Stulle
	ScreenToClient(rcDown);

	CRect rcUp;
	// ==> Advanced Transfer Window Layout [Stulle] - Stulle
	if(m_dwTopListIDC == IDC_DOWNLOADCLIENTS)
		clientlistctrl.GetWindowRect(rcUp);
	else
	// <== Advanced Transfer Window Layout [Stulle] - Stulle
		downloadclientsctrl.GetWindowRect(rcUp);
	ScreenToClient(rcUp);

	thePrefs.SetSplitterbarPosition((rcDown.bottom * 100) / rcWnd.Height());

	RemoveAnchor(*m_btnWnd2);
	RemoveAnchor(IDC_DOWNLOADLIST);
	RemoveAnchor(IDC_UPLOADLIST);
	RemoveAnchor(IDC_QUEUELIST);
	RemoveAnchor(IDC_CLIENTLIST);
	RemoveAnchor(IDC_DOWNLOADCLIENTS);

	// ==> CPU/MEM usage [$ick$/Stulle] - Max 
	RemoveAnchor(IDC_MEMCOUNT);
	RemoveAnchor(IDC_MEM);
	RemoveAnchor(IDC_CPUCOUNT);
	RemoveAnchor(IDC_CPU);
	// <== CPU/MEM usage [$ick$/Stulle] - Max 

	AddAnchor(*m_btnWnd2, CSize(0, thePrefs.GetSplitterbarPosition()));
	AddAnchor(IDC_DOWNLOADLIST, TOP_LEFT, CSize(100, thePrefs.GetSplitterbarPosition()));
	// ==> Advanced Transfer Window Layout [Stulle] - Stulle
	/*
	AddAnchor(IDC_UPLOADLIST, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
	AddAnchor(IDC_QUEUELIST, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
	AddAnchor(IDC_CLIENTLIST, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
	AddAnchor(IDC_DOWNLOADCLIENTS, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
	*/
	switch (m_dwTopListIDC)
	{
		case IDC_UPLOADLIST:
			AddAnchor(IDC_UPLOADLIST,TOP_LEFT,CSize(100,thePrefs.GetSplitterbarPosition()));
			AddAnchor(IDC_QUEUELIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
			AddAnchor(IDC_CLIENTLIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
			AddAnchor(IDC_DOWNLOADCLIENTS, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
			break;
		case IDC_QUEUELIST:
			AddAnchor(IDC_UPLOADLIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
			AddAnchor(IDC_QUEUELIST,TOP_LEFT,CSize(100,thePrefs.GetSplitterbarPosition()));
			AddAnchor(IDC_CLIENTLIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
			AddAnchor(IDC_DOWNLOADCLIENTS, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
			break;
		case IDC_DOWNLOADCLIENTS:
			AddAnchor(IDC_UPLOADLIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
			AddAnchor(IDC_QUEUELIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
			AddAnchor(IDC_CLIENTLIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
			AddAnchor(IDC_DOWNLOADCLIENTS,TOP_LEFT,CSize(100, thePrefs.GetSplitterbarPosition()));
			break;
		case IDC_CLIENTLIST:
			AddAnchor(IDC_UPLOADLIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
			AddAnchor(IDC_QUEUELIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
			AddAnchor(IDC_CLIENTLIST,TOP_LEFT,CSize(100,thePrefs.GetSplitterbarPosition()));
			AddAnchor(IDC_DOWNLOADCLIENTS, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
			break;
		default:
		case IDC_DOWNLOADLIST:
			AddAnchor(IDC_UPLOADLIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
			AddAnchor(IDC_QUEUELIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
			AddAnchor(IDC_CLIENTLIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
			AddAnchor(IDC_DOWNLOADCLIENTS, CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
			break;
	}
	// <== Advanced Transfer Window Layout [Stulle] - Stulle

	// ==> CPU/MEM usage [$ick$/Stulle] - Max 
	AddAnchor(IDC_MEMCOUNT,BOTTOM_LEFT,BOTTOM_RIGHT); // contador
	AddAnchor(IDC_MEM,BOTTOM_LEFT,BOTTOM_RIGHT); // texto
	AddAnchor(IDC_CPUCOUNT,BOTTOM_LEFT,BOTTOM_RIGHT);  
	AddAnchor(IDC_CPU,BOTTOM_LEFT,BOTTOM_RIGHT); 
	// <== CPU/MEM usage [$ick$/Stulle] - Max 

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
/*
void CTransferWnd::UpdateFilesCount(int iCount)
{
	if (m_dwShowListIDC == IDC_DOWNLOADLIST || m_dwShowListIDC == IDC_DOWNLOADLIST + IDC_UPLOADLIST)
	{
		CString strBuffer;
		strBuffer.Format(_T("%s (%u)"), GetResString(IDS_TW_DOWNLOADS), iCount);
		m_btnWnd1->SetWindowText(strBuffer);
	}
}
*/
void CTransferWnd::UpdateFilesCount(UINT iCount, UINT countsources, UINT countreadyfiles) 
{
	// ==> Advanced Transfer Window Layout [Stulle] - Stulle
	/*
	if (m_dwShowListIDC == IDC_DOWNLOADLIST || m_dwShowListIDC == IDC_DOWNLOADLIST + IDC_UPLOADLIST)
	*/
	if (m_dwTopListIDC == IDC_DOWNLOADLIST)
	// <== Advanced Transfer Window Layout [Stulle] - Stulle
	{
		CString strBuffer;
		strBuffer.Format(_T("%s (%u/%u)/%s (%u)"), GetResString(IDS_TW_DOWNLOADS), countreadyfiles, iCount, GetResString(IDS_DL_SOURCES), countsources);
		m_btnWnd1->SetWindowText(strBuffer);
	}
}
//Xman end

void CTransferWnd::UpdateListCount(EWnd2 listindex, int iCount /*=-1*/)
{
	switch (m_dwShowListIDC)
	{
		case IDC_DOWNLOADLIST + IDC_UPLOADLIST: {
			// ==> Advanced Transfer Window Layout [Stulle] - Stulle [Fafner: fix not updating lists]
			switch (m_dwTopListIDC) {
				case IDC_DOWNLOADLIST:
					break;
				case IDC_UPLOADLIST:
					if (listindex == wnd2Uploading)
					{
						CString strBuffer;
        				uint32 itemCount = iCount == -1 ? uploadlistctrl.GetItemCount() : iCount;
						//Xman Xtreme Upload
						/*
        				uint32 activeCount = theApp.uploadqueue->GetActiveUploadsCount();
						if (activeCount >= itemCount)
							strBuffer.Format(_T(" (%i)"), itemCount);
						else
							strBuffer.Format(_T(" (%i/%i)"), activeCount, itemCount);
						//Upload Speed Sens
						strBuffer.AppendFormat(_T(" %i Max"), theApp.uploadqueue->GetActiveUploadsCountLongPerspective());
						*/
						// ==> Mephisto Upload - Mephisto
						/*
						uint32 activeCount = theApp.uploadBandwidthThrottler->GetNumberOfFullyActivatedSlots();
						//Xman upload health
						//Xman count block/success send
						float health = theApp.uploadBandwidthThrottler->GetAvgHealth();
						float avgblocks = theApp.uploadBandwidthThrottler->GetAvgBlockRatio();
						if(thePrefs.ShowBlockRatio())
							strBuffer.Format(_T(" (%i/%i) %0.0f%% %0.0f%%"), activeCount, itemCount, health, avgblocks);
						else
							strBuffer.Format(_T(" (%i/%i) %0.0f%%"), activeCount, itemCount, health);
						*/
						uint32 activeCount = theApp.uploadBandwidthThrottler->GetActiveSlotCount();
						//Xman upload health
						//Xman count block/success send
						float health = theApp.uploadBandwidthThrottler->GetAvgHealth();
						float avgblocks = theApp.uploadBandwidthThrottler->GetAvgBlockRatio();
						CString strReached = (theApp.uploadBandwidthThrottler->GetBandwithReached()?_T("R"):_T("NR"));
						if(thePrefs.ShowBlockRatio())
							strBuffer.Format(_T(" (%i/%i) %0.0f%% %0.0f%%, %s"), activeCount, itemCount, health, avgblocks, strReached);
						else
							strBuffer.Format(_T(" (%i/%i) %0.0f%%, %s"), activeCount, itemCount, health, strReached);
						// <== Mephisto Upload - Mephisto
						//Xman end
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
					ASSERT(0);
			}
			// <== Advanced Transfer Window Layout [Stulle] - Stulle [Fafner: fix not updating lists]
			if (m_uWnd2 != listindex)
				return;
			CString strBuffer;
			switch (m_uWnd2)
			{
				case wnd2Uploading: {
					uint32 itemCount = iCount == -1 ? uploadlistctrl.GetItemCount() : iCount;
					//Xman Xtreme Upload
					/*
					uint32 activeCount = theApp.uploadqueue->GetActiveUploadsCount();
					if (activeCount >= itemCount)
						strBuffer.Format(_T(" (%i)"), itemCount);
					else
						strBuffer.Format(_T(" (%i/%i)"), activeCount, itemCount);
					*/
					// ==> Mephisto Upload - Mephisto
					/*
					uint32 activeCount = theApp.uploadBandwidthThrottler->GetNumberOfFullyActivatedSlots();
					//Xman upload health
					//Xman count block/success send
					float health = theApp.uploadBandwidthThrottler->GetAvgHealth();
					float avgblocks = theApp.uploadBandwidthThrottler->GetAvgBlockRatio();
					if(thePrefs.ShowBlockRatio())
						strBuffer.Format(_T(" (%i/%i) %0.0f%% %0.0f%%"), activeCount, itemCount, health, avgblocks);
					else
						strBuffer.Format(_T(" (%i/%i) %0.0f%%"), activeCount, itemCount, health);
					*/
					uint32 activeCount = theApp.uploadBandwidthThrottler->GetActiveSlotCount();
					//Xman upload health
					//Xman count block/success send
					float health = theApp.uploadBandwidthThrottler->GetAvgHealth();
					float avgblocks = theApp.uploadBandwidthThrottler->GetAvgBlockRatio();
					CString strReached = (theApp.uploadBandwidthThrottler->GetBandwithReached()?_T("R"):_T("NR"));
					if(thePrefs.ShowBlockRatio())
						strBuffer.Format(_T(" (%i/%i) %0.0f%% %0.0f%%, %s"), activeCount, itemCount, health, avgblocks, strReached);
					else
						strBuffer.Format(_T(" (%i/%i) %0.0f%%, %s"), activeCount, itemCount, health, strReached);
					// <== Mephisto Upload - Mephisto
					//Xman end
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
				//Xman Xtreme Upload
				/*
				uint32 activeCount = theApp.uploadqueue->GetActiveUploadsCount();
				if (activeCount >= itemCount)
					strBuffer.Format(_T(" (%i)"), itemCount);
				else
					strBuffer.Format(_T(" (%i/%i)"), activeCount, itemCount);
				*/
				// ==> Mephisto Upload - Mephisto
				/*
				uint32 activeCount = theApp.uploadBandwidthThrottler->GetNumberOfFullyActivatedSlots();
				//Xman upload health
				//Xman count block/success send
				float health = theApp.uploadBandwidthThrottler->GetAvgHealth();
				float avgblocks = theApp.uploadBandwidthThrottler->GetAvgBlockRatio();
				if(thePrefs.ShowBlockRatio())
					strBuffer.Format(_T(" (%i/%i) %0.0f%% %0.0f%%"), activeCount, itemCount, health, avgblocks);
				else
					strBuffer.Format(_T(" (%i/%i) %0.0f%%"), activeCount, itemCount, health);
				*/
				uint32 activeCount = theApp.uploadBandwidthThrottler->GetActiveSlotCount();
				//Xman upload health
				//Xman count block/success send
				float health = theApp.uploadBandwidthThrottler->GetAvgHealth();
				float avgblocks = theApp.uploadBandwidthThrottler->GetAvgBlockRatio();
				CString strReached = (theApp.uploadBandwidthThrottler->GetBandwithReached()?_T("R"):_T("NR"));
				if(thePrefs.ShowBlockRatio())
					strBuffer.Format(_T(" (%i/%i) %0.0f%% %0.0f%%, %s"), activeCount, itemCount, health, avgblocks, strReached);
				else
					strBuffer.Format(_T(" (%i/%i) %0.0f%%, %s"), activeCount, itemCount, health, strReached);
				// <== Mephisto Upload - Mephisto
				//Xman end
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
		// ==> Advanced Transfer Window Layout [Stulle] - Stulle
		if(m_dwTopListIDC == IDC_DOWNLOADCLIENTS)
		{
			SwitchUploadList();
			return;
		}
		// <== Advanced Transfer Window Layout [Stulle] - Stulle
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
		// ==> Advanced Transfer Window Layout [Stulle] - Stulle
		else if(m_dwTopListIDC == IDC_QUEUELIST)
		{
			SwitchUploadList();
			return;
		}
		// <== Advanced Transfer Window Layout [Stulle] - Stulle
		uploadlistctrl.Hide();
		clientlistctrl.Hide();
		downloadclientsctrl.Hide();
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_SHOW);
		queuelistctrl.Show();
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
		// ==> Advanced Transfer Window Layout [Stulle] - Stulle
		else if(m_dwTopListIDC == IDC_CLIENTLIST)
		{
			SwitchUploadList();
			return;
		}
		// <== Advanced Transfer Window Layout [Stulle] - Stulle
		uploadlistctrl.Hide();
		queuelistctrl.Hide();
		downloadclientsctrl.Hide();
		clientlistctrl.Show();
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
		m_btnWnd2->CheckButton(MP_VIEW2_CLIENTS);
		SetWnd2Icon(w2iClientsKnown);
	}
	else
	{
		SetWnd2(wnd2Uploading);
		// ==> Advanced Transfer Window Layout [Stulle] - Stulle
		if(m_dwTopListIDC == IDC_UPLOADLIST)
		{
			SwitchUploadList();
			return;
		}
		// <== Advanced Transfer Window Layout [Stulle] - Stulle
		queuelistctrl.Hide();
		clientlistctrl.Hide();
		downloadclientsctrl.Hide();
		uploadlistctrl.Show();
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
		m_btnWnd2->CheckButton(MP_VIEW2_UPLOADING);
		SetWnd2Icon(w2iUploading);
	}
	// ==> Advanced Transfer Window Layout [Stulle] - Stulle
	switch (m_dwTopListIDC)
	{
		case IDC_UPLOADLIST:
			uploadlistctrl.Show();
			break;
		case IDC_QUEUELIST:
			queuelistctrl.Show();
			break;
		case IDC_DOWNLOADCLIENTS:
			downloadclientsctrl.Show();
			break;
		case IDC_CLIENTLIST:
			clientlistctrl.Show();
			break;
		default:
		case IDC_DOWNLOADLIST:
			break;
	}
	// <== Advanced Transfer Window Layout [Stulle] - Stulle
	UpdateListCount(m_uWnd2);
}

void CTransferWnd::ShowWnd2(EWnd2 uWnd2)
{
	// ==> Advanced Transfer Window Layout [Stulle] - Stulle
	/*
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
		queuelistctrl.Show();
		OnBnClickedQueueRefreshButton(); //Xman update this list
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
		clientlistctrl.Show();
		//Xman SortingFix for Morph-Code-Improvement Don't Refresh item if not needed
		clientlistctrl.UpdateAll();
		//Xman end
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
		uploadlistctrl.Show();
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
		m_btnWnd2->CheckButton(MP_VIEW2_UPLOADING);
		SetWnd2Icon(w2iUploading);
	}
	*/
	queuelistctrl.Hide();
	clientlistctrl.Hide();
	uploadlistctrl.Hide();
	downloadclientsctrl.Hide();
	switch (m_dwTopListIDC)
	{
		case IDC_UPLOADLIST:
			uploadlistctrl.Show();
			break;
		case IDC_QUEUELIST:
			queuelistctrl.Show();
			break;
		case IDC_DOWNLOADCLIENTS:
			downloadclientsctrl.Show();
			break;
		case IDC_CLIENTLIST:
			clientlistctrl.Show();
			break;
		default:
		case IDC_DOWNLOADLIST:
			break;
	}
	if (uWnd2 == wnd2Downloading) 
	{
		SetWnd2(uWnd2);
		downloadclientsctrl.Show();
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
		m_btnWnd2->CheckButton(MP_VIEW2_DOWNLOADING);
		SetWnd2Icon(w2iDownloading);
	}
	else if (uWnd2 == wnd2OnQueue && !thePrefs.IsQueueListDisabled())
	{
		SetWnd2(uWnd2);
		queuelistctrl.Show();
		OnBnClickedQueueRefreshButton(); //Xman update this list
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_SHOW);
		m_btnWnd2->CheckButton(MP_VIEW2_ONQUEUE);
		SetWnd2Icon(w2iOnQueue);
	}
	else if (uWnd2 == wnd2Clients && !thePrefs.IsKnownClientListDisabled())
	{
		SetWnd2(uWnd2);
		clientlistctrl.Show();
		//Xman SortingFix for Morph-Code-Improvement Don't Refresh item if not needed
		clientlistctrl.UpdateAll();
		//Xman end
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
		m_btnWnd2->CheckButton(MP_VIEW2_CLIENTS);
		SetWnd2Icon(w2iClientsKnown);
	}
	else
	{
		SetWnd2(wnd2Uploading);
		uploadlistctrl.Show();
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
		m_btnWnd2->CheckButton(MP_VIEW2_UPLOADING);
		SetWnd2Icon(w2iUploading);
	}
	// <== Advanced Transfer Window Layout [Stulle] - Stulle
	UpdateListCount(m_uWnd2);
}

void CTransferWnd::SetWnd2(EWnd2 uWnd2)
{
	m_uWnd2 = uWnd2;
	thePrefs.SetTransferWnd2(m_uWnd2);
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

void CTransferWnd::Localize()
{
	LocalizeToolbars();
	GetDlgItem(IDC_QUEUECOUNT_LABEL)->SetWindowText(GetResString(IDS_TW_QUEUE));
	GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->SetWindowText(GetResString(IDS_SV_UPDATE));

	CreateCategoryMenus(); // Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

	uploadlistctrl.Localize();
	queuelistctrl.Localize();
	downloadlistctrl.Localize();
	clientlistctrl.Localize();
	downloadclientsctrl.Localize();

	if (m_dwShowListIDC == IDC_DOWNLOADLIST + IDC_UPLOADLIST)
		// ==> Advanced Transfer Window Layout [Stulle] - Stulle
		/*
		ShowSplitWindow();
		*/
		ShowSplitWindow(false,m_dwTopListIDC);
		// <== Advanced Transfer Window Layout [Stulle] - Stulle
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
	//Xman faster Updating of Queuelist
	/*
	CUpDownClient* update = theApp.uploadqueue->GetNextClient(NULL);

	while( update ){
		queuelistctrl.RefreshClient( update);
		update = theApp.uploadqueue->GetNextClient(update);
	}
	*/
	if (queuelistctrl.GetItemCount()>1)
	{
		theApp.emuledlg->transferwnd->GetQueueList()->UpdateAll();
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
	if (rightclickindex == -1)
		return;

	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
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
	menu.AppendMenu(MF_STRING,MP_CAT_ADD,GetResString(IDS_CAT_ADD));
	menu.AppendMenu(flag,MP_CAT_EDIT,GetResString(IDS_CAT_EDIT));
	menu.AppendMenu(flag,MP_CAT_REMOVE, GetResString(IDS_CAT_REMOVE));

	menu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);

	VERIFY( PrioMenu.DestroyMenu() );
	VERIFY( CatMenu.DestroyMenu() );
	VERIFY( menu.DestroyMenu() );
	*/
	// If the current category is '0'...  Well, we can't very well delete the default category, now can we...
	// Nor can we merge it.
	m_mnuCategory.EnableMenuItem(MP_CAT_REMOVE, rightclickindex == 0 ? MF_GRAYED : MF_ENABLED);
	m_mnuCategory.EnableMenuItem(MP_CAT_MERGE, rightclickindex == 0 ? MF_GRAYED : MF_ENABLED);
	m_mnuCategory.EnableMenuItem(8, MF_BYPOSITION);

	Category_Struct* curCat = thePrefs.GetCategory(rightclickindex);
	if (curCat) { //MORPH - HOTFIX by SiRoB, Possible crash when NULL is returned by GetCategory()
		// Check and enable the appropriate menu items in Select View Filter
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0, (curCat->viewfilters.nFromCats == 0) ? MF_CHECKED : MF_UNCHECKED);
		//m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+1, (curCat->viewfilters.nFromCats == 1) ? MF_CHECKED : MF_UNCHECKED);
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+2, (curCat->viewfilters.nFromCats == 2) ? MF_CHECKED : MF_UNCHECKED);

		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+3, (curCat->viewfilters.bComplete) ? MF_CHECKED : MF_UNCHECKED);
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+4, (curCat->viewfilters.bCompleting) ? MF_CHECKED : MF_UNCHECKED);
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+5, (curCat->viewfilters.bTransferring) ? MF_CHECKED : MF_UNCHECKED);
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+6, (curCat->viewfilters.bWaiting) ? MF_CHECKED : MF_UNCHECKED);
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+7, (curCat->viewfilters.bPaused) ? MF_CHECKED : MF_UNCHECKED);
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+8, (curCat->viewfilters.bStopped) ? MF_CHECKED : MF_UNCHECKED);
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+9, (curCat->viewfilters.bHashing) ? MF_CHECKED : MF_UNCHECKED);
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+10, (curCat->viewfilters.bErrorUnknown) ? MF_CHECKED : MF_UNCHECKED);

		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+11, (curCat->viewfilters.bVideo) ? MF_CHECKED : MF_UNCHECKED);
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+12, (curCat->viewfilters.bAudio) ? MF_CHECKED : MF_UNCHECKED);
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+13, (curCat->viewfilters.bArchives) ? MF_CHECKED : MF_UNCHECKED);
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+14, (curCat->viewfilters.bImages) ? MF_CHECKED : MF_UNCHECKED);
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+15, (curCat->viewfilters.bSuspendFilters) ? MF_CHECKED : MF_UNCHECKED);
		
		m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+16, (curCat->viewfilters.bSeenComplet) ? MF_CHECKED : MF_UNCHECKED); //MORPH - Added by SiRoB, Seen Complet filter

		// Check the appropriate menu item for the Prio menu...
	    m_mnuCatPriority.CheckMenuRadioItem(MP_PRIOLOW, MP_PRIOHIGH, MP_PRIOLOW+curCat->prio,0);
//		m_mnuCatPriority.CheckMenuItem(MP_DOWNLOAD_ALPHABETICAL, curCat->downloadInAlphabeticalOrder ? MF_CHECKED : MF_UNCHECKED);
		// Check the appropriate menu item for the Dl Mode menu...
		m_mnuCatDlMode.CheckMenuRadioItem(MP_CAT_DL_MODE, MP_CAT_DL_MODE+2, MP_CAT_DL_MODE+curCat->m_iDlMode,0);
	    m_mnuCategory.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	}
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

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
		// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
		// If the current category is '0'...  Well, we can't very well delete the default category, now can we...
		/*
		if (m_nDropIndex > 0 && thePrefs.GetCategory(m_nDropIndex)->care4all)	// not droppable
			m_dlTab.SetCurSel(-1);
		else
		*/
		// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
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
			CTypedPtrList<CPtrList, CPartFile *> selectedList;
			POSITION pos = downloadlistctrl.GetFirstSelectedItemPosition();
			while (pos != NULL)
			{ 
				int index = downloadlistctrl.GetNextSelectedItem(pos);
				if (index > -1 && (((CtrlItem_Struct *)downloadlistctrl.GetItemData(index))->type == FILE_TYPE))
					selectedList.AddTail((CPartFile *)((CtrlItem_Struct *)downloadlistctrl.GetItemData(index))->value);
			}

			while (!selectedList.IsEmpty())
			{
				CPartFile *file = selectedList.GetHead();
				selectedList.RemoveHead();
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
	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	/*
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
	}
	
	switch (wParam)
	{ 
		case MP_CAT_ADD: {
			m_nLastCatTT=-1;
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
			downloadlistctrl.ChangeCategory(0);
			thePrefs.SaveCats();
			if (thePrefs.GetCatCount()==1) 
				thePrefs.GetCategory(0)->filter=0;
			theApp.emuledlg->searchwnd->UpdateCatTabs();
			VerifyCatTabSize();
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
	*/
	Category_Struct* curCat;
	curCat = thePrefs.GetCategory(rightclickindex);

	switch (wParam)
	{
		case MP_CAT_ADD: {
			m_nLastCatTT=-1;
			AddCategoryInteractive();
			break;
		}
		case MP_CAT_SET0+17:
		case MP_CAT_EDIT: {
			m_nLastCatTT=-1;
			CString oldincpath=thePrefs.GetCatPath(rightclickindex);
			CCatDialog dialog(rightclickindex);
			dialog.DoModal();

			CString csName;
			csName.Format(_T("%s"), thePrefs.GetCategory(rightclickindex)->strTitle );
			EditCatTabLabel(rightclickindex,csName);
		
			theApp.emuledlg->searchwnd->UpdateCatTabs();
			theApp.emuledlg->transferwnd->GetDownloadList()->UpdateCurrentCategoryView();
			thePrefs.SaveCats();
			// ==> handled in dialog already [Stulle] - Stulle
			/*
			if (CompareDirectories(oldincpath, thePrefs.GetCatPath(rightclickindex) ))
				theApp.emuledlg->sharedfileswnd->Reload();
			*/
			// <== handled in dialog already [Stulle] - Stulle
			break;
		}
		case MP_CAT_MERGE: {
			int useCat;

			CSelCategoryDlg* getCatDlg = new CSelCategoryDlg((CWnd*)theApp.emuledlg);
			int nResult = getCatDlg->DoModal();

			if (nResult == IDCANCEL)
				break;

			useCat = getCatDlg->GetInput();
			delete getCatDlg;

			if (useCat == rightclickindex) break;
			if( thePrefs.UseAddRemoveInc() &&
				_tcsicmp(thePrefs.GetCatPath(useCat), thePrefs.GetCatPath(rightclickindex))!=0 &&
				_tcsicmp(thePrefs.GetCatPath(rightclickindex), thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR))!=0)
			{
				int nResult = XMessageBox(GetSafeHwnd(), GetResString(IDS_KEEPCATINCSHARED), GetResString(IDS_KEEPCATINCSHARED_TITLE), MB_YESNO | MB_DEFBUTTON2 | MB_DONOTASKAGAIN | MB_ICONQUESTION);
				if ((nResult & MB_DONOTASKAGAIN) > 0)
					thePrefs.m_bAddRemovedInc = false;
				if ((nResult & 0xFF) == IDYES)
				{
					CDirectoryItem* newDir = new CDirectoryItem(thePrefs.GetCatPath(rightclickindex));
					theApp.emuledlg->sharedfileswnd->m_ctlSharedDirTree.EditSharedDirectories(newDir, true, false);
					delete newDir;
				}
			}
			m_nLastCatTT=-1;

			if (useCat > rightclickindex) useCat--;

			theApp.downloadqueue->ResetCatParts(rightclickindex, useCat);
			thePrefs.RemoveCat(rightclickindex);
			m_dlTab.DeleteItem(rightclickindex);
			m_dlTab.SetCurSel(useCat);
			downloadlistctrl.ChangeCategory(useCat);
			downloadlistctrl.UpdateCurrentCategoryView();
			thePrefs.SaveCats();
			//MORPH NOTE - This is not really great because both categories might have shared the same incoming folder.
			theApp.emuledlg->sharedfileswnd->Reload();
			// ==> Automatic shared files updater [MoNKi] - Stulle
			if(thePrefs.GetDirectoryWatcher())
				theApp.ResetDirectoryWatcher();
			// <== Automatic shared files updater [MoNKi] - Stulle
			break;
		}
		case MP_CAT_REMOVE: {
			m_nLastCatTT=-1;
			bool toreload=( _tcsicmp(thePrefs.GetCatPath(rightclickindex), thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR))!=0);
			if(toreload && thePrefs.UseAddRemoveInc())
			{
				int nResult = XMessageBox(GetSafeHwnd(), GetResString(IDS_KEEPCATINCSHARED), GetResString(IDS_KEEPCATINCSHARED_TITLE), MB_YESNO | MB_DEFBUTTON2 | MB_DONOTASKAGAIN | MB_ICONQUESTION);
				if ((nResult & MB_DONOTASKAGAIN) > 0)
					thePrefs.m_bAddRemovedInc = false;
				if ((nResult & 0xFF) == IDYES)
				{
					CDirectoryItem* newDir = new CDirectoryItem(thePrefs.GetCatPath(rightclickindex));
					theApp.emuledlg->sharedfileswnd->m_ctlSharedDirTree.EditSharedDirectories(newDir, true, false);
					delete newDir;
				}
			}
			theApp.downloadqueue->ResetCatParts(rightclickindex);
			thePrefs.RemoveCat(rightclickindex);
			m_dlTab.DeleteItem(rightclickindex);
			m_dlTab.SetCurSel(0);
			downloadlistctrl.ChangeCategory(0);
			thePrefs.SaveCats();
			theApp.emuledlg->searchwnd->UpdateCatTabs();
			VerifyCatTabSize();
			//MORPH NOTE - this is not great because of shareSubdir we might have shared this folder already and a reload is unneeded
			// but it's still better than falsely sharing files so we go through this trouble and reload... for now...
			if (toreload)
			{ // Automatic shared files updater [MoNKi] - Stulle
				theApp.emuledlg->sharedfileswnd->Reload();
			// ==> Automatic shared files updater [MoNKi] - Stulle
				if(thePrefs.GetDirectoryWatcher())
					theApp.ResetDirectoryWatcher();
			}
			// <== Automatic shared files updater [MoNKi] - Stulle
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

			/*
		case MP_DOWNLOAD_ALPHABETICAL: {
            BOOL newSetting = !thePrefs.GetCategory(rightclickindex)->downloadInAlphabeticalOrder;
            thePrefs.GetCategory(rightclickindex)->downloadInAlphabeticalOrder = newSetting;
			thePrefs.SaveCats();
            if(newSetting) {
                // any auto prio files will be set to normal now.
                theApp.downloadqueue->RemoveAutoPrioInCat(rightclickindex, PR_NORMAL);
            }

            break;
		}
			*/

		case MP_CAT_STOPLAST: {
			theApp.downloadqueue->StopPauseLastFile(MP_STOP, rightclickindex);
			break;
		}
		case MP_CAT_PAUSELAST: {
			theApp.downloadqueue->StopPauseLastFile(MP_PAUSE, rightclickindex);
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
			// ==> Advanced Transfer Window Layout [Stulle] - Stulle
			/*
			ShowSplitWindow();
			*/
			if(thePrefs.GetSplitWindow())
				ShowList(m_dwTopListIDC);
			else
				ShowSplitWindow(false,m_dwTopListIDC);
			// <== Advanced Transfer Window Layout [Stulle] - Stulle
			break;
		case MP_VIEW1_DOWNLOADS:
			// ==> Advanced Transfer Window Layout [Stulle] - Stulle
			if(thePrefs.GetSplitWindow())
				ShowSplitWindow(false,IDC_DOWNLOADLIST);
			else
			// <== Advanced Transfer Window Layout [Stulle] - Stulle
				ShowList(IDC_DOWNLOADLIST);
			break;
		case MP_VIEW1_UPLOADING:
			// ==> Advanced Transfer Window Layout [Stulle] - Stulle
			if(thePrefs.GetSplitWindow())
				ShowSplitWindow(false,IDC_UPLOADLIST);
			else
			// <== Advanced Transfer Window Layout [Stulle] - Stulle
				ShowList(IDC_UPLOADLIST);
			break;
		case MP_VIEW1_DOWNLOADING:
			// ==> Advanced Transfer Window Layout [Stulle] - Stulle
			if(thePrefs.GetSplitWindow())
				ShowSplitWindow(false,IDC_DOWNLOADCLIENTS);
			else
			// <== Advanced Transfer Window Layout [Stulle] - Stulle
				ShowList(IDC_DOWNLOADCLIENTS);
			break;
		case MP_VIEW1_ONQUEUE:
			// ==> Advanced Transfer Window Layout [Stulle] - Stulle
			if(thePrefs.GetSplitWindow())
				ShowSplitWindow(false,IDC_QUEUELIST);
			else
			// <== Advanced Transfer Window Layout [Stulle] - Stulle
				ShowList(IDC_QUEUELIST);
			break;
		case MP_VIEW1_CLIENTS:
			// ==> Advanced Transfer Window Layout [Stulle] - Stulle
			if(thePrefs.GetSplitWindow())
				ShowSplitWindow(false,IDC_CLIENTLIST);
			else
			// <== Advanced Transfer Window Layout [Stulle] - Stulle
				ShowList(IDC_CLIENTLIST);
			break;

		// Handle the new view filter menu.
		case MP_CAT_SET0+1:
			if (rightclickindex != 0 && theApp.downloadqueue->GetCategoryFileCount(rightclickindex))
			{
				MessageBox(GetResString(IDS_CAT_FROMCATSINFO), GetResString(IDS_CAT_FROMCATSCAP));
				curCat->viewfilters.nFromCats = 2;
				EditCatTabLabel(rightclickindex, CString(curCat->strTitle));
				break;
			}
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
		case MP_HM_OPENINC:
			ShellExecute(NULL, _T("open"), curCat->strIncomingPath,NULL, NULL, SW_SHOW);
			break;
	}

	if (wParam >= MP_CAT_SET0 && wParam <= MP_CAT_SET0 + 16)
		downloadlistctrl.ChangeCategory(m_dlTab.GetCurSel());
	if (wParam >= MP_CAT_DL_MODE && wParam <= MP_CAT_DL_MODE + 2)
		curCat->m_iDlMode = wParam - MP_CAT_DL_MODE;

	return TRUE;
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
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

	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	/*
	if (!index)
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
	}
	*/
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

	int count,dwl;
	if (thePrefs.ShowCatTabInfos()) {
		CPartFile* cur_file;
		count=dwl=0;
		for (int i=0;i<theApp.downloadqueue->GetFileCount();i++) {
			cur_file=theApp.downloadqueue->GetFileByIndex(i);
			if (cur_file==0) continue;
			if (cur_file->CheckShowItemInGivenCat(index)) {
				if (cur_file->GetTransferringSrcCount()>0) ++dwl;
			}
		}
		CString title=newlabel;
		downloadlistctrl.GetCompleteDownloads(index, count);
		newlabel.Format(_T("%s %i/%i"),title,dwl,count);
	}

	tabitem.pszText = newlabel.LockBuffer();
	m_dlTab.SetItem(index,&tabitem);
	newlabel.UnlockBuffer();

	VerifyCatTabSize();
}

int CTransferWnd::AddCategory(CString newtitle,CString newincoming,CString newcomment, CString newautocat, bool addTab)
{
	Category_Struct* newcat=new Category_Struct;
	newcat->strTitle = newtitle;
	newcat->prio=PR_NORMAL;
	newcat->strIncomingPath = newincoming;
	newcat->strComment = newcomment;
	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	/*
	newcat->regexp.Empty();
	newcat->ac_regexpeval=false;
	newcat->autocat=newautocat;
    newcat->downloadInAlphabeticalOrder = FALSE;
	newcat->filter=0;
	newcat->filterNeg=false;
	newcat->care4all=false;
	newcat->color= (DWORD)-1;
	*/
//  newcat->downloadInAlphabeticalOrder = FALSE; // ZZ:DownloadManager
	newcat->m_iDlMode = 0;

	// Initialize View Filter Variables
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
	newcat->selectioncriteria.bAdvancedFilterMask = true;
	newcat->selectioncriteria.bFileSize = true;
	newcat->bResumeFileOnlyInSameCat = false;
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

	int index=thePrefs.AddCat(newcat);
	if (addTab) m_dlTab.InsertItem(index,newtitle);
	VerifyCatTabSize();
	
	return index;
}

int CTransferWnd::AddCategoryInteractive()
{
	m_nLastCatTT=-1;
	int newindex = AddCategory(_T("?"),thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR),_T(""),_T(""),false);
	CCatDialog dialog(newindex);
	if (dialog.DoModal() == IDOK)
	{
		theApp.emuledlg->searchwnd->UpdateCatTabs();
		m_dlTab.InsertItem(newindex,thePrefs.GetCategory(newindex)->strTitle);
		m_dlTab.SetTabTextColor(newindex, thePrefs.GetCatColor(newindex) );
		EditCatTabLabel(newindex);
		thePrefs.SaveCats();
		VerifyCatTabSize();
		// ==> handled in dialog already [Stulle] - Stulle
		/*
		if (CompareDirectories(thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR), thePrefs.GetCatPath(newindex)))
			theApp.emuledlg->sharedfileswnd->Reload();
		*/
		// <== handled in dialog already [Stulle] - Stulle
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
	unsigned int nTab = m_dlTab.HitTest( &hitinfo );

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
			//Xman // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
			/*
			speed += cur_file->GetDatarate() / 1024.0F;
			*/
			speed += cur_file->GetDownloadDatarate() / 1024.0F;
			//Xman end
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
	int compl = downloadlistctrl.GetCompleteDownloads(tab, total);

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
	UINT from=m_dlTab.GetLastMovementSource();
	UINT to=m_dlTab.GetLastMovementDestionation();

	if (from==0 || to==0 || from==to-1) return;

	// do the reorder
	
	// rearrange the cat-map
	if (!thePrefs.MoveCat(from,to)) return;

	// update partfile-stored assignment
	theApp.downloadqueue->MoveCat((uint8)from,(uint8)to);

	// move category of completed files
	downloadlistctrl.MoveCompletedfilesCat((uint8)from,(uint8)to);

	// of the tabcontrol itself
	m_dlTab.ReorderTab(from,to);

	UpdateCatTabTitles();
	theApp.emuledlg->searchwnd->UpdateCatTabs();

	if (to>from) --to;
	m_dlTab.SetCurSel(to);
	downloadlistctrl.ChangeCategory(to);
}

// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
/*
void CTransferWnd::VerifyCatTabSize()
{
	if (m_dwShowListIDC != IDC_DOWNLOADLIST && m_dwShowListIDC != IDC_UPLOADLIST + IDC_DOWNLOADLIST)
		return;
*/
// [Comment by Mighty Knife:]
//
// Verify the proper positioning of the download tab's. If the
// positioning is wrong, the tab's are repositioned.
//
// If the download-list is not visible, no repositioning of
// the tab's is performed. That's the case if the download list
// is hidden by the category buttons next to the download tab's.
// If repositioning took place, the download tab's would be made
// visible, what's undesired if the download list is not visible!
//
// Unfortunately if the dialog is build up in the InitDialog
// routine, the download window is not visible, but the resizing must
// be done. In such a case if _forceverify==true, the resizing is
// forced, even if the download window is not visible!

void CTransferWnd::VerifyCatTabSize(bool _forceverify)
{
	// ==> Advanced Transfer Window Layout [Stulle] - Stulle
	/*
	if ((m_dwShowListIDC != IDC_DOWNLOADLIST && m_dwShowListIDC != IDC_UPLOADLIST + IDC_DOWNLOADLIST) && (!_forceverify))
	*/
	if (m_dwTopListIDC != IDC_DOWNLOADLIST && !_forceverify)
	// <== Advanced Transfer Window Layout [Stulle] - Stulle
		return;
// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

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

// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
/*
CString CTransferWnd::GetCatTitle(int catid)
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
}
*/
// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

void CTransferWnd::OnBnClickedChangeView()
{
	// ==> Advanced Transfer Window Layout [Stulle] - Stulle
	/*
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
	*/
	switch (m_dwTopListIDC)
	{
		case IDC_DOWNLOADLIST:
			if(thePrefs.GetSplitWindow())
				ShowSplitWindow(false,IDC_UPLOADLIST);
			else
				ShowList(IDC_UPLOADLIST);
			break;
		case IDC_UPLOADLIST:
			if(thePrefs.GetSplitWindow())
				ShowSplitWindow(false,IDC_DOWNLOADCLIENTS);
			else
				ShowList(IDC_DOWNLOADCLIENTS);
			break;
		case IDC_DOWNLOADCLIENTS:
			if (!thePrefs.IsQueueListDisabled()){
				if(thePrefs.GetSplitWindow())
					ShowSplitWindow(false,IDC_QUEUELIST);
				else
					ShowList(IDC_QUEUELIST);
				break;
			}
		case IDC_QUEUELIST:
			if (!thePrefs.IsKnownClientListDisabled()){
				if(thePrefs.GetSplitWindow())
					ShowSplitWindow(false,IDC_CLIENTLIST);
				else
					ShowList(IDC_CLIENTLIST);
				break;
			}
		case IDC_CLIENTLIST:
			if(thePrefs.GetSplitWindow())
				ShowSplitWindow(false,IDC_DOWNLOADLIST);
			else
				ShowList(IDC_DOWNLOADLIST);
			break;
		default:
			ASSERT(0);
			break;
	}
	// <== Advanced Transfer Window Layout [Stulle] - Stulle
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
	// ==> CPU/MEM usage [$ick$/Stulle] - Max
	/*
	rcDown.bottom = rcWnd.bottom - 20;
	*/
	uint8 m_uTemp = (thePrefs.GetSysInfo() ? 35 : 20);
	rcDown.bottom = rcWnd.bottom - m_uTemp;
	// <== CPU/MEM usage [$ick$/Stulle] - Max
	rcDown.top = 28;
	m_wndSplitter.DestroyWindow();
	RemoveAnchor(dwListIDC);
	// ==> Advanced Transfer Window Layout [Stulle] - Stulle
	m_btnWnd1->PressButton(MP_VIEW1_SPLIT_WINDOW,FALSE);
	thePrefs.SetSplitWindow(false);
	// <== Advanced Transfer Window Layout [Stulle] - Stulle
	m_btnWnd2->ShowWindow(SW_HIDE);

	m_dwShowListIDC = dwListIDC;
	m_dwTopListIDC = dwListIDC; // Advanced Transfer Window Layout [Stulle] - Stulle
	uploadlistctrl.ShowWindow((m_dwShowListIDC == IDC_UPLOADLIST) ? SW_SHOW : SW_HIDE);
	queuelistctrl.ShowWindow((m_dwShowListIDC == IDC_QUEUELIST) ? SW_SHOW : SW_HIDE);
	downloadclientsctrl.ShowWindow((m_dwShowListIDC == IDC_DOWNLOADCLIENTS) ? SW_SHOW : SW_HIDE);
	clientlistctrl.ShowWindow((m_dwShowListIDC == IDC_CLIENTLIST) ? SW_SHOW : SW_HIDE);
	downloadlistctrl.ShowWindow((m_dwShowListIDC == IDC_DOWNLOADLIST) ? SW_SHOW : SW_HIDE);
	m_dlTab.ShowWindow((m_dwShowListIDC == IDC_DOWNLOADLIST) ? SW_SHOW : SW_HIDE);
	theApp.emuledlg->transferwnd->ShowToolbar(m_dwShowListIDC == IDC_DOWNLOADLIST);
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
			OnBnClickedQueueRefreshButton(); //Xman update this list
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
			//Xman SortingFix for Morph-Code-Improvement Don't Refresh item if not needed
			clientlistctrl.UpdateAll();
			//Xman end
			break;
		default:
			ASSERT(0);
	}
	AddAnchor(dwListIDC, TOP_LEFT, BOTTOM_RIGHT);
}

// ==> Advanced Transfer Window Layout [Stulle] - Stulle
/*
void CTransferWnd::ShowSplitWindow(bool bReDraw) 
{
	thePrefs.SetTransferWnd1(0);
	m_dlTab.ShowWindow(SW_SHOW);
	if (!bReDraw && m_dwShowListIDC == IDC_DOWNLOADLIST + IDC_UPLOADLIST)
		return;

	m_btnWnd1->CheckButton(MP_VIEW1_SPLIT_WINDOW);
	SetWnd1Icon(w1iDownloadFiles);
*/
void CTransferWnd::ShowSplitWindow(bool bReDraw, uint32 dwListIDC, bool bInitSplitted) 
{
	m_dlTab.ShowWindow((dwListIDC == IDC_DOWNLOADLIST) ? SW_SHOW : SW_HIDE);
	if (!bReDraw && // we don't force a redraw
		m_dwShowListIDC == IDC_DOWNLOADLIST + IDC_UPLOADLIST && // we are already in split mode
		m_dwTopListIDC == dwListIDC) // we did not change the top list
		return;
	// <== Advanced Transfer Window Layout [Stulle] - Stulle

	CRect rcWnd;
	GetWindowRect(rcWnd);
	ScreenToClient(rcWnd);

	LONG splitpos = (thePrefs.GetSplitterbarPosition() * rcWnd.Height()) / 100;

	// do some more magic, don't ask -- just fix it..
	// ==> Advanced Transfer Window Layout [Stulle] - Stulle
	/*
	if (bReDraw || m_dwShowListIDC != 0 && m_dwShowListIDC != IDC_DOWNLOADLIST + IDC_UPLOADLIST)
	*/
	if (bReDraw || // we force redraw of the window
		(m_dwShowListIDC != 0 && m_dwShowListIDC != IDC_DOWNLOADLIST + IDC_UPLOADLIST && !bInitSplitted) || // we switched to splitted but did not initiate
		(m_dwTopListIDC != dwListIDC)) // we switched from one top to another top
	// <== Advanced Transfer Window Layout [Stulle] - Stulle
		splitpos += 10;

	CRect rcDown;
	downloadlistctrl.GetWindowRect(rcDown);
	ScreenToClient(rcDown);
	rcDown.bottom = splitpos - 5; // Magic constant '5'..
	downloadlistctrl.MoveWindow(rcDown);
	uint8 m_uTemp = (thePrefs.GetSysInfo() ? 35 : 20); // CPU/MEM usage [$ick$/Stulle] - Stulle

	// ==> Advanced Transfer Window Layout [Stulle] - Stulle
	m_dwTopListIDC = dwListIDC;
	switch (m_dwTopListIDC)
	{
		case IDC_DOWNLOADLIST:
			//downloadlistctrl.MoveWindow(rcDown);
			thePrefs.SetTransferWnd1(1);
			break;
		case IDC_UPLOADLIST:
		{
			uploadlistctrl.MoveWindow(rcDown);
			thePrefs.SetTransferWnd1(2);
			if(m_uWnd2==wnd2Uploading)
				SwitchUploadList();
		} break;
		case IDC_QUEUELIST:
		{
			queuelistctrl.MoveWindow(rcDown);
			thePrefs.SetTransferWnd1(3);
			if(m_uWnd2==wnd2OnQueue)
				SwitchUploadList();
		} break;
		case IDC_DOWNLOADCLIENTS:
		{
			downloadclientsctrl.MoveWindow(rcDown);
			thePrefs.SetTransferWnd1(4);
			if(m_uWnd2==wnd2Downloading)
				SwitchUploadList();
		} break;
		case IDC_CLIENTLIST:
		{
			clientlistctrl.MoveWindow(rcDown);
			thePrefs.SetTransferWnd1(5);
			if(m_uWnd2==wnd2Clients)
				SwitchUploadList();
		} break;
		default:
			ASSERT(0);
			break;
	}

	if(dwListIDC!=IDC_UPLOADLIST){
	// <== Advanced Transfer Window Layout [Stulle] - Stulle
		uploadlistctrl.GetWindowRect(rcDown);
		ScreenToClient(rcDown);
		rcDown.right = rcWnd.right - 7;
		// ==> CPU/MEM usage [$ick$/Stulle] - Max
		/*
		rcDown.bottom = rcWnd.bottom - 20;
		*/
		rcDown.bottom = rcWnd.bottom - m_uTemp;
		// <== CPU/MEM usage [$ick$/Stulle] - Max
		rcDown.top = splitpos + 20;
		uploadlistctrl.MoveWindow(rcDown);
	// ==> Advanced Transfer Window Layout [Stulle] - Stulle
	}

	if(dwListIDC!=IDC_QUEUELIST){
	// <== Advanced Transfer Window Layout [Stulle] - Stulle
		queuelistctrl.GetWindowRect(rcDown);
		ScreenToClient(rcDown);
		rcDown.right = rcWnd.right - 7;
		// ==> CPU/MEM usage [$ick$/Stulle] - Max
		/*
		rcDown.bottom = rcWnd.bottom - 20;
		*/
		rcDown.bottom = rcWnd.bottom - m_uTemp;
		// <== CPU/MEM usage [$ick$/Stulle] - Max
		rcDown.top = splitpos + 20;
		queuelistctrl.MoveWindow(rcDown);
	// ==> Advanced Transfer Window Layout [Stulle] - Stulle
	}

	if(dwListIDC!=IDC_CLIENTLIST){
	// <== Advanced Transfer Window Layout [Stulle] - Stulle
		clientlistctrl.GetWindowRect(rcDown);
		ScreenToClient(rcDown);
		rcDown.right = rcWnd.right - 7;
		// ==> CPU/MEM usage [$ick$/Stulle] - Max
		/*
		rcDown.bottom = rcWnd.bottom - 20;
		*/
		rcDown.bottom = rcWnd.bottom - m_uTemp;
		// <== CPU/MEM usage [$ick$/Stulle] - Max
		rcDown.top = splitpos + 20;
		clientlistctrl.MoveWindow(rcDown);
	// ==> Advanced Transfer Window Layout [Stulle] - Stulle
	}

	if(dwListIDC!=IDC_DOWNLOADCLIENTS){
	// <== Advanced Transfer Window Layout [Stulle] - Stulle
		downloadclientsctrl.GetWindowRect(rcDown);
		ScreenToClient(rcDown);
		rcDown.right = rcWnd.right - 7;
		// ==> CPU/MEM usage [$ick$/Stulle] - Max
		/*
		rcDown.bottom = rcWnd.bottom - 20;
		*/
		rcDown.bottom = rcWnd.bottom - m_uTemp;
		// <== CPU/MEM usage [$ick$/Stulle] - Max
		rcDown.top = splitpos + 20;
		downloadclientsctrl.MoveWindow(rcDown);
	} // Advanced Transfer Window Layout [Stulle] - Stulle

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
	m_btnWnd2->MoveWindow(WND2_BUTTON_XOFF, rcSpl.top - (WND_SPLITTER_YOFF - 1) - 5, rcBtn2.Width(), WND2_BUTTON_HEIGHT);
	DoResize(0);

	m_dwShowListIDC = IDC_DOWNLOADLIST + IDC_UPLOADLIST;
	// ==> Advanced Transfer Window Layout [Stulle] - Stulle
	/*
	downloadlistctrl.ShowFilesCount();
	m_btnWnd2->ShowWindow(SW_SHOW);
	theApp.emuledlg->transferwnd->ShowToolbar(true);
	*/
	m_btnWnd1->PressButton(MP_VIEW1_SPLIT_WINDOW);
	m_btnWnd2->ShowWindow(SW_SHOW);
	theApp.emuledlg->transferwnd->ShowToolbar(m_dwTopListIDC == IDC_DOWNLOADLIST);
	thePrefs.SetSplitWindow(true);
	// <== Advanced Transfer Window Layout [Stulle] - Stulle

	RemoveAnchor(*m_btnWnd2);
	RemoveAnchor(IDC_DOWNLOADLIST);
	RemoveAnchor(IDC_UPLOADLIST);
	RemoveAnchor(IDC_QUEUELIST);
	RemoveAnchor(IDC_DOWNLOADCLIENTS);
	RemoveAnchor(IDC_CLIENTLIST);

	AddAnchor(*m_btnWnd2, CSize(0, thePrefs.GetSplitterbarPosition()));
	AddAnchor(IDC_DOWNLOADLIST, TOP_LEFT, CSize(100, thePrefs.GetSplitterbarPosition()));
	// ==> Advanced Transfer Window Layout [Stulle] - Stulle
	/*
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
	*/
	downloadlistctrl.ShowWindow(SW_HIDE);
	uploadlistctrl.ShowWindow((m_uWnd2 == wnd2Uploading) ? SW_SHOW : SW_HIDE);
	queuelistctrl.ShowWindow((m_uWnd2 == wnd2OnQueue) ? SW_SHOW : SW_HIDE);
	downloadclientsctrl.ShowWindow((m_uWnd2 == wnd2Downloading) ? SW_SHOW : SW_HIDE);
	clientlistctrl.ShowWindow((m_uWnd2 == wnd2Clients) ? SW_SHOW : SW_HIDE);
	m_btnWnd2->EnableButton(MP_VIEW2_UPLOADING);
	m_btnWnd2->EnableButton(MP_VIEW2_DOWNLOADING);
	m_btnWnd2->EnableButton(MP_VIEW2_ONQUEUE,thePrefs.IsQueueListDisabled()?FALSE:TRUE);
	m_btnWnd2->EnableButton(MP_VIEW2_CLIENTS,thePrefs.IsKnownClientListDisabled()?FALSE:TRUE);
	switch (m_dwTopListIDC)
	{
		case IDC_UPLOADLIST:
			AddAnchor(IDC_UPLOADLIST,TOP_LEFT,CSize(100,thePrefs.GetSplitterbarPosition()));
			AddAnchor(IDC_QUEUELIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
			AddAnchor(IDC_CLIENTLIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
			AddAnchor(IDC_DOWNLOADCLIENTS,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
			uploadlistctrl.ShowWindow(SW_SHOW);
			UpdateListCountTop(wnd2Uploading);
			SetWnd1Icon(w1iUploading);
			m_btnWnd1->CheckButton(MP_VIEW1_UPLOADING);
			m_btnWnd2->EnableButton(MP_VIEW2_UPLOADING,FALSE);
			break;
		case IDC_QUEUELIST:
			AddAnchor(IDC_UPLOADLIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
			AddAnchor(IDC_QUEUELIST,TOP_LEFT,CSize(100,thePrefs.GetSplitterbarPosition()));
			AddAnchor(IDC_CLIENTLIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
			AddAnchor(IDC_DOWNLOADCLIENTS,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
			queuelistctrl.ShowWindow(SW_SHOW);
			UpdateListCountTop(wnd2OnQueue);
			SetWnd1Icon(w1iOnQueue);
			m_btnWnd1->CheckButton(MP_VIEW1_ONQUEUE);
			m_btnWnd2->EnableButton(MP_VIEW2_ONQUEUE,FALSE);
			break;
		case IDC_DOWNLOADCLIENTS:
			AddAnchor(IDC_UPLOADLIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
			AddAnchor(IDC_QUEUELIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
			AddAnchor(IDC_CLIENTLIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
			AddAnchor(IDC_DOWNLOADCLIENTS,TOP_LEFT,CSize(100,thePrefs.GetSplitterbarPosition()));
			downloadclientsctrl.ShowWindow(SW_SHOW);
			UpdateListCountTop(wnd2Downloading);
			SetWnd1Icon(w1iDownloading);
			m_btnWnd1->CheckButton(MP_VIEW1_DOWNLOADING);
			m_btnWnd2->EnableButton(MP_VIEW2_DOWNLOADING,FALSE);
			break;
		case IDC_CLIENTLIST:
			AddAnchor(IDC_UPLOADLIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
			AddAnchor(IDC_QUEUELIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
			AddAnchor(IDC_CLIENTLIST,TOP_LEFT,CSize(100,thePrefs.GetSplitterbarPosition()));
			AddAnchor(IDC_DOWNLOADCLIENTS,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
			clientlistctrl.ShowWindow(SW_SHOW);
			UpdateListCountTop(wnd2Clients);
			SetWnd1Icon(w1iClientsKnown);
			m_btnWnd1->CheckButton(MP_VIEW1_CLIENTS);
			m_btnWnd2->EnableButton(MP_VIEW2_CLIENTS,FALSE);
			break;
		default:
		case IDC_DOWNLOADLIST:
			AddAnchor(IDC_UPLOADLIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
			AddAnchor(IDC_QUEUELIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
			AddAnchor(IDC_CLIENTLIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
			AddAnchor(IDC_DOWNLOADCLIENTS,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
			downloadlistctrl.ShowWindow(SW_SHOW);
			downloadlistctrl.ShowFilesCount();
			SetWnd1Icon(w1iDownloadFiles);
			m_btnWnd1->CheckButton(MP_VIEW1_DOWNLOADS);
			VerifyCatTabSize();
			break;
	}
	GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow((m_uWnd2 == wnd2OnQueue || m_dwTopListIDC == IDC_QUEUELIST) ? SW_SHOW : SW_HIDE);
	// <== Advanced Transfer Window Layout [Stulle] - Stulle

	UpdateListCount(m_uWnd2);
}

void CTransferWnd::OnDisableList()
{
	// ==> Advanced Transfer Window Layout [Stulle] - Stulle
	/*
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
	*/
	bool bSwitchLowerList = false;
	bool bSwitchUpperList = false;
	if (thePrefs.m_bDisableKnownClientList)
	{
		clientlistctrl.DeleteAllItems();
		if (m_uWnd2 == wnd2Clients)
			bSwitchLowerList = true;
		else if (m_dwTopListIDC == IDC_CLIENTLIST)
			bSwitchUpperList = true;
	}
	if (thePrefs.m_bDisableQueueList)
	{
		queuelistctrl.DeleteAllItems();
		if (m_uWnd2 == wnd2OnQueue)
			bSwitchLowerList = true;
		else if (m_dwTopListIDC == IDC_QUEUELIST)
			bSwitchUpperList = true;
	}
	if (bSwitchLowerList)
		SwitchUploadList();
	if(bSwitchUpperList)
		OnBnClickedChangeView();
	// <== Advanced Transfer Window Layout [Stulle] - Stulle
}

void CTransferWnd::OnWnd1BtnDropDown(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	CTitleMenu menu;
	menu.CreatePopupMenu();
	menu.AddMenuTitle(GetResString(IDS_COLOR_W3)); // XP Style Menu [Xanatos] - Stulle
	menu.EnableIcons();

	// ==> Advanced Transfer Window Layout [Stulle] - Stulle
	/*
	menu.AppendMenu(MF_STRING | (m_dwShowListIDC == IDC_DOWNLOADLIST + IDC_UPLOADLIST ? MF_GRAYED : 0), MP_VIEW1_SPLIT_WINDOW, GetResString(IDS_SPLIT_WINDOW), _T("SplitWindow"));
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING | (m_dwShowListIDC == IDC_DOWNLOADLIST ? MF_GRAYED : 0), MP_VIEW1_DOWNLOADS, GetResString(IDS_TW_DOWNLOADS), _T("DownloadFiles"));
	menu.AppendMenu(MF_STRING | (m_dwShowListIDC == IDC_UPLOADLIST ? MF_GRAYED : 0), MP_VIEW1_UPLOADING, GetResString(IDS_UPLOADING), _T("Upload"));
	menu.AppendMenu(MF_STRING | (m_dwShowListIDC == IDC_DOWNLOADCLIENTS ? MF_GRAYED : 0), MP_VIEW1_DOWNLOADING, GetResString(IDS_DOWNLOADING), _T("Download"));
	if (!thePrefs.IsQueueListDisabled())
		menu.AppendMenu(MF_STRING | (m_dwShowListIDC == IDC_QUEUELIST ? MF_GRAYED : 0), MP_VIEW1_ONQUEUE, GetResString(IDS_ONQUEUE), _T("ClientsOnQueue"));
	if (!thePrefs.IsKnownClientListDisabled())
		menu.AppendMenu(MF_STRING | (m_dwShowListIDC == IDC_CLIENTLIST ? MF_GRAYED : 0), MP_VIEW1_CLIENTS, GetResString(IDS_CLIENTLIST), _T("ClientsKnown"));
	*/
	menu.AppendMenu(MF_STRING | (thePrefs.GetSplitWindow() ? MF_CHECKED : MF_UNCHECKED), MP_VIEW1_SPLIT_WINDOW, GetResString(IDS_SPLIT_WINDOW), _T("SplitWindow"));
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING | (m_dwTopListIDC == IDC_DOWNLOADLIST ? MF_GRAYED : 0), MP_VIEW1_DOWNLOADS, GetResString(IDS_TW_DOWNLOADS), _T("DownloadFiles"));
	menu.AppendMenu(MF_STRING | (m_dwTopListIDC == IDC_UPLOADLIST ? MF_GRAYED : 0), MP_VIEW1_UPLOADING, GetResString(IDS_UPLOADING), _T("Upload"));
	menu.AppendMenu(MF_STRING | (m_dwTopListIDC == IDC_DOWNLOADCLIENTS ? MF_GRAYED : 0), MP_VIEW1_DOWNLOADING, GetResString(IDS_DOWNLOADING), _T("Download"));
	if (!thePrefs.IsQueueListDisabled())
		menu.AppendMenu(MF_STRING | (m_dwTopListIDC == IDC_QUEUELIST ? MF_GRAYED : 0), MP_VIEW1_ONQUEUE, GetResString(IDS_ONQUEUE), _T("ClientsOnQueue"));
	if (!thePrefs.IsKnownClientListDisabled())
		menu.AppendMenu(MF_STRING | (m_dwTopListIDC == IDC_CLIENTLIST ? MF_GRAYED : 0), MP_VIEW1_CLIENTS, GetResString(IDS_CLIENTLIST), _T("ClientsKnown"));
	// <== Advanced Transfer Window Layout [Stulle] - Stulle

	CRect rc;
	m_btnWnd1->GetWindowRect(&rc);
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rc.left, rc.bottom, this);
	VERIFY( menu.DestroyMenu() ); // XP Style Menu [Xanatos] - Stulle
}

void CTransferWnd::OnWnd2BtnDropDown(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	CTitleMenu menu;
	menu.CreatePopupMenu();
	menu.AddMenuTitle(GetResString(IDS_COLOR_W3)); // XP Style Menu [Xanatos] - Stulle
	menu.EnableIcons();

	// ==> Advanced Transfer Window Layout [Stulle] - Stulle
	/*
	menu.AppendMenu(MF_STRING | (m_uWnd2 == wnd2Uploading ? MF_GRAYED : 0), MP_VIEW2_UPLOADING, GetResString(IDS_UPLOADING), _T("Upload"));
	menu.AppendMenu(MF_STRING | (m_uWnd2 == wnd2Downloading ? MF_GRAYED : 0), MP_VIEW2_DOWNLOADING, GetResString(IDS_DOWNLOADING), _T("Download"));
	if (!thePrefs.IsQueueListDisabled())
		menu.AppendMenu(MF_STRING | (m_uWnd2 == wnd2OnQueue ? MF_GRAYED : 0), MP_VIEW2_ONQUEUE, GetResString(IDS_ONQUEUE), _T("ClientsOnQueue"));
	if (!thePrefs.IsKnownClientListDisabled())
		menu.AppendMenu(MF_STRING | (m_uWnd2 == wnd2Clients ? MF_GRAYED : 0), MP_VIEW2_CLIENTS, GetResString(IDS_CLIENTLIST), _T("ClientsKnown"));
	*/
	menu.AppendMenu(MF_STRING | ((m_uWnd2 == wnd2Uploading || m_dwTopListIDC == IDC_UPLOADLIST) ? MF_GRAYED : 0), MP_VIEW2_UPLOADING, GetResString(IDS_UPLOADING), _T("Upload"));
	menu.AppendMenu(MF_STRING | ((m_uWnd2 == wnd2Downloading || m_dwTopListIDC == IDC_DOWNLOADCLIENTS) ? MF_GRAYED : 0), MP_VIEW2_DOWNLOADING, GetResString(IDS_DOWNLOADING), _T("Download"));
	if (!thePrefs.IsQueueListDisabled())
		menu.AppendMenu(MF_STRING | ((m_uWnd2 == wnd2OnQueue || m_dwTopListIDC == IDC_QUEUELIST) ? MF_GRAYED : 0), MP_VIEW2_ONQUEUE, GetResString(IDS_ONQUEUE), _T("ClientsOnQueue"));
	if (!thePrefs.IsKnownClientListDisabled())
		menu.AppendMenu(MF_STRING | ((m_uWnd2 == wnd2Clients || m_dwTopListIDC == IDC_CLIENTLIST) ? MF_GRAYED : 0), MP_VIEW2_CLIENTS, GetResString(IDS_CLIENTLIST), _T("ClientsKnown"));
	// <== Advanced Transfer Window Layout [Stulle] - Stulle

	CRect rc;
	m_btnWnd2->GetWindowRect(&rc);
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rc.left, rc.bottom, this);
	VERIFY( menu.DestroyMenu() ); // XP Style Menu [Xanatos] - Stulle
}

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
		//  - WinXP && Win2003 with COMCTL32 v5.82 //zz_fly, DolphinX
		// The value returned by 'GetMaxSize' is just couple of pixels too small so that the 
		// last toolbar button is nearly not visible at all.
		// So, to circumvent such problems, the toolbar control should be created right with
		// the needed size so that we do not really need to call the 'GetMaxSize' function.
		// Although it would be better to call it to adapt for system metrics basically.
		if (theApp.m_ullComCtrlVer > MAKEDLLVERULL(5,82,0,0)) //zz_fly :: in the case of disable theme service in xp and 2k3
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
		//  - WinXP && Win2003 with COMCTL32 v5.82 //zz_fly, DolphinX
		// The value returned by 'GetMaxSize' is just couple of pixels too small so that the 
		// last toolbar button is nearly not visible at all.
		// So, to circumvent such problems, the toolbar control should be created right with
		// the needed size so that we do not really need to call the 'GetMaxSize' function.
		// Although it would be better to call it to adapt for system metrics basically.
		if (theApp.m_ullComCtrlVer > MAKEDLLVERULL(5,82,0,0)) //zz_fly :: in the case of disable theme service in xp and 2k3
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

	if (bResetLists)
	{
		LocalizeToolbars();
		// ==> Advanced Transfer Window Layout [Stulle] - Stulle
		/*
		ShowSplitWindow(true);
		*/
		bool bWasSplit = thePrefs.GetSplitWindow();
		ShowSplitWindow(true,m_dwTopListIDC);
		// <== Advanced Transfer Window Layout [Stulle] - Stulle
		VerifyCatTabSize();
		ShowWnd2(m_uWnd2);
		// ==> Advanced Transfer Window Layout [Stulle] - Stulle
		if(!bWasSplit)
			ShowList(m_dwTopListIDC);
		// <== Advanced Transfer Window Layout [Stulle] - Stulle
	}
}

BOOL CTransferWnd::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	theApp.ShowHelp(eMule_FAQ_GUI_Transfers);
	return TRUE;
}

HBRUSH CTransferWnd::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
// ==> Design Settings [eWombat/Stulle] - Max
/*
	HBRUSH hbr = theApp.emuledlg->GetCtlColor(pDC, pWnd, nCtlColor);
	if (hbr)
		return hbr;
	return __super::OnCtlColor(pDC, pWnd, nCtlColor);
}
*/
	HBRUSH hbr = theApp.emuledlg->GetCtlColor(pDC, pWnd, nCtlColor);
	if (hbr)
		return hbr;
	hbr = __super::OnCtlColor(pDC, pWnd, nCtlColor);

	switch(nCtlColor)
	{
	case CTLCOLOR_EDIT:
		break;
	default:
		pDC->SetBkMode(TRANSPARENT);
	case CTLCOLOR_DLG:
		hbr = (HBRUSH) m_brMyBrush.GetSafeHandle();
		break;
	}

	return hbr;
}
// <== Design Settings [eWombat/Stulle] - Max

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
		// ==> Advanced Transfer Window Layout [Stulle] - Stulle
		/*
		if (m_dwShowListIDC == IDC_DOWNLOADLIST + IDC_UPLOADLIST)
			this->ShowSplitWindow(true);
		*/
		if (m_dwShowListIDC == IDC_DOWNLOADLIST + IDC_UPLOADLIST)
			this->ShowSplitWindow(true,m_dwTopListIDC);
		// <== Advanced Transfer Window Layout [Stulle] - Stulle
		else
			ShowList(m_dwShowListIDC);
		QueueListResize(thePrefs.GetSysInfo()?15:0); // CPU/MEM usage [$ick$/Stulle] - Stulle
	}

	if (m_wndSplitter)
	{
		if (rcWnd.Height() > 0)
		{
			CRect rcDown;
			// ==> Advanced Transfer Window Layout [Stulle] - Stulle
			/*
			downloadlistctrl.GetWindowRect(rcDown);
			*/
			switch (m_dwTopListIDC)
			{
				case IDC_UPLOADLIST:
					uploadlistctrl.GetWindowRect(rcDown);
					break;
				case IDC_QUEUELIST:
					queuelistctrl.GetWindowRect(rcDown);
					break;
				case IDC_DOWNLOADCLIENTS:
					downloadclientsctrl.GetWindowRect(rcDown);
					break;
				case IDC_CLIENTLIST:
					clientlistctrl.GetWindowRect(rcDown);
					break;
				default:
				case IDC_DOWNLOADLIST:
					downloadlistctrl.GetWindowRect(rcDown);
					break;
			}
			// <== Advanced Transfer Window Layout [Stulle] - Stulle
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

// ==> CPU/MEM usage [$ick$/Stulle] - Max 
void CTransferWnd::ShowRessources()
{
	if(thePrefs.GetSysInfo() == false)
		return;

	CString buffer = NULL;

	buffer.Format(_T("%3d%%"),theApp.sysinfo->GetCpuUsage());
	if(thePrefs.GetSysInfoGlobal())
		buffer.AppendFormat(_T(" (%3d%%)"),theApp.sysinfo->GetGlobalCpuUsage());
//	_stprintf(buffer,_T("%3d%% (%3d%%)"),theApp.sysinfo->GetCpuUsage(),theApp.sysinfo->GetGlobalCpuUsage());
	SetDlgItemText(IDC_CPU, _T("CPU :"));
	this->GetDlgItem(IDC_CPUCOUNT)->SetWindowText(buffer);

	buffer.Format(_T("%s"),CastItoXBytes(theApp.sysinfo->GetMemoryUsage()));
	if(thePrefs.GetSysInfoGlobal())
		buffer.AppendFormat(_T(" (%s)"),CastItoXBytes(theApp.sysinfo->GetGlobalMemoryUsage()));
//	_stprintf(buffer,_T("%s (%s)"),CastItoXBytes(theApp.sysinfo->GetMemoryUsage()),CastItoXBytes(theApp.sysinfo->GetGlobalMemoryUsage()));
	SetDlgItemText(IDC_MEM, _T("RAM :"));
	this->GetDlgItem(IDC_MEMCOUNT)->SetWindowText(buffer);

	return;
}

void CTransferWnd::EnableSysInfo(bool bEnable)
{
	GetDlgItem(IDC_CPU)->ShowWindow((bEnable ? SW_SHOW : SW_HIDE));
	GetDlgItem(IDC_CPUCOUNT)->ShowWindow((bEnable ? SW_SHOW : SW_HIDE));
	GetDlgItem(IDC_MEM)->ShowWindow((bEnable ? SW_SHOW : SW_HIDE));
	GetDlgItem(IDC_MEMCOUNT)->ShowWindow((bEnable ? SW_SHOW : SW_HIDE));

	uint8 value = bEnable?15:0;

	CRect rcDown,rcW;
	GetWindowRect(rcW);
	ScreenToClient(rcW);

	if(m_dwTopListIDC!=IDC_UPLOADLIST){ // Advanced Transfer Window Layout [Stulle] - Stulle
		GetDlgItem(IDC_UPLOADLIST)->GetWindowRect(rcDown);
		ScreenToClient(rcDown);
		rcDown.bottom=rcW.bottom-20-value;
		uploadlistctrl.MoveWindow(rcDown);
	// ==> Advanced Transfer Window Layout [Stulle] - Stulle
	}

	if(m_dwTopListIDC!=IDC_DOWNLOADCLIENTS){
	// <== Advanced Transfer Window Layout [Stulle] - Stulle
		GetDlgItem(IDC_DOWNLOADCLIENTS)->GetWindowRect(rcDown);
		ScreenToClient(rcDown);
		rcDown.bottom=rcW.bottom-20-value;
		downloadclientsctrl.MoveWindow(rcDown);
	// ==> Advanced Transfer Window Layout [Stulle] - Stulle
	}

	if(m_dwTopListIDC!=IDC_QUEUELIST){
	// <== Advanced Transfer Window Layout [Stulle] - Stulle
		GetDlgItem(IDC_QUEUELIST)->GetWindowRect(rcDown);
		ScreenToClient(rcDown);
		rcDown.bottom=rcW.bottom-20-value;
		queuelistctrl.MoveWindow(rcDown);
	// ==> Advanced Transfer Window Layout [Stulle] - Stulle
	}

	if(m_dwTopListIDC!=IDC_CLIENTLIST){
	// <== Advanced Transfer Window Layout [Stulle] - Stulle
		GetDlgItem(IDC_CLIENTLIST)->GetWindowRect(rcDown);
		ScreenToClient(rcDown);
		rcDown.bottom=rcW.bottom-20-value;
		clientlistctrl.MoveWindow(rcDown);
	} // Advanced Transfer Window Layout [Stulle] - Stulle

	QueueListResize(value);
	Localize();

	return;
}

void CTransferWnd::QueueListResize(uint8 value)
{
	CRect rcDown,rcW;
	GetWindowRect(rcW);
	ScreenToClient(rcW);

	RemoveAnchor(IDC_QUEUE_REFRESH_BUTTON);
    RemoveAnchor(IDC_QUEUE); //Commander - Added: ClientQueueProgressBar
	RemoveAnchor(IDC_QUEUE2); //Commander - Added: ClientQueueProgressBar
	RemoveAnchor(IDC_QUEUECOUNT_LABEL);
	RemoveAnchor(IDC_QUEUECOUNT);

	GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->GetWindowRect(rcDown);
	ScreenToClient(rcDown);
	rcDown.top=rcW.bottom-19-value;
	rcDown.bottom=rcW.bottom-3-value;
	GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->MoveWindow(rcDown);

	GetDlgItem(IDC_QUEUE)->GetWindowRect(rcDown);
	ScreenToClient(rcDown);
	rcDown.top=rcW.bottom-17-value;
	rcDown.bottom=rcW.bottom-5-value;
	GetDlgItem(IDC_QUEUE)->MoveWindow(rcDown);

	GetDlgItem(IDC_QUEUE2)->GetWindowRect(rcDown);
	ScreenToClient(rcDown);
	rcDown.top=rcW.bottom-17-value;
	rcDown.bottom=rcW.bottom-5-value;
	GetDlgItem(IDC_QUEUE2)->MoveWindow(rcDown);

	GetDlgItem(IDC_QUEUECOUNT_LABEL)->GetWindowRect(rcDown);
	ScreenToClient(rcDown);
	rcDown.top=rcW.bottom-18-value;
	rcDown.bottom=rcW.bottom-5-value;
	GetDlgItem(IDC_QUEUECOUNT_LABEL)->MoveWindow(rcDown);

	GetDlgItem(IDC_QUEUECOUNT)->GetWindowRect(rcDown);
	ScreenToClient(rcDown);
	rcDown.top=rcW.bottom-18-value;
	rcDown.bottom=rcW.bottom-5-value;
	GetDlgItem(IDC_QUEUECOUNT)->MoveWindow(rcDown);

	AddAnchor(IDC_QUEUECOUNT,BOTTOM_LEFT);
	AddAnchor(IDC_QUEUECOUNT_LABEL,BOTTOM_LEFT);
    AddAnchor(IDC_QUEUE, BOTTOM_LEFT, CSize(73,100)); //Commander - Added: ClientQueueProgressBar
	AddAnchor(IDC_QUEUE2, CSize(73,100), BOTTOM_RIGHT); //Commander - Added: ClientQueueProgressBar
	AddAnchor(IDC_QUEUE_REFRESH_BUTTON, BOTTOM_RIGHT);
}
// <== CPU/MEM usage [$ick$/Stulle] - Max 

// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
void CTransferWnd::CreateCategoryMenus()
{
	if (m_mnuCatPriority) VERIFY( m_mnuCatPriority.DestroyMenu() );
	if (m_mnuCatDlMode) VERIFY( m_mnuCatDlMode.DestroyMenu() );
	if (m_mnuCatViewFilter) VERIFY( m_mnuCatViewFilter.DestroyMenu() );
	if (m_mnuCategory) VERIFY( m_mnuCategory.DestroyMenu() );
	
	// Create sub-menus first...

	// Priority Menu
	m_mnuCatPriority.CreateMenu();
	m_mnuCatPriority.AddMenuTitle(GetResString(IDS_PRIORITY),false,false);
	m_mnuCatPriority.AppendMenu(MF_STRING, MP_PRIOLOW, GetResString(IDS_PRIOLOW));
	m_mnuCatPriority.AppendMenu(MF_STRING, MP_PRIONORMAL, GetResString(IDS_PRIONORMAL));
	m_mnuCatPriority.AppendMenu(MF_STRING, MP_PRIOHIGH, GetResString(IDS_PRIOHIGH));
//	m_mnuCatPriority.AppendMenu(MF_STRING, MP_DOWNLOAD_ALPHABETICAL, GetResString(IDS_DOWNLOAD_ALPHABETICAL));

	// Dl Mode Menu
	m_mnuCatDlMode.CreateMenu();
	m_mnuCatDlMode.AddMenuTitle(GetResString(IDS_DL_MODE),false,false);
	m_mnuCatDlMode.AppendMenu(MF_STRING, MP_CAT_DL_MODE, GetResString(IDS_DEFAULT));
	m_mnuCatDlMode.AppendMenu(MF_STRING, MP_CAT_DL_MODE+1, GetResString(IDS_DOWNLOAD_ALPHABETICAL));
	m_mnuCatDlMode.AppendMenu(MF_STRING, MP_CAT_DL_MODE+2, GetResString(IDS_LP));

	// View Filter Menu
	m_mnuCatViewFilter.CreateMenu();
	m_mnuCatViewFilter.AddMenuTitle(GetResString(IDS_CHANGECATVIEW));
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0, GetResString(IDS_ALL) );
	//m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+1, GetResString(IDS_ALLOTHERS) );
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+2, GetResString(IDS_CAT_THISCAT) );
		
	m_mnuCatViewFilter.AppendMenu(MF_SEPARATOR);
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+3, GetResString(IDS_COMPLETE));
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+4, GetResString(IDS_COMPLETING));
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+5, GetResString(IDS_DOWNLOADING));
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+6, GetResString(IDS_WAITING));
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+7, GetResString(IDS_PAUSED));
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+8, GetResString(IDS_STOPPED));
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+9, GetResString(IDS_HASHING));
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
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+15, GetResString(IDS_CAT_SUSPENDFILTERS));
	m_mnuCatViewFilter.AppendMenu(MF_STRING, MP_CAT_SET0+17, GetResString(IDS_COL_MORECOLORS));

	// Create the main menu...
	m_mnuCategory.CreatePopupMenu();
	m_mnuCategory.AddMenuTitle(GetResString(IDS_CAT),true);

	m_mnuCategory.AppendMenu(MF_STRING | MF_POPUP, (UINT_PTR)m_mnuCatViewFilter.m_hMenu, GetResString(IDS_CHANGECATVIEW),_T("SEARCHPARAMS"));
	m_mnuCategory.AppendMenu(MF_SEPARATOR);
	m_mnuCategory.AppendMenu(MF_STRING, MP_CAT_ADD, GetResString(IDS_CAT_ADD),_T("CATADD"));
	m_mnuCategory.AppendMenu(MF_STRING, MP_CAT_EDIT, GetResString(IDS_CAT_EDIT),_T("CATEDIT"));
	m_mnuCategory.AppendMenu(MF_STRING, MP_CAT_MERGE, GetResString(IDS_CAT_MERGE),_T("CATMERGE"));
	m_mnuCategory.AppendMenu(MF_STRING, MP_CAT_REMOVE, GetResString(IDS_CAT_REMOVE),_T("CATREMOVE"));
	m_mnuCategory.AppendMenu(MF_SEPARATOR);
	m_mnuCategory.AppendMenu(MF_STRING | MF_POPUP, (UINT_PTR)m_mnuCatDlMode.m_hMenu, GetResString(IDS_DL_MODE),_T("DLMODE"));
	m_mnuCategory.AppendMenu(MF_SEPARATOR);
	m_mnuCategory.AppendMenu(MF_STRING,MP_HM_OPENINC, GetResString(IDS_OPENINC), _T("Incoming") );
	m_mnuCategory.AppendMenu(MF_SEPARATOR);
	m_mnuCategory.AppendMenu(MF_STRING | MF_POPUP, (UINT_PTR)m_mnuCatPriority.m_hMenu, GetResString(IDS_PRIORITY), _T("FILEPRIORITY"));
	m_mnuCategory.AppendMenu(MF_STRING, MP_CANCEL, GetResString(IDS_MAIN_BTN_CANCEL), _T("DELETE"));
	m_mnuCategory.AppendMenu(MF_STRING, MP_STOP, GetResString(IDS_DL_STOP), _T("STOP"));
	m_mnuCategory.AppendMenu(MF_STRING, MP_PAUSE, GetResString(IDS_DL_PAUSE), _T("PAUSE"));
	m_mnuCategory.AppendMenu(MF_STRING, MP_RESUME, GetResString(IDS_DL_RESUME), _T("RESUME"));
	m_mnuCategory.AppendMenu(MF_SEPARATOR);
	m_mnuCategory.AppendMenu(MF_STRING, MP_CAT_STOPLAST, GetResString(IDS_CAT_STOPLAST), _T("CATSTOPLAST"));	
	m_mnuCategory.AppendMenu(MF_STRING, MP_CAT_PAUSELAST, GetResString(IDS_CAT_PAUSELAST), _T("CATPAUSELAST"));	
	m_mnuCategory.AppendMenu(MF_STRING, MP_RESUMENEXT, GetResString(IDS_CAT_RESUMENEXT), _T("CATRESUMENEXT"));    	

}
// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

// ==> XP Style Menu [Xanatos] - Stulle
void CTransferWnd::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	HMENU hMenu = AfxGetThreadState()->m_hTrackingMenu;
	if(CMenu *pMenu = CMenu::FromHandle(hMenu))
		pMenu->MeasureItem(lpMeasureItemStruct);
	
	CResizableFormView::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}

LRESULT CTransferWnd::OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu) 
{
	if (pMenu->IsKindOf(RUNTIME_CLASS(CTitleMenu)) )
		return CTitleMenu::OnMenuChar(nChar, nFlags, pMenu);

	return CResizableFormView::OnMenuChar(nChar, nFlags, pMenu);
}
// <== XP Style Menu [Xanatos] - Stulle

// ==> Design Settings [eWombat/Stulle] - Stulle
void CTransferWnd::SetBackgroundColor(int nStyle)
{
	COLORREF crTempColor = thePrefs.GetStyleBackColor(background_styles, nStyle);

	if(crTempColor == CLR_DEFAULT)
		crTempColor = thePrefs.GetStyleBackColor(background_styles, style_b_default);

	if(crTempColor == CLR_DEFAULT)
		crTempColor = COLORREF(RGB(255,255,255));

	switch(nStyle)
	{
		case style_b_clientlist:
			clientlistctrl.SetBkColor(crTempColor);
			break;
		case style_b_dlclientlist:
			downloadclientsctrl.SetBkColor(crTempColor);
			break;
		case style_b_queuelist:
			queuelistctrl.SetBkColor(crTempColor);
			break;
		case style_b_uploadlist:
			uploadlistctrl.SetBkColor(crTempColor);
			break;
		case style_b_downloadlist:
			downloadlistctrl.SetBkColor(crTempColor);
			break;
		default:
			break;
	}
}
// <== Design Settings [eWombat/Stulle] - Stulle

// ==> Design Settings [eWombat/Stulle] - Max
void CTransferWnd::OnBackcolor() 
{
	COLORREF crTempColor = thePrefs.GetStyleBackColor(window_styles, style_w_transfer);

	if(crTempColor == CLR_DEFAULT)
		crTempColor = thePrefs.GetStyleBackColor(window_styles, style_w_default);
		
	m_brMyBrush.DeleteObject();

	// ==> Visual Studio 2010 Compatibility [Stulle/Avi-3k/ied] - Stulle
#if _MSC_VER<1600
	if(crTempColor != CLR_DEFAULT)
		m_brMyBrush.CreateSolidBrush(crTempColor);
	else
		m_brMyBrush.CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
#else
	if(crTempColor != CLR_DEFAULT)
	{
		m_brMyBrush.CreateSolidBrush(crTempColor);
		m_Refresh.SetBackgroundColor(crTempColor);
	}
	else
	{
		m_brMyBrush.CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
		m_Refresh.SetBackgroundColor(COLORREF(-1));
	}
#endif
	// <== Visual Studio 2010 Compatibility [Stulle/Avi-3k/ied] - Stulle
}
// <== Design Settings [eWombat/Stulle] - Max

// ==> Advanced Transfer Window Layout [Stulle] - Stulle
void CTransferWnd::UpdateListCountTop(EWnd2 listindex)
{
	switch(listindex)
	{
		case wnd2Uploading:
		{
			CString strBuffer;
        	uint32 itemCount = uploadlistctrl.GetItemCount();
			//Xman Xtreme Upload
			/*
        	uint32 activeCount = theApp.uploadqueue->GetActiveUploadsCount();
			if (activeCount >= itemCount)
				strBuffer.Format(_T(" (%i)"), itemCount);
			else
				strBuffer.Format(_T(" (%i/%i)"), activeCount, itemCount);
			//Upload Speed Sens
			strBuffer.AppendFormat(_T(" %i Max"), theApp.uploadqueue->GetActiveUploadsCountLongPerspective());
			*/
			// ==> Mephisto Upload - Mephisto
			/*
			uint32 activeCount = theApp.uploadBandwidthThrottler->GetNumberOfFullyActivatedSlots();
			//Xman upload health
			//Xman count block/success send
			float health = theApp.uploadBandwidthThrottler->GetAvgHealth();
			float avgblocks = theApp.uploadBandwidthThrottler->GetAvgBlockRatio();
			if(thePrefs.ShowBlockRatio())
				strBuffer.Format(_T(" (%i/%i) %0.0f%% %0.0f%%"), activeCount, itemCount, health, avgblocks);
			else
				strBuffer.Format(_T(" (%i/%i) %0.0f%%"), activeCount, itemCount, health);
			*/
			uint32 activeCount = theApp.uploadBandwidthThrottler->GetActiveSlotCount();
			//Xman upload health
			//Xman count block/success send
			float health = theApp.uploadBandwidthThrottler->GetAvgHealth();
			float avgblocks = theApp.uploadBandwidthThrottler->GetAvgBlockRatio();
			CString strReached = (theApp.uploadBandwidthThrottler->GetBandwithReached()?_T("R"):_T("NR"));
			if(thePrefs.ShowBlockRatio())
				strBuffer.Format(_T(" (%i/%i) %0.0f%% %0.0f%%, %s"), activeCount, itemCount, health, avgblocks, strReached);
			else
				strBuffer.Format(_T(" (%i/%i) %0.0f%%, %s"), activeCount, itemCount, health, strReached);
			// <== Mephisto Upload - Mephisto
			//Xman end
			m_btnWnd1->SetWindowText(GetResString(IDS_UPLOADING) + strBuffer);
        } break;
		case wnd2OnQueue:
		{
			CString strBuffer;
			strBuffer.Format(_T(" (%i)"), queuelistctrl.GetItemCount());
			m_btnWnd1->SetWindowText(GetResString(IDS_ONQUEUE) + strBuffer);
		} break;
		case wnd2Clients:
		{
			CString strBuffer;
			strBuffer.Format(_T(" (%i)"), clientlistctrl.GetItemCount());
			m_btnWnd1->SetWindowText(GetResString(IDS_CLIENTLIST) + strBuffer);
    	} break;
		case wnd2Downloading:
		{
			CString strBuffer;
			strBuffer.Format(_T(" (%i)"), downloadclientsctrl.GetItemCount());
			m_btnWnd1->SetWindowText(GetResString(IDS_DOWNLOADING) + strBuffer);
		} break;
		default:
			ASSERT(0);
			break;
	}
}
// <== Advanced Transfer Window Layout [Stulle] - Stulle