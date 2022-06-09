// Engine.cpp : main source file for Engine.exe
//

#include "stdafx.h"
/*
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>

CAppModule _Module;
*/
#include "resource.h"

#include "EmEngine.h"

CEmEngine g_stEngine;
/*
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int Run(LPTSTR lpstrCmdLine = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	if(!g_stEngine.Init(NULL))
		return -1;

	int nRet = theLoop.Run();

	g_stEngine.Uninit();

	_Module.RemoveMessageLoop();
	return nRet;
}
*/
int Run()
{
	BOOL bRet;
	MSG msg;

	for(;;)
	{
		bRet = ::GetMessage(&msg, NULL, 0, 0);

		if(bRet == -1)
			continue;   // error, don't process
		else if(!bRet)
			break;   // WM_QUIT, exit message loop

		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpstrCmdLine, int nCmdShow)
{
	HRESULT hRes = ::CoInitialize(NULL);

//	If you are running on NT 4.0 or higher you can use the following call instead to 
//	make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ASSERT(SUCCEEDED(hRes));

/*
//	This resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
//	(I thunk, he thank, we're gethunken?)
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// Add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
*/
	if(!g_stEngine.Init())
		return -1;
	int nRet = Run();
	g_stEngine.Uninit();

	::CoUninitialize();

	return nRet;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
