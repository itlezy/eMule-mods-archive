//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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
#include "emuleDlg.h"
#include "PPgAcK.h"
#include "OtherFunctions.h"
#include "Preferences.h"
#include "Opcodes.h"
//>>> taz::ASF
#include "ServerWnd.h"
#include "ServerListCtrl.h"
//<<< taz::ASF
//>>> taz::AcK filters [Aenarion/Xanatos]
#include "InputBox.h"
#include "MD5Sum.h"
#include "Knownfile.h"
//<<< taz::AcK filters [Aenarion/Xanatos]

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CPPgAcK, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgAcK, CPropertyPage)
//>>> taz::ASF
	ON_EN_CHANGE(IDC_FILTERNAME, OnSettingsChange)
	ON_EN_CHANGE(IDC_FILTERDESC, OnSettingsChange)
	ON_EN_CHANGE(IDC_FILTERUSERVAL, OnSettingsChange)
	ON_EN_CHANGE(IDC_FILTERFILEVAL, OnSettingsChange)
	ON_BN_CLICKED(IDC_ASF_AUTOFILTER , OnSettingsChange)
	ON_BN_CLICKED(IDC_ASF_ON, OnBnClickedASF_ON)
	ON_BN_CLICKED(IDC_SERVERANALYZER_ON, OnBnClickedSERVERANALYZER_ON)
	ON_BN_CLICKED(IDC_ASF_AUTOFILTER, OnSettingsChange)
//<<< taz::ASF
//>>> taz::AcK filters [Aenarion/Xanatos]
	ON_BN_CLICKED(IDC_FF_CHK, OnClickButton)
	ON_BN_CLICKED(IDC_FF2_CHK, OnSettingsChange)
	ON_BN_CLICKED(IDC_CHANGE_PASSFF, OnBnClickedChangePassff)
//<<< taz::AcK filters [Aenarion/Xanatos]

	//ON_WM_HELPINFO()
	ON_WM_HSCROLL()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

CPPgAcK::CPPgAcK()
	: CPropertyPage(CPPgAcK::IDD)
{
	m_iFilter = (thePrefs.ASF_ON() ? 0 : 1); //>>> taz::ASF
}

CPPgAcK::~CPPgAcK()
{
}

void CPPgAcK::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_ASF_ON, m_iFilter); //>>> taz::ASF
}

BOOL CPPgAcK::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	LoadSettings();
	Localize();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CPPgAcK::LoadSettings(void)
{
	if (m_hWnd)
	{
//>>> taz::ASF
		GetDlgItem(IDC_FILTERNAME)->SetWindowText(thePrefs.ServerFilterVal);
		GetDlgItem(IDC_FILTERDESC)->SetWindowText(thePrefs.ServerFilterDesc);
		SetDlgItemInt(IDC_FILTERUSERVAL, thePrefs.ServerFilterByUsers, FALSE);
		SetDlgItemInt(IDC_FILTERFILEVAL, thePrefs.ServerFilterByFiles, FALSE);
		m_iFilter = (thePrefs.ASF_ON() ? 0 : 1);
		GetDlgItem(IDC_ASF_AUTOFILTER)->EnableWindow(thePrefs.ASF_ON());
		CheckRadioButton(IDC_ASF_AUTOFILTER, IDC_SERVERANALYZER_ON, IDC_ASF_AUTOFILTER + m_iFilter);

		if(!thePrefs.ASF_ON())
			thePrefs.SetASF_AutoFilter(false);
		if(thePrefs.ASF_AutoFilter())
			CheckDlgButton(IDC_ASF_AUTOFILTER,1);
		else
			CheckDlgButton(IDC_ASF_AUTOFILTER,0);
//<<< taz::ASF
//>>> taz::AcK filters [Aenarion/Xanatos]
		CheckDlgButton(IDC_FF_CHK, thePrefs.m_bFamilyFilter);
		CheckDlgButton(IDC_FF2_CHK, thePrefs.m_bFakeFilter);
		GetDlgItem(IDC_CHANGE_PASSFF)->EnableWindow(thePrefs.m_isPWProtShow);
//<<< taz::AcK filters [Aenarion/Xanatos]
	}
}

BOOL CPPgAcK::OnApply()
{
	if (!UpdateData())
		return FALSE;

//>>> taz::ASF
	GetDlgItem(IDC_FILTERNAME)->GetWindowText(thePrefs.ServerFilterVal,ARRSIZE(thePrefs.ServerFilterVal));
	GetDlgItem(IDC_FILTERDESC)->GetWindowText(thePrefs.ServerFilterDesc,ARRSIZE(thePrefs.ServerFilterDesc));

	thePrefs.ServerFilterByUsers = GetDlgItemInt(IDC_FILTERUSERVAL, NULL, FALSE);
	if (thePrefs.ServerFilterByUsers < 0)
		thePrefs.ServerFilterByUsers = 0;
	else if (thePrefs.ServerFilterByUsers > 1000000000)
		thePrefs.ServerFilterByUsers = 1000000000;

	thePrefs.ServerFilterByFiles = GetDlgItemInt(IDC_FILTERFILEVAL, NULL, FALSE);
	if (thePrefs.ServerFilterByFiles < 0)
		thePrefs.ServerFilterByFiles = 0;
	else if (thePrefs.ServerFilterByFiles > 1000000000)
		thePrefs.ServerFilterByFiles = 1000000000;
	if (thePrefs.ASF_ON()!= (m_iFilter == 0)){
		theApp.emuledlg->serverwnd->serverlistctrl.ShowServers();
		thePrefs.SetASF(m_iFilter == 0);
	}
	thePrefs.SetASF_AutoFilter(IsDlgButtonChecked(IDC_ASF_AUTOFILTER)!=0);

	if(thePrefs.ASF_AutoFilter())
		theApp.emuledlg->serverwnd->OnBnClickedServerFilterDlg();

	theApp.emuledlg->serverwnd->GetDlgItem(IDC_SRVFILTER_BTN)->EnableWindow(thePrefs.ASF_ON() && !thePrefs.ASF_AutoFilter());
//<<< taz::ASF
//>>> taz::AcK filters [Aenarion/Xanatos]
	thePrefs.m_bFakeFilter = IsDlgButtonChecked(IDC_FF2_CHK) != 0;
	GetDlgItem(IDC_CHANGE_PASSFF)->EnableWindow(thePrefs.m_isPWProtShow);
//<<< taz::AcK filters [Aenarion/Xanatos]

	LoadSettings();
	SetModified(FALSE);
	return CPropertyPage::OnApply();
}

void CPPgAcK::Localize(void)
{
	if (m_hWnd)
	{
//>>> taz::ASF
		GetDlgItem(IDC_FILTERNAMELABEL)->SetWindowText(GetResString(IDS_FILTERNAMELABEL));
		GetDlgItem(IDC_FILTERDESCLABEL)->SetWindowText(GetResString(IDS_FILTERDESCLABEL));
		GetDlgItem(IDC_FILTERUSER)->SetWindowText(GetResString(IDS_FILTERUSERS_DESC));
		GetDlgItem(IDC_FILTERUSER2)->SetWindowText(GetResString(IDS_LUSERS));
		GetDlgItem(IDC_FILTERFILE)->SetWindowText(GetResString(IDS_FILTERUSERS_DESC));
		GetDlgItem(IDC_FILTERFILE2)->SetWindowText(GetResString(IDS_FILES));
		GetDlgItem(IDC_ASF_AUTOFILTER)->SetWindowText(GetResString(IDS_ASF_AUTOFILTER));
		GetDlgItem(IDC_ASF_ON)->SetWindowText(GetResString(IDS_ASF_ON));
		GetDlgItem(IDC_SERVERANALYZER_ON)->SetWindowText(GetResString(IDS_SERVERANALYZER_ON));
		GetDlgItem(IDC_ASF_AUTOFILTER)->SetWindowText(GetResString(IDS_ASF_AUTOFILTER));
//<<< taz::ASF
//>>> taz::AcK filters [Aenarion/Xanatos]
        GetDlgItem(IDC_FF_GROUP)->SetWindowText(GetResString(IDS_FF_GROUP));
		GetDlgItem(IDC_FF_CHK)->SetWindowText(GetResString(IDS_ACTIVATE_FF));
		GetDlgItem(IDC_CHANGE_PASSFF)->SetWindowText(GetResString(IDS_CHANGE_PASSFF));
		GetDlgItem(IDC_FF2_CHK)->SetWindowText(GetResString(IDS_ACTIVATE_FF2));
//<<< taz::AcK filters [Aenarion/Xanatos]
	}
}

//>>> taz::ASF
void CPPgAcK::OnBnClickedASF_ON(){
	GetDlgItem(IDC_ASF_AUTOFILTER)->EnableWindow(TRUE);
	CheckRadioButton(IDC_ASF_AUTOFILTER, IDC_SERVERANALYZER_ON, IDC_ASF_AUTOFILTER);
	m_iFilter = 0;
	OnSettingsChange();
}

void CPPgAcK::OnBnClickedSERVERANALYZER_ON(){
	GetDlgItem(IDC_ASF_AUTOFILTER)->EnableWindow(FALSE);
	CheckRadioButton(IDC_ASF_AUTOFILTER, IDC_SERVERANALYZER_ON, IDC_ASF_AUTOFILTER + 1);
	m_iFilter = 1;
	OnSettingsChange();
}
//<<< taz::ASF

//>>> taz::AcK filters [Aenarion/Xanatos]
void CPPgAcK::OnClickButton()
{
	if(thePrefs.m_sPassFF.IsEmpty() || !thePrefs.m_isPWProtShow){

	InputBox inputbox1;
	CString pass1 = L"";

	inputbox1.SetLabels(GetResString(IDS_FAMILY_FILTER),GetResString(IDS_NEWPASS), L"");
	inputbox1.SetPassword(true);

	if (inputbox1.DoModal() == IDOK)
		pass1=inputbox1.GetInput();
	else
		return;

	if(pass1.GetLength() > 0)
	{
		thePrefs.SetPWProtShow(true);
		thePrefs.m_sPassFF=MD5Sum(pass1).GetHash().GetBuffer(0);
	}
	else
		return;

	}

	if(thePrefs.m_isPWProtShow==true)
	{
		InputBox inputbox;
		CString pass;
		inputbox.SetLabels(GetResString(IDS_FAMILY_FILTER),GetResString(IDS_CONFIRMPASS), L"");
		inputbox.SetPassword(true);
		if (inputbox.DoModal() == IDOK && inputbox.GetInput().GetLength() > 0){
			pass=inputbox.GetInput();
			pass=MD5Sum(pass).GetHash().GetBuffer(0);

			if(thePrefs.GetPassFF() == pass){
				thePrefs.m_bFamilyFilter = IsDlgButtonChecked(IDC_FF_CHK) !=0;
				SetModified(TRUE);
			}
			else SetModified(FALSE);
		}
		else SetModified(FALSE);
	}
}

void CPPgAcK::OnBnClickedChangePassff()
{
	if(thePrefs.m_sPassFF.IsEmpty() || !thePrefs.m_isPWProtShow)
	{
		AfxMessageBox(GetResString(IDS_NONCEPASS) ,MB_OK);
		return;
	}
	InputBox inputbox;
	CString pass;
	inputbox.SetLabels(GetResString(IDS_FAMILY_FILTER),GetResString(IDS_CHANGEPASS), L"");
	inputbox.SetPassword(true);
	if (inputbox.DoModal() == IDOK) {

		if(inputbox.GetInput().GetLength() > 0){
			pass=inputbox.GetInput();
			pass=MD5Sum(pass).GetHash().GetBuffer(0);

			if(thePrefs.GetPassFF() == pass){
				InputBox inputbox1;
				CString pass1 = L"";
				inputbox1.SetLabels(GetResString(IDS_FAMILY_FILTER),GetResString(IDS_CHANGEORDELETEPASS), L"");
				inputbox1.SetPassword(true);

				if (inputbox1.DoModal() == IDOK){
					pass1=inputbox1.GetInput();
				}
				else
					return;

				if(pass1.GetLength() > 0)
				{
					thePrefs.m_sPassFF=MD5Sum(pass1).GetHash().GetBuffer(0);
				}
			}
		}
	}
}
//<<< taz::AcK filters [Aenarion/Xanatos]