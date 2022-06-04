// PpgMorph.cpp : implementation file
//

#include "stdafx.h"
#include "emule.h"
#include "PPGtooltipped.h" //MORPH leuk_he addded tooltipped
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
#include "Ntservice.h"
#include "TransferDlg.h" // MORPH START leuk_he disable catcolor


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
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

CPPgMorph::CPPgMorph()
//MORPH START leuk_he tooltipped
/*
    : CPropertyPage(CPPgMorph::IDD)
*/
	: CPPgtooltipped(CPPgMorph::IDD)
//MORPH END leuk_he tooltipped
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
	m_htiUpSecu = NULL;
	m_htiDlSecu = NULL;
	m_htiDisp = NULL;
	m_htiEnableDownloadInRed = NULL; //MORPH - Added by IceCream, show download in red
	m_htiEnableDownloadInBold = NULL; //MORPH - Added by SiRoB, show download in Bold
	m_htiShowClientPercentage = NULL; //MORPH - Added by SiRoB, show download in Bold
	//MORPH START - Added by Stulle, Global Source Limit
	m_htiGlobalHlGroup = NULL;
	m_htiGlobalHL = NULL;
	m_htiGlobalHlLimit = NULL;
	//MORPH END   - Added by Stulle, Global Source Limit
        //MORPH START - Added by schnulli900, filter clients with failed downloads [Xman]
	m_htiFilterClientFailedDown = NULL; 
        //MORPH END   - Added by schnulli900, filter clients with failed downloads [Xman]
	m_htiEnableAntiLeecher = NULL; //MORPH - Added by IceCream, activate Anti-leecher
	m_htiEnableAntiCreditHack = NULL; //MORPH - Added by IceCream, activate Anti-CreditHack
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
	m_htiHighProcess = NULL; //MORPH - Added by IceCream, high process priority
	m_htiInfiniteQueue = NULL;	//Morph - added by AndCycle, SLUGFILLER: infiniteQueue
	m_htiDontRemoveSpareTrickleSlot = NULL; //Morph - added by AndCycle, Dont Remove Spare Trickle Slot
	m_htiUseDownloadOverhead = NULL ;//
	m_htiCompressLevel =NULL ;// morph settable compresslevel
	m_htiUseCompression=NULL ;// morph Use compress 

	m_htiDisplayFunnyNick = NULL;//MORPH - Added by SiRoB, Optionnal funnynick display
	m_htiDisableCatColors = NULL;
	m_htiClientQueueProgressBar = NULL; //MORPH - Added by Commander, ClientQueueProgressBar
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
		int iImgUM = 8; // default icon
		int iImgDYNUP = 8; // default icon
		int iImgDM = 8;
		//MORPH START - Added by SiRoB, khaos::categorymod+
		int iImgSCC = 8;
		int iImgSAC = 8;
		int iImgA4AF = 8;
		int iImgTimeRem = 8;
		//MORPH END - Added by SiRoB, khaos::categorymod+
		int iImgSecu = 8;
 		int iImgDisp = 8;
		int iImgGlobal = 8;
		//MORPH START - Added by SiRoB, Upload Splitting Class
		int iImgFriend = 8;
		int iImgPowerShare = 8;
		int iImgNormal = 8;
		//MORPH END   - Added by SiRoB, Upload Splitting Class
	    int iImgConTweaks = 8; // Stulle slotlimit
		
		CImageList* piml = m_ctrlTreeOptions.GetImageList(TVSIL_NORMAL);
		if (piml){
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
			iImgSecu = piml->Add(CTempIconLoader(_T("SECURITY")));
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
        //		m_htiDlSecu = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_SECURITY), iImgSecu, m_htiDM); leuk_he nothing under this securty icon right now
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
		
		m_htiUpSecu = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_SECURITY), iImgSecu, m_htiUM);
		m_htiEnableAntiLeecher = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ANTI_LEECHER), m_htiUpSecu, m_bEnableAntiLeecher); //MORPH - Added by IceCream, Enable Anti-leecher
		m_htiEnableAntiCreditHack = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ANTI_CREDITHACK), m_htiUpSecu, m_bEnableAntiCreditHack); //MORPH - Added by IceCream, Enable Anti-CreditHack
		//MORPH START - Added by schnulli900, filter clients with failed downloads [Xman]
		m_htiFilterClientFailedDown = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_FILTER_CLIENTFAILEDDOWN), m_htiUpSecu, m_bFilterClientFailedDown); 
		//MORPH END   - Added by schnulli900, filter clients with failed downloads [Xman]

		//MORPH START - Added by Commander, ClientQueueProgressBar
		m_htiUpDisplay = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_PW_DISPLAY), iImgDisp, m_htiUM);
		m_htiClientQueueProgressBar = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_CLIENTQUEUEPROGRESSBAR), m_htiUpDisplay, m_bClientQueueProgressBar); //MORPH - Added by IceCream, Enable Anti-leecher
	    //MORPH END - Added by Commander, ClientQueueProgressBar
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
		//MORPH START - Added by IceCream, high process priority
		m_htiHighProcess = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_HIGHPROCESS), TVI_ROOT, m_bHighProcess);
		//MORPH END   - Added by IceCream, high process priority
		m_htiDisplayFunnyNick = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DISPLAYFUNNYNICK), TVI_ROOT, m_bFunnyNick);//MORPH - Added by SiRoB, Optionnal funnynick display
		// Mighty Knife: Report hashing files, Log friendlist activities
		m_htiReportHashingFiles = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MORPH_RFHA), TVI_ROOT, m_bReportHashingFiles);
		m_htiLogFriendlistActivities = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MORPH_RAIF), TVI_ROOT, m_bLogFriendlistActivities);
		// [end] Mighty Knife

		// Mighty Knife: Static server handling
		m_htiDontRemoveStaticServers = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MORPH_KSSERV), TVI_ROOT, m_bDontRemoveStaticServers);
		// [end] Mighty Knife

		m_ctrlTreeOptions.Expand(m_htiDM, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiUM, TVE_EXPAND);
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

        //MORPH START - Added by schnulli900, filter clients with failed downloads [Xman]
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiFilterClientFailedDown, m_bFilterClientFailedDown); 
        //MORPH END   - Added by schnulli900, filter clients with failed downloads [Xman]
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiEnableAntiLeecher, m_bEnableAntiLeecher); //MORPH - Added by IceCream, enable Anti-leecher
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiEnableAntiCreditHack, m_bEnableAntiCreditHack); //MORPH - Added by IceCream, enable Anti-CreditHack
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiInfiniteQueue, m_bInfiniteQueue);	//Morph - added by AndCycle, SLUGFILLER: infiniteQueue
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiDontRemoveSpareTrickleSlot, m_bDontRemoveSpareTrickleSlot); //Morph - added by AndCycle, Dont Remove Spare Trickle Slot
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiUseDownloadOverhead, m_bUseDownloadOverhead);//Morph - leuk_he use download overhead in upload
	DDX_TreeEdit(pDX, IDC_MORPH_OPTS, m_htiCompressLevel, m_iCompressLevel); //Morph - Compresslevel
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiUseCompression, m_bUseCompression); //Morph - Compresslevel
	DDV_MinMaxInt(pDX, m_iCompressLevel,1,9);//Morph - Compresslevel
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiDisplayFunnyNick, m_bFunnyNick);//MORPH - Added by SiRoB, Optionnal funnynick display
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiClientQueueProgressBar, m_bClientQueueProgressBar); //MORPH - Added by Commander, ClientQueueProgressBar
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

	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiHighProcess, m_bHighProcess); //MORPH - Added by IceCream, high process priority 

	// Mighty Knife: Report hashing files, Log friendlist activities
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiReportHashingFiles, m_bReportHashingFiles); 
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiLogFriendlistActivities, m_bLogFriendlistActivities); 
	// [end] Mighty Knife

	// Mighty Knife: Static server handling
	DDX_TreeCheck(pDX, IDC_MORPH_OPTS, m_htiDontRemoveStaticServers, m_bDontRemoveStaticServers); 
	// [end] Mighty Knife
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

        //MORPH START - Added by schnulli900, filter clients with failed downloads [Xman]
	m_bFilterClientFailedDown = thePrefs.m_bFilterClientFailedDown; 
        //MORPH END   - Added by schnulli900, filter clients with failed downloads [Xman]
	m_bEnableAntiLeecher = thePrefs.enableAntiLeecher; //MORPH - Added by IceCream, enabnle Anti-leecher
	m_bEnableAntiCreditHack = thePrefs.enableAntiCreditHack; //MORPH - Added by IceCream, enabnle Anti-CreditHack
	m_bInfiniteQueue = thePrefs.infiniteQueue;	//Morph - added by AndCycle, SLUGFILLER: infiniteQueue
	m_bDontRemoveSpareTrickleSlot = thePrefs.m_bDontRemoveSpareTrickleSlot; //Morph - added by AndCycle, Dont Remove Spare Trickle Slot
	m_bUseDownloadOverhead= thePrefs.m_bUseDownloadOverhead; // MORPH leuk_he include download overhead in upload stats
    m_iCompressLevel = thePrefs.m_iCompressLevel; //Compresslevel
	m_bUseCompression = thePrefs.m_bUseCompression; // use compression
	m_bFunnyNick = thePrefs.m_bFunnyNick;//MORPH - Added by SiRoB, Optionnal funnynick display
	m_bClientQueueProgressBar = thePrefs.m_bClientQueueProgressBar;//MORPH - Added by Commander, ClientQueueProgressBar
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
	//MORPH START - Added by IceCream, high process priority
	m_bHighProcess = thePrefs.GetEnableHighProcess();
	//MORPH END   - Added by IceCream, high process priority
	
	// Mighty Knife: Report hashing files, Log friendlist activities
	m_bReportHashingFiles = thePrefs.GetReportHashingFiles ();
	m_bLogFriendlistActivities = thePrefs.GetLogFriendlistActivities ();
	// [end] Mighty Knife

	// Mighty Knife: Static server handling
	m_bDontRemoveStaticServers = thePrefs.GetDontRemoveStaticServers ();
	// [end] Mighty Knife

	CPropertyPage::OnInitDialog();
	InitTooltips(&m_ctrlTreeOptions);
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
        //MORPH START - Added by schnulli900, filter clients with failed downloads [Xman]
	thePrefs.m_bFilterClientFailedDown = m_bFilterClientFailedDown; 
        //MORPH END   - Added by schnulli900, filter clients with failed downloads [Xman]
	thePrefs.enableAntiLeecher = m_bEnableAntiLeecher; //MORPH - Added by IceCream, enable Anti-leecher
	thePrefs.enableAntiCreditHack = m_bEnableAntiCreditHack; //MORPH - Added by IceCream, enable Anti-CreditHack
	thePrefs.infiniteQueue = m_bInfiniteQueue;	//Morph - added by AndCycle, SLUGFILLER: infiniteQueue
	thePrefs.m_bDontRemoveSpareTrickleSlot = m_bDontRemoveSpareTrickleSlot; //Morph - added by AndCycle, Dont Remove Spare Trickle Slot
	thePrefs.m_bUseDownloadOverhead= m_bUseDownloadOverhead; // MORPH leuk_he include download overhead in upload stats
	thePrefs.m_iCompressLevel = m_iCompressLevel; // morph settable compression
	thePrefs.m_bUseCompression = m_bUseCompression; // use compression
	thePrefs.m_bFunnyNick = m_bFunnyNick;//MORPH - Added by SiRoB, Optionnal funnynick display
	thePrefs.m_bClientQueueProgressBar = m_bClientQueueProgressBar; //MORPH - Added by Commander, ClientQueueProgressBar
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

	//MORPH START - Added by IceCream, high process priority
	thePrefs.SetEnableHighProcess(m_bHighProcess);
	//MORPH END   - Added by IceCream, high process priority

	// Mighty Knife: Report hashing files, Log friendlist activities
	thePrefs.SetReportHashingFiles (m_bReportHashingFiles);
	thePrefs.SetLogFriendlistActivities (m_bLogFriendlistActivities);
	// [end] Mighty Knife

	// Mighty Knife: Static server handling
	thePrefs.SetDontRemoveStaticServers (m_bDontRemoveStaticServers);
	// [end] Mighty Knife

	theApp.scheduler->SaveOriginals(); //Added by SiRoB, Fix for Param used in scheduler

	SetModified(FALSE);


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
		// first we set tools for the groups
		SetTool(m_htiDM,IDS_DM_TIP);
		SetTool(m_htiSCC,IDS_SCC_TIP);
		SetTool(m_htiSAC,IDS_SAC_TIP);
		//SetTool(m_htiAdvA4AFMode,IDS_DEFAULT_TIP);
		SetTool(m_htiTimeRemainingMode,IDS_REMTIMEAVRREAL_TIP);
		SetTool(m_htiDisp,IDS_PW_DISPLAY_TIP);
		SetTool(m_htiUM,IDS_UM_TIP);
		SetTool(m_htiDYNUP,IDS_DYNUPLOAD_TIP);
		SetTool(m_htiUpSecu,IDS_SECURITY_TIP);
		SetTool(m_htiUpDisplay,IDS_PW_DISPLAY_TIP2);
		SetTool(m_htiFriend,IDS_PW_FRIENDS_TIP);
		SetTool(m_htiPowerShare,IDS_POWERSHARE_TIP);
		SetTool(m_htiSlotLimitGroup,IDS_SLOT_LIMIT_GROUP_TIP);
		SetTool(m_htiGlobalHlGroup ,IDS_GLOBAL_HL_TIP);

		GetDlgItem(IDC_WARNINGMORPH)->SetWindowText(GetResString(IDS_WARNINGMORPH));
		CString Buffer;
		//MORPH START - Added by Yun.SF3, Auto DynUp changing
		if (m_htiDynUpAutoSwitching) { m_ctrlTreeOptions.SetItemText(m_htiDynUpAutoSwitching, GetResString(IDS_AUTODYNUPSWITCHING));
		                                SetTool(m_htiDynUpAutoSwitching,IDS_AUTODYNUPSWITCHING_TIP);  }
		if (m_htiMaxConnectionsSwitchBorder){
			Buffer.Format(GetResString(IDS_MAXCONNECTIONSSWITCHBORDER),100);
			m_ctrlTreeOptions.SetEditLabel(m_htiMaxConnectionsSwitchBorder, Buffer);
			SetTool(m_htiMaxConnectionsSwitchBorder,IDS_MAXCONNECTIONSSWITCHBORDER_TIP);
		}
		//MORPH END - Added by Yun.SF3, Auto DynUp changing
		if (m_htiDynUpOFF) {m_ctrlTreeOptions.SetItemText(m_htiDynUpOFF, GetResString(IDS_DISABLED));
							SetTool(m_htiDynUpOFF,IDS_DYNDISABLED_TIP);
		}
		if (m_htiDynUpSUC) {m_ctrlTreeOptions.SetItemText(m_htiDynUpSUC, GetResString(IDS_SUC));
							SetTool(m_htiDynUpSUC,IDS_SUC_TIP);
		}
		if (m_htiDynUpUSS) {m_ctrlTreeOptions.SetItemText(m_htiDynUpUSS, GetResString(IDS_USS));
                            SetTool(m_htiDynUpUSS,IDC_USS_TIP);
		}
		if (m_htiSUCLog) { m_ctrlTreeOptions.SetItemText(m_htiSUCLog, GetResString(IDS_SUC_LOG));
						   SetTool(m_htiSUCLog,IDC_SUC_LOG_TIP);
		}
		if (m_htiSUCHigh){
			Buffer.Format(GetResString(IDS_SUC_HIGH),900);
			m_ctrlTreeOptions.SetEditLabel(m_htiSUCHigh, Buffer);
			SetTool(m_htiSUCHigh,IDS_SUC_HIGH_TIP);
		}
		if (m_htiSUCLow){
			Buffer.Format(GetResString(IDS_SUC_LOW),600);
			m_ctrlTreeOptions.SetEditLabel(m_htiSUCLow, Buffer);
			SetTool(m_htiSUCLow,IDS_SUC_LOW_TIP);
		}
		if (m_htiSUCPitch){
			Buffer.Format(GetResString(IDS_SUC_PITCH),3000);
			m_ctrlTreeOptions.SetEditLabel(m_htiSUCPitch, Buffer);
			SetTool(m_htiSUCPitch,IDS_SUC_PITCH_TIP);
		}
		if (m_htiSUCDrift){
			Buffer.Format(GetResString(IDS_SUC_DRIFT),50);
			m_ctrlTreeOptions.SetEditLabel(m_htiSUCDrift, Buffer);
			SetTool(m_htiSUCDrift,IDS_SUC_DRIFT_TIP);
		}
		if (m_htiUSSLog) {m_ctrlTreeOptions.SetItemText(m_htiUSSLog, GetResString(IDS_USS_LOG));
						  SetTool(m_htiUSSLog,IDS_USS_LOG_TIP);
		}
		if (m_htiUSSUDP) m_ctrlTreeOptions.SetItemText(m_htiUSSUDP, GetResString(IDS_USS_UDP));//MORPH - Added by SiRoB, USS UDP preferency
		if (m_htiUSSLimit) m_ctrlTreeOptions.SetItemText(m_htiUSSLimit, GetResString(IDS_USS_USEMAXPING));
		if (m_htiUSSPingLimit){
			Buffer.Format(GetResString(IDS_USS_MAXPING),200);
			m_ctrlTreeOptions.SetEditLabel(m_htiUSSPingLimit, Buffer);
		}
		if (m_htiUSSPingTolerance){
			Buffer.Format(GetResString(IDS_USS_PINGTOLERANCE),800);
			m_ctrlTreeOptions.SetEditLabel(m_htiUSSPingTolerance, Buffer);
			SetTool(m_htiUSSPingTolerance,IDS_USS_PINGTOLERANCE_TIP);
		}
		if (m_htiUSSGoingUpDivider){
			Buffer.Format(GetResString(IDS_USS_GOINGUPDIVIDER),1000);
			m_ctrlTreeOptions.SetEditLabel(m_htiUSSGoingUpDivider, Buffer);
			SetTool(m_htiUSSGoingUpDivider,IDS_USS_GOINGUPDIVIDER_TIP);
		}
		if (m_htiUSSGoingDownDivider){
			Buffer.Format(GetResString(IDS_USS_GOINGDOWNDIVIDER),1000);
			m_ctrlTreeOptions.SetEditLabel(m_htiUSSGoingDownDivider, Buffer);
			SetTool(m_htiUSSGoingDownDivider,IDS_USS_GOINGDOWNDIVIDER_TIP);
		}
		if (m_htiUSSNumberOfPings){
			Buffer.Format(GetResString(IDS_USS_NUMBEROFPINGS),1);
			m_ctrlTreeOptions.SetEditLabel(m_htiUSSNumberOfPings, Buffer);
			SetTool(m_htiUSSNumberOfPings,IDS_USS_NUMBEROFPINGS_TIP);
		}
		if (m_htiMinUpload) {m_ctrlTreeOptions.SetEditLabel(m_htiMinUpload, GetResString(IDS_MINUPLOAD));
							 SetTool(m_htiMinUpload,IDS_MINUPLOAD_TIP);
		}
		if (m_htiEnableDownloadInRed) {m_ctrlTreeOptions.SetItemText(m_htiEnableDownloadInRed, GetResString(IDS_DOWNLOAD_IN_RED)); //MORPH - Added by IceCream, show download in red
									   SetTool(m_htiEnableDownloadInRed,IDS_DOWNLOAD_IN_RED_TIP);
		}
		if (m_htiEnableDownloadInBold) {m_ctrlTreeOptions.SetItemText(m_htiEnableDownloadInBold, GetResString(IDS_DOWNLOAD_IN_BOLD)); //MORPH - Added by SiRoB, show download in Bold
										SetTool(m_htiEnableDownloadInBold,IDS_DOWNLOAD_IN_BOLD_TIP);
		}
		if (m_htiShowClientPercentage) {m_ctrlTreeOptions.SetItemText(m_htiShowClientPercentage, GetResString(IDS_CLIENTPERCENTAGE));
										SetTool(m_htiShowClientPercentage,IDS_CLIENTPERCENTAGE_TIP);
		}
		//MORPH START - Added by schnulli900, filter clients with failed downloads [Xman]
		if (m_htiFilterClientFailedDown) {m_ctrlTreeOptions.SetItemText(m_htiFilterClientFailedDown, GetResString(IDS_FILTER_CLIENTFAILEDDOWN)); 
									 SetTool(m_htiFilterClientFailedDown,IDS_FILTER_CLIENTFAILEDDOWN_TIP);
		}
		//MORPH END   - Added by schnulli900, filter clients with failed downloads [Xman]
		if (m_htiEnableAntiLeecher) {m_ctrlTreeOptions.SetItemText(m_htiEnableAntiLeecher, GetResString(IDS_ANTI_LEECHER)); //MORPH - Added by IceCream, enable Anti-leecher
									 SetTool(m_htiEnableAntiLeecher,IDS_ANTI_LEECHER_TIP);
		}
		if (m_htiEnableAntiCreditHack) {m_ctrlTreeOptions.SetItemText(m_htiEnableAntiCreditHack, GetResString(IDS_ANTI_CREDITHACK)); //MORPH - Added by IceCream, enable Anti-CreditHack
										SetTool(m_htiEnableAntiCreditHack,IDS_ANTI_CREDITHACK_TIP);
		}
		if (m_htiInfiniteQueue) {m_ctrlTreeOptions.SetItemText(m_htiInfiniteQueue, GetResString(IDS_INFINITEQUEUE));	//Morph - added by AndCycle, SLUGFILLER: infiniteQueue
								 SetTool(m_htiInfiniteQueue,IDS_INFINITEQUEUE_TIP);
		}						  
		if (m_htiDontRemoveSpareTrickleSlot){m_ctrlTreeOptions.SetItemText(m_htiDontRemoveSpareTrickleSlot, GetResString(IDS_DONTREMOVESPARETRICKLESLOT));//Morph - added by AndCycle, Dont Remove Spare Trickle Slot
											 SetTool(m_htiDontRemoveSpareTrickleSlot,IDS_DONTREMOVESPARETRICKLESLOT_TIP);
		}
		if (m_htiUseDownloadOverhead){m_ctrlTreeOptions.SetItemText(m_htiUseDownloadOverhead, GetResString(IDS_USEDOWNLOADOVERHEAD));//Morph 
											 SetTool(m_htiUseDownloadOverhead,IDS_USEDOWNLOADOVERHEAD_TIP);
		}


		if (m_htiDisplayFunnyNick) {m_ctrlTreeOptions.SetItemText(m_htiDisplayFunnyNick, GetResString(IDS_DISPLAYFUNNYNICK));//MORPH - Added by SiRoB, Optionnal funnynick display
				                    SetTool(m_htiDisplayFunnyNick,IDS_DISPLAYFUNNYNICK_TIP);
		}
		if (m_htiClientQueueProgressBar) {m_ctrlTreeOptions.SetItemText(m_htiClientQueueProgressBar, GetResString(IDS_CLIENTQUEUEPROGRESSBAR));//MORPH - Added by Commander, ClientQueueProgressBar
									      SetTool(m_htiClientQueueProgressBar,IDS_CLIENTQUEUEPROGRESSBAR_TIP);
		}
		// ==> [MoNKi: -USS initial TTL-] - Stulle
		if (m_htiUSSTTL) {m_ctrlTreeOptions.SetEditLabel(m_htiUSSTTL, GetResString(IDS_USS_INITIAL_TTL));
		SetTool(m_htiUSSTTL,IDS_USS_INITIAL_TTL_TIP); }
		// <== [MoNKi: -USS initial TTL-] - Stulle

		//MORPH START - Added by Stulle, Global Source Limit
		if (m_htiGlobalHL){ m_ctrlTreeOptions.SetItemText(m_htiGlobalHL, GetResString(IDS_SUC_ENABLED));
		SetTool(m_htiGlobalHL ,IDS_SUC_ENABLED_TIP);}
		if (m_htiGlobalHlLimit){ m_ctrlTreeOptions.SetEditLabel(m_htiGlobalHlLimit, GetResString(IDS_GLOBAL_HL_LIMIT));
		SetTool(m_htiGlobalHlLimit,IDS_GLOBAL_HL_LIMIT_TIP);}
		//MORPH END   - Added by Stulle, Global Source Limit
		//MORPH START - Added by SiRoB, Datarate Average Time Management
		if (m_htiDownloadDataRateAverageTime) {m_ctrlTreeOptions.SetEditLabel(m_htiDownloadDataRateAverageTime, GetResString(IDS_DATARATEAVERAGETIME));
											   SetTool(m_htiDownloadDataRateAverageTime,IDS_DOWNLOADDATARATEAVERAGETIME_TIP);
		}
		if (m_htiUploadDataRateAverageTime) {m_ctrlTreeOptions.SetEditLabel(m_htiUploadDataRateAverageTime, GetResString(IDS_DATARATEAVERAGETIME));
											 SetTool(m_htiUploadDataRateAverageTime,IDS_UPLOADDATARATEAVERAGETIME_TIP);
		}
		//MORPH END   - Added by SiRoB, Datarate Average Time Management
		//MORPH START - Added by SiRoB, Upload Splitting Class
		if (m_htiGlobalDataRateFriend) {m_ctrlTreeOptions.SetEditLabel(m_htiGlobalDataRateFriend, GetResString(IDS_MINDATARATEFRIEND));
										SetTool(m_htiGlobalDataRateFriend,IDS_MINDATARATEFRIEND_TIP);
		}
		if (m_htiMaxClientDataRateFriend) {m_ctrlTreeOptions.SetEditLabel(m_htiMaxClientDataRateFriend, GetResString(IDS_MAXCLIENTDATARATEFRIEND));
										   SetTool(m_htiMaxClientDataRateFriend,IDS_MAXCLIENTDATARATEFRIEND_TIP);
		}
		if (m_htiGlobalDataRatePowerShare) {m_ctrlTreeOptions.SetEditLabel(m_htiGlobalDataRatePowerShare, GetResString(IDS_MINDATARATEPOWERSHARE));
											SetTool(m_htiGlobalDataRatePowerShare,IDS_MINDATARATEPOWERSHARE_TIP);
		}
		if (m_htiMaxClientDataRatePowerShare) {m_ctrlTreeOptions.SetEditLabel(m_htiMaxClientDataRatePowerShare, GetResString(IDS_MAXCLIENTDATARATEPOWERSHARE));
											   SetTool(m_htiMaxClientDataRatePowerShare,IDS_MAXCLIENTDATARATEPOWERSHARE_TIP);
		}
		if (m_htiMaxClientDataRate) {m_ctrlTreeOptions.SetEditLabel(m_htiMaxClientDataRate, GetResString(IDS_MAXCLIENTDATARATE));
									 SetTool(m_htiMaxClientDataRate,IDS_MAXCLIENTDATARATE_TIP);
		}
		if (m_htiMaxGlobalDataRatePowerShare){ m_ctrlTreeOptions.SetEditLabel(m_htiMaxGlobalDataRatePowerShare, GetResString(IDS_MAXDATARATEPOWERSHARE));
		                                       SetTool(m_htiMaxGlobalDataRatePowerShare,IDS_MAXDATARATEPOWERSHARE_TIP);
		}
		//MORPH END   - Added by SiRoB, Upload Splitting Class
		// ==> Slot Limit - Stulle
		if (m_htiSlotLimitGroup) {m_ctrlTreeOptions.SetItemText(m_htiSlotLimitGroup, GetResString(IDS_SLOT_LIMIT_GROUP));
		                          SetTool(m_htiSlotLimitGroup,IDS_SLOT_LIMIT_GROUP_TIP);
		}
		if (m_htiSlotLimitNum) {m_ctrlTreeOptions.SetEditLabel(m_htiSlotLimitNum, GetResString(IDS_SLOT_LIMIT_NUM));
								SetTool(m_htiSlotLimitNum,IDS_SLOT_LIMIT_NUM_TIP);
		}
		// <== Slot Limit - Stulle
		//MORPH START - Added by SiRoB, khaos::categorymod+
		if (m_htiShowCatNames) {m_ctrlTreeOptions.SetItemText(m_htiShowCatNames, GetResString(IDS_CAT_SHOWCATNAME));
		                        SetTool(m_htiShowCatNames,IDS_CAT_SHOWCATNAME_TIP);
		}
		if (m_htiSelectCat) { m_ctrlTreeOptions.SetItemText(m_htiSelectCat, GetResString(IDS_CAT_SHOWSELCATDLG));
							  SetTool(m_htiSelectCat,IDS_CAT_SHOWSELCATDLG_TIP);
		}
		if (m_htiUseAutoCat) {m_ctrlTreeOptions.SetItemText(m_htiUseAutoCat, GetResString(IDS_CAT_USEAUTOCAT));
							  SetTool(m_htiUseAutoCat,IDS_CAT_USEAUTOCAT_TIP);
		}
        if (m_htiUseActiveCat) {m_ctrlTreeOptions.SetItemText(m_htiUseActiveCat, GetResString(IDS_CAT_USEACTIVE));
								SetTool(m_htiUseActiveCat,IDS_CAT_USEACTIVE_TIP);
		}
		if (m_htiAutoSetResOrder) {m_ctrlTreeOptions.SetItemText(m_htiAutoSetResOrder, GetResString(IDS_CAT_AUTORESUMEORD));
		                           SetTool(m_htiAutoSetResOrder,IDS_CAT_AUTORESUMEORD_TIP);
		}
		if (m_htiSmallFileDLPush) {m_ctrlTreeOptions.SetItemText(m_htiSmallFileDLPush, GetResString(IDS_CAT_SMALLFILEDLPUSH));
								   SetTool(m_htiSmallFileDLPush,IDS_CAT_SMALLFILEDLPUSH_TIP);
		}
		if (m_htiResumeFileInNewCat) {m_ctrlTreeOptions.SetEditLabel(m_htiResumeFileInNewCat, GetResString(IDS_CAT_STARTFILESONADD));
		                              SetTool(m_htiResumeFileInNewCat,IDS_CAT_STARTFILESONADD_TIP);
		}
		if (m_bAddRemovedInc) {m_ctrlTreeOptions.SetItemText(m_htiAddRemovedInc, GetResString(IDS_ADD_REMOVED_INC));
								SetTool(m_htiAddRemovedInc,IDS_ADD_REMOVED_INC_TIP);
		}
		if (m_htiSmartA4AFSwapping) {m_ctrlTreeOptions.SetItemText(m_htiSmartA4AFSwapping, GetResString(IDS_A4AF_SMARTSWAP));
									 SetTool(m_htiSmartA4AFSwapping,IDS_A4AF_SMARTSWAP_TIP);
		}
		if (m_htiShowA4AFDebugOutput) {m_ctrlTreeOptions.SetItemText(m_htiShowA4AFDebugOutput, GetResString(IDS_A4AF_SHOWDEBUG));
									   SetTool(m_htiShowA4AFDebugOutput,IDS_A4AF_SHOWDEBUG_TIP);
		}
		if (m_htiAdvA4AFMode){ m_ctrlTreeOptions.SetItemText(m_htiAdvA4AFMode, /*GetResString(IDS_DEFAULT) + " " +*/ GetResString(IDS_A4AF_ADVMODE));
		                       SetTool(m_htiAdvA4AFMode,IDS_A4AF_ADVMODE_TIP);
		}
		if (m_htiDisableAdvA4AF) {m_ctrlTreeOptions.SetItemText(m_htiDisableAdvA4AF, GetResString(IDS_A4AF_DISABLED));
								  SetTool(m_htiDisableAdvA4AF,IDS_A4AF_DISABLED_TIP);
		}
		if (m_htiBalanceSources) {m_ctrlTreeOptions.SetItemText(m_htiBalanceSources, GetResString(IDS_A4AF_BALANCE));
								  SetTool(m_htiBalanceSources,IDS_A4AF_BALANCE_TIP);
		}
		if (m_htiStackSources) {m_ctrlTreeOptions.SetItemText(m_htiStackSources, GetResString(IDS_A4AF_STACK));
								SetTool(m_htiStackSources,IDS_A4AF_STACK_TIP);
		}
		if (m_htiUseSLS) {m_ctrlTreeOptions.SetItemText(m_htiUseSLS, GetResString(IDS_SLS_USESLS));
						  SetTool(m_htiUseSLS,IDS_SLS_USESLS_TIP);
		}
		// khaos::accuratetimerem+
		if (m_htiTimeRemainingMode) { m_ctrlTreeOptions.SetItemText(m_htiTimeRemainingMode, GetResString(IDS_REMTIMEAVRREAL));
									  SetTool(m_htiTimeRemainingMode,IDS_REMTIMEAVRREAL_TIP);
		}
		if (m_htiTimeRemBoth) {m_ctrlTreeOptions.SetItemText(m_htiTimeRemBoth, GetResString(IDS_BOTH));
							   SetTool(m_htiTimeRemBoth,IDS_BOTH_TIP);
		}
		if (m_htiTimeRemRealTime) {m_ctrlTreeOptions.SetItemText(m_htiTimeRemRealTime, GetResString(IDS_REALTIME));
								   SetTool(m_htiTimeRemRealTime,IDS_REALTIME_TIP_TIP);
		}
		if (m_htiTimeRemAverage) {m_ctrlTreeOptions.SetItemText(m_htiTimeRemAverage, GetResString(IDS_AVG));
								  SetTool(m_htiTimeRemAverage,IDS_AVG_TIP);
		}
		// khaos::accuratetimerem-
		//MORPH END - Added by SiRoB, khaos::categorymod+
		//MORPH START - Added by SiRoB, ICS Optional
		if (m_htiUseICS) {m_ctrlTreeOptions.SetItemText(m_htiUseICS, GetResString(IDS_ICS_USEICS));
						  SetTool(m_htiUseICS,IDS_ICS_USEICS_TIP);
		}
		//MORPH START - Added by SiRoB, ICS Optional
		//MORPH START - Added by IceCream, high process priority
		if (m_htiHighProcess) {m_ctrlTreeOptions.SetItemText(m_htiHighProcess, GetResString(IDS_HIGHPROCESS));
							   SetTool(m_htiHighProcess,IDS_HIGHPROCESS_TIP);
		}
		//MORPH END   - Added by IceCream, high process priority
       
		if (m_htiSlotLimitNone) {m_ctrlTreeOptions.SetItemText(m_htiSlotLimitNone, GetResString(IDS_SLOT_LIMIT_NONE));
							SetTool(m_htiSlotLimitNone,IDS_SLOT_LIMIT_NONE_TIP);
		}
		if (m_htiSlotLimitThree) {m_ctrlTreeOptions.SetItemText(m_htiSlotLimitThree, GetResString(IDS_SLOT_LIMIT_THREE));
							SetTool(m_htiSlotLimitThree,IDS_SLOT_LIMIT_THREE_TIP);
		}
		if (m_htiSlotLimitNumB) {m_ctrlTreeOptions.SetItemText(m_htiSlotLimitNumB, GetResString(IDS_SLOT_LIMIT_NUM_B));
							SetTool(m_htiSlotLimitNumB,IDS_SLOT_LIMIT_NUM_TIP);
		}
		if (m_htiReportHashingFiles) {m_ctrlTreeOptions.SetItemText(m_htiReportHashingFiles, GetResString(IDS_MORPH_RFHA));
						  SetTool(m_htiReportHashingFiles,IDS_MORPH_RFHA_TIP);
		}
		if (m_htiLogFriendlistActivities) {m_ctrlTreeOptions.SetItemText(m_htiLogFriendlistActivities, GetResString(IDS_MORPH_RAIF));
						  SetTool(m_htiLogFriendlistActivities,IDS_MORPH_RAIF_TIP);
		}
		if (m_htiDontRemoveStaticServers) {m_ctrlTreeOptions.SetItemText(m_htiDontRemoveStaticServers, GetResString(IDS_MORPH_KSSERV));
						  SetTool(m_htiDontRemoveStaticServers,IDS_MORPH_KSSERV_SIP);
		}
		if (m_htiMaxGlobalDataRateFriend) {m_ctrlTreeOptions.SetEditLabel(m_htiMaxGlobalDataRateFriend, GetResString(IDS_MAXDATARATEFRIEND));
								SetTool(m_htiMaxGlobalDataRateFriend,IDS_MAXDATARATEFRIEND_TIP);
		}
		if (m_htiCompressLevel) {m_ctrlTreeOptions.SetEditLabel(m_htiCompressLevel, GetResString(IDS_COMPRESSLEVEL));
								SetTool(m_htiCompressLevel,IDS_COMPRESSLEVEL_TIP);
		}
		if (m_htiUseCompression) {m_ctrlTreeOptions.SetItemText(m_htiUseCompression, GetResString(IDS_USECOMPRESS));
						  SetTool(m_htiUseCompression,IDS_USECOMPRESS_TIP);
		}
		if (m_htiDisableCatColors) {m_ctrlTreeOptions.SetItemText(m_htiDisableCatColors, GetResString(IDS_DISABLECATCOLORS));
						  SetTool(m_htiDisableCatColors,IDS_DISABLECATCOLOR_TIP);
		}
		if (m_htiPingDataSize) {m_ctrlTreeOptions.SetEditLabel(m_htiPingDataSize, GetResString(IDS_USSPINGDATASIZE));
						  SetTool(m_htiPingDataSize,IDS_PINGDATASIZE_TIP);
		}
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
	m_htiDlSecu = NULL;
	m_htiDisp = NULL;
	m_htiEnableDownloadInRed = NULL; //MORPH - Added by IceCream, show download in red
	m_htiEnableDownloadInBold = NULL; //MORPH - Added by SiRoB, show download in Bold
	m_htiShowClientPercentage = NULL;
	//MORPH START - Added by Stulle, Global Source Limit
	m_htiGlobalHL = NULL;
	m_htiGlobalHlLimit = NULL;
	//MORPH END   - Added by Stulle, Global Source Limit
	m_htiUpSecu = NULL;
        //MORPH START - Added by schnulli900, filter clients with failed downloads [Xman]
	m_htiFilterClientFailedDown = NULL; 
        //MORPH END   - Added by schnulli900, filter clients with failed downloads [Xman]
	m_htiEnableAntiLeecher = NULL; //MORPH - Added by IceCream, enable Anti-leecher
	m_htiEnableAntiCreditHack = NULL; //MORPH - Added by IceCream, enable Anti-CreditHack
	m_htiDisableCatColors = NULL;
	m_htiClientQueueProgressBar = NULL; //MORPH - Added by Commander, ClientQueueProgressBar
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
	m_htiHighProcess = NULL; //MORPH - Added by IceCream, high process priority
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

void CPPgMorph::OnHelp()
{
	//theApp.ShowHelp(0);
}

BOOL CPPgMorph::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgMorph::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}
//MORPH START leuk_he ask on exit
// CAskExit dialog

IMPLEMENT_DYNAMIC(CAskExit, CDialog)
CAskExit::CAskExit()
	: CPPgtooltippedDialog(CAskExit::IDD)
{
}

BOOL CAskExit::OnInitDialog() 
{
   CDialog::OnInitDialog();
   InitTooltips();
     // localize:
    if(m_hWnd)
	{
		GetDlgItem(IDC_EXITQUESTION)->SetWindowText(GetResString(IDS_MAIN_EXIT));
		GetDlgItem(IDC_DONTASKMEAGAINCB)->SetWindowText(GetResString(IDS_DONOTASKAGAIN));
		GetDlgItem(IDYES)->SetWindowText(GetResString(IDS_YES));
		GetDlgItem(IDNO)->SetWindowText(GetResString(IDS_NO));
		GetDlgItem(IDYESSERVICE)->SetWindowText(GetResString(IDS_YESSERVICE));
		GetDlgItem(IDNOMINIMIZE)->SetWindowText(GetResString(IDS_NOMINIMIZE));

        SetTool(IDC_EXITQUESTION,IDC_EXITQUESTION_TIP);
        SetTool(IDYES,IDC_EXITQUESTION_TIP);
        SetTool(IDNO,IDC_EXITQUESTION_TIP);
		SetTool(IDC_DONTASKMEAGAINCB,IDC_DONTASKMEAGAINCB_TIP);
		SetTool(IDYESSERVICE,IDS_YESSERVICETIP);
		SetTool(IDNOMINIMIZE,IDS_NOMINIMIZETIP);

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



CAskExit::~CAskExit()
{
}

void CAskExit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAskExit, CDialog)
	ON_BN_CLICKED(IDYES, OnBnClickedYes)
	ON_BN_CLICKED(IDNO, OnBnClickedCancel)
	ON_BN_CLICKED(IDYESSERVICE, OnBnClickedYesservice)
	ON_BN_CLICKED(IDNOMINIMIZE, OnBnClickedNominimize)
END_MESSAGE_MAP()


// CAskExit message handlers

void CAskExit::OnBnClickedYes()
{   thePrefs.confirmExit= (IsDlgButtonChecked(IDC_DONTASKMEAGAINCB)==0);
	// TODO: Add your control notification handler code here
    EndDialog(IDYES);
}

void CAskExit::OnBnClickedCancel()
{   thePrefs.confirmExit= IsDlgButtonChecked(IDC_DONTASKMEAGAINCB)==0;
	EndDialog(IDNO);
}

//MORPH END leuk_he ask on exit


void CAskExit::OnBnClickedYesservice()
{   NtserviceStartwhenclose=true;
    EndDialog(IDYES);
}

void CAskExit::OnBnClickedNominimize()
{
	if (thePrefs.GetMinToTray())
		theApp.emuledlg->PostMessage(WM_SYSCOMMAND , SC_MINIMIZE, 0);
	else
	    theApp.emuledlg->PostMessage(WM_SYSCOMMAND, MP_MINIMIZETOTRAY, 0);
	 EndDialog(IDNO);
}
