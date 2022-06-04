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
#include "PreferencesDlg.h" //>>> [ionix] - iONiX::Advanced WebInterface Account Management

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
	// MORPH start tabbed option [leuk_he]
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_WEBSERVER1, OnTcnSelchangeTab)
	// MORPH end tabbed option [leuk_he]
END_MESSAGE_MAP()

CPPgWebServer::CPPgWebServer()
// MORPH START leuk_he tooltipped
/*
    : CPropertyPage(CPPgWebServer::IDD)
*/
	: CPPgtooltipped(CPPgWebServer::IDD)
// MORPH END leuk_he tooltipped
{
	bCreated = false;
	m_icoBrowse = NULL;
	// MORPH start tabbed option [leuk_he]
	m_imageList.DeleteImageList();
	m_imageList.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 14+1, 0);
	m_imageList.Add(CTempIconLoader(_T("WEB")));
	// MORPH end tabbed option [leuk_he]
}

CPPgWebServer::~CPPgWebServer()
{
}

void CPPgWebServer::DoDataExchange(CDataExchange* pDX)
{
  	CPropertyPage::DoDataExchange(pDX);
  // MORPH start tabbed options [leuk_he]
    DDX_Control(pDX, IDC_TAB_WEBSERVER1 , m_tabCtr);
  // MORPH end tabbed options [leuk_he]
}

BOOL CPPgWebServer::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	AddBuddyButton(GetDlgItem(IDC_TMPLPATH)->m_hWnd, ::GetDlgItem(m_hWnd, IDC_TMPLBROWSE));
	InitAttachedBrowseButton(::GetDlgItem(m_hWnd, IDC_TMPLBROWSE), m_icoBrowse);

	((CEdit*)GetDlgItem(IDC_WSPASS))->SetLimitText(12);
	((CEdit*)GetDlgItem(IDC_WSPORT))->SetLimitText(6);

	LoadSettings();
	InitTooltips(); // MORPH leuk_he tooltipped
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

	CheckDlgButton(IDC_WS_GZIP,(thePrefs.GetWebUseGzip())?1:0 );
	CheckDlgButton(IDC_WS_ALLOWHILEVFUNC,(thePrefs.GetWebAdminAllowedHiLevFunc())?1:0 );
	
	//MORPH START - UPnP
#ifdef USE_OFFICIAL_UPNP
	GetDlgItem(IDC_WSUPNP)->EnableWindow(thePrefs.IsUPnPEnabled() && thePrefs.GetWSIsEnabled());
	CheckDlgButton(IDC_WSUPNP, (thePrefs.IsUPnPEnabled() && thePrefs.m_bWebUseUPnP) ? TRUE : FALSE);
#else
	GetDlgItem(IDC_WSUPNP)->EnableWindow(thePrefs.IsUPnPNat() && thePrefs.GetWSIsEnabled());
	CheckDlgButton(IDC_WSUPNP, (thePrefs.IsUPnPNat() && thePrefs.GetUPnPNatWeb()) ? TRUE : FALSE);
#endif
	//MORPH END   - UPnP
	
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

	//MORPH START - UPnP
#ifdef USE_OFFICIAL_UPNP
		if (IsDlgButtonChecked(IDC_WSUPNP))
		{
			ASSERT( thePrefs.IsUPnPEnabled() );
			if (!thePrefs.m_bWebUseUPnP && thePrefs.GetWSIsEnabled() && theApp.m_pUPnPFinder != NULL) // add the port to existing mapping without having eMule restarting (if all conditions are met)
				theApp.m_pUPnPFinder->GetImplementation()->LateEnableWebServerPort(thePrefs.GetWSPort());
			thePrefs.m_bWebUseUPnP = true;
		}
		else
			thePrefs.m_bWebUseUPnP = false;
#else
		if ((UINT)thePrefs.GetUPnPNatWeb() != IsDlgButtonChecked(IDC_WSUPNP))
		{
			theApp.m_UPnP_IGDControlPoint->SetUPnPNat(thePrefs.IsUPnPNat()); // and start/stop nat. 
			thePrefs.SetUPnPNatWeb(IsDlgButtonChecked(IDC_WSUPNP)!=0);
		}
#endif
	//MORPH END   - UPnP

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

		// MORPH start tabbed options [leuk_he]
		InitTab(true);
		m_tabCtr.SetCurSel(theApp.emuledlg->preferenceswnd->StartPageWebServer);
		// MORPH end tabbed options [leuk_he]

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
		//MORPH START leuk_he tooltipped
		SetTool(IDC_WSPASS,IDC_WSPASS_TIP);
		SetTool(IDC_WSPASSLOW,IDC_WSPASSLOW_TIP);
		SetTool(IDC_MMPASSWORDFIELD,IDC_MMPASSWORDFIELD_TIP);
		SetTool(IDC_WSPORT,IDC_WSPORT_TIP);
		SetTool(IDC_MMPORT_FIELD,IDC_MMPORT_FIELD_TIP);
		SetTool(IDC_TMPLPATH,IDC_TMPLPATH_TIP);
		SetTool(IDC_WSTIMEOUT,IDC_WSTIMEOUT_TIP);
		SetTool(IDC_WSENABLED,IDC_WSENABLED_TIP);
		SetTool(IDC_WSENABLEDLOW,IDC_WSENABLEDLOW_TIP);
		SetTool(IDC_MMENABLED,IDC_MMENABLED_TIP);
		SetTool(IDC_WS_GZIP,IDC_WS_GZIP_TIP);
		SetTool(IDC_WS_ALLOWHILEVFUNC,IDC_WS_ALLOWHILEVFUNC_TIP);
		SetTool(IDC_WSRELOADTMPL,IDC_WSRELOADTMPL_TIP);
	    SetTool(IDC_TMPLBROWSE,IDC_TMPLBROWSE_TIP);
		SetTool(IDC_GUIDELINK,IDC_GUIDELINK_TIP);
		SetTool(IDC_WSUPNP,IDS_WSUPNP_TIP);
		//MORPH END leuk_he tooltipped
	}
}

void CPPgWebServer::OnEnChangeWSEnabled()
{
//>>> [ionix] - iONiX::Advanced WebInterface Account Management
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
	//MORPH START - UPnP
#ifdef USE_OFFICIAL_UPNP
	GetDlgItem(IDC_WSUPNP)->EnableWindow(thePrefs.IsUPnPEnabled() && bIsWIEnabled);
#else
	GetDlgItem(IDC_WSUPNP)->EnableWindow(thePrefs.IsUPnPNat() && bIsWIEnabled);
#endif
	//MORPH END   - UPnP
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
	//MORPH START - UPnP
#ifdef USE_OFFICIAL_UPNP
	GetDlgItem(IDC_WSUPNP)->EnableWindow(thePrefs.IsUPnPEnabled() && bIsWIEnabled);
#else
	GetDlgItem(IDC_WSUPNP)->EnableWindow(thePrefs.IsUPnPNat() && bIsWIEnabled);
#endif
	//MORPH END   - UPnP
//<<< [ionix] - iONiX::Advanced WebInterface Account Management

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

// MORPH start tabbed option [leuk_he]
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
	int cur_sel = m_tabCtr.GetCurSel();
	theApp.emuledlg->preferenceswnd->SwitchTab(cur_sel);
	*pResult = 0;
}
// MORPH end tabbed option [leuk_he]
