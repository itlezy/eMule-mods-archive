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
#include "SearchDlg.h"
#include "PPgDisplay.h"
#include <dlgs.h>
#include "Preferences.h"
#include "OtherFunctions.h"
#include "emuledlg.h"
#include "TransferWnd.h"
#include "ServerWnd.h"
//Xman
#include "opcodes.h"
#include "SearchResultsWnd.h"// WiZaRd
//stats +
#include "StatisticsDlg.h"
#include "UserMsgs.h"
//stats -

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CPPgDisplay, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgDisplay, CPropertyPage)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_SHOWRATEONTITLE, OnSettingsChange)
	ON_BN_CLICKED(IDC_DISABLEHISTORYLIST, OnSettingsChange)
	ON_BN_CLICKED(IDC_SHOWCATINFO, OnSettingsChange)
	ON_BN_CLICKED(IDC_SHOWTRANSTOOLBAR,OnSettingsChange)
	ON_BN_CLICKED(IDC_STORESEARCHES, OnSettingsChange)
//stats +
        ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_FILL_GRAPHS, OnSettingsChange)
//stats -
END_MESSAGE_MAP()

CPPgDisplay::CPPgDisplay()
	: CPropertyPage(CPPgDisplay::IDD)
{
//stats +
    m_iGraphsUpdate = 0;
	m_iGraphsAvgTime = 0;
	m_iStatsUpdate = 0;
	m_iStatsColors = 0;
	m_pdwStatsColors = NULL;
	//m_bModified = FALSE;
	m_bFillGraphs = false; 
//stats -
}

//stats +
void CPPgDisplay::OnDestroy()
{
	delete[] m_pdwStatsColors;
	m_pdwStatsColors = NULL;
}
//stats -

CPPgDisplay::~CPPgDisplay()
{
}

void CPPgDisplay::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
//stats +
    DDX_Control(pDX, IDC_SLIDER, m_ctlGraphsUpdate);
	DDX_Control(pDX, IDC_SLIDER2, m_ctlStatsUpdate);
	DDX_Control(pDX, IDC_SLIDER3, m_ctlGraphsAvgTime);
//stats -
}

void CPPgDisplay::LoadSettings(void)
{
	CheckDlgButton(IDC_SHOWRATEONTITLE,thePrefs.showRatesInTitle);
	CheckDlgButton(IDC_DISABLEHISTORYLIST,thePrefs.m_bDisableHistoryList);

	CheckDlgButton(IDC_STORESEARCHES,thePrefs.IsStoringSearchesEnabled());
	CheckDlgButton(IDC_SHOWCATINFO,(UINT)thePrefs.ShowCatTabInfos());
	CheckDlgButton(IDC_SHOWTRANSTOOLBAR, (uint8)thePrefs.IsTransToolbarEnabled());
}

BOOL CPPgDisplay::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	// Barry - Controls depth of 3d colour shading
	CSliderCtrl *slider3D = (CSliderCtrl*)GetDlgItem(IDC_3DDEPTH);
	slider3D->SetRange(0, 5, true);
	slider3D->SetPos(thePrefs.Get3DDepth());
	slider3D->SetTicFreq(1);

//stats +
m_ctlGraphsUpdate.SetRange(0, 200, TRUE);// X: [CI] - [Code Improvement]

	m_ctlGraphsUpdate.SetPos(thePrefs.GetTrafficOMeterInterval());
	m_ctlGraphsUpdate.SetTicFreq(10);
	m_ctlGraphsUpdate.SetPageSize(10);

	m_ctlStatsUpdate.SetPos(thePrefs.GetStatsInterval());
	m_ctlStatsUpdate.SetTicFreq(10);
	m_ctlStatsUpdate.SetPageSize(10);

	m_ctlGraphsAvgTime.SetRange(0, 99);
	m_ctlGraphsAvgTime.SetPos(thePrefs.GetStatsAverageMinutes() - 1);
	for (int i = 10; i < 100; i += 10)
		m_ctlGraphsAvgTime.SetTic(i - 1);
	m_ctlGraphsAvgTime.SetPageSize(10);

	m_iGraphsUpdate = thePrefs.GetTrafficOMeterInterval();
	m_iGraphsAvgTime = thePrefs.GetStatsInterval();
	m_iStatsUpdate = thePrefs.GetStatsAverageMinutes();

	CheckDlgButton(IDC_FILL_GRAPHS, thePrefs.GetFillGraphs());

	m_iStatsColors = thePrefs.GetNumStatsColors();
	m_pdwStatsColors = new DWORD[m_iStatsColors];
	thePrefs.GetAllStatsColors(m_iStatsColors, m_pdwStatsColors);
//stats -

	LoadSettings();
	Localize();
	SetModified(FALSE); //stats ?
	m_bModified = false; // X: [CI] - [Code Improvement] Apply if modified

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPgDisplay::OnApply()
{
	if(m_bModified){ // X: [CI] - [Code Improvement] Apply if modified
	thePrefs.depth3D = ((CSliderCtrl*)GetDlgItem(IDC_3DDEPTH))->GetPos();
	thePrefs.m_bStoreSearches = IsDlgButtonChecked(IDC_STORESEARCHES) != 0;
	thePrefs.showRatesInTitle = IsDlgButtonChecked(IDC_SHOWRATEONTITLE)!=0;

	thePrefs.ShowCatTabInfos(IsDlgButtonChecked(IDC_SHOWCATINFO) != 0);
	if (!thePrefs.ShowCatTabInfos())
		theApp.emuledlg->transferwnd->UpdateCatTabTitles();

	bool bListDisabled = false;
	bool bResetToolbar = false;
	
	if (thePrefs.m_bDisableHistoryList != (IsDlgButtonChecked(IDC_DISABLEHISTORYLIST) != 0)) {
		thePrefs.m_bDisableHistoryList = (IsDlgButtonChecked(IDC_DISABLEHISTORYLIST) != 0);
		if (thePrefs.m_bDisableHistoryList)
			bListDisabled = true;
		else
			theApp.emuledlg->transferwnd->historylistctrl.ReloadFileList();
		bResetToolbar = true;
	}

	if (bListDisabled)
		theApp.emuledlg->transferwnd->OnDisableList();

	if ((IsDlgButtonChecked(IDC_SHOWTRANSTOOLBAR) != 0) != thePrefs.IsTransToolbarEnabled()) {
		thePrefs.m_bWinaTransToolbar = !thePrefs.m_bWinaTransToolbar;
		theApp.emuledlg->transferwnd->ResetTransToolbar(thePrefs.m_bWinaTransToolbar);
	}
	else if ((IsDlgButtonChecked(IDC_SHOWTRANSTOOLBAR) != 0) && bResetToolbar)
		theApp.emuledlg->transferwnd->ResetTransToolbar(thePrefs.m_bWinaTransToolbar);
	LoadSettings();

	if (!thePrefs.ShowRatesOnTitle())
		theApp.emuledlg->SetWindowText(_T("eMule v") + theApp.m_strCurVersionLong + _T(" -LPE-"));

//stats +
		bool bInvalidateGraphs = false;

		if (thePrefs.SetAllStatsColors(m_iStatsColors, m_pdwStatsColors)){
			//theApp.emuledlg->ShowTransferRate(true); // X: [SGW] - [SpeedGraphWnd] remove meter icon
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
		
                if (thePrefs.GetFillGraphs() != (IsDlgButtonChecked(IDC_FILL_GRAPHS) == BST_CHECKED)){
			thePrefs.SetFillGraphs(!thePrefs.GetFillGraphs());
			bInvalidateGraphs = true;
		}

		if (bInvalidateGraphs){
			//theApp.emuledlg->statisticswnd->RepaintMeters();//theApp.emuledlg->statisticswnd->Localize();// X: [RUL] - [Remove Useless Localize]
			theApp.emuledlg->statisticswnd->ShowInterval();
		}
		theApp.emuledlg->statisticswnd->RepaintMeters();
		theApp.emuledlg->statisticswnd->GetDlgItem(IDC_STATTREE)->EnableWindow(thePrefs.GetStatsInterval() > 0);
//stats -
		SetModified(FALSE);
		m_bModified = false; // X: [CI] - [Code Improvement] Apply if modified
	}
	return CPropertyPage::OnApply();
}

void CPPgDisplay::Localize(void)
{
	if(m_hWnd)
	{
		SetWindowText(GetResString(IDS_PW_DISPLAY));
		SetDlgItemText(IDC_3DDEP,GetResString(IDS_3DDEP));
		SetDlgItemText(IDC_FLAT,GetResString(IDS_FLAT));
		SetDlgItemText(IDC_ROUND,GetResString(IDS_ROUND));
		SetDlgItemText(IDC_SHOWRATEONTITLE,GetResString(IDS_SHOWRATEONTITLE) + _T(" / ") + GetResString(IDS_STATS_RUNTIME));
		SetDlgItemText(IDC_DISABLEHISTORYLIST,GetResString(IDS_DISABLEHISTORYLIST));
		SetDlgItemText(IDC_STATIC_CPUMEM,GetResString(IDS_STATIC_CPUMEM));
		SetDlgItemText(IDC_SHOWCATINFO,GetResString(IDS_SHOWCATINFO));
		SetDlgItemText(IDC_STORESEARCHES,GetResString(IDS_STORESEARCHES));

		SetDlgItemText(IDC_SHOWTRANSTOOLBAR,GetResString(IDS_PW_SHOWTRANSTOOLBAR));

//stats +
	SetDlgItemText(IDC_GRAPHS,GetResString(IDS_GRAPHS));
		SetDlgItemText(IDC_STREE,GetResString(IDS_STREE));
		SetWindowText(GetResString(IDS_STATSSETUPINFO));
		SetDlgItemText(IDC_FILL_GRAPHS, GetResString(IDS_FILLGRAPHS) );
	        
		ShowInterval();
//stats -
	}
}

void CPPgDisplay::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
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

void CPPgDisplay::ShowInterval()
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