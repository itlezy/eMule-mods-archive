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
#include "SharedFilesWnd.h"
#include "PPgDirectories.h"
#include "otherfunctions.h"
#include "InputBox.h"
#include "SharedFileList.h"
#include "Preferences.h"
#include "HelpIDs.h"
#include "UserMsgs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CPPgDirectories, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgDirectories, CPropertyPage)
	//ON_BN_CLICKED(IDC_SELTEMPDIR, OnBnClickedSeltempdir) // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
	ON_BN_CLICKED(IDC_SELINCDIR,OnBnClickedSelincdir)
	ON_EN_CHANGE(IDC_INCFILES,	OnSettingsChange)
	//ON_EN_CHANGE(IDC_TEMPFILES, OnSettingsChange) // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
	// NEO: SSD - [ShareSubDirectories] -- Xanatos --
	//ON_BN_CLICKED(IDC_UNCADD,	OnBnClickedAddUNC)
	//ON_BN_CLICKED(IDC_UNCREM,	OnBnClickedRemUNC)
	ON_WM_HELPINFO()
	//ON_BN_CLICKED(IDC_SELTEMPDIRADD, OnBnClickedSeltempdiradd) 
	// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
	ON_BN_CLICKED(IDC_TEMPADD,	OnBnClickedAddTemp)
	ON_BN_CLICKED(IDC_TEMPREM,	OnBnClickedRemTemp)
	// NEO: MTD END <-- Xanatos --
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
	//DDX_Control(pDX, IDC_UNCLIST, m_ctlUncPaths); // NEO: SSD - [ShareSubDirectories] <-- Xanatos --
	DDX_Control(pDX, IDC_TEMPLIST, m_ctlTempPaths); // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
}

BOOL CPPgDirectories::OnInitDialog()
{
	CWaitCursor curWait; // initialization of that dialog may take a while..
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	((CEdit*)GetDlgItem(IDC_INCFILES))->SetLimitText(509);
	//((CEdit*)GetDlgItem(IDC_TEMPFILES))->SetLimitText(509); // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
	// NEO: SSD - [ShareSubDirectories] -- Xanatos --
	//m_ctlUncPaths.InsertColumn(0, GetResString(IDS_UNCFOLDERS), LVCFMT_LEFT, 280, -1); 
	//m_ctlUncPaths.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
	m_ctlTempPaths.InsertColumn(0, GetResString(IDS_PW_TEMP), LVCFMT_LEFT, 280, -1); 
	m_ctlTempPaths.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
	// NEO: MTD END <-- Xanatos --

	//GetDlgItem(IDC_SELTEMPDIRADD)->ShowWindow(thePrefs.IsExtControlsEnabled()?SW_SHOW:SW_HIDE); // NEO: MTD - [MultiTempDirectories] <-- Xanatos --

	LoadSettings();
	Localize();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CPPgDirectories::LoadSettings(void)
{
	GetDlgItem(IDC_INCFILES)->SetWindowText(thePrefs.m_strIncomingDir);

	//CString tempfolders;
	//for (int i=0;i<thePrefs.tempdir.GetCount();i++) {
	//	tempfolders.Append(thePrefs.GetTempDir(i));
	//	if (i+1<thePrefs.tempdir.GetCount())
	//		tempfolders.Append(_T("|") );
	//}
	//GetDlgItem(IDC_TEMPFILES)->SetWindowText(tempfolders); 

	// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
	for (int i=0;i<thePrefs.tempdir.GetCount();i++)
		m_ctlTempPaths.InsertItem(m_ctlTempPaths.GetItemCount(), thePrefs.GetTempDir(i));
	// NEO: MTD END <-- Xanatos --

	//m_ShareSelector.SetSharedDirectories(&thePrefs.shareddir_list);
	m_ShareSelector.SetSharedDirectories(&thePrefs.shareddir_list, &thePrefs.sharedsubdir_list); // NEO: SSD - [ShareSubDirectories] <-- Xanatos --
	//FillUncList();
}

void CPPgDirectories::OnBnClickedSelincdir()
{
	TCHAR buffer[MAX_PATH] = {0};
	GetDlgItemText(IDC_INCFILES, buffer, _countof(buffer));
	if(SelectDir(GetSafeHwnd(),buffer,GetResString(IDS_SELECT_INCOMINGDIR)))
		GetDlgItem(IDC_INCFILES)->SetWindowText(buffer);
}

// NEO: MTD - [MultiTempDirectories] -- Xanatos --
/*void CPPgDirectories::OnBnClickedSeltempdir()
{
	TCHAR buffer[MAX_PATH] = {0};
	GetDlgItemText(IDC_TEMPFILES, buffer, _countof(buffer));
	if(SelectDir(GetSafeHwnd(),buffer,GetResString(IDS_SELECT_TEMPDIR)))
		GetDlgItem(IDC_TEMPFILES)->SetWindowText(buffer);
}*/

BOOL CPPgDirectories::OnApply()
{
	bool testtempdirchanged=false;
	CString testincdirchanged = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);

	CString strIncomingDir;
	GetDlgItemText(IDC_INCFILES, strIncomingDir);
	MakeFoldername(strIncomingDir);
	if (strIncomingDir.IsEmpty()){
		strIncomingDir = thePrefs.GetDefaultDirectory(EMULE_INCOMINGDIR, true); // will create the directory here if it doesnt exists
		SetDlgItemText(IDC_INCFILES, strIncomingDir);
	}
	else if (thePrefs.IsInstallationDirectory(strIncomingDir)){
		AfxMessageBox(GetResString(IDS_WRN_INCFILE_RESERVED));
		return FALSE;
	}
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
			if (ff.IsDirectory() || ff.IsDots() || ff.IsSystem() || ff.IsTemporary() || ff.GetLength()==0)
				continue;

			// ignore real(!) LNK files
			TCHAR szExt[_MAX_EXT];
			_tsplitpath(ff.GetFileName(), NULL, NULL, NULL, szExt);
			if (_tcsicmp(szExt, _T(".lnk")) == 0){
				SHFILEINFO info;
				if (SHGetFileInfo(ff.GetFilePath(), 0, &info, sizeof(info), SHGFI_ATTRIBUTES) && (info.dwAttributes & SFGAO_LINK)){
					CComPtr<IShellLink> pShellLink;
					if (SUCCEEDED(pShellLink.CoCreateInstance(CLSID_ShellLink))){
						CComQIPtr<IPersistFile> pPersistFile = pShellLink;
						if (pPersistFile){
							USES_CONVERSION;
							if (SUCCEEDED(pPersistFile->Load(T2COLE(ff.GetFilePath()), STGM_READ))){
								TCHAR szResolvedPath[MAX_PATH];
								if (pShellLink->GetPath(szResolvedPath, ARRSIZE(szResolvedPath), NULL, 0) == NOERROR){
									TRACE(_T("%hs: Did not share file \"%s\" - not supported file type\n"), __FUNCTION__, ff.GetFilePath());
									continue;
								}
							}
						}
					}
				}
			}
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
	// NEO: MTD - [MultiTempDirectories] -- Xanatos --
	//CString strTempDir;
	//GetDlgItemText(IDC_TEMPFILES, strTempDir);
	//if (strTempDir.IsEmpty()){
	//	strTempDir = thePrefs.GetDefaultDirectory(EMULE_TEMPDIR, true); // will create the directory here if it doesnt exists
	//	SetDlgItemText(IDC_TEMPFILES, strTempDir);
	//}

	//int curPos=0;
	CStringArray temptempfolders;
	//CString atmp=strTempDir.Tokenize(_T("|"), curPos);
	//while (!atmp.IsEmpty())
	//{
	//	atmp.Trim();
	//	if (!atmp.IsEmpty()) {
		// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
		for (int i = 0; i < m_ctlTempPaths.GetItemCount(); i++)
		{
			CString atmp = m_ctlTempPaths.GetItemText(i, 0);
		// NEO: MTD END <-- Xanatos --
			if (CompareDirectories(strIncomingDir, atmp)==0){
					AfxMessageBox(GetResString(IDS_WRN_INCTEMP_SAME));
					return FALSE;
			}	
			if (thePrefs.IsInstallationDirectory(atmp)){
				AfxMessageBox(GetResString(IDS_WRN_TEMPFILES_RESERVED));
				return FALSE;
			}
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
	//	atmp = strTempDir.Tokenize(_T("|"), curPos);
	//}

	if (temptempfolders.IsEmpty())
		temptempfolders.Add(thePrefs.GetDefaultDirectory(EMULE_TEMPDIR, true)); // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
		//temptempfolders.Add(strTempDir = thePrefs.GetDefaultDirectory(EMULE_TEMPDIR, true));

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
	if(!thePrefs.GetCategory(0)->strIncomingPath.IsEmpty())// NEO: NXC - [NewExtendedCategories] <-- Xanatos --
	thePrefs.GetCategory(0)->strIncomingPath = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);

	thePrefs.shareddir_list.RemoveAll();
	//m_ShareSelector.GetSharedDirectories(&thePrefs.shareddir_list); // NEO: SSD - [ShareSubDirectories] <-- Xanatos --
	m_ShareSelector.GetSharedDirectories(&thePrefs.shareddir_list, &thePrefs.sharedsubdir_list);
	//for (int i = 0; i < m_ctlUncPaths.GetItemCount(); i++)
	//	thePrefs.shareddir_list.AddTail(m_ctlUncPaths.GetItemText(i, 0));

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
		//for (int cat=1; cat<=thePrefs.GetCatCount()-1;cat++){
		for (int cat=1; cat<=thePrefs.GetFullCatCount()-1;cat++){ // NEO: NSC - [NeoSharedCategories] <-- Xanatos --
			oldpath=CString(thePrefs.GetCatPath(cat));
			if (oldpath.Left(testincdirchanged.GetLength()).CompareNoCase(testincdirchanged)==0) {

				if (!dontaskagain) {
					dontaskagain=true;
					if (AfxMessageBox(GetResString(IDS_UPDATECATINCOMINGDIRS),MB_YESNO)==IDNO)
						break;
				}
				thePrefs.GetCategory(cat)->strIncomingPath = _T(""); // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
				//thePrefs.GetCategory(cat)->strIncomingPath = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR) + oldpath.Mid(testincdirchanged.GetLength());
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
	else if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return CPropertyPage::OnCommand(wParam, lParam);
}

void CPPgDirectories::Localize(void)
{
	if(m_hWnd)
	{
		SetWindowText(GetResString(IDS_PW_DIR));

		GetDlgItem(IDC_INCOMING_FRM)->SetWindowText(GetResString(IDS_PW_INCOMING));
		//GetDlgItem(IDC_TEMP_FRM)->SetWindowText(GetResString(IDS_PW_TEMP)); // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
		GetDlgItem(IDC_SELINCDIR)->SetWindowText(GetResString(IDS_PW_BROWSE));
		//GetDlgItem(IDC_SELTEMPDIR)->SetWindowText(GetResString(IDS_PW_BROWSE)); // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
		GetDlgItem(IDC_SHARED_FRM)->SetWindowText(GetResString(IDS_PW_SHARED));
	}
}

// NEO: SSD - [ShareSubDirectories] -- Xanatos --
/*void CPPgDirectories::FillUncList(void)
{
	m_ctlUncPaths.DeleteAllItems();

	for (POSITION pos = thePrefs.shareddir_list.GetHeadPosition(); pos != 0; )
	{
		CString folder = thePrefs.shareddir_list.GetNext(pos);
		if (PathIsUNC(folder))
			m_ctlUncPaths.InsertItem(0, folder);
	}
}

void CPPgDirectories::OnBnClickedAddUNC()
{
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

	if (unc.Right(1) == _T("\\"))
		unc.Delete(unc.GetLength()-1, 1);

	for (POSITION pos = thePrefs.shareddir_list.GetHeadPosition();pos != 0;){
		if (unc.CompareNoCase(thePrefs.shareddir_list.GetNext(pos))==0)
			return;
	}
	for (int posi = 0; posi < m_ctlUncPaths.GetItemCount(); posi++){
		if (unc.CompareNoCase(m_ctlUncPaths.GetItemText(posi, 0)) == 0)
			return;
	}

	m_ctlUncPaths.InsertItem(m_ctlUncPaths.GetItemCount(), unc);
	SetModified();
}

void CPPgDirectories::OnBnClickedRemUNC()
{
	int index = m_ctlUncPaths.GetSelectionMark();
	if (index == -1 || m_ctlUncPaths.GetSelectedCount() == 0)
		return;
	m_ctlUncPaths.DeleteItem(index);
	SetModified();
}*/

// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
void CPPgDirectories::OnBnClickedAddTemp()
{
	TCHAR buffer[MAX_PATH] = {0};

	if(SelectDir(GetSafeHwnd(),buffer,GetResString(IDS_SELECT_TEMPDIR))) {
		m_ctlTempPaths.InsertItem(m_ctlTempPaths.GetItemCount(), buffer);
		SetModified();
	}
}

void CPPgDirectories::OnBnClickedRemTemp()
{
	int index = m_ctlTempPaths.GetSelectionMark();
	if (index == -1 || m_ctlTempPaths.GetSelectedCount() == 0)
		return;
	m_ctlTempPaths.DeleteItem(index);
	SetModified();
}
// NEO: MTD END <-- Xanatos --

void CPPgDirectories::OnHelp()
{
	theApp.ShowHelp(eMule_FAQ_Preferences_Directories);
}

BOOL CPPgDirectories::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}

// NEO: MTD - [MultiTempDirectories] -- Xanatos --
/*void CPPgDirectories::OnBnClickedSeltempdiradd()
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
}*/
