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
#include "HTRichEditCtrl.h"
#include "Preferences.h"
#include "OtherFunctions.h"
#include "emuledlg.h"
#include "TransferDlg.h"
#include "ServerWnd.h"
//Xman
#include "opcodes.h"
#include "SharedFilesWnd.h" // NEO: AKF - [AllKnownFiles] <-- Xanatos --
#include "IPFilter.h"// X: [NIPFD] - [No IPFilter Description]
#include "SearchResultsWnd.h"// WiZaRd

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_TOOLTIP_DELAY_SEC	32


IMPLEMENT_DYNAMIC(CPPgDisplay, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgDisplay, CPropertyPage)
	ON_BN_CLICKED(IDC_MINTRAY, OnSettingsChange)
	ON_BN_CLICKED(IDC_DBLCLICK, OnSettingsChange)
	ON_EN_CHANGE(IDC_TOOLTIPDELAY, OnSettingsChange)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_SHOWRATEONTITLE, OnSettingsChange)
	ON_BN_CLICKED(IDC_DISABLEHIST , OnSettingsChange)
	ON_BN_CLICKED(IDC_DISABLEKNOWNLIST, OnSettingsChange)
	ON_BN_CLICKED(IDC_DISABLEQUEUELIST, OnSettingsChange)
	ON_BN_CLICKED(IDC_NOIPFDESC,OnSettingsChange)// X: [NIPFD] - [No IPFilter Description]
	ON_BN_CLICKED(IDC_SHOWCATINFO, OnSettingsChange)
	ON_BN_CLICKED(IDC_SHOWDWLPERCENT, OnSettingsChange)
	ON_BN_CLICKED(IDC_SELECT_HYPERTEXT_FONT, OnBnClickedSelectHypertextFont)
	ON_BN_CLICKED(IDC_CLEARCOMPL,OnSettingsChange)
	ON_BN_CLICKED(IDC_SHOWTRANSTOOLBAR,OnSettingsChange)
	ON_BN_CLICKED(IDC_STORESEARCHES, OnSettingsChange)
	ON_BN_CLICKED(IDC_WIN7TASKBARGOODIES, OnSettingsChange)
	ON_BN_CLICKED(IDC_RESETHIST, OnBtnClickedResetHist)
END_MESSAGE_MAP()

CPPgDisplay::CPPgDisplay()
	: CPropertyPage(CPPgDisplay::IDD)
{
	m_eSelectFont = sfServer;
}

CPPgDisplay::~CPPgDisplay()
{
}

void CPPgDisplay::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

void CPPgDisplay::LoadSettings(void)
{
	CheckDlgButton(IDC_MINTRAY,thePrefs.mintotray);
	CheckDlgButton(IDC_DBLCLICK,thePrefs.transferDoubleclick);
	CheckDlgButton(IDC_SHOWRATEONTITLE,thePrefs.showRatesInTitle);
	CheckDlgButton(IDC_DISABLEKNOWNLIST,thePrefs.m_bDisableKnownClientList);
	CheckDlgButton(IDC_DISABLEQUEUELIST,thePrefs.m_bDisableQueueList);
	CheckDlgButton(IDC_NOIPFDESC, thePrefs.noIPFilterDesc);// X: [NIPFD] - [No IPFilter Description]

	CheckDlgButton(IDC_STORESEARCHES,thePrefs.IsStoringSearchesEnabled());
	CheckDlgButton(IDC_SHOWCATINFO,(UINT)thePrefs.ShowCatTabInfos());
	CheckDlgButton(IDC_SHOWDWLPERCENT,(UINT)thePrefs.GetUseDwlPercentage() );
	CheckDlgButton(IDC_CLEARCOMPL, (uint8)thePrefs.GetRemoveFinishedDownloads());
	CheckDlgButton(IDC_SHOWTRANSTOOLBAR, (uint8)thePrefs.IsTransToolbarEnabled());
	CheckDlgButton(IDC_DISABLEHIST, (uint8)thePrefs.GetUseAutocompletion());
	
#ifndef HAVE_WIN7_SDK_H
	GetDlgItem(IDC_WIN7TASKBARGOODIES)->EnableWindow(FALSE);
#else
	if ( thePrefs.GetWindowsVersion() >= _WINVER_7_)
		CheckDlgButton(IDC_WIN7TASKBARGOODIES, (uint8)thePrefs.IsWin7TaskbarGoodiesEnabled());	
	else
		GetDlgItem(IDC_WIN7TASKBARGOODIES)->EnableWindow(FALSE);
#endif

	SetDlgItemInt(IDC_TOOLTIPDELAY, thePrefs.m_iToolDelayTime, FALSE);
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

	CSpinButtonCtrl *pSpinCtrl = (CSpinButtonCtrl *)GetDlgItem(IDC_TOOLTIPDELAY_SPIN);
	if (pSpinCtrl)
		pSpinCtrl->SetRange(0, MAX_TOOLTIP_DELAY_SEC);

	LoadSettings();
	Localize();
	m_bModified = false; // X: [CI] - [Code Improvement] Apply if modified

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPgDisplay::OnApply()
{
	if(m_bModified){ // X: [CI] - [Code Improvement] Apply if modified
		bool mintotray_old = thePrefs.mintotray;
		thePrefs.mintotray = IsDlgButtonChecked(IDC_MINTRAY)!=0;
		thePrefs.transferDoubleclick = IsDlgButtonChecked(IDC_DBLCLICK)!=0;
		thePrefs.depth3D = ((CSliderCtrl*)GetDlgItem(IDC_3DDEPTH))->GetPos();
		thePrefs.m_bShowDwlPercentage = IsDlgButtonChecked(IDC_SHOWDWLPERCENT)!=0;
		thePrefs.m_bRemoveFinishedDownloads = IsDlgButtonChecked(IDC_CLEARCOMPL)!=0;
		thePrefs.m_bUseAutocompl = IsDlgButtonChecked(IDC_DISABLEHIST)!=0;
		thePrefs.m_bStoreSearches = IsDlgButtonChecked(IDC_STORESEARCHES) != 0;

#ifdef HAVE_WIN7_SDK_H
		thePrefs.m_bShowWin7TaskbarGoodies = IsDlgButtonChecked(IDC_WIN7TASKBARGOODIES) != 0;
		theApp.emuledlg->EnableTaskbarGoodies(thePrefs.m_bShowWin7TaskbarGoodies);
#endif

		thePrefs.showRatesInTitle = IsDlgButtonChecked(IDC_SHOWRATEONTITLE)!=0;

		thePrefs.ShowCatTabInfos(IsDlgButtonChecked(IDC_SHOWCATINFO) != 0);
		if (!thePrefs.ShowCatTabInfos())
			theApp.emuledlg->transferwnd->UpdateCatTabTitles();

		bool bListDisabled = false;
		bool bResetToolbar = false;
		if (thePrefs.m_bDisableKnownClientList != (IsDlgButtonChecked(IDC_DISABLEKNOWNLIST) != 0)) {
			thePrefs.m_bDisableKnownClientList = (IsDlgButtonChecked(IDC_DISABLEKNOWNLIST) != 0);
			if (thePrefs.m_bDisableKnownClientList)
				bListDisabled = true;
			else
				theApp.emuledlg->transferwnd->GetClientList()->ShowKnownClients();
			bResetToolbar = true;
		}

		if (thePrefs.m_bDisableQueueList != (IsDlgButtonChecked(IDC_DISABLEQUEUELIST) != 0)) {
			thePrefs.m_bDisableQueueList = (IsDlgButtonChecked(IDC_DISABLEQUEUELIST) != 0);
			if (thePrefs.m_bDisableQueueList)
				bListDisabled = true;
			else
				theApp.emuledlg->transferwnd->GetQueueList()->ShowQueueClients();
			bResetToolbar = true;
		}

		UINT nToolDelayTime=GetDlgItemInt(IDC_TOOLTIPDELAY,NULL,FALSE);
		thePrefs.m_iToolDelayTime = (nToolDelayTime > MAX_TOOLTIP_DELAY_SEC)?MAX_TOOLTIP_DELAY_SEC:nToolDelayTime;

		theApp.emuledlg->SetToolTipsDelay(thePrefs.GetToolTipDelay()*1000);

		theApp.emuledlg->transferwnd->GetDownloadList()->SetStyle();
		theApp.emuledlg->searchwnd->m_pwndResults->searchlistctrl.SetStyle();// WiZaRd

		if (bListDisabled)
			theApp.emuledlg->transferwnd->OnDisableList();

		if ((IsDlgButtonChecked(IDC_SHOWTRANSTOOLBAR) != 0) != thePrefs.IsTransToolbarEnabled()) {
			thePrefs.m_bWinaTransToolbar = !thePrefs.m_bWinaTransToolbar;
			theApp.emuledlg->transferwnd->ResetTransToolbar(thePrefs.m_bWinaTransToolbar);
			theApp.emuledlg->sharedfileswnd->ResetShareToolbar(thePrefs.m_bWinaTransToolbar); // NEO: AKF - [AllKnownFiles] <-- Xanatos --
		}
		else if ((IsDlgButtonChecked(IDC_SHOWTRANSTOOLBAR) != 0) && bResetToolbar)
			theApp.emuledlg->transferwnd->ResetTransToolbar(thePrefs.m_bWinaTransToolbar);
		
		if(thePrefs.noIPFilterDesc != (IsDlgButtonChecked(IDC_NOIPFDESC) != 0)) {// X: [NIPFD] - [No IPFilter Description]
			thePrefs.noIPFilterDesc = (IsDlgButtonChecked(IDC_NOIPFDESC) != 0);
			CWaitCursor curHourglass;
			theApp.ipfilter->LoadFromDefaultFile();
			//if (thePrefs.GetFilterServerByIP())
			theApp.emuledlg->serverwnd->serverlistctrl.RemoveAllFilteredServers();
		}


		LoadSettings();

		if (mintotray_old != thePrefs.mintotray)
			theApp.emuledlg->TrayMinimizeToTrayChange();
		if (!thePrefs.ShowRatesOnTitle())
		theApp.emuledlg->SetWindowText(_T("eMule v") + theApp.m_strCurVersionLong);

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
		SetDlgItemText(IDC_MINTRAY,GetResString(IDS_PW_TRAY));
		SetDlgItemText(IDC_DBLCLICK,GetResString(IDS_PW_DBLCLICK));
		SetDlgItemText(IDC_TOOLTIPDELAY_LBL,GetResString(IDS_PW_TOOL));
		SetDlgItemText(IDC_3DDEP,GetResString(IDS_3DDEP));
		SetDlgItemText(IDC_FLAT,GetResString(IDS_FLAT));
		SetDlgItemText(IDC_ROUND,GetResString(IDS_ROUND));
		SetDlgItemText(IDC_SHOWRATEONTITLE,GetResString(IDS_SHOWRATEONTITLE) + _T(" / ") + GetResString(IDS_STATS_RUNTIME));
		SetDlgItemText(IDC_DISABLEKNOWNLIST,GetResString(IDS_DISABLEKNOWNLIST));
		SetDlgItemText(IDC_DISABLEQUEUELIST,GetResString(IDS_DISABLEQUEUELIST));
		SetDlgItemText(IDC_NOIPFDESC,GetResString(IDS_NOIPFDESC));// X: [NIPFD] - [No IPFilter Description]
		SetDlgItemText(IDC_STATIC_CPUMEM,GetResString(IDS_STATIC_CPUMEM));
		SetDlgItemText(IDC_SHOWCATINFO,GetResString(IDS_SHOWCATINFO));
		SetDlgItemText(IDC_HYPERTEXT_FONT_HINT, GetResString(IDS_HYPERTEXT_FONT_HINT));
		SetDlgItemText(IDC_SELECT_HYPERTEXT_FONT, GetResString(IDS_SELECT_FONT) + _T("..."));
		SetDlgItemText(IDC_SHOWDWLPERCENT, GetResString(IDS_SHOWDWLPERCENTAGE));
		SetDlgItemText(IDC_CLEARCOMPL,GetResString(IDS_AUTOREMOVEFD));
		SetDlgItemText(IDC_STORESEARCHES,GetResString(IDS_STORESEARCHES));

		SetDlgItemText(IDC_RESETLABEL,GetResString(IDS_RESETLABEL));
		SetDlgItemText(IDC_RESETHIST,GetResString(IDS_PW_RESET));
		SetDlgItemText(IDC_DISABLEHIST,GetResString(IDS_ENABLED));

		SetDlgItemText(IDC_SHOWTRANSTOOLBAR,GetResString(IDS_PW_SHOWTRANSTOOLBAR));
		SetDlgItemText(IDC_WIN7TASKBARGOODIES,GetResString(IDS_SHOWWIN7TASKBARGOODIES));
	}
}

void CPPgDisplay::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified

	UpdateData(false); 
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);

}

// NOTE: Can't use 'lCustData' for a structure which would hold that static members,
// because 's_pfnChooseFontHook' will be needed *before* WM_INITDIALOG (which would
// give as the 'lCustData').
static LPCFHOOKPROC s_pfnChooseFontHook = NULL;
static CPPgDisplay* s_pThis = NULL;

UINT_PTR CALLBACK CPPgDisplay::ChooseFontHook(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	UINT_PTR uResult;

	// Call MFC's common dialog Hook function
	if (s_pfnChooseFontHook != NULL)
		uResult = (*s_pfnChooseFontHook)(hdlg, uiMsg, wParam, lParam);
	else
		uResult = 0;

	// Do our own Hook processing
	switch (uiMsg)
	{
	case WM_COMMAND:
		if (LOWORD(wParam) == psh3/*Apply*/ && HIWORD(wParam) == BN_CLICKED)
		{
			LOGFONT lf;
			CFontDialog *pDlg = (CFontDialog *)CWnd::FromHandle(hdlg);
			ASSERT( pDlg != NULL );
			if (pDlg != NULL)
			{
				pDlg->GetCurrentFont(&lf);
				if (s_pThis->m_eSelectFont == sfLog)
					theApp.emuledlg->ApplyLogFont(&lf);
				else
					theApp.emuledlg->ApplyHyperTextFont(&lf);
			}
		}
		break;
	}

	// If the hook procedure returns zero, the default dialog box procedure processes the message.
	return uResult;
}

void CPPgDisplay::OnBnClickedSelectHypertextFont()
{
	if (GetAsyncKeyState(VK_CONTROL) < 0)
		m_eSelectFont = sfLog;
	else
		m_eSelectFont = sfServer;

	// get current font description
	CFont* pFont;
	if (m_eSelectFont == sfLog)
		pFont = &theApp.m_fontLog;
	else
		pFont = &theApp.m_fontHyperText;
	LOGFONT lf;
	if (pFont != NULL)
	   pFont->GetObject(sizeof(LOGFONT), &lf);
	else
		AfxGetMainWnd()->GetFont()->GetLogFont(&lf);

	// Initialize 'CFontDialog'
	CFontDialog dlg(&lf, CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT);
	dlg.m_cf.Flags |= CF_APPLY | CF_ENABLEHOOK;

	// Set 'lpfnHook' to our own Hook function. But save MFC's hook!
	s_pfnChooseFontHook = dlg.m_cf.lpfnHook;
	dlg.m_cf.lpfnHook = ChooseFontHook;
	s_pThis = this;

	if (dlg.DoModal() == IDOK)
	{
		if (m_eSelectFont == sfLog)
			theApp.emuledlg->ApplyLogFont(&lf);
		else
			theApp.emuledlg->ApplyHyperTextFont(&lf);
	}

	s_pfnChooseFontHook = NULL;
	s_pThis = NULL;
}

void CPPgDisplay::OnBtnClickedResetHist()
{
	theApp.emuledlg->searchwnd->ResetHistory();
	theApp.emuledlg->serverwnd->ResetHistory();
}
