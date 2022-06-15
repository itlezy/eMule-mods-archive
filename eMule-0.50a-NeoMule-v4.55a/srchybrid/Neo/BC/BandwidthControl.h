//this file is part of NeoMule
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

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->

#include <deque>
#include <Iphlpapi.h>
#include "Ping.h"
#include "Opcodes.h"

#define AVE_TIMES 50+1 // High Precision // 5 sec

typedef struct
{
    DWORD Bps;          //bytes per second          
    DWORD ave;          //average throughput
    DWORD time;         //time in milliseconds that the sample was taken
    uint64 total;       //total received or sent in bits
} STATS_STRUCT;

#define INIT_STATS_STRUCT(x,i) \
	x[i].ave=0; \
	x[i].Bps=0; \
	x[i].time=0; \
	x[i].total=0; \


class CBandwidthControl : public CObject
{

DECLARE_DYNCREATE(CBandwidthControl)

public:
	CBandwidthControl();
	~CBandwidthControl(void);
	
	float	GetMaxUpload();
	float	GetCalcUpload()							{return upload;}
	float	GetMaxDownload();
	float	GetCalcDownload()						{return download;}

	
	float	GetCurrUpload()							{return eMuleUpRate;}
	float	GetCurrDataUpload()						{return eMuleUpDataRate;}
	float	GetCurrDownload()						{return eMuleDownRate;}
	float	GetCurrDataDownload()					{return eMuleDownDataRate;}
	float	GetCurrUploadOverhead()					{return GetCurrUpload() - GetCurrDataUpload();}
	float	GetCurrDownloadOverhead()				{return GetCurrDownload() - GetCurrDataDownload();}
	//// NEO: NAFC - [NetworkAdapterFeatbackControl]
	//float	GetCurrGlobalUpload()					{return AdapterUpRate;}
	//float	GetCurrGlobalDownload()					{return AdapterDownRate;}
	//// NEO: NAFC END

	float	GetCurrStatsUploadLimit();
	float	GetCurrStatsDownloadLimit();

	float	GetCurrStatsUpload()					{return eMuleStatsUpRate;}
	float	GetCurrStatsDataUpload()				{return eMuleStatsUpDataRate;}
	float	GetCurrStatsDownload()					{return eMuleStatsDownRate;}
	float	GetCurrStatsDataDownload()				{return eMuleStatsDownDataRate;}
	float	GetCurrStatsUploadOverhead()			{return GetCurrStatsUpload() - GetCurrStatsDataUpload();}
	float	GetCurrStatsDownloadOverhead()			{return GetCurrStatsDownload() - GetCurrStatsDataDownload();}
	// NEO: NAFC - [NetworkAdapterFeatbackControl]
	float	GetCurrStatsGlobalUpload()				{return AdapterStatsUpRate;}
	float	GetCurrStatsGlobalDownload()			{return AdapterStatsDownRate;}
	// NEO: NAFC END
 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
	float	GetCurrStatsReleaseUpload()				{return ReleaseStatsUpRate;}
	float	GetCurrStatsFriendUpload()				{return FriendStatsUpRate;}
 #endif // BW_MOD // NEO: BM END

	bool	IsMinUpReached()						{return	minUpReached;} // for accept to reserve
	float	GetMaxUpDataRate()						{return maxUpDataRate;} // for accept to upload
	float	GetMaxDownDataRate()					{return maxDownDataRate;}

	float	GetCurrPing()							{return avg_ping;}
	float	GetLowestPing()							{return lowest_ping;}
	float	GetBasePingUp()							{return base_ping_up;}
	float	GetBasePingDown()						{return base_ping_down;}

	uint64	GetTotalSentBytes()						{return eMuleUp;}
	uint64	GetTotalReceivedBytes()					{return eMuleDown;}

	bool	WasInitialized()						{return wasInit;}
	// ZZ:UploadSpeedSense -->
	bool	IsPingWorking()							{return (pingThread ? pingThread->isWorking() : false);}
	bool	IsPingPreparing()						{return (pingThread ? pingThread->isPreparing() : false);}
	bool	IsPingTimeout()							{return (pingThread ? pingThread->isTimeout() : false);}
	bool	SetPingedServer(CString pingServer);
	CString	GetPingedServer()						{return (pingThread ? pingThread->GetPingedServer() : _T("-"));}
	bool	AddHostsToCheck(CTypedPtrList<CPtrList, CServer*> &list)	{if (pingThread) return pingThread->AddHostsToCheck(list); else return false;}
    bool	AddHostsToCheck(CTypedPtrList<CPtrList, CUpDownClient*> &list)	{if (pingThread) return pingThread->AddHostsToCheck(list); else return false;}
	
	float	GetCurrStatsPing();
	// ZZ:UploadSpeedSense <--
	bool	MissingConnection();
	void	SetMissingConnection(bool bMissing);

	// NEO: NAFC - [NetworkAdapterFeatbackControl]
	bool	IsNAFCWorking()							{return m_currentAdapterIndex > 1;}
	bool	IsNAFCPreparing()						{return m_currentAdapterIndex == 1;}
	DWORD	GetAdapterIP()							{return m_currentAdapterAdress;}
	BOOL	SetAdapterIndex(DWORD dwIP = 0);
	CStringList* GetAdapterList();
	// NEO: NAFC END

	UINT	GetMSS();

	bool	AcceptNewClient();

	void	AddUpIP(bool bUDP = false);
	void	AddUp(uint32 up, bool bUDP = false, bool fileData = false, bool bSolid = true);
	void	AddDownIP(bool bUDP = false);
	void	AddDown(uint32 down, bool bUDP = false, bool fileData = false, bool bSolid = true);

	void	StartBandwidthControl();
	void	EndBandwidthControl(void);

	void	InitStats(bool bAll = true);

	virtual int Process(); 
	virtual	BOOL	InitInstance()					{return true;}

	// NEO: NMFS - [NiceMultiFriendSlots]
	UINT	IsSessionRatio();
	BOOL	IsSessionRatioWorking()					{return m_sessionratio;}
	// NEO: NMFS END

private:
	bool	Init();

	void	StartPinging(bool bOnInit = false);
	void	StopPinging();

	void	ReadNafc(); // NEO: NAFC - [NetworkAdapterFeatbackControl]
	void	CalculateStats();

	void	CheckAdapterIP(); // NEO: NAFC - [NetworkAdapterFeatbackControl]

	void	CalculateSpeedSense(); 
	void	ReCalculateBasePing(float lowestPing, int UpDown = 0);

	void	CalculateNAFC(); // NEO: NAFC - [NetworkAdapterFeatbackControl]

	void	CalcAverages( uint64 dwTotal, DWORD dwTime, DWORD dwBPS, CArray<STATS_STRUCT>& pStats, UINT& ArrayIndex, UINT EntryCount); //from AMUC - Herbert
	float	GetAverages(CList<float,float> &average, float value, int len = 3);

	float	upload;
	float	uploadTemp;
	bool	useUploadTemp;
	
	float	maxUpDataRate;
	bool	minUpReached;

	float	download;
	float	downloadTemp;
	bool	useDownloadTemp;

	float	maxDownDataRate;

	// NEO: NAFC - [NetworkAdapterFeatbackControl]
	uint32	m_LastGetAdapterTry;
	DWORD	m_currentAdapterAdress;
	DWORD	m_currentAdapterIndex;
	// NEO: NAFC END

	// Stats begin
	uint64	eMuleUp;
	uint64	eMuleUpData;
	uint64	eMuleDown;
	uint64	eMuleDownData;
	// NEO: NAFC - [NetworkAdapterFeatbackControl]
	uint64	AdapterUp;
	uint64	AdapterDown;
	// NEO: NAFC END

	// High Precision Stats
	CArray<STATS_STRUCT> eMuleUpAvg;
	CArray<STATS_STRUCT> eMuleUpDataAvg;
	CArray<STATS_STRUCT> eMuleUpStats;
	CArray<STATS_STRUCT> eMuleUpDataStats;
	// NEO: NAFC - [NetworkAdapterFeatbackControl]
	CArray<STATS_STRUCT> AdapterUpAvg;
	CArray<STATS_STRUCT> AdapterUpStats;
	// NEO: NAFC END

	// Low Precision Stats for GUI
	CArray<STATS_STRUCT> eMuleDownAvg;
	CArray<STATS_STRUCT> eMuleDownDataAvg;
	CArray<STATS_STRUCT> eMuleDownStats;
	CArray<STATS_STRUCT> eMuleDownDataStats;
	// NEO: NAFC - [NetworkAdapterFeatbackControl]
	CArray<STATS_STRUCT> AdapterDownAvg;
	CArray<STATS_STRUCT> AdapterDownStats;
	// NEO: NAFC END
 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
	CArray<STATS_STRUCT> ReleaseUpStats;
	CArray<STATS_STRUCT> FriendUpStats;
 #endif // BW_MOD // NEO: BM END
	UINT	StatisticEntries;

	UINT	m_nArrayIndexHigh;
	UINT	m_nArrayIndexLow;

	// High
	float	eMuleUpRate;
	float	eMuleUpDataRate;
	float	eMuleDownRate;
	float	eMuleDownDataRate;
	// NEO: NAFC - [NetworkAdapterFeatbackControl]
	//float	AdapterUpRate;
	//float	AdapterDownRate;
	// NEO: NAFC END

	// Low
	float	eMuleStatsUpRate;
	float	eMuleStatsUpDataRate;
	float	eMuleStatsDownRate;
	float	eMuleStatsDownDataRate;
	// NEO: NAFC - [NetworkAdapterFeatbackControl]
	float	AdapterStatsUpRate;
	float	AdapterStatsDownRate;
	// NEO: NAFC END
 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
	float	ReleaseStatsUpRate;
	float	FriendStatsUpRate;
 #endif // BW_MOD // NEO: BM END

	// Stats end

	float	pingUpTolerance;
	float	pingDownTolerance;

	float	avg_ping;
	float	lowest_ping;
	float	base_ping_up;
	float	base_ping_down;

	uint32	m_uMissingConnection;

	CPingThread* pingThread;
	bool	pingThreadFailed;

	// NEO: NAFC - [NetworkAdapterFeatbackControl]
	uint32	m_lastProcessTime;

	// old toals for difference
    uint64	LastTotalUp;
	uint64	LastTotalUpData;
    uint64	LastTotalDown;
    uint64	LastTotalDownData;
	uint64	LastAdapterUp;
	uint64	LastAdapterDown;

	sint32	m_nUploadSlopeControl;
	sint32	m_nDownloadSlopeControl;
	// NEO: NAFC END

	UINT	m_iMSS;

	bool	acceptNewClient;

	bool	wasInit;
	uint8	uPlatform;

	BOOL	m_sessionratio; // NEO: NMFS - [NiceMultiFriendSlots]
};

uint32 CalcIPOverhead(uint32 len = 0, bool bUDP = false, bool bSolid = true);

#endif // NEO_BC // NEO: NBC END <-- Xanatos --