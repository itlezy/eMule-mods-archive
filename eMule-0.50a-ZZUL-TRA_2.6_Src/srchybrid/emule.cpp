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
#include "LastCommonRouteFinder.h"
#include "UploadBandwidthThrottler.h"
#include "ClientList.h"
#include "FriendList.h"
#include "ClientUDPSocket.h"
#include "DownloadQueue.h"
#include "IPFilter.h"
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
//#include "PeerCacheFinder.h" // X: [RPC] - [Remove PeerCache]
#include "emuleDlg.h"
#include "SearchDlg.h"
#include "enbitmap.h"
#include "FirewallOpener.h"
#include "StringConversion.h"
#include "Log.h"
#include "Collection.h"
#include "LangIDs.h"
#include "UPnPImplWrapper.h"
#include "VisualStylesXP.h"
#include "Addons/Modname/Modname.h" //>>> WiZaRd::Easy ModVersion
#include "Addons/GDIPlusUtil/GDIPlusUtil.h" // X: [GPUI] - [GDI Plus UI]
#include "Addons/SpeedGraph/SpeedGraphWnd.h" // X: [SGW] - [SpeedGraphWnd]
#ifdef CLIENTANALYZER
#include "Addons/AntiLeech/ClientAnalyzer.h" //>>> WiZaRd::ClientAnalyzer
#endif
#include "Addons/SplashScreenEx/SplashScreenEx.h" //ZZUL-TRA :: NewSplashscreen
// ZZUL-TRA :: AutoSharedFilesUpdater :: Start
#include "SharedFilesWnd.h"
CEvent* CemuleApp::m_directoryWatcherCloseEvent;
CEvent* CemuleApp::m_directoryWatcherReloadEvent;
CCriticalSection CemuleApp::m_directoryWatcherCS;
// ZZUL-TRA :: AutoSharedFilesUpdater :: End
#include "Addons/DLPUserNickCheck/DLP.h" // ZZUL-TRA :: DLP_OwnNickCheck
#include "Addons/SysInfo/SysInfo.h" // ZZUL-TRA :: SysInfo

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#if _MSC_VER>=1400 && defined(_UNICODE)
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

CLogFile theLog;
CLogFile theVerboseLog;
bool g_bLowColorDesktop = false;
//bool g_bGdiPlusInstalled = false; // X: [GPUI] - [GDI Plus UI]

//#define USE_16COLOR_ICONS


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
static TCHAR s_szCrtDebugReportFilePath[MAX_PATH] = APP_CRT_DEBUG_LOG_FILE;
#endif //_DEBUG


///////////////////////////////////////////////////////////////////////////////
// SafeSEH - Safe Exception Handlers
//
// This security feature must be enabled at compile time, due to using the
// linker command line option "/SafeSEH". Depending on the used libraries and
// object files which are used to link eMule.exe, the linker may or may not
// throw some errors about 'safeseh'. Those errors have to get resolved until
// the linker is capable of linking eMule.exe *with* "/SafeSEH".
//
// At runtime, we just can check if the linker created an according SafeSEH
// exception table in the '__safe_se_handler_table' object. If SafeSEH was not
// specified at all during link time, the address of '__safe_se_handler_table'
// is NULL -> hence, no SafeSEH is enabled.
///////////////////////////////////////////////////////////////////////////////
extern "C" PVOID __safe_se_handler_table[];
extern "C" BYTE  __safe_se_handler_count;

void InitSafeSEH()
{
	// Need to workaround the optimizer of the C-compiler...
	volatile PVOID safe_se_handler_table = __safe_se_handler_table;
	if (safe_se_handler_table == NULL)
	{
		AfxMessageBox(_T("eMule.exe was not linked with /SafeSEH!"), MB_ICONSTOP);
	}
}


///////////////////////////////////////////////////////////////////////////////
// DEP - Data Execution Prevention
// 
// For Windows XP SP2 and later. Does *not* have any performance impact!
//
// VS2003:	DEP must be enabled dynamically because the linker does not support 
//			the "/NXCOMPAT" command line option.
// VS2005:	DEP can get enabled at link time by using the "/NXCOMPAT" command 
//			line option.
// VS2008:	DEP can get enabled at link time by using the "DEP" option within
//			'Visual Studio Linker Advanced Options'.
//
#ifndef PROCESS_DEP_ENABLE
#define	PROCESS_DEP_ENABLE						0x00000001
#define	PROCESS_DEP_DISABLE_ATL_THUNK_EMULATION	0x00000002
BOOL WINAPI GetProcessDEPPolicy(HANDLE hProcess, LPDWORD lpFlags, PBOOL lpPermanent);
BOOL WINAPI SetProcessDEPPolicy(DWORD dwFlags);
#endif//!PROCESS_DEP_ENABLE

void InitDEP()
{
	BOOL (WINAPI *pfnGetProcessDEPPolicy)(HANDLE hProcess, LPDWORD lpFlags, PBOOL lpPermanent);
	BOOL (WINAPI *pfnSetProcessDEPPolicy)(DWORD dwFlags);
	(FARPROC&)pfnGetProcessDEPPolicy = GetProcAddress(GetModuleHandle(_T("kernel32")), "GetProcessDEPPolicy");
	(FARPROC&)pfnSetProcessDEPPolicy = GetProcAddress(GetModuleHandle(_T("kernel32")), "SetProcessDEPPolicy");
	if (pfnGetProcessDEPPolicy && pfnSetProcessDEPPolicy)
	{
		DWORD dwFlags;
		BOOL bPermanent;
		if ((*pfnGetProcessDEPPolicy)(GetCurrentProcess(), &dwFlags, &bPermanent))
		{
			// Vista SP1
			// ===============================================================
			//
			// BOOT.INI nx=OptIn,  VS2003/VS2005
			// ---------------------------------
			// DEP flags: 00000000
			// Permanent: 0
			//
			// BOOT.INI nx=OptOut, VS2003/VS2005
			// ---------------------------------
			// DEP flags: 00000001 (PROCESS_DEP_ENABLE)
			// Permanent: 0
			//
			// BOOT.INI nx=OptIn/OptOut, VS2003 + EditBinX/NXCOMPAT
			// ----------------------------------------------------
			// DEP flags: 00000003 (PROCESS_DEP_ENABLE | *PROCESS_DEP_DISABLE_ATL_THUNK_EMULATION*)
			// Permanent: *1*
			// ---
			// There is no way to remove the PROCESS_DEP_DISABLE_ATL_THUNK_EMULATION flag at runtime,
			// because the DEP policy is already permanent due to the NXCOMPAT flag.
			//
			// BOOT.INI nx=OptIn/OptOut, VS2005 + /NXCOMPAT
			// --------------------------------------------
			// DEP flags: 00000003 (PROCESS_DEP_ENABLE | PROCESS_DEP_DISABLE_ATL_THUNK_EMULATION)
			// Permanent: *1*
			//
			// NOTE: It is ultimately important to explicitly enable the DEP policy even if the
			// process' DEP policy is already enabled. If the DEP policy is already enabled due
			// to an OptOut system policy, the DEP policy is though not yet permanent. As long as
			// the DEP policy is not permanent it could get changed during runtime...
			//
			// So, if the DEP policy for the current process is already enabled but not permanent,
			// it has to be explicitly enabled by calling 'SetProcessDEPPolicy' to make it permanent.
			//
			if (  ((dwFlags & PROCESS_DEP_ENABLE) == 0 || !bPermanent)
#if _ATL_VER>0x0710
				|| (dwFlags & PROCESS_DEP_DISABLE_ATL_THUNK_EMULATION) == 0
#endif
			   )
			{
				// VS2003:	Enable DEP (with ATL-thunk emulation) if not already set by system policy
				//			or if the policy is not yet permanent.
				//
				// VS2005:	Enable DEP (without ATL-thunk emulation) if not already set by system policy 
				//			or linker "/NXCOMPAT" option or if the policy is not yet permanent. We should
				//			not reach this code path at all because the "/NXCOMPAT" option is specified.
				//			However, the code path is here for safety reasons.
				dwFlags = PROCESS_DEP_ENABLE;
#if _ATL_VER>0x0710
				// VS2005: Disable ATL-thunks.
				dwFlags |= PROCESS_DEP_DISABLE_ATL_THUNK_EMULATION;
#endif
				(*pfnSetProcessDEPPolicy)(dwFlags);
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
// Heap Corruption Detection
//
// For Windows Vista and later. Does *not* have any performance impact!
// 
#ifndef HeapEnableTerminationOnCorruption
#define HeapEnableTerminationOnCorruption (HEAP_INFORMATION_CLASS)1
WINBASEAPI BOOL WINAPI HeapSetInformation(HANDLE HeapHandle, HEAP_INFORMATION_CLASS HeapInformationClass, PVOID HeapInformation, SIZE_T HeapInformationLength);
#endif//!HeapEnableTerminationOnCorruption

void InitHeapCorruptionDetection()
{
	BOOL (WINAPI *pfnHeapSetInformation)(HANDLE HeapHandle, HEAP_INFORMATION_CLASS HeapInformationClass, PVOID HeapInformation, SIZE_T HeapInformationLength);
	(FARPROC&)pfnHeapSetInformation = GetProcAddress(GetModuleHandle(_T("kernel32")), "HeapSetInformation");
	if (pfnHeapSetInformation)
	{
		(*pfnHeapSetInformation)(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
	}
}


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
END_MESSAGE_MAP()

CemuleApp::CemuleApp(LPCTSTR lpszAppName)
	:CWinApp(lpszAppName)
{
	// Initialize Windows security features.
#ifndef _DEBUG
	InitSafeSEH();
#endif
	InitDEP();
	InitHeapCorruptionDetection();

	// This does not seem to work well with multithreading, although there is no reason why it should not.
	//_set_sbh_threshold(768);

	srand(time(NULL));
	m_dwPublicIP = 0;
	m_bAutoStart = false;

	// X: [GPUI] - [GDI Plus UI]
	/*// NOTE: Do *NOT* forget to specify /DELAYLOAD:gdiplus.dll as link parameter.
	HMODULE hLib = LoadLibrary(_T("gdiplus.dll"));
	if (hLib != NULL) {
		g_bGdiPlusInstalled = GetProcAddress(hLib, "GdiplusStartup") != NULL;
		FreeLibrary(hLib);
	}*/
	m_ullComCtrlVer = MAKEDLLVERULL(4,0,0,0);
	m_hSystemImageList = NULL;
	m_sizSmallSystemIcon.cx = 16;
	m_sizSmallSystemIcon.cy = 16;
	m_hBigSystemImageList = NULL;
	m_sizBigSystemIcon.cx = 32;
	m_sizBigSystemIcon.cy = 32;
	m_iDfltImageListColorFlags = ILC_COLOR;

	m_pSplashWnd = NULL; //ZZUL-TRA :: NewSplashscreen
	m_bRestartApp = false; // ZZUL-TRA :: AutoRestartIfNecessary

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
	m_strCurVersionLong += _T(" BETA3");
#endif

	m_strCurVersionLong.AppendFormat(_T("  %s"), MOD_VERSION); //>>> WiZaRd::Easy ModVersion

	// create the protocol version number
	CString strTmp;
	strTmp.Format(_T("0x%u"), m_dwProductVersionMS);
	VERIFY( _stscanf(strTmp, _T("0x%x"), &m_uCurVersionShort) == 1 );
	ASSERT( m_uCurVersionShort < 0x99 );
	m_pSpeedGraphWnd = NULL; // X: [SGW] - [SpeedGraphWnd]

	// create the version check number
	strTmp.Format(_T("0x%u%c"), m_dwProductVersionMS, _T('A') + CemuleApp::m_nVersionUpd);
	VERIFY( _stscanf(strTmp, _T("0x%x"), &m_uCurVersionCheck) == 1 );
	ASSERT( m_uCurVersionCheck < 0x999 );
// MOD Note: end

	m_bGuardClipboardPrompt = false;

	m_dwReAskTick = 0; // ZZUL-TRA :: AutoDropImmunity

	// ZZUL-TRA :: ReAskSrcAfterIPChange :: Start
	m_uLastValidID[0] = 0;
	m_uLastValidID[1] = 0;
	m_uLastValidID[2] = 0;
	m_dwLastIpCheckDetected = ::GetTickCount(); // do not trigger after startup!
	// ZZUL-TRA :: ReAskSrcAfterIPChange :: End
}


CemuleApp theApp(_T("eMule"));


// Workaround for bugged 'AfxSocketTerm' (needed at least for MFC 7.0, 7.1, 8.0, 9.0)
//MORPH START - Changed by Stulle, Visual Studio 2010 Compatibility
/*
#if _MFC_VER==0x0700 || _MFC_VER==0x0710 || _MFC_VER==0x0800 || _MFC_VER==0x0900
*/
#if _MFC_VER==0x0700 || _MFC_VER==0x0710 || _MFC_VER==0x0800 || _MFC_VER==0x0900 || _MFC_VER==0x0A00
//MORPH END   - Changed by Stulle, Visual Studio 2010 Compatibility
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

// ZZUL-TRA :: Winsock2 :: Start
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
// ZZUL-TRA :: Winsock2 :: End

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
#ifndef _BETA
	if (GetProfileInt(_T("eMule"), _T("CreateCrashDump"), 0))
#endif
		theCrashDumper.Enable(_T("eMule ") + m_strCurVersionLongDbg, true, thePrefs.GetMuleDirectory(EMULE_CONFIGDIR));

	///////////////////////////////////////////////////////////////////////////
	// Locale initialization -- BE VERY CAREFUL HERE!!!
	//
	_tsetlocale(LC_ALL, _T(""));		// set all categories of locale to user-default ANSI code page obtained from the OS.
	_tsetlocale(LC_NUMERIC, _T("C"));	// set numeric category to 'C'
	//_tsetlocale(LC_CTYPE, _T("C"));		// set character types category to 'C' (VERY IMPORTANT, we need binary string compares!)

	AfxOleInit();

	pstrPendingLink = NULL;
	if (ProcessCommandline())
		return false;

	///////////////////////////////////////////////////////////////////////////
	// Common Controls initialization
	//
	//						Mjr Min
	// ----------------------------
	// W98 SE, IE5			5	8
	// W2K SP4, IE6 SP1		5	81
	// XP SP2 				6   0
	// XP SP3				6   0
	// Vista SP1			6   16
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

	///////////////////////////////////////////////////////////////////////////
	// Shell32 initialization
	//
	//						Mjr Min
	// ----------------------------
	// W98 SE, IE5			4	72
	// W2K SP4, IE6 SP1		5	0
	// XP SP2 				6   0
	// Vista SP1			6   0
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

	g_gdiplus.Init(); // X: [GPUI] - [GDI Plus UI]

	m_sizSmallSystemIcon.cx = GetSystemMetrics(SM_CXSMICON);
	m_sizSmallSystemIcon.cy = GetSystemMetrics(SM_CYSMICON);
	UpdateLargeIconSize();
	UpdateDesktopColorDepth();

	CWinApp::InitInstance();

	// ZZUL-TRA :: Winsock2 :: Start
	/*
	if (!AfxSocketInit())
	{
		AfxMessageBox(GetResString(IDS_SOCKETS_INIT_FAILED));
		return FALSE;
	}
	*/
	m_bWinSock2 = false;
	memset(&m_wsaData,0,sizeof(WSADATA));
	if (!InitWinsock2(&m_wsaData)){ // first try it with winsock2
		memset(&m_wsaData,0,sizeof(WSADATA));
		if (!AfxSocketInit(&m_wsaData)){ // then try it with old winsock
			AfxMessageBox(GetResString(IDS_SOCKETS_INIT_FAILED));
			return FALSE;
		}
	} else
		m_bWinSock2 = true;
	// ZZUL-TRA :: Winsock2 :: End
//MORPH START - Changed by Stulle, Visual Studio 2010 Compatibility
/*
#if _MFC_VER==0x0700 || _MFC_VER==0x0710 || _MFC_VER==0x0800 || _MFC_VER==0x0900
*/
#if _MFC_VER==0x0700 || _MFC_VER==0x0710 || _MFC_VER==0x0800 || _MFC_VER==0x0900 || _MFC_VER==0x0A00
//MORPH END   - Changed by Stulle, Visual Studio 2010 Compatibility
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

	// create & initalize all the important stuff 
	thePrefs.Init();
	theStats.Init();

	//ZZUL-TRA :: NewSplashscreen :: Start
	m_dwSplashTime = (DWORD)-1;
        {
		//use temporary variables to determine if we should show the splash
		bool bStartMinimized = thePrefs.GetStartMinimized();
		if (!bStartMinimized)
			bStartMinimized = theApp.DidWeAutoStart();
		
		// temporary disable the 'startup minimized' option, otherwise no window will be shown at all
		if (thePrefs.IsFirstStart())
			bStartMinimized = false;

		// show splashscreen as early as possible to "entertain" user while starting emule up
	if (thePrefs.UseSplashScreen() && !bStartMinimized)
       {
		//Xman final version: don't show splash on old windows->crash
			switch (thePrefs.GetWindowsVersion())
			{
				case _WINVER_98_:
				case _WINVER_95_:	
				case _WINVER_ME_:
					break;
				default:
			ShowSplash(true);
			}
		}
	}
        //ZZUL-TRA :: NewSplashscreen :: End

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

#ifdef _DEBUG
	_sntprintf(s_szCrtDebugReportFilePath, _countof(s_szCrtDebugReportFilePath) - 1, _T("%s%s"), thePrefs.GetMuleDirectory(EMULE_LOGDIR, false), APP_CRT_DEBUG_LOG_FILE);
#endif
	VERIFY( theLog.SetFilePath(thePrefs.GetMuleDirectory(EMULE_LOGDIR, thePrefs.GetLog2Disk()) + _T("eMule.log")) );
	VERIFY( theVerboseLog.SetFilePath(thePrefs.GetMuleDirectory(EMULE_LOGDIR, false) + _T("eMule_Verbose.log")) );
	theLog.SetMaxFileSize(thePrefs.GetMaxLogFileSize());
	theLog.SetFileFormat(thePrefs.GetLogFileFormat());
	theVerboseLog.SetMaxFileSize(thePrefs.GetMaxLogFileSize());
	theVerboseLog.SetFileFormat(thePrefs.GetLogFileFormat());
	if (thePrefs.GetLog2Disk()){
		theLog.Open();
		theLog.Log(_T("\r\n"));
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
    if(thePrefs.GetHighresTimer()) {
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

	// ZZ:UploadSpeedSense -->
    lastCommonRouteFinder = new LastCommonRouteFinder();
    uploadBandwidthThrottler = new UploadBandwidthThrottler();
	// ZZ:UploadSpeedSense <--
	dlp = new CDLP(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR)); // ZZUL-TRA :: DLP_OwnNickCheck
	clientlist = new CClientList();
	friendlist = new CFriendList();
	searchlist = new CSearchList();
	knownfiles = new CKnownFileList();
	serverlist = new CServerList();
	serverconnect = new CServerConnect();
	sharedfiles = new CSharedFileList(serverconnect);
	listensocket = new CListenSocket();
	clientudp	= new CClientUDPSocket();
	UpdateSplash(_T("Loading Credits ...")); //ZZUL-TRA :: NewSplashscreen
	clientcredits = new CClientCreditsList();
#ifdef CLIENTANALYZER
	antileechlist   = new CAntiLeechDataList(); //>>> WiZaRd::ClientAnalyzer
#endif
	pSysInfo = new CSysInfo(); // ZZUL-TRA :: SysInfo
	downloadqueue = new CDownloadQueue();	// bugfix - do this before creating the uploadqueue
	uploadqueue = new CUploadQueue();
	UpdateSplash(_T("Loading IP-Filter ...")); //ZZUL-TRA :: NewSplashscreen
	ipfilter 	= new CIPFilter();
	webserver = new CWebServer(); // Webserver [kuchin]
	scheduler = new CScheduler();
	//m_pPeerCache = new CPeerCacheFinder(); // X: [RPC] - [Remove PeerCache]
	ip2country = new CIP2Country(); // ZZUL-TRA :: IP2Country
	// ZZUL-TRA :: AutoSharedFilesUpdater :: Start
	m_directoryWatcherCloseEvent = NULL;
	m_directoryWatcherReloadEvent = NULL;
	// ZZUL-TRA :: AutoSharedFilesUpdater :: End
	
	if(g_gdiplus.IsInited()){ // X: [GPUI] - [GDI Plus UI]
		m_pSpeedGraphWnd = new CSpeedGraphWnd(); // X: [SGW] - [SpeedGraphWnd]
		m_pSpeedGraphWnd->EnableToolTips(true);
		m_pSpeedGraphWnd->OpenDialog(SW_HIDE);
	}

	thePerfLog.Startup();

	UpdateSplash(_T("Initializing Main-Window ...")); //ZZUL-TRA :: NewSplashscreen
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

	DestroySplash(); //ZZUL-TRA :: NewSplashscreen

// ZZUL-TRA :: AutoRestartIfNecessary :: Start
	if(m_bRestartApp){
		CString strExePath;
		strExePath.Format(L"%s%s.exe", thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), m_pszExeName);
		ShellOpenFile(strExePath);
	}
	// ZZUL-TRA :: AutoRestartIfNecessary :: End

	AddDebugLogLine(DLP_VERYLOW, _T("%hs: returning: FALSE"), __FUNCTION__);

	return FALSE;
}

int CemuleApp::ExitInstance()
{
	AddDebugLogLine(DLP_VERYLOW, _T("%hs"), __FUNCTION__);

	if (m_wTimerRes != 0) {
        timeEndPeriod(m_wTimerRes);
    }

	return CWinApp::ExitInstance();
}

#ifdef _DEBUG
int CrtDebugReportCB(int reportType, char* message, int* returnValue)
{
	FILE* fp = _tfsopen(s_szCrtDebugReportFilePath, _T("a"), _SH_DENYWR);
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

	//this codepart is to determine special cases when we do add a link to our eMule
	//because in this case it would be nonsense to start another instance!
	if(bIgnoreRunningInstances)
	{       
		if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen
			&& (cmdInfo.m_strFileName.Find(_T("://")) > 0
			|| CCollection::HasCollectionExtention(cmdInfo.m_strFileName)) )
			bIgnoreRunningInstances = false;
	}
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

void CemuleApp::UpdateSentBytes(uint32 bytesToAdd, bool sentToFriend) {
	SetTimeOnTransfer();
	theStats.sessionSentBytes+=bytesToAdd;

    if(sentToFriend == true) {
	    theStats.sessionSentBytesToFriend += bytesToAdd;
    }
}

void CemuleApp::SetTimeOnTransfer() {
	if (theStats.transferStarttime>0) return;
	
	theStats.transferStarttime=GetTickCount();
}

CString CemuleApp::CreateKadSourceLink(const CAbstractFile* f)
{
	CString strLink;
	if( Kademlia::CKademlia::IsConnected() && theApp.clientlist->GetBuddy() && theApp.IsFirewalled() )
	{
		CString KadID;
		Kademlia::CKademlia::GetPrefs()->GetKadID().Xor(Kademlia::CUInt128(true)).ToHexString(&KadID);
		strLink.Format(_T("ed2k://|file|%s|%I64u|%s|/|kadsources,%s:%s|/"),
			EncodeUrlUtf8(StripInvalidFilenameChars(f->GetFileName())),
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

		_snprintf(buffer, _countof(buffer), "%.1f", (float)downloadqueue->GetDatarate() / 1024);
		buffer[_countof(buffer) - 1] = '\0';
		file.Write(buffer, strlen(buffer)); 
		file.Write("|", 1);

		_snprintf(buffer, _countof(buffer), "%.1f", (float)uploadqueue->GetDatarate() / 1024); 
		buffer[_countof(buffer) - 1] = '\0';
		file.Write(buffer, strlen(buffer));
		file.Write("|", 1);

		_itoa(uploadqueue->GetWaitingUserCount(), buffer, 10);
		file.Write(buffer, strlen(buffer));

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

int CemuleApp::GetFileTypeSystemImageIdx(LPCTSTR pszFilePath, int iLength /* = -1 */, bool bNormalsSize)
{
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
			// Get index for the system's big icon image list
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
	return (int)vData;
}

bool CemuleApp::IsConnected(bool bIgnoreEd2k, bool bIgnoreKad)
{
	return ( (theApp.serverconnect->IsConnected() && !bIgnoreEd2k) || (Kademlia::CKademlia::IsConnected() && !bIgnoreKad));
}

bool CemuleApp::IsPortchangeAllowed() {
	return ( theApp.clientlist->GetClientCount()==0 && !IsConnected() );
}

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
	if (m_dwPublicIP == 0 && Kademlia::CKademlia::IsConnected() && Kademlia::CKademlia::GetIPAddress() && !bIgnoreKadIP)
		return ntohl(Kademlia::CKademlia::GetIPAddress());
	return m_dwPublicIP;
}

void CemuleApp::SetPublicIP(const uint32 dwIP){
	if (dwIP != 0){
		ASSERT ( !IsLowID(dwIP));
		//ASSERT ( m_pPeerCache ); // X: [RPC] - [Remove PeerCache]

		if ( GetPublicIP() == 0)
			AddDebugLogLine(DLP_VERYLOW, false, _T("My public IP Address is: %s"),ipstr(dwIP));
		else if (Kademlia::CKademlia::IsConnected() && Kademlia::CKademlia::GetPrefs()->GetIPAddress())
			if(ntohl(Kademlia::CKademlia::GetIPAddress()) != dwIP)
				AddDebugLogLine(DLP_DEFAULT, false,  _T("Public IP Address reported from Kademlia (%s) differs from new found (%s)"),ipstr(ntohl(Kademlia::CKademlia::GetIPAddress())),ipstr(dwIP));
		//m_pPeerCache->FoundMyPublicIPAddress(dwIP);	 // X: [RPC] - [Remove PeerCache]
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
	if(Kademlia::CKademlia::IsConnected())
	{
		if(theApp.serverconnect->IsConnected())
		{
			if(theApp.serverconnect->IsLowID())
			{
				if(Kademlia::CKademlia::IsFirewalled())
				{
					//Both Connected - Both Firewalled
					return false;
				}
				else
				{
					if(client->GetServerIP() == theApp.serverconnect->GetCurrentServer()->GetIP() && client->GetServerPort() == theApp.serverconnect->GetCurrentServer()->GetPort())
					{
						//Both Connected - Server lowID, Kad Open - Client on same server
						//We prevent a callback to the server as this breaks the protocol and will get you banned.
						return false;
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
				//Only Kad Connected - Kad Firewalled
				return false;
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
				//Only Server Connected - Server LowID
				return false;
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
					static UINT (WINAPI *_pfnPrivateExtractIcons)(LPCTSTR, int, int, int, HICON*, UINT*, UINT, UINT) 
						= (UINT (WINAPI *)(LPCTSTR, int, int, int, HICON*, UINT*, UINT, UINT))GetProcAddress(GetModuleHandle(_T("user32")), _TWINAPI("PrivateExtractIcons"));
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
				// scheme 'Windows-Standard (extragro�)' -> always try to use 'LoadImage'!
				//
				// If the ICO file contains a 16x16 icon, 'LoadImage' will though return a 32x32 icon,
				// if LR_DEFAULTSIZE is specified! -> always specify the requested size!
				hIcon = (HICON)::LoadImage(NULL, szFullResPath, IMAGE_ICON, cx, cy, uFlags | LR_LOADFROMFILE);
				if (hIcon == NULL && GetLastError() != ERROR_PATH_NOT_FOUND && g_gdiplus.IsInited()) // X: [GPUI] - [GDI Plus UI]
				{
					/*ULONG_PTR gdiplusToken = 0;
					Gdiplus::GdiplusStartupInput gdiplusStartupInput;
					if (Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) == Gdiplus::Ok)
					{*/
						Gdiplus::Bitmap bmp(szFullResPath);
						bmp.GetHICON(&hIcon);
					//}
					//Gdiplus::GdiplusShutdown(gdiplusToken);
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

//Xman [MoNKi: -Check already downloaded files-]
/*
void CemuleApp::AddEd2kLinksToDownload(CString strLinks, int cat)
*/
void CemuleApp::AddEd2kLinksToDownload(CString strLinks, int cat, bool askIfAlreadyDownloaded)
//Xman end
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
					//Xman [MoNKi: -Check already downloaded files-]
					if ( askIfAlreadyDownloaded )
					{
						if ( knownfiles->CheckAlreadyDownloadedFileQuestion(pLink->GetFileLink()->GetHashKey(), pLink->GetFileLink()->GetName()) )
					downloadqueue->AddFileLinkToDownload(pLink->GetFileLink(), cat);
				}
				else
						theApp.downloadqueue->AddFileLinkToDownload(pLink->GetFileLink(), cat);
					//Xman end
				}
				else
				{
					delete pLink;
					throw CString(_T("bad link"));
				}
				delete pLink;
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
			//Xman [MoNKi: -Check already downloaded files-]
			/*
			AddEd2kLinksToDownload(pszTrimmedLinks, 0);
			*/
			AddEd2kLinksToDownload(pszTrimmedLinks, 0, true);
			//Xman end
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
	//Xman [MoNKi: -Check already downloaded files-]
	/*
	AddEd2kLinksToDownload(strLinks, cat);
	*/
	AddEd2kLinksToDownload(strLinks, cat, true);
	//Xman end
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
		if (m_iDfltImageListColorFlags == ILC_COLOR32 && m_ullComCtrlVer < MAKEDLLVERULL(6,0,0,0))
		{
			// We fall back to 16-bit image lists because we do not provide 24-bit
			// versions of icons any longer (due to resource size restrictions for Win98). We
			// could also fall back to 24-bit image lists here but the difference is minimal
			// and considered not to be worth the additinoal memory consumption.
			//
			// Though, do not fall back to 8-bit image lists because this would let Windows
			// reduce the color resolution to the standard 256 color window system palette.
			// We need a 16-bit or 24-bit image list to hold all our 256 color icons (which
			// are not pre-quantized to standard 256 color windows system palette) without
			// loosing any colors.
			m_iDfltImageListColorFlags = ILC_COLOR16;
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

bool CemuleApp::IsXPThemeActive() const
{
	// TRUE: If an XP style (and only an XP style) is active
	return theApp.m_ullComCtrlVer < MAKEDLLVERULL(6,16,0,0) && g_xpStyle.IsThemeActive() && g_xpStyle.IsAppThemed();
}

bool CemuleApp::IsVistaThemeActive() const
{
	// TRUE: If a Vista (or better) style is active
	return theApp.m_ullComCtrlVer >= MAKEDLLVERULL(6,16,0,0) && g_xpStyle.IsThemeActive() && g_xpStyle.IsAppThemed();
}

// ZZUL-TRA :: ReAskSrcAfterIPChange :: Start
void CemuleApp::CheckIdChange()
{
	DWORD dwCurTick = ::GetTickCount();

	uint32 uTempID[2] = {0,0};
	if(serverconnect->IsConnected())
		uTempID[0] = serverconnect->GetClientID();
	    uTempID[1] = GetPublicIP();

	for(int i=0; i<2; i++){
		if(m_uLastValidID[i] != 0 && uTempID[i] != 0 && m_uLastValidID[i] != uTempID[i] && dwCurTick >= (m_dwLastIpCheckDetected + MIN2MS(15))){ // no triggering for 15 minutes!
			// Public IP has been changed, it's necessary to inform all sources about it
			// All sources would be informed during their next session refresh (with TCP)
			// about the new IP.
			clientlist->TrigReaskForDownload();

			m_dwLastIpCheckDetected = dwCurTick;

			Log(LOG_WARNING, _T("Change from %u (%s ID) to %u (%s ID) detected!"), 
				m_uLastValidID[i],
				(m_uLastValidID[i] < 16777216) ? _T("low") : _T("high"),
				uTempID[i],
				(uTempID[i] < 16777216)  ? _T("low") : _T("high"));

			Log(LOG_WARNING, _T("All Sources will be reasked immediately!"));
			Log(LOG_WARNING, _T("Detected via %s"),
				(i == 0)?_T("Server Connection"):_T("Kad Connection"));
		}
		if(uTempID[i] != 0 && uTempID[i] != m_uLastValidID[i])
			m_uLastValidID[i] = uTempID[i];

		SetReAskTick(dwCurTick + MIN2MS(5)); // ZZUL-TRA :: AutoDropImmunity - 5 minutes drop immunity //morph4u 20->5
	}
	return;
}
// ZZUL-TRA :: ReAskSrcAfterIPChange :: End

//ZZUL-TRA :: NewSplashscreen :: Start
void CemuleApp::ShowSplash(bool start)
{
	ASSERT( m_pSplashWnd == NULL );
	if (g_gdiplus.IsInited() && m_pSplashWnd == NULL)
	{
		m_pSplashWnd = new CSplashScreenEx();
		if (m_pSplashWnd != NULL)
		{

			if (m_pSplashWnd->Create(NULL,CSS_FADE | CSS_CENTERSCREEN | CSS_SHADOW | CSS_HIDEONCLICK))
			{
				m_pSplashWnd->Show(start);
				splashscreenfinished=false;

				UpdateSplash(start?_T("Loading ..."):_T("Shutting down ..."));
			}
			else
			{
				delete m_pSplashWnd;
				m_pSplashWnd = NULL;
				splashscreenfinished=true;
			}
		}
	}
}

void CemuleApp::DestroySplash()
{
	if (m_pSplashWnd != NULL)
	{
		m_pSplashWnd->Hide();
		m_pSplashWnd->DestroyWindow();
		delete m_pSplashWnd;
		m_pSplashWnd = NULL;
	}
	splashscreenfinished=true;
}

void CemuleApp::SplashHide(int hide)
{
	if (m_pSplashWnd != NULL){
		m_pSplashWnd->ShowWindow(hide);
		if(hide != SW_HIDE)
			m_pSplashWnd->RedrawWindow();
	}
}

void CemuleApp::UpdateSplash(LPCTSTR Text){
	if(m_pSplashWnd)
		m_pSplashWnd->SetText(Text);
}

void CemuleApp::UpdateSplash2(LPCTSTR Text){
	if(m_pSplashWnd)
		m_pSplashWnd->SetText2(Text);
}
//ZZUL-TRA :: NewSplashscreen :: End

// ZZUL-TRA :: AutoSharedFilesUpdater :: Start
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } } 

// This thread will check if the user made changes on shared
// directories, reloading it if size/name/date/attributes has
// changed on files/directories inside a shared dir or on the
// shared dir itself.
UINT CemuleApp::CheckDirectoryForChangesThread(LPVOID /*pParam*/)
{
	m_directoryWatcherCS.Lock();

	DWORD lastReloadTime = ::GetTickCount()-SEC2MS(10); //Forces first reload
	DWORD reloadSleepTime = 5;

	// Sets the minimum time between reloads.
	// To set an fixed time between reloads change
	// minSecondsBetweenReloads to a value greater
	// than 0, for example 600 for 10 minutes (600 seconds)
	const DWORD minSecondsBetweenReloads = 0;
	// Tux NOTE: Of course we -could- make it changeable here.

	// We use this event when FindFirstChangeNotification fails
	CEvent nullEvent(FALSE,TRUE); 

	// We use a second list to store inactive shares. Note, dirs will only be added to this list
	// when they got inactive during runtime so we will not add vast numbers of always inactive
	// shares. This makes a reset required when such a always inactive dir comes available out
	// of a sudden. Anyway, there should not be too many of those to boot with.
	CStringList inactiveDirList;

	// Get all shared directories
	CStringList dirList;
	CString curDir;
	
	// Incoming Dir
	curDir=thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
	if (curDir.Right(1)==_T("\\"))
		curDir = curDir.Left(curDir.GetLength() - 1);

	dirList.AddTail( curDir );

	// Categories dirs
	for (int i=1; i < thePrefs.GetCatCount(); i++)
	{
		curDir=CString( thePrefs.GetCatPath(i) );
		if (curDir.Right(1)==_T("\\"))
			curDir = curDir.Left(curDir.GetLength() - 1);

		if (dirList.Find(curDir) == NULL)
			dirList.AddTail(curDir);
	}

	// The other shared dirs
	POSITION pos = thePrefs.shareddir_list.GetHeadPosition();
	while (pos) {
		curDir = thePrefs.shareddir_list.GetNext(pos);
		
		// If this folder does not exist we do not need to watch this folder
		if (_taccess(curDir, 0) != 0)
			continue;
			
		if (curDir.Right(1)==_T("\\"))
			curDir = curDir.Left(curDir.GetLength() - 1);

		if (dirList.Find(curDir) == NULL)
			dirList.AddTail(curDir);
	}

	// Dirs of single shared files
	//if(thePrefs.GetSingleSharedDirWatcher()/* && theApp.sharedfiles->ProbablyHaveSingleSharedFiles()*/)
	//{
	for (POSITION pos2 = theApp.sharedfiles->m_liSingleSharedFiles.GetHeadPosition(); pos2 != NULL; theApp.sharedfiles->m_liSingleSharedFiles.GetNext(pos2))
	{
		curDir = theApp.sharedfiles->m_liSingleSharedFiles.GetAt(pos2);
		if (_taccess(curDir, 0) != 0)
			continue; // only add for this single shared file if it exists

		int length = curDir.ReverseFind(_T('\\'));
		if (length != -1) // should always be true... anyway, just in case...
			curDir = curDir.Left(length);

		if (dirList.Find(curDir) == NULL)
			dirList.AddTail(curDir);
	}
	//}

	// dirList now contains all shared dirs.
	// Now we get the parent dirs of shared dirs,
	// Why? To check if the user renames or removes a shared dir,
	// because FindFirstChangeNotification don't notifies this changes
	// on the own directory.
	
	// Save the start position of parents
	int parentsStartPosition = dirList.GetCount();
	int curPos = 0;
	
	pos = dirList.GetHeadPosition();
	while (pos && curPos != parentsStartPosition) {
		curDir = dirList.GetNext(pos); 
		curPos++;

		int findPos = curDir.ReverseFind(_T('\\'));
		if (findPos != -1)
			curDir = curDir.Left(findPos);

		if (dirList.Find(curDir) == NULL)
			dirList.AddTail(curDir);
	}

	int nChangeHandles = dirList.GetCount() + 2; // We have 2 additional events

	// v3.5: There is a limit to WaitForMultipleObjects which is MAXIMUM_WAIT_OBJECTS == 64.
	// To prevent ASFU from crashing eMule entirely we disable ASFU. Working around the limit
	// might be possible but it would involve multiple threads checking parts of the above
	// created dirList. Coding that is too much of a pain at this point so we stick to the
	// easier way.
	if(nChangeHandles > MAXIMUM_WAIT_OBJECTS)
	{
		AddLogLine(true, _T("ASFU: You are sharing too many folders for ASFU! Disabling!"));
		thePrefs.SetDirectoryWatcher(false);
		m_directoryWatcherCS.Unlock();
		return 1;
	}

	// Save the position of the first parent in the list
	POSITION parentListPos = dirList.FindIndex(parentsStartPosition);

	HANDLE *dwChangeHandles = NULL;
	dwChangeHandles = new HANDLE[nChangeHandles];
	
	if (!m_directoryWatcherCloseEvent) {
		ASSERT(0);
		DebugLogError(_T("ASFU: Crashed :-((")); // it would crash ;)
		thePrefs.SetDirectoryWatcher(false);
	}
	else if (dwChangeHandles) {
		// dwChangeHandles[0] will be an event handler to finish the thread 
		dwChangeHandles[0] = m_directoryWatcherCloseEvent->m_hObject;

		// dwChangeHandles[1] will be an event handler to reload the files
		dwChangeHandles[1] = m_directoryWatcherReloadEvent->m_hObject;

		// Generate shared/parents directories handlers
		curPos = 2; // Start at pos 2, because the pos 0 and 1 for the events.
		pos = dirList.GetHeadPosition();
		while (pos) {
			curDir = dirList.GetNext(pos); 
			dwChangeHandles[curPos] = FindFirstChangeNotification(
				curDir, FALSE,
				FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
				FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE |
				FILE_NOTIFY_CHANGE_ATTRIBUTES);
			
			if (dwChangeHandles[curPos] == INVALID_HANDLE_VALUE)
				dwChangeHandles[curPos] = nullEvent.m_hObject;
			curPos++; 
		}
	
		// Waits for an event
		DWORD dwWaitStatus;
		DWORD dwWaitStatusClose;

		while (m_directoryWatcherCloseEvent /*true*/) { 
			dwWaitStatus = WaitForMultipleObjects(nChangeHandles, dwChangeHandles, FALSE, INFINITE); 
	 
			// figure out what got signaled
			if (dwWaitStatus - WAIT_OBJECT_0 >= 2)
			{
				// get the dir from the list
				pos = dirList.GetHeadPosition();
				for (DWORD dw = 2; dw <= dwWaitStatus - WAIT_OBJECT_0; dw++)
					curDir = dirList.GetNext(pos);

				if (_taccess(curDir, 0) != 0){ // this one disappeared just now... w000t
					if ( inactiveDirList.Find( curDir ) == NULL ) // well, it should not be in the list but to be sure
						inactiveDirList.AddTail( curDir ); // add to list of inactive shares
				}
			}
	 
			// Maybe more than one object has been released,
			// check if the Close Event has been signaled
			// because it has precedence.
			dwWaitStatusClose = WaitForSingleObject(m_directoryWatcherCloseEvent->m_hObject, 0);
			if (dwWaitStatusClose == WAIT_OBJECT_0) {
				// We want to finalize the thread
				dwWaitStatus = WAIT_OBJECT_0;
			}

			if (dwWaitStatus > WAIT_OBJECT_0 &&	dwWaitStatus < WAIT_OBJECT_0  + nChangeHandles) {
				bool reloadShared = true;

				if (dwWaitStatus > (WAIT_OBJECT_0 + 1) + parentsStartPosition) {
					// A parent of a shared dir has been modified.
					// Search changes in shared dirs and reload
					// it only if needed.
					reloadShared = false;

					// Note, this was changed in v3.4 and will run through all our monitored shares
					// so we can maintain a complete list of inactive shares. This list will then allow
					// us to reload share when a shared folder goes missing and when it pops up again.
					pos = dirList.GetHeadPosition();
					while(pos && pos != parentListPos) {
						curDir = dirList.GetNext(pos); 
						if (curDir.Right(1) != _T(":")) { // not a root dir
							if (_taccess(curDir, 0) != 0) { // does not exist
								if (inactiveDirList.Find(curDir) == NULL) { // and not yet in list
									inactiveDirList.AddTail(curDir); // add to list of inactive shares

								// Reload shared files
								reloadShared = true;
								}
							}
							else { // does exist
								POSITION inactivePos = inactiveDirList.Find(curDir);
								if (inactivePos != NULL) { // and in list of inactive shares
									inactiveDirList.RemoveAt(inactivePos); // remove from list

								// Reload shared files
								reloadShared = true;
								}
							}
						}
					}
				}

				if (reloadShared) {
					// Here we have a problem, if more than one file
					// has changed (for example when the user deletes or moves
					// a list of files), there will be a lot of notifications and
					// the shared file list will be reloaded multiple times.
					// The next code tries to minimize the number of reloads.

					// Stop all pending notifications
					// (we are going to reload ALL files, no more notifications needed)
					for (int i = 2; i < nChangeHandles; i++) {
						if (dwChangeHandles[i] != nullEvent.m_hObject)
							FindCloseChangeNotification(dwChangeHandles[i]);
					}

					// Wait a few seconds. Should be sufficient to skip
					// a lot of notifications generated by multiple files
					const DWORD curTime = ::GetTickCount();
					const DWORD ts = curTime - lastReloadTime;
					const uint32 seconds = ts/1000;
					
					if (minSecondsBetweenReloads == 0) {
						if (seconds < reloadSleepTime) {
							if (reloadSleepTime < 1280) //Max 21 minutes between reloads (aprox)
								reloadSleepTime *= 2;
						}
						else
							reloadSleepTime = 5;
					}
					else {
						if (seconds < minSecondsBetweenReloads)
							reloadSleepTime = minSecondsBetweenReloads - seconds;
						else
							reloadSleepTime = 5;
					}
						
					AddDebugLogLine(false, GetResString(IDS_ASFU_RELOADTIME), reloadSleepTime);
					
					// Waits reloadSleepTime seconds or until the close event is set
					dwWaitStatus = WaitForSingleObject(m_directoryWatcherCloseEvent->m_hObject, reloadSleepTime * 1000);

					// Checks if a part file is completing
					// and delay the reload in this case
					reloadShared = false;
					bool firstTime = true;
					while (!reloadShared){
						reloadShared = true;
						for (int i = 0; i < theApp.downloadqueue->GetFileCount() && reloadShared; i++) {
							CPartFile *pFile = theApp.downloadqueue->GetFileByIndex(i);
							if (pFile && pFile->GetStatus() == PS_COMPLETING) {
								if (firstTime) {
									AddDebugLogLine(false, GetResString(IDS_ASFU_DELAY));
									firstTime = false;
								}
								reloadShared = false;
							}
						}
						if (!reloadShared) {
							// Waits 10 seconds or until the close event is set
							dwWaitStatus = WaitForSingleObject(m_directoryWatcherCloseEvent->m_hObject, 10000);

							if (dwWaitStatus != WAIT_TIMEOUT) {
								// We want to close eMule
								// this forces the 'while' end
								reloadShared = true;
							}
						}
					}

					// Reload
					if (dwWaitStatus == WAIT_TIMEOUT) {
						//Restart all notifications again
						curPos = 2; // Start at pos 2, because the pos 0 and 1 is for the events.
						pos = dirList.GetHeadPosition();
						while (pos) {
							curDir = dirList.GetNext(pos); 
							
							dwChangeHandles[curPos] = FindFirstChangeNotification(
								curDir, FALSE,
								FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
								FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE |
								FILE_NOTIFY_CHANGE_ATTRIBUTES);
							
							if (dwChangeHandles[curPos] == INVALID_HANDLE_VALUE)
								dwChangeHandles[curPos] = nullEvent.m_hObject;
							curPos++;
						}
					
						// Reload shared files
						if (theApp.emuledlg->IsRunning()) {
							AddDebugLogLine(false, GetResString(IDS_ASFU_RELOADING));
							AddLogLine(false, GetResString(IDS_ASFU_RELOAD));

							m_directoryWatcherReloadEvent->ResetEvent();
//							theApp.sharedfiles->Reload();
//							theApp.emuledlg->sharedfileswnd->Reload();
							theApp.emuledlg->sharedfileswnd->SendMessage(WM_COMMAND, IDC_RELOADSHAREDFILES); //>>> WiZaRd::FiX! - Don't access the main thread from within a thread
							lastReloadTime = ::GetTickCount();
						}
					}
					else {
						delete[] dwChangeHandles;
						m_directoryWatcherCS.Unlock();
						return 1;
					}
				}
				else {
					// Get new changes
					FindNextChangeNotification(dwChangeHandles[dwWaitStatus - WAIT_OBJECT_0]);
				}
			}
			else {
				// End the thread
				for (int i = 2; i < nChangeHandles; i++) {
					if (dwChangeHandles[i] != nullEvent.m_hObject)
						FindCloseChangeNotification(dwChangeHandles[i]);
				}
				delete[] dwChangeHandles;
				m_directoryWatcherCloseEvent->ResetEvent();
				m_directoryWatcherCS.Unlock();
				return 1;
			}
		}
	}

	//This shouldn't execute never, but...
	m_directoryWatcherCS.Unlock();
	return 1;
}

void CemuleApp::ResetDirectoryWatcher(){
	// End previous thread (if exists)
	EndDirectoryWatcher();

	if(thePrefs.GetDirectoryWatcher()){

	if (m_directoryWatcherCloseEvent == NULL)
		m_directoryWatcherCloseEvent = new CEvent(FALSE, TRUE);

	if (m_directoryWatcherReloadEvent == NULL)
		m_directoryWatcherReloadEvent = new CEvent(FALSE, TRUE);

		if(m_directoryWatcherCloseEvent != NULL &&
			m_directoryWatcherReloadEvent != NULL)
		{
			// This is based on v3.2. v3.3 was never called this but
			// adding capabilities for shareSubdir is worth considering
			// the previous version as v3.3. New v3.4 addresses single
			// shared files and some gui handling around device changes.
			// v3.5 fixes crashes when too many dirs are shared. See 
			// above for a more detailed explanation.
			
			AddDebugLogLine(false, _T("ASFU: Starting v3.5"));

		// Starts new thread
		AfxBeginThread(CheckDirectoryForChangesThread, this);
	}
}
}

void CemuleApp::EndDirectoryWatcher()
{ 
      if (m_directoryWatcherCloseEvent != NULL) { 
           // Notifies the thread to finalize 
           m_directoryWatcherCloseEvent->SetEvent(); 
  
           // Waits until the thread ends 
           m_directoryWatcherCS.Lock(); 
           m_directoryWatcherCS.Unlock(); 
  
           SAFE_DELETE(m_directoryWatcherCloseEvent);
           AddDebugLogLine(false, _T("ASFU: Closed"));
      }

      SAFE_DELETE(m_directoryWatcherReloadEvent);
 }

void CemuleApp::DirectoryWatcherExternalReload()
{
	if (m_directoryWatcherCloseEvent != NULL &&	m_directoryWatcherReloadEvent != NULL) {
		AddDebugLogLine(false, _T("ASFU: Forcing reload"));

		// Notifies the thread to reload
		m_directoryWatcherReloadEvent->SetEvent();
	}
}
// ZZUL-TRA :: AutoSharedFilesUpdater :: End

// Commander - Added: FriendLinks [emulEspaa] - Start - added by zz_fly
bool CemuleApp::IsEd2kFriendLinkInClipboard()
{
	static const CHAR _szEd2kFriendLink[] = "ed2k://|friend|";
	return IsEd2kLinkInClipboard(_szEd2kFriendLink, ARRSIZE(_szEd2kFriendLink)-1);
}
// Commander - Added: FriendLinks [emulEspaa] - End