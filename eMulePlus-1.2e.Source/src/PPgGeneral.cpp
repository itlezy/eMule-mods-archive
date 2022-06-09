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
#include "PPgGeneral.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CPPgGeneral, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgGeneral, CPropertyPage)
	ON_EN_CHANGE(IDC_NICK, OnSettingsChange)
	ON_EN_CHANGE(IDC_TOOLTIPDELAY, OnSettingsChange)
	ON_CBN_SELCHANGE(IDC_LANGS, OnSettingsChange)
	ON_BN_CLICKED(IDC_BEEPER, OnSettingsChange)
	ON_BN_CLICKED(IDC_SPLASHON, OnSettingsChange)
	ON_BN_CLICKED(IDC_NOTIFY, OnSettingsChange)
	ON_BN_CLICKED(IDC_ONLINESIG, OnSettingsChange)
	ON_BN_CLICKED(IDC_MULTIPLE, OnSettingsChange)
	ON_BN_CLICKED(IDC_DC_CLIENT, OnSettingsChange)
	ON_BN_CLICKED(IDC_MP_STANDARD, OnSettingsChange)
	ON_BN_CLICKED(IDC_MP_HIGH, OnSettingsChange)
	ON_BN_CLICKED(IDC_WEBSVEDIT , OnBnClickedEditWebservices)
	ON_BN_CLICKED(IDC_ED2KFIX, OnBnClickedEd2kfix)
	ON_BN_CLICKED(IDC_AUTOTAKEED2KLINKS, OnSettingsChange)
	ON_BN_CLICKED(IDC_FAKE_CHECKUPDATEONSTART, OnSettingsChange)
	ON_BN_CLICKED(IDC_FAKE_UPDATE, OnBnClickedFakeUpdate)
	ON_BN_CLICKED(IDC_WATCH_CLIPBOARD, OnSettingsChange)
	ON_BN_CLICKED(IDC_LOCALIZEDLINKS, OnSettingsChange)
	ON_EN_CHANGE(IDC_FAKE_LIST_URL, OnSettingsChange)
	ON_WM_HSCROLL()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_3DDEPTH, On3DDepth)
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPPgGeneral::CPPgGeneral()
	: CPropertyPage(CPPgGeneral::IDD)
	, m_bBeepOnErrors(FALSE)
	, m_bShowSplashscreen(FALSE)
	, m_bAllowMultipleInstances(FALSE)
	, m_bOnlineSignature(FALSE)
	, m_bDoubleClickClientDetails(FALSE)
	, m_bAutoTakeEd2kLinks(FALSE)
	, m_iMainProcess(0)
	, m_bUpdateFakeList(FALSE)
	, m_bWatchClipboard(FALSE)
	, m_bLocalizedLinks(FALSE)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPPgGeneral::~CPPgGeneral()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPPgGeneral::DoDataExchange(CDataExchange *pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LANGS, m_LanguageCombo);
	DDX_Control(pDX, IDC_PREVIEW, m_3DPreview);
	DDX_Control(pDX, IDC_3DDEPTH, m_3DSlider);
	DDX_Check(pDX, IDC_AUTOTAKEED2KLINKS, m_bAutoTakeEd2kLinks);
	DDX_Check(pDX, IDC_BEEPER, m_bBeepOnErrors);
	DDX_Check(pDX, IDC_SPLASHON, m_bShowSplashscreen);
	DDX_Check(pDX, IDC_MULTIPLE, m_bAllowMultipleInstances);
	DDX_Check(pDX, IDC_ONLINESIG, m_bOnlineSignature);
	DDX_Check(pDX, IDC_DC_CLIENT, m_bDoubleClickClientDetails);
	DDX_Check(pDX, IDC_FAKE_CHECKUPDATEONSTART, m_bUpdateFakeList);
	DDX_Check(pDX, IDC_WATCH_CLIPBOARD, m_bWatchClipboard);
	DDX_Check(pDX, IDC_LOCALIZEDLINKS, m_bLocalizedLinks);
	DDX_Radio(pDX, IDC_MP_STANDARD, m_iMainProcess);
	DDX_Text(pDX, IDC_TOOLTIPDELAY, m_strTooltipDelay);
	DDX_Text(pDX, IDC_NICK, m_strUserNick);
	DDX_Text(pDX, IDC_FAKE_LIST_URL, m_strUpdateFakeListURL);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CPPgGeneral::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	(reinterpret_cast<CEdit*>(GetDlgItem(IDC_NICK)))->SetLimitText(MAX_NICK_LENGTH);

	GetDlgItem(IDC_ED2KFIX)->EnableWindow(!CheckIsRegistrySet());
	LoadSettings();
	DrawPreview();
	Localize();

	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPPgGeneral::LoadLanguagesCombo(void)
{
	CString	strLang;
	const StructLanguage *pLangs = g_aLangs;

	do
	{
		if (PRIMARYLANGID(pLangs->uLangs) == LANG_SPECIAL)
		{
		//	Unique languages not supported by OS (language names are left untranslated)
			switch (SUBLANGID(pLangs->uLangs))
			{
				case SUBLNG_EXTREMADURAN:
					strLang = _T("Extremaduran");
					break;
			}
		}
		else
		{
			GetLocaleInfo(pLangs->uLangs, (LOCALE_SLANGUAGE | LOCALE_USE_CP_ACP), strLang.GetBuffer(128), 128);
			strLang.ReleaseBuffer();

		//	No sublangs in brackets for the following languages
			switch(PRIMARYLANGID(pLangs->uLangs))
			{
				case LANG_ENGLISH:
				case LANG_GERMAN:
				case LANG_FRENCH:
				case LANG_SPANISH:
				case LANG_DUTCH:
				case LANG_ITALIAN:
				case LANG_MALAY:
				case LANG_NORWEGIAN:
				case LANG_SERBIAN:	//LANG_CROATIAN == LANG_SERBIAN
				{
					int iBracket = strLang.Find(_T(" ("));

				//	In some localized systems " - " could be used...
					if ((iBracket > 0) || ((iBracket = strLang.Find(_T(" -"))) > 0))
						strLang.Truncate(iBracket);
					break;
				}
			}
		}
	//	Current position in the automatically sorted list
		int		iRc = m_LanguageCombo.AddString(strLang);

		if (iRc >= 0)
		{
			m_LanguageCombo.SetItemData(iRc, pLangs->uLangs);
			if (pLangs->uLangs == m_pPrefs->GetLanguageID())
			{
			//	Select default language
				m_LanguageCombo.SetCurSel(iRc);
			}
		}
		pLangs++;
	} while (pLangs->uLangs != 0);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPPgGeneral::LoadSettings(void)
{
	m_strUserNick = m_pPrefs->GetUserNick();

	LoadLanguagesCombo();

	m_bBeepOnErrors = m_pPrefs->IsErrorBeepEnabled();
	m_bShowSplashscreen = m_pPrefs->UseSplashScreen();
	m_bAllowMultipleInstances = m_pPrefs->GetMultiple();
	m_bOnlineSignature = m_pPrefs->IsOnlineSignatureEnabled();
	m_bDoubleClickClientDetails = m_pPrefs->GetDetailsOnClick();
	m_bAutoTakeEd2kLinks = m_pPrefs->AutoTakeED2KLinks();
	m_iMainProcess = (m_pPrefs->GetMainProcessPriority()) ? 1 : 0;
	m_bUpdateFakeList = m_pPrefs->IsUpdateFakeStartupEnabled();
	m_strUpdateFakeListURL = m_pPrefs->GetFakeListURL();
	m_bWatchClipboard = m_pPrefs->IsWatchClipboard4ED2KLinks();
	m_bLocalizedLinks = m_pPrefs->GetExportLocalizedLinks();
	m_strTooltipDelay.Format(_T("%u"), m_pPrefs->GetToolTipDelay());

	m_3DSlider.SetRange(-5, 5, true);
	int tmp = m_pPrefs->Get3DDepth();
	if (tmp > 5)
		tmp -= 256;
	m_3DSlider.SetPos(tmp);

	UpdateData(FALSE);
	SetModified(FALSE);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CPPgGeneral::OnApply()
{
	if(m_bModified)
	{
		UpdateData(TRUE);

		if (!m_strUserNick.IsEmpty())
			m_pPrefs->SetUserNick(m_strUserNick);

		int iTmp = m_LanguageCombo.GetCurSel();

		if (iTmp != CB_ERR)
		{
			uint16	uNewLang = static_cast<uint16>(m_LanguageCombo.GetItemData(iTmp));

			if (m_pPrefs->GetLanguageID() != uNewLang)
			{
				m_pPrefs->SetLanguageID(uNewLang);
				g_App.m_pPrefs->InitThreadLocale();

				g_App.m_pMDlg->m_dlgPreferences.Localize();
				g_App.m_pMDlg->m_dlgStatistics.Localize();
				g_App.m_pMDlg->m_wndServer.Localize();
				g_App.m_pMDlg->m_wndTransfer.Localize();
				g_App.m_pMDlg->m_dlgSearch.Localize();
				g_App.m_pMDlg->m_wndSharedFiles.Localize();
				g_App.m_pMDlg->m_wndChat.Localize();
				g_App.m_pMDlg->Localize();
				g_App.m_pMDlg->m_wndIRC.Localize();
			}
		}

		m_pPrefs->SetErrorBeepEnabled(B2b(m_bBeepOnErrors));
		m_pPrefs->SetUseSplashScreen(B2b(m_bShowSplashscreen));
		m_pPrefs->SetOnlineSignatureEnabled(B2b(m_bOnlineSignature));
		m_pPrefs->SetDetailsOnClick(B2b(m_bDoubleClickClientDetails));
		m_pPrefs->SetMultiple(B2b(m_bAllowMultipleInstances));

		byte	byteTmp = static_cast<byte>(m_3DSlider.GetPos());

		if (byteTmp != m_pPrefs->Get3DDepth())
		{
			m_pPrefs->Set3DDepth(byteTmp);
		//	Status bar format is changed, update the download list
			g_App.m_pMDlg->m_wndTransfer.m_ctlDownloadList.Invalidate();
		//	Update upload lists only if upload progress bars are enabled
			if (m_pPrefs->IsUploadPartsEnabled())
			{
				g_App.m_pMDlg->m_wndTransfer.m_ctlUploadList.Invalidate();
				g_App.m_pMDlg->m_wndTransfer.m_ctlQueueList.Invalidate();
			}
		}

		m_pPrefs->SetAutoTakeED2KLinks(B2b(m_bAutoTakeEd2kLinks));
		m_pPrefs->SetUpdateFakeStartup(B2b(m_bUpdateFakeList));
		m_pPrefs->SetFakeListURL(m_strUpdateFakeListURL.Trim());
		m_pPrefs->SetWatchClipboard4ED2KLinks(B2b(m_bWatchClipboard));
		m_pPrefs->SetExportLocalizedLinks(B2b(m_bLocalizedLinks));

		if ((iTmp = _tstoi(m_strTooltipDelay)) > 60)
			iTmp = 60;
		m_pPrefs->SetToolTipDelay(iTmp);

		iTmp *= 1000;
		g_App.m_pMDlg->m_wndTransfer.m_ttip.SetDelayTime(TTDT_INITIAL, iTmp);
		g_App.m_pMDlg->m_wndServer.m_ttip.SetDelayTime(TTDT_INITIAL, iTmp);
		g_App.m_pMDlg->m_ttip.SetDelayTime(TTDT_INITIAL, iTmp);
		g_App.m_pMDlg->m_dlgSearch.m_ttip.SetDelayTime(TTDT_INITIAL, iTmp);
		g_App.m_pMDlg->m_wndSharedFiles.m_ttip.SetDelayTime(TTDT_INITIAL, iTmp);
		g_App.m_pMDlg->m_wndChat.m_ttip.SetDelayTime(TTDT_INITIAL, iTmp);

		m_pPrefs->SetMainProcessPriority((m_iMainProcess == 0) ? 0 : 1);

		SetModified(FALSE);
	}

	return CPropertyPage::OnApply();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPPgGeneral::Localize(void)
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_NICK_FRM, IDS_NICK },
		{ IDC_LANG_FRM, IDS_PW_LANG },
		{ IDC_MISC_FRM, IDS_PW_MISC },
		{ IDC_BEEPER, IDS_PW_BEEP },
		{ IDC_SPLASHON, IDS_PW_SPLASH },
		{ IDC_TOOLTIPDELAY_LBL, IDS_PW_TOOL },
		{ IDC_ONLINESIG, IDS_PREF_ONLINESIG },
		{ IDC_MULTIPLE, IDS_MULTIPLE },
		{ IDC_DC_CLIENT, IDS_DC_CLIENT },
		{ IDC_3DDEP, IDS_3DDEP },
		{ IDC_MAINPROCESS, IDS_MAINPROCESS_LBL },
		{ IDC_MP_STANDARD, IDS_HASHPRIO_STANDARD },
		{ IDC_MP_HIGH, IDS_PRIOHIGH },
		{ IDC_WEBSVEDIT, IDS_WEBSVEDIT },
		{ IDC_ED2KFIX, IDS_ED2KLINKFIX },
		{ IDC_AUTOTAKEED2KLINKS, IDS_AUTOTAKEED2KLINKS },
		{ IDC_FAKE_UPDATE, IDS_SV_UPDATE },
		{ IDC_FAKE_CHECKUPDATEONSTART, IDS_FAKE_CHECKUPDATEONSTART },
		{ IDC_FAKE_LIST_URL_LBL, IDS_FAKE_LIST_URL_LBL },
		{ IDC_WATCH_CLIPBOARD, IDS_WATCH_CLIPBOARD },
		{ IDC_LOCALIZEDLINKS, IDS_EXPORTLOCALIZEDLINKS }
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
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPPgGeneral::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SetModified(TRUE);
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPPgGeneral::DrawPreview()
{
	m_3DPreview.SetSliderPos(m_3DSlider.GetPos());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPPgGeneral::On3DDepth(NMHDR *pNMHDR, LRESULT *pResult)
{
	NOPRM(pNMHDR);
	DrawPreview();
	*pResult = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPPgGeneral::OnBnClickedEditWebservices()
{
	CString	strPath;

	strPath.Format(_T("\"%swebservices.dat\""), g_App.m_pPrefs->GetConfigDir());
	ShellExecute(NULL, _T("open"), g_App.m_pPrefs->GetTxtEditor(), strPath, NULL, SW_SHOW);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPPgGeneral::OnBnClickedEd2kfix()
{
	Ask4RegFix();
	GetDlgItem(IDC_ED2KFIX)->EnableWindow(!CheckIsRegistrySet());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPPgGeneral::OnBnClickedFakeUpdate()
{
	UpdateData(true);

	m_pPrefs->SetFakeListURL(m_strUpdateFakeListURL);
	g_App.m_pFakeCheck->UpdateFakeList();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
