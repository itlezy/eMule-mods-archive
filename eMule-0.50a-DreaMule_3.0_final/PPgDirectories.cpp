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
#include "emuledlg.h"
#include "SharedFilesWnd.h"
#include "PPgDirectories.h"
#include "otherfunctions.h"
#include "InputBox.h"
#include "SharedFileList.h"
#include "Preferences.h"
// Tux: LiteMule: Remove Help
#include "UserMsgs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CPPgDirectories, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgDirectories, CPropertyPage)
	ON_BN_CLICKED(IDC_SELTEMPDIR, OnBnClickedSeltempdir)
	ON_BN_CLICKED(IDC_SELINCDIR,OnBnClickedSelincdir)
	ON_EN_CHANGE(IDC_INCFILES,	OnSettingsChange)
	ON_EN_CHANGE(IDC_TEMPFILES, OnSettingsChange)
	ON_BN_CLICKED(IDC_UNCADD,	OnBnClickedAddUNC)
	ON_BN_CLICKED(IDC_UNCREM,	OnBnClickedRemUNC)
	// Tux: LiteMule: Remove Help
	ON_BN_CLICKED(IDC_SELTEMPDIRADD, OnBnClickedSeltempdiradd)
END_MESSAGE_MAP()

CPPgDirectories::CPPgDirectories()
	: CPropertyPage(CPPgDirectories::IDD)
{
}

CPPgDirectories::~CPPgDirectories()
{
}

void CPPgDirectories::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SHARESELECTOR, m_ShareSelector);
	DDX_Control(pDX, IDC_UNCLIST, m_ctlUncPaths);
}

BOOL CPPgDirectories::OnInitDialog()
{
	CWaitCursor curWait; // initialization of that dialog may take a while..
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);
	// SLUGFILLER START: shareSubdir remove - removed double init
	/*
	m_ShareSelector.Init();	
   SLUGFILLER END: shareSubdir  */


	((CEdit*)GetDlgItem(IDC_INCFILES))->SetLimitText(509);
	((CEdit*)GetDlgItem(IDC_TEMPFILES))->SetLimitText(509);
	m_ctlUncPaths.InsertColumn(0, GetResString(IDS_UNCFOLDERS), LVCFMT_LEFT, 280, -1); 
	m_ctlUncPaths.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	GetDlgItem(IDC_SELTEMPDIRADD)->ShowWindow(thePrefs.IsExtControlsEnabled()?SW_SHOW:SW_HIDE);

	LoadSettings();
	Localize();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPPgDirectories::LoadSettings(void)
{
	GetDlgItem(IDC_INCFILES)->SetWindowText(thePrefs.m_strIncomingDir);

	CString tempfolders;
	for (int i=0;i<thePrefs.tempdir.GetCount();i++) {
		tempfolders.Append(thePrefs.GetTempDir(i));
		if (i+1<thePrefs.tempdir.GetCount())
			tempfolders.Append(_T("|") );
	}

	GetDlgItem(IDC_TEMPFILES)->SetWindowText(tempfolders);
/* sharesubdir
	m_ShareSelector.SetSharedDirectories(&thePrefs.shareddir_list);
*/
	m_ShareSelector.SetSharedDirectories(&thePrefs.shareddir_list, &thePrefs.sharedsubdir_list);	// SLUGFILLER: shareSubdir
	FillUncList();
}

void CPPgDirectories::OnBnClickedSelincdir()
{
	TCHAR buffer[MAX_PATH] = {0};
	GetDlgItemText(IDC_INCFILES, buffer, _countof(buffer));
	if(SelectDir(GetSafeHwnd(),buffer,GetResString(IDS_SELECT_INCOMINGDIR)))
		GetDlgItem(IDC_INCFILES)->SetWindowText(buffer);
}

void CPPgDirectories::OnBnClickedSeltempdir()
{
	TCHAR buffer[MAX_PATH] = {0};
	GetDlgItemText(IDC_TEMPFILES, buffer, _countof(buffer));
	if(SelectDir(GetSafeHwnd(),buffer,GetResString(IDS_SELECT_TEMPDIR)))
		GetDlgItem(IDC_TEMPFILES)->SetWindowText(buffer);
}

// SLUGFILLER START: shareSubdir - don't double-share UNC
static bool FindStringNoCase(const CStringList &list, CString string) {
	if (string.Right(1) != '\\')
		string += '\\';
	for (POSITION pos = list.GetHeadPosition(); pos != NULL; )
	{
		CString str = list.GetNext(pos);
		if (str.Right(1) != '\\')
			str += '\\';
		if (str.CompareNoCase(string) == 0)
			return true;
	}
	return false;
}
// SLUGFILLER END: shareSubdir

BOOL CPPgDirectories::OnApply()
{
	bool testtempdirchanged=false;
	CString testincdirchanged = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);

	CString strIncomingDir;
	GetDlgItemText(IDC_INCFILES, strIncomingDir);
	if (strIncomingDir.IsEmpty()){
		strIncomingDir = thePrefs.GetDefaultDirectory(EMULE_INCOMINGDIR, true); // will create the directory here if it doesnt exists
		SetDlgItemText(IDC_INCFILES, strIncomingDir);
	}
	// SLUGFILLER: SafeHash remove - removed installation dir unsharing
	/*
	if (thePrefs.IsInstallationDirectory(strIncomingDir)){
		AfxMessageBox(GetResString(IDS_WRN_INCFILE_RESERVED));
		return FALSE;
	}
	*/
	// checking specified tempdir(s)
	CString strTempDir;
	GetDlgItemText(IDC_TEMPFILES, strTempDir);
	if (strTempDir.IsEmpty()){
		strTempDir = thePrefs.GetDefaultDirectory(EMULE_TEMPDIR, true); // will create the directory here if it doesnt exists
		SetDlgItemText(IDC_TEMPFILES, strTempDir);
	}

	int curPos=0;
	CStringArray temptempfolders;
	CString atmp=strTempDir.Tokenize(_T("|"), curPos);
	while (!atmp.IsEmpty())
	{
		atmp.Trim();
		if (!atmp.IsEmpty()) {
			if (CompareDirectories(strIncomingDir, atmp)==0){
					AfxMessageBox(GetResString(IDS_WRN_INCTEMP_SAME));
					return FALSE;
			}	
			// SLUGFILLER: SafeHash remove - removed installation dir unsharing
			/*
			if (thePrefs.IsInstallationDirectory(atmp)){
				AfxMessageBox(GetResString(IDS_WRN_TEMPFILES_RESERVED));
				return FALSE;
			}
			*/
			bool doubled=false;
			for (int i=0;i<temptempfolders.GetCount();i++)	// avoid double tempdirs
				if (temptempfolders.GetAt(i).CompareNoCase(atmp)==0) {
					doubled=true;
					break;
				}
			if (!doubled) {
				temptempfolders.Add(atmp);
				if (thePrefs.tempdir.GetCount()>=temptempfolders.GetCount()) {
					if( atmp.CompareNoCase(thePrefs.GetTempDir(temptempfolders.GetCount()-1))!=0	)
						testtempdirchanged=true;
				} else testtempdirchanged=true;

			}
		}
		atmp = strTempDir.Tokenize(_T("|"), curPos);
	}

	if (temptempfolders.IsEmpty())
		temptempfolders.Add(strTempDir = thePrefs.GetDefaultDirectory(EMULE_TEMPDIR, true));

	if (temptempfolders.GetCount()!=thePrefs.tempdir.GetCount())
		testtempdirchanged=true;

	// applying tempdirs
	if (testtempdirchanged) {
		thePrefs.tempdir.RemoveAll();
		for (int i=0;i<temptempfolders.GetCount();i++) {
			CString toadd=temptempfolders.GetAt(i);
			MakeFoldername(toadd);
			if (!PathFileExists(toadd))
				CreateDirectory(toadd,NULL);
			if (PathFileExists(toadd))
				thePrefs.tempdir.Add(toadd);
		}
	}
	if (thePrefs.tempdir.IsEmpty())
		thePrefs.tempdir.Add(thePrefs.GetDefaultDirectory(EMULE_TEMPDIR, true));

	thePrefs.m_strIncomingDir = strIncomingDir;
	MakeFoldername(thePrefs.m_strIncomingDir);
	thePrefs.GetCategory(0)->strIncomingPath = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);

	// SLUGFILLER START: shareSubdir
	m_ShareSelector.GetSharedDirectories(&thePrefs.shareddir_list, &thePrefs.sharedsubdir_list);
	thePrefs.inactive_shareddir_list.RemoveAll();
	thePrefs.inactive_sharedsubdir_list.RemoveAll();
    for (int i = 0; i < m_ctlUncPaths.GetItemCount(); i++){
		CString unc = m_ctlUncPaths.GetItemText(i, 0);
		bool    sharesub= (m_ctlUncPaths.GetItemText(i, 1).Compare(_T("+")) ==0);
		if (!PathFileExists(unc))	// only add directories which still exist
		{   if  (sharesub) // maintain inactive dir list
				thePrefs.inactive_sharedsubdir_list.AddTail(unc);
		    else
		 		thePrefs.inactive_shareddir_list.AddTail(unc);
			continue;
		}
		if (FindStringNoCase(thePrefs.shareddir_list, unc))	// don't double-share UNC
			continue; // log?
		// TODO: Could be indirectly shared via subdir. We should probably check for that too.
		if  (sharesub) // maintain inactive dir list
			thePrefs.sharedsubdir_list.AddTail(unc);
		else
			thePrefs.shareddir_list.AddTail(unc);
	}

	FillUncList();
	// SLUGFILLER END: shareSubdir

/* old Code sharesubdir 
	thePrefs.shareddir_list.RemoveAll();
	m_ShareSelector.GetSharedDirectories(&thePrefs.shareddir_list);
	for (int i = 0; i < m_ctlUncPaths.GetItemCount(); i++)
		thePrefs.shareddir_list.AddTail(m_ctlUncPaths.GetItemText(i, 0));
   end sharesubdir old code*/
	// check shared directories for reserved folder names
	POSITION pos = thePrefs.shareddir_list.GetHeadPosition();
	while (pos){
		POSITION posLast = pos;
		const CString& rstrDir = thePrefs.shareddir_list.GetNext(pos);
		if (!thePrefs.IsShareableDirectory(rstrDir))
			thePrefs.shareddir_list.RemoveAt(posLast);
	}

	if (testtempdirchanged)
		AfxMessageBox(GetResString(IDS_SETTINGCHANGED_RESTART));
	
	// on changing incoming dir, update incoming dirs of category of the same path
	if (testincdirchanged.CompareNoCase(thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR)) != 0) {
		CString oldpath;
		bool dontaskagain=false;
		for (int cat=1; cat<=thePrefs.GetCatCount()-1;cat++){
			oldpath=CString(thePrefs.GetCatPath(cat));
			if (oldpath.Left(testincdirchanged.GetLength()).CompareNoCase(testincdirchanged)==0) {

				if (!dontaskagain) {
					dontaskagain=true;
					if (AfxMessageBox(GetResString(IDS_UPDATECATINCOMINGDIRS),MB_YESNO)==IDNO)
						break;
				}
				thePrefs.GetCategory(cat)->strIncomingPath = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR) + oldpath.Mid(testincdirchanged.GetLength());
			}
		}
	}

	theApp.emuledlg->sharedfileswnd->Reload();
	
	SetModified(0);
	return CPropertyPage::OnApply();
}

BOOL CPPgDirectories::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == UM_ITEMSTATECHANGED)
		SetModified();	
	// Tux: LiteMule: Remove Help: removed code block
	return CPropertyPage::OnCommand(wParam, lParam);
}

void CPPgDirectories::Localize(void)
{
	if(m_hWnd)
	{
		SetWindowText(GetResString(IDS_PW_DIR));

		GetDlgItem(IDC_INCOMING_FRM)->SetWindowText(GetResString(IDS_PW_INCOMING));
		GetDlgItem(IDC_TEMP_FRM)->SetWindowText(GetResString(IDS_PW_TEMP));
		GetDlgItem(IDC_SELINCDIR)->SetWindowText(GetResString(IDS_PW_BROWSE));
		GetDlgItem(IDC_SELTEMPDIR)->SetWindowText(GetResString(IDS_PW_BROWSE));
		GetDlgItem(IDC_SHARED_FRM)->SetWindowText(GetResString(IDS_PW_SHARED));
	}
}

void CPPgDirectories::FillUncList(void)
{
	m_ctlUncPaths.DeleteAllItems();
	// SLUGFILLER START: shareSubdir remove - don't refill list, use it only for adding
/* old code
	for (POSITION pos = thePrefs.shareddir_list.GetHeadPosition(); pos != 0; )
	{
		CString folder = thePrefs.shareddir_list.GetNext(pos);
		if (PathIsUNC(folder))
			m_ctlUncPaths.InsertItem(0, folder);
	}
 end old code */
	//inactive sharelist	 HIER WAS IK
	for (POSITION pos = thePrefs.inactive_shareddir_list.GetHeadPosition(); pos != 0; )
	{
		CString folder = thePrefs.inactive_shareddir_list.GetNext(pos);
		{
		  int nIndex = m_ctlUncPaths.InsertItem(0,folder);
          m_ctlUncPaths.SetItemText(nIndex,1,L" ");
		}
	}
	for (POSITION pos = thePrefs.inactive_sharedsubdir_list.GetHeadPosition(); pos != 0; )
	{
		CString folder = thePrefs.inactive_sharedsubdir_list.GetNext(pos);
    	int nIndex = m_ctlUncPaths.InsertItem(0,folder);
         m_ctlUncPaths.SetItemText(nIndex,1,L"+");
	}

}


IMPLEMENT_DYNAMIC(CAddSharedDirDialog, CDialog)

CAddSharedDirDialog::CAddSharedDirDialog(LPTSTR sUnc,bool bSubdir,CWnd* pParent /*=NULL*/)
	: CDialog(CAddSharedDirDialog::IDD, pParent)
{
	m_sUnc=sUnc;
	m_bSubdir=bSubdir;
} // ssd

BOOL CAddSharedDirDialog::OnInitDialog(){
	CDialog::OnInitDialog();
	InitWindowStyles(this);
	SetWindowText(GetResString(IDS_ADDSHAREDIR));

	CheckDlgButton(IDC_SHAREWITHSUBDIR, m_bSubdir ? BST_CHECKED : BST_UNCHECKED);

	GetDlgItem(IDC_TEXTSHRETOADD)->SetWindowText(GetResString(IDS_TEXTSHRETOADD));
	GetDlgItem(IDC_SHAREWITHSUBDIR)->SetWindowText(GetResString(IDS_SHAREWITHSUBDIR));
	GetDlgItem(IDC_INPUTTEXT)->SetWindowText(m_sUnc);


	GetDlgItem(IDCANCEL)->SetWindowText(GetResString(IDS_CANCEL));

	return TRUE;
}


BEGIN_MESSAGE_MAP(CAddSharedDirDialog, CDialog)
END_MESSAGE_MAP()

void CAddSharedDirDialog::OnOK()
{	
	GetDlgItem(IDC_INPUTTEXT)->GetWindowText ( m_sUnc);
	m_bSubdir = IsDlgButtonChecked(IDC_SHAREWITHSUBDIR)!=0;

	CDialog::OnOK();
} // ssd

void CPPgDirectories::OnBnClickedAddUNC()
{

   /* old code, replace by dialog that contains "include subdir"  checkbox
	InputBox inputbox;
	inputbox.SetLabels(GetResString(IDS_UNCFOLDERS), GetResString(IDS_UNCFOLDERS), _T("\\\\Server\\Share"));
	if (inputbox.DoModal() != IDOK)
		return;
	CString unc=inputbox.GetInput();

	// basic unc-check 
	if (!PathIsUNC(unc)){
		AfxMessageBox(GetResString(IDS_ERR_BADUNC), MB_ICONERROR);
		return;
	}
	 end old code */
  // MORPH START SHARESUBDIR 
	CAddSharedDirDialog AddSharedDirDialog(_T("\\\\Server\\Share"),false);
	int result = AddSharedDirDialog.DoModal();
		if (result != IDOK) 
			return;
	CString unc=AddSharedDirDialog.GetUNC();
	bool   bsharesubdir=AddSharedDirDialog.GetSubDir();
  // MORPH END SHARESUBDIR
	if (unc.Right(1) == _T("\\"))
		unc.Delete(unc.GetLength()-1, 1);

// MORPH START SHARESUBDIR 
	if (bsharesubdir) {
		for (POSITION pos = thePrefs.sharedsubdir_list.GetHeadPosition();pos != 0;){
			if (unc.CompareNoCase(thePrefs.sharedsubdir_list.GetNext(pos))==0)
				//	 message that it is already shared? 
				return;
			}
		}
	else {
  // MORPH END SHARESUBDIR 
	for (POSITION pos = thePrefs.shareddir_list.GetHeadPosition();pos != 0;){
		if (unc.CompareNoCase(thePrefs.shareddir_list.GetNext(pos))==0)
			return;
	}
	} // ssd
	for (int posi = 0; posi < m_ctlUncPaths.GetItemCount(); posi++){
		if (unc.CompareNoCase(m_ctlUncPaths.GetItemText(posi, 0)) == 0)
			return;
	}

   /* old code:
	m_ctlUncPaths.InsertItem(m_ctlUncPaths.GetItemCount(), unc);
   end old code */ 
  // MOROPH START sharesubdir
	int nIndex=m_ctlUncPaths.InsertItem(m_ctlUncPaths.GetItemCount(), unc);
	if (bsharesubdir) 
	    m_ctlUncPaths.SetItemText(nIndex,1,L"+");
	else
		m_ctlUncPaths.SetItemText(nIndex,1,L" ");
// MOROPH END sharesubdir
	SetModified();
}

void CPPgDirectories::OnBnClickedRemUNC()
{
	int index = m_ctlUncPaths.GetSelectionMark();
	if (index == -1 || m_ctlUncPaths.GetSelectedCount() == 0)
		return;
	m_ctlUncPaths.DeleteItem(index);
	SetModified();
}

// Tux: LiteMule: Remove Help: removed code block

void CPPgDirectories::OnBnClickedSeltempdiradd()
{
	CString paths;
	GetDlgItemText(IDC_TEMPFILES, paths);

	TCHAR buffer[MAX_PATH] = {0};
	//GetDlgItemText(IDC_TEMPFILES, buffer, _countof(buffer));

	if(SelectDir(GetSafeHwnd(),buffer,GetResString(IDS_SELECT_TEMPDIR))) {
		paths.Append(_T("|"));
		paths.Append(buffer);
		SetDlgItemText(IDC_TEMPFILES, paths);
	}
}
