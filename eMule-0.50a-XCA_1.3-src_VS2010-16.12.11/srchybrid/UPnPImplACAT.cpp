//this file is part of eMule Xtreme-Mod (http://www.xtreme-mod.net)
//Copyright (C)2002-2007 Xtreme-Mod (emulextreme@yahoo.de)

//emule Xtreme is a modification of eMule
//Copyright (C)2002-2007 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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

//
//
//	Author: Acat-Team
//  
//  last changes:
//  12.06.2006 fixed wrong IP-LAN-Detection (Xman)
//  12.06.2006 fixed warnings due to wrong Datatypes (Xman)
//  03.10.2006 added function: RemoveSpecifiedPort(), needed for Restart of Listening (Xman)
//


#include "StdAfx.h"
#include <Iphlpapi.h>
#include "preferences.h"
#include "UPnPImplACAT.h"
#include "Log.h"
#include "Otherfunctions.h"
#include "Resource.h" //zz_fly :: show UPnP status

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define UPNPPORTMAP0   "WANIPConnection"
#define UPNPPORTMAP1   "WANPPPConnection"
#define UPNPADDPORTMAP "AddPortMapping"
#define UPNPDELPORTMAP "DeletePortMapping"
#define UPNPQRYPORTMAP "GetSpecificPortMappingEntry"

const uint32	UPNPADDR = 0xFAFFFFEF;
const int	UPNPPORT = 1900;

Poco::FastMutex CUPnPImplACAT::m_mutBusy;

const CStringA getProperty(const CStringA& all, const CStringA& name)
{
	CStringA startTag = '<' + name + '>';
	CStringA endTag = "</" + name + '>';
	CStringA property;

	int posStart = all.Find(startTag);
	if (posStart<0) return CStringA();

	int posEnd = all.Find(endTag, posStart);
	if (posStart>=posEnd) return CStringA();

	return all.Mid(posStart + startTag.GetLength(), posEnd - posStart - startTag.GetLength());
}

static CStringA NGetAddressFromUrl(const CStringA& str, CStringA& post, CStringA& host, int& port)
{
	CStringA s = str;

	post.Empty();
	host = post;
	port = 0;
	int pos = s.Find("://");
	if (!pos) return CStringA();
	s.Delete(0, pos + 3);

	pos = s.Find('/');
	if (!pos) {
		host = s;
		s.Empty();
	} else {
		host = s.Mid(0, pos);
		s.Delete(0, pos);
	}

	if (s.IsEmpty()) {
		post.Empty();
	} else {
		post = s;
	}

	pos = 0;
	CStringA addr = host.Tokenize(":", pos);
	s = host.Tokenize(":", pos);
	if (s.IsEmpty()) {
		port = 80;
	} else {
		port = atoi(s);
	}

	return addr;
}

CUPnPImplACAT::CUPnPImplACAT(){
	m_bAbortDiscovery = false;
	m_bSucceededOnce = false;
	m_uLocalIP = 0;
}

bool CUPnPImplACAT::Search()
{
#define NUMBEROFDEVICES	2
	const char* devices[][2] = {
		{UPNPPORTMAP1, "service"},
		{UPNPPORTMAP0, "service"},
		{"InternetGatewayDevice", "device"},
	};

	MIB_IPFORWARDROW ip_forward;
	if ( GetBestRoute(/*inet_addr("223.255.255.255")*/0xffffffdf, 0, &ip_forward) == NO_ERROR)
	{
		SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
		int timeout = 500;
		setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
		struct sockaddr_in sockaddr;
		memset(&sockaddr, 0, sizeof(sockaddr));
		sockaddr.sin_family = AF_INET;
		sockaddr.sin_port = htons(UPNPPORT);
		sockaddr.sin_addr.S_un.S_addr = ip_forward.dwForwardNextHop;
		int rlen = 0;
		for (INT_PTR i=0; rlen<=0 && i<500; i++) {
			if (!(i%100)) {
				for (INT_PTR i=0; i<NUMBEROFDEVICES; i++) {
					m_name.Format("urn:schemas-upnp-org:%s:%s:1", devices[i][1], devices[i][0]);
					CStringA request;
					request.Format("M-SEARCH * HTTP/1.1\r\nHOST: 239.255.255.250:1900\r\nMAN: \"ssdp:discover\"\r\nMX: 6\r\nST: %s\r\n\r\n", m_name);
					sendto(s, request, request.GetLength(), 0, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
				}
			}

			if (m_bAbortDiscovery) // requesting to abort ASAP?
				break;

			char buffer[10240];
			rlen = recv(s, buffer, sizeof(buffer) - 1, 0);
			if (rlen <= 0) continue;
			closesocket(s);
			buffer[rlen] = 0;
			char* pFound = strchr(buffer, ' ');
			if(pFound == NULL || pFound[1] != '2')
				return false;

			for (INT_PTR d=0; d<NUMBEROFDEVICES; d++) {
				m_name.Format("urn:schemas-upnp-org:%s:%s:1", devices[d][1], devices[d][0]);
				if (strstr(pFound, m_name)) {
					pFound = (char*)stristr(pFound, "\r\nlocation:");
					if(pFound)
					{
						pFound += 11;
						char* pCrlf = strstr(pFound, "\r\n");
						if(pCrlf) *pCrlf = 0;
						m_description = pFound;
						m_description.Trim();
						return GetDescription();
					}
				}
			}
		}
		closesocket(s);
	}

	return false;
}

bool CUPnPImplACAT::GetDescription()
{
	if(m_name.IsEmpty() || m_description.IsEmpty()) return false;
	CStringA post, host, addr;
	int port = 0;
	addr = NGetAddressFromUrl(m_description, post, host, port);
	if(addr.IsEmpty())return false;
	CStringA request;
	request.Format(
		"GET %s HTTP/1.1\r\n"
		"HOST: %s\r\n"
		"ACCEPT-LANGUAGE: en\r\n\r\n"
		, post
		, host
	);
	CStringA response;
	struct sockaddr_in sockaddr;
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	sockaddr.sin_addr.S_un.S_addr = inet_addr(addr);
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	int timeout = 50;
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
	connect(s, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
	/*int n =*/ send(s, request, request.GetLength(), 0);
	char buffer[10240];
	int rlen;
	int datalen = 0;
	do{
		rlen = recv(s, buffer + datalen, sizeof(buffer) - 1 - datalen, 0);
		if (rlen == SOCKET_ERROR)
		{
			if(WSAGetLastError() == WSAETIMEDOUT)
				break;
			closesocket(s);
			return false;
		}
		datalen += rlen;
	}
	while(rlen > 0);
	closesocket(s);
	if (!datalen) return false;
	buffer[datalen] = 0;
	response = buffer;
	char* pFound = strchr(buffer, ' ');
	if(pFound == NULL || pFound[1] != '2')
		return false;
	CStringA result = pFound;

	//m_friendlyname = getProperty(result, "friendlyName");
	//m_modelname = getProperty(result, "modelName");
	CStringA baseurl = getProperty(result, "URLBase");
	if(baseurl.IsEmpty())baseurl = "http://" + host + '/';
	if(baseurl[baseurl.GetLength() - 1]!='/')baseurl += '/';

	CStringA serviceType = "<serviceType>" + m_name + "</serviceType>";
	int pos = result.Find(serviceType);
	if (pos >= 0) {
		result.Delete(0, pos + serviceType.GetLength());
		pos = result.Find("</service>");
		if (pos >= 0) {
			result = result.Mid(0, pos);
			m_controlurl = getProperty(result, "controlURL");
			if (!m_controlurl.IsEmpty() && m_controlurl[0] == '/') {
				m_controlurl = baseurl + m_controlurl.Mid(1);
			}
		}
	}

	return isComplete();
}

bool CUPnPImplACAT::InvokeCommand(const char* name, const char* args)
{
	if(!isComplete())return false;
	CStringA post, host, addr;
	int port = 0;
	addr = NGetAddressFromUrl(m_controlurl, post, host, port);
	if(addr.IsEmpty())return false;

	CStringA cnt;
	cnt.Format(
		"<?xml version=\"1.0\"?>"
		"<s:Envelope\r\n"
		"xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"\r\n"
		"    s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n"
		"  <s:Body>\r\n"
		"    <u:%s xmlns:u=\"%s\">\r\n"
		"%s"
		"    </u:%s>\r\n"
		"  </s:Body>\r\n</s:Envelope>\r\n\r\n"		
		, name, m_name
		, args
		, name);
	CStringA request;
	request.Format(
		"POST %s HTTP/1.1\r\n"
		"HOST: %s\r\n"
		"Content-Length: %d\r\n"
		"Content-Type: text/xml; charset=\"utf-8\"\r\n"
		"SOAPAction: \"%s#%s\"\r\n\r\n"
		"%s"
		, post
		, host
		, cnt.GetLength()
		, m_name, name
		, cnt
	);

	struct sockaddr_in sockaddr;
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	sockaddr.sin_addr.S_un.S_addr = inet_addr(addr);
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	int timeout = 500;
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
	connect(s, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
	/*int n =*/ send(s, request, request.GetLength(), 0);
	char buffer[32];
	int datalen = recv(s, buffer, 31, 0);
	closesocket(s);
	if (datalen == SOCKET_ERROR || datalen == 0)
		return false;
	buffer[datalen] = 0;
	char* pFound = strchr(buffer, ' ');
	return pFound && pFound[1] == '2';
}

/////////////////////////////////////////////////////////////////////////////////
// Returns the Local IP
/////////////////////////////////////////////////////////////////////////////////
DWORD CUPnPImplACAT::GetLocalIP()
{
	m_uLocalIP = GetHostIP();
	if(m_uLocalIP != 0)
	{
		struct in_addr addr;
		addr.S_un.S_addr = m_uLocalIP;
		m_slocalIP = inet_ntoa(addr);
		m_uLocalIP = m_uLocalIP;
	}
	return m_uLocalIP;
}

/////////////////////////////////////////////////////////////////////////////////
// Returns a CString with the local IP in format xxx.xxx.xxx.xxx
/////////////////////////////////////////////////////////////////////////////////
CStringA CUPnPImplACAT::GetLocalIPStr()
{
	if(m_uLocalIP == 0)
		GetLocalIP();

	return m_slocalIP;
}

bool CUPnPImplACAT::IsReady(){
	// the only check we need to do is if we are already busy with some async/threaded function
	Poco::FastMutex::SingleLock lockTest(m_mutBusy);
	 if (!m_bAbortDiscovery && lockTest.tryLock())
		 return true;
	 return false;
}

void CUPnPImplACAT::StopAsyncFind(){
	Poco::FastMutex::SingleLock lockTest(m_mutBusy);
	m_bAbortDiscovery = true; // if there is a thread, tell him to abort as soon as possible - he won't sent a Resultmessage when aborted
	if (!lockTest.Lock(7000)) // give the thread 7 seconds to exit gracefully - it should never really take that long
	{
		// that quite bad, something seems to be locked up. There isn't a good solution here, we need the thread to quit
		// or it might try to access a deleted object later, but termianting him is quite bad too. Well..
		DebugLogError(_T("Waiting for UPnP StartDiscoveryThread to quit failed, trying to terminate the thread..."));
		if (isRunning()){
			DebugLogError(terminate()?_T("...OK"):_T("...Failed"));
		}
		else
			ASSERT( false );
	}
	else {
		DebugLog(_T("Aborted any possible UPnP StartDiscoveryThread"));
		m_bAbortDiscovery = false;
	}
}

void CUPnPImplACAT::DeletePorts()
{
	m_bUPnPPortsForwarded = TRIS_FALSE;
	// this function itself blocking, because its called at the end of eMule and we need to wait for it to finish
	// before going on anyway. It might be caled from the non-blocking StartDiscovery() function too however
	Poco::FastMutex::SingleLock lockTest(m_mutBusy);
	if (lockTest.tryLock()){
		for(size_t i = 0; i < PI_COUNT; ++i)
			DeletePort(m_Mappings[i]);
	}
	else
		DebugLogError(_T("Unable to remove port mappings - implementation still busy"));
}

void CUPnPImplACAT::DeletePort(UPNPNAT_MAPPING&mapping)
{
	if(mapping.externalPort == 0) return;
	const char* achProtocol = mapping.bTCP ? "TCP" : "UDP";
	CStringA args;
	args.Format(
		"<NewRemoteHost></NewRemoteHost>"
		"<NewExternalPort>%u</NewExternalPort>"
		"<NewProtocol>%s</NewProtocol>"
		, mapping.externalPort
		, achProtocol
	);

	if (InvokeCommand(UPNPDELPORTMAP, args)) 
		DebugLog(_T("Sucessfully removed mapping for port %u (%hs)"), mapping.externalPort, achProtocol);
	else
		DebugLogWarning(_T("Failed to remove mapping for port %u (%hs)"), mapping.externalPort, achProtocol);
	mapping.externalPort = 0;
}

void CUPnPImplACAT::StartDiscovery(uint16 nTCPPort, uint16 nUDPPort, uint16 nUDPServerPort, uint16 nTCPWebPort){
	DebugLog(_T("Using ACAT UPnPLib based implementation"));
	m_Mappings[PI_UDP].bChanged = (nUDPPort != 0);
	m_Mappings[PI_UDP].internalPort = nUDPPort;
	m_Mappings[PI_ServerUDP].bChanged = (nUDPServerPort != 0);
	m_Mappings[PI_ServerUDP].internalPort = nUDPServerPort;
	m_Mappings[PI_TCP].bChanged = (nTCPPort != 0);
	m_Mappings[PI_TCP].internalPort = nTCPPort;
	m_Mappings[PI_WebTCP].bChanged = (nTCPWebPort != 0);
	m_Mappings[PI_WebTCP].internalPort = nTCPWebPort;
	m_bUPnPPortsForwarded = TRIS_UNKNOWN;
	m_bCheckAndRefresh = false;

	m_uLocalIP = 0;
	m_slocalIP.Empty();
	m_name.Empty();
	m_description.Empty();
	m_controlurl.Empty();

	if (m_bAbortDiscovery)
		return;

	Thread::start();
}

bool CUPnPImplACAT::CheckAndRefresh()
{
	// on a CheckAndRefresh we don't do any new time consuming discovery tries, we expect to find the same router like the first time
	// and of course we also don't delete old ports (this was done on Discovery) but only check that our current mappings still exist
	// and refresh them if not
	if (m_bAbortDiscovery || !m_bSucceededOnce|| !IsReady() || !isComplete() || m_Mappings[PI_TCP].internalPort == 0)
	{
		DebugLog(_T("Not refreshing UPnP ports because they don't seem to be forwarded in the first place"));
		return false;
	}
	else
		DebugLog(_T("Checking and refreshing UPnP ports"));

	m_bCheckAndRefresh = true;
	Thread::start();
	return true;
}

void CUPnPImplACAT::run()
{
	DbgSetThreadName("CUPnPImplACAT::run");
	InitThreadLocale();
	Poco::FastMutex::SingleLock sLock(m_mutBusy);
	if (!sLock.tryLock()){
		DebugLogWarning(_T("CUPnPImplACAT::run, failed to acquire Lock, another Mapping try might be running already"));
		return;
	}

	if (m_bAbortDiscovery) // requesting to abort ASAP?
		return;

	bool bSucceeded = false;
#if !(defined(_DEBUG) || defined(_BETA))
	try
#endif
	{
		if (!m_bCheckAndRefresh)
		{
			if(!IsLANIP(GetLocalIP())){
				m_bUPnPPortsForwarded = TRIS_FALSE;
				SetStatusString(GetResString(IDS_UPNPSTATUS_NOTINLAN)); //zz_fly :: localize output
				SendResultMessage();
				return;
			}

			if (m_bAbortDiscovery) // requesting to abort ASAP?
				return;

			if (!Search()) {
				SetStatusString(GetResString(IDS_UPNPSTATUS_ERROR)); //zz_fly :: show UPnP status
				m_bUPnPPortsForwarded = TRIS_FALSE;
				SendResultMessage();
				return;
			}
			if (m_bAbortDiscovery) // requesting to abort ASAP?
				return;
		}

		ThreadLocalPtr<SFMT> p_rng(&t_rng);
		bool tryRandom = thePrefs.GetUPnPNatTryRandom();
		bSucceeded = OpenPort(m_Mappings[PI_UDP], tryRandom, m_bCheckAndRefresh);
		if (bSucceeded)
			bSucceeded = OpenPort(m_Mappings[PI_ServerUDP], tryRandom, m_bCheckAndRefresh);
		if (bSucceeded)
			bSucceeded = OpenPort(m_Mappings[PI_TCP], tryRandom, m_bCheckAndRefresh);
		if (bSucceeded)
			OpenPort(m_Mappings[PI_WebTCP], false, m_bCheckAndRefresh); // don't fail if only the webinterface port fails for some reason
	}
#if !(defined(_DEBUG) || defined(_BETA))
	catch(...)
	{
		DebugLogError(_T("Unknown Exception in CUPnPImplACAT::CStartDiscoveryThread::Run()"));
	}
#endif
	if (!m_bAbortDiscovery){ // dont send a result on a abort request
		m_bUPnPPortsForwarded = bSucceeded ? TRIS_TRUE : TRIS_FALSE;
		m_bSucceededOnce |= bSucceeded;
		SendResultMessage();
	}
}

bool CUPnPImplACAT::OpenPort(UPNPNAT_MAPPING&mapping, bool tryRandom, bool bCheckAndRefresh)
{
	if(mapping.internalPort == 0)
		return true;

	if (m_bAbortDiscovery)
		return false;

	const char*achProtocol = mapping.bTCP ? "TCP" : "UDP";
	CStringA args;
	if(mapping.bChanged)
	{
		DeletePort(mapping);
		mapping.bChanged = false;
		mapping.externalPort = mapping.internalPort;
	}
	else if (bCheckAndRefresh)
	{
		args.Format(
			"<NewRemoteHost></NewRemoteHost>"
			"<NewExternalPort>%u</NewExternalPort>"
			"<NewProtocol>%s</NewProtocol>"
			, mapping.externalPort
			, achProtocol
		);

		if (InvokeCommand(UPNPQRYPORTMAP, args)) {
			DebugLog(_T("Checking UPnP: Mapping for port %u (%hs) on local IP %hs still exists"), mapping.internalPort, achProtocol, GetLocalIPStr());
			return true;
		}
		else 
			DebugLogWarning(_T("Checking UPnP: Mapping for port %u (%hs) on local IP %hs is gone, trying to reopen port"), mapping.internalPort, achProtocol, GetLocalIPStr());
	}
	else if(mapping.externalPort == 0)
		mapping.externalPort = mapping.internalPort;

	for (int retries = 200; retries>0; retries--)
	{
		if (m_bAbortDiscovery)
		{
			mapping.externalPort = 0;
			return false;
		}

		args.Format(
			"<NewRemoteHost></NewRemoteHost>"
			"<NewExternalPort>%u</NewExternalPort>"
			"<NewProtocol>%s</NewProtocol>"
			"<NewInternalPort>%u</NewInternalPort>"
			"<NewInternalClient>%s</NewInternalClient>"
			"<NewEnabled>1</NewEnabled>"
			"<NewPortMappingDescription>%s [%s: %u]</NewPortMappingDescription>"
			"<NewLeaseDuration>0</NewLeaseDuration>"
			, mapping.externalPort
			, achProtocol
			, mapping.internalPort
			, GetLocalIPStr()
			, mapping.description, achProtocol, mapping.externalPort
		);
		if (InvokeCommand(UPNPADDPORTMAP, args))
			break;

		if (!tryRandom)
		{
			mapping.externalPort = 0;
			return false;
		}

		mapping.externalPort = (WORD)(2049 + t_rng->getUInt32() % (65535 - 2049));
	}

	args.Format(
		"<NewRemoteHost></NewRemoteHost>"
		"<NewExternalPort>%u</NewExternalPort>"
		"<NewProtocol>%s</NewProtocol>"
		, mapping.externalPort
		, achProtocol
	);

	if (InvokeCommand(UPNPQRYPORTMAP, args)) {
		DebugLog(_T("Sucessfully added mapping for port %u (%hs) on local IP %hs"), mapping.internalPort, achProtocol, GetLocalIPStr());
		return true;
	}
	DebugLogWarning(_T("Failed to verfiy mapping for port %u (%hs) on local IP %hs - considering as failed"), mapping.internalPort, achProtocol, GetLocalIPStr());
	// maybe counting this as error is a bit harsh as this may lead to false negatives, however if we would risk false postives
	// this would mean that the fallback implementations are not tried because eMule thinks it worked out fine
	mapping.externalPort = 0;
	return false;
}
