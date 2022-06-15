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

#include <windows.h>
//#include <winsock.h> //<<< eWombat [WINSOCK2]
#include "types.h"
#include "Pinger.h"
// ZZ:UploadSpeedSense -->
#include "LastCommonRouteFinder.h"
// ZZ:UploadSpeedSense <--


typedef struct {
	float avg_ping;
	bool changed;
	float lowest_ping;
	bool lowest_changed;
	bool success;
} pingVal;

class CPingThread: public CWinThread
{
	DECLARE_DYNCREATE(CPingThread)

protected:
	CPingThread(void);

public:
	~CPingThread(void);
	virtual int	 Run();
	float	Ping();
	pingVal GetPing();
	bool	isWorking()	{return works;}
	bool	isTimeout()	{return timeout;}
	bool	isPreparing (bool onlyLowestPing = false)  {return (prepares || !onlyLowestPing && searching);}
	bool    SetPingedServer(CString pingServer);
	CString	GetPingedServer();
	// ZZ:UploadSpeedSense -->
	bool	AddHostsToCheck(CTypedPtrList<CPtrList, CServer*> &list)	{return lastCommonRouteFinder->AddHostsToCheck(list);}
    bool	AddHostsToCheck(CTypedPtrList<CPtrList, CUpDownClient*> &list)	{return lastCommonRouteFinder->AddHostsToCheck(list);}
	// ZZ:UploadSpeedSense <--
	void	StopPingThread(void);
	void	StartPingThread(void);
	void	EndPingThread(void);
	void	Init();
	virtual	BOOL InitInstance() { return true; }

private:
	static const unsigned int m_lowest_ping_values = 3;
	void	findHostToPing();
	void	findLowestPing(bool useURL = false);
	// ZZ:UploadSpeedSense -->
    LastCommonRouteFinder*    lastCommonRouteFinder;
	Pinger*	pinger;
    PingStatus pingStatus;
	// ZZ:UploadSpeedSense <--

	CEvent*		endPingThread;

	float		avg_ping;		// Average ping
	bool		isPingThread;			// is pingThread enabled ?
	bool		works;
	bool		timeout;
	bool		changed;
	bool		lowest_changed;

 	float		lowest_list[m_lowest_ping_values];
	int			lowest_index;
	float		lowest_ping;		// Lowest ping

	LPHOSTENT		pHost;			// Pointer to host entry structure
    struct in_addr	iaDest;			// Internet address structure
	DWORD			dwAddress;		// IP Address
	bool			findHost;
	bool			lowestPingIsFound;
	bool			prepares;
	bool			searching;
	bool			autoHost;
	bool			staticLowestPing;
	//bool			UDPing;
	ePingMode		PingMode;
	bool			noTTL;
	bool			noTTLExec;
	uint32			lastCommonHost;
	uint32			lastCommonTTL;
	uint32			hostToPing;
	uint32			hostToPingNoTTL;
	uint8			failedTimes;
	uint32			lastRestartTime;
	uint16			timeOut;
};

#endif // NEO_BC // NEO: NBC END <-- Xanatos --