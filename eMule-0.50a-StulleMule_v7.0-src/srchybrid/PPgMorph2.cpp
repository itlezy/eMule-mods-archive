// PpgMorph2.cpp : implementation file
//

#include "stdafx.h"
#include "emule.h"
#include "PPGtooltipped.h" //MORPH leuk_he addded tooltipped
#include "PPgMorph2.h"
#include "emuleDlg.h"
#include "OtherFunctions.h"
#include "serverWnd.h"
#include "Fakecheck.h"
#include "IPFilter.h"
#include "IP2Country.h"
#include "Scheduler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

// CPPgMorph dialog

IMPLEMENT_DYNAMIC(CPPgMorph2, CPropertyPage)
CPPgMorph2::CPPgMorph2()
// MORPH START leuk_he tooltipped
: CPPgtooltipped(CPPgMorph2::IDD)// leuk_he  tooltipped 
/*
: CPropertyPage(CPPgMorph2::IDD)
*/
// MORPH END leuk_he tooltipped
{
}

CPPgMorph2::~CPPgMorph2()
{
}

void CPPgMorph2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPPgMorph2, CPropertyPage)
	//MORPH START - Added by milobac and Yun.SF3, FakeCheck, FakeReport, Auto-updating
	ON_BN_CLICKED(IDC_UPDATEFAKELISTSTART, OnSettingsChange)
	ON_BN_CLICKED(IDC_RESETFAKESURL, OnBnClickedResetfakes)
	ON_BN_CLICKED(IDC_UPDATEFAKES, OnBnClickedUpdatefakes)
	ON_EN_CHANGE(IDC_UPDATE_URL_FAKELIST, OnSettingsChange)
	ON_BN_CLICKED(IDC_UPDATEFAKELISTWEEK, OnSettingsChange)
	//MORPH END - Added by milobac and Yun.SF3, FakeCheck, FakeReport, Auto-updating
	//MORPH START added by Yun.SF3: Ipfilter.dat update
	ON_EN_CHANGE(IDC_UPDATE_URL_IPFILTER, OnSettingsChange)
	ON_BN_CLICKED(IDC_UPDATEIPFURL, OnBnClickedUpdateipfurl)
	ON_BN_CLICKED(IDC_RESETIPFURL, OnBnClickedResetipfurl)
	ON_BN_CLICKED(IDC_AUTOUPIPFILTER , OnSettingsChange)
	ON_BN_CLICKED(IDC_AUTOUPIPFILTERWEEK, OnSettingsChange)
	//MORPH END added by Yun.SF3: Ipfilter.dat update
	//Commander - Added: IP2Country Auto-updating - Start
	ON_EN_CHANGE(IDC_UPDATE_URL_IP2COUNTRY, OnSettingsChange)
    ON_BN_CLICKED(IDC_UPDATEIPCURL, OnBnClickedUpdateipcurl)
	ON_BN_CLICKED(IDC_RESETIPCURL, OnBnClickedResetipcurl)
	ON_BN_CLICKED(IDC_AUTOUPIP2COUNTRY , OnSettingsChange)
	ON_BN_CLICKED(IDC_AUTOUPCOUNTRYWEEK, OnSettingsChange)
	//Commander - Added: IP2Country Auto-updating - End
	ON_WM_HELPINFO()
END_MESSAGE_MAP()
// CPPgMorph message handlers

BOOL CPPgMorph2::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	LoadSettings();
	InitTooltips(); //MORPH leuk_he tolltipped
	Localize();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

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

void CPPgMorph2::LoadSettings(void)
{
	//MORPH START - Added by milobac and Yun.SF3, FakeCheck, FakeReport, Auto-updating
	GetDlgItem(IDC_UPDATE_URL_FAKELIST)->SetWindowText(thePrefs.UpdateURLFakeList);
	if(thePrefs.UpdateFakeStartup)
		CheckDlgButton(IDC_UPDATEFAKELISTSTART,1);
	else
		CheckDlgButton(IDC_UPDATEFAKELISTSTART,0);
    if (theApp.scheduler->HasWeekly(ACTION_UPDFAKES)) // check in schedule.
 		CheckDlgButton(IDC_UPDATEFAKELISTWEEK,1);
	else
		CheckDlgButton(IDC_UPDATEFAKELISTWEEK,0);
	//MORPH END - Added by milobac and Yun.SF3, FakeCheck, FakeReport, Auto-updating

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

	//Commander - Added: IP2Country Auto-updating - Start
	GetDlgItem(IDC_UPDATE_URL_IP2COUNTRY)->SetWindowText(thePrefs.UpdateURLIP2Country);
	if(thePrefs.AutoUpdateIP2Country)
		CheckDlgButton(IDC_AUTOUPIP2COUNTRY,1);
	else
		CheckDlgButton(IDC_AUTOUPIP2COUNTRY,0);
    if (theApp.scheduler->HasWeekly(ACTION_UPDCOUNTRY)) // check in schedule.
 		CheckDlgButton(IDC_AUTOUPCOUNTRYWEEK,1);
	else
		CheckDlgButton(IDC_AUTOUPCOUNTRYWEEK,0);
	//Commander - Added: IP2Country Auto-updating - End

	//Commander - Added: IP2Country Auto-updating - Start
	TCHAR sTime[30];
	sTime[0] = _T('\0');
	SysTimeToStr(thePrefs.GetIP2CountryVersion(), sTime);
	GetDlgItem(IDC_IP2COUNTRY_VERSION)->SetWindowText(sTime);
	//Commander - Added: IP2Country Auto-updating - End

	//Commander - Added: IP2Country Auto-updating - Start
	sTime[0] = _T('\0');
	SysTimeToStr(thePrefs.GetFakesDatVersion(), sTime);
	GetDlgItem(IDC_FAKELIST_VERSION)->SetWindowText(sTime);
	//Commander - Added: IP2Country Auto-updating - End

	//MORPH START - Added by Stulle, New IP Filter by Ozzy [Stulle/Ozzy]
	/*
	//Commander - Added: IP2Country Auto-updating - Start
	sTime[0] = _T('\0');
	SysTimeToStr(thePrefs.GetIPfilterVersion(), sTime);
	GetDlgItem(IDC_IPFILTER_VERSION)->SetWindowText(sTime);
	//Commander - Added: IP2Country Auto-updating - End
	*/
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
}

BOOL CPPgMorph2::OnApply()
{

	CString buffer;
	
	//MORPH START - Added by milobac and Yun.SF3, FakeCheck, FakeReport, Auto-updating
	GetDlgItem(IDC_UPDATE_URL_FAKELIST)->GetWindowText(buffer);
	_tcscpy(thePrefs.UpdateURLFakeList, buffer);
	thePrefs.UpdateFakeStartup = IsDlgButtonChecked(IDC_UPDATEFAKELISTSTART)!=0;
	theApp.scheduler->SetWeekly(ACTION_UPDFAKES,IsDlgButtonChecked(IDC_UPDATEFAKELISTWEEK)!=0);
	//MORPH END   - Added by milobac and Yun.SF3, FakeCheck, FakeReport, Auto-updating

	//MORPH START - Added by Yun.SF3: Ipfilter.dat update
	GetDlgItem(IDC_UPDATE_URL_IPFILTER)->GetWindowText(buffer);
	_tcscpy(thePrefs.UpdateURLIPFilter, buffer);
	thePrefs.AutoUpdateIPFilter = IsDlgButtonChecked(IDC_AUTOUPIPFILTER)!=0;
	theApp.scheduler->SetWeekly(ACTION_UPDIPCONF,IsDlgButtonChecked(IDC_AUTOUPIPFILTERWEEK)!=0);
	//MORPH END   - Added by Yun.SF3: Ipfilter.dat update

    //Commander - Added: IP2Country Auto-updating - Start
    GetDlgItem(IDC_UPDATE_URL_IP2COUNTRY)->GetWindowText(buffer);
	_tcscpy(thePrefs.UpdateURLIP2Country, buffer);
	thePrefs.AutoUpdateIP2Country = IsDlgButtonChecked(IDC_AUTOUPIP2COUNTRY)!=0;
	theApp.scheduler->SetWeekly(ACTION_UPDCOUNTRY,IsDlgButtonChecked(IDC_AUTOUPCOUNTRYWEEK)!=0);
	//Commander - Added: IP2Country Auto-updating - End

	LoadSettings();
	SetModified(FALSE);
	
	return CPropertyPage::OnApply();
}
void CPPgMorph2::Localize(void)
{
	if(m_hWnd)
	{
		//SetWindowText(GetResString(IDS_MORPH2));
		
		//MORPH START - Added by milobac and Yun.SF3, FakeCheck, FakeReport, Auto-updating
		GetDlgItem(IDC_MORPH2_FILE)->SetWindowText(GetResString(IDS_FILES));
		GetDlgItem(IDC_UPDATEFAKELISTSTART)->SetWindowText(GetResString(IDS_UPDATEFAKECHECKONSTART));
		GetDlgItem(IDC_UPDATEFAKES)->SetWindowText(GetResString(IDS_UPDATEFAKES));
		GetDlgItem(IDC_URL_FOR_UPDATING)->SetWindowText(GetResString(IDS_URL_FOR_UPDATING));
		GetDlgItem(IDC_RESETFAKESURL)->SetWindowText(GetResString(IDS_RESET));
		GetDlgItem(IDC_UPDATEFAKELISTWEEK)->SetWindowText(GetResString(IDS_UPDATEFAKELISTWEEK));
		//MORPH END   - Added by milobac and Yun.SF3, FakeCheck, FakeReport, Auto-updating
		
		//MORPH START - Added by Yun.SF3: Ipfilter.dat update
		GetDlgItem(IDC_MORPH2_SECURITY)->SetWindowText(GetResString(IDS_SECURITY));
		GetDlgItem(IDC_AUTOUPIPFILTER)->SetWindowText(GetResString(IDS_UPDATEIPFILTERONSTART));
		GetDlgItem(IDC_UPDATEIPFURL)->SetWindowText(GetResString(IDS_UPDATEIPCURL));
		GetDlgItem(IDC_URL_FOR_UPDATING2)->SetWindowText(GetResString(IDS_URL_FOR_UPDATING));
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

		//MORPH START - Added by Commander: IP2Country update
		GetDlgItem(IDC_MORPH2_COUNTRY)->SetWindowText(GetResString(IDS_COUNTRY));
		GetDlgItem(IDC_AUTOUPIP2COUNTRY)->SetWindowText(GetResString(IDS_AUTOUPIP2COUNTRY));
		GetDlgItem(IDC_UPDATEIPCURL)->SetWindowText(GetResString(IDS_UPDATEIPCURL));
		GetDlgItem(IDC_URL_FOR_UPDATING_IP2COUNTRY)->SetWindowText(GetResString(IDS_URL_FOR_UPDATING_IP2COUNTRY));
		GetDlgItem(IDC_RESETIPCURL)->SetWindowText(GetResString(IDS_RESET));
		GetDlgItem(IDC_AUTOUPCOUNTRYWEEK)->SetWindowText(GetResString(IDS_AUTOUPCOUNTRYWEEK));
		//MORPH END - Added by Commander: IP2Country update
        // MORPH START leuk_he tooltipped
		//SetTool(IDC_MORPH2_FILE,IDC_MORPH2_FILE_TIP);
		SetTool(IDC_UPDATEFAKELISTSTART,IDC_UPDATEFAKELISTSTART_TIP);
		SetTool(IDC_UPDATEFAKES,IDC_UPDATEFAKES_TIP);	  
		SetTool(IDC_UPDATE_URL_FAKELIST,IDC_UPDATE_URL_FAKELIST_TIP );
		SetTool(IDC_RESETFAKESURL,IDC_RESETFAKESURL_TIP);
		SetTool(IDC_UPDATEFAKES,IDC_UPDATEFAKES_TIP);
        SetTool(IDC_UPDATEFAKELISTWEEK,IDS_UPDATEFAKELISTWEEK_TIP);		         
		//SetTool(IDC_MORPH2_SECURITY,IDC_MORPH2_SECURITY_TIP);
		SetTool(IDC_AUTOUPIPFILTER,IDC_AUTOUPIPFILTER_TIP);
		SetTool(IDC_UPDATEIPFURL,IDC_UPDATEIPFURL_TIP);
		SetTool(IDC_URL_FOR_UPDATING2,IDC_UPDATE_URL_IPFILTER_TIP);
		SetTool(IDC_RESETIPFURL,IDC_RESETIPFURL_TIP);
		SetTool(IDC_UPDATE_URL_IPFILTER,IDC_UPDATE_URL_IPFILTER_TIP);
		 SetTool(IDC_AUTOUPIPFILTERWEEK,IDS_UPDATEIPFILTERWEEK_TIP);
		//SetTool(IDC_MORPH2_COUNTRY,IDC_MORPH2_COUNTRY_TIP);
		SetTool(IDC_AUTOUPIP2COUNTRY,IDC_MORPH2_COUNTRY_TIP);
		SetTool(IDC_UPDATEIPCURL,IDC_UPDATEIPCURL_TIP);
		SetTool(IDC_URL_FOR_UPDATING_IP2COUNTRY,IDC_URL_FOR_UPDATING_IP2COUNTRY_TIP);
		SetTool(IDC_RESETIPCURL,IDC_RESETIPCURL_TIP);
		SetTool(IDC_UPDATE_URL_IP2COUNTRY,IDC_URL_FOR_UPDATING_IP2COUNTRY_TIP);
		SetTool(IDC_AUTOUPCOUNTRYWEEK,IDS_AUTOUPCOUNTRYWEEK_TIP);
		//  MORPH END leuk_he tooltipped  
	}
}

void CPPgMorph2::OnBnClickedUpdatefakes()
{
	OnApply();
	theApp.FakeCheck->DownloadFakeList();
	TCHAR sBuffer[30];
	sBuffer[0] = _T('\0'); 
	SysTimeToStr(thePrefs.GetFakesDatVersion(), sBuffer);
	GetDlgItem(IDC_FAKELIST_VERSION)->SetWindowText(sBuffer);
}

void CPPgMorph2::OnBnClickedResetfakes()
{
	CString strBuffer = _T("http://emulepawcio.sourceforge.net/fakes.zip");
	GetDlgItem(IDC_UPDATE_URL_FAKELIST)->SetWindowText(strBuffer);
	memset(thePrefs.GetFakesDatVersion(), 0, sizeof(SYSTEMTIME));
	GetDlgItem(IDC_FAKELIST_VERSION)->SetWindowText(_T(""));
}

//MORPH START added by Yun.SF3: Ipfilter.dat update
void CPPgMorph2::OnBnClickedUpdateipfurl()
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

void CPPgMorph2::OnBnClickedResetipfurl()
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

//Commander - Added: IP2Country Auto-updating - Start
void CPPgMorph2::OnBnClickedUpdateipcurl()
{
	OnApply();
	theApp.ip2country->UpdateIP2CountryURL();
	TCHAR sBuffer[30];
	sBuffer[0] = _T('\0'); 
	SysTimeToStr(thePrefs.GetIP2CountryVersion(), sBuffer);
	GetDlgItem(IDC_IP2COUNTRY_VERSION)->SetWindowText(sBuffer);
}

//Commander - Added: IP2Country Auto-updating - End
void CPPgMorph2::OnBnClickedResetipcurl()
{
	CString strBuffer = _T("http://ip-to-country.webhosting.info/downloads/ip-to-country.csv.zip");
	GetDlgItem(IDC_UPDATE_URL_IP2COUNTRY)->SetWindowText(strBuffer);
	memset(thePrefs.GetIP2CountryVersion(), 0, sizeof(SYSTEMTIME));
	GetDlgItem(IDC_IP2COUNTRY_VERSION)->SetWindowText(_T(""));
}

void CPPgMorph2::OnHelp()
{
	//theApp.ShowHelp(0);
}

BOOL CPPgMorph2::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgMorph2::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}
