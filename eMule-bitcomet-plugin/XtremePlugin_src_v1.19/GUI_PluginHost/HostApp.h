// PluginHost.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#include "Core_Common_include/Singleton.h"

// HostApp:
// �йش����ʵ�֣������ PluginHost.cpp
//

class HostApp : public Core_Common::Singleton<HostApp>
{
public:
	HostApp();
	~HostApp();

};

