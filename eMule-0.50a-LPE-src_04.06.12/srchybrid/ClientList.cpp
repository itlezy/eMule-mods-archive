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
#include "kademlia/kademlia/UDPFirewallTester.h"
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

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CClientList::CClientList(){
	// ==> {relax on startup} [WiZaRd] 
	/*
	m_dwLastBannCleanUp = 0;
	m_dwLastTrackedCleanUp = 0;
	m_dwLastClientCleanUp = 0;
	*/
	const uint32 cur_tick = ::GetTickCount(); 
	m_dwLastBannCleanUp = cur_tick + CLIENTBANTIME; 
	m_dwLastTrackedCleanUp = cur_tick + KEEPTRACK_TIME; 
	m_dwLastClientCleanUp = cur_tick + CLIENTLIST_CLEANUP_TIME;
	// <== {relax on startup} [WiZaRd] 

	m_nBuddyStatus = Disconnected;
#ifdef REPLACE_ATLMAP
	unordered_map<uint32, uint32>(571).swap(m_bannedList);
	unordered_map<uint32, CDeletedClient*>(4999).swap(m_trackedClientsList);
#else
	m_bannedList.InitHashTable(571); //Xman changed, was 331
	m_trackedClientsList.InitHashTable(4999); //Xman changed, was 2011
#endif
	m_globDeadSourceList.Init(true);
	m_pBuddy = NULL;
}

CClientList::~CClientList(){
	RemoveAllTrackedClients();
}

/*
0 eMule
1 eDonkeyHybird
2 eDonkey
3 aMule
4 MLDonkey
5 shareaza
6 compact
7 mods
8 default port
9 other port
10 unknown client
11 error
12 identified
13 failed
14 LowID
15 eD2K
16 Kad
17 eD2K/Kad
18 unknown

19 Hydranode
20 eMule Plus
21 TrustyFiles
22 cDonkey
23 xMule
24 lphant
25 easymule2
*/
void CClientList::GetStatistics(uint_ptr &ruTotalClients, uint_ptr stats[NUM_CLIENTLIST_STATS],
#ifdef REPLACE_ATLMAP
								unordered_map<uint32, uint_ptr>**clientMaps
								//Xman extended stats
								unordered_map<POSITION, uint_ptr>& MODs;
#else
								CAtlMap<uint32, uint_ptr>**clientMaps,
								//Xman extended stats
								CAtlMap<POSITION, uint_ptr>& MODs
#endif
								//,uint32 &totalMODs
								//Xman end
								)
{
	ruTotalClients = (uint_ptr)list.GetCount();
	memset(stats, 0, sizeof(stats[0]) * NUM_CLIENTLIST_STATS);

	//Xman extended stats
	POSITION			pos_MOD;
	CString				strMODName;
#ifndef REPLACE_ATLMAP
	CAtlMap<uint32, uint_ptr>::CPair*pPairClient = NULL;
#endif
	//reset values
	//MODs.RemoveAll();
	static uint32 lastmodlistclean;
	if(::GetTickCount()-lastmodlistclean> HR2MS(6))
	{
		//don¡ät clean it up every time -> jumping statistics
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
				stats[0]++;
				if(clientMaps[0]){
#ifdef REPLACE_ATLMAP
					(*clientMaps[0])[cur_client->GetVersion()]++;
#else
					pPairClient = clientMaps[0]->Lookup(cur_client->GetVersion());
					if(pPairClient != NULL)
						++pPairClient->m_value;
					else
						clientMaps[0]->SetAt(cur_client->GetVersion(), 1);
#endif
				}
			
				//Xman extended stats
				strMODName = cur_client->GetClientModVer();

				if (!strMODName.IsEmpty())
				{
					stats[7]++;
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

#ifdef REPLACE_ATLMAP
					pos_MOD = liMODsTypes.Find(strMODName);
					if (!pos_MOD)
					{
						pos_MOD = liMODsTypes.AddTail(strMODName);
					}
					++statsmaps.MODs[pos_MOD];
#else
					pos_MOD = liMODsTypes.Find(strMODName);
					if (!pos_MOD)
					{
						pos_MOD = liMODsTypes.AddTail(strMODName);
					}
					CAtlMap<POSITION, uint_ptr>::CPair *pPairMOD = MODs.Lookup(pos_MOD);
					if(pPairMOD != NULL)
						++pPairMOD->m_value;
					else
						MODs.SetAt(pos_MOD, 1);
#endif
				}
				//Xman end
				break;

			case SO_EDONKEYHYBRID : 
				stats[1]++;
				if(clientMaps[1]){
#ifdef REPLACE_ATLMAP
					(*clientMaps[1])[cur_client->GetVersion()]++;
#else
					pPairClient = clientMaps[1]->Lookup(cur_client->GetVersion());
					if(pPairClient != NULL)
						++pPairClient->m_value;
					else
						clientMaps[1]->SetAt(cur_client->GetVersion(), 1);
#endif
				}
				break;
			
			case SO_AMULE:
				stats[3]++;
				if(clientMaps[3]){
#ifdef REPLACE_ATLMAP
					(*clientMaps[3])[cur_client->GetVersion()]++;
#else
					pPairClient = clientMaps[3]->Lookup(cur_client->GetVersion());
					if(pPairClient != NULL)
						++pPairClient->m_value;
					else
						clientMaps[3]->SetAt(cur_client->GetVersion(), 1);
#endif
				}
				break;

			case SO_EDONKEY:
				stats[2]++;
				if(clientMaps[2]){
#ifdef REPLACE_ATLMAP
					(*clientMaps[2])[cur_client->GetVersion()]++;
#else
					pPairClient = clientMaps[2]->Lookup(cur_client->GetVersion());
					if(pPairClient != NULL)
						++pPairClient->m_value;
					else
						clientMaps[2]->SetAt(cur_client->GetVersion(), 1);
#endif
				}
				break;

			case SO_MLDONKEY:
				stats[4]++;
				break;
			
			case SO_SHAREAZA:
				stats[5]++;
				break;

			// all remaining 'eMule Compatible' clients
			// Spike2 - Enhanced Client Recognition v2 - START
			case SO_HYDRANODE:
				stats[6]++;
				++stats[OFFICIAL_CLIENTLIST_STATS];
				break;
			case SO_EMULEPLUS:
				stats[6]++;
				++stats[OFFICIAL_CLIENTLIST_STATS + 1];
				break;
			case SO_TRUSTYFILES:
				stats[6]++;
				++stats[OFFICIAL_CLIENTLIST_STATS + 2];
				break;
			// Spike2 - Enhanced Client Recognition v2 - END
			case SO_CDONKEY:
				stats[6]++;
				++stats[OFFICIAL_CLIENTLIST_STATS + 3];
				break;
			case SO_XMULE:
				stats[6]++;
				++stats[OFFICIAL_CLIENTLIST_STATS + 4];
				break;
			case SO_LPHANT:
				stats[6]++;
				++stats[OFFICIAL_CLIENTLIST_STATS + 5];
				break;
			case SO_EASYMULE2:
				stats[6]++;
				++stats[OFFICIAL_CLIENTLIST_STATS + 6];
				break;

			default:
				stats[10]++;
				break;
		}

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

		if (cur_client->GetDownloadState()==DS_ERROR)
			stats[11]++; // Error

		if (cur_client->GetUserPort() == 4662)
				stats[8]++; // Default Port
		else if (cur_client->GetUserPort() != 0)// X: [BF] - [Bug Fix]
				stats[9]++; // Other Port

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
		if(!theApp.uploadqueue->RemoveFromUploadQueue(toremove, CString(_T("CClientList::RemoveClient: ")) + pszReason))
			theApp.uploadqueue->RemoveFromWaitingQueue(toremove);
		else
			ASSERT(!theApp.uploadqueue->RemoveFromWaitingQueue(toremove));
		theApp.downloadqueue->RemoveSource(toremove);
		list.RemoveAt(pos);
	}
	RemoveFromKadList(toremove);
	RemoveConnectingClient(toremove);
}

void CClientList::DeleteAll(){
	theApp.uploadqueue->DeleteAll();
	theApp.downloadqueue->DeleteAll();

	while (!list.IsEmpty())// X: [CI] - [Code Improvement]
		delete list.RemoveHead();
	liMODsTypes.RemoveAll(); //Xman extended stats
}

bool CClientList::AttachToAlreadyKnown(CUpDownClient** client, CClientReqSocket* sender){
	CUpDownClient* tocheck = (*client);
	CUpDownClient* found_client = NULL;
	CUpDownClient* found_client2 = NULL;
	for (POSITION pos = list.GetHeadPosition(); pos != NULL; ){// X: [CI] - [Code Improvement]
		CUpDownClient* cur_client =	list.GetNext(pos);
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
						//if (thePrefs.GetLogBannedClients())
							//AddDebugLogLine(false, _T("Clients: %s (%s), Banreason: Userhash invalid"), tocheck->GetUserName(), ipstr(tocheck->GetConnectIP()));
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

CUpDownClient* CClientList::FindClientByUserHash(const uchar* clienthash, uint32 dwIP, uint16 nTCPPort) const
{
	CUpDownClient* pFound = NULL;
	for (POSITION pos = list.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* cur_client = list.GetNext(pos);
		if (!md4cmp(cur_client->GetUserHash() ,clienthash)){
			if ((dwIP == 0 || dwIP == cur_client->GetIP()) && (nTCPPort == 0 || nTCPPort == cur_client->GetUserPort())) 
				return cur_client;
			else if(pFound == NULL)
				pFound = cur_client;
		}
	}
	return pFound;
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
#ifdef REPLACE_ATLMAP
	m_bannedList[dwIP] = ::GetTickCount();
#else
	m_bannedList.SetAt(dwIP, ::GetTickCount());
#endif
}

bool CClientList::IsBannedClient(uint32 dwIP) const
{
#ifdef REPLACE_ATLMAP
	unordered_map<uint32, uint32>::const_iterator it = m_bannedList.find(dwIP);
	if(it != m_bannedList.end()){
		uint32 dwBantime = it->second;
#else
	uint32 dwBantime;
	if (m_bannedList.Lookup(dwIP, dwBantime)){
#endif
		if (dwBantime + CLIENTBANTIME > ::GetTickCount())
			return true;
	}
	return false; 
}

void CClientList::RemoveBannedClient(uint32 dwIP){
#ifdef REPLACE_ATLMAP
	m_bannedList.erase(dwIP);
#else
	m_bannedList.RemoveKey(dwIP);
#endif
}

void CClientList::RemoveAllBannedClients(){
#ifdef REPLACE_ATLMAP
	unordered_map<uint32, uint32>(331).swap(m_bannedList);
#else
	m_bannedList.RemoveAll();
#endif
	theApp.emuledlg->transferwnd->ShowQueueCount();
}


///////////////////////////////////////////////////////////////////////////////
// Tracked clients
//Xman Extened credit- table-arragement
/*
void CClientList::AddTrackClient(CUpDownClient* toadd){
	CDeletedClient* pResult = 0;
	if (m_trackedClientsList.Lookup(toadd->GetIP(), pResult)){
		pResult->m_dwInserted = ::GetTickCount();
		for (size_t i = 0; i != pResult->m_ItemsList.GetCount(); i++){
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
#ifdef REPLACE_ATLMAP
	CDeletedClient* &pResult = m_trackedClientsList[toadd->GetIP()];
	if(pResult){
#else
	CDeletedClient* pResult = 0;
	if (m_trackedClientsList.Lookup(toadd->GetIP(), pResult)){
#endif
		pResult->m_dwInserted = ::GetTickCount();
		for (size_t i = 0; i != pResult->m_ItemsList.GetCount(); i++){
			if (pResult->m_ItemsList[i].nPort == toadd->GetUserPort()){
				// already tracked, update
				//Xman don't keep a track of the credit-pointer, but of the hash
				md4cpy(pResult->m_ItemsList[i].pHash, toadd->GetUserHash());
				return;
			}
		}
		//Xman new tracked port & hash
		PORTANDHASH porthash;
		md4cpy(porthash.pHash,toadd->GetUserHash());
		porthash.nPort=toadd->GetUserPort();
		pResult->m_ItemsList.Add(porthash);
	}
	else{
#ifdef REPLACE_ATLMAP
		pResult = new CDeletedClient(toadd);
#else
		m_trackedClientsList.SetAt(toadd->GetIP(), new CDeletedClient(toadd));
#endif
	}
}
//Xman end
// true = everything ok, hash didn't changed
// false = hash changed
bool CClientList::ComparePriorUserhash(uint32 dwIP, uint16 nPort, void* pNewHash){
#ifdef REPLACE_ATLMAP
	unordered_map<uint32, CDeletedClient*>::const_iterator it = m_trackedClientsList.find(dwIP);
	if(it != m_trackedClientsList.end()){
		CDeletedClient* pResult = it->second;
#else
	CDeletedClient* pResult = 0;
	if (m_trackedClientsList.Lookup(dwIP, pResult)){
#endif
		for (size_t i = 0; i != pResult->m_ItemsList.GetCount(); i++){
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

size_t CClientList::GetClientsFromIP(uint32 dwIP) const
{
#ifdef REPLACE_ATLMAP
	unordered_map<uint32, CDeletedClient*>::const_iterator it = m_trackedClientsList.find(dwIP);
	if(it != m_trackedClientsList.end()){
		return it->second->m_ItemsList.GetCount();
	}
#else
	CDeletedClient* pResult;
	if (m_trackedClientsList.Lookup(dwIP, pResult)){
		return pResult->m_ItemsList.GetCount();
	}
#endif
	return 0;
}

void CClientList::TrackBadRequest(const CUpDownClient* upcClient, int nIncreaseCounter){
	if (upcClient->GetIP() == 0){
		ASSERT( false );
		return;
	}
#ifdef REPLACE_ATLMAP
	CDeletedClient* &pResult = m_trackedClientsList[upcClient->GetIP()];
	if(pResult){
		pResult->m_dwInserted = ::GetTickCount();
		pResult->m_cBadRequest += nIncreaseCounter;
	}
	else{
		pResult = new CDeletedClient(upcClient);
		pResult->m_cBadRequest = nIncreaseCounter;
	}
#else
	CDeletedClient* pResult = NULL;
	if (m_trackedClientsList.Lookup(upcClient->GetIP(), pResult)){
		pResult->m_dwInserted = ::GetTickCount();
		pResult->m_cBadRequest += nIncreaseCounter;
	}
	else{
		CDeletedClient* ccToAdd = new CDeletedClient(upcClient);
		ccToAdd->m_cBadRequest = nIncreaseCounter;
		m_trackedClientsList.SetAt(upcClient->GetIP(), ccToAdd);
	}
#endif
}

uint32 CClientList::GetBadRequests(const CUpDownClient* upcClient) const{
	CDeletedClient* pResult = NULL;
	if (upcClient->GetIP() == 0){
		ASSERT( false );
		return 0;
	}
#ifdef REPLACE_ATLMAP
	unordered_map<uint32, CDeletedClient*>::const_iterator it = m_trackedClientsList.find(upcClient->GetIP());
	if(it != m_trackedClientsList.end()){
		return it->second->m_cBadRequest;
	}
#else
	if (m_trackedClientsList.Lookup(upcClient->GetIP(), pResult)){
		return pResult->m_cBadRequest;
	}
#endif
	else
		return 0;
}

void CClientList::RemoveAllTrackedClients(){
#ifdef REPLACE_ATLMAP
	for(unordered_map<uint32, CDeletedClient*>::const_iterator it = m_trackedClientsList.begin(); it != m_trackedClientsList.end();)
	{
		CDeletedClient* pResult = it->second;
		it = m_trackedClientsList.erase(it);
		delete pResult;
	}
#else
	POSITION pos = m_trackedClientsList.GetStartPosition();
	uint32 nKey;
	CDeletedClient* pResult;
	while (pos != NULL){
		m_trackedClientsList.GetNextAssoc(pos, nKey, pResult);
		//m_trackedClientsList.RemoveKey(nKey);
		delete pResult;
	}
#endif
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
		
#ifdef REPLACE_ATLMAP
		for(unordered_map<uint32, uint32>::const_iterator it = m_bannedList.begin(); it != m_bannedList.end();)
		{
			if (it->second + CLIENTBANTIME < cur_tick )
				it = m_bannedList.erase(it);
			else
				++it;
		}
#else
		POSITION pos = m_bannedList.GetStartPosition();
		uint32 nKey;
		uint32 dwBantime;
		while (pos != NULL)
		{
			m_bannedList.GetNextAssoc( pos, nKey, dwBantime );
			if (dwBantime + CLIENTBANTIME < cur_tick )
				RemoveBannedClient(nKey);
		}
#endif
		theApp.emuledlg->transferwnd->ShowQueueCount();
	}

	///////////////////////////////////////////////////////////////////////////
	// Cleanup tracked client list
	//
	if (m_dwLastTrackedCleanUp + TRACKED_CLEANUP_TIME < cur_tick)
	{
		m_dwLastTrackedCleanUp = cur_tick;
#ifdef REPLACE_ATLMAP
		if (thePrefs.GetLogBannedClients()) 
			AddDebugLogLine(false, _T("Cleaning up TrackedClientList, %i clients on List..."), m_trackedClientsList.size());
		for(unordered_map<uint32, CDeletedClient*>::const_iterator it = m_trackedClientsList.begin(); it != m_trackedClientsList.end();)
		{
			CDeletedClient* pResult = it->second;
			if (pResult->m_dwInserted + KEEPTRACK_TIME < cur_tick ){
				it = m_trackedClientsList.erase(it);
				delete pResult;
			}				
			else
				++it;
		}
		if (thePrefs.GetLogBannedClients())
			AddDebugLogLine(false, _T("...done, %i clients left on list"), m_trackedClientsList.size());
#else
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
#endif
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

	for (pos1 = m_KadList.GetHeadPosition(); (pos2 = pos1) != NULL; ){
		CUpDownClient* cur_client =	m_KadList.GetNext(pos1);
		if( !Kademlia::CKademlia::IsRunning() )
		{
			//Clear out this list if we stop running Kad.
			//Setting the Kad state to KS_NONE causes it to be removed in the switch below.
			cur_client->SetKadState(KS_NONE);
		}
		switch(cur_client->GetKadState())
		{
			case KS_QUEUED_FWCHECK:
			case KS_QUEUED_FWCHECK_UDP:
				//Another client asked us to try to connect to them to check their firewalled status.
				cur_client->TryToConnect(true, true);
				break;
			case KS_CONNECTING_FWCHECK:
				//Ignore this state as we are just waiting for results.
				break;
			case KS_FWCHECK_UDP:
			case KS_CONNECTING_FWCHECK_UDP:
				// we want a UDP firewallcheck from this client and are just waiting to get connected to send the request
				break;
			case KS_CONNECTED_FWCHECK:
				//We successfully connected to the client.
				//We now send a ack to let them know.
				if (cur_client->GetKadVersion() >= KADEMLIA_VERSION7_49a){
					// the result is now sent per TCP instead of UDP, because this will fail if our intern UDP port is unreachable.
					// But we want the TCP testresult regardless if UDP is firewalled, the new UDP state and test takes care of the rest					
					ASSERT( cur_client->socket != NULL && cur_client->socket->IsConnected() );
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugSend("OP_KAD_FWTCPCHECK_ACK", cur_client);
#endif
					Packet* pPacket = new Packet(OP_KAD_FWTCPCHECK_ACK, 0, OP_EMULEPROT);
					if (!cur_client->SafeConnectAndSendPacket(pPacket))
						cur_client = NULL;
				}
				else {
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientKadUDPLevel() > 0)
						DebugSend("KADEMLIA_FIREWALLED_ACK_RES", cur_client->GetIP(), cur_client->GetKadPort());
#endif
					Kademlia::CKademlia::GetUDPListener()->SendNullPacket(KADEMLIA_FIREWALLED_ACK_RES, ntohl(cur_client->GetIP()), cur_client->GetKadPort(), 0, NULL);
				}
				//We are done with this client. Set Kad status to KS_NONE and it will be removed in the next cycle.
				if (cur_client != NULL)
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
					cur_client->TryToConnect(true, true);
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
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugSend("OP__BuddyPing", cur_client);
#endif
					Packet* buddyPing = new Packet(OP_BUDDYPING, 0, OP_EMULEPROT);
					theStats.AddUpDataOverheadOther(buddyPing->size);
					VERIFY( cur_client->SendPacket(buddyPing, true, true) );
					cur_client->SetLastBuddyPingPongTime();
				}
				break;

			default:
				/*
				RemoveFromKadList(cur_client);
				*/
				//Enig123::WiZaRd::Optimization - removed function overhead
				if(cur_client == m_pBuddy)
				{
					buddy = Disconnected;
					m_pBuddy = NULL;			
					m_nBuddyStatus = Disconnected; //>>> WiZaRd::FiX?
					theApp.emuledlg->serverwnd->UpdateMyInfo();
				}
				m_KadList.RemoveAt(pos2);
				//Enig123::WiZaRd::Optimization <<-
				break;
		}
	}
	
	//We either never had a buddy, or lost our buddy..
	if( buddy == Disconnected )
	{
		if( m_nBuddyStatus != Disconnected || m_pBuddy )
		{
			if( Kademlia::CKademlia::IsRunning() && theApp.IsFirewalled() && Kademlia::CUDPFirewallTester::IsFirewalledUDP(true))
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
		//we only need a buddy if direct callback is not available
		if( Kademlia::CKademlia::IsFirewalled() && Kademlia::CUDPFirewallTester::IsFirewalledUDP(true))
		{
			//TODO 0.49b: Kad buddies won'T work with RequireCrypt, so it is disabled for now but should (and will)
			//be fixed in later version
			// Update: Buddy connections itself support obfuscation properly since 0.49a (this makes it work fine if our buddy uses require crypt)
			// ,however callback requests don't support it yet so we wouldn't be able to answer callback requests with RequireCrypt, protocolchange intended for the next version
			if( m_nBuddyStatus == Disconnected && Kademlia::CKademlia::GetPrefs()->GetFindBuddy() && !thePrefs.IsClientCryptLayerRequired())
			{
				DEBUG_ONLY( DebugLog(_T("Starting Buddysearch")) );
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

	///////////////////////////////////////////////////////////////////////////
	// Cleanup client list
	//
	//CleanUpClientList(); //Xman moved to uploadqueue

	///////////////////////////////////////////////////////////////////////////
	// Process Direct Callbacks for Timeouts
	//
	ProcessConnectingClientsList();
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
#ifdef _DEBUG// X: [RDL] - [Remove Debug Log]
	if (thePrefs.m_iDbgHeap >= 2)
		ASSERT_VALID(tocheck);
#endif
	return list.Find(tocheck)!=NULL;
}


///////////////////////////////////////////////////////////////////////////////
// Kad client list

bool CClientList::RequestTCP(Kademlia::CContact* contact, uint8 byConnectOptions)
{
	uint32 nContactIP = ntohl(contact->GetIPAddress());
	// don't connect ourself
	if (theApp.serverconnect->GetLocalIP() == nContactIP && thePrefs.GetPort() == contact->GetTCPPort())
		return false;

	CUpDownClient* pNewClient = FindClientByIP(nContactIP, contact->GetTCPPort());

	const bool bNewClient = pNewClient == NULL; //Xman Code Improvement don't search new generated clients in lists (seen by Wizard)

	if (!pNewClient)
		pNewClient = new CUpDownClient(0, contact->GetTCPPort(), contact->GetIPAddress(), 0, 0, false );
	else if (pNewClient->GetKadState() != KS_NONE)
		return false; // already busy with this client in some way (probably buddy stuff), don't mess with it

	//Add client to the lists to be processed.
	pNewClient->SetKadPort(contact->GetUDPPort());
	pNewClient->SetKadState(KS_QUEUED_FWCHECK);
	if (contact->GetClientID() != 0){
		byte ID[16];
		contact->GetClientID().ToByteArray(ID);
		pNewClient->SetUserHash(ID);
		pNewClient->SetConnectOptions(byConnectOptions, true, false);
	}
	//Xman Code Improvement don't search new generated clients in lists (seen by Wizard)
	if(bNewClient)
	{
		m_KadList.AddTail(pNewClient);
		AddClient(pNewClient, true); 
	}
	else
		AddToKadList(pNewClient); 
	//Xman end
	return true;
}

void CClientList::RequestBuddy(Kademlia::CContact* contact, uint8 byConnectOptions)
{
	uint32 nContactIP = ntohl(contact->GetIPAddress());
	// don't connect ourself
	if (theApp.serverconnect->GetLocalIP() == nContactIP && thePrefs.GetPort() == contact->GetTCPPort())
		return;
	CUpDownClient* pNewClient = FindClientByIP(nContactIP, contact->GetTCPPort());

	const bool bNewClient = pNewClient == NULL; //Xman Code Improvement don't search new generated clients in lists (seen by Wizard)

	if (!pNewClient)
		pNewClient = new CUpDownClient(0, contact->GetTCPPort(), contact->GetIPAddress(), 0, 0, false );
	else if (pNewClient->GetKadState() != KS_NONE)
		return; // already busy with this client in some way (probably fw stuff), don't mess with it
	else if (IsKadFirewallCheckIP(nContactIP)){ // doing a kad firewall check with this IP, abort 
		DEBUG_ONLY( DebugLogWarning(_T("KAD tcp Firewallcheck / Buddy request collosion for IP %s"), ipstr(nContactIP)) );
		return;
	}
	//Add client to the lists to be processed.
	pNewClient->SetKadPort(contact->GetUDPPort());
	pNewClient->SetKadState(KS_QUEUED_BUDDY);
	byte ID[16];
	contact->GetClientID().ToByteArray(ID);
	pNewClient->SetUserHash(ID);
	pNewClient->SetConnectOptions(byConnectOptions, true, false);
	//Xman Code Improvement don't search new generated clients in lists (seen by Wizard)
	if(bNewClient)
	{
		m_KadList.AddTail(pNewClient);
		AddClient(pNewClient, true); 
	}
	else
		AddToKadList(pNewClient); 
	//Xman end
}

bool CClientList::IncomingBuddy(Kademlia::CContact* contact, Kademlia::CUInt128* buddyID )
{
	uint32 nContactIP = ntohl(contact->GetIPAddress());
	//If eMule already knows this client, abort this.. It could cause conflicts.
	//Although the odds of this happening is very small, it could still happen.
	if (FindClientByIP(nContactIP, contact->GetTCPPort()))
		return false;
	if (IsKadFirewallCheckIP(nContactIP)){ // doing a kad firewall check with this IP, abort 
		DEBUG_ONLY( DebugLogWarning(_T("KAD tcp Firewallcheck / Buddy request collosion for IP %s"), ipstr(nContactIP)) );
		return false;
	}
	if (theApp.serverconnect->GetLocalIP() == nContactIP && thePrefs.GetPort() == contact->GetTCPPort())
		return false; // don't connect ourself

	//Add client to the lists to be processed.
	CUpDownClient* pNewClient = new CUpDownClient(0, contact->GetTCPPort(), contact->GetIPAddress(), 0, 0, false );
	pNewClient->SetKadPort(contact->GetUDPPort());
	pNewClient->SetKadState(KS_INCOMING_BUDDY);
	byte ID[16];
	contact->GetClientID().ToByteArray(ID);
	pNewClient->SetUserHash(ID); //??
	buddyID->ToByteArray(ID);
	pNewClient->SetBuddyID(ID);
	//Xman Code Improvement don't search new generated clients in lists (seen by Wizard)
	//Xman it's a new client -> no dupe check
	//AddToKadList(pNewClient);
	//AddClient(pNewClient);
	m_KadList.AddTail(pNewClient);
	AddClient(pNewClient, true); 
	//Xman end
	return true;
}

void CClientList::RemoveFromKadList(CUpDownClient* torem){
	POSITION pos = m_KadList.Find(torem);
	if(pos)
	{
		if(torem == m_pBuddy)
		{
			m_pBuddy = NULL;
			m_nBuddyStatus = Disconnected; //Enig123::WiZaRd::FiX?
			theApp.emuledlg->serverwnd->UpdateMyInfo();
		}
		m_KadList.RemoveAt(pos);
	}
}

void CClientList::AddToKadList(CUpDownClient* toadd){
	if(!toadd)
		return;
	POSITION pos = m_KadList.Find(toadd);
	if(!pos)
		m_KadList.AddTail(toadd);
}

bool CClientList::DoRequestFirewallCheckUDP(const Kademlia::CContact& contact){
	// first make sure we don't know this IP already from somewhere
	if (FindClientByIP(ntohl(contact.GetIPAddress())) != NULL)
		return false;
	// fine, justcreate the client object, set the state and wait
	// TODO: We don't know the clients usershash, this means we cannot build an obfuscated connection, which 
	// again mean that the whole check won't work on "Require Obfuscation" setting, which is not a huge problem,
	// but certainly not nice. Only somewhat acceptable way to solve this is to use the KadID instead.
	CUpDownClient* pNewClient = new CUpDownClient(0, contact.GetTCPPort(), contact.GetIPAddress(), 0, 0, false );
	pNewClient->SetKadState(KS_QUEUED_FWCHECK_UDP);
	DebugLog(_T("Selected client for UDP Firewallcheck: %s"), ipstr(ntohl(contact.GetIPAddress())));
	/*
	AddToKadList(pNewClient);
	AddClient(pNewClient);
	*/	//Enig123::Optimizations
	m_KadList.AddTail(pNewClient);
	AddClient(pNewClient, true);
	ASSERT( !pNewClient->SupportsDirectUDPCallback() );
	return true;
}

// Maella -Extended clean-up II- //rework by Xman
//Note: this feature is important for Xtreme Downloadmanager
void CClientList::CleanUpClientList(){
	const uint32 cur_tick = ::GetTickCount();
	if (m_dwLastClientCleanUp + CLIENTLIST_CLEANUP_TIME < cur_tick ){
		m_dwLastClientCleanUp = cur_tick;
		uint_ptr cDeleted = 0;
		for (POSITION pos = list.GetHeadPosition(); pos != NULL;){
			CUpDownClient* pCurClient =	list.GetNext(pos);
			if ((pCurClient->GetUploadState() == US_NONE || pCurClient->GetUploadState() == US_BANNED /* && !pCurClient->IsBanned()*/) //Xman Code Improvement: how should this happen ?
				&& pCurClient->GetDownloadState() == DS_NONE
				&& pCurClient->GetChatState() == MS_NONE
				&& pCurClient->GetKadState() == KS_NONE
				&& pCurClient->socket == NULL
                && pCurClient->GetRequestFile() == NULL) // Dazzle: it's possible that a client is added to queue, later is dropped because of queue-full.
			{
				const DWORD delta = cur_tick - pCurClient->m_lastCleanUpCheck;
				if(delta > HR2MS(2)){ // 2 hour
					if(!pCurClient->m_OtherNoNeeded_list.IsEmpty() || !pCurClient->m_OtherRequests_list.IsEmpty())
					{
						AddDebugLogLine(false, _T("Extended clean-up reports an error in CleanUpProcess with client %s"),pCurClient->DbgGetClientInfo());
						pCurClient->m_lastCleanUpCheck = cur_tick;
					}
					else
					{
						cDeleted++;
						delete pCurClient;
					}
				}
			}
			else{
				pCurClient->m_lastCleanUpCheck = cur_tick;
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

CDeletedClient::CDeletedClient(const CUpDownClient* pClient)
{
	m_cBadRequest = 0;
	m_dwInserted = ::GetTickCount();
	//Xman Extened credit- table-arragement
	//make the Tracked-client-list independent 
	//track new port & hash
	PORTANDHASH porthash;
	md4cpy(porthash.pHash,pClient->GetUserHash());
	porthash.nPort= pClient->GetUserPort();
	//Xman end
	m_ItemsList.Add(porthash);
}

void CClientList::AddKadFirewallRequest(uint32 dwIP){
	IPANDTICS add = {dwIP, ::GetTickCount()};
	listFirewallCheckRequests.AddHead(add);
	while (!listFirewallCheckRequests.IsEmpty()){
		if (add.dwInserted - listFirewallCheckRequests.GetTail().dwInserted > SEC2MS(180))
			listFirewallCheckRequests.RemoveTail();
		else
			break;
	}
}

bool CClientList::IsKadFirewallCheckIP(uint32 dwIP) const{
	const DWORD cur_tick = GetTickCount();
	for (POSITION pos = listFirewallCheckRequests.GetHeadPosition(); pos != NULL; listFirewallCheckRequests.GetNext(pos)){
		if (listFirewallCheckRequests.GetAt(pos).dwIP == dwIP && cur_tick - listFirewallCheckRequests.GetAt(pos).dwInserted < SEC2MS(180))
			return true;
	}
	return false;
}

void CClientList::AddConnectingClient(CUpDownClient* pToAdd){
	for (POSITION pos = m_liConnectingClients.GetHeadPosition(); pos != NULL; ){// X: [CI] - [Code Improvement]
		if (m_liConnectingClients.GetNext(pos).pClient == pToAdd){
			ASSERT( false );
			return;
		}
	}
	ASSERT( pToAdd->GetConnectingState() != CCS_NONE );
	CONNECTINGCLIENT cc = {pToAdd, ::GetTickCount()};
	m_liConnectingClients.AddTail(cc);
}
void CClientList::ProcessConnectingClientsList(){
	// we do check if any connects have timed out by now
	const uint32 cur_tick = ::GetTickCount();
	for (POSITION pos1 = m_liConnectingClients.GetHeadPosition(), pos2;(pos2 = pos1) != NULL;){// X: [CI] - [Code Improvement]
		CONNECTINGCLIENT cc = m_liConnectingClients.GetNext(pos1);
		if (cc.dwInserted + SEC2MS(45) < cur_tick){
			ASSERT( cc.pClient->GetConnectingState() != CCS_NONE );
			m_liConnectingClients.RemoveAt(pos2);
			CString dbgInfo;
			if (cc.pClient->Disconnected(_T("Connectiontry Timeout")))
				delete cc.pClient;
		}
	}
}

void CClientList::RemoveConnectingClient(CUpDownClient* pToRemove){
	for (POSITION pos = m_liConnectingClients.GetHeadPosition(); pos != NULL; m_liConnectingClients.GetNext(pos)){
		if (m_liConnectingClients.GetAt(pos).pClient == pToRemove){
			m_liConnectingClients.RemoveAt(pos);
			return;
		}
	}
}

void CClientList::AddTrackCallbackRequests(uint32 dwIP){
	IPANDTICS add = {dwIP, ::GetTickCount()};
	listDirectCallbackRequests.AddHead(add);
	while (!listDirectCallbackRequests.IsEmpty()){
		if (add.dwInserted - listDirectCallbackRequests.GetTail().dwInserted > MIN2MS(3))
			listDirectCallbackRequests.RemoveTail();
		else
			break;
	}	
}

bool CClientList::AllowCalbackRequest(uint32 dwIP) const
{
	const DWORD cur_tick = GetTickCount();
	for (POSITION pos = listDirectCallbackRequests.GetHeadPosition(); pos != NULL; listDirectCallbackRequests.GetNext(pos)){
		if (listDirectCallbackRequests.GetAt(pos).dwIP == dwIP && cur_tick - listDirectCallbackRequests.GetAt(pos).dwInserted < MIN2MS(3))
			return false;
	}
	return true;
}

#ifdef _DEBUG
void CClientList::PrintStatistic()
{
	AddLogLine(false,_T("Clients in Clientlist: %u"), list.GetCount());
	AddLogLine(false, _T("Clients in Bannedlist: %u"), m_bannedList.GetSize());
	AddLogLine(false, _T("Tracked Clients: %u"), m_trackedClientsList.GetSize());
	AddLogLine(false, _T("Clients in Kadlist: %u"), m_KadList.GetCount());

	AddLogLine(false, _T("sum of listelements of all known clients:"));
	size_t PartStatusMapCount=0;
	size_t upHistoryCount=0;
	size_t downHistoryCount=0;
	size_t DontSwapListCount=0;
	size_t BlockRequestedCount=0;
	size_t DoneBlocksCount=0;
	size_t RequestedFilesCount=0;
	size_t PendingBlockCount=0;
	size_t DownloadBlockCount=0;
	size_t NoNeededListCount=0;
	size_t OtherRequestListCount=0;
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
