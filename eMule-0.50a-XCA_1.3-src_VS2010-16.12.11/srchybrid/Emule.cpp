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
//#include <atlimage.h>
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
//#include "LastCommonRouteFinder.h" //Xman
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
#include "UPnPImplWrapper.h" //Official UPNP
#include "TransferDlg.h"
#include "VisualStylesXP.h"

//Xman
#include "BandWidthControl.h" // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
#include "P2PThreat.h" // netfinity: Detect worms that are harmful to P2P apps
//Xman new slpash-screen arrangement
#include "SplashScreenEx.h"
#include "FileVerify.h"// X: [FV] - [FileVerify]
#include "DirectDownloadDlg.h"// X: [UIC] - [UIChange] allow change cat
#include "SpeedGraphWnd.h" // X: [SGW] - [SpeedGraphWnd]
#include "GDIPlusUtil.h" // X: [GPUI] - [GDI Plus UI]
#ifdef CLIENTANALYZER
#include "Addons/AntiLeech/ClientAnalyzer.h" //>>> WiZaRd::ClientAnalyzer
#endif
#include "Addons/Modname/Modname.h" //>>> WiZaRd::Easy ModVersion

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
/*bool g_bUnicoWS = false;// X: [CI] - [Code Improvement]

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
		// NOTE: Do *NOT* use any MFC nor W-functions here!
		// NOTE: Do *NOT* use eMule's localization functions here!
		MessageBoxA(NULL,"Failed to load the \"unicows.dll\".",	"eMule", MB_ICONSTOP | MB_SYSTEMMODAL | MB_SETFOREGROUND);
		exit(1);
	}

	//g_bUnicoWS = true;// X: [CI] - [Code Improvement]
	return hUnicoWS;
}

// NOTE: Do *NOT* change the name of this function. It *HAS* to be named "_PfnLoadUnicows" !
extern "C" HMODULE (__stdcall *_PfnLoadUnicows)(void) = &ExplicitPreLoadUnicows;
*/

///////////////////////////////////////////////////////////////////////////////
// C-RTL Memory Debug Support
// 
#ifdef _DEBUG
static CMemoryState oldMemState, newMemState, diffMemState;

_CRT_ALLOC_HOOK g_pfnPrevCrtAllocHook = NULL;
CAtlMap<const unsigned char*, UINT> g_allocations;
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
#if !defined(_WIN64) && !defined(_DEBUG)// X: [64P] - [64BitPortable]
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
#endif

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
	if(CemuleDlg::IsRunning())
		theApp.QueueDebugLogLine(false, _T("%s"), msg);
}

void CALLBACK myDebugAndLogHandler(LPCSTR lpMsg)
{
	if(CemuleDlg::IsRunning())
		theApp.QueueDebugLogLine(false, _T("%hs"), lpMsg);
}

void CALLBACK myLogHandler(LPCSTR lpMsg)
{
	if(CemuleDlg::IsRunning())
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
	if(thePrefs.InitWinVersion() == 0){
		AfxMessageBox(MOD_VERSION _T(" must be running on Windows 2000 or higer"),	MB_ICONSTOP | MB_SYSTEMMODAL | MB_SETFOREGROUND);
		exit(1);
	}
	// Initialize Windows security features.
#if !defined(_WIN64) && !defined(_DEBUG)// X: [64P] - [64BitPortable]
	InitSafeSEH();
#endif
	InitDEP();
	InitHeapCorruptionDetection();

	// This does not seem to work well with multithreading, although there is no reason why it should not.
	//_set_sbh_threshold(768);

	m_dwPublicIP = 0;
	//Xman -Reask sources after IP change- v4
	m_bneedpublicIP = false; 
	last_ip_change = 0;
	last_valid_serverid = 0;
	last_valid_ip = 0;
	recheck_ip = 0;
	last_traffic_reception = 0;
	internetmaybedown = 1;
	//Xman end
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

	m_pSplashWnd = NULL; //Xman new slpash-screen arrangement
	pstrPendingLink = NULL;

	//Xman dynamic IP-Filters
	ipdlgisopen=false;

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
	m_strCurVersionLong.AppendFormat(_T(" %s"), MOD_VERSION); //>>> WiZaRd::Easy ModVersion

	// create the protocol version number
	CString strTmp;
	strTmp.Format(_T("0x%u"), m_dwProductVersionMS);
	VERIFY( _stscanf_s(strTmp, _T("0x%x"), &m_uCurVersionShort) == 1 );
	ASSERT( m_uCurVersionShort < 0x99 );

	// create the version check number
	strTmp.Format(_T("0x%u%c"), m_dwProductVersionMS, _T('A') + CemuleApp::m_nVersionUpd);
	VERIFY( _stscanf_s(strTmp, _T("0x%x"), &m_uCurVersionCheck) == 1 );
	ASSERT( m_uCurVersionCheck < 0x999 );
// MOD Note: end

	m_bGuardClipboardPrompt = false;
	m_pSpeedGraphWnd = NULL; // X: [SGW] - [SpeedGraphWnd]

}


CemuleApp theApp(_T("eMule"));
#if _MFC_VER==0x0900 &&  defined(_AFXDLL)
CProcessLocal<_AFX_SOCK_STATE> _afxSockState;
#ifndef _DEBUG
_AFX_SOCK_STATE::~_AFX_SOCK_STATE()
{
	if (m_pfnSockTerm != NULL)
		m_pfnSockTerm();
}
#endif
#endif

// Workaround for bugged 'AfxSocketTerm' (needed at least for MFC 7.0, 7.1, 8.0, 9.0)
#if _MFC_VER==0x0700 || _MFC_VER==0x0710 || _MFC_VER==0x0800 || _MFC_VER==0x0900 || _MFC_VER==0x0A00
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
// Xman better always enabled, except we don't want
#ifndef _BETA
	//if (GetProfileInt(_T("eMule"), _T("CreateCrashDump"), 0))
	if (GetProfileInt(_T("eMule"), _T("NoCrashDump"), 0)==0)
#endif
		/*
		theCrashDumper.Enable(_T("eMule ") + m_strCurVersionLongDbg + _T(" ") + MOD_VERSION, true, thePrefs.GetMuleDirectory(EMULE_CONFIGDIR)); //Xman ModId
		*/
        CMiniDumper::Enable(_T("eMule ") + theApp.m_strCurVersionLong, true, thePrefs.GetMuleDirectory(EMULE_CONFIGDIR));

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

	if (ProcessCommandline())
		return false;

	//Xman
	// MOD BEGIN netfinity: P2PThreat - Detect worms that could be harmful to the network or eMule
	if (theP2PThreat.IsMachineInfected())
		return false;
	// MOD END netfinity
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
/*
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
	}*/

	g_gdiplus.Init(); // X: [GPUI] - [GDI Plus UI]

	m_sizSmallSystemIcon.cx = GetSystemMetrics(SM_CXSMICON);
	m_sizSmallSystemIcon.cy = GetSystemMetrics(SM_CYSMICON);
	UpdateLargeIconSize();
	UpdateDesktopColorDepth();

	CWinApp::InitInstance();
	/*
	if (!AfxSocketInit())
	{
		AfxMessageBox(GetResString(IDS_SOCKETS_INIT_FAILED));
		return FALSE;
	}*/
	//<<< eWombat [WINSOCK2] 
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

#if _MFC_VER==0x0700 || _MFC_VER==0x0710 || _MFC_VER==0x0800 || _MFC_VER==0x0900 || _MFC_VER==0x0A00
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

	// create & initialize all the important stuff 
	thePrefs.Init();
	theStats.Init();

	//Xman new slpash-screen arrangement
	m_dwSplashTime = (DWORD)-1;
	{
		//use temporary variables to determine if we should show the splash
		// temporary disable the 'startup minimized' option, otherwise no window will be shown at all
		// ==> Invisible Mode [TPT/MoNKi] - Stulle
		bool bStartMinimized = !thePrefs.IsFirstStart() && (thePrefs.GetStartMinimized() || theApp.DidWeAutoStart() ||(thePrefs.m_bInvisibleMode && thePrefs.m_bInvisibleModeStart));
		// <== Invisible Mode [TPT/MoNKi] - Stulle;

		// show splashscreen as early as possible to "entertain" user while starting emule up
		if (thePrefs.UseSplashScreen() && !bStartMinimized)
		{
			//Xman final version: don't show splash on old windows->crash
/*			switch (thePrefs.GetWindowsVersion())
			{
				case _WINVER_98_:
				case _WINVER_95_:	
				case _WINVER_ME_:
					break;
				default:*/
			ShowSplash(true);
//			}
		}
	}

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

#ifdef _DEBUG
	_sntprintf(s_szCrtDebugReportFilePath, _countof(s_szCrtDebugReportFilePath) - 1, _T("%s%s"), thePrefs.GetMuleDirectory(EMULE_LOGDIR, false), APP_CRT_DEBUG_LOG_FILE);
	s_szCrtDebugReportFilePath[_countof(s_szCrtDebugReportFilePath) - 1] = _T('\0');
#endif
	VERIFY( theLog.SetFilePath(thePrefs.GetMuleDirectory(EMULE_LOGDIR, thePrefs.GetLog2Disk()) + _T("eMule.log")) );
	VERIFY( theVerboseLog.SetFilePath(thePrefs.GetMuleDirectory(EMULE_LOGDIR, !thePrefs.GetLog2Disk() && thePrefs.GetDebug2Disk()) + _T("eMule_Verbose.log")) );
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
			QueueLogLine(false, GetResString(
				(m_pFirewallOpener->OpenPort(thePrefs.GetPort(), NAT_PROTOCOL_TCP, EMULE_DEFAULTRULENAME_TCP, true))
				?IDS_FO_TEMPTCP_S:IDS_FO_TEMPTCP_F
				), thePrefs.GetPort());// X: [CI] - [Code Improvement]

			if (thePrefs.GetUDPPort()){
				// open port for this session
				QueueLogLine(false, GetResString(
					(m_pFirewallOpener->OpenPort(thePrefs.GetUDPPort(), NAT_PROTOCOL_UDP, EMULE_DEFAULTRULENAME_UDP, true))
					?IDS_FO_TEMPUDP_S:IDS_FO_TEMPUDP_F
					), thePrefs.GetUDPPort());// X: [CI] - [Code Improvement]
			}
		}
	}

	// UPnP Port forwarding
	m_pUPnPFinder = new CUPnPImplWrapper(); //Official UPNP
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
	pBandWidthControl = new CBandWidthControl();
	// Maella end
	//lastCommonRouteFinder = new LastCommonRouteFinder();
    uploadBandwidthThrottler = new UploadBandwidthThrottler();
	// ZZ:UploadSpeedSense <--
	//Xman end
	// X: queued disc-access for read/flushing-threads
	m_DiscAccessQueue.start();

	UpdateSplash(_T("Loading IP to Country ...")); //Xman new slpash-screen arrangement
	ip2country = new CIP2Country(); //EastShare - added by AndCycle, IP to Country
	fileverify = new CFileVerify(); // X: [FV] - [FileVerify]
	UpdateSplash(_T("Loading lists ...")); //Xman new slpash-screen arrangement
	clientlist = new CClientList();
	friendlist = new CFriendList();
	searchlist = new CSearchList();
	knownfiles = new CKnownFileList();
	serverlist = new CServerList();
	UpdateSplash2(_T("Creating sockets ...")); //Xman new slpash-screen arrangement
	serverconnect = new CServerConnect();
	sharedfiles = new CSharedFileList(serverconnect);
	listensocket = new CListenSocket();
	clientudp	= new CClientUDPSocket();
	UpdateSplash(_T("Loading credits ...")); //Xman new slpash-screen arrangement
	clientcredits = new CClientCreditsList();
	UpdateSplash(_T("Loading queues ...")); //Xman new slpash-screen arrangement
#ifdef CLIENTANALYZER
	antileechlist   = new CAntiLeechDataList(); //>>> WiZaRd::ClientAnalyzer
	UpdateSplash2(_T("Loading AntiLeech Data ...")); //Xman new slpash-screen arrangement
#endif
	downloadqueue = new CDownloadQueue();	// bugfix - do this before creating the uploadqueue
	uploadqueue = new CUploadQueue();
	UpdateSplash(_T("Loading IPfilter ...")); //Xman new slpash-screen arrangement
	ipfilter 	= new CIPFilter();
	webserver = new CWebServer(); // Webserver [kuchin]
	scheduler = new CScheduler();
	//m_pPeerCache = new CPeerCacheFinder(); // X: [RPC] - [Remove PeerCache]
	if(g_gdiplus.IsInited()){ // X: [GPUI] - [GDI Plus UI]
		m_pSpeedGraphWnd = new CSpeedGraphWnd(); // X: [SGW] - [SpeedGraphWnd]
		m_pSpeedGraphWnd->EnableToolTips(true);
		m_pSpeedGraphWnd->OpenDialog(SW_HIDE);
	}

	thePerfLog.Startup();

	//Xman don't overwrite bak files if last sessions crashed
	//thePrefs.m_this_session_aborted_in_an_unnormal_way=true;
	//thePrefs.Save();
	if(thePrefs.IsFirstStart()) // X: keep eMule section first
		theApp.WriteProfileString(L"eMule", L"AppVersion", theApp.m_strCurVersionLong);
	theApp.WriteProfileInt(_T("Xtreme"), _T("last_session_aborted_in_an_unnormal_way"), true);// X: [CI] - [Code Improvement]
	//Xman end

	UpdateSplash(_T("Initializing  main-window ...")); //Xman new slpash-screen arrangement
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

	DestroySplash(); //Xman new slpash-screen arrangement

	AddDebugLogLine(DLP_VERYLOW, _T("%hs: returning: FALSE"), __FUNCTION__);
	return FALSE;
}

int CemuleApp::ExitInstance()
{
	if (pstrPendingLink != NULL)// X: [BF] - [Bug Fix]
		delete pstrPendingLink;

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
	
	// this codepart is to determine special cases when we do add a link to our eMule
	// because in this case it would be nonsense to start another instance!
	if (!bIgnoreRunningInstances ||
		(cmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen
			&& (cmdInfo.m_strFileName.Find(_T("://")) > 0
				 || CCollection::HasCollectionExtention(cmdInfo.m_strFileName)))
	){
		bAlreadyRunning = (::GetLastError() == ERROR_ALREADY_EXISTS ||::GetLastError() == ERROR_ACCESS_DENIED);
		if (bAlreadyRunning) EnumWindows(SearchEmuleWindow, (LPARAM)&maininst);
	}

//>>> Fix for "exit" command (Original by leuk_he)
	if(!bAlreadyRunning)
	{
		if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen
			&& cmdInfo.m_strFileName.Find(L"exit") > 0)
			return TRUE; //do NOT start!
	}
//<<< Fix for "exit" command

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
	DWORD_PTR dwMsgResult = 0;
	LRESULT res = ::SendMessageTimeout(hWnd,UWM_ARE_YOU_EMULE,0, 0,SMTO_BLOCK |SMTO_ABORTIFHUNG,10000,(PDWORD_PTR)&dwMsgResult);
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
void CemuleApp::UpdateReceivedBytes(uint32 /*bytesToAdd*/) {
	SetTimeOnTransfer();
	//theStats.sessionReceivedBytes+=bytesToAdd; 
}
//Xman end

void CemuleApp::UpdateSentBytes(uint32 bytesToAdd, bool sentToFriend) {
	SetTimeOnTransfer();
	//theStats.sessionSentBytes+=bytesToAdd; // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-

    if(sentToFriend == true) {
	    theStats.sessionSentBytesToFriend += bytesToAdd;
    }
}

void CemuleApp::SetTimeOnTransfer() {
	if (theStats.transferStarttime == 0)
		theStats.transferStarttime = GetTickCount();
}

#if defined(_DEBUG)
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
#endif
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
			_tcscpy_s(pGlobalT, strText.GetLength() + 1, strText);
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
			strcpy_s(pGlobalA, strTextA.GetLength() + 1, strTextA);
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
	CString strFullPath = thePrefs.GetMuleDirectory(EMULE_CONFIGBASEDIR);
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
		CString strError = GetResString(IDS_ERROR_SAVEFILE) + _T(' ') + CString(_szFileName);
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
				_ultoa_s(serverconnect->GetCurrentServer()->GetPort(), buffer, 10); 
				file.Write(buffer, (UINT) strlen(buffer));
			}
			else{
				strBuff = "0";
				file.Write(strBuff, (UINT) strBuff.GetLength());
			}
		} 
		else 
			file.Write("0",1); 
		file.Write("\n",1); 

		_snprintf(buffer, _countof(buffer) - 1, "%.1f", (float)emuledlg->GetDownloadDatarate()/1024.0f); 
		buffer[_countof(buffer) - 1] = '\0';
		file.Write(buffer,(UINT)strlen(buffer)); 
		file.Write("|", 1);

		_snprintf(buffer, _countof(buffer) - 1,"%.1f", (float)emuledlg->GetUploadDatarate()/1024.0f);
		buffer[_countof(buffer) - 1] = '\0';
		file.Write(buffer,(UINT)strlen(buffer)); 
		file.Write("|", 1);

		_ultoa_s_ptr(uploadqueue->GetWaitingUserCount(), buffer, _countof(buffer), 10);
		file.Write(buffer,(UINT)strlen(buffer)); 

		file.Close(); 
	}
	catch (CFileException* ex)
	{
		CString strError = GetResString(IDS_ERROR_SAVEFILE) + _T(' ') + CString(_szFileName);
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		ex->GetErrorMessage(szError, _countof(szError));
		strError += _T(" - ");
		strError += szError;
		LogError(LOG_STATUSBAR, _T("%s"), strError);
		ex->Delete();
	}
} //End Added By Bouc7

int CemuleApp::GetFileTypeSystemImageIdx(LPCTSTR pszFilePath, INT_PTR iLength /* = -1 */, bool bNormalsSize)
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
		for (INT_PTR i = iLength - 1; i >= 0; i--){
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
#ifdef REPLACE_ATLMAP
	if (bNormalsSize){
		CStringToIntMap::const_iterator it = m_aBigExtToSysImgIdx.find(pszCacheExt);
		if(it == m_aBigExtToSysImgIdx.end()){
#else
	int vData;
	if (bNormalsSize){
		if (!m_aBigExtToSysImgIdx.Lookup(pszCacheExt, vData)){
#endif
			// Get index for the system's big icon image list
			SHFILEINFO sfi;
			DWORD_PTR dwResult = SHGetFileInfo(pszFilePath, dwFileAttributes, &sfi, sizeof(sfi),
				SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX);
			if (dwResult == 0)
				return 0;
			ASSERT( m_hBigSystemImageList == NULL || m_hBigSystemImageList == (HIMAGELIST)dwResult );
			m_hBigSystemImageList = (HIMAGELIST)dwResult;

			// Store icon index in local cache
#ifdef REPLACE_ATLMAP
			m_aBigExtToSysImgIdx[pszCacheExt] = sfi.iIcon;
#else
			m_aBigExtToSysImgIdx.SetAt(pszCacheExt, sfi.iIcon);
#endif
			return sfi.iIcon;
		}
#ifdef REPLACE_ATLMAP
		return it->second;
#endif
	}
	else{
#ifdef REPLACE_ATLMAP
		CStringToIntMap::const_iterator it = m_aExtToSysImgIdx.find(pszCacheExt);
		if(it == m_aExtToSysImgIdx.end()){
#else
		if (!m_aExtToSysImgIdx.Lookup(pszCacheExt, vData)){
#endif
			// Get index for the system's small icon image list
			SHFILEINFO sfi;
			DWORD_PTR dwResult = SHGetFileInfo(pszFilePath, dwFileAttributes, &sfi, sizeof(sfi),
				SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
			if (dwResult == 0)
				return 0;
			ASSERT( m_hSystemImageList == NULL || m_hSystemImageList == (HIMAGELIST)dwResult );
			m_hSystemImageList = (HIMAGELIST)dwResult;

			// Store icon index in local cache
#ifdef REPLACE_ATLMAP
			m_aExtToSysImgIdx[pszCacheExt] = sfi.iIcon;
#else
			m_aExtToSysImgIdx.SetAt(pszCacheExt, sfi.iIcon);
#endif
			return sfi.iIcon;
		}
#ifdef REPLACE_ATLMAP
		return it->second;
#endif
	}

	// Return already cached value
#ifndef REPLACE_ATLMAP
	return vData;
#endif
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

		//Xman new adapter selection 
		if(m_dwPublicIP!=dwIP)
		{
			AddLogLine(false,_T("received an IP: %u.%u.%u.%u, NAFC-Adapter will be checked"), (uint8)dwIP, (uint8)(dwIP>>8), (uint8)(dwIP>>16), (uint8)(dwIP>>24));
			theApp.pBandWidthControl->checkAdapterIndex(dwIP);
			m_dwPublicIP = dwIP;
			if (serverlist != NULL)
				serverlist->CheckForExpiredUDPKeys();
		}
		//Xman end

		if ( GetPublicIP() == 0)
			AddDebugLogLine(DLP_VERYLOW, false, _T("My public IP Address is: %s"),ipstr(dwIP));
		else if (Kademlia::CKademlia::IsConnected() && Kademlia::CKademlia::GetPrefs()->GetIPAddress())
			if(ntohl(Kademlia::CKademlia::GetIPAddress()) != dwIP)
			{
				// Xman reconnect Kad on IP-change
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
		//m_pPeerCache->FoundMyPublicIPAddress(dwIP);	 // X: [RPC] - [Remove PeerCache]
	}
	else{
		m_dwPublicIP = 0;
		AddDebugLogLine(DLP_VERYLOW, false, _T("Deleted public IP"));
	}
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
				_tcsncpy(szSkinResource, szExpSkinRes, _countof(szSkinResource) - 1);
				szSkinResource[_countof(szSkinResource) - 1] = _T('\0');
			}

			// create absolute path to icon resource file
			TCHAR szFullResPath[MAX_PATH];
			if (PathIsRelative(szSkinResource))
			{
				TCHAR szSkinResFolder[MAX_PATH];
				_tcsncpy(szSkinResFolder, pszSkinProfile, _countof(szSkinResFolder) - 1);
				szSkinResFolder[_countof(szSkinResFolder) - 1] = _T('\0');
				PathRemoveFileSpec(szSkinResFolder);
				_tmakepathlimit(szFullResPath, NULL, szSkinResFolder, szSkinResource, NULL);
			}
			else
			{
				_tcsncpy(szFullResPath, szSkinResource, _countof(szFullResPath) - 1);
				szFullResPath[_countof(szFullResPath) - 1] = _T('\0');
			}

			// check for optional icon index or resource identifier within the icon resource file
			bool bExtractIcon = false;
			CString strFullResPath = szFullResPath;
			int iIconIndex = 0;
			int iComma = strFullResPath.ReverseFind(_T(','));
			if (iComma != -1){
				if (_stscanf_s((LPCTSTR)strFullResPath + iComma + 1, _T("%d"), &iIconIndex) == 1)
					bExtractIcon = true;
				strFullResPath = strFullResPath.Left(iComma);
			}

			if (bExtractIcon)
			{
				if (uFlags != 0 || !(cx == cy && (cx == 16 || cx == 32)))
				{
					/*static UINT (WINAPI *_pfnPrivateExtractIcons)(LPCTSTR, int, int, int, HICON*, UINT*, UINT, UINT) 
						= (UINT (WINAPI *)(LPCTSTR, int, int, int, HICON*, UINT*, UINT, UINT))GetProcAddress(GetModuleHandle(_T("user32")), _TWINAPI("PrivateExtractIcons"));
					if (_pfnPrivateExtractIcons)
					{
						UINT uIconId;
						(*_pfnPrivateExtractIcons)(strFullResPath, iIconIndex, cx, cy, &hIcon, &uIconId, 1, uFlags);
					}*/
					UINT uIconId;
					PrivateExtractIcons(strFullResPath, iIconIndex, cx, cy, &hIcon, &uIconId, 1, uFlags);
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
				_tcsncpy(szSkinResource, szExpSkinRes, _countof(szSkinResource) - 1);
				szSkinResource[_countof(szSkinResource) - 1] = _T('\0');
			}

			// create absolute path to resource file
			TCHAR szFullResPath[MAX_PATH];
			if (PathIsRelative(szSkinResource))
			{
				TCHAR szSkinResFolder[MAX_PATH];
				_tcsncpy(szSkinResFolder, pszSkinProfile, _countof(szSkinResFolder) - 1);
				szSkinResFolder[_countof(szSkinResFolder) - 1] = _T('\0');
				PathRemoveFileSpec(szSkinResFolder);
				_tmakepathlimit(szFullResPath, NULL, szSkinResFolder, szSkinResource, NULL);
			}
			else
			{
				_tcsncpy(szFullResPath, szSkinResource, _countof(szFullResPath) - 1);
				szFullResPath[_countof(szFullResPath) - 1] = _T('\0');
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
				_tcsncpy(szSkinResource, szExpSkinRes, _countof(szSkinResource) - 1);
				szSkinResource[_countof(szSkinResource) - 1] = _T('\0');
			}

			// create absolute path to resource file
			TCHAR szFullResPath[MAX_PATH];
			if (PathIsRelative(szSkinResource))
			{
				TCHAR szSkinResFolder[MAX_PATH];
				_tcsncpy(szSkinResFolder, pszSkinProfile, _countof(szSkinResFolder) - 1);
				szSkinResFolder[_countof(szSkinResFolder) - 1] = _T('\0');
				PathRemoveFileSpec(szSkinResFolder);
				_tmakepathlimit(szFullResPath, NULL, szSkinResFolder, szSkinResource, NULL);
			}
			else
			{
				_tcsncpy(szFullResPath, szSkinResource, _countof(szFullResPath) - 1);
				szFullResPath[_countof(szFullResPath) - 1] = _T('\0');
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
			int iVals = _stscanf_s(szColor, _T("%i , %i , %i"), &red, &grn, &blu);
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

void CemuleApp::AddEd2kLinksToDownload(CString strLinks, size_t cat, bool askIfAlreadyDownloaded) //Xman [MoNKi: -Check already downloaded files-]
{
	int curPos = 0;
	CString strTok = strLinks.Tokenize(_T(" \t\r\n"), curPos); // tokenize by whitespaces
	while (!strTok.IsEmpty())
	{
		if (strTok.Right(1) != _T('/'))
			strTok += _T('/');
		try
		{
			CED2KLink* pLink = CED2KLink::CreateLinkFromUrl(strTok);
			if (pLink){
				if (pLink->GetKind() == CED2KLink::kFile){
					//Xman [MoNKi: -Check already downloaded files-]
					if ( !askIfAlreadyDownloaded
						|| knownfiles->CheckAlreadyDownloadedFileQuestion(((CED2KFileLink*)pLink)/*->GetFileLink()*/->GetHashKey(), ((CED2KFileLink*)pLink)/*->GetFileLink()*/->GetName()))// X: [CI] - [Code Improvement]
							downloadqueue->AddFileLinkToDownload(((CED2KFileLink*)pLink)/*->GetFileLink()*/,cat);
					//Xman end
				}
				else{
					delete pLink;
					throw GetResString(IDS_ERR_NOTAFILELINK);
				}
				delete pLink;
			}
		}
		catch(CString error)
		{
			TCHAR szBuffer[200];
			_sntprintf(szBuffer, _countof(szBuffer) - 1, GetResString(IDS_ERR_INVALIDLINK), error);
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
	if (strLinks.IsEmpty() || strLinks.Compare(m_strLastClipboardContents) == 0)
		return;

	// Do not alter (trim) 'strLinks' and then copy back to 'm_strLastClipboardContents'! The
	// next clipboard content compare would fail because of the modified string.
	LPCTSTR pszTrimmedLinks = strLinks;
	while (_istspace((_TUCHAR)*pszTrimmedLinks)) // Skip leading whitespaces
		pszTrimmedLinks++;
	if (_tcsnicmp(pszTrimmedLinks, _T("ed2k://|file|"), 13) == 0)
	{
		//m_bGuardClipboardPrompt = true;

		// NEO: MLD - [ModelesDialogs] -- Xanatos -->
		if(!emuledlg->directdowndlg){ // X: [UIC] - [UIChange] allow change cat
			emuledlg->directdowndlg = new CDirectDownloadDlg();
			emuledlg->directdowndlg->ClipboardPromptMode(pszTrimmedLinks);
			emuledlg->directdowndlg->OpenDialog();
		}
		emuledlg->directdowndlg->AddLink(pszTrimmedLinks);
		//dialog->setw();
		// NEO: MLD END <-- Xanatos --
		// Don't feed too long strings into the MessageBox function, it may freak out..
		/*CString strLinksDisplay;
		if (strLinks.GetLength() > 512)
			strLinksDisplay = strLinks.Left(512) + _T("...");
		else
			strLinksDisplay = strLinks;
		if (AfxMessageBox(GetResString(IDS_ADDDOWNLOADSFROMCB) + _T("\r\n") + strLinksDisplay, MB_YESNO | MB_TOPMOST) == IDYES)
			AddEd2kLinksToDownload(pszTrimmedLinks, emuledlg->transferwnd->GetDownloadList()->curTab, true); //Xman [MoNKi: -Check already downloaded files-]// X: [AC] - [ActionChange] use transferwnd's select cat as default cat*/
	}
	m_strLastClipboardContents = strLinks; // Save the unmodified(!) clipboard contents
}

void CemuleApp::PasteClipboard(size_t cat)
{
	CString strLinks = CopyTextFromClipboard();
	strLinks.Trim();
	if (strLinks.IsEmpty())
		return;

	AddEd2kLinksToDownload(strLinks, cat, true); //Xman [MoNKi: -Check already downloaded files-]
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
	int iLen = _vsntprintf(bufferline, _countof(bufferline) - 1, line, argptr);
	if (iLen < 0){
		_tcsncpy(bufferline + _countof(bufferline) - 4, _T("..."), 4);
		iLen = _countof(bufferline) - 1;
	}
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
	int iLen = _vsntprintf(bufferline, _countof(bufferline) - 1, line, argptr);
	if (iLen < 0){
		_tcsncpy(bufferline + _countof(bufferline) - 4, _T("..."), 4);
		iLen = _countof(bufferline) - 1;
	}
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
	int iLen = _vsntprintf(bufferline, _countof(bufferline) - 1, line, argptr);
	if (iLen < 0){
		_tcsncpy(bufferline + _countof(bufferline) - 4, _T("..."), 4);
		iLen = _countof(bufferline) - 1;
	}
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
	int iLen = _vsntprintf(bufferline, _countof(bufferline) - 1, line, argptr);
	if (iLen < 0){
		_tcsncpy(bufferline + _countof(bufferline) - 4, _T("..."), 4);
		iLen = _countof(bufferline) - 1;
	}
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
		/*OSVERSIONINFO osvi;
		osvi.dwOSVersionInfoSize = sizeof(osvi);
		if (GetVersionEx(&osvi)
			&& osvi.dwPlatformId == VER_PLATFORM_WIN32_NT
			&& osvi.dwMajorVersion >= 5) // Win2000/XP or higher*/
			m_strDefaultFontFaceName = _T("MS Shell Dlg 2");
		//else
			//m_strDefaultFontFaceName = _T("MS Shell Dlg");
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
		logBrush.lbHatch = (ULONG_PTR)bm.GetSafeHandle();
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
			_sntprintf(szCtrlType, _countof(szCtrlType) - 1, _T("0x%08x"), dwCtrlType);
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
			if (_stscanf_s(szShellLargeIconSize, _T("%u"), &uIconSize) == 1 && uIconSize > 0)
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

	if (IsConnected() || (uploadqueue != NULL && uploadqueue->GetUploadQueueLength() > 0)
		|| emuledlg->GetDownloadDatarate()>0)
	//Xman end
	{
		/*EXECUTION_STATE (WINAPI *pfnSetThreadExecutionState)(EXECUTION_STATE);
		(FARPROC&)pfnSetThreadExecutionState = GetProcAddress(GetModuleHandle(_T("kernel32")), "SetThreadExecutionState");
		if (pfnSetThreadExecutionState)
			VERIFY( pfnSetThreadExecutionState(ES_SYSTEM_REQUIRED) );
		else
			ASSERT( false );*/
		SetThreadExecutionState(ES_SYSTEM_REQUIRED);
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
	if (g_gdiplus.IsInited() && m_pSplashWnd == NULL)
	{
		m_pSplashWnd = new CSplashScreenEx();
		if (m_pSplashWnd != NULL)
		{

			if (m_pSplashWnd->Create(NULL,CSS_FADE | CSS_CENTERSCREEN | CSS_SHADOW | CSS_HIDEONCLICK))
			{
				m_pSplashWnd->Show(start);
				spashscreenfinished=false;

				UpdateSplash(start?_T("Loading ..."):_T("Shutting down ..."));
			}
			else
			{
				delete m_pSplashWnd;
				m_pSplashWnd = NULL;
				spashscreenfinished=true;
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
	spashscreenfinished=true;
/*#ifdef _BETA
	if (!thePrefs.m_bFirstStart)
		AfxMessageBox(GetResString(IDS_BETANAG), MB_ICONINFORMATION | MB_OK, 0);
#endif*/
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
//Xman end

// X: queued disc-access for read/flushing-threads
//threading-info: synchronized with main-thread which is the only caller
void CemuleApp::AddNewDiscAccessThread(Runnable* threadtoadd)
	{
		//when shuting down, let all flush thread run... in partfile we wait for it
	if(CemuleDlg::IsRunning()==false && typeid(*(threadtoadd)) == typeid(CReadBlockFromFileThread))
	{
		delete threadtoadd;
		return;
	}

	m_DiscAccessQueue.push_back(threadtoadd);
}

// Commander - Added: FriendLinks [emulEspaa] - Start - added by zz_fly
bool CemuleApp::IsEd2kFriendLinkInClipboard()
{
	static const CHAR _szEd2kFriendLink[] = "ed2k://|friend|";
	return IsEd2kLinkInClipboard(_szEd2kFriendLink, ARRSIZE(_szEd2kFriendLink)-1);
}
// Commander - Added: FriendLinks [emulEspaa] - End

void CemuleApp::SetClientIcon(CImageList& imageList){
	imageList.Add(CTempIconLoader(_T("ClientDefault")));		//0
	imageList.Add(CTempIconLoader(_T("ClientDefaultPlus"))); //1
	imageList.Add(CTempIconLoader(_T("ClientCompatible"))); //2
	imageList.Add(CTempIconLoader(_T("ClientCompatiblePlus"))); //3
	// X-Ray :: FileStatusIcons :: Start
	imageList.Add(CTempIconLoader(_T("STATUS_DOWNLOADING"))); //4
	imageList.Add(CTempIconLoader(_T("STATUS_PAUSED"))); //5
	imageList.Add(CTempIconLoader(_T("STATUS_STOPPED"))); //6
	imageList.Add(CTempIconLoader(_T("STATUS_WAITING"))); //7
	imageList.Add(CTempIconLoader(_T("STATUS_ERRONEOUS")));//8
    // X-Ray :: FileStatusIcons :: End
#ifdef CLIENTANALYZER			
	imageList.Add(CTempIconLoader(_T("BADGUY"))); //9
#endif
	imageList.SetOverlayImage(imageList.Add(CTempIconLoader(_T("ClientSecureOvl"))), 1);
	imageList.SetOverlayImage(imageList.Add(CTempIconLoader(_T("OverlayObfu"))), 2);
	imageList.SetOverlayImage(imageList.Add(CTempIconLoader(_T("OverlaySecureObfu"))), 3);
	imageList.SetOverlayImage(imageList.Add(CTempIconLoader(_T("STATUS_PREVIEW"))), 4); //preview icon
	imageList.SetOverlayImage(imageList.Add(CTempIconLoader(_T("SHAREVISIBLE"))), 5); //morph4u share visible
}
