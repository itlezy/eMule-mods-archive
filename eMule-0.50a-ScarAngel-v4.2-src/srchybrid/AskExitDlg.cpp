//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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

// Extended Prompt on Exit dialog [leuk_he] - Stulle

#include "stdafx.h"
#include "eMule.h"
#include "eMuleDlg.h"
#include "OtherFunctions.h"
#include "resource.h"
#include "MenuCmds.h"
#include "AskExitDlg.h"
#include "Preferences.h"
#include "NTService.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

// CAskExitDlg dialog

IMPLEMENT_DYNAMIC(CAskExitDlg, CDialog)
CAskExitDlg::CAskExitDlg()
	: CDialog(CAskExitDlg::IDD)
{
}

BOOL CAskExitDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	// localize:
	if(m_hWnd)
	{
		GetDlgItem(IDC_EXITQUESTION)->SetWindowText(GetResString(IDS_MAIN_EXIT));
		GetDlgItem(IDC_DONTASKMEAGAINCB)->SetWindowText(GetResString(IDS_DONOTASKAGAIN));
		GetDlgItem(IDYES)->SetWindowText(GetResString(IDS_YES));
		GetDlgItem(IDNO)->SetWindowText(GetResString(IDS_NO));
		GetDlgItem(IDYESSERVICE)->SetWindowText(GetResString(IDS_YESSERVICE));
		GetDlgItem(IDNOMINIMIZE)->SetWindowText(GetResString(IDS_NOMINIMIZE));
	}

	if(thePrefs.confirmExit)
		CheckDlgButton(IDC_DONTASKMEAGAINCB,0);
	else
		CheckDlgButton(IDC_DONTASKMEAGAINCB,1);

	int b_installed;
	int  i_startupmode;
	int rights;

	NTServiceGet(b_installed,i_startupmode,	rights);
	if (b_installed == 1) 
		GetDlgItem( IDYESSERVICE)->EnableWindow(true);
	else
		GetDlgItem( IDYESSERVICE)->EnableWindow(false);
	return TRUE;  
}



CAskExitDlg::~CAskExitDlg()
{
}

void CAskExitDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAskExitDlg, CDialog)
	ON_BN_CLICKED(IDYES, OnBnClickedYes)
	ON_BN_CLICKED(IDNO, OnBnClickedCancel)
	ON_BN_CLICKED(IDYESSERVICE, OnBnClickedYesservice)
	ON_BN_CLICKED(IDNOMINIMIZE, OnBnClickedNominimize)
END_MESSAGE_MAP()


// CAskExitDlg message handlers

void CAskExitDlg::OnBnClickedYes()
{
	thePrefs.confirmExit = (IsDlgButtonChecked(IDC_DONTASKMEAGAINCB)==0);
	// TODO: Add your control notification handler code here
	EndDialog(IDYES);
}

void CAskExitDlg::OnBnClickedCancel()
{
	thePrefs.confirmExit = IsDlgButtonChecked(IDC_DONTASKMEAGAINCB)==0;
	EndDialog(IDNO);
}

void CAskExitDlg::OnBnClickedYesservice()
{
	NtserviceStartwhenclose=true;
	EndDialog(IDYES);
}

void CAskExitDlg::OnBnClickedNominimize()
{
	if (thePrefs.GetMinToTray())
		theApp.emuledlg->PostMessage(WM_SYSCOMMAND , SC_MINIMIZE, 0);
	else
		theApp.emuledlg->PostMessage(WM_SYSCOMMAND, MP_MINIMIZETOTRAY, 0);
	EndDialog(IDNO);
}