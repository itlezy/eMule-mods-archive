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
#include <atlcoll.h>	// Slugfiller: modid
#include "IP2Country.h" //Xman extended stats

class CClientReqSocket;
class CUpDownClient;
namespace Kademlia{
	class CContact;
	class CUInt128;
};
typedef CTypedPtrList<CPtrList, CUpDownClient*> CUpDownClientPtrList;

#define	NUM_CLIENTLIST_STATS	19
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
struct PORTANDHASH{
	uint16 nPort;
	uchar pHash[16];
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
	CArray<PORTANDHASH> m_ItemsList;
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
	friend class CClientListCtrl;

public:
	CClientList();
	~CClientList();

	// Clients
	void	AddClient(CUpDownClient* toadd,bool bSkipDupTest = false);
	void	RemoveClient(CUpDownClient* toremove, LPCTSTR pszReason = NULL);
	void	GetStatistics(uint32& totalclient, int stats[NUM_CLIENTLIST_STATS],
						  CMap<uint32, uint32, uint32, uint32>& clientVersionEDonkey,
						  CMap<uint32, uint32, uint32, uint32>& clientVersionEDonkeyHybrid,
						  CMap<uint32, uint32, uint32, uint32>& clientVersionEMule,
						  //Xman extended stats
						  /*
						  CMap<uint32, uint32, uint32, uint32>& clientVersionAMule);
						  */
						  CMap<uint32, uint32, uint32, uint32>& clientVersionAMule,
						  CMap<POSITION, POSITION, uint32, uint32>& MODs,
						  uint32 &totalMODs,
						  CMap<Country_Struct*, Country_Struct*, uint32, uint32>& pCountries
						  );
						  //Xman end
	//Xman
	// Slugfiller: modid
	void	GetModStatistics(CRBMap<uint32, CRBMap<CString, uint32>* > *clientMods);
	void	ReleaseModStatistics(CRBMap<uint32, CRBMap<CString, uint32>* > *clientMods);
	// Slugfiller: modid

	uint32	GetClientCount()	{ return list.GetCount();}
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
	UINT	GetBannedCount() const		{ return m_bannedList.GetCount(); }
	void	RemoveAllBannedClients();

	// Tracked clients
	void	AddTrackClient(CUpDownClient* toadd);
	bool	ComparePriorUserhash(uint32 dwIP, uint16 nPort, void* pNewHash);
	UINT	GetClientsFromIP(uint32 dwIP) const;
	void	TrackBadRequest(const CUpDownClient* upcClient, int nIncreaseCounter);
	uint32	GetBadRequests(const CUpDownClient* upcClient) const;
	UINT	GetTrackedCount() const		{ return m_trackedClientsList.GetCount(); }
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
	//Xman
	/*
    bool GiveClientsForTraceRoute();
	// ZZ:UploadSpeedSense <--

    void	ProcessA4AFClients() const; // ZZ:DownloadManager
	*/
	//Xman end
	CDeadSourceList	m_globDeadSourceList;

	// Maella -Extended clean-up II-
	/*
protected:
	*/
	void CleanUp(CPartFile* pDeletedFile);
	// Maella end

	//Xman -Reask sources after IP change- v4 
	void TrigReaskForDownload(bool immediate);

#ifdef PRINT_STATISTIC
	void PrintStatistic();
#endif

	void	CleanUpClientList();
protected: // Maella -Extended clean-up II-
	void	ProcessConnectingClientsList();

private:
	CUpDownClientPtrList list;
	CUpDownClientPtrList m_KadList;
	CMap<uint32, uint32, uint32, uint32> m_bannedList;
	CMap<uint32, uint32, CDeletedClient*, CDeletedClient*> m_trackedClientsList;
	uint32	m_dwLastBannCleanUp;
	uint32	m_dwLastTrackedCleanUp;
	uint32 m_dwLastClientCleanUp;
	CUpDownClient* m_pBuddy;
	uint8 m_nBuddyStatus;
	CList<IPANDTICS> listFirewallCheckRequests;
	CList<IPANDTICS> listDirectCallbackRequests;
	CList<CONNECTINGCLIENT> m_liConnectingClients;

//EastShare Start - added by AndCycle, IP to Country
public:
	void ResetIP2Country();
//EastShare End - added by AndCycle, IP to Country

//Xman extended stats
public:
	CString GetMODType(POSITION pos_in)
	{
		if (pos_in != 0)
			return liMODsTypes.GetAt(pos_in);
		else
			return _T("");
	}

protected:
	CList<CString, CString&> liMODsTypes;
//Xman end

	// ==> Compat Client Stats [Stulle] - Stulle
public:
	void	GetCompatClientsStats(CRBMap<CString, uint32> *compatClients);
	// <== Compat Client Stats [Stulle] - Stulle

	void RecalculateReAskTimes(); // Timer for ReAsk File Sources [Stulle] - Stulle

	void BanReducedClients(bool bCommunity, bool bThief); // Ban clients with reduced score immediatly on setting changed [Stulle] - Stulle
};
