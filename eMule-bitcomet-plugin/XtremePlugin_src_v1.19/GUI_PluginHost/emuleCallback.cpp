#include "bitcomet_inc.h"

#include "emuleCallback.h"
#include "LimitSingleInstance.h"

emuleCallback::emuleCallback(void)
{
}

emuleCallback::~emuleCallback(void)
{
}

bool emuleCallback::is_BCHost_running()
{
	#define SINGLETON_CLIENT_GUID _T("{SIMPLEBT-D19EACFB-5FD1-4615-A179-A9B9E38A6506}")

	CLimitSingleton bchost_singleton(SINGLETON_CLIENT_GUID);
	return bchost_singleton.IsAnotherInstanceRunning();	
}

bool emuleCallback::is_plugin_host_running()
{
	#define SINGLETON_PLUGINHOST_GUID _T("{SIMPLEBT-53DE14D9-A616-4ff0-BA62-9DF424D0665C}")
	
	CLimitSingleton player_singleton(SINGLETON_PLUGINHOST_GUID);
	return player_singleton.IsAnotherInstanceRunning();	
}