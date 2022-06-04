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
#include "UploadBandwidthThrottler.h" //Morph

class CUpDownClient;
typedef CTypedPtrList<CPtrList, CUpDownClient*> CUpDownClientPtrList;

class CUploadQueue
{

public:
	CUploadQueue();
	~CUploadQueue();

	void	Process();
	void	AddClientToQueue(CUpDownClient* client,bool bIgnoreTimelimit = false, bool addInFirstPlace = false);
	/*zz*/void	ScheduleRemovalFromUploadQueue(CUpDownClient* client, LPCTSTR pszDebugReason, CString strDisplayReason, bool earlyabort = false);
	bool	RemoveFromUploadQueue(CUpDownClient* client, LPCTSTR pszReason = NULL, bool updatewindow = true, bool earlyabort = false);
	bool	RemoveFromWaitingQueue(CUpDownClient* client,bool updatewindow = true);
	bool	IsOnUploadQueue(CUpDownClient* client)	const {return (waitinglist.Find(client) != 0);}
	bool	IsDownloading(CUpDownClient* client)	const {return (uploadinglist.Find(client) != 0);}

    void    UpdateDatarates();

	//MORPH - Changed by SiRoB, Keep An average datarate value for USS system
	/*
	uint32	GetDatarate();
	*/
	uint32	GetDatarate(bool breel = false);
	uint32	GetDatarateOverHead(); //MORPH - Added by SiRoB, Upload OverHead from uploadbandwidththrottler
	uint32	GetDatarateExcludingPowershare(); //MORPH - Added by SiRoB, Upload powershare from uploadbandwidththrottler
	uint32  GetToNetworkDatarate();

	//MORPH START - Added & Modified by SiRoB, Smart Upload Control v2 (SUC) [lovelace]
	uint32	GetAvgRespondTime(uint8 index)	{return AvgRespondTime[index]>10 ? AvgRespondTime[index] : 1500;}
	void	SetAvgRespondTime(uint8 index,uint32 in_AvgRespondTime)	{AvgRespondTime[index]=in_AvgRespondTime;}
	//uint32	GetMaxVUR()	{return MaxVUR;}//[lovelace]
	uint32	GetMaxVUR();
	//void	SetMaxVUR(uint32 in_MaxVUR, uint32 min, uint32 max){MaxVUR=((in_MaxVUR>max)?max:((in_MaxVUR<min)?min:in_MaxVUR));}//[lovelace]
	void	SetMaxVUR(uint32 in_MaxVUR){MaxVUR=in_MaxVUR;}
	//MORPH END   - Added & Modified by SiRoB, Smart Upload Control v2 (SUC) [lovelace]

	int		GetWaitingUserCount() const				{return waitinglist.GetCount();}
	int		GetUploadQueueLength() const			{return uploadinglist.GetCount();}
	//MORPH START - Upload Splitting Class
	uint32	GetNumberOfSlotInAboveClass(uint32 classID) {uint32 retvalue = 0; for (uint32 i = 0; i < classID; i++) retvalue+=m_aiSlotCounter[i]; return retvalue;}
	uint32	GetActiveUploadsCount(uint32 classID = LAST_CLASS)					{return GetNumberOfSlotInAboveClass(classID)+m_MaxActiveClientsShortTimeClass[classID];}
	uint32	GetActiveUploadsCountLongPerspective(uint32 classID = LAST_CLASS)					{return GetNumberOfSlotInAboveClass(classID)+m_MaxActiveClientsClass[classID];}
    /*zz*/uint32 GetEffectiveUploadListCount(uint32 classID = LAST_CLASS);
	//MORPH END  - Upload Splitting Class
	uint32	GetWaitingUserForFileCount(const CSimpleArray<CObject*>& raFiles, bool bOnlyIfChanged);
	uint32	GetDatarateForFile(const CSimpleArray<CObject*>& raFiles) const;
	
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

	bool    RemoveOrMoveDown(CUpDownClient* client, bool onlyCheckForRemove = false);
	void	MoveDownInUploadQueue(CUpDownClient* client);
	//MORPH START - Changed by SiRoB, Upload Splitting Class
	CUpDownClient* FindBestClientInQueue(bool allowLowIdAddNextConnectToBeSet = false, CUpDownClient* lowIdClientMustBeInSameOrBetterClassAsThisClient = NULL, bool checkforaddinuploadinglist = false);
	bool	RightClientIsBetter(CUpDownClient* leftClient, uint32 leftScore, CUpDownClient* rightClient, uint32 rightScore, bool checkforaddinuploadinglist = false);
	//MORPH END   - Changed by SiRoB, Upload Splitting Class
	void	ReSortUploadSlots(bool force = false);

	CUpDownClientPtrList	waitinglist;
	CUpDownClientPtrList	uploadinglist;

	//Morph - added by AndCycle, separate special prio compare
	int	RightClientIsSuperior(CUpDownClient* leftClient, CUpDownClient* rightClient);
	
protected:
	void	RemoveFromWaitingQueue(POSITION pos, bool updatewindow);
	//MORPH START - Upload Splitting Class
	bool		AcceptNewClient(uint32 classID);
	//==MagicAngel=> Fix Completing Bug - Stulle idea :) - evcz
	//bool		AcceptNewClient(uint32 curUploadSlots, uint32 classID);
	bool		AcceptNewClient(uint32 curUploadSlots, uint32 classID, bool bForceExtra=false);
	//bool		ForceNewClient(bool simulateScheduledClosingOfSlot, uint32 classID);
	bool		ForceNewClient(bool simulateScheduledClosingOfSlot, uint32 classID, bool bForceExtra=false);
	//<=MagicAngel== Fix Completing Bug - Stulle idea :) - evcz
	//MORPH END   - Upload Splitting Class

	bool		AddUpNextClient(LPCTSTR pszReason, CUpDownClient* directadd = 0, bool highPrioCheck = false);
	
	static VOID CALLBACK UploadTimer(HWND hWnd, UINT nMsg, UINT nId, DWORD dwTime);

private:
	void	UpdateMaxClientScore();
	uint32	GetMaxClientScore()						{return m_imaxscore;}
	void    UpdateActiveClientsInfo(DWORD curTick);

    void InsertInUploadingList(CUpDownClient* newclient);
    double GetAverageCombinedFilePrioAndCredit();
	/*zz*/uint32 GetWantedNumberOfTrickleUploads(uint32 classID); //MORPH - Upload Splitting Class
	void CheckForHighPrioClient();

	//MORPH START - Added By AndCycle, ZZUL_20050212-0200
    //MORPH START - Changed by SiRoB, Upload Splitting Class
	/*
	CUpDownClient* FindLastUnScheduledForRemovalClientInUploadList();
	CUpDownClient* FindBestScheduledForRemovalClientInUploadListThatCanBeReinstated(bool checkforaddinuploadinglist);
	*/
	CUpDownClient* FindLastUnScheduledForRemovalClientInUploadList(uint32 classID);
	CUpDownClient* FindBestScheduledForRemovalClientInUploadListThatCanBeReinstated(bool checkforaddinuploadinglist);
	//MORPH END   - Changed by SiRoB, Upload Splitting Class
	//MORPH END   - Added By AndCycle, ZZUL_20050212-0200

	// By BadWolf - Accurate Speed Measurement
	typedef struct TransferredData {
		uint64	datalen;
		DWORD	timestamp;
	};
	CList<uint64> avarage_dr_list;
	CList<uint64> avarage_overhead_dr_list;    //MORPH - Added by SiRoB, Upload OverHead from uploadbandwidththrottler
	CList<TransferredData> avarage_dr_USS_list; //MORPH - Added by SiRoB, Keep An average datarate value for USS system
	CList<uint64> avarage_friend_dr_list;    //MORPH - Added by SiRoB, Upload Friend from uploadbandwidththrottler
	CList<uint64> avarage_powershare_dr_list;    //MORPH - Added by SiRoB, Upload Powershare from uploadbandwidththrottler
	CList<DWORD,DWORD> avarage_tick_list;
	DWORD	avarage_tick_listLastRemovedTimestamp; //MORPH - Added by SiRoB, Better datarate mesurement for low and high speed
	DWORD	avarage_dr_USS_listLastRemovedTimestamp;  //MORPH - Added by SiRoB, Keep An average datarate value for USS system
	CList<int,int> activeClients_listClass[NB_SPLITTING_CLASS]; //MORPH - Upload Splitting Class
	CList<DWORD,DWORD> activeClients_tick_listClass[NB_SPLITTING_CLASS];
	uint32	datarate;   //datarate sent to network (including friends)
	uint32	datarateoverhead;   //MORPH - Added by SiRoB, Upload OverHead from uploadbandwidththrottler
	uint32	datarate_USS; //MORPH - Added by SiRoB, Keep An average datarate value for USS system
	uint32  friendDatarate; // datarate of sent to friends (included in above total)
	uint32	powershareDatarate;   //MORPH - Added by SiRoB, Upload OverHead from uploadbandwidththrottler
	// By BadWolf - Accurate Speed Measurement

	UINT_PTR h_timer;
	uint32	successfullupcount;
	uint32	failedupcount;
	uint32	totaluploadtime;
	uint32	m_nLastStartUpload;
	uint32	m_dwRemovedClientByScore;

	uint32	m_imaxscore;

    DWORD   m_dwLastCalculatedAverageCombinedFilePrioAndCredit;
    float   m_fAverageCombinedFilePrioAndCredit;
	//MORPH START - Upload Splitting Class
	uint32  m_iHighestNumberOfFullyActivatedSlotsSinceLastCallClass[NB_SPLITTING_CLASS];
    uint32  m_MaxActiveClientsClass[NB_SPLITTING_CLASS];
	uint32  m_MaxActiveClientsShortTimeClass[NB_SPLITTING_CLASS];
	bool	m_abAddClientOfThisClass[NB_SPLITTING_CLASS];
	uint32	m_aiSlotCounter[NB_SPLITTING_CLASS];
	//MORPH END   - Upload Splitting Class
	DWORD   m_lastCalculatedDataRateTick;
    uint64  m_avarage_dr_sum;
	uint64  m_avarage_overhead_dr_sum; //MORPH - Added by SiRoB, Upload OverHead from uploadbandwidththrottler
	uint64  m_avarage_friend_dr_sum; //MORPH - Added by SiRoB, Upload Friend from uploadbandwidththrottler
	uint64  m_avarage_powershare_dr_sum; //MORPH - Added by SiRoB, Upload Powershare from uploadbandwidththrottler
    uint64  m_avarage_dr_USS_sum; //MORPH - Added by SiRoB, Keep An average datarate value for USS system
	DWORD   m_lastproccesstick;	 //MORPH -- lh use same tick to check al slots. 
    DWORD   m_dwLastResortedUploadSlots;
	bool	m_bStatisticsWaitingListDirty;

	DWORD   m_dwLastCheckedForHighPrioClient;

	//MORPH START - Added by SiRoB, Smart Upload Control v2 (SUC) [lovelace]
	uint32	AvgRespondTime[2];
	uint32	MaxVUR;
	//MORPH END - Added by SiRoB, Smart Upload Control v2 (SUC) [lovelace]
};
