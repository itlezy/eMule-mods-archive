//this file is part of NeoMule
//Copyright (C)2006-2008 David Xanatos ( XanatosDavid@googlemail.com / http://NeoMule.sf.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "stdafx.h"

#include "SystemInfo.h"
#include "emule.h"
#include "emuledlg.h"

DWORD tmp_cpu_usage;
UINT AFX_CDECL cpu_reader_run(LPVOID /*lpParam*/)
{
	while(theApp.emuledlg->IsRunning()){
		Sleep(500);

		static CList<int,int> average;
		average.AddTail(theApp.sysinfo->ReadCPUUsage());
		if(average.GetCount()>6)
			average.RemoveHead();
		int total = 0;
		for(POSITION pos = average.GetHeadPosition(); pos != NULL; total += average.GetNext(pos));

		tmp_cpu_usage = total/average.GetCount();
	}
	return 0;
}

CSystemInfo::CSystemInfo(){
	CSysInfo::Init();
	AfxBeginThread(cpu_reader_run, NULL,THREAD_PRIORITY_ABOVE_NORMAL);
}

CSystemInfo::~CSystemInfo(){
}

int CSystemInfo::GetCpuUsage(){
	return tmp_cpu_usage;
	//return CPUUsageReader::ReadCPUUsage();
}

DWORD CSystemInfo::GetMemoryUsage(){
	return CSysInfo::GetProcessMemoryUsage();
}