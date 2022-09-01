// PPgDesign.cpp : implementation file
//

#include "stdafx.h"
#ifdef DESIGN_SETTINGS
#include "emule.h"
#include "PPgDesign.h"
#include "Preferences.h"
#include "SharedFilesWnd.h"
#include "SharedFilesCtrl.h"
#include "UserMsgs.h"
#include "emuledlg.h"
#include "TransferDlg.h"
#include "ServerWnd.h"
#include "ServerListCtrl.h"
#include "MuleStatusBarCtrl.h"
#include "IrcWnd.h"
#include "KademliaWnd.h"
#include "SearchDlg.h"
#include "ChatWnd.h"
#include "StatisticsDlg.h"
#include "MuleToolbarCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

// CPPgDesign dialog

IMPLEMENT_DYNAMIC(CPPgDesign, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgDesign, CPropertyPage)
	ON_BN_CLICKED(IDC_COLOR_ON_OFF, OnBnClickedOnOff)
	ON_BN_CLICKED(IDC_COLOR_BOLD, OnBnClickedBold)
	ON_BN_CLICKED(IDC_COLOR_UNDERLINED, OnBnClickedUnderlined)
	ON_BN_CLICKED(IDC_COLOR_ITALIC, OnBnClickedItalic)
    ON_MESSAGE(UM_CPN_SELCHANGE, OnColorPopupSelChange)
	ON_CBN_SELCHANGE(IDC_COLOR_MASTER_COMBO, OnCbnSelchangeStyleselMaster)
	ON_CBN_SELCHANGE(IDC_COLOR_SUB_COMBO, OnCbnSelchangeStyleselSub)
	ON_BN_CLICKED(IDC_COLOR_AUTOCHECK, OnCbClickedAutoText)
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

CPPgDesign::CPPgDesign()
: CPropertyPage(CPPgDesign::IDD)
{
}

CPPgDesign::~CPPgDesign()
{
}

void CPPgDesign::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COLOR_MASTER_COMBO, m_MasterCombo);
	DDX_Control(pDX, IDC_COLOR_SUB_COMBO, m_SubCombo);
	DDX_Control(pDX, IDC_COLOR_ON_OFF, m_OnOff);
	DDX_Control(pDX, IDC_COLOR_BOLD, m_bold);
	DDX_Control(pDX, IDC_COLOR_UNDERLINED, m_underlined);
	DDX_Control(pDX, IDC_COLOR_ITALIC, m_italic);
	DDX_Control(pDX, IDC_COLOR_FONT, m_FontColor);
	DDX_Control(pDX, IDC_COLOR_BACK, m_BackColor);
}

BOOL CPPgDesign::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_bDesignChanged = false;
	m_bold.SetWindowText(_T(""));
	m_bold.SetIcon((HICON)::LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE((int)IDI_FONTB), IMAGE_ICON, 16, 16, 0));
	m_underlined.SetWindowText(_T(""));
	m_underlined.SetIcon((HICON)::LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE((int)IDI_FONTUL), IMAGE_ICON, 16, 16, 0));
	m_italic.SetWindowText(_T(""));
	m_italic.SetIcon((HICON)::LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE((int)IDI_FONTI), IMAGE_ICON, 16, 16, 0));
	m_bold.SetFont(theApp.GetBoldFont());
	m_underlined.SetFont(theApp.GetULFont());
	m_italic.SetFont(theApp.GetItalicFont());

	m_FontColor.SetColor(GetSysColor(COLOR_WINDOWTEXT));
	m_FontColor.SetDefaultColor(GetSysColor(COLOR_WINDOWTEXT));
	m_BackColor.SetColor(COLORREF(RGB(255,255,255)));
	m_BackColor.SetDefaultColor(COLORREF(RGB(255,255,255)));

	LoadSettings();
	Localize();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPPgDesign::LoadSettings(void)
{
	if(m_hWnd)
	{
		for (int i=0;i<style_c_count;i++)
			thePrefs.GetStyle(client_styles, i, &nClientStyles[i]);

		for (int i=0;i<style_d_count;i++)
			thePrefs.GetStyle(download_styles, i, &nDownloadStyles[i]);

		for (int i=0;i<style_s_count;i++)
			thePrefs.GetStyle(share_styles, i, &nShareStyles[i]);

		for (int i=0;i<style_se_count;i++)
			thePrefs.GetStyle(server_styles, i, &nServerStyles[i]);

		for (int i=0;i<style_b_count;i++)
			thePrefs.GetStyle(background_styles, i, &nBackgroundStyles[i]);

		for (int i=0;i<style_w_count;i++)
			thePrefs.GetStyle(window_styles, i, &nWindowStyles[i]);

		if(thePrefs.GetAutoTextColors())
			CheckDlgButton(IDC_COLOR_AUTOCHECK,1);
		else
			CheckDlgButton(IDC_COLOR_AUTOCHECK,0);
	}
}

BOOL CPPgDesign::OnApply()
{
	for (int i=0;i<style_c_count;i++)
		thePrefs.SetStyle(client_styles, i, &nClientStyles[i]);

	for (int i=0;i<style_d_count;i++)
		thePrefs.SetStyle(download_styles, i, &nDownloadStyles[i]);

	for (int i=0;i<style_s_count;i++)
		thePrefs.SetStyle(share_styles, i, &nShareStyles[i]);

	for (int i=0;i<style_se_count;i++)
		thePrefs.SetStyle(server_styles, i, &nServerStyles[i]);

	for (int i=0;i<style_b_count;i++)
		thePrefs.SetStyle(background_styles, i, &nBackgroundStyles[i]);

	for (int i=0;i<style_w_count;i++)
		thePrefs.SetStyle(window_styles, i, &nWindowStyles[i]);

	if(IsDlgButtonChecked(IDC_COLOR_AUTOCHECK))
		thePrefs.m_bAutoTextColors = true;
	else
		thePrefs.m_bAutoTextColors = false;

	if(m_bDesignChanged)
	{
		m_bDesignChanged = false;

		theApp.emuledlg->kademliawnd->OnBackcolor();
		theApp.emuledlg->kademliawnd->Localize();
		theApp.emuledlg->serverwnd->OnBackcolor();
		theApp.emuledlg->serverwnd->Localize();
		theApp.emuledlg->transferwnd->OnBackcolor();
		theApp.emuledlg->transferwnd->Localize();
		theApp.emuledlg->searchwnd->OnBackcolor();
		theApp.emuledlg->searchwnd->Localize();
		theApp.emuledlg->sharedfileswnd->OnBackcolor();
		theApp.emuledlg->sharedfileswnd->Localize();
		theApp.emuledlg->chatwnd->OnBackcolor();
		theApp.emuledlg->chatwnd->Localize();
		theApp.emuledlg->ircwnd->OnBackcolor();
		theApp.emuledlg->ircwnd->Localize();
		theApp.emuledlg->statisticswnd->OnBackcolor();
		theApp.emuledlg->statisticswnd->Localize();

		theApp.emuledlg->statusbar->UpdateColor();
		theApp.emuledlg->RedrawWindow();
		theApp.emuledlg->toolbar->UpdateBackground();
		theApp.emuledlg->toolbar->RedrawWindow();

		theApp.emuledlg->activewnd->RedrawWindow();//redraw only the active Wnd after color change - Max
	}

	LoadSettings();
	SetModified(FALSE);

	return CPropertyPage::OnApply();
}

void CPPgDesign::Localize(void)
{
	if(m_hWnd)
	{
		GetDlgItem(IDC_COLOR_BOX)->SetWindowText(GetResString(IDS_COLOR_BOX));
		GetDlgItem(IDC_COLOR_FONT_LABEL)->SetWindowText(GetResString(IDS_COLOR_FONT_LABEL));
		GetDlgItem(IDC_COLOR_BACK_LABEL)->SetWindowText(GetResString(IDS_COLOR_BACK_LABEL));
		GetDlgItem(IDC_COLOR_AUTOCHECK)->SetWindowText(GetResString(IDS_COLOR_AUTOCHECK));

		m_FontColor.CustomText = GetResString(IDS_COL_MORECOLORS);
		m_FontColor.DefaultText = GetResString(IDS_DEFAULT);
		m_BackColor.CustomText = GetResString(IDS_COL_MORECOLORS);
		m_BackColor.DefaultText = GetResString(IDS_DEFAULT);

		InitMasterStyleCombo();
	}
}

void CPPgDesign::OnHelp()
{
	//theApp.ShowHelp(0);
}

void CPPgDesign::InitMasterStyleCombo()
{
	int iSel = m_MasterCombo.GetCurSel();
	m_MasterCombo.ResetContent();
	m_MasterCombo.AddString(GetResString(IDS_COLOR_MASTER1));
	m_MasterCombo.AddString(GetResString(IDS_COLOR_MASTER2));
	m_MasterCombo.AddString(GetResString(IDS_COLOR_MASTER3));
	m_MasterCombo.AddString(GetResString(IDS_COLOR_MASTER4));
	m_MasterCombo.AddString(GetResString(IDS_COLOR_MASTER5));
	m_MasterCombo.AddString(GetResString(IDS_COLOR_MASTER6));

	m_MasterCombo.SetCurSel(iSel != CB_ERR ? iSel : 0);

	InitSubStyleCombo();

	UpdateStyles();

	return;
}

void CPPgDesign::InitSubStyleCombo()
{
	int iMasterSel = m_MasterCombo.GetCurSel();
	m_SubCombo.ResetContent();

	switch(iMasterSel)
	{
		case client_styles: // client styles
		{
			m_SubCombo.AddString(GetResString(IDS_DEFAULT));
			m_SubCombo.AddString(GetResString(IDS_COLOR_C1));
			m_SubCombo.AddString(GetResString(IDS_COLOR_C2));
			m_SubCombo.AddString(GetResString(IDS_COLOR_C3));
			m_SubCombo.AddString(GetResString(IDS_COLOR_C4));
			m_SubCombo.AddString(GetResString(IDS_COLOR_C5));
			m_SubCombo.AddString(GetResString(IDS_COLOR_C6));
			m_SubCombo.AddString(GetResString(IDS_COLOR_C7));
		}break;
		case download_styles: // download styles
		{
			m_SubCombo.AddString(GetResString(IDS_DEFAULT));
			m_SubCombo.AddString(GetResString(IDS_DOWNLOADING));
			m_SubCombo.AddString(GetResString(IDS_COMPLETE));
			m_SubCombo.AddString(GetResString(IDS_COMPLETING));
			m_SubCombo.AddString(GetResString(IDS_HASHING));
			m_SubCombo.AddString(GetResString(IDS_PAUSED));
			m_SubCombo.AddString(GetResString(IDS_STOPPED));
			m_SubCombo.AddString(GetResString(IDS_ERRORLIKE));
		}break;
		case share_styles: // share styles
		{
			m_SubCombo.AddString(GetResString(IDS_DEFAULT));
			m_SubCombo.AddString(GetResString(IDS_COLOR_S1));
			m_SubCombo.AddString(GetResString(IDS_COLOR_S2));
			CString strTemp = GetResString(IDS_PRIORITY) + _T(" (") + GetResString(IDS_PW_CON_UPLBL) + _T("): ");
			m_SubCombo.AddString(strTemp + GetResString(IDS_PRIOAUTO));
			m_SubCombo.AddString(strTemp + GetResString(IDS_PRIOVERYLOW));
			m_SubCombo.AddString(strTemp + GetResString(IDS_PRIOLOW));
			m_SubCombo.AddString(strTemp + GetResString(IDS_PRIONORMAL));
			m_SubCombo.AddString(strTemp + GetResString(IDS_PRIOHIGH));
			m_SubCombo.AddString(strTemp + GetResString(IDS_PRIORELEASE));
			m_SubCombo.AddString(GetResString(IDS_COLOR_PERM) + _T(": ") + GetResString(IDS_HIDDEN));
			m_SubCombo.AddString(GetResString(IDS_COLOR_PERM) + _T(": ") + GetResString(IDS_FSTATUS_FRIENDSONLY));
			m_SubCombo.AddString(GetResString(IDS_COLOR_PERM) + _T(": ") + GetResString(IDS_COMMUNITY));
			m_SubCombo.AddString(GetResString(IDS_COLOR_PERM) + _T(": ") + GetResString(IDS_PW_EVER));
			m_SubCombo.AddString(GetResString(IDS_COLOR_S3));
		}break;
		case server_styles: // server styles
		{
			m_SubCombo.AddString(GetResString(IDS_DEFAULT));
			m_SubCombo.AddString(GetResString(IDS_COLOR_SE1));
			m_SubCombo.AddString(GetResString(IDS_COLOR_SE2));
			m_SubCombo.AddString(GetResString(IDS_COLOR_SE3));
			m_SubCombo.AddString(GetResString(IDS_COLOR_SE4));
			m_SubCombo.AddString(GetResString(IDS_COLOR_SE5));
		}break;
		case background_styles: // background styles
		{
			m_SubCombo.AddString(GetResString(IDS_DEFAULT));
			m_SubCombo.AddString(GetResString(IDS_COLOR_B1));
			m_SubCombo.AddString(GetResString(IDS_COLOR_B2));
			m_SubCombo.AddString(GetResString(IDS_COLOR_B3));
			m_SubCombo.AddString(GetResString(IDS_COLOR_B4));
			m_SubCombo.AddString(GetResString(IDS_COLOR_B5));
			m_SubCombo.AddString(GetResString(IDS_COLOR_B6));
		}break;
		case window_styles: // window styles
		{
			m_SubCombo.AddString(GetResString(IDS_DEFAULT));
			m_SubCombo.AddString(GetResString(IDS_COLOR_W1));
			m_SubCombo.AddString(GetResString(IDS_COLOR_W2));
			m_SubCombo.AddString(GetResString(IDS_COLOR_W3));
			m_SubCombo.AddString(GetResString(IDS_COLOR_W4));
			m_SubCombo.AddString(GetResString(IDS_COLOR_W5));
			m_SubCombo.AddString(GetResString(IDS_COLOR_W6));
			m_SubCombo.AddString(GetResString(IDS_COLOR_W7));
			m_SubCombo.AddString(GetResString(IDS_COLOR_W8));
			m_SubCombo.AddString(GetResString(IDS_COLOR_W9));
			m_SubCombo.AddString(GetResString(IDS_COLOR_W10));
		}break;
		default:
			break;
	}

	m_SubCombo.SetCurSel(0); // alway first one!
}

void CPPgDesign::UpdateStyles()
{
	int iCurStyle = m_SubCombo.GetCurSel();
	int iMasterValue = m_MasterCombo.GetCurSel();
	StylesStruct styles;
	styles = GetStyle(iMasterValue, iCurStyle);
	bool bEnable = false;
	bool bOnOff = (iCurStyle != style_c_default && iCurStyle != style_d_default && iCurStyle != style_s_default && iCurStyle != style_se_default);

	// set default for background
	if(iMasterValue == window_styles)
		m_BackColor.SetDefaultColor(::GetSysColor(COLOR_BTNFACE));
	else
		m_BackColor.SetDefaultColor(::GetSysColor(COLOR_WINDOW));

	m_BackColor.SetColor(styles.nBackColor);
	if(iMasterValue < background_styles)
	{
		m_FontColor.SetColor(styles.nFontColor);
		bEnable = true;
	}

	if(bOnOff)
	{
		m_OnOff.EnableWindow(bEnable);

		if(styles.nOnOff != 0 && bEnable)
		{
			m_BackColor.EnableWindow(true);
			m_OnOff.SetWindowText( GetResString(IDS_COLOR_OFF) );
		}
		else
		{
			m_OnOff.SetWindowText( GetResString(IDS_COLOR_ON) );
			bEnable = false;
			m_BackColor.EnableWindow(iMasterValue >= background_styles);
		}
	}
	else
	{
		m_BackColor.EnableWindow(true);
		m_OnOff.EnableWindow(FALSE);
		m_OnOff.SetWindowText( GetResString(IDS_COLOR_ON) );
	}
	m_bold.EnableWindow(bEnable);
	m_underlined.EnableWindow(bEnable);
	m_italic.EnableWindow(bEnable);
	m_FontColor.EnableWindow(bEnable);

	int iStyle = (styles.nFlags & STYLE_FONTMASK);
	m_bold.SetCheck(iStyle== STYLE_BOLD ? 1:0);
	m_underlined.SetCheck(iStyle== STYLE_UNDERLINE ? 1:0);
	m_italic.SetCheck(iStyle== STYLE_ITALIC ? 1:0);

	SetStyle(iMasterValue, iCurStyle, &styles);

	RedrawWindow(); // work around all glitches. :D
}

void CPPgDesign::OnFontStyle(int iStyle)
{
	m_bold.SetCheck(iStyle==1 ? 1:0);
	m_underlined.SetCheck(iStyle==2 ? 1:0);
	m_italic.SetCheck(iStyle==3 ? 1:0);
	iStyle = 0 ; //Now select font

	if (m_bold.GetCheck())
		iStyle = STYLE_BOLD;
	else if (m_underlined.GetCheck())
		iStyle = STYLE_UNDERLINE;
	else if (m_italic.GetCheck())
		iStyle = STYLE_ITALIC;

	int iCurStyle = m_SubCombo.GetCurSel();
	int iMasterValue = m_MasterCombo.GetCurSel();
	StylesStruct styles;
	styles = GetStyle(iMasterValue, iCurStyle);

	DWORD flags = (styles.nFlags & ~STYLE_FONTMASK) | iStyle;
	if (flags != styles.nFlags)
	{
		SetModified();
		styles.nFlags = flags;
		SetStyle(iMasterValue, iCurStyle, &styles);
		UpdateStyles();
		m_bDesignChanged = true;
	}
}

LONG CPPgDesign::OnColorPopupSelChange(UINT /*lParam*/, LONG /*wParam*/)
{
	int iCurStyle = m_SubCombo.GetCurSel();
	int iMasterValue = m_MasterCombo.GetCurSel();
	StylesStruct styles;
	styles = GetStyle(iMasterValue, iCurStyle);

	// font
	if (iCurStyle >= 0)
	{
		COLORREF crColor = m_FontColor.GetColor();
		if (crColor != styles.nFontColor)
		{
			styles.nFontColor = crColor;
			SetModified(TRUE);
			SetStyle(iMasterValue, iCurStyle, &styles);
			UpdateStyles();
			m_bDesignChanged = true;
		}
	}

	// background
	if (iCurStyle >= 0)
	{
		COLORREF crColor = m_BackColor.GetColor();
		if (crColor != styles.nBackColor)
		{
			styles.nBackColor = crColor;
			SetModified(TRUE);
			SetStyle(iMasterValue, iCurStyle, &styles);
			UpdateStyles();
			m_bDesignChanged = true;
		}
	}
	return TRUE;
}

void CPPgDesign::OnBnClickedBold()
{
	OnFontStyle(m_bold.GetCheck() ? 1:0);
}

void CPPgDesign::OnBnClickedUnderlined()
{
	OnFontStyle(m_underlined.GetCheck() ? 2:0);
}

void CPPgDesign::OnBnClickedItalic()
{
	OnFontStyle(m_italic.GetCheck() ? 3:0);
}

void CPPgDesign::OnCbnSelchangeStyleselMaster()
{
	InitSubStyleCombo();
	UpdateStyles();
	if(m_bFocusWasOnCombo)
		m_MasterCombo.SetFocus();
	m_bFocusWasOnCombo = false;
}

void CPPgDesign::OnCbnSelchangeStyleselSub()
{
	UpdateStyles();
	if(m_bFocusWasOnCombo)
		m_SubCombo.SetFocus();
	m_bFocusWasOnCombo = false;
}

void CPPgDesign::OnBnClickedOnOff()
{
	int iCurStyle = m_SubCombo.GetCurSel();
	int iMasterValue = m_MasterCombo.GetCurSel();
	StylesStruct styles;
	styles = GetStyle(iMasterValue, iCurStyle);

	short sOnOff = styles.nOnOff;

	if(sOnOff == 1)
		sOnOff = 0;
	else
		sOnOff = 1;
	styles.nOnOff = sOnOff;

	SetStyle(iMasterValue, iCurStyle, &styles);
	UpdateStyles();
	SetModified(TRUE);
	m_bDesignChanged = true;
}

void CPPgDesign::OnCbClickedAutoText()
{
	m_bDesignChanged = true;
	SetModified(TRUE);
}

StylesStruct CPPgDesign::GetStyle(int nMaster, int nStyle)
{
	if(nMaster == client_styles)
		return nClientStyles[nStyle];
	else if(nMaster == download_styles)
		return nDownloadStyles[nStyle];
	else if(nMaster == share_styles)
		return nShareStyles[nStyle];
	else if(nMaster == server_styles)
		return nServerStyles[nStyle];
	else if(nMaster == background_styles)
		return nBackgroundStyles[nStyle];
	else if(nMaster == window_styles)
		return nWindowStyles[nStyle];
	return nClientStyles[nStyle];
}

void CPPgDesign::SetStyle(int nMaster, int nStyle, StylesStruct *style)
{
	switch(nMaster)
	{
		case client_styles:
			{
				nClientStyles[nStyle].nFlags = style->nFlags;
				nClientStyles[nStyle].nFontColor = style->nFontColor;
				nClientStyles[nStyle].nBackColor = style->nBackColor;
				nClientStyles[nStyle].nOnOff = style->nOnOff;
			}break;
		case download_styles:
			{
				nDownloadStyles[nStyle].nFlags = style->nFlags;
				nDownloadStyles[nStyle].nFontColor = style->nFontColor;
				nDownloadStyles[nStyle].nBackColor = style->nBackColor;
				nDownloadStyles[nStyle].nOnOff = style->nOnOff;
			}break;
		case share_styles:
			{
				nShareStyles[nStyle].nFlags = style->nFlags;
				nShareStyles[nStyle].nFontColor = style->nFontColor;
				nShareStyles[nStyle].nBackColor = style->nBackColor;
				nShareStyles[nStyle].nOnOff = style->nOnOff;
			}break;
		case server_styles:
			{
				nServerStyles[nStyle].nFlags = style->nFlags;
				nServerStyles[nStyle].nFontColor = style->nFontColor;
				nServerStyles[nStyle].nBackColor = style->nBackColor;
				nServerStyles[nStyle].nOnOff = style->nOnOff;
			}break;
		case background_styles:
			{
				nBackgroundStyles[nStyle].nBackColor = style->nBackColor;
			}break;
		case window_styles:
			{
				nWindowStyles[nStyle].nBackColor = style->nBackColor;
			}break;
		default:
			break;
	}
}

void CPPgDesign::OnEnKillfocusMasterCombo()
{
	m_bFocusWasOnCombo = true;
}

void CPPgDesign::OnEnKillfocusSubCombo()
{
	m_bFocusWasOnCombo = true;
}
#endif