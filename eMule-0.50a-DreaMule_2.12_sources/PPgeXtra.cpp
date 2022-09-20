//this file is part of eMule
//Copyright (C)2002 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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
#include "OtherFunctions.h"
#include "Preferences.h"
#include "HelpIDs.h"
#include "Opcodes.h"
#include "shellapi.h"
#include "Windows.h"
#include "ppgextra.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CPPgeXtra, CPropertyPage)

CPPgeXtra::CPPgeXtra()
	: CPropertyPage(CPPgeXtra::IDD)
{
}

CPPgeXtra::~CPPgeXtra()
{
}

void CPPgeXtra::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPPgeXtra, CPropertyPage)

	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDC_EN_HOTKEY, OnBnClickedEnHotkey)
	ON_BN_CLICKED(IDC_CHECK1, OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_CHECK2, OnBnClickedCheck2)
	ON_BN_CLICKED(IDC_CHECK3, OnBnClickedCheck3)
	ON_BN_CLICKED(IDC_CHECK4, OnBnClickedCheck4)
	ON_CBN_SELCHANGE(IDC_COMBOKEY, OnCbnSelchangeCombokey)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERFDC, OnNMCustomdrawSliderfdc)
	ON_BN_CLICKED(IDC_FDCTEA, OnBnClickedFdctea)
END_MESSAGE_MAP()
// CPPgeXtra message handlers


BOOL CPPgeXtra::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);
	//[FDC]Start set max and min range for check parameter and pointer position
	fdcspos = 100 - thePrefs.FDCSensitivity;//convert the max percentage mismatch to percent match so the slider goes the right way
    CSliderCtrl* fdcslider = (CSliderCtrl*)GetDlgItem(IDC_SLIDERFDC);
	fdcslider->SetRange(6,30,false);//was SetRange(8,30,false) greater range default moved for 1.5a
	fdcslider->SetPos(fdcspos);
	b_localdoubleFDC = thePrefs.doubleFDC; //set double error state
	((CButton*)GetDlgItem(IDC_FDCTEA))->SetCheck(b_localdoubleFDC);//set enable state
	//[FDC]End set max and min range for check parameter

    //Get saved or default states
	key = thePrefs.hotkey_key;
	mod = thePrefs.hotkey_mod;
	enablestate = thePrefs.hotkey_enabled; //get enabled state
	((CButton*)GetDlgItem(IDC_EN_HOTKEY))->SetCheck(enablestate);//set enable state
	Localize();
	//Note: Localize has just set the hot keys check box state, 
	//So OnBnClickedEnHotkey retrieves it and sets the items enable state
	OnBnClickedEnHotkey();
	//[FDC] Start control enable state
	GetDlgItem(IDC_SLIDERFDC)->EnableWindow(thePrefs.BadNameCheck);
	GetDlgItem(IDC_STATIC_FDC)->EnableWindow(thePrefs.BadNameCheck);
	GetDlgItem(IDC_STATIC_LOW)->EnableWindow(thePrefs.BadNameCheck);
	GetDlgItem(IDC_STATIC_HIGH)->EnableWindow(thePrefs.BadNameCheck);
	GetDlgItem(IDC_FDCTEA)->EnableWindow(thePrefs.BadNameCheck);
	//[FDC] Start control enable state
    //set the checkbox/button state from the mod (hotkey_mod) variable. Note order as onscreen is check1,2,4,3
    CheckDlgButton(IDC_CHECK1, (mod & MOD_CONTROL) == MOD_CONTROL);
	CheckDlgButton(IDC_CHECK2, (mod & MOD_ALT) == MOD_ALT);
	CheckDlgButton(IDC_CHECK4, (mod & MOD_WIN) == MOD_WIN);
	CheckDlgButton(IDC_CHECK3, (mod & MOD_SHIFT) == MOD_SHIFT);
	((CComboBox*)GetDlgItem(IDC_COMBOKEY))->SetCurSel(key-65);
	return TRUE;  // return TRUE unless you set the focus to a control
}

BOOL CPPgeXtra::OnApply()
{	/*Start Hotkey save code*/
    if(thePrefs.hotkey_enabled == false && enablestate == true)//Not enabled apply - enable
	  {
		thePrefs.hotkey_enabled = enablestate;
		RegisterHotKey(thePrefs.HideWnd, 0x1515, mod, key);
	   } else
	       if(thePrefs.hotkey_enabled == true && enablestate == false)//Enabled apply - disable
			 {
			   thePrefs.hotkey_enabled = enablestate;
			   UnregisterHotKey(thePrefs.HideWnd, 0x1515);
			  } else //Hidden is enabled and hot keys have been changed
				  if(thePrefs.hotkey_enabled == true && (thePrefs.hotkey_key != key || thePrefs.hotkey_mod != mod))
		            {	//Unregister the old sequence
						UnregisterHotKey(thePrefs.HideWnd, 0x1515);
						//Register new sequence
						RegisterHotKey(thePrefs.HideWnd, 0x1515, mod, key);
	 				 }
    //SAVE and changes made, if none the no harm done
    thePrefs.hotkey_mod = mod;
    thePrefs.hotkey_key = key;
    /*End Hotkey save code*/
    //[FDC] Save settting, get convert and save.
	thePrefs.FDCSensitivity = 100 - fdcspos;
	thePrefs.doubleFDC = b_localdoubleFDC;//save double error state


    //[IPUP] IP update 


	SetModified(TRUE);
	return CPropertyPage::OnApply();
}

void CPPgeXtra::Localize(void)
{
	if(m_hWnd)
	{//Set dialog text
	 GetDlgItem(IDC_EN_HOTKEY)->SetWindowText(GetResString(IDS_EN_HOTKEY));
	 GetDlgItem(IDC_HOTKEY)->SetWindowText(GetResString(IDS_HOTKEY));
	 //[FDC] Set dialog text for FDC sensitivity slider
	 GetDlgItem(IDC_STATIC_FDC)->SetWindowText(GetResString(IDS_FDC_SENSITIVITY));
	 GetDlgItem(IDC_STATIC_LOW)->SetWindowText(GetResString(IDS_PRIOLOW));//reused string table entry
	 GetDlgItem(IDC_STATIC_HIGH)->SetWindowText(GetResString(IDS_PRIOHIGH));//reused string table entry
	 GetDlgItem(IDC_FDCTEA)->SetWindowText(GetResString(IDS_FCDTEA));
 	}
}



void CPPgeXtra::OnBnClickedEnHotkey()
{
    //Get enabled disabled state
    enablestate =(IsDlgButtonChecked(IDC_EN_HOTKEY)!=0);
   
	//Toggle hot keys selection controls state
	GetDlgItem(IDC_HOTKEY)->EnableWindow(enablestate);
	GetDlgItem(IDC_CHECK1)->EnableWindow(enablestate);
	GetDlgItem(IDC_CHECK2)->EnableWindow(enablestate);
	GetDlgItem(IDC_CHECK3)->EnableWindow(enablestate);
	GetDlgItem(IDC_CHECK4)->EnableWindow(enablestate);
	SetModified(TRUE);
}

void CPPgeXtra::OnBnClickedCheck1()
{//Ctrl
 if(IsDlgButtonChecked(IDC_CHECK1)) mod |= MOD_CONTROL;//02
   else mod &= 0xfffd;
 SetModified(TRUE);
}

void CPPgeXtra::OnBnClickedCheck2()
{//Alt
 if(IsDlgButtonChecked(IDC_CHECK2)) mod |= MOD_ALT;//01
   else mod &= 0xfffe;
 SetModified(TRUE);
}

void CPPgeXtra::OnBnClickedCheck4()
{//Win
 if(IsDlgButtonChecked(IDC_CHECK4)) mod |= MOD_WIN;//08
   else mod &= 0xfff7;
 SetModified(TRUE);
}

void CPPgeXtra::OnBnClickedCheck3()
{//Shift
 if(IsDlgButtonChecked(IDC_CHECK3)) mod |= MOD_SHIFT;//04
   else mod &= 0xfffb;
 SetModified(TRUE);	
}

void CPPgeXtra::OnCbnSelchangeCombokey()
{//Get Letter Key
 CString strLetter;
 GetDlgItem(IDC_COMBOKEY)->GetWindowText(strLetter);
 key=(int)strLetter[0];
 SetModified(TRUE);
}

/*[FDC] check slider position on redraw*/
void CPPgeXtra::OnNMCustomdrawSliderfdc(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	// [FDC] get slider position
	int newpos = ((CSliderCtrl*)GetDlgItem(IDC_SLIDERFDC))->GetPos();
	if(newpos!=fdcspos){//store new position and set modified to enable apply
						fdcspos = newpos;
						SetModified(TRUE);
	                    }
	*pResult = 0;
}







/*[FDC] 2 bad names for alert*/
void CPPgeXtra::OnBnClickedFdctea()
{//get 2 bad names checkbox state
 b_localdoubleFDC =(IsDlgButtonChecked(IDC_FDCTEA)!=0);
 SetModified(TRUE);
}
