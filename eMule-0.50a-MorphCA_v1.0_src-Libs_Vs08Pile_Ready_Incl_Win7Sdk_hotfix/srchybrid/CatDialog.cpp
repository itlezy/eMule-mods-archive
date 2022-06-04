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
//#include "CustomAutoComplete.h" // morph
#include "Preferences.h"
#include "otherfunctions.h"
#include "SharedFileList.h"
#include "emuledlg.h"
#include "TransferDlg.h"
#include "CatDialog.h"
#include "UserMsgs.h"
#include "SharedFilesWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif
/* Morph delete: no regexp
#define	REGULAREXPRESSIONS_STRINGS_PROFILE	_T("AC_VF_RegExpr.dat")
emd ,ooprm*/

// CCatDialog dialog

IMPLEMENT_DYNAMIC(CCatDialog, CDialog)

BEGIN_MESSAGE_MAP(CCatDialog, CDialog)
	ON_BN_CLICKED(IDC_BROWSE, OnBnClickedBrowse)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_MESSAGE(UM_CPN_SELENDOK, OnSelChange) //UM_CPN_SELCHANGE
END_MESSAGE_MAP()

CCatDialog::CCatDialog(int index)
	: CDialog(CCatDialog::IDD, 0)
{
	m_myCat=thePrefs.GetCategory(index);
	if (m_myCat==NULL) return;
/* morph delete: no regexp
	if (m_myCat == NULL)
		return;
	m_pacRegExp=NULL;
end morph */
	newcolor = (DWORD)-1;
}

CCatDialog::~CCatDialog()
{
/* morph no regexp
	if (m_pacRegExp){
		m_pacRegExp->SaveList(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + REGULAREXPRESSIONS_STRINGS_PROFILE);
		m_pacRegExp->Unbind();
		m_pacRegExp->Release();
	}
*/ //end morph
}

BOOL CCatDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	InitWindowStyles(this);
	SetIcon(theApp.LoadIcon(_T("CATEGORY"),16,16),FALSE);
	Localize();
	m_ctlColor.SetDefaultColor(GetSysColor(COLOR_BTNTEXT));
	UpdateData();

/* morph no regexp
	if (!thePrefs.IsExtControlsEnabled()) {
		GetDlgItem(IDC_REGEXPR)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_REGEXP)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_REGEXP)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_REB)->ShowWindow(SW_HIDE);
	}

	m_pacRegExp = new CCustomAutoComplete();
	m_pacRegExp->AddRef();
	if (m_pacRegExp->Bind(::GetDlgItem(m_hWnd, IDC_REGEXP), ACO_UPDOWNKEYDROPSLIST | ACO_AUTOSUGGEST)) {
		m_pacRegExp->LoadList(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + REGULAREXPRESSIONS_STRINGS_PROFILE);
	}
	if (theApp.m_fontSymbol.m_hObject){
		GetDlgItem(IDC_REB)->SetFont(&theApp.m_fontSymbol);
		GetDlgItem(IDC_REB)->SetWindowText(_T("6")); // show a down-arrow
	}
 */ //end morph
	return true;
}

void CCatDialog::UpdateData()
{
	GetDlgItem(IDC_TITLE)->SetWindowText(m_myCat->strTitle);
	GetDlgItem(IDC_INCOMING)->SetWindowText(m_myCat->strIncomingPath);
	GetDlgItem(IDC_COMMENT)->SetWindowText(m_myCat->strComment);
  /* morph: no regexp
	if (m_myCat->filter==18)
		SetDlgItemText(IDC_REGEXP,m_myCat->regexp);

	CheckDlgButton(IDC_REGEXPR,m_myCat->ac_regexpeval);
 end morph */

	newcolor = m_myCat->color;
	m_ctlColor.SetColor(m_myCat->color == -1 ? m_ctlColor.GetDefaultColor() : m_myCat->color);

	// HoaX_69: AutoCat
	//MORPH START - Changed by SiRoB, Due to Khaos Categorie
	/*
	GetDlgItem(IDC_AUTOCATEXT)->SetWindowText(m_myCat->autocat);
	*/
	GetDlgItem(IDC_AUTOCATEXT)->SetWindowText(m_myCat->viewfilters.sAdvancedFilterMask);
	//MORPH END   - Changed by SiRoB, Due to Khaos Categorie

	m_prio.SetCurSel(m_myCat->prio);

	// khaos::kmod+ Category Advanced A4AF Mode and Auto Cat
	if (m_comboA4AF.IsWindowEnabled())
	m_comboA4AF.SetCurSel(m_myCat->iAdvA4AFMode);
	// khaos::kmod-

	// khaos::categorymod+ Update the data for the filter options.
	CString buffer;
	GetDlgItem(IDC_FS_MIN)->SetWindowText(CastItoUIXBytes(m_myCat->viewfilters.nFSizeMin));
	GetDlgItem(IDC_FS_MAX)->SetWindowText(CastItoUIXBytes(m_myCat->viewfilters.nFSizeMax));
	GetDlgItem(IDC_RS_MIN)->SetWindowText(CastItoUIXBytes(m_myCat->viewfilters.nRSizeMin));
	GetDlgItem(IDC_RS_MAX)->SetWindowText(CastItoUIXBytes(m_myCat->viewfilters.nRSizeMax));
	buffer.Format(_T("%u"), m_myCat->viewfilters.nTimeRemainingMin >= 60 ? (m_myCat->viewfilters.nTimeRemainingMin / 60) : 0);
	GetDlgItem(IDC_RT_MIN)->SetWindowText(buffer);
	buffer.Format(_T("%u"), m_myCat->viewfilters.nTimeRemainingMax >= 60 ? (m_myCat->viewfilters.nTimeRemainingMax / 60) : 0);
	GetDlgItem(IDC_RT_MAX)->SetWindowText(buffer);
	buffer.Format(_T("%u"), m_myCat->viewfilters.nSourceCountMin);
	GetDlgItem(IDC_SC_MIN)->SetWindowText(buffer);
	buffer.Format(_T("%u"), m_myCat->viewfilters.nSourceCountMax);
	GetDlgItem(IDC_SC_MAX)->SetWindowText(buffer);
	buffer.Format(_T("%u"), m_myCat->viewfilters.nAvailSourceCountMin);
	GetDlgItem(IDC_ASC_MIN)->SetWindowText(buffer);
	buffer.Format(_T("%u"), m_myCat->viewfilters.nAvailSourceCountMax);
	GetDlgItem(IDC_ASC_MAX)->SetWindowText(buffer);

	CheckDlgButton(IDC_CHECK_FS, m_myCat->selectioncriteria.bFileSize?1:0);
	CheckDlgButton(IDC_CHECK_MASK, m_myCat->selectioncriteria.bAdvancedFilterMask?1:0);

	CheckDlgButton(IDC_CHECK_RESUMEFILEONLYINSAMECAT, m_myCat->bResumeFileOnlyInSameCat?1:0); //MORPH - Added by SiRoB, Resume file only in the same category
	// khaos::categorymod-
}

void CCatDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CATCOLOR, m_ctlColor);
	DDX_Control(pDX, IDC_PRIOCOMBO, m_prio);
	// khaos::kmod+ Category Advanced A4AF Mode
	DDX_Control(pDX, IDC_COMBO_A4AF, m_comboA4AF);
	// khaos::kmod-
}

void CCatDialog::Localize()
{
	GetDlgItem(IDC_STATIC_TITLE)->SetWindowText(GetResString(IDS_TITLE));
	GetDlgItem(IDC_STATIC_INCOMING)->SetWindowText(GetResString(IDS_PW_INCOMING) + _T("  ") + GetResString(IDS_SHAREWARNING) );
	GetDlgItem(IDC_STATIC_COMMENT)->SetWindowText(GetResString(IDS_COMMENT));
	GetDlgItem(IDCANCEL)->SetWindowText(GetResString(IDS_CANCEL));
	GetDlgItem(IDC_STATIC_COLOR)->SetWindowText(GetResString(IDS_COLOR));
	GetDlgItem(IDC_STATIC_PRIO)->SetWindowText(GetResString(IDS_STARTPRIO));
/* MOrph remove
	GetDlgItem(IDC_STATIC_AUTOCAT)->SetWindowText(GetResString(IDS_AUTOCAT_LABEL));
	GetDlgItem(IDC_REGEXPR)->SetWindowText(GetResString(IDS_ASREGEXPR));
   end morph remove*/

	// khaos::kmod+ Category Advanced A4AF Mode and Auto Cat
	GetDlgItem(IDC_STATIC_A4AF)->SetWindowText(GetResString(IDS_A4AF_ADVMODE));

	m_comboA4AF.EnableWindow(true);
	while (m_comboA4AF.GetCount()>0) m_comboA4AF.DeleteString(0);
	if (thePrefs.AdvancedA4AFMode())
	{
		m_comboA4AF.AddString(GetResString(IDS_DEFAULT));
		m_comboA4AF.AddString(GetResString(IDS_A4AF_BALANCE));
		m_comboA4AF.AddString(GetResString(IDS_A4AF_STACK));
		m_comboA4AF.SetCurSel(m_myCat->iAdvA4AFMode);
	}
	else
	{
		m_comboA4AF.AddString(GetResString(IDS_DISABLED));
		m_comboA4AF.SetCurSel(0);
		m_comboA4AF.EnableWindow(false);
	}
	// khaos::kmod-

	// khaos::categorymod+
	GetDlgItem(IDC_CHECK_MASK)->SetWindowText(GetResString(IDS_CAT_AUTOCAT));
	GetDlgItem(IDC_STATIC_MIN)->SetWindowText(GetResString(IDS_CAT_MINIMUM));
	GetDlgItem(IDC_STATIC_MAX)->SetWindowText(GetResString(IDS_CAT_MAXIMUM));
	GetDlgItem(IDC_STATIC_MIN2)->SetWindowText(GetResString(IDS_CAT_MINIMUM));
	GetDlgItem(IDC_STATIC_MAX2)->SetWindowText(GetResString(IDS_CAT_MAXIMUM));
	GetDlgItem(IDC_STATIC_MIN3)->SetWindowText(GetResString(IDS_CAT_MINIMUM));
	GetDlgItem(IDC_STATIC_MAX3)->SetWindowText(GetResString(IDS_CAT_MAXIMUM));
	GetDlgItem(IDC_STATIC_MIN4)->SetWindowText(GetResString(IDS_CAT_MINIMUM));
	GetDlgItem(IDC_STATIC_MAX4)->SetWindowText(GetResString(IDS_CAT_MAXIMUM));
	GetDlgItem(IDC_STATIC_MIN5)->SetWindowText(GetResString(IDS_CAT_MINIMUM));
	GetDlgItem(IDC_STATIC_MAX5)->SetWindowText(GetResString(IDS_CAT_MAXIMUM));
	GetDlgItem(IDC_STATIC_EXP)->SetWindowText(GetResString(IDS_CAT_DLGHELP));
	GetDlgItem(IDC_STATIC_FSIZE)->SetWindowText(GetResString(IDS_CAT_FS));
	GetDlgItem(IDC_STATIC_RSIZE)->SetWindowText(GetResString(IDS_CAT_RS));
	GetDlgItem(IDC_STATIC_RTIME)->SetWindowText(GetResString(IDS_CAT_RT));
	GetDlgItem(IDC_STATIC_SCOUNT)->SetWindowText(GetResString(IDS_CAT_SC));
	GetDlgItem(IDC_STATIC_ASCOUNT)->SetWindowText(GetResString(IDS_CAT_ASC));
	// khaos::categorymod-
    GetDlgItem(IDC_FILTERGROUP)->SetWindowText(GetResString(IDS_CAT_FILTERS));

	GetDlgItem(IDC_CHECK_RESUMEFILEONLYINSAMECAT)->SetWindowText(GetResString(IDS_CAT_RESUMEFILEONLYINSAMECAT)); //MORPH - Added by SiRoB, Resume file only in the same category
	GetDlgItem(IDOK)->SetWindowText(GetResString(IDS_TREEOPTIONS_OK));

	m_ctlColor.CustomText = GetResString(IDS_COL_MORECOLORS);
	m_ctlColor.DefaultText = GetResString(IDS_DEFAULT);

	SetWindowText(GetResString(IDS_EDITCAT));

/* morph delete
	SetDlgItemText(IDC_STATIC_REGEXP,GetResString(IDS_STATIC_REGEXP));	

	m_prio.ResetContent();
	end morph delete*/

	while (m_prio.GetCount()>0) m_prio.DeleteString(0);
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
		GetDlgItem(IDC_INCOMING)->SetWindowText(buffer);
}

void CCatDialog::OnBnClickedOk()
{
	CString oldpath = m_myCat->strIncomingPath;
	if (GetDlgItem(IDC_TITLE)->GetWindowTextLength()>0)
		GetDlgItem(IDC_TITLE)->GetWindowText(m_myCat->strTitle);
	
	if (GetDlgItem(IDC_INCOMING)->GetWindowTextLength()>2)
		GetDlgItem(IDC_INCOMING)->GetWindowText(m_myCat->strIncomingPath);
	
	GetDlgItem(IDC_COMMENT)->GetWindowText(m_myCat->strComment);

/* Morph delete
	m_myCat->ac_regexpeval= IsDlgButtonChecked(IDC_REGEXPR)>0;
  end morph delete */

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

	if (m_myCat->strIncomingPath.CompareNoCase(oldpath)!=0)
		theApp.emuledlg->sharedfileswnd->Reload();

	m_myCat->color=newcolor;
	m_myCat->prio=m_prio.GetCurSel();
	//MORPH STRAT - Changed by SiRoB, Due to Khoas categorie
	/*
	GetDlgItem(IDC_AUTOCATEXT)->GetWindowText(m_myCat->autocat);
	*/
	GetDlgItem(IDC_AUTOCATEXT)->GetWindowText(m_myCat->viewfilters.sAdvancedFilterMask);
	//MORPH END   - Changed by SiRoB, Due to Khoas categorie

	// khaos::kmod+ Category Advanced A4AF Mode and Auto Cat
	m_myCat->iAdvA4AFMode = m_comboA4AF.GetCurSel();
	// khaos::kmod-

	// khaos::categorymod+	
	CString sBuffer;
	
	GetDlgItem(IDC_FS_MIN)->GetWindowText(sBuffer);
	m_myCat->viewfilters.nFSizeMin = CastXBytesToI(sBuffer);
	GetDlgItem(IDC_FS_MAX)->GetWindowText(sBuffer);
	m_myCat->viewfilters.nFSizeMax = CastXBytesToI(sBuffer);
	GetDlgItem(IDC_RS_MIN)->GetWindowText(sBuffer);
	m_myCat->viewfilters.nRSizeMin = CastXBytesToI(sBuffer);
	GetDlgItem(IDC_RS_MAX)->GetWindowText(sBuffer);
	m_myCat->viewfilters.nRSizeMax = CastXBytesToI(sBuffer);
	GetDlgItem(IDC_RT_MIN)->GetWindowText(sBuffer);
	m_myCat->viewfilters.nTimeRemainingMin = (uint32) (60 * _tstoi(sBuffer));
	GetDlgItem(IDC_RT_MAX)->GetWindowText(sBuffer);
	m_myCat->viewfilters.nTimeRemainingMax = (uint32) (60 * _tstoi(sBuffer));
	GetDlgItem(IDC_SC_MIN)->GetWindowText(sBuffer);
	m_myCat->viewfilters.nSourceCountMin = _tstoi(sBuffer);
	GetDlgItem(IDC_SC_MAX)->GetWindowText(sBuffer);
	m_myCat->viewfilters.nSourceCountMax = _tstoi(sBuffer);
	GetDlgItem(IDC_ASC_MIN)->GetWindowText(sBuffer);
	m_myCat->viewfilters.nAvailSourceCountMin = _tstoi(sBuffer);
	GetDlgItem(IDC_ASC_MAX)->GetWindowText(sBuffer);
	m_myCat->viewfilters.nAvailSourceCountMax = _tstoi(sBuffer);

	m_myCat->selectioncriteria.bFileSize = IsDlgButtonChecked(IDC_CHECK_FS)?true:false;
	m_myCat->selectioncriteria.bAdvancedFilterMask = IsDlgButtonChecked(IDC_CHECK_MASK)?true:false;	
	m_myCat->bResumeFileOnlyInSameCat = IsDlgButtonChecked(IDC_CHECK_RESUMEFILEONLYINSAMECAT)?true:false;	//MORPH - Added by SiRoB, Resume file only in the same category
	// khaos::categorymod-
/* morph delete
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
   end morph delete */

	theApp.emuledlg->transferwnd->GetDownloadList()->Invalidate();

	OnOK();
}

LONG CCatDialog::OnSelChange(UINT lParam, LONG /*wParam*/)
{
	if (lParam == CLR_DEFAULT)
		newcolor = (DWORD)-1;
	else
		newcolor = m_ctlColor.GetColor();
	return TRUE;
}

/* morph delete
void CCatDialog::OnDDBnClicked()
{
	CWnd* box = GetDlgItem(IDC_REGEXP);
	box->SetFocus();
	box->SetWindowText(_T(""));
	box->SendMessage(WM_KEYDOWN, VK_DOWN, 0x00510001);
}
end morph delete */
