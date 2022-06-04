//this file is part of eMule morphXT
//Copyright (C)2006 leuk_he ( strEmail.Format("%s@%s", "leukhe", "gmail.com") / http://emulemorph.sf.net )
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
#include <string.h>
#include "emule.h"
#include "emuledlg.h"
#include "otherfunctions.h"
#include "PPGtooltipped.h" //MORPH leuk_he addded tooltipped
#include "WebServer.h"
#include "PPgIonixWebServer.h"
#include "NTService.h"
#include "PPgNTServer.h"
#include "PreferencesDlg.h"
#include "resource.h"
                 
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CPPgNTService, CPropertyPage)
CPPgNTService::CPPgNTService()
	:  CPPgtooltipped(CPPgNTService::IDD)
{
	m_bIsInit = false;
	// MORPH start tabbed option [leuk_he]
	m_imageList.DeleteImageList();
	m_imageList.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 14+1, 0);
	m_imageList.Add(CTempIconLoader(_T("WEB")));
	// MORPH end tabbed option [leuk_he]
}

CPPgNTService::~CPPgNTService()
{
}


void CPPgNTService::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_SERVICE_OPT_BOX, m_cbOptLvl);
  // MORPH start tabbed options [leuk_he]
   DDX_Control(pDX, IDC_TAB_SERVICE3 , m_tabCtr);
  // MORPH end tabbed options [leuk_he]

}

BEGIN_MESSAGE_MAP(CPPgNTService, CPropertyPage)
	ON_BN_CLICKED(IDC_SVC_INSTALLSERVICE, OnBnClickedInstall)	
	ON_BN_CLICKED(IDC_SVC_SERVERUNINSTALL, OnBnClickedUnInstall)	

    ON_BN_CLICKED(IDC_SVC_STARTWITHSYSTEM, OnBnStartSystem)	
	ON_BN_CLICKED(IDC_SVC_MANUALSTART, OnBnManualStart)	
	ON_BN_CLICKED(IDC_SVC_SETTINGS ,   OnBnAllSettings)	
	ON_BN_CLICKED(IDC_SVC_RUNBROWSER , OnBnRunBRowser)	
	ON_BN_CLICKED(IDC_SVC_REPLACESERVICE , OnBnReplaceStart)	
	ON_CBN_SELCHANGE(IDC_SERVICE_OPT_BOX, OnCbnSelChangeOptLvl)
	//MORPH START - Added by Stulle, Adjustable NT Service Strings
	ON_EN_CHANGE(IDC_SERVICE_NAME, OnCbnSelChangeOptLvl)
	ON_EN_CHANGE(IDC_SERVICE_DISP_NAME, OnCbnSelChangeOptLvl)
	ON_EN_CHANGE(IDC_SERVICE_DESCR, OnCbnSelChangeOptLvl)
	//MORPH END   - Added by Stulle, Adjustable NT Service Strings
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_SERVICE3, OnTcnSelchangeTab)
	ON_WM_HELPINFO()
END_MESSAGE_MAP()


BOOL CPPgNTService::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);
 
	InitOptLvlCbn(true);

	LoadSettings();
    InitTooltips(); //MORPH leuk_he tolltipped

	Localize();

	return TRUE;
}

void CPPgNTService::LoadSettings(void)
{
	if(m_hWnd)
	{
		if(thePrefs.GetServiceStartupMode()==2) 
		{
			CheckDlgButton(IDC_SVC_RUNBROWSER   ,BST_UNCHECKED );
			CheckDlgButton(IDC_SVC_REPLACESERVICE, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(IDC_SVC_RUNBROWSER   ,BST_CHECKED );
			CheckDlgButton(IDC_SVC_REPLACESERVICE, BST_UNCHECKED);
		}

		//MORPH START - Added by Stulle, Adjustable NT Service Strings
		GetDlgItem(IDC_SERVICE_NAME)->SetWindowText(thePrefs.GetServiceName());
		GetDlgItem(IDC_SERVICE_DISP_NAME)->SetWindowText(thePrefs.GetServiceDispName());
		GetDlgItem(IDC_SERVICE_DESCR)->SetWindowText(thePrefs.GetServiceDescr());
		//MORPH END   - Added by Stulle, Adjustable NT Service Strings


    	FillStatus();
		m_bIsInit = true;
	}
}



BOOL CPPgNTService::OnApply()
{
	if(m_bModified)
	{
		int b_installed;
		int  i_startupmode;
		int rights;
		// Startup with system, store in service.
		NTServiceGet(b_installed,i_startupmode,	rights);

		//MORPH START - Added by Stulle, Adjustable NT Service Strings
		CString strServiceName, strServiceDispName, strServiceDescr;
		GetDlgItem(IDC_SERVICE_NAME)->GetWindowText(strServiceName);
		GetDlgItem(IDC_SERVICE_DISP_NAME)->GetWindowText(strServiceDispName);
		GetDlgItem(IDC_SERVICE_DESCR)->GetWindowText(strServiceDescr);

		int iChangedStr = 0; // nothing changed
		if(strServiceName.Compare(thePrefs.GetServiceName()) != 0)
			iChangedStr = 1; // name under which we install changed, this is important!
		else if((strServiceDispName.Compare(thePrefs.GetServiceDispName()) != 0) || (strServiceDescr.Compare(thePrefs.GetServiceDescr()) != 0))
			iChangedStr = 2; // only visual strings changed, not so important...

		if(iChangedStr>0)
		{
			if(b_installed == 0)
			{
				thePrefs.SetServiceName(strServiceName);
				thePrefs.SetServiceDispName(strServiceDispName);
				thePrefs.SetServiceDescr(strServiceDescr);
				FillStatus();
			}
			else
			{
				int iResult = IDCANCEL;
				if(iChangedStr == 1)
					iResult = MessageBox(GetResString(IDS_SERVICE_NAME_CHANGED),GetResString(IDS_SERVICE_STR_CHANGED),MB_YESNOCANCEL|MB_ICONQUESTION|MB_DEFBUTTON3);
				else if(iChangedStr == 2)
				{
					if(NTServiceChangeDisplayStrings(strServiceDispName,strServiceDescr) != 0)
					{
						if(MessageBox(GetResString(IDS_SERVICE_DISP_CHANGE_FAIL),GetResString(IDS_SERVICE_STR_CHANGE_FAIL),MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2) == IDYES)
						{
							iChangedStr = 1;
							iResult = IDYES;
						}
					}
					else
						iResult = IDNO;
				}

				if(iChangedStr == 1 && iResult == IDYES) // reinstall service
				{
					if(CmdRemoveService()==0)
					{
						thePrefs.SetServiceName(strServiceName);
						thePrefs.SetServiceDispName(strServiceDispName);
						thePrefs.SetServiceDescr(strServiceDescr);
						if(CmdInstallService(i_startupmode == 1) != 0)
							MessageBox(GetResString(IDS_SERVICE_INSTALL_FAIL), GetResString(IDS_SERVICE_INSTALL_TITLE), MB_OK|MB_ICONWARNING);
					}
					else
					{
						MessageBox(GetResString(IDS_SERVICE_UNINSTALL_FAIL),GetResString(IDS_SERVICE_UNINSTALL_TITLE),MB_OK|MB_ICONWARNING);
						GetDlgItem(IDC_SERVICE_NAME)->SetWindowText(thePrefs.GetServiceName());
						GetDlgItem(IDC_SERVICE_DISP_NAME)->SetWindowText(thePrefs.GetServiceDispName());
						GetDlgItem(IDC_SERVICE_DESCR)->SetWindowText(thePrefs.GetServiceDescr());
					}
					FillStatus();
				}
				else if(iResult == IDNO) // just save settings
				{
					thePrefs.SetServiceName(strServiceName);
					thePrefs.SetServiceDispName(strServiceDispName);
					thePrefs.SetServiceDescr(strServiceDescr);
					FillStatus();
				}
				else // revert settings
				{
					GetDlgItem(IDC_SERVICE_NAME)->SetWindowText(thePrefs.GetServiceName());
					GetDlgItem(IDC_SERVICE_DISP_NAME)->SetWindowText(thePrefs.GetServiceDispName());
					GetDlgItem(IDC_SERVICE_DESCR)->SetWindowText(thePrefs.GetServiceDescr());
				}
			}
		}
		//MORPH END   - Added by Stulle, Adjustable NT Service Strings

		if (b_installed==1 && 
				(i_startupmode ==0 && (IsDlgButtonChecked(IDC_SVC_STARTWITHSYSTEM)==BST_CHECKED))||
				(i_startupmode ==1 && (IsDlgButtonChecked(IDC_SVC_MANUALSTART)==BST_CHECKED)))
			NTServiceSetStartupMode(IsDlgButtonChecked(IDC_SVC_STARTWITHSYSTEM)==BST_CHECKED);
	   // TODO: Apply setting 
		if ( IsDlgButtonChecked(IDC_SVC_RUNBROWSER)==BST_CHECKED)
		   thePrefs.m_iServiceStartupMode=1;
		else 
		   thePrefs.m_iServiceStartupMode=2;

		int iSel = m_cbOptLvl.GetCurSel();
		thePrefs.m_iServiceOptLvl = m_cbOptLvl.GetItemData(iSel);

		SetModified(FALSE);
		LoadSettings();
	}
	return CPropertyPage::OnApply();
}

void CPPgNTService::Localize(void)
{
	if(m_hWnd){
		SetWindowText(GetResString(IDS_TAB_NT_SERVICE));
		// MORPH start tabbed options [leuk_he]
		InitTab(true,2);
		m_tabCtr.SetCurSel(theApp.emuledlg->preferenceswnd->StartPageWebServer);
		// MORPH end tabbed options [leuk_he]

		GetDlgItem(IDC_SVC_INSTALLSERVICE)->SetWindowText(GetResString(IDS_SVC_INSTALLSERVICE));
		GetDlgItem(IDC_SVC_SERVERUNINSTALL)->SetWindowText(GetResString(IDS_SVC_SERVERUNINSTALL));
		GetDlgItem(IDC_SVC_STARTWITHSYSTEM)->SetWindowText(GetResString(IDS_SVC_STARTWITHSYSTEM));
		GetDlgItem(IDC_SVC_MANUALSTART)->SetWindowText(GetResString(IDS_SVC_MANUALSTART));
		GetDlgItem(IDC_SVC_SETTINGS)->SetWindowText(GetResString(IDS_SVC_SETTINGS));
		GetDlgItem(IDC_SVC_RUNBROWSER)->SetWindowText(GetResString(IDS_SVC_RUNBROWSER));
		GetDlgItem(IDC_SVC_REPLACESERVICE )->SetWindowText(GetResString(IDS_SVC_REPLACESERVICE));
		GetDlgItem(IDC_SVC_ONSTARTBOX)->SetWindowText(GetResString(IDS_SVC_ONSTARTBOX));
		GetDlgItem(IDC_SVC_STARTUPBOX)->SetWindowText(GetResString(IDS_SVC_STARTUPBOX));
		GetDlgItem(IDC_SVC_CURRENT_STATUS_LABEL)->SetWindowText(GetResString(IDS_SVC_CURRENT_STATUS_LABEL));
		GetDlgItem(IDC_SERVICE_OPT_GROUP)->SetWindowText(GetResString(IDS_SERVICE_OPT_GROUP));
		GetDlgItem(IDC_SERVICE_OPT_LABEL)->SetWindowText(GetResString(IDS_SERVICE_OPT_LABEL));
		InitOptLvlCbn();

		//MORPH START - Added by Stulle, Adjustable NT Service Strings
		GetDlgItem(IDC_SERVICE_STR_GROUP)->SetWindowText(GetResString(IDS_SERVICE_STR_GROUP));
		GetDlgItem(IDC_SERVICE_NAME_LABEL)->SetWindowText(GetResString(IDS_SERVICE_NAME));
		GetDlgItem(IDC_SERVICE_DISP_NAME_LABEL)->SetWindowText(GetResString(IDS_SERVICE_DISP_NAME));
		GetDlgItem(IDC_SERVICE_DESCR_LABEL)->SetWindowText(GetResString(IDS_SERVICE_DESCR));
		//MORPH END   - Added by Stulle, Adjustable NT Service Strings

		SetTool(IDC_SVC_INSTALLSERVICE,IDS_SVC_INSTALLSERVICE_TIP);
		SetTool(IDC_SVC_SERVERUNINSTALL,IDS_SVC_SERVERUNINSTALL_TIP);
		SetTool(IDC_SVC_STARTWITHSYSTEM,IDS_SVC_STARTWITHSYSTEM_TIP);
		SetTool(IDC_SVC_MANUALSTART,IDS_SVC_MANUALSTART_TIP);
		SetTool(IDC_SVC_SETTINGS,IDS_SVC_SETTINGS_TIP);
		SetTool(IDC_SVC_RUNBROWSER,IDS_SVC_RUNBROWSER_TIP);
		SetTool(IDC_SVC_REPLACESERVICE ,IDS_SVC_REPLACESERVICE_TIP);
		SetTool(IDC_SVC_CURRENT_STATUS,IDS_SVC_CURRENT_STATUS_TIP);
		SetTool(IDC_SERVICE_OPT_BOX,IDS_SERVICE_OPT_BOX_TIP);
		//MORPH START - Added by Stulle, Adjustable NT Service Strings
		SetTool(IDC_SERVICE_NAME,IDS_SERVICE_NAME_TIP);
		SetTool(IDC_SERVICE_DISP_NAME,IDS_SERVICE_DISP_NAME_TIP);
		SetTool(IDC_SERVICE_DESCR,IDS_SERVICE_DESCR_TIP);
		//MORPH END   - Added by Stulle, Adjustable NT Service Strings
	}
}

void CPPgNTService::OnHelp()
{
//heApp.ShowHelp();
}

BOOL CPPgNTService::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgNTService::OnHelpInfo(HELPINFO* /*pHelpInfo */)
{
	OnHelp();
	return TRUE;
}



int  CPPgNTService::FillStatus(){
	int b_installed;
	int  i_startupmode;
	int rights;
    /* no win98
	if (afxData.bWin95) {
		GetDlgItem( IDC_SVC_CURRENT_STATUS)->SetWindowText(GetResString(IDS_SVC_OSTOOOLD));
		GetDlgItem( IDC_SVC_INSTALLSERVICE)->EnableWindow(false); 
		GetDlgItem( IDC_SVC_SERVERUNINSTALL)->EnableWindow(false); 
        return -1;
	} 
	end no win98 vs2008*/
		NTServiceGet(b_installed,i_startupmode,	rights);

		if (RunningAsService())		{ 
			GetDlgItem( IDC_SVC_CURRENT_STATUS)->SetWindowText(GetResString(IDS_SVC_RUNNINGASSERVICE));
			GetDlgItem( IDC_SVC_INSTALLSERVICE)->EnableWindow(false); // installing makes no sense when already running
			GetDlgItem( IDC_SVC_SERVERUNINSTALL)->EnableWindow(false); // cannot uninstall self
			if ( i_startupmode==1){
				CheckDlgButton(IDC_SVC_STARTWITHSYSTEM, BST_CHECKED);
				CheckDlgButton(IDC_SVC_MANUALSTART,     BST_UNCHECKED);
			}
			else {
				CheckDlgButton(IDC_SVC_STARTWITHSYSTEM, BST_UNCHECKED);
				CheckDlgButton(IDC_SVC_MANUALSTART,     BST_CHECKED);
			}
		}
		else {//This instance is not the running process
			if (b_installed==-1)// undetermined
			{
				GetDlgItem( IDC_SVC_INSTALLSERVICE)->EnableWindow(true); // probably fails but let user try
				GetDlgItem( IDC_SVC_SERVERUNINSTALL)->EnableWindow(true); // probably fails but let user try
				GetDlgItem( IDC_SVC_CURRENT_STATUS)->SetWindowText(GetResString(IDS_SVC_ACCESSDENIED)); 
			}
			else if (b_installed==0){
				GetDlgItem( IDC_SVC_CURRENT_STATUS)->SetWindowText(GetResString(IDS_SVC_NOTINSTALLED)); 
				GetDlgItem( IDC_SVC_INSTALLSERVICE)->EnableWindow(true); 
				GetDlgItem( IDC_SVC_SERVERUNINSTALL)->EnableWindow(false);
			}	else if (b_installed==1 && i_startupmode ==4 ){
				GetDlgItem( IDC_SVC_CURRENT_STATUS)->SetWindowText(GetResString(IDS_SVC_INSTALLED_DISABLED)); 
				GetDlgItem( IDC_SVC_INSTALLSERVICE)->EnableWindow(false); 
				GetDlgItem( IDC_SVC_SERVERUNINSTALL)->EnableWindow(true);
			}	else if (b_installed==1){
				GetDlgItem( IDC_SVC_CURRENT_STATUS)->SetWindowText(GetResString(IDS_SVC_INSTALLED)); 
				GetDlgItem( IDC_SVC_INSTALLSERVICE)->EnableWindow(false); 
 				GetDlgItem( IDC_SVC_SERVERUNINSTALL)->EnableWindow(true);
			}
			if(i_startupmode==1){
				CheckDlgButton(IDC_SVC_STARTWITHSYSTEM, BST_CHECKED);
				CheckDlgButton(IDC_SVC_MANUALSTART,     BST_UNCHECKED);
			}
			else{
				CheckDlgButton(IDC_SVC_STARTWITHSYSTEM, BST_UNCHECKED);
				CheckDlgButton(IDC_SVC_MANUALSTART,     BST_CHECKED);
			}
		}
		return 0;
}


void CPPgNTService::OnBnClickedInstall()
{
	OnApply(); // MORPH - Added by Stulle, Adjustable NT Service Strings
	if (CmdInstallService((IsDlgButtonChecked(IDC_SVC_STARTWITHSYSTEM))==BST_CHECKED )==0)
	{
		FillStatus();
		if(AfxMessageBox(GetResString(IDS_APPLY_SETTINGS),MB_YESNO) == IDYES)
			OnBnAllSettings();
		SetModified();
		if (thePrefs.m_nCurrentUserDirMode == 0) // my documents and running as a service is not a good idea. but leave it to user
			AfxMessageBox(GetResString(IDS_CHANGEUSERASSERVICE),MB_OK);
	}
	else
		SetDlgItemText(IDC_SVC_CURRENT_STATUS,GetResString(IDS_SVC_INSTALLFAILED)); 
}


void CPPgNTService::OnBnClickedUnInstall()
{
	if (CmdRemoveService()==0) {
		FillStatus();
		SetModified();
	}
	else
		SetDlgItemText(IDC_SVC_CURRENT_STATUS,GetResString(IDS_SVC_UNINSTALLFAILED)); 

}

void CPPgNTService::OnBnStartSystem(){
	if(IsDlgButtonChecked(IDC_SVC_MANUALSTART)==BST_CHECKED )
		SetModified();
	CheckDlgButton(IDC_SVC_STARTWITHSYSTEM, BST_CHECKED);
	CheckDlgButton(IDC_SVC_MANUALSTART,     BST_UNCHECKED);
};	
void CPPgNTService::OnBnManualStart(){
	if(IsDlgButtonChecked(IDC_SVC_STARTWITHSYSTEM)==BST_CHECKED )
		SetModified();
	CheckDlgButton(IDC_SVC_STARTWITHSYSTEM,BST_UNCHECKED );
	CheckDlgButton(IDC_SVC_MANUALSTART,    BST_CHECKED);
};	

void CPPgNTService::OnBnAllSettings(){
	thePrefs.startupsound=false;
	thePrefs.m_bEnableMiniMule=false;
	thePrefs.splashscreen=false;
	thePrefs.startMinimized=true;
	thePrefs.bringtoforeground=false;
	if(theApp.emuledlg->preferenceswnd->m_wndGeneral)
	{
		if(theApp.emuledlg->preferenceswnd->m_wndGeneral.GetDlgItem(IDC_STARTUPSOUNDON))
			theApp.emuledlg->preferenceswnd->m_wndGeneral.CheckDlgButton(IDC_STARTUPSOUNDON,BST_UNCHECKED);
		if(theApp.emuledlg->preferenceswnd->m_wndGeneral.GetDlgItem(IDC_MINIMULE))
			theApp.emuledlg->preferenceswnd->m_wndGeneral.CheckDlgButton(IDC_MINIMULE,BST_UNCHECKED);
		if(theApp.emuledlg->preferenceswnd->m_wndGeneral.GetDlgItem(IDC_SPLASHON))
			theApp.emuledlg->preferenceswnd->m_wndGeneral.CheckDlgButton(IDC_SPLASHON,BST_UNCHECKED);
		if(theApp.emuledlg->preferenceswnd->m_wndGeneral.GetDlgItem(IDC_STARTMIN))
			theApp.emuledlg->preferenceswnd->m_wndGeneral.CheckDlgButton(IDC_STARTMIN,BST_CHECKED);
		if(theApp.emuledlg->preferenceswnd->m_wndGeneral.GetDlgItem(IDC_BRINGTOFOREGROUND))
			theApp.emuledlg->preferenceswnd->m_wndGeneral.CheckDlgButton(IDC_BRINGTOFOREGROUND,BST_UNCHECKED);
	}
	thePrefs.SetAutoConnect(true);
	if(theApp.emuledlg->preferenceswnd->m_wndConnection && theApp.emuledlg->preferenceswnd->m_wndConnection.GetDlgItem(IDC_AUTOCONNECT))
		theApp.emuledlg->preferenceswnd->m_wndConnection.CheckDlgButton(IDC_AUTOCONNECT,BST_CHECKED);
	thePrefs.SetWSIsEnabled(true); 
	if(theApp.emuledlg->preferenceswnd->m_wndWebServer && theApp.emuledlg->preferenceswnd->m_wndWebServer.GetDlgItem(IDC_WSENABLED))
	{
		theApp.emuledlg->preferenceswnd->m_wndWebServer.CheckDlgButton(IDC_WSENABLED,BST_CHECKED);
		theApp.emuledlg->preferenceswnd->m_wndWebServer.OnEnChangeWSEnabled();
	}
	RemAutoStart(); // remove from windows startup. 
	thePrefs.m_bSelCatOnAdd=false;
	if(theApp.emuledlg->preferenceswnd->m_wndMorph)
	{
		if(theApp.emuledlg->preferenceswnd->m_wndMorph.m_ctrlTreeOptions &&
			theApp.emuledlg->preferenceswnd->m_wndMorph.m_htiSelectCat)
			theApp.emuledlg->preferenceswnd->m_wndMorph.m_ctrlTreeOptions.SetCheckBoxEnable(theApp.emuledlg->preferenceswnd->m_wndMorph.m_htiSelectCat,BST_UNCHECKED);
		if(theApp.emuledlg->preferenceswnd->m_wndMorph.m_bSelectCat)
			theApp.emuledlg->preferenceswnd->m_wndMorph.m_bSelectCat = false;
	}
	thePrefs.notifierSoundType = ntfstNoSound;
	thePrefs.notifierOnDownloadFinished = false;
    thePrefs.notifierOnNewDownload = false;
    thePrefs.notifierOnChat = false;
    thePrefs.notifierOnLog = false;
	thePrefs.notifierOnImportantError = false;
    thePrefs.notifierOnEveryChatMsg = false;
	thePrefs.notifierOnNewVersion = false;
	if(theApp.emuledlg->preferenceswnd->m_wndNotify)
	{
		if(theApp.emuledlg->preferenceswnd->m_wndNotify.GetDlgItem(IDC_CB_TBN_USESOUND) &&
			theApp.emuledlg->preferenceswnd->m_wndNotify.GetDlgItem(IDC_CB_TBN_USESPEECH) &&
			theApp.emuledlg->preferenceswnd->m_wndNotify.GetDlgItem(IDC_CB_TBN_NOSOUND))
				theApp.emuledlg->preferenceswnd->m_wndNotify.CheckRadioButton(IDC_CB_TBN_NOSOUND, IDC_CB_TBN_USESPEECH, IDC_CB_TBN_NOSOUND);
		if(theApp.emuledlg->preferenceswnd->m_wndNotify.GetDlgItem(IDC_CB_TBN_ONDOWNLOAD))
			theApp.emuledlg->preferenceswnd->m_wndNotify.CheckDlgButton(IDC_CB_TBN_ONDOWNLOAD,BST_UNCHECKED);
		if(theApp.emuledlg->preferenceswnd->m_wndNotify.GetDlgItem(IDC_CB_TBN_ONNEWDOWNLOAD))
			theApp.emuledlg->preferenceswnd->m_wndNotify.CheckDlgButton(IDC_CB_TBN_ONNEWDOWNLOAD,BST_UNCHECKED);
		if(theApp.emuledlg->preferenceswnd->m_wndNotify.GetDlgItem(IDC_CB_TBN_ONCHAT))
			theApp.emuledlg->preferenceswnd->m_wndNotify.CheckDlgButton(IDC_CB_TBN_ONCHAT,BST_UNCHECKED);
		if(theApp.emuledlg->preferenceswnd->m_wndNotify.GetDlgItem(IDC_CB_TBN_ONLOG))
			theApp.emuledlg->preferenceswnd->m_wndNotify.CheckDlgButton(IDC_CB_TBN_ONLOG,BST_UNCHECKED);
		if(theApp.emuledlg->preferenceswnd->m_wndNotify.GetDlgItem(IDC_CB_TBN_IMPORTATNT))
			theApp.emuledlg->preferenceswnd->m_wndNotify.CheckDlgButton(IDC_CB_TBN_IMPORTATNT,BST_UNCHECKED);
		if(theApp.emuledlg->preferenceswnd->m_wndNotify.GetDlgItem(IDC_CB_TBN_POP_ALWAYS))
			theApp.emuledlg->preferenceswnd->m_wndNotify.CheckDlgButton(IDC_CB_TBN_POP_ALWAYS,BST_UNCHECKED);
		if(theApp.emuledlg->preferenceswnd->m_wndNotify.GetDlgItem(IDC_CB_TBN_ONNEWVERSION))
			theApp.emuledlg->preferenceswnd->m_wndNotify.CheckDlgButton(IDC_CB_TBN_ONNEWVERSION,BST_UNCHECKED);
	}
	thePrefs.beepOnError=false;
	if(theApp.emuledlg->preferenceswnd->m_wndTweaks)
	{
		if(theApp.emuledlg->preferenceswnd->m_wndTweaks.m_ctrlTreeOptions &&
			theApp.emuledlg->preferenceswnd->m_wndTweaks.m_htiBeeper)
			theApp.emuledlg->preferenceswnd->m_wndTweaks.m_ctrlTreeOptions.SetCheckBoxEnable(theApp.emuledlg->preferenceswnd->m_wndTweaks.m_htiBeeper,BST_UNCHECKED);
		if(theApp.emuledlg->preferenceswnd->m_wndTweaks.m_bBeeper)
			theApp.emuledlg->preferenceswnd->m_wndTweaks.m_bBeeper = false;
	}
//	SetModified();
};	

void CPPgNTService::OnBnReplaceStart(){
	CheckDlgButton(IDC_SVC_RUNBROWSER   ,BST_UNCHECKED );
	CheckDlgButton(IDC_SVC_REPLACESERVICE, BST_CHECKED);
	SetModified();
};	

void CPPgNTService::OnBnRunBRowser(){
	CheckDlgButton(IDC_SVC_RUNBROWSER   ,BST_CHECKED );
	CheckDlgButton(IDC_SVC_REPLACESERVICE,    BST_UNCHECKED);
	SetModified();
};	

void CPPgNTService::InitOptLvlCbn(bool bFirstInit)
{
	int iSel = m_cbOptLvl.GetCurSel();
	int iItem;
	m_cbOptLvl.ResetContent();
	iItem = m_cbOptLvl.AddString(_T("0: ") + GetResString(IDS_SERVICE_OPT_NONE));		m_cbOptLvl.SetItemData(iItem, SVC_NO_OPT);
	iItem = m_cbOptLvl.AddString(_T("2: ") + GetResString(IDS_SERVICE_OPT_BASIC));		m_cbOptLvl.SetItemData(iItem, SVC_BASIC_OPT);
	iItem = m_cbOptLvl.AddString(_T("6: ") + GetResString(IDS_SERVICE_OPT_LISTS));		m_cbOptLvl.SetItemData(iItem, SVC_LIST_OPT);
	iItem = m_cbOptLvl.AddString(_T("10: ") + GetResString(IDS_SERVICE_OPT_FULL));		m_cbOptLvl.SetItemData(iItem, SVC_FULL_OPT);

	if(bFirstInit)
	{
		bool bFound = false;
		for(int i = 0; i < m_cbOptLvl.GetCount(); i++)
		{
			if (m_cbOptLvl.GetItemData(i) == thePrefs.GetServiceOptLvl())
			{
				m_cbOptLvl.SetCurSel(i);
				bFound = true;
				break;
			}
		}
		if (!bFound) ASSERT(FALSE); // Should not happen
	}
	else
		m_cbOptLvl.SetCurSel(iSel != CB_ERR ? iSel : 0);
}

// MORPH start tabbed option [leuk_he]
void CPPgNTService::InitTab(bool firstinit, int Page)
{
	if (m_tabCtr.GetSafeHwnd() != NULL  && firstinit) {
		m_tabCtr.DeleteAllItems();
		m_tabCtr.SetImageList(&m_imageList);
		m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, WEBSERVER, GetResString(IDS_TAB_WEB_SERVER), 0, (LPARAM)WEBSERVER); 
		m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, MULTIWEBSERVER, GetResString(IDS_TAB_MULTI_USER), 0, (LPARAM)MULTIWEBSERVER);
		m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, NTSERVICE   , GetResString(IDS_TAB_NT_SERVICE), 0, (LPARAM)NTSERVICE);
	}
	if (m_tabCtr.GetSafeHwnd() != NULL)
		m_tabCtr.SetCurSel(Page);
}
void CPPgNTService::OnTcnSelchangeTab(NMHDR * /*pNMHDR */, LRESULT *pResult)
{
	int cur_sel = m_tabCtr.GetCurSel();
	theApp.emuledlg->preferenceswnd->SwitchTab(cur_sel);
	*pResult = 0;
}
// MORPH end tabbed option [leuk_he]