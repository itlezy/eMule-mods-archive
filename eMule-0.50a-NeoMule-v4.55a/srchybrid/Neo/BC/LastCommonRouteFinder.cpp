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


#include "stdafx.h"
#include "emule.h"
#include "Opcodes.h"
#include "LastCommonRouteFinder.h"
#include "Server.h"
#include "OtherFunctions.h"
#include "UpDownClient.h"
#include "Preferences.h"
#include "Pinger.h"
#include "emuledlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->

LastCommonRouteFinder::LastCommonRouteFinder() {
	lastCommonHost = 0;
	lastCommonTTL = 0;
	hostToPing = 0;
	hostToPingNoTTL = 0; // Add-on

	needMoreHosts = true; //false;
	run = true;

	newTraceRouteHostEvent = new CEvent(0, 0);

}

LastCommonRouteFinder::~LastCommonRouteFinder() {
	delete newTraceRouteHostEvent;
}

bool LastCommonRouteFinder::AddHostToCheck(CString pingServer) {
	if(needMoreHosts) {
		addHostLocker.Lock();

		if(needMoreHosts) {
			// Lookup destination
			// Use inet_addr() to determine if we're dealing with a name
			// or an address
			if (pingServer==_T("")){
				addHostLocker.Unlock();
				return false; // didn't get enough hosts
			}

			LPHOSTENT		pHost;			// Pointer to host entry structure
			struct in_addr	iaDest;			// Internet address structure
			DWORD			dwAddress;		// IP Address

			iaDest.s_addr = inet_addr(CT2CA(pingServer));

			if (iaDest.s_addr == INADDR_NONE) {
				pHost = gethostbyname(CT2CA(pingServer));
			} else {
				pHost = gethostbyaddr((const char*)&iaDest, sizeof(struct in_addr), AF_INET);
			}

			if (pHost == NULL) {
				theApp.QueueDebugLogLine(false, _T("\n%s not found\n"), pingServer);
				addHostLocker.Unlock();
				return false; // didn't get enough hosts
			}

			dwAddress = *(DWORD*)(*pHost->h_addr_list);

			//if(!IsGoodIP(dwAddress, true)){
			//	addHostLocker.Unlock();
			//	return false; // didn't get enough hosts
			//}

			hostsToTraceRoute.AddHead(dwAddress);

			// Signal that there's hosts to fetch.
			newTraceRouteHostEvent->SetEvent();

			addHostLocker.Unlock();
			return true; // got enough hosts
		} else {
			addHostLocker.Unlock();
			return true; // allready got enough hosts, don't need more
		}
	} else {
		return true; // allready got enough hosts, don't need more
	}
}

bool LastCommonRouteFinder::AddHostsToCheck(CTypedPtrList<CPtrList, CServer*> &list) {
	if(needMoreHosts) {
		addHostLocker.Lock();

		if(needMoreHosts) {
			if(list.GetCount() >= 10) {
				hostsToTraceRoute.RemoveAll();

				uint32 startPos = rand()/(RAND_MAX/list.GetCount());

				POSITION pos = list.GetHeadPosition();
				for(uint32 skipCounter = startPos; skipCounter < (uint32)list.GetCount() && pos != NULL; skipCounter++) {
					list.GetNext(pos);
				}

				uint32 tryCount = 0;
				while(pos != NULL && hostsToTraceRoute.GetCount() < 10 && tryCount <= (uint32)list.GetCount()) {
					tryCount++;
					CServer* server = list.GetNext(pos);

					uint32 ip = server->GetIP();

					if(IsGoodIP(ip, true)) {
						hostsToTraceRoute.AddTail(ip);
					}

					//                    if(pos == NULL) {
					//                        POSITION pos = list.GetHeadPosition();
					//                    }
				}
			}

			if(hostsToTraceRoute.GetCount() >= 10 || !run) {
				needMoreHosts = false;

				// Signal that there's hosts to fetch.
				newTraceRouteHostEvent->SetEvent();

				addHostLocker.Unlock();
				return true; // got enough hosts
			} else {
				addHostLocker.Unlock();
				return false; // didn't get enough hosts
			}
		} else {
			addHostLocker.Unlock();
			return true; // allready got enough hosts, don't need more
		}
	} else {
		return true; // allready got enough hosts, don't need more
	}
}

bool LastCommonRouteFinder::AddHostsToCheck(CTypedPtrList<CPtrList, CUpDownClient*> &list) {
	if(needMoreHosts) {
		addHostLocker.Lock();

		if(needMoreHosts) {
			if(list.GetCount() >= 10) {
				hostsToTraceRoute.RemoveAll();

				uint32 startPos = rand()/(RAND_MAX/list.GetCount());

				POSITION pos = list.GetHeadPosition();
				for(uint32 skipCounter = startPos; skipCounter < (uint32)list.GetCount() && pos != NULL; skipCounter++) {
					list.GetNext(pos);
				}

				uint32 tryCount = 0;
				while(pos != NULL && hostsToTraceRoute.GetCount() < 10 && tryCount <= (uint32)list.GetCount()) {
					tryCount++;
					CUpDownClient* client = list.GetNext(pos);

					uint32 ip = client->GetIP();

					if(IsGoodIP(ip, true)) {
						hostsToTraceRoute.AddTail(ip);
					}

					//                    if(pos == NULL) {
					//                        POSITION pos = list.GetHeadPosition();
					//                    }
				}
			}

			if(hostsToTraceRoute.GetCount() >= 10 || !run) {
				needMoreHosts = false;

				// Signal that there's hosts to fetch.
				newTraceRouteHostEvent->SetEvent();

				addHostLocker.Unlock();
				return true; // got enough hosts
			} else {
				addHostLocker.Unlock();
				return false; // didn't get enough hosts
			}
		} else {
			addHostLocker.Unlock();
			return true; // allready got enough hosts, don't need more
		}
	} else {
		return true; // allready got enough hosts, don't need more
	}
}

bool LastCommonRouteFinder::Find() {
	Pinger pinger;

	// retry loop. enabled will be set to false in end of this loop, if to many failures (tries too large)
	bool foundLastCommonHost = false;
	lastCommonHost = 0;
	lastCommonTTL = 0;
	hostToPing = 0;
	hostToPingNoTTL = 0; // Add-on

	hostsToTraceRoute.RemoveAll();

	bool atLeastOnePingSucceded = false;
	int traceRouteTries = 0;
	while(run && foundLastCommonHost == false && traceRouteTries < 3 && hostsToTraceRoute.GetCount() < 10) {
		traceRouteTries++;

		lastCommonHost = 0;

		theApp.QueueDebugLogLine(false,_T("SpeedSense: Try #%i. Collecting hosts..."), traceRouteTries);

		addHostLocker.Lock();
		needMoreHosts = true;
		addHostLocker.Unlock();

		// wait for hosts to traceroute
		newTraceRouteHostEvent->Lock();

		theApp.QueueDebugLogLine(false,_T("SpeedSense: Got enough hosts. Listing the hosts that will be tracerouted:"));

		POSITION pos = hostsToTraceRoute.GetHeadPosition();
		int counter = 0;
		while(pos != NULL) {
			counter++;
			uint32 hostToTraceRoute = hostsToTraceRoute.GetNext(pos);
			IN_ADDR stDestAddr;
			stDestAddr.s_addr = hostToTraceRoute;

			theApp.QueueDebugLogLine(false,_T("SpeedSense: Host #%i: %s"), counter, ipstr(stDestAddr));
		}

		// find the last common host, using traceroute

		theApp.QueueDebugLogLine(false,_T("SpeedSense: Starting traceroutes to find last common host."));

		bool failed = false;

		uint32 curHost = 0;
		uint32 curPingAddress = 0;
		for(uint32 ttl = 1; run && (curHost != 0 && ttl <= 64 || curHost == 0 && ttl < 5) && foundLastCommonHost == false && failed == false; ttl++) {
			theApp.QueueDebugLogLine(false,_T("SpeedSense: Pinging for TTL %i..."), ttl);
			curHost = 0;
			PingStatus pingStatus = {0,0,0,0,0,0};

			uint32 lastSuccedingPingAddress = 0;
			uint32 lastDestinationAddress = 0;
			uint32 hostsToTraceRouteCounter = 0;
			bool failedThisTtl = false;
			bool firstLoop = true;
			pos = hostsToTraceRoute.GetHeadPosition();
			while(run && failed == false && pos != NULL && (firstLoop ||
				pingStatus.success && pingStatus.destinationAddress == curHost ||
				pingStatus.success == false && pingStatus.error == IP_REQ_TIMED_OUT))
			{
				firstLoop = false;

				lastSuccedingPingAddress = 0;
				lastDestinationAddress = 0;
				hostsToTraceRouteCounter = 0;
				failedThisTtl = false;

				POSITION lastPos = pos;

				// this is the current address we send ping to, in loop below.
				// PENDING: Don't confuse this with curHost, which is unfortunately almost
				// the same name. Will rename one of these variables as soon as possible, to
				// get more different names.
				uint32 curAddress = hostsToTraceRoute.GetNext(pos);

				pingStatus.success = false;
				for(int counter = 0; run && counter < 3 && pingStatus.success == false; counter++) {
					CString test = ipstr(curAddress);
					pingStatus = pinger.Ping(curAddress, ttl, true);
					if((pingStatus.success == false ||
						pingStatus.success == true &&
						pingStatus.status != IP_SUCCESS &&
						pingStatus.status != IP_TTL_EXPIRED_TRANSIT) && counter < 3-1) {
							IN_ADDR stDestAddr;
							stDestAddr.s_addr = curAddress;
							theApp.QueueDebugLogLine(false,_T("SpeedSense: Failure #%i to ping host! (TTL: %i IP: %s error: %i). Sleeping 1 sec before retry. Error info follows."), counter+1, ttl, ipstr(stDestAddr), pingStatus.error);
							pinger.PIcmpErr(pingStatus.error);

							Sleep(1000);

						}
				}

				if(pingStatus.success == true && pingStatus.status == IP_TTL_EXPIRED_TRANSIT) {
					if(curHost == 0) {
						curHost = pingStatus.destinationAddress;
						curPingAddress = curAddress;
					}
					atLeastOnePingSucceded = true;
					lastSuccedingPingAddress = curAddress;
					lastDestinationAddress = pingStatus.destinationAddress;
				}
				else {
					// failed to ping this host for some reason.
					// Or we reached the actual host we are pinging. We don't want that, since it is too close.
					// Remove it.
					IN_ADDR stDestAddr;
					stDestAddr.s_addr = curAddress;
					if(pingStatus.success == true && pingStatus.status == IP_SUCCESS) {
						theApp.QueueDebugLogLine(false,_T("SpeedSense: Host was too close! (TTL: %i IP: %s status: %i). Removing this host and restarting host collection."), ttl, ipstr(stDestAddr), pingStatus.status);

						hostsToTraceRoute.RemoveAt(lastPos);
						//failed = true;
					} else if(pingStatus.success == true && pingStatus.status == IP_DEST_HOST_UNREACHABLE) {
						theApp.QueueDebugLogLine(false,_T("SpeedSense: Host unreacheable! (TTL: %i IP: %s status: %i). Removing this host. Status info follows."), ttl, ipstr(stDestAddr), pingStatus.status);
						pinger.PIcmpErr(pingStatus.status);

						hostsToTraceRoute.RemoveAt(lastPos);
					} else if(pingStatus.success == true) {
						theApp.QueueDebugLogLine(false,_T("SpeedSense: Unknown ping status! (TTL: %i IP: %s status: %i). Reason follows. Changing ping method to see if it helps."), ttl, ipstr(stDestAddr), pingStatus.status);
						pinger.PIcmpErr(pingStatus.status);
					} else {
						if(pingStatus.error == IP_REQ_TIMED_OUT) {
							theApp.QueueDebugLogLine(false,_T("SpeedSense: Timeout when pinging a host! (TTL: %i IP: %s Error: %i). Keeping host. Error info follows."), ttl, ipstr(stDestAddr), pingStatus.error);
							pinger.PIcmpErr(pingStatus.error);

							if(hostsToTraceRouteCounter > 2 && lastSuccedingPingAddress == 0) {
								// several pings have timed out on this ttl. Probably we can't ping on this ttl at all
								failedThisTtl = true;
							}
						} else {
							theApp.QueueDebugLogLine(false,_T("SpeedSense: Unknown pinging error! (TTL: %i IP: %s status: %i). Reason follows. Changing ping method to see if it helps."), ttl, ipstr(stDestAddr), pingStatus.error);
							pinger.PIcmpErr(pingStatus.error);
						}
					}

					if(hostsToTraceRoute.GetSize() <= 8) {
						theApp.QueueDebugLogLine(false,_T("SpeedSense: To few hosts to traceroute left. Restarting host colletion."));
						failed = true;
					}
				}
			}

			if(failed == false) {
				if(curHost != 0 && lastDestinationAddress != 0) {
					if(lastDestinationAddress == curHost) {
						IN_ADDR stDestAddr;
						stDestAddr.s_addr = curHost;
						theApp.QueueDebugLogLine(false,_T("SpeedSense: Host at TTL %i: %s"), ttl, ipstr(stDestAddr));

						lastCommonHost = curHost;
						lastCommonTTL = ttl;
					} else /*if(lastSuccedingPingAddress != 0)*/ {
						foundLastCommonHost = true;
						hostToPingNoTTL = lastCommonHost; // Add-on
						hostToPing = curPingAddress;

						CString hostToPingString = ipstr(lastDestinationAddress);

						if(lastCommonHost != 0) {
							theApp.QueueDebugLogLine(false,_T("SpeedSense: Found differing host at TTL %i: %s."), ttl, hostToPingString);
						} else {
							CString lastCommonHostString = ipstr(curHost);

							hostToPingNoTTL = curHost; // Add-on
							lastCommonHost = curHost;
							lastCommonTTL = ttl;
							theApp.QueueDebugLogLine(false,_T("SpeedSense: Found differing host at TTL %i, but last ttl couldn't be pinged so we don't know last common host. Taking a chance and using first differing ip as last commonhost. Host to ping: %s. Faked LastCommonHost: %s"), ttl, hostToPingString, lastCommonHostString);
						}
					}
				} else {
					if(ttl < 4) {
						theApp.QueueDebugLogLine(false,_T("SpeedSense: Could perform no ping at all at TTL %i. Trying next ttl."), ttl);
					} else {
						theApp.QueueDebugLogLine(false,_T("SpeedSense: Could perform no ping at all at TTL %i. Giving up."), ttl);
					}
					lastCommonHost = 0;
				}
			}
		}
	}
	if (run){
		theApp.QueueDebugLogLine(false,_T("SpeedSense: Done tracerouting. Evaluating results."));
		if(foundLastCommonHost == true) {
			IN_ADDR stLastCommonHostAddr;
			stLastCommonHostAddr.s_addr = lastCommonHost;

			IN_ADDR stHostToPingAddr;
			stHostToPingAddr.s_addr = hostToPing;

			// log result
			theApp.QueueDebugLogLine(false,_T("SpeedSense: Found last common host. LastCommonHost: %s @ TTL: %i HostToPing: %s"), ipstr(stLastCommonHostAddr), lastCommonTTL, ipstr(stHostToPingAddr));
		} else {
			theApp.QueueDebugLogLine(false,_T("SpeedSense: Tracerouting failed to many times. Checking diffrent mode UDP/ICMP or manually entered URL/IP..."));
			return false;
		}

		return true;
	}
	return false;
}

#endif // NEO_BC // NEO: NBC END <-- Xanatos --