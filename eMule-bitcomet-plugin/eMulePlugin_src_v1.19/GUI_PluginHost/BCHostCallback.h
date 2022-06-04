#pragma once

#include "Core_eMule_include/InterfaceEMulePlugin.h"

class BCHostCallBack : public Core_eMule::InterfaceEMulePlugin_CallBack
{
public:
	virtual void on_notify_plugin_exit(void);
	virtual void on_notify_show_host_dlg(bool show);

public:
	BCHostCallBack();

	void set_host_wnd(HWND hwnd);

protected:
	HWND m_hHostWnd;
};

