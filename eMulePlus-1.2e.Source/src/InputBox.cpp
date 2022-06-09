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
#include "InputBox.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(InputBox, CDialog)

InputBox::InputBox(const CString &strTitle, const CString &strDefText, bool bEditFilenameMode/*=false*/)
	: CDialog(InputBox::IDD, NULL/*pParent*/),
	m_strTitle(strTitle), m_strDefault(strDefText)
{
	m_bCancel = true;
	m_bFilenameMode = bEditFilenameMode;
}

InputBox::~InputBox()
{
}

void InputBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(InputBox, CDialog)
	ON_BN_CLICKED(IDC_CLEANUP, OnCleanFilename)
END_MESSAGE_MAP()

void InputBox::OnOK()
{
	m_bCancel = false;
	GetDlgItemText(IDC_TEXT, m_strReturn);
	CDialog::OnOK();
}

BOOL InputBox::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetDlgItemText(IDC_TEXT, m_strDefault);
	SetWindowText(m_strTitle);

	SetDlgItemText(IDOK, GetResString(IDS_OK_BUTTON));
	SetDlgItemText(IDCANCEL, GetResString(IDS_CANCEL));
	SetDlgItemText(IDC_CLEANUP, GetResString(IDS_CLEANUP));
	GetDlgItem(IDC_CLEANUP)->ShowWindow((m_bFilenameMode) ? SW_NORMAL : SW_HIDE);

	return TRUE;
}

void InputBox::OnCleanFilename()
{
	CString strFilename;
	GetDlgItemText(IDC_TEXT, strFilename);
	SetDlgItemText(IDC_TEXT, CleanupFilename(strFilename));
}
