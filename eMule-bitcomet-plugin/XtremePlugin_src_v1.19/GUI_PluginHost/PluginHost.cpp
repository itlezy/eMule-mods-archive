// PluginHost.cpp : 定义应用程序的类行为。
//

#include "bitcomet_inc.h"

#include "Core_Common_include/Win32_IPC.h"
#include "Core_Common_include/InterfaceExceptionDebugger.h"
#include "Core_Common_include/System.h"

#include "LimitSingleInstance.H"
#include "PluginHost.h"
#include "PluginHostDlg.h"
#include "langstring.h"
#include "HostApp.h"
#include "version.h"

using namespace Core_Common;

#ifdef _DEBUG
	//#include "vld/vld.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// The one and only CLimitSingleton object, AutoPtr to ensure destruct the Mutex object when quit.
CAutoPtr<CLimitSingleton> g_PluginHostSingletonPtr;
#define SINGLETON_PLUGINHOST_GUID _T("{SIMPLEBT-53DE14D9-A616-4ff0-BA62-9DF424D0665C}")

//识别操作系统语言
#define LANGID_ZH_CN MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_SIMPLIFIED)
bool is_system_zh_cn(void)
{
	return (::GetUserDefaultLangID() == LANGID_ZH_CN || ::GetACP() == 936);
}

BOOL CALLBACK searchOtherInstance(HWND hWnd, LPARAM lParam) {
	DWORD result;
	LRESULT ok = ::SendMessageTimeout(hWnd, WM_WHERE_ARE_YOU, 0, 0,
		SMTO_BLOCK | SMTO_ABORTIFHUNG, 5000, &result);
	if(ok == 0)
		return TRUE;
	if(result == WM_WHERE_ARE_YOU) {
		// found it
		HWND *target = (HWND *)lParam;
		*target = hWnd;
		return FALSE;
	}
	return TRUE;
}


// 唯一的一个 CPluginHostApp 对象

CPluginHostApp theApp;

// CPluginHostApp

BEGIN_MESSAGE_MAP(CPluginHostApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CPluginHostApp 构造

CPluginHostApp::CPluginHostApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}

// CPluginHostApp 初始化

int AFX_CDECL MFCNewHandler(size_t nSize)
{
	throw std::bad_alloc();
}

extern _PNH AFXAPI AfxSetNewHandler(_PNH pfnNewHandler);

BOOL CPluginHostApp::InitInstance()
{
	AfxSetNewHandler(MFCNewHandler);

	Core_Common::InterfaceExceptionDebugger::set_program_id( _T("em") VERSIONSTRING_BUILD );
	bool is_chinese = (PRIMARYLANGID(Core_Common::System::system_language_id()) == LANG_CHINESE);
	Core_Common::InterfaceExceptionDebugger::set_program_name( is_chinese ? _T("电驴插件") : _T("eMule Plugin") );
	Core_Common::InterfaceExceptionDebugger::set_unhandled_exception_filter();
	boost::function<BOOL(void)> func = boost::bind(&CPluginHostApp::InitInstanceInternal,this);
	return Core_Common::InterfaceExceptionDebugger::call_with_exception_debugger( &func, FALSE );
}

BOOL CPluginHostApp::InitInstanceInternal()
{
	if (!AfxSocketInit())
	{
		AfxMessageBox(_T("Socket init failed."));
		return FALSE;
	}

	AfxEnableControlContainer();

	if (!AfxOleInit())
	{
		AfxMessageBox(_T("OLE init failed."));
		return 0;
	}

	CWinApp::InitInstance();

	//设置系统语言
	if(is_system_zh_cn())
	{
		CLangString::set_lang(CLangString::LANG_ZH);
	}
	else
	{
		CLangString::set_lang(CLangString::LANG_EN);
	}

	//检查运行参数
	bool get_ed2k_cmdline_arg = false;
	if (__argc >= 2 && tstring(__targv[1]).find(_T("://")) != tstring::npos)
	{
		get_ed2k_cmdline_arg = true;
	}

	bool is_host_mode = false;
	if (__argc >= 2 && tstring(__targv[1]) == _T("/host"))
	{
		is_host_mode = true;
	}

	if (__argc >= 2 && !is_host_mode && !get_ed2k_cmdline_arg)
	{
		AfxMessageBox(STRING(S_HELP));
		return FALSE;
	}

	// 只允许一个进程实例
	CLimitSingleton *SingletonPtr = new CLimitSingleton(SINGLETON_PLUGINHOST_GUID);
	if (SingletonPtr->IsAnotherInstanceRunning())
	{
		HWND hwndOther = NULL;
		EnumWindows(searchOtherInstance, (LPARAM)&hwndOther);
		if(hwndOther != NULL )
		{
			if(!is_host_mode)
			{
				// 显示插件窗口
				DWORD result;
				::SendMessageTimeout(hwndOther, WM_WHERE_ARE_YOU, (WPARAM)TRUE, 0,
					SMTO_BLOCK | SMTO_ABORTIFHUNG, 5000, &result);
			}
			if(get_ed2k_cmdline_arg)
			{
				// 打开 ed2k 链接
				tstring ed2k_link = tstring(__targv[1]);
				COPYDATASTRUCT sendstruct;
				sendstruct.cbData = (ed2k_link.size() + 1)*sizeof(TCHAR);
				sendstruct.dwData = 0;
				sendstruct.lpData = const_cast<LPTSTR>((LPCTSTR)ed2k_link.data());
				if (hwndOther){
					SendMessage(hwndOther, WM_COPYDATA, (WPARAM)0, (LPARAM)(PCOPYDATASTRUCT)&sendstruct);
					return true;
				}
			}
		}

		set_startup_event();

		safe_delete(SingletonPtr);
		return FALSE;
	}
	else
	{
		g_PluginHostSingletonPtr.Attach(SingletonPtr);
	}

	HostApp::NewInstance();

	CPluginHostDlg* pDlg = new CPluginHostDlg;

	if(get_ed2k_cmdline_arg)
	{
		pDlg->m_strPendingLink = tstring(__targv[1]);
	}

	pDlg->Create(CPluginHostDlg::IDD);
	m_pMainWnd = pDlg;

	bool is_show_host_wnd = pDlg->is_show_host_wnd();
	if(is_show_host_wnd)
	{
		pDlg->ShowWindow(SW_SHOW);
	}

	return TRUE;
}

int CPluginHostApp::ExitInstance()
{
	CPluginHostDlg* pDlg = (CPluginHostDlg*)m_pMainWnd;
	if(pDlg)
	{
		pDlg->DestroyWindow();
		safe_delete(pDlg);
		//pDlg = NULL; // CFrameWnd delete this in PostNcDestroy()
		m_pMainWnd = NULL;
	}

	if(HostApp::HasInstance())
	{
		HostApp::DeleteInstance();
	}

	return CWinApp::ExitInstance();
}

void CPluginHostApp::set_startup_event()
{
	#define EVENT_NAME_PLUGIN_PROCESS_CREATE _T("BitCometPluginHostProcessCreated")
	
	Core_Common::Adapt_Win32_Event hEventProcessCreate;
	hEventProcessCreate.create(false, false, EVENT_NAME_PLUGIN_PROCESS_CREATE);
	SetEvent(hEventProcessCreate);
}

int CPluginHostApp::Run()
{
	Core_Common::InterfaceExceptionDebugger::set_unhandled_exception_filter();

	boost::function<int(void)> func = boost::bind(&CPluginHostApp::RunInternal,this);
	return Core_Common::InterfaceExceptionDebugger::call_with_exception_debugger( &func, int(0) );

	//return CWinApp::Run();
}

int CPluginHostApp::RunInternal()
{
	return CWinApp::Run();
}
