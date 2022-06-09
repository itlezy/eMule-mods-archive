// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

// Change these values to use different versions
#define WINVER			0x0400
#define _WIN32_WINNT	0x0400
#define _WIN32_IE		0x0500

/*
#define _RICHEDIT_VER	0x0100
#define _WTL_USE_CSTRING

#include <atlbase.h>
#include <atlapp.h>

extern CAppModule _Module;

#include <atlwin.h>

#include <atlmisc.h>

#define ASSERT ATLASSERT
#define VERIFY ATLVERIFY
#define TRACE ATLTRACE
#define TRACE2 ATLTRACE2
*/
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit
#define _AFX_ALL_WARNINGS	// turns off MFC's hiding of some common and often safely ignored warning messages
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include <afxmt.h>
#include <afxpriv.h>
#include <afxtempl.h>

//////////////////////////
#define NEW_SOCKETS
#define NEW_SOCKETS_ENGINE
#define SUPPORT_CLIENT_PEER

#include <atlbase.h>
#include <comdef.h>
#include <locale.h>


#if defined(_DEBUG)
#define EMULE_TRY
#define EMULE_CATCH
#define EMULE_CATCH2
#else
#define EMULE_TRY try{
#define EMULE_CATCH } catch(...){}
/*	catch(CString &str){ try{g_eMuleApp.m_pdlgEmule->AddBugReport(__FUNCTION__,__FILE__, __LINE__,str);}catch(...){} }\
	catch(CException *err){ try{g_eMuleApp.m_pdlgEmule->AddBugReport(__FUNCTION__,__FILE__, __LINE__,GetErrorMessage(err));err->Delete();}catch(...){} }\
	catch(...){ try{g_eMuleApp.m_pdlgEmule->AddBugReport(__FUNCTION__,__FILE__, __LINE__,_T(""));}catch(...){} }*/
#define EMULE_CATCH2 } catch(...){}
#endif

//#define GetResString(id)  CString(_T("NOT IMPLEMENTED YET"))

#include <winsock2.h>
#include <ws2tcpip.h>
#define _WINSOCKAPI_
/*
#include "Other/atlwfile.h"

#include <atlcrack.h>
#include <atlddx.h>
*/
#define PROCESS_OPCODES

#include "EmEngine.h"
extern CEmEngine g_stEngine;
