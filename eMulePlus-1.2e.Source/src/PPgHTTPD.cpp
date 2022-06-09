//	This file is part of eMule Plus
//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//	as published by the Free Software Foundation; either
//	version 2 of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "stdafx.h"
#include "emule.h"
#include "PPgHTTPD.h"
#include "WebServer.h"
#include "otherfunctions.h"
#include "AddBuddy.h"
#include "MMServer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#define HIDDEN_PASSWORD _T("*****")

IMPLEMENT_DYNAMIC(CPPgHTTPD, CPropertyPage)
CPPgHTTPD::CPPgHTTPD()
	: CPropertyPage(CPPgHTTPD::IDD)
	, m_bWSEnabled(FALSE)
	, m_bWSGuestEnabled(FALSE)
	, m_bWSIntruderDetectEnabled(FALSE)
	, m_bMMEnabled(FALSE)
	, m_strWSPasswd(HIDDEN_PASSWORD)
	, m_strWSGuestPasswd(HIDDEN_PASSWORD)
	, m_strMMPasswd(HIDDEN_PASSWORD)
{
}

CPPgHTTPD::~CPPgHTTPD()
{
}

void CPPgHTTPD::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_HTTPDPORT, m_strWSPort);
	DDX_Text(pDX, IDC_HTTPDPASS, m_strWSPasswd);
	DDX_Text(pDX, IDC_HTTPDPASSLOW, m_strWSGuestPasswd);
	DDX_Text(pDX, IDC_TMPLPATH, m_strTmplPath);
	DDX_Check(pDX, IDC_HTTPDENABLED, m_bWSEnabled);
	DDX_Check(pDX, IDC_HTTPDENABLEDLOW, m_bWSGuestEnabled);
	DDX_Check(pDX, IDC_HTTPDINTRUDERDETECTION, m_bWSIntruderDetectEnabled);
	DDX_Text(pDX, IDC_HTTPDTEMPDISABLELOGIN, m_strTempDisableLogin);
	DDX_Text(pDX, IDC_HTTPDLOGINATTEMPTSALLOWED, m_strLoginAttemptsAllowed);
	DDX_Text(pDX, IDC_MMPORT, m_strMMPort);
	DDX_Text(pDX, IDC_MMPASS, m_strMMPasswd);
	DDX_Check(pDX, IDC_MMENABLED, m_bMMEnabled);
}

BEGIN_MESSAGE_MAP(CPPgHTTPD, CPropertyPage)
	ON_EN_CHANGE(IDC_HTTPDPASS, OnDataChange)
	ON_EN_CHANGE(IDC_HTTPDPASSLOW, OnDataChange)
	ON_EN_CHANGE(IDC_HTTPDPORT, OnDataChange)
	ON_EN_CHANGE(IDC_TMPLPATH, OnDataChange)
	ON_BN_CLICKED(IDC_HTTPDENABLED, OnEnChangeHTTPDEnabled)
	ON_BN_CLICKED(IDC_HTTPDENABLEDLOW, OnEnChangeHTTPDGuestEnabled)
	ON_BN_CLICKED(IDC_HTTPDRELOADTMPL, OnReloadTemplates)
	ON_EN_CHANGE(IDC_HTTPDLOGINATTEMPTSALLOWED, OnDataChange)
	ON_EN_CHANGE(IDC_HTTPDTEMPDISABLELOGIN, OnDataChange)
	ON_BN_CLICKED(IDC_HTTPDINTRUDERDETECTION, OnEnChangeIntruderDetection)
	ON_BN_CLICKED(IDC_TMPLBROWSE, OnBnClickedTmplbrowse)
	ON_EN_CHANGE(IDC_MMPORT, OnDataChange)
	ON_EN_CHANGE(IDC_MMPASS, OnDataChange)
	ON_BN_CLICKED(IDC_MMENABLED, OnEnChangeMMEnabled)
END_MESSAGE_MAP()

BOOL CPPgHTTPD::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	CWnd	*pWnd = (reinterpret_cast<CEdit*>(GetDlgItem(IDC_TMPLPATH)));

	(reinterpret_cast<CEdit*>(GetDlgItem(IDC_HTTPDPASS)))->SetLimitText(32);
	(reinterpret_cast<CEdit*>(GetDlgItem(IDC_HTTPDPASSLOW)))->SetLimitText(32);
	(reinterpret_cast<CEdit*>(GetDlgItem(IDC_HTTPDPORT)))->SetLimitText(5);
	(reinterpret_cast<CEdit*>(pWnd))->SetLimitText(MAX_PATH);
	(reinterpret_cast<CEdit*>(GetDlgItem(IDC_HTTPDLOGINATTEMPTSALLOWED)))->SetLimitText(2);
	(reinterpret_cast<CEdit*>(GetDlgItem(IDC_HTTPDTEMPDISABLELOGIN)))->SetLimitText(2);
	(reinterpret_cast<CEdit*>(GetDlgItem(IDC_MMPORT)))->SetLimitText(5);
	(reinterpret_cast<CEdit*>(GetDlgItem(IDC_MMPASS)))->SetLimitText(32);

	AddBuddy(pWnd->m_hWnd, ::GetDlgItem(m_hWnd, IDC_TMPLBROWSE), BDS_RIGHT);

	LoadSettings();
	Localize();

	return TRUE;
}

void CPPgHTTPD::LoadSettings(void)
{
	m_strWSPort.Format(_T("%u"), m_pPrefs->GetWSPort());
	m_strMMPort.Format(_T("%u"), m_pPrefs->GetMMPort());
	
	m_strTmplPath = m_pPrefs->GetTemplate();
	m_bWSEnabled = m_pPrefs->GetWSIsEnabled();
	m_bWSGuestEnabled = m_pPrefs->GetWSIsLowUserEnabled();
	m_bMMEnabled = m_pPrefs->IsMMServerEnabled();

	m_bWSIntruderDetectEnabled = m_pPrefs->IsWSIntruderDetectionEnabled();

	m_strTempDisableLogin.Format(_T("%u"), m_pPrefs->GetWSTempDisableLogin());
	m_strLoginAttemptsAllowed.Format(_T("%u"), m_pPrefs->GetWSLoginAttemptsAllowed());	

	UpdateData(FALSE);

	OnEnChangeMMEnabled();
	OnEnChangeHTTPDEnabled();

	SetModified(FALSE);
}

BOOL CPPgHTTPD::OnApply()
{
	if(m_bModified)
	{
		UpdateData(TRUE);
		
		if (m_strWSPasswd != HIDDEN_PASSWORD)
			m_pPrefs->SetWSPass(m_strWSPasswd);

		if (m_strWSGuestPasswd != HIDDEN_PASSWORD)
			m_pPrefs->SetWSLowPass(m_strWSGuestPasswd);

		if (m_strMMPasswd != HIDDEN_PASSWORD)
			m_pPrefs->SetMMPass(m_strMMPasswd);

		uint32	dwVal;

		if (((dwVal = _tstoi(m_strMMPort)) - 1u) > 0xFFFEu)	//	Valid values range is 1..65535
			dwVal = m_pPrefs->GetMMPort();
		if (dwVal != m_pPrefs->GetMMPort())
		{
#ifdef OLD_SOCKETS_ENABLED
			m_pPrefs->SetMMPort(static_cast<uint16>(dwVal));
			g_App.m_pMMServer->StopServer();
			g_App.m_pMMServer->Init();
#endif
		}

		if (((dwVal = _tstoi(m_strWSPort)) - 1u) > 0xFFFEu)	//	Valid values range is 1..65535
			dwVal = m_pPrefs->GetWSPort();
		if (dwVal != m_pPrefs->GetWSPort())
		{
			m_pPrefs->SetWSPort(static_cast<uint16>(dwVal));
			g_App.m_pWebServer->RestartServer();
		}
		m_pPrefs->SetWSIsEnabled(B2b(m_bWSEnabled));
		m_pPrefs->SetWSIsLowUserEnabled(B2b(m_bWSGuestEnabled));
		m_pPrefs->SetMMIsEnabled(B2b(m_bMMEnabled));
		m_pPrefs->SetTemplate(m_strTmplPath);
		m_pPrefs->SetWSIntruderDetectionEnabled(B2b(m_bWSIntruderDetectEnabled));

		dwVal = _tstoi(m_strTempDisableLogin);
		for (;;)
		{
			if (dwVal > PREF_MAX_WSDISABLELOGIN)
				dwVal = PREF_MAX_WSDISABLELOGIN;
			else if ((dwVal - 1u) <= (PREF_MIN_WSDISABLELOGIN - 1))	// < min & != 0
				dwVal = PREF_MIN_WSDISABLELOGIN;
			else
				break;
			m_strTempDisableLogin.Format(_T("%u"), dwVal);
			break;
		}
		m_pPrefs->SetWSTempDisableLogin(dwVal);

		dwVal = _tstoi(m_strLoginAttemptsAllowed);
		for (;;)
		{
			if (dwVal > PREF_MAX_WSBADLOGINTRIES)
				dwVal = PREF_MAX_WSBADLOGINTRIES;
			else if (dwVal < PREF_MIN_WSBADLOGINTRIES)
				dwVal = PREF_MIN_WSBADLOGINTRIES;
			else
				break;
			m_strLoginAttemptsAllowed.Format(_T("%u"), dwVal);
			break;
		}
		m_pPrefs->SetWSLoginAttemptsAllowed(dwVal);

#ifdef OLD_SOCKETS_ENABLED
		if (m_bMMEnabled)
			g_App.m_pMMServer->Init();
		else
			g_App.m_pMMServer->StopServer();
#endif

		UpdateData(FALSE);

		g_App.m_pWebServer->StartServer();

		SetModified(FALSE);
	}

	return CPropertyPage::OnApply();
}

void CPPgHTTPD::Localize(void)
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_HTTPDPASS_LBL, IDS_PASSWORD },
		{ IDC_HTTPDPORT_LBL, IDS_PORT },
		{ IDC_HTTPDENABLED, IDS_ENABLED },
		{ IDC_HTTPDRELOADTMPL, IDS_HTTPD_RELOAD_TMPL },
		{ IDC_HTTPDPASS_LBL2, IDS_PASSWORD },
		{ IDC_STATIC_GENERAL, IDS_PW_GENERAL },
		{ IDC_HTTPDTEMPLATE_LBL, IDS_TEMPLATE },
		{ IDC_STATIC_LOWUSER, IDS_WEB_LOWUSER },
		{ IDC_HTTPDENABLEDLOW, IDS_ENABLED },
		{ IDC_STATIC_INTRUDER, IDS_INTR },
		{ IDC_HTTPDINTRUDERDETECTION, IDS_ENABLED },
		{ IDC_INTR_LOGIN, IDS_INTR_LOGIN },
		{ IDC_INTR_TIME, IDS_INTR_TIME },
		{ IDC_INTR_DISABLE, IDS_INTR_DISABLE },
		{ IDC_MMENABLED, IDS_ENABLED },
		{ IDC_MMPASSWORD, IDS_PASSWORD },
		{ IDC_MMPORT_LBL, IDS_PORT }
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

void CPPgHTTPD::OnEnChangeMMEnabled()
{
	UpdateData(TRUE);
	GetDlgItem(IDC_MMPASS)->EnableWindow(m_bMMEnabled);
	GetDlgItem(IDC_MMPORT)->EnableWindow(m_bMMEnabled);
	SetModified();
}

void CPPgHTTPD::OnEnChangeHTTPDEnabled()
{
	OnEnChangeIntruderDetection();

	GetDlgItem(IDC_HTTPDPASS)->EnableWindow(m_bWSEnabled);
	GetDlgItem(IDC_HTTPDPORT)->EnableWindow(m_bWSEnabled);
	GetDlgItem(IDC_HTTPDRELOADTMPL)->EnableWindow(m_bWSEnabled);
	GetDlgItem(IDC_HTTPDENABLEDLOW)->EnableWindow(m_bWSEnabled);
	GetDlgItem(IDC_HTTPDPASSLOW)->EnableWindow(m_bWSEnabled && m_bWSGuestEnabled);
	GetDlgItem(IDC_HTTPDINTRUDERDETECTION)->EnableWindow(m_bWSEnabled);
	GetDlgItem(IDC_TMPLPATH)->EnableWindow(m_bWSEnabled);
	GetDlgItem(IDC_TMPLBROWSE)->EnableWindow(m_bWSEnabled);
	SetModified();
}

void CPPgHTTPD::OnReloadTemplates()
{
	UpdateData(TRUE);
	m_pPrefs->SetTemplate(m_strTmplPath);
	g_App.m_pWebServer->ReloadTemplates();
}

void CPPgHTTPD::OnBnClickedTmplbrowse()
{
	CString	strBuf(GetResString(IDS_TEMPLATE));

	strBuf += _T(" (*.tmpl)|*.tmpl||");

	UpdateData(TRUE);
	if (DialogBrowseFile(m_strTmplPath, strBuf, NULL, OFN_FILEMUSTEXIST, true, g_App.m_pPrefs->GetAppDir()))
	{
		UpdateData(FALSE);
		SetModified();
	}
}

void CPPgHTTPD::OnEnChangeHTTPDGuestEnabled()
{
	UpdateData(TRUE);

	GetDlgItem(IDC_HTTPDPASSLOW)->EnableWindow(m_bWSGuestEnabled);

	SetModified();
}

void CPPgHTTPD::OnEnChangeIntruderDetection()
{
	UpdateData(TRUE);

	GetDlgItem(IDC_HTTPDTEMPDISABLELOGIN)->EnableWindow(m_bWSEnabled && m_bWSIntruderDetectEnabled);
	GetDlgItem(IDC_HTTPDLOGINATTEMPTSALLOWED)->EnableWindow(m_bWSEnabled && m_bWSIntruderDetectEnabled);

	SetModified();
}
