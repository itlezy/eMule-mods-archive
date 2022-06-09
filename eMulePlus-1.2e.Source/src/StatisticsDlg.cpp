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

#include "stdafx.h"
#include "emule.h"
#include "SharedFileList.h"
#include "StatisticsDlg.h"
#include "UploadQueue.h"
#include "otherfunctions.h"
#include "ServerList.h"
#include "WebServer.h"
#include "IP2Country.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(CStatisticsDlg, CDialog)
CStatisticsDlg::CStatisticsDlg(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CStatisticsDlg::IDD, pParent), m_DownloadOMeter(3), m_Statistics(3), m_UploadOMeter(3)
{
	m_oldcx = 0;
	m_oldcy = 0;

	//eklmn: add initial data in the list
	downrateHistory.push_front(0);
	uprateHistory.push_front(0);
	timeHistory.push_front(::GetTickCount());
}

CStatisticsDlg::~CStatisticsDlg()
{
	// v- eklmn: bugfix(06): resource cleanup in statistic
	if (m_imagelistStatTree) m_imagelistStatTree.DeleteImageList();
	if (stattree) stattree.DeleteAllItems();
	// ^- eklmn: bugfix(06): resource cleanup in statistic
}

void CStatisticsDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATTREE, stattree);
}


BEGIN_MESSAGE_MAP(CStatisticsDlg, CResizableDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BNMENU, OnMenuButtonClicked)
	ON_WM_DESTROY()	// eklmn: bugfix(00): resource cleanup due to CResizableDialog
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CStatisticsDlg message handlers
BOOL CStatisticsDlg::OnInitDialog()
{
	static const uint16 s_auIconResID[] =
	{
		IDI_TVI_GENERIC,		// Dots & Arrow (Default icon for stats)
		IDI_UP1DOWN1,			// Transfer
		IDI_CONNECTEDHIGH,		// Connection
		IDI_USERS,				// Clients
		IDI_PREF_SERVER,		// Server
		IDI_SHAREDFILES,		// Shared Files
		IDI_UPLOAD,				// Transfer > Upload
		IDI_DIRECTDOWNLOAD,		// Transfer > Download
		IDI_SMALLSTATISTICS,	// Session Sections
		IDI_TVI_CUMULATIVE,		// Cumulative Sections
		IDI_PREF_TWEAK,			// Records
		IDI_PREF_CONNECTION,	// Connection > General
		IDI_PREF_SCHEDULER,		// Time Section
		IDI_PREF_STATISTICS,	// Time > Averages and Projections
		IDI_TVI_DAY,			// Time > Averages and Projections > Daily
		IDI_TVI_MONTH,			// Time > Averages and Projections > Monthly
		IDI_TVI_YEAR			// Time > Averages and Projections > Yearly
	};

	EMULE_TRY

	CResizableDialog::OnInitDialog();
	EnableWindow(FALSE);

	stattree.Init();

	m_imagelistStatTree.Create(16, 16, g_App.m_iDfltImageListColorFlags | ILC_MASK, ARRSIZE(s_auIconResID), 0);
	FillImgLstWith16x16Icons(&m_imagelistStatTree, s_auIconResID, ARRSIZE(s_auIconResID));
	stattree.SetImageList(&m_imagelistStatTree, TVSIL_NORMAL);

	Localize();

	CRect rClient;
	GetClientRect(&rClient);

	m_DownloadOMeter.m_nYDecimals = 0;
	m_UploadOMeter.m_nYDecimals = 0;
	m_Statistics.m_nYDecimals = 0;
	int iStatYGrids=int((g_App.m_pPrefs->GetStatsMax()/10.0)+0.1)-1;
	if (iStatYGrids>10)
		iStatYGrids=int((g_App.m_pPrefs->GetStatsMax()/50.0)+0.1)-1;
	m_Statistics.m_nYGrids=iStatYGrids;

	// Setup download scope
	CRect rect;
	GetDlgItem(IDC_SCOPE_D)->GetWindowRect(rect);
	ScreenToClient(rect);
	m_DownloadOMeter.Create(WS_VISIBLE | WS_CHILD, rect, this);
	SetARange(true, g_App.m_pPrefs->GetMaxGraphDownloadRate() / 10);
	m_DownloadOMeter.SetYUnits(GetResString(IDS_KBYTESEC));
	//m_DownloadOMeter.SetXUnits(GetResString(IDS_TIME));
	CSize sDlMeter[2];
	sDlMeter[0].cx = 100 * rect.left / rClient.Width();
	sDlMeter[0].cy = 100 * rect.top / rClient.Height();
	sDlMeter[1].cx = 100 * rect.right / rClient.Width();
	sDlMeter[1].cy = 100 * rect.bottom / rClient.Height();

	// Setup upload scope
	GetDlgItem(IDC_SCOPE_U)->GetWindowRect(rect);
	ScreenToClient(rect);
	m_UploadOMeter.Create(WS_VISIBLE | WS_CHILD, rect, this);
	SetARange(false, g_App.m_pPrefs->GetMaxGraphUploadRate() / 10);
	m_UploadOMeter.SetYUnits(GetResString(IDS_KBYTESEC));
	//m_UploadOMeter.SetXUnits(GetResString(IDS_TIME));
	CSize sUpMeter[2];
	sUpMeter[0].cx = 100 * rect.left / rClient.Width();
	sUpMeter[0].cy = 100 * rect.top / rClient.Height();
	sUpMeter[1].cx = 100 * rect.right / rClient.Width();
	sUpMeter[1].cy = 100 * rect.bottom / rClient.Height();

	// Setup connections scope
	GetDlgItem(IDC_STATSSCOPE)->GetWindowRect(rect) ;
	ScreenToClient(rect) ;
	m_Statistics.Create(WS_VISIBLE | WS_CHILD, rect, this) ;
	m_Statistics.SetRanges(0, g_App.m_pPrefs->GetStatsMax());
	m_Statistics.autofitYscale=false; // DonGato
	m_Statistics.SetYUnits(_T("")) ;
	m_Statistics.SetXUnits(GetResString(IDS_TIME));

	CSize sStatistics[2];
	sStatistics[0].cx = 100 * rect.left / rClient.Width();
	sStatistics[0].cy = 100 * rect.top / rClient.Height();
	sStatistics[1].cx = 100 * rect.right / rClient.Width();
	sStatistics[1].cy = 100 * rect.bottom  / rClient.Height();

	m_byteStatGraphRatio = g_App.m_pPrefs->GetGraphRatio();		//Cax2 - Resize active connections line...

	RepaintMeters();

	if (g_App.m_pPrefs->GetStatsInterval() == 0) 
		GetDlgItem(IDC_STATTREE)->EnableWindow(false);

	UpdateData(FALSE);

	m_ilastMaxConnReached = 0;
	peakconnections = 0;
	totalconnectionchecks = 0;
	averageconnections = 0;
	activeconnections = 0;
	maxDown=0;
	maxDownavg=0;
	maxUp =					0;
	maxUpavg =				0;
	rateDown =				0;
	rateUp =				0;
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
	cum_DL_maximal =		g_App.m_pPrefs->GetConnMaxDownRate();
	cum_DL_average =		g_App.m_pPrefs->GetConnAvgDownRate();
	cum_DL_max_average =	g_App.m_pPrefs->GetConnMaxAvgDownRate();

	cum_UL_maximal =		g_App.m_pPrefs->GetConnMaxUpRate();
	cum_UL_average =		g_App.m_pPrefs->GetConnAvgUpRate();
	cum_UL_max_average =	g_App.m_pPrefs->GetConnMaxAvgUpRate();

	AddAnchor(IDC_STATTREE,TOP_LEFT, BOTTOM_CENTER );

	AddAnchor(IDC_SCOPE_D, TOP_CENTER, CSize(100,34));
	AddAnchor(m_DownloadOMeter.m_hWnd, TOP_CENTER, CSize(100,34));
	AddAnchor(IDC_SCOPE_U, CSize(50,34), CSize(100,68));
	AddAnchor(m_UploadOMeter.m_hWnd, CSize(50,34), CSize(100,68));
	AddAnchor(IDC_STATSSCOPE, CSize(50,68), BOTTOM_RIGHT);
	AddAnchor(m_Statistics.m_hWnd, CSize(50,68), BOTTOM_RIGHT);

	AddAnchor(IDC_STATIC_D3, TOP_CENTER);
	AddAnchor(IDC_C0, TOP_RIGHT);
	AddAnchor(IDC_STATIC_D, TOP_RIGHT);
	AddAnchor(IDC_C0_3, TOP_RIGHT);
	AddAnchor(IDC_TIMEAVG1, TOP_RIGHT);
	AddAnchor(IDC_C0_2, TOP_RIGHT);
	AddAnchor(IDC_STATIC_D2, TOP_RIGHT);
	AddAnchor(m_Led1[0].m_hWnd, TOP_RIGHT);
	AddAnchor(m_Led1[1].m_hWnd, TOP_RIGHT);
	AddAnchor(m_Led1[2].m_hWnd, TOP_RIGHT);

	AddAnchor(IDC_STATIC_U, CSize(50,34));
	AddAnchor(IDC_C1, CSize(100,34));
	AddAnchor(IDC_STATIC_U2, CSize(100,34));
	AddAnchor(IDC_C1_3, CSize(100,34));
	AddAnchor(IDC_TIMEAVG2, CSize(100,34));
	AddAnchor(IDC_C1_2, CSize(100,34));
	AddAnchor(IDC_STATIC_U3, CSize(100,34));
	AddAnchor(m_Led2[0].m_hWnd, CSize(100,34));
	AddAnchor(m_Led2[1].m_hWnd, CSize(100,34));
	AddAnchor(m_Led2[2].m_hWnd, CSize(100,34));

	AddAnchor(IDC_STATIC_A, CSize(50,68));
	AddAnchor(IDC_S3, CSize(100,68));
	AddAnchor(IDC_STATIC_S2, CSize(100,68));
	AddAnchor(IDC_S0, CSize(100,68));
	AddAnchor(IDC_STATIC_S0, CSize(100,68));
	AddAnchor(IDC_S1, CSize(100,68));
	AddAnchor(IDC_STATIC_S1, CSize(100,68));
	AddAnchor(m_Led3[1].m_hWnd, CSize(100,68));
	AddAnchor(m_Led3[0].m_hWnd, CSize(100,68));
	AddAnchor(m_Led3[2].m_hWnd, CSize(100,68));

	m_oldcx = 0;
	m_oldcy = 0;

	EnableWindow(TRUE);

	return true;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// -khaos--+++> This function is going to basically calculate and save a bunch of averages.
//				I made a seperate funtion so that it would always run instead of having
//				the averages not be calculated if the graphs are disabled (Which is bad!).
void CStatisticsDlg::UpdateConnectionStats(double uploadrate, double downloadrate)
{
	// Can't do anything before OnInitDialog
	if(!m_hWnd)
		return;

	rateUp = uploadrate;
	rateDown = downloadrate;

	if (maxUp<uploadrate) maxUp=uploadrate;
	if (cum_UL_maximal<maxUp)
	{
		cum_UL_maximal=maxUp;
		g_App.m_pPrefs->Add2ConnMaxUpRate(cum_UL_maximal);
	}

	if (maxDown<downloadrate) maxDown=downloadrate; // MOVED from SetCurrentRate!
	if (cum_DL_maximal < maxDown)
	{
		cum_DL_maximal=maxDown;
		g_App.m_pPrefs->Add2ConnMaxDownRate(cum_DL_maximal);
	}

	cum_DL_average = GetAvgDownloadRate(AVG_TOTAL);
	if (cum_DL_max_average<cum_DL_average)
	{
		cum_DL_max_average=cum_DL_average;
		g_App.m_pPrefs->Add2ConnMaxAvgDownRate(cum_DL_max_average);
	}

	cum_UL_average = GetAvgUploadRate(AVG_TOTAL);
	if (cum_UL_max_average<cum_UL_average)
	{
		cum_UL_max_average=cum_UL_average;
		g_App.m_pPrefs->Add2ConnMaxAvgUpRate(cum_UL_max_average);
	}


	// Transfer Times (Increment Session)
	if (uploadrate > 0 || downloadrate > 0)
	{
		if (start_timeTransfers == 0) start_timeTransfers = GetTickCount();
		else time_thisTransfer = (GetTickCount() - start_timeTransfers) / 1000;

		if (uploadrate > 0)
		{
			if (start_timeUploads == 0) start_timeUploads = GetTickCount();
			else time_thisUpload = (GetTickCount() - start_timeUploads) / 1000;
		}

		if (downloadrate > 0)
		{
			if (start_timeDownloads == 0) start_timeDownloads = GetTickCount();
			else time_thisDownload = (GetTickCount() - start_timeDownloads) / 1000;
		}
	}

	if (uploadrate == 0 && downloadrate == 0 && (time_thisTransfer > 0 || start_timeTransfers > 0))
	{
		timeTransfers += time_thisTransfer;
		time_thisTransfer = 0;
		start_timeTransfers = 0;
	}

	if (uploadrate == 0 && (time_thisUpload > 0 || start_timeUploads > 0))
	{
		timeUploads += time_thisUpload;
		time_thisUpload = 0;
		start_timeUploads = 0;
	}

	if (downloadrate == 0 && (time_thisDownload > 0 || start_timeDownloads > 0))
	{
		timeDownloads += time_thisDownload;
		time_thisDownload = 0;
		start_timeDownloads = 0;
	}

	// Server Durations
	if (g_App.stat_serverConnectTime==0) time_thisServerDuration = 0;
	else time_thisServerDuration = ( GetTickCount() - g_App.stat_serverConnectTime ) / 1000;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CStatisticsDlg::ShowStatistics(bool forceUpdate)
{
	stattree.SetRedraw(false);

	CString	cbuffer, strBuf2;
	uint64	aqwDQData[STATS_DLDATA_COUNT];
	uint32	adwDQSrc[STATS_DLSRC_COUNT];
	// Set Tree Values

	// TRANSFER SECTION
	// If a section is not expanded, don't waste CPU cycles updating it.
	if (forceUpdate || stattree.IsExpanded(h_transfer))
	{
		uint32	statGoodSessions =				0;
		uint32	statBadSessions =				0;
		double	percentSessions =				0;
		// Transfer Ratios
		if ( g_App.stat_sessionReceivedBytes>0 && g_App.stat_sessionSentBytes>0 )
		{
			// Session
			if (g_App.stat_sessionReceivedBytes<g_App.stat_sessionSentBytes)
			{
				cbuffer.Format(_T("%s %.2f : 1"),GetResString(IDS_STATS_SRATIO),static_cast<double>(g_App.stat_sessionSentBytes)/g_App.stat_sessionReceivedBytes);
				stattree.SetItemText(trans[0], cbuffer);
			}
			else
			{
				cbuffer.Format(_T("%s 1 : %.2f"),GetResString(IDS_STATS_SRATIO),static_cast<double>(g_App.stat_sessionReceivedBytes)/g_App.stat_sessionSentBytes);
				stattree.SetItemText(trans[0], cbuffer);
			}
		}
		else
		{
			cbuffer.Format(_T("%s %s"), GetResString(IDS_STATS_SRATIO), GetResString(IDS_FSTAT_WAITING));
			stattree.SetItemText(trans[0], cbuffer);
		}
		if ( (g_App.m_pPrefs->GetTotalDownloaded()>0 && g_App.m_pPrefs->GetTotalUploaded()>0) || (g_App.stat_sessionReceivedBytes>0 && g_App.stat_sessionSentBytes>0) )
		{
			// Cumulative
			if ((g_App.stat_sessionReceivedBytes+g_App.m_pPrefs->GetTotalDownloaded())<(g_App.stat_sessionSentBytes+g_App.m_pPrefs->GetTotalUploaded()))
			{
				cbuffer.Format(_T("%s %.2f : 1"),GetResString(IDS_STATS_CRATIO),static_cast<double>(g_App.stat_sessionSentBytes+g_App.m_pPrefs->GetTotalUploaded())/(g_App.stat_sessionReceivedBytes+g_App.m_pPrefs->GetTotalDownloaded()));
				stattree.SetItemText(trans[1], cbuffer);
			}
			else
			{
				cbuffer.Format(_T("%s 1 : %.2f"),GetResString(IDS_STATS_CRATIO),static_cast<double>(g_App.stat_sessionReceivedBytes+g_App.m_pPrefs->GetTotalDownloaded())/(g_App.stat_sessionSentBytes+g_App.m_pPrefs->GetTotalUploaded()));
				stattree.SetItemText(trans[1], cbuffer);
			}
		}
		else
		{
			cbuffer.Format(_T("%s %s"), GetResString(IDS_STATS_CRATIO), GetResString(IDS_FSTAT_WAITING));
			stattree.SetItemText(trans[1], cbuffer);
		}
		// TRANSFER -> DOWNLOADS SECTION
		if (forceUpdate || stattree.IsExpanded(h_download))
		{
			uint64	DownOHTotal = 0;
			uint64	DownOHTotalPackets = 0;
			g_App.m_pDownloadQueue->GetDownloadStats(adwDQSrc, aqwDQData);
			// TRANSFER -> DOWNLOADS -> SESSION SECTION
			if (forceUpdate || stattree.IsExpanded(h_down_session))
			{
				// Downloaded Data
				cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_DDATA), CastItoXBytes(g_App.stat_sessionReceivedBytes));
				stattree.SetItemText(down_S[0], cbuffer);
				if (forceUpdate || stattree.IsExpanded(down_S[0]))
				{
					// Downloaded Data By Client
					if (forceUpdate || stattree.IsExpanded(hdown_scb))
					{
						uint64 DownDataTotal = g_App.m_pPrefs->GetDownSessionClientData();
						uint64 DownDataClient = 0;
						double percentClientTransferred = 0;
						int i = 0;
						for (int j = 0;j<SO_LAST;j++)
							if ((EnumClientTypes)j != SO_OLDEMULE)
							{
								if ((EnumClientTypes)j == SO_EMULE)
									DownDataClient = g_App.m_pPrefs->GetDownData((EnumClientTypes)j)
													+ g_App.m_pPrefs->GetDownData(SO_OLDEMULE);
								else
									DownDataClient = g_App.m_pPrefs->GetDownData((EnumClientTypes)j);
								if ( DownDataTotal!=0 && DownDataClient!=0 )
									percentClientTransferred = (double) 100 * DownDataClient / DownDataTotal;
								else
									percentClientTransferred = 0;
								cbuffer.Format(_T("%s: %s (%.1f%%)"), GetClientNameString((EnumClientTypes)j), CastItoXBytes(DownDataClient), percentClientTransferred);
								stattree.SetItemText(down_scb[i], cbuffer);
								i++;
							}
					}
				}
				// Completed Downloads
				cbuffer.Format(_T("%s: %u"), GetResString(IDS_STATS_COMPDL), g_App.m_pPrefs->GetDownSessionCompletedFiles());
				stattree.SetItemText(down_S[1], cbuffer);
				// Active Downloads (not paused or stopped)
				cbuffer.Format(GetResString(IDS_DWTOT_NUM), g_App.m_pDownloadQueue->GetActiveFileCount());
				stattree.SetItemText(down_S[2], cbuffer);
				// Transferring Chunks
				cbuffer.Format(GetResString(IDS_STATS_ACTDL), adwDQSrc[STATS_DLSRC_TRANSFERRING]);
				stattree.SetItemText(down_S[3], cbuffer);
				// Total Size of Active Downloads
				cbuffer.Format(GetResString(IDS_DWTOT_TSD), CastItoXBytes(aqwDQData[STATS_DLDAT_ACTFILESIZE]));
				stattree.SetItemText(down_S[4], cbuffer);
				// Total Size of All Downloads
				cbuffer.Format(GetResString(IDS_DWTOT_TSAD), CastItoXBytes(aqwDQData[STATS_DLDAT_FILESIZETOTAL]));
				stattree.SetItemText(down_S[5], cbuffer);
				// Total Size on Disk of All Downloads
				cbuffer.Format(GetResString(IDS_DWTOT_TSODAD), CastItoXBytes(aqwDQData[STATS_DLDAT_FILEREALSIZE]));
				stattree.SetItemText(down_S[6], cbuffer);
				// Total Completed Size
				uint64 ui64BytesTransferred = (aqwDQData[STATS_DLDAT_ACTFILESIZE] - aqwDQData[STATS_DLDAT_SIZE2TRANSFER]);
				cbuffer.Format(GetResString(IDS_DWTOT_TCS), CastItoXBytes(ui64BytesTransferred), (aqwDQData[STATS_DLDAT_ACTFILESIZE] != 0) ? (100*static_cast<double>(ui64BytesTransferred) / aqwDQData[STATS_DLDAT_ACTFILESIZE]) : 0);
				stattree.SetItemText( down_S[7] , cbuffer );
				// Total Size Left to Transfer
				cbuffer.Format(GetResString(IDS_DWTOT_TSL), CastItoXBytes(aqwDQData[STATS_DLDAT_SIZE2TRANSFER]), (aqwDQData[STATS_DLDAT_ACTFILESIZE] != 0) ? (100*static_cast<double>(aqwDQData[STATS_DLDAT_SIZE2TRANSFER]) / aqwDQData[STATS_DLDAT_ACTFILESIZE]) : 0);
				stattree.SetItemText(down_S[8], cbuffer);
				// Total Space Needed by Active Downloads
				cbuffer.Format(GetResString(IDS_DWTOT_TSN), CastItoXBytes(aqwDQData[STATS_DLDAT_ACTFILEREQSPACE]));
				stattree.SetItemText(down_S[9], cbuffer);
				// Total Space Needed by All Downloads
				cbuffer.Format( GetResString(IDS_DWTOT_TSNA), CastItoXBytes(aqwDQData[STATS_DLDAT_FILEREQSPACETOTAL]));
				stattree.SetItemText(down_S[10], cbuffer);
				// Free Space on Tempdrive
				uint64 t_FreeBytes = GetFreeDiskSpaceX(g_App.m_pPrefs->GetTempDir().GetBuffer());
				strBuf2.Format(GetResString(IDS_DWTOT_FS), CastItoXBytes(t_FreeBytes));
				if(aqwDQData[STATS_DLDAT_ACTFILEREQSPACE] > t_FreeBytes)
					cbuffer.Format(GetResString(IDS_NEEDFREEDISKSPACE), strBuf2, CastItoXBytes(aqwDQData[STATS_DLDAT_ACTFILEREQSPACE] - t_FreeBytes));
				else
					cbuffer.Format(GetResString(IDS_FREEDISKSPACEAFTER), strBuf2, CastItoXBytes(t_FreeBytes - aqwDQData[STATS_DLDAT_ACTFILEREQSPACE]));
				stattree.SetItemText(down_S[11], cbuffer);
				// Found Sources
				cbuffer.Format(GetResString(IDS_STATS_FOUNDSRC), adwDQSrc[STATS_DLSRC_TOTAL]);
				stattree.SetItemText(down_S[12], cbuffer);
				if (forceUpdate || stattree.IsExpanded(down_S[12]))
				{
					int i = 0;
					// Sources By Status
					cbuffer.Format(_T("%s: %u"), GetResString(IDS_ONQUEUE), adwDQSrc[STATS_DLSRC_ONQUEUE]);
					stattree.SetItemText(down_sources[i], cbuffer); i++;
					cbuffer.Format(_T("%s: %u"), GetResString(IDS_QUEUEFULL), adwDQSrc[STATS_DLSRC_QUEUEFULL]);
					stattree.SetItemText(down_sources[i], cbuffer); i++;
					cbuffer.Format(_T("%s: %u"), GetResString(IDS_STATS_HIGHQR), adwDQSrc[STATS_DLSRC_HIGH_QR]);
					stattree.SetItemText(down_sources[i], cbuffer); i++;
					cbuffer.Format(_T("%s: %u"), GetResString(IDS_NONEEDEDPARTS), adwDQSrc[STATS_DLSRC_NNS]);
					stattree.SetItemText(down_sources[i], cbuffer); i++;
					cbuffer.Format(_T("%s: %u"), GetResString(IDS_ASKING), adwDQSrc[STATS_DLSRC_CONNECTED]);
					stattree.SetItemText(down_sources[i], cbuffer); i++;
					cbuffer.Format(_T("%s: %u"), GetResString(IDS_CONNECTING), adwDQSrc[STATS_DLSRC_CONNECTING]);
					stattree.SetItemText(down_sources[i], cbuffer); i++;
					cbuffer.Format(_T("%s: %u"), GetResString(IDS_CONNVIASERVER), adwDQSrc[STATS_DLSRC_CONNECTING_VIA_SRV]);
					stattree.SetItemText(down_sources[i], cbuffer); i++;
					cbuffer.Format(_T("%s: %u"), GetResString(IDS_WAITFILEREQ), adwDQSrc[STATS_DLSRC_WAIT4FILEREQ]);
					stattree.SetItemText(down_sources[i], cbuffer); i++;
					cbuffer.Format(_T("%s: %u"), GetResString(IDS_ASKED4ANOTHERFILE), g_App.m_pClientList->GetA4AFSourcesCount());
					stattree.SetItemText(down_sources[i], cbuffer); i++;
					cbuffer.Format(_T("%s: %u"), GetResString(IDS_NOCONNECTLOW2LOW), adwDQSrc[STATS_DLSRC_LOW2LOW]);
					stattree.SetItemText(down_sources[i], cbuffer); i++;
					cbuffer.Format(_T("%s: %u"), GetResString(IDS_ANOTHER_SERVER_LOWID), adwDQSrc[STATS_DLSRC_LOWID_ON_OTHER_SRV]);
					stattree.SetItemText(down_sources[i], cbuffer); i++;
					cbuffer.Format(_T("%s: %u"), GetResString(IDS_BANNED), adwDQSrc[STATS_DLSRC_BANNED]);
					stattree.SetItemText(down_sources[i], cbuffer); i++;
				}
				// Set Download Sessions
				statGoodSessions =	g_App.m_pPrefs->GetDownS_SuccessfulSessions() + adwDQSrc[STATS_DLSRC_TRANSFERRING]; // Add Active Downloads
				statBadSessions =	g_App.m_pPrefs->GetDownS_FailedSessions();
				cbuffer.Format(_T("%s: %u"), GetResString(IDS_STATS_DLSES), statGoodSessions + statBadSessions);
				stattree.SetItemText(down_S[13], cbuffer);
				if (forceUpdate || stattree.IsExpanded(down_S[13]))
				{
				// Set Successful Download Sessions and Average Downloaded Per Session
					percentSessions = 0;
					if (statGoodSessions > 0)
					{
						percentSessions = (double) 100 * statGoodSessions / (statGoodSessions + statBadSessions);
						cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_AVGDATADLSES), CastItoXBytes((uint64)g_App.stat_sessionReceivedBytes / statGoodSessions));
					}
					else cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_AVGDATADLSES), CastItoXBytes(0));
					stattree.SetItemText(down_ssessions[2], cbuffer); // Set Avg DL/Session
					cbuffer.Format(_T("%s: %u (%.1f%%)"), GetResString(IDS_STATS_SDLSES), statGoodSessions, percentSessions);
					stattree.SetItemText(down_ssessions[0], cbuffer); // Set Succ Sessions
				// Set Failed Download Sessions (Avoid Division)
					if (percentSessions != 0 && statBadSessions > 0) percentSessions = 100 - percentSessions; // There were some good sessions and bad ones...
					else if (percentSessions == 0 && statBadSessions > 0) percentSessions = 100; // There were bad sessions and no good ones, must be 100%
					else percentSessions = 0; // No sessions at all, or no bad ones.
					cbuffer.Format(_T("%s: %u (%.1f%%)"), GetResString(IDS_STATS_FDLSES), statBadSessions, percentSessions);
					stattree.SetItemText(down_ssessions[1], cbuffer);
				// Set Failed Download Sessions because remote client does not have required data
					uint16 statBadSessionsNoRequiredData =	g_App.m_pPrefs->GetDownS_FailedSessionsNoRequiredData();
					percentSessions = 0;
					if ((statGoodSessions + statBadSessions) > 0)
					{
						percentSessions = (double) 100 * statBadSessionsNoRequiredData / (statGoodSessions + statBadSessions);
					}
					cbuffer.Format(_T("%s: %u (%.1f%%)"), GetResString(IDS_STATS_FDLSES_NRD), statBadSessionsNoRequiredData, percentSessions);
					stattree.SetItemText(htiDLFailedSesNRD, cbuffer);
				// Set Average Download Time
					cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_AVGDLTIME), CastSecondsToLngHM(g_App.m_pPrefs->GetDownS_AvgTime()));
					stattree.SetItemText(down_ssessions[3] , cbuffer);
				}
				// Set Gain Due To Compression
				cbuffer.Format(GetResString(IDS_STATS_GAINCOMP), CastItoXBytes(g_App.m_pPrefs->GetSesSavedFromCompression()));
				stattree.SetItemText(down_S[14], cbuffer);
				// Set Lost Due To Corruption
				cbuffer.Format(GetResString(IDS_STATS_LOSTCORRUPT), CastItoXBytes(g_App.m_pPrefs->GetSesLostFromCorruption()));
				stattree.SetItemText(down_S[15], cbuffer);
				// Set Parts Saved Due To ICH
				cbuffer.Format(GetResString(IDS_STATS_ICHSAVED), g_App.m_pPrefs->GetSesPartsSavedByICH());
				stattree.SetItemText(down_S[16], cbuffer);

				// Calculate downline OH totals
				DownOHTotal = g_App.m_pDownloadQueue->GetDownDataOverheadFileRequest() + g_App.m_pDownloadQueue->GetDownDataOverheadSourceExchange() + g_App.m_pDownloadQueue->GetDownDataOverheadServer() + g_App.m_pDownloadQueue->GetDownDataOverheadOther();
				DownOHTotalPackets = g_App.m_pDownloadQueue->GetDownDataOverheadFileRequestPackets() + g_App.m_pDownloadQueue->GetDownDataOverheadSourceExchangePackets() + g_App.m_pDownloadQueue->GetDownDataOverheadServerPackets() + g_App.m_pDownloadQueue->GetDownDataOverheadOtherPackets();

				// Downline Overhead
				cbuffer.Format( GetResString( IDS_TOVERHEAD ) , CastItoXBytes( DownOHTotal ) , CastItoIShort( DownOHTotalPackets ) );
				stattree.SetItemText( hdown_soh , cbuffer );
				if (forceUpdate || stattree.IsExpanded(hdown_soh))
				{
					int i = 0;
					// Set down session file req OH
					cbuffer.Format( GetResString( IDS_FROVERHEAD ) , CastItoXBytes( g_App.m_pDownloadQueue->GetDownDataOverheadFileRequest() ) , CastItoIShort( g_App.m_pDownloadQueue->GetDownDataOverheadFileRequestPackets() ) );
					stattree.SetItemText( down_soh[i] , cbuffer ); i++;
					// Set down session source exch OH
					cbuffer.Format( GetResString( IDS_SSOVERHEAD ) , CastItoXBytes( g_App.m_pDownloadQueue->GetDownDataOverheadSourceExchange() ), CastItoIShort( g_App.m_pDownloadQueue->GetDownDataOverheadSourceExchangePackets() ) );
					stattree.SetItemText( down_soh[i] , cbuffer ); i++;
					// Set down session server OH
					cbuffer.Format( GetResString( IDS_SOVERHEAD ) , CastItoXBytes( g_App.m_pDownloadQueue->GetDownDataOverheadServer() ), CastItoIShort( g_App.m_pDownloadQueue->GetDownDataOverheadServerPackets() ) );
					stattree.SetItemText( down_soh[i] , cbuffer ); i++;
					// Set down session others OH
					cbuffer.Format( GetResString( IDS_OOVERHEAD ) , CastItoXBytes( g_App.m_pDownloadQueue->GetDownDataOverheadOther() ), CastItoIShort( g_App.m_pDownloadQueue->GetDownDataOverheadOtherPackets() ) );
					stattree.SetItemText( down_soh[i] , cbuffer ); i++;
				}

			}
			// TRANSFER -> DOWNLOADS -> CUMULATIVE SECTION
			if (forceUpdate || stattree.IsExpanded(h_down_total))
			{
				// Downloaded Data
				cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_DDATA), CastItoXBytes(g_App.stat_sessionReceivedBytes+g_App.m_pPrefs->GetTotalDownloaded()));
				stattree.SetItemText(down_T[0], cbuffer);
				if (forceUpdate || stattree.IsExpanded(down_T[0]))
				{
					// Downloaded Data By Client
					if (forceUpdate || stattree.IsExpanded(hdown_tcb))
					{
						uint64 DownDataTotal = g_App.m_pPrefs->GetDownTotalClientData();
						uint64 DownDataClient = 0;
						double percentClientTransferred = 0;
						//eklmn: sequence was replaced to loop...
						int i = 0;
						for (int j = 0;j<SO_LAST;j++)
							if ((EnumClientTypes)j != SO_OLDEMULE)
							{
								if ((EnumClientTypes)j == SO_EMULE)
									DownDataClient = g_App.m_pPrefs->GetCumDownData((EnumClientTypes)j)
													+ g_App.m_pPrefs->GetCumDownData(SO_OLDEMULE);
								else
									DownDataClient = g_App.m_pPrefs->GetCumDownData((EnumClientTypes)j);
								if ( DownDataTotal!=0 && DownDataClient!=0 )
									percentClientTransferred = (double) 100 * DownDataClient / DownDataTotal;
								else
									percentClientTransferred = 0;
								cbuffer.Format(_T("%s: %s (%.1f%%)"), GetClientNameString((EnumClientTypes)j), CastItoXBytes(DownDataClient) , percentClientTransferred);
								stattree.SetItemText( down_tcb[i] , cbuffer );
								i++;
							}
					}
				}
				// Set Cum Completed Downloads
				cbuffer.Format(_T("%s: %u"), GetResString(IDS_STATS_COMPDL), g_App.m_pPrefs->GetDownCompletedFiles());
				stattree.SetItemText(down_T[1], cbuffer);
				// Set Cum Download Sessions
				statGoodSessions = g_App.m_pPrefs->GetDownC_SuccessfulSessions() + adwDQSrc[STATS_DLSRC_TRANSFERRING]; // Need to reset these from the session section. Declared up there.
				statBadSessions = g_App.m_pPrefs->GetDownC_FailedSessions(); // ^^^^^^^^^^^^^^
				cbuffer.Format(_T("%s: %u"), GetResString(IDS_STATS_DLSES), statGoodSessions + statBadSessions);
				stattree.SetItemText(down_T[2], cbuffer);
				if (forceUpdate || stattree.IsExpanded(down_T[2]))
				{
					// Set Cum Successful Download Sessions & Cum Average Download Per Sessions (Save an if-else statement)
					if (statGoodSessions > 0)
					{
						percentSessions = (double) 100 * statGoodSessions / (statGoodSessions + statBadSessions);
						cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_AVGDATADLSES), CastItoXBytes((uint64)(g_App.stat_sessionReceivedBytes + g_App.m_pPrefs->GetTotalDownloaded()) / statGoodSessions));
					}
					else
					{
						percentSessions = 0;
						cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_AVGDATADLSES), CastItoXBytes(0));
					}
					stattree.SetItemText(down_tsessions[2], cbuffer); // Set Avg DL/Session
					cbuffer.Format(_T("%s: %u (%.1f%%)"), GetResString(IDS_STATS_SDLSES), statGoodSessions, percentSessions);
					stattree.SetItemText(down_tsessions[0], cbuffer); // Set Successful Sessions
					// Set Cum Failed Download Sessions
					if (percentSessions != 0 && statBadSessions > 0) percentSessions = 100 - percentSessions; // There were some good sessions and bad ones...
					else if (percentSessions == 0 && statBadSessions > 0) percentSessions = 100; // There were bad sessions and no good ones, must be 100%
					else percentSessions = 0; // No sessions at all, or no bad ones.
					cbuffer.Format(_T("%s: %u (%.1f%%)"), GetResString(IDS_STATS_FDLSES), statBadSessions, percentSessions);
					stattree.SetItemText(down_tsessions[1], cbuffer);
					// Set Cumulative Average Download Time
					uint32 avgDownTime = g_App.m_pPrefs->GetDownS_AvgTime();
					if (g_App.m_pPrefs->GetDownC_AvgTime()<=0) g_App.m_pPrefs->Add2DownCAvgTime(avgDownTime);
					avgDownTime = (uint32) (avgDownTime+g_App.m_pPrefs->GetDownC_AvgTime())/2;
					cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_AVGDLTIME), CastSecondsToLngHM(avgDownTime));
					stattree.SetItemText(down_tsessions[3], cbuffer);
				}
				// Set Cumulative Gained Due To Compression
				cbuffer.Format(GetResString(IDS_STATS_GAINCOMP), CastItoXBytes(g_App.m_pPrefs->GetSesSavedFromCompression() + g_App.m_pPrefs->GetCumSavedFromCompression()));
				stattree.SetItemText(down_T[3], cbuffer);
				// Set Cumulative Lost Due To Corruption
				cbuffer.Format(GetResString(IDS_STATS_LOSTCORRUPT), CastItoXBytes(g_App.m_pPrefs->GetSesLostFromCorruption() + g_App.m_pPrefs->GetCumLostFromCorruption()));
				stattree.SetItemText(down_T[4], cbuffer);
				// Set Cumulative Saved Due To ICH
				cbuffer.Format(GetResString(IDS_STATS_ICHSAVED), g_App.m_pPrefs->GetSesPartsSavedByICH() + g_App.m_pPrefs->GetCumPartsSavedByICH());
				stattree.SetItemText(down_T[5], cbuffer);

				if (DownOHTotal == 0 || DownOHTotalPackets == 0)
				{
					DownOHTotal = g_App.m_pDownloadQueue->GetDownDataOverheadFileRequest() + g_App.m_pDownloadQueue->GetDownDataOverheadSourceExchange() + g_App.m_pDownloadQueue->GetDownDataOverheadServer() + g_App.m_pDownloadQueue->GetDownDataOverheadOther();
					DownOHTotalPackets = g_App.m_pDownloadQueue->GetDownDataOverheadFileRequestPackets() + g_App.m_pDownloadQueue->GetDownDataOverheadSourceExchangePackets() + g_App.m_pDownloadQueue->GetDownDataOverheadServerPackets() + g_App.m_pDownloadQueue->GetDownDataOverheadOtherPackets();
				}
				// Total Overhead
				cbuffer.Format(GetResString(IDS_TOVERHEAD),CastItoXBytes(DownOHTotal + g_App.m_pPrefs->GetDownOverheadTotal()), CastItoIShort(DownOHTotalPackets + g_App.m_pPrefs->GetDownOverheadTotalPackets()));
				stattree.SetItemText(hdown_toh, cbuffer);
				if (forceUpdate || stattree.IsExpanded(hdown_toh))
				{
					int i = 0;
					// File Request Overhead
					cbuffer.Format(GetResString(IDS_FROVERHEAD), CastItoXBytes( g_App.m_pDownloadQueue->GetDownDataOverheadFileRequest() + g_App.m_pPrefs->GetDownOverheadFileReq()), CastItoIShort(g_App.m_pDownloadQueue->GetDownDataOverheadFileRequestPackets() + g_App.m_pPrefs->GetDownOverheadFileReqPackets()));
					stattree.SetItemText(down_toh[i], cbuffer); i++;
					// Source Exchange Overhead
					cbuffer.Format(GetResString(IDS_SSOVERHEAD), CastItoXBytes( g_App.m_pDownloadQueue->GetDownDataOverheadSourceExchange()+g_App.m_pPrefs->GetDownOverheadSrcEx()), CastItoIShort(g_App.m_pDownloadQueue->GetDownDataOverheadSourceExchangePackets()+g_App.m_pPrefs->GetDownOverheadSrcExPackets()));
					stattree.SetItemText(down_toh[i], cbuffer); i++;
					// Server Overhead
					cbuffer.Format(GetResString(IDS_SOVERHEAD), CastItoXBytes( g_App.m_pDownloadQueue->GetDownDataOverheadServer()+g_App.m_pPrefs->GetDownOverheadServer()), CastItoIShort(g_App.m_pDownloadQueue->GetDownDataOverheadServerPackets()+g_App.m_pPrefs->GetDownOverheadServerPackets()));
					stattree.SetItemText(down_toh[i], cbuffer); i++;
					// Others Overhead
					cbuffer.Format(GetResString(IDS_OOVERHEAD), CastItoXBytes( g_App.m_pDownloadQueue->GetDownDataOverheadOther()+g_App.m_pPrefs->GetDownOverheadOther()), CastItoIShort(g_App.m_pDownloadQueue->GetDownDataOverheadOtherPackets()+g_App.m_pPrefs->GetDownOverheadOtherPackets()));
					stattree.SetItemText(down_toh[i], cbuffer); i++;
				}
			} // - End Transfer -> Downloads -> Cumulative Section
		} // - End Transfer -> Downloads Section
		// TRANSFER-> UPLOADS SECTION
		if (forceUpdate || stattree.IsExpanded(h_upload))
		{
			uint64 UpOHTotal =			0;
			uint64 UpOHTotalPackets =	0;
			// TRANSFER -> UPLOADS -> SESSION SECTION
			if (forceUpdate || stattree.IsExpanded(h_up_session))
			{
				// Uploaded Data
				cbuffer.Format(_T("%s: %s"),GetResString(IDS_STATS_UDATA),CastItoXBytes(g_App.stat_sessionSentBytes));
				stattree.SetItemText(up_S[0], cbuffer);
				if (forceUpdate || stattree.IsExpanded(up_S[0]))
				{
					// Uploaded Data By Client
					if (forceUpdate || stattree.IsExpanded(hup_scb))
					{
						uint64 UpDataTotal =	g_App.m_pPrefs->GetUpSessionClientData();
						uint64 UpDataClient = 0;
						double percentClientTransferred = 0;
						//eklmn: sequence was replaced to loop...
						int i = 0;
						for (int j = 0;j<SO_LAST;j++)
							if ((EnumClientTypes)j != SO_OLDEMULE)
							{
								if ((EnumClientTypes)j == SO_EMULE)
									UpDataClient = g_App.m_pPrefs->GetUpData((EnumClientTypes)j)
													+ g_App.m_pPrefs->GetUpData(SO_OLDEMULE);
								else
									UpDataClient = g_App.m_pPrefs->GetUpData((EnumClientTypes)j);
								if ( UpDataTotal!=0 && UpDataClient!=0 )
									percentClientTransferred = (double) 100 * UpDataClient / UpDataTotal;
								else
									percentClientTransferred = 0;
								cbuffer.Format(_T("%s: %s (%.1f%%)"), GetClientNameString((EnumClientTypes)j), CastItoXBytes(UpDataClient), percentClientTransferred);
								stattree.SetItemText(up_scb[i], cbuffer);
								i++;
							}
					}
					// Uploaded Data By Source
					if (forceUpdate || stattree.IsExpanded(hup_ssb))
					{
						int i = 0;
						uint64	DataSourceFile =	g_App.m_pPrefs->GetUpData_File();
						uint64	DataSourcePF =		g_App.m_pPrefs->GetUpData_PartFile();
						uint64	DataSourceTotal =	g_App.m_pPrefs->GetUpSessionDataFile();
						double	percentFileTransferred = 0;

						if ( DataSourceTotal!=0 && DataSourceFile!=0 )
							percentFileTransferred = (double) 100 * DataSourceFile / DataSourceTotal;
						cbuffer.Format(_T("%s: %s (%.1f%%)"), GetResString(IDS_STATS_DSFILE), CastItoXBytes(DataSourceFile), percentFileTransferred);
						stattree.SetItemText(up_ssb[i], cbuffer); i++;

						if ( DataSourceTotal!=0 && DataSourcePF!=0 )
							percentFileTransferred = (double) 100 * DataSourcePF / DataSourceTotal;
						else
							percentFileTransferred = 0;
						cbuffer.Format(_T("%s: %s (%.1f%%)"), GetResString(IDS_STATS_DSPF), CastItoXBytes(DataSourcePF), percentFileTransferred);
						stattree.SetItemText(up_ssb[i], cbuffer); i++;
					}
					// Uploaded Data By Community
					if (forceUpdate || stattree.IsExpanded(hup_scomb))
					{
						int i = 0;
						uint64	DataCommunity =		g_App.m_pPrefs->GetUpData_Community();
						uint64	DataNoCommunity =	g_App.m_pPrefs->GetUpData_NoCommunity();
						uint64	DataTotal =			g_App.m_pPrefs->GetUpSessionDataCommunity();
						double	percentCommunityTransferred = 0;

						if ( DataTotal!=0 && DataCommunity!=0 )
							percentCommunityTransferred = (double) 100 * DataCommunity / DataTotal;
						cbuffer.Format(_T("%s: %s (%.1f%%)"), GetResString(IDS_YES), CastItoXBytes(DataCommunity), percentCommunityTransferred);
						stattree.SetItemText(up_scomb[i], cbuffer); i++;

						if ( DataTotal!=0 && DataNoCommunity!=0 )
							percentCommunityTransferred = (double) 100 * DataNoCommunity / DataTotal;
						else
							percentCommunityTransferred = 0;
						cbuffer.Format(_T("%s: %s (%.1f%%)"), GetResString(IDS_NO), CastItoXBytes(DataNoCommunity), percentCommunityTransferred);
						stattree.SetItemText(up_scomb[i], cbuffer); i++;
					}
					// Uploaded Data By Priority
					if (forceUpdate || stattree.IsExpanded(hULPrioDataNode))
					{
						int i = 0;
						uint64	qwData;

						qwData = g_App.m_pPrefs->GetUpDataByPriority(PR_RELEASE);
						cbuffer.Format(_T("%s: %s (%.1f%%)"), GetResString(IDS_PRIORELEASE), CastItoXBytes(qwData), GetPercent(qwData, g_App.stat_sessionSentBytes));
						stattree.SetItemText(hULPrioDataItems[i], cbuffer); i++;

						qwData = g_App.m_pPrefs->GetUpDataByPriority(PR_HIGH);
						cbuffer.Format(_T("%s: %s (%.1f%%)"), GetResString(IDS_PRIOHIGH), CastItoXBytes(qwData), GetPercent(qwData, g_App.stat_sessionSentBytes));
						stattree.SetItemText(hULPrioDataItems[i], cbuffer); i++;

						qwData = g_App.m_pPrefs->GetUpDataByPriority(PR_NORMAL);
						cbuffer.Format(_T("%s: %s (%.1f%%)"), GetResString(IDS_PRIONORMAL), CastItoXBytes(qwData), GetPercent(qwData, g_App.stat_sessionSentBytes));
						stattree.SetItemText(hULPrioDataItems[i], cbuffer); i++;

						qwData = g_App.m_pPrefs->GetUpDataByPriority(PR_LOW);
						cbuffer.Format(_T("%s: %s (%.1f%%)"), GetResString(IDS_PRIOLOW), CastItoXBytes(qwData), GetPercent(qwData, g_App.stat_sessionSentBytes));
						stattree.SetItemText(hULPrioDataItems[i], cbuffer); i++;

						qwData = g_App.m_pPrefs->GetUpDataByPriority(PR_VERYLOW);
						cbuffer.Format(_T("%s: %s (%.1f%%)"), GetResString(IDS_PRIOVERYLOW), CastItoXBytes(qwData), GetPercent(qwData, g_App.stat_sessionSentBytes));
						stattree.SetItemText(hULPrioDataItems[i], cbuffer); i++;
					}
				}
				// Set Active Uploads
				cbuffer.Format(GetResString(IDS_STATS_ACTUL),g_App.m_pUploadQueue->GetUploadQueueLength());
				stattree.SetItemText(up_S[1], cbuffer);
				// Set Queue Length
				cbuffer.Format(GetResString(IDS_STATS_WAITINGUSERS),g_App.m_pUploadQueue->GetWaitingUserCount());
				stattree.SetItemText(up_S[2], cbuffer);

				// Set Upload Sessions
				statGoodSessions = g_App.m_pUploadQueue->GetSuccessfulUpCount(); // + g_App.m_pUploadQueue->GetUploadQueueLength();
				statBadSessions = g_App.m_pUploadQueue->GetFailedUpCount();
				cbuffer.Format(_T("%s: %u"), GetResString(IDS_STATS_ULSES), statGoodSessions + statBadSessions);
				stattree.SetItemText(up_S[3], cbuffer);
				if (forceUpdate || stattree.IsExpanded(up_S[3]))
				{
					// Set Successful Upload Sessions & Average Uploaded Per Session
					uint32 statTotalSessions = statGoodSessions+statBadSessions;
					if (statGoodSessions>0)
					{
						percentSessions = (double) 100*(statGoodSessions+g_App.m_pUploadQueue->GetUploadQueueLength())/(statGoodSessions+g_App.m_pUploadQueue->GetUploadQueueLength()+statBadSessions);
						cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_AVGDATAULSES), CastItoXBytes((uint64)g_App.stat_sessionSentBytes / (statGoodSessions+g_App.m_pUploadQueue->GetUploadQueueLength())));
					}
					else
					{
						percentSessions = 0;
						cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_AVGDATAULSES), GetResString(IDS_FSTAT_WAITING));
					}
					stattree.SetItemText(up_ssessions[2], cbuffer);
					// Successful sessions
					cbuffer.Format(_T("%s: %u (%.1f%%)"),GetResString(IDS_STATS_SUCCUPCOUNT),statGoodSessions,GetPercent(statGoodSessions,statTotalSessions));
					stattree.SetItemText(up_ssessions[0], cbuffer);
					// chunks stasts
					cbuffer.Format(_T("%s: %u (%.1f%%)"),GetResString(IDS_STATS_SUCCUP_FC),g_App.m_pUploadQueue->GetULFullChunkCount(),GetPercent(g_App.m_pUploadQueue->GetULFullChunkCount(),statTotalSessions));
					stattree.SetItemText(up_ssessions_s[0], cbuffer);
					cbuffer.Format(_T("%s: %u (%.1f%%)"),GetResString(IDS_STATS_SUCCUP_PC),g_App.m_pUploadQueue->GetULPartChunkCount(),GetPercent(g_App.m_pUploadQueue->GetULPartChunkCount(),statTotalSessions));
					stattree.SetItemText(up_ssessions_s[1], cbuffer);
					//subtree for "part of chunk"
					for (int i = 0; i<ETS_TERMINATOR; i++)
					{
						cbuffer.Format(_T("%s: %u (%.1f%%)"), GetUpEndReason(i),
							g_App.m_pUploadQueue->GetULPartChunkSubCount((EnumEndTransferSession)i),
							GetPercent(g_App.m_pUploadQueue->GetULPartChunkSubCount((EnumEndTransferSession)i), statTotalSessions));
						stattree.SetItemText(up_ssessions_spc[i], cbuffer);
					}

					// Set Failed Upload Sessions
					cbuffer.Format(_T("%s: %u (%.1f%%)"),GetResString(IDS_STATS_FAILUPCOUNT),statBadSessions,GetPercent(statBadSessions,statTotalSessions));
					stattree.SetItemText(up_ssessions[1], cbuffer);
					//subtree for "Failed"
					for (int i = 0; i<ETS_TERMINATOR; i++)
					{
						cbuffer.Format(_T("%s: %u (%.1f%%)"), GetUpEndReason(i),
							g_App.m_pUploadQueue->GetFailedSubCount((EnumEndTransferSession)i),
							GetPercent(g_App.m_pUploadQueue->GetFailedSubCount((EnumEndTransferSession)i), statTotalSessions));
						stattree.SetItemText(up_ssessions_f[i], cbuffer);
					}

					// Set Avg Upload time
					cbuffer.Format(GetResString(IDS_STATS_AVEUPTIME), CastSecondsToLngHM(g_App.m_pUploadQueue->GetAverageUpTime()));
					stattree.SetItemText(up_ssessions[3], cbuffer);
				}
				// Calculate Upline OH Totals
				UpOHTotal = g_App.m_pUploadQueue->GetUpDataOverheadFileRequest() + g_App.m_pUploadQueue->GetUpDataOverheadSourceExchange() + g_App.m_pUploadQueue->GetUpDataOverheadServer() + g_App.m_pUploadQueue->GetUpDataOverheadOther();
				UpOHTotalPackets = g_App.m_pUploadQueue->GetUpDataOverheadFileRequestPackets() + g_App.m_pUploadQueue->GetUpDataOverheadSourceExchangePackets() + g_App.m_pUploadQueue->GetUpDataOverheadServerPackets() + g_App.m_pUploadQueue->GetUpDataOverheadOtherPackets();
				// Total Upline Overhead
				cbuffer.Format(GetResString(IDS_TOVERHEAD), CastItoXBytes(UpOHTotal), CastItoIShort(UpOHTotalPackets));
				stattree.SetItemText(hup_soh, cbuffer);
				if (forceUpdate || stattree.IsExpanded(hup_soh))
				{
					int i = 0;
					// File Request Overhead
					cbuffer.Format(GetResString(IDS_FROVERHEAD), CastItoXBytes( g_App.m_pUploadQueue->GetUpDataOverheadFileRequest()), CastItoIShort(g_App.m_pUploadQueue->GetUpDataOverheadFileRequestPackets()));
					stattree.SetItemText(up_soh[i], cbuffer); i++;
					// Source Exchanged Overhead
					cbuffer.Format(GetResString(IDS_SSOVERHEAD), CastItoXBytes( g_App.m_pUploadQueue->GetUpDataOverheadSourceExchange()), CastItoIShort(g_App.m_pUploadQueue->GetUpDataOverheadSourceExchangePackets()));
					stattree.SetItemText(up_soh[i], cbuffer); i++;
					// Server Overhead
					cbuffer.Format(GetResString(IDS_SOVERHEAD), CastItoXBytes( g_App.m_pUploadQueue->GetUpDataOverheadServer()), CastItoIShort(g_App.m_pUploadQueue->GetUpDataOverheadServerPackets()));
					stattree.SetItemText(up_soh[i], cbuffer); i++;
					// Others Overhead
					cbuffer.Format(GetResString(IDS_OOVERHEAD), CastItoXBytes( g_App.m_pUploadQueue->GetUpDataOverheadOther()), CastItoIShort(g_App.m_pUploadQueue->GetUpDataOverheadOtherPackets()));
					stattree.SetItemText(up_soh[i], cbuffer); i++;
				}
			} // - End Transfer -> Uploads -> Session Section
			// TRANSFER -> UPLOADS -> CUMULATIVE SECTION
			if (forceUpdate || stattree.IsExpanded(h_up_total))
			{

				// Uploaded Data
				cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_UDATA), CastItoXBytes(g_App.stat_sessionSentBytes+g_App.m_pPrefs->GetTotalUploaded()));
				stattree.SetItemText(up_T[0],cbuffer);
				if (forceUpdate || stattree.IsExpanded(up_T[0]))
				{
					// Uploaded Data By Client
					if (forceUpdate || stattree.IsExpanded(hup_tcb))
					{
						uint64 UpDataTotal =	g_App.m_pPrefs->GetUpTotalClientData();
						uint64 UpDataClient =	0;
						double percentClientTransferred = 0;
						int i = 0;
						//eklmn: sequence was replaced to loop...
						for (int j = 0;j<SO_LAST;j++)
							if ((EnumClientTypes)j != SO_OLDEMULE)
							{
								if ((EnumClientTypes)j == SO_EMULE)
									UpDataClient = g_App.m_pPrefs->GetCumUpData((EnumClientTypes)j)
													+ g_App.m_pPrefs->GetCumUpData(SO_OLDEMULE);
								else
									UpDataClient = g_App.m_pPrefs->GetCumUpData((EnumClientTypes)j);

								if ( UpDataTotal!=0 && UpDataClient!=0 )
									percentClientTransferred = (double) 100 * UpDataClient / UpDataTotal;
								else
									percentClientTransferred = 0;
								cbuffer.Format(_T("%s: %s (%.1f%%)"), GetClientNameString((EnumClientTypes)j), CastItoXBytes(UpDataClient), percentClientTransferred);
								stattree.SetItemText(up_tcb[i], cbuffer);
								i++;
							}
					}
					// Uploaded Data By Source
					if (forceUpdate || stattree.IsExpanded(hup_tsb))
					{
						int i = 0;
						uint64	DataSourceFile =	g_App.m_pPrefs->GetCumUpData_File();
						uint64	DataSourcePF =		g_App.m_pPrefs->GetCumUpData_PartFile();
						uint64	DataSourceTotal =	g_App.m_pPrefs->GetUpTotalDataFile();
						double	percentFileTransferred = 0;

						if ( DataSourceTotal!=0 && DataSourceFile!=0 )
							percentFileTransferred = (double) 100 * DataSourceFile / DataSourceTotal;
						cbuffer.Format(_T("%s: %s (%.1f%%)"), GetResString(IDS_STATS_DSFILE), CastItoXBytes(DataSourceFile), percentFileTransferred);
						stattree.SetItemText(up_tsb[i], cbuffer); i++;

						if ( DataSourceTotal!=0 && DataSourcePF!=0 )
							percentFileTransferred = (double) 100 * DataSourcePF / DataSourceTotal;
						else
							percentFileTransferred = 0;
						cbuffer.Format(_T("%s: %s (%.1f%%)"), GetResString(IDS_STATS_DSPF), CastItoXBytes(DataSourcePF), percentFileTransferred);
						stattree.SetItemText(up_tsb[i], cbuffer); i++;
					}
					// Uploaded Data By Community
					if (forceUpdate || stattree.IsExpanded(hup_tcomb))
					{
						int i = 0;
						uint64	DataCommunity =		g_App.m_pPrefs->GetCumUpData_Community();
						uint64	DataNoCommunity =	g_App.m_pPrefs->GetCumUpData_NoCommunity();
						uint64	DataTotal =			g_App.m_pPrefs->GetUpTotalDataCommunity();
						double	percentCommunityTransferred = 0;

						if ( DataTotal!=0 && DataCommunity!=0 )
							percentCommunityTransferred = (double) 100 * DataCommunity / DataTotal;
						cbuffer.Format(_T("%s: %s (%.1f%%)"), GetResString(IDS_YES), CastItoXBytes(DataCommunity), percentCommunityTransferred);
						stattree.SetItemText(up_tcomb[i], cbuffer); i++;

						if ( DataTotal!=0 && DataNoCommunity!=0 )
							percentCommunityTransferred = (double) 100 * DataNoCommunity / DataTotal;
						else
							percentCommunityTransferred = 0;
						cbuffer.Format(_T("%s: %s (%.1f%%)"), GetResString(IDS_NO), CastItoXBytes(DataNoCommunity), percentCommunityTransferred);
						stattree.SetItemText(up_tcomb[i], cbuffer); i++;
					}
				}
				// Upload Sessions
				statGoodSessions = g_App.m_pUploadQueue->GetSuccessfulUpCount() + g_App.m_pPrefs->GetUpSuccessfulSessions() + g_App.m_pUploadQueue->GetUploadQueueLength();
				statBadSessions = g_App.m_pUploadQueue->GetFailedUpCount() + g_App.m_pPrefs->GetUpFailedSessions();
				cbuffer.Format(_T("%s: %u"), GetResString(IDS_STATS_ULSES), statGoodSessions + statBadSessions);
				stattree.SetItemText(up_T[1], cbuffer);
				if (forceUpdate || stattree.IsExpanded(up_T[1]))
				{
					// Set Successful Upload Sessions & Average Uploaded Per Session
					if (statGoodSessions>0)
					{ // Blackholes are when God divided by 0
						percentSessions = (double) 100*statGoodSessions/(statGoodSessions+statBadSessions);
						cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_AVGDATAULSES), CastItoXBytes((uint64) (g_App.stat_sessionSentBytes + g_App.m_pPrefs->GetTotalUploaded()) / statGoodSessions));
					}
					else
					{
						percentSessions = 0;
						cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_AVGDATAULSES), GetResString(IDS_FSTAT_WAITING));
					}
					stattree.SetItemText(up_tsessions[2], cbuffer);
					//cbuffer.Format(GetResString(IDS_STATS_SUCCUPCOUNT),statGoodSessions,percentSessions);
					cbuffer.Format(_T("%s: %u (%.1f%%)"), GetResString(IDS_STATS_SUCCUPCOUNT), statGoodSessions, percentSessions);
					stattree.SetItemText(up_tsessions[0], cbuffer);
					// Set Failed Upload Sessions
					if (percentSessions != 0 && statBadSessions > 0) percentSessions = 100 - percentSessions; // There were some good sessions and bad ones...
					else if (percentSessions == 0 && statBadSessions > 0) percentSessions = 100; // There were bad sessions and no good ones, must be 100%
					else percentSessions = 0; // No sessions at all, or no bad ones.
					//cbuffer.Format(GetResString(IDS_STATS_FAILUPCOUNT),statBadSessions,percentSessions);
					cbuffer.Format(_T("%s: %u (%.1f%%)"), GetResString(IDS_STATS_FAILUPCOUNT), statBadSessions, percentSessions);
					stattree.SetItemText(up_tsessions[1], cbuffer);
					// Set Avg Upload time
					uint32 avguptime = g_App.m_pUploadQueue->GetAverageUpTime();
					if (g_App.m_pPrefs->GetUpAvgTime() <= 0) g_App.m_pPrefs->Add2UpAvgTime(avguptime);
					avguptime = (avguptime + g_App.m_pPrefs->GetUpAvgTime()) / 2u;
					cbuffer.Format(GetResString(IDS_STATS_AVEUPTIME), CastSecondsToLngHM(avguptime));
					stattree.SetItemText(up_tsessions[3], cbuffer);
				}

				if (UpOHTotal == 0 || UpOHTotalPackets == 0)
				{
					// Calculate Upline OH Totals
					UpOHTotal = g_App.m_pUploadQueue->GetUpDataOverheadFileRequest() + g_App.m_pUploadQueue->GetUpDataOverheadSourceExchange() + g_App.m_pUploadQueue->GetUpDataOverheadServer() + g_App.m_pUploadQueue->GetUpDataOverheadOther();
					UpOHTotalPackets = g_App.m_pUploadQueue->GetUpDataOverheadFileRequestPackets() + g_App.m_pUploadQueue->GetUpDataOverheadSourceExchangePackets() + g_App.m_pUploadQueue->GetUpDataOverheadServerPackets() + g_App.m_pUploadQueue->GetUpDataOverheadOtherPackets();
				}
				// Set Cumulative Total Overhead
				cbuffer.Format(GetResString(IDS_TOVERHEAD),CastItoXBytes(UpOHTotal + g_App.m_pPrefs->GetUpOverheadTotal()), CastItoIShort(UpOHTotalPackets + g_App.m_pPrefs->GetUpOverheadTotalPackets()));
				stattree.SetItemText(hup_toh, cbuffer);
				if (forceUpdate || stattree.IsExpanded(hup_toh))
				{
					int i = 0;
					// Set up total file req OH
					cbuffer.Format(GetResString(IDS_FROVERHEAD), CastItoXBytes( g_App.m_pUploadQueue->GetUpDataOverheadFileRequest() + g_App.m_pPrefs->GetUpOverheadFileReq()), CastItoIShort(g_App.m_pUploadQueue->GetUpDataOverheadFileRequestPackets() + g_App.m_pPrefs->GetUpOverheadFileReqPackets()));
					stattree.SetItemText(up_toh[i], cbuffer); i++;
					// Set up total source exch OH
					cbuffer.Format(GetResString(IDS_SSOVERHEAD), CastItoXBytes( g_App.m_pUploadQueue->GetUpDataOverheadSourceExchange()+g_App.m_pPrefs->GetUpOverheadSrcEx()), CastItoIShort(g_App.m_pUploadQueue->GetUpDataOverheadSourceExchangePackets()+g_App.m_pPrefs->GetUpOverheadSrcExPackets()));
					stattree.SetItemText(up_toh[i], cbuffer); i++;
					// Set up total server OH
					cbuffer.Format(GetResString(IDS_SOVERHEAD), CastItoXBytes( g_App.m_pUploadQueue->GetUpDataOverheadServer()+g_App.m_pPrefs->GetUpOverheadServer()), CastItoIShort(g_App.m_pUploadQueue->GetUpDataOverheadServerPackets()+g_App.m_pPrefs->GetUpOverheadServerPackets()));
					stattree.SetItemText(up_toh[i], cbuffer); i++;
					// Set up total others OH
					cbuffer.Format(GetResString(IDS_OOVERHEAD), CastItoXBytes( g_App.m_pUploadQueue->GetUpDataOverheadOther()+g_App.m_pPrefs->GetUpOverheadOther()), CastItoIShort(g_App.m_pUploadQueue->GetUpDataOverheadOtherPackets()+g_App.m_pPrefs->GetUpOverheadOtherPackets()));
					stattree.SetItemText(up_toh[i], cbuffer); i++;
				}
			} // - End Transfer -> Uploads -> Cumulative Section
		} // - End Transfer -> Uploads Section
	} // - END TRANSFER SECTION


	// CONNECTION SECTION
	if (forceUpdate || stattree.IsExpanded(h_connection))
	{
		// CONNECTION -> SESSION SECTION
		if (forceUpdate || stattree.IsExpanded(h_conn_session))
		{
			// CONNECTION -> SESSION -> GENERAL SECTION
			if (forceUpdate || stattree.IsExpanded(hconn_sg))
			{
				int i = 0;
				// Server Reconnects
				if (g_App.stat_reconnects>0) cbuffer.Format(GetResString(IDS_STATS_RECONNECTS),g_App.stat_reconnects-1);
				else cbuffer.Format(GetResString(IDS_STATS_RECONNECTS),0);
				stattree.SetItemText(conn_sg[i], cbuffer); i++;
				// Active Connections
				cbuffer.Format(_T("%s: %u"), GetResString(IDS_SF_ACTIVECON), activeconnections);
				stattree.SetItemText(conn_sg[i], cbuffer); i++;
				// Average Connections
				cbuffer.Format(_T("%s: %u"), GetResString(IDS_SF_AVGCON), (uint32)averageconnections);
				stattree.SetItemText(conn_sg[i], cbuffer); i++;
				// Peak Connections
				cbuffer.Format(_T("%s: %u"), GetResString(IDS_SF_PEAKCON), peakconnections);
				stattree.SetItemText(conn_sg[i], cbuffer); i++;
				// Connect Limit Reached
#ifdef OLD_SOCKETS_ENABLED
				uint32 m_itemp = g_App.m_pListenSocket->GetMaxConnectionsReachedCount();
				if( m_itemp != m_ilastMaxConnReached )
				{
					COleDateTime	currentTime(COleDateTime::GetCurrentTime());
					cbuffer.Format(_T("%s: %i : %s"), GetResString(IDS_SF_MAXCONLIMITREACHED), m_itemp, currentTime.Format(_T("%c")));
					stattree.SetItemText(conn_sg[i], cbuffer);
					m_ilastMaxConnReached = m_itemp;
				}
				else if( m_itemp == 0 )
				{
					cbuffer.Format(_T("%s: %i"), GetResString(IDS_SF_MAXCONLIMITREACHED), m_itemp);
					stattree.SetItemText(conn_sg[i], cbuffer);
				}
#endif //OLD_SOCKETS_ENABLED
				i++;
			} // - End Connection -> Session -> General Section
			// CONNECTION -> SESSION -> UPLOADS SECTION
			if (forceUpdate || stattree.IsExpanded(hconn_su))
			{
				CString	strKBSec = GetResString(IDS_KBYTESEC);
				double	dAvgUpRate = GetAvgUploadRate(AVG_SESSION);
				int		i = 0;

				// Upload Rate
				cbuffer.Format(_T("%s: %.2f %s"), GetResString(IDS_ST_UPLOAD), rateUp, strKBSec);			stattree.SetItemText(conn_su[i], cbuffer); i++;
				// Average Upload Rate
				cbuffer.Format(GetResString(IDS_STATS_AVGUL), dAvgUpRate);	stattree.SetItemText(conn_su[i], cbuffer); i++;
				// Max Upload Rate
				cbuffer.Format(_T("%s: %.2f %s"), GetResString(IDS_STATS_MAXUL), maxUp, strKBSec);			stattree.SetItemText(conn_su[i], cbuffer); i++;
				// Max Average Upload Rate
				if (dAvgUpRate > maxUpavg)
					maxUpavg = dAvgUpRate;
				cbuffer.Format(_T("%s: %.2f %s"), GetResString(IDS_STATS_MAXAVGUL), maxUpavg, strKBSec);	stattree.SetItemText(conn_su[i], cbuffer); i++;
			} // - End Connection -> Session -> Uploads Section
			// CONNECTION -> SESSION -> DOWNLOADS SECTION
			if (forceUpdate || stattree.IsExpanded(hconn_sd))
			{
				double dAvgDownRate = GetAvgDownloadRate(AVG_SESSION);
				int		i = 0;

				// Download Rate
				cbuffer.Format(_T("%s: %.2f %s"), GetResString(IDS_ST_DOWNLOAD), rateDown, GetResString(IDS_KBYTESEC));		stattree.SetItemText(conn_sd[i], cbuffer); i++;
				// Average Download Rate
				cbuffer.Format(GetResString(IDS_STATS_AVGDL), dAvgDownRate);	stattree.SetItemText(conn_sd[i], cbuffer); i++;
				// Max Download Rate
				cbuffer.Format(GetResString(IDS_STATS_MAXDL), maxDown);							stattree.SetItemText(conn_sd[i], cbuffer); i++;
				// Max Average Download Rate
				if (dAvgDownRate > maxDownavg)
					maxDownavg = dAvgDownRate;
				cbuffer.Format(GetResString(IDS_STATS_MAXAVGDL), maxDownavg);					stattree.SetItemText(conn_sd[i], cbuffer); i++;
			} // - End Connection -> Session -> Downloads Section
		} // - End Connection -> Session Section
		// CONNECTION -> CUMULATIVE SECTION
		if (forceUpdate || stattree.IsExpanded(h_conn_total))
		{
			// CONNECTION -> CUMULATIVE -> GENERAL SECTION
			if (forceUpdate || stattree.IsExpanded(hconn_tg))
			{
				int i = 0;
				// Server Reconnects
				if(g_App.stat_reconnects>0)
					cbuffer.Format(GetResString(IDS_STATS_RECONNECTS),g_App.stat_reconnects - 1 + g_App.m_pPrefs->GetConnNumReconnects());
				else
					cbuffer.Format(GetResString(IDS_STATS_RECONNECTS),g_App.m_pPrefs->GetConnNumReconnects());
				stattree.SetItemText(conn_tg[i], cbuffer); i++;
				// Average Connections
				cbuffer.Format(_T("%s: %u"), GetResString(IDS_SF_AVGCON), (activeconnections + g_App.m_pPrefs->GetConnAvgConnections()) / 2);
				stattree.SetItemText(conn_tg[i], cbuffer); i++;
				// Peak Connections
				cbuffer.Format(_T("%s: %u"), GetResString(IDS_SF_PEAKCON), g_App.m_pPrefs->GetConnPeakConnections());
				stattree.SetItemText(conn_tg[i], cbuffer); i++;
				// Connection Limit Reached
#ifdef OLD_SOCKETS_ENABLED
				cbuffer.Format(_T("%s: %u"), GetResString(IDS_SF_MAXCONLIMITREACHED), g_App.m_pListenSocket->GetMaxConnectionsReachedCount() + g_App.m_pPrefs->GetConnMaxConnLimitReached());
				stattree.SetItemText(conn_tg[i], cbuffer); i++;
#endif //OLD_SOCKETS_ENABLED
			} // - End Connection -> Cumulative -> General Section
			// CONNECTION -> CUMULATIVE -> UPLOADS SECTION
			if (forceUpdate || stattree.IsExpanded(hconn_tu))
			{
				int i = 0;
				// Average Upload Rate
				cbuffer.Format(GetResString(IDS_STATS_AVGUL),cum_UL_average);
				stattree.SetItemText(conn_tu[i], cbuffer); i++;
				// Max Upload Rate
				cbuffer.Format(_T("%s: %.2f ") + GetResString(IDS_KBYTESEC), GetResString(IDS_STATS_MAXUL), cum_UL_maximal);
				stattree.SetItemText(conn_tu[i], cbuffer); i++;
				// Max Average Upload Rate
				cbuffer.Format(_T("%s: %.2f ") + GetResString(IDS_KBYTESEC), GetResString(IDS_STATS_MAXAVGUL), cum_UL_max_average);
				stattree.SetItemText(conn_tu[i], cbuffer); i++;
			} // - End Connection -> Cumulative -> Uploads Section
			// CONNECTION -> CUMULATIVE -> DOWNLOADS SECTION
			if (forceUpdate || stattree.IsExpanded(hconn_td))
			{
				int i = 0;
				// Average Download Rate
				cbuffer.Format(GetResString(IDS_STATS_AVGDL), cum_DL_average);
				stattree.SetItemText(conn_td[i], cbuffer); i++;
				// Max Download Rate
				cbuffer.Format(GetResString(IDS_STATS_MAXDL), cum_DL_maximal);
				stattree.SetItemText(conn_td[i], cbuffer); i++;
				// Max Average Download Rate
				cbuffer.Format(GetResString(IDS_STATS_MAXAVGDL), cum_DL_max_average);
				stattree.SetItemText(conn_td[i], cbuffer); i++;
			} // - End Connection -> Cumulative -> Downloads Section
		} // - End Connection -> Cumulative Section
	} // - END CONNECTION SECTION


	// TIME STATISTICS SECTION
	if (forceUpdate || stattree.IsExpanded(h_time))
	{
		// Statistics Last Reset
		cbuffer.Format(GetResString(IDS_STATS_LASTRESETSTATIC), g_App.m_pPrefs->GetStatsLastResetStr());
		stattree.SetItemText(tvitime[0], cbuffer);
		// Time Since Last Reset
		time_t timeDiff, timeNow;
		if (g_App.m_pPrefs->GetStatsLastResetLng())
		{
			time(&timeNow);
			timeDiff = timeNow - g_App.m_pPrefs->GetStatsLastResetLng(); // In seconds
			cbuffer.Format(GetResString(IDS_STATS_TIMESINCERESET), CastSecondsToLngHM(timeDiff));
		}
		else
		{
			timeDiff = 0;
			cbuffer.Format(GetResString(IDS_STATS_TIMESINCERESET), GetResString(IDS_UNKNOWN));
		}
		stattree.SetItemText(tvitime[1], cbuffer);
		// TIME STATISTICS -> SESSION SECTION
		if (forceUpdate || stattree.IsExpanded(htime_s))
		{
			int i = 0;
			uint32	dwSesRunTime = (::GetTickCount() - g_App.stat_starttime) / 1000u;

			cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_RUNTIME), CastSecondsToLngHM(dwSesRunTime));
			stattree.SetItemText(tvitime_s[i], cbuffer); i++;
			if (dwSesRunTime == 0)
				dwSesRunTime = 1;
			// Transfer Time
			cbuffer.Format(_T("%s: %s (%.1f%%)"), GetResString(IDS_STATS_TRANSTIME), CastSecondsToLngHM(GetTransferTime()), (double) (100 * GetTransferTime()) / dwSesRunTime);
			stattree.SetItemText(tvitime_s[i], cbuffer);
			if (forceUpdate || stattree.IsExpanded(tvitime_s[i]))
			{
				int x = 0;
				// Upload Time
				cbuffer.Format(_T("%s: %s (%.1f%%)"), GetResString(IDS_STATS_UPTIME), CastSecondsToLngHM(GetUploadTime()), (double) (100 * GetUploadTime()) / dwSesRunTime);
				stattree.SetItemText(tvitime_st[x], cbuffer); x++;
				// Download Time
				cbuffer.Format(_T("%s: %s (%.1f%%)"), GetResString(IDS_STATS_DOWNTIME), CastSecondsToLngHM(GetDownloadTime()), (double) (100 * GetDownloadTime()) / dwSesRunTime);
				stattree.SetItemText(tvitime_st[x], cbuffer); x++;
			}
			i++;
			// Current Server Duration
			cbuffer.Format(_T("%s: %s (%.1f%%)"), GetResString(IDS_STATS_CURRSRVDUR), CastSecondsToLngHM(time_thisServerDuration), (double) (100 * time_thisServerDuration) / dwSesRunTime);
			stattree.SetItemText(tvitime_s[i], cbuffer); i++;
			// Total Server Duration
			cbuffer.Format(_T("%s: %s (%.1f%%)"), GetResString(IDS_STATS_TOTALSRVDUR), CastSecondsToLngHM(GetServerDuration()), (double) (100 * GetServerDuration()) / dwSesRunTime);
			stattree.SetItemText(tvitime_s[i], cbuffer); i++;
		}
		// TIME STATISTICS -> CUMULATIVE SECTION
		if (forceUpdate || stattree.IsExpanded(htime_t))
		{
			int i = 0;
			uint32	dwTotalRunTime = (::GetTickCount() - g_App.stat_starttime) / 1000u + g_App.m_pPrefs->GetConnRunTime();

			cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_RUNTIME), CastSecondsToLngHM(dwTotalRunTime));
			stattree.SetItemText(tvitime_t[i], cbuffer); i++;
			if (dwTotalRunTime == 0)
				dwTotalRunTime = 1;
			// Transfer Time
			cbuffer.Format(_T("%s: %s (%.1f%%)"), GetResString(IDS_STATS_TRANSTIME), CastSecondsToLngHM(GetTransferTime() + g_App.m_pPrefs->GetConnTransferTime()), (double) (100 * (GetTransferTime() + g_App.m_pPrefs->GetConnTransferTime())) / dwTotalRunTime);
			stattree.SetItemText(tvitime_t[i], cbuffer);
			if (forceUpdate || stattree.IsExpanded(tvitime_t[i]))
			{
				int x = 0;
				// Upload Time
				cbuffer.Format(_T("%s: %s (%.1f%%)"), GetResString(IDS_STATS_UPTIME), CastSecondsToLngHM(GetUploadTime() + g_App.m_pPrefs->GetConnUploadTime()), (double) (100 * (GetUploadTime() + g_App.m_pPrefs->GetConnUploadTime())) / dwTotalRunTime);
				stattree.SetItemText(tvitime_tt[x], cbuffer); x++;
				// Download Time
				cbuffer.Format(_T("%s: %s (%.1f%%)"), GetResString(IDS_STATS_DOWNTIME), CastSecondsToLngHM(GetDownloadTime() + g_App.m_pPrefs->GetConnDownloadTime()), (double) (100 * (GetDownloadTime() + g_App.m_pPrefs->GetConnDownloadTime())) / dwTotalRunTime);
				stattree.SetItemText(tvitime_tt[x], cbuffer); x++;
			}
			i++;
			// Total Server Duration
			cbuffer.Format(_T("%s: %s (%.1f%%)"), GetResString(IDS_STATS_TOTALSRVDUR), CastSecondsToLngHM(GetServerDuration() + g_App.m_pPrefs->GetConnServerDuration()), (double) (100 * (GetServerDuration() + g_App.m_pPrefs->GetConnServerDuration())) / dwTotalRunTime);
			stattree.SetItemText(tvitime_t[i], cbuffer); i++;
		}
		// TIME STATISTICS -> PROJECTED AVERAGES SECTION
		if ( (forceUpdate || stattree.IsExpanded(htime_aap)) && timeDiff > 0 )
		{
			double avgModifier[3];
			avgModifier[0] = (double) 86400 / timeDiff; // Days
			avgModifier[1] = (double) 2628000 / timeDiff; // Months
			avgModifier[2] = (double) 31536000 / timeDiff; // Years
			// TIME STATISTICS -> PROJECTED AVERAGES -> TIME PERIODS
			// This section is completely scalable.  Might add "Week" to it in the future.
			// For each time period that we are calculating a projected average for...
			for (int mx = 0; mx < 3; mx++)
			{
				if (forceUpdate || stattree.IsExpanded(time_aaph[mx]))
				{
					// TIME STATISTICS -> PROJECTED AVERAGES -> TIME PERIOD -> UPLOADS SECTION
					if (forceUpdate || stattree.IsExpanded(time_aap_hup[mx]))
					{
						// Uploaded Data
						cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_UDATA), CastItoXBytes((uint64)((double)(uint64)(g_App.stat_sessionSentBytes+g_App.m_pPrefs->GetTotalUploaded())*avgModifier[mx])));
						stattree.SetItemText(time_aap_up[mx][0],cbuffer);
						if (forceUpdate || stattree.IsExpanded(time_aap_up[mx][0]))
						{
							// Uploaded Data By Client
							if (forceUpdate || stattree.IsExpanded(time_aap_up_hd[mx][0]))
							{
								double dUpDataTotal = g_App.m_pPrefs->GetUpTotalClientData() * avgModifier[mx];
								uint64 UpDataTotal = static_cast<uint64>(dUpDataTotal);
								uint64 UpDataClient;
								double dUpDataClient, percentClientTransferred;
								//eklmn: sequence was replaced to loop...
								int i = 0;
								for (int j = 0;j<SO_LAST;j++)
									if ((EnumClientTypes)j != SO_OLDEMULE)
									{
										if ((EnumClientTypes)j == SO_EMULE)
											dUpDataClient = (g_App.m_pPrefs->GetCumUpData((EnumClientTypes)j)
															+ g_App.m_pPrefs->GetCumUpData(SO_OLDEMULE)) * avgModifier[mx];
										else
											dUpDataClient = g_App.m_pPrefs->GetCumUpData((EnumClientTypes)j) * avgModifier[mx];
										UpDataClient = static_cast<uint64>(dUpDataClient);

										if ((UpDataTotal != 0) && (UpDataClient != 0))
											percentClientTransferred = 100.0 * dUpDataClient / dUpDataTotal;
										else
											percentClientTransferred = 0;
										cbuffer.Format(_T("%s: %s (%.1f%%)"), GetClientNameString((EnumClientTypes)j), CastItoXBytes(UpDataClient), percentClientTransferred);
										stattree.SetItemText( time_aap_up_dc[mx][i] , cbuffer );
										i++;
									}
							}
							// Uploaded Data By Source
							if (forceUpdate || stattree.IsExpanded(time_aap_up_hd[mx][1]))
							{
								int i = 0;
								double	dDataSourceFile = g_App.m_pPrefs->GetCumUpData_File() * avgModifier[mx];
								uint64	DataSourceFile = static_cast<uint64>(dDataSourceFile);
								double	dDataSourcePF = g_App.m_pPrefs->GetCumUpData_PartFile() * avgModifier[mx];
								uint64	DataSourcePF = static_cast<uint64>(dDataSourcePF);
								double	dDataSourceTotal = g_App.m_pPrefs->GetUpTotalDataFile() * avgModifier[mx];
								uint64	DataSourceTotal = static_cast<uint64>(dDataSourceTotal);
								double	percentFileTransferred = 0;

								if ((DataSourceTotal != 0) && (DataSourceFile != 0))
									percentFileTransferred = 100.0 * dDataSourceFile / dDataSourceTotal;
								cbuffer.Format(_T("%s: %s (%.1f%%)"), GetResString(IDS_STATS_DSFILE), CastItoXBytes(DataSourceFile), percentFileTransferred);
								stattree.SetItemText(time_aap_up_ds[mx][i], cbuffer); i++;

								if ((DataSourceTotal != 0) && (DataSourcePF != 0))
									percentFileTransferred = 100.0 * dDataSourcePF / dDataSourceTotal;
								else
									percentFileTransferred = 0;
								cbuffer.Format(_T("%s: %s (%.1f%%)"), GetResString(IDS_STATS_DSPF), CastItoXBytes(DataSourcePF), percentFileTransferred);
								stattree.SetItemText(time_aap_up_ds[mx][i], cbuffer); i++;
							}
							// Uploaded Data By Community
							if (forceUpdate || stattree.IsExpanded(time_aap_up_hd[mx][2]))
							{
								int i = 0;
								double	dDataCommunity = g_App.m_pPrefs->GetCumUpData_Community() * avgModifier[mx];
								uint64	DataCommunity = static_cast<uint64>(dDataCommunity);
								double	dDataNoCommunity = g_App.m_pPrefs->GetCumUpData_NoCommunity() * avgModifier[mx];
								uint64	DataNoCommunity = static_cast<uint64>(dDataNoCommunity);
								double	dDataTotal = g_App.m_pPrefs->GetUpTotalDataCommunity() * avgModifier[mx];
								uint64	DataTotal = static_cast<uint64>(dDataTotal);
								double	percentCommunityTransferred = 0;

								if ((DataTotal != 0) && (DataCommunity != 0))
									percentCommunityTransferred = 100.0 * dDataCommunity / dDataTotal;
								cbuffer.Format(_T("%s: %s (%.1f%%)"), GetResString(IDS_YES), CastItoXBytes(DataCommunity), percentCommunityTransferred);
								stattree.SetItemText( time_aap_up_com[mx][i] , cbuffer ); i++;

								if ((DataTotal != 0) && (DataNoCommunity != 0))
									percentCommunityTransferred = 100.0 * dDataNoCommunity / dDataTotal;
								else
									percentCommunityTransferred = 0;
								cbuffer.Format(_T("%s: %s (%.1f%%)"), GetResString(IDS_NO), CastItoXBytes(DataNoCommunity), percentCommunityTransferred);
								stattree.SetItemText( time_aap_up_com[mx][i] , cbuffer ); i++;
							}
						}
						// Upload Sessions
						double dGoodSessions = (g_App.m_pUploadQueue->GetSuccessfulUpCount() + g_App.m_pPrefs->GetUpSuccessfulSessions() + g_App.m_pUploadQueue->GetUploadQueueLength()) * avgModifier[mx];
						uint32 statGoodSessions = static_cast<uint32>(dGoodSessions);
						double dBadSessions = (g_App.m_pUploadQueue->GetFailedUpCount() + g_App.m_pPrefs->GetUpFailedSessions()) * avgModifier[mx];
						uint32 statBadSessions = static_cast<uint32>(dBadSessions);
						double percentSessions;
						cbuffer.Format(_T("%s: %u"), GetResString(IDS_STATS_ULSES), statGoodSessions + statBadSessions);
						stattree.SetItemText(time_aap_up[mx][1], cbuffer);
						if (forceUpdate || stattree.IsExpanded(time_aap_up[mx][1]))
						{
							// Set Successful Upload Sessions
							if (statGoodSessions > 0) percentSessions = 100.0 * dGoodSessions / (dGoodSessions + dBadSessions);
							else percentSessions = 0;
							cbuffer.Format(_T("%s: %u (%.1f%%)"), GetResString(IDS_STATS_SUCCUPCOUNT), statGoodSessions, percentSessions);
							stattree.SetItemText(time_aap_up_s[mx][0], cbuffer);
							// Set Failed Upload Sessions
							if (percentSessions != 0 && statBadSessions > 0) percentSessions = 100 - percentSessions; // There were some good sessions and bad ones...
							else if (percentSessions == 0 && statBadSessions > 0) percentSessions = 100; // There were bad sessions and no good ones, must be 100%
							else percentSessions = 0; // No sessions at all, or no bad ones.
							cbuffer.Format(_T("%s: %u (%.1f%%)"), GetResString(IDS_STATS_FAILUPCOUNT), statBadSessions, percentSessions);
							stattree.SetItemText(time_aap_up_s[mx][1], cbuffer);
						}

						// Calculate Upline OH Totals
						uint64 UpOHTotal = g_App.m_pUploadQueue->GetUpDataOverheadFileRequest() + g_App.m_pUploadQueue->GetUpDataOverheadSourceExchange() + g_App.m_pUploadQueue->GetUpDataOverheadServer() + g_App.m_pUploadQueue->GetUpDataOverheadOther();
						uint64 UpOHTotalPackets = g_App.m_pUploadQueue->GetUpDataOverheadFileRequestPackets() + g_App.m_pUploadQueue->GetUpDataOverheadSourceExchangePackets() + g_App.m_pUploadQueue->GetUpDataOverheadServerPackets() + g_App.m_pUploadQueue->GetUpDataOverheadOtherPackets();

						// Set Cumulative Total Overhead
						cbuffer.Format(GetResString(IDS_TOVERHEAD), CastItoXBytes((uint64)((double)(uint64)(UpOHTotal + g_App.m_pPrefs->GetUpOverheadTotal()) * avgModifier[mx])), CastItoIShort((uint64)((double)(uint64)(UpOHTotalPackets + g_App.m_pPrefs->GetUpOverheadTotalPackets()) * avgModifier[mx])));
						stattree.SetItemText(time_aap_up[mx][2], cbuffer);
						if (forceUpdate || stattree.IsExpanded(time_aap_up[mx][2]))
						{
							int i = 0;
							// Set up total file req OH
							cbuffer.Format(GetResString(IDS_FROVERHEAD), CastItoXBytes((uint64)((double)(uint64)(g_App.m_pUploadQueue->GetUpDataOverheadFileRequest() + g_App.m_pPrefs->GetUpOverheadFileReq()) * avgModifier[mx])), CastItoIShort((uint64)((double)(uint64)(g_App.m_pUploadQueue->GetUpDataOverheadFileRequestPackets() + g_App.m_pPrefs->GetUpOverheadFileReqPackets()) * avgModifier[mx])));
							stattree.SetItemText(time_aap_up_oh[mx][i], cbuffer); i++;
							// Set up total source exch OH
							cbuffer.Format(GetResString(IDS_SSOVERHEAD), CastItoXBytes((uint64)((double)(uint64)(g_App.m_pUploadQueue->GetUpDataOverheadSourceExchange() + g_App.m_pPrefs->GetUpOverheadSrcEx()) * avgModifier[mx])), CastItoIShort((uint64)((double)(uint64)(g_App.m_pUploadQueue->GetUpDataOverheadSourceExchangePackets() + g_App.m_pPrefs->GetUpOverheadSrcExPackets()) * avgModifier[mx])));
							stattree.SetItemText(time_aap_up_oh[mx][i], cbuffer); i++;
							// Set up total server OH
							cbuffer.Format(GetResString(IDS_SOVERHEAD), CastItoXBytes((uint64)((double)(uint64)(g_App.m_pUploadQueue->GetUpDataOverheadServer() + g_App.m_pPrefs->GetUpOverheadServer()) * avgModifier[mx])), CastItoIShort((uint64)((double)(uint64)(g_App.m_pUploadQueue->GetUpDataOverheadServerPackets() + g_App.m_pPrefs->GetUpOverheadServerPackets()) * avgModifier[mx])));
							stattree.SetItemText(time_aap_up_oh[mx][i], cbuffer); i++;
							// Set up total others OH
							cbuffer.Format(GetResString(IDS_OOVERHEAD), CastItoXBytes((uint64)((double)(uint64)(g_App.m_pUploadQueue->GetUpDataOverheadOther() + g_App.m_pPrefs->GetUpOverheadOther()) * avgModifier[mx])), CastItoIShort((uint64)((double)(uint64)(g_App.m_pUploadQueue->GetUpDataOverheadOtherPackets() + g_App.m_pPrefs->GetUpOverheadOtherPackets()) * avgModifier[mx])));
							stattree.SetItemText(time_aap_up_oh[mx][i], cbuffer); i++;
						}
					} // - End Time Statistics -> Projected Averages -> Time Period -> Uploads Section
					// TIME STATISTICS -> PROJECTED AVERAGES -> TIME PERIOD -> DOWNLOADS SECTION
					if (forceUpdate || stattree.IsExpanded(time_aap_hdown[mx]))
					{
						g_App.m_pDownloadQueue->GetDownloadStats(adwDQSrc, aqwDQData);
						// Downloaded Data
						cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_DDATA), CastItoXBytes((uint64)((double)(uint64)(g_App.stat_sessionReceivedBytes+g_App.m_pPrefs->GetTotalDownloaded()) * avgModifier[mx])));
						stattree.SetItemText(time_aap_down[mx][0], cbuffer);
						if (forceUpdate || stattree.IsExpanded(time_aap_down[mx][0]))
						{
							// Downloaded Data By Client
							if (forceUpdate || stattree.IsExpanded(time_aap_down_hd[mx][0]))
							{
								double dDownDataTotal = g_App.m_pPrefs->GetDownTotalClientData() * avgModifier[mx];
								uint64 DownDataTotal = static_cast<uint64>(dDownDataTotal);
								uint64 DownDataClient;
								double dDownDataClient, percentClientTransferred;
								//eklmn: sequence was replaced to loop...
								int i = 0;
								for (int j = 0;j<SO_LAST;j++)
									if ((EnumClientTypes)j != SO_OLDEMULE)
									{
										if ((EnumClientTypes)j == SO_EMULE)
											dDownDataClient = (g_App.m_pPrefs->GetCumDownData((EnumClientTypes)j)
															+ g_App.m_pPrefs->GetCumDownData(SO_OLDEMULE)) * avgModifier[mx];
										else
											dDownDataClient = g_App.m_pPrefs->GetCumDownData((EnumClientTypes)j) * avgModifier[mx];
										DownDataClient = static_cast<uint64>(dDownDataClient);

										if ((DownDataTotal != 0) && (DownDataClient != 0))
											percentClientTransferred = 100.0 * dDownDataClient / dDownDataTotal;
										else
											percentClientTransferred = 0;
										cbuffer.Format(_T("%s: %s (%.1f%%)"), GetClientNameString((EnumClientTypes)j), CastItoXBytes(DownDataClient), percentClientTransferred);
										stattree.SetItemText( time_aap_down_dc[mx][i] , cbuffer );
										i++;
									}
							}
						}
						// Set Cum Completed Downloads
						cbuffer.Format(_T("%s: %I64u"), GetResString(IDS_STATS_COMPDL), (uint64)(g_App.m_pPrefs->GetDownCompletedFiles() * avgModifier[mx]));
						stattree.SetItemText(time_aap_down[mx][1], cbuffer);
						// Set Cum Download Sessions
						double	dGoodSessions = (uint32)(g_App.m_pPrefs->GetDownC_SuccessfulSessions() + adwDQSrc[STATS_DLSRC_TRANSFERRING]) * avgModifier[mx];
						uint32	statGoodSessions = static_cast<uint32>(dGoodSessions);
						double	dBadSessions = (double)g_App.m_pPrefs->GetDownC_FailedSessions() * avgModifier[mx];
						uint32	statBadSessions = static_cast<uint32>(dBadSessions);
						double	percentSessions;
						cbuffer.Format(_T("%s: %u"), GetResString(IDS_STATS_DLSES), statGoodSessions + statBadSessions);
						stattree.SetItemText(time_aap_down[mx][2], cbuffer);
						if (forceUpdate || stattree.IsExpanded(time_aap_down[mx][2]))
						{
							// Set Cum Successful Download Sessions
							if (statGoodSessions > 0) percentSessions = 100.0 * dGoodSessions / (dGoodSessions + dBadSessions);
							else percentSessions = 0;
							cbuffer.Format(_T("%s: %u (%.1f%%)"), GetResString(IDS_STATS_SDLSES), statGoodSessions, percentSessions);
							stattree.SetItemText( time_aap_down_s[mx][0] , cbuffer ); // Set Successful Sessions
							// Set Cum Failed Download Sessions
							if (percentSessions != 0 && statBadSessions > 0) percentSessions = 100 - percentSessions; // There were some good sessions and bad ones...
							else if (percentSessions == 0 && statBadSessions > 0) percentSessions = 100; // There were bad sessions and no good ones, must be 100%
							else percentSessions = 0; // No sessions at all, or no bad ones.
							cbuffer.Format(_T("%s: %u (%.1f%%)"), GetResString(IDS_STATS_FDLSES), statBadSessions, percentSessions);
							stattree.SetItemText( time_aap_down_s[mx][1] , cbuffer );
						}
						// Set Cumulative Gained Due To Compression
						cbuffer.Format(GetResString(IDS_STATS_GAINCOMP), CastItoXBytes((uint64)((double)(uint64)(g_App.m_pPrefs->GetSesSavedFromCompression() + g_App.m_pPrefs->GetCumSavedFromCompression()) * avgModifier[mx])));
						stattree.SetItemText(time_aap_down[mx][3], cbuffer);
						// Set Cumulative Lost Due To Corruption
						cbuffer.Format(GetResString(IDS_STATS_LOSTCORRUPT), CastItoXBytes((uint64)((double)(uint64)(g_App.m_pPrefs->GetSesLostFromCorruption() + g_App.m_pPrefs->GetCumLostFromCorruption()) * avgModifier[mx])));
						stattree.SetItemText(time_aap_down[mx][4], cbuffer);
						// Set Cumulative Saved Due To ICH
						cbuffer.Format(GetResString(IDS_STATS_ICHSAVED), (uint32)((g_App.m_pPrefs->GetSesPartsSavedByICH() + g_App.m_pPrefs->GetCumPartsSavedByICH()) * avgModifier[mx]));
						stattree.SetItemText(time_aap_down[mx][5], cbuffer);

						uint64 DownOHTotal = g_App.m_pDownloadQueue->GetDownDataOverheadFileRequest() + g_App.m_pDownloadQueue->GetDownDataOverheadSourceExchange() + g_App.m_pDownloadQueue->GetDownDataOverheadServer() + g_App.m_pDownloadQueue->GetDownDataOverheadOther();
						uint64 DownOHTotalPackets = g_App.m_pDownloadQueue->GetDownDataOverheadFileRequestPackets() + g_App.m_pDownloadQueue->GetDownDataOverheadSourceExchangePackets() + g_App.m_pDownloadQueue->GetDownDataOverheadServerPackets() + g_App.m_pDownloadQueue->GetDownDataOverheadOtherPackets();
						// Total Overhead
						cbuffer.Format(GetResString(IDS_TOVERHEAD), CastItoXBytes((uint64)((double)(uint64)(DownOHTotal + g_App.m_pPrefs->GetDownOverheadTotal()) * avgModifier[mx])), CastItoIShort((uint64)((double)(uint64)(DownOHTotalPackets + g_App.m_pPrefs->GetDownOverheadTotalPackets()) * avgModifier[mx])));
						stattree.SetItemText(time_aap_down[mx][6], cbuffer);
						if (forceUpdate || stattree.IsExpanded(time_aap_down[mx][6]))
						{
							int i = 0;
							// File Request Overhead
							cbuffer.Format(GetResString(IDS_FROVERHEAD), CastItoXBytes((uint64)((double)(uint64)(g_App.m_pDownloadQueue->GetDownDataOverheadFileRequest() + g_App.m_pPrefs->GetDownOverheadFileReq()) * avgModifier[mx])), CastItoIShort((uint64)((double)(uint64)(g_App.m_pDownloadQueue->GetDownDataOverheadFileRequestPackets() + g_App.m_pPrefs->GetDownOverheadFileReqPackets()) * avgModifier[mx])));
							stattree.SetItemText(time_aap_down_oh[mx][i], cbuffer); i++;
							// Source Exchange Overhead
							cbuffer.Format(GetResString(IDS_SSOVERHEAD), CastItoXBytes((uint64)((double)(uint64)(g_App.m_pDownloadQueue->GetDownDataOverheadSourceExchange()+g_App.m_pPrefs->GetDownOverheadSrcEx()) * avgModifier[mx])), CastItoIShort((uint64)((double)(uint64)(g_App.m_pDownloadQueue->GetDownDataOverheadSourceExchangePackets()+g_App.m_pPrefs->GetDownOverheadSrcExPackets()) * avgModifier[mx])));
							stattree.SetItemText(time_aap_down_oh[mx][i], cbuffer); i++;
							// Server Overhead
							cbuffer.Format(GetResString(IDS_SOVERHEAD), CastItoXBytes((uint64)((double)(uint64)(g_App.m_pDownloadQueue->GetDownDataOverheadServer()+g_App.m_pPrefs->GetDownOverheadServer()) * avgModifier[mx])), CastItoIShort((uint64)((double)(uint64)(g_App.m_pDownloadQueue->GetDownDataOverheadServerPackets()+g_App.m_pPrefs->GetDownOverheadServerPackets()) * avgModifier[mx])));
							stattree.SetItemText(time_aap_down_oh[mx][i], cbuffer); i++;
							// Others Overhead
							cbuffer.Format(GetResString(IDS_OOVERHEAD), CastItoXBytes((uint64)((double)(uint64)(g_App.m_pDownloadQueue->GetDownDataOverheadOther()+g_App.m_pPrefs->GetDownOverheadOther()) * avgModifier[mx])), CastItoIShort((uint64)((double)(uint64)(g_App.m_pDownloadQueue->GetDownDataOverheadOtherPackets()+g_App.m_pPrefs->GetDownOverheadOtherPackets()) * avgModifier[mx])));
							stattree.SetItemText(time_aap_down_oh[mx][i], cbuffer); i++;
						}
					} // - End Time Statistics -> Projected Averages -> Time Period -> Downloads Section
				} // - End Time Statistics -> Projected Averages -> Time Period Sections
			} // - End Time Statistics -> Projected Averages Section
		} // - End Time Statistics -> Projected Averages Section Loop
	} // - END TIME STATISTICS SECTION


	// CLIENTS SECTION		[Original idea and code by xrmb]
	//						Note:	This section now has dynamic tree items.  This technique
	//								may appear in other areas, however, there is usually an
	//								advantage to displaying 0 datems.  Here, with the ver-
	//								sions being displayed the way they are, it makes sense.
	//								Who wants to stare at totally blank tree items?  ;)
	if (forceUpdate || stattree.IsExpanded(h_clients))
	{
		CMap<POSITION, POSITION, uint32, uint32>	clientMODs(200), clientPlusMODs;
		CMap<int, int, uint32, uint32>				clientCountries;

		uint32	totalclient, totalMODs, dwTotalPlusMODs, myStats[18];
		ClientsData 	AllClients;	// xrmb : statsclientstatus
		// get clientversion-counts
		g_App.m_pClientList->GetStatistics(totalclient, myStats, &AllClients, totalMODs, &dwTotalPlusMODs, &clientMODs, &clientPlusMODs, &clientCountries);
		//eklmn: check NULL exception
		if (totalclient == 0)
		{
			totalclient = 1;

			cbuffer.Format(_T("%s: %u"), GetResString(IDS_STATS_ONLISTCLIENTS), 0);
			stattree.SetItemText(hcliconnected, cbuffer);

			cbuffer.Format(_T("%s: %u (0%%)"), GetResString(IDS_STATS_SUI), myStats[14]);
			stattree.SetItemText(cligen[0], cbuffer);

			cbuffer.Format(_T("%s: %u (0%%)"), GetResString(IDS_STATS_NONSUI), myStats[15]);
			stattree.SetItemText(cligen[1], cbuffer);

			cbuffer.Format(_T("%s: %u (0%%)"), GetResString(IDS_LOWID), myStats[16]);
			stattree.SetItemText(cligen[2], cbuffer);
		}
		else
		{
			cbuffer.Format(_T("%s: %u"), GetResString(IDS_STATS_ONLISTCLIENTS), totalclient);
			stattree.SetItemText(hcliconnected, cbuffer);

			cbuffer.Format(_T("%s: %u (%.1f%%)"), GetResString(IDS_STATS_SUI), myStats[14], static_cast<double>(100*myStats[14])/totalclient);
			stattree.SetItemText(cligen[0], cbuffer);

			cbuffer.Format(_T("%s: %u (%.1f%%)"), GetResString(IDS_STATS_NONSUI), myStats[15], static_cast<double>(100*myStats[15])/totalclient);
			stattree.SetItemText(cligen[1], cbuffer);

			cbuffer.Format(_T("%s: %u (%.1f%%)"), GetResString(IDS_LOWID), myStats[16], static_cast<double>(100*myStats[16])/totalclient);
			stattree.SetItemText(cligen[2], cbuffer);
		}

		// CLIENTS -> CLIENT SOFTWARE SECTION
		if (forceUpdate || stattree.IsExpanded(hcliconnected))
		{
			uint32 dwTotalMules = myStats[SO_EMULE] + myStats[SO_OLDEMULE];

			if (dwTotalMules)
				cbuffer.Format(_T("%s: %u (%.1f%%)"), GetResString(IDS_STATS_CLIENT_MODS), totalMODs,100.0*totalMODs/dwTotalMules);
			else
				cbuffer.Format(_T("%s: %u (0.0%%)"), GetResString(IDS_STATS_CLIENT_MODS), totalMODs);
			stattree.SetItemText(hclisoftMODs, cbuffer);
			//eklmn: delete old stuff
			for (int im = 0; im < STAT_CLIENT_MOD_NODES; im++)
			{
				if (clisoftMODs[im] != NULL) stattree.DeleteChildItems(clisoftMODs[im]);
				m_nMODs[im] = 0;
			}
		//	Build a new MOD's subtree
			if (totalMODs)
			{
				uint32	dwLastTop = 0xFFFFFFFF;

				while (!clientMODs.IsEmpty())
				{
					POSITION	MOD_pos, top_pos = 0, pos = clientMODs.GetStartPosition();
					uint32		dwMODCnt, dwCurrTop = 0;

					while(pos)
					{
						clientMODs.GetNextAssoc(pos, MOD_pos, dwMODCnt);
						if ((dwCurrTop < dwMODCnt) && (dwMODCnt <= dwLastTop))
						{
							top_pos = MOD_pos;
							dwCurrTop = dwMODCnt;
						}
					}
					dwLastTop = dwCurrTop;

					if (top_pos)
					{
					// Sort clients by known types
						g_App.m_pClientList->GetMODType(top_pos, &strBuf2);
						for (uint32 dwModIdx = 0; dwModIdx < STAT_CLIENT_MOD_NODES; dwModIdx++)
						{
							if ((dwModIdx == (STAT_CLIENT_MOD_NODES - 1)) || (strBuf2.Find(GetMODType(dwModIdx)) >= 0))
							{
								m_nMODs[dwModIdx] += dwLastTop;
								// check item
								if (clisoftMODs[dwModIdx] == NULL)
								{
								// Delete "Others"
									if ((dwModIdx != (STAT_CLIENT_MOD_NODES - 1)) && (clisoftMODs[STAT_CLIENT_MOD_NODES - 1] != NULL))
									{
										stattree.DeleteItem(clisoftMODs[STAT_CLIENT_MOD_NODES - 1]);
										clisoftMODs[STAT_CLIENT_MOD_NODES - 1] = NULL;
									}
									clisoftMODs[dwModIdx] = stattree.InsertItem(cbuffer, hclisoftMODs);
								}
								// now insert a string
								cbuffer.Format(_T("%s: %u"), strBuf2, dwLastTop);
								stattree.InsertItem(cbuffer, clisoftMODs[dwModIdx]);
								break;
							}
						}
						clientMODs.RemoveKey(top_pos);
					}
				}
				for (uint32 dwModIdx = 0; dwModIdx < STAT_CLIENT_MOD_NODES; dwModIdx++)
				{
					if (clisoftMODs[dwModIdx] != NULL)
					{
						cbuffer.Format(_T("%s: %u (%.1f%%)"), GetMODType(dwModIdx), m_nMODs[dwModIdx], 100.0*m_nMODs[dwModIdx]/dwTotalMules);
						stattree.SetItemText(clisoftMODs[dwModIdx], cbuffer);
					}
				}
			}

			cbuffer.Format(_T("%s: %u (%.1f%%)"), GetResString(IDS_STATS_CLIENT_MODS),
				dwTotalPlusMODs, (myStats[SO_PLUS]) ? (100.0 * dwTotalPlusMODs / myStats[SO_PLUS]) : 0.0);
			stattree.SetItemText(hclisoftPlusMODs, cbuffer);
			stattree.DeleteChildItems(hclisoftPlusMODs);
		//	Build a new MOD's subtree
			if (dwTotalPlusMODs)
			{
				uint32	dwLastTop = 0xFFFFFFFF;

				while (!clientPlusMODs.IsEmpty())
				{
					POSITION	MOD_pos, top_pos = 0, pos = clientPlusMODs.GetStartPosition();
					uint32		dwMODCnt, dwCurrTop = 0;

					while(pos)
					{
						clientPlusMODs.GetNextAssoc(pos, MOD_pos, dwMODCnt);
						if ((dwCurrTop < dwMODCnt) && (dwMODCnt <= dwLastTop))
						{
							top_pos = MOD_pos;
							dwCurrTop = dwMODCnt;
						}
					}
					dwLastTop = dwCurrTop;

					if (top_pos)
					{
						g_App.m_pClientList->GetMODType(top_pos, &strBuf2);
						cbuffer.Format(_T("%s: %u"), strBuf2, dwLastTop);
						stattree.InsertItem(cbuffer, hclisoftPlusMODs);
						clientPlusMODs.RemoveKey(top_pos);
					}
				}
			}

			uint32	clients;
			double	percentClients = 0;
			int it = 0;
			for (int j = 0; j < SO_LAST; j++)
				if ((EnumClientTypes)j != SO_OLDEMULE)
				{
					clients = myStats[j];
					if ((EnumClientTypes)j == SO_EMULE)
						clients += myStats[SO_OLDEMULE];
					percentClients = static_cast<double>(100*clients)/totalclient;
					cbuffer.Format(_T("%s: %u (%.1f%%)"), GetClientNameString((EnumClientTypes)j), clients, percentClients);
					stattree.SetItemText(clisoft[it], cbuffer);
					//eklmn: here we are building subtrees
					switch ((EnumClientTypes)j)
					{
						case SO_PLUS:
						case SO_EMULE:
						case SO_EDONKEYHYBRID:
						case SO_XMULE:
						case SO_MLDONKEY:
						case SO_AMULE:
						case SO_SHAREAZA:
						case SO_LPHANT:
							if (forceUpdate || stattree.IsExpanded(clisoft[it]) || clients != 0)
							{
								//eklmn: since "other versions" must be always on the bottom of the list, let's delete it,
								//           if last of the 4 top values is empty
								if (cli_versions_other[j])
								{
									stattree.DeleteItem(cli_versions_other[j]);
									cli_versions_other[j] = NULL;
								}
								//uint32 totcnt = 0;
								//uint32 verCount = 0;
								//--- find top 4 eDonkey client versions ---
								uint64	currtop = 0;
								uint64	lasttop = 0xFFFFFFFFFFFFFFFF;
								uint32	SubTotal = 0;
								for (uint32 i = 0; i < 4; i++)
								{
									POSITION pos = AllClients.m_pClients[j].GetStartPosition();
									uint32 topver=0;
									uint32 topcnt=0;
									uint64 cntver=0;
									uint32 ver;
									uint32 cnt;
									while(pos)
									{
										AllClients.m_pClients[j].GetNextAssoc(pos, ver, cnt);
										cntver = cnt; cntver <<= 32; cntver += ver;	//SyruS sort by popularity and version
										if(currtop<cntver && cntver<lasttop)
										{
											topver = ver;
											topcnt = cnt;
											currtop = cntver;
										}
									}
									lasttop=currtop;
									currtop=0;

									if (topcnt)
									{
										SubTotal += topcnt;
										cbuffer.Format(_T("%s: %u (%.1f%%)"), GetClientVersionString((EnumClientTypes)j,topver), topcnt, static_cast<double>(topcnt)/clients*100.0);
										//eklmn: if we found something just replaced a text
										if (cli_versions[j][i] != NULL)
											stattree.SetItemText(cli_versions[j][i], cbuffer);
										else
											cli_versions[j][i] = stattree.InsertItem(cbuffer, clisoft[it]);
									}
									else
									{
										if (cli_versions[j][i] != NULL)
										{
											stattree.DeleteItem(cli_versions[j][i]);
											cli_versions[j][i] = NULL;
										}
									}
								}
								//eklmn: check a other clients
								if ((clients-SubTotal) != 0)
								{
									cbuffer.Format(_T("%s: %u (%.1f%%)"), GetResString(IDS_STATS_OTHERS), (clients-SubTotal),100.0*(clients-SubTotal)/clients);
									if (cli_versions_other[j])
										stattree.SetItemText(cli_versions_other[j], cbuffer);
									else
										cli_versions_other[j] = stattree.InsertItem(cbuffer, clisoft[it]);
								}

							}
						default:
							break;
					}
					it++;
				}
		} // - End Clients -> Client Software Section
		// General Client Statistics
		cbuffer.Format(_T("%s: %u"), GetResString(IDS_STATS_PROBLEMATIC), myStats[17]);stattree.SetItemText(cligen[3], cbuffer);
		cbuffer.Format(_T("%s: %u"), GetResString(IDS_BANNED), g_App.m_pUploadQueue->GetBanCount()); stattree.SetItemText(cligen[4], cbuffer);

		//Filtered ... subtree
		cbuffer.Format(_T("%s: %u"), GetResString(IDS_STATS_FILTEREDCLIENTS),g_App.m_lTotalFiltered); stattree.SetItemText(hFiltered, cbuffer);
		cbuffer.Format(_T("%s: %u"), GetResString(IDS_STATS_FILTEREDCLIENTS_IN),g_App.m_lIncomingFiltered); stattree.SetItemText(hFilteredItems[0], cbuffer);
		cbuffer.Format(_T("%s: %u"), GetResString(IDS_STATS_FILTEREDCLIENTS_OUT),g_App.m_lOutgoingFiltered); stattree.SetItemText(hFilteredItems[1], cbuffer);
		cbuffer.Format(_T("%s: %u"), GetResString(IDS_STATS_FILTEREDCLIENTS_SX),g_App.m_lSXFiltered); stattree.SetItemText(hFilteredItems[2], cbuffer);

	//	Countries subtree
		if (forceUpdate || stattree.IsExpanded(h_clients))
		{
			stattree.DeleteChildItems(hCountries);
			if (g_App.m_pIP2Country->IsIP2Country())
			{
				if (totalclient)
				{
					uint32	dwLastTop = 0xFFFFFFFF, dwCountryTotal = 0;

					while (!clientCountries.IsEmpty())
					{
						POSITION	pos = clientCountries.GetStartPosition();
						uint32		dwCountryCnt, dwCurrTop = 0;
						int			iCountryIdx, iTopIdx = 0;

						while(pos)
						{
							clientCountries.GetNextAssoc(pos, iCountryIdx, dwCountryCnt);
							if ((dwCurrTop < dwCountryCnt) && (dwCountryCnt <= dwLastTop))
							{
								iTopIdx = iCountryIdx;
								dwCurrTop = dwCountryCnt;
							}
						}
						if ((dwLastTop = dwCurrTop) != 0)
						{
							cbuffer.Format( _T("%s: %u (%.1f%%)"), g_App.m_pIP2Country->GetCountryNameByIndex(static_cast<uint16>(iTopIdx)),
								dwLastTop, static_cast<double>(100 * dwLastTop) / totalclient );
							stattree.InsertItem(cbuffer, hCountries);
							clientCountries.RemoveKey(iTopIdx);
							dwCountryTotal++;
						}
					}
					cbuffer.Format(_T("%s: %u"), GetResString(IDS_COUNTRIES), dwCountryTotal);
				}
				else
					cbuffer.Format(_T("%s: <%s>"), GetResString(IDS_COUNTRIES), GetResString(IDS_FSTAT_WAITING));
			}
			else
				cbuffer.Format(_T("%s: <%s>"), GetResString(IDS_COUNTRIES), GetResString(IDS_DISABLED));
			stattree.SetItemText(hCountries, cbuffer);
		} // - End Countries
	} // - END CLIENTS SECTION


	// UPDATE RECORDS FOR SERVERS AND SHARED FILES
	g_App.m_pPrefs->SetRecordStructMembers();

	// SERVERS SECTION
	if (forceUpdate || stattree.IsExpanded(h_servers))
	{
		// Get stat values
		uint32	servtotal, servfail, servuser, servfile, servtuser, servtfile, dwSrvLowIdUsers;
		double	servocc;
		g_App.m_pServerList->GetServersStatus(servtotal, servfail, servuser, servfile, dwSrvLowIdUsers, servtuser, servtfile, servocc);
		// Set working servers value
		cbuffer.Format(_T("%s: %u"),GetResString(IDS_SF_WORKING),servtotal-servfail);stattree.SetItemText(srv[0], cbuffer);
		if (forceUpdate || stattree.IsExpanded(srv[0]))
		{
			// Set users on working servers value
			cbuffer.Format(_T("%s: %s; %s: %s (%.1f%%)"),GetResString(IDS_SF_WUSER),CastItoThousands(servuser),GetResString(IDS_LOWID),CastItoThousands(dwSrvLowIdUsers),servuser ? (dwSrvLowIdUsers * 100.0 / servuser) : 0.0);stattree.SetItemText(srv_w[0], cbuffer);
			// Set files on working servers value
			cbuffer.Format(_T("%s: %s"),GetResString(IDS_SF_WFILE),CastItoThousands(servfile));stattree.SetItemText(srv_w[1], cbuffer);
			// Set server occ value
			cbuffer.Format(GetResString(IDS_SF_SRVOCC),servocc);stattree.SetItemText(srv_w[2], cbuffer);
		}
		// Set failed servers value
		cbuffer.Format(_T("%s: %u"),GetResString(IDS_SF_FAIL),servfail);stattree.SetItemText(srv[1], cbuffer);
		// Set deleted servers value
		cbuffer.Format(_T("%s: %u"),GetResString(IDS_SF_DELCOUNT),g_App.m_pServerList->GetDeletedServerCount());stattree.SetItemText(srv[2], cbuffer);
		// Set total servers value
		cbuffer.Format(_T("%s: %u"),GetResString(IDS_SF_TOTAL),servtotal);stattree.SetItemText(srv[3], cbuffer);
		// Set total users value
		cbuffer.Format(_T("%s: %s"),GetResString(IDS_SF_USER),CastItoThousands(servtuser));stattree.SetItemText(srv[4], cbuffer);
		// Set total files value
		cbuffer.Format(_T("%s: %s"),GetResString(IDS_SF_FILE),CastItoThousands(servtfile));stattree.SetItemText(srv[5], cbuffer);
		// UDP Search status
		g_App.m_pDownloadQueue->GetUDPSearchStatus(&strBuf2);
		cbuffer.Format(GetResString(IDS_STATS_UDPSEARCH), strBuf2);
		stattree.SetItemText(srv[6], cbuffer);
		// SERVERS -> RECORDS SECTION
		if (forceUpdate || stattree.IsExpanded(hsrv_records))
		{
			// Set most working servers
			cbuffer.Format(_T("%s: %u"), GetResString(IDS_STATS_SVRECWORKING), g_App.m_pPrefs->GetSrvrsMostWorkingServers() );
			stattree.SetItemText(srv_r[0], cbuffer);
			// Set most users online
			cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_SVRECUSERS), CastItoThousands(g_App.m_pPrefs->GetSrvrsMostUsersOnline()) );
			stattree.SetItemText(srv_r[1], cbuffer);
			// Set most files avail
			cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_SVRECFILES), CastItoThousands(g_App.m_pPrefs->GetSrvrsMostFilesAvail()) );
			stattree.SetItemText(srv_r[2], cbuffer);
		} // - End Servers -> Records Section
	} // - END SERVERS SECTION


	// SHARED FILES SECTION
	if (forceUpdate || stattree.IsExpanded(h_shared))
	{
		// Set Number of Shared Files
		cbuffer.Format(GetResString(IDS_SHAREDFILESCOUNT),g_App.m_pSharedFilesList->GetCount());
		stattree.SetItemText(shar[0], cbuffer);
		// Set Average File Size
		uint64 qwLargestFile;
		uint64 allsize = g_App.m_pSharedFilesList->GetDatasize(&qwLargestFile); // returns total share size as well as largest filesize

		cbuffer.Format( GetResString(IDS_SF_AVERAGESIZE),
			CastItoXBytes((g_App.m_pSharedFilesList->GetCount() != 0) ? (allsize / g_App.m_pSharedFilesList->GetCount()) : 0) );
		stattree.SetItemText(shar[1], cbuffer);
		// Set Largest File Size
		cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_LARGESTFILE), CastItoXBytes(qwLargestFile));
		stattree.SetItemText(shar[2], cbuffer);
		// Set Total Share Size
		cbuffer.Format(GetResString(IDS_SF_SIZE),CastItoXBytes(allsize));
		stattree.SetItemText(shar[3], cbuffer);

		// SHARED FILES -> RECORDS SECTION
		if (forceUpdate || stattree.IsExpanded(hshar_records))
		{
			// Set Most Files Shared
			cbuffer.Format(_T("%s: %u"), GetResString(IDS_STATS_SHRECNUM), g_App.m_pPrefs->GetSharedMostFilesShared() );
			stattree.SetItemText(shar_r[0], cbuffer);
			// Set largest avg file size
			cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_SHRECASIZE), CastItoXBytes(g_App.m_pPrefs->GetSharedLargestAvgFileSize()) );
			stattree.SetItemText(shar_r[1], cbuffer);
			// Set largest file size
			cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_LARGESTFILE), CastItoXBytes(g_App.m_pPrefs->GetSharedLargestFileSize()) );
			stattree.SetItemText(shar_r[2], cbuffer);
			// Set largest share size
			cbuffer.Format(_T("%s: %s"), GetResString(IDS_STATS_SHRECSIZE), CastItoXBytes(g_App.m_pPrefs->GetSharedLargestShareSize()) );
			stattree.SetItemText(shar_r[3], cbuffer);
		} // - End Shared Files -> Records Section
	} // - END SHARED FILES SECTION

	// - End Set Tree Values

	stattree.SetRedraw(true);
} // ShowStatistics(bool forceRedraw = false){}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CStatisticsDlg::Localize()
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_STATIC_D3, IDS_ST_DOWNLOAD },
		{ IDC_STATIC_U, IDS_ST_UPLOAD },
		{ IDC_STATIC_A, IDS_SP_ACTCON },
		{ IDC_BNMENU, IDS_STATISTICS }
	};
	static const uint16 s_auResTbl2[][2] =
	{
		{ IDC_STATIC_D, IDS_ST_CURRENT },
		{ IDC_STATIC_U2, IDS_ST_CURRENT },
		{ IDC_STATIC_D2, IDS_ST_SESSION },
		{ IDC_STATIC_U3, IDS_ST_SESSION },
		{ IDC_STATIC_S2, IDS_ST_ACTIVEDOWNLOAD },
		{ IDC_STATIC_S1, IDS_ST_ACTIVEUPLOAD }
	};
	CString	strBuffer;

	for (uint32 i = 0; i < ARRSIZE(s_auResTbl); i++)
	{
		GetResString(&strBuffer, static_cast<UINT>(s_auResTbl[i][1]));
		SetDlgItemText(s_auResTbl[i][0], strBuffer);
	}

	for (uint32 i = 0; i < ARRSIZE(s_auResTbl2); i++)
	{
		GetResString(&strBuffer, static_cast<UINT>(s_auResTbl2[i][1]));
		strBuffer.MakeLower();
		SetDlgItemText(s_auResTbl2[i][0], strBuffer);
	}

	GetResString(&strBuffer, IDS_PW_GENERAL);
	strBuffer.MakeLower();
	if (g_App.m_pPrefs->GetGraphRatio() == 255) // 255 magic number for %
		strBuffer += _T(" (%)");
	else
		strBuffer.AppendFormat(_T(" (1:%u)"), g_App.m_pPrefs->GetGraphRatio());
	SetDlgItemText(IDC_STATIC_S0, strBuffer);
	strBuffer.Format( _T("%s (%u %s)"), GetResString(IDS_AVG).MakeLower(),
		g_App.m_pPrefs->GetStatsAverageMinutes(), GetResString(IDS_MINS) );
	SetDlgItemText(IDC_TIMEAVG1, strBuffer);
	SetDlgItemText(IDC_TIMEAVG2, strBuffer);

	if (IsRightToLeftLanguage())
		GetDlgItem(IDC_STATTREE)->ModifyStyleEx(0, WS_EX_RTLREADING);
	else
		GetDlgItem(IDC_STATTREE)->ModifyStyleEx(WS_EX_RTLREADING, 0);

	CreateStatsTree();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Menu Button: Displays the menu of stat tree commands.
void CStatisticsDlg::OnMenuButtonClicked()
{
	CRect rectBn;
	CPoint thePoint;
	CSize theSize;
	GetDlgItem(IDC_BNMENU)->GetWindowRect(&rectBn);
	thePoint = rectBn.TopLeft();
	theSize.SetSize(42,18);
	stattree.DoMenu(thePoint.operator+(theSize));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CStatisticsDlg::CreateStatsTree()
{
	CString buffer;

	stattree.SetRedraw(false);

	EMULE_TRY

	stattree.DeleteAllItems();

//	Setup Tree
//	Transfer section
	h_transfer = stattree.InsertItem(GetResString(IDS_TRANSFER_NOUN), 1, 1);
		buffer.Format(_T("%s %s"), GetResString(IDS_STATS_SRATIO), GetResString(IDS_FSTAT_WAITING));			// Make It Pretty
		trans[0]= stattree.InsertItem(buffer, h_transfer);												// Session Ratio
		buffer.Format(_T("%s %s"), GetResString(IDS_STATS_CRATIO), GetResString(IDS_FSTAT_WAITING));			// Make It Pretty
		trans[1]= stattree.InsertItem(buffer, h_transfer);												// Cumulative Ratio

//	Upload section
	h_upload = stattree.InsertItem(GetResString(IDS_TW_UPLOADS), 6,6,h_transfer);
	//	UL: session
		h_up_session= stattree.InsertItem(GetResString(IDS_STATS_SESSION), 8,8,h_upload);				// Session Section (Uploads)
		for(int i = 0; i<4; i++) up_S[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), h_up_session);
				hup_scb= stattree.InsertItem(GetResString(IDS_CLIENTS),up_S[0]);						// Clients Section
				for(int i = 0; i<(SO_LAST-1); i++) up_scb[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), hup_scb);
				hup_ssb= stattree.InsertItem(GetResString(IDS_STATS_DATASOURCE),up_S[0]);				// Data Source Section
				for(int i = 0; i<2; i++) up_ssb[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), hup_ssb);
				hup_scomb= stattree.InsertItem(GetResString(IDS_COMMUNITY), up_S[0]);		// Community Section
				for(int i = 0; i<2; i++) up_scomb[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), hup_scomb);
				hULPrioDataNode = stattree.InsertItem(GetResString(IDS_PRIORITY),up_S[0]);		// FilePriority Section
				for(int i = 0; i<5; i++) hULPrioDataItems[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), hULPrioDataNode);
			//	upload session section
				for(int i = 0; i<4; i++) up_ssessions[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), up_S[3]);
					// successful sessions
					for(int i = 0; i<2; i++) up_ssessions_s[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), up_ssessions[0]);
					for(int i = 0; i<ETS_TERMINATOR; i++) up_ssessions_spc[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), up_ssessions_s[1]);
					// failed sessions
					for(int i = 0; i<ETS_TERMINATOR; i++) up_ssessions_f[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), up_ssessions[1]);
				hup_soh= stattree.InsertItem(GetResString(IDS_STATS_OVRHD),h_up_session);				// Upline Overhead (Session)
				for(int i = 0; i<4; i++) up_soh[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), hup_soh);
	//	UL: cumulative
		h_up_total= stattree.InsertItem(GetResString(IDS_STATS_CUMULATIVE),9,9, h_upload);				// Cumulative Section (Uploads)
		up_T[0]= stattree.InsertItem(GetResString(IDS_FSTAT_WAITING),h_up_total);						// Uploaded Data (Total)
			hup_tcb= stattree.InsertItem(GetResString(IDS_CLIENTS),up_T[0]);							// Clients Section
			for(int i = 0; i<(SO_LAST-1); i++) up_tcb[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), hup_tcb);
			hup_tsb= stattree.InsertItem(GetResString(IDS_STATS_DATASOURCE),up_T[0]);					// Data Source Section
			for(int i = 0; i<2; i++) up_tsb[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), hup_tsb);
			hup_tcomb= stattree.InsertItem(GetResString(IDS_COMMUNITY), up_T[0]);			// Community Section
			for(int i = 0; i<2; i++) up_tcomb[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), hup_tcomb);
			up_T[1]= stattree.InsertItem(GetResString(IDS_FSTAT_WAITING),h_up_total);					// Upload Sessions (Total)
			for(int i = 0; i<4; i++) up_tsessions[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), up_T[1]);
			hup_toh= stattree.InsertItem(GetResString(IDS_STATS_OVRHD),h_up_total);						// Upline Overhead (Total)
			for(int i = 0; i<4; i++) up_toh[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), hup_toh);

//	Download section
	h_download = stattree.InsertItem(GetResString(IDS_TW_DOWNLOADS), 7,7,h_transfer);
	//	DL: session
		h_down_session= stattree.InsertItem(GetResString(IDS_STATS_SESSION),8,8, h_download);			// Session Section (Downloads)
		for(int i = 0; i<17; i++) down_S[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), h_down_session);
			hdown_scb= stattree.InsertItem(GetResString(IDS_CLIENTS),down_S[0]);						// Clients Section
			for(int i = 0; i<(SO_LAST-1); i++) down_scb[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), hdown_scb);
			for(int i = 0; i < ARRSIZE(down_sources); i++) down_sources[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), down_S[12]);
			for(int i = 0; i<4; i++) down_ssessions[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), down_S[13]);
				htiDLFailedSesNRD = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), down_ssessions[1]);
			hdown_soh= stattree.InsertItem(GetResString(IDS_STATS_OVRHD),h_down_session);				// Downline Overhead (Session)
			for(int i = 0; i<4; i++) down_soh[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), hdown_soh);

	//	DL: cumulative
		h_down_total= stattree.InsertItem(GetResString(IDS_STATS_CUMULATIVE),9,9, h_download);			// Cumulative Section (Downloads)
		for(int i = 0; i<6; i++) down_T[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), h_down_total);
			hdown_tcb= stattree.InsertItem(GetResString(IDS_CLIENTS),down_T[0]);						// Clients Section
			for(int i = 0; i<(SO_LAST-1); i++) down_tcb[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), hdown_tcb);
			for(int i = 0; i<4; i++) down_tsessions[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), down_T[2]);
			hdown_toh= stattree.InsertItem(GetResString(IDS_STATS_OVRHD),h_down_total);					// Downline Overhead (Total)
			for(int i = 0; i<4; i++) down_toh[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), hdown_toh);

//	Connection section
	h_connection = stattree.InsertItem(GetResString(IDS_FSTAT_CONNECTION),2,2);
	//	Connection: session
		h_conn_session= stattree.InsertItem(GetResString(IDS_STATS_SESSION),8,8,h_connection);		// Session Section (Connection)
			hconn_sg= stattree.InsertItem(GetResString(IDS_PW_GENERAL), 11, 11, h_conn_session);	// General Section (Session)
			for(int i = 0; i<5; i++) conn_sg[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), hconn_sg);
			hconn_su= stattree.InsertItem(GetResString(IDS_UPLOAD_NOUN),6,6,h_conn_session);		// Uploads Section (Session)
			for(int i = 0; i<4; i++) conn_su[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), hconn_su);
			hconn_sd= stattree.InsertItem(GetResString(IDS_DOWNLOAD_NOUN),7,7,h_conn_session);			// Downloads Section (Session)
			for(int i = 0; i<4; i++) conn_sd[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), hconn_sd);
	//	Connection: cumulative
		h_conn_total= stattree.InsertItem(GetResString(IDS_STATS_CUMULATIVE),9,9,h_connection);		// Cumulative Section (Connection)
			hconn_tg= stattree.InsertItem(GetResString(IDS_PW_GENERAL), 11, 11, h_conn_total);		// General (Total)
			for(int i = 0; i<4; i++) conn_tg[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), hconn_tg);
			hconn_tu= stattree.InsertItem(GetResString(IDS_UPLOAD_NOUN),6,6,h_conn_total);			// Uploads (Total)
			for(int i = 0; i<3; i++) conn_tu[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), hconn_tu);
			hconn_td= stattree.InsertItem(GetResString(IDS_DOWNLOAD_NOUN),7,7,h_conn_total);				// Downloads (Total)
			for(int i = 0; i<3; i++) conn_td[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), hconn_td);

//	Time section
	h_time = stattree.InsertItem(GetResString(IDS_STATS_TIMESTATS),12,12);							// Time Statistics Section
	for(int i = 0; i<2; i++) tvitime[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), h_time);
	//	Time: session
		htime_s = stattree.InsertItem(GetResString(IDS_STATS_SESSION),8,8,h_time);					// Session Section (Time)
			for(int i = 0; i<4; i++) tvitime_s[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), htime_s);
			for(int i = 0; i<2; i++) tvitime_st[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), tvitime_s[1]);
	//	Time: cumulative
		htime_t = stattree.InsertItem(GetResString(IDS_STATS_CUMULATIVE),9,9,h_time);				// Cumulative Section (Time)
			for(int i = 0; i<3; i++) tvitime_t[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), htime_t);
			for(int i = 0; i<2; i++) tvitime_tt[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), tvitime_t[1]);

		htime_aap = stattree.InsertItem(GetResString(IDS_STATS_AVGANDPROJ),13,13,h_time);			// Projected Averages Section
			time_aaph[0] = stattree.InsertItem(GetResString(IDS_STATS_DAYLY),14,14,htime_aap);		// Daily Section
			time_aaph[1] = stattree.InsertItem(GetResString(IDS_STATS_MONTHLY),15,15,htime_aap);	// Monthly Section
			time_aaph[2] = stattree.InsertItem(GetResString(IDS_STATS_YEARLY),16,16,htime_aap);		// Yearly Section
			for(int x = 0; x<3; x++)
			{
				time_aap_hup[x] = stattree.InsertItem(GetResString(IDS_TW_UPLOADS),6,6,time_aaph[x]);	// Upload Section
				for(int i = 0; i<3; i++) time_aap_up[x][i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING),time_aap_hup[x]);

				time_aap_up_hd[x][0] = stattree.InsertItem(GetResString(IDS_CLIENTS),time_aap_up[x][0]);					// Clients Section
				for(int i = 0; i<(SO_LAST-1); i++) time_aap_up_dc[x][i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), time_aap_up_hd[x][0]);
				time_aap_up_hd[x][1] = stattree.InsertItem(GetResString(IDS_STATS_DATASOURCE),time_aap_up[x][0]);			// Data Source Section
				for(int i = 0; i<2; i++) time_aap_up_ds[x][i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), time_aap_up_hd[x][1]);
				time_aap_up_hd[x][2] = stattree.InsertItem(GetResString(IDS_COMMUNITY), time_aap_up[x][0]);		// Community Section
				for(int i = 0; i<2; i++) time_aap_up_com[x][i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), time_aap_up_hd[x][2]);
				for(int i = 0; i<2; i++) time_aap_up_s[x][i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), time_aap_up[x][1]);
				for(int i = 0; i<4; i++) time_aap_up_oh[x][i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), time_aap_up[x][2]);

				time_aap_hdown[x] = stattree.InsertItem(GetResString(IDS_TW_DOWNLOADS),7,7,time_aaph[x]);					// Download Section
				for(int i = 0; i<7; i++) time_aap_down[x][i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING),time_aap_hdown[x]);
				time_aap_down_hd[x][0] = stattree.InsertItem(GetResString(IDS_CLIENTS),time_aap_down[x][0]);				// Clients Section
				for(int i = 0; i<(SO_LAST-1); i++) time_aap_down_dc[x][i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), time_aap_down_hd[x][0]);
				for(int i = 0; i<2; i++) time_aap_down_s[x][i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), time_aap_down[x][2]);
				for(int i = 0; i<4; i++) time_aap_down_oh[x][i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), time_aap_down[x][6]);
			}

//	Client section
	h_clients = stattree.InsertItem(GetResString(IDS_CLIENTS),3,3);															// Clients Section
		hcliconnected = stattree.InsertItem(GetResString(IDS_STATS_ONLISTCLIENTS),h_clients);								// Connected Section
			for (int i = 0; i < (SO_LAST-1); i++)
				clisoft[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), hcliconnected /*hclisoft*/);
			GetResString(&buffer, IDS_STATS_CLIENT_MODS);
			hclisoftMODs = stattree.InsertItem(buffer, clisoft[SO_EMULE]);			// eMule MODs Section
			hclisoftPlusMODs = stattree.InsertItem(buffer, clisoft[SO_PLUS]);			// eMule Plus MODs Section
			for (int im = 0; im < STAT_CLIENT_MOD_NODES; im++)
			{
				clisoftMODs[im] = NULL; m_nMODs[im] = 0;
			}
			for (int i = 0; i<(SO_LAST-1); i++)
			{
				cli_versions_other[i] = NULL; for (int j=0; j<4;j++) cli_versions[i][j] = NULL;
			}
			for(int i = 0; i<5; i++) cligen[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), h_clients);	// general clients stats
		hFiltered = stattree.InsertItem(GetResString(IDS_STATS_FILTEREDCLIENTS),h_clients);							// subsections: filtered
			for(int i = 0; i<3; i++) hFilteredItems[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), hFiltered);
		hCountries = stattree.InsertItem(GetResString(IDS_COUNTRIES),h_clients);									// Countries Section			

//	Server section
	h_servers = stattree.InsertItem(GetResString(IDS_SERVERS),4,4);											// Servers section
		for(int i = 0; i<7; i++) srv[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), h_servers);			// Servers Items
		for(int i = 0; i<3; i++) srv_w[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), srv[0]);			// Working Servers Items
		hsrv_records = stattree.InsertItem(GetResString(IDS_STATS_RECORDS),10,10,h_servers);						// Servers Records Section
		for(int i = 0; i<3; i++) srv_r[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), hsrv_records);		// Record Items

//	Shared files
	h_shared = stattree.InsertItem( GetResString(IDS_SHAREDFILES),5,5 );											// Shared Files Section
		for(int i = 0; i<4; i++) shar[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), h_shared);
		hshar_records= stattree.InsertItem(GetResString(IDS_STATS_RECORDS),10,10,h_shared);							// Shared Records Section
		for(int i = 0; i<4; i++) shar_r[i] = stattree.InsertItem(GetResString(IDS_FSTAT_WAITING), hshar_records);

//	Make section headers bold in order to make the tree easier to view at a glance.
	stattree.SetItemState(h_transfer, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(h_connection, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(h_time, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(htime_s, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(htime_t, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(htime_aap, TVIS_BOLD, TVIS_BOLD);
	for(int i = 0; i<3; i++)
	{
		stattree.SetItemState(time_aaph[i], TVIS_BOLD, TVIS_BOLD);
		stattree.SetItemState(time_aap_hup[i], TVIS_BOLD, TVIS_BOLD);
		stattree.SetItemState(time_aap_hdown[i], TVIS_BOLD, TVIS_BOLD);
	}
	stattree.SetItemState(h_clients, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(h_servers, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(h_shared, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(h_upload, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(h_download, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(h_up_session, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(h_up_total, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(h_down_session, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(h_down_total, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(h_conn_session, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(h_conn_total, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(hsrv_records, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(hshar_records, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(hconn_sg, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(hconn_su, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(hconn_sd, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(hconn_tg, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(hconn_tu, TVIS_BOLD, TVIS_BOLD);
	stattree.SetItemState(hconn_td, TVIS_BOLD, TVIS_BOLD);

//	Expand our purdy new tree...
	CString strTreeMask(g_App.m_pPrefs->GetExpandedTreeItems());

	if (strTreeMask.IsEmpty())
		strTreeMask = PREF_DEF_STATREE_MASK;
	stattree.ApplyExpandedMask(strTreeMask);

//	Select the top item so that the tree is not scrolled to the bottom when first viewed.
	stattree.SelectItem(h_transfer);

//	End Tree Setup
	EMULE_CATCH

	stattree.SetRedraw(true);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CStatisticsDlg::UpdateActConScale()
{
	EMULE_TRY

	int iStatYGrids=int((g_App.m_pPrefs->GetStatsMax()/10.0)+0.1)-1;
	if (iStatYGrids>10)
		iStatYGrids=int((g_App.m_pPrefs->GetStatsMax()/50.0)+0.1)-1;
	m_Statistics.m_nYGrids=iStatYGrids;

	m_Statistics.SetRanges(0, g_App.m_pPrefs->GetStatsMax());

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CStatisticsDlg::ShowInterval()
{
	if (!g_App.m_pMDlg->IsRunning())
		return;

	EMULE_TRY

	EnableWindow(FALSE);

	if(!m_DownloadOMeter.GetSafeHwnd() || !m_UploadOMeter.GetSafeHwnd())
		return;

	CRect r;
	m_DownloadOMeter.GetPlotRect(r);

	if (g_App.m_pPrefs->GetTrafficOMeterInterval() == 0)
	{
		m_DownloadOMeter.SetXUnits(GetResString(IDS_STOPPED));
		m_UploadOMeter.SetXUnits(GetResString(IDS_STOPPED));
	}
	else
	{
		CString buffer = CastSecondsToHM(r.Width() * g_App.m_pPrefs->GetTrafficOMeterInterval());

		m_UploadOMeter.SetXUnits(buffer);
		m_DownloadOMeter.SetXUnits(buffer);
	}

	UpdateData(FALSE);
	EnableWindow(TRUE);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CStatisticsDlg::SetARange(bool SetDownload,int maxValue)
{
	COScopeCtrl	*pScope;
	uint32		dwNotch, dwScale = 0;

	if (SetDownload)
	{
		do
		{
			dwScale += 10;
			dwNotch = (maxValue + (dwScale - 1)) / dwScale;
		} while(dwNotch > 12);
		pScope = &m_DownloadOMeter;
	}
	else
	{
		do
		{
			dwScale += 2;
			dwNotch = (maxValue + (dwScale - 1)) / dwScale;
		} while(dwNotch > 12);
		pScope = &m_UploadOMeter;
	}
	pScope->m_nYGrids = dwNotch - 1;
	dwNotch *= dwScale;
	pScope->SetRange(0, dwNotch, 0);
	pScope->SetRange(0, dwNotch, 1);
	pScope->SetRange(0, dwNotch, 2);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CStatisticsDlg::OnSize(UINT nType, int cx, int cy)
{
	CResizableDialog::OnSize(nType, cx, cy);
	if ((cx > 0 && cy > 0) && (cx != m_oldcx && cy != m_oldcy))
	{
		m_oldcx=cx;
		m_oldcy=cy;
		ShowInterval();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double CStatisticsDlg::GetAvgDownloadRate(int averageType)
{
	EMULE_TRY

	//check initial state
	if (g_App.stat_transferStarttime==0)
		return 0.0f;

	if (averageType==AVG_SESSION)
	{
		DWORD dwRunTime_MS = ::GetTickCount() - g_App.stat_transferStarttime;

		//Cax2 - stops the sudden session upload average 'jump' at the beginning.
		if (dwRunTime_MS < 5000)
			return 0;
		//eklmn: calculation of mean value only with one division allows us to get better result
		return ((double)g_App.stat_sessionReceivedBytes)/((double) dwRunTime_MS * 1.024);
	}
	else
	{
		if (downrateHistory.empty())
			return 0;
		//Cax2 - we could use that formula... but it's faster if we compute that ratio beforehand,
		//Cax2 - & more accurate if we use the actual data we have now instead of the front of the list.
		return ((double)(g_App.stat_sessionReceivedBytes - downrateHistory.back()))/((double)(::GetTickCount() - timeHistory.back())*1.024);
	}

	EMULE_CATCH

	return 0.0f;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double CStatisticsDlg::GetAvgUploadRate(int averageType)
{
	EMULE_TRY

	//check initial state
	if (g_App.stat_transferStarttime==0)
		return 0.0f;

	if (averageType == AVG_SESSION)
	{
		DWORD dwRunTime_MS = ::GetTickCount() - g_App.stat_transferStarttime;

		//Cax2 - stops the sudden session upload average 'jump' at the beginning.
		if (dwRunTime_MS < 5000)
			return 0;
		//eklmn: calculation of mean value only with one division allows us to get better result
		return ((double)g_App.stat_sessionSentBytes)/((double) dwRunTime_MS * 1.024);
	}
	else
	{
		if (uprateHistory.empty())
			return 0.0f;
		//Cax2 - precomputed ratio. see above
		return ((double)(g_App.stat_sessionSentBytes - uprateHistory.back()))/((double)(::GetTickCount() - timeHistory.back())*1.024);
	}

	EMULE_CATCH

	return 0.0f;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CStatisticsDlg::RecordRate()
{
	EMULE_TRY

	if (g_App.stat_transferStarttime==0)
		return;

	DWORD tickNow = ::GetTickCount();

	// Cax 2: save every 10th of the statsaverageminutes (no need to be more accurate...)
	// check time has passed since we updated the list last
	DWORD nUpdateTime = g_App.m_pPrefs->GetStatsAverageMinutes()*60*100;		//A 10th of the average interval
	if (tickNow - timeHistory.front() < nUpdateTime)
		return;

	downrateHistory.push_front(g_App.stat_sessionReceivedBytes);
	uprateHistory.push_front(g_App.stat_sessionSentBytes);
	timeHistory.push_front(tickNow);

	// limit to maxmins
	nUpdateTime *= 10; //Now we finally get the average interval expressed in ticks!
	while (tickNow-timeHistory.back()>nUpdateTime)		//Cax2 - keep them in sync
	{
		downrateHistory.pop_back();
		uprateHistory.pop_back();
		timeHistory.pop_back();
	}
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CStatisticsDlg::UpdateConnectionsStatus()
{
	EMULE_TRY

	// Can't do anything before OnInitDialog
	if(!m_hWnd)
		return;

#ifdef OLD_SOCKETS_ENABLED
	activeconnections = g_App.m_pListenSocket->GetNumOpenSockets(); // netwolf 07.05.03 (moved here from CStatisticsDlg::SetCurrentRate())
	if (peakconnections < activeconnections)
		peakconnections = activeconnections;
	// -khaos--+++>
	if (peakconnections > g_App.m_pPrefs->GetConnPeakConnections())
		g_App.m_pPrefs->Add2ConnPeakConnections(peakconnections);
	// <-----khaos-
	if (g_App.m_pServerConnect->IsConnected())
	{
		totalconnectionchecks++;
		double percent;
		percent = (static_cast<double>(totalconnectionchecks-1.0)/static_cast<double>(totalconnectionchecks));
		if (percent > 0.99)
			percent = 0.99;
		averageconnections = (averageconnections*percent) + static_cast<double>(static_cast<double>(activeconnections)*static_cast<double>(1.0-percent));
	}
#endif //OLD_SOCKETS_ENABLED

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CStatisticsDlg::SetCurrentRate(uint32 dwUpRate, uint32 dwDownRate)
{
	EMULE_TRY

	if (!g_App.m_pMDlg->IsRunning())
		return;

	// Can't do anything before OnInitDialog
	if(!m_hWnd)
		return;

	double	adPlotDataUp[3], adPlotDataDown[3], adPlotDataMore[3];
	uint64	myBigStats[STATS_DLDATA_COUNT];
	uint32	adwRateStats[STATS_DLSRC_COUNT];

//	Current rate
	adPlotDataUp[2] = static_cast<double>(dwUpRate) / 1024.0;
	adPlotDataDown[2] = static_cast<double>(dwDownRate) / 1024.0;
	if (maxDown < adPlotDataDown[2])
		maxDown = adPlotDataDown[2];

//	Averages
	adPlotDataDown[0] = GetAvgDownloadRate(AVG_SESSION);
	adPlotDataUp[0] = GetAvgUploadRate(AVG_SESSION);
	adPlotDataDown[1] = GetAvgDownloadRate(AVG_TIME);
	adPlotDataUp[1] = GetAvgUploadRate(AVG_TIME);
//	Show
	m_DownloadOMeter.AppendPoints(adPlotDataDown);
	m_UploadOMeter.AppendPoints(adPlotDataUp);

	// get Partialfiles summary

	g_App.m_pDownloadQueue->GetDownloadStats(adwRateStats, myBigStats);
	if(g_App.m_pPrefs->GetGraphRatio() == 255) // 255 magic number for %
		adPlotDataMore[0] = (100*activeconnections/g_App.m_pPrefs->GetMaxConnections());
	else
		adPlotDataMore[0] = (activeconnections/g_App.m_pPrefs->GetGraphRatio());
	adPlotDataMore[1] = g_App.m_pUploadQueue->GetUploadQueueLength();
	adPlotDataMore[2] = adwRateStats[STATS_DLSRC_TRANSFERRING];

	m_Statistics.AppendPoints(adPlotDataMore);

	UpDown updown = { dwDownRate, dwUpRate, activeconnections };

	g_App.m_pWebServer->AddStatsLine(&updown);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CStatisticsDlg::RepaintMeters()
{
	EMULE_TRY

	EnableWindow(FALSE);

	m_DownloadOMeter.SetBackgroundColor(g_App.m_pPrefs->GetStatsColor(0));
	m_DownloadOMeter.SetGridColor(g_App.m_pPrefs->GetStatsColor(1));
	m_DownloadOMeter.SetPlotColor(g_App.m_pPrefs->GetStatsColor(4), 0);
	m_DownloadOMeter.SetPlotColor(g_App.m_pPrefs->GetStatsColor(3), 1);
	m_DownloadOMeter.SetPlotColor(g_App.m_pPrefs->GetStatsColor(2), 2);
	m_UploadOMeter.SetBackgroundColor(g_App.m_pPrefs->GetStatsColor(0));
	m_UploadOMeter.SetGridColor(g_App.m_pPrefs->GetStatsColor(1));
	m_UploadOMeter.SetPlotColor(g_App.m_pPrefs->GetStatsColor(7), 0);
	m_UploadOMeter.SetPlotColor(g_App.m_pPrefs->GetStatsColor(6), 1);
	m_UploadOMeter.SetPlotColor(g_App.m_pPrefs->GetStatsColor(5), 2);
	m_Statistics.SetBackgroundColor(g_App.m_pPrefs->GetStatsColor(0));
	m_Statistics.SetGridColor(g_App.m_pPrefs->GetStatsColor(1));
	m_Statistics.SetPlotColor(g_App.m_pPrefs->GetStatsColor(8), 0);
	m_Statistics.SetPlotColor(g_App.m_pPrefs->GetStatsColor(9), 1);
	m_Statistics.SetPlotColor(g_App.m_pPrefs->GetStatsColor(10), 2);

	if (m_byteStatGraphRatio != g_App.m_pPrefs->GetGraphRatio())	//	Resize the general connections...
	{
		// xrmb: if either old or new is % we need to invalidate 
		// (maybe not true, but don't have the time to think about this)
		// btw. 255 is still the magic number for %
		if(m_byteStatGraphRatio == 255 || g_App.m_pPrefs->GetGraphRatio() == 255)
			m_Statistics.Invalidate();
		else
			m_Statistics.ReSizePlot(0,(double)m_byteStatGraphRatio/g_App.m_pPrefs->GetGraphRatio());
		m_byteStatGraphRatio = g_App.m_pPrefs->GetGraphRatio();
	}

	SetupLegend(IDC_C0_2, 0, 1);
	SetupLegend(IDC_C0_3, 1, 1);
	SetupLegend(IDC_C0,   2, 1);

	SetupLegend(IDC_C1_2, 0, 2);
	SetupLegend(IDC_C1_3, 1, 2);
	SetupLegend(IDC_C1,   2, 2);

	SetupLegend(IDC_S0,   0, 3);
	SetupLegend(IDC_S1,   1, 3);
	SetupLegend(IDC_S3,   2, 3);

	EnableWindow(TRUE);		//Cax2 show the updated graphs immediately!

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CStatisticsDlg::OnDestroy()
{
	CResizableDialog::OnDestroy();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CStatisticsDlg::SetupLegend( int ResIdx, int ElmtIdx, int legendNr)
{
	EMULE_TRY

	CRect Rect;

	GetDlgItem( ResIdx )->GetWindowRect( Rect );
	ScreenToClient( Rect );

	if (legendNr==1)
	{
		if (!m_Led1[ElmtIdx])
			m_Led1[ElmtIdx].Create(WS_VISIBLE | WS_CHILD, Rect, this);
		m_Led1[ElmtIdx].SetBackgroundColor(m_DownloadOMeter.GetPlotColor(ElmtIdx));
		m_Led1[ElmtIdx].SetFrameColor(RGB(0x00, 0x00, 0x00));
	}
	else if (legendNr==2)
	{
		if (!m_Led2[ElmtIdx])
			m_Led2[ElmtIdx].Create( WS_VISIBLE | WS_CHILD, Rect, this);
		m_Led2[ElmtIdx].SetBackgroundColor(m_UploadOMeter.GetPlotColor(ElmtIdx));
		m_Led2[ElmtIdx].SetFrameColor(RGB(0x00, 0x00, 0x00));
	}
	else if (legendNr==3)
	{
		if (!m_Led3[ElmtIdx])
			m_Led3[ElmtIdx].Create(WS_VISIBLE | WS_CHILD, Rect, this);
		m_Led3[ElmtIdx].SetBackgroundColor(m_Statistics.GetPlotColor(ElmtIdx));
		m_Led3[ElmtIdx].SetFrameColor(RGB(0x00, 0x00, 0x00));
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString CStatisticsDlg::GetMODType(uint32 dwIndex)
{
	static const TCHAR *apcModStrs[] =
	{
		_T("Morph"), _T("Xtreme")
	};

	if (dwIndex < ARRSIZE(apcModStrs))
		return apcModStrs[dwIndex];
	else
		return ::GetResString(IDS_STATS_OTHERS);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString CStatisticsDlg::GetUpEndReason(unsigned uiState)
{
	static const uint16 s_auResTbl[] =
	{
		IDS_STATS_UL_SESSION_T,		//ETS_TIMEOUT
		IDS_STATS_UL_SESSION_D,		//ETS_DISCONNECT
		IDS_STATS_UL_SESSION_B,		//ETS_BAN
		IDS_STATS_UL_SESSION_C,		//ETS_CANCELED
		IDS_STATS_UL_SESSION_ED,	//ETS_END_OF_DOWNLOAD
		IDS_STATS_UL_SESSION_FE,	//ETS_FILE_ERROR
		IDS_STATS_UL_SESSION_BC,	//ETS_BLOCKED_CHUNK
	};
	CString		strBuf;

	if (uiState < ARRSIZE(s_auResTbl))
		GetResString(&strBuf, s_auResTbl[uiState]);
	return strBuf;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
