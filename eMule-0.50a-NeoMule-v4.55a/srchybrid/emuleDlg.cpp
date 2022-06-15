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
#include <afxinet.h>
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
#include <HtmlHelp.h>
#include <share.h>
#include "emule.h"
#include "emuleDlg.h"
#include "ServerWnd.h"
#include "KademliaWnd.h"
#include "TransferWnd.h"
#include "SearchResultsWnd.h"
#include "SearchDlg.h"
#include "SharedFilesWnd.h"
#include "ChatWnd.h"
#include "IrcWnd.h"
#include "StatisticsDlg.h"
#include "CreditsDlg.h"
#include "Neo/CP/PreferencesDlg.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --
#include "Sockets.h"
#include "KnownFileList.h"
#include "ServerList.h"
#include "Opcodes.h"
#include "SharedFileList.h"
#include "ED2KLink.h"
//#include "SplashScreen.h" // NEO: SS - [SplashScreen] <-- Xanatos --
#include "PartFileConvert.h"
#include "EnBitmap.h"
#include "Wizard.h"
#include "Exceptions.h"
#include "SearchList.h"
#include "HTRichEditCtrl.h"
#include "FrameGrabThread.h"
#include "kademlia/kademlia/kademlia.h"
#include "kademlia/kademlia/SearchManager.h"
#include "kademlia/routing/RoutingZone.h"
#include "kademlia/routing/contact.h"
#include "kademlia/kademlia/prefs.h"
#include "KadSearchListCtrl.h"
#include "KadContactListCtrl.h"
#include "PerfLog.h"
#include "DropTarget.h"
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
#include "Neo/BC/BandwidthControl.h"
#else
#include "LastCommonRouteFinder.h"
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
#include "WebServer.h"
#include "MMServer.h"
#include "DownloadQueue.h"
#include "ClientUDPSocket.h"
#include "UploadQueue.h"
#include "ClientList.h"
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
#include "Neo/BC/DownloadBandwidthThrottler.h"
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
#include "Neo/BC/UploadBandwidthThrottler.h"
#else
#include "UploadBandwidthThrottler.h"
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
#include "FriendList.h"
#include "IPFilter.h"
#include "Statistics.h"
#include "MuleToolbarCtrl.h"
#include "TaskbarNotifier.h"
#include "MuleStatusbarCtrl.h"
#include "ListenSocket.h"
#include "Server.h"
#include "PartFile.h"
#include "Scheduler.h"
#include "ClientCredits.h"
#include "MenuCmds.h"
#include "MuleSystrayDlg.h"
#include "IPFilterDlg.h"
#include "WebServices.h"
#include "DirectDownloadDlg.h"
#include "PeerCacheFinder.h"
#include "Statistics.h"
#include "FirewallOpener.h"
#include "StringConversion.h"
#include "aichsyncthread.h"
#include "Log.h"
#include "MiniMule.h"
#include "UserMsgs.h"
#include "TextToSpeech.h"
#include "Collection.h"
#include "CollectionViewDialog.h"
#include "VisualStylesXP.h"
#include "UPnPImpl.h"
#include "UPnPImplWrapper.h"
#include "Neo/NeoVersion.h" // NEO: NV - [NeoVersion] <-- Xanatos --
#include "Neo/NeoPreferences.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
#include "Neo/LanCast/Lancast.h"
#endif //LANCAST // NEO: NLC END <-- Xanatos --
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
#include "Neo/VooDoo/voodoo.h"
#include "Neo/VooDoo/VoodooListDlg.h"
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
#include "Neo/Functions.h" // NEO: MOD <-- Xanatos --
#ifdef NATTUNNELING // NEO: UTCP - [UserModeTCP] -- Xanatos -->
#include "Neo/NatT/NatManager.h"
#endif //NATTUNNELING // NEO: UTCP END <-- Xanatos --
#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase] -- Xanatos -->
#include "Neo/Sources/SourceList.h"
#include "Neo/Sources/SourceListDlg.h"
#endif // NEO_CD // NEO: NCD END <-- Xanatos --
#include "inputbox.h" // NEO: TPP - [TrayPasswordProtection] <-- Xanatos --
#include "Neo/SysInfo/SystemInfo.h" // NEO: SI - [SysInfo] <-- Xanatos --
#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->
#include "Neo/Argos.h"
#endif // ARGOS // NEO: NA END <-- Xanatos --
#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
#include "Neo\GUI\IP2Country.h" 
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --
#include "Neo\GUI\XPMenu\MenuXP.h" // NEO: NMX - [NeoMenuXP] <-- Xanatos --

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define	SYS_TRAY_ICON_COOKIE_FORCE_UPDATE	(UINT)-1

BOOL (WINAPI *_TransparentBlt)(HDC, int, int, int, int, HDC, int, int, int, int, UINT) = NULL;
const static UINT UWM_ARE_YOU_EMULE = RegisterWindowMessage(EMULE_GUID);
const static UINT UWM_RESTORE_WINDOW_IM=RegisterWindowMessage(EMULE_GUID_INVMODE); // NEO: IM - [InvisibelMode] <-- Xanatos --
UINT _uMainThreadId = 0;


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
	ON_MESSAGE(WM_POWERBROADCAST, OnPowerBroadcast)
	ON_WM_MEASUREITEM() // NEO: NMX - [NeoMenuXP] <-- Xanatos --
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	ON_NOTIFY_EX_RANGE(UDM_TOOLTIP_DISPLAY, 0, 0xFFFF, OnToolTipNotify)
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

	ON_MESSAGE(WM_HOTKEY, OnHotKey) // NEO: IM - [InvisibelMode] <-- Xanatos --

	///////////////////////////////////////////////////////////////////////////
	// WM_COMMAND messages
	//
	ON_COMMAND(MP_CONNECT, StartConnection)
	ON_COMMAND(MP_DISCONNECT, CloseConnection)
	ON_COMMAND(MP_EXIT, OnClose)
	ON_COMMAND(MP_RESTORE, RestoreWindow)
	// quick-speed changer -- 
	ON_COMMAND_RANGE(MP_QS_U10, MP_QS_UP10, QuickSpeedUpload)
	ON_COMMAND_RANGE(MP_QS_D10, MP_QS_DC, QuickSpeedDownload)
	//--- quickspeed - paralize all ---
	ON_COMMAND_RANGE(MP_QS_PA, MP_QS_UA, QuickSpeedOther)
	// quick-speed changer -- based on xrmb	
	ON_NOTIFY_EX_RANGE(RBN_CHEVRONPUSHED, 0, 0xFFFF, OnChevronPushed)

	ON_REGISTERED_MESSAGE(UWM_ARE_YOU_EMULE, OnAreYouEmule)
	ON_BN_CLICKED(IDC_HOTMENU, OnBnClickedHotmenu)

	// Allows "invisible mode" on multiple instances of eMule
	ON_REGISTERED_MESSAGE(UWM_RESTORE_WINDOW_IM, OnRestoreWindowInvisibleMode) // NEO: IM - [InvisibelMode] <-- Xanatos --

	///////////////////////////////////////////////////////////////////////////
	// WM_USER messages
	//
	ON_MESSAGE(UM_TASKBARNOTIFIERCLICKED, OnTaskbarNotifierClicked)
	ON_MESSAGE(UM_CLOSE_MINIMULE, OnCloseMiniMule)
	
	// Webserver messages
	ON_MESSAGE(WEB_GUI_INTERACTION, OnWebGUIInteraction)
	ON_MESSAGE(WEB_CLEAR_COMPLETED, OnWebServerClearCompleted)
	ON_MESSAGE(WEB_FILE_RENAME, OnWebServerFileRename)
	ON_MESSAGE(WEB_ADDDOWNLOADS, OnWebAddDownloads)
	ON_MESSAGE(WEB_CATPRIO, OnWebSetCatPrio)
	ON_MESSAGE(WEB_ADDREMOVEFRIEND, OnAddRemoveFriend)

	// Version Check DNS
	ON_MESSAGE(UM_VERSIONCHECK_RESPONSE, OnVersionCheckResponse)
	ON_MESSAGE(UM_NEO_VERSIONCHECK_RESPONSE, OnNeoVersionCheckResponse) // NEO: NVC - [NeoVersionCheck] <-- Xanatos --

	// PeerCache DNS
	ON_MESSAGE(UM_PEERCHACHE_RESPONSE, OnPeerCacheResponse)

	// UPnP
	ON_MESSAGE(UM_UPNP_RESULT, OnUPnPResult)

	///////////////////////////////////////////////////////////////////////////
	// WM_APP messages
	//
	ON_MESSAGE(TM_DOTIMER, DoTimer) // NEO: ND - [NeoDebug] <-- Xanatos --
	ON_MESSAGE(TM_FINISHEDHASHING, OnFileHashed)
	ON_MESSAGE(TM_FILEOPPROGRESS, OnFileOpProgress)
	ON_MESSAGE(TM_HASHFAILED, OnHashFailed)
	ON_MESSAGE(TM_FLUSHDONE, OnFlushDone) //MORPH - Added by SiRoB, Flush Thread // NEO: FFT - [FileFlushThread] <-- Xanatos --
	ON_MESSAGE(TM_PARTHASHED, OnPartHashed) // SLUGFILLER: SafeHash // NEO: SSH - [SlugFillerSafeHash] <-- Xanatos --
	ON_MESSAGE(TM_BLOCKHASHED, OnBlockHashed)  // NEO: SafeHash // NEO: SCV - [SubChunkVerification] <-- Xanatos --
	ON_MESSAGE(TM_READBLOCKFROMFILEDONE, OnReadBlockFromFileDone) // NEO: RBT - [ReadBlockThread] <-- Xanatos --
	ON_MESSAGE(TM_IMPORTPART, OnImportPart) // NEO: PIX - [PartImportExport] <-- Xanatos --
	ON_MESSAGE(TM_FILEIMPORTED, OnFileImported) // NEO: PIX - [PartImportExport] <-- Xanatos --
	ON_MESSAGE(TM_FILEMOVED, OnFileMoved) // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
	ON_MESSAGE(TM_FRAMEGRABFINISHED, OnFrameGrabFinished)
	ON_MESSAGE(TM_FILEALLOCEXC, OnFileAllocExc)
	ON_MESSAGE(TM_FILECOMPLETED, OnFileCompleted)
	ON_MESSAGE(TM_CONSOLETHREADEVENT, OnConsoleThreadEvent)
#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->
	ON_MESSAGE(TM_ARGOS_RESULT, OnArgosResult)
#endif // ARGOS // NEO: NA END <-- Xanatos --
END_MESSAGE_MAP()

CemuleDlg::CemuleDlg(CWnd* pParent /*=NULL*/)
	: CTrayDialog(CemuleDlg::IDD, pParent)
{
	_uMainThreadId = GetCurrentThreadId();
	preferenceswnd = new CPreferencesDlg;
	serverwnd = new CServerWnd;
	kademliawnd = new CKademliaWnd;
	transferwnd = new CTransferWnd;
	sharedfileswnd = new CSharedFilesWnd;
	searchwnd = new CSearchDlg;
	chatwnd = new CChatWnd;
	ircwnd = new CIrcWnd;
	statisticswnd = new CStatisticsDlg;
	toolbar = new CMuleToolbarCtrl;
	statusbar = new CMuleStatusBarCtrl;
	m_wndTaskbarNotifier = new CTaskbarNotifier;

	m_hIcon = NULL;
	theApp.m_app_state = APP_STATE_RUNNING;
	ready = false; 
	m_bStartMinimizedChecked = false;
	m_bStartMinimized = false;
	memset(&m_wpFirstRestore, 0, sizeof m_wpFirstRestore);
	m_uUpDatarate = 0;
	m_uDownDatarate = 0;
	status = 0;
	activewnd = NULL;
	for (int i = 0; i < _countof(connicons); i++)
		connicons[i] = NULL;
	transicons[0] = NULL;
	transicons[1] = NULL;
	transicons[2] = NULL;
	transicons[3] = NULL;
	imicons[0] = NULL;
	imicons[1] = NULL;
	imicons[2] = NULL;
	m_iMsgIcon = 0;
	m_iMsgBlinkState = false;
	m_icoSysTrayConnected = NULL;
	m_icoSysTrayDisconnected = NULL;
	m_icoSysTrayLowID = NULL;
	usericon = NULL;
	m_icoSysTrayCurrent = NULL;
	m_hTimer = 0;
	notifierenabled = false;
	m_pDropTarget = new CMainFrameDropTarget;
	// NEO: SS - [SplashScreen] -- Xanatos --
	/*m_pSplashWnd = NULL;
	m_dwSplashTime = (DWORD)-1;*/
	m_pSystrayDlg = NULL;
	m_pMiniMule = NULL;
	m_uLastSysTrayIconCookie = SYS_TRAY_ICON_COOKIE_FORCE_UPDATE;
	m_hUPnPTimeOutTimer = 0;
	m_bConnectRequestDelayedForUPnP = false;
	m_bEd2kSuspendDisconnect = false;
	m_bKadSuspendDisconnect = false;
	m_TrayLocked = false; // NEO: TPP - [TrayPasswordProtection] <-- Xanatos --
}

CemuleDlg::~CemuleDlg()
{
	CloseTTS();
	DestroyMiniMule();
	if (m_icoSysTrayCurrent) VERIFY( DestroyIcon(m_icoSysTrayCurrent) );
	if (m_hIcon) VERIFY( ::DestroyIcon(m_hIcon) );
	for (int i = 0; i < _countof(connicons); i++){
		if (connicons[i]) VERIFY( ::DestroyIcon(connicons[i]) );
	}
	if (transicons[0]) VERIFY( ::DestroyIcon(transicons[0]) );
	if (transicons[1]) VERIFY( ::DestroyIcon(transicons[1]) );
	if (transicons[2]) VERIFY( ::DestroyIcon(transicons[2]) );
	if (transicons[3]) VERIFY( ::DestroyIcon(transicons[3]) );
	if (imicons[0]) VERIFY( ::DestroyIcon(imicons[0]) );
	if (imicons[1]) VERIFY( ::DestroyIcon(imicons[1]) );
	if (imicons[2]) VERIFY( ::DestroyIcon(imicons[2]) );
	if (m_icoSysTrayConnected) VERIFY( ::DestroyIcon(m_icoSysTrayConnected) );
	if (m_icoSysTrayDisconnected) VERIFY( ::DestroyIcon(m_icoSysTrayDisconnected) );
	if (m_icoSysTrayLowID) VERIFY( ::DestroyIcon(m_icoSysTrayLowID) );
	if (usericon) VERIFY( ::DestroyIcon(usericon) );

	// already destroyed by windows?
	//VERIFY( m_menuUploadCtrl.DestroyMenu() );
	//VERIFY( m_menuDownloadCtrl.DestroyMenu() );
	//VERIFY( m_SysMenuOptions.DestroyMenu() );

	delete preferenceswnd;
	delete serverwnd;
	delete kademliawnd;
	delete transferwnd;
	delete sharedfileswnd;
	delete chatwnd;
	delete ircwnd;
	delete statisticswnd;
	delete toolbar;
	delete statusbar;
	delete m_wndTaskbarNotifier;
	delete m_pDropTarget;
}

void CemuleDlg::DoDataExchange(CDataExchange* pDX)
{
	CTrayDialog::DoDataExchange(pDX);
}

LRESULT CemuleDlg::OnAreYouEmule(WPARAM, LPARAM)
{
	return UWM_ARE_YOU_EMULE;
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
	m_bStartMinimized = thePrefs.GetStartMinimized();
	if (!m_bStartMinimized)
		m_bStartMinimized = theApp.DidWeAutoStart();

	// temporary disable the 'startup minimized' option, otherwise no window will be shown at all
	if (thePrefs.IsFirstStart())
		m_bStartMinimized = false;

	// NEO: SS - [SplashScreen] -- Xanatos --
	// show splashscreen as early as possible to "enTatrain" user while starting emule up
	/*if (thePrefs.UseSplashScreen() && !m_bStartMinimized)
		ShowSplash();*/

	// Create global GUI objects
	theApp.CreateAllFonts();
	theApp.CreateBackwardDiagonalBrush();
	m_wndTaskbarNotifier->SetTextDefaultFont();
	CTrayDialog::OnInitDialog();
	InitWindowStyles(this);
	CreateToolbarCmdIconMap();

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL){
		pSysMenu->AppendMenu(MF_SEPARATOR);

		ASSERT( (MP_ABOUTBOX & 0xFFF0) == MP_ABOUTBOX && MP_ABOUTBOX < 0xF000);
		pSysMenu->AppendMenu(MF_STRING, MP_ABOUTBOX, GetResString(IDS_ABOUTBOX));

		// NEO: NVC - [NeoVersionCheck] -- Xanatos -->
		ASSERT( (MP_NEOVERSIONCHECK & 0xFFF0) == MP_NEOVERSIONCHECK && MP_NEOVERSIONCHECK < 0xF000);
		pSysMenu->AppendMenu(MF_STRING, MP_NEOVERSIONCHECK, GetResString(IDS_X_VERSIONCHECK));
		// NEO: NVC END <-- Xanatos --

		ASSERT( (MP_VERSIONCHECK & 0xFFF0) == MP_VERSIONCHECK && MP_VERSIONCHECK < 0xF000);
		pSysMenu->AppendMenu(MF_STRING, MP_VERSIONCHECK, GetResString(IDS_VERSIONCHECK));

		// remaining system menu entries are created later...
	}

	CWnd* pwndToolbarX = toolbar;
	if (toolbar->Create(WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), this, IDC_TOOLBAR))
	{
		toolbar->Init();
		if (thePrefs.GetUseReBarToolbar())
		{
		    if (m_ctlMainTopReBar.Create(WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
									     RBS_BANDBORDERS | RBS_AUTOSIZE | CCS_NODIVIDER, 
									     CRect(0, 0, 0, 0), this, AFX_IDW_REBAR))
		    {
			    CSize sizeBar;
			    VERIFY( toolbar->GetMaxSize(&sizeBar) );
			    REBARBANDINFO rbbi = {0};
			    rbbi.cbSize = sizeof(rbbi);
				rbbi.fMask = RBBIM_STYLE | RBBIM_SIZE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_IDEALSIZE | RBBIM_ID;
			    rbbi.fStyle = RBBS_NOGRIPPER | RBBS_BREAK | RBBS_USECHEVRON;
			    rbbi.hwndChild = toolbar->m_hWnd;
			    rbbi.cxMinChild = sizeBar.cy;
			    rbbi.cyMinChild = sizeBar.cy;
			    rbbi.cxIdeal = sizeBar.cx;
			    rbbi.cx = rbbi.cxIdeal;
				rbbi.wID = 0;
			    VERIFY( m_ctlMainTopReBar.InsertBand((UINT)-1, &rbbi) );
				toolbar->SaveCurHeight();
		    	toolbar->UpdateBackground();
    
			    pwndToolbarX = &m_ctlMainTopReBar;
		    }
		}
	}

	// NEO: PSM - [PlusSpeedMeter] -- Xanatos -->
	toolbar->SetSpeedMeterRange(thePrefs.GetMaxGraphUploadRate(true),thePrefs.GetMaxGraphDownloadRate());
	toolbar->ShowSpeedMeter(NeoPrefs.UsePlusSpeedMeter());
	// NEO: PSM END <-- Xanatos --

	UpdateTrayBarsColors(); // NEO: NSTI - [NewSystemTrayIcon] <-- Xanatos --

	// NEO: STI - [StaticTray] -- Xanatos -->
	if(NeoPrefs.UseStaticTrayIcon()) 
		TrayShow(true);
	// NEO: MOD END <-- Xanatos --

	// set title
	//SetWindowText(_T("eMule v") + theApp.m_strCurVersionLong);
	SetWindowText(theApp.GetAppTitle(true)); // NEO: NV - [NeoVersion] <-- Xanatos --

	// Init taskbar notifier
	m_wndTaskbarNotifier->Create(this);
	LoadNotifier(thePrefs.GetNotifierConfiguration());

	// set statusbar
	// the statusbar control is created as a custom control in the dialog resource,
	// this solves font and sizing problems when using large system fonts
	statusbar->SubclassWindow(GetDlgItem(IDC_STATUSBAR)->m_hWnd);
	statusbar->EnableToolTips(true);
	SetStatusBarPartsSize();

	// create main window dialog pages
	DialogCreateIndirect(serverwnd, IDD_SERVER);
	DialogCreateIndirect(sharedfileswnd, IDD_FILES);
	searchwnd->Create(this); // can not use 'DialogCreateIndirect' for the SearchWnd, grrr..
	DialogCreateIndirect(chatwnd, IDD_CHAT);
	DialogCreateIndirect(transferwnd, IDD_TRANSFER);
	DialogCreateIndirect(statisticswnd, IDD_STATISTICS);
	DialogCreateIndirect(kademliawnd, IDD_KADEMLIAWND);
	DialogCreateIndirect(ircwnd, IDD_IRC);

	// with the top rebar control, some XP themes look better with some additional lite borders.. some not..
	//serverwnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//sharedfileswnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//searchwnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//chatwnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//transferwnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//statisticswnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//kademliawnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//ircwnd->ModifyStyleEx(0, WS_EX_STATICEDGE);

	// optional: restore last used main window dialog
	if (thePrefs.GetRestoreLastMainWndDlg()){
		switch (thePrefs.GetLastMainWndDlgID()){
		case IDD_SERVER:
			SetActiveDialog(serverwnd);
			break;
		case IDD_FILES:
			SetActiveDialog(sharedfileswnd);
			break;
		case IDD_SEARCH:
			SetActiveDialog(searchwnd);
			break;
		case IDD_CHAT:
			SetActiveDialog(chatwnd);
			break;
		case IDD_TRANSFER:
			SetActiveDialog(transferwnd);
			break;
		case IDD_STATISTICS:
			SetActiveDialog(statisticswnd);
			break;
		case IDD_KADEMLIAWND:
			SetActiveDialog(kademliawnd);
			break;
		case IDD_IRC:
			SetActiveDialog(ircwnd);
			break;
		}
	}

	// if still no active window, activate server window
	if (activewnd == NULL)
		SetActiveDialog(kademliawnd); // NEO: MOD <-- Xanatos --
		//SetActiveDialog(serverwnd);

	SetAllIcons();
	Localize();

	// set updateintervall of graphic rate display (in seconds)
	//ShowConnectionState(false);

	// adjust all main window sizes for toolbar height and maximize the child windows
	CRect rcClient, rcToolbar, rcStatusbar;
	GetClientRect(&rcClient);
	pwndToolbarX->GetWindowRect(&rcToolbar);
	statusbar->GetWindowRect(&rcStatusbar);
	rcClient.top += rcToolbar.Height();
	rcClient.bottom -= rcStatusbar.Height();

	CWnd* apWnds[] =
	{
		serverwnd,
		kademliawnd,
		transferwnd,
		sharedfileswnd,
		searchwnd,
		chatwnd,
		ircwnd,
		statisticswnd
	};
	for (int i = 0; i < _countof(apWnds); i++)
		apWnds[i]->SetWindowPos(NULL, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER);

	// anchors
	AddAnchor(*serverwnd,		TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*kademliawnd,		TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*transferwnd,		TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*sharedfileswnd,	TOP_LEFT, BOTTOM_RIGHT);
    AddAnchor(*searchwnd,		TOP_LEFT, BOTTOM_RIGHT);
    AddAnchor(*chatwnd,			TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*ircwnd,			TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*statisticswnd,	TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*pwndToolbarX,	TOP_LEFT, TOP_RIGHT);
	AddAnchor(*statusbar,		BOTTOM_LEFT, BOTTOM_RIGHT);

	statisticswnd->ShowInterval();

	// tray icon
	TraySetMinimizeToTray(thePrefs.GetMinTrayPTR());
	TrayMinimizeToTrayChange();

	ShowTransferRate(true);
    ShowPing();
	searchwnd->UpdateCatTabs();

	///////////////////////////////////////////////////////////////////////////
	// Restore saved window placement
	//
	WINDOWPLACEMENT wp = {0};
	wp.length = sizeof(wp);
	wp = thePrefs.GetEmuleWindowPlacement();
	if (m_bStartMinimized)
	{
		// To avoid the window flickering during startup we try to set the proper window show state right here.
		if (*thePrefs.GetMinTrayPTR())
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
		else {
			// Minimize to System Taskbar
			//
			if (wp.showCmd == SW_SHOWMAXIMIZED)
				wp.flags = WPF_RESTORETOMAXIMIZED;
			wp.showCmd = SW_MINIMIZE; // Minimize window but do not activate it.
			m_bStartMinimizedChecked = true;
		}
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

	if (thePrefs.GetWSIsEnabled())
		theApp.webserver->StartServer();
	theApp.mmserver->Init();

	VERIFY( (m_hTimer = ::SetTimer(NULL, NULL, 300, StartupTimer)) != NULL );
	if (thePrefs.GetVerbose() && !m_hTimer)
		AddDebugLogLine(true,_T("Failed to create 'startup' timer - %s"),GetErrorMessage(GetLastError()));

	theStats.starttime = GetTickCount();

	// Start UPnP prot forwarding
	if (thePrefs.IsUPnPEnabled())
		StartUPnP();

	if (thePrefs.IsFirstStart())
	{
		// temporary disable the 'startup minimized' option, otherwise no window will be shown at all
		m_bStartMinimized = false;
		//DestroySplash();
		theApp.HideSplash(); // NEO: SS - [SplashScreen] <-- Xanatos --

		// SLUGFILLER: SafeHash remove - wait until emule is ready before opening the wizard  // NEO: STS - [SlugFillerThreadSafe] -- Xanatos -->
		/*extern BOOL FirstTimeWizard();
		if (FirstTimeWizard()){
			// start connection wizard
			CConnectionWizardDlg conWizard;
			conWizard.DoModal();
		}*/  // NEO: STS END <-- Xanatos --
	}

	VERIFY( m_pDropTarget->Register(this) );

	// initalize PeerCache
	theApp.m_pPeerCache->Init(thePrefs.GetPeerCacheLastSearch(), thePrefs.WasPeerCacheFound(), thePrefs.IsPeerCacheDownloadEnabled(), thePrefs.GetPeerCachePort());
	
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	m_ttip.Create(this);
	SetTTDelay();
	m_ttip.AddTool(statusbar, _T(""));
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

	// NEO: SSH - [SlugFillerSafeHash] -- Xanatos -->
	// SiRoB: SafeHash fix (see StartupTimer)
	/*// start aichsyncthread
	AfxBeginThread(RUNTIME_CLASS(CAICHSyncThread), THREAD_PRIORITY_BELOW_NORMAL,0);
	*/
	// NEO: SSH END <-- Xanatos --

	// debug info
	DebugLog(_T("Using '%s' as config directory"), thePrefs.GetMuleDirectory(EMULE_CONFIGDIR)); 

	// NEO: IM - [InvisibelMode] -- Xanatos -->
	if(NeoPrefs.GetInvisibleMode()) 
		RegisterInvisibleHotKey();
	// NEO: IM END <-- Xanatos --

	theApp.AttachSplash(); // NEO: SS - [SplashScreen] <-- Xanatos --

	return TRUE;
}

// modders: dont remove or change the original versioncheck! (additionals are ok)
void CemuleDlg::DoVersioncheck(bool manual) {

	if (!manual && thePrefs.GetLastVC()!=0) {
		CTime last(thePrefs.GetLastVC());
		struct tm tmTemp;
		time_t tLast = safe_mktime(last.GetLocalTm(&tmTemp));
		time_t tNow = safe_mktime(CTime::GetCurrentTime().GetLocalTm(&tmTemp));
#ifndef _BETA
		if ( (difftime(tNow,tLast) / 86400) < thePrefs.GetUpdateDays() ){
#else
		if ( (difftime(tNow,tLast) / 86400) < 3 ){
#endif
			return;
		}
	}
#ifndef _BETA
	if (WSAAsyncGetHostByName(m_hWnd, UM_VERSIONCHECK_RESPONSE, "vcdns2.emule-project.org", m_acVCDNSBuffer, sizeof(m_acVCDNSBuffer)) == 0){
#else
	if (WSAAsyncGetHostByName(m_hWnd, UM_VERSIONCHECK_RESPONSE, "vcdns1.emule-project.org", m_acVCDNSBuffer, sizeof(m_acVCDNSBuffer)) == 0){
#endif
		AddLogLine(true,GetResString(IDS_NEWVERSIONFAILED));
	}
}

// NEO: NVC - [NeoVersionCheck] -- Xanatos -->
void CemuleDlg::DoNeoVersioncheck(bool manual){

	if (!manual && thePrefs.GetLastVC()!=0) {
		CTime last(thePrefs.GetLastVC());
		time_t tLast=safe_mktime(last.GetLocalTm());
		time_t tNow=safe_mktime(CTime::GetCurrentTime().GetLocalTm());
#ifdef _DEBUG_NEO
		if ( (difftime(tNow,tLast) / 86400) < 1 ){
#else
		if ( (difftime(tNow,tLast) / 86400) < thePrefs.GetUpdateDays() ){
#endif
			return;
		}
	}
	if (WSAAsyncGetHostByName(m_hWnd, UM_NEO_VERSIONCHECK_RESPONSE, "neomule.ath.cx", m_acNVCDNSBuffer, sizeof(m_acNVCDNSBuffer)) == 0){
		AddLogLine(true,GetResString(IDS_NEWVERSIONFAILED));
	}
}
// NEO: NVC END <-- Xanatos --

void CALLBACK CemuleDlg::StartupTimer(HWND /*hwnd*/, UINT /*uiMsg*/, UINT /*idEvent*/, DWORD /*dwTime*/)
// NEO: ND - [NeoDebug] -- Xanatos -->
{
	theApp.emuledlg->SendMessage(TM_DOTIMER, (WPARAM)-1, NULL);
}

// Note: the timers does not crash on a exception so I use the messags to call the functions 
//			so when an error aprears it will be detected properly.
afx_msg LRESULT CemuleDlg::DoTimer(WPARAM wParam, LPARAM /*lParam*/)
{
	if(wParam == NULL)
		theApp.uploadqueue->UploadTimer();
	else if(wParam == -1)
		theApp.emuledlg->StartupTimer();
	else
		ASSERT(0);
	return 0;
}

void CemuleDlg::StartupTimer()
// NEO: ND END <-- Xanatos --
{
	// NOTE: Always handle all type of MFC exceptions in TimerProcs - otherwise we'll get mem leaks
#ifndef _DEBUG // NEO: ND - [NeoDebug] <-- Xanatos --
	try
	{
#endif // NEO: ND - [NeoDebug] <-- Xanatos --
		switch(theApp.emuledlg->status){
			case 0:
				theApp.emuledlg->status++;
				theApp.emuledlg->ready = true;
				// NEO: SSH - [SlugFillerSafeHash] -- Xanatos -->
				// SLUGFILLER: SafeHash remove - moved down
				/*theApp.sharedfiles->SetOutputCtrl(&theApp.emuledlg->sharedfileswnd->sharedfilesctrl);*/
				// NEO: SSH END <-- Xanatos --
				theApp.emuledlg->status++;
				break;
			case 1:
				break;
			case 2:
				theApp.emuledlg->status++;
				try{
					theApp.serverlist->Init();
				}
				catch(...){
					ASSERT(0);
					LogError(LOG_STATUSBAR,_T("Failed to initialize server list - Unknown exception"));
				}
				theApp.emuledlg->status++;
				break;
			case 3:
				break;
			case 4:{
				bool bError = false;
				theApp.emuledlg->status++;

				CSingleLock sLock1(&theApp.hashing_mut,TRUE); // NEO: SSH - [SlugFillerSafeHash] <-- Xanatos -- // David: obtimise file acceses

				// NOTE: If we have an unhandled exception in CDownloadQueue::Init, MFC will silently catch it
				// and the creation of the TCP and the UDP socket will not be done -> client will get a LowID!
				try{
					theApp.UpdateSplash(GetResString(IDS_X_SS_PREP_DQ)); // NEO: SS - [SplashScreen] <-- Xanatos --
					theApp.downloadqueue->Init();
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
					theApp.UpdateSplash(GetResString(IDS_X_SS_LOAD_SRC)); // NEO: SS - [SplashScreen]
					theApp.downloadqueue->LoadSources();
#endif // NEO_SS // NEO: NSS END <-- Xanatos --
				}
				catch(...){
					ASSERT(0);
					LogError(LOG_STATUSBAR,_T("Failed to initialize download queue - Unknown exception"));
					bError = true;
				}
				if (!theApp.listensocket->StartListening()) {
					CString strError;
					strError.Format(GetResString(IDS_MAIN_SOCKETERROR), thePrefs.GetPort());
					LogError(LOG_STATUSBAR, _T("%s"), strError);
					if (thePrefs.GetNotifierOnImportantError())
						theApp.emuledlg->ShowNotifier(strError, TBN_IMPORTANTEVENT);
					bError = true;
				}
				if (!theApp.clientudp->Create()) {
					CString strError;
					strError.Format(GetResString(IDS_MAIN_SOCKETERROR), thePrefs.GetUDPPort());
					LogError(LOG_STATUSBAR, _T("%s"), strError);
					if (thePrefs.GetNotifierOnImportantError())
						theApp.emuledlg->ShowNotifier(strError, TBN_IMPORTANTEVENT);
				}
				// NEO: MOD - [BindToAdapter] -- Xanatos -->
				if(!theApp.serverconnect->CreateUDP()){
					CString strError;
					strError.Format(GetResString(IDS_MAIN_SOCKETERROR), thePrefs.GetServerUDPPort());
					LogError(LOG_STATUSBAR, _T("%s"), strError);
					if (thePrefs.GetNotifierOnImportantError())
						theApp.emuledlg->ShowNotifier(strError, TBN_IMPORTANTEVENT);
				}
				// NEO: MOD END <-- Xanatos --
				
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
				if(NeoPrefs.IsLanSupportEnabled())
					theApp.lancast->SelectAdapters();

				if (NeoPrefs.IsLancastEnabled())
					theApp.lancast->Start();
#endif //LANCAST // NEO: NLC END <-- Xanatos --

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
				if (NeoPrefs.IsVoodooEnabled()){
					theApp.UpdateSplash(GetResString(IDS_X_SS_PREP_VD)); // NEO: SS - [SplashScreen]
					theApp.voodoo->Start();
				}
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

#ifdef WS2 // NEO: WS2 - [WINSOCK2] -- Xanatos -->
				AddLogLine(false,_T("Winsock: Version %d.%d [%.40s] %.40s"), HIBYTE( theApp.m_wsaData.wVersion ),LOBYTE(theApp.m_wsaData.wVersion ),
					CString(theApp.m_wsaData.szDescription), CString(theApp.m_wsaData.szSystemStatus));
				if (theApp.m_wsaData.iMaxSockets!=0)
					AddLogLine(false,_T("Winsock: max. sockets %d"), theApp.m_wsaData.iMaxSockets);
				else
					AddLogLine(false,_T("Winsock: unlimited sockets"));
#endif // WS2 // NEO: WS2 END <-- Xanatos --

				if (!bError){ // show the success msg, only if we had no serious error
					AddLogLine(true, GetResString(IDS_MAIN_READY), theApp.m_strCurVersionLong);
					ModLog(GetResString(IDS_X_MAIN_READY),theApp.GetAppTitle()); // NEO: NV - [NeoVersion] <-- Xanatos --
				}

				// NEO: SSH - [SlugFillerSafeHash] -- Xanatos -->
				// SLUGFILLER: SafeHash remove - moved down
				/*if(thePrefs.DoAutoConnect())
					theApp.emuledlg->OnBnClickedButton2();*/
				// NEO: SSH END <-- Xanatos --

				theApp.UpdateSplash(GetResString(IDS_X_SS_DONE)); // NEO: SS - [SplashScreen] <-- Xanatos --
				break;
			}
			case 5:
				if (thePrefs.IsStoringSearchesEnabled())
					theApp.searchlist->LoadSearches();
				theApp.emuledlg->status++;
				break;
		// BEGIN SLUGFILLER: SafeHash - delay load shared files // NEO: SSH - [SlugFillerSafeHash] -- Xanatos -->
		case 6:
			theApp.emuledlg->status++;
			theApp.sharedfiles->SetOutputCtrl(&theApp.emuledlg->sharedfileswnd->sharedfilesctrl);
			
			// BEGIN SiRoB: SafeHash fix originaly in OnInitDialog (delay load shared files)
			// start aichsyncthread
			AfxBeginThread(RUNTIME_CLASS(CAICHSyncThread), THREAD_PRIORITY_BELOW_NORMAL,0);
			// END SiRoB: SafeHash
			theApp.emuledlg->status++;
			break;
		case 7:
			break;
		case 255:
			break;
		// END SLUGFILLER: SafeHash	// NEO: SSH END <-- Xanatos --
			default:
				// BEGIN SLUGFILLER: SafeHash // NEO: SSH - [SlugFillerSafeHash] -- Xanatos -->
				theApp.emuledlg->status = 255;
				//autoconnect only after emule loaded completely
				if(thePrefs.DoAutoConnect())
					theApp.emuledlg->OnBnClickedButton2();
				// wait until emule is ready before opening the wizard
				if (thePrefs.IsFirstStart())
				{
					extern BOOL FirstTimeWizard();
					if (FirstTimeWizard()){
						// start connection wizard
						CConnectionWizardDlg conWizard;
						conWizard.DoModal();
					}
				}
				// END SLUGFILLER: SafeHash	// NEO: SSH END <-- Xanatos --
				theApp.emuledlg->StopTimer();
		}
#ifndef _DEBUG // NEO: ND - [NeoDebug] <-- Xanatos --
	}
	CATCH_DFLT_EXCEPTIONS(_T("CemuleDlg::StartupTimer"))
#endif // NEO: ND - [NeoDebug] <-- Xanatos --
}

void CemuleDlg::StopTimer()
{
	if (m_hTimer){
		VERIFY( ::KillTimer(NULL, m_hTimer) );
		m_hTimer = 0;
	}
	
	if (thePrefs.UpdateNotify())
		//DoVersioncheck(false);
		DoNeoVersioncheck(false); // NEO: NVC - [NeoVersionCheck] <-- Xanatos --
	
	theApp.HideSplash(); // NEO: SS - [SplashScreen] <-- Xanatos --

	if (theApp.pstrPendingLink != NULL){
		OnWMData(NULL, (LPARAM)&theApp.sendstruct);
		delete theApp.pstrPendingLink;
	}
}

void CemuleDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// Systemmenu-Speedselector
	if (nID >= MP_QS_U10 && nID <= MP_QS_UP10) {
		QuickSpeedUpload(nID);
		return;
	}
	if (nID >= MP_QS_D10 && nID <= MP_QS_DC) {
		QuickSpeedDownload(nID);
		return;
	}
	if (nID == MP_QS_PA || nID == MP_QS_UA) {
		QuickSpeedOther(nID);
		return;
	}
	
	switch (nID /*& 0xFFF0*/)
	{
		case MP_ABOUTBOX: {
			CCreditsDlg dlgAbout;
			dlgAbout.DoModal();
			break;
		}
		case MP_VERSIONCHECK:
			DoVersioncheck(true);
			break;
		// NEO: NVC - [NeoVersionCheck] -- Xanatos -->
		case MP_NEOVERSIONCHECK:
			DoNeoVersioncheck(true);
			break;
		// NEO: NVC END <-- Xanatos --
		case MP_CONNECT:
			StartConnection();
			break;
		case MP_DISCONNECT:
			CloseConnection();
			break;
		default:
			CTrayDialog::OnSysCommand(nID, lParam);
	}

	if ((nID & 0xFFF0) == SC_MINIMIZE		||
		(nID & 0xFFF0) == MP_MINIMIZETOTRAY	||
		(nID & 0xFFF0) == SC_RESTORE		||
		(nID & 0xFFF0) == SC_MAXIMIZE)
	{
		ShowTransferRate(true);
		ShowPing();
		transferwnd->UpdateCatTabTitles();
	}
}

void CemuleDlg::PostStartupMinimized()
{
	if (!m_bStartMinimizedChecked)
	{
		//TODO: Use full initialized 'WINDOWPLACEMENT' and remove the 'OnCancel' call...
		// Isn't that easy.. Read comments in OnInitDialog..
		m_bStartMinimizedChecked = true;
		if (m_bStartMinimized)
		{
			if (theApp.DidWeAutoStart())
			{
				if (!thePrefs.mintotray) {
					thePrefs.mintotray = true;
					MinimizeWindow();
					thePrefs.mintotray = false;
				}
				else
					MinimizeWindow();
			}
			else
				MinimizeWindow();
		}
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

void CemuleDlg::OnBnClickedButton2(){
	if (!theApp.IsConnected())
		//connect if not currently connected
		if (!theApp.serverconnect->IsConnecting() && !Kademlia::CKademlia::IsRunning() ){
			StartConnection();
		}
		else {
			CloseConnection();
		}
	else{
		//disconnect if currently connected
		CloseConnection();
	}
}

void CemuleDlg::ResetServerInfo(){
	serverwnd->servermsgbox->Reset();
}
void CemuleDlg::ResetLog(){
	serverwnd->logbox->Reset();
}

// NEO: ML - [ModLog] -- Xanatos -->
void CemuleDlg::ResetModLog(){
	serverwnd->modlogbox->Reset();
}
// NEO: ML END <-- Xanatos --

void CemuleDlg::ResetDebugLog(){
	serverwnd->debuglog->Reset();
}

void CemuleDlg::AddLogText(UINT uFlags, LPCTSTR pszText)
{
	if (GetCurrentThreadId() != _uMainThreadId)
	{
		theApp.QueueLogLineEx(uFlags, _T("%s"), pszText);
		return;
	}

	if (uFlags & LOG_STATUSBAR)
	{
        if (statusbar->m_hWnd /*&& ready*/)
		{
			if (theApp.m_app_state != APP_STATE_SHUTTINGDOWN)
				statusbar->SetText(pszText, SBarLog, 0);
		}
		else
			AfxMessageBox(pszText);
	}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	Debug(_T("%s\n"), pszText);
#endif

	if ((uFlags & LOG_DEBUG) && !thePrefs.GetVerbose())
		return;

	TCHAR temp[1060];
	int iLen = _sntprintf(temp, _countof(temp), _T("%s: %s\r\n"), CTime::GetCurrentTime().Format(thePrefs.GetDateTimeFormat4Log()), pszText);
	if (iLen >= 0)
	{
		//if (!(uFlags & LOG_DEBUG))
		if (!(uFlags & LOG_DEBUG) && !(uFlags & LOG_MOD) ) // NEO: ML - [ModLog] <-- Xanatos --
		{
			serverwnd->logbox->AddTyped(temp, iLen, uFlags & LOGMSGTYPEMASK);
			if (IsWindow(serverwnd->StatusSelector) && serverwnd->StatusSelector.GetCurSel() != CServerWnd::PaneLog)
				serverwnd->StatusSelector.HighlightItem(CServerWnd::PaneLog, TRUE);
			if (!(uFlags & LOG_DONTNOTIFY) && ready)
				ShowNotifier(pszText, TBN_LOG);
			if (thePrefs.GetLog2Disk())
				theLog.Log(temp, iLen);
		}

		// NEO: ML - [ModLog] -- Xanatos -->
		if (uFlags & LOG_MOD) 
		{
			serverwnd->modlogbox->AddTyped(temp, iLen, uFlags);
			if (IsWindow(serverwnd->StatusSelector) && serverwnd->StatusSelector.GetCurSel() != CServerWnd::PaneModLog)
				serverwnd->StatusSelector.HighlightItem(CServerWnd::PaneModLog, TRUE);
			if (ready)
				ShowNotifier(pszText, TBN_LOG);
			if (thePrefs.GetLog2Disk())
				theModLog.Log(temp, iLen);
		}
		// NEO: ML END - [ModLog] <-- Xanatos --

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
CString CemuleDlg::GetServerInfoText()
{
	return serverwnd->servermsgbox->GetText();
}

// NEO: ML - [ModLog] -- Xanatos -->
CString CemuleDlg::GetLastModLogEntry()
{
	return serverwnd->modlogbox->GetLastLogEntry();
}

CString CemuleDlg::GetAllModLogEntries(){
	return serverwnd->modlogbox->GetLastLogEntry();
}
// NEO: ML END <-- Xanatos --

void CemuleDlg::AddServerMessageLine(UINT uFlags, LPCTSTR pszLine)
{
	CString strMsgLine(pszLine);
	strMsgLine += _T('\n');
	if ((uFlags & LOGMSGTYPEMASK) == LOG_INFO)
		serverwnd->servermsgbox->AppendText(strMsgLine);
	else
		serverwnd->servermsgbox->AddTyped(strMsgLine, strMsgLine.GetLength(), uFlags & LOGMSGTYPEMASK);
	if (IsWindow(serverwnd->StatusSelector) && serverwnd->StatusSelector.GetCurSel() != CServerWnd::PaneServerInfo)
		serverwnd->StatusSelector.HighlightItem(CServerWnd::PaneServerInfo, TRUE);
}

UINT CemuleDlg::GetConnectionStateIconIndex() const
{
	if (theApp.serverconnect->IsConnected() && !Kademlia::CKademlia::IsConnected())
	{
		if (theApp.serverconnect->IsLowID())
			return 3; // LowNot
		else
			return 6; // HighNot
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
			return 4; // LowLow
		else if (theApp.serverconnect->IsLowID())
			return 5; // LowHigh
		else if (Kademlia::CKademlia::IsFirewalled())
			return 7; // HighLow
		else
			return 8; // HighHigh
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

CString CemuleDlg::GetConnectionStateString()
{
	CString status;
	if (theApp.serverconnect->IsConnected())
		status = _T("eD2K:") + GetResString(IDS_CONNECTED);
	else if (theApp.serverconnect->IsConnecting())
		status = _T("eD2K:") + GetResString(IDS_CONNECTING);
	else
		status = _T("eD2K:") + GetResString(IDS_NOTCONNECTED);

	if (Kademlia::CKademlia::IsConnected())
		status += _T("|Kad:") + GetResString(IDS_CONNECTED);
	else if (Kademlia::CKademlia::IsRunning())
		status += _T("|Kad:") + GetResString(IDS_CONNECTING);
	else
		status += _T("|Kad:") + GetResString(IDS_NOTCONNECTED);
	return status;
}

void CemuleDlg::ShowConnectionState()
{
	//theApp.downloadqueue->OnConnectionState(theApp.IsConnected());
	theApp.downloadqueue->OnConnectionState(theApp.GetConState()); // NEO: NCC - [NeoConnectionChecker] <-- Xanatos --
	serverwnd->UpdateMyInfo();
	serverwnd->UpdateControlsState();
	kademliawnd->UpdateControlsState();

	ShowConnectionStateIcon();
	statusbar->SetText(GetConnectionStateString(), SBarConnected, 0);

	if (theApp.IsConnected())
	{
		CString strPane(GetResString(IDS_MAIN_BTN_DISCONNECT));
		TBBUTTONINFO tbi;
		tbi.cbSize = sizeof(TBBUTTONINFO);
		tbi.dwMask = TBIF_IMAGE | TBIF_TEXT;
		tbi.iImage = 1;
		tbi.pszText = const_cast<LPTSTR>((LPCTSTR)strPane);
		toolbar->SetButtonInfo(TBBTN_CONNECT, &tbi);
	}
	else
	{
		if (theApp.serverconnect->IsConnecting() || Kademlia::CKademlia::IsRunning()) 
		{
			CString strPane(GetResString(IDS_MAIN_BTN_CANCEL));
			TBBUTTONINFO tbi;
			tbi.cbSize = sizeof(TBBUTTONINFO);
			tbi.dwMask = TBIF_IMAGE | TBIF_TEXT;
			tbi.iImage = 2;
			tbi.pszText = const_cast<LPTSTR>((LPCTSTR)strPane);
			toolbar->SetButtonInfo(TBBTN_CONNECT, &tbi);
		} 
		else 
		{
			CString strPane(GetResString(IDS_MAIN_BTN_CONNECT));
			TBBUTTONINFO tbi;
			tbi.cbSize = sizeof(TBBUTTONINFO);
			tbi.dwMask = TBIF_IMAGE | TBIF_TEXT;
			tbi.iImage = 0;
			tbi.pszText = const_cast<LPTSTR>((LPCTSTR)strPane);
			toolbar->SetButtonInfo(TBBTN_CONNECT, &tbi);
		}
	}
	ShowUserCount();
}

void CemuleDlg::ShowUserCount()
{
	uint32 totaluser, totalfile;
	totaluser = totalfile = 0;
	theApp.serverlist->GetUserFileStatus( totaluser, totalfile );
	CString buffer;
	if (theApp.serverconnect->IsConnected() && Kademlia::CKademlia::IsRunning() && Kademlia::CKademlia::IsConnected())
		buffer.Format(_T("%s:%s(%s)|%s:%s(%s)"), GetResString(IDS_UUSERS), CastItoIShort(totaluser, false, 1), CastItoIShort(Kademlia::CKademlia::GetKademliaUsers(), false, 1), GetResString(IDS_FILES), CastItoIShort(totalfile, false, 1), CastItoIShort(Kademlia::CKademlia::GetKademliaFiles(), false, 1));
	else if (theApp.serverconnect->IsConnected())
		buffer.Format(_T("%s:%s|%s:%s"), GetResString(IDS_UUSERS), CastItoIShort(totaluser, false, 1), GetResString(IDS_FILES), CastItoIShort(totalfile, false, 1));
	else if (Kademlia::CKademlia::IsRunning() && Kademlia::CKademlia::IsConnected())
		buffer.Format(_T("%s:%s|%s:%s"), GetResString(IDS_UUSERS), CastItoIShort(Kademlia::CKademlia::GetKademliaUsers(), false, 1), GetResString(IDS_FILES), CastItoIShort(Kademlia::CKademlia::GetKademliaFiles(), false, 1));
	else
		buffer.Format(_T("%s:0|%s:0"), GetResString(IDS_UUSERS), GetResString(IDS_FILES));
	statusbar->SetText(buffer, SBarUsers, 0);
}

void CemuleDlg::ShowMessageState(UINT iconnr)
{
	m_iMsgIcon = iconnr;
	statusbar->SetIcon(SBarChatMsg, imicons[m_iMsgIcon]);
}

void CemuleDlg::ShowTransferStateIcon()
{
	if (m_uUpDatarate && m_uDownDatarate)
		statusbar->SetIcon(SBarUpDown, transicons[3]);
	else if (m_uUpDatarate)
		statusbar->SetIcon(SBarUpDown, transicons[2]);
	else if (m_uDownDatarate)
		statusbar->SetIcon(SBarUpDown, transicons[1]);
	else
		statusbar->SetIcon(SBarUpDown, transicons[0]);
}

CString CemuleDlg::GetUpDatarateString(UINT uUpDatarate)
{
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
	m_uUpDatarate = uUpDatarate != (UINT)-1 ? uUpDatarate : (uint32)(theApp.bandwidthControl->GetCurrDataUpload()* 1024);
#else
	m_uUpDatarate = uUpDatarate != (UINT)-1 ? uUpDatarate : theApp.uploadqueue->GetDatarate();
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
	TCHAR szBuff[128];
	if (thePrefs.ShowOverhead()) {
		_sntprintf(szBuff, _countof(szBuff), _T("%.1f (%.1f)"), (float)m_uUpDatarate/1024, (float)theStats.GetUpDatarateOverhead()/1024);
		szBuff[_countof(szBuff) - 1] = _T('\0');
	}
	else {
		_sntprintf(szBuff, _countof(szBuff), _T("%.1f"), (float)m_uUpDatarate/1024);
		szBuff[_countof(szBuff) - 1] = _T('\0');
	}
	return szBuff;
}

CString CemuleDlg::GetDownDatarateString(UINT uDownDatarate)
{
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
	m_uDownDatarate = uDownDatarate != (UINT)-1 ? uDownDatarate :  (uint32)(theApp.bandwidthControl->GetCurrDataDownload()* 1024);
#else
	m_uDownDatarate = uDownDatarate != (UINT)-1 ? uDownDatarate : theApp.downloadqueue->GetDatarate();
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
	TCHAR szBuff[128];
	if (thePrefs.ShowOverhead()) {
		_sntprintf(szBuff, _countof(szBuff), _T("%.1f (%.1f)"), (float)m_uDownDatarate/1024, (float)theStats.GetDownDatarateOverhead()/1024);
		szBuff[_countof(szBuff) - 1] = _T('\0');
	}
	else {
		_sntprintf(szBuff, _countof(szBuff), _T("%.1f"), (float)m_uDownDatarate/1024);
		szBuff[_countof(szBuff) - 1] = _T('\0');
	}
	return szBuff;
}

CString CemuleDlg::GetTransferRateString()
{
	TCHAR szBuff[128];
	if (thePrefs.ShowOverhead()) {
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
		_sntprintf(szBuff, ARRSIZE(szBuff), GetResString(IDS_UPDOWN),
				   (float)m_uUpDatarate/1024, theApp.bandwidthControl->GetCurrUpload(),
				   (float)m_uDownDatarate/1024, theApp.bandwidthControl->GetCurrDownload());
#else
		_sntprintf(szBuff, _countof(szBuff), GetResString(IDS_UPDOWN),
			       (float)m_uUpDatarate/1024, (float)theStats.GetUpDatarateOverhead()/1024,
			       (float)m_uDownDatarate/1024, (float)theStats.GetDownDatarateOverhead()/1024);
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
		szBuff[_countof(szBuff) - 1] = _T('\0');
	}
	else {
		_sntprintf(szBuff, _countof(szBuff), GetResString(IDS_UPDOWNSMALL), (float)m_uUpDatarate/1024, (float)m_uDownDatarate/1024);
		szBuff[_countof(szBuff) - 1] = _T('\0');
	}
	return szBuff;
}

void CemuleDlg::ShowTransferRate(bool bForceAll)
{
	if (bForceAll)
		m_uLastSysTrayIconCookie = SYS_TRAY_ICON_COOKIE_FORCE_UPDATE;

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
	if(theApp.bandwidthControl){ // can be 0 on close !!!
		m_uDownDatarate = (uint32)(theApp.bandwidthControl->GetCurrDataDownload() * 1024);
		m_uUpDatarate = (uint32)(theApp.bandwidthControl->GetCurrDataUpload() * 1024);
	}
#else
	m_uDownDatarate = theApp.downloadqueue->GetDatarate();
	m_uUpDatarate = theApp.uploadqueue->GetDatarate();
#endif // NEO_BC // NEO: NBC END <-- Xanatos --

	CString strTransferRate = GetTransferRateString();
	if (TrayIsVisible() || bForceAll)
	{
		TCHAR buffer2[64];
		// set trayicon-icon
		// NEO: NSTI - [NewSystemTrayIcon] -- Xanatos -->
		int iDownRate = (int)ceil((float)(m_uDownDatarate/64) / thePrefs.GetMaxGraphDownloadRate());
		if (iDownRate > 16)
			iDownRate = 16;
		int iUpRate = (int)ceil((float)(m_uUpDatarate/64) / thePrefs.GetMaxGraphUploadRate(true));
		if (iUpRate > 16)
			iUpRate = 16;
		UpdateTrayIcon(iDownRate, iUpRate);
		// NEO: NSTI END <-- Xanatos --
		//int iDownRateProcent = (int)ceil((m_uDownDatarate/10.24) / thePrefs.GetMaxGraphDownloadRate());
		//if (iDownRateProcent > 100)
		//	iDownRateProcent = 100;
		//UpdateTrayIcon(iDownRateProcent);

		if (theApp.IsConnected()) {
			//_sntprintf(buffer2, _countof(buffer2), _T("eMule v%s (%s)\r\n%s"), theApp.m_strCurVersionLong, GetResString(IDS_CONNECTED), strTransferRate);
			_sntprintf(buffer2, _countof(buffer2), _T("%s (%s)\r\n%s"), MOD_VERSION, GetResString(IDS_CONNECTED), strTransferRate); // NEO: NV - [NeoVersion] <-- Xanatos --
			buffer2[_countof(buffer2) - 1] = _T('\0');
		}
		else {
			//_sntprintf(buffer2, _countof(buffer2), _T("eMule v%s (%s)\r\n%s"), theApp.m_strCurVersionLong, GetResString(IDS_DISCONNECTED), strTransferRate);
			_sntprintf(buffer2, _countof(buffer2), _T("%s (%s)\r\n%s"), MOD_VERSION, GetResString(IDS_DISCONNECTED), strTransferRate); // NEO: NV - [NeoVersion] <-- Xanatos --
			buffer2[_countof(buffer2) - 1] = _T('\0');
		}

		// Win98: '\r\n' is not displayed correctly in tooltip
		if (afxIsWin95()) {
			LPTSTR psz = buffer2;
			while (*psz) {
				if (*psz == _T('\r') || *psz == _T('\n'))
					*psz = _T(' ');
				psz++;
			}
		}
		TraySetToolTip(buffer2);
	}

	if (IsWindowVisible() || bForceAll)
	{
		statusbar->SetText(strTransferRate, SBarUpDown, 0);
		ShowTransferStateIcon();
	}
	//if (IsWindowVisible() && thePrefs.ShowRatesOnTitle())
	if (IsWindowVisible() && (thePrefs.ShowRatesOnTitle() || NeoPrefs.ShowSysInfoOnTitle())) // NEO: SI - [SysInfo] <-- Xanatos --
	{
		// MOD: CI#1 - [CodeImprovement] -- Xanatos -->
		CString buffer;
		if(thePrefs.ShowRatesOnTitle()){ // NEO: SI - [SysInfo]
			buffer.AppendFormat(_T("(U:%.1f D:%.1f)"), (float)m_uUpDatarate/1024, (float)m_uDownDatarate/1024);  // NEO: NV - [NeoVersion]
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
			if(NeoPrefs.UseVoodooTransfer())
				buffer.AppendFormat(_T(" + {vUp:%.1f vDown:%.1f}"), (float)theApp.voodoo->GetUpDatarate()/1024, (float)theApp.voodoo->GetDownDatarate()/1024);
#endif // VOODOO // NEO: VOODOO END
		}

		// NEO: SI - [SysInfo]
		if(NeoPrefs.ShowSysInfoOnTitle()){
			if(!buffer.IsEmpty())
				buffer.Append(_T(" ~ "));
			buffer.AppendFormat(_T("[Cpu:%2i%% Memory:%3iMb]"), theApp.sysinfo->GetCpuUsage(), (theApp.sysinfo->GetMemoryUsage()/1024));
		}
		// NEO: SI END

		if(!buffer.IsEmpty())
			buffer.Append(_T(" - "));
		buffer.AppendFormat(_T("%s"),theApp.GetAppTitle(true));  // NEO: NV - [NeoVersion]
		SetWindowText(buffer);
		// MOD: CI#1 END <-- Xanatos --
		//TCHAR szBuff[128];
		//_sntprintf(szBuff, _countof(szBuff), _T("(U:%.1f D:%.1f) eMule v%s"), (float)m_uUpDatarate/1024, (float)m_uDownDatarate/1024, theApp.m_strCurVersionLong);
		//szBuff[_countof(szBuff) - 1] = _T('\0');
		//SetWindowText(szBuff);
	}

	// NEO: PSM - [PlusSpeedMeter] -- Xanatos -->
	if(toolbar->IsSpeedMeterEnabled())
		toolbar->SetSpeedMeterValues(m_uUpDatarate, m_uDownDatarate);
	// NEO: PSM END <-- Xanatos --

	if (m_pMiniMule && m_pMiniMule->m_hWnd && m_pMiniMule->IsWindowVisible() && !m_pMiniMule->GetAutoClose())
	{
		m_pMiniMule->UpdateContent(m_uUpDatarate, m_uDownDatarate);
	}
}

void CemuleDlg::ShowPing()
{
    if (IsWindowVisible())
	{
        CString buffer;
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
		if (NeoPrefs.IsCheckConnection() || NeoPrefs.IsUSSEnabled() || NeoPrefs.IsDSSEnabled()){
			if(theApp.bandwidthControl->IsPingPreparing())
				buffer = GetResString(IDS_X_SS_PREPARING);
			else if(theApp.bandwidthControl->IsPingTimeout())
				buffer = GetResString(IDS_X_SS_TIMEOUT);
			else if(theApp.bandwidthControl->IsPingWorking())
			{
				buffer.Format(_T("%ims (%i)"), (int)theApp.bandwidthControl->GetCurrPing(), (int)theApp.bandwidthControl->GetLowestPing());

				if(NeoPrefs.IsUSSEnabled()) {
					buffer.AppendFormat(_T(" | %.1f (%i)"), theApp.bandwidthControl->GetCalcUpload(), (int) theApp.bandwidthControl->GetBasePingUp());
				}

				if(NeoPrefs.IsDSSEnabled()) {
					buffer.AppendFormat(_T(" | %.1f (%i)"), theApp.bandwidthControl->GetCalcDownload(), (int) theApp.bandwidthControl->GetBasePingDown());
                }
 
			}
			else
				buffer = GetResString(IDS_X_SS_FAILED);
        }
#else
        if (thePrefs.IsDynUpEnabled())
		{
			CurrentPingStruct lastPing = theApp.lastCommonRouteFinder->GetCurrentPing();
            if (lastPing.state.GetLength() == 0)
			{
                if (lastPing.lowest > 0 && !thePrefs.IsDynUpUseMillisecondPingTolerance())
                    buffer.Format(_T("%.1f | %ims | %i%%"),lastPing.currentLimit/1024.0f, lastPing.latency, lastPing.latency*100/lastPing.lowest);
                else
                    buffer.Format(_T("%.1f | %ims"),lastPing.currentLimit/1024.0f, lastPing.latency);
            }
			else
                buffer.SetString(lastPing.state);
        }
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
		statusbar->SetText(buffer, SBarChatMsg, 0);
    }
}

void CemuleDlg::OnOK()
{
}

void CemuleDlg::OnCancel()
{
	//if (!thePrefs.GetStraightWindowStyles())
	if (!thePrefs.GetStraightWindowStyles() && *thePrefs.GetMinTrayPTR()) // NEO: MOD - [ESCFix] <-- Xanatos --
		MinimizeWindow();
}

void CemuleDlg::MinimizeWindow()
{
	if (*thePrefs.GetMinTrayPTR())
	{
		//TrayShow();
		// NEO: IM - [InvisibelMode] -- Xanatos -->
		if(NeoPrefs.GetInvisibleMode()){
			// NEO: STI - [StaticTray]
			if(NeoPrefs.UseStaticTrayIcon()) 
				TrayHide(true);
			// NEO: MOD END
			ShowWindow(SW_HIDE);
		}else if(TrayShow())
			ShowWindow(SW_HIDE);
		// NEO: IM END <-- Xanatos --
		ShowWindow(SW_HIDE);
	}
	else
	{
		ShowWindow(SW_MINIMIZE);
	}
	ShowTransferRate();
	ShowPing();
}

// NEO: TPP - [TrayPasswordProtection] -- Xanatos -->
BOOL CemuleDlg::ShowWindow(int nCmdShow)
{
	if((NeoPrefs.IsTrayPasswordProtection() && nCmdShow == SW_HIDE) || nCmdShow == -1){
		if(nCmdShow == -1)
			if(!TrayShow())
				return FALSE;
		if(NeoPrefs.GetTrayPassword().IsEmpty()){
			InputBox inputbox;
			//inputbox.SetPassword(true);
			inputbox.SetLabels(GetResString(IDS_X_LOCK_TRAY), GetResString(IDS_X_LOCK_TRAY_PW),m_TrayPassword);
			if (inputbox.DoModal() != IDOK){
				if(!NeoPrefs.UseStaticTrayIcon()) // NEO: STI - [StaticTray]
					TrayHide();
				return FALSE;
			}

			m_TrayPassword = inputbox.GetInput();
		}else
			m_TrayPassword = NeoPrefs.GetTrayPassword();
		m_TrayLocked = true;
		nCmdShow = SW_HIDE;
	}
	return CResizableDialog::ShowWindow(nCmdShow);
}
// NEO: TPP END <-- Xanatos --

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
	if (iToolbarButtonID != -1)
		toolbar->PressMuleButton(iToolbarButtonID);
	if (dlg == transferwnd){
		if (thePrefs.ShowCatTabInfos())
			transferwnd->UpdateCatTabTitles();
	}
	else if (dlg == chatwnd){
		chatwnd->chatselector.ShowChat();
	}
	else if (dlg == statisticswnd){
		statisticswnd->ShowStatistics();
	}
}

void CemuleDlg::SetStatusBarPartsSize()
{
	CRect rect;
	statusbar->GetClientRect(&rect);
	int ussShift = 0;
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
	if(NeoPrefs.IsCheckConnection() || NeoPrefs.IsUSSEnabled() || NeoPrefs.IsDSSEnabled())
		ussShift += 80;

	if(NeoPrefs.IsUSSEnabled())
		ussShift += 40;

	if(NeoPrefs.IsDSSEnabled())
		ussShift += 40;
#else
	if(thePrefs.IsDynUpEnabled())
	{
        if (thePrefs.IsDynUpUseMillisecondPingTolerance())
            ussShift = 45;
        else
            ussShift = 90;
	}
#endif // NEO_BC // NEO: NBC END <-- Xanatos --

	int aiWidths[5] =
	{ 
		rect.right - 675 - ussShift,
		rect.right - 440 - ussShift,
		rect.right - 250 - ussShift,
		rect.right -  25 - ussShift,
		-1
	};
	statusbar->SetParts(_countof(aiWidths), aiWidths);
}

void CemuleDlg::OnSize(UINT nType, int cx, int cy)
{
	CTrayDialog::OnSize(nType, cx, cy);
	SetStatusBarPartsSize();
	transferwnd->VerifyCatTabSize();
}

void CemuleDlg::ProcessED2KLink(LPCTSTR pszData)
{
	try {
		CString link2;
		CString link;
		link2 = pszData;
		link2.Replace(_T("%7c"),_T("|"));
		link = OptUtf8ToStr(URLDecode(link2));
		CED2KLink* pLink = CED2KLink::CreateLinkFromUrl(link);
		_ASSERT( pLink !=0 );
		switch (pLink->GetKind()) {
		case CED2KLink::kFile:
			{
				CED2KFileLink* pFileLink = pLink->GetFileLink();
				_ASSERT(pFileLink !=0);
				//theApp.downloadqueue->AddFileLinkToDownload(pFileLink,searchwnd->GetSelectedCat());
				theApp.downloadqueue->AddFileLinkToDownload(pFileLink,-1); // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
				pLink = NULL; // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
			}
			break;
		case CED2KLink::kServerList:
			{
				CED2KServerListLink* pListLink = pLink->GetServerListLink(); 
				_ASSERT( pListLink !=0 ); 
				CString strAddress = pListLink->GetAddress(); 
				if(strAddress.GetLength() != 0)
					serverwnd->UpdateServerMetFromURL(strAddress);
			}
			break;
		case CED2KLink::kNodesList:
			{
				CED2KNodesListLink* pListLink = pLink->GetNodesListLink(); 
				_ASSERT( pListLink !=0 ); 
				CString strAddress = pListLink->GetAddress();
				// Becasue the nodes.dat is vital for kad and its routing and doesn't needs to be updated in general
				// we request a confirm to avoid accidental / malicious updating of this file. This is a bit inconsitent
				// as the same kinda applies to the server.met, but those require more updates and are easier to understand
				CString strConfirm;
				strConfirm.Format(GetResString(IDS_CONFIRMNODESDOWNLOAD), strAddress);
				if(strAddress.GetLength() != 0 && AfxMessageBox(strConfirm, MB_YESNO | MB_ICONQUESTION, 0) == IDYES)
					kademliawnd->UpdateNodesDatFromURL(strAddress);
			}
			break;
		case CED2KLink::kServer:
			{
				CString defName;
				CED2KServerLink* pSrvLink = pLink->GetServerLink();
				_ASSERT( pSrvLink !=0 );
				CServer* pSrv = new CServer(pSrvLink->GetPort(), pSrvLink->GetAddress());
				_ASSERT( pSrv !=0 );
				pSrvLink->GetDefaultName(defName);
				pSrv->SetListName(defName);

				// Barry - Default all new servers to high priority
				if (thePrefs.GetManualAddedServersHighPriority())
					pSrv->SetPreference(SRV_PR_HIGH);

				if (!serverwnd->serverlistctrl.AddServer(pSrv,true)) 
					delete pSrv; 
				else
					AddLogLine(true,GetResString(IDS_SERVERADDED), pSrv->GetListName());
			}
			break;
			// NEO: TFL - [TetraFriendLinks] -- Xanatos -->
		case CED2KLink::kFriend:
			{
				CED2KFriendLink* pFLink = pLink->GetFriendLink();
				ASSERT( pFLink !=0 );

				if (theApp.friendlist->AddFriend(pFLink->GetHash(), 0, pFLink->GetIP(), pFLink->GetPort(), 0, pFLink->GetName(), pFLink->GetHasHash()))	//T-ToDo, T?
					AddLogLine(true,GetResString(IDS_X_FRIENDADDED), pFLink->GetName());
			}
			break;
			// NEO: TFL END  <-- Xanatos --
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
		if (thePrefs.IsBringToFront())
		{
			FlashWindow(TRUE);
			if (IsIconic())
				ShowWindow(SW_SHOWNORMAL);
			else if (TrayHide())
				RestoreWindow();
			else
				SetForegroundWindow();
		}
		ProcessED2KLink((LPCTSTR)data->lpData);
	}
	else if(data->dwData == OP_COLLECTION){
		FlashWindow(TRUE);
		if (IsIconic())
			ShowWindow(SW_SHOWNORMAL);
		else if (TrayHide())
			RestoreWindow();
		else
			SetForegroundWindow();

		CCollection* pCollection = new CCollection();
		CString strPath = CString((LPCTSTR)data->lpData);
		if (pCollection->InitCollectionFromFile(strPath, strPath.Right((strPath.GetLength()-1)-strPath.ReverseFind('\\')))){
			// NEO: MLD - [ModelesDialogs] -- Xanatos -->
			CCollectionViewDialog* dialog = new CCollectionViewDialog(); 
			dialog->SetCollection(pCollection,true);
			dialog->OpenDialog();
			// NEO: MLD END <-- Xanatos --
			//CCollectionViewDialog dialog;
			//dialog.SetCollection(pCollection);
			//dialog.DoModal();
		}
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
		if (clcommand==_T("exit")) {OnClose(); return true;}
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
			if (down.GetLength()>0) thePrefs.SetMaxDownload(_tstoi(down));
			if (up.GetLength()>0) thePrefs.SetMaxUpload(_tstoi(up));

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

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
				strBuff.Format(GetResString(IDS_UPDOWNSMALL), theApp.bandwidthControl->GetCurrDataUpload(), theApp.bandwidthControl->GetCurrDataDownload());
#else
				strBuff.Format(GetResString(IDS_UPDOWNSMALL), (float)theApp.uploadqueue->GetDatarate()/1024, (float)theApp.downloadqueue->GetDatarate()/1024);
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
				_ftprintf(file, _T("%s"), strBuff); // next string (getTextList) is already prefixed with '\n'!
				_ftprintf(file, _T("%s\n"), transferwnd->downloadlistctrl.getTextList());
				
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
	if (theApp.m_app_state == APP_STATE_SHUTTINGDOWN)
		return FALSE;

	CKnownFile* result = (CKnownFile*)lParam;
	ASSERT( result->IsKindOf(RUNTIME_CLASS(CKnownFile)) );

	if (wParam)
	{
		// File hashing finished for a part file when:
		// - part file just completed
		// - part file was rehashed at startup because the file date of part.met did not match the part file date

		CPartFile* requester = (CPartFile*)wParam;
		ASSERT( requester->IsKindOf(RUNTIME_CLASS(CPartFile)) );

		// SLUGFILLER: SafeHash - could have been canceled
		if (theApp.downloadqueue->IsPartFile(requester))
			requester->PartFileHashFinished(result);
		else
			delete result;
		// SLUGFILLER: SafeHash
	}
	else
	{
		ASSERT( !result->IsKindOf(RUNTIME_CLASS(CPartFile)) );

		// File hashing finished for a shared file (none partfile)
		//	- reading shared directories at startup and hashing files which were not found in known.met
		//	- reading shared directories during runtime (user hit Reload button, added a shared directory, ...)
		theApp.sharedfiles->FileHashingFinished(result);
	}
	return TRUE;
}

LRESULT CemuleDlg::OnFileOpProgress(WPARAM wParam, LPARAM lParam)
{
	if (theApp.m_app_state == APP_STATE_SHUTTINGDOWN)
		return FALSE;

	// NEO: MOD - [HashProgress] -- Xanatos -->
	if(lParam == NULL)
		sharedfileswnd->sharedfilesctrl.ShowFilesCount(wParam);
	else
	// NEO: MOD END <-- Xanatos --
	{
		CKnownFile* pKnownFile = (CKnownFile*)lParam;
		ASSERT( pKnownFile->IsKindOf(RUNTIME_CLASS(CKnownFile)) );

		if (pKnownFile->IsKindOf(RUNTIME_CLASS(CPartFile)))
		{
			CPartFile* pPartFile = static_cast<CPartFile*>(pKnownFile);
			pPartFile->SetFileOpProgress(wParam);
			pPartFile->UpdateDisplayedInfo(true);
		}
	}

	return 0;
}

// BEGIN SLUGFILLER: SafeHash // NEO: SSH - [SlugFillerSafeHash] -- Xanatos -->
LRESULT CemuleDlg::OnHashFailed(WPARAM /*wParam*/, LPARAM lParam)
{
	// BEGIN SiRoB: Fix crash at shutdown
	if (!IsRunning()) {
		UnknownFile_Struct* hashed = (UnknownFile_Struct*)lParam;
		delete hashed;
		ASSERT(0);
		return FALSE;
	}
	// END SiRoB: Fix crash at shutdown
	theApp.sharedfiles->HashFailed((UnknownFile_Struct*)lParam);
	return 0;
}

LRESULT CemuleDlg::OnPartHashed(WPARAM wParam,LPARAM lParam)
{
	// BEGIN SiRoB: Fix crash at shutdown
	if (!IsRunning()){
		delete (PartHashResult*)wParam;
		return FALSE;
	}
	// END SiRoB: Fix crash at shutdown
	CPartFile* pOwner = (CPartFile*)lParam;
	if (theApp.downloadqueue->IsPartFile(pOwner))	// could have been canceled
		pOwner->PartHashFinished(((PartHashResult*)wParam)->uPart, ((PartHashResult*)wParam)->corrupted, ((PartHashResult*)wParam)->AICHRecover,((PartHashResult*)wParam)->AICHok);
	delete (PartHashResult*)wParam;
	return 0;
}
// END SLUGFILLER: SafeHash // NEO: SSH END <-- Xanatos --

// NEO: SCV - [SubChunkVerification] -- Xanatos -->
LRESULT CemuleDlg::OnBlockHashed(WPARAM wParam,LPARAM lParam)
{
	// BEGIN SiRoB: Fix crash at shutdown
	if (!IsRunning()){
		delete (PartHashResult*)wParam;
		return FALSE;
	}
	// END SiRoB: Fix crash at shutdown
	CPartFile* pOwner = (CPartFile*)lParam;
	if (theApp.downloadqueue->IsPartFile(pOwner))	// could have been canceled
		pOwner->BlockHashFinished(((BlockHashResult*)wParam)->uPart, ((BlockHashResult*)wParam)->resultMap, ((BlockHashResult*)wParam)->AICHRecover, ((BlockHashResult*)wParam)->ForceMD4);
	delete (PartHashResult*)wParam;
	return 0;
}
// NEO: SCV END <-- Xanatos --

//MORPH START - Added by SiRoB, Flush Thread // NEO: FFT - [FileFlushThread] -- Xanatos -->
LRESULT CemuleDlg::OnFlushDone(WPARAM /*wParam*/ ,LPARAM lParam)
{
	CPartFile* partfile = (CPartFile*) lParam;
	if (theApp.m_app_state != APP_STATE_SHUTTINGDOWN && theApp.downloadqueue->IsPartFile(partfile))	// could have been canceled
		partfile->FlushDone();
	return 0;
}
//MORPH END   - Added by SiRoB, Flush Thread // NEO: FFT END <-- Xanatos --

// NEO: RBT - [ReadBlockThread] -- Xanatos -->
LRESULT CemuleDlg::OnReadBlockFromFileDone(WPARAM wParam,LPARAM lParam)
{
	if(!IsRunning()){
		if (wParam != _RBT_ERROR_ && wParam != _RBT_ACTIVE_ && wParam != NULL)
			delete[] (byte*)wParam;
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
		if(((ReadBlockOrder*)lParam)->m_voodoo)
			delete (sDataRequest*)((ReadBlockOrder*)lParam)->m_request;
#endif // VOODOO // NEO: VOODOO END
		delete (ReadBlockOrder*)lParam;
		return 0;
	}

	if(wParam == _RBT_ERROR_) //An error occured
		theApp.sharedfiles->Reload(); // reload list

	ReadBlockOrder* readOrder = (ReadBlockOrder*)lParam;
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	if(readOrder->m_voodoo)
		((CVoodooSocket*)readOrder->m_client)->SendFileData((sDataRequest*)readOrder->m_request,(byte*)wParam); // m_request is deleted inside SendFileData
	else
#endif // VOODOO // NEO: VOODOO END
	if (theApp.uploadqueue->IsDownloading((CUpDownClient*)readOrder->m_client) // could have been canceld
	 && ((CUpDownClient*)readOrder->m_client)->m_BlockRequests_queue.Find((Requested_Block_Struct*)readOrder->m_request)) // check is this block pointer still valid
		((Requested_Block_Struct*)readOrder->m_request)->filedata = (byte*)wParam;

	delete readOrder;
	return 0;
}
// NEO: RBT END <-- Xanatos --

// NEO: PIX - [PartImportExport] -- Xanatos -->
LRESULT CemuleDlg::OnImportPart(WPARAM wParam,LPARAM lParam)
{
	CPartFile* partfile = (CPartFile*) lParam;
	if (IsRunning() && theApp.downloadqueue->IsPartFile(partfile)) {	// could have been canceled 
		if(wParam){
			ImportPart_Struct* importpart = (ImportPart_Struct*)wParam;
			partfile->WriteToBuffer(importpart->end-importpart->start+1, importpart->data,importpart->start, importpart->end, NULL, 0);
		}
		partfile->FlushBuffer();
	}
	if(wParam){
		delete[] ((ImportPart_Struct*)wParam)->data;
		delete	(ImportPart_Struct*)wParam;
	}

	return 0;
}

LRESULT CemuleDlg::OnFileImported(WPARAM /*wParam*/, LPARAM lParam)
{
	CPartFile* partfile = (CPartFile*)lParam;
	ASSERT( partfile != NULL );
	if (partfile){
		if(partfile->GetStatus() == PS_IMPORTING)
			partfile->SetStatus(PS_READY);

		partfile->SetFileOpProgress(0);
		partfile->UpdateDisplayedInfo(true);
	}
	return 0;
}
// NEO: PIX END <-- Xanatos --

// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
LRESULT CemuleDlg::OnFileMoved(WPARAM wParam, LPARAM lParam)
{
	CPartFile* partfile = (CPartFile*)lParam;
	ASSERT( partfile != NULL );
	if (partfile && (wParam & FILE_MOVE_THREAD_SUCCESS)){
		partfile->SetStatus(PS_READY);

		partfile->SetFileOpProgress(0);
		partfile->UpdateDisplayedInfo(true);
	}
	return 0;
}
// NEO: MTD END <-- Xanatos --

LRESULT CemuleDlg::OnFileAllocExc(WPARAM wParam,LPARAM lParam)
{
	// NEO: SSH - [SlugFillerSafeHash] -- Xanatos -->
	// BEGIN SiRoB: Fix crash at shutdown
	CFileException* error = (CFileException*)lParam;
	if (!IsRunning()) { //MORPH - Changed by SiRoB, Flush Thread
		if (error != NULL)
			error->Delete();
		return FALSE;
	}
	// END SiRoB: Fix crash at shutdown
	// NEO: SSH END <-- Xanatos --

	if (lParam == 0)
		((CPartFile*)wParam)->FlushBuffersExceptionHandler();
	else
		((CPartFile*)wParam)->FlushBuffersExceptionHandler((CFileException*)lParam);
	return 0;
}

LRESULT CemuleDlg::OnFileCompleted(WPARAM wParam, LPARAM lParam)
{
	CPartFile* partfile = (CPartFile*)lParam;
	ASSERT( partfile != NULL );
	if (partfile)
		partfile->PerformFileCompleteEnd(wParam);
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
		theApp.m_app_state = APP_STATE_SHUTTINGDOWN;
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
	TrayHide(true); // NEO: STI - [StaticTray] <-- Xanatos --
	CTrayDialog::OnDestroy();
}

bool CemuleDlg::CanClose()
{
	if (theApp.m_app_state == APP_STATE_RUNNING && thePrefs.IsConfirmExitEnabled())
	{
		if (AfxMessageBox(GetResString(IDS_MAIN_EXIT), MB_YESNO | MB_DEFBUTTON2) == IDNO)
			return false;
	}
	return true;
}

void CemuleDlg::OnClose()
{
	if (!CanClose())
		return;

	// NEO: SS - [SplashScreen] -- Xanatos -->
	if (thePrefs.UseSplashScreen())
		theApp.ShowSplash();

	theApp.UpdateSplash(GetResString(IDS_X_SS_E_ED));
	// NEO: SS END <-- Xanatos --

	Log(_T("Closing eMule"));
	CloseTTS();
	m_pDropTarget->Revoke();
	theApp.m_app_state = APP_STATE_SHUTTINGDOWN;
	theApp.serverconnect->Disconnect();
	theApp.OnlineSig(); // Added By Bouc7 

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
	if (activewnd){
		if (activewnd->IsKindOf(RUNTIME_CLASS(CServerWnd)))
			thePrefs.SetLastMainWndDlgID(IDD_SERVER);
		else if (activewnd->IsKindOf(RUNTIME_CLASS(CSharedFilesWnd)))
			thePrefs.SetLastMainWndDlgID(IDD_FILES);
		else if (activewnd->IsKindOf(RUNTIME_CLASS(CSearchDlg)))
			thePrefs.SetLastMainWndDlgID(IDD_SEARCH);
		else if (activewnd->IsKindOf(RUNTIME_CLASS(CChatWnd)))
			thePrefs.SetLastMainWndDlgID(IDD_CHAT);
		else if (activewnd->IsKindOf(RUNTIME_CLASS(CTransferWnd)))
			thePrefs.SetLastMainWndDlgID(IDD_TRANSFER);
		else if (activewnd->IsKindOf(RUNTIME_CLASS(CStatisticsDlg)))
			thePrefs.SetLastMainWndDlgID(IDD_STATISTICS);
		else if (activewnd->IsKindOf(RUNTIME_CLASS(CKademliaWnd)))
			thePrefs.SetLastMainWndDlgID(IDD_KADEMLIAWND);
		else if (activewnd->IsKindOf(RUNTIME_CLASS(CIrcWnd)))
			thePrefs.SetLastMainWndDlgID(IDD_IRC);
		else{
			ASSERT(0);
			thePrefs.SetLastMainWndDlgID(0);
		}
	}

	Kademlia::CKademlia::Stop();	// couple of data files are written
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	theApp.lancast->Stop();
#endif //LANCAST // NEO: NLC END <-- Xanatos --
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	theApp.voodoo->Stop();
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

	// try to wait untill the hashing thread notices that we are shutting down
	CSingleLock sLock1(&theApp.hashing_mut); // only one filehash at a time
	sLock1.Lock(2000);

	theApp.emuledlg->preferenceswnd->CloseDialog(); // NEO: MLD - [ModelesDialogs] <-- Xanatos --
	// saving data & stuff
	theApp.emuledlg->preferenceswnd->m_wndSecurity.DeleteDDB();

	theApp.UpdateSplash(GetResString(IDS_X_SS_E_SKF)); // NEO: SS - [SplashScreen] <-- Xanatos --
	theApp.knownfiles->Save();										// CKnownFileList::Save
	//transferwnd->downloadlistctrl.SaveSettings();
	//transferwnd->downloadclientsctrl.SaveSettings();
	//transferwnd->uploadlistctrl.SaveSettings();
	//transferwnd->queuelistctrl.SaveSettings();
	//transferwnd->clientlistctrl.SaveSettings();
	//sharedfileswnd->sharedfilesctrl.SaveSettings();
	//chatwnd->m_FriendListCtrl.SaveSettings();
	searchwnd->SaveAllSettings();
	serverwnd->SaveAllSettings();
	kademliawnd->SaveAllSettings();

	theApp.m_pPeerCache->Save();
	theApp.scheduler->RestoreOriginals();
	theApp.searchlist->SaveSpamFilter();
	if (thePrefs.IsStoringSearchesEnabled())
		theApp.searchlist->StoreSearches();
	
	// close uPnP Ports
	theApp.m_pUPnPFinder->GetImplementation()->StopAsyncFind();
	if (thePrefs.CloseUPnPOnExit())
		theApp.m_pUPnPFinder->GetImplementation()->DeletePorts();

	thePrefs.Save();
	NeoPrefs.Save(); // NEO: NCFG - [NeoConfiguration] <-- Xanatos --
	thePerfLog.Shutdown();

	// NEO: IM - [InvisibelMode] -- Xanatos -->
	if(NeoPrefs.GetInvisibleMode()) 
		UnRegisterInvisibleHotKey();
	// NEO: IM END <-- Xanatos --

	// explicitly delete all listview items which may hold ptrs to objects which will get deleted
	// by the dtors (some lines below) to avoid potential problems during application shutdown.
	transferwnd->downloadlistctrl.DeleteAllItems();
	transferwnd->downloadclientsctrl.DeleteAllItems(); // NEO: SP - [SharedParts] <-- Xanatos --
	chatwnd->chatselector.DeleteAllItems();
	chatwnd->m_FriendListCtrl.DeleteAllItems();
	theApp.UpdateSplash(GetResString(IDS_X_SS_E_CL)); // NEO: SS - [SplashScreen] <-- Xanatos --
	theApp.uploadqueue->DeleteAll(); // NEO: MOD - [DeleteAll] <-- Xanatos --
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
	theApp.UpdateSplash(GetResString(IDS_X_SS_E_SAVE_SRC)); // NEO: SS - [SplashScreen] <-- Xanatos --
	theApp.downloadqueue->SaveSources(); 
#endif // NEO_SS // NEO: NSS END <-- Xanatos --
	theApp.UpdateSplash(GetResString(IDS_X_SS_E_CLEAR_SRC)); // NEO: SS - [SplashScreen] <-- Xanatos --
	theApp.downloadqueue->DeleteAll(); // NEO: MOD - [DeleteAll] <-- Xanatos --
	// NEO: MOD END <-- Xanatos --
	theApp.UpdateSplash(GetResString(IDS_X_SS_E_CL)); // NEO: SS - [SplashScreen] <-- Xanatos --
	theApp.clientlist->DeleteAll();
	searchwnd->DeleteAllSearchListCtrlItems();
	sharedfileswnd->sharedfilesctrl.DeleteAllItems();
    transferwnd->queuelistctrl.DeleteAllItems();
	transferwnd->clientlistctrl.DeleteAllItems();
	transferwnd->uploadlistctrl.DeleteAllItems();
	transferwnd->downloadclientsctrl.DeleteAllItems();
	serverwnd->serverlistctrl.DeleteAllItems();

	CPartFileConvert::CloseGUI();
	CPartFileConvert::RemoveAllJobs();

    theApp.uploadBandwidthThrottler->EndThread();
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
	theApp.downloadBandwidthThrottler->EndThread();
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
	theApp.bandwidthControl->EndBandwidthControl();
#else
	theApp.lastCommonRouteFinder->EndThread();
#endif // NEO_BC // NEO: NBC END <-- Xanatos --

#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->
	theApp.argos->EndThread();
#endif // ARGOS // NEO: NA END <-- Xanatos --

	theApp.sharedfiles->DeletePartFileInstances();

	searchwnd->SendMessage(WM_CLOSE);

	// NEO: STS - [SlugFillerThreadSafe] -- Xanatos -->
	DebugLog(_T("*** Waiting for all threads to finish"));
	theApp.m_threadlock.WriteLock(); // SLUGFILLER: SafeHash - Last chance, let all running threads close before we start deleting
	DebugLog(_T("*** threads finish"));
	// NEO: STS END <-- Xanatos -- 

    // NOTE: Do not move those dtors into 'CemuleApp::InitInstance' (althought they should be there). The
	// dtors are indirectly calling functions which access several windows which would not be available 
	// after we have closed the main window -> crash!
	delete theApp.mmserver;			theApp.mmserver = NULL;
	delete theApp.listensocket;		theApp.listensocket = NULL;
	delete theApp.clientudp;		theApp.clientudp = NULL;
#ifdef NATTUNNELING // NEO: UTCP - [UserModeTCP] -- Xanatos -->
	delete theApp.natmanager;		theApp.natmanager = NULL;
#endif //NATTUNNELING // NEO: UTCP END <-- Xanatos --
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	delete theApp.lancast;			theApp.lancast = NULL;
#endif //LANCAST // NEO: NLC END <-- Xanatos --
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	delete theApp.voodoo;			theApp.voodoo = NULL;
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
	delete theApp.sharedfiles;		theApp.sharedfiles = NULL;
	delete theApp.serverconnect;	theApp.serverconnect = NULL;
	delete theApp.serverlist;		theApp.serverlist = NULL;		// CServerList::SaveServermetToFile
	delete theApp.knownfiles;		theApp.knownfiles = NULL;
	delete theApp.searchlist;		theApp.searchlist = NULL;
	delete theApp.clientcredits;	theApp.clientcredits = NULL;	// CClientCreditsList::SaveList
#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase] -- Xanatos -->
	theApp.UpdateSplash(GetResString(IDS_X_SS_E_SS)); // NEO: SS - [SplashScreen]
	delete theApp.sourcelist;		theApp.sourcelist = NULL;		// CSourceList::SaveList
#endif // NEO_CD // NEO: NCD END <-- Xanatos --
	theApp.UpdateSplash(GetResString(IDS_X_SS_E_DL)); // NEO: SS - [SplashScreen]
	delete theApp.downloadqueue;	theApp.downloadqueue = NULL;	// N * (CPartFile::FlushBuffer + CPartFile::SavePartFile)
	delete theApp.uploadqueue;		theApp.uploadqueue = NULL;
	theApp.UpdateSplash(GetResString(IDS_X_SS_E_CC)); // NEO: SS - [SplashScreen] <-- Xanatos --
	delete theApp.friendlist;		theApp.friendlist = NULL;		// CFriendList::SaveList // NEO: FIX - [CrashFix] <-- Xanatos --
	delete theApp.clientlist;		theApp.clientlist = NULL;
	theApp.UpdateSplash(GetResString(IDS_X_SS_E_FS)); // NEO: SS - [SplashScreen] <-- Xanatos --
	//delete theApp.friendlist;		theApp.friendlist = NULL;		// CFriendList::SaveList
	delete theApp.scheduler;		theApp.scheduler = NULL;
	delete theApp.ipfilter;			theApp.ipfilter = NULL;			// CIPFilter::SaveToDefaultFile
#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
	delete theApp.ip2country;		theApp.ip2country = NULL;
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --
	delete theApp.webserver;		theApp.webserver = NULL;
	delete theApp.m_pPeerCache;		theApp.m_pPeerCache = NULL;
	delete theApp.m_pFirewallOpener;theApp.m_pFirewallOpener = NULL;
	delete theApp.uploadBandwidthThrottler; theApp.uploadBandwidthThrottler = NULL;
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
	delete theApp.downloadBandwidthThrottler; theApp.downloadBandwidthThrottler = NULL;
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
	delete theApp.bandwidthControl; theApp.bandwidthControl = NULL;
#else
	delete theApp.lastCommonRouteFinder; theApp.lastCommonRouteFinder = NULL;
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
	delete theApp.m_pUPnPFinder;	theApp.m_pUPnPFinder = NULL;
	delete theApp.sysinfo;			theApp.sysinfo = NULL; // NEO: SI - [SysInfo] <-- Xanatos --
#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->
	delete theApp.argos;			theApp.argos = NULL;
#endif // ARGOS // NEO: NA END <-- Xanatos --

	theApp.UpdateSplash(GetResString(IDS_X_SS_E_SD)); // NEO: SS - [SplashScreen] <-- Xanatos --
	thePrefs.Uninit();
	theApp.m_app_state = APP_STATE_DONE;
	CTrayDialog::OnCancel();
	AddDebugLogLine(DLP_VERYLOW, _T("Closed eMule"));
}

void CemuleDlg::DestroyMiniMule()
{
	if (m_pMiniMule)
	{
		if (!m_pMiniMule->IsInCallback()) // for safety
		{
			TRACE("%s - m_pMiniMule->DestroyWindow();\n", __FUNCTION__);
			m_pMiniMule->DestroyWindow();
			ASSERT( m_pMiniMule == NULL );
			m_pMiniMule = NULL;
		}
		else
			ASSERT(0);
	}
}

LRESULT CemuleDlg::OnCloseMiniMule(WPARAM wParam, LPARAM /*lParam*/)
{
	TRACE("%s -> DestroyMiniMule();\n", __FUNCTION__);
	DestroyMiniMule();
	if (wParam)
		RestoreWindow();
	return 0;
}

void CemuleDlg::OnTrayLButtonUp(CPoint /*pt*/)
{
	if(!IsRunning())
		return;

	// Avoid reentrancy problems with main window, options dialog and mini mule window
	// NEO: MLD - [ModelesDialogs] -- Xanatos --
	/*if (IsPreferencesDlgOpen()) {
		MessageBeep(MB_OK);
		preferenceswnd->SetForegroundWindow();
		preferenceswnd->BringWindowToTop();
		return;
	}*/

	if (m_pMiniMule) {
		TRACE("%s - m_pMiniMule->ShowWindow(SW_SHOW);\n", __FUNCTION__);
		m_pMiniMule->ShowWindow(SW_SHOW);
		m_pMiniMule->SetForegroundWindow();
		m_pMiniMule->BringWindowToTop();
		return;
	}

	if (thePrefs.GetEnableMiniMule())
	{
		try
		{
			TRACE("%s - m_pMiniMule = new CMiniMule(this);\n", __FUNCTION__);
			ASSERT( m_pMiniMule == NULL );
			m_pMiniMule = new CMiniMule(this);
			m_pMiniMule->Create(CMiniMule::IDD, this);
			//m_pMiniMule->ShowWindow(SW_SHOW);	// do not explicitly show the window, it will do that for itself when it's ready..
			m_pMiniMule->SetForegroundWindow();
			m_pMiniMule->BringWindowToTop();
		}
		catch(...)
		{
			ASSERT(0);
			m_pMiniMule = NULL;
		}
	}
}

void CemuleDlg::OnTrayRButtonUp(CPoint pt)
{
	if (!IsRunning())
		return;

	// Avoid reentrancy problems with main window, options dialog and mini mule window
	// NEO: MLD - [ModelesDialogs] -- Xanatos --
	/*if (IsPreferencesDlgOpen()) {
		MessageBeep(MB_OK);
		preferenceswnd->SetForegroundWindow();
		preferenceswnd->BringWindowToTop();
		return;
	}*/

	if (m_pMiniMule)
	{
		if (m_pMiniMule->GetAutoClose())
		{
			TRACE("%s - m_pMiniMule->GetAutoClose() -> DestroyMiniMule();\n", __FUNCTION__);
			DestroyMiniMule();
		}
		else
		{
			// Avoid reentrancy problems with main window, options dialog and mini mule window
			if (m_pMiniMule->m_hWnd && !m_pMiniMule->IsWindowEnabled()) {
				MessageBeep(MB_OK);
				return;
			}
		}
	}

	if (m_pSystrayDlg) {
		m_pSystrayDlg->BringWindowToTop();
		return;
	}

	m_pSystrayDlg = new CMuleSystrayDlg(this, pt,
										thePrefs.GetMaxGraphUploadRate(true), thePrefs.GetMaxGraphDownloadRate(),
										thePrefs.GetMaxUpload(), thePrefs.GetMaxDownload());
	if (m_pSystrayDlg)
	{
		UINT nResult = m_pSystrayDlg->DoModal();
		delete m_pSystrayDlg;
		m_pSystrayDlg = NULL;
		switch (nResult)
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
				ShowPreferences();
				break;
		}
	}
}

void CemuleDlg::AddSpeedSelectorMenus(CMenu* addToMenu)
{
	CString text;

	// Create UploadPopup Menu
	ASSERT( m_menuUploadCtrl.m_hMenu == NULL );
	if (m_menuUploadCtrl.CreateMenu())
	{
		text.Format(_T("20%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphUploadRate(true)*0.2),GetResString(IDS_KBYTESPERSEC));	m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U20,  text);
		text.Format(_T("40%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphUploadRate(true)*0.4),GetResString(IDS_KBYTESPERSEC));	m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U40,  text);
		text.Format(_T("60%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphUploadRate(true)*0.6),GetResString(IDS_KBYTESPERSEC));	m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U60,  text);
		text.Format(_T("80%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphUploadRate(true)*0.8),GetResString(IDS_KBYTESPERSEC));	m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U80,  text);
		text.Format(_T("100%%\t%i %s"), (uint16)(thePrefs.GetMaxGraphUploadRate(true)),GetResString(IDS_KBYTESPERSEC));		m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U100, text);
		m_menuUploadCtrl.AppendMenu(MF_SEPARATOR);
		
		if (GetRecMaxUpload() > 0) {
			text.Format(GetResString(IDS_PW_MINREC) + GetResString(IDS_KBYTESPERSEC), GetRecMaxUpload());
			m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_UP10, text);
		}

		text.Format(_T("%s:"), GetResString(IDS_PW_UPL));
		addToMenu->AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)m_menuUploadCtrl.m_hMenu, text);
	}

	// Create DownloadPopup Menu
	ASSERT( m_menuDownloadCtrl.m_hMenu == NULL );
	if (m_menuDownloadCtrl.CreateMenu())
	{
		text.Format(_T("20%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphDownloadRate()*0.2),GetResString(IDS_KBYTESPERSEC));	m_menuDownloadCtrl.AppendMenu(MF_STRING|MF_POPUP, MP_QS_D20,  text);
		text.Format(_T("40%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphDownloadRate()*0.4),GetResString(IDS_KBYTESPERSEC));	m_menuDownloadCtrl.AppendMenu(MF_STRING|MF_POPUP, MP_QS_D40,  text);
		text.Format(_T("60%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphDownloadRate()*0.6),GetResString(IDS_KBYTESPERSEC));	m_menuDownloadCtrl.AppendMenu(MF_STRING|MF_POPUP, MP_QS_D60,  text);
		text.Format(_T("80%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphDownloadRate()*0.8),GetResString(IDS_KBYTESPERSEC));	m_menuDownloadCtrl.AppendMenu(MF_STRING|MF_POPUP, MP_QS_D80,  text);
		text.Format(_T("100%%\t%i %s"), (uint16)(thePrefs.GetMaxGraphDownloadRate()),GetResString(IDS_KBYTESPERSEC));		m_menuDownloadCtrl.AppendMenu(MF_STRING|MF_POPUP, MP_QS_D100, text);

		text.Format(_T("%s:"), GetResString(IDS_PW_DOWNL));
		addToMenu->AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)m_menuDownloadCtrl.m_hMenu, text);
	}

	addToMenu->AppendMenu(MF_SEPARATOR);
	addToMenu->AppendMenu(MF_STRING, MP_CONNECT, GetResString(IDS_MAIN_BTN_CONNECT));
	addToMenu->AppendMenu(MF_STRING, MP_DISCONNECT, GetResString(IDS_MAIN_BTN_DISCONNECT)); 
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
		}
		else{
			m_bConnectRequestDelayedForUPnP = false;
			if (m_hUPnPTimeOutTimer != 0){
				VERIFY( ::KillTimer(NULL, m_hUPnPTimeOutTimer) );
				m_hUPnPTimeOutTimer = 0;
			}
			AddLogLine(true, GetResString(IDS_CONNECTING));

			// ed2k
			if ((thePrefs.GetNetworkED2K() || m_bEd2kSuspendDisconnect) && !theApp.serverconnect->IsConnecting() && !theApp.serverconnect->IsConnected()) {
				theApp.serverconnect->ConnectToAnyServer();
			}

			// kad
			if ((thePrefs.GetNetworkKademlia() || m_bKadSuspendDisconnect) && !Kademlia::CKademlia::IsRunning()) {
				Kademlia::CKademlia::Start();
			}
		}

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
	theApp.OnlineSig(); // Added By Bouc7 
	ShowConnectionState();
}

void CemuleDlg::RestoreWindow()
{
	// NEO: TPP - [TrayPasswordProtection] -- Xanatos -->
	static bool oneIsOpen = false;
	if(oneIsOpen)
		return;
	if(m_TrayLocked){

		InputBox inputbox;
		inputbox.SetLabels(_T("Password"), _T("Enter Password"),_T(""));
		inputbox.SetPassword(true);
		oneIsOpen = true;
		if (inputbox.DoModal() != IDOK){
			oneIsOpen = false;
			return;
		}
		oneIsOpen = false;
		if(m_TrayPassword.Compare(inputbox.GetInput()) != 0)
			return;
		m_TrayLocked = false;
	}
	// NEO: TPP END <-- Xanatos --

	// NEO: MLD - [ModelesDialogs] -- Xanatos --
	/*if (IsPreferencesDlgOpen()) {
		MessageBeep(MB_OK);
		preferenceswnd->SetForegroundWindow();
		preferenceswnd->BringWindowToTop();
		return;
	}*/
	if (TrayIsVisible())
		TrayHide();

	DestroyMiniMule();

	if (m_wpFirstRestore.length)
	{
		SetWindowPlacement(&m_wpFirstRestore);
		memset(&m_wpFirstRestore, 0, sizeof m_wpFirstRestore);
		SetForegroundWindow();
		BringWindowToTop();
	}
	else
		CTrayDialog::RestoreWindow();
}

// NEO: NSTI - [NewSystemTrayIcon] -- Xanatos -->
void CemuleDlg::UpdateTrayIcon(int iDown, int iUp)
{
	// compute an id of the icon to be generated
	UINT uSysTrayIconCookie = iDown;
	if(NeoPrefs.IsShowSystemTrayUpload())
		uSysTrayIconCookie += iUp;
	if (theApp.IsConnected()) {
		if (!theApp.IsFirewalled())
			uSysTrayIconCookie += 50;
	}
	else
		uSysTrayIconCookie += 100;

	// dont update if the same icon as displayed would be generated
	if (m_uLastSysTrayIconCookie == uSysTrayIconCookie)
		return;
	m_uLastSysTrayIconCookie = uSysTrayIconCookie;

	// prepare it up
	if (m_iMsgIcon!=0 && thePrefs.DoFlashOnNewMessage()==true ) {
		m_iMsgBlinkState=!m_iMsgBlinkState;

		if (m_iMsgBlinkState)
			m_TrayIcon.Init(imicons[1], 16, NeoPrefs.IsShowSystemTrayUpload()?2:1, NeoPrefs.UseThinSystemTrayBars()?1:2, 16, 16, NULL);
	} else m_iMsgBlinkState=false;

	if (!m_iMsgBlinkState) {
		if (theApp.IsConnected()) {
			if (theApp.IsFirewalled())
				m_TrayIcon.Init(m_icoSysTrayLowID, 16, NeoPrefs.IsShowSystemTrayUpload()?2:1, NeoPrefs.UseThinSystemTrayBars()?1:2, 16, 16, NULL);
			else
				m_TrayIcon.Init(m_icoSysTrayConnected, 16, NeoPrefs.IsShowSystemTrayUpload()?2:1, NeoPrefs.UseThinSystemTrayBars()?1:2, 16, 16, NULL);
		}
		else
			m_TrayIcon.Init(m_icoSysTrayDisconnected, 16, NeoPrefs.IsShowSystemTrayUpload()?2:1, NeoPrefs.UseThinSystemTrayBars()?1:2, 16, 16, NULL);
	}

	// generate the icon (do *not* destroy that icon using DestroyIcon(), that's done in 'TrayUpdate')
	int aiVals[2] = {iDown, iUp};
	m_icoSysTrayCurrent = m_TrayIcon.Create(aiVals);
	ASSERT( m_icoSysTrayCurrent != NULL );
	if (m_icoSysTrayCurrent)
		TraySetIcon(m_icoSysTrayCurrent, true);
	TrayUpdate();
}

void CemuleDlg::UpdateTrayBarsColors()
{
	COLORREF aColors[16];
	COLORREF aColors2[16];

	COLORREF cDown1 = thePrefs.GetStatsColor(30); //max
	COLORREF cDown2 = thePrefs.GetStatsColor(31); //min
	COLORREF cUp1 = thePrefs.GetStatsColor(32); //max
	COLORREF cUp2 = thePrefs.GetStatsColor(33); //min

	for(int i=0; i<16; i++)
	{
		if(i+1 < NeoPrefs.GetTrayBarsMaxCollor())
		{
			aColors[i] = RGB(
				(GetRValue(cDown1)*i + GetRValue(cDown2)*(15-i))/15,
				(GetGValue(cDown1)*i + GetGValue(cDown2)*(15-i))/15,
				(GetBValue(cDown1)*i + GetBValue(cDown2)*(15-i))/15);
			aColors2[i] = RGB(
				(GetRValue(cUp1)*i + GetRValue(cUp2)*(15-i))/15,
				(GetGValue(cUp1)*i + GetGValue(cUp2)*(15-i))/15,
				(GetBValue(cUp1)*i + GetBValue(cUp2)*(15-i))/15);
		}
		else
		{
			aColors[i] = cDown1;
			aColors2[i] = cUp1;
		}
	}
	m_TrayIcon.SetColorLevels(aColors, aColors2, 16);
}
// NEO: NSTI END <-- Xanatos --

/*void CemuleDlg::UpdateTrayIcon(int iPercent)
{
	// compute an id of the icon to be generated
	UINT uSysTrayIconCookie = (iPercent > 0) ? (16 - ((iPercent*15/100) + 1)) : 0;
	if (theApp.IsConnected()) {
		if (!theApp.IsFirewalled())
			uSysTrayIconCookie += 50;
	}
	else
		uSysTrayIconCookie += 100;
	
	// dont update if the same icon as displayed would be generated
	if (m_uLastSysTrayIconCookie == uSysTrayIconCookie)
		return;
	m_uLastSysTrayIconCookie = uSysTrayIconCookie;

	// prepare it up
	if (m_iMsgIcon!=0 && thePrefs.DoFlashOnNewMessage()==true ) {
		m_iMsgBlinkState=!m_iMsgBlinkState;

		if (m_iMsgBlinkState)
			m_TrayIcon.Init(imicons[1], 100, 1, 1, 16, 16, thePrefs.GetStatsColor(11));
	} else m_iMsgBlinkState=false;

	if (!m_iMsgBlinkState) {
		if (theApp.IsConnected()) {
			if (theApp.IsFirewalled())
				m_TrayIcon.Init(m_icoSysTrayLowID, 100, 1, 1, 16, 16, thePrefs.GetStatsColor(11));
			else
				m_TrayIcon.Init(m_icoSysTrayConnected, 100, 1, 1, 16, 16, thePrefs.GetStatsColor(11));
		}
		else
			m_TrayIcon.Init(m_icoSysTrayDisconnected, 100, 1, 1, 16, 16, thePrefs.GetStatsColor(11));
	}

	// load our limit and color info
	static const int aiLimits[1] = { 100 }; // set the limits of where the bar color changes (low-high)
	COLORREF aColors[1] = { thePrefs.GetStatsColor(11) }; // set the corresponding color for each level
	m_TrayIcon.SetColorLevels(aiLimits, aColors, _countof(aiLimits));

	// generate the icon (do *not* destroy that icon using DestroyIcon(), that's done in 'TrayUpdate')
	int aiVals[1] = { iPercent };
	m_icoSysTrayCurrent = m_TrayIcon.Create(aiVals);
	ASSERT( m_icoSysTrayCurrent != NULL );
	if (m_icoSysTrayCurrent)
		TraySetIcon(m_icoSysTrayCurrent, true);
	TrayUpdate();
}*/

int CemuleDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	return CTrayDialog::OnCreate(lpCreateStruct);
}

void CemuleDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	if (IsRunning()){
		ShowTransferRate(true);

		if (bShow == TRUE && activewnd == chatwnd)
			chatwnd->chatselector.ShowChat();

	}
	CTrayDialog::OnShowWindow(bShow, nStatus);
}

void CemuleDlg::ShowNotifier(LPCTSTR pszText, int iMsgType, LPCTSTR pszLink, bool bForceSoundOFF)
{
	if (!notifierenabled)
		return;

	LPCTSTR pszSoundEvent = NULL;
	int iSoundPrio = 0;
	bool bShowIt = false;
	switch (iMsgType)
	{
		case TBN_CHAT:
            if (thePrefs.GetNotifierOnChat())
			{
				m_wndTaskbarNotifier->Show(pszText, iMsgType, pszLink);
				bShowIt = true;
				pszSoundEvent = _T("eMule_Chat");
				iSoundPrio = 1;
			}
			break;
		case TBN_DOWNLOADFINISHED:
            if (thePrefs.GetNotifierOnDownloadFinished())
			{
				m_wndTaskbarNotifier->Show(pszText, iMsgType, pszLink);
				bShowIt = true;
				pszSoundEvent = _T("eMule_DownloadFinished");
				iSoundPrio = 1;
				SendNotificationMail(iMsgType, pszText);
			}
			break;
		case TBN_DOWNLOADADDED:
            if (thePrefs.GetNotifierOnNewDownload())
			{
				m_wndTaskbarNotifier->Show(pszText, iMsgType, pszLink);
				bShowIt = true;
				pszSoundEvent = _T("eMule_DownloadAdded");
				iSoundPrio = 1;
			}
			break;
		case TBN_LOG:
            if (thePrefs.GetNotifierOnLog())
			{
				m_wndTaskbarNotifier->Show(pszText, iMsgType, pszLink);
				bShowIt = true;
				pszSoundEvent = _T("eMule_LogEntryAdded");
			}
			break;
		case TBN_IMPORTANTEVENT:
			if (thePrefs.GetNotifierOnImportantError())
			{
				m_wndTaskbarNotifier->Show(pszText, iMsgType, pszLink);
				bShowIt = true;
				pszSoundEvent = _T("eMule_Urgent");
				iSoundPrio = 1;
				SendNotificationMail(iMsgType, pszText);
			}
			break;

		case TBN_NEWVERSION:
		case TBN_NEWNEOVERSION: // NEO: NVC - [NeoVersionCheck] <-- Xanatos --
			if (thePrefs.GetNotifierOnNewVersion())
			{
				m_wndTaskbarNotifier->Show(pszText, iMsgType, pszLink);
				bShowIt = true;
				pszSoundEvent = _T("eMule_NewVersion");
				iSoundPrio = 1;
			}
			break;
		case TBN_NULL:
            m_wndTaskbarNotifier->Show(pszText, iMsgType, pszLink);
			bShowIt = true;
			break;
	}
	
	if (bShowIt && !bForceSoundOFF && thePrefs.GetNotifierSoundType() != ntfstNoSound)
	{
		bool bNotifiedWithAudio = false;
		if (thePrefs.GetNotifierSoundType() == ntfstSpeech)
			bNotifiedWithAudio = Speak(pszText);

		if (!bNotifiedWithAudio)
		{
			if (!thePrefs.GetNotifierSoundFile().IsEmpty())
			{
				PlaySound(thePrefs.GetNotifierSoundFile(), NULL, SND_FILENAME | SND_NOSTOP | SND_NOWAIT | SND_ASYNC);
			}
			else if (pszSoundEvent)
			{
				// use 'SND_NOSTOP' only for low priority events, otherwise the 'Log message' event may overrule a more important
				// event which is fired nearly at the same time.
				PlaySound(pszSoundEvent, NULL, SND_APPLICATION | SND_ASYNC | SND_NODEFAULT | SND_NOWAIT | ((iSoundPrio > 0) ? 0 : SND_NOSTOP));
			}
		}
	}
}

void CemuleDlg::LoadNotifier(CString configuration)
{
	notifierenabled = m_wndTaskbarNotifier->LoadConfiguration(configuration)!=FALSE;
}

LRESULT CemuleDlg::OnTaskbarNotifierClicked(WPARAM /*wParam*/, LPARAM lParam)
{
	if (lParam)
	{
		LPTSTR pszLink = (LPTSTR)lParam;
		ShellOpenFile(pszLink, NULL);
		free(pszLink);
		pszLink = NULL;
	}

	switch (m_wndTaskbarNotifier->GetMessageType())
	{
		case TBN_CHAT:
			RestoreWindow();
			SetActiveDialog(chatwnd);
			break;

		case TBN_DOWNLOADFINISHED:
			// if we had a link and opened the downloaded file, dont restore the app window
			if (lParam==0)
			{
				RestoreWindow();
				SetActiveDialog(transferwnd);
			}
			break;

		case TBN_DOWNLOADADDED:
			RestoreWindow();
			SetActiveDialog(transferwnd);
			break;

		case TBN_IMPORTANTEVENT:
			RestoreWindow();
			SetActiveDialog(serverwnd);	
			break;

		case TBN_LOG:
			RestoreWindow();
			SetActiveDialog(serverwnd);	
			break;

		case TBN_NEWVERSION:
		{
			CString theUrl;
			theUrl.Format(_T("/en/version_check.php?version=%i&language=%i"),theApp.m_uCurVersionCheck,thePrefs.GetLanguageID());
			theUrl = thePrefs.GetVersionCheckBaseURL()+theUrl;
			ShellExecute(NULL, NULL, theUrl, NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
			break;
		}
		// NEO: NVC - [NeoVersionCheck] -- Xanatos -->
		case TBN_NEWNEOVERSION:
		{
			ShellExecute(NULL, NULL, _T("http://sourceforge.net/projects/neomule/"), NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
			break;
		}
		// NEO: NVC END <-- Xanatos --
	}
    return 0;
}

void CemuleDlg::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	// Do not update the Shell's large icon size, because we still have an image list
	// from the shell which contains the old large icon size.
	//theApp.UpdateLargeIconSize();
	theApp.UpdateDesktopColorDepth();
	CTrayDialog::OnSettingChange(uFlags, lpszSection);
}

void CemuleDlg::OnSysColorChange()
{
	theApp.UpdateDesktopColorDepth();
	CTrayDialog::OnSysColorChange();
	SetAllIcons();
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
	connicons[0] = theApp.LoadIcon(_T("ConnectedNotNot"), 16, 16);
	connicons[1] = theApp.LoadIcon(_T("ConnectedNotLow"), 16, 16);
	connicons[2] = theApp.LoadIcon(_T("ConnectedNotHigh"), 16, 16);
	connicons[3] = theApp.LoadIcon(_T("ConnectedLowNot"), 16, 16);
	connicons[4] = theApp.LoadIcon(_T("ConnectedLowLow"), 16, 16);
	connicons[5] = theApp.LoadIcon(_T("ConnectedLowHigh"), 16, 16);
	connicons[6] = theApp.LoadIcon(_T("ConnectedHighNot"), 16, 16);
	connicons[7] = theApp.LoadIcon(_T("ConnectedHighLow"), 16, 16);
	connicons[8] = theApp.LoadIcon(_T("ConnectedHighHigh"), 16, 16);
	ShowConnectionStateIcon();

	// transfer state
	if (transicons[0]) VERIFY( ::DestroyIcon(transicons[0]) );
	if (transicons[1]) VERIFY( ::DestroyIcon(transicons[1]) );
	if (transicons[2]) VERIFY( ::DestroyIcon(transicons[2]) );
	if (transicons[3]) VERIFY( ::DestroyIcon(transicons[3]) );
	transicons[0] = theApp.LoadIcon(_T("UP0DOWN0"), 16, 16);
	transicons[1] = theApp.LoadIcon(_T("UP0DOWN1"), 16, 16);
	transicons[2] = theApp.LoadIcon(_T("UP1DOWN0"), 16, 16);
	transicons[3] = theApp.LoadIcon(_T("UP1DOWN1"), 16, 16);
	ShowTransferStateIcon();

	// users state
	if (usericon) VERIFY( ::DestroyIcon(usericon) );
	usericon = theApp.LoadIcon(_T("StatsClients"), 16, 16);
	ShowUserStateIcon();

	// traybar icons
	if (m_icoSysTrayConnected) VERIFY( ::DestroyIcon(m_icoSysTrayConnected) );
	if (m_icoSysTrayDisconnected) VERIFY( ::DestroyIcon(m_icoSysTrayDisconnected) );
	if (m_icoSysTrayLowID) VERIFY( ::DestroyIcon(m_icoSysTrayLowID) );
	m_icoSysTrayConnected = theApp.LoadIcon(_T("TrayConnected"), 16, 16);
	m_icoSysTrayDisconnected = theApp.LoadIcon(_T("TrayNotConnected"), 16, 16);
	m_icoSysTrayLowID = theApp.LoadIcon(_T("TrayLowID"), 16, 16);
	ShowTransferRate(true);

	if (imicons[0]) VERIFY( ::DestroyIcon(imicons[0]) );
	if (imicons[1]) VERIFY( ::DestroyIcon(imicons[1]) );
	if (imicons[2]) VERIFY( ::DestroyIcon(imicons[2]) );
	imicons[0] = NULL;
	imicons[1] = theApp.LoadIcon(_T("Message"), 16, 16);
	imicons[2] = theApp.LoadIcon(_T("MessagePending"), 16, 16);
	ShowMessageState(m_iMsgIcon);
}

void CemuleDlg::Localize()
{
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu)
	{
		VERIFY( pSysMenu->ModifyMenu(MP_ABOUTBOX, MF_BYCOMMAND | MF_STRING, MP_ABOUTBOX, GetResString(IDS_ABOUTBOX)) );
		VERIFY( pSysMenu->ModifyMenu(MP_NEOVERSIONCHECK, MF_BYCOMMAND | MF_STRING, MP_NEOVERSIONCHECK, GetResString(IDS_X_VERSIONCHECK)) ); // NEO: NVC - [NeoVersionCheck] <-- Xanatos --
		VERIFY( pSysMenu->ModifyMenu(MP_VERSIONCHECK, MF_BYCOMMAND | MF_STRING, MP_VERSIONCHECK, GetResString(IDS_VERSIONCHECK)) );

		switch (thePrefs.GetWindowsVersion())
		{
			case _WINVER_98_:
			case _WINVER_95_:
			case _WINVER_ME_:
				// NOTE: I think the reason why the old version of the following code crashed under Win9X was because
				// of the menus were destroyed right after they were added to the system menu. New code should work
				// under Win9X too but I can't test it.
				break;
			default:{
				// localize the 'speed control' sub menus by deleting the current menus and creating a new ones.

				// remove any already available 'speed control' menus from system menu
				UINT uOptMenuPos = pSysMenu->GetMenuItemCount() - 1;
				CMenu* pAccelMenu = pSysMenu->GetSubMenu(uOptMenuPos);
				if (pAccelMenu)
				{
					ASSERT( pAccelMenu->m_hMenu == m_SysMenuOptions.m_hMenu );
					VERIFY( pSysMenu->RemoveMenu(uOptMenuPos, MF_BYPOSITION) );
					pAccelMenu = NULL;
				}

				// destroy all 'speed control' menus
				if (m_menuUploadCtrl)
					VERIFY( m_menuUploadCtrl.DestroyMenu() );
				if (m_menuDownloadCtrl)
					VERIFY( m_menuDownloadCtrl.DestroyMenu() );
				if (m_SysMenuOptions)
					VERIFY( m_SysMenuOptions.DestroyMenu() );

				// create new 'speed control' menus
				if (m_SysMenuOptions.CreateMenu())
				{
					AddSpeedSelectorMenus(&m_SysMenuOptions);
					pSysMenu->AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)m_SysMenuOptions.m_hMenu, GetResString(IDS_EM_PREFS));
				}
			}
		}
	}

	ShowUserStateIcon();
	toolbar->Localize();
	ShowConnectionState();
	ShowTransferRate(true);
	ShowUserCount();
	CPartFileConvert::Localize();
	if (m_pMiniMule)
		m_pMiniMule->Localize();
}

void CemuleDlg::ShowUserStateIcon()
{
	statusbar->SetIcon(SBarUsers, usericon);
}

void CemuleDlg::QuickSpeedOther(UINT nID)
{
	switch (nID) {
		case MP_QS_PA: 
			thePrefs.SetMaxUpload(1);
			thePrefs.SetMaxDownload(1);
			break ;
		case MP_QS_UA: 
			thePrefs.SetMaxUpload(thePrefs.GetMaxGraphUploadRate(true));
			thePrefs.SetMaxDownload(thePrefs.GetMaxGraphDownloadRate());
			break ;
	}
}


void CemuleDlg::QuickSpeedUpload(UINT nID)
{
	switch (nID) {
		case MP_QS_U10: thePrefs.SetMaxUpload((UINT)(thePrefs.GetMaxGraphUploadRate(true)*0.1)); break ;
		case MP_QS_U20: thePrefs.SetMaxUpload((UINT)(thePrefs.GetMaxGraphUploadRate(true)*0.2)); break ;
		case MP_QS_U30: thePrefs.SetMaxUpload((UINT)(thePrefs.GetMaxGraphUploadRate(true)*0.3)); break ;
		case MP_QS_U40: thePrefs.SetMaxUpload((UINT)(thePrefs.GetMaxGraphUploadRate(true)*0.4)); break ;
		case MP_QS_U50: thePrefs.SetMaxUpload((UINT)(thePrefs.GetMaxGraphUploadRate(true)*0.5)); break ;
		case MP_QS_U60: thePrefs.SetMaxUpload((UINT)(thePrefs.GetMaxGraphUploadRate(true)*0.6)); break ;
		case MP_QS_U70: thePrefs.SetMaxUpload((UINT)(thePrefs.GetMaxGraphUploadRate(true)*0.7)); break ;
		case MP_QS_U80: thePrefs.SetMaxUpload((UINT)(thePrefs.GetMaxGraphUploadRate(true)*0.8)); break ;
		case MP_QS_U90: thePrefs.SetMaxUpload((UINT)(thePrefs.GetMaxGraphUploadRate(true)*0.9)); break ;
		case MP_QS_U100: thePrefs.SetMaxUpload((UINT)thePrefs.GetMaxGraphUploadRate(true)); break ;
//		case MP_QS_UPC: thePrefs.SetMaxUpload(UNLIMITED); break ;
		case MP_QS_UP10: thePrefs.SetMaxUpload(GetRecMaxUpload()); break ;
	}
}

void CemuleDlg::QuickSpeedDownload(UINT nID)
{
	switch (nID) {
		case MP_QS_D10: thePrefs.SetMaxDownload((UINT)(thePrefs.GetMaxGraphDownloadRate()*0.1)); break ;
		case MP_QS_D20: thePrefs.SetMaxDownload((UINT)(thePrefs.GetMaxGraphDownloadRate()*0.2)); break ;
		case MP_QS_D30: thePrefs.SetMaxDownload((UINT)(thePrefs.GetMaxGraphDownloadRate()*0.3)); break ;
		case MP_QS_D40: thePrefs.SetMaxDownload((UINT)(thePrefs.GetMaxGraphDownloadRate()*0.4)); break ;
		case MP_QS_D50: thePrefs.SetMaxDownload((UINT)(thePrefs.GetMaxGraphDownloadRate()*0.5)); break ;
		case MP_QS_D60: thePrefs.SetMaxDownload((UINT)(thePrefs.GetMaxGraphDownloadRate()*0.6)); break ;
		case MP_QS_D70: thePrefs.SetMaxDownload((UINT)(thePrefs.GetMaxGraphDownloadRate()*0.7)); break ;
		case MP_QS_D80: thePrefs.SetMaxDownload((UINT)(thePrefs.GetMaxGraphDownloadRate()*0.8)); break ;
		case MP_QS_D90: thePrefs.SetMaxDownload((UINT)(thePrefs.GetMaxGraphDownloadRate()*0.9)); break ;
		case MP_QS_D100: thePrefs.SetMaxDownload((UINT)thePrefs.GetMaxGraphDownloadRate()); break ;
//		case MP_QS_DC: thePrefs.SetMaxDownload(UNLIMITED); break ;
	}
}
// quick-speed changer -- based on xrmb

int CemuleDlg::GetRecMaxUpload() {
	
	if (thePrefs.GetMaxGraphUploadRate(true)<7) return 0;
	if (thePrefs.GetMaxGraphUploadRate(true)<15) return thePrefs.GetMaxGraphUploadRate(true)-3;
	return (thePrefs.GetMaxGraphUploadRate(true)-4);

}

BOOL CemuleDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{	
		case TBBTN_CONNECT:
			OnBnClickedButton2();
			break;
		case MP_HM_KAD:
		case TBBTN_KAD:
			SetActiveDialog(kademliawnd);
			break;
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
		case TBBTN_SHARED:
		case MP_HM_FILES:
			SetActiveDialog(sharedfileswnd);
			break;
		case TBBTN_MESSAGES:
		case MP_HM_MSGS:
			SetActiveDialog(chatwnd);
			break;
		case TBBTN_IRC:
		case MP_HM_IRC:
			SetActiveDialog(ircwnd);
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
		case TBBTN_TOOLS:
			ShowToolPopup(true);
			break;
		case MP_HM_OPENINC:
			ShellExecute(NULL, _T("open"), thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR),NULL, NULL, SW_SHOW); 
			break;
		case MP_HM_HELP:
		case TBBTN_HELP:
			if (activewnd != NULL) {
				HELPINFO hi;
				ZeroMemory(&hi, sizeof(HELPINFO));
				hi.cbSize = sizeof(HELPINFO);
				activewnd->SendMessage(WM_HELP, 0, (LPARAM)&hi);
			}
			else
				wParam = ID_HELP;
			break;
		case MP_HM_CON:
			OnBnClickedButton2();
			break;
		case MP_HM_EXIT:
			OnClose();
			break;
		// NEO: NV - [NeoVersion] -- Xanatos -->
		case MP_X_LINK1: 
			ShellExecute(NULL, NULL, _T("http://NeoMule.sf.net"), NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
			break;
		case MP_X_LINK2: 
			ShellExecute(NULL, NULL, _T("http://sourceforge.net/projects/neomule/"), NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
			break;
		case MP_X_LINK3:
			ShellExecute(NULL, NULL, _T("http://eselfarm.info/efarm/board.php?boardid=277"), NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
			break;
		// NEO: NV END <-- Xanatos --
		case MP_HM_LINK1: // MOD: dont remove!
			ShellExecute(NULL, NULL, thePrefs.GetHomepageBaseURL(), NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
			break;
		case MP_HM_LINK2:
			ShellExecute(NULL, NULL, thePrefs.GetHomepageBaseURL()+ CString(_T("/faq/")), NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
			break;
		case MP_HM_LINK3: {
			CString theUrl;
			theUrl.Format( thePrefs.GetVersionCheckBaseURL() + CString(_T("/en/version_check.php?version=%i&language=%i")),theApp.m_uCurVersionCheck,thePrefs.GetLanguageID());
			ShellExecute(NULL, NULL, theUrl, NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
			break;
		}
		case MP_WEBSVC_EDIT:
			theWebServices.Edit();
			break;
		case MP_HM_CONVERTPF:
			CPartFileConvert::ShowGUI();
			break;
		case MP_HM_SCHEDONOFF:
			thePrefs.SetSchedulerEnabled(!thePrefs.IsSchedulerEnabled());
			theApp.scheduler->Check(true);
			break;
		case MP_HM_1STSWIZARD:
			extern BOOL FirstTimeWizard();
			if (FirstTimeWizard()){
				// start connection wizard
				CConnectionWizardDlg conWizard;
				conWizard.DoModal();
			}
			break;
		case MP_HM_IPFILTER:{
			// NEO: MLD - [ModelesDialogs] -- Xanatos -->
			CIPFilterDlg* dialog = new CIPFilterDlg(); 
			dialog->OpenDialog();
			// NEO: MLD END <-- Xanatos --
			//CIPFilterDlg dlg;
			//dlg.DoModal();
			break;
		}
		case MP_HM_DIRECT_DOWNLOAD:{
			// NEO: MLD - [ModelesDialogs] -- Xanatos -->
			CDirectDownloadDlg* dialog = new CDirectDownloadDlg(); 
			dialog->OpenDialog();
			// NEO: MLD END <-- Xanatos --
			//CDirectDownloadDlg dlg;
			//dlg.DoModal();
			break;
		}
		// NEO: QS - [QuickStart] -- Xanatos -->
		case MP_HM_QUICKSTART:
			if(!NeoPrefs.OnQuickStart())
				theApp.downloadqueue->DoQuickStart();
			else
				theApp.downloadqueue->StopQuickStart();
			break;
		// NEO: QS END <-- Xanatos --
		// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
		case MP_MTD_LOAD:{

			CFileDialog dlg(TRUE, _T("part.met"), NULL, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT, _T("Temp Files (*.part.met)|*.part.met||"), NULL, 0);
			CString newName;
			dlg.m_ofn.lpstrFile = newName.GetBuffer(_MAX_PATH);
			if(dlg.DoModal()!=IDOK){
				newName.ReleaseBuffer();
				break;
			}
			newName.ReleaseBuffer();

			LPWSTR szName = newName.GetBuffer();
			CString Path;
			CStringList Files;
			LPWSTR curName = 0;
			for(int i=0;i<newName.GetAllocLength();i++)
			{
				if(_T('\0')==szName[i] && _T('\0')!=szName[i+1]){
					if(curName)
						Files.AddTail(curName);
					else
						Path = szName;
					curName = szName+i+1;
				}
			}
			newName.ReleaseBuffer();
			if(Files.IsEmpty()){
				Files.AddTail(newName.Right(newName.GetLength() - newName.ReverseFind(_T('\\'))-1));
				Path = newName.Left(newName.ReverseFind(_T('\\'))+1);
			}

			for (POSITION pos = Files.GetHeadPosition();pos != 0;){
				CString Name = Files.GetNext(pos);
				CPartFile* toadd = new CPartFile();
				if (toadd->LoadPartFile(Path,Name)){
					theApp.downloadqueue->GetFileList()->AddTail(toadd);
					//if (toadd->GetStatus(true) == PS_READY) // NEO: SAFS - [ShowAllFilesInShare]
						theApp.sharedfiles->SafeAddKFile(toadd); // part files are always shared files
					theApp.emuledlg->transferwnd->downloadlistctrl.AddFile(toadd);// show in downloadwindow
					// NEO: PP - [PasswordProtection]
					if (toadd->IsPWProtHidden()) 
						theApp.emuledlg->transferwnd->downloadlistctrl.HideFile(toadd);
					// NEO: PP END
				}else{ 
					delete toadd;
				}
			}
			break;
		}
		case MP_MTD_LOADDIR:{
			LPMALLOC pMalloc = NULL;
			if (SHGetMalloc(&pMalloc) == NOERROR)
			{
				// buffer - a place to hold the file system pathname
				TCHAR buffer[MAX_PATH];

				// This struct holds the various options for the dialog
				BROWSEINFO bi;
				bi.hwndOwner = this->m_hWnd;
				bi.pidlRoot = NULL;
				bi.pszDisplayName = buffer;
				CString title=GetResString(IDS_IMP_SELFOLDER);
				bi.lpszTitle = title.GetBuffer(title.GetLength());
				bi.ulFlags =  BIF_EDITBOX | BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON | BIF_SHAREABLE ;
				bi.lpfn = NULL;

				// Now cause the dialog to appear.
				LPITEMIDLIST pidlRoot;
				if((pidlRoot = SHBrowseForFolder(&bi)) != NULL)
				{
					//
					// Again, almost undocumented.  How to get a ASCII pathname
					// from the LPITEMIDLIST struct.  I guess you just have to
					// "know" this stuff.
					//
					if(SHGetPathFromIDList(pidlRoot, buffer)){
						// Do something with the converted string.
						theApp.downloadqueue->Init(CString(buffer));
					}

					// Free the returned item identifier list using the
					// shell's task allocator!Arghhhh.
					pMalloc->Free(pidlRoot);
				}
				pMalloc->Release();
			}
			break;
		}
		// NEO: MTD END <-- Xanatos --
		// NEO: SSF - [ShareSingleFiles] -- Xanatos -->
		case MP_SHARE_FILE:{

			CFileDialog dlg(TRUE, _T("*.*"), NULL, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT, _T("All Files (*.*)|*.*||"), NULL, 0);
			CString newName;
			dlg.m_ofn.lpstrFile = newName.GetBuffer(_MAX_PATH);
			if(dlg.DoModal()!=IDOK){
				newName.ReleaseBuffer();
				break;
			}
			newName.ReleaseBuffer();

			LPWSTR szName = newName.GetBuffer();
			CString Path;
			CStringList Files;
			LPWSTR curName = 0;
			for(int i=0;i<newName.GetAllocLength();i++)
			{
				if(_T('\0')==szName[i] && _T('\0')!=szName[i+1]){
					if(curName)
						Files.AddTail(curName);
					else
						Path = szName;
					curName = szName+i+1;
				}
			}
			newName.ReleaseBuffer();
			if(Files.IsEmpty()){
				Files.AddTail(newName.Right(newName.GetLength() - newName.ReverseFind(_T('\\'))-1));
				Path = newName.Left(newName.ReverseFind(_T('\\'))+1);
			}

			for (POSITION pos = Files.GetHeadPosition();pos != 0;){
				CString sFile = MkPath(Path,Files.GetNext(pos));

				bool dontadd = false;
				for (POSITION pos = thePrefs.sharedfile_list.GetHeadPosition(); pos != 0; ) {
					if(thePrefs.sharedfile_list.GetNext(pos).CompareNoCase(sFile) == 0)
					{
						dontadd = true;
						break;
					}
				}
				if(dontadd)
					continue;

				thePrefs.sharedfile_list.AddTail(sFile);
			}

			theApp.sharedfiles->Reload();
			break;
		}
		// NEO: SSF END <-- Xanatos --
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
		case TBBTN_VOODOO:
		case MP_HM_VOODOOLIST:{
			// NEO: MLD - [ModelesDialogs]
			CVoodooListDlg* dlg = new CVoodooListDlg();
			dlg->OpenDialog(); 
			// NEO: MLD END
			//CVoodooListDlg dlg;
			//dlg.DoModal();
			break;
		}
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
		// NEO: XSF - [ExtendedSharedFiles] -- Xanatos -->
		case MP_LOCAL_FILES:
			theApp.sharedfiles->ShowLocalFilesDialog();
			break;
		// NEO: XSF END <-- Xanatos --
#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase] -- Xanatos -->
		case MP_HM_SOURCELIST:{
			// NEO: MLD - [ModelesDialogs]
			CSourceListDlg* dlg = new CSourceListDlg();
			dlg->OpenDialog(); 
			// NEO: MLD END
			//CSourceListDlg dlg; 
			//dlg.DoModal();
			break;
		}
#endif // NEO_CD // NEO: NCD END <-- Xanatos --
		// NEO: TPP - [TrayPasswordProtection] -- Xanatos -->
		case MP_HM_TRAY_LOCL:
			ShowWindow(-1);
			break;
		// NEO: TPP END <-- Xanatos --
	}	
	// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
	if(wParam==MP_TEMPAUTO){
		NeoPrefs.SetUsedTempDir(AUTO_TEMPDIR);
	}else
	if (wParam>=MP_TEMPLIST && wParam<=MP_TEMPLIST+99) {
		NeoPrefs.SetUsedTempDir(wParam-MP_TEMPLIST);
	}else
	// NEO: MTD END <-- Xanatos --
	if (wParam>=MP_WEBURL && wParam<=MP_WEBURL+99) {
		theWebServices.RunURL(NULL, wParam);
	}
	else if (wParam>=MP_SCHACTIONS && wParam<=MP_SCHACTIONS+99) {
		theApp.scheduler->ActivateSchedule(wParam-MP_SCHACTIONS);
		theApp.scheduler->SaveOriginals(); // use the new settings as original
	}

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

// NEO: SSH - [SlugFillerSafeHash] -- Xanatos -->
bool CemuleDlg::IsMainThread()
{
	// this checks if a function is called form the main thread
	return (GetCurrentThreadId() == _uMainThreadId);
}
// NEO: SSH END <-- Xanatos --

void CemuleDlg::OnBnClickedHotmenu()
{
	ShowToolPopup(false);
}

void CemuleDlg::ShowToolPopup(bool toolsonly)
{
	POINT point;
	::GetCursorPos(&point);

	//CTitleMenu menu;
	CMenuXP menu; // NEO: NMX - [NeoMenuXP] <-- Xanatos --
	menu.CreatePopupMenu();
	if (!toolsonly)
		menu.AddMenuTitle(GetResString(IDS_HOTMENU), true);
	else
		menu.AddMenuTitle(GetResString(IDS_TOOLS), true);

	CTitleMenu Links;
	Links.CreateMenu();
	Links.AddMenuTitle(NULL, true);
	// NEO: NV - [NeoVersion] -- Xanatos -->
	Links.AppendMenu(MF_STRING,MP_X_LINK1, GetResString(IDS_X_LINKHP), _T("WEB"));
	Links.AppendMenu(MF_STRING,MP_X_LINK2, GetResString(IDS_X_LINKVC), _T("WEB"));	
	Links.AppendMenu(MF_STRING,MP_X_LINK3, GetResString(IDS_X_LINKFORUM), _T("WEB"));	
	Links.AppendMenu(MF_SEPARATOR);
	// NEO: NV END <-- Xanatos --
	Links.AppendMenu(MF_STRING, MP_HM_LINK1, GetResString(IDS_HM_LINKHP), _T("WEB"));
	Links.AppendMenu(MF_STRING, MP_HM_LINK2, GetResString(IDS_HM_LINKFAQ), _T("WEB"));
	Links.AppendMenu(MF_STRING, MP_HM_LINK3, GetResString(IDS_HM_LINKVC), _T("WEB"));
	theWebServices.GetGeneralMenuEntries(&Links);
	//Links.InsertMenu(3, MF_BYPOSITION | MF_SEPARATOR);
	Links.InsertMenu(7, MF_BYPOSITION | MF_SEPARATOR); // NEO: NV - [NeoVersion] <-- Xanatos --
	Links.AppendMenu(MF_STRING, MP_WEBSVC_EDIT, GetResString(IDS_WEBSVEDIT));

	CMenu scheduler;
	scheduler.CreateMenu();
	CString schedonoff= (!thePrefs.IsSchedulerEnabled())?GetResString(IDS_HM_SCHED_ON):GetResString(IDS_HM_SCHED_OFF);

	scheduler.AppendMenu(MF_STRING,MP_HM_SCHEDONOFF, schedonoff);
	if (theApp.scheduler->GetCount()>0) {
		scheduler.AppendMenu(MF_SEPARATOR);
		for (UINT i=0; i<theApp.scheduler->GetCount();i++)
			scheduler.AppendMenu(MF_STRING,MP_SCHACTIONS+i, theApp.scheduler->GetSchedule(i)->title);
	}

	if (!toolsonly) {
		if (theApp.serverconnect->IsConnected())
			menu.AppendMenu(MF_STRING,MP_HM_CON,GetResString(IDS_MAIN_BTN_DISCONNECT), _T("DISCONNECT"));
		else if (theApp.serverconnect->IsConnecting())
			menu.AppendMenu(MF_STRING,MP_HM_CON,GetResString(IDS_MAIN_BTN_CANCEL), _T("STOPCONNECTING"));
		else
			menu.AppendMenu(MF_STRING,MP_HM_CON,GetResString(IDS_MAIN_BTN_CONNECT), _T("CONNECT"));

		menu.AppendMenu(MF_STRING,MP_HM_KAD, GetResString(IDS_EM_KADEMLIA), _T("KADEMLIA") );
		menu.AppendMenu(MF_STRING,MP_HM_SRVR, GetResString(IDS_EM_SERVER), _T("SERVER") );
		menu.AppendMenu(MF_STRING,MP_HM_TRANSFER, GetResString(IDS_EM_TRANS),_T("TRANSFER") );
		menu.AppendMenu(MF_STRING,MP_HM_SEARCH, GetResString(IDS_EM_SEARCH), _T("SEARCH"));
		menu.AppendMenu(MF_STRING,MP_HM_FILES, GetResString(IDS_EM_FILES), _T("SharedFiles"));
		menu.AppendMenu(MF_STRING,MP_HM_MSGS, GetResString(IDS_EM_MESSAGES), _T("MESSAGES"));
		menu.AppendMenu(MF_STRING,MP_HM_IRC, GetResString(IDS_IRC), _T("IRC"));
		menu.AppendMenu(MF_STRING,MP_HM_STATS, GetResString(IDS_EM_STATISTIC), _T("STATISTICS"));
		menu.AppendMenu(MF_STRING,MP_HM_PREFS, GetResString(IDS_EM_PREFS), _T("PREFERENCES"));
		menu.AppendMenu(MF_STRING,MP_HM_HELP, GetResString(IDS_EM_HELP), _T("HELP"));
		menu.AppendMenu(MF_SEPARATOR);
	}

	menu.AppendMenu(MF_STRING,MP_HM_OPENINC, GetResString(IDS_OPENINC) + _T("..."), _T("INCOMING"));
	menu.AppendMenu(MF_STRING,MP_HM_CONVERTPF, GetResString(IDS_IMPORTSPLPF) + _T("..."), _T("CONVERT"));
	menu.AppendMenu(MF_STRING,MP_HM_1STSWIZARD, GetResString(IDS_WIZ1) + _T("..."), _T("WIZARD"));
	menu.AppendMenu(MF_STRING,MP_HM_IPFILTER, GetResString(IDS_IPFILTER) + _T("..."), _T("IPFILTER"));
	menu.AppendMenu(MF_STRING,MP_HM_DIRECT_DOWNLOAD, GetResString(IDS_SW_DIRECTDOWNLOAD) + _T("..."), _T("PASTELINK"));
	menu.AppendMenu(MF_SEPARATOR); // NEO: MOD <-- Xanatos --
	menu.AppendMenu(MF_STRING,MP_LOCAL_FILES, GetResString(IDS_X_LOCAL_FILES) + _T("..."), _T("VIRTUALDIR")); // NEO: XSF - [ExtendedSharedFiles] <-- Xanatos --
	menu.AppendMenu(MF_STRING,MP_HM_TRAY_LOCL, GetResString(IDS_X_LOCK_TRAY), _T("SECURITY")); // NEO: TPP - [TrayPasswordProtection] <-- Xanatos --
	// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
	CTitleMenu TempDirMenu;
	TempDirMenu.CreateMenu();
	TempDirMenu.AddMenuTitle(NULL, true);
	TempDirMenu.AppendMenu(MF_STRING,MP_MTD_LOAD,GetResString(IDS_X_MTD_LOAD), _T("MTD_LOAD"));
	TempDirMenu.AppendMenu(MF_STRING,MP_MTD_LOADDIR,GetResString(IDS_X_MTD_LOADDIR), _T("MTD_LOADDIR"));
	TempDirMenu.AppendMenu(MF_SEPARATOR);
	CTitleMenu TempDefMenu;
	TempDefMenu.CreateMenu();
	TempDefMenu.AddMenuTitle(NULL, true);
	UpdateMTDMenu(TempDefMenu,NeoPrefs.GetUsedTempDir() != AUTO_TEMPDIR ? thePrefs.GetTempDir(NeoPrefs.GetUsedTempDir()) : _T(""), false);
	TempDirMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)TempDefMenu.m_hMenu, GetResString(IDS_X_MTD_SELECT), _T("MTD_SELECT"));
	menu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)TempDirMenu.m_hMenu, GetResString(IDS_X_TEMPDIRMENUTITLE), _T("TEMPDIRS"));
	// NEO: MTD END <-- Xanatos --
	menu.AppendMenu(MF_STRING,MP_SHARE_FILE,GetResString(IDS_X_SHARE_FILE), _T("SHAREFILE")); // NEO: SSF - [ShareSingleFiles] <-- Xanatos --
	menu.AppendMenu(MF_STRING,MP_HM_QUICKSTART,GetResString(NeoPrefs.OnQuickStart() ? IDS_X_QUICK_START_STOP : IDS_X_QUICK_START_START), _T("QUICKSTART")); // NEO: QS - [QuickStart] <-- Xanatos --
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	menu.AppendMenu(MF_STRING,MP_HM_VOODOOLIST, GetResString(IDS_X_VOODOO_LIST), _T("VOODOOLIST"));
	menu.EnableMenuItem(MP_HM_VOODOOLIST, NeoPrefs.IsVoodooEnabled() ? MF_ENABLED : MF_GRAYED); 
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase] -- Xanatos -->
	menu.AppendMenu(MF_STRING,MP_HM_SOURCELIST, GetResString(IDS_X_SOURCE_DATABASE), _T("SOURCEDATABASE"));
	menu.EnableMenuItem(MP_HM_SOURCELIST, NeoPrefs.EnableSourceList() ? MF_ENABLED : MF_GRAYED); 
#endif // NEO_CD // NEO: NCD END <-- Xanatos --

	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)Links.m_hMenu, GetResString(IDS_LINKS), _T("WEB") );
	menu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)scheduler.m_hMenu, GetResString(IDS_SCHEDULER), _T("SCHEDULER") );

	if (!toolsonly) {
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING,MP_HM_EXIT, GetResString(IDS_EXIT), _T("EXIT"));
	}
	menu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
	VERIFY( Links.DestroyMenu() );
	VERIFY( scheduler.DestroyMenu() );
	VERIFY( TempDefMenu.DestroyMenu() );
	VERIFY( TempDirMenu.DestroyMenu() );
	// NEO: MTD END <-- Xanatos --
	VERIFY( menu.DestroyMenu() );
}


void CemuleDlg::ApplyHyperTextFont(LPLOGFONT plf)
{
	theApp.m_fontHyperText.DeleteObject();
	if (theApp.m_fontHyperText.CreateFontIndirect(plf))
	{
		thePrefs.SetHyperTextFont(plf);
		serverwnd->servermsgbox->SetFont(&theApp.m_fontHyperText);
		chatwnd->chatselector.UpdateFonts(&theApp.m_fontHyperText);
		ircwnd->UpdateFonts(&theApp.m_fontHyperText);
	}
}

void CemuleDlg::ApplyLogFont(LPLOGFONT plf)
{
	theApp.m_fontLog.DeleteObject();
	if (theApp.m_fontLog.CreateFontIndirect(plf))
	{
		thePrefs.SetLogFont(plf);
		serverwnd->logbox->SetFont(&theApp.m_fontLog);
		serverwnd->modlogbox->SetFont(&theApp.m_fontLog); // NEO: ML - [ModLog] <-- Xanatos --
		serverwnd->debuglog->SetFont(&theApp.m_fontLog);
	}
}

LRESULT CemuleDlg::OnFrameGrabFinished(WPARAM wParam,LPARAM lParam){
	CKnownFile* pOwner = (CKnownFile*)wParam;
	FrameGrabResult_Struct* result = (FrameGrabResult_Struct*)lParam;
	
	if (theApp.knownfiles->IsKnownFile(pOwner) || theApp.downloadqueue->IsPartFile(pOwner) ){
		pOwner->GrabbingFinished(result->imgResults,result->nImagesGrabbed, result->pSender);
	}
	else{
		ASSERT ( false );
	}

	delete result;
	return 0;
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

void ApplySystemFont(CWnd* pWnd)
{
	CWnd* pWndChild = pWnd->GetWindow(GW_CHILD);
	while (pWndChild)
	{
		ApplySystemFont(pWndChild);
		pWndChild = pWndChild->GetNextWindow();
	}

	CHAR szClassName[MAX_PATH];
	if (::GetClassNameA(*pWnd, szClassName, _countof(szClassName)))
	{
		if (   __ascii_stricmp(szClassName, "SysListView32") == 0
			|| __ascii_stricmp(szClassName, "SysTreeView32") == 0)
		{
			pWnd->SendMessage(WM_SETFONT, NULL, FALSE);
		}
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
	//ApplySystemFont(pWnd);
	if (thePrefs.GetStraightWindowStyles() < 0)
		return;
	else if (thePrefs.GetStraightWindowStyles() > 0)
		/*StraightWindowStyles(pWnd)*/;	// no longer needed
	else
	{
		s_bIsXPStyle = g_xpStyle.IsAppThemed() && g_xpStyle.IsThemeActive();
		if (!s_bIsXPStyle)
			FlatWindowStyles(pWnd);
	}
}

#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->
LRESULT CemuleDlg::OnArgosResult(WPARAM wParam, LPARAM lParam)
{
	TArgosResult* ArgosResult = (TArgosResult*) wParam;
	uint32 IP = (uint32) lParam;
	
	CUpDownClient* Leecher = theApp.clientlist->FindClientByIP(IP);
	if(Leecher){
		if(ArgosResult->GPLBreaker && NeoPrefs.ZeroScoreGPLBreaker()){
			if (thePrefs.GetLogBannedClients())
				AddDebugLogLine(false, _T("Clients: %s (%s), 0-Scorereason: DLP; %s"), Leecher->GetUserName(), ipstr(Leecher->GetConnectIP()), ArgosResult->Comment);
			Leecher->SetGPLEvilDoer();
		}else{
			if (thePrefs.GetLogBannedClients())
				AddDebugLogLine(false, _T("Clients: %s (%s), Banreason: DLP; %s"), Leecher->GetUserName(), ipstr(Leecher->GetConnectIP()), ArgosResult->Comment);
			Leecher->Ban(ArgosResult->Comment);
		}
	}else{
		if (thePrefs.GetLogBannedClients())
			AddDebugLogLine(false, _T("Clients: %s, Banreason: DLP; %s"), ipstr(IP), ArgosResult->Comment);
		theApp.clientlist->AddBannedClient(IP);
	}

	delete ArgosResult;	
	return 0;
}
#endif // ARGOS // NEO: NA END <-- Xanatos --

LRESULT CemuleDlg::OnVersionCheckResponse(WPARAM /*wParam*/, LPARAM lParam)
{
	if (WSAGETASYNCERROR(lParam) == 0)
	{
		int iBufLen = WSAGETASYNCBUFLEN(lParam);
		if (iBufLen >= sizeof(HOSTENT))
		{
			LPHOSTENT pHost = (LPHOSTENT)m_acVCDNSBuffer;
			if (pHost->h_length == 4 && pHost->h_addr_list && pHost->h_addr_list[0])
			{
				uint32 dwResult = ((LPIN_ADDR)(pHost->h_addr_list[0]))->s_addr;		
				// last byte contains informations about mirror urls, to avoid effects of future DDoS Attacks against eMules Homepage
				thePrefs.SetWebMirrorAlertLevel((uint8)(dwResult >> 24));
				uint8 abyCurVer[4] = { (uint8)(CemuleApp::m_nVersionBld + 1), (uint8)(CemuleApp::m_nVersionUpd), (uint8)(CemuleApp::m_nVersionMin), (uint8)0};
				dwResult &= 0x00FFFFFF;
				if (dwResult > *(uint32*)abyCurVer){
					SetActiveWindow();
#ifndef _BETA
					Log(LOG_SUCCESS|LOG_STATUSBAR,GetResString(IDS_NEWVERSIONAVL));
					ShowNotifier(GetResString(IDS_NEWVERSIONAVLPOPUP), TBN_NEWVERSION);
					thePrefs.UpdateLastVC();
					if (!thePrefs.GetNotifierOnNewVersion()){
						if (AfxMessageBox(GetResString(IDS_NEWVERSIONAVL)+GetResString(IDS_VISITVERSIONCHECK),MB_YESNO)==IDYES) {
							CString theUrl;
							theUrl.Format(_T("/en/version_check.php?version=%i&language=%i"),theApp.m_uCurVersionCheck,thePrefs.GetLanguageID());
							theUrl = thePrefs.GetVersionCheckBaseURL()+theUrl;
							ShellExecute(NULL, NULL, theUrl, NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
						}
					}
#else
					Log(LOG_SUCCESS|LOG_STATUSBAR,GetResString(IDS_NEWVERSIONAVLBETA));
					if (AfxMessageBox(GetResString(IDS_NEWVERSIONAVLBETA)+GetResString(IDS_VISITVERSIONCHECK),MB_OK)==IDOK) {
						CString theUrl;
						theUrl.Format(_T("/en/download.php?version=%i&language=%i"),theApp.m_uCurVersionCheck,thePrefs.GetLanguageID());
						theUrl = thePrefs.GetVersionCheckBaseURL()+theUrl;
						ShellExecute(NULL, NULL, theUrl, NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
					}
#endif
				}
				else{
					thePrefs.UpdateLastVC();
					AddLogLine(true,GetResString(IDS_NONEWERVERSION));
				}
				return 0;
			}
		}
	}
	LogWarning(LOG_STATUSBAR,GetResString(IDS_NEWVERSIONFAILED));
	return 0;
}

// NEO: NVC - [NeoVersionCheck] -- Xanatos -->
LRESULT CemuleDlg::OnNeoVersionCheckResponse(WPARAM /*wParam*/, LPARAM lParam)
{
	if (WSAGETASYNCERROR(lParam) == 0)
	{
		int iBufLen = WSAGETASYNCBUFLEN(lParam);
		if (iBufLen >= sizeof(HOSTENT))
		{
			LPHOSTENT pHost = (LPHOSTENT)m_acNVCDNSBuffer;
			if (pHost->h_length == 4 && pHost->h_addr_list && pHost->h_addr_list[0])
			{
				uint32 dwResult = ((LPIN_ADDR)(pHost->h_addr_list[0]))->s_addr;
				uint8 abyCurVer[4] = { MOD_VERSION_TERTIARY, MOD_VERSION_SECUNDARY, MOD_VERSION_PRIMARY, 0};
				dwResult &= 0x00FFFFFF;
				if (dwResult > *(uint32*)abyCurVer){
					SetActiveWindow();
					Log(LOG_SUCCESS|LOG_STATUSBAR,GetResString(IDS_X_NEW_NEO_VERSION));
					ShowNotifier(GetResString(IDS_X_NEW_NEO_VERSION), TBN_NEWNEOVERSION);
					if (AfxMessageBox(GetResString(IDS_X_NEW_NEO_VERSION_DOWNLAOD),MB_YESNO)==IDYES) {
						ShellExecute(NULL, NULL, _T("http://sourceforge.net/projects/neomule/"), NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
					}
				}
				else{
					thePrefs.UpdateLastVC();
					AddLogLine(true,GetResString(IDS_X_NO_NEW_NEO_VERSION));
				}
				return 0;
			}
		}
	}
	LogWarning(LOG_STATUSBAR,GetResString(IDS_NEWVERSIONFAILED));
	return 0;
}
// NEO: NVC END <-- Xanatos --

// NEO: SS - [SplashScreen] -- Xanatos --
/*void CemuleDlg::ShowSplash()
{
	ASSERT( m_pSplashWnd == NULL );
	if (m_pSplashWnd == NULL)
	{
		m_pSplashWnd = new CSplashScreen;
		if (m_pSplashWnd != NULL)
		{
			ASSERT(m_hWnd);
			if (m_pSplashWnd->Create(CSplashScreen::IDD, this))
			{
				m_pSplashWnd->ShowWindow(SW_SHOW);
				m_pSplashWnd->UpdateWindow();
				m_dwSplashTime = ::GetCurrentTime();
			}
			else
			{
				delete m_pSplashWnd;
				m_pSplashWnd = NULL;
			}
		}
	}
}

void CemuleDlg::DestroySplash()
{
	if (m_pSplashWnd != NULL)
	{
		m_pSplashWnd->DestroyWindow();
		delete m_pSplashWnd;
		m_pSplashWnd = NULL;
	}
#ifdef _BETA
	if (!thePrefs.IsFirstStart())
		AfxMessageBox(GetResString(IDS_BETANAG), MB_ICONINFORMATION | MB_OK, 0);
#endif
}*/

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
		DWORD dwNow = GetTickCount();
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

LRESULT CemuleDlg::OnKickIdle(UINT /*nWhy*/, long lIdleCount)
{
	LRESULT lResult = 0;

	// NEO: SS - [SplashScreen] -- Xanatos --
	/*if (m_pSplashWnd)
	{
		if (::GetCurrentTime() - m_dwSplashTime > 2500)
		{
			// timeout expired, destroy the splash window
			DestroySplash();
			UpdateWindow();
		}
		else
		{
			// check again later...
			lResult = 1;
		}
	}*/

	if (m_bStartMinimized)
		PostStartupMinimized();

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
			lResult = theApp.OnIdle(lIdleCount);
		}
	}

	return lResult;
}

int CemuleDlg::MapWindowToToolbarButton(CWnd* pWnd) const
{
	int iButtonID = -1;
	if (pWnd == transferwnd)        iButtonID = TBBTN_TRANSFERS;
	else if (pWnd == serverwnd)     iButtonID = TBBTN_SERVER;
	else if (pWnd == chatwnd)       iButtonID = TBBTN_MESSAGES;
	else if (pWnd == ircwnd)        iButtonID = TBBTN_IRC;
	else if (pWnd == sharedfileswnd)iButtonID = TBBTN_SHARED;
	else if (pWnd == searchwnd)     iButtonID = TBBTN_SEARCH;
	else if (pWnd == statisticswnd)	iButtonID = TBBTN_STATS;
	else if	(pWnd == kademliawnd)	iButtonID = TBBTN_KAD;
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
		case TBBTN_MESSAGES:	pWnd = chatwnd;			break;
		case TBBTN_IRC:			pWnd = ircwnd;			break;
		case TBBTN_SHARED:		pWnd = sharedfileswnd;	break;
		case TBBTN_SEARCH:		pWnd = searchwnd;		break;
		case TBBTN_STATS:		pWnd = statisticswnd;	break;
		case TBBTN_KAD:			pWnd = kademliawnd;		break;
		default:				pWnd = NULL; ASSERT(0);
	}
	return pWnd;
}

bool CemuleDlg::IsWindowToolbarButton(int iButtonID) const
{
	switch (iButtonID)
	{
		case TBBTN_TRANSFERS:	return true;
		case TBBTN_SERVER:		return true;
		case TBBTN_MESSAGES:	return true;
		case TBBTN_IRC:			return true;
		case TBBTN_SHARED:		return true;
		case TBBTN_SEARCH:		return true;
		case TBBTN_STATS:		return true;
		case TBBTN_KAD:			return true;
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
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	m_ttip.RelayEvent(pMsg); 
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
	BOOL bResult = CTrayDialog::PreTranslateMessage(pMsg);

	//if (m_pSplashWnd && m_pSplashWnd->m_hWnd != NULL &&
	if (theApp.IsSplash() && // NEO: SS - [SplashScreen] <-- Xanatos --
		(pMsg->message == WM_KEYDOWN	   ||
		 pMsg->message == WM_SYSKEYDOWN	   ||
		 pMsg->message == WM_LBUTTONDOWN   ||
		 pMsg->message == WM_RBUTTONDOWN   ||
		 pMsg->message == WM_MBUTTONDOWN   ||
		 pMsg->message == WM_NCLBUTTONDOWN ||
		 pMsg->message == WM_NCRBUTTONDOWN ||
		 pMsg->message == WM_NCMBUTTONDOWN))
	{
		//DestroySplash();
		theApp.HideSplash(); // NEO: SS - [SplashScreen] <-- Xanatos --
		UpdateWindow();
	}
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

void CemuleDlg::HtmlHelp(DWORD_PTR dwData, UINT nCmd)
{
	CWinApp* pApp = AfxGetApp();
	ASSERT_VALID(pApp);
	ASSERT(pApp->m_pszHelpFilePath != NULL);
	// to call HtmlHelp the m_fUseHtmlHelp must be set in
	// the application's constructor
	ASSERT(pApp->m_eHelpType == afxHTMLHelp);

	CWaitCursor wait;

	PrepareForHelp();

	// need to use top level parent (for the case where m_hWnd is in DLL)
	CWnd* pWnd = GetTopLevelParent();

	TRACE(traceAppMsg, 0, _T("HtmlHelp: pszHelpFile = '%s', dwData: $%lx, fuCommand: %d.\n"), pApp->m_pszHelpFilePath, dwData, nCmd);

	bool bHelpError = false;
	CString strHelpError;
	int iTry = 0;
	while (iTry++ < 2)
	{
		if (!AfxHtmlHelp(pWnd->m_hWnd, pApp->m_pszHelpFilePath, nCmd, dwData))
		{
			bHelpError = true;
			strHelpError.LoadString(AFX_IDP_FAILED_TO_LAUNCH_HELP);

			typedef struct tagHH_LAST_ERROR
			{
				int      cbStruct;
				HRESULT  hr;
				BSTR     description;
			} HH_LAST_ERROR;
			HH_LAST_ERROR hhLastError = {0};
			hhLastError.cbStruct = sizeof hhLastError;
			HWND hwndResult = AfxHtmlHelp(pWnd->m_hWnd, NULL, HH_GET_LAST_ERROR, reinterpret_cast<DWORD>(&hhLastError));
			if (hwndResult != 0)
			{
				if (FAILED(hhLastError.hr))
				{
					if (hhLastError.description)
					{
						USES_CONVERSION;
						strHelpError = OLE2T(hhLastError.description);
						::SysFreeString(hhLastError.description);
					}
					if (   hhLastError.hr == 0x8004020A  /*no topics IDs available in Help file*/
						|| hhLastError.hr == 0x8004020B) /*requested Help topic ID not found*/
					{
						// try opening once again without help topic ID
						if (nCmd != HH_DISPLAY_TOC)
						{
							nCmd = HH_DISPLAY_TOC;
							dwData = 0;
							continue;
						}
					}
				}
			}
			break;
		}
		else
		{
			bHelpError = false;
			strHelpError.Empty();
			break;
		}
	}

	if (bHelpError)
	{
		if (AfxMessageBox(CString(pApp->m_pszHelpFilePath) + _T("\n\n") + strHelpError + _T("\n\n") + GetResString(IDS_ERR_NOHELP), MB_YESNO | MB_ICONERROR) == IDYES)
		{
			CString strUrl = thePrefs.GetHomepageBaseURL() + _T("/home/perl/help.cgi");
			ShellExecute(NULL, NULL, strUrl, NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
		}
	}
}

LRESULT CemuleDlg::OnPeerCacheResponse(WPARAM wParam, LPARAM lParam)
{
	return theApp.m_pPeerCache->OnPeerCacheCheckResponse(wParam,lParam);
}

void CemuleDlg::CreateToolbarCmdIconMap()
{
	m_mapTbarCmdToIcon.SetAt(TBBTN_CONNECT, _T("Connect"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_KAD, _T("Kademlia"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_SERVER, _T("Server"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_TRANSFERS, _T("Transfer"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_SEARCH, _T("Search"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_SHARED, _T("SharedFiles"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_MESSAGES, _T("Messages"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_IRC, _T("IRC"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_STATS, _T("Statistics"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_OPTIONS, _T("Preferences"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_TOOLS, _T("Tools"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_HELP, _T("Help"));
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	m_mapTbarCmdToIcon.SetAt(TBBTN_VOODOO, _T("Voodootb"));
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
}

LPCTSTR CemuleDlg::GetIconFromCmdId(UINT uId)
{
	LPCTSTR pszIconId = NULL;
	if (m_mapTbarCmdToIcon.Lookup(uId, pszIconId))
		return pszIconId;
	return NULL;
}

BOOL CemuleDlg::OnChevronPushed(UINT id, NMHDR* pNMHDR, LRESULT* plResult)
{
	UNREFERENCED_PARAMETER(id);
	if (!thePrefs.GetUseReBarToolbar())
		return FALSE;

	NMREBARCHEVRON* pnmrc = (NMREBARCHEVRON*)pNMHDR;

	ASSERT( id == AFX_IDW_REBAR );
	ASSERT( pnmrc->uBand == 0 );
	ASSERT( pnmrc->wID == 0 );
	ASSERT( m_mapTbarCmdToIcon.GetSize() != 0 );

	// get visible area of rebar/toolbar
	CRect rcVisibleButtons;
	toolbar->GetClientRect(&rcVisibleButtons);

	// search the first toolbar button which is not fully visible
	int iButtons = toolbar->GetButtonCount();
	int i;
	for (i = 0; i < iButtons; i++)
	{
		CRect rcButton;
		toolbar->GetItemRect(i, &rcButton);

		CRect rcVisible;
		if (!rcVisible.IntersectRect(&rcVisibleButtons, &rcButton) || !EqualRect(rcButton, rcVisible))
			break;
	}

	// create menu for all toolbar buttons which are not (fully) visible
	BOOL bLastMenuItemIsSep = TRUE;
	CTitleMenu menu;
	menu.CreatePopupMenu();
	menu.AddMenuTitle(_T("eMule"), true);
	while (i < iButtons)
	{
		TCHAR szString[256];
		szString[0] = _T('\0');
		TBBUTTONINFO tbbi = {0};
		tbbi.cbSize = sizeof tbbi;
		tbbi.dwMask = TBIF_BYINDEX | TBIF_COMMAND | TBIF_STYLE | TBIF_STATE | TBIF_TEXT;
		tbbi.cchText = _countof(szString);
		tbbi.pszText = szString;
		if (toolbar->GetButtonInfo(i, &tbbi) != -1)
		{
			if (tbbi.fsStyle & TBSTYLE_SEP)
			{
				if (!bLastMenuItemIsSep)
					bLastMenuItemIsSep = menu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
			}
			else
			{
				if (szString[0] != _T('\0') && menu.AppendMenu(MF_STRING, tbbi.idCommand, szString, GetIconFromCmdId(tbbi.idCommand)))
				{
					bLastMenuItemIsSep = FALSE;
					if (tbbi.fsState & TBSTATE_CHECKED)
						menu.CheckMenuItem(tbbi.idCommand, MF_BYCOMMAND | MF_CHECKED);
					if ((tbbi.fsState & TBSTATE_ENABLED) == 0)
						menu.EnableMenuItem(tbbi.idCommand, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
				}
			}
		}

		i++;
	}

	CPoint ptMenu(pnmrc->rc.left, pnmrc->rc.top);
	ClientToScreen(&ptMenu);
	ptMenu.y += rcVisibleButtons.Height();
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, ptMenu.x, ptMenu.y, this);
	*plResult = 1;
	return FALSE;
}

bool CemuleDlg::IsPreferencesDlgOpen() const
{
	//return (preferenceswnd->m_hWnd != NULL);
	return preferenceswnd->IsDialogOpen(); // NEO: MLD - [ModelesDialogs] <-- Xanatos --
}

int CemuleDlg::ShowPreferences(UINT /*uStartPageID*/) // NEO: NCFG - [NeoConfiguration] <-- Xanatos --
{
	// NEO: TPP - [TrayPasswordProtection] -- Xanatos -->
	if(m_TrayLocked == true)
		return 0;
	// NEO: TPP END <-- Xanatos --
	if (IsPreferencesDlgOpen())
	{
		preferenceswnd->SetForegroundWindow();
		preferenceswnd->BringWindowToTop();
		return -1;
	}
	else
	{
		// NEO: NCFG - [NeoConfiguration] -- Xanatos --
		/*if (uStartPageID != (UINT)-1)
			preferenceswnd->SetStartPage(uStartPageID);*/
		//return preferenceswnd->DoModal();
		// NEO: MLD - [ModelesDialogs] -- Xanatos -->
		preferenceswnd->OpenDialog(FALSE);
		return 0;
		// NEO: MLD END <-- Xanatos --
	}
}



//////////////////////////////////////////////////////////////////
// Webserver related

LRESULT CemuleDlg::OnWebAddDownloads(WPARAM wParam, LPARAM lParam)
{
	CString link=CString((TCHAR*)wParam);
	if (link.GetLength()==32 && link.Left(4).CompareNoCase(_T("ed2k"))!=0) {
		uchar fileid[16];
		DecodeBase16(link, link.GetLength(), fileid, _countof(fileid));
		theApp.searchlist->AddFileToDownloadByHash(fileid,(uint8)lParam);

	} else
		theApp.AddEd2kLinksToDownload((TCHAR*)wParam, (int)lParam);

	return 0;
}

LRESULT CemuleDlg::OnAddRemoveFriend(WPARAM wParam, LPARAM lParam)
{
	if (lParam==0) { // remove
		theApp.friendlist->RemoveFriend((CFriend*)wParam);
	} else {		// add
		theApp.friendlist->AddFriend((CUpDownClient*)wParam);
	}

	return 0;
}

LRESULT CemuleDlg::OnWebSetCatPrio(WPARAM wParam, LPARAM lParam)
{
	theApp.downloadqueue->SetCatPrio(wParam,(uint8)lParam);
	return 0;
}
LRESULT CemuleDlg::OnWebServerClearCompleted(WPARAM wParam, LPARAM lParam)
{
	if(!wParam)
	{
		int cat=(int)lParam;
		transferwnd->downloadlistctrl.ClearCompleted(cat);
	}
	else
	{
		uchar* pFileHash = reinterpret_cast<uchar*>(lParam);
		CKnownFile* file=theApp.knownfiles->FindKnownFileByID(pFileHash);
		if (file)
			transferwnd->downloadlistctrl.RemoveFile((CPartFile*)file);
		delete[] pFileHash;
	}

	return 0;
}

LRESULT CemuleDlg::OnWebServerFileRename(WPARAM wParam, LPARAM lParam)
{
	CString sNewName = ((LPCTSTR)(lParam));

	((CPartFile*)wParam)->SetFileName(sNewName);
	((CPartFile*)wParam)->SavePartFile();
	((CPartFile*)wParam)->UpdateDisplayedInfo();
	sharedfileswnd->sharedfilesctrl.UpdateFile( (CKnownFile*)((CPartFile*)wParam));

	return 0;
}

LRESULT CemuleDlg::OnWebGUIInteraction(WPARAM wParam, LPARAM lParam) {

	switch (wParam) {
		case WEBGUIIA_UPDATEMYINFO:
			serverwnd->UpdateMyInfo();
			break;
		case WEBGUIIA_WINFUNC:{
			if (thePrefs.GetWebAdminAllowedHiLevFunc())
			{
				try {
					HANDLE hToken;
					TOKEN_PRIVILEGES tkp;	// Get a token for this process.
					
					if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
						throw;
					LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
					tkp.PrivilegeCount = 1;  // one privilege to set
					tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;	// Get the shutdown privilege for this process.
					AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

					if (lParam==1) {	// shutdown
						ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0);
					} else 
					if (lParam==2) {
						ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0);
					}

				} catch(...)
					{
						AddLogLine(true, GetResString(IDS_WEB_REBOOT) + _T(' ') + GetResString(IDS_FAILED));
				}
			}
			else 
				AddLogLine(true, GetResString(IDS_WEB_REBOOT) + _T(' ') + GetResString(IDS_ACCESSDENIED));
			break;
		}
		case WEBGUIIA_UPD_CATTABS:
			theApp.emuledlg->transferwnd->UpdateCatTabTitles();
			break;
		case WEBGUIIA_UPD_SFUPDATE: {
			CKnownFile* kf=(CKnownFile*)lParam;
			if (kf)
				theApp.sharedfiles->UpdateFile(kf);
			}
			break;
		case WEBGUIIA_UPDATESERVER:
			serverwnd->serverlistctrl.RefreshServer((CServer*)lParam);
			break;
		case WEBGUIIA_STOPCONNECTING:
			theApp.serverconnect->StopConnectionTry();
			break;
		case WEBGUIIA_CONNECTTOSERVER: {
			CServer* server=(CServer*)lParam;
			if (server==NULL) 
				theApp.serverconnect->ConnectToAnyServer();
			else 
				theApp.serverconnect->ConnectToServer(server);
			break;
			}
		case WEBGUIIA_DISCONNECT:
			if (lParam!=2)	// !KAD
				theApp.serverconnect->Disconnect();
			if (lParam!=1)	// !ED2K
				Kademlia::CKademlia::Stop();
			break;

		case WEBGUIIA_SERVER_REMOVE: {
			serverwnd->serverlistctrl.RemoveServer((CServer*)lParam);
			break;
		}
		case WEBGUIIA_SHARED_FILES_RELOAD: {
			theApp.sharedfiles->Reload();
			break;
		}
		case WEBGUIIA_ADD_TO_STATIC: {
			serverwnd->serverlistctrl.StaticServerFileAppend((CServer*)lParam);
			break;
		}
		case WEBGUIIA_REMOVE_FROM_STATIC: {
			serverwnd->serverlistctrl.StaticServerFileRemove((CServer*)lParam);
			break;
		}
		case WEBGUIIA_UPDATESERVERMETFROMURL:
			theApp.emuledlg->serverwnd->UpdateServerMetFromURL((TCHAR*)lParam);
			break;
		case WEBGUIIA_SHOWSTATISTICS:
			theApp.emuledlg->statisticswnd->ShowStatistics(lParam!=0);
			break;
		case WEBGUIIA_DELETEALLSEARCHES:
			theApp.emuledlg->searchwnd->DeleteAllSearches();
			break;

		case WEBGUIIA_KAD_BOOTSTRAP:{
			CString dest=CString((TCHAR*)lParam);
			int pos=dest.Find(_T(':'));
			if (pos!=-1) {
				uint16 port = (uint16)_tstoi(dest.Right(dest.GetLength() - pos - 1));
				CString ip = dest.Left(pos);
				// JOHNTODO - Switch between Kad1 and Kad2
				Kademlia::CKademlia::Bootstrap(ip, port, true);
			}
			break;
		}
		case WEBGUIIA_KAD_START:
			Kademlia::CKademlia::Start();
			break;
		case WEBGUIIA_KAD_STOP:
			Kademlia::CKademlia::Stop();
			break;
		case WEBGUIIA_KAD_RCFW:
			Kademlia::CKademlia::RecheckFirewalled();
			break;
	}

	return 0;
}

void CemuleDlg::TrayMinimizeToTrayChange()
{
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		if (!thePrefs.GetMinToTray())
		{
			// just for safety, ensure that we are not adding duplicate menu entries..
			if (pSysMenu->EnableMenuItem(MP_MINIMIZETOTRAY, MF_BYCOMMAND | MF_ENABLED) == -1)
			{
				ASSERT( (MP_MINIMIZETOTRAY & 0xFFF0) == MP_MINIMIZETOTRAY && MP_MINIMIZETOTRAY < 0xF000);
				VERIFY( pSysMenu->InsertMenu(SC_MINIMIZE, MF_BYCOMMAND, MP_MINIMIZETOTRAY, GetResString(IDS_PW_TRAY)) );
			}
			else
				ASSERT(0);
		}
		else
		{
			(void)pSysMenu->RemoveMenu(MP_MINIMIZETOTRAY, MF_BYCOMMAND);
		}
	}
	CTrayDialog::TrayMinimizeToTrayChange();
}

void CemuleDlg::SetToolTipsDelay(UINT uMilliseconds)
{
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	UNREFERENCED_PARAMETER(uMilliseconds);
	transferwnd->SetTTDelay();
	serverwnd->SetTTDelay();
	SetTTDelay();
	searchwnd->SetTTDelay(); 
	sharedfileswnd->SetTTDelay(); 
#else
	//searchwnd->SetToolTipsDelay(uMilliseconds);
	transferwnd->SetToolTipsDelay(uMilliseconds);
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
}

void CemuleDlg::UPnPTimeOutTimer(HWND /*hwnd*/, UINT /*uiMsg*/, UINT /*idEvent*/, DWORD /*dwTime*/){
	::PostMessage(theApp.emuledlg->GetSafeHwnd(), UM_UPNP_RESULT, (WPARAM)CUPnPImpl::UPNP_TIMEOUT, 0);
}

LRESULT CemuleDlg::OnUPnPResult(WPARAM wParam, LPARAM /*lParam*/){
	if (wParam == CUPnPImpl::UPNP_FAILED){		
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
	if (wParam == CUPnPImpl::UPNP_OK){
		// remember the last working implementation
		thePrefs.SetLastWorkingUPnPImpl(theApp.m_pUPnPFinder->GetImplementation()->GetImplementationID());
		Log(GetResString(IDS_UPNPSUCCESS), theApp.m_pUPnPFinder->GetImplementation()->GetUsedTCPPort()
			, theApp.m_pUPnPFinder->GetImplementation()->GetUsedUDPPort());
	}
	else
		LogWarning(GetResString(IDS_UPNPFAILED));

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
				// TODO: do we need to reinitiate UPNP?
				PostMessage(WM_SYSCOMMAND , MP_CONNECT, 0); // tell to connect.. a sec later...
			}
			return TRUE; // message processed.
			break;
		}
		case PBT_APMSUSPEND:
		{		
			DebugLog(_T("System is going is suspending operation, disconnecting. wParam=%d lPararm=%ld"),wParam,lParam);
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
					, ((nForceUDPPort != 0) ? nForceUDPPort :thePrefs.GetUDPPort()));
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

// NEO: IM - [InvisibelMode] -- Xanatos -->
LRESULT CemuleDlg::OnHotKey(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	//ikabotTest
	//if(wParam == HOTKEY_INVISIBLEMODE_ID) RestoreWindow();

	// Allows "invisible mode" on multiple instances of eMule
	// Restore the rest of hidden emules
	EnumWindows(AskEmulesForInvisibleMode, INVMODE_RESTOREWINDOW);
	
	return 0;
}

BOOL CemuleDlg::RegisterInvisibleHotKey()
{
	if(m_hWnd && IsRunning()){
		BOOL res = RegisterHotKey( this->m_hWnd, HOTKEY_INVISIBLEMODE_ID ,
						   NeoPrefs.GetInvisibleModeHKKeyModifier(),
						   NeoPrefs.GetInvisibleModeHKKey());
		return res;
	} else
		return FALSE;
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
	} else
		return false;
}

// Allows "invisible mode" on multiple instances of eMule
// LOWORD(WPARAM) -> HotKey KeyModifier
// HIWORD(WPARAM) -> HotKey VirtualKey
// LPARAM		  -> int:	INVMODE_RESTOREWINDOW	-> Restores the window
//							INVMODE_REGISTERHOTKEY	-> Registers the hotkey
LRESULT CemuleDlg::OnRestoreWindowInvisibleMode(WPARAM wParam, LPARAM lParam)
{
	if (NeoPrefs.GetInvisibleMode() &&
		(UINT)LOWORD(wParam) == NeoPrefs.GetInvisibleModeHKKeyModifier() &&
		(TCHAR)HIWORD(wParam) == NeoPrefs.GetInvisibleModeHKKey()) {
			switch(lParam){
				case INVMODE_RESTOREWINDOW:
					RestoreWindow();
					// NEO: STI - [StaticTray]
					if(NeoPrefs.UseStaticTrayIcon()) 
						TrayShow(true);
					// NEO: MOD END
					break;
				case INVMODE_REGISTERHOTKEY:
					RegisterInvisibleHotKey();
					break;
			}
			return UWM_RESTORE_WINDOW_IM;
	} else
		return false;
} 

// Allows "invisible mode" on multiple instances of eMule
BOOL CALLBACK CemuleDlg::AskEmulesForInvisibleMode(HWND hWnd, LPARAM lParam){
	DWORD dwMsgResult;
	WPARAM msgwParam;

	msgwParam=MAKEWPARAM(NeoPrefs.GetInvisibleModeHKKeyModifier(),
				NeoPrefs.GetInvisibleModeHKKey());

	LRESULT res = ::SendMessageTimeout(hWnd,UWM_RESTORE_WINDOW_IM, msgwParam, lParam,
				SMTO_BLOCK |SMTO_ABORTIFHUNG,10000,&dwMsgResult);
	
	return res; 
} 
// NEO: IM END <-- Xanatos --

// NEO: NMX - [NeoMenuXP] -- Xanatos -->
void CemuleDlg::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	HMENU hMenu = AfxGetThreadState()->m_hTrackingMenu;
	if(CMenu *pMenu = CMenu::FromHandle(hMenu))
		pMenu->MeasureItem(lpMeasureItemStruct);
	
	if(nIDCtl)
		CTrayDialog::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}
// NEO: NMX END <-- Xanatos --

#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
BOOL CemuleDlg::OnToolTipNotify(UINT /*id*/, NMHDR *pNMH, LRESULT* /*pResult*/)
{
	NM_PPTOOLTIP_DISPLAY * pNotify = (NM_PPTOOLTIP_DISPLAY*)pNMH;
	int control_id = CWnd::FromHandle(pNotify->ti->hWnd)->GetDlgCtrlID();
	switch(control_id)
	{
		case IDC_STATUSBAR:{
			pNotify->ti->hIcon = statusbar->GetTipInfo(*((CString *)&pNotify->ti->sTooltip));
			return TRUE;
		}
		default:
			if(pNotify->ti->hIcon)
				pNotify->ti->hIcon = DuplicateIcon(AfxGetInstanceHandle(), pNotify->ti->hIcon);
			return TRUE;
	}
}

void CemuleDlg::SetTTDelay()
{
	m_ttip.SetDelayTime(TTDT_AUTOPOP, 40000);
	m_ttip.SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay() * 500);
}
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --