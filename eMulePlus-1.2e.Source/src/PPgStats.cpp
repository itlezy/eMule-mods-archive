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

#include "stdafx.h"
#include "emule.h"
#include "PPgStats.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CPPgStats, CPropertyPage)
CPPgStats::CPPgStats()
	: CPropertyPage(CPPgStats::IDD)
	, m_iGraphsUpdateInterval(0)
	, m_iAverageGraphTime(0)
	, m_iStatisticsUpdInterval(0)
{
}

CPPgStats::~CPPgStats()
{
}

void CPPgStats::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COLOR_BUTTON, m_ColorButton);
	DDX_Control(pDX, IDC_COLORSELECTOR, m_ColorsCombo);
	DDX_Control(pDX, IDC_RATIO, m_RatioCombo);
	DDX_Text(pDX, IDC_ACSTAT, m_strStat);
	DDX_Control(pDX, IDC_SLIDER, m_UpdateIntervalSlider);
	DDX_Control(pDX, IDC_SLIDER3, m_AverageGraphTimeSlider);
	DDX_Control(pDX, IDC_SLIDER2, m_StatisticsUpdateIntervalSlider);
	DDX_Slider(pDX, IDC_SLIDER, m_iGraphsUpdateInterval);
	DDX_Slider(pDX, IDC_SLIDER3, m_iAverageGraphTime);
	DDX_Slider(pDX, IDC_SLIDER2, m_iStatisticsUpdInterval);
}


BEGIN_MESSAGE_MAP(CPPgStats, CPropertyPage)
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_ACSTAT, OnSettingsChange)
	ON_CBN_SELCHANGE(IDC_COLORSELECTOR, OnCbnSelchangeColorselector)
	ON_MESSAGE(CPN_SELCHANGE, OnSelChange)
	ON_BN_CLICKED(IDC_COLOR_BUTTON, OnSettingsChange)
	ON_CBN_SELCHANGE(IDC_RATIO, OnSettingsChange)
END_MESSAGE_MAP()


BOOL CPPgStats::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	for (int i = 0; i < 12; i++)
		m_dwStatColors[i] = g_App.m_pPrefs->GetStatsColor(i);

	m_UpdateIntervalSlider.SetRange(0, 120, true);
	m_StatisticsUpdateIntervalSlider.SetRange(4, 120, true);
	m_AverageGraphTimeSlider.SetRange(1, 60, true);

	m_iGraphsUpdateInterval = m_pPrefs->GetTrafficOMeterInterval();
	m_iStatisticsUpdInterval = m_pPrefs->GetStatsInterval();
	m_iAverageGraphTime = m_pPrefs->GetStatsAverageMinutes();
	
	m_strStat.Format(_T("%u"), m_pPrefs->GetStatsMax());
	
	m_RatioCombo.AddString(_T("1:1"));
	m_RatioCombo.AddString(_T("1:2"));
	m_RatioCombo.AddString(_T("1:3"));
	m_RatioCombo.AddString(_T("1:4"));
	m_RatioCombo.AddString(_T("1:5"));
	m_RatioCombo.AddString(_T("1:10"));
	m_RatioCombo.AddString(_T("%"));
	int n = m_pPrefs->GetGraphRatio();
	if (n <= 5)
		m_RatioCombo.SetCurSel(n - 1);
	else if (n == 10)
		m_RatioCombo.SetCurSel(5); // item 5 = 1:10
	else
		m_RatioCombo.SetCurSel(6); // item 6 = %
	m_ColorButton.TrackSelection = TRUE;
	Localize();
	UpdateData(FALSE);
	SetModified(FALSE);

	return TRUE;
}

BOOL CPPgStats::OnApply()
{
	if(m_bModified)
	{
		UpdateData(TRUE);

		for (int i = 0; i < 12; i++)
			g_App.m_pPrefs->SetStatsColor(i, m_dwStatColors[i]);

		g_App.m_pPrefs->SetTrafficOMeterInterval(static_cast<uint16>(m_iGraphsUpdateInterval));
		g_App.m_pPrefs->SetStatsInterval(static_cast<uint16>(m_iStatisticsUpdInterval));
		g_App.m_pPrefs->SetStatsAverageMinutes(static_cast<byte>(m_iAverageGraphTime));
		int n = m_RatioCombo.GetCurSel();
		m_pPrefs->SetGraphRatio(static_cast<byte>((n == 6) ? 255 : ((n == 5) ? 10 : n + 1)));
		g_App.m_pMDlg->m_dlgStatistics.Localize();
		g_App.m_pMDlg->m_dlgStatistics.ShowInterval();

		uint16		uActiveConnScale = static_cast<uint16>(_tstoi(m_strStat));

		if(uActiveConnScale > m_pPrefs->GetMaxConnections())
		{
			uActiveConnScale = m_pPrefs->GetMaxConnections();
			m_strStat.Format(_T("%u"), uActiveConnScale);
		}
		m_pPrefs->SetStatsMax(uActiveConnScale);

		UpdateData(FALSE);

		g_App.m_pMDlg->m_dlgStatistics.UpdateActConScale();
		g_App.m_pMDlg->m_dlgStatistics.RepaintMeters();

		SetModified(FALSE);
	}

	return CPropertyPage::OnApply();
}

void CPPgStats::Localize(void)
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_GRAPHS, IDS_GRAPHS },
		{ IDC_STREE, IDS_STREE },
		{ IDC_ACSTAT_LBL, IDS_ACSTAT_LBL },
		{ IDC_ACRATIO_LBL, IDS_ACRATIO_LBL },
		{ IDC_PREFCOLORS, IDS_COLORS }
	};
	static const UINT s_auResTbl2[] =
	{
		IDS_SP_BACKGROUND, IDS_SP_GRID,
		IDS_SP_DL1, IDS_SP_DL2, IDS_SP_DL3,
		IDS_SP_UL1, IDS_SP_UL2, IDS_SP_UL3,
		IDS_SP_ACTCON, IDS_SP_ACTUL, IDS_SP_ACTDL
	};

	if (::IsWindow(m_hWnd))
	{
		CString	strRes;

		for (uint32 i = 0; i < ARRSIZE(s_auResTbl); i++)
		{
			::GetResString(&strRes, static_cast<UINT>(s_auResTbl[i][1]));
			SetDlgItemText(s_auResTbl[i][0], strRes);
		}

		m_ColorsCombo.ResetContent();
		for (unsigned ui = 0; ui < ARRSIZE(s_auResTbl2); ui++)
		{
			::GetResString(&strRes, static_cast<UINT>(s_auResTbl2[ui]));
			m_ColorsCombo.AddString(strRes);
		}
		m_ColorsCombo.SetCurSel(0);

		ShowInterval();
		::GetResString(&strRes, IDS_COL_MORECOLORS);
		m_ColorButton.CustomText = strRes;
		::GetResString(&strRes, IDS_COL_AUTOMATIC);
		m_ColorButton.DefaultText = strRes;
		OnCbnSelchangeColorselector();
	}
}

void CPPgStats::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	SetModified(TRUE);
	UpdateData(TRUE);

	ShowInterval();
	
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPPgStats::ShowInterval()
{
	CString	strRes;

	if (m_iGraphsUpdateInterval == 0)
		GetResString(&strRes, IDS_DISABLED);
	else
		strRes.Format(GetResString(IDS_STATS_UPDATELABEL), m_iGraphsUpdateInterval);
	SetDlgItemText(IDC_SLIDERINFO, strRes);
	
	strRes.Format(GetResString(IDS_STATS_UPDATELABEL), m_iStatisticsUpdInterval);
	SetDlgItemText(IDC_SLIDERINFO2, strRes);
	strRes.Format(GetResString(IDS_STATS_AVGLABEL), m_iAverageGraphTime);
	SetDlgItemText(IDC_SLIDERINFO3, strRes);
}

void CPPgStats::OnCbnSelchangeColorselector()
{
	int	iIdx = m_ColorsCombo.GetCurSel();

	m_ColorButton.SetColor((COLORREF)m_dwStatColors[iIdx]);
	m_ColorButton.SetDefaultColor(m_pPrefs->GetDefaultStatsColor(iIdx));
}

LONG CPPgStats::OnSelChange(UINT newColor, LONG /*wParam*/)
{
	m_dwStatColors[m_ColorsCombo.GetCurSel()] = (COLORREF)newColor;
	SetModified(TRUE);
	return TRUE;
}
