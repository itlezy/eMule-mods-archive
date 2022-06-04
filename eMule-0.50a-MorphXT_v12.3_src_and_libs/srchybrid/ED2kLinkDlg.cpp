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
#include "ED2kLinkDlg.h"
#include "KnownFile.h"
#include "partfile.h"
#include "preferences.h"
#include "shahashset.h"
#include "UserMsgs.h"

//emulEspaña. [MoNKi: -HTTP Sources in eLinks-]
#include "StringConversion.h"
//End emulEspaña

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CED2kLinkDlg, CResizablePage) 

BEGIN_MESSAGE_MAP(CED2kLinkDlg, CResizablePage) 
	ON_BN_CLICKED(IDC_LD_CLIPBOARDBUT, OnBnClickedClipboard)
	ON_BN_CLICKED(IDC_LD_SOURCECHE, OnSettingsChange)
	ON_BN_CLICKED(IDC_LD_HTMLCHE, OnSettingsChange)
	ON_BN_CLICKED(IDC_LD_HOSTNAMECHE, OnSettingsChange)
	ON_BN_CLICKED(IDC_LD_HASHSETCHE, OnSettingsChange)
	ON_MESSAGE(UM_DATA_CHANGED, OnDataChanged)
	//EastShare Start - added by AndCycle, phpBB URL-Tags style link
	ON_BN_CLICKED(IDC_LD_PHPBBCHE, OnSettingsChange)
	//EastShare End - added by AndCycle, phpBB URL-Tags style link
	//emulEspaña. Added by MoNKi [MoNKi: -HTTP Sources in eLinks-]
	ON_BN_CLICKED(IDC_HTTP_SRC, OnSettingsChange)
	ON_BN_CLICKED(IDC_BTN_HTTP_ADD, OnBtnAddHttp)
	ON_BN_CLICKED(IDC_BTN_HTTP_REMOVE, OnBtnRemoveHttp)
	ON_BN_CLICKED(IDC_BTN_HTTP_CLEAR, OnBtnClearHttp)
	//End emulEspaña
END_MESSAGE_MAP() 

CED2kLinkDlg::CED2kLinkDlg() 
   : CResizablePage(CED2kLinkDlg::IDD, IDS_CMT_READALL) 
{ 
	m_paFiles = NULL;
	m_bDataChanged = false;
	m_strCaption = GetResString(IDS_SW_LINK);
	m_psp.pszTitle = m_strCaption;
	m_psp.dwFlags |= PSP_USETITLE;
	m_bReducedDlg = false;
} 

CED2kLinkDlg::~CED2kLinkDlg() 
{ 
} 

void CED2kLinkDlg::DoDataExchange(CDataExchange* pDX) 
{ 
	CResizablePage::DoDataExchange(pDX); 
	DDX_Control(pDX, IDC_LD_LINKEDI, m_ctrlLinkEdit);
} 

BOOL CED2kLinkDlg::OnInitDialog()
{ 
	CResizablePage::OnInitDialog(); 
	InitWindowStyles(this);



	if (!m_bReducedDlg)
	{
		AddAnchor(IDC_LD_BASICGROUP,BOTTOM_LEFT,BOTTOM_RIGHT);
		AddAnchor(IDC_LD_SOURCECHE,BOTTOM_LEFT,BOTTOM_LEFT);
		AddAnchor(IDC_LD_ADVANCEDGROUP,BOTTOM_LEFT,BOTTOM_RIGHT);
		AddAnchor(IDC_LD_HTMLCHE,BOTTOM_LEFT,BOTTOM_LEFT);
		AddAnchor(IDC_LD_HASHSETCHE,BOTTOM_LEFT,BOTTOM_LEFT);
		AddAnchor(IDC_LD_HOSTNAMECHE,BOTTOM_LEFT,BOTTOM_LEFT);

		//EastShare Start - added by AndCycle, phpBB URL-Tags style link
		AddAnchor(IDC_LD_PHPBBCHE,BOTTOM_LEFT,BOTTOM_LEFT);
		//EastShare End - added by AndCycle, phpBB URL-Tags style link
		//emulEspaña. Added by MoNKi [MoNKi: -HTTP Sources in eLinks-]
		AddAnchor(IDC_HTTP_SRC,BOTTOM_LEFT,BOTTOM_RIGHT);
		AddAnchor(IDC_EDIT_HTTP,BOTTOM_LEFT,BOTTOM_RIGHT);
		AddAnchor(IDC_BTN_HTTP_ADD,BOTTOM_RIGHT,BOTTOM_RIGHT);
		AddAnchor(IDC_LIST_HTTP,BOTTOM_LEFT,BOTTOM_RIGHT);
		AddAnchor(IDC_BTN_HTTP_REMOVE,BOTTOM_RIGHT,BOTTOM_RIGHT);
		AddAnchor(IDC_BTN_HTTP_CLEAR,BOTTOM_RIGHT,BOTTOM_RIGHT);
		if(m_paFiles->GetSize() > 1){
			GetDlgItem(IDC_HTTP_SRC)->EnableWindow(FALSE);
		}
		//End emulEspaña

		// enabled/disable checkbox depending on situation
		if (theApp.IsConnected() && !theApp.IsFirewalled())
			GetDlgItem(IDC_LD_SOURCECHE)->EnableWindow(TRUE);
		else{
			GetDlgItem(IDC_LD_SOURCECHE)->EnableWindow(FALSE);
			((CButton*)GetDlgItem(IDC_LD_SOURCECHE))->SetCheck(BST_UNCHECKED);
		}
		if (theApp.IsConnected() && !theApp.IsFirewalled() && !thePrefs.GetYourHostname().IsEmpty() && thePrefs.GetYourHostname().Find(_T('.')) != -1)
			GetDlgItem(IDC_LD_HOSTNAMECHE)->EnableWindow(TRUE);
		else{
			GetDlgItem(IDC_LD_HOSTNAMECHE)->EnableWindow(FALSE);
			((CButton*)GetDlgItem(IDC_LD_HOSTNAMECHE))->SetCheck(BST_UNCHECKED);
		}
	}
	else
	{
		CRect rcDefault, rcNew;
		GetDlgItem(IDC_LD_LINKGROUP)->GetWindowRect(rcDefault);
		GetDlgItem(IDC_LD_ADVANCEDGROUP)->GetWindowRect(rcNew);
		int nDeltaY = rcNew.bottom - rcDefault.bottom;
		GetDlgItem(IDC_LD_LINKGROUP)->SetWindowPos(NULL, 0, 0, rcDefault.Width(), rcDefault.Height() + nDeltaY, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
		GetDlgItem(IDC_LD_LINKEDI)->GetWindowRect(rcDefault);
		GetDlgItem(IDC_LD_LINKEDI)->SetWindowPos(NULL, 0, 0, rcDefault.Width(), rcDefault.Height() + nDeltaY, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
		GetDlgItem(IDC_LD_CLIPBOARDBUT)->GetWindowRect(rcDefault);
		ScreenToClient(rcDefault);
		GetDlgItem(IDC_LD_CLIPBOARDBUT)->SetWindowPos(NULL, rcDefault.left, rcDefault.top + nDeltaY, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
		
		GetDlgItem(IDC_LD_BASICGROUP)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LD_SOURCECHE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LD_ADVANCEDGROUP)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LD_HTMLCHE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LD_HASHSETCHE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LD_HOSTNAMECHE)->ShowWindow(SW_HIDE);
	}
	AddAnchor(IDC_LD_LINKGROUP,TOP_LEFT,BOTTOM_RIGHT);
	AddAnchor(IDC_LD_LINKEDI,TOP_LEFT,BOTTOM_RIGHT);
	AddAnchor(IDC_LD_CLIPBOARDBUT,BOTTOM_RIGHT);
	Localize(); 

	return TRUE; 
} 

BOOL CED2kLinkDlg::OnSetActive()
{
	if (!CResizablePage::OnSetActive())
		return FALSE;

	if (m_bDataChanged)
	{
		//hashsetlink - check if at least one file has a hasset
		BOOL bShowHashset = FALSE;
		BOOL bShowAICH = FALSE;
		BOOL bShowHTML = FALSE;
		for (int i = 0; i != m_paFiles->GetSize(); i++){
			if (!(*m_paFiles)[i]->IsKindOf(RUNTIME_CLASS(CKnownFile)))
				continue;
			bShowHTML = TRUE;
			CKnownFile* file = STATIC_DOWNCAST(CKnownFile, (*m_paFiles)[i]);
			if (file->GetFileIdentifier().GetAvailableMD4PartHashCount() > 0 && file->GetFileIdentifier().HasExpectedMD4HashCount())
			{
				bShowHashset = TRUE;
			}
			if (file->GetFileIdentifier().HasAICHHash())
			{	
				bShowAICH = TRUE;
			}
			if (bShowHashset && bShowAICH)
				break;
		}
		GetDlgItem(IDC_LD_HASHSETCHE)->EnableWindow(bShowHashset);
		if (!bShowHashset)
			((CButton*)GetDlgItem(IDC_LD_HASHSETCHE))->SetCheck(BST_UNCHECKED);

		GetDlgItem(IDC_LD_HTMLCHE)->EnableWindow(bShowHTML);

		UpdateLink();
		m_bDataChanged = false;
	}

	return TRUE;
}

LRESULT CED2kLinkDlg::OnDataChanged(WPARAM, LPARAM)
{
	//emulEspaña. Added by MoNKi [MoNKi: -HTTP Sources in eLinks-]
	CListBox *httpList = (CListBox *) GetDlgItem(IDC_LIST_HTTP);
	if(httpList)
		httpList->ResetContent(); 
	//End emulEspaña
	m_bDataChanged = true;
	return 1;
}

void CED2kLinkDlg::Localize(void)
{ 	
	GetDlgItem(IDC_LD_LINKGROUP)->SetWindowText(GetResString(IDS_SW_LINK)); 
	GetDlgItem(IDC_LD_CLIPBOARDBUT)->SetWindowText(GetResString(IDS_LD_COPYCLIPBOARD));
	if (!m_bReducedDlg)
	{
		GetDlgItem(IDC_LD_BASICGROUP)->SetWindowText(GetResString(IDS_LD_BASICOPT));
		GetDlgItem(IDC_LD_SOURCECHE)->SetWindowText(GetResString(IDS_LD_ADDSOURCE)); 
		GetDlgItem(IDC_LD_ADVANCEDGROUP)->SetWindowText(GetResString(IDS_LD_ADVANCEDOPT)); 
		GetDlgItem(IDC_LD_HTMLCHE)->SetWindowText(GetResString(IDS_LD_ADDHTML)); 
		GetDlgItem(IDC_LD_HASHSETCHE)->SetWindowText(GetResString(IDS_LD_ADDHASHSET)); 
		GetDlgItem(IDC_LD_HOSTNAMECHE)->SetWindowText(GetResString(IDS_LD_HOSTNAME));
		//EastShare Start - added by AndCycle, phpBB URL-Tags style link
		GetDlgItem(IDC_LD_PHPBBCHE)->SetWindowText(GetResString(IDS_LD_ADDPHPBB));
		//EastShare End - added by AndCycle, phpBB URL-Tags style link
		//emulEspaña. Added by MoNKi [MoNKi: -HTTP Sources in eLinks-]
		GetDlgItem(IDC_HTTP_SRC)->SetWindowText(GetResString(IDS_HTTP_SRC)); 
		GetDlgItem(IDC_BTN_HTTP_ADD)->SetWindowText(GetResString(IDS_ADD)); 
		GetDlgItem(IDC_BTN_HTTP_REMOVE)->SetWindowText(GetResString(IDS_REMOVE)); 
		GetDlgItem(IDC_BTN_HTTP_CLEAR)->SetWindowText(GetResString(IDS_CLEAR)); 
		//End emulEspaña
	}
}

void CED2kLinkDlg::UpdateLink()
{
	CString strLinks;
	CString strBuffer;
	const bool bHashset = ((CButton*)GetDlgItem(IDC_LD_HASHSETCHE))->GetCheck() == BST_CHECKED;
	const bool bHTML = ((CButton*)GetDlgItem(IDC_LD_HTMLCHE))->GetCheck() == BST_CHECKED;
	const bool bSource = ((CButton*)GetDlgItem(IDC_LD_SOURCECHE))->GetCheck() == BST_CHECKED && theApp.IsConnected() && theApp.GetPublicIP() != 0 && !theApp.IsFirewalled();
	const bool bHostname = ((CButton*)GetDlgItem(IDC_LD_HOSTNAMECHE))->GetCheck() == BST_CHECKED && theApp.IsConnected() && !theApp.IsFirewalled()
		&& !thePrefs.GetYourHostname().IsEmpty() && thePrefs.GetYourHostname().Find(_T('.')) != -1;
	//EastShare Start - added by AndCycle, phpBB URL-Tags style link
	const bool bPHPBB = ((CButton*)GetDlgItem(IDC_LD_PHPBBCHE))->GetCheck() == BST_CHECKED && !bHTML;
	//EastShare End - added by AndCycle, phpBB URL-Tags style link
	//emulEspaña. Added by MoNKi [MoNKi: -HTTP Sources in eLinks-]
	const bool bHTTP = ((CButton*)GetDlgItem(IDC_HTTP_SRC))->GetCheck() == BST_CHECKED;
	//End emulEspaña

	for (int i = 0; i != m_paFiles->GetSize(); i++)
	{
		if (!(*m_paFiles)[i]->IsKindOf(RUNTIME_CLASS(CKnownFile)))
			continue;

		if (!strLinks.IsEmpty())
			strLinks += _T("\r\n\r\n");
	
		//emulEspaña. Added by MoNKi [MoNKi: -HTTP Sources in eLinks-]
		CListBox *httpList = NULL;
		if(bHTTP)
			httpList = (CListBox *) GetDlgItem(IDC_LIST_HTTP);
		//End emulEspaña

		CKnownFile* file = STATIC_DOWNCAST(CKnownFile, (*m_paFiles)[i]);
		//EastShare Start - added by AndCycle, phpBB URL-Tags style link
		//emulEspaña. Added by MoNKi [MoNKi: -HTTP Sources in eLinks-]
		/*
		strLinks += file->GetED2kLink(bHashset, bHTML, bHostname, bSource, theApp.GetPublicIP());
		*/
		strLinks += file->GetED2kLink(bHashset, bHTML, bHostname, bSource, theApp.GetPublicIP(),bPHPBB, httpList);
		//End emulEspaña
		//EastShare End - added by AndCycle, phpBB URL-Tags style link
	}
	m_ctrlLinkEdit.SetWindowText(strLinks);

}

void CED2kLinkDlg::OnBnClickedClipboard()
{
	CString strBuffer;
	m_ctrlLinkEdit.GetWindowText(strBuffer);
	theApp.CopyTextToClipboard(strBuffer);
}

void CED2kLinkDlg::OnSettingsChange()
{

	//EastShare Start - added by AndCycle, phpBB URL-Tags style link
	//hide up if using html checked
	if (((CButton*)GetDlgItem(IDC_LD_HTMLCHE))->GetCheck() == BST_CHECKED) {
		//((CButton*)GetDlgItem(IDC_LD_PHPBBCHE))->SetCheck(BST_UNCHECKED);
		GetDlgItem(IDC_LD_PHPBBCHE)->EnableWindow(FALSE);
	}
	else 
		GetDlgItem(IDC_LD_PHPBBCHE)->EnableWindow(TRUE);

	if (((CButton*)GetDlgItem(IDC_LD_PHPBBCHE))->GetCheck() == BST_CHECKED) {
		//((CButton*)GetDlgItem(IDC_LD_HTMLCHE))->SetCheck(BST_UNCHECKED);
		GetDlgItem(IDC_LD_HTMLCHE)->EnableWindow(FALSE);
	}
	else 
		GetDlgItem(IDC_LD_HTMLCHE)->EnableWindow(TRUE);
	//EastShare End - added by AndCycle, phpBB URL-Tags style link
	//emulEspaña. Added by MoNKi [MoNKi: -HTTP Sources in eLinks-]
	const bool bHTTP = ((CButton*)GetDlgItem(IDC_HTTP_SRC))->GetCheck() == BST_CHECKED;
	GetDlgItem(IDC_EDIT_HTTP)->EnableWindow(bHTTP);
	GetDlgItem(IDC_LIST_HTTP)->EnableWindow(bHTTP);
	GetDlgItem(IDC_BTN_HTTP_ADD)->EnableWindow(bHTTP);
	GetDlgItem(IDC_BTN_HTTP_REMOVE)->EnableWindow(bHTTP);
	GetDlgItem(IDC_BTN_HTTP_CLEAR)->EnableWindow(bHTTP);
	//End emulEspaña
	UpdateLink();
}

BOOL CED2kLinkDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (LOWORD(wParam) == IDCANCEL)
		return ::SendMessage(::GetParent(m_hWnd), WM_COMMAND, wParam, lParam);
	return CResizablePage::OnCommand(wParam, lParam);
}

//emulEspaña. Added by MoNKi [MoNKi: -HTTP Sources in eLinks-]
void CED2kLinkDlg::OnBtnAddHttp()
{
	CListBox *httpList = (CListBox *) GetDlgItem(IDC_LIST_HTTP);
	if(httpList){
		CString httpSrc;
		GetDlgItem(IDC_EDIT_HTTP)->GetWindowText(httpSrc);
		if(!httpSrc.IsEmpty()){
			if(httpSrc.Left(7).CompareNoCase(_T("http://")) == 0){
				httpList->AddString(httpSrc);
				GetDlgItem(IDC_EDIT_HTTP)->SetWindowText(_T(""));
			}
			else{
				MessageBox(GetResString(IDS_INVALID_HTTP_SRC), 0, MB_OK | MB_ICONEXCLAMATION);
			}
		}
	}

	UpdateLink();
}

void CED2kLinkDlg::OnBtnRemoveHttp()
{
	CListBox *httpList = (CListBox *) GetDlgItem(IDC_LIST_HTTP);
	if(httpList){
		httpList->DeleteString(httpList->GetCurSel());
	}	
	UpdateLink();
}

void CED2kLinkDlg::OnBtnClearHttp()
{
	if(MessageBox(GetResString(IDS_HTTP_LIST_CLEAR), 0, MB_YESNO | MB_ICONQUESTION) == IDYES){
		CListBox *httpList = (CListBox *) GetDlgItem(IDC_LIST_HTTP);
		if(httpList){
			httpList->ResetContent(); 
		}		

		UpdateLink();
	}
}
//emulEspaña. Added by MoNKi [MoNKi: -HTTP Sources in eLinks-]