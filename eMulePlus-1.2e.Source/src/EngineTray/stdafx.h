// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

// Change these values to use different versions
#define WINVER			0x0400
#define _WIN32_WINNT	0x0400
#define _WIN32_IE		0x0500
#define _RICHEDIT_VER	0x0100

#define _WTL_USE_CSTRING

#include <atlbase.h>
#include <atlapp.h>

extern CAppModule _Module;

#include <atlwin.h>

#include <atlmisc.h>

//////////////////////////
#define NEW_SOCKETS
#define NEW_SOCKETS_ENGINE
#define NEW_SOCKETS_TRAY

#define ASSERT ATLASSERT
#define VERIFY ATLVERIFY
#define TRACE ATLTRACE
#define TRACE2 ATLTRACE2

#include <comdef.h>

#if defined(_DEBUG)
#define EMULE_TRY
#define EMULE_CATCH
#define EMULE_CATCH2
#else
#define EMULE_TRY try{
#define EMULE_CATCH }\
/*	catch(CString &str){ try{g_eMuleApp.m_pdlgEmule->AddBugReport(__FUNCTION__,__FILE__, __LINE__,str);}catch(...){} }\
	catch(CException *err){ try{g_eMuleApp.m_pdlgEmule->AddBugReport(__FUNCTION__,__FILE__, __LINE__,GetErrorMessage(err));err->Delete();}catch(...){} }*/\
	catch(...){ /*try{g_eMuleApp.m_pdlgEmule->AddBugReport(__FUNCTION__,__FILE__, __LINE__,_T(""));}catch(...){}*/ }
#define EMULE_CATCH2 } catch(...){}
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#define _WINSOCKAPI_

#include "../Engine/Other/atlwfile.h"

#include <atlcrack.h>
#include <atlddx.h>
#include <atlsync.h>

typedef struct
{
	BYTE		hash[16];
} HashType;


const CString DEF_ADDR = _T("127.0.0.1");
const UINT DEF_PORT = 9090;

#include "EmEngine.h"
extern CEmEngine g_stEngine;
