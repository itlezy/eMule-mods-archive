// ExceptionAttacher.cpp  Version 1.1
//
// Copyright ?1998 Bruce Dawson
//
// This module contains the boilerplate code that you need to add to any
// MFC app in order to wrap the main thread in an exception handler.
// AfxWinMain() in this source file replaces the AfxWinMain() in the MFC
// library.
//
// Author:       Bruce Dawson
//               brucedawson@cygnus-software.com
//
// Modified by:  Hans Dietrich
//               hdietrich2@hotmail.com
//
// A paper by the original author can be found at:
//     http://www.cygnus-software.com/papers/release_debugging.html
//
///////////////////////////////////////////////////////////////////////////////
// modified by rnysmile, works in vs.net unicode build
// 

#include "stdafx.h"
//#include "../version.h"
//#include "../BuildNumber.h"


#include "ExceptionHandler.h"

//
// Kinda new at this ... but I had xcrash ( awesome btw ) running and catching my boo
// boos but when the exception was in a seperate thread of the application, the default was still occurring.
// I added the following code to ExceptionAttacher.cpp and made it call RecordExceptionInfo.

//LONG WINAPI CustomUnhandledExceptionFilter(PEXCEPTION_POINTERS pExInfo)
//{
//	RecordExceptionInfo(pExInfo, _T("catched in CustomUnhandledExceptionFilter()") );
//
//	::TerminateProcess(GetCurrentProcess(), 100);
//
//	return EXCEPTION_EXECUTE_HANDLER;
//}
/*
extern int AFXAPI AfxWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPTSTR lpCmdLine, int nCmdShow);

extern "C" int WINAPI
_tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPTSTR lpCmdLine, int nCmdShow)
{
		return AfxWinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}

#ifdef AFX_CORE1_SEG
#pragma code_seg(AFX_CORE1_SEG)
#endif

// This is the initial entry point into an MFC app. Normally this is in the
// MFC library:  mfc\src\winmain.cpp

int AFXAPI AfxWinMain(HINSTANCE hInstance, 
					  HINSTANCE hPrevInstance,	
					  LPTSTR lpCmdLine, 
					  int nCmdShow)
{
	// Wrap WinMain in a structured exception handler (different from C++
	// exception handling) in order to make sure that all access violations
	// and other exceptions are displayed - regardless of when they happen.
	// This should be done for each thread, if at all possible, so that exceptions
	// will be reliably caught, even inside the debugger.
	__try
	{
		TRACE(_T("in ExceptionAttacher.cpp - AfxWinMain\n"));

		// The code inside the __try block is the MFC version of AfxWinMain(),
		// copied verbatim from the MFC source code.
		bcassert(hPrevInstance == NULL);
		
		int nReturnCode = -1;
		CWinThread* pThread = AfxGetThread();
		CWinApp* pApp = AfxGetApp();

		// AFX internal initialization
		if (!AfxWinInit(hInstance, hPrevInstance, lpCmdLine, nCmdShow))
			goto InitFailure;

		// App global initializations (rare)
		if (pApp != NULL && !pApp->InitApplication())
			goto InitFailure;

		// Perform specific initializations
		if (!pThread->InitInstance())
		{
			if (pThread->m_pMainWnd != NULL)
			{
				TRACE(traceAppMsg, 0, "Warning: Destroying non-NULL m_pMainWnd\n");
				pThread->m_pMainWnd->DestroyWindow();
			}
			nReturnCode = pThread->ExitInstance();
			goto InitFailure;
		}
	
		::SetUnhandledExceptionFilter(CustomUnhandledExceptionFilter);

		nReturnCode = pThread->Run();

InitFailure:
	#ifdef _DEBUG
		// Check for missing AfxLockTempMap calls
		if (AfxGetModuleThreadState()->m_nTempMapLock != 0)
		{
			TRACE(traceAppMsg, 0, "Warning: Temp map lock count non-zero (%ld).\n",
				AfxGetModuleThreadState()->m_nTempMapLock);
		}
		AfxLockTempMaps();
		AfxUnlockTempMaps(-1);
	#endif

		AfxWinTerm();
		return nReturnCode;
	}
	__except(RecordExceptionInfo(GetExceptionInformation(), 
				_T("ExceptionAttacher.cpp - AfxWinMain")))
	{
		// Do nothing here - RecordExceptionInfo() has already done
		// everything that is needed. Actually this code won't even
		// get called unless you return EXCEPTION_EXECUTE_HANDLER from
		// the __except clause.
	}
	return 0;
}
//*/

//void __cdecl CustomInvalidParameterHandler(const wchar_t* expression,
//							   const wchar_t* function, 
//							   const wchar_t* file, 
//							   unsigned int line, 
//							   uintptr_t pReserved)
//{
//	//wprintf(L"Invalid parameter detected in function %s."
//	//	L" File: %s Line: %d\n", function, file, line);
//	//wprintf(L"Expression: %s\n", expression);
//
//	throw NULL;
//}


