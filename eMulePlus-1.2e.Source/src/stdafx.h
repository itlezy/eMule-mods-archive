// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently
#pragma once

// Define special compilation for Visual Studio 7 (2002) when detected
#if _MSC_VER == 1300
#define VS2002
#elif _MSC_VER < 1300
#error Can only be compiled in VS.NET environment
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0400		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0400		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 4.0 or later.
#define _WIN32_IE 0x0400	// Change this to the appropriate value to target IE 5.0 or later.
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif

#include <winsock2.h>
#pragma warning(push)	// preserve current state
#pragma warning(disable : 4127 4706)	// conditional expression is constant, assignment within conditional expression
#include <ws2tcpip.h>
#pragma warning(pop)
#define _WINSOCKAPI_

#ifndef NEW_SOCKETS
#define OLD_SOCKETS_ENABLED
#endif //NEW_SOCKETS

//#ifdef OLD_SOCKETS_ENABLED
#include <afxsock.h>		// MFC support for Windows Sockets
//#endif //OLD_SOCKETS_ENABLED
#include <afxdhtml.h>

#include <afxmt.h>		// MFC Multithreaded Extensions (Syncronization Objects)
#include <afxdlgs.h>
#include <..\src\mfc\afximpl.h>
#include <atlcoll.h>
#include <afxcoll.h>
#include <afxtempl.h>
#include <math.h>

// If you compile interim release, uncomment #define _INTERIM_RELEASE
// This way it will crash for beta-testers and we'll know where it happened
//#define _INTERIM_RELEASE

#if defined(_INTERIM_RELEASE) || defined(_DEBUG)
#define EMULE_TRY
#define EMULE_CATCH
#define EMULE_CATCH2
#else
#define EMULE_TRY try{
#define EMULE_CATCH }\
catch(const CString &str){ try{g_App.m_pMDlg->AddBugReport(_T(__FUNCTION__), _T(__FILE__), __LINE__, str);}catch(...){} }\
catch(CException *err){ try{g_App.m_pMDlg->AddBugReport(_T(__FUNCTION__), _T(__FILE__), __LINE__, GetErrorMessage(err));err->Delete();}catch(...){} }\
catch(...){ try{g_App.m_pMDlg->AddBugReport(_T(__FUNCTION__), _T(__FILE__), __LINE__, _T(""));}catch(...){} }
#define EMULE_CATCH2 } catch(...){}
#endif

#ifdef NEW_SOCKETS
#include <comdef.h>

#define PCTSTR LPCTSTR
#define PCVOID LPCVOID
#ifndef _countof
#	define _countof(string) (sizeof((string)) / sizeof((string)[0]))
#endif // _countof

enum T_CLIENT_TYPE {
	T_CLIENT_PEER,
	T_CLIENT_SERVER,
	T_CLIENT_WEB
};
#endif //NEW_SOCKETS

#include "types.h"

#define ARRSIZE(x)			(sizeof(x) / sizeof(x[0]))
//	Suppresses Unreferenced Parameter warning
#define NOPRM				UNREFERENCED_PARAMETER
//	Length of the constant string (Unicode or ASCII)
#define CSTRLEN(str)		(sizeof(str) / sizeof(*(str)) - 1u)
//	Fast conversion from BOOL into bool type (assuming that BOOL has only standard values)
#define B2b(b)				static_cast<bool>((b) & 1)
