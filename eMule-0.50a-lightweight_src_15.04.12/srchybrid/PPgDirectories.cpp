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
#include "emuledlg.h"
#include "PPgDirectories.h"
#include "otherfunctions.h"
#include "InputBox.h"
#include "SharedFileList.h"
#include "Preferences.h"
#include "UserMsgs.h"
#include "opcodes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CPPgDirectories, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgDirectories, CPropertyPage)
//	ON_BN_CLICKED(IDC_SELTEMPDIR, OnBnClickedSeltempdir) // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
	ON_BN_CLICKED(IDC_SELINCDIR,OnBnClickedSelincdir)
	ON_EN_CHANGE(IDC_INCFILES,	OnSettingsChange)
//	ON_EN_CHANGE(IDC_TEMPFILES, OnSettingsChange) // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
	// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
	ON_BN_CLICKED(IDC_TEMPADD,	OnBnClickedAddTemp)
	ON_BN_CLICKED(IDC_TEMPREM,	OnBnClickedRemTemp)
	ON_BN_CLICKED(IDC_SHAREADD,	OnBnClickedAddShare)
	ON_BN_CLICKED(IDC_SHAREADDALL,	OnBnClickedAddAllShare)
	ON_BN_CLICKED(IDC_SHAREREM,	OnBnClickedRemShare)
	ON_BN_CLICKED(IDC_DEFAULT_RADIO, OnSettingsChange)// X: [QOH] - [QueryOnHashing]
	ON_BN_CLICKED(IDC_NOHASHING_RADIO, OnSettingsChange)
	ON_BN_CLICKED(IDC_QUERY_RADIO, OnSettingsChange)
	ON_BN_CLICKED(IDC_DONTSHAREXT_LBL, OnSettingsChangeExt)// X: [DSE] - [DontShareExt]
	ON_EN_CHANGE(IDC_DONTSHAREXT, OnSettingsChange)// X: [DSE] - [DontShareExt]
	// NEO: MTD END <-- Xanatos --
//	ON_BN_CLICKED(IDC_SELTEMPDIRADD, OnBnClickedSeltempdiradd)
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
	DDX_Control(pDX, IDC_TEMPLIST, m_ctlTempPaths); // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
	DDX_Control(pDX, IDC_SHARELIST, m_ctlSharePaths);
}

BOOL CPPgDirectories::OnInitDialog()
{
	CWaitCursor curWait; // initialization of that dialog may take a while..
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	((CEdit*)GetDlgItem(IDC_INCFILES))->SetLimitText(MAX_PATH);
	// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
	m_ctlTempPaths.InsertColumn(0, _T("")/*GetResString(IDS_PW_TEMP)*/, LVCFMT_LEFT, 280, -1); 
	m_ctlTempPaths.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	m_ctlSharePaths.InsertColumn(0, _T("")/*GetResString(IDS_PW_SHARED)*/, LVCFMT_LEFT, 280, -1); 
	m_ctlSharePaths.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
	//GetDlgItem(IDC_SELTEMPDIRADD)->ShowWindow(thePrefs.m_bExtControls?SW_SHOW:SW_HIDE);
	// NEO: MTD END <-- Xanatos --

	LoadSettings();
	Localize();
	m_bModified = false; // X: [CI] - [Code Improvement] Apply if modified

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CPPgDirectories::LoadSettings(void)
{
	SetDlgItemText(IDC_INCFILES,thePrefs.m_strIncomingDir);
	CheckDlgButton(IDC_DONTSHAREXT_LBL,thePrefs.dontsharext);// X: [DSE] - [DontShareExt]
	GetDlgItem(IDC_DONTSHAREXT)->EnableWindow(thePrefs.dontsharext);
	SetDlgItemText(IDC_DONTSHAREXT,thePrefs.shareExt);
	CheckDlgButton((thePrefs.queryOnHashing==1)?IDC_NOHASHING_RADIO:((thePrefs.queryOnHashing==2)?IDC_QUERY_RADIO:IDC_DEFAULT_RADIO),true);// X: [QOH] - [QueryOnHashing]

	// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
	for (size_t i=0;i<thePrefs.tempdir.GetCount();i++)
		m_ctlTempPaths.InsertItem(m_ctlTempPaths.GetItemCount(), thePrefs.tempdir.GetAt(i));
	// NEO: MTD END <-- Xanatos --

	POSITION pos = thePrefs.shareddir_list.GetHeadPosition();
	while (pos)	// avoid double sharedirs
		m_ctlSharePaths.InsertItem(m_ctlSharePaths.GetItemCount(), thePrefs.shareddir_list.GetNext(pos));
}

void CPPgDirectories::OnBnClickedSelincdir()
{
	TCHAR buffer[MAX_PATH] = {0};
	GetDlgItemText(IDC_INCFILES, buffer, _countof(buffer));
	if(SelectDir(GetSafeHwnd(),buffer,GetResString(IDS_SELECT_INCOMINGDIR)))
		SetDlgItemText(IDC_INCFILES,buffer);
}
BOOL CPPgDirectories::OnApply()
{
	if(m_bModified){ // X: [CI] - [Code Improvement] Apply if modified
	if(IsDlgButtonChecked(IDC_DEFAULT_RADIO))// X: [QOH] - [QueryOnHashing]
		thePrefs.queryOnHashing=0;
	else if(IsDlgButtonChecked(IDC_NOHASHING_RADIO))
		thePrefs.queryOnHashing=1;
	else if(IsDlgButtonChecked(IDC_QUERY_RADIO))
		thePrefs.queryOnHashing=2;
	thePrefs.dontsharext=IsDlgButtonChecked(IDC_DONTSHAREXT_LBL)!=0;// X: [DSE] - [DontShareExt]
	if(thePrefs.dontsharext){
		GetDlgItemText(IDC_DONTSHAREXT,thePrefs.shareExt);
		thePrefs.shareExt.MakeLower();
		SetDlgItemText(IDC_DONTSHAREXT,thePrefs.shareExt);
	}
	bool testtempdirchanged=false;
	CString testincdirchanged = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);

	CString strIncomingDir;
	GetDlgItemText(IDC_INCFILES, strIncomingDir);
	MakeFoldername(strIncomingDir);
	if (strIncomingDir.IsEmpty()){
		strIncomingDir = thePrefs.GetDefaultDirectory(EMULE_INCOMINGDIR, true); // will create the directory here if it doesnt exists
		SetDlgItemText(IDC_INCFILES, strIncomingDir);
	}
	// SLUGFILLER: SafeHash remove - removed installation dir unsharing
	/*
	else if (thePrefs.IsInstallationDirectory(strIncomingDir)){
		AfxMessageBox(GetResString(IDS_WRN_INCFILE_RESERVED));
		return FALSE;
	}
	*/
	else if (strIncomingDir.CompareNoCase(testincdirchanged) != 0 && strIncomingDir.CompareNoCase(thePrefs.GetDefaultDirectory(EMULE_INCOMINGDIR, false)) != 0){
		// if the user chooses a non-default directory which already contains files, inform him that all those files
		// will be shared
		CFileFind ff;
		CString strSearchPath;
		strSearchPath.Format(_T("%s\\*"),strIncomingDir);
		bool bEnd = !ff.FindFile(strSearchPath, 0);
		bool bExistingFile = false;
		while (!bEnd)
		{
			bEnd = !ff.FindNextFile();
			if (ff.IsDirectory() || ff.IsDots() || ff.IsSystem() || ff.IsTemporary() || ff.GetLength()==0 || ff.GetLength()>MAX_EMULE_FILE_SIZE)
				continue;

			// ignore real LNK files
			TCHAR szExt[_MAX_EXT];
			_tsplitpath_s(ff.GetFileName(), NULL, 0, NULL, 0, NULL, 0, szExt, _countof(szExt));
			if (_tcsicmp(szExt, _T(".lnk")) == 0){
				SHFILEINFO info;
				if (SHGetFileInfo(ff.GetFilePath(), 0, &info, sizeof(info), SHGFI_ATTRIBUTES) && (info.dwAttributes & SFGAO_LINK)){
					if (!thePrefs.GetResolveSharedShellLinks())
						continue;
				}
			}

			// ignore real THUMBS.DB files -- seems that lot of ppl have 'thumbs.db' files without the 'System' file attribute
			if (ff.GetFileName().CompareNoCase(_T("thumbs.db")) == 0)
				continue;

			bExistingFile = true;
			break;
		}
		if (bExistingFile
			&& AfxMessageBox(GetResString(IDS_WRN_INCFILE_EXISTS), MB_OKCANCEL | MB_ICONINFORMATION) == IDCANCEL)
		{
			return FALSE;
		}
	}
	
	// checking specified tempdir(s)
		CAtlArray<CString> temptempfolders;
	// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
	for (int i = 0; i < m_ctlTempPaths.GetItemCount(); i++)
	{
		CString atmp = m_ctlTempPaths.GetItemText(i, 0);
	// NEO: MTD END <-- Xanatos --

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
			for (size_t i=0;i<temptempfolders.GetCount();i++)	// avoid double tempdirs
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

	if (temptempfolders.IsEmpty())
		temptempfolders.Add(thePrefs.GetDefaultDirectory(EMULE_TEMPDIR, true)); // NEO: MTD - [MultiTempDirectories] <-- Xanatos --

	if (temptempfolders.GetCount()!=thePrefs.tempdir.GetCount())
		testtempdirchanged=true;

	// applying tempdirs
	if (testtempdirchanged) {
		thePrefs.tempdir.RemoveAll();
			for (size_t i=0;i<temptempfolders.GetCount();i++) {
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

	//sharedirs
	thePrefs.shareddir_list.RemoveAll();
	for (int i = 0; i < m_ctlSharePaths.GetItemCount(); i++){
		CString atmp = m_ctlSharePaths.GetItemText(i, 0);
		bool doubled=false;
		POSITION pos = thePrefs.shareddir_list.GetHeadPosition();
		while (pos){	// avoid double sharedirs
			const CString& rstrDir = thePrefs.shareddir_list.GetNext(pos);
			if (rstrDir.CompareNoCase(atmp)==0) {
				doubled=true;
				break;
			}
		}
		// remove check shared directories for reserved folder names
		if (!doubled/* && thePrefs.IsShareableDirectory(atmp)*/)// SLUGFILLER: SafeHash remove - removed installation dir unsharing

			thePrefs.shareddir_list.AddHead(atmp);
	}

	
	// on changing incoming dir, update incoming dirs of category of the same path
	if (testincdirchanged.CompareNoCase(thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR)) != 0) {
			thePrefs.GetCategory(0)->strIncomingPath = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
		CString oldpath;
		bool dontaskagain=false;
			for (size_t cat=1; cat<=thePrefs.GetCatCount()-1;cat++){
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
			thePrefs.SaveCats();
	}


	// X-Ray :: AutoRestartIfNecessary :: Start
	/*
		if (testtempdirchanged)
			AfxMessageBox(GetResString(IDS_SETTINGCHANGED_RESTART));
	*/
	if (testtempdirchanged && AfxMessageBox(GetResString(IDS_RESTARTEMULEONCHANGE), MB_YESNO | MB_ICONEXCLAMATION, 0) == IDYES)
		theApp.emuledlg->RestartMuleApp();
	// X-Ray :: AutoRestartIfNecessary :: End

	theApp.sharedfiles->Reload(thePrefs.queryOnHashing!=1);// X: [QOH] - [QueryOnHashing]
	
		SetModified(FALSE);
		m_bModified = false; // X: [CI] - [Code Improvement] Apply if modified
	}
	return CPropertyPage::OnApply();
}

BOOL CPPgDirectories::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == UM_ITEMSTATECHANGED)
		OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified
	return CPropertyPage::OnCommand(wParam, lParam);
}

void CPPgDirectories::Localize(void)
{
	if(m_hWnd)
	{
		SetWindowText(GetResString(IDS_PW_DIR));

		SetDlgItemText(IDC_INCOMING_FRM,GetResString(IDS_PW_INCOMING));
//		SetDlgItemText(IDC_SELINCDIR,GetResString(IDS_PW_BROWSE));
//		SetDlgItemText(IDC_SELTEMPDIR,GetResString(IDS_PW_BROWSE)); // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
//		SetDlgItemText(IDC_SHARED_FRM,GetResString(IDS_PW_SHARED));
		SetDlgItemText(IDC_DONTSHAREXT_LBL,GetResString(IDS_DONTSHAREXT));// X: [DSE] - [DontShareExt]
		SetDlgItemText(IDC_QUERYONHASHING_STATIC,GetResString(IDS_ACTIONBE4ASHING));// X: [QOH] - [QueryOnHashing]
		SetDlgItemText(IDC_DEFAULT_RADIO,GetResString(IDS_DEFAULT));
		SetDlgItemText(IDC_NOHASHING_RADIO,GetResString(IDS_NOHASHING));
		SetDlgItemText(IDC_QUERY_RADIO,GetResString(IDS_QUERY));

		HDITEM hdi;// X: [AL] - [Additional Localize]
		hdi.mask = HDI_TEXT;
		CString strRes;
		strRes=GetResString(IDS_PW_TEMP);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		m_ctlTempPaths.GetHeaderCtrl()->SetItem(0, &hdi);
		strRes=GetResString(IDS_PW_SHARED);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		m_ctlSharePaths.GetHeaderCtrl()->SetItem(0, &hdi);
		//m_ctlTempPaths.SetDlgItemText(0,GetResString(IDS_PW_TEMP));
		//m_ctlSharePaths.SetDlgItemText(0,GetResString(IDS_PW_SHARED));

	}
}

void CPPgDirectories::OnBnClickedAddTemp()
{
	TCHAR buffer[MAX_PATH] = {0};

	if(SelectDir(GetSafeHwnd(),buffer,GetResString(IDS_SELECT_TEMPDIR))) {
		m_ctlTempPaths.InsertItem(m_ctlTempPaths.GetItemCount(), buffer);
		OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified
	}
}

void CPPgDirectories::OnBnClickedRemTemp()
{
	int index = m_ctlTempPaths.GetSelectionMark();
	if (index == -1 || m_ctlTempPaths.GetSelectedCount() == 0)
		return;
	m_ctlTempPaths.DeleteItem(index);
	OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified
}

void CPPgDirectories::OnBnClickedAddShare()
{
	TCHAR buffer[MAX_PATH] = {0};

	if(SelectDir(GetSafeHwnd(),buffer,GetResString(IDS_SELECT_SHAREDIR))) {
		size_t len = _tcslen(buffer);
		
		if (buffer[len-1] != _T('\\')){
			buffer[len] = _T('\\');
			buffer[len+1] = 0;
		}
		m_ctlSharePaths.InsertItem(m_ctlSharePaths.GetItemCount(),buffer);
		OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified
	}
}

void CPPgDirectories::OnBnClickedAddAllShare()
{
	TCHAR buffer[MAX_PATH] = {0};

	if(SelectDir(GetSafeHwnd(),buffer,GetResString(IDS_SELECT_SHAREDIR_SUB))) {
		size_t len = _tcslen(buffer);
		
		if (buffer[len-1] != _T('\\')){
			buffer[len] = _T('\\');
			buffer[len+1] = 0;
		}
		FindAllDir(buffer);
		OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified
	}
}

void CPPgDirectories::OnBnClickedRemShare()
{
	int index = m_ctlSharePaths.GetSelectionMark();
	if (index == -1 || m_ctlSharePaths.GetSelectedCount() == 0)
		return;
	m_ctlSharePaths.DeleteItem(index);
	OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified
}

void CPPgDirectories::FindAllDir(CString strParent, size_t level){
	if(level > 16)
		return;
	CFileFind f;
	BOOL bFind = f.FindFile(strParent + _T("*.*"));
	CString strDir;
	while(bFind){
		bFind = f.FindNextFile();  
		if(f.IsDirectory() && (!f.IsDots())){
			strDir = strParent + f.GetFileName() + _T('\\');
			m_ctlSharePaths.InsertItem(m_ctlSharePaths.GetItemCount(),strDir);
			FindAllDir(strDir, level + 1);  
		}
	}  
	f.Close();  
}

void CPPgDirectories::OnSettingsChangeExt()// X: [DSE] - [DontShareExt]
{
	OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified
	GetDlgItem(IDC_DONTSHAREXT)->EnableWindow(IsDlgButtonChecked(IDC_DONTSHAREXT_LBL));
}
