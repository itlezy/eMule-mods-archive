//this file is part of eMule
//Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "DownloadQueue.h"
#include "ClientUDPSocket.h"
#include "UploadQueue.h"
#include "ClientList.h"
#include "UploadBandwidthThrottler.h"
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
#include "SearchParamsWnd.h" // «‘ºˆ√ﬂ∞°
#include "CWebBrowser.h"          //«‘ºˆ√ﬂ∞°
#include "WebTool.h"                    // «‘ºˆ√ﬂ∞°
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

//#include "TextToSpeech.h"//boizaum remove
#include "Collection.h"
#include "CollectionViewDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// by tax99 ----------
//#define	IDBAR_SEARCH_PARAMS		(AFX_IDW_CONTROLBAR_FIRST + 32 + 1)	// do NOT change that ID, if not absolutely needed (it is stored by MFC in the bar profile!)
// --------------------
//boizaum
#include "AdunanzA.h"
#include "DAMessageBox.h"
//boizaum

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

	//by tax99
	ON_WM_CTLCOLOR()
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

	// Version Check DNS
	ON_MESSAGE(UM_VERSIONCHECK_RESPONSE, OnVersionCheckResponse)

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

	ON_BN_CLICKED(IDC_BUTTON_SERVERON, OnBnClickedServer)
	ON_BN_CLICKED(IDC_BUTTON_LOGO, OnBnClickedLogo)

	ON_BN_CLICKED(IDC_BUTTON_TRANSFER, OnBnClickedTransfer)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, OnBnClickedSearch)
	ON_BN_CLICKED(IDC_BUTTON_SHARE, OnBnClickedShare)

	ON_BN_CLICKED(IDC_BUTTON_CHAT, OnBnClickedChat)
	ON_BN_CLICKED(IDC_BUTTON_MESSAGE, OnBnClickedMessage)
	ON_BN_CLICKED(IDC_BUTTON_SETTING, OnBnClickedSetting)

	ON_BN_CLICKED(IDC_BUTTON_HELP, OnBnClickedHelp)
	ON_BN_CLICKED(IDC_BUTTON_STATUS, OnBnClickedStatus)

END_MESSAGE_MAP()

CemuleDlg::CemuleDlg(CWnd* pParent /*=NULL*/)
	: CTrayDialog(CemuleDlg::IDD, pParent)
{
	// by tax99 ------------------------
	//wndParams = new CSearchParamsWnd;
	// ------------------------------

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
	browser = CWebBrowser::GetHtmlView();
	webtool = new CWebTool(this);
	m_Image = new CBitmap();
	m_Image->LoadBitmap(IDB_BITMAP1);

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
	for (int i = 0; i < ARRSIZE(connicons); i++)
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
}

CemuleDlg::~CemuleDlg()
{
//	CloseTTS();//remove speak
	DestroyMiniMule();
	if (m_icoSysTrayCurrent) VERIFY( DestroyIcon(m_icoSysTrayCurrent) );
	if (m_hIcon) VERIFY( ::DestroyIcon(m_hIcon) );
	for (int i = 0; i < ARRSIZE(connicons); i++){
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
	delete m_Image;
	delete m_pDropTarget;
//	delete browser;
	delete webtool;
}

void CemuleDlg::DoDataExchange(CDataExchange* pDX){

	DDX_Control(pDX, IDC_BUTTON_SEARCH , m_ButtonSearch);
	DDX_Control(pDX, IDC_BUTTON_SHARE , m_ButtonShare);
	DDX_Control(pDX, IDC_BUTTON_TRANSFER , m_ButtonTransfer);

	DDX_Control(pDX, IDC_BUTTON_CHAT , m_ButtonChat);
	DDX_Control(pDX, IDC_BUTTON_MESSAGE , m_ButtonMessage);
	DDX_Control(pDX, IDC_BUTTON_SETTING , m_ButtonSetting);

	DDX_Control(pDX, IDC_BUTTON_SERVERON , m_ButtonServer);

	DDX_Control(pDX, IDC_BUTTON_HELP , m_ButtonHelp);

	DDX_Control(pDX, IDC_BUTTON_STATUS , m_ButtonStatus);

	DDX_Control(pDX , IDC_BUTTON_LOGO , m_Logo);


	CTrayDialog::DoDataExchange(pDX);
}

LRESULT CemuleDlg::OnAreYouEmule(WPARAM, LPARAM)
{
	return UWM_ARE_YOU_EMULE;
}
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
	//by tax99 button setting----------------------
	m_ButtonSearch.SetSkin(IDB_SEARCH1 , IDB_SEARCH3 , IDB_SEARCH2 , IDB_SEARCH3 , 0 , 0 ,0 , 0 , 0 );
	m_ButtonTransfer.SetSkin(IDB_TRANSFER1, IDB_TRANSFER3 , IDB_TRANSFER2 , IDB_TRANSFER3 , 0 , 0 ,0 , 0 , 0 );
	m_ButtonShare.SetSkin(IDB_SHARE1 , IDB_SHARE3 , IDB_SHARE2 , IDB_SHARE3 , 0 , 0 ,0 , 0 , 0 );

	m_ButtonChat.SetSkin(IDB_CHAT1 , IDB_CHAT3 , IDB_CHAT2 , IDB_CHAT3 , 0 , 0 ,0 , 0 , 0 );
	m_ButtonMessage.SetSkin(IDB_FRIEND1, IDB_FRIEND3 , IDB_FRIEND2 , IDB_FRIEND3 , 0 , 0 ,0 , 0 , 0 );
	m_ButtonSetting.SetSkin(IDB_SETTING1 , IDB_SETTING3 , IDB_SETTING2 , IDB_SETTING3 , 0 , 0 ,0 , 0 , 0 );

	m_ButtonHelp.SetSkin(IDB_HELP1 , IDB_HELP3 , IDB_HELP2 , IDB_HELP3 , 0 , 0 ,0 , 0 , 0 );
	m_ButtonStatus.SetSkin(IDB_STATUS1 , IDB_STATUS3 , IDB_STATUS2 , IDB_STATUS3 , 0 , 0 ,0 , 0 , 0 ); //≈Î∞Ë

	m_ButtonServer.SetSkin(IDB_SERVER1 ,IDB_SERVER3 , IDB_SERVER2, IDB_SERVER3 , 0 , 0 , 0 , 0 , 0 );

	m_Logo.SetSkin(IDB_LOGOOFF , IDB_LOGOOFF , IDB_LOGOOFF , IDB_LOGOON ,0,0,0,0,0);
	//m_ServerState.SetSkin(IDB_SERVERON1 ,IDB_SERVERON1 , IDB_SERVERON1, IDB_SERVERON2 , 0 , 0 , 0 , 0 , 0 );
	//m_ServerState.SetSkin(IDB_SERVER1 ,IDB_SERVER3 , IDB_SERVER2, IDB_SERVER3 , 0 , 0 , 0 , 0 , 0 );



	//
	m_brBrush.CreateSolidBrush( RGB(255,255,255) );
	m_fontMarlett.CreatePointFont(10 * 10, _T("Marlett"));



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
	//CreateMenuCmdIconMap();

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL){
		pSysMenu->AppendMenu(MF_SEPARATOR);

		ASSERT( (MP_ABOUTBOX & 0xFFF0) == MP_ABOUTBOX && MP_ABOUTBOX < 0xF000);
		pSysMenu->AppendMenu(MF_STRING, MP_ABOUTBOX, GetResString(IDS_ABOUTBOX));

		ASSERT( (MP_VERSIONCHECK & 0xFFF0) == MP_VERSIONCHECK && MP_VERSIONCHECK < 0xF000);
		pSysMenu->AppendMenu(MF_STRING, MP_VERSIONCHECK, GetResString(IDS_VERSIONCHECK));



		// remaining system menu entries are created later...
	}

         toolbar->Create(WS_CHILD , CRect(0,0,0,0), this, IDC_TOOLBAR);

	CWnd* pwndToolbarX = toolbar;
	//if (toolbar->Create(WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), this, IDC_TOOLBAR))
//	{
	//	toolbar->Init();
	//	if (thePrefs.GetUseReBarToolbar())
	//	{
	//	    if (m_ctlMainTopReBar.Create(WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
	//								     RBS_BANDBORDERS | RBS_AUTOSIZE | CCS_NODIVIDER,
	//								     CRect(0, 0, 0, 0), this, AFX_IDW_REBAR))
	//	    {
	//		    CSize sizeBar;
	//		    VERIFY( toolbar->GetMaxSize(&sizeBar) );
	//		    REBARBANDINFO rbbi = {0};
	//		    rbbi.cbSize = sizeof(rbbi);
	//			rbbi.fMask = RBBIM_STYLE | RBBIM_SIZE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_IDEALSIZE | RBBIM_ID;
	//		    rbbi.fStyle = RBBS_NOGRIPPER | RBBS_BREAK | RBBS_USECHEVRON;
	//		    rbbi.hwndChild = toolbar->m_hWnd;
	//		    rbbi.cxMinChild = sizeBar.cy;
	//		    rbbi.cyMinChild = sizeBar.cy;
	//		    rbbi.cxIdeal = sizeBar.cx;
	//		    rbbi.cx = rbbi.cxIdeal;
	//			rbbi.wID = 0;
	//		    VERIFY( m_ctlMainTopReBar.InsertBand((UINT)-1, &rbbi) );
	//			toolbar->SaveCurHeight();
	//	    	toolbar->UpdateBackground();
    //
	//		    pwndToolbarX = &m_ctlMainTopReBar;
	//	    }
	//	}
	//}

	// set title
	SetWindowText(MOD_VERSION );//+ " eMule v0.47c");// + theApp.m_strCurVersionLong ); // Maella -Support for tag ET_MOD_VERSION 0x55

	// Init taskbar notifier
	m_wndTaskbarNotifier->Create(this);
		LoadNotifier(thePrefs.GetNotifierConfiguration());

	// set statusbar
	statusbar->Create(WS_CHILD|WS_VISIBLE|CCS_BOTTOM,CRect(0,0,0,0), this, IDC_STATUSBAR);
	statusbar->EnableToolTips(true);
	SetStatusBarPartsSize();

	// create main window dialog pages
	serverwnd->Create(IDD_SERVER);
	sharedfileswnd->Create(IDD_FILES);
	webtool->Create(IDD_WEBTOOL);
	// by tax99 ---------------
//	wndParams->Create(this, IDD_SEARCH_PARAMS,
//						 WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_SIZE_FIXED | CBRS_SIZE_DYNAMIC | CBRS_GRIPPER,
//						 IDBAR_SEARCH_PARAMS);
//	wndParams->SetWindowText(GetResString(IDS_SEARCHPARAMS));
	// -------------------------
	searchwnd->Create(this);
	chatwnd->Create(IDD_CHAT);
	transferwnd->Create(IDD_TRANSFER);
	statisticswnd->Create(IDD_STATISTICS);
	kademliawnd->Create(IDD_KADEMLIAWND);
	ircwnd->Create(IDD_IRC);

	// with the top rebar control, some XP themes look better with some additional lite borders.. some not..
	//serverwnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//sharedfileswnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//searchwnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//chatwnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//transferwnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//statisticswnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//kademliawnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//ircwnd->ModifyStyleEx(0, WS_EX_STATICEDGE);

	CRect a(0,0,0,0);

	browser->Create(NULL , NULL , WS_VISIBLE | WS_CHILD , a, this , AFX_IDW_PANE_FIRST , NULL );
	browser->Navigate2(_T("http://pootzmod.sourceforge.net/212user.html") , NULL , NULL );



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

	// adjust all main window sizes for toolbar height and maximize the child windows
	CRect rcClient, rcToolbar, rcStatusbar;
	GetClientRect(&rcClient);
	pwndToolbarX->GetWindowRect(&rcToolbar);
	statusbar->GetWindowRect(&rcStatusbar);

	rcClient.top += rcToolbar.Height()+35;
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
	for (int i = 0; i < ARRSIZE(apWnds); i++)
         		apWnds[i]->SetWindowPos(NULL, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER);  //¡÷ºÆ√≥∏Æ 1∞«
	// anchors
	AddAnchor(*serverwnd,		TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*kademliawnd,		TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*transferwnd,		TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*sharedfileswnd,	TOP_LEFT, BOTTOM_RIGHT);
    AddAnchor(*searchwnd,		TOP_LEFT, BOTTOM_RIGHT);
    AddAnchor(*chatwnd,			TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*browser,			TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*ircwnd,			TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*statisticswnd,	TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*webtool,	TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*statusbar,		BOTTOM_LEFT, BOTTOM_RIGHT);
	//boizaum
	AddAnchor(*pwndToolbarX,	TOP_LEFT, TOP_RIGHT);
	//boizaum
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
//    CRect rect;
//	GetClientRect(&rect);
//	if(rect.Width() < 803 )
//	{
  //     MoveWindow(10 , 10 , 803 , 615 );
	//}


	if (thePrefs.GetWSIsEnabled())
		theApp.webserver->StartServer();
//remove mobile mule

	VERIFY( (m_hTimer = ::SetTimer(NULL, NULL, 300, StartupTimer)) != NULL );
	if (thePrefs.GetVerbose() && !m_hTimer)
		AddDebugLogLine(true,_T("Failed to create 'startup' timer - %s"),GetErrorMessage(GetLastError()));

	theStats.starttime = GetTickCount();

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

	//by tax99
//	((CComboBox *)(wndParams->GetDlgItem(IDC_COMBO3)))->SetCurSel(0);
	browser->SetFocus();
	SetActiveDialog(browser);
	//browser->ShowWindow(SW_SHOW);
	//}

	VERIFY( m_pDropTarget->Register(this) );

	// initalize PeerCache
	theApp.m_pPeerCache->Init(thePrefs.GetPeerCacheLastSearch(), thePrefs.WasPeerCacheFound(), thePrefs.IsPeerCacheDownloadEnabled(), thePrefs.GetPeerCachePort());

	//Xman
	// SiRoB: SafeHash fix (see StartupTimer)
	/*
	// start aichsyncthread
	AfxBeginThread(RUNTIME_CLASS(CAICHSyncThread), THREAD_PRIORITY_BELOW_NORMAL,0);
	*/

	return TRUE;
}

// modders: dont remove or change the original versioncheck! (additionals are ok)
void CemuleDlg::DoVersioncheck(bool manual) {

// ¡÷ºÆ√≥∏Æ S
//	if (!manual && thePrefs.GetLastVC()!=0) {
//		CTime last(thePrefs.GetLastVC());
//		time_t tLast=safe_mktime(last.GetLocalTm());
//		time_t tNow=safe_mktime(CTime::GetCurrentTime().GetLocalTm());

//		if ( (difftime(tNow,tLast) / 86400)<thePrefs.GetUpdateDays() )
//			return;
//	}
//	if (WSAAsyncGetHostByName(m_hWnd, UM_VERSIONCHECK_RESPONSE, "vcdns2.emule-project.org", m_acVCDNSBuffer, sizeof(m_acVCDNSBuffer)) == 0){
//		AddLogLine(true,GetResString(IDS_NEWVERSIONFAILED));
//	}
// ------------------E
}


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
				theApp.UpdateSplash(_T("inicializando arquivos para baixar ...")); //Xman new slpash-screen arrangement

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
					AddLogLine(false,_T("Winsock: Version %d.%d [%.40s] %.40s"), HIBYTE( theApp.m_wsaData.wVersion ),LOBYTE(theApp.m_wsaData.wVersion ),
						CString(theApp.m_wsaData.szDescription), CString(theApp.m_wsaData.szSystemStatus));
					if (theApp.m_wsaData.iMaxSockets!=0)
						AddLogLine(false,_T("Winsock: max. sockets %d"), theApp.m_wsaData.iMaxSockets);
					else
						AddLogLine(false,_T("Winsock: unlimited sockets"));
					AddLogLine(false,_T("***************Winsock***************"));
					//>>> eWombat [WINSOCK2]
					AddLogLine(true, GetResString(IDS_MAIN_READY),theApp.m_strCurVersionLong + _T(" ") + MOD_VERSION); // XMan // Maella -Support for tag ET_MOD_VERSION 0x55
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
				theApp.sharedfiles->SetOutputCtrl(&theApp.emuledlg->sharedfileswnd->sharedfilesctrl);
				theApp.emuledlg->sharedfileswnd->historylistctrl.Init(); //Xman [MoNKi: -Downloaded History-]

				// BEGIN SiRoB: SafeHash fix originaly in OnInitDialog (delay load shared files)
				// start aichsyncthread
				theApp.UpdateSplash(_T("Sincronizando AICH-Hashes...")); //Xman new slpash-screen arrangement
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
				theApp.UpdateSplash(_T("Pronto")); //Xman new slpash-screen arrangement
				//autoconnect only after emule loaded completely
				//if(thePrefs.DoAutoConnect())
				theApp.emuledlg->OnBnClickedButton2();

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

	if (thePrefs.UpdateNotify())
		DoVersioncheck(false);

//boizaum teste
				if (AduTipShow(ADUTIP_PRIMEIRARODADA)) {
					theApp.DestroySplash();
				CDAMessageBox mb(NULL,
					_T("Bem vindos ao DreaMule\n")
					_T("O DreaMule È uma vers„o do emule, com centenas de novos recuros e mudanÁas.\n")
					_T("Essa nova vers„o a 2.12 possui diversas correÁıes")
					_T("Espero que todos gostem e sejam bem vindos !"));
				if (mb.DoModal() == IDOK) {
					AduTipBlock(ADUTIP_PRIMEIRARODADA);
				}
			}
//boizaum teste
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
		Draw(&dc);  // ±‚¡∏√ﬂ∞° ø∑ 1
	}
	//±‚¡∏ ∫Ø∞Ê 1 √ﬂ∞° 2
	else{
		CPaintDC dc(this);
		CTrayDialog::OnPaint();
		Draw(&dc);
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
	int iLen = _sntprintf(temp, ARRSIZE(temp), _T("%s: %s\r\n"), CTime::GetCurrentTime().Format(thePrefs.GetDateTimeFormat4Log()), pszText);
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
	UINT uIconIdx = GetConnectionStateIconIndex();
	if (uIconIdx >= ARRSIZE(connicons)){
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
	theApp.downloadqueue->OnConnectionState(theApp.IsConnected());
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
		toolbar->SetButtonInfo(IDC_TOOLBARBUTTON+0, &tbi);
		m_Logo.EnableWindow(TRUE);
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
			m_Logo.EnableWindow(TRUE);
			ShowUserCount();
		}
		else
		{
			CString strPane(GetResString(IDS_MAIN_BTN_CONNECT));
			TBBUTTONINFO tbi;
			tbi.cbSize = sizeof(TBBUTTONINFO);
			tbi.dwMask = TBIF_IMAGE | TBIF_TEXT;
			tbi.iImage = 0;
			tbi.pszText = const_cast<LPTSTR>((LPCTSTR)strPane);
			toolbar->SetButtonInfo(IDC_TOOLBARBUTTON+0, &tbi);
			m_Logo.EnableWindow(TRUE);
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
	buffer.Format(_T("%s:%s(%s)|%s:%s(%s)"), GetResString(IDS_UUSERS), CastItoIShort(totaluser, false, 1), CastItoIShort(Kademlia::CKademlia::GetKademliaUsers(), false, 1), GetResString(IDS_FILES), CastItoIShort(totalfile, false, 1), CastItoIShort(Kademlia::CKademlia::GetKademliaFiles(), false, 1));
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
		_sntprintf(szBuff, ARRSIZE(szBuff), _T("%.1f (%.1f)"), (float)m_uUpDatarate/1024, (float)m_uploadOverheadRate/1024);
	else
	//xman end
		_sntprintf(szBuff, ARRSIZE(szBuff), _T("%.1f"), (float)m_uUpDatarate/1024);
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
		_sntprintf(szBuff, ARRSIZE(szBuff), _T("%.1f (%.1f)"), (float)m_uDownDatarate/1024, (float)m_downloadOverheadRate/1024);
	else
	//Xman end
		_sntprintf(szBuff, ARRSIZE(szBuff), _T("%.1f"), (float)m_uDownDatarate/1024);
	return szBuff;
}

CString CemuleDlg::GetTransferRateString()
{
	TCHAR szBuff[128];
	//Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	if (thePrefs.ShowOverhead())
		_sntprintf(szBuff, ARRSIZE(szBuff), GetResString(IDS_UPDOWN),
				  (float)m_uUpDatarate/1024, (float)m_uploadOverheadRate/1024,
				  (float)m_uDownDatarate/1024, (float)m_downloadOverheadRate/1024);
	//Xman end
	else
		_sntprintf(szBuff, ARRSIZE(szBuff), GetResString(IDS_UPDOWNSMALL), (float)m_uUpDatarate/1024, (float)m_uDownDatarate/1024);
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

	CString strTransferRate = GetTransferRateString();
	if (TrayIsVisible() || bForceAll)
	{
		TCHAR buffer2[100];
		// set trayicon-icon
		int iDownRateProcent = (int)ceil((m_uDownDatarate/10.24) / thePrefs.GetMaxGraphDownloadRate());
		if (iDownRateProcent > 100)
			iDownRateProcent = 100;
		UpdateTrayIcon(iDownRateProcent);

//>>> WiZaRd::ToolTip-FiX
		//first of all it's not interesting which client we use
		//and second this will create a buggy tool tip because only limited
		//length is allowed/supported by M$/OS version/IE version
		//It is necessary to change that part in case you are using a rather long mod string/version string (as I do...)
		if (theApp.IsConnected())
			_sntprintf(buffer2, ARRSIZE(buffer2), _T("(%s) - %s"), GetResString(IDS_CONNECTED), strTransferRate);
		else
			_sntprintf(buffer2, ARRSIZE(buffer2), _T("(%s) - %s"), GetResString(IDS_DISCONNECTED), strTransferRate);
//<<< WiZaRd::ToolTip-FiX

		buffer2[63] = _T('\0');
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
		else if(theApp.downloadqueue->GetLimitState()==2)
			strTransferRate.Append(_T(" R"));
		//Xman end
		statusbar->SetText(strTransferRate, SBarUpDown, 0);
		ShowTransferStateIcon();
	}
	if (IsWindowVisible() && thePrefs.ShowRatesOnTitle())
	{
//>>> WiZaRd::FiX
		//well, as read somewhere else the stupid printf commands shouldn't be used at all
		//so I use the opportunity to clean it up
		CString szBuff;
		szBuff.Format(_T("(U:%.1f D:%.1f) eMule v%s"), m_uUpDatarate/1024.0f, m_uDownDatarate/1024.0f, theApp.m_strCurVersionLong);
		/*TCHAR szBuff[128];
		_sntprintf(szBuff, ARRSIZE(szBuff), _T("(U:%.1f D:%.1f) eMule v%s"), (float)m_uUpDatarate/1024, (float)m_uDownDatarate/1024, theApp.m_strCurVersionLong);*/
//<<< WiZaRd::FiX
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
	// by tax99 √£±‚∏¶ ¥≠∑∂¿ª∞ÊøÏ NULL ¿ª ≥—∞‹πﬁæ∆ √£±‚ √¢¿∏∑Œ πŸ≤„¡ÿ¥Ÿ

	browser->ShowWindow(SW_HIDE);

	if(dlg == NULL )
	{
		m_ButtonServer.EnableWindow(TRUE);
		m_ButtonShare.EnableWindow(TRUE);
    m_ButtonTransfer.EnableWindow(TRUE);
	m_ButtonSearch.EnableWindow(FALSE);
	m_ButtonChat.EnableWindow(TRUE);
	m_ButtonMessage.EnableWindow(TRUE);
	m_ButtonSetting.EnableWindow(TRUE);
	m_ButtonHelp.EnableWindow(TRUE);
	m_ButtonStatus.EnableWindow(TRUE);
	m_Logo.EnableWindow(TRUE);
		dlg = searchwnd;
	}
	if(dlg != browser)
	{
	if (dlg == activewnd)
		return;

		//browser->Navigate2(_T("about:blank"));
		webtool->ShowWindow(SW_HIDE);
	}
	else
	{
		webtool->ShowWindow(SW_SHOW);
		m_ButtonServer.EnableWindow(TRUE);
		m_ButtonShare.EnableWindow(TRUE);
    m_ButtonTransfer.EnableWindow(TRUE);
	m_ButtonSearch.EnableWindow(TRUE);
	m_ButtonChat.EnableWindow(TRUE);
	m_ButtonMessage.EnableWindow(TRUE);
	m_ButtonSetting.EnableWindow(TRUE);
	m_ButtonHelp.EnableWindow(TRUE);
	m_ButtonStatus.EnableWindow(TRUE);
	m_Logo.EnableWindow(TRUE);
			}


	if (activewnd)
		activewnd->ShowWindow(SW_HIDE);
	dlg->ShowWindow(SW_SHOW);
	dlg->SetFocus();
	activewnd = dlg;
}

void CemuleDlg::SetStatusBarPartsSize()
{
	CRect rect;
	statusbar->GetClientRect(&rect);
	int ussShift = 0;

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
	int aiWidths[5] =
	{
		rect.right - 675 - ussShift,
			rect.right - 440 - ussShift,
			rect.right - 235 - ussShift,
		rect.right -  25 - ussShift,
		-1
	};
	statusbar->SetParts(ARRSIZE(aiWidths), aiWidths);
}

void CemuleDlg::OnSize(UINT nType, int cx, int cy)
{
	CTrayDialog::OnSize(nType, cx, cy);
	SetStatusBarPartsSize();
	transferwnd->VerifyCatTabSize();

	int x , y;

	CRect rect;
	GetClientRect(&rect);
	m_Logo.SetWindowPos(NULL , 0 , 0 , 182 , 60 , SWP_NOZORDER);
	x = rect.right - 50;
	y = 0;

	m_ButtonStatus.SetWindowPos(NULL , x , y , 0 , 0 , SWP_NOSIZE );

	x-= 38;
	m_ButtonHelp.SetWindowPos(NULL , x , y , 0 , 0 , SWP_NOSIZE );

	x-= 38;
	m_ButtonSetting.SetWindowPos(NULL , x , y , 0 , 0 , SWP_NOSIZE );

	x-= 38;
	m_ButtonMessage.SetWindowPos(NULL , x , y , 0 , 0 , SWP_NOSIZE );

	x-= 38;
	m_ButtonChat.SetWindowPos(NULL , x , y , 0 , 0 , SWP_NOSIZE );

	x-= 90;
	m_ButtonShare.SetWindowPos(NULL , 488 , 6 , 0 , 0 , SWP_NOSIZE );

	x-= 90;
	m_ButtonTransfer.SetWindowPos(NULL , 388 , 6 , 0 , 0 , SWP_NOSIZE );

	x-= 60;
	m_ButtonSearch.SetWindowPos(NULL , 288 , 6 , 0 , 0 , SWP_NOSIZE );

	m_ButtonServer.SetWindowPos(NULL , 188 , 6 , 0 , 0 , SWP_NOSIZE);

	if(webtool != NULL)
		webtool->SetWindowPos(NULL , 0 , 40 , cx - 57 , 40 , SWP_NOZORDER );
	// by tax99	position
	/*if(browser != NULL)
		browser->SetWindowPos(NULL, 50, 120 , cx - 56, cy - 120, SWP_NOZORDER);	*/
//-------------------------E
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
			strBuff.Format(_T("%sstatus.log"),thePrefs.GetAppDir());
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
	// BEGIN SiRoB: Fix crash at shutdown
	if (theApp.m_app_state == APP_STATE_SHUTTINGDOWN)
		return FALSE;
	// END SiRoB: Fix crash at shutdown
	CPartFile* pOwner = (CPartFile*)lParam;
	if (theApp.downloadqueue->IsPartFile(pOwner))	// could have been canceled
		pOwner->PartHashFinished((UINT)wParam, false);
	return 0;
}

LRESULT CemuleDlg::OnPartHashedCorrupt(WPARAM wParam,LPARAM lParam)
{
	// BEGIN SiRoB: Fix crash at shutdown
	if (theApp.m_app_state == APP_STATE_SHUTTINGDOWN)
		return FALSE;
	// END SiRoB: Fix crash at shutdown
	CPartFile* pOwner = (CPartFile*)lParam;
	if (theApp.downloadqueue->IsPartFile(pOwner))	// could have been canceled
		pOwner->PartHashFinished((UINT)wParam, true);
	return 0;
}

LRESULT CemuleDlg::OnPartHashedOKAICHRecover(WPARAM wParam,LPARAM lParam)
{
	// BEGIN SiRoB: Fix crash at shutdown
	if (theApp.m_app_state == APP_STATE_SHUTTINGDOWN)
		return FALSE;
	// END SiRoB: Fix crash at shutdown
	CPartFile* pOwner = (CPartFile*)lParam;
	if (theApp.downloadqueue->IsPartFile(pOwner))	// could have been canceled
		pOwner->PartHashFinishedAICHRecover((UINT)wParam, false);
	return 0;
}

LRESULT CemuleDlg::OnPartHashedCorruptAICHRecover(WPARAM wParam,LPARAM lParam)
{
	// BEGIN SiRoB: Fix crash at shutdown
	if (theApp.m_app_state == APP_STATE_SHUTTINGDOWN)
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

void CemuleDlg::OnClose()
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
	theApp.UpdateSplash(_T("Desligando ..."));
	//Xman end

	Log(_T("Fechando Dreamule"));
;
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

	theApp.UpdateSplash(_T("esperando acabar o hash")); //Xman new slpash-screen arrangement

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

	theApp.UpdateSplash(_T("salvando configuraÁıes ...")); //Xman new slpash-screen arrangement

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
	//Xman don't overwrite bak files if last sessions crashed
	//remark: it would be better to set the flag after all deletions, but this isn't possible, because the prefs need access to the objects when saving
	thePrefs.m_this_session_aborted_in_an_unnormal_way=false;
	thePrefs.Save();
	//Xman end
	thePerfLog.Shutdown();

	theApp.UpdateSplash(_T("limpando items mostrados ...")); //Xman new slpash-screen arrangement

	// explicitly delete all listview items which may hold ptrs to objects which will get deleted
	// by the dtors (some lines below) to avoid potential problems during application shutdown.
	transferwnd->downloadlistctrl.DeleteAllItems();
	chatwnd->chatselector.DeleteAllItems();
	theApp.clientlist->DeleteAll();
	searchwnd->DeleteAllSearchListCtrlItems();
	sharedfileswnd->sharedfilesctrl.DeleteAllItems();
    transferwnd->queuelistctrl.DeleteAllItems();
	transferwnd->clientlistctrl.DeleteAllItems();
	transferwnd->uploadlistctrl.DeleteAllItems();
	serverwnd->serverlistctrl.DeleteAllItems();
	sharedfileswnd->historylistctrl.DeleteAllItems(); //Xman [MoNKi: -Downloaded History-]

	CPartFileConvert::CloseGUI();
	CPartFileConvert::RemoveAllJobs();

    theApp.uploadBandwidthThrottler->EndThread();
    //Xman
	//theApp.lastCommonRouteFinder->EndThread();

	theApp.sharedfiles->DeletePartFileInstances();

	searchwnd->SendMessage(WM_CLOSE);

	theApp.UpdateSplash(_T("limpando listas ..."));  //Xman new slpash-screen arrangement

	//Xman
	theApp.m_threadlock.WriteLock();	// SLUGFILLER: SafeHash - Last chance, let all running threads close before we start deleting

    // NOTE: Do not move those dtors into 'CemuleApp::InitInstance' (althought they should be there). The
	// dtors are indirectly calling functions which access several windows which would not be available
	// after we have closed the main window -> crash!
//mobile mule delete
	delete theApp.listensocket;		theApp.listensocket = NULL;
	delete theApp.clientudp;		theApp.clientudp = NULL;
	delete theApp.sharedfiles;		theApp.sharedfiles = NULL;
	delete theApp.serverconnect;	theApp.serverconnect = NULL;
	delete theApp.serverlist;		theApp.serverlist = NULL;		// CServerList::SaveServermetToFile
	delete theApp.knownfiles;		theApp.knownfiles = NULL;
	delete theApp.searchlist;		theApp.searchlist = NULL;
	theApp.UpdateSplash(_T("salvando creditos ..."));  //Xman new slpash-screen arrangement
	delete theApp.clientcredits;	theApp.clientcredits = NULL;
	theApp.UpdateSplash(_T("limpando fila ..."));  //Xman new slpash-screen arrangement
	delete theApp.downloadqueue;	theApp.downloadqueue = NULL;
	delete theApp.uploadqueue;		theApp.uploadqueue = NULL;
	delete theApp.clientlist;		theApp.clientlist = NULL;
	delete theApp.friendlist;		theApp.friendlist = NULL;		// CFriendList::SaveList
	delete theApp.scheduler;		theApp.scheduler = NULL;
	theApp.UpdateSplash(_T("descarregando filtro de ip ..."));  //Xman new slpash-screen arrangement
	delete theApp.ipfilter;			theApp.ipfilter = NULL;
	delete theApp.webserver;		theApp.webserver = NULL;
	delete theApp.m_pPeerCache;		theApp.m_pPeerCache = NULL;
	delete theApp.m_pFirewallOpener;theApp.m_pFirewallOpener = NULL;
	theApp.UpdateSplash(_T("desligando controle de banda ..."));  //Xman new slpash-screen arrangement
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
	theApp.UpdateSplash(_T("descarregando ip-to-country ..."));  //Xman new slpash-screen arrangement

	//EastShare Start - added by AndCycle, IP to Country
	delete theApp.ip2country;		theApp.ip2country = NULL;
	//EastShare End   - added by AndCycle, IP to Country

	delete theApp.dlp; theApp.dlp=NULL; //Xman DLP

	theApp.UpdateSplash(_T("Programa Finalizado !")); //Xman new slpash-screen arrangement

	thePrefs.Uninit();
	theApp.m_app_state = APP_STATE_DONE;
	CTrayDialog::OnCancel();
	AddDebugLogLine(DLP_VERYLOW, _T("Emule fechado"));
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
		//Xman modified
		text.Format(_T("20%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphUploadRate()*0.2),GetResString(IDS_KBYTESPERSEC));	m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U20,  text);
		text.Format(_T("40%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphUploadRate()*0.4),GetResString(IDS_KBYTESPERSEC));	m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U40,  text);
		text.Format(_T("60%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphUploadRate()*0.6),GetResString(IDS_KBYTESPERSEC));	m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U60,  text);
		text.Format(_T("80%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphUploadRate()*0.8),GetResString(IDS_KBYTESPERSEC));	m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U80,  text);
		text.Format(_T("90%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphUploadRate()*0.9),GetResString(IDS_KBYTESPERSEC));	m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U100, text);
		//Xman end
		m_menuUploadCtrl.AppendMenu(MF_SEPARATOR);

		if (GetRecMaxUpload()>0) {
			text.Format(GetResString(IDS_PW_MINREC) + GetResString(IDS_KBYTESPERSEC), (uint16)GetRecMaxUpload());
			m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_UP10, text );
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
	m_TrayIcon.SetColorLevels(aiLimits, aColors, ARRSIZE(aiLimits));

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
//		if (thePrefs.GetNotifierSoundType() == ntfstSpeech)
//			bNotifiedWithAudio = Speak(pszText);
//remove speak
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
			ShellExecute(NULL, NULL, theUrl, NULL, thePrefs.GetAppDir(), SW_SHOWDEFAULT);
			break;
		}
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
	for (int i = 0; i < ARRSIZE(connicons); i++){
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
//		VERIFY( pSysMenu->ModifyMenu(MP_ABOUTBOX, MF_BYCOMMAND | MF_STRING, MP_ABOUTBOX, GetResString(IDS_ABOUTBOX)) );
//		VERIFY( pSysMenu->ModifyMenu(MP_VERSIONCHECK, MF_BYCOMMAND | MF_STRING, MP_VERSIONCHECK, GetResString(IDS_VERSIONCHECK)) );

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

	if (thePrefs.GetMaxGraphUploadRate()<7) return 0;
	if (thePrefs.GetMaxGraphUploadRate()<15) return thePrefs.GetMaxGraphUploadRate()-1.5f;
	return (thePrefs.GetMaxGraphUploadRate()-2.5f);

}
//Xman end

BOOL CemuleDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{
		case IDC_TOOLBARBUTTON + 0:

			OnBnClickedButton2();
			SetActiveDialog(browser);
			break;
		case MP_HM_KAD:
		case IDC_TOOLBARBUTTON +1:
			SetActiveDialog(kademliawnd);
			break;
		case IDC_TOOLBARBUTTON + 2:
		case MP_HM_SRVR:
			SetActiveDialog(serverwnd);
			break;
		case IDC_TOOLBARBUTTON + 3:
		case MP_HM_TRANSFER:
			SetActiveDialog(transferwnd);
			break;
		case IDC_TOOLBARBUTTON + 4:
		case MP_HM_SEARCH:
			SetActiveDialog(searchwnd);
			break;
		case IDC_TOOLBARBUTTON + 5:
		case MP_HM_FILES:
			SetActiveDialog(sharedfileswnd);
			break;
		case IDC_TOOLBARBUTTON + 6:
		case MP_HM_MSGS:
			SetActiveDialog(chatwnd);
			break;
		case IDC_TOOLBARBUTTON + 7:
		case MP_HM_IRC:
			SetActiveDialog(ircwnd);
			break;
		case IDC_TOOLBARBUTTON + 8:
		case MP_HM_STATS:
			SetActiveDialog(statisticswnd);
			break;
		case IDC_TOOLBARBUTTON + 9:
		case MP_HM_PREFS:
			toolbar->CheckButton(IDC_TOOLBARBUTTON+9,TRUE);
			ShowPreferences();
			toolbar->CheckButton(IDC_TOOLBARBUTTON+9,FALSE);
			break;
		case IDC_TOOLBARBUTTON + 10:
			ShowToolPopup(true);
			break;
		case MP_HM_OPENINC:
			ShellExecute(NULL, _T("open"), thePrefs.GetIncomingDir(),NULL, NULL, SW_SHOW);
			break;
		case MP_HM_HELP:
		case IDC_TOOLBARBUTTON + 11:
		case MP_HM_CON:
			OnBnClickedButton2();
			break;
		case MP_HM_EXIT:
			OnClose();
			break;
		case MP_HM_LINK1: // MOD: dont remove!
			ShellExecute(NULL, NULL, thePrefs.GetHomepageBaseURL(), NULL, thePrefs.GetAppDir(), SW_SHOWDEFAULT);
			break;
		case MP_HM_LINK2:
			ShellExecute(NULL, NULL, thePrefs.GetHomepageBaseURL()+ CString(_T("/faq/")), NULL, thePrefs.GetAppDir(), SW_SHOWDEFAULT);
			break;
		case MP_HM_LINK3: {
			CString theUrl;
			theUrl.Format( thePrefs.GetVersionCheckBaseURL() + CString(_T("/en/version_check.php?version=%i&language=%i")),theApp.m_uCurVersionCheck,thePrefs.GetLanguageID());
			ShellExecute(NULL, NULL, theUrl, NULL, thePrefs.GetAppDir(), SW_SHOWDEFAULT);
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
	Links.AppendMenu(MF_STRING, MP_HM_LINK3, GetResString(IDS_HM_LINKVC), _T("WEB"));
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
			scheduler.AppendMenu(MF_STRING,MP_SCHACTIONS+i, theApp.scheduler->GetSchedule(i)->title );
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
	if (::GetClassNameA(*pWnd, szClassName, ARRSIZE(szClassName)))
	{
		bool bButton = (__ascii_stricmp(szClassName, "Button") == 0);

		if (   (__ascii_stricmp(szClassName, "EDIT") == 0 && (pWnd->GetExStyle() & WS_EX_STATICEDGE))
			|| __ascii_stricmp(szClassName, "SysListView32") == 0
			|| __ascii_stricmp(szClassName, "msctls_trackbar32") == 0
			)
		{
			pWnd->ModifyStyleEx(WS_EX_STATICEDGE, WS_EX_CLIENTEDGE);
		}

		if (bButton)
			pWnd->ModifyStyle(BS_FLAT, 0);
	}
}

void FlatWindowStyles(CWnd* pWnd)
{
	CWnd* pWndChild = pWnd->GetWindow(GW_CHILD);
	while (pWndChild)
	{
		FlatWindowStyles(pWndChild);
		pWndChild = pWndChild->GetNextWindow();
	}

	CHAR szClassName[MAX_PATH];
	if (::GetClassNameA(*pWnd, szClassName, ARRSIZE(szClassName)))
	{
		bool bButton = (__ascii_stricmp(szClassName, "Button") == 0);

//		if (   !bButton
//			//|| (__ascii_stricmp(szClassName, "SysListView32") == 0 && (pWnd->GetStyle() & WS_BORDER) == 0)
//			|| __ascii_stricmp(szClassName, "msctls_trackbar32") == 0
//			)
//		{
//			pWnd->ModifyStyleEx(WS_EX_CLIENTEDGE, WS_EX_STATICEDGE);
//		}

		if (bButton)
			pWnd->ModifyStyle(0, BS_FLAT);
	}
}

void InitWindowStyles(CWnd* pWnd)
{
	if (thePrefs.GetStraightWindowStyles() < 0)
		return;
	else if (thePrefs.GetStraightWindowStyles() > 0)
		StraightWindowStyles(pWnd);
	else
		FlatWindowStyles(pWnd);
}

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
					if(theApp.IsSplash()) theApp.DestroySplash(); //Xman new slpash-screen arrangement
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
							ShellExecute(NULL, NULL, theUrl, NULL, thePrefs.GetAppDir(), SW_SHOWDEFAULT);
						}
					}
#else
					Log(LOG_SUCCESS|LOG_STATUSBAR,GetResString(IDS_NEWVERSIONAVLBETA));
					if (AfxMessageBox(GetResString(IDS_NEWVERSIONAVLBETA)+GetResString(IDS_VISITVERSIONCHECK),MB_OK)==IDOK) {
						CString theUrl;
						theUrl.Format(_T("/en/download.php?version=%i&language=%i"),theApp.m_uCurVersionCheck,thePrefs.GetLanguageID());
						theUrl = thePrefs.GetVersionCheckBaseURL()+theUrl;
						ShellExecute(NULL, NULL, theUrl, NULL, thePrefs.GetAppDir(), SW_SHOWDEFAULT);
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

//LRESULT CemuleDlg::OnKickIdle(UINT /*nWhy*/, long /*lIdleCount*/)
//{
//	LRESULT lResult = 0;
//
//	//Xman new slpash-screen arrangement
//	if (theApp.IsSplash() && theApp.spashscreenfinished)
//	{
//		if (::GetCurrentTime() - theApp.m_dwSplashTime > 3500)
//		{
//			// timeout expired, destroy the splash window
//			theApp.DestroySplash();
//			UpdateWindow();
//		}
//		else
//		{
//			// check again later...
//			lResult = 1;
//		}
//	}
//	//Xman end
//
//	if (m_bStartMinimized)
//		PostStartupMinimized();
//
//	if (searchwnd && searchwnd->m_hWnd)
//	{
//		if (theApp.m_app_state != APP_STATE_SHUTINGDOWN)
//		{
//			//extern void Mfc_IdleUpdateCmdUiTopLevelFrameList(CWnd* pMainFrame);
//			//Mfc_IdleUpdateCmdUiTopLevelFrameList(this);
//			theApp.OnIdle(0/*lIdleCount*/);	// NOTE: DO **NOT** CALL THIS WITH 'lIdleCount>0'
//
//#ifdef _DEBUG
//			// We really should call this to free up the temporary object maps from MFC.
//			// It may/will show bugs (wrong usage of temp. MFC data) on couple of (hidden) places,
//			// therefore it's right now too dangerous to put this in 'Release' builds..
//			// ---
//			// The Microsoft Foundation Class (MFC) Libraries create temporary objects that are
//			// used inside of message handler functions. In MFC applications, these temporary
//			// objects are automatically cleaned up in the CWinApp::OnIdle() function that is
//			// called in between processing messages.
//
//			// To slow to be called on each KickIdle. Need a timer
//			//extern void Mfc_IdleFreeTempMaps();
//			//if (lIdleCount >= 0)
//			//	Mfc_IdleFreeTempMaps();
//#endif
//		}
//	}
//
//	return lResult;
//}

//Xman clean up temporary handle maps
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
			static uint32 lastprocess;
			if(lIdleCount>0)
			{
				extern void Mfc_IdleFreeTempMaps();
				Mfc_IdleFreeTempMaps();
				lastprocess=::GetTickCount();
				return 0;
			}
			if(theApp.OnIdle(0 /*lIdleCount*/) && ::GetTickCount() - lastprocess > MIN2MS(3))
				lResult=1;
			else
				lResult=0;

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
			ShellExecute(NULL, NULL, strUrl, NULL, thePrefs.GetAppDir(), SW_SHOWDEFAULT);
		}
	}
}

LRESULT CemuleDlg::OnPeerCacheResponse(WPARAM wParam, LPARAM lParam)
{
	return theApp.m_pPeerCache->OnPeerCacheCheckResponse(wParam,lParam);
}

#if (_WIN32_IE < 0x0500)

#ifndef TBIF_BYINDEX
#define TBIF_BYINDEX            0x80000000
#endif

typedef struct tagNMREBARCHEVRON
{
    NMHDR hdr;
    UINT uBand;
    UINT wID;
    LPARAM lParam;
    RECT rc;
    LPARAM lParamNM;
} NMREBARCHEVRON, *LPNMREBARCHEVRON;
#endif //(_WIN32_IE < 0x0500)

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
		DecodeBase16(link, link.GetLength(), fileid, ARRSIZE(fileid));
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


//Modder Comment !
//This related to interface changes , everything above the bip
//tipt
//i say bip
//bip
void CemuleDlg::Draw(CDC* pDC)
{
	CRect rc;
	GetClientRect(&rc);
	//CPaintDC DC(this); // device context for painting
	CBitmap backbuffer;
	backbuffer.CreateCompatibleBitmap(pDC, rc.Width() , rc.Height() );

	CDC rDC;
	rDC.CreateCompatibleDC(pDC);
	rDC.SelectObject(m_Image);


	CDC mDC;
	mDC.CreateCompatibleDC(pDC);
	mDC.SelectObject(backbuffer);

	mDC.FillSolidRect(0 , 0 , rc.Width(), rc.Height() , RGB(8,36,107));
	mDC.StretchBlt(0 , 0 , rc.Width(), 60 , &rDC , 0 , 104 , 6 , 60 , SRCCOPY);
	mDC.BitBlt(0 , 63 , 9 , 55 , &rDC , 183 , 1 , SRCCOPY );
	mDC.StretchBlt(0 , 115 , 9 , rc.Height() - 82 , &rDC , 183 , 60 , 9 , 10 , SRCCOPY );

	mDC.BitBlt(rc.Width() - 9, 63, 9 , 55 , &rDC , 539 , 0 , SRCCOPY );
	mDC.StretchBlt(rc.Width() - 9 , 115 , 9 , rc.Height() - 82 , &rDC , 539 , 60 , 9 , 10 , SRCCOPY );

	pDC->BitBlt(0, 0, rc.Width(), rc.Height() , &mDC, 0, 0, SRCCOPY);


}

HBRUSH CemuleDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	return (HBRUSH)m_brBrush;
	}
void CemuleDlg:: OnBnClickedShare()
{
	m_ButtonServer.EnableWindow(TRUE);
	m_ButtonShare.EnableWindow(FALSE);
    m_ButtonTransfer.EnableWindow(TRUE);
	m_ButtonSearch.EnableWindow(TRUE);
	m_ButtonChat.EnableWindow(TRUE);
	m_ButtonMessage.EnableWindow(TRUE);
	m_ButtonSetting.EnableWindow(TRUE);
	m_ButtonHelp.EnableWindow(TRUE);
	m_ButtonStatus.EnableWindow(TRUE);
	m_Logo.EnableWindow(TRUE);
	SetActiveDialog(sharedfileswnd);
}
void CemuleDlg:: OnBnClickedTransfer()
{
	m_ButtonServer.EnableWindow(TRUE);
	m_ButtonShare.EnableWindow(TRUE);
    m_ButtonTransfer.EnableWindow(FALSE);
	m_ButtonSearch.EnableWindow(TRUE);
	m_ButtonChat.EnableWindow(TRUE);
	m_ButtonMessage.EnableWindow(TRUE);
	m_ButtonSetting.EnableWindow(TRUE);
	m_ButtonHelp.EnableWindow(TRUE);
	m_ButtonStatus.EnableWindow(TRUE);
	m_Logo.EnableWindow(TRUE);
	SetActiveDialog(transferwnd);
}
void CemuleDlg:: OnBnClickedSearch()
{
	m_ButtonServer.EnableWindow(TRUE);
	m_ButtonShare.EnableWindow(TRUE);
    m_ButtonTransfer.EnableWindow(TRUE);
	m_ButtonSearch.EnableWindow(FALSE);
	m_ButtonChat.EnableWindow(TRUE);
	m_ButtonMessage.EnableWindow(TRUE);
	m_ButtonSetting.EnableWindow(TRUE);
	m_ButtonHelp.EnableWindow(TRUE);
	m_ButtonStatus.EnableWindow(TRUE);
	m_Logo.EnableWindow(TRUE);
	SetActiveDialog(searchwnd);
}



void CemuleDlg:: OnBnClickedChat()
	{
	m_ButtonServer.EnableWindow(TRUE);
	m_ButtonShare.EnableWindow(TRUE);
    m_ButtonTransfer.EnableWindow(TRUE);
	m_ButtonSearch.EnableWindow(TRUE);
	m_ButtonChat.EnableWindow(FALSE);
	m_ButtonMessage.EnableWindow(TRUE);
	m_ButtonSetting.EnableWindow(TRUE);
	m_ButtonHelp.EnableWindow(TRUE);
	m_ButtonStatus.EnableWindow(TRUE);
	m_Logo.EnableWindow(TRUE);
	SetActiveDialog(ircwnd);
	}
void CemuleDlg:: OnBnClickedMessage()
	{
	m_ButtonServer.EnableWindow(TRUE);
	m_ButtonShare.EnableWindow(TRUE);
    m_ButtonTransfer.EnableWindow(TRUE);
	m_ButtonSearch.EnableWindow(TRUE);
	m_ButtonChat.EnableWindow(TRUE);
	m_ButtonMessage.EnableWindow(FALSE);
	m_ButtonSetting.EnableWindow(TRUE);
	m_ButtonHelp.EnableWindow(TRUE);
	m_ButtonStatus.EnableWindow(TRUE);
	m_Logo.EnableWindow(TRUE);
	SetActiveDialog(chatwnd);
	}
void CemuleDlg:: OnBnClickedSetting()
{
	m_ButtonServer.EnableWindow(TRUE);
	m_ButtonShare.EnableWindow(TRUE);
    m_ButtonTransfer.EnableWindow(TRUE);
	m_ButtonSearch.EnableWindow(TRUE);
	m_ButtonChat.EnableWindow(TRUE);
	m_ButtonMessage.EnableWindow(TRUE);
	//m_ButtonSetting.EnableWindow(FALSE);
	m_ButtonSetting.EnableWindow(TRUE);
	m_ButtonHelp.EnableWindow(TRUE);
	preferenceswnd->DoModal();
	m_ButtonSearch.EnableWindow(TRUE);
	m_ButtonStatus.EnableWindow(TRUE);
	m_Logo.EnableWindow(TRUE);

}


void CemuleDlg:: OnBnClickedHelp()
{
	m_ButtonServer.EnableWindow(TRUE);
	m_ButtonShare.EnableWindow(TRUE);
    m_ButtonTransfer.EnableWindow(TRUE);
	m_ButtonSearch.EnableWindow(TRUE);
	m_ButtonChat.EnableWindow(TRUE);
	m_ButtonMessage.EnableWindow(TRUE);
	m_ButtonSetting.EnableWindow(TRUE);
	m_ButtonHelp.EnableWindow(TRUE);
	m_ButtonStatus.EnableWindow(TRUE);
	m_Logo.EnableWindow(TRUE);
	//ShellExecute(NULL, NULL, _T("iexplore"), _T("http://pootzmod.sourceforge.net/1user.html"), NULL, SW_SHOW);
	SetActiveDialog(kademliawnd);
}


void CemuleDlg:: OnBnClickedStatus()
			{
	m_ButtonServer.EnableWindow(TRUE);
	m_ButtonShare.EnableWindow(TRUE);
    m_ButtonTransfer.EnableWindow(TRUE);
	m_ButtonSearch.EnableWindow(TRUE);
	m_ButtonChat.EnableWindow(TRUE);
	m_ButtonMessage.EnableWindow(TRUE);
	m_ButtonSetting.EnableWindow(TRUE);
	m_ButtonHelp.EnableWindow(TRUE);
	m_ButtonSearch.EnableWindow(TRUE);
	m_ButtonStatus.EnableWindow(FALSE);
	m_Logo.EnableWindow(TRUE);
	SetActiveDialog(statisticswnd);

					}

void CemuleDlg:: OnBnClickedServer()
					{
	m_ButtonServer.EnableWindow(FALSE);
	m_ButtonShare.EnableWindow(TRUE);
    m_ButtonTransfer.EnableWindow(TRUE);
	m_ButtonSearch.EnableWindow(TRUE);
	m_ButtonChat.EnableWindow(TRUE);
	m_ButtonMessage.EnableWindow(TRUE);
	m_ButtonSetting.EnableWindow(TRUE);
	m_ButtonHelp.EnableWindow(TRUE);
	m_ButtonSearch.EnableWindow(TRUE);
	m_ButtonStatus.EnableWindow(TRUE);
	m_Logo.EnableWindow(TRUE);
	SetActiveDialog(serverwnd);

			}


void CemuleDlg:: OnBnClickedLogo()
{
	m_ButtonServer.EnableWindow(TRUE);
	m_ButtonShare.EnableWindow(TRUE);
    m_ButtonTransfer.EnableWindow(TRUE);
	m_ButtonSearch.EnableWindow(TRUE);
	m_ButtonChat.EnableWindow(TRUE);
	m_ButtonMessage.EnableWindow(TRUE);
	m_ButtonSetting.EnableWindow(TRUE);
	m_ButtonHelp.EnableWindow(TRUE);
	m_ButtonSearch.EnableWindow(TRUE);
	m_ButtonStatus.EnableWindow(TRUE);
	m_Logo.EnableWindow(TRUE);
	//browser->Navigate2(_T("http://pootzmod.sourceforge.net/2user.html") , NULL , NULL );
	SetActiveDialog(browser);
}
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
