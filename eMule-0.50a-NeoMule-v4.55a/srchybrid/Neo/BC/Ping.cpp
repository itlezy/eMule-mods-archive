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


#include "StdAfx.h"
#include "Neo/BC/BandwidthControl.h"
#include "ping.h"

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->

#include "emule.h"
#include "neo/Neopreferences.h"
#include "math.h"
#include "emuleDlg.h"
#include "Ws2tcpip.h"
#include "otherfunctions.h"

IMPLEMENT_DYNCREATE(CPingThread, CWinThread)
CPingThread::CPingThread(void)
{
	isPingThread=false;
	pingStatus.success=false;
	dwAddress=0;
	pHost=NULL;
	hostToPing=0;
	hostToPingNoTTL=0;
	lastCommonTTL=0;
	findHost=false;
	prepares=true;
	searching=true;
	works=false;
	timeout=false;
	lowestPingIsFound=false;
	autoHost = NeoPrefs.IsAutoHostToPing();
	staticLowestPing = NeoPrefs.IsStaticLowestPing();
	PingMode = (ePingMode)NeoPrefs.GetPingMode();
	noTTLExec = noTTL = NeoPrefs.IsNoTTL();
	endPingThread = new CEvent();
	m_bAutoDelete=false;
	lastRestartTime=0;
	failedTimes=0;
	timeOut = TIMEOUT;
	lastCommonRouteFinder = NULL;
	pinger = NULL;
}

CPingThread::~CPingThread(void)
{
	if (pinger)
		delete pinger;
	// ZZ:SpeedSense -->
	if (lastCommonRouteFinder)
		delete lastCommonRouteFinder;
	// ZZ:SpeedSense <--
	if (endPingThread)
		delete endPingThread;
}


void CPingThread::Init(void)
{
	avg_ping=0.1f;
	lowest_ping=0.1f;
	lowest_index=0;
	for (int i=0; i<m_lowest_ping_values; i++)
		lowest_list[i]=0.0f;
	
	if (PingMode == pingUDP && !noTTLExec)
		Pinger::sharedPingMode = pingAuto;
	else
		Pinger::sharedPingMode = PingMode;

	if (pinger)
		delete pinger;
	pinger = new Pinger();

	// ZZ:SpeedSense -->
	if (lastCommonRouteFinder){
		lastCommonRouteFinder->StopSearching();
		delete lastCommonRouteFinder;
	}
	lastCommonRouteFinder = new LastCommonRouteFinder();
	// ZZ:SpeedSense <--
}

pingVal CPingThread::GetPing(){
	pingVal ret={avg_ping, changed, lowest_ping, lowest_changed, pingStatus.success}; 
	changed=false;
	lowest_changed=false;
	return ret;
}

void CPingThread::findHostToPing(){
	lastCommonRouteFinder->AddHostToCheck(NeoPrefs.GetURLPing());
	if (lastCommonRouteFinder->Find()){
		hostToPing = lastCommonRouteFinder->GetHostToPing();
		hostToPingNoTTL = lastCommonRouteFinder->GetHostToPingNoTTL();
		lastCommonTTL = lastCommonRouteFinder->GetLastCommonTTL();
		lastCommonHost = lastCommonRouteFinder->GetLastCommonHost();
	}
	else {
		hostToPing = 0;
		hostToPingNoTTL = 0;
		lastCommonTTL = 0;
		lastCommonHost = 0;
	}
}

// ZZ:SpeedSense -->
void CPingThread::findLowestPing(bool useURL){
	theApp.QueueDebugLogLine(false,_T("SpeedSense: Finding a start value for lowest ping..."));

	uint32 toPing=0;
	uint32 TTL=0;
	uint32 fails=0;

	float tempVals;

	if (hostToPing && autoHost && !useURL){
		if (noTTLExec)
			toPing=hostToPingNoTTL;
		else
		toPing=hostToPing;
		TTL=lastCommonTTL;
	}
	else if (dwAddress){
		toPing=dwAddress;
		TTL=DEFAULT_TTL;
	}
	else {
		works = false;
		theApp.QueueDebugLogLine(false,_T("SpeedSense: Failed calculeting lowest ping - no host to ping, check configuration. Restarting SpeedSense..."));
		return;
	}

	// finding lowest ping
	tempVals=(float)TIMEOUT;
	for(int initialPingCounter = 0; isPingThread && initialPingCounter < 10; initialPingCounter++) {
		Sleep(200);

		pingStatus = pinger->Ping(toPing, TTL, true);

		if (pingStatus.success /*&& (pinger->PingMode() == pingICMP || pinger->PingMode() == pingUDP && !pinger->IsTimeOut())*/)
			tempVals=min(tempVals, pingStatus.delay);
		else {
			fails++;
			theApp.QueueDebugLogLine(false,_T("SpeedSense: Ping #%i failed. Reason follows"), initialPingCounter);
			pinger->PIcmpErr(pingStatus.error);
		}
	}

	if (fails >= 6){
		works=false;
		theApp.QueueDebugLogLine(false,_T("SpeedSense: Failed calculeting lowest ping - too many fails. Restarting SpeedSense..."));
		return;
	}

	if (tempVals < 0.1f)
		tempVals = 0.1f;

	lowest_ping = tempVals;
	for (int i=0; i<m_lowest_ping_values; i++)
		lowest_list[i]=lowest_ping;
	timeOut = (uint16)(800 * log10f(8+lowest_ping*lowest_ping));
	theApp.QueueDebugLogLine(false,_T("SpeedSense: Lowest ping: %3.2f ms"), lowest_ping);
}
// ZZ:SpeedSense <--

int CPingThread::Run(void)
{
	// BEGIN SLUGFILLER: SafeHash / NEO: STS - [SlugFillerThreadSafe] -- Xanatos -->
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return 0;
	// END SLUGFILLER: SafeHash // NEO: STS END <-- Xanatos --

	float temp_lowest_ping;
	uint32 start;
	uint32 end;

	endPingThread->ResetEvent();

	while (isPingThread) {
		start=::GetTickCount();

		// Check if user has changed the option in preference (auto/manual host)
		if (autoHost != NeoPrefs.IsAutoHostToPing() 
		 || (PingMode != NeoPrefs.GetPingMode()
		  && (NeoPrefs.GetPingMode() != pingAuto && NeoPrefs.GetPingMode() != Pinger::sharedPingMode) )
		 || noTTL != NeoPrefs.IsNoTTL()
		 || staticLowestPing != NeoPrefs.IsStaticLowestPing()
		 )
		{
			Init();
			autoHost = NeoPrefs.IsAutoHostToPing();
			staticLowestPing = NeoPrefs.IsStaticLowestPing();
			PingMode = (ePingMode)NeoPrefs.GetPingMode();
			if (noTTL != NeoPrefs.IsNoTTL())
				noTTLExec = NeoPrefs.IsNoTTL();
			noTTL = NeoPrefs.IsNoTTL();
			if (autoHost)
				findHost=true;
			else
				SetPingedServer(NeoPrefs.GetURLPing());
		}

		// Check if pinging failed and there is a need to restart
		int lastedTime =  ::GetTickCount()-lastRestartTime;
		if ((!works || findHost) && lastedTime > SEC2MS(10)){
			searching=true;
			if (autoHost) 
				findHostToPing();

			if (Pinger::sharedPingMode == pingICMP || Pinger::sharedPingMode == pingRAW) 
				SetPingedServer(NeoPrefs.GetURLPing());
			else
				dwAddress = 0;

			if (dwAddress || (hostToPing  && autoHost))
				works=true;
			else {
				if (noTTLExec)
					noTTLExec = false;
				else if (noTTL && (Pinger::sharedPingMode == pingICMP || Pinger::sharedPingMode == pingRAW))
					noTTLExec = true;
				if (!noTTLExec && PingMode == pingUDP){ // or only (NeoPrefs.IsUDPing()) ?
					Pinger::sharedPingMode = ((Pinger::sharedPingMode == PingMode) ? pingAuto : PingMode);
					if (Pinger::sharedPingMode == pingAuto)
						dwAddress = 0;
					delete pinger;
					pinger = new Pinger();
				}
			}

			lowestPingIsFound = false;
			findHost=false;
			searching=false;
			if (!works){
				prepares=false;
			}
			lastRestartTime=::GetTickCount();
		}

		// Check the need to find out of lowest ping
		if (!lowestPingIsFound && !findHost && works){
			if (!prepares){
				prepares=true;
				::Sleep(1000);
			}

			findLowestPing();
			if (!works && dwAddress && (Pinger::sharedPingMode == pingICMP || Pinger::sharedPingMode == pingRAW) && autoHost){
				works = true;
				findLowestPing(true);
			}

			if (works){
				theApp.QueueDebugLogLine(false,_T("SpeedSense: Initialized succesfully."));
				temp_lowest_ping=lowest_ping;
				lowestPingIsFound=true;
				lowest_changed=true;
			}
			else {
				if (noTTLExec)
					noTTLExec = false;
				else if (noTTL && (Pinger::sharedPingMode == pingICMP || Pinger::sharedPingMode == pingRAW))
					noTTLExec = true;
				if (!noTTLExec && PingMode == pingUDP){
					Pinger::sharedPingMode = ((Pinger::sharedPingMode == PingMode) ? pingAuto : PingMode);
					if (Pinger::sharedPingMode == pingAuto)
						dwAddress = 0;
					delete pinger;
					pinger = new Pinger();
				}
			}

			prepares=false;
			lastRestartTime=::GetTickCount();
		}

		// Send new ICMP packet
		if (works) {
			avg_ping = Ping();
			changed = true;

			if(staticLowestPing == false){
				// Check lowest ping
				if (avg_ping<lowest_ping && (pinger->PingMode() == pingICMP || pinger->PingMode() == pingRAW /*|| pinger->PingMode() == pingUDP && !pinger->IsTimeOut()*/)){
					temp_lowest_ping = lowest_ping * m_lowest_ping_values - lowest_list[lowest_index];
					lowest_list[lowest_index]=avg_ping;
					temp_lowest_ping += lowest_list[lowest_index];
					lowest_ping = temp_lowest_ping/m_lowest_ping_values;
					lowest_changed=true;
					lowest_index++;
					lowest_index%=m_lowest_ping_values;
					timeOut = (uint16)(800 * log10f(8+lowest_ping*lowest_ping));
					theApp.QueueDebugLogLine(false,_T("SpeedSense: Lowest ping: %3.2f ms"), lowest_ping);
				}
				// End check
			}

			// ZZ:SpeedSense -->
		}

		uint32 ticksBetweenPings = 1000;
		float upload=theApp.bandwidthControl->GetMaxUpload();
		if(upload > 0.0f) {
			// UDP pinger 20 /IP/ + 8 /UDP/ + 4 /Data/ = 32 bytes
			if (pinger->PingMode() == pingUDP)
				ticksBetweenPings = (uint16)(3200/upload); //4*100/upload
			// ping packages being 64 bytes, this should use 1% of bandwidth (one hundredth of bw).
			else
				ticksBetweenPings = (uint16)(6400/upload); //64*100/upload

			if(ticksBetweenPings < 125) {
				// never ping more than 8 packages a second
				ticksBetweenPings = 125;
			} else if(ticksBetweenPings > 1000) {
				ticksBetweenPings = 1000;
			}
		}

		// ZZ:SpeedSense <--
		end=::GetTickCount();

		// Sleep before next loop
		if ((end-=start) < ticksBetweenPings)
			::Sleep(ticksBetweenPings-end);
	}

	endPingThread->SetEvent();
	return 0;
}

CString	 CPingThread::GetPingedServer()
{
	if (isPreparing() || !lowestPingIsFound || autoHost != NeoPrefs.IsAutoHostToPing())
		return _T("-");

	if (NeoPrefs.IsAutoHostToPing() && lastCommonHost){
		IN_ADDR stLastCommonHostAddr;
		stLastCommonHostAddr.s_addr = lastCommonHost;

		CString ret;
		ret.Format(_T("%s"), ipstr(stLastCommonHostAddr));
		return ret;
	}

	if (dwAddress)
		return NeoPrefs.GetURLPing();

	return _T("Failed - bad address");
}

bool CPingThread::SetPingedServer(CString pingServer)
{	
	dwAddress = 0;
	// Lookup destination
	// Use inet_addr() to determine if we're dealing with a name
	// or an address
	if (pingServer==_T(""))
		return false;

	iaDest.s_addr = inet_addr(CT2CA(pingServer));

	if (iaDest.s_addr == INADDR_NONE) {
		pHost = gethostbyname(CT2CA(pingServer));
	} else {
		pHost = gethostbyaddr((const char*)&iaDest, sizeof(struct in_addr), AF_INET);
	}

	if (pHost == NULL) {
		theApp.QueueDebugLogLine(false, _T("%s not found"), pingServer);
		return false;
	}

	dwAddress = *(DWORD*)(*pHost->h_addr_list);

	if (!autoHost || !hostToPing)
		lowestPingIsFound=false;

	return true;
}

float CPingThread::Ping(void)
{
	if (hostToPing && autoHost) {
		if (hostToPingNoTTL && noTTL)
			pingStatus = pinger->Ping(hostToPingNoTTL,lastCommonTTL,false, timeOut);
		else
			pingStatus = pinger->Ping(hostToPing,lastCommonTTL,false, timeOut);

		if(pingStatus.success && pingStatus.destinationAddress != lastCommonHost) {
			// something has changed about the topology! We got another ip back from this ttl than expected.
			// Do the tracerouting again to figure out new topology
			IN_ADDR stLastCommonHostAddr;
			stLastCommonHostAddr.s_addr = lastCommonHost;

			IN_ADDR stDestinationAddr;
			stDestinationAddr.s_addr = pingStatus.destinationAddress;

			theApp.QueueDebugLogLine(false,_T("SpeedSense: Network topology has changed. TTL: %i Expected ip: %s Got ip: %s Will do a new traceroute."), lastCommonTTL, ipstr(stLastCommonHostAddr), inet_ntoa(stDestinationAddr));
			findHost = true;
			lowestPingIsFound = false;
			works = false;
		}
	}
	else if (dwAddress) {
		pingStatus = pinger->Ping(dwAddress,DEFAULT_TTL,false, timeOut);
	} else {
		works=false;
		theApp.QueueDebugLogLine(false,_T("No host to ping, check configuration. Restarting SpeedSense."));
	}

	if (pingStatus.success || pingStatus.error == IP_REQ_TIMED_OUT){
		failedTimes = 0;

		timeout = (pingStatus.delay >= timeOut);

		if (pingStatus.delay < 0.1f)
			pingStatus.delay = 0.1f;

		return pingStatus.delay;
	}
	else {
		failedTimes++;
		if (failedTimes == 5){
			failedTimes = 0;
			works=false;
			lowestPingIsFound=false;
			theApp.QueueDebugLogLine(false,_T("SpeedSense: failed too many times. Restarting SpeedSense..."));
		}
		return timeOut;
	}
}

// Stop thread (can be called more than one time)
void CPingThread::StopPingThread(void)
{
	if (isPingThread) {
		SuspendThread();
		isPingThread = false; 
	}
}

// Resumes thread (can be called more than one time)
void CPingThread::StartPingThread(void)
{
	if (!isPingThread) {
		isPingThread = true;
		ResumeThread();
	}
}

// Finish thread
void CPingThread::EndPingThread(void)
{
	isPingThread = false;
	lastCommonRouteFinder->StopSearching();
	ResumeThread();
	endPingThread->Lock();
}

#endif // NEO_BC // NEO: NBC END <-- Xanatos --