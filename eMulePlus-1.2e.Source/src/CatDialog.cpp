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
#include "CatDialog.h"
#include "Preferences.h"
#include "SharedFileList.h"
#include "otherfunctions.h"
#include "AddBuddy.h"

IMPLEMENT_DYNAMIC(CCatDialog, CDialog)
CCatDialog::CCatDialog(int index)
	: CDialog(CCatDialog::IDD, 0)
{
	m_pCat = CCat::GetCatByIndex(index);
	if (m_pCat == NULL)
		return;
}

BOOL CCatDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	Localize();
	UpdateData();

	AddBuddy(GetDlgItem(IDC_INCOMING)->m_hWnd, GetDlgItem(IDC_BROWSE)->m_hWnd, BDS_RIGHT);
	AddBuddy(GetDlgItem(IDC_TEMP)->m_hWnd, GetDlgItem(IDC_BROWSE_TEMP)->m_hWnd, BDS_RIGHT);

	return true;
}

void CCatDialog::UpdateData()
{
	SetDlgItemText(IDC_TITLE, m_pCat->GetTitle());
	SetDlgItemText(IDC_INCOMING, m_pCat->GetPath());
	SetDlgItemText(IDC_TEMP, m_pCat->GetTempPath());
	SetDlgItemText(IDC_COMMENT, m_pCat->GetComment());

	m_dwNewColor = m_pCat->GetColor();
	m_ctlColor.SetColor(m_dwNewColor);

	SetDlgItemText(IDC_AUTOCATEXT, m_pCat->GetAutoCatExt());

	m_cmbPriority.SetCurSel(m_pCat->GetPriority());
}

CCatDialog::~CCatDialog()
{
}

void CCatDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CATCOLOR, m_ctlColor);
	DDX_Control(pDX, IDC_PRIOCOMBO, m_cmbPriority);
}


BEGIN_MESSAGE_MAP(CCatDialog, CDialog)
	ON_BN_CLICKED(IDC_BROWSE, OnBnClickedBrowse)
	ON_BN_CLICKED(IDC_BROWSE_TEMP, OnBnClickedBrowseTemp)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_MESSAGE(CPN_SELENDOK, OnSelChange) //CPN_SELCHANGE
END_MESSAGE_MAP()

// CCatDialog message handlers

void CCatDialog::Localize()
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_STATIC_TITLE, IDS_TITLE },
		{ IDC_STATIC_COMMENT, IDS_COMMENT },
		{ IDCANCEL, IDS_CANCEL },
		{ IDC_STATIC_COLOR, IDS_COLOR },
		{ IDC_STATIC_PRIO, IDS_STARTPRIO },
		{ IDC_STATIC_AUTOCAT, IDS_AUTOCAT_LABEL },
		{ IDOK, IDS_OK_BUTTON }
	};
	static const uint16 s_auResList[] =
	{
		IDS_PRIOLOW, IDS_PRIONORMAL, IDS_PRIOHIGH, IDS_PRIOAUTO
	};

	CString strBuffer = GetResString(IDS_PW_INCOMING);

	strBuffer.Remove(_T(':'));
	SetDlgItemText(IDC_STATIC_INCOMING, strBuffer);

	GetResString(&strBuffer, IDS_PW_TEMP);
	strBuffer.Remove(_T(':'));
	SetDlgItemText(IDC_STATIC_TEMP, strBuffer);

	for (uint32 i = 0; i < ARRSIZE(s_auResTbl); i++)
	{
		GetResString(&strBuffer, static_cast<UINT>(s_auResTbl[i][1]));
		SetDlgItemText(s_auResTbl[i][0], strBuffer);
	}

	m_ctlColor.CustomText = GetResString(IDS_COL_MORECOLORS);
	m_ctlColor.DefaultText = GetResString(IDS_DEFAULT);
	m_ctlColor.SetDefaultColor(NULL);

	GetResString(&strBuffer, IDS_CAT_EDIT);
	SetWindowText(strBuffer);

	while (m_cmbPriority.GetCount() > 0)
		m_cmbPriority.DeleteString(0);
	m_cmbPriority.AddString(_T(""));	// Don't change
	for (uint32 i = 0; i < ARRSIZE(s_auResList); i++)
	{
		GetResString(&strBuffer, static_cast<UINT>(s_auResList[i]));
		m_cmbPriority.AddString(strBuffer);
	}
	m_cmbPriority.SetCurSel(m_pCat->GetPriority());
}

void CCatDialog::OnBnClickedBrowse()
{
	CString	strDir;

	GetDlgItemText(IDC_INCOMING, strDir);

	CString strNewPath = BrowseFolder(GetSafeHwnd(), GetResString(IDS_SELECTOUTPUTDIR), strDir);

	if (strNewPath != strDir)
		SetDlgItemText(IDC_INCOMING, strNewPath);
}

void CCatDialog::OnBnClickedBrowseTemp()
{
	CString	strDir;

	GetDlgItemText(IDC_TEMP, strDir);

	CString strNewPath = BrowseFolder(GetSafeHwnd(), GetResString(IDS_SELECT_TEMPDIR), strDir);

	if (strNewPath != strDir)
		SetDlgItemText(IDC_TEMP, strNewPath);
}

void CCatDialog::OnBnClickedOk()
{
	CString	strOldPath = m_pCat->GetPath();

	TCHAR		strPath[MAX_PATH];
	TCHAR		strTempPath[MAX_PATH];

	GetDlgItemText(IDC_TITLE, m_pCat->m_strTitle);
	if (m_pCat->m_strTitle.GetLength() > 64)
		m_pCat->m_strTitle.Truncate(64);

	if (GetDlgItem(IDC_INCOMING)->GetWindowTextLength() > 2)
	{
		GetDlgItemText(IDC_INCOMING, strPath, MAX_PATH);

		::MakeFolderName(strPath);
		m_pCat->m_strSavePath = strPath;

		if (!g_App.m_pPrefs->IsShareableDirectory(m_pCat->GetPath()))
		{
			_tcscpy(strPath,g_App.m_pPrefs->GetIncomingDir());
			::MakeFolderName(strPath);
			m_pCat->m_strSavePath = strPath;
		}

		CString	strNewPath = m_pCat->GetPath();

	//	Create all required directories
		CreateAllDirectories(&strNewPath);

		if (strNewPath.CompareNoCase(strOldPath) != 0)
		{
			if (g_App.m_pPrefs->SharedDirListCheckAndAdd(m_pCat->GetPath(), true))
			{
			//	New path was added to the list, scan for files
				g_App.m_pSharedFilesList->Reload();
			}
		}
	}

	if (GetDlgItem(IDC_TEMP)->GetWindowTextLength() > 2)
	{
		GetDlgItemText(IDC_TEMP, strTempPath, MAX_PATH);

		::MakeFolderName(strTempPath);
		m_pCat->m_strTempPath = strTempPath;
	}

	GetDlgItemText(IDC_COMMENT, m_pCat->m_strComment);

	m_pCat->m_crColor = m_dwNewColor;
	m_pCat->m_iPriority = static_cast<byte>(m_cmbPriority.GetCurSel());

	GetDlgItemText(IDC_AUTOCATEXT, m_pCat->m_strAutoCatExt);

	g_App.m_pMDlg->m_wndTransfer.m_ctlDownloadList.Invalidate();

	OnOK();
}

LONG CCatDialog::OnSelChange(UINT lParam, LONG wParam)
{
	NOPRM(wParam);
	if (lParam == CLR_DEFAULT)
		m_dwNewColor = 0;
	else
		m_dwNewColor = m_ctlColor.GetColor();

	return TRUE;
}
