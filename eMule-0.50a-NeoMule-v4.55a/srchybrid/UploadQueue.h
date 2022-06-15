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
	friend class CClientUDPSocket; // NEO: MOD - [NewUploadState] <-- Xanatos --
	friend class CClientReqSocket; // NEO: MOD - [NewUploadState] <-- Xanatos --

public:
	CUploadQueue();
	~CUploadQueue();

	void	Process();
	void	CalculateUploadRate(); // NEO: ASM - [AccurateSpeedMeasure] <-- Xanatos --
	void	AddClientToQueue(CUpDownClient* client,bool bIgnoreTimelimit = false);
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	void	AddLanClient(CUpDownClient* client);
#endif //LANCAST // NEO: NLC END <-- Xanatos --
	bool	RemoveFromUploadQueue(CUpDownClient* client, LPCTSTR pszReason = NULL, bool updatewindow = true, bool earlyabort = false);
	bool	RemoveFromWaitingQueue(CUpDownClient* client,bool updatewindow = true);
	bool	IsOnUploadQueue(CUpDownClient* client)	const {return (waitinglist.Find(client) != 0);}
	bool	IsDownloading(CUpDownClient* client)	const {return (uploadinglist.Find(client) != 0);}

    //void    UpdateDatarates(); // NEO: ASM - [AccurateSpeedMeasure] <-- Xanatos --
	uint32	GetDatarate();
    uint32  GetToNetworkDatarate();

	bool	CheckForTimeOver(CUpDownClient* client);
	int		GetWaitingUserCount() const				{return waitinglist.GetCount();}
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	int		GetLanUploadsCount()					{return lanSlots;}
	int		GetUploadQueueLength()					{return uploadinglist.GetCount() - lanSlots;}
#else
	int		GetUploadQueueLength() const			{return uploadinglist.GetCount();}
#endif //LANCAST // NEO: NLC END <-- Xanatos --
#ifdef NEO_UBT // NEO: NUSM - [NeoUploadSlotManagement] -- Xanatos -->
	int		GetActiveUploadsCount()					{return activeSlots;}
 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
	uint32	GetReservedDatarate()					{return reservedDatarate;}
	uint32	GetReservedSlots()						{return reservedSlots;}
 #endif // BW_MOD // NEO: BM END
#else
	uint32	GetActiveUploadsCount()	const			{return m_MaxActiveClientsShortTime;}
#endif // NEO_UBT // NEO: NUSM END <-- Xanatos --

	int		GetReleaseSlots()						{return releaseSlots;} // NEO: SRS - [SmartReleaseSharing] <-- Xanatos --
	int		GetFriendSlots()						{return friendSlots;} // NEO: NMFS - [NiceMultiFriendSlots] <-- Xanatos --
	
	POSITION GetFirstFromUploadList()				{return uploadinglist.GetHeadPosition();}
	CUpDownClient* GetNextFromUploadList(POSITION &curpos)	{return uploadinglist.GetNext(curpos);}
	CUpDownClient* GetQueueClientAt(POSITION &curpos)	{return uploadinglist.GetAt(curpos);}

	POSITION GetFirstFromWaitingList()				{return waitinglist.GetHeadPosition();}
	CUpDownClient* GetNextFromWaitingList(POSITION &curpos)	{return waitinglist.GetNext(curpos);}
	CUpDownClient* GetWaitClientAt(POSITION &curpos)	{return waitinglist.GetAt(curpos);}

	CUpDownClient*	GetWaitingClientByIP_UDP(uint32 dwIP, uint16 nUDPPort, bool bIgnorePortOnUniqueIP, bool* pbMultipleIPs = NULL);
	CUpDownClient*	GetWaitingClientByIP(uint32 dwIP);
	CUpDownClient*	GetNextClient(const CUpDownClient* update);

	void			AddClientDirectToQueue(CUpDownClient* client); // NEO: UPC - [UploadingProblemClient] <-- Xanatos --

	
	void	DeleteAll();
	UINT	GetWaitingPosition(CUpDownClient* client);
	
	uint32	GetSuccessfullUpCount()					{return successfullupcount;}
	uint32	GetFailedUpCount()						{return failedupcount;}
	uint32	GetAverageUpTime();

    CUpDownClient* FindBestClientInQueue();
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
	void ReSortUploadSlots(CUpDownClient* client);
#else
    void ReSortUploadSlots(bool force = false);
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --

#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	void GetTransferTipInfo(CString &info);
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

	CUpDownClientPtrList waitinglist;
	CUpDownClientPtrList uploadinglist;

protected:
	void	RemoveFromWaitingQueue(POSITION pos, bool updatewindow);
	bool		AcceptNewClient(bool addOnNextConnect = false);
#ifndef NEO_UBT // NEO: NUBT -- Xanatos -->
	bool		AcceptNewClient(uint32 curUploadSlots);
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
	bool		ForceNewClient(bool allowEmptyWaitingQueue = false);

	bool		AddUpNextClient(LPCTSTR pszReason, CUpDownClient* directadd = 0);
	
	static VOID CALLBACK UploadTimer(HWND hWnd, UINT nMsg, UINT nId, DWORD dwTime);
	// NEO: ND - [NeoDebug] -- Xanatos -->
	friend class CemuleDlg; 
	static void			 UploadTimer();
	// NEO: ND END <-- Xanatos --

private:
	void	UpdateMaxClientScore();
	uint32	GetMaxClientScore()						{return m_imaxscore;}
    void    UpdateActiveClientsInfo(DWORD curTick);

    void InsertInUploadingList(CUpDownClient* newclient);
    float GetAverageCombinedFilePrioAndCredit();


	// By BadWolf - Accurate Speed Measurement
	typedef struct TransferredData {
		uint32	datalen;
		DWORD	timestamp;
	};
	// NEO: ASM - [AccurateSpeedMeasure] -- Xanatos -->
	CList<TransferredData> avarage_dr_list;
	uint64	m_datarateMS;
	// NEO: ASM END <-- Xanatos --
	//CList<uint64> avarage_dr_list; 
    //CList<uint64> avarage_friend_dr_list;
	//CList<DWORD,DWORD> avarage_tick_list;
	//CList<int,int> activeClients_list;
    //CList<DWORD,DWORD> activeClients_tick_list;
	uint32	datarate;   //datarate sent to network (including friends)
    //uint32  friendDatarate; // datarate of sent to friends (included in above total)
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
#if !defined(NEO_UBT) // NEO: NUBT - [NeoUploadBandwidthThrottler] <-- Xanatos --
    uint32  m_iHighestNumberOfFullyActivatedSlotsSinceLastCall;
    uint32  m_MaxActiveClients;
    uint32  m_MaxActiveClientsShortTime;
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --

#ifdef NEO_UBT // NEO: NUSM - [NeoUploadSlotManagement] -- Xanatos -->
	uint32	lastUploadSlotCheck;
	UINT	activeSlots;
	int		waituntilnextlook;
	int		dataratestocheck;
 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
	uint32	reservedDatarate;
	uint32	reservedSlots;
 #endif // BW_MOD // NEO: BM END
#endif // NEO_UBT // NEO: NUSM END <-- Xanatos --
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	uint16	lanSlots;
#endif //LANCAST // NEO: NLC END <-- Xanatos --

	// NEO: ASM - [AccurateSpeedMeasure] -- Xanatos --
    //DWORD   m_lastCalculatedDataRateTick;
    //uint64  m_avarage_dr_sum;

#if !defined(NEO_UBT) // NEO: NUBT - [NeoUploadBandwidthThrottler] <-- Xanatos --
    DWORD   m_dwLastResortedUploadSlots;
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --

	uint16	releaseSlots; // NEO: SRS - [SmartReleaseSharing] <-- Xanatos --
	uint16	friendSlots; // NEO: NMFS - [NiceMultiFriendSlots] <-- Xanatos --

	uint32	m_uLastKadFirewallRecheck; // NEO: RKF - [RecheckKadFirewalled]
};
