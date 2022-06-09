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
#include "langids.h"
#include "emule.h"
//#include "SearchDlg.h"
#include "PreferencesDlg.h"
#include "ppggeneral.h"
#include "HttpDownloadDlg.h"
#include "Preferences.h"
#include "emuledlg.h"
//#include "StatisticsDlg.h"
//#include "ServerWnd.h"
#include "TransferDlg.h"
//#include "ChatWnd.h"
//#include "SharedFilesWnd.h"
//#include "KademliaWnd.h"
#include "WebServices.h"
#include "StringConversion.h"
#include "Log.h"
#include "opcodes.h" //Xman default nick
#include "IP2Country.h"// X: [AL] - [Additional Localize]
#include "Addons/Modname/Modname.h"

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
	ON_BN_CLICKED(IDC_EXIT, OnSettingsChange)
	ON_BN_CLICKED(IDC_SPLASHON, OnSettingsChange)
	ON_BN_CLICKED(IDC_BRINGTOFOREGROUND, OnSettingsChange)
	ON_CBN_SELCHANGE(IDC_LANGS, OnLangChange)
	ON_BN_CLICKED(IDC_ED2KFIX, OnBnClickedEd2kfix)
	ON_BN_CLICKED(IDC_WEBSVEDIT , OnBnClickedEditWebservices)
	ON_BN_CLICKED(IDC_ONLINESIG, OnSettingsChange)
	ON_BN_CLICKED(IDC_MINIMULE, OnSettingsChange)
	ON_BN_CLICKED(IDC_PREVENTSTANDBY, OnSettingsChange)
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
	DDX_Control(pDX, IDC_LANGS, m_language);
}

void CPPgGeneral::LoadSettings(void)
{
	SetDlgItemText(IDC_NICK,thePrefs.GetUserNick());

	for(int i = 0; i < m_language.GetCount(); i++)
		if(m_language.GetItemData(i) == thePrefs.GetLanguageID())
			m_language.SetCurSel(i);
	
	CheckDlgButton(IDC_STARTWIN,thePrefs.m_bAutoStart);
	CheckDlgButton(IDC_STARTMIN,thePrefs.startMinimized);
	CheckDlgButton(IDC_ONLINESIG,thePrefs.onlineSig);
	CheckDlgButton(IDC_EXIT,thePrefs.confirmExit);
	CheckDlgButton(IDC_SPLASHON,thePrefs.splashscreen);
	CheckDlgButton(IDC_BRINGTOFOREGROUND,thePrefs.bringtoforeground);
	CheckDlgButton(IDC_MINIMULE,thePrefs.m_bEnableMiniMule);

	CheckDlgButton(IDC_PREVENTSTANDBY,thePrefs.GetPreventStandby());
}

BOOL CPPgGeneral::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	((CEdit*)GetDlgItem(IDC_NICK))->SetLimitText(MAXUSERNICKLENGTH);

	CAtlArray<WORD> aLanguageIDs;
	thePrefs.GetLanguages(aLanguageIDs);
	for (size_t i = 0; i < aLanguageIDs.GetCount(); i++){
		TCHAR szLang[128];
		int ret=GetLocaleInfo(aLanguageIDs[i], LOCALE_SLANGUAGE, szLang, ARRSIZE(szLang));

		if (ret==0)
			switch(aLanguageIDs[i]) {
				case LANGID_UG_CN:
					_tcscpy(szLang,_T("Uyghur") );
					break;
				case LANGID_GL_ES:
					_tcscpy(szLang,_T("Galician") );
					break;
				case LANGID_FR_BR:
					_tcscpy(szLang,_T("Breton (Brezhoneg)") );
					break;
				case LANGID_MT_MT:
					_tcscpy(szLang,_T("Maltese") );
					break;
				case LANGID_ES_AS:
					_tcscpy(szLang,_T("Asturian") );
					break;
				case LANGID_VA_ES:
					_tcscpy(szLang,_T("Valencian") );
					break;
				case LANGID_VA_ES_RACV:
					_tcscpy(szLang, _T("Valencian (RACV)"));
					break;
				default:
					ASSERT(0);
					_tcscpy(szLang,_T("?(unknown language)?") );
					break;
			}

		m_language.SetItemData(m_language.AddString(szLang), aLanguageIDs[i]);
	}

	UpdateEd2kLinkFixCtrl();

	LoadSettings();
	Localize();
	m_bModified = false; // X: [CI] - [Code Improvement] Apply if modified

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

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

BOOL CPPgGeneral::OnApply()
{
	if(m_bModified){ // X: [CI] - [Code Improvement] Apply if modified
		CString strNick;
		GetDlgItemText(IDC_NICK,strNick);
		strNick.Trim();
		if (!IsValidEd2kString(strNick))
			strNick.Empty();
		//Xman default Nick 
		if (strNick.IsEmpty())
		{
			strNick = MOD_USERNICK; //DEFAULT_NICK
			SetDlgItemText(IDC_NICK,strNick);
		}
		thePrefs.SetUserNick(strNick);

		if (m_language.GetCurSel() != CB_ERR)
		{
			WORD wNewLang = (WORD)m_language.GetItemData(m_language.GetCurSel());
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
				CIP2Country::defaultCountry.LongCountryName = GetResString(IDS_IP2COUNTRY_NALONG);// X: [AL] - [Additional Localize]
				theApp.emuledlg->Localize();
			}
		}

		thePrefs.startMinimized = IsDlgButtonChecked(IDC_STARTMIN)!=0;
		thePrefs.m_bAutoStart = IsDlgButtonChecked(IDC_STARTWIN)!=0;
		if( thePrefs.m_bAutoStart )
			AddAutoStart();
		else
			RemAutoStart();
		thePrefs.confirmExit = IsDlgButtonChecked(IDC_EXIT)!=0;
		thePrefs.splashscreen = IsDlgButtonChecked(IDC_SPLASHON)!=0;
		thePrefs.bringtoforeground = IsDlgButtonChecked(IDC_BRINGTOFOREGROUND)!=0;
		thePrefs.onlineSig = IsDlgButtonChecked(IDC_ONLINESIG)!=0;
		thePrefs.m_bEnableMiniMule = IsDlgButtonChecked(IDC_MINIMULE) != 0;
		thePrefs.m_bPreventStandby = IsDlgButtonChecked(IDC_PREVENTSTANDBY) != 0;

		LoadSettings();

		SetModified(FALSE);
		m_bModified = false; // X: [CI] - [Code Improvement] Apply if modified
	}
	return CPropertyPage::OnApply();
}

void CPPgGeneral::UpdateEd2kLinkFixCtrl()
{
	GetDlgItem(IDC_ED2KFIX)->EnableWindow(Ask4RegFix(true, false, true));
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
		SetDlgItemText(IDC_NICK_FRM,GetResString(IDS_QL_USERNAME));
		SetDlgItemText(IDC_LANG_FRM,GetResString(IDS_PW_LANG));
		SetDlgItemText(IDC_MISC_FRM,GetResString(IDS_PW_MISC));
		SetDlgItemText(IDC_EXIT,GetResString(IDS_PW_PROMPT));
		SetDlgItemText(IDC_SPLASHON,GetResString(IDS_PW_SPLASH));
		SetDlgItemText(IDC_BRINGTOFOREGROUND,GetResString(IDS_PW_FRONT));
		SetDlgItemText(IDC_ONLINESIG,GetResString(IDS_PREF_ONLINESIG));	
		SetDlgItemText(IDC_STARTMIN,GetResString(IDS_PREF_STARTMIN));	
		SetDlgItemText(IDC_WEBSVEDIT,GetResString(IDS_WEBSVEDIT));
		SetDlgItemText(IDC_ED2KFIX,GetResString(IDS_ED2KLINKFIX));
		SetDlgItemText(IDC_STARTUP,GetResString(IDS_STARTUP));
		SetDlgItemText(IDC_STARTWIN,GetResString(IDS_STARTWITHWINDOWS));
		SetDlgItemText(IDC_MINIMULE,GetResString(IDS_ENABLEMINIMULE));
		SetDlgItemText(IDC_PREVENTSTANDBY,GetResString(IDS_PREVENTSTANDBY));
	}
}

void CPPgGeneral::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified
	UpdateData(false); 
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPPgGeneral::OnBnClickedEditWebservices()
{
	theWebServices.Edit();
}

void CPPgGeneral::OnLangChange()
{
//#define MIRRORS_URL	_T("http://langmirror%i.emule-project.org/lang/%i%i%i%i/")

	WORD byNewLang = (WORD)m_language.GetItemData(m_language.GetCurSel());
	if (thePrefs.GetLanguageID() != byNewLang){
		if	(!thePrefs.IsLanguageSupported(byNewLang, false)){
			/*if (AfxMessageBox(GetResString(IDS_ASKDOWNLOADLANGCAP) + _T("\r\n\r\n") + GetResString(IDS_ASKDOWNLOADLANG), MB_ICONQUESTION | MB_YESNO) == IDYES){
				// download file
				// create url, use random mirror for load balancing
				UINT nRand = (rand()/(RAND_MAX/3))+1;
				CString strUrl;
				strUrl.Format(MIRRORS_URL, nRand, CemuleApp::m_nVersionMjr, CemuleApp::m_nVersionMin, CemuleApp::m_nVersionUpd, CemuleApp::m_nVersionBld);
				strUrl += thePrefs.GetLangNameByID(byNewLang) + _T(".dll");
				// safeto
				CString strFilename = thePrefs.GetMuleDirectory(EMULE_ADDLANGDIR, true);
				strFilename.Append(thePrefs.GetLangNameByID(byNewLang) + _T(".dll"));
				// start
				CHttpDownloadDlg dlgDownload;
				dlgDownload.m_strTitle = GetResString(IDS_DOWNLOAD_LANGFILE);
				dlgDownload.m_sURLToDownload = strUrl;
				dlgDownload.m_sFileToDownloadInto = strFilename;
				if (dlgDownload.DoModal() == IDOK && thePrefs.IsLanguageSupported(byNewLang, true))
				{
					// everything ok, new language downloaded and working
					OnSettingsChange();
					return;
				}
				CString strErr;
				strErr.Format(GetResString(IDS_ERR_FAILEDDOWNLOADLANG), strUrl);
				LogError(LOG_STATUSBAR, _T("%s"), strErr);
				AfxMessageBox(strErr, MB_ICONERROR | MB_OK);
			}*/
			// undo change selection
			for(int i = 0; i < m_language.GetCount(); i++)
				if(m_language.GetItemData(i) == thePrefs.GetLanguageID())
					m_language.SetCurSel(i);
		}
		else
			OnSettingsChange();
	}
}
