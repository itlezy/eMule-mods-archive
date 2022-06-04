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
#include "PPgFiles.h"
#include "Inputbox.h"
#include "OtherFunctions.h"
#include "TransferWnd.h"
#include "emuledlg.h"
#include "Preferences.h"
#include "SharedFileList.h"
#include "DownloadQueue.h"// X: [DCE] - [DontCompressExt]

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CPPgFiles, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgFiles, CPropertyPage)
	ON_BN_CLICKED(IDC_PF_TIMECALC, OnSettingsChange)
	ON_BN_CLICKED(IDC_UAP, OnSettingsChange)
	ON_BN_CLICKED(IDC_DAP, OnSettingsChange)
	ON_BN_CLICKED(IDC_PREVIEWPRIO, OnSettingsChange)
	ON_BN_CLICKED(IDC_ADDNEWFILESPAUSED, OnSettingsChange)
	ON_BN_CLICKED(IDC_FULLCHUNKTRANS, OnSettingsChange)
	ON_BN_CLICKED(IDC_STARTNEXTFILE, OnSettingsChangeSNF)
	ON_BN_CLICKED(IDC_WATCHCB, OnSettingsChange)
	ON_BN_CLICKED(IDC_STARTNEXTFILECAT, OnSettingsChangeCat1)
	ON_BN_CLICKED(IDC_STARTNEXTFILECAT2, OnSettingsChangeCat2)
	ON_BN_CLICKED(IDC_FNCLEANUP, OnSettingsChange)
	ON_BN_CLICKED(IDC_FNC, OnSetCleanupFilter)
	ON_EN_CHANGE(IDC_VIDEOPLAYER, OnSettingsChange)
	ON_BN_CLICKED(IDC_REMEMBERDOWNLOADED, OnBnClickedRememberdownloaded) //Xman remove unused AICH-hashes
	ON_BN_CLICKED(IDC_REMEMBERAICH, OnSettingsChange) //Xman remove unused AICH-hashes
	ON_BN_CLICKED(IDC_PAUSEONCOMP, OnSettingsChange) // NEO: POFC - [PauseOnFileComplete]
	ON_BN_CLICKED(IDC_REMEMBERCANCELLED, OnSettingsChange) 
	ON_BN_CLICKED(IDC_BROWSEV, BrowseVideoplayer)
	ON_BN_CLICKED(IDC_DONTCOMPRESSEXT_LBL, OnSettingsChangeExt)// X: [DCE] - [DontCompressExt]
	ON_EN_CHANGE(IDC_DONTCOMPRESSEXT, OnSettingsChange)// X: [DCE] - [DontCompressExt]
END_MESSAGE_MAP()

CPPgFiles::CPPgFiles()
	: CPropertyPage(CPPgFiles::IDD)
{
}

CPPgFiles::~CPPgFiles()
{
}

BOOL CPPgFiles::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	LoadSettings();
	Localize();
	m_bModified = false; // X: [CI] - [Code Improvement] Apply if modified

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CPPgFiles::LoadSettings(void)
{
	CheckDlgButton(IDC_ADDNEWFILESPAUSED,thePrefs.addnewfilespaused);
	CheckDlgButton(IDC_PF_TIMECALC,!thePrefs.m_bUseOldTimeRemaining);
	CheckDlgButton(IDC_PREVIEWPRIO,thePrefs.m_bpreviewprio);
	CheckDlgButton(IDC_DAP,thePrefs.m_bDAP);
	CheckDlgButton(IDC_UAP,thePrefs.m_bUAP);
	CheckDlgButton(IDC_FULLCHUNKTRANS,thePrefs.m_btransferfullchunks);

	CheckDlgButton( IDC_STARTNEXTFILECAT, FALSE);
	CheckDlgButton( IDC_STARTNEXTFILECAT2, FALSE);

	CheckDlgButton(IDC_PAUSEONCOMP,thePrefs.m_bPauseOnFileComplete); // NEO: POFC - [PauseOnFileComplete]

	if (thePrefs.m_istartnextfile)
	{
		CheckDlgButton(IDC_STARTNEXTFILE,1);
		if (thePrefs.m_istartnextfile == 2)
			CheckDlgButton( IDC_STARTNEXTFILECAT, TRUE);
		else if (thePrefs.m_istartnextfile == 3)
			CheckDlgButton( IDC_STARTNEXTFILECAT2, TRUE);
	}
	else
		CheckDlgButton(IDC_STARTNEXTFILE,0);

	SetDlgItemText(IDC_VIDEOPLAYER,thePrefs.m_strVideoPlayer);

	CheckDlgButton(IDC_FNCLEANUP, (uint8)thePrefs.AutoFilenameCleanup());
	CheckDlgButton(IDC_WATCHCB,thePrefs.watchclipboard);
	CheckDlgButton(IDC_REMEMBERDOWNLOADED,thePrefs.IsRememberingDownloadedFiles());
	CheckDlgButton(IDC_REMEMBERCANCELLED,thePrefs.IsRememberingCancelledFiles());

	//Xman remove unused AICH-hashes
	if(thePrefs.IsRememberingDownloadedFiles()==false)
	{
		CheckDlgButton(IDC_REMEMBERAICH,FALSE);
		GetDlgItem(IDC_REMEMBERAICH)->EnableWindow(FALSE);
	}
	else
		CheckDlgButton(IDC_REMEMBERAICH, thePrefs.GetRememberAICH());
	//Xman end
	if(!thePrefs.m_bUseCompression){
		GetDlgItem(IDC_DONTCOMPRESSEXT_LBL)->EnableWindow(FALSE);
		GetDlgItem(IDC_DONTCOMPRESSEXT)->EnableWindow(FALSE);
	}
	else{
		CheckDlgButton(IDC_DONTCOMPRESSEXT_LBL,thePrefs.dontcompressext);// X: [DCE] - [DontCompressExt]
		GetDlgItem(IDC_DONTCOMPRESSEXT)->EnableWindow(thePrefs.dontcompressext);
		SetDlgItemText(IDC_DONTCOMPRESSEXT,thePrefs.compressExt);
	}

	GetDlgItem(IDC_STARTNEXTFILECAT)->EnableWindow(IsDlgButtonChecked(IDC_STARTNEXTFILE));
	GetDlgItem(IDC_STARTNEXTFILECAT2)->EnableWindow(IsDlgButtonChecked(IDC_STARTNEXTFILE));
}

BOOL CPPgFiles::OnApply()
{
	if(m_bModified){ // X: [CI] - [Code Improvement] Apply if modified
	CString buffer;

    bool bOldPreviewPrio = thePrefs.m_bpreviewprio;
	thePrefs.m_bpreviewprio = IsDlgButtonChecked(IDC_PREVIEWPRIO)!=0;

    if (bOldPreviewPrio != thePrefs.m_bpreviewprio)
		theApp.emuledlg->transferwnd->downloadlistctrl.CreateMenues();
  
	thePrefs.m_bDAP = IsDlgButtonChecked(IDC_DAP)!=0;
	thePrefs.m_bUAP = IsDlgButtonChecked(IDC_UAP)!=0;
	thePrefs.m_bPauseOnFileComplete = IsDlgButtonChecked(IDC_PAUSEONCOMP)!=0; // NEO: POFC - [PauseOnFileComplete]

	if (IsDlgButtonChecked(IDC_STARTNEXTFILE))
	{
		thePrefs.m_istartnextfile = 1;
		if (IsDlgButtonChecked(IDC_STARTNEXTFILECAT))
			thePrefs.m_istartnextfile = 2;
		else if (IsDlgButtonChecked(IDC_STARTNEXTFILECAT2))
			thePrefs.m_istartnextfile = 3;
	}
	else
		thePrefs.m_istartnextfile = 0;

	thePrefs.m_btransferfullchunks = IsDlgButtonChecked(IDC_FULLCHUNKTRANS)!=0;

	thePrefs.watchclipboard = IsDlgButtonChecked(IDC_WATCHCB)!=0;
	thePrefs.SetRememberDownloadedFiles(IsDlgButtonChecked(IDC_REMEMBERDOWNLOADED)!=0);
	thePrefs.SetRememberCancelledFiles(IsDlgButtonChecked(IDC_REMEMBERCANCELLED)!=0);

	thePrefs.SetRememberAICH(IsDlgButtonChecked(IDC_REMEMBERAICH)!=0); //Xman remove unused AICH-hashes

	thePrefs.addnewfilespaused = IsDlgButtonChecked(IDC_ADDNEWFILESPAUSED)!=0;
	thePrefs.autofilenamecleanup = IsDlgButtonChecked(IDC_FNCLEANUP)!=0;
	thePrefs.m_bUseOldTimeRemaining = IsDlgButtonChecked(IDC_PF_TIMECALC)==0;

		GetDlgItemText(IDC_VIDEOPLAYER,thePrefs.m_strVideoPlayer);
		thePrefs.m_strVideoPlayer.Trim();
		
	bool changed=false;// X: [DCE] - [DontCompressExt]
	bool checked=IsDlgButtonChecked(IDC_DONTCOMPRESSEXT_LBL)!=0;
	if(checked){
		CString text;
		GetDlgItemText(IDC_DONTCOMPRESSEXT,text);
		if(changed=(text.CompareNoCase(thePrefs.compressExt)!=0))
			thePrefs.compressExt=text.MakeLower();
	}
	if(thePrefs.dontcompressext!=checked||changed){
		thePrefs.dontcompressext=checked;
		theApp.sharedfiles->Reload(false);// X: [QOH] - [QueryOnHashing]
		theApp.downloadqueue->UpdateCompressible();// X: [DCE] - [DontCompressExt]
	}

	LoadSettings();
	SetModified(FALSE);
		m_bModified = false; // X: [CI] - [Code Improvement] Apply if modified
	}
	return CPropertyPage::OnApply();
}

void CPPgFiles::Localize(void)
{
	if (m_hWnd)
	{
		SetWindowText(GetResString(IDS_PW_FILES));
		//Xman removed: not enough space
		//SetDlgItemText(IDC_LBL_MISC,GetResString(IDS_PW_MISC));
		SetDlgItemText(IDC_PF_TIMECALC,GetResString(IDS_PF_ADVANCEDCALC));
		SetDlgItemText(IDC_UAP,GetResString(IDS_PW_UAP));
		SetDlgItemText(IDC_DAP,GetResString(IDS_PW_DAP));
		SetDlgItemText(IDC_PREVIEWPRIO,GetResString(IDS_DOWNLOADMOVIECHUNKS));
		SetDlgItemText(IDC_ADDNEWFILESPAUSED,GetResString(IDS_ADDNEWFILESPAUSED));
		SetDlgItemText(IDC_WATCHCB,GetResString(IDS_PF_WATCHCB));
		SetDlgItemText(IDC_FULLCHUNKTRANS,GetResString(IDS_FULLCHUNKTRANS));
		SetDlgItemText(IDC_STARTNEXTFILE,GetResString(IDS_STARTNEXTFILE));
		SetDlgItemText(IDC_STARTNEXTFILECAT,GetResString(IDS_PREF_STARTNEXTFILECAT));
		SetDlgItemText(IDC_STARTNEXTFILECAT2,GetResString(IDS_PREF_STARTNEXTFILECATONLY));
		SetDlgItemText(IDC_FNC,GetResString(IDS_EDIT));
		SetDlgItemText(IDC_ONND,GetResString(IDS_ONNEWDOWNLOAD));
		SetDlgItemText(IDC_FNCLEANUP,GetResString(IDS_AUTOCLEANUPFN));
		SetDlgItemText(IDC_PAUSEONCOMP,GetResString(IDS_PAUSEONCOMP)); // NEO: POFC - [PauseOnFileComplete]

		SetDlgItemText(IDC_STATICVIDEOPLAYER,GetResString(IDS_PW_VIDEOPLAYER));
		SetDlgItemText(IDC_REMEMBERDOWNLOADED,GetResString(IDS_PW_REMEMBERDOWNLOADED));
		SetDlgItemText(IDC_REMEMBERCANCELLED,GetResString(IDS_PW_REMEMBERCANCELLED));		
		SetDlgItemText(IDC_REMEMBERAICH,GetResString(IDS_REMEMBERAICH)); //Xman remove unused AICH-hashes
		SetDlgItemText(IDC_DONTCOMPRESSEXT_LBL,GetResString(IDS_DONTCOMPRESSEXT));// X: [DCE] - [DontCompressExt]
	}
}

void CPPgFiles::OnSetCleanupFilter()
{
	CString prompt = GetResString(IDS_FILTERFILENAMEWORD);
	InputBox inputbox;
	inputbox.SetLabels(GetResString(IDS_FNFILTERTITLE), prompt, thePrefs.GetFilenameCleanups());
	inputbox.DoModal();
	if (!inputbox.WasCancelled())
		thePrefs.SetFilenameCleanups(inputbox.GetInput());
}

void CPPgFiles::BrowseVideoplayer()
{
	CString strPlayerPath;
	GetDlgItemText(IDC_VIDEOPLAYER, strPlayerPath);
	CFileDialog dlgFile(TRUE, _T("exe"), strPlayerPath, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY, _T("Executable (*.exe)|*.exe||"), NULL, 0);
	if (dlgFile.DoModal() == IDOK)
	{
		SetDlgItemText(IDC_VIDEOPLAYER,dlgFile.GetPathName());
		OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified
	}
}

void CPPgFiles::OnSettingsChangeSNF()
{
	OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified
	GetDlgItem(IDC_STARTNEXTFILECAT)->EnableWindow(IsDlgButtonChecked(IDC_STARTNEXTFILE));
	GetDlgItem(IDC_STARTNEXTFILECAT2)->EnableWindow(IsDlgButtonChecked(IDC_STARTNEXTFILE));
}

void CPPgFiles::OnSettingsChangeCat(uint8 index)
{
	OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified
	bool on = IsDlgButtonChecked(index == 1 ? IDC_STARTNEXTFILECAT : IDC_STARTNEXTFILECAT2)!=0;
	if (on)
		CheckDlgButton(index == 1 ? IDC_STARTNEXTFILECAT2 : IDC_STARTNEXTFILECAT, FALSE);
}

//Xman remove unused AICH-hashes
void CPPgFiles::OnBnClickedRememberdownloaded()
{
	OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified
	if(IsDlgButtonChecked(IDC_REMEMBERDOWNLOADED))
	{
		GetDlgItem(IDC_REMEMBERAICH)->EnableWindow(true);
	}
	else
	{
		CheckDlgButton(IDC_REMEMBERAICH,FALSE);
		GetDlgItem(IDC_REMEMBERAICH)->EnableWindow(false);
	}
}
//Xman end
void CPPgFiles::OnSettingsChangeExt()// X: [DCE] - [DontCompressExt]
{
	OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified
	GetDlgItem(IDC_DONTCOMPRESSEXT)->EnableWindow(IsDlgButtonChecked(IDC_DONTCOMPRESSEXT_LBL));
}
