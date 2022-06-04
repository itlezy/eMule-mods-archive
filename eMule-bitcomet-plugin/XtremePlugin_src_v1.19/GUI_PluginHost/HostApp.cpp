// HostApp.cpp : 定义应用程序的类行为。
//

#include "bitcomet_inc.h"
#include "HostApp.h"

#include "Core_eMule_include/InterfaceEMulePlugin.h"
#include "Core_IPC_include/InterfaceIPC.h"

#include "Core_Common_include/string_path.h"
#include "Core_Common_include/FileSystem.h"
#include "Core_Common_include/System.h"
#include "Core_Common_include/InterfaceMessage.h"
#include "Core_Common_include/InterfaceTimer.h"
#include "Core_Common_include/InterfaceLog.h"
using namespace Core_Common;

#include "emuleCallback.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// {DFA820AE-3BE0-4220-A088-B22A6514F268}
const Core_IPC::DAEMON_ID s_PluginHost_Daemon_ID = {0xdfa820ae, 0x3be0, 0x4220, 0xa0, 0x88, 0xb2, 0x2a, 0x65, 0x14, 0xf2, 0x68};


// HostApp 构造

HostApp::HostApp()
{
	// set ipc deamon id
	Core_IPC::InterfaceIPC::set_deamon_id(s_PluginHost_Daemon_ID);

	static emuleCallback emule_callback;
	Core_eMule::InterfaceEMule::m_emule_callback = &emule_callback;

	// 初始化各模块
	InterfaceLog::init();
	InterfaceMessage::init();
	InterfaceTimer::init();
	Core_IPC::InterfaceIPC::init();
	Core_eMule::InterfaceEMulePlugin::init();

	InterfaceLog::start();
	InterfaceMessage::start();
	InterfaceTimer::start();
	Core_IPC::InterfaceIPC::start();
	Core_eMule::InterfaceEMulePlugin::start();

	// log 模块
#ifdef _DEBUG
	static Core_Common::InterfaceLogCallbackWindow s_log_window;
	Core_Common::InterfaceLog::add_callback(&s_log_window);
	static Core_Common::InterfaceLogCallbackFile s_log_file( (System::system_app_path() / _T("pluginhost.txt") ).get_string() );
	Core_Common::InterfaceLog::add_callback(&s_log_file);
#endif

}

HostApp::~HostApp()
{
	// 退出各模块
	Core_eMule::InterfaceEMulePlugin::stop();
	Core_IPC::InterfaceIPC::stop();
	InterfaceMessage::stop();
	InterfaceTimer::stop();
	InterfaceLog::stop();

	Core_eMule::InterfaceEMulePlugin::release();
	Core_IPC::InterfaceIPC::release();
	InterfaceMessage::release();
	InterfaceTimer::release();
	InterfaceLog::release();
}


