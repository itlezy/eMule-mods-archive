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
#include "emule.h"
#include "PPgWebServer.h"
#include "otherfunctions.h"
#include "WebServer.h"
#include "MMServer.h"
#include "emuledlg.h"
#include "Preferences.h"
#include "ServerWnd.h"
#include "HelpIDs.h"
// ==> UPnP support [MoNKi] - leuk_he
/*
#include ".\ppgwebserver.h"
#include "UPnPImplWrapper.h"
#include "UPnPImpl.h"
*/
// <== UPnP support [MoNKi] - leuk_he
// ==> Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he/Stulle] - Stulle
#include "PreferencesDlg.h"
#include "MD5Sum.h"
// <== Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he/Stulle] - Stulle
#include "NTService.h" // Run eMule as NT Service [leuk_he/Stulle] - Stulle

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define HIDDEN_PASSWORD _T("*****")


IMPLEMENT_DYNAMIC(CPPgWebServer, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgWebServer, CPropertyPage)
	ON_EN_CHANGE(IDC_WSPASS, OnDataChange)
	ON_EN_CHANGE(IDC_WSPASSLOW, OnDataChange)
	ON_EN_CHANGE(IDC_WSPORT, OnDataChange)
	ON_EN_CHANGE(IDC_MMPASSWORDFIELD, OnDataChange)
	ON_EN_CHANGE(IDC_TMPLPATH, OnDataChange)
	ON_EN_CHANGE(IDC_MMPORT_FIELD, OnDataChange)
	ON_EN_CHANGE(IDC_WSTIMEOUT, OnDataChange)
	ON_BN_CLICKED(IDC_WSENABLED, OnEnChangeWSEnabled)
	ON_BN_CLICKED(IDC_WSENABLEDLOW, OnEnChangeWSEnabled)
	ON_BN_CLICKED(IDC_MMENABLED, OnEnChangeMMEnabled)
	ON_BN_CLICKED(IDC_WSRELOADTMPL, OnReloadTemplates)
	ON_BN_CLICKED(IDC_TMPLBROWSE, OnBnClickedTmplbrowse)
	ON_BN_CLICKED(IDC_WS_GZIP, OnDataChange)
	ON_BN_CLICKED(IDC_WS_ALLOWHILEVFUNC, OnDataChange)
	ON_BN_CLICKED(IDC_WSUPNP, OnDataChange)
	ON_WM_HELPINFO()
	ON_WM_DESTROY()
	// ==> Tabbed WebInterface settings panel [Stulle] - Stulle
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_WEBSERVER, OnTcnSelchangeTab)
	// <== Tabbed WebInterface settings panel [Stulle] - Stulle
	// ==> Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he/Stulle] - Stulle
	ON_CBN_SELCHANGE(IDC_ACCOUNTSELECT, UpdateSelection)
	ON_BN_CLICKED(IDC_ADVADMINENABLED,   OnEnableChange)
	ON_BN_CLICKED(IDC_ADVADMIN_KAD, OnSettingsChange)
	ON_BN_CLICKED(IDC_ADVADMIN_TRANSFER, OnSettingsChange)
	ON_BN_CLICKED(IDC_ADVADMIN_SEARCH, OnSettingsChange)
	ON_BN_CLICKED(IDC_ADVADMIN_SERVER, OnSettingsChange)
	ON_BN_CLICKED(IDC_ADVADMIN_SHARED, OnSettingsChange)
	ON_BN_CLICKED(IDC_ADVADMIN_STATS, OnSettingsChange)
	ON_BN_CLICKED(IDC_ADVADMIN_PREFS, OnSettingsChange)
	ON_BN_CLICKED(IDC_ADVADMIN_DFILES, OnSettingsChange)
	ON_CBN_SELCHANGE(IDC_ADVADMIN_USERLEVEL, OnSettingsChange)	
	ON_BN_CLICKED(IDC_ADVADMIN_DELETE, OnBnClickedDel)
	ON_BN_CLICKED(IDC_ADVADMIN_NEW, OnBnClickedNew)	
	ON_EN_CHANGE(IDC_ADVADMIN_PASS, OnMultiPWChange)
	ON_EN_CHANGE(IDC_ADVADMIN_CATS, OnMultiCatsChange)
//	ON_EN_CHANGE(IDC_ADVADMIN_USER, OnSettingsChange)
	// <== Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he/Stulle] - Stulle
	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	ON_BN_CLICKED(IDC_SVC_INSTALLSERVICE, OnBnClickedInstall)	
	ON_BN_CLICKED(IDC_SVC_SERVERUNINSTALL, OnBnClickedUnInstall)	
    ON_BN_CLICKED(IDC_SVC_STARTWITHSYSTEM, OnBnStartSystem)	
	ON_BN_CLICKED(IDC_SVC_MANUALSTART, OnBnManualStart)	
	ON_BN_CLICKED(IDC_SVC_SETTINGS ,   OnBnAllSettings)	
	ON_BN_CLICKED(IDC_SVC_RUNBROWSER , OnBnRunBRowser)	
	ON_BN_CLICKED(IDC_SVC_REPLACESERVICE , OnBnReplaceStart)	
	ON_CBN_SELCHANGE(IDC_SERVICE_OPT_BOX, OnCbnSelChangeOptLvl)
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle
	// ==> Adjustable NT Service Strings [Stulle] - Stulle
	ON_EN_CHANGE(IDC_SERVICE_NAME, OnDataChange)
	ON_EN_CHANGE(IDC_SERVICE_DISP_NAME, OnDataChange)
	ON_EN_CHANGE(IDC_SERVICE_DESCR, OnDataChange)
	// <== Adjustable NT Service Strings [Stulle] - Stulle
END_MESSAGE_MAP()

CPPgWebServer::CPPgWebServer()
	: CPropertyPage(CPPgWebServer::IDD)
{
	bCreated = false;
	m_icoBrowse = NULL;
	// ==> Tabbed WebInterface settings panel [Stulle] - Stulle
	m_imageList.DeleteImageList();
	m_imageList.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 14+1, 0);
	m_imageList.Add(CTempIconLoader(_T("WEB")));
	// <== Tabbed WebInterface settings panel [Stulle] - Stulle
}

CPPgWebServer::~CPPgWebServer()
{
}

void CPPgWebServer::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	// ==> Tabbed WebInterface settings panel [Stulle] - Stulle
	DDX_Control(pDX, IDC_TAB_WEBSERVER , m_tabCtr);
	// <== Tabbed WebInterface settings panel [Stulle] - Stulle
	// ==> Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he/Stulle] - Stulle
	DDX_Control(pDX, IDC_ACCOUNTSELECT, m_cbAccountSelector); 
	DDX_Control(pDX, IDC_ADVADMIN_USERLEVEL, m_cbUserlevel); 
	// <== Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he/Stulle] - Stulle
	DDX_Control(pDX, IDC_SERVICE_OPT_BOX, m_cbOptLvl); // Run eMule as NT Service [leuk_he/Stulle] - Stulle
}

BOOL CPPgWebServer::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	AddBuddyButton(GetDlgItem(IDC_TMPLPATH)->m_hWnd, ::GetDlgItem(m_hWnd, IDC_TMPLBROWSE));
	InitAttachedBrowseButton(::GetDlgItem(m_hWnd, IDC_TMPLBROWSE), m_icoBrowse);

	((CEdit*)GetDlgItem(IDC_WSPASS))->SetLimitText(12);
	((CEdit*)GetDlgItem(IDC_WSPORT))->SetLimitText(6);

	// ==> Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he/Stulle] - Stulle
	FillComboBox();
	FillUserlevelBox();
	m_cbAccountSelector.SetCurSel(0); // new account
	// <== Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he/Stulle] - Stulle

	InitOptLvlCbn(true); // Run eMule as NT Service [leuk_he/Stulle] - Stulle

	LoadSettings();
	Localize();

	OnEnChangeWSEnabled();

	// note: there are better classes to create a pure hyperlink, however since it is only needed here
	//		 we rather use an already existing class
	CRect rect;
	GetDlgItem(IDC_GUIDELINK)->GetWindowRect(rect);
	::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&rect, 2);
	m_wndMobileLink.CreateEx(NULL,0,_T("MsgWnd"),WS_BORDER | WS_VISIBLE | WS_CHILD | HTC_WORDWRAP | HTC_UNDERLINE_HOVER,rect.left,rect.top,rect.Width(),rect.Height(),m_hWnd,0);
	m_wndMobileLink.SetBkColor(::GetSysColor(COLOR_3DFACE)); // still not the right color, will fix this later (need to merge the .rc file before it changes ;) )
	m_wndMobileLink.SetFont(GetFont());
	if (!bCreated){
		bCreated = true;
		m_wndMobileLink.AppendText(_T("Link: "));
		m_wndMobileLink.AppendHyperLink(GetResString(IDS_MMGUIDELINK),0,CString(_T("http://mobil.emule-project.net")),0,0);
	}

	// ==> Tabbed WebInterface settings panel [Stulle] - Stulle
	m_currentTab = WEBSERVER;
	InitTab(true, theApp.emuledlg->preferenceswnd->m_WebServerTab);
	SetTab(theApp.emuledlg->preferenceswnd->m_WebServerTab);
	// <== Tabbed WebInterface settings panel [Stulle] - Stulle

	return TRUE;
}

void CPPgWebServer::LoadSettings(void)
{
	CString strBuffer;

	GetDlgItem(IDC_WSPASS)->SetWindowText(HIDDEN_PASSWORD);
	GetDlgItem(IDC_WSPASSLOW)->SetWindowText(HIDDEN_PASSWORD);
	GetDlgItem(IDC_MMPASSWORDFIELD)->SetWindowText(HIDDEN_PASSWORD);

	strBuffer.Format(_T("%d"), thePrefs.GetWSPort());
	GetDlgItem(IDC_WSPORT)->SetWindowText(strBuffer);

	strBuffer.Format(_T("%d"), thePrefs.GetMMPort());
	GetDlgItem(IDC_MMPORT_FIELD)->SetWindowText(strBuffer);

	GetDlgItem(IDC_TMPLPATH)->SetWindowText(thePrefs.GetTemplate());

	strBuffer.Format(_T("%d"), thePrefs.GetWebTimeoutMins());
	SetDlgItemText(IDC_WSTIMEOUT,strBuffer);

	if(thePrefs.GetWSIsEnabled())
		CheckDlgButton(IDC_WSENABLED,1);
	else
		CheckDlgButton(IDC_WSENABLED,0);

	if(thePrefs.GetWSIsLowUserEnabled())
		CheckDlgButton(IDC_WSENABLEDLOW,1);
	else
		CheckDlgButton(IDC_WSENABLEDLOW,0);

	if(thePrefs.IsMMServerEnabled())
		CheckDlgButton(IDC_MMENABLED,1);
	else
		CheckDlgButton(IDC_MMENABLED,0);

	CheckDlgButton(IDC_WS_GZIP,(thePrefs.GetWebUseGzip())? 1 : 0);
	CheckDlgButton(IDC_WS_ALLOWHILEVFUNC,(thePrefs.GetWebAdminAllowedHiLevFunc())? 1 : 0);

	GetDlgItem(IDC_WSUPNP)->EnableWindow(thePrefs.IsUPnPEnabled() && thePrefs.GetWSIsEnabled());
	CheckDlgButton(IDC_WSUPNP, (thePrefs.IsUPnPEnabled() && thePrefs.m_bWebUseUPnP) ? TRUE : FALSE);
	
	CheckDlgButton(IDC_ADVADMINENABLED, thePrefs.UseIonixWebsrv()); // Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he/Stulle] - Stulle

	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	if(thePrefs.GetServiceStartupMode()==2) {
			CheckDlgButton(IDC_SVC_RUNBROWSER   ,BST_UNCHECKED );
			CheckDlgButton(IDC_SVC_REPLACESERVICE, BST_CHECKED);
	} else {
			CheckDlgButton(IDC_SVC_RUNBROWSER   ,BST_CHECKED );
            CheckDlgButton(IDC_SVC_REPLACESERVICE, BST_UNCHECKED);
	}
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle

	// ==> Adjustable NT Service Strings [Stulle] - Stulle
	GetDlgItem(IDC_SERVICE_NAME)->SetWindowText(thePrefs.GetServiceName());
	GetDlgItem(IDC_SERVICE_DISP_NAME)->SetWindowText(thePrefs.GetServiceDispName());
	GetDlgItem(IDC_SERVICE_DESCR)->SetWindowText(thePrefs.GetServiceDescr());
	// <== Adjustable NT Service Strings [Stulle] - Stulle

	OnEnChangeMMEnabled();

	SetModified(FALSE);	// FoRcHa
}

BOOL CPPgWebServer::OnApply()
{	
	if(m_bModified)
	{
		CString sBuf;

		// get and check templatefile existance...
		GetDlgItem(IDC_TMPLPATH)->GetWindowText(sBuf);
		if ( IsDlgButtonChecked(IDC_WSENABLED) && !PathFileExists(sBuf)) {
			CString buffer;
			buffer.Format(GetResString(IDS_WEB_ERR_CANTLOAD),sBuf);
			AfxMessageBox(buffer,MB_OK);
			return FALSE;
		}
		thePrefs.SetTemplate(sBuf);
		theApp.webserver->ReloadTemplates();


		uint16 oldPort=thePrefs.GetWSPort();

		GetDlgItem(IDC_WSPASS)->GetWindowText(sBuf);
		if(sBuf != HIDDEN_PASSWORD)
			thePrefs.SetWSPass(sBuf);
		
		GetDlgItem(IDC_WSPASSLOW)->GetWindowText(sBuf);
		if(sBuf != HIDDEN_PASSWORD)
			thePrefs.SetWSLowPass(sBuf);

		GetDlgItem(IDC_WSPORT)->GetWindowText(sBuf);
		if (_tstoi(sBuf)!=oldPort) {
			thePrefs.SetWSPort((uint16)_tstoi(sBuf));
			theApp.webserver->RestartServer();
		}

		GetDlgItemText(IDC_WSTIMEOUT,sBuf);
		thePrefs.m_iWebTimeoutMins=_tstoi(sBuf);

		thePrefs.SetWSIsEnabled(IsDlgButtonChecked(IDC_WSENABLED)!=0);
		thePrefs.SetWSIsLowUserEnabled(IsDlgButtonChecked(IDC_WSENABLEDLOW)!=0);
		thePrefs.SetWebUseGzip(IsDlgButtonChecked(IDC_WS_GZIP)!=0);
		theApp.webserver->StartServer();
		thePrefs.m_bAllowAdminHiLevFunc= (IsDlgButtonChecked(IDC_WS_ALLOWHILEVFUNC)!=0);

		// mobilemule
		GetDlgItem(IDC_MMPORT_FIELD)->GetWindowText(sBuf);
		if (_tstoi(sBuf)!= thePrefs.GetMMPort() ) {
			thePrefs.SetMMPort((uint16)_tstoi(sBuf));
			theApp.mmserver->StopServer();
			theApp.mmserver->Init();
		}
		thePrefs.SetMMIsEnabled(IsDlgButtonChecked(IDC_MMENABLED)!=0);
		if (IsDlgButtonChecked(IDC_MMENABLED))
			theApp.mmserver->Init();
		else
			theApp.mmserver->StopServer();
		GetDlgItem(IDC_MMPASSWORDFIELD)->GetWindowText(sBuf);
		if(sBuf != HIDDEN_PASSWORD)
			thePrefs.SetMMPass(sBuf);
		
		// ==> UPnP support [MoNKi] - leuk_he
		/*
		if (IsDlgButtonChecked(IDC_WSUPNP))
		{
			ASSERT( thePrefs.IsUPnPEnabled() );
			if (!thePrefs.m_bWebUseUPnP && thePrefs.GetWSIsEnabled() && theApp.m_pUPnPFinder != NULL) // add the port to existing mapping without having eMule restarting (if all conditions are met)
				theApp.m_pUPnPFinder->GetImplementation()->LateEnableWebServerPort(thePrefs.GetWSPort());
			thePrefs.m_bWebUseUPnP = true;
		}
		else
			thePrefs.m_bWebUseUPnP = false;
		*/
		if ((UINT)thePrefs.GetUPnPNatWeb() != IsDlgButtonChecked(IDC_WSUPNP))
		{
			theApp.m_UPnP_IGDControlPoint->SetUPnPNat(thePrefs.IsUPnPEnabled()); // and start/stop nat. 
			thePrefs.SetUPnPNatWeb(IsDlgButtonChecked(IDC_WSUPNP)!=0);
		}
		// <== UPnP support [MoNKi] - leuk_he

		// ==> Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he/Stulle] - Stulle
		theApp.webserver->SaveWebServConf();
		// <== Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he/Stulle] - Stulle

		// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
		int b_installed;
		int  i_startupmode;
		int rights;
		// Startup with system, store in service.
		NTServiceGet(b_installed,i_startupmode,	rights);

		// ==> Adjustable NT Service Strings [Stulle] - Stulle
		CString strServiceName, strServiceDispName, strServiceDescr;
		GetDlgItem(IDC_SERVICE_NAME)->GetWindowText(strServiceName);
		GetDlgItem(IDC_SERVICE_DISP_NAME)->GetWindowText(strServiceDispName);
		GetDlgItem(IDC_SERVICE_DESCR)->GetWindowText(strServiceDescr);

		int iChangedStr = 0; // nothing changed
		if(strServiceName.Compare(thePrefs.GetServiceName()) != 0)
			iChangedStr = 1; // name under which we install changed, this is important!
		else if((strServiceDispName.Compare(thePrefs.GetServiceDispName()) != 0) || (strServiceDescr.Compare(thePrefs.GetServiceDescr()) != 0))
			iChangedStr = 2; // only visual strings changed, not so important...

		if(iChangedStr>0)
		{
			if(b_installed == 0)
			{
				thePrefs.SetServiceName(strServiceName);
				thePrefs.SetServiceDispName(strServiceDispName);
				thePrefs.SetServiceDescr(strServiceDescr);
				FillStatus();
			}
			else
			{
				int iResult = IDCANCEL;
				if(iChangedStr == 1)
					iResult = MessageBox(GetResString(IDS_SERVICE_NAME_CHANGED),GetResString(IDS_SERVICE_STR_CHANGED),MB_YESNOCANCEL|MB_ICONQUESTION|MB_DEFBUTTON3);
				else if(iChangedStr == 2)
				{
					if(NTServiceChangeDisplayStrings(strServiceDispName,strServiceDescr) != 0)
					{
						if(MessageBox(GetResString(IDS_SERVICE_DISP_CHANGE_FAIL),GetResString(IDS_SERVICE_STR_CHANGE_FAIL),MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2) == IDYES)
						{
							iChangedStr = 1;
							iResult = IDYES;
						}
					}
					else
						iResult = IDNO;
				}

				if(iChangedStr == 1 && iResult == IDYES) // reinstall service
				{
					if(CmdRemoveService()==0)
					{
						thePrefs.SetServiceName(strServiceName);
						thePrefs.SetServiceDispName(strServiceDispName);
						thePrefs.SetServiceDescr(strServiceDescr);
						if(CmdInstallService(i_startupmode == 1) != 0)
							MessageBox(GetResString(IDS_SERVICE_INSTALL_FAIL), GetResString(IDS_SERVICE_INSTALL_TITLE), MB_OK|MB_ICONWARNING);
					}
					else
					{
						MessageBox(GetResString(IDS_SERVICE_UNINSTALL_FAIL),GetResString(IDS_SERVICE_UNINSTALL_TITLE),MB_OK|MB_ICONWARNING);
						GetDlgItem(IDC_SERVICE_NAME)->SetWindowText(thePrefs.GetServiceName());
						GetDlgItem(IDC_SERVICE_DISP_NAME)->SetWindowText(thePrefs.GetServiceDispName());
						GetDlgItem(IDC_SERVICE_DESCR)->SetWindowText(thePrefs.GetServiceDescr());
					}
					FillStatus();
				}
				else if(iResult == IDNO) // just save settings
				{
					thePrefs.SetServiceName(strServiceName);
					thePrefs.SetServiceDispName(strServiceDispName);
					thePrefs.SetServiceDescr(strServiceDescr);
					FillStatus();
				}
				else // revert settings
				{
					GetDlgItem(IDC_SERVICE_NAME)->SetWindowText(thePrefs.GetServiceName());
					GetDlgItem(IDC_SERVICE_DISP_NAME)->SetWindowText(thePrefs.GetServiceDispName());
					GetDlgItem(IDC_SERVICE_DESCR)->SetWindowText(thePrefs.GetServiceDescr());
				}
			}
		}
		// <== Adjustable NT Service Strings [Stulle] - Stulle

		if (b_installed==1 && 
				(i_startupmode ==0 && (IsDlgButtonChecked(IDC_SVC_STARTWITHSYSTEM)==BST_CHECKED))||
				(i_startupmode ==1 && (IsDlgButtonChecked(IDC_SVC_MANUALSTART)==BST_CHECKED)))
			NTServiceSetStartupMode(IsDlgButtonChecked(IDC_SVC_STARTWITHSYSTEM)==BST_CHECKED);
	   // TODO: Apply setting 
		if ( IsDlgButtonChecked(IDC_SVC_RUNBROWSER)==BST_CHECKED)
		   thePrefs.m_iServiceStartupMode=1;
		else 
		   thePrefs.m_iServiceStartupMode=2;

		int iSel = m_cbOptLvl.GetCurSel();
		thePrefs.m_iServiceOptLvl = m_cbOptLvl.GetItemData(iSel);
		// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle

		theApp.emuledlg->serverwnd->UpdateMyInfo();
		SetModified(FALSE);
		SetTmplButtonState();
	}

	return CPropertyPage::OnApply();
}

void CPPgWebServer::Localize(void)
{
	if(m_hWnd){
		SetWindowText(GetResString(IDS_PW_WS));
		GetDlgItem(IDC_WSPASS_LBL)->SetWindowText(GetResString(IDS_WS_PASS));
		GetDlgItem(IDC_WSPORT_LBL)->SetWindowText(GetResString(IDS_PORT));
		GetDlgItem(IDC_WSENABLED)->SetWindowText(GetResString(IDS_ENABLED));
		GetDlgItem(IDC_WSRELOADTMPL)->SetWindowText(GetResString(IDS_SF_RELOAD));
		GetDlgItem(IDC_WSENABLED)->SetWindowText(GetResString(IDS_ENABLED));
		SetDlgItemText(IDC_WS_GZIP,GetResString(IDS_WEB_GZIP_COMPRESSION));
		SetDlgItemText(IDC_WSUPNP, GetResString(IDS_WEBUPNPINCLUDE));

		GetDlgItem(IDC_WSPASS_LBL2)->SetWindowText(GetResString(IDS_WS_PASS));
		GetDlgItem(IDC_WSENABLEDLOW)->SetWindowText(GetResString(IDS_ENABLED));
		GetDlgItem(IDC_STATIC_GENERAL)->SetWindowText(GetResString(IDS_PW_GENERAL));

		GetDlgItem(IDC_STATIC_ADMIN)->SetWindowText(GetResString(IDS_ADMIN));
		GetDlgItem(IDC_STATIC_LOWUSER)->SetWindowText(GetResString(IDS_WEB_LOWUSER));
		GetDlgItem(IDC_WSENABLEDLOW)->SetWindowText(GetResString(IDS_ENABLED));

		GetDlgItem(IDC_TEMPLATE)->SetWindowText(GetResString(IDS_WS_RELOAD_TMPL));
		SetDlgItemText(IDC_WSTIMEOUTLABEL,GetResString(IDS_WEB_SESSIONTIMEOUT)+_T(":"));
		SetDlgItemText(IDC_MINS,GetResString(IDS_LONGMINS) );

		GetDlgItem(IDC_MMENABLED)->SetWindowText(GetResString(IDS_ENABLEMM));
		GetDlgItem(IDC_STATIC_MOBILEMULE)->SetWindowText(GetResString(IDS_MOBILEMULE));
		GetDlgItem(IDC_MMPASSWORD)->SetWindowText(GetResString(IDS_WS_PASS));
		GetDlgItem(IDC_MMPORT_LBL)->SetWindowText(GetResString(IDS_PORT));

		GetDlgItem(IDC_WS_ALLOWHILEVFUNC)->SetWindowText(GetResString(IDS_WEB_ALLOWHILEVFUNC));

		// ==> Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he/Stulle] - Stulle
		GetDlgItem(IDC_ADVADMINENABLED)->SetWindowText(GetResString(IDS_ADVADMINENABLED));
		GetDlgItem(IDC_ADVADMIN_NOTE)->SetWindowText(GetResString(IDS_ADVADMIN_NOTE));
		GetDlgItem(IDC_STATIC_ADVADMIN)->SetWindowText(GetResString(IDS_ADVADMIN_GROUP));
		GetDlgItem(IDC_STATIC_ADVADMIN_ACC)->SetWindowText(GetResString(IDS_ADVADMIN_ACC));
		GetDlgItem(IDC_ADVADMIN_DELETE)->SetWindowText(GetResString(IDS_ADVADMIN_DELETE));
		GetDlgItem(IDC_ADVADMIN_NEW)->SetWindowText(GetResString(IDS_ADVADMIN_NEW));
		GetDlgItem(IDC_ADVADMIN_KAD)->SetWindowText(GetResString(IDS_ADVADMIN_KAD));
		GetDlgItem(IDC_ADVADMIN_TRANSFER)->SetWindowText(GetResString(IDS_ADVADMIN_TRANSFER));
		GetDlgItem(IDC_ADVADMIN_SEARCH)->SetWindowText(GetResString(IDS_ADVADMIN_SEARCH));
		GetDlgItem(IDC_ADVADMIN_SERVER)->SetWindowText(GetResString(IDS_ADVADMIN_SERVER));
		GetDlgItem(IDC_ADVADMIN_SHARED)->SetWindowText(GetResString(IDS_ADVADMIN_SHARED));
		GetDlgItem(IDC_ADVADMIN_STATS)->SetWindowText(GetResString(IDS_ADVADMIN_STATS));
		GetDlgItem(IDC_ADVADMIN_PREFS)->SetWindowText(GetResString(IDS_ADVADMIN_PREFS));
		GetDlgItem(IDC_ADVADMIN_DFILES)->SetWindowText(GetResString(IDS_ADVADMIN_DFILES));
		GetDlgItem(IDC_STATIC_ADVADMIN_USERLEVEL)->SetWindowText(GetResString(IDS_ADVADMIN_USERLEVEL));
		GetDlgItem(IDC_STATIC_ADVADMIN_PASS)->SetWindowText(GetResString(IDS_ADVADMIN_PASS));
		GetDlgItem(IDC_STATIC_ADVADMIN_USER)->SetWindowText(GetResString(IDS_ADVADMIN_USER));
		GetDlgItem(IDC_STATIC_ADVADMIN_CATS)->SetWindowText(GetResString(IDS_ADVADMIN_CAT));
		// <== Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he/Stulle] - Stulle

		// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
		GetDlgItem(IDC_SVC_INSTALLSERVICE)->SetWindowText(GetResString(IDS_SVC_INSTALLSERVICE));
		GetDlgItem(IDC_SVC_SERVERUNINSTALL)->SetWindowText(GetResString(IDS_SVC_SERVERUNINSTALL));
		GetDlgItem(IDC_SVC_STARTWITHSYSTEM)->SetWindowText(GetResString(IDS_SVC_STARTWITHSYSTEM));
		GetDlgItem(IDC_SVC_MANUALSTART)->SetWindowText(GetResString(IDS_SVC_MANUALSTART));
		GetDlgItem(IDC_SVC_SETTINGS)->SetWindowText(GetResString(IDS_SVC_SETTINGS));
		GetDlgItem(IDC_SVC_RUNBROWSER)->SetWindowText(GetResString(IDS_SVC_RUNBROWSER));
		GetDlgItem(IDC_SVC_REPLACESERVICE )->SetWindowText(GetResString(IDS_SVC_REPLACESERVICE));
		GetDlgItem(IDC_SVC_ONSTARTBOX)->SetWindowText(GetResString(IDS_SVC_ONSTARTBOX));
		GetDlgItem(IDC_SVC_STARTUPBOX)->SetWindowText(GetResString(IDS_SVC_STARTUPBOX));
		GetDlgItem(IDC_SVC_CURRENT_STATUS_LABEL)->SetWindowText(GetResString(IDS_SVC_CURRENT_STATUS_LABEL));
		GetDlgItem(IDC_SERVICE_OPT_GROUP)->SetWindowText(GetResString(IDS_SERVICE_OPT_GROUP));
		GetDlgItem(IDC_SERVICE_OPT_LABEL)->SetWindowText(GetResString(IDS_SERVICE_OPT_LABEL));
		InitOptLvlCbn();
		// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle

		// ==> Adjustable NT Service Strings [Stulle] - Stulle
		GetDlgItem(IDC_SERVICE_STR_GROUP)->SetWindowText(GetResString(IDS_SERVICE_STR_GROUP));
		GetDlgItem(IDC_SERVICE_NAME_LABEL)->SetWindowText(GetResString(IDS_SERVICE_NAME));
		GetDlgItem(IDC_SERVICE_DISP_NAME_LABEL)->SetWindowText(GetResString(IDS_SERVICE_DISP_NAME));
		GetDlgItem(IDC_SERVICE_DESCR_LABEL)->SetWindowText(GetResString(IDS_SERVICE_DESCR));
		// <== Adjustable NT Service Strings [Stulle] - Stulle
	}
}

void CPPgWebServer::OnEnChangeWSEnabled()
{
	// ==> Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he/Stulle] - Stulle
	/*
	UINT bIsWIEnabled=IsDlgButtonChecked(IDC_WSENABLED);
	GetDlgItem(IDC_WSPASS)->EnableWindow(bIsWIEnabled);	
	GetDlgItem(IDC_WSPORT)->EnableWindow(bIsWIEnabled);	
	GetDlgItem(IDC_WSENABLEDLOW)->EnableWindow(bIsWIEnabled);
	GetDlgItem(IDC_TMPLPATH)->EnableWindow(bIsWIEnabled);
	GetDlgItem(IDC_TMPLBROWSE)->EnableWindow(bIsWIEnabled);
	GetDlgItem(IDC_WS_GZIP)->EnableWindow(bIsWIEnabled);
	GetDlgItem(IDC_WS_ALLOWHILEVFUNC)->EnableWindow(bIsWIEnabled);
	GetDlgItem(IDC_WSTIMEOUT)->EnableWindow(bIsWIEnabled);
	GetDlgItem(IDC_WSPASSLOW)->EnableWindow(bIsWIEnabled && IsDlgButtonChecked(IDC_WSENABLEDLOW));
	GetDlgItem(IDC_WSUPNP)->EnableWindow(thePrefs.IsUPnPEnabled() && bIsWIEnabled);
	*/
	bool bSingleWSEnalbed = IsDlgButtonChecked(IDC_WSENABLED) && theApp.webserver->iMultiUserversion <= 0;

	if(bSingleWSEnalbed)
	{
		GetDlgItem(IDC_WSPASS)->EnableWindow(TRUE);
		GetDlgItem(IDC_WS_ALLOWHILEVFUNC)->EnableWindow(TRUE);
		GetDlgItem(IDC_WSPASSLOW)->EnableWindow(TRUE);
		GetDlgItem(IDC_WSENABLEDLOW)->EnableWindow(TRUE);
	}
	else
	{	
		GetDlgItem(IDC_WSPASS)->EnableWindow(FALSE);
		GetDlgItem(IDC_WS_ALLOWHILEVFUNC)->EnableWindow(FALSE);
		GetDlgItem(IDC_WSPASSLOW)->EnableWindow(FALSE);
		GetDlgItem(IDC_WSENABLEDLOW)->EnableWindow(FALSE);
	}

	UINT bIsWIEnabled=IsDlgButtonChecked(IDC_WSENABLED);
	GetDlgItem(IDC_WSPORT)->EnableWindow(bIsWIEnabled);	
	GetDlgItem(IDC_TMPLPATH)->EnableWindow(bIsWIEnabled);
	GetDlgItem(IDC_TMPLBROWSE)->EnableWindow(bIsWIEnabled);
	GetDlgItem(IDC_WS_GZIP)->EnableWindow(bIsWIEnabled);
	GetDlgItem(IDC_WSTIMEOUT)->EnableWindow(bIsWIEnabled);
	GetDlgItem(IDC_WSUPNP)->EnableWindow(thePrefs.IsUPnPEnabled() && bIsWIEnabled);
	// <== Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he/Stulle] - Stulle
	
	//GetDlgItem(IDC_WSRELOADTMPL)->EnableWindow(bIsWIEnabled);
	SetTmplButtonState();


	SetModified();
}

void CPPgWebServer::OnEnChangeMMEnabled()
{
	GetDlgItem(IDC_MMPASSWORDFIELD)->EnableWindow(IsDlgButtonChecked(IDC_MMENABLED));	
	GetDlgItem(IDC_MMPORT_FIELD)->EnableWindow(IsDlgButtonChecked(IDC_MMENABLED));

	SetModified();
}

void CPPgWebServer::OnReloadTemplates()
{
	theApp.webserver->ReloadTemplates();
}

void CPPgWebServer::OnBnClickedTmplbrowse()
{
	CString strTempl;
	GetDlgItemText(IDC_TMPLPATH, strTempl);
	CString buffer;
	buffer=GetResString(IDS_WS_RELOAD_TMPL)+_T("(*.tmpl)|*.tmpl||");
    if (DialogBrowseFile(buffer, _T("Template ")+buffer, strTempl)){
		GetDlgItem(IDC_TMPLPATH)->SetWindowText(buffer);
		SetModified();
	}
	SetTmplButtonState();
}

void CPPgWebServer::SetTmplButtonState(){
	CString buffer;
	GetDlgItemText(IDC_TMPLPATH,buffer);

	GetDlgItem(IDC_WSRELOADTMPL)->EnableWindow( thePrefs.GetWSIsEnabled() && (buffer.CompareNoCase(thePrefs.GetTemplate())==0));
}

void CPPgWebServer::OnHelp()
{
	theApp.ShowHelp(eMule_FAQ_Preferences_WebInterface);
}

BOOL CPPgWebServer::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgWebServer::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}

void CPPgWebServer::OnDestroy()
{
	CPropertyPage::OnDestroy();
	if (m_icoBrowse)
	{
		VERIFY( DestroyIcon(m_icoBrowse) );
		m_icoBrowse = NULL;
	}
}

// ==> Tabbed WebInterface settings panel [Stulle] - Stulle
void CPPgWebServer::InitTab(bool firstinit, int Page)
{
	if (firstinit) {
		m_tabCtr.DeleteAllItems();
		m_tabCtr.SetImageList(&m_imageList);
		m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, WEBSERVER, GetResString(IDS_TAB_WEB_SERVER), 0, (LPARAM)WEBSERVER); 
		m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, MULTIWEBSERVER, GetResString(IDS_TAB_MULTI_USER), 0, (LPARAM)MULTIWEBSERVER); 
		m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, NTSERVICE, GetResString(IDS_TAB_NT_SERVICE), 0, (LPARAM)NTSERVICE); 
	}

	if (m_tabCtr.GetSafeHwnd() != NULL)
		m_tabCtr.SetCurSel(Page);
}
void CPPgWebServer::OnTcnSelchangeTab(NMHDR * /* pNMHDR */, LRESULT *pResult)
{
	// Retrieve tab to display
	TCITEM tabCtrlItem; 
	tabCtrlItem.mask = TCIF_PARAM;
	if(m_tabCtr.GetItem(m_tabCtr.GetCurSel(), &tabCtrlItem) == TRUE){
		SetTab(static_cast<eTab>(tabCtrlItem.lParam));
	}

	*pResult = 0;
}

void CPPgWebServer::SetTab(eTab tab){
	if(m_currentTab != tab){
		// Hide all control
		switch(m_currentTab){
			case WEBSERVER:
				GetDlgItem(IDC_WSENABLED)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_WSENABLED)->EnableWindow(FALSE);
				GetDlgItem(IDC_WS_GZIP)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_WS_GZIP)->EnableWindow(FALSE);
				GetDlgItem(IDC_WSPORT)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_WSPORT)->EnableWindow(FALSE);
				GetDlgItem(IDC_TMPLPATH)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_TMPLPATH)->EnableWindow(FALSE);
				GetDlgItem(IDC_TMPLBROWSE)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_TMPLBROWSE)->EnableWindow(FALSE);
				GetDlgItem(IDC_WSRELOADTMPL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_WSRELOADTMPL)->EnableWindow(FALSE);
				GetDlgItem(IDC_WSTIMEOUT)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_WSTIMEOUT)->EnableWindow(FALSE);
				GetDlgItem(IDC_WSPASS)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_WSPASS)->EnableWindow(FALSE);
				GetDlgItem(IDC_WS_ALLOWHILEVFUNC)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_WS_ALLOWHILEVFUNC)->EnableWindow(FALSE);
				GetDlgItem(IDC_WSENABLEDLOW)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_WSENABLEDLOW)->EnableWindow(FALSE);
				GetDlgItem(IDC_WSPASSLOW)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_WSPASSLOW)->EnableWindow(FALSE);
				GetDlgItem(IDC_MMENABLED)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_MMENABLED)->EnableWindow(FALSE);
				GetDlgItem(IDC_MMPASSWORDFIELD)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_MMPASSWORDFIELD)->EnableWindow(FALSE);
				GetDlgItem(IDC_MMPORT_FIELD)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_MMPORT_FIELD)->EnableWindow(FALSE);
				GetDlgItem(IDC_GUIDELINK)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_GUIDELINK)->EnableWindow(FALSE);
				GetDlgItem(IDC_STATIC_GENERAL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_STATIC_GENERAL)->EnableWindow(FALSE);
				GetDlgItem(IDC_WSPORT_LBL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_WSPORT_LBL)->EnableWindow(FALSE);
				GetDlgItem(IDC_TEMPLATE)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_TEMPLATE)->EnableWindow(FALSE);
				GetDlgItem(IDC_STATIC_ADMIN)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_STATIC_ADMIN)->EnableWindow(FALSE);
				GetDlgItem(IDC_WSPASS_LBL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_WSPASS_LBL)->EnableWindow(FALSE);
				GetDlgItem(IDC_STATIC_LOWUSER)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_STATIC_LOWUSER)->EnableWindow(FALSE);
				GetDlgItem(IDC_WSPASS_LBL2)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_WSPASS_LBL2)->EnableWindow(FALSE);
				GetDlgItem(IDC_STATIC_MOBILEMULE)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_STATIC_MOBILEMULE)->EnableWindow(FALSE);
				GetDlgItem(IDC_MMPASSWORD)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_MMPASSWORD)->EnableWindow(FALSE);
				GetDlgItem(IDC_MMPORT_LBL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_MMPORT_LBL)->EnableWindow(FALSE);
				GetDlgItem(IDC_WSTIMEOUTLABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_WSTIMEOUTLABEL)->EnableWindow(FALSE);
				GetDlgItem(IDC_MINS)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_MINS)->EnableWindow(FALSE);
				GetDlgItem(IDC_WSUPNP)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_WSUPNP)->EnableWindow(FALSE);
				if(m_wndMobileLink.GetSafeHwnd())
				{
					m_wndMobileLink.ShowWindow(SW_HIDE);
					m_wndMobileLink.EnableWindow(FALSE);
				}
				break;
			// ==> Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he/Stulle] - Stulle
			case MULTIWEBSERVER:
				GetDlgItem(IDC_ADVADMINENABLED)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMINENABLED)->EnableWindow(FALSE);
				GetDlgItem(IDC_STATIC_ADVADMIN)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_STATIC_ADVADMIN)->EnableWindow(FALSE);
				GetDlgItem(IDC_ACCOUNTSELECT)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ACCOUNTSELECT)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_USER)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_USER)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_PASS)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_PASS)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_KAD)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_KAD)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_TRANSFER)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_TRANSFER)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_SEARCH)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_SEARCH)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_SERVER)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_SERVER)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_SHARED)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_SHARED)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_STATS)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_STATS)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_PREFS)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_PREFS)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_DFILES)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_DFILES)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_USERLEVEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_USERLEVEL)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_CATS)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_CATS)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_DELETE)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_DELETE)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_NEW)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_NEW)->EnableWindow(FALSE);
				GetDlgItem(IDC_STATIC_ADVADMIN_ACC)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_STATIC_ADVADMIN_ACC)->EnableWindow(FALSE);
				GetDlgItem(IDC_STATIC_ADVADMIN_CATS)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_STATIC_ADVADMIN_CATS)->EnableWindow(FALSE);
				GetDlgItem(IDC_STATIC_ADVADMIN_USER)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_STATIC_ADVADMIN_USER)->EnableWindow(FALSE);
				GetDlgItem(IDC_STATIC_ADVADMIN_PASS)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_STATIC_ADVADMIN_PASS)->EnableWindow(FALSE);
				GetDlgItem(IDC_STATIC_ADVADMIN_USERLEVEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_STATIC_ADVADMIN_USERLEVEL)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_NOTE)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_NOTE)->EnableWindow(FALSE);
				break;
			// <== Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he/Stulle] - Stulle
			// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
			case NTSERVICE:
				GetDlgItem(IDC_SVC_CURRENT_STATUS_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SVC_CURRENT_STATUS_LABEL)->EnableWindow(FALSE);
				GetDlgItem(IDC_SVC_CURRENT_STATUS)->ShowWindow(SW_HIDE);
				//GetDlgItem(IDC_SVC_CURRENT_STATUS)->EnableWindow(FALSE);
				GetDlgItem(IDC_SVC_INSTALLSERVICE)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SVC_INSTALLSERVICE)->EnableWindow(FALSE);
				GetDlgItem(IDC_SVC_SERVERUNINSTALL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SVC_SERVERUNINSTALL)->EnableWindow(FALSE);
				GetDlgItem(IDC_SVC_STARTUPBOX)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SVC_STARTUPBOX)->EnableWindow(FALSE);
				GetDlgItem(IDC_SVC_STARTWITHSYSTEM)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SVC_STARTWITHSYSTEM)->EnableWindow(FALSE);
				GetDlgItem(IDC_SVC_MANUALSTART)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SVC_MANUALSTART)->EnableWindow(FALSE);
				GetDlgItem(IDC_SVC_SETTINGS)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SVC_SETTINGS)->EnableWindow(FALSE);
				GetDlgItem(IDC_SVC_ONSTARTBOX)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SVC_ONSTARTBOX)->EnableWindow(FALSE);
				GetDlgItem(IDC_SVC_RUNBROWSER)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SVC_RUNBROWSER)->EnableWindow(FALSE);
				GetDlgItem(IDC_SVC_REPLACESERVICE)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SVC_REPLACESERVICE)->EnableWindow(FALSE);
				// ==> Adjustable NT Service Strings [Stulle] - Stulle
				GetDlgItem(IDC_SERVICE_STR_GROUP)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SERVICE_STR_GROUP)->EnableWindow(FALSE);
				GetDlgItem(IDC_SERVICE_NAME_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SERVICE_NAME_LABEL)->EnableWindow(FALSE);
				GetDlgItem(IDC_SERVICE_NAME)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SERVICE_NAME)->EnableWindow(FALSE);
				GetDlgItem(IDC_SERVICE_DISP_NAME_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SERVICE_DISP_NAME_LABEL)->EnableWindow(FALSE);
				GetDlgItem(IDC_SERVICE_DISP_NAME)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SERVICE_DISP_NAME)->EnableWindow(FALSE);
				GetDlgItem(IDC_SERVICE_DESCR_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SERVICE_DESCR_LABEL)->EnableWindow(FALSE);
				GetDlgItem(IDC_SERVICE_DESCR)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SERVICE_DESCR)->EnableWindow(FALSE);
				// <== Adjustable NT Service Strings [Stulle] - Stulle
				GetDlgItem(IDC_SERVICE_OPT_GROUP)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SERVICE_OPT_GROUP)->EnableWindow(FALSE);
				GetDlgItem(IDC_SERVICE_OPT_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SERVICE_OPT_LABEL)->EnableWindow(FALSE);
				GetDlgItem(IDC_SERVICE_OPT_BOX)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SERVICE_OPT_BOX)->EnableWindow(FALSE);
				break;
			// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle
			default:
				break;
		}

		// Show new controls
		theApp.emuledlg->preferenceswnd->m_WebServerTab = tab;
		m_currentTab = tab;
		switch(m_currentTab){
			case WEBSERVER:
				GetDlgItem(IDC_WSENABLED)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_WSENABLED)->EnableWindow(TRUE);
				GetDlgItem(IDC_WS_GZIP)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_WSPORT)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_TMPLPATH)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_TMPLBROWSE)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_WSRELOADTMPL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_WSRELOADTMPL)->EnableWindow(TRUE);
				GetDlgItem(IDC_WSTIMEOUT)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_WSPASS)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_WS_ALLOWHILEVFUNC)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_WSENABLEDLOW)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_WSPASSLOW)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_MMENABLED)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_MMENABLED)->EnableWindow(TRUE);
				GetDlgItem(IDC_MMPASSWORDFIELD)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_MMPASSWORDFIELD)->EnableWindow(TRUE);
				GetDlgItem(IDC_MMPORT_FIELD)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_MMPORT_FIELD)->EnableWindow(TRUE);
				GetDlgItem(IDC_GUIDELINK)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_GUIDELINK)->EnableWindow(TRUE);
				GetDlgItem(IDC_STATIC_GENERAL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_STATIC_GENERAL)->EnableWindow(TRUE);
				GetDlgItem(IDC_WSPORT_LBL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_WSPORT_LBL)->EnableWindow(TRUE);
				GetDlgItem(IDC_TEMPLATE)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_TEMPLATE)->EnableWindow(TRUE);
				GetDlgItem(IDC_STATIC_ADMIN)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_STATIC_ADMIN)->EnableWindow(TRUE);
				GetDlgItem(IDC_WSPASS_LBL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_WSPASS_LBL)->EnableWindow(TRUE);
				GetDlgItem(IDC_STATIC_LOWUSER)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_STATIC_LOWUSER)->EnableWindow(TRUE);
				GetDlgItem(IDC_WSPASS_LBL2)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_WSPASS_LBL2)->EnableWindow(TRUE);
				GetDlgItem(IDC_STATIC_MOBILEMULE)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_STATIC_MOBILEMULE)->EnableWindow(TRUE);
				GetDlgItem(IDC_MMPASSWORD)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_MMPASSWORD)->EnableWindow(TRUE);
				GetDlgItem(IDC_MMPORT_LBL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_MMPORT_LBL)->EnableWindow(TRUE);
				GetDlgItem(IDC_WSTIMEOUTLABEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_WSTIMEOUTLABEL)->EnableWindow(TRUE);
				GetDlgItem(IDC_MINS)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_MINS)->EnableWindow(TRUE);
				GetDlgItem(IDC_WSUPNP)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_WSUPNP)->EnableWindow(thePrefs.IsUPnPEnabled() && thePrefs.GetWSIsEnabled());
				OnEnChangeWSEnabled();
				if(m_wndMobileLink.GetSafeHwnd())
				{
					m_wndMobileLink.ShowWindow(SW_SHOW);
					m_wndMobileLink.EnableWindow(TRUE);
				}
				break;
			// ==> Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he/Stulle] - Stulle
			case MULTIWEBSERVER:
				GetDlgItem(IDC_ADVADMINENABLED)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMINENABLED)->EnableWindow(TRUE);
				GetDlgItem(IDC_STATIC_ADVADMIN)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_STATIC_ADVADMIN)->EnableWindow(TRUE);
				GetDlgItem(IDC_ACCOUNTSELECT)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ACCOUNTSELECT)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_USER)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_USER)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_PASS)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_PASS)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_KAD)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_KAD)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_TRANSFER)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_TRANSFER)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_SEARCH)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_SEARCH)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_SERVER)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_SERVER)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_SHARED)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_SHARED)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_STATS)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_STATS)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_PREFS)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_PREFS)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_DFILES)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_DFILES)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_USERLEVEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_USERLEVEL)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_CATS)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_CATS)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_DELETE)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_DELETE)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_NEW)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_NEW)->EnableWindow(TRUE);
				GetDlgItem(IDC_STATIC_ADVADMIN_ACC)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_STATIC_ADVADMIN_ACC)->EnableWindow(TRUE);
				GetDlgItem(IDC_STATIC_ADVADMIN_CATS)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_STATIC_ADVADMIN_CATS)->EnableWindow(TRUE);
				GetDlgItem(IDC_STATIC_ADVADMIN_USER)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_STATIC_ADVADMIN_USER)->EnableWindow(TRUE);
				GetDlgItem(IDC_STATIC_ADVADMIN_PASS)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_STATIC_ADVADMIN_PASS)->EnableWindow(TRUE);
				GetDlgItem(IDC_STATIC_ADVADMIN_USERLEVEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_STATIC_ADVADMIN_USERLEVEL)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_NOTE)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_NOTE)->EnableWindow(TRUE);
				SetMultiBoxes();
				break;
			// <== Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he/Stulle] - Stulle
			// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
			case NTSERVICE:
				GetDlgItem(IDC_SVC_CURRENT_STATUS_LABEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_SVC_CURRENT_STATUS_LABEL)->EnableWindow(TRUE);
				GetDlgItem(IDC_SVC_CURRENT_STATUS)->ShowWindow(SW_SHOW);
				//GetDlgItem(IDC_SVC_CURRENT_STATUS)->EnableWindow(TRUE);
				GetDlgItem(IDC_SVC_INSTALLSERVICE)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_SVC_INSTALLSERVICE)->EnableWindow(TRUE);
				GetDlgItem(IDC_SVC_SERVERUNINSTALL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_SVC_SERVERUNINSTALL)->EnableWindow(TRUE);
				GetDlgItem(IDC_SVC_STARTUPBOX)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_SVC_STARTUPBOX)->EnableWindow(TRUE);
				GetDlgItem(IDC_SVC_STARTWITHSYSTEM)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_SVC_STARTWITHSYSTEM)->EnableWindow(TRUE);
				GetDlgItem(IDC_SVC_MANUALSTART)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_SVC_MANUALSTART)->EnableWindow(TRUE);
				GetDlgItem(IDC_SVC_SETTINGS)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_SVC_SETTINGS)->EnableWindow(TRUE);
				GetDlgItem(IDC_SVC_ONSTARTBOX)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_SVC_ONSTARTBOX)->EnableWindow(TRUE);
				GetDlgItem(IDC_SVC_RUNBROWSER)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_SVC_RUNBROWSER)->EnableWindow(TRUE);
				GetDlgItem(IDC_SVC_REPLACESERVICE)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_SVC_REPLACESERVICE)->EnableWindow(TRUE);
				// ==> Adjustable NT Service Strings [Stulle] - Stulle
				GetDlgItem(IDC_SERVICE_STR_GROUP)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_SERVICE_STR_GROUP)->EnableWindow(TRUE);
				GetDlgItem(IDC_SERVICE_NAME_LABEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_SERVICE_NAME_LABEL)->EnableWindow(TRUE);
				GetDlgItem(IDC_SERVICE_NAME)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_SERVICE_NAME)->EnableWindow(TRUE);
				GetDlgItem(IDC_SERVICE_DISP_NAME_LABEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_SERVICE_DISP_NAME_LABEL)->EnableWindow(TRUE);
				GetDlgItem(IDC_SERVICE_DISP_NAME)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_SERVICE_DISP_NAME)->EnableWindow(TRUE);
				GetDlgItem(IDC_SERVICE_DESCR_LABEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_SERVICE_DESCR_LABEL)->EnableWindow(TRUE);
				GetDlgItem(IDC_SERVICE_DESCR)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_SERVICE_DESCR)->EnableWindow(TRUE);
				// <== Adjustable NT Service Strings [Stulle] - Stulle
				GetDlgItem(IDC_SERVICE_OPT_GROUP)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_SERVICE_OPT_GROUP)->EnableWindow(TRUE);
				GetDlgItem(IDC_SERVICE_OPT_LABEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_SERVICE_OPT_LABEL)->EnableWindow(TRUE);
				GetDlgItem(IDC_SERVICE_OPT_BOX)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_SERVICE_OPT_BOX)->EnableWindow(TRUE);
				FillStatus();
				break;
			// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle
			default:
				break;
		}
	}
}
// <== Tabbed WebInterface settings panel [Stulle] - Stulle

// ==> Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he/Stulle] - Stulle
BOOL CPPgWebServer::OnSetActive()
{
	//if (IsWindow(m_hWnd))
	SetMultiBoxes();

	return TRUE;
}

afx_msg void CPPgWebServer::OnEnableChange()
{
	thePrefs.m_bIonixWebsrv = (IsDlgButtonChecked(IDC_ADVADMINENABLED)!=0);

	SetMultiBoxes();
	if (!thePrefs.m_bIonixWebsrv) {
		FillComboBox();
		FillUserlevelBox();
	}
	SetModified();
}

afx_msg void CPPgWebServer::SetMultiBoxes()
{
	bool bWSEnalbed = IsDlgButtonChecked(IDC_WSENABLED) && theApp.webserver->iMultiUserversion > 0;

	if(bWSEnalbed && IsDlgButtonChecked(IDC_ADVADMINENABLED)!=0)
	{
		GetDlgItem(IDC_ADVADMINENABLED)->EnableWindow(TRUE);
		m_cbAccountSelector.EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_DELETE)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_NEW)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_KAD)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_TRANSFER)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_SEARCH)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_SERVER)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_SHARED)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_STATS)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_PREFS)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_DFILES)->EnableWindow(TRUE);
		m_cbUserlevel.EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_PASS)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_USER)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_CATS)->EnableWindow(TRUE);
		UpdateSelection();
	}
	else
	{	
		if(bWSEnalbed && theApp.webserver->iMultiUserversion)
			GetDlgItem(IDC_ADVADMINENABLED)->EnableWindow(TRUE);
		else
			GetDlgItem(IDC_ADVADMINENABLED)->EnableWindow(FALSE);
		m_cbAccountSelector.EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_DELETE)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_NEW)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_KAD)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_TRANSFER)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_SEARCH)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_SERVER)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_SHARED)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_STATS)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_PREFS)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_DFILES)->EnableWindow(FALSE);
		m_cbUserlevel.EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_PASS)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_USER)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_CATS)->EnableWindow(FALSE);
	}
}

void CPPgWebServer::UpdateSelection()
{
	int accountsel = m_cbAccountSelector.GetCurSel();
	WebServDef tmp;
	if(accountsel == -1 || !theApp.webserver->AdvLogins.Lookup(accountsel , tmp))
	{
		//reset all if no selection possible
		GetDlgItem(IDC_ADVADMIN_DELETE)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_NEW)->EnableWindow(TRUE);
		CheckDlgButton(IDC_ADVADMIN_KAD, BST_UNCHECKED);
		CheckDlgButton(IDC_ADVADMIN_TRANSFER, BST_UNCHECKED);
		CheckDlgButton(IDC_ADVADMIN_SEARCH, BST_UNCHECKED);
		CheckDlgButton(IDC_ADVADMIN_SERVER, BST_UNCHECKED);
		CheckDlgButton(IDC_ADVADMIN_SHARED, BST_UNCHECKED);
		CheckDlgButton(IDC_ADVADMIN_STATS, BST_UNCHECKED);
		CheckDlgButton(IDC_ADVADMIN_PREFS, BST_UNCHECKED);
		CheckDlgButton(IDC_ADVADMIN_DFILES, BST_UNCHECKED);
		m_cbUserlevel.SetCurSel(0);
		GetDlgItem(IDC_ADVADMIN_PASS)->SetWindowText(_T(""));
		GetDlgItem(IDC_ADVADMIN_USER)->SetWindowText(_T(""));
		GetDlgItem(IDC_ADVADMIN_CATS)->SetWindowText(_T(""));
	}
	else
	{
		//set all data to our selectors
		GetDlgItem(IDC_ADVADMIN_DELETE)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_NEW)->EnableWindow(FALSE);
		CheckDlgButton(IDC_ADVADMIN_KAD, tmp.RightsToKad);
		CheckDlgButton(IDC_ADVADMIN_TRANSFER, tmp.RightsToTransfered);
		CheckDlgButton(IDC_ADVADMIN_SEARCH, tmp.RightsToSearch);
		CheckDlgButton(IDC_ADVADMIN_SERVER, tmp.RightsToServers);
		CheckDlgButton(IDC_ADVADMIN_SHARED, tmp.RightsToSharedList);
		CheckDlgButton(IDC_ADVADMIN_STATS, tmp.RightsToStats);
		CheckDlgButton(IDC_ADVADMIN_PREFS, tmp.RightsToPrefs);
		CheckDlgButton(IDC_ADVADMIN_DFILES, tmp.RightsToDownloadFiles);
		m_cbUserlevel.SetCurSel(tmp.RightsToAddRemove);
		GetDlgItem(IDC_ADVADMIN_PASS)->SetWindowText(HIDDEN_PASSWORD);
		GetDlgItem(IDC_ADVADMIN_USER)->SetWindowText(tmp.User);
		GetDlgItem(IDC_ADVADMIN_CATS)->SetWindowText(tmp.RightsToCategories);
	}
}

void CPPgWebServer::FillComboBox()
{
	//clear old values first
	m_cbAccountSelector.ResetContent();
	m_cbAccountSelector.InsertString(0, GetResString(IDS_ADVADMIN_NEW));
	for(POSITION pos = theApp.webserver->AdvLogins.GetHeadPosition(); pos; theApp.webserver->AdvLogins.GetNext(pos))
		m_cbAccountSelector.InsertString(theApp.webserver->AdvLogins.GetKeyAt(pos), theApp.webserver->AdvLogins.GetValueAt(pos).User);	
}

void CPPgWebServer::FillUserlevelBox()
{
	//clear old values first
	m_cbUserlevel.ResetContent();
	m_cbUserlevel.InsertString(0, GetResString(IDS_ADVADMIN_GUEST));
	m_cbUserlevel.InsertString(1, GetResString(IDS_ADVADMIN_OPERATOR));
	m_cbUserlevel.InsertString(2, GetResString(IDS_ADVADMIN_ADMIN));
	m_cbUserlevel.InsertString(3, GetResString(IDS_ADVADMIN_HIADMIN));
}


afx_msg void CPPgWebServer::OnMultiPWChange()
{
	int accountsel  = m_cbAccountSelector.GetCurSel();
	WebServDef tmp;
	if(accountsel  == -1 || !theApp.webserver->AdvLogins.Lookup(accountsel , tmp))
		return;

	CString buffer;
	GetDlgItem(IDC_ADVADMIN_PASS)->GetWindowText(buffer);
	if(buffer != HIDDEN_PASSWORD && MD5Sum(buffer).GetHash() != tmp.Pass)
		OnSettingsChange();
}

afx_msg void CPPgWebServer::OnMultiCatsChange()
{
	int accountsel  = m_cbAccountSelector.GetCurSel();
	WebServDef tmp;
	if(accountsel  == -1 || !theApp.webserver->AdvLogins.Lookup(accountsel , tmp))
		return;

	CString buffer;

	GetDlgItem(IDC_ADVADMIN_CATS)->GetWindowText(buffer);
	if(buffer != HIDDEN_PASSWORD && buffer != tmp.RightsToCategories)
		OnSettingsChange();
}

#define SET_TCHAR_TO_STRING(t, s) {_stprintf(t, _T("%s"), s);}

void CPPgWebServer::OnSettingsChange()
{
	SetModified();

	int accountsel  = m_cbAccountSelector.GetCurSel();
	WebServDef tmp;
	if(accountsel  == -1 || !theApp.webserver->AdvLogins.Lookup(accountsel , tmp))
		return;

	tmp.RightsToKad = IsDlgButtonChecked(IDC_ADVADMIN_KAD)!=0;
	tmp.RightsToTransfered = IsDlgButtonChecked(IDC_ADVADMIN_TRANSFER)!=0;
	tmp.RightsToSearch = IsDlgButtonChecked(IDC_ADVADMIN_SEARCH)!=0;
	tmp.RightsToServers = IsDlgButtonChecked(IDC_ADVADMIN_SERVER)!=0;
	tmp.RightsToSharedList = IsDlgButtonChecked(IDC_ADVADMIN_SHARED)!=0;
	tmp.RightsToStats = IsDlgButtonChecked(IDC_ADVADMIN_STATS)!=0;
	tmp.RightsToPrefs = IsDlgButtonChecked(IDC_ADVADMIN_PREFS)!=0;
	tmp.RightsToDownloadFiles = IsDlgButtonChecked(IDC_ADVADMIN_DFILES)!=0;
	//tmp.RightsToAddRemove = IsDlgButtonChecked(IDC_ADVADMIN_ADMIN)!=0;
	int j = m_cbUserlevel.GetCurSel();
	ASSERT(j <= 3); //only 0,1,2,3 allowed
	tmp.RightsToAddRemove = (uint8) j;
	
	CString buffer;
	GetDlgItem(IDC_ADVADMIN_PASS)->GetWindowText(buffer);
	if(buffer != HIDDEN_PASSWORD)
		SET_TCHAR_TO_STRING(tmp.Pass, MD5Sum(buffer).GetHash());
	
	GetDlgItem(IDC_ADVADMIN_USER)->GetWindowText(buffer);
	SET_TCHAR_TO_STRING(tmp.User, buffer);

	GetDlgItem(IDC_ADVADMIN_CATS)->GetWindowText(buffer);
	SET_TCHAR_TO_STRING(tmp.RightsToCategories, buffer);

	theApp.webserver->AdvLogins.SetAt(accountsel , tmp);

	FillComboBox();
	m_cbAccountSelector.SetCurSel(accountsel );
}

void CPPgWebServer::OnBnClickedNew()
{

    WebServDef tmp;
	CString buffer;
	GetDlgItem(IDC_ADVADMIN_USER)->GetWindowText(buffer);
	SET_TCHAR_TO_STRING(tmp.User, buffer);
    if (_tcslen(tmp.User)==0) // username not filled?
       return; 

	SetModified();

	int i = theApp.webserver->AdvLogins.IsEmpty() ? 1 : theApp.webserver->AdvLogins.GetCount()+1;

	tmp.RightsToKad = IsDlgButtonChecked(IDC_ADVADMIN_KAD)!=0;
	tmp.RightsToTransfered = IsDlgButtonChecked(IDC_ADVADMIN_TRANSFER)!=0;
	tmp.RightsToSearch = IsDlgButtonChecked(IDC_ADVADMIN_SEARCH)!=0;
	tmp.RightsToServers = IsDlgButtonChecked(IDC_ADVADMIN_SERVER)!=0;
	tmp.RightsToSharedList = IsDlgButtonChecked(IDC_ADVADMIN_SHARED)!=0;
	tmp.RightsToStats = IsDlgButtonChecked(IDC_ADVADMIN_STATS)!=0;
	tmp.RightsToPrefs = IsDlgButtonChecked(IDC_ADVADMIN_PREFS)!=0;
	tmp.RightsToDownloadFiles = IsDlgButtonChecked(IDC_ADVADMIN_DFILES)!=0;
	//tmp.RightsToAddRemove = IsDlgButtonChecked(IDC_ADVADMIN_ADMIN)!=0;
	int j = m_cbUserlevel.GetCurSel();
	ASSERT(j <= 3); //only 0,1,2,3 allowed
	tmp.RightsToAddRemove = (uint8) j;

	GetDlgItem(IDC_ADVADMIN_PASS)->GetWindowText(buffer);
	if(buffer != HIDDEN_PASSWORD)
		SET_TCHAR_TO_STRING(tmp.Pass, MD5Sum(buffer).GetHash());

	GetDlgItem(IDC_ADVADMIN_USER)->GetWindowText(buffer);
	SET_TCHAR_TO_STRING(tmp.User, buffer);

	GetDlgItem(IDC_ADVADMIN_CATS)->GetWindowText(buffer);
	SET_TCHAR_TO_STRING(tmp.RightsToCategories, buffer);

	theApp.webserver->AdvLogins.SetAt(i, tmp);
	theApp.webserver->SaveWebServConf(); //DEBUG should only be done at apply?
	FillComboBox();
	m_cbAccountSelector.SetCurSel(i);
    UpdateSelection();


	GetDlgItem(IDC_ADVADMIN_DELETE)->EnableWindow(TRUE);
	GetDlgItem(IDC_ADVADMIN_NEW)->EnableWindow(FALSE);
}

void CPPgWebServer::OnBnClickedDel()
{
	SetModified();

	const int i = m_cbAccountSelector.GetCurSel();  
    WebServDef tmp;  
    if(i == -1 || !theApp.webserver->AdvLogins.Lookup(i, tmp))   
         return;  
  
	CRBMap<uint32, WebServDef> tmpmap;  
  
    //retrieve all "wrong" entries  
    for(POSITION pos = theApp.webserver->AdvLogins.GetHeadPosition(); pos;)  
    {  
         POSITION pos2 = pos;  
         theApp.webserver->AdvLogins.GetNext(pos);  
  
         const int j = theApp.webserver->AdvLogins.GetKeyAt(pos2);  
         if(j == i)  
              theApp.webserver->AdvLogins.RemoveAt(pos2);  
         else if(j > i)  
         {  
              tmpmap.SetAt(j-1, theApp.webserver->AdvLogins.GetValueAt(pos2));  
              theApp.webserver->AdvLogins.RemoveAt(pos2);  
         }  
    }
	
	//reinsert all "wrong" entries correctly
	for(POSITION pos = tmpmap.GetHeadPosition(); pos; tmpmap.GetNext(pos))
		theApp.webserver->AdvLogins.SetAt(tmpmap.GetKeyAt(pos), tmpmap.GetValueAt(pos));
	
	FillComboBox();
	m_cbAccountSelector.SetCurSel(0); //set to the empty field
	UpdateSelection();
}
// <== Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he/Stulle] - Stulle

// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
int  CPPgWebServer::FillStatus(){
	int b_installed;
	int  i_startupmode;
	int rights;
		NTServiceGet(b_installed,i_startupmode,	rights);

		if (RunningAsService())		{ 
			GetDlgItem( IDC_SVC_CURRENT_STATUS)->SetWindowText(GetResString(IDS_SVC_RUNNINGASSERVICE));
			GetDlgItem( IDC_SVC_INSTALLSERVICE)->EnableWindow(false); // installing makes no sense when already running
			GetDlgItem( IDC_SVC_SERVERUNINSTALL)->EnableWindow(false); // cannot uninstall self
			if ( i_startupmode==1){
				CheckDlgButton(IDC_SVC_STARTWITHSYSTEM, BST_CHECKED);
				CheckDlgButton(IDC_SVC_MANUALSTART,     BST_UNCHECKED);
			}
			else {
				CheckDlgButton(IDC_SVC_STARTWITHSYSTEM, BST_UNCHECKED);
				CheckDlgButton(IDC_SVC_MANUALSTART,     BST_CHECKED);
			}
		}
		else {//This instance is not the running process
			if (b_installed==-1)// undetermined
			{
				GetDlgItem( IDC_SVC_INSTALLSERVICE)->EnableWindow(true); // probably fails but let user try
				GetDlgItem( IDC_SVC_SERVERUNINSTALL)->EnableWindow(true); // probably fails but let user try
				GetDlgItem( IDC_SVC_CURRENT_STATUS)->SetWindowText(GetResString(IDS_SVC_ACCESSDENIED)); 
			}
			else if (b_installed==0){
				GetDlgItem( IDC_SVC_CURRENT_STATUS)->SetWindowText(GetResString(IDS_SVC_NOTINSTALLED)); 
				GetDlgItem( IDC_SVC_INSTALLSERVICE)->EnableWindow(true); 
				GetDlgItem( IDC_SVC_SERVERUNINSTALL)->EnableWindow(false);
			}	else if (b_installed==1 && i_startupmode ==4 ){
				GetDlgItem( IDC_SVC_CURRENT_STATUS)->SetWindowText(GetResString(IDS_SVC_INSTALLED_DISABLED)); 
				GetDlgItem( IDC_SVC_INSTALLSERVICE)->EnableWindow(false); 
				GetDlgItem( IDC_SVC_SERVERUNINSTALL)->EnableWindow(true);
			}	else if (b_installed==1){
				GetDlgItem( IDC_SVC_CURRENT_STATUS)->SetWindowText(GetResString(IDS_SVC_INSTALLED)); 
				GetDlgItem( IDC_SVC_INSTALLSERVICE)->EnableWindow(false); 
 				GetDlgItem( IDC_SVC_SERVERUNINSTALL)->EnableWindow(true);
			}
			if(i_startupmode==1){
				CheckDlgButton(IDC_SVC_STARTWITHSYSTEM, BST_CHECKED);
				CheckDlgButton(IDC_SVC_MANUALSTART,     BST_UNCHECKED);
			}
			else{
				CheckDlgButton(IDC_SVC_STARTWITHSYSTEM, BST_UNCHECKED);
				CheckDlgButton(IDC_SVC_MANUALSTART,     BST_CHECKED);
			}
		}
		return 0;
}

void CPPgWebServer::OnBnClickedInstall()
{
	OnApply(); // Adjustable NT Service Strings [Stulle] - Stulle
	if (CmdInstallService((IsDlgButtonChecked(IDC_SVC_STARTWITHSYSTEM))==BST_CHECKED )==0)
	{
		FillStatus();
		if(AfxMessageBox(GetResString(IDS_APPLY_SETTINGS),MB_YESNO) == IDYES)
			OnBnAllSettings();
		SetModified();
		if (thePrefs.m_nCurrentUserDirMode == 0) // my documents and running as a service is not a good idea. but leave it to user
			AfxMessageBox(GetResString(IDS_CHANGEUSERASSERVICE),MB_OK);
	}
	else
		SetDlgItemText(IDC_SVC_CURRENT_STATUS,GetResString(IDS_SVC_INSTALLFAILED)); 
}

void CPPgWebServer::OnBnClickedUnInstall()
{
	if (CmdRemoveService()==0) {
		FillStatus();
		SetModified();
	}
	else
		SetDlgItemText(IDC_SVC_CURRENT_STATUS,GetResString(IDS_SVC_UNINSTALLFAILED)); 

}

void CPPgWebServer::OnBnStartSystem(){
	if(IsDlgButtonChecked(IDC_SVC_MANUALSTART)==BST_CHECKED )
		SetModified();
	CheckDlgButton(IDC_SVC_STARTWITHSYSTEM, BST_CHECKED);
	CheckDlgButton(IDC_SVC_MANUALSTART,     BST_UNCHECKED);
};	
void CPPgWebServer::OnBnManualStart(){
	if(IsDlgButtonChecked(IDC_SVC_STARTWITHSYSTEM)==BST_CHECKED )
		SetModified();
	CheckDlgButton(IDC_SVC_STARTWITHSYSTEM,BST_UNCHECKED );
	CheckDlgButton(IDC_SVC_MANUALSTART,    BST_CHECKED);
};	

void CPPgWebServer::OnBnAllSettings(){
	thePrefs.startupsound=false;
	thePrefs.SetAutoConnect(true);
	thePrefs.SetWSIsEnabled(true); 
	RemAutoStart(); // remove from windows startup. 
	thePrefs.m_bEnableMiniMule=false;
	thePrefs.m_bSelCatOnAdd=false;
	thePrefs.notifierSoundType = ntfstNoSound;
	thePrefs.notifierOnDownloadFinished =false;
	thePrefs.splashscreen=false;
	thePrefs.startMinimized=true;
	thePrefs.beepOnError=false;
	thePrefs.bringtoforeground=false;
//	SetModified();
};	

void CPPgWebServer::OnBnReplaceStart(){
	CheckDlgButton(IDC_SVC_RUNBROWSER   ,BST_UNCHECKED );
	CheckDlgButton(IDC_SVC_REPLACESERVICE, BST_CHECKED);
	SetModified();
};	

void CPPgWebServer::OnBnRunBRowser(){
	CheckDlgButton(IDC_SVC_RUNBROWSER   ,BST_CHECKED );
	CheckDlgButton(IDC_SVC_REPLACESERVICE,    BST_UNCHECKED);
	SetModified();
};	

void CPPgWebServer::InitOptLvlCbn(bool bFirstInit)
{
	int iSel = m_cbOptLvl.GetCurSel();
	int iItem;
	m_cbOptLvl.ResetContent();
	iItem = m_cbOptLvl.AddString(_T("0: ") + GetResString(IDS_SERVICE_OPT_NONE));		m_cbOptLvl.SetItemData(iItem, SVC_NO_OPT);
	iItem = m_cbOptLvl.AddString(_T("4: ") + GetResString(IDS_SERVICE_OPT_BASIC));		m_cbOptLvl.SetItemData(iItem, SVC_LIST_OPT);
	iItem = m_cbOptLvl.AddString(_T("6: ") + GetResString(IDS_SERVICE_OPT_LISTS));		m_cbOptLvl.SetItemData(iItem, SVC_SVR_OPT);
	iItem = m_cbOptLvl.AddString(_T("10: ") + GetResString(IDS_SERVICE_OPT_FULL));		m_cbOptLvl.SetItemData(iItem, SVC_FULL_OPT);

	if(bFirstInit)
	{
		switch(thePrefs.GetServiceOptLvl())
		{
			case SVC_NO_OPT:
				m_cbOptLvl.SetCurSel(0);
				break;
			case SVC_LIST_OPT:
				m_cbOptLvl.SetCurSel(1);
				break;
			case SVC_SVR_OPT:
				m_cbOptLvl.SetCurSel(2);
				break;
			case SVC_FULL_OPT:
				m_cbOptLvl.SetCurSel(3);
				break;
		}
	}
	else
		m_cbOptLvl.SetCurSel(iSel != CB_ERR ? iSel : 0);
}
// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle