//this file is part of eMule
//Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// CPPGProxy dialog

IMPLEMENT_DYNAMIC(CPPgProxy, CPropertyPage)
CPPgProxy::CPPgProxy()
	: CPropertyPage(CPPgProxy::IDD)
	, m_bEnableProxy(FALSE)
	, m_bEnableAuth(FALSE)
{
}

CPPgProxy::~CPPgProxy()
{
}

void CPPgProxy::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROXYTYPE, m_ProxyTypeCombo);
	DDX_Check(pDX, IDC_ENABLEPROXY, m_bEnableProxy);
	DDX_Check(pDX, IDC_ENABLEAUTH, m_bEnableAuth);
	DDX_Text(pDX, IDC_PROXYPORT, m_strProxyPort);
	DDX_Text(pDX, IDC_USERNAME, m_strProxyUser);
	DDX_Text(pDX, IDC_PASSWORD, m_strProxyPassword);
	DDX_Text(pDX, IDC_PROXYNAME, m_strProxyName);
}


BEGIN_MESSAGE_MAP(CPPgProxy, CPropertyPage)
	ON_BN_CLICKED(IDC_ENABLEPROXY, OnBnClickedEnableproxy)
	ON_BN_CLICKED(IDC_ENABLEAUTH, OnBnClickedEnableauth)
	ON_CBN_SELCHANGE(IDC_PROXYTYPE, OnCbnSelchangeProxytype)
	ON_EN_CHANGE(IDC_PROXYNAME, OnSettingsChange)
	ON_EN_CHANGE(IDC_PROXYPORT, OnSettingsChange)
	ON_EN_CHANGE(IDC_USERNAME, OnSettingsChange)
	ON_EN_CHANGE(IDC_PASSWORD, OnSettingsChange)
END_MESSAGE_MAP()


BOOL CPPgProxy::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_ProxyTypeCombo.AddString(_T("SOCKS4"));
	m_ProxyTypeCombo.AddString(_T("SOCKS4a"));
	m_ProxyTypeCombo.AddString(_T("SOCKS5"));
	m_ProxyTypeCombo.AddString(_T("HTTP 1.1"));

	(reinterpret_cast<CEdit*>(GetDlgItem(IDC_PROXYPORT)))->SetLimitText(5);
	m_proxy = m_pPrefs->GetProxySettings();
	LoadSettings();
	Localize();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPgProxy::OnApply()
{
	UpdateData(TRUE);

	m_proxy.m_bUseProxy = B2b(m_bEnableProxy);
	m_proxy.m_bEnablePassword = B2b(m_bEnableAuth);
	m_proxy.m_nType = (uint16)(m_ProxyTypeCombo.GetCurSel() + 1);	// PROXYTYPE_NOPROXY not in the list
	
	m_proxy.m_strName = m_strProxyName;
	if (m_strProxyName.IsEmpty())
		m_proxy.m_bUseProxy = false;

	unsigned	uiVal = _tstoi(m_strProxyPort);
	
//	Valid values range is 1..65535
	if ((uiVal - 1u) > 0xFFFEu)
		uiVal = PREF_DEF_PROXY_PORT;
	m_proxy.m_uPort = static_cast<uint16>(uiVal);
	
	m_proxy.m_strUser = m_strProxyUser;
	if (m_strProxyUser.IsEmpty())
		m_proxy.m_bEnablePassword = false;
		
	m_proxy.m_strPassword = m_strProxyPassword;
	if (m_strProxyPassword.IsEmpty())
		m_proxy.m_bEnablePassword = false;
	
	m_pPrefs->SetProxySettings(m_proxy);
	LoadSettings();
	return CPropertyPage::OnApply();
}

void CPPgProxy::OnBnClickedEnableproxy()
{
	SetModified();
	UpdateData(TRUE);

	GetDlgItem(IDC_ENABLEAUTH)->EnableWindow(m_bEnableProxy);
	m_ProxyTypeCombo.EnableWindow(m_bEnableProxy);
	GetDlgItem(IDC_PROXYNAME)->EnableWindow(m_bEnableProxy);
	GetDlgItem(IDC_PROXYPORT)->EnableWindow(m_bEnableProxy);
	GetDlgItem(IDC_USERNAME)->EnableWindow(m_bEnableProxy);
	GetDlgItem(IDC_PASSWORD)->EnableWindow(m_bEnableProxy);
	if (m_bEnableProxy)
	{
		OnBnClickedEnableauth();
		OnCbnSelchangeProxytype();
	}
}

void CPPgProxy::OnBnClickedEnableauth()
{
	SetModified();
	UpdateData(TRUE);

	GetDlgItem(IDC_USERNAME)->EnableWindow(m_bEnableAuth);
	GetDlgItem(IDC_PASSWORD)->EnableWindow(m_bEnableAuth);
}

void CPPgProxy::OnCbnSelchangeProxytype()
{
	SetModified();

	int		iCurType = m_ProxyTypeCombo.GetCurSel() + 1;	//	PROXYTYPE_NOPROXY not in the list
	CWnd	*pWnd = GetDlgItem(IDC_ENABLEAUTH);

	if ((iCurType != PROXYTYPE_SOCKS5) && (iCurType != PROXYTYPE_HTTP11))
	{
		m_bEnableAuth = false;
		UpdateData(TRUE);
		OnBnClickedEnableauth();
		pWnd->EnableWindow(false);
	} 
	else
		pWnd->EnableWindow(true);
}

void CPPgProxy::LoadSettings()
{
	m_bEnableProxy = m_proxy.m_bUseProxy;
	m_bEnableAuth = m_proxy.m_bEnablePassword;
	m_ProxyTypeCombo.SetCurSel(m_proxy.m_nType - 1);	//	PROXYTYPE_NOPROXY not in the list
	m_strProxyName = m_proxy.m_strName;
	m_strProxyPort.Format(_T("%u"), m_proxy.m_uPort);
	m_strProxyUser = m_proxy.m_strUser;
	m_strProxyPassword = m_proxy.m_strPassword;
	UpdateData(FALSE);
	OnBnClickedEnableproxy();
}

void CPPgProxy::Localize(void)
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_ENABLEPROXY, IDS_PROXY_ENABLED },
		{ IDC_PROXYTYPE_LBL, IDS_PROXY_TYPE },
		{ IDC_PROXYNAME_LBL, IDS_PROXY_ADDRESS },
		{ IDC_PROXYPORT_LBL, IDS_PROXY_PORT },
		{ IDC_ENABLEAUTH, IDS_AUTHENTICATION },
		{ IDC_USERNAME_LBL, IDS_PROXY_USERNAME },
		{ IDC_PASSWORD_LBL, IDS_PASSWORD },
		{ IDC_AUTH_LBL, IDS_PROXY_AUTH }
	};

	if (::IsWindow(m_hWnd))
	{
		CString	strRes;

		for (uint32 i = 0; i < ARRSIZE(s_auResTbl); i++)
		{
			::GetResString(&strRes, static_cast<UINT>(s_auResTbl[i][1]));
			SetDlgItemText(s_auResTbl[i][0], strRes);
		}
	}
}
