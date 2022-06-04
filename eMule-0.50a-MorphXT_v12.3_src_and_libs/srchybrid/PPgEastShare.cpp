// PpgEastShare.cpp : implementation file
//
//this file is part of eMule morph
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation;
//version 2 of the License

#include "stdafx.h"
#include "emule.h"
#include "PPGtooltipped.h" //MORPH leuk_he addded tooltipped
#include "PPgEastShare.h"
#include "OtherFunctions.h"
#include "UserMsgs.h"

//EastShare Start - added by AndCycle, IP to Country
#include "ip2country.h"
//EastShare End - added by AndCycle, IP to Country

//MORPH START - Added by SiRoB, reduce a little CPU usage for ratio count
#include "ClientCredits.h"	
//MORPH END   - Added by SiRoB, reduce a little CPU usage for ratio count

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////
// CPPgEastShare dialog

IMPLEMENT_DYNAMIC(CPPgEastShare, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgEastShare, CPropertyPage)
	ON_WM_HSCROLL()
    ON_WM_DESTROY()
	ON_MESSAGE(UM_TREEOPTSCTRL_NOTIFY, OnTreeOptsCtrlNotify)
	ON_WM_HELPINFO()
END_MESSAGE_MAP()


CPPgEastShare::CPPgEastShare()
//MORPH START  - leuk_he tooltipped
/*
    : CPropertyPage(CPPgEastShare::IDD)
*/
	: CPPgtooltipped(CPPgEastShare::IDD)
//MORPH END - leuk_he tooltipped		
	, m_ctrlTreeOptions(theApp.m_iDfltImageListColorFlags)
{
	m_bInitializedTreeOpts = false;
	m_htiEnablePreferShareAll = NULL; //EastShare - PreferShareAll by AndCycle
	m_htiIsPayBackFirst = NULL; //EastShare - added by AndCycle, Pay Back First
	m_htiPayBackFirstLimit = NULL; //MORPH - Added by SiRoB, Pay Back First Tweak
	m_htiOnlyDownloadCompleteFiles = NULL;//EastShare - Added by AndCycle, Only download complete files v2.1 (shadow)
	m_htiSaveUploadQueueWaitTime = NULL;//Morph - added by AndCycle, Save Upload Queue Wait Time (MSUQWT)
	m_htiEnableChunkDots = NULL; //EastShare - Added by Pretender, Option for ChunkDots

	//EastShare Start - added by AndCycle, IP to Country
	m_htiIP2CountryName = NULL;
	m_htiIP2CountryName_DISABLE = NULL;
	m_htiIP2CountryName_SHORT = NULL;
	m_htiIP2CountryName_MID = NULL;
	m_htiIP2CountryName_LONG = NULL;
	m_htiIP2CountryShowFlag = NULL;
	//EastShare End - added by AndCycle, IP to Country

	//EastShare START - Added by Pretender
	m_htiCreditSystem = NULL;
	m_htiOfficialCredit = NULL;
	m_htiLovelaceCredit = NULL;
	m_htiRatioCredit = NULL;
	m_htiPawcioCredit = NULL;
	m_htiESCredit = NULL;
	//EastShare END - Added by Pretender

	//Morph - added by AndCycle, Equal Chance For Each File
	m_htiEnableEqualChanceForEachFile = NULL;
	//Morph - added by AndCycle, Equal Chance For Each File

	m_htiFollowTheMajority = NULL; // EastShare       - FollowTheMajority by AndCycle
	m_htiFairPlay = NULL; //EastShare	- FairPlay by AndCycle

	//EastShare START - Added by TAHO, .met control
	m_htiMetControl = NULL;
	m_htiDontPurge = NULL;
	m_htiPartiallyPurge = NULL;
	m_htiCompletelyPurge = NULL;
	if(thePrefs.DoPartiallyPurgeOldKnownFiles())
		m_iPurgeMode = 1;
	else if(thePrefs.DoCompletlyPurgeOldKnownFiles())
		m_iPurgeMode = 2;
	else
		m_iPurgeMode = 0;
	m_htiKnownMet = NULL;
	m_htiRemoveAichImmediatly = NULL;
	//EastShare END - Added by TAHO, .met control
}

CPPgEastShare::~CPPgEastShare()
{
}

void CPPgEastShare::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EASTSHARE_OPTS, m_ctrlTreeOptions);
	if (!m_bInitializedTreeOpts)
	{
		int iImgIP2Country = 8; //EastShare - added by AndCycle, IP to Country
		int iImgCS = 8; //EastShare Added by linekin, CreditSystem
		int iImgMETC = 8; //EastShare Added by TAHO, .met Control
		CImageList* piml = m_ctrlTreeOptions.GetImageList(TVSIL_NORMAL);
		if (piml){
			iImgIP2Country = piml->Add(CTempIconLoader(_T("SEARCHMETHOD_GLOBAL"))); //EastShare - added by AndCycle, IP to Country
			iImgCS = piml->Add(CTempIconLoader(_T("STATSCLIENTS"))); // EastShare START - Added by Pretender, CS icon
			iImgMETC = piml->Add(CTempIconLoader(_T("HARDDISK"))); // EastShare START - Added by TAHO, .met control
		}
		m_htiEnablePreferShareAll = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PREFER_SHARE_ALL), TVI_ROOT, m_bEnablePreferShareAll);//EastShare - PreferShareAll by AndCycle
		m_htiOnlyDownloadCompleteFiles = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ONLY_DOWNLOAD_COMPLETE_FILES), TVI_ROOT, m_bOnlyDownloadCompleteFiles);//EastShare - Added by AndCycle, Only download complete files v2.1 (shadow)
		m_htiSaveUploadQueueWaitTime = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SAVE_UPLOAD_QUEUE_WAIT_TIME), TVI_ROOT, m_bSaveUploadQueueWaitTime);//Morph - added by AndCycle, Save Upload Queue Wait Time (MSUQWT)
		m_htiEnableChunkDots = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ENABLE_CHUNKDOTS), TVI_ROOT, m_bEnableChunkDots);//EastShare - Added by Pretender, Option for ChunkDots

		//Morph - added by AndCycle, Equal Chance For Each File
		m_htiEnableEqualChanceForEachFile = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ECFEF), TVI_ROOT, m_bEnableEqualChanceForEachFile);
		//Morph - added by AndCycle, Equal Chance For Each File

		// EastShare Start - FollowTheMajority by AndCycle
		m_htiFollowTheMajority = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_FOLLOW_THE_MAJORITY), TVI_ROOT, m_bFollowTheMajority);
		// EastShare End   - FollowTheMajority by AndCycle

		//EastShare	Start - FairPlay by AndCycle
		m_htiFairPlay = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_FAIR_PLAY), TVI_ROOT, m_bFairPlay); 
		//EastShare	End   - FairPlay by AndCycle

		//EastShare Start - added by AndCycle, IP to Country
		m_htiIP2CountryName = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_IP2COUNTRY), iImgIP2Country, TVI_ROOT);
		m_htiIP2CountryName_DISABLE = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_DISABLED), m_htiIP2CountryName, m_iIP2CountryName == IP2CountryName_DISABLE);
		m_htiIP2CountryName_SHORT = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_COUNTRYNAME_SHORT), m_htiIP2CountryName, m_iIP2CountryName == IP2CountryName_SHORT);
		m_htiIP2CountryName_MID = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_COUNTRYNAME_MID), m_htiIP2CountryName, m_iIP2CountryName == IP2CountryName_MID);
		m_htiIP2CountryName_LONG = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_COUNTRYNAME_LONG), m_htiIP2CountryName, m_iIP2CountryName == IP2CountryName_LONG);
		m_htiIP2CountryShowFlag = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_COUNTRYNAME_SHOWFLAG), m_htiIP2CountryName, m_bIP2CountryShowFlag);
		//EastShare End - added by AndCycle, IP to Country

		// EastShare START - Added by linekin, new creditsystem by [lovelace]  // Modified by Pretender
		m_htiCreditSystem = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_CREDIT_SYSTEM), iImgCS, TVI_ROOT);
		//EastShare Start - CreditSystemSelection
		m_htiOfficialCredit = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_OFFICIAL_CREDIT), m_htiCreditSystem, m_iCreditSystem == 0);
		m_htiLovelaceCredit = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_LOVELACE_CREDIT), m_htiCreditSystem, m_iCreditSystem == 1);
		//m_htiRatioCredit = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_RATIO_CREDIT), m_htiCreditSystem, m_iCreditSystem == CS_RATIO);
		m_htiPawcioCredit = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_PAWCIO_CREDIT), m_htiCreditSystem, m_iCreditSystem == 2);
		m_htiESCredit = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_EASTSHARE_CREDIT), m_htiCreditSystem, m_iCreditSystem == 3);
		//EastShare End - CreditSystemSelection
		m_htiIsPayBackFirst = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PAYBACKFIRST), m_htiCreditSystem, m_bIsPayBackFirst);//EastShare - added by AndCycle, Pay Back First
		// EastShare END - Added by linekin, new creditsystem by [lovelace]
		//MORPH START - Added by SiRoB, Pay Back First Tweak
		m_htiPayBackFirstLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_PAYBACKFIRSTLIMIT),TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,m_htiIsPayBackFirst);
		m_ctrlTreeOptions.AddEditBox(m_htiPayBackFirstLimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_ctrlTreeOptions.Expand(m_htiIsPayBackFirst, TVE_EXPAND);
		//MORPH END   - Added by SiRoB, Pay Back First Tweak
		
		// EastShare START - Added by TAHO, .met control // Modified by Pretender
		m_htiMetControl = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_MET_FILE_CONTROL), iImgMETC, TVI_ROOT);
		m_htiDontPurge = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_PURGE_DONT), m_htiMetControl, m_iPurgeMode == 0);
		m_htiPartiallyPurge = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_PURGE_PARTIALLY), m_htiMetControl, m_iPurgeMode == 1);
		m_htiCompletelyPurge = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_PURGE_COMPLETLY), m_htiMetControl, m_iPurgeMode == 2);
		m_htiKnownMet = m_ctrlTreeOptions.InsertItem(GetResString(IDS_EXPIRED_KNOWN), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiMetControl);
		m_ctrlTreeOptions.AddEditBox(m_htiKnownMet, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiRemoveAichImmediatly = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_REMOVE_AICH_IMMEDIATLY), m_htiMetControl, m_bRemoveAichImmediatly);
		// EastShare END - Added by TAHO, .met control

		// EastShare START - Added by Pretender
		m_ctrlTreeOptions.Expand(m_htiIP2CountryName, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiCreditSystem, TVE_EXPAND);//EastShare
		m_ctrlTreeOptions.Expand(m_htiMetControl, TVE_EXPAND);
		// EastShare END - Added by Pretender
		m_ctrlTreeOptions.SendMessage(WM_VSCROLL, SB_TOP);
		m_bInitializedTreeOpts = true;
	}

	//this is bad using enum for radio button...need (int &) ^*&^#*^$(, by AndCycle

	//EastShare - added by AndCycle, IP to Country
	DDX_TreeRadio(pDX, IDC_EASTSHARE_OPTS, m_htiIP2CountryName, /*(int &)*/ m_iIP2CountryName);
	DDX_TreeCheck(pDX, IDC_EASTSHARE_OPTS, m_htiIP2CountryShowFlag, m_bIP2CountryShowFlag);
	//EastShare - added by AndCycle, IP to Country

	DDX_TreeRadio(pDX, IDC_EASTSHARE_OPTS, m_htiCreditSystem,  m_iCreditSystem); //EastShare - added by linekin , CreditSystem

	DDX_TreeCheck(pDX, IDC_EASTSHARE_OPTS, m_htiEnableEqualChanceForEachFile, m_bEnableEqualChanceForEachFile);//Morph - added by AndCycle, Equal Chance For Each File

	DDX_TreeCheck(pDX, IDC_EASTSHARE_OPTS, m_htiFollowTheMajority, m_bFollowTheMajority); // EastShare       - FollowTheMajority by AndCycle
	DDX_TreeCheck(pDX, IDC_EASTSHARE_OPTS, m_htiFairPlay, m_bFairPlay); //EastShare	- FairPlay by AndCycle

	// EastShare START - Added by TAHO, .met flies Control
	DDX_TreeRadio(pDX, IDC_EASTSHARE_OPTS, m_htiMetControl, m_iPurgeMode);
	DDX_TreeEdit(pDX, IDC_EASTSHARE_OPTS, m_htiKnownMet, m_iKnownMetDays);
	DDX_TreeCheck(pDX, IDC_EASTSHARE_OPTS, m_htiRemoveAichImmediatly, m_bRemoveAichImmediatly);
	// EastShare END - Added by TAHO, .met flies Control
	
	DDX_TreeCheck(pDX, IDC_EASTSHARE_OPTS, m_htiEnablePreferShareAll, m_bEnablePreferShareAll);//EastShare - PreferShareAll by AndCycle
	DDX_TreeCheck(pDX, IDC_EASTSHARE_OPTS, m_htiIsPayBackFirst, m_bIsPayBackFirst);//EastShare - added by AndCycle, Pay Back First
	DDX_TreeEdit(pDX, IDC_EASTSHARE_OPTS, m_htiPayBackFirstLimit, m_iPayBackFirstLimit); //MORPH - Added by SiRoB, Pay Back First Tweak
	DDX_TreeCheck(pDX, IDC_EASTSHARE_OPTS, m_htiOnlyDownloadCompleteFiles, m_bOnlyDownloadCompleteFiles);//EastShare - Added by AndCycle, Only download complete files v2.1 (shadow)
	DDX_TreeCheck(pDX, IDC_EASTSHARE_OPTS, m_htiSaveUploadQueueWaitTime, m_bSaveUploadQueueWaitTime);//Morph - added by AndCycle, Save Upload Queue Wait Time (MSUQWT)
	DDX_TreeCheck(pDX, IDC_EASTSHARE_OPTS, m_htiEnableChunkDots, m_bEnableChunkDots);//EastShare - Added by Pretender, Option for ChunkDots
}


BOOL CPPgEastShare::OnInitDialog()
{

	m_bEnablePreferShareAll = thePrefs.shareall;//EastShare - PreferShareAll by AndCycle
	m_bIsPayBackFirst = thePrefs.m_bPayBackFirst;//EastShare - added by AndCycle, Pay Back First
	m_iPayBackFirstLimit = thePrefs.m_iPayBackFirstLimit;//MORPH - Added by SiRoB, Pay Back First Tweak
	m_bOnlyDownloadCompleteFiles = thePrefs.m_bOnlyDownloadCompleteFiles;//EastShare - Added by AndCycle, Only download complete files v2.1 (shadow)
	m_bSaveUploadQueueWaitTime = thePrefs.m_bSaveUploadQueueWaitTime;//Morph - added by AndCycle, Save Upload Queue Wait Time (MSUQWT)
	m_bEnableChunkDots = thePrefs.m_bEnableChunkDots;//EastShare - Added by Pretender, Option for ChunkDots

	//EastShare Start - added by AndCycle, IP to Country
	m_iIP2CountryName = thePrefs.GetIP2CountryNameMode(); 
	m_bIP2CountryShowFlag = thePrefs.IsIP2CountryShowFlag();
	//EastShare End - added by AndCycle, IP to Country

	m_iCreditSystem = thePrefs.GetCreditSystem(); //EastShare - Added by linekin , CreditSystem 
	m_bEnableEqualChanceForEachFile = thePrefs.IsEqualChanceEnable();//Morph - added by AndCycle, Equal Chance For Each File
	m_bFollowTheMajority = thePrefs.IsFollowTheMajorityEnabled(); // EastShare       - FollowTheMajority by AndCycle
	m_bFairPlay = thePrefs.GetFairPlay() > 0 ; //EastShare	- FairPlay by AndCycle

	//EastShare START - Added by TAHO , .met file control
	m_iKnownMetDays = thePrefs.GetKnownMetDays();
	m_bRemoveAichImmediatly = thePrefs.DoRemoveAichImmediatly();
	//EastShare END   - Added by TAHO , .met file control
	
	CPropertyPage::OnInitDialog();
	InitTooltips(&m_ctrlTreeOptions);// MORPH leuk_he tooltipped.
	Localize();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPgEastShare::OnKillActive()
{
	// if prop page is closed by pressing VK_ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	return CPropertyPage::OnKillActive();
}

BOOL CPPgEastShare::OnApply()
{

	bool bRestartApp = false;

	// if prop page is closed by pressing VK_ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	
	if (!UpdateData())
		return FALSE;

	thePrefs.shareall = m_bEnablePreferShareAll;//EastShare - PreferShareAll by AndCycle
	thePrefs.m_bPayBackFirst = m_bIsPayBackFirst;//EastShare - added by AndCycle, Pay Back First
	thePrefs.m_iPayBackFirstLimit = (uint8)min(m_iPayBackFirstLimit,255);//MORPH - Added by SiRoB, Pay Back First Tweak, leuk_he is a uint8, limit it.
	thePrefs.m_bOnlyDownloadCompleteFiles = m_bOnlyDownloadCompleteFiles;//EastShare - Added by AndCycle, Only download complete files v2.1 (shadow)
	thePrefs.m_bEnableChunkDots = m_bEnableChunkDots;//EastShare - Added by Pretender, Option for ChunkDots

	//EastShare Start - added by AndCycle, IP to Country
	if(	(thePrefs.m_iIP2CountryNameMode != IP2CountryName_DISABLE || thePrefs.m_bIP2CountryShowFlag) !=
		(m_iIP2CountryName != IP2CountryName_DISABLE || m_bIP2CountryShowFlag)	){
		//check if need to load or unload DLL and ip table
		if(m_iIP2CountryName != IP2CountryName_DISABLE || m_bIP2CountryShowFlag){
			theApp.ip2country->Load();
		}
		else{
			theApp.ip2country->Unload();
		}
	}
	thePrefs.m_iIP2CountryNameMode = m_iIP2CountryName;
	thePrefs.m_bIP2CountryShowFlag = m_bIP2CountryShowFlag;
	theApp.ip2country->Refresh();//refresh passive windows
	//EastShare End - added by AndCycle, IP to Country

	//Morph - added by AndCycle, Save Upload Queue Wait Time (MSUQWT)
	if(m_bSaveUploadQueueWaitTime != thePrefs.m_bSaveUploadQueueWaitTime)	bRestartApp = true;
	thePrefs.m_bSaveUploadQueueWaitTime = m_bSaveUploadQueueWaitTime;
	//Morph - added by AndCycle, Save Upload Queue Wait Time (MSUQWT)

/*	theApp.emuledlg->serverwnd->ToggleDebugWindow();
	theApp.emuledlg->serverwnd->UpdateLogTabSelection(); */


	//MORPH START - Changed by SiRoB, reduce a little CPU usage for ratio count
	if(thePrefs.creditSystemMode != m_iCreditSystem){
		thePrefs.creditSystemMode = m_iCreditSystem; //EastShare - Added by linekin , CreditSystem 
		theApp.clientcredits->ResetCheckScoreRatio();
	}
	//MORPH END - Changed by SiRoB, reduce a little CPU usage for ratio count
	
	//Morph - added by AndCycle, Equal Chance For Each File
	thePrefs.m_bEnableEqualChanceForEachFile = m_bEnableEqualChanceForEachFile;
	//Morph - added by AndCycle, Equal Chance For Each File

	thePrefs.m_bFollowTheMajority = m_bFollowTheMajority; // EastShare       - FollowTheMajority by AndCycle
	thePrefs.m_iFairPlay = m_bFairPlay ? 1 : 0; //EastShare	- FairPlay by AndCycle

	//EastShare START - Added by TAHO , .met file control
	switch(m_iPurgeMode)
	{
	case 1: // partial purge
		{
			thePrefs.m_bPartiallyPurgeOldKnownFiles = true;
			thePrefs.m_bCompletlyPurgeOldKnownFiles = false;
		} break;
	case 2: // complete purge
		{
			thePrefs.m_bPartiallyPurgeOldKnownFiles = false;
			thePrefs.m_bCompletlyPurgeOldKnownFiles = true;
		} break;
	default:
		ASSERT( false ); //wth?! anyway, we proceed to don't purge
	case 0: // don't purge
		{
			thePrefs.m_bPartiallyPurgeOldKnownFiles = false;
			thePrefs.m_bCompletlyPurgeOldKnownFiles = false;
		} break;
	}
	thePrefs.SetKnownMetDays(m_iKnownMetDays);
	thePrefs.m_bRemoveAichImmediatly = m_bRemoveAichImmediatly;
	//EastShare END   - Added by TAHO , .met file control

	SetModified(FALSE);

	if (bRestartApp){
		AfxMessageBox(GetResString(IDS_SETTINGCHANGED_RESTART));
	}

	return CPropertyPage::OnApply();
}

void CPPgEastShare::OnHScroll(UINT /*nSBCode*/, UINT /*nPos*/, CScrollBar* /*pScrollBar*/) 
{
	SetModified(TRUE);
	CString temp;
}

void CPPgEastShare::Localize(void)
{	
	if(m_hWnd)
	{
		GetDlgItem(IDC_WARNINGEASTSHARE)->SetWindowText(GetResString(IDS_WARNINGMORPH));

		if (m_htiEnablePreferShareAll) m_ctrlTreeOptions.SetItemText(m_htiEnablePreferShareAll, GetResString(IDS_PREFER_SHARE_ALL));//EastShare - PreferShareAll by AndCycle
		if (m_htiIsPayBackFirst) m_ctrlTreeOptions.SetItemText(m_htiIsPayBackFirst, GetResString(IDS_PAYBACKFIRST));//EastShare - added by AndCycle, Pay Back First
		if (m_htiPayBackFirstLimit) m_ctrlTreeOptions.SetEditLabel(m_htiPayBackFirstLimit, GetResString(IDS_PAYBACKFIRSTLIMIT));//MORPH - Added by SiRoB, Pay Back First Tweak
		if (m_htiOnlyDownloadCompleteFiles) m_ctrlTreeOptions.SetItemText(m_htiOnlyDownloadCompleteFiles,GetResString(IDS_ONLY_DOWNLOAD_COMPLETE_FILES));//EastShare - Added by AndCycle, Only download complete files v2.1 (shadow)
		if (m_htiSaveUploadQueueWaitTime) m_ctrlTreeOptions.SetItemText(m_htiSaveUploadQueueWaitTime,GetResString(IDS_SAVE_UPLOAD_QUEUE_WAIT_TIME));//Morph - added by AndCycle, Save Upload Queue Wait Time (MSUQWT)
		if (m_htiEnableChunkDots) m_ctrlTreeOptions.SetItemText(m_htiEnableChunkDots, GetResString(IDS_ENABLE_CHUNKDOTS));//EastShare - Added by Pretender, Option for ChunkDots

		//EastShare START - Added By TAHO, .met file control // Modified by Pretender
		if (m_htiKnownMet) m_ctrlTreeOptions.SetEditLabel(m_htiKnownMet, (GetResString(IDS_EXPIRED_KNOWN)));
		if (m_htiRemoveAichImmediatly) m_ctrlTreeOptions.SetItemText(m_htiRemoveAichImmediatly, GetResString(IDS_REMOVE_AICH_IMMEDIATLY));
		//EastShare END - Added By TAHO, .met file control
		
		// MORPH START - leuk_he tooltipped
		SetTool(m_htiEnablePreferShareAll,IDS_PREFER_SHARE_ALL_TIP);
		SetTool(m_htiOnlyDownloadCompleteFiles,IDS_ONLY_DOWNLOAD_COMPLETE_FILES_TIP);
		SetTool(m_htiSaveUploadQueueWaitTime,IDS_SAVE_UPLOAD_QUEUE_WAIT_TIME_TIP);
		SetTool(m_htiEnableChunkDots,IDS_ENABLE_CHUNKDOTS_TIP);
		SetTool(m_htiEnableEqualChanceForEachFile,IDS_ECFEF_TIP);
		SetTool(m_htiFollowTheMajority, IDS_FOLLOW_THE_MAJORITY_TIP); // EastShare       - FollowTheMajority by AndCycle
		SetTool(m_htiFairPlay, IDS_FAIR_PLAY_TIP); //EastShare	- FairPlay by AndCycle
		SetTool(m_htiIP2CountryName,IDS_IP2COUNTRY_TIP);
		SetTool(m_htiIP2CountryName_DISABLE,IDS_DISABLED_TIP);
		SetTool(m_htiIP2CountryShowFlag,IDS_COUNTRYNAME_SHOWFLAG_TIP);
		SetTool(m_htiCreditSystem,IDS_CREDIT_SYSTEM_TIP);
		SetTool(m_htiOfficialCredit,IDS_OFFICIAL_CREDIT_TIP);
		SetTool(m_htiLovelaceCredit,IDS_LOVELACE_CREDIT_TIP);
	//	SetTool(m_htiRatioCredit,IDS_RATIO_CREDIT_TIP);
		SetTool(m_htiPawcioCredit,IDS_PAWCIO_CREDIT_TIP);
		SetTool(m_htiESCredit,IDS_EASTSHARE_CREDIT_TIP);
		SetTool(m_htiIsPayBackFirst,IDS_PAYBACKFIRST_TIP);
		SetTool(m_htiPayBackFirstLimit,IDS_PAYBACKFIRSTLIMI_TIP);
		//SetTool(m_htiMetControl,IDS_MET_FILE_CONTROL_TIP);
		SetTool(m_htiKnownMet,IDS_EXPIRED_KNOWN_TIP);
		SetTool(m_htiDontPurge,IDS_MET_PURGE_DONT_TIP);
		SetTool(m_htiPartiallyPurge,IDS_MET_PURGE_PART_TIP);
		SetTool(m_htiCompletelyPurge,IDS_MET_PURGE_COMPL_TIP);
		SetTool(m_htiRemoveAichImmediatly,IDS_REMOVE_AICH_TIP);
		// MORPH END - leuk_he tooltipped
	}	

}

void CPPgEastShare::OnDestroy()
{
	m_ctrlTreeOptions.DeleteAllItems();
	m_ctrlTreeOptions.DestroyWindow();
	m_bInitializedTreeOpts = false;

	m_htiEnablePreferShareAll = NULL; //EastShare - PreferShareAll by AndCycle
	m_htiIsPayBackFirst = NULL; //EastShare - added by AndCycle, Pay Back First
	m_htiPayBackFirstLimit = NULL; //MORPH - Added by SiRoB, Pay Back First Tweak
	m_htiOnlyDownloadCompleteFiles = NULL;//EastShare - Added by AndCycle, Only download complete files v2.1 (shadow)
	m_htiSaveUploadQueueWaitTime = NULL;//Morph - added by AndCycle, Save Upload Queue Wait Time (MSUQWT)
	m_htiEnableChunkDots = NULL; //EastShare - Added by Pretender, Option for ChunkDots

	//EastShare Start - added by AndCycle, IP to Country
	m_htiIP2CountryName = NULL;
	m_htiIP2CountryName_DISABLE = NULL;
	m_htiIP2CountryName_SHORT = NULL;
	m_htiIP2CountryName_MID = NULL;
	m_htiIP2CountryName_LONG = NULL;
	m_htiIP2CountryShowFlag = NULL;
	//EastShare End - added by AndCycle, IP to Country

	//EastShare START - Added by Pretender
	m_htiCreditSystem = NULL;
	m_htiOfficialCredit = NULL;
	m_htiLovelaceCredit = NULL;
	m_htiRatioCredit = NULL;
	m_htiPawcioCredit = NULL;
	m_htiESCredit = NULL;
	//EastShare END - Added by Pretender

	//Morph - added by AndCycle, Equal Chance For Each File
	m_htiEnableEqualChanceForEachFile = NULL;
	//Morph - added by AndCycle, Equal Chance For Each File

	m_htiFollowTheMajority = NULL; // EastShare       - FollowTheMajority by AndCycle

	m_htiFairPlay = NULL; //EastShare	- FairPlay by AndCycle

	//EastShare START - Added by TAHO, .met control
	m_htiMetControl = NULL;
	m_htiDontPurge = NULL;
	m_htiPartiallyPurge = NULL;
	m_htiCompletelyPurge = NULL;
	m_htiKnownMet = NULL;
	m_htiRemoveAichImmediatly = NULL;
	//EastShare END - Added by TAHO, .met control
	
	CPropertyPage::OnDestroy();
}
LRESULT CPPgEastShare::OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM /*lParam*/)
{
	if (wParam == IDC_EASTSHARE_OPTS){
		//TREEOPTSCTRLNOTIFY* pton = (TREEOPTSCTRLNOTIFY*)lParam;
		SetModified();
	}
	return 0;
}

void CPPgEastShare::OnHelp()
{
	//theApp.ShowHelp(0);
}

BOOL CPPgEastShare::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgEastShare::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}
