//this file is part of eMule
//Copyright (C)2002-2007 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "PPgIRC.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CPPgIRC, CPropertyPage)
CPPgIRC::CPPgIRC()
	: CPropertyPage(CPPgIRC::IDD)
	, m_bUseChannelFilter(FALSE)
	, m_bUsePerform(FALSE)
	, m_bTimeStamp(FALSE)
	, m_bListOnConnect(FALSE)
	, m_bIgnoreInfoMessages(FALSE)
	, m_bStripColor(FALSE)
{
}

CPPgIRC::~CPPgIRC()
{
}

void CPPgIRC::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_IRC_SERVER_BOX, m_strServer);
	DDX_Text(pDX, IDC_IRC_NICK_BOX, m_strNick);
	DDX_Text(pDX, IDC_IRC_NAME_BOX, m_strName);
	DDX_Text(pDX, IDC_IRC_MINUSER_BOX, m_strMinUser);
	DDX_Check(pDX, IDC_IRC_USECHANFILTER, m_bUseChannelFilter);
	DDX_Text(pDX, IDC_IRC_PERFORM_BOX, m_strPerform);
	DDX_Check(pDX, IDC_IRC_USEPERFORM, m_bUsePerform);
	DDX_Check(pDX, IDC_IRC_TIMESTAMP, m_bTimeStamp);
	DDX_Check(pDX, IDC_IRC_LISTONCONNECT, m_bListOnConnect);
	DDX_Check(pDX, IDC_IRC_INFOMESSAGE, m_bIgnoreInfoMessages);
	DDX_Check(pDX, IDC_IRC_STRIPCOLOR, m_bStripColor);
}

BEGIN_MESSAGE_MAP(CPPgIRC, CPropertyPage)
	ON_BN_CLICKED(IDC_IRC_TIMESTAMP, OnSettingsChange)
	ON_BN_CLICKED(IDC_IRC_USECHANFILTER, OnBnClickedUseFilter)
	ON_BN_CLICKED(IDC_IRC_USEPERFORM, OnBnClickedUsePerform)
	ON_BN_CLICKED(IDC_IRC_INFOMESSAGE, OnSettingsChange)
	ON_BN_CLICKED(IDC_IRC_STRIPCOLOR, OnSettingsChange)
	ON_EN_CHANGE(IDC_IRC_NICK_BOX, OnEnChangeNick)
	ON_EN_CHANGE(IDC_IRC_PERFORM_BOX, OnSettingsChange)
	ON_EN_CHANGE(IDC_IRC_SERVER_BOX, OnSettingsChange)
	ON_EN_CHANGE(IDC_IRC_NAME_BOX, OnSettingsChange)
	ON_EN_CHANGE(IDC_IRC_MINUSER_BOX, OnSettingsChange)
	ON_BN_CLICKED(IDC_IRC_LISTONCONNECT, OnSettingsChange)
END_MESSAGE_MAP()


BOOL CPPgIRC::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	(reinterpret_cast<CEdit*>(GetDlgItem(IDC_IRC_NICK_BOX)))->SetLimitText(20);
	(reinterpret_cast<CEdit*>(GetDlgItem(IDC_IRC_MINUSER_BOX)))->SetLimitText(6);
	(reinterpret_cast<CEdit*>(GetDlgItem(IDC_IRC_SERVER_BOX)))->SetLimitText(40);
	(reinterpret_cast<CEdit*>(GetDlgItem(IDC_IRC_NAME_BOX)))->SetLimitText(40);
	(reinterpret_cast<CEdit*>(GetDlgItem(IDC_IRC_PERFORM_BOX)))->SetLimitText(250);

	LoadSettings();
	Localize();
	m_bNickModified = false;

	return TRUE;
}

void CPPgIRC::LoadSettings(void)
{
	m_bTimeStamp = m_pPrefs->GetIRCAddTimestamp();
	m_bIgnoreInfoMessages = m_pPrefs->GetIrcIgnoreInfoMessage();
	m_bStripColor = m_pPrefs->GetIrcStripColor();
	m_bUseChannelFilter = m_pPrefs->GetIRCUseChanFilter();
	m_bUsePerform = m_pPrefs->GetIrcUsePerform();
	m_bListOnConnect = m_pPrefs->GetIRCListOnConnect();
	m_strServer = m_pPrefs->GetIRCServer();
	m_strNick = m_pPrefs->GetIRCNick();
	m_strName = m_pPrefs->GetIRCChanNameFilter();
	m_strPerform = m_pPrefs->GetIrcPerformString();
	m_strMinUser.Format(_T("%u"), m_pPrefs->GetIRCChannelUserFilter());

	UpdateData(FALSE);

	OnBnClickedUseFilter();
	OnBnClickedUsePerform();

	SetModified(FALSE);
}


BOOL CPPgIRC::OnApply()
{
	if (m_bModified)
	{
		UpdateData(TRUE);

		m_pPrefs->SetIRCAddTimestamp(B2b(m_bTimeStamp));
		m_pPrefs->SetIrcIgnoreInfoMessage(B2b(m_bIgnoreInfoMessages));
		m_pPrefs->SetIrcStripColor(B2b(m_bStripColor));
		m_pPrefs->SetIRCListonConnect(B2b(m_bListOnConnect));
		m_pPrefs->SetIRCUseChanFilter(B2b(m_bUseChannelFilter));
		m_pPrefs->SetIrcUsePerform(B2b(m_bUsePerform));

		if (!m_strNick.IsEmpty())
		{
			m_pPrefs->SetIRCNick(m_strNick);
			if (g_App.m_pMDlg->m_wndIRC.GetLoggedIn() && m_bNickModified)
			{
				CString	strTmp(_T("NICK "));

				m_bNickModified = false;
				strTmp += m_strNick;
				g_App.m_pMDlg->m_wndIRC.SendString(strTmp);
			}
		}

		if (!m_strServer.IsEmpty())
			m_pPrefs->SetIRCServer(m_strServer);

		if (!m_strName.IsEmpty())
			m_pPrefs->SetIRCChanNameFilter(m_strName);

		if (!m_strPerform.IsEmpty())
			m_pPrefs->SetIRCPerformString(m_strPerform);
		else
			m_pPrefs->SetIRCPerformString(_T(" "));

		m_pPrefs->SetIRCChanUserFilter(static_cast<uint16>(_tstoi(m_strMinUser)));

		SetModified(FALSE);
	}

	return CPropertyPage::OnApply();
}

void CPPgIRC::Localize(void)
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_IRC_SERVER_FRM, IDS_PW_SERVER },
		{ IDC_IRC_MISC_FRM, IDS_PW_MISC },
		{ IDC_IRC_TIMESTAMP, IDS_IRC_ADDTIMESTAMP },
		{ IDC_IRC_NICK_FRM, IDS_NICK },
		{ IDC_IRC_NAME_TEXT, IDS_IRC_NAME },
		{ IDC_IRC_MINUSER_TEXT, IDS_UUSERS },
		{ IDC_IRC_FILTER_FRM, IDS_IRC_CHANNELLIST },
		{ IDC_IRC_USECHANFILTER, IDS_IRC_USEFILTER },
		{ IDC_IRC_PERFORM_FRM, IDS_IRC_PERFORM },
		{ IDC_IRC_USEPERFORM, IDS_IRC_USEPERFORM },
		{ IDC_IRC_LISTONCONNECT, IDS_IRC_LOADCHANNELLISTONCON },
		{ IDC_IRC_INFOMESSAGE, IDS_IRC_IGNOREINFOMESSAGE },
		{ IDC_IRC_STRIPCOLOR, IDS_IRC_STRIPCOLOR }
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

void CPPgIRC::OnBnClickedUsePerform()
{
	UpdateData(TRUE);
	GetDlgItem(IDC_IRC_PERFORM_BOX)->EnableWindow(m_bUsePerform);

	SetModified();
}

void CPPgIRC::OnBnClickedUseFilter()
{
	UpdateData(TRUE);
	GetDlgItem(IDC_IRC_NAME_BOX)->EnableWindow(m_bUseChannelFilter);
	GetDlgItem(IDC_IRC_MINUSER_BOX)->EnableWindow(m_bUseChannelFilter);

	SetModified();
}
