//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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
#include "SivkaFileSettings.h"
#include "OtherFunctions.h" // for GetResString()

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CSivkaFileSettings, CDialog)
CSivkaFileSettings::CSivkaFileSettings()
	: CDialog(CSivkaFileSettings::IDD, 0)
{
}

CSivkaFileSettings::~CSivkaFileSettings()
{
}

void CSivkaFileSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSivkaFileSettings, CDialog)
	ON_BN_CLICKED(IDC_REMOVENNSLIMITLABEL, OnBnClickedEnableAutoNNS)
	ON_BN_CLICKED(IDC_REMOVEFQSLIMITLABEL, OnBnClickedEnableAutoFQS)
	ON_BN_CLICKED(IDC_REMOVEQRSLIMITLABEL, OnBnClickedEnableAutoQRS)
	ON_BN_CLICKED(IDC_TAKEOVER, OnBnClickedTakeOver)
	ON_BN_CLICKED(IDC_DEFAULT_BUTTON, OnBnClickedSwitch)
	ON_BN_CLICKED(IDC_HARDLIMIT_TAKEOVER, OnBnClickedMaxSourcesPerFileTakeOver)

	ON_BN_CLICKED(IDC_REMOVENNSLIMITLABEL_TAKEOVER, OnBnClickedEnableAutoDropNNSTakeOver)
	ON_BN_CLICKED(IDC_NNS_TIMER_TAKEOVER, OnBnClickedAutoNNS_TimerTakeOver)
	ON_BN_CLICKED(IDC_MAXREMOVENNSLIMIT_TAKEOVER, OnBnClickedMaxRemoveNNSLimitTakeOver)

	ON_BN_CLICKED(IDC_REMOVEFQSLIMITLABEL_TAKEOVER, OnBnClickedEnableAutoDropFQSTakeOver)
	ON_BN_CLICKED(IDC_FQS_TIMER_TAKEOVER, OnBnClickedAutoFQS_TimerTakeOver)
	ON_BN_CLICKED(IDC_MAXREMOVEFQSLIMIT_TAKEOVER, OnBnClickedMaxRemoveFQSLimitTakeOver)

	ON_BN_CLICKED(IDC_REMOVEQRSLIMITLABEL_TAKEOVER, OnBnClickedEnableAutoDropQRSTakeOver)
	ON_BN_CLICKED(IDC_HQRS_TIMER_TAKEOVER, OnBnClickedAutoHQRS_TimerTakeOver)
	ON_BN_CLICKED(IDC_REMOVEQRS_TAKEOVER, OnBnClickedMaxRemoveQRSTakeOver)
	ON_BN_CLICKED(IDC_MAXREMOVEQRSLIMIT_TAKEOVER, OnBnClickedMaxRemoveQRSLimitTakeOver)
	ON_BN_CLICKED(IDC_HQR_XMAN_TAKEOVER, OnBnClickedHQRXmanTakeOver)
	ON_BN_CLICKED(IDC_HQR_XMAN, OnBnClickedHQRXman)
	ON_BN_CLICKED(IDC_GLOBAL_HL_TAKEOVER, OnBnClickedGlobalHlTakeOver) // Global Source Limit (customize for files) - Stulle
END_MESSAGE_MAP()

BOOL CSivkaFileSettings::OnInitDialog()
{
	app_prefs->m_MaxSourcesPerFileTakeOver = false;
	app_prefs->m_EnableAutoDropNNSTakeOver = false;
	app_prefs->m_AutoNNS_TimerTakeOver = false;
	app_prefs->m_MaxRemoveNNSLimitTakeOver = false;
	app_prefs->m_EnableAutoDropFQSTakeOver = false;
	app_prefs->m_AutoFQS_TimerTakeOver = false;
	app_prefs->m_MaxRemoveFQSLimitTakeOver = false;
	app_prefs->m_EnableAutoDropQRSTakeOver = false;
	app_prefs->m_AutoHQRS_TimerTakeOver = false;
	app_prefs->m_MaxRemoveQRSTakeOver = false;
	app_prefs->m_MaxRemoveQRSLimitTakeOver = false;
	app_prefs->m_bHQRXmanTakeOver = false;
	app_prefs->m_bGlobalHlTakeOver = false; // Global Source Limit (customize for files) - Stulle
	
	m_RestoreDefault = false;
	app_prefs->m_TakeOverFileSettings = false;
	
	CDialog::OnInitDialog();

	LoadSettings();
	Localize();

	return TRUE;
}

void CSivkaFileSettings::LoadSettings(void)
{
	if(m_hWnd)
	{
		CString strBuffer;

		strBuffer.Format(_T("%d"), app_prefs->m_MaxSourcesPerFileTemp);
		GetDlgItem(IDC_HARDLIMIT)->SetWindowText(strBuffer);
		CheckDlgButton(IDC_HARDLIMIT_TAKEOVER, app_prefs->m_MaxSourcesPerFileTakeOver);
		OnBnClickedMaxSourcesPerFileTakeOver();
		// ==> Global Source Limit (customize for files) - Stulle
		CheckDlgButton(IDC_GLOBAL_HL, app_prefs->m_bGlobalHlTemp);
		CheckDlgButton(IDC_GLOBAL_HL_TAKEOVER, app_prefs->m_bGlobalHlTakeOver);
		OnBnClickedGlobalHlTakeOver();
		// <== Global Source Limit (customize for files) - Stulle

		CheckDlgButton(IDC_REMOVENNSLIMITLABEL, app_prefs->m_EnableAutoDropNNSTemp);
		CheckDlgButton(IDC_REMOVENNSLIMITLABEL_TAKEOVER, app_prefs->m_EnableAutoDropNNSTakeOver);
		strBuffer.Format(_T("%d"), app_prefs->m_AutoNNS_TimerTemp/1000);
		GetDlgItem(IDC_NNS_TIMER)->SetWindowText(strBuffer);
		strBuffer.Format(_T("%d"), app_prefs->m_MaxRemoveNNSLimitTemp);
		GetDlgItem(IDC_MAXREMOVENNSLIMIT)->SetWindowText(strBuffer);
		OnBnClickedEnableAutoDropNNSTakeOver();

		CheckDlgButton(IDC_REMOVEFQSLIMITLABEL, app_prefs->m_EnableAutoDropFQSTemp);
		CheckDlgButton(IDC_REMOVEFQSLIMITLABEL_TAKEOVER, app_prefs->m_EnableAutoDropFQSTakeOver);
		strBuffer.Format(_T("%d"), app_prefs->m_AutoFQS_TimerTemp/1000);
		GetDlgItem(IDC_FQS_TIMER)->SetWindowText(strBuffer);
		strBuffer.Format(_T("%d"), app_prefs->m_MaxRemoveFQSLimitTemp);
		GetDlgItem(IDC_MAXREMOVEFQSLIMIT)->SetWindowText(strBuffer);
		OnBnClickedEnableAutoDropFQSTakeOver();

		CheckDlgButton(IDC_REMOVEQRSLIMITLABEL, app_prefs->m_EnableAutoDropQRSTemp);
		CheckDlgButton(IDC_REMOVEQRSLIMITLABEL_TAKEOVER, app_prefs->m_EnableAutoDropQRSTakeOver);
		strBuffer.Format(_T("%d"), app_prefs->m_AutoHQRS_TimerTemp/1000);
		GetDlgItem(IDC_HQRS_TIMER)->SetWindowText(strBuffer);
		strBuffer.Format(_T("%d"), app_prefs->m_MaxRemoveQRSTemp);
		GetDlgItem(IDC_REMOVEQRS)->SetWindowText(strBuffer);
		strBuffer.Format(_T("%d"), app_prefs->m_MaxRemoveQRSLimitTemp);
		GetDlgItem(IDC_MAXREMOVEQRSLIMIT)->SetWindowText(strBuffer);
		CheckDlgButton(IDC_HQR_XMAN, app_prefs->m_bHQRXmanTemp);
		CheckDlgButton(IDC_HQR_XMAN_TAKEOVER, app_prefs->m_bHQRXmanTakeOver);
		OnBnClickedEnableAutoDropQRSTakeOver();
	}
}

void CSivkaFileSettings::OnBnClickedTakeOver()
{
	TCHAR buffer[510];

	app_prefs->m_MaxSourcesPerFileTakeOver = (IsDlgButtonChecked(IDC_HARDLIMIT_TAKEOVER))!=0;
	if(GetDlgItem(IDC_HARDLIMIT)->GetWindowTextLength() && thePrefs.m_MaxSourcesPerFileTakeOver)
	{
		GetDlgItem(IDC_HARDLIMIT)->GetWindowText(buffer,20);
			app_prefs->m_MaxSourcesPerFileTemp = (uint16)_tstoi(buffer);
	}
	// ==> Global Source Limit (customize for files) - Stulle
	app_prefs->m_bGlobalHlTakeOver = (IsDlgButtonChecked(IDC_GLOBAL_HL_TAKEOVER))!=0;
	if(app_prefs->m_bGlobalHlTakeOver)
		app_prefs->m_bGlobalHlTemp = (IsDlgButtonChecked(IDC_GLOBAL_HL))!=0;
	// <== Global Source Limit (customize for files) - Stulle

	app_prefs->m_EnableAutoDropNNSTakeOver = (IsDlgButtonChecked(IDC_REMOVENNSLIMITLABEL_TAKEOVER))!=0;
	if(app_prefs->m_EnableAutoDropNNSTakeOver)
		app_prefs->m_EnableAutoDropNNSTemp = (IsDlgButtonChecked(IDC_REMOVENNSLIMITLABEL))!=0;
	app_prefs->m_AutoNNS_TimerTakeOver = (IsDlgButtonChecked(IDC_NNS_TIMER_TAKEOVER))!=0;
	if(GetDlgItem(IDC_NNS_TIMER)->GetWindowTextLength() && app_prefs->m_AutoNNS_TimerTakeOver && app_prefs->m_EnableAutoDropNNSTemp)
	{
		GetDlgItem(IDC_NNS_TIMER)->GetWindowText(buffer,20);
		if (_tstoi(buffer) >= 0 && _tstoi(buffer) <= 60)
			app_prefs->m_AutoNNS_TimerTemp = _tstoi(buffer)*1000;
	}
	app_prefs->m_MaxRemoveNNSLimitTakeOver = (IsDlgButtonChecked(IDC_MAXREMOVENNSLIMIT_TAKEOVER))!=0;
	if(GetDlgItem(IDC_MAXREMOVENNSLIMIT)->GetWindowTextLength() && app_prefs->m_MaxRemoveNNSLimitTakeOver && app_prefs->m_EnableAutoDropNNSTemp)
	{
		GetDlgItem(IDC_MAXREMOVENNSLIMIT)->GetWindowText(buffer,20);
		if (_tstoi(buffer) >= 50 && _tstoi(buffer) <= 100)
			app_prefs->m_MaxRemoveNNSLimitTemp = (uint16)_tstoi(buffer);
	}

	app_prefs->m_EnableAutoDropFQSTakeOver = (IsDlgButtonChecked(IDC_REMOVEFQSLIMITLABEL_TAKEOVER))!=0;
	if(app_prefs->m_EnableAutoDropFQSTakeOver)
		app_prefs->m_EnableAutoDropFQSTemp = (IsDlgButtonChecked(IDC_REMOVEFQSLIMITLABEL))!=0;
	app_prefs->m_AutoFQS_TimerTakeOver = (IsDlgButtonChecked(IDC_FQS_TIMER_TAKEOVER))!=0;
	if(GetDlgItem(IDC_FQS_TIMER)->GetWindowTextLength() && app_prefs->m_AutoFQS_TimerTakeOver && app_prefs->m_EnableAutoDropFQSTemp)
	{
		GetDlgItem(IDC_FQS_TIMER)->GetWindowText(buffer,20);
		if (_tstoi(buffer) >= 0 && _tstoi(buffer) <= 60)
			app_prefs->m_AutoFQS_TimerTemp = _tstoi(buffer)*1000;
	}
	app_prefs->m_MaxRemoveFQSLimitTakeOver = (IsDlgButtonChecked(IDC_MAXREMOVEFQSLIMIT_TAKEOVER))!=0;
	if(GetDlgItem(IDC_MAXREMOVEFQSLIMIT)->GetWindowTextLength() && app_prefs->m_MaxRemoveFQSLimitTakeOver && app_prefs->m_EnableAutoDropFQSTemp)
	{
		GetDlgItem(IDC_MAXREMOVEFQSLIMIT)->GetWindowText(buffer,20);
		if (_tstoi(buffer) >= 50 && _tstoi(buffer) <= 100)
			app_prefs->m_MaxRemoveFQSLimitTemp = (uint16)_tstoi(buffer);
	}

	app_prefs->m_EnableAutoDropQRSTakeOver = (IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL_TAKEOVER))!=0;
	if(app_prefs->m_EnableAutoDropQRSTakeOver)
		app_prefs->m_EnableAutoDropQRSTemp = (IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL))!=0;
	app_prefs->m_AutoHQRS_TimerTakeOver = (IsDlgButtonChecked(IDC_HQRS_TIMER_TAKEOVER))!=0;
	if(GetDlgItem(IDC_HQRS_TIMER)->GetWindowTextLength() && app_prefs->m_AutoHQRS_TimerTakeOver && app_prefs->m_EnableAutoDropQRSTemp)
	{
		GetDlgItem(IDC_HQRS_TIMER)->GetWindowText(buffer,20);
		if (_tstoi(buffer) >= 0 && _tstoi(buffer) <= 120)
			app_prefs->m_AutoHQRS_TimerTemp = _tstoi(buffer)*1000;
	}
	app_prefs->m_MaxRemoveQRSTakeOver = (IsDlgButtonChecked(IDC_REMOVEQRS_TAKEOVER))!=0;
	if(GetDlgItem(IDC_REMOVEQRS)->GetWindowTextLength() && app_prefs->m_MaxRemoveQRSTakeOver && app_prefs->m_EnableAutoDropQRSTemp)
	{
		GetDlgItem(IDC_REMOVEQRS)->GetWindowText(buffer,20);
		if (_tstoi(buffer) >= 1000 && _tstoi(buffer) <= 10000)
			app_prefs->m_MaxRemoveQRSTemp = (uint16)_tstoi(buffer);
	}
	app_prefs->m_MaxRemoveQRSLimitTakeOver = (IsDlgButtonChecked(IDC_MAXREMOVEQRSLIMIT_TAKEOVER))!=0;
	if(GetDlgItem(IDC_MAXREMOVEQRSLIMIT)->GetWindowTextLength() && app_prefs->m_MaxRemoveQRSLimitTakeOver && app_prefs->m_EnableAutoDropQRSTemp)
	{
		GetDlgItem(IDC_MAXREMOVEQRSLIMIT)->GetWindowText(buffer,20);
		if (_tstoi(buffer) >= 50 && _tstoi(buffer) <= 100)
			app_prefs->m_MaxRemoveQRSLimitTemp = (uint16)_tstoi(buffer);
	}
	app_prefs->m_bHQRXmanTakeOver = (IsDlgButtonChecked(IDC_HQR_XMAN_TAKEOVER))!=0;
	if(app_prefs->m_bHQRXmanTakeOver)
		app_prefs->m_bHQRXmanTemp = (IsDlgButtonChecked(IDC_HQR_XMAN))!=0;

	LoadSettings();
	app_prefs->m_TakeOverFileSettings = true;
}

void CSivkaFileSettings::OnBnClickedSwitch()
{
	if(m_RestoreDefault)
	{
		LoadSettings();
		GetDlgItem(IDC_DEFAULT_BUTTON)->SetWindowText(GetResString(IDS_DEFAULT));
		m_RestoreDefault = false;
	}
	else
	{
		SetWithDefaultValues();
		GetDlgItem(IDC_DEFAULT_BUTTON)->SetWindowText(GetResString(IDS_MAIN_POPUP_RESTORE));
		m_RestoreDefault = true;
	}
}

void CSivkaFileSettings::Localize(void)
{	
	if(m_hWnd)
	{
		SetWindowText(GetResString(IDS_SIVKAFILESETTINGS));
		GetDlgItem(IDC_HARDLIMIT_LABEL)->SetWindowText(GetResString(IDS_HARDLIMIT_LABEL));
		GetDlgItem(IDC_GLOBAL_HL)->SetWindowText(GetResString(IDS_GLOBAL_HL)); // Global Source Limit (customize for files) - Stulle
		GetDlgItem(IDC_NNS_TIMERLABEL)->SetWindowText(GetResString(IDS_NNS_TIMERLABEL));
		GetDlgItem(IDC_REMOVENNSLIMITLABEL)->SetWindowText(GetResString(IDS_REMOVENNSLIMITLABEL));
		GetDlgItem(IDC_FQS_TIMERLABEL)->SetWindowText(GetResString(IDS_FQS_TIMERLABEL));
		GetDlgItem(IDC_REMOVEFQSLIMITLABEL)->SetWindowText(GetResString(IDS_REMOVEFQSLIMITLABEL));
		GetDlgItem(IDC_HQRS_TIMERLABEL)->SetWindowText(GetResString(IDS_HQRS_TIMERLABEL));
		GetDlgItem(IDC_REMOVEQRSLABEL)->SetWindowText(GetResString(IDS_REMOVEQRSLABEL));
		GetDlgItem(IDC_REMOVEQRSLIMITLABEL)->SetWindowText(GetResString(IDS_REMOVEQRSLIMITLABEL));
		GetDlgItem(IDC_HQR_XMAN)->SetWindowText(GetResString(IDS_XMAN_DROPPING));
		GetDlgItem(IDC_TAKEOVER)->SetWindowText(GetResString(IDS_TAKEOVER));
		GetDlgItem(IDC_DEFAULT_BUTTON)->SetWindowText(GetResString(IDS_DEFAULT));
		GetDlgItem(IDOK)->SetWindowText(GetResString(IDS_FD_CLOSE));
	}
}

void CSivkaFileSettings::SetWithDefaultValues()
{
	if(m_hWnd)
	{
		CString strBuffer;

		GetDlgItem(IDC_HARDLIMIT_LABEL)->EnableWindow(true);
		GetDlgItem(IDC_HARDLIMIT)->EnableWindow(true);
		strBuffer.Format(_T("%d"), 0);
		GetDlgItem(IDC_HARDLIMIT)->SetWindowText(strBuffer);
		CheckDlgButton(IDC_HARDLIMIT_TAKEOVER, true);
		// ==> Global Source Limit (customize for files) - Stulle
		CheckDlgButton(IDC_GLOBAL_HL_TAKEOVER, true);
		GetDlgItem(IDC_GLOBAL_HL)->EnableWindow(true);
		CheckDlgButton(IDC_GLOBAL_HL, thePrefs.GetGlobalHlDefault());
		// <== Global Source Limit (customize for files) - Stulle

		GetDlgItem(IDC_REMOVENNSLIMITLABEL)->EnableWindow(true);
		CheckDlgButton(IDC_REMOVENNSLIMITLABEL, thePrefs.GetEnableAutoDropNNSDefault());
		CheckDlgButton(IDC_REMOVENNSLIMITLABEL_TAKEOVER, true);
		GetDlgItem(IDC_NNS_TIMERLABEL)->EnableWindow(thePrefs.GetEnableAutoDropNNSDefault());
		GetDlgItem(IDC_NNS_TIMER)->EnableWindow(thePrefs.GetEnableAutoDropNNSDefault());
		strBuffer.Format(_T("%d"), thePrefs.GetAutoNNS_TimerDefault()/1000);
		GetDlgItem(IDC_NNS_TIMER)->SetWindowText(strBuffer);
		GetDlgItem(IDC_NNS_TIMER_TAKEOVER)->EnableWindow(thePrefs.GetEnableAutoDropNNSDefault());
		CheckDlgButton(IDC_NNS_TIMER_TAKEOVER, thePrefs.GetEnableAutoDropNNSDefault());
		GetDlgItem(IDC_MAXREMOVENNSLIMIT)->EnableWindow(thePrefs.GetEnableAutoDropNNSDefault());
		strBuffer.Format(_T("%d"), thePrefs.GetMaxRemoveNNSLimitDefault());
		GetDlgItem(IDC_MAXREMOVENNSLIMIT)->SetWindowText(strBuffer);
		GetDlgItem(IDC_MAXREMOVENNSLIMIT_TAKEOVER)->EnableWindow(thePrefs.GetEnableAutoDropNNSDefault());
		CheckDlgButton(IDC_MAXREMOVENNSLIMIT_TAKEOVER, thePrefs.GetEnableAutoDropNNSDefault());

		GetDlgItem(IDC_REMOVEFQSLIMITLABEL)->EnableWindow(true);
		CheckDlgButton(IDC_REMOVEFQSLIMITLABEL, thePrefs.GetEnableAutoDropFQSDefault());
		CheckDlgButton(IDC_REMOVEFQSLIMITLABEL_TAKEOVER, true);
		GetDlgItem(IDC_FQS_TIMERLABEL)->EnableWindow(thePrefs.GetEnableAutoDropFQSDefault());
		GetDlgItem(IDC_FQS_TIMER)->EnableWindow(thePrefs.GetEnableAutoDropFQSDefault());
		strBuffer.Format(_T("%d"), thePrefs.GetAutoFQS_TimerDefault()/1000);
		GetDlgItem(IDC_FQS_TIMER)->SetWindowText(strBuffer);
		GetDlgItem(IDC_FQS_TIMER_TAKEOVER)->EnableWindow(thePrefs.GetEnableAutoDropFQSDefault());
		CheckDlgButton(IDC_FQS_TIMER_TAKEOVER, thePrefs.GetEnableAutoDropFQSDefault());
		GetDlgItem(IDC_MAXREMOVEFQSLIMIT)->EnableWindow(thePrefs.GetEnableAutoDropFQSDefault());
		strBuffer.Format(_T("%d"), thePrefs.GetMaxRemoveFQSLimitDefault());
		GetDlgItem(IDC_MAXREMOVEFQSLIMIT)->SetWindowText(strBuffer);
		GetDlgItem(IDC_MAXREMOVEFQSLIMIT_TAKEOVER)->EnableWindow(thePrefs.GetEnableAutoDropFQSDefault());
		CheckDlgButton(IDC_MAXREMOVEFQSLIMIT_TAKEOVER, thePrefs.GetEnableAutoDropFQSDefault());

		GetDlgItem(IDC_REMOVEQRSLIMITLABEL)->EnableWindow(true);
		CheckDlgButton(IDC_REMOVEQRSLIMITLABEL, thePrefs.GetEnableAutoDropQRSDefault());
		CheckDlgButton(IDC_REMOVEQRSLIMITLABEL_TAKEOVER, true);
		GetDlgItem(IDC_HQRS_TIMERLABEL)->EnableWindow(thePrefs.GetEnableAutoDropQRSDefault());
		GetDlgItem(IDC_HQRS_TIMER)->EnableWindow(thePrefs.GetEnableAutoDropQRSDefault());
		strBuffer.Format(_T("%d"), thePrefs.GetAutoHQRS_TimerDefault()/1000);
		GetDlgItem(IDC_HQRS_TIMER)->SetWindowText(strBuffer);
		GetDlgItem(IDC_HQRS_TIMER_TAKEOVER)->EnableWindow(thePrefs.GetEnableAutoDropQRSDefault());
		CheckDlgButton(IDC_HQRS_TIMER_TAKEOVER, thePrefs.GetEnableAutoDropQRSDefault());
		GetDlgItem(IDC_REMOVEQRSLABEL)->EnableWindow(thePrefs.GetEnableAutoDropQRSDefault());
		GetDlgItem(IDC_REMOVEQRS)->EnableWindow(thePrefs.GetEnableAutoDropQRSDefault());
		strBuffer.Format(_T("%d"), thePrefs.GetMaxRemoveQRSDefault());
		GetDlgItem(IDC_REMOVEQRS)->SetWindowText(strBuffer);
		GetDlgItem(IDC_REMOVEQRS_TAKEOVER)->EnableWindow(thePrefs.GetEnableAutoDropQRSDefault());
		CheckDlgButton(IDC_REMOVEQRS_TAKEOVER, thePrefs.GetEnableAutoDropQRSDefault());
		GetDlgItem(IDC_MAXREMOVEQRSLIMIT)->EnableWindow(thePrefs.GetEnableAutoDropQRSDefault());
		strBuffer.Format(_T("%d"), thePrefs.GetMaxRemoveQRSLimitDefault());
		GetDlgItem(IDC_MAXREMOVEQRSLIMIT)->SetWindowText(strBuffer);
		GetDlgItem(IDC_MAXREMOVEQRSLIMIT_TAKEOVER)->EnableWindow(thePrefs.GetEnableAutoDropQRSDefault() && thePrefs.GetHQRXmanDefault());
		CheckDlgButton(IDC_MAXREMOVEQRSLIMIT_TAKEOVER, thePrefs.GetEnableAutoDropQRSDefault());
		CheckDlgButton(IDC_HQR_XMAN, thePrefs.GetHQRXmanDefault());
		CheckDlgButton(IDC_HQR_XMAN_TAKEOVER, thePrefs.GetEnableAutoDropQRSDefault());
	}
}

void CSivkaFileSettings::OnBnClickedMaxSourcesPerFileTakeOver()
{
	GetDlgItem(IDC_HARDLIMIT)->EnableWindow(IsDlgButtonChecked(IDC_HARDLIMIT_TAKEOVER));
	GetDlgItem(IDC_HARDLIMIT_LABEL)->EnableWindow(IsDlgButtonChecked(IDC_HARDLIMIT_TAKEOVER));
}
// ==> Global Source Limit (customize for files) - Stulle
void CSivkaFileSettings::OnBnClickedGlobalHlTakeOver()
{
	GetDlgItem(IDC_GLOBAL_HL)->EnableWindow(IsDlgButtonChecked(IDC_GLOBAL_HL_TAKEOVER));
}
// <== Global Source Limit (customize for files) - Stulle
//////////////////////////////////////////////////////////////////////////////////////////////////
void CSivkaFileSettings::OnBnClickedEnableAutoDropNNSTakeOver()
{
	GetDlgItem(IDC_REMOVENNSLIMITLABEL)->EnableWindow(IsDlgButtonChecked(IDC_REMOVENNSLIMITLABEL_TAKEOVER));
	OnBnClickedEnableAutoNNS();

}
void CSivkaFileSettings::OnBnClickedEnableAutoNNS()
{
	GetDlgItem(IDC_NNS_TIMER_TAKEOVER)->EnableWindow(IsDlgButtonChecked(IDC_REMOVENNSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVENNSLIMITLABEL_TAKEOVER));
	CheckDlgButton(IDC_NNS_TIMER_TAKEOVER, IsDlgButtonChecked(IDC_REMOVENNSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVENNSLIMITLABEL_TAKEOVER) && app_prefs->m_AutoNNS_TimerTakeOver);
	OnBnClickedAutoNNS_TimerTakeOver();
	
	GetDlgItem(IDC_MAXREMOVENNSLIMIT_TAKEOVER)->EnableWindow(IsDlgButtonChecked(IDC_REMOVENNSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVENNSLIMITLABEL_TAKEOVER));
	CheckDlgButton(IDC_MAXREMOVENNSLIMIT_TAKEOVER, IsDlgButtonChecked(IDC_REMOVENNSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVENNSLIMITLABEL_TAKEOVER) && app_prefs->m_MaxRemoveNNSLimitTakeOver);
	OnBnClickedMaxRemoveNNSLimitTakeOver();
}
void CSivkaFileSettings::OnBnClickedAutoNNS_TimerTakeOver()
{
	GetDlgItem(IDC_NNS_TIMERLABEL)->EnableWindow(IsDlgButtonChecked(IDC_REMOVENNSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVENNSLIMITLABEL_TAKEOVER) && IsDlgButtonChecked(IDC_NNS_TIMER_TAKEOVER));
	GetDlgItem(IDC_NNS_TIMER)->EnableWindow(IsDlgButtonChecked(IDC_REMOVENNSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVENNSLIMITLABEL_TAKEOVER) && IsDlgButtonChecked(IDC_NNS_TIMER_TAKEOVER));
}
void CSivkaFileSettings::OnBnClickedMaxRemoveNNSLimitTakeOver()
{
	GetDlgItem(IDC_MAXREMOVENNSLIMIT)->EnableWindow(IsDlgButtonChecked(IDC_REMOVENNSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVENNSLIMITLABEL_TAKEOVER) && IsDlgButtonChecked(IDC_MAXREMOVENNSLIMIT_TAKEOVER));
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CSivkaFileSettings::OnBnClickedEnableAutoDropFQSTakeOver()
{
	GetDlgItem(IDC_REMOVEFQSLIMITLABEL)->EnableWindow(IsDlgButtonChecked(IDC_REMOVEFQSLIMITLABEL_TAKEOVER));
	OnBnClickedEnableAutoFQS();

}
void CSivkaFileSettings::OnBnClickedEnableAutoFQS()
{
	GetDlgItem(IDC_FQS_TIMER_TAKEOVER)->EnableWindow(IsDlgButtonChecked(IDC_REMOVEFQSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVEFQSLIMITLABEL_TAKEOVER));
	CheckDlgButton(IDC_FQS_TIMER_TAKEOVER, IsDlgButtonChecked(IDC_REMOVEFQSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVEFQSLIMITLABEL_TAKEOVER) && app_prefs->m_AutoFQS_TimerTakeOver);
	OnBnClickedAutoFQS_TimerTakeOver();

	GetDlgItem(IDC_MAXREMOVEFQSLIMIT_TAKEOVER)->EnableWindow(IsDlgButtonChecked(IDC_REMOVEFQSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVEFQSLIMITLABEL_TAKEOVER));
	CheckDlgButton(IDC_MAXREMOVEFQSLIMIT_TAKEOVER, IsDlgButtonChecked(IDC_REMOVEFQSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVEFQSLIMITLABEL_TAKEOVER) && app_prefs->m_MaxRemoveFQSLimitTakeOver);
	OnBnClickedMaxRemoveFQSLimitTakeOver();
}
void CSivkaFileSettings::OnBnClickedAutoFQS_TimerTakeOver()
{
	GetDlgItem(IDC_FQS_TIMERLABEL)->EnableWindow(IsDlgButtonChecked(IDC_REMOVEFQSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVEFQSLIMITLABEL_TAKEOVER) && IsDlgButtonChecked(IDC_FQS_TIMER_TAKEOVER));
	GetDlgItem(IDC_FQS_TIMER)->EnableWindow(IsDlgButtonChecked(IDC_REMOVEFQSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVEFQSLIMITLABEL_TAKEOVER) && IsDlgButtonChecked(IDC_FQS_TIMER_TAKEOVER));
}
void CSivkaFileSettings::OnBnClickedMaxRemoveFQSLimitTakeOver()
{
	GetDlgItem(IDC_MAXREMOVEFQSLIMIT)->EnableWindow(IsDlgButtonChecked(IDC_REMOVEFQSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVEFQSLIMITLABEL_TAKEOVER) && IsDlgButtonChecked(IDC_MAXREMOVEFQSLIMIT_TAKEOVER));
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CSivkaFileSettings::OnBnClickedEnableAutoDropQRSTakeOver()
{
	GetDlgItem(IDC_REMOVEQRSLIMITLABEL)->EnableWindow(IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL_TAKEOVER));
	OnBnClickedEnableAutoQRS();

}
void CSivkaFileSettings::OnBnClickedEnableAutoQRS()
{
	GetDlgItem(IDC_HQRS_TIMER_TAKEOVER)->EnableWindow(IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL_TAKEOVER));
	CheckDlgButton(IDC_HQRS_TIMER_TAKEOVER, IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL_TAKEOVER) && app_prefs->m_AutoHQRS_TimerTakeOver);
	OnBnClickedAutoHQRS_TimerTakeOver();

	GetDlgItem(IDC_MAXREMOVEQRSLIMIT_TAKEOVER)->EnableWindow(IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL_TAKEOVER));
	CheckDlgButton(IDC_MAXREMOVEQRSLIMIT_TAKEOVER, IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL_TAKEOVER) && app_prefs->m_MaxRemoveQRSLimitTakeOver);
	OnBnClickedMaxRemoveQRSLimitTakeOver();
	
	GetDlgItem(IDC_HQR_XMAN_TAKEOVER)->EnableWindow(IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL_TAKEOVER));
	CheckDlgButton(IDC_HQR_XMAN_TAKEOVER, IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL_TAKEOVER) && app_prefs->m_bHQRXmanTakeOver);
	OnBnClickedHQRXmanTakeOver();

	GetDlgItem(IDC_REMOVEQRS_TAKEOVER)->EnableWindow(IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL_TAKEOVER) && IsDlgButtonChecked(IDC_HQR_XMAN) == false);
	CheckDlgButton(IDC_REMOVEQRS_TAKEOVER, IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL_TAKEOVER) && app_prefs->m_MaxRemoveQRSTakeOver && IsDlgButtonChecked(IDC_HQR_XMAN) == false);
	OnBnClickedMaxRemoveQRSTakeOver();
}
void CSivkaFileSettings::OnBnClickedAutoHQRS_TimerTakeOver()
{
	GetDlgItem(IDC_HQRS_TIMERLABEL)->EnableWindow(IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL_TAKEOVER) && IsDlgButtonChecked(IDC_HQRS_TIMER_TAKEOVER));
	GetDlgItem(IDC_HQRS_TIMER)->EnableWindow(IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL_TAKEOVER) && IsDlgButtonChecked(IDC_HQRS_TIMER_TAKEOVER));
}
void CSivkaFileSettings::OnBnClickedMaxRemoveQRSTakeOver()
{
	GetDlgItem(IDC_REMOVEQRSLABEL)->EnableWindow(IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL_TAKEOVER) && IsDlgButtonChecked(IDC_REMOVEQRS_TAKEOVER) && IsDlgButtonChecked(IDC_HQR_XMAN) == false);
	GetDlgItem(IDC_REMOVEQRS)->EnableWindow(IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL_TAKEOVER) && IsDlgButtonChecked(IDC_REMOVEQRS_TAKEOVER) && IsDlgButtonChecked(IDC_HQR_XMAN) == false);
}
void CSivkaFileSettings::OnBnClickedMaxRemoveQRSLimitTakeOver()
{
	GetDlgItem(IDC_MAXREMOVEQRSLIMIT)->EnableWindow(IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL_TAKEOVER) && IsDlgButtonChecked(IDC_MAXREMOVEQRSLIMIT_TAKEOVER));
}
void CSivkaFileSettings::OnBnClickedHQRXmanTakeOver()
{
	GetDlgItem(IDC_HQR_XMAN)->EnableWindow(IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL_TAKEOVER) && IsDlgButtonChecked(IDC_HQR_XMAN_TAKEOVER));
}
void CSivkaFileSettings::OnBnClickedHQRXman()
{
	GetDlgItem(IDC_REMOVEQRS_TAKEOVER)->EnableWindow(IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL_TAKEOVER) && IsDlgButtonChecked(IDC_HQR_XMAN) == false);
	CheckDlgButton(IDC_REMOVEQRS_TAKEOVER, IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL) && IsDlgButtonChecked(IDC_REMOVEQRSLIMITLABEL_TAKEOVER) && app_prefs->m_MaxRemoveQRSTakeOver && IsDlgButtonChecked(IDC_HQR_XMAN) == false);
	OnBnClickedMaxRemoveQRSTakeOver();
}