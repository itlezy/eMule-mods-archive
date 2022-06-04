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

class CUpDownClient;
typedef CTypedPtrList<CPtrList, CUpDownClient*> CUpDownClientPtrList;

class CUploadQueue
{

public:
	CUploadQueue();
	~CUploadQueue();

	void	Process();
//ZZUL +
	void	AddClientToQueue(CUpDownClient* client,bool bIgnoreTimelimit = false, bool addInFirstPlace = false);
	void	ScheduleRemovalFromUploadQueue(CUpDownClient* client, LPCTSTR pszDebugReason, CString strDisplayReason, bool earlyabort = false);
//ZZUL -
	bool	RemoveFromUploadQueue(CUpDownClient* client, LPCTSTR pszReason = NULL, bool updatewindow = true, bool earlyabort = false);
	bool	RemoveFromWaitingQueue(CUpDownClient* client,bool updatewindow = true);
	bool	IsOnUploadQueue(CUpDownClient* client)	const {return (waitinglist.Find(client) != 0);}
	bool	IsDownloading(CUpDownClient* client)	const {return (uploadinglist.Find(client) != 0);}

    void    UpdateDatarates();
	uint32	GetDatarate();
//ZZUL +
 	uint32	GetDatarateExcludingPowershare(); //powershare graph
//ZZUL -
    uint32  GetToNetworkDatarate();

	int		GetWaitingUserCount() const				{return waitinglist.GetCount();}
	int		GetUploadQueueLength() const			{return uploadinglist.GetCount();}
	uint32	GetActiveUploadsCount()	const			{return m_MaxActiveClientsShortTime;}
	uint32	GetWaitingUserForFileCount(const CSimpleArray<CObject*>& raFiles, bool bOnlyIfChanged);
	uint32	GetDatarateForFile(const CSimpleArray<CObject*>& raFiles) const;
//ZZUL +
		uint32	GetActiveUploadsCountLongPerspective()					{return m_MaxActiveClients;}
    uint32 GetEffectiveUploadListCount();
//ZZUL -	
	POSITION GetFirstFromUploadList()				{return uploadinglist.GetHeadPosition();}
	CUpDownClient* GetNextFromUploadList(POSITION &curpos)	{return uploadinglist.GetNext(curpos);}
	CUpDownClient* GetQueueClientAt(POSITION &curpos)	{return uploadinglist.GetAt(curpos);}

	POSITION GetFirstFromWaitingList()				{return waitinglist.GetHeadPosition();}
	CUpDownClient* GetNextFromWaitingList(POSITION &curpos)	{return waitinglist.GetNext(curpos);}
	CUpDownClient* GetWaitClientAt(POSITION &curpos)	{return waitinglist.GetAt(curpos);}
//ZZUL +
	void	UpdateBanCount();
//ZZUL -
	CUpDownClient*	GetWaitingClientByIP_UDP(uint32 dwIP, uint16 nUDPPort, bool bIgnorePortOnUniqueIP, bool* pbMultipleIPs = NULL);
	CUpDownClient*	GetWaitingClientByIP(uint32 dwIP);
	CUpDownClient*	GetNextClient(const CUpDownClient* update);

	
	void	DeleteAll();
	UINT	GetWaitingPosition(CUpDownClient* client);
	
	uint32	GetSuccessfullUpCount()					{return successfullupcount;}
	uint32	GetFailedUpCount()						{return failedupcount;}
	uint32	GetAverageUpTime();
//ZZUL +
    bool    RemoveOrMoveDown(CUpDownClient* client, bool onlyCheckForRemove = false);
	void	MoveDownInUploadQueue(CUpDownClient* client);
	CUpDownClient* FindBestClientInQueue(bool allowLowIdAddNextConnectToBeSet = false, CUpDownClient* lowIdClientMustBeInSameOrBetterClassAsThisClient = NULL);
    bool RightClientIsBetter(CUpDownClient* leftClient, uint32 leftScore, CUpDownClient* rightClient, uint32 rightScore);
//ZZUL -
   void ReSortUploadSlots(bool force = false);

	CUpDownClientPtrList waitinglist;
	CUpDownClientPtrList uploadinglist;

protected:
	void		RemoveFromWaitingQueue(POSITION pos, bool updatewindow);
//ZZUL +
	bool		AcceptNewClient();
	bool		AcceptNewClient(uint32 curUploadSlots);
	bool		ForceNewClient(bool simulateScheduledClosingOfSlot = false);
    bool        CanForceClient(uint32 curUploadSlots);
	bool		AddUpNextClient(LPCTSTR pszReason, CUpDownClient* directadd = 0, bool highPrioCheck = false);
//ZZUL -

#ifdef HIGHRES
	void		UseHighSpeedUploadTimer(bool bEnable);
#endif	
	static VOID CALLBACK UploadTimer(HWND hWnd, UINT nMsg, UINT nId, DWORD dwTime);
#ifdef HIGHRES
	static VOID CALLBACK HSUploadTimer(HWND hWnd, UINT nMsg, UINT nId, DWORD dwTime);
#endif

private:
	void	UpdateMaxClientScore();
	uint32	GetMaxClientScore()						{return m_imaxscore;}
    void    UpdateActiveClientsInfo(DWORD curTick);

    void InsertInUploadingList(CUpDownClient* newclient);
    float GetAverageCombinedFilePrioAndCredit();
 //ZZUL +
   uint32 GetWantedNumberOfTrickleUploads();
    void CheckForHighPrioClient();

    CUpDownClient* FindLastUnScheduledForRemovalClientInUploadList();
    CUpDownClient* FindBestScheduledForRemovalClientInUploadListThatCanBeReinstated();
//ZZUL -

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
//powershare graph +
	uint32	powershareDatarate;  
 	CList<uint64> avarage_powershare_dr_list; 
//powershare graph -
	// By BadWolf - Accurate Speed Measurement

	UINT_PTR h_timer;
#ifdef HIGHRES
	UINT_PTR m_hHighSpeedUploadTimer;
#endif
	uint32	successfullupcount;
	uint32	failedupcount;
	uint32	totaluploadtime;
	uint32	m_nLastStartUpload;
	uint32	m_dwRemovedClientByScore;

	uint32	m_imaxscore;

    DWORD   m_dwLastCalculatedAverageCombinedFilePrioAndCredit;
    float   m_fAverageCombinedFilePrioAndCredit;
    uint32  m_iHighestNumberOfFullyActivatedSlotsSinceLastCall;
    uint32  m_MaxActiveClients;
    uint32  m_MaxActiveClientsShortTime;

    DWORD   m_lastCalculatedDataRateTick;
    uint64  m_avarage_dr_sum;

    DWORD   m_dwLastResortedUploadSlots;
	bool	m_bStatisticsWaitingListDirty;
//ZZUL +
    DWORD   m_dwLastCheckedForHighPrioClient;
//ZZUL -

// ZZUL-TRA :: BlockRatio :: Start
	uint32	m_dwNextBlockingCheck;
	// ZZUL-TRA :: BlockRatio :: End
};
