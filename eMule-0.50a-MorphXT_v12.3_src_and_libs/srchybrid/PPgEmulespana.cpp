//this file is part of eMule
//Copyright (C)2002-2006 ( merkur-@users.sourceforge.net / http://www.emule-project.net )
//
// PPgEmulespana.cpp - emulEspaña Mod: Added by Announ
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
#include "PPGtooltipped.h" //MORPH leuk_he addded tooltipped
#include "emule.h"
#include "PPgEmulespana.h"
/*Commented by SiRoB
#include "../CxImage/ximage.h"
*/
#include "OtherFunctions.h"
#include "Preferences.h"
#include "UserMsgs.h"

/*Commented by SiRoB
// added by MoNKi [MoNKi: -invisible mode-]
#include "TreeOptsPrefs\TreeOptionsInvisibleModCombo.h"
#include "TreeOptsPrefs\TreeOptionsInvisibleKeyCombo.h"
#include ".\ppgemulespana.h"
// End MoNKi
*/

// added by MoNKi [MoNKi: -Wap Server-]
#include "TreeOptsPrefs\PassTreeOptionsEdit.h"
#include "emuledlg.h"
#include "serverwnd.h"
#define HIDDEN_PASSWORD _T("*****")
// End MoNKi

/*Commented by SiRoB
// added by MoNKi [MoNKi: -Skin Selector-]
//#include "emuledlg.h"
#include "MuleToolbarCtrl.h"
// End MoNKi
*/

// Cuadro de diálogo de CPPgEmulespana

IMPLEMENT_DYNAMIC(CPPgEmulespana, CPropertyPage)
CPPgEmulespana::CPPgEmulespana()
/* MORPH START leuk_he tooltipped
	: CPropertyPage(CPPgEmulespana::IDD)
*/
	: CPPgtooltipped(CPPgEmulespana::IDD)
// MORPH END leuk_he tooltipped
	, m_ctrlTreeOptions(theApp.m_iDfltImageListColorFlags)	
{
	m_bInitializedTreeOpts = false;

/*Commented by SiRoB
	// added by MoNKi [MoNKi: -invisible mode-]
	m_htiInvisibleModeRoot = NULL;
	m_htiInvisibleMode = NULL;
	m_htiInvisibleModeMod = NULL;
	m_htiInvisibleModeKey = NULL;
	m_bInvisibleMode = false;
	m_sInvisibleModeMod = "";
	m_sInvisibleModeKey = "";
	m_iInvisibleModeActualKeyModifier = 0;
	// End MoNKi
*/
	// Added by MoNKi [MoNKi: -UPnPNAT Support-]
	m_htiUPnPGroup = NULL;
//	m_htiUPnP = NULL;
//	m_htiUPnPWeb = NULL;
	//m_htiUPnPTryRandom = NULL;
//	m_bUPnP = false;
//	m_bUPnPWeb = false;
	// MORPH START leuk_he upnp bindaddr    	
    m_dwUpnpBindAddr = 0; 
    m_htiUpnpBinaddr = NULL;
	// MORPH END leuk_he upnp bindaddr    	
	m_iUPnPPort=0;
	m_bUPnPClearOnClose=TRUE;
    m_bUPnPLimitToFirstConnection=false;
	m_iDetectuPnP=0;
	m_htiUPnPPort=NULL;
	m_htiUPnPClearOnClose=NULL;
    m_htiUPnPLimitToFirstConnection=NULL;;
	m_htiDetectuPnP=NULL;
	m_bUPnPForceUpdate=false;
	m_htiUPnPForceUpdate=NULL;
	//m_bUPnPTryRandom = false;
	// End MoNKi

/*Commented by SiRoB
	// Added by MoNKi [MoNKi: -Support for High Contrast Mode-]
	m_htiHighContrast = NULL;
	m_htiHighContrastDisableSkins = NULL;
	m_bHighContrast = false;
	m_bHighContrastDisableSkins = false;
	// End MoNKi
*/
	// Added by by MoNKi [MoNKi: -Improved ICS-Firewall support-]
	m_htiICFSupportRoot = NULL;
	m_htiICFSupport = NULL;
	m_htiICFSupportClearAtEnd = NULL;
	m_bICFSupport = false;
	m_bICFSupportClearAtEnd = false;
	// End MoNKi

	// Added by MoNKi [ SlugFiller: -lowIdRetry- ]
	m_htiLowIdRetry = NULL;
	m_iLowIdRetry = 0;
	// End SlugFiller

	// Added by MoNKi [ MoNKi: -Wap Server- ]
	m_htiWapRoot = NULL;
	m_htiWapEnable = NULL;
	m_htiWapPort = NULL;
	m_htiWapTemplate = NULL;
	m_htiWapPass = NULL;
	m_htiWapLowEnable = NULL;
	m_htiWapLowPass = NULL;
	m_bWapEnable = false;
	m_iWapPort = 0;
	m_sWapTemplate = "";
	m_sWapPass = "";
	m_bWapLowEnable = false;
	m_sWapLowPass = "";
	// End MoNKi

/*Commented by SiRoB
	// Added by MoNKi [MoNKi: -USS initial TTL-]
	m_htiUSSRoot = NULL;
	m_htiUSSTTL = NULL;
	m_iUSSTTL = 0;
	// End MoNKi
*/
	// Added by MoNKi [MoNKi: -Random Ports-]
	m_htiRandomPortsResetTime = NULL;
	m_iRandomPortsResetTime = 0;
	// End MoNKi

/*Commented by SiRoB
	// Added by MoNKi [MoNKi: -Custom incoming folder icon-]
	m_htiCustomIncomingIcon = NULL;
	m_bCustomIncomingIcon = false;
	// End MoNKi

    */
}

CPPgEmulespana::~CPPgEmulespana()
{
}

void CPPgEmulespana::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EXT_OPTS, m_ctrlTreeOptions);
	if ( !m_bInitializedTreeOpts )
	{
		CImageList* piml = m_ctrlTreeOptions.GetImageList(TVSIL_NORMAL);

		// Added by MoNKi [ SlugFiller: -lowIdRetry- ]
		m_htiLowIdRetry = m_ctrlTreeOptions.InsertItem(_T("LowID retries"), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htiLowIdRetry, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// End SlugFiller

/*Commented by SiRoB
		// Added by MoNKi [MoNKi: -Custom incoming folder icon-]
		m_htiCustomIncomingIcon  = m_ctrlTreeOptions.InsertCheckBox(_T("Custom incoming folder icon"), TVI_ROOT, m_bCustomIncomingIcon);
		// End MoNKi
*/

		// Added by MoNKi [MoNKi: -Random Ports-]
		m_htiRandomPortsResetTime = m_ctrlTreeOptions.InsertItem(_T("Random ports safe restart time"), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htiRandomPortsResetTime, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// End MoNKi

/*Commented by SiRoB
		// Added by MoNKi [MoNKi: -invisible mode-]
		int iImgInvisibleMode = 8; // default icon
		if (piml){
			iImgInvisibleMode = piml->Add(CTempIconLoader(_T("TrayLowID")));
		}
		m_htiInvisibleModeRoot = m_ctrlTreeOptions.InsertItem(_T("Invisible Mode"), iImgInvisibleMode, iImgInvisibleMode, TVI_ROOT);
		m_htiInvisibleMode = m_ctrlTreeOptions.InsertCheckBox(_T("Enable Invisible Mode"), m_htiInvisibleModeRoot, m_bInvisibleMode);
		m_htiInvisibleModeMod = m_ctrlTreeOptions.InsertItem(_T("Key Modifier"), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiInvisibleModeRoot);
		m_ctrlTreeOptions.AddComboBox(m_htiInvisibleModeMod, RUNTIME_CLASS(CTreeOptionsInvisibleModCombo));
		m_htiInvisibleModeKey = m_ctrlTreeOptions.InsertItem(_T("Key"), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiInvisibleModeRoot);
		m_ctrlTreeOptions.AddComboBox(m_htiInvisibleModeKey, RUNTIME_CLASS(CTreeOptionsInvisibleKeyCombo));
		m_ctrlTreeOptions.Expand(m_htiInvisibleModeRoot, TVE_EXPAND);
		// End MoNKi
*/
		// Added by by MoNKi [MoNKi: -Improved ICS-Firewall support-]
		int iImgICF = 8; // default icon
		if (piml){
			iImgICF = piml->Add(CTempIconLoader(_T("PROXY")));
		}
		m_htiICFSupportRoot = m_ctrlTreeOptions.InsertGroup(_T("Internet Connection Firewall (ICF)"), iImgICF,  TVI_ROOT);// leuk_he: item -> group
		m_htiICFSupport = m_ctrlTreeOptions.InsertCheckBox(_T("Enable Windows Internet Connection Firewall (ICF) support"), m_htiICFSupportRoot, m_bICFSupport);
		m_htiICFSupportClearAtEnd = m_ctrlTreeOptions.InsertCheckBox(_T("Clear mappings at end"), m_htiICFSupportRoot, m_bICFSupportClearAtEnd);
		m_htiICFSupportServerUDP = m_ctrlTreeOptions.InsertCheckBox(_T("Add mapping for \"ServerUDP\" port"), m_htiICFSupportRoot, m_bICFSupportServerUDP);
		m_ctrlTreeOptions.Expand(m_htiICFSupportRoot, TVE_EXPAND);
		// End MoNKi

		// Added by MoNKi [MoNKi: -UPnPNAT Support-]
		int iImgUPnP = 8; // default icon
		if (piml){
			iImgUPnP = piml->Add(CTempIconLoader(_T("UPNP")));
		}
		m_htiUPnPGroup = m_ctrlTreeOptions.InsertGroup(_T("Universal Plug & Play (UPnP)"), iImgUPnP,  TVI_ROOT); // leuk_he item ->group
//		m_htiUPnP = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_UPNP_ENABLE), m_htiUPnPGroup, m_bUPnP);
//  		m_htiUPnPWeb = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_UPNP_ENABLEWEB), m_htiUPnPGroup, m_bUPnPWeb);
		// MORPH START leuk_he upnp bindaddr
         m_htiUpnpBinaddr =	 m_ctrlTreeOptions.InsertItem(GetResString(IDS_UPNPBINDADDR), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiUPnPGroup);
		 m_ctrlTreeOptions.AddIPAddress(m_htiUpnpBinaddr , RUNTIME_CLASS(CTreeOptionsIPAddressCtrl));
        //MORPH END leuk_he upnp binaddr
 		m_htiUPnPPort= m_ctrlTreeOptions.InsertItem(GetResString(IDS_PORT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiUPnPGroup);
        m_ctrlTreeOptions.AddEditBox(m_htiUPnPPort, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiUPnPClearOnClose = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_UPNPCLOSEONEXIT), m_htiUPnPGroup, m_bUPnPClearOnClose);
		m_htiUPnPLimitToFirstConnection = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_UPNPLIMITTOFIRSTCONNECTION ), m_htiUPnPGroup, m_bUPnPLimitToFirstConnection);
		m_htiDetectuPnP= m_ctrlTreeOptions.InsertItem(GetResString(IDS_UPNPDETECTSTATUS), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiUPnPGroup);
        m_ctrlTreeOptions.AddEditBox(m_htiDetectuPnP, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiUPnPForceUpdate= m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_UPNFORCEUPDATE), m_htiUPnPGroup, m_bUPnPForceUpdate);

		m_ctrlTreeOptions.Expand(m_htiUPnPGroup, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiUPnP, TVE_EXPAND);
		// End MoNKi

		// Added by MoNKi [MoNKi: -Wap server-]
		int iImgWap = 8; // default icon
		if (piml){
			iImgWap = piml->Add(CTempIconLoader(_T("MOBILE")));
		}
		m_htiWapRoot = m_ctrlTreeOptions.InsertGroup(_T("Wap Interface"), iImgWap,  TVI_ROOT); // leuk_he item -> group
		m_htiWapEnable  = m_ctrlTreeOptions.InsertCheckBox(_T("Enable Wap Interface"), m_htiWapRoot, m_bWapEnable);
		m_htiWapTemplate = m_ctrlTreeOptions.InsertItem(_T("Template"), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiWapRoot);
		m_ctrlTreeOptions.AddFileEditBox(m_htiWapTemplate,RUNTIME_CLASS(CTreeOptionsEdit), RUNTIME_CLASS(CTreeOptionsBrowseButton));
		m_htiWapPort = m_ctrlTreeOptions.InsertItem(_T("Port"), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiWapRoot);
		m_ctrlTreeOptions.AddEditBox(m_htiWapPort, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiWapPass = m_ctrlTreeOptions.InsertItem(_T("Pass"), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiWapRoot);
		m_ctrlTreeOptions.AddPassEditBox(m_htiWapPass, RUNTIME_CLASS(CPassTreeOptionsEdit));
		m_htiWapLowEnable  = m_ctrlTreeOptions.InsertCheckBox(_T("Low Enable"), m_htiWapRoot, m_bWapLowEnable);
		m_htiWapLowPass = m_ctrlTreeOptions.InsertItem(_T("Low Pass"), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiWapRoot);
		m_ctrlTreeOptions.AddPassEditBox(m_htiWapLowPass, RUNTIME_CLASS(CPassTreeOptionsEdit));

		m_ctrlTreeOptions.Expand(m_htiWapRoot, TVE_EXPAND);

		// End MoNKi

/*Commented by SiRoB
		// Added by MoNKi [MoNKi: -Support for High Contrast Mode-]
		int iImgContrast = 8; // default icon
		if (piml){
			iImgContrast = piml->Add(CTempIconLoader(_T("CONTRAST")));
		}
		m_htiHighContrastRoot = m_ctrlTreeOptions.InsertItem(_T("High contrast support"), iImgContrast, iImgContrast, TVI_ROOT);
		m_htiHighContrast  = m_ctrlTreeOptions.InsertCheckBox(_T("Enable high contrast support"), m_htiHighContrastRoot, m_bHighContrast);
		m_htiHighContrastDisableSkins = m_ctrlTreeOptions.InsertCheckBox(_T("Disable skins on high contrast"), m_htiHighContrastRoot, m_bHighContrastDisableSkins);
		m_ctrlTreeOptions.Expand(m_htiHighContrastRoot, TVE_EXPAND);
		// End MoNKi
*/
/*Commented by SiRoB
		// Added by MoNKi [MoNKi: -USS initial TTL-]
		int iImgUSS = 8; // default icon
		if (piml){
			iImgUSS = piml->Add(CTempIconLoader(_T("upload")));
		}
		m_htiUSSRoot = m_ctrlTreeOptions.InsertItem(GetResString(IDS_DYNUP), iImgUSS, iImgUSS, TVI_ROOT);
		m_htiUSSTTL = m_ctrlTreeOptions.InsertItem(_T("Initial TTL"), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiUSSRoot);
		m_ctrlTreeOptions.AddEditBox(m_htiUSSTTL, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_ctrlTreeOptions.Expand(m_htiUSSRoot, TVE_EXPAND);
		// End MoNKi
*/
		m_ctrlTreeOptions.SelectItem(m_ctrlTreeOptions.GetRootItem());
		m_bInitializedTreeOpts = true;
	}

/*Commented by SiRoB
	// Added by MoNKi [MoNKi: -invisible mode-]
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiInvisibleMode, m_bInvisibleMode);
	DDX_TreeCombo(pDX, IDC_EXT_OPTS, m_htiInvisibleModeMod, m_sInvisibleModeMod);
	DDX_TreeCombo(pDX, IDC_EXT_OPTS, m_htiInvisibleModeKey, m_sInvisibleModeKey);
	// End MoNKi
*/
	// Added by MoNKi [MoNKi: -UPnPNAT Support-]
	//DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiUPnP, m_bUPnP);
	//DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiUPnPTryRandom, m_bUPnPTryRandom);
//	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiUPnPWeb, m_bUPnPWeb);
	// MORPH start leuke_he upnp bindaddr
	DDX_TreeIPAddress(pDX, IDC_EXT_OPTS,m_htiUpnpBinaddr  , m_dwUpnpBindAddr);
  	// MORPH end leuke_he upnp bindaddr
		DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiUPnPPort, m_iUPnPPort);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiUPnPClearOnClose, m_bUPnPClearOnClose);
//	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiUPnPWeb, m_bUPnPWeb);
	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiDetectuPnP, m_iDetectuPnP);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiUPnPForceUpdate, m_bUPnPForceUpdate);

	//m_ctrlTreeOptions.SetCheckBoxEnable(m_htiUPnPTryRandom, m_bUPnP);
	// End MoNKi

/*Commented by SiRoB
	// Added by MoNKi [MoNKi: -Support for High Contrast Mode-]
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiHighContrast, m_bHighContrast);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiHighContrastDisableSkins, m_bHighContrastDisableSkins);
	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiHighContrastDisableSkins, m_bHighContrast);
	// End MoNKi
*/
	// Added by by MoNKi [MoNKi: -Improved ICS-Firewall support-]
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiICFSupport, m_bICFSupport);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiICFSupportClearAtEnd, m_bICFSupportClearAtEnd);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiICFSupportServerUDP, m_bICFSupportServerUDP);
	// End MoNKi

/*Commented by SiRoB
	// Added by by MoNKi [MoNKi: -Skin Selector-]
	DDX_Control(pDX, IDC_TAB_EMULESPANA_PREFS, m_tabPrefs);
	DDX_Control(pDX, IDC_LIST_SKINS, m_listSkins);
	DDX_Control(pDX, IDC_LIST_TB_SKINS, m_listTBSkins);
	DDX_Control(pDX, IDC_EDIT_SKINS_DIR, m_editSkinsDir);
	// End MoNKi
*/
	// Added by MoNKi [ SlugFiller: -lowIdRetry- ]
	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiLowIdRetry, m_iLowIdRetry);
	DDV_MinMaxInt(pDX, m_iLowIdRetry, 0, 255);
	// End SlugFiller

	// Added by MoNKi [MoNKi: -Random Ports-]
	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiRandomPortsResetTime, m_iRandomPortsResetTime);
	DDV_MinMaxInt(pDX, m_iRandomPortsResetTime, 0, 900);
	// End MoNKi

/*Commented by SiRoB
	// Added by MoNKi [MoNKi: -USS initial TTL-]
	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiUSSTTL, m_iUSSTTL);
	DDV_MinMaxInt(pDX, m_iUSSTTL, 1, 20);
	// End MoNKi
*/
	// Added by MoNKi [ MoNKi: -Wap Server- ]
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiWapEnable, m_bWapEnable);
	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiWapPort, m_iWapPort);
	DDV_MinMaxInt(pDX, m_iWapPort, 0, 0xFFFF);
	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiWapTemplate, m_sWapTemplate);
	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiWapPass, m_sWapPass);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiWapLowEnable, m_bWapLowEnable);
	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiWapLowPass, m_sWapLowPass);
	// end MoNKi

/*Commented by SiRoB
	// Added by MoNKi [MoNKi: -Custom incoming folder icon-]
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiCustomIncomingIcon, m_bCustomIncomingIcon);
	// End MoNKi
*/
}


BEGIN_MESSAGE_MAP(CPPgEmulespana, CPropertyPage)
/*Commented by SiRoB
	ON_WM_PAINT()
*/
	ON_MESSAGE(UM_TREEOPTSCTRL_NOTIFY, OnTreeOptsCtrlNotify)
	ON_WM_DESTROY()
/*Commented by SiRoB
	 // Added by by MoNKi [MoNKi: -Skin Selector-]
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_EMULESPANA_PREFS, OnTcnSelchangeTabEmulespanaPrefs)
	ON_BN_CLICKED(IDC_BTN_SKINS_DIR, OnBnClickedBtnSkinsDir)
	ON_BN_CLICKED(IDC_BTN_SKINS_RELOAD, OnBnClickedBtnSkinsReload)
	ON_EN_CHANGE(IDC_EDIT_SKINS_DIR, OnEnChangeEditSkinsDir)
	ON_WM_SYSCOLORCHANGE()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_SKINS, OnLvnItemchangedListSkins)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_TB_SKINS, OnLvnItemchangedListTbSkins)
	// End MoNKi
*/
	ON_WM_HELPINFO()
END_MESSAGE_MAP()


// Controladores de mensajes de CPPgEmulespana

/*Commented by SiRoB
void CPPgEmulespana::OnPaint()
{
	if(thePrefs.m_bICFSupportStatusChanged){
		m_ctrlTreeOptions.SetCheckBox(m_htiICFSupport,thePrefs.GetICFSupport());
		m_bICFSupport = thePrefs.GetICFSupport();
		thePrefs.m_bICFSupportStatusChanged = false;
	}

	CPaintDC dc(this); // device context for painting
	CxImage cImage;
	CRect rc;

	cImage.LoadResource(FindResource(NULL,MAKEINTRESOURCE(IDR_EE_PREFS_PNG),_T("PNG")), CXIMAGE_FORMAT_PNG);
	rc.SetRect(10,10, 10 + cImage.GetWidth(), 10 + cImage.GetHeight());
	cImage.Draw(dc.m_hDC,rc);
}
*/

void CPPgEmulespana::Localize()
{
	if ( m_hWnd )
	{
		GetDlgItem(IDC_WARNING)->SetWindowText(GetResString(IDS_WARNINGMORPH));
/*Commented by SiRoB
		SetWindowText(GetResString(IDS_PW_EMULESPANA));

		// Added by MoNKi [MoNKi: -invisible mode-]
		if ( m_htiInvisibleModeRoot )
		{
			m_ctrlTreeOptions.SetItemText(m_htiInvisibleMode, GetResString(IDS_INVMODE));
			m_ctrlTreeOptions.SetItemText(m_htiInvisibleModeMod, GetResString(IDS_INVMODE_MODKEY));
			m_ctrlTreeOptions.SetItemText(m_htiInvisibleModeKey, GetResString(IDS_INVMODE_VKEY));

			m_sInvisibleModeMod = "";
			if (m_iInvisibleModeActualKeyModifier & MOD_CONTROL)
				m_sInvisibleModeMod=GetResString(IDS_CTRLKEY);
			if (m_iInvisibleModeActualKeyModifier & MOD_ALT){
				if (!m_sInvisibleModeMod.IsEmpty()) m_sInvisibleModeMod += " + ";
				m_sInvisibleModeMod+=GetResString(IDS_ALTKEY);
			}
			if (m_iInvisibleModeActualKeyModifier & MOD_SHIFT){
				if (!m_sInvisibleModeMod.IsEmpty()) m_sInvisibleModeMod += " + ";
				m_sInvisibleModeMod+=GetResString(IDS_SHIFTKEY);
			}

			m_ctrlTreeOptions.SetComboText(m_htiInvisibleModeMod, m_sInvisibleModeMod);		
			m_ctrlTreeOptions.SetComboText(m_htiInvisibleModeKey, m_sInvisibleModeKey);

			if(m_ctrlTreeOptions.GetCheckBox(m_htiInvisibleMode, m_bInvisibleMode)){
				if(m_bInvisibleMode)
					m_ctrlTreeOptions.SetItemText(m_htiInvisibleModeRoot, GetResString(IDS_INVMODE_GROUP) + 
						_T(" (") + m_sInvisibleModeMod + _T(" + ") + m_sInvisibleModeKey + _T(")"));
				else
					m_ctrlTreeOptions.SetItemText(m_htiInvisibleModeRoot, GetResString(IDS_INVMODE_GROUP));
			}
		}
		// End MoNKi
*/

		// Added by MoNKi [MoNKi: -Random Ports-]
		if(m_htiRandomPortsResetTime)
			m_ctrlTreeOptions.SetEditLabel(m_htiRandomPortsResetTime, GetResString(IDS_RANDOMPORTSRESETTIME));
		// End MoNKi

		// Added by MoNKi [MoNKi: -UPnPNAT Support-]
		if (m_htiUPnPGroup) m_ctrlTreeOptions.SetItemText(m_htiUPnPGroup, GetResString(IDS_UPNP));
		if (m_htiUPnP) m_ctrlTreeOptions.SetItemText(m_htiUPnP, GetResString(IDS_UPNP_ENABLE));
//		if (m_htiUPnPWeb) m_ctrlTreeOptions.SetItemText(m_htiUPnPWeb, GetResString(IDS_UPNP_ENABLEWEB));
         //MORPH START leuk_he upnp bindaddr
		if (m_htiUpnpBinaddr) m_ctrlTreeOptions.SetEditLabel(m_htiUpnpBinaddr, GetResString(IDS_UPNPBINDADDR));
		//MORPH END leuk_he upnp bindaddr


		// End MoNKi
		 // MORPH START leuk_he tooltipped
		SetTool(m_htiLowIdRetry ,LOWIDRETRIES);
        SetTool(m_htiRandomPortsResetTime ,RANDOMPORTTIME_TIP);
        SetTool(m_htiICFSupportRoot,ICPCONN_TIP);
        SetTool(m_htiICFSupport, ENABLE_TIP);
        SetTool(m_htiICFSupportClearAtEnd,CLEARMAPTIP);
        SetTool(m_htiICFSupportServerUDP,ADDMPATIP);
        SetTool(m_htiUPnPGroup ,UPNP_GROUP_TIP);
        //SetTool(m_htiUPnP,IDS_UPNP_ENABLE_TIP);
//        SetTool(m_htiUPnPWeb,IDS_UPNP_ENABLEWEB_TIP);
        SetTool(m_htiUpnpBinaddr,IDS_UPNPBINDADDR_TIP);
        SetTool(m_htiWapRoot,WAP_TIP);
        SetTool(m_htiWapEnable ,ENABLEWAP_TIP);
        SetTool(m_htiWapTemplate,TEMPLATE_TIP);
        SetTool(m_htiWapPort,WPAPORT_TIP);
        SetTool(m_htiWapPass,WAPPASS_TIP);
        SetTool(m_htiWapLowEnable ,WAPLOW1_TIP);
        SetTool(m_htiWapLowPass,WAPLOWPASS_TIP);

		SetTool(m_htiUPnPPort,IDS_UPNPPORT_TIP);
		SetTool(m_htiUPnPClearOnClose,IDS_UPNPCLEARONCLOSE_TIP);
		SetTool(m_htiUPnPLimitToFirstConnection,IDS_UPNPLIMITTOFIRSTCONNECTION_TIP);
		SetTool(m_htiDetectuPnP,IDS_DETECTUPNP_TIP);
		SetTool(m_htiUPnPForceUpdate,IDS_UPNFORCEUPDATE_TIP);

     //MORPH END leuk_he tooltipped
	

/*Commented by SiRoB
		// Added by MoNKi [MoNKi: -Support for High Contrast Mode-]
		if ( m_htiHighContrastRoot ){
			m_ctrlTreeOptions.SetItemText(m_htiHighContrastRoot, GetResString(IDS_HIGHCONTRASTROOT));
			m_ctrlTreeOptions.SetItemText(m_htiHighContrast, GetResString(IDS_HIGHCONTRAST));
			m_ctrlTreeOptions.SetItemText(m_htiHighContrastDisableSkins, GetResString(IDS_HIGHCONTRASTDISABLESKINS));
		}
		// End MoNKi		
*/
		// Added by by MoNKi [MoNKi: -Improved ICS-Firewall support-]
		if (m_htiICFSupport){
			m_ctrlTreeOptions.SetItemText(m_htiICFSupportRoot, GetResString(IDS_ICF));
			m_ctrlTreeOptions.SetItemText(m_htiICFSupport, GetResString(IDS_ICFSUPPORT));
			m_ctrlTreeOptions.SetItemText(m_htiICFSupportClearAtEnd, GetResString(IDS_FO_PREF_STARTUP));
			m_ctrlTreeOptions.SetItemText(m_htiICFSupportServerUDP, GetResString(IDS_ICF_SERVERUDP));
		}
		// End MoNKi

/*Commented by SiRoB
		// Added by by MoNKi [MoNKi: -Skin Selector-]
		TCITEM tabItem;
		tmpString = GetResString(IDS_EM_PREFS); 
		tmpString.Remove('&'); 
		tabItem.mask = TCIF_TEXT;
		tabItem.pszText = tmpString.GetBuffer();
		m_tabPrefs.SetItem(0, &tabItem);
		tmpString.ReleaseBuffer();
		tmpString = GetResString(IDS_TOOLBARSKINS);
		tabItem.pszText = tmpString.GetBuffer();
		m_tabPrefs.SetItem(1, &tabItem);
		tmpString.ReleaseBuffer();
		tmpString = GetResString(IDS_SKIN_PROF);
		tabItem.pszText = tmpString.GetBuffer();
		m_tabPrefs.SetItem(2, &tabItem);
		tmpString.ReleaseBuffer();
		GetDlgItem(IDC_BTN_SKINS_RELOAD)->SetWindowText(GetResString(IDS_SF_RELOAD));
		if(m_tabPrefs.GetCurSel() == 1)
			GetDlgItem(IDC_SKINS_LBL)->SetWindowText(GetResString(IDS_SELECTTOOLBARBITMAPDIR));
		else if (m_tabPrefs.GetCurSel() == 2)
			GetDlgItem(IDC_SKINS_LBL)->SetWindowText(GetResString(IDS_SEL_SKINDIR));
		
		m_listTBSkins.Localize();
		m_listSkins.Localize();
		// End MoNKi
*/
		// Added by MoNKi [ SlugFiller: -lowIdRetry- ]
		if(m_htiLowIdRetry)
			m_ctrlTreeOptions.SetEditLabel(m_htiLowIdRetry, GetResString(IDS_RECONNECTONLOWID));
		// End SlugFiller

		// Added by MoNKi [ MoNKi: -Wap Server- ]
		if(m_htiWapRoot){
			m_ctrlTreeOptions.SetItemText(m_htiWapRoot, GetResString(IDS_PW_WAP));
			m_ctrlTreeOptions.SetItemText(m_htiWapEnable, GetResString(IDS_ENABLED));
			m_ctrlTreeOptions.SetEditLabel(m_htiWapPort, GetResString(IDS_PORT));
			m_ctrlTreeOptions.SetEditLabel(m_htiWapTemplate, GetResString(IDS_WS_RELOAD_TMPL));
			m_ctrlTreeOptions.SetEditLabel(m_htiWapPass, GetResString(IDS_WS_PASS));
			m_ctrlTreeOptions.SetItemText(m_htiWapLowEnable, GetResString(IDS_WEB_LOWUSER));
			m_ctrlTreeOptions.SetEditLabel(m_htiWapLowPass, GetResString(IDS_WS_PASS));
		}
		// End MoNKi

/*Commented by SiRoB
		// Added by MoNKi [MoNKi: -USS initial TTL-]
		if(m_htiUSSRoot){
			m_ctrlTreeOptions.SetEditLabel(m_htiUSSTTL, GetResString(IDS_USS_INITIAL_TTL));
		}
		// End MoNKi

		// Added by MoNKi [MoNKi: -Custom incoming folder icon-]
		m_ctrlTreeOptions.SetItemText(m_htiCustomIncomingIcon, GetResString(IDS_CUSTOM_INCOMING_ICON));
		// End MoNKi
*/
	}
}

LRESULT CPPgEmulespana::OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM /*lParam*/)
{
	if (wParam == IDC_EXT_OPTS)
	{
/*Commented by SiRoB
		TREEOPTSCTRLNOTIFY* pton = (TREEOPTSCTRLNOTIFY*)lParam;

		// Added by MoNKi [MoNKi: -invisible mode-]
		if (pton->hItem == m_htiInvisibleModeMod)
		{
			CTreeOptionsInvisibleModCombo* modCombo = (CTreeOptionsInvisibleModCombo*)pton->nmhdr.code;
			if (modCombo)
				modCombo->GetLBText(modCombo->GetCurSel(),m_sInvisibleModeMod);

			m_iInvisibleModeActualKeyModifier = 0;
			if (m_sInvisibleModeMod.Find(GetResString(IDS_CTRLKEY))!=-1)
				m_iInvisibleModeActualKeyModifier |= MOD_CONTROL;
			if (m_sInvisibleModeMod.Find(GetResString(IDS_ALTKEY))!=-1)
				m_iInvisibleModeActualKeyModifier |= MOD_ALT;
			if (m_sInvisibleModeMod.Find(GetResString(IDS_SHIFTKEY))!=-1)
				m_iInvisibleModeActualKeyModifier |= MOD_SHIFT;
		}

		if (pton->hItem == m_htiInvisibleModeKey)
		{
			CTreeOptionsInvisibleModCombo* keyCombo = (CTreeOptionsInvisibleModCombo*)pton->nmhdr.code;
			if (keyCombo)
				keyCombo->GetLBText(keyCombo->GetCurSel(),m_sInvisibleModeKey);
		}

		if(m_ctrlTreeOptions.GetCheckBox(m_htiInvisibleMode, m_bInvisibleMode)){
			if(m_bInvisibleMode)
				m_ctrlTreeOptions.SetItemText(m_htiInvisibleModeRoot, GetResString(IDS_INVMODE_GROUP) + 
					_T(" (") + m_sInvisibleModeMod + _T(" + ") + m_sInvisibleModeKey + _T(")"));
			else
				m_ctrlTreeOptions.SetItemText(m_htiInvisibleModeRoot, GetResString(IDS_INVMODE_GROUP));
		}
		// End MoNKi
*/
		// Added by MoNKi [MoNKi: -UPnPNAT Support-]
		/*if (pton->hItem == m_htiUPnP){
			if(m_ctrlTreeOptions.GetCheckBox(m_htiUPnP, m_bUPnP)){
				m_ctrlTreeOptions.SetCheckBoxEnable(m_htiUPnPTryRandom, m_bUPnP);
			}
		}*/
		// End MoNKi

/*Commented by SiRoB
		// Added by MoNKi [MoNKi: -Support for High Contrast Mode-]
		if (pton->hItem == m_htiHighContrast){
			if(m_ctrlTreeOptions.GetCheckBox(m_htiHighContrast, m_bHighContrast)){
				m_ctrlTreeOptions.SetCheckBoxEnable(m_htiHighContrastDisableSkins, m_bHighContrast);
			}
		}
		// End MoNKi
*/
		SetModified();
	}
	return 0;
}

void CPPgEmulespana::OnDestroy()
{
	m_ctrlTreeOptions.DeleteAllItems();
	m_ctrlTreeOptions.DestroyWindow();
	m_bInitializedTreeOpts = false;

/*Commented by SiRoB
	// Added by MoNKi [MoNKi: -invisible mode-]
	m_htiInvisibleModeRoot = NULL;
	m_htiInvisibleMode = NULL;
	m_htiInvisibleModeMod = NULL;
	m_htiInvisibleModeKey = NULL;
	// End MoNKi
*/
	// Added by MoNKi [MoNKi: -UPnPNAT Support-]
	m_htiUPnPGroup = NULL;
	m_htiUPnP = NULL;
//	m_htiUPnPWeb = NULL;
	//m_htiUPnPTryRandom = NULL;
	// End MoNKi

/*Commented by SiRoB
	// Added by MoNKi [MoNKi: -Support for High Contrast Mode-]
	m_htiHighContrast = NULL;
	m_htiHighContrastDisableSkins = NULL;
	// End MoNKi

	// Added by by MoNKi [MoNKi: -Improved ICS-Firewall support-]
	m_htiICFSupportRoot = NULL;
	m_htiICFSupport = NULL;
	m_htiICFSupportClearAtEnd = NULL;
	// End MoNKi
*/
	// Added by MoNKi [ SlugFiller: -lowIdRetry- ]
	m_htiLowIdRetry = NULL;
	// End SlugFiller

	// Added by MoNKi [ MoNKi: -Wap Server- ]
	m_htiWapRoot = NULL;
	m_htiWapEnable = NULL;
	m_htiWapPort = NULL;
	m_htiWapTemplate = NULL;
	m_htiWapPass = NULL;
	m_htiWapLowEnable = NULL;
	m_htiWapLowPass = NULL;
	// End MoNKi

/*Commented by SiRoB
	// Added by MoNKi [MoNKi: -USS initial TTL-]
	m_htiUSSRoot = NULL;
	m_htiUSSTTL = NULL;
	// End MoNKi

	// Added by MoNKi [MoNKi: -Custom incoming folder icon-]
	m_htiCustomIncomingIcon = NULL;
	// End MoNKi
*/

	CPropertyPage::OnDestroy();
}

BOOL CPPgEmulespana::OnApply()
{
	// if prop page is closed by pressing VK_ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();

	if (!UpdateData())
		return FALSE;

	bool bRestartApp = false;

	// Added by MoNKi [MoNKi: -Random Ports-]
	 thePrefs.SetRandomPortsSafeResetOnRestartTime((uint16)m_iRandomPortsResetTime);
	// End MoNKi

/*Commented by SiRoB
	// Added by MoNKi [MoNKi: -invisible mode-]
	thePrefs.SetInvisibleMode(m_bInvisibleMode, m_iInvisibleModeActualKeyModifier, m_sInvisibleModeKey.GetAt(0));
	// End MoNKi
*/
	// Added by MoNKi [MoNKi: -UPnPNAT Support-]
	/*if((BOOL)thePrefs.IsUPnPEnabled() != m_bUPnP ||
		(BOOL)thePrefs.GetUPnPNatWeb() != m_bUPnPWeb)
	{
		theApp.m_UPnP_IGDControlPoint->SetUPnPNat(m_bUPnP); // and start/stop nat. 
		thePrefs.SetUPnPNatWeb(m_bUPnPWeb);
	}
	*/
/*    if ((BOOL)thePrefs.GetUPnPNatWeb() != m_bUPnPWeb)
	{
		theApp.m_UPnP_IGDControlPoint->SetUPnPNat(thePrefs.IsUPnPNat()); // and start/stop nat. 
		thePrefs.SetUPnPNatWeb(m_bUPnPWeb);
	}
*/	// MORPH START leuk_he upnp bindaddr
	thePrefs.SetUpnpBindAddr(m_dwUpnpBindAddr);// Note: read code in thePrefs..
	// MORPH END  leuk_he upnp bindaddr
	if (m_iUPnPPort>-1  && m_iUPnPPort < 65535)
	    thePrefs.SetUPnPPort((uint16)m_iUPnPPort );
	thePrefs.SetUPnPClearOnClose (m_bUPnPClearOnClose);
    thePrefs.SetUPnPLimitToFirstConnection(m_bUPnPLimitToFirstConnection);
	if (m_iDetectuPnP==2 || m_iDetectuPnP==0 || m_iDetectuPnP==-1 ||m_iDetectuPnP==-2 ||m_iDetectuPnP==-10)
		thePrefs.SetUpnpDetect(m_iDetectuPnP); // other values are undefined. 
	thePrefs.m_bUPnPForceUpdate=thePrefs.m_bUPnPForceUpdate;
	
	// End MoNKi

/*Commented by SiRoB
	// Added by MoNKi [MoNKi: -Support for High Contrast Mode-]
	thePrefs.SetHighContrastSupport(m_bHighContrast);
	thePrefs.SetHighContrastDisableSkins(m_bHighContrastDisableSkins);
	theApp.ApplySkin(thePrefs.GetSkinProfile());
	// End MoNKi
*/
	// Added by by MoNKi [MoNKi: -Improved ICS-Firewall support-]
	if(thePrefs.m_bICFSupportStatusChanged){
		m_ctrlTreeOptions.SetCheckBox(m_htiICFSupport,thePrefs.GetICFSupport());
		m_bICFSupport = thePrefs.GetICFSupport();
		thePrefs.m_bICFSupportStatusChanged = false;
	}

	if((BOOL)thePrefs.GetICFSupport() != m_bICFSupport
		|| (BOOL)thePrefs.IsOpenPortsOnStartupEnabled() != m_bICFSupportClearAtEnd
		|| (BOOL)thePrefs.GetICFSupportServerUDP() != m_bICFSupportServerUDP)
	{
		bRestartApp = true;
	}
	thePrefs.SetICFSupport(m_bICFSupport);
	thePrefs.m_bOpenPortsOnStartUp = m_bICFSupportClearAtEnd;
	thePrefs.SetICFSupportServerUDP(m_bICFSupportServerUDP);
	// End MoNKi

/*Commented by SiRoB
	// Added by by MoNKi [MoNKi: -Skin Selector-]
	CString sToolbarSkin, sIniSkin;

	thePrefs.SetToolbarBitmapFolderSettings(m_sTBSkinsDir);
	thePrefs.SetSkinProfileDir(m_sSkinsDir);

	sToolbarSkin = m_listTBSkins.GetSelectedSkin();
	sIniSkin = m_listSkins.GetSelectedSkin();

	if(sToolbarSkin.MakeLower() != CString(thePrefs.GetToolbarBitmapSettings()).MakeLower()){
		m_listTBSkins.SelectCurrentSkin();
	}
	
	if(sIniSkin.MakeLower() != CString(thePrefs.GetSkinProfile()).MakeLower()){
		m_listSkins.SelectCurrentSkin();
	}

	thePrefs.SetColumnSortAscending(CPreferences::tableToolbarSkins, m_listTBSkins.GetSortAscending()); 
	thePrefs.SetColumnSortAscending(CPreferences::tableSkins, m_listSkins.GetSortAscending()); 
	// End MoNKi
*/
	// Added by MoNKi [ SlugFiller: -lowIdRetry- ]
	if(m_iLowIdRetry<0)
		m_iLowIdRetry = 0;
	if(m_iLowIdRetry>255)
		m_iLowIdRetry = 255;
	thePrefs.LowIdRetries = m_iLowIdRetry;
	// End SlugFiller

	// Added by by MoNKi [MoNKi: -Wap Server-]
	thePrefs.SetWapServerEnabled(m_bWapEnable);
	thePrefs.SetWapIsLowUserEnabled(m_bWapLowEnable);
	if(m_sWapPass != CString(HIDDEN_PASSWORD))
		thePrefs.SetWapPass(m_sWapPass);
	if(m_sWapLowPass != CString(HIDDEN_PASSWORD))
		thePrefs.SetWapLowPass(m_sWapLowPass);
	if(m_sWapTemplate != thePrefs.GetWapTemplate()){
		thePrefs.SetWapTemplate(m_sWapTemplate);
		theApp.wapserver->ReloadTemplates();
	}
	if(m_iWapPort != thePrefs.GetWapPort()){
		thePrefs.SetWapPort((uint16)m_iWapPort);
		theApp.wapserver->RestartServer();
	}
	theApp.wapserver->StartServer();
	theApp.emuledlg->serverwnd->UpdateMyInfo();
	// End MoNKi

/*Commented by SiRoB
	// Added by MoNKi [MoNKi: -USS initial TTL-]
	thePrefs.SetUSSInitialTTL(m_iUSSTTL);
	// End MoNKi
	// Added by MoNKi [MoNKi: -Custom incoming folder icon-]
	bool oldIncomingIcon = thePrefs.GetCustomIncomingIcon();
	thePrefs.SetCustomIncomingIcon(m_bCustomIncomingIcon);
	if(m_bCustomIncomingIcon)
		theApp.AddIncomingFolderIcon();
	else if(oldIncomingIcon)
		theApp.RemoveIncomingFolderIcon();
	// End MoNKi
*/	
	if (bRestartApp)
		AfxMessageBox(GetResString(IDS_SETTINGCHANGED_RESTART));

	SetModified(FALSE);
	return CPropertyPage::OnApply();
}

BOOL CPPgEmulespana::OnInitDialog()
{
	// Added by MoNKi [MoNKi: -Random Ports-]
	m_iRandomPortsResetTime = thePrefs.GetRandomPortsSafeResetOnRestartTime();
	// End MoNKi
/*Commented by SiRoB
	// Added by MoNKi [MoNKi: -invisible mode-]
	m_bInvisibleMode = thePrefs.GetInvisibleMode();
	m_iInvisibleModeActualKeyModifier = thePrefs.GetInvisibleModeHKKeyModifier();
	m_sInvisibleModeKey = thePrefs.GetInvisibleModeHKKey();

	m_sInvisibleModeMod = "";
	if (m_iInvisibleModeActualKeyModifier & MOD_CONTROL)
		m_sInvisibleModeMod=GetResString(IDS_CTRLKEY);
	if (m_iInvisibleModeActualKeyModifier & MOD_ALT){
		if (!m_sInvisibleModeMod.IsEmpty()) m_sInvisibleModeMod += " + ";
		m_sInvisibleModeMod+=GetResString(IDS_ALTKEY);
	}
	if (m_iInvisibleModeActualKeyModifier & MOD_SHIFT){
		if (!m_sInvisibleModeMod.IsEmpty()) m_sInvisibleModeMod += " + ";
		m_sInvisibleModeMod+=GetResString(IDS_SHIFTKEY);
	}
	// End MoNKi
*/
	// Added by MoNKi [MoNKi: -UPnPNAT Support-]
//	m_bUPnP = thePrefs.IsUPnPEnabled();
//	m_bUPnPWeb = thePrefs.GetUPnPNatWeb();
	// End MoNKi
    // MORPH START leuk_he upnp bindaddr
	m_dwUpnpBindAddr=thePrefs.GetUpnpBindAddr();
	// MORPH END  leuk_he upnp bindaddr
	m_iUPnPPort = thePrefs.GetUPnPPort();
	m_bUPnPClearOnClose = thePrefs.GetUPnPClearOnClose();
	m_bUPnPLimitToFirstConnection=thePrefs.GetUPnPLimitToFirstConnection();
	m_iDetectuPnP=thePrefs.GetUpnpDetect(); // allowed values: 
	m_bUPnPForceUpdate=thePrefs.m_bUPnPForceUpdate; 
/*Commented by SiRoB
	// Added by MoNKi [MoNKi: -Support for High Contrast Mode-]
	m_bHighContrast = thePrefs.GetHighContrastSupport();
	m_bHighContrastDisableSkins = thePrefs.GetHighContrastDisableSkins();
	// End MoNKi
*/
	// Added by by MoNKi [MoNKi: -Improved ICS-Firewall support-]
	m_bICFSupport = thePrefs.GetICFSupport();
	m_bICFSupportClearAtEnd = thePrefs.IsOpenPortsOnStartupEnabled();
	m_bICFSupportServerUDP = thePrefs.GetICFSupportServerUDP();
	// End MoNKi
	
	// Added by MoNKi [ SlugFiller: -lowIdRetry- ]
	m_iLowIdRetry = thePrefs.GetLowIdRetries();
	// End SlugFiller

	// Added by by MoNKi [MoNKi: -Wap Server-]
	m_bWapEnable = thePrefs.GetWapServerEnabled();
	m_iWapPort = thePrefs.GetWapPort();
	m_sWapTemplate = thePrefs.GetWapTemplate();
	m_sWapPass = HIDDEN_PASSWORD;
	m_bWapLowEnable = thePrefs.GetWapIsLowUserEnabled();
	m_sWapLowPass = HIDDEN_PASSWORD;
	// End MoNKi

/*Commented by SiRoB
	// Added by MoNKi [MoNKi: -USS initial TTL-]
	m_iUSSTTL = thePrefs.GetUSSInitialTTL();
	// End MoNKi

	// Added by MoNKi [MoNKi: -Custom incoming folder icon-]
	m_bCustomIncomingIcon = thePrefs.GetCustomIncomingIcon();
	// End MoNKi
*/
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

/*Commented by SiRoB
	// Added by by MoNKi [MoNKi: -Skin Selector-]
	CString tmpString = GetResString(IDS_EM_PREFS); 
	tmpString.Remove('&'); 
	m_tabPrefs.InsertItem(0,tmpString);
	m_tabPrefs.InsertItem(1,GetResString(IDS_TOOLBARSKINS));
	m_tabPrefs.InsertItem(2,GetResString(IDS_SKIN_PROF));

	m_sTBSkinsDir = thePrefs.GetToolbarBitmapFolderSettings();
	m_listTBSkins.SetSortAscending(thePrefs.GetColumnSortAscending(CPreferences::tableToolbarSkins)); 
	m_listTBSkins.Init();
	m_listTBSkins.LoadToolBars(m_sTBSkinsDir);

	m_sSkinsDir = thePrefs.GetSkinProfileDir();
	m_listSkins.SetSortAscending(thePrefs.GetColumnSortAscending(CPreferences::tableSkins)); 
	m_listSkins.Init();
	m_listSkins.LoadSkins(m_sSkinsDir);
	ChangeTab(0);
	// End MoNKi
*/
	InitTooltips(&m_ctrlTreeOptions);
	Localize();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CPPgEmulespana::OnHelp()
{
	//theApp.ShowHelp(0);
}

BOOL CPPgEmulespana::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgEmulespana::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}

BOOL CPPgEmulespana::OnKillActive()
{
	// if prop page is closed by pressing VK_ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	return CPropertyPage::OnKillActive();
}

/*Commented by SiRoB
// Added by by MoNKi [MoNKi: -Skin Selector-]
void CPPgEmulespana::ChangeTab(int nitem)
{
	switch(nitem){
		case 0:
			m_ctrlTreeOptions.ShowWindow(SW_SHOW);
			m_listTBSkins.ShowWindow(SW_HIDE);
			m_listSkins.ShowWindow(SW_HIDE);
			m_editSkinsDir.ShowWindow(SW_HIDE);
			GetDlgItem(IDC_SKINS_LBL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BTN_SKINS_DIR)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BTN_SKINS_RELOAD)->ShowWindow(SW_HIDE);
			break;
		case 1:
			m_ctrlTreeOptions.ShowWindow(SW_HIDE);
			m_listTBSkins.ShowWindow(SW_SHOW);
			m_listSkins.ShowWindow(SW_HIDE);
			m_editSkinsDir.SetWindowText(m_sTBSkinsDir);
			m_editSkinsDir.ShowWindow(SW_SHOW);
			GetDlgItem(IDC_SKINS_LBL)->SetWindowText(GetResString(IDS_SELECTTOOLBARBITMAPDIR));
			GetDlgItem(IDC_SKINS_LBL)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_BTN_SKINS_DIR)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_BTN_SKINS_RELOAD)->ShowWindow(SW_SHOW);
			break;
		case 2:
			m_ctrlTreeOptions.ShowWindow(SW_HIDE);
			m_listTBSkins.ShowWindow(SW_HIDE);
			m_listSkins.ShowWindow(SW_SHOW);
			m_editSkinsDir.SetWindowText(m_sSkinsDir);
			m_editSkinsDir.ShowWindow(SW_SHOW);
			GetDlgItem(IDC_SKINS_LBL)->SetWindowText(GetResString(IDS_SEL_SKINDIR));
			GetDlgItem(IDC_SKINS_LBL)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_BTN_SKINS_DIR)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_BTN_SKINS_RELOAD)->ShowWindow(SW_SHOW);
			break;
	}
}

void CPPgEmulespana::OnTcnSelchangeTabEmulespanaPrefs(NMHDR *pNMHDR, LRESULT *pResult)
{
	ChangeTab(m_tabPrefs.GetCurSel());

	*pResult = 0;
}

void CPPgEmulespana::OnBnClickedBtnSkinsDir()
{
	TCHAR buffer[MAX_PATH] = {0};
	CString title;

	_tcsncpy(buffer, m_sTBSkinsDir.GetBuffer(), ARRSIZE(buffer));
	m_sTBSkinsDir.ReleaseBuffer();
	
	GetDlgItem(IDC_SKINS_LBL)->GetWindowText(title);
	if(SelectDir(GetSafeHwnd(),buffer,title.GetBuffer())){
		m_editSkinsDir.SetWindowText(buffer);
	}
}

void CPPgEmulespana::OnBnClickedBtnSkinsReload()
{
	switch(m_tabPrefs.GetCurSel()){
		case 1:
			m_listTBSkins.LoadToolBars(m_sTBSkinsDir);
			break;
		case 2:
			m_listSkins.LoadSkins(m_sSkinsDir);
			break;
	}
}

void CPPgEmulespana::OnEnChangeEditSkinsDir()
{
	switch(m_tabPrefs.GetCurSel()){
		case 1:
			m_editSkinsDir.GetWindowText(m_sTBSkinsDir);
			break;
		case 2:
			m_editSkinsDir.GetWindowText(m_sSkinsDir);
			break;
	}
	SetModified();
}

void CPPgEmulespana::OnSysColorChange()
{
	CPropertyPage::OnSysColorChange();

	m_listTBSkins.SendMessage(WM_SYSCOLORCHANGE);
	m_listSkins.SendMessage(WM_SYSCOLORCHANGE);
}

void CPPgEmulespana::OnLvnItemchangedListSkins(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	SetModified();

	*pResult = 0;
}

void CPPgEmulespana::OnLvnItemchangedListTbSkins(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	SetModified();

	*pResult = 0;
}
// End MoNKi
*/
