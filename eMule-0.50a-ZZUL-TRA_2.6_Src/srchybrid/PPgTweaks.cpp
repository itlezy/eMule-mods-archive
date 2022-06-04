//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "SearchDlg.h"
#include "PPgTweaks.h"
#include "Scheduler.h"
#include "DownloadQueue.h"
#include "Preferences.h"
#include "OtherFunctions.h"
#include "TransferDlg.h"
#include "emuledlg.h"
#include "SharedFilesWnd.h"
#include "ServerWnd.h"
#include "Log.h"
#include "UserMsgs.h"
#include "opcodes.h"// NEO: QS - [QuickStart]
// ZZUL-TRA :: InvisibleMode :: Start
#include "Addons\InvisibleMod\TreeOptionsInvisibleModCombo.h"
#include "Addons\InvisibleMod\TreeOptionsInvisibleKeyCombo.h"
// ZZUL-TRA :: InvisibleMode :: End
#include "Addons/IP2Country/IP2Country.h" //>>> shadow2004::IP2Country [EastShare]
#include "sharedfilelist.h" //ZZUL-TRA :: PowerShare

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define	DFLT_MAXCONPERFIVE	20
#define DFLT_MAXHALFOPEN	9

///////////////////////////////////////////////////////////////////////////////
// CPPgTweaks dialog

IMPLEMENT_DYNAMIC(CPPgTweaks, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgTweaks, CPropertyPage)
	ON_WM_HSCROLL()
	ON_WM_DESTROY()
	ON_MESSAGE(UM_TREEOPTSCTRL_NOTIFY, OnTreeOptsCtrlNotify)
	ON_BN_CLICKED(IDC_OPENPREFINI, OnBnClickedOpenprefini)
END_MESSAGE_MAP()

CPPgTweaks::CPPgTweaks()
	: CPropertyPage(CPPgTweaks::IDD)
	, m_ctrlTreeOptions(theApp.m_iDfltImageListColorFlags)
{
	m_iFileBufferSize = 0;
	m_iQueueSize = 0;
	m_iMaxConnPerFive = 0;
	m_iMaxHalfOpen = 0;
	m_bConditionalTCPAccept = false;
	m_bAutoTakeEd2kLinks = false;
	m_bVerbose = false;
	m_bDebugSourceExchange = false;
	m_bLogBannedClients = false;
	m_bLogRatingDescReceived = false;
	m_bLogSecureIdent = false;
	m_bLogFilteredIPs = false;
	m_bLogFileSaving = false;
	m_bLogA4AF = false;
	m_bLogUlDlEvents = false;
#ifdef CLIENTANALYZER
	m_iCreditSystem = 0; //>>> WiZaRd::ClientAnalyzer
	m_bLogAnalyzerEvents = false;
#else
	m_bCreditSystem = false;
#endif
	m_bLog2Disk = false;
	m_bDebug2Disk = false;
	m_iCommitFiles = 0;
	m_bFilterLANIPs = false;
	m_bExtControls = false;
	m_uServerKeepAliveTimeout = 0;
	m_bSparsePartFiles = false;
	m_bFullAlloc = false;
	m_bCheckDiskspace = false;
	m_fMinFreeDiskSpaceMB = 0.0F;
	(void)m_sYourHostname;
	m_bFirewallStartup = false;
	m_iLogLevel = 0;
    m_bDynUpEnabled = false;
    m_iDynUpMinUpload = 0;
    m_iDynUpPingTolerance = 0;
    m_iDynUpGoingUpDivider = 0;
    m_iDynUpGoingDownDivider = 0;
    m_iDynUpNumberOfPings = 0;
    m_bA4AFSaveCpu = false;
	m_iExtractMetaData = 0;
	m_bAutoArchDisable = true;
	m_bCloseUPnPOnExit = true;
	m_bSkipWANIPSetup = false;
	m_bSkipWANPPPSetup = false;
	m_iShareeMule = 0;
	m_bResolveShellLinks = false;

	bShowedWarning = false;
	m_bInitializedTreeOpts = false;
	m_htiTCPGroup = NULL;
	m_htiMaxCon5Sec = NULL;
	m_htiMaxHalfOpen = NULL;
	m_htiConditionalTCPAccept = NULL;
	m_htiAutoTakeEd2kLinks = NULL;
	m_htiVerboseGroup = NULL;
	m_htiVerbose = NULL;
	m_htiDebugSourceExchange = NULL;
	m_htiLogBannedClients = NULL;
	m_htiLogRatingDescReceived = NULL;
	m_htiLogSecureIdent = NULL;
	m_htiLogFilteredIPs = NULL;
	m_htiLogFileSaving = NULL;
	m_htiLogUlDlEvents = NULL;
	m_htiCreditSystem = NULL;
	m_htiLog2Disk = NULL;
	m_htiDebug2Disk = NULL;
	m_htiCommit = NULL;
	m_htiCommitNever = NULL;
	m_htiCommitOnShutdown = NULL;
	m_htiCommitAlways = NULL;
	m_htiFilterLANIPs = NULL;
	m_htiExtControls = NULL;
	m_htiServerKeepAliveTimeout = NULL;
	m_htiSparsePartFiles = NULL;
	m_htiFullAlloc = NULL;
	m_htiCheckDiskspace = NULL;
	m_htiMinFreeDiskSpace = NULL;
	m_htiYourHostname = NULL;
	m_htiFirewallStartup = NULL;
	m_htiLogLevel = NULL;
    m_htiDynUp = NULL;
	m_htiDynUpEnabled = NULL;
    m_htiDynUpMinUpload = NULL;
    m_htiDynUpPingTolerance = NULL;
    m_htiDynUpPingToleranceMilliseconds = NULL;
    m_htiDynUpPingToleranceGroup = NULL;
    m_htiDynUpRadioPingTolerance = NULL;
    m_htiDynUpRadioPingToleranceMilliseconds = NULL;
    m_htiDynUpGoingUpDivider = NULL;
    m_htiDynUpGoingDownDivider = NULL;
    m_htiDynUpNumberOfPings = NULL;
    m_htiA4AFSaveCpu = NULL;
	m_htiLogA4AF = NULL;
	m_htiExtractMetaData = NULL;
	m_htiAutoArch = NULL;
	m_htiUPnP = NULL;
	m_htiCloseUPnPPorts = NULL;
	m_htiSkipWANIPSetup = NULL;
	m_htiSkipWANPPPSetup = NULL;
	m_htiShareeMule = NULL;
	m_htiShareeMuleMultiUser = NULL;
	m_htiShareeMulePublicUser = NULL;
	m_htiShareeMuleOldStyle = NULL;
	m_htiResolveShellLinks = NULL;

// ZZUL-TRA :: Advanced official preferences :: Start
	bMiniMuleAutoClose=false;
	iMiniMuleTransparency=0;
	bCreateCrashDump=false;
	bIgnoreInstances=false;
	iMaxLogBuff=0;
	m_iMaxChatHistory=0;
	m_iPreviewSmallBlocks=0;
	m_bRestoreLastMainWndDlg=false;
	m_bRestoreLastLogPane=false;
	m_bPreviewCopiedArchives=false;
	m_iStraightWindowStyles=0;
	m_iLogFileFormat=0;
	m_bRTLWindowsLayout=false;
	m_bPreviewOnIconDblClk=false;
	m_bUseUserSortedServerList=false;
	iServerUDPPort=65535;
	m_bRemoveFilesToBin=false;
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	m_iDebugSearchResultDetailLevel=0;
#endif
	m_hti_advanced=NULL;
	m_hti_bMiniMuleAutoClose=NULL;
	m_hti_iMiniMuleTransparency=NULL;
	m_hti_bCreateCrashDump=NULL;
	m_hti_bIgnoreInstances=NULL;
	m_hti_sMediaInfo_MediaInfoDllPath=NULL;
	m_hti_iMaxLogBuff=NULL;
	m_hti_m_iMaxChatHistory=NULL;
	m_hti_m_iPreviewSmallBlocks=NULL;
	m_hti_m_bRestoreLastMainWndDlg=NULL;
	m_hti_m_bRestoreLastLogPane=NULL;
	m_hti_m_bPreviewCopiedArchives=NULL;
	m_hti_m_iStraightWindowStyles=NULL;
	m_hti_m_iLogFileFormat=NULL;
	m_hti_m_bRTLWindowsLayout=NULL;
	m_hti_m_bPreviewOnIconDblClk=NULL;
	m_hti_sInternetSecurityZone=NULL;
	m_hti_sTxtEditor=NULL;
	m_hti_iServerUDPPort=NULL;
	m_hti_m_bRemoveFilesToBin=NULL;
	m_hti_HighresTimer=NULL;
	m_hti_TrustEveryHash=NULL;
	m_hti_InspectAllFileTypes=NULL;
	m_hti_maxmsgsessions=NULL;
	m_hti_PreferRestrictedOverUser=NULL;
	m_hti_UseUserSortedServerList=NULL;
	m_hti_WebFileUploadSizeLimitMB =NULL;
	m_hti_AllowedIPs=NULL;
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	m_hti_DebugSearchResultDetailLevel=NULL;
#endif
	m_htiCryptTCPPaddingLength=NULL;
	m_htidatetimeformat = NULL;
	m_htidatetimeformat4log = NULL;
	m_htidatetimeformat4list = NULL;
	m_htiLogError = NULL;
	m_htiLogWarning = NULL;
	m_htiLogSuccess = NULL;
	m_htidontcompressavi = NULL;
	m_htiShowCopyEd2kLinkCmd = NULL;
	m_htiIconflashOnNewMessage = NULL;
	m_htiReBarToolbar = NULL;
	m_htiICH = NULL;
	m_htiShowVerticalHourMarkers = NULL;
    m_htiAdjustNTFSDaylightFileTime = NULL;
    m_htidontcompressavi = NULL;
	m_htiFileBufferTimeLimit = NULL;
	m_htiRearrangeKadSearchKeywords = NULL;
	m_htiUpdateQueue = NULL;
	m_htiRepaint = NULL;
	m_htiBeeper = NULL;
	m_htiMsgOnlySec = NULL;
	m_htiExtraPreviewWithMenu = NULL;
	m_htiShowUpDownIconInTaskbar = NULL;
	m_htiKeepUnavailableFixedSharedDirs = NULL;
	m_htiForceSpeedsToKB = NULL;
// ZZUL-TRA :: Advanced official preferences :: End

/////////////////////////////////////////////////////////////////////////////
// ZZUL-TRA :: Start

	m_htiZZULTRA = NULL; //ZZUL-TRA

    m_bLogEmulator = NULL; // ZZUL-TRA :: EmulateOthers
    m_htiLogEmulator = NULL; // ZZUL-TRA :: EmulateOthers
    m_iDisconnectTime = NULL;
    m_htiDisconnectTime = NULL;

	m_htiShowActiveDownloadsBold = NULL;
	m_htiShowRuntimeOnTitle = NULL;
	m_htiShowDownloadColor = NULL;
	m_htiShowFileStatusIcons = NULL;
	m_htiDirectoryWatcher = NULL;
    m_bDirectoryWatcher = false;
	m_htiAutoDropSystem = NULL;
	m_htiFunnyNick = NULL;
	m_htiShowSessionDownload = NULL;
	m_htiUsePercentBar = NULL; // ZZUL-TRA :: PercentBar

// ZZUL-TRA :: PowerShare :: Start
	m_htiPowerShareLimit = NULL; 
	m_htiPsAmountLimit = NULL; 
	m_htiPowershareMode = NULL;
	m_htiPowershareDisabled = NULL;
	m_htiPowershareActivated = NULL;
	m_htiPowershareAuto = NULL;
	m_htiPowershareLimited = NULL;
// ZZUL-TRA :: PowerShare :: End

// ZZUL-TRA :: InvisibleMode :: Start
	m_bInvisibleMode = false;
	// ZZUL-TRA :: InvisibleMode :: End

#ifdef CLIENTANALYZER
//>>> WiZaRd::ClientAnalyzer
	m_htiCSNone = NULL;
	m_htiCSOffi = NULL;
	m_htiCSAnalyzer = NULL;
//<<< WiZaRd::ClientAnalyzer
	m_htiLogAnalyzerEvents = NULL;
#endif
    m_htiIsPayBackFirst = NULL; //Pay BackFirst
	m_htiPayBackFirstLimit = NULL; //Pay BackFirst
 
	// ZZUL-TRA :: Global Hardlimit :: Start
	m_htiIsGlobalHardlimit = NULL; 
	m_htiGlobalHardlimit = NULL;
	// ZZUL-TRA :: Global Hardlimit :: End

	m_htiGetCPU = false; // ZZUL-TRA :: SysInfo

	// ZZUL-TRA :: InvisibleMode :: Start 
	m_htiInvisibleMode = NULL;
	m_htiInvisibleModeMod = NULL;
	m_htiInvisibleModeKey = NULL;
	// ZZUL-TRA :: InvisibleMode :: End

    //>>> shadow2004::IP2Country [EastShare]
    m_htiI2CGroup = NULL;
	m_htiI2C2Letter = NULL;
	m_htiI2C3Letter = NULL;
	m_htiI2CFullCaption = NULL;
	m_htiI2CShowFlag = NULL;
	m_iI2CCaption = 0;
	m_bI2CShowFlag = false;
    //m_htiIP2CUpdate = NULL;
	//m_bIP2CUpdate = false;
    //<<< shadow2004::IP2Country [EastShare]
// NEO: QS - [QuickStart]
	m_htiQuickStartEnable = NULL;
	m_htiQuickStartTime = NULL;
	m_htiQuickStartTimePerFile = NULL;
	m_htiQuickMaxConperFive = NULL;
	m_htiQuickMaxHalfOpen = NULL;
	m_htiQuickMaxConnections = NULL;
	// NEO: QS END

// ZZUL-TRA :: End
///////////////////////////////////////////////////////////////////
}

CPPgTweaks::~CPPgTweaks()
{
}

void CPPgTweaks::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FILEBUFFERSIZE, m_ctlFileBuffSize);
	DDX_Control(pDX, IDC_QUEUESIZE, m_ctlQueueSize);
	DDX_Control(pDX, IDC_EXT_OPTS, m_ctrlTreeOptions);
	if (!m_bInitializedTreeOpts)
	{
		int iImgBackup = 8; // default icon
		int iImgLog = 8;
		int iImgDynyp = 8;
		int iImgConnection = 8;
		int iImgA4AF = 8;
		int iImgMetaData = 8;
		int iImgUPnP = 8;
		int iImgShareeMule = 8;
		int	iImgI2C = 8; //>>> shadow2004::IP2Country [EastShare]
 		int iImgPowerShare = 8; //ZZUL-TRA :: PowerShare
        int iImgZZULTRA = 8; //ZZUL-TRA
    CImageList* piml = m_ctrlTreeOptions.GetImageList(TVSIL_NORMAL);
		if (piml){
			iImgBackup =	piml->Add(CTempIconLoader(_T("HARDDISK")));
			iImgLog =		piml->Add(CTempIconLoader(_T("LOG")));
			iImgDynyp =		piml->Add(CTempIconLoader(_T("UPLOAD")));
			iImgConnection=	piml->Add(CTempIconLoader(_T("CONNECTION")));
            iImgA4AF =		piml->Add(CTempIconLoader(_T("DOWNLOAD")));
            iImgMetaData =	piml->Add(CTempIconLoader(_T("MEDIAINFO")));
			iImgUPnP =		piml->Add(CTempIconLoader(_T("CONNECTEDHIGHHIGH")));
			iImgShareeMule =piml->Add(CTempIconLoader(_T("VIEWFILES")));
			iImgI2C	=	piml->Add(CTempIconLoader(L"SEARCHMETHOD_GLOBAL")); //>>> shadow2004::IP2Country [EastShare]
			iImgPowerShare = piml->Add(CTempIconLoader(_T("POWERSHARE"))); //ZZUL-TRA :: PowerShare
			iImgZZULTRA = piml->Add(CTempIconLoader(_T("ZZULTRA"))); //ZZUL-TRA
		}

		/////////////////////////////////////////////////////////////////////////////
		// ZZUL-TRA group
		//

		m_htiZZULTRA = m_ctrlTreeOptions.InsertGroup(_T("ZZUL-TRA ") + GetResString(IDS_META_PREFERENCES), iImgZZULTRA, TVI_ROOT); //ZZUL-TRA

        //PayBack First
		m_htiIsPayBackFirst = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PAYBACKFIRST), m_htiZZULTRA, m_bIsPayBackFirst);
		m_htiPayBackFirstLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_PAYBACKFIRSTLIMIT),TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,m_htiIsPayBackFirst);
		m_ctrlTreeOptions.AddEditBox(m_htiPayBackFirstLimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_ctrlTreeOptions.Expand(m_htiIsPayBackFirst, TVE_EXPAND);
		//PayBack First

		// ZZUL-TRA :: Global Hardlimit :: Start
		m_htiIsGlobalHardlimit = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PRF_DOWN_GHL_CHK), m_htiZZULTRA, m_bIsGlobalHardlimit);
		m_htiGlobalHardlimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_DL_SOURCES),TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,m_htiIsGlobalHardlimit);
		m_ctrlTreeOptions.AddEditBox(m_htiGlobalHardlimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_ctrlTreeOptions.Expand(m_htiIsGlobalHardlimit, TVE_EXPAND);
		// ZZUL-TRA :: Global Hardlimit :: End

		m_htiShowActiveDownloadsBold = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DOWNLOADS_BOLD), m_htiZZULTRA, m_bShowActiveDownloadsBold);
		m_htiUsePercentBar = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PERCENT_BAR), m_htiZZULTRA, m_bUsePercentBar); // ZZUL-TRA :: PercentBar
		m_htiShowRuntimeOnTitle = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_RUNTIME_NICK), m_htiZZULTRA, m_bShowRuntimeOnTitle);
		m_htiShowDownloadColor = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DOWNLOAD_COLOR), m_htiZZULTRA, m_bShowDownloadColor);
		m_htiShowFileStatusIcons = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_STATUS_ICONS), m_htiZZULTRA, m_bShowFileStatusIcons);
		m_htiShowSessionDownload = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SESSION_DOWNLOAD), m_htiZZULTRA, m_bShowSessionDownload);
	    m_htiGetCPU = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SHOW_CPU), m_htiZZULTRA, m_bGetCPU); // ZZUL-TRA :: SysInfo
		m_htiDirectoryWatcher = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DIRECTORY_WATCHER), m_htiZZULTRA, m_bDirectoryWatcher);
		m_htiAutoDropSystem = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_AUTO_DROP), m_htiZZULTRA, m_bAutoDropSystem);
		m_htiFunnyNick = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_FUNNY_NICK), m_htiZZULTRA, m_bFunnyNick);
		m_htiDisconnectTime = m_ctrlTreeOptions.InsertItem(GetResString(IDS_DISCONNECT_TIME), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiZZULTRA);
		m_ctrlTreeOptions.AddEditBox(m_htiDisconnectTime, RUNTIME_CLASS(CNumTreeOptionsEdit));

//ZZUL-TRA :: PowerShare :: Start
		m_htiPowershareMode = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_POWERSHARE), iImgPowerShare, m_htiZZULTRA);
		m_htiPowershareDisabled = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_DISABLED), m_htiPowershareMode, m_iPowershareMode == 0);
		m_htiPowershareActivated =  m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_ENABLED), m_htiPowershareMode, m_iPowershareMode == 1);
		m_htiPowershareAuto =  m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_PRIOAUTO), m_htiPowershareMode, m_iPowershareMode == 2);
		m_htiPowershareLimited =  m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_POWERSHARE_LIMITED), m_htiPowershareMode, m_iPowershareMode == 3);
		m_htiPowerShareLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_POWERSHARE_LIMIT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiPowershareLimited );
		m_ctrlTreeOptions.AddEditBox(m_htiPowerShareLimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiPsAmountLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_PS_AMOUNT_LIMIT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiPowershareLimited );
		m_ctrlTreeOptions.AddEditBox(m_htiPsAmountLimit, RUNTIME_CLASS(CNumTreeOptionsEdit));	
		m_ctrlTreeOptions.Expand(m_htiPowershareLimited, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiPowershareMode, TVE_EXPAND);
//ZZUL-TRA :: PowerShare :: End

		// NEO: QS - [QuickStart]
		m_htiQuickStartEnable = m_ctrlTreeOptions.InsertCheckBox(_T("")/*GetResString(IDS_X_QUICK_START_ENABLE)*/,m_htiZZULTRA, m_uQuickStart, 0);
		m_htiQuickStartTime = m_ctrlTreeOptions.InsertItem(_T("")/*GetResString(IDS_X_QUICK_START_TIME)*/, TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiQuickStartEnable);
		m_ctrlTreeOptions.AddEditBox(m_htiQuickStartTime,RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiQuickStartTimePerFile = m_ctrlTreeOptions.InsertItem(_T("")/*GetResString(IDS_X_QUICK_START_TIME_PER_FILE)*/, TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiQuickStartEnable);
		m_ctrlTreeOptions.AddEditBox(m_htiQuickStartTimePerFile,RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiQuickMaxConperFive = m_ctrlTreeOptions.InsertItem(_T("")/*GetResString(IDS_X_QUICK_START_MAXPER5)*/, TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiQuickStartEnable);
		m_ctrlTreeOptions.AddEditBox(m_htiQuickMaxConperFive,RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiQuickMaxHalfOpen = m_ctrlTreeOptions.InsertItem(_T("")/*GetResString(IDS_X_QUICK_START_MAXHALF)*/, TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiQuickStartEnable);
		m_ctrlTreeOptions.AddEditBox(m_htiQuickMaxHalfOpen,RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiQuickMaxConnections = m_ctrlTreeOptions.InsertItem(_T("")/*GetResString(IDS_X_QUICK_START_MAXCON)*/, TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiQuickStartEnable);
		m_ctrlTreeOptions.AddEditBox(m_htiQuickMaxConnections,RUNTIME_CLASS(CNumTreeOptionsEdit));
        m_ctrlTreeOptions.Expand(m_htiQuickStartEnable, TVE_EXPAND);
		// NEO: QS END

        // ZZUL-TRA :: InvisibleMode :: Start
		m_htiInvisibleMode = m_ctrlTreeOptions.InsertCheckBox(_T("")/*GetResString(IDS_INVMODE)*/, m_htiZZULTRA, m_bInvisibleMode);
		m_htiInvisibleModeMod = m_ctrlTreeOptions.InsertItem(_T("")/*GetResString(IDS_INVMODE_MODKEY)*/, TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiInvisibleMode);
		m_ctrlTreeOptions.AddComboBox(m_htiInvisibleModeMod, RUNTIME_CLASS(CTreeOptionsInvisibleModCombo));
		m_htiInvisibleModeKey = m_ctrlTreeOptions.InsertItem(_T("")/*GetResString(IDS_INVMODE_VKEY)*/, TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiInvisibleMode);
		m_ctrlTreeOptions.AddComboBox(m_htiInvisibleModeKey, RUNTIME_CLASS(CTreeOptionsInvisibleKeyCombo));
		// ZZUL-TRA :: InvisibleMode :: End

		//>>> shadow2004::IP2Country [EastShare]
		m_htiI2CGroup		= m_ctrlTreeOptions.InsertGroup(GetResString(IDS_IP2COUNTRY), iImgI2C, m_htiZZULTRA);
		m_htiI2C2Letter		= m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_IP2COUNTRY_2LETTERS), m_htiI2CGroup, m_iI2CCaption == 0);
		m_htiI2C3Letter		= m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_IP2COUNTRY_3LETTERS), m_htiI2CGroup, m_iI2CCaption == 1);
		m_htiI2CFullCaption	= m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_IP2COUNTRY_FULLCAPTION), m_htiI2CGroup, m_iI2CCaption == 2);
		m_htiI2CShowFlag	= m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_IP2COUNTRY_SHOWFLAGS), m_htiI2CGroup, m_bI2CShowFlag);
		//m_htiIP2CUpdate     = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MIMUPDATE), m_htiI2CGroup, m_bIP2CUpdate); 
		m_ctrlTreeOptions.Expand(m_htiI2CGroup, TVE_EXPAND);
		//<<< shadow2004::IP2Country [EastShare]

		/////////////////////////////////////////////////////////////////////////////
		// TCP/IP group
		//
		m_htiTCPGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_TCPIP_CONNS), iImgConnection, TVI_ROOT);
		m_htiMaxCon5Sec = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MAXCON5SECLABEL), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiTCPGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiMaxCon5Sec, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiMaxHalfOpen = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MAXHALFOPENCONS), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiTCPGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiMaxHalfOpen, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiConditionalTCPAccept = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_CONDTCPACCEPT), m_htiTCPGroup, m_bConditionalTCPAccept);
		m_htiServerKeepAliveTimeout = m_ctrlTreeOptions.InsertItem(GetResString(IDS_SERVERKEEPALIVETIMEOUT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiTCPGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiServerKeepAliveTimeout, RUNTIME_CLASS(CNumTreeOptionsEdit));

#ifdef CLIENTANALYZER
//>>> WiZaRd::ClientAnalyzer
		m_htiCreditSystem = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_USECREDITSYSTEM), iImgDynyp, TVI_ROOT);
		m_htiCSNone = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_CS_NONE), m_htiCreditSystem, m_iCreditSystem == 0);
		m_htiCSOffi = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_CS_OFFI), m_htiCreditSystem, m_iCreditSystem == 1);
		m_htiCSAnalyzer = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_CS_ANALYZER), m_htiCreditSystem, m_iCreditSystem == 2);
//<<< WiZaRd::ClientAnalyzer
#else
		m_htiCreditSystem = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_USECREDITSYSTEM), TVI_ROOT, m_bCreditSystem);
#endif
		/////////////////////////////////////////////////////////////////////////////
		// Miscellaneous group
		//
		m_htiAutoTakeEd2kLinks = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_AUTOTAKEED2KLINKS), TVI_ROOT, m_bAutoTakeEd2kLinks);
		m_htiFirewallStartup = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_FO_PREF_STARTUP), TVI_ROOT, m_bFirewallStartup);
		m_htiFilterLANIPs = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PW_FILTER), TVI_ROOT, m_bFilterLANIPs);
		m_htiExtControls = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SHOWEXTSETTINGS), TVI_ROOT, m_bExtControls);
        m_htiA4AFSaveCpu = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_A4AF_SAVE_CPU), TVI_ROOT, m_bA4AFSaveCpu); // ZZ:DownloadManager
		m_htiAutoArch  = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DISABLE_AUTOARCHPREV), TVI_ROOT, m_bAutoArchDisable);
		m_htiYourHostname = m_ctrlTreeOptions.InsertItem(GetResString(IDS_YOURHOSTNAME), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htiYourHostname, RUNTIME_CLASS(CTreeOptionsEditEx));

		/////////////////////////////////////////////////////////////////////////////
		// File related group
		//
		m_htiSparsePartFiles = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SPARSEPARTFILES), TVI_ROOT, m_bSparsePartFiles);
		m_htiFullAlloc = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_FULLALLOC), TVI_ROOT, m_bFullAlloc);
		m_htiCheckDiskspace = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_CHECKDISKSPACE), TVI_ROOT, m_bCheckDiskspace);
		m_htiMinFreeDiskSpace = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MINFREEDISKSPACE), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiCheckDiskspace);
		m_ctrlTreeOptions.AddEditBox(m_htiMinFreeDiskSpace, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiCommit = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_COMMITFILES), iImgBackup, TVI_ROOT);
		m_htiCommitNever = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_NEVER), m_htiCommit, m_iCommitFiles == 0);
		m_htiCommitOnShutdown = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_ONSHUTDOWN), m_htiCommit, m_iCommitFiles == 1);
		m_htiCommitAlways = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_ALWAYS), m_htiCommit, m_iCommitFiles == 2);
		m_htiExtractMetaData = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_EXTRACT_META_DATA), iImgMetaData, TVI_ROOT);
		m_htiExtractMetaDataNever = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_NEVER), m_htiExtractMetaData, m_iExtractMetaData == 0);
		m_htiExtractMetaDataID3Lib = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_META_DATA_ID3LIB), m_htiExtractMetaData, m_iExtractMetaData == 1);
		//m_htiExtractMetaDataMediaDet = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_META_DATA_MEDIADET), m_htiExtractMetaData, m_iExtractMetaData == 2);
		m_htiResolveShellLinks = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_RESOLVELINKS), TVI_ROOT, m_bResolveShellLinks);

		/////////////////////////////////////////////////////////////////////////////
		// Logging group
		//
		m_htiLog2Disk = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOG2DISK), TVI_ROOT, m_bLog2Disk);
		if (thePrefs.GetEnableVerboseOptions())
		{
			m_htiVerboseGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_VERBOSE), iImgLog, TVI_ROOT);
			m_htiVerbose = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ENABLED), m_htiVerboseGroup, m_bVerbose);
			m_htiLogLevel = m_ctrlTreeOptions.InsertItem(GetResString(IDS_LOG_LEVEL), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiVerboseGroup);
			m_ctrlTreeOptions.AddEditBox(m_htiLogLevel, RUNTIME_CLASS(CNumTreeOptionsEdit));
			m_htiDebug2Disk = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOG2DISK), m_htiVerboseGroup, m_bDebug2Disk);
			m_htiDebugSourceExchange = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DEBUG_SOURCE_EXCHANGE), m_htiVerboseGroup, m_bDebugSourceExchange);
			m_htiLogBannedClients = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOG_BANNED_CLIENTS), m_htiVerboseGroup, m_bLogBannedClients);
			m_htiLogRatingDescReceived = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOG_RATING_RECV), m_htiVerboseGroup, m_bLogRatingDescReceived);
			m_htiLogSecureIdent = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOG_SECURE_IDENT), m_htiVerboseGroup, m_bLogSecureIdent);
			m_htiLogFilteredIPs = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOG_FILTERED_IPS), m_htiVerboseGroup, m_bLogFilteredIPs);
			m_htiLogFileSaving = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOG_FILE_SAVING), m_htiVerboseGroup, m_bLogFileSaving);
			m_htiLogA4AF = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOG_A4AF), m_htiVerboseGroup, m_bLogA4AF); // ZZ:DownloadManager
			m_htiLogUlDlEvents = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOG_ULDL_EVENTS), m_htiVerboseGroup, m_bLogUlDlEvents);
			m_htiLogEmulator = m_ctrlTreeOptions.InsertCheckBox(_T(""), m_htiVerboseGroup, m_bLogEmulator); // ZZUL-TRA :: EmulateOthers
#ifdef CLIENTANALYZER
		    m_htiLogAnalyzerEvents = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOG_ANALYZER_EVENTS), m_htiVerboseGroup, m_bLogAnalyzerEvents);
#endif
		}

		/////////////////////////////////////////////////////////////////////////////
		// USS group
		//
        m_htiDynUp = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_DYNUP), iImgDynyp, TVI_ROOT);
		m_htiDynUpEnabled = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DYNUPENABLED), m_htiDynUp, m_bDynUpEnabled);
        m_htiDynUpMinUpload = m_ctrlTreeOptions.InsertItem(GetResString(IDS_DYNUP_MINUPLOAD), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDynUp);
		m_ctrlTreeOptions.AddEditBox(m_htiDynUpMinUpload, RUNTIME_CLASS(CNumTreeOptionsEdit));
        m_htiDynUpPingTolerance = m_ctrlTreeOptions.InsertItem(GetResString(IDS_DYNUP_PINGTOLERANCE), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDynUp);
		m_ctrlTreeOptions.AddEditBox(m_htiDynUpPingTolerance, RUNTIME_CLASS(CNumTreeOptionsEdit));
        m_htiDynUpPingToleranceMilliseconds = m_ctrlTreeOptions.InsertItem(GetResString(IDS_DYNUP_PINGTOLERANCE_MS), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDynUp);
        m_ctrlTreeOptions.AddEditBox(m_htiDynUpPingToleranceMilliseconds, RUNTIME_CLASS(CNumTreeOptionsEdit));
        m_htiDynUpPingToleranceGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_DYNUP_RADIO_PINGTOLERANCE_HEADER), iImgDynyp, m_htiDynUp);
        m_htiDynUpRadioPingTolerance = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_DYNUP_RADIO_PINGTOLERANCE_PERCENT), m_htiDynUpPingToleranceGroup, m_iDynUpRadioPingTolerance == 0);
        m_htiDynUpRadioPingToleranceMilliseconds = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_DYNUP_RADIO_PINGTOLERANCE_MS), m_htiDynUpPingToleranceGroup, m_iDynUpRadioPingTolerance == 1);
        m_htiDynUpGoingUpDivider = m_ctrlTreeOptions.InsertItem(GetResString(IDS_DYNUP_GOINGUPDIVIDER), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDynUp);
		m_ctrlTreeOptions.AddEditBox(m_htiDynUpGoingUpDivider, RUNTIME_CLASS(CNumTreeOptionsEdit));
        m_htiDynUpGoingDownDivider = m_ctrlTreeOptions.InsertItem(GetResString(IDS_DYNUP_GOINGDOWNDIVIDER), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDynUp);
		m_ctrlTreeOptions.AddEditBox(m_htiDynUpGoingDownDivider, RUNTIME_CLASS(CNumTreeOptionsEdit));
        m_htiDynUpNumberOfPings = m_ctrlTreeOptions.InsertItem(GetResString(IDS_DYNUP_NUMBEROFPINGS), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDynUp);
		m_ctrlTreeOptions.AddEditBox(m_htiDynUpNumberOfPings, RUNTIME_CLASS(CNumTreeOptionsEdit));

		/////////////////////////////////////////////////////////////////////////////
		// UPnP group
		//
        m_htiUPnP = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_UPNP), iImgUPnP, TVI_ROOT);
		m_htiCloseUPnPPorts = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_UPNPCLOSEONEXIT), m_htiUPnP, m_bCloseUPnPOnExit);
		m_htiSkipWANIPSetup = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_UPNPSKIPWANIP), m_htiUPnP, m_bSkipWANIPSetup);
		m_htiSkipWANPPPSetup = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_UPNPSKIPWANPPP), m_htiUPnP, m_bSkipWANPPPSetup);

		/////////////////////////////////////////////////////////////////////////////
		// eMule Shared User
		//
		m_htiShareeMule = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_SHAREEMULELABEL), iImgShareeMule, TVI_ROOT);
		m_htiShareeMuleMultiUser = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SHAREEMULEMULTI), m_htiShareeMule, m_iShareeMule == 0);
		m_htiShareeMulePublicUser = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SHAREEMULEPUBLIC), m_htiShareeMule, m_iShareeMule == 1);
		m_htiShareeMuleOldStyle = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SHAREEMULEOLD), m_htiShareeMule, m_iShareeMule == 2);

// ZZUL-TRA :: Advanced official preferences :: Start
		m_hti_advanced = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_ADVANCEDPREFS), iImgLog, TVI_ROOT);
		m_hti_bMiniMuleAutoClose=m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MINIMULEAUTOCLOSE),m_hti_advanced,bMiniMuleAutoClose);
		m_hti_iMiniMuleTransparency= m_ctrlTreeOptions.InsertItem(GetResString(IDS_MINIMULETRANSPARENCY),TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,m_hti_advanced);
		m_ctrlTreeOptions.AddEditBox(m_hti_iMiniMuleTransparency, RUNTIME_CLASS(CNumTreeOptionsEdit));

		m_hti_bCreateCrashDump=m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_CREATECRASHDUMP),m_hti_advanced,bCreateCrashDump);
		m_hti_bIgnoreInstances=m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_IGNOREINSTANCES),m_hti_advanced,bIgnoreInstances);

		m_hti_sMediaInfo_MediaInfoDllPath= m_ctrlTreeOptions.InsertItem(GetResString(IDS_MEDIAINFO_MEDIAINFODLLPATH), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_hti_advanced);
		m_ctrlTreeOptions.AddFileEditBox(m_hti_sMediaInfo_MediaInfoDllPath,RUNTIME_CLASS(CTreeOptionsEdit), RUNTIME_CLASS(CTreeOptionsBrowseButton));

		m_hti_iMaxLogBuff= m_ctrlTreeOptions.InsertItem(GetResString(IDS_MAXLOGBUFF),TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,m_hti_advanced);
		m_ctrlTreeOptions.AddEditBox(m_hti_iMaxLogBuff, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_hti_m_iMaxChatHistory= m_ctrlTreeOptions.InsertItem(GetResString(IDS_MAXCHATHISTORY),TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,m_hti_advanced);
		m_ctrlTreeOptions.AddEditBox(m_hti_m_iMaxChatHistory, RUNTIME_CLASS(CNumTreeOptionsEdit));

		m_hti_m_iPreviewSmallBlocks=m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PREVIEWSMALLBLOCKS),m_hti_advanced,m_iPreviewSmallBlocks);
		m_hti_m_bRestoreLastMainWndDlg=m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_RESTORELASTMAINWNDDLG),m_hti_advanced,m_bRestoreLastMainWndDlg);
		m_hti_m_bRestoreLastLogPane=m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_RESTORELASTLOGPANE),m_hti_advanced,m_bRestoreLastLogPane);
		m_hti_m_bPreviewCopiedArchives=m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PREVIEWCOPIEDARCHIVES),m_hti_advanced,m_bPreviewCopiedArchives);
		m_hti_m_iStraightWindowStyles=m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_STRAIGHTWINDOWSTYLES),m_hti_advanced,m_iStraightWindowStyles);
		m_hti_m_iLogFileFormat=m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOGFILEFORMAT),m_hti_advanced,m_iLogFileFormat);
		m_hti_m_bRTLWindowsLayout=m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_RTLWINDOWSLAYOUT),m_hti_advanced,m_bRTLWindowsLayout);
		m_hti_m_bPreviewOnIconDblClk=m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PREVIEWONICONDBLCLK),m_hti_advanced,m_bPreviewOnIconDblClk);

		m_hti_sInternetSecurityZone= m_ctrlTreeOptions.InsertItem(GetResString(IDS_INTERNETSECURITYZONE), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_hti_advanced);
		m_ctrlTreeOptions.AddEditBox(m_hti_sInternetSecurityZone, RUNTIME_CLASS(CTreeOptionsEditEx));

		m_hti_sTxtEditor= m_ctrlTreeOptions.InsertItem(GetResString(IDS_TXTEDITOR), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_hti_advanced);
		m_ctrlTreeOptions.AddFileEditBox(m_hti_sTxtEditor,RUNTIME_CLASS(CTreeOptionsEdit), RUNTIME_CLASS(CTreeOptionsBrowseButton));

		m_hti_iServerUDPPort= m_ctrlTreeOptions.InsertItem(GetResString(IDS_SERVERUDPPORT),TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,m_hti_advanced);
		m_ctrlTreeOptions.AddEditBox(m_hti_iServerUDPPort, RUNTIME_CLASS(CNumTreeOptionsEdit));

		m_hti_m_bRemoveFilesToBin=m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_REMOVEFILESTOBIN),m_hti_advanced,m_bRemoveFilesToBin);

		m_hti_HighresTimer=m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_HIGHRESTIMER),m_hti_advanced,m_bHighresTimer);
		m_hti_TrustEveryHash=m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_TRUSTEVERYHASH),m_hti_advanced,m_bTrustEveryHash);
		m_htiICH = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_ICH), m_hti_advanced, m_ICH);


		m_hti_InspectAllFileTypes=m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_INSPECTALLFILETYPES),m_hti_advanced,m_iInspectAllFileTypes);
		m_hti_maxmsgsessions=m_ctrlTreeOptions.InsertItem(GetResString(IDS_MAXMSGSESSIONS),TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,m_hti_advanced);
		m_ctrlTreeOptions.AddEditBox(m_hti_maxmsgsessions, RUNTIME_CLASS(CNumTreeOptionsEdit));													   

		m_hti_PreferRestrictedOverUser=m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PREFERRESTRICTEDOVERUSER),m_hti_advanced,m_bPreferRestrictedOverUser);
		m_hti_UseUserSortedServerList=m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_USEUSERSORTEDSERVERLIST),m_hti_advanced,m_bUseUserSortedServerList);
		m_hti_WebFileUploadSizeLimitMB=m_ctrlTreeOptions.InsertItem(GetResString(IDS_WEBFILEUPLOADSIZELIMITMB),TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,m_hti_advanced);
		m_ctrlTreeOptions.AddEditBox(m_hti_WebFileUploadSizeLimitMB, RUNTIME_CLASS(CNumTreeOptionsEdit));													   										   
		m_hti_AllowedIPs=m_ctrlTreeOptions.InsertItem(GetResString(IDS_ALLOWEDIPS),TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,m_hti_advanced);
		m_ctrlTreeOptions.AddEditBox(m_hti_AllowedIPs, RUNTIME_CLASS(CTreeOptionsEditEx));
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
		m_hti_DebugSearchResultDetailLevel=m_ctrlTreeOptions.InsertItem(GetResString(IDS_DEBUGSEARCHDETAILLEVEL),TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,m_hti_advanced);
		m_ctrlTreeOptions.AddEditBox(m_hti_DebugSearchResultDetailLevel, RUNTIME_CLASS(CTreeOptionsEditEx));
#endif
		m_htiCryptTCPPaddingLength=m_ctrlTreeOptions.InsertItem(GetResString(IDS_CRYPTTCPPADDINGLENGTH),TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,m_hti_advanced);
		m_ctrlTreeOptions.AddEditBox(m_htiCryptTCPPaddingLength, RUNTIME_CLASS(CNumTreeOptionsEdit));													   

		m_htiAdjustNTFSDaylightFileTime = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_ADJUSTNTFSDAYLIGHTFILETIME), m_hti_advanced, m_bAdjustNTFSDaylightFileTime);

		m_htidatetimeformat= m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_DATETIMEFORMAT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_hti_advanced);
		m_ctrlTreeOptions.AddEditBox(m_htidatetimeformat, RUNTIME_CLASS(CTreeOptionsEditEx));

		m_htidatetimeformat4log = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_DATETIMEFORMAT4LOG), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_hti_advanced);
		m_ctrlTreeOptions.AddEditBox(m_htidatetimeformat4log, RUNTIME_CLASS(CTreeOptionsEditEx));

		m_htidatetimeformat4list = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_DATETIMEFORMAT4LIST), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_hti_advanced);
		m_ctrlTreeOptions.AddEditBox(m_htidatetimeformat4list, RUNTIME_CLASS(CTreeOptionsEditEx));

		m_htiLogError = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_LOGERROR), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_hti_advanced);
		m_ctrlTreeOptions.AddColorSelector(m_htiLogError, RUNTIME_CLASS(CTreeOptionsBrowseButton));
		m_htiLogWarning = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_LOGWARNING), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_hti_advanced);
		m_ctrlTreeOptions.AddColorSelector(m_htiLogWarning, RUNTIME_CLASS(CTreeOptionsBrowseButton));
		m_htiLogSuccess = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_LOGSUCCESS), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_hti_advanced);
		m_ctrlTreeOptions.AddColorSelector(m_htiLogSuccess, RUNTIME_CLASS(CTreeOptionsBrowseButton));
		m_htiShowVerticalHourMarkers = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_SHOWVERTICALHOURMARKERS), m_hti_advanced, m_bShowVerticalHourMarkers);
   	    m_htiReBarToolbar = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_REBARTOOLBAR), m_hti_advanced, m_bReBarToolbar);		
    	m_htiIconflashOnNewMessage = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_ICON_FLASH_ON_NEW_MESSAGE), m_hti_advanced, m_bIconflashOnNewMessage);
		m_htiShowCopyEd2kLinkCmd = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_SHOWCOPYED2KLINK), m_hti_advanced, m_bShowCopyEd2kLinkCmd);
		m_htidontcompressavi = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_DONTCOMPRESSAVI), m_hti_advanced, m_dontcompressavi);
		m_htiFileBufferTimeLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_FILE_BUFFER_TIME_LIMIT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_hti_advanced);
		m_ctrlTreeOptions.AddEditBox(m_htiFileBufferTimeLimit, RUNTIME_CLASS(CTreeOptionsEditEx));
		m_htiRearrangeKadSearchKeywords = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_REARRANGEKADSEARCH), m_hti_advanced, m_bRearrangeKadSearchKeywords);
		m_htiUpdateQueue = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_UPDATEQUEUE), m_hti_advanced, m_bUpdateQueue);
		m_htiRepaint = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_REPAINTGRAPHS), m_hti_advanced, m_bRepaint);
		m_htiBeeper = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PW_BEEP), m_hti_advanced, m_bBeeper);
		m_htiMsgOnlySec = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MSGONLYSEC), m_hti_advanced, m_bMsgOnlySec);
		m_htiExtraPreviewWithMenu = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_EXTRAPREVIEWWITHMENU), m_hti_advanced, m_bExtraPreviewWithMenu);
		m_htiShowUpDownIconInTaskbar = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SHOWUPDOWNICONINTASKBAR), m_hti_advanced, m_bShowUpDownIconInTaskbar);
		m_htiKeepUnavailableFixedSharedDirs = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_KEEPUNAVAILABLEFIXEDSHAREDDIRS), m_hti_advanced, m_bKeepUnavailableFixedSharedDirs);
		m_htiForceSpeedsToKB = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_FORCESPEEDSTOKB), m_hti_advanced, m_bForceSpeedsToKB);
// ZZUL-TRA :: Advanced official preferences :: End
	
#ifdef CLIENTANALYZER
//>>> WiZaRd::ClientAnalyzer
		m_ctrlTreeOptions.Expand(m_htiCreditSystem, TVE_EXPAND); 

		switch (thePrefs.UseCreditSystem())
		{
		case 0:
		   m_ctrlTreeOptions.SetItemState(m_htiCSNone, TVIS_BOLD, TVIS_BOLD);
		   break;
		case 1:
		   m_ctrlTreeOptions.SetItemState(m_htiCSOffi, TVIS_BOLD, TVIS_BOLD);
		   break;
        case 2:
            m_ctrlTreeOptions.SetItemState(m_htiCSAnalyzer, TVIS_BOLD, TVIS_BOLD);
		    break;
		}
//<<< WiZaRd::ClientAnalyzer
#endif

		//m_ctrlTreeOptions.Expand(m_htiZZULTRA, TVE_EXPAND); //ZZUL-TRA
		m_ctrlTreeOptions.Expand(m_htiInvisibleMode, TVE_EXPAND);  // ZZUL-TRA :: InvisibleMode 
	    m_ctrlTreeOptions.Expand(m_htiTCPGroup, TVE_EXPAND);
        if (m_htiVerboseGroup)
		    m_ctrlTreeOptions.Expand(m_htiVerboseGroup, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiCommit, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiCheckDiskspace, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiDynUp, TVE_EXPAND);
        m_ctrlTreeOptions.Expand(m_htiDynUpPingToleranceGroup, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiExtractMetaData, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiUPnP, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiShareeMule, TVE_EXPAND);
        m_ctrlTreeOptions.SendMessage(WM_VSCROLL, SB_TOP);
        m_bInitializedTreeOpts = true;
	}

	/////////////////////////////////////////////////////////////////////////////
	// ZZUL-TRA group
	//

	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiShowActiveDownloadsBold, m_bShowActiveDownloadsBold);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiShowRuntimeOnTitle, m_bShowRuntimeOnTitle);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiShowDownloadColor, m_bShowDownloadColor);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiShowFileStatusIcons, m_bShowFileStatusIcons);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiDirectoryWatcher, m_bDirectoryWatcher);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiAutoDropSystem, m_bAutoDropSystem);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiFunnyNick, m_bFunnyNick);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiShowSessionDownload, m_bShowSessionDownload);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiUsePercentBar, m_bUsePercentBar); // ZZUL-TRA :: PercentBar
	//ZZUL-TRA :: PowerShare :: Start
	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiPowerShareLimit, m_iPowerShareLimit);
	DDV_MinMaxInt(pDX, m_iPowerShareLimit, 0, INT_MAX);
	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiPsAmountLimit, m_iPsAmountLimit);
	DDV_MinMaxInt(pDX, m_iPsAmountLimit, 0, MAX_PS_AMOUNT_LIMIT);
	DDX_TreeRadio(pDX, IDC_EXT_OPTS, m_htiPowershareMode, m_iPowershareMode);
	//ZZUL-TRA :: PowerShare :: End
	// ZZUL-TRA :: InvisibleMode :: Start
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiInvisibleMode, m_bInvisibleMode);
	DDX_TreeCombo(pDX, IDC_EXT_OPTS, m_htiInvisibleModeMod, m_sInvisibleModeMod);
	DDX_TreeCombo(pDX, IDC_EXT_OPTS, m_htiInvisibleModeKey, m_sInvisibleModeKey);
	// ZZUL-TRA :: InvisibleMode :: End
	//>>> shadow2004::IP2Country [EastShare]
	DDX_TreeRadio(pDX, IDC_EXT_OPTS, m_htiI2CGroup, m_iI2CCaption);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiI2CShowFlag, m_bI2CShowFlag);
	//DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiIP2CUpdate, m_bIP2CUpdate);
	//<<< shadow2004::IP2Country [EastShare]
    DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiIsPayBackFirst, m_bIsPayBackFirst); //PayBack First
	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiPayBackFirstLimit, m_iPayBackFirstLimit); //PayBack First
	// ZZUL-TRA :: Global Hardlimit :: Start
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiIsGlobalHardlimit, m_bIsGlobalHardlimit); 
	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiGlobalHardlimit, m_iGlobalHardlimit); 
	DDV_MinMaxInt(pDX, m_iGlobalHardlimit, VAL_GSL_MIN, VAL_GSL_MAX);
	// ZZUL-TRA :: Global Hardlimit :: End
	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiDisconnectTime, m_iDisconnectTime);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiGetCPU, m_bGetCPU); // ZZUL-TRA :: SysInfo
    // NEO: QS - [QuickStart]
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiQuickStartEnable, m_uQuickStart);
	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiQuickStartTime, m_iQuickStartTime);
	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiQuickStartTimePerFile, m_iQuickStartTimePerFile);
	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiQuickMaxConperFive, m_iQuickMaxConperFive);
	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiQuickMaxHalfOpen, m_iQuickMaxHalfOpen);
	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiQuickMaxConnections, m_iQuickMaxConnections);
	// NEO: QS END

	/////////////////////////////////////////////////////////////////////////////
	// TCP/IP group
	//
	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiMaxCon5Sec, m_iMaxConnPerFive);
	DDV_MinMaxInt(pDX, m_iMaxConnPerFive, 1, INT_MAX);
	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiMaxHalfOpen, m_iMaxHalfOpen);
	DDV_MinMaxInt(pDX, m_iMaxHalfOpen, 1, INT_MAX);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiConditionalTCPAccept, m_bConditionalTCPAccept);
	DDX_Text(pDX, IDC_EXT_OPTS, m_htiServerKeepAliveTimeout, m_uServerKeepAliveTimeout);

	/////////////////////////////////////////////////////////////////////////////
	// Miscellaneous group
	//
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiAutoTakeEd2kLinks, m_bAutoTakeEd2kLinks);
#ifdef CLIENTANALYZER
	DDX_TreeRadio(pDX, IDC_EXT_OPTS, m_htiCreditSystem, m_iCreditSystem); //>>> WiZaRd::ClientAnalyzer
#else
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiCreditSystem, m_bCreditSystem);
#endif
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiFirewallStartup, m_bFirewallStartup);
	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiFirewallStartup, thePrefs.GetWindowsVersion() == _WINVER_XP_ && IsRunningXPSP2() == 0);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiFilterLANIPs, m_bFilterLANIPs);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiExtControls, m_bExtControls);
    DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiA4AFSaveCpu, m_bA4AFSaveCpu);
	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiYourHostname, m_sYourHostname);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiAutoArch, m_bAutoArchDisable);
	
	/////////////////////////////////////////////////////////////////////////////
	// File related group
	//
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiSparsePartFiles, m_bSparsePartFiles);
	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiSparsePartFiles, thePrefs.GetWindowsVersion() != _WINVER_VISTA_ /*only disable on Vista, not later versions*/);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiFullAlloc, m_bFullAlloc);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiCheckDiskspace, m_bCheckDiskspace);
	DDX_Text(pDX, IDC_EXT_OPTS, m_htiMinFreeDiskSpace, m_fMinFreeDiskSpaceMB);
	DDV_MinMaxFloat(pDX, m_fMinFreeDiskSpaceMB, 0.0, UINT_MAX / (1024*1024));
	DDX_TreeRadio(pDX, IDC_EXT_OPTS, m_htiCommit, m_iCommitFiles);
	DDX_TreeRadio(pDX, IDC_EXT_OPTS, m_htiExtractMetaData, m_iExtractMetaData);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiResolveShellLinks, m_bResolveShellLinks);

	/////////////////////////////////////////////////////////////////////////////
	// Logging group
	//
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiLog2Disk, m_bLog2Disk);
	if (m_htiLogLevel){
		DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiLogLevel, m_iLogLevel);
		DDV_MinMaxInt(pDX, m_iLogLevel, 1, 5);
	}	
	if (m_htiVerbose)				DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiVerbose, m_bVerbose);
	if (m_htiDebug2Disk)			DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiDebug2Disk, m_bDebug2Disk);
	if (m_htiDebug2Disk)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiDebug2Disk, m_bVerbose);
	if (m_htiDebugSourceExchange)	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiDebugSourceExchange, m_bDebugSourceExchange);
	if (m_htiDebugSourceExchange)	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiDebugSourceExchange, m_bVerbose);
	if (m_htiLogBannedClients)		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiLogBannedClients, m_bLogBannedClients);
	if (m_htiLogBannedClients)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogBannedClients, m_bVerbose);
	if (m_htiLogRatingDescReceived) DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiLogRatingDescReceived, m_bLogRatingDescReceived);
	if (m_htiLogRatingDescReceived) m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogRatingDescReceived, m_bVerbose);
	if (m_htiLogSecureIdent)		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiLogSecureIdent, m_bLogSecureIdent);
	if (m_htiLogSecureIdent)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogSecureIdent, m_bVerbose);
	if (m_htiLogFilteredIPs)		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiLogFilteredIPs, m_bLogFilteredIPs);
	if (m_htiLogFilteredIPs)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogFilteredIPs, m_bVerbose);
	if (m_htiLogFileSaving)			DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiLogFileSaving, m_bLogFileSaving);
	if (m_htiLogFileSaving)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogFileSaving, m_bVerbose);
    if (m_htiLogA4AF)			    DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiLogA4AF, m_bLogA4AF);
	if (m_htiLogA4AF)               m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogA4AF, m_bVerbose);
	if (m_htiLogUlDlEvents)			DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiLogUlDlEvents, m_bLogUlDlEvents);
	if (m_htiLogUlDlEvents)         m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogUlDlEvents, m_bVerbose);
// ZZUL-TRA :: EmulateOthers :: Start
    if (m_htiLogEmulator){
		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiLogEmulator, m_bLogEmulator); 
		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogEmulator, m_bVerbose); 
	}
// ZZUL-TRA :: EmulateOthers :: End
#ifdef CLIENTANALYZER
	if (m_htiLogAnalyzerEvents){
        DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiLogAnalyzerEvents, m_bLogAnalyzerEvents);
		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogAnalyzerEvents, m_bVerbose); 
        }
#endif

	/////////////////////////////////////////////////////////////////////////////
	// USS group
	//
    DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiDynUpEnabled, m_bDynUpEnabled);
    DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiDynUpMinUpload, m_iDynUpMinUpload);
	DDV_MinMaxInt(pDX, m_iDynUpMinUpload, 1, INT_MAX);
    DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiDynUpPingTolerance, m_iDynUpPingTolerance);
	DDV_MinMaxInt(pDX, m_iDynUpPingTolerance, 100, INT_MAX);
    DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiDynUpPingToleranceMilliseconds, m_iDynUpPingToleranceMilliseconds);
	DDV_MinMaxInt(pDX, m_iDynUpPingTolerance, 1, INT_MAX);
    DDX_TreeRadio(pDX, IDC_EXT_OPTS, m_htiDynUpPingToleranceGroup, m_iDynUpRadioPingTolerance);
    DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiDynUpGoingUpDivider, m_iDynUpGoingUpDivider);
	DDV_MinMaxInt(pDX, m_iDynUpGoingUpDivider, 1, INT_MAX);
    DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiDynUpGoingDownDivider, m_iDynUpGoingDownDivider);
	DDV_MinMaxInt(pDX, m_iDynUpGoingDownDivider, 1, INT_MAX);
    DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiDynUpNumberOfPings, m_iDynUpNumberOfPings);
	DDV_MinMaxInt(pDX, m_iDynUpNumberOfPings, 1, INT_MAX);
   
	/////////////////////////////////////////////////////////////////////////////
	// UPnP group
	//
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiCloseUPnPPorts, m_bCloseUPnPOnExit);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiSkipWANIPSetup, m_bSkipWANIPSetup);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiSkipWANPPPSetup, m_bSkipWANPPPSetup);

// ZZUL-TRA :: Advanced official preferences :: Start
	if (m_hti_bMiniMuleAutoClose) DDX_TreeCheck(pDX, IDC_EXT_OPTS,m_hti_bMiniMuleAutoClose,bMiniMuleAutoClose);
	if (m_hti_iMiniMuleTransparency) {DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_hti_iMiniMuleTransparency, iMiniMuleTransparency);
								   DDV_MinMaxInt(pDX, iMiniMuleTransparency, 0, 100);}
	if(m_hti_bCreateCrashDump) DDX_TreeCheck(pDX,IDC_EXT_OPTS,m_hti_bCreateCrashDump,bCreateCrashDump);
	if(m_hti_bIgnoreInstances) DDX_TreeCheck(pDX,IDC_EXT_OPTS,m_hti_bIgnoreInstances,bIgnoreInstances);

	if (m_hti_iMaxLogBuff) {DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_hti_iMaxLogBuff, iMaxLogBuff);
										 DDV_MinMaxInt(pDX, iMaxLogBuff, 64, 512);}
	if (m_hti_m_iMaxChatHistory) {DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_hti_m_iMaxChatHistory, m_iMaxChatHistory);
										 DDV_MinMaxInt(pDX, m_iMaxChatHistory, 3, 2048);}
	if(m_hti_m_iPreviewSmallBlocks) DDX_TreeCheck(pDX,IDC_EXT_OPTS,m_hti_m_iPreviewSmallBlocks,m_iPreviewSmallBlocks);
	if(m_hti_m_bRestoreLastMainWndDlg) DDX_TreeCheck(pDX,IDC_EXT_OPTS,m_hti_m_bRestoreLastMainWndDlg,m_bRestoreLastMainWndDlg);
	if(m_hti_m_bRestoreLastLogPane) DDX_TreeCheck(pDX,IDC_EXT_OPTS,m_hti_m_bRestoreLastLogPane,m_bRestoreLastLogPane);
	if(m_hti_m_bPreviewCopiedArchives) DDX_TreeCheck(pDX,IDC_EXT_OPTS,m_hti_m_bPreviewCopiedArchives,m_bPreviewCopiedArchives);
	if(m_hti_m_iStraightWindowStyles) DDX_TreeCheck(pDX,IDC_EXT_OPTS,m_hti_m_iStraightWindowStyles,m_iStraightWindowStyles);
	if(m_hti_m_iLogFileFormat) DDX_TreeCheck(pDX,IDC_EXT_OPTS,m_hti_m_iLogFileFormat,m_iLogFileFormat);
	if(m_hti_m_bRTLWindowsLayout) DDX_TreeCheck(pDX,IDC_EXT_OPTS,m_hti_m_bRTLWindowsLayout,m_bRTLWindowsLayout);
	if(m_hti_m_bPreviewOnIconDblClk) DDX_TreeCheck(pDX,IDC_EXT_OPTS,m_hti_m_bPreviewOnIconDblClk,m_bPreviewOnIconDblClk);

	if (m_hti_sInternetSecurityZone) {DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_hti_sInternetSecurityZone, sInternetSecurityZone);}
									  //TODO only allow  Untrusted|Internet|Intranet|Trusted|LocalMachine 
	if (m_hti_sTxtEditor)	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_hti_sTxtEditor, sTxtEditor);
	if (m_hti_iServerUDPPort) {DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_hti_iServerUDPPort, iServerUDPPort);
										 DDV_MinMaxInt(pDX, iServerUDPPort, 0,65535);}
	if(m_hti_m_bRemoveFilesToBin) DDX_TreeCheck(pDX,IDC_EXT_OPTS,m_hti_m_bRemoveFilesToBin,m_bRemoveFilesToBin);
   
	if(m_hti_HighresTimer) DDX_TreeCheck(pDX,IDC_EXT_OPTS,m_hti_HighresTimer,m_bHighresTimer);
	if(m_hti_TrustEveryHash) DDX_TreeCheck(pDX,IDC_EXT_OPTS,m_hti_TrustEveryHash,m_bTrustEveryHash);
    if(m_hti_InspectAllFileTypes) DDX_TreeCheck(pDX,IDC_EXT_OPTS,m_hti_InspectAllFileTypes,m_iInspectAllFileTypes);
	if(m_hti_maxmsgsessions) {DDX_TreeEdit(pDX,IDC_EXT_OPTS,m_hti_maxmsgsessions,(int)m_umaxmsgsessions);
						DDV_MinMaxInt(pDX, m_umaxmsgsessions, 0, 6000);}
    if(m_hti_PreferRestrictedOverUser) DDX_TreeCheck(pDX,IDC_EXT_OPTS,m_hti_PreferRestrictedOverUser,m_bPreferRestrictedOverUser);
	if(m_hti_UseUserSortedServerList) DDX_TreeCheck(pDX,IDC_EXT_OPTS,m_hti_UseUserSortedServerList,m_bUseUserSortedServerList);
	if(m_hti_WebFileUploadSizeLimitMB) { DDX_TreeEdit(pDX,IDC_EXT_OPTS,m_hti_WebFileUploadSizeLimitMB,m_iWebFileUploadSizeLimitMB);
										DDV_MinMaxInt(pDX, m_iWebFileUploadSizeLimitMB, 0, INT_MAX);}
    if(m_hti_AllowedIPs) DDX_TreeEdit(pDX,IDC_EXT_OPTS,m_hti_AllowedIPs,m_sAllowedIPs); //TODO: check string for ip
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	if(m_hti_DebugSearchResultDetailLevel) DDX_TreeEdit(pDX,IDC_EXT_OPTS,m_hti_DebugSearchResultDetailLevel,m_iDebugSearchResultDetailLevel); //TODO: check string for ip
#endif
	if (m_htiCryptTCPPaddingLength) { DDX_TreeEdit(pDX,IDC_EXT_OPTS,m_htiCryptTCPPaddingLength,m_iCryptTCPPaddingLength );
									  DDV_MinMaxInt(pDX, m_iCryptTCPPaddingLength , 1,256);}
	if (m_htiFileBufferTimeLimit) { DDX_TreeEdit(pDX,IDC_EXT_OPTS,m_htiFileBufferTimeLimit,m_iFileBufferTimeLimit );
									  DDV_MinMaxInt(pDX, m_iFileBufferTimeLimit , 1,6000);} // max 10 minutes

	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiAdjustNTFSDaylightFileTime, m_bAdjustNTFSDaylightFileTime);
	DDX_Text(pDX, IDC_EXT_OPTS, m_htidatetimeformat, m_strDateTimeFormat);
	DDX_Text(pDX, IDC_EXT_OPTS, m_htidatetimeformat4log, m_strDateTimeFormat4Log);
	DDX_Text(pDX, IDC_EXT_OPTS, m_htidatetimeformat4list, m_strDateTimeFormat4List);
	DDX_TreeColor(pDX, IDC_EXT_OPTS, m_htiLogError, m_crLogError);
	DDX_TreeColor(pDX, IDC_EXT_OPTS, m_htiLogWarning, m_crLogWarning);
	DDX_TreeColor(pDX, IDC_EXT_OPTS, m_htiLogSuccess, m_crLogSuccess);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiShowVerticalHourMarkers, m_bShowVerticalHourMarkers);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiICH, m_ICH);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htidontcompressavi, m_dontcompressavi);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiShowCopyEd2kLinkCmd, m_bShowCopyEd2kLinkCmd);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiIconflashOnNewMessage, m_bIconflashOnNewMessage);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiReBarToolbar, m_bReBarToolbar);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiRearrangeKadSearchKeywords, m_bRearrangeKadSearchKeywords);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiUpdateQueue, m_bUpdateQueue);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiRepaint, m_bRepaint);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiBeeper, m_bBeeper);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiMsgOnlySec, m_bMsgOnlySec);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiExtraPreviewWithMenu, m_bExtraPreviewWithMenu);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiShowUpDownIconInTaskbar, m_bShowUpDownIconInTaskbar);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiKeepUnavailableFixedSharedDirs, m_bKeepUnavailableFixedSharedDirs);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiForceSpeedsToKB, m_bForceSpeedsToKB);
// ZZUL-TRA :: Advanced official preferences :: End

	/////////////////////////////////////////////////////////////////////////////
	// eMule Shared User
	//
	DDX_TreeRadio(pDX, IDC_EXT_OPTS, m_htiShareeMule, m_iShareeMule);
	m_ctrlTreeOptions.SetRadioButtonEnable(m_htiShareeMulePublicUser, thePrefs.GetWindowsVersion() >= _WINVER_VISTA_);
}

BOOL CPPgTweaks::OnInitDialog()
{
	m_iMaxConnPerFive = thePrefs.GetMaxConperFive();
	m_iMaxHalfOpen = thePrefs.GetMaxHalfConnections();
	m_bConditionalTCPAccept = thePrefs.GetConditionalTCPAccept();
	m_bAutoTakeEd2kLinks = thePrefs.AutoTakeED2KLinks();
	if (thePrefs.GetEnableVerboseOptions())
	{
		m_bVerbose = thePrefs.m_bVerbose;
		m_bDebug2Disk = thePrefs.debug2disk;							// do *not* use the according 'Get...' function here!
		m_bDebugSourceExchange = thePrefs.m_bDebugSourceExchange;		// do *not* use the according 'Get...' function here!
		m_bLogBannedClients = thePrefs.m_bLogBannedClients;				// do *not* use the according 'Get...' function here!
		m_bLogRatingDescReceived = thePrefs.m_bLogRatingDescReceived;	// do *not* use the according 'Get...' function here!
		m_bLogSecureIdent = thePrefs.m_bLogSecureIdent;					// do *not* use the according 'Get...' function here!
		m_bLogFilteredIPs = thePrefs.m_bLogFilteredIPs;					// do *not* use the according 'Get...' function here!
		m_bLogFileSaving = thePrefs.m_bLogFileSaving;					// do *not* use the according 'Get...' function here!
        m_bLogA4AF = thePrefs.m_bLogA4AF;                   		    // do *not* use the according 'Get...' function here! // ZZ:DownloadManager
        m_bLogEmulator = thePrefs.m_bEmuLog; // ZZUL-TRA :: EmulateOthers
#ifdef  CLIENTANALYZER
	    m_bLogAnalyzerEvents = thePrefs.m_bLogAnalyzerEvents;
#endif
		m_bLogUlDlEvents = thePrefs.m_bLogUlDlEvents;
		m_iLogLevel = 5 - thePrefs.m_byLogLevel;
	}
	m_bLog2Disk = thePrefs.log2disk;
#ifdef CLIENTANALYZER
	m_iCreditSystem = thePrefs.m_uiCreditSystem; //>>> WiZaRd::ClientAnalyzer
#else
	m_bCreditSystem = thePrefs.m_bCreditSystem;
#endif
	m_iCommitFiles = thePrefs.m_iCommitFiles;
	m_iExtractMetaData = thePrefs.m_iExtractMetaData;
	m_bFilterLANIPs = thePrefs.filterLANIPs;
	m_bExtControls = thePrefs.m_bExtControls;
	m_uServerKeepAliveTimeout = thePrefs.m_dwServerKeepAliveTimeout / 60000;
	m_bSparsePartFiles = thePrefs.GetSparsePartFiles();
	m_bFullAlloc= thePrefs.m_bAllocFull;
	m_bCheckDiskspace = thePrefs.checkDiskspace;
	m_bResolveShellLinks = thePrefs.GetResolveSharedShellLinks();
	m_fMinFreeDiskSpaceMB = (float)(thePrefs.m_uMinFreeDiskSpace / (1024.0 * 1024.0));
	m_sYourHostname = thePrefs.GetYourHostname();
	m_bFirewallStartup = ((thePrefs.GetWindowsVersion() == _WINVER_XP_) ? thePrefs.m_bOpenPortsOnStartUp : 0); 
	m_bAutoArchDisable = !thePrefs.m_bAutomaticArcPreviewStart;

    m_bDynUpEnabled = thePrefs.m_bDynUpEnabled;
    m_iDynUpMinUpload = thePrefs.GetMinUpload();
    m_iDynUpPingTolerance = thePrefs.GetDynUpPingTolerance();
    m_iDynUpPingToleranceMilliseconds = thePrefs.GetDynUpPingToleranceMilliseconds();
    m_iDynUpRadioPingTolerance = thePrefs.IsDynUpUseMillisecondPingTolerance()?1:0;
    m_iDynUpGoingUpDivider = thePrefs.GetDynUpGoingUpDivider();
    m_iDynUpGoingDownDivider = thePrefs.GetDynUpGoingDownDivider();
    m_iDynUpNumberOfPings = thePrefs.GetDynUpNumberOfPings();

	m_bCloseUPnPOnExit = thePrefs.CloseUPnPOnExit();
	m_bSkipWANIPSetup = thePrefs.GetSkipWANIPSetup();
	m_bSkipWANPPPSetup = thePrefs.GetSkipWANPPPSetup();

	m_iShareeMule = thePrefs.m_nCurrentUserDirMode;

    m_bA4AFSaveCpu = thePrefs.GetA4AFSaveCpu();

	m_ctrlTreeOptions.SetImageListColorFlags(theApp.m_iDfltImageListColorFlags);
 
// ZZUL-TRA :: Advanced official preferences :: Start
	bMiniMuleAutoClose=thePrefs.bMiniMuleAutoClose;
	iMiniMuleTransparency=thePrefs.iMiniMuleTransparency;
	bCreateCrashDump=thePrefs.bCreateCrashDump;
	bIgnoreInstances=thePrefs.bIgnoreInstances;
	sMediaInfo_MediaInfoDllPath=thePrefs.sMediaInfo_MediaInfoDllPath;
	iMaxLogBuff=thePrefs.GetMaxLogBuff()/1024;
	m_iMaxChatHistory=thePrefs.m_iMaxChatHistory;
	m_iPreviewSmallBlocks=thePrefs.m_iPreviewSmallBlocks;
	m_bRestoreLastMainWndDlg=thePrefs.m_bRestoreLastMainWndDlg;
	m_bRestoreLastLogPane=thePrefs.m_bRestoreLastLogPane;
	m_bPreviewCopiedArchives=thePrefs.m_bPreviewCopiedArchives;
	m_iStraightWindowStyles=thePrefs.m_iStraightWindowStyles;
	m_iLogFileFormat=thePrefs.m_iLogFileFormat;
	m_bRTLWindowsLayout=thePrefs.m_bRTLWindowsLayout;
	m_bPreviewOnIconDblClk=thePrefs.m_bPreviewOnIconDblClk;
	sInternetSecurityZone=thePrefs.sInternetSecurityZone;
	sTxtEditor=thePrefs.GetTxtEditor();
	iServerUDPPort=thePrefs.GetServerUDPPort();
	m_bRemoveFilesToBin=thePrefs.GetRemoveToBin();
	m_bHighresTimer=thePrefs.m_bHighresTimer;
	m_bTrustEveryHash=thePrefs.m_bTrustEveryHash;
	m_iInspectAllFileTypes=thePrefs.m_iInspectAllFileTypes;
	m_umaxmsgsessions=thePrefs.maxmsgsessions;
	m_bPreferRestrictedOverUser=thePrefs.m_bPreferRestrictedOverUser;
	m_bUseUserSortedServerList=thePrefs.m_bUseUserSortedServerList;
	m_iWebFileUploadSizeLimitMB=thePrefs.m_iWebFileUploadSizeLimitMB;
	m_sAllowedIPs=_T("");
	if (thePrefs.GetAllowedRemoteAccessIPs().GetCount() > 0)
		for (int i = 0; i <  thePrefs.GetAllowedRemoteAccessIPs().GetCount(); i++)
           m_sAllowedIPs= m_sAllowedIPs+ _T(";") + ipstr(thePrefs.GetAllowedRemoteAccessIPs()[i]);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	m_iDebugSearchResultDetailLevel=thePrefs.GetDebugSearchResultDetailLevel();
#endif
	m_iCryptTCPPaddingLength  = thePrefs.GetCryptTCPPaddingLength();
	m_bAdjustNTFSDaylightFileTime = thePrefs.m_bAdjustNTFSDaylightFileTime;
	m_strDateTimeFormat = thePrefs.m_strDateTimeFormat;
	m_strDateTimeFormat4Log = thePrefs.m_strDateTimeFormat4Log;
	m_strDateTimeFormat4List = thePrefs.m_strDateTimeFormat4Lists;
	m_crLogError = thePrefs.m_crLogError;
	m_crLogWarning = thePrefs.m_crLogWarning;
	m_crLogSuccess = thePrefs.m_crLogSuccess;
	m_ICH = thePrefs.ICH;
	m_dontcompressavi = thePrefs.dontcompressavi;
	m_bShowCopyEd2kLinkCmd = thePrefs.m_bShowCopyEd2kLinkCmd;
	m_bIconflashOnNewMessage = thePrefs.m_bIconflashOnNewMessage;
	m_bShowVerticalHourMarkers = thePrefs.m_bShowVerticalHourMarkers;
	m_bReBarToolbar = !thePrefs.m_bReBarToolbar;
	m_iFileBufferTimeLimit = thePrefs.GetFileBufferTimeLimit()/1000;
	m_bRearrangeKadSearchKeywords = thePrefs.GetRearrangeKadSearchKeywords();
	m_bUpdateQueue = thePrefs.m_bupdatequeuelist;
	m_bRepaint = thePrefs.IsGraphRecreateDisabled();
	m_bBeeper = thePrefs.beepOnError;
	m_bMsgOnlySec = thePrefs.msgsecure;
	m_bExtraPreviewWithMenu = thePrefs.GetExtraPreviewWithMenu();
	m_bShowUpDownIconInTaskbar = thePrefs.IsShowUpDownIconInTaskbar();
	m_bKeepUnavailableFixedSharedDirs = thePrefs.m_bKeepUnavailableFixedSharedDirs;
	m_bForceSpeedsToKB = thePrefs.GetForceSpeedsToKB();
// ZZUL-TRA :: Advanced official preferences :: End

    m_bIsPayBackFirst = thePrefs.m_bPayBackFirst; //PayBack First
	m_iPayBackFirstLimit = thePrefs.m_iPayBackFirstLimit; //PayBack First 

  // ZZUL-TRA :: Global Hardlimit :: Start
	m_bIsGlobalHardlimit = thePrefs.m_bGlobalHL; 
	m_iGlobalHardlimit = thePrefs.m_uGlobalHL; 
	// ZZUL-TRA :: Global Hardlimit :: End

	   m_bGetCPU = thePrefs.m_bShowCpu; // ZZUL-TRA :: SysInfo

// NEO: QS - [QuickStart]
	m_uQuickStart = thePrefs.m_uQuickStart;
	m_iQuickStartTime = thePrefs.m_iQuickStartTime;
	m_iQuickStartTimePerFile = thePrefs.m_iQuickStartTimePerFile;
	m_iQuickMaxConperFive = thePrefs.m_iQuickMaxConperFive;
	m_iQuickMaxHalfOpen = thePrefs.m_iQuickMaxHalfOpen;
	m_iQuickMaxConnections = thePrefs.m_iQuickMaxConnections;
	// NEO: QS END

	m_bShowActiveDownloadsBold = thePrefs.m_bShowActiveDownloadsBold;
	m_bShowRuntimeOnTitle = thePrefs.m_bShowRuntimeOnTitle;
	m_bShowDownloadColor = thePrefs.m_bShowDownloadColor;
	m_bShowFileStatusIcons = thePrefs.m_bShowFileStatusIcons;
	m_bDirectoryWatcher = thePrefs.m_bDirectoryWatcher;
	m_bAutoDropSystem = thePrefs.m_bAutoDropSystem;
	m_bFunnyNick = thePrefs.m_bFunnyNick;
	m_bShowSessionDownload = thePrefs.m_bShowSessionDownload;
	m_iDisconnectTime = thePrefs.m_iDisconnectTime;
	m_bUsePercentBar = thePrefs.m_bUsePercentBar; // ZZUL-TRA :: PercentBar

//ZZUL-TRA :: PowerShare :: Start
    m_iPowershareMode = thePrefs.m_iPowershareMode;
	m_iPowerShareLimit = thePrefs.PowerShareLimit; 
	m_iPsAmountLimit = thePrefs.GetPsAmountLimit(); 
//ZZUL-TRA :: PowerShare :: End

	// ZZUL-TRA :: InvisibleMode :: Start
	m_bInvisibleMode = thePrefs.m_bInvisibleMode;
	m_sInvisibleModeKey = thePrefs.m_cInvisibleModeHotKey;
	m_sInvisibleModeMod = ModKey[thePrefs.m_iInvisibleModeHotKeyModifier];
	// ZZUL-TRA :: InvisibleMode :: End

    //>>> shadow2004::IP2Country [EastShare]
	m_iI2CCaption = thePrefs.GetIP2CountryNameMode();
	m_bI2CShowFlag = thePrefs.IsIP2CountryShowFlag();
	//m_bIP2CUpdate = thePrefs.IsAutoUpdateIP2CountryEnabled();
    //<<< shadow2004::IP2Country [EastShare]

    CPropertyPage::OnInitDialog();
	InitWindowStyles(this);
	m_ctrlTreeOptions.SetItemHeight(m_ctrlTreeOptions.GetItemHeight() + 2);

	m_iFileBufferSize = thePrefs.m_iFileBufferSize;
	m_ctlFileBuffSize.SetRange(16, 7*1024+512, TRUE); // Spike2 - increased a bit to 7,5 MB max. because of IntelliFlush
	int iMin, iMax;
	m_ctlFileBuffSize.GetRange(iMin, iMax);
	m_ctlFileBuffSize.SetPos(m_iFileBufferSize/1024);
	int iPage = 128;
	for (int i = ((iMin+iPage-1)/iPage)*iPage; i < iMax; i += iPage)
		m_ctlFileBuffSize.SetTic(i);
	m_ctlFileBuffSize.SetPageSize(iPage);

	m_iQueueSize = thePrefs.m_iQueueSize;
	m_ctlQueueSize.SetRange(1, 301, TRUE);  // Spike2 - bigger Queue
	m_ctlQueueSize.SetPos(m_iQueueSize/100);
	m_ctlQueueSize.SetTicFreq(10);
	m_ctlQueueSize.SetPageSize(10);

	Localize();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPgTweaks::OnKillActive()
{
	// if prop page is closed by pressing ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	return CPropertyPage::OnKillActive();
}

BOOL CPPgTweaks::OnApply()
{
	// if prop page is closed by pressing ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();

	if (!UpdateData())
		return FALSE;

	thePrefs.SetMaxConsPerFive(m_iMaxConnPerFive ? m_iMaxConnPerFive : DFLT_MAXCONPERFIVE);
	theApp.scheduler->original_cons5s = thePrefs.GetMaxConperFive();
	thePrefs.SetMaxHalfConnections(m_iMaxHalfOpen ? m_iMaxHalfOpen : DFLT_MAXHALFOPEN);
	thePrefs.m_bConditionalTCPAccept = m_bConditionalTCPAccept;

	if (thePrefs.AutoTakeED2KLinks() != m_bAutoTakeEd2kLinks)
	{
		thePrefs.autotakeed2klinks = m_bAutoTakeEd2kLinks;
		if (thePrefs.AutoTakeED2KLinks())
			Ask4RegFix(false, true, false);
		else
			RevertReg();
	}

	if (!thePrefs.log2disk && m_bLog2Disk)
		theLog.Open();
	else if (thePrefs.log2disk && !m_bLog2Disk)
		theLog.Close();
	thePrefs.log2disk = m_bLog2Disk;

	if (thePrefs.GetEnableVerboseOptions())
	{
		if (!thePrefs.GetDebug2Disk() && m_bVerbose && m_bDebug2Disk)
			theVerboseLog.Open();
		else if (thePrefs.GetDebug2Disk() && (!m_bVerbose || !m_bDebug2Disk))
			theVerboseLog.Close();
		thePrefs.debug2disk = m_bDebug2Disk;
		thePrefs.m_bDebugSourceExchange = m_bDebugSourceExchange;
		thePrefs.m_bLogBannedClients = m_bLogBannedClients;
		thePrefs.m_bLogRatingDescReceived = m_bLogRatingDescReceived;
		thePrefs.m_bLogSecureIdent = m_bLogSecureIdent;
		thePrefs.m_bLogFilteredIPs = m_bLogFilteredIPs;
		thePrefs.m_bLogFileSaving = m_bLogFileSaving;
        thePrefs.m_bLogA4AF = m_bLogA4AF;
        thePrefs.m_bEmuLog = m_bLogEmulator; // ZZUL-TRA :: EmulateOthers
#ifdef CLIENTANALYZER
        thePrefs.m_bLogAnalyzerEvents = m_bLogAnalyzerEvents;
#endif
		thePrefs.m_bLogUlDlEvents = m_bLogUlDlEvents;
		thePrefs.m_byLogLevel = 5 - m_iLogLevel;

		thePrefs.m_bVerbose = m_bVerbose; // store after related options were stored!
	}

#ifdef CLIENTANALYZER
	thePrefs.m_uiCreditSystem = m_iCreditSystem; //>>> WiZaRd::ClientAnalyzer
#else
	thePrefs.m_bCreditSystem = m_bCreditSystem;
#endif
	thePrefs.m_iCommitFiles = m_iCommitFiles;
	thePrefs.m_iExtractMetaData = m_iExtractMetaData;
	thePrefs.filterLANIPs = m_bFilterLANIPs;
	thePrefs.m_iFileBufferSize = m_iFileBufferSize;
	thePrefs.m_iQueueSize = m_iQueueSize;
	if (thePrefs.m_bExtControls != m_bExtControls) {
		thePrefs.m_bExtControls = m_bExtControls;
		theApp.emuledlg->transferwnd->GetDownloadList()->CreateMenues();
		theApp.emuledlg->searchwnd->CreateMenus();
		theApp.emuledlg->sharedfileswnd->sharedfilesctrl.CreateMenues();
	}
	thePrefs.m_dwServerKeepAliveTimeout = m_uServerKeepAliveTimeout * 60000;
	thePrefs.m_bSparsePartFiles = m_bSparsePartFiles;
	thePrefs.m_bAllocFull= m_bFullAlloc;
	thePrefs.checkDiskspace = m_bCheckDiskspace;
	thePrefs.m_bResolveSharedShellLinks = m_bResolveShellLinks;
	thePrefs.m_uMinFreeDiskSpace = (UINT)(m_fMinFreeDiskSpaceMB * (1024 * 1024));
	if (thePrefs.GetYourHostname() != m_sYourHostname) {
		thePrefs.SetYourHostname(m_sYourHostname);
		theApp.emuledlg->serverwnd->UpdateMyInfo();
	}
	thePrefs.m_bOpenPortsOnStartUp = m_bFirewallStartup;

    thePrefs.m_bDynUpEnabled = m_bDynUpEnabled;
    thePrefs.minupload = (uint16)m_iDynUpMinUpload;
    thePrefs.m_iDynUpPingTolerance = m_iDynUpPingTolerance;
    thePrefs.m_iDynUpPingToleranceMilliseconds = m_iDynUpPingToleranceMilliseconds;
    thePrefs.m_bDynUpUseMillisecondPingTolerance = (m_iDynUpRadioPingTolerance == 1);
    thePrefs.m_iDynUpGoingUpDivider = m_iDynUpGoingUpDivider;
    thePrefs.m_iDynUpGoingDownDivider = m_iDynUpGoingDownDivider;
    thePrefs.m_iDynUpNumberOfPings = m_iDynUpNumberOfPings;
	thePrefs.m_bAutomaticArcPreviewStart = !m_bAutoArchDisable;

	thePrefs.m_bCloseUPnPOnExit = m_bCloseUPnPOnExit;
	thePrefs.SetSkipWANIPSetup(m_bSkipWANIPSetup);
	thePrefs.SetSkipWANPPPSetup(m_bSkipWANPPPSetup);

	thePrefs.ChangeUserDirMode(m_iShareeMule);

    thePrefs.m_bA4AFSaveCpu = m_bA4AFSaveCpu;

  // ZZUL-TRA :: Advanced official preferences :: Start
	thePrefs.bMiniMuleAutoClose=bMiniMuleAutoClose;
	thePrefs.iMiniMuleTransparency=iMiniMuleTransparency;
	thePrefs.bCreateCrashDump=bCreateCrashDump;
	thePrefs.bIgnoreInstances=bIgnoreInstances;
	thePrefs.sMediaInfo_MediaInfoDllPath=sMediaInfo_MediaInfoDllPath;
	thePrefs.iMaxLogBuff=iMaxLogBuff*1024;
	thePrefs.m_iMaxChatHistory=m_iMaxChatHistory;
	thePrefs.m_iPreviewSmallBlocks=m_iPreviewSmallBlocks;
	thePrefs.m_bRestoreLastMainWndDlg=m_bRestoreLastMainWndDlg;
	thePrefs.m_bRestoreLastLogPane=m_bRestoreLastLogPane;
	thePrefs.m_bPreviewCopiedArchives=m_bPreviewCopiedArchives;
	thePrefs.m_iStraightWindowStyles=m_iStraightWindowStyles;
	thePrefs.m_iLogFileFormat=(ELogFileFormat)m_iLogFileFormat;
	thePrefs.m_bRTLWindowsLayout=m_bRTLWindowsLayout;
	thePrefs.m_bPreviewOnIconDblClk=m_bPreviewOnIconDblClk;
	thePrefs.sInternetSecurityZone=sInternetSecurityZone;
	thePrefs.m_strTxtEditor=sTxtEditor;
	thePrefs.nServerUDPPort=(uint16) iServerUDPPort; 
	thePrefs.m_bRemove2bin=m_bRemoveFilesToBin;
	thePrefs.m_bHighresTimer=m_bHighresTimer;
	thePrefs.m_bTrustEveryHash=m_bTrustEveryHash;
	thePrefs.m_iInspectAllFileTypes=m_iInspectAllFileTypes;
	thePrefs.maxmsgsessions=m_umaxmsgsessions;
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
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	thePrefs.m_iDebugSearchResultDetailLevel=m_iDebugSearchResultDetailLevel;
#endif
	if (m_iCryptTCPPaddingLength > 255 ) m_iCryptTCPPaddingLength=255;
	thePrefs.m_byCryptTCPPaddingLength=(uint8)m_iCryptTCPPaddingLength ;
    thePrefs.m_bAdjustNTFSDaylightFileTime = m_bAdjustNTFSDaylightFileTime;
	thePrefs.m_strDateTimeFormat = m_strDateTimeFormat;
	thePrefs.m_strDateTimeFormat4Log = m_strDateTimeFormat4Log;
	thePrefs.m_strDateTimeFormat4Lists = m_strDateTimeFormat4List;
	thePrefs.m_crLogError = m_crLogError;
	thePrefs.m_crLogWarning = m_crLogWarning;
	thePrefs.m_crLogSuccess = m_crLogSuccess;

	thePrefs.ICH = m_ICH;
	thePrefs.dontcompressavi = m_dontcompressavi;
	thePrefs.m_bShowCopyEd2kLinkCmd = m_bShowCopyEd2kLinkCmd;
	thePrefs.m_bIconflashOnNewMessage = m_bIconflashOnNewMessage;
	thePrefs.m_bShowVerticalHourMarkers = m_bShowVerticalHourMarkers;
	thePrefs.m_bReBarToolbar = !m_bReBarToolbar;
	thePrefs.m_uFileBufferTimeLimit = SEC2MS(m_iFileBufferTimeLimit);
	thePrefs.m_bRearrangeKadSearchKeywords = m_bRearrangeKadSearchKeywords;
	thePrefs.m_bupdatequeuelist = m_bUpdateQueue;
	thePrefs.dontRecreateGraphs = m_bRepaint;
	thePrefs.beepOnError = m_bBeeper;
	thePrefs.msgsecure = m_bMsgOnlySec;
	thePrefs.m_bExtraPreviewWithMenu = m_bExtraPreviewWithMenu;
	thePrefs.m_bShowUpDownIconInTaskbar = m_bShowUpDownIconInTaskbar;
	thePrefs.m_bKeepUnavailableFixedSharedDirs = m_bKeepUnavailableFixedSharedDirs;
	thePrefs.m_bForceSpeedsToKB = m_bForceSpeedsToKB;
// ZZUL-TRA :: Advanced official preferences :: End

    thePrefs.m_bPayBackFirst = m_bIsPayBackFirst;//PayBack First
	thePrefs.m_iPayBackFirstLimit = (uint8)min(m_iPayBackFirstLimit,255);//PayBack First

	// ZZUL-TRA :: Global Hardlimit :: Start
	thePrefs.m_bGlobalHL = m_bIsGlobalHardlimit;
	thePrefs.m_uGlobalHL = m_iGlobalHardlimit;
	// ZZUL-TRA :: Global Hardlimit :: End

	  thePrefs.m_bShowCpu = m_bGetCPU; // ZZUL-TRA :: SysInfo

//ZZUL-TRA :: PowerShare :: Start
	thePrefs.m_iPowershareMode = m_iPowershareMode;
	thePrefs.PowerShareLimit = m_iPowerShareLimit;
	theApp.sharedfiles->UpdatePartsInfo();
	thePrefs.PsAmountLimit = m_iPsAmountLimit;
//ZZUL-TRA :: PowerShare :: End

	thePrefs.m_bShowActiveDownloadsBold = m_bShowActiveDownloadsBold;
	thePrefs.m_bShowRuntimeOnTitle = m_bShowRuntimeOnTitle;
	thePrefs.m_bShowDownloadColor = m_bShowDownloadColor;
	thePrefs.m_bShowFileStatusIcons = m_bShowFileStatusIcons;
	thePrefs.m_bAutoDropSystem = m_bAutoDropSystem;
	thePrefs.m_bFunnyNick = m_bFunnyNick;
	thePrefs.m_bShowSessionDownload = m_bShowSessionDownload;
	thePrefs.m_bUsePercentBar = m_bUsePercentBar; // ZZUL-TRA :: PercentBar

    thePrefs.m_iDisconnectTime = m_iDisconnectTime;
      theApp.emuledlg->serverwnd->serverlistctrl.ShowServerCount();

	if (thePrefs.m_bDirectoryWatcher != m_bDirectoryWatcher) {
		thePrefs.m_bDirectoryWatcher = m_bDirectoryWatcher;
		theApp.ResetDirectoryWatcher();
	}

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

    //>>> shadow2004::IP2Country [EastShare]
	//Check if ip2country is loaded at this point
	if (!theApp.ip2country->IsIP2Country())
		theApp.ip2country->Load();
	thePrefs.SetIP2CountryNameMode((uint8)m_iI2CCaption);
	thePrefs.SetIP2CountryShowFlag(m_bI2CShowFlag);
	//thePrefs.SetAutoUpdateIP2Country(m_bIP2CUpdate);
	theApp.ip2country->Refresh();//refresh passive windows
    //<<< shadow2004::IP2Country [EastShare]

	    // ZZUL-TRA :: InvisibleMode :: Start
		int actualKeyModifier=0;
		for(;actualKeyModifier<8;++actualKeyModifier)
			if(m_sInvisibleModeMod.Compare(ModKey[actualKeyModifier])==0)
				break;
		thePrefs.SetInvisibleMode(m_bInvisibleMode, actualKeyModifier, (char)m_sInvisibleModeKey.GetAt(0));
		// ZZUL-TRA :: InvisibleMode :: End

	if (thePrefs.GetEnableVerboseOptions())
	{
	    theApp.emuledlg->serverwnd->ToggleDebugWindow();
		theApp.emuledlg->serverwnd->UpdateLogTabSelection();
	}
	theApp.downloadqueue->CheckDiskspace();

	SetModified(FALSE);
	return CPropertyPage::OnApply();
}

void CPPgTweaks::OnHScroll(UINT /*nSBCode*/, UINT /*nPos*/, CScrollBar* pScrollBar) 
{
	if (pScrollBar->GetSafeHwnd() == m_ctlFileBuffSize.m_hWnd)
	{
		m_iFileBufferSize = m_ctlFileBuffSize.GetPos() * 1024;
        CString temp;
		temp.Format(_T("%s: %s"), GetResString(IDS_FILEBUFFERSIZE), CastItoXBytes(m_iFileBufferSize, false, false));
		GetDlgItem(IDC_FILEBUFFERSIZE_STATIC)->SetWindowText(temp);
		SetModified(TRUE);
	}
	else if (pScrollBar->GetSafeHwnd() == m_ctlQueueSize.m_hWnd)
	{
		m_iQueueSize = ((CSliderCtrl*)pScrollBar)->GetPos() * 100;
		CString temp;
		temp.Format(_T("%s: %s"), GetResString(IDS_QUEUESIZE), GetFormatedUInt(m_iQueueSize));
		GetDlgItem(IDC_QUEUESIZE_STATIC)->SetWindowText(temp);
		SetModified(TRUE);
	}
}

void CPPgTweaks::Localize(void)
{	
	if(m_hWnd)
	{
		SetWindowText(GetResString(IDS_PW_TWEAK));
		GetDlgItem(IDC_PREFINI_STATIC)->SetWindowText(GetResString(IDS_PW_TWEAK));
		GetDlgItem(IDC_OPENPREFINI)->SetWindowText(GetResString(IDS_OPENPREFINI));

		if (m_htiTCPGroup) m_ctrlTreeOptions.SetItemText(m_htiTCPGroup, GetResString(IDS_TCPIP_CONNS));
		if (m_htiMaxCon5Sec) m_ctrlTreeOptions.SetEditLabel(m_htiMaxCon5Sec, GetResString(IDS_MAXCON5SECLABEL));
		if (m_htiMaxHalfOpen) m_ctrlTreeOptions.SetEditLabel(m_htiMaxHalfOpen, GetResString(IDS_MAXHALFOPENCONS));
		if (m_htiConditionalTCPAccept) m_ctrlTreeOptions.SetItemText(m_htiConditionalTCPAccept, GetResString(IDS_CONDTCPACCEPT));
		if (m_htiAutoTakeEd2kLinks) m_ctrlTreeOptions.SetItemText(m_htiAutoTakeEd2kLinks, GetResString(IDS_AUTOTAKEED2KLINKS));
#ifdef CLIENTANALYZER
//>>> WiZaRd::ClientAnalyzer
		if (m_htiCreditSystem) m_ctrlTreeOptions.SetItemText(m_htiCreditSystem, GetResString(IDS_USECREDITSYSTEM));
		if (m_htiCSNone) m_ctrlTreeOptions.SetItemText(m_htiCSNone, GetResString(IDS_CS_NONE));
		if (m_htiCSOffi) m_ctrlTreeOptions.SetItemText(m_htiCSOffi, GetResString(IDS_CS_OFFI));
		if (m_htiCSAnalyzer) m_ctrlTreeOptions.SetItemText(m_htiCSAnalyzer, GetResString(IDS_CS_ANALYZER));
//<<< WiZaRd::ClientAnalyzer
		if (m_htiLogAnalyzerEvents) m_ctrlTreeOptions.SetItemText(m_htiLogAnalyzerEvents, GetResString(IDS_LOG_ANALYZER_EVENTS));
#else
		if (m_htiCreditSystem) m_ctrlTreeOptions.SetItemText(m_htiCreditSystem, GetResString(IDS_USECREDITSYSTEM));
#endif
		if (m_htiLog2Disk) m_ctrlTreeOptions.SetItemText(m_htiLog2Disk, GetResString(IDS_LOG2DISK));
		if (m_htiVerboseGroup) m_ctrlTreeOptions.SetItemText(m_htiVerboseGroup, GetResString(IDS_VERBOSE));
		if (m_htiVerbose) m_ctrlTreeOptions.SetItemText(m_htiVerbose, GetResString(IDS_ENABLED));
		if (m_htiDebug2Disk) m_ctrlTreeOptions.SetItemText(m_htiDebug2Disk, GetResString(IDS_LOG2DISK));
		if (m_htiDebugSourceExchange) m_ctrlTreeOptions.SetItemText(m_htiDebugSourceExchange, GetResString(IDS_DEBUG_SOURCE_EXCHANGE));
		if (m_htiLogBannedClients) m_ctrlTreeOptions.SetItemText(m_htiLogBannedClients, GetResString(IDS_LOG_BANNED_CLIENTS));
		if (m_htiLogRatingDescReceived) m_ctrlTreeOptions.SetItemText(m_htiLogRatingDescReceived, GetResString(IDS_LOG_RATING_RECV));
		if (m_htiLogSecureIdent) m_ctrlTreeOptions.SetItemText(m_htiLogSecureIdent, GetResString(IDS_LOG_SECURE_IDENT));
		if (m_htiLogFilteredIPs) m_ctrlTreeOptions.SetItemText(m_htiLogFilteredIPs, GetResString(IDS_LOG_FILTERED_IPS));
		if (m_htiLogFileSaving) m_ctrlTreeOptions.SetItemText(m_htiLogFileSaving, GetResString(IDS_LOG_FILE_SAVING));
		if (m_htiLogLevel) m_ctrlTreeOptions.SetEditLabel(m_htiLogLevel, GetResString(IDS_LOG_LEVEL));
		if (m_htiLogA4AF) m_ctrlTreeOptions.SetItemText(m_htiLogA4AF, GetResString(IDS_LOG_A4AF));
		if (m_htiLogEmulator) m_ctrlTreeOptions.SetItemText(m_htiLogEmulator, _T("Log Emulator")); // ZZUL-TRA :: EmulateOthers
		if (m_htiLogUlDlEvents) m_ctrlTreeOptions.SetItemText(m_htiLogUlDlEvents, GetResString(IDS_LOG_ULDL_EVENTS));
		if (m_htiCommit) m_ctrlTreeOptions.SetItemText(m_htiCommit, GetResString(IDS_COMMITFILES));
		if (m_htiCommitNever) m_ctrlTreeOptions.SetItemText(m_htiCommitNever, GetResString(IDS_NEVER));
		if (m_htiCommitOnShutdown) m_ctrlTreeOptions.SetItemText(m_htiCommitOnShutdown, GetResString(IDS_ONSHUTDOWN));
		if (m_htiCommitAlways) m_ctrlTreeOptions.SetItemText(m_htiCommitAlways, GetResString(IDS_ALWAYS));
		if (m_htiExtractMetaData) m_ctrlTreeOptions.SetItemText(m_htiExtractMetaData, GetResString(IDS_EXTRACT_META_DATA));
		if (m_htiExtractMetaDataNever) m_ctrlTreeOptions.SetItemText(m_htiExtractMetaDataNever, GetResString(IDS_NEVER));
		if (m_htiExtractMetaDataID3Lib) m_ctrlTreeOptions.SetItemText(m_htiExtractMetaDataID3Lib, GetResString(IDS_META_DATA_ID3LIB));
		//if (m_htiExtractMetaDataMediaDet) m_ctrlTreeOptions.SetItemText(m_htiExtractMetaDataMediaDet, GetResString(IDS_META_DATA_MEDIADET));
		if (m_htiFilterLANIPs) m_ctrlTreeOptions.SetItemText(m_htiFilterLANIPs, GetResString(IDS_PW_FILTER));
		if (m_htiExtControls) m_ctrlTreeOptions.SetItemText(m_htiExtControls, GetResString(IDS_SHOWEXTSETTINGS));
		if (m_htiServerKeepAliveTimeout) m_ctrlTreeOptions.SetEditLabel(m_htiServerKeepAliveTimeout, GetResString(IDS_SERVERKEEPALIVETIMEOUT));
		if (m_htiSparsePartFiles) m_ctrlTreeOptions.SetItemText(m_htiSparsePartFiles, GetResString(IDS_SPARSEPARTFILES));
		if (m_htiCheckDiskspace) m_ctrlTreeOptions.SetItemText(m_htiCheckDiskspace, GetResString(IDS_CHECKDISKSPACE));
		if (m_htiMinFreeDiskSpace) m_ctrlTreeOptions.SetEditLabel(m_htiMinFreeDiskSpace, GetResString(IDS_MINFREEDISKSPACE));
		if (m_htiYourHostname) m_ctrlTreeOptions.SetEditLabel(m_htiYourHostname, GetResString(IDS_YOURHOSTNAME));	// itsonlyme: hostnameSource
		if (m_htiFirewallStartup) m_ctrlTreeOptions.SetItemText(m_htiFirewallStartup, GetResString(IDS_FO_PREF_STARTUP));
        if (m_htiDynUp) m_ctrlTreeOptions.SetItemText(m_htiDynUp, GetResString(IDS_DYNUP));
		if (m_htiDynUpEnabled) m_ctrlTreeOptions.SetItemText(m_htiDynUpEnabled, GetResString(IDS_DYNUPENABLED));
        if (m_htiDynUpMinUpload) m_ctrlTreeOptions.SetEditLabel(m_htiDynUpMinUpload, GetResString(IDS_DYNUP_MINUPLOAD));
        if (m_htiDynUpPingTolerance) m_ctrlTreeOptions.SetEditLabel(m_htiDynUpPingTolerance, GetResString(IDS_DYNUP_PINGTOLERANCE));
        if (m_htiDynUpGoingUpDivider) m_ctrlTreeOptions.SetEditLabel(m_htiDynUpGoingUpDivider, GetResString(IDS_DYNUP_GOINGUPDIVIDER));
        if (m_htiDynUpGoingDownDivider) m_ctrlTreeOptions.SetEditLabel(m_htiDynUpGoingDownDivider, GetResString(IDS_DYNUP_GOINGDOWNDIVIDER));
        if (m_htiDynUpNumberOfPings) m_ctrlTreeOptions.SetEditLabel(m_htiDynUpNumberOfPings, GetResString(IDS_DYNUP_NUMBEROFPINGS));
        if (m_htiA4AFSaveCpu) m_ctrlTreeOptions.SetItemText(m_htiA4AFSaveCpu, GetResString(IDS_A4AF_SAVE_CPU));
        if (m_htiFullAlloc) m_ctrlTreeOptions.SetItemText(m_htiFullAlloc, GetResString(IDS_FULLALLOC));
		if (m_htiAutoArch) m_ctrlTreeOptions.SetItemText(m_htiAutoArch, GetResString(IDS_DISABLE_AUTOARCHPREV));
        if (m_htiUPnP) m_ctrlTreeOptions.SetItemText(m_htiUPnP, GetResString(IDS_UPNP));
		if (m_htiCloseUPnPPorts) m_ctrlTreeOptions.SetItemText(m_htiCloseUPnPPorts, GetResString(IDS_UPNPCLOSEONEXIT));
		if (m_htiSkipWANIPSetup) m_ctrlTreeOptions.SetItemText(m_htiSkipWANIPSetup, GetResString(IDS_UPNPSKIPWANIP));
		if (m_htiSkipWANPPPSetup) m_ctrlTreeOptions.SetItemText(m_htiSkipWANPPPSetup, GetResString(IDS_UPNPSKIPWANPPP));
		if (m_htiShareeMule) m_ctrlTreeOptions.SetItemText(m_htiShareeMule, GetResString(IDS_SHAREEMULELABEL));
		if (m_htiShareeMuleMultiUser) m_ctrlTreeOptions.SetItemText(m_htiShareeMuleMultiUser, GetResString(IDS_SHAREEMULEMULTI));
		if (m_htiShareeMulePublicUser) m_ctrlTreeOptions.SetItemText(m_htiShareeMulePublicUser, GetResString(IDS_SHAREEMULEPUBLIC));
		if (m_htiShareeMuleOldStyle) m_ctrlTreeOptions.SetItemText(m_htiShareeMuleOldStyle, GetResString(IDS_SHAREEMULEOLD));
		if (m_htiResolveShellLinks) m_ctrlTreeOptions.SetItemText(m_htiResolveShellLinks, GetResString(IDS_RESOLVELINKS));

// ZZUL-TRA :: Advanced official preferences :: Start
		if (m_hti_advanced) m_ctrlTreeOptions.SetItemText(m_hti_advanced, GetResString(IDS_ADVANCEDPREFS));
		if (m_hti_bMiniMuleAutoClose) m_ctrlTreeOptions.SetItemText(m_hti_bMiniMuleAutoClose, GetResString(IDS_MINIMULEAUTOCLOSE));
		if (m_hti_iMiniMuleTransparency) m_ctrlTreeOptions.SetEditLabel(m_hti_iMiniMuleTransparency, GetResString(IDS_MINIMULETRANSPARENCY));
		if (m_hti_bCreateCrashDump) m_ctrlTreeOptions.SetItemText(m_hti_bCreateCrashDump, GetResString(IDS_CREATECRASHDUMP));
		if (m_hti_bIgnoreInstances) m_ctrlTreeOptions.SetItemText(m_hti_bIgnoreInstances, GetResString(IDS_IGNOREINSTANCES));
		if (m_hti_sMediaInfo_MediaInfoDllPath) m_ctrlTreeOptions.SetEditLabel(m_hti_sMediaInfo_MediaInfoDllPath, GetResString(IDS_MEDIAINFO_MEDIAINFODLLPATH));
		if (m_hti_iMaxLogBuff) m_ctrlTreeOptions.SetEditLabel(m_hti_iMaxLogBuff, GetResString(IDS_MAXLOGBUFF));
		if (m_hti_m_iMaxChatHistory) m_ctrlTreeOptions.SetEditLabel(m_hti_m_iMaxChatHistory, GetResString(IDS_MAXCHATHISTORY));
		if (m_hti_m_iPreviewSmallBlocks) m_ctrlTreeOptions.SetItemText(m_hti_m_iPreviewSmallBlocks, GetResString(IDS_PREVIEWSMALLBLOCKS));
		if (m_hti_m_bRestoreLastMainWndDlg) m_ctrlTreeOptions.SetItemText(m_hti_m_bRestoreLastMainWndDlg, GetResString(IDS_RESTORELASTMAINWNDDLG));
		if (m_hti_m_bRestoreLastLogPane) m_ctrlTreeOptions.SetItemText(m_hti_m_bRestoreLastLogPane, GetResString(IDS_RESTORELASTLOGPANE));
		if (m_hti_m_bPreviewCopiedArchives) m_ctrlTreeOptions.SetItemText(m_hti_m_bPreviewCopiedArchives, GetResString(IDS_PREVIEWCOPIEDARCHIVES));
		if (m_hti_m_iStraightWindowStyles) m_ctrlTreeOptions.SetItemText(m_hti_m_iStraightWindowStyles, GetResString(IDS_STRAIGHTWINDOWSTYLES));
		if (m_hti_m_iLogFileFormat) m_ctrlTreeOptions.SetItemText(m_hti_m_iLogFileFormat, GetResString(IDS_LOGFILEFORMAT));
		if (m_hti_m_bRTLWindowsLayout) m_ctrlTreeOptions.SetItemText(m_hti_m_bRTLWindowsLayout, GetResString(IDS_RTLWINDOWSLAYOUT));
		if (m_hti_m_bPreviewOnIconDblClk) m_ctrlTreeOptions.SetItemText(m_hti_m_bPreviewOnIconDblClk, GetResString(IDS_PREVIEWONICONDBLCLK));
		if (m_hti_sInternetSecurityZone) m_ctrlTreeOptions.SetEditLabel(m_hti_sInternetSecurityZone, GetResString(IDS_INTERNETSECURITYZONE));
		if (m_hti_sTxtEditor) m_ctrlTreeOptions.SetEditLabel(m_hti_sTxtEditor, GetResString(IDS_TXTEDITOR));
		if (m_hti_iServerUDPPort) m_ctrlTreeOptions.SetEditLabel(m_hti_iServerUDPPort, GetResString(IDS_SERVERUDPPORT));
		if (m_hti_m_bRemoveFilesToBin) m_ctrlTreeOptions.SetItemText(m_hti_m_bRemoveFilesToBin, GetResString(IDS_REMOVEFILESTOBIN));
		if (m_hti_HighresTimer) m_ctrlTreeOptions.SetItemText(m_hti_HighresTimer, GetResString(IDS_HIGHRESTIMER));
		if (m_hti_TrustEveryHash) m_ctrlTreeOptions.SetItemText(m_hti_TrustEveryHash, GetResString(IDS_TRUSTEVERYHASH));
		if (m_htiICH) m_ctrlTreeOptions.SetItemText(m_htiICH, GetResString(IDS_X_ICH));
		if (m_hti_InspectAllFileTypes) m_ctrlTreeOptions.SetItemText(m_hti_InspectAllFileTypes, GetResString(IDS_INSPECTALLFILETYPES));
		if (m_hti_maxmsgsessions) m_ctrlTreeOptions.SetEditLabel(m_hti_maxmsgsessions, GetResString(IDS_MAXMSGSESSIONS));
		if (m_hti_PreferRestrictedOverUser) m_ctrlTreeOptions.SetItemText(m_hti_PreferRestrictedOverUser, GetResString(IDS_PREFERRESTRICTEDOVERUSER));
		if (m_hti_UseUserSortedServerList) m_ctrlTreeOptions.SetItemText(m_hti_UseUserSortedServerList, GetResString(IDS_USEUSERSORTEDSERVERLIST));
		if (m_hti_WebFileUploadSizeLimitMB) m_ctrlTreeOptions.SetEditLabel(m_hti_WebFileUploadSizeLimitMB, GetResString(IDS_WEBFILEUPLOADSIZELIMITMB));
		if (m_hti_AllowedIPs) m_ctrlTreeOptions.SetEditLabel(m_hti_AllowedIPs, GetResString(IDS_ALLOWEDIPS));
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
		if (m_hti_DebugSearchResultDetailLevel) m_ctrlTreeOptions.SetEditLabel(m_hti_DebugSearchResultDetailLevel, GetResString(IDS_DEBUGSEARCHDETAILLEVEL));
#endif
		if (m_htiCryptTCPPaddingLength) m_ctrlTreeOptions.SetEditLabel(m_htiCryptTCPPaddingLength, GetResString(IDS_CRYPTTCPPADDINGLENGTH));
		if (m_htiAdjustNTFSDaylightFileTime) m_ctrlTreeOptions.SetItemText(m_htiAdjustNTFSDaylightFileTime, GetResString(IDS_X_ADJUSTNTFSDAYLIGHTFILETIME));
		if (m_htidatetimeformat) m_ctrlTreeOptions.SetEditLabel(m_htidatetimeformat, GetResString(IDS_X_DATETIMEFORMAT));
		if (m_htidatetimeformat4log) m_ctrlTreeOptions.SetEditLabel(m_htidatetimeformat4log, GetResString(IDS_X_DATETIMEFORMAT4LOG));
		if (m_htidatetimeformat4list) m_ctrlTreeOptions.SetEditLabel(m_htidatetimeformat4list, GetResString(IDS_X_DATETIMEFORMAT4LIST));
		if (m_htiLogError) m_ctrlTreeOptions.SetEditLabel(m_htiLogError, GetResString(IDS_X_LOGERROR));
		if (m_htiLogWarning) m_ctrlTreeOptions.SetEditLabel(m_htiLogWarning, GetResString(IDS_X_LOGWARNING));
		if (m_htiLogSuccess) m_ctrlTreeOptions.SetEditLabel(m_htiLogSuccess, GetResString(IDS_X_LOGSUCCESS));
		if (m_htiShowVerticalHourMarkers) m_ctrlTreeOptions.SetItemText(m_htiShowVerticalHourMarkers, GetResString(IDS_X_SHOWVERTICALHOURMARKERS));
		if (m_htiReBarToolbar) m_ctrlTreeOptions.SetItemText(m_htiReBarToolbar, GetResString(IDS_X_REBARTOOLBAR));
		if (m_htiIconflashOnNewMessage) m_ctrlTreeOptions.SetItemText(m_htiIconflashOnNewMessage, GetResString(IDS_X_ICON_FLASH_ON_NEW_MESSAGE));
		if (m_htiShowCopyEd2kLinkCmd) m_ctrlTreeOptions.SetItemText(m_htiShowCopyEd2kLinkCmd, GetResString(IDS_X_SHOWCOPYED2KLINK));
		if (m_htidontcompressavi) m_ctrlTreeOptions.SetItemText(m_htidontcompressavi, GetResString(IDS_X_DONTCOMPRESSAVI));
		if (m_htiFileBufferTimeLimit) m_ctrlTreeOptions.SetEditLabel(m_htiFileBufferTimeLimit, GetResString(IDS_X_FILE_BUFFER_TIME_LIMIT));
		if (m_htiRearrangeKadSearchKeywords) m_ctrlTreeOptions.SetItemText(m_htiRearrangeKadSearchKeywords, GetResString(IDS_X_REARRANGEKADSEARCH));
		if (m_htiUpdateQueue) m_ctrlTreeOptions.SetItemText(m_htiUpdateQueue, GetResString(IDS_UPDATEQUEUE));
		if (m_htiRepaint) m_ctrlTreeOptions.SetItemText(m_htiRepaint, GetResString(IDS_REPAINTGRAPHS));
		if (m_htiBeeper) m_ctrlTreeOptions.SetItemText(m_htiBeeper, GetResString(IDS_PW_BEEP));
		if (m_htiMsgOnlySec) m_ctrlTreeOptions.SetItemText(m_htiMsgOnlySec, GetResString(IDS_MSGONLYSEC));
		if (m_htiExtraPreviewWithMenu) m_ctrlTreeOptions.SetItemText(m_htiExtraPreviewWithMenu, GetResString(IDS_EXTRAPREVIEWWITHMENU));
		if (m_htiShowUpDownIconInTaskbar) m_ctrlTreeOptions.SetItemText(m_htiShowUpDownIconInTaskbar, GetResString(IDS_SHOWUPDOWNICONINTASKBAR));
		if (m_htiKeepUnavailableFixedSharedDirs) m_ctrlTreeOptions.SetItemText(m_htiKeepUnavailableFixedSharedDirs, GetResString(IDS_KEEPUNAVAILABLEFIXEDSHAREDDIRS));
		if (m_htiForceSpeedsToKB) m_ctrlTreeOptions.SetItemText(m_htiForceSpeedsToKB, GetResString(IDS_FORCESPEEDSTOKB));
// ZZUL-TRA :: Advanced official preferences :: End

        if (m_htiIsPayBackFirst) m_ctrlTreeOptions.SetItemText(m_htiIsPayBackFirst, GetResString(IDS_PAYBACKFIRST));//PayBack First
		if (m_htiPayBackFirstLimit) m_ctrlTreeOptions.SetEditLabel(m_htiPayBackFirstLimit, GetResString(IDS_PAYBACKFIRSTLIMIT));//PayBack First
// NEO: QS - [QuickStart]
		if (m_htiQuickStartEnable) m_ctrlTreeOptions.SetItemText(m_htiQuickStartEnable,GetResString(IDS_X_QUICK_START_ENABLE));// morph4u
		if (m_htiQuickStartTime) m_ctrlTreeOptions.SetEditLabel(m_htiQuickStartTime,GetResString(IDS_X_QUICK_START_TIME));
		if (m_htiQuickStartTimePerFile) m_ctrlTreeOptions.SetEditLabel(m_htiQuickStartTimePerFile,GetResString(IDS_X_QUICK_START_TIME_PER_FILE));
		if (m_htiQuickMaxConperFive) m_ctrlTreeOptions.SetEditLabel(m_htiQuickMaxConperFive,GetResString(IDS_X_QUICK_START_MAXPER5));
		if (m_htiQuickMaxHalfOpen) m_ctrlTreeOptions.SetEditLabel(m_htiQuickMaxHalfOpen,GetResString(IDS_X_QUICK_START_MAXHALF));
		if (m_htiQuickMaxConnections) m_ctrlTreeOptions.SetEditLabel(m_htiQuickMaxConnections,GetResString(IDS_X_QUICK_START_MAXCON));
		// NEO: QS END

        //>>> shadow2004::IP2Country [EastShare]
		if(m_htiI2CGroup) m_ctrlTreeOptions.SetItemText(m_htiI2CGroup, GetResString(IDS_IP2COUNTRY));
		if(m_htiI2C2Letter) m_ctrlTreeOptions.SetItemText(m_htiI2C2Letter, GetResString(IDS_IP2COUNTRY_2LETTERS));
		if(m_htiI2C3Letter) m_ctrlTreeOptions.SetItemText(m_htiI2C3Letter, GetResString(IDS_IP2COUNTRY_3LETTERS));
		if(m_htiI2CFullCaption) m_ctrlTreeOptions.SetItemText(m_htiI2CFullCaption, GetResString(IDS_IP2COUNTRY_FULLCAPTION));
		if(m_htiI2CShowFlag) m_ctrlTreeOptions.SetItemText(m_htiI2CShowFlag, GetResString(IDS_IP2COUNTRY_SHOWFLAGS));
		//if (m_htiIP2CUpdate) m_ctrlTreeOptions.SetItemText(m_htiIP2CUpdate, GetResString(IDS_MIMUPDATE)); 
		//<<< shadow2004::IP2Country [EastShare]
//ZZUL-TRA :: PowerShare :: Start
		if (m_htiPsAmountLimit) m_ctrlTreeOptions.SetEditLabel(m_htiPsAmountLimit, GetResString(IDS_PS_AMOUNT_LIMIT));
		if (m_htiPowershareMode) m_ctrlTreeOptions.SetItemText(m_htiPowershareMode, GetResString(IDS_POWERSHARE));
		if (m_htiPowershareDisabled) m_ctrlTreeOptions.SetItemText(m_htiPowershareDisabled, GetResString(IDS_DISABLED));
		if (m_htiPowershareActivated) m_ctrlTreeOptions.SetItemText(m_htiPowershareActivated, GetResString(IDS_ENABLED));
		if (m_htiPowershareAuto) m_ctrlTreeOptions.SetItemText(m_htiPowershareAuto, GetResString(IDS_PRIOAUTO));
		if (m_htiPowershareLimited) m_ctrlTreeOptions.SetItemText(m_htiPowershareLimited, GetResString(IDS_POWERSHARE_LIMITED));
		if (m_htiPowerShareLimit) m_ctrlTreeOptions.SetEditLabel(m_htiPowerShareLimit, GetResString(IDS_POWERSHARE_LIMIT));
//ZZUL-TRA :: PowerShare :: End
	
		if(m_htiShowActiveDownloadsBold) m_ctrlTreeOptions.SetItemText(m_htiShowActiveDownloadsBold, GetResString(IDS_DOWNLOADS_BOLD));
		if(m_htiShowRuntimeOnTitle) m_ctrlTreeOptions.SetItemText(m_htiShowRuntimeOnTitle, GetResString(IDS_RUNTIME_NICK));
		if(m_htiShowDownloadColor) m_ctrlTreeOptions.SetItemText(m_htiShowDownloadColor, GetResString(IDS_DOWNLOAD_COLOR));
		if(m_htiShowFileStatusIcons) m_ctrlTreeOptions.SetItemText(m_htiShowFileStatusIcons, GetResString(IDS_STATUS_ICONS));
		if(m_htiShowSessionDownload) m_ctrlTreeOptions.SetItemText(m_htiShowSessionDownload, GetResString(IDS_SESSION_DOWNLOAD));
		if(m_htiUsePercentBar) m_ctrlTreeOptions.SetItemText(m_htiUsePercentBar, GetResString(IDS_PERCENT_BAR)); // ZZUL-TRA :: PercentBar
		if(m_htiDirectoryWatcher) m_ctrlTreeOptions.SetItemText(m_htiDirectoryWatcher, GetResString(IDS_DIRECTORY_WATCHER));
		if(m_htiAutoDropSystem) m_ctrlTreeOptions.SetItemText(m_htiAutoDropSystem, GetResString(IDS_AUTO_DROP));
		if(m_htiFunnyNick) m_ctrlTreeOptions.SetItemText(m_htiFunnyNick, GetResString(IDS_FUNNY_NICK));
		if(m_htiDisconnectTime) m_ctrlTreeOptions.SetEditLabel(m_htiDisconnectTime, GetResString(IDS_DISCONNECT_TIME));

// ZZUL-TRA :: InvisibleMode :: Start
		if ( m_htiInvisibleMode )
		{
			m_ctrlTreeOptions.SetItemText(m_htiInvisibleModeMod, GetResString(IDS_INVMODE_MODKEY));
			m_ctrlTreeOptions.SetItemText(m_htiInvisibleModeKey, GetResString(IDS_INVMODE_VKEY));

			m_ctrlTreeOptions.SetComboText(m_htiInvisibleModeMod, m_sInvisibleModeMod);		
			m_ctrlTreeOptions.SetComboText(m_htiInvisibleModeKey, m_sInvisibleModeKey);
	
			BOOL bCheck;
			if(m_ctrlTreeOptions.GetCheckBox(m_htiInvisibleMode, /*m_bInvisibleMode*/bCheck)){
				CString text = GetResString(IDS_INVMODE);
				if(/*m_bInvisibleMode*/bCheck)
					text.AppendFormat(_T(" (%s + %s)"),m_sInvisibleModeMod, m_sInvisibleModeKey);
				m_ctrlTreeOptions.SetItemText(m_htiInvisibleMode, text);
			}
		}
		// ZZUL-TRA :: InvisibleMode :: End

        CString temp;
		temp.Format(_T("%s: %s"), GetResString(IDS_FILEBUFFERSIZE), CastItoXBytes(m_iFileBufferSize, false, false));
		GetDlgItem(IDC_FILEBUFFERSIZE_STATIC)->SetWindowText(temp);
		temp.Format(_T("%s: %s"), GetResString(IDS_QUEUESIZE), GetFormatedUInt(m_iQueueSize));
		GetDlgItem(IDC_QUEUESIZE_STATIC)->SetWindowText(temp);
	}
}

void CPPgTweaks::OnDestroy()
{
	m_ctrlTreeOptions.DeleteAllItems();
	m_ctrlTreeOptions.DestroyWindow();
	m_bInitializedTreeOpts = false;
	m_htiTCPGroup = NULL;
	m_htiMaxCon5Sec = NULL;
	m_htiMaxHalfOpen = NULL;
	m_htiConditionalTCPAccept = NULL;
	m_htiAutoTakeEd2kLinks = NULL;
	m_htiVerboseGroup = NULL;
	m_htiVerbose = NULL;
	m_htiDebugSourceExchange = NULL;
	m_htiLogBannedClients = NULL;
	m_htiLogRatingDescReceived = NULL;
	m_htiLogSecureIdent = NULL;
	m_htiLogFilteredIPs = NULL;
	m_htiLogFileSaving = NULL;
    m_htiLogA4AF = NULL;
	m_htiLogLevel = NULL;
	m_htiLogUlDlEvents = NULL;
	m_htiCreditSystem = NULL;
#ifdef CLIENTANALYZER
//>>> WiZaRd::ClientAnalyzer
	m_htiCSNone = NULL;
	m_htiCSOffi = NULL;
	m_htiCSAnalyzer = NULL;
//<<< WiZaRd::ClientAnalyzer
	m_htiLogAnalyzerEvents = NULL;
#endif
	
	m_htiZZULTRA = NULL; //ZZUL-TRA

//ZZUL-TRA :: PowerShare :: Start
    m_htiPowerShareLimit = NULL; 
	m_htiPsAmountLimit = NULL; 
	m_htiPowershareMode = NULL;
	m_htiPowershareDisabled = NULL;
	m_htiPowershareActivated = NULL;
	m_htiPowershareAuto = NULL;
	m_htiPowershareLimited = NULL;
//ZZUL-TRA :: PowerShare :: End

	m_htiLog2Disk = NULL;
	m_htiDebug2Disk = NULL;
	m_htiCommit = NULL;
	m_htiCommitNever = NULL;
	m_htiCommitOnShutdown = NULL;
	m_htiCommitAlways = NULL;
	m_htiFilterLANIPs = NULL;
	m_htiExtControls = NULL;
	m_htiServerKeepAliveTimeout = NULL;
	m_htiSparsePartFiles = NULL;
	m_htiFullAlloc = NULL;
	m_htiCheckDiskspace = NULL;
	m_htiMinFreeDiskSpace = NULL;
	m_htiYourHostname = NULL;
	m_htiFirewallStartup = NULL;
    m_htiDynUp = NULL;
	m_htiDynUpEnabled = NULL;
    m_htiDynUpMinUpload = NULL;
    m_htiDynUpPingTolerance = NULL;
    m_htiDynUpPingToleranceMilliseconds = NULL;
    m_htiDynUpPingToleranceGroup = NULL;
    m_htiDynUpRadioPingTolerance = NULL;
    m_htiDynUpRadioPingToleranceMilliseconds = NULL;
    m_htiDynUpGoingUpDivider = NULL;
    m_htiDynUpGoingDownDivider = NULL;
    m_htiDynUpNumberOfPings = NULL;
    m_htiA4AFSaveCpu = NULL;
	m_htiExtractMetaData = NULL;
	m_htiExtractMetaDataNever = NULL;
	m_htiExtractMetaDataID3Lib = NULL;
	m_htiAutoArch = NULL;
	m_htiUPnP = NULL;
	m_htiCloseUPnPPorts = NULL;
	m_htiSkipWANIPSetup = NULL;
	m_htiSkipWANPPPSetup = NULL;
	m_htiShareeMule = NULL;
	m_htiShareeMuleMultiUser = NULL;
	m_htiShareeMulePublicUser = NULL;
	m_htiShareeMuleOldStyle = NULL;
	//m_htiExtractMetaDataMediaDet = NULL;
	m_htiResolveShellLinks = NULL;
 
// ZZUL-TRA :: Advanced official preferences :: Start
        m_hti_advanced=NULL;
	m_hti_bMiniMuleAutoClose=NULL;
	m_hti_iMiniMuleTransparency=NULL;
	m_hti_bCreateCrashDump=NULL;
	m_hti_bIgnoreInstances=NULL;
	m_hti_sMediaInfo_MediaInfoDllPath=NULL;
	m_hti_iMaxLogBuff=NULL;
	m_hti_m_iMaxChatHistory=NULL;
	m_hti_m_iPreviewSmallBlocks=NULL;
	m_hti_m_bRestoreLastMainWndDlg=NULL;
	m_hti_m_bRestoreLastLogPane=NULL;
	m_hti_m_bPreviewCopiedArchives=NULL;
	m_hti_m_iStraightWindowStyles=NULL;
	m_hti_m_iLogFileFormat=NULL;
	m_hti_m_bRTLWindowsLayout=NULL;
	m_hti_m_bPreviewOnIconDblClk=NULL;
	m_hti_sInternetSecurityZone=NULL;
	m_hti_sTxtEditor=NULL;
	m_hti_iServerUDPPort=NULL;
	m_hti_m_bRemoveFilesToBin=NULL;
	m_hti_HighresTimer=NULL;
	m_hti_TrustEveryHash=NULL;
	m_hti_InspectAllFileTypes=NULL;
	m_hti_maxmsgsessions=NULL;
	m_hti_PreferRestrictedOverUser=NULL;
	m_hti_UseUserSortedServerList=NULL;
	m_hti_WebFileUploadSizeLimitMB =NULL;
	m_hti_AllowedIPs=NULL;
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	m_hti_DebugSearchResultDetailLevel=NULL;
#endif
	m_htiCryptTCPPaddingLength=NULL;
	m_htidatetimeformat = NULL;
	m_htidatetimeformat4log = NULL;
	m_htidatetimeformat4list = NULL;
	m_htiLogError = NULL;
	m_htiLogWarning = NULL;
	m_htiLogSuccess = NULL;
	m_htidontcompressavi = NULL;
	m_htiShowCopyEd2kLinkCmd = NULL;
	m_htiIconflashOnNewMessage = NULL;
	m_htiReBarToolbar = NULL;
	m_htiICH = NULL;
	m_htiShowVerticalHourMarkers = NULL;
    m_htiAdjustNTFSDaylightFileTime = NULL;
    m_htidontcompressavi = NULL;
	m_htiFileBufferTimeLimit = NULL;
	m_htiRearrangeKadSearchKeywords = NULL;
	m_htiUpdateQueue = NULL;
	m_htiRepaint = NULL;
	m_htiBeeper = NULL;
	m_htiMsgOnlySec = NULL;
	m_htiExtraPreviewWithMenu = NULL;
	m_htiShowUpDownIconInTaskbar = NULL;
	m_htiKeepUnavailableFixedSharedDirs = NULL;
	m_htiForceSpeedsToKB = NULL;
// ZZUL-TRA :: Advanced official preferences :: End

   m_htiIsPayBackFirst = NULL; //PayBack First
	m_htiPayBackFirstLimit = NULL; //PayBack First

//>>> shadow2004::IP2Country [EastShare]
	m_htiI2CGroup = NULL;
	m_htiI2C2Letter = NULL;
	m_htiI2C3Letter = NULL;
	m_htiI2CFullCaption = NULL;
	m_htiI2CShowFlag = NULL;
	m_iI2CCaption = 0;
	m_bI2CShowFlag = false;
    //m_htiIP2CUpdate = NULL;
	//m_bIP2CUpdate = false;
//<<< shadow2004::IP2Country [EastShare]

    CPropertyPage::OnDestroy();
}

LRESULT CPPgTweaks::OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam)
{
	if (wParam == IDC_EXT_OPTS)
	{
		BOOL bCheck;
		TREEOPTSCTRLNOTIFY* pton = (TREEOPTSCTRLNOTIFY*)lParam;
		if (m_htiVerbose && pton->hItem == m_htiVerbose)
		{
			//BOOL bCheck;
			if (m_ctrlTreeOptions.GetCheckBox(m_htiVerbose, bCheck))
			{
				if (m_htiDebug2Disk)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiDebug2Disk, bCheck);
				if (m_htiDebugSourceExchange)	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiDebugSourceExchange, bCheck);
				if (m_htiLogBannedClients)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogBannedClients, bCheck);
				if (m_htiLogRatingDescReceived) m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogRatingDescReceived, bCheck);
				if (m_htiLogSecureIdent)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogSecureIdent, bCheck);
				if (m_htiLogFilteredIPs)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogFilteredIPs, bCheck);
				if (m_htiLogFileSaving)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogFileSaving, bCheck);
                if (m_htiLogA4AF)			    m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogA4AF, bCheck);
				if (m_htiLogUlDlEvents)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogUlDlEvents, bCheck);
				if (m_htiLogEmulator)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogEmulator, bCheck); // ZZUL-TRA :: EmulateOthers
#ifdef CLIENTANALYZER
				if (m_htiLogAnalyzerEvents)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogAnalyzerEvents, bCheck); //CA
#endif
			}
		}
		else if (   (m_htiShareeMuleMultiUser  && pton->hItem == m_htiShareeMuleMultiUser)
			     || (m_htiShareeMulePublicUser && pton->hItem == m_htiShareeMulePublicUser)
			     || (m_htiShareeMuleOldStyle   && pton->hItem == m_htiShareeMuleOldStyle))
		{
			if (m_htiShareeMule && !bShowedWarning){
				HTREEITEM tmp;
				int nIndex;
				m_ctrlTreeOptions.GetRadioButton(m_htiShareeMule, nIndex, tmp);
				if (nIndex != thePrefs.m_nCurrentUserDirMode){
					// TODO offer cancel option
					AfxMessageBox(GetResString(IDS_SHAREEMULEWARNING), MB_ICONINFORMATION | MB_OK);
					bShowedWarning = true;
				}
			}
		}

// ZZUL-TRA :: InvisibleMode :: Start
		else if(m_htiInvisibleMode && pton->hItem == m_htiInvisibleMode)
		{
			if(m_ctrlTreeOptions.GetCheckBox(m_htiInvisibleMode, /*m_bInvisibleMode*/bCheck)){
				CString text = GetResString(IDS_INVMODE);
				if(/*m_bInvisibleMode*/bCheck)
					text.AppendFormat(_T(" (%s + %s)"),m_sInvisibleModeMod, m_sInvisibleModeKey);
				m_ctrlTreeOptions.SetItemText(m_htiInvisibleMode, text);
			}
		}
		// ZZUL-TRA :: InvisibleMode :: End

		SetModified();
	}
	return 0;
}

void CPPgTweaks::OnBnClickedOpenprefini()
{
	ShellOpenFile(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("preferences.ini"));
}
