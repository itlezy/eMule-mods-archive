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
#include "emuledlg.h"
#include "Preferences.h"
#include "ServerWnd.h"
#include "UPnPImplWrapper.h"
#include "UPnPImpl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define HIDDEN_PASSWORD _T("*****")


IMPLEMENT_DYNAMIC(CPPgWebServer, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgWebServer, CPropertyPage)
	ON_EN_CHANGE(IDC_WSPASS, OnSettingsChange) // X: [CI] - [Code Improvement] Apply if modified
	ON_EN_CHANGE(IDC_WSPASSLOW, OnSettingsChange)
	ON_EN_CHANGE(IDC_WSPORT, OnSettingsChange)
	ON_EN_CHANGE(IDC_TMPLPATH, OnDataChange)
	ON_EN_CHANGE(IDC_WSTIMEOUT, OnSettingsChange)
	ON_BN_CLICKED(IDC_WSENABLED, OnEnChangeWSEnabled)
	ON_BN_CLICKED(IDC_WSENABLEDLOW, OnEnChangeWSEnabled)
	ON_BN_CLICKED(IDC_WSRELOADTMPL, OnReloadTemplates)
	ON_BN_CLICKED(IDC_TMPLBROWSE, OnBnClickedTmplbrowse)
	ON_BN_CLICKED(IDC_WS_GZIP, OnSettingsChange) // X: [CI] - [Code Improvement] Apply if modified
	ON_BN_CLICKED(IDC_WS_ALLOWHILEVFUNC, OnSettingsChange)
	ON_BN_CLICKED(IDC_WSUPNP, OnSettingsChange)
END_MESSAGE_MAP()

CPPgWebServer::CPPgWebServer()
	: CPropertyPage(CPPgWebServer::IDD)
{
	bCreated = false;
}

CPPgWebServer::~CPPgWebServer()
{
}

/*void CPPgWebServer::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}
*/
BOOL CPPgWebServer::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	((CEdit*)GetDlgItem(IDC_WSPASS))->SetLimitText(12);
	((CEdit*)GetDlgItem(IDC_WSPORT))->SetLimitText(6);

	LoadSettings();
	Localize();

	OnEnChangeWSEnabled();

	// note: there are better classes to create a pure hyperlink, however since it is only needed here
	//		 we rather use an already existing class
	CRect rect;
	::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&rect, 2);

	m_bModified = false; // X: [CI] - [Code Improvement] Apply if modified
	return TRUE;
}

void CPPgWebServer::LoadSettings(void)
{
	CString strBuffer;

	SetDlgItemText(IDC_WSPASS,HIDDEN_PASSWORD);
	SetDlgItemText(IDC_WSPASSLOW,HIDDEN_PASSWORD);

	strBuffer.Format(_T("%d"), thePrefs.GetWSPort());
	SetDlgItemText(IDC_WSPORT,strBuffer);

	SetDlgItemText(IDC_TMPLPATH,thePrefs.GetTemplate());

	strBuffer.Format(_T("%d"), thePrefs.GetWebTimeoutMins());
	SetDlgItemText(IDC_WSTIMEOUT,strBuffer);

	CheckDlgButton(IDC_WSENABLED,thePrefs.GetWSIsEnabled());
	CheckDlgButton(IDC_WSENABLEDLOW,thePrefs.GetWSIsLowUserEnabled());
	CheckDlgButton(IDC_WS_GZIP,thePrefs.GetWebUseGzip());
	CheckDlgButton(IDC_WS_ALLOWHILEVFUNC,thePrefs.GetWebAdminAllowedHiLevFunc());

	GetDlgItem(IDC_WSUPNP)->EnableWindow(thePrefs.IsUPnPEnabled() && thePrefs.GetWSIsEnabled());
	CheckDlgButton(IDC_WSUPNP, (thePrefs.IsUPnPEnabled() && thePrefs.m_bWebUseUPnP) ? TRUE : FALSE);
}

BOOL CPPgWebServer::OnApply()
{	
	if(m_bModified)
	{
		CString sBuf;

		// get and check templatefile existance...
		GetDlgItemText(IDC_TMPLPATH,sBuf);
		if ( IsDlgButtonChecked(IDC_WSENABLED) && !PathFileExists(sBuf)) {
			CString buffer;
			buffer.Format(GetResString(IDS_WEB_ERR_CANTLOAD),sBuf);
			AfxMessageBox(buffer,MB_OK);
			return FALSE;
		}
		thePrefs.SetTemplate(sBuf);
		theApp.webserver->ReloadTemplates();


		uint16 oldPort=thePrefs.GetWSPort();

		GetDlgItemText(IDC_WSPASS,sBuf);
		if(sBuf != HIDDEN_PASSWORD)
			thePrefs.SetWSPass(sBuf);
		
		GetDlgItemText(IDC_WSPASSLOW,sBuf);
		if(sBuf != HIDDEN_PASSWORD)
			thePrefs.SetWSLowPass(sBuf);

		bool restartWebServer = false;
		uint16 iTemp=(uint16)GetDlgItemInt(IDC_WSPORT,NULL,FALSE);
		if (iTemp!=oldPort) {
			thePrefs.SetWSPort(iTemp);
			restartWebServer = true;
		}

		thePrefs.m_iWebTimeoutMins=GetDlgItemInt(IDC_WSTIMEOUT,NULL,FALSE);

		thePrefs.SetWSIsEnabled(IsDlgButtonChecked(IDC_WSENABLED)!=0);
		thePrefs.SetWSIsLowUserEnabled(IsDlgButtonChecked(IDC_WSENABLEDLOW)!=0);
		thePrefs.SetWebUseGzip(IsDlgButtonChecked(IDC_WS_GZIP)!=0);
		thePrefs.m_bAllowAdminHiLevFunc= (IsDlgButtonChecked(IDC_WS_ALLOWHILEVFUNC)!=0);

		if (IsDlgButtonChecked(IDC_WSUPNP))
		{
			ASSERT( thePrefs.IsUPnPEnabled() );
			if (thePrefs.GetWSIsEnabled() && theApp.m_pUPnPFinder != NULL) // add the port to existing mapping without having eMule restarting (if all conditions are met)
				theApp.m_pUPnPFinder->GetImplementation()->ChangePort(PI_WebTCP, thePrefs.GetWSPort());
			thePrefs.m_bWebUseUPnP = true;
		}
		else
			thePrefs.m_bWebUseUPnP = false;

                if (restartWebServer)
			theApp.webserver->RestartServer();
		else
			theApp.webserver->StartServer();
		theApp.emuledlg->serverwnd->UpdateMyInfo();
		SetModified(FALSE);
		SetTmplButtonState();
		m_bModified = false; // X: [CI] - [Code Improvement] Apply if modified
	}
	return CPropertyPage::OnApply();
}

void CPPgWebServer::Localize(void)
{
	if(m_hWnd){
		SetWindowText(GetResString(IDS_PW_WS));
		SetDlgItemText(IDC_WSPASS_LBL,GetResString(IDS_WS_PASS));
		SetDlgItemText(IDC_WSPORT_LBL,GetResString(IDS_PORT));
		SetDlgItemText(IDC_WSENABLED,GetResString(IDS_ENABLED));
		SetDlgItemText(IDC_WSRELOADTMPL,GetResString(IDS_SF_RELOAD));
		SetDlgItemText(IDC_WSENABLED,GetResString(IDS_ENABLED));
		SetDlgItemText(IDC_WS_GZIP,GetResString(IDS_WEB_GZIP_COMPRESSION));
		SetDlgItemText(IDC_WSUPNP, GetResString(IDS_WEBUPNPINCLUDE));

		SetDlgItemText(IDC_WSPASS_LBL2,GetResString(IDS_WS_PASS));
		SetDlgItemText(IDC_WSENABLEDLOW,GetResString(IDS_ENABLED));
		SetDlgItemText(IDC_STATIC_GENERAL,GetResString(IDS_PW_GENERAL));

		SetDlgItemText(IDC_STATIC_ADMIN,GetResString(IDS_ADMIN));
		SetDlgItemText(IDC_STATIC_LOWUSER,GetResString(IDS_WEB_LOWUSER));
		SetDlgItemText(IDC_WSENABLEDLOW,GetResString(IDS_ENABLED));

		SetDlgItemText(IDC_TEMPLATE,GetResString(IDS_WS_RELOAD_TMPL));
		SetDlgItemText(IDC_WSTIMEOUTLABEL,GetResString(IDS_WEB_SESSIONTIMEOUT)+_T(':'));
		SetDlgItemText(IDC_MINS,GetResString(IDS_LONGMINS) );

		SetDlgItemText(IDC_WS_ALLOWHILEVFUNC,GetResString(IDS_WEB_ALLOWHILEVFUNC));
	}
}

void CPPgWebServer::OnEnChangeWSEnabled()
{
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

	//GetDlgItem(IDC_WSRELOADTMPL)->EnableWindow(bIsWIEnabled);
	SetTmplButtonState();
	OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified
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
		SetDlgItemText(IDC_TMPLPATH,buffer);
		OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified
		SetTmplButtonState();
	}
}

void CPPgWebServer::SetTmplButtonState(){
	CString buffer;
	GetDlgItemText(IDC_TMPLPATH,buffer);
	GetDlgItem(IDC_WSRELOADTMPL)->EnableWindow( thePrefs.GetWSIsEnabled() && (buffer.CompareNoCase(thePrefs.GetTemplate())==0));
}
