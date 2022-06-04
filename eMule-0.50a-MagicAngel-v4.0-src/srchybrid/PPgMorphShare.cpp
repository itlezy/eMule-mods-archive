// PpgMorph.cpp : implementation file
//

#include "stdafx.h"
#include "emule.h"
#include "PPGtooltipped.h" //MORPH leuk_he addded tooltipped
#include "PPgMorphShare.h"
#include "emuledlg.h"
#include "serverWnd.h" //MORPH - Added by SiRoB
#include "OtherFunctions.h"
#include "Scheduler.h" //MORPH - Added by SiRoB, Fix for Param used in scheduler
#include "searchDlg.h"
#include "sharedfilelist.h" //MORPH - Added by SiRoB, POWERSHARE Limit
#include "uploadqueue.h" //MORPH - Added by SiRoB, PS Internal prio
#include "UserMsgs.h"
//MORPH START show less controls
#include "PreferencesDlg.h"
#include "PPgTweaks.h"
//MORPH END show less controls

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


///////////////////////////////////////////////////////////////////////////////
// CPPgMorphShare dialog

IMPLEMENT_DYNAMIC(CPPgMorphShare, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgMorphShare, CPropertyPage)
	ON_WM_HSCROLL()
    ON_WM_DESTROY()
	ON_MESSAGE(UM_TREEOPTSCTRL_NOTIFY, OnTreeOptsCtrlNotify)
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

CPPgMorphShare::CPPgMorphShare()
//	: CPropertyPage(CPPgMorphShare::IDD)
	: CPPgtooltipped(CPPgMorphShare::IDD)
	, m_ctrlTreeOptions(theApp.m_iDfltImageListColorFlags)
{
	m_bInitializedTreeOpts = false;
	m_htiSpreadbar = NULL; //MORPH	- Added by AndCycle, SLUGFILLER: Spreadbars - per file
	m_htiHideOS = NULL;	//MORPH - Added by SiRoB, SLUGFILLER: hideOS
	m_htiSelectiveShare = NULL;	//MORPH - Added by SiRoB, SLUGFILLER: hideOS
	m_htiShareOnlyTheNeed = NULL; //MORPH - Added by SiRoB, SHARE_ONLY_THE_NEED
	m_htiPowerShareLimit = NULL; //MORPH - Added by SiRoB, POWERSHARE Limit
	//MORPH START - Added by SiRoB, Avoid misusing of powersharing	
	m_htiPowershareMode = NULL;
	m_htiPowershareDisabled = NULL;
	m_htiPowershareActivated = NULL;
	m_htiPowershareAuto = NULL;
	m_htiPowershareLimited = NULL;
	//MORPH END   - Added by SiRoB, Avoid misusing of powersharing	
	m_htiPowershareInternalPrio = NULL; //Morph - added by AndCyle, selective PS internal Prio
	//MORPH START - Added by SiRoB, Show Permission
	m_htiPermissions = NULL;
	m_htiPermAll = NULL;
	m_htiPermFriend = NULL;
	m_htiPermNone = NULL;
	// Mighty Knife: Community visible filelist
	m_htiPermCommunity = NULL;
	// [end] Mighty Knife
	//MORPH END   - Added by SiRoB, Show Permission
	m_htiFolderIcons = NULL;
	m_htiDisplay = NULL;
	m_htiStaticIcon = NULL; //MORPH - Added, Static Tray Icon
	m_htiShowLessControls = NULL; // show less controls
	m_htiFakeAlyzerIndications = NULL; //MORPH - Added by Stulle, Fakealyzer [netfinity]
}

CPPgMorphShare::~CPPgMorphShare()
{
}

void CPPgMorphShare::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MORPH_OPTS, m_ctrlTreeOptions);
	if (!m_bInitializedTreeOpts)
	{
		int iImgSFM = 8;
		int iImgPerm = 8;
		int iImgPS = 8;
		int iImgDisp = 8;

		CImageList* piml = m_ctrlTreeOptions.GetImageList(TVSIL_NORMAL);
		if (piml){
			iImgSFM = piml->Add(CTempIconLoader(_T("SHAREDFILES")));
			iImgPS = piml->Add(CTempIconLoader(_T("FILEPOWERSHARE"))); //MORPH - Added by SiRoB, POWERSHARE Limit
			iImgPerm = piml->Add(CTempIconLoader(_T("FILEPERMISSION"))); //MORPH - Added by SiRoB, Show Permission
			iImgDisp = piml->Add(CTempIconLoader(_T("DISPLAY")));
		}
		
		CString Buffer;
		m_htiSFM = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_SFM), iImgSFM, TVI_ROOT);

		//MORPH	Start	- Added by AndCycle, SLUGFILLER: Spreadbars - per file
		m_htiSpreadbar = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SPREADBAR_DEFAULT_CHECKBOX), m_htiSFM, m_iSpreadbar);
		//MORPH	End	- Added by AndCycle, SLUGFILLER: Spreadbars - per file

		//MORPH START - Added by SiRoB, SLUGFILLER: hideOS
		m_htiHideOS = m_ctrlTreeOptions.InsertItem(GetResString(IDS_HIDEOVERSHARES), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiSpreadbar);
		m_ctrlTreeOptions.AddEditBox(m_htiHideOS, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiSelectiveShare = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SELECTIVESHARE), m_htiHideOS, m_bSelectiveShare);
		//MORPH END   - Added by SiRoB, SLUGFILLER: hideOS
		//MORPH START - Added by SiRoB, SHARE_ONLY_THE_NEED
		m_htiShareOnlyTheNeed = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SHAREONLYTHENEED), m_htiSFM, m_iShareOnlyTheNeed);
		//MORPH END   - Added by SiRoB, SHARE_ONLY_THE_NEED

		//MORPH START - Added by SiRoB, Avoid misusing of powersharing
		m_htiPowershareMode = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_POWERSHARE), iImgPS, m_htiSFM);
		m_htiPowershareDisabled = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_POWERSHARE_DISABLED), m_htiPowershareMode, m_iPowershareMode == 0);
		m_htiPowershareActivated =  m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_POWERSHARE_ACTIVATED), m_htiPowershareMode, m_iPowershareMode == 1);
		m_htiPowershareAuto =  m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_POWERSHARE_AUTO), m_htiPowershareMode, m_iPowershareMode == 2);
		//MORPH END   - Added by SiRoB, Avoid misusing of powersharing
		//MORPH START - Added by SiRoB, POWERSHARE Limit
		m_htiPowershareLimited =  m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_POWERSHARE_LIMITED), m_htiPowershareMode, m_iPowershareMode == 3);
		m_htiPowerShareLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_POWERSHARE_LIMIT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiPowershareLimited );
		m_ctrlTreeOptions.AddEditBox(m_htiPowerShareLimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		//MORPH END   - Added by SiRoB, POWERSHARE Limit
		//Morph Start - added by AndCyle, selective PS internal Prio
		m_htiPowershareInternalPrio = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_POWERSHARE_INTERPRIO), m_htiPowershareMode, m_bPowershareInternalPrio);
		//Morph End - added by AndCyle, selective PS internal Prio
		
		//MORPH START - Added by SiRoB, Show Permission
		m_htiPermissions = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_PERMISSION), iImgPerm, m_htiSFM);
		m_htiPermAll = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_PW_EVER), m_htiPermissions, m_iPermissions == 0);
		m_htiPermFriend = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_FSTATUS_FRIENDSONLY), m_htiPermissions, m_iPermissions == 1);
		m_htiPermNone = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_HIDDEN), m_htiPermissions, m_iPermissions == 2);
		// Mighty Knife: Community visible filelist
		m_htiPermCommunity = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_COMMUNITY), m_htiPermissions, m_iPermissions == 3);
		m_htiCommunityName = m_ctrlTreeOptions.InsertItem(GetResString(IDS_COMMUNITYTAG), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,m_htiPermCommunity);
		m_ctrlTreeOptions.AddEditBox(m_htiCommunityName, RUNTIME_CLASS(CTreeOptionsEdit));
		// [end] Mighty Knife
		//MORPH END   - Added by SiRoB, Show Permission

		m_htiDisplay = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_PW_DISPLAY), iImgDisp, TVI_ROOT);
		m_htiFolderIcons = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_FOLDERICONS),m_htiDisplay, m_bFolderIcons);
		m_htiStaticIcon = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_STATIC_ICON),m_htiDisplay, m_bStaticIcon); //MORPH - Added, Static Tray Icon
		// MORPH START show less controls
		m_htiShowLessControls= m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MORPHWIZ_SHOWLESS),m_htiDisplay, m_bShowLessControls);
		// MORPH END  show less controls
		m_htiFakeAlyzerIndications = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_FAKEALYZER_INDICATIONS),m_htiDisplay, m_bFakeAlyzerIndications); //MORPH - Added by Stulle, Fakealyzer [netfinity]


		m_ctrlTreeOptions.Expand(m_htiSFM, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiSpreadbar, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiHideOS, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiPermissions, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiPowershareMode, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiDisplay, TVE_EXPAND);
		m_ctrlTreeOptions.SendMessage(WM_VSCROLL, SB_TOP);
		m_bInitializedTreeOpts = true;
	}
	//MORPH	Start	- Added by AndCycle, SLUGFILLER: Spreadbars - per file
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiSpreadbar, m_iSpreadbar);
	//MORPH	End	- Added by AndCycle, SLUGFILLER: Spreadbars - per file
	//MORPH START - Added by SiRoB, SLUGFILLER: hideOS
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiHideOS, m_iHideOS);
	DDV_MinMaxInt(pDX, m_iHideOS, 0, INT_MAX);
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiSelectiveShare, m_bSelectiveShare);
	//MORPH END - Added by SiRoB, SLUGFILLER: hideOS
	//MORPH START - Added by SiRoB, SHARE_ONLY_THE_NEED
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiShareOnlyTheNeed, m_iShareOnlyTheNeed);
	//MORPH END   - Added by SiRoB, SHARE_ONLY_THE_NEED
	//MORPH START - Added by SiRoB, POWERSHARE Limit
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiPowerShareLimit, m_iPowerShareLimit);
	DDV_MinMaxInt(pDX, m_iPowerShareLimit, 0, INT_MAX);
	//MORPH END   - Added by SiRoB, POWERSHARE Limit
	//MORPH START - Added by SiRoB, Avoid misusing of powersharing
	DDX_TreeRadio(pDX, IDC_MORPH_OPTS, m_htiPowershareMode, m_iPowershareMode);
	//MORPH END   - Added by SiRoB, Avoid misusing of powersharing
	//Morph Start - added by AndCyle, selective PS internal Prio
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiPowershareInternalPrio, m_bPowershareInternalPrio);
	//Morph End - added by AndCyle, selective PS internal Prio
	//MORPH START - Added by SiRoB, Show Permission
	DDX_TreeRadio(pDX, IDC_MORPH_OPTS, m_htiPermissions, m_iPermissions);
	// Mighty Knife: Community visualization
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiCommunityName, m_sCommunityName);
	// [end] Mighty Knife
	//MORPH END   - Added by SiRoB, Show Permission
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiFolderIcons, m_bFolderIcons);
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiStaticIcon, m_bStaticIcon); //MORPH - Added, Static Tray Icon
	// MORPH START show less controls
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiShowLessControls, m_bShowLessControls);
	// MORPH END  show less controls
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiFakeAlyzerIndications, m_bFakeAlyzerIndications); //MORPH - Added by Stulle, Fakealyzer [netfinity]
}


BOOL CPPgMorphShare::OnInitDialog()
{
	m_iPowershareMode = thePrefs.m_iPowershareMode;//MORPH - Added by SiRoB, Avoid misusing of powersharing
	m_iSpreadbar = thePrefs.GetSpreadbarSetStatus(); //MORPH	- Added by AndCycle, SLUGFILLER: Spreadbars - per file
	m_iHideOS = thePrefs.hideOS; //MORPH - Added by SiRoB, SLUGFILLER: hideOS
	m_bSelectiveShare = thePrefs.selectiveShare; //MORPH - Added by SiRoB, SLUGFILLER: hideOS
	m_iShareOnlyTheNeed = thePrefs.ShareOnlyTheNeed; //MORPH - Added by SiRoB, SHARE_ONLY_THE_NEED
	m_iPowerShareLimit = thePrefs.PowerShareLimit; //MORPH - Added by SiRoB, POWERSHARE Limit
	m_bPowershareInternalPrio = thePrefs.m_bPowershareInternalPrio; //Morph - added by AndCyle, selective PS internal Prio
	m_iPermissions = thePrefs.permissions; //MORPH - Added by SiRoB, Show Permission
	// Mighty Knife: Community visualization
	m_sCommunityName = thePrefs.m_sCommunityName;
	// [end] Mighty Knife
	m_bFolderIcons = thePrefs.m_bShowFolderIcons;
	m_bStaticIcon = thePrefs.GetStaticIcon(); //MORPH - Added, Static Tray Icon
	// MORPH START show less controls
    m_bShowLessControls = thePrefs.IsLessControls();
	m_bOverideBySetExtControls = false;
	// MORPH END  show less controls
	m_bFakeAlyzerIndications = thePrefs.GetFakeAlyzerIndications(); //MORPH - Added by Stulle, Fakealyzer [netfinity]

	CPropertyPage::OnInitDialog();
	//InitTooltips(); //leuk_he tooltipped

	InitTooltips(&m_ctrlTreeOptions);
	Localize();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPgMorphShare::OnKillActive()
{
	// if prop page is closed by pressing VK_ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	return CPropertyPage::OnKillActive();
}

BOOL CPPgMorphShare::OnApply()
{
	// if prop page is closed by pressing VK_ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	
	if (!UpdateData())
		return FALSE;
	thePrefs.m_iPowershareMode = m_iPowershareMode;//MORPH - Added by SiRoB, Avoid misusing of powersharing
	thePrefs.m_iSpreadbarSetStatus = m_iSpreadbar; //MORPH	- Added by AndCycle, SLUGFILLER: Spreadbars - per file
	thePrefs.hideOS = m_iHideOS;	//MORPH - Added by SiRoB, SLUGFILLER: hideOS
	thePrefs.selectiveShare = m_bSelectiveShare; //MORPH - Added by SiRoB, SLUGFILLER: hideOS
	thePrefs.ShareOnlyTheNeed = m_iShareOnlyTheNeed!=0; //MORPH - Added by SiRoB, SHARE_ONLY_THE_NEED
	//MORPH START - Added by SiRoB, POWERSHARE Limit
	thePrefs.PowerShareLimit = m_iPowerShareLimit;
	theApp.sharedfiles->UpdatePartsInfo();
	//MORPH END   - Added by SiRoB, POWERSHARE Limit
	thePrefs.permissions = m_iPermissions; //MORPH - Added by SiRoB, Show Permission
	// Mighty Knife: Community visualization
	_stprintf (thePrefs.m_sCommunityName,_T("%s"), m_sCommunityName);
	// [end] Mighty Knife
	bool oldValue = thePrefs.m_bPowershareInternalPrio;
	thePrefs.m_bPowershareInternalPrio = m_bPowershareInternalPrio; //Morph - added by AndCyle, selective PS internal Prio
	if(thePrefs.m_bPowershareInternalPrio != oldValue)
		theApp.uploadqueue->ReSortUploadSlots(true);
	if(thePrefs.m_bShowFolderIcons != (m_bFolderIcons == 1))
	{
		if(m_bFolderIcons)
		{
			theApp.AddIncomingFolderIcon();
			theApp.AddTempFolderIcon();
		}
		else
		{
			theApp.RemoveIncomingFolderIcon();
			theApp.RemoveTempFolderIcon();
		}
	}
	thePrefs.m_bShowFolderIcons = m_bFolderIcons;
	//MORPH START - Added, Static Tray Icon
	if(m_bStaticIcon != thePrefs.m_bStaticIcon)
	{
		if(m_bStaticIcon)
			theApp.emuledlg->TrayShow();
		else if(theApp.emuledlg->IsWindowVisible()) //only hide when window visible
			theApp.emuledlg->TrayHide();
	}
	thePrefs.m_bStaticIcon = m_bStaticIcon;
	//MORPH END   - Added, Static Tray Icon
	//MORPH START show less controls
	// disable extended controls when we want to show less controls
	if(m_bShowLessControls && thePrefs.m_bExtControls)
		thePrefs.m_bExtControls = false;
	if(!m_bOverideBySetExtControls)
	    thePrefs.SetLessControls(m_bShowLessControls);
	m_bOverideBySetExtControls = false; // we just want to override once
	//MORPH END  show less controls
	thePrefs.m_bFakeAlyzerIndications = m_bFakeAlyzerIndications; //MORPH - Added by Stulle, Fakealyzer [netfinity]
	
	//theApp.scheduler->SaveOriginals(); //Removed by SiRoB, no scheduler param in this ppg //Added by SiRoB, Fix for Param used in scheduler

	SetModified(FALSE);


	return CPropertyPage::OnApply();
}

void CPPgMorphShare::Localize(void)
{	
	if(m_hWnd)
	{
		GetDlgItem(IDC_WARNINGMORPH)->SetWindowText(GetResString(IDS_WARNINGMORPH));
		CString Buffer;
		//if (m_htiSpreadbar) m_ctrlTreeOptions.SetItemText(m_htiSpreadbar, GetResString(IDS_SPREADBAR));//MORPH	- Added by AndCycle, SLUGFILLER: Spreadbars - per file
		if (m_htiHideOS) {m_ctrlTreeOptions.SetEditLabel(m_htiHideOS, GetResString(IDS_HIDEOVERSHARES));//MORPH - Added by SiRoB, SLUGFILLER: hideOS
		                  SetTool(m_htiHideOS,IDS_HIDEOVERSHARES); 
		}
		if (m_htiSelectiveShare) m_ctrlTreeOptions.SetItemText(m_htiSelectiveShare, GetResString(IDS_SELECTIVESHARE));//MORPH - Added by SiRoB, SLUGFILLER: hideOS
		                   SetTool(m_htiSelectiveShare,IDS_SELECTIVESHARE); 
		if (m_htiShareOnlyTheNeed) m_ctrlTreeOptions.SetItemText(m_htiShareOnlyTheNeed, GetResString(IDS_SHAREONLYTHENEED));//MORPH - Added by SiRoB, SHARE_ONLY_THE_NEED
		//MORPH START - Added by SiRoB, Avoid misusing of powersharing
		if (m_htiPowershareMode) m_ctrlTreeOptions.SetItemText(m_htiPowershareMode, GetResString(IDS_POWERSHARE));
		if (m_htiPowershareDisabled) m_ctrlTreeOptions.SetItemText(m_htiPowershareDisabled, GetResString(IDS_POWERSHARE_DISABLED));
		if (m_htiPowershareActivated) m_ctrlTreeOptions.SetItemText(m_htiPowershareActivated, GetResString(IDS_POWERSHARE_ACTIVATED));
		if (m_htiPowershareAuto) m_ctrlTreeOptions.SetItemText(m_htiPowershareAuto, GetResString(IDS_POWERSHARE_AUTO));
		if (m_htiPowershareLimited) m_ctrlTreeOptions.SetItemText(m_htiPowershareLimited, GetResString(IDS_POWERSHARE_LIMITED));
		//MORPH START - Added by SiRoB, POWERSHARE Limit
		if (m_htiPowerShareLimit) m_ctrlTreeOptions.SetEditLabel(m_htiPowerShareLimit, GetResString(IDS_POWERSHARE_LIMIT));
		//MORPH END   - Added by SiRoB, POWERSHARE Limit
		//Morph Start - added by AndCyle, selective PS internal Prio
		if (m_htiPowershareInternalPrio) m_ctrlTreeOptions.SetItemText(m_htiPowershareInternalPrio, GetResString(IDS_POWERSHARE_INTERPRIO));
		//Morph End - added by AndCyle, selective PS internal Prio
		//MORPH START - Added by SiRoB, Show Permission
		if (m_htiPermissions) m_ctrlTreeOptions.SetItemText(m_htiPermissions, GetResString(IDS_PERMISSION));
		if (m_htiPermAll) m_ctrlTreeOptions.SetItemText(m_htiPermAll, GetResString(IDS_PW_EVER));
		if (m_htiPermFriend) m_ctrlTreeOptions.SetItemText(m_htiPermFriend, GetResString(IDS_FSTATUS_FRIENDSONLY));
		if (m_htiPermNone) m_ctrlTreeOptions.SetItemText(m_htiPermNone, GetResString(IDS_HIDDEN));
		// Mighty Knife: Community visible filelist
		if (m_htiPermCommunity) m_ctrlTreeOptions.SetItemText(m_htiPermCommunity, GetResString(IDS_COMMUNITY));
		// [end] Mighty Knife
		//MORPH END   - Added by SiRoB, Show Permission
		if (m_htiFolderIcons) m_ctrlTreeOptions.SetItemText(m_htiFolderIcons, GetResString(IDS_FOLDERICONS));
		if (m_htiStaticIcon) m_ctrlTreeOptions.SetItemText(m_htiStaticIcon, GetResString(IDS_STATIC_ICON)); //MORPH - Added, Static Tray Icon
		// MORPH START show less controls
		if (m_htiShowLessControls) m_ctrlTreeOptions.SetItemText(m_htiShowLessControls, GetResString(IDS_MORPHWIZ_SHOWLESS)); 
		// MORPH END  show less controls
		if (m_htiFakeAlyzerIndications) m_ctrlTreeOptions.SetItemText(m_htiFakeAlyzerIndications, GetResString(IDS_FAKEALYZER_INDICATIONS)); //MORPH - Added by Stulle, Fakealyzer [netfinity]
        // MORPH START leuk_he tooltipped
       SetTool(m_htiSFM ,IDS_SFM_TIP);
       SetTool(m_htiSpreadbar ,IDS_SPREADBAR_DEFAULT_CHECKBOX_TIP);
       SetTool(m_htiHideOS ,IDS_HIDEOVERSHARES_TIP);
	   SetTool(m_htiSelectiveShare ,IDS_SELECTIVESHARE_TIP);
	   SetTool(m_htiShareOnlyTheNeed ,IDS_SHAREONLYTHENEED_TIP);
		SetTool(m_htiPowershareMode ,IDS_POWERSHARE_TIP);
		SetTool(m_htiPowershareDisabled ,IDS_POWERSHARE_DISABLED_TIP);
		SetTool(m_htiPowershareActivated ,IDS_POWERSHARE_ACTIVATED_TIP);
		SetTool(m_htiPowershareAuto ,IDS_POWERSHARE_AUTO_TIP);
		SetTool(m_htiPowershareLimited ,IDS_POWERSHARE_LIMITED_TIP);
		SetTool(m_htiPowerShareLimit ,IDS_POWERSHARE_LIMIT_TIP);
		SetTool(m_htiPowershareInternalPrio ,IDS_POWERSHARE_INTERPRIO_TIP);
		SetTool(m_htiPermissions ,IDS_PERMISSION_TIP);
		SetTool(m_htiPermAll ,IDS_FSTATUS_EVER_TIP);
		SetTool(m_htiPermFriend ,IDS_FSTATUS_FRIENDSONLY_TIP);
		SetTool(m_htiPermNone ,IDS_HIDDEN_TIP);
		SetTool(m_htiPermCommunity ,IDS_COMMUNITY_TIP);
		SetTool(m_htiCommunityName, IDS_COMMUNITYTAG_TIP);
		SetTool(m_htiDisplay ,IDS_PW_DISPLAY_TIP);
		SetTool(m_htiFolderIcons ,IDS_FOLDERICON_TIP);
		SetTool(m_htiStaticIcon ,IDS_STATIC_ICON_TIP);
		SetTool(m_htiShowLessControls ,IDS_MORPHWIZ_SHOWLESS_TIP);
		SetTool(m_htiFakeAlyzerIndications ,IDS_FAKEALYZER_INDICATIONS_TIP);
        // MORPH END leuk_he tooltipped

	}

}

void CPPgMorphShare::OnDestroy()
{
	m_ctrlTreeOptions.DeleteAllItems();
	m_ctrlTreeOptions.DestroyWindow();
	m_bInitializedTreeOpts = false;
	m_htiSpreadbar = NULL; //MORPH	- Added by AndCycle, SLUGFILLER: Spreadbars - per file
	m_htiHideOS = NULL;	//MORPH - Added by SiRoB, SLUGFILLER: hideOS
	m_htiSelectiveShare = NULL;	//MORPH - Added by SiRoB, SLUGFILLER: hideOS
	m_htiShareOnlyTheNeed = NULL; //MORPH - Added by SiRoB, SHARE_ONLY_THE_NEED
	m_htiPowerShareLimit = NULL; //MORPH - Added by SiRoB, POWERSHARE Limit
	//MORPH START - Added by SiRoB, Avoid misusing of powersharing	
	m_htiPowershareMode = NULL;
	m_htiPowershareDisabled = NULL;
	m_htiPowershareActivated = NULL;
	m_htiPowershareAuto = NULL;
	m_htiPowershareLimited = NULL;
	//MORPH END   - Added by SiRoB, Avoid misusing of powersharing	
	m_htiPowershareInternalPrio = NULL; //Morph - added by AndCyle, selective PS internal Prio
	//MORPH START - Added by SiRoB, Show Permission
	m_htiPermissions = NULL;
	m_htiPermAll = NULL;
	m_htiPermFriend = NULL;
	m_htiPermNone = NULL;
	m_htiPermCommunity = NULL;
	//MORPH END   - Added by SiRoB, Show Permission
	m_htiFolderIcons = NULL;
	m_htiDisplay = NULL;
	m_htiStaticIcon = NULL; //MORPH - Added, Static Tray Icon
    m_htiShowLessControls = NULL; // MORPH show less controls
	m_htiFakeAlyzerIndications = NULL; //MORPH - Added by Stulle, Fakealyzer [netfinity]

	CPropertyPage::OnDestroy();
}
LRESULT CPPgMorphShare::OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM /*lParam*/)
{
	if (wParam == IDC_MORPH_OPTS){
		//TREEOPTSCTRLNOTIFY* pton = (TREEOPTSCTRLNOTIFY*)lParam;
		//		if (bCheck && m_ctrlTreeOptions.GetCheckBox(m_htiUSSEnabled,bCheck))
		//			if (bCheck) m_ctrlTreeOptions.SetCheckBox(m_htiUSSEnabled,false);
		//}else if (pton->hItem == m_htiUSSEnabled){
		//	BOOL bCheck;
		//	if (m_ctrlTreeOptions.GetCheckBox(m_htiUSSEnabled, bCheck))
		//		if (bCheck && m_ctrlTreeOptions.GetCheckBox(m_htiSUCEnabled,bCheck))
		//			if (bCheck) m_ctrlTreeOptions.SetCheckBox(m_htiSUCEnabled,false);
		//	
		//}
		SetModified();
	}
	return 0;
}
void CPPgMorphShare::OnHelp()
{
	//theApp.ShowHelp(0);
}

BOOL CPPgMorphShare::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgMorphShare::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}