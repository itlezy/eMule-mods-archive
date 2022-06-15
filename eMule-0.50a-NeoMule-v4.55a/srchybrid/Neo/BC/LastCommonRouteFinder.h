//this file is part of NeoMule
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

#pragma once

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->

class CServer;
class CUpDownClient;

class LastCommonRouteFinder 
{
public:
    LastCommonRouteFinder();
    ~LastCommonRouteFinder();

    bool Find();
	bool AddHostToCheck(CString pingServer);
    bool AddHostsToCheck(CTypedPtrList<CPtrList, CServer*> &list);
    bool AddHostsToCheck(CTypedPtrList<CPtrList, CUpDownClient*> &list);

	uint32 GetHostToPing() {return hostToPing;}
	uint32 GetHostToPingNoTTL() {return hostToPingNoTTL;} // Add-on
	uint32 GetLastCommonHost()	{return lastCommonHost;}
	uint32 GetLastCommonTTL() {return lastCommonTTL;}
	void   StopSearching() {run=false; newTraceRouteHostEvent->SetEvent();}
private:
	uint32 lastCommonHost;
	uint32 lastCommonTTL;
	uint32 hostToPing;
	uint32 hostToPingNoTTL; // Add-on

	bool needMoreHosts;
	bool run;
    CCriticalSection addHostLocker;

    CEvent* newTraceRouteHostEvent;

    CList<uint32,uint32> hostsToTraceRoute;
};

#endif // NEO_BC // NEO: NBC END <-- Xanatos --