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

#include "StdAfx.h"
#include <share.h>
#include "DebugHelpers.h"
#include "emule.h"
#include "Voodoo.h"
#include "VoodooSocket.h"
#include "Preferences.h"
#include "Log.h"
#include "packets.h"
#include "downloadqueue.h"
#include "PartFile.h"
#include "Neo/LanCast/LanCast.h"
#include "eMuleDlg.h"
#include "downloadlistctrl.h"
#include "sharedfilelist.h"
#include "knownfilelist.h"
#include "transferwnd.h"
#include "Neo/functions.h"
#include "OpCodes.h"
#include "Neo/NeoOpCodes.h"
#include "UpDownClient.h"
#include "Uploadqueue.h"
// NEO: VOODOOs - [VoodooSearchForwarding]
#include "Sockets.h"
#include "SearchDlg.h"
#include "SearchResultsWnd.h"
#include "SearchParams.h"
#include "Statistics.h"
#include "Searchlist.h"
#include "Exceptions.h"
#include "Server.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "kademlia/kademlia/SearchManager.h"
#include "kademlia/kademlia/search.h"
// NEO: VOODOOs END
#include "Neo/BC\DownloadBandwidthThrottler.h"
#include "Neo/BC\UploadBandwidthThrottler.h"
#include "Neo/BC\BandwidthControl.h"
#include "clientlist.h"
#include "Neo/NeoPreferences.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --
#include "Neo/FilePreferences.h" // NEO: FCFG - [FileConfiguration] <-- Xanatos --

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define VOODOO_FILENAME _T("voodoolist.dat")

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->

#if !defined(LANCAST) || !defined(NEO_BC)
#error The UniversalPartfileInterface (VOODOO) requires the Neo Lancast Upload system from the Neo bandwidth control
#endif

IMPLEMENT_DYNAMIC(CVoodooClient, CObject)

////////////////////////////////////////////////////////////////////////////////
// CVoodoo
//

CVoodoo::CVoodoo()
{
	bListening = false;
	bStarted = false;
	uLastSearch= ::GetTickCount();
	m_port = 0;
}

CVoodoo::~CVoodoo()
{
}

bool CVoodoo::Rebind()
{
	Close();
	bListening = false;
	//KillAllSockets();

	if(NeoPrefs.IsVoodooAllowed()){
		if(!StartListening()){
			LogError(LOG_STATUSBAR, GetResString(IDS_MAIN_SOCKETERROR),NeoPrefs.GetVoodooPort());
			return false;
		}
	}
	return true;
}

bool CVoodoo::Start()
{
	if(bStarted)
		return true;

	bStarted = true;

	LoadKnownFile();

	if(NeoPrefs.IsVoodooAllowed())
		if(!StartListening()){
			LogError(LOG_STATUSBAR, GetResString(IDS_MAIN_SOCKETERROR),NeoPrefs.GetVoodooPort());
			return false;
		}

	if(NeoPrefs.IsAutoConnectVoodoo() == TRUE)
		TryToConnect();

	uLastSearch = 0;

	return true;
}

void CVoodoo::Stop()
{
	bStarted = false;

	Close();
	bListening = false;
	KillAllSockets();

	ClearKnown();
}

bool CVoodoo::StartListening()
{
	if(bListening)
		return true;

	if (!Create(NeoPrefs.GetVoodooPort(), SOCK_STREAM, FD_ACCEPT, NULL/*NeoPrefs.GetVoodooAdapter()*/, FALSE/*bReuseAddr*/))
		return false;

	if (!Listen())
		return false;

	bListening = true;
	m_port = NeoPrefs.GetVoodooPort();
	return true;
}

void CVoodoo::OnAccept(int nErrorCode)
{

	ASSERT(bListening);
	if (!nErrorCode)
	{
		CVoodooSocket* newclient;
		SOCKADDR_IN SockAddr = {0};
		int iSockAddrLen = sizeof SockAddr;
		newclient = new CVoodooSocket();
		if (!Accept(*newclient, (SOCKADDR*)&SockAddr, &iSockAddrLen)){
			newclient->Safe_Delete();
			DWORD nError = GetLastError();
			if (nError == WSAEWOULDBLOCK)
				DebugLogError(LOG_STATUSBAR, _T("%hs Accept() says WSAEWOULDBLOCK - setting counter to zero!"), __FUNCTION__);
			else
				DebugLogError(LOG_STATUSBAR, _T("%hs: Accept() says %s - setting counter to zero!"), __FUNCTION__, GetErrorMessage(nError, 1));
			return;
		}

		if (SockAddr.sin_addr.S_un.S_addr == 0) // for safety..
		{
			iSockAddrLen = sizeof SockAddr;
			newclient->GetPeerName((SOCKADDR*)&SockAddr, &iSockAddrLen);
			DebugLogWarning(_T("SockAddr.sin_addr.S_un.S_addr == 0;  GetPeerName returned %s"), ipstr(SockAddr.sin_addr.S_un.S_addr));
		}

		ASSERT( SockAddr.sin_addr.S_un.S_addr != 0 && SockAddr.sin_addr.S_un.S_addr != INADDR_NONE );

		if (!theApp.lancast->IsLanIP(SockAddr.sin_addr.S_un.S_addr)){
			if (thePrefs.GetLogFilteredIPs())
				AddDebugLogLine(false, _T("Rejecting connection attempt (IP=%s) - Not LAN Member"), ipstr(SockAddr.sin_addr.S_un.S_addr));
			newclient->Safe_Delete();
			return;
		}

		newclient->AsyncSelect(FD_WRITE | FD_READ | FD_CLOSE);

		newclient->AsyncSelect(FD_WRITE | FD_READ | FD_CLOSE);
		if (SockAddr.sin_addr.S_un.S_addr != 0)
			newclient->SetIP(SockAddr.sin_addr.S_un.S_addr);

		newclient->ConfigSocket();

		newclient->SetOnLan();
	}
}

void CVoodoo::Process()
{
	for (POSITION pos = socket_list.GetHeadPosition(); pos != NULL; )
	{
		
		CVoodooSocket* cur_sock = socket_list.GetNext(pos);
		if (cur_sock->deletethis)
		{
			if (cur_sock->m_SocketData.hSocket != INVALID_SOCKET){
				cur_sock->Close();			// calls 'closesocket'
			}
			else{
				cur_sock->Delete_Timed();	// may delete 'cur_sock'
			}
		}
		else{
			if(!cur_sock->CheckTimeOut())	// may call 'shutdown'
				cur_sock->Process();
		}
	}

	if(NeoPrefs.IsAutoConnectVoodoo() != FALSE){
		for (POSITION pos = known_list.GetHeadPosition();pos != 0;){
			CVoodooClient* cur_client = known_list.GetNext(pos);
			if(cur_client->m_uLost && (cur_client->m_uLost == (uint32)-1 || ::GetTickCount() - cur_client->m_uLost > NeoPrefs.GetVoodooReconectTimeMs() * cur_client->m_uFails))
			{
				cur_client->m_uLost = 0; // will be set again when connection fails
				ModLog(GetResString(IDS_X_VOODOO_CLIENT_RECONNECT), CVoodooSocket::GetClientDesc(cur_client->m_uAction), cur_client->clientAddress.IsEmpty() ? ipstr(cur_client->clientIP) : cur_client->clientAddress, cur_client->clientPort);
				ConnectVoodooClient(cur_client->clientIP, cur_client->clientPort, cur_client->m_uAction, cur_client->clientAddress);
				break; // only one at a time
			}
		}
	}

	// NEO: VOODOOs - [VoodooSearchForwarding]
	// cleanup search forwarding
	if(NeoPrefs.UseVoodooSearch(true)){
		SSearchMaster Master;
		DWORD MasterID;
		for (POSITION pos = m_SearchMasterMap.GetStartPosition();pos != 0;){
			m_SearchMasterMap.GetNextAssoc(pos,MasterID,Master);
			if(Master.KillTimer && ::GetTickCount() - Master.KillTimer > SEC2MS(15+5)) // the normal 15 sec kad and 5 just in case
				m_SearchMasterMap.RemoveKey(MasterID);
		}
	}
	// NEO: VOODOOs END

	// NEO: NLC - [NeoLanCast]
	// search for clients
	if(NeoPrefs.IsVoodooCastEnabled() &&  (uLastSearch == 0 ||::GetTickCount() - uLastSearch > NeoPrefs.VoodooSearchIntervalsMs())){
		uLastSearch = ::GetTickCount();

		if(NeoPrefs.SearchForSlaves() && NeoPrefs.SearchForMaster())
			theApp.lancast->SendVoodoo(VA_PARTNER);
		else if(NeoPrefs.SearchForSlaves())
			theApp.lancast->SendVoodoo(VA_SLAVE);
		else if(NeoPrefs.SearchForMaster())
			theApp.lancast->SendVoodoo(VA_MASTER);
		else if(NeoPrefs.IsVoodooCastEnabled() == TRUE)
			theApp.lancast->SendVoodoo(VA_QUERY);
	}
	// NEO: NLC END
}

void CVoodoo::AddSocket(CVoodooSocket* toadd)
{
	socket_list.AddTail(toadd);
}

void CVoodoo::RemoveSocket(CVoodooSocket* todel)
{
	POSITION pos = socket_list.Find(todel);
	if(pos)
		socket_list.RemoveAt(pos);
}

void CVoodoo::KillAllSockets()
{
	for (POSITION pos = socket_list.GetHeadPosition(); pos != 0; pos = socket_list.GetHeadPosition())
	{
		CVoodooSocket* cur_socket = socket_list.GetAt(pos);
		cur_socket->SendGoodBy(true);
		cur_socket->Disconnect(_T("stop voodoo"));
		delete cur_socket;
	}
}

bool CVoodoo::IsValidSocket(CVoodooSocket* totest)
{
	return socket_list.Find(totest) != NULL;
}

////////////////////////////////
// New Connection

void CVoodoo::ConnectVoodooClient(LPCTSTR lpszHostAddress, uint16 nHostPort, uint8 uAction)
{
	LPHOSTENT pHost;
	in_addr iaDest;
	// Lookup destination
	// Use inet_addr() to determine if we're dealing with a name or an address
	iaDest.s_addr = inet_addr(CT2CA(lpszHostAddress));
	if (iaDest.s_addr == INADDR_NONE){
		pHost = gethostbyname(CT2CA(lpszHostAddress));
	}else{
		pHost = gethostbyaddr((const char*)&iaDest, sizeof(struct in_addr), AF_INET);
		lpszHostAddress = NULL;
	}

	if (pHost == NULL) {
		ModLog(GetResString(IDS_X_VOODOO_CLIENT_NOT_FOUNT), lpszHostAddress);
		return;
	}

	DWORD nHostAddress = *(DWORD*)(*pHost->h_addr_list);
	ConnectVoodooClient(nHostAddress, nHostPort, uAction, lpszHostAddress);

}

void CVoodoo::ConnectVoodooClient(DWORD nHostAddress, uint16 nHostPort, uint8 uAction, LPCTSTR lpszHostAddress)
{
	if((uAction & VA_SLAVE) && !NeoPrefs.UseVoodooTransfer()){
		ModLog(GetResString(IDS_X_VOODOO_SLAVE_CON_UNAVALIBLY));
		return;
	}

	if((uAction & VA_MASTER) && !NeoPrefs.IsSlaveAllowed()){
		ModLog(GetResString(IDS_X_VOODOO_MASTER_CON_UNAVALIBLY));
		return;
	}

	CVoodooClient* cur_cleint = GetClientByIP(nHostAddress,nHostPort);
	CVoodooSocket* cur_socket = cur_cleint ? cur_cleint->socket : NULL;
	if(!cur_socket){
		SOCKADDR_IN sockAddr = {0};
		sockAddr.sin_family = AF_INET;
		sockAddr.sin_port = htons((u_short)nHostPort);
		sockAddr.sin_addr.S_un.S_addr = nHostAddress;

		ModLog(GetResString(IDS_X_VOODOO_CLIENT_CONENCT), CVoodooSocket::GetClientDesc(uAction)
			, lpszHostAddress ? lpszHostAddress : ipstr(nHostAddress), nHostPort);	

		CVoodooSocket* newclient;
		newclient = new CVoodooSocket();
		if (!newclient->Create()){
			newclient->Safe_Delete();
			return;
		}

		newclient->SetOnLan();
		newclient->SetAddress(lpszHostAddress);
		newclient->SetPort(nHostPort);
		newclient->SetIP(nHostAddress);
		newclient->Connect((SOCKADDR*)&sockAddr, sizeof sockAddr);

		cur_socket = newclient;
	}
	else if(I2B(uAction & VA_SLAVE) == cur_socket->IsSlave() && I2B(uAction & VA_MASTER) == cur_socket->IsMaster()){
		ModLog(GetResString(IDS_X_VOODOO_CLIENT_ALREADY_CONENCTED), 
			cur_socket->GetClientDesc(), lpszHostAddress ? lpszHostAddress : ipstr(nHostAddress), nHostPort);
		return;
	}

	cur_socket->SetAction(uAction);
	cur_socket->SendVoodooHello((uAction == VA_QUERY) ? VH_QUERY : VH_HELLO);
}

////////////////////////////
// Identyfy voodoo clients

// NEO: NLC - [NeoLanCast]
bool CVoodoo::IsVoodooClientEd2k(DWORD dwIP, uint16 uPort)
{
	for (POSITION pos = socket_list.GetHeadPosition(); pos != 0;)
	{
		CVoodooSocket* cur_socket = socket_list.GetNext(pos);
		if(cur_socket->GetIP() == dwIP && cur_socket->GetED2KPort() == uPort
		&& (cur_socket->IsSlave() || cur_socket->IsMaster()))
			return true;
	}

	return false;
}

bool CVoodoo::IsVoodooClient(DWORD dwIP, uint16 uPort)
{
	for (POSITION pos = socket_list.GetHeadPosition(); pos != 0;)
	{
		CVoodooSocket* cur_socket = socket_list.GetNext(pos);
		if(cur_socket->GetIP() == dwIP && cur_socket->GetPort() == uPort
		&& (cur_socket->IsSlave() || cur_socket->IsMaster()))
			return true;
	}

	return false;
}

void CVoodoo::AddVoodooClient(DWORD dwIP, uint16 uPort, uint8 uAction)
{
	if(IsVoodooClient(dwIP,uPort))
		return;

	if((uAction == VA_QUERY)
	|| (NeoPrefs.UseVoodooTransfer() && (uAction & VA_SLAVE) && NeoPrefs.SearchForSlaves())
	|| (NeoPrefs.IsSlaveAllowed() && (uAction & VA_MASTER) && NeoPrefs.SearchForMaster()))
		ConnectVoodooClient(dwIP, uPort, uAction);
	
}
// NEO: NLC END

///////////////////////////////
// Voodoo List

void CVoodoo::ClearKnown()
{
	while(!known_list.IsEmpty())
		delete known_list.RemoveHead();
}

CVoodooClient* CVoodoo::GetClientByAddress(LPCTSTR address, uint16 port)
{
	for (POSITION pos = known_list.GetHeadPosition();pos != 0;){
        CVoodooClient* cur_client = known_list.GetNext(pos);
        if ((port == cur_client->clientPort || port == 0) && !_tcscmp(cur_client->clientAddress, address))
			return cur_client; 
	}
	return NULL;
}

CVoodooClient* CVoodoo::GetClientByIP(uint32 nIP, uint16 nPort, bool bResolve, CVoodooSocket* Socket)
{
	if(nIP == 0)
		return NULL;

	for (POSITION pos = known_list.GetHeadPosition();pos != 0;){
        CVoodooClient* cur_client = known_list.GetNext(pos);

		if(bResolve && cur_client->clientIP == 0) // we need the ip try to resolve the host
		{
			LPHOSTENT pHost;
			in_addr iaDest;
			// Lookup destination
			// Use inet_addr() to determine if we're dealing with a name or an address
			iaDest.s_addr = inet_addr(CT2CA(cur_client->clientAddress));
			if (iaDest.s_addr == INADDR_NONE){
				pHost = gethostbyname(CT2CA(cur_client->clientAddress));
			}else{
				pHost = gethostbyaddr((const char*)&iaDest, sizeof(struct in_addr), AF_INET);
			}

			if (pHost)
				cur_client->clientIP = *(DWORD*)(*pHost->h_addr_list);
		}

		if (cur_client->clientIP == nIP 
		 && (nPort == cur_client->clientPort || !nPort)
		 && (!cur_client->socket || Socket == cur_client->socket || !Socket)
		 )
			return cur_client;
	}
	return NULL;
}

bool CVoodoo::AddKnown(CVoodooSocket* VoodooClient) // ~ ConnectionEstablished
{
	CVoodooClient* cur_client = GetClientByIP(VoodooClient->GetIP(), VoodooClient->GetPort(), false, VoodooClient);
	if(cur_client == NULL)
		cur_client = GetClientByIP(VoodooClient->GetIP(), VoodooClient->GetPort(), true, VoodooClient); // try to resolve the host ip's
	if(!cur_client == NULL && !VoodooClient->GetAddress().IsEmpty())
		cur_client = GetClientByAddress(VoodooClient->GetAddress(),VoodooClient->GetPort());
	
	if(!cur_client){
		cur_client = new CVoodooClient;
		cur_client->m_uAction = VA_NONE;

		if(!VoodooClient->GetAddress().IsEmpty())
			cur_client->clientAddress = VoodooClient->GetAddress();
		else
			cur_client->clientAddress = ipstr(VoodooClient->GetIP());
		known_list.AddTail(cur_client);
	}else if(cur_client->m_uAction == VA_BLOCK){
		return false; // Disconnect the cleint
	}else if(cur_client->socket && cur_client->socket != VoodooClient){
		ASSERT(0); // should not happen
		cur_client->socket->Disconnect(_T("Socket Conflict")); // we disconenct the old socket
	}

	cur_client->Attach(VoodooClient);

	if(VoodooClient->GetPort() != 0)
		SaveKnownToFile();

	return true;
}

void CVoodoo::CleanUpKnown(CVoodooSocket* VoodooClient)
{
	for (POSITION pos = known_list.GetHeadPosition();pos != 0;){
		POSITION toRemove = pos;
        CVoodooClient* cur_client = known_list.GetNext(pos);
		if(VoodooClient->GetIP() == cur_client->clientIP
		&& VoodooClient == cur_client->socket
		){
			if(cur_client->clientPort){
				cur_client->m_uFails++; // note that connection was lost
				if(VoodooClient->IsMaster() || VoodooClient->IsSlave())
					cur_client->m_uLost = (uint32)-1; // imminet reconnect
                else if(VoodooClient->GetAction())
					cur_client->m_uLost = ::GetTickCount(); // time reconnect
				
				cur_client->socket = NULL;
			}else{
				known_list.RemoveAt(toRemove);
				cur_client->socket = NULL;
				delete cur_client;
			}
			break;
		}
	}
}

bool CVoodoo::IsValidKnownClinet(CVoodooClient* VoodooClient)
{
	return (known_list.Find(VoodooClient) != NULL);
}

void CVoodoo::SaveKnownToFile()
{
	CStdioFile voodoolist;
	
	if (!voodoolist.Open(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + VOODOO_FILENAME, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareDenyWrite))
		return;

	CString strLine;
	for (POSITION pos = known_list.GetHeadPosition();pos != 0;){
        CVoodooClient* cur_client = known_list.GetNext(pos);
		if(cur_client->clientPort == 0)
			continue;
		strLine.Empty();
		if(!cur_client->m_sSpell.IsEmpty())
			strLine.AppendFormat(_T("!%s,"),cur_client->m_sSpell);

		strLine.AppendFormat(_T("%u,%s:%u,%u,%u"), 
												cur_client->m_uAction, 
												cur_client->clientAddress, 
												cur_client->clientPort, 
												cur_client->m_uType, 
												cur_client->m_uPerm);
		strLine.AppendFormat(_T(";%s"),cur_client->m_sName);
		voodoolist.WriteString(strLine + _T("\n"));
	}

	voodoolist.Close();
}

void CVoodoo::LoadKnownFile()
{
	CStdioFile voodoolist;
	if (!voodoolist.Open(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + VOODOO_FILENAME, CFile::modeRead | CFile::typeText | CFile::shareDenyWrite))
		return;

	UINT uAction;
	TCHAR Address[100];
	DWORD clientIP;
	CString clientAddress;
	UINT clientPort;
	UINT uType;
	UINT uPerm;
	CString sSpell;

	CString strLine;
	while (voodoolist.ReadString(strLine))
	{
		int pos = strLine.Find(':');
		if(pos == -1)
			continue;
		strLine.SetAt(pos, ' ');

		if(strLine.Left(1) == _T("!")){
			int pos = strLine.Find(',');
			if(pos == -1)
				continue;
			sSpell = strLine.Mid(1,pos-1);
			strLine = strLine.Mid(pos+1);
		}else
			sSpell.Empty();

		int test = _stscanf(strLine,_T("%u,%s %u,%u,%u,%s;"),&uAction, Address, &clientPort, &uType, &uPerm, sSpell);
		if(test < 5){
			continue;
		}

		clientIP = inet_addr(CT2CA(Address));
		if (clientIP == INADDR_NONE){
			clientIP = 0;
			clientAddress = Address;
		}else{
			clientAddress = ipstr(clientIP);
		}

		if(GetClientByAddress(clientAddress, (uint16)clientPort) != NULL)
			continue;

		CVoodooClient* new_client = new CVoodooClient;
		new_client->clientIP = clientIP;
		new_client->clientAddress = clientAddress;
		new_client->m_uAction = (uint8)uAction;
		new_client->clientPort = (uint16)clientPort;
		new_client->m_uType = (uint8)uType;
		new_client->m_uPerm = (uint8)uPerm;
		new_client->m_sSpell = sSpell;

		pos = strLine.Find(';');
		if(pos != -1)
			new_client->m_sName = strLine.Mid(pos + 1);

		known_list.AddTail(new_client);
	}

	voodoolist.Close();
}

void CVoodoo::TryToConnect()
{
	for (POSITION pos = known_list.GetHeadPosition();pos != 0;){
        CVoodooClient* cur_client = known_list.GetNext(pos);
		if(NeoPrefs.UseVoodooTransfer() && NeoPrefs.IsSlaveAllowed() && cur_client->m_uAction == VA_PARTNER)
			ConnectVoodooClient(cur_client->clientAddress,cur_client->clientPort,VA_PARTNER);
		else if(NeoPrefs.UseVoodooTransfer() && cur_client->m_uAction == VA_SLAVE)
			ConnectVoodooClient(cur_client->clientAddress,cur_client->clientPort,VA_SLAVE);
		else if(NeoPrefs.IsSlaveAllowed() && cur_client->m_uAction == VA_MASTER)
			ConnectVoodooClient(cur_client->clientAddress,cur_client->clientPort,VA_MASTER);
	}
}

/////////////////////////////
// Command Manifesting

void CVoodoo::ManifestGapList(CPartFile* pFile)
{
	for (POSITION pos = socket_list.GetHeadPosition(); pos != 0;)
	{
		CVoodooSocket* cur_socket = socket_list.GetNext(pos);
		if(cur_socket->IsSlave() && !cur_socket->IsFileErr(pFile))
			cur_socket->SendGapList(pFile);
	}
}

void CVoodoo::ManifestDownloadOrder(CPartFile* pFile)
{
	for (POSITION pos = socket_list.GetHeadPosition(); pos != 0;)
	{
		CVoodooSocket* cur_socket = socket_list.GetNext(pos);
		if(cur_socket->IsSlave() && cur_socket->IsED2K() && !cur_socket->IsFileErr(pFile))
			cur_socket->SendDownloadOrder(pFile);
	}
}

void CVoodoo::ManifestDownloadInstruction(CPartFile* pFile, uint8 uInstruction, uint32 Flag1, uint32 Flag2)
{
	for (POSITION pos = socket_list.GetHeadPosition(); pos != 0;)
	{
		CVoodooSocket* cur_socket = socket_list.GetNext(pos);
		if(cur_socket->IsSlave() && !cur_socket->IsFileErr(pFile))
			cur_socket->SendDownloadInstruction(pFile, uInstruction, Flag1, Flag2);
	}
}

void CVoodoo::ManifestShareInstruction(CKnownFile* kFile, uint8 uInstruction, uint32 Flag1, uint32 Flag2)
{
	for (POSITION pos = socket_list.GetHeadPosition(); pos != 0;)
	{
		CVoodooSocket* cur_socket = socket_list.GetNext(pos);
		if(cur_socket->IsSlave() && !cur_socket->IsFileErr(kFile))
			cur_socket->SendShareInstruction(kFile, uInstruction, Flag1, Flag2);
	}
}

// NEO: VOODOOs - [VoodooSearchForwarding]
// Search forwarding
void CVoodoo::ManifestNewSearch(SSearchParams* pParams)
{
	for (POSITION pos = socket_list.GetHeadPosition(); pos != 0;)
	{
		CVoodooSocket* cur_socket = socket_list.GetNext(pos);
		if(cur_socket->IsSlave() && cur_socket->GetVoodooSearchVersion() > 0)
			cur_socket->SendNewSearch(pParams);
	}
}

void CVoodoo::ManifestSearchCommand(DWORD dwSearchID, uint8 uCommand)
{
	for (POSITION pos = socket_list.GetHeadPosition(); pos != 0;)
	{
		CVoodooSocket* cur_socket = socket_list.GetNext(pos);
		if(cur_socket->IsSlave() && cur_socket->GetVoodooSearchVersion() > 0 && (uCommand != SC_MORE || cur_socket->m_bHaveMoreResults))
			cur_socket->SendSearchCommand(dwSearchID,uCommand);
	}
}
// NEO: VOODOOs END

void CVoodoo::ManifestThrottleBlock(CPartFile* pFile, uint64 start, uint64 end, bool bRelease, CVoodooSocket* source)
{
	for (POSITION pos = socket_list.GetHeadPosition(); pos != 0;)
	{
		CVoodooSocket* cur_socket = socket_list.GetNext(pos);
		if(source == cur_socket)
			continue;
		if(cur_socket->GetAdvDownloadSyncVersion() > (bRelease ? 1 : 0) && (cur_socket->IsSlave() 
		 || pFile->GetMasterDatas(cur_socket) != NULL) && !cur_socket->IsFileErr(pFile))
			cur_socket->SendThrottleBlock(pFile, start, end, bRelease);
	}
}

void CVoodoo::ManifestCorruptedSenderWarning(DWORD dwIP)
{
	for (POSITION pos = socket_list.GetHeadPosition(); pos != 0;)
	{
		CVoodooSocket* cur_socket = socket_list.GetNext(pos);
		if(cur_socket->IsSlave() && cur_socket->GetCorruptionHandlingVersion())
			cur_socket->SendCorruptedSenderWarning(dwIP);
	}
}

// stats
uint32 CVoodoo::GetUpDatarate()
{
	uint32 uUpDatarate = 0;
	for (POSITION pos = socket_list.GetHeadPosition(); pos != 0;)
	{
		CVoodooSocket* cur_socket = socket_list.GetNext(pos);
		if(cur_socket->IsSlave())
			uUpDatarate += cur_socket->m_Stats.uUpDatarate;
	}
	return uUpDatarate;
}

uint32 CVoodoo::GetDownDatarate()
{
	uint32 uDownDatarate = 0;
	for (POSITION pos = socket_list.GetHeadPosition(); pos != 0;)
	{
		CVoodooSocket* cur_socket = socket_list.GetNext(pos);
		if(cur_socket->IsSlave())
			uDownDatarate += cur_socket->m_Stats.uDownDatarate;
	}
	return uDownDatarate;
}

// file stats
//uint32 CVoodoo::GetUpDatarate(CKnownFile* File)
//{
//	uint32 uUpDatarate = 0;
//	for (POSITION pos = socket_list.GetHeadPosition(); pos != 0;)
//	{
//		CVoodooSocket* cur_socket = socket_list.GetNext(pos);
//		SFileVoodooStats Stats;
//		if(cur_socket->IsSlave() && cur_socket->m_FileStats.Lookup(File,Stats))
//			if(::GetTickCount() - Stats.uLastStatUpdate > STAT_REFRESH * 10) // to long delay, zero stats
//				cur_socket->m_FileStats.RemoveKey(File);
//			else
//				uUpDatarate += Stats.uUpDatarate;
//	}
//	return uUpDatarate;
//}

uint32 CVoodoo::GetDownDatarate(CKnownFile* File)
{
	uint32 uDownDatarate = 0;
	for (POSITION pos = socket_list.GetHeadPosition(); pos != 0;)
	{
		CVoodooSocket* cur_socket = socket_list.GetNext(pos);
		SFileVoodooStats Stats;
		if(cur_socket->IsSlave() && cur_socket->m_FileStats.Lookup(File,Stats)){
			if(::GetTickCount() - Stats.uLastStatUpdate > STAT_REFRESH * 10) // to long delay, zero stats
				cur_socket->m_FileStats.RemoveKey(File);
			else
				uDownDatarate += Stats.uDownDatarate;
		}
	}
	return uDownDatarate;
}

// NEO: VOODOOx - [VoodooSourceExchange]
// Source exchange
void CVoodoo::ManifestSourceListRequest(CPartFile* pFile)
{
	for (POSITION pos = socket_list.GetHeadPosition(); pos != 0;)
	{
		CVoodooSocket* cur_socket = socket_list.GetNext(pos);
		if((cur_socket->IsSlave() || pFile->GetMasterDatas(cur_socket)) && cur_socket->GetVoodooXSVersion() > 0) 
			cur_socket->RequestSourceList(pFile);
	}
}

void CVoodoo::ManifestSingleSource(CPartFile* pFile, CUpDownClient* sClient)
{
	for (POSITION pos = socket_list.GetHeadPosition(); pos != 0;)
	{
		CVoodooSocket* cur_socket = socket_list.GetNext(pos);
		if((cur_socket->IsSlave() || pFile->GetMasterDatas(cur_socket)) && cur_socket->GetVoodooXSVersion() > 0)
			cur_socket->SendSingleSource(pFile, sClient);
	}
}
// NEO: VOODOOx END

// NEO: VOODOOn - [VoodooForNeo]
void CVoodoo::ManifestNeoPreferences(EFilePrefsLevel Kind, CKnownFile* kFile, int Cat)
{
	for (POSITION pos = socket_list.GetHeadPosition(); pos != 0;)
	{
		CVoodooSocket* cur_socket = socket_list.GetNext(pos);
		if(cur_socket->IsSlave() && cur_socket->GetNeoFilePrefsVersion() > 0)
			cur_socket->SendNeoPreferences(Kind, kFile, Cat);
	}
}

void CVoodoo::ManifestDownloadCommand(CTypedPtrList<CPtrList, CPartFile*>& FileQueue, uint8 uCommand, uint32 Flag1, uint32 Flag2)
{
	for (POSITION pos = socket_list.GetHeadPosition(); pos != 0;)
	{
		CVoodooSocket* cur_socket = socket_list.GetNext(pos);
		if(cur_socket->GetNeoCommandVersion() > 0)
			cur_socket->SendDownloadCommand(FileQueue, uCommand, Flag1, Flag2);
	}
}
// NEO: VOODOOn END
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
