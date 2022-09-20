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
#include "langids.h"
#include "emule.h"
#include "SearchDlg.h"
#include "PreferencesDlg.h"
#include "ppggeneral.h"
#include "HttpDownloadDlg.h"
#include "Preferences.h"
#include "emuledlg.h"
#include "StatisticsDlg.h"
#include "ServerWnd.h"
#include "TransferWnd.h"
#include "ChatWnd.h"
#include "SharedFilesWnd.h"
#include "KademliaWnd.h"
#include "IrcWnd.h"
#include "WebServices.h"
#include "StringConversion.h"
#include "Log.h"
#include "DLP.h" //Xman dlp check own usernick if valid
#include "opcodes.h" //Xman default nick

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CPPgGeneral, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgGeneral, CPropertyPage)
	ON_BN_CLICKED(IDC_STARTMIN, OnSettingsChange)
	ON_BN_CLICKED(IDC_STARTWIN, OnSettingsChange)
	ON_EN_CHANGE(IDC_NICK, OnSettingsChange)
	ON_BN_CLICKED(IDC_BEEPER, OnSettingsChange)
	ON_BN_CLICKED(IDC_EXIT, OnSettingsChange)
	ON_BN_CLICKED(IDC_SPLASHON, OnSettingsChange)
	ON_BN_CLICKED(IDC_BRINGTOFOREGROUND, OnSettingsChange)
//>>> WiZaRd::Lang Reduction
//	ON_CBN_SELCHANGE(IDC_LANGS, OnLangChange) 
	ON_BN_CLICKED(IDC_LANGID_PT_BR, OnLangChange)
	ON_BN_CLICKED(IDC_LANGID_FR_FR, OnLangChange)
	ON_BN_CLICKED(IDC_LANGID_EN_US, OnLangChange)
	ON_BN_CLICKED(IDC_LANGID_DE_DE, OnLangChange)
	ON_BN_CLICKED(IDC_LANGID_ES_ES_T, OnLangChange)
	ON_BN_CLICKED(IDC_LANGID_IT_IT, OnLangChange)
//<<< WiZaRd::Lang Reduction
	ON_BN_CLICKED(IDC_ED2KFIX, OnBnClickedEd2kfix)
	ON_BN_CLICKED(IDC_WEBSVEDIT , OnBnClickedEditWebservices)
	ON_BN_CLICKED(IDC_ONLINESIG, OnSettingsChange)
//	ON_BN_CLICKED(IDC_CHECK4UPDATE, OnBnClickedCheck4Update)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()

CPPgGeneral::CPPgGeneral()
: CPropertyPage(CPPgGeneral::IDD)
{
}

CPPgGeneral::~CPPgGeneral()
{
}

void CPPgGeneral::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
//	DDX_Control(pDX, IDC_LANGS, m_language); //>>> WiZaRd::Lang Reduction
}

//>>> WiZaRd::Lang Reduction
struct languagePair
{
	int ID;
	int IDC;
};

const languagePair langPairs[] = 
{
	{LANGID_DE_DE,	IDC_LANGID_DE_DE},
	{LANGID_EN_US,	IDC_LANGID_EN_US},
	{LANGID_ES_ES_T,	IDC_LANGID_ES_ES_T},
	{LANGID_IT_IT,	IDC_LANGID_IT_IT},
	{LANGID_PT_BR,	IDC_LANGID_PT_BR},
	{LANGID_FR_FR,	IDC_LANGID_FR_FR}
};
//<<< WiZaRd::Lang Reduction

void CPPgGeneral::LoadSettings(void)
{
	GetDlgItem(IDC_NICK)->SetWindowText(thePrefs.GetUserNick());

//>>> WiZaRd::Lang Reduction
	m_iLanguage = thePrefs.GetLanguageID();
	CWordArray aLanguageIDs;
	thePrefs.GetLanguages(aLanguageIDs);
	for (int i = 0; i < aLanguageIDs.GetSize(); ++i)
	{
		TCHAR szLang[128];
		int ret = GetLocaleInfo(aLanguageIDs[i], LOCALE_SLANGUAGE, szLang, ARRSIZE(szLang));
		ASSERT(ret != 0);
		int iIDC = -1;
		for(int j = 0; j < ARRSIZE(langPairs); ++j)
			if(langPairs[j].ID == aLanguageIDs[i])
				iIDC = langPairs[j].IDC;
		if(iIDC == -1)
			ASSERT(0);
		else
		{
			GetDlgItem(iIDC)->SetWindowText(szLang);
			CheckDlgButton(iIDC, aLanguageIDs[i] == m_iLanguage ? BST_CHECKED : BST_UNCHECKED);
		}
	}
//	for(int i = 0; i < m_language.GetCount(); ++i)
//		if(m_language.GetItemData(i) == thePrefs.GetLanguageID())
//			m_language.SetCurSel(i);
//<<< WiZaRd::Lang Reduction
	
	if(thePrefs.m_bAutoStart)
		CheckDlgButton(IDC_STARTWIN,1);
	else
		CheckDlgButton(IDC_STARTWIN,0);

	if(thePrefs.startMinimized)
		CheckDlgButton(IDC_STARTMIN,1);
	else
		CheckDlgButton(IDC_STARTMIN,0);

	if (thePrefs.onlineSig)
		CheckDlgButton(IDC_ONLINESIG,1);
	else
		CheckDlgButton(IDC_ONLINESIG,0);
	
	if(thePrefs.beepOnError)
		CheckDlgButton(IDC_BEEPER,1);
	else
		CheckDlgButton(IDC_BEEPER,0);

	if(thePrefs.confirmExit)
		CheckDlgButton(IDC_EXIT,1);
	else
		CheckDlgButton(IDC_EXIT,0);

	if(thePrefs.splashscreen)
		CheckDlgButton(IDC_SPLASHON,1);
	else
		CheckDlgButton(IDC_SPLASHON,0);

	if(thePrefs.bringtoforeground)
		CheckDlgButton(IDC_BRINGTOFOREGROUND,1);
	else
		CheckDlgButton(IDC_BRINGTOFOREGROUND,0);

	if(thePrefs.m_bEnableMiniMule)
		CheckDlgButton(IDC_MINIMULE,1);
	else
		CheckDlgButton(IDC_MINIMULE,0);

}

BOOL CPPgGeneral::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	((CEdit*)GetDlgItem(IDC_NICK))->SetLimitText(thePrefs.GetMaxUserNickLength());

//>>> WiZaRd::Lang Reduction
/*
	CWordArray aLanguageIDs;
	thePrefs.GetLanguages(aLanguageIDs);
	for (int i = 0; i < aLanguageIDs.GetSize(); i++){
		TCHAR szLang[128];
		int ret=GetLocaleInfo(aLanguageIDs[i], LOCALE_SLANGUAGE, szLang, ARRSIZE(szLang));

		if (ret==0 && aLanguageIDs[i]== LANGID_GL_ES )
			_tcscpy(szLang,_T("Galician") );
		else if (ret==0 && aLanguageIDs[i]==LANGID_FR_BR )
			_tcscpy(szLang,_T("Breton (Brezhoneg)") );
		else if (ret==0 && aLanguageIDs[i]==LANGID_MT_MT )
			_tcscpy(szLang,_T("Maltese") );
		else if (ret==0 && aLanguageIDs[i]==LANGID_ES_AS )
			_tcscpy(szLang,_T("Asturian") );
		else if (ret==0 && aLanguageIDs[i]==LANGID_VA_ES )
			_tcscpy(szLang,_T("Valencian") );

		m_language.SetItemData(m_language.AddString(szLang), aLanguageIDs[i]);
	}
*/
//<<< WiZaRd::Lang Reduction

	UpdateEd2kLinkFixCtrl();

	GetDlgItem(IDC_ONLINESIG)->ShowWindow( thePrefs.IsExtControlsEnabled()?SW_SHOW:SW_HIDE );

	// Tux: LiteMule: Remove Version Check: removed code block
	
	LoadSettings();
	Localize();
	// Tux: LiteMule: Remove Version Check

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

#ifdef _DEBUG
void ModifyAllWindowStyles(CWnd* pWnd, DWORD dwRemove, DWORD dwAdd)
{
	CWnd* pWndChild = pWnd->GetWindow(GW_CHILD);
	while (pWndChild)
	{
		ModifyAllWindowStyles(pWndChild, dwRemove, dwAdd);
		pWndChild = pWndChild->GetNextWindow();
	}

	if (pWnd->ModifyStyleEx(dwRemove, dwAdd, SWP_FRAMECHANGED))
	{
		pWnd->Invalidate();
//		pWnd->UpdateWindow();
	}
}
#endif

BOOL CPPgGeneral::OnApply()
{
	CString strNick;
	GetDlgItem(IDC_NICK)->GetWindowText(strNick);
	strNick.Trim();
	if (!IsValidEd2kString(strNick))
		strNick.Empty();
	//Xman default Nick 
	if (strNick.IsEmpty())
	{
		strNick = "[DreaMule]Usuario"; //DEFAULT_NICK;
		GetDlgItem(IDC_NICK)->SetWindowText(strNick);
	}
	//Xman DLP check the nick
	CString strNicktocheck=strNick + _T(" [xxx]");
	if(theApp.dlp->IsDLPavailable() && (theApp.dlp->DLPCheckUsername_Hard(strNicktocheck) || theApp.dlp->DLPCheckUsername_Soft(strNicktocheck)))
	{
		AfxMessageBox(_T("This nick is not allowed"));
		strNick = "[DreaMule]Usuario"; //DEFAULT_NICK;
		GetDlgItem(IDC_NICK)->SetWindowText(strNick);
	}
	//Xman end
	thePrefs.SetUserNick(strNick);

//>>> WiZaRd::Lang Reduction
//	if (m_language.GetCurSel() != CB_ERR)
//<<< WiZaRd::Lang Reduction
	{
//>>> WiZaRd::Lang Reduction
//		WORD wNewLang = (WORD)m_language.GetItemData(m_language.GetCurSel());
		const WORD wNewLang = (WORD)m_iLanguage;
//<<< WiZaRd::Lang Reduction
		if (thePrefs.GetLanguageID() != wNewLang)
		{
			thePrefs.SetLanguageID(wNewLang);
			thePrefs.SetLanguage();

#ifdef _DEBUG
			// Can't yet be switched on-the-fly, too much unresolved issues..
			if (thePrefs.GetRTLWindowsLayout())
			{
				ModifyAllWindowStyles(theApp.emuledlg, WS_EX_LAYOUTRTL | WS_EX_RTLREADING | WS_EX_RIGHT | WS_EX_LEFTSCROLLBAR, 0);
				ModifyAllWindowStyles(theApp.emuledlg->preferenceswnd, WS_EX_LAYOUTRTL | WS_EX_RTLREADING | WS_EX_RIGHT | WS_EX_LEFTSCROLLBAR, 0);
				theApp.DisableRTLWindowsLayout();
				thePrefs.m_bRTLWindowsLayout = false;
			}
#endif
			theApp.emuledlg->preferenceswnd->Localize();
			theApp.emuledlg->statisticswnd->CreateMyTree();
			theApp.emuledlg->statisticswnd->Localize();
			theApp.emuledlg->statisticswnd->ShowStatistics(true);
			theApp.emuledlg->serverwnd->Localize();
			theApp.emuledlg->transferwnd->Localize();
			theApp.emuledlg->transferwnd->UpdateCatTabTitles();
			theApp.emuledlg->searchwnd->Localize();
			theApp.emuledlg->sharedfileswnd->Localize();
			theApp.emuledlg->chatwnd->Localize();
			theApp.emuledlg->Localize();
			theApp.emuledlg->ircwnd->Localize();
			theApp.emuledlg->kademliawnd->Localize();
		}
	}

	thePrefs.startMinimized = IsDlgButtonChecked(IDC_STARTMIN)!=0;
	thePrefs.m_bAutoStart = IsDlgButtonChecked(IDC_STARTWIN)!=0;
	if( thePrefs.m_bAutoStart )
		AddAutoStart();
	else
		RemAutoStart();
	thePrefs.beepOnError = IsDlgButtonChecked(IDC_BEEPER)!=0;
	thePrefs.confirmExit = IsDlgButtonChecked(IDC_EXIT)!=0;
	thePrefs.splashscreen = IsDlgButtonChecked(IDC_SPLASHON)!=0;
	thePrefs.bringtoforeground = IsDlgButtonChecked(IDC_BRINGTOFOREGROUND)!=0;
	// Tux: LiteMule: Remove Version Check
	thePrefs.onlineSig = IsDlgButtonChecked(IDC_ONLINESIG)!=0;
	// Tux: LiteMule: Remove Version Check
	thePrefs.m_bEnableMiniMule = IsDlgButtonChecked(IDC_MINIMULE) != 0;


	theApp.emuledlg->transferwnd->downloadlistctrl.SetStyle();
	LoadSettings();

	SetModified(FALSE);
	return CPropertyPage::OnApply();
}

void CPPgGeneral::UpdateEd2kLinkFixCtrl()
{
	GetDlgItem(IDC_ED2KFIX)->EnableWindow(HaveEd2kRegAccess() && Ask4RegFix(true));
}

BOOL CPPgGeneral::OnSetActive()
{
	UpdateEd2kLinkFixCtrl();
	return __super::OnSetActive();
}

void CPPgGeneral::OnBnClickedEd2kfix()
{
	Ask4RegFix(false, false, true);
	GetDlgItem(IDC_ED2KFIX)->EnableWindow(Ask4RegFix(true));
}

void CPPgGeneral::Localize(void)
{
	if(m_hWnd)
	{
		SetWindowText(GetResString(IDS_PW_GENERAL));
		GetDlgItem(IDC_NICK_FRM)->SetWindowText(GetResString(IDS_QL_USERNAME));
		GetDlgItem(IDC_LANG_FRM)->SetWindowText(GetResString(IDS_PW_LANG));
		GetDlgItem(IDC_MISC_FRM)->SetWindowText(GetResString(IDS_PW_MISC));
		GetDlgItem(IDC_BEEPER)->SetWindowText(GetResString(IDS_PW_BEEP));
		GetDlgItem(IDC_EXIT)->SetWindowText(GetResString(IDS_PW_PROMPT));
		GetDlgItem(IDC_SPLASHON)->SetWindowText(GetResString(IDS_PW_SPLASH));
		GetDlgItem(IDC_BRINGTOFOREGROUND)->SetWindowText(GetResString(IDS_PW_FRONT));
		GetDlgItem(IDC_ONLINESIG)->SetWindowText(GetResString(IDS_PREF_ONLINESIG));	
		GetDlgItem(IDC_STARTMIN)->SetWindowText(GetResString(IDS_PREF_STARTMIN));	
		GetDlgItem(IDC_WEBSVEDIT)->SetWindowText(GetResString(IDS_WEBSVEDIT));
		GetDlgItem(IDC_ED2KFIX)->SetWindowText(GetResString(IDS_ED2KLINKFIX));
		// Tux: LiteMule: Remove Version Check
		GetDlgItem(IDC_STARTUP)->SetWindowText(GetResString(IDS_STARTUP));
		GetDlgItem(IDC_STARTWIN)->SetWindowText(GetResString(IDS_STARTWITHWINDOWS));
		GetDlgItem(IDC_MINIMULE)->SetWindowText(GetResString(IDS_ENABLEMINIMULE));
		//Xman versions check
//		GetDlgItem(IDC_CHECK4UPDATEMOD)->SetWindowText(GetResString(IDS_CHECK4UPDATEMOD));
	}
}

void CPPgGeneral::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	SetModified(TRUE);

	// Tux: LiteMule: Remove Version Check: removed code block

	UpdateData(false); 
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPPgGeneral::OnBnClickedEditWebservices()
{
	theWebServices.Edit();
}

void CPPgGeneral::OnLangChange()
{
#define MIRRORS_URL	_T("http://www.dreamule.org/dlllinguagens/")
//>>> WiZaRd::Lang Reduction

//	WORD byNewLang = (WORD)m_language.GetItemData(m_language.GetCurSel());
	int iNewLang = -1;
	int iOldIDC = -1;
	for(int j = 0; j < ARRSIZE(langPairs); ++j)
	{
		if(IsDlgButtonChecked(langPairs[j].IDC))
				iNewLang = langPairs[j].ID;
		else if(m_iLanguage == langPairs[j].ID)
				iOldIDC = langPairs[j].IDC;
		}
	if(iNewLang == -1)
		return;	
	const WORD byNewLang = (WORD)iNewLang;
//<<< WiZaRd::Lang Reduction
	if (thePrefs.GetLanguageID() != byNewLang)
	{
		if	(!thePrefs.IsLanguageSupported(byNewLang, false))
		{
			if (AfxMessageBox(GetResString(IDS_ASKDOWNLOADLANGCAP) + _T("\r\n\r\n") + GetResString(IDS_ASKDOWNLOADLANG), MB_ICONQUESTION | MB_YESNO) == IDYES)
			{
				// download file
				// create url, use random mirror for load balancing
				UINT nRand = (rand()/(RAND_MAX/3))+1;
				CString strUrl;
				//strUrl.Format(MIRRORS_URL, nRand, CemuleApp::m_nVersionMjr, CemuleApp::m_nVersionMin, CemuleApp::m_nVersionUpd, CemuleApp::m_nVersionBld);
				strUrl += thePrefs.GetLangDLLNameByID(byNewLang);
				// safeto
				CString strFilename = thePrefs.GetMuleDirectory(EMULE_ADDLANGDIR, true);
				strFilename.Append(thePrefs.GetLangDLLNameByID(byNewLang));
				// start
				CHttpDownloadDlg dlgDownload;
				dlgDownload.m_strTitle = GetResString(IDS_DOWNLOAD_LANGFILE);
				dlgDownload.m_sURLToDownload = strUrl;
				dlgDownload.m_sFileToDownloadInto = strFilename;
				if (dlgDownload.DoModal() == IDOK && thePrefs.IsLanguageSupported(byNewLang, true))
				{
					// everything ok, new language downloaded and working
					m_iLanguage = iNewLang; //>>> WiZaRd::Lang Reduction
					OnSettingsChange();
					return;
				}
				CString strErr;
				strErr.Format(GetResString(IDS_ERR_FAILEDDOWNLOADLANG), strUrl);
				LogError(LOG_STATUSBAR, _T("%s"), strErr);
				AfxMessageBox(strErr, MB_ICONERROR | MB_OK);
			}
//>>> WiZaRd::Lang Reduction
			// undo change selection
			for(int j = 0; j < ARRSIZE(langPairs); ++j)
				CheckDlgButton(langPairs[j].IDC, langPairs[j].IDC == iOldIDC ? BST_CHECKED : BST_UNCHECKED);
//			for(int i = 0; i < m_language.GetCount(); i++)
//				if(m_language.GetItemData(i) == thePrefs.GetLanguageID())
//					m_language.SetCurSel(i);
//<<< WiZaRd::Lang Reduction
		}
		else
		{
			m_iLanguage = iNewLang; //>>> WiZaRd::Lang Reduction
			OnSettingsChange();
		}
	}
}

// Tux: LiteMule: Remove Version Check: removed code block
// Tux: LiteMule: Remove Help: removed code block