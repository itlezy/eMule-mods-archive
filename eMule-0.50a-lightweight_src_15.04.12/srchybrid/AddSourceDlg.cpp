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
#include "AddSourceDlg.h"
#include "PartFile.h"
#include "OtherFunctions.h"
#include "UpDownClient.h"
#include "DownloadQueue.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CAddSourceDlg dialog

IMPLEMENT_DYNAMIC(CAddSourceDlg, CDialog)

BEGIN_MESSAGE_MAP(CAddSourceDlg, CModResizableDialog)  // NEO: MLD - [ModelesDialogs] <-- Xanatos --
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

CAddSourceDlg::CAddSourceDlg(CWnd* pParent /*=NULL*/)
	: CModResizableDialog(CAddSourceDlg::IDD, pParent)  // NEO: MLD - [ModelesDialogs] <-- Xanatos --
{
	m_pFile = NULL;
}

CAddSourceDlg::~CAddSourceDlg()
{
}

void CAddSourceDlg::SetFile(CPartFile *pFile)
{
	m_pFile = pFile;
}

BOOL CAddSourceDlg::OnInitDialog()
{
	CModResizableDialog::OnInitDialog();
	InitWindowStyles(this);

	AddAnchor(IDOK, BOTTOM_RIGHT);
	AddAnchor(IDC_BUTTON1, BOTTOM_RIGHT);
	AddAnchor(IDCANCEL, BOTTOM_RIGHT);

	if (m_pFile)
		SetWindowText(m_pFile->GetFileName());

	// localize
	SetDlgItemText(IDC_BUTTON1,GetResString(IDS_ADD));
	SetDlgItemText(IDCANCEL,GetResString(IDS_CANCEL));
	SetDlgItemText(IDC_UIP,GetResString(IDS_USERSIP));
	SetDlgItemText(IDC_PORT,GetResString(IDS_PORT));	
	SetDlgItemText(IDOK,GetResString(IDS_TREEOPTIONS_OK));

	EnableSaveRestore(_T("AddSourceDlg"), !thePrefs.prefReadonly); // X: [ROP] - [ReadOnlyPreference]

	GetDlgItem(IDC_EDIT2)->SetFocus();
	return FALSE; // return FALSE, we changed the focus!
}

void CAddSourceDlg::OnBnClickedButton1()
{
	if (!m_pFile)
		return;

	CString sip;
	GetDlgItemText(IDC_EDIT2,sip);
	if (sip.IsEmpty())
		return;

	// if the port is specified with the IP, ignore any possible specified port in the port control
	uint16 port;
	int iColon = sip.Find(_T(':'));
	if (iColon != -1) {
		port = (uint16)_tstoi(sip.Mid(iColon + 1));
		sip = sip.Left(iColon);
	}
	else {
		BOOL bTranslated = FALSE;
		port = (uint16)GetDlgItemInt(IDC_EDIT3, &bTranslated, FALSE);
		if (!bTranslated)
			return;
	}

	uint32 ip;
	if ((ip = inet_addr(CT2CA(sip))) == INADDR_NONE && _tcscmp(sip, _T("255.255.255.255")) != 0)
		ip = 0;
	if (IsGoodIPPort(ip, port))
	{
		CUpDownClient* toadd = new CUpDownClient(m_pFile, port, ntohl(ip), 0, 0);
		toadd->SetSourceFrom(SF_PASSIVE);
		theApp.downloadqueue->CheckAndAddSource(m_pFile, toadd);
	}
}

void CAddSourceDlg::OnBnClickedOk()
{
	OnBnClickedButton1();
	OnOK();
}
