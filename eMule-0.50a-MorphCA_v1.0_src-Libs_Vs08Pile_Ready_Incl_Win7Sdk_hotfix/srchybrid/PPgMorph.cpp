// PpgMorph.cpp : implementation file
//

#include "stdafx.h"
#include "emule.h"
#include "PPgMorph.h"
#include "emuledlg.h"
#include "serverWnd.h" //MORPH - Added by SiRoB
#include "OtherFunctions.h"
#include "Scheduler.h" //MORPH - Added by SiRoB, Fix for Param used in scheduler
#include "StatisticsDlg.h" //MORPH - Added by SiRoB, Datarate Average Time Management
#include "searchDlg.h"
#include "UserMsgs.h"
#include "Log.h" //MORPH - Added by Stulle, Global Source Limit
#include "DownloadQueue.h" //MORPH - Added by Stulle, Global Source Limit
#include "TransferDlg.h"
#include "opcodes.h"
#include "sharedfilelist.h" //MORPH - Added by SiRoB, POWERSHARE Limit
#include "uploadqueue.h" //MORPH - Added by SiRoB, PS Internal prio
//EastShare Start - added by AndCycle, IP to Country
#include "ip2country.h"
//EastShare End - added by AndCycle, IP to Country
#include "Preferences.h"
//MORPH START 
#include "ClientCredits.h"	
//MORPH END   

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


///////////////////////////////////////////////////////////////////////////////
// CPPgMorph dialog

IMPLEMENT_DYNAMIC(CPPgMorph, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgMorph, CPropertyPage)
	ON_WM_HSCROLL()
    ON_WM_DESTROY()
	ON_MESSAGE(UM_TREEOPTSCTRL_NOTIFY, OnTreeOptsCtrlNotify)
END_MESSAGE_MAP()

CPPgMorph::CPPgMorph()

    : CPropertyPage(CPPgMorph::IDD)
	, m_ctrlTreeOptions(theApp.m_iDfltImageListColorFlags)
{
	m_bInitializedTreeOpts = false;
	m_htiDM = NULL;
	m_htiUM = NULL;
	m_htiDYNUP = NULL;
	m_htiDynUpOFF = NULL;
	m_htiDynUpSUC = NULL;
	m_htiDynUpUSS = NULL;
	m_htiDynUpAutoSwitching = NULL;//MORPH - Added by Yun.SF3, Auto DynUp changing
	m_htiMaxConnectionsSwitchBorder = NULL;//MORPH - Added by Yun.SF3, Auto DynUp changing
	m_htiSUCLog = NULL;
	m_htiSUCHigh = NULL;
	m_htiSUCLow = NULL;
	m_htiSUCPitch = NULL;
	m_htiSUCDrift = NULL;
	m_htiUSSLog = NULL;
	m_htiUSSUDP = NULL; //MORPH - Added by SiRoB, USS UDP preferency
	m_htiUSSLimit = NULL; // EastShare - Added by TAHO , USS limit
	m_htiUSSPingLimit = NULL; // EastShare - Added by TAHO, USS limit
    m_htiUSSPingTolerance = NULL;
    m_htiUSSGoingUpDivider = NULL;
    m_htiUSSGoingDownDivider = NULL;
    m_htiUSSNumberOfPings = NULL;
	m_htiPingDataSize = NULL; //MORPH leuk_he ICMP ping datasize <> 0 setting
	// ==> [MoNKi: -USS initial TTL-] - Stulle
	m_htiUSSTTL = NULL;
	// <== [MoNKi: -USS initial TTL-] - Stulle
	m_htiMinUpload = NULL;
	m_htiDisp = NULL;
	m_htiEnableDownloadInRed = NULL; //MORPH - Added by IceCream, show download in red
	m_htiEnableDownloadInBold = NULL; //MORPH - Added by SiRoB, show download in Bold
	m_htiShowClientPercentage = NULL; //MORPH - Added by SiRoB, show download in Bold
	//MORPH START - Added by Stulle, Global Source Limit
	m_htiGlobalHlGroup = NULL;
	m_htiGlobalHL = NULL;
	m_htiGlobalHlLimit = NULL;
	//MORPH END   - Added by Stulle, Global Source Limit
      m_htiShowModstring = NULL;
	m_htiSCC = NULL;
	//MORPH START - Added by SiRoB, khaos::categorymod+
	m_htiShowCatNames = NULL;
	m_htiSelectCat = NULL;
	m_htiUseActiveCat = NULL;
	m_htiAutoSetResOrder = NULL;
	m_htiShowA4AFDebugOutput = NULL;
	m_htiSmartA4AFSwapping = NULL;
	m_htiAdvA4AFMode = NULL;
	m_htiBalanceSources = NULL;
	m_htiStackSources = NULL;
	m_htiDisableAdvA4AF = NULL;
	m_htiSmallFileDLPush = NULL;
	m_htiResumeFileInNewCat = NULL;
	m_htiUseAutoCat = NULL;
	m_htiAddRemovedInc = NULL;
	m_htiUseSLS = NULL;
	// khaos::accuratetimerem+
	m_htiTimeRemainingMode = NULL;
	m_htiTimeRemBoth = NULL;
	m_htiTimeRemAverage = NULL;
	m_htiTimeRemRealTime = NULL;
	// khaos::accuratetimerem-
	//MORPH END - Added by SiRoB, khaos::categorymod+
	//MORPH START - Added by SiRoB, ICS Optional
	m_htiUseICS = NULL;
	//MORPH END   - Added by SiRoB, ICS Optional
	m_htiInfiniteQueue = NULL;	//Morph - added by AndCycle, SLUGFILLER: infiniteQueue
	m_htiDontRemoveSpareTrickleSlot = NULL; //Morph - added by AndCycle, Dont Remove Spare Trickle Slot
	m_htiUseDownloadOverhead = NULL ;//
	m_htiCompressLevel =NULL ;// morph settable compresslevel
	m_htiUseCompression=NULL ;// morph Use compress 

	m_htiDisplayFunnyNick = NULL;//MORPH - Added by SiRoB, Optionnal funnynick display
	m_htiDisableCatColors = NULL;
	//MORPH START - Added by SiRoB, Upload Splitting Class
	m_htiFriend = NULL;
	m_htiGlobalDataRateFriend = NULL;
	m_htiMaxGlobalDataRateFriend = NULL;
	m_htiMaxClientDataRateFriend = NULL;
	m_htiPowerShare = NULL;
	m_htiGlobalDataRatePowerShare = NULL;
	m_htiMaxGlobalDataRatePowerShare = NULL;
	m_htiMaxClientDataRatePowerShare = NULL;
	m_htiMaxClientDataRate = NULL;
	//MORPH END   - Added by SiRoB, Upload Splitting Class
	//MORPH START - Added by SiRoB, Datarate Average Time Management
	m_htiDownloadDataRateAverageTime = NULL;
	m_htiUploadDataRateAverageTime = NULL;
	//MORPH END   - Added by SiRoB, Datarate Average Time Management
	// ==> Slot Limit - Stulle
	if (thePrefs.GetSlotLimitThree())
		m_iSlotLimiter = 1;
	else if (thePrefs.GetSlotLimitNumB())
		m_iSlotLimiter = 2;
	else
		m_iSlotLimiter = 0;
	m_htiSlotLimitGroup = NULL;
	m_htiSlotLimitNone = NULL;
	m_htiSlotLimitThree = NULL;
	m_htiSlotLimitNumB = NULL;
	m_htiSlotLimitNum = NULL;
	// <== Slot Limit - Stulle

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
    // Added by MoNKi [ SlugFiller: -lowIdRetry- ]
	m_htiLowIdRetry = NULL;
	m_iLowIdRetry = 0;
	// End SlugFiller

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
    m_htiIP2CountryAutoUpdate = NULL;

	m_htiMisc = NULL;

	//EastShare START - Added by Pretender
	m_htiCreditSystem = NULL;
	//EastShare END - Added by Pretender

	//Morph - added by AndCycle, Equal Chance For Each File
	m_htiEnableEqualChanceForEachFile = NULL;
	//Morph - added by AndCycle, Equal Chance For Each File
	
	m_htiStaticIcon = NULL; //MORPH - Added, Static Tray Icon
}

CPPgMorph::~CPPgMorph()
{
}

void CPPgMorph::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MORPH_OPTS, m_ctrlTreeOptions);
	if (!m_bInitializedTreeOpts)
	{
        int iImgIP2Country = 8; //EastShare - added by AndCycle, IP to Country
		int iImgCS = 8; //EastShare Added by linekin, CreditSystem
		int iImgMisc = 8;
		int iImgUM = 8; // default icon
		int iImgDYNUP = 8; // default icon
		int iImgDM = 8;
		//MORPH START - Added by SiRoB, khaos::categorymod+
		int iImgSCC = 8;
		int iImgSAC = 8;
		int iImgA4AF = 8;
		int iImgTimeRem = 8;
		//MORPH END - Added by SiRoB, khaos::categorymod+
 		int iImgDisp = 8;
		int iImgGlobal = 8;
		//MORPH START - Added by SiRoB, Upload Splitting Class
		int iImgFriend = 8;
		int iImgPowerShare = 8;
		int iImgNormal = 8;
		//MORPH END   - Added by SiRoB, Upload Splitting Class
	    int iImgConTweaks = 8; // Stulle slotlimit
        int iImgSFM = 8;
		int iImgPerm = 8;
		int iImgPS = 8;
		CImageList* piml = m_ctrlTreeOptions.GetImageList(TVSIL_NORMAL);
		if (piml){
            iImgSFM = piml->Add(CTempIconLoader(_T("SHAREDFILES")));
			iImgPS = piml->Add(CTempIconLoader(_T("FILEPOWERSHARE"))); //MORPH - Added by SiRoB, POWERSHARE Limit
			iImgPerm = piml->Add(CTempIconLoader(_T("FILEPERMISSION"))); //MORPH - Added by SiRoB, Show Permission
            iImgIP2Country = piml->Add(CTempIconLoader(_T("COUNTRY"))); //EastShare - added by AndCycle, IP to Country
			iImgCS = piml->Add(CTempIconLoader(_T("STATSCLIENTS"))); // EastShare START - Added by Pretender, CS icon
            iImgMisc = piml->Add(CTempIconLoader(_T("PREFERENCES")));
			iImgUM = piml->Add(CTempIconLoader(_T("UPLOAD")));
			iImgDYNUP = piml->Add(CTempIconLoader(_T("SUC")));
			iImgDM = piml->Add(CTempIconLoader(_T("DOWNLOAD")));
			//MORPH START - Added by SiRoB, khaos::categorymod+
			iImgSCC = piml->Add(CTempIconLoader(_T("CATEGORY")));
			iImgSAC = piml->Add(CTempIconLoader(_T("ClientCompatible")));
			iImgA4AF = piml->Add(CTempIconLoader(_T("ADVA4AF")));
			// khaos::accuratetimerem+
			iImgTimeRem = piml->Add(CTempIconLoader(_T("STATSTIME")));
			// khaos::accuratetimerem-
			//MORPH END - Added by SiRoB, khaos::categorymod+
			iImgDisp = piml->Add(CTempIconLoader(_T("DISPLAY")));
			iImgGlobal = piml->Add(CTempIconLoader(_T("SEARCHMETHOD_GLOBAL")));
			//MORPH START - Added by SiRoB, Upload Splitting Class
			iImgFriend = piml->Add(CTempIconLoader(_T("FRIEND")));
			iImgPowerShare = piml->Add(CTempIconLoader(_T("FILEPOWERSHARE")));
			iImgNormal = piml->Add(CTempIconLoader(_T("ClientCompatible")));
			//MORPH END   - Added by SiRoB, Upload Splitting Class
             iImgConTweaks =  piml->Add(CTempIconLoader(_T("CONNECTION")));// ==> Slot Limit - Stulle
		}
		
		m_htiDM = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_DM), iImgDM, TVI_ROOT);
		//MORPH START - Added by SiRoB, khaos::categorymod+
		m_htiSCC = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_SCC), iImgSCC, m_htiDM);
		m_htiShowCatNames = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_CAT_SHOWCATNAME), m_htiSCC, m_bShowCatNames);
		m_htiSelectCat = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_CAT_SHOWSELCATDLG), m_htiSCC, m_bSelectCat);
		m_htiUseAutoCat = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_CAT_USEAUTOCAT), m_htiSCC, m_bUseAutoCat);
		m_htiUseActiveCat = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_CAT_USEACTIVE), m_htiSCC, m_bUseActiveCat);
		m_htiAutoSetResOrder = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_CAT_AUTORESUMEORD), m_htiSCC, m_bAutoSetResOrder);
		m_htiSmallFileDLPush = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_CAT_SMALLFILEDLPUSH), m_htiSCC, m_bSmallFileDLPush);
		m_htiResumeFileInNewCat = m_ctrlTreeOptions.InsertItem(GetResString(IDS_CAT_STARTFILESONADD), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiSCC);
		m_ctrlTreeOptions.AddEditBox(m_htiResumeFileInNewCat, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiAddRemovedInc = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ADD_REMOVED_INC), m_htiSCC, m_bAddRemovedInc);

		m_htiSAC = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_SAC), iImgSAC, m_htiDM);
		m_htiShowA4AFDebugOutput  = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_A4AF_SHOWDEBUG), m_htiSAC, m_bShowA4AFDebugOutput);
		m_htiSmartA4AFSwapping = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_A4AF_SMARTSWAP), m_htiSAC, m_bSmartA4AFSwapping);
		m_htiAdvA4AFMode = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_DEFAULT) + _T(" ") + GetResString(IDS_A4AF_ADVMODE), iImgA4AF, m_htiSAC);
		m_htiDisableAdvA4AF = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_A4AF_DISABLED), m_htiAdvA4AFMode, m_iAdvA4AFMode == 0);
		m_htiBalanceSources = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_A4AF_BALANCE), m_htiAdvA4AFMode, m_iAdvA4AFMode == 1);
		m_htiStackSources = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_A4AF_STACK), m_htiAdvA4AFMode, m_iAdvA4AFMode == 2);
		
		//m_ctrlTreeOptions.Expand(m_htiSCC, TVE_EXPAND);
		//m_ctrlTreeOptions.Expand(m_htiSAC, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiAdvA4AFMode, TVE_EXPAND);
		//MORPH END - Added by SiRoB, khaos::categorymod+
		
		// khaos::accuratetimerem+
		m_htiTimeRemainingMode = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_REMTIMEAVRREAL), iImgTimeRem, m_htiDM);
		m_htiTimeRemBoth = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_BOTH), m_htiTimeRemainingMode, m_iTimeRemainingMode == 0);
		m_htiTimeRemRealTime = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_REALTIME), m_htiTimeRemainingMode, m_iTimeRemainingMode == 1);
		m_htiTimeRemAverage = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_AVG), m_htiTimeRemainingMode, m_iTimeRemainingMode == 2);
		//m_ctrlTreeOptions.Expand(m_htiTimeRemainingMode, TVE_EXPAND); // khaos::accuratetimerem+
		// khaos::accuratetimerem-
        m_htiDisp = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_PW_DISPLAY), iImgDisp, m_htiDM);
		m_htiEnableDownloadInRed = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DOWNLOAD_IN_RED), m_htiDisp, m_bEnableDownloadInRed); //MORPH - Added by SiRoB, show download in Bold
		m_htiEnableDownloadInBold = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DOWNLOAD_IN_BOLD), m_htiDisp, m_bEnableDownloadInBold); //MORPH - Added by SiRoB, show download in Bold
		m_htiShowClientPercentage = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_CLIENTPERCENTAGE), m_htiDisp, m_bShowClientPercentage);
		// MORPH START leuk_he disable catcolor
        m_htiDisableCatColors= m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DISABLECATCOLORS), m_htiDisp, m_bDisableCatColors);
	    // MORPH END   leuk_he disable catcolor

		//MORPH START - Added by SiRoB, Datarate Average Time Management
		m_htiDownloadDataRateAverageTime = m_ctrlTreeOptions.InsertItem(GetResString(IDS_DATARATEAVERAGETIME), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDisp);
		m_ctrlTreeOptions.AddEditBox(m_htiDownloadDataRateAverageTime, RUNTIME_CLASS(CNumTreeOptionsEdit));
		//MORPH END   - Added by SiRoB, Datarate Average Time Management
		//MORPH START - Added by Stulle, Global Source Limit
		m_htiGlobalHlGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_GLOBAL_HL), iImgGlobal, m_htiDM);
		m_htiGlobalHL = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SUC_ENABLED), m_htiGlobalHlGroup, m_bGlobalHL);
		m_htiGlobalHlLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_GLOBAL_HL_LIMIT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiGlobalHlGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiGlobalHlLimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		//MORPH END   - Added by Stulle, Global Source Limit
		//MORPH START - Added by SiRoB, khaos::categorymod+
		m_htiUseSLS = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SLS_USESLS), m_htiDM, m_bUseSLS);
		//MORPH END - Added by SiRoB, khaos::categorymod+
		//MORPH START - Added by SiRoB, ICS Optional
		m_htiUseICS = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ICS_USEICS), m_htiDM, m_bUseICS);
		//MORPH END   - Added by SiRoB, ICS Optional

		CString Buffer;
		m_htiUM = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_UM), iImgUM, TVI_ROOT);

		//MORPH START - Added by SiRoB, Pay Back First Tweak
		m_htiCreditSystem = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_PAYBACKFIRST), iImgCS, m_htiUM);	
		m_htiIsPayBackFirst = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PAYBACKFIRST), m_htiCreditSystem, m_bIsPayBackFirst);//EastShare - added by AndCycle, Pay Back First
		m_htiPayBackFirstLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_PAYBACKFIRSTLIMIT),TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,m_htiIsPayBackFirst);
		m_ctrlTreeOptions.AddEditBox(m_htiPayBackFirstLimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_ctrlTreeOptions.Expand(m_htiIsPayBackFirst, TVE_EXPAND);
		//MORPH END   - Added by SiRoB, Pay Back First Tweak

		m_htiDYNUP = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_DYNUPLOAD), iImgDYNUP, m_htiUM);
		
		m_htiDynUpOFF = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_DISABLED), m_htiDYNUP, m_iDynUpMode == 0);
		m_htiDynUpSUC = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SUC), m_htiDYNUP, m_iDynUpMode == 1);
		m_htiSUCLog = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SUC_LOG), m_htiDynUpSUC, m_bSUCLog);
		Buffer.Format(GetResString(IDS_SUC_HIGH),900);
		m_htiSUCHigh = m_ctrlTreeOptions.InsertItem(Buffer, TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDynUpSUC);
		m_ctrlTreeOptions.AddEditBox(m_htiSUCHigh, RUNTIME_CLASS(CNumTreeOptionsEdit));
		Buffer.Format(GetResString(IDS_SUC_LOW),600);
		m_htiSUCLow = m_ctrlTreeOptions.InsertItem(Buffer, TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDynUpSUC);
		m_ctrlTreeOptions.AddEditBox(m_htiSUCLow, RUNTIME_CLASS(CNumTreeOptionsEdit));
		Buffer.Format(GetResString(IDS_SUC_PITCH),3000);
		m_htiSUCPitch = m_ctrlTreeOptions.InsertItem(Buffer, TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDynUpSUC);
		m_ctrlTreeOptions.AddEditBox(m_htiSUCPitch, RUNTIME_CLASS(CNumTreeOptionsEdit));
		Buffer.Format(GetResString(IDS_SUC_DRIFT),50);
		m_htiSUCDrift = m_ctrlTreeOptions.InsertItem(Buffer, TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDynUpSUC);
		m_ctrlTreeOptions.AddEditBox(m_htiSUCDrift, RUNTIME_CLASS(CNumTreeOptionsEdit));
		
		m_htiDynUpUSS = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_USS), m_htiDYNUP, m_iDynUpMode == 2);
		m_htiUSSLog = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_USS_LOG), m_htiDynUpUSS, m_bUSSLog);
		m_htiUSSUDP = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_USS_UDP), m_htiDynUpUSS, m_bUSSUDP); //MORPH - Added by SiRoB, USS UDP preferency
		
		// EastShare START - Added by TAHO, USS limit
		m_htiUSSLimit = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_USS_USEMAXPING), m_htiDynUpUSS, m_bUSSLimit);
		//Buffer.Format("Max ping value (ms): ",800); //modified by Pretender
		Buffer.Format(GetResString(IDS_USS_MAXPING),200); //Added by Pretender
		m_htiUSSPingLimit = m_ctrlTreeOptions.InsertItem(Buffer, TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDynUpUSS);
		m_ctrlTreeOptions.AddEditBox(m_htiUSSPingLimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// EastShare END - Added by TAHO, USS limit

		Buffer.Format(GetResString(IDS_USS_PINGTOLERANCE),800);
		m_htiUSSPingTolerance = m_ctrlTreeOptions.InsertItem(Buffer, TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDynUpUSS);
		m_ctrlTreeOptions.AddEditBox(m_htiUSSPingTolerance, RUNTIME_CLASS(CNumTreeOptionsEdit));
		Buffer.Format(GetResString(IDS_USS_GOINGUPDIVIDER),1000);
		m_htiUSSGoingUpDivider = m_ctrlTreeOptions.InsertItem(Buffer, TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDynUpUSS);
		m_ctrlTreeOptions.AddEditBox(m_htiUSSGoingUpDivider, RUNTIME_CLASS(CNumTreeOptionsEdit));
		Buffer.Format(GetResString(IDS_USS_GOINGDOWNDIVIDER),1000);
		m_htiUSSGoingDownDivider = m_ctrlTreeOptions.InsertItem(Buffer, TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDynUpUSS);
		m_ctrlTreeOptions.AddEditBox(m_htiUSSGoingDownDivider, RUNTIME_CLASS(CNumTreeOptionsEdit));
		Buffer.Format(GetResString(IDS_USS_NUMBEROFPINGS),1);
		m_htiUSSNumberOfPings = m_ctrlTreeOptions.InsertItem(Buffer, TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDynUpUSS);
		m_ctrlTreeOptions.AddEditBox(m_htiUSSNumberOfPings, RUNTIME_CLASS(CNumTreeOptionsEdit));
        m_htiPingDataSize = m_ctrlTreeOptions.InsertItem(GetResString(IDS_USSPINGDATASIZE), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDynUpUSS);// MORPH leuk_he ICMP ping datasize <> 0 setting
		m_ctrlTreeOptions.AddEditBox(m_htiPingDataSize , RUNTIME_CLASS(CNumTreeOptionsEdit)); //MORPH leuk_he ICMP ping datasize <> 0 setting
		// ==> [MoNKi: -USS initial TTL-] - Stulle
		m_htiUSSTTL = m_ctrlTreeOptions.InsertItem(GetResString(IDS_USS_INITIAL_TTL), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDynUpUSS);
		m_ctrlTreeOptions.AddEditBox(m_htiUSSTTL, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// <== [MoNKi: -USS initial TTL-] - Stulle
        
		//MORPH START - Added by Yun.SF3, Auto DynUp changing
		m_htiDynUpAutoSwitching = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_AUTODYNUPSWITCHING), m_htiDYNUP, m_iDynUpMode == 3);
		Buffer.Format(GetResString(IDS_MAXCONNECTIONSSWITCHBORDER), 20);
		m_htiMaxConnectionsSwitchBorder = m_ctrlTreeOptions.InsertItem(Buffer, TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDynUpAutoSwitching);
		m_ctrlTreeOptions.AddEditBox(m_htiMaxConnectionsSwitchBorder, RUNTIME_CLASS(CNumTreeOptionsEdit));
		//MORPH END - Added by Yun.SF3, Auto DynUp changing
	
		m_htiMinUpload = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MINUPLOAD), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDYNUP);
		m_ctrlTreeOptions.AddEditBox(m_htiMinUpload, RUNTIME_CLASS(CNumTreeOptionsEdit));
				
		m_htiUpDisplay = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_PW_DISPLAY), iImgDisp, m_htiUM);

		//MORPH START - Added by SiRoB, Datarate Average Time Management
		m_htiUploadDataRateAverageTime = m_ctrlTreeOptions.InsertItem(GetResString(IDS_DATARATEAVERAGETIME), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiUpDisplay);
		m_ctrlTreeOptions.AddEditBox(m_htiUploadDataRateAverageTime, RUNTIME_CLASS(CNumTreeOptionsEdit));
		//MORPH END   - Added by SiRoB, Datarate Average Time Management

		//MORPH START - Added by SiRoB, Upload Splitting Class
		m_htiFriend = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_PW_FRIENDS), iImgFriend, m_htiUM);
		m_htiGlobalDataRateFriend = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MINDATARATEFRIEND), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiFriend);
		m_ctrlTreeOptions.AddEditBox(m_htiGlobalDataRateFriend, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiMaxGlobalDataRateFriend = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MAXDATARATEFRIEND), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiFriend);
		m_ctrlTreeOptions.AddEditBox(m_htiMaxGlobalDataRateFriend, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiMaxClientDataRateFriend = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MAXCLIENTDATARATEFRIEND), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiFriend);
		m_ctrlTreeOptions.AddEditBox(m_htiMaxClientDataRateFriend, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiPowerShare = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_POWERSHARE), iImgPowerShare, m_htiUM);
		m_htiGlobalDataRatePowerShare = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MINDATARATEPOWERSHARE), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiPowerShare);
		m_ctrlTreeOptions.AddEditBox(m_htiGlobalDataRatePowerShare, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiMaxGlobalDataRatePowerShare = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MAXDATARATEPOWERSHARE), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiPowerShare);
		m_ctrlTreeOptions.AddEditBox(m_htiMaxGlobalDataRatePowerShare, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiMaxClientDataRatePowerShare = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MAXCLIENTDATARATEPOWERSHARE), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiPowerShare);
		m_ctrlTreeOptions.AddEditBox(m_htiMaxClientDataRatePowerShare, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiMaxClientDataRate = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MAXCLIENTDATARATE), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiUM);
		m_ctrlTreeOptions.AddEditBox(m_htiMaxClientDataRate, RUNTIME_CLASS(CNumTreeOptionsEdit));
		//MORPH END   - Added by SiRoB, Upload Splitting Class
		
		// ==> Slot Limit - Stulle
		m_htiSlotLimitGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_SLOT_LIMIT_GROUP), iImgConTweaks, m_htiUM);
		m_htiSlotLimitNone = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SLOT_LIMIT_NONE), m_htiSlotLimitGroup, m_iSlotLimiter == 0);
		m_htiSlotLimitThree = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SLOT_LIMIT_THREE), m_htiSlotLimitGroup, m_iSlotLimiter == 1);
		m_htiSlotLimitNumB = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SLOT_LIMIT_NUM_B), m_htiSlotLimitGroup, m_iSlotLimiter == 2);
		m_htiSlotLimitNum = m_ctrlTreeOptions.InsertItem(GetResString(IDS_SLOT_LIMIT_NUM), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiSlotLimitNumB);
		m_ctrlTreeOptions.AddEditBox(m_htiSlotLimitNum, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// <== Slot Limit - Stulle

		m_htiInfiniteQueue = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_INFINITEQUEUE), m_htiUM, m_bInfiniteQueue);	//Morph - added by AndCycle, SLUGFILLER: infiniteQueue
		m_htiDontRemoveSpareTrickleSlot = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DONTREMOVESPARETRICKLESLOT), m_htiUM, m_bDontRemoveSpareTrickleSlot); //Morph - added by AndCycle, Dont Remove Spare Trickle Slot
		m_htiUseDownloadOverhead = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_USEDOWNLOADOVERHEAD), m_htiUM, m_bUseDownloadOverhead); //Morph - leuk_he use download overhead in upload
		m_htiCompressLevel = m_ctrlTreeOptions.InsertItem(GetResString(IDS_COMPRESSLEVEL), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiUM);
		m_ctrlTreeOptions.AddEditBox(m_htiCompressLevel, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiUseCompression= m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_USECOMPRESS), m_htiUM, m_bUseCompression); //MORPH - added by Commander, Show WC Session stats

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

		m_ctrlTreeOptions.Expand(m_htiPowershareLimited, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiPermCommunity, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiSpreadbar, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiHideOS, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiPermissions, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiPowershareMode, TVE_EXPAND);

        //EastShare Start - added by AndCycle, IP to Country
		m_htiIP2CountryName = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_IP2COUNTRY), iImgIP2Country, TVI_ROOT);
		m_htiIP2CountryName_DISABLE = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_DISABLED), m_htiIP2CountryName, m_iIP2CountryName == IP2CountryName_DISABLE);
		m_htiIP2CountryName_SHORT = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_COUNTRYNAME_SHORT), m_htiIP2CountryName, m_iIP2CountryName == IP2CountryName_SHORT);
		m_htiIP2CountryName_MID = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_COUNTRYNAME_MID), m_htiIP2CountryName, m_iIP2CountryName == IP2CountryName_MID);
		m_htiIP2CountryName_LONG = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_COUNTRYNAME_LONG), m_htiIP2CountryName, m_iIP2CountryName == IP2CountryName_LONG);
		m_htiIP2CountryShowFlag = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_COUNTRYNAME_SHOWFLAG), m_htiIP2CountryName, m_bIP2CountryShowFlag);
		//EastShare End - added by AndCycle, IP to Country
		m_htiIP2CountryAutoUpdate = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_AUTOUPIP2COUNTRY), m_htiIP2CountryName, m_bIP2CountryAutoUpdate);

        m_htiMisc = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_PW_MISC), iImgMisc, TVI_ROOT);

		m_htiShowModstring = m_ctrlTreeOptions.InsertCheckBox(_T("Show Usernick - Runtime on title"), m_htiMisc, m_bShowModstring); 

		m_htiDisplayFunnyNick = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DISPLAYFUNNYNICK), m_htiMisc, m_bFunnyNick);//MORPH - Added by SiRoB, Optionnal funnynick display
		// Mighty Knife: Report hashing files, Log friendlist activities
	    m_htiReportHashingFiles = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MORPH_RFHA), m_htiMisc, m_bReportHashingFiles);
	    m_htiLogFriendlistActivities = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MORPH_RAIF), m_htiMisc, m_bLogFriendlistActivities);
		// [end] Mighty Knife

		// Mighty Knife: Static server handling
	    m_htiDontRemoveStaticServers = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MORPH_KSSERV), m_htiMisc, m_bDontRemoveStaticServers);
		// [end] Mighty Knife

		m_htiEnablePreferShareAll = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PREFER_SHARE_ALL), m_htiMisc, m_bEnablePreferShareAll);//EastShare - PreferShareAll by AndCycle
		m_htiOnlyDownloadCompleteFiles = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ONLY_DOWNLOAD_COMPLETE_FILES), m_htiMisc, m_bOnlyDownloadCompleteFiles);//EastShare - Added by AndCycle, Only download complete files v2.1 (shadow)
		m_htiSaveUploadQueueWaitTime = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SAVE_UPLOAD_QUEUE_WAIT_TIME), m_htiMisc, m_bSaveUploadQueueWaitTime);//Morph - added by AndCycle, Save Upload Queue Wait Time (MSUQWT)
		m_htiEnableChunkDots = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ENABLE_CHUNKDOTS),m_htiMisc, m_bEnableChunkDots);//EastShare - Added by Pretender, Option for ChunkDots

        // Added by MoNKi [ SlugFiller: -lowIdRetry- ]
		m_htiLowIdRetry = m_ctrlTreeOptions.InsertItem(_T("LowID retries"), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiMisc);
		m_ctrlTreeOptions.AddEditBox(m_htiLowIdRetry, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// End SlugFiller

		//Morph - added by AndCycle, Equal Chance For Each File
		m_htiEnableEqualChanceForEachFile = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ECFEF), m_htiMisc, m_bEnableEqualChanceForEachFile);
		//Morph - added by AndCycle, Equal Chance For Each File

		m_htiStaticIcon = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_STATIC_ICON),m_htiMisc, m_bStaticIcon); //MORPH - Added, Static Tray Icon
		
		m_ctrlTreeOptions.SendMessage(WM_VSCROLL, SB_TOP);
		m_bInitializedTreeOpts = true;
	}
	DDX_TreeRadio(pDX, IDC_MORPH_OPTS, m_htiDYNUP, m_iDynUpMode);
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiMaxConnectionsSwitchBorder, m_iMaxConnectionsSwitchBorder);//MORPH - Added by Yun.SF3, Auto DynUp changing
	DDV_MinMaxInt(pDX, m_iMaxConnectionsSwitchBorder, 20 , 60000);//MORPH - Added by Yun.SF3, Auto DynUp changing
	
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiSUCLog, m_bSUCLog);
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiSUCHigh, m_iSUCHigh);
	DDV_MinMaxInt(pDX, m_iSUCHigh, 350, 1000);
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiSUCLow, m_iSUCLow);
	DDV_MinMaxInt(pDX, m_iSUCLow, 350, m_iSUCHigh);
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiSUCPitch, m_iSUCPitch);
	DDV_MinMaxInt(pDX, m_iSUCPitch, 2500, 10000);
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiSUCDrift, m_iSUCDrift);
	DDV_MinMaxInt(pDX, m_iSUCDrift, 0, 100);
	
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiUSSLog, m_bUSSLog);
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiUSSUDP, m_bUSSUDP); //MORPH - Added by SiRoB, USS UDP preferency
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, 	m_htiPingDataSize , m_sPingDataSize); // MORPH leuk_he ICMP ping datasize <> 0 setting

	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiUSSLimit, m_bUSSLimit); // EastShare - Added by TAHO, USS limit
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiUSSPingLimit, m_iUSSPingLimit); // EastShare - Added by TAHO, USS limit
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiUSSPingTolerance, m_iUSSPingTolerance);
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiUSSGoingUpDivider, m_iUSSGoingUpDivider);
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiUSSGoingDownDivider, m_iUSSGoingDownDivider);
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiUSSNumberOfPings, m_iUSSNumberOfPings);
	// ==> [MoNKi: -USS initial TTL-] - Stulle
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiUSSTTL, m_iUSSTTL);
	DDV_MinMaxInt(pDX, m_iUSSTTL, 1, 20);
	// <== [MoNKi: -USS initial TTL-] - Stulle
	
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiMinUpload, m_iMinUpload);
	if(thePrefs.GetMaxGraphUploadRate(false)!=1) // work arround bug that would always cause a warning to pop up.
		DDV_MinMaxInt(pDX, m_iMinUpload, 1, thePrefs.GetMaxGraphUploadRate(false));

	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiEnableDownloadInRed, m_bEnableDownloadInRed); //MORPH - Added by IceCream, show download in red
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiEnableDownloadInBold, m_bEnableDownloadInBold); //MORPH - Added by SiRoB, show download in Bold
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiShowClientPercentage, m_bShowClientPercentage);
	// MORPH START leuk_he disable catcolor
    DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiDisableCatColors, m_bDisableCatColors); //MORPH - Added by SiRoB, show download in Bold
    // MORPH END   leuk_he disable catcolor

	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiShowModstring, m_bShowModstring); 

	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiInfiniteQueue, m_bInfiniteQueue);	//Morph - added by AndCycle, SLUGFILLER: infiniteQueue
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiDontRemoveSpareTrickleSlot, m_bDontRemoveSpareTrickleSlot); //Morph - added by AndCycle, Dont Remove Spare Trickle Slot
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiUseDownloadOverhead, m_bUseDownloadOverhead);//Morph - leuk_he use download overhead in upload
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiCompressLevel, m_iCompressLevel); //Morph - Compresslevel
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiUseCompression, m_bUseCompression); //Morph - Compresslevel
	DDV_MinMaxInt(pDX, m_iCompressLevel,1,9);//Morph - Compresslevel
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiDisplayFunnyNick, m_bFunnyNick);//MORPH - Added by SiRoB, Optionnal funnynick display

	//MORPH START - Added by SiRoB, Datarate Average Time Management
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiDownloadDataRateAverageTime, m_iDownloadDataRateAverageTime);//MORPH - Added by SiRoB, Upload Splitting Class
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiUploadDataRateAverageTime, m_iUploadDataRateAverageTime);//MORPH - Added by SiRoB, Upload Splitting Class
	//MORPH END   - Added by SiRoB, Datarate Average Time Management
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiGlobalDataRateFriend, m_iGlobalDataRateFriend);//MORPH - Added by SiRoB, Upload Splitting Class
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiMaxGlobalDataRateFriend, m_iMaxGlobalDataRateFriend);//MORPH - Added by SiRoB, Upload Splitting Class
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiMaxClientDataRateFriend, m_iMaxClientDataRateFriend);//MORPH - Added by SiRoB, Upload Splitting Class
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiGlobalDataRatePowerShare, m_iGlobalDataRatePowerShare);//MORPH - Added by SiRoB, Upload Splitting Class
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiMaxGlobalDataRatePowerShare, m_iMaxGlobalDataRatePowerShare);//MORPH - Added by SiRoB, Upload Splitting Class
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiMaxClientDataRatePowerShare, m_iMaxClientDataRatePowerShare);//MORPH - Added by SiRoB, Upload Splitting Class
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiMaxClientDataRate, m_iMaxClientDataRate);//MORPH - Added by SiRoB, Upload Splitting Class
	// ==> Slot Limit - Stulle
	DDX_TreeRadio(pDX, IDC_MORPH_OPTS, m_htiSlotLimitGroup, m_iSlotLimiter);
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiSlotLimitNum, m_iSlotLimitNum);
	DDV_MinMaxInt(pDX, m_iSlotLimitNum, 60, 255);
	// <== Slot Limit - Stulle

	//MORPH START - Added by SiRoB, khaos::categorymod+
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiResumeFileInNewCat, m_iResumeFileInNewCat);
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiShowCatNames, m_bShowCatNames);
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiSelectCat, m_bSelectCat);
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiUseActiveCat, m_bUseActiveCat);
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiAutoSetResOrder, m_bAutoSetResOrder);
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiSmallFileDLPush, m_bSmallFileDLPush);
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiShowA4AFDebugOutput, m_bShowA4AFDebugOutput);
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiSmartA4AFSwapping, m_bSmartA4AFSwapping);
	DDX_TreeRadio(pDX, IDC_MORPH_OPTS, m_htiAdvA4AFMode, m_iAdvA4AFMode);
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiUseAutoCat, m_bUseAutoCat);
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiAddRemovedInc, m_bAddRemovedInc);
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiUseSLS, m_bUseSLS);
	// khaos::accuratetimerem+
	DDX_TreeRadio(pDX, IDC_MORPH_OPTS, m_htiTimeRemainingMode, m_iTimeRemainingMode);
	// khaos::accuratetimerem-
	//MORPH START - Added by Stulle, Global Source Limit
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiGlobalHL, m_bGlobalHL);
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiGlobalHlLimit, m_iGlobalHL);
	DDV_MinMaxInt(pDX, m_iGlobalHL, 1000, MAX_GSL);
	//MORPH END   - Added by Stulle, Global Source Limit
	//MORPH END - Added by SiRoB, khaos::categorymod+
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiUseICS, m_bUseICS);//MORPH - Added by SiRoB, ICS Optional

	// Mighty Knife: Report hashing files, Log friendlist activities
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiReportHashingFiles, m_bReportHashingFiles); 
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiLogFriendlistActivities, m_bLogFriendlistActivities); 
	// [end] Mighty Knife

	// Mighty Knife: Static server handling
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiDontRemoveStaticServers, m_bDontRemoveStaticServers); 
	// [end] Mighty Knife
   
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
	DDV_MinMaxInt(pDX, m_iShareOnlyTheNeed, 0, INT_MAX);
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

    //this is bad using enum for radio button...need (int &) ^*&^#*^$(, by AndCycle

    // Added by MoNKi [ SlugFiller: -lowIdRetry- ]
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiLowIdRetry, m_iLowIdRetry);
	DDV_MinMaxInt(pDX, m_iLowIdRetry, 0, 255);
	// End SlugFiller

	//EastShare - added by AndCycle, IP to Country
	DDX_TreeRadio(pDX, IDC_MORPH_OPTS, m_htiIP2CountryName, /*(int &)*/ m_iIP2CountryName);
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiIP2CountryShowFlag, m_bIP2CountryShowFlag);
	//EastShare - added by AndCycle, IP to Country
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiIP2CountryAutoUpdate, m_bIP2CountryAutoUpdate);

	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiEnableEqualChanceForEachFile, m_bEnableEqualChanceForEachFile);//Morph - added by AndCycle, Equal Chance For Each File

	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiEnablePreferShareAll, m_bEnablePreferShareAll);//EastShare - PreferShareAll by AndCycle
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiIsPayBackFirst, m_bIsPayBackFirst);//EastShare - added by AndCycle, Pay Back First
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiPayBackFirstLimit, m_iPayBackFirstLimit); //MORPH - Added by SiRoB, Pay Back First Tweak
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiOnlyDownloadCompleteFiles, m_bOnlyDownloadCompleteFiles);//EastShare - Added by AndCycle, Only download complete files v2.1 (shadow)
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiSaveUploadQueueWaitTime, m_bSaveUploadQueueWaitTime);//Morph - added by AndCycle, Save Upload Queue Wait Time (MSUQWT)
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiEnableChunkDots, m_bEnableChunkDots);//EastShare - Added by Pretender, Option for ChunkDots

	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiStaticIcon, m_bStaticIcon); //MORPH - Added, Static Tray Icon
	
}

BOOL CPPgMorph::OnInitDialog()
{
	if (thePrefs.isautodynupswitching)
		m_iDynUpMode = 3;
	else if (thePrefs.m_bSUCEnabled)
		m_iDynUpMode = 1;
	else if (thePrefs.m_bDynUpEnabled)
		m_iDynUpMode = 2;
	else
		m_iDynUpMode = 0;
	m_iMaxConnectionsSwitchBorder = thePrefs.maxconnectionsswitchborder;//MORPH - Added by Yun.SF3, Auto DynUp changing
	m_bSUCLog =  thePrefs.m_bSUCLog;
	m_iSUCHigh = thePrefs.m_iSUCHigh;
	m_iSUCLow = thePrefs.m_iSUCLow;
	m_iSUCPitch = thePrefs.m_iSUCPitch;
	m_iSUCDrift = thePrefs.m_iSUCDrift;;
	m_bUSSLog = thePrefs.m_bDynUpLog;
	m_bUSSUDP = thePrefs.m_bUSSUDP; //MORPH - Added by SiRoB, USS UDP preferency
	m_sPingDataSize = thePrefs.m_sPingDataSize; // MORPH leuk_he ICMP ping datasize <> 0 setting
	m_bUSSLimit = thePrefs.m_bDynUpUseMillisecondPingTolerance; // EastShare - Added by TAHO, USS limit
	m_iUSSPingLimit = thePrefs.m_iDynUpPingToleranceMilliseconds; // EastShare - Added by TAHO, USS limit
    m_iUSSPingTolerance = thePrefs.m_iDynUpPingTolerance;
    m_iUSSGoingUpDivider = thePrefs.m_iDynUpGoingUpDivider;
    m_iUSSGoingDownDivider = thePrefs.m_iDynUpGoingDownDivider;
    m_iUSSNumberOfPings = thePrefs.m_iDynUpNumberOfPings;
	// ==> [MoNKi: -USS initial TTL-] - Stulle
	m_iUSSTTL = thePrefs.GetUSSInitialTTL();
	// <== [MoNKi: -USS initial TTL-] - Stulle
	m_iMinUpload = thePrefs.minupload;
	m_bEnableDownloadInRed = thePrefs.enableDownloadInRed; //MORPH - Added by IceCream, show download in red
	m_bEnableDownloadInBold = thePrefs.m_bShowActiveDownloadsBold; //MORPH - Added by SiRoB, show download in Bold
	m_bShowClientPercentage = thePrefs.m_bShowClientPercentage;
	// MORPH START leuk_he disable catcolor
	m_bDisableCatColors   =  thePrefs.m_bDisableCatColors ;
	// MORPH START leuk_he disable catcolor

	//MORPH START - Added by SiRoB, Datarate Average Time Management
	m_iDownloadDataRateAverageTime = thePrefs.m_iDownloadDataRateAverageTime/1000;
	m_iUploadDataRateAverageTime = thePrefs.m_iUploadDataRateAverageTime/1000;
	//MORPH END   - Added by SiRoB, Datarate Average Time Management
	//MORPH START - Added by Stulle, Global Source Limit
	m_bGlobalHL = thePrefs.IsUseGlobalHL();
	m_iGlobalHL = thePrefs.GetGlobalHL();
	//MORPH END   - Added by Stulle, Global Source Limit

	m_bShowModstring = thePrefs.m_bShowRuntimeOnTitle; 

	m_bInfiniteQueue = thePrefs.infiniteQueue;	//Morph - added by AndCycle, SLUGFILLER: infiniteQueue
	m_bDontRemoveSpareTrickleSlot = thePrefs.m_bDontRemoveSpareTrickleSlot; //Morph - added by AndCycle, Dont Remove Spare Trickle Slot
	m_bUseDownloadOverhead= thePrefs.m_bUseDownloadOverhead; // MORPH leuk_he include download overhead in upload stats
    m_iCompressLevel = thePrefs.m_iCompressLevel; //Compresslevel
	m_bUseCompression = thePrefs.m_bUseCompression; // use compression
	m_bFunnyNick = thePrefs.m_bFunnyNick;//MORPH - Added by SiRoB, Optionnal funnynick display
	//MORPH START - Added by SiRoB, Upload Splitting Class
	m_iGlobalDataRateFriend = thePrefs.globaldataratefriend;
	m_iMaxGlobalDataRateFriend = thePrefs.maxglobaldataratefriend;
	m_iMaxClientDataRateFriend = thePrefs.maxclientdataratefriend;
	m_iGlobalDataRatePowerShare = thePrefs.globaldataratepowershare;
	m_iMaxGlobalDataRatePowerShare = thePrefs.maxglobaldataratepowershare;
	m_iMaxClientDataRatePowerShare = thePrefs.maxclientdataratepowershare;
	m_iMaxClientDataRate = thePrefs.maxclientdatarate;
	//MORPH END   - Added by SiRoB, Upload Splitting Class
	m_iSlotLimitNum = thePrefs.GetSlotLimitNum(); // Slot Limit - Stulle

	//MORPH START - Added by SiRoB, khaos::categorymod+
	m_bShowCatNames = thePrefs.ShowCatNameInDownList();
	m_bSelectCat = thePrefs.SelectCatForNewDL();
	m_bUseActiveCat = thePrefs.UseActiveCatForLinks();
	m_bAutoSetResOrder = thePrefs.AutoSetResumeOrder();
	m_bShowA4AFDebugOutput = thePrefs.m_bShowA4AFDebugOutput;
	m_bSmartA4AFSwapping = thePrefs.UseSmartA4AFSwapping();
	m_iAdvA4AFMode = thePrefs.AdvancedA4AFMode();
	m_bSmallFileDLPush = thePrefs.SmallFileDLPush();
	m_iResumeFileInNewCat = thePrefs.StartDLInEmptyCats();
	m_bUseAutoCat = thePrefs.UseAutoCat();
	m_bAddRemovedInc = thePrefs.UseAddRemoveInc();
	m_bUseSLS = thePrefs.UseSaveLoadSources();
	// khaos::accuratetimerem+
	m_iTimeRemainingMode = thePrefs.GetTimeRemainingMode();
	// khaos::accuratetimerem-
	//MORPH END - Added by SiRoB, khaos::categorymod+
	//MORPH START - Added by SiRoB, ICS Optional
	m_bUseICS = thePrefs.UseICS();
	//MORPH END   - Added by SiRoB, ICS Optional
	
	// Mighty Knife: Report hashing files, Log friendlist activities
	m_bReportHashingFiles = thePrefs.GetReportHashingFiles ();
	m_bLogFriendlistActivities = thePrefs.GetLogFriendlistActivities ();
	// [end] Mighty Knife

	// Mighty Knife: Static server handling
	m_bDontRemoveStaticServers = thePrefs.GetDontRemoveStaticServers ();
	// [end] Mighty Knife
	
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
    // Added by MoNKi [ SlugFiller: -lowIdRetry- ]
	m_iLowIdRetry = thePrefs.GetLowIdRetries();
	// End SlugFiller

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
	m_bIP2CountryAutoUpdate = thePrefs.IsAutoUPdateIP2CountryEnabled();

	m_bEnableEqualChanceForEachFile = thePrefs.IsEqualChanceEnable();//Morph - added by AndCycle, Equal Chance For Each File

	m_bStaticIcon = thePrefs.GetStaticIcon(); //MORPH - Added, Static Tray Icon
	
	CPropertyPage::OnInitDialog();
	Localize();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPgMorph::OnKillActive()
{
	// if prop page is closed by pressing VK_ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	return CPropertyPage::OnKillActive();
}

BOOL CPPgMorph::OnApply()
{
	bool bRestartApp = false;
	// if prop page is closed by pressing VK_ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	
	if (!UpdateData())
		return FALSE;
	if (m_iDynUpMode == 3)
		thePrefs.isautodynupswitching = true;//MORPH - Added by Yun.SF3, Auto DynUp changing
	else{
		thePrefs.isautodynupswitching = false;
		thePrefs.m_bSUCEnabled = (m_iDynUpMode == 1);
		thePrefs.m_bDynUpEnabled = (m_iDynUpMode == 2);
	}
	thePrefs.maxconnectionsswitchborder = m_iMaxConnectionsSwitchBorder;//MORPH - Added by Yun.SF3, Auto DynUp changing
	
	thePrefs.m_bSUCLog = m_bSUCLog;
	thePrefs.m_iSUCHigh = m_iSUCHigh;
	thePrefs.m_iSUCLow = m_iSUCLow;
	thePrefs.m_iSUCPitch = m_iSUCPitch;
	thePrefs.m_iSUCDrift = m_iSUCDrift;
	thePrefs.m_bDynUpLog = m_bUSSLog;
	thePrefs.m_bUSSUDP = m_bUSSUDP; //MORPH - Added by SiRoB, USS UDP preferency
	if ( m_sPingDataSize < 0 ) m_sPingDataSize=0;   // MORPH leuk_he ICMP ping datasize <> 0 setting
	if ( m_sPingDataSize  > 65) m_sPingDataSize=64; //MORPH leuk_he ICMP ping datasize <> 0 setting
	thePrefs.m_sPingDataSize =	(short) m_sPingDataSize; //MORPH leuk_he ICMP ping datasize <> 0 setting
	thePrefs.m_bDynUpUseMillisecondPingTolerance = m_bUSSLimit; // EastShare - Added by TAHO, USS limit
	thePrefs.m_iDynUpPingToleranceMilliseconds = m_iUSSPingLimit; // EastShare - Added by TAHO, USS limit
    thePrefs.m_iDynUpPingTolerance = m_iUSSPingTolerance;
    thePrefs.m_iDynUpGoingUpDivider = m_iUSSGoingUpDivider;
    thePrefs.m_iDynUpGoingDownDivider = m_iUSSGoingDownDivider;
    thePrefs.m_iDynUpNumberOfPings = m_iUSSNumberOfPings;
	// ==> [MoNKi: -USS initial TTL-] - Stulle
	thePrefs.SetUSSInitialTTL((uint8)m_iUSSTTL);
	// <== [MoNKi: -USS initial TTL-] - Stulle
	if(thePrefs.GetMaxGraphUploadRate(false)==1) // work arround bug that would always cause a warning to pop up.
		m_iMinUpload = 1;
	thePrefs.SetMinUpload(m_iMinUpload);
	thePrefs.enableDownloadInRed = m_bEnableDownloadInRed; //MORPH - Added by IceCream, show download in red
	thePrefs.m_bShowActiveDownloadsBold = m_bEnableDownloadInBold; //MORPH - Added by SiRoB, show download in Bold
	thePrefs.m_bShowClientPercentage = m_bShowClientPercentage;
	// MORPH START leuk_he disable catcolor
	if ( thePrefs.m_bDisableCatColors   !=  m_bDisableCatColors) {
		thePrefs.m_bDisableCatColors   =  m_bDisableCatColors ;
		theApp.emuledlg->transferwnd->UpdateCatTabTitles();
	}
	//to call.UpdateCatTabTitles
	// MORPH START leuk_he disable catcolor

	//MORPH START - Added by SiRoB, Datarate Average Time Management
	bool updateLegend = false;
	updateLegend = thePrefs.m_iDownloadDataRateAverageTime/1000 != m_iDownloadDataRateAverageTime;
	thePrefs.m_iDownloadDataRateAverageTime = 1000*max(1, m_iDownloadDataRateAverageTime);
	updateLegend |= thePrefs.m_iUploadDataRateAverageTime/1000 != m_iUploadDataRateAverageTime;
	thePrefs.m_iUploadDataRateAverageTime = 1000*max(1, m_iUploadDataRateAverageTime);
	//MORPH END   - Added by SiRoB, Datarate Average Time Management
	//MORPH START - Added by Stulle, Global Source Limit
	if (thePrefs.GetGlobalHL() != (UINT)m_iGlobalHL ||
		thePrefs.IsUseGlobalHL() != m_bGlobalHL)
	{
		thePrefs.m_bGlobalHL = m_bGlobalHL;
		thePrefs.m_uGlobalHL = m_iGlobalHL;
		if(m_bGlobalHL && theApp.downloadqueue->GetPassiveMode())
		{
			theApp.downloadqueue->SetPassiveMode(false);
			theApp.downloadqueue->SetUpdateHlTime(50000); // 50 sec
			AddDebugLogLine(true,_T("{GSL} Global Source Limit settings have changed! Disabled PassiveMode!"));
		}
	}
	//MORPH END   - Added by Stulle, Global Source Limit
       
	thePrefs.m_bShowRuntimeOnTitle = m_bShowModstring; 

	thePrefs.infiniteQueue = m_bInfiniteQueue;	//Morph - added by AndCycle, SLUGFILLER: infiniteQueue
	thePrefs.m_bDontRemoveSpareTrickleSlot = m_bDontRemoveSpareTrickleSlot; //Morph - added by AndCycle, Dont Remove Spare Trickle Slot
	thePrefs.m_bUseDownloadOverhead= m_bUseDownloadOverhead; // MORPH leuk_he include download overhead in upload stats
	thePrefs.m_iCompressLevel = m_iCompressLevel; // morph settable compression
	thePrefs.m_bUseCompression = m_bUseCompression; // use compression
	thePrefs.m_bFunnyNick = m_bFunnyNick;//MORPH - Added by SiRoB, Optionnal funnynick display
	//MORPH START - Added by SiRoB, Upload Splitting Class
	updateLegend |= thePrefs.globaldataratefriend != m_iGlobalDataRateFriend;
	thePrefs.globaldataratefriend = m_iGlobalDataRateFriend;
	updateLegend |= thePrefs.maxglobaldataratefriend != m_iMaxGlobalDataRateFriend;
	thePrefs.maxglobaldataratefriend = m_iMaxGlobalDataRateFriend;
	updateLegend |= thePrefs.maxclientdataratefriend != m_iMaxClientDataRateFriend;
	thePrefs.maxclientdataratefriend = m_iMaxClientDataRateFriend;
	updateLegend |= thePrefs.globaldataratepowershare != m_iGlobalDataRatePowerShare;
	thePrefs.globaldataratepowershare = m_iGlobalDataRatePowerShare;
	updateLegend |= thePrefs.maxglobaldataratepowershare != m_iMaxGlobalDataRatePowerShare;
	thePrefs.maxglobaldataratepowershare = m_iMaxGlobalDataRatePowerShare;
	updateLegend |= thePrefs.maxclientdataratepowershare != m_iMaxClientDataRatePowerShare;
	thePrefs.maxclientdataratepowershare = m_iMaxClientDataRatePowerShare;
	updateLegend |= thePrefs.maxclientdatarate != m_iMaxClientDataRate;
	thePrefs.maxclientdatarate = m_iMaxClientDataRate;
	//MORPH END   - Added by SiRoB, Upload Splitting Class
	if (updateLegend)
		theApp.emuledlg->statisticswnd->RepaintMeters();
	
	// ==> Slot Limit - Stulle
	thePrefs.m_bSlotLimitThree = (m_iSlotLimiter == 1);
	thePrefs.m_bSlotLimitNum = (m_iSlotLimiter == 2);
	thePrefs.m_iSlotLimitNum = (uint8)m_iSlotLimitNum;
    // <== Slot Limit - Stulle

	//MORPH START - Added by SiRoB, khaos::categorymod+
	thePrefs.m_bShowCatNames = m_bShowCatNames;
	thePrefs.m_bSelCatOnAdd = m_bSelectCat;
	thePrefs.m_bActiveCatDefault = m_bUseActiveCat;
	thePrefs.m_bAutoSetResumeOrder = m_bAutoSetResOrder;
	thePrefs.m_bShowA4AFDebugOutput = m_bShowA4AFDebugOutput;
	thePrefs.m_bSmartA4AFSwapping = m_bSmartA4AFSwapping;
	thePrefs.m_iAdvancedA4AFMode = (uint8)m_iAdvA4AFMode;
	thePrefs.m_bSmallFileDLPush = m_bSmallFileDLPush;
	thePrefs.m_iStartDLInEmptyCats = (uint8)m_iResumeFileInNewCat;
	thePrefs.m_bUseAutoCat = m_bUseAutoCat;
	thePrefs.m_bAddRemovedInc = m_bAddRemovedInc;
	thePrefs.m_bUseSaveLoadSources = m_bUseSLS;
	// khaos::accuratetimerem+
	thePrefs.m_iTimeRemainingMode = (uint8)m_iTimeRemainingMode;
	// khaos::accuratetimerem-
	//MORPH END - Added by SiRoB, khaos::categorymod+
	//MORPH START - Added by SiRoB, ICS Optional
	thePrefs.m_bUseIntelligentChunkSelection = m_bUseICS;
	//MORPH END   - Added by SiRoB, ICS Optional

	// Mighty Knife: Report hashing files, Log friendlist activities
	thePrefs.SetReportHashingFiles (m_bReportHashingFiles);
	thePrefs.SetLogFriendlistActivities (m_bLogFriendlistActivities);
	// [end] Mighty Knife

	// Mighty Knife: Static server handling
	thePrefs.SetDontRemoveStaticServers (m_bDontRemoveStaticServers);
	// [end] Mighty Knife

	theApp.scheduler->SaveOriginals(); //Added by SiRoB, Fix for Param used in scheduler

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
    // Added by MoNKi [ SlugFiller: -lowIdRetry- ]
	if(m_iLowIdRetry<0)
		m_iLowIdRetry = 0;
	if(m_iLowIdRetry>255)
		m_iLowIdRetry = 255;
	thePrefs.LowIdRetries = m_iLowIdRetry;
	// End SlugFiller

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
	thePrefs.AutoUpdateIP2Country = m_bIP2CountryAutoUpdate;

	//Morph - added by AndCycle, Save Upload Queue Wait Time (MSUQWT)
	if(m_bSaveUploadQueueWaitTime != thePrefs.m_bSaveUploadQueueWaitTime)	bRestartApp = true;
	thePrefs.m_bSaveUploadQueueWaitTime = m_bSaveUploadQueueWaitTime;
	//Morph - added by AndCycle, Save Upload Queue Wait Time (MSUQWT)

	//Morph - added by AndCycle, Equal Chance For Each File
	thePrefs.m_bEnableEqualChanceForEachFile = m_bEnableEqualChanceForEachFile;
	//Morph - added by AndCycle, Equal Chance For Each File

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
	
	SetModified(FALSE);

	if (bRestartApp){
		AfxMessageBox(GetResString(IDS_SETTINGCHANGED_RESTART));
	}

	return CPropertyPage::OnApply();
}

void CPPgMorph::OnHScroll(UINT /*nSBCode*/, UINT /*nPos*/, CScrollBar* /*pScrollBar*/) 
{
	SetModified(TRUE);
	CString temp;
}

void CPPgMorph::Localize(void)
{	
	if(m_hWnd)
	{
		CString Buffer;
		//MORPH START - Added by Yun.SF3, Auto DynUp changing
		if (m_htiDynUpAutoSwitching) m_ctrlTreeOptions.SetItemText(m_htiDynUpAutoSwitching, GetResString(IDS_AUTODYNUPSWITCHING));
		if (m_htiMaxConnectionsSwitchBorder){
			Buffer.Format(GetResString(IDS_MAXCONNECTIONSSWITCHBORDER),100);
			m_ctrlTreeOptions.SetEditLabel(m_htiMaxConnectionsSwitchBorder, Buffer);
		}
		//MORPH END - Added by Yun.SF3, Auto DynUp changing
		if (m_htiDynUpOFF) m_ctrlTreeOptions.SetItemText(m_htiDynUpOFF, GetResString(IDS_DISABLED));
		if (m_htiDynUpSUC) m_ctrlTreeOptions.SetItemText(m_htiDynUpSUC, GetResString(IDS_SUC));
		if (m_htiDynUpUSS) m_ctrlTreeOptions.SetItemText(m_htiDynUpUSS, GetResString(IDS_USS));
		if (m_htiSUCLog) m_ctrlTreeOptions.SetItemText(m_htiSUCLog, GetResString(IDS_SUC_LOG));
		if (m_htiSUCHigh){
			Buffer.Format(GetResString(IDS_SUC_HIGH),900);
			m_ctrlTreeOptions.SetEditLabel(m_htiSUCHigh, Buffer);
		}
		if (m_htiSUCLow){
			Buffer.Format(GetResString(IDS_SUC_LOW),600);
			m_ctrlTreeOptions.SetEditLabel(m_htiSUCLow, Buffer);
		}
		if (m_htiSUCPitch){
			Buffer.Format(GetResString(IDS_SUC_PITCH),3000);
			m_ctrlTreeOptions.SetEditLabel(m_htiSUCPitch, Buffer);
		}
		if (m_htiSUCDrift){
			Buffer.Format(GetResString(IDS_SUC_DRIFT),50);
			m_ctrlTreeOptions.SetEditLabel(m_htiSUCDrift, Buffer);
		}
		if (m_htiUSSLog) m_ctrlTreeOptions.SetItemText(m_htiUSSLog, GetResString(IDS_USS_LOG));
		if (m_htiUSSUDP) m_ctrlTreeOptions.SetItemText(m_htiUSSUDP, GetResString(IDS_USS_UDP));//MORPH - Added by SiRoB, USS UDP preferency
		if (m_htiUSSLimit) m_ctrlTreeOptions.SetItemText(m_htiUSSLimit, GetResString(IDS_USS_USEMAXPING));
		if (m_htiUSSPingLimit){
			Buffer.Format(GetResString(IDS_USS_MAXPING),200);
			m_ctrlTreeOptions.SetEditLabel(m_htiUSSPingLimit, Buffer);
		}
		if (m_htiUSSPingTolerance){
			Buffer.Format(GetResString(IDS_USS_PINGTOLERANCE),800);
			m_ctrlTreeOptions.SetEditLabel(m_htiUSSPingTolerance, Buffer);
		}
		if (m_htiUSSGoingUpDivider){
			Buffer.Format(GetResString(IDS_USS_GOINGUPDIVIDER),1000);
			m_ctrlTreeOptions.SetEditLabel(m_htiUSSGoingUpDivider, Buffer);
		}
		if (m_htiUSSGoingDownDivider){
			Buffer.Format(GetResString(IDS_USS_GOINGDOWNDIVIDER),1000);
			m_ctrlTreeOptions.SetEditLabel(m_htiUSSGoingDownDivider, Buffer);
		}
		if (m_htiUSSNumberOfPings){
			Buffer.Format(GetResString(IDS_USS_NUMBEROFPINGS),1);
			m_ctrlTreeOptions.SetEditLabel(m_htiUSSNumberOfPings, Buffer);
		}
		if (m_htiMinUpload) {m_ctrlTreeOptions.SetEditLabel(m_htiMinUpload, GetResString(IDS_MINUPLOAD));
		}
		if (m_htiEnableDownloadInRed) m_ctrlTreeOptions.SetItemText(m_htiEnableDownloadInRed, GetResString(IDS_DOWNLOAD_IN_RED)); //MORPH - Added by IceCream, show download in red
		if (m_htiEnableDownloadInBold) m_ctrlTreeOptions.SetItemText(m_htiEnableDownloadInBold, GetResString(IDS_DOWNLOAD_IN_BOLD)); //MORPH - Added by SiRoB, show download in Bold
		if (m_htiShowClientPercentage) m_ctrlTreeOptions.SetItemText(m_htiShowClientPercentage, GetResString(IDS_CLIENTPERCENTAGE));		
		if (m_htiInfiniteQueue) m_ctrlTreeOptions.SetItemText(m_htiInfiniteQueue, GetResString(IDS_INFINITEQUEUE));	//Morph - added by AndCycle, SLUGFILLER: infiniteQueue
		if (m_htiDontRemoveSpareTrickleSlot)m_ctrlTreeOptions.SetItemText(m_htiDontRemoveSpareTrickleSlot, GetResString(IDS_DONTREMOVESPARETRICKLESLOT));//Morph - added by AndCycle, Dont Remove Spare Trickle Slot
		if (m_htiUseDownloadOverhead)m_ctrlTreeOptions.SetItemText(m_htiUseDownloadOverhead, GetResString(IDS_USEDOWNLOADOVERHEAD));//Morph 
		if (m_htiDisplayFunnyNick) m_ctrlTreeOptions.SetItemText(m_htiDisplayFunnyNick, GetResString(IDS_DISPLAYFUNNYNICK));//MORPH - Added by SiRoB, Optionnal funnynick display
		// ==> [MoNKi: -USS initial TTL-] - Stulle
		if (m_htiUSSTTL) m_ctrlTreeOptions.SetEditLabel(m_htiUSSTTL, GetResString(IDS_USS_INITIAL_TTL));
		// <== [MoNKi: -USS initial TTL-] - Stulle
		//MORPH START - Added by Stulle, Global Source Limit
		if (m_htiGlobalHL)m_ctrlTreeOptions.SetItemText(m_htiGlobalHL, GetResString(IDS_SUC_ENABLED));
		if (m_htiGlobalHlLimit)m_ctrlTreeOptions.SetEditLabel(m_htiGlobalHlLimit, GetResString(IDS_GLOBAL_HL_LIMIT));
		//MORPH END   - Added by Stulle, Global Source Limit
		//MORPH START - Added by SiRoB, Datarate Average Time Management
		if (m_htiDownloadDataRateAverageTime) m_ctrlTreeOptions.SetEditLabel(m_htiDownloadDataRateAverageTime, GetResString(IDS_DATARATEAVERAGETIME));
		if (m_htiUploadDataRateAverageTime) m_ctrlTreeOptions.SetEditLabel(m_htiUploadDataRateAverageTime, GetResString(IDS_DATARATEAVERAGETIME));
		//MORPH END   - Added by SiRoB, Datarate Average Time Management
		//MORPH START - Added by SiRoB, Upload Splitting Class
		if (m_htiGlobalDataRateFriend) m_ctrlTreeOptions.SetEditLabel(m_htiGlobalDataRateFriend, GetResString(IDS_MINDATARATEFRIEND));
		if (m_htiMaxClientDataRateFriend) m_ctrlTreeOptions.SetEditLabel(m_htiMaxClientDataRateFriend, GetResString(IDS_MAXCLIENTDATARATEFRIEND));
		if (m_htiGlobalDataRatePowerShare) m_ctrlTreeOptions.SetEditLabel(m_htiGlobalDataRatePowerShare, GetResString(IDS_MINDATARATEPOWERSHARE));
		if (m_htiMaxClientDataRatePowerShare) m_ctrlTreeOptions.SetEditLabel(m_htiMaxClientDataRatePowerShare, GetResString(IDS_MAXCLIENTDATARATEPOWERSHARE));
		if (m_htiMaxClientDataRate) m_ctrlTreeOptions.SetEditLabel(m_htiMaxClientDataRate, GetResString(IDS_MAXCLIENTDATARATE));
		if (m_htiMaxGlobalDataRatePowerShare)m_ctrlTreeOptions.SetEditLabel(m_htiMaxGlobalDataRatePowerShare, GetResString(IDS_MAXDATARATEPOWERSHARE));
		//MORPH END   - Added by SiRoB, Upload Splitting Class
		// ==> Slot Limit - Stulle
		if (m_htiSlotLimitGroup) m_ctrlTreeOptions.SetItemText(m_htiSlotLimitGroup, GetResString(IDS_SLOT_LIMIT_GROUP));
		if (m_htiSlotLimitNum) m_ctrlTreeOptions.SetEditLabel(m_htiSlotLimitNum, GetResString(IDS_SLOT_LIMIT_NUM));
		// <== Slot Limit - Stulle

		if (m_htiMisc) m_ctrlTreeOptions.SetItemText(m_htiMisc, GetResString(IDS_PW_MISC));

		//MORPH START - Added by SiRoB, khaos::categorymod+
		if (m_htiShowCatNames) m_ctrlTreeOptions.SetItemText(m_htiShowCatNames, GetResString(IDS_CAT_SHOWCATNAME));
		if (m_htiSelectCat) m_ctrlTreeOptions.SetItemText(m_htiSelectCat, GetResString(IDS_CAT_SHOWSELCATDLG));
		if (m_htiUseAutoCat) m_ctrlTreeOptions.SetItemText(m_htiUseAutoCat, GetResString(IDS_CAT_USEAUTOCAT));
        if (m_htiUseActiveCat) m_ctrlTreeOptions.SetItemText(m_htiUseActiveCat, GetResString(IDS_CAT_USEACTIVE));
		if (m_htiAutoSetResOrder) m_ctrlTreeOptions.SetItemText(m_htiAutoSetResOrder, GetResString(IDS_CAT_AUTORESUMEORD));
		if (m_htiSmallFileDLPush) m_ctrlTreeOptions.SetItemText(m_htiSmallFileDLPush, GetResString(IDS_CAT_SMALLFILEDLPUSH));
		if (m_htiResumeFileInNewCat) m_ctrlTreeOptions.SetEditLabel(m_htiResumeFileInNewCat, GetResString(IDS_CAT_STARTFILESONADD));
		if (m_bAddRemovedInc) m_ctrlTreeOptions.SetItemText(m_htiAddRemovedInc, GetResString(IDS_ADD_REMOVED_INC));
		if (m_htiSmartA4AFSwapping) m_ctrlTreeOptions.SetItemText(m_htiSmartA4AFSwapping, GetResString(IDS_A4AF_SMARTSWAP));
		if (m_htiShowA4AFDebugOutput) m_ctrlTreeOptions.SetItemText(m_htiShowA4AFDebugOutput, GetResString(IDS_A4AF_SHOWDEBUG));
		if (m_htiAdvA4AFMode)m_ctrlTreeOptions.SetItemText(m_htiAdvA4AFMode, /*GetResString(IDS_DEFAULT) + " " +*/ GetResString(IDS_A4AF_ADVMODE));
		if (m_htiDisableAdvA4AF) m_ctrlTreeOptions.SetItemText(m_htiDisableAdvA4AF, GetResString(IDS_A4AF_DISABLED));
		if (m_htiBalanceSources) m_ctrlTreeOptions.SetItemText(m_htiBalanceSources, GetResString(IDS_A4AF_BALANCE));
		if (m_htiStackSources) m_ctrlTreeOptions.SetItemText(m_htiStackSources, GetResString(IDS_A4AF_STACK));
		if (m_htiUseSLS) m_ctrlTreeOptions.SetItemText(m_htiUseSLS, GetResString(IDS_SLS_USESLS));
		// khaos::accuratetimerem+
		if (m_htiTimeRemainingMode)m_ctrlTreeOptions.SetItemText(m_htiTimeRemainingMode, GetResString(IDS_REMTIMEAVRREAL));
		if (m_htiTimeRemBoth) m_ctrlTreeOptions.SetItemText(m_htiTimeRemBoth, GetResString(IDS_BOTH));
		if (m_htiTimeRemRealTime) m_ctrlTreeOptions.SetItemText(m_htiTimeRemRealTime, GetResString(IDS_REALTIME));
		if (m_htiTimeRemAverage) m_ctrlTreeOptions.SetItemText(m_htiTimeRemAverage, GetResString(IDS_AVG));
		// khaos::accuratetimerem-
		//MORPH END - Added by SiRoB, khaos::categorymod+
		//MORPH START - Added by SiRoB, ICS Optional
		if (m_htiUseICS) m_ctrlTreeOptions.SetItemText(m_htiUseICS, GetResString(IDS_ICS_USEICS));
		//MORPH START - Added by SiRoB, ICS Optional
		if (m_htiHideOS) m_ctrlTreeOptions.SetEditLabel(m_htiHideOS, GetResString(IDS_HIDEOVERSHARES));//MORPH - Added by SiRoB, SLUGFILLER: hideOS
		if (m_htiSelectiveShare) m_ctrlTreeOptions.SetItemText(m_htiSelectiveShare, GetResString(IDS_SELECTIVESHARE));//MORPH - Added by SiRoB, SLUGFILLER: hideOS
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
        // Added by MoNKi [ SlugFiller: -lowIdRetry- ]
		if(m_htiLowIdRetry)	m_ctrlTreeOptions.SetEditLabel(m_htiLowIdRetry, GetResString(IDS_RECONNECTONLOWID));
		// End SlugFiller
		if (m_htiEnablePreferShareAll) m_ctrlTreeOptions.SetItemText(m_htiEnablePreferShareAll, GetResString(IDS_PREFER_SHARE_ALL));//EastShare - PreferShareAll by AndCycle
		if (m_htiIsPayBackFirst) m_ctrlTreeOptions.SetItemText(m_htiIsPayBackFirst, GetResString(IDS_PAYBACKFIRST));//EastShare - added by AndCycle, Pay Back First
		if (m_htiPayBackFirstLimit) m_ctrlTreeOptions.SetEditLabel(m_htiPayBackFirstLimit, GetResString(IDS_PAYBACKFIRSTLIMIT));//MORPH - Added by SiRoB, Pay Back First Tweak
		if (m_htiOnlyDownloadCompleteFiles) m_ctrlTreeOptions.SetItemText(m_htiOnlyDownloadCompleteFiles,GetResString(IDS_ONLY_DOWNLOAD_COMPLETE_FILES));//EastShare - Added by AndCycle, Only download complete files v2.1 (shadow)
		if (m_htiSaveUploadQueueWaitTime) m_ctrlTreeOptions.SetItemText(m_htiSaveUploadQueueWaitTime,GetResString(IDS_SAVE_UPLOAD_QUEUE_WAIT_TIME));//Morph - added by AndCycle, Save Upload Queue Wait Time (MSUQWT)
		if (m_htiEnableChunkDots) m_ctrlTreeOptions.SetItemText(m_htiEnableChunkDots, GetResString(IDS_ENABLE_CHUNKDOTS));//EastShare - Added by Pretender, Option for ChunkDots
		//MORPH END   - Added by SiRoB, Show Permission
		if (m_htiStaticIcon) m_ctrlTreeOptions.SetItemText(m_htiStaticIcon, GetResString(IDS_STATIC_ICON)); //MORPH - Added, Static Tray Icon
	}
}

void CPPgMorph::OnDestroy()
{
	m_ctrlTreeOptions.DeleteAllItems();
	m_ctrlTreeOptions.DestroyWindow();
	m_bInitializedTreeOpts = false;
	m_htiDM = NULL;
	m_htiUM = NULL;
	m_htiDYNUP = NULL;
	m_htiDynUpOFF = NULL;
	m_htiDynUpSUC = NULL;
	m_htiDynUpUSS = NULL;
	m_htiDynUpAutoSwitching = NULL;//MORPH - Added by Yun.SF3, Auto DynUp changing
	m_htiMaxConnectionsSwitchBorder = NULL;//MORPH - Added by Yun.SF3, Auto DynUp changing
	
	m_htiSUCLog = NULL;
	m_htiSUCHigh = NULL;
	m_htiSUCLow = NULL;
	m_htiSUCPitch = NULL;
	m_htiSUCDrift = NULL;
	m_htiUSSLog = NULL;
	m_htiUSSUDP = NULL;//MORPH - Added by SiRoB, USS UDP preferency
	m_htiUSSLimit = NULL; // EastShare - Added by TAHO, USS limit
	m_htiUSSPingLimit = NULL;
    m_htiUSSPingTolerance = NULL;
    m_htiUSSGoingUpDivider = NULL;
    m_htiUSSGoingDownDivider = NULL;
    m_htiUSSNumberOfPings = NULL;
	// ==> [MoNKi: -USS initial TTL-] - Stulle
	m_htiUSSTTL = NULL;
	// <== [MoNKi: -USS initial TTL-] - Stulle
	m_htiMinUpload = NULL;
	m_htiDisp = NULL;
	m_htiEnableDownloadInRed = NULL; //MORPH - Added by IceCream, show download in red
	m_htiEnableDownloadInBold = NULL; //MORPH - Added by SiRoB, show download in Bold
	m_htiShowClientPercentage = NULL;
	//MORPH START - Added by Stulle, Global Source Limit
	m_htiGlobalHL = NULL;
	m_htiGlobalHlLimit = NULL;
	//MORPH END   - Added by Stulle, Global Source Limit
       
	m_htiShowModstring = NULL; 

	m_htiDisableCatColors = NULL;
	//MORPH START - Added by SiRoB, Datarate Average Time Management
	m_htiDownloadDataRateAverageTime = NULL;
	m_htiUploadDataRateAverageTime = NULL;
	//MORPH END   - Added by SiRoB, Datarate Average Time Management
	//MORPH START - Added by SiRoB, Upload Splitting Class
	m_htiFriend = NULL;
	m_htiGlobalDataRateFriend = NULL;
	m_htiMaxClientDataRateFriend = NULL;
	m_htiPowerShare = NULL;
	m_htiGlobalDataRatePowerShare = NULL;
	m_htiMaxClientDataRatePowerShare = NULL;
	m_htiMaxClientDataRate = NULL;
	//MORPH END   - Added by SiRoB, Upload Splitting Class
	// ==> Slot Limit - Stulle
	m_htiSlotLimitGroup = NULL;
	m_htiSlotLimitNone = NULL;
	m_htiSlotLimitThree = NULL;
	m_htiSlotLimitNumB = NULL;
	m_htiSlotLimitNum = NULL;
	// <== Slot Limit - Stulle
	m_htiSCC = NULL;
	//MORPH START - Added by SiRoB, khaos::categorymod+
	m_htiShowCatNames = NULL;
	m_htiSelectCat = NULL;
	m_htiUseActiveCat = NULL;
	m_htiAutoSetResOrder = NULL;
	m_htiSmartA4AFSwapping = NULL;
	m_htiAdvA4AFMode = NULL;
	m_htiBalanceSources = NULL;
	m_htiStackSources = NULL;
	m_htiShowA4AFDebugOutput = NULL;
	m_htiDisableAdvA4AF = NULL;
	m_htiSmallFileDLPush = NULL;
	m_htiResumeFileInNewCat = NULL;
	m_htiUseAutoCat = NULL;
	m_htiAddRemovedInc = NULL;
	m_htiUseSLS = NULL;
	// khaos::accuratetimerem+
	m_htiTimeRemainingMode = NULL;
	m_htiTimeRemBoth = NULL;
	m_htiTimeRemAverage = NULL;
	m_htiTimeRemRealTime = NULL;
	// khaos::accuratetimerem-
	//MORPH END - Added by SiRoB, khaos::categorymod+
	m_htiUseICS = NULL;//MORPH - Added by SiRoB, ICS Optional
	
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
    m_htiEnablePreferShareAll = NULL; //EastShare - PreferShareAll by AndCycle
	m_htiIsPayBackFirst = NULL; //EastShare - added by AndCycle, Pay Back First
	m_htiPayBackFirstLimit = NULL; //MORPH - Added by SiRoB, Pay Back First Tweak
	m_htiOnlyDownloadCompleteFiles = NULL;//EastShare - Added by AndCycle, Only download complete files v2.1 (shadow)
	m_htiSaveUploadQueueWaitTime = NULL;//Morph - added by AndCycle, Save Upload Queue Wait Time (MSUQWT)
	m_htiEnableChunkDots = NULL; //EastShare - Added by Pretender, Option for ChunkDots

	// Added by MoNKi [ SlugFiller: -lowIdRetry- ]
	m_htiLowIdRetry = NULL;
	// End SlugFiller

	//EastShare Start - added by AndCycle, IP to Country
	m_htiIP2CountryName = NULL;
	m_htiIP2CountryName_DISABLE = NULL;
	m_htiIP2CountryName_SHORT = NULL;
	m_htiIP2CountryName_MID = NULL;
	m_htiIP2CountryName_LONG = NULL;
	m_htiIP2CountryShowFlag = NULL;
	//EastShare End - added by AndCycle, IP to Country
	m_htiIP2CountryAutoUpdate = NULL;

	//EastShare START - Added by Pretender
	m_htiCreditSystem = NULL;
	//EastShare END - Added by Pretender

	//Morph - added by AndCycle, Equal Chance For Each File
	m_htiEnableEqualChanceForEachFile = NULL;
	//Morph - added by AndCycle, Equal Chance For Each File

	m_htiStaticIcon = NULL; //MORPH - Added, Static Tray Icon

	CPropertyPage::OnDestroy();
}

LRESULT CPPgMorph::OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM /*lParam*/)
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
