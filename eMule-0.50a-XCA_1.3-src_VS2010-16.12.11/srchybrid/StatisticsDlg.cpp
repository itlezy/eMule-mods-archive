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
#include "StatisticsDlg.h"
#include "UploadQueue.h"
#include "Statistics.h"
#include "emuledlg.h"
#include "OtherFunctions.h"
#include "WebServer.h"
#include "DownloadQueue.h"
#include "ClientList.h"
#include "ListenSocket.h"
#include "ServerList.h"
#include "SharedFileList.h"
#include "UpDownClient.h"
#include "UploadBandwidthThrottler.h" //Xman Xtreme upload
#include "BandWidthControl.h" // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
#include <math.h> //Xman 

#include "UserMsgs.h"
#include "Kademlia/Kademlia/kademlia.h"
#include "Kademlia/Kademlia/Prefs.h"
#include "kademlia/kademlia/UDPFirewallTester.h"
#include "ipfilter.h"// X: [SFH] - [Show IP Filter Hits]
#include "VolumeInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
extern _CRT_ALLOC_HOOK g_pfnPrevCrtAllocHook;
#endif


// CStatisticsDlg dialog
static const LPCTSTR softString[DOWN_SOFT_COUNT]={
	_T("eMule: %s (%1.1f%%)"),
	_T("eD Hybrid: %s (%1.1f%%)"),
	_T("eDonkey: %s (%1.1f%%)"),
	_T("aMule: %s (%1.1f%%)"),
	_T("MLdonkey: %s (%1.1f%%)"),
	_T("Shareaza: %s (%1.1f%%)"),
	_T("eM Compat: %s (%1.1f%%)"),
	_T("URL: %s (%1.1f%%)")
} ;
static const UINT portStrID[PORT_COUNT]={IDS_STATS_PRTDEF,IDS_STATS_PRTOTHER,IDS_STATS_PRTPEERCACHE};

IMPLEMENT_DYNAMIC(CStatisticsDlg, CDialog)

BEGIN_MESSAGE_MAP(CStatisticsDlg, CResizableDialog)
	ON_WM_SHOWWINDOW()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BNMENU, OnMenuButtonClicked)	
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CTLCOLOR()
	ON_STN_DBLCLK(IDC_SCOPE_D, OnStnDblclickScope)
	ON_STN_DBLCLK(IDC_SCOPE_U, OnStnDblclickScope)
	ON_STN_DBLCLK(IDC_STATSSCOPE, OnStnDblclickScope)
	ON_MESSAGE(UM_OSCOPEPOSITION, OnOscopePositionMsg)
//TK4 MOD +
	ON_WM_CONTEXTMENU() 
	ON_MESSAGE(UM_RESTORE_SCOPES,restoreScopes)
//TK4 MOD -
END_MESSAGE_MAP()

//Xman
// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
CStatisticsDlg::CStatisticsDlg(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CStatisticsDlg::IDD, pParent),
  m_DownloadOMeter(NUMBEROFLINES+1), 
  m_UploadOMeter(NUMBEROFLINES+1), //Xman no friendup!
  m_Statistics(3), //Xman no full aktivated slots
  m_intervalGraph(0),
  m_intervalStat(0)

{
	m_TimeToolTips = NULL;
}
//Xman end

CStatisticsDlg::~CStatisticsDlg()
{
	delete m_TimeToolTips;

#ifdef _DEBUG
	POSITION pos = blockFiles.GetStartPosition();
	while (pos != NULL) 
	{
		const unsigned char* fileName;
		HTREEITEM* pTag;
		blockFiles.GetNextAssoc(pos, fileName, pTag);
		delete pTag;
	}
#endif
}

void CStatisticsDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATTREE, stattree);
}

void CStatisticsDlg::OnSysColorChange()
{
	CResizableDialog::OnSysColorChange();
	SetAllIcons();
}

void CStatisticsDlg::SetAllIcons()
{
	InitWindowStyles(this);

	CImageList iml;
	iml.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);
	iml.Add(CTempIconLoader(_T("StatsGeneric")));			// Dots & Arrow (Default icon for stats)
	iml.Add(CTempIconLoader(_T("TransferUpDown")));			// Transfer
	iml.Add(CTempIconLoader(_T("Connection")));				// Connection
	iml.Add(CTempIconLoader(_T("StatsClients")));			// Clients
	iml.Add(CTempIconLoader(_T("Server")));					// Server
	iml.Add(CTempIconLoader(_T("SharedFiles")));			// Shared Files
	iml.Add(CTempIconLoader(_T("Upload")));					// Transfer > Upload
	iml.Add(CTempIconLoader(_T("Download")));				// Transfer > Download
	iml.Add(CTempIconLoader(_T("StatsDetail")));			// Session Sections
	iml.Add(CTempIconLoader(_T("StatsCumulative")));		// Cumulative Sections
	iml.Add(CTempIconLoader(_T("StatsRecords")));			// Records
	iml.Add(CTempIconLoader(_T("TransferUpDown")));			// Connection > General
	iml.Add(CTempIconLoader(_T("StatsTime")));				// Time Section
	iml.Add(CTempIconLoader(_T("StatsProjected")));			// Time > Averages and Projections
	iml.Add(CTempIconLoader(_T("StatsDay")));				// Time > Averages and Projections > Daily
	iml.Add(CTempIconLoader(_T("StatsMonth")));				// Time > Averages and Projections > Monthly
	iml.Add(CTempIconLoader(_T("StatsYear")));				// Time > Averages and Projections > Yearly
	iml.Add(CTempIconLoader(_T("HardDisk")));				// Diskspace
#ifdef CLIENTANALYZER
	iml.Add(CTempIconLoader(L"BADGUY"));						// 18
#endif
	stattree.SetImageList(&iml, TVSIL_NORMAL);
	imagelistStatTree.DeleteImageList();
	imagelistStatTree.Attach(iml.Detach());

	COLORREF crBk = GetSysColor(COLOR_WINDOW);
	COLORREF crFg = GetSysColor(COLOR_WINDOWTEXT);

	theApp.LoadSkinColorAlt(_T("StatisticsTvBk"), _T("DefLvBk"), crBk);
	theApp.LoadSkinColorAlt(_T("StatisticsTvFg"), _T("DefLvFg"), crFg);

	stattree.SetBkColor(crBk);
	stattree.SetTextColor(crFg);
	// can't use 'TVM_SETLINECOLOR' because the color may not match that one used in "StatsGeneric" item image.
	//stattree.SendMessage(TVM_SETLINECOLOR, 0, (LPARAM)crFg);
}

BOOL CStatisticsDlg::OnInitDialog()
{
	CResizableDialog::OnInitDialog();
	EnableWindow(FALSE);
	SetAllIcons();
//TK4 MOD +
       /*
	m_bTreepaneHidden=false;
        */
	paneMaximized    = false;
	statsTreeHidden  = false;
	scopesViewHidden = false;
//TK4 MOD -

	if (theApp.m_fontSymbol.m_hObject)
	{
		GetDlgItem(IDC_BNMENU)->SetFont(&theApp.m_fontSymbol);
		SetDlgItemText(IDC_BNMENU,_T("6")); // show a down-arrow
	}

	// Win98: Explicitly set to Unicode to receive Unicode notifications.
	//stattree.SendMessage(CCM_SETUNICODEFORMAT, TRUE);
	if (thePrefs.GetUseSystemFontForMainControls())
		stattree.SendMessage(WM_SETFONT, NULL, FALSE);

	// Setup download-scope
	CRect rcDown;
	GetDlgItem(IDC_SCOPE_D)->GetWindowRect(rcDown);
	GetDlgItem(IDC_SCOPE_D)->DestroyWindow();
	ScreenToClient(rcDown);
	m_DownloadOMeter.Create(WS_VISIBLE | WS_CHILD, rcDown, this, IDC_SCOPE_D);
	SetARange(true, (int)thePrefs.GetMaxGraphDownloadRate()); //Xman
	m_DownloadOMeter.SetYUnits(GetResString(IDS_KBYTESPERSEC));
	
	// Setup upload-scope
	CRect rcUp;
	GetDlgItem(IDC_SCOPE_U)->GetWindowRect(rcUp);
	GetDlgItem(IDC_SCOPE_U)->DestroyWindow();
	ScreenToClient(rcUp);
	// compensate rounding errors due to dialog units, make each of the 3 panes with same height
	rcUp.top = rcDown.bottom + 4;
	rcUp.bottom = rcUp.top + rcDown.Height();
	m_UploadOMeter.Create(WS_VISIBLE | WS_CHILD, rcUp, this, IDC_SCOPE_U);
	SetARange(false, (int)thePrefs.GetMaxGraphUploadRate()); //Xman
	m_UploadOMeter.SetYUnits(GetResString(IDS_KBYTESPERSEC));
	
	// Setup additional graph-scope
	CRect rcConn;
	GetDlgItem(IDC_STATSSCOPE)->GetWindowRect(rcConn);
	GetDlgItem(IDC_STATSSCOPE)->DestroyWindow();
	ScreenToClient(rcConn);
	// compensate rounding errors due to dialog units, make each of the 3 panes with same height
	rcConn.top = rcUp.bottom + 4;
	rcConn.bottom = rcConn.top + rcDown.Height();
	m_Statistics.Create(WS_VISIBLE | WS_CHILD, rcConn, this, IDC_STATSSCOPE);
	m_Statistics.SetRanges(0, thePrefs.GetStatsMax());
	m_Statistics.autofitYscale = false;
	// Set the trend ratio of the Active Connections trend in the Connection Statistics scope.
	m_Statistics.SetTrendRatio(0, thePrefs.GetStatsConnectionsGraphRatio());

	m_Statistics.SetYUnits(_T(""));
	m_Statistics.SetXUnits(GetResString(IDS_TIME));

	//RepaintMeters();
	m_Statistics.SetBackgroundColor(thePrefs.GetStatsColor(0)) ;
	m_Statistics.SetGridColor(thePrefs.GetStatsColor(1)) ;
	
	m_DownloadOMeter.InvalidateCtrl();
	m_UploadOMeter.InvalidateCtrl();
	m_Statistics.InvalidateCtrl();

	if (thePrefs.GetStatsInterval()==0) 
		stattree.EnableWindow(false);

	UpdateData(FALSE);

	EnableWindow( TRUE );

	m_ilastMaxConnReached = 0;
	CRect rcW,rcSpl,rcTree,rcStat;
	
	GetWindowRect(rcW);
	ScreenToClient(rcW);

	stattree.GetWindowRect(rcTree);
	ScreenToClient(rcTree);
	m_DownloadOMeter.GetWindowRect(rcDown);
	ScreenToClient(rcDown);
	m_UploadOMeter.GetWindowRect(rcUp);
	ScreenToClient(rcUp);
	m_Statistics.GetWindowRect(rcStat);
	ScreenToClient(rcStat);
		
	//vertical splitter
	rcSpl.left = rcTree.right;
	rcSpl.right = rcSpl.left + 4;
	rcSpl.top = rcW.top + 2;
	rcSpl.bottom = rcW.bottom - 5;
	m_wndSplitterstat.Create(WS_CHILD | WS_VISIBLE, rcSpl, this, IDC_SPLITTER_STAT);

	int PosStatVinitX = rcSpl.left;
	int PosStatVnewX = thePrefs.GetSplitterbarPositionStat()*rcW.Width()/100;
	int maxX = rcW.right-13;
	int minX = rcW.left+8;
	if (thePrefs.GetSplitterbarPositionStat() > 90)
		PosStatVnewX = maxX;
	else if (thePrefs.GetSplitterbarPositionStat() < 10)
		PosStatVnewX = minX;
	rcSpl.left = PosStatVnewX;
	rcSpl.right = PosStatVnewX + 4;

	m_wndSplitterstat.MoveWindow(rcSpl);

	//HR splitter
	rcSpl.left = rcDown.left;
	rcSpl.right = rcDown.right;
	rcSpl.top = rcDown.bottom;
	rcSpl.bottom = rcSpl.top + 4; 
	m_wndSplitterstat_HR.Create(WS_CHILD | WS_VISIBLE, rcSpl, this, IDC_SPLITTER_STAT_HR);

	//int PosStatVinitZ = rcSpl.top;
	int PosStatVnewZ = thePrefs.GetSplitterbarPositionStat_HR()*rcW.Height()/100;
	int maxZ = rcW.bottom-14;
	int minZ = 0;
	if (thePrefs.GetSplitterbarPositionStat_HR() > 90) 
		PosStatVnewZ = maxZ;
	else if (thePrefs.GetSplitterbarPositionStat_HR() < 10)
		PosStatVnewZ = minZ;
	rcSpl.top = PosStatVnewZ;
	rcSpl.bottom = PosStatVnewZ+4;

	m_wndSplitterstat_HR.MoveWindow(rcSpl);

	//Xman statistic fix bluesonicboy
	//Init. Pos. fix - Set Download Scope relative to this splitter, right and left are set
	//                 top will always be 0. Also set Upload Scope top position bottom will be set later.
	if(rcSpl.top) 
		rcDown.bottom = rcSpl.top - 1;
	else            
		rcDown.bottom = 0;
	m_DownloadOMeter.MoveWindow(rcDown);
	rcUp.top = rcSpl.bottom + 1;
	//Xman end

	//HL splitter
	rcSpl.left = rcUp.left;
	rcSpl.right = rcUp.right;
	rcSpl.top = rcUp.bottom;
	rcSpl.bottom = rcSpl.top + 4;
	m_wndSplitterstat_HL.Create(WS_CHILD | WS_VISIBLE, rcSpl, this, IDC_SPLITTER_STAT_HL);

	//int PosStatVinitY = rcSpl.top;
	int PosStatVnewY = thePrefs.GetSplitterbarPositionStat_HL()*rcW.Height()/100;
	int maxY = rcW.bottom-9;
	int minY = 10;
	if (thePrefs.GetSplitterbarPositionStat_HL() > 90)
		PosStatVnewY = maxY;
	else if (thePrefs.GetSplitterbarPositionStat_HL() < 10)
		PosStatVnewY = minY;
	rcSpl.top = PosStatVnewY;
	rcSpl.bottom = PosStatVnewY+4;

	m_wndSplitterstat_HL.MoveWindow(rcSpl);

	//Xman statistic fix bluesonicboy
	//  Init. Pos. fix - Set Upload Scope bottom relative to this splitter, right and left are set.
	//                                      Also set Connection Scope Top relative to this splitter; bottom, right and left are set.
	if(rcSpl.top) 
		rcUp.bottom = rcSpl.top - 1;
	else
		rcUp.bottom = 0;
	if(rcUp.top > rcUp.bottom)
		rcUp.top = rcUp.bottom;
	m_UploadOMeter.MoveWindow(rcUp);
	rcStat.top = rcSpl.bottom + 1;
	if(rcStat.top > rcStat.bottom)
		rcStat.top = rcStat.bottom;
	m_Statistics.MoveWindow(rcStat);

	DoResize_V(PosStatVnewX - PosStatVinitX);
	/*
	// statistic fix 
	DoResize_HL(PosStatVnewY - PosStatVinitY);
	DoResize_HR(PosStatVnewZ - PosStatVinitZ);
	*/
	//Xman end

	Localize();
	
	m_TimeToolTips = new CToolTipCtrl();
	m_TimeToolTips->Create(this);
	m_TimeToolTips->AddTool(GetDlgItem(IDC_SCOPE_D), _T(""), NULL, 0);
	m_TimeToolTips->AddTool(GetDlgItem(IDC_SCOPE_U), _T(""), NULL, 0);
	m_TimeToolTips->AddTool(GetDlgItem(IDC_STATSSCOPE),	_T(""), NULL, 0);
	// Any Autopop-Time which is specified higher than ~30 sec. will get reset to 5 sec.
	m_TimeToolTips->SetDelayTime(TTDT_AUTOPOP, 30000);
	m_TimeToolTips->SetDelayTime(TTDT_INITIAL, 30000);
	m_TimeToolTips->SetDelayTime(TTDT_RESHOW,  30000);
	EnableToolTips(TRUE);

	return true;
}

void CStatisticsDlg::initCSize()
{
	UINT x = thePrefs.GetSplitterbarPositionStat();
	UINT y = thePrefs.GetSplitterbarPositionStat_HL();
	UINT z = thePrefs.GetSplitterbarPositionStat_HR();
	if (x > 90)
		x = 100;
	else if (x < 10)
		x = 0;
	if (y > 90)
		y = 100;
	else if (y < 10)
		y = 0;
	if (z > 90)
		z = 100;
	else if (z < 10)
		z = 0;

	RemoveAnchor(IDC_BNMENU);
	AddAnchor(IDC_BNMENU,TOP_LEFT);

	//StatTitle
	RemoveAnchor(IDC_STATIC_LASTRESET);
	AddAnchor(IDC_STATIC_LASTRESET,TOP_LEFT,CSize(x,0));
	//stattree
	RemoveAnchor(stattree);
	AddAnchor(stattree,TOP_LEFT,CSize(x,100));

	//graph
	RemoveAnchor(m_DownloadOMeter);
	AddAnchor(m_DownloadOMeter,CSize(x,0),CSize(100,z));

	RemoveAnchor(m_UploadOMeter);
	AddAnchor(m_UploadOMeter,CSize(x,z),CSize(100,y));

	RemoveAnchor(m_Statistics);
	AddAnchor(m_Statistics,CSize(x,y),BOTTOM_RIGHT);
	
	//set range
	CRect rcW;
	GetWindowRect(rcW);
	ScreenToClient(rcW);

	CRect rcHR,rcHL;
	m_wndSplitterstat_HR.GetWindowRect(rcHR);
	m_wndSplitterstat_HL.GetWindowRect(rcHL);
	ScreenToClient(rcHR);
	ScreenToClient(rcHL);

	m_wndSplitterstat.SetRange(rcW.left+11, rcW.right-11);
	m_wndSplitterstat_HL.SetRange(rcHR.bottom+5, rcW.bottom-7);
	m_wndSplitterstat_HR.SetRange(rcW.top+3, rcHL.top-5);
}


void CStatisticsDlg::DoResize_HL(int delta)
{
	if(!delta)
		return;
	m_DownloadOMeter.InvalidateCtrl(true);
	CSplitterControl::ChangeHeight(&m_UploadOMeter, delta , CW_TOPALIGN);
	CSplitterControl::ChangeHeight(&m_Statistics, -delta, CW_BOTTOMALIGN);

	CRect rcW;
 
	GetWindowRect(rcW);
	ScreenToClient(rcW);

	CRect rcspl;

	m_UploadOMeter.GetWindowRect(rcspl);
	ScreenToClient(rcspl);
	thePrefs.SetSplitterbarPositionStat_HL((int)floor((float)(rcspl.bottom*100.0f)/(float)rcW.Height())); // Xman

	initCSize();

	ShowInterval();
	Invalidate();
	UpdateWindow();
}

void CStatisticsDlg::DoResize_HR(int delta)
{
	if(!delta)
		return;
	CSplitterControl::ChangeHeight(&m_DownloadOMeter, delta , CW_TOPALIGN);
	CSplitterControl::ChangeHeight(&m_UploadOMeter, -delta, CW_BOTTOMALIGN);
	m_Statistics.InvalidateCtrl(true);
	CRect rcW;
 
	GetWindowRect(rcW);
	ScreenToClient(rcW);

	CRect rcspl;

	m_DownloadOMeter.GetWindowRect(rcspl);
	ScreenToClient(rcspl);
	thePrefs.SetSplitterbarPositionStat_HR((int)floor((float)(rcspl.bottom*100.0f)/(float)(rcW.Height()))); // Xman

	initCSize();

	ShowInterval();
	Invalidate();
	UpdateWindow();
}

void CStatisticsDlg::DoResize_V(int delta)
{
	if(!delta)
		return;
	CSplitterControl::ChangeWidth(GetDlgItem(IDC_STATIC_LASTRESET), delta);
	CSplitterControl::ChangeWidth(&stattree, delta);
	CSplitterControl::ChangeWidth(&m_DownloadOMeter, -delta, CW_RIGHTALIGN);
	CSplitterControl::ChangeWidth(&m_UploadOMeter, -delta, CW_RIGHTALIGN);
	CSplitterControl::ChangeWidth(&m_Statistics, -delta, CW_RIGHTALIGN);

	CRect rcW;
 
	GetWindowRect(rcW);
	ScreenToClient(rcW);

	CRect rcspl;

	stattree.GetWindowRect(rcspl);
	ScreenToClient(rcspl);
	thePrefs.SetSplitterbarPositionStat((int)floor((float)(rcspl.right*100.0f)/(float)rcW.Width())); // Xman

	if (rcspl.left==rcspl.right) {
		stattree.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BNMENU)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_LASTRESET)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATTREE)->EnableWindow(false);//TK4 Mod
		m_bTreepaneHidden=true;
	} else if (m_bTreepaneHidden) {
		m_bTreepaneHidden=false;
		stattree.ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BNMENU)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_LASTRESET)->ShowWindow(SW_SHOW);
	    GetDlgItem(IDC_STATTREE)->EnableWindow(true);//TK4 Mod
	}

	initCSize();

	ShowInterval();
	Invalidate();
	UpdateWindow();
}

LRESULT CStatisticsDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message) 
	{
		case WM_PAINT:
			if (m_wndSplitterstat) 
			{
				CRect rctree,rcSpl,rcW;
				GetWindowRect(rcW);
				ScreenToClient(rcW);

				stattree.GetWindowRect(rctree);
				ScreenToClient(rctree);
				
				if (rcW.Width()>0) 
				{
					rcSpl.left = rctree.right;
					rcSpl.right = rcSpl.left + 4;
					rcSpl.top = rcW.top + 2;
					rcSpl.bottom = rcW.bottom - 5;
					m_wndSplitterstat.MoveWindow(rcSpl,true);
				}
			}
			if (m_wndSplitterstat_HL) 
			{
				CRect rcUp,rcSpl,rcW;
				CWnd* pWnd;

				GetWindowRect(rcW);
				ScreenToClient(rcW);

				pWnd = &m_UploadOMeter;
				pWnd->GetWindowRect(rcUp);

				ScreenToClient(rcUp);

				if (rcW.Height()>0) 
				{
					rcSpl.left = rcUp.left;
					rcSpl.right = rcUp.right;
					rcSpl.top = rcUp.bottom;
					rcSpl.bottom = rcUp.bottom + 4;
					m_wndSplitterstat_HL.MoveWindow(rcSpl,true);
				}
			}
			if (m_wndSplitterstat_HR) 
			{
				CRect rcDown,rcSpl,rcW;
				CWnd* pWnd;

				GetWindowRect(rcW);
				ScreenToClient(rcW);

				pWnd = &m_DownloadOMeter;
				pWnd->GetWindowRect(rcDown);
				ScreenToClient(rcDown);

				if (rcW.Height()>0) 
				{
					rcSpl.left = rcDown.left;
					rcSpl.right = rcDown.right;
					rcSpl.top = rcDown.bottom;
					rcSpl.bottom = rcDown.bottom + 4;
					m_wndSplitterstat_HR.MoveWindow(rcSpl,true);
				}
			}
			break;
		case WM_NOTIFY:
			if (wParam == IDC_SPLITTER_STAT)
			{ 
				SPC_NMHDR* pHdr = (SPC_NMHDR*) lParam;
				DoResize_V(pHdr->delta);
			}
			else if (wParam == IDC_SPLITTER_STAT_HL)
			{ 
				SPC_NMHDR* pHdr = (SPC_NMHDR*) lParam;
				DoResize_HL(pHdr->delta);
			}
			else if (wParam == IDC_SPLITTER_STAT_HR)
			{ 
				SPC_NMHDR* pHdr = (SPC_NMHDR*) lParam;
				DoResize_HR(pHdr->delta);
			}
			break;

		case WM_WINDOWPOSCHANGED:
			if (m_wndSplitterstat)
				m_wndSplitterstat.Invalidate();
			if (m_wndSplitterstat_HL)
				m_wndSplitterstat_HL.Invalidate();
			if (m_wndSplitterstat_HR)
				m_wndSplitterstat_HR.Invalidate();
			break;

		case WM_SIZE:
		{
			//set range
			if (m_wndSplitterstat)
			{
				CRect rcW;
				GetWindowRect(rcW);
				ScreenToClient(rcW);
				if (rcW.Width()>0)
				{
					RECT rcSpl;
					long splitposstat=thePrefs.GetSplitterbarPositionStat()*rcW.Width()/100;
					rcSpl.left = splitposstat; 
					rcSpl.right = rcSpl.left + 4; 
					rcSpl.top = rcW.top + 2; 
					rcSpl.bottom = rcW.bottom - 5;
	   				m_wndSplitterstat.MoveWindow(&rcSpl,true);
					m_wndSplitterstat.SetRange(rcW.left+11, rcW.right-11);
				}
			}
			if (m_wndSplitterstat_HR)
			{
				CRect rcW;
				GetWindowRect(rcW);
				ScreenToClient(rcW);
				if (rcW.Width()>0)
				{
					RECT rcSpl;
					long splitposstat=thePrefs.GetSplitterbarPositionStat()*rcW.Width()/100;
					long splitposstat_HR=thePrefs.GetSplitterbarPositionStat_HR()*rcW.Height()/100;
					long splitposstat_HL=thePrefs.GetSplitterbarPositionStat_HL()*rcW.Height()/100;
					rcSpl.left = splitposstat + 7;
					rcSpl.right = rcW.right - 14; 
					rcSpl.top = splitposstat_HR; 
					rcSpl.bottom = rcSpl.top + 4;
					m_wndSplitterstat_HR.MoveWindow(&rcSpl,true);
					m_wndSplitterstat_HR.SetRange(rcW.top+3, splitposstat_HL-4);
				}
			}
			if (m_wndSplitterstat_HL)
			{
				CRect rcW;
				GetWindowRect(rcW);
				ScreenToClient(rcW);
				if (rcW.Width()>0)
				{
					RECT rcSpl;
					long splitposstat=thePrefs.GetSplitterbarPositionStat()*rcW.Width()/100;
					long splitposstat_HR=thePrefs.GetSplitterbarPositionStat_HR()*rcW.Height()/100;
					long splitposstat_HL=thePrefs.GetSplitterbarPositionStat_HL()*rcW.Height()/100;
					rcSpl.left = splitposstat + 7; 
					rcSpl.right = rcW.right - 14; 
					rcSpl.top = splitposstat_HL; 
					rcSpl.bottom = rcSpl.top + 4;
					m_wndSplitterstat_HL.MoveWindow(&rcSpl,true);
					m_wndSplitterstat_HL.SetRange(splitposstat_HR+14, rcW.bottom-7);
				}
			}
			break;
		}
	}
	return CResizableDialog::DefWindowProc(message, wParam, lParam);
}

//Xman
//remark: no friendupload, no full activated slots
// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
void CStatisticsDlg::RepaintMeters() 
{
	CString Buffer;

	m_DownloadOMeter.SetBackgroundColor(thePrefs.GetStatsColor(0)) ;
	m_DownloadOMeter.SetGridColor(thePrefs.GetStatsColor(1)) ;
	m_DownloadOMeter.SetPlotColor(thePrefs.GetStatsColor(2), CURRENT);
	m_DownloadOMeter.SetPlotColor(thePrefs.GetStatsColor(3), MINUTE);
	m_DownloadOMeter.SetPlotColor(thePrefs.GetStatsColor(4), SESSION);
	m_DownloadOMeter.SetPlotColor(thePrefs.GetStatsColor(12), OVERALL);
	m_DownloadOMeter.SetPlotColor(thePrefs.GetStatsColor(13), ADAPTER);
	m_DownloadOMeter.SetBarsPlot(thePrefs.GetFillGraphs(), CURRENT);
	m_DownloadOMeter.SetBarsPlot(thePrefs.GetFillGraphs() && !thePrefs.GetNAFCFullControl(), OVERALL);
	m_DownloadOMeter.SetBarsPlot(thePrefs.GetFillGraphs() && thePrefs.GetNAFCFullControl(), ADAPTER);

	m_UploadOMeter.SetBackgroundColor(thePrefs.GetStatsColor(0)) ;
	m_UploadOMeter.SetGridColor(thePrefs.GetStatsColor(1)) ;
	m_UploadOMeter.SetPlotColor(thePrefs.GetStatsColor(5), CURRENT);
	m_UploadOMeter.SetPlotColor(thePrefs.GetStatsColor(6), MINUTE);
	m_UploadOMeter.SetPlotColor(thePrefs.GetStatsColor(7), SESSION);
	m_UploadOMeter.SetPlotColor(thePrefs.GetStatsColor(12), OVERALL);
	m_UploadOMeter.SetPlotColor(thePrefs.GetStatsColor(13), ADAPTER);
	m_UploadOMeter.SetBarsPlot(thePrefs.GetFillGraphs(), CURRENT);
	m_UploadOMeter.SetBarsPlot(thePrefs.GetFillGraphs() && !thePrefs.GetNAFCFullControl(), OVERALL);
	m_UploadOMeter.SetBarsPlot(thePrefs.GetFillGraphs() && thePrefs.GetNAFCFullControl(), ADAPTER);

	m_Statistics.SetBackgroundColor(thePrefs.GetStatsColor(0)) ;
	m_Statistics.SetGridColor(thePrefs.GetStatsColor(1)) ;
	m_Statistics.SetPlotColor( thePrefs.GetStatsColor(8),0) ; //Connections
	m_Statistics.SetPlotColor( thePrefs.GetStatsColor(10),1) ; // uploads
	m_Statistics.SetPlotColor( thePrefs.GetStatsColor(9),2) ; //downloads
	//m_Statistics.SetPlotColor( thePrefs.GetStatsColor(12),3) ; //fully activated slots 
	m_Statistics.SetBarsPlot(thePrefs.GetFillGraphs(), 0);

	//Xman from TPT
	//Download Graph
	m_DownloadOMeter.SetYUnits(GetResString(IDS_ST_DOWNLOAD));
	m_DownloadOMeter.SetLegendLabel(GetResString(IDS_ST_CURRENT),CURRENT);
	Buffer.Format(_T(" (%u %s)"),thePrefs.GetStatsAverageMinutes(),GetResString(IDS_MINS));
	m_DownloadOMeter.SetLegendLabel(GetResString(IDS_AVG)+Buffer,MINUTE);
	m_DownloadOMeter.SetLegendLabel(GetResString(IDS_ST_SESSION),SESSION);
	m_DownloadOMeter.SetLegendLabel(GetResString(IDS_EMULE_CTRL_DATA),OVERALL);
	m_DownloadOMeter.SetLegendLabel(GetResString(IDS_NETWORK_ADAPTER),ADAPTER);


	m_UploadOMeter.SetYUnits(GetResString(IDS_ST_UPLOAD));
	m_UploadOMeter.SetLegendLabel(GetResString(IDS_ST_CURRENT),CURRENT);
	Buffer.Format(_T(" (%u %s)"),thePrefs.GetStatsAverageMinutes(),GetResString(IDS_MINS));
	m_UploadOMeter.SetLegendLabel(GetResString(IDS_AVG)+Buffer,MINUTE);
	m_UploadOMeter.SetLegendLabel(GetResString(IDS_ST_SESSION),SESSION);
	 m_UploadOMeter.SetLegendLabel(GetResString(IDS_EMULE_CTRL_DATA),OVERALL);
	m_UploadOMeter.SetLegendLabel(GetResString(IDS_NETWORK_ADAPTER),ADAPTER);

	m_Statistics.SetYUnits(GetResString(IDS_FSTAT_CONNECTION)); //Connections
	Buffer.Format(_T("%s (1:%u)"), GetResString(IDS_ST_ACTIVEC), thePrefs.GetStatsConnectionsGraphRatio());
	m_Statistics.SetLegendLabel(Buffer,0);
	m_Statistics.SetLegendLabel(GetResString(IDS_SP_TOTALUL),1); //upload
	m_Statistics.SetLegendLabel(GetResString(IDS_ST_ACTIVED),2); //download


}
// Maella end

// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
void CStatisticsDlg::ShowGraphs() {	
	if (!CemuleDlg::IsRunning())
		return;

	// (pre)Update statistic graph (Active connections, Active Upload, etc...)
	//CDownloadQueue::SDownloadStats myStats;
	//theApp.downloadqueue->GetDownloadSourcesStats(myStats);

	double plotStatistic[3];
	plotStatistic[0] = (double)(theApp.listensocket->GetOpenSockets());      // Active connections
	plotStatistic[1] = (double)(theApp.uploadqueue->GetUploadQueueLength()); // Active upload
	plotStatistic[2] = (double)(theApp.downloadqueue->GetTransferingSources()/*myStats.a[1]*/);                                 // Active download

	// (pre)Update upload/download graphs
	double plotSentData[NUMBEROFLINES+1];
	double plotReceivedData[NUMBEROFLINES+1];

	// Get Current datarates
	uint32 plotOutData[NUMBEROFLINES+1];
	uint32 plotinData[NUMBEROFLINES+1];

	//Xman smooth-accurate-graph
	UINT wantedinterval= (thePrefs.usesmoothgraph && 30>thePrefs.GetTrafficOMeterInterval())?30 : thePrefs.GetTrafficOMeterInterval();// X: [CI] - [Code Improvement]

	theApp.pBandWidthControl->GetDatarates(wantedinterval,
										   plotinData[CURRENT], plotinData[OVERALL],
										   plotOutData[CURRENT], plotOutData[OVERALL],
										   plotinData[ADAPTER], plotOutData[ADAPTER]);
	//Xman end
	theApp.pBandWidthControl->GetFullHistoryDatarates(plotinData[MINUTE], plotOutData[MINUTE],
													  plotinData[SESSION], plotOutData[SESSION]);

	plotReceivedData[CURRENT] = (double)plotinData[CURRENT]/1024.0;
	
	//Xman show additional graph lines
	if(thePrefs.m_bShowAdditionalGraph==true)
	{
		plotReceivedData[OVERALL] = (double)plotinData[OVERALL]/1024.0;
		plotReceivedData[ADAPTER] = (double)plotinData[ADAPTER]/1024.0;
	}
	else
	{
		plotReceivedData[OVERALL] = 0;
		plotReceivedData[ADAPTER] = 0;
	}
	//Xman end
	plotReceivedData[MINUTE]  = (double)plotinData[MINUTE]/1024.0;
	plotReceivedData[SESSION] = (double)plotinData[SESSION]/1024.0;
	plotSentData[CURRENT]     = (double)plotOutData[CURRENT]/1024.0;
	
	//Xman show additional graph lines
	if(thePrefs.m_bShowAdditionalGraph==true)
	{
		plotSentData[OVERALL]     = (double)plotOutData[OVERALL]/1024.0;
		plotSentData[ADAPTER]     = (double)plotOutData[ADAPTER]/1024.0;
	}
	else if(thePrefs.GetNAFCFullControl())
	{
		plotSentData[OVERALL]     = 0;
		plotSentData[ADAPTER]     = (double)plotOutData[ADAPTER]/1024.0;
	}
	else
	{
		plotSentData[OVERALL]     = (double)plotOutData[OVERALL]/1024.0;
		plotSentData[ADAPTER]     = 0;
	}
	//Xman end

	plotSentData[MINUTE]      = (double)plotOutData[MINUTE]/1024.0;
	plotSentData[SESSION]     = (double)plotOutData[SESSION]/1024.0;

	// Webserver
	UpDown updown;
	updown.upload = plotOutData[CURRENT];
	updown.download = plotinData[CURRENT];
	updown.connections = theApp.listensocket->GetActiveConnections();
	theApp.webserver->AddStatsLine(updown);

	// Update graph
	m_Statistics.AppendPoints(plotStatistic);
	m_UploadOMeter.AppendPoints(plotSentData);
	m_DownloadOMeter.AppendPoints(plotReceivedData);
}
// Maella end

// Maella -Graph: code Improvement for rate display-
void CStatisticsDlg::Process() {

	if (!CemuleDlg::IsRunning())
		return;

	// Wait 5 seconds before starting the full processing
	// => security
	if(::GetTickCount() > theApp.pBandWidthControl->GetStartTick() + 5000){
		// Remark: this method should be called every seconds
		theStats.UpdateConnectionStats();

		// Check and then update graph
		if(thePrefs.GetTrafficOMeterInterval() != 0 && 
			thePrefs.GetTrafficOMeterInterval() <= ++m_intervalGraph){
			m_intervalGraph = 0;
			ShowGraphs();
		}

		// Check and then update statistic
		if(thePrefs.GetStatsInterval() != 0 && 
			thePrefs.GetStatsInterval() <= ++m_intervalStat
			&& theApp.emuledlg->activewnd == theApp.emuledlg->statisticswnd){// X: [CI] - [Code Improvement]
			//m_intervalStat = 0; // X: move to ShowStatistics
			ShowStatistics();
		}
	}
}
// Maella end

//Xman end


void CStatisticsDlg::ShowStatistics(bool forceUpdate) 
{
	m_intervalStat = 0;// X: [CI] - [Code Improvement]
	stattree.SetRedraw(false);
	CString	cbuffer;
	// Set Tree Values

	//Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	//cache needed values
	uint64 bEmuleIn= theApp.pBandWidthControl->GeteMuleIn();
	uint64 bEmuleOut= theApp.pBandWidthControl->GeteMuleOut();

	// TRANSFER SECTION
	// If a section is not expanded, don't waste CPU cycles updating it.
	if (forceUpdate || stattree.IsExpanded(h_transfer)) 
	{
		uint32	statGoodSessions =				0;
		uint32	statBadSessions =				0;
		double	percentSessions =				0;
		// Transfer Ratios
		if ( bEmuleIn>0 && bEmuleOut>1 ) // X: [BF] - [Bug Fix] don't show ratio when bEmuleOut=1
		{
			// Session
			if (bEmuleIn<bEmuleOut) 
				cbuffer.Format(_T("%s %.2f : 1"),GetResString(IDS_STATS_SRATIO),(float)bEmuleOut/bEmuleIn);
			else 
				cbuffer.Format(_T("%s 1 : %.2f"),GetResString(IDS_STATS_SRATIO),(float)bEmuleIn/bEmuleOut);
			stattree.SetItemText(trans[0], cbuffer);
			// Session
			if (bEmuleOut > theStats.sessionSentBytesToFriend && bEmuleIn<bEmuleOut-theStats.sessionSentBytesToFriend) 
				cbuffer.Format(_T("%s %.2f : 1"),GetResString(IDS_STATS_FRATIO),(float)(bEmuleOut-theStats.sessionSentBytesToFriend)/bEmuleIn);
			else 
				cbuffer.Format(_T("%s 1 : %.2f"),GetResString(IDS_STATS_FRATIO),(float)bEmuleIn/(bEmuleOut-theStats.sessionSentBytesToFriend));
			stattree.SetItemText(trans[1], cbuffer);
		}

		if ( (thePrefs.GetTotalDownloaded()>0 && thePrefs.GetTotalUploaded()>1) || (bEmuleIn>0 && bEmuleOut>1) )  // X: [BF] - [Bug Fix] don't show ratio when bEmuleOut=1
		{
			// Cumulative
			if ((bEmuleIn+thePrefs.GetTotalDownloaded())<(bEmuleOut+thePrefs.GetTotalUploaded())) 
				cbuffer.Format(_T("%s %.2f : 1"),GetResString(IDS_STATS_CRATIO),(float)(bEmuleOut+thePrefs.GetTotalUploaded())/(bEmuleIn+thePrefs.GetTotalDownloaded()));
			else 
				cbuffer.Format(_T("%s 1 : %.2f"),GetResString(IDS_STATS_CRATIO),(float)(bEmuleIn+thePrefs.GetTotalDownloaded())/(bEmuleOut+thePrefs.GetTotalUploaded()));
			stattree.SetItemText(trans[2], cbuffer);
		}
		// TRANSFER -> DOWNLOADS SECTION
		if (forceUpdate || stattree.IsExpanded(h_download)) 
		{
			uint64	DownOHTotal = 0;
			uint64	DownOHTotalPackets = 0;
			CDownloadQueue::SDownloadStats myStats;
			theApp.downloadqueue->GetDownloadSourcesStats(myStats);
			// TRANSFER -> DOWNLOADS -> SESSION SECTION
			if (forceUpdate || stattree.IsExpanded(h_down_session)) 
			{
				// Downloaded Data
				//Xman show complete internettraffic
				cbuffer.Format( GetResString( IDS_STATS_DDATA) + _T(" / %s")   , CastItoXBytes( bEmuleIn, false, false ) , CastItoXBytes( theApp.pBandWidthControl->GetSessionNetworkIn(), false, false ) );
				//Xman end
				stattree.SetItemText( down_S[0] , cbuffer );
				if (forceUpdate || stattree.IsExpanded(down_S[0])) 
				{
					// Downloaded Data By Client
					if (forceUpdate || stattree.IsExpanded(hdown_scb)) 
					{
						uint64 DownDataTotal =		thePrefs.GetDownSessionClientData();
						uint64 DownDataClient;
						double percentClientTransferred;
						for(INT_PTR i=0;i<DOWN_SOFT_COUNT;i++){
							DownDataClient = thePrefs.sesDownData[i];
							if ( DownDataTotal!=0 && DownDataClient!=0 )
								percentClientTransferred = (double) 100 * DownDataClient / DownDataTotal;
							else
								percentClientTransferred = 0;
							cbuffer.Format( softString[i] , CastItoXBytes( DownDataClient, false, false ) , percentClientTransferred);
							stattree.SetItemText( down_scb[i] , cbuffer );
						}
					}
					// Downloaded Data By Port
					if (forceUpdate || stattree.IsExpanded(hdown_spb)) 
					{
						uint64	PortDataTotal =		thePrefs.GetDownSessionDataPort();
						uint64	PortData;
						double	percentPortTransferred;
						for(INT_PTR i=0;i<PORT_COUNT;i++){
							PortData =	thePrefs.sesDownDataPort[i];
							if ( PortDataTotal!=0 && PortData!=0 )
								percentPortTransferred = (double) 100 * PortData / PortDataTotal;
							else
								percentPortTransferred = 0;
							cbuffer.Format( _T("%s: %s (%1.1f%%)") , GetResString(portStrID[i]) , CastItoXBytes( PortData, false, false ) , percentPortTransferred);
							stattree.SetItemText( down_spb[i] , cbuffer );
						}
					}
				}
				// Completed Downloads
				cbuffer.Format( _T("%s: %u") , GetResString( IDS_STATS_COMPDL ) , thePrefs.GetDownSessionCompletedFiles() );
				stattree.SetItemText( down_S[1] , cbuffer );
				// Active Downloads
				cbuffer.Format( GetResString( IDS_STATS_ACTDL ) , myStats.a[1] );
				stattree.SetItemText( down_S[2] , cbuffer );
				// Found Sources
				cbuffer.Format( GetResString( IDS_STATS_FOUNDSRC ) , myStats.a[0] );
				stattree.SetItemText( down_S[3] , cbuffer );
				if (forceUpdate || stattree.IsExpanded(down_S[3])) 
				{ 
					int i = 0;
					// Sources By Status
					cbuffer.Format( _T("%s: %u") , GetResString( IDS_ONQUEUE ) , myStats.a[2] );
					stattree.SetItemText( down_sources[i] , cbuffer );
					i++;
					cbuffer.Format( _T("%s: %u") , GetResString( IDS_QUEUEFULL ) , myStats.a[3] );
					stattree.SetItemText( down_sources[i] , cbuffer );
					i++;
					cbuffer.Format( _T("%s: %u") , GetResString( IDS_NONEEDEDPARTS ) , myStats.a[4] );
					stattree.SetItemText( down_sources[i] , cbuffer );
					i++;
					cbuffer.Format( _T("%s: %u") , GetResString( IDS_ASKING ) , myStats.a[5] );
					stattree.SetItemText( down_sources[i] , cbuffer );
					i++;
					cbuffer.Format( _T("%s: %u") , GetResString( IDS_RECHASHSET ) , myStats.a[6] );
					stattree.SetItemText( down_sources[i] , cbuffer );
					i++;
					cbuffer.Format( _T("%s: %u") , GetResString( IDS_CONNECTING ) , myStats.a[7] );
					stattree.SetItemText( down_sources[i] , cbuffer );
					i++;
					cbuffer.Format( _T("%s: %u") , GetResString(IDS_CONNVIASERVER) , myStats.a[8] );
					stattree.SetItemText( down_sources[i] , cbuffer );
					i++;
					cbuffer.Format( _T("%s: %u") , GetResString(IDS_TOOMANYCONNS) , myStats.a[9] );
					stattree.SetItemText( down_sources[i] , cbuffer );
					i++;
					cbuffer.Format( _T("%s: %u") , GetResString(IDS_NOCONNECTLOW2LOW) , myStats.a[10] );
					stattree.SetItemText( down_sources[i] , cbuffer );
					i++;
					cbuffer.Format( _T("%s: %u") , GetResString(IDS_STATS_PROBLEMATIC) , myStats.a[12] );
					stattree.SetItemText( down_sources[i] , cbuffer );
					i++;
					cbuffer.Format( _T("%s: %u") , GetResString(IDS_BANNED) , myStats.a[13] );
					stattree.SetItemText( down_sources[i] , cbuffer );
					i++;
					cbuffer.Format( _T("%s: %u") , GetResString(IDS_ASKED4ANOTHERFILE) , myStats.a[15] );
					stattree.SetItemText( down_sources[i] , cbuffer );
					i++;
					cbuffer.Format( _T("%s: %u") , GetResString(IDS_UNKNOWN) , myStats.a[11] );
					stattree.SetItemText( down_sources[i] , cbuffer );
					i++;

					// where from? (3)
					cbuffer.Format( _T("%s: %u") , GetResString( IDS_VIAED2KSQ ) , myStats.a[16] );
					stattree.SetItemText( down_sources[i] , cbuffer );
					i++;
					cbuffer.Format( _T("%s: %u") , GetResString( IDS_VIAKAD ) , myStats.a[17] );
					stattree.SetItemText( down_sources[i] , cbuffer );
					i++;
					cbuffer.Format( _T("%s: %u") , GetResString( IDS_VIASE ) , myStats.a[18] );
					stattree.SetItemText( down_sources[i] , cbuffer );
					i++;
					cbuffer.Format( _T("%s: %u") , GetResString( IDS_VIAPASSIVE ) , myStats.a[14] );
					stattree.SetItemText( down_sources[i] , cbuffer );
					i++;
					cbuffer.Format( _T("eD2K: %u (%.1f%%)") , myStats.a[19], myStats.a[0] ? (myStats.a[19] * 100.0 / myStats.a[0]) : 0.0 );
					stattree.SetItemText( down_sources[i] , cbuffer );
					i++;
					cbuffer.Format( _T("Kad: %u (%.1f%%)") , myStats.a[20], myStats.a[0] ? (myStats.a[20] * 100.0 / myStats.a[0]) : 0.0 );
					stattree.SetItemText( down_sources[i] , cbuffer );
					i++;
					cbuffer.Format( _T("eD2K/Kad: %u (%.1f%%)") , myStats.a[21], myStats.a[0] ? (myStats.a[21] * 100.0 / myStats.a[0]) : 0.0 );
					stattree.SetItemText( down_sources[i] , cbuffer );
					i++;

					cbuffer.Format(_T("%s: %s, %s: %s (%.1f%%)"), GetResString(IDS_UDPREASKS), CastItoIShort(theApp.downloadqueue->GetUDPFileReasks()), GetResString(IDS_UFAILED), CastItoIShort(theApp.downloadqueue->GetFailedUDPFileReasks()), theApp.downloadqueue->GetUDPFileReasks() ? (theApp.downloadqueue->GetFailedUDPFileReasks() * 100.0 / theApp.downloadqueue->GetUDPFileReasks()) : 0.0 );
					stattree.SetItemText( down_sources[i] , cbuffer );
					i++;
					
					//Xman x4 Xtreme Mod: count failed tcp-connections:
					cbuffer.Format(_T("%s: %s, %s: %s (%.1f%%)"), _T("TCP-connections"), CastItoIShort(theApp.downloadqueue->GetTCPFileReasks()), _T("failed"), CastItoIShort(theApp.downloadqueue->GetFailedTCPFileReasks()), theApp.downloadqueue->GetTCPFileReasks() ? (theApp.downloadqueue->GetFailedTCPFileReasks() * 100.0 / theApp.downloadqueue->GetTCPFileReasks()) : 0.0 );
					stattree.SetItemText( down_sources[i] , cbuffer );
					i++;
					//Xman end

					cbuffer.Format(_T("%s: %s (%s + %s)"), GetResString(IDS_DEADSOURCES), CastItoIShort(theApp.clientlist->m_globDeadSourceList.GetDeadSourcesCount() + myStats.a[22]), CastItoIShort(theApp.clientlist->m_globDeadSourceList.GetDeadSourcesCount()), CastItoIShort(myStats.a[22]));
					stattree.SetItemText( down_sources[i] , cbuffer );
					i++;
				}
				// Set Download Sessions
				statGoodSessions =	thePrefs.GetDownS_SuccessfulSessions() + myStats.a[1]; // Add Active Downloads
				statBadSessions =	thePrefs.GetDownS_FailedSessions();
				cbuffer.Format( _T("%s: %u") , GetResString(IDS_STATS_DLSES) , statGoodSessions + statBadSessions );
				stattree.SetItemText( down_S[4] , cbuffer );
				if (forceUpdate || stattree.IsExpanded(down_S[4])) 
				{
					// Set Successful Download Sessions and Average Downloaded Per Session
					percentSessions = 0;
					if (statGoodSessions > 0) 
					{
						percentSessions = (double) 100 * statGoodSessions / (statGoodSessions + statBadSessions);
						cbuffer.Format( _T("%s: %s") , GetResString(IDS_STATS_AVGDATADLSES) , CastItoXBytes( bEmuleIn / statGoodSessions, false, false ) ); 
					}
					else 
						cbuffer.Format( _T("%s: %s") , GetResString(IDS_STATS_AVGDATADLSES) , CastItoXBytes((uint_ptr)0, false, false) );
					stattree.SetItemText( down_ssessions[2] , cbuffer ); // Set Avg DL/Session
					cbuffer.Format( _T("%s: %u (%1.1f%%)") , GetResString(IDS_STATS_SDLSES) , statGoodSessions , percentSessions );
					// Maella -Download Stop Reason-
					if(thePrefs.GetVerbose() == true){
                        CString details;
						details.Format(_T(" (active: %u, paused: %u, no needed part: %u, timeout: %u, socket: %u, out of part: %u, exception: %u, others: %u)"), 
									   myStats.a[1],
									   CUpDownClient::GetDownStopCount(false, CUpDownClient::DSR_PAUSED),
									   CUpDownClient::GetDownStopCount(false, CUpDownClient::DSR_NONEEDEDPARTS),
									   //CUpDownClient::GetDownStopCount(false, CUpDownClient::DSR_CORRUPTEDBLOCK),
									   CUpDownClient::GetDownStopCount(false, CUpDownClient::DSR_TIMEOUT),
									   CUpDownClient::GetDownStopCount(false, CUpDownClient::DSR_SOCKET),
									   CUpDownClient::GetDownStopCount(false, CUpDownClient::DSR_OUTOFPART),
									   CUpDownClient::GetDownStopCount(false, CUpDownClient::DSR_EXCEPTION),
									   thePrefs.GetDownS_SuccessfulSessions() - 
									   CUpDownClient::GetDownStopCount(false, CUpDownClient::DSR_PAUSED) -
									   CUpDownClient::GetDownStopCount(false, CUpDownClient::DSR_NONEEDEDPARTS) -
									   CUpDownClient::GetDownStopCount(false, CUpDownClient::DSR_CORRUPTEDBLOCK) -
									   CUpDownClient::GetDownStopCount(false, CUpDownClient::DSR_TIMEOUT) -
									   CUpDownClient::GetDownStopCount(false, CUpDownClient::DSR_SOCKET) -
									   CUpDownClient::GetDownStopCount(false, CUpDownClient::DSR_OUTOFPART) -
									   CUpDownClient::GetDownStopCount(false, CUpDownClient::DSR_EXCEPTION));
                        cbuffer += details;
					}
					// Maella end
					
					stattree.SetItemText( down_ssessions[0] , cbuffer ); // Set Succ Sessions
					// Set Failed Download Sessions (Avoid Division)
					if (percentSessions != 0 && statBadSessions > 0) 
						percentSessions = 100 - percentSessions; // There were some good sessions and bad ones...
					else if (percentSessions == 0 && statBadSessions > 0) 
						percentSessions = 100; // There were bad sessions and no good ones, must be 100%
					else 
						percentSessions = 0; // No sessions at all, or no bad ones.
					cbuffer.Format( _T("%s: %u (%1.1f%%)") , GetResString(IDS_STATS_FDLSES) , statBadSessions , percentSessions );
					// Maella -Download Stop Reason-
					if(thePrefs.GetVerbose() == true){
                        CString details;
						details.Format(_T(" (paused: %u, no needed part: %u, timeout: %u, socket: %u, out of part: %u, exception: %u, others: %u)"),
									   CUpDownClient::GetDownStopCount(true, CUpDownClient::DSR_PAUSED),
									   CUpDownClient::GetDownStopCount(true, CUpDownClient::DSR_NONEEDEDPARTS),
									   //CUpDownClient::GetDownStopCount(true, CUpDownClient::DSR_CORRUPTEDBLOCK),
									   CUpDownClient::GetDownStopCount(true, CUpDownClient::DSR_TIMEOUT),
									   CUpDownClient::GetDownStopCount(true, CUpDownClient::DSR_SOCKET),
									   CUpDownClient::GetDownStopCount(true, CUpDownClient::DSR_OUTOFPART),
									   CUpDownClient::GetDownStopCount(true, CUpDownClient::DSR_EXCEPTION),
									   statBadSessions - 
									   CUpDownClient::GetDownStopCount(true, CUpDownClient::DSR_PAUSED) -
									   CUpDownClient::GetDownStopCount(true, CUpDownClient::DSR_NONEEDEDPARTS) -
									   CUpDownClient::GetDownStopCount(true, CUpDownClient::DSR_CORRUPTEDBLOCK) -
									   CUpDownClient::GetDownStopCount(true, CUpDownClient::DSR_TIMEOUT) -
									   CUpDownClient::GetDownStopCount(true, CUpDownClient::DSR_SOCKET) -
									   CUpDownClient::GetDownStopCount(true, CUpDownClient::DSR_OUTOFPART) -
									   CUpDownClient::GetDownStopCount(true, CUpDownClient::DSR_EXCEPTION));
                        cbuffer += details;
					}
					// Maella end					
					stattree.SetItemText( down_ssessions[1] , cbuffer );
					// Set Average Download Time
					cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_AVGDLTIME), CastSecondsToLngHM(thePrefs.GetDownS_AvgTime()));
					stattree.SetItemText( down_ssessions[3] , cbuffer );
				}
				// Set Gain Due To Compression
				cbuffer.Format(GetResString(IDS_STATS_GAINCOMP) + _T(" (%.1f%%)"), CastItoXBytes(thePrefs.GetSesSavedFromCompression(), false, false), bEmuleIn!=0 ? (thePrefs.GetSesSavedFromCompression() * 100.0 / bEmuleIn) : 0.0);
				stattree.SetItemText( down_S[5] , cbuffer );
				// Set Lost Due To Corruption
				cbuffer.Format(GetResString(IDS_STATS_LOSTCORRUPT) + _T(" (%.1f%%)"), CastItoXBytes(thePrefs.GetSesLostFromCorruption(), false, false), bEmuleIn!=0 ? (thePrefs.GetSesLostFromCorruption() * 100.0 / bEmuleIn) : 0.0);
				stattree.SetItemText( down_S[6] , cbuffer );
				// Set Parts Saved Due To ICH
				cbuffer.Format(GetResString(IDS_STATS_ICHSAVED), thePrefs.GetSesPartsSavedByICH());
				stattree.SetItemText( down_S[7] , cbuffer );

				// Calculate downline OH totals
				DownOHTotal =	theStats.GetDownDataOverheadFileRequest() + 
								theStats.GetDownDataOverheadSourceExchange() + 
								theStats.GetDownDataOverheadServer() + 
								theStats.GetDownDataOverheadKad() + 
								theStats.GetDownDataOverheadOther();
				DownOHTotalPackets = 
								theStats.GetDownDataOverheadFileRequestPackets() + 
								theStats.GetDownDataOverheadSourceExchangePackets() + 
								theStats.GetDownDataOverheadServerPackets() + 
								theStats.GetDownDataOverheadKadPackets() + 
								theStats.GetDownDataOverheadOtherPackets();

				// Downline Overhead
				cbuffer.Format( GetResString( IDS_TOVERHEAD ) , CastItoXBytes( DownOHTotal, false, false ) , CastItoIShort( DownOHTotalPackets ) );
				stattree.SetItemText( hdown_soh , cbuffer );
				if (forceUpdate || stattree.IsExpanded(hdown_soh)) 
				{
					int i = 0;
					// Set down session file req OH
					cbuffer.Format( GetResString( IDS_FROVERHEAD ) , CastItoXBytes( theStats.GetDownDataOverheadFileRequest(), false, false ) , CastItoIShort( theStats.GetDownDataOverheadFileRequestPackets() ) );
					stattree.SetItemText( down_soh[i] , cbuffer );
					i++;
					// Set down session source exch OH
					cbuffer.Format( GetResString( IDS_SSOVERHEAD ) , CastItoXBytes( theStats.GetDownDataOverheadSourceExchange(), false, false ), CastItoIShort( theStats.GetDownDataOverheadSourceExchangePackets() ) );
					stattree.SetItemText( down_soh[i] , cbuffer );
					i++;
					// Set down session server OH
					cbuffer.Format(GetResString(IDS_SOVERHEAD),
								   CastItoXBytes(theStats.GetDownDataOverheadServer(), false, false), 
								   CastItoIShort(theStats.GetDownDataOverheadServerPackets()));
					stattree.SetItemText( down_soh[i] , cbuffer );
					i++;
					// Set down session Kad OH
					cbuffer.Format(GetResString(IDS_KADOVERHEAD),
								   CastItoXBytes(theStats.GetDownDataOverheadKad(), false, false), 
								   CastItoIShort(theStats.GetDownDataOverheadKadPackets()));
					stattree.SetItemText( down_soh[i] , cbuffer );
					i++;
					//Xman bandwidthcontrol:  count obfuscation data
					cbuffer.Format(_T("Obfuscation: %s"), CastItoXBytes(theApp.pBandWidthControl->GeteMuleInObfuscation(),false,false));
					stattree.SetItemText(down_soh[i], cbuffer);
					i++;
					//Xman end
				}
			}
			// TRANSFER -> DOWNLOADS -> CUMULATIVE SECTION
			if (forceUpdate || stattree.IsExpanded(h_down_total)) 
			{
				// Downloaded Data
				uint64 ullCumReceived = bEmuleIn + thePrefs.GetTotalDownloaded();
				cbuffer.Format(GetResString(IDS_STATS_DDATA), CastItoXBytes(ullCumReceived, false, false));
				stattree.SetItemText(down_T[0], cbuffer);
				if (forceUpdate || stattree.IsExpanded(down_T[0])) 
				{
					// Downloaded Data By Client
					if (forceUpdate || stattree.IsExpanded(hdown_tcb)) 
					{
						uint64 DownDataTotal = thePrefs.GetDownTotalClientData();
						uint64 DownDataClient;
						double percentClientTransferred;
						for(INT_PTR i=0;i<DOWN_SOFT_COUNT;i++){
							DownDataClient = thePrefs.GetCumDownData((ESoft)i);
							if ( DownDataTotal!=0 && DownDataClient!=0 )
								percentClientTransferred = (double) 100 * DownDataClient / DownDataTotal;
							else
								percentClientTransferred = 0;
							cbuffer.Format( softString[i] , CastItoXBytes( DownDataClient, false, false ) , percentClientTransferred);
							stattree.SetItemText( down_tcb[i] , cbuffer );
						}
					}
					// Downloaded Data By Port
					if (forceUpdate || stattree.IsExpanded(hdown_tpb)) 
					{
						uint64	PortDataTotal =		thePrefs.GetDownTotalPortData();
						uint64	PortData;
						double	percentPortTransferred;
						for(INT_PTR i=0;i<PORT_COUNT;i++){
							PortData =	thePrefs.GetCumDownDataPort((EPort)i);
							if ( PortDataTotal!=0 && PortData!=0 )
								percentPortTransferred = (double) 100 * PortData / PortDataTotal;
							else
								percentPortTransferred = 0;
							cbuffer.Format( _T("%s: %s (%1.1f%%)") , GetResString(portStrID[i]) , CastItoXBytes( PortData, false, false ) , percentPortTransferred);
							stattree.SetItemText( down_tpb[i] , cbuffer );
						}
					}
				}
				// Set Cum Completed Downloads
				cbuffer.Format(_T("%s: %u"), GetResString(IDS_STATS_COMPDL), thePrefs.GetDownCompletedFiles() );
				stattree.SetItemText(down_T[1], cbuffer);
				// Set Cum Download Sessions
				statGoodSessions = thePrefs.GetDownC_SuccessfulSessions() + myStats.a[1]; // Need to reset these from the session section.  Declared up there.
				statBadSessions = thePrefs.GetDownC_FailedSessions(); // ^^^^^^^^^^^^^^
				cbuffer.Format(_T("%s: %u"), GetResString(IDS_STATS_DLSES), statGoodSessions+statBadSessions );
				stattree.SetItemText(down_T[2], cbuffer);
				if (forceUpdate || stattree.IsExpanded(down_T[2])) 
				{
					// Set Cum Successful Download Sessions & Cum Average Download Per Sessions (Save an if-else statement)
					if (statGoodSessions > 0) 
					{
						percentSessions = (double) 100 * statGoodSessions / (statGoodSessions + statBadSessions);
						cbuffer.Format( _T("%s: %s") , GetResString(IDS_STATS_AVGDATADLSES), CastItoXBytes(ullCumReceived / statGoodSessions, false, false));
					}
					else 
					{
						percentSessions = 0;
						cbuffer.Format( _T("%s: %s") , GetResString(IDS_STATS_AVGDATADLSES) , CastItoXBytes((uint_ptr)0, false, false) );
					}
					stattree.SetItemText( down_tsessions[2] , cbuffer ); // Set Avg DL/Session
					cbuffer.Format( _T("%s: %u (%1.1f%%)"), GetResString(IDS_STATS_SDLSES) , statGoodSessions , percentSessions );
					stattree.SetItemText( down_tsessions[0] , cbuffer ); // Set Successful Sessions
					// Set Cum Failed Download Sessions
					if (percentSessions != 0 && statBadSessions > 0) 
						percentSessions = 100 - percentSessions; // There were some good sessions and bad ones...
					else if (percentSessions == 0 && statBadSessions > 0) 
						percentSessions = 100; // There were bad sessions and no good ones, must be 100%
					else 
						percentSessions = 0; // No sessions at all, or no bad ones.
					cbuffer.Format( _T("%s: %u (%1.1f%%)") , GetResString(IDS_STATS_FDLSES) , statBadSessions , percentSessions);
					stattree.SetItemText( down_tsessions[1] , cbuffer );
					// Set Cumulative Average Download Time
					uint32 avgDownTime = thePrefs.GetDownS_AvgTime();
					if (thePrefs.GetDownC_AvgTime()<=0) 
						thePrefs.SetDownCAvgTime(avgDownTime);
					avgDownTime = (uint32) (avgDownTime+thePrefs.GetDownC_AvgTime())/2;
					cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_AVGDLTIME), CastSecondsToLngHM(avgDownTime));
					stattree.SetItemText(down_tsessions[3], cbuffer);
				}
				// Set Cumulative Gained Due To Compression
				uint64 ullCumCompressed = thePrefs.GetSesSavedFromCompression() + thePrefs.GetCumSavedFromCompression();
				cbuffer.Format(GetResString(IDS_STATS_GAINCOMP) + _T(" (%.1f%%)"), CastItoXBytes(ullCumCompressed, false, false), ullCumReceived!=0 ? (ullCumCompressed * 100.0 / ullCumReceived) : 0.0);
				stattree.SetItemText( down_T[3] , cbuffer );
				// Set Cumulative Lost Due To Corruption
				uint64 ullCumCorrupted = thePrefs.GetSesLostFromCorruption() + thePrefs.GetCumLostFromCorruption();
				cbuffer.Format(GetResString(IDS_STATS_LOSTCORRUPT) + _T(" (%.1f%%)"), CastItoXBytes(ullCumCorrupted, false, false), ullCumReceived!=0 ? (ullCumCorrupted * 100.0 / ullCumReceived) : 0.0);
				stattree.SetItemText( down_T[4] , cbuffer );
				// Set Cumulative Saved Due To ICH
				cbuffer.Format(GetResString(IDS_STATS_ICHSAVED), thePrefs.GetSesPartsSavedByICH() + thePrefs.GetCumPartsSavedByICH());
				stattree.SetItemText( down_T[5] , cbuffer );

				if (DownOHTotal == 0 || DownOHTotalPackets == 0) 
				{
					DownOHTotal =	theStats.GetDownDataOverheadFileRequest() + 
									theStats.GetDownDataOverheadSourceExchange() + 
									theStats.GetDownDataOverheadServer() + 
									theStats.GetDownDataOverheadKad() + 
									theStats.GetDownDataOverheadOther();
					DownOHTotalPackets = 
									theStats.GetDownDataOverheadFileRequestPackets() + 
									theStats.GetDownDataOverheadSourceExchangePackets() + 
									theStats.GetDownDataOverheadServerPackets() + 
									theStats.GetDownDataOverheadKadPackets() + 
									theStats.GetDownDataOverheadOtherPackets();
				}
				// Total Overhead
				cbuffer.Format(GetResString(IDS_TOVERHEAD),CastItoXBytes(DownOHTotal + thePrefs.GetDownOverheadTotal(), false, false), CastItoIShort(DownOHTotalPackets + thePrefs.GetDownOverheadTotalPackets()));
				stattree.SetItemText(hdown_toh, cbuffer);
				if (forceUpdate || stattree.IsExpanded(hdown_toh)) 
				{
					int i = 0;
					// File Request Overhead
					cbuffer.Format(GetResString(IDS_FROVERHEAD), CastItoXBytes( theStats.GetDownDataOverheadFileRequest() + thePrefs.GetDownOverheadFileReq(), false, false), CastItoIShort(theStats.GetDownDataOverheadFileRequestPackets() + thePrefs.GetDownOverheadFileReqPackets()));
					stattree.SetItemText(down_toh[i], cbuffer);
					i++;
					// Source Exchange Overhead
					cbuffer.Format(GetResString(IDS_SSOVERHEAD), CastItoXBytes( theStats.GetDownDataOverheadSourceExchange()+thePrefs.GetDownOverheadSrcEx(), false, false), CastItoIShort(theStats.GetDownDataOverheadSourceExchangePackets()+thePrefs.GetDownOverheadSrcExPackets()));
					stattree.SetItemText(down_toh[i], cbuffer);
					i++;
					// Server Overhead
					cbuffer.Format(GetResString(IDS_SOVERHEAD), 
								   CastItoXBytes(theStats.GetDownDataOverheadServer() + 
												 thePrefs.GetDownOverheadServer(), false, false), 
								   CastItoIShort(theStats.GetDownDataOverheadServerPackets() +
												 thePrefs.GetDownOverheadServerPackets()));
					stattree.SetItemText(down_toh[i], cbuffer);
					i++;
					// Kad Overhead
					cbuffer.Format(GetResString(IDS_KADOVERHEAD), 
								   CastItoXBytes(theStats.GetDownDataOverheadKad() + 
												 thePrefs.GetDownOverheadKad(), false, false), 
								   CastItoIShort(theStats.GetDownDataOverheadKadPackets() +
												 thePrefs.GetDownOverheadKadPackets()));
					stattree.SetItemText(down_toh[i], cbuffer);
					i++;
				}
			} // - End Transfer -> Downloads -> Cumulative Section
		} // - End Transfer -> Downloads Section
		// TRANSFER-> UPLOADS SECTION
		if (forceUpdate || stattree.IsExpanded(h_upload)) 
		{
			uint64 UpOHTotal =			0;
			uint64 UpOHTotalPackets =	0;
			// TRANSFER -> UPLOADS -> SESSION SECTION
			if (forceUpdate || stattree.IsExpanded(h_up_session)) 
			{
				// Uploaded Data
				//Xman show complete internettraffic
				cbuffer.Format(GetResString(IDS_STATS_UDATA) + _T(" / %s")  ,CastItoXBytes(bEmuleOut, false, false),CastItoXBytes(theApp.pBandWidthControl->GetSessionNetworkOut(), false, false));
				//Xman end
				stattree.SetItemText(up_S[0], cbuffer);
				if (forceUpdate || stattree.IsExpanded(up_S[0])) 
				{
					// Uploaded Data By Client
					if (forceUpdate || stattree.IsExpanded(hup_scb)) 
					{
						uint64 UpDataTotal =		thePrefs.GetUpSessionClientData();
						uint64 UpDataClient;
						double percentClientTransferred;
						for(INT_PTR i=0;i<UP_SOFT_COUNT;i++){
							UpDataClient = thePrefs.sesUpData[i];
							if ( UpDataTotal!=0 && UpDataClient!=0 )
								percentClientTransferred = (double) 100 * UpDataClient / UpDataTotal;
							else
								percentClientTransferred = 0;
							cbuffer.Format( softString[i] , CastItoXBytes( UpDataClient, false, false ) , percentClientTransferred );
							stattree.SetItemText( up_scb[i] , cbuffer );
						}
					}
					// Uploaded Data By Port
					if (forceUpdate || stattree.IsExpanded(hup_spb)) 
					{
						uint64	PortDataTotal =		thePrefs.GetUpSessionPortData();
						uint64	PortData;
						double	percentPortTransferred;
						for(INT_PTR i=0;i<PORT_COUNT;i++){
							PortData =	thePrefs.sesUpDataPort[i];
							if ( PortDataTotal!=0 && PortData!=0 )
								percentPortTransferred = (double) 100 * PortData / PortDataTotal;
							else
								percentPortTransferred = 0;
							cbuffer.Format( _T("%s: %s (%1.1f%%)") , GetResString(portStrID[i]) , CastItoXBytes( PortData, false, false ) , percentPortTransferred);
							stattree.SetItemText( up_spb[i] , cbuffer );
						}
					}
					// Uploaded Data By Source
					if (forceUpdate || stattree.IsExpanded(hup_ssb)) 
					{
						int i = 0;
						uint64	DataSourceFile =	thePrefs.GetUpData_File();
						uint64	DataSourcePF =		thePrefs.GetUpData_Partfile();
						uint64	DataSourceTotal =		thePrefs.GetUpSessionDataFile();
						double	percentFileTransferred = 0;

						if ( DataSourceTotal!=0 && DataSourceFile!=0 )
							percentFileTransferred = (double) 100 * DataSourceFile / DataSourceTotal;
						cbuffer.Format( _T("%s: %s (%1.1f%%)") , GetResString(IDS_STATS_DSFILE) , CastItoXBytes( DataSourceFile, false, false ) , percentFileTransferred);
						stattree.SetItemText( up_ssb[i] , cbuffer );
						i++;

						if ( DataSourceTotal!=0 && DataSourcePF!=0 )
							percentFileTransferred = (double) 100 * DataSourcePF / DataSourceTotal;
						else
							percentFileTransferred = 0;
						cbuffer.Format( _T("%s: %s (%1.1f%%)") , GetResString(IDS_STATS_DSPF) , CastItoXBytes( DataSourcePF, false, false ) , percentFileTransferred);
						stattree.SetItemText( up_ssb[i] , cbuffer );
						i++;
					}
				}

                // Amount of data uploaded to friends
				cbuffer.Format(GetResString(IDS_STATS_UDATA_FRIENDS),CastItoXBytes(theStats.sessionSentBytesToFriend, false, false));
				stattree.SetItemText(up_S[1], cbuffer);

				// Set fully Active Uploads
				cbuffer.Format(GetResString(IDS_STATS_ACTUL_ZZ),theApp.uploadBandwidthThrottler->GetNumberOfFullyActivatedSlots()); //theApp.uploadqueue->GetUploadQueueLength()
				stattree.SetItemText(up_S[2], cbuffer);

                // Set Set Total Uploads                
				cbuffer.Format(GetResString(IDS_STATS_TOTALUL),theApp.uploadqueue->GetUploadQueueLength());
				stattree.SetItemText(up_S[3], cbuffer);

				// Set Queue Length
				cbuffer.Format(GetResString(IDS_STATS_WAITINGUSERS),theApp.uploadqueue->GetWaitingUserCount());
				stattree.SetItemText(up_S[4], cbuffer);

				// Set Upload Sessions
				statGoodSessions = (uint32)(theApp.uploadqueue->GetSuccessfullUpCount() + theApp.uploadqueue->GetUploadQueueLength());
				statBadSessions = theApp.uploadqueue->GetFailedUpCount();
				cbuffer.Format(_T("%s: %u"), GetResString(IDS_STATS_ULSES), statGoodSessions + statBadSessions);
				stattree.SetItemText(up_S[5], cbuffer);
				if (forceUpdate || stattree.IsExpanded(up_S[5])) 
				{
					// Set Successful Upload Sessions & Average Uploaded Per Session
					if (statGoodSessions>0) 
					{ // Blackholes are when God divided by 0
						percentSessions = (double) 100*statGoodSessions/(statGoodSessions+statBadSessions);
						cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_AVGDATAULSES), CastItoXBytes( bEmuleOut / statGoodSessions, false, false) ); 
					}
					else 
					{
						percentSessions = 0;
						cbuffer.Format( _T("%s: %s") , GetResString(IDS_STATS_AVGDATAULSES) , GetResString(IDS_FSTAT_WAITING) );
					}
					stattree.SetItemText(up_ssessions[2], cbuffer);
					cbuffer.Format(GetResString(IDS_STATS_SUCCUPCOUNT),statGoodSessions,percentSessions);
					// Maella -Upload Stop Reason-
					if(thePrefs.GetVerbose() == true){
                        CString details;
						details.Format(_T(" (active: %u, socket: %u, completed: %u, cancelled/ended: %u, different file: %u, exception: %u,blocking: %u, others: %u)"),
									   theApp.uploadqueue->GetUploadQueueLength(),
									   CUpDownClient::GetUpStopCount(false, CUpDownClient::USR_SOCKET),
									   CUpDownClient::GetUpStopCount(false, CUpDownClient::USR_COMPLETEDRANSFER),
									   CUpDownClient::GetUpStopCount(false, CUpDownClient::USR_CANCELLED),
									   CUpDownClient::GetUpStopCount(false, CUpDownClient::USR_DIFFERENT_FILE),
									   CUpDownClient::GetUpStopCount(false, CUpDownClient::USR_EXCEPTION),
									   CUpDownClient::GetUpStopCount(false, CUpDownClient::USR_BLOCKING),
									   theApp.uploadqueue->GetSuccessfullUpCount() - 
									   CUpDownClient::GetUpStopCount(false, CUpDownClient::USR_SOCKET) -
									   CUpDownClient::GetUpStopCount(false, CUpDownClient::USR_COMPLETEDRANSFER) -
									   CUpDownClient::GetUpStopCount(false, CUpDownClient::USR_CANCELLED) -
									   CUpDownClient::GetUpStopCount(false, CUpDownClient::USR_DIFFERENT_FILE) -
									   CUpDownClient::GetUpStopCount(false, CUpDownClient::USR_EXCEPTION) -
									   CUpDownClient::GetUpStopCount(false, CUpDownClient::USR_BLOCKING));
                        cbuffer += details;
					}
					// Maella end
					stattree.SetItemText(up_ssessions[0], cbuffer);
					// Set Failed Upload Sessions
					if (percentSessions != 0 && statBadSessions > 0) 
						percentSessions = 100 - percentSessions; // There were some good sessions and bad ones...
					else if (percentSessions == 0 && statBadSessions > 0) 
						percentSessions = 100; // There were bad sessions and no good ones, must be 100%
					else 
						percentSessions = 0; // No sessions at all, or no bad ones.
					cbuffer.Format(GetResString(IDS_STATS_FAILUPCOUNT),statBadSessions,percentSessions);
					// Maella -Upload Stop Reason-
					if(thePrefs.GetVerbose() == true){
						CString details;
						details.Format(_T(" (socket: %u, completed: %u, cancelled/ended: %u, different file: %u, exception: %u,blocking: %u, others: %u)"),
							CUpDownClient::GetUpStopCount(true, CUpDownClient::USR_SOCKET),
							CUpDownClient::GetUpStopCount(true, CUpDownClient::USR_COMPLETEDRANSFER),
							CUpDownClient::GetUpStopCount(true, CUpDownClient::USR_CANCELLED),
							CUpDownClient::GetUpStopCount(true, CUpDownClient::USR_DIFFERENT_FILE),
							CUpDownClient::GetUpStopCount(true, CUpDownClient::USR_EXCEPTION),
							CUpDownClient::GetUpStopCount(true, CUpDownClient::USR_BLOCKING),
							statBadSessions - 
							CUpDownClient::GetUpStopCount(true, CUpDownClient::USR_SOCKET) -
							CUpDownClient::GetUpStopCount(true, CUpDownClient::USR_COMPLETEDRANSFER) -
							CUpDownClient::GetUpStopCount(true, CUpDownClient::USR_CANCELLED) -
							CUpDownClient::GetUpStopCount(true, CUpDownClient::USR_DIFFERENT_FILE) -
							CUpDownClient::GetUpStopCount(true, CUpDownClient::USR_EXCEPTION) -
							CUpDownClient::GetUpStopCount(true, CUpDownClient::USR_BLOCKING));
						cbuffer += details;
					}
					// Maella end
					stattree.SetItemText(up_ssessions[1], cbuffer);
					// Set Avg Upload time
					//DWORD running=theApp.uploadqueue->GetAverageUpTime();
					cbuffer.Format(GetResString(IDS_STATS_AVEUPTIME),CastSecondsToLngHM(theApp.uploadqueue->GetAverageUpTime()));
					stattree.SetItemText(up_ssessions[3], cbuffer);
				}
				// Calculate Upline OH Totals
				UpOHTotal = theStats.GetUpDataOverheadFileRequest() + 
							theStats.GetUpDataOverheadSourceExchange() + 
							theStats.GetUpDataOverheadServer() + 
							theStats.GetUpDataOverheadKad() + 
							theStats.GetUpDataOverheadOther();
				UpOHTotalPackets = theStats.GetUpDataOverheadFileRequestPackets() + 
							theStats.GetUpDataOverheadSourceExchangePackets() + 
							theStats.GetUpDataOverheadServerPackets() + 
							theStats.GetUpDataOverheadKadPackets() + 
							theStats.GetUpDataOverheadOtherPackets();
				// Total Upline Overhead
				cbuffer.Format(GetResString(IDS_TOVERHEAD), CastItoXBytes( UpOHTotal, false, false ), CastItoIShort(UpOHTotalPackets));
				stattree.SetItemText(hup_soh, cbuffer);
				if (forceUpdate || stattree.IsExpanded(hup_soh)) 
				{
					int i = 0;
					// File Request Overhead
					cbuffer.Format(GetResString(IDS_FROVERHEAD), CastItoXBytes( theStats.GetUpDataOverheadFileRequest(), false, false ), CastItoIShort(theStats.GetUpDataOverheadFileRequestPackets()));
					stattree.SetItemText(up_soh[i], cbuffer);
					i++;
					// Source Exchanged Overhead
					cbuffer.Format(GetResString(IDS_SSOVERHEAD), CastItoXBytes( theStats.GetUpDataOverheadSourceExchange(), false, false ), CastItoIShort(theStats.GetUpDataOverheadSourceExchangePackets()));
					stattree.SetItemText(up_soh[i], cbuffer);
					i++;
					// Server Overhead
					cbuffer.Format(GetResString(IDS_SOVERHEAD), 
								   CastItoXBytes(theStats.GetUpDataOverheadServer(), false, false), 
								   CastItoIShort(theStats.GetUpDataOverheadServerPackets()));
					stattree.SetItemText(up_soh[i], cbuffer);
					i++;
					// Kad Overhead
					cbuffer.Format(GetResString(IDS_KADOVERHEAD), 
								   CastItoXBytes(theStats.GetUpDataOverheadKad(), false, false), 
								   CastItoIShort(theStats.GetUpDataOverheadKadPackets()));
					stattree.SetItemText(up_soh[i], cbuffer);
					i++;
					//Xman bandwidthcontrol:  count obfuscation data
					cbuffer.Format(_T("Obfuscation: %s"), CastItoXBytes(theApp.pBandWidthControl->GeteMuleOutObfuscation(),false,false));
					stattree.SetItemText(up_soh[i], cbuffer);
					i++;
					//Xman end
				}
			} // - End Transfer -> Uploads -> Session Section
			// TRANSFER -> UPLOADS -> CUMULATIVE SECTION
			if (forceUpdate || stattree.IsExpanded(h_up_total)) 
			{
				// Uploaded Data
				cbuffer.Format(GetResString(IDS_STATS_UDATA),CastItoXBytes( bEmuleOut+thePrefs.GetTotalUploaded(), false, false));
				stattree.SetItemText(up_T[0],cbuffer);
				if (forceUpdate || stattree.IsExpanded(up_T[0])) 
				{
					// Uploaded Data By Client
					if (forceUpdate || stattree.IsExpanded(hup_tcb)) 
					{
						uint64 UpDataTotal =		thePrefs.GetUpTotalClientData();
						uint64 UpDataClient;
						double percentClientTransferred;
						for(INT_PTR i=0;i<UP_SOFT_COUNT;i++){
							UpDataClient = thePrefs.GetCumUpData((ESoft)i);
							if ( UpDataTotal!=0 && UpDataClient!=0 )
								percentClientTransferred = (double) 100 * UpDataClient / UpDataTotal;
							else
								percentClientTransferred = 0;
							cbuffer.Format( softString[i] , CastItoXBytes( UpDataClient, false, false ) , percentClientTransferred );
							stattree.SetItemText( up_tcb[i] , cbuffer );
						}
					}
					// Uploaded Data By Port
					if (forceUpdate || stattree.IsExpanded(hup_tpb)) 
					{
						uint64 PortDataTotal = thePrefs.GetUpTotalPortData();
						uint64 PortData;
						double percentPortTransferred;
						for(INT_PTR i=0;i<PORT_COUNT;i++){
							PortData =thePrefs.GetCumUpDataPort((EPort)i);
							if ( PortDataTotal!=0 && PortData!=0 )
								percentPortTransferred = (double) 100 * PortData / PortDataTotal;
							else
								percentPortTransferred = 0;
							cbuffer.Format( _T("%s: %s (%1.1f%%)") , GetResString(portStrID[i]) , CastItoXBytes( PortData, false, false ) , percentPortTransferred);
							stattree.SetItemText( up_tpb[i] , cbuffer );
						}
					}
					// Uploaded Data By Source
					if (forceUpdate || stattree.IsExpanded(hup_tsb)) 
					{
						int i = 0;
						uint64	DataSourceFile =	thePrefs.GetCumUpData_File();
						uint64	DataSourcePF =		thePrefs.GetCumUpData_Partfile();
						uint64	DataSourceTotal =	thePrefs.GetUpTotalDataFile();
						double	percentFileTransferred = 0;

						if ( DataSourceTotal!=0 && DataSourceFile!=0 )
							percentFileTransferred = (double) 100 * DataSourceFile / DataSourceTotal;
						cbuffer.Format( _T("%s: %s (%1.1f%%)") , GetResString(IDS_STATS_DSFILE) , CastItoXBytes( DataSourceFile, false, false ) , percentFileTransferred);
						stattree.SetItemText( up_tsb[i] , cbuffer );
						i++;

						if ( DataSourceTotal!=0 && DataSourcePF!=0 )
							percentFileTransferred = (double) 100 * DataSourcePF / DataSourceTotal;
						else
							percentFileTransferred = 0;
						cbuffer.Format( _T("%s: %s (%1.1f%%)") , GetResString(IDS_STATS_DSPF) , CastItoXBytes( DataSourcePF, false, false ) , percentFileTransferred);
						stattree.SetItemText( up_tsb[i] , cbuffer );
						i++;
					}
				}
				// Upload Sessions
				statGoodSessions = (uint32)(theApp.uploadqueue->GetSuccessfullUpCount() + thePrefs.GetUpSuccessfulSessions() + theApp.uploadqueue->GetUploadQueueLength());
				statBadSessions = theApp.uploadqueue->GetFailedUpCount() + thePrefs.GetUpFailedSessions();
				cbuffer.Format(_T("%s: %u"), GetResString(IDS_STATS_ULSES), statGoodSessions + statBadSessions);
				stattree.SetItemText(up_T[1], cbuffer);
				if (forceUpdate || stattree.IsExpanded(up_T[1])) 
				{
					// Set Successful Upload Sessions & Average Uploaded Per Session
					if (statGoodSessions>0) 
					{ // Blackholes are when God divided by 0
						percentSessions = (double) 100*statGoodSessions/(statGoodSessions+statBadSessions);
						cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_AVGDATAULSES), CastItoXBytes((bEmuleOut + thePrefs.GetTotalUploaded()) / statGoodSessions, false, false) ); 
					}
					else 
					{
						percentSessions = 0;
						cbuffer.Format( _T("%s: %s") , GetResString(IDS_STATS_AVGDATAULSES) , GetResString(IDS_FSTAT_WAITING) ); 
					}
					stattree.SetItemText(up_tsessions[2], cbuffer);
					cbuffer.Format(GetResString(IDS_STATS_SUCCUPCOUNT),statGoodSessions,percentSessions);
					stattree.SetItemText(up_tsessions[0], cbuffer);
					// Set Failed Upload Sessions
					if (percentSessions != 0 && statBadSessions > 0) 
						percentSessions = 100 - percentSessions; // There were some good sessions and bad ones...
					else if (percentSessions == 0 && statBadSessions > 0) 
						percentSessions = 100; // There were bad sessions and no good ones, must be 100%
					else 
						percentSessions = 0; // No sessions at all, or no bad ones.
					cbuffer.Format(GetResString(IDS_STATS_FAILUPCOUNT),statBadSessions,percentSessions);
					stattree.SetItemText(up_tsessions[1], cbuffer);
					// Set Avg Upload time
					uint32 avguptime = theApp.uploadqueue->GetAverageUpTime();
					if (thePrefs.GetUpAvgTime()<=0) 
						thePrefs.SetUpAvgTime(avguptime);
					avguptime = (uint32) (avguptime+thePrefs.GetUpAvgTime())/2;
					cbuffer.Format(GetResString(IDS_STATS_AVEUPTIME),CastSecondsToLngHM(avguptime));
					stattree.SetItemText(up_tsessions[3], cbuffer);
				}

				if (UpOHTotal == 0 || UpOHTotalPackets == 0) 
				{
					// Calculate Upline OH Totals
					UpOHTotal = theStats.GetUpDataOverheadFileRequest() + 
								theStats.GetUpDataOverheadSourceExchange() + 
								theStats.GetUpDataOverheadServer() + 
								theStats.GetUpDataOverheadKad() + 
								theStats.GetUpDataOverheadOther();
					UpOHTotalPackets = theStats.GetUpDataOverheadFileRequestPackets() + 
								theStats.GetUpDataOverheadSourceExchangePackets() + 
								theStats.GetUpDataOverheadServerPackets() + 
								theStats.GetUpDataOverheadKadPackets() + 
								theStats.GetUpDataOverheadOtherPackets();
				}
				// Set Cumulative Total Overhead
				cbuffer.Format(GetResString(IDS_TOVERHEAD),CastItoXBytes(UpOHTotal + thePrefs.GetUpOverheadTotal(), false, false), CastItoIShort(UpOHTotalPackets + thePrefs.GetUpOverheadTotalPackets()));
				stattree.SetItemText(hup_toh, cbuffer);
				if (forceUpdate || stattree.IsExpanded(hup_toh)) 
				{
					int i = 0;
					// Set up total file req OH
					cbuffer.Format(GetResString(IDS_FROVERHEAD), CastItoXBytes( theStats.GetUpDataOverheadFileRequest() + thePrefs.GetUpOverheadFileReq(), false, false), CastItoIShort(theStats.GetUpDataOverheadFileRequestPackets() + thePrefs.GetUpOverheadFileReqPackets()));
					stattree.SetItemText(up_toh[i], cbuffer);
					i++;
					// Set up total source exch OH
					cbuffer.Format(GetResString(IDS_SSOVERHEAD), CastItoXBytes( theStats.GetUpDataOverheadSourceExchange()+thePrefs.GetUpOverheadSrcEx(), false, false), CastItoIShort(theStats.GetUpDataOverheadSourceExchangePackets()+thePrefs.GetUpOverheadSrcExPackets()));
					stattree.SetItemText(up_toh[i], cbuffer);
					i++;
					// Set up total server OH
					cbuffer.Format(GetResString(IDS_SOVERHEAD), 
								   CastItoXBytes(theStats.GetUpDataOverheadServer()
												 + thePrefs.GetUpOverheadServer(), false, false), 
								   CastItoIShort(theStats.GetUpDataOverheadServerPackets()
												 + thePrefs.GetUpOverheadServerPackets()));
					stattree.SetItemText(up_toh[i], cbuffer);
					i++;
					// Set up total Kad OH
					cbuffer.Format(GetResString(IDS_KADOVERHEAD), 
								   CastItoXBytes(theStats.GetUpDataOverheadKad() + 
											     thePrefs.GetUpOverheadKad(), false, false), 
								   CastItoIShort(theStats.GetUpDataOverheadKadPackets() + 
												 thePrefs.GetUpOverheadKadPackets()));
					stattree.SetItemText(up_toh[i], cbuffer);
					i++;
				}
			} // - End Transfer -> Uploads -> Cumulative Section
		} // - End Transfer -> Uploads Section
	} // - END TRANSFER SECTION

#ifdef CLIENTANALYZER
	if (forceUpdate || stattree.IsExpanded(h_Analyzer)) 
	{
		const CString strTypes[] = {
			L"Nickthieveries",
			L"Modthieveries",
			L"File fakes",
			L"UDP-FNF fakes",
			L"Fast asks",
			L"Spams", 
			L"FastXS",
			L"Mod fakes",
		};
		for(uint8 i = 0; i != _countof(strTypes); ++i)
		{
			cbuffer.Format(L"%s: %u", strTypes[i], theApp.antileechlist->GetBadActionCounter(i));
			stattree.SetItemText(h_BadActions[i], cbuffer);
		}
	}
#endif

	// CONNECTION SECTION
	if (forceUpdate || stattree.IsExpanded(h_connection)) 
	{		
		// CONNECTION -> SESSION SECTION
		if (forceUpdate || stattree.IsExpanded(h_conn_session)) 
		{			
			// CONNECTION -> SESSION -> GENERAL SECTION
			if (forceUpdate || stattree.IsExpanded(hconn_sg)) 
			{
				int i = 0;
				// Server Reconnects
				if (theStats.reconnects>0) 
					cbuffer.Format(GetResString(IDS_STATS_RECONNECTS),theStats.reconnects-1);
				else 
					cbuffer.Format(GetResString(IDS_STATS_RECONNECTS),0);
				stattree.SetItemText(conn_sg[i], cbuffer);
				i++;
				// Active Connections
				cbuffer.Format(_T("%s: %i (%s:%u | %s:%u | %s:%u)"),GetResString(IDS_SF_ACTIVECON),theApp.listensocket->GetActiveConnections(), GetResString(IDS_HALF), theApp.listensocket->GetTotalHalfCon(), GetResString(IDS_CONCOMPL) ,theApp.listensocket->GetTotalComp(), GetResString(IDS_STATS_PRTOTHER) ,theApp.listensocket->GetActiveConnections() - theApp.listensocket->GetTotalHalfCon() - theApp.listensocket->GetTotalComp());
				stattree.SetItemText(conn_sg[i], cbuffer);
				i++;
				// Average Connections
				cbuffer.Format(_T("%s: %i"),GetResString(IDS_SF_AVGCON),(int)theApp.listensocket->GetAverageConnections());
				stattree.SetItemText(conn_sg[i], cbuffer);
				i++;
				// Peak Connections
				cbuffer.Format(_T("%s: %i"),GetResString(IDS_SF_PEAKCON),theApp.listensocket->GetPeakConnections());
				stattree.SetItemText(conn_sg[i], cbuffer);
				i++;
				// Connect Limit Reached
				uint32 m_itemp = theApp.listensocket->GetMaxConnectionReached();
				if( m_itemp != m_ilastMaxConnReached )
				{
					cbuffer.Format(_T("%s: %i : %s"), GetResString(IDS_SF_MAXCONLIMITREACHED), m_itemp, CTime::GetCurrentTime().Format(_T("%c")));
					stattree.SetItemText(conn_sg[i], cbuffer);
					m_ilastMaxConnReached = m_itemp;
				}
				else if( m_itemp == 0 )
				{
					cbuffer.Format(_T("%s: %i"),GetResString(IDS_SF_MAXCONLIMITREACHED),m_itemp);
					stattree.SetItemText(conn_sg[i], cbuffer);
				}
				i++;
			} // - End Connection -> Session -> General Section
			// CONNECTION -> SESSION -> UPLOADS SECTION
			if (forceUpdate || stattree.IsExpanded(hconn_su)) 
			{
				int i = 0;
				// Upload Rate
				cbuffer.Format(_T("%s: %s"), GetResString(IDS_ST_UPLOAD),CastItoXBytes(theStats.currentUploadRate, true, true)); // Xman
				stattree.SetItemText(conn_su[i], cbuffer);
				i++;
				// Average Upload Rate
				cbuffer.Format(GetResString(IDS_STATS_AVGUL),CastItoXBytes(theStats.sessionUploadRate, true, true)); // Xman
				stattree.SetItemText(conn_su[i], cbuffer); 
				i++;
				// Max Upload Rate
				cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_MAXUL), CastItoXBytes(theStats.currentMaxUploadRate, true, true)); // Xman
				stattree.SetItemText(conn_su[i], cbuffer);
				i++;
				// Max Average Upload Rate								
				cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_MAXAVGUL), CastItoXBytes(theStats.sessionMaxUploadRate, true, true)); // Xman
				stattree.SetItemText(conn_su[i], cbuffer); 
				i++;
			} // - End Connection -> Session -> Uploads Section
			// CONNECTION -> SESSION -> DOWNLOADS SECTION
			if (forceUpdate || stattree.IsExpanded(hconn_sd)) 
			{
				int i = 0;
				// Download Rate
				cbuffer.Format(_T("%s: %s"), GetResString(IDS_ST_DOWNLOAD), CastItoXBytes(theStats.currentDownloadRate, true, true)); // Xman
				stattree.SetItemText(conn_sd[i], cbuffer); 
				i++;
				// Average Download Rate
				cbuffer.Format(GetResString(IDS_STATS_AVGDL),CastItoXBytes(theStats.sessionDownloadRate, true, true));	 // Xman
				stattree.SetItemText(conn_sd[i], cbuffer); 
				i++;
				// Max Download Rate
				cbuffer.Format(GetResString(IDS_STATS_MAXDL),CastItoXBytes(theStats.currentMaxDownloadRate, true, true)); // Xman
				stattree.SetItemText(conn_sd[i], cbuffer); 
				i++;
				// Max Average Download Rate								
				cbuffer.Format(GetResString(IDS_STATS_MAXAVGDL), CastItoXBytes(theStats.sessionMaxDownloadRate, true, true));	// Xman
				stattree.SetItemText(conn_sd[i], cbuffer);
				i++;
			} // - End Connection -> Session -> Downloads Section		
		} // - End Connection -> Session Section
		// CONNECTION -> CUMULATIVE SECTION
		if (forceUpdate || stattree.IsExpanded(h_conn_total)) 
		{			
			// CONNECTION -> CUMULATIVE -> GENERAL SECTION
			if (forceUpdate || stattree.IsExpanded(hconn_tg)) 
			{
				int i = 0;
				// Server Reconnects
				if(theStats.reconnects>0)
					cbuffer.Format(GetResString(IDS_STATS_RECONNECTS),theStats.reconnects - 1 + thePrefs.GetConnNumReconnects());
				else
					cbuffer.Format(GetResString(IDS_STATS_RECONNECTS),thePrefs.GetConnNumReconnects());
				stattree.SetItemText(conn_tg[i], cbuffer);
				i++;
				// Average Connections
				cbuffer.Format(_T("%s: %i"), GetResString(IDS_SF_AVGCON), (int) (theApp.listensocket->GetActiveConnections() + thePrefs.GetConnAvgConnections()) / 2 );
				stattree.SetItemText(conn_tg[i], cbuffer);
				i++;
				// Peak Connections
				cbuffer.Format(_T("%s: %i"), GetResString(IDS_SF_PEAKCON), thePrefs.GetConnPeakConnections());
				stattree.SetItemText(conn_tg[i], cbuffer);
				i++;
				// Connection Limit Reached
				cbuffer.Format(_T("%s: %i"), GetResString(IDS_SF_MAXCONLIMITREACHED), theApp.listensocket->GetMaxConnectionReached() + thePrefs.GetConnMaxConnLimitReached());
				stattree.SetItemText(conn_tg[i], cbuffer);
				i++;
			} // - End Connection -> Cumulative -> General Section
			// CONNECTION -> CUMULATIVE -> UPLOADS SECTION
			if (forceUpdate || stattree.IsExpanded(hconn_tu)) 
			{
				int i = 0;
				// Average Upload Rate
				cbuffer.Format(GetResString(IDS_STATS_AVGUL),CastItoXBytes(theStats.cumUpavg, true, true));
				stattree.SetItemText(conn_tu[i], cbuffer);
				i++;
				// Max Upload Rate
				cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_MAXUL), CastItoXBytes(theStats.maxcumUp, true, true));
				stattree.SetItemText(conn_tu[i], cbuffer);
				i++;
				// Max Average Upload Rate
				cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_MAXAVGUL), CastItoXBytes(theStats.maxcumUpavg, true, true));
				stattree.SetItemText(conn_tu[i], cbuffer);
				i++;
			} // - End Connection -> Cumulative -> Uploads Section
			// CONNECTION -> CUMULATIVE -> DOWNLOADS SECTION
			if (forceUpdate || stattree.IsExpanded(hconn_td)) 
			{
				int i = 0;
				// Average Download Rate
				cbuffer.Format(GetResString(IDS_STATS_AVGDL), CastItoXBytes(theStats.cumDownavg, true, true));
				stattree.SetItemText(conn_td[i], cbuffer);
				i++;
				// Max Download Rate
				cbuffer.Format(GetResString(IDS_STATS_MAXDL), CastItoXBytes(theStats.maxcumDown, true, true));
				stattree.SetItemText(conn_td[i], cbuffer);
				i++;
				// Max Average Download Rate
				cbuffer.Format(GetResString(IDS_STATS_MAXAVGDL), CastItoXBytes(theStats.maxcumDownavg, true, true));
				stattree.SetItemText(conn_td[i], cbuffer);
				i++;
			} // - End Connection -> Cumulative -> Downloads Section
		} // - End Connection -> Cumulative Section
	} // - END CONNECTION SECTION


	// TIME STATISTICS SECTION
	if (forceUpdate || stattree.IsExpanded(h_time)) 
	{
		// Statistics Last Reset
		cbuffer.Format(GetResString(IDS_STATS_LASTRESETSTATIC), thePrefs.GetStatsLastResetStr(false));
        stattree.SetItemText(tvitime[0], cbuffer);
		// Time Since Last Reset
		uint_ptr timeDiff;
		if (thePrefs.GetStatsLastResetLng()) 
		{
			timeDiff = (uint_ptr)(time(NULL) - thePrefs.GetStatsLastResetLng()); // In seconds
			cbuffer.Format(GetResString(IDS_STATS_TIMESINCERESET), CastSecondsToLngHM(timeDiff));
		}
		else 
		{
			timeDiff = 0;
			cbuffer.Format(GetResString(IDS_STATS_TIMESINCERESET), GetResString(IDS_UNKNOWN));
		}
		stattree.SetItemText(tvitime[1], cbuffer);
		// TIME STATISTICS -> SESSION SECTION
		if (forceUpdate || stattree.IsExpanded(htime_s)) 
		{
			int i = 0;
			// Run Time
			uint_ptr sessionRunTime = (GetTickCount() - theStats.starttime) / 1000;
			cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_RUNTIME), CastSecondsToLngHM(sessionRunTime));
			stattree.SetItemText(tvitime_s[i], cbuffer);
			i++;
			if (!sessionRunTime) 
				sessionRunTime = 1;
			// Transfer Time
			cbuffer.Format(_T("%s: %s (%1.1f%%)"), GetResString(IDS_STATS_TRANSTIME), CastSecondsToLngHM(theStats.GetTransferTime()), (double) (100 *  theStats.GetTransferTime() ) / sessionRunTime);
			stattree.SetItemText(tvitime_s[i], cbuffer);
			if (forceUpdate || stattree.IsExpanded(tvitime_s[i])) 
			{
				int x = 0;
				// Upload Time
				cbuffer.Format(_T("%s: %s (%1.1f%%)"), GetResString(IDS_STATS_UPTIME), CastSecondsToLngHM(theStats.GetUploadTime()), (double) (100 * theStats.GetUploadTime()) / sessionRunTime);
				stattree.SetItemText(tvitime_st[x], cbuffer);
				x++;
				// Download Time
				cbuffer.Format(_T("%s: %s (%1.1f%%)"), GetResString(IDS_STATS_DOWNTIME), CastSecondsToLngHM(theStats.GetDownloadTime()), (double) (100 * theStats.GetDownloadTime()) / sessionRunTime);
				stattree.SetItemText(tvitime_st[x], cbuffer);
				x++;
			}
			i++;
			// Current Server Duration				
			cbuffer.Format(_T("%s: %s (%1.1f%%)"), GetResString(IDS_STATS_CURRSRVDUR), CastSecondsToLngHM(theStats.time_thisServerDuration), (double) (100 * theStats.time_thisServerDuration) / sessionRunTime);
			stattree.SetItemText(tvitime_s[i], cbuffer);
			i++;
			// Total Server Duration
			cbuffer.Format(_T("%s: %s (%1.1f%%)"), GetResString(IDS_STATS_TOTALSRVDUR), CastSecondsToLngHM(theStats.GetServerDuration()), (double) (100 * theStats.GetServerDuration()) / sessionRunTime);
			stattree.SetItemText(tvitime_s[i], cbuffer);
			i++;
		}
		// TIME STATISTICS -> CUMULATIVE SECTION
		if (forceUpdate || stattree.IsExpanded(htime_t)) 
		{
			int i = 0;
			// Run Time
			uint_ptr totalRunTime = (uint_ptr)(((GetTickCount() - theStats.starttime)/1000) + thePrefs.GetConnRunTime());
			cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_RUNTIME), CastSecondsToLngHM(totalRunTime));
			stattree.SetItemText(tvitime_t[i], cbuffer);
			i++;
			if (!totalRunTime) 
				totalRunTime = 1;
			// Transfer Time
			cbuffer.Format(_T("%s: %s (%1.1f%%)"), GetResString(IDS_STATS_TRANSTIME), CastSecondsToLngHM(theStats.GetTransferTime() + thePrefs.GetConnTransferTime()), (double) (100 * (theStats.GetTransferTime() + thePrefs.GetConnTransferTime())) / totalRunTime);
			stattree.SetItemText(tvitime_t[i], cbuffer);
			if (forceUpdate || stattree.IsExpanded(tvitime_t[i])) 
			{
				int x = 0;
				// Upload Time
				cbuffer.Format(_T("%s: %s (%1.1f%%)"), GetResString(IDS_STATS_UPTIME), CastSecondsToLngHM(theStats.GetUploadTime() + thePrefs.GetConnUploadTime()), (double) (100 * (theStats.GetUploadTime() + thePrefs.GetConnUploadTime())) / totalRunTime);
				stattree.SetItemText(tvitime_tt[x], cbuffer);
				x++;
				// Download Time
				cbuffer.Format(_T("%s: %s (%1.1f%%)"), GetResString(IDS_STATS_DOWNTIME), CastSecondsToLngHM(theStats.GetDownloadTime() + thePrefs.GetConnDownloadTime()), (double) (100 * (theStats.GetDownloadTime() + thePrefs.GetConnDownloadTime())) / totalRunTime);
				stattree.SetItemText(tvitime_tt[x], cbuffer);
				x++;
			} 
			i++;
			// Total Server Duration
			cbuffer.Format(_T("%s: %s (%1.1f%%)"), GetResString(IDS_STATS_TOTALSRVDUR), CastSecondsToLngHM(theStats.GetServerDuration() + thePrefs.GetConnServerDuration()), (double) (100 * (theStats.GetServerDuration() + thePrefs.GetConnServerDuration())) / totalRunTime);
			stattree.SetItemText(tvitime_t[i], cbuffer);
			i++;
		}
		// TIME STATISTICS -> PROJECTED AVERAGES SECTION
		if ( (forceUpdate || stattree.IsExpanded(htime_aap)) && timeDiff > 0 ) 
		{
			double avgModifier[3];
			avgModifier[0] = (double) 86400 / timeDiff; // Days
			avgModifier[1] = (double) 2628000 / timeDiff; // Months
			avgModifier[2] = (double) 31536000 / timeDiff; // Years
			// TIME STATISTICS -> PROJECTED AVERAGES -> TIME PERIODS
			// This section is completely scalable.  Might add "Week" to it in the future.
			// For each time period that we are calculating a projected average for...
			for (int mx = 0; mx<3; mx++) 
			{
				if (forceUpdate || stattree.IsExpanded(time_aaph[mx])) 
				{
					// TIME STATISTICS -> PROJECTED AVERAGES -> TIME PERIOD -> UPLOADS SECTION
					if (forceUpdate || stattree.IsExpanded(time_aap_hup[mx])) 
					{
						// Uploaded Data
						cbuffer.Format(GetResString(IDS_STATS_UDATA),CastItoXBytes( ((double)(bEmuleOut+thePrefs.GetTotalUploaded()))*avgModifier[mx], false, false));
						stattree.SetItemText(time_aap_up[mx][0],cbuffer);
						if (forceUpdate || stattree.IsExpanded(time_aap_up[mx][0])) 
						{
							// Uploaded Data By Client
							if (forceUpdate || stattree.IsExpanded(time_aap_up_hd[mx][0])) 
							{
								uint64 UpDataTotal =		(uint64)(thePrefs.GetUpTotalClientData() * avgModifier[mx]);
								uint64 UpDataClient;
								double percentClientTransferred;
								for(INT_PTR i=0;i<UP_SOFT_COUNT;i++){
									UpDataClient = (uint64)(thePrefs.GetCumUpData((ESoft)i) * avgModifier[mx]);
									if ( UpDataTotal!=0 && UpDataClient!=0 )
										percentClientTransferred = (double) 100 * UpDataClient / UpDataTotal;
									else
										percentClientTransferred = 0;
									cbuffer.Format( softString[i] , CastItoXBytes( UpDataClient, false, false ) , percentClientTransferred );
									stattree.SetItemText( time_aap_up_dc[mx][i] , cbuffer );
								}
							}
							// Uploaded Data By Port
							if (forceUpdate || stattree.IsExpanded(time_aap_up_hd[mx][1])) 
							{
								uint64 PortDataTotal = (uint64)(thePrefs.GetUpTotalPortData() * avgModifier[mx]);
								uint64 PortData;
								double percentPortTransferred;
								for(INT_PTR i=0;i<PORT_COUNT;i++){
									PortData =(uint64)(thePrefs.GetCumUpDataPort((EPort)i) * avgModifier[mx]);
									if ( PortDataTotal!=0 && PortData!=0 )
										percentPortTransferred = (double) 100 * PortData / PortDataTotal;
									else
										percentPortTransferred = 0;
									cbuffer.Format( _T("%s: %s (%1.1f%%)") , GetResString(portStrID[i]) , CastItoXBytes( PortData, false, false ) , percentPortTransferred);
									stattree.SetItemText( time_aap_up_dp[mx][i] , cbuffer );
								}
							}
							// Uploaded Data By Source
							if (forceUpdate || stattree.IsExpanded(time_aap_up_hd[mx][2])) 
							{
								int i = 0;
								uint64	DataSourceFile =	(uint64)(thePrefs.GetCumUpData_File() * avgModifier[mx]);
								uint64	DataSourcePF =		(uint64)(thePrefs.GetCumUpData_Partfile() * avgModifier[mx]);
								uint64	DataSourceTotal =	(uint64)(thePrefs.GetUpTotalDataFile() * avgModifier[mx]);
								double	percentFileTransferred = 0;

								if ( DataSourceTotal!=0 && DataSourceFile!=0 )
									percentFileTransferred = (double) 100 * DataSourceFile / DataSourceTotal;
								cbuffer.Format( _T("%s: %s (%1.1f%%)") , GetResString(IDS_STATS_DSFILE) , CastItoXBytes( DataSourceFile, false, false ) , percentFileTransferred);
								stattree.SetItemText( time_aap_up_ds[mx][i] , cbuffer );
								i++;

								if ( DataSourceTotal!=0 && DataSourcePF!=0 )
									percentFileTransferred = (double) 100 * DataSourcePF / DataSourceTotal;
								else
									percentFileTransferred = 0;
								cbuffer.Format( _T("%s: %s (%1.1f%%)") , GetResString(IDS_STATS_DSPF) , CastItoXBytes( DataSourcePF, false, false ) , percentFileTransferred);
								stattree.SetItemText( time_aap_up_ds[mx][i] , cbuffer );
								i++;
							}
						}
						// Upload Sessions
						uint32 statGoodSessions = (uint32) ((theApp.uploadqueue->GetSuccessfullUpCount() + thePrefs.GetUpSuccessfulSessions() + theApp.uploadqueue->GetUploadQueueLength()) * avgModifier[mx]);
						uint32 statBadSessions = (uint32) ((theApp.uploadqueue->GetFailedUpCount() + thePrefs.GetUpFailedSessions()) * avgModifier[mx]);
						double percentSessions;
						cbuffer.Format(_T("%s: %u"), GetResString(IDS_STATS_ULSES), statGoodSessions + statBadSessions);
						stattree.SetItemText(time_aap_up[mx][1], cbuffer);
						if (forceUpdate || stattree.IsExpanded(time_aap_up[mx][1])) 
						{
							// Set Successful Upload Sessions
							if (statGoodSessions>0) 
								percentSessions = (double) 100*statGoodSessions/(statGoodSessions+statBadSessions);
							else 
								percentSessions = 0;
							cbuffer.Format(GetResString(IDS_STATS_SUCCUPCOUNT),statGoodSessions,percentSessions);
							stattree.SetItemText(time_aap_up_s[mx][0], cbuffer);
							// Set Failed Upload Sessions
							if (percentSessions != 0 && statBadSessions > 0) 
								percentSessions = 100 - percentSessions; // There were some good sessions and bad ones...
							else if (percentSessions == 0 && statBadSessions > 0) 
								percentSessions = 100; // There were bad sessions and no good ones, must be 100%
							else 
								percentSessions = 0; // No sessions at all, or no bad ones.
							cbuffer.Format(GetResString(IDS_STATS_FAILUPCOUNT),statBadSessions,percentSessions);
							stattree.SetItemText(time_aap_up_s[mx][1], cbuffer);
						}

						// Calculate Upline OH Totals
						uint64 UpOHTotal = (uint64)((theStats.GetUpDataOverheadFileRequest() + 
													theStats.GetUpDataOverheadSourceExchange() + 
													theStats.GetUpDataOverheadServer() + 
													theStats.GetUpDataOverheadKad() + 
													theStats.GetUpDataOverheadOther()
												   ) * avgModifier[mx]);
						uint64 UpOHTotalPackets = (uint64)((theStats.GetUpDataOverheadFileRequestPackets() + 
														   theStats.GetUpDataOverheadSourceExchangePackets() + 
														   theStats.GetUpDataOverheadServerPackets() + 
														   theStats.GetUpDataOverheadKadPackets() + 
														   theStats.GetUpDataOverheadOtherPackets()
														  ) * avgModifier[mx]);

						// Set Cumulative Total Overhead
						cbuffer.Format(GetResString(IDS_TOVERHEAD),CastItoXBytes(UpOHTotal + ((uint64)(thePrefs.GetUpOverheadTotal() * avgModifier[mx])), false, false), CastItoIShort((uint64)(UpOHTotalPackets + ((uint64)thePrefs.GetUpOverheadTotalPackets() * avgModifier[mx]))));
						stattree.SetItemText(time_aap_up[mx][2], cbuffer);
						if (forceUpdate || stattree.IsExpanded(time_aap_up[mx][2])) 
						{
							int i = 0;
							// Set up total file req OH
							cbuffer.Format(GetResString(IDS_FROVERHEAD), CastItoXBytes( (uint64)(theStats.GetUpDataOverheadFileRequest() + thePrefs.GetUpOverheadFileReq()) * avgModifier[mx], false, false), CastItoIShort( (uint64)(theStats.GetUpDataOverheadFileRequestPackets() + thePrefs.GetUpOverheadFileReqPackets()) * avgModifier[mx]));
							stattree.SetItemText(time_aap_up_oh[mx][i], cbuffer);
							i++;
							// Set up total source exch OH
							cbuffer.Format(GetResString(IDS_SSOVERHEAD), CastItoXBytes( (uint64)(theStats.GetUpDataOverheadSourceExchange()+thePrefs.GetUpOverheadSrcEx()) * avgModifier[mx], false, false), CastItoIShort( (uint64)(theStats.GetUpDataOverheadSourceExchangePackets()+thePrefs.GetUpOverheadSrcExPackets()) * avgModifier[mx]));
							stattree.SetItemText(time_aap_up_oh[mx][i], cbuffer);
							i++;
							// Set up total server OH
							cbuffer.Format(GetResString(IDS_SOVERHEAD), 
										   CastItoXBytes((theStats.GetUpDataOverheadServer() + 
														          thePrefs.GetUpOverheadServer()
																 ) * avgModifier[mx], false, false), 
										   CastItoIShort((uint64)((theStats.GetUpDataOverheadServerPackets() + 
																  thePrefs.GetUpOverheadServerPackets()
																 ) * avgModifier[mx])));
							stattree.SetItemText(time_aap_up_oh[mx][i], cbuffer);
							i++;
							// Set up total Kad OH
							cbuffer.Format(GetResString(IDS_KADOVERHEAD), 
										   CastItoXBytes((uint64)((theStats.GetUpDataOverheadKad() + 
																  thePrefs.GetUpOverheadKad()
																 ) * avgModifier[mx]), false, false), 
										   CastItoIShort((uint64)((theStats.GetUpDataOverheadKadPackets() + 
																  thePrefs.GetUpOverheadKadPackets()
																 ) * avgModifier[mx])));
							stattree.SetItemText(time_aap_up_oh[mx][i], cbuffer);
							i++;
						}
					} // - End Time Statistics -> Projected Averages -> Time Period -> Uploads Section
					// TIME STATISTICS -> PROJECTED AVERAGES -> TIME PERIOD -> DOWNLOADS SECTION
					if (forceUpdate || stattree.IsExpanded(time_aap_hdown[mx])) 
					{
						//CDownloadQueue::SDownloadStats myStats;
						//theApp.downloadqueue->GetDownloadSourcesStats(myStats);
						// Downloaded Data
						cbuffer.Format(GetResString(IDS_STATS_DDATA),CastItoXBytes( (uint64)((bEmuleIn+thePrefs.GetTotalDownloaded()) * avgModifier[mx]), false, false ));
						stattree.SetItemText(time_aap_down[mx][0], cbuffer);
						if (forceUpdate || stattree.IsExpanded(time_aap_down[mx][0])) 
						{
							// Downloaded Data By Client
							if (forceUpdate || stattree.IsExpanded(time_aap_down_hd[mx][0])) 
							{
								uint64 DownDataTotal =		(uint64)(thePrefs.GetDownTotalClientData() * avgModifier[mx]);
								uint64 DownDataClient;
								double percentClientTransferred;
								for(INT_PTR i=0;i<DOWN_SOFT_COUNT;i++){
									DownDataClient =		(uint64)(thePrefs.GetCumDownData((ESoft)i) * avgModifier[mx]);
									if ( DownDataTotal!=0 && DownDataClient!=0 )
										percentClientTransferred = (double) 100 * DownDataClient / DownDataTotal;
									else
										percentClientTransferred = 0;
									cbuffer.Format( softString[i] , CastItoXBytes( DownDataClient, false, false ) , percentClientTransferred);
									stattree.SetItemText( time_aap_down_dc[mx][i] , cbuffer );
								}
							}
							// Downloaded Data By Port
							if (forceUpdate || stattree.IsExpanded(time_aap_down_hd[mx][1])) 
							{
								uint64	PortDataTotal =		(uint64)(thePrefs.GetDownTotalPortData() * avgModifier[mx]);
								uint64	PortData;
								double	percentPortTransferred;
								for(INT_PTR i=0;i<PORT_COUNT;i++){
									PortData =	(uint64)(thePrefs.GetCumDownDataPort((EPort)i) * avgModifier[mx]);
									if ( PortDataTotal!=0 && PortData!=0 )
										percentPortTransferred = (double) 100 * PortData / PortDataTotal;
									else
										percentPortTransferred = 0;
									cbuffer.Format( _T("%s: %s (%1.1f%%)") , GetResString(portStrID[i]) , CastItoXBytes( PortData, false, false ) , percentPortTransferred);
									stattree.SetItemText( time_aap_down_dp[mx][i] , cbuffer );
								}
							}
						}
						// Set Cum Completed Downloads
						cbuffer.Format(_T("%s: %I64u"), GetResString(IDS_STATS_COMPDL), (uint64) (thePrefs.GetDownCompletedFiles() * avgModifier[mx]) );
						stattree.SetItemText(time_aap_down[mx][1], cbuffer);
						// Set Cum Download Sessions
						uint32	statGoodSessions = (uint32) ((thePrefs.GetDownC_SuccessfulSessions() + theApp.downloadqueue->GetTransferingSources()/*myStats.a[1]*/) * avgModifier[mx]);
						uint32	statBadSessions = (uint32) (thePrefs.GetDownC_FailedSessions() * avgModifier[mx]);
						double	percentSessions;
						cbuffer.Format(_T("%s: %u"), GetResString(IDS_STATS_DLSES), statGoodSessions+statBadSessions );
						stattree.SetItemText(time_aap_down[mx][2], cbuffer);
						if (forceUpdate || stattree.IsExpanded(time_aap_down[mx][2])) 
						{
							// Set Cum Successful Download Sessions
							if (statGoodSessions > 0) 
								percentSessions = (double) 100 * statGoodSessions / (statGoodSessions + statBadSessions);
							else 
								percentSessions = 0;
							cbuffer.Format( _T("%s: %u (%1.1f%%)"), GetResString(IDS_STATS_SDLSES) , statGoodSessions , percentSessions );
							stattree.SetItemText( time_aap_down_s[mx][0] , cbuffer ); // Set Successful Sessions
							// Set Cum Failed Download Sessions
							if (percentSessions != 0 && statBadSessions > 0) 
								percentSessions = 100 - percentSessions; // There were some good sessions and bad ones...
							else if (percentSessions == 0 && statBadSessions > 0) 
								percentSessions = 100; // There were bad sessions and no good ones, must be 100%
							else 
								percentSessions = 0; // No sessions at all, or no bad ones.
							cbuffer.Format( _T("%s: %u (%1.1f%%)") , GetResString(IDS_STATS_FDLSES) , statBadSessions , percentSessions);
							stattree.SetItemText( time_aap_down_s[mx][1] , cbuffer );
						}
						// Set Cumulative Gained Due To Compression
						cbuffer.Format( GetResString( IDS_STATS_GAINCOMP ) , CastItoXBytes( (uint64)(thePrefs.GetSesSavedFromCompression()+ thePrefs.GetCumSavedFromCompression()) * avgModifier[mx], false, false ) );
						stattree.SetItemText( time_aap_down[mx][3] , cbuffer );
						// Set Cumulative Lost Due To Corruption
						cbuffer.Format( GetResString( IDS_STATS_LOSTCORRUPT ) , CastItoXBytes( (uint64)(thePrefs.GetSesLostFromCorruption() + thePrefs.GetCumLostFromCorruption()) * avgModifier[mx], false, false ) );
						stattree.SetItemText( time_aap_down[mx][4] , cbuffer );
						// Set Cumulative Saved Due To ICH
						cbuffer.Format(GetResString(IDS_STATS_ICHSAVED), (uint32)((thePrefs.GetSesPartsSavedByICH() + thePrefs.GetCumPartsSavedByICH()) * avgModifier[mx]));
						stattree.SetItemText( time_aap_down[mx][5] , cbuffer );

						uint64 DownOHTotal =	theStats.GetDownDataOverheadFileRequest() + 
												theStats.GetDownDataOverheadSourceExchange() + 
												theStats.GetDownDataOverheadServer() + 
												theStats.GetDownDataOverheadKad() + 
												theStats.GetDownDataOverheadOther();
						uint64 DownOHTotalPackets = 
												theStats.GetDownDataOverheadFileRequestPackets() + 
												theStats.GetDownDataOverheadSourceExchangePackets() + 
												theStats.GetDownDataOverheadServerPackets() + 
												theStats.GetDownDataOverheadKadPackets() + 
												theStats.GetDownDataOverheadOtherPackets();
						// Total Overhead
						cbuffer.Format(GetResString(IDS_TOVERHEAD),CastItoXBytes( (uint64)(DownOHTotal + thePrefs.GetDownOverheadTotal()) * avgModifier[mx], false, false), CastItoIShort((uint64)(DownOHTotalPackets + thePrefs.GetDownOverheadTotalPackets()) * avgModifier[mx]));
						stattree.SetItemText(time_aap_down[mx][6], cbuffer);
						if (forceUpdate || stattree.IsExpanded(time_aap_down[mx][6])) 
						{
							int i = 0;
							// File Request Overhead
							cbuffer.Format(GetResString(IDS_FROVERHEAD), CastItoXBytes( (uint64)(theStats.GetDownDataOverheadFileRequest() + thePrefs.GetDownOverheadFileReq()) * avgModifier[mx], false, false), CastItoIShort((uint64)(theStats.GetDownDataOverheadFileRequestPackets() + thePrefs.GetDownOverheadFileReqPackets()) * avgModifier[mx]));
							stattree.SetItemText(time_aap_down_oh[mx][i], cbuffer);
							i++;
							// Source Exchange Overhead
							cbuffer.Format(GetResString(IDS_SSOVERHEAD), CastItoXBytes( (uint64)(theStats.GetDownDataOverheadSourceExchange()+thePrefs.GetDownOverheadSrcEx()) * avgModifier[mx], false, false), CastItoIShort((uint64)(theStats.GetDownDataOverheadSourceExchangePackets()+thePrefs.GetDownOverheadSrcExPackets()) * avgModifier[mx]));
							stattree.SetItemText(time_aap_down_oh[mx][i], cbuffer);
							i++;
							// Server Overhead
							cbuffer.Format(GetResString(IDS_SOVERHEAD), 
										   CastItoXBytes((uint64)(theStats.GetDownDataOverheadServer() +
													     thePrefs.GetDownOverheadServer()) * avgModifier[mx], false, false), 
										   CastItoIShort((uint64)(theStats.GetDownDataOverheadServerPackets() + 
														 thePrefs.GetDownOverheadServerPackets()) * avgModifier[mx]));
							stattree.SetItemText(time_aap_down_oh[mx][i], cbuffer);
							i++;
							// Kad Overhead
							cbuffer.Format(GetResString(IDS_KADOVERHEAD), 
										   CastItoXBytes((uint64)(theStats.GetDownDataOverheadKad() +
													              thePrefs.GetDownOverheadKad()
																 ) * avgModifier[mx], false, false), 
										   CastItoIShort((uint64)(theStats.GetDownDataOverheadKadPackets() + 
														          thePrefs.GetDownOverheadKadPackets()
																 ) * avgModifier[mx]));
							stattree.SetItemText(time_aap_down_oh[mx][i], cbuffer);
							i++;
						}
					} // - End Time Statistics -> Projected Averages -> Time Period -> Downloads Section
				} // - End Time Statistics -> Projected Averages -> Time Period Sections
			} // - End Time Statistics -> Projected Averages Section
		} // - End Time Statistics -> Projected Averages Section Loop
	} // - END TIME STATISTICS SECTION
	
	
	// CLIENTS SECTION
	//						Note:	This section now has dynamic tree items.  This technique
	//								may appear in other areas, however, there is usually an
	//								advantage to displaying 0 datems.  Here, with the ver-
	//								sions being displayed the way they are, it makes sense.
	//								Who wants to stare at totally blank tree items?  ;)
	if (forceUpdate || stattree.IsExpanded(h_clients)) 
	{
		// X: [CI] - [Code Improvement]
		uint_ptr							totalclient;
		uint_ptr							myStats[NUM_CLIENTLIST_STATS];
#ifdef REPLACE_ATLMAP
		unordered_map<uint32, uint_ptr>*	clientVersion[MAX_CLIENTS_WITH_SUB_VERSION];
		unordered_map<POSITION, uint_ptr>	clientMODs;
		unordered_map<Country_Struct*, uint_ptr>	clientCountries;
#else
		CAtlMap<uint32, uint_ptr>*	clientVersion[MAX_CLIENTS_WITH_SUB_VERSION];
		/*CAtlMap<uint32, uint_ptr>	clientVersionEDonkey;
		CAtlMap<uint32, uint_ptr>	clientVersionEDonkeyHybrid;
		CAtlMap<uint32, uint_ptr>	clientVersionEMule;
		CAtlMap<uint32, uint_ptr>	clientVersionAMule;*/

		//Xman extended stats
		CAtlMap<POSITION, uint_ptr>	clientMODs;
		CAtlMap<Country_Struct*, uint_ptr>	clientCountries;
#endif
		StatsMaps maps = {
			NULL,
			clientMODs,
			clientCountries,
			NULL// X: [IP2L] - [IP2Location]
		};
		//Xman end
		bool showSoft = forceUpdate || stattree.IsExpanded(hclisoft);
		//Xman end
#ifdef REPLACE_ATLMAP
		for(size_t j = 0; j<MAX_CLIENTS_WITH_SUB_VERSION; ++j){
			if(showSoft || stattree.IsExpanded(clisoft[j]))
				clientVersion[j] = new unordered_map<uint32, uint_ptr>();
			else
				clientVersion[j] = NULL;
		}
		if(showSoft || stattree.IsExpanded(clisoft[0]))
			maps.clientMods = new unordered_map<uint_ptr, CStringToUIntPTRMap* >();
		if(theApp.ip2country->IsIP2Country() && (forceUpdate || stattree.IsExpanded(cligen[7])))
			maps.Locations = new unordered_map<Location_Struct*, uint_ptr>();
#else
		for(size_t j = 0; j<MAX_CLIENTS_WITH_SUB_VERSION; ++j){
			if(showSoft || stattree.IsExpanded(clisoft[j]))
				clientVersion[j] = new CAtlMap<uint32, uint_ptr>();
			else
				clientVersion[j] = NULL;
		}
		if(showSoft || stattree.IsExpanded(clisoft[0]))
			maps.clientMods = new CRBMap<uint_ptr, CRBMap<CString, uint_ptr>* >();
		if(theApp.ip2country->IsIP2Country() && (forceUpdate || stattree.IsExpanded(cligen[7])))
			maps.Locations = new CRBMap<Location_Struct*, uint_ptr>();
#endif

		theApp.clientlist->GetStatistics(totalclient, myStats, clientVersion, maps);
//Xman Code Improvement:

		cbuffer.Format(GetResString(IDS_STATS_FILTEREDCLIENTS) + _T("/%u"), theStats.filteredclients, theApp.ipfilter->hits);// X: [SFH] - [Show IP Filter Hits]
		stattree.SetItemText(cligen[2], cbuffer);
		if(myStats[6] == 0)
			stattree.DeleteChildItems(clisoft[6]);// X: delete All children 1 level compatClient
		if(myStats[7] == 0)
			stattree.DeleteChildItems(cligen[6]);// X: delete All children 1 level mod
		for (size_t i = 0; i < _countof(cli_lastCount); i++){
			if(myStats[i] == 0 && cli_lastCount[i] != 0){
				cli_lastCount[i] = 0;
				stattree.DeleteRestChildItemsR(stattree.GetChildItem(clisoft[i]));// X: delete All children 3 level soft,version,other
			}
		}

		if(totalclient == 0){
			cbuffer = GetResString(IDS_FSTAT_WAITING);
			for(int i = 0; i<DOWN_SOFT_COUNT-1; i++)
				stattree.SetItemText(clisoft[i], cbuffer);
			for(int i = 0; i<4; i++)
				stattree.SetItemText(clinet[i], cbuffer);
			for(int i = 0; i<2; i++)
				stattree.SetItemText(cliport[i], cbuffer);
			for (int i = 0; i < 2; i++)
				stattree.SetItemText(clifirewalled[i], cbuffer);
			stattree.SetItemText(cligen[0], cbuffer);
			stattree.SetItemText(cligen[1], cbuffer);
			for(int i = 3; i<8; i++)
				stattree.SetItemText(cligen[i], cbuffer);
			if(stattree.ItemHasChildren(cligen[7]))
				stattree.DeleteRestChildItemsR(stattree.GetChildItem(cligen[7]));// X: delete All children 2 level country, location
		}
		else
		{
			cbuffer.Format(_T("%s: %u "), GetResString(IDS_CLIENTLIST), totalclient);// X: [UIC] - [UIChange] show more statistics when totalclient==0
			stattree.SetItemText(cligen[5], cbuffer);
			//int SIclients=totalclient; //Xman Xtreme Mod
			cbuffer.Format(_T("%s: %u (%.1f%%) : %u (%.1f%%)"), GetResString(IDS_STATS_SECUREIDENT), myStats[12] , (double)100*myStats[12] / totalclient/*(totalclient>0)?((double)100*myStats[12] / totalclient):0*/, myStats[13] , (double)100*myStats[13] / totalclient/*(totalclient>0)?((double)100*myStats[13] / totalclient ):0*/);
			stattree.SetItemText(cligen[3], cbuffer);

			cbuffer.Format(_T("%s: %u (%.1f%%)"), GetResString(IDS_IDLOW), myStats[14] , (double)100*myStats[14] / totalclient/*(totalclient>0)?((double)100*myStats[14] / totalclient):0*/);
			stattree.SetItemText(cligen[4], cbuffer);

			uint_ptr totalknownclient = totalclient - myStats[10];
			// CLIENTS -> CLIENT SOFTWARE SECTION
			if(totalknownclient == 0){
				cbuffer = GetResString(IDS_FSTAT_WAITING);
				for(int i = 0; i<DOWN_SOFT_COUNT-1; i++)
					stattree.SetItemText(clisoft[i], cbuffer);
			}
			else if (showSoft) 
			{
				static const LPCTSTR softString2[DOWN_SOFT_COUNT-1]={
					_T("eMule: %i (%1.1f%%)"),
					_T("eD Hybrid: %i (%1.1f%%)"),
					_T("eDonkey: %i (%1.1f%%)"),
					_T("aMule: %i (%1.1f%%)"),
					_T("MLdonkey: %i (%1.1f%%)"),
					_T("Shareaza: %i (%1.1f%%)"),
					_T("eM Compat: %i (%1.1f%%)")
				} ;
				for(size_t i = 0; i<DOWN_SOFT_COUNT-1; ++i){
					cbuffer.Format(softString2[i], myStats[i],(double)100*myStats[i]/totalknownclient);
					stattree.SetItemText(clisoft[i], cbuffer);
				}
				/*cbuffer.Format(_T("eMule: %i (%1.1f%%)"),		myStats[ 2],(double)100*myStats[ 2]/totalknownclient);stattree.SetItemText(clisoft[0], cbuffer);
				cbuffer.Format(_T("eD Hybrid: %i (%1.1f%%)"),	myStats[ 4],(double)100*myStats[ 4]/totalknownclient);stattree.SetItemText(clisoft[1], cbuffer);
				cbuffer.Format(_T("eDonkey: %i (%1.1f%%)"),		myStats[ 1],(double)100*myStats[ 1]/totalknownclient);stattree.SetItemText(clisoft[2], cbuffer);
				cbuffer.Format(_T("aMule: %i (%1.1f%%)"),		myStats[10],(double)100*myStats[10]/totalknownclient);stattree.SetItemText(clisoft[3], cbuffer);
				cbuffer.Format(_T("MLdonkey: %i (%1.1f%%)"),	myStats[ 3],(double)100*myStats[ 3]/totalknownclient);stattree.SetItemText(clisoft[4], cbuffer);
				cbuffer.Format(_T("Shareaza: %i (%1.1f%%)"),	myStats[11],(double)100*myStats[11]/totalknownclient);stattree.SetItemText(clisoft[5], cbuffer);
				cbuffer.Format(_T("eM Compat: %i (%1.1f%%)"),	myStats[ 5],(double)100*myStats[ 5]/totalknownclient);stattree.SetItemText(clisoft[6], cbuffer);*/
				//cbuffer.Format(GetResString(IDS_STATS_UNKNOWNCLIENT)+_T(" (%1.1f%%)"),myStats[10] , (double)100*myStats[10]/totalknownclient);stattree.SetItemText(clisoft[7], cbuffer);


				for(size_t j = 0; j<MAX_CLIENTS_WITH_SUB_VERSION; ++j){
					// CLIENTS -> CLIENT SOFTWARE -> MAX_CLIENTS_WITH_SUB_VERSION SECTION
					if (clientVersion[j])
					{
						uint32 verCount = 0;

						//--- find top 4 client versions ---
						uint_ptr currtopcnt = 0;
						uint32 currtopver = 0;
						uint_ptr totalOther = 0;
						for (size_t i = 0; i < MAX_SUB_CLIENT_VERSIONS; i++)
						{
							uint32 topver = 0;
							uint_ptr topcnt = 0;
							double topper = 0.0;
#ifdef REPLACE_ATLMAP
							for (unordered_map<uint32, uint_ptr>::const_iterator it = clientVersion[j]->begin(); it != clientVersion[j]->end(); ++it)
							{
								uint32 ver = it->first;
								uint_ptr cnt = it->second;
#else
							POSITION pos = clientVersion[j]->GetStartPosition();
							while (pos)
							{
								uint32 ver;
								uint_ptr cnt;
								clientVersion[j]->GetNextAssoc(pos, ver, cnt);
#endif
								if (currtopcnt < cnt){
									currtopcnt = cnt;
									currtopver = ver;
								}
								else if (currtopcnt == cnt && currtopver < ver)
									currtopver = ver;
							}
							topver = currtopver;
							topcnt = currtopcnt;
							if(myStats[j])topper = (double)currtopcnt/myStats[j];
							currtopver = 0;
#ifdef REPLACE_ATLMAP
							clientVersion[j]->erase(topver);
#else
							clientVersion[j]->RemoveKey(topver);
#endif
							if (topcnt)
							{
								currtopcnt = 0;
								UINT verMaj = topver/(100*10*100);
								UINT verMin = (topver - (verMaj*100*10*100))/(100*10);
								UINT verUp = (topver - (verMaj*100*10*100) - (verMin*100*10))/(100);
								if ((j == 0 || j == 3) && (topver < MAKE_CLIENT_VERSION(0,40,0) && verUp == 0))
									cbuffer.Format(_T("v%u.%u: %i (%1.1f%%)"), verMaj, verMin, topcnt, topper*100);
								else if(j == 0 && verUp < 26)
									cbuffer.Format(_T("v%u.%u%c: %i (%1.1f%%)"), verMaj, verMin, _T('a') + verUp, topcnt, topper*100);
								else
									cbuffer.Format(_T("v%u.%u.%u: %i (%1.1f%%)"), verMaj, verMin, verUp, topcnt, topper*100);
							}
							else{
								if(j==0 && i < cli_lastCount[j])// Slugfiller: modid
									stattree.DeleteChildItems(cli_versions[i]);// X: delete All children 1 level version
								continue;
							}

							if (i >= MAX_SUB_CLIENT_VERSIONS/2)
								totalOther += topcnt;

							if (i >= cli_lastCount[j]) 
							{
								if (i == MAX_SUB_CLIENT_VERSIONS/2)
									cli_other[j] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, clisoft[j]);
								cli_versions[MAX_SUB_CLIENT_VERSIONS*j+i] = stattree.InsertItem(cbuffer
									, (i >= MAX_SUB_CLIENT_VERSIONS/2)?cli_other[j]:clisoft[j]);
							}
							else
								stattree.SetItemText(cli_versions[MAX_SUB_CLIENT_VERSIONS*j+i], cbuffer);

							if(j==0){//EMULE SECTION
								//Xman
								// Slugfiller: modid
#ifdef REPLACE_ATLMAP
								unordered_map<uint_ptr, CStringToUIntPTRMap* >::const_iterator it1 = maps.clientMods->find(topver);
								if(it1 != maps.clientMods->end()){
									HTREEITEM hChild = (stattree.ItemHasChildren(cli_versions[i]))?stattree.GetChildItem(cli_versions[i]):NULL;
									CStringToUIntPTRMap*versionMods = it1->second;
									for (CStringToUIntPTRMap::const_iterator it2 = versionMods->begin(); it2 != versionMods->end(); ++it2)
									{
										CString name = it2->first;
										uint_ptr count = it2->second;
#else
								CRBMap<CString, uint_ptr> *versionMods;

								if (maps.clientMods->Lookup(topver, versionMods)) {
									POSITION mpos = versionMods->GetHeadPosition();
									HTREEITEM hChild = (stattree.ItemHasChildren(cli_versions[i]))?stattree.GetChildItem(cli_versions[i]):NULL;
									while(mpos != NULL )
									{
										CString name;
										uint_ptr count;
										versionMods->GetNextAssoc(mpos, name, count);
#endif
										if (name.IsEmpty())
											name = _T("Official eMule");
										cbuffer.Format(_T("%s: %i (%1.1f%%)"), name, count, (double)count/topcnt*100);
										if(hChild != NULL){
											stattree.SetItemText(hChild, cbuffer);
											hChild = stattree.GetNextItem(hChild, TVGN_NEXT);
										}
										else
											stattree.InsertItem(cbuffer, cli_versions[i]);
									}
									stattree.DeleteRestChildItems(hChild);// X: delete All Rest children 1 level version
								}
								else
									stattree.DeleteChildItems(cli_versions[i]);// X: delete All children 1 level version
								// Slugfiller: modid
							}

							verCount++;
						}
						if (verCount > MAX_SUB_CLIENT_VERSIONS/2) 
						{
							cbuffer.Format(_T("%s: %i (%1.1f%%)"), GetResString(IDS_STATS_MINORCLIENTS), totalOther, (double)100 * totalOther / myStats[j]);
							stattree.SetItemText(cli_other[j], cbuffer);
						}
						else if (cli_lastCount[j] > MAX_SUB_CLIENT_VERSIONS/2)
							stattree.DeleteItem(cli_other[j]);

						if (verCount < cli_lastCount[j]){
							for (uint32 i = 0; i < cli_lastCount[j] - verCount; i++) 
								stattree.DeleteItem(cli_versions[cli_lastCount[j] + (MAX_SUB_CLIENT_VERSIONS*j-1) - i]);// X: delete All Rest children 1 level soft
						}
						cli_lastCount[j] = verCount;
					}
					else if(myStats[j] && cli_lastCount[j] == 0/*!stattree.ItemHasChildren(clisoft[j])*/){// X: add an item to show plus
						cli_versions[MAX_SUB_CLIENT_VERSIONS*j] = stattree.InsertItem(_T(""), clisoft[j]);
						cli_lastCount[j] = 1;
					}
				} // End Clients -> Client Software -> MAX_CLIENTS_WITH_SUB_VERSION Section

//Spike2 - Enhanced Client Recognition v2 - START
				// ==> Compat Client Stats [Stulle] - Stulle
				if(showSoft || stattree.IsExpanded(clisoft[6]))
				{
					HTREEITEM hChildItem = (stattree.ItemHasChildren(clisoft[6]))?stattree.GetChildItem(clisoft[6]):NULL;
					if(myStats[6]){
						for (size_t i = 0; i < _countof(CompactClientName); ++i)
						{
							if(myStats[OFFICIAL_CLIENTLIST_STATS + i]){
								cbuffer.Format(_T("%s: %i (%1.1f%%)"), CompactClientName[i], myStats[OFFICIAL_CLIENTLIST_STATS + i], (double)myStats[OFFICIAL_CLIENTLIST_STATS + i]/myStats[6]*100);
								if(hChildItem != NULL){
									stattree.SetItemText(hChildItem, cbuffer);
									hChildItem = stattree.GetNextItem(hChildItem, TVGN_NEXT);
								}
								else
									stattree.InsertItem(cbuffer, clisoft[6]);
							}
						}
					}
					stattree.DeleteRestChildItems(hChildItem);// X: delete All Rest children 1 level compatClient
				}
				else if(myStats[6] && !stattree.ItemHasChildren(clisoft[6]))// X: add an item to show plus
					stattree.InsertItem(_T(""), clisoft[6]);
			// <== Compat Client Stats [Stulle] - Stulle
//Spike2 - Enhanced Client Recognition v2 - END

			} // - End Clients -> Client Software Section
			// CLIENTS -> NETWORK SECTION
			if (forceUpdate || stattree.IsExpanded(hclinet)) 
			{
				cbuffer.Format( _T("eD2K: %u (%.1f%%)") , myStats[15], myStats[15] * 100.0 / totalclient/*totalclient ? (myStats[15] * 100.0 / totalclient) : 0.0*/ );
				stattree.SetItemText( clinet[0] , cbuffer );
				cbuffer.Format( _T("Kad: %u (%.1f%%)") , myStats[16], myStats[16] * 100.0 / totalclient/*totalclient ? (myStats[16] * 100.0 / totalclient) : 0.0*/ );
				stattree.SetItemText( clinet[1] , cbuffer );
				cbuffer.Format( _T("eD2K/Kad: %u (%.1f%%)") , myStats[17], myStats[17] * 100.0 / totalclient/*totalclient ? (myStats[17] * 100.0 / totalclient) : 0.0*/ );
				stattree.SetItemText( clinet[2] , cbuffer );
				cbuffer.Format( _T("%s: %u (%.1f%%)") , GetResString(IDS_UNKNOWN), myStats[18], myStats[18] * 100.0 / totalclient/*totalclient ? (myStats[18] * 100.0 / totalclient) : 0.0*/ );
				stattree.SetItemText( clinet[3] , cbuffer );
			}
			// End Clients -> Network Section

			// CLIENTS -> PORT SECTION
			if (forceUpdate || stattree.IsExpanded(hcliport)) 
			{
				cbuffer.Format(_T("%s: %u (%1.1f%%)"), GetResString(IDS_STATS_PRTDEF), myStats[8], myStats[8]>0?((double)100*myStats[8]/(myStats[8]+myStats[9])):0);
				stattree.SetItemText(cliport[0], cbuffer);
				cbuffer.Format(_T("%s: %u (%1.1f%%)"), GetResString(IDS_STATS_PRTOTHER), myStats[9], myStats[9]>0?((double)100*myStats[9]/(myStats[8]+myStats[9])):0);
				stattree.SetItemText(cliport[1], cbuffer);
			} // - End Clients -> Port Section

			// CLIENTS -> FIREWALLED (KAD) SECTION
			if (forceUpdate || stattree.IsExpanded(hclifirewalled)) 
			{
				if (!Kademlia::CKademlia::IsRunning() || Kademlia::CUDPFirewallTester::IsFirewalledUDP(true)) {
					cbuffer.Format(_T("UDP: %s"), GetResString(IDS_KAD_UNKNOWN));
					stattree.SetItemText(clifirewalled[0], cbuffer);
					cbuffer.Format(_T("TCP: %s"), GetResString(IDS_KAD_UNKNOWN));
					stattree.SetItemText(clifirewalled[1], cbuffer);
				}
				else {
					if (Kademlia::CKademlia::GetPrefs()->StatsGetFirewalledRatio(true) > 0)
						cbuffer.Format(_T("UDP: %1.1f%%"), Kademlia::CKademlia::GetPrefs()->StatsGetFirewalledRatio(true) * 100);
					else
						cbuffer.Format(_T("UDP: %s"), GetResString(IDS_FSTAT_WAITING));
					stattree.SetItemText(clifirewalled[0], cbuffer);
					if (Kademlia::CKademlia::GetPrefs()->StatsGetFirewalledRatio(false) > 0)
						cbuffer.Format(_T("TCP: %1.1f%%"), Kademlia::CKademlia::GetPrefs()->StatsGetFirewalledRatio(false) * 100);
					else
						cbuffer.Format(_T("TCP: %s"), GetResString(IDS_FSTAT_WAITING));
					stattree.SetItemText(clifirewalled[1], cbuffer);
				}
			} // - End Clients -> Firewalled (Kad) Section

			// General Client Statistics
			cbuffer.Format(_T("%s: %u (%1.1f%%)"), GetResString(IDS_STATS_PROBLEMATIC), myStats[11], (double)100*myStats[11]/totalclient);
			stattree.SetItemText(cligen[0], cbuffer);
		
			//Xman Anti-Leecher
			//cbuffer.Format(_T("%s: %i/%u"), GetResString(IDS_BANNED), theApp.clientlist->GetBannedCount(), theStats.leecherclients);
			cbuffer.Format(_T("%s: %i"), GetResString(IDS_BANNED), theApp.clientlist->GetBannedCount());
			stattree.SetItemText(cligen[1], cbuffer);
			//Xman end

			//Xman extended stats
			//Mods:
			HTREEITEM hChildItem = (stattree.ItemHasChildren(cligen[6]))?stattree.GetChildItem(cligen[6]):NULL;
			if(myStats[7] == 0)
				cbuffer.Format(_T("Mods: 0"));
			else
			{
				if(forceUpdate || stattree.IsExpanded(cligen[6])){
					uint_ptr	dwLastTop = (uint_ptr)-1;
#ifdef REPLACE_ATLMAP
					while (!clientMODs.empty())
					{
						POSITION	top_pos = NULL;
						uint_ptr	dwCurrTop = 0;
						for (unordered_map<POSITION, uint_ptr>::const_iterator it = clientMODs.begin(); it != clientMODs.end(); ++it)
						{
							POSITION	MOD_pos = it->first;
							uint_ptr	dwMODCnt = it->second;
#else
					while (!clientMODs.IsEmpty())
					{
						POSITION	MOD_pos, top_pos = NULL, pos = clientMODs.GetStartPosition();
						uint_ptr		dwMODCnt, dwCurrTop = 0;

						while(pos){
							clientMODs.GetNextAssoc(pos, MOD_pos, dwMODCnt);
#endif
							if ((dwCurrTop < dwMODCnt) && (dwMODCnt <= dwLastTop))
							{
								top_pos = MOD_pos;
								dwCurrTop = dwMODCnt;
							}
						}
						if ((dwLastTop = dwCurrTop) != 0)
						{
							cbuffer.Format(_T("%s: %u (%.1f%%)"), theApp.clientlist->GetMODType(top_pos), dwLastTop, static_cast<double>(100*dwLastTop)/myStats[7]);
							if(hChildItem != NULL){
								stattree.SetItemText(hChildItem, cbuffer);
								hChildItem = stattree.GetNextItem(hChildItem, TVGN_NEXT);
							}
							else
								stattree.InsertItem(cbuffer, cligen[6]);
#ifdef REPLACE_ATLMAP
							clientMODs.erase(top_pos);
#else
							clientMODs.RemoveKey(top_pos);
#endif
						}
					}
				}
				else{
					// X: add an item to show plus
					if(hChildItem != NULL)
						hChildItem = stattree.GetNextItem(hChildItem, TVGN_NEXT);
					else
						stattree.InsertItem(_T(""), cligen[6]);
				}
				cbuffer.Format(_T("Mods: %u (%.1f%%)"), myStats[7], static_cast<double>(100*myStats[7])/totalknownclient);
			}
			stattree.DeleteRestChildItems(hChildItem);// X: delete All Rest children 1 level mod
			stattree.SetItemText(cligen[6], cbuffer);

			//Countries
			if (theApp.ip2country->IsIP2Country())
			{
#ifdef REPLACE_ATLMAP
				cbuffer.Format(_T("%s: %u"), GetResString(IDS_STATS_COUNTRIES), clientCountries.size());
#else
				cbuffer.Format(_T("%s: %u"), GetResString(IDS_STATS_COUNTRIES), clientCountries.GetCount());
#endif
				stattree.SetItemText(cligen[7], cbuffer);
				//if (totalclient)
				//{
				HTREEITEM hChildItem = (stattree.ItemHasChildren(cligen[7]))?stattree.GetChildItem(cligen[7]):NULL;
				if (maps.Locations){
					uint_ptr	dwLastTop = (uint_ptr)-1;
#ifdef REPLACE_ATLMAP
					while (!clientCountries.empty())
					{
						Country_Struct*	iTopIdx = NULL;
						uint_ptr	dwCurrTop = 0;
						for (unordered_map<Country_Struct*, uint_ptr>::const_iterator it = clientCountries.begin(); it != clientCountries.end(); ++it)
						{
							Country_Struct*	cstruct = it->first;
							uint_ptr		dwCountryCnt = it->second;
#else
					while (!clientCountries.IsEmpty())
					{
						POSITION	pos = clientCountries.GetStartPosition();
						uint_ptr		dwCountryCnt, dwCurrTop = 0;
						Country_Struct*			iTopIdx = NULL;
						Country_Struct*	cstruct;

						while(pos)
						{
							clientCountries.GetNextAssoc(pos, cstruct, dwCountryCnt);
#endif
							if ((dwCurrTop < dwCountryCnt) && (dwCountryCnt <= dwLastTop))
							{
								iTopIdx = cstruct;
								dwCurrTop = dwCountryCnt;
							}
						}
						if ((dwLastTop = dwCurrTop) != 0)
						{
							HTREEITEM hChildItem2 = NULL;// X: [IP2L] - [IP2Location]
							cbuffer.Format(_T("%s: %u (%.1f%%)"), theApp.ip2country->GetCountryNameFromRef(iTopIdx/*,true*/), dwLastTop, static_cast<double>(100*dwLastTop)/totalclient);
							HTREEITEM hNextChildItem = NULL;
							if(hChildItem != NULL){
								if(stattree.ItemHasChildren(hChildItem))
									hChildItem2 = stattree.GetChildItem(hChildItem);
								stattree.SetItemText(hChildItem, cbuffer);
								hNextChildItem = stattree.GetNextItem(hChildItem, TVGN_NEXT);
							}
							else{
								hChildItem = stattree.InsertItem(cbuffer, cligen[7]);
								hNextChildItem = NULL;
							}

#ifdef REPLACE_ATLMAP// X: [IP2L] - [IP2Location]
							for (unordered_map<Location_Struct*, uint_ptr>::const_iterator it = maps.Locations->begin(); it != maps.Locations->end();){
								Location_Struct*	lstruct = it->first;
								uint_ptr		dwLocationCnt = it->second;
#else
							for (POSITION pos = maps.Locations->GetHeadPosition(); pos != 0; ){
								uint_ptr		dwLocationCnt;
								Location_Struct*	lstruct;
								maps.Locations->GetNextAssoc(pos, lstruct, dwLocationCnt);
#endif
								if(lstruct->country == iTopIdx){
									cbuffer.Format(_T("%s: %u"), lstruct->locationName, dwLocationCnt);
									if(hChildItem2 != NULL){
										stattree.SetItemText(hChildItem2, cbuffer);
										hChildItem2 = stattree.GetNextItem(hChildItem2, TVGN_NEXT);
									}
									else
										stattree.InsertItem(cbuffer, hChildItem);
#ifdef REPLACE_ATLMAP
									it = maps.Locations->erase(it);
								}
								else
									++it;
							}
							stattree.DeleteRestChildItems(hChildItem2);// X: delete All Rest children 1 level location
							clientCountries.erase(iTopIdx);
#else
									maps.Locations->RemoveKey(lstruct);
								}
							}
							stattree.DeleteRestChildItems(hChildItem2);// X: delete All Rest children 1 level location
							clientCountries.RemoveKey(iTopIdx);
#endif
							hChildItem = hNextChildItem;
						}
					}
				}
				else/* if(clientCountries.GetCount())*/{
					// X: add an item to show plus
					if(hChildItem != NULL)
						hChildItem = stattree.GetNextItem(hChildItem, TVGN_NEXT);
					else
						stattree.InsertItem(_T(""), cligen[7]);
				}
				stattree.DeleteRestChildItemsR(hChildItem);// X: delete All Rest children 2 level country, location
				//}
				//else
					//cbuffer.Format(_T("Countries: <%s>"), GetResString(IDS_FSTAT_WAITING));
			}
			else{
				cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_COUNTRIES), GetResString(IDS_DISABLED));
				stattree.SetItemText(cligen[7], cbuffer);
			}
			// - End Countries

			//Xman end extended stats

		}//Xman Code Improvement
		for(INT_PTR j = 0; j<MAX_CLIENTS_WITH_SUB_VERSION; ++j)
			delete clientVersion[j];

		if(maps.clientMods){
#ifdef REPLACE_ATLMAP
			for (unordered_map<uint_ptr, CStringToUIntPTRMap* >::const_iterator it = maps.clientMods->begin(); it != maps.clientMods->end(); ++it)
				delete it->second;
#else
			POSITION pos = maps.clientMods->GetHeadPosition();
			while(pos != NULL)
			{
				uint_ptr version;
				CRBMap<CString, uint_ptr> *versionMods;
				maps.clientMods->GetNextAssoc(pos, version, versionMods);
				delete versionMods;
			}
			maps.clientMods->RemoveAll();
#endif
			delete maps.clientMods;
		}
		delete maps.Locations;

	} // - END CLIENTS SECTION
	else{
		stattree.DeleteChildItems(cligen[6]);// X: delete All children 1 level mod
		for (size_t i = 0; i < _countof(cli_lastCount); i++){
			if(cli_lastCount[i] != 0){
				cli_lastCount[i] = 0;
				stattree.DeleteRestChildItemsR(stattree.GetChildItem(clisoft[i]));// X: delete All Rest children 3 level soft,version,other
			}
		}
		if(stattree.ItemHasChildren(cligen[7]))
			stattree.DeleteRestChildItemsR(stattree.GetChildItem(cligen[7]));// X: delete All children 2 level country, location
	}


	// UPDATE RECORDS FOR SERVERS AND SHARED FILES
	thePrefs.SetRecordStructMembers();

	// SERVERS SECTION
	if (forceUpdate || stattree.IsExpanded(h_servers)) 
	{		
		// Get stat values
		uint32	servtotal, servfail, servuser, servfile, servlowiduser, servtuser, servtfile;
		float	servocc;
		theApp.serverlist->GetStatus( servtotal, servfail, servuser, servfile, servlowiduser, servtuser, servtfile, servocc);
		// Set working servers value
		cbuffer.Format(_T("%s: %s"),GetResString(IDS_SF_WORKING),CastItoIShort(servtotal-servfail));stattree.SetItemText(srv[0], cbuffer);
		if (forceUpdate || stattree.IsExpanded(srv[0])) 
		{			
			// Set users on working servers value
			cbuffer.Format(_T("%s: %s; %s: %s (%.1f%%)"),GetResString(IDS_SF_WUSER),CastItoIShort(servuser),GetResString(IDS_IDLOW),CastItoIShort(servlowiduser),servuser ? (servlowiduser * 100.0 / servuser) : 0.0);stattree.SetItemText(srv_w[0], cbuffer);
			// Set files on working servers value
			cbuffer.Format(_T("%s: %s"),GetResString(IDS_SF_WFILE),CastItoIShort(servfile));stattree.SetItemText(srv_w[1], cbuffer);
			// Set server occ value
			cbuffer.Format(GetResString(IDS_SF_SRVOCC),servocc);stattree.SetItemText(srv_w[2], cbuffer);
		}
		// Set failed servers value
		cbuffer.Format(_T("%s: %s"),GetResString(IDS_SF_FAIL),CastItoIShort(servfail));stattree.SetItemText(srv[1], cbuffer);
		// Set deleted servers value
		cbuffer.Format(_T("%s: %s"),GetResString(IDS_SF_DELCOUNT),CastItoIShort(theApp.serverlist->GetDeletedServerCount()));stattree.SetItemText(srv[2], cbuffer);
		// Set total servers value
		cbuffer.Format(_T("%s: %s"),GetResString(IDS_SF_TOTAL),CastItoIShort(servtotal));stattree.SetItemText(srv[3], cbuffer);
		// Set total users value
		cbuffer.Format(_T("%s: %s"),GetResString(IDS_SF_USER),CastItoIShort(servtuser));stattree.SetItemText(srv[4], cbuffer);
		// Set total files value
		cbuffer.Format(_T("%s: %s"),GetResString(IDS_SF_FILE),CastItoIShort(servtfile));stattree.SetItemText(srv[5], cbuffer);
		// SERVERS -> RECORDS SECTION
		if (forceUpdate || stattree.IsExpanded(hsrv_records)) 
		{			
			// Set most working servers
			cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_SVRECWORKING), CastItoIShort(thePrefs.GetSrvrsMostWorkingServers()));
			stattree.SetItemText(srv_r[0], cbuffer);
			// Set most users online
			cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_SVRECUSERS), CastItoIShort(thePrefs.GetSrvrsMostUsersOnline()));
			stattree.SetItemText(srv_r[1], cbuffer);
			// Set most files avail
			cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_SVRECFILES), CastItoIShort(thePrefs.GetSrvrsMostFilesAvail()));
			stattree.SetItemText(srv_r[2], cbuffer);
		} // - End Servers -> Records Section
	} // - END SERVERS SECTION

	
	// SHARED FILES SECTION
	if (forceUpdate || stattree.IsExpanded(h_shared)) 
	{		
		// Set Number of Shared Files
		cbuffer.Format(GetResString(IDS_SHAREDFILESCOUNT),theApp.sharedfiles->GetCount());
		// SLUGFILLER: SafeHash - extra statistics
		if (theApp.sharedfiles->GetHashingCount())
		{
			CString tempbuffer;
			tempbuffer.Format(GetResString(IDS_HASHINGFILESCOUNT),theApp.sharedfiles->GetHashingCount());
			cbuffer += tempbuffer;
		}
		// SLUGFILLER: SafeHash
		stattree.SetItemText(shar[0], cbuffer);
		// Set Average File Size
		uint64 bytesLargestFile = 0;
		uint64 allsize=theApp.sharedfiles->GetDatasize(bytesLargestFile); // Func returns total share size and sets pointeredd uint64 to largest single filesize
		CString cbuffer2;
		if(theApp.sharedfiles->GetCount() != 0)
			cbuffer2 = CastItoXBytes(allsize/(uint64)theApp.sharedfiles->GetCount(), false, false);
		else 
			cbuffer2 = _T("%s"), CastItoXBytes((uint_ptr)0, false, false);
		cbuffer.Format(GetResString(IDS_SF_AVERAGESIZE),cbuffer2);
		stattree.SetItemText(shar[1], cbuffer);
		// Set Largest File Size
		cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_LARGESTFILE), CastItoXBytes(bytesLargestFile, false, false) );
		stattree.SetItemText(shar[2], cbuffer);
		// Set Total Share Size
		cbuffer.Format(GetResString(IDS_SF_SIZE),CastItoXBytes(allsize, false, false));
		stattree.SetItemText(shar[3], cbuffer);

		// SHARED FILES -> RECORDS SECTION
		if (forceUpdate || stattree.IsExpanded(hshar_records)) 
		{			
			// Set Most Files Shared
			cbuffer.Format(_T("%s: %u"), GetResString(IDS_STATS_SHRECNUM), thePrefs.GetSharedMostFilesShared() );
			stattree.SetItemText(shar_r[0], cbuffer);
			// Set largest avg file size
			cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_SHRECASIZE), CastItoXBytes(thePrefs.GetSharedLargestAvgFileSize(), false, false) );
			stattree.SetItemText(shar_r[1], cbuffer);
			// Set largest file size
			cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_LARGESTFILE), CastItoXBytes(thePrefs.GetSharedLargestFileSize(), false, false) );
			stattree.SetItemText(shar_r[2], cbuffer);
			// Set largest share size
			cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_SHRECSIZE), CastItoXBytes(thePrefs.GetSharedLargestShareSize(), false, false) );
			stattree.SetItemText(shar_r[3], cbuffer);
		} // - End Shared Files -> Records Section
	} // - END SHARED FILES SECTION

	if (forceUpdate || stattree.IsExpanded(h_total_downloads)) 
	{			
		uint64 ui64TotalFileSize = 0;
		uint64 ui64TotalLeftToTransfer = 0;
		uint64 ui64TotalAdditionalNeededSpace = 0;
		int iActiveFiles = theApp.downloadqueue->GetDownloadFilesStats(ui64TotalFileSize, ui64TotalLeftToTransfer, ui64TotalAdditionalNeededSpace);

		cbuffer.Format(GetResString(IDS_DWTOT_NR), iActiveFiles);
		stattree.SetItemText(h_total_num_of_dls, cbuffer);

		cbuffer.Format(GetResString(IDS_DWTOT_TSD), CastItoXBytes(ui64TotalFileSize, false, false));
		stattree.SetItemText(h_total_size_of_dls, cbuffer);

		uint64 ui64TotalTransferred = ui64TotalFileSize - ui64TotalLeftToTransfer;
		double fPercent = 0.0;
		if (ui64TotalFileSize != 0)
			fPercent = (ui64TotalTransferred * 100.0) / ui64TotalFileSize;
		cbuffer.Format(GetResString(IDS_DWTOT_TCS), CastItoXBytes(ui64TotalTransferred, false, false), fPercent);
		stattree.SetItemText(h_total_size_dld, cbuffer);

		cbuffer.Format(GetResString(IDS_DWTOT_TSL), CastItoXBytes(ui64TotalLeftToTransfer, false, false));
		stattree.SetItemText(h_total_size_left_to_dl, cbuffer);

		cbuffer.Format(GetResString(IDS_DWTOT_TSN), CastItoXBytes(ui64TotalAdditionalNeededSpace, false, false));
		stattree.SetItemText(h_total_size_needed, cbuffer);

		CString buffer2;
		uint64 ui64TotalFreeSpace = g_VolumeInfo.GetFreeTempSpace();
		buffer2.Format(GetResString(IDS_DWTOT_FS), CastItoXBytes(ui64TotalFreeSpace, false, false));

		if (ui64TotalAdditionalNeededSpace > ui64TotalFreeSpace)
			cbuffer.Format(GetResString(IDS_NEEDFREEDISKSPACE), buffer2, CastItoXBytes(ui64TotalAdditionalNeededSpace - ui64TotalFreeSpace, false, false));
		else
			cbuffer = buffer2;
		stattree.SetItemText(h_total_size_left_on_drive, cbuffer);
	}
	// - End Set Tree Values


#ifdef _DEBUG
	if (g_pfnPrevCrtAllocHook)
	{
		_CrtMemState memState;
		_CrtMemCheckpoint(&memState);

		cbuffer.Format(_T("%s: %u bytes in %u blocks"),_T("Free"),memState.lSizes[ 0 ],memState.lCounts[ 0 ] );
		stattree.SetItemText(debug1, cbuffer);
		cbuffer.Format(_T("%s: %u bytes in %u blocks"),_T("Normal"),memState.lSizes[ 1 ],memState.lCounts[ 1 ] );
		stattree.SetItemText(debug2, cbuffer);
		cbuffer.Format(_T("%s: %u bytes in %u blocks"),_T("CRT"),memState.lSizes[ 2 ],memState.lCounts[ 2 ] );
		stattree.SetItemText(debug3, cbuffer);
		cbuffer.Format(_T("%s: %u bytes in %u blocks"),_T("Ignore"),memState.lSizes[ 3 ],memState.lCounts[ 3 ] );
		stattree.SetItemText(debug4, cbuffer);
		cbuffer.Format(_T("%s: %u bytes in %u blocks"),_T("Client"),memState.lSizes[ 4 ],memState.lCounts[ 4 ] );
		stattree.SetItemText(debug5, cbuffer);

		extern CAtlMap<const unsigned char*, UINT> g_allocations;

		POSITION pos = blockFiles.GetStartPosition();
		while (pos != NULL) 
		{
			const unsigned char* pszFileName;
			HTREEITEM* pTag;
			blockFiles.GetNextAssoc(pos, pszFileName, pTag);
			stattree.SetItemText(*pTag, _T(""));
		}
		pos = g_allocations.GetStartPosition();
		while (pos != NULL) 
		{
			const unsigned char* pszFileName;
			UINT count;
			g_allocations.GetNextAssoc(pos, pszFileName, count);
			HTREEITEM* pTag;
			if (blockFiles.Lookup(pszFileName, pTag) == 0) 
			{
				pTag = new HTREEITEM;
				*pTag = stattree.InsertItem(_T("0"), debug2);
				stattree.SetItemData(*pTag, 1);
				blockFiles.SetAt(pszFileName, pTag);
			}
			cbuffer.Format(_T("%s : %u blocks"), pszFileName, count);
			stattree.SetItemText(*pTag, cbuffer);
		}
	}
#endif

#ifdef USE_MEM_STATS
	if (forceUpdate || stattree.IsExpanded(h_allocs))
	{
		ULONGLONG ullTotalAllocs = 0;
		for (int i = 0; i < ALLOC_SLOTS; i++)
			ullTotalAllocs += g_aAllocStats[i];
		for (int i = 0; i < ALLOC_SLOTS; i++)
		{
			unsigned uStart, uEnd;
			if (i <= 1)
				uStart = uEnd = i;
			else {
				uStart = 1 << (i - 1);
				uEnd = (i == ALLOC_SLOTS - 1) ? (unsigned)-1 : (uStart << 1) - 1;
			}
			CString strLabel;
			strLabel.Format(_T("Block size %08X-%08X: %s (%1.1f%%)"), uStart, uEnd, CastItoIShort(g_aAllocStats[i], false, 2), ullTotalAllocs != 0 ? g_aAllocStats[i] * 100.0 / ullTotalAllocs : 0.0);
			stattree.SetItemText(h_allocSizes[i], strLabel);
		}
	}
#endif

	stattree.SetRedraw(true);

} // ShowStatistics(bool forceRedraw = false){}

void CStatisticsDlg::UpdateConnectionsGraph()
{
	// This updates the Y-Axis scale of the Connections Statistics graph...
	// And it updates the trend ratio for the active connections trend.

	m_Statistics.SetRanges(0, thePrefs.GetStatsMax());
	m_Statistics.SetTrendRatio(0, thePrefs.GetStatsConnectionsGraphRatio());
}

void CStatisticsDlg::OnShowWindow(BOOL /*bShow*/, UINT /*nStatus*/)
{

}
//Xman
void CStatisticsDlg::OnSize(UINT nType, int cx, int cy)
{
	CResizableDialog::OnSize(nType, cx, cy);
		ShowInterval();
}
//Xman end
//Xman
void CStatisticsDlg::ShowInterval()
{
	if (!CemuleDlg::IsRunning())
		return;

	// Check if OScope already initialized
	if (m_DownloadOMeter.GetSafeHwnd() != NULL && m_UploadOMeter.GetSafeHwnd() != NULL)
	{
		// Retrieve the size (in pixel) of the OScope
		CRect plotRect;
		m_UploadOMeter.GetPlotRect(plotRect);
		
		// Dynamic update of time scale [Maella]
		int shownSecs = plotRect.Width() * thePrefs.GetTrafficOMeterInterval(); 
		
		// CB Mod ---> Make Setters
		m_Statistics.m_nXPartial = m_DownloadOMeter.m_nXPartial = m_UploadOMeter.m_nXPartial = shownSecs % 3600;
		m_Statistics.m_nXGrids = m_DownloadOMeter.m_nXGrids = m_UploadOMeter.m_nXGrids = shownSecs / 3600;
		
		if(shownSecs <= 0) 
		{
			m_DownloadOMeter.Reset();
			m_DownloadOMeter.SetXUnits(GetResString(IDS_STOPPED)); 
			m_UploadOMeter.Reset(); 
			m_UploadOMeter.SetXUnits(GetResString(IDS_STOPPED)); 
			m_Statistics.Reset();
			m_Statistics.SetXUnits(GetResString(IDS_STOPPED)); 
		} 
		else 
		{
			const CString buffer = CastSecondsToHM(shownSecs);
			m_UploadOMeter.SetXUnits(buffer);
			m_DownloadOMeter.SetXUnits(buffer); 
			m_Statistics.SetXUnits(buffer); 
		}
		UpdateData(FALSE);
	}
}
//Xman end
//Xman
// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
void CStatisticsDlg::SetARange(bool SetDownload, int maxValue){

	// Maella -Network Adapter Feedback Control-
	m_lastRange[(SetDownload==false)?0:1] = maxValue;
	// Maella end

	if (SetDownload) {
		m_DownloadOMeter.SetRange(0, maxValue+2, ADAPTER);
		m_DownloadOMeter.SetRange(0, maxValue+2, OVERALL);
		m_DownloadOMeter.SetRange(0, maxValue+2, MINUTE);
		m_DownloadOMeter.SetRange(0, maxValue+2, SESSION);
		m_DownloadOMeter.SetRange(0, maxValue+2, CURRENT);

	}else{
		m_UploadOMeter.SetRange(0, maxValue+2, ADAPTER);
		m_UploadOMeter.SetRange(0, maxValue+2, OVERALL);
		m_UploadOMeter.SetRange(0, maxValue+2, MINUTE);
		m_UploadOMeter.SetRange(0, maxValue+2, SESSION);
		m_UploadOMeter.SetRange(0, maxValue+2, CURRENT);
	}
}
// Maella end

// Various changes in Localize() and a new button event...
void CStatisticsDlg::Localize()// X: [RUL] - [Remove Useless Localize]
{
	CreateMyTree();
	ShowStatistics(/*true*/);
	RepaintMeters();

	CString myBuffer;
	myBuffer.Format(GetResString(IDS_STATS_LASTRESETSTATIC), thePrefs.GetStatsLastResetStr(false));
	SetDlgItemText(IDC_STATIC_LASTRESET,myBuffer);
}
// End Localize

// Menu Button: Displays the menu of stat tree commands.
void CStatisticsDlg::OnMenuButtonClicked()
{
	CRect rectBn;
	CPoint thePoint;
	GetDlgItem(IDC_BNMENU)->GetWindowRect(&rectBn);
	thePoint = rectBn.BottomRight();
	stattree.DoMenu(thePoint);
}

void CStatisticsDlg::CreateMyTree() // X: [RUL] - [Remove Useless Localize]
{
	stattree.DeleteAllItems();

	// Setup Tree
	h_transfer = stattree.InsertItem(GetResString(IDS_FSTAT_TRANSFER),1,1);				// Transfers Section
	CString buffer;
	buffer.Format(_T("%s %s"),GetResString(IDS_STATS_SRATIO),GetResString(IDS_FSTAT_WAITING));// Make It Pretty
	trans[0]= stattree.InsertItem(buffer, h_transfer);										// Session Ratio

	buffer.Format(_T("%s %s"),GetResString(IDS_STATS_FRATIO),GetResString(IDS_FSTAT_WAITING));// Make It Pretty
	trans[1]= stattree.InsertItem(buffer, h_transfer);										// Friend Session Ratio

	buffer.Format(_T("%s %s"),GetResString(IDS_STATS_CRATIO),GetResString(IDS_FSTAT_WAITING));// Make It Pretty
	trans[2]= stattree.InsertItem(buffer, h_transfer);										// Cumulative Ratio

	h_upload = stattree.InsertItem(GetResString(IDS_TW_UPLOADS), 6,6,h_transfer);		// Uploads Section
	
	h_up_session= stattree.InsertItem(GetResString(IDS_STATS_SESSION), 8,8,h_upload);		// Session Section (Uploads)
	for(int i = 0; i<_countof(up_S); i++) 
		up_S[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, h_up_session); //MORPH - Added by Yun.SF3, ZZ Upload System
	hup_scb= stattree.InsertItem(GetResString(IDS_CLIENTS),up_S[0]);							// Clients Section
	for(int i = 0; i<_countof(up_scb); i++) 
		up_scb[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, hup_scb);
	hup_spb= stattree.InsertItem(GetResString(IDS_PORT),up_S[0]);								// Ports Section
	for(int i = 0; i<_countof(up_spb); i++) 
		up_spb[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, hup_spb);
	hup_ssb= stattree.InsertItem(GetResString(IDS_STATS_DATASOURCE),up_S[0]);					// Data Source Section
	for(int i = 0; i<2; i++) 
		up_ssb[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, hup_ssb);
	for(int i = 0; i<4; i++) 
		up_ssessions[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, up_S[5]); //MORPH - Added by Yun.SF3, ZZ Upload System
	hup_soh= stattree.InsertItem(_T("")/*GetResString(IDS_STATS_OVRHD)*/,h_up_session);					// Upline Overhead (Session)
	for(int i = 0; i<_countof(up_soh); i++) 
		up_soh[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, hup_soh);
	h_up_total= stattree.InsertItem(GetResString(IDS_STATS_CUMULATIVE),9,9, h_upload);		// Cumulative Section (Uploads)
	up_T[0]= stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/,h_up_total);				// Uploaded Data (Total)
	hup_tcb= stattree.InsertItem(GetResString(IDS_CLIENTS),up_T[0]);							// Clients Section
	for(int i = 0; i<_countof(up_tcb); i++) 
		up_tcb[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, hup_tcb);
	hup_tpb= stattree.InsertItem(GetResString(IDS_PORT),up_T[0]);								// Ports Section
	for(int i = 0; i<_countof(up_tpb); i++) 
		up_tpb[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, hup_tpb);
	hup_tsb= stattree.InsertItem(GetResString(IDS_STATS_DATASOURCE),up_T[0]);					// Data Source Section
	for(int i = 0; i<2; i++) 
		up_tsb[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, hup_tsb);
	up_T[1]= stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/,h_up_total);				// Upload Sessions (Total)
	for(int i = 0; i<4; i++) 
		up_tsessions[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, up_T[1]);
	hup_toh= stattree.InsertItem(_T("")/*GetResString(IDS_STATS_OVRHD)*/,h_up_total);						// Upline Overhead (Total)
	for(int i = 0; i<_countof(up_toh); i++) 
		up_toh[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, hup_toh);
	h_download = stattree.InsertItem(GetResString(IDS_TW_DOWNLOADS), 7,7,h_transfer);	// Downloads Section
	h_down_session= stattree.InsertItem(GetResString(IDS_STATS_SESSION),8,8, h_download);	// Session Section (Downloads)
	for(int i = 0; i<8; i++) 
		down_S[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, h_down_session);
	hdown_scb= stattree.InsertItem(GetResString(IDS_CLIENTS),down_S[0]);						// Clients Section
	for(int i = 0; i<_countof(down_scb); i++) 
		down_scb[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, hdown_scb);
	hdown_spb= stattree.InsertItem(GetResString(IDS_PORT),down_S[0]);							// Ports Section
	for(int i = 0; i<_countof(down_spb); i++) 
		down_spb[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, hdown_spb);
	for(int i = 0; i<_countof(down_sources); i++) 
		down_sources[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, down_S[3]);
	for(int i = 0; i<4; i++) 
		down_ssessions[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, down_S[4]);
	hdown_soh= stattree.InsertItem(_T("")/*GetResString(IDS_STATS_OVRHD)*/,h_down_session);				// Downline Overhead (Session)
	for(int i = 0; i<_countof(down_soh); i++) 
		down_soh[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, hdown_soh);
	h_down_total= stattree.InsertItem(GetResString(IDS_STATS_CUMULATIVE),9,9, h_download);	// Cumulative Section (Downloads)
	for(int i = 0; i<6; i++)
		down_T[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, h_down_total);
	hdown_tcb= stattree.InsertItem(GetResString(IDS_CLIENTS),down_T[0]);						// Clients Section
	for(int i = 0; i<_countof(down_tcb); i++) 
		down_tcb[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, hdown_tcb);
	hdown_tpb= stattree.InsertItem(GetResString(IDS_PORT),down_T[0]);							// Ports Section
	for(int i = 0; i<_countof(down_tpb); i++) 
		down_tpb[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, hdown_tpb);
	for(int i = 0; i<4; i++)
		down_tsessions[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, down_T[2]);
	hdown_toh= stattree.InsertItem(_T("")/*GetResString(IDS_STATS_OVRHD)*/,h_down_total);					// Downline Overhead (Total)
	for(int i = 0; i<_countof(down_toh); i++) 
		down_toh[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, hdown_toh);
#ifdef CLIENTANALYZER
	h_Analyzer = stattree.InsertItem(L"ClientAnalyzer", 18, 18);
	for(int i = 0; i < _countof(h_BadActions); ++i)
		h_BadActions[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), h_Analyzer);
#endif
	h_connection = stattree.InsertItem(GetResString(IDS_FSTAT_CONNECTION),2,2);				// Connection Section
	h_conn_session= stattree.InsertItem(GetResString(IDS_STATS_SESSION),8,8,h_connection);	// Session Section (Connection)
	hconn_sg= stattree.InsertItem(GetResString(IDS_STATS_GENERAL),11,11,h_conn_session);	// General Section (Session)
	for(int i = 0; i<5; i++) 
		conn_sg[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, hconn_sg);
	hconn_su= stattree.InsertItem(GetResString(IDS_PW_CON_UPLBL),6,6,h_conn_session);		// Uploads Section (Session)
	for(int i = 0; i<4; i++)
		conn_su[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, hconn_su);
	hconn_sd= stattree.InsertItem(GetResString(IDS_PW_CON_DOWNLBL),7,7,h_conn_session);			// Downloads Section (Session)
	for(int i = 0; i<4; i++)
		conn_sd[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, hconn_sd);
	h_conn_total= stattree.InsertItem(GetResString(IDS_STATS_CUMULATIVE),9,9,h_connection);	// Cumulative Section (Connection)
	hconn_tg= stattree.InsertItem(GetResString(IDS_STATS_GENERAL),11,11,h_conn_total);		// General (Total)
	for(int i = 0; i<4; i++)
		conn_tg[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, hconn_tg);
	hconn_tu= stattree.InsertItem(GetResString(IDS_PW_CON_UPLBL),6,6,h_conn_total);			// Uploads (Total)
	for(int i = 0; i<3; i++)
		conn_tu[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, hconn_tu);
	hconn_td= stattree.InsertItem(GetResString(IDS_PW_CON_DOWNLBL),7,7,h_conn_total);				// Downloads (Total)
	for(int i = 0; i<3; i++)
		conn_td[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, hconn_td);
	h_time = stattree.InsertItem(GetResString(IDS_STATS_TIMESTATS),12,12);					// Time Statistics Section
	for(int i = 0; i<2; i++)
		tvitime[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, h_time);
	htime_s = stattree.InsertItem(GetResString(IDS_STATS_SESSION),8,8,h_time);				// Session Section (Time)
	for(int i = 0; i<4; i++)
		tvitime_s[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, htime_s);
	for(int i = 0; i<2; i++)
		tvitime_st[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, tvitime_s[1]);
	htime_t = stattree.InsertItem(GetResString(IDS_STATS_CUMULATIVE),9,9,h_time);			// Cumulative Section (Time)
	for(int i = 0; i<3; i++)
		tvitime_t[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, htime_t);
	for(int i = 0; i<2; i++)
		tvitime_tt[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, tvitime_t[1]);
	htime_aap = stattree.InsertItem(GetResString(IDS_STATS_AVGANDPROJ),13,13,h_time);		// Projected Averages Section
	time_aaph[0] = stattree.InsertItem(GetResString(IDS_DAYLY),14,14,htime_aap);			// Daily Section
	time_aaph[1] = stattree.InsertItem(GetResString(IDS_STATS_MONTHLY),15,15,htime_aap);	// Monthly Section
	time_aaph[2] = stattree.InsertItem(GetResString(IDS_STATS_YEARLY),16,16,htime_aap);		// Yearly Section
	for(int x = 0; x<3; x++) 
	{
		time_aap_hup[x] = stattree.InsertItem(GetResString(IDS_TW_UPLOADS),6,6,time_aaph[x]);	// Upload Section
		for(int i = 0; i<3; i++)
			time_aap_up[x][i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/,time_aap_hup[x]);
		time_aap_up_hd[x][0] = stattree.InsertItem(GetResString(IDS_CLIENTS),time_aap_up[x][0]);							// Clients Section
		for(int i = 0; i<7; i++)
			time_aap_up_dc[x][i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, time_aap_up_hd[x][0]);
		time_aap_up_hd[x][1] = stattree.InsertItem(GetResString(IDS_PORT),time_aap_up[x][0]);								// Ports Section
		for(int i = 0; i<_countof(time_aap_up_dp[0]); i++)
			time_aap_up_dp[x][i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, time_aap_up_hd[x][1]);
		time_aap_up_hd[x][2] = stattree.InsertItem(GetResString(IDS_STATS_DATASOURCE),time_aap_up[x][0]);					// Data Source Section
		for(int i = 0; i<2; i++)
			time_aap_up_ds[x][i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, time_aap_up_hd[x][2]);
		for(int i = 0; i<2; i++)
			time_aap_up_s[x][i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, time_aap_up[x][1]);
		for(int i = 0; i<4; i++)
			time_aap_up_oh[x][i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, time_aap_up[x][2]);
        time_aap_hdown[x] = stattree.InsertItem(GetResString(IDS_TW_DOWNLOADS),7,7,time_aaph[x]);// Download Section
		for(int i = 0; i<7; i++)
			time_aap_down[x][i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/,time_aap_hdown[x]);
		time_aap_down_hd[x][0] = stattree.InsertItem(GetResString(IDS_CLIENTS),time_aap_down[x][0]);							// Clients Section
		for(int i = 0; i<8; i++)
			time_aap_down_dc[x][i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, time_aap_down_hd[x][0]);
		time_aap_down_hd[x][1] = stattree.InsertItem(GetResString(IDS_PORT),time_aap_down[x][0]);								// Ports Section
		for(int i = 0; i<_countof(time_aap_down_dp[0]); i++)
			time_aap_down_dp[x][i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, time_aap_down_hd[x][1]);
		for(int i = 0; i<2; i++)
			time_aap_down_s[x][i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, time_aap_down[x][2]);
		for(int i = 0; i<4; i++)
			time_aap_down_oh[x][i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, time_aap_down[x][6]);
	}
	h_clients = stattree.InsertItem(GetResString(IDS_CLIENTS),3,3);							// Clients Section
	//CString strWait = GetResString(IDS_FSTAT_WAITING);
	cligen[5] = stattree.InsertItem(_T("")/*strWait*/, h_clients);
	hclisoft = stattree.InsertItem(GetResString(IDS_CD_CSOFT),h_clients);				// Client Software Section
	for(int i = 0; i<DOWN_SOFT_COUNT-1; i++)
		clisoft[i] = stattree.InsertItem(_T("")/*strWait*/, hclisoft);
	hclinet = stattree.InsertItem(GetResString(IDS_NETWORK),h_clients);						// Client Network Section
	for(int i = 0; i<4; i++)
		clinet[i] = stattree.InsertItem(_T("")/*strWait*/, hclinet);
	hcliport = stattree.InsertItem(GetResString(IDS_PORT),h_clients);						// Client Port Section
	for(int i = 0; i<2; i++)
		cliport[i] = stattree.InsertItem(_T("")/*strWait*/, hcliport);
	hclifirewalled = stattree.InsertItem(GetResString(IDS_FIREWALLED) + _T(" (") + GetResString(IDS_KADEMLIA) + _T(')'),h_clients);
	for (int i = 0; i < 2; i++)
		clifirewalled[i] = stattree.InsertItem(_T("")/*strWait*/, hclifirewalled);	
	cligen[4] = stattree.InsertItem(_T("")/*strWait*/, h_clients);
	cligen[3] = stattree.InsertItem(_T("")/*strWait*/, h_clients);
	for(int i = 0; i<3; i++)
		cligen[i] = stattree.InsertItem(_T("")/*strWait*/, h_clients);
	//Xman extended stats
	cligen[6] = stattree.InsertItem(_T("")/*strWait*/, h_clients); //mods
	cligen[7] = stattree.InsertItem(_T("")/*strWait*/, h_clients); //Countries
	//Xman end
	h_servers = stattree.InsertItem(GetResString(IDS_FSTAT_SERVERS),4,4);					// Servers section
	for(int i = 0; i<6; i++)
		srv[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, h_servers);		// Servers Items
	for(int i = 0; i<3; i++)
		srv_w[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, srv[0]);	// Working Servers Items
	hsrv_records = stattree.InsertItem(GetResString(IDS_STATS_RECORDS),10,10,h_servers);	// Servers Records Section
	for(int i = 0; i<3; i++)
		srv_r[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, hsrv_records);	// Record Items
	h_shared = stattree.InsertItem(GetResString(IDS_SHAREDFILES),5,5 );					// Shared Files Section
	for(int i = 0; i<4; i++)
		shar[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, h_shared);
	hshar_records= stattree.InsertItem(GetResString(IDS_STATS_RECORDS),10,10,h_shared);		// Shared Records Section
	for(int i = 0; i<4; i++)
		shar_r[i] = stattree.InsertItem(_T("")/*GetResString(IDS_FSTAT_WAITING)*/, hshar_records);
	h_total_downloads=stattree.InsertItem(GetResString(IDS_DWTOT),17,17);
	h_total_num_of_dls=stattree.InsertItem(_T("")/*GetResString(IDS_DWTOT_NR)*/,h_total_downloads);
	h_total_size_of_dls=stattree.InsertItem(_T("")/*GetResString(IDS_DWTOT_TSD)*/,h_total_downloads);
	h_total_size_dld=stattree.InsertItem(_T("")/*GetResString(IDS_DWTOT_TCS)*/,h_total_downloads);
	h_total_size_left_to_dl=stattree.InsertItem(_T("")/*GetResString(IDS_DWTOT_TSL)*/,h_total_downloads);
	h_total_size_left_on_drive=stattree.InsertItem(_T("")/*GetResString(IDS_DWTOT_FS)*/,h_total_downloads);
	h_total_size_needed=stattree.InsertItem(_T("")/*GetResString(IDS_DWTOT_TSN)*/,h_total_downloads);

#ifdef _DEBUG
	if (g_pfnPrevCrtAllocHook)
	{
		h_debug = stattree.InsertItem( _T("Debug info") );stattree.SetItemData(h_debug,0);
		h_blocks = stattree.InsertItem(_T("Blocks"),h_debug);stattree.SetItemData(h_blocks,1);
		debug1 =  stattree.InsertItem(_T("Free"),h_blocks);stattree.SetItemData(debug1,1);
		debug2 =  stattree.InsertItem(_T("Normal"),h_blocks);stattree.SetItemData(debug2,1);
		debug3 =  stattree.InsertItem(_T("CRT"),h_blocks);stattree.SetItemData(debug3,1);
		debug4 =  stattree.InsertItem(_T("Ignore"),h_blocks);stattree.SetItemData(debug4,1);
		debug5 =  stattree.InsertItem(_T("Client"),h_blocks);stattree.SetItemData(debug5,1);
		stattree.Expand(h_debug,TVE_EXPAND);
		stattree.Expand(h_blocks,TVE_EXPAND);
	}
#endif

#ifdef USE_MEM_STATS
	h_allocs = stattree.InsertItem(_T("Allocations"));
	for (int i = 0; i < ALLOC_SLOTS; i++)
	{
		unsigned uStart, uEnd;
		if (i <= 1)
			uStart = uEnd = i;
		else {
			uStart = 1 << (i - 1);
			uEnd = (i == ALLOC_SLOTS - 1) ? (unsigned)-1 : (uStart << 1) - 1;
		}
		CString strLabel;
		strLabel.Format(_T("Block size %08X-%08X: %s (%1.1f%%)"), uStart, uEnd, CastItoIShort(g_aAllocStats[i], false, 2), 0.0);
		h_allocSizes[i] = stattree.InsertItem(strLabel, h_allocs);
	}
#endif

	// Make section headers bold in order to make the tree easier to view at a glance.
	stattree.SetItemState(h_transfer, TVIS_BOLD, TVIS_BOLD);
#ifdef CLIENTANALYZER
	stattree.SetItemState(h_Analyzer, TVIS_BOLD, TVIS_BOLD); 
#endif
	stattree.SetItemState(h_connection, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(h_time, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(htime_s, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(htime_t, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(htime_aap, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(h_total_downloads, TVIS_BOLD, TVIS_BOLD);
	for(int i = 0; i<3; i++) 
	{
		stattree.SetItemState(time_aaph[i], TVIS_BOLD, TVIS_BOLD);
		stattree.SetItemState(time_aap_hup[i], TVIS_BOLD, TVIS_BOLD);
		stattree.SetItemState(time_aap_hdown[i], TVIS_BOLD, TVIS_BOLD);
	}
	stattree.SetItemState(h_clients, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(h_servers, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(h_shared, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(h_upload, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(h_download, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(h_up_session, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(h_up_total, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(h_down_session, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(h_down_total, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(h_conn_session, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(h_conn_total, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(hsrv_records, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(hshar_records, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(hconn_sg, TVIS_BOLD, TVIS_BOLD);	
	stattree.SetItemState(hconn_su, TVIS_BOLD, TVIS_BOLD);	
	stattree.SetItemState(hconn_sd, TVIS_BOLD, TVIS_BOLD);	
	stattree.SetItemState(hconn_tg, TVIS_BOLD, TVIS_BOLD);	
	stattree.SetItemState(hconn_tu, TVIS_BOLD, TVIS_BOLD);	
	stattree.SetItemState(hconn_td, TVIS_BOLD, TVIS_BOLD);	
	
	// Expand our purdy new tree...
	stattree.ApplyExpandedMask(thePrefs.GetExpandedTreeItems());
	
	// Select the top item so that the tree is not scrolled to the bottom when first viewed.
	stattree.SelectItem(h_transfer);
	stattree.Init();

	// Initialize our client version counts
	for (int i = 0; i < _countof(cli_lastCount); i++)
		cli_lastCount[i] = 0;

	// End Tree Setup
}

void CStatisticsDlg::OnStnDblclickScope()
{
	theApp.emuledlg->ShowPreferences(IDD_PPG_STATS);
}

LRESULT CStatisticsDlg::OnOscopePositionMsg(WPARAM /*wParam*/, LPARAM lParam)
{
	LPCTSTR pszInfo = (LPCTSTR)lParam;
	m_TimeToolTips->UpdateTipText(pszInfo, GetDlgItem(IDC_SCOPE_D));
	m_TimeToolTips->UpdateTipText(pszInfo, GetDlgItem(IDC_SCOPE_U));
	m_TimeToolTips->UpdateTipText(pszInfo, GetDlgItem(IDC_STATSSCOPE));
	m_TimeToolTips->Update();
	return 0;
}

BOOL CStatisticsDlg::PreTranslateMessage(MSG* pMsg) 
{
	m_TimeToolTips->RelayEvent(pMsg);

	if (pMsg->message == WM_KEYDOWN)
	{
		// Don't handle Ctrl+Tab in this window. It will be handled by main window.
		if (pMsg->wParam == VK_TAB && GetAsyncKeyState(VK_CONTROL) < 0)
			return FALSE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

//TK4 MOD +
void CStatisticsDlg::OnContextMenu(CWnd* /* pWnd */, CPoint doWhere)
{
	int myFlags;
    CRect rc;
	m_DownloadOMeter.GetWindowRect(rc);
	if(doWhere.x < rc.left || doWhere.x > rc.right) 
		return; //if left or right of the scopes then return

	mnuContext.CreatePopupMenu();

	if(paneMaximized) 
		myFlags = MF_GRAYED;
	else             
		myFlags = MF_STRING;


    if(m_bTreepaneHidden) 
	mnuContext.AppendMenu(MF_STRING, MP_HIDE_STREE, GetResString(IDS_RESTSTATSTREE));
    else    
	mnuContext.AppendMenu(MF_STRING, MP_HIDE_STREE, GetResString(IDS_HIDESTATSTREE));

    mnuContext.AppendMenu(MF_STRING, MP_HIDE_SCOPES, GetResString(IDS_HIDE_SCOPES));
    mnuContext.AppendMenu(MF_SEPARATOR);
    mnuContext.AppendMenu(myFlags, MP_MAX_DL, GetResString(IDS_MAX_DL));
    mnuContext.AppendMenu(myFlags, MP_MAX_UP, GetResString(IDS_MAX_UP));
    mnuContext.AppendMenu(myFlags, MP_MAX_ST, GetResString(IDS_MAX_ST));
	mnuContext.AppendMenu(MF_SEPARATOR);
    mnuContext.AppendMenu(myFlags, MP_RESET_OSCOPES, GetResString(IDS_STATS_MNU_OSR)); 
	mnuContext.AppendMenu(MF_SEPARATOR);
	mnuContext.AppendMenu(MF_STRING, MP_STATISICS_PREF, GetResString(IDS_META_PREFERENCES));

	//put the menu at cursor position	   
    mnuContext.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, doWhere.x, doWhere.y, this);
	VERIFY( mnuContext.DestroyMenu() );
}
// Process context menu items and mouse clicks
BOOL CStatisticsDlg::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	switch(LOWORD(wParam)) 
	      {
		    case IDC_SCOPE_D:
		         {
	               if(paneMaximized) 
					   restoreScopeSizes();
	                else            
					   zoomScopeDl();
                    break;
		          }
		    case IDC_SCOPE_U:
		         {
				   if(paneMaximized) 
					   restoreScopeSizes();
                   else            
					   zoomScopeUl();
                    break;
		          }
			case IDC_STATSSCOPE:
		         {
                   if(paneMaximized) 
					  restoreScopeSizes();
                   else            
					  zoomScopeST();
                    break;
		          }
		 	case IDC_BNMENU:
				{
                    OnMenuButtonClicked();
					break;
				}
			case MP_HIDE_STREE:
			     {
                    hideStatsTree();
				    break;
				  }
			case MP_RESET_OSCOPES:
		         {
		           resetScopeSizes();
                   break;
		          }
			case MP_STATISICS_PREF:
			     {
					 theApp.emuledlg->ShowPreferences(IDD_PPG_STATS);
				    break;
				  }
			case MP_MAX_DL:
			     {
                    maximizeScopeDl();
				    break;
				  }
			case MP_MAX_UP:
			     {
                    maximizeScopeUl();
				    break;
				  }
			case MP_MAX_ST:
			     {
                    maximizeScopeST();
				    break;
				  }
			case MP_HIDE_SCOPES:
			     {
					hideScopesView();
					break;
				 }
	       }
	return true;
}

//TK4 Mod - handle message from statstree child window for scopes restore-[SFE] Stats Functional Enhancements
LRESULT CStatisticsDlg::restoreScopes(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
   hideScopesView();
   return 0;
}

//TK4 Mod:Reset Scopes spacing -[SFE] Stats Functional Enhancements
void CStatisticsDlg::resetScopeSizes()
{
	/*CRect rcW;
	GetWindowRect(rcW);
	ScreenToClient(rcW);
	float scopeheight = (((float)rcW.bottom -  ((float)rcW.top))/3.0F);
	midY1 = (float)rcW.bottom / (float)((float)rcW.top + scopeheight);
	midY2 = (float)rcW.bottom / (float)((float)rcW.top + scopeheight + 4.0F + scopeheight);*/
    midY1 = 3.0F;
    midY2 = 1.48385145F;
	//set sizes
	restoreScopeSizes();
}

//TK4 Mod:Restore all Scopes -[SFE] Stats Functional Enhancements
void CStatisticsDlg::restoreScopeSizes()
{
	CRect rcW;
	CRect rect;
	GetWindowRect(rcW);
	ScreenToClient(rcW);
	//set restore values from save proportions
	long mY1 = (long)((float)rcW.bottom / midY1);
	long mY2 = (long)((float)rcW.bottom / midY2);
    //size downloads scope
	m_DownloadOMeter.GetWindowRect(rect);
	ScreenToClient(rect);
    rect.top = rcW.top;
	rect.bottom = mY1;
	m_DownloadOMeter.MoveWindow(rect);
	thePrefs.SetSplitterbarPositionStat_HR(rect.bottom*100/rcW.Height());
    //size uploads scope
	m_UploadOMeter.GetWindowRect(rect);
	ScreenToClient(rect);
    rect.top = mY1 + 6;
	rect.bottom = mY2;
    m_UploadOMeter.MoveWindow(rect);
	thePrefs.SetSplitterbarPositionStat_HL(rect.bottom*100/rcW.Height());
	//size stats scope
	m_Statistics.GetWindowRect(rect);
	ScreenToClient(rect);
    rect.top = mY2 + 6;
	rect.bottom = rcW.bottom - 5;
	m_Statistics.MoveWindow(rect);

    m_Statistics.InvalidateCtrl(true);
	m_UploadOMeter.InvalidateCtrl(true);
    m_DownloadOMeter.InvalidateCtrl(true);

	Invalidate();
	UpdateWindow();

	m_DownloadOMeter.ShowWindow(SW_SHOW);
	m_UploadOMeter.ShowWindow(SW_SHOW);
	m_Statistics.ShowWindow(SW_SHOW);
	m_wndSplitterstat_HR.ShowWindow(SW_SHOW);
	m_wndSplitterstat_HL.ShowWindow(SW_SHOW);
	initCSize();
	//TK4 Mod - set pane zoom state
	paneMaximized = false;
}

//TK4 Mod:Zoom DL Scope -[SFE] Stats Functional Enhancements
void CStatisticsDlg::zoomScopeDl()
{	
	CRect rcW;
	CRect rcspl;
	long delta;
	//get parent windows size
	GetWindowRect(rcW);
	ScreenToClient(rcW);
	//Save the relative positions of the panes
	m_DownloadOMeter.GetWindowRect(rcspl);
	ScreenToClient(rcspl);

	if(rcspl.bottom !=0) 
		midY1 = (float)rcW.bottom / (float)rcspl.bottom; 
	else                
		midY1 = (float)rcW.bottom;

	m_UploadOMeter.GetWindowRect(rcspl);
	ScreenToClient(rcspl);

 	if(rcspl.bottom !=0 ) 
		midY2 = (float)rcW.bottom / (float)rcspl.bottom;  
	else                
		midY2 = (float)rcW.bottom;

	//START resize connections window
	m_wndSplitterstat_HL.GetWindowRect(rcspl);
	ScreenToClient(rcspl);
    delta = rcW.bottom - rcspl.bottom;
	CSplitterControl::ChangeHeight(&m_UploadOMeter, delta , CW_TOPALIGN);
	CSplitterControl::ChangeHeight(&m_Statistics, -delta, CW_BOTTOMALIGN);
	thePrefs.SetSplitterbarPositionStat_HL(99);
	//END resize connections window

    //START resize uploads window
	m_wndSplitterstat_HR.GetWindowRect(rcspl);
	ScreenToClient(rcspl);
    delta = rcW.bottom - rcspl.bottom;
	CSplitterControl::ChangeHeight(&m_DownloadOMeter, delta , CW_TOPALIGN);
	CSplitterControl::ChangeHeight(&m_UploadOMeter, -delta, CW_BOTTOMALIGN);
	thePrefs.SetSplitterbarPositionStat_HR(99);
	//END resize uploads window

    m_DownloadOMeter.InvalidateCtrl(true);
	
	initCSize();
	Invalidate();
	UpdateWindow();
	//TK4 Mod - set pane zoom state
	
    //Hide the controls & panes that are resized
	m_UploadOMeter.ShowWindow(SW_HIDE);
	m_Statistics.ShowWindow(SW_HIDE);
	m_wndSplitterstat_HR.ShowWindow(SW_HIDE);
	m_wndSplitterstat_HL.ShowWindow(SW_HIDE);
	//set pame maximized
	paneMaximized = true;
}

//TK4 Mod:Zoom UL Scope -[SFE] Stats Functional Enhancements
void CStatisticsDlg::zoomScopeUl()
{	
	CRect rcW;
	CRect rcspl;
	long delta;
	//get parent windows size
	GetWindowRect(rcW);
	ScreenToClient(rcW);
	//Save the relative positions of the panes
	m_DownloadOMeter.GetWindowRect(rcspl);
	ScreenToClient(rcspl);

	if(rcspl.bottom !=0 ) 
		midY1 = (float)rcW.bottom / (float)rcspl.bottom; 
	else                 
		midY1 = (float)rcW.bottom;

	m_UploadOMeter.GetWindowRect(rcspl);
	ScreenToClient(rcspl);

	if(rcspl.bottom !=0 ) 
		midY2 = (float)rcW.bottom / (float)rcspl.bottom;  
	else                 
		midY2 = (float)rcW.bottom;

	//START resize connections window
	m_wndSplitterstat_HL.GetWindowRect(rcspl);
	ScreenToClient(rcspl);
    delta = rcW.bottom - rcspl.bottom;//uses more of the available screen
	CSplitterControl::ChangeHeight(&m_UploadOMeter, delta, CW_TOPALIGN);
	CSplitterControl::ChangeHeight(&m_Statistics,  -delta, CW_BOTTOMALIGN);
	//it's always 98-99 99 works, no need for calculating.
	thePrefs.SetSplitterbarPositionStat_HL(99);
	//END resize connections window

    //START resize uploads window
	m_wndSplitterstat_HR.GetWindowRect(rcspl);
	ScreenToClient(rcspl);
	//Delta values:delta = rcspl.top; = delta = 0 - rcspl.top; = delta = rcW.top - rcspl.top;
	CSplitterControl::ChangeHeight(&m_DownloadOMeter, -rcspl.top , CW_TOPALIGN);
	CSplitterControl::ChangeHeight(&m_UploadOMeter, rcspl.top , CW_BOTTOMALIGN);
	//it's zero as we have moved it to the top, no need for calculating.
	thePrefs.SetSplitterbarPositionStat_HR(0);
	//END resize uploads window

    m_UploadOMeter.InvalidateCtrl(true);

	initCSize();
	Invalidate();
	UpdateWindow();

	//Hide the controls & panes that are resized
	m_DownloadOMeter.ShowWindow(SW_HIDE);
	m_Statistics.ShowWindow(SW_HIDE);
	m_wndSplitterstat_HR.ShowWindow(SW_HIDE);
	m_wndSplitterstat_HL.ShowWindow(SW_HIDE);
	//TK4 Mod - set pane zoom state
	paneMaximized = true;
}


//TK4 Mod:Zoom Stats Scope -[SFE] Stats Functional Enhancements
void CStatisticsDlg::zoomScopeST()
{	
	CRect rcW;
	CRect rcspl;

	//get parent windows size
	GetWindowRect(rcW);
	ScreenToClient(rcW);
	//Save the relative positions of the panes
	m_DownloadOMeter.GetWindowRect(rcspl);
	ScreenToClient(rcspl);

	if(rcspl.bottom !=0 ) 
		midY1 = (float)rcW.bottom / (float)rcspl.bottom; 
	else                 
		midY1 = (float)rcW.bottom;

	m_UploadOMeter.GetWindowRect(rcspl);
	ScreenToClient(rcspl);

	if(rcspl.bottom !=0 ) 
		midY2 = (float)rcW.bottom / (float)rcspl.bottom;  
	else                 
		midY2 = (float)rcW.bottom;

    //START resize uploads window
	m_wndSplitterstat_HR.GetWindowRect(rcspl);
	ScreenToClient(rcspl);
	//Delta values:delta = rcspl.top; = delta = 0 - rcspl.top; = delta = rcW.top - rcspl.top;
	CSplitterControl::ChangeHeight(&m_DownloadOMeter, -rcspl.top , CW_TOPALIGN);
	CSplitterControl::ChangeHeight(&m_UploadOMeter, rcspl.top , CW_BOTTOMALIGN);
	//it's zero as we have moved it to the top, no need for calculating.
	thePrefs.SetSplitterbarPositionStat_HR(0);
	//END resize uploads window

	//START resize connections window
    m_wndSplitterstat_HL.GetWindowRect(rcspl);
    ScreenToClient(rcspl);
    //Delta values:delta = rcspl.top; = delta = 0 - rcspl.top; = delta = rcW.top - rcspl.top;
    CSplitterControl::ChangeHeight(&m_UploadOMeter, -rcspl.top , CW_TOPALIGN);
    CSplitterControl::ChangeHeight(&m_Statistics, rcspl.top , CW_BOTTOMALIGN);
    //it's zero as we have moved it to the top, no need for calculating.
    thePrefs.SetSplitterbarPositionStat_HL(0);
    //END resize uploads window
	  
    m_Statistics.InvalidateCtrl(true);
	initCSize();
	Invalidate();
	UpdateWindow();

	//Hide the controls & panes that are resized
	m_DownloadOMeter.ShowWindow(SW_HIDE);
	m_UploadOMeter.ShowWindow(SW_HIDE);
	m_wndSplitterstat_HR.ShowWindow(SW_HIDE);
	m_wndSplitterstat_HL.ShowWindow(SW_HIDE);
	//TK4 Mod - set pane zoom state
	paneMaximized = true;
}
//TK4 Mod:MAX DL Scope -[SFE] Stats Functional Enhancements
void CStatisticsDlg::maximizeScopeDl()
{	
	if(thePrefs.GetSplitterbarPositionStat_HL()>97 && thePrefs.GetSplitterbarPositionStat_HR()>97) 
		return;

	CRect rcspl;
	long delta;

	//START resize connections window
	m_Statistics.GetWindowRect(rcspl);
	ScreenToClient(rcspl);
    delta = rcspl.bottom - rcspl.top;
	CSplitterControl::ChangeHeight(&m_UploadOMeter, delta , CW_TOPALIGN);
	CSplitterControl::ChangeHeight(&m_Statistics, -delta , CW_BOTTOMALIGN);
	thePrefs.SetSplitterbarPositionStat_HR(99); 
    //END resize connections window

    //START resize uploads window
	m_UploadOMeter.GetWindowRect(rcspl);
	ScreenToClient(rcspl);
    delta = rcspl.bottom - rcspl.top;
	CSplitterControl::ChangeHeight(&m_DownloadOMeter, delta , CW_TOPALIGN);
	CSplitterControl::ChangeHeight(&m_UploadOMeter, -delta , CW_BOTTOMALIGN);
	thePrefs.SetSplitterbarPositionStat_HL(99); 
	//END resize uploads window

    m_DownloadOMeter.InvalidateCtrl(true);
	initCSize();
	Invalidate();
	UpdateWindow();
}
//TK4 Mod:MAX UL Scope -[SFE] Stats Functional Enhancements
void CStatisticsDlg::maximizeScopeUl()
{	
	if(thePrefs.GetSplitterbarPositionStat_HL()>97 && thePrefs.GetSplitterbarPositionStat_HR()<1) 
		return;

	CRect rcspl;
	long delta;

	//START resize connections window
    m_wndSplitterstat_HR.GetWindowRect(rcspl);
	ScreenToClient(rcspl);
	CSplitterControl::ChangeHeight(&m_DownloadOMeter, -rcspl.top , CW_TOPALIGN);
	CSplitterControl::ChangeHeight(&m_UploadOMeter, rcspl.top , CW_BOTTOMALIGN);
	thePrefs.SetSplitterbarPositionStat_HR(0);
    //END resize connections window

    //START resize uploads window
    m_Statistics.GetWindowRect(rcspl);
	ScreenToClient(rcspl);
    delta = rcspl.bottom - rcspl.top;
	CSplitterControl::ChangeHeight(&m_UploadOMeter, delta , CW_TOPALIGN);
	CSplitterControl::ChangeHeight(&m_Statistics, -delta , CW_BOTTOMALIGN);
	thePrefs.SetSplitterbarPositionStat_HL(99); 
	//END resize uploads window 

    m_UploadOMeter.InvalidateCtrl(true);
	initCSize();
	Invalidate();
	UpdateWindow();
}

//TK4 Mod:MAX UL Scope -[SFE] Stats Functional Enhancements
void CStatisticsDlg::maximizeScopeST()
{	
	if(thePrefs.GetSplitterbarPositionStat_HL()<1 && thePrefs.GetSplitterbarPositionStat_HR()<1) 
		return;

	CRect rcspl;
	long delta;

	//START resize connections window
    m_wndSplitterstat_HR.GetWindowRect(rcspl);
	ScreenToClient(rcspl);
	CSplitterControl::ChangeHeight(&m_DownloadOMeter, -rcspl.top , CW_TOPALIGN);
	CSplitterControl::ChangeHeight(&m_UploadOMeter, rcspl.top , CW_BOTTOMALIGN);
	thePrefs.SetSplitterbarPositionStat_HR(0);
	//END resize connections window

    //START resize uploads window
	m_wndSplitterstat_HL.GetWindowRect(rcspl);
	ScreenToClient(rcspl);
    delta = rcspl.top - 5;
	CSplitterControl::ChangeHeight(&m_UploadOMeter, -delta , CW_TOPALIGN);
	CSplitterControl::ChangeHeight(&m_Statistics, delta , CW_BOTTOMALIGN);
	thePrefs.SetSplitterbarPositionStat_HL(0);  
	//END resize uploads window 

    m_Statistics.InvalidateCtrl(true);
	initCSize();
	Invalidate();
	UpdateWindow();

}
/*TK4 Mod:Hide and Restore Stats Tree *extra code added to DoResize_V() also.* -[SFE] Stats Functional Enhancements*/
void CStatisticsDlg::hideStatsTree()
{	
	CRect rcTree;

	stattree.GetWindowRect(rcTree);
	ScreenToClient(rcTree);
    if(rcTree.right > rcTree.left)
	  {  //do resize, scopes full width statstree = 0 width
		 DoResize_V(-(rcTree.right - 8));
	  } else {//Restore stats window to default position
		       CRect rcW;
	           GetWindowRect(rcW);
	           ScreenToClient(rcW);
			   DoResize_V((int)((float)rcW.right/3.5F));
	          }
}

/*TK4 Mod:Hide and Restore Stats Tree *extra code added to DoResize_V() also.* -[SFE] Stats Functional Enhancements*/
void CStatisticsDlg::hideScopesView()
{	
	CRect rcScopes;

	m_DownloadOMeter.GetWindowRect(rcScopes);
	ScreenToClient(rcScopes);
    if(rcScopes.right > rcScopes.left) 
		DoResize_V(rcScopes.right - rcScopes.left); //do resize, scopes full width statstree = 0 width
    else {//Restore stats window to default position
		       CRect rcW;
	           GetWindowRect(rcW);
	           ScreenToClient(rcW);
			   DoResize_V(-(int)((float)rcW.right/1.445F));
	        }
}
//TK4 MOD -

HBRUSH CStatisticsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = theApp.emuledlg->GetCtlColor(pDC, pWnd, nCtlColor);
	if (hbr)
		return hbr;
	return __super::OnCtlColor(pDC, pWnd, nCtlColor);
}