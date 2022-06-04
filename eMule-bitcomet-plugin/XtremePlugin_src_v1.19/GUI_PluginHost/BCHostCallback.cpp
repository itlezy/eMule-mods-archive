#include "bitcomet_inc.h"

#include "BCHostCallback.h"

#include "resource.h"		// ������
#include "PluginHostDlg.h"

BCHostCallBack::BCHostCallBack()
: m_hHostWnd(NULL)
{

}

void BCHostCallBack::set_host_wnd(HWND hwnd)
{
	m_hHostWnd = hwnd;
}

void BCHostCallBack::on_notify_plugin_exit(void)
{
	// ��¿���ڻ�BitComet������������������������
	if(m_hHostWnd)
	{
		PostMessage(m_hHostWnd, WM_CLOSE, 0, 0);
	}
}

void BCHostCallBack::on_notify_show_host_dlg(bool show)
{
	if(!CPluginHostDlg::is_debug_mode())
		return;

	if(m_hHostWnd)
	{
		ShowWindow(m_hHostWnd, show ? SW_SHOW : SW_HIDE);
	}
}
