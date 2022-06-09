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
    m_htiShowFileStatusIcons = NULL;
    m_htiDropBlocking = NULL;
    m_htiShowBlocking = NULL;
	m_htiShowCpuRam = NULL;
	m_htiShowMinQR = NULL;
	m_htiDropSystem = NULL;

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
	m_htiBrokenURLs = NULL; //MORPH - Added by WiZaRd, Fix broken HTTP downloads
	//MORPH END   - Added by SiRoB, ICS Optional
	m_htiInfiniteQueue = NULL;	//Morph - added by AndCycle, SLUGFILLER: infiniteQueue
#ifdef SPARE_TRICKLE
	m_htiDontRemoveSpareTrickleSlot = NULL; //Morph - added by AndCycle, Dont Remove Spare Trickle Slot
#endif

#ifdef DOWN_OVERHEAD
	m_htiUseDownloadOverhead = NULL ;//
#endif
	m_htiCompressLevel =NULL ;// morph settable compresslevel
	m_htiUseCompression=NULL ;// morph Use compress 
	m_htiUseAutoCompression=NULL ;

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
	/*
	//MORPH START - Added by SiRoB, Datarate Average Time Management
	m_htiDownloadDataRateAverageTime = NULL;
	m_htiUploadDataRateAverageTime = NULL;
	//MORPH END   - Added by SiRoB, Datarate Average Time Management
	*/
	
    m_htiSpreadbar = NULL; //MORPH	- Added by AndCycle, SLUGFILLER: Spreadbars - per file
	m_htiHideOS = NULL;	//MORPH - Added by SiRoB, SLUGFILLER: hideOS
	m_htiSelectiveShare = NULL;	//MORPH - Added by SiRoB, SLUGFILLER: hideOS
	m_htiShareOnlyTheNeed = NULL; //MORPH - Added by SiRoB, SHARE_ONLY_THE_NEED
	m_htiPowerShareLimit = NULL; //MORPH - Added by SiRoB, POWERSHARE Limit
	m_htiPsAmountLimit = NULL; // Limit PS by amount of data uploaded - Stulle
	//MORPH START - Added by SiRoB, Avoid misusing of powersharing	
	m_htiPowershareMode = NULL;
	m_htiPowershareDisabled = NULL;
	m_htiPowershareActivated = NULL;
	m_htiPowershareAuto = NULL;
	m_htiPowershareLimited = NULL;
	//MORPH END   - Added by SiRoB, Avoid misusing of powersharing	
	m_htiPowershareInternalPrio = NULL; //Morph - added by AndCyle, selective PS internal Prio

    // Added by MoNKi [ SlugFiller: -lowIdRetry- ]
	m_htiLowIdRetry = NULL;
	m_iLowIdRetry = 0;
	// End SlugFiller
	m_htiMMOpen = NULL;

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
// Tux: Feature: Automatic shared files updater [start]
	m_htiDirWatcher = NULL;
	m_bDirWatcher = false;
	// Tux: Feature: Automatic shared files updater [end]
	m_htiMisc = NULL;

	//EastShare START - Added by Pretender
	m_htiCreditSystem = NULL;
	//EastShare END - Added by Pretender

	//Morph - added by AndCycle, Equal Chance For Each File
	m_htiEnableEqualChanceForEachFile = NULL;
	//Morph - added by AndCycle, Equal Chance For Each File
	
	// NEO: QS - [QuickStart]
	m_htiQuickStart = NULL;
	m_htiQuickStartEnable = NULL;
	m_htiQuickStartTime = NULL;
	m_htiQuickStartTimePerFile = NULL;
	m_htiQuickMaxConperFive = NULL;
	m_htiQuickMaxHalfOpen = NULL;
	m_htiQuickMaxConnections = NULL;
	// NEO: QS END
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
		//MORPH END   - Added by SiRoB, Upload Splitting Class

        int iImgSFM = 8;
		int iImgQuickStart = 8; // NEO: QS - [QuickStart]
		CImageList* piml = m_ctrlTreeOptions.GetImageList(TVSIL_NORMAL);
		if (piml){
            iImgSFM = piml->Add(CTempIconLoader(_T("SHAREDFILES")));
			iImgIP2Country = piml->Add(CTempIconLoader(_T("COUNTRY"))); //EastShare - added by AndCycle, IP to Country
			iImgCS = piml->Add(CTempIconLoader(_T("STATSCLIENTS"))); // EastShare START - Added by Pretender, CS icon
            iImgMisc = piml->Add(CTempIconLoader(_T("PREFERENCES")));
			iImgUM = piml->Add(CTempIconLoader(_T("UPLOAD")));
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
			//MORPH END   - Added by SiRoB, Upload Splitting Class

			iImgQuickStart = piml->Add(CTempIconLoader(_T("QUICKSTART"))); // NEO: QS - [QuickStart]
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
        
		//MORPH START - Added by Stulle, Global Source Limit
		m_htiGlobalHlGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_GLOBAL_HL), iImgGlobal, m_htiDM);
		m_htiGlobalHL = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SUC_ENABLED), m_htiGlobalHlGroup, m_bGlobalHL);
		m_htiGlobalHlLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_GLOBAL_HL_LIMIT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiGlobalHlGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiGlobalHlLimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		//MORPH END   - Added by Stulle, Global Source Limit

		m_htiDropSystem = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_AUTO_DROP), m_htiDM, m_bDropSystem); 

		//MORPH START - Added by SiRoB, khaos::categorymod+
		m_htiUseSLS = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SLS_USESLS), m_htiDM, m_bUseSLS);
		//MORPH END - Added by SiRoB, khaos::categorymod+
		//MORPH START - Added by SiRoB, ICS Optional
		m_htiUseICS = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ICS_USEICS), m_htiDM, m_bUseICS);
		//MORPH END   - Added by SiRoB, ICS Optional
		m_htiOnlyDownloadCompleteFiles = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ONLY_DOWNLOAD_COMPLETE_FILES), m_htiDM, m_bOnlyDownloadCompleteFiles);//EastShare - Added by AndCycle, Only download complete files v2.1 (shadow)
        //MORPH START - Added by WiZaRd, Fix broken HTTP downloads
		m_htiBrokenURLs = m_ctrlTreeOptions.InsertItem(GetResString(IDS_BROKEN_URLS), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,m_htiDM);
		m_ctrlTreeOptions.AddEditBox(m_htiBrokenURLs, RUNTIME_CLASS(CTreeOptionsEditEx));
		//MORPH END   - Added by WiZaRd, Fix broken HTTP downloads

		m_htiUM = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_UM), iImgUM, TVI_ROOT);

		//MORPH START - Added by SiRoB, Pay Back First Tweak
		m_htiCreditSystem = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_PAYBACKFIRST), iImgCS, m_htiUM);	
		m_htiIsPayBackFirst = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PAYBACKFIRST), m_htiCreditSystem, m_bIsPayBackFirst);//EastShare - added by AndCycle, Pay Back First
		m_htiPayBackFirstLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_PAYBACKFIRSTLIMIT),TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,m_htiIsPayBackFirst);
		m_ctrlTreeOptions.AddEditBox(m_htiPayBackFirstLimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_ctrlTreeOptions.Expand(m_htiIsPayBackFirst, TVE_EXPAND);
		//MORPH END   - Added by SiRoB, Pay Back First Tweak
				
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
		//MORPH END   - Added by SiRoB, Upload Splitting Class
		
		//MORPH START - Added by SiRoB, Upload Splitting Class
        m_htiMaxClientDataRate = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MAXCLIENTDATARATE), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiUM);
		m_ctrlTreeOptions.AddEditBox(m_htiMaxClientDataRate, RUNTIME_CLASS(CNumTreeOptionsEdit));
		//MORPH END   - Added by SiRoB, Upload Splitting Class

#ifdef SPARE_TRICKLE
		m_htiDontRemoveSpareTrickleSlot = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DONTREMOVESPARETRICKLESLOT), m_htiUM, m_bDontRemoveSpareTrickleSlot); //Morph - added by AndCycle, Dont Remove Spare Trickle Slot
#endif

#ifdef DOWN_OVERHEAD
		m_htiUseDownloadOverhead = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_USEDOWNLOADOVERHEAD), m_htiUM, m_bUseDownloadOverhead); //Morph - leuk_he use download overhead in upload
#endif
		m_htiUseCompression= m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_USECOMPRESS), m_htiUM, m_bUseCompression); 
	    m_htiCompressLevel = m_ctrlTreeOptions.InsertItem(GetResString(IDS_COMPRESSLEVEL), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiUM);
		m_ctrlTreeOptions.AddEditBox(m_htiCompressLevel, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiUseAutoCompression= m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_USEAUTOCOMPRESS), m_htiUM, m_bAutoCompress); 

		m_htiDropBlocking= m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DROP_BLOCKING), m_htiUM, m_bDropBlocking); 
		m_htiInfiniteQueue = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_INFINITEQUEUE), m_htiUM, m_bInfiniteQueue);	//Morph - added by AndCycle, SLUGFILLER: infiniteQueue

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
		m_htiPowershareMode = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_POWERSHARE), iImgPowerShare, m_htiSFM);
		m_htiPowershareDisabled = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_POWERSHARE_DISABLED), m_htiPowershareMode, m_iPowershareMode == 0);
		m_htiPowershareActivated =  m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_POWERSHARE_ACTIVATED), m_htiPowershareMode, m_iPowershareMode == 1);
		m_htiPowershareAuto =  m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_POWERSHARE_AUTO), m_htiPowershareMode, m_iPowershareMode == 2);
		//MORPH END   - Added by SiRoB, Avoid misusing of powersharing
		//MORPH START - Added by SiRoB, POWERSHARE Limit
		m_htiPowershareLimited =  m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_POWERSHARE_LIMITED), m_htiPowershareMode, m_iPowershareMode == 3);
		m_htiPowerShareLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_POWERSHARE_LIMIT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiPowershareLimited );
		m_ctrlTreeOptions.AddEditBox(m_htiPowerShareLimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		//MORPH END   - Added by SiRoB, POWERSHARE Limit
		// ==> Limit PS by amount of data uploaded - Stulle
		m_htiPsAmountLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_PS_AMOUNT_LIMIT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiPowershareLimited );
		m_ctrlTreeOptions.AddEditBox(m_htiPsAmountLimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// <== Limit PS by amount of data uploaded - Stulle
		//Morph Start - added by AndCyle, selective PS internal Prio
		m_htiPowershareInternalPrio = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_POWERSHARE_INTERPRIO), m_htiPowershareMode, m_bPowershareInternalPrio);
		//Morph End - added by AndCyle, selective PS internal Prio
#ifdef COMMUNITY	
		m_htiCommunityName = m_ctrlTreeOptions.InsertItem(GetResString(IDS_COMMUNITYTAG), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiSFM);

		m_ctrlTreeOptions.AddEditBox(m_htiCommunityName, RUNTIME_CLASS(CTreeOptionsEditEx));
#endif		
		m_ctrlTreeOptions.Expand(m_htiPowershareLimited, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiSpreadbar, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiHideOS, TVE_EXPAND);

		m_ctrlTreeOptions.Expand(m_htiPowershareMode, TVE_EXPAND);

        //EastShare Start - added by AndCycle, IP to Country
		m_htiIP2CountryName = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_IP2COUNTRY), iImgIP2Country, TVI_ROOT);
		m_htiIP2CountryName_DISABLE = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_DISABLED), m_htiIP2CountryName, m_iIP2CountryName == IP2CountryName_DISABLE);
		m_htiIP2CountryName_SHORT = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_COUNTRYNAME_SHORT), m_htiIP2CountryName, m_iIP2CountryName == IP2CountryName_SHORT);
		m_htiIP2CountryName_MID = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_COUNTRYNAME_MID), m_htiIP2CountryName, m_iIP2CountryName == IP2CountryName_MID);
		m_htiIP2CountryName_LONG = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_COUNTRYNAME_LONG), m_htiIP2CountryName, m_iIP2CountryName == IP2CountryName_LONG);
		m_htiIP2CountryShowFlag = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_COUNTRYNAME_SHOWFLAG), m_htiIP2CountryName, m_bIP2CountryShowFlag);
		//EastShare End - added by AndCycle, IP to Country

       // NEO: QS - [QuickStart]
		m_htiQuickStart = m_ctrlTreeOptions.InsertGroup(_T("")/*GetResString(IDS_X_QUICK_START)*/,iImgQuickStart, TVI_ROOT);
		m_htiQuickStartEnable = m_ctrlTreeOptions.InsertCheckBox(_T("")/*GetResString(IDS_X_QUICK_START_ENABLE)*/,m_htiQuickStart,m_uQuickStart, 0);
		m_htiQuickStartTime = m_ctrlTreeOptions.InsertItem(_T("")/*GetResString(IDS_X_QUICK_START_TIME)*/, TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiQuickStart);
		m_ctrlTreeOptions.AddEditBox(m_htiQuickStartTime,RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiQuickStartTimePerFile = m_ctrlTreeOptions.InsertItem(_T("")/*GetResString(IDS_X_QUICK_START_TIME_PER_FILE)*/, TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiQuickStart);
		m_ctrlTreeOptions.AddEditBox(m_htiQuickStartTimePerFile,RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiQuickMaxConperFive = m_ctrlTreeOptions.InsertItem(_T("")/*GetResString(IDS_X_QUICK_START_MAXPER5)*/, TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiQuickStart);
		m_ctrlTreeOptions.AddEditBox(m_htiQuickMaxConperFive,RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiQuickMaxHalfOpen = m_ctrlTreeOptions.InsertItem(_T("")/*GetResString(IDS_X_QUICK_START_MAXHALF)*/, TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiQuickStart);
		m_ctrlTreeOptions.AddEditBox(m_htiQuickMaxHalfOpen,RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiQuickMaxConnections = m_ctrlTreeOptions.InsertItem(_T("")/*GetResString(IDS_X_QUICK_START_MAXCON)*/, TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiQuickStart);
		m_ctrlTreeOptions.AddEditBox(m_htiQuickMaxConnections,RUNTIME_CLASS(CNumTreeOptionsEdit));
		// NEO: QS END

		//display +
        m_htiDisp = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_PW_DISPLAY), iImgDisp, TVI_ROOT);
	
		m_htiEnableDownloadInRed = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DOWNLOAD_IN_YELLOW), m_htiDisp, m_bEnableDownloadInRed); //MORPH - Added by SiRoB, show download in Bold
		m_htiEnableDownloadInBold = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DOWNLOAD_IN_BOLD), m_htiDisp, m_bEnableDownloadInBold); //MORPH - Added by SiRoB, show download in Bold
		m_htiShowClientPercentage = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_CLIENTPERCENTAGE), m_htiDisp, m_bShowClientPercentage);
		m_htiDisplayFunnyNick = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DISPLAYFUNNYNICK), m_htiDisp, m_bFunnyNick);//MORPH - Added by SiRoB, Optionnal funnynick display
 		m_htiEnableChunkDots = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ENABLE_CHUNKDOTS),m_htiDisp, m_bEnableChunkDots);//EastShare - Added by Pretender, Option for ChunkDots
		// MORPH START leuk_he disable catcolor
        m_htiDisableCatColors= m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DISABLECATCOLORS), m_htiDisp, m_bDisableCatColors);
	    // MORPH END   leuk_he disable catcolor
		m_htiShowMinQR = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SHOW_MINQR), m_htiDisp, m_bShowMinQR); 
		m_htiShowBlocking= m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SHOW_BLOCKRATIO), m_htiDisp, m_bShowBlocking); 
        m_htiShowModstring = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SHOW_RUNTIME), m_htiDisp, m_bShowModstring); 
		m_htiShowFileStatusIcons = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SHOW_STATUSICON), m_htiDisp, m_bShowFileStatusIcons); 
		m_htiShowCpuRam = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SHOW_CPU_USAGE), m_htiDisp, m_bShowCpuRam); 
		m_htiShowSessionDown = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SHOW_SESSION_DOWNLOAD), m_htiDisp, m_bShowSessionDown); 
		/*
		//set only in prefernces.ini
		//MORPH START - Added by SiRoB, Datarate Average Time Management /download
		m_htiDownloadDataRateAverageTime = m_ctrlTreeOptions.InsertItem(GetResString(IDS_DOWNLOAD) + _T(" ") + GetResString(IDS_DATARATEAVERAGETIME), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDisp);
		m_ctrlTreeOptions.AddEditBox(m_htiDownloadDataRateAverageTime, RUNTIME_CLASS(CNumTreeOptionsEdit));
		//MORPH END   - Added by SiRoB, Datarate Average Time Management
		//MORPH START - Added by SiRoB, Datarate Average Time Management /upload
		m_htiUploadDataRateAverageTime = m_ctrlTreeOptions.InsertItem(GetResString(IDS_PW_CON_UPLBL) + _T(" ") + GetResString(IDS_DATARATEAVERAGETIME), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDisp);
		m_ctrlTreeOptions.AddEditBox(m_htiUploadDataRateAverageTime, RUNTIME_CLASS(CNumTreeOptionsEdit));
		//MORPH END   - Added by SiRoB, Datarate Average Time Management
*/
		//display -

        m_htiMisc = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_PW_MISC), iImgMisc, TVI_ROOT);
		m_htiMMOpen = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MM_OPEN), m_htiMisc, m_bMMOpen);
		m_htiDirWatcher = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DIR_WATCHER), m_htiMisc, m_bDirWatcher);	// Tux: Feature: Automatic shared files updater

		// Mighty Knife: Report hashing files, Log friendlist activities
	    m_htiReportHashingFiles = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MORPH_RFHA), m_htiMisc, m_bReportHashingFiles);
	    m_htiLogFriendlistActivities = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MORPH_RAIF), m_htiMisc, m_bLogFriendlistActivities);
		// [end] Mighty Knife

		// Mighty Knife: Static server handling
	    m_htiDontRemoveStaticServers = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MORPH_KSSERV), m_htiMisc, m_bDontRemoveStaticServers);
		// [end] Mighty Knife

		m_htiEnablePreferShareAll = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PREFER_SHARE_ALL), m_htiMisc, m_bEnablePreferShareAll);//EastShare - PreferShareAll by AndCycle
		m_htiSaveUploadQueueWaitTime = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SAVE_UPLOAD_QUEUE_WAIT_TIME), m_htiMisc, m_bSaveUploadQueueWaitTime);//Morph - added by AndCycle, Save Upload Queue Wait Time (MSUQWT)

        //Morph - added by AndCycle, Equal Chance For Each File
		m_htiEnableEqualChanceForEachFile = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ECFEF), m_htiMisc, m_bEnableEqualChanceForEachFile);
		//Morph - added by AndCycle, Equal Chance For Each File

        // Added by MoNKi [ SlugFiller: -lowIdRetry- ]
		m_htiLowIdRetry = m_ctrlTreeOptions.InsertItem(_T("LowID retries"), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiMisc);
		m_ctrlTreeOptions.AddEditBox(m_htiLowIdRetry, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// End SlugFiller

		m_ctrlTreeOptions.SendMessage(WM_VSCROLL, SB_TOP);
		m_bInitializedTreeOpts = true;
	}
	
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiEnableDownloadInRed, m_bEnableDownloadInRed); //MORPH - Added by IceCream, show download in red
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiEnableDownloadInBold, m_bEnableDownloadInBold); //MORPH - Added by SiRoB, show download in Bold
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiShowClientPercentage, m_bShowClientPercentage);
	// MORPH START leuk_he disable catcolor
    DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiDisableCatColors, m_bDisableCatColors); //MORPH - Added by SiRoB, show download in Bold
    // MORPH END   leuk_he disable catcolor

	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiShowModstring, m_bShowModstring); 
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiShowFileStatusIcons, m_bShowFileStatusIcons); 
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiDropBlocking, m_bDropBlocking); 
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiShowBlocking, m_bShowBlocking);
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiShowCpuRam, m_bShowCpuRam); 
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiShowMinQR, m_bShowMinQR); 
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiDropSystem, m_bDropSystem); 
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiShowSessionDown, m_bShowSessionDown); 

	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiInfiniteQueue, m_bInfiniteQueue);	//Morph - added by AndCycle, SLUGFILLER: infiniteQueue
#ifdef SPARE_TRICKLE
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiDontRemoveSpareTrickleSlot, m_bDontRemoveSpareTrickleSlot); //Morph - added by AndCycle, Dont Remove Spare Trickle Slot
#endif

#ifdef DOWN_OVERHEAD
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiUseDownloadOverhead, m_bUseDownloadOverhead);//Morph - leuk_he use download overhead in upload
#endif
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiCompressLevel, m_iCompressLevel); //Morph - Compresslevel
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiUseCompression, m_bUseCompression); 
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiUseAutoCompression, m_bAutoCompress); 
	DDV_MinMaxInt(pDX, m_iCompressLevel,1,9);//Morph - Compresslevel
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiDisplayFunnyNick, m_bFunnyNick);//MORPH - Added by SiRoB, Optionnal funnynick display

	/*
	//MORPH START - Added by SiRoB, Datarate Average Time Management
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiDownloadDataRateAverageTime, m_iDownloadDataRateAverageTime);//MORPH - Added by SiRoB, Upload Splitting Class
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiUploadDataRateAverageTime, m_iUploadDataRateAverageTime);//MORPH - Added by SiRoB, Upload Splitting Class
	//MORPH END   - Added by SiRoB, Datarate Average Time Management
	*/
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiGlobalDataRateFriend, m_iGlobalDataRateFriend);//MORPH - Added by SiRoB, Upload Splitting Class
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiMaxGlobalDataRateFriend, m_iMaxGlobalDataRateFriend);//MORPH - Added by SiRoB, Upload Splitting Class
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiMaxClientDataRateFriend, m_iMaxClientDataRateFriend);//MORPH - Added by SiRoB, Upload Splitting Class
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiGlobalDataRatePowerShare, m_iGlobalDataRatePowerShare);//MORPH - Added by SiRoB, Upload Splitting Class
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiMaxGlobalDataRatePowerShare, m_iMaxGlobalDataRatePowerShare);//MORPH - Added by SiRoB, Upload Splitting Class
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiMaxClientDataRatePowerShare, m_iMaxClientDataRatePowerShare);//MORPH - Added by SiRoB, Upload Splitting Class
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiMaxClientDataRate, m_iMaxClientDataRate);//MORPH - Added by SiRoB, Upload Splitting Class
	
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
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiBrokenURLs, m_sBrokenURLs); //MORPH - Added by WiZaRd, Fix broken HTTP downloads

	// Mighty Knife: Report hashing files, Log friendlist activities
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiReportHashingFiles, m_bReportHashingFiles); 
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiLogFriendlistActivities, m_bLogFriendlistActivities); 
	// [end] Mighty Knife

	// Mighty Knife: Static server handling
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiDontRemoveStaticServers, m_bDontRemoveStaticServers); 
	// [end] Mighty Knife
   
    // NEO: QS - [QuickStart]
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiQuickStartEnable, m_uQuickStart);
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiQuickStartTime, m_iQuickStartTime);
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiQuickStartTimePerFile, m_iQuickStartTimePerFile);
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiQuickMaxConperFive, m_iQuickMaxConperFive);
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiQuickMaxHalfOpen, m_iQuickMaxHalfOpen);
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiQuickMaxConnections, m_iQuickMaxConnections);
	// NEO: QS END

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
	// ==> Limit PS by amount of data uploaded - Stulle
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiPsAmountLimit, m_iPsAmountLimit);
	DDV_MinMaxInt(pDX, m_iPsAmountLimit, 0, MAX_PS_AMOUNT_LIMIT);
	// <== Limit PS by amount of data uploaded - Stulle
	//MORPH START - Added by SiRoB, Avoid misusing of powersharing
	DDX_TreeRadio(pDX, IDC_MORPH_OPTS, m_htiPowershareMode, m_iPowershareMode);
	//MORPH END   - Added by SiRoB, Avoid misusing of powersharing
	//Morph Start - added by AndCyle, selective PS internal Prio
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiPowershareInternalPrio, m_bPowershareInternalPrio);
	//Morph End - added by AndCyle, selective PS internal Prio
#ifdef COMMUNITY	
	// Mighty Knife: Community visualization
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiCommunityName, m_sCommunityName);
	// [end] Mighty Knife
    //this is bad using enum for radio button...need (int &) ^*&^#*^$(, by AndCycle
#endif
    // Added by MoNKi [ SlugFiller: -lowIdRetry- ]
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiLowIdRetry, m_iLowIdRetry);
	DDV_MinMaxInt(pDX, m_iLowIdRetry, 0, 255);
	// End SlugFiller

	//EastShare - added by AndCycle, IP to Country
	DDX_TreeRadio(pDX, IDC_MORPH_OPTS, m_htiIP2CountryName, /*(int &)*/ m_iIP2CountryName);
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiIP2CountryShowFlag, m_bIP2CountryShowFlag);
	//EastShare - added by AndCycle, IP to Country
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiMMOpen, m_bMMOpen);
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiDirWatcher, m_bDirWatcher);	// Tux: Feature: Automatic shared files updater

	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiEnableEqualChanceForEachFile, m_bEnableEqualChanceForEachFile);//Morph - added by AndCycle, Equal Chance For Each File

	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiEnablePreferShareAll, m_bEnablePreferShareAll);//EastShare - PreferShareAll by AndCycle
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiIsPayBackFirst, m_bIsPayBackFirst);//EastShare - added by AndCycle, Pay Back First
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiPayBackFirstLimit, m_iPayBackFirstLimit); //MORPH - Added by SiRoB, Pay Back First Tweak
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiOnlyDownloadCompleteFiles, m_bOnlyDownloadCompleteFiles);//EastShare - Added by AndCycle, Only download complete files v2.1 (shadow)
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiSaveUploadQueueWaitTime, m_bSaveUploadQueueWaitTime);//Morph - added by AndCycle, Save Upload Queue Wait Time (MSUQWT)
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiEnableChunkDots, m_bEnableChunkDots);//EastShare - Added by Pretender, Option for ChunkDots
}

BOOL CPPgMorph::OnInitDialog()
{
	m_bEnableDownloadInRed = thePrefs.enableDownloadInRed; //MORPH - Added by IceCream, show download in red
	m_bEnableDownloadInBold = thePrefs.m_bShowActiveDownloadsBold; //MORPH - Added by SiRoB, show download in Bold
	m_bShowClientPercentage = thePrefs.m_bShowClientPercentage;
	// MORPH START leuk_he disable catcolor
	m_bDisableCatColors   =  thePrefs.m_bDisableCatColors ;
	// MORPH START leuk_he disable catcolor
    /*
	//MORPH START - Added by SiRoB, Datarate Average Time Management
	m_iDownloadDataRateAverageTime = thePrefs.m_iDownloadDataRateAverageTime/1000;
	m_iUploadDataRateAverageTime = thePrefs.m_iUploadDataRateAverageTime/1000;
	//MORPH END   - Added by SiRoB, Datarate Average Time Management
	*/
	//MORPH START - Added by Stulle, Global Source Limit
	m_bGlobalHL = thePrefs.IsUseGlobalHL();
	m_iGlobalHL = thePrefs.GetGlobalHL();
	//MORPH END   - Added by Stulle, Global Source Limit

	m_bShowModstring = thePrefs.m_bShowRuntimeOnTitle; 
	m_bShowFileStatusIcons = thePrefs.m_bShowFileStatusIcons; 
    m_bDropBlocking = thePrefs.m_bDropBlockers; 
	m_bShowBlocking = thePrefs.m_showblockratio; 
	m_bShowCpuRam = thePrefs.m_ShowCpu; 
	m_bShowMinQR = thePrefs.m_bShowMinQR; 
	m_bDropSystem = thePrefs.m_bAutoDropSystem; 
	m_bShowSessionDown = thePrefs.m_bShowSessionDownload; 
	m_bMMOpen = thePrefs.m_bMMOpen;
	m_bDirWatcher = thePrefs.GetDirectoryWatcher();	// Tux: Feature: Automatic shared files updater

	m_bInfiniteQueue = thePrefs.infiniteQueue;	//Morph - added by AndCycle, SLUGFILLER: infiniteQueue
#ifdef SPARE_TRICKLE
	m_bDontRemoveSpareTrickleSlot = thePrefs.m_bDontRemoveSpareTrickleSlot; //Morph - added by AndCycle, Dont Remove Spare Trickle Slot
#endif

#ifdef DOWN_OVERHEAD
	m_bUseDownloadOverhead= thePrefs.m_bUseDownloadOverhead; // MORPH leuk_he include download overhead in upload stats
#endif
    m_iCompressLevel = thePrefs.m_iCompressLevel; //Compresslevel
	m_bUseCompression = thePrefs.m_bUseCompression; // use compression
	m_bAutoCompress = thePrefs.m_bAutoCompress; 
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
	m_iPsAmountLimit = thePrefs.GetPsAmountLimit(); // Limit PS by amount of data uploaded - Stulle

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
	m_sBrokenURLs = thePrefs.GetBrokenURLs(); //MORPH - Added by WiZaRd, Fix broken HTTP downloads

	// Mighty Knife: Report hashing files, Log friendlist activities
	m_bReportHashingFiles = thePrefs.GetReportHashingFiles ();
	m_bLogFriendlistActivities = thePrefs.GetLogFriendlistActivities ();
	// [end] Mighty Knife

	// NEO: QS - [QuickStart]
	m_uQuickStart = thePrefs.m_uQuickStart;
	m_iQuickStartTime = thePrefs.m_iQuickStartTime;
	m_iQuickStartTimePerFile = thePrefs.m_iQuickStartTimePerFile;
	m_iQuickMaxConperFive = thePrefs.m_iQuickMaxConperFive;
	m_iQuickMaxHalfOpen = thePrefs.m_iQuickMaxHalfOpen;
	m_iQuickMaxConnections = thePrefs.m_iQuickMaxConnections;
	// NEO: QS END

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
#ifdef COMMUNITY	
	// Mighty Knife: Community visualization
	m_sCommunityName = thePrefs.m_sCommunityName;
	// [end] Mighty Knife
#endif
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

	m_bEnableEqualChanceForEachFile = thePrefs.IsEqualChanceEnable();//Morph - added by AndCycle, Equal Chance For Each File
	
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
	thePrefs.m_bShowFileStatusIcons = m_bShowFileStatusIcons; 
    thePrefs.m_bDropBlockers = m_bDropBlocking; 
	thePrefs.m_showblockratio = m_bShowBlocking; 
	thePrefs.m_ShowCpu = m_bShowCpuRam; 
	thePrefs.m_bShowMinQR = m_bShowMinQR; 
	thePrefs.m_bAutoDropSystem = m_bDropSystem; 
	thePrefs.m_bShowSessionDownload = m_bShowSessionDown; 
	thePrefs.m_bMMOpen = m_bMMOpen;

	thePrefs.infiniteQueue = m_bInfiniteQueue;	//Morph - added by AndCycle, SLUGFILLER: infiniteQueue
#ifdef SPARE_TRICKLE
	thePrefs.m_bDontRemoveSpareTrickleSlot = m_bDontRemoveSpareTrickleSlot; //Morph - added by AndCycle, Dont Remove Spare Trickle Slot
#endif

#ifdef DOWN_OVERHEAD
	thePrefs.m_bUseDownloadOverhead= m_bUseDownloadOverhead; // MORPH leuk_he include download overhead in upload stats
#endif
	thePrefs.m_iCompressLevel = m_iCompressLevel; // morph settable compression
	thePrefs.m_bUseCompression = m_bUseCompression; // use compression
	thePrefs.m_bAutoCompress = m_bAutoCompress; 
	thePrefs.m_bFunnyNick = m_bFunnyNick;//MORPH - Added by SiRoB, Optionnal funnynick display

    bool updateLegend = false;
    /*
	//MORPH START - Added by SiRoB, Datarate Average Time Management
	updateLegend = thePrefs.m_iDownloadDataRateAverageTime/1000 != m_iDownloadDataRateAverageTime;
	thePrefs.m_iDownloadDataRateAverageTime = 1000*max(1, m_iDownloadDataRateAverageTime);
	updateLegend |= thePrefs.m_iUploadDataRateAverageTime/1000 != m_iUploadDataRateAverageTime;
	thePrefs.m_iUploadDataRateAverageTime = 1000*max(1, m_iUploadDataRateAverageTime);
	//MORPH END   - Added by SiRoB, Datarate Average Time Management
    */
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
	thePrefs.SetBrokenURLs(m_sBrokenURLs); //MORPH - Added by WiZaRd, Fix broken HTTP downloads
// Tux: Feature: Automatic shared files updater [start]
	if (thePrefs.m_bDirectoryWatcher != m_bDirWatcher) {
		thePrefs.m_bDirectoryWatcher = m_bDirWatcher;
		theApp.ResetDirectoryWatcher();
	}
	// Tux: Feature: Automatic shared files updater [end]
	// Mighty Knife: Report hashing files, Log friendlist activities
	thePrefs.SetReportHashingFiles (m_bReportHashingFiles);
	thePrefs.SetLogFriendlistActivities (m_bLogFriendlistActivities);
	// [end] Mighty Knife

	// Mighty Knife: Static server handling
	thePrefs.SetDontRemoveStaticServers (m_bDontRemoveStaticServers);
	// [end] Mighty Knife

	theApp.scheduler->SaveOriginals(); //Added by SiRoB, Fix for Param used in scheduler

        // NEO: QS - [QuickStart]
		thePrefs.m_uQuickStart = m_uQuickStart;
		if(m_iQuickStartTime<TIM_QUICK_START_MIN||m_iQuickStartTime>TIM_QUICK_START_MAX)
			m_iQuickStartTime=TIM_QUICK_START_DEF;
		thePrefs.m_iQuickStartTime = m_iQuickStartTime;
		if(m_iQuickStartTimePerFile<VAL_QUICK_START_PF_MIN||m_iQuickStartTimePerFile>VAL_QUICK_START_PF_MAX)
			m_iQuickStartTimePerFile=VAL_QUICK_START_PF_DEF;
		thePrefs.m_iQuickStartTimePerFile = m_iQuickStartTimePerFile;
		thePrefs.m_iQuickMaxConperFive = m_iQuickMaxConperFive;
		thePrefs.m_iQuickMaxHalfOpen = m_iQuickMaxHalfOpen;
		thePrefs.m_iQuickMaxConnections = m_iQuickMaxConnections;
		// NEO: QS END

// ==> Limit PS by amount of data uploaded - Stulle
	thePrefs.PsAmountLimit = m_iPsAmountLimit;
	// <== Limit PS by amount of data uploaded - Stulle
    thePrefs.m_iPowershareMode = m_iPowershareMode;//MORPH - Added by SiRoB, Avoid misusing of powersharing
	thePrefs.m_iSpreadbarSetStatus = m_iSpreadbar; //MORPH	- Added by AndCycle, SLUGFILLER: Spreadbars - per file
	thePrefs.hideOS = m_iHideOS;	//MORPH - Added by SiRoB, SLUGFILLER: hideOS
	thePrefs.selectiveShare = m_bSelectiveShare; //MORPH - Added by SiRoB, SLUGFILLER: hideOS
	thePrefs.ShareOnlyTheNeed = m_iShareOnlyTheNeed!=0; //MORPH - Added by SiRoB, SHARE_ONLY_THE_NEED
	//MORPH START - Added by SiRoB, POWERSHARE Limit
	thePrefs.PowerShareLimit = m_iPowerShareLimit;
	theApp.sharedfiles->UpdatePartsInfo();
	//MORPH END   - Added by SiRoB, POWERSHARE Limit
#ifdef COMMUNITY	
	// Mighty Knife: Community visualization
	_stprintf (thePrefs.m_sCommunityName,_T("%s"), m_sCommunityName);
	// [end] Mighty Knife
#endif
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

	//Morph - added by AndCycle, Save Upload Queue Wait Time (MSUQWT)
	if(m_bSaveUploadQueueWaitTime != thePrefs.m_bSaveUploadQueueWaitTime)	bRestartApp = true;
	thePrefs.m_bSaveUploadQueueWaitTime = m_bSaveUploadQueueWaitTime;
	//Morph - added by AndCycle, Save Upload Queue Wait Time (MSUQWT)

	//Morph - added by AndCycle, Equal Chance For Each File
	thePrefs.m_bEnableEqualChanceForEachFile = m_bEnableEqualChanceForEachFile;
	//Morph - added by AndCycle, Equal Chance For Each File

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
		// NEO: QS - [QuickStart]
		if (m_htiQuickStart) m_ctrlTreeOptions.SetItemText(m_htiQuickStart,GetResString(IDS_X_QUICK_START));
		if (m_htiQuickStartEnable) m_ctrlTreeOptions.SetItemText(m_htiQuickStartEnable,GetResString(IDS_X_QUICK_START_ENABLE));
		if (m_htiQuickStartTime) m_ctrlTreeOptions.SetEditLabel(m_htiQuickStartTime,GetResString(IDS_X_QUICK_START_TIME));
		if (m_htiQuickStartTimePerFile) m_ctrlTreeOptions.SetEditLabel(m_htiQuickStartTimePerFile,GetResString(IDS_X_QUICK_START_TIME_PER_FILE));
		if (m_htiQuickMaxConperFive) m_ctrlTreeOptions.SetEditLabel(m_htiQuickMaxConperFive,GetResString(IDS_X_QUICK_START_MAXPER5));
		if (m_htiQuickMaxHalfOpen) m_ctrlTreeOptions.SetEditLabel(m_htiQuickMaxHalfOpen,GetResString(IDS_X_QUICK_START_MAXHALF));
		if (m_htiQuickMaxConnections) m_ctrlTreeOptions.SetEditLabel(m_htiQuickMaxConnections,GetResString(IDS_X_QUICK_START_MAXCON));
		// NEO: QS END
		if (m_htiEnableDownloadInRed) m_ctrlTreeOptions.SetItemText(m_htiEnableDownloadInRed, GetResString(IDS_DOWNLOAD_IN_YELLOW)); //MORPH - Added by IceCream, show download in red
		if (m_htiEnableDownloadInBold) m_ctrlTreeOptions.SetItemText(m_htiEnableDownloadInBold, GetResString(IDS_DOWNLOAD_IN_BOLD)); //MORPH - Added by SiRoB, show download in Bold
		if (m_htiShowClientPercentage) m_ctrlTreeOptions.SetItemText(m_htiShowClientPercentage, GetResString(IDS_CLIENTPERCENTAGE));		
		if (m_htiInfiniteQueue) m_ctrlTreeOptions.SetItemText(m_htiInfiniteQueue, GetResString(IDS_INFINITEQUEUE));	//Morph - added by AndCycle, SLUGFILLER: infiniteQueue
#ifdef SPARE_TRICKLE
		if (m_htiDontRemoveSpareTrickleSlot)m_ctrlTreeOptions.SetItemText(m_htiDontRemoveSpareTrickleSlot, GetResString(IDS_DONTREMOVESPARETRICKLESLOT));//Morph - added by AndCycle, Dont Remove Spare Trickle Slot
#endif

#ifdef DOWN_OVERHEAD
		if (m_htiUseDownloadOverhead)m_ctrlTreeOptions.SetItemText(m_htiUseDownloadOverhead, GetResString(IDS_USEDOWNLOADOVERHEAD));//Morph 
#endif
		if (m_htiDisplayFunnyNick) m_ctrlTreeOptions.SetItemText(m_htiDisplayFunnyNick, GetResString(IDS_DISPLAYFUNNYNICK));//MORPH - Added by SiRoB, Optionnal funnynick display
	
		//MORPH START - Added by Stulle, Global Source Limit
		if (m_htiGlobalHL)m_ctrlTreeOptions.SetItemText(m_htiGlobalHL, GetResString(IDS_SUC_ENABLED));
		if (m_htiGlobalHlLimit)m_ctrlTreeOptions.SetEditLabel(m_htiGlobalHlLimit, GetResString(IDS_GLOBAL_HL_LIMIT));
		//MORPH END   - Added by Stulle, Global Source Limit
		/*
		//MORPH START - Added by SiRoB, Datarate Average Time Management
		if (m_htiDownloadDataRateAverageTime) m_ctrlTreeOptions.SetEditLabel(m_htiDownloadDataRateAverageTime, GetResString(IDS_DOWNLOAD) + _T(" ") + GetResString(IDS_DATARATEAVERAGETIME));
		if (m_htiUploadDataRateAverageTime) m_ctrlTreeOptions.SetEditLabel(m_htiUploadDataRateAverageTime, GetResString(IDS_PW_CON_UPLBL) + _T(" ") + GetResString(IDS_DATARATEAVERAGETIME));
		//MORPH END   - Added by SiRoB, Datarate Average Time Management
		*/
		//MORPH START - Added by SiRoB, Upload Splitting Class
		if (m_htiGlobalDataRateFriend) m_ctrlTreeOptions.SetEditLabel(m_htiGlobalDataRateFriend, GetResString(IDS_MINDATARATEFRIEND));
		if (m_htiMaxClientDataRateFriend) m_ctrlTreeOptions.SetEditLabel(m_htiMaxClientDataRateFriend, GetResString(IDS_MAXCLIENTDATARATEFRIEND));
		if (m_htiGlobalDataRatePowerShare) m_ctrlTreeOptions.SetEditLabel(m_htiGlobalDataRatePowerShare, GetResString(IDS_MINDATARATEPOWERSHARE));
		if (m_htiMaxClientDataRatePowerShare) m_ctrlTreeOptions.SetEditLabel(m_htiMaxClientDataRatePowerShare, GetResString(IDS_MAXCLIENTDATARATEPOWERSHARE));
		if (m_htiMaxClientDataRate) m_ctrlTreeOptions.SetEditLabel(m_htiMaxClientDataRate, GetResString(IDS_MAXCLIENTDATARATE));
		if (m_htiMaxGlobalDataRatePowerShare)m_ctrlTreeOptions.SetEditLabel(m_htiMaxGlobalDataRatePowerShare, GetResString(IDS_MAXDATARATEPOWERSHARE));
		//MORPH END   - Added by SiRoB, Upload Splitting Class
		
		if (m_htiMisc) m_ctrlTreeOptions.SetItemText(m_htiMisc, GetResString(IDS_PW_MISC));
		if (m_htiMMOpen) m_ctrlTreeOptions.SetItemText(m_htiMMOpen, GetResString(IDS_MM_OPEN));

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
// ==> Limit PS by amount of data uploaded - Stulle
		if (m_htiPsAmountLimit) m_ctrlTreeOptions.SetEditLabel(m_htiPsAmountLimit, GetResString(IDS_PS_AMOUNT_LIMIT));
		// <== Limit PS by amount of data uploaded - Stulle
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

		// Added by MoNKi [ SlugFiller: -lowIdRetry- ]
		if(m_htiLowIdRetry)	m_ctrlTreeOptions.SetEditLabel(m_htiLowIdRetry, GetResString(IDS_RECONNECTONLOWID));
		// End SlugFiller
		if (m_htiEnablePreferShareAll) m_ctrlTreeOptions.SetItemText(m_htiEnablePreferShareAll, GetResString(IDS_PREFER_SHARE_ALL));//EastShare - PreferShareAll by AndCycle
		if (m_htiIsPayBackFirst) m_ctrlTreeOptions.SetItemText(m_htiIsPayBackFirst, GetResString(IDS_PAYBACKFIRST));//EastShare - added by AndCycle, Pay Back First
		if (m_htiPayBackFirstLimit) m_ctrlTreeOptions.SetEditLabel(m_htiPayBackFirstLimit, GetResString(IDS_PAYBACKFIRSTLIMIT));//MORPH - Added by SiRoB, Pay Back First Tweak
		if (m_htiOnlyDownloadCompleteFiles) m_ctrlTreeOptions.SetItemText(m_htiOnlyDownloadCompleteFiles,GetResString(IDS_ONLY_DOWNLOAD_COMPLETE_FILES));//EastShare - Added by AndCycle, Only download complete files v2.1 (shadow)
		if (m_htiSaveUploadQueueWaitTime) m_ctrlTreeOptions.SetItemText(m_htiSaveUploadQueueWaitTime,GetResString(IDS_SAVE_UPLOAD_QUEUE_WAIT_TIME));//Morph - added by AndCycle, Save Upload Queue Wait Time (MSUQWT)
		if (m_htiEnableChunkDots) m_ctrlTreeOptions.SetItemText(m_htiEnableChunkDots, GetResString(IDS_ENABLE_CHUNKDOTS));//EastShare - Added by Pretender, Option for ChunkDots
		if (m_htiBrokenURLs) m_ctrlTreeOptions.SetEditLabel(m_htiBrokenURLs, GetResString(IDS_BROKEN_URLS));
		if (m_htiCompressLevel) m_ctrlTreeOptions.SetEditLabel(m_htiCompressLevel, GetResString(IDS_COMPRESSLEVEL));

        if (m_htiShowMinQR) m_ctrlTreeOptions.SetItemText(m_htiShowMinQR, GetResString(IDS_SHOW_MINQR));
		if (m_htiShowBlocking) m_ctrlTreeOptions.SetItemText(m_htiShowBlocking, GetResString(IDS_SHOW_BLOCKRATIO));
		if (m_htiShowModstring) m_ctrlTreeOptions.SetItemText(m_htiShowModstring,GetResString(IDS_SHOW_RUNTIME));
		if (m_htiShowFileStatusIcons) m_ctrlTreeOptions.SetItemText(m_htiShowFileStatusIcons, GetResString(IDS_SHOW_STATUSICON));
		if (m_htiShowCpuRam) m_ctrlTreeOptions.SetItemText(m_htiShowCpuRam,GetResString(IDS_SHOW_CPU_USAGE));
		if (m_htiShowSessionDown) m_ctrlTreeOptions.SetItemText(m_htiShowSessionDown, GetResString(IDS_SHOW_SESSION_DOWNLOAD));
		if (m_htiDropSystem) m_ctrlTreeOptions.SetItemText(m_htiDropSystem, GetResString(IDS_AUTO_DROP));
		if (m_htiDropBlocking) m_ctrlTreeOptions.SetItemText(m_htiDropBlocking, GetResString(IDS_DROP_BLOCKING));

	}
}

void CPPgMorph::OnDestroy()
{
	m_ctrlTreeOptions.DeleteAllItems();
	m_ctrlTreeOptions.DestroyWindow();
	m_bInitializedTreeOpts = false;
	m_htiDM = NULL;
	m_htiUM = NULL;
	
	m_htiDisp = NULL;
	m_htiEnableDownloadInRed = NULL; //MORPH - Added by IceCream, show download in red
	m_htiEnableDownloadInBold = NULL; //MORPH - Added by SiRoB, show download in Bold
	m_htiShowClientPercentage = NULL;
	//MORPH START - Added by Stulle, Global Source Limit
	m_htiGlobalHL = NULL;
	m_htiGlobalHlLimit = NULL;
	//MORPH END   - Added by Stulle, Global Source Limit
       
	m_htiShowModstring = NULL; 
	m_htiShowFileStatusIcons = NULL; 
	m_htiShowCpuRam = NULL; 
	m_htiShowMinQR = NULL; 
	m_htiDropSystem = NULL; 

	m_htiDisableCatColors = NULL;
	/*
	//MORPH START - Added by SiRoB, Datarate Average Time Management
	m_htiDownloadDataRateAverageTime = NULL;
	m_htiUploadDataRateAverageTime = NULL;
	//MORPH END   - Added by SiRoB, Datarate Average Time Management
	*/
	//MORPH START - Added by SiRoB, Upload Splitting Class
	m_htiFriend = NULL;
	m_htiGlobalDataRateFriend = NULL;
	m_htiMaxClientDataRateFriend = NULL;
	m_htiPowerShare = NULL;
	m_htiGlobalDataRatePowerShare = NULL;
	m_htiMaxClientDataRatePowerShare = NULL;
	m_htiMaxClientDataRate = NULL;
	//MORPH END   - Added by SiRoB, Upload Splitting Class
	
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
	m_htiPsAmountLimit = NULL; // Limit PS by amount of data uploaded - Stulle
	//MORPH START - Added by SiRoB, Avoid misusing of powersharing	
	m_htiPowershareMode = NULL;
	m_htiPowershareDisabled = NULL;
	m_htiPowershareActivated = NULL;
	m_htiPowershareAuto = NULL;
	m_htiPowershareLimited = NULL;
	//MORPH END   - Added by SiRoB, Avoid misusing of powersharing	
	m_htiPowershareInternalPrio = NULL; //Morph - added by AndCyle, selective PS internal Prio

    m_htiEnablePreferShareAll = NULL; //EastShare - PreferShareAll by AndCycle
	m_htiIsPayBackFirst = NULL; //EastShare - added by AndCycle, Pay Back First
	m_htiPayBackFirstLimit = NULL; //MORPH - Added by SiRoB, Pay Back First Tweak
	m_htiOnlyDownloadCompleteFiles = NULL;//EastShare - Added by AndCycle, Only download complete files v2.1 (shadow)
	m_htiSaveUploadQueueWaitTime = NULL;//Morph - added by AndCycle, Save Upload Queue Wait Time (MSUQWT)
	m_htiEnableChunkDots = NULL; //EastShare - Added by Pretender, Option for ChunkDots

	// Added by MoNKi [ SlugFiller: -lowIdRetry- ]
	m_htiLowIdRetry = NULL;
	// End SlugFiller
	m_htiBrokenURLs = NULL; //MORPH - Added by WiZaRd, Fix broken HTTP downloads

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
	//EastShare END - Added by Pretender

	//Morph - added by AndCycle, Equal Chance For Each File
	m_htiEnableEqualChanceForEachFile = NULL;
	//Morph - added by AndCycle, Equal Chance For Each File
// Tux: Feature: Automatic shared files updater [start]
	m_htiDirWatcher = NULL;
	m_bDirWatcher = false;
	// Tux: Feature: Automatic shared files updater [end]

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
