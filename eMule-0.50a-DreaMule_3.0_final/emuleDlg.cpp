//this file is part of eMule
//Copyright (C)2002-2007 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "ChatWnd.h"
#include "IrcWnd.h"
#include "StatisticsDlg.h"
#include "CreditsDlg.h"
#include "PreferencesDlg.h"
#include "Sockets.h"
#include "KnownFileList.h"
#include "ServerList.h"
#include "Opcodes.h"
#include "SharedFileList.h"
#include "ED2KLink.h"
//#include "Splashscreen.h"  //Xman slpashscreen
//#include "SplashScreenEx.h" //Xman splashscreen
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
//#include "LastCommonRouteFinder.h" //Xman
#include "WebServer.h"
// Tux: LiteMule: Remove MobileMule
#include "DownloadQueue.h"
#include "ClientUDPSocket.h"
#include "UploadQueue.h"
#include "ClientList.h"
#include "UploadBandwidthThrottler.h"
#include "FriendList.h"
#include "IPFilter.h"
#include "Statistics.h"
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
//Xman
#include "BandWidthControl.h" // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
#include "IP2Country.h" //EastShare - added by AndCycle, IP to Country
#include "DLP.h" //Xman DLP
// Tux: LiteMule: Remove TTS
#include "Collection.h"
#include "CollectionViewDialog.h"
#include "VisualStylesXP.h"
#include "./Addons/ModIconMapping.h" //>>> WiZaRd::ModIconMappings

// X-Ray :: Statusbar :: Start
// X-Ray :: Statusbar :: End
#ifdef NATTUNNELING // NEO: UTCP - [UserModeTCP] -- Xanatos -->
#include "Neo/NatManager.h"
#endif //NATTUNNELING // NEO: UTCP END <-- Xanatos --

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//boizaum
#include "AdunanzA.h"
#include "DAMessageBox.h"
//boizaum
bool fechadu = false;
#define	SYS_TRAY_ICON_COOKIE_FORCE_UPDATE	(UINT)-1

BOOL (WINAPI *_TransparentBlt)(HDC, int, int, int, int, HDC, int, int, int, int, UINT) = NULL;
const static UINT UWM_ARE_YOU_EMULE = RegisterWindowMessage(EMULE_GUID);
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
	ON_MESSAGE(WM_COPYDATA, OnWMData)
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
	ON_MESSAGE(WM_USERCHANGED, OnUserChanged)
	ON_WM_SHOWWINDOW()
	ON_WM_DESTROY()
	ON_WM_SETTINGCHANGE()
	ON_WM_ACTIVATE()//boi
	///////////////////////////////////////////////////////////////////////////
	// WM_COMMAND messages
	//
	ON_COMMAND(MP_CONNECT, StartConnection)
	ON_COMMAND(MP_DISCONNECT, CloseConnection)
//>>> WiZaRd::Minimize on Close
	ON_COMMAND(MP_EXIT, DoClose) 
//	ON_COMMAND(MP_EXIT, OnClose)
//<<< WiZaRd::Minimize on Close
	ON_COMMAND(MP_RESTORE, RestoreWindow)
	// quick-speed changer --
	ON_COMMAND_RANGE(MP_QS_U10, MP_QS_UP10, QuickSpeedUpload)
	ON_COMMAND_RANGE(MP_QS_D10, MP_QS_DC, QuickSpeedDownload)
	//--- quickspeed - paralize all ---
	ON_COMMAND_RANGE(MP_QS_PA, MP_QS_UA, QuickSpeedOther)
	// quick-speed changer -- based on xrmb

	ON_REGISTERED_MESSAGE(UWM_ARE_YOU_EMULE, OnAreYouEmule)
	ON_BN_CLICKED(IDC_HOTMENU, OnBnClickedHotmenu)

	///////////////////////////////////////////////////////////////////////////
	// WM_USER messages
	//
	ON_MESSAGE(UM_TASKBARNOTIFIERCLICKED, OnTaskbarNotifierClicked)
	ON_MESSAGE(UM_CLOSE_MINIMULE, OnCloseMiniMule)

	//TK4 Mod 1.3a onward - hot key
	ON_MESSAGE(WM_HOTKEY, OnHotKey)

	// Webserver messages
	ON_MESSAGE(WEB_GUI_INTERACTION, OnWebGUIInteraction)
	ON_MESSAGE(WEB_CLEAR_COMPLETED, OnWebServerClearCompleted)
	ON_MESSAGE(WEB_FILE_RENAME, OnWebServerFileRename)
	ON_MESSAGE(WEB_ADDDOWNLOADS, OnWebAddDownloads)
	ON_MESSAGE(WEB_CATPRIO, OnWebSetCatPrio)
	ON_MESSAGE(WEB_ADDREMOVEFRIEND, OnAddRemoveFriend)

	// Tux: LiteMule: Remove Version Check

	// PeerCache DNS
	ON_MESSAGE(UM_PEERCHACHE_RESPONSE, OnPeerCacheResponse)

	///////////////////////////////////////////////////////////////////////////
	// WM_APP messages
	//
	ON_MESSAGE(TM_FINISHEDHASHING, OnFileHashed)
	ON_MESSAGE(TM_FILEOPPROGRESS, OnFileOpProgress)
	ON_MESSAGE(TM_HASHFAILED, OnHashFailed)
	//Xman
	// BEGIN SLUGFILLER: SafeHash
	ON_MESSAGE(TM_PARTHASHEDOK, OnPartHashedOK)
	ON_MESSAGE(TM_PARTHASHEDCORRUPT, OnPartHashedCorrupt)
	ON_MESSAGE(TM_PARTHASHEDOKAICHRECOVER, OnPartHashedOKAICHRecover)
	ON_MESSAGE(TM_PARTHASHEDCORRUPTAICHRECOVER, OnPartHashedCorruptAICHRecover)
	// END SLUGFILLER: SafeHash
	ON_MESSAGE(TM_READBLOCKFROMFILEDONE, OnReadBlockFromFileDone) // SiRoB: ReadBlockFromFileThread
	ON_MESSAGE(TM_FLUSHDONE, OnFlushDone) // SiRoB: Flush Thread
	ON_MESSAGE(TM_DOTIMER, DoTimer) //Xman process timer code via messages (Xanatos)
	//Xman end
	ON_MESSAGE(TM_FRAMEGRABFINISHED, OnFrameGrabFinished)
	ON_MESSAGE(TM_FILEALLOCEXC, OnFileAllocExc)
	ON_MESSAGE(TM_FILECOMPLETED, OnFileCompleted)

	ON_MESSAGE(TM_CONSOLETHREADEVENT, OnConsoleThreadEvent)


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
	mediaplayerwnd = new CMediaPlayerWnd; //>>> WiZaRd::MediaPlayer
	searchwnd = new CSearchDlg;
	chatwnd = new CChatWnd;
	ircwnd = new CIrcWnd;
	statisticswnd = new CStatisticsDlg;
	statusbar = new CMuleStatusBarCtrl;
	m_wndTaskbarNotifier = new CTaskbarNotifier;
//>>> WiZaRd::WebBrowser [Pruna]
	webbrowser = new CWebBrowserWnd;   //Added by thilon on 2006.08.01
//<<< WiZaRd::WebBrowser [Pruna]

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
	//Xman new slpash-screen arrangement
	//m_pSplashWnd = NULL;
	//m_dwSplashTime = (DWORD)-1;
	//Xman end
	m_pSystrayDlg = NULL;
	m_pMiniMule = NULL;
	m_uLastSysTrayIconCookie = SYS_TRAY_ICON_COOKIE_FORCE_UPDATE;

	// X-Ray :: Statusbar :: Start
	fileicon = NULL;
	sysinfoicon = NULL;
	// X-Ray :: Statusbar :: End
}

CemuleDlg::~CemuleDlg()
{
	// Tux: LiteMule: Remove TTS
	DestroyMiniMule();
	if (m_icoSysTrayCurrent) VERIFY( DestroyIcon(m_icoSysTrayCurrent) );
	if (m_hIcon) VERIFY( ::DestroyIcon(m_hIcon) );
	for (int i = 0; i < _countof(connicons); i++){
		if (connicons[i]) VERIFY( ::DestroyIcon(connicons[i]) );
	}
	if (transicons[0]) VERIFY( ::DestroyIcon(transicons[0]) );
	if (transicons[1]) VERIFY( ::DestroyIcon(transicons[1]) );
	// X-Ray :: Statusbar :: Start
	/*
	if (transicons[2]) VERIFY( ::DestroyIcon(transicons[2]) );
	if (transicons[3]) VERIFY( ::DestroyIcon(transicons[3]) );
	*/
	// X-Ray :: Statusbar :: End
	if (imicons[0]) VERIFY( ::DestroyIcon(imicons[0]) );
	if (imicons[1]) VERIFY( ::DestroyIcon(imicons[1]) );
	if (imicons[2]) VERIFY( ::DestroyIcon(imicons[2]) );
	if (m_icoSysTrayConnected) VERIFY( ::DestroyIcon(m_icoSysTrayConnected) );
	if (m_icoSysTrayDisconnected) VERIFY( ::DestroyIcon(m_icoSysTrayDisconnected) );
	if (m_icoSysTrayLowID) VERIFY( ::DestroyIcon(m_icoSysTrayLowID) );
	if (usericon) VERIFY( ::DestroyIcon(usericon) );

	// X-Ray :: Statusbar :: Start
	if (fileicon) VERIFY( ::DestroyIcon(fileicon) );
	if (sysinfoicon) VERIFY( ::DestroyIcon(sysinfoicon) );
	// X-Ray :: Statusbar :: End

	// already destroyed by windows?
	//VERIFY( m_menuUploadCtrl.DestroyMenu() );
	//VERIFY( m_menuDownloadCtrl.DestroyMenu() );
	//VERIFY( m_SysMenuOptions.DestroyMenu() );

	delete preferenceswnd;
	delete serverwnd;
	delete kademliawnd;
	delete transferwnd;
	delete sharedfileswnd;
	delete mediaplayerwnd; //>>> WiZaRd::MediaPlayer
	delete chatwnd;
	delete ircwnd;
	delete statisticswnd;
	delete statusbar;
	delete m_wndTaskbarNotifier;
	delete m_pDropTarget;
//>>> WiZaRd::WebBrowser [Pruna]
	delete webbrowser;	//added by thilon on 2006.08.01
//<<< WiZaRd::WebBrowser [Pruna]
}

void CemuleDlg::DoDataExchange(CDataExchange* pDX)
{
	CTrayDialog::DoDataExchange(pDX);

	// X-Ray :: Toolbar :: Start
	DDX_Control(pDX, IDC_TB_BTN_CONNECT, m_co_ConnectBtn);
	DDX_Control(pDX, IDC_TB_BTN_SEARCH, m_co_SearchBtn);
	DDX_Control(pDX, IDC_TB_BTN_TRANSFER, m_co_TransferBtn);
	DDX_Control(pDX, IDC_TB_BTN_FILES, m_co_FilesBtn);
	DDX_Control(pDX, IDC_TB_BTN_MEDIAPLAYER, m_co_MediaPlayerBtn); //>>> WiZaRd::MediaPlayer
	//DDX_Control(pDX, IDC_TB_BTN_KADEMLIA, m_co_KademliaBtn);
	DDX_Control(pDX, IDC_TB_BTN_SERVER, m_co_ServerBtn);
	//DDX_Control(pDX, IDC_TB_BTN_MESSAGES, m_co_MessagesBtn);
	DDX_Control(pDX, IDC_TB_BTN_IRC, m_co_IrcBtn);
	DDX_Control(pDX, IDC_TB_BTN_STATISTIC, m_co_StatisticBtn);
	DDX_Control(pDX, IDC_TB_BTN_PREFERENCES, m_co_PreferencesBtn);


	// X-Ray :: Toolbar :: End
}

LRESULT CemuleDlg::OnAreYouEmule(WPARAM, LPARAM)
{
	RestoreWindow();//boizaum always focus new windows
	return UWM_ARE_YOU_EMULE;
}
// X-Ray :: Toolbar :: Start


// X-Ray :: Toolbar :: Start
#define	TB_BIGSMALLSPACE	25 //space between big and small buttons
#define TB_HEIGHT			60 //height of the MAIN bitmaps
//#define BIG_BUTTON_SIZE		56 //56x56
#define BIG_BUTTON_SIZE		72 //56x56
static const int iBigButtons[] = 
{
	IDC_TB_BTN_CONNECT,	
	IDC_TB_BTN_SEARCH,
	IDC_TB_BTN_TRANSFER,
	IDC_TB_BTN_FILES
};

#define SMALL_BUTTON_SIZE	50 //50x50
static const int iSmallButtons[] = 
{
	//IDC_TB_BTN_KADEMLIA,
	IDC_TB_BTN_MEDIAPLAYER, //>>> WiZaRd::MediaPlayer
	IDC_TB_BTN_SERVER,	
	IDC_TB_BTN_IRC,
	IDC_TB_BTN_STATISTIC,
	IDC_TB_BTN_PREFERENCES
};

//this is the complete "width" in pixels of all buttons including help
static const int iHelpOffSet = TB_BIGSMALLSPACE+ARRSIZE(iBigButtons)*BIG_BUTTON_SIZE+ARRSIZE(iSmallButtons)*SMALL_BUTTON_SIZE;
// X-Ray :: Toolbar :: End
/*TK4 Mod version 1.3a onward - handle hot key*/
LRESULT CemuleDlg::OnHotKey(WPARAM wp, LPARAM)
{   //if not our hotkey,(more than one hot key can be added); so return
	if(wp != 0x1515) return 0;
	//window hidden and not minimize to systray then show it
    if(!IsWindowVisible() && !m_bTrayIconVisible)
	  {
		  if(m_bTrayState){//was in systray state
			                TrayShow(); //Show systray mule
							if(m_bWasMiniMule)
							  { //if minimule was open when we hid re-open it
								m_pMiniMule = new CMiniMule(this);
								m_pMiniMule->Create(CMiniMule::IDD, this);
								m_pMiniMule->SetForegroundWindow();
								m_pMiniMule->BringWindowToTop();
							  }
						   } else//was full emule window or minimized, restore
							     ShowWindow(SW_SHOWNORMAL); //Show window
		} else {//Window is visible or minimized to systray hide it and note it's state
		        if(m_bTrayIconVisible)
				   {
					TrayHide(); //If minimize to tray hide systray mule
					m_bTrayState = true;
					if(m_pMiniMule)
					  {
						DestroyMiniMule(); //We don't want to leave minimule about to be see if user neglects to close it
						m_bWasMiniMule = true;//minimule to restore
					   } else //no minimule to restore
						     m_bWasMiniMule = false;
					} else {
							ShowWindow(SW_HIDE);  //Hide the window
							m_bTrayState = false;
							}
				  }
	 return 0;
 }
BOOL CemuleDlg::OnInitDialog()
{

	// X-Ray :: Toolbar :: Start
	m_co_ToolLeft.LoadImage(IDR_TB_TOOLBAR_LEFT, _T("JPG"));
	m_co_ToolMid.LoadImage(IDR_TB_TOOLBAR_CENTER, _T("JPG"));
	m_co_ToolRight.LoadImage(IDR_TB_TOOLBAR_RIGHT, _T("JPG"));
	// X-Ray :: Toolbar :: End

	m_bStartMinimized = thePrefs.GetStartMinimized();
	if (!m_bStartMinimized)
		m_bStartMinimized = theApp.DidWeAutoStart();

	// temporary disable the 'startup minimized' option, otherwise no window will be shown at all
	if (thePrefs.IsFirstStart())
		m_bStartMinimized = false;

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
	// CreateToolbarCmdIconMap(); // X-Ray :: Toolbar :: Cleanup

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL){
		pSysMenu->AppendMenu(MF_SEPARATOR);

		ASSERT( (MP_ABOUTBOX & 0xFFF0) == MP_ABOUTBOX && MP_ABOUTBOX < 0xF000);
		pSysMenu->AppendMenu(MF_STRING, MP_ABOUTBOX, GetResString(IDS_ABOUTBOX));

		// Tux: LiteMule: Remove Version Check
//	}

		// remaining system menu entries are created later...
	}

	// set title
	SetWindowText(MOD_VERSION );//+ " eMule v0.47c");// + theApp.m_strCurVersionLong ); // Maella -Support for tag ET_MOD_VERSION 0x55

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
	serverwnd->Create(IDD_SERVER);
	sharedfileswnd->Create(IDD_FILES);
	mediaplayerwnd->Create(IDD_MEDIAPLAYER); //>>> WiZaRd::MediaPlayer
	searchwnd->Create(this);
	chatwnd->Create(IDD_CHAT);
	transferwnd->Create(IDD_TRANSFER);
	statisticswnd->Create(IDD_STATISTICS);
	kademliawnd->Create(IDD_KADEMLIAWND);
	ircwnd->Create(IDD_IRC);

//>>> WiZaRd::WebBrowser [Pruna]	
	webbrowser->Create(IDD_WEBBROWSER);		//Added by thilon on 2006.08.01
//<<< WiZaRd::WebBrowser [Pruna]

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
		if(webbrowser!=NULL)
		{
			SetActiveDialog(webbrowser);
		}

	

	// if still no active window, activate server window
		//SetActiveDialog(serverwnd);
//<<< WiZaRd::WebBrowser [Pruna]

	SetAllIcons();
	Localize();
/*bool    CPreferneces::hotkey_enabled = false;				// TK4 mod
uint    CPreferneces::hotkey_mod = MOD_CONTROL + MOD_ALT;	// TK4 mod
uint    CPreferneces::hotkey_key = (uint)'H';				// TK4 mod
RegisterHotKey(m_hWnd, 0x1515, MOD_CONTROL + MOD_ALT, 'C'); */
	//TK4 Mod - Set Hot keys for Show / Hide
	thePrefs.HideWnd = m_hWnd;
	if(thePrefs.hotkey_enabled) RegisterHotKey(m_hWnd, 0x1515, thePrefs.hotkey_mod, thePrefs.hotkey_key); //TK4 Mod 1.3a onward - Register HotKey

	// set updateintervall of graphic rate display (in seconds)
	//ShowConnectionState(false);

	// X-Ray :: Toolbar :: Start
	/*
	// adjust all main window sizes for toolbar height and maximize the child windows
	CRect rcClient, rcToolbar, rcStatusbar;
	GetClientRect(&rcClient);
	pwndToolbarX->GetWindowRect(&rcToolbar);
	statusbar->GetWindowRect(&rcStatusbar);
	rcClient.top += rcToolbar.Height();
	rcClient.bottom -= rcStatusbar.Height();
	*/
	CRect rect;
	GetClientRect(&rect);

	CRect srect;
	statusbar->GetClientRect(&srect);
	statusbar->SetWindowPos(NULL, 0, rect.bottom-srect.Height(), rect.Width(), rect.Height(), SWP_NOZORDER);
	// X-Ray :: Toolbar :: End

	CWnd* apWnds[] =
	{
		serverwnd,
		kademliawnd,
		transferwnd,
		sharedfileswnd,
		mediaplayerwnd, //>>> WiZaRd::MediaPlayer
		searchwnd,
		chatwnd,
		ircwnd,
		statisticswnd,
		webbrowser	// Added by thilon on 2006.08.01
	};
	for (int i = 0; i < _countof(apWnds); i++)
		/*
		apWnds[i]->SetWindowPos(NULL, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER);
		*/
		apWnds[i]->SetWindowPos(NULL, 0, 66, rect.Width(), rect.Height()-66-srect.Height(), SWP_NOZORDER); // X-Ray :: Toolbar

	// X-Ray :: Speedgraph :: Start
	RECT rect1, rect2;

	// set updateintervall of graphic rate display (in seconds)
	rect1.left = rect.right-213;
	rect1.top = rect.top+9;
	rect1.right = rect1.left+206;
	rect1.bottom = rect1.top+26;

	rect2.left = rect.right-213;
	rect2.top = rect.top+36;
	rect2.right = rect2.left+206;
	rect2.bottom = rect2.top+26;

	RECT grect = {25,25,50,50};
	m_co_UpTrafficGraph.CreateEx(NULL, NULL ,NULL, WS_CHILD | WS_VISIBLE, grect, this, 1234, NULL);
	m_co_DownTrafficGraph.CreateEx(NULL, NULL ,NULL, WS_CHILD | WS_VISIBLE, grect, this, 1235, NULL);

	m_co_DownTrafficGraph.SetWindowPos(NULL, rect1.left, rect1.top, rect1.right-rect1.left, rect1.bottom-rect1.top, SWP_NOZORDER);
	m_co_UpTrafficGraph.SetWindowPos(NULL, rect2.left, rect2.top, rect2.right-rect2.left, rect2.bottom-rect2.top, SWP_NOZORDER);

	m_co_UpTrafficGraph.Init();
	m_co_DownTrafficGraph.Init();

	m_co_UpTrafficGraph.Init_Color(RGB(255, 0, 255), RGB(255, 0, 0), RGB( 0, 0, 64), RGB(96, 96, 128), RGB(192, 192, 255));
	m_co_DownTrafficGraph.Init_Color(RGB(255, 255, 0), RGB(0, 255, 0), RGB( 0, 0, 64), RGB(96, 96, 128), RGB(192, 192, 255));

	// Traffic graph
	SetSpeedMeterRange((int)thePrefs.GetMaxGraphUploadRate(), (int)thePrefs.GetMaxGraphDownloadRate());
	// X-Ray :: Speedgraph :: End


	// X-Ray :: Toolbar :: Start
	static CxSkinButton* xBigSkinButtons[] = 
	{		
		&m_co_TransferBtn,
		&m_co_SearchBtn,
		&m_co_FilesBtn
	};

	static CxSkinButton* xSmallSkinButtons[] = 
	{
		&m_co_MediaPlayerBtn, //>>> WiZaRd::MediaPlayer
		&m_co_ServerBtn,		
		&m_co_KademliaBtn,
		&m_co_MessagesBtn,
		&m_co_IrcBtn,
		&m_co_StatisticBtn,
		&m_co_PreferencesBtn
	};

	// big buttons
	int offset = 0;
	
	for(int i = 0; i < _countof(iBigButtons); ++i)
		GetDlgItem(iBigButtons[i])->SetWindowPos(NULL, offset+BIG_BUTTON_SIZE*i, 10, BIG_BUTTON_SIZE, BIG_BUTTON_SIZE-16, SWP_NOZORDER);

	// small buttons
	offset += 152+_countof(iBigButtons)*BIG_BUTTON_SIZE;
	for(int i = 0; i < _countof(iSmallButtons); ++i)
		GetDlgItem(iSmallButtons[i])->SetWindowPos(NULL, offset+SMALL_BUTTON_SIZE*i, 14, SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE, SWP_NOZORDER);

	RECT trect4 = {0, /*39*/BIG_BUTTON_SIZE-30, BIG_BUTTON_SIZE, BIG_BUTTON_SIZE-16};
	for(int i = 0; i < _countof(xBigSkinButtons); ++i)
		xBigSkinButtons[i]->Set_TextPos(trect4);

	m_co_ConnectBtn.SetSkin(IDB_TB_CONNECT_NORMAL, IDB_TB_CONNECT_NORMAL, IDB_TB_CONNECT_OVER, IDB_TB_CONNECT_NORMAL, 0, 0, 0, 0, 0);	
	m_co_TransferBtn.SetSkin(IDB_TB_TRANSFER_NORMAL, IDB_TB_TRANSFER_CLICK, IDB_TB_TRANSFER_OVER, IDB_TB_TRANSFER_CLICK, 0, 0, 0, 0, 0);
	m_co_SearchBtn.SetSkin(IDB_TB_SEARCH_NORMAL, IDB_TB_SEARCH_CLICK, IDB_TB_SEARCH_OVER, IDB_TB_SEARCH_CLICK, 0, 0, 0, 0, 0);
	m_co_FilesBtn.SetSkin(IDB_TB_FILES_NORMAL, IDB_TB_FILES_CLICK, IDB_TB_FILES_OVER, IDB_TB_FILES_CLICK, 0, 0, 0, 0, 0);

	RECT trect5 = {0, SMALL_BUTTON_SIZE-15, SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE};
	for(int i = 0; i < _countof(xSmallSkinButtons); ++i)
		xSmallSkinButtons[i]->Set_TextPos(trect5);

	m_co_MediaPlayerBtn.SetSkin(IDB_TB_MEDIAPLAYER_NORMAL, IDB_TB_MEDIAPLAYER_CLICK, IDB_TB_MEDIAPLAYER_OVER, IDB_TB_MEDIAPLAYER_CLICK, 0, 0, 0, 0, 0); //>>> WiZaRd::MediaPlayer
	m_co_ServerBtn.SetSkin(IDB_TB_SERVER_NORMAL, IDB_TB_SERVER_CLICK, IDB_TB_SERVER_OVER, IDB_TB_SERVER_CLICK, 0, 0, 0, 0, 0);
	m_co_MessagesBtn.SetSkin(IDB_TB_MESSAGES_NORMAL, IDB_TB_MESSAGES_CLICK, IDB_TB_MESSAGES_OVER, IDB_TB_MESSAGES_CLICK, 0, 0, 0, 0, 0);
	m_co_IrcBtn.SetSkin(IDB_TB_IRC_NORMAL, IDB_TB_IRC_CLICK, IDB_TB_IRC_OVER, IDB_TB_IRC_CLICK, 0, 0, 0, 0, 0);
	m_co_StatisticBtn.SetSkin(IDB_TB_STATISTIC_NORMAL, IDB_TB_STATISTIC_CLICK, IDB_TB_STATISTIC_OVER, IDB_TB_STATISTIC_CLICK, 0, 0, 0, 0, 0);
	m_co_PreferencesBtn.SetSkin(IDB_TB_PREFERENCES_NORMAL, IDB_TB_PREFERENCES_CLICK, IDB_TB_PREFERENCES_OVER, IDB_TB_PREFERENCES_CLICK, 0, 0, 0, 0, 0);
	// X-Ray :: Toolbar :: End

	// anchors
	AddAnchor(*serverwnd,		TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*kademliawnd,		TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*transferwnd,		TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*sharedfileswnd,	TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*mediaplayerwnd,	TOP_LEFT, BOTTOM_RIGHT); //>>> WiZaRd::MediaPlayer
    AddAnchor(*searchwnd,		TOP_LEFT, BOTTOM_RIGHT);
    AddAnchor(*chatwnd,			TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*ircwnd,			TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*statisticswnd,	TOP_LEFT, BOTTOM_RIGHT);
//>>> WiZaRd::WebBrowser [Pruna]
	AddAnchor(*webbrowser,      TOP_LEFT, BOTTOM_RIGHT); // Added by thilon on 2006.08.01
//<<< WiZaRd::WebBrowser [Pruna]
	AddAnchor(*statusbar,		BOTTOM_LEFT, BOTTOM_RIGHT);

	// X-Ray :: Speedgraph :: Start
	AddAnchor(m_co_UpTrafficGraph,TOP_RIGHT,TOP_RIGHT);
	AddAnchor(m_co_DownTrafficGraph,TOP_RIGHT,TOP_RIGHT);
	// X-Ray :: Speedgraph :: End

	statisticswnd->ShowInterval();

	// tray icon
	TraySetMinimizeToTray(thePrefs.GetMinTrayPTR());
	TrayMinimizeToTrayChange();

	ShowTransferRate(true);
    ShowPing();
	searchwnd->UpdateCatTabs();

	theApp.emuledlg->Update_TrafficGraph(); // X-Ray :: Speedgraph

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
//>>> WiZaRd::First Start Maximized
		if(thePrefs.IsFirstStart())
		{
			wp.flags = WPF_RESTORETOMAXIMIZED;
			wp.showCmd = SW_MAXIMIZE;
		}
		else
//<<< WiZaRd::First Start Maximized
		if (wp.showCmd != SW_SHOWMAXIMIZED)
			wp.showCmd = SW_SHOWNORMAL;
		m_bStartMinimizedChecked = true;
	}
	SetWindowPlacement(&wp);

	if (thePrefs.GetWSIsEnabled())
		theApp.webserver->StartServer();
	// Tux: LiteMule: Remove MobileMule

	VERIFY( (m_hTimer = ::SetTimer(NULL, NULL, 300, StartupTimer)) != NULL );
	if (thePrefs.GetVerbose() && !m_hTimer)
		AddDebugLogLine(true,_T("Failed to create 'startup' timer - %s"),GetErrorMessage(GetLastError()));

	theStats.starttime = GetTickCount();

	//Xman official UPNP removed
	/*
	// Start UPnP prot forwarding
	if (theApp.m_pUPnPFinder != NULL && thePrefs.IsUPnPEnabled()){
		try
		{
			if (theApp.m_pUPnPFinder->AreServicesHealthy()){
				theApp.m_pUPnPFinder->SetMessageOnResult(GetSafeHwnd(), UM_UPNP_RESULT);
				VERIFY( (m_hUPnPTimeOutTimer = ::SetTimer(NULL, NULL, SEC2MS(30), UPnPTimeOutTimer)) != NULL );
				theApp.m_pUPnPFinder->StartDiscovery(thePrefs.GetPort(), thePrefs.GetUDPPort());
			}
		}
		catch ( CUPnPFinder::UPnPError& ) {}
		catch ( CException* e ) { e->Delete(); }
	}
	*/

	if (thePrefs.IsFirstStart())
	{
		// temporary disable the 'startup minimized' option, otherwise no window will be shown at all
		m_bStartMinimized = false;

		//Xman
		// SLUGFILLER: SafeHash remove - wait until emule is ready before opening the wizard
		/*
		//Xman new slpash-screen arrangement
		//DestroySplash();
		theApp.DestroySplash();

		extern BOOL FirstTimeWizard();
		if (FirstTimeWizard()){
			// start connection wizard
			CConnectionWizardDlg conWizard;
			conWizard.DoModal();
		}
				*/
	}

	VERIFY( m_pDropTarget->Register(this) );

	// initalize PeerCache
	theApp.m_pPeerCache->Init(thePrefs.GetPeerCacheLastSearch(), thePrefs.WasPeerCacheFound(), thePrefs.IsPeerCacheDownloadEnabled(), thePrefs.GetPeerCachePort());

	//Xman
	// SiRoB: SafeHash fix (see StartupTimer)
	/*
	// start aichsyncthread
	AfxBeginThread(RUNTIME_CLASS(CAICHSyncThread), THREAD_PRIORITY_BELOW_NORMAL,0);
	*/

	// debug info
	DebugLog(_T("Using '%s' as config directory"), thePrefs.GetMuleDirectory(EMULE_CONFIGDIR)); 
	return TRUE;
}

// modders: dont remove or change the original versioncheck! (additionals are ok)

// Tux: LiteMule: Remove Version Check: removed code block

void CALLBACK CemuleDlg::StartupTimer(HWND /*hwnd*/, UINT /*uiMsg*/, UINT /*idEvent*/, DWORD /*dwTime*/)
{
	//Xman
	// SLUGFILLER: doubleLucas - not ready to init, come back next cycle
	if (!::IsWindow(theApp.emuledlg->m_hWnd))
		return;
	if (!::IsWindow(theApp.emuledlg->sharedfileswnd->sharedfilesctrl.m_hWnd))
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
				theApp.emuledlg->ready = true;
				//Xman
				// SLUGFILLER: SafeHash remove - moved down
				/*
				theApp.sharedfiles->SetOutputCtrl(&theApp.emuledlg->sharedfileswnd->sharedfilesctrl);
				*/
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
				theApp.UpdateSplash(GetResString(IDS_SPLASH_LOADING8)); //Xman new slpash-screen arrangement

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
				if(!theApp.listensocket->StartListening()){
					CString strError;
					strError.Format(GetResString(IDS_MAIN_SOCKETERROR), thePrefs.GetPort());
					LogError(LOG_STATUSBAR, _T("%s"), strError);
					if (thePrefs.GetNotifierOnImportantError())
						theApp.emuledlg->ShowNotifier(strError, TBN_IMPORTANTEVENT);
					bError = true;
				}
				if(!theApp.clientudp->Create()){
					CString strError;
					strError.Format(GetResString(IDS_MAIN_SOCKETERROR), thePrefs.GetUDPPort());
					LogError(LOG_STATUSBAR, _T("%s"), strError);
					if (thePrefs.GetNotifierOnImportantError())
						theApp.emuledlg->ShowNotifier(strError, TBN_IMPORTANTEVENT);
				}

				if (!bError) // show the success msg, only if we had no serious error
				{
					//<<< eWombat [WINSOCK2] for Pawcio: BC
					AddLogLine(false,_T("***************Winsock***************"));
					AddLogLine(false,_T("Winsock: Versão %d.%d [%.40s] %.40s"), HIBYTE( theApp.m_wsaData.wVersion ),LOBYTE(theApp.m_wsaData.wVersion ),
						CString(theApp.m_wsaData.szDescription), CString(theApp.m_wsaData.szSystemStatus));
					if (theApp.m_wsaData.iMaxSockets!=0)
						AddLogLine(false,_T("Winsock: max. sockets %d"), theApp.m_wsaData.iMaxSockets);
					else
						AddLogLine(false,_T("Winsock: ilimitados sockets"));
					AddLogLine(false,_T("***************Winsock***************"));
					//>>> eWombat [WINSOCK2]
					AddLogLine(true, GetResString(IDS_MAIN_READY), MOD_VERSION); // XMan // Maella -Support for tag ET_MOD_VERSION 0x55
				}
				//Xman
				// SLUGFILLER: SafeHash remove - moved down
				/*
				if(thePrefs.DoAutoConnect())
					theApp.emuledlg->OnBnClickedButton2();
				*/
				theApp.emuledlg->status++;
				break;
			}
			case 5:
				break;
			//Xman
			// BEGIN SLUGFILLER: SafeHash - delay load shared files
			case 6:
				theApp.emuledlg->status++;
				//Xman remove unused AICH-hashes
				theApp.m_AICH_Is_synchronizing=true;
				theApp.sharedfiles->SetOutputCtrl(&theApp.emuledlg->sharedfileswnd->sharedfilesctrl);
				theApp.emuledlg->sharedfileswnd->historylistctrl.Init(); //Xman [MoNKi: -Downloaded History-]

				// BEGIN SiRoB: SafeHash fix originaly in OnInitDialog (delay load shared files)
				// start aichsyncthread
				theApp.UpdateSplash(GetResString(IDS_SPLASH_LOADING9)); //Xman new slpash-screen arrangement
				AfxBeginThread(RUNTIME_CLASS(CAICHSyncThread), THREAD_PRIORITY_BELOW_NORMAL,0);
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
				theApp.UpdateSplash(GetResString(IDS_SPLASH_LOADING10)); //Xman new slpash-screen arrangement
				//autoconnect only after emule loaded completely
				//if(thePrefs.DoAutoConnect())
				theApp.emuledlg->OnBnClickedButton2();
				theApp.emuledlg->Mostar();
				// wait until emule is ready before opening the wizard
				if (thePrefs.IsFirstStart())
				{
					//Xman new slpash-screen arrangement
					//DestroySplash();
					theApp.DestroySplash();
					//Xman end
					extern BOOL FirstTimeWizard();
					if (FirstTimeWizard()){
						// start connection wizard
						CConnectionWizardDlg conWizard;
						conWizard.DoModal();
					}
				}
			// END SLUGFILLER: SafeHash
				theApp.emuledlg->StopTimer();
		}
	}
	CATCH_DFLT_EXCEPTIONS(_T("CemuleDlg::StartupTimer"))
		// Maella -Code Improvement-
		// Remark: The macro CATCH_DFLT_EXCEPTIONS will not catch all types of exception.
		//         The exceptions thrown in callback function are not intercepted by the dbghelp.dll (e.g. eMule Dump, crashRpt, etc...)
		catch(...) {
			if(theApp.emuledlg != NULL)
				AddLogLine(true, _T("Falha %s em "), __FUNCTION__);
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
	// Tux: LiteMule: Remove Version Check

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
		// Tux: LiteMule: Remove Version Check
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
	else{
		CPaintDC dc(this); // X-Ray :: Toolbar
		CTrayDialog::OnPaint();

		// X-Ray :: Toolbar :: Start
		CRect rect;
		GetClientRect(&rect);

		BITMAP bmpInfo1;
		m_co_ToolLeft.GetBitmap(&bmpInfo1);
		CDC dcMemory1;
		dcMemory1.CreateCompatibleDC(&dc);
		CBitmap* pOldBitmap1 = dcMemory1.SelectObject(&m_co_ToolLeft);
		dc.BitBlt(rect.left, rect.top, bmpInfo1.bmWidth, bmpInfo1.bmHeight, &dcMemory1, 0, 0, SRCCOPY);

		BITMAP bmpInfo2;
		m_co_ToolRight.GetBitmap(&bmpInfo2);
		CDC dcMemory2;
		dcMemory2.CreateCompatibleDC(&dc);
		CBitmap* pOldBitmap2 = dcMemory2.SelectObject(&m_co_ToolRight);
		dc.BitBlt(rect.right-bmpInfo2.bmWidth, rect.top, bmpInfo2.bmWidth, bmpInfo2.bmHeight, &dcMemory2, 0, 0, SRCCOPY);

		BITMAP bmpInfo3;
		m_co_ToolMid.GetBitmap(&bmpInfo3);
		CDC dcMemory3;
		dcMemory3.CreateCompatibleDC(&dc);
		CBitmap* pOldBitmap3 = dcMemory3.SelectObject(&m_co_ToolMid);
		dc.StretchBlt(rect.left+bmpInfo1.bmWidth, rect.top, rect.Width()-bmpInfo1.bmWidth-bmpInfo2.bmWidth, bmpInfo3.bmHeight, &dcMemory3, 0, 0, bmpInfo3.bmWidth, bmpInfo3.bmHeight, SRCCOPY);

		dcMemory1.SelectObject(pOldBitmap1);
		dcMemory2.SelectObject(pOldBitmap2);
		dcMemory3.SelectObject(pOldBitmap3);
		// X-Ray :: Toolbar :: End
	}
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

void CemuleDlg::ResetDebugLog(){
	serverwnd->debuglog->Reset();
}

//Xman Anti-Leecher-Log
void CemuleDlg::ResetLeecherLog()
{
	serverwnd->leecherlog->Reset();
}
//Xman end

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
		{
			theApp.DestroySplash(); //Xman new slpash-screen arrangement
			AfxMessageBox(pszText);
		}
	}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	Debug(_T("%s\n"), pszText);
#endif

	if ((uFlags & LOG_DEBUG) && !thePrefs.GetVerbose())
		return;

	//Xman Anti-Leecher-Log
	if ((uFlags & LOG_LEECHER) && !thePrefs.GetVerbose())
		return;


	TCHAR temp[1060];
	int iLen = _sntprintf(temp, _countof(temp), _T("%s: %s\r\n"), CTime::GetCurrentTime().Format(thePrefs.GetDateTimeFormat4Log()), pszText);
	if (iLen >= 0)
	{
		if (!(uFlags & LOG_DEBUG) && !(uFlags & LOG_LEECHER)) //Xman Anti-Leecher-Log
		{
			serverwnd->logbox->AddTyped(temp, iLen, uFlags & LOGMSGTYPEMASK);
			if (IsWindow(serverwnd->StatusSelector) && serverwnd->StatusSelector.GetCurSel() != CServerWnd::PaneLog)
				serverwnd->StatusSelector.HighlightItem(CServerWnd::PaneLog, TRUE);
			if (!(uFlags & LOG_DONTNOTIFY) && ready)
				ShowNotifier(pszText, TBN_LOG);
			if (thePrefs.GetLog2Disk())
				theLog.Log(temp, iLen);
		}
		else
		//Xman Anti-Leecher-Log
		if (thePrefs.GetVerbose() && (uFlags & LOG_LEECHER) )
		{
			serverwnd->leecherlog->AddTyped(temp, iLen, uFlags & LOGMSGTYPEMASK);
			if (IsWindow(serverwnd->StatusSelector) && serverwnd->StatusSelector.GetCurSel() != CServerWnd::PaneLeecherLog)
				serverwnd->StatusSelector.HighlightItem(CServerWnd::PaneLeecherLog, TRUE);

			if (thePrefs.GetDebug2Disk())
				theVerboseLog.Log(temp, iLen);
		}
		else
		//Xman end
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
	// X-Ray :: Statusbar :: Start
	/*
	UINT uIconIdx = GetConnectionStateIconIndex();
	if (uIconIdx >= ARRSIZE(connicons)){
		ASSERT(0);
		uIconIdx = 0;
	}
	statusbar->SetIcon(SBarConnected, connicons[uIconIdx]);
	*/
	if (theApp.serverconnect->IsConnected()){
		if (theApp.serverconnect->IsLowID())
			statusbar->SetIcon(SBarServer, connicons[2]);	// LowID eD2k
		else
			statusbar->SetIcon(SBarServer, connicons[1]);	// HighID eD2k
	}
	else statusbar->SetIcon(SBarServer, connicons[0]);		// NotConnect eD2k

	if (Kademlia::CKademlia::IsConnected()){
		if (Kademlia::CKademlia::IsFirewalled())
			statusbar->SetIcon(SBarKad, connicons[4]);		// LowID Kad
		else
			statusbar->SetIcon(SBarKad, connicons[3]);		// HighID Kad
	}
	else statusbar->SetIcon(SBarKad, connicons[5]);			// NotConnect Kad
	// X-Ray :: Statusbar :: End
}

// X-Ray :: Statusbar :: Start
/*
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
*/
// X-Ray :: Statusbar :: End

void CemuleDlg::ShowConnectionState()
{
	theApp.downloadqueue->OnConnectionState(theApp.IsConnected());
	serverwnd->UpdateMyInfo();
	serverwnd->UpdateControlsState();
	kademliawnd->UpdateControlsState();

	ShowConnectionStateIcon();
	// statusbar->SetText(GetConnectionStateString(), SBarConnected, 0); // X-Ray :: Statusbar

	// X-Ray :: Statusbar :: Start
	CString strStatusEd2k;
	CString strStatusKad;
	if(theApp.serverconnect->IsConnected())
		strStatusEd2k = "Lig.";
	else if (theApp.serverconnect->IsConnecting())
		strStatusEd2k = "...";
	else
		strStatusEd2k = "Des.";

	//Most likley needs a rewrite
	if(Kademlia::CKademlia::IsConnected())
		strStatusKad = "Lig.";
	else if (Kademlia::CKademlia::IsRunning())
		strStatusKad = "...";
	else
		strStatusKad = "Des.";

	statusbar->SetTipText(SBarServer, strStatusEd2k);
	statusbar->SetText(strStatusEd2k, SBarServer, 0);

	statusbar->SetTipText(SBarKad, strStatusKad);
	statusbar->SetText(strStatusKad, SBarKad, 0);
	// X-Ray :: Statusbar :: End

	if (theApp.IsConnected())
	{
		// X-Ray :: Toolbar :: Start
		/*
		CString strPane(GetResString(IDS_MAIN_BTN_DISCONNECT));
		TBBUTTONINFO tbi;
		tbi.cbSize = sizeof(TBBUTTONINFO);
		tbi.dwMask = TBIF_IMAGE | TBIF_TEXT;
		tbi.iImage = 1;
		tbi.pszText = const_cast<LPTSTR>((LPCTSTR)strPane);
		toolbar->SetButtonInfo(TBBTN_CONNECT, &tbi);
		*/
		//SetDlgItemText(IDC_TB_BTN_CONNECT, _T("Dis&connect")); // hardcoded looks better
		//m_co_ConnectBtn.SetSkin(IDB_TB_CONNECT_CLICK, IDB_TB_CONNECT_CLICK, IDB_TB_DISCONNECT_OVER, IDB_TB_CONNECT_CLICK, 0, 0, 0, 0, 0);
		// X-Ray :: Toolbar :: End
	}
	else
	{
		if (theApp.serverconnect->IsConnecting() || Kademlia::CKademlia::IsRunning())
		{
			// X-Ray :: Toolbar :: Start
			/*
			CString strPane(GetResString(IDS_MAIN_BTN_CANCEL));
			TBBUTTONINFO tbi;
			tbi.cbSize = sizeof(TBBUTTONINFO);
			tbi.dwMask = TBIF_IMAGE | TBIF_TEXT;
			tbi.iImage = 2;
		tbi.pszText = const_cast<LPTSTR>((LPCTSTR)strPane);
			toolbar->SetButtonInfo(TBBTN_CONNECT, &tbi);
			*/
			// X-Ray :: Toolbar :: End			
			ShowUserCount();
		}
		else
		{
			// X-Ray :: Toolbar :: Start
			/*
			CString strPane(GetResString(IDS_MAIN_BTN_CONNECT));
			TBBUTTONINFO tbi;
			tbi.cbSize = sizeof(TBBUTTONINFO);
			tbi.dwMask = TBIF_IMAGE | TBIF_TEXT;
			tbi.iImage = 0;
			tbi.pszText = const_cast<LPTSTR>((LPCTSTR)strPane);
			toolbar->SetButtonInfo(TBBTN_CONNECT, &tbi);
			*/
			// X-Ray :: Toolbar :: End
			ShowUserCount();
		}

	}
}

void CemuleDlg::ShowUserCount()
{
	uint32 totaluser, totalfile;
	totaluser = totalfile = 0;
	theApp.serverlist->GetUserFileStatus( totaluser, totalfile );
	CString buffer;

	// X-Ray :: Statusbar :: Start
	/*
	buffer.Format(_T("%s:%s(%s)|%s:%s(%s)"), GetResString(IDS_UUSERS), CastItoIShort(totaluser, false, 1), CastItoIShort(Kademlia::CKademlia::GetKademliaUsers(), false, 1), GetResString(IDS_FILES), CastItoIShort(totalfile, false, 1), CastItoIShort(Kademlia::CKademlia::GetKademliaFiles(), false, 1));
	statusbar->SetText(buffer, SBarUsers, 0);
	*/

	if (thePrefs.GetNetworkED2K() == 1 && thePrefs.GetNetworkKademlia() == 1 ) 
		buffer.Format( _T("%s(%s)") ,CastItoIShort(totaluser,false,1), CastItoIShort(Kademlia::CKademlia::GetKademliaUsers(),false,1) );
	else if (thePrefs.GetNetworkED2K() != 1 && thePrefs.GetNetworkKademlia() == 1 ) 
		buffer.Format( _T("Kad: %s") , CastItoIShort(Kademlia::CKademlia::GetKademliaUsers(),false,1));
	else if (thePrefs.GetNetworkED2K() == 1 && thePrefs.GetNetworkKademlia() != 1 ) 
		buffer.Format( _T("eD2k: %s") ,CastItoIShort(totaluser,false,1));
	else
		buffer.Format( _T("No Network Selected"));

	SetStatusBarPartsSize();
	statusbar->SetText(buffer,SBarUsers,0);

	buffer.Format( _T("%s"), CastItoIShort(totalfile,false,1));
	SetStatusBarPartsSize();
	statusbar->SetText(buffer,SBarFiles,0);
	// X-Ray :: Statusbar :: End
}

void CemuleDlg::ShowMessageState(UINT iconnr)
{
	m_iMsgIcon = iconnr;
	// X-Ray :: Statusbar :: Start
	/*
	statusbar->SetIcon(SBarChatMsg, imicons[m_iMsgIcon]);
	*/
	statusbar->SetIcon(SBarMessage, imicons[m_iMsgIcon]);
	// X-Ray :: Statusbar :: End
}

void CemuleDlg::ShowTransferStateIcon()
{
	// X-Ray :: Statusbar :: Start
	/*
	if (m_uUpDatarate && m_uDownDatarate)
		statusbar->SetIcon(SBarUpDown, transicons[3]);
	else if (m_uUpDatarate)
		statusbar->SetIcon(SBarUpDown, transicons[2]);
	else if (m_uDownDatarate)
		statusbar->SetIcon(SBarUpDown, transicons[1]);
	else
		statusbar->SetIcon(SBarUpDown, transicons[0]);
	*/
	statusbar->SetIcon(SBarUp,transicons[0]);
	statusbar->SetIcon(SBarDown,transicons[1]);

//>>> WiZaRd::ShutDownIfFinished
	if (sysinfoicon) VERIFY( ::DestroyIcon(sysinfoicon) );
	sysinfoicon = theApp.LoadIcon(thePrefs.IsShutDownIfFinished() ? L"CONTACT0" : L"CONTACT4", 16, 16);
	statusbar->SetIcon(SBarSysInfo,sysinfoicon);
//<<< WiZaRd::ShutDownIfFinished
	// X-Ray :: Statusbar :: End
}

CString CemuleDlg::GetUpDatarateString(UINT /*uUpDatarate*/)
{
	//Xman Code Improvement
	//it's enough to update the datarate in ShowTransferrate
	//m_uUpDatarate = uUpDatarate != (UINT)-1 ? uUpDatarate : theApp.uploadqueue->GetDatarate();
	//Xman end
	TCHAR szBuff[128];
	//Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	if (thePrefs.ShowOverhead())
	{
		_sntprintf(szBuff, _countof(szBuff), _T("%.1f (%.1f)"), (float)m_uUpDatarate/1024, (float)m_uploadOverheadRate/1024);
		szBuff[_countof(szBuff) - 1] = _T('\0');
	}
	else
	//xman end
	{
		_sntprintf(szBuff, _countof(szBuff), _T("%.1f"), (float)m_uUpDatarate/1024);
		szBuff[_countof(szBuff) - 1] = _T('\0');
	}
	return szBuff;
}

CString CemuleDlg::GetDownDatarateString(UINT /*uDownDatarate*/)
{
	//Xman Code Improvement
	//it's enough to update the datarate in ShowTransferrate
	//m_uDownDatarate = uDownDatarate != (UINT)-1 ? uDownDatarate : theApp.downloadqueue->GetDatarate();
	//Xman end

	TCHAR szBuff[128];
	//Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	if (thePrefs.ShowOverhead())
	{
		_sntprintf(szBuff, _countof(szBuff), _T("%.1f (%.1f)"), (float)m_uDownDatarate/1024, (float)m_downloadOverheadRate/1024);
		szBuff[_countof(szBuff) - 1] = _T('\0');
	}
	else
	//Xman end
	{
		_sntprintf(szBuff, _countof(szBuff), _T("%.1f"), (float)m_uDownDatarate/1024);
		szBuff[_countof(szBuff) - 1] = _T('\0');
	}
	return szBuff;
}

CString CemuleDlg::GetTransferRateString()
{
	TCHAR szBuff[128];
	//Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	if (thePrefs.ShowOverhead())
	{
		_sntprintf(szBuff, _countof(szBuff), GetResString(IDS_UPDOWN),
				  (float)m_uUpDatarate/1024, (float)m_uploadOverheadRate/1024,
				  (float)m_uDownDatarate/1024, (float)m_downloadOverheadRate/1024);
		szBuff[_countof(szBuff) - 1] = _T('\0');
	}
	//Xman end
	else
	{
		_sntprintf(szBuff, _countof(szBuff), GetResString(IDS_UPDOWNSMALL), (float)m_uUpDatarate/1024, (float)m_uDownDatarate/1024);
		szBuff[_countof(szBuff) - 1] = _T('\0');
	}
	return szBuff;
}

void CemuleDlg::ShowTransferRate(bool bForceAll)
{
	if (bForceAll)
		m_uLastSysTrayIconCookie = SYS_TRAY_ICON_COOKIE_FORCE_UPDATE;

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
	// X-Ray :: Statusbar :: Start
	TCHAR buffer[100];
	TCHAR buffer1[100];
	uint32 m_uLastUpRateOverhead = m_uploadOverheadRate;
	uint32 m_uLastDownRateOverhead = m_downloadOverheadRate;

	if( thePrefs.ShowOverhead())
	{
		_stprintf(buffer, _T("U: %.1f(%.1f)"), (float)m_uUpDatarate/1024, (float)m_uLastUpRateOverhead/1024);
		_stprintf(buffer1, _T("D: %.1f(%.1f)"), (float)m_uDownDatarate/1024, (float)m_uLastDownRateOverhead/1024);
	}
	else
	{
		_stprintf(buffer, _T("U: %.1f"), (float)m_uUpDatarate/1024);
		_stprintf(buffer1, _T("D: %.1f"), (float)m_uDownDatarate/1024);
	}
	// X-Ray :: Statusbar :: End

	CString strTransferRate = GetTransferRateString();
	if (TrayIsVisible() || bForceAll)
	{
		TCHAR buffer2[64];
		// set trayicon-icon
		int iDownRateProcent = (int)ceil((m_uDownDatarate/10.24) / thePrefs.GetMaxGraphDownloadRate());
		if (iDownRateProcent > 100)
			iDownRateProcent = 100;
		UpdateTrayIcon(iDownRateProcent);

		if (theApp.IsConnected())
		{
			_sntprintf(buffer2, _countof(buffer2), _T("DreaMule (%s)\r\n%s"), GetResString(IDS_CONNECTED), strTransferRate);
			buffer2[_countof(buffer2) - 1] = _T('\0');
		}
		else
		{
			_sntprintf(buffer2, _countof(buffer2), _T("DreaMule (%s)\r\n%s"), GetResString(IDS_DISCONNECTED), strTransferRate);
			buffer2[_countof(buffer2) - 1] = _T('\0');
		}

		// Win98: '\r\n' is not displayed correctly in tooltip
		if (afxData.bWin95) {
			LPTSTR psz = buffer2;
			while (*psz) {
				if (*psz == _T('\r') || *psz == _T('\n'))
					*psz = _T(' ');
				psz++;
			}
		}
		TraySetToolTip(buffer2);
	}

	//Xman see all sources
	if (activewnd == transferwnd && IsWindowVisible()){
		transferwnd->downloadlistctrl.ShowFilesCount();
	}
	//Xman end


	if (IsWindowVisible() || bForceAll)
	{
		//Xman GlobalMaxHarlimit for fairness
		if(theApp.downloadqueue->GetLimitState()==1)
			strTransferRate.Append(_T(" r"));
		else if(theApp.downloadqueue->GetLimitState()>=2)
			strTransferRate.Append(_T(" R"));
		//Xman end
		// X-Ray :: Statusbar :: Start
		/*
		statusbar->SetText(strTransferRate, SBarUpDown, 0);
		ShowTransferStateIcon();
		*/
		statusbar->SetText(buffer,SBarUp,0);
		statusbar->SetText(buffer1,SBarDown,0);

		TCHAR buffer2[100];
//>>> WiZaRd::ShutDownIfFinished
//		_sntprintf(buffer2,ARRSIZE(buffer2), MOD_VERSION);
		_sntprintf(buffer2,ARRSIZE(buffer2), GetResString(IDS_SHUTDOWNIFFINISHED));
//<<< WiZaRd::ShutDownIfFinished
		statusbar->SetText(buffer2,SBarSysInfo,0);

		ShowTransferStateIcon();
		// X-Ray :: Statusbar :: End
	}
	if (IsWindowVisible() && thePrefs.ShowRatesOnTitle())
	{
//dreamule mod name show
		TCHAR szBuff[128];
		_sntprintf(szBuff, _countof(szBuff), _T("(U:%.1f D:%.1f)%s"), (float)m_uUpDatarate/1024, (float)m_uDownDatarate/1024, MOD_VERSION);
//dreamule mod name show
		szBuff[_countof(szBuff) - 1] = _T('\0');
		SetWindowText(szBuff);
	}
	if (m_pMiniMule && m_pMiniMule->m_hWnd && m_pMiniMule->IsWindowVisible() && !m_pMiniMule->GetAutoClose())
	{
		m_pMiniMule->UpdateContent(m_uUpDatarate, m_uDownDatarate);
	}
}

void CemuleDlg::ShowPing()
{
    /* Xman
    if (IsWindowVisible())
	{
        CString buffer;
        if (thePrefs.IsDynUpEnabled())
		{
			//Xman
			//CurrentPingStruct lastPing = theApp.lastCommonRouteFinder->GetCurrentPing();
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
		statusbar->SetText(buffer, SBarChatMsg, 0);
    }
	*/
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
		if (*thePrefs.GetMinTrayPTR())
		{
			TrayShow();
			ShowWindow(SW_HIDE);
		}
		else
		{
			ShowWindow(SW_MINIMIZE);
		}
		ShowTransferRate();
		ShowPing();
	}

void CemuleDlg::SetActiveDialog(CWnd* dlg)
{
	if (dlg == activewnd)
		return;
	// X-Ray :: Toolbar :: Start
	static CxSkinButton* lastBtn = NULL;
	if(lastBtn)
		lastBtn->EnableWindow(TRUE);
	// X-Ray :: Toolbar :: End
	if (activewnd)
		activewnd->ShowWindow(SW_HIDE);
	//boizaum not focus on reload webbrowser
	if (dlg != webbrowser)
	{
	dlg->ShowWindow(SW_SHOW);
	dlg->SetFocus();
	}
	//boizaum not focus on reload webbrowser
	activewnd = dlg;
	// X-Ray :: Toolbar :: Cleanup :: Start
	/*
	int iToolbarButtonID = MapWindowToToolbarButton(dlg);
	if (iToolbarButtonID != -1)
		toolbar->PressMuleButton(iToolbarButtonID);
	*/
	// X-Ray :: Toolbar :: Cleanup :: End
	//>>> WiZaRd::WebBrowser [Pruna]
	if(activewnd != webbrowser)
		webbrowser->ShowWindow(SW_HIDE);
//<<< WiZaRd::WebBrowser [Pruna]
	if (dlg == transferwnd){
		if (thePrefs.ShowCatTabInfos())
			transferwnd->UpdateCatTabTitles();
		// X-Ray :: Toolbar :: Start
		m_co_TransferBtn.EnableWindow(FALSE); 
		lastBtn = &m_co_TransferBtn;
		// X-Ray :: Toolbar :: End
	}
	// X-Ray :: Toolbar :: Start
	else if (dlg == serverwnd)
	{
		m_co_ServerBtn.EnableWindow(FALSE);
		lastBtn = &m_co_ServerBtn;
	}
	else if (dlg == kademliawnd)
	{
		m_co_KademliaBtn.EnableWindow(FALSE);
		lastBtn = &m_co_KademliaBtn;
	}
	else if (dlg == searchwnd)
	{
		m_co_SearchBtn.EnableWindow(FALSE);
		lastBtn = &m_co_SearchBtn;
	}
	else if (dlg == sharedfileswnd)
	{
		m_co_FilesBtn.EnableWindow(FALSE);
		lastBtn = &m_co_FilesBtn;
	}
//>>> WiZaRd::MediaPlayer
	else if (dlg == mediaplayerwnd)
	{
		mediaplayerwnd->Refresh();
		m_co_MediaPlayerBtn.EnableWindow(FALSE);
		lastBtn = &m_co_MediaPlayerBtn;
	}
//<<< WiZaRd::MediaPlayer
	else if (dlg == ircwnd)
	{
		m_co_IrcBtn.EnableWindow(FALSE);
		lastBtn = &m_co_IrcBtn;
	}
	// X-Ray :: Toolbar :: End
	else if (dlg == chatwnd){
		chatwnd->chatselector.ShowChat();
		// X-Ray :: Toolbar :: Start
		m_co_MessagesBtn.EnableWindow(FALSE); 
		lastBtn = &m_co_MessagesBtn;
		// X-Ray :: Toolbar :: End
	}
	else if (dlg == statisticswnd){
		statisticswnd->ShowStatistics();
		// X-Ray :: Toolbar :: Start
		m_co_StatisticBtn.EnableWindow(FALSE);
		lastBtn = &m_co_StatisticBtn;
		// X-Ray :: Toolbar :: End
	}
//>>> WiZaRd::WebBrowser [Pruna]
	else if (dlg == webbrowser)
	{
		webbrowser->ShowWindow(SW_SHOW);
//>>> WiZaRd::Toolbar
		lastBtn = NULL; //do we need another button?
//<<< WiZaRd::Toolbar
	}
//<<< WiZaRd::WebBrowser [Pruna]
}

void CemuleDlg::boi()
{
SetActiveDialog(kademliawnd);
}
void CemuleDlg::SetStatusBarPartsSize()
{
	// X-Ray :: FiXeS :: Workaround :: Start :: WiZaRd
	//if startup takes too long then we might run into invalid calls here...
	if(!statusbar || !statusbar->m_hWnd)
		return;
	// X-Ray :: FiXeS :: Workaround :: End :: WiZaRd

	CRect rect;
	statusbar->GetClientRect(&rect);
	// X-Ray :: Statusbar :: Start
	int ovhShift = 0;	// Shift if Display OverHead
	int ousrShift = 0;	// Shift if Display only Users
	int kadShift = 0;	// Shift if Kad not activated
	int ed2kShift = 0;	// Shift if eD2k not activated
	// X-Ray :: Statusbar :: End
	int ussShift = 0;
	// X-Ray :: Statusbar :: Start
	// Shift if Kad not activated
	if (thePrefs.GetNetworkKademlia() != 1) {kadShift = 45;}

	// Shift if eD2K not activated
	if (thePrefs.GetNetworkED2K() != 1) {ed2kShift = 45;}

	// Shift if Display OverHead
	if(thePrefs.ShowOverhead() == 1){ovhShift = 35;}
	// X-Ray :: Statusbar :: End

	//Xman
	//only used for with USS
	/*
	if(thePrefs.IsDynUpEnabled())
	{
        if (thePrefs.IsDynUpUseMillisecondPingTolerance())
            ussShift = 45;
        else
            ussShift = 90;
        }
	*/
	//Xman changed
	/*
	int aiWidths[5] =
	{
		rect.right - 675 - ussShift,
		rect.right - 440 - ussShift,
		rect.right - 250 - ussShift,
		rect.right -  25 - ussShift,
		-1
	};
	*/
	int messSize = 25;	// Status Bar : Message Size
	int kadSize = 45;	// Status Bar : Kad Size
	int ed2kSize = 45;	// Status Bar : eD2k Size
	int dlSize = 70;	// Status Bar : Download Size
	int ulSize = 70;	// Status Bar : Upload Size
	int usSize = 90;	// Status Bar : Users Size
	int flSize = 80;	// Status Bar : Files Size
//>>> WiZaRd::ShutDownIfFinished
//	int sysinfo = 155;
	int sysinfo = 212;
//<<< WiZaRd::ShutDownIfFinished

	if (thePrefs.GetNetworkED2K() == 0 && thePrefs.GetNetworkKademlia() == 0 )
		usSize += 20;

	int aiWidths[9] = { rect.right-sysinfo-usSize-flSize -ulSize -dlSize -ed2kSize -kadSize -messSize +kadShift+ed2kShift -(ovhShift*2) +ousrShift -ussShift,//sysinfo 
		rect.right-usSize-flSize -ulSize -dlSize -ed2kSize -kadSize -messSize +kadShift+ed2kShift -(ovhShift*2) +ousrShift -ussShift, //users 
		rect.right-flSize-ulSize -dlSize -ed2kSize -kadSize -messSize +kadShift +ed2kShift -(ovhShift*2) +ousrShift -ussShift, //files 
		rect.right-ulSize-dlSize -ed2kSize -kadSize -messSize +kadShift +ed2kShift -(ovhShift*2) -ussShift, //ul
		rect.right-dlSize-ed2kSize -kadSize -messSize +kadShift +ed2kShift -ovhShift -ussShift, //dl
		rect.right-ed2kSize-kadSize -messSize +kadShift +ed2kShift -ussShift, //ed2k 
		rect.right-kadSize-messSize +kadShift -ussShift, //kad
		rect.right-messSize -ussShift, //messages
		-1 };
	// X-Ray :: Statusbar :: End
	statusbar->SetParts(_countof(aiWidths), aiWidths);
}

void CemuleDlg::OnSize(UINT nType, int cx, int cy)
{
	CTrayDialog::OnSize(nType, cx, cy);

	// X-Ray :: Toolbar :: Start
	if(cx != 0){
	CRect rect;
	GetClientRect(&rect);

		//we need at least 0+40+_countof(iBigButtons)*BIG_BUTTON_SIZE+_countof(iSmallButtons)*SMALL_BUTTON_SIZE size
		//plus if we want the last buttons to be on the right side some padding (m_co_ToolRight width)
		BITMAP bmpInfo;
		m_co_ToolRight.GetBitmap(&bmpInfo);
		int iDiff = rect.Width() - (0+152+_countof(iBigButtons)*BIG_BUTTON_SIZE+_countof(iSmallButtons)*SMALL_BUTTON_SIZE+bmpInfo.bmWidth);
		if(iDiff > 0)
		{
			for(int i = 1; i < 6; ++i)//numero de botoes
				GetDlgItem(iSmallButtons[_countof(iSmallButtons)-i])->SetWindowPos(NULL, rect.right-bmpInfo.bmWidth-SMALL_BUTTON_SIZE*i, 14, SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE, SWP_NOZORDER);
		}
		else
		{
			for(int i = 1; i < 6; ++i)
				GetDlgItem(iSmallButtons[_countof(iSmallButtons)-i])->SetWindowPos(NULL, 23+rect.left+iHelpOffSet-SMALL_BUTTON_SIZE*i, 14, SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE, SWP_NOZORDER);
		}

		rect.bottom = TB_HEIGHT;
		InvalidateRect(&rect, FALSE);
	}
	// X-Ray :: Toolbar :: End

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
				//Xman [MoNKi: -Check already downloaded files-]
				if(theApp.knownfiles->CheckAlreadyDownloadedFileQuestion(pFileLink->GetHashKey(),pFileLink->GetName()))
				{
				theApp.downloadqueue->AddFileLinkToDownload(pFileLink,searchwnd->GetSelectedCat());
			}
				//Xman end
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
			FlashWindow(true);
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
			CCollectionViewDialog dialog;
			dialog.SetCollection(pCollection);
			dialog.DoModal();
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
		if (clcommand==_T("exit")) 
		{
//>>> WiZaRd::Minimize on Close
			DoClose(); 
//			OnClose(); 
//<<< WiZaRd::Minimize on Close
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
			thePrefs.CheckSlotSpeed();
			//Xman end

			return true;
		}

		if (clcommand==_T("help") || clcommand==_T("/?")) {
			// show usage
			return true;
		}

		if (clcommand==_T("status")) {
			CString strBuff;
			strBuff.Format(_T("%sstatus.log"),thePrefs.GetMuleDirectory(EMULE_CONFIGBASEDIR));
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
				uint32 eMuleIn;
				uint32 eMuleOut;
				uint32 notUsed;
				theApp.pBandWidthControl->GetDatarates(thePrefs.GetDatarateSamples(),
														eMuleIn, notUsed,
													   eMuleOut, notUsed,
													   notUsed, notUsed);


				strBuff.Format(GetResString(IDS_UPDOWNSMALL), (float)eMuleOut/1024, (float)eMuleIn/1024);
				// Maella end
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

	CKnownFile* pKnownFile = (CKnownFile*)lParam;
	ASSERT( pKnownFile->IsKindOf(RUNTIME_CLASS(CKnownFile)) );

	if (pKnownFile->IsKindOf(RUNTIME_CLASS(CPartFile)))
	{
		CPartFile* pPartFile = static_cast<CPartFile*>(pKnownFile);
		pPartFile->SetFileOpProgress(wParam);
		pPartFile->UpdateDisplayedInfo(true);
	}

	return 0;
}

//Xman
// BEGIN SLUGFILLER: SafeHash
LRESULT CemuleDlg::OnHashFailed(WPARAM /*wParam*/, LPARAM lParam)
{
	// BEGIN SiRoB: Fix crash at shutdown
	if (theApp.m_app_state == APP_STATE_SHUTTINGDOWN) {
		UnknownFile_Struct* hashed = (UnknownFile_Struct*)lParam;
		delete hashed;
		return FALSE;
	}
	// END SiRoB: Fix crash at shutdown
	theApp.sharedfiles->HashFailed((UnknownFile_Struct*)lParam);
	return 0;
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
		pOwner->PartHashFinished((UINT)wParam, false);
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
		pOwner->PartHashFinished((UINT)wParam, true);
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
		pOwner->PartHashFinishedAICHRecover((UINT)wParam, false);
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
		pOwner->PartHashFinishedAICHRecover((UINT)wParam, true);
	return 0;
}
// END SLUGFILLER: SafeHash

// BEGIN SiRoB: ReadBlockFromFileThread
LRESULT CemuleDlg::OnReadBlockFromFileDone(WPARAM wParam,LPARAM lParam)
{
	CUpDownClient* client = (CUpDownClient*) lParam;
	if (theApp.m_app_state == APP_STATE_RUNNING && theApp.uploadqueue->IsDownloading(client))	// could have been canceled
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
	ASSERT(!(theApp.m_app_state == APP_STATE_RUNNING && theApp.downloadqueue==NULL));

	CPartFile* partfile = (CPartFile*) lParam;
	if (theApp.m_app_state == APP_STATE_RUNNING && theApp.downloadqueue!=NULL && theApp.downloadqueue->IsPartFile(partfile))	// could have been canceled
		partfile->FlushDone();
	return 0;
}
// END SiRoB: Flush Thread


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
		((CPartFile*)wParam)->FlushBuffersExceptionHandler((CFileException*)lParam);
	return 0;
}

LRESULT CemuleDlg::OnFileCompleted(WPARAM wParam, LPARAM lParam)
{
	// X-Ray :: FiXeS :: Bugfix :: Start :: SiRoB
	if (theApp.m_app_state == APP_STATE_SHUTTINGDOWN)
		return FALSE;
	// X-Ray :: FiXeS :: Bugfix :: End :: SiRoB
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
		theApp.m_app_state= APP_STATE_SHUTTINGDOWN;
//>>> WiZaRd::Minimize on Close
		DoClose(); 
//		OnClose();
//<<< WiZaRd::Minimize on Close
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
//>>> WiZaRd::Minimize on Close
		DoClose(); 
//		OnClose();	// do not invoke if shutdown takes longer than 20 sec, read above
//<<< WiZaRd::Minimize on Close
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
	if(thePrefs.hotkey_enabled) UnregisterHotKey(m_hWnd, 0x1515); //TK4 Mod 1.3a onward - Unregister HotKey
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

		if (AfxMessageBox(GetResString(IDS_MAIN_EXIT), MB_YESNO | MB_DEFBUTTON2) == IDNO)
			return false;

	}
	return true;
}

//>>> WiZaRd::Minimize on Close
void CemuleDlg::OnClose()
{	
	if(thePrefs.GetMinimizeOnClose())
	{

		TrayShow();
		ShowWindow(SW_HIDE);
		//WiZaRd: note that there is an issue! If the balloontips are disabled in the registry this won't cause anything!
		if (IsRunningXPSP2OrHigher() && !fechadu)
		{
			fechadu = true;
			//ShowBalloonTip(_T("O DreaMule ainda está ativado,pois deixando aberto todos ganharão, incluindo você. Para fechar-lo clique com o botão direito e aperte 'Sair'. Você pode mudar isso nas configurações -> Exbição. E desmarcar Minimizar ao Fechar "), _T("Ainda estou aqui !"), 1000);
			ShowBalloonTip(GetResString(IDS_TRAY_CLOSE),GetResString(IDS_TRAY_TITLE),700);
//			TrayUpdate();
		}

	}
	else
		DoClose();
}

void CemuleDlg::DoClose()
//void CemuleDlg::OnClose()
//<<< WiZaRd::Minimize on Close
{
	if (!CanClose() )
		return;

	//Xman new slpash-screen arrangement
	if (thePrefs.UseSplashScreen())
		{
			//Xman don't show splash on old windows->crash
			switch (thePrefs.GetWindowsVersion())
			{
			case _WINVER_98_:
			case _WINVER_95_:
			case _WINVER_ME_:
				break;
			default:
				theApp.ShowSplash(false);
			}
		}
	theApp.UpdateSplash(GetResString(IDS_SPLASH_UNLOADING));
	//Xman end
	mediaplayerwnd->Uninit(); //>>> MediaPlayer

	Log(_T("Fechando Dreamule"));
	m_pDropTarget->Revoke();
	theApp.m_app_state = APP_STATE_SHUTTINGDOWN;

	//Xman queued disc-access for read/flushing-threads
	theApp.ForeAllDiscAccessThreadsToFinish();

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

	Kademlia::CKademlia::Stop(); 	// couple of data files are written

	theApp.UpdateSplash(GetResString(IDS_SPLASH_UNLOADING1)); //Xman new slpash-screen arrangement

	// try to wait untill the hashing thread notices that we are shutting down
	CSingleLock sLock1(&theApp.hashing_mut); // only one filehash at a time
	sLock1.Lock(2000);

	//Xman queued disc-access for read/flushing-threads
	//if we don't unlock we can cause a deadlock here:
	//resuming the disc-access-thread with th above ForeAllDiscAccessThreadsToFinish
	//doesn't mean the thread start at once... they start later and the main app has the mutex
	//so the threads wait for ever.. and also the main app for the threads.
	sLock1.Unlock();
	//Xman end

	theApp.UpdateSplash(GetResString(IDS_SPLASH_UNLOADING2)); //Xman new slpash-screen arrangement

	// saving data & stuff
	theApp.emuledlg->preferenceswnd->m_wndSecurity.DeleteDDB();

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
	//sharedfileswnd->historylistctrl.SaveSettings(CPreferences::tableHistory); //Xman [MoNKi: -Downloaded History-]

	//Xman new adapter selection
	if(theApp.pBandWidthControl->GetwasNAFCLastActive()==true)
		thePrefs.SetNAFCFullControl(true);
	//Xman end

	theApp.m_pPeerCache->Save();
	theApp.scheduler->RestoreOriginals();
	theApp.searchlist->SaveSpamFilter();

	//Xman official UPNP removed
	/*
	// close uPnP Ports
	theApp.m_pUPnPFinder->StopAsyncFind();
	if (thePrefs.CloseUPnPOnExit())
		theApp.m_pUPnPFinder->DeletePorts();
	*/
	//Xman don't overwrite bak files if last sessions crashed
	//remark: it would be better to set the flag after all deletions, but this isn't possible, because the prefs need access to the objects when saving
	thePrefs.m_this_session_aborted_in_an_unnormal_way=false;
	thePrefs.Save();
	//Xman end
	thePerfLog.Shutdown();

	theApp.UpdateSplash(GetResString(IDS_SPLASH_UNLOADING3)); //Xman new slpash-screen arrangement

	// explicitly delete all listview items which may hold ptrs to objects which will get deleted
	// by the dtors (some lines below) to avoid potential problems during application shutdown.
	transferwnd->downloadlistctrl.DeleteAllItems();
	chatwnd->chatselector.DeleteAllItems();
	chatwnd->m_FriendListCtrl.DeleteAllItems();
	theApp.clientlist->DeleteAll();
	searchwnd->DeleteAllSearchListCtrlItems();
	sharedfileswnd->sharedfilesctrl.DeleteAllItems();
	sharedfileswnd->audiolistctrl.DeleteAllItems(); //>>> WiZaRd::SharedFiles Redesign
    transferwnd->queuelistctrl.DeleteAllItems();
	transferwnd->clientlistctrl.DeleteAllItems();
	transferwnd->uploadlistctrl.DeleteAllItems();
	serverwnd->serverlistctrl.DeleteAllItems();
	transferwnd->downloadclientsctrl.DeleteAllItems();
	sharedfileswnd->historylistctrl.DeleteAllItems(); //Xman [MoNKi: -Downloaded History-]

	CPartFileConvert::CloseGUI();
	CPartFileConvert::RemoveAllJobs();

    theApp.uploadBandwidthThrottler->EndThread();
    //Xman
	//theApp.lastCommonRouteFinder->EndThread();

	theApp.sharedfiles->DeletePartFileInstances();

	searchwnd->SendMessage(WM_CLOSE);

	theApp.UpdateSplash(GetResString(IDS_SPLASH_UNLOADING4));  //Xman new slpash-screen arrangement

	//Xman
	theApp.m_threadlock.WriteLock();	// SLUGFILLER: SafeHash - Last chance, let all running threads close before we start deleting

    // NOTE: Do not move those dtors into 'CemuleApp::InitInstance' (althought they should be there). The
	// dtors are indirectly calling functions which access several windows which would not be available
	// after we have closed the main window -> crash!

	// Tux: Fix: Crash fix by WiZaRd [start]
	try
	{
	// Tux: Fix: Crash fix by WiZaRd [end]
		// Tux: LiteMule: Remove MobileMule
	delete theApp.listensocket;		theApp.listensocket = NULL;
	delete theApp.clientudp;		theApp.clientudp = NULL;
	delete theApp.sharedfiles;		theApp.sharedfiles = NULL;
	delete theApp.serverconnect;	theApp.serverconnect = NULL;
	delete theApp.serverlist;		theApp.serverlist = NULL;		// CServerList::SaveServermetToFile
	delete theApp.knownfiles;		theApp.knownfiles = NULL;
	delete theApp.searchlist;		theApp.searchlist = NULL;
	theApp.UpdateSplash(GetResString(IDS_SPLASH_UNLOADING5));  //Xman new slpash-screen arrangement
	delete theApp.clientcredits;	theApp.clientcredits = NULL;
	theApp.UpdateSplash(GetResString(IDS_SPLASH_UNLOADING6));  //Xman new slpash-screen arrangement
	delete theApp.downloadqueue;	theApp.downloadqueue = NULL;
	delete theApp.uploadqueue;		theApp.uploadqueue = NULL;
	delete theApp.clientlist;		theApp.clientlist = NULL;
	delete theApp.friendlist;		theApp.friendlist = NULL;		// CFriendList::SaveList
	delete theApp.scheduler;		theApp.scheduler = NULL;

	theApp.UpdateSplash(GetResString(IDS_SPLASH_UNLOADING7));  //Xman new slpash-screen arrangement
	delete theApp.ipfilter;			theApp.ipfilter = NULL;
	delete theApp.webserver;		theApp.webserver = NULL;
	theApp.UpdateSplash(GetResString(IDS_SPLASH_UNLOADING8));  //Xman new slpash-screen arrangement
	delete theApp.uploadBandwidthThrottler; theApp.uploadBandwidthThrottler = NULL;
	

	//Xman
	//delete theApp.lastCommonRouteFinder; theApp.lastCommonRouteFinder = NULL;

	//Xman
	// Maella [patch] -Bandwidth: overall bandwidth measure-
	delete theApp.pBandWidthControl;theApp.pBandWidthControl = NULL;
	// Maella end
	//Xman
	//upnp_start
	theApp.m_UPnPNat.clearNATPortMapping();
	//upnp_end
	theApp.UpdateSplash(GetResString(IDS_SPLASH_UNLOADING9));  //Xman new slpash-screen arrangement

	//EastShare Start - added by AndCycle, IP to Country
	delete theApp.ip2country;		theApp.ip2country = NULL;
	//EastShare End   - added by AndCycle, IP to Country
	delete theApp.theModIconMap;	theApp.theModIconMap = NULL;	//>>> WiZaRd::ModIconMappings

	delete theApp.dlp; theApp.dlp=NULL; //Xman DLP

	theApp.UpdateSplash(GetResString(IDS_SPLASH_UNLOADING10)); //Xman new slpash-screen arrangement
	// Tux: Fix: Crash fix by WiZaRd [start]
	}	
	catch(...)
	{
		//official sometimes crashes on shutdown... dunno why... (yet)
	}
	// Tux: Fix: Crash fix by WiZaRd [end]

	thePrefs.Uninit();
	theApp.m_app_state = APP_STATE_DONE;
	CTrayDialog::OnCancel();
	AddDebugLogLine(DLP_VERYLOW, _T("DreaMule fechado"));
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
	if (IsPreferencesDlgOpen()) {
		MessageBeep(MB_OK);
		preferenceswnd->SetForegroundWindow();
		preferenceswnd->BringWindowToTop();
		return;
	}

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
	if(!IsRunning())
		return;

	// Avoid reentrancy problems with main window, options dialog and mini mule window
	if (IsPreferencesDlgOpen()) {
		MessageBeep(MB_OK);
		preferenceswnd->SetForegroundWindow();
		preferenceswnd->BringWindowToTop();
		return;
	}

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
										(int)thePrefs.GetMaxGraphUploadRate(), (int)thePrefs.GetMaxGraphDownloadRate(),
										(int)thePrefs.GetMaxUpload(), (int)thePrefs.GetMaxDownload());
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
//>>> WiZaRd::Minimize on Close
				DoClose(); 
//				OnClose();
//<<< WiZaRd::Minimize on Close
				break;
			case IDC_PREFERENCES:
				ShellOpenFile(thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR));
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
		//Xman modified
		text.Format(_T("20%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphUploadRate()*0.2),GetResString(IDS_KBYTESPERSEC));	m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U20,  text);
		text.Format(_T("40%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphUploadRate()*0.4),GetResString(IDS_KBYTESPERSEC));	m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U40,  text);
		text.Format(_T("60%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphUploadRate()*0.6),GetResString(IDS_KBYTESPERSEC));	m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U60,  text);
		text.Format(_T("80%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphUploadRate()*0.8),GetResString(IDS_KBYTESPERSEC));	m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U80,  text);
		text.Format(_T("90%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphUploadRate()*0.9),GetResString(IDS_KBYTESPERSEC));	m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U100, text);
		//Xman end
		
		//Xman 6.0 this makes no sense at all. recommended are 90%
		//Xman removed
		/*
		m_menuUploadCtrl.AppendMenu(MF_SEPARATOR);

		if (GetRecMaxUpload()>0) {
			text.Format(GetResString(IDS_PW_MINREC) + GetResString(IDS_KBYTESPERSEC), (uint16)GetRecMaxUpload());
			m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_UP10, text );
		}
		*/
		//Xman end

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
		//Xman official UPNP removed
		/*
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
		*/
		AddLogLine(true, GetResString(IDS_CONNECTING));

		// ed2k
		if (thePrefs.GetNetworkED2K() && !theApp.serverconnect->IsConnecting() && !theApp.serverconnect->IsConnected()) {
				theApp.serverconnect->ConnectToAnyServer();
			}

		// kad
				if( /*thePrefs.GetNetworkKademlia() &&*/ !Kademlia::CKademlia::IsRunning())
		{
					Kademlia::CKademlia::Start();
		}


		ShowConnectionState();
	}
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
void CemuleDlg::Mostar()
{
		SetForegroundWindow();
		BringWindowToTop();
}
void CemuleDlg::RestoreWindow()
{
	if (IsPreferencesDlgOpen()) {
		MessageBeep(MB_OK);
		preferenceswnd->SetForegroundWindow();
		preferenceswnd->BringWindowToTop();
		return;
	}
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

void CemuleDlg::UpdateTrayIcon(int iPercent)
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
}

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
				// Tux: LiteMule: Remove Sendmail
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
				// Tux: LiteMule: Remove Sendmail
			}
			break;

		// Tux: LiteMule: Remove Version Check: removed code block
		case TBN_NULL:
            m_wndTaskbarNotifier->Show(pszText, iMsgType, pszLink);
			bShowIt = true;
			break;
	}

	if (bShowIt && !bForceSoundOFF && thePrefs.GetNotifierSoundType() != ntfstNoSound)
	{
		bool bNotifiedWithAudio = false;
		// Tux: LiteMule: Remove TTS

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

		// Tux: LiteMule: Remove Version Check: removed code block
	}
    return 0;
}

void CemuleDlg::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	theApp.UpdateDesktopColorDepth();
	CTrayDialog::OnSettingChange(uFlags, lpszSection);
}

void CemuleDlg::OnSysColorChange()
{
	theApp.UpdateDesktopColorDepth();
	CTrayDialog::OnSysColorChange();
	SetAllIcons();
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
	// X-Ray :: Statusbar :: Start
	/*
	connicons[0] = theApp.LoadIcon(_T("ConnectedNotNot"), 16, 16);
	connicons[1] = theApp.LoadIcon(_T("ConnectedNotLow"), 16, 16);
	connicons[2] = theApp.LoadIcon(_T("ConnectedNotHigh"), 16, 16);
	connicons[3] = theApp.LoadIcon(_T("ConnectedLowNot"), 16, 16);
	connicons[4] = theApp.LoadIcon(_T("ConnectedLowLow"), 16, 16);
	connicons[5] = theApp.LoadIcon(_T("ConnectedLowHigh"), 16, 16);
	connicons[6] = theApp.LoadIcon(_T("ConnectedHighNot"), 16, 16);
	connicons[7] = theApp.LoadIcon(_T("ConnectedHighLow"), 16, 16);
	connicons[8] = theApp.LoadIcon(_T("ConnectedHighHigh"), 16, 16);
	*/
	connicons[0] = theApp.LoadIcon(_T("ConnectedServerNot"), 16, 16);	//not connect
	connicons[1] = theApp.LoadIcon(_T("ConnectedServer"), 16, 16);		//connected high ID
	connicons[2] = theApp.LoadIcon(_T("ConnectedServerLow"), 16, 16);	//connected lowID
	connicons[3] = theApp.LoadIcon(_T("ConnectedKad"), 16, 16);			//kad connected
	connicons[4] = theApp.LoadIcon(_T("ConnectedKadLow"), 16, 16);		//kad firewalled
	connicons[5] = theApp.LoadIcon(_T("ConnectedKadNot"), 16, 16);		//kad not connect
	// X-Ray :: Statusbar :: End
	ShowConnectionStateIcon();

	// transfer state
	if (transicons[0]) VERIFY( ::DestroyIcon(transicons[0]) );
	if (transicons[1]) VERIFY( ::DestroyIcon(transicons[1]) );
	// X-Ray :: Statusbar :: Start
	/*
	if (transicons[2]) VERIFY( ::DestroyIcon(transicons[2]) );
	if (transicons[3]) VERIFY( ::DestroyIcon(transicons[3]) );
	transicons[0] = theApp.LoadIcon(_T("UP0DOWN0"), 16, 16);
	transicons[1] = theApp.LoadIcon(_T("UP0DOWN1"), 16, 16);
	transicons[2] = theApp.LoadIcon(_T("UP1DOWN0"), 16, 16);
	transicons[3] = theApp.LoadIcon(_T("UP1DOWN1"), 16, 16);
	*/
	transicons[0] = theApp.LoadIcon(_T("UPLOAD"), 16, 16);
	transicons[1] = theApp.LoadIcon(_T("DOWNLOAD"), 16, 16);
	// X-Ray :: Statusbar :: End

	ShowTransferStateIcon();

	// users state
	if (usericon) VERIFY( ::DestroyIcon(usericon) );
	// X-Ray :: Statusbar :: Start
	/*
	usericon = theApp.LoadIcon(_T("StatsClients"), 16, 16);
	*/
	usericon = theApp.LoadIcon(_T("SBARUSERS"), 16, 16);

	// files state
	if (fileicon) VERIFY( ::DestroyIcon(fileicon) );
	fileicon = theApp.LoadIcon(_T("SBARFILES"), 16, 16);

	// system info
//>>> WiZaRd::ShutDownIfFinished
//	if (sysinfoicon) VERIFY( ::DestroyIcon(sysinfoicon) );
//	sysinfoicon = theApp.LoadIcon(_T("SYSINFO"), 16, 16);
//<<< WiZaRd::ShutDownIfFinished
	// X-Ray :: Statusbar :: End

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
		// Tux: LiteMule: Remove Version Check

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
	// toolbar->Localize(); // X-Ray :: Toolbar :: Cleanup

//DreaMule toolbar translatable
	SetDlgItemText(IDC_TB_BTN_TRANSFER, GetResString(IDS_MENU_TRANS));
	SetDlgItemText(IDC_TB_BTN_SEARCH, GetResString(IDS_MENU_PROCURA));
	SetDlgItemText(IDC_TB_BTN_FILES, GetResString(IDS_MENU_ARQUIVOS));
	SetDlgItemText(IDC_TB_BTN_MEDIAPLAYER, GetResString(IDS_MEDIAPLAYER)); //>>> WiZaRd::MediaPlayer
	
	SetDlgItemText(IDC_TB_BTN_SERVER, GetResString(IDS_MENU_SERVER));
	SetDlgItemText(IDC_TB_BTN_IRC, GetResString(IDS_MEU_IRC));
	SetDlgItemText(IDC_TB_BTN_STATISTIC, GetResString(IDS_MENU_GRAFICOS));
	SetDlgItemText(IDC_TB_BTN_PREFERENCES, GetResString(IDS_MENU_CONFIGS));
//DreaMule toolbar translatable



	// X-Ray :: Toolbar :: End

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
	statusbar->SetIcon(SBarFiles, fileicon); // X-Ray :: Statusbar
}

void CemuleDlg::QuickSpeedOther(UINT nID)
{
	//Xman changed the values !
	// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
	switch (nID) {
		case MP_QS_PA: thePrefs.SetMaxUpload((float)(3));
			thePrefs.SetMaxDownload((float)(1));
			thePrefs.CheckSlotSpeed(); //XMan Xtreme Upload
			break ;
		case MP_QS_UA:
			thePrefs.SetMaxUpload((float)(thePrefs.GetMaxGraphUploadRate()-2));
			thePrefs.SetMaxDownload((float)(thePrefs.GetMaxGraphDownloadRate()));
			thePrefs.CheckSlotSpeed(); //Xman Xtreme Upload
			break ;
	}
	//Xman end
}


void CemuleDlg::QuickSpeedUpload(UINT nID)
{
	switch (nID) {
		//Xman
		// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
		case MP_QS_U10: thePrefs.SetMaxUpload((float)(thePrefs.GetMaxGraphUploadRate()*0.1)); break ;
		case MP_QS_U20: thePrefs.SetMaxUpload((float)(thePrefs.GetMaxGraphUploadRate()*0.2)); break ;
		case MP_QS_U30: thePrefs.SetMaxUpload((float)(thePrefs.GetMaxGraphUploadRate()*0.3)); break ;
		case MP_QS_U40: thePrefs.SetMaxUpload((float)(thePrefs.GetMaxGraphUploadRate()*0.4)); break ;
		case MP_QS_U50: thePrefs.SetMaxUpload((float)(thePrefs.GetMaxGraphUploadRate()*0.5)); break ;
		case MP_QS_U60: thePrefs.SetMaxUpload((float)(thePrefs.GetMaxGraphUploadRate()*0.6)); break ;
		case MP_QS_U70: thePrefs.SetMaxUpload((float)(thePrefs.GetMaxGraphUploadRate()*0.7)); break ;
		case MP_QS_U80: thePrefs.SetMaxUpload((float)(thePrefs.GetMaxGraphUploadRate()*0.8)); break ;
		case MP_QS_U90: thePrefs.SetMaxUpload((float)(thePrefs.GetMaxGraphUploadRate()*0.9)); break ;
		case MP_QS_U100: thePrefs.SetMaxUpload(thePrefs.GetMaxGraphUploadRate()); break ;
//		case MP_QS_UPC: thePrefs.SetMaxUpload(UNLIMITED); break ;
		case MP_QS_UP10: thePrefs.SetMaxUpload(GetRecMaxUpload()); break ;
		//Xman end
	}
	thePrefs.CheckSlotSpeed(); //Xman Xtreme Upload
}

void CemuleDlg::QuickSpeedDownload(UINT nID)
{
	switch (nID) {
		//Xman
		// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
		case MP_QS_D10: thePrefs.SetMaxDownload(0.1f * thePrefs.GetMaxGraphDownloadRate()); break ;
		case MP_QS_D20: thePrefs.SetMaxDownload(0.2f * thePrefs.GetMaxGraphDownloadRate()); break ;
		case MP_QS_D30: thePrefs.SetMaxDownload(0.3f * thePrefs.GetMaxGraphDownloadRate()); break ;
		case MP_QS_D40: thePrefs.SetMaxDownload(0.4f * thePrefs.GetMaxGraphDownloadRate()); break ;
		case MP_QS_D50: thePrefs.SetMaxDownload(0.5f * thePrefs.GetMaxGraphDownloadRate()); break ;
		case MP_QS_D60: thePrefs.SetMaxDownload(0.6f * thePrefs.GetMaxGraphDownloadRate()); break ;
		case MP_QS_D70: thePrefs.SetMaxDownload(0.7f * thePrefs.GetMaxGraphDownloadRate()); break ;
		case MP_QS_D80: thePrefs.SetMaxDownload(0.8f * thePrefs.GetMaxGraphDownloadRate()); break ;
		case MP_QS_D90: thePrefs.SetMaxDownload(0.9f * thePrefs.GetMaxGraphDownloadRate()); break ;
		case MP_QS_D100: thePrefs.SetMaxDownload(thePrefs.GetMaxGraphDownloadRate()); break ;
//		case MP_QS_DC: thePrefs.SetMaxDownload(UNLIMITED); break ;
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
		case IDC_TB_BTN_CONNECT: // X-Ray :: Toolbar
			
			SetActiveDialog(webbrowser);
			break;
		case MP_HM_KAD:
		case IDC_TB_BTN_KADEMLIA: // X-Ray :: Toolbar
			SetActiveDialog(kademliawnd);
			break;
		case IDC_TB_BTN_SERVER: // X-Ray :: Toolbar
		case MP_HM_SRVR:
			SetActiveDialog(serverwnd);
			break;
		case IDC_TB_BTN_TRANSFER: // X-Ray :: Toolbar
		case MP_HM_TRANSFER:
			SetActiveDialog(transferwnd);
			break;
		case IDC_TB_BTN_SEARCH: // X-Ray :: Toolbar
		case MP_HM_SEARCH:
			SetActiveDialog(searchwnd);
			break;
		case IDC_TB_BTN_FILES: // X-Ray :: Toolbar
		case MP_HM_FILES:
			SetActiveDialog(sharedfileswnd);
			break;
//>>> WiZaRd::MediaPlayer
		case IDC_TB_BTN_MEDIAPLAYER: // X-Ray :: Toolbar
		case MP_HM_MEDIAPLAYER:
			SetActiveDialog(mediaplayerwnd);
			break;			
//<<< WiZaRd::MediaPlayer
		case IDC_TB_BTN_MESSAGES: // X-Ray :: Toolbar
		case MP_HM_MSGS:
			SetActiveDialog(chatwnd);
			break;
		case IDC_TB_BTN_IRC: // X-Ray :: Toolbar
		case MP_HM_IRC:
			SetActiveDialog(ircwnd);
			break;
		case IDC_TB_BTN_STATISTIC: // X-Ray :: Toolbar
		case MP_HM_STATS:
			SetActiveDialog(statisticswnd);
			break;
		case IDC_TB_BTN_PREFERENCES: // X-Ray :: Toolbar
		case MP_HM_PREFS:
			// m_co_PreferencesBtn.EnableWindow(FALSE); // X-Ray :: Toolbar // X-Ray :: ModelessDialogs
			ShowPreferences();
			// m_co_PreferencesBtn.EnableWindow(TRUE); // X-Ray :: Toolbar // X-Ray :: ModelessDialogs
			SetSpeedMeterRange((int)thePrefs.GetMaxGraphUploadRate(), (int)thePrefs.GetMaxGraphDownloadRate()); // X-Ray :: Speedgraph
			break;
		case MP_HM_OPENINC:
			ShellExecute(NULL, _T("open"), thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR),NULL, NULL, SW_SHOW);
			break;
		// Tux: LiteMule: Remove Help
		case MP_HM_CON:
			OnBnClickedButton2();
			break;
		case MP_HM_EXIT:
//>>> WiZaRd::Minimize on Close
			DoClose(); 
//			OnClose();
//<<< WiZaRd::Minimize on Close
			break;
		case MP_HM_LINK1: // MOD: dont remove!
	//		ShellExecute(NULL, NULL, thePrefs.GetHomepageBaseURL(), NULL, thePrefs.GetAppDir(), SW_SHOWDEFAULT);
			break;
		case MP_HM_LINK2:
//			ShellExecute(NULL, NULL, thePrefs.GetHomepageBaseURL()+ CString(_T("/faq/")), NULL, thePrefs.GetAppDir(), SW_SHOWDEFAULT);
			break;
		// Tux: LiteMule: Remove Version Check

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
			CIPFilterDlg dlg;
			dlg.DoModal();
			break;
		}
		case MP_HM_DIRECT_DOWNLOAD:{
			CDirectDownloadDlg dlg;
			dlg.DoModal();
			break;
		}
	}
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
	// X-Ray :: Toolbar :: Cleanup :: Start
	/*
	UINT nCmdID;
	if (toolbar->MapAccelerator((TCHAR)nChar, &nCmdID)){
		OnCommand(nCmdID, 0);
		return MAKELONG(0,MNC_CLOSE);
	}
	*/
	// X-Ray :: Toolbar :: Cleanup :: End

	return CTrayDialog::OnMenuChar(nChar, nFlags, pMenu);
}

// Barry - To find out if app is running or shutting/shut down
bool CemuleDlg::IsRunning()
{
	return (theApp.m_app_state == APP_STATE_RUNNING);
}


void CemuleDlg::OnBnClickedHotmenu()
{
	ShowToolPopup(false);
}

void CemuleDlg::ShowToolPopup(bool toolsonly)
{
	POINT point;
	::GetCursorPos(&point);

	CTitleMenu menu;
	menu.CreatePopupMenu();
	if (!toolsonly)
		menu.AddMenuTitle(GetResString(IDS_HOTMENU), true);
	else
		menu.AddMenuTitle(GetResString(IDS_TOOLS), true);

	CTitleMenu Links;
	Links.CreateMenu();
	Links.AddMenuTitle(NULL, true);
	Links.AppendMenu(MF_STRING, MP_HM_LINK1, GetResString(IDS_HM_LINKHP), _T("WEB"));
	Links.AppendMenu(MF_STRING, MP_HM_LINK2, GetResString(IDS_HM_LINKFAQ), _T("WEB"));
	// Tux: LiteMule: Remove Version Check
	theWebServices.GetGeneralMenuEntries(&Links);
	Links.InsertMenu(3, MF_BYPOSITION | MF_SEPARATOR);
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
		menu.AppendMenu(MF_STRING,MP_HM_MEDIAPLAYER, GetResString(IDS_MEDIAPLAYER), L"Audio"); //>>> WiZaRd::Mediaplayer
		menu.AppendMenu(MF_STRING,MP_HM_MSGS, GetResString(IDS_EM_MESSAGES), _T("MESSAGES"));
		menu.AppendMenu(MF_STRING,MP_HM_IRC, GetResString(IDS_IRC), _T("IRC"));
		menu.AppendMenu(MF_STRING,MP_HM_STATS, GetResString(IDS_EM_STATISTIC), _T("STATISTICS"));
		menu.AppendMenu(MF_STRING,MP_HM_PREFS, GetResString(IDS_EM_PREFS), _T("PREFERENCES"));
		// Tux: LiteMule: Remove Help
		menu.AppendMenu(MF_SEPARATOR);
	}

	menu.AppendMenu(MF_STRING,MP_HM_OPENINC, GetResString(IDS_OPENINC) + _T("..."), _T("OPENFOLDER"));
	menu.AppendMenu(MF_STRING,MP_HM_CONVERTPF, GetResString(IDS_IMPORTSPLPF) + _T("..."), _T("CONVERT"));
	menu.AppendMenu(MF_STRING,MP_HM_1STSWIZARD, GetResString(IDS_WIZ1) + _T("..."), _T("WIZARD"));
	menu.AppendMenu(MF_STRING,MP_HM_IPFILTER, GetResString(IDS_IPFILTER) + _T("..."), _T("IPFILTER"));
	menu.AppendMenu(MF_STRING,MP_HM_DIRECT_DOWNLOAD, GetResString(IDS_SW_DIRECTDOWNLOAD) + _T("..."), _T("PASTELINK"));

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
		serverwnd->debuglog->SetFont(&theApp.m_fontLog);
		serverwnd->leecherlog->SetFont(&theApp.m_fontLog); //Xman Anti-Leecher-Log
	}
}

LRESULT CemuleDlg::OnFrameGrabFinished(WPARAM wParam,LPARAM lParam){
	CKnownFile* pOwner = (CKnownFile*)wParam;
	FrameGrabResult_Struct* result = (FrameGrabResult_Struct*)lParam;

	if (theApp.m_app_state != APP_STATE_SHUTTINGDOWN) { // X-Ray :: FiXeS :: Bugfix :: SiRoB
	if (theApp.knownfiles->IsKnownFile(pOwner) || theApp.downloadqueue->IsPartFile(pOwner) ){
		pOwner->GrabbingFinished(result->imgResults,result->nImagesGrabbed, result->pSender);
	}
	else{
		ASSERT ( false );
	}
	} // X-Ray :: FiXeS :: Bugfix :: SiRoB

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
	else if (thePrefs.GetStraightWindowStyles() > 0)
		/*StraightWindowStyles(pWnd)*/;	// no longer needed
	else
	{
		s_bIsXPStyle = g_xpStyle.IsAppThemed() && g_xpStyle.IsThemeActive();
		if (!s_bIsXPStyle)
		FlatWindowStyles(pWnd);
}
}


//Xman new slpash-screen arrangement
//moved to emule.cpp
/*
void CemuleDlg::ShowSplash()
{
	//Xman Splashscreen
	ASSERT( m_pSplashWnd == NULL );
	if (m_pSplashWnd == NULL)
	{
		m_pSplashWnd = new CSplashScreenEx();
		if (m_pSplashWnd != NULL)
		{
			ASSERT(m_hWnd);

			if (m_pSplashWnd->Create(this,MOD_MAJOR_VERSION ,0,CSS_FADE | CSS_CENTERSCREEN | CSS_SHADOW))
			{
				m_pSplashWnd->SetBitmap(IDB_SPLASH,0,255,0);
				m_pSplashWnd->SetTextFont(_T("Tahoma"),155,CSS_TEXT_BOLD);
				CRect x=CRect(10,230,210,265);
				m_pSplashWnd->SetTextRect(x);
				m_pSplashWnd->SetTextColor(RGB(0,0,0));
				m_pSplashWnd->SetTextFormat(DT_SINGLELINE | DT_CENTER | DT_VCENTER);
				m_pSplashWnd->Show();
				Sleep(1000);
				m_dwSplashTime = ::GetCurrentTime();
				m_pSplashWnd->SetText(MOD_VERSION);
			}
			else
			{
				delete m_pSplashWnd;
				m_pSplashWnd = NULL;
			}
		}
	}
	//Xman end
}

void CemuleDlg::DestroySplash()
{
	if (m_pSplashWnd != NULL)
	{
		m_pSplashWnd->DestroyWindow();
		//m_pSplashWnd->Hide();
		delete m_pSplashWnd;
		m_pSplashWnd = NULL;
	}
}
*/


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

	//Xman new slpash-screen arrangement
	if (theApp.IsSplash() && theApp.spashscreenfinished)
	{
		if (::GetCurrentTime() - theApp.m_dwSplashTime > 3500)
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

			//Xman Code Improvement
			//enough to clean up handle maps every minute
			static uint32 lastprocess;
			if(lIdleCount>0)
			{
				theApp.OnIdle(lIdleCount); //free maps
				lastprocess=::GetTickCount();
				return 0;
			}

			if(theApp.OnIdle(0 /*lIdleCount*/) && ::GetTickCount() - lastprocess > MIN2MS(1)) 
				lResult=1;
			else
				lResult=0;
			//Xman end
		}
	}

	return lResult;
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

//>>> Minimize on Close
	//The problem is that "CLOSE" will cause minimize now 
	//That's why I catch the keystroke here and perform the proper function
	if(pMsg->message == WM_SYSKEYDOWN && pMsg->wParam == VK_F4)
		DoClose();
//<<< Minimize on Close

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
//			CString strUrl = thePrefs.GetHomepageBaseURL() + _T("/home/perl/help.cgi");
	//		ShellExecute(NULL, NULL, strUrl, NULL, thePrefs.GetAppDir(), SW_SHOWDEFAULT);
		}
	}
}

LRESULT CemuleDlg::OnPeerCacheResponse(WPARAM wParam, LPARAM lParam)
{
	return theApp.m_pPeerCache->OnPeerCacheCheckResponse(wParam,lParam);
}

// X-Ray :: Toolbar :: Cleanup :: Start
/*
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
}

LPCTSTR CemuleDlg::GetIconFromCmdId(UINT uId)
{
	LPCTSTR pszIconId = NULL;
	if (m_mapTbarCmdToIcon.Lookup(uId, pszIconId))
		return pszIconId;
	return NULL;
}

#ifndef TBIF_BYINDEX
#define TBIF_BYINDEX            0x80000000
#endif

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
*/
// X-Ray :: Toolbar :: Cleanup :: End

bool CemuleDlg::IsPreferencesDlgOpen() const
{
	return (preferenceswnd->m_hWnd != NULL);
}

int CemuleDlg::ShowPreferences(UINT uStartPageID)
{
	if (IsPreferencesDlgOpen())
	{
		preferenceswnd->SetForegroundWindow();
		preferenceswnd->BringWindowToTop();
		return -1;
	}
	else
	{
		if (uStartPageID != (UINT)-1)
			preferenceswnd->SetStartPage(uStartPageID);
		return preferenceswnd->DoModal();
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
		theApp.AddEd2kLinksToDownload( (TCHAR*)wParam,(int)lParam);

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
				CString ip=dest.Left(pos);
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
	//searchwnd->SetToolTipsDelay(uMilliseconds);
	transferwnd->SetToolTipsDelay(uMilliseconds);
}

// X-Ray :: Toolbar :: Start
void CemuleDlg::InvalidateButtons()
{
	static CxSkinButton* xSkinButtons[] = 
	{
		&m_co_ConnectBtn,		
		&m_co_KademliaBtn,
		&m_co_MediaPlayerBtn, //>>> WiZaRd::MediaPlayer
		&m_co_ServerBtn,		
		&m_co_TransferBtn,
		&m_co_SearchBtn,
		&m_co_FilesBtn,
		&m_co_MessagesBtn,
		&m_co_IrcBtn,
		&m_co_StatisticBtn,
		&m_co_PreferencesBtn
	};

	for(int i = 0; i < _countof(xSkinButtons); ++i)
		xSkinButtons[i]->Invalidate();
}
// X-Ray :: Toolbar :: End

// X-Ray :: Speedgraph :: Start
void CemuleDlg::Update_TrafficGraph()
{
				// Xman
				// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
				uint32 eMuleIn;
				uint32 eMuleOut;
				uint32 notUsed;
				theApp.pBandWidthControl->GetDatarates(thePrefs.GetDatarateSamples(),
														eMuleIn, notUsed,
													   eMuleOut, notUsed,
													   notUsed, notUsed);


	m_co_UpTrafficGraph.Set_TrafficValue(eMuleOut);
	m_co_DownTrafficGraph.Set_TrafficValue(eMuleIn);
}
// X-Ray :: Speedgraph :: End
//Xman process timer code via messages (Xanatos)
// Note: the timers does not crash on a exception so I use the messags to call the functions so when an error aprears it will be detected
afx_msg LRESULT CemuleDlg::DoTimer(WPARAM wParam, LPARAM /*lParam*/)
{
	if (!theApp.emuledlg->IsRunning())
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
//Dreamule PowerOff(by slaram) =>
BOOL CemuleDlg::OnSystemPowerOff(bool bOff)
{	
//	theApp.m_app_state = APP_STATE_SHUTTINGDOWN;
//	DoClose();

	HANDLE hToken; 
	TOKEN_PRIVILEGES tkp;	// Get a token for this process.

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		return FALSE;
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
	tkp.PrivilegeCount = 1;  // one privilege to set    
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;	// Get the shutdown privilege for this process.
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
	if (GetLastError() != ERROR_SUCCESS) 
		return FALSE; 

	if(bOff)	// shutdown
		return ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0);
	else
		return ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0);
}
//Dreamule PowerOff <=

void CemuleDlg::OnActivate(UINT nState, CWnd* /*pWndOther*/, BOOL /*bMinimized*/) 
{
    switch( nState )
    {
    case WA_ACTIVE:
		CemuleDlg::mediaplayerwnd->Refresh();//Boizaum, when change window, conserve the Video
        break;
    }    
}