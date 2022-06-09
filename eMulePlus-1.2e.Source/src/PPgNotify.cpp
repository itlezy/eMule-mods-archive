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
#include "PPgNotify.h"
#include "AddBuddy.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CPPgNotify, CPropertyPage)
CPPgNotify::CPPgNotify()
	: CPropertyPage(CPPgNotify::IDD)
	, m_bUseSound(FALSE)
	, m_bOnLog(FALSE)
	, m_bOnChat(FALSE)
	, m_bOnChatMessage(FALSE)
	, m_bOnDownloadAdded(FALSE)
	, m_bOnDownloadFinished(FALSE)
	, m_bUseScheduler(FALSE)
	, m_bOnWebServer(FALSE)
	, m_bOnImportant(FALSE)
	, m_bOnServerError(FALSE)
{
}

CPPgNotify::~CPPgNotify()
{
}

void CPPgNotify::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CB_TBN_USESOUND, m_bUseSound);
	DDX_Check(pDX, IDC_CB_TBN_ONLOG, m_bOnLog);
	DDX_Check(pDX, IDC_CB_TBN_ONCHAT, m_bOnChat);
	DDX_Check(pDX, IDC_CB_TBN_POP_ALWAYS, m_bOnChatMessage);
	DDX_Check(pDX, IDC_CB_TBN_ONDOWNLOADADD, m_bOnDownloadAdded);
	DDX_Check(pDX, IDC_CB_TBN_ONDOWNLOAD, m_bOnDownloadFinished);
	DDX_Check(pDX, IDC_CB_TBN_USESCHEDULER, m_bUseScheduler);
	DDX_Check(pDX, IDC_CB_TBN_WEBSERVER, m_bOnWebServer);
	DDX_Check(pDX, IDC_CB_TBN_IMPORTANT, m_bOnImportant);
	DDX_Check(pDX, IDC_CB_TBN_SERVER, m_bOnServerError);
	DDX_Text(pDX, IDC_EDIT_TBN_WAVFILE, m_strWavFileName);	
	DDX_Control(pDX, IDC_COMBO_TBN_DTIME, m_DtimeCombo);
	DDX_Control(pDX, IDC_COMBO_TBN_FSIZE, m_FsizeCombo);
	DDX_Control(pDX, IDC_EDIT_TBN_WAVFILE, m_WavFileNameEdit);
	DDX_Control(pDX, IDC_BTN_BROWSE_WAV, m_WavFileNameButton);
}

BEGIN_MESSAGE_MAP(CPPgNotify, CPropertyPage)
	ON_BN_CLICKED(IDC_CB_TBN_USESOUND, OnBnClickedCbTbnUsesound)
	ON_BN_CLICKED(IDC_CB_TBN_ONLOG, OnSettingsChange)
	ON_BN_CLICKED(IDC_CB_TBN_ONCHAT, OnBnClickedCbTbnOnchat)
	ON_BN_CLICKED(IDC_CB_TBN_POP_ALWAYS, OnSettingsChange)
	ON_BN_CLICKED(IDC_CB_TBN_ONDOWNLOADADD, OnSettingsChange)
	ON_BN_CLICKED(IDC_CB_TBN_ONDOWNLOAD, OnSettingsChange)
	ON_BN_CLICKED(IDC_CB_TBN_IMPORTANT, OnSettingsChange)
	ON_BN_CLICKED(IDC_CB_TBN_WEBSERVER, OnSettingsChange)
	ON_BN_CLICKED(IDC_CB_TBN_SERVER, OnSettingsChange)
	ON_BN_CLICKED(IDC_CB_TBN_USESCHEDULER, OnSettingsChange)
	ON_BN_CLICKED(IDC_BTN_BROWSE_WAV, OnBnClickedBtnBrowseWav)
	ON_CBN_SELCHANGE(IDC_COMBO_TBN_DTIME, OnSettingsChange)
	ON_CBN_SELCHANGE(IDC_COMBO_TBN_FSIZE, OnSettingsChange)
	ON_EN_CHANGE(IDC_EDIT_TBN_WAVFILE, OnSettingsChange)
END_MESSAGE_MAP()

BOOL CPPgNotify::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_DtimeCombo.SetItemData(m_DtimeCombo.AddString(_T("1")), 1000);
	m_DtimeCombo.SetItemData(m_DtimeCombo.AddString(_T("2")), 2000);
	m_DtimeCombo.SetItemData(m_DtimeCombo.AddString(_T("3")), 3000);
	m_DtimeCombo.SetItemData(m_DtimeCombo.AddString(_T("4")), 4000);
	m_DtimeCombo.SetItemData(m_DtimeCombo.AddString(_T("5")), 5000);
	m_DtimeCombo.SetItemData(m_DtimeCombo.AddString(_T("6")), 6000);
	m_DtimeCombo.SetItemData(m_DtimeCombo.AddString(_T("7")), 7000);
	m_DtimeCombo.SetItemData(m_DtimeCombo.AddString(_T("8")), 8000);
	m_DtimeCombo.SetItemData(m_DtimeCombo.AddString(_T("9")), 9000);

	m_FsizeCombo.SetItemData(m_FsizeCombo.AddString(_T("7")), 70);
	m_FsizeCombo.SetItemData(m_FsizeCombo.AddString(_T("8")), 80);
	m_FsizeCombo.SetItemData(m_FsizeCombo.AddString(_T("9")), 90);
	m_FsizeCombo.SetItemData(m_FsizeCombo.AddString(_T("10")), 100);

	AddBuddy(m_WavFileNameEdit.m_hWnd, m_WavFileNameButton.m_hWnd, BDS_RIGHT);

	LoadSettings();
	Localize();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPPgNotify::LoadSettings(void)
{
	if(::IsWindow(m_hWnd))
	{
		m_bOnDownloadAdded = m_pPrefs->GetUseDownloadAddNotifier();
		m_bOnDownloadFinished = m_pPrefs->GetUseDownloadNotifier();
		m_bOnChat = m_pPrefs->GetUseChatNotifier();
		m_bUseSound = m_pPrefs->GetUseSoundInNotifier();
		m_bOnLog = m_pPrefs->GetUseLogNotifier();
		m_bUseScheduler = m_pPrefs->GetUseSchedulerNotifier();
		m_bOnChatMessage = m_pPrefs->GetNotifierPopsEveryChatMsg();
		m_bOnImportant = m_pPrefs->GetNotifierPopOnImportantError();
		m_bOnWebServer = m_pPrefs->GetNotifierPopOnWebServerError();
		m_bOnServerError = m_pPrefs->GetNotifierPopOnServerError();
		m_strWavFileName = m_pPrefs->GetNotifierWavSoundPath();

		GetDlgItem(IDC_CB_TBN_POP_ALWAYS)->EnableWindow(m_bOnChat);

		for(int i = 0; i != m_DtimeCombo.GetCount(); i++)
			if (m_DtimeCombo.GetItemData(i) == m_pPrefs->NotificationDisplayTime())
				m_DtimeCombo.SetCurSel(i);

		for(int i = 0; i != m_FsizeCombo.GetCount(); i++)
			if (m_FsizeCombo.GetItemData(i) == m_pPrefs->NotificationFontSize())
				m_FsizeCombo.SetCurSel(i);

		UpdateData(FALSE);

		OnBnClickedCbTbnUsesound();
	}

	SetModified(FALSE);
}

BOOL CPPgNotify::OnApply()
{
	if(m_bModified)
	{
		int	iVal;

		UpdateData(TRUE);

		m_pPrefs->SetUseDownloadAddNotifier(B2b(m_bOnDownloadAdded));
		m_pPrefs->SetUseDownloadNotifier(B2b(m_bOnDownloadFinished));
		m_pPrefs->SetUseChatNotifier(B2b(m_bOnChat));
		m_pPrefs->SetUseLogNotifier(B2b(m_bOnLog));
		m_pPrefs->SetUseSoundInNotifier(B2b(m_bUseSound));
		m_pPrefs->SetUseSchedulerNotifier(B2b(m_bUseScheduler));
		m_pPrefs->SetNotifierPopsEveryChatMsg(B2b(m_bOnChatMessage));
		m_pPrefs->SetNotifierPopOnImportantError(B2b(m_bOnImportant));
		m_pPrefs->SetNotifierPopOnWebServerError(B2b(m_bOnWebServer));
		m_pPrefs->SetNotifierPopOnServerError(B2b(m_bOnServerError));
		m_pPrefs->SetNotifierWavSoundPath(m_strWavFileName);

		if ((iVal = m_DtimeCombo.GetCurSel()) != CB_ERR)
			m_pPrefs->SetNotificationDisplayTime(m_DtimeCombo.GetItemData(iVal));

		if ((iVal = m_FsizeCombo.GetCurSel()) != CB_ERR)
			m_pPrefs->SetNotificationFontSize(m_FsizeCombo.GetItemData(iVal));

		((CEmuleDlg*)AfxGetMainWnd())->m_wndTaskbarNotifier.SetTextFont( _T("Arial"),
			g_App.m_pPrefs->NotificationFontSize(), TN_TEXT_NORMAL,
			TN_TEXT_UNDERLINE ); // Popup properties

		SetModified(FALSE);
	}

	return CPropertyPage::OnApply();
}

void CPPgNotify::Localize(void)
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_CB_TBN_USESOUND, IDS_PW_TBN_USESOUND },
		{ IDC_CB_TBN_ONLOG, IDS_PW_TBN_ONLOG },
		{ IDC_CB_TBN_ONCHAT, IDS_PW_TBN_ONCHAT },
		{ IDC_CB_TBN_POP_ALWAYS, IDS_PW_TBN_POP_ALWAYS },
		{ IDC_CB_TBN_ONDOWNLOADADD, IDS_PW_TBN_ONDOWNLOADADD },
		{ IDC_CB_TBN_ONDOWNLOAD, IDS_PW_TBN_ONDOWNLOAD },
		{ IDC_CB_TBN_IMPORTANT, IDS_PS_TBN_IMPORTANT },
		{ IDC_CB_TBN_WEBSERVER, IDS_PS_TBN_WEBSERVER },
		{ IDC_CB_TBN_SERVER, IDS_PS_TBN_SERVER },
		{ IDC_CB_TBN_USESCHEDULER, IDS_PW_TBN_USESCHEDULER },
		{ IDC_TBN_WARNING, IDS_PW_TBN_WARNING },
		{ IDC_TBN_OPTIONS, IDS_PW_TBN_OPTIONS },
		{ IDC_TBN_PROPERTIES, IDS_TBN_PROPERTIES },
		{ IDC_CB_TBN_DTIME, IDS_CB_TBN_DTIME },
		{ IDC_CB_TBN_FSIZE, IDS_CB_TBN_FSIZE },
		{ IDC_CB_TBN_SEC, IDS_SECS }
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

void CPPgNotify::OnBnClickedCbTbnOnchat()
{
	UpdateData(TRUE);
	GetDlgItem(IDC_CB_TBN_POP_ALWAYS)->EnableWindow(m_bOnChat);

	SetModified();
}

void CPPgNotify::OnBnClickedBtnBrowseWav()
{
	UpdateData(TRUE);
	if (DialogBrowseFile(m_strWavFileName, _T("Audio-Wav (*.wav)|*.wav||"), NULL, OFN_FILEMUSTEXIST))
	{
		UpdateData(FALSE);
		SetModified();
	}
}

void CPPgNotify::OnBnClickedCbTbnUsesound()
{
	UpdateData(TRUE);
	m_WavFileNameEdit.EnableWindow(m_bUseSound);
	m_WavFileNameButton.EnableWindow(m_bUseSound);

	SetModified();
}
