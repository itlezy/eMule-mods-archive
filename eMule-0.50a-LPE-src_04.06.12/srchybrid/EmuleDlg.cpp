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
#include <math.h>
#define MMNODRV			// mmsystem: Installable driver support
//#define MMNOSOUND		// mmsystem: Sound support
#define MMNOWAVE		// mmsystem: Waveform support
#define MMNOMIDI		// mmsystem: MIDI support
#define MMNOAUX			// mmsystem: Auxiliary audio support
#define MMNOMIXER		// mmsystem: Mixer support
#define MMNOTIMER		// mmsystem: Timer support
#define MMNOJOY			// mmsystem: Joystick support
#define MMNOMCI			// mmsystem: MCI support
#define MMNOMMIO		// mmsystem: Multimedia file I/O support
#define MMNOMMSYSTEM	// mmsystem: General MMSYSTEM functions
#include <Mmsystem.h>
#include <share.h>
#include "emule.h"
#include "emuleDlg.h"
#include "ServerWnd.h"
#include "TransferWnd.h"
#include "SearchResultsWnd.h"
#include "SearchDlg.h"
#include "StatisticsDlg.h"
#include "PreferencesDlg.h"
#include "Sockets.h"
#include "KnownFileList.h"
#include "ServerList.h"
#include "Opcodes.h"
#include "SharedFileList.h"
#include "ED2KLink.h"
//#include "Splashscreen.h"  //Xman slpashscreen
//#include "SplashScreenEx.h" //Xman splashscreen
#include "Exceptions.h"
#include "SearchList.h"
#include "HTRichEditCtrl.h"
#include "kademlia/kademlia/kademlia.h"
#include "kademlia/kademlia/SearchManager.h"
#include "kademlia/routing/RoutingZone.h"
#include "kademlia/routing/contact.h"
#include "kademlia/kademlia/prefs.h"
#include "PerfLog.h"
//#include "LastCommonRouteFinder.h" //Xman
#include "DownloadQueue.h"
#include "ClientUDPSocket.h"
#include "UploadQueue.h"
#include "ClientList.h"
#include "UploadBandwidthThrottler.h"
#include "IPFilter.h"
#include "Statistics.h"
#include "MuleToolbarCtrl.h"
#include "MuleStatusbarCtrl.h"
#include "ListenSocket.h"
#include "Server.h"
#include "PartFile.h"
#include "ClientCredits.h"
#include "MenuCmds.h"
#include "DirectDownloadDlg.h"
#include "ResumeDownloadDlg.h"// X: [IP] - [Import Parts]
#include "Statistics.h"
#include "FirewallOpener.h"
#include "StringConversion.h"
#include "aichsyncthread.h"
#include "Log.h"
#include "UserMsgs.h"
//Xman
#include "BandWidthControl.h" // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
#include "IP2Country.h" //EastShare - added by AndCycle, IP to Country
#include "Collection.h"
#include "CollectionViewDialog.h"
#include "VisualStylesXP.h"
#include "UPnPImpl.h"
#include "UPnPImplWrapper.h"
#include "FileVerify.h"// X: [FV] - [FileVerify]
#include <dbt.h>
#include "XMessageBox.h"
#include "SpeedGraphWnd.h" // X: [SGW] - [SpeedGraphWnd]
#include "HttpDownloadDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



UINT g_uMainThreadId = GetCurrentThreadId();
const static UINT UWM_ARE_YOU_EMULE = RegisterWindowMessage(EMULE_GUID);

#ifdef HAVE_WIN7_SDK_H
const static UINT UWM_TASK_BUTTON_CREATED = RegisterWindowMessage(_T("TaskbarButtonCreated"));
#endif

// ==> Invisible Mode [TPT/MoNKi] - Stulle
// Allows "invisible mode" on multiple instances of eMule
#ifdef _DEBUG
#define EMULE_GUID_INVMODE				"EMULE-{4EADC6FC-516F-4b7c-9066-97D893649569}-DEBUG-INVISIBLEMODE"
#else
#define EMULE_GUID_INVMODE				"EMULE-{4EADC6FC-516F-4b7c-9066-97D893649569}-INVISIBLEMODE"
#endif
const static UINT UWM_RESTORE_WINDOW_IM=RegisterWindowMessage(_T(EMULE_GUID_INVMODE));
// <== Invisible Mode [TPT/MoNKi] - Stulle


///////////////////////////////////////////////////////////////////////////
// CemuleDlg Dialog

IMPLEMENT_DYNAMIC(CMsgBoxException, CException)

BEGIN_MESSAGE_MAP(CemuleDlg, CTrayDialog)
	///////////////////////////////////////////////////////////////////////////
	// Windows messages
	//
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_ENDSESSION()
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_WM_MENUCHAR()
	ON_WM_QUERYENDSESSION()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_COPYDATA, OnWMData)
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
	ON_MESSAGE(WM_USERCHANGED, OnUserChanged)
	ON_WM_SHOWWINDOW()
	ON_WM_DESTROY()
	ON_WM_SETTINGCHANGE()
	ON_WM_DEVICECHANGE()
	ON_MESSAGE(WM_POWERBROADCAST, OnPowerBroadcast)
	ON_MESSAGE(WM_HOTKEY, OnHotKey)	// Invisible Mode [TPT/MoNKi] - Stulle

	///////////////////////////////////////////////////////////////////////////
	// WM_COMMAND messages
	//
	ON_COMMAND(MP_CONNECT, StartConnection)
	ON_COMMAND(MP_DISCONNECT, CloseConnection)
	ON_COMMAND(MP_EXIT, OnClose)
	ON_COMMAND(MP_RESTORE, RestoreWindow)
	ON_COMMAND(SC_MINIMIZE, MinimizeWindow)
	// quick-speed changer -- 
	ON_COMMAND_RANGE(MP_QS_U20, MP_QS_UP10, QuickSpeedUpload)
	ON_COMMAND_RANGE(MP_QS_D20, MP_QS_D100, QuickSpeedDownload)
	//--- quickspeed - paralize all ---
	ON_COMMAND(MP_QS_UA, QuickSpeedOther)
	// quick-speed changer -- based on xrmb	

	ON_MESSAGE(UM_TRAY_ICON_NOTIFY_MESSAGE, OnTrayNotify)

	ON_REGISTERED_MESSAGE(UWM_ARE_YOU_EMULE, OnAreYouEmule)
	// ==> Invisible Mode [TPT/MoNKi] - Stulle
	ON_REGISTERED_MESSAGE(UWM_RESTORE_WINDOW_IM, OnRestoreWindowInvisibleMode)
	// <== Invisible Mode [TPT/MoNKi] - Stulle
	
	///////////////////////////////////////////////////////////////////////////
	// WM_USER messages
	//

	// UPnP
	ON_MESSAGE(UM_UPNP_RESULT, OnUPnPResult)

	///////////////////////////////////////////////////////////////////////////
	// WM_APP messages
	//
	ON_MESSAGE(TM_FINISHEDHASHING, OnFileHashed)
	ON_MESSAGE(TM_FILEOPPROGRESS, OnFileOpProgress)
	ON_MESSAGE(TM_HASHFAILED, OnHashFailed)
	//Xman
	// BEGIN SLUGFILLER: SafeHash
	ON_MESSAGE(TM_PARTHASHEDOK, OnPartHashedOK)
	ON_MESSAGE(TM_PARTHASHEDOKNOAICH, OnPartHashedOKNoAICH)
	ON_MESSAGE(TM_PARTHASHEDCORRUPT, OnPartHashedCorrupt)
	ON_MESSAGE(TM_PARTHASHEDCORRUPTNOAICH, OnPartHashedCorruptNoAICH)
	ON_MESSAGE(TM_PARTHASHEDOKAICHRECOVER, OnPartHashedOKAICHRecover)
	ON_MESSAGE(TM_PARTHASHEDCORRUPTAICHRECOVER, OnPartHashedCorruptAICHRecover)
	// END SLUGFILLER: SafeHash
	ON_MESSAGE(TM_READBLOCKFROMFILEDONE, OnReadBlockFromFileDone) // SiRoB: ReadBlockFromFileThread
	ON_MESSAGE(TM_FLUSHDONE, OnFlushDone) // SiRoB: Flush Thread
	ON_MESSAGE(TM_DOTIMER, DoTimer) //Xman process timer code via messages (Xanatos)
	//Xman end
	ON_MESSAGE(TM_FILEALLOCEXC, OnFileAllocExc)
	ON_MESSAGE(TM_FILECOMPLETED, OnFileCompleted)
	ON_MESSAGE(TM_CONSOLETHREADEVENT, OnConsoleThreadEvent)

#ifdef HAVE_WIN7_SDK_H
	ON_REGISTERED_MESSAGE(UWM_TASK_BUTTON_CREATED, OnTaskbarBtnCreated)
#endif

END_MESSAGE_MAP()

CemuleDlg::CemuleDlg(CWnd* pParent /*=NULL*/)
	: CTrayDialog(CemuleDlg::IDD, pParent)
{
	preferenceswnd = new CPreferencesDlg;
	serverwnd = new CServerWnd;
	transferwnd = new CTransferWnd;
	searchwnd = new CSearchDlg;
	statisticswnd = new CStatisticsDlg;
	toolbar = new CMuleToolbarCtrl;
	statusbar = new CMuleStatusBarCtrl;
	directdowndlg = NULL; // X: [UIC] - [UIChange] allow change cat

	contactCount=0; //KadCount
	m_hIcon = NULL;
	theApp.m_app_state = APP_STATE_RUNNING;
	m_bStartMinimizedChecked = false;
	m_bStartMinimized = false;
	memset(&m_wpFirstRestore, 0, sizeof m_wpFirstRestore);
	m_uUpDatarate = 0;
	m_uDownDatarate = 0;
	status = 0;
	activewnd = NULL;
	for (int i = 0; i < _countof(connicons); i++)
		connicons[i] = NULL;	
/*
	transicons[0] = NULL;
	transicons[1] = NULL;
	transicons[2] = NULL;
	transicons[3] = NULL;
*/
	//m_icoSysTrayConnected = NULL;
	//m_icoSysTrayDisconnected = NULL;
	//m_icoSysTrayLowID = NULL;
	m_icoSysTrayCurrent = NULL;
	m_hTimer = 0;
	m_hUPnPTimeOutTimer = 0;
	m_bConnectRequestDelayedForUPnP = false;
	m_bEd2kSuspendDisconnect = false;
	m_bKadSuspendDisconnect = false;
#ifdef HAVE_WIN7_SDK_H
	m_bInitedCOM = false;
#endif
	b_HideApp = false; // Invisible Mode [TPT/MoNKi] - Stulle
}

CemuleDlg::~CemuleDlg()
{
	if (m_hIcon) VERIFY( ::DestroyIcon(m_hIcon) );
	for (size_t i = 0; i < _countof(connicons); i++){
		if (connicons[i]) VERIFY( ::DestroyIcon(connicons[i]) );
	}
/*
	if (transicons[0]) VERIFY( ::DestroyIcon(transicons[0]) );
	if (transicons[1]) VERIFY( ::DestroyIcon(transicons[1]) );
	if (transicons[2]) VERIFY( ::DestroyIcon(transicons[2]) );
	if (transicons[3]) VERIFY( ::DestroyIcon(transicons[3]) );
*/
    if (m_icoSysTrayCurrent) VERIFY( ::DestroyIcon(m_icoSysTrayCurrent) );
	//if (m_icoSysTrayConnected) VERIFY( ::DestroyIcon(m_icoSysTrayConnected) );
	//if (m_icoSysTrayDisconnected) VERIFY( ::DestroyIcon(m_icoSysTrayDisconnected) );
	//if (m_icoSysTrayLowID) VERIFY( ::DestroyIcon(m_icoSysTrayLowID) );

#ifdef HAVE_WIN7_SDK_H
	if (m_pTaskbarList != NULL)
	{
		m_pTaskbarList.Release();
		ASSERT( m_bInitedCOM );
	}
	if (m_bInitedCOM)
		CoUninitialize();
#endif

	delete preferenceswnd;
	delete serverwnd;
	delete transferwnd;
	delete statisticswnd;
	delete toolbar;
	delete statusbar;
}

LRESULT CemuleDlg::OnAreYouEmule(WPARAM, LPARAM)
{
	return UWM_ARE_YOU_EMULE;
}

void CemuleDlg::ShowToolbar(){
	// adjust all main window sizes for toolbar height and maximize the child windows
	CRect rcClient;
	GetClientRect(&rcClient);

	if(thePrefs.isshowtoolbar){
		toolbar->ShowWindow(SW_SHOW);
		CRect rcToolbar;
		toolbar->GetWindowRect(&rcToolbar);
		rcClient.top += rcToolbar.Height();
	}
	else
		toolbar->ShowWindow(SW_HIDE);

	CRect rcStatusbar;
	statusbar->GetWindowRect(&rcStatusbar);
	rcClient.bottom -= rcStatusbar.Height();

	CWnd* apWnds[] =
	{
		serverwnd,
		transferwnd,
		searchwnd,
		statisticswnd
	};
	for (size_t i = 0; i < _countof(apWnds); i++)
		apWnds[i]->SetWindowPos(NULL, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER);

	// anchors
	AddAnchor(*serverwnd,		TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*transferwnd,		TOP_LEFT, BOTTOM_RIGHT);
    AddAnchor(*searchwnd,		TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*statisticswnd,	TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*toolbar,			TOP_LEFT, TOP_RIGHT);
	AddAnchor(*statusbar,		BOTTOM_LEFT, BOTTOM_RIGHT);
}

void DialogCreateIndirect(CDialog *pWnd, UINT uID)
{
#if 0
	// This could be a nice way to change the font size of the main windows without needing
	// to re-design the dialog resources. However, that technique does not work for the
	// SearchWnd and it also introduces new glitches (which would need to get resolved)
	// in almost all of the main windows.
	CDialogTemplate dlgTempl;
	dlgTempl.Load(MAKEINTRESOURCE(uID));
	dlgTempl.SetFont(_T("MS Shell Dlg"), 8);
	pWnd->CreateIndirect(dlgTempl.m_hTemplate);
	FreeResource(dlgTempl.Detach());
#else
	pWnd->Create(uID);
#endif
}

BOOL CemuleDlg::OnInitDialog()
{
#ifdef HAVE_WIN7_SDK_H
	// allow the TaskbarButtonCreated- & (tbb-)WM_COMMAND message to be sent to our window if our app is running elevated
	if (thePrefs.GetWindowsVersion() >= _WINVER_7_)
	{
		int res = CoInitialize(NULL);
		if (res == S_OK || res == S_FALSE)
		{
			m_bInitedCOM = true;
			typedef BOOL (WINAPI* PChangeWindowMessageFilter)(UINT message, DWORD dwFlag);
			PChangeWindowMessageFilter ChangeWindowMessageFilter = 
			(PChangeWindowMessageFilter )(GetProcAddress(
				GetModuleHandle(TEXT("user32.dll")), "ChangeWindowMessageFilter"));
			if (ChangeWindowMessageFilter) {
				ChangeWindowMessageFilter(UWM_TASK_BUTTON_CREATED,1);
				ChangeWindowMessageFilter(WM_COMMAND, 1);
			}
		}
		else
			ASSERT( false );
	}
#endif
	// temporary disable the 'startup minimized' option, otherwise no window will be shown at all
	// ==> Invisible Mode [TPT/MoNKi] - Stulle
	b_WindowWasVisible=thePrefs.IsFirstStart() || (!thePrefs.GetStartMinimized() && !theApp.DidWeAutoStart());
	m_bStartMinimized = !b_WindowWasVisible || (thePrefs.m_bInvisibleMode && thePrefs.m_bInvisibleModeStart);
	// <== Invisible Mode [TPT/MoNKi] - Stulle
	//Xman new slpash-screen arrangement
	/*
	// show splashscreen as early as possible to "entertain" user while starting emule up
	if (thePrefs.UseSplashScreen() && !m_bStartMinimized)
	{
		//Xman final version: don't show splash on old windows->crash
		switch (thePrefs.GetWindowsVersion())
		{
			case _WINVER_98_:
			case _WINVER_95_:	
			case _WINVER_ME_:
				break;
			default:
			ShowSplash();
		}
	}
	*/
	// Create global GUI objects
	theApp.CreateAllFonts();
	theApp.CreateBackwardDiagonalBrush();
	CTrayDialog::OnInitDialog();
	InitWindowStyles(this);

	if (toolbar->Create(WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), this, IDC_TOOLBAR))
		toolbar->Init();

	// set title
	SetWindowText(_T("eMule v") + theApp.m_strCurVersionLong + _T(" -LPE-")); 

	// set statusbar
	// the statusbar control is created as a custom control in the dialog resource,
	// this solves font and sizing problems when using large system fonts
	//morph4u changed to non custom control +
	//statusbar->SubclassWindow(GetDlgItem(IDC_STATUSBAR)->m_hWnd); 
	statusbar->Create(WS_CHILD|WS_VISIBLE|CCS_BOTTOM|SBARS_SIZEGRIP,CRect(0,0,0,0), this, IDC_STATUSBAR);
	statusbar->SetFont(GetFont());
	//morph4u changed to non custom control -
	statusbar->EnableToolTips(true);
	SetStatusBarPartsSize();

	// create main window dialog pages
	DialogCreateIndirect(serverwnd, IDD_SERVER);
	searchwnd->Create(this); // can not use 'DialogCreateIndirect' for the SearchWnd, grrr..
	DialogCreateIndirect(transferwnd, IDD_TRANSFER);
	DialogCreateIndirect(statisticswnd, IDD_STATISTICS);

	// with the top rebar control, some XP themes look better with some additional lite borders.. some not..
	//serverwnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//searchwnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//transferwnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//statisticswnd->ModifyStyleEx(0, WS_EX_STATICEDGE);

	// optional: restore last used main window dialog
	if (thePrefs.GetRestoreLastMainWndDlg()){
		switch (thePrefs.GetLastMainWndDlgID()){
		case IDD_SERVER:
			SetActiveDialog(serverwnd);
			break;
		case IDD_SEARCH:
			SetActiveDialog(searchwnd);
			break;
		case IDD_TRANSFER:
			SetActiveDialog(transferwnd);
			break;
		case IDD_STATISTICS:
			SetActiveDialog(statisticswnd);
			break;
		}
	}

	// if still no active window, activate server window
	if (activewnd == NULL)
		SetActiveDialog(serverwnd);

	SetAllIcons();

	// set updateintervall of graphic rate display (in seconds)
	//ShowConnectionState(false);
	ShowToolbar();

	CreateMenues();
	CreateTrayMenues();
	ShowConnectionState();
        //Localize();// X: [RUL] - [Remove Useless Localize]

	statisticswnd->ShowInterval();

	// tray icon
	//ShowTransferRate(true);
	searchwnd->UpdateCatTabs();

	///////////////////////////////////////////////////////////////////////////
	// Restore saved window placement
	//
	WINDOWPLACEMENT wp = {0};
	wp.length = sizeof(wp);
	wp = thePrefs.GetEmuleWindowPlacement();
	if (m_bStartMinimized)
	{
		// Minimize to System Tray
		//
		// Unfortunately this does not work. The eMule main window is a modal dialog which is invoked
		// by CDialog::DoModal which eventually calls CWnd::RunModalLoop. Look at 'MLF_SHOWONIDLE' and
		// 'bShowIdle' in the above noted functions to see why it's not possible to create the window
		// right in hidden state.

		//--- attempt #1
		//wp.showCmd = SW_HIDE;
		//TrayShow();
		//--- doesn't work at all

		//--- attempt #2
		//if (wp.showCmd == SW_SHOWMAXIMIZED)
		//	wp.flags = WPF_RESTORETOMAXIMIZED;
		//m_bStartMinimizedChecked = false; // post-hide the window..
		//--- creates window flickering

		//--- attempt #3
		// Minimize the window into the task bar and later move it into the tray bar
		if (wp.showCmd == SW_SHOWMAXIMIZED)
			wp.flags = WPF_RESTORETOMAXIMIZED;
		wp.showCmd = SW_MINIMIZE;

		m_bStartMinimizedChecked = false;

		// to get properly restored from tray bar (after attempt #3) we have to use a patched 'restore' window cmd..
		m_wpFirstRestore = thePrefs.GetEmuleWindowPlacement();
		m_wpFirstRestore.length = sizeof(m_wpFirstRestore);
		if (m_wpFirstRestore.showCmd != SW_SHOWMAXIMIZED)
			m_wpFirstRestore.showCmd = SW_SHOWNORMAL;
		}
	else
	{
		// Allow only SW_SHOWNORMAL and SW_SHOWMAXIMIZED. Ignore SW_SHOWMINIMIZED to make sure the window
		// becomes visible. If user wants SW_SHOWMINIMIZED, we already have an explicit option for this (see above).
		if (wp.showCmd != SW_SHOWMAXIMIZED)
			wp.showCmd = SW_SHOWNORMAL;
		m_bStartMinimizedChecked = true;
	}
	SetWindowPlacement(&wp);

	VERIFY( (m_hTimer = ::SetTimer(NULL, NULL, 300, StartupTimer)) != NULL );
	if (thePrefs.GetVerbose() && !m_hTimer)
		AddDebugLogLine(true,_T("Failed to create 'startup' timer - %s"),GetErrorMessage(GetLastError()));

	//theStats.starttime = GetTickCount();// X: move to CStatistics::CStatistics()

	// Start UPnP prot forwarding
	if (thePrefs.IsUPnPEnabled())
		StartUPnP();

		
	// ==> Invisible Mode [TPT/MoNKi] - Stulle
	if(thePrefs.m_bInvisibleMode)
		RegisterInvisibleHotKey();
	// <== Invisible Mode [TPT/MoNKi] - Stulle

	//Xman
	// SiRoB: SafeHash fix (see StartupTimer)
	/*
	// start aichsyncthread
	AfxBeginThread(RUNTIME_CLASS(CAICHSyncThread), THREAD_PRIORITY_BELOW_NORMAL,0);
	*/

	// debug info
	DebugLog(_T("Using '%s' as config directory"), thePrefs.GetMuleDirectory(EMULE_CONFIGDIR)); 
	
	if (!thePrefs.HasCustomTaskIconColor())
		SetTaskbarIconColor();

	return TRUE;
}

void CALLBACK CemuleDlg::StartupTimer(HWND /*hwnd*/, UINT /*uiMsg*/, UINT_PTR /*idEvent*/, DWORD /*dwTime*/)
{
	//Xman
	// SLUGFILLER: doubleLucas - not ready to init, come back next cycle
	if (!::IsWindow(theApp.emuledlg->m_hWnd))
		return;
	if (!::IsWindow(theApp.emuledlg->serverwnd->serverlistctrl.m_hWnd))
		return;
	if (!::IsWindow(theApp.emuledlg->transferwnd->downloadlistctrl.m_hWnd))
		return;
	//Xman end
	// NOTE: Always handle all type of MFC exceptions in TimerProcs - otherwise we'll get mem leaks
	try
	{
		switch(theApp.emuledlg->status){
			case 0:
				theApp.emuledlg->status++;
				break;
			case 1:
				theApp.emuledlg->status++;
				try{
					theApp.serverlist->Init();
					if(thePrefs.AutoUpdateIPFilter()){
						theApp.ipfilter->UpdateIPFilterURL(_T(""));
						theApp.SplashHide(SW_SHOW); //Xman new slpash-screen arrangement
					}
				}
				catch(...){
					ASSERT(0);
					LogError(LOG_STATUSBAR,_T("Failed to initialize server list - Unknown exception"));
				}
				theApp.emuledlg->status++;
				break;
			case 2:
				break;
			case 3:{
				bool bError = false;
				theApp.emuledlg->status++;
				theApp.UpdateSplash(_T("Initializing  files to download ...")); //Xman new slpash-screen arrangement

				// NOTE: If we have an unhandled exception in CDownloadQueue::Init, MFC will silently catch it
				// and the creation of the TCP and the UDP socket will not be done -> client will get a LowID!
				try{
					theApp.downloadqueue->Init();
				}
				catch(...){
					ASSERT(0);
					LogError(LOG_STATUSBAR,_T("Failed to initialize download queue - Unknown exception"));
					bError = true;
				}
				if (!theApp.listensocket->StartListening()) {
					CString strError;
					strError.Format(GetResString(IDS_MAIN_SOCKETERROR), thePrefs.GetPort());
					LogError(LOG_STATUSBAR|LOG_DONTNOTIFY, _T("%s"), strError);
					if (thePrefs.GetNotifier())
						theApp.emuledlg->TraySetBalloonToolTip(GetResString(IDS_ERROR), strError, NIIF_ERROR);
					bError = true;
				}
				if (!theApp.clientudp->Create()) {
					CString strError;
					strError.Format(GetResString(IDS_MAIN_SOCKETERROR), thePrefs.GetUDPPort());
					LogError(LOG_STATUSBAR|LOG_DONTNOTIFY, _T("%s"), strError);
					if (thePrefs.GetNotifier())
						theApp.emuledlg->TraySetBalloonToolTip(GetResString(IDS_ERROR), strError, NIIF_ERROR);
				}
				
				if (!bError) // show the success msg, only if we had no serious error
				{
					//<<< eWombat [WINSOCK2] for Pawcio: BC
					AddLogLine(false,_T("Winsock: Version %d.%d [%.40s] %.40s"), HIBYTE( theApp.m_wsaData.wVersion ),LOBYTE(theApp.m_wsaData.wVersion ),
						CString(theApp.m_wsaData.szDescription), CString(theApp.m_wsaData.szSystemStatus));
					if (theApp.m_wsaData.iMaxSockets!=0)
						AddLogLine(false,_T("Winsock: max. sockets %d"), theApp.m_wsaData.iMaxSockets);
					else
						AddLogLine(false,_T("Winsock: unlimited sockets"));
					//>>> eWombat [WINSOCK2]
					AddLogLine(true, GetResString(IDS_MAIN_READY), theApp.m_strCurVersionLong + _T(" -LPE-"));
				}
#ifdef HAVE_WIN7_SDK_H
				theApp.emuledlg->UpdateStatusBarProgress();
#endif
				theApp.emuledlg->status++;
				break;
			}
			case 4:
				break;
			case 5:
				if (thePrefs.IsStoringSearchesEnabled())
					theApp.searchlist->LoadSearches();
				theApp.emuledlg->status++;
				break;
			//Xman
			// BEGIN SLUGFILLER: SafeHash - delay load shared files
			case 6:
				theApp.emuledlg->status++;
				//Xman remove unused AICH-hashes
				//theApp.m_AICH_Is_synchronizing=true;
				theApp.sharedfiles->SetOutputCtrl(&theApp.emuledlg->transferwnd->sharedfilesctrl);
				theApp.SplashHide(SW_SHOW); //Xman new slpash-screen arrangement
				//if (!thePrefs.m_bDisableHistoryList)
				//	theApp.emuledlg->transferwnd->historylistctrl.ReloadFileList(); //Xman [MoNKi: -Downloaded History-]

				// BEGIN SiRoB: SafeHash fix originaly in OnInitDialog (delay load shared files)
				// start aichsyncthread
				theApp.UpdateSplash(_T("Synchronize AICH-Hashes...")); //Xman new slpash-screen arrangement
				(new CAICHSyncThread())->start();
				// END SiRoB: SafeHash
				theApp.emuledlg->status++;
				break;
			case 7:
				break;
			case 255:
				break;
			// END SLUGFILLER: SafeHash
			default:
			//Xman
			// BEGIN SLUGFILLER: SafeHash
				theApp.emuledlg->status = 255;
				if(thePrefs.showSpeedGraph && !thePrefs.m_bInvisibleMode && theApp.m_pSpeedGraphWnd) // X: [SGW] - [SpeedGraphWnd]
					theApp.m_pSpeedGraphWnd->ShowWindow(SW_SHOW);
				theApp.UpdateSplash(_T("Ready")); //Xman new slpash-screen arrangement
				//autoconnect only after emule loaded completely
				// ==> Invisible Mode [TPT/MoNKi] - Stulle
				if (!thePrefs.m_bInvisibleMode || !thePrefs.m_bInvisibleModeStart)
					theApp.emuledlg->TrayShow();
				// <== Invisible Mode [TPT/MoNKi] - Stulle
				//if(thePrefs.DoAutoConnect())
					theApp.emuledlg->AutoConnect();
				// wait until emule is ready before opening the wizard
				if (thePrefs.IsFirstStart())
				{
					//Xman new slpash-screen arrangement
					//DestroySplash();
					theApp.DestroySplash();
					//Xman end

					//morph4u update nodes.dat at firststart +
					CString strURL;
		            strURL = thePrefs.GetNodesDatUpdateURL();
		            theApp.emuledlg->UpdateNodesDatFromURL(strURL);
					//morph4u update nodes.dat at firststart -
				}
				else
					theApp.m_dwSplashTime = ::GetCurrentTime();
			// END SLUGFILLER: SafeHash
				theApp.emuledlg->StopTimer();
				break;
		   }
		}
	CATCH_DFLT_EXCEPTIONS(_T("CemuleDlg::StartupTimer"))
		// Maella -Code Improvement-
		// Remark: The macro CATCH_DFLT_EXCEPTIONS will not catch all types of exception.
		//         The exceptions thrown in callback function are not intercepted by the dbghelp.dll (e.g. eMule Dump, crashRpt, etc...)
		catch(...) {
			if(theApp.emuledlg != NULL)
				AddLogLine(true, _T("Unknown %s exception in "), __FUNCTION__);
		}
		// Maella end

}

void CemuleDlg::StopTimer()
{
	if (m_hTimer){
		VERIFY( ::KillTimer(NULL, m_hTimer) );
		m_hTimer = 0;
	}

	theApp.spashscreenfinished=true; //Xman new slpash-screen arrangement

	if (theApp.pstrPendingLink != NULL){
		OnWMData(NULL, (LPARAM)&theApp.sendstruct);
		delete theApp.pstrPendingLink;
		theApp.pstrPendingLink = NULL;
	}
}

void CemuleDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// Systemmenu-Speedselector
	if (nID >= MP_QS_U20 && nID <= MP_QS_UP10) {
		QuickSpeedUpload(nID);
		return;
	}
	if (nID >= MP_QS_U20 && nID <= MP_QS_D100) {
		QuickSpeedDownload(nID);
		return;
	}
	if (/*nID == MP_QS_PA || */nID == MP_QS_UA) {
		QuickSpeedOther();
		return;
	}

	switch (nID /*& 0xFFF0*/)
	{
	case MP_CONNECT:
		StartConnection();
		break;
	case MP_DISCONNECT:
		CloseConnection();
		break;
	case SC_MINIMIZE:
		MinimizeWindow();
	default:
		CTrayDialog::OnSysCommand(nID, lParam);
		break;
	}

	if ((nID & 0xFFF0) == SC_MINIMIZE		||
		(nID & 0xFFF0) == SC_RESTORE		||
		(nID & 0xFFF0) == SC_MAXIMIZE)
	{
		ShowTransferRate(true);
		transferwnd->UpdateCatTabTitles();
	}
}

void CemuleDlg::OnPaint()
{
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
		CTrayDialog::OnPaint();
}

HCURSOR CemuleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CemuleDlg::AutoConnect(){
	if (!theApp.IsConnected())
		//connect if not currently connected
		if (!theApp.serverconnect->IsConnecting() && !Kademlia::CKademlia::IsRunning() ){
			StartConnection();
		    //netMenu.ModifyMenu(0 ,MF_BYPOSITION | MF_STRING, MP_HM_CON,GetResString(IDS_MAIN_BTN_DISCONNECT)); //morph4u
		}
		else {
			CloseConnection();
		    //netMenu.ModifyMenu(0 ,MF_BYPOSITION | MF_STRING, MP_HM_CON,GetResString(IDS_MAIN_BTN_CONNECT)); //morph4u
		}
	else{
		//disconnect if currently connected
		CloseConnection();
		//netMenu.ModifyMenu(0 ,MF_BYPOSITION | MF_STRING, MP_HM_CON,GetResString(IDS_MAIN_BTN_CONNECT)); //morph4u
	}
}

void CemuleDlg::ResetLog(){
	serverwnd->logbox->Reset();
}

void CemuleDlg::ResetDebugLog(){
	serverwnd->debuglog->Reset();
}

void CemuleDlg::AddLogText(UINT uFlags, LPCTSTR pszText)
{
	if (GetCurrentThreadId() != g_uMainThreadId)
	{
		theApp.QueueLogLineEx(uFlags, _T("%s"), pszText);
		return;
	}

	if (uFlags & LOG_STATUSBAR)
	{
        if (statusbar->m_hWnd /*&& ready*/)
		{
			if (theApp.m_app_state == APP_STATE_RUNNING)
				statusbar->SetText(pszText, SBarLog, 0);
		}
		else
		{
			theApp.SplashHide(SW_HIDE); //Xman new slpash-screen arrangement
			XMSGBOXPARAMS params;
			params.nTimeoutSeconds = 5;
			XMessageBox(NULL, pszText, _T("eMule"), MB_OK | MB_ICONINFORMATION, &params);
			theApp.SplashHide(SW_SHOW); //Xman new slpash-screen arrangement
		}
		}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	Debug(_T("%s\n"), pszText);
#endif

	if ((uFlags & LOG_DEBUG) && !thePrefs.GetVerbose())
		return;

	TCHAR temp[1060];
	int iLen = _sntprintf(temp, _countof(temp) - 1, _T("%s: %s\r\n"), CTime::GetCurrentTime().Format(thePrefs.GetDateTimeFormat4Log()), pszText);
	if (iLen < 0){
		_tcsncpy(temp + _countof(temp) - 4, _T("..."), 4);
		iLen = _countof(temp) - 1;
	}
	if (iLen > 0)
	{
		if (!(uFlags & LOG_DEBUG))
		{
			serverwnd->logbox->AddTyped(temp, iLen, uFlags & LOGMSGTYPEMASK);
			if (IsWindow(serverwnd->StatusSelector) && serverwnd->StatusSelector.GetCurSel() != CServerWnd::PaneLog)
				serverwnd->StatusSelector.HighlightItem(CServerWnd::PaneLog, TRUE);
			if (!(uFlags & LOG_DONTNOTIFY) && thePrefs.GetNotifier()){
				if(uFlags & LOG_WARNING)
					TraySetBalloonToolTip(GetResString(IDS_WARNING), pszText, NIIF_WARNING);
				else if(uFlags & LOG_ERROR)
					TraySetBalloonToolTip(GetResString(IDS_ERROR), pszText, NIIF_ERROR);
				else
					TraySetBalloonToolTip(GetResString(IDS_SV_LOG), pszText);
			}
			if (thePrefs.GetLog2Disk())
				theLog.Log(temp, iLen);
		}

		if (thePrefs.GetVerbose() && ((uFlags & LOG_DEBUG) || thePrefs.GetFullVerbose()))
		{
			serverwnd->debuglog->AddTyped(temp, iLen, uFlags & LOGMSGTYPEMASK);
			if (IsWindow(serverwnd->StatusSelector) && serverwnd->StatusSelector.GetCurSel() != CServerWnd::PaneVerboseLog)
				serverwnd->StatusSelector.HighlightItem(CServerWnd::PaneVerboseLog, TRUE);

			if (thePrefs.GetDebug2Disk())
				theVerboseLog.Log(temp, iLen);
		}
	}
}

CString CemuleDlg::GetLastLogEntry()
{
	return serverwnd->logbox->GetLastLogEntry();
}

CString CemuleDlg::GetAllLogEntries()
{
	return serverwnd->logbox->GetAllLogEntries();
}

CString CemuleDlg::GetLastDebugLogEntry()
{
	return serverwnd->debuglog->GetLastLogEntry();
}

CString CemuleDlg::GetAllDebugLogEntries()
{
	return serverwnd->debuglog->GetAllLogEntries();
}

UINT CemuleDlg::GetConnectionStateIconIndex() const
{
	if (theApp.serverconnect->IsConnected() && !Kademlia::CKademlia::IsConnected())
	{
		if (theApp.serverconnect->IsLowID())
			return 1; // LowNot
		else
			return 2; // HighNot
	}
	else if (!theApp.serverconnect->IsConnected() && Kademlia::CKademlia::IsConnected())
	{
		if (Kademlia::CKademlia::IsFirewalled())
			return 1; // NotLow 
	else
			return 2; // NotHigh
	}
	else if (theApp.serverconnect->IsConnected() && Kademlia::CKademlia::IsConnected())
	{
		if (theApp.serverconnect->IsLowID() && Kademlia::CKademlia::IsFirewalled())
			return 1; // LowLow
		else if (theApp.serverconnect->IsLowID())
			return 1; // LowHigh
		else if (Kademlia::CKademlia::IsFirewalled())
			return 1; // HighLow
		else
			return 2; // HighHigh
	}
	else
	{
		return 0; // NotNot
	}
}

void CemuleDlg::ShowConnectionStateIcon()
{
	UINT uIconIdx = GetConnectionStateIconIndex();
	if (uIconIdx >= _countof(connicons)){
		ASSERT(0);
		uIconIdx = 0;
	}
	statusbar->SetIcon(SBarConnected, connicons[uIconIdx]);
}

/*CString CemuleDlg::GetConnectionStateString()
{
	CString status;
	if(Kademlia::CKademlia::IsConnected())
		status = GetResString(IDS_KADEMLIA);
	else
		status = _T("");

	if(theApp.serverconnect->IsConnected())
		status += status.IsEmpty()?_T("eD2K"):_T(" | eD2K");
	return status;
}*/

void CemuleDlg::ShowConnectionState()
{
	theApp.downloadqueue->OnConnectionState(theApp.IsConnected());
	serverwnd->UpdateMyInfo();
	//serverwnd->UpdateControlsState();

	ShowConnectionStateIcon();
	//statusbar->SetText(GetConnectionStateString(), SBarConnected, 0);
	ShowKadCount(); //KadCount
#ifdef HAVE_WIN7_SDK_H
	UpdateThumbBarButtons();
#endif
}

void CemuleDlg::ShowKadCount() //KadCount
{
	CString buffer;
	buffer.Format(_T("%s: %i  %s"), GetResString(IDS_KADEMLIA), contactCount, GetResString(IDS_KADCONTACTLAB));
	statusbar->SetText(buffer, SBarKadCount, 0);
}

/*void CemuleDlg::ShowTransferStateIcon()
{
	if (m_uUpDatarate && m_uDownDatarate)
		statusbar->SetIcon(SBarUpDown, transicons[3]);
	else if (m_uUpDatarate)
		statusbar->SetIcon(SBarUpDown, transicons[2]);
	else if (m_uDownDatarate)
		statusbar->SetIcon(SBarUpDown, transicons[1]);
	else
		statusbar->SetIcon(SBarUpDown, transicons[0]);
}*/

CString CemuleDlg::GetUpDatarateString(/*UINT uUpDatarate*/)
{
	//Xman Code Improvement
	//it's enough to update the datarate in ShowTransferrate
	//m_uUpDatarate = uUpDatarate != (UINT)-1 ? uUpDatarate : theApp.uploadqueue->GetDatarate();
	//Xman end
	TCHAR szBuff[128];
		//Xman
		// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
		_sntprintf(szBuff, _countof(szBuff) - 1, _T("%.1f (%.1f)"), (float)m_uUpDatarate/1024, (float)m_uploadOverheadRate/1024);
		//Xman end
		szBuff[_countof(szBuff) - 1] = _T('\0');
	return szBuff;
}

CString CemuleDlg::GetDownDatarateString(/*UINT uDownDatarate*/)
{
	//Xman Code Improvement
	//it's enough to update the datarate in ShowTransferrate
	//m_uDownDatarate = uDownDatarate != (UINT)-1 ? uDownDatarate : theApp.downloadqueue->GetDatarate();
	//Xman end

	TCHAR szBuff[128];
		//Xman
		// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
		_sntprintf(szBuff, _countof(szBuff) - 1, _T("%.1f (%.1f)"), (float)m_uDownDatarate/1024, (float)m_downloadOverheadRate/1024);
		//Xman end
	szBuff[_countof(szBuff) - 1] = _T('\0');
	return szBuff;
}

CString CemuleDlg::GetTransferRateString()
{
	TCHAR szBuff[128];
		_sntprintf(szBuff, _countof(szBuff) - 1, GetResString(IDS_UPDOWN_NEW),
				  (float)m_uDownDatarate/1024, (float)m_downloadOverheadRate/1024,
				  (float)m_uUpDatarate/1024, (float)m_uploadOverheadRate/1024);
	szBuff[_countof(szBuff) - 1] = _T('\0');
	return szBuff;
}

void CemuleDlg::ShowTransferRate(bool bForceAll)
{

	//Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	// Retrieve the current datarates
	uint32 eMuleIn;	uint32 eMuleInOverall;
	uint32 eMuleOut; uint32 eMuleOutOverall;
	uint32 notUsed;
	theApp.pBandWidthControl->GetDatarates(thePrefs.GetDatarateSamples(),
										   eMuleIn, eMuleInOverall,
										   eMuleOut, eMuleOutOverall,
										   notUsed, notUsed);

	
	m_uDownDatarate = eMuleIn;
	m_uUpDatarate = eMuleOut;
	m_uploadOverheadRate=eMuleOutOverall-eMuleOut;
	m_downloadOverheadRate=eMuleInOverall-eMuleIn;
	//Xman end

	CString strTransferRate = GetTransferRateString();
	// set trayicon-icon
	//UpdateTrayIcon();
	TraySetIcon(m_icoSysTrayCurrent);

	TCHAR buffer2[128];//NOTIFYICONDATA_V2_TIP_SIZE
//>>> WiZaRd::ToolTip-FiX
	//first of all it's not interesting which client we use
	//and second this will create a buggy tool tip because only limited 
	//length is allowed/supported by M$/OS version/IE version
	//It is necessary to change that part in case you are using a rather long mod string/version string (as I do...)
	_sntprintf(buffer2, _countof(buffer2) - 1, _T("%s\r\n%s"), GetResString(theApp.IsConnected()?IDS_CONNECTED:IDS_DISCONNECTED), strTransferRate);
	//buffer2[_countof(buffer2) - 1] = _T('\0');
//<<< WiZaRd::ToolTip-FiX

	// Win98: '\r\n' is not displayed correctly in tooltip
	//if (afxData.bWin95) {
	//	LPTSTR psz = buffer2;
	//	while (*psz) {
	//		if (*psz == _T('\r') || *psz == _T('\n'))
	//			*psz = _T(' ');
	//		psz++;
	//	}
	//}
	TraySetToolTip(buffer2);

	//Xman see all sources
	if (activewnd == transferwnd && IsWindowVisible())
		transferwnd->UpdateFilesCount();
	//Xman end


	if (IsWindowVisible() || bForceAll)
	{
		statusbar->SetText(strTransferRate, SBarUpDown, 0);
		//ShowTransferStateIcon();
        uint_ptr sessionRunTime = (GetTickCount() - theStats.starttime) / 1000; 
		if (IsWindowVisible() && thePrefs.ShowRatesOnTitle())
		{
			TCHAR szBuff[128];
			_sntprintf(szBuff, _countof(szBuff) - 1, _T("(D:%.1f U:%.1f) eMule v%s  (%s)"), (float)m_uDownDatarate/1024, (float)m_uUpDatarate/1024, theApp.m_strCurVersionLong + _T(" -LPE-"),  CastSecondsToLngHM(sessionRunTime));
			szBuff[_countof(szBuff) - 1] = _T('\0');
			SetWindowText(szBuff);
		}
	}
}

void CemuleDlg::OnOK()
{
}

void CemuleDlg::OnCancel()
{
	if (!thePrefs.GetStraightWindowStyles())
		MinimizeWindow();
}

void CemuleDlg::MinimizeWindow()
{
	ShowWindow(SW_MINIMIZE);
	if(m_bTrayIconVisible)
		ShowWindow(SW_HIDE);
	//ShowTransferRate();
}

void CemuleDlg::SetActiveDialog(CWnd* dlg)
{
	if (dlg == activewnd)
		return;
	if (activewnd)
		activewnd->ShowWindow(SW_HIDE);
	dlg->ShowWindow(SW_SHOW);
	dlg->SetFocus();
	activewnd = dlg;
	int iToolbarButtonID = MapWindowToToolbarButton(dlg);
	if (iToolbarButtonID != -1){
		viewMenu.CheckMenuRadioItem(MP_HM_SRVR,MP_HM_STATS,iToolbarButtonID+MP_HM_SRVR-IDC_TOOLBARBUTTON,MF_BYCOMMAND);
		toolbar->PressMuleButton(iToolbarButtonID);
	}
	if (dlg == transferwnd){
		if (thePrefs.ShowCatTabInfos())
			transferwnd->UpdateCatTabTitles();
	}
	else if (dlg == searchwnd){
		if(transferwnd->downloadlistctrl.curTab != 0)
			searchwnd->SetSelectedCat(transferwnd->downloadlistctrl.curTab);// X: [UIC] - [UIChange] follow transfer tab
	}
	else if (dlg == statisticswnd){
		statisticswnd->ShowStatistics();
	}
}

void CemuleDlg::SetStatusBarPartsSize()
{
//>>> WiZaRd::Workaround
	//if startup takes too long then we might run into invalid calls here...
	if(!statusbar || !statusbar->m_hWnd)
		return;
//<<< WiZaRd::Workaround
	RECT rect;
	statusbar->GetClientRect(&rect);
	int aiWidths[5] = 
	{ 
        rect.right - 370, //KadCount
		rect.right - 235, 
		rect.right - 45, 
        rect.right - 20,
		-1
	};
	statusbar->SetParts(_countof(aiWidths), aiWidths);
}

void CemuleDlg::OnSize(UINT nType, int cx, int cy)
{
	CTrayDialog::OnSize(nType, cx, cy);
	SetStatusBarPartsSize();
}

void CemuleDlg::ProcessED2KLink(LPCTSTR pszData)
{
	try {
		CString link2;
		CString link;
		link2 = pszData;
		link2.Replace(_T("%7c"),_T("|"));
		link2.Replace(_T("%7C"),_T("|"));
		link = OptUtf8ToStr(URLDecode(link2));
		CED2KLink* pLink = CED2KLink::CreateLinkFromUrl(link);
		_ASSERT( pLink !=0 );
		switch (pLink->GetKind()) {
			case CED2KLink::kFile:
			{
				//CED2KFileLink* pFileLink = pLink->GetFileLink();
				//_ASSERT(pFileLink !=0);
				CED2KFileLink* pFileLink = (CED2KFileLink*)pLink;
				//Xman [MoNKi: -Check already downloaded files-]
				if(theApp.knownfiles->CheckAlreadyDownloadedFileQuestion(pFileLink->GetHashKey(),pFileLink->GetName()))
					theApp.downloadqueue->AddFileLinkToDownload(pFileLink,transferwnd->downloadlistctrl.curTab);// X: [AC] - [ActionChange] use transferwnd's select cat as default cat
				//Xman end
			}
			break;
			case CED2KLink::kServerList:
			{
				//CED2KServerListLink* pListLink = pLink->GetServerListLink(); 
				//_ASSERT( pListLink !=0 ); 
				CString strAddress = ((CED2KServerListLink*)pLink)->GetAddress(); 
                CString strConfirm;
				strConfirm.Format(GetResString(IDS_CONFIRMSERVERDOWNLOAD), strAddress);
				if(strAddress.GetLength() != 0 && AfxMessageBox(strConfirm, MB_YESNO | MB_ICONQUESTION, 0) == IDYES)
					serverwnd->serverlistctrl.UpdateServerMetFromURL(strAddress);
			}
			break;
			case CED2KLink::kNodesList:
			{
				//CED2KNodesListLink* pListLink = pLink->GetNodesListLink(); 
				//_ASSERT( pListLink !=0 ); 
				CString strAddress = ((CED2KNodesListLink*)pLink)->GetAddress();
				// Because the nodes.dat is vital for kad and its routing and doesn't needs to be updated in general
				// we request a confirm to avoid accidental / malicious updating of this file. This is a bit inconsistent
				// as the same kinda applies to the server.met, but those require more updates and are easier to understand
				CString strConfirm;
				strConfirm.Format(GetResString(IDS_CONFIRMNODESDOWNLOAD), strAddress);
				if(strAddress.GetLength() != 0 && AfxMessageBox(strConfirm, MB_YESNO | MB_ICONQUESTION, 0) == IDYES)
					UpdateNodesDatFromURL(strAddress);
			}
			break;
			case CED2KLink::kServer:
			{
				CString defName;
				//CED2KServerLink* pSrvLink = pLink->GetServerLink();
				//_ASSERT( pSrvLink !=0 );
				CED2KServerLink* pSrvLink = (CED2KServerLink*)pLink;
				CServer* pSrv = new CServer(pSrvLink->GetPort(), pSrvLink->GetAddress());
				_ASSERT( pSrv !=0 );
				pSrvLink->GetDefaultName(defName);
				pSrv->SetListName(defName);

				// Barry - Default all new servers to high priority
				//if (thePrefs.GetManualAddedServersHighPriority())
					pSrv->SetPreference(SRV_PR_HIGH);

				if (!serverwnd->serverlistctrl.AddServer(pSrv,true)) 
					delete pSrv; 
				else
					AddLogLine(true,GetResString(IDS_SERVERADDED), pSrv->GetListName());
			}
			break;
			case CED2KLink::kSearch:
			{
				//CED2KSearchLink* pListLink = pLink->GetSearchLink();
				//_ASSERT( pListLink !=0 ); 
				CED2KSearchLink* pListLink = (CED2KSearchLink*)pLink;
				SetActiveDialog(searchwnd);
				searchwnd->ProcessEd2kSearchLinkRequest(pListLink->GetSearchTerm());
			}
			break;
			default:
				break;
		}
		delete pLink;
	}
	catch(CString strError){
		LogWarning(LOG_STATUSBAR, GetResString(IDS_LINKNOTADDED) + _T(" - ") + strError);
	}
	catch(...){
		LogWarning(LOG_STATUSBAR, GetResString(IDS_LINKNOTADDED));
	}
}

LRESULT CemuleDlg::OnWMData(WPARAM /*wParam*/, LPARAM lParam)
{
	PCOPYDATASTRUCT data = (PCOPYDATASTRUCT)lParam;
	if (data->dwData == OP_ED2KLINK)
	{
		if (thePrefs.bringtoforeground)
		{
			FlashWindow(true);
			if (IsIconic())
				ShowWindow(SW_SHOWNORMAL);
			else
				RestoreWindow();
		}
		ProcessED2KLink((LPCTSTR)data->lpData);
	}
	else if(data->dwData == OP_COLLECTION){
		FlashWindow(TRUE);
		if (IsIconic())
			ShowWindow(SW_SHOWNORMAL);
		else
			RestoreWindow();

		CCollection* pCollection = new CCollection();
		CString strPath = CString((LPCTSTR)data->lpData);
		if(pCollection->InitCollectionFromFile(strPath, strPath.Right((strPath.GetLength()-1)-strPath.ReverseFind('\\')))){
			// NEO: MLD - [ModelesDialogs] -- Xanatos -->
			CCollectionViewDialog* dialog = new CCollectionViewDialog(); 
			dialog->SetCollection(pCollection,true);
			dialog->OpenDialog();
			// NEO: MLD END <-- Xanatos --
			//CCollectionViewDialog dialog;
			//dialog.SetCollection(pCollection);
			//dialog.DoModal();
		}
		else
			delete pCollection;
	}
	else if (data->dwData == OP_CLCOMMAND){
		// command line command received
		CString clcommand((LPCTSTR)data->lpData);
		clcommand.MakeLower();
		AddLogLine(true,_T("CLI: %s"),clcommand);

		if (clcommand==_T("connect")) {StartConnection(); return true;}
		if (clcommand==_T("disconnect")) {theApp.serverconnect->Disconnect(); return true;}
		if (clcommand==_T("resume")) {theApp.downloadqueue->StartNextFile(); return true;}
		if (clcommand==_T("exit"))
		{
			theApp.m_app_state = APP_STATE_SHUTTINGDOWN; // do no ask to close
			OnClose(); 
			return true;
		}
		if (clcommand==_T("restore")) {RestoreWindow();return true;}
		if (clcommand==_T("reloadipf")) {theApp.ipfilter->LoadFromDefaultFile(); return true;}
		if (clcommand.Left(7).MakeLower()==_T("limits=") && clcommand.GetLength()>8) {
			CString down;
			CString up=clcommand.Mid(7);
			int pos=up.Find(_T(','));
			if (pos>0) {
				down=up.Mid(pos+1);
				up=up.Left(pos);
			}
			//Xman
			// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
			if (down.GetLength()>0) thePrefs.SetMaxDownload((float)_tstof(down));
			if (up.GetLength()>0) thePrefs.SetMaxUpload((float)_tstof(up));
			//Xman Xtreme Upload:
			//thePrefs.CheckSlotSpeed();
			//Xman end
			return true;
		}

		if (clcommand==_T("help") || clcommand==_T("/?")) {
			// show usage
			return true;
		}
		
		if (clcommand==_T("status")) {
			CString strBuff;
			strBuff.Format(_T("%sstatus.log"), thePrefs.GetMuleDirectory(EMULE_CONFIGBASEDIR));
			FILE* file = _tfsopen(strBuff, _T("wt"), _SH_DENYWR);
			if (file){
				if (theApp.serverconnect->IsConnected())
					strBuff = GetResString(IDS_CONNECTED);
				else if (theApp.serverconnect->IsConnecting())
					strBuff = GetResString(IDS_CONNECTING);
				else
					strBuff = GetResString(IDS_DISCONNECTED);
				_ftprintf(file, _T("%s\n"), strBuff);

				// Xman
				// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
				strBuff.Format(GetResString(IDS_UPDOWNSMALL_NEW), (float)m_uUpDatarate/1024, (float)m_uDownDatarate/1024);
				// Maella end
				_ftprintf(file, _T("%s"), strBuff); // next string (getTextList) is already prefixed with '\n'!
				_ftprintf(file, _T("%s\n"), theApp.downloadqueue->getTextList());
				
				fclose(file);
			}
			return true;
		}
		// show "unknown command";
	}
	return true;
}

LRESULT CemuleDlg::OnFileHashed(WPARAM wParam, LPARAM lParam)
{
	if (theApp.m_app_state != APP_STATE_RUNNING)
		return FALSE;

	CKnownFile* result = (CKnownFile*)lParam;
	ASSERT( IsKindOfCKnownFile(result)/*result->IsKindOf(RUNTIME_CLASS(CKnownFile))*/ );

	if (wParam)
	{
		// File hashing finished for a part file when:
		// - part file just completed
		// - part file was rehashed at startup because the file date of part.met did not match the part file date

		CPartFile* requester = (CPartFile*)wParam;
		ASSERT( IsCPartFile(requester)/*requester->IsKindOf(RUNTIME_CLASS(CPartFile))*/ );

		// SLUGFILLER: SafeHash - could have been canceled
		if (theApp.downloadqueue->IsPartFile(requester))
			requester->PartFileHashFinished(result);
		else
			delete result;
		// SLUGFILLER: SafeHash
	}
	else
	{
		ASSERT( !IsCPartFile(result)/*!result->IsKindOf(RUNTIME_CLASS(CPartFile))*/ );

		// File hashing finished for a shared file (none partfile)
		//	- reading shared directories at startup and hashing files which were not found in known.met
		//	- reading shared directories during runtime (user hit Reload button, added a shared directory, ...)
		theApp.sharedfiles->FileHashingFinished(result);
	}
	return TRUE;
}

LRESULT CemuleDlg::OnFileOpProgress(WPARAM wParam, LPARAM lParam)
{
	if (theApp.m_app_state != APP_STATE_RUNNING)
		return FALSE;

	CKnownFile* pKnownFile = (CKnownFile*)lParam;
	ASSERT( IsKindOfCKnownFile(pKnownFile)/*pKnownFile->IsKindOf(RUNTIME_CLASS(CKnownFile))*/ );

	if (IsCPartFile(pKnownFile)/*pKnownFile->IsKindOf(RUNTIME_CLASS(CPartFile))*/)
	{
		CPartFile* pPartFile = static_cast<CPartFile*>(pKnownFile);
		ASSERT( wParam <= 100 );
		pPartFile->SetFileOpProgress((UINT)wParam);
		if(activewnd == transferwnd){
		pPartFile->UpdateDisplayedInfo(true);
			transferwnd->partstatusctrl.Refresh(pPartFile);
		}
	}

	return 0;
}

//Xman
// BEGIN SLUGFILLER: SafeHash
LRESULT CemuleDlg::OnHashFailed(WPARAM /*wParam*/, LPARAM lParam)
{
	// BEGIN SiRoB: Fix crash at shutdown
	if (theApp.m_app_state != APP_STATE_RUNNING) {
		UnknownFile_Struct* hashed = (UnknownFile_Struct*)lParam;
		delete hashed;
		return FALSE;
	}
	// END SiRoB: Fix crash at shutdown
	theApp.sharedfiles->HashFailed((UnknownFile_Struct*)lParam);
	return 0;
}

LRESULT CemuleDlg::OnFileAllocExc(WPARAM wParam,LPARAM lParam)
{
	//Xman
	//MORPH START - Added by SiRoB, Fix crash at shutdown
	ASSERT(!(theApp.m_app_state == APP_STATE_RUNNING && theApp.downloadqueue==NULL)); 

	CFileException* error = (CFileException*)lParam;
	if (theApp.m_app_state != APP_STATE_RUNNING || theApp.downloadqueue==NULL || !theApp.downloadqueue->IsPartFile((CPartFile*)wParam)) { //MORPH - Changed by SiRoB, Flush Thread
		if (error != NULL)
			error->Delete();
		return FALSE;
	}
	//MORPH END   - Added by SiRoB, Fix crash at shutdown
	if (lParam == 0)
		((CPartFile*)wParam)->FlushBuffersExceptionHandler();
	else
		((CPartFile*)wParam)->FlushBuffersExceptionHandler(error);
	return 0;
}

LRESULT CemuleDlg::OnFileCompleted(WPARAM wParam, LPARAM lParam)
{
	//MORPH START - Added by SiRoB, Fix crash at shutdown
	if (theApp.m_app_state != APP_STATE_RUNNING)
		return FALSE;
	//MORPH END   - Added by SiRoB, Fix crash at shutdown
	CPartFile* partfile = (CPartFile*)lParam;
	ASSERT( partfile != NULL );
	if (partfile)
		partfile->PerformFileCompleteEnd((DWORD)wParam);
	return 0;
}

#ifdef _DEBUG
void BeBusy(UINT uSeconds, LPCSTR pszCaller)
{
	UINT s = 0;
	while (uSeconds--) {
		theVerboseLog.Logf(_T("%hs: called=%hs, waited %u sec."), __FUNCTION__, pszCaller, s++);
		Sleep(1000);
	}
}
#endif

BOOL CemuleDlg::OnQueryEndSession()
{
	AddDebugLogLine(DLP_VERYLOW, _T("%hs"), __FUNCTION__);
	if (!CTrayDialog::OnQueryEndSession())
		return FALSE;

	AddDebugLogLine(DLP_VERYLOW, _T("%hs: returning TRUE"), __FUNCTION__);
	return TRUE;
}

void CemuleDlg::OnEndSession(BOOL bEnding)
{
	AddDebugLogLine(DLP_VERYLOW, _T("%hs: bEnding=%d"), __FUNCTION__, bEnding);
	if (bEnding && theApp.m_app_state == APP_STATE_RUNNING)
	{
		// If eMule was *not* started with "RUNAS":
		// When user is logging of (or reboots or shutdown system), Windows sends the
		// WM_QUERYENDSESSION/WM_ENDSESSION to all top level windows.
		// Here we can consume as much time as we need to perform our shutdown. Even if we
		// take longer than 20 seconds, Windows will just show a dialog box that 'emule'
		// is not terminating in time and gives the user a chance to cancel that. If the user
		// does not cancel the Windows dialog, Windows will though wait until eMule has 
		// terminated by itself - no data loss, no file corruption, everything is fine.
		theApp.m_app_state= APP_STATE_SHUTTINGDOWN;
		OnClose();
	}

	CTrayDialog::OnEndSession(bEnding);
	AddDebugLogLine(DLP_VERYLOW, _T("%hs: returning"), __FUNCTION__);
}

LRESULT CemuleDlg::OnUserChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	AddDebugLogLine(DLP_VERYLOW, _T("%hs"), __FUNCTION__);
	// Just want to know if we ever get this message. Maybe it helps us to handle the
	// logoff/reboot/shutdown problem when eMule was started with "RUNAS".
	return Default();
}

LRESULT CemuleDlg::OnConsoleThreadEvent(WPARAM wParam, LPARAM lParam)
{
	AddDebugLogLine(DLP_VERYLOW, _T("%hs: nEvent=%u, nThreadID=%u"), __FUNCTION__, wParam, lParam);

	// If eMule was started with "RUNAS":
	// This message handler receives a 'console event' from the concurrently and thus
	// asynchronously running console control handler thread which was spawned by Windows
	// in case the user logs off/reboots/shutdown. Even if the console control handler thread
	// is waiting on the result from this message handler (is waiting until the main thread
	// has finished processing this inter-application message), the application will get
	// forcefully terminated by Windows after 20 seconds! There is no known way to prevent
	// that. This means, that if we would invoke our standard shutdown code ('OnClose') here
	// and the shutdown takes longer than 20 sec, we will get forcefully terminated by 
	// Windows, regardless of what we are doing. This means, MET-file and PART-file corruption
	// may occure. Because the shutdown code in 'OnClose' does also shutdown Kad (which takes
	// a noticeable amount of time) it is not that unlikely that we run into problems with
	// not being finished with our shutdown in 20 seconds.
	// 
	if (theApp.m_app_state == APP_STATE_RUNNING)
	{
#if 1
		// And it really should be OK to expect that emule can shutdown in 20 sec on almost
		// all computers. So, use the proper shutdown.
		theApp.m_app_state = APP_STATE_SHUTTINGDOWN;
		OnClose();	// do not invoke if shutdown takes longer than 20 sec, read above
#else
		// As a minimum action we at least set the 'shutting down' flag, this will help e.g.
		// the CUploadQueue::UploadTimer to not start any file save actions which could get
		// interrupted by windows and which would then lead to corrupted MET-files.
		// Setting this flag also helps any possible running threads to stop their work.
		theApp.m_app_state = APP_STATE_SHUTTINGDOWN;

#ifdef _DEBUG
		// Simulate some work.
		//
		// NOTE: If the console thread has already exited, Windows may terminate the process
		// even before the 20 sec. timeout!
		//BeBusy(70, __FUNCTION__);
#endif

		// Actually, just calling 'ExitProcess' should be the most safe thing which we can
		// do here. Because we received this message via the main message queue we are 
		// totally in-sync with the application and therefore we know that we are currently
		// not within a file save action and thus we simply can not cause any file corruption
		// when we exit right now.
		//
		// Of course, there may be some data loss. But it's the same amount of data loss which
		// could occure if we keep running. But if we keep running and wait until Windows
		// terminates us after 20 sec, there is also the chance for file corruption.
		if (thePrefs.GetDebug2Disk()) {
			theVerboseLog.Logf(_T("%hs: ExitProcess"), __FUNCTION__);
			theVerboseLog.Close();
		}
		ExitProcess(0);
#endif
	}

	AddDebugLogLine(DLP_VERYLOW, _T("%hs: returning"), __FUNCTION__);
	return 1;
}

void CemuleDlg::OnDestroy()
{
	AddDebugLogLine(DLP_VERYLOW, _T("%hs"), __FUNCTION__);

	// If eMule was started with "RUNAS":
	// When user is logging of (or reboots or shutdown system), Windows may or may not send 
	// a WM_DESTROY (depends on how long the application needed to process the 
	// CTRL_LOGOFF_EVENT). But, regardless of what happened and regardless of how long any
	// application specific shutdown took, Windows fill forcefully terminate the process 
	// after 1-2 seconds after WM_DESTROY! So, we can not use WM_DESTROY for any lengthy
	// shutdown actions in that case.
	CTrayDialog::OnDestroy();
}

bool CemuleDlg::CanClose()
{
	if (theApp.m_app_state == APP_STATE_RUNNING && thePrefs.IsConfirmExitEnabled())
	{
//XMessageBox +
        XMSGBOXPARAMS params;
		params.nTimeoutSeconds = 10; // timeout in seconds (before default button selected),YES: MB_DEFBUTTON1,NO: MB_DEFBUTTON2
		int nResult = XMessageBox(GetSafeHwnd(), GetResString(IDS_MAIN_EXIT), GetResString(IDS_CLOSEEMULE), MB_YESNO | MB_DEFBUTTON1/*MB_DEFBUTTON2*/ | MB_DONOTASKAGAIN | MB_ICONQUESTION, &params);
//XMessageBox -
		if ((nResult & MB_DONOTASKAGAIN) > 0)
			thePrefs.SetConfirmExit(false);
		if ((nResult & 0xFF) == IDNO)
			return false;
	}
	return true;
}

void CemuleDlg::OnClose()
{
	if(theApp.IsSplash()) theApp.DestroySplash(); //Xman new slpash-screen arrangement

	// X-Ray :: AutoRestartIfNecessary :: Start
	/*
	if (!CanClose())
	*/
	if (!theApp.m_bRestartApp && !CanClose())
	// X-Ray :: AutoRestartIfNecessary :: End
		return;

	TrayHide();
	//Xman new slpash-screen arrangement
	if (thePrefs.UseSplashScreen())// morph4u
	{
			//Xman don't show splash on old windows->crash
/*			switch (thePrefs.GetWindowsVersion())
			{
			case _WINVER_98_:
			case _WINVER_95_:	
			case _WINVER_ME_:
				break;
			default:*/
		ShowWindow(SW_HIDE);
		theApp.ShowSplash(false);
//			}
	}
	//Xman end

	Log(_T("Closing eMule"));

	//zz_fly :: known2 buffer 
	//write all buffer in to file
	//if lock fail, there must be another thread saving the hashset, not needed to save it again
	Poco::FastMutex::SingleLock lockSaveHashSet(CAICHRecoveryHashSet::m_mutSaveHashSet);
	if (lockSaveHashSet.Lock(3000)){
		CAICHRecoveryHashSet::SaveHashSetToFile(true); 
		lockSaveHashSet.Unlock();
	}
	//zz_fly :: end
	theApp.m_app_state = APP_STATE_SHUTTINGDOWN;

	// X: queued disc-access for read/flushing-threads
	theApp.m_DiscAccessQueue.stop();

	if(theApp.m_pSpeedGraphWnd){
		theApp.m_pSpeedGraphWnd->CloseDialog();	theApp.m_pSpeedGraphWnd = NULL; // X: [SGW] - [SpeedGraphWnd]
	}

	theApp.serverconnect->Disconnect();

	// get main window placement
	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	GetWindowPlacement(&wp);
	ASSERT( wp.showCmd == SW_SHOWMAXIMIZED || wp.showCmd == SW_SHOWMINIMIZED || wp.showCmd == SW_SHOWNORMAL );
	if (wp.showCmd == SW_SHOWMINIMIZED && (wp.flags & WPF_RESTORETOMAXIMIZED))
		wp.showCmd = SW_SHOWMAXIMIZED;
	wp.flags = 0;
	thePrefs.SetWindowLayout(wp);

	// get active main window dialog
	if (thePrefs.GetRestoreLastMainWndDlg() && activewnd){
		if (activewnd->IsKindOf(RUNTIME_CLASS(CServerWnd)))
			thePrefs.SetLastMainWndDlgID(IDD_SERVER);
		else if (activewnd->IsKindOf(RUNTIME_CLASS(CSearchDlg)))
			thePrefs.SetLastMainWndDlgID(IDD_SEARCH);
		else if (activewnd->IsKindOf(RUNTIME_CLASS(CTransferWnd)))
			thePrefs.SetLastMainWndDlgID(IDD_TRANSFER);
		else if (activewnd->IsKindOf(RUNTIME_CLASS(CStatisticsDlg)))
			thePrefs.SetLastMainWndDlgID(IDD_STATISTICS);
		else{
			ASSERT(0);
			thePrefs.SetLastMainWndDlgID(0);
		}
	}

	Kademlia::CKademlia::Stop(); 	// couple of data files are written

	theApp.UpdateSplash(_T("Waiting for hash end")); //Xman new slpash-screen arrangement

	// try to wait until the hashing thread notices that we are shutting down
	Poco::FastMutex::SingleLock sLock1(theApp.hashing_mut); // only one filehash at a time
	sLock1.Lock(2000);

	//Xman queued disc-access for read/flushing-threads
	//if we don't unlock we can cause a deadlock here:
	//resuming the disc-access-thread with th above ForeAllDiscAccessThreadsToFinish
	//doesn't mean the thread start at once... they start later and the main app has the mutex
	//so the threads wait for ever.. and also the main app for the threads.
	sLock1.Unlock(); 
	//Xman end

	theApp.UpdateSplash(_T("Saving settings ...")); //Xman new slpash-screen arrangement

	preferenceswnd->CloseDialog(); // NEO: MLD - [ModelesDialogs] <-- Xanatos --
	// saving data & stuff

	// ==> Threaded Known Files Saving [Stulle] - Stulle
	if (theApp.knownfiles->isRunning()) // we just saved something
		theApp.knownfiles->join();
	else // we might have missed something
	// <== Threaded Known Files Saving [Stulle] - Stulle
		theApp.knownfiles->Save();										// CKnownFileList::Save
	theApp.sharedfiles->Save();
	serverwnd->SaveAllSettings();

	//Xman new adapter selection
	if(theApp.pBandWidthControl->GetwasNAFCLastActive())
		thePrefs.SetNAFCFullControl(true);
	//Xman end

	theApp.searchlist->SaveSpamFilter();
	if (thePrefs.IsStoringSearchesEnabled())
		theApp.searchlist->StoreSearches();

	// close uPnP Ports
	theApp.m_pUPnPFinder->GetImplementation()->StopAsyncFind();
	if (thePrefs.CloseUPnPOnExit())
		theApp.m_pUPnPFinder->GetImplementation()->DeletePorts();

	//Xman don't overwrite bak files if last sessions crashed
	//remark: it would be better to set the flag after all deletions, but this isn't possible, because the prefs need access to the objects when saving
	//thePrefs.m_this_session_aborted_in_an_unnormal_way=false;
	bool last_session_aborted_in_an_unnormal_way = thePrefs.m_last_session_aborted_in_an_unnormal_way;
	thePrefs.m_last_session_aborted_in_an_unnormal_way = false;// X: [CI] - [Code Improvement]
	thePrefs.Save();
	thePrefs.m_last_session_aborted_in_an_unnormal_way = last_session_aborted_in_an_unnormal_way;
	//Xman end
	thePerfLog.Shutdown();

	// ==> Invisible Mode [TPT/MoNKi] - Stulle
	if(thePrefs.m_bInvisibleMode)
		UnRegisterInvisibleHotKey();
	// <== Invisible Mode [TPT/MoNKi] - Stulle

	theApp.UpdateSplash(_T("Clearing displayed items ...")); //Xman new slpash-screen arrangement

	// explicitly delete all listview items which may hold ptrs to objects which will get deleted
	// by the dtors (some lines below) to avoid potential problems during application shutdown.
	transferwnd->downloadlistctrl.DeleteAllItems();

	for (POSITION pos = theApp.downloadqueue->filelist.GetHeadPosition();pos != 0;){ // X: [ISS] - [Improved Source Save]
		CPartFile* file = theApp.downloadqueue->filelist.GetNext(pos);
		if(!file->IsStopped())
			file->m_sourcesaver.SaveSources();
	}
	theApp.clientlist->DeleteAll();
	searchwnd->DeleteAllSearchListCtrlItems();
	transferwnd->uploadlistctrl.DeleteAllItems();
	transferwnd->sharedfilesctrl.DeleteAllItems();
    transferwnd->historylistctrl.DeleteAllItems();
	serverwnd->serverlistctrl.DeleteAllItems();
	transferwnd->downloadclientsctrl.DeleteAllItems();

    theApp.uploadBandwidthThrottler->EndThread();
    //Xman
	//theApp.lastCommonRouteFinder->EndThread();

	theApp.sharedfiles->DeletePartFileInstances();

	searchwnd->SendMessage(WM_CLOSE);

	theApp.UpdateSplash(_T("Clearing lists ..."));  //Xman new slpash-screen arrangement

	//Xman
	theApp.m_threadlock.WriteLock();	// SLUGFILLER: SafeHash - Last chance, let all running threads close before we start deleting

    // NOTE: Do not move those dtors into 'CemuleApp::InitInstance' (althought they should be there). The
	// dtors are indirectly calling functions which access several windows which would not be available 
	// after we have closed the main window -> crash!
	delete theApp.listensocket;		theApp.listensocket = NULL;
	delete theApp.clientudp;		theApp.clientudp = NULL;
	delete theApp.sharedfiles;		theApp.sharedfiles = NULL;
	delete theApp.serverconnect;	theApp.serverconnect = NULL;
	delete theApp.serverlist;		theApp.serverlist = NULL;		// CServerList::SaveServermetToFile
	delete theApp.knownfiles;		theApp.knownfiles = NULL;
	delete theApp.searchlist;		theApp.searchlist = NULL;
	theApp.UpdateSplash(_T("Saving credits ..."));  //Xman new slpash-screen arrangement
	delete theApp.clientcredits;	theApp.clientcredits = NULL;	// CClientCreditsList::SaveList
	theApp.UpdateSplash(_T("Clearing queues ..."));  //Xman new slpash-screen arrangement
	delete theApp.downloadqueue;	theApp.downloadqueue = NULL;	// N * (CPartFile::FlushBuffer + CPartFile::SavePartFile)
	delete theApp.uploadqueue;		theApp.uploadqueue = NULL;
	delete theApp.clientlist;		theApp.clientlist = NULL;
	theApp.UpdateSplash(_T("Unloading IPfilter ..."));  //Xman new slpash-screen arrangement
	delete theApp.ipfilter;			theApp.ipfilter = NULL;
	delete theApp.m_pFirewallOpener;theApp.m_pFirewallOpener = NULL;
	theApp.UpdateSplash(_T("Shutdown bandwidthcontrol ..."));  //Xman new slpash-screen arrangement
	delete theApp.uploadBandwidthThrottler; theApp.uploadBandwidthThrottler = NULL;
	//Xman
	//delete theApp.lastCommonRouteFinder; theApp.lastCommonRouteFinder = NULL;
	delete theApp.m_pUPnPFinder;	theApp.m_pUPnPFinder = NULL; //Official UPNP

	//Xman
	// Maella [patch] -Bandwidth: overall bandwidth measure-	
	delete theApp.pBandWidthControl;theApp.pBandWidthControl = NULL;
	// Maella end

	//EastShare Start - added by AndCycle, IP to Country
	delete theApp.ip2country;		theApp.ip2country = NULL;
	//EastShare End   - added by AndCycle, IP to Country

	// X: [FV] - [FileVerify]
	delete theApp.fileverify;		theApp.fileverify = NULL;

	theApp.UpdateSplash(_T("Shutdown done")); //Xman new slpash-screen arrangement

	thePrefs.Uninit();
	theApp.m_app_state = APP_STATE_DONE;
	CTrayDialog::OnCancel();
	AddDebugLogLine(DLP_VERYLOW, _T("Closed eMule"));
}

void CemuleDlg::StartConnection()
{
	if (   (!theApp.serverconnect->IsConnecting() && !theApp.serverconnect->IsConnected())
		|| !Kademlia::CKademlia::IsRunning())
	{
		// UPnP is still trying to open the ports. In order to not get a LowID by connecting to the servers / kad before
		// the ports are opened we delay the connection untill UPnP gets a result or the timeout is reached
		// If the user clicks two times on the button, let him have his will and connect regardless
		if (m_hUPnPTimeOutTimer != 0 && !m_bConnectRequestDelayedForUPnP){
			AddLogLine(false, GetResString(IDS_DELAYEDBYUPNP));
			AddLogLine(true, GetResString(IDS_DELAYEDBYUPNP2));
			m_bConnectRequestDelayedForUPnP = true;
			return;
		}
		else{
			m_bConnectRequestDelayedForUPnP = false;
			if (m_hUPnPTimeOutTimer != 0){
				VERIFY( ::KillTimer(NULL, m_hUPnPTimeOutTimer) );
				m_hUPnPTimeOutTimer = 0;
			}
			//AddLogLine(true, GetResString(IDS_CONNECTING)); //morph4u todo

			// ed2k
			if ((thePrefs.GetNetworkED2K() || m_bEd2kSuspendDisconnect) && !theApp.serverconnect->IsConnecting() && !theApp.serverconnect->IsConnected()) {
				theApp.serverconnect->ConnectToAnyServer();
			}

			// kad
			if (/*(thePrefs.GetNetworkKademlia() || m_bKadSuspendDisconnect) &&*/ !Kademlia::CKademlia::IsRunning()) { //morph4u todo
				Kademlia::CKademlia::Start();
			}
		} //Official UPNP

		ShowConnectionState();
	}
	m_bEd2kSuspendDisconnect = false;
	m_bKadSuspendDisconnect = false;
}

void CemuleDlg::CloseConnection()
{
	if (theApp.serverconnect->IsConnected()){
		theApp.serverconnect->Disconnect();
	}

	if (theApp.serverconnect->IsConnecting()){
		theApp.serverconnect->StopConnectionTry();
	}
	Kademlia::CKademlia::Stop();
	ShowConnectionState();
}

void CemuleDlg::RestoreWindow()
{
	if (m_wpFirstRestore.length)
	{
		SetWindowPlacement(&m_wpFirstRestore);
		memset(&m_wpFirstRestore, 0, sizeof m_wpFirstRestore);
	}
	else if(!IsWindowVisible()||IsIconic())
		ShowWindow(SW_RESTORE);
	SetForegroundWindow();
	BringWindowToTop();
}

/*void CemuleDlg::UpdateTrayIcon()
{
	// X: [SGW] - [SpeedGraphWnd] remove meter icon
	HICON newSysTrayIcon  = 
		theApp.IsConnected()?
		(theApp.IsFirewalled()?m_icoSysTrayLowID:m_icoSysTrayConnected)
		:m_icoSysTrayDisconnected;
	if (newSysTrayIcon != m_icoSysTrayCurrent){
		m_icoSysTrayCurrent = newSysTrayIcon;
		TraySetIcon(m_icoSysTrayCurrent);
}
}*/

int CemuleDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	return CTrayDialog::OnCreate(lpCreateStruct);
}

void CemuleDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	if (IsRunning() && bShow == TRUE)
		ShowTransferRate(true);
	CTrayDialog::OnShowWindow(bShow, nStatus);
}

void CemuleDlg::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	TRACE(_T("CemuleDlg::OnSettingChange: uFlags=0x%08x  lpszSection=\"%s\"\n"), lpszSection);
	// Do not update the Shell's large icon size, because we still have an image list
	// from the shell which contains the old large icon size.
	theApp.UpdateDesktopColorDepth();
	CTrayDialog::OnSettingChange(uFlags, lpszSection);
}

void CemuleDlg::OnSysColorChange()
{
	theApp.UpdateDesktopColorDepth();
	CTrayDialog::OnSysColorChange();
	//SetAllIcons();
}

HBRUSH CemuleDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = GetCtlColor(pDC, pWnd, nCtlColor);
	if (hbr)
		return hbr;
	return __super::OnCtlColor(pDC, pWnd, nCtlColor);
}

HBRUSH CemuleDlg::GetCtlColor(CDC* /*pDC*/, CWnd* /*pWnd*/, UINT nCtlColor)
{
	UNREFERENCED_PARAMETER(nCtlColor);
	// This function could be used to give the entire eMule (at least all of the main windows)
	// a somewhat more Vista like look by giving them all a bright background color.
	// However, again, the ownerdrawn tab controls are noticeable disturbing that attempt. They
	// do not change their background color accordingly. They don't use NMCUSTOMDRAW nor to they
	// use WM_CTLCOLOR...
	//
	//if (theApp.m_ullComCtrlVer >= MAKEDLLVERULL(6,16,0,0) && g_xpStyle.IsThemeActive() && g_xpStyle.IsAppThemed()) {
	//	if (nCtlColor == CTLCOLOR_DLG || nCtlColor == CTLCOLOR_STATIC)
	//		return GetSysColorBrush(COLOR_WINDOW);
	//}
	return NULL;
}

void CemuleDlg::SetAllIcons()
{
	// application icon (although it's not customizable, we may need to load a different color resolution)
	if (m_hIcon)
		VERIFY( ::DestroyIcon(m_hIcon) );
	// NOTE: the application icon name is prefixed with "AAA" to make sure it's alphabetically sorted by the
	// resource compiler as the 1st icon in the resource table!
	m_hIcon = AfxGetApp()->LoadIcon(_T("AAAEMULEAPP"));
	SetIcon(m_hIcon, TRUE);
	// this scales the 32x32 icon down to 16x16, does not look nice at least under WinXP
	//SetIcon(m_hIcon, FALSE);

	// connection state
	for (int i = 0; i < _countof(connicons); i++){
		if (connicons[i]) VERIFY( ::DestroyIcon(connicons[i]) );
	}
	connicons[0] = theApp.LoadIcon(_T("ConnectedNot"), 16, 16);
	connicons[1] = theApp.LoadIcon(_T("ConnectedLow"), 16, 16);
	connicons[2] = theApp.LoadIcon(_T("ConnectedHigh"), 16, 16);
	ShowConnectionStateIcon();

	// transfer state
	/*if (transicons[0]) VERIFY( ::DestroyIcon(transicons[0]) );
	if (transicons[1]) VERIFY( ::DestroyIcon(transicons[1]) );
	if (transicons[2]) VERIFY( ::DestroyIcon(transicons[2]) );
	if (transicons[3]) VERIFY( ::DestroyIcon(transicons[3]) );
	transicons[0] = theApp.LoadIcon(_T("UP0DOWN0"), 16, 16);
	transicons[1] = theApp.LoadIcon(_T("UP0DOWN1"), 16, 16);
	transicons[2] = theApp.LoadIcon(_T("UP1DOWN0"), 16, 16);
	transicons[3] = theApp.LoadIcon(_T("UP1DOWN1"), 16, 16);
	ShowTransferStateIcon();*/

	// traybar icons
	
	if (m_icoSysTrayCurrent) VERIFY( ::DestroyIcon(m_icoSysTrayCurrent) );
	//if (m_icoSysTrayConnected) VERIFY( ::DestroyIcon(m_icoSysTrayConnected) );
	//if (m_icoSysTrayDisconnected) VERIFY( ::DestroyIcon(m_icoSysTrayDisconnected) );
	//if (m_icoSysTrayLowID) VERIFY( ::DestroyIcon(m_icoSysTrayLowID) );
	m_icoSysTrayCurrent = theApp.LoadIcon(_T("AAAEMULEAPP"), 16, 16);
	//m_icoSysTrayConnected = theApp.LoadIcon(_T("TrayConnected"), 16, 16);
	//m_icoSysTrayDisconnected = theApp.LoadIcon(_T("TrayNotConnected"), 16, 16);
	//m_icoSysTrayLowID = theApp.LoadIcon(_T("TrayLowID"), 16, 16);
	ShowTransferRate(true);
}

void CemuleDlg::CreateMenues()
{
	if (toolMenu)
		VERIFY( toolMenu.DestroyMenu() );
	if (viewMenu)
		VERIFY( viewMenu.DestroyMenu() );
	if (fileMenu)
		VERIFY( fileMenu.DestroyMenu() );
	if (mainMenu)
		VERIFY( mainMenu.DestroyMenu() );
    //if (netMenu)
	//	VERIFY( netMenu.DestroyMenu() );
    if (shutMenu)
		VERIFY( shutMenu.DestroyMenu() );
    
	mainMenu.CreateMenu();

    /*netMenu.CreateMenu();
	netMenu.AppendMenu(MF_STRING,MP_HM_CON,GetResString(IDS_MAIN_BTN_CONNECT));
	netMenu.AppendMenu(MF_SEPARATOR);

	mainMenu.AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)netMenu.m_hMenu, _T("Network"));*/

	fileMenu.CreateMenu();
	fileMenu.AppendMenu(MF_STRING,MP_HM_DIRECT_DOWNLOAD, GetResString(IDS_SW_DIRECTDOWNLOAD) + _T("..."));
	fileMenu.AppendMenu(MF_STRING,MP_HM_RESUME_DOWNLOAD, GetResString(IDS_SW_RESUMEDOWNLOAD) + _T("..."));
	fileMenu.AppendMenu(MF_STRING,MP_VIEWCOLLECTION, GetResString(IDS_VIEWCOLLECTION));

	mainMenu.AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)fileMenu.m_hMenu, GetResString(IDS_MNUFILE));

	viewMenu.CreateMenu();
	viewMenu.AppendMenu(MF_STRING,MP_HM_SRVR, GetResString(IDS_SERVER));
	viewMenu.AppendMenu(MF_STRING,MP_HM_TRANSFER, GetResString(IDS_EM_TRANS));
	viewMenu.AppendMenu(MF_STRING,MP_HM_SEARCH, GetResString(IDS_EM_SEARCH));
	viewMenu.AppendMenu(MF_STRING,MP_HM_STATS, GetResString(IDS_EM_STATISTIC));
	viewMenu.AppendMenu(MF_SEPARATOR);
	viewMenu.AppendMenu(((thePrefs.isshowtoolbar)?MF_CHECKED:MF_UNCHECKED),MP_HM_SHOWTOOLBAR, GetResString(IDS_MNUTOOLBAR));
	viewMenu.AppendMenu(((thePrefs.isshowcatbar)?MF_CHECKED:MF_UNCHECKED),MP_HM_SHOWCATTABBAR, GetResString(IDS_MNUCATTABBAR));

	viewMenu.AppendMenu(((thePrefs.m_bShowProgressBar)?MF_CHECKED:MF_UNCHECKED),MP_PROGRESS, _T("Show Progress Bar")); //morph4u :: PercentBar
	viewMenu.AppendMenu(((thePrefs.m_bGridlines)?MF_CHECKED:MF_UNCHECKED),MP_GRIDLINES, _T("Show GridLines"));
	viewMenu.AppendMenu(((thePrefs.m_bShowSystemIcon)?MF_CHECKED:MF_UNCHECKED),MP_LISTICONS, _T("Show System Icons"));

	mainMenu.AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)viewMenu.m_hMenu, GetResString(IDS_MNUVIEW));

	toolMenu.CreateMenu();
	toolMenu.AppendMenu(MF_STRING,MP_HM_PREFS, GetResString(IDS_EM_PREFS));
	toolMenu.AppendMenu(MF_SEPARATOR);
	toolMenu.AppendMenu(MF_STRING,MP_HM_QUICKSTART,GetResString(IDS_X_QUICK_START_START)); // NEO: QS - [QuickStart] <-- Xanatos --
	toolMenu.AppendMenu((thePrefs.showSpeedGraph?MF_CHECKED:MF_UNCHECKED),MP_HM_TOGGLESPEEDGRAPH, GetResString(IDS_TOGGLESPEEDGRAPH));
	toolMenu.AppendMenu(MF_STRING,MP_HM_REMOVEALLBANNEDCLIENTS, GetResString(IDS_SW_REMOVEALLBANNEDCLIENTS)); 

	mainMenu.AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)toolMenu.m_hMenu, GetResString(IDS_TOOLS));

    shutMenu.CreateMenu();
	shutMenu.AppendMenu(((thePrefs.m_bShutDownMule)?MF_CHECKED:MF_UNCHECKED),MP_SHUT_EMULE, _T("eMule on downloads complete"));
	shutMenu.AppendMenu(((thePrefs.m_bShutDownPC)?MF_CHECKED:MF_UNCHECKED),MP_SHUT_PC, _T("PC on downloads complete"));
	shutMenu.AppendMenu(MF_SEPARATOR);
	shutMenu.AppendMenu(MF_STRING,MP_HM_EXIT, GetResString(IDS_EXIT));

	mainMenu.AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)shutMenu.m_hMenu, _T("Shutdown"));

	int iToolbarButtonID = MapWindowToToolbarButton(activewnd);
	if (iToolbarButtonID != -1)
		viewMenu.CheckMenuRadioItem(MP_HM_SRVR,MP_HM_STATS,iToolbarButtonID+MP_HM_SRVR-IDC_TOOLBARBUTTON,MF_BYCOMMAND);
	SetMenu(&mainMenu);
}

void CemuleDlg::CreateTrayMenues()
{
	if (downspeedmenu)
		VERIFY( downspeedmenu.DestroyMenu() );
	if (upspeedmenu)
		VERIFY( upspeedmenu.DestroyMenu() );
	if (trayPopup)
		VERIFY( trayPopup.DestroyMenu() );
	trayPopup.CreatePopupMenu();
	CString text;
	trayPopup.AppendMenu(MF_STRING, MP_CONNECT, GetResString(IDS_MAIN_BTN_CONNECT)); 
	trayPopup.AppendMenu(MF_SEPARATOR);

	trayPopup.AppendMenu(MF_STRING,MP_HM_OPENINC, GetResString(IDS_OPENINC));
	trayPopup.AppendMenu(MF_STRING,MP_HM_DIRECT_DOWNLOAD, GetResString(IDS_SW_DIRECTDOWNLOAD) + _T("..."));
	trayPopup.AppendMenu(MF_STRING,MP_VIEWCOLLECTION, GetResString(IDS_VIEWCOLLECTION));
	trayPopup.AppendMenu((thePrefs.showSpeedGraph?MF_CHECKED:MF_UNCHECKED),MP_HM_TOGGLESPEEDGRAPH, GetResString(IDS_TOGGLESPEEDGRAPH));
	trayPopup.AppendMenu(MF_SEPARATOR);

	// Create UploadPopup Menu
	ASSERT( upspeedmenu.m_hMenu == NULL );
	if (upspeedmenu.CreateMenu())
	{
		//Xman modified
		text.Format(_T("20%%\t%i KB/s"),  (uint16)(thePrefs.maxGraphUploadRate*0.2));	upspeedmenu.AppendMenu(MF_STRING, MP_QS_U20,  text);
		text.Format(_T("40%%\t%i KB/s"),  (uint16)(thePrefs.maxGraphUploadRate*0.4));	upspeedmenu.AppendMenu(MF_STRING, MP_QS_U40,  text);
		text.Format(_T("60%%\t%i KB/s"),  (uint16)(thePrefs.maxGraphUploadRate*0.6));	upspeedmenu.AppendMenu(MF_STRING, MP_QS_U60,  text);
		text.Format(_T("80%%\t%i KB/s"),  (uint16)(thePrefs.maxGraphUploadRate*0.8));	upspeedmenu.AppendMenu(MF_STRING, MP_QS_U80,  text);
		text.Format(_T("90%%\t%i KB/s"),  (uint16)(thePrefs.maxGraphUploadRate*0.9));	upspeedmenu.AppendMenu(MF_STRING, MP_QS_U100, text);
		//Xman end

		text.Format(_T("%s:"), GetResString(IDS_PW_UPL));
		trayPopup.AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)upspeedmenu.m_hMenu, text);
	}
	// Create DownloadPopup Menu
	ASSERT( downspeedmenu.m_hMenu == NULL );
	if (downspeedmenu.CreateMenu())
	{
		text.Format(_T("20%%\t%i KB/s"),  (uint16)(thePrefs.maxGraphDownloadRate*0.2));	downspeedmenu.AppendMenu(MF_STRING|MF_POPUP, MP_QS_D20,  text);
		text.Format(_T("40%%\t%i KB/s"),  (uint16)(thePrefs.maxGraphDownloadRate*0.4));	downspeedmenu.AppendMenu(MF_STRING|MF_POPUP, MP_QS_D40,  text);
		text.Format(_T("60%%\t%i KB/s"),  (uint16)(thePrefs.maxGraphDownloadRate*0.6));	downspeedmenu.AppendMenu(MF_STRING|MF_POPUP, MP_QS_D60,  text);
		text.Format(_T("80%%\t%i KB/s"),  (uint16)(thePrefs.maxGraphDownloadRate*0.8));	downspeedmenu.AppendMenu(MF_STRING|MF_POPUP, MP_QS_D80,  text);
		text.Format(_T("100%%\t%i KB/s"), (uint16)(thePrefs.maxGraphDownloadRate));		downspeedmenu.AppendMenu(MF_STRING|MF_POPUP, MP_QS_D100, text);

		text.Format(_T("%s:"), GetResString(IDS_PW_DOWNL));
		trayPopup.AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)downspeedmenu.m_hMenu, text);
	}
	trayPopup.AppendMenu(MF_STRING, MP_HM_PREFS, GetResString(IDS_EM_PREFS));
	trayPopup.AppendMenu(MF_SEPARATOR);
	trayPopup.AppendMenu(MF_STRING, MP_RESTORE, GetResString(IDS_MAIN_POPUP_RESTORE));
	trayPopup.AppendMenu(MF_STRING, MP_EXIT , GetResString(IDS_EXIT));

}

void CemuleDlg::Localize()// X: [RUL] - [Remove Useless Localize]
{
	preferenceswnd->Localize();
	statisticswnd->Localize();
	serverwnd->Localize();
	serverwnd->serverlistctrl.Localize();
	transferwnd->LocalizeAll();
	searchwnd->Localize();
	toolbar->Localize();
	CreateMenues();
	CreateTrayMenues();
	ShowConnectionState();
	ShowTransferRate(true);
}

void CemuleDlg::QuickSpeedOther()
{
	//Xman changed the values !
	// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
/*	switch (nID) {
		case MP_QS_PA:
			thePrefs.SetMaxUpload((float)(3));
			thePrefs.SetMaxDownload((float)(1));
			//thePrefs.CheckSlotSpeed(); //XMan Xtreme Upload
			break ;
		case MP_QS_UA: */
	thePrefs.SetMaxUpload((float)(thePrefs.maxGraphUploadRate-2));
	thePrefs.SetMaxDownload((float)(thePrefs.maxGraphDownloadRate));
	//thePrefs.CheckSlotSpeed(); //Xman Xtreme Upload
/*			break ;
	}*/
	//Xman end
}


void CemuleDlg::QuickSpeedUpload(UINT nID)
{
	switch (nID) {
		//Xman
		// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
		case MP_QS_U20: thePrefs.SetMaxUpload((float)(thePrefs.GetMaxGraphUploadRate()*0.2)); break ;
		case MP_QS_U40: thePrefs.SetMaxUpload((float)(thePrefs.GetMaxGraphUploadRate()*0.4)); break ;
		case MP_QS_U60: thePrefs.SetMaxUpload((float)(thePrefs.GetMaxGraphUploadRate()*0.6)); break ;
		case MP_QS_U80: thePrefs.SetMaxUpload((float)(thePrefs.GetMaxGraphUploadRate()*0.8)); break ;
		case MP_QS_U100: thePrefs.SetMaxUpload(thePrefs.GetMaxGraphUploadRate()); break ;
		case MP_QS_UP10: thePrefs.SetMaxUpload(GetRecMaxUpload()); break ;
		//Xman end
	}
	//thePrefs.CheckSlotSpeed(); //Xman Xtreme Upload
}

void CemuleDlg::QuickSpeedDownload(UINT nID)
{
	switch (nID) {
		//Xman
		// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
		case MP_QS_D20: thePrefs.SetMaxDownload(0.2f * thePrefs.GetMaxGraphDownloadRate()); break ;
		case MP_QS_D40: thePrefs.SetMaxDownload(0.4f * thePrefs.GetMaxGraphDownloadRate()); break ;
		case MP_QS_D60: thePrefs.SetMaxDownload(0.6f * thePrefs.GetMaxGraphDownloadRate()); break ;
		case MP_QS_D80: thePrefs.SetMaxDownload(0.8f * thePrefs.GetMaxGraphDownloadRate()); break ;
		case MP_QS_D100: thePrefs.SetMaxDownload(thePrefs.GetMaxGraphDownloadRate()); break ;
		//Xman end
	}
}
// quick-speed changer -- based on xrmb

//Xman
// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
float CemuleDlg::GetRecMaxUpload() {
	/*
	if (thePrefs.GetMaxGraphUploadRate()<7) return 0;
	if (thePrefs.GetMaxGraphUploadRate()<15) return thePrefs.GetMaxGraphUploadRate()-1.5f;
	return (thePrefs.GetMaxGraphUploadRate()-2.5f);
	*/
	//Xman changed 6.0.1
	if (thePrefs.GetMaxGraphUploadRate()<10) return thePrefs.GetMaxGraphUploadRate()-1.5f;
	if (thePrefs.GetMaxGraphUploadRate()<20) return thePrefs.GetMaxGraphUploadRate()-2.5f;
	if (thePrefs.GetMaxGraphUploadRate()<26) return thePrefs.GetMaxGraphUploadRate()-3.0f;
	return (thePrefs.GetMaxGraphUploadRate()*0.9f);

}
//Xman end

BOOL CemuleDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{	
		case TBBTN_SERVER:
		case MP_HM_SRVR:
			SetActiveDialog(serverwnd);
			break;
		case TBBTN_TRANSFERS:
		case MP_HM_TRANSFER:
			SetActiveDialog(transferwnd);
			break;
		case TBBTN_SEARCH:
		case MP_HM_SEARCH:
			SetActiveDialog(searchwnd);
			break;
		case TBBTN_STATS:
		case MP_HM_STATS:
			SetActiveDialog(statisticswnd);
			break;
		case TBBTN_OPTIONS:
		case MP_HM_PREFS:
			toolbar->CheckButton(TBBTN_OPTIONS, TRUE);
			ShowPreferences();
			toolbar->CheckButton(TBBTN_OPTIONS, FALSE);
			break;
		case MP_HM_SHOWTOOLBAR:
			thePrefs.isshowtoolbar=!thePrefs.isshowtoolbar;
			viewMenu.CheckMenuItem(MP_HM_SHOWTOOLBAR,(thePrefs.isshowtoolbar)?MF_BYCOMMAND|MF_CHECKED:MF_BYCOMMAND|MF_UNCHECKED);
			RemoveAllAnchors();
			ShowToolbar();
			break;
        case MP_HM_SHOWCATTABBAR:
			thePrefs.isshowcatbar=!thePrefs.isshowcatbar;
			transferwnd->ShowCatTab();
			viewMenu.CheckMenuItem(MP_HM_SHOWCATTABBAR,(thePrefs.isshowcatbar)?MF_BYCOMMAND|MF_CHECKED:MF_BYCOMMAND|MF_UNCHECKED);
			break;
		case MP_GRIDLINES:
			thePrefs.m_bGridlines=!thePrefs.m_bGridlines;
			viewMenu.CheckMenuItem(MP_GRIDLINES,(thePrefs.m_bGridlines)?MF_BYCOMMAND|MF_CHECKED:MF_BYCOMMAND|MF_UNCHECKED);
            transferwnd->downloadlistctrl.SetGridLine();
            transferwnd->downloadclientsctrl.SetGridLine();
            transferwnd->uploadlistctrl.SetGridLine();
            transferwnd->sharedfilesctrl.SetGridLine();
            transferwnd->historylistctrl.SetGridLine();
            serverwnd->serverlistctrl.SetGridLine();
            searchwnd->m_pwndResults->searchlistctrl.SetGridLine();
			break;
        case MP_SHUT_EMULE:
			thePrefs.m_bShutDownMule=!thePrefs.m_bShutDownMule;
			shutMenu.CheckMenuItem(MP_SHUT_EMULE,(thePrefs.m_bShutDownMule)?MF_BYCOMMAND|MF_CHECKED:MF_BYCOMMAND|MF_UNCHECKED);
			break;
        case MP_SHUT_PC:
			thePrefs.m_bShutDownPC=!thePrefs.m_bShutDownPC;
			shutMenu.CheckMenuItem(MP_SHUT_PC,(thePrefs.m_bShutDownPC)?MF_BYCOMMAND|MF_CHECKED:MF_BYCOMMAND|MF_UNCHECKED);
			break;
			//morph4u :: PercentBar :: Star
		case MP_PROGRESS:
			thePrefs.m_bShowProgressBar=!thePrefs.m_bShowProgressBar;
			viewMenu.CheckMenuItem(MP_PROGRESS,(thePrefs.m_bShowProgressBar)?MF_BYCOMMAND|MF_CHECKED:MF_BYCOMMAND|MF_UNCHECKED);
			break;
			//morph4u :: PercentBar :: End
        case MP_LISTICONS:
			thePrefs.m_bShowSystemIcon=!thePrefs.m_bShowSystemIcon;
			viewMenu.CheckMenuItem(MP_LISTICONS,(thePrefs.m_bShowSystemIcon)?MF_BYCOMMAND|MF_CHECKED:MF_BYCOMMAND|MF_UNCHECKED);
			break;
        /*case MP_HM_CON:
			AutoConnect();
			break;*/
		case MP_HM_EXIT:
			OnClose();
			break;
		case MP_HM_DIRECT_DOWNLOAD:{
			// NEO: MLD - [ModelesDialogs] -- Xanatos -->
			CDirectDownloadDlg* dialog = new CDirectDownloadDlg(); 
			dialog->OpenDialog();
			// NEO: MLD END <-- Xanatos --
			//CDirectDownloadDlg dlg;
			//dlg.DoModal();
			break;
			}
		case MP_HM_RESUME_DOWNLOAD:{
			// NEO: MLD - [ModelesDialogs] -- Xanatos -->
			CResumeDownloadDlg* dialog = new CResumeDownloadDlg(); // X: [IP] - [Import Parts]
			dialog->OpenDialog();
			// NEO: MLD END <-- Xanatos --
			break;
			}
		case MP_HM_REMOVEALLBANNEDCLIENTS:{ 
			theApp.clientlist->RemoveAllBannedClients();
			break;
			}
		case MP_VIEWCOLLECTION:{
			CFileDialog dlg(true, COLLECTION_FILEEXTENSION, NULL, OFN_FILEMUSTEXIST|OFN_HIDEREADONLY, GetResString(IDS_SEARCH_EMULECOLLECTION) + _T(" (*") COLLECTION_FILEEXTENSION _T(")|*") COLLECTION_FILEEXTENSION _T("||"));
			if(dlg.DoModal()!=IDOK)
				break;
			CCollection* pCollection = new CCollection();
			if(pCollection->InitCollectionFromFile(dlg.GetPathName(), dlg.GetFileName())){
				// NEO: MLD - [ModelesDialogs] -- Xanatos -->
				CCollectionViewDialog* dialog = new CCollectionViewDialog(); 
				dialog->SetCollection(pCollection,true);
				dialog->OpenDialog();
				// NEO: MLD END <-- Xanatos --
				//CCollectionViewDialog dialog;
				//dialog.SetCollection(pCollection);
				//dialog.DoModal();
			}
			else
				delete pCollection;
			break;
		}
		// NEO: QS - [QuickStart] -- Xanatos -->
		case MP_HM_QUICKSTART:
			if(!thePrefs.m_bOnQuickStart)
				theApp.downloadqueue->DoQuickStart();
			else
				theApp.downloadqueue->StopQuickStart();
			break;
		// NEO: QS END <-- Xanatos --
        case MP_HM_TOGGLESPEEDGRAPH: // X: [SGW] - [SpeedGraphWnd]
			if(theApp.m_pSpeedGraphWnd){
				theApp.m_pSpeedGraphWnd->ShowWindow((thePrefs.showSpeedGraph = !theApp.m_pSpeedGraphWnd->IsWindowVisible())?SW_SHOW:SW_HIDE);
				toolMenu.CheckMenuItem(MP_HM_TOGGLESPEEDGRAPH, thePrefs.showSpeedGraph?MF_CHECKED:MF_UNCHECKED);
				trayPopup.CheckMenuItem(MP_HM_TOGGLESPEEDGRAPH, thePrefs.showSpeedGraph?MF_CHECKED:MF_UNCHECKED);
			}
			break;
		case MP_HM_OPENINC: // X: [SGW] - [SpeedGraphWnd]
			ShellExecute(NULL, _T("open"), thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR),NULL, NULL, SW_SHOW);
			break;
	}	

#ifdef HAVE_WIN7_SDK_H
	if (HIWORD(wParam) == THBN_CLICKED) {
		OnTBBPressed(LOWORD(wParam));
		return TRUE;
	}
#endif

	return CTrayDialog::OnCommand(wParam, lParam);
}

LRESULT CemuleDlg::OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu)
{
	UINT nCmdID;
	if (toolbar->MapAccelerator((TCHAR)nChar, &nCmdID)){
		OnCommand(nCmdID, 0);
		return MAKELONG(0,MNC_CLOSE);
	}
	return CTrayDialog::OnMenuChar(nChar, nFlags, pMenu);
}

// Barry - To find out if app is running or shutting/shut down
bool CemuleDlg::IsRunning()
{
	return (theApp.m_app_state == APP_STATE_RUNNING);
}

void CemuleDlg::ShowTrayPopup(CPoint&pt)
{
	if (!IsRunning())
		return;

	if(theApp.IsConnected())
		trayPopup.ModifyMenu(0,MF_BYPOSITION | MF_STRING, MP_DISCONNECT, GetResString(IDS_MAIN_BTN_DISCONNECT)); 
	else
		trayPopup.ModifyMenu(0,MF_BYPOSITION | MF_STRING, MP_CONNECT, GetResString(IDS_MAIN_BTN_CONNECT));
	if(!IsWindowVisible()||IsIconic())
		trayPopup.ModifyMenu(11,MF_BYPOSITION | MF_STRING, MP_RESTORE, GetResString(IDS_MAIN_POPUP_RESTORE));
	else
		trayPopup.ModifyMenu(11,MF_BYPOSITION | MF_STRING, SC_MINIMIZE, GetResString(IDS_PW_TRAY));

	SetForegroundWindow();
	trayPopup.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
}

void StraightWindowStyles(CWnd* pWnd)
{
	CWnd* pWndChild = pWnd->GetWindow(GW_CHILD);
	while (pWndChild)
	{
		StraightWindowStyles(pWndChild);
		pWndChild = pWndChild->GetNextWindow();
	}

	CHAR szClassName[MAX_PATH];
	if (::GetClassNameA(*pWnd, szClassName, _countof(szClassName)))
	{
		if (__ascii_stricmp(szClassName, "Button") == 0)
			pWnd->ModifyStyle(BS_FLAT, 0);
		else if (   (__ascii_stricmp(szClassName, "EDIT") == 0 && (pWnd->GetExStyle() & WS_EX_STATICEDGE))
			|| __ascii_stricmp(szClassName, "SysListView32") == 0
			|| __ascii_stricmp(szClassName, "msctls_trackbar32") == 0
			)
		{
			pWnd->ModifyStyleEx(WS_EX_STATICEDGE, WS_EX_CLIENTEDGE);
		}
		//else if (__ascii_stricmp(szClassName, "SysTreeView32") == 0)
		//{
		//	pWnd->ModifyStyleEx(WS_EX_STATICEDGE, WS_EX_CLIENTEDGE);
		//}
	}
}

static bool s_bIsXPStyle;

void FlatWindowStyles(CWnd* pWnd)
{
	CWnd* pWndChild = pWnd->GetWindow(GW_CHILD);
	while (pWndChild)
	{
		FlatWindowStyles(pWndChild);
		pWndChild = pWndChild->GetNextWindow();
	}

	CHAR szClassName[MAX_PATH];
	if (::GetClassNameA(*pWnd, szClassName, _countof(szClassName)))
	{
		if (__ascii_stricmp(szClassName, "Button") == 0)
		{
			if (!s_bIsXPStyle || (pWnd->GetStyle() & BS_ICON) == 0)
				pWnd->ModifyStyle(0, BS_FLAT);
		}
		else if (__ascii_stricmp(szClassName, "SysListView32") == 0)
		{
			pWnd->ModifyStyleEx(WS_EX_CLIENTEDGE, WS_EX_STATICEDGE);
		}
		else if (__ascii_stricmp(szClassName, "SysTreeView32") == 0)
		{
			pWnd->ModifyStyleEx(WS_EX_CLIENTEDGE, WS_EX_STATICEDGE);
		}
	}
}

void InitWindowStyles(CWnd* pWnd)
{
	if (thePrefs.GetStraightWindowStyles() < 0)
		return;
//	else if (thePrefs.GetStraightWindowStyles() > 0)
		/*StraightWindowStyles(pWnd);*/	// no longer needed
	else if (thePrefs.GetStraightWindowStyles() == 0)
	{
		s_bIsXPStyle = g_xpStyle.IsAppThemed() && g_xpStyle.IsThemeActive();
		if (!s_bIsXPStyle)
			FlatWindowStyles(pWnd);
	}
}

BOOL CemuleApp::IsIdleMessage(MSG *pMsg)
{
	// This function is closely related to 'CemuleDlg::OnKickIdle'.
	//
	// * See MFC source code for 'CWnd::RunModalLoop' to see how those functions are related 
	//	 to each other.
	//
	// * See MFC documentation for 'CWnd::IsIdleMessage' to see why WM_TIMER messages are
	//	 filtered here.
	//
	// Generally we want to filter WM_TIMER messages because they are triggering idle
	// processing (e.g. cleaning up temp. MFC maps) and because they are occuring very often
	// in eMule (we have a rather high frequency timer in upload queue). To save CPU load but
	// do not miss the chance to cleanup MFC temp. maps and other stuff, we do not use each
	// occuring WM_TIMER message -- that would just be overkill! However, we can not simply
	// filter all WM_TIMER messages. If eMule is running in taskbar the only messages which
	// are received by main window are those WM_TIMER messages, thus those messages are the
	// only chance to trigger some idle processing. So, we must use at last some of those
	// messages because otherwise we would not do any idle processing at all in some cases.
	//

	static DWORD s_dwLastIdleMessage;
	if (pMsg->message == WM_TIMER)
	{
		// Allow this WM_TIMER message to trigger idle processing only if we did not do so
		// since some seconds.
		const DWORD dwNow = GetTickCount();
		if (dwNow - s_dwLastIdleMessage >= SEC2MS(5))
		{
			s_dwLastIdleMessage = dwNow;
			return TRUE;// Request idle processing (will send a WM_KICKIDLE)
		}
		return FALSE;	// No idle processing
	}

	if (!CWinApp::IsIdleMessage(pMsg))
		return FALSE;	// No idle processing

	s_dwLastIdleMessage = GetTickCount();
	return TRUE;		// Request idle processing (will send a WM_KICKIDLE)
}

LRESULT CemuleDlg::OnKickIdle(WPARAM /*nWhy*/, LPARAM lIdleCount)
{
	LRESULT lResult = 0;
	//Xman new slpash-screen arrangement
	if (theApp.IsSplash() && theApp.spashscreenfinished)
	{
		if (::GetCurrentTime() - theApp.m_dwSplashTime > 250) 
		{
			// timeout expired, destroy the splash window
			theApp.DestroySplash();
			UpdateWindow();
		}
		else
		{
			// check again later...
			lResult = 1;
		}
	}
	//Xman end
	
	if (!m_bStartMinimizedChecked){
		//TODO: Use full initialized 'WINDOWPLACEMENT' and remove the 'OnCancel' call...
		// Isn't that easy.. Read comments in OnInitDialog..
		m_bStartMinimizedChecked = true;
		if (m_bStartMinimized){
			ShowWindow(SW_HIDE);
			// ==> Invisible Mode [TPT/MoNKi] - Stulle
			if (thePrefs.m_bInvisibleMode && thePrefs.m_bInvisibleModeStart)
				b_HideApp = true;
			// <== Invisible Mode [TPT/MoNKi] - Stulle
		}
	}

	if (searchwnd && searchwnd->m_hWnd)
	{
		if (theApp.m_app_state != APP_STATE_SHUTTINGDOWN)
		{
//#ifdef _DEBUG
//			TCHAR szDbg[80];
//			wsprintf(szDbg, L"%10u: lIdleCount=%d, %s", GetTickCount(), lIdleCount, (lIdleCount > 0) ? L"FreeTempMaps" : L"");
//			SetWindowText(szDbg);
//			TRACE(_T("%s\n"), szDbg);
//#endif
			// NOTE: See also 'CemuleApp::IsIdleMessage'. If 'CemuleApp::IsIdleMessage'
			// would not filter most of the WM_TIMER messages we might get a performance
			// problem here because the idle processing would be performed very, very often.
			//
			// The default MFC implementation of 'CWinApp::OnIdle' is sufficient for us. We
			// will get called with 'lIdleCount=0' and with 'lIdleCount=1'.
			//
			// CWinApp::OnIdle(0)	takes care about pending MFC GUI stuff and returns 'TRUE'
			//						to request another invocation to perform more idle processing
			// CWinApp::OnIdle(>=1)	frees temporary internally MFC maps and returns 'FALSE'
			//						because no more idle processing is needed.

			//Xman Code Improvement
			//enough to clean up handle maps every minute
			static uint32 lastprocess;
			if(lIdleCount>0)
			{
				theApp.OnIdle((LONG)lIdleCount); //free maps
				lastprocess=::GetTickCount();
				return 0;
			}

			if(theApp.OnIdle(0 /*lIdleCount*/) && ::GetTickCount() - lastprocess > MIN2MS(1)) 
				return 1;
			else
				return 0;
			//Xman end
		}
	}

	return lResult;
}


int CemuleDlg::MapWindowToToolbarButton(CWnd* pWnd) const
{
	int iButtonID = -1;
	if (pWnd == transferwnd)        iButtonID = TBBTN_TRANSFERS;
	else if (pWnd == serverwnd)     iButtonID = TBBTN_SERVER;
	else if (pWnd == searchwnd)     iButtonID = TBBTN_SEARCH;
	else if (pWnd == statisticswnd)	iButtonID = TBBTN_STATS;
	else ASSERT(0);
	return iButtonID;
}

CWnd* CemuleDlg::MapToolbarButtonToWindow(int iButtonID) const
{
	CWnd* pWnd;
	switch (iButtonID)
	{
		case TBBTN_TRANSFERS:	pWnd = transferwnd;		break;
		case TBBTN_SERVER:		pWnd = serverwnd;		break;
		case TBBTN_SEARCH:		pWnd = searchwnd;		break;
		case TBBTN_STATS:		pWnd = statisticswnd;	break;
		default:				pWnd = NULL; ASSERT(0);	break;
	}
	return pWnd;
}

bool CemuleDlg::IsWindowToolbarButton(int iButtonID) const
{
	switch (iButtonID)
	{
		case TBBTN_TRANSFERS:	return true;
		case TBBTN_SERVER:		return true;
		case TBBTN_SEARCH:		return true;
		case TBBTN_STATS:		return true;
	}
	return false;
}

int CemuleDlg::GetNextWindowToolbarButton(int iButtonID, int iDirection) const
{
	ASSERT( iDirection == 1 || iDirection == -1 );
	int iButtonCount = toolbar->GetButtonCount();
	if (iButtonCount > 0)
	{
		int iButtonIdx = toolbar->CommandToIndex(iButtonID);
		if (iButtonIdx >= 0 && iButtonIdx < iButtonCount)
		{
			int iEvaluatedButtons = 0;
			while (iEvaluatedButtons < iButtonCount)
			{
				iButtonIdx = iButtonIdx + iDirection;
				if (iButtonIdx < 0)
					iButtonIdx = iButtonCount - 1;
				else if (iButtonIdx >= iButtonCount)
					iButtonIdx = 0;

				TBBUTTON tbbt = {0};
				if (toolbar->GetButton(iButtonIdx, &tbbt))
				{
					if (IsWindowToolbarButton(tbbt.idCommand))
						return tbbt.idCommand;
				}
				iEvaluatedButtons++;
			}
		}
	}
	return -1;
}

BOOL CemuleDlg::PreTranslateMessage(MSG* pMsg)
{
	BOOL bResult = CTrayDialog::PreTranslateMessage(pMsg);

	//Xman new slpash-screen arrangement
	//if (m_pSplashWnd && m_pSplashWnd->m_hWnd != NULL &&
	if (theApp.IsSplash() && 
		(pMsg->message == WM_KEYDOWN	   ||
		 pMsg->message == WM_SYSKEYDOWN	   ||
		 pMsg->message == WM_LBUTTONDOWN   ||
		 pMsg->message == WM_RBUTTONDOWN   ||
		 pMsg->message == WM_MBUTTONDOWN   ||
		 pMsg->message == WM_NCLBUTTONDOWN ||
		 pMsg->message == WM_NCRBUTTONDOWN ||
		 pMsg->message == WM_NCMBUTTONDOWN))
	{
		theApp.DestroySplash();
		UpdateWindow();
	}
	//Xman end
	else
	{
		if (pMsg->message == WM_KEYDOWN)
		{
			// Handle Ctrl+Tab and Ctrl+Shift+Tab
			if (pMsg->wParam == VK_TAB && GetAsyncKeyState(VK_CONTROL) < 0)
			{
				int iButtonID = MapWindowToToolbarButton(activewnd);
				if (iButtonID != -1)
				{
					int iNextButtonID = GetNextWindowToolbarButton(iButtonID, GetAsyncKeyState(VK_SHIFT) < 0 ? -1 : 1);
					if (iNextButtonID != -1)
					{
						CWnd* pWndNext = MapToolbarButtonToWindow(iNextButtonID);
						if (pWndNext)
							SetActiveDialog(pWndNext);
					}
				}
			}
		}
	}

	return bResult;
}

INT_PTR CemuleDlg::ShowPreferences(UINT uStartPageID)
{
	if (preferenceswnd->IsDialogOpen())
	{
		preferenceswnd->SetForegroundWindow();
		preferenceswnd->BringWindowToTop();
		return -1;
	}
	else
	{
		if (uStartPageID != (UINT)-1)
			preferenceswnd->SetStartPage(uStartPageID);
		// NEO: NCFG - [NeoConfiguration] -- Xanatos --
		//return preferenceswnd->DoModal();
		// NEO: MLD - [ModelesDialogs] -- Xanatos -->
		preferenceswnd->OpenDialog(SW_SHOW, FALSE);
		return 0;
		// NEO: MLD END <-- Xanatos --
	}
}

void CemuleDlg::SetToolTipsDelay(UINT uMilliseconds)
{
	//searchwnd->SetToolTipsDelay(uMilliseconds);
	transferwnd->SetToolTipsDelay(uMilliseconds);
}

void CemuleDlg::UPnPTimeOutTimer(HWND /*hwnd*/, UINT /*uiMsg*/, UINT_PTR /*idEvent*/, DWORD /*dwTime*/){
	::PostMessage(theApp.emuledlg->GetSafeHwnd(), UM_UPNP_RESULT, (WPARAM)CUPnPImpl::UPNP_TIMEOUT, 0);
}

LRESULT CemuleDlg::OnUPnPResult(WPARAM wParam, LPARAM lParam){
	bool bWasRefresh = lParam != 0;
	if (!bWasRefresh && wParam == CUPnPImpl::UPNP_FAILED){		
		// UPnP failed, check if we can retry it with another implementation
		if (theApp.m_pUPnPFinder->SwitchImplentation()){
			StartUPnP(false);
			return 0;
		}
		else
			DebugLog(_T("No more available UPnP implementations left"));

	}
	if (m_hUPnPTimeOutTimer != 0){
		VERIFY( ::KillTimer(NULL, m_hUPnPTimeOutTimer) );
		m_hUPnPTimeOutTimer = 0;
	}
	if(IsRunning() && m_bConnectRequestDelayedForUPnP){
		StartConnection();
	}
	if (!bWasRefresh && wParam == CUPnPImpl::UPNP_OK){
		theApp.listensocket->boundcheck = true;//Xman NAFC
		// remember the last working implementation
		thePrefs.SetLastWorkingUPnPImpl(theApp.m_pUPnPFinder->GetImplementation()->GetImplementationID());
		Log(GetResString(IDS_UPNPSUCCESS), theApp.m_pUPnPFinder->GetImplementation()->GetUsedTCPPort()
			, theApp.m_pUPnPFinder->GetImplementation()->GetUsedUDPPort());
	}
	else if (!bWasRefresh)
		LogWarning(GetResString(IDS_UPNPFAILED));

	serverwnd->UpdateMyInfo(); //zz_fly :: show UPnP status
	return 0;
}

LRESULT  CemuleDlg::OnPowerBroadcast(WPARAM wParam, LPARAM lParam)
{
	//DebugLog(_T("DEBUG:Power state change. wParam=%d lPararm=%ld"),wParam,lParam);
	switch (wParam) {
		case PBT_APMRESUMEAUTOMATIC:
		{
			if (m_bEd2kSuspendDisconnect || m_bKadSuspendDisconnect)
			{
				DebugLog(_T("Reconnect after Power state change. wParam=%d lPararm=%ld"),wParam,lParam);
				RefreshUPnP(true);
				PostMessage(WM_SYSCOMMAND , MP_CONNECT, 0); // tell to connect.. a sec later...
			}
			return TRUE; // message processed.
			break;
		}
		case PBT_APMSUSPEND:
		{		
			DebugLog(_T("System is suspending operation, disconnecting. wParam=%d lPararm=%ld"),wParam,lParam);
			m_bEd2kSuspendDisconnect = theApp.serverconnect->IsConnected();
			m_bKadSuspendDisconnect = Kademlia::CKademlia::IsConnected();
			CloseConnection();
			return TRUE; // message processed.
			break;
		}
		default:
			return FALSE; // we do not process this message
	}

}

void CemuleDlg::StartUPnP(bool bReset, uint16 nForceTCPPort, uint16 nForceUDPPort) {
	if (theApp.m_pUPnPFinder != NULL && (m_hUPnPTimeOutTimer == 0 || !bReset)){
		if (bReset){
			theApp.m_pUPnPFinder->Reset();
			Log(GetResString(IDS_UPNPSETUP));
		}
		try
		{
			if (theApp.m_pUPnPFinder->GetImplementation()->IsReady()){
				theApp.m_pUPnPFinder->GetImplementation()->SetMessageOnResult(GetSafeHwnd(), UM_UPNP_RESULT);
				if (bReset)
					VERIFY( (m_hUPnPTimeOutTimer = ::SetTimer(NULL, NULL, SEC2MS(40), UPnPTimeOutTimer)) != NULL );
				theApp.m_pUPnPFinder->GetImplementation()->StartDiscovery(((nForceTCPPort != 0) ? nForceTCPPort : thePrefs.GetPort())
					, ((nForceUDPPort != 0) ? nForceUDPPort :thePrefs.GetUDPPort())
					, (thePrefs.GetServerUDPPort() == 0xFFFF) ? theApp.serverconnect->GetUDPPort() : thePrefs.GetServerUDPPort());
			}
			else
				::PostMessage(theApp.emuledlg->GetSafeHwnd(), UM_UPNP_RESULT, (WPARAM)CUPnPImpl::UPNP_FAILED, 0);
		}
		catch ( CUPnPImpl::UPnPError& ) {}
		catch ( CException* e ) { e->Delete(); }
	}
	else
		ASSERT( false );
}

void CemuleDlg::RefreshUPnP(bool bRequestAnswer)
{
	if (!thePrefs.IsUPnPEnabled())
		return;
	if (theApp.m_pUPnPFinder != NULL && m_hUPnPTimeOutTimer == 0){
		try
		{
			if (theApp.m_pUPnPFinder->GetImplementation()->IsReady())
			{
				if (bRequestAnswer)
					theApp.m_pUPnPFinder->GetImplementation()->SetMessageOnResult(GetSafeHwnd(), UM_UPNP_RESULT);
				if (theApp.m_pUPnPFinder->GetImplementation()->CheckAndRefresh() && bRequestAnswer)
				{
					VERIFY( (m_hUPnPTimeOutTimer = ::SetTimer(NULL, NULL, SEC2MS(10), UPnPTimeOutTimer)) != NULL );
				}
				else
					theApp.m_pUPnPFinder->GetImplementation()->SetMessageOnResult(0, 0);
			}
			else
				DebugLogWarning(_T("RefreshUPnP, implementation not ready"));
		}
		catch ( CUPnPImpl::UPnPError& ) {}
		catch ( CException* e ) { e->Delete(); }
	}
	else
		ASSERT( false );
}

//////////////////////////////////////////////////////////////////
// Windows 7 GUI goodies

#ifdef HAVE_WIN7_SDK_H
// update thumbbarbutton structs and add/update the GUI thumbbar
void CemuleDlg::UpdateThumbBarButtons(bool initialAddToDlg) {
	
	if (!m_pTaskbarList)
		return;
	
	THUMBBUTTONMASK dwMask = THB_ICON | THB_FLAGS;
	for (size_t i=TBB_FIRST; i<=TBB_LAST;i++) {
		m_thbButtons[i].dwMask = dwMask;
		m_thbButtons[i].iId = i;
		m_thbButtons[i].iBitmap = 0;
		m_thbButtons[i].dwFlags = THBF_DISMISSONCLICK;
		CString tooltip;

		switch(i) {
			case TBB_CONNECT:
			{
				if (theApp.IsConnected() == false)
				{
					m_thbButtons[i].hIcon   =  theApp.LoadIcon(_T("CONNECT"), 16, 16);
					tooltip = GetResString(IDS_MAIN_BTN_CONNECT);
				}
				else
				{
					m_thbButtons[i].hIcon   = theApp.LoadIcon(_T("DISCONNECT"), 16, 16);
					tooltip = GetResString(IDS_MAIN_BTN_DISCONNECT);
				}
				break;
			}
			case TBB_UNTHROTTLE:
			{
				m_thbButtons[i].hIcon   =  theApp.LoadIcon(_T("SPEEDMAX"), 16, 16);
				tooltip = GetResString(IDS_PW_UA);
				break;
			}
			case TBB_PREFERENCES:
				m_thbButtons[i].hIcon   =  theApp.LoadIcon(_T("PREFERENCES"), 16, 16);
				tooltip = GetResString(IDS_EM_PREFS);
				break;
//>>> WiZaRd::Additional Thumbbuttons
			case TBB_OPENINC:
				m_thbButtons[i].hIcon   =  theApp.LoadIcon(L"FOLDERS", 16, 16);
				tooltip = GetResString(IDS_OPENINC);
				break;
//<<< WiZaRd::Additional Thumbbuttons
		}
		// set tooltips in widechar
		if (!tooltip.IsEmpty()) {
			tooltip.Remove('&');
			wcscpy(m_thbButtons[i].szTip,tooltip);
			m_thbButtons[i].dwMask |= THB_TOOLTIP;
		}
	}

	if (initialAddToDlg)
		m_pTaskbarList->ThumbBarAddButtons(m_hWnd, ARRAYSIZE(m_thbButtons), m_thbButtons);
	else
		m_pTaskbarList->ThumbBarUpdateButtons(m_hWnd, ARRAYSIZE(m_thbButtons), m_thbButtons);

	// clean up icons, they were copied in the previous call
	for (size_t i=TBB_FIRST; i<=TBB_LAST;i++) {
		DestroyIcon(m_thbButtons[i].hIcon);
	}
}

// Handle pressed thumbbar button
void CemuleDlg::OnTBBPressed(UINT id)
{
	switch (id) {
		case TBB_CONNECT:
			if(theApp.IsConnected() == false)
				AutoConnect();
			else
				CloseConnection();
			break;
		case TBB_UNTHROTTLE:
			QuickSpeedOther();
			break;
		case TBB_PREFERENCES:
			ShowPreferences();
			break;
//>>> WiZaRd::Additional Thumbbuttons
		case TBB_OPENINC:
			ShellExecute(NULL, L"open", thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR), NULL, NULL, SW_SHOW); 
			break;
//<<< WiZaRd::Additional Thumbbuttons
	}
}

// When Windows tells us, the taskbarbutton was created, it is safe to initialize our taskbar stuff
LRESULT CemuleDlg::OnTaskbarBtnCreated ( WPARAM , LPARAM  )
{
	// Sanity check that the OS is Win 7 or later
	if (thePrefs.GetWindowsVersion() >= _WINVER_7_ && IsRunning())
	{
		if (m_pTaskbarList)
			m_pTaskbarList.Release();
		
		if (m_pTaskbarList.CoCreateInstance ( CLSID_TaskbarList ) == S_OK)
		{
			m_pTaskbarList->SetProgressState ( m_hWnd, TBPF_NOPROGRESS );
			
			m_currentTBP_state = TBPF_NOPROGRESS;
			m_prevProgress=0;
			m_ovlIcon = NULL;
			
			UpdateThumbBarButtons(true);
			UpdateStatusBarProgress();
		}
		else
			ASSERT( false );
	}
	return 0;
}

// Updates global progress and /down state overlayicon
// Overlayicon looks rather annoying than useful, so its disabled by default for the common user and can be enabled by ini setting only (Ornis)
void CemuleDlg::EnableTaskbarGoodies(bool enable)
{
	if (m_pTaskbarList) {
		m_pTaskbarList->SetOverlayIcon ( m_hWnd, NULL, _T("") );
		if (!enable) {
			m_pTaskbarList->SetProgressState ( m_hWnd, TBPF_NOPROGRESS );
			m_currentTBP_state=TBPF_NOPROGRESS;
			m_prevProgress=0;
			m_ovlIcon = NULL;
		}
		else
			UpdateStatusBarProgress();
	}
}
void CemuleDlg::UpdateStatusBarProgress()
{
	if (m_pTaskbarList && thePrefs.IsWin7TaskbarGoodiesEnabled()) 
	{
		// calc global progress & status
		float globalDone = theStats.m_fGlobalDone;
		float globalSize = theStats.m_fGlobalSize;
		float overallProgress = globalSize?(globalDone/globalSize):0;

		TBPFLAG			new_state=m_currentTBP_state;

		if (globalSize==0) {
			// if there is no download, disable progress
			if (m_currentTBP_state!=TBPF_NOPROGRESS) {
				m_currentTBP_state=TBPF_NOPROGRESS;
				m_pTaskbarList->SetProgressState ( m_hWnd, TBPF_NOPROGRESS );
			}
		} else {

			new_state=TBPF_PAUSED;

			if (theStats.m_dwOverallStatus & STATE_DOWNLOADING) // smth downloading
				new_state=TBPF_NORMAL;
			
			if (theStats.m_dwOverallStatus & STATE_ERROROUS) // smth error
				new_state=TBPF_ERROR;

			if (new_state!=m_currentTBP_state) {
				m_pTaskbarList->SetProgressState ( m_hWnd, new_state );
				m_currentTBP_state=new_state;
			}

			if (overallProgress != m_prevProgress) {
				m_pTaskbarList->SetProgressValue(m_hWnd,(ULONGLONG)(overallProgress*100) ,100);
				m_prevProgress=overallProgress;
			}

		}
		// overlay up/down-speed
		/*if (thePrefs.IsShowUpDownIconInTaskbar()) 
		{
			bool bUp   = theApp.emuledlg->transferwnd->uploadlistctrl.GetItemCount() >0;
			bool bDown = theStats.m_dwOverallStatus & STATE_DOWNLOADING;

			HICON newicon = NULL;
			if (bUp && bDown)
				newicon=transicons[3];
			else if (bUp)
				newicon=transicons[2];
			else if (bDown)
				newicon=transicons[1];
			else
				newicon = NULL;

			if (m_ovlIcon!=newicon) {
				m_ovlIcon=newicon;
				m_pTaskbarList->SetOverlayIcon ( m_hWnd, m_ovlIcon, _T("eMule Up/Down Indicator") );
			}
		}*/
	}
}
#endif

void CemuleDlg::SetTaskbarIconColor()
{
	bool bBrightTaskbarIconSpeed = false;
	bool bTransparent = false;
	COLORREF cr = RGB(0, 0, 0);
	if (thePrefs.IsRunningAeroGlassTheme())
	{
		HMODULE hDWMAPI = LoadLibrary(_T("dwmapi.dll"));
		if (hDWMAPI){
			HRESULT (WINAPI *pfnDwmGetColorizationColor)(DWORD*, BOOL*);
			(FARPROC&)pfnDwmGetColorizationColor = GetProcAddress(hDWMAPI, "DwmGetColorizationColor");
			DWORD dwGlassColor = 0;
			BOOL bOpaque;
			if (pfnDwmGetColorizationColor != NULL)
			{
				if (pfnDwmGetColorizationColor(&dwGlassColor, &bOpaque) == S_OK)
				{
					uint8 byAlpha = (uint8)(dwGlassColor >> 24);
					cr = 0xFFFFFF & dwGlassColor;
					if (byAlpha < 200 && bOpaque == FALSE)
					{
						// on transparent themes we can never figure out which excact color is shown (may we could in real time?)
						// but given that a color is blended against the background, it is a good guess that a bright speedbar will
						// be the best solution in most cases
						bTransparent = true;
					}							
				}
			}
			FreeLibrary(hDWMAPI);
		}
	}
	else
	{
		if (g_xpStyle.IsThemeActive() && g_xpStyle.IsAppThemed())
		{
			CWnd* ptmpWnd = new CWnd();
			VERIFY( ptmpWnd->Create(_T("STATIC"), _T("Tmp"), 0, CRect(0, 0, 10, 10), this, 1235) );
			VERIFY( g_xpStyle.SetWindowTheme(ptmpWnd->GetSafeHwnd(), L"TrayNotifyHoriz", NULL) == S_OK );
			HTHEME hTheme = g_xpStyle.OpenThemeData(ptmpWnd->GetSafeHwnd(), L"TrayNotify");
			if (hTheme != NULL)
			{

				if (!g_xpStyle.GetThemeColor(hTheme, TNP_BACKGROUND, 0, TMT_FILLCOLORHINT, &cr) == S_OK)
					ASSERT( false );
				g_xpStyle.CloseThemeData(hTheme);
			}
			else
				ASSERT( false );
			ptmpWnd->DestroyWindow();
			delete ptmpWnd;
		}
		else
		{
			DEBUG_ONLY(DebugLog(_T("Taskbar Notifier Color: GetSysColor() used")));
			cr = GetSysColor(COLOR_3DFACE);
		}
	}
	uint8 iRed = GetRValue(cr);
	uint8 iBlue = GetBValue(cr);
	uint8 iGreen = GetGValue(cr);
	uint16 iBrightness = (uint16)sqrt(((iRed * iRed * 0.241f) + (iGreen * iGreen * 0.691f) + (iBlue * iBlue * 0.068f)));
	ASSERT( iBrightness <= 255 );
	bBrightTaskbarIconSpeed = iBrightness < 132;
	DebugLog(_T("Taskbar Notifier Color: R:%u G:%u B:%u, Brightness: %u, Transparent: %s"), iRed, iGreen, iBlue, iBrightness, bTransparent ? _T("Yes") : _T("No"));
	thePrefs.SetStatsColor(11, (bBrightTaskbarIconSpeed || bTransparent)?RGB(255, 255, 255):RGB(0, 0, 0));
}

LRESULT CemuleDlg::OnPartHashedOK(WPARAM wParam,LPARAM lParam)
{
	//Xman
	// BEGIN SiRoB: Fix crash at shutdown
	if (theApp.m_app_state != APP_STATE_RUNNING || theApp.downloadqueue==NULL)
		return FALSE;
	// END SiRoB: Fix crash at shutdown
	CPartFile* pOwner = (CPartFile*)lParam;
	if (theApp.downloadqueue->IsPartFile(pOwner))	// could have been canceled
		pOwner->PartHashFinished(wParam, true, false);
	return 0;
}

LRESULT CemuleDlg::OnPartHashedOKNoAICH(WPARAM wParam,LPARAM lParam)
{
	//Xman
	// BEGIN SiRoB: Fix crash at shutdown
	if (theApp.m_app_state != APP_STATE_RUNNING || theApp.downloadqueue==NULL)
		return FALSE;
	// END SiRoB: Fix crash at shutdown
	CPartFile* pOwner = (CPartFile*)lParam;
	if (theApp.downloadqueue->IsPartFile(pOwner)){	// could have been canceled
		pOwner->PartHashFinished(wParam, false, false);
	}
	return 0;
}

LRESULT CemuleDlg::OnPartHashedCorrupt(WPARAM wParam,LPARAM lParam)
{
	//Xman
	// BEGIN SiRoB: Fix crash at shutdown
	if (theApp.m_app_state != APP_STATE_RUNNING || theApp.downloadqueue==NULL)
		return FALSE;
	// END SiRoB: Fix crash at shutdown
	CPartFile* pOwner = (CPartFile*)lParam;
	if (theApp.downloadqueue->IsPartFile(pOwner))	// could have been canceled
		pOwner->PartHashFinished(wParam, true, true);
	return 0;
}

LRESULT CemuleDlg::OnPartHashedCorruptNoAICH(WPARAM wParam,LPARAM lParam)
{
	//Xman
	// BEGIN SiRoB: Fix crash at shutdown
	if (theApp.m_app_state != APP_STATE_RUNNING || theApp.downloadqueue==NULL)
		return FALSE;
	// END SiRoB: Fix crash at shutdown
	CPartFile* pOwner = (CPartFile*)lParam;
	if (theApp.downloadqueue->IsPartFile(pOwner))	// could have been canceled
		pOwner->PartHashFinished(wParam, false, true);
	return 0;
}

LRESULT CemuleDlg::OnPartHashedOKAICHRecover(WPARAM wParam,LPARAM lParam)
{
	//Xman
	// BEGIN SiRoB: Fix crash at shutdown
	if (theApp.m_app_state != APP_STATE_RUNNING || theApp.downloadqueue==NULL)
		return FALSE;
	// END SiRoB: Fix crash at shutdown
	CPartFile* pOwner = (CPartFile*)lParam;
	if (theApp.downloadqueue->IsPartFile(pOwner))	// could have been canceled
		pOwner->PartHashFinishedAICHRecover(wParam, false);
	return 0;
}

LRESULT CemuleDlg::OnPartHashedCorruptAICHRecover(WPARAM wParam,LPARAM lParam)
{
	//Xman
	// BEGIN SiRoB: Fix crash at shutdown
	if (theApp.m_app_state != APP_STATE_RUNNING || theApp.downloadqueue==NULL)
		return FALSE;
	// END SiRoB: Fix crash at shutdown
	CPartFile* pOwner = (CPartFile*)lParam;
	if (theApp.downloadqueue->IsPartFile(pOwner))	// could have been canceled
		pOwner->PartHashFinishedAICHRecover(wParam, true);
	return 0;
}
// END SLUGFILLER: SafeHash

// BEGIN SiRoB: ReadBlockFromFileThread
LRESULT CemuleDlg::OnReadBlockFromFileDone(WPARAM wParam,LPARAM lParam)
{
	CUpDownClient* client = (CUpDownClient*) lParam;
	if (theApp.m_app_state == APP_STATE_RUNNING && theApp.uploadqueue && theApp.uploadqueue->IsDownloading(client))	// could have been canceled
	{
		client->SetReadBlockFromFileBuffer((byte*)wParam);
		client->CreateNextBlockPackage(); //complete the process
	}
	else if (wParam != -1 && wParam != -2 && wParam != NULL)
		delete[] (byte*)wParam;
	return 0;
}
// END SiRoB: ReadBlockFromFileThread
// BEGIN SiRoB: Flush Thread
LRESULT CemuleDlg::OnFlushDone(WPARAM /*wParam*/,LPARAM lParam)
{
	CPartFile* partfile = (CPartFile*) lParam;
	if (theApp.m_app_state == APP_STATE_RUNNING && theApp.downloadqueue!=NULL && theApp.downloadqueue->IsPartFile(partfile))	// could have been canceled
		partfile->FlushDone();
	return 0;
}
// END SiRoB: Flush Thread
//Xman process timer code via messages (Xanatos)
// Note: the timers does not crash on a exception so I use the messags to call the functions so when an error aprears it will be detected
afx_msg LRESULT CemuleDlg::DoTimer(WPARAM wParam, LPARAM /*lParam*/)
{
	if (!CemuleDlg::IsRunning())
		return 0;

	if(wParam == NULL)
		theApp.uploadqueue->UploadTimer();
	/* only used for uploadtimer
	else if(wParam == -1)
		theApp.emuledlg->StartupTimer();
	*/
	else
		ASSERT(0);
	return 0;
}

// ==> Invisible Mode [TPT/MoNKi] - Stulle
LRESULT CemuleDlg::OnHotKey(WPARAM wParam, LPARAM /*lParam*/)
{
	if(wParam == HOTKEY_INVISIBLEMODE_ID){
		b_HideApp = !b_HideApp;
		EnumWindows(AskEmulesForInvisibleMode, b_HideApp?INVMODE_HIDEWINDOW:INVMODE_RESTOREWINDOW);
	}
	return 0;
}

void CemuleDlg::ToggleHide()
{
	b_HideApp = true;
	TrayHide();
	if(b_WindowWasVisible = IsWindowVisible())
		ShowWindow(SW_HIDE);
	if(preferenceswnd->IsDialogOpen())
		preferenceswnd->ShowWindow(SW_HIDE);
	if(theApp.m_pSpeedGraphWnd && theApp.m_pSpeedGraphWnd->IsWindowVisible()) // X: [SGW] - [SpeedGraphWnd]
		theApp.m_pSpeedGraphWnd->ShowWindow(SW_HIDE);
}

void CemuleDlg::ToggleShow()
{
	b_HideApp = false;
	TrayShow();
	if(b_WindowWasVisible){
		RestoreWindow();
		if (m_bStartMinimized){
			m_bStartMinimized = false;
			UpdateWindow();
		}
	}
	if(preferenceswnd->IsDialogOpen())
		preferenceswnd->ShowWindow(SW_SHOW);
	if(thePrefs.showSpeedGraph && theApp.m_pSpeedGraphWnd) // X: [SGW] - [SpeedGraphWnd]
		theApp.m_pSpeedGraphWnd->ShowWindow(SW_SHOW);
}

BOOL CemuleDlg::RegisterInvisibleHotKey()
{
	if(m_hWnd && IsRunning()){
		return RegisterHotKey( this->m_hWnd, HOTKEY_INVISIBLEMODE_ID ,
						   thePrefs.m_iInvisibleModeHotKeyModifier,
						   thePrefs.m_cInvisibleModeHotKey)!=0;
	}
	return false;
}

BOOL CemuleDlg::UnRegisterInvisibleHotKey()
{
	if(m_hWnd){
		bool res = !(UnregisterHotKey(this->m_hWnd, HOTKEY_INVISIBLEMODE_ID));

		// Allows "invisible mode" on multiple instances of eMule
		// Only one app (eMule) can register the hotkey, if we unregister, we need
		// to register the hotkey in other emule.
		EnumWindows(AskEmulesForInvisibleMode, INVMODE_REGISTERHOTKEY);
		return res;
	}
	return false;
}

// Allows "invisible mode" on multiple instances of eMule
// LOWORD(WPARAM) -> HotKey KeyModifier
// HIWORD(WPARAM) -> HotKey VirtualKey
// LPARAM		  -> int:	INVMODE_RESTOREWINDOW	-> Restores the window
//							INVMODE_REGISTERHOTKEY	-> Registers the hotkey
LRESULT CemuleDlg::OnRestoreWindowInvisibleMode(WPARAM wParam, LPARAM lParam)
{
	if (thePrefs.m_bInvisibleMode &&
		LOWORD(wParam) == thePrefs.m_iInvisibleModeHotKeyModifier &&
		(char)HIWORD(wParam) == thePrefs.m_cInvisibleModeHotKey) {
			switch(lParam){
				case INVMODE_RESTOREWINDOW:
					ToggleShow();
					break;
				case INVMODE_REGISTERHOTKEY:
					RegisterInvisibleHotKey();
					break;
				case INVMODE_HIDEWINDOW:
					ToggleHide();
					break;
			}
			return UWM_RESTORE_WINDOW_IM;
	}
	return false;
} 

// Allows "invisible mode" on multiple instances of eMule
BOOL CALLBACK CemuleDlg::AskEmulesForInvisibleMode(HWND hWnd, LPARAM lParam){
	DWORD_PTR dwMsgResult;
	WPARAM msgwParam = MAKEWPARAM(thePrefs.m_iInvisibleModeHotKeyModifier,
				thePrefs.m_cInvisibleModeHotKey);

	return (BOOL)::SendMessageTimeout(hWnd,UWM_RESTORE_WINDOW_IM, msgwParam, lParam,
				SMTO_BLOCK |SMTO_ABORTIFHUNG,10000,&dwMsgResult);
} 
// <== Invisible Mode [TPT/MoNKi] - Stulle

LRESULT CemuleDlg::OnTrayNotify(WPARAM wParam, LPARAM lParam)
{
	UINT uID = (UINT)wParam;
	if (uID != 1)
		return 0;

	if(!IsRunning())
		return 0;

	UINT uMsg = (UINT)lParam;
	switch (uMsg)
	{
		case WM_LBUTTONUP:
			if(!IsWindowVisible()||IsIconic())
				RestoreWindow();
			else
				MinimizeWindow();
			break;

		case WM_RBUTTONUP:
		case WM_CONTEXTMENU:{
			CPoint pt;
			GetCursorPos(&pt);
			ShowTrayPopup(pt);
			break;
		}

		case NIN_BALLOONUSERCLICK:
			if(!IsWindowVisible()||IsIconic())
				RestoreWindow();
		case NIN_BALLOONTIMEOUT:
			KillBallonTimer();
			TraySetBalloonToolTip(_T(""), _T(""));
			break;
	} 
	return 1;
}

//morph4u shutdown +
void CemuleDlg::SaveSettings (bool _shutdown) {
	
	theApp.knownfiles->Save();
	theApp.sharedfiles->Save();
	serverwnd->SaveAllSettings();
    
	thePrefs.Save();
	if (_shutdown) {
		thePerfLog.Shutdown();
	}
}
//morph4u shutdown -

void CemuleDlg::UpdateNodesDatFromURL(CString strURL){
	CString strTempFilename;
	strTempFilename.Format(_T("%stemp-%d-nodes.dat"), thePrefs.GetMuleDirectory(EMULE_CONFIGDIR), ::GetTickCount());

	// try to download nodes.dat
	Log(GetResString(IDS_DOWNLOADING_NODESDAT_FROM), strURL);
	CHttpDownloadDlg dlgDownload;
	dlgDownload.m_strTitle = GetResString(IDS_DOWNLOADING_NODESDAT);
	dlgDownload.m_sURLToDownload = strURL;
	dlgDownload.m_sFileToDownloadInto = strTempFilename;
	if (dlgDownload.DoModal() != IDOK) {
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_FAILEDDOWNLOADNODES), strURL);
		return;
	}

	if (!Kademlia::CKademlia::IsRunning()){
		Kademlia::CKademlia::Start();
		ShowConnectionState();
	}
	Kademlia::CKademlia::GetRoutingZone()->ReadFile(strTempFilename);
	(void)_tremove(strTempFilename);		
}

//KadCount +
void CemuleDlg::ContactAdd(const Kademlia::CContact* contact)
{
	contactCount++;
}

void CemuleDlg::ContactRem(const Kademlia::CContact* contact)
{
	contactCount--;
}
//KadCount -
// X-Ray :: AutoRestartIfNecessary :: Start
void CemuleDlg::RestartMuleApp()
{ 
	theApp.m_bRestartApp = true;
	PostMessage(WM_CLOSE);
}
// X-Ray :: AutoRestartIfNecessary :: End