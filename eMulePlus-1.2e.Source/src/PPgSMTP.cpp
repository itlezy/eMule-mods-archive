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
#include "otherfunctions.h"
#include "PPgSMTP.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CPPgSMTP, CPropertyPage)
CPPgSMTP::CPPgSMTP()
	: CPropertyPage(CPPgSMTP::IDD)
{
}

CPPgSMTP::~CPPgSMTP()
{
}

void CPPgSMTP::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_SMTPSERVER, m_strSMTPServer);
	DDX_Text(pDX, IDC_SMTPNAME, m_strSMTPName);
	DDX_Text(pDX, IDC_SMTPFROM, m_strSMTPFrom);
	DDX_Text(pDX, IDC_SMTPTO, m_strSMTPTo);
	DDX_Text(pDX, IDC_SMTPUSERNAME, m_strSMTPUserName);
	DDX_Text(pDX, IDC_SMTPPASSWORD, m_strSMTPPassword);
	DDX_Check(pDX, IDC_SMTPAUTHENTICATED, m_bSMTPAuthenticated);
	DDX_Check(pDX, IDC_SMTPINFOENABLED, m_bSMTPInfoEnabled);
	DDX_Check(pDX, IDC_SMTPWARNINGENABLED, m_bSMTPWarningEnabled);
	DDX_Check(pDX, IDC_SMTPMSGINSUBJECT, m_bSMTPMsgInSubjEnabled);
}

BEGIN_MESSAGE_MAP(CPPgSMTP, CPropertyPage)
	ON_EN_CHANGE(IDC_SMTPSERVER, OnDataChange)
	ON_EN_CHANGE(IDC_SMTPNAME, OnDataChange)
	ON_EN_CHANGE(IDC_SMTPFROM, OnDataChange)
	ON_EN_CHANGE(IDC_SMTPTO, OnDataChange)
	ON_EN_CHANGE(IDC_SMTPUSERNAME, OnDataChange)
	ON_EN_CHANGE(IDC_SMTPPASSWORD, OnDataChange)
	ON_BN_CLICKED(IDC_SMTPAUTHENTICATED, OnEnChangeSMTPAuthenticated)
	ON_BN_CLICKED(IDC_SMTPINFOENABLED, OnDataChange)
	ON_BN_CLICKED(IDC_SMTPWARNINGENABLED, OnDataChange)
	ON_BN_CLICKED(IDC_SMTPMSGINSUBJECT, OnDataChange)
END_MESSAGE_MAP()

BOOL CPPgSMTP::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	LoadSettings();
	Localize();

	return TRUE;
}

void CPPgSMTP::LoadSettings(void)
{
	m_strSMTPServer = m_pPrefs->GetSMTPServer();
	m_strSMTPName = m_pPrefs->GetSMTPName();
	m_strSMTPFrom = m_pPrefs->GetSMTPFrom();
	m_strSMTPTo = m_pPrefs->GetSMTPTo();
	m_strSMTPUserName = m_pPrefs->GetSMTPUserName();
	m_strSMTPPassword = m_pPrefs->GetSMTPPassword();

	m_bSMTPAuthenticated = m_pPrefs->IsSMTPAuthenticated();
	m_bSMTPInfoEnabled = m_pPrefs->IsSMTPInfoEnabled();
	m_bSMTPWarningEnabled = m_pPrefs->IsSMTPWarningEnabled();
	m_bSMTPMsgInSubjEnabled = m_pPrefs->IsSMTPMsgInSubjEnabled();

	UpdateData(FALSE);

	OnEnChangeSMTPAuthenticated();

	SetModified(FALSE);
}

BOOL CPPgSMTP::OnApply()
{
	if (m_bModified)
	{
		UpdateData(TRUE);

		if (!m_strSMTPServer.IsEmpty())
			m_pPrefs->SetSMTPServer(m_strSMTPServer);
		if (!m_strSMTPName.IsEmpty())
			m_pPrefs->SetSMTPName(m_strSMTPName);
		if (!m_strSMTPFrom.IsEmpty())
			m_pPrefs->SetSMTPFrom(m_strSMTPFrom);
		if (!m_strSMTPTo.IsEmpty())
			m_pPrefs->SetSMTPTo(m_strSMTPTo);
		if (!m_strSMTPUserName.IsEmpty())
			m_pPrefs->SetSMTPUserName(m_strSMTPUserName);
		if (!m_strSMTPPassword.IsEmpty())
			m_pPrefs->SetSMTPPassword(m_strSMTPPassword);

		m_pPrefs->SetSMTPAuthenticated(B2b(m_bSMTPAuthenticated));
		m_pPrefs->SetSMTPInfoEnabled(B2b(m_bSMTPInfoEnabled));
		m_pPrefs->SetSMTPWarningEnabled(B2b(m_bSMTPWarningEnabled));
		m_pPrefs->SetSMTPMsgInSubjEnabled(B2b(m_bSMTPMsgInSubjEnabled));

		UpdateData(FALSE);

		SetModified(FALSE);
	}

	return CPropertyPage::OnApply();
}

void CPPgSMTP::Localize(void)
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_SMTPAUTHENTICATION_FRM, IDS_AUTHENTICATION },
		{ IDC_SMTPSERVER_LBL, IDS_PW_SERVER },
		{ IDC_SMTPNAME_LBL, IDS_SW_NAME },
		{ IDC_SMTPFROM_LBL, IDS_FROM },
		{ IDC_SMTPTO_LBL, IDS_TO },
		{ IDC_SMTPUSERNAME_LBL, IDS_PROXY_USERNAME },
		{ IDC_SMTPPASSWORD_LBL, IDS_PASSWORD },
		{ IDC_SMTPAUTHENTICATED, IDS_ENABLED },
		{ IDC_SMTPINFOENABLED, IDS_SMTPMSG_INFO },
		{ IDC_SMTPWARNINGENABLED, IDS_SMTPMSG_WARNING },
		{ IDC_SMTPMSGINSUBJECT, IDS_SMTPMSGINSUBJECT }
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

void CPPgSMTP::OnEnChangeSMTPAuthenticated()
{
	UpdateData(TRUE);

	GetDlgItem(IDC_SMTPUSERNAME)->EnableWindow(m_bSMTPAuthenticated);
	GetDlgItem(IDC_SMTPPASSWORD)->EnableWindow(m_bSMTPAuthenticated);

	SetModified();
}
