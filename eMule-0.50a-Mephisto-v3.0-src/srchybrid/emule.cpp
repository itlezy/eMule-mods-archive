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
//Xman
/*
#include "LastCommonRouteFinder.h"
*/
//Xman end
#include "UploadBandwidthThrottler.h"
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
// ==> UPnP support [MoNKi] - leuk_he
/*
#include "UPnPImplWrapper.h"
*/
// <== UPnP support [MoNKi] - leuk_he
#include "VisualStylesXP.h"

//Xman
#include "BandWidthControl.h" // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
#include "DLP.h" //Xman DLP
#include "P2PThreat.h" // netfinity: Detect worms that are harmful to P2P apps
//Xman new slpash-screen arrangement
#include "SplashScreenEx.h"

// ==> Automatic shared files updater [MoNKi] - Stulle
#include "SharedFilesWnd.h"
CEvent* CemuleApp::m_directoryWatcherCloseEvent;
CEvent* CemuleApp::m_directoryWatcherReloadEvent;
CCriticalSection CemuleApp::m_directoryWatcherCS;
// <== Automatic shared files updater [MoNKi] - Stulle
#include "NTService.h" // Run eMule as NT Service [leuk_he/Stulle] - Stulle

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
bool g_bGdiPlusInstalled = false;

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
	ON_COMMAND(ID_HELP, OnHelp)
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
	//Xman -Reask sources after IP change- v4
	m_bneedpublicIP = false; 
	last_ip_change = 0;
	last_valid_serverid = 0;
	last_valid_ip = 0;
	recheck_ip = 0;
	last_traffic_reception = 0;
	internetmaybedown=1;
	//Xman end
	m_bAutoStart = false;

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

	m_pSplashWnd = NULL; //Xman new slpash-screen arrangement

	//Xman dynamic IP-Filters
	ipdlgisopen=false;

	//Xman queued disc-access for read/flushing-threads
	//m_uRunningNonBlockedDiscAccessThreads=0;
	//Xman end

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
	//Xman
	/*
	m_strCurVersionLong += _T(" BETA2");
	*/
	m_strCurVersionLong.AppendFormat(_T(" a%u "),CemuleApp::m_nMVersionBld);
	m_strCurVersionLong += _T(__TIME__);
	m_strCurVersionLong += _T(" ");
	m_strCurVersionLong += _T(__DATE__);
	//Xman end
#endif

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

	// ==> ModID [itsonlyme/SiRoB] - Stulle
	m_strModVersion = CemuleApp::m_szMVersion;
	m_strModVersion.AppendFormat(_T(" %u.%u"), CemuleApp::m_nMVersionMjr, CemuleApp::m_nMVersionMin);
	m_strModLongVersion = CemuleApp::m_szMVersionLong;
	m_strModLongVersion.AppendFormat(_T("%u.%u"), CemuleApp::m_nMVersionMjr, CemuleApp::m_nMVersionMin);
	m_strModVersionPure = CemuleApp::m_szMVersion;
	m_strModVersionPure.AppendFormat(_T(" "));
	m_uModLength = (uint8)(m_strModVersionPure.GetLength()); // one space included!
	// <== ModID [itsonlyme/SiRoB] - Stulle
	//==> UPnP support [MoNKi] - leuk_he
	m_UPnP_IGDControlPoint = CUPnP_IGDControlPoint::GetInstance();
	//<== UPnP support [MoNKi] - leuk_he

}


CemuleApp theApp(_T("eMule"));


// Workaround for bugged 'AfxSocketTerm' (needed at least for MFC 7.0, 7.1, 8.0, 9.0)
// ==> Visual Studio 2010 Compatibility [Stulle/Avi-3k/ied] - Stulle
/*
#if _MFC_VER==0x0700 || _MFC_VER==0x0710 || _MFC_VER==0x0800 || _MFC_VER==0x0900
*/
#if _MFC_VER==0x0700 || _MFC_VER==0x0710 || _MFC_VER==0x0800 || _MFC_VER==0x0900 || _MFC_VER==0x0A00
// <== Visual Studio 2010 Compatibility [Stulle/Avi-3k/ied] - Stulle
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

//<<< eWombat [WINSOCK2] 
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
// >>> eWombat [WINSOCK2]

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
	// Xman better always enabled, except we don't want
	/*
	if (GetProfileInt(_T("eMule"), _T("CreateCrashDump"), 0))
	*/
	if (GetProfileInt(_T("eMule"), _T("NoCrashDump"), 0)==0)
	//Xman end
#endif
		//Xman ModId
		/*
		theCrashDumper.Enable(_T("eMule ") + m_strCurVersionLongDbg, true, thePrefs.GetMuleDirectory(EMULE_CONFIGDIR));
		*/
		// ==> ModID [itsonlyme/SiRoB] - Stulle
		/*
		theCrashDumper.Enable(_T("eMule ") + m_strCurVersionLongDbg + _T(" ") + MOD_VERSION, true, thePrefs.GetMuleDirectory(EMULE_CONFIGDIR));
		*/
		theCrashDumper.Enable(_T("eMule ") + m_strCurVersionLongDbg + _T(" ") + m_strModLongVersion, true, thePrefs.GetMuleDirectory(EMULE_CONFIGDIR));
		// <== ModID [itsonlyme/SiRoB] - Stulle
		//Xman end

	///////////////////////////////////////////////////////////////////////////
	// Locale initialization -- BE VERY CAREFUL HERE!!!
	//
	_tsetlocale(LC_ALL, _T(""));		// set all categories of locale to user-default ANSI code page obtained from the OS.
	_tsetlocale(LC_NUMERIC, _T("C"));	// set numeric category to 'C'
	//_tsetlocale(LC_CTYPE, _T("C"));		// set character types category to 'C' (VERY IMPORTANT, we need binary string compares!)

	AfxOleInit();

	//Xman
	// leuk_he: prevent switch to ... busy popup during windows startup. see kb 248019
	if (AfxOleGetMessageFilter()) {
		AfxOleGetMessageFilter()->EnableBusyDialog(false);
		AfxOleGetMessageFilter()->EnableNotRespondingDialog(false);
		AfxOleGetMessageFilter()->SetMessagePendingDelay(60 * 1000); // 60 secs instead of 5
	}
	else {
		ASSERT(0);  // dll not loaded?
	}
	// leuk_he: end prevent switch

	pstrPendingLink = NULL;

	//Xman
	// MOD BEGIN netfinity: P2PThreat - Detect worms that could be harmful to the network or eMule
	if (theP2PThreat.IsMachineInfected())
		return false;
	// MOD END netfinity

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

	m_sizSmallSystemIcon.cx = GetSystemMetrics(SM_CXSMICON);
	m_sizSmallSystemIcon.cy = GetSystemMetrics(SM_CYSMICON);
	UpdateLargeIconSize();
	UpdateDesktopColorDepth();

	CWinApp::InitInstance();

	//<<< eWombat [WINSOCK2] 
	/*
	if (!AfxSocketInit())
	{
		AfxMessageBox(GetResString(IDS_SOCKETS_INIT_FAILED));
		return FALSE;
	}
	*/
	memset(&m_wsaData,0,sizeof(WSADATA));
	if (!InitWinsock2(&m_wsaData)) // <<< eWombat first try it with winsock2
	{
		memset(&m_wsaData,0,sizeof(WSADATA));
		if (!AfxSocketInit(&m_wsaData)) // <<< eWombat then try it with old winsock
		{
			AfxMessageBox(GetResString(IDS_SOCKETS_INIT_FAILED));
			return FALSE;
		}
	}
	//>>> eWombat [WINSOCK2]
// ==> Visual Studio 2010 Compatibility [Stulle/Avi-3k/ied] - Stulle
/*
#if _MFC_VER==0x0700 || _MFC_VER==0x0710 || _MFC_VER==0x0800 || _MFC_VER==0x0900
*/
#if _MFC_VER==0x0700 || _MFC_VER==0x0710 || _MFC_VER==0x0800 || _MFC_VER==0x0900 || _MFC_VER==0x0A00
// <== Visual Studio 2010 Compatibility [Stulle/Avi-3k/ied] - Stulle
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

	//Xman new slpash-screen arrangement
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

	UpdateSplash(_T("Loading ...")); 
	//Xman end

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

	//Xman process prio
	SetPriorityClass(GetCurrentProcess(), thePrefs.GetMainProcessPriority()); // [TPT] - Select process priority 
	//Xman end

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

	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	// use service control handler instead
	if (!RunningAsService())
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle
		SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);

	CemuleDlg dlg;
	emuledlg = &dlg;
	m_pMainWnd = &dlg;

	sysinfo = new CSystemInfo();// CPU/MEM usage [$ick$/Stulle] - Max 

	// Barry - Auto-take ed2k links
	if (thePrefs.AutoTakeED2KLinks())
		Ask4RegFix(false, true, false);

	if (thePrefs.GetAutoStart())
		::AddAutoStart();
	else
		::RemAutoStart();

	m_pFirewallOpener = new CFirewallOpener();
	m_pFirewallOpener->Init(true); // we need to init it now (even if we may not use it yet) because of CoInitializeSecurity - which kinda ruins the sense of the class interface but ooohh well :P
	
	// ==> Improved ICS-Firewall support [MoNKi] - Max
	if(!thePrefs.GetICFSupport()&& !IsRunningXPSP2() && thePrefs.GetICFSupportFirstTime() && m_pFirewallOpener->DoesFWConnectionExist()){ 	 
		if(MessageBox(NULL, GetResString(IDS_ICFSUPPORTFIRST), _T("eMule"), MB_YESNO | MB_ICONQUESTION) == IDYES){ 	 
			thePrefs.SetICFSupport(true); 	 
		} 	 
		thePrefs.SetICFSupportFirstTime(false); 	 
	}
	// <== Improved ICS-Firewall support [MoNKi] - Max
		
	// Open WinXP firewallports if set in preferences and possible
	// ==> Random Ports [MoNKi] - Stulle
	/*
	if (thePrefs.IsOpenPortsOnStartupEnabled()){
	*/
	if (thePrefs.IsOpenPortsOnStartupEnabled() || thePrefs.GetUseRandomPorts()){
	// <== Random Ports [MoNKi] - Stulle
		if (m_pFirewallOpener->DoesFWConnectionExist()){

			// ==> Improved ICS-Firewall support [MoNKi] - Max
			/*
			// delete old rules added by eMule
			m_pFirewallOpener->RemoveRule(EMULE_DEFAULTRULENAME_UDP);
			m_pFirewallOpener->RemoveRule(EMULE_DEFAULTRULENAME_TCP);
			*/
			// delete old rules added by eMule
			m_pFirewallOpener->ClearOld();
			// <== Improved ICS-Firewall support [MoNKi] - Max

			// open port for this session
			// ==> Random Ports [MoNKi] - Stulle
			/*
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
			*/
			// <== Random Ports [MoNKi] - Stulle
		}
	}

	// UPnP Port forwarding
	// ==> UPnP support [MoNKi] - leuk_he
	/*
#ifdef DUAL_UPNP //zz_fly :: dual upnp
	//UPnP chooser
	m_pUPnPFinder = NULL;
	m_pUPnPNat = NULL;
	if (thePrefs.m_bUseACATUPnPCurrent)
		m_pUPnPNat = new MyUPnP();
	else
		m_pUPnPFinder = new CUPnPImplWrapper(); //Official UPNP
#else
	m_pUPnPFinder = new CUPnPImplWrapper();
#endif //zz_fly :: dual upnp
	*/
	if((m_UPnP_IGDControlPoint != NULL && thePrefs.IsUPnPEnabled()) || thePrefs.GetUpnpDetect()>0){  //leuk_he add startupwizard auto detect
      m_UPnP_IGDControlPoint->Init(thePrefs.GetUPnPLimitToFirstConnection());
		if(thePrefs.GetUPnPClearOnClose() /*|| thePrefs.GetUseRandomPorts()*/)
			m_UPnP_IGDControlPoint->DeleteAllPortMappingsOnClose();
	}
	// <== UPnP support [MoNKi] - leuk_he

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

	UpdateSplash(_T("Loading bandwidthcontrol ...")); //Xman new slpash-screen arrangement

	// ZZ:UploadSpeedSense -->
	//Xman
	// - Maella [patch] -Bandwidth: overall bandwidth measure-	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	/*
    lastCommonRouteFinder = new LastCommonRouteFinder();
	*/
	pBandWidthControl = new CBandWidthControl();
	// Maella end
	//Xman end
    uploadBandwidthThrottler = new UploadBandwidthThrottler();
	// ZZ:UploadSpeedSense <--

	UpdateSplash(_T("Loading DLP ...")); //Xman new slpash-screen arrangement
	dlp = new CDLP(thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR),thePrefs.GetMuleDirectory(EMULE_CONFIGDIR)); //Xman DLP
	UpdateSplash(_T("Loading IP to Country ...")); //Xman new slpash-screen arrangement
	ip2country = new CIP2Country(); //EastShare - added by AndCycle, IP to Country
	UpdateSplash(_T("Loading lists ...")); //Xman new slpash-screen arrangement
	clientlist = new CClientList();
	friendlist = new CFriendList();
	searchlist = new CSearchList();
	knownfiles = new CKnownFileList();
	serverlist = new CServerList();
	serverconnect = new CServerConnect();
	sharedfiles = new CSharedFileList(serverconnect);
	listensocket = new CListenSocket();
	clientudp	= new CClientUDPSocket();
	UpdateSplash(_T("Loading credits ...")); //Xman new slpash-screen arrangement
	clientcredits = new CClientCreditsList();
	UpdateSplash(_T("Loading queues ...")); //Xman new slpash-screen arrangement
	downloadqueue = new CDownloadQueue();	// bugfix - do this before creating the uploadqueue
	uploadqueue = new CUploadQueue();
	UpdateSplash(_T("Loading IPfilter ...")); //Xman new slpash-screen arrangement
	ipfilter 	= new CIPFilter();
	webserver = new CWebServer(); // Webserver [kuchin]
	mmserver = new CMMServer();
	scheduler = new CScheduler();
	m_pPeerCache = new CPeerCacheFinder();
	
	// ==> TBH: minimule - Max
	minimule = new CTBHMM(); 
	// <== TBH: minimule - Max
	
	thePerfLog.Startup();

	//Xman don't overwrite bak files if last sessions crashed
	thePrefs.m_this_session_aborted_in_an_unnormal_way=true;
	thePrefs.Save();
	//Xman end

	// ==> Automatic shared files updater [MoNKi] - Stulle
	m_directoryWatcherCloseEvent = NULL;
	m_directoryWatcherReloadEvent = NULL;
	if(thePrefs.GetDirectoryWatcher() && !thePrefs.GetSingleSharedDirWatcher())
		theApp.ResetDirectoryWatcher();
	// <== Automatic shared files updater [MoNKi] - Stulle

	UpdateSplash(_T("initializing  main-window ...")); //Xman new slpash-screen arrangement
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

	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	if (NtserviceStartwhenclose)
		NtServiceStart(); // how to handle errors...? 
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle

	ClearDebugLogQueue(true);
	ClearLogQueue(true);

	DestroySplash(); //Xman new slpash-screen arrangement

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
	bool bExitParam=false; // Run eMule as NT Service [leuk_he/Stulle] - Stulle

	for (int i = 1; i < __argc; i++){
		LPCTSTR pszParam = __targv[i];
		if (pszParam[0] == _T('-') || pszParam[0] == _T('/')){
			pszParam++;
			// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
			if (_tcscmp(pszParam, _T("install"))==0)  {CmdInstallService();bExitParam = true; }
			if (_tcscmp(pszParam, _T("uninstall"))==0){CmdRemoveService();bExitParam = true; }
			if (_tcscmp(pszParam, _T("AsAService"))==0){OnStartAsService();} // NTservice entry point registation.
			// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle
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
	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	// for vista
	if (Is_Terminal_Services()) // Terminal services active? use global\ prefix for ts awareness. 
		strMutextName.Format(_T("Global\\%s:%u"), EMULE_GUID, uTcpPort);
	else
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle
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
			// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
			else if (IsServiceRunningMutexActive()) 
				PassLinkToWebService(sendstruct.dwData,*command);
			// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle
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
			// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
			else if (IsServiceRunningMutexActive()) 
				PassLinkToWebService(sendstruct.dwData,*command);
			// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle
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
			// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
			/*
			// Don't start if we were invoked with 'exit' command.
			if (command->CompareNoCase(_T("exit")) == 0) {
			*/
			else if (IsServiceRunningMutexActive()) 
				PassLinkToWebService(sendstruct.dwData,*command);

			// Don't start if we were invoked with 'exit' command.
			if (  (command->CompareNoCase(_T("exit")) == 0)||
				(command->CompareNoCase(_T("uninstall")) == 0))
			{
			// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle
				delete command;
				return true;
			}
			delete command;
		}
    }
	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	else if (maininst == NULL && bAlreadyRunning== true && IsServiceRunningMutexActive() ){ // should be service....
		if (InterfaceToService()== false)	{ // stop service or start browser to 127.0.0.1....
			m_hMutexOneInstance = ::CreateMutex(NULL, FALSE, strMutextName);  // gui..so create mutex to prevent 2nd startup. 
			return bAlreadyRunning = ( ::GetLastError() == ERROR_ALREADY_EXISTS ||::GetLastError() == ERROR_ACCESS_DENIED);
		}
		else
			return true; // let browser do GUI 
	}
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle
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

//Xman
// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
/*
void CemuleApp::UpdateReceivedBytes(uint32 bytesToAdd) {
	SetTimeOnTransfer();
	theStats.sessionReceivedBytes+=bytesToAdd;
}
*/
void CemuleApp::UpdateReceivedBytes(uint32 /*bytesToAdd*/) {
	SetTimeOnTransfer();
	//theStats.sessionReceivedBytes+=bytesToAdd; 
}
//Xman end

void CemuleApp::UpdateSentBytes(uint32 bytesToAdd, bool sentToFriend) {
	SetTimeOnTransfer();
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	/*
	theStats.sessionSentBytes+=bytesToAdd;
	*/
	//Xman end

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

		//Xman
		// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
		/*
		_snprintf(buffer, _countof(buffer), "%.1f", (float)downloadqueue->GetDatarate() / 1024);
		buffer[_countof(buffer) - 1] = '\0';
		file.Write(buffer, strlen(buffer)); 
		file.Write("|", 1);

		_snprintf(buffer, _countof(buffer), "%.1f", (float)uploadqueue->GetDatarate() / 1024); 
		buffer[_countof(buffer) - 1] = '\0';
		*/
		uint32 eMuleIn;
		uint32 eMuleOut;
		uint32 notUsed;
		theApp.pBandWidthControl->GetDatarates(thePrefs.GetDatarateSamples(),
			eMuleIn, notUsed,
			eMuleOut, notUsed,
			notUsed, notUsed);
		_snprintf(buffer, _countof(buffer), "%.1f", (float)eMuleIn/1024.0f); 
		buffer[_countof(buffer) - 1] = '\0';
		file.Write(buffer, strlen(buffer)); 
		file.Write("|", 1);
		_snprintf(buffer, _countof(buffer), "%.1f", (float)eMuleOut/1024.0f);
		buffer[_countof(buffer) - 1] = '\0';
		//Xman end
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
		ASSERT ( m_pPeerCache );

		//Xman new adapter selection 
		if(m_dwPublicIP!=dwIP)
		{
			uint32 test=dwIP;
			CString tmp;
			tmp.Format(_T("received an IP: %u.%u.%u.%u, NAFC-Adapter will be checked"), (uint8)test, (uint8)(test>>8), (uint8)(test>>16), (uint8)(test>>24));
			AddLogLine(false,tmp);
			theApp.pBandWidthControl->checkAdapterIndex(dwIP);
		}
		//Xman end

		if ( GetPublicIP() == 0)
			AddDebugLogLine(DLP_VERYLOW, false, _T("My public IP Address is: %s"),ipstr(dwIP));
		else if (Kademlia::CKademlia::IsConnected() && Kademlia::CKademlia::GetPrefs()->GetIPAddress())
			if(ntohl(Kademlia::CKademlia::GetIPAddress()) != dwIP)
			{
				// Xman reconnect Kad on IP-change
				/*
				AddDebugLogLine(DLP_DEFAULT, false,  _T("Public IP Address reported from Kademlia (%s) differs from new found (%s)"),ipstr(ntohl(Kademlia::CKademlia::GetIPAddress())),ipstr(dwIP));
				*/
				AddDebugLogLine(DLP_DEFAULT, false,  _T("Public IP Address reported from Kademlia (%s) differs from new found (%s), restart Kad"),ipstr(ntohl(Kademlia::CKademlia::GetIPAddress())),ipstr(dwIP));
				Kademlia::CKademlia::Stop();
				Kademlia::CKademlia::Start();
				//Kad loaded the old IP, we must reset
				if(Kademlia::CKademlia::IsRunning())
				{
					Kademlia::CKademlia::GetPrefs()->SetIPAddress(0);
					Kademlia::CKademlia::GetPrefs()->SetIPAddress(htonl(dwIP));
				}
				//Xman end
			}
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
					// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
					/*
					//Xman [MoNKi: -Check already downloaded files-]
					if ( askIfAlreadyDownloaded )
					{
						if ( knownfiles->CheckAlreadyDownloadedFileQuestion(pLink->GetFileLink()->GetHashKey(), pLink->GetFileLink()->GetName()) )
							downloadqueue->AddFileLinkToDownload(pLink->GetFileLink(), cat);
					}
					else
						theApp.downloadqueue->AddFileLinkToDownload(pLink->GetFileLink(), cat);
					//Xman end
					*/
					// pFileLink IS NOT A LEAK, DO NOT DELETE.
					CED2KFileLink* pFileLink = (CED2KFileLink*)CED2KLink::CreateLinkFromUrl(strTok.Trim());

					//Xman [MoNKi: -Check already downloaded files-]
					if ( askIfAlreadyDownloaded )
					{
						if ( knownfiles->CheckAlreadyDownloadedFileQuestion(pLink->GetFileLink()->GetHashKey(), pLink->GetFileLink()->GetName()) )
							downloadqueue->AddFileLinkToDownload(pFileLink->GetFileLink(),cat, true);
					}
					else
						theApp.downloadqueue->AddFileLinkToDownload(pFileLink->GetFileLink(),cat, true);
					// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
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
			// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
			/*
			AddEd2kLinksToDownload(pszTrimmedLinks, 0, true);
			*/
			AddEd2kLinksToDownload(strLinks, -1, true);
			// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
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
	//Xman
	/*
	if (IsConnected() || (uploadqueue != NULL && uploadqueue->GetUploadQueueLength() > 0)
		|| (downloadqueue != NULL && downloadqueue->GetDatarate() > 0))
	*/
	// Retrieve the current datarates
	uint32 eMuleIn;
	uint32 notUsed;
	theApp.pBandWidthControl->GetDatarates(thePrefs.GetDatarateSamples(),
		eMuleIn, notUsed,
		notUsed, notUsed,
		notUsed, notUsed);
	if (IsConnected() || (uploadqueue != NULL && uploadqueue->GetUploadQueueLength() > 0)
		|| eMuleIn>0)
	//Xman end
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

//Xman new slpash-screen arrangement
void CemuleApp::ShowSplash(bool start)
{
	ASSERT( m_pSplashWnd == NULL );
	if (m_pSplashWnd == NULL)
	{
		m_pSplashWnd = new CSplashScreenEx();
		if (m_pSplashWnd != NULL)
		{

			// ==> ModID [itsonlyme/SiRoB] - Stulle
			/*
			if (m_pSplashWnd->Create(NULL,MOD_MAJOR_VERSION ,0,CSS_FADE | CSS_CENTERSCREEN | CSS_SHADOW | CSS_HIDEONCLICK))
			*/
			CString temp = _T("eMule v") + theApp.m_strCurVersionLong;
			if (m_pSplashWnd->Create(NULL,temp ,0,CSS_FADE | CSS_CENTERSCREEN | CSS_SHADOW | CSS_HIDEONCLICK))
			// <== ModID [itsonlyme/SiRoB] - Stulle
			{
				m_pSplashWnd->SetBitmap(IDB_SPLASH,0,255,0);
				m_pSplashWnd->SetTextFont(_T("Tahoma"),155,CSS_TEXT_BOLD);
				//CRect x=CRect(10,230,210,265);
				// ==> changed - Stulle
				/*
				CRect x=CRect(10,230,210,248);
				*/
				CRect x=CRect(75,225,325,258);
				// <== changed - Stulle
				m_pSplashWnd->SetTextRect(x);
				m_pSplashWnd->SetTextColor(RGB(0,0,0));
				m_pSplashWnd->SetTextFormat(DT_SINGLELINE | DT_CENTER | DT_VCENTER);
				m_pSplashWnd->Show();
				spashscreenfinished=false;
				if(start) Sleep(1000);
				m_dwSplashTime = ::GetCurrentTime();
				// ==> ModID [itsonlyme/SiRoB] - Stulle
				/*
				m_pSplashWnd->SetText(MOD_VERSION);
				*/
				m_pSplashWnd->SetText(theApp.m_strModLongVersion);
				// <== ModID [itsonlyme/SiRoB] - Stulle
			}
			else
			{
				delete m_pSplashWnd;
				m_pSplashWnd = NULL;
				spashscreenfinished=true;
			}
		}
	}
#ifdef _BETA
	if (!RunningAsService()) // Run eMule as NT Service [leuk_he/Stulle] - Stulle
	if (!thePrefs.IsFirstStart())
		AfxMessageBox(GetResString(IDS_BETANAG), MB_ICONINFORMATION | MB_OK, 0);
#endif
}

void CemuleApp::DestroySplash()
{
	if (m_pSplashWnd != NULL)
	{
		m_pSplashWnd->DestroyWindow();
		m_pSplashWnd->Hide();
		delete m_pSplashWnd;
		m_pSplashWnd = NULL;
	}
	spashscreenfinished=true;
}

void CemuleApp::UpdateSplash(LPCTSTR Text){
	if(m_pSplashWnd)
		m_pSplashWnd->SetText2(Text);
}
//Xman end

//Xman queued disc-access for read/flushing-threads
/*
#define allowed_Threads 1
//threading-info: synchronized with main-thread which is the only caller
void CemuleApp::AddNewDiscAccessThread(CWinThread* threadtoadd)
{
	if(emuledlg->IsRunning()==false)
	{
		//when shuting down, let all flush thread run... in partfile we wait for it
		if(threadtoadd->IsKindOf(RUNTIME_CLASS(CReadBlockFromFileThread)))
			threadtoadd->Delete();
		else
			threadtoadd->ResumeThread();
		return;
	}


	threadqueuelock.Lock();


	if(m_uRunningNonBlockedDiscAccessThreads<=allowed_Threads-1 /*|| thePrefs.dontusediscaccessqueue==true*//*) 
	{
		m_uRunningNonBlockedDiscAccessThreads++;
		threadtoadd->ResumeThread();
	}
	else
	{
		threadqueue.AddTail(threadtoadd);
	}
	threadqueuelock.Unlock();
}

//threading-info: called by different threads
void CemuleApp::ResumeNextDiscAccessThread()
{
	threadqueuelock.Lock();
	if(threadqueue.IsEmpty()==false && (m_uRunningNonBlockedDiscAccessThreads<=allowed_Threads /*|| thePrefs.dontusediscaccessqueue==true*//*))
	{
		CWinThread* threadtorun=threadqueue.RemoveHead();
		threadtorun->ResumeThread();
	}
	else if(m_uRunningNonBlockedDiscAccessThreads > 0)
		m_uRunningNonBlockedDiscAccessThreads--;

	threadqueuelock.Unlock();
}

//doing this when shutting down
//threading-info: synchronized with mainthread which is the only caller
void CemuleApp::ForeAllDiscAccessThreadsToFinish()
{
	threadqueuelock.Lock();
	while(threadqueue.IsEmpty()==false)
	{
		CWinThread* threadtorun=threadqueue.RemoveHead();
		if(threadtorun->IsKindOf(RUNTIME_CLASS(CReadBlockFromFileThread)))
			threadtorun->Delete();
		else
			threadtorun->ResumeThread();
	}

	threadqueuelock.Unlock();
}
*/
//Xman end

// ==> UPnP support [MoNKi] - leuk_he
/*
#ifdef DUAL_UPNP //zz_fly :: dual upnp
//ACAT UPnP
BOOL CemuleApp::AddUPnPNatPort(MyUPnP::UPNPNAT_MAPPING *mapping, bool tryRandom){
	if(!thePrefs.m_bUseACATUPnPCurrent)
		return false;

	CString args;
	if(m_pUPnPNat->AddNATPortMapping(mapping, tryRandom) == MyUPnP::UNAT_OK ){
		if(theApp.emuledlg->IsRunning()){
			AddLogLine(false, _T("Added UPnP NAT Support: (%s) NAT ROUTER/FIREWALL:%i -> %s:%i"),
				mapping->description, mapping->externalPort, m_pUPnPNat->GetLocalIPStr(), mapping->internalPort);
		}
		return true;
	}
	else{
		if(theApp.emuledlg->IsRunning()){
			AddLogLine(false, _T("Error adding UPnP NAT Support: (%s) NAT ROUTER/FIREWALL:%i -> %s:%i (%s)"),
				mapping->description, mapping->externalPort, m_pUPnPNat->GetLocalIPStr(), mapping->internalPort, m_pUPnPNat->GetLastError());
		}
		return false;
	}
}

BOOL CemuleApp::RemoveUPnPNatPort(MyUPnP::UPNPNAT_MAPPING *mapping){
	if(!thePrefs.m_bUseACATUPnPCurrent)
		return false;

	if(m_pUPnPNat->RemoveNATPortMapping(*mapping) == MyUPnP::UNAT_OK )
		return true;
	else
		return false;
}
#endif //zz_fly :: dual upnp
*/
// <== UPnP support [MoNKi] - leuk_he

// Commander - Added: FriendLinks [emulEspaa] - Start - added by zz_fly
bool CemuleApp::IsEd2kFriendLinkInClipboard()
{
	static const CHAR _szEd2kFriendLink[] = "ed2k://|friend|";
	return IsEd2kLinkInClipboard(_szEd2kFriendLink, ARRSIZE(_szEd2kFriendLink)-1);
}
// Commander - Added: FriendLinks [emulEspaa] - End

// ==> Design Settings [eWombat/Stulle] - Stulle
void CemuleApp::CreateExtraFonts(CFont *font)
{
	DestroyExtraFonts();
	LOGFONT lf;
	font->GetLogFont(&lf);
	lf.lfWeight=FW_BOLD;
	m_ExtraFonts[0].CreateFontIndirect(&lf); // bold
	lf.lfUnderline=TRUE;
	m_ExtraFonts[1].CreateFontIndirect(&lf); // bold underlined
	lf.lfItalic = TRUE;
	m_ExtraFonts[2].CreateFontIndirect(&lf); // bold underlined italic
	lf.lfWeight=FW_NORMAL;
	m_ExtraFonts[3].CreateFontIndirect(&lf); // underlined italic
	lf.lfUnderline=FALSE;
	m_ExtraFonts[4].CreateFontIndirect(&lf); // italic
	lf.lfWeight=FW_BOLD;
	m_ExtraFonts[5].CreateFontIndirect(&lf); // italic bold
	lf.lfWeight=FW_NORMAL;
	lf.lfItalic = FALSE;
	lf.lfUnderline=TRUE;
	m_ExtraFonts[6].CreateFontIndirect(&lf); // underlined
	lf.lfUnderline=FALSE;

	// narrow
	_tcscpy(lf.lfFaceName, _T("Arial Narrow"));
	m_ExtraFonts[7].CreateFontIndirect(&lf); // normal
	lf.lfWeight=FW_BOLD;
	m_ExtraFonts[8].CreateFontIndirect(&lf); // bold
	lf.lfUnderline=TRUE;
	m_ExtraFonts[9].CreateFontIndirect(&lf); // bold underlined
	lf.lfItalic = TRUE;
	m_ExtraFonts[10].CreateFontIndirect(&lf); // bold underlined italic
	lf.lfWeight=FW_NORMAL;
	m_ExtraFonts[11].CreateFontIndirect(&lf); // underlined italic
	lf.lfUnderline=FALSE;
	m_ExtraFonts[12].CreateFontIndirect(&lf); // italic
	lf.lfWeight=FW_BOLD;
	m_ExtraFonts[13].CreateFontIndirect(&lf); // italic bold
	lf.lfWeight=FW_NORMAL;
	lf.lfItalic = FALSE;
	lf.lfUnderline=TRUE;
	m_ExtraFonts[14].CreateFontIndirect(&lf); // underlined
}

void CemuleApp::DestroyExtraFonts()
{
	for (UINT i=0;i<ARRSIZE(m_ExtraFonts);i++)
	{
	if (m_ExtraFonts[i].GetSafeHandle())
		m_ExtraFonts[i].DeleteObject();
	}
}

CFont* CemuleApp::GetFontByStyle(DWORD nStyle,bool bNarrow)
{
	nStyle &= STYLE_FONTMASK;
	int i = 0;
	if(bNarrow)
	{
		i = 8;
		if(nStyle == 0)
			return &m_ExtraFonts[7];
	}

	if(nStyle & STYLE_BOLD)
	{
		if(nStyle & STYLE_UNDERLINE)
		{
			if(nStyle & STYLE_ITALIC)
				return &m_ExtraFonts[2+i];
			return &m_ExtraFonts[1+i];
		}
		return &m_ExtraFonts[0+i];
	}
	else if(nStyle & STYLE_ITALIC)
	{
		if(nStyle & STYLE_UNDERLINE)
			return &m_ExtraFonts[3+i];
		if(nStyle & STYLE_BOLD)
			return &m_ExtraFonts[5+i];
		return &m_ExtraFonts[4+i];
	}
	else if(nStyle & STYLE_UNDERLINE)
		return &m_ExtraFonts[6+i];

	return emuledlg->GetFont();
}
// <== Design Settings [eWombat/Stulle] - Stulle

// ==> Automatic shared files updater [MoNKi] - Stulle
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

	// Sets the minimun time between reloads.
	// To set an fixed time between reloads change
	// minSecondsBetweenReloads to a value greater
	// than 0, for example 600 for 10 minutes (600 seconds)
	const DWORD minSecondsBetweenReloads = thePrefs.GetTimeBetweenReloads(); //Variable time

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

		if( dirList.Find( curDir ) == NULL ) {
			dirList.AddTail( curDir );
		}
	}

	// The other shared dirs
	POSITION pos = thePrefs.shareddir_list.GetHeadPosition();
	while(pos){
		curDir = thePrefs.shareddir_list.GetNext(pos);

		// If this folder does not exist we do not need to watch this folder
		if (_taccess(curDir, 0) != 0)
			continue;

		if (curDir.Right(1)==_T("\\"))
			curDir = curDir.Left(curDir.GetLength() - 1);

		if( dirList.Find( curDir ) == NULL ) {
			dirList.AddTail( curDir );
		}
	}

	// Dirs of single shared files
	if(thePrefs.GetSingleSharedDirWatcher()/* && theApp.sharedfiles->ProbablyHaveSingleSharedFiles()*/)
	{
		for (POSITION pos = theApp.sharedfiles->m_liSingleSharedFiles.GetHeadPosition(); pos != NULL; theApp.sharedfiles->m_liSingleSharedFiles.GetNext(pos))
		{
			curDir = theApp.sharedfiles->m_liSingleSharedFiles.GetAt(pos);
			if (_taccess(curDir, 0) != 0)
				continue; // only add for this single shared file if it exists

			int length = curDir.ReverseFind(_T('\\'));
			if(length != -1) // should always be true... anyway, just in case...
				curDir = curDir.Left(length);

			if( dirList.Find( curDir ) == NULL ) {
				dirList.AddTail( curDir );
			}
		}
	}

	// dirList now contains all shared dirs.
	// Now we get the parent dirs of shared dirs,
	// Why? To check if the user renames or removes a shared dir,
	// because FindFirstChangeNotification don't notifies this changes
	// on the own directory.
	
	// Save the start position of parents
	int parentsStartPosition = dirList.GetCount();
	int curPos = 0;
	
	pos = dirList.GetHeadPosition();
	while(pos && curPos != parentsStartPosition){
		curDir = dirList.GetNext(pos); 
		curPos++;

		int findPos = curDir.ReverseFind(_T('\\'));
		if(findPos != -1)
			curDir = curDir.Left(findPos);

		if( dirList.Find( curDir ) == NULL ) {
			dirList.AddTail( curDir );
		}
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
	
	if(!m_directoryWatcherCloseEvent)
	{
		ASSERT(0);
		DebugLogError(_T("ASFU: Crashed. Will be disabled now")); // it would crash ;)
		thePrefs.SetDirectoryWatcher(false);
	}
	else if(dwChangeHandles){
		// dwChangeHandles[0] will be an event handler to finish the thread 
		dwChangeHandles[0] = m_directoryWatcherCloseEvent->m_hObject;

		// dwChangeHandles[1] will be an event handler to reload the files
		dwChangeHandles[1] = m_directoryWatcherReloadEvent->m_hObject;

		// Generate shared/parents directories handlers
		curPos = 2; // Start at pos 2, because the pos 0 and 1 for the events.
		pos = dirList.GetHeadPosition();
		while(pos){
			curDir = dirList.GetNext(pos); 
			dwChangeHandles[curPos] = FindFirstChangeNotification(
				curDir, FALSE,
				FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
				FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE |
				FILE_NOTIFY_CHANGE_ATTRIBUTES);
			
			if(dwChangeHandles[curPos] == INVALID_HANDLE_VALUE){
				dwChangeHandles[curPos] = nullEvent.m_hObject;
			}
			curPos++; 
		}
	
		// Waits for an event
		DWORD dwWaitStatus;
		DWORD dwWaitStatusClose;

		while(m_directoryWatcherCloseEvent /*true*/){ 
			dwWaitStatus = WaitForMultipleObjects(nChangeHandles, dwChangeHandles, 
				FALSE, INFINITE); 

			// figure out what got signaled
			if(dwWaitStatus - WAIT_OBJECT_0 >= 2)
			{
				// get the dir from the list
				pos = dirList.GetHeadPosition();
				for(DWORD dw = 2; dw <= dwWaitStatus - WAIT_OBJECT_0; dw++)
					curDir = dirList.GetNext(pos);

				if(_taccess(curDir, 0) != 0){ // this one disappeared just now... w000t
					if( inactiveDirList.Find( curDir ) == NULL ) // well, it should not be in the list but to be sure
						inactiveDirList.AddTail( curDir ); // add to list of inactive shares
				}
				//theApp.QueueDebugLogLine(false,_T("ASFU: Handle number %i --> %s"), dwWaitStatus - WAIT_OBJECT_0, curDir);
			}
			//else
				//theApp.QueueDebugLogLine(false,_T("ASFU: Handle number %i"), dwWaitStatus-WAIT_OBJECT_0);

			// Maybe more than one object has been released,
			// check if the Close Event has been signaled
			// because it has precedence.
			dwWaitStatusClose = WaitForSingleObject(m_directoryWatcherCloseEvent->m_hObject, 0);
			if(dwWaitStatusClose == WAIT_OBJECT_0){
				// We want to finalize the thread
				dwWaitStatus = WAIT_OBJECT_0;
			}

			if(dwWaitStatus > WAIT_OBJECT_0 &&
				dwWaitStatus < WAIT_OBJECT_0  + nChangeHandles)
			{
				bool reloadShared = true;

				if(dwWaitStatus > (WAIT_OBJECT_0 + 1) + parentsStartPosition){
					// A parent of a shared dir has been modified.
					// Search changes in shared dirs and reload
					// it only if needed.
					reloadShared = false;

					// Note, this was changed in v3.4 and will run through all our monitored shares
					// so we can maintain a complete list of inactive shares. This list will then allow
					// us to reload share when a shared folder goes missing and when it pops up again.
					pos = dirList.GetHeadPosition();
					while(pos && pos != parentListPos){
						curDir = dirList.GetNext(pos); 
						if (curDir.Right(1) != _T(":")){ // not a root dir
							if(_taccess(curDir, 0) != 0){ // does not exist
								if( inactiveDirList.Find( curDir ) == NULL ) { // and not yet in list
									inactiveDirList.AddTail( curDir ); // add to list of inactive shares

									// Reload shared files
									reloadShared = true;
								}
							}
							else{ // does exist
								POSITION inactivePos = inactiveDirList.Find( curDir );
								if( inactivePos != NULL ) { // and in list of inactive shares
									inactiveDirList.RemoveAt( inactivePos ); // remove from list

									// Reload shared files
									reloadShared = true;
								}
							}
						}
					}
				}

				if(reloadShared){
					// Here we have a problem, if more than one file
					// has changed (for example when the user deletes or moves
					// a list of files), there will be a lot of notifications and
					// the shared file list will be reloaded multiple times.
					// The next code tries to minimize the number of reloads.

					// Stop all pending notifications
					// (we are going to reload ALL files, no more notifications needed)
					for(int i=2; i<nChangeHandles; i++){
						if(dwChangeHandles[i] != nullEvent.m_hObject)
							FindCloseChangeNotification(dwChangeHandles[i]);
					}

					// Wait a few seconds. Should be sufficient to skip
					// a lot of notifications generated by multiple files
					const DWORD curTime = ::GetTickCount();
					const DWORD ts = curTime - lastReloadTime;
					const uint32 seconds = ts/1000;
					
					if(minSecondsBetweenReloads == 0)
					{
						if(seconds < reloadSleepTime)
						{
							if(reloadSleepTime < 1280) //Max 21 minutes between reloads (aprox)
								reloadSleepTime *= 2;
						}
						else
							reloadSleepTime = 5;
					}
					else
					{
						if(seconds < minSecondsBetweenReloads)
							reloadSleepTime  = minSecondsBetweenReloads - seconds;
						else
							reloadSleepTime  = 5;
					}
						
					AddDebugLogLine(false, GetResString(IDS_ASFU_RELOADTIME), reloadSleepTime);
					
					// Waits reloadSleepTime seconds or until the close event is set
					dwWaitStatus = WaitForSingleObject(
						m_directoryWatcherCloseEvent->m_hObject,
						reloadSleepTime * 1000);

					// Checks if a part file is completing
					// and delay the reload in this case
					reloadShared = false;
					bool firstTime = true;
					while(!reloadShared){
						reloadShared = true;
						for(int i=0; i < theApp.downloadqueue->GetFileCount() && reloadShared; i++){
							CPartFile *pFile = theApp.downloadqueue->GetFileByIndex(i);
							if(pFile && pFile->GetStatus() == PS_COMPLETING){
								if(firstTime){
									AddDebugLogLine(false, GetResString(IDS_ASFU_DELAY));
									firstTime = false;
								}
								reloadShared = false;
							}
						}
						if(!reloadShared){
							// Waits 10 seconds or until the close event is set
							dwWaitStatus = WaitForSingleObject(
								m_directoryWatcherCloseEvent->m_hObject, 10000);

							if(dwWaitStatus != WAIT_TIMEOUT){
								// We want to close eMule
								// this forces the 'while' end
								reloadShared = true;
							}
						}
					}

					// Reload
					if(dwWaitStatus == WAIT_TIMEOUT){
						//Restart all notifications again
						curPos = 2; // Start at pos 2, because the pos 0 and 1 is for the events.
						pos = dirList.GetHeadPosition();
						while(pos){
							curDir = dirList.GetNext(pos);

							dwChangeHandles[curPos] = FindFirstChangeNotification(
								curDir, FALSE,
								FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
								FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE |
								FILE_NOTIFY_CHANGE_ATTRIBUTES);

							if(dwChangeHandles[curPos] == INVALID_HANDLE_VALUE){
								dwChangeHandles[curPos] = nullEvent.m_hObject;
							}
							curPos++;
						}
					
						// Reload shared files
						if(theApp.emuledlg->IsRunning()){
							AddDebugLogLine(false, GetResString(IDS_ASFU_RELOADING));
							AddLogLine(false, GetResString(IDS_ASFU_RELOAD));

							m_directoryWatcherReloadEvent->ResetEvent();
//							theApp.sharedfiles->Reload();
//							theApp.emuledlg->sharedfileswnd->Reload(true);
							theApp.emuledlg->sharedfileswnd->SendMessage(WM_COMMAND, IDC_RELOADSHAREDFILES); // fix by Wiz
							lastReloadTime = ::GetTickCount();
						}
					}
					else{
						delete[] dwChangeHandles;
						m_directoryWatcherCS.Unlock();
						return 1;
					}
				}
				else{
					// Get new changes
					FindNextChangeNotification(dwChangeHandles[dwWaitStatus - WAIT_OBJECT_0]);
				}
			}
			else{
				// End the thread
				for(int i=2; i<nChangeHandles; i++){
					if(dwChangeHandles[i] != nullEvent.m_hObject)
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

		if(m_directoryWatcherCloseEvent == NULL)
			m_directoryWatcherCloseEvent = new CEvent(FALSE, TRUE);

		if(m_directoryWatcherReloadEvent == NULL)
			m_directoryWatcherReloadEvent = new CEvent(FALSE, TRUE);

		if(m_directoryWatcherCloseEvent != NULL &&
			m_directoryWatcherReloadEvent != NULL)
		{
			// This is based on v3.2. v3.3 was never called this but
			// adding capabilities for shareSubdir is worth considering
			// the previous version as v3.3. New v3.4 addresses single
			// shared files and some gui handling around device changes.
			// v3.5 fixes crashes when too many dirs are shared. See 
			// above for a more detaied explanation.

			// ScarAngel note: the capability to work with sub dirs has been
			// removed for the time being.
			AddDebugLogLine(false, _T("ASFU: Starting v3.5"));

			// Starts new thread
			AfxBeginThread(CheckDirectoryForChangesThread, this);
		}
	}
}

void CemuleApp::EndDirectoryWatcher() 
 { 
      if(m_directoryWatcherCloseEvent != NULL) 
      { 
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

void CemuleApp::DirectoryWatcherExternalReload(){
	if(m_directoryWatcherCloseEvent != NULL &&
		m_directoryWatcherReloadEvent != NULL){
		AddDebugLogLine(false, _T("ASFU: Forcing reload"));

		// Notifies the thread to reload
		m_directoryWatcherReloadEvent->SetEvent();
	}
}
// <== Automatic shared files updater [MoNKi] - Stulle

// ==> UPnP support [MoNKi] - leuk_he
void CemuleApp::RebindUPnP()
{
	if(!thePrefs.IsUPnPEnabled())
		return;
	clientudp->Rebind();
	listensocket->Rebind();

	if(theApp.m_UPnP_IGDControlPoint->IsUpnpAcceptsPorts())
	{
		if(thePrefs.GetUPnPNatWeb())
		{
			// Remove Web Interface UPnP
			m_UPnP_IGDControlPoint->DeletePortMapping(thePrefs.GetWSPort(), CUPnP_IGDControlPoint::UNAT_TCP, _T("Web Interface"));

			// Readd Web Interface UPnP
			m_UPnP_IGDControlPoint->AddPortMapping(thePrefs.GetWSPort(), CUPnP_IGDControlPoint::UNAT_TCP, _T("Web Interface"));
		}

		//TODO: Wap interface needs an own setting and should be rebound, too
		/*
		{
			// Remove Wap Interface UPnP
			theApp.m_UPnP_IGDControlPoint->DeletePortMapping(thePrefs.GetWapPort(), CUPnP_IGDControlPoint::UNAT_TCP, _T("Wap Interface"));

			// Readd Wap Interface UPnP
			theApp.m_UPnP_IGDControlPoint->AddPortMapping(thePrefs.GetWapPort(), CUPnP_IGDControlPoint::UNAT_TCP, _T("Wap Interface"));
		}
		*/
	}
}
// <== UPnP support [MoNKi] - leuk_he

// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
bool CemuleApp::IsRunningAsService(int OptimizeLevel )
{
	//if (OptimizeLevel < 5)	// 5: all optmization except server list : need an option for preferneces. 
	if (OptimizeLevel < thePrefs.GetServiceOptLvl())
		return RunningAsService();
	else
		return false;  // disable optimizations
}
// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle