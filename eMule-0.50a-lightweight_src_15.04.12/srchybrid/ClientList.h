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
#pragma once
#include "DeadSourceList.h"
//Xman
class CClientReqSocket;
class CUpDownClient;
namespace Kademlia{
	class CContact;
	class CUInt128;
};
typedef CAtlList<CUpDownClient*> CUpDownClientPtrList;

static LPCTSTR CompactClientName[] = {
	_T("Hydranode"),// 0
	_T("eMule Plus"),// 1
	_T("TrustyFiles"),// 2
	_T("cDonkey"),// 3
	_T("xMule"),// 4
	_T("lphant"),// 5
	_T("easyMule2")// 6
};
#define	OFFICIAL_CLIENTLIST_STATS	19
#define	NUM_CLIENTLIST_STATS	(OFFICIAL_CLIENTLIST_STATS+_countof(CompactClientName))
#define BAN_CLEANUP_TIME		1200000 // 20 min

//------------CDeletedClient Class----------------------
// this class / list is a bit overkill, but currently needed to avoid any exploit possibtility
// it will keep track of certain clients attributes for 2 hours, while the CUpDownClient object might be deleted already
// currently: IP, Port, UserHash

//Xman Extened credit- table-arragement
//make the Tracked-client-list independent 
/*
struct PORTANDHASH{
	uint16 nPort;
	void* pHash;
};
*/
struct PORTANDHASH{ // netfinity: Rearranged for alignment reasons
	uchar pHash[16];
	uint16 nPort;
};
//Xman end

struct IPANDTICS{
	uint32 dwIP;
	uint32 dwInserted;
};
struct CONNECTINGCLIENT{
	CUpDownClient* pClient;
	uint32 dwInserted;
};

class CDeletedClient{
public:
	CDeletedClient(const CUpDownClient* pClient);
	CAtlArray<PORTANDHASH> m_ItemsList;
	uint32				m_dwInserted;
	uint32				m_cBadRequest;
};

enum buddyState
{
	Disconnected,
	Connecting,
	Connected
};

// ----------------------CClientList Class---------------
class CClientList
{
public:
	CClientList();
	~CClientList();

	// Clients
	void	AddClient(CUpDownClient* toadd,bool bSkipDupTest = false);
	void	RemoveClient(CUpDownClient* toremove, LPCTSTR pszReason = NULL);
	void	GetStatistics(uint_ptr& totalclient, uint_ptr stats[NUM_CLIENTLIST_STATS],
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
						  );
	//Xman

	size_t	GetClientCount()	{ return list.GetCount();}
	void	DeleteAll();
	bool	AttachToAlreadyKnown(CUpDownClient** client, CClientReqSocket* sender);
	CUpDownClient* FindClientByIP(uint32 clientip, UINT port) const;
	CUpDownClient* FindClientByUserHash(const uchar* clienthash, uint32 dwIP = 0, uint16 nTCPPort = 0) const;
	CUpDownClient* FindClientByIP(uint32 clientip) const;
	CUpDownClient* FindClientByIP_UDP(uint32 clientip, UINT nUDPport) const;
	CUpDownClient* FindClientByServerID(uint32 uServerIP, uint32 uUserID) const;
	CUpDownClient* FindClientByUserID_KadPort(uint32 clientID,uint16 kadPort) const;
	CUpDownClient* FindClientByIP_KadPort(uint32 ip, uint16 port) const;

	// Banned clients
	void	AddBannedClient(uint32 dwIP);
	bool	IsBannedClient(uint32 dwIP) const;
	void	RemoveBannedClient(uint32 dwIP);
#ifdef REPLACE_ATLMAP
	size_t	GetBannedCount() const		{ return m_bannedList.size(); }
#else
	INT_PTR	GetBannedCount() const		{ return m_bannedList.GetCount(); }
#endif
	void	RemoveAllBannedClients();

	// Tracked clients
	void	AddTrackClient(CUpDownClient* toadd);
	bool	ComparePriorUserhash(uint32 dwIP, uint16 nPort, void* pNewHash);
	size_t	GetClientsFromIP(uint32 dwIP) const;
	void	TrackBadRequest(const CUpDownClient* upcClient, int nIncreaseCounter);
	uint32	GetBadRequests(const CUpDownClient* upcClient) const;
#ifdef REPLACE_ATLMAP
	size_t	GetTrackedCount() const		{ return m_trackedClientsList.size(); }
#else
	INT_PTR	GetTrackedCount() const		{ return m_trackedClientsList.GetCount(); }
#endif
	void	RemoveAllTrackedClients();

	// Kad client list, buddy handling
	bool	RequestTCP(Kademlia::CContact* contact, uint8 byConnectOptions);
	void	RequestBuddy(Kademlia::CContact* contact, uint8 byConnectOptions);
	bool	IncomingBuddy(Kademlia::CContact* contact, Kademlia::CUInt128* buddyID);
	void	RemoveFromKadList(CUpDownClient* torem);
	void	AddToKadList(CUpDownClient* toadd);
	bool	DoRequestFirewallCheckUDP(const Kademlia::CContact& contact);
	//bool	DebugDoRequestFirewallCheckUDP(uint32 ip, uint16 port);
	uint8	GetBuddyStatus()			{ return m_nBuddyStatus; }
	CUpDownClient* GetBuddy()			{ return m_pBuddy; }

	void	AddKadFirewallRequest(uint32 dwIP);
	bool	IsKadFirewallCheckIP(uint32 dwIP) const;

	// Direct Callback List
	void	AddTrackCallbackRequests(uint32 dwIP);
	bool	AllowCalbackRequest(uint32 dwIP) const;

	// Connecting Clients
	void	AddConnectingClient(CUpDownClient* pToAdd);
	void	RemoveConnectingClient(CUpDownClient* pToRemove);

	void	Process();
	bool	IsValidClient(CUpDownClient* tocheck) const;
	void	Debug_SocketDeleted(CClientReqSocket* deleted) const;

    // ZZ:UploadSpeedSense -->
    //bool GiveClientsForTraceRoute(); //Xman
	// ZZ:UploadSpeedSense <--

    //void	ProcessA4AFClients() const; // ZZ:DownloadManager //Xman
	CDeadSourceList	m_globDeadSourceList;

	// Maella -Extended clean-up II-
	void CleanUp(CPartFile* pDeletedFile);
	// Maella end

	//Xman -Reask sources after IP change- v4 
	void TrigReaskForDownload(bool immediate);

#ifdef _DEBUG
	void PrintStatistic();
#endif

	void	CleanUpClientList(); // Maella -Extended clean-up II-

protected:
	void	ProcessConnectingClientsList();
private:
	CUpDownClientPtrList list;
	CUpDownClientPtrList m_KadList;
#ifdef REPLACE_MFCMAP
	unordered_map<uint32, uint32> m_bannedList;
	unordered_map<uint32, CDeletedClient*> m_trackedClientsList;
#else
	CMap<uint32, uint32, uint32, uint32> m_bannedList;
	CMap<uint32, uint32, CDeletedClient*, CDeletedClient*> m_trackedClientsList;
#endif
	uint32	m_dwLastBannCleanUp;
	uint32	m_dwLastTrackedCleanUp;
	uint32 m_dwLastClientCleanUp;
	CUpDownClient* m_pBuddy;
	uint8 m_nBuddyStatus;
	CAtlList<IPANDTICS> listFirewallCheckRequests;
	CAtlList<IPANDTICS> listDirectCallbackRequests;
	CAtlList<CONNECTINGCLIENT> m_liConnectingClients;
public:

//Xman extended stats
	CString GetMODType(POSITION pos_in)
	{
		if (pos_in != 0)
			return liMODsTypes.GetAt(pos_in);
		else
			return _T("");
	}

protected:
	CAtlList<CString> liMODsTypes;
//Xman end
};
