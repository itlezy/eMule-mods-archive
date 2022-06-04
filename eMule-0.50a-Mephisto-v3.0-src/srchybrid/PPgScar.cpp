// PpgStulle.cpp : implementation file
//

/* guess what is included already... LOL */
#include "stdafx.h"
#include "emule.h"
#include "PPgScar.h"
#include "emuledlg.h"
#include "UserMsgs.h"
#include "opcodes.h"
#include "otherfunctions.h"
#include "MuleToolbarCtrl.h" // TBH: minimule - Max
#include "ClientCredits.h" // CreditSystems [EastShare/ MorphXT] - Stulle
#include "log.h"
#include "DownloadQueue.h" // Global Source Limit [Max/Stulle] - Stulle
#include "TransferDlg.h" // CPU/MEM usage [$ick$/Stulle] - Max
#include "XMessageBox.h" // TBH: Backup [TBH/EastShare/MorphXT] - Stulle
#include "sharedfilelist.h" // PowerShare [ZZ/MorphXT] - Stulle
// ==> Design Settings [eWombat/Stulle] - Stulle
#include "SharedFilesWnd.h"
#include "SharedFilesCtrl.h"
#include "ServerWnd.h"
#include "ServerListCtrl.h"
// <== Design Settings [eWombat/Stulle] - Stulle
// ==> Invisible Mode [TPT/MoNKi] - Stulle
#include "TreeOptionsInvisibleModCombo.h"
#include "TreeOptionsInvisibleKeyCombo.h"
// <== Invisible Mode [TPT/MoNKi] - Stulle
#include "FirewallOpener.h" // Improved ICS-Firewall support [MoNKi] - Max
// ==> Random Ports [MoNKi] - Stulle
#include "ClientUDPSocket.h"
#include "ListenSocket.h"
// <== Random Ports [MoNKi] - Stulle
// ==> Design Settings [eWombat/Stulle] - Max
#include "MuleStatusBarCtrl.h"
#include "IrcWnd.h"
#include "KademliaWnd.h"
#include "SearchDlg.h"
#include "ChatWnd.h"
#include "StatisticsDlg.h"
// <== Design Settings [eWombat/Stulle] - Max
#include "PreferencesDlg.h" // Alwasy maximize slot speed [Stulle] - Stulle
#include "ClientList.h" // Timer for ReAsk File Sources [Stulle] - Stulle
// ==> Advanced Updates [MorphXT/Stulle] - Stulle
#include "Scheduler.h"
#include "IPFilter.h"
#include "DLP.h"
// <== Advanced Updates [MorphXT/Stulle] - Stulle

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


///////////////////////////////////////////////////////////////////////////////
// CPPgScar dialog

IMPLEMENT_DYNAMIC(CPPgScar, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgScar, CPropertyPage)
	ON_WM_HSCROLL()
	ON_WM_DESTROY()
	ON_MESSAGE(UM_TREEOPTSCTRL_NOTIFY, OnTreeOptsCtrlNotify)
	ON_NOTIFY(TCN_SELCHANGE, IDC_PPG_SCAR_TAB, OnTabSelectionChange) // Tabbed Preferences [TPT] - Stulle
	// ==> TBH: Backup [TBH/EastShare/MorphXT] - Stulle
	ON_BN_CLICKED(IDC_BACKUPNOW, OnBnClickedBackupnow)
	ON_BN_CLICKED(IDC_DAT, OnBnClickedDat)
	ON_BN_CLICKED(IDC_MET, OnBnClickedMet)
	ON_BN_CLICKED(IDC_INI, OnBnClickedIni)
	ON_BN_CLICKED(IDC_PART, OnBnClickedPart)
	ON_BN_CLICKED(IDC_PARTMET, OnBnClickedPartMet)
	ON_BN_CLICKED(IDC_SELECTALL, OnBnClickedSelectall)
	ON_BN_CLICKED(IDC_AUTOBACKUP, OnBnClickedAutobackup)
	ON_BN_CLICKED(IDC_AUTOBACKUP2, OnBnClickedAutobackup2)
	// <== TBH: Backup [TBH/EastShare/MorphXT] - Stulle
	// ==> Design Settings [eWombat/Stulle] - Stulle
	ON_BN_CLICKED(IDC_COLOR_ON_OFF, OnBnClickedOnOff)
	ON_BN_CLICKED(IDC_COLOR_BOLD, OnBnClickedBold)
	ON_BN_CLICKED(IDC_COLOR_UNDERLINED, OnBnClickedUnderlined)
	ON_BN_CLICKED(IDC_COLOR_ITALIC, OnBnClickedItalic)
    ON_MESSAGE(UM_CPN_SELCHANGE, OnColorPopupSelChange)
	ON_CBN_SELCHANGE(IDC_COLOR_MASTER_COMBO, OnCbnSelchangeStyleselMaster)
	ON_CBN_SELCHANGE(IDC_COLOR_SUB_COMBO, OnCbnSelchangeStyleselSub)
	ON_EN_KILLFOCUS(IDC_COLOR_MASTER_COMBO, OnEnKillfocusMasterCombo)
	ON_EN_KILLFOCUS(IDC_COLOR_SUB_COMBO, OnEnKillfocusSubCombo)
	ON_BN_CLICKED(IDC_EASTEREGG, OnBnClickedEasteregg) // Diabolic Easteregg [Stulle] - Mephisto
	// <== Design Settings [eWombat/Stulle] - Stulle
	// ==> Advanced Updates [MorphXT/Stulle] - Stulle
	ON_EN_CHANGE(IDC_ANTI_LEECH_URL, OnSettingsChange)
	ON_BN_CLICKED(IDC_ANTI_LEECH_UPDATE, OnBnClickedUpdateALUrl)
	ON_BN_CLICKED(IDC_ANTI_LEECH_RESET, OnBnClickedResetALUrl)
	ON_BN_CLICKED(IDC_ANTI_LEECH_STARTUP , OnSettingsChange)
	ON_BN_CLICKED(IDC_ANTI_LEECH_WEEK, OnSettingsChange)
	ON_EN_CHANGE(IDC_IPFILTER_URL, OnSettingsChange)
	ON_BN_CLICKED(IDC_IPFILTER_UPDATE, OnBnClickedUpdateipfurl)
	ON_BN_CLICKED(IDC_IPFILTER_RESET, OnBnClickedResetipfurl)
	ON_BN_CLICKED(IDC_IPFILTER_STARTUP , OnSettingsChange)
	ON_BN_CLICKED(IDC_IPFILTER_WEEK, OnSettingsChange)
	ON_EN_CHANGE(IDC_COUNTRY_URL, OnSettingsChange)
    ON_BN_CLICKED(IDC_COUNTRY_UPDATE, OnBnClickedUpdateipcurl)
	ON_BN_CLICKED(IDC_COUNTRY_RESET, OnBnClickedResetipcurl)
	ON_BN_CLICKED(IDC_COUNTRY_STARTUP , OnSettingsChange)
	// <== Advanced Updates [MorphXT/Stulle] - Stulle
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

CPPgScar::CPPgScar()
	: CPropertyPage(CPPgScar::IDD)
	
	, m_ctrlTreeOptions(theApp.m_iDfltImageListColorFlags)
	, m_ctrlAdvTreeOptions(theApp.m_iDfltImageListColorFlags) // Advanced Options [Official/MorphXT] - Stulle
{
	// ==> Tabbed Preferences [TPT] - Stulle
	// Create an icon list for the tab control
	m_imageList.DeleteImageList();
	m_imageList.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	m_imageList.SetBkColor(CLR_NONE);
	m_imageList.Add(CTempIconLoader(_T("AAAEMULEAPP")));
	m_imageList.Add(CTempIconLoader(_T("BACKUP")));
	m_imageList.Add(CTempIconLoader(_T("SEARCHFILETYPE_PICTURE")));
	m_imageList.Add(CTempIconLoader(_T("TWEAK")));
	m_imageList.Add(CTempIconLoader(_T("UPDATE")));
	m_imageList.Add(CTempIconLoader(_T("SUPPORT")));
	// <== Tabbed Preferences [TPT] - Stulle

	m_bInitializedTreeOpts = false;
	m_htiPush = NULL; // push files - Stulle
	// ==> push small files [sivka] - Stulle
	m_htiEnablePushSmallFile = NULL;
	m_iPushSmallFiles = 0;
	m_htiPushSmallFileBoost = NULL;
	// <== push small files [sivka] - Stulle
	m_htiEnablePushRareFile = NULL;  // push rare file - Stulle

	// ==> FunnyNick [SiRoB/Stulle] - Stulle
	m_htiFnTag = NULL;
	m_htiFnActive = NULL;
	m_htiFnTagMode = NULL;
	m_htiNoTag = NULL;
	m_htiShortTag = NULL;
	m_htiFullTag = NULL;
	m_htiCustomTag = NULL;
	m_htiFnCustomTag = NULL;
	m_htiFnTagAtEnd = NULL;
	// <== FunnyNick [SiRoB/Stulle] - Stulle

	m_htiConTweaks = NULL;
	// ==> Quick start [TPT] - Max
	m_htiQuickStartGroup = NULL;
	m_htiQuickStart = NULL;
	m_htiQuickStartMaxTime = NULL;
	m_htiQuickStartMaxConnPerFive = NULL;
	m_htiQuickStartMaxConn = NULL;
	m_htiQuickStartMaxConnPerFiveBack = NULL;
	m_htiQuickStartMaxConnBack = NULL;
	m_htiQuickStartAfterIPChange = NULL;
	// <== Quick start [TPT] - Max
	// ==> Enforce Ratio [Stulle] - Stulle
	m_htiRatioGroup = NULL;
	m_htiEnforceRatio = NULL;
	m_htiRatioValue = NULL;
	// <== Enforce Ratio [Stulle] - Stulle
	// ==> Improved ICS-Firewall support [MoNKi] - Max
	m_htiICFSupportRoot = NULL;
	m_htiICFSupport = NULL;
	m_htiICFSupportClearAtEnd = NULL;
	m_htiICFSupportServerUDP = NULL;
	// <== Improved ICS-Firewall support [MoNKi] - Max
	// ==> UPnP support [MoNKi] - leuk_he
	m_htiUPnPNatGroup = NULL;
	m_htiUPnPNat = NULL;
	m_htiUpnPNATwebservice = NULL;
	m_htiUpnpBinaddr = NULL;
	m_htiUPnPForceUpdate=NULL;
	// <== UPnP support [MoNKi] - leuk_he 
	// ==> Random Ports [MoNKi] - Stulle
	m_htiRndGrp = NULL;
	m_htiRandomports = NULL;
	m_htiRandomFirstPort = NULL;
	m_htiRandomLastPort = NULL;
	m_htiRandomPortsResetTime = NULL;
	// <== Random Ports [MoNKi] - Stulle
	// ==> Mephisto Upload - Mephisto
	m_htiMephistoUploadGrp = NULL;
	m_htiMephistoWarning1 = NULL;
	m_htiMephistoWarning2 = NULL;
	m_htiMinSlots = NULL;
	m_htiNoNewSlotTimer = NULL;
	m_htiFullLoops = NULL;
	m_htiMonitorLoops = NULL;
	m_htiNotReachedBW = NULL;
	m_htiNoTrickleTimer = NULL;
	m_htiMoveDownKB = NULL;
	// <== Mephisto Upload - Mephisto
	// ==> Multiple Part Transfer [Stulle] - Mephisto
	m_iChunksMode = thePrefs.GetChunksMode();
	m_htiChunksGroup = NULL;
	m_htiChunksScore = NULL;
	m_htiChunksXman = NULL;
	m_htiChunksFinish = NULL;
	m_htiChunksToFinish = NULL;
	m_htiChunksFull = NULL;
	m_htiChunksToUpload = NULL;
	// <== Multiple Part Transfer [Stulle] - Mephisto
	m_htiMaxUpMinutes = NULL; // Adjust max upload time [Stulle] - Mephisto
	m_htiReAskFileSrc = NULL; // Timer for ReAsk File Sources [Stulle] - Stulle
	m_htiACC = NULL; // ACC [Max/WiZaRd] - Max
	m_htiIgnoreThird = NULL; // Do not reserve 1/3 of your uploadlimit for emule [Stulle] - Stulle
	m_htiUlThres = NULL; // Disable accepting only clients who asked within last 30min [Stulle] - Stulle
	m_htiMaxSlotSpeed = NULL; // Alwasy maximize slot speed [Stulle] - Stulle

	// ==> Anti Uploader Ban [Stulle] - Stulle
	m_htiAntiUploaderBanLimit = NULL;
	m_htiAntiCase1 = NULL;
	m_htiAntiCase2 = NULL;
	m_htiAntiCase3 = NULL;
	// <== Anti Uploader Ban [Stulle] - Stulle

	// ==> CreditSystems [EastShare/ MorphXT] - Stulle
	m_iCreditSystem = thePrefs.GetCreditSystem();
	m_htiCreditSystem = NULL;
	m_htiOfficialCredit = NULL;
	m_htiLovelaceCredit = NULL;
	m_htiRatioCredit = NULL;
	m_htiPawcioCredit = NULL;
	m_htiESCredit = NULL;
	m_htiSivkaCredit = NULL;
	m_htiSwatCredit = NULL;
	m_htiXmanCredit = NULL;
	m_htiTk4Credit = NULL;
	m_htiZzulCredit = NULL;
	// <== CreditSystems [EastShare/ MorphXT] - Stulle
	m_htiFineCS = NULL; // Modified FineCS [CiccioBastardo/Stulle] - Stulle
	// ==> Pay Back First [AndCycle/SiRoB/Stulle] - Stulle
	m_htiIsPayBackFirst = NULL;
	m_htiPayBackFirstLimit = NULL;
	m_htiIsPayBackFirst2 = NULL;
	m_htiPayBackFirstLimit2 = NULL;
	// <== Pay Back First [AndCycle/SiRoB/Stulle] - Stulle

	m_htiDisplay = NULL;
	// ==> CPU/MEM usage [$ick$/Stulle] - Max
	m_htiSysInfoGroup = NULL;
	m_htiSysInfo = NULL;
	m_htiSysInfoGlobal = NULL;
	// <== CPU/MEM usage [$ick$/Stulle] - Max
	// ==> Invisible Mode [TPT/MoNKi] - Stulle
	m_htiInvisibleModeRoot = NULL;
	m_htiInvisibleMode = NULL;
	m_htiInvisibleModeMod = NULL;
	m_htiInvisibleModeKey = NULL;
	m_htiInvisibleModeStart = NULL;
	m_bInvisibleMode = false;
	m_sInvisibleModeMod = "";
	m_sInvisibleModeKey = "";
	m_iInvisibleModeActualKeyModifier = 0;
	m_bInvisibleModeStart = false;
	// <== Invisible Mode [TPT/MoNKi] - Stulle
	m_htiShowSrcOnTitle = NULL; // Show sources on title - Stulle
	m_htiShowGlobalHL = NULL; // show global HL - Stulle
	m_htiShowFileHLconst = NULL; // show HL per file constantly - Stulle
	m_htiShowInMSN7 = NULL; // Show in MSN7 [TPT] - Stulle
	m_htiQueueProgressBar = NULL; // Client queue progress bar [Commander] - Stulle
	m_htiTrayComplete = NULL; // Completed in Tray [Stulle] - Stulle
	m_htiColorFeedback = NULL; // Feedback personalization [Stulle] - Stulle
	m_htiShowClientPercentage = NULL; // Show Client Percentage optional [Stulle] - Stulle
	m_htiFollowTheMajority = NULL; // Follow The Majority [AndCycle/Stulle] - Stulle
	m_htiShowSpeedMeter = NULL; // High resolution speedmeter on toolbar [eFMod/Stulle] - Myth88
	// ==> Static Tray Icon [MorphXT] - MyTh88
	m_htiStaticIcon = NULL;
	// <== Static Tray Icon [MorphXT] - MyTh88

	// ==> File Settings [sivka/Stulle] - Stulle
	m_htiFileDefaults = NULL;
	m_htiAutoNNS = NULL;
	m_htiAutoNNSTimer = NULL;
	m_htiAutoNNSLimit = NULL;
	m_htiAutoFQS = NULL;
	m_htiAutoFQSTimer = NULL;
	m_htiAutoFQSLimit = NULL;
	m_htiAutoQRS = NULL;
	m_htiAutoQRSTimer = NULL;
	m_htiAutoQRSMax = NULL;
	m_htiAutoQRSLimit = NULL;
	m_htiAutoQRSWay = NULL;
	m_htiHQRXman = NULL;
	m_htiHQRSivka = NULL;
	m_iHQRXmanDefault = (thePrefs.GetHQRXmanDefault()) ? 1 : 0;
	// <== File Settings [sivka/Stulle] - Stulle

	// ==> TBH: minimule - Max
	m_htiMMGroup = NULL;
	m_htiShowMM = NULL;
	m_htiMMLives = NULL;
	m_htiMMUpdateTime = NULL;
	m_htiMMTrans = NULL;
	m_htiMMCompl = NULL;
	m_htiMMOpen = NULL;
	// <== TBH: minimule - Max

	// ==> Control download priority [tommy_gun/iONiX] - MyTh88
	m_htiAutoDownPrioGroup = NULL;
	m_htiAutoDownPrioOff = NULL;
	m_htiAutoDownPrioPerc = NULL;
	m_htiAutoDownPrioPercVal = NULL;
	m_htiAutoDownPrioSize = NULL;
	m_htiAutoDownPrioSizeVal = NULL;
	m_htiAutoDownPrioValGroup = NULL;
	m_htiAutoDownPrioLow = NULL;
	m_htiAutoDownPrioNormal = NULL;
	m_htiAutoDownPrioHigh = NULL;
	// <== Control download priority [tommy_gun/iONiX] - MyTh88

	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	m_htiSCC = NULL;
	m_htiDlMode = NULL;
	m_htiDlNone = NULL;
	m_htiDlAlph = NULL;
	m_htiDlLP = NULL;
	m_htiShowCatNames = NULL;
	m_htiSelectCat = NULL;
	m_htiUseActiveCat = NULL;
	m_htiAutoSetResOrder = NULL;
	m_htiSmallFileDLPush = NULL;
	m_htiResumeFileInNewCat = NULL;
	m_htiUseAutoCat = NULL;
	m_htiAddRemovedInc = NULL;
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

	m_htiSharedPrefs = NULL; // Shared Files Management [Stulle] - Stulle
	// ==> PowerShare [ZZ/MorphXT] - Stulle
	m_htiPowershareMode = NULL;
	m_htiPowershareDisabled = NULL;
	m_htiPowershareActivated = NULL;
	m_htiPowershareAuto = NULL;
	m_htiPowershareLimited = NULL;
	m_htiPowerShareLimit = NULL;
	// <== PowerShare [ZZ/MorphXT] - Stulle
	m_htiPsAmountLimit = NULL; // Limit PS by amount of data uploaded [Stulle] - Stulle
	// ==> Spread Credits Slot [Stulle] - Stulle
	m_htiSpreadCreditsSlotGroup = NULL;
	m_htiSpreadCreditsSlot = NULL;
	m_htiSpreadCreditsSlotCounter = NULL;
	// <== Spread Credits Slot [Stulle] - Stulle
	// ==> Release Bonus [sivka] - Stulle
	m_htiReleaseBonusGroup = NULL;
	m_htiReleaseBonus0 = NULL;
	m_htiReleaseBonus1 = NULL;
	m_htiReleaseBonusDays = NULL;
	m_htiReleaseBonusDaysEdit = NULL;
	// <== Release Bonus [sivka] - Stulle
	m_htiReleaseScoreAssurance = NULL; // Release Score Assurance [Stulle] - Stulle
	m_htiSpreadBars = NULL; // Spread bars [Slugfiller/MorphXT] - Stulle
	// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	m_htiHideOS = NULL;
	m_htiSelectiveShare = NULL;
	m_htiShareOnlyTheNeed = NULL;
	// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	m_htiFairPlay = NULL; // Fair Play [AndCycle/Stulle] - Stulle

	m_htiMisc = NULL;
	// ==> Global Source Limit [Max/Stulle] - Stulle
	m_htiGlobalHlGroup = NULL;
	m_htiGlobalHL = NULL;
	m_htiGlobalHlLimit = NULL;
	m_htiGlobalHlAll = NULL;
	m_htiGlobalHlDefault = NULL;
	// <== Global Source Limit [Max/Stulle] - Stulle
	// ==> Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	m_htiEmulatorGroup = NULL;
	m_htiEmuMLDonkey = NULL;
	m_htiEmueDonkey = NULL;
	m_htiEmueDonkeyHybrid = NULL;
	m_htiEmuShareaza = NULL;
	m_htiEmuLphant = NULL;
	m_htiLogEmulator = NULL;
	// <== Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	// ==> Automatic shared files updater [MoNKi] - Stulle
	m_htiAutoSharedGroup = NULL;
	m_htiAutoSharedUpdater = NULL;
	m_htiSingleSharedDirUpdater = NULL;
	m_htiTimeBetweenReloads = NULL;
	// <== Automatic shared files updater [MoNKi] - Stulle
	m_htiSUQWT = NULL; // SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
	m_htiStartupSound = NULL; // Startupsound [Commander] - mav744
	m_htiCompressLevel = NULL; // Adjust Compress Level [Stulle] - Stulle

	// ==> Advanced Options [Official/MorphXT] - Stulle
	m_bInitializedAdvTreeOpts = false;
	m_hti_AdvMiniMule=NULL;
	m_hti_bMiniMuleAutoClose=NULL;
	m_hti_iMiniMuleTransparency=NULL;

	m_hti_MediaInfo=NULL;
	m_hti_sMediaInfo_MediaInfoDllPath=NULL;
	m_hti_bMediaInfo_RIFF=NULL;
	m_hti_bMediaInfo_ID3LIB=NULL;
#ifdef HAVE_QEDIT_H
	m_hti_MediaInfo_MediaDet=NULL;
#endif//HAVE_QEDIT_H
	m_hti_MediaInfo_RM=NULL;
#ifdef HAVE_WMSDK_H
	m_hti_MediaInfo_WM=NULL;
#endif//HAVE_WMSDK_H

	m_hti_AdvDisplay=NULL;
	m_hti_m_bRestoreLastMainWndDlg=NULL;
	m_hti_m_bRestoreLastLogPane=NULL;
	m_hti_m_iStraightWindowStyles=NULL;
	m_hti_m_bRTLWindowsLayout=NULL;
	m_hti_m_iMaxChatHistory=NULL;
	m_hti_maxmsgsessions=NULL;
	m_htidatetimeformat = NULL;
	m_htidatetimeformat4lists = NULL;
	m_htiShowVerticalHourMarkers=NULL;
	m_htiReBarToolbar = NULL;
	m_htiIconflashOnNewMessage = NULL;
	m_htiShowCopyEd2kLinkCmd = NULL;
	m_htiUpdateQueue = NULL;
	m_htiRepaint = NULL;
	m_htiExtraPreviewWithMenu = NULL;
	m_htiShowUpDownIconInTaskbar = NULL;
	m_htiForceSpeedsToKB = NULL;

	m_hti_Log=NULL;
	m_hti_m_iLogFileFormat=NULL;
	m_hti_iMaxLogBuff=NULL;
	m_htidatetimeformat4log = NULL;
	m_htiLogError = NULL;
	m_htiLogWarning = NULL;
	m_htiLogSuccess = NULL;

	m_hti_bCheckComctl32 =NULL;
	m_hti_bCheckShell32=NULL;
	m_hti_bIgnoreInstances=NULL;
	m_hti_sNotifierMailEncryptCertName=NULL;
	m_hti_m_iPreviewSmallBlocks=NULL;
	m_hti_m_bPreviewCopiedArchives=NULL;
	m_hti_m_bPreviewOnIconDblClk=NULL;
	m_hti_sInternetSecurityZone=NULL;
	m_hti_sTxtEditor=NULL;
	m_hti_iServerUDPPort=NULL;
	m_hti_m_bRemoveFilesToBin=NULL;
	m_hti_HighresTimer=NULL;
	m_hti_TrustEveryHash=NULL;
	m_hti_InspectAllFileTypes=NULL;
	m_hti_PreferRestrictedOverUser=NULL;
	m_hti_WebFileUploadSizeLimitMB =NULL;
	m_hti_AllowedIPs=NULL;
	m_hti_UseUserSortedServerList=NULL;
	m_hti_DebugSearchResultDetailLevel=NULL;
	m_htiAdjustNTFSDaylightFileTime = NULL;
	m_htidontcompressavi = NULL;
	m_htiICH = NULL;
    m_dwBindAddr = NULL;
	m_htiFileBufferTimeLimit = NULL;
	m_htiRearrangeKadSearchKeywords = NULL;
	m_htiBeeper = NULL;
	m_htiMsgOnlySec = NULL;
	m_htiDisablePeerCache = NULL;
	m_htiKeepUnavailableFixedSharedDirs = NULL;
	// <== Advanced Options [Official/MorphXT] - Stulle
}

CPPgScar::~CPPgScar()
{
}

// ==> Advanced Updates [MorphXT/Stulle] - Stulle
void SysTimeToStr(LPSYSTEMTIME st, LPTSTR str)
{
	TCHAR sDate[15];
	sDate[0] = _T('\0');
	GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, st, NULL, sDate, 100);
	TCHAR sTime[15];
	sTime[0] = _T('\0');
	GetTimeFormat(LOCALE_USER_DEFAULT, 0, st, NULL ,sTime ,100);
	_stprintf(str, _T("%s %s"), sDate, sTime);
}
// <== Advanced Updates [MorphXT/Stulle] - Stulle

void CPPgScar::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PPG_SCAR_TAB, m_tabCtr); // Tabbed Preferences [TPT] - Stulle
	DDX_Control(pDX, IDC_SCAR_OPTS, m_ctrlTreeOptions);
	if (!m_bInitializedTreeOpts)
	{
		int iImgPush = 8;
		int iImgFunnyNick = 8;
		int iImgConTweaks = 8;
		int iImgQuickstart = 8;
		int iImgRatio = 8;
		int iImgICF = 8;
		int iImgUPnP = 8;
		int iImgRndGrp = 8;
		int iImgMephUlGrp = 8;
		int iImgWarning = 8;
		int iImgChunks = 8;
		int iImgCS = 8;
		int iImgDisplay = 8;
		int iImgSysInfo = 8;
		int iImgInvisibleMode = 8;
		int iImgDropDefaults = 8;
		int iImgMinimule = 8;
		int iImgAutoDownPrio = 8; //MyTh88 auto download priority
		int iImgPriority = 8; //MyTh88 auto download priority
		int iImgSCC = 8;
		int iImgDlMode = 8;
		int iImgSharedPrefs = 8;
		int iImgPS = 8;
		int iImgSpreadCredits = 8;
		int iImgReleaseBonus = 8;
		int iImgMisc = 8;
		int iImgGlobal = 8;
		int iImgEmulate = 8;
		int iImgASFU = 8;

		CImageList* piml = m_ctrlTreeOptions.GetImageList(TVSIL_NORMAL);
		if (piml){
			iImgPush = piml->Add(CTempIconLoader(_T("SPEED")));
			iImgFunnyNick = piml->Add(CTempIconLoader(_T("FUNNYNICK")));
			iImgConTweaks =  piml->Add(CTempIconLoader(_T("CONNECTION")));
			iImgQuickstart = piml->Add(CTempIconLoader(_T("QUICKSTART"))); // Thx to the eF-Mod team for the icon
			iImgRatio = piml->Add(CTempIconLoader(_T("TRANSFERUPDOWN")));
			iImgICF = piml->Add(CTempIconLoader(_T("PROXY")));
			iImgUPnP = piml->Add(CTempIconLoader(_T("UPNP")));
			iImgRndGrp = piml->Add(CTempIconLoader(_T("PORTS")));
			iImgMephUlGrp = piml->Add(CTempIconLoader(_T("AAAEMULEAPP")));
			iImgWarning = piml->Add(CTempIconLoader(_T("PREFWARNING")));
			iImgChunks = piml->Add(CTempIconLoader(_T("UPLOAD")));
			iImgCS = piml->Add(CTempIconLoader(_T("STATSCLIENTS")));
			iImgDisplay = piml->Add(CTempIconLoader(_T("DISPLAY")));
			iImgSysInfo = piml->Add(CTempIconLoader(_T("SYSINFO")));
			iImgInvisibleMode = piml->Add(CTempIconLoader(_T("INVMODE")));
			iImgDropDefaults = piml->Add(CTempIconLoader(_T("DROPDEFAULTS")));
			iImgMinimule = piml->Add(CTempIconLoader(_T("MINIMULE")));
			iImgAutoDownPrio = piml->Add(CTempIconLoader(_T("DOWNLOAD"))); //MyTh88 auto download priority
			iImgPriority = piml->Add(CTempIconLoader(_T("FILEPRIORITY"))); //MyTh88 auto download priority
			iImgSCC = piml->Add(CTempIconLoader(_T("CATEGORY")));
			iImgDlMode = piml->Add(CTempIconLoader(_T("DLMODE")));
			iImgSharedPrefs = piml->Add(CTempIconLoader(_T("SHAREDFILESMANAGEMENT")));
			iImgPS = piml->Add(CTempIconLoader(_T("FILEPOWERSHARE")));
			iImgSpreadCredits = piml->Add(CTempIconLoader(_T("SPREADCREDITS")));
			iImgReleaseBonus = piml->Add(CTempIconLoader(_T("RELEASEBONUS")));
			iImgMisc = piml->Add(CTempIconLoader(_T("SRCUNKNOWN")));
			iImgGlobal = piml->Add(CTempIconLoader(_T("SEARCHMETHOD_GLOBAL")));
			iImgEmulate = piml->Add(CTempIconLoader(_T("EMULATEICON")));
			iImgASFU = piml->Add(CTempIconLoader(_T("SHAREDFILESLIST")));
		}
		
		CString Buffer;
		m_htiPush = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_PUSH), iImgPush, TVI_ROOT); // push files - Stulle
		// ==> push small files [sivka]  - Stulle
		m_htiEnablePushSmallFile = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PUSH_SMALL), m_htiPush, m_bEnablePushSmallFile);
		m_htiPushSmallFileBoost = m_ctrlTreeOptions.InsertItem(GetResString(IDS_PUSH_SMALL_BOOST), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiPush);
		m_ctrlTreeOptions.AddEditBox(m_htiPushSmallFileBoost, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// <== push small files [sivka]  - Stulle
		m_htiEnablePushRareFile = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PUSH_RARE), m_htiPush, m_bEnablePushRareFile); // push rare file - Stulle

		// ==> FunnyNick [SiRoB/Stulle] - Stulle
		m_htiFnTag = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_FUNNYNICK), iImgFunnyNick, TVI_ROOT);
		m_htiFnActive = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ENABLE), m_htiFnTag, m_bFnActive);
		m_htiFnTagMode = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_FN_TAG), iImgFunnyNick, m_htiFnTag);
		m_htiNoTag = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_NO_TAG), m_htiFnTagMode, m_iFnTag == 0);
		m_htiShortTag = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SHORT_TAG), m_htiFnTagMode, m_iFnTag == 1);
		m_htiFullTag = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_FULL_TAG), m_htiFnTagMode, m_iFnTag == 2);
		m_htiCustomTag = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_CUSTOM_TAG),m_htiFnTagMode,m_iFnTag == 3);
		m_htiFnCustomTag = m_ctrlTreeOptions.InsertItem(GetResString(IDS_SET_CUSTOM_TAG), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiCustomTag);
		m_ctrlTreeOptions.AddEditBox(m_htiFnCustomTag, RUNTIME_CLASS(CTreeOptionsEdit));
		if(m_bFnActive) m_ctrlTreeOptions.Expand(m_htiCustomTag, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiFnTagMode, TVE_EXPAND);
		m_htiFnTagAtEnd = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_FN_TAG_AT_END), m_htiFnTag, m_bFnTagAtEnd);
		// <== FunnyNick [SiRoB/Stulle] - Stulle

		m_htiConTweaks = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_CON_TWEAKS), iImgConTweaks, TVI_ROOT);
		// ==> Quick start [TPT] - Max
		m_htiQuickStartGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_QUICK_START_GROUP), iImgQuickstart, m_htiConTweaks);
		m_htiQuickStart = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_QUICK_START), m_htiQuickStartGroup, m_bQuickStart);
		m_htiQuickStartMaxTime = m_ctrlTreeOptions.InsertItem(GetResString(IDS_QUICK_START_MAX_TIME), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiQuickStart);
		m_ctrlTreeOptions.AddEditBox(m_htiQuickStartMaxTime, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiQuickStartMaxConnPerFive = m_ctrlTreeOptions.InsertItem(GetResString(IDS_QUICK_START_MAX_CONN_PER_FIVE), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiQuickStart);
		m_ctrlTreeOptions.AddEditBox(m_htiQuickStartMaxConnPerFive, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiQuickStartMaxConn = m_ctrlTreeOptions.InsertItem(GetResString(IDS_QUICK_START_MAX_CONN), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiQuickStart);
		m_ctrlTreeOptions.AddEditBox(m_htiQuickStartMaxConn, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiQuickStartMaxConnPerFiveBack = m_ctrlTreeOptions.InsertItem(GetResString(IDS_QUICK_START_MAX_CONN_PER_FIVE_BACK), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiQuickStart);
		m_ctrlTreeOptions.AddEditBox(m_htiQuickStartMaxConnPerFiveBack, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiQuickStartMaxConnBack = m_ctrlTreeOptions.InsertItem(GetResString(IDS_QUICK_START_MAX_CONN_BACK), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiQuickStart);
		m_ctrlTreeOptions.AddEditBox(m_htiQuickStartMaxConnBack, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_ctrlTreeOptions.Expand(m_htiQuickStart, TVE_EXPAND);
		m_htiQuickStartAfterIPChange = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_QUICK_START_AFTER_IP_CHANGE), m_htiQuickStartGroup, m_bQuickStartAfterIPChange);
		// <== Quick start [TPT] - Max
		// ==> Enforce Ratio [Stulle] - Stulle
		m_htiRatioGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_RATIO_GROUP), iImgRatio, m_htiConTweaks);
		m_htiEnforceRatio = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ENFORCE_RATIO), m_htiRatioGroup, m_bEnforceRatio);
		m_htiRatioValue = m_ctrlTreeOptions.InsertItem(GetResString(IDS_RATIO_VALUE), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiRatioGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiRatioValue, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// <== Enforce Ratio [Stulle] - Stulle
		// ==> Improved ICS-Firewall support [MoNKi] - Max
		m_htiICFSupportRoot = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_ICF), iImgICF,  m_htiConTweaks);
		m_htiICFSupport = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ICFSUPPORT), m_htiICFSupportRoot, m_bICFSupport);
		m_htiICFSupportClearAtEnd = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_FO_PREF_STARTUP), m_htiICFSupportRoot, m_bICFSupportClearAtEnd);
		m_htiICFSupportServerUDP = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ICF_SERVERUDP), m_htiICFSupportRoot, m_bICFSupportServerUDP);
		// <== Improved ICS-Firewall support [MoNKi] - Max
		// ==> UPnP support [MoNKi] - leuk_he
		m_htiUPnPNatGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_UPNP_GROUP), iImgUPnP,  m_htiConTweaks);
		m_htiUPnPNat = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_UPNP_ENABLE),m_htiUPnPNatGroup, m_bUPnPNat);
		m_htiUpnPNATwebservice = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_UPNP_ENABLE_WEB),m_htiUPnPNatGroup, m_bUpnPNATwebservice);
        m_htiUpnpBinaddr = m_ctrlTreeOptions.InsertItem(GetResString(IDS_UPNPBINDADDR), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiUPnPNatGroup);
		m_ctrlTreeOptions.AddIPAddress(m_htiUpnpBinaddr , RUNTIME_CLASS(CTreeOptionsIPAddressCtrl));
		m_htiUPnPForceUpdate= m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_UPNFORCEUPDATE), m_htiUPnPNatGroup, m_bUPnPForceUpdate);
		// <== UPnP support [MoNKi] - leuk_he
		// ==> Random Ports [MoNKi] - Stulle
		m_htiRndGrp = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_RND_PORT_GROUP), iImgRndGrp,  m_htiConTweaks);
		m_htiRandomports= m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ENABLE),m_htiRndGrp, m_bRandomports);
		m_htiRandomFirstPort = m_ctrlTreeOptions.InsertItem(GetResString(IDS_RND_PORT_FIRST), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiRndGrp);
		m_ctrlTreeOptions.AddEditBox(m_htiRandomFirstPort, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiRandomLastPort = m_ctrlTreeOptions.InsertItem(GetResString(IDS_RND_PORT_LAST), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiRndGrp);
		m_ctrlTreeOptions.AddEditBox(m_htiRandomLastPort , RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiRandomPortsResetTime = m_ctrlTreeOptions.InsertItem(GetResString(IDS_RND_PORT_RESET), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiRndGrp);
		m_ctrlTreeOptions.AddEditBox(m_htiRandomPortsResetTime, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// <== Random Ports [MoNKi] - Stulle
		// ==> Mephisto Upload - Mephisto
		m_htiMephistoUploadGrp = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_MEPHISTO_UPLOAD), iImgMephUlGrp,  m_htiConTweaks);
		m_htiMephistoWarning1 = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_MEPHISTO_WARNING_1), iImgWarning,  m_htiMephistoUploadGrp);
		m_htiMephistoWarning2 = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_MEPHISTO_WARNING_2), iImgWarning,  m_htiMephistoUploadGrp);
		m_htiMinSlots = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MIN_SLOTS), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiMephistoUploadGrp);
		m_ctrlTreeOptions.AddEditBox(m_htiMinSlots, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiNoNewSlotTimer = m_ctrlTreeOptions.InsertItem(GetResString(IDS_NO_NEW_SLOT_TIMER), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiMephistoUploadGrp);
		m_ctrlTreeOptions.AddEditBox(m_htiNoNewSlotTimer, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiFullLoops = m_ctrlTreeOptions.InsertItem(GetResString(IDS_FULL_LOOPS), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiMephistoUploadGrp);
		m_ctrlTreeOptions.AddEditBox(m_htiFullLoops, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiMonitorLoops = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MONITOR_LOOPS), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiMephistoUploadGrp);
		m_ctrlTreeOptions.AddEditBox(m_htiMonitorLoops, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiNotReachedBW = m_ctrlTreeOptions.InsertItem(GetResString(IDS_NOT_REACHED_BW), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiMephistoUploadGrp);
		m_ctrlTreeOptions.AddEditBox(m_htiNotReachedBW, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiNoTrickleTimer = m_ctrlTreeOptions.InsertItem(GetResString(IDS_NO_TRICKLE_TIMER), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiMephistoUploadGrp);
		m_ctrlTreeOptions.AddEditBox(m_htiNoTrickleTimer, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiMoveDownKB = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MOVE_DOWN_KB), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiMephistoUploadGrp);
		m_ctrlTreeOptions.AddEditBox(m_htiMoveDownKB, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// <== Mephisto Upload - Mephisto
		// ==> Multiple Part Transfer [Stulle] - Mephisto
		m_htiChunksGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_CHUNKS_GROUP), iImgChunks, m_htiConTweaks);
		m_htiChunksScore = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_CHUNKS_NONE), m_htiChunksGroup, m_iChunksMode == CHUNK_SCORE);
		m_htiChunksXman = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_CHUNKS_XMAN), m_htiChunksGroup, m_iChunksMode == CHUNK_XMAN);
		m_htiChunksFinish = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_CHUNKS_FINISH), m_htiChunksGroup, m_iChunksMode == CHUNK_FINISH);
		m_htiChunksToFinish = m_ctrlTreeOptions.InsertItem(GetResString(IDS_CHUNKS_TO_FINISH), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiChunksFinish);
		m_ctrlTreeOptions.AddEditBox(m_htiChunksToFinish , RUNTIME_CLASS(CNumTreeOptionsEdit));
		if(m_iChunksMode == CHUNK_FINISH) m_ctrlTreeOptions.Expand(m_htiChunksFinish, TVE_EXPAND);
		m_htiChunksFull = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_CHUNKS_FULL), m_htiChunksGroup, m_iChunksMode == CHUNK_FULL);
		m_htiChunksToUpload = m_ctrlTreeOptions.InsertItem(GetResString(IDS_CHUNKS_TO_UPLOAD), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiChunksFull);
		m_ctrlTreeOptions.AddEditBox(m_htiChunksToUpload , RUNTIME_CLASS(CNumTreeOptionsEdit));
		if(m_iChunksMode == CHUNK_FULL) m_ctrlTreeOptions.Expand(m_htiChunksFull, TVE_EXPAND);
		// <== Multiple Part Transfer [Stulle] - Mephisto
		// ==> Adjust max upload time [Stulle] - Mephisto
		m_htiMaxUpMinutes = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MAX_UP_MINUTES), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiChunksGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiMaxUpMinutes, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// <== Adjust max upload time [Stulle] - Mephisto
		// ==> Timer for ReAsk File Sources [Stulle] - Stulle
		m_htiReAskFileSrc = m_ctrlTreeOptions.InsertItem(GetResString(IDS_REASK_FILE_SRC), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiConTweaks);
		m_ctrlTreeOptions.AddEditBox(m_htiReAskFileSrc, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// <== Timer for ReAsk File Sources [Stulle] - Stulle
		m_htiACC = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ACC), m_htiConTweaks, m_bACC); // ACC [Max/WiZaRd] - Max
		m_htiIgnoreThird = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_IGNORE_THIRD), m_htiConTweaks, m_bIgnoreThird); // Do not reserve 1/3 of your uploadlimit for emule [Stulle] - Stulle
		m_htiUlThres = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_UL_THRES), m_htiConTweaks, m_bUlThres); // Disable accepting only clients who asked within last 30min [Stulle] - Stulle
		// ==> Mephisto Upload - Mephisto
		/*
		m_htiMaxSlotSpeed = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MAX_SLOTSPEED), m_htiConTweaks, m_bMaxSlotSpeed); // Alwasy maximize slot speed [Stulle] - Stulle
		*/
		// <== Mephisto Upload - Mephisto

		// ==> Anti Uploader Ban [Stulle] - Stulle
		m_htiAntiUploaderBanLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_UNBAN_UPLOADER), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htiAntiUploaderBanLimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiAntiCase1 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_ANTI_CASE_1), m_htiAntiUploaderBanLimit, m_iAntiUploaderBanCase == 0);
		m_htiAntiCase2 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_ANTI_CASE_2), m_htiAntiUploaderBanLimit, m_iAntiUploaderBanCase == 1);
		m_htiAntiCase3 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_ANTI_CASE_3), m_htiAntiUploaderBanLimit, m_iAntiUploaderBanCase == 2);
		// <== Anti Uploader Ban [Stulle] - Stulle

		// ==> CreditSystems [EastShare/ MorphXT] - Stulle
		m_htiCreditSystem = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_CREDIT_SYSTEM), iImgCS, TVI_ROOT);
		m_htiOfficialCredit = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_OFFICIAL_CREDIT), m_htiCreditSystem, m_iCreditSystem == CS_OFFICIAL);
		m_htiLovelaceCredit = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_LOVELACE_CREDIT), m_htiCreditSystem, m_iCreditSystem == CS_LOVELACE);
		m_htiRatioCredit = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_RATIO_CREDIT), m_htiCreditSystem, m_iCreditSystem == CS_RATIO);
		m_htiPawcioCredit = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_PAWCIO_CREDIT), m_htiCreditSystem, m_iCreditSystem == CS_PAWCIO);
		m_htiESCredit = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_EASTSHARE_CREDIT), m_htiCreditSystem, m_iCreditSystem == CS_EASTSHARE);
		m_htiSivkaCredit = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SIVKA_CREDIT), m_htiCreditSystem, m_iCreditSystem == CS_SIVKA);
		m_htiSwatCredit = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SWAT_CREDIT), m_htiCreditSystem, m_iCreditSystem == CS_SWAT);
		m_htiXmanCredit = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_XMAN_CREDIT), m_htiCreditSystem, m_iCreditSystem == CS_XMAN);
		m_htiTk4Credit = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_TK4_CREDIT), m_htiCreditSystem, m_iCreditSystem == CS_TK4);
		m_htiZzulCredit = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_ZZUL_CREDIT), m_htiCreditSystem, m_iCreditSystem == CS_ZZUL);
		// <== CreditSystems [EastShare/ MorphXT] - Stulle
		// ==> Modified FineCS [CiccioBastardo/Stulle] - Stulle
		m_htiFineCS = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_FINECS), m_htiCreditSystem, m_bFineCS);
		// <== Modified FineCS [CiccioBastardo/Stulle] - Stulle
		// ==> Pay Back First [AndCycle/SiRoB/Stulle] - Stulle
		m_htiIsPayBackFirst = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PAYBACKFIRST), m_htiCreditSystem, m_bIsPayBackFirst);
		m_htiPayBackFirstLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_PAYBACKFIRSTLIMIT),TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,m_htiIsPayBackFirst);
		m_ctrlTreeOptions.AddEditBox(m_htiPayBackFirstLimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_ctrlTreeOptions.Expand(m_htiIsPayBackFirst, TVE_EXPAND);
		m_htiIsPayBackFirst2 = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PAYBACKFIRST2), m_htiCreditSystem, m_bIsPayBackFirst2);
		m_htiPayBackFirstLimit2 = m_ctrlTreeOptions.InsertItem(GetResString(IDS_PAYBACKFIRSTLIMIT2),TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,m_htiIsPayBackFirst2);
		m_ctrlTreeOptions.AddEditBox(m_htiPayBackFirstLimit2, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_ctrlTreeOptions.Expand(m_htiIsPayBackFirst2, TVE_EXPAND);
		// <== Pay Back First [AndCycle/SiRoB/Stulle] - Stulle

		m_htiDisplay = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_PW_DISPLAY), iImgDisplay, TVI_ROOT);
		// ==> CPU/MEM usage [$ick$/Stulle] - Max
		m_htiSysInfoGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_SYS_INFO_GROUP), iImgSysInfo, m_htiDisplay);
		m_htiSysInfo = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ENABLE), m_htiSysInfoGroup, m_bSysInfo);
		m_htiSysInfoGlobal = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SYS_INFO_GLOBAL), m_htiSysInfoGroup, m_bSysInfoGlobal);
		// <== CPU/MEM usage [$ick$/Stulle] - Max
		// ==> Invisible Mode [TPT/MoNKi] - Stulle
		m_htiInvisibleModeRoot = m_ctrlTreeOptions.InsertItem(GetResString(IDS_INVMODE_GROUP), iImgInvisibleMode, iImgInvisibleMode, m_htiDisplay);
		m_htiInvisibleMode = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_INVMODE), m_htiInvisibleModeRoot, m_bInvisibleMode);
		m_htiInvisibleModeMod = m_ctrlTreeOptions.InsertItem(GetResString(IDS_INVMODE_MODKEY), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiInvisibleModeRoot);
		m_ctrlTreeOptions.AddComboBox(m_htiInvisibleModeMod, RUNTIME_CLASS(CTreeOptionsInvisibleModCombo));
		m_htiInvisibleModeKey = m_ctrlTreeOptions.InsertItem(GetResString(IDS_INVMODE_VKEY), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiInvisibleModeRoot);
		m_ctrlTreeOptions.AddComboBox(m_htiInvisibleModeKey, RUNTIME_CLASS(CTreeOptionsInvisibleKeyCombo));
		m_htiInvisibleModeStart = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_INVMODE_START), m_htiInvisibleModeRoot, m_bInvisibleModeStart);
		// <== Invisible Mode [TPT/MoNKi] - Stulle
		m_htiShowSrcOnTitle = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SHOWSRCONTITLE), m_htiDisplay, showSrcInTitle); // Show sources on title - Stulle
		m_htiShowGlobalHL = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SHOW_GLOBAL_HL), m_htiDisplay, m_bShowGlobalHL); // show global HL - Stulle
		m_htiShowFileHLconst = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SHOW_FILE_HL_CONST), m_htiDisplay, m_bShowFileHLconst); // show HL per file constantly - Stulle
		m_htiShowInMSN7 = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SHOWINMSN7), m_htiDisplay, m_bShowInMSN7); // Show in MSN7 [TPT] - Stulle
		m_htiQueueProgressBar = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_CLIENTQUEUEPROGRESSBAR), m_htiDisplay, m_bQueueProgressBar); // Client queue progress bar [Commander] - Stulle
		m_htiTrayComplete = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_TRAY_COMPLETE), m_htiDisplay, m_bTrayComplete); // Completed in Tray [Stulle] - Stulle
		m_htiColorFeedback = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_COLOR_FEEDBACK), m_htiDisplay, m_bColorFeedback); // Feedback personalization [Stulle] - Stulle
		m_htiShowClientPercentage = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_CLIENTPERCENTAGE), m_htiDisplay, m_bShowClientPercentage); // Show Client Percentage optional [Stulle] - Stulle
		m_htiFollowTheMajority = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_FOLLOWTHEMAJORITY), m_htiDisplay, m_bFollowTheMajority); // Follow The Majority [AndCycle/Stulle] - Stulle
		m_htiShowSpeedMeter = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SHOW_SPEED_METER), m_htiDisplay, m_bShowSpeedMeter); // High resolution speedmeter on toolbar [eFMod/Stulle] - Myth88
		// ==> Static Tray Icon [MorphXT] - MyTh88
		m_htiStaticIcon = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_STATIC_ICON),m_htiDisplay, m_bStaticIcon);
		// <== Static Tray Icon [MorphXT] - MyTh88

		// ==> File Settings [sivka/Stulle] - Stulle
		m_htiFileDefaults = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_FILE_DEFAULTS), iImgDropDefaults, TVI_ROOT);
		m_htiAutoNNS = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_AUTO_NNS), m_htiFileDefaults, m_bEnableAutoDropNNSDefault);
		m_htiAutoNNSTimer = m_ctrlTreeOptions.InsertItem(GetResString(IDS_NNS_TIMERLABEL), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiAutoNNS);
		m_ctrlTreeOptions.AddEditBox(m_htiAutoNNSTimer, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiAutoNNSLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_REMOVENNSLIMITLABEL), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiAutoNNS);
		m_ctrlTreeOptions.AddEditBox(m_htiAutoNNSLimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_ctrlTreeOptions.Expand(m_htiAutoNNS, TVE_EXPAND);
		m_htiAutoFQS = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_AUTO_FQS), m_htiFileDefaults, m_bEnableAutoDropFQSDefault);
		m_htiAutoFQSTimer = m_ctrlTreeOptions.InsertItem(GetResString(IDS_FQS_TIMERLABEL), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiAutoFQS);
		m_ctrlTreeOptions.AddEditBox(m_htiAutoFQSTimer, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiAutoFQSLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_REMOVEFQSLIMITLABEL), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiAutoFQS);
		m_ctrlTreeOptions.AddEditBox(m_htiAutoFQSLimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_ctrlTreeOptions.Expand(m_htiAutoFQS, TVE_EXPAND);
		m_htiAutoQRS = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_AUTO_QRS), m_htiFileDefaults, m_bEnableAutoDropQRSDefault);
		m_htiAutoQRSTimer = m_ctrlTreeOptions.InsertItem(GetResString(IDS_HQRS_TIMERLABEL), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiAutoQRS);
		m_ctrlTreeOptions.AddEditBox(m_htiAutoQRSTimer, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiAutoQRSLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_REMOVEQRSLIMITLABEL), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiAutoQRS);
		m_ctrlTreeOptions.AddEditBox(m_htiAutoQRSLimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiAutoQRSWay = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_HQR_WAY), iImgDropDefaults, m_htiAutoQRS);
		m_htiHQRSivka = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SIVKA_DROPPING), m_htiAutoQRSWay, m_iHQRXmanDefault == 0);
		m_htiAutoQRSMax = m_ctrlTreeOptions.InsertItem(GetResString(IDS_REMOVEQRSLABEL), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiHQRSivka);
		m_ctrlTreeOptions.AddEditBox(m_htiAutoQRSMax, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_ctrlTreeOptions.Expand(m_htiHQRSivka, TVE_EXPAND);
		m_htiHQRXman = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_XMAN_DROPPING), m_htiAutoQRSWay, m_iHQRXmanDefault == 1);
		m_ctrlTreeOptions.Expand(m_htiAutoQRSWay, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiAutoQRS, TVE_EXPAND);
		// <== File Settings [sivka/Stulle] - Stulle

		// ==> TBH: minimule - Max
		m_htiMMGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_MM_GROUP), iImgMinimule, TVI_ROOT);
		m_htiShowMM = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MM_SHOW), m_htiMMGroup, m_bShowMM);
		m_htiMMLives = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MM_LIVES), m_htiMMGroup, m_bMMLives);
		m_htiMMUpdateTime = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MM_UPDATE_TIME), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiMMGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiMMUpdateTime, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiMMTrans = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MM_TRANS), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiMMGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiMMTrans, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiMMCompl = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MM_COMPL), m_htiMMGroup, m_bMMCompl);
		m_htiMMOpen = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MM_OPEN), m_htiMMGroup, m_bMMOpen);
		// <== TBH: minimule - Max

		// ==> Control download priority [tommy_gun/iONiX] - MyTh88
		m_htiAutoDownPrioGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_AUTO_DOWN_GROUP), iImgAutoDownPrio, TVI_ROOT);
		m_htiAutoDownPrioOff = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_POWERSHARE_DISABLED), m_htiAutoDownPrioGroup, m_iAutoDownPrio == 0);
		m_htiAutoDownPrioPerc = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_AUTO_DOWN_PERC), m_htiAutoDownPrioGroup, m_iAutoDownPrio == 1);
		m_htiAutoDownPrioPercVal = m_ctrlTreeOptions.InsertItem(GetResString(IDS_AUTO_DOWN_PERC_VAL), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiAutoDownPrioPerc);
		m_ctrlTreeOptions.AddEditBox(m_htiAutoDownPrioPercVal, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_ctrlTreeOptions.Expand(m_htiAutoDownPrioPerc, TVE_EXPAND);
		m_htiAutoDownPrioSize = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_AUTO_DOWN_SIZE), m_htiAutoDownPrioGroup, m_iAutoDownPrio == 2);
		m_htiAutoDownPrioSizeVal = m_ctrlTreeOptions.InsertItem(GetResString(IDS_AUTO_DOWN_SIZE_VAL), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiAutoDownPrioSize);
		m_ctrlTreeOptions.AddEditBox(m_htiAutoDownPrioSizeVal, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_ctrlTreeOptions.Expand(m_htiAutoDownPrioSize, TVE_EXPAND);
		m_htiAutoDownPrioValGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_AUTO_DOWN_PRIO), iImgPriority, m_htiAutoDownPrioGroup);
		m_htiAutoDownPrioLow = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_PRIOLOW), m_htiAutoDownPrioValGroup, m_iAutoDownPrioVal == 0);
		m_htiAutoDownPrioNormal = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_PRIONORMAL), m_htiAutoDownPrioValGroup, m_iAutoDownPrioVal == 1);
		m_htiAutoDownPrioHigh = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_PRIOHIGH), m_htiAutoDownPrioValGroup, m_iAutoDownPrioVal == 2);
		m_ctrlTreeOptions.Expand(m_htiAutoDownPrioValGroup, TVE_EXPAND);
		// <== Control download priority [tommy_gun/iONiX] - MyTh88

		// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
		m_htiSCC = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_SCC), iImgSCC, TVI_ROOT);
		m_htiDlMode = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_DL_MODE_DEFAULT), iImgDlMode, m_htiSCC);
		m_htiDlNone = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_DL_NONE), m_htiDlMode, m_iDlMode == 0);
		m_htiDlAlph = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_DOWNLOAD_ALPHABETICAL), m_htiDlMode, m_iDlMode == 1);
		m_htiDlLP = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_LP), m_htiDlMode, m_iDlMode == 2);
		m_htiShowCatNames = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_CAT_SHOWCATNAME), m_htiSCC, m_bShowCatNames);
		m_htiSelectCat = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_CAT_SHOWSELCATDLG), m_htiSCC, m_bSelectCat);
		m_htiUseAutoCat = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_CAT_USEAUTOCAT), m_htiSCC, m_bUseAutoCat);
		m_htiUseActiveCat = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_CAT_USEACTIVE), m_htiSCC, m_bUseActiveCat);
		m_htiAutoSetResOrder = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_CAT_AUTORESUMEORD), m_htiSCC, m_bAutoSetResOrder);
		m_htiSmallFileDLPush = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_CAT_SMALLFILEDLPUSH), m_htiSCC, m_bSmallFileDLPush);
		m_htiResumeFileInNewCat = m_ctrlTreeOptions.InsertItem(GetResString(IDS_CAT_STARTFILESONADD), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiSCC);
		m_ctrlTreeOptions.AddEditBox(m_htiResumeFileInNewCat, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiAddRemovedInc = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ADD_REMOVED_INC), m_htiSCC, m_bAddRemovedInc);
		// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

		m_htiSharedPrefs = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_SHARED_PREFS), iImgSharedPrefs, TVI_ROOT); // Shared Files Management [Stulle] - Stulle
		// ==> PowerShare [ZZ/MorphXT] - Stulle
		m_htiPowershareMode = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_POWERSHARE), iImgPS, m_htiSharedPrefs);
		m_htiPowershareDisabled = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_POWERSHARE_DISABLED), m_htiPowershareMode, m_iPowershareMode == 0);
		m_htiPowershareActivated =  m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_POWERSHARE_ACTIVATED), m_htiPowershareMode, m_iPowershareMode == 1);
		m_htiPowershareAuto =  m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_POWERSHARE_AUTO), m_htiPowershareMode, m_iPowershareMode == 2);
		m_htiPowershareLimited =  m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_POWERSHARE_LIMITED), m_htiPowershareMode, m_iPowershareMode == 3);
		m_htiPowerShareLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_POWERSHARE_LIMIT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiPowershareLimited );
		m_ctrlTreeOptions.AddEditBox(m_htiPowerShareLimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// <== PowerShare [ZZ/MorphXT] - Stulle
		// ==> Limit PS by amount of data uploaded [Stulle] - Stulle
		m_htiPsAmountLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_PS_AMOUNT_LIMIT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiPowershareLimited );
		m_ctrlTreeOptions.AddEditBox(m_htiPsAmountLimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// <== Limit PS by amount of data uploaded [Stulle] - Stulle
		// ==> Spread Credits Slot [Stulle] - Stulle
		m_htiSpreadCreditsSlotGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_SPREAD_CREDITS_SLOT), iImgSpreadCredits, m_htiSharedPrefs);
		m_htiSpreadCreditsSlot = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ENABLE), m_htiSpreadCreditsSlotGroup, m_bSpreadCreditsSlot);
		m_htiSpreadCreditsSlotCounter = m_ctrlTreeOptions.InsertItem(GetResString(IDS_SPREAD_CREDITS_SLOT_COUNTER), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiSpreadCreditsSlotGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiSpreadCreditsSlotCounter, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// <== Spread Credits Slot [Stulle] - Stulle
		// ==> Release Bonus [sivka] - Stulle
		m_htiReleaseBonusGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_RELEASE_BONUS_GROUP), iImgReleaseBonus, m_htiSharedPrefs);
		m_htiReleaseBonus0 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_POWERSHARE_DISABLED), m_htiReleaseBonusGroup, m_iReleaseBonus == 0);
		m_htiReleaseBonus1 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_RELEASE_BONUS_12), m_htiReleaseBonusGroup, m_iReleaseBonus == 1);
		m_htiReleaseBonusDays = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_DAYS2), m_htiReleaseBonusGroup, m_iReleaseBonus == 2);
		m_htiReleaseBonusDaysEdit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_RELEASE_BONUS_EDIT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiReleaseBonusDays);
		m_ctrlTreeOptions.AddEditBox(m_htiReleaseBonusDaysEdit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_ctrlTreeOptions.Expand(m_htiReleaseBonusDays, TVE_EXPAND);
		// <== Release Bonus [sivka] - Stulle
		m_htiReleaseScoreAssurance = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_RELEASE_SCORE_ASSURANCE), m_htiReleaseBonusGroup, m_bReleaseScoreAssurance); // Release Score Assurance [Stulle] - Stulle
		m_htiSpreadBars = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SPREAD_BARS), m_htiSharedPrefs, m_bSpreadBars); // Spread bars [Slugfiller/MorphXT] - Stulle
		// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
		m_htiHideOS = m_ctrlTreeOptions.InsertItem(GetResString(IDS_HIDEOVERSHARES), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiSpreadBars);
		m_ctrlTreeOptions.AddEditBox(m_htiHideOS, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiSelectiveShare = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SELECTIVESHARE), m_htiHideOS, m_bSelectiveShare);
		m_ctrlTreeOptions.Expand(m_htiSpreadBars, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiHideOS, TVE_EXPAND);
		m_htiShareOnlyTheNeed = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SHAREONLYTHENEED), m_htiSharedPrefs, m_iShareOnlyTheNeed);
		// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
		// ==> Fair Play [AndCycle/Stulle] - Stulle
		m_htiFairPlay = m_ctrlTreeOptions.InsertItem(GetResString(IDS_FAIR_PLAY), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiSharedPrefs);
		m_ctrlTreeOptions.AddEditBox(m_htiFairPlay, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// <== Fair Play [AndCycle/Stulle] - Stulle

		m_htiMisc = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_MISC), iImgMisc, TVI_ROOT);
		// ==> Global Source Limit [Max/Stulle] - Stulle
		m_htiGlobalHlGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_GLOBAL_HL), iImgGlobal, m_htiMisc);
		m_htiGlobalHL = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ENABLE), m_htiGlobalHlGroup, m_bGlobalHL);
		m_htiGlobalHlLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_GLOBAL_HL_LIMIT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiGlobalHlGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiGlobalHlLimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiGlobalHlAll = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_GLOBAL_HL_ALL), m_htiGlobalHlGroup, m_bGlobalHlAll);
		m_htiGlobalHlDefault = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_GLOBAL_HL_DEFAULT), m_htiGlobalHlGroup, m_bGlobalHlDefault);
		m_ctrlTreeOptions.Expand(m_htiGlobalHlGroup, TVE_EXPAND);
		// <== Global Source Limit [Max/Stulle] - Stulle
		// ==> Emulate others [WiZaRd/Spike/shadow2004] - Stulle
		m_htiEmulatorGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_EMULATOR_GROUP), iImgEmulate, m_htiMisc);
		m_htiEmuMLDonkey = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_EMULATE_ML), m_htiEmulatorGroup, m_bEmuMLDonkey);
		m_htiEmueDonkey = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_EMULATE_DONK), m_htiEmulatorGroup, m_bEmueDonkey);
		m_htiEmueDonkeyHybrid = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_EMULATE_DONK_HYB), m_htiEmulatorGroup, m_bEmueDonkeyHybrid);
		m_htiEmuShareaza = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_EMULATE_SHA), m_htiEmulatorGroup, m_bEmuShareaza);
		m_htiEmuLphant = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_EMULATE_PHANT), m_htiEmulatorGroup, m_bEmuLphant);
		m_htiLogEmulator = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_EMULATE_LOG), m_htiEmulatorGroup, m_bLogEmulator);
		// <== Emulate others [WiZaRd/Spike/shadow2004] - Stulle
		// ==> Automatic shared files updater [MoNKi] - Stulle
		m_htiAutoSharedGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_AUTO_SHARED_UPDATER), iImgASFU, m_htiMisc);
		m_htiAutoSharedUpdater = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ENABLE), m_htiAutoSharedGroup, m_bAutoSharedUpdater);
		m_htiSingleSharedDirUpdater = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ASFU_SINGLE), m_htiAutoSharedGroup, m_bSingleSharedDirUpdater);
		m_htiTimeBetweenReloads = m_ctrlTreeOptions.InsertItem(GetResString(IDS_ASFU_TIMEBETWEEN), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiAutoSharedGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiTimeBetweenReloads, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// <== Automatic shared files updater [MoNKi] - Stulle
		m_htiSUQWT = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SUQWT), m_htiMisc, m_bSUQWT); // SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
		m_htiStartupSound = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_STARTUPSOUND), m_htiMisc, m_bStartupSound); // Startupsound [Commander] - mav744
		// ==> Adjust Compress Level [Stulle] - Stulle
		m_htiCompressLevel = m_ctrlTreeOptions.InsertItem(GetResString(IDS_COMPRESS_LVL), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiMisc);
		m_ctrlTreeOptions.AddEditBox(m_htiCompressLevel, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// <== Adjust Compress Level [Stulle] - Stulle

		m_ctrlTreeOptions.SendMessage(WM_VSCROLL, SB_TOP);
		m_bInitializedTreeOpts = true;
	}
	// ==> push small files [sivka] - Stulle
    DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiEnablePushSmallFile, m_bEnablePushSmallFile);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiPushSmallFileBoost, m_iPushSmallFileBoost);
	DDV_MinMaxInt(pDX, m_iPushSmallFileBoost, 1, 65536);
	// <== push small files [sivka] - Stulle
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiEnablePushRareFile, m_bEnablePushRareFile); // push rare file - Stulle

	// ==> FunnyNick [SiRoB/Stulle] - Stulle
	if(m_htiFnActive)	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiFnActive, m_bFnActive);
	if(m_htiFnTagMode)	DDX_TreeRadio(pDX, IDC_SCAR_OPTS, m_htiFnTagMode, (int &)m_iFnTag);
	if(m_htiFnCustomTag)DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiFnCustomTag, m_sFnCustomTag);
	if(m_htiFnTagAtEnd)	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiFnTagAtEnd, m_bFnTagAtEnd);
	// <== FunnyNick [SiRoB/Stulle] - Stulle
	
	// ==> Quick start [TPT] - Max
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiQuickStart, m_bQuickStart);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiQuickStartMaxTime, m_iQuickStartMaxTime);
	DDV_MinMaxInt(pDX, m_iQuickStartMaxTime, 8, 18);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiQuickStartMaxConnPerFive, m_iQuickStartMaxConnPerFive);
	DDV_MinMaxInt(pDX, m_iQuickStartMaxConnPerFive, 5, 200);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiQuickStartMaxConn, m_iQuickStartMaxConn);
	DDV_MinMaxInt(pDX, m_iQuickStartMaxConn, 200, 2000);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiQuickStartMaxConnPerFiveBack, m_iQuickStartMaxConnPerFiveBack);
	DDV_MinMaxInt(pDX, m_iQuickStartMaxConnPerFiveBack, 1, INT_MAX);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiQuickStartMaxConnBack, m_iQuickStartMaxConnBack);
	DDV_MinMaxInt(pDX, m_iQuickStartMaxConnBack, 1, INT_MAX);
	if(m_htiQuickStartAfterIPChange) DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiQuickStartAfterIPChange, m_bQuickStartAfterIPChange);
	// <== Quick start [TPT] - Max
	// ==> Enforce Ratio [Stulle] - Stulle
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiEnforceRatio, m_bEnforceRatio);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiRatioValue, m_iRatioValue);
	DDV_MinMaxInt(pDX, m_iRatioValue, 1, 10);
	// <== Enforce Ratio [Stulle] - Stulle
	// ==> Improved ICS-Firewall support [MoNKi] - Max
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiICFSupport, m_bICFSupport);
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiICFSupportClearAtEnd, m_bICFSupportClearAtEnd);
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiICFSupportServerUDP, m_bICFSupportServerUDP);
	// <== Improved ICS-Firewall support [MoNKi] - Max
	// ==> UPnP support [MoNKi] - leuk_he 
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS,m_htiUPnPNat,m_bUPnPNat  )	;
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS,m_htiUpnPNATwebservice,  m_bUpnPNATwebservice);
	DDX_TreeIPAddress(pDX, IDC_SCAR_OPTS,m_htiUpnpBinaddr  , m_dwUpnpBindAddr);
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiUPnPForceUpdate, m_bUPnPForceUpdate);
	// <== UPnP support [MoNKi] - leuk_he
	// ==> Random Ports [MoNKi] - Stulle
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS,m_htiRandomports,m_bRandomports);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS,m_htiRandomFirstPort,  m_iRandomFirstPort);
	DDV_MinMaxInt(pDX, m_iRandomFirstPort, 1, USHRT_MAX);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS,m_htiRandomLastPort,  m_iRandomLastPort);
	DDV_MinMaxInt(pDX, m_iRandomLastPort, m_iRandomFirstPort, USHRT_MAX);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiRandomPortsResetTime, m_iRandomPortsResetTime);
	DDV_MinMaxInt(pDX, m_iRandomPortsResetTime, 0, 900);
	// <== Random Ports [MoNKi] - Stulle
	// ==> Mephisto Upload - Mephisto
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiMinSlots, m_iMinSlots);
	DDV_MinMaxInt(pDX, m_iMinSlots, 2, 10);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiNoNewSlotTimer, m_iNoNewSlotTimer);
	DDV_MinMaxInt(pDX, m_iNoNewSlotTimer, 1, 120);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiFullLoops, m_iFullLoops);
	DDV_MinMaxInt(pDX, m_iFullLoops, 1, 100);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiMonitorLoops, m_iMonitorLoops);
	DDV_MinMaxInt(pDX, m_iMonitorLoops, 1, 10);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiNotReachedBW, m_iNotReachedBW);
	DDV_MinMaxInt(pDX, m_iNotReachedBW, 1, 10);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiNoTrickleTimer, m_iNoTrickleTimer);
	DDV_MinMaxInt(pDX, m_iNoTrickleTimer, 0, 120);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiMoveDownKB, m_iMoveDownKB);
	DDV_MinMaxInt(pDX, m_iMoveDownKB, 1, 180);
	// <== Mephisto Upload - Mephisto
	// ==> Multiple Part Transfer [Stulle] - Mephisto
	DDX_TreeRadio(pDX, IDC_SCAR_OPTS, m_htiChunksGroup, (int &)m_iChunksMode);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiChunksToFinish, m_iChunksToFinish);
	DDV_MinMaxInt(pDX, m_iChunksToFinish, 2, 255);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiChunksToUpload, m_iChunksToUpload);
	DDV_MinMaxInt(pDX, m_iChunksToUpload, 2, 255);
	// <== Multiple Part Transfer [Stulle] - Mephisto
	// ==> Adjust max upload time [Stulle] - Mephisto
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiMaxUpMinutes, m_iMaxUpMinutes);
	DDV_MinMaxInt(pDX, m_iMaxUpMinutes, 60, 300);
	// <== Adjust max upload time [Stulle] - Mephisto
	// ==> Timer for ReAsk File Sources [Stulle] - Stulle
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiReAskFileSrc, m_iReAskFileSrc);
	DDV_MinMaxInt(pDX, m_iReAskFileSrc, 29, 55);
	// <== Timer for ReAsk File Sources [Stulle] - Stulle
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiACC, m_bACC); // ACC [Max/WiZaRd] - Max
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiIgnoreThird, m_bIgnoreThird); // Do not reserve 1/3 of your uploadlimit for emule [Stulle] - Stulle
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiUlThres, m_bUlThres); // Disable accepting only clients who asked within last 30min [Stulle] - Stulle
	// ==> Mephisto Upload - Mephisto
	/*
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiMaxSlotSpeed, m_bMaxSlotSpeed); // Alwasy maximize slot speed [Stulle] - Stulle
	*/
	// <== Mephisto Upload - Mephisto

	// ==> Anti Uploader Ban [Stulle] - Stulle
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiAntiUploaderBanLimit, m_iAntiUploaderBanLimit);
	DDV_MinMaxInt(pDX, m_iAntiUploaderBanLimit, 0, 20);
	DDX_TreeRadio(pDX, IDC_SCAR_OPTS, m_htiAntiUploaderBanLimit, (int &)m_iAntiUploaderBanCase);
	// <== Anti Uploader Ban [Stulle] - Stulle

	DDX_TreeRadio(pDX, IDC_SCAR_OPTS, m_htiCreditSystem, (int &)m_iCreditSystem); // CreditSystems [EastShare/ MorphXT] - Stulle
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiFineCS, m_bFineCS); // Modified FineCS [CiccioBastardo/Stulle] - Stulle
	// ==> Pay Back First [AndCycle/SiRoB/Stulle] - Stulle
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiIsPayBackFirst, m_bIsPayBackFirst);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiPayBackFirstLimit, m_iPayBackFirstLimit);
	DDV_MinMaxInt(pDX, m_iPayBackFirstLimit, 5, 255);
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiIsPayBackFirst2, m_bIsPayBackFirst2);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiPayBackFirstLimit2, m_iPayBackFirstLimit2);
	DDV_MinMaxInt(pDX, m_iPayBackFirstLimit2, 5, 1024);
	// <== Pay Back First [AndCycle/SiRoB/Stulle] - Stulle

	// ==> CPU/MEM usage [$ick$/Stulle] - Max
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiSysInfo, m_bSysInfo);
	if(m_htiSysInfoGlobal) DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiSysInfoGlobal, m_bSysInfoGlobal);
	// <== CPU/MEM usage [$ick$/Stulle] - Max
	// ==> Invisible Mode [TPT/MoNKi] - Stulle
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiInvisibleMode, m_bInvisibleMode);
	DDX_TreeCombo(pDX, IDC_SCAR_OPTS, m_htiInvisibleModeMod, m_sInvisibleModeMod);
	DDX_TreeCombo(pDX, IDC_SCAR_OPTS, m_htiInvisibleModeKey, m_sInvisibleModeKey);
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiInvisibleModeStart, m_bInvisibleModeStart);
	// <== Invisible Mode [TPT/MoNKi] - Stulle
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiShowSrcOnTitle, showSrcInTitle); // Show sources on title - Stulle
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiShowGlobalHL, m_bShowGlobalHL); // show global HL - Stulle
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiShowFileHLconst, m_bShowFileHLconst); // show HL per file constantly - Stulle
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiShowInMSN7, m_bShowInMSN7); // Show in MSN7 [TPT] - Stulle
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiQueueProgressBar, m_bQueueProgressBar); // Client queue progress bar [Commander] - Stulle
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiTrayComplete, m_bTrayComplete); // Completed in Tray [Stulle] - Stulle
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiColorFeedback, m_bColorFeedback); // Feedback personalization [Stulle] - Stulle
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiShowClientPercentage, m_bShowClientPercentage); // Show Client Percentage optional [Stulle] - Stulle
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiFollowTheMajority, m_bFollowTheMajority); // Follow The Majority [AndCycle/Stulle] - Stulle
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiShowSpeedMeter, m_bShowSpeedMeter); // High resolution speedmeter on toolbar [eFMod/Stulle] - Myth88
	// ==> Static Tray Icon [MorphXT] - MyTh88
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiStaticIcon, m_bStaticIcon);
	// <== Static Tray Icon [MorphXT] - MyTh88

	// ==> File Settings [sivka/Stulle] - Stulle
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiAutoNNS, m_bEnableAutoDropNNSDefault);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiAutoNNSTimer, m_iAutoNNS_TimerDefault);
	DDV_MinMaxInt(pDX, m_iAutoNNS_TimerDefault, 0, 60);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiAutoNNSLimit, m_iMaxRemoveNNSLimitDefault);
	DDV_MinMaxInt(pDX, m_iMaxRemoveNNSLimitDefault, 50, 100);
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiAutoFQS, m_bEnableAutoDropFQSDefault);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiAutoFQSTimer, m_iAutoFQS_TimerDefault);
	DDV_MinMaxInt(pDX, m_iAutoFQS_TimerDefault, 0, 60);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiAutoFQSLimit, m_iMaxRemoveFQSLimitDefault);
	DDV_MinMaxInt(pDX, m_iMaxRemoveFQSLimitDefault, 50, 100);
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiAutoQRS, m_bEnableAutoDropQRSDefault);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiAutoQRSTimer, m_iAutoHQRS_TimerDefault);
	DDV_MinMaxInt(pDX, m_iAutoHQRS_TimerDefault, 0, 120);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiAutoQRSMax, m_iMaxRemoveQRSDefault);
	DDV_MinMaxInt(pDX, m_iMaxRemoveQRSDefault, 2500, 10000);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiAutoQRSLimit, m_iMaxRemoveQRSLimitDefault);
	DDV_MinMaxInt(pDX, m_iMaxRemoveQRSLimitDefault, 50, 100);
	DDX_TreeRadio(pDX, IDC_SCAR_OPTS, m_htiAutoQRSWay, (int &)m_iHQRXmanDefault);
	// <== File Settings [sivka/Stulle] - Stulle

	// ==> TBH: minimule - Max
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiShowMM, m_bShowMM);
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiMMLives, m_bMMLives);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiMMUpdateTime, m_iMMUpdateTime);
	DDV_MinMaxInt(pDX, m_iMMUpdateTime, 0, MIN2S(60));
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiMMTrans, m_iMMTrans);
	DDV_MinMaxInt(pDX, m_iMMTrans, 1, 255);
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiMMCompl, m_bMMCompl);
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiMMOpen, m_bMMOpen);
	// <== TBH: minimule - Max

	// ==> Control download priority [tommy_gun/iONiX] - MyTh88
	DDX_TreeRadio(pDX, IDC_SCAR_OPTS, m_htiAutoDownPrioGroup, m_iAutoDownPrio);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiAutoDownPrioPercVal, m_iAutoDownPrioPerc);
	DDV_MinMaxInt(pDX, m_iAutoDownPrioPerc, 1, 100);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiAutoDownPrioSizeVal, m_iAutoDownPrioSize);
	DDV_MinMaxInt(pDX, m_iAutoDownPrioSize, 1, 1024);
	DDX_TreeRadio(pDX, IDC_SCAR_OPTS, m_htiAutoDownPrioValGroup, m_iAutoDownPrioVal);
	// <== Control download priority [tommy_gun/iONiX] - MyTh88

	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	DDX_TreeRadio(pDX, IDC_SCAR_OPTS, m_htiDlMode, (int &)m_iDlMode);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiResumeFileInNewCat, m_iResumeFileInNewCat);
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiShowCatNames, m_bShowCatNames);
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiSelectCat, m_bSelectCat);
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiUseActiveCat, m_bUseActiveCat);
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiAutoSetResOrder, m_bAutoSetResOrder);
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiSmallFileDLPush, m_bSmallFileDLPush);
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiUseAutoCat, m_bUseAutoCat);
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiAddRemovedInc, m_bAddRemovedInc);
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

	// ==> PowerShare [ZZ/MorphXT] - Stulle
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiPowerShareLimit, m_iPowerShareLimit);
	DDV_MinMaxInt(pDX, m_iShareOnlyTheNeed, 0, INT_MAX);
	DDX_TreeRadio(pDX, IDC_SCAR_OPTS, m_htiPowershareMode, m_iPowershareMode);
	// <== PowerShare [ZZ/MorphXT] - Stulle
	// ==> Limit PS by amount of data uploaded [Stulle] - Stulle
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiPsAmountLimit, m_iPsAmountLimit);
	DDV_MinMaxInt(pDX, m_iPsAmountLimit, 0, MAX_PS_AMOUNT_LIMIT);
	// <== Limit PS by amount of data uploaded [Stulle] - Stulle
	// ==> Spread Credits Slot [Stulle] - Stulle
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiSpreadCreditsSlot, m_bSpreadCreditsSlot);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiSpreadCreditsSlotCounter, m_iSpreadCreditsSlotCounter);
	DDV_MinMaxInt(pDX, m_iSpreadCreditsSlotCounter, 3, 20);
	// <== Spread Credits Slot [Stulle] - Stulle
	// ==> Release Bonus [sivka] - Stulle
	DDX_TreeRadio(pDX, IDC_SCAR_OPTS, m_htiReleaseBonusGroup, m_iReleaseBonus);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiReleaseBonusDaysEdit, m_iReleaseBonusDays);
	DDV_MinMaxInt(pDX, m_iReleaseBonusDays, 1, 16);
	// <== Release Bonus [sivka] - Stulle
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiReleaseScoreAssurance, m_bReleaseScoreAssurance); // Release Score Assurance [Stulle] - Stulle
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiSpreadBars, m_bSpreadBars); // Spread bars [Slugfiller/MorphXT] - Stulle
	// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiHideOS, m_iHideOS);
	DDV_MinMaxInt(pDX, m_iHideOS, 0, INT_MAX);
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiSelectiveShare, m_bSelectiveShare);
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiShareOnlyTheNeed, m_iShareOnlyTheNeed);
	// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	// ==> Fair Play [AndCycle/Stulle] - Stulle
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiFairPlay, m_iFairPlay);
	DDV_MinMaxInt(pDX, m_iFairPlay, 0, 10);
	// <== Fair Play [AndCycle/Stulle] - Stulle

	// ==> Global Source Limit [Max/Stulle] - Stulle
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiGlobalHL, m_bGlobalHL);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiGlobalHlLimit, m_iGlobalHL);
	DDV_MinMaxInt(pDX, m_iGlobalHL, 1000, MAX_GSL);
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiGlobalHlAll, m_bGlobalHlAll);
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiGlobalHlDefault, m_bGlobalHlDefault);
	// <== Global Source Limit [Max/Stulle] - Stulle
	// ==> Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiEmuMLDonkey, m_bEmuMLDonkey);
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiEmueDonkey, m_bEmueDonkey);
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiEmueDonkeyHybrid, m_bEmueDonkeyHybrid);
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiEmuShareaza, m_bEmuShareaza);
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiEmuLphant, m_bEmuLphant);
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiLogEmulator, m_bLogEmulator);
	// <== Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	// ==> Automatic shared files updater [MoNKi] - Stulle
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiAutoSharedUpdater, m_bAutoSharedUpdater);
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiSingleSharedDirUpdater, m_bSingleSharedDirUpdater);
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiTimeBetweenReloads, m_iTimeBetweenReloads);
	DDV_MinMaxInt(pDX, m_iTimeBetweenReloads, 0, 1800);
	// <== Automatic shared files updater [MoNKi] - Stulle
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiSUQWT, m_bSUQWT); // SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
	DDX_TreeCheck(pDX, IDC_SCAR_OPTS, m_htiStartupSound, m_bStartupSound); // Startupsound [Commander] - mav744
	// ==> Adjust Compress Level [Stulle] - Stulle
	DDX_TreeEdit(pDX, IDC_SCAR_OPTS, m_htiCompressLevel, m_iCompressLevel);
	DDV_MinMaxInt(pDX, m_iCompressLevel, 0, 9);
	// <== Adjust Compress Level [Stulle] - Stulle

	// ==> FunnyNick [SiRoB/Stulle] - Stulle
	if(m_htiFnTagMode)	m_ctrlTreeOptions.SetGroupEnable(m_htiFnTagMode, m_bFnActive);
	if(m_htiFnTagAtEnd)	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiFnTagAtEnd, m_bFnActive);
	if(m_bFnActive) m_ctrlTreeOptions.Expand(m_htiCustomTag, TVE_EXPAND);
	else m_ctrlTreeOptions.Expand(m_htiCustomTag, TVE_COLLAPSE);
	// <== FunnyNick [SiRoB/Stulle] - Stulle
	// ==> Quick start [TPT] - Max
	if (m_htiQuickStartAfterIPChange)	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiQuickStartAfterIPChange, m_bQuickStart);
	// <== Quick start [TPT] - Max
	// ==> Multiple Part Transfer [Stulle] - Mephisto
	if(m_iChunksMode == CHUNK_FINISH) m_ctrlTreeOptions.Expand(m_htiChunksFinish, TVE_EXPAND);
	else m_ctrlTreeOptions.Expand(m_htiChunksFinish, TVE_COLLAPSE);
	if(m_iChunksMode == CHUNK_FULL) m_ctrlTreeOptions.Expand(m_htiChunksFull, TVE_EXPAND);
	else m_ctrlTreeOptions.Expand(m_htiChunksFull, TVE_COLLAPSE);
	// <== Multiple Part Transfer [Stulle] - Mephisto
	// ==> CPU/MEM usage [$ick$/Stulle] - Max
	if (m_htiSysInfoGlobal)	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiSysInfoGlobal, m_bSysInfo);
	// <== CPU/MEM usage [$ick$/Stulle] - Max
	// ==> Automatic shared files updater [MoNKi] - Stulle
	if (m_htiSingleSharedDirUpdater)	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiSingleSharedDirUpdater, m_bAutoSharedUpdater);
	// <== Automatic shared files updater [MoNKi] - Stulle

	// ==> Advanced Options [Official/MorphXT] - Stulle
	DDX_Control(pDX, IDC_ADVANCED_OPTS, m_ctrlAdvTreeOptions);
	if (!m_bInitializedAdvTreeOpts)
	{
		int iImgAdvMinimule = 8;
		int iImgMediaInfo = 8;
		int iImgAdvDisplay = 8;
		int iImgLog = 8;

		CImageList* piml = m_ctrlAdvTreeOptions.GetImageList(TVSIL_NORMAL);
		if (piml){
			iImgAdvMinimule = piml->Add(CTempIconLoader(_T("MINIMULE")));
            iImgMediaInfo =	piml->Add(CTempIconLoader(_T("MediaInfo")));
			iImgAdvDisplay = piml->Add(CTempIconLoader(_T("DISPLAY")));
			iImgLog = piml->Add(CTempIconLoader(_T("Log")));
		}

		m_hti_AdvMiniMule = m_ctrlAdvTreeOptions.InsertGroup(GetResString(IDS_MINIMULE), iImgAdvMinimule, TVI_ROOT);
		m_hti_bMiniMuleAutoClose=m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_MINIMULEAUTOCLOSE),m_hti_AdvMiniMule,bMiniMuleAutoClose);
		m_hti_iMiniMuleTransparency= m_ctrlAdvTreeOptions.InsertItem(GetResString(IDS_MINIMULETRANSPARENCY),TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,m_hti_AdvMiniMule);
		m_ctrlAdvTreeOptions.AddEditBox(m_hti_iMiniMuleTransparency, RUNTIME_CLASS(CNumTreeOptionsEdit));

		m_hti_MediaInfo = m_ctrlAdvTreeOptions.InsertGroup(GetResString(IDS_FILEINFO), iImgMediaInfo, TVI_ROOT);
		m_hti_sMediaInfo_MediaInfoDllPath= m_ctrlAdvTreeOptions.InsertItem(GetResString(IDS_MEDIAINFO_MEDIAINFODLLPATH), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_hti_MediaInfo);
		m_ctrlAdvTreeOptions.AddFileEditBox(m_hti_sMediaInfo_MediaInfoDllPath,RUNTIME_CLASS(CTreeOptionsEdit), RUNTIME_CLASS(CTreeOptionsBrowseButton));
		m_hti_bMediaInfo_RIFF=m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_MEDIAINFO_RIFF),m_hti_MediaInfo,bMediaInfo_RIFF);
		m_hti_bMediaInfo_ID3LIB=m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_MEDIAINFO_ID3LIB),m_hti_MediaInfo,bMediaInfo_ID3LIB);
#ifdef HAVE_QEDIT_H
		m_hti_MediaInfo_MediaDet=m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_MEDIAINFO_MEDIADET),m_hti_MediaInfo,m_bMediaInfo_MediaDet);
#endif//HAVE_QEDIT_H
		m_hti_MediaInfo_RM=m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_MEDIAINFO_RM),m_hti_MediaInfo,m_bMediaInfo_RM);
#ifdef HAVE_WMSDK_H
		m_hti_MediaInfo_WM=m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_MEDIAINFO_WM),m_hti_MediaInfo,m_bMediaInfo_WM);
#endif//HAVE_WMSDK_H

		m_hti_AdvDisplay = m_ctrlAdvTreeOptions.InsertGroup(GetResString(IDS_PW_DISPLAY), iImgAdvDisplay, TVI_ROOT);
		m_hti_m_bRestoreLastMainWndDlg=m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_RESTORELASTMAINWNDDLG),m_hti_AdvDisplay,m_bRestoreLastMainWndDlg);
		m_hti_m_bRestoreLastLogPane=m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_RESTORELASTLOGPANE),m_hti_AdvDisplay,m_bRestoreLastLogPane);
		m_hti_m_iStraightWindowStyles=m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_STRAIGHTWINDOWSTYLES),m_hti_AdvDisplay,m_iStraightWindowStyles);
		m_hti_m_bRTLWindowsLayout=m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_RTLWINDOWSLAYOUT),m_hti_AdvDisplay,m_bRTLWindowsLayout);
		m_hti_m_iMaxChatHistory= m_ctrlAdvTreeOptions.InsertItem(GetResString(IDS_MAXCHATHISTORY),TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,m_hti_AdvDisplay);
		m_ctrlAdvTreeOptions.AddEditBox(m_hti_m_iMaxChatHistory, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_hti_maxmsgsessions=m_ctrlAdvTreeOptions.InsertItem(GetResString(IDS_MAXMSGSESSIONS),TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,m_hti_AdvDisplay);
		m_ctrlAdvTreeOptions.AddEditBox(m_hti_maxmsgsessions, RUNTIME_CLASS(CNumTreeOptionsEdit));													   
		m_htidatetimeformat = m_ctrlAdvTreeOptions.InsertItem(GetResString(IDS_X_DATETIMEFORMAT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_hti_AdvDisplay);
		m_ctrlAdvTreeOptions.AddEditBox(m_htidatetimeformat, RUNTIME_CLASS(CTreeOptionsEdit));
		m_htidatetimeformat4lists = m_ctrlAdvTreeOptions.InsertItem(GetResString(IDS_X_DATETIMEFORMAT4LIST), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_hti_AdvDisplay);
		m_ctrlAdvTreeOptions.AddEditBox(m_htidatetimeformat4lists, RUNTIME_CLASS(CTreeOptionsEdit));
		m_htiShowVerticalHourMarkers = m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_X_SHOWVERTICALHOURMARKERS), m_hti_AdvDisplay, m_bShowVerticalHourMarkers);
		m_htiReBarToolbar = m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_X_REBARTOOLBAR), m_hti_AdvDisplay, m_bReBarToolbar);		
		m_htiIconflashOnNewMessage = m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_X_ICON_FLASH_ON_NEW_MESSAGE), m_hti_AdvDisplay, m_bIconflashOnNewMessage);
		m_htiShowCopyEd2kLinkCmd = m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_X_SHOWCOPYED2KLINK), m_hti_AdvDisplay, m_bShowCopyEd2kLinkCmd);
		m_htiUpdateQueue = m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_UPDATEQUEUE), m_hti_AdvDisplay, m_bUpdateQueue);
		m_htiRepaint = m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_REPAINTGRAPHS), m_hti_AdvDisplay, m_bRepaint);
		m_htiExtraPreviewWithMenu = m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_EXTRAPREVIEWWITHMENU), m_hti_AdvDisplay, m_bExtraPreviewWithMenu);
		m_htiShowUpDownIconInTaskbar = m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_SHOWUPDOWNICONINTASKBAR), m_hti_AdvDisplay, m_bShowUpDownIconInTaskbar);
		m_htiForceSpeedsToKB = m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_FORCESPEEDSTOKB), m_hti_AdvDisplay, m_bForceSpeedsToKB);

		m_hti_Log = m_ctrlAdvTreeOptions.InsertGroup(GetResString(IDS_SV_LOG), iImgLog, TVI_ROOT);
		m_hti_m_iLogFileFormat=m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_LOGFILEFORMAT),m_hti_Log,m_iLogFileFormat);
		m_htidatetimeformat4log = m_ctrlAdvTreeOptions.InsertItem(GetResString(IDS_X_DATETIMEFORMAT4LOG), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_hti_Log);
		m_ctrlAdvTreeOptions.AddEditBox(m_htidatetimeformat4log, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_hti_iMaxLogBuff= m_ctrlAdvTreeOptions.InsertItem(GetResString(IDS_MAXLOGBUFF),TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,m_hti_Log);
		m_ctrlAdvTreeOptions.AddEditBox(m_hti_iMaxLogBuff, RUNTIME_CLASS(CTreeOptionsEdit));
		m_htiLogError = m_ctrlAdvTreeOptions.InsertItem(GetResString(IDS_X_LOGERROR), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_hti_Log);
		m_ctrlAdvTreeOptions.AddColorSelector(m_htiLogError, RUNTIME_CLASS(CTreeOptionsBrowseButton));
		m_htiLogWarning = m_ctrlAdvTreeOptions.InsertItem(GetResString(IDS_X_LOGWARNING), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_hti_Log);
		m_ctrlAdvTreeOptions.AddColorSelector(m_htiLogWarning, RUNTIME_CLASS(CTreeOptionsBrowseButton));
		m_htiLogSuccess = m_ctrlAdvTreeOptions.InsertItem(GetResString(IDS_X_LOGSUCCESS), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_hti_Log);
		m_ctrlAdvTreeOptions.AddColorSelector(m_htiLogSuccess, RUNTIME_CLASS(CTreeOptionsBrowseButton));

		m_hti_bCheckComctl32 =m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_CHECKCOMCTL32 ),TVI_ROOT,bCheckComctl32 );
		m_hti_bCheckShell32=m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_CHECKSHELL32),TVI_ROOT,bCheckShell32);
		m_hti_bIgnoreInstances=m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_IGNOREINSTANCES),TVI_ROOT,bIgnoreInstances);
		m_hti_sNotifierMailEncryptCertName= m_ctrlAdvTreeOptions.InsertItem(GetResString(IDS_NOTIFIERMAILENCRYPTCERTNAME), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlAdvTreeOptions.AddEditBox(m_hti_sNotifierMailEncryptCertName, RUNTIME_CLASS(CTreeOptionsEditEx));
		m_hti_m_iPreviewSmallBlocks=m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_PREVIEWSMALLBLOCKS),TVI_ROOT,m_iPreviewSmallBlocks);
		m_hti_m_bPreviewCopiedArchives=m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_PREVIEWCOPIEDARCHIVES),TVI_ROOT,m_bPreviewCopiedArchives);
		m_hti_m_bPreviewOnIconDblClk=m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_PREVIEWONICONDBLCLK),TVI_ROOT,m_bPreviewOnIconDblClk);
		m_hti_sInternetSecurityZone= m_ctrlAdvTreeOptions.InsertItem(GetResString(IDS_INTERNETSECURITYZONE), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlAdvTreeOptions.AddEditBox(m_hti_sInternetSecurityZone, RUNTIME_CLASS(CTreeOptionsEditEx));
		m_hti_sTxtEditor= m_ctrlAdvTreeOptions.InsertItem(GetResString(IDS_TXTEDITOR), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlAdvTreeOptions.AddFileEditBox(m_hti_sTxtEditor,RUNTIME_CLASS(CTreeOptionsEdit), RUNTIME_CLASS(CTreeOptionsBrowseButton));
		m_hti_iServerUDPPort= m_ctrlAdvTreeOptions.InsertItem(GetResString(IDS_SERVERUDPPORT),TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,TVI_ROOT);
		m_ctrlAdvTreeOptions.AddEditBox(m_hti_iServerUDPPort, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_hti_m_bRemoveFilesToBin=m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_REMOVEFILESTOBIN),TVI_ROOT,m_bRemoveFilesToBin);
		m_hti_HighresTimer=m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_HIGHRESTIMER),TVI_ROOT,m_bHighresTimer);
		m_hti_TrustEveryHash=m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_TRUSTEVERYHASH),TVI_ROOT,m_bTrustEveryHash);
		m_hti_InspectAllFileTypes=m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_INSPECTALLFILETYPES),TVI_ROOT,m_iInspectAllFileTypes);
		m_hti_PreferRestrictedOverUser=m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_PREFERRESTRICTEDOVERUSER),TVI_ROOT,m_bPreferRestrictedOverUser);
		m_hti_UseUserSortedServerList=m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_USEUSERSORTEDSERVERLIST),TVI_ROOT,m_bUseUserSortedServerList);
		m_hti_WebFileUploadSizeLimitMB=m_ctrlAdvTreeOptions.InsertItem(GetResString(IDS_WEBFILEUPLOADSIZELIMITMB),TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,TVI_ROOT);
		m_ctrlAdvTreeOptions.AddEditBox(m_hti_WebFileUploadSizeLimitMB, RUNTIME_CLASS(CNumTreeOptionsEdit));													   										   
		m_hti_AllowedIPs=m_ctrlAdvTreeOptions.InsertItem(GetResString(IDS_ALLOWEDIPS),TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,TVI_ROOT);
		m_ctrlAdvTreeOptions.AddEditBox(m_hti_AllowedIPs, RUNTIME_CLASS(CTreeOptionsEditEx));
		m_hti_DebugSearchResultDetailLevel=m_ctrlAdvTreeOptions.InsertItem(GetResString(IDS_DEBUGSEARCHDETAILLEVEL),TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,TVI_ROOT);
		m_ctrlAdvTreeOptions.AddEditBox(m_hti_DebugSearchResultDetailLevel, RUNTIME_CLASS(CTreeOptionsEditEx));
		m_htiAdjustNTFSDaylightFileTime = m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_X_ADJUSTNTFSDAYLIGHTFILETIME), TVI_ROOT, m_bAdjustNTFSDaylightFileTime);
		m_htidontcompressavi = m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_X_DONTCOMPRESSAVI), TVI_ROOT, m_dontcompressavi);
		m_htiICH = m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_X_ICH), TVI_ROOT, m_ICH);
		m_htiBindAddr = m_ctrlAdvTreeOptions.InsertItem(GetResString(IDS_BINDADDR), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlAdvTreeOptions.AddIPAddress(m_htiBindAddr , RUNTIME_CLASS(CTreeOptionsIPAddressCtrl));
		m_htiFileBufferTimeLimit = m_ctrlAdvTreeOptions.InsertItem(GetResString(IDS_X_FILE_BUFFER_TIME_LIMIT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlAdvTreeOptions.AddEditBox(m_htiFileBufferTimeLimit, RUNTIME_CLASS(CTreeOptionsEditEx));
		m_htiRearrangeKadSearchKeywords = m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_X_REARRANGEKADSEARCH), TVI_ROOT, m_bRearrangeKadSearchKeywords);
		m_htiBeeper = m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_PW_BEEP), TVI_ROOT, m_bBeeper);
		m_htiMsgOnlySec = m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_MSGONLYSEC), TVI_ROOT, m_bMsgOnlySec);
		m_htiDisablePeerCache = m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_DISABLEPEERACHE), TVI_ROOT, m_bDisablePeerCache);
		m_htiKeepUnavailableFixedSharedDirs = m_ctrlAdvTreeOptions.InsertCheckBox(GetResString(IDS_KEEPUNAVAILABLEFIXEDSHAREDDIRS), TVI_ROOT, m_bKeepUnavailableFixedSharedDirs);

		m_ctrlAdvTreeOptions.SendMessage(WM_VSCROLL, SB_TOP);
		m_bInitializedAdvTreeOpts = true;
	}

	DDX_TreeCheck(pDX, IDC_ADVANCED_OPTS,m_hti_bMiniMuleAutoClose,bMiniMuleAutoClose);
	DDX_TreeEdit(pDX, IDC_ADVANCED_OPTS, m_hti_iMiniMuleTransparency, iMiniMuleTransparency);
	DDV_MinMaxInt(pDX, iMiniMuleTransparency, 0, 100);

	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_hti_bMediaInfo_RIFF,bMediaInfo_RIFF);
	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_hti_bMediaInfo_ID3LIB,bMediaInfo_ID3LIB);
#ifdef HAVE_QEDIT_H
	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_hti_MediaInfo_MediaDet,m_bMediaInfo_MediaDet);
#endif//HAVE_QEDIT_H
	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_hti_MediaInfo_RM,m_bMediaInfo_RM);
#ifdef HAVE_WMSDK_H
	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_hti_MediaInfo_WM,m_bMediaInfo_WM);
#endif//HAVE_WMSDK_H

	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_hti_m_bRestoreLastMainWndDlg,m_bRestoreLastMainWndDlg);
	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_hti_m_bRestoreLastLogPane,m_bRestoreLastLogPane);
	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_hti_m_iStraightWindowStyles,m_iStraightWindowStyles);
	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_hti_m_bRTLWindowsLayout,m_bRTLWindowsLayout);
	DDX_TreeEdit(pDX, IDC_ADVANCED_OPTS, m_hti_m_iMaxChatHistory, m_iMaxChatHistory);
	DDV_MinMaxInt(pDX, m_iMaxChatHistory, 3, 2048);
	DDX_TreeEdit(pDX,IDC_ADVANCED_OPTS,m_hti_maxmsgsessions,(int)m_umaxmsgsessions);
	DDV_MinMaxInt(pDX, m_umaxmsgsessions, 0, 6000);
	DDX_Text(pDX, IDC_ADVANCED_OPTS, m_htidatetimeformat, m_strDateTimeFormat);
	DDX_Text(pDX, IDC_ADVANCED_OPTS, m_htidatetimeformat4lists, m_strDateTimeFormat4Lists);
	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_htiShowVerticalHourMarkers,m_bShowVerticalHourMarkers);
	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_htiReBarToolbar,m_bReBarToolbar);
	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_htiIconflashOnNewMessage,m_bIconflashOnNewMessage);
	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_htiShowCopyEd2kLinkCmd,m_bShowCopyEd2kLinkCmd);
	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_htiUpdateQueue,m_bUpdateQueue);
	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_htiRepaint,m_bRepaint);
	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_htiExtraPreviewWithMenu,m_bExtraPreviewWithMenu);
	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_htiShowUpDownIconInTaskbar,m_bShowUpDownIconInTaskbar);
	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_htiForceSpeedsToKB,m_bForceSpeedsToKB);

	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_hti_m_iLogFileFormat,m_iLogFileFormat);
	DDX_TreeEdit(pDX, IDC_ADVANCED_OPTS, m_hti_iMaxLogBuff, iMaxLogBuff);
	DDV_MinMaxInt(pDX, iMaxLogBuff, 64, 512);
	DDX_Text(pDX, IDC_ADVANCED_OPTS, m_htidatetimeformat4log, m_strDateTimeFormat4Log);
	DDX_TreeColor(pDX, IDC_ADVANCED_OPTS, m_htiLogError, m_crLogError);
	DDX_TreeColor(pDX, IDC_ADVANCED_OPTS, m_htiLogWarning, m_crLogWarning);
	DDX_TreeColor(pDX, IDC_ADVANCED_OPTS, m_htiLogSuccess, m_crLogSuccess);

	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_hti_bCheckComctl32 ,bCheckComctl32 );
	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_hti_bCheckShell32,bCheckShell32);
	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_hti_bIgnoreInstances,bIgnoreInstances);
	DDX_TreeEdit(pDX, IDC_ADVANCED_OPTS, m_hti_sNotifierMailEncryptCertName, sNotifierMailEncryptCertName);
	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_hti_m_iPreviewSmallBlocks,m_iPreviewSmallBlocks);
	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_hti_m_bPreviewCopiedArchives,m_bPreviewCopiedArchives);
	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_hti_m_bPreviewOnIconDblClk,m_bPreviewOnIconDblClk);
	DDX_TreeEdit(pDX, IDC_ADVANCED_OPTS, m_hti_sInternetSecurityZone, sInternetSecurityZone);
	DDX_TreeEdit(pDX, IDC_ADVANCED_OPTS, m_hti_sTxtEditor, sTxtEditor); 
	DDX_TreeEdit(pDX, IDC_ADVANCED_OPTS, m_hti_iServerUDPPort, iServerUDPPort);
	DDV_MinMaxInt(pDX, iServerUDPPort, 0,65535);
	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_hti_m_bRemoveFilesToBin,m_bRemoveFilesToBin);
	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_hti_HighresTimer,m_bHighresTimer);
	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_hti_TrustEveryHash,m_bTrustEveryHash);
	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_hti_InspectAllFileTypes,m_iInspectAllFileTypes);
	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_hti_PreferRestrictedOverUser,m_bPreferRestrictedOverUser);
	DDX_TreeEdit(pDX,IDC_ADVANCED_OPTS,m_hti_WebFileUploadSizeLimitMB,m_iWebFileUploadSizeLimitMB);
	DDV_MinMaxInt(pDX, m_iWebFileUploadSizeLimitMB, 0, INT_MAX);
	DDX_TreeEdit(pDX,IDC_ADVANCED_OPTS,m_hti_AllowedIPs,m_sAllowedIPs);
	DDX_TreeCheck(pDX,IDC_ADVANCED_OPTS,m_hti_UseUserSortedServerList,m_bUseUserSortedServerList);
	DDX_TreeEdit(pDX,IDC_ADVANCED_OPTS,m_hti_DebugSearchResultDetailLevel,m_iDebugSearchResultDetailLevel); //TODO: check string for ip
	DDX_TreeCheck(pDX, IDC_ADVANCED_OPTS, m_htiAdjustNTFSDaylightFileTime, m_bAdjustNTFSDaylightFileTime);
	DDX_TreeCheck(pDX, IDC_ADVANCED_OPTS, m_htidontcompressavi, m_dontcompressavi);
	DDX_TreeCheck(pDX, IDC_ADVANCED_OPTS, m_htiICH, m_ICH);
	DDX_TreeIPAddress( pDX, IDC_ADVANCED_OPTS,m_htiBindAddr  , m_dwBindAddr);
	DDX_TreeEdit(pDX,IDC_ADVANCED_OPTS,m_htiFileBufferTimeLimit,m_iFileBufferTimeLimit);
	DDV_MinMaxInt(pDX, m_iFileBufferTimeLimit , 1,600); // max 10 minutes
	DDX_TreeCheck(pDX, IDC_ADVANCED_OPTS, m_htiRearrangeKadSearchKeywords, m_bRearrangeKadSearchKeywords);
	DDX_TreeCheck(pDX, IDC_ADVANCED_OPTS, m_htiBeeper, m_bBeeper);
	DDX_TreeCheck(pDX, IDC_ADVANCED_OPTS, m_htiMsgOnlySec, m_bMsgOnlySec);
	DDX_TreeCheck(pDX, IDC_ADVANCED_OPTS, m_htiDisablePeerCache, m_bDisablePeerCache);
	DDX_TreeCheck(pDX, IDC_ADVANCED_OPTS, m_htiKeepUnavailableFixedSharedDirs, m_bKeepUnavailableFixedSharedDirs);
	// <== Advanced Options [Official/MorphXT] - Stulle
}


BOOL CPPgScar::OnInitDialog()
{
	// ==> push small files [sivka] - Stulle
	m_bEnablePushSmallFile = thePrefs.GetEnablePushSmallFile();
	m_iPushSmallFileBoost = thePrefs.GetPushSmallFileBoost();
	// <== push small files [sivka] - Stulle
	m_bEnablePushRareFile = thePrefs.enablePushRareFile; // push rare file - Stulle

	// ==> FunnyNick [SiRoB/Stulle] - Stulle
	m_bFnActive = thePrefs.DisplayFunnyNick();
	m_iFnTag = thePrefs.GetFnTag();
	m_sFnCustomTag = thePrefs.m_sFnCustomTag;
	m_bFnTagAtEnd = thePrefs.GetFnTagAtEnd();
	// <== FunnyNick [SiRoB/Stulle] - Stulle

	// ==> Quick start [TPT] - Max
	m_bQuickStart = thePrefs.GetQuickStart();
	m_iQuickStartMaxTime = (int)(thePrefs.GetQuickStartMaxTime());
	m_iQuickStartMaxConnPerFive = (int)(thePrefs.GetQuickStartMaxConnPerFive());
	m_iQuickStartMaxConn = (int)(thePrefs.GetQuickStartMaxConn());
	m_iQuickStartMaxConnPerFiveBack = (int)(thePrefs.GetQuickStartMaxConnPerFiveBack());
	m_iQuickStartMaxConnBack = (int)(thePrefs.GetQuickStartMaxConnBack());
	m_bQuickStartAfterIPChange = thePrefs.GetQuickStartAfterIPChange();
	// <== Quick start [TPT] - Max
	// ==> Enforce Ratio [Stulle] - Stulle
	m_bEnforceRatio = thePrefs.GetEnforceRatio();
	m_iRatioValue = (int)thePrefs.GetRatioValue();
	// <== Enforce Ratio [Stulle] - Stulle
	// ==> Improved ICS-Firewall support [MoNKi] - Max
	m_bICFSupport = thePrefs.GetICFSupport();
	m_bICFSupportClearAtEnd = thePrefs.IsOpenPortsOnStartupEnabled();
	m_bICFSupportServerUDP = thePrefs.GetICFSupportServerUDP();
	// <== Improved ICS-Firewall support [MoNKi] - Max
	//==> UPnP support [MoNKi] - leuk_he 
	m_bUPnPNat = thePrefs.IsUPnPEnabled();
	m_bUpnPNATwebservice = thePrefs.GetUPnPNatWeb();
	m_dwUpnpBindAddr = thePrefs.GetUpnpBindAddr();
	m_bUPnPForceUpdate=thePrefs.m_bUPnPForceUpdate; 
	//<== UPnP support [MoNKi] - leuk_he
	// ==> Random Ports [MoNKi] - Stulle
	m_bRandomports = thePrefs.GetUseRandomPorts();
	m_iRandomFirstPort = thePrefs.GetMinRandomPort();
	m_iRandomLastPort = thePrefs.GetMaxRandomPort();
	m_iRandomPortsResetTime = thePrefs.GetRandomPortsSafeResetOnRestartTime();
	// <== Random Ports [MoNKi] - Stulle
	// ==> Mephisto Upload - Mephisto
	m_iMinSlots = thePrefs.GetMinSlots();
	m_iNoNewSlotTimer = thePrefs.GetNoNewSlotTimer();
	m_iFullLoops = thePrefs.GetFullLoops();
	m_iMonitorLoops = thePrefs.GetMonitorLoops();
	m_iNotReachedBW = thePrefs.GetNotReachedBW();
	m_iNoTrickleTimer = thePrefs.GetNoTrickleTimer();
	m_iMoveDownKB = thePrefs.GetMoveDownKB();
	// <== Mephisto Upload - Mephisto
	// ==> Multiple Part Transfer [Stulle] - Mephisto
	m_iChunksToFinish = thePrefs.GetChunksToFinish();
	m_iChunksToUpload = thePrefs.GetChunksToUpload();
	// <== Multiple Part Transfer [Stulle] - Mephisto
	m_iMaxUpMinutes = thePrefs.GetMaxUpMinutes(); // Adjust max upload time [Stulle] - Mephisto
	m_iReAskFileSrc = (thePrefs.GetReAskTimeDif() + FILEREASKTIME)/60000; // Timer for ReAsk File Sources [Stulle] - Stulle
	m_bACC = thePrefs.GetACC(); // ACC [Max/WiZaRd] - Max
	m_bIgnoreThird = thePrefs.GetIgnoreThird(); // Do not reserve 1/3 of your uploadlimit for emule [Stulle] - Stulle
	m_bUlThres = !(thePrefs.GetDisableUlThres()); // Disable accepting only clients who asked within last 30min [Stulle] - Stulle
	m_bMaxSlotSpeed = thePrefs.GetMaxSlotSpeed(); // Alwasy maximize slot speed [Stulle] - Stulle

	// ==> Anti Uploader Ban [Stulle] - Stulle
	m_iAntiUploaderBanLimit = thePrefs.GetAntiUploaderBanLimit();
	m_iAntiUploaderBanCase = thePrefs.GetAntiUploaderBanCase();
	// <== Anti Uploader Ban [Stulle] - Stulle

	m_bFineCS = thePrefs.m_bFineCS; // Modified FineCS [CiccioBastardo/Stulle] - Stulle
	// ==> Pay Back First [AndCycle/SiRoB/Stulle] - Stulle
	m_bIsPayBackFirst = thePrefs.m_bPayBackFirst;
	m_iPayBackFirstLimit = thePrefs.m_iPayBackFirstLimit;
	m_bIsPayBackFirst2 = thePrefs.m_bPayBackFirst2;
	m_iPayBackFirstLimit2 = thePrefs.m_iPayBackFirstLimit2;
	// <== Pay Back First [AndCycle/SiRoB/Stulle] - Stulle

	// ==> CPU/MEM usage [$ick$/Stulle] - Max
	m_bSysInfo = thePrefs.GetSysInfo();
	m_bSysInfoGlobal = thePrefs.GetSysInfoGlobal();
	// <== CPU/MEM usage [$ick$/Stulle] - Max
	// ==> Invisible Mode [TPT/MoNKi] - Stulle
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
	m_bInvisibleModeStart = thePrefs.GetInvisibleModeStart();
	// <== Invisible Mode [TPT/MoNKi] - Stulle
	showSrcInTitle = thePrefs.ShowSrcOnTitle(); // Show sources on title - Stulle
	m_bShowGlobalHL = thePrefs.GetShowGlobalHL(); // show global HL - Stulle
	m_bShowFileHLconst = thePrefs.GetShowFileHLconst(); // show HL per file constantly - Stulle
	m_bShowInMSN7 = thePrefs.GetShowMSN7(); // Show in MSN7 [TPT] - Stulle
	m_bQueueProgressBar = thePrefs.ShowClientQueueProgressBar(); // Client queue progress bar [Commander] - Stulle
	m_bTrayComplete = thePrefs.GetTrayComplete(); // Completed in Tray [Stulle] - Stulle
	m_bColorFeedback = thePrefs.GetColorFeedback(); // Feedback personalization [Stulle] - Stulle
	m_bShowClientPercentage = thePrefs.GetShowClientPercentage(); // Show Client Percentage optional [Stulle] - Stulle
	m_bFollowTheMajority = thePrefs.IsFollowTheMajorityEnabled(); // Follow The Majority [AndCycle/Stulle] - Stulle
	m_bShowSpeedMeter = thePrefs.GetShowSpeedMeter(); // High resolution speedmeter on toolbar [eFMod/Stulle] - Myth88
	// ==> Static Tray Icon [MorphXT] - MyTh88
	m_bStaticIcon = thePrefs.GetStaticIcon();
	// <== Static Tray Icon [MorphXT] - MyTh88

	// ==> File Settings [sivka/Stulle] - Stulle
	m_bEnableAutoDropNNSDefault = thePrefs.m_EnableAutoDropNNSDefault;
	m_iAutoNNS_TimerDefault = (thePrefs.m_AutoNNS_TimerDefault/1000);
	m_iMaxRemoveNNSLimitDefault = thePrefs.m_MaxRemoveNNSLimitDefault;
	m_bEnableAutoDropFQSDefault = thePrefs.m_EnableAutoDropFQSDefault;
	m_iAutoFQS_TimerDefault = (thePrefs.m_AutoFQS_TimerDefault/1000);
	m_iMaxRemoveFQSLimitDefault = thePrefs.m_MaxRemoveFQSLimitDefault;
	m_bEnableAutoDropQRSDefault = thePrefs.m_EnableAutoDropQRSDefault;
	m_iAutoHQRS_TimerDefault = (thePrefs.m_AutoHQRS_TimerDefault/1000);
	m_iMaxRemoveQRSDefault = thePrefs.m_MaxRemoveQRSDefault;
	m_iMaxRemoveQRSLimitDefault = thePrefs.m_MaxRemoveQRSLimitDefault;
	// <== File Settings [sivka/Stulle] - Stulle

	// ==> TBH: minimule - Max
	m_bShowMM = thePrefs.IsMiniMuleEnabled();
	m_bMMLives = thePrefs.GetMiniMuleLives();
	m_iMMUpdateTime = thePrefs.GetMiniMuleUpdate();
	m_iMMTrans = thePrefs.GetMiniMuleTransparency();
	m_bMMCompl = thePrefs.m_bMMCompl;
	m_bMMOpen = thePrefs.m_bMMOpen;
	// <== TBH: minimule - Max

	// ==> Control download priority [tommy_gun/iONiX] - MyTh88
	m_iAutoDownPrio = thePrefs.GetBowlfishMode();
	m_iAutoDownPrioPerc = thePrefs.GetBowlfishPrioPercentValue();
	m_iAutoDownPrioSize = thePrefs.GetBowlfishPrioSizeValue();
	m_iAutoDownPrioVal = thePrefs.GetBowlfishPrioNewValue();
	// <== Control download priority [tommy_gun/iONiX] - MyTh88


	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	m_iDlMode = thePrefs.GetDlMode();
	m_bShowCatNames = thePrefs.ShowCatNameInDownList();
	m_bSelectCat = thePrefs.SelectCatForNewDL();
	m_bUseActiveCat = thePrefs.UseActiveCatForLinks();
	m_bAutoSetResOrder = thePrefs.AutoSetResumeOrder();
	m_bSmallFileDLPush = thePrefs.SmallFileDLPush();
	m_iResumeFileInNewCat = thePrefs.StartDLInEmptyCats();
	m_bUseAutoCat = thePrefs.UseAutoCat();
	m_bAddRemovedInc = thePrefs.UseAddRemoveInc();
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

	// ==> PowerShare [ZZ/MorphXT] - Stulle
	m_iPowershareMode = thePrefs.m_iPowershareMode;
	m_iPowerShareLimit = thePrefs.PowerShareLimit;
	// <== PowerShare [ZZ/MorphXT] - Stulle
	m_iPsAmountLimit = thePrefs.GetPsAmountLimit(); // Limit PS by amount of data uploaded [Stulle] - Stulle
	// ==> Spread Credits Slot [Stulle] - Stulle
	m_bSpreadCreditsSlot = thePrefs.GetSpreadCreditsSlot();
	m_iSpreadCreditsSlotCounter = (int)(thePrefs.GetSpreadCreditsSlotCounter());
	// <== Spread Credits Slot [Stulle] - Stulle
	// ==> Release Bonus [sivka] - Stulle
	if (thePrefs.GetReleaseBonus() <= 1)
	{
		m_iReleaseBonus = thePrefs.GetReleaseBonus();
		m_iReleaseBonusDays = 1;
	}
	else
	{
		m_iReleaseBonus = 2;
		m_iReleaseBonusDays = (int)(thePrefs.GetReleaseBonus()/2);
	}
	// <== Release Bonus [sivka] - Stulle
	m_bReleaseScoreAssurance = thePrefs.GetReleaseScoreAssurance(); // Release Score Assurance [Stulle] - Stulle
	m_bSpreadBars = thePrefs.GetSpreadbarSetStatus(); // Spread bars [Slugfiller/MorphXT] - Stulle
	// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	m_iHideOS = thePrefs.hideOS;
	m_bSelectiveShare = thePrefs.selectiveShare;
	m_iShareOnlyTheNeed = thePrefs.ShareOnlyTheNeed;
	// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	m_iFairPlay = thePrefs.GetFairPlay(); // Fair Play [AndCycle/Stulle] - Stulle

	// ==> Global Source Limit [Max/Stulle] - Stulle
	m_bGlobalHL = thePrefs.IsUseGlobalHL();
	m_iGlobalHL = thePrefs.GetGlobalHL();
	m_bGlobalHlAll = thePrefs.GetGlobalHlAll();
	m_bGlobalHlDefault = thePrefs.GetGlobalHlDefault();
	// <== Global Source Limit [Max/Stulle] - Stulle
	// ==> Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	m_bEmuMLDonkey = thePrefs.IsEmuMLDonkey();
	m_bEmueDonkey = thePrefs.IsEmueDonkey();
	m_bEmueDonkeyHybrid = thePrefs.IsEmueDonkeyHybrid();
	m_bEmuShareaza = thePrefs.IsEmuShareaza();
	m_bEmuLphant = thePrefs.IsEmuLphant();
	m_bLogEmulator = thePrefs.IsEmuLog();
	// <== Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	// ==> Automatic shared files updater [MoNKi] - Stulle
	m_bAutoSharedUpdater = thePrefs.GetDirectoryWatcher();
	m_bSingleSharedDirUpdater = thePrefs.GetSingleSharedDirWatcher();
	m_iTimeBetweenReloads = thePrefs.GetTimeBetweenReloads();
	// <== Automatic shared files updater [MoNKi] - Stulle
	m_bSUQWT = thePrefs.SaveUploadQueueWaitTime(); // SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
	m_bStartupSound = thePrefs.UseStartupSound(); // Startupsound [Commander] - mav744
	m_iCompressLevel = thePrefs.GetCompressLevel(); // Adjust Compress Level [Stulle] - Stulle

	// ==> Advanced Options [Official/MorphXT] - Stulle
	bMiniMuleAutoClose=thePrefs.bMiniMuleAutoClose;
	iMiniMuleTransparency=thePrefs.iMiniMuleTransparency;

	sMediaInfo_MediaInfoDllPath=thePrefs.sMediaInfo_MediaInfoDllPath;
	bMediaInfo_RIFF=thePrefs.bMediaInfo_RIFF;
	bMediaInfo_ID3LIB=thePrefs.bMediaInfo_ID3LIB;
#ifdef HAVE_QEDIT_H
	m_bMediaInfo_MediaDet=thePrefs.m_bMediaInfo_MediaDet;
#endif//HAVE_QEDIT_H
	m_bMediaInfo_RM=thePrefs.m_bMediaInfo_RM;
#ifdef HAVE_WMSDK_H
	m_bMediaInfo_WM=thePrefs.m_bMediaInfo_WM;
#endif//HAVE_WMSDK_H

	m_bRestoreLastMainWndDlg=thePrefs.m_bRestoreLastMainWndDlg;
	m_bRestoreLastLogPane=thePrefs.m_bRestoreLastLogPane;
	m_iStraightWindowStyles=thePrefs.m_iStraightWindowStyles;
	m_bRTLWindowsLayout=thePrefs.m_bRTLWindowsLayout;
	m_iMaxChatHistory=thePrefs.m_iMaxChatHistory;
	m_umaxmsgsessions=thePrefs.maxmsgsessions;
	m_strDateTimeFormat = thePrefs.m_strDateTimeFormat;
	m_strDateTimeFormat4Lists = thePrefs.m_strDateTimeFormat4Lists;
	m_bShowVerticalHourMarkers = thePrefs.m_bShowVerticalHourMarkers;
	m_bReBarToolbar = thePrefs.m_bReBarToolbar;
	m_bIconflashOnNewMessage = thePrefs.m_bIconflashOnNewMessage;
	m_bShowCopyEd2kLinkCmd = thePrefs.m_bShowCopyEd2kLinkCmd;
	m_bUpdateQueue = thePrefs.m_bupdatequeuelist;
	m_bRepaint = thePrefs.IsGraphRecreateDisabled();
	m_bExtraPreviewWithMenu = thePrefs.GetExtraPreviewWithMenu();
	m_bShowUpDownIconInTaskbar = thePrefs.IsShowUpDownIconInTaskbar();
	m_bForceSpeedsToKB = thePrefs.GetForceSpeedsToKB();

	m_iLogFileFormat=thePrefs.m_iLogFileFormat;
	iMaxLogBuff=thePrefs.GetMaxLogBuff()/1024;
	m_strDateTimeFormat4Log = thePrefs.m_strDateTimeFormat4Log;
	m_crLogError = thePrefs.m_crLogError;
	m_crLogWarning = thePrefs.m_crLogWarning;
	m_crLogSuccess = thePrefs.m_crLogSuccess;

	bCheckComctl32 =thePrefs.bCheckComctl32 ;
	bCheckShell32=thePrefs.bCheckShell32;
	bIgnoreInstances=thePrefs.bIgnoreInstances;
	sNotifierMailEncryptCertName=thePrefs.sNotifierMailEncryptCertName;
	m_iPreviewSmallBlocks=thePrefs.m_iPreviewSmallBlocks;
	m_bPreviewCopiedArchives=thePrefs.m_bPreviewCopiedArchives;
	m_bPreviewOnIconDblClk=thePrefs.m_bPreviewOnIconDblClk;
	sInternetSecurityZone=thePrefs.sInternetSecurityZone;
	sTxtEditor=thePrefs.GetTxtEditor();
	iServerUDPPort=thePrefs.GetServerUDPPort();
	m_bRemoveFilesToBin=thePrefs.GetRemoveToBin();
	m_bHighresTimer=thePrefs.m_bHighresTimer;
	m_bTrustEveryHash=thePrefs.m_bTrustEveryHash;
	m_iInspectAllFileTypes=thePrefs.m_iInspectAllFileTypes;
	m_bPreferRestrictedOverUser=thePrefs.m_bPreferRestrictedOverUser;
	m_bUseUserSortedServerList=thePrefs.m_bUseUserSortedServerList;
	m_iWebFileUploadSizeLimitMB=thePrefs.m_iWebFileUploadSizeLimitMB;
	m_sAllowedIPs=_T("");
	if (thePrefs.GetAllowedRemoteAccessIPs().GetCount() > 0)
		for (int i = 0; i <  thePrefs.GetAllowedRemoteAccessIPs().GetCount(); i++)
           m_sAllowedIPs= m_sAllowedIPs+ _T(";") + ipstr(thePrefs.GetAllowedRemoteAccessIPs()[i]);
	m_iDebugSearchResultDetailLevel=thePrefs.GetDebugSearchResultDetailLevel();
	m_bAdjustNTFSDaylightFileTime = thePrefs.m_bAdjustNTFSDaylightFileTime;
	m_dontcompressavi = thePrefs.dontcompressavi;
	m_ICH = thePrefs.ICH;
	if ((thePrefs.GetBindAddrA()!= NULL)  && inet_addr(thePrefs.GetBindAddrA())!= INADDR_NONE)
	     m_dwBindAddr = ntohl(inet_addr(thePrefs.GetBindAddrA())); 
	else 
		m_dwBindAddr = 0;
	m_iFileBufferTimeLimit = thePrefs.GetFileBufferTimeLimit()/1000;
	m_bRearrangeKadSearchKeywords = thePrefs.GetRearrangeKadSearchKeywords();
	m_bBeeper = thePrefs.beepOnError;
	m_bMsgOnlySec = thePrefs.MsgOnlySecure();
	m_bDisablePeerCache = !thePrefs.m_bPeerCacheEnabled;
	m_bKeepUnavailableFixedSharedDirs = thePrefs.m_bKeepUnavailableFixedSharedDirs;
	// <== Advanced Options [Official/MorphXT] - Stulle

	CPropertyPage::OnInitDialog();

	// ==> Tabbed Preferences [TPT] - Stulle
	// Init the Tab control
	InitTab();

	// Create and Init all controls
	InitControl();

	// Set default tab
	m_currentTab = SCAR;
	SetTab(theApp.emuledlg->preferenceswnd->m_ScarTab);
	// <== Tabbed Preferences [TPT] - Stulle

	// ==> Design Settings [eWombat/Stulle] - Stulle
	m_bDesignChanged = false;
	m_bFocusWasOnCombo = false;
	m_bold.SetWindowText(_T(""));
	m_bold.SetIcon((HICON)::LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE((int)IDI_FONTB), IMAGE_ICON, 16, 16, 0));
	m_underlined.SetWindowText(_T(""));
	m_underlined.SetIcon((HICON)::LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE((int)IDI_FONTUL), IMAGE_ICON, 16, 16, 0));
	m_italic.SetWindowText(_T(""));
	m_italic.SetIcon((HICON)::LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE((int)IDI_FONTI), IMAGE_ICON, 16, 16, 0));

	m_FontColor.SetColor(GetSysColor(COLOR_WINDOWTEXT));
	m_FontColor.SetDefaultColor(GetSysColor(COLOR_WINDOWTEXT));
	m_BackColor.SetColor(COLORREF(RGB(255,255,255)));
	m_BackColor.SetDefaultColor(COLORREF(RGB(255,255,255)));
	// <== Design Settings [eWombat/Stulle] - Stulle

	// ==> push small files [sivka] - Stulle
	InitWindowStyles(this);
	m_ctlPushSmallSize.SetRange(1, PARTSIZE>>10, TRUE);
	m_ctlPushSmallSize.SetPos(thePrefs.GetPushSmallFileSize()>>10);
	m_ctlPushSmallSize.SetTicFreq(1024);
	m_ctlPushSmallSize.SetPageSize(1024);
	ShowPushSmallFileValues();
	LoadSettings();
	// <== push small files [sivka] - Stulle

	Localize();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPPgScar::LoadSettings(void)
{
	if(m_hWnd)
	{
		CString strBuffer;
	
		// ==> push small files [sivka] - Stulle
		// ==> Tabbed Preferences [TPT] - Stulle
		/*
		((CSliderCtrl*)GetDlgItem(IDC_PUSHSMALL_SLIDER))->SetRange(1, PARTSIZE, TRUE);
		((CSliderCtrl*)GetDlgItem(IDC_PUSHSMALL_SLIDER))->SetPos(thePrefs.GetPushSmallFileSize());
		ShowPushSmallFileValues();
		*/
		/*
		m_ctlPushSmallSize.SetPos(thePrefs.GetPushSmallFileSize());
		ShowPushSmallFileValues();
		*/
		// <== Tabbed Preferences [TPT] - Stulle
		// <== push small files [sivka] - Stulle

		// ==> TBH: Backup [TBH/EastShare/MorphXT] - Stulle
		m_AutoBackup.SetCheck(thePrefs.GetAutoBackup());
		m_AutoBackup2.EnableWindow(thePrefs.GetAutoBackup());
		m_AutoBackup2.SetCheck(thePrefs.GetAutoBackup2());
		// <== TBH: Backup [TBH/EastShare/MorphXT] - Stulle

		// ==> Design Settings [eWombat/Stulle] - Stulle
		for (int i=0;i<style_c_count;i++)
			thePrefs.GetStyle(client_styles, i, &nClientStyles[i]);

		for (int i=0;i<style_d_count;i++)
			thePrefs.GetStyle(download_styles, i, &nDownloadStyles[i]);

		for (int i=0;i<style_s_count;i++)
			thePrefs.GetStyle(share_styles, i, &nShareStyles[i]);

		for (int i=0;i<style_se_count;i++)
			thePrefs.GetStyle(server_styles, i, &nServerStyles[i]);

		for (int i=0;i<style_b_count;i++)
			thePrefs.GetStyle(background_styles, i, &nBackgroundStyles[i]);

		for (int i=0;i<style_w_count;i++)
			thePrefs.GetStyle(window_styles, i, &nWindowStyles[i]);

		// ==> Feedback personalization [Stulle] - Stulle
		for (int i=0;i<style_f_count;i++)
			thePrefs.GetStyle(feedback_styles, i, &nFeedBackStyles[i]);
		// <== Feedback personalization [Stulle] - Stulle
		// <== Design Settings [eWombat/Stulle] - Stulle

		// ==> Advanced Updates [MorphXT/Stulle] - Stulle
		m_AntiLeechStart.SetCheck(thePrefs.IsAutoUpdateAntiLeech());
		m_AntiLeechWeek.SetCheck(theApp.scheduler->HasWeekly(ACTION_UPDANTILEECH));
		m_AntiLeechURL.SetWindowText(thePrefs.GetAntiLeechURL());

		m_IpFilterStart.SetCheck(thePrefs.AutoUpdateIPFilter());
		m_IpFilterWeek.SetCheck(theApp.scheduler->HasWeekly(ACTION_UPDIPCONF));
		m_IpFilterURL.SetWindowText(thePrefs.GetAutoUpdateIPFilter_URL());

		m_CountryStart.SetCheck(thePrefs.AutoUpdateIP2Country);
		m_CountryURL.SetWindowText(thePrefs.UpdateURLIP2Country);

		TCHAR sTime[30];
		if(thePrefs.IsIPFilterViaDynDNS())
		{
			CString strBuffer=NULL;
			if(PathFileExists(theApp.ipfilter->GetDefaultFilePath()))
				strBuffer.Format(_T("v%u"), thePrefs.GetIPFilterVersionNum());
			else
				strBuffer=GetResString(IDS_DL_NONE);
			m_IpFilterTime.SetWindowText(strBuffer);
		}
		else
		{
			sTime[0] = _T('\0');
			SysTimeToStr(thePrefs.GetIPfilterVersion(), sTime);
			m_IpFilterTime.SetWindowText(sTime);
		}

		sTime[0] = _T('\0');
		SysTimeToStr(thePrefs.GetIP2CountryVersion(), sTime);
		m_CountryTime.SetWindowText(sTime);
		// <== Advanced Updates [MorphXT/Stulle] - Stulle
	}
}

BOOL CPPgScar::OnKillActive()
{
	// if prop page is closed by pressing VK_ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	m_ctrlAdvTreeOptions.HandleChildControlLosingFocus(); // Advanced Options [Official/MorphXT] - Stulle
	return CPropertyPage::OnKillActive();
}

BOOL CPPgScar::OnApply()
{
	// if prop page is closed by pressing VK_ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	m_ctrlAdvTreeOptions.HandleChildControlLosingFocus(); // Advanced Options [Official/MorphXT] - Stulle

	if (!UpdateData())
		return FALSE;
	// ==> push small files [sivka] - Stulle
	thePrefs.enablePushSmallFile = m_bEnablePushSmallFile;
	thePrefs.m_iPushSmallBoost = (uint16)m_iPushSmallFileBoost;
	// ==> Tabbed Preferences [TPT] - Stulle
	/*
	((CSliderCtrl*)GetDlgItem(IDC_PUSHSMALL_SLIDER))->SetRange(1, PARTSIZE, TRUE);
	thePrefs.m_iPushSmallFiles = ((CSliderCtrl*)GetDlgItem(IDC_PUSHSMALL_SLIDER))->GetPos();
	*/
	thePrefs.m_iPushSmallFiles = m_ctlPushSmallSize.GetPos()<<10;
	// <== Tabbed Preferences [TPT] - Stulle
	// <== push small files [sivka] - Stulle
	thePrefs.enablePushRareFile = m_bEnablePushRareFile; // push rare file - Stulle

	// ==> FunnyNick [SiRoB/Stulle] - Stulle
	thePrefs.m_bFunnyNick = m_bFnActive;
	thePrefs.FnTagMode = (uint8)m_iFnTag;
	_stprintf (thePrefs.m_sFnCustomTag,_T("%s"), m_sFnCustomTag);
	thePrefs.m_bFnTagAtEnd = m_bFnTagAtEnd;
	// <== FunnyNick [SiRoB/Stulle] - Stulle

	// ==> Quick start [TPT] - Max
	thePrefs.m_bQuickStart = m_bQuickStart;
	thePrefs.m_iQuickStartMaxTime = (uint16)m_iQuickStartMaxTime;
	thePrefs.m_iQuickStartMaxConnPerFive = (uint16)m_iQuickStartMaxConnPerFive;
	thePrefs.m_iQuickStartMaxConn = m_iQuickStartMaxConn;
	thePrefs.m_iQuickStartMaxConnPerFiveBack = (uint16)m_iQuickStartMaxConnPerFiveBack;
	thePrefs.m_iQuickStartMaxConnBack = m_iQuickStartMaxConnBack;
	thePrefs.m_bQuickStartAfterIPChange = m_bQuickStartAfterIPChange;
	// <== Quick start [TPT] - Max
	// ==> Enforce Ratio [Stulle] - Stulle
	thePrefs.m_bEnforceRatio = m_bEnforceRatio;
	thePrefs.m_uRatioValue = (uint8) m_iRatioValue;
	// <== Enforce Ratio [Stulle] - Stulle
	// ==> Improved ICS-Firewall support [MoNKi] - Max
	bool bRestartApp = false;
	if(thePrefs.m_bICFSupportStatusChanged)
	{
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
	// <== Improved ICS-Firewall support [MoNKi] - Max
	//==> UPnP support [MoNKi] - leuk_he
	if((BOOL)thePrefs.IsUPnPEnabled() != m_bUPnPNat ||
		(BOOL)thePrefs.GetUPnPNatWeb() != m_bUpnPNATwebservice)
	{
		theApp.m_UPnP_IGDControlPoint->SetUPnPNat(m_bUPnPNat); // and start/stop nat. 
		thePrefs.SetUPnPNatWeb(m_bUpnPNATwebservice);
	}
	thePrefs.SetUpnpBindAddr(m_dwUpnpBindAddr);// Note: read code in thePrefs..
	thePrefs.m_bUPnPForceUpdate=thePrefs.m_bUPnPForceUpdate;
	//<== UPnP support [MoNKi] - leuk_he
	// ==> Random Ports [MoNKi] - Stulle
	bool oldUseRandom = thePrefs.GetUseRandomPorts();
	uint16 oldRndMin = thePrefs.GetMinRandomPort();
	uint16 oldRndMax = thePrefs.GetMaxRandomPort();
	thePrefs.SetUseRandomPorts(m_bRandomports);
	thePrefs.SetMinRandomPort((uint16)m_iRandomFirstPort);
	thePrefs.SetMaxRandomPort((uint16)m_iRandomLastPort);
	thePrefs.SetRandomPortsSafeResetOnRestartTime((uint16)m_iRandomPortsResetTime);

	if(m_bRandomports != oldUseRandom){
		if (theApp.IsPortchangeAllowed()){
			theApp.listensocket->Rebind();
			theApp.clientudp->Rebind();
		}
		else{
			bRestartApp = true;
			thePrefs.SetRandomPortsResetOnRestart(true);
		}
		// ==> Improved ICS-Firewall support [MoNKi] - Max
		theApp.m_pFirewallOpener->ClearMappingsAtEnd();
		// <== Improved ICS-Firewall support [MoNKi] - Max
	}
	else if(oldUseRandom){
		if(m_iRandomFirstPort != oldRndMin || m_iRandomLastPort != oldRndMax){
			if (theApp.IsPortchangeAllowed()){
				theApp.listensocket->Rebind();
				theApp.clientudp->Rebind();
			}
			else {
				bRestartApp = true;
				thePrefs.SetRandomPortsResetOnRestart(true);
			}

			// ==> Improved ICS-Firewall support [MoNKi] - Max
			theApp.m_pFirewallOpener->ClearMappingsAtEnd();
			// <== Improved ICS-Firewall support [MoNKi] - Max
		}
	}
	// <== Random Ports [MoNKi] - Stulle
	// ==> Mephisto Upload - Mephisto
	thePrefs.m_uMinSlots = (uint8)m_iMinSlots;
	thePrefs.m_uNoNewSlotTimer = (uint8)m_iNoNewSlotTimer;
	thePrefs.m_uFullLoops = (uint8)m_iFullLoops;
	thePrefs.m_uMonitorLoops = (uint8)m_iMonitorLoops;
	thePrefs.m_uNotReachedBW = (uint8)m_iNotReachedBW;
	thePrefs.m_uNoTrickleTimer = (uint8)m_iNoTrickleTimer;
	thePrefs.m_uMoveDownKB = (uint16)m_iMoveDownKB;
	// <== Mephisto Upload - Mephisto
	// ==> Multiple Part Transfer [Stulle] - Mephisto
	thePrefs.m_uChunksMode = (uint8)m_iChunksMode;
	thePrefs.m_uChunksToFinish = (uint8)m_iChunksToFinish;
	thePrefs.m_uChunksToUpload = (uint8)m_iChunksToUpload;
	// <== Multiple Part Transfer [Stulle] - Mephisto
	thePrefs.m_uMaxUpMinutes = (uint16)m_iMaxUpMinutes; // Adjust max upload time [Stulle] - Mephisto
	// ==> Timer for ReAsk File Sources [Stulle] - Stulle
	uint32 uTemp = (m_iReAskFileSrc-29)*60000;
	if(uTemp != thePrefs.m_uReAskTimeDif)
	{
		thePrefs.m_uReAskTimeDif = uTemp;
		theApp.clientlist->RecalculateReAskTimes();
	}
	// <== Timer for ReAsk File Sources [Stulle] - Stulle
	thePrefs.m_bACC = m_bACC; // ACC [Max/WiZaRd] - Max
	thePrefs.m_bIgnoreThird = m_bIgnoreThird; // Do not reserve 1/3 of your uploadlimit for emule [Stulle] - Stulle
	thePrefs.m_bDisableUlThres = !m_bUlThres; // Disable accepting only clients who asked within last 30min [Stulle] - Stulle
	// ==> Alwasy maximize slot speed [Stulle] - Stulle
	if(m_bMaxSlotSpeed && m_bMaxSlotSpeed != thePrefs.m_bMaxSlotSpeed)
		thePrefs.CheckSlotSpeed();
	thePrefs.m_bMaxSlotSpeed = m_bMaxSlotSpeed;
	// <== Alwasy maximize slot speed [Stulle] - Stulle

	// ==> Anti Uploader Ban [Stulle] - Stulle
	thePrefs.m_uAntiUploaderBanLimit = (uint16)m_iAntiUploaderBanLimit;
	thePrefs.AntiUploaderBanCaseMode = (uint8)m_iAntiUploaderBanCase;
	// <== Anti Uploader Ban [Stulle] - Stulle

	// ==> CreditSystems [EastShare/ MorphXT] - Stulle
	if(thePrefs.creditSystemMode != m_iCreditSystem){
		thePrefs.creditSystemMode = (uint8)m_iCreditSystem;
		theApp.clientcredits->ResetCheckScoreRatio();
	}
	// <== CreditSystems [EastShare/ MorphXT] - Stulle
	thePrefs.m_bFineCS = m_bFineCS; // Modified FineCS [CiccioBastardo/Stulle] - Stulle
	// ==> Pay Back First [AndCycle/SiRoB/Stulle] - Stulle
	thePrefs.m_bPayBackFirst = m_bIsPayBackFirst;
	thePrefs.m_iPayBackFirstLimit = (uint8)m_iPayBackFirstLimit;
	thePrefs.m_bPayBackFirst2 = m_bIsPayBackFirst2;
	thePrefs.m_iPayBackFirstLimit2 = (uint16)m_iPayBackFirstLimit2;
	// <== Pay Back First [AndCycle/SiRoB/Stulle] - Stulle

	// ==> CPU/MEM usage [$ick$/Stulle] - Max
	if(thePrefs.m_bSysInfo != m_bSysInfo)
	{
		thePrefs.m_bSysInfo = m_bSysInfo;
		theApp.emuledlg->transferwnd->EnableSysInfo(m_bSysInfo);
	}
	thePrefs.m_bSysInfoGlobal = m_bSysInfoGlobal;
	// <== CPU/MEM usage [$ick$/Stulle] - Max
	// ==> Invisible Mode [TPT/MoNKi] - Stulle
	thePrefs.SetInvisibleMode(m_bInvisibleMode, m_iInvisibleModeActualKeyModifier, (char)m_sInvisibleModeKey.GetAt(0));
	thePrefs.m_bInvisibleModeStart = m_bInvisibleModeStart;
	// <== Invisible Mode [TPT/MoNKi] - Stulle
	thePrefs.showSrcInTitle = showSrcInTitle; // Show sources on title - Stulle
	thePrefs.ShowGlobalHL = m_bShowGlobalHL; // show global HL - Stulle
	thePrefs.ShowFileHLconst = m_bShowFileHLconst; // show HL per file constantly - Stulle
	thePrefs.m_bShowInMSN7 = m_bShowInMSN7; // Show in MSN7 [TPT] - Stulle
	thePrefs.m_bClientQueueProgressBar = m_bQueueProgressBar; // Client queue progress bar [Commander] - Stulle
	thePrefs.m_bTrayComplete = m_bTrayComplete; // Completed in Tray [Stulle] - Stulle
	thePrefs.m_bColorFeedback = m_bColorFeedback; // Feedback personalization [Stulle] - Stulle
	thePrefs.m_bShowClientPercentage = m_bShowClientPercentage; // Show Client Percentage optional [Stulle] - Stulle
	thePrefs.m_bFollowTheMajority = m_bFollowTheMajority; // Follow The Majority [AndCycle/Stulle] - Stulle
	// ==> High resolution speedmeter on toolbar [eFMod/Stulle] - Myth88
	if(m_bShowSpeedMeter != thePrefs.GetShowSpeedMeter())
	{
		thePrefs.m_bShowSpeedMeter = m_bShowSpeedMeter;
		theApp.emuledlg->toolbar->ShowSpeedMeter(m_bShowSpeedMeter);
		LONG cx = thePrefs.GetToolbarIconSize().cx;
		LONG cy = thePrefs.GetToolbarIconSize().cy;
		if(m_bShowSpeedMeter || cx != 16)
			theApp.emuledlg->toolbar->OnCommand(MP_LARGEICONS,NULL);
		else 
			theApp.emuledlg->toolbar->OnCommand(MP_SMALLICONS,NULL);
		thePrefs.SetToolbarIconSize(CSize(cx,cy));
		switch(thePrefs.GetToolbarLabelSettings())
		{
		case NoLabels:
			theApp.emuledlg->toolbar->OnCommand(MP_NOTEXTLABELS,NULL);
			break;
		case LabelsBelow:
			theApp.emuledlg->toolbar->OnCommand(MP_TEXTLABELS,NULL);
			break;
		case LabelsRight:
			theApp.emuledlg->toolbar->OnCommand(MP_TEXTLABELSONRIGHT,NULL);
			break;
		}
	}
	// <== High resolution speedmeter on toolbar [eFMod/Stulle] - Myth88
	// ==> Static Tray Icon [MorphXT] - MyTh88
	if(m_bStaticIcon != thePrefs.m_bStaticIcon)
	{
		if(m_bStaticIcon)
			// ==> TBH: minimule - Stulle
			/*
			theApp.emuledlg->TrayShow();
			*/
			theApp.emuledlg->TrayShow(FALSE);
			// <== TBH: minimule - Stulle
		else if(theApp.emuledlg->IsWindowVisible()) //only hide when window visible
			theApp.emuledlg->TrayHide();
	}
	thePrefs.m_bStaticIcon = m_bStaticIcon;
	// <== Static Tray Icon [MorphXT] - MyTh88

	// ==> File Settings [sivka/Stulle] - Stulle
	thePrefs.m_EnableAutoDropNNSDefault = m_bEnableAutoDropNNSDefault;
	thePrefs.m_AutoNNS_TimerDefault = (m_iAutoNNS_TimerDefault*1000);
	thePrefs.m_MaxRemoveNNSLimitDefault = (uint16)m_iMaxRemoveNNSLimitDefault;
	thePrefs.m_EnableAutoDropFQSDefault = m_bEnableAutoDropFQSDefault;
	thePrefs.m_AutoFQS_TimerDefault = (m_iAutoFQS_TimerDefault*1000);
	thePrefs.m_MaxRemoveFQSLimitDefault = (uint16)m_iMaxRemoveFQSLimitDefault;
	thePrefs.m_EnableAutoDropQRSDefault = m_bEnableAutoDropQRSDefault;
	thePrefs.m_AutoHQRS_TimerDefault = (m_iAutoHQRS_TimerDefault*1000);
	thePrefs.m_MaxRemoveQRSDefault = (uint16)m_iMaxRemoveQRSDefault;
	thePrefs.m_MaxRemoveQRSLimitDefault = (uint16)m_iMaxRemoveQRSLimitDefault;
	thePrefs.m_bHQRXmanDefault = m_iHQRXmanDefault == 1;
	// <== File Settings [sivka/Stulle] - Stulle

	// ==> TBH: minimule - Max
	thePrefs.m_bMiniMule = m_bShowMM;
	thePrefs.SetMiniMuleLives(m_bMMLives);
	thePrefs.m_iMiniMuleUpdate = m_iMMUpdateTime;
	thePrefs.SetMiniMuleTransparency((uint8)m_iMMTrans);
	thePrefs.m_bMMCompl = m_bMMCompl;
	thePrefs.m_bMMOpen = m_bMMOpen;
	// <== TBH: minimule - Max

	// ==> Control download priority [tommy_gun/iONiX] - MyTh88
	thePrefs.m_uiBowlfishMode = (uint8)m_iAutoDownPrio;
	thePrefs.m_nBowlfishPrioPercentValue = (uint8)m_iAutoDownPrioPerc;
	thePrefs.m_nBowlfishPrioSizeValue = (uint16)m_iAutoDownPrioSize;
	thePrefs.m_nBowlfishPrioNewValue = (uint8)m_iAutoDownPrioVal;
	// <== Control download priority [tommy_gun/iONiX] - MyTh88

	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	thePrefs.dlMode = (uint8)m_iDlMode;
	thePrefs.m_bShowCatNames = m_bShowCatNames;
	thePrefs.m_bSelCatOnAdd = m_bSelectCat;
	thePrefs.m_bActiveCatDefault = m_bUseActiveCat;
	thePrefs.m_bAutoSetResumeOrder = m_bAutoSetResOrder;
	thePrefs.m_bSmallFileDLPush = m_bSmallFileDLPush;
	thePrefs.m_iStartDLInEmptyCats = (uint8)m_iResumeFileInNewCat;
	thePrefs.m_bUseAutoCat = m_bUseAutoCat;
	thePrefs.m_bAddRemovedInc = m_bAddRemovedInc;
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

	// ==> PowerShare [ZZ/MorphXT] - Stulle
	thePrefs.m_iPowershareMode = m_iPowershareMode;
	thePrefs.PowerShareLimit = m_iPowerShareLimit;
	theApp.sharedfiles->UpdatePartsInfo();
	// <== PowerShare [ZZ/MorphXT] - Stulle
	// ==> Limit PS by amount of data uploaded [Stulle] - Stulle
	thePrefs.PsAmountLimit = m_iPsAmountLimit;
	// <== Limit PS by amount of data uploaded [Stulle] - Stulle
	// ==> Spread Credits Slot [Stulle] - Stulle
	thePrefs.SpreadCreditsSlot = m_bSpreadCreditsSlot;
	thePrefs.SpreadCreditsSlotCounter = (uint16)m_iSpreadCreditsSlotCounter;
	// <== Spread Credits Slot [Stulle] - Stulle
	// ==> Release Bonus [sivka] - Stulle
	if (m_iReleaseBonus <= 1)
        thePrefs.m_uReleaseBonus = (uint8)m_iReleaseBonus;
	else
		thePrefs.m_uReleaseBonus = (uint8)(m_iReleaseBonusDays*2);
	// <== Release Bonus [sivka] - Stulle
	thePrefs.m_bReleaseScoreAssurance = m_bReleaseScoreAssurance; // Release Score Assurance [Stulle] - Stulle
	thePrefs.m_bSpreadbarSetStatus = m_bSpreadBars; // Spread bars [Slugfiller/MorphXT] - Stulle
	// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	thePrefs.hideOS = m_iHideOS;
	thePrefs.selectiveShare = m_bSelectiveShare;
	thePrefs.ShareOnlyTheNeed = m_iShareOnlyTheNeed!=0;
	// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	thePrefs.m_iFairPlay = m_iFairPlay; // Fair Play [AndCycle/Stulle] - Stulle

	// ==> Global Source Limit [Max/Stulle] - Stulle
	if (thePrefs.GetGlobalHL() != (UINT)m_iGlobalHL ||
		thePrefs.IsUseGlobalHL() != m_bGlobalHL ||
		thePrefs.m_bGlobalHlAll != m_bGlobalHlAll)
	{
		thePrefs.m_bGlobalHL = m_bGlobalHL;
		thePrefs.m_uGlobalHL = m_iGlobalHL;
		thePrefs.m_bGlobalHlAll = m_bGlobalHlAll;
		if(m_bGlobalHL && theApp.downloadqueue->GetPassiveMode())
		{
			theApp.downloadqueue->SetPassiveMode(false);
			theApp.downloadqueue->SetUpdateHlTime(50000); // 50 sec
			AddDebugLogLine(true,_T("{GSL} Global Source Limit settings have changed! Disabled PassiveMode!"));
		}
	}
	thePrefs.m_bGlobalHlDefault = m_bGlobalHlDefault;
	// <== Global Source Limit [Max/Stulle] - Stulle
	// ==> Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	thePrefs.m_bEmuMLDonkey = m_bEmuMLDonkey;
	thePrefs.m_bEmueDonkey = m_bEmueDonkey;
	thePrefs.m_bEmueDonkeyHybrid = m_bEmueDonkeyHybrid;
	thePrefs.m_bEmuShareaza = m_bEmuShareaza;
	thePrefs.m_bEmuLphant = m_bEmuLphant;
	thePrefs.m_bLogEmulator = m_bLogEmulator;
	// <== Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	// ==> Automatic shared files updater [MoNKi] - Stulle
	if(m_bAutoSharedUpdater != thePrefs.GetDirectoryWatcher() ||
		m_bSingleSharedDirUpdater != thePrefs.GetSingleSharedDirWatcher() ||
		(uint32)m_iTimeBetweenReloads != thePrefs.GetTimeBetweenReloads())
	{
		thePrefs.SetDirectoryWatcher(m_bAutoSharedUpdater);
		thePrefs.SetSingleSharedDirWatcher(m_bSingleSharedDirUpdater);
		thePrefs.SetTimeBetweenReloads((uint32)m_iTimeBetweenReloads);
		theApp.ResetDirectoryWatcher();
	}
	// <== Automatic shared files updater [MoNKi] - Stulle
	// ==> SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
	if(thePrefs.m_bSaveUploadQueueWaitTime != m_bSUQWT)
	{
		thePrefs.m_bSaveUploadQueueWaitTime = m_bSUQWT;
		bRestartApp = true;
	}
	// <== SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
	thePrefs.startupsound = m_bStartupSound; // Startupsound [Commander] - mav744
	// ==> Adjust Compress Level [Stulle] - Stulle
	thePrefs.m_uCompressLevel = (uint8)m_iCompressLevel;
	if(m_iCompressLevel == 0)
		thePrefs.m_bUseCompression = false;
	// <=== Adjust Compress Level [Stulle] - Stulle

	// ==> TBH: Backup [TBH/EastShare/MorphXT] - Stulle
	thePrefs.m_bAutoBackup = m_AutoBackup.GetCheck() == BST_CHECKED;
	thePrefs.m_bAutoBackup2 = m_AutoBackup2.GetCheck() == BST_CHECKED;
	// <== TBH: Backup [TBH/EastShare/MorphXT] - Stulle

	// ==> Design Settings [eWombat/Stulle] - Stulle
	for (int i=0;i<style_c_count;i++)
		thePrefs.SetStyle(client_styles, i, &nClientStyles[i]);

	for (int i=0;i<style_d_count;i++)
		thePrefs.SetStyle(download_styles, i, &nDownloadStyles[i]);

	for (int i=0;i<style_s_count;i++)
		thePrefs.SetStyle(share_styles, i, &nShareStyles[i]);

	for (int i=0;i<style_se_count;i++)
		thePrefs.SetStyle(server_styles, i, &nServerStyles[i]);

	for (int i=0;i<style_b_count;i++)
		thePrefs.SetStyle(background_styles, i, &nBackgroundStyles[i]);

	for (int i=0;i<style_w_count;i++)
		thePrefs.SetStyle(window_styles, i, &nWindowStyles[i]);

	// ==> Feedback personalization [Stulle] - Stulle
	for (int i=0;i<style_f_count;i++)
		thePrefs.SetStyle(feedback_styles, i, &nFeedBackStyles[i]);
	// <== Feedback personalization [Stulle] - Stulle

	if(m_bDesignChanged)
	{
		m_bDesignChanged = false;
                
		theApp.emuledlg->kademliawnd->OnBackcolor();
		theApp.emuledlg->kademliawnd->Localize();
		theApp.emuledlg->serverwnd->OnBackcolor();
		theApp.emuledlg->serverwnd->Localize();
		theApp.emuledlg->transferwnd->OnBackcolor();
		theApp.emuledlg->transferwnd->Localize();
		theApp.emuledlg->searchwnd->OnBackcolor();
		theApp.emuledlg->searchwnd->Localize();
		theApp.emuledlg->sharedfileswnd->OnBackcolor();
		theApp.emuledlg->sharedfileswnd->Localize();
		theApp.emuledlg->chatwnd->OnBackcolor();
		theApp.emuledlg->chatwnd->Localize();
		theApp.emuledlg->ircwnd->OnBackcolor();
		theApp.emuledlg->ircwnd->Localize();
		theApp.emuledlg->statisticswnd->OnBackcolor();
		theApp.emuledlg->statisticswnd->Localize();

		theApp.emuledlg->statusbar->UpdateColor();
		theApp.emuledlg->RedrawWindow();
		theApp.emuledlg->toolbar->UpdateBackground();
		theApp.emuledlg->toolbar->RedrawWindow();

		theApp.emuledlg->activewnd->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);//redraw only the active Wnd after color change - Max
	}
	// <== Design Settings [eWombat/Stulle] - Stulle

	// ==> Advanced Options [Official/MorphXT] - Stulle
	thePrefs.bMiniMuleAutoClose=bMiniMuleAutoClose;
	thePrefs.iMiniMuleTransparency=iMiniMuleTransparency;

	thePrefs.sMediaInfo_MediaInfoDllPath=sMediaInfo_MediaInfoDllPath;
	thePrefs.bMediaInfo_RIFF=bMediaInfo_RIFF;
	thePrefs.bMediaInfo_ID3LIB=bMediaInfo_ID3LIB;
#ifdef HAVE_QEDIT_H
	thePrefs.m_bMediaInfo_MediaDet=m_bMediaInfo_MediaDet;
#endif//HAVE_QEDIT_H
	thePrefs.m_bMediaInfo_RM=m_bMediaInfo_RM;
#ifdef HAVE_WMSDK_H
	thePrefs.m_bMediaInfo_WM=m_bMediaInfo_WM;
#endif//HAVE_WMSDK_H

	thePrefs.m_bRestoreLastMainWndDlg=m_bRestoreLastMainWndDlg;
	thePrefs.m_bRestoreLastLogPane=m_bRestoreLastLogPane;
	thePrefs.m_iStraightWindowStyles=m_iStraightWindowStyles;
	thePrefs.m_bRTLWindowsLayout=m_bRTLWindowsLayout;
	thePrefs.m_iMaxChatHistory=m_iMaxChatHistory;
	thePrefs.maxmsgsessions=m_umaxmsgsessions;
	thePrefs.m_strDateTimeFormat = m_strDateTimeFormat;
	thePrefs.m_strDateTimeFormat4Lists = m_strDateTimeFormat4Lists;
	thePrefs.m_bShowVerticalHourMarkers = m_bShowVerticalHourMarkers;
	thePrefs.m_bReBarToolbar = m_bReBarToolbar;
	thePrefs.m_bIconflashOnNewMessage = m_bIconflashOnNewMessage;
	thePrefs.m_bShowCopyEd2kLinkCmd = m_bShowCopyEd2kLinkCmd;
	thePrefs.m_bupdatequeuelist = m_bUpdateQueue;
	thePrefs.dontRecreateGraphs = m_bRepaint;
	thePrefs.m_bExtraPreviewWithMenu = m_bExtraPreviewWithMenu;
	thePrefs.m_bShowUpDownIconInTaskbar = m_bShowUpDownIconInTaskbar;
	thePrefs.m_bForceSpeedsToKB = m_bForceSpeedsToKB;

	thePrefs.m_iLogFileFormat=(ELogFileFormat)m_iLogFileFormat;
	thePrefs.iMaxLogBuff=iMaxLogBuff*1024;
	thePrefs.m_strDateTimeFormat4Log = m_strDateTimeFormat4Log;
	thePrefs.m_crLogError = m_crLogError;
	thePrefs.m_crLogWarning = m_crLogWarning;
	thePrefs.m_crLogSuccess = m_crLogSuccess;

	thePrefs.bCheckComctl32 =bCheckComctl32;
	thePrefs.bCheckShell32=bCheckShell32;
	thePrefs.bIgnoreInstances=bIgnoreInstances;
	thePrefs.sNotifierMailEncryptCertName=sNotifierMailEncryptCertName;
	thePrefs.m_iPreviewSmallBlocks=m_iPreviewSmallBlocks;
	thePrefs.m_bPreviewCopiedArchives=m_bPreviewCopiedArchives;
	thePrefs.m_bPreviewOnIconDblClk=m_bPreviewOnIconDblClk;
	thePrefs.sInternetSecurityZone=sInternetSecurityZone;
	thePrefs.m_strTxtEditor=sTxtEditor;
	thePrefs.nServerUDPPort=(uint16) iServerUDPPort; 
	thePrefs.m_bRemove2bin=m_bRemoveFilesToBin;
	thePrefs.m_bHighresTimer=m_bHighresTimer;
	thePrefs.m_bTrustEveryHash=m_bTrustEveryHash;
	thePrefs.m_iInspectAllFileTypes=m_iInspectAllFileTypes;
	thePrefs.m_bPreferRestrictedOverUser=m_bPreferRestrictedOverUser;
	thePrefs.m_bUseUserSortedServerList=m_bUseUserSortedServerList;
	thePrefs.m_iWebFileUploadSizeLimitMB=m_iWebFileUploadSizeLimitMB;
	int iPos = 0;
	CString strIP = m_sAllowedIPs.Tokenize(L";", iPos);
	thePrefs.m_aAllowedRemoteAccessIPs.RemoveAll();
	while (!strIP.IsEmpty())
	{
		u_long nIP = inet_addr(CStringA(strIP));
		if (nIP != INADDR_ANY && nIP != INADDR_NONE)
			thePrefs.m_aAllowedRemoteAccessIPs.Add(nIP);
		strIP = m_sAllowedIPs.Tokenize(L";", iPos);
	}
	thePrefs.m_iDebugSearchResultDetailLevel=m_iDebugSearchResultDetailLevel;
	thePrefs.m_bAdjustNTFSDaylightFileTime = m_bAdjustNTFSDaylightFileTime;
	thePrefs.dontcompressavi = m_dontcompressavi;
	thePrefs.ICH = m_ICH;
	if ( m_dwBindAddr == 0) //0.0.0.0
		thePrefs.SetBindAddr(NULL);
	else
	    thePrefs.SetBindAddr(ipstr(ntohl(m_dwBindAddr)));
	thePrefs.m_uFileBufferTimeLimit = SEC2MS(m_iFileBufferTimeLimit);
	thePrefs.m_bRearrangeKadSearchKeywords = m_bRearrangeKadSearchKeywords;
	thePrefs.beepOnError = m_bBeeper;
	thePrefs.msgsecure = m_bMsgOnlySec;
	thePrefs.m_bPeerCacheEnabled = !m_bDisablePeerCache;
	thePrefs.m_bKeepUnavailableFixedSharedDirs = m_bKeepUnavailableFixedSharedDirs;
	// <== Advanced Options [Official/MorphXT] - Stulle

	// ==> Advanced Updates [MorphXT/Stulle] - Stulle
	thePrefs.m_bAutoUpdateAntiLeech = m_AntiLeechStart.GetCheck() == BST_CHECKED;
	theApp.scheduler->SetWeekly(ACTION_UPDANTILEECH, m_AntiLeechWeek.GetCheck() == BST_CHECKED);
	m_AntiLeechURL.GetWindowText(thePrefs.m_strAntiLeechURL);

	thePrefs.m_bautoupdateipfilter = m_IpFilterStart.GetCheck() == BST_CHECKED;
	theApp.scheduler->SetWeekly(ACTION_UPDIPCONF, m_IpFilterWeek.GetCheck() == BST_CHECKED);
	m_IpFilterURL.GetWindowText(thePrefs.m_strautoupdateipfilter_url);

	m_CountryURL.GetWindowText(thePrefs.UpdateURLIP2Country);
	thePrefs.AutoUpdateIP2Country = m_CountryStart.GetCheck() == BST_CHECKED;
	// <== Advanced Updates [MorphXT/Stulle] - Stulle

	LoadSettings();

	// ==> Show sources on title - Stulle
	TCHAR buffer[510];

	if (!thePrefs.ShowRatesOnTitle() && !thePrefs.ShowSrcOnTitle()) {
		// ==> ModID [itsonlyme/SiRoB] - Stulle
		/*
		_stprintf(buffer,_T("eMule v%s"),theApp.m_strCurVersionLong + _T(" ") + MOD_VERSION); // Xman // Maella -Support for tag ET_MOD_VERSION 0x55
		*/
		// <== ModID [itsonlyme/SiRoB] - Stulle
		_stprintf(buffer,_T("eMule v%s [%s]"),theApp.m_strCurVersionLong,theApp.m_strModLongVersion);
		theApp.emuledlg->SetWindowText(buffer);
	}
	// <== Show sources on title - Stulle

	SetModified(FALSE);

	return CPropertyPage::OnApply();
}

void CPPgScar::Localize(void)
{	
	if(m_hWnd)
	{
		// ==> Tabbed Preferences [TPT] - Stulle
		/*
		GetDlgItem(IDC_WARNING)->SetWindowText(GetResString(IDS_TWEAKS_WARNING));
		*/
		int row = m_tabCtr.GetRowCount();
		InitTab(); // To update string, could be improved
		if(row != 0 && row != m_tabCtr.GetRowCount()){
			// Shift all windows object
			// .. to do
		}

		m_strWarning.SetWindowText(GetResString(IDS_TWEAKS_WARNING));
		// <== Tabbed Preferences [TPT] - Stulle

		// ==> push small files [sivka] - Stulle
		if (m_htiPush) m_ctrlTreeOptions.SetItemText(m_htiPush, GetResString(IDS_PUSH));
		if (m_htiEnablePushSmallFile) m_ctrlTreeOptions.SetItemText(m_htiEnablePushSmallFile, GetResString(IDS_PUSH_SMALL));
		if (m_htiPushSmallFileBoost) m_ctrlTreeOptions.SetEditLabel(m_htiPushSmallFileBoost, GetResString(IDS_PUSH_SMALL_BOOST));
		GetDlgItem(IDC_PUSHSMALL_LABEL)->SetWindowText(GetResString(IDS_PUSH_SMALL));
		// <== push small files [sivka] - Stulle
		if (m_htiEnablePushRareFile) m_ctrlTreeOptions.SetItemText(m_htiEnablePushRareFile, GetResString(IDS_PUSH_RARE)); // push rare file - Stulle

		// ==> FunnyNick [SiRoB/Stulle] - Stulle
		if (m_htiFnTag) m_ctrlTreeOptions.SetItemText(m_htiFnTag, GetResString(IDS_FUNNYNICK));
		if (m_htiFnActive) m_ctrlTreeOptions.SetItemText(m_htiFnActive,GetResString(IDS_ENABLE));
		if (m_htiFnTagMode) m_ctrlTreeOptions.SetItemText(m_htiFnTagMode, GetResString(IDS_FN_TAG));
		if (m_htiNoTag) m_ctrlTreeOptions.SetItemText(m_htiNoTag, GetResString(IDS_NO_TAG));
		if (m_htiShortTag) m_ctrlTreeOptions.SetItemText(m_htiShortTag, GetResString(IDS_SHORT_TAG));
		if (m_htiFullTag) m_ctrlTreeOptions.SetItemText(m_htiFullTag, GetResString(IDS_FULL_TAG));
		if (m_htiCustomTag) m_ctrlTreeOptions.SetItemText(m_htiCustomTag, GetResString(IDS_CUSTOM_TAG));
		if (m_htiFnCustomTag) m_ctrlTreeOptions.SetEditLabel(m_htiFnCustomTag, GetResString(IDS_SET_CUSTOM_TAG));
		if (m_htiFnTagAtEnd) m_ctrlTreeOptions.SetItemText(m_htiFnTagAtEnd, GetResString(IDS_FN_TAG_AT_END));
		// <== FunnyNick [SiRoB/Stulle] - Stulle

		if (m_htiConTweaks) m_ctrlTreeOptions.SetItemText(m_htiConTweaks, GetResString(IDS_CON_TWEAKS));
		// ==> Quick start [TPT] - Max
		if (m_htiQuickStartGroup) m_ctrlTreeOptions.SetItemText(m_htiQuickStartGroup, GetResString(IDS_QUICK_START_GROUP));
		if (m_htiQuickStart) m_ctrlTreeOptions.SetItemText(m_htiQuickStart, GetResString(IDS_QUICK_START));
		if (m_htiQuickStartMaxTime) m_ctrlTreeOptions.SetEditLabel(m_htiQuickStartMaxTime, GetResString(IDS_QUICK_START_MAX_TIME));
		if (m_htiQuickStartMaxConnPerFive) m_ctrlTreeOptions.SetEditLabel(m_htiQuickStartMaxConnPerFive, GetResString(IDS_QUICK_START_MAX_CONN_PER_FIVE));
		if (m_htiQuickStartMaxConn) m_ctrlTreeOptions.SetEditLabel(m_htiQuickStartMaxConn, GetResString(IDS_QUICK_START_MAX_CONN));
		if (m_htiQuickStartMaxConnPerFiveBack) m_ctrlTreeOptions.SetEditLabel(m_htiQuickStartMaxConnPerFiveBack, GetResString(IDS_QUICK_START_MAX_CONN_PER_FIVE_BACK));
		if (m_htiQuickStartMaxConnBack) m_ctrlTreeOptions.SetEditLabel(m_htiQuickStartMaxConnBack, GetResString(IDS_QUICK_START_MAX_CONN_BACK));
		if (m_htiQuickStartAfterIPChange) m_ctrlTreeOptions.SetItemText(m_htiQuickStartAfterIPChange, GetResString(IDS_QUICK_START_AFTER_IP_CHANGE));
		// <== Quick start [TPT] - Max
		// ==> Enforce Ratio [Stulle] - Stulle
		if (m_htiRatioGroup) m_ctrlTreeOptions.SetItemText(m_htiRatioGroup, GetResString(IDS_RATIO_GROUP));
		if (m_htiEnforceRatio) m_ctrlTreeOptions.SetItemText(m_htiEnforceRatio, GetResString(IDS_ENFORCE_RATIO));
		if (m_htiRatioValue) m_ctrlTreeOptions.SetEditLabel(m_htiRatioValue, GetResString(IDS_RATIO_VALUE));
		// <== Enforce Ratio [Stulle] - Stulle
		// ==> Improved ICS-Firewall support [MoNKi] - Max
		if (m_htiICFSupport)
		{
			m_ctrlTreeOptions.SetItemText(m_htiICFSupportRoot, GetResString(IDS_ICF));
			m_ctrlTreeOptions.SetItemText(m_htiICFSupport, GetResString(IDS_ICFSUPPORT));
			m_ctrlTreeOptions.SetItemText(m_htiICFSupportClearAtEnd, GetResString(IDS_FO_PREF_STARTUP));
			m_ctrlTreeOptions.SetItemText(m_htiICFSupportServerUDP, GetResString(IDS_ICF_SERVERUDP));
		}
		// <== Improved ICS-Firewall support [MoNKi] - Max
		// ==> UPnP support [MoNKi] - leuk_he
		if (m_htiUPnPNatGroup) m_ctrlTreeOptions.SetItemText(m_htiUPnPNatGroup, GetResString(IDS_UPNP_GROUP));
		if (m_htiUPnPNat) m_ctrlTreeOptions.SetItemText(m_htiUPnPNat, GetResString(IDS_UPNP_ENABLE));
		if (m_htiUpnPNATwebservice) m_ctrlTreeOptions.SetItemText(m_htiUpnPNATwebservice, GetResString(IDS_UPNP_ENABLE_WEB));
		if (m_htiUpnpBinaddr) m_ctrlTreeOptions.SetEditLabel(m_htiUpnpBinaddr, GetResString(IDS_UPNPBINDADDR));
		// <== UPnP support [MoNKi] - leuk_he
		// ==> Random Ports [MoNKi] - Stulle
		if (m_htiRndGrp) m_ctrlTreeOptions.SetItemText(m_htiRndGrp, GetResString(IDS_RND_PORT_GROUP));
		if (m_htiRandomports) m_ctrlTreeOptions.SetItemText(m_htiRandomports, GetResString(IDS_ENABLED));
		if (m_htiRandomFirstPort) m_ctrlTreeOptions.SetEditLabel(m_htiRandomFirstPort, GetResString(IDS_RND_PORT_FIRST));
		if (m_htiRandomLastPort) m_ctrlTreeOptions.SetEditLabel(m_htiRandomLastPort, GetResString(IDS_RND_PORT_LAST));
		if (m_htiRandomPortsResetTime) m_ctrlTreeOptions.SetEditLabel(m_htiRandomPortsResetTime, GetResString(IDS_RND_PORT_RESET));
		// <== Random Ports [MoNKi] - Stulle
		// ==> Mephisto Upload - Mephisto
		if (m_htiMinSlots) m_ctrlTreeOptions.SetEditLabel(m_htiMinSlots, GetResString(IDS_MIN_SLOTS));
		if (m_htiNoNewSlotTimer) m_ctrlTreeOptions.SetEditLabel(m_htiNoNewSlotTimer, GetResString(IDS_NO_NEW_SLOT_TIMER));
		if (m_htiFullLoops) m_ctrlTreeOptions.SetEditLabel(m_htiFullLoops, GetResString(IDS_FULL_LOOPS));
		if (m_htiMonitorLoops) m_ctrlTreeOptions.SetEditLabel(m_htiMonitorLoops, GetResString(IDS_MONITOR_LOOPS));
		if (m_htiNotReachedBW) m_ctrlTreeOptions.SetEditLabel(m_htiNotReachedBW, GetResString(IDS_NOT_REACHED_BW));
		if (m_htiNoTrickleTimer) m_ctrlTreeOptions.SetEditLabel(m_htiNoTrickleTimer, GetResString(IDS_NO_TRICKLE_TIMER));
		if (m_htiMoveDownKB) m_ctrlTreeOptions.SetEditLabel(m_htiMoveDownKB, GetResString(IDS_MOVE_DOWN_KB));
		// <== Mephisto Upload - Mephisto
		// ==> Multiple Part Transfer [Stulle] - Mephisto
		if (m_htiChunksToFinish) m_ctrlTreeOptions.SetEditLabel(m_htiChunksToFinish, GetResString(IDS_CHUNKS_TO_FINISH));
		if (m_htiChunksToUpload) m_ctrlTreeOptions.SetEditLabel(m_htiChunksToUpload, GetResString(IDS_CHUNKS_TO_UPLOAD));
		// <== Multiple Part Transfer [Stulle] - Mephisto
		if (m_htiMaxUpMinutes) m_ctrlTreeOptions.SetEditLabel(m_htiMaxUpMinutes, GetResString(IDS_MAX_UP_MINUTES)); // Adjust max upload time [Stulle] - Mephisto
		if (m_htiReAskFileSrc) m_ctrlTreeOptions.SetEditLabel(m_htiReAskFileSrc, GetResString(IDS_REASK_FILE_SRC)); // Timer for ReAsk File Sources [Stulle] - Stulle
		if (m_htiACC) m_ctrlTreeOptions.SetItemText(m_htiACC, GetResString(IDS_ACC)); // ACC [Max/WiZaRd] - Max
		if (m_htiIgnoreThird) m_ctrlTreeOptions.SetItemText(m_htiIgnoreThird, GetResString(IDS_IGNORE_THIRD)); // Do not reserve 1/3 of your uploadlimit for emule [Stulle] - Stulle
		if (m_htiUlThres) m_ctrlTreeOptions.SetItemText(m_htiUlThres, GetResString(IDS_UL_THRES)); // Disable accepting only clients who asked within last 30min [Stulle] - Stulle
		// ==> Mephisto Upload - Mephisto
		/*
		if (m_htiMaxSlotSpeed) m_ctrlTreeOptions.SetItemText(m_htiMaxSlotSpeed, GetResString(IDS_MAX_SLOTSPEED)); // Alwasy maximize slot speed [Stulle] - Stulle
		*/
		// <== Mephisto Upload - Mephisto

		// ==> Anti Uploader Ban [Stulle] - Stulle
		if (m_htiAntiUploaderBanLimit) m_ctrlTreeOptions.SetEditLabel(m_htiAntiUploaderBanLimit, GetResString(IDS_UNBAN_UPLOADER));
		if (m_htiAntiCase1) m_ctrlTreeOptions.SetItemText(m_htiAntiCase1, GetResString(IDS_ANTI_CASE_1));
		if (m_htiAntiCase2) m_ctrlTreeOptions.SetItemText(m_htiAntiCase2, GetResString(IDS_ANTI_CASE_2));
		if (m_htiAntiCase3) m_ctrlTreeOptions.SetItemText(m_htiAntiCase3, GetResString(IDS_ANTI_CASE_3));
		// <== Anti Uploader Ban [Stulle] - Stulle

		// ==> CreditSystems [EastShare/ MorphXT] - Stulle
		if (m_htiCreditSystem) m_ctrlTreeOptions.SetItemText(m_htiCreditSystem, GetResString(IDS_CREDIT_SYSTEM));
		if (m_htiOfficialCredit) m_ctrlTreeOptions.SetItemText(m_htiOfficialCredit, GetResString(IDS_OFFICIAL_CREDIT));
		if (m_htiLovelaceCredit) m_ctrlTreeOptions.SetItemText(m_htiLovelaceCredit, GetResString(IDS_LOVELACE_CREDIT));
		if (m_htiRatioCredit) m_ctrlTreeOptions.SetItemText(m_htiRatioCredit, GetResString(IDS_RATIO_CREDIT));
		if (m_htiPawcioCredit) m_ctrlTreeOptions.SetItemText(m_htiPawcioCredit, GetResString(IDS_PAWCIO_CREDIT));
		if (m_htiESCredit) m_ctrlTreeOptions.SetItemText(m_htiESCredit, GetResString(IDS_EASTSHARE_CREDIT));
		if (m_htiSivkaCredit) m_ctrlTreeOptions.SetItemText(m_htiSivkaCredit, GetResString(IDS_SIVKA_CREDIT));
		if (m_htiSwatCredit) m_ctrlTreeOptions.SetItemText(m_htiSwatCredit, GetResString(IDS_SWAT_CREDIT));
		if (m_htiXmanCredit) m_ctrlTreeOptions.SetItemText(m_htiXmanCredit, GetResString(IDS_XMAN_CREDIT));
		if (m_htiTk4Credit) m_ctrlTreeOptions.SetItemText(m_htiTk4Credit, GetResString(IDS_TK4_CREDIT));
		if (m_htiZzulCredit) m_ctrlTreeOptions.SetItemText(m_htiZzulCredit, GetResString(IDS_ZZUL_CREDIT));
		// <== CreditSystems [EastShare/ MorphXT] - Stulle
		if (m_htiFineCS) m_ctrlTreeOptions.SetItemText(m_htiFineCS, GetResString(IDS_FINECS)); // Modified FineCS [CiccioBastardo/Stulle] - Stulle
		// ==> Pay Back First [AndCycle/SiRoB/Stulle] - Stulle
		if (m_htiIsPayBackFirst) m_ctrlTreeOptions.SetItemText(m_htiIsPayBackFirst, GetResString(IDS_PAYBACKFIRST));
		if (m_htiPayBackFirstLimit) m_ctrlTreeOptions.SetEditLabel(m_htiPayBackFirstLimit, GetResString(IDS_PAYBACKFIRSTLIMIT));
		if (m_htiIsPayBackFirst2) m_ctrlTreeOptions.SetItemText(m_htiIsPayBackFirst2, GetResString(IDS_PAYBACKFIRST2));
		if (m_htiPayBackFirstLimit2) m_ctrlTreeOptions.SetEditLabel(m_htiPayBackFirstLimit2, GetResString(IDS_PAYBACKFIRSTLIMIT2));
		// <== Pay Back First [AndCycle/SiRoB/Stulle] - Stulle

		if (m_htiDisplay) m_ctrlTreeOptions.SetItemText(m_htiDisplay, GetResString(IDS_PW_DISPLAY));
		// ==> CPU/MEM usage [$ick$/Stulle] - Max
		if (m_htiSysInfoGroup) m_ctrlTreeOptions.SetItemText(m_htiSysInfoGroup, GetResString(IDS_SYS_INFO_GROUP));
		if (m_htiSysInfo) m_ctrlTreeOptions.SetItemText(m_htiSysInfo, GetResString(IDS_ENABLED));
		if (m_htiSysInfoGlobal) m_ctrlTreeOptions.SetItemText(m_htiSysInfoGlobal, GetResString(IDS_SYS_INFO_GLOBAL));
		// <== CPU/MEM usage [$ick$/Stulle] - Max
		// ==> Invisible Mode [TPT/MoNKi] - Stulle
		if ( m_htiInvisibleModeRoot )
		{
			m_ctrlTreeOptions.SetItemText(m_htiInvisibleMode, GetResString(IDS_INVMODE));
			m_ctrlTreeOptions.SetItemText(m_htiInvisibleModeStart, GetResString(IDS_INVMODE_START));
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

			BOOL bCheck;
			if(m_ctrlTreeOptions.GetCheckBox(m_htiInvisibleMode, /*m_bInvisibleMode*/bCheck)){
				if(/*m_bInvisibleMode*/bCheck)
					m_ctrlTreeOptions.SetItemText(m_htiInvisibleModeRoot, GetResString(IDS_INVMODE_GROUP) + 
						_T(" (") + m_sInvisibleModeMod + _T(" + ") + m_sInvisibleModeKey + _T(")"));
				else
					m_ctrlTreeOptions.SetItemText(m_htiInvisibleModeRoot, GetResString(IDS_INVMODE_GROUP));
			}
		}
		// <== Invisible Mode [TPT/MoNKi] - Stulle
		if (m_htiShowSrcOnTitle) m_ctrlTreeOptions.SetItemText(m_htiShowSrcOnTitle, GetResString(IDS_SHOWSRCONTITLE)); // Show sources on title - Stulle
		if (m_htiShowGlobalHL) m_ctrlTreeOptions.SetItemText(m_htiShowGlobalHL, GetResString(IDS_SHOW_GLOBAL_HL)); // show global HL - Stulle
		if (m_htiShowFileHLconst) m_ctrlTreeOptions.SetItemText(m_htiShowFileHLconst, GetResString(IDS_SHOW_FILE_HL_CONST)); // show HL per file constantly - Stulle
		if (m_htiShowInMSN7) m_ctrlTreeOptions.SetItemText(m_htiShowInMSN7, GetResString(IDS_SHOWINMSN7)); // Show in MSN7 [TPT] - Stulle
		if (m_htiQueueProgressBar) m_ctrlTreeOptions.SetItemText(m_htiQueueProgressBar, GetResString(IDS_CLIENTQUEUEPROGRESSBAR)); // Client queue progress bar [Commander] - Stulle
		if (m_htiTrayComplete) m_ctrlTreeOptions.SetItemText(m_htiTrayComplete, GetResString(IDS_TRAY_COMPLETE)); // Completed in Tray [Stulle] - Stulle
		if (m_htiColorFeedback) m_ctrlTreeOptions.SetItemText(m_htiColorFeedback, GetResString(IDS_COLOR_FEEDBACK)); // Color Feedback -  MyTh88
		if (m_htiShowClientPercentage) m_ctrlTreeOptions.SetItemText(m_htiShowClientPercentage, GetResString(IDS_CLIENTPERCENTAGE)); // Show Client Percentage optional [Stulle] - Stulle
		if (m_htiFollowTheMajority) m_ctrlTreeOptions.SetItemText(m_htiFollowTheMajority, GetResString(IDS_FOLLOWTHEMAJORITY)); // Follow The Majority [AndCycle/Stulle] - Stulle
		if (m_htiShowSpeedMeter) m_ctrlTreeOptions.SetItemText(m_htiShowSpeedMeter, GetResString(IDS_SHOW_SPEED_METER)); // High resolution speedmeter on toolbar [eFMod/Stulle] - Myth88
		// ==> Static Tray Icon [MorphXT] - MyTh88
		if (m_htiStaticIcon) m_ctrlTreeOptions.SetItemText(m_htiStaticIcon, GetResString(IDS_STATIC_ICON));
		// <== Static Tray Icon [MorphXT] - MyTh88

		// ==> File Settings [sivka/Stulle] - Stulle
		if (m_htiFileDefaults) m_ctrlTreeOptions.SetItemText(m_htiFileDefaults, GetResString(IDS_FILE_DEFAULTS));
		if (m_htiAutoNNS) m_ctrlTreeOptions.SetItemText(m_htiAutoNNS, GetResString(IDS_AUTO_NNS));
		if (m_htiAutoNNSTimer) m_ctrlTreeOptions.SetEditLabel(m_htiAutoNNSTimer, GetResString(IDS_NNS_TIMERLABEL));
		if (m_htiAutoNNSLimit) m_ctrlTreeOptions.SetEditLabel(m_htiAutoNNSLimit, GetResString(IDS_REMOVENNSLIMITLABEL));
		if (m_htiAutoFQS) m_ctrlTreeOptions.SetItemText(m_htiAutoFQS, GetResString(IDS_AUTO_FQS));
		if (m_htiAutoFQSTimer) m_ctrlTreeOptions.SetEditLabel(m_htiAutoFQSTimer, GetResString(IDS_FQS_TIMERLABEL));
		if (m_htiAutoFQSLimit) m_ctrlTreeOptions.SetEditLabel(m_htiAutoFQSLimit, GetResString(IDS_REMOVEFQSLIMITLABEL));
		if (m_htiAutoQRS) m_ctrlTreeOptions.SetItemText(m_htiAutoQRS, GetResString(IDS_AUTO_QRS));
		if (m_htiAutoQRSTimer) m_ctrlTreeOptions.SetEditLabel(m_htiAutoQRSTimer, GetResString(IDS_HQRS_TIMERLABEL));
		if (m_htiAutoQRSMax) m_ctrlTreeOptions.SetEditLabel(m_htiAutoQRSMax, GetResString(IDS_REMOVEQRSLABEL));
		if (m_htiHQRSivka) m_ctrlTreeOptions.SetItemText(m_htiHQRSivka, GetResString(IDS_SIVKA_DROPPING));
		if (m_htiAutoQRSLimit) m_ctrlTreeOptions.SetEditLabel(m_htiAutoQRSLimit, GetResString(IDS_REMOVEQRSLIMITLABEL));
		// <== File Settings [sivka/Stulle] - Stulle

		// ==> TBH: minimule - Max
		if (m_htiMMGroup) m_ctrlTreeOptions.SetItemText(m_htiMMGroup, GetResString(IDS_MM_GROUP));
		if (m_htiShowMM) m_ctrlTreeOptions.SetItemText(m_htiShowMM, GetResString(IDS_MM_SHOW));
		if (m_htiMMLives) m_ctrlTreeOptions.SetItemText(m_htiMMLives, GetResString(IDS_MM_LIVES));
		if (m_htiMMUpdateTime) m_ctrlTreeOptions.SetEditLabel(m_htiMMUpdateTime, GetResString(IDS_MM_UPDATE_TIME));
		if (m_htiMMTrans) m_ctrlTreeOptions.SetEditLabel(m_htiMMTrans, GetResString(IDS_MM_TRANS));
		if (m_htiMMCompl) m_ctrlTreeOptions.SetItemText(m_htiMMCompl, GetResString(IDS_MM_COMPL));
		if (m_htiMMOpen) m_ctrlTreeOptions.SetItemText(m_htiMMOpen, GetResString(IDS_MM_OPEN));
		// <== TBH: minimule - Max

		// ==> Control download priority [tommy_gun/iONiX] - MyTh88
		if (m_htiAutoDownPrioGroup) m_ctrlTreeOptions.SetItemText(m_htiAutoDownPrioGroup, GetResString(IDS_AUTO_DOWN_GROUP));
		if (m_htiAutoDownPrioOff) m_ctrlTreeOptions.SetItemText(m_htiAutoDownPrioOff, GetResString(IDS_POWERSHARE_DISABLED));
		if (m_htiAutoDownPrioPerc) m_ctrlTreeOptions.SetItemText(m_htiAutoDownPrioPerc, GetResString(IDS_AUTO_DOWN_PERC));
		if (m_htiAutoDownPrioPercVal) m_ctrlTreeOptions.SetEditLabel(m_htiAutoDownPrioPercVal, GetResString(IDS_AUTO_DOWN_PERC_VAL));
		if (m_htiAutoDownPrioSize) m_ctrlTreeOptions.SetItemText(m_htiAutoDownPrioSize, GetResString(IDS_AUTO_DOWN_SIZE));
		if (m_htiAutoDownPrioSizeVal) m_ctrlTreeOptions.SetEditLabel(m_htiAutoDownPrioSizeVal, GetResString(IDS_AUTO_DOWN_SIZE_VAL));
		if (m_htiAutoDownPrioValGroup) m_ctrlTreeOptions.SetItemText(m_htiAutoDownPrioValGroup, GetResString(IDS_AUTO_DOWN_PRIO));
		if (m_htiAutoDownPrioLow) m_ctrlTreeOptions.SetItemText(m_htiAutoDownPrioLow, GetResString(IDS_PRIOLOW));
		if (m_htiAutoDownPrioNormal) m_ctrlTreeOptions.SetItemText(m_htiAutoDownPrioNormal, GetResString(IDS_PRIONORMAL));
		if (m_htiAutoDownPrioHigh) m_ctrlTreeOptions.SetItemText(m_htiAutoDownPrioHigh, GetResString(IDS_PRIOHIGH));
		// <== Control download priority [tommy_gun/iONiX] - MyTh88

		// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
		if (m_htiSCC) m_ctrlTreeOptions.SetItemText(m_htiSCC, GetResString(IDS_SCC));
		if (m_htiDlMode) m_ctrlTreeOptions.SetItemText(m_htiDlMode, GetResString(IDS_DL_MODE_DEFAULT));
		if (m_htiDlNone) m_ctrlTreeOptions.SetItemText(m_htiDlNone, GetResString(IDS_DL_NONE));
		if (m_htiDlAlph) m_ctrlTreeOptions.SetItemText(m_htiDlAlph, GetResString(IDS_DOWNLOAD_ALPHABETICAL));
		if (m_htiDlLP) m_ctrlTreeOptions.SetItemText(m_htiDlLP, GetResString(IDS_LP));
		if (m_htiShowCatNames) m_ctrlTreeOptions.SetItemText(m_htiShowCatNames, GetResString(IDS_CAT_SHOWCATNAME));
		if (m_htiSelectCat) m_ctrlTreeOptions.SetItemText(m_htiSelectCat, GetResString(IDS_CAT_SHOWSELCATDLG));
		if (m_htiUseAutoCat) m_ctrlTreeOptions.SetItemText(m_htiUseAutoCat, GetResString(IDS_CAT_USEAUTOCAT));
        if (m_htiUseActiveCat) m_ctrlTreeOptions.SetItemText(m_htiUseActiveCat, GetResString(IDS_CAT_USEACTIVE));
		if (m_htiAutoSetResOrder) m_ctrlTreeOptions.SetItemText(m_htiAutoSetResOrder, GetResString(IDS_CAT_AUTORESUMEORD));
		if (m_htiSmallFileDLPush) m_ctrlTreeOptions.SetItemText(m_htiSmallFileDLPush, GetResString(IDS_CAT_SMALLFILEDLPUSH));
		if (m_htiResumeFileInNewCat) m_ctrlTreeOptions.SetEditLabel(m_htiResumeFileInNewCat, GetResString(IDS_CAT_STARTFILESONADD));
		if (m_bAddRemovedInc) m_ctrlTreeOptions.SetItemText(m_htiAddRemovedInc, GetResString(IDS_ADD_REMOVED_INC));
		// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

		if (m_htiSharedPrefs) m_ctrlTreeOptions.SetItemText(m_htiSharedPrefs, GetResString(IDS_SHARED_PREFS));
		// ==> PowerShare [ZZ/MorphXT] - Stulle
		if (m_htiPowershareMode) m_ctrlTreeOptions.SetItemText(m_htiPowershareMode, GetResString(IDS_POWERSHARE));
		if (m_htiPowershareDisabled) m_ctrlTreeOptions.SetItemText(m_htiPowershareDisabled, GetResString(IDS_POWERSHARE_DISABLED));
		if (m_htiPowershareActivated) m_ctrlTreeOptions.SetItemText(m_htiPowershareActivated, GetResString(IDS_POWERSHARE_ACTIVATED));
		if (m_htiPowershareAuto) m_ctrlTreeOptions.SetItemText(m_htiPowershareAuto, GetResString(IDS_POWERSHARE_AUTO));
		if (m_htiPowershareLimited) m_ctrlTreeOptions.SetItemText(m_htiPowershareLimited, GetResString(IDS_POWERSHARE_LIMITED));
		if (m_htiPowerShareLimit) m_ctrlTreeOptions.SetEditLabel(m_htiPowerShareLimit, GetResString(IDS_POWERSHARE_LIMIT));
		// <== PowerShare [ZZ/MorphXT] - Stulle
		// ==> Limit PS by amount of data uploaded [Stulle] - Stulle
		if (m_htiPsAmountLimit) m_ctrlTreeOptions.SetEditLabel(m_htiPsAmountLimit, GetResString(IDS_PS_AMOUNT_LIMIT));
		// <== Limit PS by amount of data uploaded [Stulle] - Stulle
		// ==> Spread Credits Slot [Stulle] - Stulle
		if (m_htiSpreadCreditsSlotGroup) m_ctrlTreeOptions.SetItemText(m_htiSpreadCreditsSlotGroup, GetResString(IDS_SPREAD_CREDITS_SLOT));
		if (m_htiSpreadCreditsSlot) m_ctrlTreeOptions.SetItemText(m_htiSpreadCreditsSlot, GetResString(IDS_ENABLE));
		if (m_htiSpreadCreditsSlotCounter) m_ctrlTreeOptions.SetEditLabel(m_htiSpreadCreditsSlotCounter, GetResString(IDS_SPREAD_CREDITS_SLOT_COUNTER));
		// <== Spread Credits Slot [Stulle] - Stulle
		// ==> Release Bonus [sivka] - Stulle
		if (m_htiReleaseBonusGroup) m_ctrlTreeOptions.SetItemText(m_htiReleaseBonusGroup, GetResString(IDS_RELEASE_BONUS_GROUP));
		if (m_htiReleaseBonus0) m_ctrlTreeOptions.SetItemText(m_htiReleaseBonus0, GetResString(IDS_POWERSHARE_DISABLED));
		if (m_htiReleaseBonus1) m_ctrlTreeOptions.SetItemText(m_htiReleaseBonus1, GetResString(IDS_RELEASE_BONUS_12));
		if (m_htiReleaseBonusDays) m_ctrlTreeOptions.SetItemText(m_htiReleaseBonusDays, GetResString(IDS_DAYS2));
		if (m_htiReleaseBonusDaysEdit) m_ctrlTreeOptions.SetEditLabel(m_htiReleaseBonusDaysEdit, GetResString(IDS_RELEASE_BONUS_EDIT));
		// <== Release Bonus [sivka] - Stulle
		if (m_htiReleaseScoreAssurance) m_ctrlTreeOptions.SetItemText(m_htiReleaseScoreAssurance, GetResString(IDS_RELEASE_SCORE_ASSURANCE)); // Release Score Assurance [Stulle] - Stulle
		if (m_htiSpreadBars) m_ctrlTreeOptions.SetItemText(m_htiSpreadBars, GetResString(IDS_SPREAD_BARS)); // Spread bars [Slugfiller/MorphXT] - Stulle
		// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
		if (m_htiHideOS) m_ctrlTreeOptions.SetEditLabel(m_htiHideOS, GetResString(IDS_HIDEOVERSHARES));
		if (m_htiSelectiveShare) m_ctrlTreeOptions.SetItemText(m_htiSelectiveShare, GetResString(IDS_SELECTIVESHARE));
		if (m_htiShareOnlyTheNeed) m_ctrlTreeOptions.SetItemText(m_htiShareOnlyTheNeed, GetResString(IDS_SHAREONLYTHENEED));
		// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
		if (m_htiFairPlay) m_ctrlTreeOptions.SetEditLabel(m_htiFairPlay, GetResString(IDS_FAIR_PLAY)); // Fair Play [AndCycle/Stulle] - Stulle

		if (m_htiMisc) m_ctrlTreeOptions.SetItemText(m_htiMisc, GetResString(IDS_MISC));
		// ==> Global Source Limit [Max/Stulle] - Stulle
		if (m_htiGlobalHlGroup) m_ctrlTreeOptions.SetItemText(m_htiGlobalHlGroup, GetResString(IDS_GLOBAL_HL));
		if (m_htiGlobalHL) m_ctrlTreeOptions.SetItemText(m_htiGlobalHL, GetResString(IDS_ENABLE));
		if (m_htiGlobalHlLimit) m_ctrlTreeOptions.SetEditLabel(m_htiGlobalHlLimit, GetResString(IDS_GLOBAL_HL_LIMIT));
		if (m_htiGlobalHlAll) m_ctrlTreeOptions.SetItemText(m_htiGlobalHlAll, GetResString(IDS_GLOBAL_HL_ALL));
		if (m_htiGlobalHlDefault) m_ctrlTreeOptions.SetItemText(m_htiGlobalHlDefault, GetResString(IDS_GLOBAL_HL_DEFAULT));
		// <== Global Source Limit [Max/Stulle] - Stulle
		// ==> Emulate others [WiZaRd/Spike/shadow2004] - Stulle
		if (m_htiEmulatorGroup) m_ctrlTreeOptions.SetItemText(m_htiEmulatorGroup, GetResString(IDS_EMULATOR_GROUP));
		if (m_htiEmuMLDonkey) m_ctrlTreeOptions.SetItemText(m_htiEmuMLDonkey, GetResString(IDS_EMULATE_ML));
		if (m_htiEmueDonkey) m_ctrlTreeOptions.SetItemText(m_htiEmueDonkey, GetResString(IDS_EMULATE_DONK));
		if (m_htiEmueDonkeyHybrid) m_ctrlTreeOptions.SetItemText(m_htiEmueDonkeyHybrid, GetResString(IDS_EMULATE_DONK_HYB));
		if (m_htiEmuShareaza) m_ctrlTreeOptions.SetItemText(m_htiEmuShareaza, GetResString(IDS_EMULATE_SHA));
		if (m_htiEmuLphant) m_ctrlTreeOptions.SetItemText(m_htiEmuLphant, GetResString(IDS_EMULATE_PHANT));
		if (m_htiLogEmulator) m_ctrlTreeOptions.SetItemText(m_htiLogEmulator, GetResString(IDS_EMULATE_LOG));
		// <== Emulate others [WiZaRd/Spike/shadow2004] - Stulle
		// ==> Automatic shared files updater [MoNKi] - Stulle
		if (m_htiAutoSharedGroup) m_ctrlTreeOptions.SetItemText(m_htiAutoSharedGroup, GetResString(IDS_AUTO_SHARED_UPDATER));
		if (m_htiAutoSharedUpdater) m_ctrlTreeOptions.SetItemText(m_htiAutoSharedUpdater, GetResString(IDS_ENABLE));
		if (m_htiSingleSharedDirUpdater) m_ctrlTreeOptions.SetItemText(m_htiSingleSharedDirUpdater,GetResString(IDS_ASFU_SINGLE));
		if (m_htiTimeBetweenReloads) m_ctrlTreeOptions.SetEditLabel(m_htiTimeBetweenReloads, GetResString(IDS_ASFU_TIMEBETWEEN));
		// <== Automatic shared files updater [MoNKi] - Stulle
		if (m_htiSUQWT) m_ctrlTreeOptions.SetItemText(m_htiSUQWT, GetResString(IDS_SUQWT)); // SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
		if (m_htiStartupSound) m_ctrlTreeOptions.SetItemText(m_htiStartupSound, GetResString(IDS_STARTUPSOUND)); // Startupsound [Commander] - mav744
		if (m_htiCompressLevel) m_ctrlTreeOptions.SetEditLabel(m_htiCompressLevel, GetResString(IDS_COMPRESS_LVL)); // Adjust Compress Level [Stulle] - Stulle

		// ==> TBH: Backup [TBH/EastShare/MorphXT] - Stulle
		m_BackupBox.SetWindowText( GetResString(IDS_BACKUP_FILEFRAME) );
		m_SelectAll.SetWindowText( GetResString(IDS_BACKUP_SELECTALL) );
		m_BackupNow.SetWindowText( GetResString(IDS_BACKUP_BACKUPNOW) );
		m_AutoBackupBox.SetWindowText( GetResString(IDS_BACKUP_AUTOFRAME) );
		m_AutoBackup.SetWindowText( GetResString(IDS_BACKUP_AUTOBACKUP));
		m_AutoBackup2.SetWindowText( GetResString(IDS_BACKUP_AUTOBACKUP2));
		m_Note.SetWindowText( GetResString(IDS_BACKUP_NOTE));
		m_NoteText.SetWindowText( GetResString(IDS_BACKUP_MESSAGE) );
		// <== TBH: Backup [TBH/EastShare/MorphXT] - Stulle

		// ==> Design Settings [eWombat/Stulle] - Stulle
		m_ColorBox.SetWindowText( GetResString(IDS_COLOR_BOX) );
		m_FontColorLabel.SetWindowText( GetResString(IDS_COLOR_FONT_LABEL) );
		m_BackColorLabel.SetWindowText( GetResString(IDS_COLOR_BACK_LABEL) );
		m_ColorWarning.SetWindowText(GetResString(IDS_COLOR_WARNING));
		m_EasterEgg.SetWindowText(GetResString(IDS_EASTEREGG)); // Diabolic Easteregg [Stulle] - Mephisto

		m_FontColor.CustomText = GetResString(IDS_COL_MORECOLORS);
		m_FontColor.DefaultText = GetResString(IDS_DEFAULT);
		m_BackColor.CustomText = GetResString(IDS_COL_MORECOLORS);
		m_BackColor.DefaultText = GetResString(IDS_DEFAULT);

		InitMasterStyleCombo();
		// <== Design Settings [eWombat/Stulle] - Stulle

		// ==> Advanced Options [Official/MorphXT] - Stulle
		if (m_hti_AdvMiniMule) m_ctrlTreeOptions.SetItemText(m_hti_AdvMiniMule, GetResString(IDS_MINIMULE));
		if (m_hti_bMiniMuleAutoClose) m_ctrlAdvTreeOptions.SetItemText(m_hti_bMiniMuleAutoClose, GetResString(IDS_MINIMULEAUTOCLOSE));
		if (m_hti_iMiniMuleTransparency) m_ctrlAdvTreeOptions.SetEditLabel(m_hti_iMiniMuleTransparency, GetResString(IDS_MINIMULETRANSPARENCY));

		if (m_hti_MediaInfo) m_ctrlTreeOptions.SetItemText(m_hti_MediaInfo, GetResString(IDS_FILEINFO));
		if (m_hti_sMediaInfo_MediaInfoDllPath) m_ctrlAdvTreeOptions.SetEditLabel(m_hti_sMediaInfo_MediaInfoDllPath, GetResString(IDS_MEDIAINFO_MEDIAINFODLLPATH));
		if (m_hti_bMediaInfo_RIFF) m_ctrlAdvTreeOptions.SetItemText(m_hti_bMediaInfo_RIFF, GetResString(IDS_MEDIAINFO_RIFF));
		if (m_hti_bMediaInfo_ID3LIB) m_ctrlAdvTreeOptions.SetItemText(m_hti_bMediaInfo_ID3LIB, GetResString(IDS_MEDIAINFO_ID3LIB));
#ifdef HAVE_QEDIT_H
		if (m_hti_MediaInfo_MediaDet) m_ctrlAdvTreeOptions.SetItemText(m_hti_MediaInfo_MediaDet, GetResString(IDS_MEDIAINFO_MEDIADET));
#endif//HAVE_QEDIT_H
		if (m_hti_MediaInfo_RM) m_ctrlAdvTreeOptions.SetItemText(m_hti_MediaInfo_RM, GetResString(IDS_MEDIAINFO_RM));
#ifdef HAVE_WMSDK_H
		if (m_hti_MediaInfo_WM) m_ctrlAdvTreeOptions.SetItemText(m_hti_MediaInfo_WM, GetResString(IDS_MEDIAINFO_WM));
#endif//HAVE_WMSDK_H

		if (m_hti_AdvDisplay) m_ctrlTreeOptions.SetItemText(m_hti_AdvDisplay, GetResString(IDS_PW_DISPLAY));
		if (m_hti_m_bRestoreLastMainWndDlg) m_ctrlAdvTreeOptions.SetItemText(m_hti_m_bRestoreLastMainWndDlg, GetResString(IDS_RESTORELASTMAINWNDDLG));
		if (m_hti_m_bRestoreLastLogPane) m_ctrlAdvTreeOptions.SetItemText(m_hti_m_bRestoreLastLogPane, GetResString(IDS_RESTORELASTLOGPANE));
		if (m_hti_m_iStraightWindowStyles) m_ctrlAdvTreeOptions.SetEditLabel(m_hti_m_iStraightWindowStyles, GetResString(IDS_STRAIGHTWINDOWSTYLES));
		if (m_hti_m_bRTLWindowsLayout) m_ctrlAdvTreeOptions.SetItemText(m_hti_m_bRTLWindowsLayout, GetResString(IDS_RTLWINDOWSLAYOUT));
		if (m_hti_m_iMaxChatHistory) m_ctrlAdvTreeOptions.SetEditLabel(m_hti_m_iMaxChatHistory, GetResString(IDS_MAXCHATHISTORY));
		if (m_hti_maxmsgsessions) m_ctrlAdvTreeOptions.SetEditLabel(m_hti_maxmsgsessions, GetResString(IDS_MAXMSGSESSIONS));
		if (m_htidatetimeformat) m_ctrlAdvTreeOptions.SetEditLabel(m_htidatetimeformat, GetResString(IDS_X_DATETIMEFORMAT));
		if (m_htidatetimeformat4lists) m_ctrlAdvTreeOptions.SetEditLabel(m_htidatetimeformat4lists, GetResString(IDS_X_DATETIMEFORMAT4LIST));
		if (m_htiShowVerticalHourMarkers) m_ctrlAdvTreeOptions.SetItemText(m_htiShowVerticalHourMarkers, GetResString(IDS_X_SHOWVERTICALHOURMARKERS));
		if (m_htiReBarToolbar) m_ctrlAdvTreeOptions.SetItemText(m_htiReBarToolbar, GetResString(IDS_X_REBARTOOLBAR));
		if (m_htiIconflashOnNewMessage) m_ctrlAdvTreeOptions.SetItemText(m_htiIconflashOnNewMessage, GetResString(IDS_X_ICON_FLASH_ON_NEW_MESSAGE));
		if (m_htiShowCopyEd2kLinkCmd) m_ctrlAdvTreeOptions.SetItemText(m_htiShowCopyEd2kLinkCmd, GetResString(IDS_X_SHOWCOPYED2KLINK));
		if (m_htiUpdateQueue) m_ctrlAdvTreeOptions.SetItemText(m_htiUpdateQueue, GetResString(IDS_UPDATEQUEUE));
		if (m_htiRepaint) m_ctrlAdvTreeOptions.SetItemText(m_htiRepaint, GetResString(IDS_REPAINTGRAPHS));
		if (m_htiExtraPreviewWithMenu) m_ctrlAdvTreeOptions.SetItemText(m_htiExtraPreviewWithMenu, GetResString(IDS_EXTRAPREVIEWWITHMENU));
		if (m_htiShowUpDownIconInTaskbar) m_ctrlAdvTreeOptions.SetItemText(m_htiShowUpDownIconInTaskbar, GetResString(IDS_SHOWUPDOWNICONINTASKBAR));
		if (m_htiForceSpeedsToKB) m_ctrlAdvTreeOptions.SetItemText(m_htiForceSpeedsToKB, GetResString(IDS_FORCESPEEDSTOKB));

		if (m_hti_Log) m_ctrlTreeOptions.SetItemText(m_hti_Log, GetResString(IDS_SV_LOG));
		if (m_hti_m_iLogFileFormat) m_ctrlAdvTreeOptions.SetItemText(m_hti_m_iLogFileFormat, GetResString(IDS_LOGFILEFORMAT));
		if (m_hti_iMaxLogBuff) m_ctrlAdvTreeOptions.SetEditLabel(m_hti_iMaxLogBuff, GetResString(IDS_MAXLOGBUFF));
		if (m_htidatetimeformat4log) m_ctrlAdvTreeOptions.SetEditLabel(m_htidatetimeformat4log, GetResString(IDS_X_DATETIMEFORMAT4LOG));
		if (m_htiLogError) m_ctrlAdvTreeOptions.SetEditLabel(m_htiLogError, GetResString(IDS_X_LOGWARNING));
		if (m_htiLogWarning) m_ctrlAdvTreeOptions.SetEditLabel(m_htiLogWarning, GetResString(IDS_X_LOGSUCCESS));
		if (m_htiLogSuccess) m_ctrlAdvTreeOptions.SetEditLabel(m_htiLogSuccess, GetResString(IDS_X_LOGERROR));

		if (m_hti_bCheckComctl32) m_ctrlAdvTreeOptions.SetItemText(m_hti_bCheckComctl32, GetResString(IDS_CHECKCOMCTL32));
		if (m_hti_bCheckShell32) m_ctrlAdvTreeOptions.SetItemText(m_hti_bCheckShell32, GetResString(IDS_CHECKSHELL32));
		if (m_hti_bIgnoreInstances) m_ctrlAdvTreeOptions.SetItemText(m_hti_bIgnoreInstances, GetResString(IDS_IGNOREINSTANCES));
		if (m_hti_sNotifierMailEncryptCertName) m_ctrlAdvTreeOptions.SetEditLabel(m_hti_sNotifierMailEncryptCertName, GetResString(IDS_NOTIFIERMAILENCRYPTCERTNAME));
		if (m_hti_m_iPreviewSmallBlocks) m_ctrlAdvTreeOptions.SetItemText(m_hti_m_iPreviewSmallBlocks, GetResString(IDS_PREVIEWSMALLBLOCKS));
		if (m_hti_m_bPreviewCopiedArchives) m_ctrlAdvTreeOptions.SetItemText(m_hti_m_bPreviewCopiedArchives, GetResString(IDS_PREVIEWCOPIEDARCHIVES));
		if (m_hti_m_bPreviewOnIconDblClk) m_ctrlAdvTreeOptions.SetItemText(m_hti_m_bPreviewOnIconDblClk, GetResString(IDS_PREVIEWONICONDBLCLK));
		if (m_hti_sInternetSecurityZone) m_ctrlAdvTreeOptions.SetEditLabel(m_hti_sInternetSecurityZone, GetResString(IDS_INTERNETSECURITYZONE));
		if (m_hti_sTxtEditor) m_ctrlAdvTreeOptions.SetEditLabel(m_hti_sTxtEditor, GetResString(IDS_TXTEDITOR));
		if (m_hti_iServerUDPPort) m_ctrlAdvTreeOptions.SetEditLabel(m_hti_iServerUDPPort, GetResString(IDS_SERVERUDPPORT));
		if (m_hti_m_bRemoveFilesToBin) m_ctrlAdvTreeOptions.SetItemText(m_hti_m_bRemoveFilesToBin, GetResString(IDS_REMOVEFILESTOBIN));
		if (m_hti_HighresTimer) m_ctrlAdvTreeOptions.SetItemText(m_hti_HighresTimer, GetResString(IDS_HIGHRESTIMER));
		if (m_hti_TrustEveryHash) m_ctrlAdvTreeOptions.SetItemText(m_hti_TrustEveryHash, GetResString(IDS_TRUSTEVERYHASH));
		if (m_hti_InspectAllFileTypes) m_ctrlAdvTreeOptions.SetItemText(m_hti_InspectAllFileTypes, GetResString(IDS_INSPECTALLFILETYPES));
		if (m_hti_PreferRestrictedOverUser) m_ctrlAdvTreeOptions.SetItemText(m_hti_PreferRestrictedOverUser, GetResString(IDS_PREFERRESTRICTEDOVERUSER));
		if (m_hti_WebFileUploadSizeLimitMB) m_ctrlAdvTreeOptions.SetEditLabel(m_hti_WebFileUploadSizeLimitMB, GetResString(IDS_WEBFILEUPLOADSIZELIMITMB));
		if (m_hti_AllowedIPs) m_ctrlAdvTreeOptions.SetEditLabel(m_hti_AllowedIPs, GetResString(IDS_ALLOWEDIPS));
		if (m_hti_UseUserSortedServerList) m_ctrlAdvTreeOptions.SetItemText(m_hti_UseUserSortedServerList, GetResString(IDS_USEUSERSORTEDSERVERLIST));
		if (m_hti_DebugSearchResultDetailLevel) m_ctrlAdvTreeOptions.SetEditLabel(m_hti_DebugSearchResultDetailLevel, GetResString(IDS_DEBUGSEARCHDETAILLEVEL));
		if (m_htiAdjustNTFSDaylightFileTime) m_ctrlAdvTreeOptions.SetItemText(m_htiAdjustNTFSDaylightFileTime, GetResString(IDS_X_ADJUSTNTFSDAYLIGHTFILETIME));
		if (m_htidontcompressavi) m_ctrlAdvTreeOptions.SetItemText(m_htidontcompressavi, GetResString(IDS_X_DONTCOMPRESSAVI));
		if (m_htiICH) m_ctrlAdvTreeOptions.SetItemText(m_htiICH, GetResString(IDS_X_ICH));
		if (m_htiBindAddr) m_ctrlAdvTreeOptions.SetEditLabel(m_htiBindAddr, GetResString(IDS_BINDADDR));
		if (m_htiFileBufferTimeLimit) m_ctrlAdvTreeOptions.SetEditLabel(m_htiFileBufferTimeLimit, GetResString(IDS_X_FILE_BUFFER_TIME_LIMIT));
		if (m_htiRearrangeKadSearchKeywords) m_ctrlAdvTreeOptions.SetItemText(m_htiRearrangeKadSearchKeywords, GetResString(IDS_X_REARRANGEKADSEARCH));
		if (m_htiBeeper) m_ctrlAdvTreeOptions.SetItemText(m_htiBeeper, GetResString(IDS_PW_BEEP));
		if (m_htiMsgOnlySec) m_ctrlAdvTreeOptions.SetItemText(m_htiMsgOnlySec, GetResString(IDS_MSGONLYSEC));
		if (m_htiDisablePeerCache) m_ctrlAdvTreeOptions.SetItemText(m_htiDisablePeerCache, GetResString(IDS_DISABLEPEERACHE));
		if (m_htiKeepUnavailableFixedSharedDirs) m_ctrlAdvTreeOptions.SetItemText(m_htiKeepUnavailableFixedSharedDirs, GetResString(IDS_KEEPUNAVAILABLEFIXEDSHAREDDIRS));
		// <== Advanced Options [Official/MorphXT] - Stulle

		// ==> Advanced Updates [MorphXT/Stulle] - Stulle
		m_AntiLeechStart.SetWindowText(GetResString(IDS_UPDATEANTILEECHERONSTART));
		m_AntiLeechWeek.SetWindowText(GetResString(IDS_AUTOUPWEEK));
		m_AntiLeechURLStatic.SetWindowText(GetResString(IDS_URL_FOR_UPDATING));
		m_AntiLeechReset.SetWindowText(GetResString(IDS_RESET));
		m_AntiLeechUpdate.SetWindowText(GetResString(IDS_UPDATEIPCURL));
		CString strBuffer=NULL;
		if(theApp.dlp->IsDLPavailable())
			strBuffer.Format(_T("v%u"),theApp.dlp->GetDLPVersion());
		else
			strBuffer=GetResString(IDS_DL_NONE);
		m_AntiLeechVersion.SetWindowText(strBuffer);

		if(thePrefs.IsIPFilterViaDynDNS())
		{
			if(PathFileExists(theApp.ipfilter->GetDefaultFilePath()))
				strBuffer.Format(_T("v%u"), thePrefs.GetIPFilterVersionNum());
			else
				strBuffer=GetResString(IDS_DL_NONE);
			m_IpFilterTime.SetWindowText(strBuffer);
		}
		else
		{
			TCHAR sTime[30];
			sTime[0] = _T('\0');
			SysTimeToStr(thePrefs.GetIPfilterVersion(), sTime);
			m_IpFilterTime.SetWindowText(sTime);
		}

		m_IpFilterBox.SetWindowText(GetResString(IDS_IPFILTER));
		m_IpFilterStart.SetWindowText(GetResString(IDS_UPDATEIPFILTERONSTART));
		m_IpFilterWeek.SetWindowText(GetResString(IDS_AUTOUPWEEK));
		m_IpFilterURLStatic.SetWindowText(GetResString(IDS_URL_FOR_UPDATING));
		m_IpFilterReset.SetWindowText(GetResString(IDS_RESET));
		m_IpFilterUpdate.SetWindowText(GetResString(IDS_UPDATEIPCURL));

		m_CountryBox.SetWindowText(GetResString(IDS_COUNTRY));
		m_CountryStart.SetWindowText(GetResString(IDS_AUTOUPIP2COUNTRY));
		m_CountryURLStatic.SetWindowText(GetResString(IDS_URL_FOR_UPDATING));
		m_CountryReset.SetWindowText(GetResString(IDS_RESET));
		m_CountryUpdate.SetWindowText(GetResString(IDS_UPDATEIPCURL));
		// <== Advanced Updates [MorphXT/Stulle] - Stulle

		// ==> Support page in preferences [Stulle] - Stulle
		m_HpLink.Clear();
		m_HpLink.AppendText(GetResString(IDS_HOMEPAGE)+_T(": "));
		// ==> Mephisto mod [Stulle] - Mephisto
		/*
		m_HpLink.AppendHyperLink(_T("eMule ScarAngel Mod"),0,CString(_T("http://scarangel.sourceforge.net/")),0,0);
		*/
		m_HpLink.AppendHyperLink(_T("eMule Mephisto Mod"),0,CString(_T("http://mephisto.emule-web.de/")),0,0);
		// <== Mephisto mod [Stulle] - Mephisto
		m_BoardGerLink.Clear();
		m_BoardGerLink.AppendText(GetResString(IDS_BOARD_GER)+_T(": "));
		m_BoardGerLink.AppendHyperLink(_T("eMule-web.de"),0,CString(_T("http://www.emule-web.de/board/")),0,0);
		m_BoardEngLink.Clear();
		m_BoardEngLink.AppendText(GetResString(IDS_BOARD_ENG)+_T(": "));
		m_BoardEngLink.AppendHyperLink(_T("eMule-project.net"),0,CString(_T("http://forum.emule-project.net/index.php?showforum=97")),0,0);
		m_RateLink.Clear();
		// ==> Mephisto mod [Stulle] - Mephisto
		/*
		m_RateLink.AppendText(GetResString(IDS_RATE)+_T(": "));
		m_RateLink.AppendHyperLink(_T("eMule-mods.de"),0,CString(_T("http://www.emule-mods.de/?comment=ScarAngel")),0,0);
		*/
		m_RateLink.AppendText(GetResString(IDS_RATE_MEPH)+_T(": "));
		m_RateLink.AppendHyperLink(_T("eMule-mods.de"),0,CString(_T("http://www.emule-mods.de/?comment=Mephisto")),0,0);
		// <== Mephisto mod [Stulle] - Mephisto
		/*
		m_XtremeLink.Clear();
		m_XtremeLink.AppendText(GetResString(IDS_XTREME)+_T(": "));
		m_XtremeLink.AppendHyperLink(_T("eMule Xtreme Mod official homepage"),0,CString(_T("http://www.xtreme-mod.net/")),0,0);
		*/

		m_HpLink.ShowWindow(SW_HIDE);
		m_HpLink.EnableWindow(FALSE);
		m_BoardGerLink.ShowWindow(SW_HIDE);
		m_BoardGerLink.EnableWindow(FALSE);
		m_BoardEngLink.ShowWindow(SW_HIDE);
		m_BoardEngLink.EnableWindow(FALSE);
		m_RateLink.ShowWindow(SW_HIDE);
		m_RateLink.EnableWindow(FALSE);
		/*
		m_XtremeLink.ShowWindow(SW_HIDE);
		m_XtremeLink.EnableWindow(FALSE);
		*/
		// <== Support page in preferences [Stulle] - Stulle
	}

}

void CPPgScar::OnDestroy()
{
	m_ctrlTreeOptions.DeleteAllItems();
	m_ctrlTreeOptions.DestroyWindow();
	m_bInitializedTreeOpts = false;
	// ==> Advanced Options [Official/MorphXT] - Stulle
	m_ctrlAdvTreeOptions.DeleteAllItems();
	m_ctrlAdvTreeOptions.DestroyWindow();
	m_bInitializedAdvTreeOpts = false;
	// <== Advanced Options [Official/MorphXT] - Stulle
	m_htiPush = NULL; // push files - Stulle
	// ==> push small files [sivka] - Stulle
	m_htiEnablePushSmallFile = NULL;
	m_htiPushSmallFileBoost = NULL;
	// <== push small files [sivka] - Stulle
	m_htiEnablePushRareFile = NULL; // push rare file - Stulle

	// ==> FunnyNick [SiRoB/Stulle] - Stulle
	m_htiFnActive = NULL;
	m_htiFnTag = NULL;
	m_htiNoTag = NULL;
	m_htiShortTag = NULL;
	m_htiFullTag = NULL;
	m_htiCustomTag = NULL;
	m_htiFnCustomTag = NULL;
	m_htiFnTagAtEnd = NULL;
	// <== FunnyNick [SiRoB/Stulle] - Stulle

	m_htiConTweaks = NULL;
	// ==> Quick start [TPT] - Max
	m_htiQuickStartGroup = NULL;
	m_htiQuickStart = NULL;
	m_htiQuickStartMaxTime = NULL;
	m_htiQuickStartMaxConnPerFive = NULL;
	m_htiQuickStartMaxConn = NULL;
	m_htiQuickStartMaxConnPerFiveBack = NULL;
	m_htiQuickStartMaxConnBack = NULL;
	m_htiQuickStartAfterIPChange = NULL;
	// <== Quick start [TPT] - Max
	// ==> Enforce Ratio [Stulle] - Stulle
	m_htiRatioGroup = NULL;
	m_htiEnforceRatio = NULL;
	m_htiRatioValue = NULL;
	// <== Enforce Ratio [Stulle] - Stulle
	// ==> Improved ICS-Firewall support [MoNKi] - Max
	m_htiICFSupportRoot = NULL;
	m_htiICFSupport = NULL;
	m_htiICFSupportClearAtEnd = NULL;
	m_htiICFSupportServerUDP = NULL;
	// <== Improved ICS-Firewall support [MoNKi] - Max
	// ==> UPnP support [MoNKi] - leuk_he
	m_htiUPnPNatGroup = NULL;
	m_htiUPnPNat = NULL;
	m_htiUpnPNATwebservice = NULL;
	m_htiUpnpBinaddr = NULL;
	m_htiUPnPForceUpdate=NULL;
	// <== UPnP support [MoNKi] - leuk_he 
	// ==> Random Ports [MoNKi] - Stulle
	m_htiRndGrp = NULL;
	m_htiRandomports = NULL;
	m_htiRandomFirstPort = NULL;
	m_htiRandomLastPort = NULL;
	m_htiRandomPortsResetTime = NULL;
	// <== Random Ports [MoNKi] - Stulle
	// ==> Mephisto Upload - Mephisto
	m_htiMephistoUploadGrp = NULL;
	m_htiMephistoWarning1 = NULL;
	m_htiMephistoWarning2 = NULL;
	m_htiMinSlots = NULL;
	m_htiNoNewSlotTimer = NULL;
	m_htiFullLoops = NULL;
	m_htiMonitorLoops = NULL;
	m_htiNotReachedBW = NULL;
	m_htiNoTrickleTimer = NULL;
	m_htiMoveDownKB = NULL;
	// <== Mephisto Upload - Mephisto
	// ==> Multiple Part Transfer [Stulle] - Mephisto
	m_htiChunksGroup = NULL;
	m_htiChunksScore = NULL;
	m_htiChunksXman = NULL;
	m_htiChunksFinish = NULL;
	m_htiChunksToFinish = NULL;
	m_htiChunksFull = NULL;
	m_htiChunksToUpload = NULL;
	// <== Multiple Part Transfer [Stulle] - Mephisto
	m_htiMaxUpMinutes = NULL; // Adjust max upload time [Stulle] - Mephisto
	m_htiReAskFileSrc = NULL; // Timer for ReAsk File Sources [Stulle] - Stulle
	m_htiACC = NULL; // ACC [Max/WiZaRd] - Max
	m_htiIgnoreThird = NULL; // Do not reserve 1/3 of your uploadlimit for emule [Stulle] - Stulle
	m_htiUlThres = NULL; // Disable accepting only clients who asked within last 30min [Stulle] - Stulle
	m_htiMaxSlotSpeed = NULL; // Alwasy maximize slot speed [Stulle] - Stulle

	// ==> Anti Uploader Ban [Stulle] - Stulle
	m_htiAntiUploaderBanLimit = NULL;
	m_htiAntiCase1 = NULL;
	m_htiAntiCase2 = NULL;
	m_htiAntiCase3 = NULL;
	// <== Anti Uploader Ban [Stulle] - Stulle

	// ==> CreditSystems [EastShare/ MorphXT] - Stulle
	m_htiCreditSystem = NULL;
	m_htiOfficialCredit = NULL;
	m_htiLovelaceCredit = NULL;
	m_htiRatioCredit = NULL;
	m_htiPawcioCredit = NULL;
	m_htiESCredit = NULL;
	m_htiSivkaCredit = NULL;
	m_htiSwatCredit = NULL;
	m_htiXmanCredit = NULL;
	m_htiTk4Credit = NULL;
	m_htiZzulCredit = NULL;
	// <== CreditSystems [EastShare/ MorphXT] - Stulle
	m_htiFineCS = NULL; // Modified FineCS [CiccioBastardo/Stulle] - Stulle
	// ==> Pay Back First [AndCycle/SiRoB/Stulle] - Stulle
	m_htiIsPayBackFirst = NULL;
	m_htiPayBackFirstLimit = NULL;
	m_htiIsPayBackFirst2 = NULL;
	m_htiPayBackFirstLimit2 = NULL;
	// <== Pay Back First [AndCycle/SiRoB/Stulle] - Stulle

	m_htiDisplay = NULL;
	// ==> CPU/MEM usage [$ick$/Stulle] - Max
	m_htiSysInfoGroup = NULL;
	m_htiSysInfo = NULL;
	m_htiSysInfoGlobal = NULL;
	// <== CPU/MEM usage [$ick$/Stulle] - Max
	// ==> Invisible Mode [TPT/MoNKi] - Stulle
	m_htiInvisibleModeRoot = NULL;
	m_htiInvisibleMode = NULL;
	m_htiInvisibleModeMod = NULL;
	m_htiInvisibleModeKey = NULL;
	m_htiInvisibleModeStart = NULL;
	// <== Invisible Mode [TPT/MoNKi] - Stulle
	m_htiShowSrcOnTitle = NULL; // Show sources on title - Stulle
	m_htiShowGlobalHL = NULL; // show global HL - Stulle
	m_htiShowFileHLconst = NULL; // show HL per file constantly - Stulle
	m_htiShowInMSN7 = NULL; // Show in MSN7 [TPT] - Stulle
	m_htiQueueProgressBar = NULL;
	m_htiTrayComplete = NULL; // Completed in Tray [Stulle] - Stulle
	m_htiColorFeedback = NULL; // Feedback personalization [Stulle] - Stulle
	m_htiShowClientPercentage = NULL; // Show Client Percentage optional [Stulle] - Stulle
	m_htiFollowTheMajority = NULL; // Follow The Majority [AndCycle/Stulle] - Stulle
	m_htiShowSpeedMeter = NULL; // High resulution speedmeter on toolbar [eFMod/Stulle] - Stulle
	// ==> Static Tray Icon [MorphXT] - MyTh88
	m_htiStaticIcon = NULL;
	// <== Static Tray Icon [MorphXT] - MyTh88

	// ==> File Settings [sivka/Stulle] - Stulle
	m_htiFileDefaults = NULL;
	m_htiAutoNNS = NULL;
	m_htiAutoNNSTimer = NULL;
	m_htiAutoNNSLimit = NULL;
	m_htiAutoFQS = NULL;
	m_htiAutoFQSTimer = NULL;
	m_htiAutoFQSLimit = NULL;
	m_htiAutoQRS = NULL;
	m_htiAutoQRSTimer = NULL;
	m_htiAutoQRSMax = NULL;
	m_htiAutoQRSLimit = NULL;
	m_htiAutoQRSWay = NULL;
	m_htiHQRXman = NULL;
	m_htiHQRSivka = NULL;
	// <== File Settings [sivka/Stulle] - Stulle

	// ==> TBH: minimule - Max
	m_htiMMGroup = NULL;
	m_htiShowMM = NULL;
	m_htiMMLives = NULL;
	m_htiMMUpdateTime = NULL;
	m_htiMMTrans = NULL;
	m_htiMMCompl = NULL;
	m_htiMMOpen = NULL;
	// <== TBH: minimule - Max

	// ==> Control download priority [tommy_gun/iONiX] - MyTh88
	m_htiAutoDownPrioGroup = NULL;
	m_htiAutoDownPrioOff = NULL;
	m_htiAutoDownPrioPerc = NULL;
	m_htiAutoDownPrioPercVal = NULL;
	m_htiAutoDownPrioSize = NULL;
	m_htiAutoDownPrioSizeVal = NULL;
	m_htiAutoDownPrioValGroup = NULL;
	m_htiAutoDownPrioLow = NULL;
	m_htiAutoDownPrioNormal = NULL;
	m_htiAutoDownPrioHigh = NULL;
	// <== Control download priority [tommy_gun/iONiX] - MyTh88

	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	m_htiSCC = NULL;
	m_htiDlMode = NULL;
	m_htiDlNone = NULL;
	m_htiDlAlph = NULL;
	m_htiDlLP = NULL;
	m_htiShowCatNames = NULL;
	m_htiSelectCat = NULL;
	m_htiUseActiveCat = NULL;
	m_htiAutoSetResOrder = NULL;
	m_htiSmallFileDLPush = NULL;
	m_htiResumeFileInNewCat = NULL;
	m_htiUseAutoCat = NULL;
	m_htiAddRemovedInc = NULL;
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

	m_htiSharedPrefs = NULL; // Shared Files Management [Stulle] - Stulle
	// ==> PowerShare [ZZ/MorphXT] - Stulle
	m_htiPowershareMode = NULL;
	m_htiPowershareDisabled = NULL;
	m_htiPowershareActivated = NULL;
	m_htiPowershareAuto = NULL;
	m_htiPowershareLimited = NULL;
	m_htiPowerShareLimit = NULL;
	// <== PowerShare [ZZ/MorphXT] - Stulle
	m_htiPsAmountLimit = NULL; // Limit PS by amount of data uploaded [Stulle] - Stulle
	// ==> Spread Credits Slot [Stulle] - Stulle
	m_htiSpreadCreditsSlotGroup = NULL;
	m_htiSpreadCreditsSlot = NULL;
	m_htiSpreadCreditsSlotCounter = NULL;
	// <== Spread Credits Slot [Stulle] - Stulle
	// ==> Release Bonus [sivka] - Stulle
	m_htiReleaseBonusGroup = NULL;
	m_htiReleaseBonus0 = NULL;
	m_htiReleaseBonus1 = NULL;
	m_htiReleaseBonusDays = NULL;
	m_htiReleaseBonusDaysEdit = NULL;
	// <== Release Bonus [sivka] - Stulle
	m_htiReleaseScoreAssurance = NULL; // Release Score Assurance [Stulle] - Stulle
	m_htiSpreadBars = NULL; // Spread bars [Slugfiller/MorphXT] - Stulle
	// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	m_htiHideOS = NULL;
	m_htiSelectiveShare = NULL;
	m_htiShareOnlyTheNeed = NULL;
	// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	m_htiFairPlay = NULL; // Fair Play [AndCycle/Stulle] - Stulle

	m_htiMisc = NULL;
	// ==> Global Source Limit [Max/Stulle] - Stulle
	m_htiGlobalHlGroup = NULL;
	m_htiGlobalHL = NULL;
	m_htiGlobalHlLimit = NULL;
	m_htiGlobalHlAll = NULL;
	m_htiGlobalHlDefault = NULL;
	// <== Global Source Limit [Max/Stulle] - Stulle
	// ==> Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	m_htiEmulatorGroup = NULL;
	m_htiEmuMLDonkey = NULL;
	m_htiEmueDonkey = NULL;
	m_htiEmueDonkeyHybrid = NULL;
	m_htiEmuShareaza = NULL;
	m_htiEmuLphant = NULL;
	m_htiLogEmulator = NULL;
	// <== Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	// ==> Automatic shared files updater [MoNKi] - Stulle
	m_htiAutoSharedGroup = NULL;
	m_htiAutoSharedUpdater = NULL;
	m_htiSingleSharedDirUpdater = NULL;
	m_htiTimeBetweenReloads = NULL;
	// <== Automatic shared files updater [MoNKi] - Stulle
	m_htiSUQWT = NULL; // SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
	m_htiStartupSound = NULL; // Startupsound [Commander] - mav744
	m_htiCompressLevel = NULL; // Adjust Compress Level [Stulle] - Stulle

	// ==> Advanced Options [Official/MorphXT] - Stulle
	m_hti_AdvMiniMule=NULL;
	m_hti_bMiniMuleAutoClose=NULL;
	m_hti_iMiniMuleTransparency=NULL;

	m_hti_MediaInfo=NULL;
	m_hti_sMediaInfo_MediaInfoDllPath=NULL;
	m_hti_bMediaInfo_RIFF=NULL;
	m_hti_bMediaInfo_ID3LIB=NULL;
#ifdef HAVE_QEDIT_H
	m_hti_MediaInfo_MediaDet=NULL;
#endif//HAVE_QEDIT_H
	m_hti_MediaInfo_RM=NULL;
#ifdef HAVE_WMSDK_H
	m_hti_MediaInfo_WM=NULL;
#endif//HAVE_WMSDK_H

	m_hti_AdvDisplay=NULL;
	m_hti_m_bRestoreLastMainWndDlg=NULL;
	m_hti_m_bRestoreLastLogPane=NULL;
	m_hti_m_iStraightWindowStyles=NULL;
	m_hti_m_bRTLWindowsLayout=NULL;
	m_hti_m_iMaxChatHistory=NULL;
	m_hti_maxmsgsessions=NULL;
	m_htidatetimeformat = NULL;
	m_htidatetimeformat4lists = NULL;
	m_htiShowVerticalHourMarkers=NULL;
	m_htiReBarToolbar = NULL;
	m_htiIconflashOnNewMessage = NULL;
	m_htiShowCopyEd2kLinkCmd = NULL;
	m_htiUpdateQueue = NULL;
	m_htiRepaint = NULL;
	m_htiExtraPreviewWithMenu = NULL;
	m_htiShowUpDownIconInTaskbar = NULL;
	m_htiForceSpeedsToKB = NULL;

	m_hti_Log=NULL;
	m_hti_m_iLogFileFormat=NULL;
	m_hti_iMaxLogBuff=NULL;
	m_htidatetimeformat4log = NULL;
	m_htiLogError = NULL;
	m_htiLogWarning = NULL;
	m_htiLogSuccess = NULL;

	m_hti_bCheckComctl32 =NULL;
	m_hti_bCheckShell32=NULL;
	m_hti_bIgnoreInstances=NULL;
	m_hti_sNotifierMailEncryptCertName=NULL;
	m_hti_m_iPreviewSmallBlocks=NULL;
	m_hti_m_bPreviewCopiedArchives=NULL;
	m_hti_m_bPreviewOnIconDblClk=NULL;
	m_hti_sInternetSecurityZone=NULL;
	m_hti_sTxtEditor=NULL;
	m_hti_iServerUDPPort=NULL;
	m_hti_m_bRemoveFilesToBin=NULL;
	m_hti_HighresTimer=NULL;
	m_hti_TrustEveryHash=NULL;
	m_hti_InspectAllFileTypes=NULL;
	m_hti_PreferRestrictedOverUser=NULL;
	m_hti_WebFileUploadSizeLimitMB =NULL;
	m_hti_AllowedIPs=NULL;
	m_hti_UseUserSortedServerList=NULL;
	m_hti_DebugSearchResultDetailLevel=NULL;
	m_htiAdjustNTFSDaylightFileTime = NULL;
	m_htidontcompressavi = NULL;
	m_htiICH = NULL;
    m_dwBindAddr = NULL;
	m_htiFileBufferTimeLimit = NULL;
	m_htiRearrangeKadSearchKeywords = NULL;
	m_htiBeeper = NULL;
	m_htiMsgOnlySec = NULL;
	m_htiDisablePeerCache = NULL;
	m_htiKeepUnavailableFixedSharedDirs = NULL;
	// <== Advanced Options [Official/MorphXT] - Stulle

	CPropertyPage::OnDestroy();
}
LRESULT CPPgScar::OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam)
{
	if (wParam == IDC_SCAR_OPTS){
		TREEOPTSCTRLNOTIFY* pton = (TREEOPTSCTRLNOTIFY*)lParam;

		BOOL bCheck;

		// ==> FunnyNick [SiRoB/Stulle] - Stulle
		if (m_htiFnActive && pton->hItem == m_htiFnActive)
		{
			if (m_ctrlTreeOptions.GetCheckBox(m_htiFnActive, bCheck))
			{
				if(m_htiFnTagMode)	m_ctrlTreeOptions.SetGroupEnable(m_htiFnTagMode, bCheck);
				if(m_htiFnTagAtEnd)	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiFnTagAtEnd, bCheck);
				if(bCheck) m_ctrlTreeOptions.Expand(m_htiCustomTag, TVE_EXPAND);
				else m_ctrlTreeOptions.Expand(m_htiCustomTag, TVE_COLLAPSE);
			}
		}
		// <== FunnyNick [SiRoB/Stulle] - Stulle
		// ==> Multiple Part Transfer [Stulle] - Mephisto
		if (m_htiChunksScore && pton->hItem == m_htiChunksScore)
		{
			if (m_ctrlTreeOptions.GetRadioButtonEnable(m_htiChunksScore, bCheck))
			{
				if(bCheck)
				{
					m_ctrlTreeOptions.Expand(m_htiChunksFinish, TVE_COLLAPSE);
					m_ctrlTreeOptions.Expand(m_htiChunksFull, TVE_COLLAPSE);
				}
			}
		}
		if (m_htiChunksXman && pton->hItem == m_htiChunksXman)
		{
			if (m_ctrlTreeOptions.GetRadioButtonEnable(m_htiChunksXman, bCheck))
			{
				if(bCheck)
				{
					m_ctrlTreeOptions.Expand(m_htiChunksFinish, TVE_COLLAPSE);
					m_ctrlTreeOptions.Expand(m_htiChunksFull, TVE_COLLAPSE);
				}
			}
		}
		if (m_htiChunksFinish && pton->hItem == m_htiChunksFinish)
		{
			if (m_ctrlTreeOptions.GetRadioButtonEnable(m_htiChunksFinish, bCheck))
			{
				if(bCheck)
				{
					m_ctrlTreeOptions.Expand(m_htiChunksFinish, TVE_EXPAND);
					m_ctrlTreeOptions.Expand(m_htiChunksFull, TVE_COLLAPSE);
				}
			}
		}
		if (m_htiChunksFull && pton->hItem == m_htiChunksFull)
		{
			if (m_ctrlTreeOptions.GetRadioButtonEnable(m_htiChunksFull, bCheck))
			{
				if(bCheck)
				{
					m_ctrlTreeOptions.Expand(m_htiChunksFinish, TVE_COLLAPSE);
					m_ctrlTreeOptions.Expand(m_htiChunksFull, TVE_EXPAND);
				}
			}
		}
		// <== Multiple Part Transfer [Stulle] - Mephisto
		// ==> Quick start [TPT] - Max
		if (m_htiQuickStart && pton->hItem == m_htiQuickStart)
		{
			if (m_ctrlTreeOptions.GetCheckBox(m_htiQuickStart, bCheck))
			{
				if (m_htiQuickStartAfterIPChange)	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiQuickStartAfterIPChange, bCheck);
			}
		}
		// <== Quick start [TPT] - Max

		// ==> CPU/MEM usage [$ick$/Stulle] - Max
		if (m_htiSysInfo && pton->hItem == m_htiSysInfo)
		{
			if (m_ctrlTreeOptions.GetCheckBox(m_htiSysInfo, bCheck))
			{
				if (m_htiSysInfoGlobal)	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiSysInfoGlobal, bCheck);
			}
		}
		// <== CPU/MEM usage [$ick$/Stulle] - Max

		// ==> Invisible Mode [TPT/MoNKi] - Stulle
		if (m_htiInvisibleModeMod && pton->hItem == m_htiInvisibleModeMod)
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

		if (m_htiInvisibleModeMod && pton->hItem == m_htiInvisibleModeKey)
		{
			CTreeOptionsInvisibleModCombo* keyCombo = (CTreeOptionsInvisibleModCombo*)pton->nmhdr.code;
			if (keyCombo)
				keyCombo->GetLBText(keyCombo->GetCurSel(),m_sInvisibleModeKey);
		}

		if(m_htiInvisibleMode && pton->hItem == m_htiInvisibleMode)
		{
			if(m_ctrlTreeOptions.GetCheckBox(m_htiInvisibleMode, /*m_bInvisibleMode*/bCheck)){
				if(/*m_bInvisibleMode*/bCheck)
					m_ctrlTreeOptions.SetItemText(m_htiInvisibleModeRoot, GetResString(IDS_INVMODE_GROUP) + 
						_T(" (") + m_sInvisibleModeMod + _T(" + ") + m_sInvisibleModeKey + _T(")"));
				else
					m_ctrlTreeOptions.SetItemText(m_htiInvisibleModeRoot, GetResString(IDS_INVMODE_GROUP));
			}

			if (m_ctrlTreeOptions.GetCheckBox(m_htiInvisibleMode, bCheck))
			{
				if (m_htiInvisibleModeStart)	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiInvisibleModeStart, bCheck);
			}
		}
		// <== Invisible Mode [TPT/MoNKi] - Stulle

		// ==> Automatic shared files updater [MoNKi] - Stulle
		if (m_htiAutoSharedUpdater && pton->hItem == m_htiAutoSharedUpdater)
		{
			if (m_ctrlTreeOptions.GetCheckBox(m_htiAutoSharedUpdater, bCheck))
			{
				if (m_htiSingleSharedDirUpdater)	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiSingleSharedDirUpdater, bCheck);
			}
		}
		// <== Automatic shared files updater [MoNKi] - Stulle

		SetModified();
	}
	else if(wParam == IDC_ADVANCED_OPTS){
//		TREEOPTSCTRLNOTIFY* pton = (TREEOPTSCTRLNOTIFY*)lParam;

//		BOOL bCheck;

		SetModified();
	}
	return 0;
}

void CPPgScar::OnHScroll(UINT /*nSBCode*/, UINT /*nPos*/, CScrollBar* pScrollBar)
{
	// ==> push small files [sivka] - Stulle
	if( pScrollBar->GetSafeHwnd() == m_ctlPushSmallSize.m_hWnd )
		ShowPushSmallFileValues();
	// <== push small files [sivka] - Stulle

	SetModified(TRUE);
	//UpdateData(false); 
	//CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

// ==> push small files [sivka] - Stulle
void CPPgScar::ShowPushSmallFileValues()
{
	// ==> Tabbed Preferences [TPT] - Stulle
	/*
	GetDlgItem(IDC_PUSHSMALL)->SetWindowText(CastItoXBytes(float(((CSliderCtrl*)GetDlgItem(IDC_PUSHSMALL_SLIDER))->GetPos())));
	*/
	m_iPushSmallLabel.SetWindowText(CastItoXBytes(float(m_ctlPushSmallSize.GetPos()<<10)));
	// <== Tabbed Preferences [TPT] - Stulle
}
// <== push small files [sivka] - Stulle

void CPPgScar::OnHelp()
{
	//theApp.ShowHelp(0);
}

BOOL CPPgScar::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgScar::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}

// ==> Tabbed Preferences [TPT] - Stulle
void CPPgScar::InitTab()
{
	// Clear all to be sure
	m_tabCtr.DeleteAllItems();
	
	// Change style
	// Remark: It seems that the multi-row can not be activated with the properties
	m_tabCtr.ModifyStyle(0, TCS_MULTILINE);

	// Add all items with icon
	m_tabCtr.SetImageList(&m_imageList);
	int iTemp=0;
	// ==> Mephisto mod [Stulle] - Mephisto
	/*
	m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, SCAR, GetResString(IDS_SCARANGEL), iTemp++, (LPARAM)SCAR);
	*/
	m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, SCAR, GetResString(IDS_MEPHISTO), iTemp++, (LPARAM)SCAR);
	// <== Mephisto mod [Stulle] - Mephisto
	m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, BACKUP, GetResString(IDS_BACKUP), iTemp++, (LPARAM)BACKUP);
	m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, COLOR, GetResString(IDS_COLOR_BOX), iTemp++, (LPARAM)COLOR);
	m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, ADVANCED, GetResString(IDS_LD_ADVANCEDOPT), iTemp++, (LPARAM)ADVANCED);
	m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, UPDATE, GetResString(IDS_SV_UPDATE), iTemp++, (LPARAM)UPDATE);
	m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, SUPPORT, GetResString(IDS_SUPPORT), iTemp++, (LPARAM)SUPPORT);

	m_tabCtr.SetCurSel(theApp.emuledlg->preferenceswnd->m_ScarTab);
}


void CPPgScar::InitControl()
{
	// Remark: don't use the dialog editor => avoid to merge rc

	// Retrieve the bottom of the tab's header
	RECT rect1;
	RECT rect2;
	m_tabCtr.GetWindowRect(&rect1);
	ScreenToClient(&rect1);
	m_tabCtr.GetItemRect(m_tabCtr.GetItemCount() - 1 , &rect2);
	const int top = rect1.top + (rect2.bottom - rect2.top + 1) * m_tabCtr.GetRowCount() + 10;
	const int left = rect1.left + 6;
	const int bottom = rect1.bottom-10;
	const int right = rect1.right - 6;

	// ScarAngel
	// Note: The objects of this tab need to be shown so the tab is always initialised correctly
	// when the Preferences Dialog is opend anew.
	m_strWarning.CreateEx(0, _T("STATIC"), _T(""), 
							WS_CHILD | WS_VISIBLE, 
							CRect(left, top, right, top+39), this, IDC_WARNING);
	m_strWarning.SetFont(GetFont());	

	m_ctrlTreeOptions.MoveWindow(CRect(left, top+49, right, bottom-39),TRUE);

	m_strPushSmall.CreateEx(0, _T("STATIC"), _T("Push Small Files"), 
							WS_CHILD | WS_VISIBLE, 
							CRect(left, bottom-34, left+92, bottom-16), this, IDC_PUSHSMALL_LABEL);
	m_strPushSmall.SetFont(GetFont());	

	m_iPushSmallLabel.CreateEx(0, _T("STATIC"), _T("MB"), 
							WS_CHILD | WS_VISIBLE, 
							CRect(right-50, bottom-34, right, bottom-16), this, IDC_PUSHSMALL);
	m_iPushSmallLabel.SetFont(GetFont());	

	m_ctlPushSmallSize.CreateEx(WS_EX_STATICEDGE,
							  WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER |
							  /*TBS_TOOLTIPS | */TBS_BOTH/* | TBS_VERT*/ | TBS_NOTICKS | WS_TABSTOP,
							  CRect(left, bottom-16, right, bottom), this, IDC_PUSHSMALL_SLIDER);
	m_ctlPushSmallSize.SetFont(GetFont());

	// Backup
	m_BackupBox.CreateEx(0, _T("BUTTON"), _T("Select File Types to Backup"), 
						   WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
						   BS_GROUPBOX,
						   CRect(left, top, right, top+120), this, IDC_BACKUP_FILEFRAME);
	m_BackupBox.SetFont(GetFont());

	m_Dat.CreateEx(0, _T("BUTTON"), _T("*.dat"), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_AUTOCHECKBOX, 
									CRect(left+12, top+15, right-90, top+30), this, IDC_DAT);
	m_Dat.SetFont(GetFont());

	m_Met.CreateEx(0, _T("BUTTON"), _T("*.met"), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_AUTOCHECKBOX, 
									CRect(left+12, top+35, right-90, top+50), this, IDC_MET);
	m_Met.SetFont(GetFont());

	m_Ini.CreateEx(0, _T("BUTTON"), _T("*.ini"), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_AUTOCHECKBOX, 
									CRect(left+12, top+55, right-90, top+70), this, IDC_INI);
	m_Ini.SetFont(GetFont());

	m_Part.CreateEx(0, _T("BUTTON"), _T("*.part"), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_AUTOCHECKBOX, 
									CRect(left+12, top+75, right-90, top+90), this, IDC_PART);
	m_Part.SetFont(GetFont());

	m_PartMet.CreateEx(0, _T("BUTTON"), _T("*.part.met"), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_AUTOCHECKBOX, 
									CRect(left+12, top+95, right-90, top+110), this, IDC_PARTMET);
	m_PartMet.SetFont(GetFont());

	m_SelectAll.CreateEx(0, _T("BUTTON"), _T("Select All"), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_FLAT | BS_DEFPUSHBUTTON,
									CRect(right-87, top+55, right-12, top+80), this, IDC_SELECTALL);
	m_SelectAll.SetFont(GetFont());

	m_BackupNow.CreateEx(0, _T("BUTTON"), _T("Backup Now"), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_FLAT,
									CRect(right-87, top+85, right-12, top+110), this, IDC_BACKUPNOW);
	m_BackupNow.SetFont(GetFont());

	m_AutoBackupBox.CreateEx(0, _T("BUTTON"), _T("Auto Backup"), 
						   WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
						   BS_GROUPBOX,
						   CRect(left, top+130, left+170, top+190), this, IDC_BACKUP_AUTO);
	m_AutoBackupBox.SetFont(GetFont());

	m_AutoBackup.CreateEx(0, _T("BUTTON"), _T("Auto Backup on Exit"), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_AUTOCHECKBOX, 
									CRect(left+12, top+147, left+158, top+162), this, IDC_AUTOBACKUP);
	m_AutoBackup.SetFont(GetFont());

	m_AutoBackup2.CreateEx(0, _T("BUTTON"), _T("Double Backup"), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_AUTOCHECKBOX, 
									CRect(left+22, top+165, left+158, top+180), this, IDC_AUTOBACKUP2);
	m_AutoBackup2.SetFont(GetFont());

	m_Note.CreateEx(0, _T("STATIC"), _T("Note:"),
						WS_CHILD /*| WS_VISIBLE*/, 
						CRect(left+10, top+200, right-10, top+215), this, IDC_BACKUP_NOTE);
	m_Note.SetFont(GetFont());

	m_NoteText.CreateEx(0, _T("STATIC"), _T(""), 
						WS_CHILD /*| WS_VISIBLE*/,
						CRect(left+10, top+220, right-10, bottom), this, IDC_BACKUP_MESSAGE);
	m_NoteText.SetFont(GetFont());

	// Design settings
	m_ColorBox.CreateEx(0, _T("BUTTON"), _T("Design Settings"), 
						   WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
						   BS_GROUPBOX,
						   CRect(left, top, right, top+175), this, IDC_COLOR_BOX);
	m_ColorBox.SetFont(GetFont());

	m_MasterCombo.Create(WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,							  
							  CRect(left+10, top+15, right-60, top+135), this, IDC_COLOR_MASTER_COMBO);
	m_MasterCombo.SetFont(GetFont());

	m_SubCombo.Create(WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,							  
							  CRect(left+10, top+45, right-60, top+165), this, IDC_COLOR_SUB_COMBO);
	m_SubCombo.SetFont(GetFont());

	m_OnOff.CreateEx(0, _T("BUTTON"), _T(""), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									/*BS_FLAT | */BS_PUSHBUTTON/* | BS_AUTOCHECKBOX*/,
									CRect(right-50, top+25, right-10, top+65), this, IDC_COLOR_ON_OFF);
	m_OnOff.SetFont(GetFont());

	m_bold.CreateEx(0, _T("BUTTON"), _T("B"), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP | BS_PUSHBUTTON |
									BS_AUTOCHECKBOX,
									CRect(left+90, top+75, left+108, top+95), this, IDC_COLOR_BOLD);
	m_bold.SetFont(GetFont());

	m_underlined.CreateEx(0, _T("BUTTON"), _T("U"), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP | BS_PUSHBUTTON |
									BS_AUTOCHECKBOX,
									CRect(left+115, top+75, left+133, top+95), this, IDC_COLOR_UNDERLINED);
	m_underlined.SetFont(GetFont());

	m_italic.CreateEx(0, _T("BUTTON"), _T("I"), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP | BS_PUSHBUTTON |
									BS_AUTOCHECKBOX,
									CRect(left+140, top+75, left+158, top+95), this, IDC_COLOR_ITALIC);
	m_italic.SetFont(GetFont());

	m_FontColorLabel.CreateEx(0, _T("STATIC"), _T("Font color"),
						WS_CHILD /*| WS_VISIBLE*/, 
						CRect(left+10, top+110, right-70, top+125), this, IDC_COLOR_FONT_LABEL);
	m_FontColorLabel.SetFont(GetFont());

	m_FontColor.CreateEx(0, _T("BUTTON"), _T(""), 
									WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON,
									CRect(right-60, top+105, right-10, top+130), this, IDC_COLOR_FONT);
	m_FontColor.SetFont(GetFont());

	m_BackColorLabel.CreateEx(0, _T("STATIC"), _T("Background color"),
						WS_CHILD /*| WS_VISIBLE*/, 
						CRect(left+10, top+145, right-70, top+160), this, IDC_COLOR_BACK_LABEL);
	m_BackColorLabel.SetFont(GetFont());

	m_BackColor.CreateEx(0, _T("BUTTON"), _T(""), 
									WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON,
									CRect(right-60, top+140, right-10, top+165), this, IDC_COLOR_BACK);
	m_BackColor.SetFont(GetFont());

	m_ColorWarning.CreateEx(0, _T("STATIC"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/, 
							CRect(left+10, top+175, right-10, top+214), this, IDC_COLOR_WARNING);
	m_ColorWarning.SetFont(GetFont());	

	m_EasterEgg.CreateEx(0, _T("BUTTON"), _T("Don't like the design? Click!"), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_FLAT | BS_DEFPUSHBUTTON,
									CRect(left, top+185, right, top+210), this, IDC_EASTEREGG);
	m_EasterEgg.SetFont(GetFont());

	// Advanced
	m_ctrlAdvTreeOptions.MoveWindow(CRect(left, top+49, right, bottom),TRUE);
	m_ctrlAdvTreeOptions.ShowWindow(SW_HIDE);
	m_ctrlAdvTreeOptions.EnableWindow(FALSE);

	// Update
	m_AntiLeechBox.CreateEx(0, _T("BUTTON"), _T("antiLeech.dll"), 
						   WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
						   BS_GROUPBOX,
						   CRect(left, top, right, top+100), this, IDC_ANTI_LEECH_BOX);
	m_AntiLeechBox.SetFont(GetFont());

	m_AntiLeechStart.CreateEx(0, _T("BUTTON"), _T("Update antiLeech.dll on startup"), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_AUTOCHECKBOX, 
									CRect(left+10, top+15, left+200, top+28), this, IDC_ANTI_LEECH_STARTUP);
	m_AntiLeechStart.SetFont(GetFont());

	m_AntiLeechWeek.CreateEx(0, _T("BUTTON"), _T("Update weekly"), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_AUTOCHECKBOX, 
									CRect(left+210, top+15, right-10, top+28), this, IDC_ANTI_LEECH_WEEK);
	m_AntiLeechWeek.SetFont(GetFont());

	m_AntiLeechURLStatic.CreateEx(0, _T("STATIC"), _T("URL for updating"), 
							WS_CHILD /*| WS_VISIBLE*/, 
							CRect(left+10, top+33, right-10, top+48), this, IDC_ANTI_LEECH_URL_STATIC);
	m_AntiLeechURLStatic.SetFont(GetFont());	

	m_AntiLeechURL.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), 
						WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
						ES_LEFT | ES_AUTOHSCROLL, 
						CRect(left+10, top+50, right-10, top+70), this,  IDC_ANTI_LEECH_URL);
	m_AntiLeechURL.SetFont(GetFont());

	m_AntiLeechVersion.CreateEx(0, _T("STATIC"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/, 
							CRect(left+10, top+75, right-130, top+90), this, IDC_ANTI_LEECH_VERSION);
	m_AntiLeechVersion.SetFont(GetFont());	

	m_AntiLeechReset.CreateEx(0, _T("BUTTON"), _T("Reset"), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_FLAT,
									CRect(right-120, top+75, right-70, top+95), this, IDC_ANTI_LEECH_RESET);
	m_AntiLeechReset.SetFont(GetFont());

	m_AntiLeechUpdate.CreateEx(0, _T("BUTTON"), _T("Update"), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_FLAT,
									CRect(right-60, top+75, right-10, top+95), this, IDC_ANTI_LEECH_UPDATE);
	m_AntiLeechUpdate.SetFont(GetFont());

	m_IpFilterBox.CreateEx(0, _T("BUTTON"), _T("IP Filter"), 
						   WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
						   BS_GROUPBOX,
						   CRect(left, top+110, right, top+210), this, IDC_IPFILTER_BOX);
	m_IpFilterBox.SetFont(GetFont());

	m_IpFilterStart.CreateEx(0, _T("BUTTON"), _T("Update ipfilter.dat on startup"), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_AUTOCHECKBOX, 
									CRect(left+10, top+125, left+200, top+138), this, IDC_IPFILTER_STARTUP);
	m_IpFilterStart.SetFont(GetFont());

	m_IpFilterWeek.CreateEx(0, _T("BUTTON"), _T("Update weekly"), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_AUTOCHECKBOX, 
									CRect(left+210, top+125, right-10, top+138), this, IDC_IPFILTER_WEEK);
	m_IpFilterWeek.SetFont(GetFont());

	m_IpFilterURLStatic.CreateEx(0, _T("STATIC"), _T("URL for updating"), 
							WS_CHILD /*| WS_VISIBLE*/, 
							CRect(left+10, top+143, right-10, top+158), this, IDC_IPFILTER_URL_STATIC);
	m_IpFilterURLStatic.SetFont(GetFont());	

	m_IpFilterURL.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), 
						WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
						ES_LEFT | ES_AUTOHSCROLL, 
						CRect(left+10, top+160, right-10, top+180), this,  IDC_IPFILTER_URL);
	m_IpFilterURL.SetFont(GetFont());

	m_IpFilterTime.CreateEx(0, _T("STATIC"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/, 
							CRect(left+10, top+185, right-130, top+200), this, IDC_IPFILTER_TIME);
	m_IpFilterTime.SetFont(GetFont());	

	m_IpFilterReset.CreateEx(0, _T("BUTTON"), _T("Reset"), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_FLAT,
									CRect(right-120, top+185, right-70, top+205), this, IDC_IPFILTER_RESET);
	m_IpFilterReset.SetFont(GetFont());

	m_IpFilterUpdate.CreateEx(0, _T("BUTTON"), _T("Update"), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_FLAT,
									CRect(right-60, top+185, right-10, top+205), this, IDC_IPFILTER_UPDATE);
	m_IpFilterUpdate.SetFont(GetFont());

	m_CountryBox.CreateEx(0, _T("BUTTON"), _T("Country"), 
						   WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
						   BS_GROUPBOX,
						   CRect(left, top+220, right, top+320), this, IDC_COUNTRY_BOX);
	m_CountryBox.SetFont(GetFont());

	m_CountryStart.CreateEx(0, _T("BUTTON"), _T("Update Ip-to-country.csv on startup"), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_AUTOCHECKBOX, 
									CRect(left+10, top+235, right-10, top+248), this, IDC_COUNTRY_STARTUP);
	m_CountryStart.SetFont(GetFont());

	m_CountryURLStatic.CreateEx(0, _T("STATIC"), _T("URL for updating"), 
							WS_CHILD /*| WS_VISIBLE*/, 
							CRect(left+10, top+253, right-10, top+268), this, IDC_COUNTRY_URL_STATIC);
	m_CountryURLStatic.SetFont(GetFont());	

	m_CountryURL.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), 
						WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
						ES_LEFT | ES_AUTOHSCROLL, 
						CRect(left+10, top+270, right-10, top+290), this,  IDC_COUNTRY_URL);
	m_CountryURL.SetFont(GetFont());

	m_CountryTime.CreateEx(0, _T("STATIC"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/, 
							CRect(left+10, top+295, right-130, top+310), this, IDC_COUNTRY_TIME);
	m_CountryTime.SetFont(GetFont());	

	m_CountryReset.CreateEx(0, _T("BUTTON"), _T("Reset"), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_FLAT,
									CRect(right-120, top+295, right-70, top+315), this, IDC_COUNTRY_RESET);
	m_CountryReset.SetFont(GetFont());

	m_CountryUpdate.CreateEx(0, _T("BUTTON"), _T("Update"), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_FLAT,
									CRect(right-60, top+295, right-10, top+315), this, IDC_COUNTRY_UPDATE);
	m_CountryUpdate.SetFont(GetFont());

	// Support
	m_HpLink.CreateEx(0, _T("BUTTON"), _T(""), 
						WS_BORDER | WS_VISIBLE | WS_CHILD | HTC_WORDWRAP | HTC_UNDERLINE_HOVER,
						CRect(left+10, top, right-10, top+20), this, IDC_HP_LINK);
	m_HpLink.SetFont(GetFont());

	m_BoardGerLink.CreateEx(0, _T("BUTTON"), _T(""), 
						WS_BORDER | WS_VISIBLE | WS_CHILD | HTC_WORDWRAP | HTC_UNDERLINE_HOVER,
						CRect(left+10, top+30, right-10, top+50), this, IDC_BOARD_GER_LINK);
	m_BoardGerLink.SetFont(GetFont());

	m_BoardEngLink.CreateEx(0, _T("BUTTON"), _T(""), 
						WS_BORDER | WS_VISIBLE | WS_CHILD | HTC_WORDWRAP | HTC_UNDERLINE_HOVER,
						CRect(left+10, top+60, right-10, top+80), this, IDC_BOARD_ENG_LINK);
	m_BoardEngLink.SetFont(GetFont());

	m_RateLink.CreateEx(0, _T("BUTTON"), _T(""), 
						WS_BORDER | WS_VISIBLE | WS_CHILD | HTC_WORDWRAP | HTC_UNDERLINE_HOVER,
						CRect(left+10, top+90, right-10, top+110), this, IDC_RATE_LINK);
	m_RateLink.SetFont(GetFont());

	/*
	m_XtremeLink.CreateEx(0, _T("BUTTON"), _T(""), 
						WS_BORDER | WS_VISIBLE | WS_CHILD | HTC_WORDWRAP | HTC_UNDERLINE_HOVER,
						CRect(left+10, top+120, right-10, top+150), this, IDC_XTREME_LINK);
	m_XtremeLink.SetFont(GetFont());
	*/
}

void CPPgScar::OnTabSelectionChange(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	// Retrieve tab to display
	TCITEM tabCtrlItem; 
	tabCtrlItem.mask = TCIF_PARAM;
	if(m_tabCtr.GetItem(m_tabCtr.GetCurSel(), &tabCtrlItem) == TRUE){
		SetTab(static_cast<eTab>(tabCtrlItem.lParam));
	}

	*pResult = 0;
}

void CPPgScar::SetTab(eTab tab){
	if(m_currentTab != tab){
		// Hide all control
		switch(m_currentTab){
			case SCAR:
				m_strWarning.ShowWindow(SW_HIDE);
				m_strWarning.EnableWindow(FALSE);
				m_ctrlTreeOptions.ShowWindow(SW_HIDE);
				m_ctrlTreeOptions.EnableWindow(FALSE);
				m_strPushSmall.ShowWindow(SW_HIDE);
				m_strPushSmall.EnableWindow(FALSE);
				m_iPushSmallLabel.ShowWindow(SW_HIDE);
				m_iPushSmallLabel.EnableWindow(FALSE);
				m_ctlPushSmallSize.ShowWindow(SW_HIDE);
				m_ctlPushSmallSize.EnableWindow(FALSE);
				break;
			case BACKUP:
				m_BackupBox.ShowWindow(SW_HIDE);
				m_BackupBox.EnableWindow(FALSE);
				m_Dat.ShowWindow(SW_HIDE);
				m_Dat.EnableWindow(FALSE);
				m_Met.ShowWindow(SW_HIDE);
				m_Met.EnableWindow(FALSE);
				m_Ini.ShowWindow(SW_HIDE);
				m_Ini.EnableWindow(FALSE);
				m_Part.ShowWindow(SW_HIDE);
				m_Part.EnableWindow(FALSE);
				m_PartMet.ShowWindow(SW_HIDE);
				m_PartMet.EnableWindow(FALSE);
				m_SelectAll.ShowWindow(SW_HIDE);
				m_SelectAll.EnableWindow(FALSE);
				m_BackupNow.ShowWindow(SW_HIDE);
				m_BackupNow.EnableWindow(FALSE);
				m_AutoBackupBox.ShowWindow(SW_HIDE);
				m_AutoBackupBox.EnableWindow(FALSE);
				m_AutoBackup.ShowWindow(SW_HIDE);
				m_AutoBackup.EnableWindow(FALSE);
				m_AutoBackup2.ShowWindow(SW_HIDE);
				m_AutoBackup2.EnableWindow(FALSE);
				m_Note.ShowWindow(SW_HIDE);
				m_Note.EnableWindow(FALSE);
				m_NoteText.ShowWindow(SW_HIDE);
				m_NoteText.EnableWindow(FALSE);
				break;
			case COLOR:
				m_ColorBox.ShowWindow(SW_HIDE);
				m_ColorBox.EnableWindow(FALSE);
				m_MasterCombo.ShowWindow(SW_HIDE);
				m_MasterCombo.EnableWindow(FALSE);
				m_SubCombo.ShowWindow(SW_HIDE);
				m_SubCombo.EnableWindow(FALSE);
				m_OnOff.ShowWindow(SW_HIDE);
				m_OnOff.EnableWindow(FALSE);
				m_bold.ShowWindow(SW_HIDE);
				m_bold.EnableWindow(FALSE);
				m_underlined.ShowWindow(SW_HIDE);
				m_underlined.EnableWindow(FALSE);
				m_italic.ShowWindow(SW_HIDE);
				m_italic.EnableWindow(FALSE);
				m_FontColorLabel.ShowWindow(SW_HIDE);
				m_FontColorLabel.EnableWindow(FALSE);
				m_FontColor.ShowWindow(SW_HIDE);
				m_FontColor.EnableWindow(FALSE);
				m_BackColorLabel.ShowWindow(SW_HIDE);
				m_BackColorLabel.EnableWindow(FALSE);
				m_BackColor.ShowWindow(SW_HIDE);
				m_BackColor.EnableWindow(FALSE);
				m_ColorWarning.ShowWindow(SW_HIDE);
				m_ColorWarning.EnableWindow(FALSE);
				m_EasterEgg.ShowWindow(SW_HIDE);
				m_EasterEgg.EnableWindow(FALSE);
				break; 
			case ADVANCED:
				m_strWarning.ShowWindow(SW_HIDE);
				m_strWarning.EnableWindow(FALSE);
				m_ctrlAdvTreeOptions.ShowWindow(SW_HIDE);
				m_ctrlAdvTreeOptions.EnableWindow(FALSE);
				break;
			case UPDATE:
				m_AntiLeechBox.ShowWindow(SW_HIDE);
				m_AntiLeechBox.EnableWindow(FALSE);
				m_AntiLeechStart.ShowWindow(SW_HIDE);
				m_AntiLeechStart.EnableWindow(FALSE);
				m_AntiLeechWeek.ShowWindow(SW_HIDE);
				m_AntiLeechWeek.EnableWindow(FALSE);
				m_AntiLeechURLStatic.ShowWindow(SW_HIDE);
				m_AntiLeechURLStatic.EnableWindow(FALSE);
				m_AntiLeechURL.ShowWindow(SW_HIDE);
				m_AntiLeechURL.EnableWindow(FALSE);
				m_AntiLeechVersion.ShowWindow(SW_HIDE);
				m_AntiLeechVersion.EnableWindow(FALSE);
				m_AntiLeechReset.ShowWindow(SW_HIDE);
				m_AntiLeechReset.EnableWindow(FALSE);
				m_AntiLeechUpdate.ShowWindow(SW_HIDE);
				m_AntiLeechUpdate.EnableWindow(FALSE);
				m_IpFilterBox.ShowWindow(SW_HIDE);
				m_IpFilterBox.EnableWindow(FALSE);
				m_IpFilterStart.ShowWindow(SW_HIDE);
				m_IpFilterStart.EnableWindow(FALSE);
				m_IpFilterWeek.ShowWindow(SW_HIDE);
				m_IpFilterWeek.EnableWindow(FALSE);
				m_IpFilterURLStatic.ShowWindow(SW_HIDE);
				m_IpFilterURLStatic.EnableWindow(FALSE);
				m_IpFilterURL.ShowWindow(SW_HIDE);
				m_IpFilterURL.EnableWindow(FALSE);
				m_IpFilterTime.ShowWindow(SW_HIDE);
				m_IpFilterTime.EnableWindow(FALSE);
				m_IpFilterReset.ShowWindow(SW_HIDE);
				m_IpFilterReset.EnableWindow(FALSE);
				m_IpFilterUpdate.ShowWindow(SW_HIDE);
				m_IpFilterUpdate.EnableWindow(FALSE);
				m_CountryBox.ShowWindow(SW_HIDE);
				m_CountryBox.EnableWindow(FALSE);
				m_CountryStart.ShowWindow(SW_HIDE);
				m_CountryStart.EnableWindow(FALSE);
				m_CountryURLStatic.ShowWindow(SW_HIDE);
				m_CountryURLStatic.EnableWindow(FALSE);
				m_CountryURL.ShowWindow(SW_HIDE);
				m_CountryURL.EnableWindow(FALSE);
				m_CountryTime.ShowWindow(SW_HIDE);
				m_CountryTime.EnableWindow(FALSE);
				m_CountryReset.ShowWindow(SW_HIDE);
				m_CountryReset.EnableWindow(FALSE);
				m_CountryUpdate.ShowWindow(SW_HIDE);
				m_CountryUpdate.EnableWindow(FALSE);
				break;
			case SUPPORT:
				m_HpLink.ShowWindow(SW_HIDE);
				m_HpLink.EnableWindow(FALSE);
				m_BoardGerLink.ShowWindow(SW_HIDE);
				m_BoardGerLink.EnableWindow(FALSE);
				m_BoardEngLink.ShowWindow(SW_HIDE);
				m_BoardEngLink.EnableWindow(FALSE);
				m_RateLink.ShowWindow(SW_HIDE);
				m_RateLink.EnableWindow(FALSE);
				/*
				m_XtremeLink.ShowWindow(SW_HIDE);
				m_XtremeLink.EnableWindow(FALSE);
				*/
			default:
				break;
		}

		// Show new controls
		theApp.emuledlg->preferenceswnd->m_ScarTab = tab;
		m_currentTab = tab;
		switch(m_currentTab){
			case SCAR:
				m_strWarning.ShowWindow(SW_SHOW);
				m_strWarning.EnableWindow(TRUE);
				m_ctrlTreeOptions.ShowWindow(SW_SHOW);
				m_ctrlTreeOptions.EnableWindow(TRUE);
				m_strPushSmall.ShowWindow(SW_SHOW);
				m_strPushSmall.EnableWindow(TRUE);
				m_iPushSmallLabel.ShowWindow(SW_SHOW);
				m_iPushSmallLabel.EnableWindow(TRUE);
				m_ctlPushSmallSize.ShowWindow(SW_SHOW);
				m_ctlPushSmallSize.EnableWindow(TRUE);
				break;
			case BACKUP:
				m_BackupBox.ShowWindow(SW_SHOW);
				m_BackupBox.EnableWindow(TRUE);
				m_Dat.ShowWindow(SW_SHOW);
				m_Dat.EnableWindow(TRUE);
				m_Met.ShowWindow(SW_SHOW);
				m_Met.EnableWindow(TRUE);
				m_Ini.ShowWindow(SW_SHOW);
				m_Ini.EnableWindow(TRUE);
				m_Part.ShowWindow(SW_SHOW);
				m_Part.EnableWindow(TRUE);
				m_PartMet.ShowWindow(SW_SHOW);
				m_PartMet.EnableWindow(TRUE);
				m_SelectAll.ShowWindow(SW_SHOW);
				m_SelectAll.EnableWindow(TRUE);
				m_BackupNow.ShowWindow(SW_SHOW);
				BackupNowEnable();
				m_AutoBackupBox.ShowWindow(SW_SHOW);
				m_AutoBackupBox.EnableWindow(TRUE);
				m_AutoBackup.ShowWindow(SW_SHOW);
				m_AutoBackup.EnableWindow(TRUE);
				m_AutoBackup2.ShowWindow(SW_SHOW);
				m_AutoBackup2.EnableWindow(m_AutoBackup.GetCheck() == BST_CHECKED);
				m_Note.ShowWindow(SW_SHOW);
				m_Note.EnableWindow(TRUE);
				m_NoteText.ShowWindow(SW_SHOW);
				m_NoteText.ShowWindow(TRUE);
				break;
			case COLOR:
				m_ColorBox.ShowWindow(SW_SHOW);
				m_ColorBox.EnableWindow(TRUE);
				m_MasterCombo.ShowWindow(SW_SHOW);
				m_MasterCombo.EnableWindow(TRUE);
				m_SubCombo.ShowWindow(SW_SHOW);
				m_SubCombo.EnableWindow(TRUE);
				m_OnOff.ShowWindow(SW_SHOW);
				m_OnOff.EnableWindow(TRUE);
				m_bold.ShowWindow(SW_SHOW);
				m_bold.EnableWindow(TRUE);
				m_underlined.ShowWindow(SW_SHOW);
				m_underlined.EnableWindow(TRUE);
				m_italic.ShowWindow(SW_SHOW);
				m_italic.EnableWindow(TRUE);
				m_FontColorLabel.ShowWindow(SW_SHOW);
				m_FontColorLabel.EnableWindow(TRUE);
				m_FontColor.ShowWindow(SW_SHOW);
				m_FontColor.EnableWindow(TRUE);
				m_BackColorLabel.ShowWindow(SW_SHOW);
				m_BackColorLabel.EnableWindow(TRUE);
				m_BackColor.ShowWindow(SW_SHOW);
//				m_BackColor.EnableWindow(TRUE);
				m_EasterEgg.ShowWindow(SW_SHOW);
				m_EasterEgg.EnableWindow(TRUE);
				UpdateStyles();
				break;
			case ADVANCED:
				m_strWarning.ShowWindow(SW_SHOW);
				m_strWarning.EnableWindow(TRUE);
				m_ctrlAdvTreeOptions.ShowWindow(SW_SHOW);
				m_ctrlAdvTreeOptions.EnableWindow(TRUE);
				break;
			case UPDATE:
				m_AntiLeechBox.ShowWindow(SW_SHOW);
				m_AntiLeechBox.EnableWindow(TRUE);
				m_AntiLeechStart.ShowWindow(SW_SHOW);
				m_AntiLeechStart.EnableWindow(TRUE);
				m_AntiLeechWeek.ShowWindow(SW_SHOW);
				m_AntiLeechWeek.EnableWindow(TRUE);
				m_AntiLeechURLStatic.ShowWindow(SW_SHOW);
				m_AntiLeechURLStatic.EnableWindow(TRUE);
				m_AntiLeechURL.ShowWindow(SW_SHOW);
				m_AntiLeechURL.EnableWindow(TRUE);
				m_AntiLeechVersion.ShowWindow(SW_SHOW);
				m_AntiLeechVersion.EnableWindow(TRUE);
				m_AntiLeechReset.ShowWindow(SW_SHOW);
				m_AntiLeechReset.EnableWindow(TRUE);
				m_AntiLeechUpdate.ShowWindow(SW_SHOW);
				m_AntiLeechUpdate.EnableWindow(TRUE);
				m_IpFilterBox.ShowWindow(SW_SHOW);
				m_IpFilterBox.EnableWindow(TRUE);
				m_IpFilterStart.ShowWindow(SW_SHOW);
				m_IpFilterStart.EnableWindow(TRUE);
				m_IpFilterWeek.ShowWindow(SW_SHOW);
				m_IpFilterWeek.EnableWindow(TRUE);
				m_IpFilterURLStatic.ShowWindow(SW_SHOW);
				m_IpFilterURLStatic.EnableWindow(TRUE);
				m_IpFilterURL.ShowWindow(SW_SHOW);
				m_IpFilterURL.EnableWindow(TRUE);
				m_IpFilterTime.ShowWindow(SW_SHOW);
				m_IpFilterTime.EnableWindow(TRUE);
				m_IpFilterReset.ShowWindow(SW_SHOW);
				m_IpFilterReset.EnableWindow(TRUE);
				m_IpFilterUpdate.ShowWindow(SW_SHOW);
				m_IpFilterUpdate.EnableWindow(TRUE);
				m_CountryBox.ShowWindow(SW_SHOW);
				m_CountryBox.EnableWindow(TRUE);
				m_CountryStart.ShowWindow(SW_SHOW);
				m_CountryStart.EnableWindow(TRUE);
				m_CountryURLStatic.ShowWindow(SW_SHOW);
				m_CountryURLStatic.EnableWindow(TRUE);
				m_CountryURL.ShowWindow(SW_SHOW);
				m_CountryURL.EnableWindow(TRUE);
				m_CountryTime.ShowWindow(SW_SHOW);
				m_CountryTime.EnableWindow(TRUE);
				m_CountryReset.ShowWindow(SW_SHOW);
				m_CountryReset.EnableWindow(TRUE);
				m_CountryUpdate.ShowWindow(SW_SHOW);
				m_CountryUpdate.EnableWindow(TRUE);
				break;
			case SUPPORT:
				m_HpLink.ShowWindow(SW_SHOW);
				m_HpLink.EnableWindow(TRUE);
				m_BoardGerLink.ShowWindow(SW_SHOW);
				m_BoardGerLink.EnableWindow(TRUE);
				m_BoardEngLink.ShowWindow(SW_SHOW);
				m_BoardEngLink.EnableWindow(TRUE);
				m_RateLink.ShowWindow(SW_SHOW);
				m_RateLink.EnableWindow(TRUE);
				/*
				m_XtremeLink.ShowWindow(SW_SHOW);
				m_XtremeLink.EnableWindow(TRUE);
				*/
			default:
				break;
		}
	}
}
// <== Tabbed Preferences [TPT] - Stulle

// ==> TBH: Backup [TBH/EastShare/MorphXT] - Stulle
void CPPgScar::BackupNowEnable()
{
	bool bTemp = (	m_Dat.GetCheck() == BST_CHECKED ||
					m_Met.GetCheck() == BST_CHECKED ||
					m_Ini.GetCheck() == BST_CHECKED ||
					m_Part.GetCheck() == BST_CHECKED ||
					m_PartMet.GetCheck() == BST_CHECKED
				 );
	m_BackupNow.EnableWindow(bTemp);
}

void CPPgScar::OnBnClickedDat()
{
	if (m_Dat.GetCheck() == BST_CHECKED) {
		m_BackupNow.EnableWindow(true);
	} else {
		if (m_Met.GetCheck() != BST_CHECKED && m_Ini.GetCheck() != BST_CHECKED && m_Part.GetCheck() != BST_CHECKED && m_PartMet.GetCheck() != BST_CHECKED)
			m_BackupNow.EnableWindow(false);
	}

}

void CPPgScar::OnBnClickedMet()
{
	if (m_Met.GetCheck() == BST_CHECKED) {
		m_BackupNow.EnableWindow(true);
	} else {
		if (m_Dat.GetCheck() != BST_CHECKED && m_Ini.GetCheck() != BST_CHECKED && m_Part.GetCheck() != BST_CHECKED && m_PartMet.GetCheck() != BST_CHECKED)
			m_BackupNow.EnableWindow(false);
	}

}

void CPPgScar::OnBnClickedIni()
{
	if (m_Ini.GetCheck() == BST_CHECKED) {
		m_BackupNow.EnableWindow(true);
	} else {
		if (m_Dat.GetCheck() != BST_CHECKED && m_Met.GetCheck() != BST_CHECKED && m_Part.GetCheck() != BST_CHECKED && m_PartMet.GetCheck() != BST_CHECKED)
			m_BackupNow.EnableWindow(false);
	}

}

void CPPgScar::OnBnClickedPart()
{
	if (m_Part.GetCheck() == BST_CHECKED) {
		m_BackupNow.EnableWindow(true);
	} else {
		if (m_Dat.GetCheck() != BST_CHECKED && m_Met.GetCheck() != BST_CHECKED && m_Ini.GetCheck() != BST_CHECKED && m_PartMet.GetCheck() != BST_CHECKED)
			m_BackupNow.EnableWindow(false);
	}

}

void CPPgScar::OnBnClickedPartMet()
{
	if (m_PartMet.GetCheck() == BST_CHECKED) {
		m_BackupNow.EnableWindow(true);
	} else {
		if (m_Dat.GetCheck() != BST_CHECKED && m_Met.GetCheck() != BST_CHECKED && m_Ini.GetCheck() != BST_CHECKED && m_Part.GetCheck() != BST_CHECKED)
			m_BackupNow.EnableWindow(false);
	}
}


void CPPgScar::OnBnClickedBackupnow()
{
	TCHAR buffer[200];
	y2All = FALSE;
	if (m_Dat.GetCheck() == BST_CHECKED)
	{
		Backup(_T("*.dat"), true);
		m_Dat.SetCheck(false);
	}

	if (m_Met.GetCheck() == BST_CHECKED)
	{
		Backup(_T("*.met"), true);
		m_Met.SetCheck(false);
	}

	if (m_Ini.GetCheck() == BST_CHECKED)
	{
		// Mighty Knife: Save current settings before backup
		theApp.emuledlg->SaveSettings (false);
		// [end] Mighty Knife
		Backup(_T("*.ini"), true);
		m_Ini.SetCheck(false);
	}

	if (m_PartMet.GetCheck() == BST_CHECKED)
	{
		Backup2(_T("*.part.met"));
		m_PartMet.SetCheck(false);
	}

	if (m_Part.GetCheck() == BST_CHECKED)
	{
		_stprintf(buffer,_T("Because of their size, backing up *.part files may take a few minutes.\nAre you sure you want to do this?"));
		if(MessageBox(buffer,_T("Are you sure?"),MB_ICONQUESTION|MB_YESNO)== IDYES)
			Backup2(_T("*.part"));
		m_Part.SetCheck(false);

	}

	BackupNowEnable();

	MessageBox(_T("File(s) Copied Successfully."), _T("BackUp complete."), MB_OK);
	y2All = FALSE;
}



void CPPgScar::Backup(LPCTSTR extensionToBack, BOOL conFirm)  
{
	WIN32_FIND_DATA FileData; 
	HANDLE hSearch; 
	TCHAR buffer[200];
	//CString szDirPath = CString(thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR));
	CString szDirPath = CString(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR));
	TCHAR szNewPath[MAX_PATH]; 

	SetCurrentDirectory(szDirPath);
	BOOL error = FALSE;
	BOOL OverWrite = TRUE;
	szDirPath +="Backup\\";

	BOOL fFinished = FALSE; 

	// Create a new directory if one does not exist
	if(!PathFileExists(szDirPath))
		CreateDirectory(szDirPath, NULL);

	// Start searching for files in the current directory. 

	hSearch = FindFirstFile(extensionToBack, &FileData); 
	if (hSearch == INVALID_HANDLE_VALUE) 
	{ 
		error = TRUE;
	} 

	// Copy each file to the new directory 
	CString str;
	while (!fFinished && !error) 
	{ 
		lstrcpy(szNewPath, szDirPath); 
		lstrcat(szNewPath, FileData.cFileName); 

		if(PathFileExists(szNewPath))
		{
			if (conFirm)
			{
				if (y2All == FALSE)
				{
					_stprintf(buffer, _T("File %s Already Exists. OverWrite It?"), FileData.cFileName);
					int rc = ::XMessageBox(m_hWnd,buffer,_T("OverWrite?"),MB_YESNO|MB_YESTOALL|MB_ICONQUESTION);
					if (rc == IDYES)
						OverWrite = TRUE;
					else if (rc == IDYESTOALL)
					{
						OverWrite = TRUE;
						y2All = TRUE;
					}
					else 
						OverWrite = FALSE;
				} else
					OverWrite = TRUE;
			} 
			else
				OverWrite = TRUE;
		}	
		if(OverWrite)
			CopyFile(FileData.cFileName, szNewPath, FALSE);

		if (!FindNextFile(hSearch, &FileData)) 
		{
			if (GetLastError() == ERROR_NO_MORE_FILES) 
			{ 
				//MessageBox("File Copied Successfully.", "BackUp complete", MB_OK); 
				fFinished = TRUE; 

			} 
			else 
			{ 
				error = TRUE;
			} 
		}

	} 


	// Close the search handle. 
	if (!FindClose(hSearch)) 
	{ 
		error = TRUE;
	} 
	if (error)
		MessageBox(_T("Error encountered during backup"),_T("Error"),MB_OK);
}


void CPPgScar::Backup2(LPCTSTR extensionToBack)  
{
	WIN32_FIND_DATA FileData;   
	HANDLE hSearch;   
	TCHAR buffer[200];  


	//CString szDirPath = CString(thePrefs.GetAppDir());  
	CString szDirPath = CString(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR));
	CString szTempPath = CString(thePrefs.GetTempDir());  
	TCHAR szNewPath[MAX_PATH]; 

	BOOL fFinished = FALSE;     
	BOOL error = FALSE;  
	BOOL OverWrite = TRUE;  
	szDirPath +="Backup\\";

	if(!PathFileExists(szDirPath))  
		CreateDirectory(szDirPath, NULL);  

	szDirPath+="Temp\\";  

	if(!PathFileExists(szDirPath))  
		CreateDirectory(szDirPath, NULL);  


	// Start searching for files in the current directory.   
	SetCurrentDirectory(szTempPath);  

	hSearch = FindFirstFile(extensionToBack, &FileData);   

	if (hSearch == INVALID_HANDLE_VALUE)   
	{   
		error = TRUE;
	}   

	// Copy each file to the new directory   
	while (!fFinished && !error)   
	{   
		lstrcpy(szNewPath, szDirPath);   
		lstrcat(szNewPath, FileData.cFileName);   

		//MessageBox(szNewPath,"New Path",MB_OK);  
		if(PathFileExists(szNewPath))  
		{  
				if (y2All == FALSE)
				{
					_stprintf(buffer, _T("File %s Already Exists. OverWrite It?"), FileData.cFileName);
					int rc = ::XMessageBox(m_hWnd,buffer,_T("OverWrite?"),MB_YESNO|MB_YESTOALL|MB_ICONQUESTION);
					if (rc == IDYES)
						OverWrite = TRUE;
					else if (rc == IDYESTOALL)
					{
						OverWrite = TRUE;
						y2All = TRUE;
					}
					else 
						OverWrite = FALSE;
				} else
					OverWrite = TRUE;  
		}  

		if(OverWrite)  
			CopyFile(FileData.cFileName, szNewPath, FALSE);  

		if (!FindNextFile(hSearch, &FileData))   
		{  
			if (GetLastError() == ERROR_NO_MORE_FILES)   
			{   

				fFinished = TRUE;   
			}   
			else   
			{   
				error = TRUE;  
			}   
		}  

	}   

	// Close the search handle.   
	if (!FindClose(hSearch))   
	{   
		error = TRUE;  
	}   
	SetCurrentDirectory(CString(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR)));  

	if (error)  
		MessageBox(_T("Error encountered during backup"),_T("Error"),MB_OK);  

} 

void CPPgScar::OnBnClickedSelectall()
{
	m_Dat.SetCheck(true);
	m_Met.SetCheck(true);
	m_Ini.SetCheck(true);
	m_Part.SetCheck(true);
	m_PartMet.SetCheck(true);
	m_BackupNow.EnableWindow(true);
}

void CPPgScar::OnBnClickedAutobackup()
{
	m_AutoBackup2.EnableWindow(m_AutoBackup.GetCheck() == BST_CHECKED);
	SetModified();
}

void CPPgScar::OnBnClickedAutobackup2()
{
	SetModified();
}

void CPPgScar::Backup3()
{
	WIN32_FIND_DATA FileData; 
	HANDLE hSearch; 
	CString szDirPath = CString(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR))+_T("Backup\\");
	if(!PathFileExists(szDirPath)) return;
	TCHAR szNewPath[MAX_PATH]; 

	SetCurrentDirectory(szDirPath);
	BOOL error = FALSE;
	szDirPath = CString(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR))+_T("Backup2\\");

	BOOL fFinished = FALSE; 

	// Create a new directory if one does not exist
	if(!PathFileExists(szDirPath))
		CreateDirectory(szDirPath, NULL);

	// Start searching for files in the current directory. 

	hSearch = FindFirstFile(_T("*.*"), &FileData); 
	if (hSearch == INVALID_HANDLE_VALUE) 
	{ 
		error = TRUE;
	} 

	// Copy each file to the new directory 
	while (!fFinished && !error) 
	{ 
		lstrcpy(szNewPath, szDirPath); 
		lstrcat(szNewPath, FileData.cFileName); 

		CopyFile(FileData.cFileName, szNewPath, FALSE);

		if (!FindNextFile(hSearch, &FileData)) 
		{
			if (GetLastError() == ERROR_NO_MORE_FILES) 
			{ 
				//MessageBox("File Copied Successfully.", "BackUp complete", MB_OK); 
				fFinished = TRUE; 

			} 
			else 
			{ 
				error = TRUE;
			} 
		}

	} 


	// Close the search handle. 
	if (!FindClose(hSearch)) 
	{ 
		error = TRUE;
	} 
	if (error)
		MessageBox(_T("Error encountered during backup"),_T("Error"),MB_OK);
}
// <== TBH: Backup [TBH/EastShare/MorphXT] - Stulle

// ==> Design Settings [eWombat/Stulle] - Stulle
void CPPgScar::InitMasterStyleCombo()
{
	int iSel = m_MasterCombo.GetCurSel();
	int iItem;
	m_MasterCombo.ResetContent();
	iItem = m_MasterCombo.AddString(GetResString(IDS_COLOR_MASTER1));		m_MasterCombo.SetItemData(iItem, client_styles);
	iItem = m_MasterCombo.AddString(GetResString(IDS_COLOR_MASTER2));		m_MasterCombo.SetItemData(iItem, download_styles);
	iItem = m_MasterCombo.AddString(GetResString(IDS_COLOR_MASTER3));		m_MasterCombo.SetItemData(iItem, share_styles);
	iItem = m_MasterCombo.AddString(GetResString(IDS_COLOR_MASTER4));		m_MasterCombo.SetItemData(iItem, server_styles);
	iItem = m_MasterCombo.AddString(GetResString(IDS_COLOR_MASTER5));		m_MasterCombo.SetItemData(iItem, background_styles);
	iItem = m_MasterCombo.AddString(GetResString(IDS_COLOR_MASTER6));		m_MasterCombo.SetItemData(iItem, window_styles);
	// ==> Feedback personalization [Stulle] - Stulle
	iItem = m_MasterCombo.AddString(GetResString(IDS_COLOR_MASTER7));		m_MasterCombo.SetItemData(iItem, feedback_styles);
	// <== Feedback personalization [Stulle] - Stulle

	m_MasterCombo.SetCurSel(iSel != CB_ERR ? iSel : 0);

	InitSubStyleCombo();

	UpdateStyles();

	return;
}

void CPPgScar::InitSubStyleCombo()
{
	int iSel = m_MasterCombo.GetCurSel();
	int iMasterSel = 0;
	if (iSel >= 0)
		iMasterSel = m_MasterCombo.GetItemData(iSel);
	int iItem;
	m_SubCombo.ResetContent();

	switch(iMasterSel)
	{
		case client_styles: // client styles
		{
			iItem = m_SubCombo.AddString(GetResString(IDS_DEFAULT));		m_SubCombo.SetItemData(iItem, style_c_default);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_C1));		m_SubCombo.SetItemData(iItem, style_c_friend);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_C2));		m_SubCombo.SetItemData(iItem, style_c_powershare);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_C3));		m_SubCombo.SetItemData(iItem, style_c_powerrelease);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_C4));		m_SubCombo.SetItemData(iItem, style_c_downloading);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_C5));		m_SubCombo.SetItemData(iItem, style_c_uploading);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_C6));		m_SubCombo.SetItemData(iItem, style_c_leecher);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_C7));		m_SubCombo.SetItemData(iItem, style_c_lowid);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_C8));		m_SubCombo.SetItemData(iItem, style_c_credits);
		}break;
		case download_styles: // download styles
		{
			iItem = m_SubCombo.AddString(GetResString(IDS_DEFAULT));		m_SubCombo.SetItemData(iItem, style_d_default);
			iItem = m_SubCombo.AddString(GetResString(IDS_DOWNLOADING));	m_SubCombo.SetItemData(iItem, style_d_downloading);
			iItem = m_SubCombo.AddString(GetResString(IDS_COMPLETE));		m_SubCombo.SetItemData(iItem, style_d_complete);
			iItem = m_SubCombo.AddString(GetResString(IDS_COMPLETING));		m_SubCombo.SetItemData(iItem, style_d_completing);
			iItem = m_SubCombo.AddString(GetResString(IDS_HASHING));		m_SubCombo.SetItemData(iItem, style_d_hashing);
			iItem = m_SubCombo.AddString(GetResString(IDS_PAUSED));			m_SubCombo.SetItemData(iItem, style_d_paused);
			iItem = m_SubCombo.AddString(GetResString(IDS_STOPPED));		m_SubCombo.SetItemData(iItem, style_d_stopped);
			iItem = m_SubCombo.AddString(GetResString(IDS_ERRORLIKE));		m_SubCombo.SetItemData(iItem, style_d_errunk);
		}break;
		case share_styles: // share styles
		{
			iItem = m_SubCombo.AddString(GetResString(IDS_DEFAULT));		m_SubCombo.SetItemData(iItem, style_s_default);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_S1));		m_SubCombo.SetItemData(iItem, style_s_incomplete);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_S2));		m_SubCombo.SetItemData(iItem, style_s_powershare);
			CString strTemp = GetResString(IDS_PRIORITY) + _T(" (") + GetResString(IDS_PW_CON_UPLBL) + _T("): ");
			iItem = m_SubCombo.AddString(strTemp + GetResString(IDS_PRIOAUTO));		m_SubCombo.SetItemData(iItem, style_s_auto);
			iItem = m_SubCombo.AddString(strTemp + GetResString(IDS_PRIOVERYLOW));	m_SubCombo.SetItemData(iItem, style_s_verylow);
			iItem = m_SubCombo.AddString(strTemp + GetResString(IDS_PRIOLOW));		m_SubCombo.SetItemData(iItem, style_s_low);
			iItem = m_SubCombo.AddString(strTemp + GetResString(IDS_PRIONORMAL));	m_SubCombo.SetItemData(iItem, style_s_normal);
			iItem = m_SubCombo.AddString(strTemp + GetResString(IDS_PRIOHIGH));		m_SubCombo.SetItemData(iItem, style_s_high);
			iItem = m_SubCombo.AddString(strTemp + GetResString(IDS_PRIORELEASE));	m_SubCombo.SetItemData(iItem, style_s_release);
			iItem = m_SubCombo.AddString(strTemp + GetResString(IDS_POWERRELEASE));	m_SubCombo.SetItemData(iItem, style_s_powerrelease);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_S3));		m_SubCombo.SetItemData(iItem, style_s_shareable);
		}break;
		case server_styles: // server styles
		{
			iItem = m_SubCombo.AddString(GetResString(IDS_DEFAULT));		m_SubCombo.SetItemData(iItem, style_se_default);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_SE1));		m_SubCombo.SetItemData(iItem, style_se_connected);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_SE2));		m_SubCombo.SetItemData(iItem, style_se_static);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_SE3));		m_SubCombo.SetItemData(iItem, style_se_filtered);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_SE4));		m_SubCombo.SetItemData(iItem, style_se_dead);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_SE5));		m_SubCombo.SetItemData(iItem, style_se_unreliable);
		}break;
		case background_styles: // background styles
		{
			iItem = m_SubCombo.AddString(GetResString(IDS_DEFAULT));		m_SubCombo.SetItemData(iItem, style_b_default);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_B1));		m_SubCombo.SetItemData(iItem, style_b_clientlist);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_B2));		m_SubCombo.SetItemData(iItem, style_b_dlclientlist);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_B3));		m_SubCombo.SetItemData(iItem, style_b_queuelist);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_B4));		m_SubCombo.SetItemData(iItem, style_b_uploadlist);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_B5));		m_SubCombo.SetItemData(iItem, style_b_downloadlist);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_B6));		m_SubCombo.SetItemData(iItem, style_b_sharedlist);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_B7));		m_SubCombo.SetItemData(iItem, style_b_serverwnd);
		}break;
		case window_styles: // window styles
		{
			iItem = m_SubCombo.AddString(GetResString(IDS_DEFAULT));		m_SubCombo.SetItemData(iItem, style_w_default);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_W1));		m_SubCombo.SetItemData(iItem, style_w_kademlia);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_W2));		m_SubCombo.SetItemData(iItem, style_w_server);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_W3));		m_SubCombo.SetItemData(iItem, style_w_transfer);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_W4));		m_SubCombo.SetItemData(iItem, style_w_search);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_W5));		m_SubCombo.SetItemData(iItem, style_w_shared);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_W6));		m_SubCombo.SetItemData(iItem, style_w_messages);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_W7));		m_SubCombo.SetItemData(iItem, style_w_irc);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_W8));		m_SubCombo.SetItemData(iItem, style_w_statistic);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_W9));		m_SubCombo.SetItemData(iItem, style_w_statusbar);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_W10));		m_SubCombo.SetItemData(iItem, style_w_toolbar);
		}break;
		// ==> Feedback personalization [Stulle] - Stulle
		case feedback_styles: // feedback styles
		{
			iItem = m_SubCombo.AddString(GetResString(IDS_DEFAULT));		m_SubCombo.SetItemData(iItem, style_f_default);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_F1));		m_SubCombo.SetItemData(iItem, style_f_label);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_F2));		m_SubCombo.SetItemData(iItem, style_f_names);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_F3));		m_SubCombo.SetItemData(iItem, style_f_fileinfo);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_F4));		m_SubCombo.SetItemData(iItem, style_f_filestate);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_F5));		m_SubCombo.SetItemData(iItem, style_f_transferred);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_F6));		m_SubCombo.SetItemData(iItem, style_f_requests);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_F7));		m_SubCombo.SetItemData(iItem, style_f_sources);
			iItem = m_SubCombo.AddString(GetResString(IDS_COLOR_F8));		m_SubCombo.SetItemData(iItem, style_f_clientsonqueue);
			iItem = m_SubCombo.AddString(GetResString(IDS_FEEDBACK_COMPLETE));		m_SubCombo.SetItemData(iItem, style_f_compeltesrc);
		}break;
		// <== Feedback personalization [Stulle] - Stulle
		default:
			break;
	}

	m_SubCombo.SetCurSel(0); // alway first one!
}

void CPPgScar::UpdateStyles()
{
	int iCurStyle = 0;
	int iMasterValue = 0;
	int iSel = m_SubCombo.GetCurSel();
	if (iSel >= 0)
		iCurStyle = m_SubCombo.GetItemData(iSel);
	iSel = m_MasterCombo.GetCurSel();
	if (iSel >= 0)
		iMasterValue = m_MasterCombo.GetItemData(iSel);
	StylesStruct styles;
	styles = GetStyle(iMasterValue, iCurStyle);
	bool bEnable = false;
	bool bOnOff = (iCurStyle != style_c_default);

	if(thePrefs.GetWindowsVersion() >= _WINVER_XP_) // using XP or higher
	{
		// Retrieve the bottom of the tab's header
		RECT rect1;
		RECT rect2;
		m_tabCtr.GetWindowRect(&rect1);
		ScreenToClient(&rect1);
		m_tabCtr.GetItemRect(m_tabCtr.GetItemCount() - 1 , &rect2);
		const int top = rect1.top + (rect2.bottom - rect2.top + 1) * m_tabCtr.GetRowCount() + 10;
		const int left = rect1.left + 6;
//		const int bottom = rect1.bottom-10;
		const int right = rect1.right - 6;

		// set the warning
		if(iMasterValue == window_styles) // window styles
		{
				m_ColorWarning.ShowWindow(SW_SHOW);
				m_ColorWarning.EnableWindow(TRUE);
				m_ColorBox.MoveWindow(CRect(left, top, right, top+224),TRUE);
				m_EasterEgg.MoveWindow(CRect(left, top+234, right, top+259),TRUE); // Diabolic Easteregg [Stulle] - Mephisto
		}
		else
		{
				m_ColorWarning.ShowWindow(SW_HIDE);
				m_ColorWarning.EnableWindow(FALSE);
				m_ColorBox.MoveWindow(CRect(left, top, right, top+175),TRUE);
				m_EasterEgg.MoveWindow(CRect(left, top+185, right, top+210),TRUE); // Diabolic Easteregg [Stulle] - Mephisto
		}
	}

	// set default for background
	if(iMasterValue == window_styles)
		m_BackColor.SetDefaultColor(GetSysColor(COLOR_BTNFACE));
	else
		m_BackColor.SetDefaultColor(COLORREF(RGB(255,255,255)));

	if(bOnOff && iMasterValue < background_styles)
	{
		m_OnOff.EnableWindow(TRUE);
		if(styles.nOnOff != 0)
		{
			bEnable = true;
			m_OnOff.SetWindowText( GetResString(IDS_COLOR_OFF) );
		}
		else
		{
			m_OnOff.SetWindowText( GetResString(IDS_COLOR_ON) );
		}
	}
	else
	{
		bEnable = true;
		m_OnOff.EnableWindow(FALSE);
		m_OnOff.SetWindowText( GetResString(IDS_COLOR_ON) );
	}

	m_bold.EnableWindow(bEnable && (iMasterValue < background_styles || iMasterValue == feedback_styles));
	m_underlined.EnableWindow(bEnable && (iMasterValue < background_styles || iMasterValue == feedback_styles));
	m_italic.EnableWindow(bEnable && (iMasterValue < background_styles || iMasterValue == feedback_styles));
	m_FontColor.EnableWindow(bEnable && (iMasterValue < background_styles || iMasterValue == feedback_styles));
	m_BackColor.EnableWindow(bEnable && iMasterValue != feedback_styles);

	if(bEnable)
	{
		//int iStyle = (styles.nFlags & STYLE_FONTMASK); // not needed, we do it bitwise
		m_bold.SetCheck(styles.nFlags & STYLE_BOLD ? 1:0);
		m_bBold = (styles.nFlags & STYLE_BOLD) ? true : false;
		m_underlined.SetCheck(styles.nFlags & STYLE_UNDERLINE ? 1:0);
		m_bUnderlined = (styles.nFlags & STYLE_UNDERLINE) ? true : false;
		m_italic.SetCheck(styles.nFlags & STYLE_ITALIC ? 1:0);
		m_bItalic = (styles.nFlags & STYLE_ITALIC) ? true : false;
		m_FontColor.SetColor(styles.nFontColor);
		m_BackColor.SetColor(styles.nBackColor);
	}

	RedrawWindow(); // work around all glitches. :D
}

void CPPgScar::OnFontStyle()
{
	// Get font style
	int iStyle = 0 ;
	if (m_bold.GetCheck())
		iStyle |= STYLE_BOLD;
	if (m_underlined.GetCheck())
		iStyle |= STYLE_UNDERLINE;
	if (m_italic.GetCheck())
		iStyle |= STYLE_ITALIC;

	int iCurStyle = 0;
	int iMasterValue = 0;
	int iSel = m_SubCombo.GetCurSel();
	if (iSel >= 0)
		iCurStyle = m_SubCombo.GetItemData(iSel);
	iSel = m_MasterCombo.GetCurSel();
	if (iSel >= 0)
		iMasterValue = m_MasterCombo.GetItemData(iSel);
	StylesStruct styles;
	styles = GetStyle(iMasterValue, iCurStyle);

	DWORD flags = (styles.nFlags & ~STYLE_FONTMASK) | iStyle;
	if (flags != styles.nFlags)
	{
		SetModified();
		styles.nFlags = flags;
		SetStyle(iMasterValue, iCurStyle, &styles);
		UpdateStyles();
		m_bDesignChanged = true;
	}
}

LONG CPPgScar::OnColorPopupSelChange(UINT /*lParam*/, LONG /*wParam*/)
{
	int iCurStyle = 0;
	int iMasterValue = 0;
	int iSel = m_SubCombo.GetCurSel();
	if (iSel >= 0)
		iCurStyle = m_SubCombo.GetItemData(iSel);
	iSel = m_MasterCombo.GetCurSel();
	if (iSel >= 0)
		iMasterValue = m_MasterCombo.GetItemData(iSel);
	StylesStruct styles;
	styles = GetStyle(iMasterValue, iCurStyle);

	// font color
	if (iCurStyle >= 0)
	{
		COLORREF crColor = m_FontColor.GetColor();
		if (crColor != styles.nFontColor)
		{
			styles.nFontColor = crColor;
			SetModified(TRUE);
			SetStyle(iMasterValue, iCurStyle, &styles);
			UpdateStyles();
			m_bDesignChanged = true;
		}
	}

	// background color
	if (iCurStyle >= 0)
	{
		COLORREF crColor = m_BackColor.GetColor();
		if (crColor != styles.nBackColor)
		{
			styles.nBackColor = crColor;
			SetModified(TRUE);
			SetStyle(iMasterValue, iCurStyle, &styles);
			UpdateStyles();
			m_bDesignChanged = true;
		}
	}
	return TRUE;
}

void CPPgScar::OnBnClickedBold()
{
	// reverse 
	if(m_bBold)
	{
		m_bold.SetCheck(FALSE);
		m_bBold = false;
	}
	else
	{
		m_bold.SetCheck(TRUE);
		m_bBold = true;
	}

	OnFontStyle();
}

void CPPgScar::OnBnClickedUnderlined()
{
	// reverse 
	if(m_bUnderlined)
	{
		m_underlined.SetCheck(FALSE);
		m_bUnderlined = false;
	}
	else
	{
		m_underlined.SetCheck(TRUE);
		m_bUnderlined = true;
	}

	OnFontStyle();
}

void CPPgScar::OnBnClickedItalic()
{
	// reverse 
	if(m_bItalic)
	{
		m_italic.SetCheck(FALSE);
		m_bItalic = false;
	}
	else
	{
		m_italic.SetCheck(TRUE);
		m_bItalic = true;
	}

	OnFontStyle();
}

void CPPgScar::OnCbnSelchangeStyleselMaster()
{
	InitSubStyleCombo();
	UpdateStyles();
	if(m_bFocusWasOnCombo)
		m_MasterCombo.SetFocus();
	m_bFocusWasOnCombo = false;
}

void CPPgScar::OnCbnSelchangeStyleselSub()
{
	UpdateStyles();
	if(m_bFocusWasOnCombo)
		m_SubCombo.SetFocus();
	m_bFocusWasOnCombo = false;
}

void CPPgScar::OnBnClickedOnOff()
{
	int iCurStyle = 0;
	int iMasterValue = 0;
	int iSel = m_SubCombo.GetCurSel();
	if (iSel >= 0)
		iCurStyle = m_SubCombo.GetItemData(iSel);
	iSel = m_MasterCombo.GetCurSel();
	if (iSel >= 0)
		iMasterValue = m_MasterCombo.GetItemData(iSel);
	StylesStruct styles;
	styles = GetStyle(iMasterValue, iCurStyle);

	short sOnOff = styles.nOnOff;

	if(sOnOff == 1)
		sOnOff = 0;
	else
		sOnOff = 1;
	styles.nOnOff = sOnOff;

	SetStyle(iMasterValue, iCurStyle, &styles);
	UpdateStyles();
	SetModified(TRUE);
	m_bDesignChanged = true;
}

StylesStruct CPPgScar::GetStyle(int nMaster, int nStyle)
{
	if(nMaster == client_styles)
		return nClientStyles[nStyle];
	else if(nMaster == download_styles)
		return nDownloadStyles[nStyle];
	else if(nMaster == share_styles)
		return nShareStyles[nStyle];
	else if(nMaster == server_styles)
		return nServerStyles[nStyle];
	else if(nMaster == background_styles)
		return nBackgroundStyles[nStyle];
	else if(nMaster == window_styles)
		return nWindowStyles[nStyle];
	// ==> Feedback personalization [Stulle] - Stulle
	else if(nMaster == feedback_styles)
		return nFeedBackStyles[nStyle];
	// <== Feedback personalization [Stulle] - Stulle
	return nClientStyles[nStyle];
}

void CPPgScar::SetStyle(int nMaster, int nStyle, StylesStruct *style)
{
	switch(nMaster)
	{
		case client_styles:
			{
				nClientStyles[nStyle].nFlags = style->nFlags;
				nClientStyles[nStyle].nFontColor = style->nFontColor;
				nClientStyles[nStyle].nBackColor = style->nBackColor;
				nClientStyles[nStyle].nOnOff = style->nOnOff;
			}break;
		case download_styles:
			{
				nDownloadStyles[nStyle].nFlags = style->nFlags;
				nDownloadStyles[nStyle].nFontColor = style->nFontColor;
				nDownloadStyles[nStyle].nBackColor = style->nBackColor;
				nDownloadStyles[nStyle].nOnOff = style->nOnOff;
			}break;
		case share_styles:
			{
				nShareStyles[nStyle].nFlags = style->nFlags;
				nShareStyles[nStyle].nFontColor = style->nFontColor;
				nShareStyles[nStyle].nBackColor = style->nBackColor;
				nShareStyles[nStyle].nOnOff = style->nOnOff;
			}break;
		case server_styles:
			{
				nServerStyles[nStyle].nFlags = style->nFlags;
				nServerStyles[nStyle].nFontColor = style->nFontColor;
				nServerStyles[nStyle].nBackColor = style->nBackColor;
				nServerStyles[nStyle].nOnOff = style->nOnOff;
			}break;
		case background_styles:
			{
				nBackgroundStyles[nStyle].nBackColor = style->nBackColor;
			}break;
		case window_styles:
			{
				nWindowStyles[nStyle].nBackColor = style->nBackColor;
			}break;
			// ==> Feedback personalization [Stulle] - Stulle
		case feedback_styles:
			{
				nFeedBackStyles[nStyle].nFlags = style->nFlags;
				nFeedBackStyles[nStyle].nFontColor = style->nFontColor;
			}break;
			// <== Feedback personalization [Stulle] - Stulle
		default:
			break;
	}
}

void CPPgScar::OnEnKillfocusMasterCombo()
{
	m_bFocusWasOnCombo = true;
}

void CPPgScar::OnEnKillfocusSubCombo()
{
	m_bFocusWasOnCombo = true;
}

// ==> Diabolic Easteregg [Stulle] - Mephisto
void CPPgScar::OnBnClickedEasteregg()
{
	if (AfxMessageBox(GetResString(IDS_EASTEREGG_PROMPT),MB_YESNO)==IDYES)
	{
		AfxMessageBox(GetResString(IDS_EASTEREGG_INSTRUCTION),MB_OK);
		thePrefs.m_bCloseEasteregg=true;
	}
}
// <== Diabolic Easteregg [Stulle] - Mephisto

// <== Design Settings [eWombat/Stulle] - Stulle

// ==> Advanced Updates [MorphXT/Stulle] - Stulle
void CPPgScar::OnBnClickedUpdateALUrl()
{
	OnApply();
	theApp.emuledlg->DoDLPVersioncheck();
}

void CPPgScar::OnBnClickedResetALUrl()
{
	CString strBuffer = _T("http://downloads.sourceforge.net/emulextreme/antiLeech.dll.new");
	m_AntiLeechURL.SetWindowText(strBuffer);
}

void CPPgScar::OnBnClickedUpdateipfurl()
{
	OnApply();
	theApp.emuledlg->CheckIPFilter();
	if(thePrefs.IsIPFilterViaDynDNS())
	{
		CString strBuffer=NULL;
		if(PathFileExists(theApp.ipfilter->GetDefaultFilePath()))
			strBuffer.Format(_T("v%u"), thePrefs.GetIPFilterVersionNum());
		else
			strBuffer=GetResString(IDS_DL_NONE);
		m_IpFilterTime.SetWindowText(strBuffer);
	}
	else
	{
		TCHAR sTime[30];
		sTime[0] = _T('\0');
		SysTimeToStr(thePrefs.GetIPfilterVersion(), sTime);
		m_IpFilterTime.SetWindowText(sTime);
	}
}

void CPPgScar::OnBnClickedResetipfurl()
{
	CString strBuffer = _T("http://downloads.sourceforge.net/scarangel/ipfilter.rar");
	m_IpFilterURL.SetWindowText(strBuffer);
	memset(thePrefs.GetIPfilterVersion(), 0, sizeof(SYSTEMTIME));
	thePrefs.m_uIPFilterVersionNum = 0;
	if(PathFileExists(theApp.ipfilter->GetDefaultFilePath()))
		strBuffer = _T("v0");
	else
		strBuffer = GetResString(IDS_DL_NONE);
	m_IpFilterTime.SetWindowText(strBuffer);
}

void CPPgScar::OnBnClickedUpdateipcurl()
{
	OnApply();
	theApp.ip2country->UpdateIP2CountryURL();
	TCHAR sBuffer[30];
	sBuffer[0] = _T('\0'); 
	SysTimeToStr(thePrefs.GetIP2CountryVersion(), sBuffer);
	m_CountryTime.SetWindowText(sBuffer);
}

void CPPgScar::OnBnClickedResetipcurl()
{
	CString strBuffer = _T("http://ip-to-country.webhosting.info/downloads/ip-to-country.csv.zip");
	m_CountryURL.SetWindowText(strBuffer);
	memset(thePrefs.GetIP2CountryVersion(), 0, sizeof(SYSTEMTIME));
	m_CountryTime.SetWindowText(_T(""));
}
// <== Advanced Updates [MorphXT/Stulle] - Stulle