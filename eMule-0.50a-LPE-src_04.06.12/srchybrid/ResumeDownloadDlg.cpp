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
#include "ResumeDownloadDlg.h"
#include "OtherFunctions.h"
#include "emuleDlg.h"
#include "DownloadQueue.h"
#include "ED2KLink.h"
#include "Preferences.h"
#include "KnownFileList.h" //Xman [MoNKi: -Check already downloaded files-]
#include "TransferWnd.h"
#include "Log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define	PREF_INI_SECTION	_T("ResumeDownloadDlg")

IMPLEMENT_DYNAMIC(CResumeDownloadDlg, CDialog)

BEGIN_MESSAGE_MAP(CResumeDownloadDlg, CModResizableDialog) // NEO: MLD - [ModelesDialogs] <-- Xanatos --
	ON_EN_KILLFOCUS(IDC_ELINK, OnEnKillfocusElink)
	ON_EN_UPDATE(IDC_ELINK, UpdateControls)
	ON_EN_UPDATE(IDC_FILENAME, UpdateControls)
	ON_BN_CLICKED(IDC_BROWSE, OnBnClickedBrowse)
END_MESSAGE_MAP()

CResumeDownloadDlg::CResumeDownloadDlg(CWnd* pParent /*=NULL*/)
	: CModResizableDialog(CResumeDownloadDlg::IDD, pParent) // NEO: MLD - [ModelesDialogs] <-- Xanatos --
{
	m_icnWnd = NULL;
}

CResumeDownloadDlg::~CResumeDownloadDlg()
{
	if (m_icnWnd)
		VERIFY( DestroyIcon(m_icnWnd) );
}

void CResumeDownloadDlg::UpdateControls()
{
	GetDlgItem(IDOK)->EnableWindow(GetDlgItem(IDC_ELINK)->GetWindowTextLength() > 0 && GetDlgItem(IDC_FILENAME)->GetWindowTextLength() > 0);
}

void CResumeDownloadDlg::OnEnKillfocusElink()
{
	CString strLinks;
	GetDlgItemText(IDC_ELINK,strLinks);
	if (strLinks.IsEmpty() || strLinks.Find(_T('\n')) == -1)
		return;
	strLinks.Replace(_T("\n"), _T("\r\n"));
	strLinks.Replace(_T("\r\r"), _T("\r"));
	SetDlgItemText(IDC_ELINK,strLinks);
}

void CResumeDownloadDlg::OnOK()
{
	CString pathname;
	GetDlgItemText(IDC_FILENAME,pathname);
	if(PathFileExists(pathname)){
		CString strLinks;
		GetDlgItemText(IDC_ELINK,strLinks);

		int curPos = 0;
		CString strTok = strLinks.Tokenize(_T(" \t\r\n"), curPos); // tokenize by whitespaces
		if (!strTok.IsEmpty())
		{
			if (strTok.Right(1) != _T('/'))
				strTok += _T('/');
			try
			{
				CED2KLink* pLink = CED2KLink::CreateLinkFromUrl(strTok);
				if (pLink){
					if (pLink->GetKind() == CED2KLink::kFile){
						//Xman [MoNKi: -Check already downloaded files-]
						if(theApp.knownfiles->CheckAlreadyDownloadedFileQuestion(((CED2KFileLink*)pLink)/*->GetFileLink()*/->GetHashKey(),((CED2KFileLink*)pLink)/*->GetFileLink()*/->GetName()))
							theApp.downloadqueue->AddFileLinkToDownload((CED2KFileLink*)pLink/*->GetFileLink()*/, (thePrefs.GetCatCount() > 1) ? theApp.emuledlg->transferwnd->downloadlistctrl.curTab : 0, pathname);
						//Xman end
					}
					else{
						delete pLink;
						throw GetResString(IDS_ERR_NOTAFILELINK);
					}
					delete pLink;
				}
			}
			catch(CString error)
			{
				TCHAR szBuffer[200];
				_sntprintf(szBuffer, _countof(szBuffer) - 1, GetResString(IDS_ERR_INVALIDLINK), error);
				szBuffer[_countof(szBuffer) - 1] = _T('\0');
				LogError(LOG_STATUSBAR, GetResString(IDS_ERR_LINKERROR), szBuffer);
			}
		}
	}
	CModResizableDialog::OnOK(); // NEO: MLD - [ModelesDialogs] <-- Xanatos --
}

BOOL CResumeDownloadDlg::OnInitDialog()
{
	CModResizableDialog::OnInitDialog(); // NEO: MLD - [ModelesDialogs] <-- Xanatos --
	InitWindowStyles(this);

	AddAnchor(IDC_ELINK, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDCANCEL, BOTTOM_RIGHT);
	AddAnchor(IDOK, BOTTOM_RIGHT);
	AddAnchor(IDC_FNAME, BOTTOM_LEFT);
	AddAnchor(IDC_FILENAME, BOTTOM_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_BROWSE, BOTTOM_RIGHT);

	EnableSaveRestore(PREF_INI_SECTION, !thePrefs.prefReadonly); // X: [ROP] - [ReadOnlyPreference]

	SetWindowText(GetResString(IDS_SW_RESUMEDOWNLOAD));
	SetDlgItemText(IDC_FSTATIC2,GetResString(IDS_SW_LINK));
	SetDlgItemText(IDC_FNAME,GetResString(IDS_DL_FILENAME));

	SetDlgItemText(IDOK,GetResString(IDS_DOWNLOAD));
	SetDlgItemText(IDCANCEL,GetResString(IDS_CANCEL));

	UpdateControls();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CResumeDownloadDlg::OnBnClickedBrowse()
{
	CFileDialog dlg(true, NULL, NULL, OFN_FILEMUSTEXIST|OFN_HIDEREADONLY);
	if(dlg.DoModal()!=IDOK)
		return;
	SetDlgItemText(IDC_FILENAME, dlg.GetPathName());
}
