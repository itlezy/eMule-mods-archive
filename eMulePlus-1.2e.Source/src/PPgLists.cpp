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
#include "PPgLists.h"
#include "IP2Country.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CPPgLists, CPropertyPage)
CPPgLists::CPPgLists()
	: CPropertyPage(CPPgLists::IDD)
	, m_bShowA4AF(FALSE)
	, m_bShowA4AFCount(FALSE)
	, m_bShowAvgDataRate(FALSE)
	, m_bShowFileTypeIcons(FALSE)
	, m_bShowTransferredOnCompleted(FALSE)
	, m_bShowDownloadPercentage(FALSE)
	, m_bShowPausedGray(FALSE)
	, m_bShowFileStatusIcons(FALSE)
	, m_bShowCountryFlag(FALSE)
	, m_bRoundSizes(FALSE)
	, m_bDisplayUploadParts(FALSE)
	, m_bSmartFilterShowSourcesOQ(FALSE)
	, m_bShowRatingIcons(FALSE)
{
}

CPPgLists::~CPPgLists()
{
}

void CPPgLists::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_A4AF, m_bShowA4AF);
	DDX_Check(pDX, IDC_A4AFCOUNT, m_bShowA4AFCount);
	DDX_Check(pDX, IDC_SHOWAVGDATARATE, m_bShowAvgDataRate);
	DDX_Check(pDX, IDC_SHOWFTYPE, m_bShowFileTypeIcons);
	DDX_Check(pDX, IDC_SHOWTRANFONCMPLT, m_bShowTransferredOnCompleted);
	DDX_Check(pDX, IDC_SHOWDWLPERCENTAGE, m_bShowDownloadPercentage);
	DDX_Check(pDX, IDC_SHOWGRAY, m_bShowPausedGray);
	DDX_Check(pDX, IDC_STATUSICONS, m_bShowFileStatusIcons);
	DDX_Check(pDX, IDC_TBSSHOWCOUNTRYFLAG, m_bShowCountryFlag);
	DDX_Check(pDX, IDC_ROUNDSIZES, m_bRoundSizes);
	DDX_Check(pDX, IDC_UPLOADPARTS, m_bDisplayUploadParts);
	DDX_Text(pDX, IDC_SMARTFILTERMAXQR, m_strSmartFilterMaxQR);
	DDX_Check(pDX, IDC_SMARTFILTERSHOWOQ, m_bSmartFilterShowSourcesOQ);
	DDX_Check(pDX, IDC_SHOWRATINGICONS, m_bShowRatingIcons);
	DDX_Control(pDX, IDC_COLOR_BUTTON, m_FakeListColorButton);
}

BEGIN_MESSAGE_MAP(CPPgLists, CPropertyPage)
	ON_BN_CLICKED(IDC_A4AF, OnSettingsChange)
	ON_BN_CLICKED(IDC_A4AFCOUNT, OnSettingsChange)
	ON_BN_CLICKED(IDC_SHOWAVGDATARATE, OnSettingsChange)
	ON_BN_CLICKED(IDC_SHOWFTYPE, OnSettingsChange)
	ON_BN_CLICKED(IDC_SHOWTRANFONCMPLT, OnSettingsChange)
	ON_BN_CLICKED(IDC_SHOWDWLPERCENTAGE, OnSettingsChange)
	ON_BN_CLICKED(IDC_SHOWGRAY, OnSettingsChange)
	ON_BN_CLICKED(IDC_STATUSICONS, OnSettingsChange)
	ON_BN_CLICKED(IDC_TBSSHOWCOUNTRYFLAG, OnSettingsChange)
	ON_BN_CLICKED(IDC_ROUNDSIZES, OnSettingsChange)
	ON_EN_CHANGE(IDC_SMARTFILTERMAXQR, OnSettingsChange)
	ON_BN_CLICKED(IDC_SMARTFILTERSHOWOQ, OnSettingsChange)
	ON_BN_CLICKED(IDC_SHOWRATINGICONS, OnSettingsChange)
	ON_BN_CLICKED(IDC_UPLOADPARTS, OnSettingsChange)
	ON_MESSAGE(CPN_SELCHANGE, OnColorButtonSelChange)
END_MESSAGE_MAP()

BOOL CPPgLists::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_FakeListColorButton.SetDefaultColor(0);

	LoadSettings();
	Localize();

	return TRUE;
}

void CPPgLists::LoadSettings(void)
{
	m_bShowA4AF = m_pPrefs->IsA4AFStringEnabled();
	m_bShowA4AFCount = m_pPrefs->IsA4AFCountEnabled();
	m_bShowAvgDataRate = m_pPrefs->GetShowAverageDataRate();
	m_bShowFileTypeIcons = m_pPrefs->ShowFileTypeIcon();
	m_bShowTransferredOnCompleted = m_pPrefs->IsTransferredOnCompleted();
	m_bShowDownloadPercentage = m_pPrefs->GetUseDwlPercentage();
	m_bShowPausedGray = m_pPrefs->ShowPausedGray();
	m_bShowFileStatusIcons = m_pPrefs->ShowFullFileStatusIcons();
	m_bShowCountryFlag = m_pPrefs->GetShowCountryFlag();
	m_bRoundSizes = m_pPrefs->ShowRoundSizes();
	m_bDisplayUploadParts = m_pPrefs->IsUploadPartsEnabled();
	m_strSmartFilterMaxQR.Format(_T("%u"), m_pPrefs->GetSmartFilterMaxQueueRank());
	m_bSmartFilterShowSourcesOQ = m_pPrefs->GetSmartFilterShowOnQueue();
	m_bShowRatingIcons = m_pPrefs->ShowRatingIcons();
	m_FakeListColorButton.SetColor(m_pPrefs->GetFakeListDownloadColor());

	UpdateData(FALSE);

	SetModified(FALSE);
}

BOOL CPPgLists::OnApply()
{
	if(m_bModified)
	{
		UpdateData(TRUE);

		bool		bPrevVal, bUpd1, bUpd2;
		COLORREF	crPrev;

		m_pPrefs->SetA4AFStringEnabled(B2b(m_bShowA4AF));
		m_pPrefs->SetA4AFCountEnabled(B2b(m_bShowA4AFCount));
		m_pPrefs->SetShowAverageDataRate(B2b(m_bShowAvgDataRate));
		m_pPrefs->SetTransferredOnCompleted(B2b(m_bShowTransferredOnCompleted));
		m_pPrefs->SetShowPausedGray(B2b(m_bShowPausedGray));
		m_pPrefs->SetShowCountryFlag(B2b(m_bShowCountryFlag));
		m_pPrefs->SetShowRoundSizes(B2b(m_bRoundSizes));
		m_pPrefs->SetSmartFilterMaxQueueRank(static_cast<uint16>(_tstoi(m_strSmartFilterMaxQR)));
		m_pPrefs->SetSmartFilterShowOnQueue(B2b(m_bSmartFilterShowSourcesOQ));
		
		bPrevVal = m_pPrefs->GetUseDwlPercentage();
		m_pPrefs->SetUseDwlPercentage(B2b(m_bShowDownloadPercentage));
		bUpd2 = (bPrevVal != m_pPrefs->GetUseDwlPercentage());
		bPrevVal = m_pPrefs->ShowFullFileStatusIcons();
		m_pPrefs->SetShowFullFileStatusIcons(B2b(m_bShowFileStatusIcons));
		bUpd2 |= (bPrevVal != m_pPrefs->ShowFullFileStatusIcons());
		crPrev = m_pPrefs->GetFakeListDownloadColor();
		m_pPrefs->SetFakeListDownloadColor(m_FakeListColorButton.GetColor());
		bUpd2 |= (crPrev != m_pPrefs->GetFakeListDownloadColor());

		bPrevVal = m_pPrefs->ShowFileTypeIcon();
		m_pPrefs->SetShowFileTypeIcon(B2b(m_bShowFileTypeIcons));
		bUpd1 = (bPrevVal != m_pPrefs->ShowFileTypeIcon());
		bPrevVal = m_pPrefs->ShowRatingIcons();
		m_pPrefs->SetShowRatingIcons(B2b(m_bShowRatingIcons));
		bUpd1 |= (bPrevVal != m_pPrefs->ShowRatingIcons());

		if (bUpd1 || bUpd2)
			g_App.m_pMDlg->m_wndTransfer.m_ctlDownloadList.Invalidate();
		if (bUpd1)
		{
			g_App.m_pMDlg->m_wndSharedFiles.m_ctlSharedFilesList.Invalidate();
			g_App.m_pMDlg->m_dlgSearch.m_ctlSearchList.Invalidate();
		}

		if (m_pPrefs->GetShowCountryFlag())
			g_App.m_pIP2Country->Load();
		else
			g_App.m_pIP2Country->Unload();

		if (m_pPrefs->IsUploadPartsEnabled() == !m_bDisplayUploadParts)
		{
			m_pPrefs->SetUploadPartsEnabled(B2b(m_bDisplayUploadParts));

			if(m_pPrefs->IsUploadPartsEnabled())
			{
				g_App.m_pMDlg->m_wndTransfer.m_ctlUploadList.ShowColumn(ULCOL_PROGRESS);
				g_App.m_pMDlg->m_wndTransfer.m_ctlQueueList.ShowColumn(QLCOL_PROGRESS);		
			}
			else
			{
				g_App.m_pMDlg->m_wndTransfer.m_ctlUploadList.HideColumn(ULCOL_PROGRESS);
				g_App.m_pMDlg->m_wndTransfer.m_ctlQueueList.HideColumn(QLCOL_PROGRESS);		
			}
		}

		g_App.m_pIP2Country->Refresh();

		SetModified(FALSE);
	}

	return CPropertyPage::OnApply();
}

void CPPgLists::Localize(void)
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_A4AF, IDS_A4AF },
		{ IDC_A4AFCOUNT, IDS_A4AFCOUNT },
		{ IDC_SHOWAVGDATARATE, IDS_SHOWAVGDATARATE },
		{ IDC_SHOWFTYPE, IDS_SHOWFTYPE },
		{ IDC_SHOWTRANFONCMPLT, IDS_SHOWTRANFONCMPLT },
		{ IDC_SHOWDWLPERCENTAGE, IDS_SHOWDWLPERCENTAGE },
		{ IDC_SHOWGRAY, IDS_SHOWGRAY },
		{ IDC_STATUSICONS, IDS_STATUSICONS },
		{ IDC_TBSSHOWCOUNTRYFLAG, IDS_PW_SHOWCOUNTRYFLAG },
		{ IDC_ROUNDSIZES, IDS_ROUNDSIZES },
		{ IDC_UPLOADPARTS, IDS_UPLOADPARTS },
		{ IDC_SMARTFILTERSHOWOQ, IDS_SMARTFILTER_SHOWOQ },
		{ IDC_SMARTFILTERMAXQRLBL, IDS_SMARTFILTER_MAXQR },
		{ IDC_SMARTFILTERGROUPBOX, IDS_SRCFILTERMENU_SMARTFILTER },
		{ IDC_SHOWRATINGICONS, IDS_SHOWRATINGICONS },
		{ IDC_FAKELIST_COLOR_STATIC, IDS_FAKELIST_COLOR_STATIC },
		{ IDC_FAKECHECK_GROUPBOX, IDS_FAKE_CHECK_HEADER }
	};

	if (::IsWindow(m_hWnd))
	{
		CString	strRes;

		for (uint32 i = 0; i < ARRSIZE(s_auResTbl); i++)
		{
			::GetResString(&strRes, static_cast<UINT>(s_auResTbl[i][1]));
			SetDlgItemText(s_auResTbl[i][0], strRes);
		}
		::GetResString(&strRes, IDS_COL_MORECOLORS);
		m_FakeListColorButton.CustomText = strRes;
		::GetResString(&strRes, IDS_COL_AUTOMATIC);
		m_FakeListColorButton.DefaultText = strRes;
	}
}
