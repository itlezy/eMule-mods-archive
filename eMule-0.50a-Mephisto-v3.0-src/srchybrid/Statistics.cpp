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
#include "Statistics.h"
#include "BandWidthControl.h" // Xman
#include "Preferences.h"
#include "Opcodes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
extern _CRT_ALLOC_HOOK g_pfnPrevCrtAllocHook;
#endif

// // Xman Maella TPT rework

#define MAXAVERAGETIME			SEC2MS(40) //millisecs

///////////////////////////////////////////////////////////////////////////////
// CStatistics

CStatistics theStats;

// Xman
/*
float	CStatistics::maxDown;
float	CStatistics::maxDownavg;
*/
//Xman end
float	CStatistics::cumDownavg;
float	CStatistics::maxcumDownavg;
float	CStatistics::maxcumDown;
float	CStatistics::cumUpavg;
float	CStatistics::maxcumUpavg;
float	CStatistics::maxcumUp;
// Xman
/*
float	CStatistics::maxUp;
float	CStatistics::maxUpavg;
float	CStatistics::rateDown;
float	CStatistics::rateUp;
*/
//Xman end
uint32	CStatistics::timeTransfers;
uint32	CStatistics::timeDownloads;
uint32	CStatistics::timeUploads;
uint32	CStatistics::start_timeTransfers;
uint32	CStatistics::start_timeDownloads;
uint32	CStatistics::start_timeUploads;
uint32	CStatistics::time_thisTransfer;
uint32	CStatistics::time_thisDownload;
uint32	CStatistics::time_thisUpload;
uint32	CStatistics::timeServerDuration;
uint32	CStatistics::time_thisServerDuration;
// Xman
/*
uint32	CStatistics::m_nDownDatarateOverhead;
uint32	CStatistics::m_nDownDataRateMSOverhead;
*/
//Xman end
uint64	CStatistics::m_nDownDataOverheadSourceExchange;
uint64	CStatistics::m_nDownDataOverheadSourceExchangePackets;
uint64	CStatistics::m_nDownDataOverheadFileRequest;
uint64	CStatistics::m_nDownDataOverheadFileRequestPackets;
uint64	CStatistics::m_nDownDataOverheadServer;
uint64	CStatistics::m_nDownDataOverheadServerPackets;
uint64	CStatistics::m_nDownDataOverheadKad;
uint64	CStatistics::m_nDownDataOverheadKadPackets;
uint64	CStatistics::m_nDownDataOverheadOther;
uint64	CStatistics::m_nDownDataOverheadOtherPackets;
// Xman
/*
uint32	CStatistics::m_nUpDatarateOverhead;
uint32	CStatistics::m_nUpDataRateMSOverhead;
*/
//Xman end
uint64	CStatistics::m_nUpDataOverheadSourceExchange;
uint64	CStatistics::m_nUpDataOverheadSourceExchangePackets;
uint64	CStatistics::m_nUpDataOverheadFileRequest;
uint64	CStatistics::m_nUpDataOverheadFileRequestPackets;
uint64	CStatistics::m_nUpDataOverheadServer;
uint64	CStatistics::m_nUpDataOverheadServerPackets;
uint64	CStatistics::m_nUpDataOverheadKad;
uint64	CStatistics::m_nUpDataOverheadKadPackets;
uint64	CStatistics::m_nUpDataOverheadOther;
uint64	CStatistics::m_nUpDataOverheadOtherPackets;
// Xman
/*
uint32	CStatistics::m_sumavgDDRO;
uint32	CStatistics::m_sumavgUDRO;
*/
//Xman end

float	CStatistics::m_fGlobalDone;
float	CStatistics::m_fGlobalSize;
DWORD	CStatistics::m_dwOverallStatus;

uint64	CStatistics::sessionReceivedBytes;
uint64	CStatistics::sessionSentBytes;
uint64	CStatistics::sessionSentBytesToFriend;
uint16	CStatistics::reconnects;
DWORD	CStatistics::transferStarttime;
DWORD	CStatistics::serverConnectTime;
uint32	CStatistics::filteredclients;
DWORD	CStatistics::starttime;
// Xman Maella TPT
float	CStatistics::currentUploadRate;
float	CStatistics::currentMaxUploadRate;
float	CStatistics::sessionUploadRate;
float	CStatistics::sessionMaxUploadRate;
float	CStatistics::currentDownloadRate;
float	CStatistics::currentMaxDownloadRate;
float	CStatistics::sessionDownloadRate;
float	CStatistics::sessionMaxDownloadRate;
uint32	CStatistics::leecherclients; //Xman Anti-Leecher


CStatistics::CStatistics()
{
	// Xman Maella TPT 
	/*
	maxDown =				0;
	maxDownavg =			0;
	*/
	//Xman end
	maxcumDown =			0;
	cumUpavg =				0;
	maxcumDownavg =			0;
	cumDownavg =			0;
	maxcumUpavg =			0;
	maxcumUp =				0;
	// Xman Maella TPT 
	/*
	maxUp =					0;
	maxUpavg =				0;
	rateDown =				0;
	rateUp =				0;
	*/
	//Xman end
	timeTransfers =			0;
	timeDownloads =			0;
	timeUploads =			0;
	start_timeTransfers =	0;
	start_timeDownloads =	0;
	start_timeUploads =		0;
	time_thisTransfer =		0;
	time_thisDownload =		0;
	time_thisUpload =		0;
	timeServerDuration =	0;
	time_thisServerDuration=0;

	sessionReceivedBytes=0;
	sessionSentBytes=0;
    sessionSentBytesToFriend=0;
	reconnects=0;
	transferStarttime=0;
	serverConnectTime=0;
	filteredclients=0;
	starttime=0;
	

	m_fGlobalDone =								0;
	m_fGlobalSize =								0;
	m_dwOverallStatus  =						0;
	// Xman Maella TPT 
	/*
	m_nDownDataRateMSOverhead = 0;
	m_nDownDatarateOverhead = 0;
	*/
	//Xman end
	m_nDownDataOverheadSourceExchange = 0;
	m_nDownDataOverheadSourceExchangePackets = 0;
	m_nDownDataOverheadFileRequest = 0;
	m_nDownDataOverheadFileRequestPackets = 0;
	m_nDownDataOverheadServer = 0;
	m_nDownDataOverheadServerPackets = 0;
	m_nDownDataOverheadKad = 0;
	m_nDownDataOverheadKadPackets = 0;
	m_nDownDataOverheadOther = 0;
	m_nDownDataOverheadOtherPackets = 0;
	// Xman Maella TPT 
	/*
	m_sumavgDDRO = 0;

	m_nUpDataRateMSOverhead = 0;
	m_nUpDatarateOverhead = 0;
	*/
	//Xman end
	m_nUpDataOverheadSourceExchange = 0;
	m_nUpDataOverheadSourceExchangePackets = 0;
	m_nUpDataOverheadFileRequest = 0;
	m_nUpDataOverheadFileRequestPackets = 0;
	m_nUpDataOverheadServer = 0;
	m_nUpDataOverheadServerPackets = 0;
	m_nUpDataOverheadKad = 0;
	m_nUpDataOverheadKadPackets = 0;
	m_nUpDataOverheadOther = 0;
	m_nUpDataOverheadOtherPackets = 0;
	// Xman Maella TPT 
	/*
	m_sumavgUDRO = 0;
	*/
	//Xman end
	
	// - Maella Bandwidth
	currentUploadRate = 0;
	currentMaxUploadRate = 0;
	sessionUploadRate = 0;
	sessionMaxUploadRate = 0;

	currentDownloadRate = 0;
	currentMaxDownloadRate = 0;
	sessionDownloadRate = 0;
	sessionMaxDownloadRate = 0;

	leecherclients=0; //Xman Anti-Leecher
}

void CStatistics::Init()
{
	maxcumDown =			thePrefs.GetConnMaxDownRate();
	cumUpavg =				thePrefs.GetConnAvgUpRate();
	maxcumDownavg =			thePrefs.GetConnMaxAvgDownRate();
	cumDownavg =			thePrefs.GetConnAvgDownRate();
	maxcumUpavg =			thePrefs.GetConnMaxAvgUpRate();
	maxcumUp =				thePrefs.GetConnMaxUpRate();
}

// This function is going to basically calculate and save a bunch of averages.
//				I made a seperate funtion so that it would always run instead of having
//				the averages not be calculated if the graphs are disabled (Which is bad!).
// Maella -Graph: code Improvement for rate display-
/*
void CStatistics::UpdateConnectionStats(float uploadrate, float downloadrate)
{
	rateUp = uploadrate;
	rateDown = downloadrate;

	if (maxUp < uploadrate)
		maxUp = uploadrate;
	if (maxcumUp < maxUp){
		maxcumUp = maxUp;
		thePrefs.SetConnMaxUpRate(maxcumUp);
	}

	if (maxDown < downloadrate)
		maxDown = downloadrate;
	if (maxcumDown < maxDown){
		maxcumDown = maxDown;
		thePrefs.SetConnMaxDownRate(maxcumDown);
	}

	cumDownavg = GetAvgDownloadRate(AVG_TOTAL);
	if (maxcumDownavg < cumDownavg){
		maxcumDownavg = cumDownavg;
		thePrefs.SetConnMaxAvgDownRate(maxcumDownavg);
	}

	cumUpavg = GetAvgUploadRate(AVG_TOTAL);
	if (maxcumUpavg < cumUpavg){
		maxcumUpavg = cumUpavg;
		thePrefs.SetConnMaxAvgUpRate(maxcumUpavg);
	}
	
	// Transfer Times (Increment Session)
	if (uploadrate > 0 || downloadrate > 0) {
		if (start_timeTransfers == 0)
			start_timeTransfers = GetTickCount();
		else
			time_thisTransfer = (GetTickCount() - start_timeTransfers) / 1000;

		if (uploadrate > 0) {
			if (start_timeUploads == 0)
				start_timeUploads = GetTickCount();
			else
				time_thisUpload = (GetTickCount() - start_timeUploads) / 1000;
		}

		if (downloadrate > 0) {
			if (start_timeDownloads == 0)
				start_timeDownloads = GetTickCount();
			else
				time_thisDownload = (GetTickCount() - start_timeDownloads) / 1000;
		}
	}

	if (uploadrate == 0 && downloadrate == 0 && (time_thisTransfer > 0 || start_timeTransfers > 0)) {
		timeTransfers += time_thisTransfer;
		time_thisTransfer = 0;
		start_timeTransfers = 0;
	}

	if (uploadrate == 0 && (time_thisUpload > 0 || start_timeUploads > 0)) {
		timeUploads += time_thisUpload;
		time_thisUpload = 0;
		start_timeUploads = 0;
	}

	if (downloadrate == 0 && (time_thisDownload > 0 || start_timeDownloads > 0)) {
		timeDownloads += time_thisDownload;
		time_thisDownload = 0;
		start_timeDownloads = 0;
	}

	// Server Durations
	if (theStats.serverConnectTime == 0) 
		time_thisServerDuration = 0;
	else
		time_thisServerDuration = (GetTickCount() - theStats.serverConnectTime) / 1000;
}

void CStatistics::RecordRate()
{
	if (theStats.transferStarttime == 0)
		return;

	// Accurate datarate Calculation
	uint32 stick = GetTickCount();
	TransferredData newitemUP = {(UINT)theStats.sessionSentBytes, stick};
	TransferredData newitemDN = {(UINT)theStats.sessionReceivedBytes, stick};

	downrateHistory.push_front(newitemDN);
	uprateHistory.push_front(newitemUP);

	// limit to maxmins
	UINT uAverageMilliseconds = thePrefs.GetStatsAverageMinutes() * 60000;
	while (downrateHistory.front().timestamp - downrateHistory.back().timestamp > uAverageMilliseconds)
		downrateHistory.pop_back();
	while (uprateHistory.front().timestamp - uprateHistory.back().timestamp > uAverageMilliseconds)
		uprateHistory.pop_back();
}

// Changed these two functions (khaos)...
float CStatistics::GetAvgDownloadRate(int averageType)
{
	DWORD running;
	switch (averageType)
	{
		case AVG_SESSION:
			if (theStats.transferStarttime == 0)
				return 0.0F;
			running = (GetTickCount() - theStats.transferStarttime) / 1000;
			if (running < 5)
				return 0.0F;
			return (float)(theStats.sessionReceivedBytes / 1024) / running;

		case AVG_TOTAL:
			if (theStats.transferStarttime == 0)
				return thePrefs.GetConnAvgDownRate();
			running = (GetTickCount() - theStats.transferStarttime) / 1000;
			if (running < 5)
				return thePrefs.GetConnAvgDownRate();
			return (((float)(theStats.sessionReceivedBytes / 1024) / running) + thePrefs.GetConnAvgDownRate()) / 2.0F;

		default:
			if (downrateHistory.size() == 0)
				return 0.0F;
			float deltat = (downrateHistory.front().timestamp - downrateHistory.back().timestamp) / 1000.0F;
			if (deltat > 0.0F)
				return ((downrateHistory.front().datalen - downrateHistory.back().datalen) / deltat) / 1024.0F;
			return 0.0F;
	}
}

float CStatistics::GetAvgUploadRate(int averageType)
{
	DWORD running;
	switch (averageType)
	{
		case AVG_SESSION:
			if (theStats.transferStarttime == 0)
				return 0.0F;
			running = (GetTickCount() - theStats.transferStarttime) / 1000;
			if (running < 5)
				return 0.0F;
			return (float)(theStats.sessionSentBytes / 1024) / running;

		case AVG_TOTAL:
			if (theStats.transferStarttime == 0)
				return thePrefs.GetConnAvgUpRate();
			running = (GetTickCount() - theStats.transferStarttime) / 1000;
			if (running < 5)
				return thePrefs.GetConnAvgUpRate();
			return (((float)(theStats.sessionSentBytes / 1024) / running) + thePrefs.GetConnAvgUpRate()) / 2.0F;

		default:
			if (uprateHistory.size() == 0)
				return 0.0F;
			float deltat = (uprateHistory.front().timestamp - uprateHistory.back().timestamp) / 1000.0F;
			if (deltat > 0.0F)
				return ((uprateHistory.front().datalen - uprateHistory.back().datalen) / deltat) / 1024.0F;
			return 0.0F;
	}
}

void CStatistics::CompDownDatarateOverhead()
{
	TransferredData newitem = {m_nDownDataRateMSOverhead, GetTickCount()};
	m_AvarageDDRO_list.AddTail(newitem);
	m_sumavgDDRO += m_nDownDataRateMSOverhead;
	m_nDownDataRateMSOverhead = 0;

	while (m_AvarageDDRO_list.GetTail().timestamp - m_AvarageDDRO_list.GetHead().timestamp > MAXAVERAGETIME)
		m_sumavgDDRO -= m_AvarageDDRO_list.RemoveHead().datalen;

	if (m_AvarageDDRO_list.GetCount() > 10)
	{
		DWORD dwDuration = m_AvarageDDRO_list.GetTail().timestamp - m_AvarageDDRO_list.GetHead().timestamp;
		if (dwDuration)
			m_nDownDatarateOverhead = 1000 * (m_sumavgDDRO - m_AvarageDDRO_list.GetHead().datalen) / dwDuration;
	}
	else
		m_nDownDatarateOverhead = 0;
}

void CStatistics::CompUpDatarateOverhead()
{
	TransferredData newitem = {m_nUpDataRateMSOverhead, GetTickCount()};
	m_AvarageUDRO_list.AddTail(newitem);
	m_sumavgUDRO += m_nUpDataRateMSOverhead;
	m_nUpDataRateMSOverhead = 0;

	while (m_AvarageUDRO_list.GetTail().timestamp - m_AvarageUDRO_list.GetHead().timestamp > MAXAVERAGETIME)
		m_sumavgUDRO -= m_AvarageUDRO_list.RemoveHead().datalen;

	if (m_AvarageUDRO_list.GetCount() > 10)
	{
		DWORD dwDuration = m_AvarageUDRO_list.GetTail().timestamp - m_AvarageUDRO_list.GetHead().timestamp;
		if (dwDuration)
			m_nUpDatarateOverhead = 1000 * (m_sumavgUDRO - m_AvarageUDRO_list.GetHead().datalen) / dwDuration;
	}
	else
		m_nUpDatarateOverhead = 0;
}

void CStatistics::ResetDownDatarateOverhead()
{
	m_nDownDataRateMSOverhead = 0;
	m_AvarageDDRO_list.RemoveAll();
	m_sumavgDDRO = 0;
	m_nDownDatarateOverhead = 0;
}

void CStatistics::ResetUpDatarateOverhead()
{
	m_nUpDataRateMSOverhead = 0;
	m_sumavgUDRO = 0;
	m_AvarageUDRO_list.RemoveAll();
	m_nUpDatarateOverhead = 0;
}
*/
void CStatistics::UpdateConnectionStats(void)
{
	// Wait at least 5 seconds
	if(::GetTickCount() - theApp.pBandWidthControl->GetStartTick() + 5000)
	{

		// Maella -Graph: code Improvement for rate display-
		uint32 plotOutData[ADAPTER+1];
		uint32 plotinData[ADAPTER+1];
		theApp.pBandWidthControl->GetDatarates(10,	//Xman: to avoid peeks use avg over 10 seconds
											plotinData[CURRENT], plotinData[OVERALL],
											plotOutData[CURRENT], plotOutData[OVERALL],
											plotinData[ADAPTER], plotOutData[ADAPTER]);

		theApp.pBandWidthControl->GetFullHistoryDatarates(plotinData[MINUTE], plotOutData[MINUTE],
														  plotinData[SESSION], plotOutData[SESSION]);

		currentDownloadRate = (float)plotinData[CURRENT]/1024.0f;
		sessionDownloadRate = (float)plotinData[SESSION]/1024.0f;
		currentUploadRate   = (float)plotOutData[CURRENT]/1024.0f;
		sessionUploadRate   = (float)plotOutData[SESSION]/1024.0f;
		// Maella end

		//Xman Patch: be realistic:
		if (currentDownloadRate> thePrefs.GetMaxGraphDownloadRate())
			currentDownloadRate=thePrefs.GetMaxGraphDownloadRate();
		if (currentUploadRate> thePrefs.GetMaxGraphUploadRate())
			currentUploadRate=thePrefs.GetMaxGraphUploadRate();
		//Xman end

		// Max Current rates (graph refresh)
		if(currentMaxUploadRate < currentUploadRate) 
			currentMaxUploadRate = currentUploadRate;
		if(currentMaxDownloadRate < currentDownloadRate) 
			currentMaxDownloadRate = currentDownloadRate;

		// Session Max rates
		if(sessionMaxUploadRate < sessionUploadRate) 
			sessionMaxUploadRate = sessionUploadRate;
		if(sessionMaxDownloadRate < sessionDownloadRate) 
			sessionMaxDownloadRate = sessionDownloadRate;

		// Cumulative Max Current rates (graph refresh)
		if(maxcumUp < currentMaxUploadRate) 
		{
			maxcumUp = currentMaxUploadRate;
			thePrefs.SetConnMaxUpRate(maxcumUp);
		}

		if(maxcumDown < currentMaxDownloadRate) 
		{
			maxcumDown = currentMaxDownloadRate;
			thePrefs.SetConnMaxDownRate(maxcumDown);
		}

		// Cumulative Max Average rates
		//official doesn't use this anymore
		//cumDownavg = (sessionDownloadRate + thePrefs.GetConnAvgDownRate()) / 2;
		//cumUpavg = (sessionUploadRate + thePrefs.GetConnAvgUpRate()) / 2;
		cumDownavg = sessionDownloadRate;
		cumUpavg = sessionUploadRate;
		if(maxcumDownavg < cumDownavg) 
		{
			maxcumDownavg = cumDownavg;
			thePrefs.SetConnMaxAvgDownRate(maxcumDownavg);
		}

		if(maxcumUpavg < cumUpavg) 
		{
			maxcumUpavg = cumUpavg;
			thePrefs.SetConnMaxAvgUpRate(maxcumUpavg);
		}
		
		// Transfer Times (Increment Session)
		if (sessionUploadRate > 0.0f || sessionDownloadRate > 0.0f) 
		{
			if (start_timeTransfers == 0)
				start_timeTransfers = GetTickCount();
			else
				time_thisTransfer = (GetTickCount() - start_timeTransfers) / 1000;

			if (sessionUploadRate > 0.0f) 
			{
				if (start_timeUploads == 0)
					start_timeUploads = GetTickCount();
				else
					time_thisUpload = (GetTickCount() - start_timeUploads) / 1000;
			}
			
			if (sessionDownloadRate > 0.0f) 
			{
				if (start_timeDownloads == 0)
					start_timeDownloads = GetTickCount();
				else
					time_thisDownload = (GetTickCount() - start_timeDownloads) / 1000;
			}
		}

		if (sessionUploadRate == 0.0f && sessionDownloadRate == 0.0f && (time_thisTransfer > 0 || start_timeTransfers > 0)) 
		{
			timeTransfers += time_thisTransfer;
			time_thisTransfer = 0;
			start_timeTransfers = 0;
		}

		if (sessionUploadRate == 0.0f && (time_thisUpload > 0 || start_timeUploads > 0)) 
		{
			timeUploads += time_thisUpload;
			time_thisUpload = 0;
			start_timeUploads = 0;
		}

		if (sessionDownloadRate == 0.0f && (time_thisDownload > 0 || start_timeDownloads > 0)) 
		{
			timeDownloads += time_thisDownload;
			time_thisDownload = 0;
			start_timeDownloads = 0;
		}

		// Server Durations
	if (theStats.serverConnectTime == 0) 
			time_thisServerDuration = 0;
		else
			time_thisServerDuration = (GetTickCount() - theStats.serverConnectTime) / 1000;
	}
}
//Xman End

#ifdef USE_MEM_STATS

#ifdef _DEBUG
#error "Does not work when _DEBUG defined!"
#endif

#ifdef _AFXDLL
#error "Not supported for _AFXDLL!"
#endif

#if _MFC_VER!=0x0710
#error "Not verified for this _MFC_VER!"
#endif

ULONGLONG g_aAllocStats[ALLOC_SLOTS];

/////////////////////////////////////////////////////////////////////////////
// Non-diagnostic memory routines

int AFX_CDECL AfxNewHandler(size_t /* nSize */)
{
	AfxThrowMemoryException();
	return 0;
}

#pragma warning(disable: 4273)

_PNH _afxNewHandler = &AfxNewHandler;

_PNH AFXAPI AfxGetNewHandler(void)
{
	return _afxNewHandler;
}

_PNH AFXAPI AfxSetNewHandler(_PNH pfnNewHandler)
{
	_PNH pfnOldHandler = _afxNewHandler;
	_afxNewHandler = pfnNewHandler;
	return pfnOldHandler;
}

AFX_STATIC_DATA const _PNH _pfnUninitialized = (_PNH)-1;

inline int log2(unsigned x)
{
	int log = 0;
	while (x >>= 1)
		++log;
	return log;
}

void* my_new(size_t n)
{
	int i;
	if (n == 0)
		i = 0;
	else {
		i = log2(n) + 1;
		if (i >= ALLOC_SLOTS)
			i = ALLOC_SLOTS - 1;
	}
	g_aAllocStats[i]++;

	void* pResult;
	for (;;)
	{
		pResult = malloc(n);
		if (pResult != NULL)
			return pResult;

		if (_afxNewHandler == NULL || (*_afxNewHandler)(n) == 0)
			break;
	}
	return pResult;
}

void* __cdecl operator new(size_t nSize)
{
	return my_new(nSize);
}

void* __cdecl operator new[](size_t nSize)
{
	return ::operator new(nSize);
}

void __cdecl operator delete(void* p)
{
	free(p);
}

void __cdecl operator delete[](void* p)
{
	::operator delete(p);
}

#endif
