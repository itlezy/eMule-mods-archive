//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "StdAfx.h"
#include "emule.h"
#include "preferences.h"
#include "UPnPImplMiniLib.h"
#include "Log.h"
#include "Otherfunctions.h"
#include ".\miniupnpc\miniupnpc.h"
#include ".\miniupnpc\upnpcommands.h"
#include ".\miniupnpc\upnperrors.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CMutex CUPnPImplMiniLib::m_mutBusy;

CUPnPImplMiniLib::CUPnPImplMiniLib(){
	m_nOldUDPPort = 0;
	m_nOldTCPPort = 0;
	m_pURLs = NULL;
	m_pIGDData = NULL;
	m_bAbortDiscovery = false;
}
	
CUPnPImplMiniLib::~CUPnPImplMiniLib(){
	if (m_pURLs != NULL)
		FreeUPNPUrls(m_pURLs);
	delete m_pURLs;
	m_pURLs = NULL;
	delete m_pIGDData;
	m_pIGDData = NULL;
}

bool CUPnPImplMiniLib::IsReady(){
	// the only check we need to do is if we are already busy with some async/threaded function
	CSingleLock lockTest(&m_mutBusy);
	 if (!m_bAbortDiscovery && lockTest.Lock(0))
		 return true;
	 else
		 return false;
}

void CUPnPImplMiniLib::StopAsyncFind(){
	CSingleLock lockTest(&m_mutBusy);
	m_bAbortDiscovery = true; // if there is a thread, tell him to abort as soon as possible - he won't sent a Resultmessage when aborted
	if (!lockTest.Lock(7000)) // give the thread 7 seconds to exit gracefully - it should never really take that long
	{
		// that quite bad, something seems to be locked up. There isn't a good solution here, we need the thread to quit
		// or it might try to access a deleted object later, but termianting him is quite bad too. Well..
		DebugLogError(_T("Waiting for UPnP StartDiscoveryThread to quit failed, trying to terminate the thread..."));
		if (m_hThreadHandle != 0){
			if (TerminateThread(m_hThreadHandle, 0))
				DebugLogError(_T("...OK"));
			else
				DebugLogError(_T("...Failed"));
		}
		else
			ASSERT( false );
		m_hThreadHandle = 0;
	}
	else {
		DebugLog(_T("Aborted any possible UPnP StartDiscoveryThread"));
		m_bAbortDiscovery = false;
		m_hThreadHandle = 0;
	}
}

void CUPnPImplMiniLib::DeletePorts()
{ 
	m_nOldUDPPort = (ArePortsForwarded() == TRIS_TRUE) ? m_nUDPPort : 0;
	m_nUDPPort = 0;
	m_nOldTCPPort = (ArePortsForwarded() == TRIS_TRUE) ? m_nTCPPort : 0;
	m_nTCPPort = 0;
	m_bUPnPPortsForwarded = TRIS_FALSE;
	DeletePorts(false); 
}

void CUPnPImplMiniLib::DeletePorts(bool bSkipLock){
	// this function itself blocking, because its called at the end of eMule and we need to wait for it to finish
	// before going on anyway. It might be caled from the non-blocking StartDiscovery() function too however
	CSingleLock lockTest(&m_mutBusy);
	if (bSkipLock || lockTest.Lock(0)){
		if (m_nOldTCPPort != 0){
			if (m_pURLs == NULL || m_pIGDData == NULL){
				ASSERT( false );
				return;
			}
			const char achTCP[] = "TCP";
			char achPort[10];
			sprintf(achPort, "%u", m_nOldTCPPort);
			int nResult = UPNP_DeletePortMapping(m_pURLs->controlURL, m_pIGDData->servicetype, achPort, achTCP);
			if (nResult == UPNPCOMMAND_SUCCESS){
				DebugLog(_T("Sucessfully removed mapping for port %u (%s)"), m_nOldTCPPort, _T("TCP"));
				m_nOldTCPPort = 0;
			}
			else
				DebugLogWarning(_T("Failed to remove mapping for port %u (%s)"), m_nOldTCPPort, _T("TCP"));

		}
		else{
			DebugLog(_T("No UPnP Mappings to remove, aborting"));
			return; // UDP port cannot be set if TCP port was empty
		}

		if (m_nOldUDPPort != 0){
			const char achTCP[] = "UDP";
			char achPort[10];
			sprintf(achPort, "%u", m_nOldUDPPort);
			int nResult = UPNP_DeletePortMapping(m_pURLs->controlURL, m_pIGDData->servicetype, achPort, achTCP);
			if (nResult == UPNPCOMMAND_SUCCESS){
				DebugLog(_T("Sucessfully removed mapping for port %u (%s)"), m_nOldUDPPort, _T("UDP"));
				m_nOldTCPPort = 0;
			}
			else
				DebugLogWarning(_T("Failed to remove mapping for port %u (%s)"), m_nOldUDPPort, _T("UDP"));
		}
	}
	else
		DebugLogError(_T("Unable to remove port mappings - implementation still busy"));

}

void CUPnPImplMiniLib::StartDiscovery(uint16 nTCPPort, uint16 nUDPPort){
	DebugLog(_T("Using MiniUPnPLib based implementation"));
	DebugLog(_T("miniupnpc (c) 2006-2008 Thomas Bernard - http://miniupnp.free.fr/"));
	m_nOldUDPPort = (ArePortsForwarded() == TRIS_TRUE) ? m_nUDPPort : 0;
	m_nUDPPort = nUDPPort;
	m_nOldTCPPort = (ArePortsForwarded() == TRIS_TRUE) ? m_nTCPPort : 0;
	m_nTCPPort = nTCPPort;
	m_bUPnPPortsForwarded = TRIS_UNKNOWN;

	if (m_pURLs != NULL)
		FreeUPNPUrls(m_pURLs);
	delete m_pURLs;
	m_pURLs = NULL;
	delete m_pIGDData;
	m_pIGDData = NULL;

	if (m_bAbortDiscovery)
		return;

	CStartDiscoveryThread* pStartDiscoveryThread = (CStartDiscoveryThread*) AfxBeginThread(RUNTIME_CLASS(CStartDiscoveryThread), THREAD_PRIORITY_NORMAL,0, CREATE_SUSPENDED);
	m_hThreadHandle = pStartDiscoveryThread->m_hThread;
	pStartDiscoveryThread->SetValues(this);
	pStartDiscoveryThread->ResumeThread();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/// CUPnPImplMiniLib::CStartDiscoveryThread Implementation
typedef CUPnPImplMiniLib::CStartDiscoveryThread CStartDiscoveryThread;
IMPLEMENT_DYNCREATE(CStartDiscoveryThread, CWinThread)

CUPnPImplMiniLib::CStartDiscoveryThread::CStartDiscoveryThread()
{
	m_pOwner = NULL;
}

BOOL CUPnPImplMiniLib::CStartDiscoveryThread::InitInstance()
{
	InitThreadLocale();
	return TRUE;
}

int CUPnPImplMiniLib::CStartDiscoveryThread::Run()
{
	DbgSetThreadName("CUPnPImplMiniLib::CStartDiscoveryThread");
	if ( !m_pOwner )
		return 0;

	CSingleLock sLock(&m_pOwner->m_mutBusy);
	if (!sLock.Lock(0)){
		DebugLogWarning(_T("CUPnPImplMiniLib::CStartDiscoveryThread::Run, failed to acquire Lock, another Mapping try might be running already"));
		return 0;
	}

	if (m_pOwner->m_bAbortDiscovery) // requesting to abort ASAP?
		return 0;

	UPNPDev* structDeviceList = upnpDiscover(2000, NULL, NULL);
	if (structDeviceList == NULL){
		DebugLog(_T("UPNP: No Internet Gateway Devices found, aborting"));
		m_pOwner->m_bUPnPPortsForwarded = TRIS_FALSE;
		m_pOwner->SendResultMessage();
		return 0;
	}

	if (m_pOwner->m_bAbortDiscovery){ // requesting to abort ASAP?
		freeUPNPDevlist(structDeviceList);
		return 0;
	}

	DebugLog(_T("List of UPNP devices found on the network:"));
	for(UPNPDev* pDevice = structDeviceList; pDevice != NULL; pDevice = pDevice->pNext)
	{
		DebugLog(_T("Desc: %S, st: %S"), pDevice->descURL, pDevice->st);
	}
	m_pOwner->m_pURLs = new UPNPUrls;
	ZeroMemory(m_pOwner->m_pURLs, sizeof(UPNPUrls));
	m_pOwner->m_pIGDData = new IGDdatas;
	ZeroMemory(m_pOwner->m_pIGDData, sizeof(IGDdatas));
	char achLanIP[16];
	achLanIP[0] = 0;

	int iResult = UPNP_GetValidIGD(structDeviceList, m_pOwner->m_pURLs, m_pOwner->m_pIGDData, achLanIP, sizeof(achLanIP));
	switch (iResult){
		case 1:
			DebugLog(_T("Found valid IGD : %S"), m_pOwner->m_pURLs->controlURL);
			break;
		case 2:
			DebugLog(_T("Found a (not connected?) IGD : %S - Trying to continue anyway"), m_pOwner->m_pURLs->controlURL);
			break;
		case 3:
			DebugLog(_T("UPnP device found. Is it an IGD ? : %S - Trying to continue anyway"), m_pOwner->m_pURLs->controlURL);
			break;
		default:
			DebugLog(_T("Found device (igd ?) : %S - Trying to continue anyway"), m_pOwner->m_pURLs->controlURL);
	}
	freeUPNPDevlist(structDeviceList);
	DebugLog(_T("Our LAN IP: %S"), achLanIP);

	if (m_pOwner->m_bAbortDiscovery) // requesting to abort ASAP?
		return 0;

	// do we still have old mappings? Remove them first
	m_pOwner->DeletePorts(true);
	
	bool bSucceeded = OpenPort(m_pOwner->m_nTCPPort, true, achLanIP);
	if (bSucceeded && m_pOwner->m_nUDPPort != 0)
		bSucceeded = OpenPort(m_pOwner->m_nUDPPort, false, achLanIP);

	if (!m_pOwner->m_bAbortDiscovery){ // dont send a result on a abort request
		m_pOwner->m_bUPnPPortsForwarded = bSucceeded ? TRIS_TRUE : TRIS_FALSE;
		m_pOwner->SendResultMessage();
	}
	return 0;
}

bool CUPnPImplMiniLib::CStartDiscoveryThread::OpenPort(uint16 nPort, bool bTCP, char* pachLANIP){
	const char achTCP[] = "TCP";
	const char achUDP[] = "UDP";
	const char achDescTCP[] = "eMule_TCP";
	const char achDescUDP[] = "eMule_UDP";
	char achPort[10];
	sprintf(achPort, "%u", nPort);
	
	if (m_pOwner->m_bAbortDiscovery)
		return false;

	int nResult;
	if (bTCP)
		nResult = UPNP_AddPortMapping(m_pOwner->m_pURLs->controlURL, m_pOwner->m_pIGDData->servicetype
		, achPort, achPort, pachLANIP, achDescTCP, achTCP);
	else
		nResult = UPNP_AddPortMapping(m_pOwner->m_pURLs->controlURL, m_pOwner->m_pIGDData->servicetype
		, achPort, achPort, pachLANIP, achDescUDP, achUDP);

	if (nResult != UPNPCOMMAND_SUCCESS){
		DebugLog(_T("Adding PortMapping failed, Error Code %u"), nResult);
		return false;
	}

	if (m_pOwner->m_bAbortDiscovery)
		return false;

	// make sure it really worked
	char achOutIP[20];
	achOutIP[0] = 0;
	if (bTCP)
		nResult = UPNP_GetSpecificPortMappingEntry(m_pOwner->m_pURLs->controlURL, m_pOwner->m_pIGDData->servicetype
		, achPort, achTCP, achOutIP, achPort);
	else
		nResult = UPNP_GetSpecificPortMappingEntry(m_pOwner->m_pURLs->controlURL, m_pOwner->m_pIGDData->servicetype
		, achPort, achUDP, achOutIP, achPort);

	if (nResult == UPNPCOMMAND_SUCCESS && achOutIP[0] != 0){
		DebugLog(_T("Sucessfully added mapping for port %u (%s) on local IP %S"), nPort, bTCP ? _T("TCP") : _T("UDP"), achOutIP);
		return true;
	}
	else {
		DebugLogWarning(_T("Failed to verfiy mapping for port %u (%s) on local IP %S - considering as failed"), nPort, bTCP ? _T("TCP") : _T("UDP"), achOutIP);
		// maybe counting this as error is a bit harsh as this may lead to false negatives, however if we would risk false postives
		// this would mean that the fallback implementations are not tried because eMule thinks it worked out fine
		return false;
	}
}
