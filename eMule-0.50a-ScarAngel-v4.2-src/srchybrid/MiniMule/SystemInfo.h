// modified by Stulle
#pragma once

#include "SysInfo.h"
#include "CPU.h"

class CSystemInfo : public CSysInfo
{
public:
	CSystemInfo();
	~CSystemInfo();

	int		GetCpuUsage();
	uint32	GetMemoryUsage();
	int		GetGlobalCpuUsage();
	uint64	GetGlobalMemoryUsage();

private:
	CPU cpu;
    int sys;
    TKTime upTime;
};
