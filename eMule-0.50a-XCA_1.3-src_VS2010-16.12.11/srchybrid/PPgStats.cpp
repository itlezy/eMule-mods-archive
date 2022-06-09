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
#include "PPgStats.h"
#include "OtherFunctions.h"
#include "emuledlg.h"
#include "Preferences.h"
#include "StatisticsDlg.h"
#include "UserMsgs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CPPgStats, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgStats, CPropertyPage)
	ON_WM_HSCROLL()
	ON_CBN_SELCHANGE(IDC_COLORSELECTOR, OnCbnSelChangeColorSelector)
    ON_MESSAGE(UM_CPN_SELCHANGE, OnColorPopupSelChange)
	ON_CBN_SELCHANGE(IDC_CRATIO, OnSettingsChange) // X: [CI] - [Code Improvement]
	ON_EN_CHANGE(IDC_CGRAPHSCALE, OnSettingsChange)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_FILL_GRAPHS, OnSettingsChange) // X: [CI] - [Code Improvement]
	ON_BN_CLICKED(IDC_ACCURATE_RADIO, OnSettingsChange) //Xman smooth-accurate-graph
	ON_BN_CLICKED(IDC_SMOOTH_RADIO, OnSettingsChange) //Xman smooth-accurate-graph
	ON_BN_CLICKED(IDC_SHOWADDITIONALGRAPH, OnSettingsChange) //Xman show additional graph lines
END_MESSAGE_MAP()

CPPgStats::CPPgStats()
	: CPropertyPage(CPPgStats::IDD)
{
	m_iGraphsUpdate = 0;
	m_iGraphsAvgTime = 0;
	m_iStatsUpdate = 0;
	m_iStatsColors = 0;
	m_pdwStatsColors = NULL;
	//m_bModified = FALSE;
	m_bFillGraphs = false;
}

CPPgStats::~CPPgStats()
{
}

void CPPgStats::OnDestroy()
{
	delete[] m_pdwStatsColors;
	m_pdwStatsColors = NULL;
}

void CPPgStats::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COLORSELECTOR, m_colors);
	DDX_Control(pDX, IDC_COLOR_BUTTON, m_ctlColor);
	DDX_Control(pDX, IDC_CRATIO, m_cratio);
	DDX_Control(pDX, IDC_SLIDER, m_ctlGraphsUpdate);
	DDX_Control(pDX, IDC_SLIDER2, m_ctlStatsUpdate);
	DDX_Control(pDX, IDC_SLIDER3, m_ctlGraphsAvgTime);
}

BOOL CPPgStats::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	m_ctlGraphsUpdate.SetRange(0, 200, TRUE);// X: [CI] - [Code Improvement]

	m_ctlGraphsUpdate.SetPos(thePrefs.GetTrafficOMeterInterval());
	m_ctlGraphsUpdate.SetTicFreq(10);
	m_ctlGraphsUpdate.SetPageSize(10);

	m_ctlStatsUpdate.SetPos(thePrefs.GetStatsInterval());
	m_ctlStatsUpdate.SetTicFreq(10);
	m_ctlStatsUpdate.SetPageSize(10);

	//Xman smooth-accurate-graph
	CheckDlgButton(thePrefs.usesmoothgraph?IDC_SMOOTH_RADIO:IDC_ACCURATE_RADIO, true);
	//Xman end

	m_ctlGraphsAvgTime.SetRange(0, 99);
	m_ctlGraphsAvgTime.SetPos(thePrefs.GetStatsAverageMinutes() - 1);
	for (int i = 10; i < 100; i += 10)
		m_ctlGraphsAvgTime.SetTic(i - 1);
	m_ctlGraphsAvgTime.SetPageSize(10);

	m_iGraphsUpdate = thePrefs.GetTrafficOMeterInterval();
	m_iGraphsAvgTime = thePrefs.GetStatsInterval();
	m_iStatsUpdate = thePrefs.GetStatsAverageMinutes();

	CheckDlgButton(IDC_FILL_GRAPHS, thePrefs.GetFillGraphs());
//Xman show additional graph lines
	CheckDlgButton(IDC_SHOWADDITIONALGRAPH, thePrefs.m_bShowAdditionalGraph);

	// Set the Connections Statistics Y-Axis Scale
	SetDlgItemInt(IDC_CGRAPHSCALE, thePrefs.GetStatsMax(), FALSE);

	// Build our ratio combo and select the item corresponding to the currently set preference
	m_cratio.AddString(_T("1:1"));
	m_cratio.AddString(_T("1:2"));
	m_cratio.AddString(_T("1:3"));
	m_cratio.AddString(_T("1:4"));
	m_cratio.AddString(_T("1:5"));
	m_cratio.AddString(_T("1:10"));
	m_cratio.AddString(_T("1:20"));
	int n = thePrefs.GetStatsConnectionsGraphRatio();
	m_cratio.SetCurSel((n == 10) ? 5: ((n == 20) ? 6 : n - 1));

	m_iStatsColors = thePrefs.GetNumStatsColors();
	m_pdwStatsColors = new DWORD[m_iStatsColors];
	thePrefs.GetAllStatsColors(m_iStatsColors, m_pdwStatsColors);

	Localize();
	SetModified(FALSE);
	m_bModified = false; // X: [CI] - [Code Improvement] Apply if modified

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPgStats::OnApply()
{
	//TODO: cache all parameters. stats should be redrawn (deleted) only if really needed
	if (m_bModified)
	{
		bool bInvalidateGraphs = false;

		if (thePrefs.SetAllStatsColors(m_iStatsColors, m_pdwStatsColors)){
			theApp.emuledlg->ShowTransferRate(true);
			bInvalidateGraphs = true;
		}

		if (thePrefs.GetTrafficOMeterInterval() != (UINT)m_iGraphsUpdate){
			thePrefs.SetTrafficOMeterInterval(m_iGraphsUpdate);
			bInvalidateGraphs = true;
		}
		if (thePrefs.GetStatsInterval() != (UINT)m_iGraphsAvgTime){
			thePrefs.SetStatsInterval(m_iGraphsAvgTime);
			bInvalidateGraphs = true;
		}
		if (thePrefs.GetStatsAverageMinutes() != (UINT)m_iStatsUpdate){
			thePrefs.SetStatsAverageMinutes(m_iStatsUpdate);
			bInvalidateGraphs = true;
		}
		

		//Xman smooth-accurate-graph
		thePrefs.usesmoothgraph=IsDlgButtonChecked(IDC_SMOOTH_RADIO)!=0;
		//Xman end

        //Xman show additional graph lines
		thePrefs.m_bShowAdditionalGraph=(IsDlgButtonChecked(IDC_SHOWADDITIONALGRAPH)!=0);
		//Xman end

		UINT statsMax = GetDlgItemInt(IDC_CGRAPHSCALE,NULL,FALSE);
		UINT maxconnp5=thePrefs.GetMaxConnections() + 5;
		if (statsMax > maxconnp5)
		{
			if (thePrefs.statsMax != maxconnp5){
				thePrefs.statsMax = maxconnp5;
				bInvalidateGraphs = true;
			}
			TCHAR buffer[20];
			_ultot_s(thePrefs.GetStatsMax(), buffer, 10); // X: [CI] - [Code Improvement]
			buffer[_countof(buffer) - 1] = _T('\0');
			SetDlgItemText(IDC_CGRAPHSCALE,buffer);
		}
		else
		{
			if (thePrefs.GetStatsMax() != statsMax){
				thePrefs.SetStatsMax(statsMax);
				bInvalidateGraphs = true;
			}
		}

		int n = m_cratio.GetCurSel();
		UINT uRatio = (n == 5) ? 10 : ((n == 6) ? 20 : n + 1); // Index 5 = 1:10 and 6 = 1:20
		if (thePrefs.GetStatsConnectionsGraphRatio() != uRatio){
			thePrefs.SetStatsConnectionsGraphRatio(uRatio); 
			bInvalidateGraphs = true;
		}

		if (thePrefs.GetFillGraphs() != (IsDlgButtonChecked(IDC_FILL_GRAPHS) == BST_CHECKED)){
			thePrefs.SetFillGraphs(!thePrefs.GetFillGraphs());
			bInvalidateGraphs = true;
		}

		if (bInvalidateGraphs){
			theApp.emuledlg->statisticswnd->UpdateConnectionsGraph(); // Set new Y upper bound and Y ratio for active connections.
			//theApp.emuledlg->statisticswnd->RepaintMeters();//theApp.emuledlg->statisticswnd->Localize();// X: [RUL] - [Remove Useless Localize]
			theApp.emuledlg->statisticswnd->ShowInterval();
		}
		theApp.emuledlg->statisticswnd->RepaintMeters();
		theApp.emuledlg->statisticswnd->GetDlgItem(IDC_STATTREE)->EnableWindow(thePrefs.GetStatsInterval() > 0);
		SetModified(FALSE);
		m_bModified = false; // X: [CI] - [Code Improvement] Apply if modified
	}
	return CPropertyPage::OnApply();
}

void CPPgStats::Localize(void)
{
	if (m_hWnd)
	{
		SetDlgItemText(IDC_GRAPHS,GetResString(IDS_GRAPHS));
		SetDlgItemText(IDC_STREE,GetResString(IDS_STREE));
		SetDlgItemText(IDC_STATIC_CGRAPHSCALE,GetResString(IDS_PPGSTATS_YSCALE));
		SetDlgItemText(IDC_STATIC_CGRAPHRATIO,GetResString(IDS_PPGSTATS_ACRATIO));
		SetWindowText(GetResString(IDS_STATSSETUPINFO));
		SetDlgItemText(IDC_PREFCOLORS,GetResString(IDS_COLORS));
		SetDlgItemText(IDC_FILL_GRAPHS, GetResString(IDS_FILLGRAPHS) );

		//Xman smooth-accurate-graph
		SetDlgItemText(IDC_SMOOTH_RADIO,GetResString(IDS_SMOOTHGRAPH));
		SetDlgItemText(IDC_ACCURATE_RADIO,GetResString(IDS_ACCURATEGRAPH));
		//Xman end
        //Xman show additional graph lines
		SetDlgItemText(IDC_SHOWADDITIONALGRAPH,GetResString(IDS_SHOWADDITIONALGRAPH));

		m_colors.ResetContent();
		int iItem;
		iItem = m_colors.AddString(GetResString(IDS_SP_BACKGROUND));		m_colors.SetItemData(iItem, 0);
		iItem = m_colors.AddString(GetResString(IDS_SP_GRID));				m_colors.SetItemData(iItem, 1);

		iItem = m_colors.AddString(GetResString(IDS_SP_DL3));				m_colors.SetItemData(iItem, 4);
		iItem = m_colors.AddString(GetResString(IDS_SP_DL2));				m_colors.SetItemData(iItem, 3);
		iItem = m_colors.AddString(GetResString(IDS_SP_DL1));				m_colors.SetItemData(iItem, 2);

		iItem = m_colors.AddString(GetResString(IDS_SP_UL3));				m_colors.SetItemData(iItem, 7);
		iItem = m_colors.AddString(GetResString(IDS_SP_UL2));				m_colors.SetItemData(iItem, 6);
		iItem = m_colors.AddString(GetResString(IDS_SP_UL1));				m_colors.SetItemData(iItem, 5);
		//Xman Xtreme Upload: this graph isn't shown at xtreme
		//iItem = m_colors.AddString(GetResString(IDS_SP_ULSLOTSNOOVERHEAD));	m_colors.SetItemData(iItem, 14);
		//iItem = m_colors.AddString(GetResString(IDS_SP_ULFRIENDS));			m_colors.SetItemData(iItem, 13);

		iItem = m_colors.AddString(GetResString(IDS_SP_ACTCON));			m_colors.SetItemData(iItem, 8);
		iItem = m_colors.AddString(GetResString(IDS_SP_ACTUL));				m_colors.SetItemData(iItem, 10);
		iItem = m_colors.AddString(GetResString(IDS_SP_ACTDL));			m_colors.SetItemData(iItem, 9);
		//iItem = m_colors.AddString(GetResString(IDS_SP_TOTALUL));			m_colors.SetItemData(iItem, 9);
		iItem = m_colors.AddString(GetResString(IDS_EMULE_CTRL_DATA));		m_colors.SetItemData(iItem, 12); //Maella Bandwidth control
		iItem = m_colors.AddString(GetResString(IDS_NETWORK_ADAPTER));		m_colors.SetItemData(iItem, 13); //Maella Bandwidth control

		//iItem = m_colors.AddString(GetResString(IDS_SP_ACTDL));				m_colors.SetItemData(iItem, 12);
		iItem = m_colors.AddString(GetResString(IDS_SP_ICONUPBAR));			m_colors.SetItemData(iItem, 14);
		iItem = m_colors.AddString(GetResString(IDS_SP_ICONBAR));			m_colors.SetItemData(iItem, 11);

		m_ctlColor.SetCustomText(GetResString(IDS_COL_MORECOLORS));
		m_ctlColor.SetDefaultText(NULL);

		m_colors.SetCurSel(0);
		OnCbnSelChangeColorSelector();
		ShowInterval();
	}
}

void CPPgStats::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CSliderCtrl* slider = (CSliderCtrl*)pScrollBar;
	int position = slider->GetPos();

	if (pScrollBar->GetSafeHwnd() == m_ctlGraphsUpdate.m_hWnd)
	{
		if (m_iGraphsUpdate != position){
			m_iGraphsUpdate = position;
			OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified
		}
	}
	else if (pScrollBar->GetSafeHwnd() == m_ctlStatsUpdate.m_hWnd)
	{
		if (m_iGraphsAvgTime != position){
			m_iGraphsAvgTime = position;
			OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified
		}
	}
	
	else
	{
		ASSERT( pScrollBar->GetSafeHwnd() == m_ctlGraphsAvgTime.m_hWnd );
		if (m_iStatsUpdate != position + 1){
			m_iStatsUpdate = position + 1;
			OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified
		}
	}

	ShowInterval();

	UpdateData(false); 
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPPgStats::ShowInterval()
{
	CString strLabel;
	
	if (m_iGraphsUpdate == 0)
		strLabel.Format(GetResString(IDS_DISABLED));
	else
		strLabel.Format(GetResString(IDS_STATS_UPDATELABEL), m_iGraphsUpdate);
	SetDlgItemText(IDC_SLIDERINFO,strLabel);

	if (m_iGraphsAvgTime == 0)
		strLabel.Format(GetResString(IDS_DISABLED));
	else
		strLabel.Format(GetResString(IDS_STATS_UPDATELABEL), m_iGraphsAvgTime);
	SetDlgItemText(IDC_SLIDERINFO2,strLabel);

	strLabel.Format(GetResString(IDS_STATS_AVGLABEL), m_iStatsUpdate);
	SetDlgItemText(IDC_SLIDERINFO3,strLabel);

}

void CPPgStats::OnCbnSelChangeColorSelector()
{
	int iSel = m_colors.GetCurSel();
	if (iSel >= 0)
	{
		DWORD_PTR iIndex = m_colors.GetItemData(iSel);
		if (/*iIndex >= 0 && */iIndex < m_iStatsColors)
			m_ctlColor.SetColor(m_pdwStatsColors[iIndex]);
	}
}

LRESULT CPPgStats::OnColorPopupSelChange(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	int iSel = m_colors.GetCurSel();
	if (iSel >= 0)
	{
		DWORD_PTR iIndex = m_colors.GetItemData(iSel);
		if (/*iIndex >= 0 && */iIndex < m_iStatsColors)
		{
			COLORREF crColor = m_ctlColor.GetColor();
			if (crColor != m_pdwStatsColors[iIndex])
			{
				m_pdwStatsColors[iIndex] = crColor;
				OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified
			}
		}
	}
	return TRUE;
}
