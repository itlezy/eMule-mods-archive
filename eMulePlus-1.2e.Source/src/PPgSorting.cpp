//	This file is part of eMule Plus
//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//	as published by the Free Software Foundation; either
//	version 2 of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "stdafx.h"
#include "emule.h"
#include "PPgSorting.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CPPgSorting, CPropertyPage)
CPPgSorting::CPPgSorting()
	: CPropertyPage(CPPgSorting::IDD)
	, m_bUseSorting(FALSE)
	, m_bUseSourcesSorting(FALSE)
	, m_bPausedStoppedLast(FALSE)
{
}

CPPgSorting::~CPPgSorting()
{
}

void CPPgSorting::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SORT_SERVERS, m_ServerColsCombo);
	DDX_Control(pDX, IDC_SORT_FILES, m_FileColsCombo);
	DDX_Control(pDX, IDC_SORT_DOWNLOADS, m_DownloadColsCombo);
	DDX_Control(pDX, IDC_SORT_UPLOADS, m_UploadColsCombo);
	DDX_Control(pDX, IDC_SORT_QUEUE, m_QueueColsCombo);
	DDX_Control(pDX, IDC_SORT_SEARCH, m_SearchColsCombo);
	DDX_Control(pDX, IDC_SORT_IRC, m_IrcColsCombo);
	DDX_Control(pDX, IDC_SORT_CLIENTLIST, m_ClientListColsCombo);
	DDX_Control(pDX, IDC_SORT_SOURCES1, m_SourceCols1Combo);
	DDX_Control(pDX, IDC_SORT_SOURCES2, m_SourceCols2Combo);
	DDX_Check(pDX, IDC_SORT_USEIT, m_bUseSorting);
	DDX_Check(pDX, IDC_SORT_SOURCES2_BOX, m_bUseSourcesSorting);
	DDX_Check(pDX, IDC_SORT_NONACTIVELAST, m_bPausedStoppedLast);
}

BEGIN_MESSAGE_MAP(CPPgSorting, CPropertyPage)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_SORT_USEIT, OnBnClickedUseSort)
	ON_CBN_SELCHANGE(IDC_SORT_SERVERS, OnCbnSelchangeSortServers)
	ON_CBN_SELCHANGE(IDC_SORT_FILES, OnCbnSelchangeSortFiles)
	ON_CBN_SELCHANGE(IDC_SORT_DOWNLOADS, OnCbnSelchangeSortDownloads)
	ON_CBN_SELCHANGE(IDC_SORT_SOURCES1, OnCbnSelchangeSortSources1)
	ON_CBN_SELCHANGE(IDC_SORT_SOURCES2, OnCbnSelchangeSortSources2)
	ON_CBN_SELCHANGE(IDC_SORT_UPLOADS, OnCbnSelchangeSortUploads)
	ON_CBN_SELCHANGE(IDC_SORT_QUEUE, OnCbnSelchangeSortQueue)
	ON_CBN_SELCHANGE(IDC_SORT_SEARCH, OnCbnSelchangeSortSearch)
	ON_CBN_SELCHANGE(IDC_SORT_IRC, OnCbnSelchangeSortIrc)
	ON_CBN_SELCHANGE(IDC_SORT_CLIENTLIST, OnCbnSelchangeSortClientList)
	ON_BN_CLICKED(IDC_SORT_SOURCES2_BOX, OnBnClickedSortSources2Box)
	ON_BN_CLICKED(IDC_SORT_NONACTIVELAST, OnBnClickedPausedStoppedLast)
END_MESSAGE_MAP()

BOOL CPPgSorting::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	LoadSettings();
	Localize();
	return TRUE;
}

void CPPgSorting::LoadSettings(void)
{
	m_bUseSorting = m_pPrefs->DoUseSort();
	m_bUseSourcesSorting = m_pPrefs->DoUseSrcSortCol2();
	m_bPausedStoppedLast = m_pPrefs->DoPausedStoppedLast();

	UpdateData(FALSE);
	OnBnClickedUseSort();
	SetModified(FALSE);
}

BOOL CPPgSorting::OnApply()
{
	if (m_bModified)
	{
		UpdateData(TRUE);

		m_pPrefs->SetUseSort(B2b(m_bUseSorting));
		m_pPrefs->SetUseSrcSortCol2(B2b(m_bUseSourcesSorting));
		m_pPrefs->SetPausedStoppedLast(B2b(m_bPausedStoppedLast));
		m_pPrefs->SetServerSortCol(m_ServerColsCombo.GetItemData(m_ServerColsCombo.GetCurSel()));
		m_pPrefs->SetUploadSortCol(m_UploadColsCombo.GetItemData(m_UploadColsCombo.GetCurSel()));
		m_pPrefs->SetQueueSortCol(m_QueueColsCombo.GetItemData(m_QueueColsCombo.GetCurSel()));
		m_pPrefs->SetSearchSortCol(m_SearchColsCombo.GetItemData(m_SearchColsCombo.GetCurSel()));
		m_pPrefs->SetIrcSortCol(m_IrcColsCombo.GetItemData(m_IrcColsCombo.GetCurSel()));
		m_pPrefs->SetClientListSortCol(m_ClientListColsCombo.GetItemData(m_ClientListColsCombo.GetCurSel()));
		m_pPrefs->SetFileSortCol(m_FileColsCombo.GetItemData(m_FileColsCombo.GetCurSel()));
		m_pPrefs->SetDownloadSortCol(m_DownloadColsCombo.GetItemData(m_DownloadColsCombo.GetCurSel()));
		m_pPrefs->SetSrcSortCol1(m_SourceCols1Combo.GetItemData(m_SourceCols1Combo.GetCurSel()));
		m_pPrefs->SetSrcSortCol2(m_SourceCols2Combo.GetItemData(m_SourceCols2Combo.GetCurSel()));

		SetModified();

		if (m_pPrefs->DoUseSort())
		{
			g_App.m_pMDlg->m_wndServer.m_ctlServerList.SortInit(m_pPrefs->GetServerSortCol());
			g_App.m_pMDlg->m_wndTransfer.m_ctlUploadList.SortInit(m_pPrefs->GetUploadSortCol());
			g_App.m_pMDlg->m_wndTransfer.m_ctlQueueList.SortInit(m_pPrefs->GetQueueSortCol());
			g_App.m_pMDlg->m_wndTransfer.m_ctlClientList.SortInit(m_pPrefs->GetClientListSortCol());
			g_App.m_pMDlg->m_dlgSearch.m_ctlSearchList.SortInit(m_pPrefs->GetSearchSortCol());
			g_App.m_pMDlg->m_wndIRC.SortInit(m_pPrefs->GetIrcSortCol());
			g_App.m_pMDlg->m_wndSharedFiles.m_ctlSharedFilesList.SortInit(m_pPrefs->GetFileSortCol());
			SetModified(FALSE); //needs the settings for downloadlistcontrol
			g_App.m_pMDlg->m_wndTransfer.m_ctlDownloadList.SortInit(DL_OVERRIDESORT); //major override...
		}
	}

	return CPropertyPage::OnApply();
}

void CPPgSorting::Localize(void)
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_SORT_USEIT, IDS_SORT_USEIT },
		{ IDC_SORT_NONACTIVELAST, IDS_SORT_NONACTIVELAST },
		{ IDC_SORT_SERVERS_LBL, IDS_SORT_SERVERS_LBL },
		{ IDC_SORT_DOWNLOADS_LBL, IDS_SORT_DOWNLOADS_LBL },
		{ IDC_SORT_UPLOADS_LBL, IDS_SORT_UPLOADS_LBL },
		{ IDC_SORT_QUEUE_LBL, IDS_SORT_QUEUE_LBL },
		{ IDC_SORT_SEARCH_LBL, IDS_SORT_SEARCH_LBL },
		{ IDC_SORT_FILES_LBL, IDS_SORT_FILES_LBL },
		{ IDC_SORT_IRC_LBL, IDS_SORT_IRC_LBL },
		{ IDC_SORT_CLIENTLIST_LBL, IDS_SORT_CLIENTLIST_LBL },
		{ IDC_SORT_GRP, IDS_SORT_GRP },
		{ IDC_SORT_SOURCES1_LBL, IDS_SORT_SOURCES1_LBL },
		{ IDC_SORT_SOURCES2_BOX, IDS_SORT_SOURCES2_LBL }
	};

	if (::IsWindow(m_hWnd))
	{
		CString	strRes;

		for (uint32 i = 0; i < ARRSIZE(s_auResTbl); i++)
		{
			::GetResString(&strRes, static_cast<UINT>(s_auResTbl[i][1]));
			SetDlgItemText(s_auResTbl[i][0], strRes);
		}

		LoadCombo();
	}
}

void CPPgSorting::OnBnClickedSortSources2Box()
{
	UpdateData(TRUE);

	m_SourceCols2Combo.EnableWindow(m_bUseSourcesSorting);

	SetModified();
}

void CPPgSorting::OnBnClickedUseSort()
{
	UpdateData(TRUE);

	m_ServerColsCombo.EnableWindow(m_bUseSorting);
	m_FileColsCombo.EnableWindow(m_bUseSorting);
	m_DownloadColsCombo.EnableWindow(m_bUseSorting);
	m_UploadColsCombo.EnableWindow(m_bUseSorting);
	m_QueueColsCombo.EnableWindow(m_bUseSorting);
	m_SearchColsCombo.EnableWindow(m_bUseSorting);
	m_IrcColsCombo.EnableWindow(m_bUseSorting);
	m_ClientListColsCombo.EnableWindow(m_bUseSorting);
	m_SourceCols1Combo.EnableWindow(m_bUseSorting);
	m_SourceCols2Combo.EnableWindow((m_bUseSorting && m_bUseSourcesSorting) ? true : false);
	GetDlgItem(IDC_SORT_SOURCES2_BOX)->EnableWindow(m_bUseSorting);

	SetModified();
}

void CPPgSorting::LoadCombo(void)
{
	static const uint16 s_auServerCols[][2] =
	{
		{ IDS_SL_SERVERNAME,	SL_COLUMN_SERVERNAME },
		{ IDS_IP,				SL_COLUMN_SERVERIP },
		{ IDS_DESCRIPTION,		SL_COLUMN_DESCRIPTION },
		{ IDS_PING,				SL_COLUMN_PING },
		{ IDS_UUSERS,			SL_COLUMN_NUMUSERS },
		{ IDS_MAXUSERS,			SL_COLUMN_NUMUSERS | MLC_SORTALT },
		{ IDS_PW_FILES,			SL_COLUMN_NUMFILES },
		{ IDS_PRIORITY,			SL_COLUMN_PREFERENCES },
		{ IDS_UFAILED,			SL_COLUMN_FAILEDCOUNT },
		{ IDS_STATICSERVER,		SL_COLUMN_STATIC },
		{ IDS_SERVER_SOFTLIMIT,	SL_COLUMN_SOFTFILELIMIT },
		{ IDS_SERVER_HARDLIMIT,	SL_COLUMN_SOFTFILELIMIT | MLC_SORTALT },
		{ IDS_SERVER_VERSION,	SL_COLUMN_SOFTWAREVER },
		{ IDS_COUNTRY,			SL_COLUMN_COUNTRY },
		{ IDS_LOWIDUSERS,		SL_COLUMN_LOWIDUSERS }
	};
	static const uint16 s_auSharedFilesCols[][2] =
	{
		{ IDS_DL_FILENAME,		SFL_COLUMN_FILENAME },
		{ IDS_DL_SIZE,			SFL_COLUMN_FILESIZE },
		{ IDS_TYPE,				SFL_COLUMN_TYPE },
		{ IDS_PRIORITY,			SFL_COLUMN_PRIORITY },
		{ IDS_PERMISSION,		SFL_COLUMN_PERMISSION },
		{ IDS_FILEHASH,			SFL_COLUMN_FILEID },
		{ IDS_SF_REQUESTS,		SFL_COLUMN_REQUESTS },
		{ IDS_SF_REQUESTS,		SFL_COLUMN_REQUESTS | MLC_SORTALT },
		{ IDS_SF_ACCEPTS,		SFL_COLUMN_ACCEPTED },
		{ IDS_SF_ACCEPTS,		SFL_COLUMN_ACCEPTED | MLC_SORTALT },
		{ IDS_SF_TRANSFERRED,	SFL_COLUMN_TRANSFERRED },
		{ IDS_SF_TRANSFERRED,	SFL_COLUMN_TRANSFERRED | MLC_SORTALT },
		{ IDS_SF_PARTTRAFFIC,	SFL_COLUMN_PARTTRAFFIC },
		{ IDS_SF_PARTTRAFFIC,	SFL_COLUMN_PARTTRAFFIC | MLC_SORTALT },
		{ IDS_SF_COLUPLOADS,	SFL_COLUMN_UPLOADS },
		{ IDS_SF_COLUPLOADS,	SFL_COLUMN_UPLOADS | MLC_SORTALT },
		{ IDS_SF_COMPLETESRC,	SFL_COLUMN_COMPLETESRC },
		{ IDS_SF_COMPLETESRC,	SFL_COLUMN_COMPLETESRC | MLC_SORTALT },
		{ IDS_SF_FOLDER,		SFL_COLUMN_FOLDER }
	};
	static const uint16 s_auUploadCols[][2] =
	{
		{ IDS_QL_USERNAME,	ULCOL_USERNAME },
		{ IDS_FILE,			ULCOL_FILENAME },
		{ IDS_DL_SPEED,		ULCOL_SPEED },
		{ IDS_DL_TRANSF,	ULCOL_TRANSFERRED },
		{ IDS_WAITED,		ULCOL_WAITED },
		{ IDS_UPLOADTIME,	ULCOL_UPLOADTIME },
		{ IDS_STATUS,		ULCOL_STATUS },
		{ IDS_UP_PARTS,		ULCOL_PARTS },
		{ IDS_DL_PROGRESS,	ULCOL_PROGRESS },
		{ IDS_COMPRESSION,	ULCOL_COMPRESSION },
		{ IDS_COUNTRY,		ULCOL_COUNTRY }
	};
	static const uint16 s_auQueueCols[][2] =
	{
		{ IDS_QL_USERNAME,	QLCOL_USERNAME },
		{ IDS_FILE,			QLCOL_FILENAME },
		{ IDS_FILEPRIO,		QLCOL_FILEPRIORITY },
		{ IDS_UP_PARTS,		QLCOL_PARTS },
		{ IDS_DL_PROGRESS,	QLCOL_PROGRESS },
		{ IDS_RATING,		QLCOL_QLRATING },
		{ IDS_SCORE,		QLCOL_SCORE },
		{ IDS_SFRATIO,		QLCOL_SFRATIO },
		{ IDS_RFRATIO,		QLCOL_RFRATIO },
		{ IDS_ASKED,		QLCOL_TIMESASKED },
		{ IDS_LASTSEEN,		QLCOL_LASTSEEN },
		{ IDS_ENTERQUEUE,	QLCOL_ENTEREDQUEUE },
		{ IDS_BANNED,		QLCOL_BANNED },
		{ IDS_COUNTRY,		QLCOL_COUNTRY }
	};
	static const uint16 s_auSearchCols[][2] =
	{
		{ IDS_DL_FILENAME,		SL_COLUMN_FILENAME },
		{ IDS_DL_SIZE,			SL_COLUMN_SIZE },
		{ IDS_DL_SOURCES,		SL_COLUMN_SOURCES },
		{ IDS_DL_SOURCES,		SL_COLUMN_SOURCES | MLC_SORTALT },
		{ IDS_TYPE,				SL_COLUMN_TYPE },
		{ IDS_FILEHASH,			SL_COLUMN_FILEHASH },
		{ IDS_LASTSEENCOMPLETE,	SL_COLUMN_LASTSEENCOMPLETE },
		{ IDS_LENGTH,			SL_COLUMN_LENGTH },
		{ IDS_BITRATE,			SL_COLUMN_BITRATE },
		{ IDS_CODEC,			SL_COLUMN_CODEC }
	};
	static const uint16 s_auIrcChanCols[][2] =
	{
		{ IDS_IRC_NAME,		IRC2COL_NAME },
		{ IDS_UUSERS,		IRC2COL_USERS },
		{ IDS_DESCRIPTION,	IRC2COL_DESCRIPTION }
	};
	static const uint16 s_auClientCols[][2] =
	{
		{ IDS_QL_USERNAME,					CLCOL_USERNAME },
		{ IDS_CL_UPLOADSTATUS,				CLCOL_UPLOADSTATUS },
		{ IDS_CL_TRANSFUP,					CLCOL_TRANSFERREDUP },
		{ IDS_CL_DOWNLSTATUS,				CLCOL_DOWNLOADSTATUS },
		{ IDS_CL_TRANSFDOWN,				CLCOL_TRANSFERREDDOWN },
		{ IDS_INFLST_USER_CLIENTSOFTWARE,	CLCOL_CLIENTSOFTWARE },
		{ IDS_CONNECTED,					CLCOL_CONNECTEDTIME },
		{ IDS_INFLST_USER_USERHASH,			CLCOL_USERHASH },
		{ IDS_COUNTRY,						CLCOL_COUNTRY }
	};
	static const uint16 s_auDownloadCols[][2] =
	{
		{ IDS_DL_FILENAME,			DLCOL_FILENAME },
		{ IDS_DL_SIZE,				DLCOL_SIZE },
		{ IDS_DL_TRANSF,			DLCOL_TRANSFERRED },
		{ IDS_SF_COMPLETED,			DLCOL_COMPLETED },
		{ IDS_DL_SPEED,				DLCOL_SPEED },
		{ IDS_DL_PROGRESS,			DLCOL_PROGRESS },
		{ IDS_DL_SOURCES,			DLCOL_NUMSOURCES },
		{ IDS_PRIORITY,				DLCOL_PRIORITY },
		{ IDS_STATUS,				DLCOL_STATUS },
		{ IDS_DL_REMAINS,			DLCOL_REMAINING },
		{ IDS_DLCOL_REMAININGTIME,	DLCOL_REMAININGTIME },
		{ IDS_LASTSEENCOMPLETE,		DLCOL_LASTSEENCOMPLETE },
		{ IDS_LASTRECEPTION,		DLCOL_LASTRECEIVED },
		{ IDS_CAT,					DLCOL_CATEGORY },
		{ IDS_WAITED,				DLCOL_WAITED },
		{ IDS_DLCOL_AVGSPEED,		DLCOL_AVGSPEED },
		{ IDS_DLCOL_AVGREMTIME,		DLCOL_AVGREMTIME },
		{ IDS_DLCOL_ETA,			DLCOL_ETA },
		{ IDS_DLCOL_AVGETA,			DLCOL_AVGETA }
	};
	static const uint16 s_auDownloadSrcCols[][2] =
	{
		{ IDS_QL_USERNAME,					DLCOL_FILENAME },
		{ IDS_DL_TRANSF,					DLCOL_TRANSFERRED },
		{ IDS_SF_COMPLETED,					DLCOL_COMPLETED },
		{ IDS_DL_SPEED,						DLCOL_SPEED },
		{ IDS_DL_PROGRESS,					DLCOL_PROGRESS },
		{ IDS_INFLST_USER_CLIENTSOFTWARE,	DLCOL_NUMSOURCES },
		{ 0,								DLCOL_PRIORITY },	//QR
		{ IDS_STATUS,						DLCOL_STATUS },
		{ IDS_DL_REMAINS,					DLCOL_REMAINING },
		{ IDS_DLCOL_REMAININGTIME,			DLCOL_REMAININGTIME },
		{ IDS_DL_ULDL,						DLCOL_ULDLRATIO },
		{ IDS_RATING,						DLCOL_QLRATING }
	};
	CString	strResStr, strAux;
	int		iRc, iCurSel;
	uint32	dwCode;

	m_ServerColsCombo.ResetContent();
	iCurSel = 0;
	for (unsigned ui = 0; ui < ARRSIZE(s_auServerCols); ui++)
	{
		GetResString(&strResStr, s_auServerCols[ui][0]);
		strAux = strResStr;
		if ((s_auServerCols[ui][1] & MLC_SORTALT) != 0)
		{
			strAux += _T("[^^]");
			strResStr += _T("[vv]");
		}
		else
			strAux += _T("[^]");

		m_ServerColsCombo.SetItemData(iRc = m_ServerColsCombo.AddString(strAux), dwCode = s_auServerCols[ui][1]);
		if (dwCode == m_pPrefs->GetServerSortCol())
			iCurSel = iRc;
		m_ServerColsCombo.SetItemData(iRc = m_ServerColsCombo.AddString(strResStr), dwCode |= MLC_SORTDESC);
		if (dwCode == m_pPrefs->GetServerSortCol())
			iCurSel = iRc;
	}
	m_ServerColsCombo.SetCurSel(iCurSel);

	m_FileColsCombo.ResetContent();
	iCurSel = 0;
	for (unsigned ui = 0; ui < ARRSIZE(s_auSharedFilesCols); ui++)
	{
		GetResString(&strResStr, s_auSharedFilesCols[ui][0]);
		strAux = strResStr;
		if ((s_auSharedFilesCols[ui][1] & MLC_SORTALT) != 0)
		{
			strAux += _T("[^^]");
			strResStr += _T("[vv]");
		}
		else
			strAux += _T("[^]");

		m_FileColsCombo.SetItemData(iRc = m_FileColsCombo.AddString(strAux), dwCode = s_auSharedFilesCols[ui][1]);
		if (dwCode == m_pPrefs->GetFileSortCol())
			iCurSel = iRc;
		m_FileColsCombo.SetItemData(iRc = m_FileColsCombo.AddString(strResStr), dwCode |= MLC_SORTDESC);
		if (dwCode == m_pPrefs->GetFileSortCol())
			iCurSel = iRc;
	}
	m_FileColsCombo.SetCurSel(iCurSel);

	m_UploadColsCombo.ResetContent();
	iCurSel = 0;
	for (unsigned ui = 0; ui < ARRSIZE(s_auUploadCols); ui++)
	{
		GetResString(&strResStr, s_auUploadCols[ui][0]);
		strAux = strResStr;
		strAux += _T("[^]");

		m_UploadColsCombo.SetItemData(iRc = m_UploadColsCombo.AddString(strAux), dwCode = s_auUploadCols[ui][1]);
		if (dwCode == m_pPrefs->GetUploadSortCol())
			iCurSel = iRc;
		m_UploadColsCombo.SetItemData(iRc = m_UploadColsCombo.AddString(strResStr), dwCode |= MLC_SORTDESC);
		if (dwCode == m_pPrefs->GetUploadSortCol())
			iCurSel = iRc;
	}
	m_UploadColsCombo.SetCurSel(iCurSel);

	m_QueueColsCombo.ResetContent();
	iCurSel = 0;
	for (unsigned ui = 0; ui < ARRSIZE(s_auQueueCols); ui++)
	{
		GetResString(&strResStr, s_auQueueCols[ui][0]);
		strAux = strResStr;
		strAux += _T("[^]");

		m_QueueColsCombo.SetItemData(iRc = m_QueueColsCombo.AddString(strAux), dwCode = s_auQueueCols[ui][1]);
		if (dwCode == m_pPrefs->GetQueueSortCol())
			iCurSel = iRc;
		m_QueueColsCombo.SetItemData(iRc = m_QueueColsCombo.AddString(strResStr), dwCode |= MLC_SORTDESC);
		if (dwCode == m_pPrefs->GetQueueSortCol())
			iCurSel = iRc;
	}
	m_QueueColsCombo.SetCurSel(iCurSel);

	m_SearchColsCombo.ResetContent();
	iCurSel = 0;
	for (unsigned ui = 0; ui < ARRSIZE(s_auSearchCols); ui++)
	{
		GetResString(&strResStr, s_auSearchCols[ui][0]);
		strAux = strResStr;
		if ((s_auSearchCols[ui][1] & MLC_SORTALT) != 0)
		{
			strAux += _T("[^^]");
			strResStr += _T("[vv]");
		}
		else
			strAux += _T("[^]");

		m_SearchColsCombo.SetItemData(iRc = m_SearchColsCombo.AddString(strAux), dwCode = s_auSearchCols[ui][1]);
		if (dwCode == m_pPrefs->GetSearchSortCol())
			iCurSel = iRc;
		m_SearchColsCombo.SetItemData(iRc = m_SearchColsCombo.AddString(strResStr), dwCode |= MLC_SORTDESC);
		if (dwCode == m_pPrefs->GetSearchSortCol())
			iCurSel = iRc;
	}
	m_SearchColsCombo.SetCurSel(iCurSel);

	m_IrcColsCombo.ResetContent();
	iCurSel = 0;
	for (unsigned ui = 0; ui < ARRSIZE(s_auIrcChanCols); ui++)
	{
		GetResString(&strResStr, s_auIrcChanCols[ui][0]);
		strAux = strResStr;
		strAux += _T("[^]");

		m_IrcColsCombo.SetItemData(iRc = m_IrcColsCombo.AddString(strAux), dwCode = s_auIrcChanCols[ui][1]);
		if (dwCode == m_pPrefs->GetIrcSortCol())
			iCurSel = iRc;
		m_IrcColsCombo.SetItemData(iRc = m_IrcColsCombo.AddString(strResStr), dwCode |= MLC_SORTDESC);
		if (dwCode == m_pPrefs->GetIrcSortCol())
			iCurSel = iRc;
	}
	m_IrcColsCombo.SetCurSel(iCurSel);

	m_ClientListColsCombo.ResetContent();
	iCurSel = 0;
	for (unsigned ui = 0; ui < ARRSIZE(s_auClientCols); ui++)
	{
		GetResString(&strResStr, s_auClientCols[ui][0]);
		strAux = strResStr;
		strAux += _T("[^]");

		m_ClientListColsCombo.SetItemData(iRc = m_ClientListColsCombo.AddString(strAux), dwCode = s_auClientCols[ui][1]);
		if (dwCode == m_pPrefs->GetClientListSortCol())
			iCurSel = iRc;
		m_ClientListColsCombo.SetItemData(iRc = m_ClientListColsCombo.AddString(strResStr), dwCode |= MLC_SORTDESC);
		if (dwCode == m_pPrefs->GetClientListSortCol())
			iCurSel = iRc;
	}
	m_ClientListColsCombo.SetCurSel(iCurSel);

	m_DownloadColsCombo.ResetContent();
	iCurSel = 0;
	for (unsigned ui = 0; ui < ARRSIZE(s_auDownloadCols); ui++)
	{
		GetResString(&strResStr, s_auDownloadCols[ui][0]);
		strAux = strResStr;
		strAux += _T("[^]");

		m_DownloadColsCombo.SetItemData(iRc = m_DownloadColsCombo.AddString(strAux), dwCode = s_auDownloadCols[ui][1]);
		if (dwCode == m_pPrefs->GetDownloadSortCol())
			iCurSel = iRc;
		m_DownloadColsCombo.SetItemData(iRc = m_DownloadColsCombo.AddString(strResStr), dwCode |= MLC_SORTDESC);
		if (dwCode == m_pPrefs->GetDownloadSortCol())
			iCurSel = iRc;
	}
	m_DownloadColsCombo.SetCurSel(iCurSel);

	m_SourceCols1Combo.ResetContent();
	iCurSel = 0;
	for (unsigned ui = 0; ui < ARRSIZE(s_auDownloadSrcCols); ui++)
	{
		if (s_auDownloadSrcCols[ui][1] == DLCOL_PRIORITY)
			strResStr = _T("QR");
		else
			GetResString(&strResStr, s_auDownloadSrcCols[ui][0]);
		strAux = strResStr;
		strAux += _T("[^]");

		m_SourceCols1Combo.SetItemData(iRc = m_SourceCols1Combo.AddString(strAux), dwCode = s_auDownloadSrcCols[ui][1]);
		if (dwCode == m_pPrefs->GetSrcSortCol1())
			iCurSel = iRc;
		m_SourceCols1Combo.SetItemData(iRc = m_SourceCols1Combo.AddString(strResStr), dwCode |= MLC_SORTDESC);
		if (dwCode == m_pPrefs->GetSrcSortCol1())
			iCurSel = iRc;
	}
	m_SourceCols1Combo.SetCurSel(iCurSel);

	m_SourceCols2Combo.ResetContent();
	iCurSel = 0;
	for (unsigned ui = 0; ui < ARRSIZE(s_auDownloadSrcCols); ui++)
	{
		if (s_auDownloadSrcCols[ui][1] == DLCOL_PRIORITY)
			strResStr = _T("QR");
		else
			GetResString(&strResStr, s_auDownloadSrcCols[ui][0]);
		strAux = strResStr;
		strAux += _T("[^]");

		m_SourceCols2Combo.SetItemData(iRc = m_SourceCols2Combo.AddString(strAux), dwCode = s_auDownloadSrcCols[ui][1]);
		if (dwCode == m_pPrefs->GetSrcSortCol2())
			iCurSel = iRc;
		m_SourceCols2Combo.SetItemData(iRc = m_SourceCols2Combo.AddString(strResStr), dwCode |= MLC_SORTDESC);
		if (dwCode == m_pPrefs->GetSrcSortCol2())
			iCurSel = iRc;
	}
	m_SourceCols2Combo.SetCurSel(iCurSel);
}
