// PluginHost.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������

// CPluginHostApp:
// �йش����ʵ�֣������ PluginHost.cpp
//

class CPluginHostApp : public CWinApp
{
public:
	CPluginHostApp();

// ��д
	public:
	virtual BOOL InitInstance();
	BOOL InitInstanceInternal();
	virtual int ExitInstance();

	void set_startup_event();

// ʵ��

	DECLARE_MESSAGE_MAP()

	virtual int Run();
	int RunInternal();
};

extern CPluginHostApp theApp;