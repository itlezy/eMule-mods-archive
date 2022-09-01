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
#include "SearchDlg.h"
#include "PreferencesDlg.h"
#include "ppggeneral.h"
#include "HttpDownloadDlg.h"
#include "Preferences.h"
#include "emuledlg.h"
#include "StatisticsDlg.h"
#include "ServerWnd.h"
#include "TransferDlg.h"
#include "ChatWnd.h"
#include "SharedFilesWnd.h"
#include "KademliaWnd.h"
#include "IrcWnd.h"
#include "WebServices.h"
#include "HelpIDs.h"
#include "StringConversion.h"
#include "Log.h"

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
	ON_BN_CLICKED(IDC_STARTUPSOUNDON, OnSettingsChange)//Commander - Added: Enable/Disable Startupsound
	ON_BN_CLICKED(IDC_BRINGTOFOREGROUND, OnSettingsChange)
	ON_CBN_SELCHANGE(IDC_LANGS, OnLangChange)
	ON_BN_CLICKED(IDC_ED2KFIX, OnBnClickedEd2kfix)
	ON_BN_CLICKED(IDC_WEBSVEDIT , OnBnClickedEditWebservices)
	ON_BN_CLICKED(IDC_ONLINESIG, OnSettingsChange)
	ON_BN_CLICKED(IDC_CHECK4UPDATE, OnBnClickedCheck4Update)
	ON_BN_CLICKED(IDC_MINIMULE, OnSettingsChange)
	ON_BN_CLICKED(IDC_PREVENTSTANDBY, OnSettingsChange)
	//Commander - Added: Invisible Mode [TPT] - Start
	ON_CBN_SELCHANGE(IDC_INVISIBLE_MODE_SELECT_COMBO, OnSettingsChange)
	ON_CBN_SELCHANGE(IDC_INVISIBLE_MODE_KEY_COMBO, OnCbnSelchangeKeymodcombo)
	ON_BN_CLICKED(IDC_INVISIBLE_MODE, OnBoxesChange)
	//Commander - Added: Invisible Mode [TPT] - End
	ON_WM_HSCROLL()
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

CPPgGeneral::CPPgGeneral()
// MORPH START leuk_he tooltipped
/*
: CPropertyPage(CPPgGeneral::IDD)
*/
: CPPgtooltipped(CPPgGeneral::IDD)
// MORRPH END leuk_he tooltipped
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
	//Commander - Added: Invisible Mode [TPT] - Start
	m_iActualKeyModifier = thePrefs.GetInvisibleModeHKKeyModifier();
	((CComboBox*)GetDlgItem(IDC_INVISIBLE_MODE_SELECT_COMBO))->SelectString(-1, CString(thePrefs.GetInvisibleModeHKKey()));
	if (!thePrefs.GetInvisibleMode()){
		GetDlgItem(IDC_INVISIBLE_MODE_SELECT_STATIC)->EnableWindow(false);
		GetDlgItem(IDC_INVISIBLE_MODE_MODIFIER_STATIC)->EnableWindow(false);
		GetDlgItem(IDC_INVISIBLE_MODE_KEY_STATIC)->EnableWindow(false);
		GetDlgItem(IDC_INVISIBLE_MODE_SYMBOL_STATIC)->EnableWindow(false);
		GetDlgItem(IDC_INVISIBLE_MODE_KEY_COMBO)->EnableWindow(false);
		GetDlgItem(IDC_INVISIBLE_MODE_SELECT_COMBO)->EnableWindow(false);
		CheckDlgButton(IDC_INVISIBLE_MODE, 0);
	} else 
		CheckDlgButton(IDC_INVISIBLE_MODE, 1);
	//Commander - Added: Invisible Mode [TPT] - End
	GetDlgItem(IDC_NICK)->SetWindowText(thePrefs.GetUserNick());

	for(int i = 0; i < m_language.GetCount(); i++)
		if(m_language.GetItemData(i) == thePrefs.GetLanguageID())
			m_language.SetCurSel(i);
	
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

	if(thePrefs.confirmExit)
		CheckDlgButton(IDC_EXIT,1);
	else
		CheckDlgButton(IDC_EXIT,0);

	if(thePrefs.splashscreen)
		CheckDlgButton(IDC_SPLASHON,1);
	else
		CheckDlgButton(IDC_SPLASHON,0);

    //Commander - Added: Enable/Disable Startupsound - Start
    if(thePrefs.startupsound)
		CheckDlgButton(IDC_STARTUPSOUNDON,1);
	else
		CheckDlgButton(IDC_STARTUPSOUNDON,0);
    //Commander - Added: Enable/Disable Startupsound - End
	if(thePrefs.bringtoforeground)
		CheckDlgButton(IDC_BRINGTOFOREGROUND,1);
	else
		CheckDlgButton(IDC_BRINGTOFOREGROUND,0);

	if(thePrefs.updatenotify)
		CheckDlgButton(IDC_CHECK4UPDATE,1);
	else
		CheckDlgButton(IDC_CHECK4UPDATE,0);

	if(thePrefs.m_bEnableMiniMule)
		CheckDlgButton(IDC_MINIMULE,1);
	else
		CheckDlgButton(IDC_MINIMULE,0);

	if (thePrefs.GetWindowsVersion() != _WINVER_95_){
		if(thePrefs.GetPreventStandby())
			CheckDlgButton(IDC_PREVENTSTANDBY,1);
		else
			CheckDlgButton(IDC_PREVENTSTANDBY,0);
	}
	else{
		CheckDlgButton(IDC_PREVENTSTANDBY,0);
		GetDlgItem(IDC_PREVENTSTANDBY)->EnableWindow(FALSE);
	}

	CString strBuffer;
	strBuffer.Format(_T("%i %s"),thePrefs.versioncheckdays,GetResString(IDS_DAYS2));
	GetDlgItem(IDC_DAYS)->SetWindowText(strBuffer);
}

BOOL CPPgGeneral::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	((CEdit*)GetDlgItem(IDC_NICK))->SetLimitText(thePrefs.GetMaxUserNickLength());

	CWordArray aLanguageIDs;
	thePrefs.GetLanguages(aLanguageIDs);
	for (int i = 0; i < aLanguageIDs.GetSize(); i++){
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
			}

		m_language.SetItemData(m_language.AddString(szLang), aLanguageIDs[i]);
	}

	UpdateEd2kLinkFixCtrl();

	CSliderCtrl *sliderUpdate = (CSliderCtrl*)GetDlgItem(IDC_CHECKDAYS);
	sliderUpdate->SetRange(2, 7, true);
	sliderUpdate->SetPos(thePrefs.GetUpdateDays());
	
	//Commander - Added: Invisible Mode [TPT] - Start
	// Add keys to ComboBox
	for(int i='A'; i<='Z'; i++)
		((CComboBox*)GetDlgItem(IDC_INVISIBLE_MODE_SELECT_COMBO))->AddString(CString((char)(i)));
	for(int i='0'; i<='9'; i++)
		((CComboBox*)GetDlgItem(IDC_INVISIBLE_MODE_SELECT_COMBO))->AddString(CString((char)(i)));
	//Commander - Added: Invisible Mode [TPT] - End

	LoadSettings();
	InitTooltips(); //MORPH leuk_he tooltipped
	Localize();
	GetDlgItem(IDC_CHECKDAYS)->ShowWindow( IsDlgButtonChecked(IDC_CHECK4UPDATE) ? SW_SHOW : SW_HIDE );
	GetDlgItem(IDC_DAYS)->ShowWindow( IsDlgButtonChecked(IDC_CHECK4UPDATE) ? SW_SHOW : SW_HIDE );

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
	//Commander - Added: Invisible Mode [TPT] - Start
	CString sKey;
	int cur_sel = ((CComboBox*)GetDlgItem(IDC_INVISIBLE_MODE_SELECT_COMBO))->GetCurSel();
	if (cur_sel>=0)
		((CComboBox*)GetDlgItem(IDC_INVISIBLE_MODE_SELECT_COMBO))->GetLBText(cur_sel, sKey);
	if (IsDlgButtonChecked(IDC_INVISIBLE_MODE) && cur_sel>=0)
		thePrefs.SetInvisibleMode(true,m_iActualKeyModifier, (char)sKey[0]);
	else
		thePrefs.SetInvisibleMode(false,m_iActualKeyModifier, (char)sKey[0]);
	//Commander - Added: Invisible Mode [TPT] - End

	CString strNick;
	GetDlgItem(IDC_NICK)->GetWindowText(strNick);
	strNick.Trim();
	if (!IsValidEd2kString(strNick))
		strNick.Empty();
	if (strNick.IsEmpty())
	{
		strNick = DEFAULT_NICK;
		GetDlgItem(IDC_NICK)->SetWindowText(strNick);
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
	thePrefs.confirmExit = IsDlgButtonChecked(IDC_EXIT)!=0;
	thePrefs.splashscreen = IsDlgButtonChecked(IDC_SPLASHON)!=0;
	thePrefs.bringtoforeground = IsDlgButtonChecked(IDC_BRINGTOFOREGROUND)!=0;
	thePrefs.updatenotify = IsDlgButtonChecked(IDC_CHECK4UPDATE)!=0;
	thePrefs.onlineSig = IsDlgButtonChecked(IDC_ONLINESIG)!=0;
	thePrefs.versioncheckdays = ((CSliderCtrl*)GetDlgItem(IDC_CHECKDAYS))->GetPos();
	thePrefs.m_bEnableMiniMule = IsDlgButtonChecked(IDC_MINIMULE) != 0;
	thePrefs.m_bPreventStandby = IsDlgButtonChecked(IDC_PREVENTSTANDBY) != 0;
	thePrefs.startupsound = IsDlgButtonChecked(IDC_STARTUPSOUNDON)!=0;//Commander - Added: Enable/Disable Startupsound

	LoadSettings();

	SetModified(FALSE);
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
		GetDlgItem(IDC_NICK_FRM)->SetWindowText(GetResString(IDS_QL_USERNAME));
		GetDlgItem(IDC_LANG_FRM)->SetWindowText(GetResString(IDS_PW_LANG));
		GetDlgItem(IDC_MISC_FRM)->SetWindowText(GetResString(IDS_PW_MISC));
		GetDlgItem(IDC_EXIT)->SetWindowText(GetResString(IDS_PW_PROMPT));
		GetDlgItem(IDC_SPLASHON)->SetWindowText(GetResString(IDS_PW_SPLASH));
		GetDlgItem(IDC_BRINGTOFOREGROUND)->SetWindowText(GetResString(IDS_PW_FRONT));
		GetDlgItem(IDC_ONLINESIG)->SetWindowText(GetResString(IDS_PREF_ONLINESIG));	
		GetDlgItem(IDC_STARTMIN)->SetWindowText(GetResString(IDS_PREF_STARTMIN));	
		GetDlgItem(IDC_WEBSVEDIT)->SetWindowText(GetResString(IDS_WEBSVEDIT));
		GetDlgItem(IDC_ED2KFIX)->SetWindowText(GetResString(IDS_ED2KLINKFIX));
		GetDlgItem(IDC_CHECK4UPDATE)->SetWindowText(GetResString(IDS_CHECK4UPDATE));
		GetDlgItem(IDC_STARTUP)->SetWindowText(GetResString(IDS_STARTUP));
		GetDlgItem(IDC_STARTWIN)->SetWindowText(GetResString(IDS_STARTWITHWINDOWS));
		GetDlgItem(IDC_MINIMULE)->SetWindowText(GetResString(IDS_ENABLEMINIMULE));
		GetDlgItem(IDC_PREVENTSTANDBY)->SetWindowText(GetResString(IDS_PREVENTSTANDBY));
		GetDlgItem(IDC_STARTUPSOUNDON)->SetWindowText(GetResString(IDS_PW_STARTUPSOUND));//Commander - Added: Enable/Disable Startupsound
		//Commander - Added: Invisible Mode [TPT] - Start
		// Add key modifiers to ComboBox
		((CComboBox*)GetDlgItem(IDC_INVISIBLE_MODE_KEY_COMBO))->ResetContent();
		((CComboBox*)GetDlgItem(IDC_INVISIBLE_MODE_KEY_COMBO))->AddString(GetResString(IDS_CTRLKEY));
		((CComboBox*)GetDlgItem(IDC_INVISIBLE_MODE_KEY_COMBO))->AddString(GetResString(IDS_ALTKEY));
		((CComboBox*)GetDlgItem(IDC_INVISIBLE_MODE_KEY_COMBO))->AddString(GetResString(IDS_SHIFTKEY));
		((CComboBox*)GetDlgItem(IDC_INVISIBLE_MODE_KEY_COMBO))->AddString(GetResString(IDS_CTRLKEY) + _T(" + ") + GetResString(IDS_ALTKEY));
		((CComboBox*)GetDlgItem(IDC_INVISIBLE_MODE_KEY_COMBO))->AddString(GetResString(IDS_CTRLKEY) + _T(" + ") + GetResString(IDS_SHIFTKEY));
		((CComboBox*)GetDlgItem(IDC_INVISIBLE_MODE_KEY_COMBO))->AddString(GetResString(IDS_ALTKEY) + _T(" + ") + GetResString(IDS_SHIFTKEY));
		((CComboBox*)GetDlgItem(IDC_INVISIBLE_MODE_KEY_COMBO))->AddString(GetResString(IDS_CTRLKEY) + _T(" + ") + GetResString(IDS_ALTKEY) + _T(" + ") + GetResString(IDS_SHIFTKEY));

		CString key_modifier;
		if (m_iActualKeyModifier & MOD_CONTROL)
			key_modifier=GetResString(IDS_CTRLKEY);
		if (m_iActualKeyModifier & MOD_ALT){
			if (!key_modifier.IsEmpty()) key_modifier += " + ";
			key_modifier+=GetResString(IDS_ALTKEY);
		}
		if (m_iActualKeyModifier & MOD_SHIFT){
			if (!key_modifier.IsEmpty()) key_modifier += " + ";
			key_modifier+=GetResString(IDS_SHIFTKEY);
		}
		((CComboBox*)GetDlgItem(IDC_INVISIBLE_MODE_KEY_COMBO))->SelectString(-1,key_modifier);
		
		GetDlgItem(IDC_INVISIBLE_MODE_GROUP_BOX)->SetWindowText(GetResString(IDS_INVMODE_GROUP));
		GetDlgItem(IDC_INVISIBLE_MODE)->SetWindowText(GetResString(IDS_INVMODE));
		GetDlgItem(IDC_INVISIBLE_MODE_SELECT_STATIC)->SetWindowText(GetResString(IDS_INVMODE_HOTKEY));
		GetDlgItem(IDC_INVISIBLE_MODE_MODIFIER_STATIC)->SetWindowText(GetResString(IDS_INVMODE_MODKEY));
		GetDlgItem(IDC_INVISIBLE_MODE_KEY_STATIC)->SetWindowText(GetResString(IDS_INVMODE_VKEY));
		//Commander - Added: Invisible Mode [TPT] - End
		//MORPH START leuk_he tooltipped
		SetTool(IDC_NICK_FRM,IDS_QL_USERNAME_TIP);
		SetTool(IDC_NICK,IDS_QL_USERNAME_TIP);
		SetTool(IDC_LANG_FRM,IDS_PW_LANG_TIP);
		SetTool(IDC_LANGS,IDS_PW_LANG_TIP);
		//SetTool(IDC_MISC_FRM,IDS_PW_MISC_TIP);
		SetTool(IDC_EXIT,IDS_PW_PROMPT_TIP);
		SetTool(IDC_SPLASHON,IDS_PW_SPLASH_TIP);
		SetTool(IDC_BRINGTOFOREGROUND,IDS_PW_FRONT_TIP);
		SetTool(IDC_ONLINESIG,IDS_PREF_ONLINESIG_TIP);
		SetTool(IDC_MINIMULE,IDS_ENABLEMINIMULE_TIP);
		SetTool(IDC_PREVENTSTANDBY,IDS_PREVENTSTANDBY_TIP);
		SetTool(IDC_STARTMIN,IDS_PREF_STARTMIN_TIP);	
		SetTool(IDC_WEBSVEDIT,IDS_WEBSVEDIT_TIP);
		SetTool(IDC_ED2KFIX,IDS_ED2KLINKFIX_TIP);
		SetTool(IDC_CHECK4UPDATE,IDS_CHECK4UPDATE_TIP);
		SetTool(IDC_CHECKDAYS,IDS_CHECK4UPDATE_TIP);
		SetTool(IDC_STARTUP,IDS_STARTUP_TIP);
		SetTool(IDC_STARTWIN,IDS_STARTWITHWINDOWS_TIP);
		SetTool(IDC_STARTUPSOUNDON,IDS_PW_STARTUPSOUND_TIP);
		SetTool(IDC_INVISIBLE_MODE_KEY_COMBO,IDC_INVISIBLE_MODE_KEY_COMBO_TIP);
		SetTool(IDC_INVISIBLE_MODE_GROUP_BOX,IDS_INVMODE_GROUP_TIP);
		SetTool(IDC_INVISIBLE_MODE,IDS_INVMODE_TIP);
		SetTool(IDC_INVISIBLE_MODE_SELECT_STATIC,IDS_INVMODE_HOTKEY_TIP);
		SetTool(IDC_INVISIBLE_MODE_MODIFIER_STATIC,IDS_INVMODE_MODKEY_TIP);
		SetTool(IDC_INVISIBLE_MODE_KEY_STATIC,IDC_INVISIBLE_MODE_KEY_COMBO_TIP);
		SetTool(IDC_INVISIBLE_MODE_SELECT_COMBO ,IDC_INVISIBLE_MODE_KEY_COMBO_TIP);
		// MORPH END leuk_he tooltipped
	}
}

void CPPgGeneral::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	SetModified(TRUE);

	if (pScrollBar==GetDlgItem(IDC_CHECKDAYS)) {
		CSliderCtrl* slider =(CSliderCtrl*)pScrollBar;
		CString text;
		text.Format(_T("%i %s"),slider->GetPos(),GetResString(IDS_DAYS2));
		GetDlgItem(IDC_DAYS)->SetWindowText(text);
	}

	UpdateData(false); 
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPPgGeneral::OnBnClickedEditWebservices()
{
	theWebServices.Edit();
}

void CPPgGeneral::OnLangChange()
{
#define MIRRORS_URL	_T("http://langmirror%i.emule-project.org/lang/%i%i%i%i/")

	WORD byNewLang = (WORD)m_language.GetItemData(m_language.GetCurSel());
	if (thePrefs.GetLanguageID() != byNewLang){
		if	(!thePrefs.IsLanguageSupported(byNewLang, false)){
			if (AfxMessageBox(GetResString(IDS_ASKDOWNLOADLANGCAP) + _T("\r\n\r\n") + GetResString(IDS_ASKDOWNLOADLANG), MB_ICONQUESTION | MB_YESNO) == IDYES){
				// download file
				// create url, use random mirror for load balancing
				UINT nRand = (rand()/(RAND_MAX/3))+1;
				CString strUrl;
				strUrl.Format(MIRRORS_URL, nRand, CemuleApp::m_nVersionMjr, CemuleApp::m_nVersionMin, CemuleApp::m_nVersionUpd, CemuleApp::m_nVersionBld);
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
					OnSettingsChange();
					return;
				}
				CString strErr;
				strErr.Format(GetResString(IDS_ERR_FAILEDDOWNLOADLANG), strUrl);
				LogError(LOG_STATUSBAR, _T("%s"), strErr);
				AfxMessageBox(strErr, MB_ICONERROR | MB_OK);
			}
			// undo change selection
			for(int i = 0; i < m_language.GetCount(); i++)
				if(m_language.GetItemData(i) == thePrefs.GetLanguageID())
					m_language.SetCurSel(i);
		}
		else
			OnSettingsChange();
	}
}

void CPPgGeneral::OnBnClickedCheck4Update()
{
	SetModified();
	GetDlgItem(IDC_CHECKDAYS)->ShowWindow( IsDlgButtonChecked(IDC_CHECK4UPDATE)?SW_SHOW:SW_HIDE );
	GetDlgItem(IDC_DAYS)->ShowWindow( IsDlgButtonChecked(IDC_CHECK4UPDATE)?SW_SHOW:SW_HIDE );
}

void CPPgGeneral::OnHelp()
{
	theApp.ShowHelp(eMule_FAQ_Preferences_General);
}

BOOL CPPgGeneral::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgGeneral::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}

//Commander - Added: Invisible Mode [TPT] - Start
void CPPgGeneral::SetBoxes()
{	
	bool bImode = IsDlgButtonChecked(IDC_INVISIBLE_MODE)!=0;

	GetDlgItem(IDC_INVISIBLE_MODE_SELECT_STATIC)->EnableWindow(bImode);
	GetDlgItem(IDC_INVISIBLE_MODE_MODIFIER_STATIC)->EnableWindow(bImode);
	GetDlgItem(IDC_INVISIBLE_MODE_KEY_STATIC)->EnableWindow(bImode);
	GetDlgItem(IDC_INVISIBLE_MODE_SYMBOL_STATIC)->EnableWindow(bImode);
	GetDlgItem(IDC_INVISIBLE_MODE_KEY_COMBO)->EnableWindow(bImode);
	GetDlgItem(IDC_INVISIBLE_MODE_SELECT_COMBO)->EnableWindow(bImode);

	SetModified();
}

void CPPgGeneral::OnCbnSelchangeKeymodcombo()
{
	CString sKeyMod;
	
	((CComboBox*)GetDlgItem(IDC_INVISIBLE_MODE_KEY_COMBO))->GetLBText(((CComboBox*)GetDlgItem(IDC_INVISIBLE_MODE_KEY_COMBO))->GetCurSel(), sKeyMod);
	m_iActualKeyModifier = 0;
	if (sKeyMod.Find(GetResString(IDS_CTRLKEY))!=-1)
		m_iActualKeyModifier |= MOD_CONTROL;
	if (sKeyMod.Find(GetResString(IDS_ALTKEY))!=-1)
		m_iActualKeyModifier |= MOD_ALT;
	if (sKeyMod.Find(GetResString(IDS_SHIFTKEY))!=-1)
		m_iActualKeyModifier |= MOD_SHIFT;
	
	SetModified();
}
//Commander - Added: Invisible Mode [TPT] - End