// PluginHost.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号

// CPluginHostApp:
// 有关此类的实现，请参阅 PluginHost.cpp
//

class CPluginHostApp : public CWinApp
{
public:
	CPluginHostApp();

// 重写
	public:
	virtual BOOL InitInstance();
	BOOL InitInstanceInternal();
	virtual int ExitInstance();

	void set_startup_event();

// 实现

	DECLARE_MESSAGE_MAP()

	virtual int Run();
	int RunInternal();
};

extern CPluginHostApp theApp;