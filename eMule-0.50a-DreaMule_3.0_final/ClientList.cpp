//this file is part of eMule
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
#include "stdafx.h"
#include "emule.h"
#include "ClientList.h"
#include "otherfunctions.h"
#include "Kademlia/Kademlia/kademlia.h"
#include "Kademlia/Kademlia/prefs.h"
#include "Kademlia/Kademlia/search.h"
#include "Kademlia/Kademlia/searchmanager.h"
#include "Kademlia/routing/contact.h"
#include "Kademlia/net/kademliaudplistener.h"
#include "kademlia/utils/UInt128.h"
//#include "LastCommonRouteFinder.h" //Xman
#include "UploadQueue.h"
#include "DownloadQueue.h"
#include "UpDownClient.h"
#include "ClientCredits.h"
#include "ListenSocket.h"
#include "Opcodes.h"
#include "Sockets.h"
#include "emuledlg.h"
#include "TransferWnd.h"
#include "serverwnd.h"
#include "Log.h"
#include "packets.h"
#include "Statistics.h"
#include "Neo/NeoOpcodes.h" // NEO: NMP - [NeoModProt] <-- Xanatos --

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CClientList::CClientList(){
	m_dwLastBannCleanUp = 0;
	m_dwLastTrackedCleanUp = 0;
	m_dwLastClientCleanUp = 0;
	m_nBuddyStatus = Disconnected;
	m_bannedList.InitHashTable(571); //Xman changed, was 331
	m_trackedClientsList.InitHashTable(4999); //Xman changed, was 2011
	m_globDeadSourceList.Init(true);
	m_pBuddy = NULL;
#ifdef NATTUNNELING // NEO: XSB - [XSBuddy] -- Xanatos -->
	m_pXsBuddy = NULL;
#endif //NATTUNNELING // NEO: XSB END <-- Xanatos --
}

CClientList::~CClientList(){
	RemoveAllTrackedClients();
}

//Xman
// Slugfiller: modid
void CClientList::GetModStatistics(CRBMap<uint32, CRBMap<CString, uint32>* > *clientMods){
	if (!clientMods)
		return;
	clientMods->RemoveAll();

	for (POSITION pos = list.GetHeadPosition(); pos != NULL;) {		
		CUpDownClient* cur_client =	list.GetNext(pos);

		switch (cur_client->GetClientSoft()) {
		case SO_EMULE   :
		case SO_OLDEMULE:
			break;
		default:
			continue;
		}

		CRBMap<CString, uint32> *versionMods;

		if (!clientMods->Lookup(cur_client->GetVersion(), versionMods)){
			versionMods = new CRBMap<CString, uint32>;
			versionMods->RemoveAll();
			clientMods->SetAt(cur_client->GetVersion(), versionMods);
		}

		uint32 count;

		if (!versionMods->Lookup(cur_client->GetClientModVer(), count))
			count = 1;
		else
			count++;

		versionMods->SetAt(cur_client->GetClientModVer(), count);
	}
}

void CClientList::ReleaseModStatistics(CRBMap<uint32, CRBMap<CString, uint32>* > *clientMods){
	if (!clientMods)
		return;
	POSITION pos = clientMods->GetHeadPosition();
	while(pos != NULL)
	{
		uint32 version;
		CRBMap<CString, uint32> *versionMods;
		clientMods->GetNextAssoc(pos, version, versionMods);
		delete versionMods;
	}
	clientMods->RemoveAll();
}
// Slugfiller: modid
//Xman end

void CClientList::GetStatistics(uint32 &ruTotalClients, int stats[NUM_CLIENTLIST_STATS], 
								CMap<uint32, uint32, uint32, uint32>& clientVersionEDonkey, 
								CMap<uint32, uint32, uint32, uint32>& clientVersionEDonkeyHybrid, 
								CMap<uint32, uint32, uint32, uint32>& clientVersionEMule, 
								CMap<uint32, uint32, uint32, uint32>& clientVersionAMule,
								//Xman extended stats
								CMap<POSITION, POSITION, uint32, uint32>& MODs,
								uint32 &totalMODs,
								CMap<Country_Struct*, Country_Struct*, uint32, uint32>& pCountries
								//Xman end
								)
{
	ruTotalClients = list.GetCount();
	memset(stats, 0, sizeof(stats[0]) * NUM_CLIENTLIST_STATS);

	//Xman extended stats
	POSITION			pos_MOD;
	CString				strMODName;
	uint32				dwCount;
	Country_Struct*		cstruct;

	//reset values
	totalMODs = 0;
	MODs.RemoveAll();
	pCountries.RemoveAll();
	static uint32 lastmodlistclean;
	if(::GetTickCount()-lastmodlistclean> HR2MS(6))
	{
		//don´t clean it up every time -> jumping statistics
		lastmodlistclean=::GetTickCount();
		liMODsTypes.RemoveAll(); //Xman extended stats
	}
	//Xman end

	for (POSITION pos = list.GetHeadPosition(); pos != NULL; )
	{
		const CUpDownClient* cur_client = list.GetNext(pos);

		if (cur_client->HasLowID())
			stats[14]++;
		
		switch (cur_client->GetClientSoft())
		{
			case SO_EMULE:
			case SO_OLDEMULE:
				stats[2]++;
				clientVersionEMule[cur_client->GetVersion()]++;
				//Xman extended stats
				strMODName = cur_client->GetClientModVer();

				if (!strMODName.IsEmpty())
				{
					//extract modname without version
					int length=strMODName.GetLength();
					int i;
					for(i=0;i<length;i++)
					{
						if(strMODName.GetAt(i)>=_T('0') && strMODName.GetAt(i)<=_T('9'))
				break;
					}
					if(i<length && i>0)
						strMODName=strMODName.Left(i);
					if(strMODName.Right(1)==_T('v') && strMODName.GetLength()>2)
					{
						strMODName = strMODName.Left(strMODName.GetLength()-1);
					}
					strMODName.Trim();

					totalMODs++;
					pos_MOD = liMODsTypes.Find(strMODName);
					if (!pos_MOD)
					{
						pos_MOD = liMODsTypes.AddTail(strMODName);
						MODs.SetAt(pos_MOD, 1);
					}
					else
					{
						dwCount = 0;
						MODs.Lookup(pos_MOD, dwCount);
						MODs.SetAt(pos_MOD, ++dwCount);
					}
				}
				//Xman end
				break;

			case SO_EDONKEYHYBRID : 
				stats[4]++;
				clientVersionEDonkeyHybrid[cur_client->GetVersion()]++;
				break;
			
			case SO_AMULE:
				stats[10]++;
				clientVersionAMule[cur_client->GetVersion()]++;
				break;

			case SO_EDONKEY:
				stats[1]++;
				clientVersionEDonkey[cur_client->GetVersion()]++;
				break;

			case SO_MLDONKEY:
				stats[3]++;
				break;
			
			case SO_SHAREAZA:
				stats[11]++;
				break;

			// all remaining 'eMule Compatible' clients
//>>> WiZaRd::ECR [Spike2]
			case SO_HYDRANODE:
			case SO_EMULEPLUS:
			case SO_TRUSTYFILES:
//<<< WiZaRd::ECR [Spike2]
			case SO_CDONKEY:
			case SO_XMULE:
			case SO_LPHANT:
				stats[5]++;
				break;

			default:
				stats[0]++;
				break;
		}

		//Xman extended stats
		//count the countries
		CMap<Country_Struct*, Country_Struct*, uint32, uint32>::CPair *pPair;

		cstruct = cur_client->m_structUserCountry;
		pPair = pCountries.PLookup(cstruct);
		if (pPair != NULL)
			pPair->value++;
		else
			pCountries.SetAt(cstruct, 1);
		//Xman end

		if (cur_client->Credits() != NULL)
		{
			switch (cur_client->Credits()->GetCurrentIdentState(cur_client->GetIP()))
			{
				case IS_IDENTIFIED:
					stats[12]++;
					break;
				case IS_IDFAILED:
				case IS_IDNEEDED:
				case IS_IDBADGUY:
					stats[13]++;
					break;
			}
		}

		if (cur_client->GetDownloadState()==DS_ERROR || cur_client->GetUploadState()==US_ERROR)
			stats[6]++; // Error

		switch (cur_client->GetUserPort())
		{
			case 4662:
				stats[8]++; // Default Port
				break;
			default:
				stats[9]++; // Other Port
		}

		// Network client stats
		if (cur_client->GetServerIP() && cur_client->GetServerPort())
		{
			stats[15]++;		// eD2K
			if(cur_client->GetKadPort())
			{
				stats[17]++;	// eD2K/Kad
				stats[16]++;	// Kad
			}
		}
		else if (cur_client->GetKadPort())
			stats[16]++;		// Kad
		else
			stats[18]++;		// Unknown
	}
}

void CClientList::AddClient(CUpDownClient* toadd, bool bSkipDupTest)
{
	// skipping the check for duplicate list entries is only to be done for optimization purposes, if the calling
	// function has ensured that this client instance is not already within the list -> there are never duplicate
	// client instances in this list.
	if (!bSkipDupTest){
		if(list.Find(toadd))
			return;
	}
	theApp.emuledlg->transferwnd->clientlistctrl.AddClient(toadd);
	list.AddTail(toadd);
}

/* Xman
// ZZ:UploadSpeedSense -->
bool CClientList::GiveClientsForTraceRoute() {
    // this is a host that lastCommonRouteFinder can use to traceroute
    return theApp.lastCommonRouteFinder->AddHostsToCheck(list);
}
// ZZ:UploadSpeedSense <--
*/

void CClientList::RemoveClient(CUpDownClient* toremove, LPCTSTR pszReason){
	POSITION pos = list.Find(toremove);
	if (pos){
		theApp.uploadqueue->RemoveFromUploadQueue(toremove, CString(_T("CClientList::RemoveClient: ")) + pszReason);
		theApp.uploadqueue->RemoveFromWaitingQueue(toremove);
		theApp.downloadqueue->RemoveSource(toremove);
		theApp.emuledlg->transferwnd->clientlistctrl.RemoveClient(toremove);
		RemoveFromKadList(toremove);
#ifdef NATTUNNELING // NEO: XSB - [XSBuddy] -- Xanatos -->
		if(toremove == m_pXsBuddy){
			m_pXsBuddy = NULL;
			toremove->SetXsBuddyStatus(XB_NONE);
		}
#endif //NATTUNNELING // NEO: XSB END <-- Xanatos --
		list.RemoveAt(pos);
	}
}

void CClientList::DeleteAll(){
	theApp.uploadqueue->DeleteAll();
	theApp.downloadqueue->DeleteAll();
	POSITION pos1, pos2;
	for (pos1 = list.GetHeadPosition();( pos2 = pos1 ) != NULL;){
		list.GetNext(pos1);
		CUpDownClient* cur_client =	list.GetAt(pos2);
		list.RemoveAt(pos2);
		delete cur_client; // recursiv: this will call RemoveClient
	}
	liMODsTypes.RemoveAll(); //Xman extended stats
}

bool CClientList::AttachToAlreadyKnown(CUpDownClient** client, CClientReqSocket* sender){
	POSITION pos1, pos2;
	CUpDownClient* tocheck = (*client);
	CUpDownClient* found_client = NULL;
	CUpDownClient* found_client2 = NULL;
	for (pos1 = list.GetHeadPosition(); (pos2 = pos1) != NULL; ){
		list.GetNext(pos1);
		CUpDownClient* cur_client =	list.GetAt(pos2);
		if (tocheck->Compare(cur_client,false)){ //matching userhash
			found_client2 = cur_client;
		}
		if (tocheck->Compare(cur_client,true)){	 //matching IP
			found_client = cur_client;
			break;
		}
	}
	if (found_client == NULL)
		found_client = found_client2;

	if (found_client != NULL){
		if (tocheck == found_client){
			//we found the same client instance (client may have sent more than one OP_HELLO). do not delete that client!
			return true;
		}
		if (sender){
			if (found_client->socket){
				if (found_client->socket->IsConnected() 
					&& (found_client->GetConnectIP() != tocheck->GetConnectIP() //Xman use ConnectIP instead of GetIP()
					|| found_client->GetUserPort() != tocheck->GetUserPort() ) )
				{
					// if found_client is connected and has the IS_IDENTIFIED, it's safe to say that the other one is a bad guy
					if (found_client->Credits() && found_client->Credits()->GetCurrentIdentState(found_client->GetIP()) == IS_IDENTIFIED){
						if (thePrefs.GetLogBannedClients())
							AddDebugLogLine(false, _T("Clients: %s (%s), Banreason: Userhash invalid"), tocheck->GetUserName(), ipstr(tocheck->GetConnectIP()));
						tocheck->Ban(_T("Userhash invalid")); //Xman
						return false;
					}
	
					//IDS_CLIENTCOL Warning: Found matching client, to a currently connected client: %s (%s) and %s (%s)
					if (thePrefs.GetLogBannedClients())
						AddDebugLogLine(true,GetResString(IDS_CLIENTCOL), tocheck->GetUserName(), ipstr(tocheck->GetConnectIP()), found_client->GetUserName(), ipstr(found_client->GetConnectIP()));
					return false;
				}
				found_client->socket->client = 0;
				found_client->socket->Safe_Delete();
			}
			found_client->socket = sender;
			tocheck->socket = 0;
		}
		*client = 0;
		delete tocheck;
		*client = found_client;
		return true;
	}
	return false;
}

CUpDownClient* CClientList::FindClientByIP(uint32 clientip, UINT port) const
{
	for (POSITION pos = list.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* cur_client = list.GetNext(pos);
		if (cur_client->GetIP() == clientip && cur_client->GetUserPort() == port)
			return cur_client;
	}
	return 0;
}

CUpDownClient* CClientList::FindClientByUserHash(const uchar* clienthash) const
{
	for (POSITION pos = list.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* cur_client = list.GetNext(pos);
		if (!md4cmp(cur_client->GetUserHash() ,clienthash))
			return cur_client;
	}
	return 0;
}

CUpDownClient* CClientList::FindClientByIP(uint32 clientip) const
{
	for (POSITION pos = list.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* cur_client = list.GetNext(pos);
		if (cur_client->GetIP() == clientip)
			return cur_client;
	}
	return 0;
}

CUpDownClient* CClientList::FindClientByIP_UDP(uint32 clientip, UINT nUDPport) const
{
	for (POSITION pos = list.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* cur_client = list.GetNext(pos);
		if (cur_client->GetIP() == clientip && cur_client->GetUDPPort() == nUDPport)
			return cur_client;
	}
	return 0;
}

CUpDownClient* CClientList::FindClientByUserID_KadPort(uint32 clientID, uint16 kadPort) const
{
	for (POSITION pos = list.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* cur_client = list.GetNext(pos);
		if (cur_client->GetUserIDHybrid() == clientID && cur_client->GetKadPort() == kadPort)
			return cur_client;
	}
	return 0;
}

CUpDownClient* CClientList::FindClientByIP_KadPort(uint32 ip, uint16 port) const
{
	for (POSITION pos = list.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* cur_client = list.GetNext(pos);
		if (cur_client->GetIP() == ip && cur_client->GetKadPort() == port)
			return cur_client;
	}
	return 0;
}

CUpDownClient* CClientList::FindClientByServerID(uint32 uServerIP, uint32 uED2KUserID) const
{
	uint32 uHybridUserID = ntohl(uED2KUserID);
	for (POSITION pos = list.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* cur_client =	list.GetNext(pos);
		if (cur_client->GetServerIP() == uServerIP && cur_client->GetUserIDHybrid() == uHybridUserID)
			return cur_client;
	}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// Banned clients

void CClientList::AddBannedClient(uint32 dwIP){
	m_bannedList.SetAt(dwIP, ::GetTickCount());
}

bool CClientList::IsBannedClient(uint32 dwIP) const
{
	uint32 dwBantime;
	if (m_bannedList.Lookup(dwIP, dwBantime)){
		if (dwBantime + CLIENTBANTIME > ::GetTickCount())
			return true;
	}
	return false; 
}

void CClientList::RemoveBannedClient(uint32 dwIP){
	m_bannedList.RemoveKey(dwIP);
}

void CClientList::RemoveAllBannedClients(){
	m_bannedList.RemoveAll();
}


///////////////////////////////////////////////////////////////////////////////
// Tracked clients

//Xman Extened credit- table-arragement
/*
void CClientList::AddTrackClient(CUpDownClient* toadd){
	CDeletedClient* pResult = 0;
	if (m_trackedClientsList.Lookup(toadd->GetIP(), pResult)){
		pResult->m_dwInserted = ::GetTickCount();
		for (int i = 0; i != pResult->m_ItemsList.GetCount(); i++){
			if (pResult->m_ItemsList[i].nPort == toadd->GetUserPort()){
				// already tracked, update
				pResult->m_ItemsList[i].pHash = toadd->Credits();
				return;
			}
		}
		PORTANDHASH porthash = { toadd->GetUserPort(), toadd->Credits()};
		pResult->m_ItemsList.Add(porthash);
	}
	else{
		m_trackedClientsList.SetAt(toadd->GetIP(), new CDeletedClient(toadd));
	}
}
*/
//Xman end

//Xman Extened credit- table-arragement
//make the Tracked-client-list independent 
void CClientList::AddTrackClient(CUpDownClient* toadd){
	CDeletedClient* pResult = 0;
	if (m_trackedClientsList.Lookup(toadd->GetIP(), pResult)){
		pResult->m_dwInserted = ::GetTickCount();
		for (int i = 0; i != pResult->m_ItemsList.GetCount(); i++){
			if (pResult->m_ItemsList[i].nPort == toadd->GetUserPort()){
				// already tracked, update
				//Xman don't keep a track of the credit-pointer, but of the hash
				md4cpy(pResult->m_ItemsList[i].pHash, toadd->GetUserHash());
				return;
			}
		}
		//Xman new tracked port & hash
		PORTANDHASH porthash;
		porthash.nPort=toadd->GetUserPort();
		md4cpy(porthash.pHash,toadd->GetUserHash());
		pResult->m_ItemsList.Add(porthash);
	}
	else{
		m_trackedClientsList.SetAt(toadd->GetIP(), new CDeletedClient(toadd));
	}
}
//Xman end

// true = everything ok, hash didn't changed
// false = hash changed
bool CClientList::ComparePriorUserhash(uint32 dwIP, uint16 nPort, void* pNewHash){
	CDeletedClient* pResult = 0;
	if (m_trackedClientsList.Lookup(dwIP, pResult)){
		for (int i = 0; i != pResult->m_ItemsList.GetCount(); i++){
			if (pResult->m_ItemsList[i].nPort == nPort){
				//Xman Extened credit- table-arragement
				//make the Tracked-client-list independent 
				//if (pResult->m_ItemsList[i].pHash != pNewHash)
				if (md4cmp(pResult->m_ItemsList[i].pHash , pNewHash)!=0)
				//Xman end
					return false;
				else
					break;
			}
		}
	}
	return true;
}

UINT CClientList::GetClientsFromIP(uint32 dwIP) const
{
	CDeletedClient* pResult;
	if (m_trackedClientsList.Lookup(dwIP, pResult))
		return pResult->m_ItemsList.GetCount();
	return 0;
}

void CClientList::TrackBadRequest(const CUpDownClient* upcClient, int nIncreaseCounter){
	CDeletedClient* pResult = NULL;
	if (upcClient->GetIP() == 0){
		ASSERT( false );
		return;
	}
	if (m_trackedClientsList.Lookup(upcClient->GetIP(), pResult)){
		pResult->m_dwInserted = ::GetTickCount();
		pResult->m_cBadRequest += nIncreaseCounter;
	}
	else{
		CDeletedClient* ccToAdd = new CDeletedClient(upcClient);
		ccToAdd->m_cBadRequest = nIncreaseCounter;
		m_trackedClientsList.SetAt(upcClient->GetIP(), ccToAdd);
	}
}

uint32 CClientList::GetBadRequests(const CUpDownClient* upcClient) const{
	CDeletedClient* pResult = NULL;
	if (upcClient->GetIP() == 0){
		ASSERT( false );
		return 0;
	}
	if (m_trackedClientsList.Lookup(upcClient->GetIP(), pResult)){
		return pResult->m_cBadRequest;
	}
	else
		return 0;
}

void CClientList::RemoveAllTrackedClients(){
	POSITION pos = m_trackedClientsList.GetStartPosition();
	uint32 nKey;
	CDeletedClient* pResult;
	while (pos != NULL){
		m_trackedClientsList.GetNextAssoc(pos, nKey, pResult);
		m_trackedClientsList.RemoveKey(nKey);
		delete pResult;
	}
}

void CClientList::Process()
{
	///////////////////////////////////////////////////////////////////////////
	// Cleanup banned client list
	//
	const uint32 cur_tick = ::GetTickCount();
	if (m_dwLastBannCleanUp + BAN_CLEANUP_TIME < cur_tick)
	{
		m_dwLastBannCleanUp = cur_tick;
		
		POSITION pos = m_bannedList.GetStartPosition();
		uint32 nKey;
		uint32 dwBantime;
		while (pos != NULL)
		{
			m_bannedList.GetNextAssoc( pos, nKey, dwBantime );
			if (dwBantime + CLIENTBANTIME < cur_tick )
				RemoveBannedClient(nKey);
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// Cleanup tracked client list
	//
	if (m_dwLastTrackedCleanUp + TRACKED_CLEANUP_TIME < cur_tick)
	{
		m_dwLastTrackedCleanUp = cur_tick;
		if (thePrefs.GetLogBannedClients()) 
			AddDebugLogLine(false, _T("Cleaning up TrackedClientList, %i clients on List..."), m_trackedClientsList.GetCount());
		POSITION pos = m_trackedClientsList.GetStartPosition();
		uint32 nKey;
		CDeletedClient* pResult;
		while (pos != NULL)
		{
			m_trackedClientsList.GetNextAssoc( pos, nKey, pResult );
			if (pResult->m_dwInserted + KEEPTRACK_TIME < cur_tick ){
				m_trackedClientsList.RemoveKey(nKey);
				delete pResult;
			}
		}
		if (thePrefs.GetLogBannedClients())
			AddDebugLogLine(false, _T("...done, %i clients left on list"), m_trackedClientsList.GetCount());
	}

	///////////////////////////////////////////////////////////////////////////
	// Process Kad client list
	//
	//We need to try to connect to the clients in m_KadList
	//If connected, remove them from the list and send a message back to Kad so we can send a ACK.
	//If we don't connect, we need to remove the client..
	//The sockets timeout should delete this object.
	POSITION pos1, pos2;

	// buddy is just a flag that is used to make sure we are still connected or connecting to a buddy.
	buddyState buddy = Disconnected;

	for (pos1 = m_KadList.GetHeadPosition(); (pos2 = pos1) != NULL; )
	{
		m_KadList.GetNext(pos1);
		CUpDownClient* cur_client =	m_KadList.GetAt(pos2);
		if( !Kademlia::CKademlia::IsRunning() )
		{
			//Clear out this list if we stop running Kad.
			//Setting the Kad state to KS_NONE causes it to be removed in the switch below.
			cur_client->SetKadState(KS_NONE);
		}
		switch(cur_client->GetKadState())
		{
			case KS_QUEUED_FWCHECK:
				//Another client asked us to try to connect to them to check their firewalled status.
				cur_client->TryToConnect(true);
				break;

			case KS_CONNECTING_FWCHECK:
				//Ignore this state as we are just waiting for results.
				break;

			case KS_CONNECTED_FWCHECK:
				//We successfully connected to the client.
				//We now send a ack to let them know.
				if (thePrefs.GetDebugClientKadUDPLevel() > 0)
					DebugSend("KADEMLIA_FIREWALLED_ACK_RES", cur_client->GetIP(), cur_client->GetKadPort());
				Kademlia::CKademlia::GetUDPListener()->SendNullPacket(KADEMLIA_FIREWALLED_ACK_RES, ntohl(cur_client->GetIP()), cur_client->GetKadPort());
				//We are done with this client. Set Kad status to KS_NONE and it will be removed in the next cycle.
				cur_client->SetKadState(KS_NONE);
				break;

			case KS_INCOMING_BUDDY:
				//A firewalled client wants us to be his buddy.
				//If we already have a buddy, we set Kad state to KS_NONE and it's removed in the next cycle.
				//If not, this client will change to KS_CONNECTED_BUDDY when it connects.
				if( m_nBuddyStatus == Connected )
					cur_client->SetKadState(KS_NONE);
				break;

			case KS_QUEUED_BUDDY:
				//We are firewalled and want to request this client to be a buddy.
				//But first we check to make sure we are not already trying another client.
				//If we are not already trying. We try to connect to this client.
				//If we are already connected to a buddy, we set this client to KS_NONE and it's removed next cycle.
				//If we are trying to connect to a buddy, we just ignore as the one we are trying may fail and we can then try this one.
				if( m_nBuddyStatus == Disconnected )
				{
					buddy = Connecting;
					m_nBuddyStatus = Connecting;
					cur_client->SetKadState(KS_CONNECTING_BUDDY);
					cur_client->TryToConnect(true);
					theApp.emuledlg->serverwnd->UpdateMyInfo();
				}
				else if( m_nBuddyStatus == Connected )
					cur_client->SetKadState(KS_NONE);
				break;

			case KS_CONNECTING_BUDDY:
				//We are trying to connect to this client.
				//Although it should NOT happen, we make sure we are not already connected to a buddy.
				//If we are we set to KS_NONE and it's removed next cycle.
				//But if we are not already connected, make sure we set the flag to connecting so we know 
				//things are working correctly.
				if( m_nBuddyStatus == Connected )
					cur_client->SetKadState(KS_NONE);
				else
				{
					ASSERT( m_nBuddyStatus == Connecting );
					buddy = Connecting;
				}
				break;

			case KS_CONNECTED_BUDDY:
				//A potential connected buddy client wanting to me in the Kad network
				//We set our flag to connected to make sure things are still working correctly.
				buddy = Connected;
				
				//If m_nBuddyStatus is not connected already, we set this client as our buddy!
				if( m_nBuddyStatus != Connected )
				{
					m_pBuddy = cur_client;
					m_nBuddyStatus = Connected;
					theApp.emuledlg->serverwnd->UpdateMyInfo();
				}
				if( m_pBuddy == cur_client && theApp.IsFirewalled() && cur_client->SendBuddyPingPong() )
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugSend("OP__BuddyPing", cur_client);
					Packet* buddyPing = new Packet(OP_BUDDYPING, 0, OP_EMULEPROT);
					theStats.AddUpDataOverheadOther(buddyPing->size);
					cur_client->SafeSendPacket(buddyPing);
					cur_client->SetLastBuddyPingPongTime();
				}
				break;

			default:
				RemoveFromKadList(cur_client);
		}
	}
	
	//We either never had a buddy, or lost our buddy..
	if( buddy == Disconnected )
	{
		if( m_nBuddyStatus != Disconnected || m_pBuddy )
		{
			if( Kademlia::CKademlia::IsRunning() && theApp.IsFirewalled() )
			{
				//We are a lowID client and we just lost our buddy.
				//Go ahead and instantly try to find a new buddy.
				Kademlia::CKademlia::GetPrefs()->SetFindBuddy();
			}
			m_pBuddy = NULL;
			m_nBuddyStatus = Disconnected;
			theApp.emuledlg->serverwnd->UpdateMyInfo();
		}
	}

	if ( Kademlia::CKademlia::IsConnected() )
	{
		if( Kademlia::CKademlia::IsFirewalled() )
		{
			//TODO: Kad buddies won'T work with RequireCrypt, so it is disabled for now but should (and will)
			//be fixed in later version
			if( m_nBuddyStatus == Disconnected && Kademlia::CKademlia::GetPrefs()->GetFindBuddy() && !thePrefs.IsClientCryptLayerRequired())
			{
				//We are a firewalled client with no buddy. We have also waited a set time 
				//to try to avoid a false firewalled status.. So lets look for a buddy..
				if( !Kademlia::CSearchManager::PrepareLookup(Kademlia::CSearch::FINDBUDDY, true, Kademlia::CUInt128(true).Xor(Kademlia::CKademlia::GetPrefs()->GetKadID())) )
				{
					//This search ID was already going. Most likely reason is that
					//we found and lost our buddy very quickly and the last search hadn't
					//had time to be removed yet. Go ahead and set this to happen again
					//next time around.
					Kademlia::CKademlia::GetPrefs()->SetFindBuddy();
				}
			}
		}
		else
		{
			if( m_pBuddy )
			{
				//Lets make sure that if we have a buddy, they are firewalled!
				//If they are also not firewalled, then someone must have fixed their firewall or stopped saturating their line.. 
				//We just set the state of this buddy to KS_NONE and things will be cleared up with the next cycle.
				if( !m_pBuddy->HasLowID() )
					m_pBuddy->SetKadState(KS_NONE);
			}
		}
	}
	else
	{
		if( m_pBuddy )
		{
			//We are not connected anymore. Just set this buddy to KS_NONE and things will be cleared out on next cycle.
			m_pBuddy->SetKadState(KS_NONE);
		}
	}

#ifdef NATTUNNELING // NEO: XSB - [XSBuddy] -- Xanatos -->
	if(theApp.IsFirewalled() && GetXsBuddyStatus() == XB_NONE && theApp.IsConnected())
	{
		m_pXsBuddy = NULL;
		uint16 skipped = 0;
		for (POSITION pos = list.GetHeadPosition(); pos != NULL; )
		{
			CUpDownClient* cur_client = list.GetNext(pos);
			if(cur_client->HasLowID())
				continue;

			if(cur_client->SupportsNatTraversal())
				continue;

			if(cur_client->GetXsBuddyStatus() == XB_NONE)
			{
				m_pXsBuddy = cur_client;
				cur_client->SetXsBuddyStatus(XB_CONNECTING);
				cur_client->TryToConnect(true);
				theApp.emuledlg->serverwnd->UpdateMyInfo();
				break;
			}
			else
				skipped++;
		}

		// When no valid cleints was found but some have status XB_DENIDED 
		// we reset them and try again to become a buddy of one of those cleints
		if(m_pXsBuddy == NULL && skipped)
			for (POSITION pos = list.GetHeadPosition(); pos != NULL; list.GetNext(pos)->SetXsBuddyStatus(XB_NONE));
	}
	else if(GetXsBuddyStatus() == XB_HIGH_BUDDY)
	{
		// we reuse the PingPong mecha from kad for timing,
		// but use own mod packets to keep things clean
		// Note: we use only one packet for Ping, the LowID send the ping and the HighID answers it
		if( m_pXsBuddy->SendBuddyPingPong() )
		{
			if (thePrefs.GetDebugClientTCPLevel() > 0)
				DebugSend("OP_XS_BuddyPing", m_pXsBuddy);
			Packet* buddyPing = new Packet(OP_XS_BUDDYPING, 0, OP_MODPROT);
			theStats.AddUpDataOverheadOther(buddyPing->size);
			m_pXsBuddy->SetLastBuddyPingPongTime();
			m_pXsBuddy->SafeSendPacket(buddyPing);	
		}
	}
#endif //NATTUNNELING // NEO: XSB END <-- Xanatos --

	///////////////////////////////////////////////////////////////////////////
	// Cleanup client list
	//
	//CleanUpClientList(); //Xman moved to uploadqueue
}

#ifdef _DEBUG
void CClientList::Debug_SocketDeleted(CClientReqSocket* deleted) const
{
	for (POSITION pos = list.GetHeadPosition(); pos != NULL;){
		CUpDownClient* cur_client =	list.GetNext(pos);
		if (!AfxIsValidAddress(cur_client, sizeof(CUpDownClient))) {
			AfxDebugBreak();
		}
		if (thePrefs.m_iDbgHeap >= 2)
			ASSERT_VALID(cur_client);
		if (cur_client->socket == deleted){
			AfxDebugBreak();
		}
	}
}
#endif

bool CClientList::IsValidClient(CUpDownClient* tocheck) const
{
	if (thePrefs.m_iDbgHeap >= 2)
		ASSERT_VALID(tocheck);
	return list.Find(tocheck)!=NULL;
}


///////////////////////////////////////////////////////////////////////////////
// Kad client list

void CClientList::RequestTCP(Kademlia::CContact* contact)
{
	uint32 nContactIP = ntohl(contact->GetIPAddress());
	// don't connect ourself
	if (theApp.serverconnect->GetLocalIP() == nContactIP && thePrefs.GetPort() == contact->GetTCPPort())
		return;

	CUpDownClient* pNewClient = FindClientByIP(nContactIP, contact->GetTCPPort());

	const bool bNewClient = pNewClient == NULL; //Xman Code Improvement don't search new generated clients in lists (seen by Wizard)

	if (!pNewClient)
		pNewClient = new CUpDownClient(0, contact->GetTCPPort(), contact->GetIPAddress(), 0, 0, false );

	//Add client to the lists to be processed.
	pNewClient->SetKadPort(contact->GetUDPPort());
	pNewClient->SetKadState(KS_QUEUED_FWCHECK);
	//Xman Code Improvement don't search new generated clients in lists (seen by Wizard)
	/*
	m_KadList.AddTail(pNewClient);
	//This method checks if this is a dup already.
	AddClient(pNewClient);
	*/
	//Xman no need to check for dupe in clientlist, either we found it or it's new
	//if not new do a dupe check in kad-list and don't add to clientlist
	if(bNewClient)
	{
		m_KadList.AddTail(pNewClient);
		AddClient(pNewClient, true); 
	}
	else
		AddToKadList(pNewClient); 
	//Xman end
}

void CClientList::RequestBuddy(Kademlia::CContact* contact)
{
	uint32 nContactIP = ntohl(contact->GetIPAddress());
	// don't connect ourself
	if (theApp.serverconnect->GetLocalIP() == nContactIP && thePrefs.GetPort() == contact->GetTCPPort())
		return;
	CUpDownClient* pNewClient = FindClientByIP(nContactIP, contact->GetTCPPort());
	
	const bool bNewClient = pNewClient == NULL; //Xman Code Improvement don't search new generated clients in lists (seen by Wizard)

	if (!pNewClient)
		pNewClient = new CUpDownClient(0, contact->GetTCPPort(), contact->GetIPAddress(), 0, 0, false );

	//Add client to the lists to be processed.
	pNewClient->SetKadPort(contact->GetUDPPort());
	pNewClient->SetKadState(KS_QUEUED_BUDDY);
	byte ID[16];
	contact->GetClientID().ToByteArray(ID);
	pNewClient->SetUserHash(ID);
	//Xman Code Improvement don't search new generated clients in lists (seen by Wizard)
	/*
	AddToKadList(pNewClient);
	//This method checks if this is a dup already.
	AddClient(pNewClient);
	*/
	//Xman no need to check for dupe in clientlist, either we found it or it's new
	//if not new do a dupe check in kad-list and don't add to clientlist
	if(bNewClient)
	{
		m_KadList.AddTail(pNewClient);
		AddClient(pNewClient, true); 
	}
	else
		AddToKadList(pNewClient); 
	//Xman end
}

void CClientList::IncomingBuddy(Kademlia::CContact* contact, Kademlia::CUInt128* buddyID )
{
	uint32 nContactIP = ntohl(contact->GetIPAddress());
	//If eMule already knows this client, abort this.. It could cause conflicts.
	//Although the odds of this happening is very small, it could still happen.
	if (FindClientByIP(nContactIP, contact->GetTCPPort()))
	{
		return;
	}

	// don't connect ourself
	if (theApp.serverconnect->GetLocalIP() == nContactIP && thePrefs.GetPort() == contact->GetTCPPort())
		return;

	//Add client to the lists to be processed.
	CUpDownClient* pNewClient = new CUpDownClient(0, contact->GetTCPPort(), contact->GetIPAddress(), 0, 0, false );
	pNewClient->SetKadPort(contact->GetUDPPort());
	pNewClient->SetKadState(KS_INCOMING_BUDDY);
	byte ID[16];
	contact->GetClientID().ToByteArray(ID);
	pNewClient->SetUserHash(ID);
	buddyID->ToByteArray(ID);
	pNewClient->SetBuddyID(ID);
	//Xman Code Improvement don't search new generated clients in lists (seen by Wizard)
	//Xman it's a new client -> no dupe check
	//AddToKadList(pNewClient);
	//AddClient(pNewClient);
	m_KadList.AddTail(pNewClient);
	AddClient(pNewClient, true); 
	//Xman end
}

void CClientList::RemoveFromKadList(CUpDownClient* torem){
	POSITION pos = m_KadList.Find(torem);
	if(pos)
	{
		if(torem == m_pBuddy)
		{
			m_pBuddy = NULL;
			theApp.emuledlg->serverwnd->UpdateMyInfo();
		}
		m_KadList.RemoveAt(pos);
	}
}

void CClientList::AddToKadList(CUpDownClient* toadd){
	if(!toadd)
		return;
	POSITION pos = m_KadList.Find(toadd);
	if(pos)
	{
		return;
	}
	m_KadList.AddTail(toadd);
}

// Maella -Extended clean-up II- //rework by Xman
//Note: this feature is important for Xtreme Downloadmanager
void CClientList::CleanUpClientList(){
	const uint32 cur_tick = ::GetTickCount();
	if (m_dwLastClientCleanUp + CLIENTLIST_CLEANUP_TIME < cur_tick ){
		m_dwLastClientCleanUp = cur_tick;
		uint32 cDeleted = 0;
		for (POSITION pos = list.GetHeadPosition(); pos != NULL;){
			CUpDownClient* pCurClient =	list.GetNext(pos);
			if ((pCurClient->GetUploadState() == US_NONE /*|| pCurClient->GetUploadState() == US_BANNED && !pCurClient->IsBanned()*/) //Xman Code Improvement: how should this happen ?
				&& pCurClient->GetDownloadState() == DS_NONE
				&& pCurClient->GetChatState() == MS_NONE
				&& pCurClient->GetKadState() == KS_NONE
				&& pCurClient->socket == NULL)
			{
				const DWORD delta = GetTickCount() - pCurClient->m_lastCleanUpCheck;
				if(delta > 7200000){ // 2 hour
					if(!pCurClient->m_OtherNoNeeded_list.IsEmpty() || !pCurClient->m_OtherRequests_list.IsEmpty())
					{
						AddDebugLogLine(false, _T("Extended clean-up reports an error in CleanUpProcess with client %s"),pCurClient->DbgGetClientInfo());
						pCurClient->m_lastCleanUpCheck = GetTickCount();
					}
					else
					{
						cDeleted++;
						delete pCurClient;
					}
				}
			}
			else{
				pCurClient->m_lastCleanUpCheck = GetTickCount();
			}
		}
		AddDebugLogLine(false,_T("Cleaned ClientList, removed %i not used known clients"), cDeleted);
	}
}
void CClientList::CleanUp(CPartFile* pDeletedFile){
	for(POSITION pos = list.GetHeadPosition(); pos != NULL;){		
		CUpDownClient* cur_client =	list.GetNext(pos);
		cur_client->CleanUp(pDeletedFile);
	}	
}
// Maella end

#ifdef PRINT_STATISTIC
void CClientList::PrintStatistic()
{
	AddLogLine(false,_T("Clients in Clientlist: %u"), list.GetSize());
	AddLogLine(false, _T("Clients in Bannedlist: %u"), m_bannedList.GetSize());
	AddLogLine(false, _T("Tracked Clients: %u"), m_trackedClientsList.GetSize());
	AddLogLine(false, _T("Clients in Kadlist: %u"), m_KadList.GetSize());

	AddLogLine(false, _T("sum of listelements of all known clients:"));
	uint32 PartStatusMapCount=0;
	uint32 upHistoryCount=0;
	uint32 downHistoryCount=0;
	uint32 DontSwapListCount=0;
	uint32 BlockRequestedCount=0;
	uint32 DoneBlocksCount=0;
	uint32 RequestedFilesCount=0;
	uint32 PendingBlockCount=0;
	uint32 DownloadBlockCount=0;
	uint32 NoNeededListCount=0;
	uint32 OtherRequestListCount=0;
	for(POSITION pos = list.GetHeadPosition(); pos != NULL;){		
		CUpDownClient* cur_client =	list.GetNext(pos);
		PartStatusMapCount += cur_client->GetPartStatusMapCount();
		upHistoryCount += cur_client->GetupHistoryCount();
		downHistoryCount += cur_client->GetdownHistoryCount();
		DontSwapListCount += cur_client->GetDontSwapListCount();
		BlockRequestedCount += cur_client->GetBlockRequestedCount();
		DoneBlocksCount += cur_client->GetDoneBlocksCount();
		RequestedFilesCount += cur_client->GetRequestedFilesCount();
		PendingBlockCount += cur_client->GetPendingBlockCount();
		DownloadBlockCount += cur_client->GetDownloadBlockCount();
		NoNeededListCount += cur_client->GetNoNeededListCount();
		OtherRequestListCount += cur_client->GetOtherRequestListCount();
	}	
	AddLogLine(false, _T("PartStatusMapCount: %u"), PartStatusMapCount);
	AddLogLine(false, _T("upHistoryCount: %u"), upHistoryCount);
	AddLogLine(false, _T("downHistoryCount %u"), downHistoryCount);
	AddLogLine(false, _T("DontSwapListCount: %u"), DontSwapListCount);
	AddLogLine(false, _T("BlockRequestedCount: %u"), BlockRequestedCount);
	AddLogLine(false, _T("DoneBlocksCount: %u"), DoneBlocksCount);
	AddLogLine(false, _T("RequestedFilesCount: %u"), RequestedFilesCount);
	AddLogLine(false, _T("PendingBlockCount: %u"), PendingBlockCount);
	AddLogLine(false, _T("DownloadBlockCount: %u"), DownloadBlockCount);
	AddLogLine(false, _T("NoNeededListCount: %u"), NoNeededListCount);
	AddLogLine(false, _T("OtherRequestListCount: %u"), OtherRequestListCount);
	AddLogLine(false, _T("------------------------------------------------"));
}
#endif

CDeletedClient::CDeletedClient(const CUpDownClient* pClient)
{
	m_cBadRequest = 0;
	m_dwInserted = ::GetTickCount();
	//Xman Extened credit- table-arragement
	//make the Tracked-client-list independent 
	//track new port & hash
	PORTANDHASH porthash;
	porthash.nPort= pClient->GetUserPort();
	md4cpy(porthash.pHash,pClient->GetUserHash());
	//Xman end
	m_ItemsList.Add(porthash);
}

void CClientList::AddKadFirewallRequest(uint32 dwIP){
	IPANDTICS add = {dwIP, ::GetTickCount()};
	listFirewallCheckRequests.AddHead(add);
	while (!listFirewallCheckRequests.IsEmpty()){
		if (::GetTickCount() - listFirewallCheckRequests.GetTail().dwInserted > SEC2MS(180))
			listFirewallCheckRequests.RemoveTail();
		else
			break;
	}
}

bool CClientList::IsKadFirewallCheckIP(uint32 dwIP) const{
	for (POSITION pos = listFirewallCheckRequests.GetHeadPosition(); pos != NULL; listFirewallCheckRequests.GetNext(pos)){
		if (listFirewallCheckRequests.GetAt(pos).dwIP == dwIP && ::GetTickCount() - listFirewallCheckRequests.GetAt(pos).dwInserted < SEC2MS(180))
			return true;
	}
	return false;
}

//Xman -Reask sources after IP change- v4 
void CClientList::TrigReaskForDownload(bool immediate){
	for(POSITION pos = list.GetHeadPosition(); pos != NULL;){				
		CUpDownClient* cur_client =	list.GetNext(pos);
		if(immediate == true){
			// Compute the next time that the file might be saftly reasked (=> no Ban())
			cur_client->SetNextTCPAskedTime(0);
		}
		else{
			// Compute the next time that the file might be saftly reasked (=> no Ban())
			cur_client->TrigNextSafeAskForDownload(cur_client->GetRequestFile());
		}
	}	
}
//Xman end

//EastShare Start - added by AndCycle, IP to Country
void CClientList::ResetIP2Country(){

	CUpDownClient *cur_client;

	for(POSITION pos = list.GetHeadPosition(); pos != NULL; list.GetNext(pos)) { 
		cur_client = theApp.clientlist->list.GetAt(pos); 
		cur_client->ResetIP2Country();
	}

}
//EastShare End - added by AndCycle, IP to Country

#ifdef NATTUNNELING // NEO: XSB - [XSBuddy] -- Xanatos -->
EXsBuddy CClientList::GetXsBuddyStatus(){ 
	if(m_pXsBuddy == NULL) 
		return XB_NONE;
	if(m_pXsBuddy->GetXsBuddyStatus() == XB_DENIDED)
		return XB_NONE;
	return m_pXsBuddy->GetXsBuddyStatus(); 
}

bool CClientList::CanAcceptXsBuddy(){
	uint16 buddys = 0;
	for (POSITION pos = list.GetHeadPosition(); pos != NULL; )
	{
		if(list.GetNext(pos)->GetXsBuddyStatus() == XB_LOW_BUDDY)
			buddys ++;
	}

	// for the moment we allow up to 3 buddys
	return buddys < 3;
}
#endif //NATTUNNELING // NEO: XSB END <-- Xanatos --
//>>> WiZaRd::ModIconMapper
void CClientList::UpdateModIconIndexes() const
{
	for(POSITION pos = list.GetHeadPosition(); pos != NULL;)  
		list.GetNext(pos)->CheckModIconIndex();
}
//<<< WiZaRd::ModIconMapper
