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
#include <afxinet.h>
#include "emule.h"
#include "emuleDlg.h"
#ifdef OLD_SOCKETS_ENABLED
#include "sockets.h"
#endif //OLD_SOCKETS_ENABLED
#include "KnownFileList.h"
#include "KnownFile.h"
#include "ServerList.h"
#include "server.h"
#include "opcodes.h"
#include "SharedFileList.h"
#include "ED2KLink.h"
#include "math.h"
#include "SplashScreen.h"
#pragma comment(lib, "winmm.lib")
#include "Mmsystem.h"
#include "EnBitmap.h"
#include "HTRichEditCtrl.h"
#include "MMServer.h"
#include "AboutDlg.h"
#include <share.h>
#include <io.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define EP_BAKMODE_AUTOMATED		0x01
#define EP_BAKMODE_SKIPZERO			0x02

const static UINT		UWM_ARE_YOU_EMULE = RegisterWindowMessage(EMULE_GUID);

BEGIN_MESSAGE_MAP(CEmuleDlg, CTrayDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_WM_CLOSE()

	ON_COMMAND(MP_CONNECT, StartConnection)
	ON_COMMAND(MP_DISCONNECT, CloseConnection)
	ON_COMMAND(MP_RESTORE, RestoreWindow)
	ON_COMMAND(MP_EXIT, OnClose)

	ON_MESSAGE(WM_TASKBARNOTIFIERCLICKED, OnTaskbarNotifierClicked)
	ON_MESSAGE(WM_COPYDATA, OnWMData)
	ON_MESSAGE(TM_FINISHEDHASHING, OnFileHashed)
	ON_MESSAGE(TM_HASHFAILED, OnHashFailed)
	ON_MESSAGE(TM_HASHINGSTARTED, OnFileHashingStarted)
	ON_MESSAGE(WEB_CONNECT_TO_SERVER, OnWebServerConnect)
	ON_MESSAGE(WEB_REMOVE_SERVER, OnWebServerRemove)
	ON_MESSAGE(WEB_ADD_TO_STATIC, OnWebServerAddToStatic)
	ON_MESSAGE(WEB_REMOVE_FROM_STATIC, OnWebServerRemoveFromStatic)
	ON_MESSAGE(WEB_CLEAR_COMPLETED, OnWebServerClearCompleted)
	ON_MESSAGE(WEB_FILE_RENAME, OnWebServerFileRename)
	ON_MESSAGE(WEB_SHARED_FILES_RELOAD, OnWebSharedFilesReload)
	ON_REGISTERED_MESSAGE(UWM_ARE_YOU_EMULE, OnAreYouEmule)

	ON_WM_QUERYENDSESSION()
	ON_WM_TIMER()
	ON_WM_NCDESTROY()
	ON_WM_ACTIVATE()
END_MESSAGE_MAP()

//Lucas - 08-Jan Define settig the Timer ID to be used.
//Could be interesting to standarize the TIMER_IDs (there are many already)
#define INIT_TIMER_ID 1234 // This is a magic number...

// CEmuleDlg Dialog

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CEmuleDlg::CEmuleDlg(CWnd* pParent /*=NULL*/)
	: CTrayDialog(CEmuleDlg::IDD, pParent)
{
	static const uint16 s_auClientIconResID[] =
	{
		IDI_COMPROT,		//SO_PLUS
		IDI_COMPROT,		//SO_EMULE
		IDI_AMULE,			//SO_AMULE
		IDI_EDONKEYHYBRID,	//SO_EDONKEYHYBRID
		IDI_NORMAL,			//SO_EDONKEY
		IDI_MLDONKEY,		//SO_MLDONKEY
		IDI_SECUREHASH,		//SO_OLDEMULE + non-SUI
		IDI_SHAREAZA,		//SO_SHAREAZA
		IDI_XMULE,			//SO_XMULE
		IDI_LPHANT,			//SO_LPHANT
		IDI_UNKNOWN			//SO_UNKNOWN
	};
	static const uint16 s_auPropIconResID[] =
	{
		IDI_FRIEND_ONLY,			//CLIENT_IMGLST_FRIEND
		IDI_CREDIT_ONLY,			//CLIENT_IMGLST_CREDITUP
		IDI_CREDIT_DOWNESTIMATED,	//CLIENT_IMGLST_CREDITDOWN
		IDI_BANNED_ONLY				//CLIENT_IMGLST_BANNED
	};

	EMULE_TRY

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hiconMyTray = NULL;
	m_bStartUpMinimized = false;
	m_bCliExit = false;

	m_pstrNewLogTextLines = new CString;
	m_pstrNewDebugLogTextLines = new CString;

	lastuprate = 0;
	lastdownrate = 0;
	g_App.m_app_state = g_App.APP_STATE_RUNNING;
	m_iStatus = 0;

	HINSTANCE hInst = AfxGetInstanceHandle();

	m_hiconConn = NULL;
	m_hiconTrans[0] = (HICON)::LoadImage(hInst, MAKEINTRESOURCE(IDI_UP0),IMAGE_ICON,16,16,0);
	m_hiconTrans[1] = (HICON)::LoadImage(hInst, MAKEINTRESOURCE(IDI_UP1),IMAGE_ICON,16,16,0);
	m_hiconTrans[2] = (HICON)::LoadImage(hInst, MAKEINTRESOURCE(IDI_DOWN0),IMAGE_ICON,16,16,0);
	m_hiconTrans[3] = (HICON)::LoadImage(hInst, MAKEINTRESOURCE(IDI_DOWN1),IMAGE_ICON,16,16,0);
	m_hiconIM[0] = 0;
	m_hiconIM[1] = (HICON)::LoadImage(hInst, MAKEINTRESOURCE(IDI_MESSAGE),IMAGE_ICON,16,16,0);
	m_hiconIM[2] = (HICON)::LoadImage(hInst, MAKEINTRESOURCE(IDI_MPENDING),IMAGE_ICON,16,16,0);
	m_hiconSourceTray = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_TRAYICON),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
	m_hiconSourceTrayLowID = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_TRAYICON_LOWID),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
	m_hiconSourceTrayGrey = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_TRAYICON_GREY),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
	m_hiconUsers = (HICON)::LoadImage(hInst, MAKEINTRESOURCE(IDI_USERS),IMAGE_ICON,16,16,0);
	m_pSystrayDlg = NULL;

	m_strDebugLogFilePath.Format(_T("%sdebug.log"), g_App.m_pPrefs->GetAppDir());
#if 1 //code left for smooth migration, delete in v1.3
	if (GetTextFileFormat(m_strDebugLogFilePath) == tffANSI)
	{
		CString strBackupFile(m_strDebugLogFilePath);

		strBackupFile += _T(".ansi");

		_tremove(strBackupFile);
		_trename(m_strDebugLogFilePath, strBackupFile);
	}
#endif
	m_strLogFilePath.Format(_T("%seMule.log"), g_App.m_pPrefs->GetAppDir());
#if 1 //code left for smooth migration, delete in v1.3
	if (GetTextFileFormat(m_strLogFilePath) == tffANSI)
	{
		CString strBackupFile(m_strLogFilePath);

		strBackupFile += _T(".ansi");

		_tremove(strBackupFile);
		_trename(m_strLogFilePath, strBackupFile);
	}
#endif

// Prepare image lists with 16x16 client icons
	CImageList	m_tmpImageList;

//	Load plain client icons
	m_clientImgLists[CLIENT_IMGLST_PLAIN].Create(16, 16, g_App.m_iDfltImageListColorFlags | ILC_MASK, ARRSIZE(s_auClientIconResID), 0);
	m_clientImgLists[CLIENT_IMGLST_PLAIN].SetBkColor(CLR_NONE);
	FillImgLstWith16x16Icons(&m_clientImgLists[CLIENT_IMGLST_PLAIN], s_auClientIconResID, ARRSIZE(s_auClientIconResID));

//	Create client icons with different properties
	m_tmpImageList.Create(16, 16, g_App.m_iDfltImageListColorFlags | ILC_MASK, 1, 0);
	m_tmpImageList.SetImageCount(1);
	for (unsigned uiListIdx = 1; uiListIdx <= ARRSIZE(s_auPropIconResID); uiListIdx++)
	{
		HICON	hIcon = reinterpret_cast<HICON>(::LoadImage(hInst, MAKEINTRESOURCE(s_auPropIconResID[uiListIdx - 1]), IMAGE_ICON, 0, 0, 0));

		m_tmpImageList.Replace(0, hIcon);
		::DestroyIcon(hIcon);

		m_clientImgLists[uiListIdx].Create(16, 16, g_App.m_iDfltImageListColorFlags | ILC_MASK, ARRSIZE(s_auClientIconResID), 0);
		m_clientImgLists[uiListIdx].SetBkColor(CLR_NONE);
		for (unsigned uiIdx = 0; uiIdx < ARRSIZE(s_auClientIconResID); uiIdx++)
		{
			HIMAGELIST hImgLst = ::ImageList_Merge(m_clientImgLists[CLIENT_IMGLST_PLAIN].m_hImageList, uiIdx, m_tmpImageList.m_hImageList, 0, 0, 0);

			hIcon = ::ImageList_GetIcon(hImgLst, 0, 0);
			m_clientImgLists[uiListIdx].Add(hIcon);
			::DestroyIcon(hIcon);
			::ImageList_Destroy(hImgLst);
		}
	}

	m_pdlgActive = NULL;

#ifdef NEW_SOCKETS
//	m_pEngineData = new CEngineData(*g_App.m_pEngine);
#endif //NEW_SOCKETS

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CEmuleDlg::~CEmuleDlg()
{
	EMULE_TRY

	if (m_hiconMyTray != NULL)
		DestroyIcon(m_hiconMyTray);

	::DestroyIcon(m_hIcon);
	if (m_hiconConn != NULL)
		::DestroyIcon(m_hiconConn);
	::DestroyIcon(m_hiconTrans[0]);
	::DestroyIcon(m_hiconTrans[1]);
	::DestroyIcon(m_hiconTrans[2]);
	::DestroyIcon(m_hiconTrans[3]);
	::DestroyIcon(m_hiconIM[1]);
	::DestroyIcon(m_hiconIM[2]);
	::DestroyIcon(m_hiconSourceTray);
	::DestroyIcon(m_hiconSourceTrayLowID);
	::DestroyIcon(m_hiconSourceTrayGrey);
	::DestroyIcon(m_hiconUsers);

	m_wndServer.DestroyWindow();
	m_wndSharedFiles.DestroyWindow();
	m_dlgSearch.DestroyWindow();
	m_wndChat.DestroyWindow();
	m_wndTransfer.DestroyWindow();
	m_dlgStatistics.DestroyWindow();
	m_wndIRC.DestroyWindow();

	delete m_pstrNewLogTextLines;
	delete m_pstrNewDebugLogTextLines;

#ifdef NEW_SOCKETS
//	if (m_pEngineData)
//		delete m_pEngineData;
#endif //NEW_SOCKETS

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::DoDataExchange(CDataExchange *pDX)
{
	CTrayDialog::DoDataExchange(pDX);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CEmuleDlg::OnAreYouEmule(WPARAM, LPARAM)
{
	return UWM_ARE_YOU_EMULE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT CEmuleDlg::CheckCurrentVersionAtInet(void *)
{
	EMULE_TRY

	g_App.m_pPrefs->InitThreadLocale();

	CString	strTmp;

	strTmp.Format( _T("http://updates.emuleplus.info/get_version.php?version=%u&language=%u"),
		CURRENT_PLUS_VERSION, g_App.m_pPrefs->GetLanguageID() );

	HINTERNET	hOpen = ::InternetOpen(HTTP_USERAGENT, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

	if (hOpen != NULL)
	{
		HINTERNET	hURL = ::InternetOpenUrl(hOpen, strTmp, _T(""), 0, INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_COOKIES, NULL);

		if (hURL != NULL)
		{
			char	pcBuf[192];
			DWORD	dwSize;

			if (::InternetReadFile(hURL, pcBuf, ARRSIZE(pcBuf), &dwSize) && dwSize >= 5)
			{
				static const byte	s_abytePageHdr[5] = {0xEF, 0xBB, 0xBF, 'O', 'K' };	// UTF-8 BOM + 'OK'

				if (memcmp(pcBuf, s_abytePageHdr, 5) == 0)
				{
					WCHAR	pwcBuf[192];

					if ((dwSize = MultiByteToWideChar(CP_UTF8, 0, pcBuf + 5, dwSize - 5, pwcBuf, ARRSIZE(pwcBuf))) != 0)
					{
						strTmp = CStringW(pwcBuf, dwSize);

					// 	Everything alright
						g_App.m_pMDlg->AddLogLine(LOG_FL_SBAR | LOG_RGB_NOTICE, _T("%s"), strTmp);
						if (strTmp.Find(_T("http://emuleplus.info")) >= 0)
							g_App.m_pMDlg->ShowNotifier(strTmp, TBN_LOG, false, true);
						g_App.m_pPrefs->SetAutoCheckLastTime(time(NULL));
					}
				}
			}
			::InternetCloseHandle(hURL);
		}
		::InternetCloseHandle(hOpen);
	}

	EMULE_CATCH2

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CEmuleDlg::OnInitDialog()
{
	EMULE_TRY

	CTrayDialog::OnInitDialog();

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu	*pSysMenu = GetSystemMenu(FALSE);

	if (pSysMenu != NULL)
	{
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, GetResString(IDS_ABOUTBOX));
	}

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

//	Set font
	m_fontDefault.CreatePointFont(g_App.m_pPrefs->GetFontSize(),g_App.m_pPrefs->GetUsedFont());

//	Set tool bar
	m_ctlToolBar.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, CRect(0,0,0,0), this, IDC_TOOLBAR);
	m_ctlToolBar.Init();
	m_ctlToolBar.ShowSpeedMeter(g_App.m_pPrefs->GetShowToolbarSpeedMeter());

	uint32	dwMaxRange = g_App.m_pPrefs->GetMaxGraphDownloadRate();

	if (g_App.m_pPrefs->GetMaxGraphUploadRate() > dwMaxRange)
		dwMaxRange = g_App.m_pPrefs->GetMaxGraphUploadRate();
	m_ctlToolBar.SetSpeedMeterRange(dwMaxRange / 10, 0);

#ifdef USE_REBAR
//	Set rebar
	REBARBANDINFO	rbbi;
	CSize			sizeBar;

	m_ctlToolBar.GetMaxSize(&sizeBar);
	m_ctlReBar.Create( WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_BORDER | RBS_BANDBORDERS
		| RBS_VARHEIGHT | CCS_NODIVIDER | CCS_TOP,
		CRect(0, 0, 0, 0), this, AFX_IDW_REBAR );

//	Insert the tool bar into the rebar
	rbbi.cbSize       = sizeof(REBARBANDINFO);
	rbbi.fMask        = RBBIM_SIZE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_STYLE;
	rbbi.cxMinChild   = 0;
	rbbi.cyMinChild   = sizeBar.cy;
	rbbi.cx           = 0;
	rbbi.fStyle       = RBBS_NOGRIPPER | RBBS_BREAK;
	rbbi.hwndChild    = (HWND) m_ctlToolBar;
	m_ctlReBar.InsertBand(~0u, &rbbi);
#endif

//	Set window title
	SetWindowText(CLIENT_NAME_WITH_VER);

//	Initialize Taskbar Notifier
	m_wndTaskbarNotifier.Create(this);

	CEnBitmap		m_imgTaskbar;

	VERIFY(m_imgTaskbar.LoadImage(IDR_POPUP,_T("JPG")));
	m_wndTaskbarNotifier.SetBitmap(&m_imgTaskbar, 255, 0, 255);
	m_wndTaskbarNotifier.SetTextFont(_T("Arial"),g_App.m_pPrefs->NotificationFontSize(),TN_TEXT_NORMAL,TN_TEXT_UNDERLINE);
	m_wndTaskbarNotifier.SetTextColor(RGB(255,255,230),RGB(255,255,255));
	m_wndTaskbarNotifier.SetTextRect(CRect(10, 29, m_wndTaskbarNotifier.m_nBitmapWidth - 10, m_wndTaskbarNotifier.m_nBitmapHeight - 9));
	m_wndTaskbarNotifier.m_bStarted = true;

// 	Set m_ctlStatusBar
	m_ctlStatusBar.Create(WS_CHILD|WS_VISIBLE|CCS_BOTTOM|SBARS_SIZEGRIP,CRect(0,0,0,0), this, IDC_STATUSBAR);
	ResizeStatusBar();
	m_ctlStatusBar.SetIcon(SB_NUMUSERS, m_hiconUsers);

// 	Create dialog pages
	m_dlgPreferences.SetPrefs(g_App.m_pPrefs);
	m_wndServer.Create(IDD_SERVER);
	m_wndSharedFiles.Create(IDD_FILES);
	m_dlgSearch.Create(IDD_SEARCH);
	m_wndChat.Create(IDD_CHAT);
	m_wndTransfer.Create(IDD_TRANSFER);
	m_dlgStatistics.Create(IDD_STATISTICS);
	m_wndIRC.Create(IDD_IRC);
	m_pdlgActive = &m_wndTransfer;

	CRect rClientRect;
	GetClientRect(&rClientRect);
#ifdef USE_REBAR
	CRect rReBarRect;
	m_ctlReBar.GetWindowRect(&rReBarRect);
	CRect rStatusbarRect;
	m_ctlStatusBar.GetWindowRect(&rStatusbarRect);
	rClientRect.top += rReBarRect.Height();
	rClientRect.bottom -= rStatusbarRect.Height();
#else
	CRect rToolbarRect;
	m_ctlToolBar.GetWindowRect(&rToolbarRect);
	CRect rStatusbarRect;
	m_ctlStatusBar.GetWindowRect(&rStatusbarRect);
	rClientRect.top += rToolbarRect.Height();
	rClientRect.bottom -= rStatusbarRect.Height();
#endif
	m_wndServer.SetWindowPos(NULL, rClientRect.left, rClientRect.top,
							rClientRect.Width(), rClientRect.Height(), SWP_NOZORDER);
	m_wndTransfer.SetWindowPos(NULL, rClientRect.left, rClientRect.top,
								rClientRect.Width(), rClientRect.Height(), SWP_NOZORDER);
	m_wndSharedFiles.SetWindowPos(NULL, rClientRect.left, rClientRect.top,
								rClientRect.Width(), rClientRect.Height(), SWP_NOZORDER);
	m_dlgSearch.SetWindowPos(NULL, rClientRect.left, rClientRect.top,
							rClientRect.Width(), rClientRect.Height(), SWP_NOZORDER);
	m_wndChat.SetWindowPos(NULL, rClientRect.left, rClientRect.top,
							rClientRect.Width(), rClientRect.Height(), SWP_NOZORDER);
	m_wndIRC.SetWindowPos(NULL, rClientRect.left, rClientRect.top,
							rClientRect.Width(), rClientRect.Height(), SWP_NOZORDER);
	m_dlgStatistics.SetWindowPos(NULL, rClientRect.left, rClientRect.top,
								rClientRect.Width(), rClientRect.Height(), SWP_NOZORDER);

// 	Load connection state icon
	ShowConnectionState(false, _T(""), true);

// 	Anchors
	AddAnchor(m_wndServer,TOP_LEFT,BOTTOM_RIGHT);
	AddAnchor(m_wndTransfer,TOP_LEFT,BOTTOM_RIGHT);
	AddAnchor(m_wndSharedFiles,TOP_LEFT,BOTTOM_RIGHT);
	AddAnchor(m_dlgSearch,TOP_LEFT,BOTTOM_RIGHT);
	AddAnchor(m_wndChat,TOP_LEFT,BOTTOM_RIGHT);
	AddAnchor(m_wndIRC,TOP_LEFT,BOTTOM_RIGHT);
	AddAnchor(m_ctlStatusBar,BOTTOM_LEFT,BOTTOM_RIGHT);
	AddAnchor(m_dlgStatistics,TOP_LEFT,BOTTOM_RIGHT);
#ifdef USE_REBAR
	AddAnchor(m_ctlReBar,TOP_LEFT, TOP_RIGHT);
#else
	AddAnchor(m_ctlToolBar,TOP_LEFT, TOP_RIGHT);
#endif

	m_dlgStatistics.ShowInterval();

//	Set Tray icon
	TraySetMinimizeToTray(g_App.m_pPrefs->GetMinTrayPTR());
	TrayMinimizeToTrayChanged();

//	Update Categories
	m_dlgSearch.UpdateCatTabs();

	m_wndServer.ShowWindow(SW_SHOW);
	m_pdlgActive = &m_wndServer;

	g_App.m_pPrefs->SetSmartIdState(1);

//	Restore saved window placement
	WINDOWPLACEMENT wp;wp.length=sizeof(wp);
	wp=g_App.m_pPrefs->GetEmuleWindowPlacement();
	SetWindowPlacement(&wp);

	Localize();

	m_ttip.Create(this);
	m_ttip.SetDelayTime(TTDT_AUTOPOP, 15000);
	m_ttip.SetDelayTime(TTDT_INITIAL, g_App.m_pPrefs->GetToolTipDelay()*1000);
	m_ttip.SendMessage(TTM_SETMAXTIPWIDTH, 0, SHRT_MAX);
	m_ttip.SetBehaviour(PPTOOLTIP_MULTIPLE_SHOW);
	m_ttip.SetNotify(m_hWnd);
	m_ttip.AddTool(&m_ctlStatusBar, _T(""));

#ifdef OLD_SOCKETS_ENABLED
	g_App.m_pMMServer->Init();
#endif //OLD_SOCKETS_ENABLED

//	SplashScreen
	if (g_App.m_pPrefs->UseSplashScreen() && !g_App.m_pPrefs->GetStartMinimized())
	{
		CSplashScreen splash(this);
		splash.DoModal();
	}

	if (!SetTimer(INIT_TIMER_ID,300,NULL))
		AfxMessageBox(GetResString(IDS_ERR_FAILEDTIMER));

//	Set proper Scheduler Shift if option is set
	if (g_App.m_pPrefs->IsSCHEnabled())
	{
		CTime curr_t = CTime::GetCurrentTime();
		uint32 secs = curr_t.GetSecond() + 60*curr_t.GetMinute() + 60*60*curr_t.GetHour();
		bool bShift1;

		if (g_App.m_pPrefs->GetSCHShift1() > g_App.m_pPrefs->GetSCHShift2())
		{
			if (secs >= g_App.m_pPrefs->GetSCHShift1())
				bShift1 = true;
			else if (secs < g_App.m_pPrefs->GetSCHShift2())
				bShift1 = true;
			else
				bShift1 = false;
		}
		else
		{
			if (secs >= g_App.m_pPrefs->GetSCHShift1() && secs < g_App.m_pPrefs->GetSCHShift2())
				bShift1 = true;
			else
				bShift1 = false;
		}

		if(bShift1)
		{
		//	Switching to Shift1 speeds
			g_App.m_pUploadQueue->SCHShift1UploadCheck();
			g_App.m_pPrefs->SetMaxUpload(g_App.m_pPrefs->GetSCHShift1Upload());
			g_App.m_pPrefs->SetMaxDownload(g_App.m_pPrefs->GetSCHShift1Download());
			g_App.m_pPrefs->SetMaxConnections(g_App.m_pPrefs->GetSCHShift1conn());
			g_App.m_pPrefs->SetMaxDownloadConperFive(g_App.m_pPrefs->GetSCHShift15sec());

			CString MessageText;
			MessageText.Format( _T("SCHEDULER: switching to Shift 1 (Max Upload:%.1f Max Download:%.1f Max Connections:%i Max In 5 secs:%i)"),
								static_cast<double>(g_App.m_pPrefs->GetSCHShift1Upload()) / 10.0,
								static_cast<double>(g_App.m_pPrefs->GetSCHShift1Download()) / 10.0,
								g_App.m_pPrefs->GetSCHShift1conn(),
								g_App.m_pPrefs->GetSCHShift15sec() );
			g_App.AddLogLine(LOG_FL_DBG, MessageText);
		}
		else
		{
			int dayOfWeek = curr_t.GetDayOfWeek();

			if ((dayOfWeek==2 && g_App.m_pPrefs->IsSCHExceptMon())
				|| (dayOfWeek==3 && g_App.m_pPrefs->IsSCHExceptTue())
				|| (dayOfWeek==4 && g_App.m_pPrefs->IsSCHExceptWed())
				|| (dayOfWeek==5 && g_App.m_pPrefs->IsSCHExceptThu())
				|| (dayOfWeek==6 && g_App.m_pPrefs->IsSCHExceptFri())
				|| (dayOfWeek==7 && g_App.m_pPrefs->IsSCHExceptSat())
				|| (dayOfWeek==1 && g_App.m_pPrefs->IsSCHExceptSun()))
			{
				g_App.AddLogLine(LOG_FL_DBG, _T("SCHEDULER: day excepted!"));
			}
			else
			{
			//	Switching to Shift2 speeds
				g_App.m_pUploadQueue->SCHShift2UploadCheck();
				g_App.m_pPrefs->SetMaxUpload(g_App.m_pPrefs->GetSCHShift2Upload());
				g_App.m_pPrefs->SetMaxDownload(g_App.m_pPrefs->GetSCHShift2Download());
				g_App.m_pPrefs->SetMaxConnections(g_App.m_pPrefs->GetSCHShift2conn());
				g_App.m_pPrefs->SetMaxDownloadConperFive(g_App.m_pPrefs->GetSCHShift25sec());

				CString MessageText;
				MessageText.Format( _T("SCHEDULER: switching to Shift 2 (Max Upload:%.1f Max Download:%.1f Max Connections:%i Max In 5 secs:%i)"),
									static_cast<double>(g_App.m_pPrefs->GetSCHShift2Upload()) / 10.0,
									static_cast<double>(g_App.m_pPrefs->GetSCHShift2Download()) / 10.0,
									g_App.m_pPrefs->GetSCHShift2conn(),
									g_App.m_pPrefs->GetSCHShift25sec() );
				g_App.AddLogLine(LOG_FL_DBG, MessageText);
			}
		}
	}

	return TRUE;

	EMULE_CATCH2

	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::OnTimer(UINT_PTR nIDEvent)
{
	EMULE_TRY

	if (nIDEvent == INIT_TIMER_ID)
	{
		switch (m_iStatus)
		{
			case 0:	// State 0 - 300ms after InitDialog()
				m_iStatus++;
				g_App.m_pSharedFilesList->SetOutputCtrl(&m_wndSharedFiles.m_ctlSharedFilesList);
				break;

			case 1:	// State 1 - 300ms after the end of State 0
				m_iStatus++;
				g_App.m_pServerList->Init();
				break;

			case 2:	// State 2 - 300ms after the server list is init'ed.
				KillTimer(nIDEvent);
				m_iStatus++;
			//	Read in all the .met files and make them available for sharing
				g_App.m_pDownloadQueue->Init();
				g_App.m_pFakeCheck->Init();
#ifdef OLD_SOCKETS_ENABLED
				if (!g_App.m_pListenSocket->StartListening())
					AddLogLine(LOG_RGB_ERROR, IDS_MAIN_SOCKETERROR, g_App.m_pPrefs->GetPort());
				if (!g_App.m_pClientUDPSocket->Create())
					AddLogLine(LOG_RGB_ERROR, IDS_MAIN_SOCKETERROR, g_App.m_pPrefs->GetUDPPort());
#endif //OLD_SOCKETS_ENABLED

			//	Display own userhash
				AddLogLine(0, _T("%s: %s"), GetResString(IDS_INFLST_USER_USERHASH), HashToString(g_App.m_pPrefs->GetUserHash()));

			//	Display eMule Plus version
				AddLogLine(LOG_FL_SBAR, IDS_MAIN_READY, CURRENT_VERSION_LONG);

			//	Display used optimization
				UINT		dwResStrId;

				switch (get_cpu_type())
				{
					default:
					case 1:
						dwResStrId = IDS_OPTIMIZATION_NO;
						break;
					case 2:
						dwResStrId = IDS_OPTIMIZATION_MMX;
						break;
					case 3:
						dwResStrId = IDS_OPTIMIZATION_AMD;
						break;
					case 4:
					case 5:
						dwResStrId = IDS_OPTIMIZATION_SSE;
						break;
				}
				AddLogLine(0, dwResStrId);

			//	Start server anyway, but you'll get response		//	WHY do we do that ??
			//	only if it's enabled in preferences.
				g_App.m_pWebServer->StartServer();

			//	Process ed2k link
				if (g_App.m_pstrPendingLink != NULL)
				{
				//	Due to CString cloning m_pstrPendingLink and m_sendStruct point to the same string buffer
					OnWMData(NULL, (LPARAM)&g_App.m_sendStruct);
					delete g_App.m_pstrPendingLink;
					g_App.m_pstrPendingLink = NULL;
				}

			//	Get current version from internet, if one week elapsed
			//  Don't put lower elapse value, we don't need DDoS attacks! ;)
			//	(later move this to new engine)
				if ( g_App.m_pPrefs->IsAutoCheckForNewVersion() &&
					(time(NULL) - g_App.m_pPrefs->GetAutoCheckLastTime()) > 7 * 24 * 60 * 60 )
				{
					AfxBeginThread(CheckCurrentVersionAtInet, NULL, THREAD_PRIORITY_BELOW_NORMAL + g_App.m_pPrefs->GetMainProcessPriority(), 0, 0);
				}

			//	Start one second timer to update session time
				if (SetTimer(INIT_TIMER_ID, 1000, NULL) == 0)
					AfxMessageBox(GetResString(IDS_ERR_FAILEDTIMER));
				break;

			case 3:	// State 3 - 1sec after connect
			//	Show the session time in the status bar once a second.
				ShowSessionTime();
				break;

			default:
				AddLogLine(LOG_FL_DBG, _T("Timer called with no reason"));
		}
	}
	CTrayDialog::OnTimer(nIDEvent);

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	EMULE_TRY

	switch(nID & 0xFFF0)
	{
		case IDM_ABOUTBOX:
		{
			CAboutDlg		dlgAbout;

			dlgAbout.DoModal();
			break;
		}
		case SC_CLOSE:
			if(g_App.m_pPrefs->GetCloseToTray())
			{
				TrayShow();
				ShowWindow(SW_HIDE);
			}
			else
				CTrayDialog::OnSysCommand(nID, lParam);
			break;
		case SC_MINIMIZE:
		case SC_MINIMIZETRAY:
		case SC_RESTORE:
		case SC_MAXIMIZE:
			CTrayDialog::OnSysCommand(nID, lParam);
			ShowTransferRate(true);
			break;
		default:
			CTrayDialog::OnSysCommand(nID, lParam);
	}

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::OnPaint()
{
	EMULE_TRY

	if (!m_bStartUpMinimized)
	{
		m_bStartUpMinimized=true;

		if (g_App.m_pPrefs->GetStartMinimized())
			OnCancel();
	}

	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CTrayDialog::OnPaint();
	}

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HCURSOR CEmuleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::OnBnClickedButton2()
{
	EMULE_TRY

#ifdef OLD_SOCKETS_ENABLED
	if (!g_App.m_pServerConnect->IsConnected())
	{
	// Connect if not currently connected
		if (!g_App.m_pServerConnect->IsConnecting())
		{
			if (g_App.m_pServerConnect->IsICCActive())
			{
			//	Internet Connection Check is active, cancel it
				g_App.m_pServerConnect->Disconnect();
				ShowConnectionState(false);
			}
			else
				StartConnection();
		}
		else
		{
			g_App.m_pServerConnect->StopConnectionTry();
			ShowConnectionState(false);
		}
	}
	else
	// Disconnect if currently connected
		CloseConnection();
#endif //OLD_SOCKETS_ENABLED

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::OutputLogText(const CString& strLogText, CHTRichEditCtrl* pRichEditCtrl)
{
	EMULE_TRY

	if (!IsRunning())
		return;

	CString		strLine = strLogText, strPart;
	int			iTagStart, iTagEnd;
	COLORREF	crColor = CLR_DEFAULT, crNewColor;
	unsigned	uiDgt, uiColor, ui;
	const TCHAR	*pcColor;

	while (!strLine.IsEmpty())
	{
		iTagEnd = -1;
		iTagStart = strLine.Find(LOG_COLOR_TAG);
		if (iTagStart >= 0)
			iTagEnd = strLine.Find(_T('>'), iTagStart + LOG_COLOR_TAGLEN);
		if (iTagEnd >= 0)
		{
			crNewColor = ~0ul;
			strPart = strLine.Mid(iTagStart + LOG_COLOR_TAGLEN, iTagEnd - iTagStart - LOG_COLOR_TAGLEN);
			if (!strPart.IsEmpty())
			{
				if (strPart == _T("Default"))
					crNewColor = CLR_DEFAULT;
				else if (strPart.GetLength() == 6)
				{
					pcColor = strPart.GetString();
					for (ui = 0, uiColor = 0; ui < 6; ui++)
					{
						if ((uiDgt = HexChr2Num(pcColor[ui])) > 15)
							break;
						uiColor = (uiColor << 4u) | uiDgt;
					}
					if (ui == 6)
						crNewColor = RGB(uiColor >> 16, uiColor >> 8, uiColor);
				}
			}
			if (crNewColor == -1)
			{
				iTagStart += LOG_COLOR_TAGLEN;	//skip broken tag -- treat it as text
				iTagEnd = iTagStart - 1;
				crNewColor = crColor;
			}
			if (iTagStart > 0)
				pRichEditCtrl->AppendText(strLine.Left(iTagStart), crColor);
			crColor = crNewColor;
			strLine.Delete(0, iTagEnd + 1);
		}
		else
		{
			pRichEditCtrl->AppendText(strLine, crColor);
			break;
		}
	}

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::AddLogText(int iMode, const CString &strTxt)
{
	EMULE_TRY

	CString			strLogLine = strTxt;

	strLogLine.Remove(_T('\b'));
	strLogLine.Remove(_T('\r'));
	strLogLine.Remove(_T('\t'));
	strLogLine.Trim();

	if (strLogLine.IsEmpty())
		return;

	CString		strCleanLine, *pstrCleanLine;
	TCHAR		acColBuf[7];
	const TCHAR	*pcMsgColor = _T("Default");
	unsigned	uiCol, uiRGB;

	if (iMode < 0)	//LOG_FL_EMBEDFMT
	{
		int		iTagStart = 0, iTagEnd;

		strCleanLine = strLogLine;
		for (;;)
		{
			iTagStart = strCleanLine.Find(LOG_COLOR_TAG, iTagStart);
			if (iTagStart < 0)
				break;
			iTagEnd = strCleanLine.Find(_T('>'), iTagStart + LOG_COLOR_TAGLEN);
			if (iTagEnd >= 0)
				strCleanLine.Delete(iTagStart, iTagEnd - iTagStart + 1);
			else
				iTagStart++;
		}
		pstrCleanLine = &strCleanLine;
	}
	else	//one color is passed for the whole message
	{
		pstrCleanLine = &strLogLine;
		uiRGB = iMode & 0xFFFFFF;
		if (uiRGB != 0)	//not default color
		{
			if (uiRGB == LOG_RGB_BLACK)	//restore pure black
				uiRGB = RGB(0x00, 0x00, 0x00);
			uiCol = GetRValue(uiRGB);
			acColBuf[0] = g_acHexDigits[uiCol >> 4];
			acColBuf[1] = g_acHexDigits[uiCol & 0xf];
			uiCol = GetGValue(uiRGB);
			acColBuf[2] = g_acHexDigits[uiCol >> 4];
			acColBuf[3] = g_acHexDigits[uiCol & 0xf];
			uiCol = GetBValue(uiRGB);
			acColBuf[4] = g_acHexDigits[uiCol >> 4];
			acColBuf[5] = g_acHexDigits[uiCol & 0xf];
			acColBuf[6] = _T('\0');
			pcMsgColor = acColBuf;
		}
	}

	if ((iMode & (LOG_FL_SBAR | LOG_FL_DBG)) == LOG_FL_SBAR)
	{
		if (m_ctlStatusBar.m_hWnd && g_App.m_app_state != CEmuleApp::APP_STATE_SHUTTINGDOWN)
			m_ctlStatusBar.SetText(*pstrCleanLine, SB_MESSAGETEXT, 0);
	}

	COleDateTime	currentTime(COleDateTime::GetCurrentTime());
	CString			strLogLine2, strTime = currentTime.Format(_T("%c: "));

	strLogLine2.Format(RGB_DARK_BLUE_TXT _T("%s") LOG_COLOR_TAG _T("%s>%s\n"), strTime, pcMsgColor, strLogLine);

//	Save to file
	if (g_App.m_pPrefs->LogToFile())
	{
		FILE	*pLogFile = _tfsopen((iMode & LOG_FL_DBG) ? m_strDebugLogFilePath : m_strLogFilePath, _T("ab"), _SH_DENYWR);

		if (pLogFile != NULL)
		{
			strTime += *pstrCleanLine;
			strTime += _T("\r\n");
#ifdef _UNICODE
		//	Write the Unicode BOM in the beginning if file was created
			if (_filelength(_fileno(pLogFile)) == 0)
				fputwc(0xFEFF, pLogFile);
#endif
			_fputts(strTime, pLogFile);
			fclose(pLogFile);
		}
	}
	// 	Only when data save to file output them into GUI
	if ((iMode & LOG_FL_DBG) != 0)
	{
		if (m_pstrNewDebugLogTextLines)
			m_pstrNewDebugLogTextLines->Append(strLogLine2);

	//	post message only if box was created
		if(IsRunning() && ::IsWindow(m_wndServer.m_ctrlBoxSwitcher.m_hWnd))
			PostUniqueMessage(WM_DLOG_REFRESH);
	}
	else
	{
		if (m_pstrNewLogTextLines)
			m_pstrNewLogTextLines->Append(strLogLine2);

	//	post message only if box was created
		if(IsRunning() && ::IsWindow(m_wndServer.m_ctrlBoxSwitcher.m_hWnd))
			PostUniqueMessage(WM_LOG_REFRESH);

		ShowNotifier(*pstrCleanLine, TBN_LOG, false, g_App.m_pPrefs->GetUseLogNotifier());
	}

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::AddServerMessageLine(LPCTSTR line)
{
	EMULE_TRY

	CString strLine = line;

	strLine.Remove(_T('\b'));
	strLine.Remove(_T('\r'));
	strLine.Remove(_T('\t'));
	strLine.Trim();
	if ((!strLine.IsEmpty()) && (::IsWindow(m_wndServer.m_ctrlBoxSwitcher.m_hWnd)))
	{
		COleDateTime	currentTime(COleDateTime::GetCurrentTime());
		CString			strTime = currentTime.Format(_T("%c: "));

		m_wndServer.m_pctlServerMsgBox->AppendText(strTime, RGB(0, 0, 128), CLR_DEFAULT, HTC_HAVENOLINK);
		strLine += _T('\n');
		m_wndServer.m_pctlServerMsgBox->AppendText(strLine);
		if (m_wndServer.m_ctrlBoxSwitcher.GetCurSel() != 1)
			m_wndServer.m_ctrlBoxSwitcher.SetItemState(1, TCIS_HIGHLIGHTED, TCIS_HIGHLIGHTED);
	}

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::AddBugReport(LPCTSTR strFunctionName, LPCTSTR sFile, uint32 dwLine, LPCTSTR pcMsg)
{
	EMULE_TRY

	if (g_App.m_pPrefs->GetBugReport())
	{
		AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("%s: EXCEPTION! (%s) Report to ") CLIENT_NAME _T(" developers. Line %u in file %s"),
			strFunctionName, pcMsg, dwLine, sFile );
	}

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::ShowConnectionState()
{
#ifdef OLD_SOCKETS_ENABLED
	ShowConnectionState(g_App.m_pServerConnect->IsConnected(), _T(""));
#endif //OLD_SOCKETS_ENABLED
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::ShowConnectionState(bool bConnected, const CString &strServer /*=""*/, bool bIconOnly /*=false*/)
{
	static const uint16 s_auConnIconResID[] = { IDI_NOTCONNECTED, IDI_CONNECTED, IDI_CONNECTEDHIGH };

	EMULE_TRY

	if(g_App.m_app_state == g_App.APP_STATE_SHUTTINGDOWN)
		return;

	CString			strBuff;
	TBBUTTONINFO	tbi;
	int				iIconID;

	tbi.dwMask = TBIF_IMAGE | TBIF_TEXT;
	tbi.cbSize = sizeof(TBBUTTONINFO);

	if (bConnected)
	{
		if (!bIconOnly)
		{
			GetResString(&strBuff, IDS_MAIN_CONNECTEDTO);
			strBuff += _T(' ');
			strBuff += strServer;
			m_ctlStatusBar.SetText(strBuff, SB_MESSAGETEXT, 0);
			m_ctlStatusBar.SetText(strServer, SB_SERVER, 0);
		}
		GetResString(&strBuff, IDS_MAIN_BTN_DISCONNECT);
		tbi.iImage = 9;
		tbi.pszText = const_cast<LPTSTR>(strBuff.GetString());
		m_ctlToolBar.SetButtonInfo(IDC_TOOLBARBUTTON + 0, &tbi);
		m_ctlToolBar.AutoSize();

#ifdef OLD_SOCKETS_ENABLED
		iIconID = (g_App.m_pServerConnect->IsLowID()) ? 1 : 2;
#endif //OLD_SOCKETS_ENABLED
	}
	else
	{
#ifdef OLD_SOCKETS_ENABLED
		if (g_App.m_pServerConnect->IsConnecting())
		{
			if (!bIconOnly)
			{
				GetResString(&strBuff, IDS_CONNECTING);
				m_ctlStatusBar.SetText(strBuff, SB_SERVER, 0);
			}
			GetResString(&strBuff, IDS_MAIN_BTN_CANCEL);
			tbi.iImage = 10;
		}
		else
		{
			if (!bIconOnly)
			{
				GetResString(&strBuff, IDS_NOTCONNECTED);
				m_ctlStatusBar.SetText(strBuff, SB_SERVER, 0);
				AddLogLine(0, IDS_DISCONNECTED);
			}
			GetResString(&strBuff, IDS_MAIN_BTN_CONNECT);
			tbi.iImage = 0;
		}
		tbi.pszText = const_cast<LPTSTR>(strBuff.GetString());
		m_ctlToolBar.SetButtonInfo(IDC_TOOLBARBUTTON + 0, &tbi);
		iIconID = 0;
		ShowUserCount(0);
#endif //OLD_SOCKETS_ENABLED
	}
	HICON	hPrevIcon = m_hiconConn;

	m_hiconConn = (HICON)::LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(s_auConnIconResID[iIconID]), IMAGE_ICON, 16, 16, 0);
	m_ctlStatusBar.SetIcon(SB_SERVER, m_hiconConn);
	if (hPrevIcon != NULL)
		::DestroyIcon(hPrevIcon);

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::ShowSessionTime()
{
	uint32		dwRunTimeSecs = static_cast<uint32>((::GetTickCount() - g_App.stat_starttime) / 1000);
	CTimeSpan	runTime(static_cast<__time64_t>(dwRunTimeSecs));
	CString		strBuffer;
	unsigned	uiDays, uiHours, uiMinutes;

	uiHours = static_cast<unsigned>(runTime.GetHours());
	uiMinutes = static_cast<unsigned>(runTime.GetMinutes());
	uiDays = static_cast<unsigned>(runTime.GetDays());

	if (uiDays == 0)
		strBuffer.Format(_T("%02u:%02u:%02u"), uiHours, uiMinutes, runTime.GetSeconds());
	else
		strBuffer.Format( _T("%2u %s %02u:%02u"), uiDays,
			GetResString((uiDays == 1) ? IDS_LONGDAY : IDS_LONGDAYS), uiHours, uiMinutes );

	m_ctlStatusBar.SetText(strBuffer, SB_SESSIONTIME, 0);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::ShowUserCount(uint32 dwCount)
{
	EMULE_TRY

	if (g_App.m_app_state == g_App.APP_STATE_SHUTTINGDOWN)
		return;

	CString	sBuffer;

	sBuffer.Format(_T("%u"), dwCount);
	m_ctlStatusBar.SetText(sBuffer, SB_NUMUSERS, 0);

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::ShowMessageState(byte iconnr)
{
	EMULE_TRY

	m_ctlStatusBar.SetIcon(SB_MESSAGESTATUS,m_hiconIM[iconnr]);

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//v- eklmn: bugfix(11): crazy output in statusbar (update only tray or window)
void CEmuleDlg::ShowTransferRate(bool bUpdateAll)
{
	EMULE_TRY

	CString sBuffer, sUpload, sDownload;

//	More detailed upload speed: 5 sec average instead of 40 sec average (calculated only once)
	double lastuprate_kB = static_cast<double>(g_App.m_pUploadQueue->GetDataRate())/1024.0;
	double lastdownrate_kB = static_cast<double>(g_App.m_pDownloadQueue->GetDataRate())/1024.0;

	double lastuprateoverhead_kB = static_cast<double>(g_App.m_pUploadQueue->GetUpDataRateOverhead())/1024.0;
	double lastdownrateoverhead_kB = static_cast<double>(g_App.m_pDownloadQueue->GetDownDataRateOverhead())/1024.0;

//	String preparation doesn't depend on output
	if (g_App.m_pPrefs->ShowOverhead())
	{
		sBuffer.Format(GetResString(IDS_UPDOWNLONG),lastuprate_kB, lastuprateoverhead_kB, lastdownrate_kB, lastdownrateoverhead_kB);
		sUpload.Format(_T("%.2f (%.2f)"),lastuprate_kB, lastuprateoverhead_kB);
		sDownload.Format(_T("%.2f (%.2f)"),lastdownrate_kB, lastdownrateoverhead_kB);
	}
	else
	{
		sBuffer.Format(GetResString(IDS_UPDOWN),lastuprate_kB, lastdownrate_kB);
		sUpload.Format(_T("%.2f"),lastuprate_kB);
		sDownload.Format(_T("%.2f"),lastdownrate_kB);
	}

//	Set SpeedMeterValues if option is activated
	if(g_App.m_pPrefs->GetShowToolbarSpeedMeter())
		m_ctlToolBar.SetSpeedMeterValues((int)lastuprate_kB, (int)lastdownrate_kB);

//	Update window only
	if (IsWindowVisible() || bUpdateAll)
	{
		m_ctlStatusBar.SetText(sUpload,SB_UPLOADRATE,0);
		m_ctlStatusBar.SetText(sDownload,SB_DOWNLOADRATE,0);
		m_ctlStatusBar.SetIcon(SB_UPLOADRATE,m_hiconTrans[(lastuprate_kB)?1:0]);
		m_ctlStatusBar.SetIcon(SB_DOWNLOADRATE,m_hiconTrans[(lastdownrate_kB)?3:2]);

		m_wndTransfer.UpdateDownloadHeader();
		m_wndTransfer.UpdateUploadHeader();
	}

//	Update tray only
	if (TrayIsVisible() || bUpdateAll)
	{
	//	Set trayicon-icon
		int DownRateProcent = (int)ceil(lastdownrate_kB * 1000 / g_App.m_pPrefs->GetMaxGraphDownloadRate());
		if (DownRateProcent>100) DownRateProcent=100;

		TraySetToolTip(sBuffer);

	//	It's better to do TrayUpdate here, because of TrayToolTip
		UpdateTrayIcon(DownRateProcent);
	}
	if (!TrayIsVisible() && g_App.m_pPrefs->ShowRatesOnTitle())
	{
		CString		strTitle;

		strTitle.Format(_T("(U:%.1f D:%.1f) ") CLIENT_NAME_WITH_VER, lastuprate_kB, lastdownrate_kB);
		SetWindowText(strTitle);
	}

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::OnCancel()
{
	EMULE_TRY

	if (*g_App.m_pPrefs->GetMinTrayPTR())
	{
		TrayShow();
		ShowWindow(SW_HIDE);
	}
	else
	{
		ShowWindow(SW_MINIMIZE);
	}
	ShowTransferRate();

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized)
{
	CWnd::OnActivate(nState, pWndOther, bMinimized);

//	Update chat and disable taskbar blinking indicator on application activation
	if ((nState != WA_INACTIVE) && (m_pdlgActive == &m_wndChat))
		g_App.m_pMDlg->m_wndChat.m_ctlChatSelector.ShowChat();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::SetActiveDialog(CDialog* dlg)
{
	EMULE_TRY

	if(dlg == m_pdlgActive)
		return;

	m_pdlgActive->ShowWindow(SW_HIDE);
	dlg->ShowWindow(SW_SHOW);
	dlg->SetFocus();
	m_pdlgActive = dlg;

	if (dlg==&m_wndTransfer)
 	{
		m_wndTransfer.UpdateCatTabTitles();
		m_ctlToolBar.PressMuleButton(IDC_TOOLBARBUTTON + MTB_TRANSFER);
	}
	else if (dlg==&m_wndServer)
		m_ctlToolBar.PressMuleButton(IDC_TOOLBARBUTTON + MTB_SERVERS);
	else if (dlg==&m_dlgSearch)
		m_ctlToolBar.PressMuleButton(IDC_TOOLBARBUTTON + MTB_SEARCH);
	else if (dlg==&m_wndSharedFiles)
		m_ctlToolBar.PressMuleButton(IDC_TOOLBARBUTTON + MTB_SHAREDFILES);
	else if (dlg==&m_wndChat)
		m_ctlToolBar.PressMuleButton(IDC_TOOLBARBUTTON + MTB_MESSAGES);
	else if (dlg==&m_wndIRC)
		m_ctlToolBar.PressMuleButton(IDC_TOOLBARBUTTON + MTB_IRC);
	else if (dlg==&m_dlgStatistics)
	{
		m_ctlToolBar.PressMuleButton(IDC_TOOLBARBUTTON + MTB_STATISTICS);
		m_dlgStatistics.ShowStatistics(true);
	}

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::OnSize(UINT nType,int cx,int cy)
{
	EMULE_TRY

	CTrayDialog::OnSize(nType,cx,cy);
	ResizeStatusBar();

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::ResizeStatusBar()
{
	CRect rect;
	m_ctlStatusBar.GetClientRect(&rect);

	int		iWidths[SB_NUMSBPARTS] =
	{
		0, 105, 22, 80, 70, 70, 258
	};

	int		iRightEdges[SB_NUMSBPARTS];

	if (g_App.m_pPrefs->ShowOverhead())
	{
		iWidths[SB_UPLOADRATE] = 105;
		iWidths[SB_DOWNLOADRATE] = 105;
		iWidths[SB_SERVER] = 188;
	}

//	Calculate the right edges given the widths
	iRightEdges[SB_NUMSBPARTS-1] = -1;	//Indicates right edge of the status bar
	iRightEdges[SB_NUMSBPARTS-2] = rect.right - iWidths[SB_NUMSBPARTS-1];
	for (int i = SB_NUMSBPARTS-3; i >= 0; i--)
	{
		iRightEdges[i] = iRightEdges[i+1] - iWidths[i+1];
	}
//	Set the status bar part sizes
	m_ctlStatusBar.SetParts(SB_NUMSBPARTS, iRightEdges);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CEmuleDlg::OnWMData(WPARAM wParam, LPARAM lParam)
{
	NOPRM(wParam);

	EMULE_TRY

	tagCOPYDATASTRUCT	*pCLData = reinterpret_cast<tagCOPYDATASTRUCT*>(lParam);

	if (pCLData->dwData == OP_ED2KLINK)
	{
		FlashWindow(true);
		if (g_App.m_pPrefs->IsBringToFront())
		{
			if (IsIconic())
				ShowWindow(SW_RESTORE);
			else if (TrayHide())
				ShowWindow(SW_SHOW);
			else
				SetForegroundWindow();
		}
		try
		{
			CED2KLink	*pLink = CED2KLink::CreateLinkFromUrl(CString(reinterpret_cast<LPCTSTR>(pCLData->lpData), pCLData->cbData / sizeof(TCHAR)));

			_ASSERT(pLink != NULL);
			switch (pLink->GetKind())
			{
				case CED2KLink::kFile:
				{
					CED2KFileLink	*pFileLink = pLink->GetFileLink();

					g_App.m_pDownloadQueue->AddFileLinkToDownload(pFileLink);
					break;
				}
				case CED2KLink::kServerList:
				{
					CED2KServerListLink	*pListLink = pLink->GetServerListLink();
					CString	strAddress = pListLink->GetAddress();

					if (!strAddress.IsEmpty())
						g_App.m_pMDlg->m_wndServer.UpdateServerMetFromURL(strAddress);
					break;
				}
				case CED2KLink::kServer:
				{
					CString	strDefName;
					CED2KServerLink	*pSrvLink = pLink->GetServerLink();
					CServer	*pSrv = new CServer(pSrvLink->GetPort(), pSrvLink->GetIP());

					pSrvLink->GetDefaultName(strDefName);
					pSrv->SetListName(strDefName);
					if (g_App.m_pPrefs->GetManuallyAddedServerHighPrio())
						pSrv->SetPreference(PR_HIGH);

					if (!g_App.m_pMDlg->m_wndServer.m_ctlServerList.AddServer(pSrv, true))
						delete pSrv;
					else
						AddLogLine(LOG_FL_SBAR, IDS_SERVERADDED, pSrv->GetListName());
					break;
				}
				default:
					break;
			}
			delete pLink;
		}
		catch(...)
		{
			OUTPUT_DEBUG_TRACE();
			AddLogLine(LOG_FL_SBAR, IDS_LINKNOTADDED);
		}
		return true;
	}
	else if (pCLData->dwData == OP_CLCOMMAND)
	{
	//	Command line command received
		CString	strCL(reinterpret_cast<LPCTSTR>(pCLData->lpData), pCLData->cbData / sizeof(TCHAR));

		strCL.MakeLower();
		AddLogLine(LOG_FL_SBAR, _T("CLI: %s"), strCL);

		if (strCL == _T("connect"))
		{
			StartConnection();
			return true;
		}
#ifdef OLD_SOCKETS_ENABLED
		else if (strCL == _T("disconnect"))
		{
			if (g_App.m_pServerConnect->IsConnecting())
				g_App.m_pServerConnect->StopConnectionTry();
			else
				g_App.m_pServerConnect->Disconnect();

			ShowConnectionState(g_App.m_pServerConnect->IsConnected(), _T(""), true);
			return true;
		}
#endif //OLD_SOCKETS_ENABLED
		else if (strCL == _T("resume"))
		{
			g_App.m_pDownloadQueue->StartNextFile(CCat::GetAllCatType());
			return true;
		}
		else if (strCL == _T("exit"))
		{
			m_bCliExit = true;
			OnClose();
			return true;
		}
		else if (strCL == _T("reload"))
		{
			g_App.m_pSharedFilesList->Reload();
			return true;
		}
		else if (strCL == _T("restore"))
		{
			RestoreWindow();
			return true;
		}
		else if (strCL == _T("preferences"))
		{
			g_App.m_pPrefs->Save();
			return true;
		}
		else if ((strCL.GetLength() > 8) && (strCL.Left(7) == _T("limits=")))
		{
			int	iPos = strCL.Find(_T(':'), 7);

			if (iPos >= 7)
			{
				if ((iPos + 1) < strCL.GetLength())	//	':' isn't last char (download substring isn't empty)
					g_App.m_pPrefs->SetMaxDownloadWithCheck(String2FranctionalRate(strCL.GetString() + iPos + 1));
			}
			if (iPos != 7)	//	upload substring isn't empty
				g_App.m_pPrefs->SetMaxUploadWithCheck(String2FranctionalRate(strCL.GetString() + 7));
			return true;
		}
		else if (strCL == _T("status"))
		{
			CString	strBuffer(g_App.m_pPrefs->GetAppDir());

			strBuffer += _T("status.log");

			FILE	*pOutput = _tfsopen(strBuffer, _T("wb"), _SH_DENYWR);

			if (pOutput != NULL)
			{
#ifdef _UNICODE
			//	Write Unicode byte-order mark 0xFEFF
				fputwc(0xFEFF, pOutput);
#endif
#ifdef OLD_SOCKETS_ENABLED
				if (g_App.m_pServerConnect->IsConnected())
					GetResString(&strBuffer, IDS_CONNECTED);
				else if (g_App.m_pServerConnect->IsConnecting())
					GetResString(&strBuffer, IDS_CONNECTING);
				else
					GetResString(&strBuffer, IDS_DISCONNECTED);
#else
				strBuffer = _T("");
#endif //OLD_SOCKETS_ENABLED

				_fputts(strBuffer, pOutput);
				_fputts(_T("\r\n"), pOutput);
				_ftprintf(pOutput, GetResString(IDS_UPDOWN), static_cast<double>(g_App.m_pUploadQueue->GetDataRate()) / 1024.0, static_cast<double>(g_App.m_pDownloadQueue->GetDataRate()) / 1024.0);
				_fputts(_T("\r\n"), pOutput);
				_fputts(g_App.m_pDownloadList->GetPartFilesStatusString(), pOutput);

				fclose(pOutput);
			}
			return true;
		}
	}
	EMULE_CATCH2

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CEmuleDlg::OnFileHashed(WPARAM wParam, LPARAM lParam)
{
	EMULE_TRY

	CKnownFile	*pNewKnownFile = reinterpret_cast<CKnownFile*>(lParam);

	if (wParam != 0)
	{
		CPartFile	*pPartFile = reinterpret_cast<CPartFile*>(wParam);

		pPartFile->PartFileHashFinished(pNewKnownFile);
	}
	else
	{
		g_App.m_pSharedFilesList->FileHashingFinished(pNewKnownFile);
	}
	return true;

	EMULE_CATCH2

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CEmuleDlg::OnHashFailed(WPARAM wParam, LPARAM lParam)
{
	NOPRM(wParam);

	g_App.m_pSharedFilesList->FileHashingFailed(reinterpret_cast<UnknownFile_Struct*>(lParam));
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CEmuleDlg::OnFileHashingStarted(WPARAM wParam, LPARAM lParam)
{
	NOPRM(lParam);

	CPartFile	*pPartFile = reinterpret_cast<CPartFile*>(wParam);

	if (pPartFile != NULL)
		pPartFile->FileRehashingStarted();
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::OnClose()
{
	EMULE_TRY

//	No GUI confirmation for the exit through CLI to achieve proper automation
	if ( (g_App.m_app_state == g_App.APP_STATE_RUNNING) &&
		!m_bCliExit && g_App.m_pPrefs->IsConfirmExitEnabled() &&
		( IDNO == MessageBox( GetResString(IDS_MAIN_EXIT), GetResString(IDS_MAIN_EXITTITLE),
		MB_YESNO | MB_DEFBUTTON2 ) ) )
	{
		return;
	}
// 	We should call this before changing the state to APP_STATE_SHUTTINGDOWN because
// 	otherwise any log message inside this function (and there are few) will produce
// 	message box
#ifdef OLD_SOCKETS_ENABLED
	if (g_App.m_pServerConnect->IsConnected())
		g_App.m_pServerConnect->Disconnect();
#endif //OLD_SOCKETS_ENABLED

	g_App.m_app_state = g_App.APP_STATE_SHUTTINGDOWN;

	g_App.OnlineSig();
	g_App.m_pDownloadQueue->SaveAllSLSFiles();

	WINDOWPLACEMENT wp;wp.length=sizeof(wp);GetWindowPlacement(&wp);
	g_App.m_pPrefs->SetWindowLayout(wp);

//	Disable redraws while shutting down, should take care of some crashes on exit
	m_wndTransfer.m_ctlDownloadList.SetRedraw(false);
	m_wndTransfer.m_ctlUploadList.SetRedraw(false);
	m_wndTransfer.m_ctlQueueList.SetRedraw(false);
	m_wndTransfer.m_ctlClientList.SetRedraw(false);
	m_dlgSearch.m_ctlSearchList.SetRedraw(false);
	m_wndSharedFiles.m_ctlSharedFilesList.SetRedraw(false);
	m_wndIRC.serverChannelList.SetRedraw(false);
	m_wndTransfer.m_ctlInfoList.SetRedraw(false);
	m_wndChat.m_FriendListCtrl.SetRedraw(false);
	m_wndChat.m_ctlChatSelector.SetRedraw(false);
	m_dlgStatistics.SetRedraw(false);
	m_wndTransfer.SetRedraw(false);
	m_dlgSearch.SetRedraw(false);
	m_wndSharedFiles.SetRedraw(false);
	m_wndIRC.SetRedraw(false);
	m_wndChat.SetRedraw(false);

	m_wndTransfer.m_ctlDownloadList.SaveSettings(CPreferences::TABLE_DOWNLOAD);
	m_wndTransfer.m_ctlUploadList.SaveSettings(CPreferences::TABLE_UPLOAD);
	m_wndTransfer.m_ctlQueueList.SaveSettings(CPreferences::TABLE_QUEUE);
	m_wndTransfer.m_ctlClientList.SaveSettings(CPreferences::TABLE_CLIENTLIST);
	m_dlgSearch.m_ctlSearchList.SaveSettings(CPreferences::TABLE_SEARCH);
	m_wndSharedFiles.m_ctlSharedFilesList.SaveSettings(CPreferences::TABLE_SHARED);
	m_wndServer.m_ctlServerList.SaveSettings(CPreferences::TABLE_SERVER);
	m_wndIRC.serverChannelList.SaveSettings(CPreferences::TABLE_IRC);
	m_wndIRC.m_ctlNickList.SaveSettings(CPreferences::TABLE_IRCNICK);
	m_wndChat.m_FriendListCtrl.SaveSettings(CPreferences::TABLE_FRIENDLIST);
	m_wndTransfer.SaveRollupItemHeights();

//	Close preferences window if it is open, otherwise it can result in a crash
//	Such closure can be initiated for example by MobileMule shutdown request
	if (m_dlgPreferences.m_hWnd != NULL)
		m_dlgPreferences.PressButton(PSBTN_CANCEL);

	g_App.m_pPrefs->Add2TotalDownloaded(g_App.stat_sessionReceivedBytes);
	g_App.m_pPrefs->Add2TotalUploaded(g_App.stat_sessionSentBytes);
	g_App.m_pPrefs->Save();

	g_App.m_pMDlg->m_dlgSearch.SaveSearchStrings();

//	Restore old registry if required
	if (g_App.m_pPrefs->AutoTakeED2KLinks())
		RevertReg();

//	Remove all list items
	m_wndSharedFiles.m_ctlSharedFilesList.DeleteAllItems();
	m_wndTransfer.m_ctlClientList.DeleteAllItems();
	m_wndTransfer.m_ctlUploadList.DeleteAllItems();
	m_wndTransfer.m_ctlQueueList.DeleteAllItems();
	m_wndTransfer.m_ctlDownloadList.DeleteAllItems();
	m_wndChat.m_ctlChatSelector.DeleteAllItems();
	m_dlgSearch.m_ctlSearchList.DeleteAllItems();

	ShowWindow(SW_HIDE);

//	All dialog windows created should be destroyed
	m_wndServer.DestroyWindow();
	m_wndTransfer.DestroyWindow();
	m_dlgSearch.DestroyWindow();
	m_wndSharedFiles.DestroyWindow();
	m_wndIRC.DestroyWindow();
	m_wndChat.DestroyWindow();
	m_dlgStatistics.DestroyWindow();

	g_App.m_pClientList->DeleteAll();

//	Moved here, should avoid crashes when closing & downloading.
	g_App.m_pKnownFilesList->Save();

	CTrayDialog::OnCancel();

//	Backup on exit
	if (g_App.m_pPrefs->IsAutoBackup())
	{
		g_App.m_pMDlg->RunBackupNow(true);
	}

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::OnTrayRButtonUp(CPoint pt)
{
	EMULE_TRY

	if(m_pSystrayDlg)
	{
		m_pSystrayDlg->BringWindowToTop();
		return;
	}

	m_pSystrayDlg = new CMuleSystrayDlg(this, pt);
	if(m_pSystrayDlg)
	{
		UINT nResult = m_pSystrayDlg->DoModal();
		delete m_pSystrayDlg;
		m_pSystrayDlg = NULL;
		switch(nResult)
		{
			case IDC_TOMAX:
				QuickSpeedOther(MP_QS_UA);
				break;
			case IDC_TOMIN:
				QuickSpeedOther(MP_QS_PA);
				break;
			case IDC_RESTORE:
				RestoreWindow();
				break;
			case IDC_CONNECT:
				StartConnection();
				break;
			case IDC_DISCONNECT:
				CloseConnection();
				break;
			case IDC_EXIT:
				OnClose();
				break;
			case IDC_PREFERENCES:
			{
				static int iOpen = 0;
				if(!iOpen)
				{
					iOpen = 1;
					m_dlgPreferences.DoModal();
					iOpen = 0;
				}
				break;
			}
			default:
				break;
		}
	}

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	StartConnection() initiates a connection attempt to a server.
void CEmuleDlg::StartConnection()
{
	EMULE_TRY

	AddLogLine(LOG_FL_SBAR, IDS_CONNECTING);
//	If there's more than one server selected in the server list...
	if (m_wndServer.m_ctlServerList.GetSelectedCount() > 1)
		m_wndServer.m_ctlServerList.PostMessage(WM_COMMAND, MP_CONNECTTO, 0L);
#ifdef OLD_SOCKETS_ENABLED
	else
		g_App.m_pServerConnect->ConnectToAnyServer();
#endif //OLD_SOCKETS_ENABLED
	ShowConnectionState(false);

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::CloseConnection()
{
	EMULE_TRY
	if (g_App.m_pServerConnect->IsConnected())
	{
		if( !g_App.m_pPrefs->IsConfirmDisconnectEnabled() ||
			( MessageBox(GetResString(IDS_BACKUP_SURE), GetResString(IDS_IRC_DISCONNECT),
			MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES ) )
		{
#ifdef OLD_SOCKETS_ENABLED
			g_App.m_pServerConnect->Disconnect();
#endif //OLD_SOCKETS_ENABLED
		}
	}
	else if (g_App.m_pServerConnect->IsConnecting())
	{
#ifdef OLD_SOCKETS_ENABLED
		g_App.m_pServerConnect->StopConnectionTry();
#endif //OLD_SOCKETS_ENABLED
	}
	g_App.OnlineSig();

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::RestoreWindow()
{
	EMULE_TRY

	if (IsIconic())
		ShowWindow(SW_RESTORE);
	else
	{
		if (TrayIsVisible())
			TrayHide();
		ShowWindow(SW_SHOW);
	}

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::UpdateTrayIcon(int procent)
{
	EMULE_TRY

//	Set the limits of where the bar color changes (low-high)
	int pLimits16[1] = {100};

// 	Set the corresponding color for each level
	COLORREF pColors16[1] = { RGB(1,255,1) };

//	Start it up
#ifdef OLD_SOCKETS_ENABLED
	if(g_App.m_pServerConnect->IsConnected() && !g_App.m_pServerConnect->IsLowID())
		m_trayMeterIcon.Init(m_hiconSourceTray,100,1,1,16,16, RGB(37,97,37));
	else if(g_App.m_pServerConnect->IsConnected() && g_App.m_pServerConnect->IsLowID())
		m_trayMeterIcon.Init(m_hiconSourceTrayLowID,100,1,1,16,16, RGB(37,97,37));
	else
#endif //OLD_SOCKETS_ENABLED
		m_trayMeterIcon.Init(m_hiconSourceTrayGrey,100,1,1,16,16, RGB(37,97,37));

//	Load our limit and color info
	m_trayMeterIcon.SetColorLevels(pLimits16,pColors16,1);

// 	Generate the icon (destroy these icons using DestroyIcon())
	int pVals16[1] = {procent};

	m_hiconMyTray = m_trayMeterIcon.Create(pVals16);
	ASSERT (m_hiconMyTray != NULL);
	if (m_hiconMyTray != NULL)
		TraySetIcon(m_hiconMyTray, true);
	TrayUpdate();

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CEmuleDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	EMULE_TRY

	return CTrayDialog::OnCreate(lpCreateStruct);

	EMULE_CATCH2

	return -1;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TaskbarNotifier
void CEmuleDlg::ShowNotifier(const CString &strText, int iMsgType, bool bForceSoundOFF, bool bMsgEnabled)
{
	EMULE_TRY

	if (bMsgEnabled && m_wndTaskbarNotifier.m_bStarted)
	{
		CString strTempMessage, strFinalMessage;
		int iPos = 0;

		for (;;)
		{
			strTempMessage = strText.Tokenize(_T(" "), iPos);
			if (strTempMessage.IsEmpty())
				break;
			int iStrLength = strTempMessage.GetLength();
			int iStrLineBreak = 30;

			while (iStrLength > 30)
			{
				strTempMessage.Insert(iStrLineBreak, _T('\n'));
				iStrLineBreak += 30;
				iStrLength -= 30;
			}

			strFinalMessage += strTempMessage;
			strFinalMessage += _T(' ');
		}

		strFinalMessage.TrimRight();

		m_wndTaskbarNotifier.Show(strFinalMessage, iMsgType, 500, g_App.m_pPrefs->NotificationDisplayTime());

		if (g_App.m_pPrefs->GetUseSoundInNotifier() && !bForceSoundOFF)
			PlaySound(g_App.m_pPrefs->GetNotifierWavSoundPath(), NULL, SND_FILENAME | SND_NOSTOP | SND_NOWAIT | SND_ASYNC);
	}

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CEmuleDlg::OnTaskbarNotifierClicked(WPARAM wParam, LPARAM lParam)
{
	NOPRM(wParam); NOPRM(lParam);

	EMULE_TRY

	int msgType = m_wndTaskbarNotifier.GetMessageType();

	switch(msgType)
	{
		case TBN_CHAT:
			RestoreWindow();
			SetActiveDialog(&m_wndChat);
			SetFocus();
			break;

		case TBN_DLOAD:
		case TBN_DLOAD_ADD:
			RestoreWindow();
			SetActiveDialog(&m_wndTransfer);
			SetFocus();
			break;

		case TBN_IMPORTANTEVENT:
		case TBN_LOG:
			RestoreWindow();
			SetActiveDialog(&m_wndServer);
			SetFocus();
			break;
	}
	return 0;

	EMULE_CATCH2

	return -1;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::Localize()
{
	EMULE_TRY

	if(m_hWnd)
	{
		m_ctlToolBar.Localize();

		if (!m_ctlStatusBar)
			return;

#ifdef OLD_SOCKETS_ENABLED
		if(g_App.m_pServerConnect->IsConnected())
		{
			CServer	*pCurServer = g_App.m_pServerConnect->GetCurrentServer();

			ShowUserCount(pCurServer->GetNumUsers());
		}
		else
		{
			m_ctlStatusBar.SetText(_T("0"), SB_NUMUSERS, 0);
			if(g_App.m_pServerConnect->IsConnecting())
				m_ctlStatusBar.SetText(GetResString(IDS_CONNECTING),SB_SERVER,0);
			else
				m_ctlStatusBar.SetText(GetResString(IDS_NOTCONNECTED),SB_SERVER,0);
		}
#endif //OLD_SOCKETS_ENABLED
		ShowTransferRate(true);
	}

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	DisConnect() toggles the server connection on and off.
void CEmuleDlg::DisConnect()
{
	EMULE_TRY

#ifdef OLD_SOCKETS_ENABLED
	if (!g_App.m_pServerConnect->IsConnected())
	{
	//	Connect if not currently connected
		if (!g_App.m_pServerConnect->IsConnecting())
		{
			if (g_App.m_pServerConnect->IsICCActive())
			{
			//	Internet Connection Check is active, cancel it
				g_App.m_pServerConnect->Disconnect();
				ShowConnectionState(false);
			}
			else
				StartConnection();
		}
		else
		{
			g_App.m_pServerConnect->StopConnectionTry();
			ShowConnectionState(false);
		}
	}
	else
#endif //OLD_SOCKETS_ENABLED
	{
	//	Disconnect if currently connected
		CloseConnection();
	}

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CEmuleDlg::PreTranslateMessage(MSG* pMsg)
{
	if (g_App.m_pPrefs->GetToolTipDelay() != 0)
		m_ttip.RelayEvent(pMsg);

	return CTrayDialog::PreTranslateMessage(pMsg);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::GetInfo4ToolTip(NM_PPTOOLTIP_DISPLAY *pNotify)
{
	int						iControlId = CWnd::FromHandle(pNotify->ti->hWnd)->GetDlgCtrlID();

	if (iControlId == IDC_STATUSBAR)
	{
		CString		strInfo;
		CPoint		pt;

		::GetCursorPos(&pt);
		m_ctlStatusBar.ScreenToClient(&pt);
		switch (m_ctlStatusBar.GetPaneAtPosition(pt))
		{
			case SB_MESSAGETEXT:
			{
				CString		strLogText = g_App.m_pMDlg->m_wndServer.m_pctlLogBox->GetToolTip();

				if (!strLogText.IsEmpty())
				{
					strInfo.Format(_T("<t=1><b>%s</b><br><hr=100%%><br>%s"), GetResString(IDS_SV_LOG), strLogText);
					pNotify->ti->hIcon = g_App.m_pMDlg->m_wndServer.m_imageList.ExtractIcon(0);
				}
				break;
			}
			case SB_SESSIONTIME:
			{
				uint32		dwRunTimeSecs = static_cast<uint32>((::GetTickCount() - g_App.stat_starttime) / 1000);

				CTimeSpan	runTime(static_cast<__time64_t>(dwRunTimeSecs));
				CString		strRuntime;
				unsigned	uiDays, uiHours, uiMinutes;

				uiHours = static_cast<unsigned>(runTime.GetHours());
				uiMinutes = static_cast<unsigned>(runTime.GetMinutes());
				uiDays = static_cast<unsigned>(runTime.GetDays());

				if (uiDays == 0)
				{
					strRuntime.Format(_T("%02u:%02u:%02u"), uiHours, uiMinutes, runTime.GetSeconds());
				}
				else
				{
					strRuntime.Format( _T("%u %s %02u:%02u"), uiDays,
						GetResString((uiDays == 1) ? IDS_LONGDAY : IDS_LONGDAYS),
						uiHours, uiMinutes );
				}

				strInfo.Format( _T("<t=1><b>%s</b><br><hr=100%%><br><b>%s:</b><t>%s"), GetResString(IDS_STATS_TIMESTATS),
					GetResString(IDS_STATS_RUNTIME), strRuntime );

				if (g_App.stat_serverConnectTime > 0)
				{
					uint32 dwServerDurationSecs = static_cast<uint32>((::GetTickCount() - g_App.stat_serverConnectTime) / 1000);
					CTimeSpan ServerDuration(static_cast<__time64_t>(dwServerDurationSecs));
					uiHours = static_cast<unsigned>(ServerDuration.GetHours());
					uiMinutes = static_cast<unsigned>(ServerDuration.GetMinutes());
					uiDays = static_cast<unsigned>(ServerDuration.GetDays());

					if (uiDays == 0)
					{
						strRuntime.Format(_T("%02u:%02u:%02u"), uiHours, uiMinutes, ServerDuration.GetSeconds());
					}
					else
					{
						strRuntime.Format( _T("%u %s %02u:%02u"), uiDays,
							GetResString((uiDays == 1) ? IDS_LONGDAY : IDS_LONGDAYS),
							uiHours, uiMinutes );
					}

					strInfo.AppendFormat(_T("<br><b>%s:</b><t>%s"),	GetResString(IDS_STATS_CURRSRVDUR), strRuntime);
				}

				pNotify->ti->hIcon = g_App.m_pMDlg->m_dlgStatistics.m_imagelistStatTree.ExtractIcon(12);
				break;
			}
			case SB_NUMUSERS:
			{
				CServer *pServer = g_App.m_pServerConnect != NULL ? g_App.m_pServerConnect->GetCurrentServer() : NULL;

				if (pServer != NULL)
					pServer = g_App.m_pServerList->GetServerByAddress(pServer->GetAddress(), pServer->GetPort());

				if (pServer == NULL)
					return;

				strInfo = pServer->GetUsersInfo4Tooltips();
 				pNotify->ti->hIcon = CopyIcon(m_hiconUsers);
				break;
			}
			case SB_UPLOADRATE:
			{
				double dblLastUprate_KB = static_cast<double>(g_App.m_pUploadQueue->GetDataRate())/1024.0;
				double dblLastUprateOverhead_KB = static_cast<double>(g_App.m_pUploadQueue->GetUpDataRateOverhead())/1024.0;

				strInfo.Format( _T("<t=1><b>%s</b><br><hr=100%%><br><b>%s:</b><t>%.2f<br><b>%s:</b><t>%.2f<br><b>%s:</b><t>%s (%s)"),
								GetResString(IDS_TW_UPLOADS), GetResString(IDS_DL_SPEED), dblLastUprate_KB,
								GetResString(IDS_STATS_OVRHD), dblLastUprateOverhead_KB, GetResString(IDS_STATS_UDATA),
								CastItoXBytes(g_App.stat_sessionSentBytes),
								CastItoXBytes(g_App.stat_sessionSentBytes + g_App.m_pPrefs->GetTotalUploaded()) );
				pNotify->ti->hIcon = CopyIcon(m_hiconTrans[(dblLastUprate_KB != 0) ? 1:0]);
				break;
			}
			case SB_DOWNLOADRATE:
			{
				double dblLastDownrate_KB = static_cast<double>(g_App.m_pDownloadQueue->GetDataRate())/1024.0;
				double dblLastDownrateOverhead_KB = static_cast<double>(g_App.m_pDownloadQueue->GetDownDataRateOverhead())/1024.0;

				strInfo.Format( _T("<t=1><b>%s</b><br><hr=100%%><br><b>%s:</b><t>%.2f<br><b>%s:</b><t>%.2f<br><b>%s:</b><t>%s (%s)"),
								GetResString(IDS_TW_DOWNLOADS), GetResString(IDS_DL_SPEED), dblLastDownrate_KB,
								GetResString(IDS_STATS_OVRHD), dblLastDownrateOverhead_KB, GetResString(IDS_STATS_DDATA),
								CastItoXBytes(g_App.stat_sessionReceivedBytes),
								CastItoXBytes( g_App.stat_sessionReceivedBytes+g_App.m_pPrefs->GetTotalDownloaded()) );
				pNotify->ti->hIcon = CopyIcon(m_hiconTrans[(dblLastDownrate_KB != 0) ? 3:2]);
				break;
			}
			case SB_SERVER:
			{
				CServer *pServer = g_App.m_pServerConnect != NULL ? g_App.m_pServerConnect->GetCurrentServer() : NULL;

				if (pServer != NULL)
					pServer = g_App.m_pServerList->GetServerByAddress(pServer->GetAddress(), pServer->GetPort());

				if (pServer == NULL)
					return;

				pNotify->ti->hIcon = pServer->GetServerInfo4Tooltips(strInfo);
				break;
			}
		}
		pNotify->ti->sTooltip = strInfo;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CEmuleDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	EMULE_TRY

	switch (wParam)
	{
		case IDC_TOOLBARBUTTON + MTB_CONNECT:
			DisConnect();
			break;
		case IDC_TOOLBARBUTTON + MTB_SERVERS:
			SetActiveDialog(&m_wndServer);
			m_ctlToolBar.PressMuleButton(wParam);
			break;
		case IDC_TOOLBARBUTTON + MTB_TRANSFER:
			SetActiveDialog(&m_wndTransfer);
			m_ctlToolBar.PressMuleButton(wParam);
			break;
		case IDC_TOOLBARBUTTON + MTB_SEARCH:
			SetActiveDialog(&m_dlgSearch);
			m_ctlToolBar.PressMuleButton(wParam);
			break;
		case IDC_TOOLBARBUTTON + MTB_SHAREDFILES:
			SetActiveDialog(&m_wndSharedFiles);
			m_ctlToolBar.PressMuleButton(wParam);
			break;
		case IDC_TOOLBARBUTTON + MTB_MESSAGES:
			SetActiveDialog(&m_wndChat);
			m_ctlToolBar.PressMuleButton(wParam);
			break;
		case IDC_TOOLBARBUTTON + MTB_IRC:
			SetActiveDialog(&m_wndIRC);
			m_ctlToolBar.PressMuleButton(wParam);
			break;
		case IDC_TOOLBARBUTTON + MTB_STATISTICS:
			SetActiveDialog(&m_dlgStatistics);
			m_ctlToolBar.PressMuleButton(wParam);
			break;
		case IDC_TOOLBARBUTTON + MTB_PREFS:
			m_ctlToolBar.PressButton(wParam,TRUE);
			m_dlgPreferences.DoModal();
			m_ctlToolBar.PressButton(wParam,FALSE);
			break;
		default:
			break;
	}

	return CTrayDialog::OnCommand(wParam, lParam);

	EMULE_CATCH2

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::QuickSpeedOther(UINT nID)
{
	EMULE_TRY

	switch (nID)
	{
		case MP_QS_PA:
			g_App.m_pPrefs->SetMaxUpload(10);
			g_App.m_pPrefs->SetMaxDownload(10);
			break;
		case MP_QS_UA:
			g_App.m_pPrefs->SetMaxUpload(g_App.m_pPrefs->GetMaxGraphUploadRate());
			g_App.m_pPrefs->SetMaxDownload(g_App.m_pPrefs->GetMaxGraphDownloadRate());
			break;
	}

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CEmuleDlg::OnQueryEndSession()
{
	EMULE_TRY

	if (!CTrayDialog::OnQueryEndSession())
		return FALSE;

	if (g_App.m_app_state == g_App.APP_STATE_DONE)
	{
		return TRUE;
	}
	else if (g_App.m_app_state == g_App.APP_STATE_RUNNING)
	{
		g_App.m_app_state = g_App.APP_STATE_SHUTTINGDOWN;
		CDialog::OnCancel();
		return TRUE;
	}

	EMULE_CATCH2

	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CEmuleDlg::OnWebServerConnect(WPARAM wParam, LPARAM lParam)
{
	NOPRM(lParam);

	EMULE_TRY

#ifdef OLD_SOCKETS_ENABLED
	g_App.m_pServerConnect->ConnectToServer((CServer*)wParam, false);
#endif

	EMULE_CATCH2

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CEmuleDlg::OnWebServerRemove(WPARAM wParam, LPARAM lParam)
{
	NOPRM(lParam);

	EMULE_TRY

	g_App.m_pMDlg->m_wndServer.m_ctlServerList.RemoveServer((CServer*)wParam);

	EMULE_CATCH2

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CEmuleDlg::OnWebServerAddToStatic(WPARAM wParam, LPARAM lParam)
{
	NOPRM(lParam);

	EMULE_TRY

	g_App.m_pMDlg->m_wndServer.m_ctlServerList.StaticServerFileAppend((CServer*)wParam);
	EMULE_CATCH2

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CEmuleDlg::OnWebServerRemoveFromStatic(WPARAM wParam, LPARAM lParam)
{
	NOPRM(lParam);

	EMULE_TRY

	g_App.m_pMDlg->m_wndServer.m_ctlServerList.StaticServerFileRemove((CServer*)wParam);

	EMULE_CATCH2

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CEmuleDlg::OnWebServerClearCompleted(WPARAM wParam, LPARAM lParam)
{
	EMULE_TRY

	if (wParam == 0)
	{
		EnumCategories eCatID = static_cast<_EnumCategories>(lParam);
		g_App.m_pDownloadList->ClearCompleted(eCatID);
	}
	else
	{
		uchar* pFileHash = reinterpret_cast<uchar*>(lParam);
		g_App.m_pDownloadList->ClearCompleted(pFileHash);
		delete[] pFileHash;
	}

	EMULE_CATCH2

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CEmuleDlg::OnWebServerFileRename(WPARAM wParam, LPARAM lParam)
{
	EMULE_TRY

	CString sNewName = ((LPCTSTR)(lParam));

	((CPartFile*)wParam)->SetFileName(sNewName);
	((CPartFile*)wParam)->SavePartFile();
	((CPartFile*)wParam)->UpdateDisplayedInfo();
	g_App.m_pSharedFilesList->UpdateItem((CKnownFile*)((CPartFile*)wParam));
	g_App.m_pMDlg->m_wndTransfer.UpdateInfoHeader();

	EMULE_CATCH2

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CEmuleDlg::OnProcessTaskUI(WPARAM wParam, LPARAM lParam)
{
	NOPRM(wParam); NOPRM(lParam);

	EMULE_TRY

#ifdef NEW_SOCKETS
/*	CTask *pTask = (CTask*)wParam;
	if(pTask)
	{
		if(pTask->ProcessForUI(*g_App.m_pEngine))
			delete pTask;
	}*/
#endif //NEW_SOCKETS

	EMULE_CATCH2

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CEmuleDlg::OnWebSharedFilesReload(WPARAM wParam, LPARAM lParam)
{
	NOPRM(wParam); NOPRM(lParam);

	EMULE_TRY

	g_App.m_pSharedFilesList->Reload();

	EMULE_CATCH2

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// To find out if app is running or shutting/shut down
bool CEmuleDlg::IsRunning()
{
	return (g_App.m_app_state == CEmuleApp::APP_STATE_RUNNING);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::OnNcDestroy()
{
	//see KB138681 [TwoBottle Mod]
	AfxGetApp()->m_pMainWnd = NULL;
	CTrayDialog::OnNcDestroy();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CEmuleDlg::TrayShow(void)
{
//	Set tray icon
	int DownRateProcent = (int)ceil(lastdownrate / 1.024 / g_App.m_pPrefs->GetMaxGraphDownloadRate());
	if (DownRateProcent>100)
		DownRateProcent=100;

	UpdateTrayIcon(DownRateProcent);

	return CTrayDialog::TrayShow();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::RunBackupNow(bool bAutomated)
{
	int	iMode = (bAutomated) ? EP_BAKMODE_AUTOMATED : 0;

	if (g_App.m_pPrefs->GetBackupDatFiles())
		BackupFromAppDir(_T("*.dat"), iMode);
	if (g_App.m_pPrefs->GetBackupMetFiles())
		BackupFromAppDir(_T("*.met"), iMode);
	if (g_App.m_pPrefs->GetBackupIniFiles())
		BackupFromAppDir(_T("*.ini"), iMode);
	if (g_App.m_pPrefs->GetBackupPartMetFiles())
		BackupFromTempDir(_T("*.part.met"), iMode | EP_BAKMODE_SKIPZERO);
	if (g_App.m_pPrefs->GetBackupPartTxtsrcFiles())
		BackupFromTempDir(_T("*.part.txtsrc"), iMode | EP_BAKMODE_SKIPZERO);
	if (g_App.m_pPrefs->GetBackupPartFiles())
	{
		if ( bAutomated ||
			::MessageBox(m_dlgPreferences.m_hWnd, GetResString(IDS_BACKUP_LONGTIME), GetResString(IDS_BACKUP_SURE), MB_ICONQUESTION|MB_YESNO) == IDYES )
		{
			BackupFromTempDir(_T("*.part"), iMode);
		}
	}
	if (!bAutomated)
		::MessageBox(m_dlgPreferences.m_hWnd, GetResString(IDS_BACKUP_SUCCESS), GetResString(IDS_BACKUP_COMPLETE), MB_OK);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::BackupFromAppDir(LPCTSTR pcExtensionToBack, int iMode)
{
	CString	strBuffer = g_App.m_pPrefs->GetBackupDir();

	strBuffer += _T('\\');

//	Create all required directories
	CreateAllDirectories(&strBuffer);
//	Copying files
	BackupFiles(pcExtensionToBack, iMode, g_App.m_pPrefs->GetConfigDir(), strBuffer);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::BackupFromTempDir(LPCTSTR pcExtensionToBack, int iMode)
{
	CString	strBuffer = g_App.m_pPrefs->GetBackupDir();

	strBuffer += _T("\\Temp\\");

//	Create all required directories
	CreateAllDirectories(&strBuffer);

	BackupFiles(pcExtensionToBack, iMode, g_App.m_pPrefs->GetTempDir(), strBuffer);

//	Additional Temp dirs backup
	CStringList	tmpTempDirList;	// List elements will be deleted in list destructor

//	Make local copy to prevent long locking of list resource
	g_App.m_pPrefs->TempDirListCopy(&tmpTempDirList);

	CString strSearchPath;
	uint32	dwTempDir = 1;

	for (POSITION pos = tmpTempDirList.GetHeadPosition(); pos != NULL;)
	{
		strSearchPath = tmpTempDirList.GetNext(pos);

		strBuffer.Format(_T("%s\\Temp[%u]\\"), g_App.m_pPrefs->GetBackupDir(), dwTempDir);

	//	Create last directory. No need to use CreateAllDirectories() as the previous path already exists
		::CreateDirectory(strBuffer, NULL);

		BackupFiles(pcExtensionToBack, iMode, strSearchPath, strBuffer);

		strBuffer += _T("backup.log");

		FILE	*pLogFile = _tfsopen(strBuffer, _T("wb"), _SH_DENYWR);

		if (pLogFile != NULL)
		{
#ifdef _UNICODE
			fputwc(0xFEFF, pLogFile);
#endif
			_fputts(_T("Backed-up directory: "), pLogFile);
			_fputts(strSearchPath, pLogFile);
			fclose(pLogFile);
		}

		dwTempDir++;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::BackupFiles(LPCTSTR pcExtensionToBack, int iMode, const CString &strPath, const CString &strBackupPath)
{
	WIN32_FIND_DATA	fileData;
	HANDLE	hSearch;
	bool	bError = false, bOverWrite = true;
	CString	strDstPath = strPath;

//	Start searching for files in the directory.
	if (strDstPath.Right(1) != _T('\\'))
		strDstPath += _T('\\');

	CString	strTmp = strDstPath;

	strTmp += pcExtensionToBack;
	if ((hSearch = FindFirstFile(strTmp, &fileData)) != INVALID_HANDLE_VALUE)
	{
		CString	strNewPath;

	//	Copy each file to the backup directory
		for (;;)
		{
		//	Skip zero size files if required to avoid backing corrupted temp. files up
			if ( ((iMode & EP_BAKMODE_SKIPZERO) == 0) ||
				((fileData.nFileSizeHigh | fileData.nFileSizeLow) != 0) )
			{
				strNewPath = strBackupPath;
				strNewPath += fileData.cFileName;

				if (((iMode & EP_BAKMODE_AUTOMATED) == 0) && PathFileExists(strNewPath))
				{
					if (!g_App.m_pPrefs->GetBackupOverwrite())
					{
						strTmp.Format(GetResString(IDS_BACKUP_EXIST), fileData.cFileName);
						bOverWrite = (MessageBox(strTmp, GetResString(IDS_BACKUP_OVER), MB_ICONQUESTION|MB_YESNO) == IDYES);
					}
				}
				if (bOverWrite)
				{
					strTmp = strDstPath;
					strTmp += fileData.cFileName;
					CopyFile(strTmp, strNewPath, FALSE);
				}
				if ((iMode & EP_BAKMODE_AUTOMATED) == 0)
					AddLogLine(LOG_FL_DBG, _T("Backup %s done"), strNewPath);
			}

			if (!FindNextFile(hSearch, &fileData))
			{
				if (GetLastError() != ERROR_NO_MORE_FILES)
					bError = true;
				else
					break;
			}
		}
		FindClose(hSearch);	//	Close the search handle
	}

	if (bError)
	{
		AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("Error encountered during backup!"));
		if ((iMode & EP_BAKMODE_AUTOMATED) == 0)
			MessageBox(GetResString(IDS_BACKUP_ERROR), GetResString(IDS_ERROR), MB_OK);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleDlg::DisableAutoBackup()
{
	if (g_App.m_pPrefs->IsAutoBackup() || g_App.m_pPrefs->IsScheduledBackup())
	{
		g_App.m_pPrefs->SetScheduledBackup(false);
		g_App.m_pPrefs->SetAutoBackup(false);

		AddLogLine(LOG_RGB_WARNING, IDS_AUTOBACKUP_DISABLED);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	PostUniqueMessage() check the existence of message in the list control's message queue & puts it into the queue if message does not exist
void CEmuleDlg::PostUniqueMessage(UINT uiMsg)
{
	if (::IsWindow(GetSafeHwnd()) && g_App.m_app_state != CEmuleApp::APP_STATE_SHUTTINGDOWN)
	{
		MSG		msg;

	//	If there's no refresh message already in the message queue... (don't want to flood it)
		if (!::PeekMessage(&msg, m_hWnd, uiMsg, uiMsg, PM_NOREMOVE))
			PostMessage(uiMsg, 0, 0);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString* CEmuleDlg::GetNewLogTextLines()
{
	CString		*pstrNewLines = NULL;

	if (!m_pstrNewLogTextLines->IsEmpty())
	{
		pstrNewLines = m_pstrNewLogTextLines;
		m_pstrNewLogTextLines = new CString;
	}

	return pstrNewLines;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString* CEmuleDlg::GetNewDebugLogTextLines()
{
	CString		*pstrNewLines = NULL;

	if (!m_pstrNewDebugLogTextLines->IsEmpty())
	{
		pstrNewLines = m_pstrNewDebugLogTextLines;
		m_pstrNewDebugLogTextLines = new CString;
	}

	return pstrNewLines;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CEmuleDlg::OnWndMsg(UINT iMessage, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	BOOL		bHandled = TRUE;

	EMULE_TRY

	if (IsRunning())
	{
		switch (iMessage)
		{
			case WM_LOG_REFRESH:
			{
				const CString	*pstrNewLines = GetNewLogTextLines();

				if (pstrNewLines != NULL)
				{
					OutputLogText(*pstrNewLines, m_wndServer.m_pctlLogBox);
					if (m_wndServer.m_ctrlBoxSwitcher.GetCurSel() != 0)
						m_wndServer.m_ctrlBoxSwitcher.SetItemState(0, TCIS_HIGHLIGHTED, TCIS_HIGHLIGHTED);

					delete pstrNewLines;
				}
				break;
			}
			case WM_DLOG_REFRESH:
			{
				const CString	*pstrNewLines = GetNewDebugLogTextLines();

				if (pstrNewLines != NULL)
				{
					OutputLogText(*pstrNewLines, m_wndServer.m_pctlDebugBox);
					if (m_wndServer.m_ctrlBoxSwitcher.GetCurSel() != 2)
						m_wndServer.m_ctrlBoxSwitcher.SetItemState(2, TCIS_HIGHLIGHTED, TCIS_HIGHLIGHTED);

					delete pstrNewLines;
				}
				break;
			}
			case WM_ACTIVATEAPP:
			{
				BOOL bIsActivated = (BOOL)wParam;

				if (!bIsActivated && m_pdlgActive != NULL)
				{
					NMHDR	hdr;

					m_ttip.Pop();	// Cancel the tooltip in eMuleDlg class

					hdr.hwndFrom = GetSafeHwnd();
					hdr.idFrom = GetDlgCtrlID();
					hdr.code = UDM_TOOLTIP_POP;
					::SendMessage(m_pdlgActive->GetSafeHwnd(), WM_NOTIFY, 0, (LPARAM)&hdr);
				}
				break;
			}
			default:
				bHandled = FALSE;
				break;
		}
	}
	else
		bHandled = FALSE;

	EMULE_CATCH

	if (!bHandled)
		bHandled = CDialog::OnWndMsg(iMessage, wParam, lParam, pResult);

	return bHandled;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CEmuleDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
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

	return CTrayDialog::OnNotify(wParam, lParam, pResult);
}
