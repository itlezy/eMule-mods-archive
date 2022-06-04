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
#include <share.h>
#include "emule.h"
#include "PPgSecurity.h"
#include "OtherFunctions.h"
#include "IPFilter.h"
#include "Preferences.h"
#include "CustomAutoComplete.h"
#include "HttpDownloadDlg.h"
#include "emuledlg.h"
#include "ZipFile.h"
#include "GZipFile.h"
//#include "RarFile.h"
#include "Log.h"
#include "ServerWnd.h"
#include "ServerListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//bool GetMimeType(LPCTSTR pszFilePath, CString& rstrMimeType);

#define	IPFILTERUPDATEURL_STRINGS_PROFILE	_T("AC_IPFilterUpdateURLs.dat")

IMPLEMENT_DYNAMIC(CPPgSecurity, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgSecurity, CPropertyPage)
	ON_BN_CLICKED(IDC_FILTERSERVERBYIPFILTER , OnSettingsChange)
	ON_BN_CLICKED(IDC_RELOADFILTER, OnReloadIPFilter)
	ON_BN_CLICKED(IDC_EDITFILTER, OnEditIPFilter)
	ON_EN_CHANGE(IDC_FILTERLEVEL, OnSettingsChange)
	ON_BN_CLICKED(IDC_USESECIDENT, OnSettingsChange)
	ON_BN_CLICKED(IDC_LOADURL, OnLoadIPFFromURL)
	ON_EN_CHANGE(IDC_UPDATEURL, OnEnChangeUpdateUrl)
	ON_BN_CLICKED(IDC_DD,OnDDClicked)
	ON_BN_CLICKED(IDC_RUNASUSER, OnBnClickedRunAsUser)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_ENABLEOBFUSCATION, OnObfuscatedRequestedChange)
	ON_BN_CLICKED(IDC_ONLYOBFUSCATED, OnSettingsChange)
	ON_BN_CLICKED(IDC_DISABLEOBFUSCATION, OnObfuscatedDisabledChange)
	ON_BN_CLICKED(IDC_SEARCHSPAMFILTER, OnSettingsChange)
	ON_BN_CLICKED(IDC_CHECK_FILE_OPEN, OnSettingsChange)
	ON_BN_CLICKED(IDC_AUTOUPDATEIPFILTER, OnSettingsChange) //Xman auto update IPFilter
END_MESSAGE_MAP()

CPPgSecurity::CPPgSecurity()
	: CPropertyPage(CPPgSecurity::IDD)
{
	m_pacIPFilterURL = NULL;
}

CPPgSecurity::~CPPgSecurity()
{
}

void CPPgSecurity::LoadSettings(void)
{
	TCHAR szBuff[12];
	_itot_s(thePrefs.filterlevel, szBuff, 10);// X: [CI] - [Code Improvement]
	szBuff[_countof(szBuff) - 1] = _T('\0');
	SetDlgItemText(IDC_FILTERLEVEL,szBuff);
	CheckDlgButton(IDC_FILTERSERVERBYIPFILTER, thePrefs.filterserverbyip);

	CheckDlgButton(IDC_USESECIDENT, thePrefs.m_bUseSecureIdent);

	GetDlgItem(IDC_RUNASUSER)->EnableWindow(( thePrefs.GetWindowsVersion() == _WINVER_XP_ || thePrefs.GetWindowsVersion() == _WINVER_2K_ || thePrefs.GetWindowsVersion() == _WINVER_2003_)
		&& thePrefs.m_nCurrentUserDirMode == 2);
	CheckDlgButton(IDC_RUNASUSER, thePrefs.IsRunAsUserEnabled());

	if (!thePrefs.IsClientCryptLayerSupported()){
		CheckDlgButton(IDC_DISABLEOBFUSCATION,1);
		GetDlgItem(IDC_ENABLEOBFUSCATION)->EnableWindow(FALSE);
		GetDlgItem(IDC_ONLYOBFUSCATED)->EnableWindow(FALSE);
	}
	else{
		CheckDlgButton(IDC_DISABLEOBFUSCATION,0);
		GetDlgItem(IDC_ENABLEOBFUSCATION)->EnableWindow(TRUE);
		GetDlgItem(IDC_ONLYOBFUSCATED)->EnableWindow(TRUE);
	}

	if (thePrefs.IsClientCryptLayerRequested()){
		CheckDlgButton(IDC_ENABLEOBFUSCATION,1);
		GetDlgItem(IDC_ONLYOBFUSCATED)->EnableWindow(TRUE);
	}
	else{
		CheckDlgButton(IDC_ENABLEOBFUSCATION,0);
		GetDlgItem(IDC_ONLYOBFUSCATED)->EnableWindow(FALSE);
	}

	CheckDlgButton(IDC_ONLYOBFUSCATED, thePrefs.IsClientCryptLayerRequired());
	CheckDlgButton(IDC_SEARCHSPAMFILTER, thePrefs.IsSearchSpamFilterEnabled());
	CheckDlgButton(IDC_CHECK_FILE_OPEN, thePrefs.GetCheckFileOpen());
    //Xman auto update IPFilter
	CheckDlgButton(IDC_AUTOUPDATEIPFILTER, thePrefs.AutoUpdateIPFilter());
	//Xman end
}

BOOL CPPgSecurity::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	LoadSettings();
	Localize();

	//if (thePrefs.GetUseAutocompletion()) {
		if (!m_pacIPFilterURL) {
			m_pacIPFilterURL = new CCustomAutoComplete();
			m_pacIPFilterURL->AddRef();
			if (m_pacIPFilterURL->Bind(::GetDlgItem(m_hWnd, IDC_UPDATEURL), ACO_UPDOWNKEYDROPSLIST | ACO_AUTOSUGGEST | ACO_FILTERPREFIXES ))
				m_pacIPFilterURL->LoadList(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + IPFILTERUPDATEURL_STRINGS_PROFILE);
		}
		SetDlgItemText(IDC_UPDATEURL,m_pacIPFilterURL->GetItem(0));
		if (theApp.m_fontSymbol.m_hObject) {
			GetDlgItem(IDC_DD)->SetFont(&theApp.m_fontSymbol);
			SetDlgItemText(IDC_DD,_T("6")); // show a down-arrow
		}
	//}
	//else
	//GetDlgItem(IDC_DD)->ShowWindow(SW_HIDE);

	//Xman auto update IPFilter
	CString url;
	GetDlgItemText(IDC_UPDATEURL,url);
	//in case we don't use Auto-completion we have to take the prefs-value
	if (url.IsEmpty())
		SetDlgItemText(IDC_UPDATEURL, thePrefs.GetAutoUpdateIPFilter_URL());
	else
	{
		//in case we use the auto-completion we must update the prefs-value 
		thePrefs.SetAutoUpdateIPFilter_URL(url);
	}
	//Xman end
	m_bModified = false; // X: [CI] - [Code Improvement] Apply if modified

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPgSecurity::OnApply()
{
	if(m_bModified) // X: [CI] - [Code Improvement] Apply if modified
	{
		//Xman auto update IPFilter
		CString url;
		GetDlgItemText(IDC_UPDATEURL,url);
		if(url.GetLength()>=_MAX_PATH)
			AfxMessageBox(_T("typed url is too long"));
		else
			thePrefs.SetAutoUpdateIPFilter_URL(url);
		//Xman end

	bool bIPFilterSettingsChanged = false;

	if (GetDlgItem(IDC_FILTERLEVEL)->GetWindowTextLength()) {
		int iNewFilterLevel = GetDlgItemInt(IDC_FILTERLEVEL,NULL, FALSE);
		if (iNewFilterLevel >= 0 && (UINT)iNewFilterLevel != thePrefs.filterlevel) {
			thePrefs.filterlevel = iNewFilterLevel;
			bIPFilterSettingsChanged = IsDlgButtonChecked(IDC_FILTERSERVERBYIPFILTER)!=0;
		}
	}
	if (!thePrefs.filterserverbyip && IsDlgButtonChecked(IDC_FILTERSERVERBYIPFILTER)!=0)
		bIPFilterSettingsChanged = true;
	thePrefs.filterserverbyip = IsDlgButtonChecked(IDC_FILTERSERVERBYIPFILTER)!=0;
	if (bIPFilterSettingsChanged)
		theApp.emuledlg->serverwnd->serverlistctrl.RemoveAllFilteredServers();

	thePrefs.m_bUseSecureIdent = IsDlgButtonChecked(IDC_USESECIDENT)!=0;
	thePrefs.m_bRunAsUser = IsDlgButtonChecked(IDC_RUNASUSER)!=0;

	thePrefs.m_bCryptLayerRequested = IsDlgButtonChecked(IDC_ENABLEOBFUSCATION) != 0;
	thePrefs.m_bCryptLayerRequired = IsDlgButtonChecked(IDC_ONLYOBFUSCATED) != 0;
	thePrefs.m_bCryptLayerSupported = IsDlgButtonChecked(IDC_DISABLEOBFUSCATION) == 0;
	thePrefs.m_bCheckFileOpen = IsDlgButtonChecked(IDC_CHECK_FILE_OPEN) != 0;
	thePrefs.m_bEnableSearchResultFilter = IsDlgButtonChecked(IDC_SEARCHSPAMFILTER) != 0;
        //Xman auto update IPFilter
		thePrefs.SetAutoUpdateIPFilter(IsDlgButtonChecked(IDC_AUTOUPDATEIPFILTER)!=0);
		//Xman end

	LoadSettings();
	SetModified(FALSE);
		m_bModified = false; // X: [CI] - [Code Improvement] Apply if modified
	}
	return CPropertyPage::OnApply();
}

void CPPgSecurity::Localize(void)
{
	if (m_hWnd)
	{
		SetWindowText(GetResString(IDS_SECURITY));
		SetDlgItemText(IDC_STATIC_IPFILTER,GetResString(IDS_IPFILTER));
		SetDlgItemText(IDC_RELOADFILTER,GetResString(IDS_SF_RELOAD));
		SetDlgItemText(IDC_EDITFILTER,GetResString(IDS_EDIT));
		SetDlgItemText(IDC_STATIC_FILTERLEVEL,GetResString(IDS_FILTERLEVEL)+_T(':'));
		SetDlgItemText(IDC_FILTERSERVERBYIPFILTER,GetResString(IDS_FILTERSERVERBYIPFILTER));

		SetDlgItemText(IDC_SEC_MISC,GetResString(IDS_PW_MISC));
		SetDlgItemText(IDC_USESECIDENT,GetResString(IDS_USESECIDENT));
		SetDlgItemText(IDC_RUNASUSER,GetResString(IDS_RUNASUSER));

		SetDlgItemText(IDC_STATIC_UPDATEFROM,GetResString(IDS_UPDATEFROM));
		SetDlgItemText(IDC_LOADURL,GetResString(IDS_LOADURL));

		SetDlgItemText(IDC_DISABLEOBFUSCATION,GetResString(IDS_DISABLEOBFUSCATION));
		SetDlgItemText(IDC_ONLYOBFUSCATED,GetResString(IDS_ONLYOBFUSCATED));
		SetDlgItemText(IDC_ENABLEOBFUSCATION,GetResString(IDS_ENABLEOBFUSCATION));
		SetDlgItemText(IDC_SEC_OBFUSCATIONBOX,GetResString(IDS_PROTOCOLOBFUSCATION));
		SetDlgItemText(IDC_SEARCHSPAMFILTER,GetResString(IDS_SEARCHSPAMFILTER));
		SetDlgItemText(IDC_CHECK_FILE_OPEN,GetResString(IDS_CHECK_FILE_OPEN));
        //Xman auto update IPFilter
		SetDlgItemText(IDC_AUTOUPDATEIPFILTER,GetResString(IDS_AUTOUPDATEIPFILTER));
	}
}

void CPPgSecurity::OnReloadIPFilter()
{
	CWaitCursor curHourglass;
	theApp.ipfilter->LoadFromDefaultFile();
	//if (thePrefs.GetFilterServerByIP())
	theApp.emuledlg->serverwnd->serverlistctrl.RemoveAllFilteredServers();
}

void CPPgSecurity::OnEditIPFilter()
{
	ShellExecute(NULL, _T("open"), thePrefs.GetTxtEditor(),
		_T('\"') + thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + DFLT_IPFILTER_FILENAME _T("\""), NULL, SW_SHOW);
}

void CPPgSecurity::OnLoadIPFFromURL()
{
	CString url;
	GetDlgItemText(IDC_UPDATEURL,url);
	//Xman auto update IPFilter
	if (!url.IsEmpty()){
	// add entered URL to LRU list even if it's not yet known whether we can download from this URL (it's just more convenient this way)
		if (m_pacIPFilterURL && m_pacIPFilterURL->IsBound())
			m_pacIPFilterURL->AddItem(url, 0);
		theApp.ipfilter->UpdateIPFilterURL(url);
	}
	else{
		AfxMessageBox(_T("Failed to auto-update IPFilter. No URL given"), MB_ICONERROR);
		return;
	}
}

void CPPgSecurity::OnDestroy()
{
	DeleteDDB();
	CPropertyPage::OnDestroy();
}

void CPPgSecurity::DeleteDDB()
{
	if (m_pacIPFilterURL)
	{
		m_pacIPFilterURL->SaveList(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + IPFILTERUPDATEURL_STRINGS_PROFILE);
		m_pacIPFilterURL->Unbind();
		m_pacIPFilterURL->Release();
		m_pacIPFilterURL = NULL;
	}
}

BOOL CPPgSecurity::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN){

		if (pMsg->wParam == VK_ESCAPE)
			return FALSE;

		if (pMsg->wParam == VK_DELETE && m_pacIPFilterURL && m_pacIPFilterURL->IsBound() && pMsg->hwnd == GetDlgItem(IDC_UPDATEURL)->m_hWnd)
		{
			if (GetAsyncKeyState(VK_MENU)<0 || GetAsyncKeyState(VK_CONTROL)<0)
				m_pacIPFilterURL->Clear();
			else
				m_pacIPFilterURL->RemoveSelectedItem();
		}

		if (pMsg->wParam == VK_RETURN){
			if (pMsg->hwnd == GetDlgItem(IDC_UPDATEURL)->m_hWnd){
				if (m_pacIPFilterURL && m_pacIPFilterURL->IsBound() ){
					CString strText;
					GetDlgItemText(IDC_UPDATEURL,strText);
					if (!strText.IsEmpty()){
						SetDlgItemText(IDC_UPDATEURL,_T("")); // this seems to be the only chance to let the dropdown list to disapear
						SetDlgItemText(IDC_UPDATEURL,strText);
						((CEdit*)GetDlgItem(IDC_UPDATEURL))->SetSel(strText.GetLength(), strText.GetLength());
					}
				}
				return TRUE;
			}
		}
	}
   
	return CPropertyPage::PreTranslateMessage(pMsg);
}

void CPPgSecurity::OnEnChangeUpdateUrl()
{
	CString strUrl;
	GetDlgItemText(IDC_UPDATEURL, strUrl);
	GetDlgItem(IDC_LOADURL)->EnableWindow(!strUrl.IsEmpty());
}

void CPPgSecurity::OnDDClicked()
{
	CWnd* box=GetDlgItem(IDC_UPDATEURL);
	box->SetFocus();
	box->SetWindowText(_T(""));
	box->SendMessage(WM_KEYDOWN,VK_DOWN,0x00510001);
}

void CPPgSecurity::OnBnClickedRunAsUser()
{
	if ( ((CButton*)GetDlgItem(IDC_RUNASUSER))->GetCheck() == BST_CHECKED){
		if (AfxMessageBox(GetResString(IDS_RAU_WARNING),MB_OKCANCEL | MB_ICONINFORMATION,0) == IDCANCEL)
			CheckDlgButton(IDC_RUNASUSER,BST_UNCHECKED);
	}
	OnSettingsChange();
}

void CPPgSecurity::OnObfuscatedDisabledChange(){
	if (IsDlgButtonChecked(IDC_DISABLEOBFUSCATION) != 0){
		GetDlgItem(IDC_ENABLEOBFUSCATION)->EnableWindow(FALSE);
		GetDlgItem(IDC_ONLYOBFUSCATED)->EnableWindow(FALSE);
		CheckDlgButton(IDC_ENABLEOBFUSCATION, 0);
		CheckDlgButton(IDC_ONLYOBFUSCATED, 0);
	}
	else{
		GetDlgItem(IDC_ENABLEOBFUSCATION)->EnableWindow(TRUE);
	}
	OnSettingsChange();
}

void CPPgSecurity::OnObfuscatedRequestedChange(){
	if (IsDlgButtonChecked(IDC_ENABLEOBFUSCATION) == 0){
		GetDlgItem(IDC_ONLYOBFUSCATED)->EnableWindow(FALSE);
		CheckDlgButton(IDC_ONLYOBFUSCATED, 0);
	}
	else{
		GetDlgItem(IDC_ENABLEOBFUSCATION)->EnableWindow(TRUE);
		GetDlgItem(IDC_ONLYOBFUSCATED)->EnableWindow(TRUE);
	}
	OnSettingsChange();
}
