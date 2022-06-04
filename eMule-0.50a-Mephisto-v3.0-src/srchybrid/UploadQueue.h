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

#include "updownclient.h" //Xman

class CUpDownClient;
typedef CTypedPtrList<CPtrList, CUpDownClient*> CUpDownClientPtrList;

class CUploadQueue
{

public:
	CUploadQueue();
	~CUploadQueue();

	void	Process();
	void	AddClientToQueue(CUpDownClient* client,bool bIgnoreTimelimit = false);
	void	AddClientDirectToQueue(CUpDownClient* client);	//Xman uploading problem client
	// Maella -Upload Stop Reason-
	/*
	bool	RemoveFromUploadQueue(CUpDownClient* client, LPCTSTR pszReason = NULL, bool updatewindow = true, bool earlyabort = false);
	*/
	bool	RemoveFromUploadQueue(CUpDownClient* client, LPCTSTR pszReason = NULL,CUpDownClient::UpStopReason reason = CUpDownClient::USR_NONE, bool updatewindow = true, bool earlyabort = false);
	//Xman end
	bool	RemoveFromWaitingQueue(CUpDownClient* client,bool updatewindow = true);
	bool	IsOnUploadQueue(CUpDownClient* client)	const {return (waitinglist.Find(client) != 0);}
	bool	IsDownloading(CUpDownClient* client)	const {return (uploadinglist.Find(client) != 0);}

	//Xman 
	/*
    void    UpdateDatarates();
	uint32	GetDatarate();
    uint32  GetToNetworkDatarate();
	*/
	//Xman end

	bool	CheckForTimeOver(CUpDownClient* client);

	//Xman Xtreme Upload
	/*
	int		GetWaitingUserCount() const				{return waitinglist.GetCount();}
	int		GetUploadQueueLength() const			{return uploadinglist.GetCount();}
	uint32	GetActiveUploadsCount()	const			{return m_MaxActiveClientsShortTime;}
	*/
	int		GetWaitingUserCount()					{return waitinglist.GetCount();}
	int		GetUploadQueueLength()					{return uploadinglist.GetCount();}
	void	ReplaceSlot(CUpDownClient* client);	//altenative method to Resortuploadslots ////Xman Xtreme Upload: Peercache-part
	void	ChangeSendBufferSize(int newvalue);
	//Xman end

	uint32	GetWaitingUserForFileCount(const CSimpleArray<CObject*>& raFiles, bool bOnlyIfChanged);
	uint32	GetDatarateForFile(const CSimpleArray<CObject*>& raFiles) const;

	//Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	void	CompUploadRate();
	//Xman end
	
	POSITION GetFirstFromUploadList()				{return uploadinglist.GetHeadPosition();}
	CUpDownClient* GetNextFromUploadList(POSITION &curpos)	{return uploadinglist.GetNext(curpos);}
	CUpDownClient* GetQueueClientAt(POSITION &curpos)	{return uploadinglist.GetAt(curpos);}

	POSITION GetFirstFromWaitingList()				{return waitinglist.GetHeadPosition();}
	CUpDownClient* GetNextFromWaitingList(POSITION &curpos)	{return waitinglist.GetNext(curpos);}
	CUpDownClient* GetWaitClientAt(POSITION &curpos)	{return waitinglist.GetAt(curpos);}

	CUpDownClient*	GetWaitingClientByIP_UDP(uint32 dwIP, uint16 nUDPPort, bool bIgnorePortOnUniqueIP, bool* pbMultipleIPs = NULL);
	CUpDownClient*	GetWaitingClientByIP(uint32 dwIP);
	CUpDownClient*	GetNextClient(const CUpDownClient* update);

	
	void	DeleteAll();
	UINT	GetWaitingPosition(CUpDownClient* client);
	
	uint32	GetSuccessfullUpCount()					{return successfullupcount;}
	uint32	GetFailedUpCount()						{return failedupcount;}
	uint32	GetAverageUpTime();

	// ==> Keep Sup clients in up if there is no other sup client in queue [Stulle] - Stulle
	/*
    CUpDownClient* FindBestClientInQueue();
	*/
    CUpDownClient* FindBestClientInQueue(bool bCheckOnly = false);
	// <== Keep Sup clients in up if there is no other sup client in queue [Stulle] - Stulle
    void ReSortUploadSlots(bool force = false);

	CUpDownClientPtrList waitinglist;
	CUpDownClientPtrList uploadinglist;

	//Xman Xtreme Upload
	uint16	currentuploadlistsize;
	bool		AcceptNewClient(bool addOnNextConnect = false); //Xman 4.8.2 must be punlic because of access in ClientUDPSocket

	void	UploadTimer(); //Xman process timer code via messages (Xanatos)
	// ==> Mephisto Upload - Mephisto
	/*
	bool	UseHighSpeedUpload()					{return m_bUseHighSpeedUpload;} //Xman for SiRoB: ReadBlockFromFileThread
	bool		AddUpNextClient(LPCTSTR pszReason, CUpDownClient* directadd = 0);
	*/
	bool		AddUpNextClient(LPCTSTR pszReason, CUpDownClient* directadd = 0, bool bByClosing = false);
	// <== Mephisto Upload - Mephisto

protected:
	void	RemoveFromWaitingQueue(POSITION pos, bool updatewindow);
	//Xman Xtreme Upload
	/*
	bool		AcceptNewClient(bool addOnNextConnect = false);
	bool		AcceptNewClient(uint32 curUploadSlots);
	*/
	//Xman end
	bool		ForceNewClient(bool allowEmptyWaitingQueue = false);

	//Xman Xtreme Upload
	/*
	bool		AddUpNextClient(LPCTSTR pszReason, CUpDownClient* directadd = 0);
	*/
	//Xman end
	//Xman for SiRoB: ReadBlockFromFileThread
	/*
	void		UseHighSpeedUploadTimer(bool bEnable);
	*/
	//Xman end
	
	static VOID CALLBACK UploadTimer(HWND hWnd, UINT nMsg, UINT nId, DWORD dwTime);
	//Xman for SiRoB: ReadBlockFromFileThread
	/*
	static VOID CALLBACK HSUploadTimer(HWND hWnd, UINT nMsg, UINT nId, DWORD dwTime);
	*/
	//Xman end

private:
	void	UpdateMaxClientScore();
	uint32	GetMaxClientScore()						{return m_imaxscore;}

	//Xman Xtreme Upload
	/*
    void    UpdateActiveClientsInfo(DWORD curTick);
	*/
	bool	lastupslotHighID;
	// ==> Mephisto Upload - Mephisto
	/*
	uint8	waituntilnextlook;
	sint8	dataratestocheck;
	*/
	// <== Mephisto Upload - Mephisto
	CList<uint32> m_blockstoplist; //Xman 4.4 this list should avoid too many uploaddrops because user set wrong uploadlimit
	bool	checkforuploadblock; //Xman 4.4 enable the check-feature.
	uint32	m_dwnextallowedscoreremove; //additionally check to avoid too short upload by score

	// ==> Superior Client Handling [Stulle] - Stulle
	/*
	//Xman always one release-slot
	CUpDownClient* releaseslotclient;
	//Xman end
	*/
	// <== Superior Client Handling [Stulle] - Stulle

	// ==> Mephisto Upload - Mephisto
	/*
    void InsertInUploadingList(CUpDownClient* newclient);
	*/
    void InsertInUploadingList(CUpDownClient* newclient, bool bBottom = false);
	// <== Mephisto Upload - Mephisto
    float GetAverageCombinedFilePrioAndCredit();

	//Xman
	/*
	// By BadWolf - Accurate Speed Measurement
	typedef struct TransferredData {
		uint32	datalen;
		DWORD	timestamp;
	};
	CList<uint64> avarage_dr_list;
    CList<uint64> avarage_friend_dr_list;
	CList<DWORD,DWORD> avarage_tick_list;
	CList<int,int> activeClients_list;
    CList<DWORD,DWORD> activeClients_tick_list;
	uint32	datarate;   //datarate sent to network (including friends)
    uint32  friendDatarate; // datarate of sent to friends (included in above total)
	// By BadWolf - Accurate Speed Measurement
	*/
	//Xman end
	UINT_PTR h_timer;
	//Xman for SiRoB: ReadBlockFromFileThread
	/*
	UINT_PTR m_hHighSpeedUploadTimer;
	*/
	// ==> Mephisto Upload - Mephisto
	/*
	bool	m_bUseHighSpeedUpload;
	*/
	// <== Mephisto Upload - Mephisto
	//Xman end
	uint32	successfullupcount;
	uint32	failedupcount;
	uint32	totaluploadtime;
	// ==> Mephisto Upload - Mephisto
	/*
	uint32	m_nLastStartUpload;
	*/
	DWORD	m_dwNextUploadTime;
	// <== Mephisto Upload - Mephisto
	uint32	m_dwRemovedClientByScore;

	uint32	m_imaxscore;

    DWORD   m_dwLastCalculatedAverageCombinedFilePrioAndCredit;
    float   m_fAverageCombinedFilePrioAndCredit;
	//Xman
	/*
    uint32  m_iHighestNumberOfFullyActivatedSlotsSinceLastCall;
    uint32  m_MaxActiveClients;
    uint32  m_MaxActiveClientsShortTime;

    DWORD   m_lastCalculatedDataRateTick;
    uint64  m_avarage_dr_sum;
	*/
	//Xman end

    DWORD   m_dwLastResortedUploadSlots;
	bool	m_bStatisticsWaitingListDirty;

	// ==> Spread Credits Slot [Stulle] - Stulle
public:
	int		m_slotcounter;
	bool	m_bSpreadCreditsSlotActive;
	CUpDownClient* FindBestSpreadClientInQueue();
	// <== Spread Credits Slot [Stulle] - Stulle

	// ==> Mephisto Upload - Mephisto
	void	MoveDownInUpload(CUpDownClient* client, bool bBottom = false);
	void	ResortSlotNumbers();
protected:
	uint32 m_dwNextBlockingCheck;
	// <== Mephisto Upload - Mephisto
};
