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
#include "ResizableLib\ResizableDialog.h"
#include "StatisticsTree.h"
#include "SplitterControl.h"
#include "OScopeCtrl.h"
#include "Preferences.h"
#ifdef CLIENTANALYZER
#include "Addons/AntiLeech/ClientAnalyzer.h" 
#endif

// NOTE: Do not set specific 'Nr. of sub client versions' per client type, current code contains too much hardcoded
// references to deal with that.
#define	MAX_CLIENTS_WITH_SUB_VERSION	4	// eMule, eDHyb, eD, aMule
#define	MAX_SUB_CLIENT_VERSIONS			8

//TK4 MOD +
#define	MP_RESET_OSCOPES	12000
#define	MP_STATISICS_PREF       12001
#define MP_HIDE_STREE           12002
#define MP_MAX_DL               12003
#define MP_MAX_UP               12004
#define MP_MAX_ST               12005
#define MP_HIDE_SCOPES          12006
//TK4 MOD -

class CStatisticsDlg : public CResizableDialog
{
	DECLARE_DYNAMIC(CStatisticsDlg)
public:
	CStatisticsDlg(CWnd* pParent = NULL);   // standard constructor
	~CStatisticsDlg();
	enum { IDD = IDD_STATISTICS };

	void Localize();
	//Xman
	//void SetCurrentRate(float uploadrate, float downloadrate);
	void ShowInterval();
	// -khaos--+++> Optional force update parameter.
	void ShowStatistics(bool forceUpdate = false);
	// <-----khaos-
	void SetARange(bool SetDownload,int maxValue);
	void RepaintMeters();
	//Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	void Process();
	// Maelle end
//TK4 MOD +
	virtual	BOOL OnCommand( WPARAM wParam, LPARAM lParam );//handle all extra messages
	void zoomScopeDl(void);
	void zoomScopeUl(void);
	void zoomScopeST(void);
	void maximizeScopeDl(void);
	void maximizeScopeUl(void);
	void maximizeScopeST(void);
	void hideStatsTree(void);
	void hideScopesView(void);
	void restoreScopeSizes(void);
	void resetScopeSizes(void);
//TK4 MOD -

	void	UpdateConnectionsGraph();
	void	DoTreeMenu();
	void	CreateMyTree();
	// -khaos--+++> New class for our humble little tree.
	CStatisticsTree stattree;
	// <-----khaos-

private:
    COScopeCtrl m_DownloadOMeter,m_UploadOMeter,m_Statistics;
	TOOLINFO tt;

	//Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	//enum Curve {CURRENT = 0, MINUTE = 1, SESSION = 2, OVERALL = 3, ADAPTER = 4};
	//Xman had to change it for filled graphs
	enum Curve {ADAPTER = 0, OVERALL = 1, MINUTE = 2, SESSION = 3, CURRENT = 4};
#define NUMBEROFLINES 4

	//double m_dPlotDataMore[4]; //Xman
	uint32 m_ilastMaxConnReached;

	uint32		cli_lastCount[MAX_CLIENTS_WITH_SUB_VERSION];
	CImageList	imagelistStatTree;
	HTREEITEM	h_transfer, trans[3]; // Transfer Header and Items
	HTREEITEM	h_upload, h_up_session, up_S[6], h_up_total, up_T[2]; // Uploads Session and Total Items and Headers
	HTREEITEM	hup_scb, up_scb[UP_SOFT_COUNT], hup_spb, up_spb[PORT_COUNT], hup_ssb, up_ssb[2]; // Session Uploaded Byte Breakdowns
	HTREEITEM	hup_tcb, up_tcb[UP_SOFT_COUNT], hup_tpb, up_tpb[PORT_COUNT], hup_tsb, up_tsb[2]; // Total Uploaded Byte Breakdowns
	HTREEITEM	hup_soh, up_soh[5 /*Xman +1 count obfuscation data*/], hup_toh, up_toh[4]; // Upline Overhead
	HTREEITEM	up_ssessions[4], up_tsessions[4]; // Breakdown of Upload Sessions
	HTREEITEM	h_download, h_down_session, down_S[8], h_down_total, down_T[6]; // Downloads Session and Total Items and Headers
	HTREEITEM	hdown_scb, down_scb[DOWN_SOFT_COUNT], hdown_spb, down_spb[PORT_COUNT]; // Session Downloaded Byte Breakdowns
	HTREEITEM	hdown_tcb, down_tcb[DOWN_SOFT_COUNT], hdown_tpb, down_tpb[PORT_COUNT]; // Total Downloaded Byte Breakdowns
	HTREEITEM	hdown_soh, down_soh[5 /*Xman +1 count obfuscation data*/], hdown_toh, down_toh[4]; // Downline Overhead
	HTREEITEM	down_ssessions[4], down_tsessions[4], down_sources[23 /*+1 Xman Xtreme Mod: Count failed tcp-connections */]; // Breakdown of Download Sessions and Sources
	HTREEITEM	h_connection, h_conn_session, h_conn_total; // Connection Section Headers
	HTREEITEM	hconn_sg, conn_sg[5], hconn_su, conn_su[4], hconn_sd, conn_sd[4]; // Connection Session Section Headers and Items
	HTREEITEM	hconn_tg, conn_tg[4], hconn_tu, conn_tu[3], hconn_td, conn_td[3]; // Connection Total Section Headers and Items
	HTREEITEM	h_clients, cligen[8/*6*Official+1*Mods+1*Country*/], hclisoft, clisoft[DOWN_SOFT_COUNT-1]; //Xman extended stats
	HTREEITEM	cli_versions[MAX_CLIENTS_WITH_SUB_VERSION*MAX_SUB_CLIENT_VERSIONS];
	HTREEITEM	cli_other[MAX_SUB_CLIENT_VERSIONS/2];
	HTREEITEM	hclinet, clinet[4]; // Clients Section
	HTREEITEM	hcliport, cliport[2]; // Clients Section
	HTREEITEM	hclifirewalled, clifirewalled[2]; // Clients Section
	HTREEITEM	h_servers, srv[6], srv_w[3], hsrv_records, srv_r[3]; // Servers Section
	HTREEITEM	h_shared, shar[4], hshar_records, shar_r[4]; // Shared Section
	// The time/projections section.  Yes, it's huge.
	HTREEITEM	h_time, tvitime[2], htime_s, tvitime_s[4], tvitime_st[2], htime_t, tvitime_t[3], tvitime_tt[2];
	HTREEITEM	htime_aap, time_aaph[3], time_aap_hup[3], time_aap_hdown[3];
	HTREEITEM	time_aap_up_hd[3][3], time_aap_down_hd[3][2];
	HTREEITEM	time_aap_up[3][3], time_aap_up_dc[3][UP_SOFT_COUNT], time_aap_up_dp[3][PORT_COUNT];
	HTREEITEM	time_aap_up_ds[3][2], time_aap_up_s[3][2], time_aap_up_oh[3][4];
	HTREEITEM	time_aap_down[3][7], time_aap_down_dc[3][DOWN_SOFT_COUNT], time_aap_down_dp[3][PORT_COUNT];
	HTREEITEM	time_aap_down_s[3][2], time_aap_down_oh[3][4];
#ifdef CLIENTANALYZER
	HTREEITEM	h_Analyzer, h_BadActions[BAD_ACTION_COUNTER];
#endif

	HTREEITEM h_total_downloads;
	HTREEITEM h_total_num_of_dls;
	HTREEITEM h_total_size_of_dls;
	HTREEITEM h_total_size_dld;
	HTREEITEM h_total_size_left_to_dl;
	HTREEITEM h_total_size_left_on_drive;
	HTREEITEM h_total_size_needed;

	void SetupLegend( int ResIdx, int ElmtIdx, int legendNr);
	void SetStatsRanges(int min, int max);
	//Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	void ShowGraphs();
	uint16 m_intervalGraph; // refresh counter for graphs
	uint16 m_intervalStat;  // refresh counter for statistic
	// Maella end
//TK4 MOD +
	bool paneMaximized; 
	bool statsTreeHidden;
	bool scopesViewHidden;
	float midY1;
	float midY2;
//TK4 MOD -

#ifdef _DEBUG
	HTREEITEM h_debug,h_blocks,debug1,debug2,debug3,debug4,debug5;
	CAtlMap<const unsigned char *,HTREEITEM *> blockFiles;
#endif
	HTREEITEM h_allocs;
	HTREEITEM h_allocSizes[32];

protected:
	void SetAllIcons();

	virtual BOOL OnInitDialog(); 
	virtual void OnSize(UINT nType,int cx,int cy);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support	

	//MORPH START - Added by SiRoB, Splitting Bar [O²]
	CSplitterControl m_wndSplitterstat; //bzubzusplitstat
	CSplitterControl m_wndSplitterstat_HL; //bzubzusplitstat
	CSplitterControl m_wndSplitterstat_HR; //bzubzusplitstat
	void DoResize_V(int delta);
	void DoResize_HL(int delta);
	void DoResize_HR(int delta);
	void initCSize();
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//MORPH END   - Added by SiRoB, Splitting Bar [O²]
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	bool	m_bTreepaneHidden;
	CTitleMenu	mnuContext; //TK4 MOD
	CToolTipCtrl* m_TimeToolTips;
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSysColorChange();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnMenuButtonClicked();
	afx_msg void OnStnDblclickScope();
	afx_msg LRESULT OnOscopePositionMsg(WPARAM wParam, LPARAM lParam);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
//TK4 MOD +
	afx_msg	void OnContextMenu( CWnd* pWnd, CPoint point );
	afx_msg LRESULT restoreScopes(WPARAM, LPARAM);
//TK4 MOD -
	//Xman
	// Maella -Network Adapter Feedback Control-
public:	
	int GetARange(bool SetDownload) const {return m_lastRange[(SetDownload==false)?0:1];}

private:
	int m_lastRange[2];
	// Maella end
};
