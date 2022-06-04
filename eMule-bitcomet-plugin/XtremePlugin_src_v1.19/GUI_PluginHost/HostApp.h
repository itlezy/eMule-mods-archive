// PluginHost.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#include "Core_Common_include/Singleton.h"

// HostApp:
// 有关此类的实现，请参阅 PluginHost.cpp
//

class HostApp : public Core_Common::Singleton<HostApp>
{
public:
	HostApp();
	~HostApp();

};

