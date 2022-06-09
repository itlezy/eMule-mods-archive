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
#include "FDGeneral.h"
#include "file_ver.h"
#include "..\otherfunctions.h"
#include "..\emule.h"
#include "..\CommentDialogLst.h"

#pragma comment(lib, "version.lib")

IMPLEMENT_DYNCREATE(CFDGeneral, CPropertyPage)

CFDGeneral::CFDGeneral() : CPropertyPage(CFDGeneral::IDD)
{
	m_pFile = NULL;
	m_hFileIcon = NULL;
}

CFDGeneral::~CFDGeneral()
{
	if(m_hFileIcon != NULL)
		::DestroyIcon(m_hFileIcon);
}

void CFDGeneral::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PARTFILESTATUS_VAL, m_ctrlPartFileStatus);
	DDX_Control(pDX, IDC_METFILE_VAL, m_ctrlMetFile);
	DDX_Control(pDX, IDC_HASH_VAL, m_ctrlHash);
	DDX_Control(pDX, IDC_FOLDER_VAL, m_ctrlFolder);
	DDX_Control(pDX, IDC_FILETYPE_VAL, m_ctrlFiletype);
	DDX_Control(pDX, IDC_FILESIZE_VAL, m_ctrlFilesize);
	DDX_Control(pDX, IDC_FILEICON, m_ctrlFileIcon);
	DDX_Control(pDX, IDC_DESCRIPTION_VAL, m_ctrlDescription);
	DDX_Control(pDX, IDC_FDG_FILENAME, m_ctrlFilename);
	DDX_Control(pDX, IDC_CATEGORY_VAL, m_ctrlCategory);
}


BEGIN_MESSAGE_MAP(CFDGeneral, CPropertyPage)
	ON_BN_CLICKED(IDC_COMMENTS_BTN, OnBnClickedCommentsBtn)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
bool IsExecuteable(const CString &strFile)
{
	int iDot = strFile.ReverseFind(_T('.'));

	if((iDot <= 0) || ((strFile.GetLength() - iDot) != 4))
	{
		return false;		// no file extension found or it has different size
	}

	CString strExt = strFile.Right(strFile.GetLength() - iDot - 1);

	strExt.MakeUpper();
	return ((strExt == _T("EXE")) || (strExt == _T("DLL")));
}

void CFDGeneral::Update()
{
	EMULE_TRY

	if (m_pFile == NULL || !::IsWindow(GetSafeHwnd()))
		return;

	m_ctrlFilename.SetWindowText(m_pFile->GetFileName());

	CCat		*pCat = CCat::GetCatByID(m_pFile->GetCatID());

	m_ctrlCategory.SetWindowText((pCat != NULL) ? pCat->GetTitle() : GetResString(IDS_CAT_UNCATEGORIZED));

	CString		strOldDir, strNewDir = m_pFile->GetOutputDir();

	if (strNewDir.GetLength() > 3)
		 strNewDir += _T("\\");

	m_ctrlFolder.GetWindowText(strOldDir);
	if (strOldDir != strNewDir)
		m_ctrlFolder.SetWindowText(strNewDir);
	m_ctrlFilesize.SetWindowText(CastItoXBytes(m_pFile->GetFileSize()));
	m_ctrlMetFile.SetWindowText(m_pFile->GetFilePath());
	m_ctrlHash.SetWindowText(HashToString(m_pFile->GetFileHash()));
	m_ctrlPartFileStatus.SetWindowText(m_pFile->GetPartFileStatus());

	EMULE_CATCH
}

void CFDGeneral::OnBnClickedCommentsBtn()
{
	CCommentDialogLst dialog(m_pFile);
	dialog.DoModal();
}

BOOL CFDGeneral::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

//	Setup file description information
	if (m_pFile != NULL)
	{
	//	Due to extension changing is unlike during file rename, file description and
	//	icon are loaded only at start not to make these "heavy" calls in Update()
		CFileVersionInfo	fvInfo;
		SHFILEINFO	shfi;
		bool		bIsExecuteable;
		TCHAR		szExecuteable[MAX_PATH];
		const TCHAR	*pExeStr, *pDescStr;
		CString		strFilename(m_pFile->GetFileName());

	//	Load file icon
		memzero(&shfi, sizeof(shfi));
		SHGetFileInfo( strFilename, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi),
						SHGFI_ICON|SHGFI_USEFILEATTRIBUTES|SHGFI_TYPENAME|SHGFI_DISPLAYNAME );
		m_hFileIcon = shfi.hIcon;
		m_ctrlFileIcon.SetIcon(m_hFileIcon);
		m_ctrlFiletype.SetWindowText(shfi.szTypeName);

	//	Setup file description
		bIsExecuteable = IsExecuteable(strFilename);
		if (bIsExecuteable)
		{
			strFilename = m_pFile->GetFilePath();
			pExeStr = strFilename;
			pDescStr = shfi.szDisplayName;
		}
		else
		{
			CString	strTempFile = g_App.m_pPrefs->GetTempDir();

			strTempFile += _T("\\$$$");
			strTempFile += strFilename;

			HANDLE	hTmpFile = ::CreateFile(strTempFile, GENERIC_READ, FILE_SHARE_READ, NULL,
				CREATE_NEW, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY, NULL);

			if (hTmpFile != INVALID_HANDLE_VALUE)
				::CloseHandle(hTmpFile);
			FindExecutable(strTempFile, _T(""), szExecuteable);
			if (hTmpFile != INVALID_HANDLE_VALUE)
				::DeleteFile(strTempFile);

			pExeStr = szExecuteable;
			pDescStr = szExecuteable;
		}
		fvInfo.ReadVersionInfo(pExeStr);
		if (fvInfo.IsValid() && fvInfo.IsVersionInfoAvailable(SFI_FILEDESCRIPTION))
			m_ctrlDescription.SetWindowText(fvInfo.GetVersionInfo(SFI_FILEDESCRIPTION));
		else
			m_ctrlDescription.SetWindowText(pDescStr);

		Localize(bIsExecuteable);
		Update();
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFDGeneral::Localize(bool bIsExecuteable)
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_FILETYPE_LBL, IDS_FILETYPE_LBL },
		{ IDC_FOLDER_LBL, IDS_FD_OUTPUT },
		{ IDC_FILESIZE_LBL, IDS_FD_SIZE },
		{ IDC_COMMENTS_BTN, IDS_CMT_SHOWALL }
	};
	static const uint16 s_auResTbl2[][2] =
	{
		{ IDC_HASH_LBL, IDS_HASH },
		{ IDC_CATEGORY_LBL, IDS_CAT },
		{ IDC_METFILE_LBL, IDS_DL_FILENAME },
		{ IDC_PARTFILESTATUS_LBL, IDS_STATUS }
	};

	if (GetSafeHwnd())
	{
		CString strBuffer = GetResString((bIsExecuteable) ? IDS_FD_DESCRIPTION : IDS_FD_OPENSWITH);

		SetDlgItemText(IDC_DESCRIPTION_LBL, strBuffer);

		for (uint32 i = 0; i < ARRSIZE(s_auResTbl); i++)
		{
			GetResString(&strBuffer, static_cast<UINT>(s_auResTbl[i][1]));
			SetDlgItemText(s_auResTbl[i][0], strBuffer);
		}

		for (uint32 i = 0; i < ARRSIZE(s_auResTbl2); i++)
		{
			GetResString(&strBuffer, static_cast<UINT>(s_auResTbl2[i][1]));
			strBuffer += _T(":");
			SetDlgItemText(s_auResTbl2[i][0], strBuffer);
		}
	}
}
