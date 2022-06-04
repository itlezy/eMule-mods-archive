#pragma once

#include "Core_eMule_include/InterfaceEMule.h"

class emuleCallback :
	public Core_eMule::InterfaceEMule_Callback
{
public:
	emuleCallback(void);

	virtual bool is_BCHost_running(void);
	virtual bool is_plugin_host_running(void);

	~emuleCallback(void);
};
