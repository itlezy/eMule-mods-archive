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


#include "StdAfx.h"
#include "BandwidthControl.h"
#include "DownloadBandwidthThrottler.h"
#include "UploadBandwidthThrottler.h"
#include "emule.h"
#include "preferences.h"
#include "math.h"
#include "uploadqueue.h"
#include "downloadqueue.h"
#include "emuleDlg.h"
#include "statistics.h"
#include "OtherFunctions.h"
#include "log.h"
#include "Sockets.h"
#include <ws2tcpip.h>
#include <Iphlpapi.h>
#include "Neo/NeoPreferences.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->

IMPLEMENT_DYNCREATE(CBandwidthControl, CObject)
CBandwidthControl::CBandwidthControl(void)
{
	upload					= thePrefs.GetMaxUpload();
	uploadTemp				= 0.0f;
	useUploadTemp			= false;

	maxUpDataRate			= 0.0f;
	minUpReached			= false;


	download				= thePrefs.GetMaxDownload();
	downloadTemp			= 0.0f;
	useDownloadTemp			= false;

	maxDownDataRate			= 0.0f;

	InitStats();

	eMuleUp					= 0;
	eMuleUpData				= 0;
	eMuleDown				= 0;
	eMuleDownData			= 0;
	// NEO: NAFC - [NetworkAdapterFeatbackControl]
	AdapterUp				= 0;
	AdapterDown				= 0;
	// NEO: NAFC END

	pingThread				= NULL;
	pingThreadFailed		= false;

	pingUpTolerance			= 0xff;
	pingDownTolerance		= 0xff;

	avg_ping				= 0.0f;
	lowest_ping				= 0.0f;
	base_ping_up			= 100.0f;
	base_ping_down			= 200.0f;

	// NEO: NAFC - [NetworkAdapterFeatbackControl]
	m_LastGetAdapterTry		= ::GetTickCount();

	m_currentAdapterIndex	= 0;
	m_currentAdapterAdress	= 0;
	// NEO: NAFC END

	m_uMissingConnection	= 0;

	// NEO: NAFC - [NetworkAdapterFeatbackControl]
	m_lastProcessTime		= ::GetTickCount();

	LastTotalUp			= 0;
	LastTotalUpData		= 0;
	LastTotalDown		= 0;
	LastTotalDownData	= 0;
	LastAdapterUp		= 0;
	LastAdapterDown		= 0;

	m_nUploadSlopeControl	= 0;
	m_nDownloadSlopeControl	= 0;
	// NEO: NAFC END

	m_iMSS = 0;

	acceptNewClient = true;

	wasInit	= false;
	uPlatform = 0;

	m_sessionratio = FALSE; // NEO: NMFS - [NiceMultiFriendSlots]
}

void CBandwidthControl::InitStats(bool bAll)
{
	if(bAll)
	{
		// High precision
		eMuleUpRate				= 0.0f;
		eMuleUpDataRate			= 0.0f;
		eMuleDownRate			= 0.0f;
		eMuleDownDataRate		= 0.0f;
		// NEO: NAFC - [NetworkAdapterFeatbackControl]
		//AdapterUpRate			= 0.0f;
		//AdapterDownRate		= 0.0f;
		// NEO: NAFC END

		eMuleUpAvg.SetSize(AVE_TIMES);
		eMuleDownAvg.SetSize(AVE_TIMES);
		eMuleUpDataAvg.SetSize(AVE_TIMES);
		eMuleDownDataAvg.SetSize(AVE_TIMES);
		// NEO: NAFC - [NetworkAdapterFeatbackControl]
		//AdapterUpAvg.SetSize(AVE_TIMES);
		//AdapterDownAvg.SetSize(AVE_TIMES);
		// NEO: NAFC END
		for (UINT i=0; i<AVE_TIMES; i++){
			INIT_STATS_STRUCT(eMuleUpAvg,i)
			INIT_STATS_STRUCT(eMuleDownAvg,i)
			INIT_STATS_STRUCT(eMuleUpDataAvg,i)
			INIT_STATS_STRUCT(eMuleDownDataAvg,i)
			// NEO: NAFC - [NetworkAdapterFeatbackControl]
			//INIT_STATS_STRUCT(AdapterUpAvg,i)
			//INIT_STATS_STRUCT(AdapterDownAvg,i)
			// NEO: NAFC END
		}

		m_nArrayIndexHigh = 0;
	}

	// Low presision
	eMuleStatsUpRate		= 0.0f;
	eMuleStatsUpDataRate	= 0.0f;
	eMuleStatsDownRate		= 0.0f;
	eMuleStatsDownDataRate	= 0.0f;
	// NEO: NAFC - [NetworkAdapterFeatbackControl]
	AdapterStatsUpRate		= 0.0f;
	AdapterStatsDownRate	= 0.0f;
	// NEO: NAFC END
 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
	ReleaseStatsUpRate		= 0.0f;
	FriendStatsUpRate		= 0.0f;
 #endif // BW_MOD // NEO: BM END

	StatisticEntries = (NeoPrefs.SmoothStatisticsGraphs() ? 400 : (NeoPrefs.GetDatarateSamples() * 10)) + 1;

	// NEO: NAFC END
	eMuleUpStats.SetSize(StatisticEntries);
	eMuleDownStats.SetSize(StatisticEntries);
	eMuleUpDataStats.SetSize(StatisticEntries);
	eMuleDownDataStats.SetSize(StatisticEntries);
	// NEO: NAFC - [NetworkAdapterFeatbackControl]
	AdapterUpStats.SetSize(StatisticEntries);
	AdapterDownStats.SetSize(StatisticEntries);
	// NEO: NAFC END
 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
	ReleaseUpStats.SetSize(StatisticEntries);
	FriendUpStats.SetSize(StatisticEntries);
 #endif // BW_MOD // NEO: BM END
	for (UINT i=0; i<StatisticEntries; i++){
		INIT_STATS_STRUCT(eMuleUpStats,i)
		INIT_STATS_STRUCT(eMuleDownStats,i)
		INIT_STATS_STRUCT(eMuleUpDataStats,i)
		INIT_STATS_STRUCT(eMuleDownDataStats,i)
		// NEO: NAFC - [NetworkAdapterFeatbackControl]
		INIT_STATS_STRUCT(AdapterUpStats,i)
		INIT_STATS_STRUCT(AdapterDownStats,i)
		// NEO: NAFC END
 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
		INIT_STATS_STRUCT(ReleaseUpStats,i)
		INIT_STATS_STRUCT(FriendUpStats,i)
 #endif // BW_MOD // NEO: BM END
	}

	m_nArrayIndexLow = 0;
}

void CBandwidthControl::StartBandwidthControl(){
	if(Init())
		AddLogLine(false,_T("Neo BandwidthControl initialized"));
}

bool CBandwidthControl::Init(){

	// NEO: NAFC - [NetworkAdapterFeatbackControl]
	OSVERSIONINFO os = { sizeof(os) };
	GetVersionEx(&os);
	if( VER_PLATFORM_WIN32_NT == os.dwPlatformId && os.dwMajorVersion >= 5)
		uPlatform = os.dwMinorVersion >= 1 ? 1 : 2;
	// NEO: NAFC END

	// start ping thread
	if (NeoPrefs.IsCheckConnection() || NeoPrefs.IsUSSEnabled() || NeoPrefs.IsDSSEnabled())
		StartPinging(true);

	// NEO: NAFC - [NetworkAdapterFeatbackControl]
	if(NeoPrefs.IsNAFCEnabled()){
		if(!SetAdapterIndex()) // if failed then 0 returned, 
			m_currentAdapterIndex = 0; // set index setting 0 to indicate fail
	}else
		m_currentAdapterIndex = 1; // Adapter 1 is local host, (for our purpose invalid), get adapter index on enable
	// NEO: NAFC END

	// Start the Bandwidth Throttler threads
//#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler]
//	theApp.uploadBandwidthThrottler->StartThread();
//#endif // NEO_UBT // NEO: NUBT END
//#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler]
//	theApp.downloadBandwidthThrottler->StartThread();
//#endif // NEO_DBT // NEO: NDBT END

	wasInit = true;
	return true;
}

void CBandwidthControl::StartPinging(bool bOnInit){
	if (pingThread)
		return;

	pingThread = (CPingThread*)AfxBeginThread(RUNTIME_CLASS(CPingThread), THREAD_PRIORITY_HIGHEST, 0, CREATE_SUSPENDED);
	if (pingThread){
		pingThread->Init();
		pingThread->StartPingThread();
	}
	else {
		pingThreadFailed = true;
		theApp.QueueDebugLogLine(false, _T("Can't initialize Speed Sense! USS and DSS are not able to work!"));
	}
	if(!bOnInit)
		theApp.emuledlg->SetStatusBarPartsSize();
}

void CBandwidthControl::StopPinging(){
	pingThreadFailed = false;
	if (!pingThread)
		return;
	pingThread->EndPingThread();
	delete pingThread;
	pingThread=NULL;
	theApp.emuledlg->SetStatusBarPartsSize();
}

void CBandwidthControl::EndBandwidthControl(void)
{
	if (pingThread){
		pingThread->EndPingThread();
		delete pingThread;
		pingThread=NULL;
	}
}

CBandwidthControl::~CBandwidthControl(void)
{
	if (pingThread) 
		delete pingThread;
}

int CBandwidthControl::Process()
{
	if (!pingThread && !pingThreadFailed && (NeoPrefs.IsCheckConnection() || NeoPrefs.IsUSSEnabled() || NeoPrefs.IsDSSEnabled())){
		StartPinging();
	}else if ((pingThread || pingThreadFailed) && !NeoPrefs.IsCheckConnection() && !NeoPrefs.IsUSSEnabled() && !NeoPrefs.IsDSSEnabled()){
		StopPinging();
		useUploadTemp=false;
	}

	// Get Total datas from NAFC
	ReadNafc();

	// Calculate the statistics
	CalculateStats();

	// Note: The pinger may work without USS/DSS as connection checker...
	if(NeoPrefs.IsUSSEnabled() || NeoPrefs.IsDSSEnabled()){
		CalculateSpeedSense();
	} else if(NeoPrefs.IsCheckConnection() && pingThread){
		pingVal ping = pingThread->GetPing();
		if (ping.changed){
			avg_ping = ping.avg_ping;
			lowest_ping = ping.lowest_ping;
		}
	}

	// NEO: NAFC - [NetworkAdapterFeatbackControl]
	if(NeoPrefs.IsNAFCUpload() || NeoPrefs.IsNAFCDownload())
		CalculateNAFC();
	// NEO: NAFC END

	if ((!NeoPrefs.IsNAFCUpload() || !IsNAFCWorking()) // NEO: NAFC - [NetworkAdapterFeatbackControl]
	 && (!NeoPrefs.IsUSSEnabled() || (pingThread && !pingThread->isPreparing(true) && !pingThread->isWorking())) ){
		minUpReached = true;
		upload=thePrefs.GetMaxUpload();
	}

	if ((!NeoPrefs.IsNAFCDownload() || !IsNAFCWorking()) // NEO: NAFC - [NetworkAdapterFeatbackControl]
	 && (!NeoPrefs.IsDSSEnabled() || (pingThread && !pingThread->isPreparing(true) && !pingThread->isWorking())) ){
		download=thePrefs.GetMaxDownload();
	}

	// Check connection state need for stand alone ability
	if(NeoPrefs.IsCheckAdapter() && NeoPrefs.IsCheckConnection())
		SetMissingConnection((!IsNAFCWorking()) && (!pingThread || !pingThread->isWorking() || pingThread->isTimeout()));
	else if(NeoPrefs.IsCheckAdapter())
		SetMissingConnection(!IsNAFCWorking());
	else if(NeoPrefs.IsCheckConnection())
		SetMissingConnection(!pingThread || !pingThread->isWorking() || pingThread->isTimeout());
	else 
		SetMissingConnection(false);

	return 0;
}

// NEO: NAFC - [NetworkAdapterFeatbackControl]
void CBandwidthControl::ReadNafc()
{
	if(NeoPrefs.IsNAFCEnabled()){
		// Retrieve the network flow
		if(IsNAFCWorking()){
			if(uPlatform == 2) // Win 2000
				CheckAdapterIP();
			MIB_IFROW ifRow;
			ifRow.dwIndex = m_currentAdapterIndex;
			if(GetIfEntry(&ifRow) == NO_ERROR){
				AdapterUp = ifRow.dwOutOctets;
				AdapterDown = ifRow.dwInOctets;
			}else{
				theApp.QueueDebugLogLine(false, _T("NAFC: Faild to get Data from Adapter, index may be wrong, trying to get new index..."));
				if(!SetAdapterIndex()){
					m_currentAdapterIndex = 0;
					theApp.QueueDebugLogLine(false, _T("NAFC: Faild to get new Adapter index, internet connection may be down..."));
				}
			}
		}else if(IsNAFCPreparing()){
			if(!SetAdapterIndex())
				m_currentAdapterIndex = 0;
		}else if(::GetTickCount() - m_LastGetAdapterTry > SEC2MS(10)){
			m_LastGetAdapterTry = ::GetTickCount();
			theApp.QueueDebugLogLine(false, _T("NAFC: Trying to get new Adapter Index..."));
			if(!SetAdapterIndex()){
				m_currentAdapterIndex = 0;
				theApp.QueueDebugLogLine(false, _T("NAFC: Faild to get new Adapter index, internet connection may be down..."));
			}
		}
	}else if(IsNAFCWorking()){
		m_currentAdapterIndex = 1; //StandBy reget adapter index on enable

		// NEO: MOD - [BindToAdapter]
		if(NeoPrefs.IsBindToAdapter())
			theApp.BindToAddress(); // NULL, means bind to all adapters...
		// NEO: MOD END
	}
}

// NEO: NAFC END

void CBandwidthControl::CalculateStats()
{
	// Calc data rates begin
	DWORD dwTime = ::GetTickCount();

	// Compute the High Precision Data Rates -->
	CalcAverages( eMuleUp, dwTime, 0, eMuleUpAvg,m_nArrayIndexHigh,AVE_TIMES);
	CalcAverages( eMuleDown, dwTime, 0, eMuleDownAvg,m_nArrayIndexHigh,AVE_TIMES);
	CalcAverages( eMuleUpData, dwTime, 0, eMuleUpDataAvg,m_nArrayIndexHigh,AVE_TIMES);
	CalcAverages( eMuleDownData, dwTime, 0, eMuleDownDataAvg,m_nArrayIndexHigh,AVE_TIMES);
	// NEO: NAFC - [NetworkAdapterFeatbackControl]
	//if(IsNAFCWorking()){
	//	CalcAverages( AdapterUp, dwTime, 0, AdapterUpAvg,m_nArrayIndexHigh,AVE_TIMES);
	//	CalcAverages( AdapterDown, dwTime, 0, AdapterDownAvg,m_nArrayIndexHigh,AVE_TIMES);
	//}
	// NEO: NAFC END

	eMuleUpRate = eMuleUpAvg[m_nArrayIndexHigh].ave/1024.0f;
	eMuleDownRate = eMuleDownAvg[m_nArrayIndexHigh].ave/1024.0f;
	eMuleUpDataRate = eMuleUpDataAvg[m_nArrayIndexHigh].ave/1024.0f;
	eMuleDownDataRate = eMuleDownDataAvg[m_nArrayIndexHigh].ave/1024.0f;
	// NEO: NAFC - [NetworkAdapterFeatbackControl]
	//if(IsNAFCWorking()){
	//	AdapterUpRate = AdapterUpAvg[m_nArrayIndexHigh].ave/1024.0f;
	//	AdapterDownRate = AdapterDownAvg[m_nArrayIndexHigh].ave/1024.0f;
	//}else{
	//	AdapterUpRate = 0.0f; 
	//	AdapterDownRate = 0.0f; 
	//}
	// NEO: NAFC END
	// <-- Compute the Data Rates done
	//increment the circular buffer index
	if( ++m_nArrayIndexHigh >= AVE_TIMES  )
		m_nArrayIndexHigh = 0;

	// Compute the Low Precision Data Rates -->
	CalcAverages( eMuleUp, dwTime, 0, eMuleUpStats,m_nArrayIndexLow,StatisticEntries);
	CalcAverages( eMuleDown, dwTime, 0, eMuleDownStats,m_nArrayIndexLow,StatisticEntries);
	CalcAverages( eMuleUpData, dwTime, 0, eMuleUpDataStats,m_nArrayIndexLow,StatisticEntries);
	CalcAverages( eMuleDownData, dwTime, 0, eMuleDownDataStats,m_nArrayIndexLow,StatisticEntries);
	// NEO: NAFC - [NetworkAdapterFeatbackControl]
	if(IsNAFCWorking()){
		CalcAverages( AdapterUp, dwTime, 0, AdapterUpStats,m_nArrayIndexLow,StatisticEntries);
		CalcAverages( AdapterDown, dwTime, 0, AdapterDownStats,m_nArrayIndexLow,StatisticEntries);
	}
	// NEO: NAFC END
 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
	CalcAverages( theStats.sessionSentBytesToRelease, dwTime, 0, ReleaseUpStats,m_nArrayIndexLow,StatisticEntries);
	CalcAverages( theStats.sessionSentBytesToFriend, dwTime, 0, FriendUpStats,m_nArrayIndexLow,StatisticEntries);
 #endif // BW_MOD // NEO: BM END

	eMuleStatsUpRate = eMuleUpStats[m_nArrayIndexLow].ave/1024.0f;
	eMuleStatsDownRate = eMuleDownStats[m_nArrayIndexLow].ave/1024.0f;
	eMuleStatsUpDataRate = eMuleUpDataStats[m_nArrayIndexLow].ave/1024.0f;
	eMuleStatsDownDataRate = eMuleDownDataStats[m_nArrayIndexLow].ave/1024.0f;
	// NEO: NAFC - [NetworkAdapterFeatbackControl]
	if(IsNAFCWorking()){
		AdapterStatsUpRate = AdapterUpStats[m_nArrayIndexLow].ave/1024.0f;
		AdapterStatsDownRate = AdapterDownStats[m_nArrayIndexLow].ave/1024.0f;
	}else{
		AdapterStatsUpRate = 0.0f; 
		AdapterStatsDownRate = 0.0f; 
	}
	// NEO: NAFC END
 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
	ReleaseStatsUpRate = ReleaseUpStats[m_nArrayIndexLow].ave/1024.0f;
	FriendStatsUpRate = FriendUpStats[m_nArrayIndexLow].ave/1024.0f;
 #endif // BW_MOD // NEO: BM END
	// <-- Compute the Data Rates done
	//increment the circular buffer index
	if( ++m_nArrayIndexLow >= StatisticEntries  )
		m_nArrayIndexLow = 0;

	//save the max values
	if (eMuleStatsUpDataRate > maxUpDataRate)
		maxUpDataRate = eMuleStatsUpDataRate;
	if (eMuleStatsDownDataRate > maxDownDataRate)
		maxDownDataRate = eMuleStatsDownDataRate;
}

bool CBandwidthControl::MissingConnection()
{
	return (m_uMissingConnection && (::GetTickCount() - m_uMissingConnection > SEC2MS(30)));
}

void CBandwidthControl::SetMissingConnection(bool bMissing)
{
	if(bMissing == false) 
		m_uMissingConnection = 0; 
	else if(m_uMissingConnection == 0) 
		m_uMissingConnection = ::GetTickCount(); 
	else if(::GetTickCount() - m_uMissingConnection > SEC2MS(20))
		if(theApp.IsPublicIP() && !theApp.serverconnect->IsConnected()) // When we have an connection wie can't be offline
			theApp.SetPublicIP(0);
}

// Speed Sense: USS; DSS
void CBandwidthControl::CalculateSpeedSense()
{
	if (pingThread && (pingThread->isPreparing(true) || pingThread->isWorking())){
		if (pingThread->isPreparing(true)){
			if (!useUploadTemp){
				useUploadTemp = true;
				uploadTemp = upload;
			}
			upload = (theApp.uploadqueue->GetActiveUploadsCount()) * 0.5f;
			if (upload < 1.0f) upload = 1.0f;

			if (!useDownloadTemp){
				useDownloadTemp = true;
				downloadTemp = download;
			}
			download = theApp.downloadqueue->GetDownloadQueueLength() * 0.5f;
			if (download < 1.0f) download = 1.0f;
		}
		else if (pingThread->isWorking()){
			pingVal ping = pingThread->GetPing();

			if (NeoPrefs.GetUpMaxPingMethod() == 0)
				base_ping_up = (float)NeoPrefs.GetBasePingUp();
			else if (NeoPrefs.GetUpMaxPingMethod() == 1 && (pingUpTolerance != NeoPrefs.GetPingUpTolerance() || ping.lowest_changed))
				ReCalculateBasePing(ping.lowest_ping, 1);
			else if (NeoPrefs.GetUpMaxPingMethod() == 2)
				base_ping_up = ping.lowest_ping * NeoPrefs.GetPingUpProzent() / 100;
			
			if (NeoPrefs.GetDownMaxPingMethod() == 0)
				base_ping_down = (float)NeoPrefs.GetBasePingDown();
			if (NeoPrefs.GetDownMaxPingMethod() == 1 && (pingDownTolerance != NeoPrefs.GetPingDownTolerance() || ping.lowest_changed))
				ReCalculateBasePing(ping.lowest_ping, 2);
			else if (NeoPrefs.GetDownMaxPingMethod() == 2)
				base_ping_down = ping.lowest_ping * NeoPrefs.GetPingDownProzent() / 100;

			//Upload --->
			float maxDiff = 0.25f / log10f(ping.lowest_ping + 3);
			if (NeoPrefs.IsUSSEnabled()) {
				if (base_ping_up < 1.0f)
					base_ping_up = 1.0f;
				//Bandwidth controlled by ping
				if (ping.changed){
					if (useUploadTemp){
						useUploadTemp = false;
						upload = uploadTemp;
					}

					// ZZ:UploadSpeedSense -->
					float normalized_ping = ping.avg_ping - ping.lowest_ping;
					float hping = base_ping_up - normalized_ping;
					if (ping.success == false)
						hping = - 2 * (base_ping_up + ping.lowest_ping);

					acceptNewClient = (hping >= 0);

					// Calculate change of upload speed
					float ulDiff = 0.0f;
					if (NeoPrefs.IsDynUpGoingDivider()) {
						if (hping > 0)
							ulDiff = hping * 10.0f / (ping.lowest_ping * NeoPrefs.GetDynUpGoingUpDivider());
						else
							ulDiff = hping * 10.0f / (ping.lowest_ping * NeoPrefs.GetDynUpGoingDownDivider());
					} 
					else {
						float multiplier = abs(hping / base_ping_up);
						if (multiplier > 1.0f){
							hping /= multiplier;
							if (multiplier > 2.0f)
								hping *= (log10f(multiplier)/0.301f);
						}
						ulDiff = hping / (ping.lowest_ping*100.0f);
					}

					if (hping > 0) {
						if (ulDiff > maxDiff)
							ulDiff = maxDiff;
					}
					else {
						if (ulDiff < -maxDiff)
							ulDiff = -maxDiff;
					}

					float additionalUp = (UPLOAD_CLIENT_DATARATE + eMuleUpRate*20.5f)/1024.0f;
					if (additionalUp > 10.0f)
						additionalUp = 10.0f;
					if (ulDiff < 0 || eMuleUpRate + additionalUp > upload) //Don't increase when upload level is not reached
						upload += ulDiff;
					if (eMuleUpRate + additionalUp < upload)
						upload = eMuleUpRate + additionalUp;
					// ZZ:UploadSpeedSense <--
				}
			}
			//Upload <---

			//Download --->
			if (NeoPrefs.IsDSSEnabled()) {
				if (base_ping_down < 1.0f)
					base_ping_down = 1.0f;
				//Bandwidth controlled by ping
				if (ping.changed){
					if (useDownloadTemp){
						useDownloadTemp = false;
						download = downloadTemp;
					}

					// DownloadSpeedSense -->
					float normalized_ping = ping.avg_ping - ping.lowest_ping;
					float hping = base_ping_down - normalized_ping;
					if (ping.success == false)
						hping = - 2 * (base_ping_down + ping.lowest_ping);

					// Calculate change of upload speed
					float downDiff = 0;
					if (NeoPrefs.IsDynDownGoingDivider()) {
						if (hping > 0)
							downDiff = hping * 20.0f / (ping.lowest_ping * NeoPrefs.GetDynDownGoingUpDivider());
						else
							downDiff = hping * 20.0f / (ping.lowest_ping * NeoPrefs.GetDynDownGoingDownDivider());
					}
					else {
						float multiplier = abs(hping / base_ping_down);
						if (multiplier > 1.0f){
							hping /= multiplier;
							if (multiplier > 2.0f)
								hping *= (log10f(multiplier)/0.2f);
						}

						downDiff = hping / (ping.lowest_ping * 64.0f);
					}

					if (hping > 0) {
						if (downDiff > 2 * maxDiff)
							downDiff = 2 * maxDiff;
					}
					else {
						if (downDiff < -2 * maxDiff)
							downDiff = -2 * maxDiff;
					}

					float additionalDown = (UPLOAD_CLIENT_DATARATE + maxDownDataRate*100.0f)/1024.0f;
					if (additionalDown > 10.0f)
						additionalDown = 10.0f;
					if (downDiff < 0 || eMuleDownRate + additionalDown > download) //Don't increase when download level is not reached
						download += downDiff;
					// DownloadSpeedSense <--
				}
			}
			//Download <---

			if (ping.changed){
				avg_ping = ping.avg_ping;
				lowest_ping = ping.lowest_ping;
			}
		}

		if (NeoPrefs.IsUSSEnabled()){
			// Do not fall under the minUpload allowed
			if(upload < NeoPrefs.GetMinBCUpload() && !useUploadTemp){
				minUpReached = true;
				upload = NeoPrefs.GetMinBCUpload();
			}
			else {
				minUpReached = false;
				if (upload > NeoPrefs.GetMaxBCUpload() && !NeoPrefs.IsAutoMaxUpload())
					upload = NeoPrefs.GetMaxBCUpload();
			}
		}

		if (NeoPrefs.IsDSSEnabled()){
			// Do not fall under the minDownload allowed
			if(download < NeoPrefs.GetMinBCDownload() && !useDownloadTemp)
				download = NeoPrefs.GetMinBCDownload();
			else if (download > NeoPrefs.GetMaxBCDownload() && !NeoPrefs.IsAutoMaxDownload())
				download = NeoPrefs.GetMaxBCDownload();
		}

	}
}

void CBandwidthControl::ReCalculateBasePing(float lowestPing, int UpDown)
{
	if (lowestPing < 0.1f) 
		lowestPing = 0.1f;
	else if (lowestPing > 2000.0f)
		lowestPing = 2000.0f;

	//float multiplier = 3.8f / log10f(8 + lowestPing);
	//float bestPing = 2.1f * (lowestPing * 14.5f / (log10f(lowestPing + 3)*log10f(lowestPing + 15))) / (multiplier + 1);
	float multiplier = 4.0f / log10f(8 + lowestPing);

	if(UpDown != 2){
		pingUpTolerance = (float)NeoPrefs.GetPingUpTolerance();
		float bestPingUp = lowestPing * 13.65f / (log10f(lowestPing + 3)*log10f(lowestPing + 4));
		if (pingUpTolerance < 12)
			base_ping_up = bestPingUp - ((bestPingUp - (bestPingUp/multiplier)) * (12.0f - pingUpTolerance)/11.0f);
		else if (pingUpTolerance > 12)
			base_ping_up = bestPingUp + (((bestPingUp*multiplier) - bestPingUp) * (pingUpTolerance - 12.0f)/12.0f);
		else
			base_ping_up = bestPingUp;
	}

	if(UpDown != 1){
		pingDownTolerance = (float)NeoPrefs.GetPingDownTolerance();
		float bestPingDown = (lowestPing * 13.65f / (log10f(lowestPing + 3)*log10f(lowestPing + 4))) * (multiplier/3.0f + 1.0f);
		if (pingDownTolerance < 12)
			base_ping_down = bestPingDown - ((bestPingDown - (bestPingDown/multiplier)) * (12.0f - pingDownTolerance)/11.0f);
		else if (pingDownTolerance > 12)
			base_ping_down = bestPingDown + (((bestPingDown*multiplier) - bestPingDown) * (pingDownTolerance - 12.0f)/12.0f);
		else
			base_ping_down = bestPingDown;
	}
}
// Speed Sense: USS; DSS end

bool CBandwidthControl::SetPingedServer(CString pingServer){
	bool retVal=true;
	if (pingThread){
		pingThread->StopPingThread();
		if (!pingThread->SetPingedServer(pingServer))
			retVal=false;
		pingThread->StartPingThread();
	}
	return retVal;
}

// NEO: NAFC - [NetworkAdapterFeatbackControl]
void CBandwidthControl::CalculateNAFC()
{
	if(IsNAFCWorking()){

		// Elapsed time
		uint32 deltaTime = ::GetTickCount() - m_lastProcessTime;
		m_lastProcessTime += deltaTime;
		if(deltaTime == 0)
			return;

		if(NeoPrefs.IsNAFCUpload()){	

			// Update the slope
			float maxUpload = NeoPrefs.GetMaxUpStream();
			m_nUploadSlopeControl += (uint32)(maxUpload * 1.024f * (float)deltaTime); // [bytes/period]

			// Correct the slope with the bytes sent during the last cycle
			m_nUploadSlopeControl -= (sint32) (NeoPrefs.IsNAFCEnabled() ? (AdapterUp - LastAdapterUp) 
																		: (eMuleUpData - LastTotalUpData));
			// Emule overhead
			sint32 overhead = (sint32) ((eMuleUp - LastTotalUp) - (eMuleUpData - LastTotalUpData));

			// Save old values
			LastTotalUp = eMuleUp;
			LastTotalUpData = eMuleUpData;
			LastAdapterUp = AdapterUp;

			// Compensate up to 1 second
			if(m_nUploadSlopeControl > (sint32)(1024.0f * maxUpload)){
				m_nUploadSlopeControl = (sint32)(1024.0f * maxUpload);
				// But be sure to be able to send at least one packet
				//if(m_nUploadSlopeControl < (GetMSS() - 40)) // TCP+IP headers
				//	m_nUploadSlopeControl = GetMSS() - 40;
			}

			// Trunk negative value (up to 0.5 second) 
			// => possible when Overhead compensation activated
			if(m_nUploadSlopeControl < (sint32)(-0.5 * 1024.0f * maxUpload))
				m_nUploadSlopeControl = (sint32)(-0.5 * 1024.0f * maxUpload);

			// Anticipate overhead (80%)
			sint32 nUploadSlopeControl = m_nUploadSlopeControl;
			if(!NeoPrefs.IsIncludeOverhead() && NeoPrefs.IsAutoMaxUpload()){
				sint32 overhead80 = 8 * overhead / 10;
				if(overhead80 > 0)                
					nUploadSlopeControl -= overhead80;
			}

			// Correct datarate with Ratio between period/elapsed time 
			// => wrong: if is the CPU is overloaded
			//           e.g. deltaTime = 200 [ms] instead of 100 [ms] => datarate = 1/2 of settings
			// float datarate = (float)nUploadSlopeControl/((float)deltaTime/TIMER_PERIOD); 

			// Reserve 20% of the upload (min 1KB) for the ctrl packets
			// Remark: With the previous versions of eMule, the bandwidth control  
			//         applied only to the data packets, but not the ctrls
			float datarate = (nUploadSlopeControl > (0.2f * 1024.0f) * maxUpload) ? 
							(float)nUploadSlopeControl : ((0.2f * 1024.0f) * maxUpload);
			if(datarate < 1024)
				datarate = 1024;

			// Set new 'instantanious' data rate
			upload = datarate/1024;

			//#ifdef _DEBUG
			//	sint32 overhead80 = 8 * overhead / 10;
			//	TRACE(_T("Upload: Time=%d, overhead=%d, ov80%%=%d, sc=%d, sc80=%d, datarate=%g\n"), 
			//			deltaTime, 
			//			overhead, 
			//			overhead80,
			//			m_nUploadSlopeControl,
			//			nUploadSlopeControl,
			//			datarate);
			//#endif  

			// Do not fall under the minUpload allowed
			if(upload < NeoPrefs.GetMinBCUpload()){
				minUpReached = true;
				upload = NeoPrefs.GetMinBCUpload();
			} else {
				minUpReached = false;
				if (upload > NeoPrefs.GetMaxBCUpload() && !NeoPrefs.IsAutoMaxUpload())
					upload = NeoPrefs.GetMaxBCUpload();
			}
		}

		if(NeoPrefs.IsNAFCDownload()){

			// Update the slope
			float maxDownload = NeoPrefs.GetMaxDownStream();
			m_nDownloadSlopeControl += (uint32)(maxDownload * 1.024f * (float)deltaTime); // [bytes/period]

			// Correct the slope with the bytes sent during the last cycle
			m_nDownloadSlopeControl -= (sint32) (NeoPrefs.IsNAFCEnabled() ? (AdapterDown - LastAdapterDown) 
																		: (eMuleDownData - LastTotalDownData));
			// Emule overhead
			sint32 overhead = (sint32) ((eMuleDown - LastTotalDown) - (eMuleDownData - LastTotalDownData));

			// Save old values
			LastTotalDown = eMuleDown;
			LastTotalDownData = eMuleDownData;
			LastAdapterDown = AdapterDown;

			// Compensate up to 1 second
			if(m_nDownloadSlopeControl > (sint32)(1024.0f * maxDownload)){
				m_nDownloadSlopeControl = (sint32)(1024.0f * maxDownload);
				// But be sure to be able to send at least one packet
				//if(m_nDownloadSlopeControl < (GetMSS() - 40)) // TCP+IP headers
				//	m_nDownloadSlopeControl = GetMSS() - 40;
			}

			// Trunk negative value (up to 0.5 second) 
			// => possible when Overhead compensation activated
			if(m_nDownloadSlopeControl < (sint32)(-0.5 * 1024.0f * maxDownload))
				m_nDownloadSlopeControl = (sint32)(-0.5 * 1024.0f * maxDownload);

			// Anticipate overhead (80%)
			sint32 nDownloadSlopeControl = m_nDownloadSlopeControl;
			if(!NeoPrefs.IsIncludeOverhead() && NeoPrefs.IsAutoMaxDownload()){
				sint32 overhead80 = 8 * overhead / 10;
				if(overhead80 > 0)                
					nDownloadSlopeControl -= overhead80;
			}

			// Correct datarate with Ratio between period/elapsed time 
			// => wrong: if is the CPU is overloaded
			//           e.g. deltaTime = 200 [ms] instead of 100 [ms] => datarate = 1/2 of settings
			// float datarate = (float)nDownloadSlopeControl/((float)deltaTime/TIMER_PERIOD); 

			// Reserve 20% of the download (min 1KB) for the ctrl packets
			// Remark: With the previous versions of eMule, the bandwidth control  
			//         applied only to the data packets, but not the ctrls
			float datarate = (nDownloadSlopeControl > (0.2f * 1024.0f) * maxDownload) ? 
							(float)nDownloadSlopeControl : ((0.2f * 1024.0f) * maxDownload);
			if(datarate < 1024)
				datarate = 1024;

			// Set new 'instantanious' data rate
			download = datarate/1024;

			//#ifdef _DEBUG
			//	sint32 overhead80 = 8 * overhead / 10;
			//	TRACE(_T("Downlaod: Time=%d, overhead=%d, ov80%%=%d, sc=%d, sc80=%d, datarate=%g\n"), 
			//			deltaTime, 
			//			overhead, 
			//			overhead80,
			//			m_nDownloadSlopeControl,
			//			nDownloadSlopeControl,
			//			datarate);
			//#endif 

			// Do not fall under the minDownload allowed
			if(download < NeoPrefs.GetMinBCDownload())
				download = NeoPrefs.GetMinBCDownload();
			else if (download > NeoPrefs.GetMaxBCDownload() && !NeoPrefs.IsAutoMaxDownload())
				download = NeoPrefs.GetMaxBCDownload();
		}

	}
}

PMIB_IPADDRTABLE GetAddrTable(){
/*#ifdef _DEBUG
	// enumerate all interfaces
	PMIB_IFTABLE pAdapterTable;
	DWORD dwTableSize = 0;

	// get adapter table size
	if(GetIfTable(NULL, &dwTableSize, true) != ERROR_INSUFFICIENT_BUFFER)
		return 0;

	// get adapter table
	pAdapterTable = (PMIB_IFTABLE)new char[dwTableSize];
	if(GetIfTable(pAdapterTable, &dwTableSize, true) != NO_ERROR){
		delete pAdapterTable;
		return 0;
	}

	// Trace list of Adapters
	for(DWORD dwNumEntries = 0; dwNumEntries < pAdapterTable->dwNumEntries; dwNumEntries++){
		const MIB_IFROW& mibIfRow = pAdapterTable->table[dwNumEntries];
		theApp.QueueDebugLogLine(false, _T("NAFC: Adapter %u is '%s'"), mibIfRow.dwIndex, (CString)mibIfRow.bDescr);
	}

	delete pAdapterTable;
#endif*/

	// enumerate all interfaces
	PMIB_IPADDRTABLE pIPAddrTable;
	DWORD dwSize = 0;

	// get address table size
	if (GetIpAddrTable(NULL, &dwSize, 0) != ERROR_INSUFFICIENT_BUFFER)
		return NULL;

	// get address table
	pIPAddrTable = (PMIB_IPADDRTABLE)new char[dwSize];
	if (GetIpAddrTable(pIPAddrTable, &dwSize, 0) != NO_ERROR) {
		delete [] pIPAddrTable;
		return NULL;
	}

	return pIPAddrTable;
}

BOOL CBandwidthControl::SetAdapterIndex(DWORD dwIP){

	// Prepare adapter choice
	DWORD wdMask = 0xFFFFFFFF; // Exact IP
	if(dwIP == 0) {
		if(NeoPrefs.IsISPCustomIP()){
			dwIP = NeoPrefs.GetISPZone();
			wdMask = NeoPrefs.GetISPMask();
		}else{
			// Retrieve the default used IP (=> in case of multiple adapters)
			char hostName[256];
			if(gethostname(hostName, sizeof(hostName)) != 0)
				return FALSE;

			hostent* lphost = gethostbyname(hostName);
			if(lphost == NULL)
				return FALSE;

			dwIP = ((LPIN_ADDR)lphost->h_addr)->s_addr;
		}
	}

	PMIB_IPADDRTABLE pIPAddrTable = GetAddrTable();
	if(pIPAddrTable == NULL)
		return FALSE;

	// Pick the interface matching the IP/Zone
	DWORD dwAddr;
	for(DWORD i = 0; i < pIPAddrTable->dwNumEntries; i++){
		dwAddr = pIPAddrTable->table[i].dwAddr;
		if((dwIP & wdMask) == (dwAddr & wdMask)){
			m_currentAdapterAdress = dwAddr;
			m_currentAdapterIndex = pIPAddrTable->table[i].dwIndex;
			if(m_currentAdapterIndex == 1)
				return FALSE;
			delete pIPAddrTable;
			MIB_IFROW ifRow;
			ifRow.dwIndex = m_currentAdapterIndex;
			if(GetIfEntry(&ifRow) != NO_ERROR)
				return FALSE;
			m_iMSS = ifRow.dwMtu - 40; // get the MSS = MTU - 40
			CString strAddr = ipstr(m_currentAdapterAdress);
			ifRow.bDescr[ifRow.dwDescrLen] = 0;
			ModLog(GetResString(IDS_X_NAFC_SELECT), CString(ifRow.bDescr), strAddr);
			if(NeoPrefs.IsCheckAdapter()){
				theApp.SetPublicIP(dwAddr);
				// NEO: RIC - [ReaskOnIDChange]
				if(NeoPrefs.IsCheckIPChange())
					theApp.CheckIDChange(dwAddr,1);
				// NEO: RIC END
			}
			// NEO: MOD - [BindToAdapter]
			if(NeoPrefs.IsBindToAdapter())
				theApp.BindToAddress(strAddr);
			// NEO: MOD END
			return TRUE; // success
		}
	}

	delete pIPAddrTable;
	return FALSE;
}

CStringList* CBandwidthControl::GetAdapterList()
{
	PMIB_IPADDRTABLE pIPAddrTable = GetAddrTable();
	if(pIPAddrTable == NULL)
		return NULL;

	CStringList* list = new CStringList;
	for(DWORD i = 0; i < pIPAddrTable->dwNumEntries; i++)
		list->AddTail(ipstr(pIPAddrTable->table[i].dwAddr));

	delete pIPAddrTable;
	return list;
}

// Need only for Windows 2000
void CBandwidthControl::CheckAdapterIP()
{
	static uint32 LastCheck = ::GetTickCount();
	if(::GetTickCount() - LastCheck < SEC2MS(3))
		return;
	LastCheck = ::GetTickCount();

	PMIB_IPADDRTABLE pIPAddrTable = GetAddrTable();
	if(pIPAddrTable == NULL){
		m_currentAdapterIndex = 0;
		return;
	}

	for(DWORD i = 0; i < pIPAddrTable->dwNumEntries; i++){
		if(pIPAddrTable->table[i].dwIndex != m_currentAdapterIndex)
			continue;

		if(pIPAddrTable->table[i].dwAddr != m_currentAdapterAdress)
			if(!SetAdapterIndex(theApp.GetPublicIP())){
				m_currentAdapterIndex = 0;
				theApp.QueueDebugLogLine(false, _T("NAFC: Faild to get proper Adapter index, internet connection may be down..."));
			}
	}

	delete pIPAddrTable;
}

// NEO: NAFC END

void CBandwidthControl::CalcAverages( uint64 dbTotal, DWORD dwTime, DWORD dwBps, CArray<STATS_STRUCT>& pStats, UINT& ArrayIndex, UINT EntryCount)
{
	ASSERT(ArrayIndex < EntryCount);

	pStats[ArrayIndex].Bps = dwBps; //set current bps
	pStats[ArrayIndex].total = dbTotal; //set total bytes sent or recv
	pStats[ArrayIndex].time = dwTime; //set the current time (milliseconds)

	int start = ArrayIndex - (EntryCount-1); //start is the index in the array for calculating averages
	if( start < 0 )
		start = EntryCount + start;

	if( pStats[start].time == 0 ) //the array entry may not have been filled in yet
		start = 0;

	uint64 dbSampleTotal = 0; //set average based upon sampling window size

	if( pStats[start].time ) { //total bytes received/sent in our sampling window
		if (dbTotal >= pStats[start].total)
			dbSampleTotal = dbTotal - pStats[start].total;
		else {
			dbSampleTotal = ((uint64)-1) - pStats[start].total;
			dbSampleTotal += dbTotal;
		}
	}

	DWORD dwElapsed = ( pStats[ArrayIndex].time - pStats[start].time ); //elapsed milliseconds

	if( dwElapsed ) //calc average
		pStats[ArrayIndex].ave = (uint32)(dbSampleTotal*1000i64/dwElapsed);
}

float CBandwidthControl::GetAverages(CList<float,float> &average, float value, int len)
{
	//if(value == UNLIMITED)
	//	return 0.0F;
	average.AddTail(value);
	if(average.GetCount()>len)
		average.RemoveHead();

	float total = 0.0F;
	for(POSITION pos = average.GetHeadPosition(); pos != NULL; total += average.GetNext(pos));

	return total/average.GetCount();
}

float CBandwidthControl::GetCurrStatsUploadLimit()
{
	static CList<float,float> eMuleUpLimitAvg;
	return NeoPrefs.SmoothStatisticsGraphs() ? GetAverages(eMuleUpLimitAvg,GetMaxUpload()) : GetMaxUpload();
}
float CBandwidthControl::GetCurrStatsDownloadLimit()
{
	static CList<float,float> eMuleDownLimitAvg;
	return NeoPrefs.SmoothStatisticsGraphs() ? GetAverages(eMuleDownLimitAvg,GetMaxDownload()) : GetMaxDownload();
}
float CBandwidthControl::GetCurrStatsPing()
{
	if(!lowest_ping)
		return 0.0F;
	static CList<float,float> eMulePingAvg;
	float cPing = ((avg_ping/lowest_ping) * 10) /*- 10*/;
	float aPing = GetAverages(eMulePingAvg,cPing,10);
	return NeoPrefs.SmoothStatisticsGraphs() || (cPing > aPing*3) ? aPing : cPing;
}

uint32 CalcIPOverhead(uint32 len, bool bUDP, bool bSolid){
	if(bUDP)
		return 20 + (((len/1300)+((len%1300)?1:0)) * 8); // IP + UDP

	if(bSolid)
		return 20 + (((len/1300)+((len%1300)?1:0)) * 20); // IP + TCP

	// Generaly eMule try to send two fragments at once to share a ACK, 
	// this can generaly not happen for control data.
	// Due to this the counting of ACK packets for file data, is a bit complicated...
	uint32 frags = (((len-1)/1300/*MAXFRAGSIZE*/)+1); // Fragments
	uint32 dfrags = frags/2; // Double Fragments
	uint32 packets = dfrags + (((dfrags * 2) == frags) ? 0 : 1); // Packets

	return packets * (20 + 20); 
}

void CBandwidthControl::AddUpIP(bool bUDP){
	uint32 oh = CalcIPOverhead(0,bUDP);
	if (NeoPrefs.IsConnectionsOverHead())
		theApp.uploadBandwidthThrottler->DecreaseToSend(oh);

	eMuleUp += oh;
}

void CBandwidthControl::AddUp(uint32 up, bool bUDP, bool fileData, bool bSolid){
	eMuleUp += up;
	if (fileData)
		eMuleUpData += up;

	if(NeoPrefs.IsIncludeTCPAck()){
		uint32 oh = CalcIPOverhead(up, bUDP, bSolid);
		eMuleUp += oh;
		if(!bUDP) 
			eMuleDown += oh;
		if(NeoPrefs.IsIncludeOverhead()){
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler]
			theApp.uploadBandwidthThrottler->DecreaseToSend(oh);
#endif // NEO_UBT // NEO: NUBT END
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler]
			if(!bUDP) 
				theApp.downloadBandwidthThrottler->DecreaseToReceive(oh);
#endif // NEO_DBT // NEO: NDBT END
		}
	}
}

void CBandwidthControl::AddDownIP(bool bUDP){
	uint32 oh = CalcIPOverhead(0,bUDP);
	if (NeoPrefs.IsConnectionsOverHead())
		theApp.downloadBandwidthThrottler->DecreaseToReceive(oh);

	eMuleDown += oh;
}

void CBandwidthControl::AddDown(uint32 down, bool bUDP, bool fileData, bool bSolid){
	eMuleDown += down;
	if (fileData)
		eMuleDownData += down;

	if(NeoPrefs.IsIncludeTCPAck()){
		uint32 oh = CalcIPOverhead(down, bUDP, bSolid);
		eMuleDown += oh;
		if(!bUDP) 
			eMuleUp += oh;
		if(NeoPrefs.IsIncludeOverhead()){
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler]
			theApp.downloadBandwidthThrottler->DecreaseToReceive(oh);
#endif // NEO_DBT // NEO: NDBT END
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler]
			if(!bUDP) 
				theApp.uploadBandwidthThrottler->DecreaseToSend(oh);
#endif // NEO_UBT // NEO: NUBT END
		}
	}
}

UINT CBandwidthControl::GetMSS()
{
	return (NeoPrefs.IsAutoMSS() && m_iMSS) ? m_iMSS : NeoPrefs.GetMSS();
}

bool CBandwidthControl::AcceptNewClient()
{
	return acceptNewClient || !NeoPrefs.IsUSSEnabled();
}

float CBandwidthControl::GetMaxUpload()
{
	return upload;
}

/*float CBandwidthControl::GetMaxDownload()
{
	if( upload < 4 )
		return (( (upload < 10) && (upload*3 < download) )? upload*3 : download);
	return (( (upload < 10) && (upload*4 < download) )? upload*4 : download);
}*/

/* Xanatos: 
* The today common Internet connections have speeds much higher then the old 512/128 ADSL's
* And the ADSL2 with 12288/1024 kbit/s is comming, the old 1:4 ratio unlimited above 10
* Is do to this facts not longer tolerable, it is not OK when users gove only 80 kbit/s (10 kb/s)
* and can download lets say 6000 kbit/s, for this reasons, I decided to extend the current ratio.
* For low speed it will be simmilar to the old one, but the speed will be unlimited
* only above 800 kbit/s (100 kb/s). The ratio will rise up to 1:10 for 400 kbit/s (50 kb/s)
* the highest upload of ADSL1 is 512 kbit/s (64 kb/s) the highest download is 5120 kbit/s
* the idea is that users will use thair complete bandwidth. Thats the same that the old ratio did.
* The session ratio will use the new dynamic ratio modifyer.
*
* And thats the formula for the new ratio: 4*log10(MaxUpload)^1.8 = Ratio
* MaxDownload = Ratio * MaxUpload
* Some Examples:
*  4*log10(5)^1.8  = 1:2
*  4*log10(10)^1.8 = 1:4
*  4*log10(20)^1.8 = 1:6
*  4*log10(30)^1.8 = 1:8
*  4*log10(40)^1.8 = 1:9
*  4*log10(50)^1.8 = 1:10
*
*/

float CBandwidthControl::GetMaxDownload()
{
	float tempUploadspeed = upload;

	float MaxRatio = 4.0F;
	float Unlimit = 10.0F;

 #ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	if(NeoPrefs.IsVoodooEnabled()){ // Use of the dynamic ratio when voodoo is enabled
		const float Dynamic = log10(tempUploadspeed);
		MaxRatio = 4.0F * pow(max(Dynamic,0.5F), 1.8F); // Dynamic ul/dl ratio
		Unlimit = 100.0F;
	}
	else
 #endif // VOODOO // NEO: VOODOO END
	{
		MaxRatio = (tempUploadspeed < 4.0F) ? 3.0F : (tempUploadspeed < 10.0F) ? 4.0F : 5.0F; // old Ratio 
		Unlimit = 10.0F;
	}

	// NEO: NMFS - [NiceMultiFriendSlots]
	UINT SessioRatio = IsSessionRatio();
	if(SessioRatio > 1){
		MaxRatio = 3; // 1:3 ZZ ratio for friend slot usage
		Unlimit = 1000.0F; // ~ no unlimit
	}
	// NEO: NMFS END

	float tempDownspeed;
	uint64 sessionSentBytes, sessionReceivedBytes;
	if (SessioRatio != 0 // NEO: NMFS - [NiceMultiFriendSlots]
	 && (sessionReceivedBytes = theStats.sessionReceivedBytes) != 0
	 && (sessionSentBytes = theStats.sessionSentBytes - theStats.sessionSentBytesToFriend) != 0 
	){
		if (sessionReceivedBytes > MaxRatio * sessionSentBytes && eMuleStatsUpDataRate < Unlimit
		 && theApp.uploadqueue->GetWaitingUserCount() > 50 && theApp.uploadqueue->GetUploadQueueLength())
		{
			float Ratio = (float)sessionReceivedBytes/sessionSentBytes;
			float MaxSmothRatio = MaxRatio + 0.05F;
			if (Ratio > MaxSmothRatio) // Smooth limit of download
				Ratio = MaxSmothRatio;
			tempDownspeed = MaxRatio * ((MaxSmothRatio - Ratio) / 0.05F * (eMuleStatsUpRate - eMuleStatsUpDataRate) + eMuleStatsUpDataRate) 
									* 16.0F * pow((float)sessionSentBytes/sessionReceivedBytes, 2);
			m_sessionratio = TRUE; // NEO: NMFS - [NiceMultiFriendSlots]
		}
		else{
			tempDownspeed = download;
			m_sessionratio = FALSE; // NEO: NMFS - [NiceMultiFriendSlots]
		}
	}else
		tempDownspeed = MaxRatio * tempUploadspeed;

	if(tempUploadspeed < Unlimit){
		if(tempDownspeed < 3.0F)
			tempDownspeed = 3.0F;
		if(tempDownspeed < download)
			return tempDownspeed;
	}
	return download;
}

// NEO: NMFS - [NiceMultiFriendSlots]
UINT CBandwidthControl::IsSessionRatio()
{
	UINT ret = 0;

	if (NeoPrefs.IsSessionRatio())
		ret |= 1;
	if(theApp.uploadqueue->GetFriendSlots() > (NeoPrefs.IsSeparateFriendBandwidth() ? 0 : 1))
		ret |= 2;

	return ret;
}
// NEO: NMFS END

#endif // NEO_BC // NEO: NBC END <-- Xanatos --