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
#include "PPgServer.h"
#include "Inputbox.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CPPgServer, CPropertyPage)
CPPgServer::CPPgServer()
	: CPropertyPage(CPPgServer::IDD)
	, m_bRemoveDead(FALSE)
	, m_bAutoServer(FALSE)
	, m_bSmartId(FALSE)
	, m_bAddSrvFromServer(FALSE)
	, m_bAddSrvFromClients(FALSE)
	, m_bAutoConnect(FALSE)
	, m_bAutoConnectStatic(FALSE)
	, m_bReconnect(FALSE)
	, m_bScoreSystem(FALSE)
	, m_bManualSrvHighPriority(FALSE)
	, m_bRestartWaiting(FALSE)
	, m_bUseAuxPort(FALSE)
{
}

CPPgServer::~CPPgServer()
{
}

void CPPgServer::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_REMOVEDEAD, m_bRemoveDead);
	DDX_Check(pDX, IDC_AUTOSERVER, m_bAutoServer);
	DDX_Check(pDX, IDC_SMARTIDCHECK, m_bSmartId);
	DDX_Check(pDX, IDC_UPDATESERVERCONNECT, m_bAddSrvFromServer);
	DDX_Check(pDX, IDC_ADDSRVFROMCLIENTS, m_bAddSrvFromClients);
	DDX_Check(pDX, IDC_RESTARTWAITING, m_bRestartWaiting);
	DDX_Check(pDX, IDC_AUTOCONNECT, m_bAutoConnect);
	DDX_Check(pDX, IDC_AUTOCONNECTSTATICONLY, m_bAutoConnectStatic);
	DDX_Check(pDX, IDC_RECONN, m_bReconnect);
	DDX_Check(pDX, IDC_SCORE, m_bScoreSystem);
	DDX_Check(pDX, IDC_MANUALSERVERHIGHPRIO, m_bManualSrvHighPriority);
	DDX_Text(pDX, IDC_SRVCONTIMEOUT, m_strSrvConnectTimeout);
	DDX_Text(pDX, IDC_SERVERKEEPALIVE, m_strSrvKeepAliveTimeout);
	DDX_Text(pDX, IDC_SERVERRETRIES, m_strServerRetries);
	DDX_Check(pDX, IDC_USE_AUX_PORT_CHECKBOX, m_bUseAuxPort);
}

BEGIN_MESSAGE_MAP(CPPgServer, CPropertyPage)
	ON_EN_CHANGE(IDC_SRVCONTIMEOUT, OnSettingsChange)
	ON_EN_CHANGE(IDC_SERVERRETRIES, OnSettingsChange)
	ON_EN_CHANGE(IDC_SERVERKEEPALIVE, OnSettingsChange)
	ON_BN_CLICKED(IDC_AUTOSERVER, OnSettingsChange)
	ON_BN_CLICKED(IDC_UPDATESERVERCONNECT, OnSettingsChange)
	ON_BN_CLICKED(IDC_ADDSRVFROMCLIENTS, OnSettingsChange)
	ON_BN_CLICKED(IDC_SCORE, OnSettingsChange)
	ON_BN_CLICKED(IDC_REMOVEDEAD, OnBnClickedRemovedead)
	ON_BN_CLICKED(IDC_RESTARTWAITING, OnSettingsChange)
	ON_BN_CLICKED(IDC_SMARTIDCHECK, OnSettingsChange)
	ON_BN_CLICKED(IDC_AUTOCONNECT, OnSettingsChange)
	ON_BN_CLICKED(IDC_RECONN, OnSettingsChange)
	ON_BN_CLICKED(IDC_AUTOCONNECTSTATICONLY, OnSettingsChange)
	ON_BN_CLICKED(IDC_MANUALSERVERHIGHPRIO, OnSettingsChange)
	ON_BN_CLICKED(IDC_ICC_BUTTON, OnSetURLsForICC)
	ON_BN_CLICKED(IDC_USE_AUX_PORT_CHECKBOX, OnSettingsChange)
END_MESSAGE_MAP()

BOOL CPPgServer::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	(reinterpret_cast<CEdit*>(GetDlgItem(IDC_SERVERKEEPALIVE)))->SetLimitText(2);
	(reinterpret_cast<CEdit*>(GetDlgItem(IDC_SERVERRETRIES)))->SetLimitText(2);

	LoadSettings();
	Localize();

	return TRUE;
}

void CPPgServer::LoadSettings(void)
{
	m_strSrvConnectTimeout.Format(_T("%u"), m_pPrefs->SrvConTimeout() / 1000);
	m_strServerRetries.Format(_T("%u"), m_pPrefs->GetDeadserverRetries());
	m_strSrvKeepAliveTimeout.Format(_T("%u"), m_pPrefs->GetServerKeepAliveTimeout() / 60000);

	m_bManualSrvHighPriority = m_pPrefs->GetManuallyAddedServerHighPrio();
	m_bSmartId = m_pPrefs->GetSmartIdCheck();
	m_bRemoveDead = m_pPrefs->DeadServer();
	m_bAutoServer = m_pPrefs->AutoServerlist();
	m_bAddSrvFromServer = m_pPrefs->GetAddServersFromServer();
	m_bAddSrvFromClients = m_pPrefs->GetAddServersFromClients();
	m_bScoreSystem = m_pPrefs->GetUseServerPriorities();
	m_bRestartWaiting = m_pPrefs->RestartWaiting();
	m_bReconnect = m_pPrefs->Reconnect();
	m_bAutoConnect = m_pPrefs->DoAutoConnect();
	m_bAutoConnectStatic = m_pPrefs->AutoConnectStaticOnly();

	m_bUseAuxPort = m_pPrefs->IsServerAuxPortUsed();

	UpdateData(FALSE);
	OnBnClickedRemovedead();
	SetModified(FALSE);
}

BOOL CPPgServer::OnApply()
{
	if(m_bModified)
	{
		UpdateData(TRUE);

		m_pPrefs->SetDeadServer(B2b(m_bRemoveDead));
		m_pPrefs->SetSmartIdCheck(B2b(m_bSmartId));

		int	iVal = _tstoi(m_strSrvConnectTimeout);
		if (iVal < PREF_MIN_SRVCONNTIMEOUT)
			iVal = PREF_MIN_SRVCONNTIMEOUT;
		if (iVal > PREF_MAX_SRVCONNTIMEOUT)
			iVal = PREF_MAX_SRVCONNTIMEOUT;

		m_pPrefs->SetSrvConTimeout(iVal * 1000);
		m_strSrvConnectTimeout.Format(_T("%u"), iVal);

		iVal = _tstoi(m_strServerRetries);
		if ((unsigned)(iVal - 1) > (PREF_MAX_DEADSRVRETRY - 1))
		{
			iVal = m_pPrefs->GetDeadserverRetries();
			m_strServerRetries.Format(_T("%u"), iVal);
		}
		m_pPrefs->SetDeadserverRetries(iVal);

		m_pPrefs->SetServerKeepAliveTimeout(_tstoi(m_strSrvKeepAliveTimeout) * 60000);

		m_pPrefs->SetUseServerPriorities(B2b(m_bScoreSystem));
		m_pPrefs->SetAutoServerlist(B2b(m_bAutoServer));
		m_pPrefs->SetAddServersFromServer(B2b(m_bAddSrvFromServer));
		m_pPrefs->SetAddServersFromClients(B2b(m_bAddSrvFromClients));
		m_pPrefs->SetRestartWaiting(B2b(m_bRestartWaiting));
		m_pPrefs->SetAutoConnect(B2b(m_bAutoConnect));
		m_pPrefs->SetReconnect(B2b(m_bReconnect));
		m_pPrefs->SetManuallyAddedServerHighPrio(B2b(m_bManualSrvHighPriority));
		m_pPrefs->SetAutoConnectStaticOnly(B2b(m_bAutoConnectStatic));
		m_pPrefs->SetServerAuxPortUsed(B2b(m_bUseAuxPort));

		SetModified(FALSE);
	}

	return CPropertyPage::OnApply();
}

void CPPgServer::Localize(void)
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_SRVCONTIMEOUT_LBL, IDS_SRVCONTIMEOUT_LBL },
		{ IDC_SRV_SEC, IDS_SECS },
		{ IDC_REMOVEDEAD, IDS_PW_RDEAD },
		{ IDC_RETRIES_LBL, IDS_PW_RETRIES },
		{ IDC_SERVERKEEPALIVE_LBL, IDS_PW_KEEPALIVE },
		{ IDC_SRV_MIN, IDS_MINS },
		{ IDC_UPDATESERVERCONNECT, IDS_PW_USC },
		{ IDC_ADDSRVFROMCLIENTS, IDS_ADDSRVFROMCLIENTS },
		{ IDC_AUTOSERVER, IDS_PW_USS },
		{ IDC_SCORE, IDS_PW_SCORE },
		{ IDC_RESTARTWAITING, IDS_RESTARTWAITING },
		{ IDC_SMARTIDCHECK, IDS_SMARTLOWIDCHECK },
		{ IDC_AUTOCONNECT, IDS_PW_AUTOCON },
		{ IDC_RECONN, IDS_PW_RECON },
		{ IDC_MANUALSERVERHIGHPRIO, IDS_MANUALSERVERHIGHPRIO },
		{ IDC_AUTOCONNECTSTATICONLY, IDS_PW_AUTOCONNECTSTATICONLY },
		{ IDC_ICC_BUTTON, IDS_ICC_BUTTON },
		{ IDC_USE_AUX_PORT_CHECKBOX, IDS_USE_AUX_PORT_CHECKBOX }
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

void CPPgServer::OnBnClickedRemovedead()
{
	UpdateData(TRUE);
	GetDlgItem(IDC_SERVERRETRIES)->EnableWindow(m_bRemoveDead);

	SetModified();
}

void CPPgServer::OnSetURLsForICC()
{
	InputBox inputbox(GetResString(IDS_ICC_INPUTBOX), g_App.m_pPrefs->GetURLsForICC());

	inputbox.DoModal();
	if (!inputbox.WasCancelled())
	{
		g_App.m_pPrefs->SetURLsForICC(inputbox.GetInput());
		g_App.InitURLs();
		SetModified();
	}
}
