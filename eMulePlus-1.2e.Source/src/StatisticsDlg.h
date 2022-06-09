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

#include "ResizableLib\ResizableDialog.h"
#include "OScopeCtrl.h"
#include "StatisticsTree.h"
#include "ColorFrameCtrl.h"
#pragma warning(push)
#pragma warning(disable:4702) // unreachable code
#include <list>
#pragma warning(pop)

#define STAT_CLIENT_MOD_NODES		3

// CStatisticsDlg dialog

class CStatisticsDlg : public CResizableDialog
{
	DECLARE_DYNAMIC(CStatisticsDlg)

	friend class CWebServer;
	
public:
	CStatisticsDlg(CWnd* pParent = NULL);   // standard constructor
	~CStatisticsDlg();
	enum { IDD = IDD_STATISTICS };

	void	Localize();
	void	SetCurrentRate(uint32 dwUpRate, uint32 dwDownRate);
	void	ShowInterval();
	void	SetARange(bool SetDownload,int maxValue);
	void	RecordRate();
	double	GetAvgDownloadRate(int averageType);
	double	GetAvgUploadRate(int averageType);
	void	RepaintMeters();
	void	UpdateConnectionsStatus();

	uint32	GetActiveConnections()		{ return activeconnections; }
	uint32	GetPeakConnections()		{ return peakconnections; }
	uint32	GetTotalConnectionChecks()	{ return totalconnectionchecks; }
	double	GetAverageConnections()		{ return averageconnections; }
	
	uint32	GetTransferTime()			{ return timeTransfers + time_thisTransfer; }
	uint32	GetUploadTime()				{ return timeUploads + time_thisUpload; }
	uint32	GetDownloadTime()			{ return timeDownloads + time_thisDownload; }
	uint32	GetServerDuration()			{ return timeServerDuration + time_thisServerDuration; }
	void	Add2TotalServerDuration()	{ timeServerDuration += time_thisServerDuration;  time_thisServerDuration = 0; }
	void	UpdateActConScale();
	void	UpdateConnectionStats(double uploadrate, double downloadrate);
	void	CreateStatsTree();
	void	DoTreeMenu();
	void	ShowStatistics(bool forceUpdate = false);

	CString		GetUpEndReason(unsigned uiState);
	CStatisticsTree stattree;
	CImageList	m_imagelistStatTree;

private:
	COScopeCtrl m_DownloadOMeter, m_UploadOMeter, m_Statistics;
	CColorFrameCtrl m_Led1[3];
	CColorFrameCtrl m_Led2[3];
	CColorFrameCtrl m_Led3[3];

	double	maxDownavg;
	double	maxDown;
	uint32	m_ilastMaxConnReached;
	byte	m_byteStatGraphRatio;

//	Cumulative Stats
	double	cum_DL_maximal;		//maxcumDown
	double	cum_DL_average;		//Downavg
	double	cum_DL_max_average;	//maxcumDownavg

	double	cum_UL_maximal;		//maxcumUp
	double	cum_UL_average;		//cumUpavg
	double	cum_UL_max_average;	//maxcumUpavg
	
	double	maxUp;
	double	maxUpavg;
	double	rateDown;
	double	rateUp;
	uint32	timeTransfers;
	uint32	timeDownloads;
	uint32	timeUploads;
	uint32	start_timeTransfers;
	uint32	start_timeDownloads;
	uint32	start_timeUploads;
	uint32	time_thisTransfer;
	uint32	time_thisDownload;
	uint32	time_thisUpload;
	uint32	timeServerDuration;
	uint32	time_thisServerDuration;

//	Tree Declarations 
	HTREEITEM	h_transfer, trans[2]; // Transfer Header and Items
	HTREEITEM	h_upload, h_up_session, up_S[4], h_up_total, up_T[2]; // Uploads Session and Total Items and Headers
	HTREEITEM	hup_scb, up_scb[SO_LAST], hup_ssb, up_ssb[2], hup_scomb, up_scomb[2], hULPrioDataNode, hULPrioDataItems[5]; // Session Uploaded Byte Breakdowns
	HTREEITEM	hup_tcb, up_tcb[SO_LAST], hup_tsb, up_tsb[2], hup_tcomb, up_tcomb[2]; // Total Uploaded Byte Breakdowns
	HTREEITEM	hup_soh, up_soh[4], hup_toh, up_toh[4]; // Upline Overhead
	HTREEITEM	up_ssessions[4], up_ssessions_s[2],up_ssessions_spc[ETS_TERMINATOR],up_ssessions_f[ETS_TERMINATOR], up_tsessions[4]; // Breakdown of Upload Sessions
	HTREEITEM	h_download, h_down_session, down_S[17], h_down_total, down_T[6]; // Downloads Session and Total Items and Headers
	HTREEITEM	hdown_scb, down_scb[SO_LAST]; // Session Downloaded Byte Breakdowns
	HTREEITEM	hdown_tcb, down_tcb[SO_LAST]; // Total Downloaded Byte Breakdowns
	HTREEITEM	hdown_soh, down_soh[4], hdown_toh, down_toh[4]; // Downline Overhead
	HTREEITEM	down_ssessions[4], htiDLFailedSesNRD, down_tsessions[4], down_sources[STATS_DLSRC_COUNT - 1]; // Breakdown of Download Sessions and Sources
	HTREEITEM	h_connection, h_conn_session, h_conn_total; // Connection Section Headers
	HTREEITEM	hconn_sg, conn_sg[5], hconn_su, conn_su[4], hconn_sd, conn_sd[4]; // Connection Session Section Headers and Items
	HTREEITEM	hconn_tg, conn_tg[4], hconn_tu, conn_tu[3], hconn_td, conn_td[3]; // Connection Total Section Headers and Items
	HTREEITEM	h_clients, hcliconnected, cligen[5], hclisoft, clisoft[SO_LAST], cli_versions[SO_LAST][4], cli_versions_other[SO_LAST], hcliport, cliport[2];	// Clients Section
	HTREEITEM	hFiltered, hFilteredItems[3], hCountries; // Clients Section
	HTREEITEM	h_servers, srv[7], srv_w[3], hsrv_records, srv_r[3]; // Servers Section
	HTREEITEM	h_shared, shar[4], hshar_records, shar_r[4]; // Shared Section
	HTREEITEM	h_time, tvitime[2], htime_s, tvitime_s[4], tvitime_st[2], htime_t, tvitime_t[3], tvitime_tt[2];
	HTREEITEM	htime_aap, time_aaph[3], time_aap_hup[3], time_aap_hdown[3];
	HTREEITEM	time_aap_up_hd[3][3], time_aap_down_hd[3][2];
	HTREEITEM	time_aap_up[3][3], time_aap_up_dc[3][SO_LAST];
	HTREEITEM	time_aap_up_ds[3][2], time_aap_up_com[3][2], time_aap_up_s[3][2], time_aap_up_oh[3][4];
	HTREEITEM	time_aap_down[3][7], time_aap_down_dc[3][SO_LAST];
	HTREEITEM	time_aap_down_s[3][2], time_aap_down_oh[3][4];
	HTREEITEM	hclisoftMODs, hclisoftPlusMODs, clisoftMODs[STAT_CLIENT_MOD_NODES];

	uint32 m_nMODs[STAT_CLIENT_MOD_NODES];

	void	SetupLegend(int ResIdx, int ElmtIdx, int legendNr);
	CString	GetMODType(uint32 dwIndex);

//	Data lists (DL, UL & time)
	std::list<uint64> uprateHistory;
	std::list<uint64> downrateHistory;
	std::list<DWORD>timeHistory;

//	Connections
	uint32	activeconnections;
	uint32	peakconnections;
	uint32	totalconnectionchecks;
	double	averageconnections;
	
	int		m_oldcx;
	int		m_oldcy;
	
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	virtual void OnSize(UINT nType,int cx,int cy);
	afx_msg void OnMenuButtonClicked();
	afx_msg void OnDestroy();
};
