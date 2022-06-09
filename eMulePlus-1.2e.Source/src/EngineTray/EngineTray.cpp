// EngineTray.cpp : main source file for EngineTray.exe
//

#include "stdafx.h"

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>

#include "resource.h"

CAppModule _Module;

CEmEngine g_stEngine;

#include "GUI/MainDlg.h"

#include "WebServer/WebServerImpl.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainDlg dlgMain;

	if(dlgMain.Create(NULL) == NULL)
	{
		ATLTRACE(_T("Main dialog creation failed!\n"));
		return 0;
	}

	dlgMain.ShowWindow(SW_HIDE/*nCmdShow*/);

	CWebServerImpl* pWebServer = new CWebServerImpl;
	pWebServer->Start();

	int nRet = theLoop.Run();

	pWebServer->Stop();
	delete pWebServer;

	_Module.RemoveMessageLoop();

	return nRet;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	HRESULT hRes = ::CoInitialize(NULL);
//	If you are running on NT 4.0 or higher you can use the following call instead to 
//	make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

//	This resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
//	(I thunk, he thank, we're gethunken?)
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// Add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
	::CoUninitialize();

	return nRet;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
