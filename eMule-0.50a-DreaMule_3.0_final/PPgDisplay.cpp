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
#include "SearchDlg.h"
#include "PPgDisplay.h"
#include <dlgs.h>
#include "HTRichEditCtrl.h"
#include "Preferences.h"
#include "OtherFunctions.h"
#include "emuledlg.h"
#include "TransferWnd.h"
#include "ServerWnd.h"

//Xman
#include "opcodes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CPPgDisplay, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgDisplay, CPropertyPage)
	ON_BN_CLICKED(IDC_MINTRAY, OnSettingsChange)
	ON_BN_CLICKED(IDC_DBLCLICK, OnSettingsChange)
	ON_EN_CHANGE(IDC_TOOLTIPDELAY, OnSettingsChange)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_UPDATEQUEUE, OnSettingsChange)
	ON_BN_CLICKED(IDC_SHOWRATEONTITLE, OnSettingsChange)
	ON_BN_CLICKED(IDC_DISABLEHIST , OnSettingsChange)
	ON_BN_CLICKED(IDC_DISABLEKNOWNLIST, OnSettingsChange)
	ON_BN_CLICKED(IDC_DISABLEQUEUELIST, OnSettingsChange)
	ON_BN_CLICKED(IDC_SHOWCATINFO, OnSettingsChange)
	ON_BN_CLICKED(IDC_SHOWDWLPERCENT, OnSettingsChange)
	ON_BN_CLICKED(IDC_REPAINT,OnSettingsChange)
	ON_BN_CLICKED(IDC_SELECT_HYPERTEXT_FONT, OnBnClickedSelectHypertextFont)
	ON_BN_CLICKED(IDC_CLEARCOMPL,OnSettingsChange)
	ON_BN_CLICKED(IDC_SHOWTRANSTOOLBAR,OnSettingsChange)
	ON_BN_CLICKED(IDC_RESETHIST, OnBtnClickedResetHist)
	ON_BN_CLICKED(IDC_MINIMIZEONCLOSE, OnSettingsChange) //>>> WiZaRd::Minimize on Close
	ON_BN_CLICKED(IDC_DISPLAYSTATUSSMILIES, OnSettingsChange) //>>> WiZaRd::StatusSmilies
	// Tux: LiteMule: Remove Help
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
	DDX_Control(pDX, IDC_PREVIEW, m_3DPreview);
}

void CPPgDisplay::LoadSettings(void)
{
	if(thePrefs.mintotray)
		CheckDlgButton(IDC_MINTRAY,1);
	else
		CheckDlgButton(IDC_MINTRAY,0);

	if(thePrefs.transferDoubleclick)
		CheckDlgButton(IDC_DBLCLICK,1);
	else
		CheckDlgButton(IDC_DBLCLICK,0);

	if(thePrefs.showRatesInTitle)
		CheckDlgButton(IDC_SHOWRATEONTITLE,1);
	else
		CheckDlgButton(IDC_SHOWRATEONTITLE,0);

	if(thePrefs.m_bupdatequeuelist)
		CheckDlgButton(IDC_UPDATEQUEUE,0);
	else
		CheckDlgButton(IDC_UPDATEQUEUE,1);

	if(thePrefs.m_bDisableKnownClientList)
		CheckDlgButton(IDC_DISABLEKNOWNLIST,1);
	else
		CheckDlgButton(IDC_DISABLEKNOWNLIST,0);

	if(thePrefs.m_bDisableQueueList)
		CheckDlgButton(IDC_DISABLEQUEUELIST,1);
	else
		CheckDlgButton(IDC_DISABLEQUEUELIST,0);

	CheckDlgButton(IDC_SHOWCATINFO,(UINT)thePrefs.ShowCatTabInfos());
	CheckDlgButton(IDC_REPAINT,(UINT)thePrefs.IsGraphRecreateDisabled() );
	CheckDlgButton(IDC_SHOWDWLPERCENT,(UINT)thePrefs.GetUseDwlPercentage() );
	CheckDlgButton(IDC_CLEARCOMPL, (uint8)thePrefs.GetRemoveFinishedDownloads());
	CheckDlgButton(IDC_SHOWTRANSTOOLBAR, (uint8)thePrefs.IsTransToolbarEnabled());

	CheckDlgButton(IDC_DISABLEHIST, (uint8)thePrefs.GetUseAutocompletion());

	CheckDlgButton(IDC_MINIMIZEONCLOSE, thePrefs.GetMinimizeOnClose() ? BST_CHECKED : BST_UNCHECKED); //>>> WiZaRd::Minimize on Close
	CheckDlgButton(IDC_DISPLAYSTATUSSMILIES, thePrefs.IsDisplayStatusSmilies() ? BST_CHECKED : BST_UNCHECKED); //>>> WiZaRd::StatusSmilies

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
	DrawPreview();


	LoadSettings();
	Localize();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPgDisplay::OnApply()
{
	TCHAR buffer[510];
	
	bool mintotray_old = thePrefs.mintotray;
	thePrefs.mintotray = IsDlgButtonChecked(IDC_MINTRAY)!=0;
	thePrefs.transferDoubleclick = IsDlgButtonChecked(IDC_DBLCLICK)!=0;
	thePrefs.depth3D = ((CSliderCtrl*)GetDlgItem(IDC_3DDEPTH))->GetPos();
	thePrefs.dontRecreateGraphs = IsDlgButtonChecked(IDC_REPAINT)!=0;
	thePrefs.m_bShowDwlPercentage = IsDlgButtonChecked(IDC_SHOWDWLPERCENT)!=0;
	thePrefs.m_bRemoveFinishedDownloads = IsDlgButtonChecked(IDC_CLEARCOMPL)!=0;
	thePrefs.m_bUseAutocompl = IsDlgButtonChecked(IDC_DISABLEHIST)!=0;

	if(IsDlgButtonChecked(IDC_UPDATEQUEUE))
		thePrefs.m_bupdatequeuelist = false;
	else
		thePrefs.m_bupdatequeuelist = true;

	if(IsDlgButtonChecked(IDC_SHOWRATEONTITLE))
		thePrefs.showRatesInTitle= true;
	else
		thePrefs.showRatesInTitle= false;

	thePrefs.ShowCatTabInfos(IsDlgButtonChecked(IDC_SHOWCATINFO)!=0);
	if (!thePrefs.ShowCatTabInfos())
		theApp.emuledlg->transferwnd->UpdateCatTabTitles();

	bool bListDisabled = false;
	bool bResetToolbar = false;
	if (thePrefs.m_bDisableKnownClientList != (IsDlgButtonChecked(IDC_DISABLEKNOWNLIST) != 0)) {
		thePrefs.m_bDisableKnownClientList = (IsDlgButtonChecked(IDC_DISABLEKNOWNLIST) != 0);
		//Xman Code Fix
		if (thePrefs.m_bDisableKnownClientList)
		{
			bListDisabled = true;
			theApp.emuledlg->transferwnd->clientlistctrl.DeleteAllItems();
		}
		else
			theApp.emuledlg->transferwnd->clientlistctrl.ShowKnownClients();
		//Xman end
		bResetToolbar = true;
	}

	if (thePrefs.m_bDisableQueueList != (IsDlgButtonChecked(IDC_DISABLEQUEUELIST) != 0)) {
		thePrefs.m_bDisableQueueList = (IsDlgButtonChecked(IDC_DISABLEQUEUELIST) != 0);
		//Xman Code Fix
		if (thePrefs.m_bDisableQueueList)
		{
			bListDisabled = true;
			theApp.emuledlg->transferwnd->queuelistctrl.DeleteAllItems();
		}
		else
			theApp.emuledlg->transferwnd->queuelistctrl.ShowQueueClients();
		//Xman end
		bResetToolbar = true;
	}

	GetDlgItem(IDC_TOOLTIPDELAY)->GetWindowText(buffer,20);
	if(_tstoi(buffer) > 32)
		thePrefs.m_iToolDelayTime = 32;
	else
		thePrefs.m_iToolDelayTime = _tstoi(buffer);
	theApp.emuledlg->SetToolTipsDelay(thePrefs.GetToolTipDelay()*1000);
	
	theApp.emuledlg->transferwnd->downloadlistctrl.SetStyle();

	if (bListDisabled)
		theApp.emuledlg->transferwnd->OnDisableList();
	if ((IsDlgButtonChecked(IDC_SHOWTRANSTOOLBAR) != 0) != thePrefs.IsTransToolbarEnabled()){
		thePrefs.m_bWinaTransToolbar = !thePrefs.m_bWinaTransToolbar;
		theApp.emuledlg->transferwnd->ResetTransToolbar(thePrefs.m_bWinaTransToolbar);
	}
	else if ((IsDlgButtonChecked(IDC_SHOWTRANSTOOLBAR) != 0) && bResetToolbar){
		theApp.emuledlg->transferwnd->ResetTransToolbar(thePrefs.m_bWinaTransToolbar);
	}

	thePrefs.SetMinimizeOnClose(IsDlgButtonChecked(IDC_MINIMIZEONCLOSE) != 0); //>>> WiZaRd::Minimize on Close
	thePrefs.SetDisplayStatusSmilies(IsDlgButtonChecked(IDC_DISPLAYSTATUSSMILIES) != 0); //>>> WiZaRd::StatusSmilies

	LoadSettings();

	if (mintotray_old != thePrefs.mintotray)
		theApp.emuledlg->TrayMinimizeToTrayChange();
	if (!thePrefs.ShowRatesOnTitle()) {
	//	_stprintf(buffer,_T("eMule v%s"),theApp.m_strCurVersionLong+ _T(" ") + MOD_VERSION); // Xman // Maella -Support for tag ET_MOD_VERSION 0x55
		_stprintf(buffer,MOD_VERSION);
		theApp.emuledlg->SetWindowText(buffer);
	}


	SetModified(FALSE);
	return CPropertyPage::OnApply();
}

void CPPgDisplay::Localize(void)
{
	if(m_hWnd)
	{
		SetWindowText(GetResString(IDS_PW_DISPLAY));
		GetDlgItem(IDC_MINTRAY)->SetWindowText(GetResString(IDS_PW_TRAY));
		GetDlgItem(IDC_DBLCLICK)->SetWindowText(GetResString(IDS_PW_DBLCLICK));
		GetDlgItem(IDC_TOOLTIPDELAY_LBL)->SetWindowText(GetResString(IDS_PW_TOOL));
		GetDlgItem(IDC_3DDEP)->SetWindowText(GetResString(IDS_3DDEP));
		GetDlgItem(IDC_FLAT)->SetWindowText(GetResString(IDS_FLAT));
		GetDlgItem(IDC_ROUND)->SetWindowText(GetResString(IDS_ROUND));
		GetDlgItem(IDC_UPDATEQUEUE)->SetWindowText(GetResString(IDS_UPDATEQUEUE));
		GetDlgItem(IDC_SHOWRATEONTITLE)->SetWindowText(GetResString(IDS_SHOWRATEONTITLE));
		GetDlgItem(IDC_DISABLEKNOWNLIST)->SetWindowText(GetResString(IDS_DISABLEKNOWNLIST));
		GetDlgItem(IDC_DISABLEQUEUELIST)->SetWindowText(GetResString(IDS_DISABLEQUEUELIST));
		GetDlgItem(IDC_STATIC_CPUMEM)->SetWindowText(GetResString(IDS_STATIC_CPUMEM));
		GetDlgItem(IDC_SHOWCATINFO)->SetWindowText(GetResString(IDS_SHOWCATINFO));
		GetDlgItem(IDC_REPAINT)->SetWindowText(GetResString(IDS_REPAINTGRAPHS));
		SetDlgItemText(IDC_HYPERTEXT_FONT_HINT, GetResString(IDS_HYPERTEXT_FONT_HINT));
		SetDlgItemText(IDC_SELECT_HYPERTEXT_FONT, GetResString(IDS_SELECT_FONT) + _T("..."));
		SetDlgItemText(IDC_SHOWDWLPERCENT, GetResString(IDS_SHOWDWLPERCENTAGE));
		GetDlgItem(IDC_CLEARCOMPL)->SetWindowText(GetResString(IDS_AUTOREMOVEFD));

		GetDlgItem(IDC_RESETLABEL)->SetWindowText(GetResString(IDS_RESETLABEL));
		GetDlgItem(IDC_RESETHIST)->SetWindowText(GetResString(IDS_PW_RESET));
		GetDlgItem(IDC_DISABLEHIST)->SetWindowText(GetResString(IDS_ENABLED));

		GetDlgItem(IDC_SHOWTRANSTOOLBAR)->SetWindowText(GetResString(IDS_PW_SHOWTRANSTOOLBAR));

		GetDlgItem(IDC_MINIMIZEONCLOSE)->SetWindowText(GetResString(IDS_MINIMIZEONCLOSE)); //>>> WiZaRd::Minimize on Close
		GetDlgItem(IDC_DISPLAYSTATUSSMILIES)->SetWindowText(GetResString(IDS_DISPLAYSTATUSSMILIES)); //>>> WiZaRd::StatusSmilies
	}
}

void CPPgDisplay::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	SetModified(TRUE);

	UpdateData(false); 
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);

	DrawPreview();
}

// NOTE: Can't use 'lCustData' for a structure which would hold that static members,
// because '_pfnChooseFontHook' will be needed *before* WM_INITDIALOG (which would
// give as the 'lCustData').
LPCFHOOKPROC _pfnChooseFontHook = NULL;
CPPgDisplay* _pThis = NULL;

UINT CALLBACK CPPgDisplay::ChooseFontHook(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	UINT uResult;

	// Call MFC's common dialog Hook function
	if (_pfnChooseFontHook != NULL)
		uResult = (*_pfnChooseFontHook)(hdlg, uiMsg, wParam, lParam);
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
				if (_pThis->m_eSelectFont == sfLog)
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
	   ::GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);

	// Initialize 'CFontDialog'
	CFontDialog dlg(&lf, CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT);
	dlg.m_cf.Flags |= CF_APPLY | CF_ENABLEHOOK;

	// Set 'lpfnHook' to our own Hook function. But save MFC's hook!
	_pfnChooseFontHook = dlg.m_cf.lpfnHook;
	dlg.m_cf.lpfnHook = ChooseFontHook;
	_pThis = this;

	if (dlg.DoModal() == IDOK)
	{
		if (m_eSelectFont == sfLog)
			theApp.emuledlg->ApplyLogFont(&lf);
		else
			theApp.emuledlg->ApplyHyperTextFont(&lf);
	}

	_pfnChooseFontHook = NULL;
	_pThis = NULL;
}

void CPPgDisplay::OnBtnClickedResetHist()
{
	theApp.emuledlg->searchwnd->ResetHistory();
	theApp.emuledlg->serverwnd->ResetHistory();
}
// Tux: LiteMule: Remove Help: removed code block

void CPPgDisplay::DrawPreview()
{
	int dep=((CSliderCtrl*)GetDlgItem(IDC_3DDEPTH))->GetPos();
	m_3DPreview.SetSliderPos( dep);
}
