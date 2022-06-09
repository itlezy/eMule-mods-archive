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
#include "emuleDlg.h"
#include "ServerWnd.h"
#include "PPgServer.h"
#include "OtherFunctions.h"
#include "Preferences.h"
#include "Opcodes.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CPPgServer, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgServer, CPropertyPage)
	ON_EN_CHANGE(IDC_SERVERRETRIES, OnSettingsChange)
	ON_BN_CLICKED(IDC_AUTOSERVER, OnSettingsChange)
	ON_BN_CLICKED(IDC_SCORE, OnSettingsChange)
	ON_BN_CLICKED(IDC_SMARTIDCHECK, OnSettingsChange)
	ON_BN_CLICKED(IDC_SAFESERVERCONNECT, OnSettingsChange)
	ON_BN_CLICKED(IDC_AUTOCONNECTSTATICONLY, OnSettingsChange)
	ON_BN_CLICKED(IDC_MANUALSERVERHIGHPRIO, OnSettingsChange)
	ON_BN_CLICKED(IDC_EDITADR, OnBnClickedEditadr)
	ON_BN_CLICKED(IDC_SERVER_REQUIREOBFUSCATION, OnSettingsChange) // MORPH lh require obfuscated server connection 
	ON_EN_CHANGE(IDC_DISCONNECT_VALUE, OnSettingsChange)//morph4u server-auto-disconnect 
	ON_BN_CLICKED(IDC_DISCONNECT_CHECK, OnSettingsChange)//morph4u server-auto-disconnect 
END_MESSAGE_MAP()

CPPgServer::CPPgServer()
	: CPropertyPage(CPPgServer::IDD)
{
}

CPPgServer::~CPPgServer()
{
}

void CPPgServer::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

BOOL CPPgServer::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

    ((CEdit*)GetDlgItem(IDC_DISCONNECT_VALUE))->SetLimitText(3);//morph4u server-auto-disconnect 

	LoadSettings();
	Localize();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CPPgServer::LoadSettings(void)
{
	SetDlgItemInt(IDC_SERVERRETRIES, thePrefs.m_uDeadServerRetries, FALSE);
	CheckDlgButton(IDC_AUTOSERVER, thePrefs.m_bAutoUpdateServerList);
	CheckDlgButton(IDC_SCORE, thePrefs.m_bUseServerPriorities);
	CheckDlgButton(IDC_SMARTIDCHECK, thePrefs.m_bSmartServerIdCheck);
	CheckDlgButton(IDC_SAFESERVERCONNECT, thePrefs.m_bSafeServerConnect);
	CheckDlgButton(IDC_AUTOCONNECTSTATICONLY, thePrefs.m_bAutoConnectToStaticServersOnly);
	CheckDlgButton(IDC_MANUALSERVERHIGHPRIO, thePrefs.m_bManualAddedServersHighPriority);
	CheckDlgButton(IDC_SERVER_REQUIREOBFUSCATION, thePrefs.IsServerCryptLayerRequiredStrict()); // MORPH lh require obfuscated server connection 
	if (!thePrefs.IsClientCryptLayerSupported()) GetDlgItem(IDC_SERVER_REQUIREOBFUSCATION)->EnableWindow(FALSE); // MORPH lh require obfuscated server connection 
//morph4u server-auto-disconnect +
	CString strBuffer;
        strBuffer.Format(_T("%d"), thePrefs.DisconnectTime);
	GetDlgItem(IDC_DISCONNECT_VALUE)->SetWindowText(strBuffer);
	CheckDlgButton(IDC_DISCONNECT_CHECK, thePrefs.m_bAutoDisconnect);
//morph4u server-auto-disconnect -
}

BOOL CPPgServer::OnApply()
{
	UINT uCurDeadServerRetries = thePrefs.m_uDeadServerRetries;
	thePrefs.m_uDeadServerRetries = GetDlgItemInt(IDC_SERVERRETRIES, NULL, FALSE);
	if (thePrefs.m_uDeadServerRetries < 1)
		thePrefs.m_uDeadServerRetries = 1;
	else if (thePrefs.m_uDeadServerRetries > MAX_SERVERFAILCOUNT)
		thePrefs.m_uDeadServerRetries = MAX_SERVERFAILCOUNT;
	if (uCurDeadServerRetries != thePrefs.m_uDeadServerRetries) {
		theApp.emuledlg->serverwnd->serverlistctrl.Invalidate();
		theApp.emuledlg->serverwnd->serverlistctrl.UpdateWindow();
	}
	thePrefs.m_bAutoUpdateServerList = IsDlgButtonChecked(IDC_AUTOSERVER)!=0;
	thePrefs.m_bUseServerPriorities = IsDlgButtonChecked(IDC_SCORE)!=0;
	thePrefs.m_bSmartServerIdCheck = IsDlgButtonChecked(IDC_SMARTIDCHECK)!=0;
	thePrefs.SetSafeServerConnectEnabled(IsDlgButtonChecked(IDC_SAFESERVERCONNECT)!=0);
	thePrefs.m_bAutoConnectToStaticServersOnly = IsDlgButtonChecked(IDC_AUTOCONNECTSTATICONLY)!=0;
	thePrefs.m_bManualAddedServersHighPriority = IsDlgButtonChecked(IDC_MANUALSERVERHIGHPRIO)!=0;
    thePrefs.m_bCryptLayerRequiredStrictServer= IsDlgButtonChecked(IDC_SERVER_REQUIREOBFUSCATION)!=0; // MORPH lh require obfuscated server connection 
//morph4u server-auto-disconnect +
	TCHAR buffer[510];
         GetDlgItem(IDC_DISCONNECT_VALUE)->GetWindowText(buffer, 20);
	if (_tstoi(buffer) > 240) 
		thePrefs.SetDisconnectTime(240);
	else if (_tstoi(buffer) < 10)
		thePrefs.SetDisconnectTime(10);
	else
		thePrefs.SetDisconnectTime(_tstoi(buffer)); 

	theApp.emuledlg->serverwnd->serverlistctrl.ShowServerCount();
	thePrefs.m_bAutoDisconnect = IsDlgButtonChecked(IDC_DISCONNECT_CHECK)!=0;
//morph4u server-auto-disconnect -

	LoadSettings();

	SetModified();
	return CPropertyPage::OnApply();
}

void CPPgServer::Localize(void)
{
	if (m_hWnd)
	{
		SetWindowText(GetResString(IDS_PW_SERVER));
		GetDlgItem(IDC_LBL_UPDATE_SERVERS)->SetWindowText(GetResString(IDS_SV_UPDATE));
		GetDlgItem(IDC_LBL_MISC)->SetWindowText(GetResString(IDS_PW_MISC));
		GetDlgItem(IDC_REMOVEDEAD)->SetWindowText(GetResString(IDS_PW_RDEAD));
		GetDlgItem(IDC_RETRIES_LBL)->SetWindowText(GetResString(IDS_PW_RETRIES));
		GetDlgItem(IDC_AUTOSERVER)->SetWindowText(GetResString(IDS_PW_USS));
		GetDlgItem(IDC_SMARTIDCHECK)->SetWindowText(GetResString(IDS_SMARTLOWIDCHECK));
		GetDlgItem(IDC_SAFESERVERCONNECT)->SetWindowText(GetResString(IDS_PW_FASTSRVCON));
		GetDlgItem(IDC_SCORE)->SetWindowText(GetResString(IDS_PW_SCORE));
		GetDlgItem(IDC_MANUALSERVERHIGHPRIO)->SetWindowText(GetResString(IDS_MANUALSERVERHIGHPRIO));
		GetDlgItem(IDC_EDITADR)->SetWindowText(GetResString(IDS_EDITLIST));
		GetDlgItem(IDC_AUTOCONNECTSTATICONLY)->SetWindowText(GetResString(IDS_PW_AUTOCONNECTSTATICONLY));
		GetDlgItem(IDC_SERVER_REQUIREOBFUSCATION)->SetWindowText(GetResString(IDS_SERVER_REQUIREOBFUSCATION)); // MORPH lh require obfuscated server connection 

		GetDlgItem(IDC_DISCONNECT_CHECK)->SetWindowText(GetResString(IDS_DISCONNECT_CHECK)); //morph4u server-auto-disconnect 
	}
}

void CPPgServer::OnBnClickedEditadr()
{
	ShellExecute(NULL, _T("open"), thePrefs.GetTxtEditor(), _T("\"") + thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("addresses.dat\""), NULL, SW_SHOW); 
}
