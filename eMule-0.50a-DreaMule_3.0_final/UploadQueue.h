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
	bool	RemoveFromUploadQueue(CUpDownClient* client, LPCTSTR pszReason = NULL,CUpDownClient::UpStopReason reason = CUpDownClient::USR_NONE, bool updatewindow = true, bool earlyabort = false); // Maella -Upload Stop Reason-
	bool	RemoveFromWaitingQueue(CUpDownClient* client,bool updatewindow = true);
	bool	IsOnUploadQueue(CUpDownClient* client)	const {return (waitinglist.Find(client) != 0);}
	bool	IsDownloading(CUpDownClient* client)	const {return (uploadinglist.Find(client) != 0);}


	bool	CheckForTimeOver(CUpDownClient* client);
	int		GetWaitingUserCount()					{return waitinglist.GetCount();}
	int		GetUploadQueueLength()					{return uploadinglist.GetCount();}
	//Xman Xtreme Upload
	//uint32	GetActiveUploadsCount()					{return m_MaxActiveClientsShortTime;}
	void	ReplaceSlot(CUpDownClient* client);	//altenative method to Resortuploadslots ////Xman Xtreme Upload: Peercache-part
	void	ChangeSendBufferSize(int newvalue);

	//Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	void	CompUploadRate();


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

    CUpDownClient* FindBestClientInQueue();
    void ReSortUploadSlots(bool force = false);

	CUpDownClientPtrList waitinglist;
	CUpDownClientPtrList uploadinglist;

	//Xman Xtreme Upload
	uint16	currentuploadlistsize;
	bool		AcceptNewClient(bool addOnNextConnect = false); //Xman 4.8.2 must be punlic because of access in ClientUDPSocket

	void	UploadTimer(); //Xman process timer code via messages (Xanatos)
	bool		AddUpNextClient(LPCTSTR pszReason, CUpDownClient* directadd = 0);

protected:
	void	RemoveFromWaitingQueue(POSITION pos, bool updatewindow);
	//bool		AcceptNewClient(uint32 curUploadSlots); //Xman Xtreme Upload
	bool		ForceNewClient(bool allowEmptyWaitingQueue = false);


	
	static VOID CALLBACK UploadTimer(HWND hWnd, UINT nMsg, UINT nId, DWORD dwTime);

private:
	void	UpdateMaxClientScore();
	uint32	GetMaxClientScore()						{return m_imaxscore;}
    
	//Xman Xtreme Upload
	bool	lastupslotHighID;
	uint8	waituntilnextlook;
	sint8	dataratestocheck;
	CList<uint32> m_blockstoplist; //Xman 4.4 this list should avoid too many uploaddrops because user set wrong uploadlimit
	bool	checkforuploadblock; //Xman 4.4 enable the check-feature.
	uint32	m_dwnextallowedscoreremove; //additionally check to avoid too short upload by score

	//Xman always one release-slot
	CUpDownClient* releaseslotclient;
	//Xman end

    void InsertInUploadingList(CUpDownClient* newclient);
    float GetAverageCombinedFilePrioAndCredit();


	UINT_PTR h_timer;
	uint32	successfullupcount;
	uint32	failedupcount;
	uint32	totaluploadtime;
	uint32	m_nLastStartUpload;
	uint32	m_dwRemovedClientByScore;

	uint32	m_imaxscore;

    DWORD   m_dwLastCalculatedAverageCombinedFilePrioAndCredit;
    float   m_fAverageCombinedFilePrioAndCredit;

    DWORD   m_dwLastResortedUploadSlots;
};
