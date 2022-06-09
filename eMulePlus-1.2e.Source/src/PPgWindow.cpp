//	this file is part of eMule Plus
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
#include "PPgWindow.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CPPgWindow, CPropertyPage)
CPPgWindow::CPPgWindow()
	: CPropertyPage(CPPgWindow::IDD)
	, m_bStartMin(FALSE)
	, m_bMinTray(FALSE)
	, m_bCloseToTray(FALSE)
	, m_bPromptOnExit(FALSE)
	, m_bPromptOnDisconnect(FALSE)
	, m_bPromptOnFriendDel(FALSE)
	, m_bBringToForeground(FALSE)
	, m_bShowRateOnTitle(FALSE)
	, m_bShowSpeedMeterOnToolbar(FALSE)
{
}

CPPgWindow::~CPPgWindow()
{
}

void CPPgWindow::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_STARTMIN, m_bStartMin);
	DDX_Check(pDX, IDC_MINTRAY, m_bMinTray);
	DDX_Check(pDX, IDC_CB_TBN_CLOSETOTRAY, m_bCloseToTray);
	DDX_Check(pDX, IDC_EXIT, m_bPromptOnExit);
	DDX_Check(pDX, IDC_COMFIRM_DISCONNECT, m_bPromptOnDisconnect);
	DDX_Check(pDX, IDC_COMFIRM_FRIENDDEL, m_bPromptOnFriendDel);
	DDX_Check(pDX, IDC_BRINGTOFOREGROUND, m_bBringToForeground);
	DDX_Check(pDX, IDC_SHOWRATEONTITLE, m_bShowRateOnTitle);
	DDX_Check(pDX, IDC_TBSPEEDMETER, m_bShowSpeedMeterOnToolbar);
	DDX_Check(pDX, IDC_KEEP_SEARCH_HISTORY, m_bKeepSearchHistory);
	DDX_Control(pDX, IDC_USEDFONT, fontPreviewCombo);
	DDX_Control(pDX, IDC_USEDFONTSIZE, fontSizeCombo);
}

BEGIN_MESSAGE_MAP(CPPgWindow, CPropertyPage)
	ON_BN_CLICKED(IDC_STARTMIN, OnSettingsChange)
	ON_BN_CLICKED(IDC_MINTRAY, OnSettingsChange)
	ON_BN_CLICKED(IDC_EXIT, OnSettingsChange)
	ON_BN_CLICKED(IDC_COMFIRM_DISCONNECT, OnSettingsChange)
	ON_BN_CLICKED(IDC_COMFIRM_FRIENDDEL, OnSettingsChange)
	ON_BN_CLICKED(IDC_BRINGTOFOREGROUND, OnSettingsChange)
	ON_BN_CLICKED(IDC_CB_TBN_CLOSETOTRAY, OnSettingsChange)
	ON_CBN_SELCHANGE(IDC_USEDFONT, OnSettingsChange)
	ON_CBN_SELCHANGE(IDC_USEDFONTSIZE, OnSettingsChange)
	ON_BN_CLICKED(IDC_SHOWRATEONTITLE, OnSettingsChange)
	ON_BN_CLICKED(IDC_TBSPEEDMETER, OnSettingsChange)
	ON_BN_CLICKED(IDC_KEEP_SEARCH_HISTORY, OnSettingsChange)
END_MESSAGE_MAP()

BOOL CPPgWindow::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	fontPreviewCombo.m_style = CFontPreviewCombo::NAME_ONLY;
	fontPreviewCombo.Init();

	fontSizeCombo.AddString(_T("8"));
	fontSizeCombo.AddString(_T("10"));
	fontSizeCombo.AddString(_T("12"));
	LoadSettings();
	Localize();

	return TRUE;
}

void CPPgWindow::LoadSettings(void)
{
	m_bStartMin = m_pPrefs->GetStartMinimized();
	m_bMinTray = m_pPrefs->DoMinToTray();
	m_bPromptOnExit = m_pPrefs->IsConfirmExitEnabled();
	m_bPromptOnDisconnect = m_pPrefs->IsConfirmDisconnectEnabled();
	m_bPromptOnFriendDel = m_pPrefs->IsConfirmFriendDelEnabled();
	m_bBringToForeground = m_pPrefs->IsBringToFront();
	m_bCloseToTray = m_pPrefs->GetCloseToTray();
	m_bShowRateOnTitle = m_pPrefs->ShowRatesOnTitle();
	m_bShowSpeedMeterOnToolbar = m_pPrefs->GetShowToolbarSpeedMeter();
	m_bKeepSearchHistory = m_pPrefs->GetKeepSearchHistory();

	int iFontIdx = fontPreviewCombo.FindStringExact(0, m_pPrefs->GetUsedFont());

	if (iFontIdx == LB_ERR)
		iFontIdx = 0;
	fontPreviewCombo.SetCurSel(iFontIdx);
	fontSizeCombo.SetCurSel(0);
	if (m_pPrefs->GetFontSize())
		fontSizeCombo.SetCurSel((int)((m_pPrefs->GetFontSize()-80)/20));

	UpdateData(FALSE);

	SetModified(FALSE);
}

BOOL CPPgWindow::OnApply()
{
	if(m_bModified)
	{
		UpdateData(TRUE);

		byte mintotray_old = m_pPrefs->DoMinToTray();

		m_pPrefs->SetStartMinimized(B2b(m_bStartMin));
		m_pPrefs->SetMinToTray(B2b(m_bMinTray));
		m_pPrefs->SetConfirmExitEnabled(B2b(m_bPromptOnExit));
		m_pPrefs->SetConfirmDisconnectEnabled(B2b(m_bPromptOnDisconnect));
		m_pPrefs->SetConfirmFriendDelEnabled(B2b(m_bPromptOnFriendDel));
		m_pPrefs->SetBringToFront(B2b(m_bBringToForeground));
		m_pPrefs->SetCloseToTray(B2b(m_bCloseToTray));
		m_pPrefs->SetRatesOnTitle(B2b(m_bShowRateOnTitle));
		m_pPrefs->SetShowToolbarSpeedMeter(B2b(m_bShowSpeedMeterOnToolbar));
		if (B2b(m_bKeepSearchHistory) != m_pPrefs->GetKeepSearchHistory())
		{
			m_pPrefs->SetKeepSearchHistory(B2b(m_bKeepSearchHistory));
			if (!m_bKeepSearchHistory)	//reset current history
				g_App.m_pMDlg->m_dlgSearch.KeepHistoryChanged();
		}

		if (mintotray_old != (byte)m_pPrefs->DoMinToTray())
		{
			g_App.m_pMDlg->TraySetMinimizeToTray(m_pPrefs->GetMinTrayPTR());
			g_App.m_pMDlg->TrayMinimizeToTrayChanged();
		}

		CString buffer;
		fontPreviewCombo.GetWindowText(buffer);
		m_pPrefs->SetUsedFont(buffer);
		m_pPrefs->SetFontSize(static_cast<byte>(fontSizeCombo.GetCurSel() * 20 + 80));
		// Font update
		g_App.m_pMDlg->m_fontDefault.DeleteObject();
		g_App.m_pMDlg->m_fontDefault.CreatePointFont(m_pPrefs->GetFontSize(),buffer);
		g_App.m_pMDlg->m_wndServer.InitFont();
		g_App.m_pMDlg->m_wndIRC.UpdateFont();
		g_App.m_pMDlg->m_wndChat.m_ctlChatSelector.UpdateFont();

		g_App.m_pMDlg->m_wndTransfer.m_ctlDownloadList.SetStyle();
		g_App.m_pMDlg->m_ctlToolBar.ShowSpeedMeter(m_pPrefs->GetShowToolbarSpeedMeter());

		SetModified(FALSE);

		if (!g_App.m_pPrefs->ShowRatesOnTitle())
			g_App.m_pMDlg->SetWindowText(CLIENT_NAME_WITH_VER);
	}

	return CPropertyPage::OnApply();
}

void CPPgWindow::Localize(void)
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_STARTMIN, IDS_PREF_STARTMIN },
		{ IDC_MINTRAY, IDS_MINIMIZE_TO_SYSTRAY },
		{ IDC_EXIT, IDS_PW_PROMPT },
		{ IDC_COMFIRM_DISCONNECT, IDS_PW_PROMPT_DISCONNECT },
		{ IDC_COMFIRM_FRIENDDEL, IDS_PW_PROMPT_FRIENDDEL },
		{ IDC_BRINGTOFOREGROUND, IDS_PW_FRONT },
		{ IDC_CB_TBN_CLOSETOTRAY, IDS_PW_TBN_CLOSETOTRAY },
		{ IDC_SHOWRATEONTITLE, IDS_SHOWRATEONTITLE },
		{ IDC_TBSPEEDMETER, IDS_PW_SHOWTOOLBARSPEEDMETER },
		{ IDC_USEDFONT_LBL, IDS_USEDFONT_LBL },
		{ IDC_KEEP_SEARCH_HISTORY, IDS_KEEP_SEARCH_HISTORY }
	};

	if (::IsWindow(m_hWnd))
	{
		CString	strRes;

		for (uint32 i = 0; i < ARRSIZE(s_auResTbl); i++)
		{
			::GetResString(&strRes, static_cast<UINT>(s_auResTbl[i][1]));
			SetDlgItemText(s_auResTbl[i][0], strRes);
		}
	}
}
