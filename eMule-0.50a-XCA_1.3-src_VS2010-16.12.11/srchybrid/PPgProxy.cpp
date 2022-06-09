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
#include "PPGProxy.h"
#include "opcodes.h"
#include "OtherFunctions.h"
#include "Preferences.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CPPgProxy, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgProxy, CPropertyPage)
	ON_BN_CLICKED(IDC_ENABLEPROXY, OnBnClickedEnableProxy)
	ON_BN_CLICKED(IDC_ENABLEAUTH, OnBnClickedEnableAuthentication)
	ON_CBN_SELCHANGE(IDC_PROXYTYPE, OnCbnSelChangeProxyType)
	ON_EN_CHANGE(IDC_PROXYNAME, OnSettingsChange) // X: [CI] - [Code Improvement]
	ON_EN_CHANGE(IDC_PROXYPORT, OnSettingsChange)
	ON_EN_CHANGE(IDC_USERNAME_A, OnSettingsChange)
	ON_EN_CHANGE(IDC_PASSWORD, OnSettingsChange)
END_MESSAGE_MAP()

CPPgProxy::CPPgProxy()
	: CPropertyPage(CPPgProxy::IDD)
{
}

CPPgProxy::~CPPgProxy()
{
}

/*void CPPgProxy::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}
*/
BOOL CPPgProxy::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	proxy = thePrefs.GetProxySettings();
	LoadSettings();
	Localize();
	m_bModified = false; // X: [CI] - [Code Improvement] Apply if modified

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPgProxy::OnApply()
{
	if(m_bModified) // X: [CI] - [Code Improvement] Apply if modified
	{
		proxy.UseProxy = (IsDlgButtonChecked(IDC_ENABLEPROXY) != 0);
		proxy.EnablePassword = ((CButton*)GetDlgItem(IDC_ENABLEAUTH))->GetCheck() != 0;
		proxy.type = (uint16)((CComboBox*)GetDlgItem(IDC_PROXYTYPE))->GetCurSel();

		if (GetDlgItem(IDC_PROXYNAME)->GetWindowTextLength()) {
			CStringA strProxyA;
			GetWindowTextA(*GetDlgItem(IDC_PROXYNAME), strProxyA.GetBuffer(256), 256);
			strProxyA.ReleaseBuffer();
			strProxyA.FreeExtra();
			int iColon = strProxyA.Find(':');
			if (iColon > -1) {
				SetDlgItemTextA(m_hWnd, IDC_PROXYPORT, strProxyA.Mid(iColon + 1));
				strProxyA = strProxyA.Left(iColon);
			}
			proxy.name = strProxyA;
		}
		else {
			proxy.name.Empty();
			proxy.UseProxy = false;
		}
		if (GetDlgItem(IDC_PROXYPORT)->GetWindowTextLength()){
			uint16 nPort=(uint16)GetDlgItemInt(IDC_PROXYPORT,NULL, FALSE);
			proxy.port = (nPort==0?1080:nPort);
		}
		else
			proxy.port = 1080;

		if (GetDlgItem(IDC_USERNAME_A)->GetWindowTextLength()) { 
			CString strUser;
			GetDlgItemText(IDC_USERNAME_A,strUser);
			proxy.user = CStringA(strUser);
		}
		else {
			proxy.user.Empty();
			proxy.EnablePassword = false;
		}

		if (GetDlgItem(IDC_PASSWORD)->GetWindowTextLength()) { 
			CString strPasswd;
			GetDlgItemText(IDC_PASSWORD,strPasswd);
			proxy.password = CStringA(strPasswd);
		}
		else {
			proxy.password.Empty();
			proxy.EnablePassword = false;
		}

		thePrefs.SetProxySettings(proxy);
		LoadSettings();
		m_bModified = false; // X: [CI] - [Code Improvement] Apply if modified
	}
	return CPropertyPage::OnApply();
}

void CPPgProxy::OnBnClickedEnableProxy()
{
	OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified
	CButton* btn = (CButton*) GetDlgItem(IDC_ENABLEPROXY);
	GetDlgItem(IDC_ENABLEAUTH)->EnableWindow(btn->GetCheck() != 0);
	GetDlgItem(IDC_PROXYTYPE)->EnableWindow(btn->GetCheck() != 0);
	GetDlgItem(IDC_PROXYNAME)->EnableWindow(btn->GetCheck() != 0);
	GetDlgItem(IDC_PROXYPORT)->EnableWindow(btn->GetCheck() != 0);
	GetDlgItem(IDC_USERNAME_A)->EnableWindow(btn->GetCheck() != 0);
	GetDlgItem(IDC_PASSWORD)->EnableWindow(btn->GetCheck() != 0);
	if (btn->GetCheck() != 0) OnBnClickedEnableAuthentication();
	if (btn->GetCheck() != 0) OnCbnSelChangeProxyType();
}

void CPPgProxy::OnBnClickedEnableAuthentication()
{
	OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified
	CButton* btn = (CButton*)GetDlgItem(IDC_ENABLEAUTH);
	GetDlgItem(IDC_USERNAME_A)->EnableWindow(btn->GetCheck() != 0);
	GetDlgItem(IDC_PASSWORD)->EnableWindow(btn->GetCheck() != 0);
}

void CPPgProxy::OnCbnSelChangeProxyType()
{
	OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified
	CComboBox* cbbox = (CComboBox*)GetDlgItem(IDC_PROXYTYPE);
	if (!(cbbox->GetCurSel() == PROXYTYPE_SOCKS5 || /*cbbox->GetCurSel() == PROXYTYPE_HTTP10 ||*/ cbbox->GetCurSel() == PROXYTYPE_HTTP11)) // netfinity: Not availably in latest AsyncProxySocketLayer
	{
		CheckDlgButton(IDC_ENABLEAUTH,0);
		OnBnClickedEnableAuthentication();
		GetDlgItem(IDC_ENABLEAUTH)->EnableWindow(FALSE);
	} 
	else
		GetDlgItem(IDC_ENABLEAUTH)->EnableWindow(TRUE);
}

void CPPgProxy::LoadSettings()
{
	CheckDlgButton(IDC_ENABLEPROXY,proxy.UseProxy);
	CheckDlgButton(IDC_ENABLEAUTH,proxy.EnablePassword);
	((CComboBox*)GetDlgItem(IDC_PROXYTYPE))->SetCurSel(proxy.type);
	SetWindowTextA(*GetDlgItem(IDC_PROXYNAME), proxy.name);
	TCHAR szBuff[12];
	_ultot_s(proxy.port, szBuff, 10);// X: [CI] - [Code Improvement]
	szBuff[_countof(szBuff) - 1] = _T('\0');
	SetDlgItemText(IDC_PROXYPORT, szBuff);
	SetWindowTextA(*GetDlgItem(IDC_USERNAME_A), proxy.user);
	SetWindowTextA(*GetDlgItem(IDC_PASSWORD), proxy.password);
	OnBnClickedEnableProxy();
}

void CPPgProxy::Localize()
{
	if (m_hWnd)
	{
		SetWindowText(GetResString(IDS_PW_PROXY));
		SetDlgItemText(IDC_ENABLEPROXY,GetResString(IDS_PROXY_ENABLE));	
		SetDlgItemText(IDC_PROXYTYPE_LBL,GetResString(IDS_PROXY_TYPE));	
		SetDlgItemText(IDC_PROXYNAME_LBL,GetResString(IDS_PROXY_HOST));	
		SetDlgItemText(IDC_PROXYPORT_LBL,GetResString(IDS_PROXY_PORT));	
		SetDlgItemText(IDC_ENABLEAUTH,GetResString(IDS_PROXY_AUTH));	
		SetDlgItemText(IDC_USERNAME_LBL,GetResString(IDS_CD_UNAME));	
		SetDlgItemText(IDC_PASSWORD_LBL,GetResString(IDS_WS_PASS) + _T(':'));	
		SetDlgItemText(IDC_AUTH_LBL,GetResString(IDS_AUTH));	
		SetDlgItemText(IDC_AUTH_LBL2,GetResString(IDS_PW_GENERAL));	
	}
}
