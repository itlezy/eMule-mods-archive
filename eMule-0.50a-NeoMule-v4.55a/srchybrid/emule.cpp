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
//#ifdef _DEBUG
//#define _CRTDBG_MAP_ALLOC
//#include <crtdbg.h>
//#endif
#include <locale.h>
#include <io.h>
#include <share.h>
#include <Mmsystem.h>
#include <atlimage.h>
#include "emule.h"
#include "opcodes.h"
#include "mdump.h"
#include "Scheduler.h"
#include "SearchList.h"
#include "kademlia/kademlia/Kademlia.h"
#include "kademlia/kademlia/Prefs.h"
#include "kademlia/kademlia/Error.h"
#include "kademlia/utils/UInt128.h"
#include "PerfLog.h"
#include <..\src\mfc\sockimpl.h>
#include <..\src\mfc\afximpl.h>
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
#include "Neo/BC/BandwidthControl.h"
#else
#include "LastCommonRouteFinder.h"
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
#include "Neo/BC/DownloadBandwidthThrottler.h"
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
#include "Neo/BC/UploadBandwidthThrottler.h"
#else
#include "UploadBandwidthThrottler.h"
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
#include "ClientList.h"
#include "FriendList.h"
#include "ClientUDPSocket.h"
#include "DownloadQueue.h"
#include "IPFilter.h"
#include "MMServer.h"
#include "Statistics.h"
#include "OtherFunctions.h"
#include "WebServer.h"
#include "UploadQueue.h"
#include "SharedFileList.h"
#include "ServerList.h"
#include "Sockets.h"
#include "ListenSocket.h"
#include "ClientCredits.h"
#include "KnownFileList.h"
#include "Server.h"
#include "UpDownClient.h"
#include "ED2KLink.h"
#include "Preferences.h"
#include "secrunasuser.h"
#include "SafeFile.h"
#include "PeerCacheFinder.h"
#include "emuleDlg.h"
#include "SearchDlg.h"
#include "enbitmap.h"
#include "FirewallOpener.h"
#include "StringConversion.h"
#include "Log.h"
#include "Collection.h"
#include "LangIDs.h"
#include "HelpIDs.h"
#include "UPnPImplWrapper.h"
#include "VisualStylesXP.h"
#include "Neo/NeoVersion.h" // NEO: NV - [NeoVersion] <-- Xanatos --
#include "Neo/GUI/SplashScreenEx.h" // NEO: SS - [SplashScreen] <-- Xanatos --
#include "Neo/NeoPreferences.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
#include "Neo/LanCast/Lancast.h"
#endif //LANCAST // NEO: NLC END <-- Xanatos --
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
#include "Neo/VooDoo/voodoo.h"
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
#ifdef NATTUNNELING // NEO: UTCP - [UserModeTCP] -- Xanatos -->
#include "Neo/NatT/NatManager.h"
#endif //NATTUNNELING // NEO: UTCP END <-- Xanatos --
#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase] -- Xanatos -->
#include "Neo/Sources/SourceList.h"
#endif // NEO_CD // NEO: NCD END <-- Xanatos --
#include "Neo/SysInfo/SystemInfo.h" // NEO: SI - [SysInfo] <-- Xanatos --
#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->
#include "Neo/Argos.h"
#endif // ARGOS // NEO: NA END <-- Xanatos --

CLogFile theLog;
CLogFile theModLog; // NEO: ML - [ModLog] <-- Xanatos --
CLogFile theVerboseLog;
bool g_bLowColorDesktop = false;
bool g_bGdiPlusInstalled = false;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#define USE_16COLOR_ICONS

#ifdef _DEBUG
int CrtDebugReportCB(int reportType, char* message, int* returnValue);
#endif

///////////////////////////////////////////////////////////////////////////////
// MSLU (Microsoft Layer for Unicode) support - UnicoWS
// 
bool g_bUnicoWS = false;

void ShowUnicowsError()
{
	// NOTE: Do *NOT* use any MFC nor W-functions here!
	// NOTE: Do *NOT* use eMule's localization functions here!
	MessageBoxA(NULL,
				"This eMule version requires the \"Microsoft(R) Layer for Unicode(TM) on Windows(R) 95/98/ME Systems\".\r\n"
				"\r\n"
				"Download the MSLU package from Microsoft(R) here:\r\n"
				"        http://www.microsoft.com/downloads/details.aspx?FamilyId=73BA7BD7-ED06-4F0D-80A4-2A7EEAEE17E2\r\n"
				"or\r\n"
				"        visit the eMule Project Download Page http://www.emule-project.net/home/perl/general.cgi?rm=download\r\n"
				"or\r\n"
				"        search the Microsoft(R) Download Center http://www.microsoft.com/downloads/ for \"MSLU\" or \"unicows\"."
				"\r\n"
				"\r\n"
				"\r\n"
				"After downloading the MSLU package, run the \"unicows.exe\" program and specify your eMule installation folder "
				"where to place the extracted files from the package.\r\n"
				"\r\n"
				"Ensure that the file \"unicows.dll\" was placed in your eMule installation folder and start eMule again.",
				"eMule",
				MB_ICONSTOP | MB_SYSTEMMODAL | MB_SETFOREGROUND);
}

extern "C" HMODULE __stdcall ExplicitPreLoadUnicows()
{
#ifdef _AFXDLL
	// UnicoWS support *requires* statically linked MFC and C-RTL.

	// NOTE: Do *NOT* use any MFC nor W-functions here!
	// NOTE: Do *NOT* use eMule's localization functions here!
	MessageBoxA(NULL, 
				"This eMule version (Unicode, MSLU, shared MFC) does not run with this version of Windows.", 
				"eMule", 
				MB_ICONSTOP | MB_SYSTEMMODAL | MB_SETFOREGROUND);
	exit(1);
#endif

	// Pre-Load UnicoWS -- needed for proper initialization of MFC/C-RTL
	HMODULE hUnicoWS = LoadLibraryA("unicows.dll");
	if (hUnicoWS == NULL)
	{
		ShowUnicowsError();
		exit(1);
	}

	g_bUnicoWS = true;
	return hUnicoWS;
}

// NOTE: Do *NOT* change the name of this function. It *HAS* to be named "_PfnLoadUnicows" !
extern "C" HMODULE (__stdcall *_PfnLoadUnicows)(void) = &ExplicitPreLoadUnicows;


///////////////////////////////////////////////////////////////////////////////
// C-RTL Memory Debug Support
// 
#ifdef _DEBUG
static CMemoryState oldMemState, newMemState, diffMemState;

_CRT_ALLOC_HOOK g_pfnPrevCrtAllocHook = NULL;
CMap<const unsigned char*, const unsigned char*, UINT, UINT> g_allocations;
int eMuleAllocHook(int mode, void* pUserData, size_t nSize, int nBlockUse, long lRequest, const unsigned char* pszFileName, int nLine);

//CString _strCrtDebugReportFilePath(_T("eMule CRT Debug Log.log"));
// don't use a CString for that memory - it will not be available on application termination!
#define APP_CRT_DEBUG_LOG_FILE _T("eMule CRT Debug Log.log")
static TCHAR _szCrtDebugReportFilePath[MAX_PATH] = APP_CRT_DEBUG_LOG_FILE;
#endif //_DEBUG


struct SLogItem
{
	UINT uFlags;
    CString line;
};

void CALLBACK myErrHandler(Kademlia::CKademliaError *error)
{
	CString msg;
	msg.Format(_T("\r\nError 0x%08X : %hs\r\n"), error->m_iErrorCode, error->m_szErrorDescription);
	if(theApp.emuledlg && theApp.emuledlg->IsRunning())
		theApp.QueueDebugLogLine(false, _T("%s"), msg);
}

void CALLBACK myDebugAndLogHandler(LPCSTR lpMsg)
{
	if(theApp.emuledlg && theApp.emuledlg->IsRunning())
		theApp.QueueDebugLogLine(false, _T("%hs"), lpMsg);
}

void CALLBACK myLogHandler(LPCSTR lpMsg)
{
	if(theApp.emuledlg && theApp.emuledlg->IsRunning())
		theApp.QueueLogLine(false, _T("%hs"), lpMsg);
}

const static UINT UWM_ARE_YOU_EMULE = RegisterWindowMessage(EMULE_GUID);

BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType);

///////////////////////////////////////////////////////////////////////////////
// CemuleApp

BEGIN_MESSAGE_MAP(CemuleApp, CWinApp)
	ON_COMMAND(ID_HELP, OnHelp)
END_MESSAGE_MAP()

CemuleApp::CemuleApp(LPCTSTR lpszAppName)
	:CWinApp(lpszAppName)
{
	// This does not seem to work well with multithreading, although there is no reason why it should not.
	//_set_sbh_threshold(768);

	srand(time(NULL));
	m_dwPublicIP = 0;
#ifdef NATTUNNELING // NEO: RTP - [ReuseTCPPort] -- Xanatos -->
	m_uPublicPort = 0;
#endif //NATTUNNELING // NEO: RTP END <-- Xanatos --
	m_bAutoStart = false;

	// NEO: NCC - [NeoConnectionChecker] -- Xanatos -->
	m_uCountConnectionFailed = 0;
	m_uLastConnectionEstablished = 0;
	// NEO: NCC END <-- Xanatos --

	// NOTE: Do *NOT* forget to specify /DELAYLOAD:gdiplus.dll as link parameter.
	HMODULE hLib = LoadLibrary(_T("gdiplus.dll"));
	if (hLib != NULL) {
		g_bGdiPlusInstalled = GetProcAddress(hLib, "GdiplusStartup") != NULL;
		FreeLibrary(hLib);
	}
	m_ullComCtrlVer = MAKEDLLVERULL(4,0,0,0);
	m_hSystemImageList = NULL;
	m_sizSmallSystemIcon.cx = 16;
	m_sizSmallSystemIcon.cy = 16;
	m_hBigSystemImageList = NULL;
	m_sizBigSystemIcon.cx = 32;
	m_sizBigSystemIcon.cy = 32;
	m_iDfltImageListColorFlags = ILC_COLOR;
	m_pSplashWnd = NULL; // NEO: SS - [SplashScreen] <-- Xanatos --
	// NEO: RIC - [ReaskOnIDChange] -- Xanatos -->
	m_uLastIP = 0;
	m_bLastIDLow = false;
	m_uLastChangeID = 0;
	// NEO: RIC END <-- Xanatos --
	m_LocalBindAddress.Empty(); // NEO: MOD - [BindToAdapter] <-- Xanatos --

// MOD Note: Do not change this part - Merkur

	// this is the "base" version number <major>.<minor>.<update>.<build>
	m_dwProductVersionMS = MAKELONG(CemuleApp::m_nVersionMin, CemuleApp::m_nVersionMjr);
	m_dwProductVersionLS = MAKELONG(CemuleApp::m_nVersionBld, CemuleApp::m_nVersionUpd);

	// create a string version (e.g. "0.30a")
	ASSERT( CemuleApp::m_nVersionUpd + 'a' <= 'f' );
	m_strCurVersionLongDbg.Format(_T("%u.%u%c.%u"), CemuleApp::m_nVersionMjr, CemuleApp::m_nVersionMin, _T('a') + CemuleApp::m_nVersionUpd, CemuleApp::m_nVersionBld);
#ifdef _DEBUG
	m_strCurVersionLong = m_strCurVersionLongDbg;
#else
	m_strCurVersionLong.Format(_T("%u.%u%c"), CemuleApp::m_nVersionMjr, CemuleApp::m_nVersionMin, _T('a') + CemuleApp::m_nVersionUpd);
#endif

#ifdef _DEBUG
	m_strCurVersionLong += _T(" DEBUG");
#endif
#ifdef _BETA
	m_strCurVersionLong += _T(" BETA2");
#endif

// NEO: NV - [NeoVersion] -- Xanatos -->
	m_strNeoVersionLong.Format(_T("%u.%02u"), (uint32)MOD_VERSION_PRIMARY, (uint32)MOD_VERSION_SECUNDARY);

#if defined(_ALFA)
	m_strNeoVersionLong.AppendFormat(_T(".%03u"), (uint32)MOD_VERSION_TERTIARY);
#endif

#ifdef _DEBUG
	m_strNeoVersionLong.AppendFormat(_T(".%04u"), (uint32)MOD_VERSION_BUILD);
#endif

#if MOD_VERSION_UPDATE
	m_strNeoVersionLong.AppendFormat(_T("%c"), _T('a') + MOD_VERSION_UPDATE - 1);
#endif

#ifdef _ALFA
	m_strNeoVersionLong += _T(" ALPHA");

	m_strNeoVersionLong += _T(" (") _T(__DATE__) _T(")");
#endif

#ifdef _BETA
	m_strNeoVersionLong+= _T(" BETA");

#if MOD_BETA_BUILD
	m_strNeoVersionLong.AppendFormat(_T("%u"), (uint32)MOD_BETA_BUILD);
#endif
#if MOD_BETA_UPDATE
	m_strNeoVersionLong.AppendFormat(_T("%c"), _T('a') + MOD_BETA_UPDATE - 1);
#endif
#endif
	// NEO: NV END <-- Xanatos --

	// create the protocol version number
	CString strTmp;
	strTmp.Format(_T("0x%u"), m_dwProductVersionMS);
	VERIFY( _stscanf(strTmp, _T("0x%x"), &m_uCurVersionShort) == 1 );
	ASSERT( m_uCurVersionShort < 0x99 );

	// create the version check number
	strTmp.Format(_T("0x%u%c"), m_dwProductVersionMS, _T('A') + CemuleApp::m_nVersionUpd);
	VERIFY( _stscanf(strTmp, _T("0x%x"), &m_uCurVersionCheck) == 1 );
	ASSERT( m_uCurVersionCheck < 0x999 );
// MOD Note: end

	m_bGuardClipboardPrompt = false;

	EnableHtmlHelp();
}


CemuleApp theApp(_T("eMule"));


// Workaround for bugged 'AfxSocketTerm' (needed at least for MFC 7.0, 7.1, 8.0, 9.0)
#if _MFC_VER==0x0700 || _MFC_VER==0x0710 || _MFC_VER==0x0800 || _MFC_VER==0x0900
void __cdecl __AfxSocketTerm()
{
#if defined(_AFXDLL) && (_MFC_VER==0x0700 || _MFC_VER==0x0710)
	VERIFY( WSACleanup() == 0 );
#else
	_AFX_SOCK_STATE* pState = _afxSockState.GetData();
	if (pState->m_pfnSockTerm != NULL){
		VERIFY( WSACleanup() == 0 );
		pState->m_pfnSockTerm = NULL;
	}
#endif
}
#else
#error "You are using an MFC version which may require a special version of the above function!"
#endif

#ifdef WS2 // NEO: WS2 - [WINSOCK2] -- Xanatos -->
BOOL InitWinsock2(WSADATA *lpwsaData) 
{  
	_AFX_SOCK_STATE* pState = _afxSockState.GetData();
	if (pState->m_pfnSockTerm == NULL)
	{
		// initialize Winsock library
		WSADATA wsaData;
		if (lpwsaData == NULL)
			lpwsaData = &wsaData;
		WORD wVersionRequested = MAKEWORD(2, 2);
		int nResult = WSAStartup(wVersionRequested, lpwsaData);
		if (nResult != 0)
			return FALSE;
		if (LOBYTE(lpwsaData->wVersion) != 2 || HIBYTE(lpwsaData->wVersion) != 2)
		{
			WSACleanup();
			return FALSE;
		}
		// setup for termination of sockets
		pState->m_pfnSockTerm = &AfxSocketTerm;
	}
#ifndef _AFXDLL
	//BLOCK: setup maps and lists specific to socket state
	{
		_AFX_SOCK_THREAD_STATE* pState = _afxSockThreadState;
		if (pState->m_pmapSocketHandle == NULL)
			pState->m_pmapSocketHandle = new CMapPtrToPtr;
		if (pState->m_pmapDeadSockets == NULL)
			pState->m_pmapDeadSockets = new CMapPtrToPtr;
		if (pState->m_plistSocketNotifications == NULL)
			pState->m_plistSocketNotifications = new CPtrList;
	}
#endif
	return TRUE;
}
#endif // WS2 // NEO: WS2 END <-- Xanatos --

// CemuleApp Initialisierung

BOOL CemuleApp::InitInstance()
{
#ifdef _DEBUG
	// set Floating Point Processor to throw several exceptions, in particular the 'Floating point devide by zero'
	UINT uEmCtrlWord = _control87(0, 0) & _MCW_EM;
	_control87(uEmCtrlWord & ~(/*_EM_INEXACT |*/ _EM_UNDERFLOW | _EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID), _MCW_EM);

	// output all ASSERT messages to debug device
	_CrtSetReportMode(_CRT_ASSERT, _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_REPORT_MODE) | _CRTDBG_MODE_DEBUG);
#endif
	free((void*)m_pszProfileName);
	m_pszProfileName = _tcsdup(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("preferences.ini"));


#ifdef _DEBUG
	oldMemState.Checkpoint();
	// Installing that memory debug code works fine in Debug builds when running within VS Debugger,
	// but some other test applications don't like that all....
	//g_pfnPrevCrtAllocHook = _CrtSetAllocHook(&eMuleAllocHook);
#endif
	//afxMemDF = allocMemDF | delayFreeMemDF;


	///////////////////////////////////////////////////////////////////////////
	// Install crash dump creation
	//
#if !defined(_BETA) && !defined(_ALFA) // NEO: ND - [NeoDebug] <-- Xanatos --
	if (GetProfileInt(_T("eMule"), _T("CreateCrashDump"), 0))
#endif
		theCrashDumper.Enable(_T("eMule ") + m_strCurVersionLongDbg, true);

	///////////////////////////////////////////////////////////////////////////
	// Locale initialization -- BE VERY CAREFUL HERE!!!
	//
	_tsetlocale(LC_ALL, _T(""));		// set all categories of locale to user-default ANSI code page obtained from the OS.
	_tsetlocale(LC_NUMERIC, _T("C"));	// set numeric category to 'C'
	//_tsetlocale(LC_CTYPE, _T("C"));		// set character types category to 'C' (VERY IMPORTANT, we need binary string compares!)

	AfxOleInit();

#ifdef _ASSERTFILE // NEO: ND - [NeoDebug] -- Xanatos -->
 #pragma message (" ***NOTE: All asserts will be loged to a CRT Debug Log file, and no window will be displayed")
	_CrtSetReportHook(CrtDebugReportCB);
#endif // NEO: ND END <-- Xanatos --

	pstrPendingLink = NULL;
	if (ProcessCommandline())
		return false;

	// Should not be needed any longer as Unicode eMule was already released since quite some
	// time. Right now, that warning (if it gets triggered) is indeed just annoying and I
	// guess nobody ever understood that warning text nor for what it was really used for.
	// So, just use the default code page.
	//extern bool CheckThreadLocale();
	//if (!CheckThreadLocale())
	//	return false;

	///////////////////////////////////////////////////////////////////////////
	// Common Controls initialization
	//
	//          Mjr Min
	// -------------------------
	// XP SP3	6   0
	// Vista    6   16
	InitCommonControls();
	DWORD dwComCtrlMjr = 4;
	DWORD dwComCtrlMin = 0;
	AtlGetCommCtrlVersion(&dwComCtrlMjr, &dwComCtrlMin);
	m_ullComCtrlVer = MAKEDLLVERULL(dwComCtrlMjr,dwComCtrlMin,0,0);
	if (m_ullComCtrlVer < MAKEDLLVERULL(5,8,0,0))
	{
		if (GetProfileInt(_T("eMule"), _T("CheckComctl32"), 1)) // just in case some user's can not install that package and have to survive without it..
		{
			if (AfxMessageBox(GetResString(IDS_COMCTRL32_DLL_TOOOLD), MB_ICONSTOP | MB_YESNO) == IDYES)
				ShellOpenFile(_T("http://www.microsoft.com/downloads/details.aspx?FamilyID=cb2cf3a2-8025-4e8f-8511-9b476a8d35d2"));

			// No need to exit eMule, it will most likely work as expected but it will have some GUI glitches here and there..
		}
	}

	DWORD dwShellMjr = 4;
	DWORD dwShellMin = 0;
	AtlGetShellVersion(&dwShellMjr, &dwShellMin);
	ULONGLONG ullShellVer = MAKEDLLVERULL(dwShellMjr,dwShellMin,0,0);
	if (ullShellVer < MAKEDLLVERULL(4,7,0,0))
	{
		if (GetProfileInt(_T("eMule"), _T("CheckShell32"), 1)) // just in case some user's can not install that package and have to survive without it..
		{
			AfxMessageBox(_T("Windows Shell library (SHELL32.DLL) is too old!\r\n\r\neMule detected a version of the \"Windows Shell library (SHELL32.DLL)\" which is too old to be properly used by eMule. To ensure full and flawless functionality of eMule we strongly recommend to update the \"Windows Shell library (SHELL32.DLL)\" to at least version 4.7.\r\n\r\nDownload and install an update of the \"Windows Shell library (SHELL32.DLL)\" at Microsoft (R) Download Center."), MB_ICONSTOP);

			// No need to exit eMule, it will most likely work as expected but it will have some GUI glitches here and there..
		}
	}

	m_sizSmallSystemIcon.cx = GetSystemMetrics(SM_CXSMICON);
	m_sizSmallSystemIcon.cy = GetSystemMetrics(SM_CYSMICON);
	UpdateLargeIconSize();
	UpdateDesktopColorDepth();

	CWinApp::InitInstance();

#ifdef WS2 // NEO: WS2 - [WINSOCK2] -- Xanatos -->
	m_bWinSock2=false;
	memset(&m_wsaData,0,sizeof(WSADATA));
	if (!GetProfileInt(_T("Neo"), _T("WinSock2"), 0) || !InitWinsock2(&m_wsaData)) // <<< eWombat first try it with winsock2
	{
		memset(&m_wsaData,0,sizeof(WSADATA));
		if (!AfxSocketInit(&m_wsaData))
		{
			AfxMessageBox(GetResString(IDS_SOCKETS_INIT_FAILED));
			return FALSE;
		}
	}else
		m_bWinSock2=true;
#else
	if (!AfxSocketInit())
	{
		AfxMessageBox(GetResString(IDS_SOCKETS_INIT_FAILED));
		return FALSE;
	}
#endif // WS2 // NEO: WS2 END <-- Xanatos --

#if _MFC_VER==0x0700 || _MFC_VER==0x0710 || _MFC_VER==0x0800 || _MFC_VER==0x0900
	atexit(__AfxSocketTerm);
#else
#error "You are using an MFC version which may require a special version of the above function!"
#endif
	AfxEnableControlContainer();
	if (!AfxInitRichEdit2()){
		if (!AfxInitRichEdit())
			AfxMessageBox(_T("Fatal Error: No Rich Edit control library found!")); // should never happen..
	}

	if (!Kademlia::CKademlia::InitUnicode(AfxGetInstanceHandle())){
		AfxMessageBox(_T("Fatal Error: Failed to load Unicode character tables for Kademlia!")); // should never happen..
		return FALSE; // DO *NOT* START !!!
	}

	extern bool SelfTest();
	if (!SelfTest())
		return FALSE; // DO *NOT* START !!!

	// NEO: SS - [SplashScreen] -- Xanatos -->
	if (GetProfileInt(_T("eMule"), _T("Splashscreen"), 1))
		ShowSplash();
	// NEO: SS END <-- Xanatos --

	// create & initalize all the important stuff 
	UpdateSplash(GetResString(IDS_X_SS_INIT_PREFS)); // NEO: SS - [SplashScreen] <-- Xanatos --
	thePrefs.Init();
	NeoPrefs.Init(); // NEO: NCFG - [NeoConfiguration] <-- Xanatos --
	theStats.Init();

	UpdateSplash(GetResString(IDS_X_SS_PREP)); // NEO: SS - [SplashScreen] <-- Xanatos --

	// check if we have to restart eMule as Secure user
	if (thePrefs.IsRunAsUserEnabled()){
		CSecRunAsUser rau;
		eResult res = rau.RestartSecure();
		if (res == RES_OK_NEED_RESTART)
			return FALSE; // emule restart as secure user, kill this instance
		else if (res == RES_FAILED){
			// something went wrong
			theApp.QueueLogLine(false, GetResString(IDS_RAU_FAILED), rau.GetCurrentUserW()); 
		}
	}

	if (thePrefs.GetRTLWindowsLayout())
		EnableRTLWindowsLayout();

	SetPriorityClass(GetCurrentProcess(), NeoPrefs.GetAppPriority()); // NEO: MOD - [AppPriority] <-- Xanatos --

#ifdef _DEBUG
	_sntprintf(_szCrtDebugReportFilePath, _countof(_szCrtDebugReportFilePath) - 1, _T("%s%s"), thePrefs.GetMuleDirectory(EMULE_LOGDIR, false), APP_CRT_DEBUG_LOG_FILE);
#endif
	VERIFY( theLog.SetFilePath(thePrefs.GetMuleDirectory(EMULE_LOGDIR, thePrefs.GetLog2Disk()) + _T("eMule.log")) );
	VERIFY( theModLog.SetFilePath(thePrefs.GetMuleDirectory(EMULE_LOGDIR, thePrefs.GetLog2Disk()) + _T("NeoMule.log")) ); // NEO: ML - [ModLog] <-- Xanatos --
	VERIFY( theVerboseLog.SetFilePath(thePrefs.GetMuleDirectory(EMULE_LOGDIR, false) + _T("eMule_Verbose.log")) );
	theLog.SetMaxFileSize(thePrefs.GetMaxLogFileSize());
	theLog.SetFileFormat(thePrefs.GetLogFileFormat());
	// NEO: ML - [ModLog] -- Xanatos -->
	theModLog.SetMaxFileSize(thePrefs.GetMaxLogFileSize()); 
	theModLog.SetFileFormat(thePrefs.GetLogFileFormat()); 
	// NEO: ML END <-- Xanatos --
	theVerboseLog.SetMaxFileSize(thePrefs.GetMaxLogFileSize());
	theVerboseLog.SetFileFormat(thePrefs.GetLogFileFormat());
	if (thePrefs.GetLog2Disk()){
		theLog.Open();
		theLog.Log(_T("\r\n"));
		// NEO: ML - [ModLog] -- Xanatos -->
		theModLog.Open(); 
		theModLog.Log(_T("\r\n")); 
		// NEO: ML END <-- Xanatos --
	}
	if (thePrefs.GetDebug2Disk()){
		theVerboseLog.Open();
		theVerboseLog.Log(_T("\r\n"));
	}
	Log(_T("Starting eMule v%s"), m_strCurVersionLong);

	SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);

	CemuleDlg dlg;
	emuledlg = &dlg;
	m_pMainWnd = &dlg;

	// Barry - Auto-take ed2k links
	if (thePrefs.AutoTakeED2KLinks())
		Ask4RegFix(false, true, false);

	if (thePrefs.GetAutoStart())
		::AddAutoStart();
	else
		::RemAutoStart();

	m_pFirewallOpener = new CFirewallOpener();
	m_pFirewallOpener->Init(true); // we need to init it now (even if we may not use it yet) because of CoInitializeSecurity - which kinda ruins the sense of the class interface but ooohh well :P
	// Open WinXP firewallports if set in preferences and possible
	if (thePrefs.IsOpenPortsOnStartupEnabled()){
		if (m_pFirewallOpener->DoesFWConnectionExist()){
			// delete old rules added by eMule
			m_pFirewallOpener->RemoveRule(EMULE_DEFAULTRULENAME_UDP);
			m_pFirewallOpener->RemoveRule(EMULE_DEFAULTRULENAME_TCP);
			// open port for this session
			if (m_pFirewallOpener->OpenPort(thePrefs.GetPort(), NAT_PROTOCOL_TCP, EMULE_DEFAULTRULENAME_TCP, true))
				QueueLogLine(false, GetResString(IDS_FO_TEMPTCP_S), thePrefs.GetPort());
			else
				QueueLogLine(false, GetResString(IDS_FO_TEMPTCP_F), thePrefs.GetPort());

			if (thePrefs.GetUDPPort()){
				// open port for this session
				if (m_pFirewallOpener->OpenPort(thePrefs.GetUDPPort(), NAT_PROTOCOL_UDP, EMULE_DEFAULTRULENAME_UDP, true))
					QueueLogLine(false, GetResString(IDS_FO_TEMPUDP_S), thePrefs.GetUDPPort());
				else
					QueueLogLine(false, GetResString(IDS_FO_TEMPUDP_F), thePrefs.GetUDPPort());
			}
		}
	}

	// UPnP Port forwarding
	m_pUPnPFinder = new CUPnPImplWrapper();

    // Highres scheduling gives better resolution for Sleep(...) calls, and timeGetTime() calls
    m_wTimerRes = 0;
    if(thePrefs.GetHighresTimer())
		SetHighTimer(true); // NEO: MOD - [HighTimer] <-- Xanatos --

	// ZZ:UploadSpeedSense -->
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
	theApp.bandwidthControl = new CBandwidthControl();
#else
	lastCommonRouteFinder = new LastCommonRouteFinder();
#endif // NEO_BC // NEO: NBC END <-- Xanatos --

    uploadBandwidthThrottler = new UploadBandwidthThrottler();
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
	uploadBandwidthThrottler->StartThread();
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
	// ZZ:UploadSpeedSense <--

#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
	downloadBandwidthThrottler = new DownloadBandwidthThrottler();
	downloadBandwidthThrottler->StartThread();
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	voodoo = new CVoodoo();
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
	UpdateSplash(GetResString(IDS_X_SS_INIT_CL)); // NEO: SS - [SplashScreen] <-- Xanatos --
	clientlist = new CClientList();
	friendlist = new CFriendList();
	searchlist = new CSearchList();
	UpdateSplash(GetResString(IDS_X_SS_INIT_KFL)); // NEO: SS - [SplashScreen] <-- Xanatos --
	knownfiles = new CKnownFileList();
	serverlist = new CServerList();
	serverconnect = new CServerConnect();
	sharedfiles = new CSharedFileList(serverconnect);
	listensocket = new CListenSocket();
	clientudp	= new CClientUDPSocket();
#ifdef NATTUNNELING // NEO: UTCP - [UserModeTCP] -- Xanatos -->
	natmanager = new CNatManager();
#endif //NATTUNNELING // NEO: UTCP END <-- Xanatos --
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	lancast = new CLanCast();
#endif //LANCAST // NEO: NLC END <-- Xanatos --
	UpdateSplash(GetResString(IDS_X_SS_INIT_CCL)); // NEO: SS - [SplashScreen] <-- Xanatos --
	clientcredits = new CClientCreditsList();
	downloadqueue = new CDownloadQueue();	// bugfix - do this before creating the uploadqueue
	uploadqueue = new CUploadQueue();
#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
	UpdateSplash(GetResString(IDS_X_SS_INIT_IP2C)); // NEO: SS - [SplashScreen]
	ip2country = new CIP2Country();
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --
	UpdateSplash(GetResString(IDS_X_SS_INIT_IPF)); // NEO: SS - [SplashScreen] <-- Xanatos --
	ipfilter 	= new CIPFilter();
	webserver = new CWebServer(); // Webserver [kuchin]
	mmserver = new CMMServer();
	scheduler = new CScheduler();
	m_pPeerCache = new CPeerCacheFinder();
#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase] -- Xanatos -->
	UpdateSplash(GetResString(IDS_X_SS_INIT_SDB)); // NEO: SS - [SplashScreen] 
	sourcelist = new CSourceList(); 
#endif // NEO_CD // NEO: NCD END <-- Xanatos --
	sysinfo = new CSystemInfo(); // NEO: SI - [SysInfo] <-- Xanatos --
#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->
	argos = new CArgos();
#endif // ARGOS // NEO: NA END <-- Xanatos --

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
	theApp.bandwidthControl->StartBandwidthControl();
#endif // NEO_BC // NEO: NBC END <-- Xanatos --

	thePerfLog.Startup();
	UpdateSplash(GetResString(IDS_X_SS_SHOW_MAIN)); // NEO: SS - [SplashScreen] <-- Xanatos --
	dlg.DoModal();

	DisableRTLWindowsLayout();

	// Barry - Restore old registry if required
	if (thePrefs.AutoTakeED2KLinks())
		RevertReg();

	::CloseHandle(m_hMutexOneInstance);
#ifdef _DEBUG
	if (g_pfnPrevCrtAllocHook)
		_CrtSetAllocHook(g_pfnPrevCrtAllocHook);

	newMemState.Checkpoint();
	if (diffMemState.Difference(oldMemState, newMemState))
	{
		TRACE("Memory usage:\n");
		diffMemState.DumpStatistics();
	}
	//_CrtDumpMemoryLeaks();
#endif //_DEBUG

	emuledlg = NULL;

	ClearDebugLogQueue(true);
	ClearLogQueue(true);

	HideSplash(); // NEO: SS - [SplashScreen] <-- Xanatos --
	AddDebugLogLine(DLP_VERYLOW, _T("%hs: returning: FALSE"), __FUNCTION__);
	return FALSE;
}

int CemuleApp::ExitInstance()
{
	AddDebugLogLine(DLP_VERYLOW, _T("%hs"), __FUNCTION__);

	SetHighTimer(false); // NEO: MOD - [HighTimer] <-- Xanatos --

	return CWinApp::ExitInstance();
}

// NEO: MOD - [HighTimer] -- Xanatos -->
void CemuleApp::SetHighTimer(bool bHigh)
{
	if(bHigh == true && m_wTimerRes == 0)
	{
		TIMECAPS tc;
		if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) == TIMERR_NOERROR) 
		{
			m_wTimerRes = min(max(tc.wPeriodMin, 1), tc.wPeriodMax);
			if(m_wTimerRes > 0) {
				MMRESULT mmResult = timeBeginPeriod(m_wTimerRes); 
				if(thePrefs.GetVerbose()) {
					if(mmResult == TIMERR_NOERROR) {
						theApp.QueueDebugLogLine(false,_T("Succeeded to set timer/scheduler resolution to %i ms."), m_wTimerRes);
					} else {
						theApp.QueueDebugLogLine(false,_T("Failed to set timer/scheduler resolution to %i ms."), m_wTimerRes);
						m_wTimerRes = 0;
					}
				}
			} else {
				theApp.QueueDebugLogLine(false,_T("m_wTimerRes == 0. Not setting timer/scheduler resolution."));
			}
		}
	}
	else if(bHigh == false && m_wTimerRes != 0)
	{
        timeEndPeriod(m_wTimerRes);
		m_wTimerRes = 0;
		//theApp.QueueDebugLogLine(false,_T("Disabled setting timer/scheduler resolution.")); // this is loged on shutdown but to late to realy end up in the log
    }
}
// NEO: MOD END <-- Xanatos --

#ifdef _DEBUG
int CrtDebugReportCB(int reportType, char* message, int* returnValue)
{
	FILE* fp = _tfsopen(_szCrtDebugReportFilePath, _T("a"), _SH_DENYWR);
	if (fp){
		time_t tNow = time(NULL);
		TCHAR szTime[40];
		_tcsftime(szTime, _countof(szTime), _T("%H:%M:%S"), localtime(&tNow));
		_ftprintf(fp, _T("%s  %u  %hs"), szTime, reportType, message);
		fclose(fp);
	}
	*returnValue = 0; // avoid invokation of 'AfxDebugBreak' in ASSERT macros
	return TRUE; // avoid further processing of this debug report message by the CRT
}

// allocation hook - for memory statistics gatering
int eMuleAllocHook(int mode, void* pUserData, size_t nSize, int nBlockUse, long lRequest, const unsigned char* pszFileName, int nLine)
{
	UINT count = 0;
	g_allocations.Lookup(pszFileName, count);
	if (mode == _HOOK_ALLOC) {
		_CrtSetAllocHook(g_pfnPrevCrtAllocHook);
		g_allocations.SetAt(pszFileName, count + 1);
		_CrtSetAllocHook(&eMuleAllocHook);
	}
	else if (mode == _HOOK_FREE){
		_CrtSetAllocHook(g_pfnPrevCrtAllocHook);
		g_allocations.SetAt(pszFileName, count - 1);
		_CrtSetAllocHook(&eMuleAllocHook);
	}
	return g_pfnPrevCrtAllocHook(mode, pUserData, nSize, nBlockUse, lRequest, pszFileName, nLine);
}
#endif

bool CemuleApp::ProcessCommandline()
{
	bool bIgnoreRunningInstances = (GetProfileInt(_T("eMule"), _T("IgnoreInstances"), 0) != 0);

	for (int i = 1; i < __argc; i++){
		LPCTSTR pszParam = __targv[i];
		if (pszParam[0] == _T('-') || pszParam[0] == _T('/')){
			pszParam++;
#ifdef _DEBUG
			if (_tcsicmp(pszParam, _T("assertfile")) == 0)
				_CrtSetReportHook(CrtDebugReportCB);
#endif
			if (_tcsicmp(pszParam, _T("ignoreinstances")) == 0)
				bIgnoreRunningInstances = true;

			if (_tcsicmp(pszParam, _T("AutoStart")) == 0)
				m_bAutoStart = true;
		}
	}

	CCommandLineInfo cmdInfo;
    ParseCommandLine(cmdInfo);
    
	// If we create our TCP listen socket with SO_REUSEADDR, we have to ensure that there are
	// not 2 emules are running using the same port.
	// NOTE: This will not prevent from some other application using that port!
	UINT uTcpPort = GetProfileInt(_T("eMule"), _T("Port"), DEFAULT_TCP_PORT_OLD);
	CString strMutextName;
	strMutextName.Format(_T("%s:%u"), EMULE_GUID, uTcpPort);
	m_hMutexOneInstance = ::CreateMutex(NULL, FALSE, strMutextName);
	
	HWND maininst = NULL;
	bool bAlreadyRunning = false;
	if (!bIgnoreRunningInstances){
		bAlreadyRunning = (::GetLastError() == ERROR_ALREADY_EXISTS ||::GetLastError() == ERROR_ACCESS_DENIED);
    	if (bAlreadyRunning) EnumWindows(SearchEmuleWindow, (LPARAM)&maininst);
	}

    if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen) {
		CString* command = new CString(cmdInfo.m_strFileName);
		if (command->Find(_T("://"))>0) {
			sendstruct.cbData = (command->GetLength() + 1)*sizeof(TCHAR);
			sendstruct.dwData = OP_ED2KLINK;
			sendstruct.lpData = const_cast<LPTSTR>((LPCTSTR)*command);
    		if (maininst){
      			SendMessage(maininst, WM_COPYDATA, (WPARAM)0, (LPARAM)(PCOPYDATASTRUCT)&sendstruct);
				delete command;
      			return true;
			}
    		else
      			pstrPendingLink = command;
		}
		else if (CCollection::HasCollectionExtention(*command)) {
			sendstruct.cbData = (command->GetLength() + 1)*sizeof(TCHAR);
			sendstruct.dwData = OP_COLLECTION;
			sendstruct.lpData = const_cast<LPTSTR>((LPCTSTR)*command);
    		if (maininst){
      			SendMessage(maininst, WM_COPYDATA, (WPARAM)0, (LPARAM)(PCOPYDATASTRUCT)&sendstruct);
      			delete command;
				return true;
			}
    		else
      			pstrPendingLink = command;
		}
		else {
			sendstruct.cbData = (command->GetLength() + 1)*sizeof(TCHAR);
			sendstruct.dwData = OP_CLCOMMAND;
			sendstruct.lpData = const_cast<LPTSTR>((LPCTSTR)*command);
    		if (maininst){
      			SendMessage(maininst, WM_COPYDATA, (WPARAM)0, (LPARAM)(PCOPYDATASTRUCT)&sendstruct);
      			delete command;
				return true;
			}
			// Don't start if we were invoked with 'exit' command.
			if (command->CompareNoCase(_T("exit")) == 0) {
				delete command;
				return true;
			}
			delete command;
		}
    }
    return (maininst || bAlreadyRunning);
}

BOOL CALLBACK CemuleApp::SearchEmuleWindow(HWND hWnd, LPARAM lParam){
	DWORD dwMsgResult;
	LRESULT res = ::SendMessageTimeout(hWnd,UWM_ARE_YOU_EMULE,0, 0,SMTO_BLOCK |SMTO_ABORTIFHUNG,10000,&dwMsgResult);
	if(res == 0)
		return TRUE;
	if(dwMsgResult == UWM_ARE_YOU_EMULE){ 
		HWND * target = (HWND *)lParam;
		*target = hWnd;
		return FALSE; 
	} 
	return TRUE; 
} 


void CemuleApp::UpdateReceivedBytes(uint32 bytesToAdd) {
	SetTimeOnTransfer();
	theStats.sessionReceivedBytes+=bytesToAdd;
}
#ifdef BW_MOD// NEO: BM - [BandwidthModeration] -- Xanatos -->
void CemuleApp::UpdateSentBytes(uint32 bytesToAdd, bool sentToFriend, bool sentToRelease) {
#else
void CemuleApp::UpdateSentBytes(uint32 bytesToAdd, bool sentToFriend) {
#endif // BW_MOD // NEO: BM END <-- Xanatos --
	SetTimeOnTransfer();
	theStats.sessionSentBytes+=bytesToAdd;

    if(sentToFriend == true) {
	    theStats.sessionSentBytesToFriend += bytesToAdd;
    }
#ifdef BW_MOD// NEO: BM - [BandwidthModeration] -- Xanatos -->
    if(sentToRelease == true) {
	    theStats.sessionSentBytesToRelease += bytesToAdd;
    }
#endif // BW_MOD // NEO: BM END <-- Xanatos --
}

void CemuleApp::SetTimeOnTransfer() {
	if (theStats.transferStarttime>0) return;
	
	theStats.transferStarttime=GetTickCount();
}

CString CemuleApp::CreateED2kSourceLink(const CAbstractFile* f)
{
	if (!IsConnected() || IsFirewalled()){
		LogWarning(LOG_STATUSBAR, GetResString(IDS_SOURCELINKFAILED));
		return _T("");
	}
	uint32 dwID = GetID();

	CString strLink;
	strLink.Format(_T("ed2k://|file|%s|%I64u|%s|/|sources,%i.%i.%i.%i:%i|/"),
		EncodeUrlUtf8(StripInvalidFilenameChars(f->GetFileName(), false)),
		f->GetFileSize(),
		EncodeBase16(f->GetFileHash(),16),
		(uint8)dwID,(uint8)(dwID>>8),(uint8)(dwID>>16),(uint8)(dwID>>24), thePrefs.GetPort() );
	return strLink;
}

CString CemuleApp::CreateKadSourceLink(const CAbstractFile* f)
{
	CString strLink;
	if( Kademlia::CKademlia::IsConnected() && theApp.clientlist->GetBuddy() && theApp.IsFirewalled() )
	{
		CString KadID;
		Kademlia::CKademlia::GetPrefs()->GetKadID().Xor(Kademlia::CUInt128(true)).ToHexString(&KadID);
		strLink.Format(_T("ed2k://|file|%s|%I64u|%s|/|kadsources,%s:%s|/"),
			EncodeUrlUtf8(StripInvalidFilenameChars(f->GetFileName(), false)),
			f->GetFileSize(),
			EncodeBase16(f->GetFileHash(),16),
			md4str(thePrefs.GetUserHash()), KadID);
	}
	return strLink;
}

//TODO: Move to emule-window
bool CemuleApp::CopyTextToClipboard(CString strText)
{
	if (strText.IsEmpty())
		return false;

	HGLOBAL hGlobalT = GlobalAlloc(GHND | GMEM_SHARE, (strText.GetLength() + 1) * sizeof(TCHAR));
	if (hGlobalT != NULL)
	{
		LPTSTR pGlobalT = static_cast<LPTSTR>(GlobalLock(hGlobalT));
		if (pGlobalT != NULL)
		{
			_tcscpy(pGlobalT, strText);
			GlobalUnlock(hGlobalT);
		}
		else
		{
			GlobalFree(hGlobalT);
			hGlobalT = NULL;
		}
	}

	CStringA strTextA(strText);
	HGLOBAL hGlobalA = GlobalAlloc(GHND | GMEM_SHARE, (strTextA.GetLength() + 1) * sizeof(CHAR));
	if (hGlobalA != NULL)
	{
		LPSTR pGlobalA = static_cast<LPSTR>(GlobalLock(hGlobalA));
		if (pGlobalA != NULL)
		{
			strcpy(pGlobalA, strTextA);
			GlobalUnlock(hGlobalA);
		}
		else
		{
			GlobalFree(hGlobalA);
			hGlobalA = NULL;
		}
	}

	if (hGlobalT == NULL && hGlobalA == NULL)
		return false;

	int iCopied = 0;
	if (OpenClipboard(NULL))
	{
		if (EmptyClipboard())
		{
			if (hGlobalT){
				if (SetClipboardData(CF_UNICODETEXT, hGlobalT) != NULL){
					iCopied++;
				}
				else{
					GlobalFree(hGlobalT);
					hGlobalT = NULL;
				}
			}
			if (hGlobalA){
				if (SetClipboardData(CF_TEXT, hGlobalA) != NULL){
					iCopied++;
				}
				else{
					GlobalFree(hGlobalA);
					hGlobalA = NULL;
				}
			}
		}
		CloseClipboard();
	}

	if (iCopied == 0)
	{
		if (hGlobalT){
			GlobalFree(hGlobalT);
			hGlobalT = NULL;
		}
		if (hGlobalA){
			GlobalFree(hGlobalA);
			hGlobalA = NULL;
		}
	}
	else
		IgnoreClipboardLinks(strText); // this is so eMule won't think the clipboard has ed2k links for adding

	return (iCopied != 0);
}

//TODO: Move to emule-window
CString CemuleApp::CopyTextFromClipboard()
{
	if (IsClipboardFormatAvailable(CF_UNICODETEXT))
	{
		if (OpenClipboard(NULL))
		{
			bool bResult = false;
			CString strClipboard;
			HGLOBAL hMem = GetClipboardData(CF_UNICODETEXT);
			if (hMem)
			{
				LPCWSTR pwsz = (LPCWSTR)GlobalLock(hMem);
				if (pwsz)
				{
					strClipboard = pwsz;
					GlobalUnlock(hMem);
					bResult = true;
				}
			}
			CloseClipboard();
			if (bResult)
				return strClipboard;
		}
	}

	if (!IsClipboardFormatAvailable(CF_TEXT))
		return _T("");
	if (!OpenClipboard(NULL))
		return _T("");

	CString	retstring;
	HGLOBAL	hglb = GetClipboardData(CF_TEXT);
	if (hglb != NULL)
	{
		LPCSTR lptstr = (LPCSTR)GlobalLock(hglb);
		if (lptstr != NULL)
		{
			retstring = lptstr;
			GlobalUnlock(hglb);
		}
	}
	CloseClipboard();

	return retstring;
}

void CemuleApp::OnlineSig() // Added By Bouc7 
{ 
	if (!thePrefs.IsOnlineSignatureEnabled())
		return;

	static const TCHAR _szFileName[] = _T("onlinesig.dat");
	CString strFullPath =  thePrefs.GetMuleDirectory(EMULE_CONFIGBASEDIR);
	strFullPath += _szFileName;

	// The 'onlinesig.dat' is potentially read by other applications at more or less frequent intervals.
	//	 -	Set the file shareing mode to allow other processes to read the file while we are writing
	//		it (see also next point).
	//	 -	Try to write the hole file data at once, so other applications are always reading 
	//		a consistent amount of file data. C-RTL uses a 4 KB buffer, this is large enough to write
	//		those 2 lines into the onlinesig.dat file with one IO operation.
	//	 -	Although this file is a text file, we set the file mode to 'binary' because of backward 
	//		compatibility with older eMule versions.
    CSafeBufferedFile file;
	CFileException fexp;
	if (!file.Open(strFullPath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite | CFile::typeBinary, &fexp)){
		CString strError = GetResString(IDS_ERROR_SAVEFILE) + _T(" ") + CString(_szFileName);
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		fexp.GetErrorMessage(szError, _countof(szError));
		strError += _T(" - ");
		strError += szError;
		LogError(LOG_STATUSBAR, _T("%s"), strError);
		return;
    }

	try
	{
		char buffer[20];
		CStringA strBuff;
		if (IsConnected()){
			file.Write("1",1); 
			file.Write("|",1);
			if(serverconnect->IsConnected()){
				strBuff = serverconnect->GetCurrentServer()->GetListName();
				file.Write(strBuff, strBuff.GetLength()); 
			}
			else{
				strBuff = "Kademlia";
				file.Write(strBuff,strBuff.GetLength()); 
			}

			file.Write("|",1); 
			if(serverconnect->IsConnected()){
				strBuff = serverconnect->GetCurrentServer()->GetAddress();
				file.Write(strBuff,strBuff.GetLength()); 
			}
			else{
				strBuff = "0.0.0.0";
				file.Write(strBuff,strBuff.GetLength()); 
			}
			file.Write("|",1); 
			if(serverconnect->IsConnected()){
				_itoa(serverconnect->GetCurrentServer()->GetPort(),buffer,10); 
				file.Write(buffer,strlen(buffer));
			}
			else{
				strBuff = "0";
				file.Write(strBuff,strBuff.GetLength());
			}
		} 
		else 
			file.Write("0",1); 

		file.Write("\n",1); 
		sprintf(buffer,"%.1f",(float)downloadqueue->GetDatarate()/1024); 
		file.Write(buffer,strlen(buffer)); 
		file.Write("|",1); 
		sprintf(buffer,"%.1f",(float)uploadqueue->GetDatarate()/1024); 
		file.Write(buffer,strlen(buffer)); 
		file.Write("|",1); 
		_itoa(uploadqueue->GetWaitingUserCount(),buffer,10); 
		file.Write(buffer,strlen(buffer)); 

		file.Close(); 
	}
	catch (CFileException* ex)
	{
		CString strError = GetResString(IDS_ERROR_SAVEFILE) + _T(" ") + CString(_szFileName);
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		ex->GetErrorMessage(szError, _countof(szError));
		strError += _T(" - ");
		strError += szError;
		LogError(LOG_STATUSBAR, _T("%s"), strError);
		ex->Delete();
	}
} //End Added By Bouc7

bool CemuleApp::GetLangHelpFilePath(CString& strResult)
{
	// Change extension for help file
	CString strHelpFile = m_pszHelpFilePath;
	CFileFind ff;
	bool bFound;
	if (thePrefs.GetLanguageID() != MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT))
	{
		int pos = strHelpFile.ReverseFind(_T('\\'));   //CML
		CString temp;
		temp.Format(_T("%s\\eMule.%u.chm"), strHelpFile.Left(pos), thePrefs.GetLanguageID());
		if (pos>0)
			strHelpFile = temp;

		// if not exists, use original help (english)
		if (!ff.FindFile(strHelpFile, 0)){
			strHelpFile = m_pszHelpFilePath;
			bFound = false;
		}
		else
			bFound = true;
		strHelpFile.Replace(_T(".HLP"), _T(".chm"));
	}
	else{
		int pos = strHelpFile.ReverseFind(_T('\\'));
		CString temp;
		temp.Format(_T("%s\\eMule.chm"), strHelpFile.Left(pos));
		strHelpFile = temp;
		strHelpFile.Replace(_T(".HLP"), _T(".chm"));
		if (!ff.FindFile(strHelpFile, 0))
			bFound = false;
		else
			bFound = true;
	}
	ff.Close();
	strResult = strHelpFile;
	return bFound;
}

void CemuleApp::SetHelpFilePath(LPCTSTR pszHelpFilePath)
{
	free((void*)m_pszHelpFilePath);
	m_pszHelpFilePath = _tcsdup(pszHelpFilePath);
}

void CemuleApp::OnHelp()
{
	if (m_dwPromptContext != 0)
	{
		// do not call WinHelp when the error is failing to lauch help
		if (m_dwPromptContext != HID_BASE_PROMPT+AFX_IDP_FAILED_TO_LAUNCH_HELP)
			ShowHelp(m_dwPromptContext);
		return;
	}
	ShowHelp(0, HELP_CONTENTS);
}

void CemuleApp::ShowHelp(UINT uTopic, UINT uCmd)
{
	CString strHelpFilePath;
	bool bResult = GetLangHelpFilePath(strHelpFilePath);
	if (!bResult){
		if (ShowWebHelp(uTopic))
			return;
	}
	SetHelpFilePath(strHelpFilePath);
	WinHelpInternal(uTopic, uCmd);
}

bool CemuleApp::ShowWebHelp(UINT uTopic)
{
	CString strHelpURL;
	strHelpURL.Format(_T("http://onlinehelp.emule-project.net/help.php?language=%u&topic=%u"), thePrefs.GetLanguageID(), uTopic);
	ShellExecute(NULL, NULL, strHelpURL, NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
	return true;
}

int CemuleApp::GetFileTypeSystemImageIdx(LPCTSTR pszFilePath, int iLength /* = -1 */, bool bNormalsSize)
{
	//TODO: This has to be MBCS aware..
	DWORD dwFileAttributes;
	LPCTSTR pszCacheExt = NULL;
	if (iLength == -1)
		iLength = _tcslen(pszFilePath);
	if (iLength > 0 && (pszFilePath[iLength - 1] == _T('\\') || pszFilePath[iLength - 1] == _T('/'))){
		// it's a directory
		pszCacheExt = _T("\\");
		dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
	}
	else{
		dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
		// search last '.' character *after* the last '\\' character
		for (int i = iLength - 1; i >= 0; i--){
			if (pszFilePath[i] == _T('\\') || pszFilePath[i] == _T('/'))
				break;
			if (pszFilePath[i] == _T('.')) {
				// point to 1st character of extension (skip the '.')
				pszCacheExt = &pszFilePath[i+1];
				break;
			}
		}
		if (pszCacheExt == NULL)
			pszCacheExt = _T("");	// empty extension
	}

	// Search extension in "ext->idx" cache.
	LPVOID vData;
	if (bNormalsSize){
		if (!m_aBigExtToSysImgIdx.Lookup(pszCacheExt, vData)){
			// Get index for the system's small icon image list
			SHFILEINFO sfi;
			DWORD dwResult = SHGetFileInfo(pszFilePath, dwFileAttributes, &sfi, sizeof(sfi),
										SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX);
			if (dwResult == 0)
				return 0;
			ASSERT( m_hBigSystemImageList == NULL || m_hBigSystemImageList == (HIMAGELIST)dwResult );
			m_hBigSystemImageList = (HIMAGELIST)dwResult;

			// Store icon index in local cache
			m_aBigExtToSysImgIdx.SetAt(pszCacheExt, (LPVOID)sfi.iIcon);
			return sfi.iIcon;
		}
	}
	else{
		if (!m_aExtToSysImgIdx.Lookup(pszCacheExt, vData)){
			// Get index for the system's small icon image list
			SHFILEINFO sfi;
			DWORD dwResult = SHGetFileInfo(pszFilePath, dwFileAttributes, &sfi, sizeof(sfi),
										SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
			if (dwResult == 0)
				return 0;
			ASSERT( m_hSystemImageList == NULL || m_hSystemImageList == (HIMAGELIST)dwResult );
			m_hSystemImageList = (HIMAGELIST)dwResult;

			// Store icon index in local cache
			m_aExtToSysImgIdx.SetAt(pszCacheExt, (LPVOID)sfi.iIcon);
			return sfi.iIcon;
		}
	}


	// Return already cached value
	// Elandal: Assumes sizeof(void*) == sizeof(int)
	return (int)vData;
}

bool CemuleApp::IsConnected()
{
	return (theApp.serverconnect->IsConnected() || Kademlia::CKademlia::IsConnected());
}

// NEO: NCC - [NeoConnectionChecker] -- Xanatos -->
bool CemuleApp::GetConState(bool bInetOnly)
{
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
	if(theApp.bandwidthControl->MissingConnection() && !theApp.IsConnected())
		return false;
#endif // NEO_BC // NEO: NBC END

	if(NeoPrefs.UseAutoConnectionChecker() && theApp.IsConnectionFailed())
		return false;

	if(bInetOnly)
		return true;

	return IsConnected();
}

bool CemuleApp::IsConnectionFailed(){
	const int value = NeoPrefs.GetAutoConnectionCheckerValue();
	return m_uCountConnectionFailed > (uint16) value && ::GetTickCount() - m_uLastConnectionEstablished > SEC2MS((uint32)value);
}
// NEO: NCC END <-- Xanatos --

bool CemuleApp::IsPortchangeAllowed() {
	return ( theApp.clientlist->GetClientCount()==0 && !IsConnected() );
}

// NEO: MOD - [BindToAdapter] -- Xanatos -->
void CemuleApp::BindToAddress(LPCTSTR LocalBindAddress){
	if(!LocalBindAddress){
		if(m_LocalBindAddress.IsEmpty())
			return; // Is already bind to all
		m_LocalBindAddress.Empty();
	}else if(m_LocalBindAddress.Compare(LocalBindAddress) == 0)
		return; // The same IP return
	else
		m_LocalBindAddress = LocalBindAddress;

	ModLog(GetResString(IDS_X_BIND_SOCKETS), m_LocalBindAddress.IsEmpty() ? GetResString(IDS_ALL) : m_LocalBindAddress);

	if(listensocket->GetConnectedPort()){ // if the socket is already bind we have to rebind
		listensocket->Rebind();
		clientudp->Rebind();
		serverconnect->Rebind();
	}
}
// NEO: MOD END <-- Xanatos --

uint32 CemuleApp::GetID(){
	uint32 ID;
	if( Kademlia::CKademlia::IsConnected() && !Kademlia::CKademlia::IsFirewalled() )
		ID = ntohl(Kademlia::CKademlia::GetIPAddress());
	else if( theApp.serverconnect->IsConnected() )
		ID = theApp.serverconnect->GetClientID();
	else if ( Kademlia::CKademlia::IsConnected() && Kademlia::CKademlia::IsFirewalled() )
		ID = 1;
	else
		ID = 0;
	return ID;
}

uint32 CemuleApp::GetPublicIP(bool bIgnoreKadIP) const {
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
	if(m_dwPublicIP == 0 && theApp.bandwidthControl->IsNAFCWorking() && NeoPrefs.IsCheckAdapter())
		return theApp.bandwidthControl->GetAdapterIP(); // NEO: NAFC - [NetworkAdapterFeatbackControl]
#endif // NEO_BC // NEO: NAFC END <-- Xanatos --
	if (m_dwPublicIP == 0 && Kademlia::CKademlia::IsConnected() && Kademlia::CKademlia::GetIPAddress() && !bIgnoreKadIP)
		return ntohl(Kademlia::CKademlia::GetIPAddress());
	return m_dwPublicIP;
}

void CemuleApp::SetPublicIP(const uint32 dwIP){
	if (dwIP != 0){
		ASSERT ( !IsLowID(dwIP));
		ASSERT ( m_pPeerCache );

		if ( GetPublicIP() == 0)
			AddDebugLogLine(DLP_VERYLOW, false, _T("My public IP Address is: %s"),ipstr(dwIP));
		else if (Kademlia::CKademlia::IsConnected() && Kademlia::CKademlia::GetPrefs()->GetIPAddress())
			if(ntohl(Kademlia::CKademlia::GetIPAddress()) != dwIP)
				AddDebugLogLine(DLP_DEFAULT, false,  _T("Public IP Address reported from Kademlia (%s) differs from new found (%s)"),ipstr(ntohl(Kademlia::CKademlia::GetIPAddress())),ipstr(dwIP));
		m_pPeerCache->FoundMyPublicIPAddress(dwIP);	
	}
	else
		AddDebugLogLine(DLP_VERYLOW, false, _T("Deleted public IP"));
	
	if (dwIP != 0 && dwIP != m_dwPublicIP && serverlist != NULL){
		m_dwPublicIP = dwIP;
		serverlist->CheckForExpiredUDPKeys();
	}
	else
		m_dwPublicIP = dwIP;
}

// NEO: RIC - [ReaskOnIDChange] -- Xanatos -->
void CemuleApp::CheckIDChange(const uint32 dwIP, int iFrom)
{
	ASSERT(dwIP);

	bool bIDOnly = false;
	if(m_uLastIP == dwIP){ // ip unchanged
		bIDOnly = true;
		if(!NeoPrefs.IsCheckL2HIDChange() || !IsConnected() || m_bLastIDLow == IsLowID(GetID()))
			return;
	}else{ // ip changed or new
#ifdef NEO_BC  // NEO: NAFC - [NetworkAdapterFeatbackControl]
		if(iFrom != 1 && NeoPrefs.IsNAFCEnabled())
			theApp.bandwidthControl->SetAdapterIndex(dwIP);
#endif // NEO_BC // NEO: NAFC END
	}

	if(m_uLastIP == 0){ // Its the first IP we got
		m_uLastIP = dwIP;
		if(IsConnected())
			m_bLastIDLow = IsLowID(GetID());
		m_uLastChangeID = ::GetTickCount();
		return;
	}

	// Public IP/ID has been changed, it's necessary to inform all sources about it
	// All sources would be informed during their next session refresh (with TCP)
	// about the new IP.		
	ModLog(LOG_STATUSBAR, GetResString(IDS_X_ID_CHANGE), 
		ipstr(m_uLastIP),m_bLastIDLow ? _T("low") : _T("high"),
		ipstr(dwIP),IsConnected() ? (IsLowID(GetID())  ? _T("low") : _T("high")) : _T("unknown"));

	m_uLastIP = dwIP;
	m_bLastIDLow = IsLowID(GetID());
	m_uLastChangeID = ::GetTickCount();

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
	if(!bIDOnly && NeoPrefs.IsRebindSocketsOnIPChange() && !NeoPrefs.IsBindToAdapter()) // NAFC will take care of the rebind
#else
	if(!bIDOnly && NeoPrefs.IsRebindSocketsOnIPChange())
#endif // NEO_BC // NEO: NBC END
	{
		ModLog(GetResString(IDS_X_BIND_SOCKETS), ipstr(dwIP));

		listensocket->Rebind();
		clientudp->Rebind();
		serverconnect->Rebind();
	}

	if(iFrom != 2 && !bIDOnly && NeoPrefs.IsReconnectKadOnIPChange()
	 && Kademlia::CKademlia::IsConnected() && Kademlia::CKademlia::GetPrefs()->GetIPAddress()
	 && (ntohl(Kademlia::CKademlia::GetIPAddress()) != dwIP)
	 ){
		// Xman reconnect Kad on IP-change
		Kademlia::CKademlia::Stop();
		Kademlia::CKademlia::ClearIPAddress(); // David: fix, avoid to stack with the old one
		Kademlia::CKademlia::Start();
	}

	bool bQSNeeded = false;
	if(NeoPrefs.IsInformOnIPChange()){
		theApp.clientlist->InformSourcesAboutIDChage();
		ModLog(GetResString(IDS_X_INFORM_ON_IP_CHANGE));
		bQSNeeded = true;
	}
	if(NeoPrefs.IsReAskOnIPChange()){
		theApp.clientlist->ReAskAllClientsForDownload();
		ModLog(GetResString(IDS_X_REASKING_ON_IP));
		bQSNeeded = true;
	}

	// NEO: QS - [QuickStart]
	if(bQSNeeded && NeoPrefs.UseQuickStartOnIPChange())
		theApp.downloadqueue->DoQuickStart();
	// NEO: QS END
}
// NEO: RIC END <-- Xanatos --

bool CemuleApp::IsFirewalled()
{
	if (theApp.serverconnect->IsConnected() && !theApp.serverconnect->IsLowID())
		return false; // we have an eD2K HighID -> not firewalled

	if (Kademlia::CKademlia::IsConnected() && !Kademlia::CKademlia::IsFirewalled())
		return false; // we have an Kad HighID -> not firewalled

	return true; // firewalled
}

bool CemuleApp::CanDoCallback( CUpDownClient *client )
{
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
	// Note: this is ment for debug purpose only, when we add a source by the CAddSourceDlg
	if(NeoPrefs.IsNatTraversalEnabled() && client->SupportsNatTraversal() && client->GetUserPort() == 0)
	{
		// the cleint does not have a TCP port and is nat traversal enabled we can try it
		return true;
	}
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --

#ifdef NATTUNNELING // NEO: XSB - [XSBuddy] -- Xanatos -->
	if(client->GetXsBuddyIP())
	{
		if(theApp.IsFirewalled())
		{
			// NEO: NATT - [NatTraversal]
			if(NeoPrefs.IsNatTraversalEnabled() && client->SupportsNatTraversal() 
			 && theApp.clientlist->GetXsBuddyStatus() == XB_HIGH_BUDDY) // we need also to have abuddy at the moment of the callback
			{
				//He have a Xs Buddy - We are LowID, but both Nat Traversal Enabled
				return true;
			}
			else
			// NEO: NATT END
			{
				//He have a Xs Buddy - We are LowID
				return false;
			}
		}
		else
		{
			//He have a Xs Buddy - We are HighID
			return true;
		}
	}
	else
#endif //NATTUNNELING // NEO: XSB END <-- Xanatos --

	if(Kademlia::CKademlia::IsConnected())
	{
		if(theApp.serverconnect->IsConnected())
		{
			if(theApp.serverconnect->IsLowID())
			{
				if(Kademlia::CKademlia::IsFirewalled())
				{
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
					if(NeoPrefs.IsNatTraversalEnabled() && client->SupportsNatTraversal(true)
						&& theApp.clientlist->GetBuddyStatus() == Connected) // we need also to have abuddy at the moment of the callback
					{
						//Both Connected - Both Firewalled, but both Nat Traversal Enabled
						return true;
					}
					else
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
					{
						//Both Connected - Both Firewalled
						return false;
					}
				}
				else
				{
					if(client->GetServerIP() == theApp.serverconnect->GetCurrentServer()->GetIP() && client->GetServerPort() == theApp.serverconnect->GetCurrentServer()->GetPort())
					{
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
						if(NeoPrefs.IsNatTraversalEnabled() && client->SupportsNatTraversal() && theApp.serverconnect->GetCurrentServer()->SupportsNatTraversal())
						{
							//Both Connected - Server lowID, Kad Open - Client on same serve
							return true;
						}
						else
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
						{
							//Both Connected - Server lowID, Kad Open - Client on same server
							//We prevent a callback to the server as this breaks the protocol and will get you banned.
							return false;
						}
					}
					else
					{
						//Both Connected - Server lowID, Kad Open - Client on remote server
						return true;
					}
				}
			}
			else
			{
				//Both Connected - Server HighID, Kad don't care
				return true;
			}
		}
		else
		{
			if(Kademlia::CKademlia::IsFirewalled())
			{
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
				if(NeoPrefs.IsNatTraversalEnabled() && client->SupportsNatTraversal(true)
				 && theApp.clientlist->GetBuddyStatus() == Connected) // we need also to have abuddy at the moment of the callback
				{
					//Only Kad Connected - Kad Firewalled, but both Nat Traversal Enabled
					return true;
				}
				else
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
				{
					//Only Kad Connected - Kad Firewalled
					return false;
				}
			}
			else
			{
				//Only Kad Conected - Kad Open
				return true;
			}
		}
	}
	else
	{
		if( theApp.serverconnect->IsConnected() )
		{
			if( theApp.serverconnect->IsLowID() )
			{
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
				if(NeoPrefs.IsNatTraversalEnabled() && client->SupportsNatTraversal() && theApp.serverconnect->GetCurrentServer()->SupportsNatTraversal())
				{
					//Only Server Connected - Server LowID, but both Nat Traversal Enabled
					return true;
				}
				else
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
				{
					//Only Server Connected - Server LowID
					return false;
				}
			}
			else
			{
				//Only Server Connected - Server HighID
				return true;
			}
		}
		else
		{
			//We are not connected at all!
			return false;
		}
	}
}

HICON CemuleApp::LoadIcon(UINT nIDResource) const
{
	// use string resource identifiers!!
	return CWinApp::LoadIcon(nIDResource);
}

HICON CemuleApp::LoadIcon(LPCTSTR lpszResourceName, int cx, int cy, UINT uFlags) const
{
	// Test using of 16 color icons. If 'LR_VGACOLOR' is specified _and_ the icon resource
	// contains a 16 color version, that 16 color version will be loaded. If there is no
	// 16 color version available, Windows will use the next (better) color version found.
#ifdef _DEBUG
	if (g_bLowColorDesktop)
		uFlags |= LR_VGACOLOR;
#endif

	HICON hIcon = NULL;
	LPCTSTR pszSkinProfile = thePrefs.GetSkinProfile();
	if (pszSkinProfile != NULL && pszSkinProfile[0] != _T('\0'))
	{
		// load icon resource file specification from skin profile
		TCHAR szSkinResource[MAX_PATH];
		GetPrivateProfileString(_T("Icons"), lpszResourceName, _T(""), szSkinResource, _countof(szSkinResource), pszSkinProfile);
		if (szSkinResource[0] != _T('\0'))
		{
			// expand any optional available environment strings
			TCHAR szExpSkinRes[MAX_PATH];
			if (ExpandEnvironmentStrings(szSkinResource, szExpSkinRes, _countof(szExpSkinRes)) != 0)
			{
				_tcsncpy(szSkinResource, szExpSkinRes, _countof(szSkinResource));
				szSkinResource[_countof(szSkinResource)-1] = _T('\0');
			}

			// create absolute path to icon resource file
			TCHAR szFullResPath[MAX_PATH];
			if (PathIsRelative(szSkinResource))
			{
				TCHAR szSkinResFolder[MAX_PATH];
				_tcsncpy(szSkinResFolder, pszSkinProfile, _countof(szSkinResFolder));
				szSkinResFolder[_countof(szSkinResFolder)-1] = _T('\0');
				PathRemoveFileSpec(szSkinResFolder);
				_tmakepathlimit(szFullResPath, NULL, szSkinResFolder, szSkinResource, NULL);
			}
			else
			{
				_tcsncpy(szFullResPath, szSkinResource, _countof(szFullResPath));
				szFullResPath[_countof(szFullResPath)-1] = _T('\0');
			}

			// check for optional icon index or resource identifier within the icon resource file
			bool bExtractIcon = false;
			CString strFullResPath = szFullResPath;
			int iIconIndex = 0;
			int iComma = strFullResPath.ReverseFind(_T(','));
			if (iComma != -1){
				if (_stscanf((LPCTSTR)strFullResPath + iComma + 1, _T("%d"), &iIconIndex) == 1)
					bExtractIcon = true;
				strFullResPath = strFullResPath.Left(iComma);
			}

			if (bExtractIcon)
			{
				if (uFlags != 0 || !(cx == cy && (cx == 16 || cx == 32)))
				{
					static UINT (WINAPI *_pfnPrivateExtractIcons)(LPCTSTR, int, int, int, HICON*, UINT*, UINT, UINT) = (UINT (WINAPI *)(LPCTSTR, int, int, int, HICON*, UINT*, UINT, UINT))GetProcAddress(GetModuleHandle(_T("user32")), _TWINAPI("PrivateExtractIcons"));
					if (_pfnPrivateExtractIcons)
					{
						UINT uIconId;
						(*_pfnPrivateExtractIcons)(strFullResPath, iIconIndex, cx, cy, &hIcon, &uIconId, 1, uFlags);
					}
				}

				if (hIcon == NULL)
				{
					HICON aIconsLarge[1] = {0};
					HICON aIconsSmall[1] = {0};
					int iExtractedIcons = ExtractIconEx(strFullResPath, iIconIndex, aIconsLarge, aIconsSmall, 1);
					if (iExtractedIcons > 0) // 'iExtractedIcons' is 2(!) if we get a large and a small icon
					{
						// alway try to return the icon size which was requested
						if (cx == 16 && aIconsSmall[0] != NULL)
						{
							hIcon = aIconsSmall[0];
							aIconsSmall[0] = NULL;
						}
						else if (cx == 32 && aIconsLarge[0] != NULL)
						{
							hIcon = aIconsLarge[0];
							aIconsLarge[0] = NULL;
						}
						else
						{
							if (aIconsSmall[0] != NULL)
							{
								hIcon = aIconsSmall[0];
								aIconsSmall[0] = NULL;
							}
							else if (aIconsLarge[0] != NULL)
							{
								hIcon = aIconsLarge[0];
								aIconsLarge[0] = NULL;
							}
						}

						for (int i = 0; i < _countof(aIconsLarge); i++)
						{
							if (aIconsLarge[i] != NULL)
								VERIFY( DestroyIcon(aIconsLarge[i]) );
							if (aIconsSmall[i] != NULL)
								VERIFY( DestroyIcon(aIconsSmall[i]) );
						}
					}
				}
			}
			else
			{
				// WINBUG???: 'ExtractIcon' does not work well on ICO-files when using the color 
				// scheme 'Windows-Standard (extragroß)' -> always try to use 'LoadImage'!
				//
				// If the ICO file contains a 16x16 icon, 'LoadImage' will though return a 32x32 icon,
				// if LR_DEFAULTSIZE is specified! -> always specify the requested size!
				hIcon = (HICON)::LoadImage(NULL, szFullResPath, IMAGE_ICON, cx, cy, uFlags | LR_LOADFROMFILE);
				if (hIcon == NULL && GetLastError() != ERROR_PATH_NOT_FOUND && g_bGdiPlusInstalled)
				{
					ULONG_PTR gdiplusToken = 0;
					Gdiplus::GdiplusStartupInput gdiplusStartupInput;
					if (Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) == Gdiplus::Ok)
					{
						Gdiplus::Bitmap bmp(szFullResPath);
						bmp.GetHICON(&hIcon);
					}
					Gdiplus::GdiplusShutdown(gdiplusToken);
				}
			}
		}
	}

	if (hIcon == NULL)
	{
		if (cx != LR_DEFAULTSIZE || cy != LR_DEFAULTSIZE || uFlags != LR_DEFAULTCOLOR)
			hIcon = (HICON)::LoadImage(AfxGetResourceHandle(), lpszResourceName, IMAGE_ICON, cx, cy, uFlags);
		if (hIcon == NULL)
		{
			//TODO: Either do not use that function or copy the returned icon. All the calling code is designed
			// in a way that the icons returned by this function are to be freed with 'DestroyIcon'. But an 
			// icon which was loaded with 'LoadIcon', is not be freed with 'DestroyIcon'.
			// Right now, we never come here...
			ASSERT(0);
			hIcon = CWinApp::LoadIcon(lpszResourceName);
		}
	}
	return hIcon;
}

HBITMAP CemuleApp::LoadImage(LPCTSTR lpszResourceName, LPCTSTR pszResourceType) const
{
	LPCTSTR pszSkinProfile = thePrefs.GetSkinProfile();
	if (pszSkinProfile != NULL && pszSkinProfile[0] != _T('\0'))
	{
		// load resource file specification from skin profile
		TCHAR szSkinResource[MAX_PATH];
		GetPrivateProfileString(_T("Bitmaps"), lpszResourceName, _T(""), szSkinResource, _countof(szSkinResource), pszSkinProfile);
		if (szSkinResource[0] != _T('\0'))
		{
			// expand any optional available environment strings
			TCHAR szExpSkinRes[MAX_PATH];
			if (ExpandEnvironmentStrings(szSkinResource, szExpSkinRes, _countof(szExpSkinRes)) != 0)
			{
				_tcsncpy(szSkinResource, szExpSkinRes, _countof(szSkinResource));
				szSkinResource[_countof(szSkinResource)-1] = _T('\0');
			}

			// create absolute path to resource file
			TCHAR szFullResPath[MAX_PATH];
			if (PathIsRelative(szSkinResource))
			{
				TCHAR szSkinResFolder[MAX_PATH];
				_tcsncpy(szSkinResFolder, pszSkinProfile, _countof(szSkinResFolder));
				szSkinResFolder[_countof(szSkinResFolder)-1] = _T('\0');
				PathRemoveFileSpec(szSkinResFolder);
				_tmakepathlimit(szFullResPath, NULL, szSkinResFolder, szSkinResource, NULL);
			}
			else
			{
				_tcsncpy(szFullResPath, szSkinResource, _countof(szFullResPath));
				szFullResPath[_countof(szFullResPath)-1] = _T('\0');
			}

			CEnBitmap bmp;
			if (bmp.LoadImage(szFullResPath))
				return (HBITMAP)bmp.Detach();
		}
	}

	CEnBitmap bmp;
	if (bmp.LoadImage(lpszResourceName, pszResourceType))
		return (HBITMAP)bmp.Detach();
	return NULL;
}

CString CemuleApp::GetSkinFileItem(LPCTSTR lpszResourceName, LPCTSTR pszResourceType) const
{
	LPCTSTR pszSkinProfile = thePrefs.GetSkinProfile();
	if (pszSkinProfile != NULL && pszSkinProfile[0] != _T('\0'))
	{
		// load resource file specification from skin profile
		TCHAR szSkinResource[MAX_PATH];
		GetPrivateProfileString(pszResourceType, lpszResourceName, _T(""), szSkinResource, _countof(szSkinResource), pszSkinProfile);
		if (szSkinResource[0] != _T('\0'))
		{
			// expand any optional available environment strings
			TCHAR szExpSkinRes[MAX_PATH];
			if (ExpandEnvironmentStrings(szSkinResource, szExpSkinRes, _countof(szExpSkinRes)) != 0)
			{
				_tcsncpy(szSkinResource, szExpSkinRes, _countof(szSkinResource));
				szSkinResource[_countof(szSkinResource)-1] = _T('\0');
			}

			// create absolute path to resource file
			TCHAR szFullResPath[MAX_PATH];
			if (PathIsRelative(szSkinResource))
			{
				TCHAR szSkinResFolder[MAX_PATH];
				_tcsncpy(szSkinResFolder, pszSkinProfile, _countof(szSkinResFolder));
				szSkinResFolder[_countof(szSkinResFolder)-1] = _T('\0');
				PathRemoveFileSpec(szSkinResFolder);
				_tmakepathlimit(szFullResPath, NULL, szSkinResFolder, szSkinResource, NULL);
			}
			else
			{
				_tcsncpy(szFullResPath, szSkinResource, _countof(szFullResPath));
				szFullResPath[_countof(szFullResPath)-1] = _T('\0');
			}

			return szFullResPath;
		}
	}
	return _T("");
}

bool CemuleApp::LoadSkinColor(LPCTSTR pszKey, COLORREF& crColor) const
{
	LPCTSTR pszSkinProfile = thePrefs.GetSkinProfile();
	if (pszSkinProfile != NULL && pszSkinProfile[0] != _T('\0'))
	{
		TCHAR szColor[MAX_PATH];
		GetPrivateProfileString(_T("Colors"), pszKey, _T(""), szColor, _countof(szColor), pszSkinProfile);
		if (szColor[0] != _T('\0'))
		{
			UINT red, grn, blu;
			int iVals = _stscanf(szColor, _T("%i , %i , %i"), &red, &grn, &blu);
			if (iVals == 3)
			{
				crColor = RGB(red, grn, blu);
				return true;
			}
		}
	}
	return false;
}

bool CemuleApp::LoadSkinColorAlt(LPCTSTR pszKey, LPCTSTR pszAlternateKey, COLORREF& crColor) const
{
	if (LoadSkinColor(pszKey, crColor))
		return true;
	return LoadSkinColor(pszAlternateKey, crColor);
}

void CemuleApp::ApplySkin(LPCTSTR pszSkinProfile)
{
	thePrefs.SetSkinProfile(pszSkinProfile);
	AfxGetMainWnd()->SendMessage(WM_SYSCOLORCHANGE);
}

CTempIconLoader::CTempIconLoader(LPCTSTR pszResourceID, int cx, int cy, UINT uFlags)
{
	m_hIcon = theApp.LoadIcon(pszResourceID, cx, cy, uFlags);
}

CTempIconLoader::CTempIconLoader(UINT uResourceID, int /*cx*/, int /*cy*/, UINT uFlags)
{
	UNREFERENCED_PARAMETER(uFlags);
	ASSERT( uFlags == 0 );
	m_hIcon = theApp.LoadIcon(uResourceID);
}

CTempIconLoader::~CTempIconLoader()
{
	if (m_hIcon)
		VERIFY( DestroyIcon(m_hIcon) );
}

void CemuleApp::AddEd2kLinksToDownload(CString strLinks, int cat)
{
	int curPos = 0;
	CString strTok = strLinks.Tokenize(_T(" \t\r\n"), curPos); // tokenize by whitespaces
	while (!strTok.IsEmpty())
	{
		if (strTok.Right(1) != _T("/"))
			strTok += _T("/");
		try
		{
			CED2KLink* pLink = CED2KLink::CreateLinkFromUrl(strTok);
			if (pLink)
			{
				if (pLink->GetKind() == CED2KLink::kFile)
				{
					downloadqueue->AddFileLinkToDownload(pLink->GetFileLink(), cat);
				}
				else
				{
					delete pLink;
					throw CString(_T("bad link"));
				}
				//delete pLink; // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
			}
		}
		catch(CString error)
		{
			TCHAR szBuffer[200];
			_sntprintf(szBuffer, _countof(szBuffer), GetResString(IDS_ERR_INVALIDLINK), error);
			szBuffer[_countof(szBuffer) - 1] = _T('\0');
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_LINKERROR), szBuffer);
		}
		strTok = strLinks.Tokenize(_T(" \t\r\n"), curPos); // tokenize by whitespaces
	}
}

void CemuleApp::SearchClipboard()
{
	if (m_bGuardClipboardPrompt)
		return;

	CString strLinks = CopyTextFromClipboard();
	if (strLinks.IsEmpty())
		return;

	if (strLinks.Compare(m_strLastClipboardContents) == 0)
		return;

	// Do not alter (trim) 'strLinks' and then copy back to 'm_strLastClipboardContents'! The
	// next clipboard content compare would fail because of the modified string.
	LPCTSTR pszTrimmedLinks = strLinks;
	while (_istspace((_TUCHAR)*pszTrimmedLinks)) // Skip leading whitespaces
		pszTrimmedLinks++;
	if (_tcsnicmp(pszTrimmedLinks, _T("ed2k://|file|"), 13) == 0)
	{
		m_bGuardClipboardPrompt = true;

		// Don't feed too long strings into the MessageBox function, it may freak out..
		CString strLinksDisplay;
		if (strLinks.GetLength() > 512)
			strLinksDisplay = strLinks.Left(512) + _T("...");
		else
			strLinksDisplay = strLinks;
		if (AfxMessageBox(GetResString(IDS_ADDDOWNLOADSFROMCB) + _T("\r\n") + strLinksDisplay, MB_YESNO | MB_TOPMOST) == IDYES)
			//AddEd2kLinksToDownload(pszTrimmedLinks, 0);
			AddEd2kLinksToDownload(pszTrimmedLinks, -1); // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
	}
	m_strLastClipboardContents = strLinks; // Save the unmodified(!) clipboard contents
	m_bGuardClipboardPrompt = false;
}

void CemuleApp::PasteClipboard(int cat)
{
	CString strLinks = CopyTextFromClipboard();
	strLinks.Trim();
	if (strLinks.IsEmpty())
		return;

	AddEd2kLinksToDownload(strLinks, cat);
}

bool CemuleApp::IsEd2kLinkInClipboard(LPCSTR pszLinkType, int iLinkTypeLen)
{
	bool bFoundLink = false;
	if (IsClipboardFormatAvailable(CF_TEXT))
	{
		if (OpenClipboard(NULL))
		{
			HGLOBAL	hText = GetClipboardData(CF_TEXT);
			if (hText != NULL)
			{ 
				// Use the ANSI string
				LPCSTR pszText = (LPCSTR)GlobalLock(hText);
				if (pszText != NULL)
				{
					while (isspace((unsigned char)*pszText))
						pszText++;
					bFoundLink = (_strnicmp(pszText, pszLinkType, iLinkTypeLen) == 0);
					GlobalUnlock(hText);
				}
			}
			CloseClipboard();
		}
	}

	return bFoundLink;
}

bool CemuleApp::IsEd2kFileLinkInClipboard()
{
	static const CHAR _szEd2kFileLink[] = "ed2k://|file|"; // Use the ANSI string
	return IsEd2kLinkInClipboard(_szEd2kFileLink, _countof(_szEd2kFileLink)-1);
}

bool CemuleApp::IsEd2kServerLinkInClipboard()
{
	static const CHAR _szEd2kServerLink[] = "ed2k://|server|"; // Use the ANSI string
	return IsEd2kLinkInClipboard(_szEd2kServerLink, _countof(_szEd2kServerLink)-1);
}

// NEO: TFL - [TetraFriendLinks] -- Xanatos -->
bool CemuleApp::IsEd2kFriendLinkInClipboard()
{
	static const CHAR _szEd2kFriendLink[] = "ed2k://|friend|"; // Use the ANSI string
	return IsEd2kLinkInClipboard(_szEd2kFriendLink, ARRSIZE(_szEd2kFriendLink)-1);
}
// NEO: TFL END  <-- Xanatos --

// Elandal:ThreadSafeLogging -->
void CemuleApp::QueueDebugLogLine(bool bAddToStatusbar, LPCTSTR line, ...)
{
	if (!thePrefs.GetVerbose())
		return;

	m_queueLock.Lock();

	TCHAR bufferline[1000];
	va_list argptr;
	va_start(argptr, line);
	int iLen = _vsntprintf(bufferline, _countof(bufferline), line, argptr);
	va_end(argptr);
	if (iLen > 0)
	{
		SLogItem* newItem = new SLogItem;
		newItem->uFlags = LOG_DEBUG | (bAddToStatusbar ? LOG_STATUSBAR : 0);
		newItem->line.SetString(bufferline, iLen);
		m_QueueDebugLog.AddTail(newItem);
	}

	m_queueLock.Unlock();
}

void CemuleApp::QueueLogLine(bool bAddToStatusbar, LPCTSTR line, ...)
{
	m_queueLock.Lock();

	TCHAR bufferline[1000];
	va_list argptr;
	va_start(argptr, line);
	int iLen = _vsntprintf(bufferline, _countof(bufferline), line, argptr);
	va_end(argptr);
	if (iLen > 0)
	{
		SLogItem* newItem = new SLogItem;
		newItem->uFlags = bAddToStatusbar ? LOG_STATUSBAR : 0;
		newItem->line.SetString(bufferline, iLen);
		m_QueueLog.AddTail(newItem);
	}

	m_queueLock.Unlock();
}

void CemuleApp::QueueDebugLogLineEx(UINT uFlags, LPCTSTR line, ...)
{
	if (!thePrefs.GetVerbose())
		return;

	m_queueLock.Lock();

	TCHAR bufferline[1000];
	va_list argptr;
	va_start(argptr, line);
	int iLen = _vsntprintf(bufferline, _countof(bufferline), line, argptr);
	va_end(argptr);
	if (iLen > 0)
	{
		SLogItem* newItem = new SLogItem;
		newItem->uFlags = uFlags | LOG_DEBUG;
		newItem->line.SetString(bufferline, iLen);
		m_QueueDebugLog.AddTail(newItem);
	}

	m_queueLock.Unlock();
}

void CemuleApp::QueueLogLineEx(UINT uFlags, LPCTSTR line, ...)
{
	m_queueLock.Lock();

	TCHAR bufferline[1000];
	va_list argptr;
	va_start(argptr, line);
	int iLen = _vsntprintf(bufferline, _countof(bufferline), line, argptr);
	va_end(argptr);
	if (iLen > 0)
	{
		SLogItem* newItem = new SLogItem;
		newItem->uFlags = uFlags;
		newItem->line.SetString(bufferline, iLen);
		m_QueueLog.AddTail(newItem);
	}

	m_queueLock.Unlock();
}

void CemuleApp::HandleDebugLogQueue()
{
	m_queueLock.Lock();
	while (!m_QueueDebugLog.IsEmpty())
	{
		const SLogItem* newItem = m_QueueDebugLog.RemoveHead();
		if (thePrefs.GetVerbose())
			Log(newItem->uFlags, _T("%s"), newItem->line);
		delete newItem;
	}
	m_queueLock.Unlock();
}

void CemuleApp::HandleLogQueue()
{
	m_queueLock.Lock();
	while (!m_QueueLog.IsEmpty())
	{
		const SLogItem* newItem = m_QueueLog.RemoveHead();
		Log(newItem->uFlags, _T("%s"), newItem->line);
		delete newItem;
	}
	m_queueLock.Unlock();
}

void CemuleApp::ClearDebugLogQueue(bool bDebugPendingMsgs)
{
	m_queueLock.Lock();
	while(!m_QueueDebugLog.IsEmpty())
	{
		if (bDebugPendingMsgs)
			TRACE(_T("Queued dbg log msg: %s\n"), m_QueueDebugLog.GetHead()->line);
		delete m_QueueDebugLog.RemoveHead();
	}
	m_queueLock.Unlock();
}

void CemuleApp::ClearLogQueue(bool bDebugPendingMsgs)
{
	m_queueLock.Lock();
	while(!m_QueueLog.IsEmpty())
	{
		if (bDebugPendingMsgs)
			TRACE(_T("Queued log msg: %s\n"), m_QueueLog.GetHead()->line);
		delete m_QueueLog.RemoveHead();
	}
	m_queueLock.Unlock();
}

// NEO: ML - [ModLog] -- Xanatos -->
void CemuleApp::QueueModLogLine(bool addtostatusbar, LPCTSTR line,...)
{
	m_queueLock.Lock();

	TCHAR bufferline[1000];

	va_list argptr;
	va_start(argptr, line);
	_vsntprintf(bufferline, ARRSIZE(bufferline), line, argptr);
	va_end(argptr);

	SLogItem* newItem = new SLogItem;
	newItem->uFlags = (addtostatusbar ? LOG_STATUSBAR : 0);
	newItem->line = bufferline;
	m_QueueModLog.AddTail(newItem);

	m_queueLock.Unlock();
}

void CemuleApp::QueueModLogLineEx(UINT uFlags, LPCTSTR line, ...)
{
	m_queueLock.Lock();

	TCHAR bufferline[1000];

	va_list argptr;
	va_start(argptr, line);
	_vsntprintf(bufferline, ARRSIZE(bufferline), line, argptr);
	va_end(argptr);

	SLogItem* newItem = new SLogItem;
	newItem->uFlags = uFlags;
	newItem->line = bufferline;
	m_QueueModLog.AddTail(newItem);

	m_queueLock.Unlock();
}

void CemuleApp::HandleModLogQueue()
{
	m_queueLock.Lock();
	while (!m_QueueModLog.IsEmpty())
	{
		const SLogItem* newItem = m_QueueModLog.RemoveHead();
		ModLog(newItem->uFlags, _T("%s"), newItem->line);
		delete newItem;
	}
	m_queueLock.Unlock();
}

void CemuleApp::ClearModLogQueue(bool bDebugPendingMsgs)
{
	m_queueLock.Lock();
	while(!m_QueueModLog.IsEmpty())
	{
		if (bDebugPendingMsgs)
			TRACE("Queued log msg: %s\n", m_QueueLog.GetHead()->line);
		delete m_QueueModLog.RemoveHead();
	}
	m_queueLock.Unlock();
}
// NEO: ML END <-- Xanatos --

// Elandal:ThreadSafeLogging <--

void CemuleApp::CreateAllFonts()
{
	///////////////////////////////////////////////////////////////////////////
	// Symbol font
	//
	//VERIFY( CreatePointFont(m_fontSymbol, 10 * 10, _T("Marlett")) );
	// Creating that font with 'SYMBOL_CHARSET' should be safer (seen in ATL/MFC code). Though
	// it seems that it does not solve the problem with '6' and '9' characters which are
	// shown for some ppl.
	m_fontSymbol.CreateFont(GetSystemMetrics(SM_CYMENUCHECK), 0, 0, 0,
		FW_NORMAL, 0, 0, 0, SYMBOL_CHARSET, 0, 0, 0, 0, _T("Marlett"));


	///////////////////////////////////////////////////////////////////////////
	// Default GUI Font
	//
	// Fonts which are returned by 'GetStockObject'
	// --------------------------------------------
	// OEM_FIXED_FONT		Terminal
	// ANSI_FIXED_FONT		Courier
	// ANSI_VAR_FONT		MS Sans Serif
	// SYSTEM_FONT			System
	// DEVICE_DEFAULT_FONT	System
	// SYSTEM_FIXED_FONT	Fixedsys
	// DEFAULT_GUI_FONT		MS Shell Dlg (*1)
	//
	// (*1) Do not use 'GetStockObject(DEFAULT_GUI_FONT)' to get the 'Tahoma' font. It does
	// not work...
	//	
	// The documentation in MSDN states that DEFAULT_GUI_FONT returns 'Tahoma' on 
	// Win2000/XP systems. Though this is wrong, it may be true for US-English locales, but
	// it is wrong for other locales. Furthermore it is even documented that "MS Shell Dlg"
	// gets mapped to "MS Sans Serif" on Windows XP systems. Only "MS Shell Dlg 2" would
	// get mapped to "Tahoma", but "MS Shell Dlg 2" can not be used on prior Windows
	// systems.
	//
	// The reason why "MS Shell Dlg" is though mapped to "Tahoma" when used within dialog
	// resources is unclear.
	//
	// So, to get the same font which is used within dialogs which were created via dialog
	// resources which have the "MS Shell Dlg, 8" specified (again, in that special case
	// "MS Shell Dlg" gets mapped to "Tahoma" and not to "MS Sans Serif"), we just query
	// the main window (which is also a dialog) for the current font.
	//
	LOGFONT lfDefault;
	AfxGetMainWnd()->GetFont()->GetLogFont(&lfDefault);
	// WinXP: lfDefault.lfFaceName = "MS Shell Dlg 2" (!)
	// Vista: lfDefault.lfFaceName = "MS Shell Dlg 2"
	//
	// It would not be an error if that font name does not match our pre-determined
	// font name, I just want to know if that ever happens.
	ASSERT( m_strDefaultFontFaceName == lfDefault.lfFaceName );


	///////////////////////////////////////////////////////////////////////////
	// Bold Default GUI Font
	//
	LOGFONT lfDefaultBold = lfDefault;
	lfDefaultBold.lfWeight = FW_BOLD;
	VERIFY( m_fontDefaultBold.CreateFontIndirect(&lfDefaultBold) );


	///////////////////////////////////////////////////////////////////////////
	// Server Log-, Message- and IRC-Window font
	//
	// Since we use "MS Shell Dlg 2" under WinXP (which will give us "Tahoma"),
	// that font is nevertheless set to "MS Sans Serif" because a scaled up "Tahoma"
	// font unfortunately does not look as good as a scaled up "MS Sans Serif" font.
	//
	// No! Do *not* use "MS Sans Serif" (never!). This will give a very old fashioned
	// font on certain Asian Windows systems. So, better use "MS Shell Dlg" or 
	// "MS Shell Dlg 2" to let Windows map that font to the proper font on all Windows
	// systems.
	//
	LPLOGFONT plfHyperText = thePrefs.GetHyperTextLogFont();
	if (plfHyperText==NULL || plfHyperText->lfFaceName[0]==_T('\0') || !m_fontHyperText.CreateFontIndirect(plfHyperText))
		CreatePointFont(m_fontHyperText, 10 * 10, lfDefault.lfFaceName);

	///////////////////////////////////////////////////////////////////////////
	// Verbose Log-font
	//
	// Why can't this font set via the font dialog??
//	HFONT hFontMono = CreateFont(10, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("Lucida Console"));
//	m_fontLog.Attach(hFontMono);
	LPLOGFONT plfLog = thePrefs.GetLogFont();
	if (plfLog!=NULL && plfLog->lfFaceName[0]!=_T('\0'))
		m_fontLog.CreateFontIndirect(plfLog);

	///////////////////////////////////////////////////////////////////////////
	// Font used for Message and IRC edit control, default font, just a little
	// larger.
	//
	// Since we use "MS Shell Dlg 2" under WinXP (which will give us "Tahoma"),
	// that font is nevertheless set to "MS Sans Serif" because a scaled up "Tahoma"
	// font unfortunately does not look as good as a scaled up "MS Sans Serif" font.
	//
	// No! Do *not* use "MS Sans Serif" (never!). This will give a very old fashioned
	// font on certain Asian Windows systems. So, better use "MS Shell Dlg" or 
	// "MS Shell Dlg 2" to let Windows map that font to the proper font on all Windows
	// systems.
	//
	CreatePointFont(m_fontChatEdit, 11 * 10, lfDefault.lfFaceName);
}

const CString &CemuleApp::GetDefaultFontFaceName()
{
	if (m_strDefaultFontFaceName.IsEmpty())
	{
		OSVERSIONINFO osvi;
		osvi.dwOSVersionInfoSize = sizeof(osvi);
		if (GetVersionEx(&osvi)
			&& osvi.dwPlatformId == VER_PLATFORM_WIN32_NT
			&& osvi.dwMajorVersion >= 5) // Win2000/XP or higher
			m_strDefaultFontFaceName = _T("MS Shell Dlg 2");
		else
			m_strDefaultFontFaceName = _T("MS Shell Dlg");
	}
	return m_strDefaultFontFaceName;
}

void CemuleApp::CreateBackwardDiagonalBrush()
{
	static const WORD awBackwardDiagonalBrushPattern[8] = { 0x0f, 0x1e, 0x3c, 0x78, 0xf0, 0xe1, 0xc3, 0x87 };
	CBitmap bm;
	if (bm.CreateBitmap(8, 8, 1, 1, awBackwardDiagonalBrushPattern))
	{
		LOGBRUSH logBrush = {0};
		logBrush.lbStyle = BS_PATTERN;
		logBrush.lbHatch = (int)bm.GetSafeHandle();
		logBrush.lbColor = RGB(0, 0, 0);
		VERIFY( m_brushBackwardDiagonal.CreateBrushIndirect(&logBrush) );
	}
}

void CemuleApp::UpdateDesktopColorDepth()
{
	g_bLowColorDesktop = (GetDesktopColorDepth() <= 8);
#ifdef _DEBUG
	if (!g_bLowColorDesktop)
		g_bLowColorDesktop = (GetProfileInt(_T("eMule"), _T("LowColorRes"), 0) != 0);
#endif

	if (g_bLowColorDesktop)
	{
		// If we have 4- or 8-bit desktop color depth, Windows will (by design) load only 
		// the 16 color versions of icons. Thus we force all image lists also to 4-bit format.
		m_iDfltImageListColorFlags = ILC_COLOR4;
	}
	else
	{
		// Get current desktop color depth and derive the image list format from it
		m_iDfltImageListColorFlags = GetAppImageListColorFlag();

		// Don't use 32-bit image lists if not supported by COMCTL32.DLL
		if (m_iDfltImageListColorFlags == ILC_COLOR32 && m_ullComCtrlVer < MAKEDLLVERULL(6,0,0,0)) {
			// We fall back to 16-bit image lists because we do not provide 24-bit
			// versions of icons any longer (due to resource size restrictions for Win98). We
			// could also fall back to 24-bit image lists here but the difference is minimal
			// and considered to be not worth the additinoal memory consumption.
			//
			// Though, do not fall back to 8-bit image lists because this would let Windows
			// reduce the color resolution to the standard 256 color window system palette.
			// We need a 16-bit or 24-bit image list to hold all our 256 color icons (which
			// are not pre-quantized to standard 256 color windows system palette) without
			// loosing any colors.
			m_iDfltImageListColorFlags = ILC_COLOR16;
		}

		// Don't use >8-bit image lists with OSs with restricted memory for GDI resources
		if (afxIsWin95()) {
			// NOTE: ILC_COLOR8 leads to converting all icons to the standard windows system
			// 256 color palette. Thus this option leads to loosing some color resolution.
			// Though there is no other chance with Win98 because of the 64K GDI limit.
			m_iDfltImageListColorFlags = ILC_COLOR8;
		}
	}

	// Doesn't help..
	//m_aExtToSysImgIdx.RemoveAll();
}

BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType)
{
	// *) This function is invoked by the system from within a *DIFFERENT* thread !!
	//
	// *) This function is invoked only, if eMule was started with "RUNAS"
	//		- when user explicitly/manually logs off from the system (CTRL_LOGOFF_EVENT).
	//		- when user explicitly/manually does a reboot or shutdown (also: CTRL_LOGOFF_EVENT).
	//		- when eMule issues a ExitWindowsEx(EWX_LOGOFF/EWX_REBOOT/EWX_SHUTDOWN)
	//
	// NOTE: Windows will in each case forcefully terminate the process after 20 seconds!
	// Every action which is started after receiving this notification will get forcefully
	// terminated by Windows after 20 seconds.

	if (thePrefs.GetDebug2Disk()) {
		static TCHAR szCtrlType[40];
		LPCTSTR pszCtrlType = NULL;
		if (dwCtrlType == CTRL_C_EVENT)				pszCtrlType = _T("CTRL_C_EVENT");
		else if (dwCtrlType == CTRL_BREAK_EVENT)	pszCtrlType = _T("CTRL_BREAK_EVENT");
		else if (dwCtrlType == CTRL_CLOSE_EVENT)	pszCtrlType = _T("CTRL_CLOSE_EVENT");
		else if (dwCtrlType == CTRL_LOGOFF_EVENT)	pszCtrlType = _T("CTRL_LOGOFF_EVENT");
		else if (dwCtrlType == CTRL_SHUTDOWN_EVENT)	pszCtrlType = _T("CTRL_SHUTDOWN_EVENT");
		else {
			_sntprintf(szCtrlType, _countof(szCtrlType), _T("0x%08x"), dwCtrlType);
			szCtrlType[_countof(szCtrlType) - 1] = _T('\0');
			pszCtrlType = szCtrlType;
		}
		theVerboseLog.Logf(_T("%hs: CtrlType=%s"), __FUNCTION__, pszCtrlType);

		// Default ProcessShutdownParameters: Level=0x00000280, Flags=0x00000000
		// Setting 'SHUTDOWN_NORETRY' does not prevent from getting terminated after 20 sec.
		//DWORD dwLevel = 0, dwFlags = 0;
		//GetProcessShutdownParameters(&dwLevel, &dwFlags);
		//theVerboseLog.Logf(_T("%hs: ProcessShutdownParameters #0: Level=0x%08x, Flags=0x%08x"), __FUNCTION__, dwLevel, dwFlags);
		//SetProcessShutdownParameters(dwLevel, SHUTDOWN_NORETRY);
	}

	if (dwCtrlType==CTRL_CLOSE_EVENT || dwCtrlType==CTRL_LOGOFF_EVENT || dwCtrlType==CTRL_SHUTDOWN_EVENT)
	{
		if (theApp.emuledlg && theApp.emuledlg->m_hWnd)
		{
			if (thePrefs.GetDebug2Disk())
				theVerboseLog.Logf(_T("%hs: Sending TM_CONSOLETHREADEVENT to main window"), __FUNCTION__);

			// Use 'SendMessage' to send the message to the (different) main thread. This is
			// done by intention because it lets this thread wait as long as the main thread
			// has called 'ExitProcess' or returns from processing the message. This is
			// needed to not let Windows terminate the process before the 20 sec. timeout.
			if (!theApp.emuledlg->SendMessage(TM_CONSOLETHREADEVENT, dwCtrlType, (LPARAM)GetCurrentThreadId()))
			{
				theApp.m_app_state = APP_STATE_SHUTTINGDOWN; // as a last attempt
				if (thePrefs.GetDebug2Disk())
					theVerboseLog.Logf(_T("%hs: Error: Failed to send TM_CONSOLETHREADEVENT to main window - error %u"), __FUNCTION__, GetLastError());
			}
		}
	}

	// Returning FALSE does not cause Windows to immediatly terminate the process. Though,
	// that only depends on the next registered console control handler. The default seems
	// to wait 20 sec. until the process has terminated. After that timeout Windows
	// nevertheless terminates the process.
	//
	// For whatever unknown reason, this is *not* always true!? It may happen that Windows
	// terminates the process *before* the 20 sec. timeout if (and only if) the console
	// control handler thread has already terminated. So, we have to take care that we do not
	// exit this thread before the main thread has called 'ExitProcess' (in a synchronous
	// way) -- see also the 'SendMessage' above.
	if (thePrefs.GetDebug2Disk())
		theVerboseLog.Logf(_T("%hs: returning"), __FUNCTION__);
	return FALSE; // FALSE: Let the system kill the process with the default handler.
}

void CemuleApp::UpdateLargeIconSize()
{
	// initialize with system values in case we don't find the Shell's registry key
	m_sizBigSystemIcon.cx = GetSystemMetrics(SM_CXICON);
	m_sizBigSystemIcon.cy = GetSystemMetrics(SM_CYICON);

	// get the Shell's registry key for the large icon size - the large icons which are 
	// returned by the Shell are based on that size rather than on the system icon size
	CRegKey key;
	if (key.Open(HKEY_CURRENT_USER, _T("Control Panel\\desktop\\WindowMetrics"), KEY_READ) == ERROR_SUCCESS)
	{
		TCHAR szShellLargeIconSize[12];
		ULONG ulChars = _countof(szShellLargeIconSize);
		if (key.QueryStringValue(_T("Shell Icon Size"), szShellLargeIconSize, &ulChars) == ERROR_SUCCESS)
		{
			UINT uIconSize = 0;
			if (_stscanf(szShellLargeIconSize, _T("%u"), &uIconSize) == 1 && uIconSize > 0)
			{
				m_sizBigSystemIcon.cx = uIconSize;
				m_sizBigSystemIcon.cy = uIconSize;
			}
		}
	}
}

void CemuleApp::ResetStandByIdleTimer()
{
	// check if anything is going on (ongoing upload, download or connected) and reset the idle timer if so
	if (IsConnected() || (uploadqueue != NULL && uploadqueue->GetUploadQueueLength() > 0)
		|| (downloadqueue != NULL && downloadqueue->GetDatarate() > 0))
	{
		EXECUTION_STATE (WINAPI *pfnSetThreadExecutionState)(EXECUTION_STATE);
		(FARPROC&)pfnSetThreadExecutionState = GetProcAddress(GetModuleHandle(_T("kernel32")), "SetThreadExecutionState");
		if (pfnSetThreadExecutionState)
			VERIFY( pfnSetThreadExecutionState(ES_SYSTEM_REQUIRED) );
		else
			ASSERT( false );
	}
}

bool CemuleApp::IsVistaThemeActive() const
{
	return theApp.m_ullComCtrlVer >= MAKEDLLVERULL(6,16,0,0) && g_xpStyle.IsThemeActive() && g_xpStyle.IsAppThemed();
}

// NEO: SS - [SplashScreen] -- Xanatos -->
void CemuleApp::ShowSplash(){
	if(m_pSplashWnd)
		return;
	m_pSplashWnd = new CSplashScreenEx;
	if(!m_pSplashWnd->Create(NULL,NULL,0,CSS_FADE | CSS_CENTERSCREEN /*| CSS_SHADOW | CSS_BACKONCLICK */)){
		delete m_pSplashWnd;
		m_pSplashWnd = NULL;
		return;
	}
	m_pSplashWnd->SetBitmap(_T("LOGO"),_T("JPG"));
	m_pSplashWnd->SetTextFont(_T("Arial"),100,CSS_TEXT_BOLD);
	//m_pSplashWnd->SetTextRect(CRect(4,174,216,331));
	m_pSplashWnd->SetTextRect(CRect(4,190,246,331));
	//m_pSplashWnd->SetTextColor(RGB(0,255,0));
	DWORD TextColor = RGB(0,128,255);
	theApp.LoadSkinColorAlt(_T("SplashTextLv"), _T("STLv"), TextColor);
	m_pSplashWnd->SetTextColor(TextColor);
	m_pSplashWnd->SetTextFormat(DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	m_pSplashWnd->Show();
	return;
}
void CemuleApp::UpdateSplash(LPCTSTR Text){
	TRACE("***Last Splash Label: %s\n", CStringA(Text));
	if(m_pSplashWnd)
		m_pSplashWnd->SetText(Text);
}
void CemuleApp::HideSplash(){
	if(m_pSplashWnd){
		m_pSplashWnd->Hide();
		delete m_pSplashWnd; m_pSplashWnd = NULL;
	}
}
void CemuleApp::AttachSplash(){
	if(m_pSplashWnd){
		m_pSplashWnd->DestroyWindow();
		m_pSplashWnd->Create(theApp.emuledlg,NULL,0,CSS_CENTERSCREEN | CSS_HIDEONCLICK);
		m_pSplashWnd->Show();
	}
}
// NEO: SS END <-- Xanatos --
