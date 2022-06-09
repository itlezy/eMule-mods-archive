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
//
// Partialy based on emule 0.26d_Fusion_Flux_[R6]

#include "stdafx.h"
#include "emule.h"
#include "PPgBackup.h"
#include "AddBuddy.h"

IMPLEMENT_DYNAMIC(CPPgBackup, CPropertyPage)
CPPgBackup::CPPgBackup()
	: CPropertyPage(CPPgBackup::IDD)
	, m_bDatFiles(FALSE)
	, m_bMetFiles(FALSE)
	, m_bIniFiles(FALSE)
	, m_bPartFiles(FALSE)
	, m_bPartMetFiles(FALSE)
	, m_bTxtsrcFiles(FALSE)
	, m_bAutoBackup(FALSE)
	, m_bOverwriteFiles(FALSE)
	, m_bScheduledBackup(FALSE)
{
}

CPPgBackup::~CPPgBackup()
{
}

void CPPgBackup::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_DAT, m_bDatFiles);
	DDX_Check(pDX, IDC_MET, m_bMetFiles);
	DDX_Check(pDX, IDC_INI, m_bIniFiles);
	DDX_Check(pDX, IDC_PART, m_bPartFiles);
	DDX_Check(pDX, IDC_PARTMET, m_bPartMetFiles);
	DDX_Check(pDX, IDC_PARTTXTSRC, m_bTxtsrcFiles);
	DDX_Check(pDX, IDC_BACKUP_AUTO, m_bAutoBackup);
	DDX_Check(pDX, IDC_BACKUP_OVERWRITE, m_bOverwriteFiles);
	DDX_Check(pDX, IDC_SCHEDULED_BACKUP, m_bScheduledBackup);
	DDX_Text(pDX, IDC_SCHEDULED_BACKUP_INTERVAL, m_strScheduledBackupInterval);
	DDX_Text(pDX, IDC_BACKUP_DIR, m_strBackupDir);
}

BOOL CPPgBackup::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	LoadSettings();
	Localize();
	CheckBackupNowButton();

	AddBuddy(::GetDlgItem(m_hWnd, IDC_BACKUP_DIR), ::GetDlgItem(m_hWnd, IDC_BACKUP_BROWSE), BDS_RIGHT);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CPPgBackup, CPropertyPage)
	ON_BN_CLICKED(IDC_BACKUP_NOW, OnBnClickedBackupnow)
	ON_BN_CLICKED(IDC_DAT, OnBnClickedCommon)
	ON_BN_CLICKED(IDC_MET, OnBnClickedCommon)
	ON_BN_CLICKED(IDC_INI, OnBnClickedCommon)
	ON_BN_CLICKED(IDC_PART, OnBnClickedPart)
	ON_BN_CLICKED(IDC_PARTMET, OnBnClickedCommon)
	ON_BN_CLICKED(IDC_PARTTXTSRC, OnBnClickedCommon)
	ON_BN_CLICKED(IDC_BACKUP_SELECTALL, OnBnClickedSelectall)
	ON_BN_CLICKED(IDC_BACKUP_OVERWRITE, OnSettingsChange)
	ON_BN_CLICKED(IDC_BACKUP_BROWSE, OnBnClickedBrowse)
	ON_EN_CHANGE(IDC_BACKUP_DIR, OnSettingsChange)
	ON_BN_CLICKED(IDC_BACKUP_AUTO, OnSettingsChange)
	ON_BN_CLICKED(IDC_SCHEDULED_BACKUP, OnBnClickedScheduledBackupCheck)
	ON_EN_CHANGE(IDC_SCHEDULED_BACKUP_INTERVAL, OnSettingsChange)
END_MESSAGE_MAP()


void CPPgBackup::OnBnClickedCommon()
{
	SetModified();
	CheckBackupNowButton();
}

void CPPgBackup::OnBnClickedPart()
{
	UpdateData(TRUE);

	if (m_bPartFiles)
	{
		if (MessageBox(GetResString(IDS_BACKUP_LONGTIME), GetResString(IDS_BACKUP_SURE), MB_ICONQUESTION|MB_YESNO) == IDYES)
		{
			m_bPartFiles = true;
			CheckBackupNowButton();
		}
		else
			m_bPartFiles = false;
	}

	UpdateData(FALSE);
	SetModified();
}

void CPPgBackup::OnBnClickedBrowse()
{
	TCHAR buffer[MAX_PATH];

	UpdateData(TRUE);
	if (SelectDir(m_strBackupDir, buffer, GetResString(IDS_SELECT_BACKUPDIR)))
	{
		m_strBackupDir = buffer;

		UpdateData(FALSE);
		SetModified();
	}
}

void CPPgBackup::OnBnClickedBackupnow()
{
	OnApply();
	g_App.m_pMDlg->RunBackupNow(false);
}

void CPPgBackup::OnBnClickedSelectall()
{
	UpdateData(TRUE);
	m_bDatFiles = true;
	m_bMetFiles = true;
	m_bIniFiles = true;
	m_bPartFiles = true;
	m_bPartMetFiles = true;
	m_bTxtsrcFiles = true;
	UpdateData(FALSE);
	SetModified();

	CheckBackupNowButton();
}

void CPPgBackup::CheckBackupNowButton()
{
	UpdateData(TRUE);

	bool	bSmthToBackup = m_bDatFiles || m_bMetFiles || m_bIniFiles || m_bPartFiles || m_bPartMetFiles || m_bTxtsrcFiles;

	GetDlgItem(IDC_BACKUP_NOW)->EnableWindow(bSmthToBackup);
	GetDlgItem(IDC_BACKUP_OVERWRITE)->EnableWindow(bSmthToBackup);
	GetDlgItem(IDC_BACKUP_AUTO)->EnableWindow(bSmthToBackup);
	GetDlgItem(IDC_SCHEDULED_BACKUP)->EnableWindow(bSmthToBackup);
	OnBnClickedScheduledBackupCheck();
}

BOOL CPPgBackup::SelectDir(const TCHAR *pcInDir, TCHAR *outdir, const CString &titletext)
{
	CoInitialize(0);
	BOOL	bDone;
	TCHAR buffer[MAX_PATH];
	BROWSEINFO bi = { GetSafeHwnd(), 0, buffer, titletext, BIF_VALIDATE | BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS, BrowseCallbackProc, (LPARAM)pcInDir, 0 };
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	bDone = SHGetPathFromIDList(pidl, outdir);
	LPMALLOC ppMalloc;
	if(SHGetMalloc(&ppMalloc) == NOERROR)
	{
		ppMalloc->Free(pidl);
		ppMalloc->Release();
	}
	CoUninitialize();
	return bDone;
}

void CPPgBackup::Localize(void)
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_BACKUP_GRPFILETYPE, IDS_BACKUP_GRPFILETYPE },
		{ IDC_BACKUP_SELECTALL, IDS_BACKUP_SELECTALL },
		{ IDC_BACKUP_NOW, IDS_BACKUP_NOW },
		{ IDC_BACKUP_OPTION, IDS_BACKUP_OPTION },
		{ IDC_BACKUP_TITLETXT, IDS_BACKUP_TITLETXT },
		{ IDC_BACKUP_OVERWRITE, IDS_BACKUP_OVERWRITE },
		{ IDC_BACKUP_AUTO, IDS_BACKUP_AUTO },
		{ IDC_SCHEDULED_BACKUP, IDS_SCHEDULED_BACKUP },
		{ IDC_HOURS, IDS_HOURS }
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

void CPPgBackup::LoadSettings(void)
{
	m_bDatFiles = m_pPrefs->GetBackupDatFiles();
	m_bMetFiles = m_pPrefs->GetBackupMetFiles();
	m_bIniFiles = m_pPrefs->GetBackupIniFiles();
	m_bPartFiles = m_pPrefs->GetBackupPartFiles();
	m_bPartMetFiles = m_pPrefs->GetBackupPartMetFiles();
	m_bTxtsrcFiles = m_pPrefs->GetBackupPartTxtsrcFiles();
	m_bAutoBackup = m_pPrefs->IsAutoBackup();
	m_bOverwriteFiles = m_pPrefs->GetBackupOverwrite();
	m_strBackupDir = m_pPrefs->GetBackupDir();
	m_bScheduledBackup = m_pPrefs->IsScheduledBackup();
	m_strScheduledBackupInterval.Format(_T("%u"), m_pPrefs->GetScheduledBackupInterval());

	UpdateData(FALSE);
	SetModified(FALSE);
}

BOOL CPPgBackup::OnApply()
{
	if (m_bModified)
	{
		UpdateData(TRUE);

		m_pPrefs->SetBackupDatFiles(B2b(m_bDatFiles));
		m_pPrefs->SetBackupMetFiles(B2b(m_bMetFiles));
		m_pPrefs->SetBackupIniFiles(B2b(m_bIniFiles));
		m_pPrefs->SetBackupPartFiles(B2b(m_bPartFiles));
		m_pPrefs->SetBackupPartMetFiles(B2b(m_bPartMetFiles));
		m_pPrefs->SetBackupPartTxtsrcFiles(B2b(m_bTxtsrcFiles));
		m_pPrefs->SetAutoBackup(B2b(m_bAutoBackup));
		m_pPrefs->SetBackupOverwrite(B2b(m_bOverwriteFiles));
		m_pPrefs->SetScheduledBackup(B2b(m_bScheduledBackup));
		m_pPrefs->SetScheduledBackupInterval(static_cast<uint16>(_tstoi(m_strScheduledBackupInterval)));

		if (!m_strBackupDir.IsEmpty())
			m_pPrefs->SetBackupDir(m_strBackupDir);
	}
	return CPropertyPage::OnApply();
}

void CPPgBackup::OnBnClickedScheduledBackupCheck()
{
	UpdateData(TRUE);

	bool	bSmthToBackup = m_bDatFiles || m_bMetFiles || m_bIniFiles || m_bPartFiles || m_bPartMetFiles || m_bTxtsrcFiles;

	GetDlgItem(IDC_SCHEDULED_BACKUP_INTERVAL)->EnableWindow(m_bScheduledBackup && bSmthToBackup);

	SetModified();
}
