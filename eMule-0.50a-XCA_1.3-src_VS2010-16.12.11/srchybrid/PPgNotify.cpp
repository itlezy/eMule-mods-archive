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
#include "PPgNotify.h"
#include "Preferences.h"
#include "OtherFunctions.h"
#include "Preferences.h"
#include "TaskbarNotifier.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CPPgNotify, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgNotify, CPropertyPage)
	ON_BN_CLICKED(IDC_CB_TBN_NOSOUND, OnBnClickedChangeNotification) // X: [CI] - [Code Improvement]
	ON_BN_CLICKED(IDC_CB_TBN_USESOUND, OnBnClickedChangeNotification)
	ON_EN_CHANGE(IDC_EDIT_TBN_WAVFILE, OnSettingsChange)
	ON_BN_CLICKED(IDC_BTN_BROWSE_WAV, OnBnClickedBrowseAudioFile)
	ON_BN_CLICKED(IDC_TEST_NOTIFICATION, OnBnClickedTestNotification)
	ON_BN_CLICKED(IDC_CB_TBN_ONNEWDOWNLOAD, OnSettingsChange)
	ON_BN_CLICKED(IDC_CB_TBN_ONDOWNLOAD, OnSettingsChange)
	ON_BN_CLICKED(IDC_CB_TBN_ONLOG, OnSettingsChange)
	ON_BN_CLICKED(IDC_CB_TBN_ONCHAT, OnBnClickedOnChat)
	ON_BN_CLICKED(IDC_CB_TBN_IMPORTATNT , OnSettingsChange)
	ON_BN_CLICKED(IDC_CB_TBN_POP_ALWAYS, OnSettingsChange)
	ON_BN_CLICKED(IDC_CB_ENABLENOTIFICATIONS, OnBnClickedChangeNotification) // X: [CI] - [Code Improvement] Apply if modified
END_MESSAGE_MAP()

CPPgNotify::CPPgNotify()
	: CPropertyPage(CPPgNotify::IDD)
{
}

CPPgNotify::~CPPgNotify()
{
}

/*void CPPgNotify::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}
*/
BOOL CPPgNotify::OnInitDialog()
{
/* Xman removed
#if _ATL_VER >= 0x0710
	m_bEnableEMail = (IsRunningXPSP2OrHigher() > 0);
#endif
*/
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	int iBtnID;
	if (thePrefs.notifierSoundType == ntfstSoundFile)
		iBtnID = IDC_CB_TBN_USESOUND;
	else {
		ASSERT( thePrefs.notifierSoundType == ntfstNoSound );
		iBtnID = IDC_CB_TBN_NOSOUND;
	}
	ASSERT( IDC_CB_TBN_NOSOUND < IDC_CB_TBN_USESOUND);	
	CheckRadioButton(IDC_CB_TBN_NOSOUND, IDC_CB_TBN_USESOUND, iBtnID);

	CheckDlgButton(IDC_CB_TBN_ONDOWNLOAD, thePrefs.notifierOnDownloadFinished ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_CB_TBN_ONNEWDOWNLOAD, thePrefs.notifierOnNewDownload ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_CB_TBN_ONCHAT, thePrefs.notifierOnChat ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_CB_TBN_ONLOG, thePrefs.notifierOnLog ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_CB_TBN_IMPORTATNT, thePrefs.notifierOnImportantError ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_CB_TBN_POP_ALWAYS, thePrefs.notifierOnEveryChatMsg ? BST_CHECKED : BST_UNCHECKED);

	CButton* btnPTR = (CButton*)GetDlgItem(IDC_CB_TBN_POP_ALWAYS);
	btnPTR->EnableWindow(IsDlgButtonChecked(IDC_CB_TBN_ONCHAT));

	SetDlgItemText(IDC_EDIT_TBN_WAVFILE, thePrefs.notifierSoundFile);

	UpdateControls();
	Localize();

	m_bModified = false; // X: [CI] - [Code Improvement] Apply if modified

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CPPgNotify::UpdateControls()
{
	GetDlgItem(IDC_EDIT_TBN_WAVFILE)->EnableWindow(IsDlgButtonChecked(IDC_CB_TBN_USESOUND));
	GetDlgItem(IDC_BTN_BROWSE_WAV)->EnableWindow(IsDlgButtonChecked(IDC_CB_TBN_USESOUND));
}

void CPPgNotify::Localize(void)
{
	if (m_hWnd)
	{
		SetWindowText(GetResString(IDS_PW_EKDEV_OPTIONS));
		SetDlgItemText(IDC_CB_TBN_USESOUND,GetResString(IDS_PW_TBN_USESOUND));
		SetDlgItemText(IDC_CB_TBN_NOSOUND,GetResString(IDS_NOSOUND));
		//SetDlgItemText(IDC_BTN_BROWSE_WAV,GetResString(IDS_PW_BROWSE));
		SetDlgItemText(IDC_CB_TBN_ONLOG,GetResString(IDS_PW_TBN_ONLOG));
		SetDlgItemText(IDC_CB_TBN_ONCHAT,GetResString(IDS_PW_TBN_ONCHAT));
		SetDlgItemText(IDC_CB_TBN_POP_ALWAYS,GetResString(IDS_PW_TBN_POP_ALWAYS));
		SetDlgItemText(IDC_CB_TBN_ONDOWNLOAD,GetResString(IDS_PW_TBN_ONDOWNLOAD) + _T(" (*)") );
		SetDlgItemText(IDC_CB_TBN_ONNEWDOWNLOAD,GetResString(IDS_TBN_ONNEWDOWNLOAD));
		SetDlgItemText(IDC_TASKBARNOTIFIER,GetResString(IDS_PW_TASKBARNOTIFIER));
		SetDlgItemText(IDC_CB_TBN_IMPORTATNT,GetResString(IDS_PS_TBN_IMPORTANT) + _T(" (*)"));
		SetDlgItemText(IDC_TBN_OPTIONS,GetResString(IDS_PW_TBN_OPTIONS));
		SetDlgItemText(IDC_CB_ENABLENOTIFICATIONS,GetResString(IDS_PW_ENABLEEMAIL));
		SetDlgItemText(IDC_TEST_NOTIFICATION, GetResString(IDS_TEST) );
	}
}

BOOL CPPgNotify::OnApply()
{
	if(m_bModified){ // X: [CI] - [Code Improvement] Apply if modified
		thePrefs.notifierOnDownloadFinished = IsDlgButtonChecked(IDC_CB_TBN_ONDOWNLOAD)!=0;
		thePrefs.notifierOnNewDownload = IsDlgButtonChecked(IDC_CB_TBN_ONNEWDOWNLOAD)!=0;
		thePrefs.notifierOnChat = IsDlgButtonChecked(IDC_CB_TBN_ONCHAT)!=0;
		thePrefs.notifierOnLog = IsDlgButtonChecked(IDC_CB_TBN_ONLOG)!=0;
		thePrefs.notifierOnImportantError = IsDlgButtonChecked(IDC_CB_TBN_IMPORTATNT)!=0;
		thePrefs.notifierOnEveryChatMsg = IsDlgButtonChecked(IDC_CB_TBN_POP_ALWAYS)!=0;

		ApplyNotifierSoundType();
		if (thePrefs.notifierSoundType != ntfstSpeech)

		SetModified(FALSE);
		m_bModified = false; // X: [CI] - [Code Improvement] Apply if modified
	}
	return CPropertyPage::OnApply();
}

void CPPgNotify::ApplyNotifierSoundType()
{
	GetDlgItemText(IDC_EDIT_TBN_WAVFILE, thePrefs.notifierSoundFile);
	if (IsDlgButtonChecked(IDC_CB_TBN_USESOUND))
		thePrefs.notifierSoundType = ntfstSoundFile;
	else {
		ASSERT( IsDlgButtonChecked(IDC_CB_TBN_NOSOUND) );
		thePrefs.notifierSoundType = ntfstNoSound;
	}
}

void CPPgNotify::OnBnClickedOnChat()
{
    GetDlgItem(IDC_CB_TBN_POP_ALWAYS)->EnableWindow(IsDlgButtonChecked(IDC_CB_TBN_ONCHAT));
	OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified
}

void CPPgNotify::OnBnClickedBrowseAudioFile()
{
	CString strWavPath;
	GetDlgItemText(IDC_EDIT_TBN_WAVFILE, strWavPath);
	CString buffer;
    if (DialogBrowseFile(buffer, _T("Audio-Files (*.wav)|*.wav||"), strWavPath)){
		SetDlgItemText(IDC_EDIT_TBN_WAVFILE, buffer);
		OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified
	}
}

void CPPgNotify::OnBnClickedChangeNotification() // X: [CI] - [Code Improvement]
{
	UpdateControls();
	OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified
}

void CPPgNotify::OnBnClickedTestNotification()
{
	// save current pref settings
	bool bCurNotifyOnImportantError = thePrefs.notifierOnImportantError;
	ENotifierSoundType iCurSoundType = thePrefs.notifierSoundType;
	CString strSoundFile = thePrefs.notifierSoundFile;

	// temporary apply current settings from dialog
	thePrefs.notifierOnImportantError = true;
	ApplyNotifierSoundType();

	// play test notification
	CString strTest;
	strTest.Format(GetResString(IDS_MAIN_READY), theApp.m_strCurVersionLong);
	theApp.emuledlg->ShowNotifier(strTest, TBN_IMPORTANTEVENT);

	// restore pref settings
	thePrefs.notifierSoundFile = strSoundFile;
	thePrefs.notifierSoundType = iCurSoundType;
	thePrefs.notifierOnImportantError = bCurNotifyOnImportantError;
}