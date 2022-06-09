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
#include "CustomAutoComplete.h"
#include "Preferences.h"
#include "otherfunctions.h"
#include "SharedFileList.h"
#include "emuledlg.h"
#include "TransferDlg.h"
#include "CatDialog.h"
#include "UserMsgs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	REGULAREXPRESSIONS_STRINGS_PROFILE	_T("AC_VF_RegExpr.dat")

// CCatDialog dialog

IMPLEMENT_DYNAMIC(CCatDialog, CDialog)

BEGIN_MESSAGE_MAP(CCatDialog, CDialog)
	ON_BN_CLICKED(IDC_BROWSE, OnBnClickedBrowse)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_DD, OnDDBnClicked)
	ON_MESSAGE(UM_CPN_SELENDOK, OnSelChange) //UM_CPN_SELCHANGE
END_MESSAGE_MAP()

CCatDialog::CCatDialog(size_t index)
	: CDialog(CCatDialog::IDD)
{
	m_myCat = thePrefs.GetCategory(index);
	if (m_myCat == NULL)
		return;
	m_pacRegExp=NULL;
	newcolor = (DWORD)-1;
}

CCatDialog::~CCatDialog()
{
	if (m_pacRegExp){
		m_pacRegExp->SaveList(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + REGULAREXPRESSIONS_STRINGS_PROFILE);
		m_pacRegExp->Unbind();
		m_pacRegExp->Release();
	}
}

BOOL CCatDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	InitWindowStyles(this);
	Localize();
	
	m_temp.ResetContent();// X: [TD] - [TempDir]
	m_temp.AddString(_T(""));
	for (size_t i = 0; i < thePrefs.tempdir.GetCount(); i++) 
		m_temp.AddString(thePrefs.GetTempDir(i));

	m_ctlColor.SetDefaultColor(GetSysColor(COLOR_BTNTEXT));
	UpdateData();

	if (!thePrefs.IsExtControlsEnabled()) {
		GetDlgItem(IDC_REGEXPR)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_REGEXP)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_REGEXP)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DD)->ShowWindow(SW_HIDE);
	}
	else if (thePrefs.GetUseAutocompletion()) {
		m_pacRegExp = new CCustomAutoComplete();
		m_pacRegExp->AddRef();
		if (m_pacRegExp->Bind(::GetDlgItem(m_hWnd, IDC_REGEXP), ACO_UPDOWNKEYDROPSLIST | ACO_AUTOSUGGEST)) {
			m_pacRegExp->LoadList(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + REGULAREXPRESSIONS_STRINGS_PROFILE);
		}
		if (theApp.m_fontSymbol.m_hObject){
			GetDlgItem(IDC_DD)->SetFont(&theApp.m_fontSymbol);
			SetDlgItemText(IDC_DD,_T("6")); // show a down-arrow
		}
	}
	else
		GetDlgItem(IDC_DD)->ShowWindow(SW_HIDE);


	return TRUE;
}

void CCatDialog::UpdateData()
{
	SetDlgItemText(IDC_TITLE,m_myCat->strTitle);
	SetDlgItemText(IDC_INCOMING,m_myCat->strIncomingPath);
	SetDlgItemText(IDC_COMMENT,m_myCat->strComment);

	SetDlgItemText(IDC_REGEXP,m_myCat->regexp);

	CheckDlgButton(IDC_REGEXPR,m_myCat->ac_regexpeval);

	newcolor = m_myCat->color;
	m_ctlColor.SetColor(m_myCat->color == -1 ? m_ctlColor.GetDefaultColor() : m_myCat->color);
	
	SetDlgItemText(IDC_AUTOCATEXT,m_myCat->autocat);

	m_prio.SetCurSel(m_myCat->prio);
	int i = m_temp.FindString(0,m_myCat->strTempPath);// X: [TD] - [TempDir]
	m_temp.SetCurSel((i==-1)?0:i);
}

void CCatDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CATCOLOR, m_ctlColor);
	DDX_Control(pDX, IDC_PRIOCOMBO, m_prio);
	DDX_Control(pDX, IDC_TEMPCOMBO, m_temp);// X: [TD] - [TempDir]
}

void CCatDialog::Localize()
{
	SetDlgItemText(IDC_STATIC_TITLE,GetResString(IDS_TITLE));
	SetDlgItemText(IDC_STATIC_INCOMING,GetResString(IDS_PW_INCOMING) + _T("  ") + GetResString(IDS_SHAREWARNING) );
	SetDlgItemText(IDC_STATIC_TEMP,GetResString(IDS_PW_TEMP));// X: [TD] - [TempDir]
	SetDlgItemText(IDC_STATIC_COMMENT,GetResString(IDS_COMMENT));
	SetDlgItemText(IDCANCEL,GetResString(IDS_CANCEL));
	SetDlgItemText(IDC_STATIC_COLOR,GetResString(IDS_COLOR));
	SetDlgItemText(IDC_STATIC_PRIO,GetResString(IDS_STARTPRIO));
	SetDlgItemText(IDC_STATIC_AUTOCAT,GetResString(IDS_AUTOCAT_LABEL));
	SetDlgItemText(IDC_REGEXPR,GetResString(IDS_ASREGEXPR));
	SetDlgItemText(IDOK,GetResString(IDS_TREEOPTIONS_OK));

	m_ctlColor.SetCustomText(GetResString(IDS_COL_MORECOLORS));
	m_ctlColor.SetDefaultText(GetResString(IDS_DEFAULT));

	SetWindowText(GetResString(IDS_EDITCAT));

	SetDlgItemText(IDC_STATIC_REGEXP,GetResString(IDS_STATIC_REGEXP));

	m_prio.ResetContent();
	m_prio.AddString(GetResString(IDS_PRIOLOW));
	m_prio.AddString(GetResString(IDS_PRIONORMAL));
	m_prio.AddString(GetResString(IDS_PRIOHIGH));
	m_prio.SetCurSel(m_myCat->prio);
}

void CCatDialog::OnBnClickedBrowse()
{	
	TCHAR buffer[MAX_PATH] = {0};
	GetDlgItemText(IDC_INCOMING, buffer, _countof(buffer));
	if (SelectDir(GetSafeHwnd(), buffer,GetResString(IDS_SELECT_INCOMINGDIR)))
		SetDlgItemText(IDC_INCOMING,buffer);
}

void CCatDialog::OnBnClickedOk()
{
	CString oldpath = m_myCat->strIncomingPath;
	if (GetDlgItem(IDC_TITLE)->GetWindowTextLength()>0)
		GetDlgItemText(IDC_TITLE,m_myCat->strTitle);
	
	if (GetDlgItem(IDC_INCOMING)->GetWindowTextLength()>2)
		GetDlgItemText(IDC_INCOMING,m_myCat->strIncomingPath);
	
	GetDlgItemText(IDC_COMMENT,m_myCat->strComment);

	m_myCat->ac_regexpeval= IsDlgButtonChecked(IDC_REGEXPR)>0;

	MakeFoldername(m_myCat->strIncomingPath);

	// SLUGFILLER: SafeHash remove - removed installation dir unsharing
	/*
	if (!thePrefs.IsShareableDirectory(m_myCat->strIncomingPath)){
		m_myCat->strIncomingPath = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
		MakeFoldername(m_myCat->strIncomingPath);
	}
	*/

	if (!PathFileExists(m_myCat->strIncomingPath)){
		if (!::CreateDirectory(m_myCat->strIncomingPath, 0)){
			AfxMessageBox(GetResString(IDS_ERR_BADFOLDER));
			m_myCat->strIncomingPath = oldpath;
			return;
		}
	}

	//if (m_myCat->strIncomingPath.CompareNoCase(oldpath)!=0)// X: [CI] - [Code Improvement]
		//theApp.sharedfiles->Reload(thePrefs.queryOnHashing!=1);// X: [QOH] - [QueryOnHashing]

	m_myCat->color=newcolor;
    m_myCat->prio=m_prio.GetCurSel();
	m_temp.GetWindowText(m_myCat->strTempPath);// X: [TD] - [TempDir]
	GetDlgItemText(IDC_AUTOCATEXT,m_myCat->autocat);

	GetDlgItemText(IDC_REGEXP,m_myCat->regexp);
	if (m_myCat->regexp.GetLength()>0) {
		if (m_pacRegExp && m_pacRegExp->IsBound()){
			m_pacRegExp->AddItem(m_myCat->regexp,0);
			m_myCat->filter=18;
		}
	} else if (m_myCat->filter==18) {
		// deactivate regexp
		m_myCat->filter=0;
	}

	//theApp.emuledlg->transferwnd->GetDownloadList()->Invalidate();// X: [CI] - [Code Improvement]

	OnOK();
}

LRESULT CCatDialog::OnSelChange(WPARAM wParam, LPARAM /*lParam*/)
{
	if (wParam == CLR_DEFAULT)
		newcolor = (DWORD)-1;
	else
		newcolor = m_ctlColor.GetColor();
	return TRUE;
}

void CCatDialog::OnDDBnClicked()
{
	CWnd* box = GetDlgItem(IDC_REGEXP);
	box->SetFocus();
	box->SetWindowText(_T(""));
	box->SendMessage(WM_KEYDOWN, VK_DOWN, 0x00510001);
}
