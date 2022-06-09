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
#include "PPgDirectories.h"
#include "SharedFileList.h"
#include "AddBuddy.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CPPgDirectories, CPropertyPage)
CPPgDirectories::CPPgDirectories()
	: CPropertyPage(CPPgDirectories::IDD)
	, m_bVideoBackup(FALSE)
	, m_bSmallBlocks(FALSE)
{
	m_bSharedDirsModified = false;
}

CPPgDirectories::~CPPgDirectories()
{
}

void CPPgDirectories::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SHARESELECTOR, m_ShareSelector);
	DDX_Control(pDX, IDC_TEMPSELECTOR, m_TempSelector);
	DDX_Text(pDX, IDC_INCFILES, m_strIncomingDir);
	DDX_Text(pDX, IDC_TEMPFILES, m_strMainTempDir);
	DDX_Text(pDX, IDC_VLC, m_strPlayer);
	DDX_Text(pDX, IDC_VIDEO_PARAMS, m_strPlayerArgs);
	DDX_Check(pDX, IDC_VIDEOBACKUP, m_bVideoBackup);
	DDX_Check(pDX, IDC_PREVIEW_SMALL, m_bSmallBlocks);
}

BEGIN_MESSAGE_MAP(CPPgDirectories, CPropertyPage)
	ON_EN_CHANGE(IDC_INCFILES, OnEnChange)
	ON_EN_CHANGE(IDC_TEMPFILES, OnEnChange)
	ON_EN_CHANGE(IDC_VLC, OnEnChange)
	ON_EN_CHANGE(IDC_VIDEO_PARAMS, OnEnChange)
	ON_BN_CLICKED(IDC_SELINCDIR, OnBnClickedSelincdir)
	ON_BN_CLICKED(IDC_SELTEMPDIR, OnBnClickedSeltempdir)
	ON_BN_CLICKED(IDC_SELVLC, OnBnClickedSelvlc)
	ON_BN_CLICKED(IDC_VIDEOBACKUP, OnBnClickedBackupPreview)
	ON_BN_CLICKED(IDC_PREVIEW_SMALL, OnEnChange)
END_MESSAGE_MAP()

BOOL CPPgDirectories::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	CWnd	*pWnd;

	pWnd = GetDlgItem(IDC_INCFILES);
	(reinterpret_cast<CEdit*>(pWnd))->SetLimitText(MAX_PATH);
	AddBuddy(pWnd->m_hWnd, ::GetDlgItem(m_hWnd, IDC_SELINCDIR), BDS_RIGHT);

	pWnd = GetDlgItem(IDC_TEMPFILES);
	(reinterpret_cast<CEdit*>(pWnd))->SetLimitText(MAX_PATH);
	AddBuddy(pWnd->m_hWnd, ::GetDlgItem(m_hWnd, IDC_SELTEMPDIR), BDS_RIGHT);

	pWnd = GetDlgItem(IDC_VLC);
	(reinterpret_cast<CEdit*>(pWnd))->SetLimitText(MAX_PATH);
	AddBuddy(pWnd->m_hWnd, ::GetDlgItem(m_hWnd, IDC_SELVLC), BDS_RIGHT);

	LoadSettings();
	Localize();

	return TRUE;
}

void CPPgDirectories::LoadSettings(void)
{
	m_strIncomingDir = m_pPrefs->GetIncomingDir();
	m_strMainTempDir = m_pPrefs->GetTempDir();
	m_strPlayer = m_pPrefs->GetVideoPlayer();
	m_strPlayerArgs = m_pPrefs->GetVideoPlayerArgs();

	m_ShareSelector.m_lstShared.RemoveAll();
	m_pPrefs->SharedDirListCopy(&m_ShareSelector.m_lstShared);
	m_ShareSelector.Init();

	m_TempSelector.m_lstShared.RemoveAll();
	m_pPrefs->TempDirListCopy(&m_TempSelector.m_lstShared);
	m_TempSelector.Init(false);	// disable CD-ROM drives as temp. location

	m_bVideoBackup = m_pPrefs->BackupPreview();
	m_bSmallBlocks = m_pPrefs->GetPreviewSmallBlocks();

	UpdateData(FALSE);
	OnBnClickedBackupPreview();
	SetModified(FALSE);
}

BOOL CPPgDirectories::SelectDir(const TCHAR* indir, TCHAR* outdir, const CString& titletext)
{
	CoInitialize(0);
	TCHAR buffer[MAX_PATH];
	BROWSEINFO bi = { GetSafeHwnd(), 0, buffer, titletext, BIF_VALIDATE | BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS, BrowseCallbackProc, (LPARAM)indir, 0 };
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	BOOL bDone = SHGetPathFromIDList(pidl, outdir);
	LPMALLOC ppMalloc;
	if(SHGetMalloc(&ppMalloc) == NOERROR)
	{
		ppMalloc->Free(pidl);
		ppMalloc->Release();
	}
	CoUninitialize();
	return bDone;
}

void CPPgDirectories::OnBnClickedSelincdir()
{
	TCHAR buffer[MAX_PATH];

	UpdateData(TRUE);
	if(SelectDir(m_strIncomingDir, buffer, GetResString(IDS_SELECT_INCOMINGDIR)))
	{
		m_strIncomingDir = buffer;

		UpdateData(FALSE);
		SetModified();
	}
}

void CPPgDirectories::OnBnClickedSeltempdir()
{
	TCHAR buffer[MAX_PATH];

	UpdateData(TRUE);
	if(SelectDir(m_strMainTempDir, buffer, GetResString(IDS_SELECT_TEMPDIR)))
	{
		m_strMainTempDir = buffer;

		UpdateData(FALSE);
		SetModified();
	}
}

void CPPgDirectories::OnBnClickedSelvlc()
{
	CFileDialog dlgFile(TRUE, _T("*.exe"), NULL, OFN_FILEMUSTEXIST | OFN_NOREADONLYRETURN, _T("Program (*.exe)|*.exe||"));

	dlgFile.m_pOFN->lpstrInitialDir = GetPathToFile(m_pPrefs->GetVideoPlayer());
	if (dlgFile.DoModal() == IDOK)
	{
		UpdateData(TRUE);

		m_strPlayer = dlgFile.GetPathName();

		UpdateData(FALSE);
		SetModified();
	}
}

BOOL CPPgDirectories::OnApply()
{
	if(m_bModified)
	{
		UpdateData(TRUE);

		if (!m_strIncomingDir.IsEmpty())
			m_pPrefs->SetIncomingDir(m_strIncomingDir);

		if (!m_strMainTempDir.IsEmpty())
			m_pPrefs->SetTempDir(m_strMainTempDir);

		m_pPrefs->SetVideoPlayer(m_strPlayer);
		m_pPrefs->SetVideoPlayerArgs(m_strPlayerArgs);
		m_pPrefs->SetBackupPreview(B2b(m_bVideoBackup));
		m_pPrefs->SetPreviewSmallBlocks(B2b(m_bSmallBlocks));

		if (m_bSharedDirsModified)
		{
			POSITION pos, pos1;

			for (pos = m_ShareSelector.m_lstShared.GetHeadPosition(); (pos1 = pos) != NULL;)
			{
				if (!m_pPrefs->IsShareableDirectory(m_ShareSelector.m_lstShared.GetNext(pos)))
					m_ShareSelector.m_lstShared.RemoveAt(pos1);
			}
			m_pPrefs->SharedDirListRefill(&m_ShareSelector.m_lstShared);
			g_App.m_pSharedFilesList->Reload();
		//	Required to update file colors as file attributes can change
			g_App.m_pMDlg->m_dlgSearch.m_ctlSearchList.Invalidate();
		}

	//	Remove main temporary directory from the list if user added it
		POSITION pos, pos1;
		CString	strMainTemp = m_pPrefs->GetTempDir();

		strMainTemp += _T('\\');

		for (pos = m_TempSelector.m_lstShared.GetHeadPosition(); (pos1 = pos) != NULL;)
		{
			if (m_TempSelector.m_lstShared.GetNext(pos).CompareNoCase(strMainTemp) == 0)
			{
				m_TempSelector.m_lstShared.RemoveAt(pos1);
				break;
			}
		}
		m_pPrefs->TempDirListRefill(&m_TempSelector.m_lstShared);

		SetModified(FALSE);
		m_bSharedDirsModified = false;
	}
	return CPropertyPage::OnApply();
}

BOOL CPPgDirectories::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if(wParam == DIRLIST_ITEMSTATECHANGED)
	{
		if(lParam == (long)m_ShareSelector.m_hWnd)
			CheckSharedChanges();
		else if(lParam == (long)m_TempSelector.m_hWnd)
			CheckTempChanges();
	}
	return CPropertyPage::OnCommand(wParam, lParam);
}

void CPPgDirectories::Localize(void)
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_INCOMING_LBL, IDS_PW_INCOMING },
		{ IDC_TEMP_LBL, IDS_PW_TEMP },
		{ IDC_VLC_LBL, IDS_PW_VLC },
		{ IDC_SHARED_LBL, IDS_PW_SHARED },
		{ IDC_TEMPDIR_LBL, IDS_PW_TEMPDIR },
		{ IDC_VIDEOBACKUP, IDS_VIDEOBACKUP },
		{ IDC_PREVIEW_SMALL, IDS_PREVIEW_SMALL },
		{ IDC_VIDEO_PARAM_LBL, IDS_AV_PARAMS }
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

void CPPgDirectories::CheckSharedChanges(void)
{
	if (m_pPrefs->SharedDirListCmp(&m_ShareSelector.m_lstShared))
	{
		SetModified();
		m_bSharedDirsModified = true;
	}
}

void CPPgDirectories::CheckTempChanges(void)
{
	if (m_pPrefs->TempDirListCmp(&m_TempSelector.m_lstShared))
		SetModified();
}

void CPPgDirectories::OnBnClickedBackupPreview()
{
	UpdateData(TRUE);
	GetDlgItem(IDC_PREVIEW_SMALL)->EnableWindow(!m_bVideoBackup);
	SetModified();
}
