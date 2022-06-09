//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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

#pragma warning(push)
#pragma warning(disable:4702) // unreachable code
#include <vector>
#include <list>
#pragma warning(pop)
#include "Loggable.h"

#define UL_QUEUE_DATARATE_SAMPLE_TIME 500

class CUpDownClient;

typedef std::list<CUpDownClient*> ClientList;

class CUploadQueue : public CLoggable
{
	friend class CWebServer;
public:
	CUploadQueue();
	~CUploadQueue();
	void	Process();
	void	AddClientToWaitingQueue(CUpDownClient* client,bool bIgnoreTimelimit = false);
	bool	RemoveFromUploadQueue(CUpDownClient* client, EnumEndTransferSession eReason, bool updatewindow = true);
	bool	RemoveFromWaitingQueue(CUpDownClient* client,bool updatewindow = true);
	bool	IsOnUploadQueue(CUpDownClient* client)	{return (GetWaitingClient(client) != NULL); }
	bool	IsDownloading(CUpDownClient* pClient);
	uint32	GetDataRate()								{return m_dwDataRate;}
	int		GetWaitingUserCount()					{return waitinglist.GetCount();}
	int	GetUploadQueueLength()					{return m_UploadingList.size();}
	CUpDownClient*	GetWaitingClientByIPAndUDPPort(uint32 dwIP, uint16 dwPort);

	void	SCHShift1UploadCheck();
	void	SCHShift2UploadCheck();

	void	DeleteAll();
	uint16	GetWaitingPosition(CUpDownClient* client);
	void	SetBanCount(uint32 dwCount)				{ m_dwBannedCount = dwCount; }
	uint32	GetBanCount() const						{ return m_dwBannedCount; }
	// v: eklmn: extended UL statistic
	uint32	GetSuccessfulUpCount()					{return m_iULSessionSuccessful;}
	uint32	GetULFullChunkCount()					{return m_iULSessionSuccessfulFullChunk;}	// Should that be full up-chuck count?
	uint32	GetULPartChunkCount()					{uint32 part_chunk = 0;
														  for (int i =0;i<ETS_TERMINATOR;i++)
															part_chunk += m_iULSessionSuccessfulPartChunk[i];
														  return part_chunk;
														}
	uint32	GetULPartChunkSubCount(EnumEndTransferSession index)		{return m_iULSessionSuccessfulPartChunk[index];}
	uint32	GetFailedUpCount()						{uint32 failed = 0;
														  for (int i =0;i<ETS_TERMINATOR;i++)
															failed += m_iULSessionFailed[i];
														  return failed;
														}
	uint32	GetFailedSubCount(EnumEndTransferSession index)		{return m_iULSessionFailed[index];}
	// ^: eklmn: extended UL statistic
	uint32	GetAverageUpTime();
	void	FindSourcesForFileById(CTypedPtrList<CPtrList, CUpDownClient*>* srclist, const uchar* filehash);
	void	GetUploadFilePartsAvailability(uint32 *pdwStatuses, uint32 dwPartCnt, const byte *pbyteFileHash);
	void	AddUpDataOverheadSourceExchange(uint32 data)	{ m_nUpDataRateMSOverhead += data;
															  m_nUpDataOverheadSourceExchange += data;
															  m_nUpDataOverheadSourceExchangePackets++;}
	void	AddUpDataOverheadFileRequest(uint32 data)		{ m_nUpDataRateMSOverhead += data;
															  m_nUpDataOverheadFileRequest += data;
															  m_nUpDataOverheadFileRequestPackets++;}
	void	AddUpDataOverheadServer(uint32 data)			{ m_nUpDataRateMSOverhead += data;
															  m_nUpDataOverheadServer += data;
															  m_nUpDataOverheadServerPackets++;}
	void	AddUpDataOverheadOther(uint32 data)				{ m_nUpDataRateMSOverhead += data;
															  m_nUpDataOverheadOther += data;
															  m_nUpDataOverheadOtherPackets++;}
	uint32	GetUpDataRateOverhead()						{return m_nUpDataRateOverhead;}
	uint64	GetUpDataOverheadSourceExchange()			{return m_nUpDataOverheadSourceExchange;}
	uint64	GetUpDataOverheadFileRequest()				{return m_nUpDataOverheadFileRequest;}
	uint64	GetUpDataOverheadServer()					{return m_nUpDataOverheadServer;}
	uint64	GetUpDataOverheadOther()					{return m_nUpDataOverheadOther;}
	uint64	GetUpDataOverheadSourceExchangePackets()	{return m_nUpDataOverheadSourceExchangePackets;}
	uint64	GetUpDataOverheadFileRequestPackets()		{return m_nUpDataOverheadFileRequestPackets;}
	uint64	GetUpDataOverheadServerPackets()			{return m_nUpDataOverheadServerPackets;}
	uint64	GetUpDataOverheadOtherPackets()				{return m_nUpDataOverheadOtherPackets;}
	void	CompUpDataRateOverhead();
	POSITION GetHeadPosition();
	CUpDownClient* GetNext(POSITION& pos);
	void	GetCopyUploadQueueList(ClientList *pCopy);
protected:
	void	RemoveFromWaitingQueue(POSITION pos, bool updatewindow);
	POSITION	GetWaitingClient(CUpDownClient* client);
	POSITION	GetWaitingClientByID(CUpDownClient* client);
	bool		CanAcceptNewClient(uint32 dwNumOfUploads);
	void		AddClientToUploadQueue(CUpDownClient* pClient = NULL);

public:
	DWORD	m_dwLastScheduledBackupTick;
	HANDLE	m_hQuitEvent;

private:
	std::deque<uint64> m_averageDataRateList;
	std::deque<DWORD> m_averageTickList;
	std::deque<uint32> m_activeClientsDeque;
	std::vector<uint32> m_activeClientsSortedVector;
	CTypedPtrList<CPtrList, CUpDownClient*> waitinglist;
	ClientList m_UploadingList;

	uint32	m_dwDataRate;   //data rate

	UINT_PTR m_hTimer;
	uint32	m_dwBannedCount;
	uint32	m_iULSessionSuccessful;
	uint32	m_iULSessionSuccessfulFullChunk;
	uint32	m_iULSessionSuccessfulPartChunk[ETS_TERMINATOR];
	uint32	m_iULSessionFailed[ETS_TERMINATOR];
	uint32 	m_iULSessionFailedNoDataForRemoteClient;

	uint32	m_iTotalUploadTime;
	sint32	m_iLeftOverBandwidth;
	uint32	m_nUpDataRateOverhead;
	uint32	m_nUpDataRateMSOverhead;
	uint64	m_nUpDataOverheadSourceExchange;
	uint64	m_nUpDataOverheadFileRequest;
	uint64	m_nUpDataOverheadServer;
	uint64	m_nUpDataOverheadOther;
	uint64	m_nUpDataOverheadSourceExchangePackets;
	uint64	m_nUpDataOverheadFileRequestPackets;
	uint64	m_nUpDataOverheadServerPackets;
	uint64	m_nUpDataOverheadOtherPackets;
	std::deque<uint32>	m_UpDataOverheadDeque;
	uint32	m_dwSumUpDataOverheadInDeque;

	uint32	m_guessedMaxLANBandwidth; // LANCAST (moosetea) the max LAN data rate

	DWORD	m_lastGaveDataTick;
	CWinThread *m_pSaveThread;

	CString	m_strUploadLogFilePath;

	CRITICAL_SECTION		m_csUploadQueueList;
};
