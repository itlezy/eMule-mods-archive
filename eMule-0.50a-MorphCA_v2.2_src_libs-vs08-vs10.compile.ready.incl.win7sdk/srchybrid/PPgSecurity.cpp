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
#include "HttpDownloadDlg.h"
#include "emuledlg.h"
#include "ZipFile.h"
#include "GZipFile.h"
#include "RarFile.h"
#include "Log.h"
#include "ServerWnd.h"
#include "ServerListCtrl.h"
#include "Scheduler.h"
#ifdef CLIENTANALYZER
#include "ClientCredits.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

IMPLEMENT_DYNAMIC(CPPgSecurity, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgSecurity, CPropertyPage)
	ON_BN_CLICKED(IDC_FILTERSERVERBYIPFILTER , OnSettingsChange)
	ON_BN_CLICKED(IDC_RELOADFILTER, OnReloadIPFilter)
	ON_BN_CLICKED(IDC_EDITFILTER, OnEditIPFilter)
	ON_EN_CHANGE(IDC_FILTERLEVEL, OnSettingsChange)
	ON_BN_CLICKED(IDC_USESECIDENT, OnSettingsChange)
	ON_BN_CLICKED(IDC_RUNASUSER, OnBnClickedRunAsUser)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_SEESHARE1, OnSettingsChange)
	ON_BN_CLICKED(IDC_SEESHARE2, OnSettingsChange)
	ON_BN_CLICKED(IDC_SEESHARE3, OnSettingsChange)
	ON_BN_CLICKED(IDC_ENABLEOBFUSCATION, OnObfuscatedRequestedChange)
	ON_BN_CLICKED(IDC_ONLYOBFUSCATED, OnSettingsChange)
	ON_BN_CLICKED(IDC_DISABLEOBFUSCATION, OnObfuscatedDisabledChange)
	ON_BN_CLICKED(IDC_SEARCHSPAMFILTER, OnSettingsChange)
	ON_BN_CLICKED(IDC_CHECK_FILE_OPEN, OnSettingsChange)
//MORPH START added by Yun.SF3: Ipfilter.dat update
	ON_EN_CHANGE(IDC_UPDATE_URL_IPFILTER, OnSettingsChange)
	ON_BN_CLICKED(IDC_UPDATEIPFURL, OnBnClickedUpdateipfurl)
	ON_BN_CLICKED(IDC_RESETIPFURL, OnBnClickedResetipfurl)
	ON_BN_CLICKED(IDC_AUTOUPIPFILTER , OnSettingsChange)
	ON_BN_CLICKED(IDC_AUTOUPIPFILTERWEEK, OnSettingsChange)
	//MORPH END added by Yun.SF3: Ipfilter.dat update
END_MESSAGE_MAP()

CPPgSecurity::CPPgSecurity()
	: CPropertyPage(CPPgSecurity::IDD)
{
}

CPPgSecurity::~CPPgSecurity()
{
}

//MORPH START added by Yun.SF3: Ipfilter.dat update
void SysTimeToStr(LPSYSTEMTIME st, LPTSTR str)
{
	TCHAR sDate[15];
	sDate[0] = _T('\0');
	GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, st, NULL, sDate, 100);
	TCHAR sTime[15];
	sTime[0] = _T('\0');
	GetTimeFormat(LOCALE_USER_DEFAULT, 0, st, NULL ,sTime ,100);
	_stprintf(str, _T("%s %s"), sDate, sTime);
}
//MORPH START added by Yun.SF3: Ipfilter.dat update

void CPPgSecurity::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

void CPPgSecurity::LoadSettings(void)
{
	CString strBuffer;
	
	strBuffer.Format(_T("%i"),thePrefs.filterlevel);
	GetDlgItem(IDC_FILTERLEVEL)->SetWindowText(strBuffer);
	CheckDlgButton(IDC_FILTERSERVERBYIPFILTER, thePrefs.filterserverbyip);

#ifndef CLIENTANALYZER
	CheckDlgButton(IDC_USESECIDENT, thePrefs.m_bUseSecureIdent);
#endif

	if ((thePrefs.GetWindowsVersion() == _WINVER_XP_ || thePrefs.GetWindowsVersion() == _WINVER_2K_ || thePrefs.GetWindowsVersion() == _WINVER_2003_)
		&& thePrefs.m_nCurrentUserDirMode == 2)
		GetDlgItem(IDC_RUNASUSER)->EnableWindow(TRUE);
	else
		GetDlgItem(IDC_RUNASUSER)->EnableWindow(FALSE);
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

	ASSERT( vsfaEverybody == 0 );
	ASSERT( vsfaFriends == 1 );
	ASSERT( vsfaNobody == 2 );
	CheckRadioButton(IDC_SEESHARE1, IDC_SEESHARE3, IDC_SEESHARE1 + thePrefs.m_iSeeShares);
}

BOOL CPPgSecurity::OnInitDialog()
{
//MORPH START added by Yun.SF3: Ipfilter.dat update
	GetDlgItem(IDC_UPDATE_URL_IPFILTER)->SetWindowText(thePrefs.UpdateURLIPFilter);
	if(thePrefs.AutoUpdateIPFilter)
		CheckDlgButton(IDC_AUTOUPIPFILTER,1);
	else
		CheckDlgButton(IDC_AUTOUPIPFILTER,0);
    if (theApp.scheduler->HasWeekly(ACTION_UPDIPCONF)) // check in schedule.
 		CheckDlgButton(IDC_AUTOUPIPFILTERWEEK,1);
	else
		CheckDlgButton(IDC_AUTOUPIPFILTERWEEK,0);
	//MORPH END added by Yun.SF3: Ipfilter.dat update
       
	//MORPH START - Added by Stulle, New IP Filter by Ozzy [Stulle/Ozzy]	
	TCHAR sTime[30];

	if(thePrefs.IsIPFilterViaDynDNS())
	{
		CString strBuffer=NULL;
		if(PathFileExists(theApp.ipfilter->GetDefaultFilePath()))
			strBuffer.Format(_T("v%u"), thePrefs.GetIPFilterVersionNum());
		else
			strBuffer=GetResString(IDS_DL_NONE);
		GetDlgItem(IDC_IPFILTER_VERSION)->SetWindowText(strBuffer);
	}
	else
	{
		sTime[0] = _T('\0');
		SysTimeToStr(thePrefs.GetIPfilterVersion(), sTime);
		GetDlgItem(IDC_IPFILTER_VERSION)->SetWindowText(sTime);
	}
	//MORPH END   - Added by Stulle, New IP Filter by Ozzy [Stulle/Ozzy]

	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);
	//MORPH START - Added by SiRoB, Allways use securedid
	GetDlgItem(IDC_USESECIDENT)->EnableWindow(FALSE);
	//MORPH END   - Added by SiRoB, Allways use securedid
	LoadSettings();
	Localize();

#ifdef CLIENTANALYZER
	GetDlgItem(IDC_USESECIDENT)->EnableWindow(FALSE);
	if(theApp.clientcredits->CryptoAvailable())
		CheckDlgButton(IDC_USESECIDENT, BST_CHECKED);
	else
		CheckDlgButton(IDC_USESECIDENT, BST_UNCHECKED);
#endif

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPgSecurity::OnApply()
{
	bool bIPFilterSettingsChanged = false;

	TCHAR buffer[510];
	if (GetDlgItem(IDC_FILTERLEVEL)->GetWindowTextLength()) {
		GetDlgItem(IDC_FILTERLEVEL)->GetWindowText(buffer,4);
		int iNewFilterLevel = _tstoi(buffer);
		if (iNewFilterLevel >= 0 && (UINT)iNewFilterLevel != thePrefs.filterlevel) {
			thePrefs.filterlevel = iNewFilterLevel;
			OnReloadIPFilter();//morph4u moved
			bIPFilterSettingsChanged = IsDlgButtonChecked(IDC_FILTERSERVERBYIPFILTER)!=0;
		}
	}
	if (!thePrefs.filterserverbyip && IsDlgButtonChecked(IDC_FILTERSERVERBYIPFILTER)!=0)
		bIPFilterSettingsChanged = true;
	thePrefs.filterserverbyip = IsDlgButtonChecked(IDC_FILTERSERVERBYIPFILTER)!=0;
	if (bIPFilterSettingsChanged)
		theApp.emuledlg->serverwnd->serverlistctrl.RemoveAllFilteredServers();

#ifndef CLIENTANALYZER
	thePrefs.m_bUseSecureIdent = IsDlgButtonChecked(IDC_USESECIDENT)!=0;
#endif
	thePrefs.m_bRunAsUser = IsDlgButtonChecked(IDC_RUNASUSER)!=0;

	thePrefs.m_bCryptLayerRequested = IsDlgButtonChecked(IDC_ENABLEOBFUSCATION) != 0;
	thePrefs.m_bCryptLayerRequired = IsDlgButtonChecked(IDC_ONLYOBFUSCATED) != 0;
	thePrefs.m_bCryptLayerSupported = IsDlgButtonChecked(IDC_DISABLEOBFUSCATION) == 0;
	thePrefs.m_bCheckFileOpen = IsDlgButtonChecked(IDC_CHECK_FILE_OPEN) != 0;
	thePrefs.m_bEnableSearchResultFilter = IsDlgButtonChecked(IDC_SEARCHSPAMFILTER) != 0;


	if (IsDlgButtonChecked(IDC_SEESHARE1))
		thePrefs.m_iSeeShares = vsfaEverybody;
	else if (IsDlgButtonChecked(IDC_SEESHARE2))
		thePrefs.m_iSeeShares = vsfaFriends;
	else
		thePrefs.m_iSeeShares = vsfaNobody;

	//MORPH START - Added by Yun.SF3: Ipfilter.dat update
	CString strBuffer;

	GetDlgItem(IDC_UPDATE_URL_IPFILTER)->GetWindowText(strBuffer);
	_tcscpy(thePrefs.UpdateURLIPFilter, strBuffer);
	thePrefs.AutoUpdateIPFilter = IsDlgButtonChecked(IDC_AUTOUPIPFILTER)!=0;
	theApp.scheduler->SetWeekly(ACTION_UPDIPCONF,IsDlgButtonChecked(IDC_AUTOUPIPFILTERWEEK)!=0);
	//MORPH END   - Added by Yun.SF3: Ipfilter.dat update

	LoadSettings();
	SetModified(FALSE);
	return CPropertyPage::OnApply();
}

void CPPgSecurity::Localize(void)
{
	if(m_hWnd)
	{
        //MORPH START - Added by Yun.SF3: Ipfilter.dat update
		GetDlgItem(IDC_AUTOUPIPFILTER)->SetWindowText(GetResString(IDS_UPDATEIPFILTERONSTART));
		GetDlgItem(IDC_UPDATEIPFURL)->SetWindowText(GetResString(IDS_UPDATEIPCURL));
		GetDlgItem(IDC_RESETIPFURL)->SetWindowText(GetResString(IDS_RESET));
		GetDlgItem(IDC_AUTOUPIPFILTERWEEK)->SetWindowText(GetResString(IDS_AUTOUPIPFILTERWEEK));
		//MORPH START - Added by Stulle, New IP Filter by Ozzy [Stulle/Ozzy]
		TCHAR sTime[30];
		if(thePrefs.IsIPFilterViaDynDNS())
		{
			CString strBuffer=NULL;
			if(PathFileExists(theApp.ipfilter->GetDefaultFilePath()))
				strBuffer.Format(_T("v%u"), thePrefs.GetIPFilterVersionNum());
			else
				strBuffer=GetResString(IDS_DL_NONE);
			GetDlgItem(IDC_IPFILTER_VERSION)->SetWindowText(strBuffer);
		}
		else
		{
			sTime[0] = _T('\0');
			SysTimeToStr(thePrefs.GetIPfilterVersion(), sTime);
			GetDlgItem(IDC_IPFILTER_VERSION)->SetWindowText(sTime);
		}
		//MORPH END   - Added by Stulle, New IP Filter by Ozzy [Stulle/Ozzy]
		//MORPH END   - Added by Yun.SF3: Ipfilter.dat update

		SetWindowText(GetResString(IDS_SECURITY));
		GetDlgItem(IDC_STATIC_IPFILTER)->SetWindowText(GetResString(IDS_IPFILTER));
		GetDlgItem(IDC_RELOADFILTER)->SetWindowText(GetResString(IDS_SF_RELOAD));
		GetDlgItem(IDC_EDITFILTER)->SetWindowText(GetResString(IDS_EDIT));
		GetDlgItem(IDC_STATIC_FILTERLEVEL)->SetWindowText(GetResString(IDS_FILTERLEVEL)+_T(":"));
		GetDlgItem(IDC_FILTERSERVERBYIPFILTER)->SetWindowText(GetResString(IDS_FILTERSERVERBYIPFILTER));

		GetDlgItem(IDC_SEC_MISC)->SetWindowText(GetResString(IDS_PW_MISC));
		GetDlgItem(IDC_USESECIDENT)->SetWindowText(GetResString(IDS_USESECIDENT));
		SetDlgItemText(IDC_RUNASUSER,GetResString(IDS_RUNASUSER));

		SetDlgItemText(IDC_STATIC_UPDATEFROM,GetResString(IDS_UPDATEFROM));

        GetDlgItem(IDC_SEEMYSHARE_FRM)->SetWindowText(GetResString(IDS_PW_SHARE));
		GetDlgItem(IDC_SEESHARE1)->SetWindowText(GetResString(IDS_PW_EVER));
		GetDlgItem(IDC_SEESHARE2)->SetWindowText(GetResString(IDS_FSTATUS_FRIENDSONLY));
		GetDlgItem(IDC_SEESHARE3)->SetWindowText(GetResString(IDS_PW_NOONE));

		GetDlgItem(IDC_DISABLEOBFUSCATION)->SetWindowText(GetResString(IDS_DISABLEOBFUSCATION));
		GetDlgItem(IDC_ONLYOBFUSCATED)->SetWindowText(GetResString(IDS_ONLYOBFUSCATED));
		GetDlgItem(IDC_ENABLEOBFUSCATION)->SetWindowText(GetResString(IDS_ENABLEOBFUSCATION));
		GetDlgItem(IDC_SEC_OBFUSCATIONBOX)->SetWindowText(GetResString(IDS_PROTOCOLOBFUSCATION));
		GetDlgItem(IDC_SEARCHSPAMFILTER)->SetWindowText(GetResString(IDS_SEARCHSPAMFILTER));
		GetDlgItem(IDC_CHECK_FILE_OPEN)->SetWindowText(GetResString(IDS_CHECK_FILE_OPEN));
	}
}

void CPPgSecurity::OnReloadIPFilter()
{
	CWaitCursor curHourglass;
	theApp.ipfilter->LoadFromDefaultFile();
	if (thePrefs.GetFilterServerByIP())
		theApp.emuledlg->serverwnd->serverlistctrl.RemoveAllFilteredServers();
}

void CPPgSecurity::OnEditIPFilter()
{
	ShellExecute(NULL, _T("open"), thePrefs.GetTxtEditor(),
		_T("\"") + thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + DFLT_IPFILTER_FILENAME _T("\""), NULL, SW_SHOW);
}

void CPPgSecurity::OnBnClickedRunAsUser()
{
	if ( ((CButton*)GetDlgItem(IDC_RUNASUSER))->GetCheck() == BST_CHECKED){
		if (AfxMessageBox(GetResString(IDS_RAU_WARNING),MB_OKCANCEL | MB_ICONINFORMATION,0) == IDCANCEL)
			((CButton*)GetDlgItem(IDC_RUNASUSER))->SetCheck(BST_UNCHECKED);
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
//MORPH START added by Yun.SF3: Ipfilter.dat update
void CPPgSecurity::OnBnClickedUpdateipfurl()
{
	OnApply();
	//MORPH START - Added by Stulle, New IP Filter by Ozzy [Stulle/Ozzy]
	/*
	theApp.ipfilter->UpdateIPFilterURL();
	TCHAR sBuffer[30];
	sBuffer[0] = _T('\0'); 
	SysTimeToStr(thePrefs.GetIPfilterVersion(), sBuffer);
	GetDlgItem(IDC_IPFILTER_VERSION)->SetWindowText(sBuffer);
	*/
	theApp.emuledlg->CheckIPFilter();
	if(thePrefs.IsIPFilterViaDynDNS())
	{
		CString strBuffer=NULL;
		if(PathFileExists(theApp.ipfilter->GetDefaultFilePath()))
			strBuffer.Format(_T("v%u"), thePrefs.GetIPFilterVersionNum());
		else
			strBuffer=GetResString(IDS_DL_NONE);
		GetDlgItem(IDC_IPFILTER_VERSION)->SetWindowText(strBuffer);
	}
	else
	{
		TCHAR sTime[30];
		sTime[0] = _T('\0');
		SysTimeToStr(thePrefs.GetIPfilterVersion(), sTime);
		GetDlgItem(IDC_IPFILTER_VERSION)->SetWindowText(sTime);
	}
	//MORPH END   - Added by Stulle, New IP Filter by Ozzy [Stulle/Ozzy]
}
//MORPH END added by Yun.SF3: Ipfilter.dat update

void CPPgSecurity::OnBnClickedResetipfurl()
{
	CString strBuffer = _T("http://downloads.sourceforge.net/scarangel/ipfilter.rar");
	GetDlgItem(IDC_UPDATE_URL_IPFILTER)->SetWindowText(strBuffer);
	//MORPH START - Added by Stulle, New IP Filter by Ozzy [Stulle/Ozzy]
	/*
	GetDlgItem(IDC_IPFILTER_VERSION)->SetWindowText(_T(""));
	*/
	thePrefs.m_uIPFilterVersionNum = 0;
	if(PathFileExists(theApp.ipfilter->GetDefaultFilePath()))
		strBuffer = _T("v0");
	else
		strBuffer = GetResString(IDS_DL_NONE);
	GetDlgItem(IDC_IPFILTER_VERSION)->SetWindowText(strBuffer);
	//MORPH END   - Added by Stulle, New IP Filter by Ozzy [Stulle/Ozzy]
}