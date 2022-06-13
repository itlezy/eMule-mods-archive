// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#define _SECURE_SCL 0 //Fix by Anis -> evitiamo che il mulo crashi solo per una divisione per 0 o per un errore "stupido" in una custom string

//Anis
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }

#include "emule_site_config.h"

// MSDN: Using the Windows Headers
// ===========================================================
//Windows Vista			_WIN32_WINNT>=0x0600	WINVER>=0x0600
//Windows Server 2003	_WIN32_WINNT>=0x0502    WINVER>=0x0502
//Windows XP			_WIN32_WINNT>=0x0501	WINVER>=0x0501
//Windows 2000			_WIN32_WINNT>=0x0500    WINVER>=0x0500
//Windows NT 4.0		_WIN32_WINNT>=0x0400	WINVER>=0x0400
//Windows Me			_WIN32_WINDOWS=0x0500	WINVER>=0x0500
//Windows 98			_WIN32_WINDOWS>=0x0410	WINVER>=0x0410
//Windows 95			_WIN32_WINDOWS>=0x0400	WINVER>=0x0400
//
//IE 7.0				_WIN32_IE>=0x0700 
//IE 6.0 SP2			_WIN32_IE>=0x0603 
//IE 6.0 SP1			_WIN32_IE>=0x0601 
//IE 6.0				_WIN32_IE>=0x0600 
//IE 5.5				_WIN32_IE>=0x0550 
//IE 5.01				_WIN32_IE>=0x0501 
//IE 5.0, 5.0a, 5.0b	_WIN32_IE>=0x0500 
//IE 4.01				_WIN32_IE>=0x0401 
//IE 4.0				_WIN32_IE>=0x0400 
//IE 3.0, 3.01, 3.02	_WIN32_IE>=0x0300 

#if defined(HAVE_VISTA_SDK)

#ifndef WINVER
#define WINVER 0x0502			// 0x0502 == Windows Server 2003, Windows XP (same as VS2005-MFC)
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT WINVER		// same as VS2005-MFC
#endif

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410	// 0x0410 == Windows 98
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0560		// 0x0560 == Internet Explorer 5.6 -> Comctl32.dll v5.8
#endif

#else//HAVE_VISTA_SDK

#ifndef WINVER
#define WINVER 0x0400			// 0x0400 == Windows 98 and Windows NT 4.0 (because of '_WIN32_WINDOWS=0x0410')
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400		// 0x0400 == Windows NT 4.0
#endif

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410	// 0x0410 == Windows 98
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0560		// 0x0560 == Internet Explorer 5.6 -> Comctl32.dll v5.8 (same as MFC internally used)
#endif

#endif//HAVE_VISTA_SDK

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// Makes certain CString constructors explicit, preventing any unintentional conversions
#define	_ATL_EX_CONVERSION_MACROS_ONLY		// Disable old ATL 3.0 string conversion macros
#ifdef _ATL_EX_CONVERSION_MACROS_ONLY
#define CharNextO CharNextW					// work around a bug in ATL headers
#endif//_ATL_EX_CONVERSION_MACROS_ONLY
#define _CONVERSION_DONT_USE_THREAD_LOCALE	// for consistency with C-RTL/MFC the ATL thread locale support has to get disabled
#define _ATL_NO_COM_SUPPORT
#define _ATL_NO_PERF_SUPPORT
#define	_ATL_NO_COMMODULE
#define _ATL_NO_CONNECTION_POINTS
#define _ATL_NO_DOCHOSTUIHANDLER
#define _ATL_NO_HOSTING

#define _ATL_ALL_WARNINGS
#define _AFX_ALL_WARNINGS

#if _MSC_VER>=1400

// _CRT_SECURE_NO_DEPRECATE - Disable all warnings for not using "_s" functions.
//
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

// _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES - Overloads all standard string functions (e.g. strcpy) with "_s" functions
// if, and only if, the size of the output buffer is known at compile time (so, if it is a static array). If there is
// a buffer overflow during runtime, it will throw an exception.
//
#ifndef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#endif

// _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT - This is a cool CRT feature but does not make sense for our code.
// With our existing code we could get exceptions which are though not justifiable because we explicitly
// terminate all our string buffers. This define could be enabled for debug builds though.
//
//#ifndef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT
//#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT 1
//#endif

#if !defined(_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES) || (_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES==0)
#ifndef _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_NON_CONFORMING_SWPRINTFS
#endif
#endif//!defined(_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES) || (_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES==0)

#endif//_MSC_VER>=1400

#ifdef _DEBUG
#define _ATL_DEBUG
#define _ATL_DEBUG_QI
#endif

#include <afxwin.h>			// MFC core and standard components
#include <afxext.h>			// MFC extensions
#include <afxdtctl.h>		// MFC support for 'CDateTimeCtrl' and 'CMonthCalCtrl'
#include <afxcmn.h>			// MFC support for Windows Common Controls
#include <afxole.h>			// MFC OLE support
#include <winsock2.h>
#define _WINSOCKAPI_
#include <afxsock.h>		// MFC support for Windows Sockets
#include <afxdhtml.h>
#include <afxmt.h>			// MFC Multithreaded Extensions (Syncronization Objects)
#include <afxdlgs.h>		// MFC Standard dialogs
#include <..\src\mfc\afximpl.h>
#include <atlcoll.h>
#include <afxcoll.h>
#include <afxtempl.h>
#include <math.h>


#ifndef EWX_FORCEIFHUNG
#define EWX_FORCEIFHUNG			0x00000010
#endif

#ifndef WS_EX_LAYOUTRTL
#define WS_EX_LAYOUTRTL			0x00400000L // Right to left mirroring
#endif

#ifndef LAYOUT_RTL
#define LAYOUT_RTL				0x00000001 // Right to left
#endif

#ifndef COLOR_HOTLIGHT
#define COLOR_HOTLIGHT			26
#endif

#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED			0x00080000
#endif

#ifndef LWA_COLORKEY
#define LWA_COLORKEY			0x00000001
#endif

#ifndef LWA_ALPHA
#define LWA_ALPHA				0x00000002
#endif

#ifndef HDF_SORTUP
#define HDF_SORTUP				0x0400
#endif

#ifndef HDF_SORTDOWN
#define HDF_SORTDOWN			0x0200
#endif

#ifndef COLOR_GRADIENTACTIVECAPTION
#define COLOR_GRADIENTACTIVECAPTION 27
#endif

#ifndef LVBKIF_TYPE_WATERMARK
#define LVBKIF_TYPE_WATERMARK   0x10000000
#endif

#ifndef LVBKIF_FLAG_ALPHABLEND
#define LVBKIF_FLAG_ALPHABLEND  0x20000000
#endif

#include "types.h"

#define ARRSIZE(x)	(sizeof(x)/sizeof(x[0]))

#ifdef _DEBUG
#define malloc(s)		  _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define calloc(c, s)	  _calloc_dbg(c, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define realloc(p, s)	  _realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define _expand(p, s)	  _expand_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define free(p)			  _free_dbg(p, _NORMAL_BLOCK)
#define _msize(p)		  _msize_dbg(p, _NORMAL_BLOCK)
#endif

typedef CArray<CStringA> CStringAArray;
typedef CStringArray CStringWArray;

#define _TWINAPI(fname)	fname "W"

extern "C" int __cdecl __ascii_stricmp(const char * dst, const char * src);

inline BOOL afxIsWin95()
{
#if _MFC_VER>=0x0900
	return FALSE;
#else
	return afxData.bWin95;
#endif
}
