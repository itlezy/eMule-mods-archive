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

/*---------------------------------------------------------------------
*
*  Some code in this file has been copied from a ping demo that was
*  created by Bob Quinn, 1997          http://www.sockets.com
*
* As some general documenation about how the ping is implemented,
* here is the description Bob Quinn wrote about the ping demo.
*
* <--- Cut --->
*
* Description:
*  This is a ping program that uses Microsoft's ICMP.DLL functions 
*  for access to Internet Control Message Protocol.  This is capable
*  of doing "ping or "traceroute", although beware that Microsoft 
*  discourages the use of these APIs.
*
*  Tested with MSVC 5 compile with "cl ms_icmp.c /link ws2_32.lib"
*  from the console (if you've run VCVARS32.BAT batch file that
*  ships with MSVC to set the proper environment variables)
*
* NOTES:
* - With both "Don't Fragment" and "Router Alert" set, the 
*    IP don't fragment bit is set, but not the router alert option.
* - The ICMP.DLL calls are not redirected to the TCP/IP service
*    provider (what's interesting about this is that the setsockopt()
*    IP_OPTION flag can't do Router Alert, but this API can ...hmmm.
* - Although the IcmpSendEcho() docs say it can return multiple
*    responses, if I receive multiple responses (e.g. sending to
*    a limited or subnet broadcast address) IcmpSendEcho() only
*    returns one.  Interesting that NT4 and Win98 don't respond
*    to broadcast pings.
* - using winsock.h  WSOCK32.LIB and version 1.1 works as well as 
*    using winsock2.h WS2_32.LIB  and version 2.2
*
* Some Background:
*
* The standard Berkeley Sockets SOCK_RAW socket type, is normally used
* to create ping (echo request/reply), and sometimes traceroute applications
* (the original traceroute application from Van Jacobson used UDP, rather
* than ICMP). Microsoft's WinSock version 2 implementations for NT4 and 
* Windows 95 support raw sockets, but none of their WinSock version 1.1
* implementations (WFWG, NT3.x or standard Windows 95) did.
*
* Microsoft has their own API for an ICMP.DLL that their ping and tracert
* applications use (by the way, they are both non-GUI text-based console
* applications. This is a proprietary API, and all function calls that 
* involve network functions operate in blocking mode. They still include 
* it with WinSock 2 implementations.
*
* There is little documentation available (I first found it in the Win32
* SDK in \MSTOOLS\ICMP, and it exists on the MS Developers' Network
* CD-ROM now, also). Microsoft disclaims this API about as strongly as 
* possible.  The README.TXT that accompanies it says:
*
* [DISCLAIMER]
* 
* We have had requests in the past to expose the functions exported from
* icmp.dll. The files in this directory are provided for your convenience
* in building applications which make use of ICMPSendEcho(). Notice that
* the functions in icmp.dll are not considered part of the Win32 API and
* will not be supported in future releases. Once we have a more complete
* solution in the operating system, this DLL, and the functions it exports,
* will be dropped.    
*      
* [DOCUMENTATION]     
*
* The ICMPSendEcho() function sends an ICMP echo request to the specified
* destination IP address and returns any replies received within the timeout
* specified. The API is synchronous, requiring the process to spawn a thread
* before calling the API to avoid blocking. An open IcmpHandle is required
* for the request to complete. IcmpCreateFile() and IcmpCloseHandle() 
* functions are used to create and destroy the context handle.
*
* <--- End cut --->
*/

/*---------------------------------------------------------------------
* Reworked (tag UDPing) to use UDP packets and ICMP_TTL_EXPIRE responses
* to avoid problems with some ISPs disallowing or limiting ICMP_ECHO
* (ping) requests. This has downside on NT based systems - raw sockets
* can be used only by administrative users; thereby current code has to
* implement both approaches.
*
* Example code and ideas used from (credits go to articles authors):
* http://tangentsoft.net/wskfaq/examples/rawping.html
* http://www.cs.concordia.ca/~teaching/comp445/labs/webpage/ch16.htm
* http://www.networksorcery.com/enp/protocol/icmp.htm
*
* NB! Take care about winsock libraries - look below for IP_TTL!
*
* Partial copyright (c) 2003 by DonQ
*/

/* ----------------------------------------------------------------
*             Win Sock 2 Raw Ping implementation
* 
* Module : PING.CPP
* Purpose: Implementation for an MFC wrapper class to encapsulate PING
* Created: PJN / 10-06-1998
* History: PJN / 23-06-1198 1) Now code can be compiled to use Winsock2 calls
*                           instead of using the ICMP.DLL. This gives another of
*                           advantages:
* 
*                           i)  Your using a API that MS has promised to continue to support.
*                           ii) Internally the class calls QueryPerformanceCounter meaning that
*                               you will get the highest resolution RTT's possible.
* 
*                           2) Also did a general tidy up of the code
*                           3) Changed default timeout to 1 second
* 
* 
* 
* Copyright (c) 1998 by PJ Naughter.  
* All rights reserved.
*
*/

#include "stdafx.h"
#pragma warning(disable:4127) // conditional expression is constant
#include <Ws2tcpip.h>       // UDPing - raw socket and TTL setting support
#pragma warning(default:4127) // conditional expression is constant
#include "emule.h"
#include "Pinger.h"
#include "emuledlg.h"
#include "OtherFunctions.h"

extern CString GetErrorMessage(DWORD dwError, DWORD dwFlags);

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->

// RAWPing Functions
void FillIcmpData(ICMPHeader* pIcmp, int nData);
BOOL DecodeResponse(char* pBuf, int nBytes);
USHORT GenerateIPChecksum(USHORT* pBuffer, int nSize);
BOOL IsSocketReadible(SOCKET socket, DWORD dwTimeout, BOOL& bReadible);
// RAWPing Functions end

#define BUFSIZE     8192
#define DEFAULT_LEN 0

ePingMode Pinger::sharedPingMode = pingICMP;
/*---------------------------------------------------------
* IcmpSendEcho() Error Strings
* 
* The values in the status word returned in the ICMP Echo 
*  Reply buffer after calling IcmpSendEcho() all have a
*  base value of 11000 (IP_STATUS_BASE).  At times,
*  when IcmpSendEcho() fails outright, GetLastError() will 
*  subsequently return these error values also.
*
* Two Errors value defined in ms_icmp.h are missing from 
*  this string table (just to simplify use of the table):
*    "IP_GENERAL_FAILURE (11050)"
*    "IP_PENDING (11255)"
*/
#define MAX_ICMP_ERR_STRING  IP_STATUS_BASE + 22
static const LPCTSTR aszSendEchoErr[] = {
	_T("IP_STATUS_BASE (11000)"),
		_T("IP_BUF_TOO_SMALL (11001)"),
		_T("IP_DEST_NET_UNREACHABLE (11002)"),
		_T("IP_DEST_HOST_UNREACHABLE (11003)"),
		_T("IP_DEST_PROT_UNREACHABLE (11004)"),
		_T("IP_DEST_PORT_UNREACHABLE (11005)"),
		_T("IP_NO_RESOURCES (11006)"),
		_T("IP_BAD_OPTION (11007)"),
		_T("IP_HW_ERROR (11008)"),
		_T("IP_PACKET_TOO_BIG (11009)"),
		_T("IP_REQ_TIMED_OUT (11010)"),
		_T("IP_BAD_REQ (11011)"),
		_T("IP_BAD_ROUTE (11012)"),
		_T("IP_TTL_EXPIRED_TRANSIT (11013)"),
		_T("IP_TTL_EXPIRED_REASSEM (11014)"),
		_T("IP_PARAM_PROBLEM (11015)"),
		_T("IP_SOURCE_QUENCH (11016)"),
		_T("IP_OPTION_TOO_BIG (11017)"),
		_T("IP_BAD_DESTINATION (11018)"),
		_T("IP_ADDR_DELETED (11019)"),
		_T("IP_SPEC_MTU_CHANGE (11020)"),
		_T("IP_MTU_CHANGE (11021)"),
		_T("IP_UNLOAD (11022)")
};

Pinger::Pinger(ePingMode epMode) {
	WSADATA wsaData;

	int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (nRet) {
		theApp.QueueDebugLogLine(false,_T("Pinger: WSAStartup() failed, err: %d\n"), nRet);
		return;
	}

	// UDPing - mode selection part -->
	mPingMode = epMode;
	if (epMode != pingICMP) {
		if(epMode != pingRAW && 0 == initPinger(pingUDP))
			mPingMode = pingUDP;
		else if (0 == initPinger(pingRAW)) 
			mPingMode = pingRAW;
		else if (epMode == pingAuto)
			mPingMode = pingICMP;       // fallback to ICMP if UDP doesn't succeed
		else {
			mPingMode = pingAuto;       // better reset this - indicates invalid pinger
			return;                     // cannot initialize requested mode
		}
	}

	if (mPingMode == pingICMP) {
		if (0 != initPinger(pingICMP)) {
			mPingMode = pingAuto;       // better reset this - indicates invalid pinger
			return;                     // cannot initialize requested mode
		}
	}
	// UDPing - mode selection part end <--

	toNowTimeOut = 0;

	// Init IPInfo structure
	stIPInfo.Tos      = 0;
	stIPInfo.Flags    = 0;
	stIPInfo.OptionsSize = 0;
	stIPInfo.OptionsData = NULL;
}

// UDPing reworked initialization -->
int Pinger::initPinger(ePingMode epMode) {
	int nRet = -1;      // generic error
	sockaddr_in sa;     // for UDP and raw sockets

	if (epMode == pingUDP) {
		// ICMP must accept all responses
		sa.sin_family = AF_INET;
		// NEO: MOD - [BindToAdapter] -- Xanatos -->
		if (theApp.GetBindAddress())
			sa.sin_addr.s_addr = inet_addr(CT2CA(theApp.GetBindAddress()));
		else
		// NEO: MOD END <-- Xanatos --
			sa.sin_addr.s_addr = INADDR_ANY;	
		sa.sin_port = 0;

		// attempt to initialize raw ICMP socket
		is = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
		if (is != INVALID_SOCKET) {
			nRet = bind(is, (sockaddr *)&sa, sizeof(sa));
			if (nRet==SOCKET_ERROR) { 
				nRet = WSAGetLastError();
				closesocket(is);        // ignore return value - error close anyway
				return nRet;
			} 
		}
		else {
			return 1;
		}

		// attempt to initialize ordinal UDP socket - why should this fail???
		// NB! no need to bind this at a moment - will be bound later, implicitly at sendto
		us = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (us == INVALID_SOCKET) {
			closesocket(is);            // ignore return value - we need to close it anyway!
			return 2;
		}
		nRet = 0;
	}

	// RAWPing initialization -->
	if (epMode == pingRAW) {
		//Use the High performace counter to get an accurate RTT
		LARGE_INTEGER Frequency;
		Frequency.QuadPart = 0;
		
		if(!QueryPerformanceFrequency(&Frequency))
			return 1;
		
		m_TimerFrequency = Frequency.QuadPart;

		//Create the raw socket
		sockRaw = WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, NULL, 0, 0);
		if (sockRaw == INVALID_SOCKET) {
			closesocket(sockRaw);
			//theApp.QueueDebugLogLine(false, _T("Pinger: Failed to create Raw Socket"));
			return 2;
		}
		nRet = 0;
	}
	// RAWPing initialization end <--
	
	if (epMode == pingICMP) {
		// Open ICMP.DLL
		hICMP_DLL = LoadLibrary(_T("ICMP.DLL"));
		if (hICMP_DLL == 0) {
			theApp.QueueDebugLogLine(false,_T("Pinger: LoadLibrary() failed: Unable to locate ICMP.DLL!"));
			return 1;
		}

		// Get pointers to ICMP.DLL functions
		lpfnIcmpCreateFile  = (IcmpCreateFile*)GetProcAddress(hICMP_DLL,"IcmpCreateFile");
		lpfnIcmpCloseHandle = (IcmpCloseHandle*)GetProcAddress(hICMP_DLL,"IcmpCloseHandle");
		lpfnIcmpSendEcho    = (IcmpSendEcho*)GetProcAddress(hICMP_DLL,"IcmpSendEcho");
		if ((!lpfnIcmpCreateFile) || 
			(!lpfnIcmpCloseHandle) || 
			(!lpfnIcmpSendEcho)) {

				theApp.QueueDebugLogLine(false,_T("Pinger: GetProcAddr() failed for at least one function."));
				return 2;
			}

		// Open the ping service
		hICMP = (HANDLE) lpfnIcmpCreateFile();
		if (hICMP == INVALID_HANDLE_VALUE) {
			theApp.QueueDebugLogLine(false, _T("Pinger: IcmpCreateFile() failed, err: "));
			PIcmpErr(GetLastError());
			return 3;
		}
		nRet = 0;
	}

	// Init IPInfo structure
	stIPInfo.Tos      = 0;
	stIPInfo.Flags    = 0;
	stIPInfo.OptionsSize = 0;
	stIPInfo.OptionsData = NULL;

	m_time.Start();
	return nRet;
}
// UDPing reworked initialization end <--

Pinger::~Pinger() {
	// UDPing reworked cleanup -->
	if (mPingMode == pingUDP) {
		closesocket(is);                // close UDP socket
		closesocket(us);                // close raw ICMP socket
	}
	// UDPing reworked cleanup end <--

	// RAWPing cleanup -->
	if (mPingMode == pingRAW) {
		closesocket(sockRaw);           // close RAW socket
	}
	// RAWPing cleanup end <--

	if (mPingMode == pingICMP) {        // UDPing check
		// Close the ICMP handle
		BOOL fRet = lpfnIcmpCloseHandle(hICMP);
		if (fRet == FALSE) {
			//int nErr = GetLastError();
			theApp.QueueDebugLogLine(false,_T("Error closing ICMP handle, err: "));
			PIcmpErr(GetLastError());
		}

		// Shut down...
		FreeLibrary(hICMP_DLL);
	}                                   // UDPing check

	mPingMode = pingAuto;               // UDPing mode reset (not neccessary :-)

	WSACleanup();
}

PingStatus Pinger::Ping(uint32 lAddr, uint32 ttl, bool doLog, uint16 timeout) {
	PingStatus returnValue;
	returnValue.success = false;
	returnValue.delay = timeout;
	returnValue.error = IP_REQ_TIMED_OUT;

	float usResTime = 0.0f;
	DWORD lastError = 0;

	// UDPing reworked ping sequence -->
	if (mPingMode == pingUDP) {

		int nTTL = ttl;
		int nRet;
		sockaddr_in sa;
		int nAddrLen = sizeof(struct sockaddr_in); 
		char bufICMP[1500];             // allow full MTU

		// clear ICMP socket before sending UDP - not best solution, but may be needed to exclude late responses etc
		u_long bytes2read = 0;
		do {
			nRet = ioctlsocket(is, FIONREAD, &bytes2read);
			if (bytes2read > 0) {       // ignore errors here
				sa.sin_family = AF_INET;
				// NEO: MOD - [BindToAdapter] -- Xanatos -->
				if (theApp.GetBindAddress())
					sa.sin_addr.s_addr = inet_addr(CT2CA(theApp.GetBindAddress()));
				else
				// NEO: MOD END <-- Xanatos --
					sa.sin_addr.s_addr = INADDR_ANY;	
				sa.sin_port = 0;

				nRet = recvfrom (is,    /* socket */ 
					(LPSTR)bufICMP,     /* buffer */ 
					1500,               /* length */ 
					0,                  /* flags  */ 
					(sockaddr*)&sa,     /* source */ 
					&nAddrLen);         /* addrlen*/

				if (toNowTimeOut) toNowTimeOut--;
			}
		} while (bytes2read > 0);

		// set TTL value for UDP packet - should success with winsock 2
		// NB! take care about IP_TTL value - it's redefined in Ws2tcpip.h!
		// TODO: solve next problem correctly:
		// eMule is linking sockets functions using wsock32.lib (IP_TTL=7)
		// to use IP_TTL define, we must enforce linker to bind this function 
		// to ws2_32.lib (IP_TTL=4) (linker options: ignore wsock32.lib)
		nRet = setsockopt(us, IPPROTO_IP, 4/*IP_TTL*/, (char*)&nTTL, sizeof(int));
		if (nRet==SOCKET_ERROR) { 
			lastError = WSAGetLastError();
			returnValue.success = false;
			returnValue.delay = timeout;
			returnValue.error = lastError;
			return returnValue;
		} 

		sa.sin_family = AF_INET;
		sa.sin_addr.s_addr = lAddr;	
		sa.sin_port = htons(UDP_PORT);

		// send lonely UDP packet with almost minimal content (0 bytes is allowed too, but no data will be sent then)
		nRet = sendto(us, (LPSTR)&nTTL, 4, 0, (sockaddr*)&sa, sizeof(sa));  // send four bytes - TTL :)
		m_time.Tick();
		if (nRet==SOCKET_ERROR) { 
			lastError = WSAGetLastError();
			returnValue.success = false;
			returnValue.error = lastError;
			return returnValue;
		} 

		IPHeader* reply = (IPHeader*)bufICMP;

		bytes2read = 0;
		int timeoutOpt = timeout;
		bool noRcvTimeOut = false;
		nRet = setsockopt(is, SOL_SOCKET, SO_RCVTIMEO, (const char*) &timeoutOpt, sizeof(timeoutOpt));
		if (nRet==SOCKET_ERROR)
			noRcvTimeOut = true;

		while((usResTime += m_time.Tick()) < timeout){
			if (noRcvTimeOut){
				nRet = ioctlsocket(is, FIONREAD, &bytes2read);
				if (nRet != 0) {
					lastError = WSAGetLastError();
					returnValue.success = false;
					returnValue.delay = timeout;
					returnValue.error = lastError;
					return returnValue;
				}
				if (bytes2read > 0) {       // read and filter incoming ICMP

				}
				else {
					Sleep(1);         // share time with other threads
					continue;
				}

			}
			sa.sin_family = AF_INET;
			// NEO: MOD - [BindToAdapter] -- Xanatos -->
			if (theApp.GetBindAddress())
				sa.sin_addr.s_addr = inet_addr(CT2CA(theApp.GetBindAddress()));
			else
			// NEO: MOD END <-- Xanatos --
				sa.sin_addr.s_addr = INADDR_ANY;	
			sa.sin_port = 0;
			nRet = recvfrom (is,    /* socket */ 
				(LPSTR)bufICMP,     /* buffer */ 
				1500,               /* length */ 
				0,                  /* flags  */ 
				(sockaddr*)&sa,     /* source */ 
				&nAddrLen);         /* addrlen*/ 

			usResTime += m_time.Tick();
			if (nRet==SOCKET_ERROR) { 
				lastError = WSAGetLastError();
				returnValue.success = false;
				returnValue.delay = timeout;
				if(lastError != WSAETIMEDOUT)
					returnValue.error = lastError;
				return returnValue;
			} 

			unsigned short header_len = reply->h_len * 4;
			ICMPHeader* icmphdr = (ICMPHeader*)(bufICMP + header_len);
			IN_ADDR stDestAddr;

			stDestAddr.s_addr = reply->source_ip;

			if (((icmphdr->type == ICMP_TTL_EXPIRE) || (icmphdr->type == ICMP_DEST_UNREACH)) &&
				(icmphdr->UDP.dest_port == htons(UDP_PORT)) && (icmphdr->hdrsent.dest_ip == lAddr)) {

				if(icmphdr->type == ICMP_TTL_EXPIRE) {
					returnValue.success = true;
					returnValue.status = IP_TTL_EXPIRED_TRANSIT;
					returnValue.delay = usResTime;
					returnValue.destinationAddress = stDestAddr.s_addr;
					returnValue.ttl = ttl;
				} else {
					returnValue.success = true;
					returnValue.status = IP_DEST_HOST_UNREACHABLE;
					returnValue.delay = usResTime;
					returnValue.destinationAddress = stDestAddr.s_addr;
					returnValue.ttl = 64 - (reply->ttl & 63);
				}

				if(doLog) {
					theApp.QueueDebugLogLine(false,_T("Reply from %s: bytes=%d time=%3.2fms TTL=%i"),
						ipstr(stDestAddr),
						nRet,
						returnValue.delay, 
						returnValue.ttl);
				}
				break;
			}
			else {              // verbose log filtered packets info (not seen yet...)
				if (toNowTimeOut) toNowTimeOut--;
				if(doLog) {
					theApp.QueueDebugLogLine(false,_T("Filtered reply from %s: bytes=%d time=%3.2fms TTL=%i type=%i"),
						ipstr(stDestAddr),
						nRet,
						usResTime, 
						64 - (reply->ttl & 63),
						icmphdr->type);
				}
			}
		}
		if (usResTime >= timeout) {
			if (toNowTimeOut < 10) toNowTimeOut++;
		}
	}
	// UDPing reworked ping sequence end <--


	// RAWPing ping sequence -->
	if (mPingMode == pingRAW) {

		// pBC
		int nTTL = ttl;
		sockaddr_in sa;
		int nAddrLen = sizeof(struct sockaddr_in); 
		char bufICMP[1500];             // allow full MTU

		// clear ICMP socket before sending UDP - not best solution, but may be needed to exclude late responses etc
		u_long bytes2read = 0;
		do {
			int nRet = ioctlsocket(sockRaw, FIONREAD, &bytes2read);
			if (bytes2read > 0) {       // ignore errors here
				sa.sin_family = AF_INET;
				// NEO: MOD - [BindToAdapter] -- Xanatos -->
				if (theApp.GetBindAddress())
					sa.sin_addr.s_addr = inet_addr(CT2CA(theApp.GetBindAddress()));
				else
				// NEO: MOD END <-- Xanatos --
					sa.sin_addr.s_addr = INADDR_ANY;	
				sa.sin_port = 0;

				nRet = recvfrom (sockRaw,  /* socket */ 
					(LPSTR)bufICMP,        /* buffer */ 
					1500,                  /* length */ 
					0,                     /* flags  */ 
					(sockaddr*)&sa,        /* source */ 
					&nAddrLen);            /* addrlen*/
			}
		} while (bytes2read > 0);

		// set TTL value for packet - should success with winsock 2
		// NB! take care about IP_TTL value - it's redefined in Ws2tcpip.h!
		// TODO: solve next problem correctly:
		// eMule is linking sockets functions using wsock32.lib (IP_TTL=7)
		// to use IP_TTL define, we must enforce linker to bind this function 
		// to ws2_32.lib (IP_TTL=4) (linker options: ignore wsock32.lib)
		int nRet = setsockopt(sockRaw, IPPROTO_IP, 4/*IP_TTL*/, (char*)&nTTL, sizeof(int));
		if (nRet==SOCKET_ERROR) { 
			lastError = WSAGetLastError();
			returnValue.success = false;
			returnValue.delay = timeout;
			returnValue.error = lastError;
			return returnValue;
		} 

		sa.sin_family = AF_INET;
		sa.sin_addr.s_addr = lAddr;	
		//sa.sin_port = htons(UDP_PORT);
		// <-- pBC

		UCHAR nPacketSize = 4;

		//Fille the ICMP packet
		int nBufSize = sizeof(ICMPHeader) + nPacketSize;
		FillIcmpData((ICMPHeader*) bufICMP, nBufSize);

		//Get the tick count prior to sending the packet
		LARGE_INTEGER TimerTick;
		VERIFY(QueryPerformanceCounter(&TimerTick));
		__int64 nStartTick = TimerTick.QuadPart;

		//Send of the packet
		int nWrote = sendto(sockRaw, bufICMP, nBufSize, 0, (sockaddr*)&sa, sizeof(sa));
		if (nWrote==SOCKET_ERROR) { 
			lastError = WSAGetLastError();
			returnValue.success = false;
			returnValue.error = lastError;
			return returnValue;
		} 

		// Loop amd read the socket unti timeout or we got iur packet
		uint16 timePased = 0;
		while(timePased < timeout)
		{
			int nRead = 0;
			BOOL bReadable;
			//Allow the specified timeout
			if (IsSocketReadible(sockRaw, timeout - timePased, bReadable)) // we will make heare a pause to share time
			{
				if (bReadable)
				{
					// NEO: MOD - [BindToAdapter] -- Xanatos -->
					if (theApp.GetBindAddress())
						sa.sin_addr.s_addr = inet_addr(CT2CA(theApp.GetBindAddress()));
					else
					// NEO: MOD END <-- Xanatos --
						sa.sin_addr.s_addr = INADDR_ANY;	
					//sa.sin_port = 0;

					//Receive the response
					nRead = recvfrom(sockRaw, bufICMP, MAX_ICMP_PACKET_SIZE, 0, (sockaddr*)&sa, &nAddrLen);

					//Get the current tick count
					VERIFY(QueryPerformanceCounter(&TimerTick));
					timePased = (uint16)(usResTime = (float)(TimerTick.QuadPart - nStartTick) * 1000 / m_TimerFrequency);

					//Now check the return response from recvfrom
					if (nRead == SOCKET_ERROR)
					{
						lastError = WSAGetLastError();
						returnValue.success = false;
						returnValue.error = lastError;
						returnValue.delay = timeout;
						if (doLog) {
							theApp.QueueDebugLogLine(false,_T("Error from %s: time=%3.2fms Error=%i"),
								ipstr(sa.sin_addr),
								usResTime, 
								returnValue.error);
						}
						return returnValue;
					}
				}
				else
				{
					//set the error to timed out
					returnValue.success = false;
					//returnValue.error = WSAETIMEDOUT;
					returnValue.delay = timeout;
					if (doLog) {
						theApp.QueueDebugLogLine(false,_T("Timeout from %s: time=%3.2fms"),
							ipstr(lAddr),
							timeout);
					}
					return returnValue;
				}
			}
			else
			{
				lastError = GetLastError();
				returnValue.success = false;
				returnValue.error = lastError;
				if (doLog) {
					theApp.QueueDebugLogLine(false,_T("Error from %s: time=%3.2fms Error=%i"),
						ipstr(lAddr),
						usResTime, 
						returnValue.error);
				}
				return returnValue;
			}

			//Decode the response we got back
			IPHeader* pIpHdr = (IPHeader*) bufICMP;
			unsigned short header_len = pIpHdr->h_len * 4;
			ICMPHeader* pIcmpHdr = (ICMPHeader*)(bufICMP + header_len);
			IN_ADDR stDestAddr;
			stDestAddr.s_addr = pIpHdr->source_ip;

			if(((pIcmpHdr->type == ICMP_TTL_EXPIRE) || (pIcmpHdr->type == ICMP_DEST_UNREACH))
			 && (pIcmpHdr->hdrsent.dest_ip == lAddr))
			{
				if(pIcmpHdr->type == ICMP_TTL_EXPIRE) {
					returnValue.success = true;
					returnValue.status = IP_TTL_EXPIRED_TRANSIT;
					returnValue.delay = usResTime;
					returnValue.destinationAddress = stDestAddr.s_addr;
					returnValue.ttl = ttl;
				} else {
					returnValue.success = true;
					returnValue.status = IP_DEST_HOST_UNREACHABLE;
					returnValue.delay = usResTime;
					returnValue.destinationAddress = stDestAddr.s_addr;
					returnValue.ttl = 64 - (pIpHdr->ttl & 63);
				}

				if(doLog) {
					theApp.QueueDebugLogLine(false,_T("Reply from %s: bytes=%d time=%3.2fms TTL=%i"),
						ipstr(stDestAddr.s_addr),
						nRead,
						returnValue.delay, 
						returnValue.ttl);
				}

				break;
			}
			else {              // verbose log filtered packets info (not seen yet...)
				if(doLog) {
					theApp.QueueDebugLogLine(false,_T("Filtered reply from %s: bytes=%d time=%3.2fms TTL=%i type=%i"),
						ipstr(stDestAddr),
						nRead,
						usResTime, 
						64 - (pIpHdr->ttl & 63),
						pIcmpHdr->type);
				}
			}
		}

	}
	// RAWPing ping sequence end <--

	if (mPingMode == pingICMP) {        // UDPing check

		IN_ADDR stDestAddr;
		char achRepData[sizeof(ICMPECHO) + BUFSIZE];

		// Address is assumed to be ok
		stDestAddr.s_addr = lAddr;
		stIPInfo.Ttl = (u_char)ttl;

		m_time.Tick();
		// Send the ICMP Echo Request and read the Reply
		DWORD dwReplyCount = lpfnIcmpSendEcho(hICMP, 
			stDestAddr.s_addr,
			0, // databuffer
			0, // DataLen, length of databuffer
			&stIPInfo, 
			achRepData, 
			sizeof(achRepData), 
			timeout
			);
		usResTime=m_time.Tick();
		if (dwReplyCount != 0) {
			IN_ADDR stDestAddr;

			stDestAddr.s_addr = *(u_long *)achRepData;

			returnValue.success = true;
			returnValue.status = *(DWORD *) &(achRepData[4]);
			returnValue.delay = (m_time.isPerformanceCounter() ? usResTime : ((float)*(u_long *) &(achRepData[8])));
			returnValue.destinationAddress = stDestAddr.s_addr;
			returnValue.ttl = (returnValue.status != IP_SUCCESS)?ttl:(*(char *)&(achRepData[20]))&0x00FF;
			if(doLog) {
				theApp.QueueDebugLogLine(false,_T("Reply from %s: bytes=%d time=%3.2fms TTL=%i Status=%i"),
					ipstr(stDestAddr),
					*(u_long *) &(achRepData[12]),
					returnValue.delay, 
					returnValue.ttl, returnValue.status);
			}
		} else {
			DWORD lastError = GetLastError();
			returnValue.success = false;
			returnValue.error = lastError;
			returnValue.delay = timeout;
			if (doLog) {
				theApp.QueueDebugLogLine(false,_T("Error from %s: time=%3.2fms Error=%i"),
					ipstr(stDestAddr),
					usResTime, 
					returnValue.error);
			}
		}
	}                                   // UDPing check

	return returnValue;
}


void Pinger::PIcmpErr(int nICMPErr) {
	int  nErrIndex = nICMPErr - IP_STATUS_BASE;

	if ((nICMPErr > MAX_ICMP_ERR_STRING) || 
		(nICMPErr < IP_STATUS_BASE+1)) {

			// Error value is out of range, display normally
			theApp.QueueDebugLogLine(false,_T("(%d) "), nICMPErr);
			DisplayErr(nICMPErr);
		} else {

			// Display ICMP Error String
			theApp.QueueDebugLogLine(false,_T("%s"), CString(aszSendEchoErr[nErrIndex]));
		}
}

void Pinger::DisplayErr(int nWSAErr) {
	TCHAR* lpMsgBuf; // Pending: was LPVOID

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		nWSAErr,
		MAKELANGID(LANG_NEUTRAL, 
		SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,    
		NULL );   
	theApp.QueueDebugLogLine(false,lpMsgBuf);

	// Free the buffer
	LocalFree(lpMsgBuf);
}

// RAWPing Functions
BOOL IsSocketReadible(SOCKET socket, DWORD dwTimeout, BOOL& bReadible)
{
	timeval timeout = {dwTimeout/1000, dwTimeout % 1000};
	fd_set fds;
	FD_ZERO(&fds);
#pragma warning(disable:4127) // conditional expression is constant
	FD_SET(socket, &fds);
#pragma warning(default:4127) // conditional expression is constant
	int nStatus = select(0, &fds, NULL, NULL, &timeout);
	if (nStatus == SOCKET_ERROR)
	{
		return FALSE;
	}
	else
	{
		bReadible = !(nStatus == 0);
		return TRUE;
	}
}

//Decode the raw Ip packet we get back
BOOL DecodeResponse(char* pBuf, int nBytes) 
{
	IPHeader* pIpHdr = (IPHeader*) pBuf;
	int nIpHdrlen = pIpHdr->h_len * 4; //Number of 32-bit words*4 = bytes

	//Not enough data recieved
	if (nBytes < nIpHdrlen + MIN_ICMP_PACKET_SIZE) 
	{
		TRACE(_T("Received too few bytes from %d\n"),nBytes);
		SetLastError(ERROR_UNEXP_NET_ERR);
		return FALSE;
	}

	//Check it is an ICMP_ECHOREPLY packet
	ICMPHeader* pIcmpHdr = (ICMPHeader*) (pBuf + nIpHdrlen);
	if (pIcmpHdr->type != 0) //type ICMP_ECHOREPLY is 0
	{
		TRACE(_T("non-echo type %d recvd\n"), pIcmpHdr->type);
		SetLastError(ERROR_UNEXP_NET_ERR);
		return FALSE;
	}

	//Check it is the same id as we sent
	if (pIcmpHdr->id != (USHORT)GetCurrentProcessId()) 
	{
		TRACE(_T("Received someone else's packet!\n"));
		SetLastError(ERROR_UNEXP_NET_ERR);
		return FALSE;
	}

	return TRUE;
}

//Fill up the ICMP packet with defined values
void FillIcmpData(ICMPHeader* pIcmp, int nData)
{
	pIcmp->type		= ICMP_ECHO_REQUEST;
	pIcmp->code		= 0;
	pIcmp->id		= (USHORT) GetCurrentProcessId();
	pIcmp->seq		= 0;
	pIcmp->checksum = 0;

	//Set up the data which will be sent
	int nHdrSize = sizeof(ICMPHeader);
	char* pData = ((char*)pIcmp) + nHdrSize;
	memset(pData, 'E', nData - nHdrSize);

	//Generate the checksum
	pIcmp->checksum = GenerateIPChecksum((USHORT*)pIcmp, nData);
}

//generate an IP checksum based on a given data buffer
USHORT GenerateIPChecksum(USHORT* pBuffer, int nSize) 
{
	unsigned long cksum = 0;

	while (nSize > 1) 
	{
		cksum += *pBuffer++;
		nSize -= sizeof(USHORT);
	}

	if (nSize) 
		cksum += *(UCHAR*)pBuffer;

	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);
	return (USHORT)(~cksum);
}

// RAWPing Functions end
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
