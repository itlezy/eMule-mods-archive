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
//#include "CustomAutoComplete.h"
#include "Neo/Functions.h" // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
#include "Preferences.h"
#include "otherfunctions.h"
#include "SharedFileList.h"
#include "emuledlg.h"
#include "TransferWnd.h"
#include "CatDialog.h"
#include "UserMsgs.h"
#include "Neo/NeoPreferences.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#define	REGULAREXPRESSIONS_STRINGS_PROFILE	_T("AC_VF_RegExpr.dat") // NEO: NXC - [NewExtendedCategories] <-- Xanatos --

// CCatDialog dialog

IMPLEMENT_DYNAMIC(CCatDialog, CDialog)

BEGIN_MESSAGE_MAP(CCatDialog, CDialog)
	ON_BN_CLICKED(IDC_BROWSE, OnBnClickedBrowse)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	//ON_BN_CLICKED(IDC_REB, OnDDBnClicked) // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
	ON_MESSAGE(UM_CPN_SELENDOK, OnSelChange) //UM_CPN_SELCHANGE
END_MESSAGE_MAP()

CCatDialog::CCatDialog(int index)
	: CDialog(CCatDialog::IDD)
{
	m_myCat = thePrefs.GetCategory(index);
	if (m_myCat == NULL)
		return;
	//m_pacRegExp=NULL; // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
	newcolor = (DWORD)-1;
}

CCatDialog::~CCatDialog()
{
	// NEO: NXC - [NewExtendedCategories] -- Xanatos --
	//if (m_pacRegExp){
	//	m_pacRegExp->SaveList(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + REGULAREXPRESSIONS_STRINGS_PROFILE);
	//	m_pacRegExp->Unbind();
	//	m_pacRegExp->Release();
	//}
}

BOOL CCatDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	InitWindowStyles(this);
	SetIcon(theApp.LoadIcon(_T("CATEGORY"),16,16),FALSE); // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
	Localize();
	m_ctlColor.SetDefaultColor(GetSysColor(COLOR_BTNTEXT));
	UpdateData();

	// NEO: NXC - [NewExtendedCategories] -- Xanatos --
	/*if (!thePrefs.IsExtControlsEnabled()) {
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
	}*/

	return TRUE;
}

void CCatDialog::UpdateData()
{
	GetDlgItem(IDC_TITLE)->SetWindowText(m_myCat->strTitle);
	GetDlgItem(IDC_INCOMING)->SetWindowText(m_myCat->strIncomingPath);
	// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
	m_temp.ResetContent();
	m_temp.AddString(_T(""));
	for (int i = 0; i < thePrefs.GetTempDirCount(); i++){
		int index = m_temp.AddString(thePrefs.GetTempDir(i));
		if (!CompareDirectories(m_myCat->strTempPath, thePrefs.GetTempDir(i)))
			m_temp.SetCurSel(index);
	}
	//m_temp.SetWindowText(m_myCat->strTempPath);
	// NEO: MTD END <-- Xanatos --
	GetDlgItem(IDC_COMMENT)->SetWindowText(m_myCat->strComment);

	// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
	GetDlgItem(IDC_AUTOCATEXT)->SetWindowText(m_myCat->viewfilters.sAdvancedFilterMask);

	m_prio.SetCurSel(m_myCat->prio);

	m_upprio.SetCurSel(m_myCat->boost);
	CheckDlgButton(IDC_CHECK_RELEASE, m_myCat->release?1:0);  // NEO: SRS - [SmartReleaseSharing]

	if (m_comboA4AF.IsWindowEnabled())
	m_comboA4AF.SetCurSel(m_myCat->iAdvA4AFMode);

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

	CheckDlgButton(IDC_CHECK_RESUMEFILEONLYINSAMECAT, m_myCat->bResumeFileOnlyInSameCat?1:0); 
	// NEO: NXC END <-- Xanatos --

	//if (m_myCat->filter==18)
	//	SetDlgItemText(IDC_REGEXP,m_myCat->regexp);

	//CheckDlgButton(IDC_REGEXPR,m_myCat->ac_regexpeval);

	newcolor = m_myCat->color;
	m_ctlColor.SetColor(m_myCat->color == -1 ? m_ctlColor.GetDefaultColor() : m_myCat->color);
	
	// NEO: NXC - [NewExtendedCategories] -- Xanatos --
	//GetDlgItem(IDC_AUTOCATEXT)->SetWindowText(m_myCat->autocat);

	//m_prio.SetCurSel(m_myCat->prio);
}

void CCatDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_TEMP, m_temp); // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
	DDX_Control(pDX, IDC_CATCOLOR, m_ctlColor);
	DDX_Control(pDX, IDC_PRIOCOMBO, m_prio);
	// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
	DDX_Control(pDX, IDC_UPPRIOCOMBO, m_upprio);
	DDX_Control(pDX, IDC_COMBO_A4AF, m_comboA4AF);
	// NEO: NXC END <-- Xanatos --
}

void CCatDialog::Localize()
{
	GetDlgItem(IDC_STATIC_TITLE)->SetWindowText(GetResString(IDS_TITLE));
	GetDlgItem(IDC_STATIC_INCOMING)->SetWindowText(GetResString(IDS_PW_INCOMING) + _T("  ") + GetResString(IDS_SHAREWARNING) );
	GetDlgItem(IDC_STATIC_TEMP)->SetWindowText(GetResString(IDS_PW_TEMP)); // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
	GetDlgItem(IDC_STATIC_COMMENT)->SetWindowText(GetResString(IDS_COMMENT));
	GetDlgItem(IDCANCEL)->SetWindowText(GetResString(IDS_CANCEL));
	GetDlgItem(IDC_STATIC_COLOR)->SetWindowText(GetResString(IDS_COLOR));
	GetDlgItem(IDC_STATIC_PRIO)->SetWindowText(GetResString(IDS_STARTPRIO));
	//GetDlgItem(IDC_STATIC_AUTOCAT)->SetWindowText(GetResString(IDS_AUTOCAT_LABEL));
	//GetDlgItem(IDC_REGEXPR)->SetWindowText(GetResString(IDS_ASREGEXPR));

	// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
	GetDlgItem(IDC_STATIC_A4AF)->SetWindowText(GetResString(IDS_X_A4AF_ADVMODE));

	GetDlgItem(IDC_STATIC_UL_PRIO)->SetWindowText(GetResString(IDS_X_STATIC_UL_PRIO));
	GetDlgItem(IDC_STATIC_RELEASE)->SetWindowText(GetResString(IDS_X_STATIC_RELEASE));

	GetDlgItem(IDC_CHECK_RELEASE)->SetWindowText(GetResString(IDS_X_CHECK_RELEASE));

	m_comboA4AF.EnableWindow(true);
	while (m_comboA4AF.GetCount()>0) m_comboA4AF.DeleteString(0);
	if (NeoPrefs.AdvancedA4AFMode())
	{
		m_comboA4AF.AddString(GetResString(IDS_DEFAULT));
		m_comboA4AF.AddString(GetResString(IDS_X_A4AF_BALANCE));
		m_comboA4AF.AddString(GetResString(IDS_X_A4AF_STACK));
		m_comboA4AF.SetCurSel(m_myCat->iAdvA4AFMode);
	}
	else
	{
		m_comboA4AF.AddString(GetResString(IDS_DISABLED));
		m_comboA4AF.SetCurSel(0);
		m_comboA4AF.EnableWindow(false);
	}

	GetDlgItem(IDC_CHECK_MASK)->SetWindowText(GetResString(IDS_X_CAT_AUTOCAT));
	GetDlgItem(IDC_STATIC_MIN)->SetWindowText(GetResString(IDS_X_CAT_MINIMUM));
	GetDlgItem(IDC_STATIC_MAX)->SetWindowText(GetResString(IDS_X_CAT_MAXIMUM));
	GetDlgItem(IDC_STATIC_MIN2)->SetWindowText(GetResString(IDS_X_CAT_MINIMUM));
	GetDlgItem(IDC_STATIC_MAX2)->SetWindowText(GetResString(IDS_X_CAT_MAXIMUM));
	GetDlgItem(IDC_STATIC_MIN3)->SetWindowText(GetResString(IDS_X_CAT_MINIMUM));
	GetDlgItem(IDC_STATIC_MAX3)->SetWindowText(GetResString(IDS_X_CAT_MAXIMUM));
	GetDlgItem(IDC_STATIC_MIN4)->SetWindowText(GetResString(IDS_X_CAT_MINIMUM));
	GetDlgItem(IDC_STATIC_MAX4)->SetWindowText(GetResString(IDS_X_CAT_MAXIMUM));
	GetDlgItem(IDC_STATIC_MIN5)->SetWindowText(GetResString(IDS_X_CAT_MINIMUM));
	GetDlgItem(IDC_STATIC_MAX5)->SetWindowText(GetResString(IDS_X_CAT_MAXIMUM));
	GetDlgItem(IDC_STATIC_EXP)->SetWindowText(GetResString(IDS_X_CAT_DLGHELP));
	GetDlgItem(IDC_STATIC_FSIZE)->SetWindowText(GetResString(IDS_X_CAT_FS));
	GetDlgItem(IDC_STATIC_RSIZE)->SetWindowText(GetResString(IDS_X_CAT_RS));
	GetDlgItem(IDC_STATIC_RTIME)->SetWindowText(GetResString(IDS_X_CAT_RT));
	GetDlgItem(IDC_STATIC_SCOUNT)->SetWindowText(GetResString(IDS_X_CAT_SC));
	GetDlgItem(IDC_STATIC_ASCOUNT)->SetWindowText(GetResString(IDS_X_CAT_ASC));

	GetDlgItem(IDOK)->SetWindowText(GetResString(IDS_TREEOPTIONS_OK));

	GetDlgItem(IDC_CHECK_RESUMEFILEONLYINSAMECAT)->SetWindowText(GetResString(IDS_X_CAT_RESUMEFILEONLYINSAMECAT));
	// NEO: NXC END <-- Xanatos --


	m_ctlColor.CustomText = GetResString(IDS_COL_MORECOLORS);
	m_ctlColor.DefaultText = GetResString(IDS_DEFAULT);

	SetWindowText(GetResString(IDS_EDITCAT));

	//SetDlgItemText(IDC_STATIC_REGEXP,GetResString(IDS_STATIC_REGEXP)); // NEO: NXC - [NewExtendedCategories] <-- Xanatos --

	m_prio.ResetContent();
	m_prio.AddString(GetResString(IDS_PRIOLOW));
	m_prio.AddString(GetResString(IDS_PRIONORMAL));
	m_prio.AddString(GetResString(IDS_PRIOHIGH));
	m_prio.SetCurSel(m_myCat->prio);

	// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
	m_upprio.ResetContent();
	m_upprio.AddString(GetResString(IDS_PRIOLOW));
	m_upprio.AddString(GetResString(IDS_PRIONORMAL));
	m_upprio.AddString(GetResString(IDS_PRIOHIGH));
	m_upprio.SetCurSel(m_myCat->boost);
	// NEO: NXC END <-- Xanatos --
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
	
	//if (GetDlgItem(IDC_INCOMING)->GetWindowTextLength()>2) // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
	GetDlgItem(IDC_INCOMING)->GetWindowText(m_myCat->strIncomingPath);
	
	GetDlgItem(IDC_COMMENT)->GetWindowText(m_myCat->strComment);

	//m_myCat->ac_regexpeval= IsDlgButtonChecked(IDC_REGEXPR)>0; // NEO: NXC - [NewExtendedCategories] <-- Xanatos --

	if(!m_myCat->strIncomingPath.IsEmpty()) // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
		MakeFoldername(m_myCat->strIncomingPath);
	// NEO: NXC - [NewExtendedCategories] -- Xanatos --
	//if (!thePrefs.IsShareableDirectory(m_myCat->strIncomingPath)){
	//	m_myCat->strIncomingPath = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
	//	MakeFoldername(m_myCat->strIncomingPath);
	//}

	// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
	m_temp.GetWindowText(m_myCat->strTempPath);
	if(!m_myCat->strTempPath.IsEmpty())
		MakeFoldername(m_myCat->strTempPath);
	// NEO: MTD END <-- Xanatos --

	//if (!PathFileExists(m_myCat->strIncomingPath)){
	if (!m_myCat->strIncomingPath.IsEmpty() && !PathFileExists(m_myCat->strIncomingPath)){ // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
		if (!::CreateDirectory(m_myCat->strIncomingPath, 0)){
			AfxMessageBox(GetResString(IDS_ERR_BADFOLDER));
			m_myCat->strIncomingPath = oldpath;
			return;
		}
	}

	if (m_myCat->strIncomingPath.CompareNoCase(oldpath)!=0)
		theApp.sharedfiles->Reload();

	m_myCat->color=newcolor;
    m_myCat->prio=m_prio.GetCurSel();

	// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
	m_myCat->boost=m_upprio.GetCurSel();
	m_myCat->release= IsDlgButtonChecked(IDC_CHECK_RELEASE)>0; // NEO: SRS - [SmartReleaseSharing]

	GetDlgItem(IDC_AUTOCATEXT)->GetWindowText(m_myCat->viewfilters.sAdvancedFilterMask);

	m_myCat->iAdvA4AFMode = m_comboA4AF.GetCurSel();

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

	m_myCat->selectioncriteria.bFileSize = I2B(IsDlgButtonChecked(IDC_CHECK_FS));
	m_myCat->selectioncriteria.bAdvancedFilterMask = I2B(IsDlgButtonChecked(IDC_CHECK_MASK));	
	m_myCat->bResumeFileOnlyInSameCat = I2B(IsDlgButtonChecked(IDC_CHECK_RESUMEFILEONLYINSAMECAT));
	// NEO: NXC END <-- Xanatos --

	//GetDlgItem(IDC_AUTOCATEXT)->GetWindowText(m_myCat->autocat);

	//GetDlgItemText(IDC_REGEXP,m_myCat->regexp);
	//if (m_myCat->regexp.GetLength()>0) {
	//	if (m_pacRegExp && m_pacRegExp->IsBound()){
	//		m_pacRegExp->AddItem(m_myCat->regexp,0);
	//		m_myCat->filter=18;
	//	}
	//} else if (m_myCat->filter==18) {
	//	// deactivate regexp
	//	m_myCat->filter=0;
	//}

	theApp.emuledlg->transferwnd->downloadlistctrl.Invalidate();

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

// NEO: NXC - [NewExtendedCategories] -- Xanatos --
//void CCatDialog::OnDDBnClicked()
//{
//	CWnd* box = GetDlgItem(IDC_REGEXP);
//	box->SetFocus();
//	box->SetWindowText(_T(""));
//	box->SendMessage(WM_KEYDOWN, VK_DOWN, 0x00510001);
//}
