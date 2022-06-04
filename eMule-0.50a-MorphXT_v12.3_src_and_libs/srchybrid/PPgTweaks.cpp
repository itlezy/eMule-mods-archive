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
#include "PPGtooltipped.h" //MORPH leuk_he addded tooltipped
#include "PPgTweaks.h"
#include "Scheduler.h"
#include "DownloadQueue.h"
#include "Preferences.h"
#include "OtherFunctions.h"
#include "TransferDlg.h"
#include "emuledlg.h"
#include "SharedFilesWnd.h"
#include "ServerWnd.h"
#include "HelpIDs.h"
#include "Log.h"
#include "UserMsgs.h"
//MORPH START show less controls
#include "PreferencesDlg.h"
#include "PPgMorphShare.h"
//MORPH END show less controls

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
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDC_OPENPREFINI, OnBnClickedOpenprefini)
END_MESSAGE_MAP()

CPPgTweaks::CPPgTweaks()
// MORPH START leuk_he tooltipped
/*
	: CPropertyPage(CPPgTweaks::IDD)

*/	
	: CPPgtooltipped(CPPgTweaks::IDD)
// MORPH END leuk_he tooltipped
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
//MORPH START - Added by SiRoB, WebCache 1.2f
	m_bLogICHEvents = false; //JP log ICH events
//MORPH END   - Added by SiRoB, WebCache 1.2f
	m_bCreditSystem = false;
	m_bLog2Disk = false;
	m_bDebug2Disk = false;
	m_bDateFileNameLog = false;//Morph - added by AndCycle, Date File Name Log
	m_iCommitFiles = 0;
	m_bFilterLANIPs = false;
	m_bExtControls = false;
	m_uServerKeepAliveTimeout = 0;
    m_dwBindAddr = 0; // MORPH leuk_he bindaddr    	
	m_bSparsePartFiles = false;
	m_bFullAlloc = false;
	m_bCheckDiskspace = false;
	m_fMinFreeDiskSpaceMB = 0.0F;
	(void)m_sYourHostname;
	// Removed by MoNKi [MoNKi: -Improved ICS-Firewall support-]
	/* Moved to PPgEmulespana
	m_bFirewallStartup = false;
	*/
	// End emulEspaña
	m_iLogLevel = 0;
	//MORPH START - Removed by Stulle, Removed dupe USS settings
	/*
    m_bDynUpEnabled = false;
    m_iDynUpMinUpload = 0;
    m_iDynUpPingTolerance = 0;
    m_iDynUpGoingUpDivider = 0;
    m_iDynUpGoingDownDivider = 0;
    m_iDynUpNumberOfPings = 0;
	*/
	//MORPH END   - Removed by Stulle, Removed dupe USS settings
    m_bA4AFSaveCpu = false;
	m_iExtractMetaData = 0;
	m_bAutoArchDisable = true;
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
	//MORPH START - Added by SiRoB, WebCache 1.2f
	m_htiLogICHEvents = NULL; //JP log ICH events
	m_htiCreditSystem = NULL;
	m_htiLog2Disk = NULL;
	m_htiDebug2Disk = NULL;
	m_htiDateFileNameLog = NULL;//Morph - added by AndCycle, Date File Name Log
	m_htiCommit = NULL;
	m_htiCommitNever = NULL;
	m_htiCommitOnShutdown = NULL;
	m_htiCommitAlways = NULL;
	m_htiFilterLANIPs = NULL;
	m_htiExtControls = NULL;
	m_htiServerKeepAliveTimeout = NULL;
    m_htiBindAddr = NULL; // MORPH leuk_he bindaddr
	m_htiSparsePartFiles = NULL;
	m_htiFullAlloc = NULL;
	m_htiCheckDiskspace = NULL;
	m_htiMinFreeDiskSpace = NULL;
	m_htiYourHostname = NULL;
	// Removed by MoNKi [MoNKi: -Improved ICS-Firewall support-]
	/* Moved to PPgEmulespana
	m_htiFirewallStartup = NULL;
	*/
	// End emulEspaña
	m_htiLogLevel = NULL;
	//MORPH START - Removed by Stulle, Removed dupe USS settings
	/*
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
	*/
	//MORPH END   - Removed by Stulle, Removed dupe USS settings
    m_htiA4AFSaveCpu = NULL;
	m_htiLogA4AF = NULL;
	m_htiExtractMetaData = NULL;
	m_htiAutoArch = NULL;
	m_htiShareeMule = NULL;
	m_htiShareeMuleMultiUser = NULL;
	m_htiShareeMulePublicUser = NULL;
	m_htiShareeMuleOldStyle = NULL;
	m_htiResolveShellLinks = NULL;

	// emulEspaña. Added by MoNKi [MoNKi: -UPnPNAT Support-]
	m_bLogUPnP = false;
	m_htiLogUPnP = NULL;
	// End emulEspaña
	 
	//MORPH START leuk_he Advanced official preferences.
	bMiniMuleAutoClose=false;
	iMiniMuleTransparency=0;
	bCreateCrashDump=false;
	bCheckComctl32 =false;
	bCheckShell32=false;
	bIgnoreInstances=false;
	bMediaInfo_RIFF=false;
	bMediaInfo_ID3LIB=false;
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
	m_iDebugSearchResultDetailLevel=0;

	// continue extra official preferences....
	m_hti_advanced=NULL;
	m_hti_bMiniMuleAutoClose=NULL;
	m_hti_iMiniMuleTransparency=NULL;
	m_hti_bCreateCrashDump=NULL;
	m_hti_bCheckComctl32 =NULL;
	m_hti_bCheckShell32=NULL;
	m_hti_bIgnoreInstances=NULL;
	m_hti_sNotifierMailEncryptCertName=NULL;
	m_hti_sMediaInfo_MediaInfoDllPath=NULL;
	m_hti_bMediaInfo_RIFF=NULL;
	m_hti_bMediaInfo_ID3LIB=NULL;
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
	m_hti_DebugSearchResultDetailLevel=NULL;
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
	m_htiDisablePeerCache = NULL;
	m_htiExtraPreviewWithMenu = NULL;
	m_htiShowUpDownIconInTaskbar = NULL;
	m_htiKeepUnavailableFixedSharedDirs = NULL;
	m_htiForceSpeedsToKB = NULL;
	//MORPH END  leuk_he Advanced official preferences.
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
		//MORPH START - Removed by Stulle, Removed dupe USS settings
		/*
		int iImgDynyp = 8;
		*/
		//MORPH END   - Removed by Stulle, Removed dupe USS settings
		int iImgConnection = 8;
		int iImgA4AF = 8;
		int iImgMetaData = 8;
		int iImgShareeMule = 8;
        CImageList* piml = m_ctrlTreeOptions.GetImageList(TVSIL_NORMAL);
		if (piml){
			iImgBackup =	piml->Add(CTempIconLoader(_T("Harddisk")));
			iImgLog =		piml->Add(CTempIconLoader(_T("Log")));
			//MORPH START - Removed by Stulle, Removed dupe USS settings
			/*
			iImgDynyp =		piml->Add(CTempIconLoader(_T("upload")));
			*/
			//MORPH END   - Removed by Stulle, Removed dupe USS settings
			iImgConnection=	piml->Add(CTempIconLoader(_T("connection")));
            iImgA4AF =		piml->Add(CTempIconLoader(_T("Download")));
            iImgMetaData =	piml->Add(CTempIconLoader(_T("MediaInfo")));
			iImgShareeMule =piml->Add(CTempIconLoader(_T("viewfiles")));
		}

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
		//MORPH Start bindaddr
		m_htiBindAddr = m_ctrlTreeOptions.InsertItem(GetResString(IDS_BINDADDR), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiTCPGroup);
		m_ctrlTreeOptions.AddIPAddress(m_htiBindAddr , RUNTIME_CLASS(CTreeOptionsIPAddressCtrl));
		//MORPH End bindaddr

		/////////////////////////////////////////////////////////////////////////////
		// Miscellaneous group
		//
		m_htiAutoTakeEd2kLinks = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_AUTOTAKEED2KLINKS), TVI_ROOT, m_bAutoTakeEd2kLinks);
		m_htiCreditSystem = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_USECREDITSYSTEM), TVI_ROOT, m_bCreditSystem);
		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiCreditSystem,false); //MORPH - Added by SiRoB, Credit System Allways Used
		// Removed by MoNKi [MoNKi: -Improved ICS-Firewall support-]
		/* Moved to PPgEmulespana
		m_htiFirewallStartup = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_FO_PREF_STARTUP), TVI_ROOT, m_bFirewallStartup);
		*/
		// End emulEspaña
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
		m_htiDateFileNameLog = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DATEFILENAMELOG), TVI_ROOT, m_bDateFileNameLog);//Morph - added by AndCycle, Date File Name Log
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
			//MORPH START - Added by SiRoB, WebCache 1.2f
			m_htiLogICHEvents = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOG_IACH), m_htiVerboseGroup, m_bLogICHEvents); //JP log ICH events
			//MORPH END   - Added by SiRoB, WebCache 1.2f
			// emulEspaña. Added by MoNKi [MoNKi: -UPnPNAT Support-]
			m_htiLogUPnP = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_UPNP_LOGEVENTS), m_htiVerboseGroup, m_bLogUPnP);
			// End emulEspaña
		}

		//MORPH START - Removed by Stulle, Removed dupe USS settings
		/*
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
		*/
		//MORPH END   - Removed by Stulle, Removed dupe USS settings

		/////////////////////////////////////////////////////////////////////////////
		// eMule Shared User
		//
		m_htiShareeMule = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_SHAREEMULELABEL), iImgShareeMule, TVI_ROOT);
		m_htiShareeMuleMultiUser = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SHAREEMULEMULTI), m_htiShareeMule, m_iShareeMule == 0);
		m_htiShareeMulePublicUser = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SHAREEMULEPUBLIC), m_htiShareeMule, m_iShareeMule == 1);
		m_htiShareeMuleOldStyle = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SHAREEMULEOLD), m_htiShareeMule, m_iShareeMule == 2);
		

		//MORPH START leuk_he Advanced official preferences.
		m_hti_advanced = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_ADVANCEDPREFS), iImgLog, TVI_ROOT);
		m_hti_bMiniMuleAutoClose=m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MINIMULEAUTOCLOSE),m_hti_advanced,bMiniMuleAutoClose);
		m_hti_iMiniMuleTransparency= m_ctrlTreeOptions.InsertItem(GetResString(IDS_MINIMULETRANSPARENCY),TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,m_hti_advanced);
		m_ctrlTreeOptions.AddEditBox(m_hti_iMiniMuleTransparency, RUNTIME_CLASS(CNumTreeOptionsEdit));

		m_hti_bCreateCrashDump=m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_CREATECRASHDUMP),m_hti_advanced,bCreateCrashDump);
		m_hti_bCheckComctl32 =m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_CHECKCOMCTL32 ),m_hti_advanced,bCheckComctl32 );
		m_hti_bCheckShell32=m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_CHECKSHELL32),m_hti_advanced,bCheckShell32);
		m_hti_bIgnoreInstances=m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_IGNOREINSTANCES),m_hti_advanced,bIgnoreInstances);

		m_hti_sNotifierMailEncryptCertName= m_ctrlTreeOptions.InsertItem(GetResString(IDS_NOTIFIERMAILENCRYPTCERTNAME), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_hti_advanced);
		m_ctrlTreeOptions.AddEditBox(m_hti_sNotifierMailEncryptCertName, RUNTIME_CLASS(CTreeOptionsEditEx));
		

		m_hti_sMediaInfo_MediaInfoDllPath= m_ctrlTreeOptions.InsertItem(GetResString(IDS_MEDIAINFO_MEDIAINFODLLPATH), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_hti_advanced);
		m_ctrlTreeOptions.AddFileEditBox(m_hti_sMediaInfo_MediaInfoDllPath,RUNTIME_CLASS(CTreeOptionsEdit), RUNTIME_CLASS(CTreeOptionsBrowseButton));

		m_hti_bMediaInfo_RIFF=m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MEDIAINFO_RIFF),m_hti_advanced,bMediaInfo_RIFF);
		m_hti_bMediaInfo_ID3LIB=m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MEDIAINFO_ID3LIB),m_hti_advanced,bMediaInfo_ID3LIB);
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

		m_hti_DebugSearchResultDetailLevel=m_ctrlTreeOptions.InsertItem(GetResString(IDS_DEBUGSEARCHDETAILLEVEL),TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,m_hti_advanced);
		m_ctrlTreeOptions.AddEditBox(m_hti_DebugSearchResultDetailLevel, RUNTIME_CLASS(CTreeOptionsEditEx));

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
		m_htiDisablePeerCache = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DISABLEPEERACHE), m_hti_advanced, m_bDisablePeerCache);
		m_htiExtraPreviewWithMenu = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_EXTRAPREVIEWWITHMENU), m_hti_advanced, m_bExtraPreviewWithMenu);
		m_htiShowUpDownIconInTaskbar = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SHOWUPDOWNICONINTASKBAR), m_hti_advanced, m_bShowUpDownIconInTaskbar);
		m_htiKeepUnavailableFixedSharedDirs = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_KEEPUNAVAILABLEFIXEDSHAREDDIRS), m_hti_advanced, m_bKeepUnavailableFixedSharedDirs);
		m_htiForceSpeedsToKB = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_FORCESPEEDSTOKB), m_hti_advanced, m_bForceSpeedsToKB);

		if (m_bExtControls) // show more controls --> still possible to manully expand. 
			m_ctrlTreeOptions.Expand(m_hti_advanced, TVE_EXPAND);
		// MORPH END  leuk_he Advanced official preferences.

	    m_ctrlTreeOptions.Expand(m_htiTCPGroup, TVE_EXPAND);
        if (m_htiVerboseGroup)
		    m_ctrlTreeOptions.Expand(m_htiVerboseGroup, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiCommit, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiCheckDiskspace, TVE_EXPAND);
		//MORPH START - Removed by Stulle, Removed dupe USS settings
		/*
		m_ctrlTreeOptions.Expand(m_htiDynUp, TVE_EXPAND);
        m_ctrlTreeOptions.Expand(m_htiDynUpPingToleranceGroup, TVE_EXPAND);
		*/
		//MORPH END   - Removed by Stulle, Removed dupe USS settings
		m_ctrlTreeOptions.Expand(m_htiExtractMetaData, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiShareeMule, TVE_EXPAND);
        m_ctrlTreeOptions.SendMessage(WM_VSCROLL, SB_TOP);
        m_bInitializedTreeOpts = true;
	}

	/////////////////////////////////////////////////////////////////////////////
	// TCP/IP group
	//
	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiMaxCon5Sec, m_iMaxConnPerFive);
	DDV_MinMaxInt(pDX, m_iMaxConnPerFive, 1, INT_MAX);
	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiMaxHalfOpen, m_iMaxHalfOpen);
	DDV_MinMaxInt(pDX, m_iMaxHalfOpen, 1, INT_MAX);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiConditionalTCPAccept, m_bConditionalTCPAccept);
	DDX_Text(pDX, IDC_EXT_OPTS, m_htiServerKeepAliveTimeout, m_uServerKeepAliveTimeout);
	DDX_TreeIPAddress( pDX, IDC_EXT_OPTS,m_htiBindAddr  , m_dwBindAddr);

	/////////////////////////////////////////////////////////////////////////////
	// Miscellaneous group
	//
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiAutoTakeEd2kLinks, m_bAutoTakeEd2kLinks);
	//MORPH - Removed by SiRoB, Hot fix to show correct disabled checkbox
	/*
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiCreditSystem, m_bCreditSystem);
	*/
	// Removed by MoNKi [MoNKi: -Improved ICS-Firewall support-]
	/* Moved to PPgEmulespana
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiFirewallStartup, m_bFirewallStartup);
	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiFirewallStartup, thePrefs.GetWindowsVersion() == _WINVER_XP_ && IsRunningXPSP2() == 0);
	*/
	// End emulEspaña
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
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiDateFileNameLog, m_bDateFileNameLog);//Morph - added by AndCycle, Date File Name Log
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

	//MORPH START - Added by SiRoB, WebCache 1.2f
	if (m_htiLogICHEvents)			DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiLogICHEvents, m_bLogICHEvents);//JP log ICH events
	if (m_htiLogICHEvents)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogICHEvents, m_bVerbose);//JP log ICH events
	//MORPH END   - Added by SiRoB, WebCache 1.2f
	// emulEspaña. Added by MoNKi [MoNKi: -UPnPNAT Support-]
	if (m_htiLogUPnP)				DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiLogUPnP, m_bLogUPnP);
	if (m_htiLogUPnP)				m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogUPnP, m_bVerbose);
	// End emulEspaña


	//MORPH START - Removed by Stulle, Removed dupe USS settings
	/*
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
	*/
	//MORPH END   - Removed by Stulle, Removed dupe USS settings
    // MORPH START leuk_he Advanced official preferences.
	if (m_hti_bMiniMuleAutoClose) DDX_TreeCheck(pDX, IDC_EXT_OPTS,m_hti_bMiniMuleAutoClose,bMiniMuleAutoClose);
	if (m_hti_iMiniMuleTransparency) {DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_hti_iMiniMuleTransparency, iMiniMuleTransparency);
								   DDV_MinMaxInt(pDX, iMiniMuleTransparency, 0, 100);}
	if(m_hti_bCreateCrashDump) DDX_TreeCheck(pDX,IDC_EXT_OPTS,m_hti_bCreateCrashDump,bCreateCrashDump);
	if(m_hti_bCheckComctl32 ) DDX_TreeCheck(pDX,IDC_EXT_OPTS,m_hti_bCheckComctl32 ,bCheckComctl32 );
	if(m_hti_bCheckShell32) DDX_TreeCheck(pDX,IDC_EXT_OPTS,m_hti_bCheckShell32,bCheckShell32);
	if(m_hti_bIgnoreInstances) DDX_TreeCheck(pDX,IDC_EXT_OPTS,m_hti_bIgnoreInstances,bIgnoreInstances);
	if (m_hti_sNotifierMailEncryptCertName) DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_hti_sNotifierMailEncryptCertName, sNotifierMailEncryptCertName);

	if(m_hti_bMediaInfo_RIFF) DDX_TreeCheck(pDX,IDC_EXT_OPTS,m_hti_bMediaInfo_RIFF,bMediaInfo_RIFF);
	if(m_hti_bMediaInfo_ID3LIB) DDX_TreeCheck(pDX,IDC_EXT_OPTS,m_hti_bMediaInfo_ID3LIB,bMediaInfo_ID3LIB);
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
	if (m_hti_sNotifierMailEncryptCertName) DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_hti_sNotifierMailEncryptCertName, sNotifierMailEncryptCertName);
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
	if(m_hti_DebugSearchResultDetailLevel) DDX_TreeEdit(pDX,IDC_EXT_OPTS,m_hti_DebugSearchResultDetailLevel,m_iDebugSearchResultDetailLevel); //TODO: check string for ip
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
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiDisablePeerCache, m_bDisablePeerCache);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiExtraPreviewWithMenu, m_bExtraPreviewWithMenu);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiShowUpDownIconInTaskbar, m_bShowUpDownIconInTaskbar);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiKeepUnavailableFixedSharedDirs, m_bKeepUnavailableFixedSharedDirs);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiForceSpeedsToKB, m_bForceSpeedsToKB);
    // MORPH END  leuk_he Advanced official preferences.

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
		m_bLogUlDlEvents = thePrefs.m_bLogUlDlEvents;
		//MORPH START - Added by SiRoB, WebCache 1.2f
		m_bLogICHEvents = thePrefs.m_bLogICHEvents;//JP log ICH events
		//MORPH END   - Added by SiRoB, WebCache 1.2f
		// emulEspaña. Added by MoNKi [MoNKi: -UPnPNAT Support-]
		m_bLogUPnP = thePrefs.GetUPnPVerboseLog();
		// End emulEspaña

		m_iLogLevel = 5 - thePrefs.m_byLogLevel;
	}
	m_bLog2Disk = thePrefs.log2disk;
	m_bDateFileNameLog = thePrefs.m_bDateFileNameLog;//Morph - added by AndCycle, Date File Name Log
	m_bCreditSystem = thePrefs.m_bCreditSystem;
	m_iCommitFiles = thePrefs.m_iCommitFiles;
	m_iExtractMetaData = thePrefs.m_iExtractMetaData;
	m_bFilterLANIPs = thePrefs.filterLANIPs;
	m_bExtControls = thePrefs.m_bExtControls;
	m_uServerKeepAliveTimeout = thePrefs.m_dwServerKeepAliveTimeout / 60000;
	//MORPH START leuk_he bindaddr 
	if ((thePrefs.GetBindAddrA()!= NULL)  && inet_addr(thePrefs.GetBindAddrA())!= INADDR_NONE)
	     m_dwBindAddr = ntohl(inet_addr(thePrefs.GetBindAddrA())); 
	else 
		m_dwBindAddr = 0;
   //MORPH END leuk_he bindaddr 
	m_bSparsePartFiles = thePrefs.GetSparsePartFiles();
	m_bFullAlloc= thePrefs.m_bAllocFull;
	m_bCheckDiskspace = thePrefs.checkDiskspace;
	m_bResolveShellLinks = thePrefs.GetResolveSharedShellLinks();
	m_fMinFreeDiskSpaceMB = (float)(thePrefs.m_uMinFreeDiskSpace / (1024.0 * 1024.0));
	m_sYourHostname = thePrefs.GetYourHostname();
	// Removed by MoNKi [MoNKi: -Improved ICS-Firewall support-]
	/* Moved to PPgEmulespana
	m_bFirewallStartup = ((thePrefs.GetWindowsVersion() == _WINVER_XP_) ? thePrefs.m_bOpenPortsOnStartUp : 0); 
	*/
	// End emulEspaña
	m_bAutoArchDisable = !thePrefs.m_bAutomaticArcPreviewStart;

    //MORPH START - Removed by Stulle, Removed dupe USS settings
    /*
    m_bDynUpEnabled = thePrefs.m_bDynUpEnabled;
    m_iDynUpMinUpload = thePrefs.GetMinUpload();
    m_iDynUpPingTolerance = thePrefs.GetDynUpPingTolerance();
    m_iDynUpPingToleranceMilliseconds = thePrefs.GetDynUpPingToleranceMilliseconds();
    m_iDynUpRadioPingTolerance = thePrefs.IsDynUpUseMillisecondPingTolerance()?1:0;
    m_iDynUpGoingUpDivider = thePrefs.GetDynUpGoingUpDivider();
    m_iDynUpGoingDownDivider = thePrefs.GetDynUpGoingDownDivider();
    m_iDynUpNumberOfPings = thePrefs.GetDynUpNumberOfPings();
    */
    //MORPH END   - Removed by Stulle, Removed dupe USS settings


	m_iShareeMule = thePrefs.m_nCurrentUserDirMode;

    m_bA4AFSaveCpu = thePrefs.GetA4AFSaveCpu();

	// MORPH START leuk_he Advanced official preferences.
	bMiniMuleAutoClose=thePrefs.bMiniMuleAutoClose;
	iMiniMuleTransparency=thePrefs.iMiniMuleTransparency;
	bCreateCrashDump=thePrefs.bCreateCrashDump;
	bCheckComctl32 =thePrefs.bCheckComctl32 ;
	bCheckShell32=thePrefs.bCheckShell32;
	bIgnoreInstances=thePrefs.bIgnoreInstances;
	sNotifierMailEncryptCertName=thePrefs.sNotifierMailEncryptCertName;
	sMediaInfo_MediaInfoDllPath=thePrefs.sMediaInfo_MediaInfoDllPath;
	bMediaInfo_RIFF=thePrefs.bMediaInfo_RIFF;
	bMediaInfo_ID3LIB=thePrefs.bMediaInfo_ID3LIB;
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
	m_iDebugSearchResultDetailLevel=thePrefs.GetDebugSearchResultDetailLevel();
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
	m_bDisablePeerCache = !thePrefs.m_bPeerCacheEnabled;
	m_bExtraPreviewWithMenu = thePrefs.GetExtraPreviewWithMenu();
	m_bShowUpDownIconInTaskbar = thePrefs.IsShowUpDownIconInTaskbar();
	m_bKeepUnavailableFixedSharedDirs = thePrefs.m_bKeepUnavailableFixedSharedDirs;
	m_bForceSpeedsToKB = thePrefs.GetForceSpeedsToKB();
	// MORPH END  leuk_he Advanced official preferences.

	m_ctrlTreeOptions.SetImageListColorFlags(theApp.m_iDfltImageListColorFlags);
    CPropertyPage::OnInitDialog();
	InitWindowStyles(this);
	m_ctrlTreeOptions.SetItemHeight(m_ctrlTreeOptions.GetItemHeight() + 2);

	m_iFileBufferSize = thePrefs.m_iFileBufferSize;
	m_ctlFileBuffSize.SetRange(16, 1024+512, TRUE);
	int iMin, iMax;
	m_ctlFileBuffSize.GetRange(iMin, iMax);
	m_ctlFileBuffSize.SetPos(m_iFileBufferSize/1024);
	int iPage = 128;
	for (int i = ((iMin+iPage-1)/iPage)*iPage; i < iMax; i += iPage)
		m_ctlFileBuffSize.SetTic(i);
	m_ctlFileBuffSize.SetPageSize(iPage);

	m_iQueueSize = thePrefs.m_iQueueSize;
	m_ctlQueueSize.SetRange(20, 100, TRUE);
	m_ctlQueueSize.SetPos(m_iQueueSize/100);
	m_ctlQueueSize.SetTicFreq(10);
	m_ctlQueueSize.SetPageSize(10);
    InitTooltips(&m_ctrlTreeOptions); // MORPH leuk_he tooltipped
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
	//MORPH START - Added by Stulle, Create logs dir if saving enabled [Stulle]
	{
		(void)thePrefs.GetMuleDirectory(EMULE_LOGDIR);
	//MORPH END   - Added by Stulle, Create logs dir if saving enabled [Stulle]
		theLog.Open();
	} //MORPH - Added by Stulle, Create logs dir if saving enabled [Stulle]
	else if (thePrefs.log2disk && !m_bLog2Disk)
		theLog.Close();
	thePrefs.log2disk = m_bLog2Disk;

	//Morph Start - added by AndCycle, Date File Name Log
	if(thePrefs.m_bDateFileNameLog != (m_bDateFileNameLog != 0))
	{
		//close log first
		theLog.Close();
		theVerboseLog.Close();

		//reset path
		// Mighty Knife: log files are places in the "log" folder
		VERIFY( theLog.SetFilePath(thePrefs.GetMuleDirectory(EMULE_LOGDIR) + _T("eMule.log")) );
		VERIFY( theVerboseLog.SetFilePath(thePrefs.GetMuleDirectory(EMULE_LOGDIR) + _T("eMule_Verbose.log")) );
		// [end] Mighty Knife

		//open log again
		theLog.Open();
		theVerboseLog.Open();
	}
	thePrefs.m_bDateFileNameLog = m_bDateFileNameLog;
	//Morph End - added by AndCycle, Date File Name Log

	if (thePrefs.GetEnableVerboseOptions())
	{
		if (!thePrefs.GetDebug2Disk() && m_bVerbose && m_bDebug2Disk)
		//MORPH START - Added by Stulle, Create logs dir if saving enabled [Stulle]
		{
			(void)thePrefs.GetMuleDirectory(EMULE_LOGDIR);
		//MORPH END   - Added by Stulle, Create logs dir if saving enabled [Stulle]
			theVerboseLog.Open();
		} //MORPH - Added by Stulle, Create logs dir if saving enabled [Stulle]
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
		thePrefs.m_bLogUlDlEvents = m_bLogUlDlEvents;
		//MORPH START - Added by SiRoB, WebCache 1.2f
		thePrefs.m_bLogICHEvents = m_bLogICHEvents;//JP log ICH events
		//MORPH END   - Added by SiRoB, WebCache 1.2f

		// emulEspaña. Added by MoNKi [MoNKi: -UPnPNAT Support-]
		thePrefs.SetUPnPVerboseLog(m_bLogUPnP);
		// End emulEspaña

		thePrefs.m_byLogLevel = 5 - m_iLogLevel;

		thePrefs.m_bVerbose = m_bVerbose; // store after related options were stored!
	}

	thePrefs.m_bCreditSystem = true/*m_bCreditSystem*/;
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
		//MORPH START show less controls
		// show all controls when we enabled the extended controls
		if(m_bExtControls && thePrefs.m_bShowLessControls)
		{
			theApp.emuledlg->preferenceswnd->m_wndMorphShare.m_bOverideBySetExtControls = true;
			thePrefs.SetLessControls(false);
		}
		//MORPH END show less controls
	}
	thePrefs.m_dwServerKeepAliveTimeout = m_uServerKeepAliveTimeout * 60000;
	// MORPH START leuk_he bindaddr
	if ( m_dwBindAddr == 0) //0.0.0.0
		thePrefs.SetBindAddr(NULL);
	else
	    thePrefs.SetBindAddr(ipstr(ntohl(m_dwBindAddr)));
	//MORPH END leuk_he bindaddr
	thePrefs.m_bSparsePartFiles = m_bSparsePartFiles;
	thePrefs.m_bAllocFull= m_bFullAlloc;
	thePrefs.checkDiskspace = m_bCheckDiskspace;
	thePrefs.m_bResolveSharedShellLinks = m_bResolveShellLinks;
	thePrefs.m_uMinFreeDiskSpace = (UINT)(m_fMinFreeDiskSpaceMB * (1024 * 1024));
	if (thePrefs.GetYourHostname() != m_sYourHostname) {
		thePrefs.SetYourHostname(m_sYourHostname);
		theApp.emuledlg->serverwnd->UpdateMyInfo();
	}
	// Removed by MoNKi [MoNKi: -Improved ICS-Firewall support-]
	/* Moved to PPgEmulespana
	thePrefs.m_bOpenPortsOnStartUp = m_bFirewallStartup;
	*/
	// End emulEspaña

	//MORPH START - Removed by Stulle, Removed dupe USS settings
	/*
    thePrefs.m_bDynUpEnabled = m_bDynUpEnabled;
    thePrefs.minupload = (uint16)m_iDynUpMinUpload;
    thePrefs.m_iDynUpPingTolerance = m_iDynUpPingTolerance;
    thePrefs.m_iDynUpPingToleranceMilliseconds = m_iDynUpPingToleranceMilliseconds;
    thePrefs.m_bDynUpUseMillisecondPingTolerance = (m_iDynUpRadioPingTolerance == 1);
    thePrefs.m_iDynUpGoingUpDivider = m_iDynUpGoingUpDivider;
    thePrefs.m_iDynUpGoingDownDivider = m_iDynUpGoingDownDivider;
    thePrefs.m_iDynUpNumberOfPings = m_iDynUpNumberOfPings;
	*/
	//MORPH END   - Removed by Stulle, Removed dupe USS settings
	thePrefs.m_bAutomaticArcPreviewStart = !m_bAutoArchDisable;


	thePrefs.ChangeUserDirMode(m_iShareeMule);

    thePrefs.m_bA4AFSaveCpu = m_bA4AFSaveCpu;
	//MORPH START leuk_he Advanced official preferences.
	thePrefs.bMiniMuleAutoClose=bMiniMuleAutoClose;
	thePrefs.iMiniMuleTransparency=iMiniMuleTransparency;
	thePrefs.bCreateCrashDump=bCreateCrashDump;
	thePrefs.bCheckComctl32 =bCheckComctl32 ;
	thePrefs.bCheckShell32=bCheckShell32;
	thePrefs.bIgnoreInstances=bIgnoreInstances;
	thePrefs.sNotifierMailEncryptCertName=sNotifierMailEncryptCertName;
	thePrefs.sMediaInfo_MediaInfoDllPath=sMediaInfo_MediaInfoDllPath;
	thePrefs.bMediaInfo_RIFF=bMediaInfo_RIFF;
	thePrefs.bMediaInfo_ID3LIB=bMediaInfo_ID3LIB;
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
	thePrefs.m_iDebugSearchResultDetailLevel=m_iDebugSearchResultDetailLevel;
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
	thePrefs.m_bPeerCacheEnabled = !m_bDisablePeerCache;
	thePrefs.m_bExtraPreviewWithMenu = m_bExtraPreviewWithMenu;
	thePrefs.m_bShowUpDownIconInTaskbar = m_bShowUpDownIconInTaskbar;
	thePrefs.m_bKeepUnavailableFixedSharedDirs = m_bKeepUnavailableFixedSharedDirs;
	thePrefs.m_bForceSpeedsToKB = m_bForceSpeedsToKB;
	//MORPH END  leuk_he Advanced official preferences.

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
		GetDlgItem(IDC_WARNING)->SetWindowText(GetResString(IDS_TWEAKS_WARNING));
		GetDlgItem(IDC_PREFINI_STATIC)->SetWindowText(GetResString(IDS_PW_TWEAK));
		GetDlgItem(IDC_OPENPREFINI)->SetWindowText(GetResString(IDS_OPENPREFINI));

		if (m_htiTCPGroup) m_ctrlTreeOptions.SetItemText(m_htiTCPGroup, GetResString(IDS_TCPIP_CONNS));
		if (m_htiMaxCon5Sec) m_ctrlTreeOptions.SetEditLabel(m_htiMaxCon5Sec, GetResString(IDS_MAXCON5SECLABEL));
		if (m_htiMaxHalfOpen) m_ctrlTreeOptions.SetEditLabel(m_htiMaxHalfOpen, GetResString(IDS_MAXHALFOPENCONS));
		if (m_htiConditionalTCPAccept) m_ctrlTreeOptions.SetItemText(m_htiConditionalTCPAccept, GetResString(IDS_CONDTCPACCEPT));
		if (m_htiAutoTakeEd2kLinks) m_ctrlTreeOptions.SetItemText(m_htiAutoTakeEd2kLinks, GetResString(IDS_AUTOTAKEED2KLINKS));
		if (m_htiCreditSystem) m_ctrlTreeOptions.SetItemText(m_htiCreditSystem, GetResString(IDS_USECREDITSYSTEM));
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
		if (m_htiLogUlDlEvents) m_ctrlTreeOptions.SetItemText(m_htiLogUlDlEvents, GetResString(IDS_LOG_ULDL_EVENTS));

		//MORPH START - Added by SiRoB, WebCache 1.2f
		if (m_htiLogICHEvents) m_ctrlTreeOptions.SetItemText(m_htiLogICHEvents, GetResString(IDS_LOG_IACH));//JP log ICH events
		//MORPH END   - Added by SiRoB, WebCache 1.2f

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
		// MORPH START leuk_he bindaddr
		if (m_htiBindAddr) m_ctrlTreeOptions.SetEditLabel(m_htiBindAddr, GetResString(IDS_BINDADDR));
		// MORPH END leuk_he bindaddr
		if (m_htiCheckDiskspace) m_ctrlTreeOptions.SetItemText(m_htiCheckDiskspace, GetResString(IDS_CHECKDISKSPACE));
		if (m_htiMinFreeDiskSpace) m_ctrlTreeOptions.SetEditLabel(m_htiMinFreeDiskSpace, GetResString(IDS_MINFREEDISKSPACE));
		if (m_htiYourHostname) m_ctrlTreeOptions.SetEditLabel(m_htiYourHostname, GetResString(IDS_YOURHOSTNAME));	// itsonlyme: hostnameSource
		// Removed by MoNKi [MoNKi: -Improved ICS-Firewall support-]
		/* Moved to PPgEmulespana
		if (m_htiFirewallStartup) m_ctrlTreeOptions.SetItemText(m_htiFirewallStartup, GetResString(IDS_FO_PREF_STARTUP));
		*/
		// End emulEspaña
		//MORPH START - Removed by Stulle, Removed dupe USS settings
		/*
        if (m_htiDynUp) m_ctrlTreeOptions.SetItemText(m_htiDynUp, GetResString(IDS_DYNUP));
		if (m_htiDynUpEnabled) m_ctrlTreeOptions.SetItemText(m_htiDynUpEnabled, GetResString(IDS_DYNUPENABLED));
        if (m_htiDynUpMinUpload) m_ctrlTreeOptions.SetEditLabel(m_htiDynUpMinUpload, GetResString(IDS_DYNUP_MINUPLOAD));
        if (m_htiDynUpPingTolerance) m_ctrlTreeOptions.SetEditLabel(m_htiDynUpPingTolerance, GetResString(IDS_DYNUP_PINGTOLERANCE));
        if (m_htiDynUpGoingUpDivider) m_ctrlTreeOptions.SetEditLabel(m_htiDynUpGoingUpDivider, GetResString(IDS_DYNUP_GOINGUPDIVIDER));
        if (m_htiDynUpGoingDownDivider) m_ctrlTreeOptions.SetEditLabel(m_htiDynUpGoingDownDivider, GetResString(IDS_DYNUP_GOINGDOWNDIVIDER));
        if (m_htiDynUpNumberOfPings) m_ctrlTreeOptions.SetEditLabel(m_htiDynUpNumberOfPings, GetResString(IDS_DYNUP_NUMBEROFPINGS));
		*/
		//MORPH END   - Removed by Stulle, Removed dupe USS settings
        if (m_htiA4AFSaveCpu) m_ctrlTreeOptions.SetItemText(m_htiA4AFSaveCpu, GetResString(IDS_A4AF_SAVE_CPU));
        if (m_htiFullAlloc) m_ctrlTreeOptions.SetItemText(m_htiFullAlloc, GetResString(IDS_FULLALLOC));
		if (m_htiAutoArch) m_ctrlTreeOptions.SetItemText(m_htiAutoArch, GetResString(IDS_DISABLE_AUTOARCHPREV));
		if (m_htiShareeMule) m_ctrlTreeOptions.SetItemText(m_htiShareeMule, GetResString(IDS_SHAREEMULELABEL));
		if (m_htiShareeMuleMultiUser) m_ctrlTreeOptions.SetItemText(m_htiShareeMuleMultiUser, GetResString(IDS_SHAREEMULEMULTI));
		if (m_htiShareeMulePublicUser) m_ctrlTreeOptions.SetItemText(m_htiShareeMulePublicUser, GetResString(IDS_SHAREEMULEPUBLIC));
		if (m_htiShareeMuleOldStyle) m_ctrlTreeOptions.SetItemText(m_htiShareeMuleOldStyle, GetResString(IDS_SHAREEMULEOLD));
		if (m_htiResolveShellLinks) m_ctrlTreeOptions.SetItemText(m_htiResolveShellLinks, GetResString(IDS_RESOLVELINKS));

        CString temp;
		temp.Format(_T("%s: %s"), GetResString(IDS_FILEBUFFERSIZE), CastItoXBytes(m_iFileBufferSize, false, false));
		GetDlgItem(IDC_FILEBUFFERSIZE_STATIC)->SetWindowText(temp);
		temp.Format(_T("%s: %s"), GetResString(IDS_QUEUESIZE), GetFormatedUInt(m_iQueueSize));
		GetDlgItem(IDC_QUEUESIZE_STATIC)->SetWindowText(temp);
        //MORPH START leuk_he tooltipped
		SetTool(IDC_FILEBUFFERSIZE,IDS_FILEBUFFERSIZE_TIP);
		SetTool(IDC_QUEUESIZE,IDS_QUEUESIZE_TIP);
		SetTool(IDC_OPENPREFINI,IDS_OPENPREFINI_TIP);
		SetTool(m_htiTCPGroup,IDS_TCPIP_CONNS_TIP);
		SetTool(m_htiMaxCon5Sec,IDS_MAXCON5SECLABEL_TIP);
		SetTool(m_htiMaxHalfOpen,IDS_MAXHALFOPENCONS_TIP);
		SetTool(m_htiConditionalTCPAccept,IDS_CONDTCPACCEPT_TIP);
		SetTool(m_htiServerKeepAliveTimeout,IDS_SERVERKEEPALIVETIMEOUT_TIP);
		SetTool( m_htiBindAddr,IDS_BINDADDR_TIP);
		SetTool(m_htiAutoTakeEd2kLinks,IDS_AUTOTAKEED2KLINKS_TIP);
		SetTool(m_htiCreditSystem,IDS_USECREDITSYSTEM_TIP);
		SetTool(m_htiFilterLANIPs,IDS_PW_FILTER_TIP);
		SetTool(m_htiExtControls,IDS_SHOWEXTSETTINGS_TIP);
		SetTool(m_htiA4AFSaveCpu,IDS_A4AF_SAVE_CPU_TIP);
		SetTool(m_htiAutoArch,IDS_AUTO_ARCH_TIP);
		SetTool(m_htiYourHostname,IDS_YOURHOSTNAME_TIP);
		SetTool(m_htiSparsePartFiles,IDS_SPARSEPARTFILES_TIP);
		SetTool(m_htiFullAlloc,IDS_FULLALLOC_TIP);
		SetTool(m_htiCheckDiskspace,IDS_CHECKDISKSPACE_TIP);
		SetTool(m_htiMinFreeDiskSpace,IDS_MINFREEDISKSPACE_TIP);
		SetTool(m_htiCommit,IDS_COMMITFILES_TIP);
		SetTool(m_htiCommitNever,IDS_NEVER_TIP1);
		SetTool(m_htiCommitOnShutdown,IDS_ONSHUTDOWN_TIP);
		SetTool(m_htiCommitAlways,IDS_ALWAYS_TIP);
		SetTool(m_htiExtractMetaData,IDS_EXTRACT_META_DATA_TIP);
		SetTool(m_htiExtractMetaDataNever,IDS_NEVER_TIP2);
		SetTool(m_htiExtractMetaDataID3Lib,IDS_META_DATA_ID3LIB_TIP);
		SetTool(m_htiLog2Disk,IDS_LOG2DISK_TIP);
		SetTool(m_htiDateFileNameLog,IDS_DATEFILENAMELOG_TIP);
		SetTool(m_htiVerboseGroup,IDS_VERBOSE_TIP);
		SetTool(m_htiVerbose,IDS_ENABLED_TIP);
		SetTool(m_htiLogLevel,IDS_LOG_LEVEL_TIP);
		SetTool(m_htiResolveShellLinks,IDS_RESOLVE_SHARED_SHELL_TIP);
		// MORPH START leuk_he Advanced official preferences.
		if (m_hti_advanced) m_ctrlTreeOptions.SetItemText(m_hti_advanced, GetResString(IDS_ADVANCEDPREFS));
		SetTool(m_hti_advanced,IDS_ADVANCEDPREFS_TIP);
		if (m_hti_bMiniMuleAutoClose) m_ctrlTreeOptions.SetItemText(m_hti_bMiniMuleAutoClose, GetResString(IDS_MINIMULEAUTOCLOSE));
		SetTool(m_hti_bMiniMuleAutoClose,IDS_MINIMULEAUTOCLOSE_TIP);
		if (m_hti_iMiniMuleTransparency) m_ctrlTreeOptions.SetEditLabel(m_hti_iMiniMuleTransparency, GetResString(IDS_MINIMULETRANSPARENCY));
		SetTool(m_hti_iMiniMuleTransparency,IDS_MINIMULETRANSPARENCY_TIP);
		if (m_hti_bCreateCrashDump) m_ctrlTreeOptions.SetItemText(m_hti_bCreateCrashDump, GetResString(IDS_CREATECRASHDUMP));
		SetTool(m_hti_bCreateCrashDump,IDS_CREATECRASHDUMP_TIP);
		if (m_hti_bCheckComctl32) m_ctrlTreeOptions.SetItemText(m_hti_bCheckComctl32, GetResString(IDS_CHECKCOMCTL32));
		SetTool(m_hti_bCheckComctl32 ,IDS_CHECKCOMCTL32_TIP);
		if (m_hti_bCheckShell32) m_ctrlTreeOptions.SetItemText(m_hti_bCheckShell32, GetResString(IDS_CHECKSHELL32));
		SetTool(m_hti_bCheckShell32,IDS_CHECKSHELL32_TIP);
		if (m_hti_bIgnoreInstances) m_ctrlTreeOptions.SetItemText(m_hti_bIgnoreInstances, GetResString(IDS_IGNOREINSTANCES));
		SetTool(m_hti_bIgnoreInstances,IDS_IGNOREINSTANCES_TIP);
		if (m_hti_sNotifierMailEncryptCertName) m_ctrlTreeOptions.SetEditLabel(m_hti_sNotifierMailEncryptCertName, GetResString(IDS_NOTIFIERMAILENCRYPTCERTNAME));
		SetTool(m_hti_sNotifierMailEncryptCertName,IDS_NOTIFIERMAILENCRYPTCERTNAME_TIP);
		if (m_hti_sMediaInfo_MediaInfoDllPath) m_ctrlTreeOptions.SetEditLabel(m_hti_sMediaInfo_MediaInfoDllPath, GetResString(IDS_MEDIAINFO_MEDIAINFODLLPATH));
		SetTool(m_hti_sMediaInfo_MediaInfoDllPath,IDS_MEDIAINFO_MEDIAINFODLLPATH_TIP);
		if (m_hti_bMediaInfo_RIFF) m_ctrlTreeOptions.SetItemText(m_hti_bMediaInfo_RIFF, GetResString(IDS_MEDIAINFO_RIFF));
		SetTool(m_hti_bMediaInfo_RIFF,IDS_MEDIAINFO_RIFF_TIP);
		if (m_hti_bMediaInfo_ID3LIB) m_ctrlTreeOptions.SetItemText(m_hti_bMediaInfo_ID3LIB, GetResString(IDS_MEDIAINFO_ID3LIB));
		SetTool(m_hti_bMediaInfo_ID3LIB,IDS_MEDIAINFO_ID3LIB_TIP);
		if (m_hti_iMaxLogBuff) m_ctrlTreeOptions.SetEditLabel(m_hti_iMaxLogBuff, GetResString(IDS_MAXLOGBUFF));
		SetTool(m_hti_iMaxLogBuff,IDS_MAXLOGBUFF_TIP);
		if (m_hti_m_iMaxChatHistory) m_ctrlTreeOptions.SetEditLabel(m_hti_m_iMaxChatHistory, GetResString(IDS_MAXCHATHISTORY));
		SetTool(m_hti_m_iMaxChatHistory,IDS_MAXCHATHISTORY_TIP);
		if (m_hti_m_iPreviewSmallBlocks) m_ctrlTreeOptions.SetEditLabel(m_hti_m_iPreviewSmallBlocks, GetResString(IDS_PREVIEWSMALLBLOCKS));
		SetTool(m_hti_m_iPreviewSmallBlocks,IDS_PREVIEWSMALLBLOCKS_TIP);
		if (m_hti_m_bRestoreLastMainWndDlg) m_ctrlTreeOptions.SetItemText(m_hti_m_bRestoreLastMainWndDlg, GetResString(IDS_RESTORELASTMAINWNDDLG));
		SetTool(m_hti_m_bRestoreLastMainWndDlg,IDS_RESTORELASTMAINWNDDLG_TIP);
		if (m_hti_m_bRestoreLastLogPane) m_ctrlTreeOptions.SetItemText(m_hti_m_bRestoreLastLogPane, GetResString(IDS_RESTORELASTLOGPANE));
		SetTool(m_hti_m_bRestoreLastLogPane,IDS_RESTORELASTLOGPANE_TIP);
		if (m_hti_m_bPreviewCopiedArchives) m_ctrlTreeOptions.SetItemText(m_hti_m_bPreviewCopiedArchives, GetResString(IDS_PREVIEWCOPIEDARCHIVES));
		SetTool(m_hti_m_bPreviewCopiedArchives,IDS_PREVIEWCOPIEDARCHIVES_TIP);
		if (m_hti_m_iStraightWindowStyles) m_ctrlTreeOptions.SetEditLabel(m_hti_m_iStraightWindowStyles, GetResString(IDS_STRAIGHTWINDOWSTYLES));
		SetTool(m_hti_m_iStraightWindowStyles,IDS_STRAIGHTWINDOWSTYLES_TIP);
		if (m_hti_m_iLogFileFormat) m_ctrlTreeOptions.SetEditLabel(m_hti_m_iLogFileFormat, GetResString(IDS_LOGFILEFORMAT));
		SetTool(m_hti_m_iLogFileFormat,IDS_LOGFILEFORMAT_TIP);
		if (m_hti_m_bRTLWindowsLayout) m_ctrlTreeOptions.SetItemText(m_hti_m_bRTLWindowsLayout, GetResString(IDS_RTLWINDOWSLAYOUT));
		SetTool(m_hti_m_bRTLWindowsLayout,IDS_RTLWINDOWSLAYOUT_TIP);
		if (m_hti_m_bPreviewOnIconDblClk) m_ctrlTreeOptions.SetItemText(m_hti_m_bPreviewOnIconDblClk, GetResString(IDS_PREVIEWONICONDBLCLK));
		SetTool(m_hti_m_bPreviewOnIconDblClk,IDS_PREVIEWONICONDBLCLK_TIP);
		if (m_hti_sInternetSecurityZone) m_ctrlTreeOptions.SetEditLabel(m_hti_sInternetSecurityZone, GetResString(IDS_INTERNETSECURITYZONE));
		SetTool(m_hti_sInternetSecurityZone,IDS_INTERNETSECURITYZONE_TIP);
		if (m_hti_sTxtEditor) m_ctrlTreeOptions.SetEditLabel(m_hti_sTxtEditor, GetResString(IDS_TXTEDITOR));
		SetTool(m_hti_sTxtEditor,IDS_TXTEDITOR_TIP);
		if (m_hti_iServerUDPPort) m_ctrlTreeOptions.SetEditLabel(m_hti_iServerUDPPort, GetResString(IDS_SERVERUDPPORT));
		SetTool(m_hti_iServerUDPPort,IDS_SERVERUDPPORT_TIP);
		if (m_hti_m_bRemoveFilesToBin) m_ctrlTreeOptions.SetItemText(m_hti_m_bRemoveFilesToBin, GetResString(IDS_REMOVEFILESTOBIN));
		SetTool(m_hti_m_bRemoveFilesToBin,IDS_REMOVEFILESTOBIN_TIP);
		if (m_hti_HighresTimer) m_ctrlTreeOptions.SetItemText(m_hti_HighresTimer, GetResString(IDS_HIGHRESTIMER));
		SetTool(m_hti_HighresTimer,IDS_HIGHRESTIMER_TIP);
		if (m_hti_TrustEveryHash) m_ctrlTreeOptions.SetItemText(m_hti_TrustEveryHash, GetResString(IDS_TRUSTEVERYHASH));
		SetTool(m_hti_TrustEveryHash,IDS_TRUSTEVERYHASH_TIP);
		if (m_htiICH) m_ctrlTreeOptions.SetItemText(m_htiICH, GetResString(IDS_X_ICH));
		SetTool(m_htiICH,IDS_X_ICH_TIP);
		if (m_hti_InspectAllFileTypes) m_ctrlTreeOptions.SetItemText(m_hti_InspectAllFileTypes, GetResString(IDS_INSPECTALLFILETYPES));
		SetTool(m_hti_InspectAllFileTypes,IDS_INSPECTALLFILETYPES_TIP);
		if (m_hti_maxmsgsessions) m_ctrlTreeOptions.SetEditLabel(m_hti_maxmsgsessions, GetResString(IDS_MAXMSGSESSIONS));
		SetTool(m_hti_maxmsgsessions,IDS_MAXMSGSESSIONS_TIP);
		if (m_hti_PreferRestrictedOverUser) m_ctrlTreeOptions.SetItemText(m_hti_PreferRestrictedOverUser, GetResString(IDS_PREFERRESTRICTEDOVERUSER));
		SetTool(m_hti_PreferRestrictedOverUser,IDS_PREFERRESTRICTEDOVERUSER_TIP);
		if (m_hti_UseUserSortedServerList) m_ctrlTreeOptions.SetItemText(m_hti_UseUserSortedServerList, GetResString(IDS_USEUSERSORTEDSERVERLIST));
		SetTool(m_hti_UseUserSortedServerList,IDS_USEUSERSORTEDSERVERLIST_TIP);
		if (m_hti_WebFileUploadSizeLimitMB) m_ctrlTreeOptions.SetEditLabel(m_hti_WebFileUploadSizeLimitMB, GetResString(IDS_WEBFILEUPLOADSIZELIMITMB));
		SetTool(m_hti_WebFileUploadSizeLimitMB ,IDS_WEBFILEUPLOADSIZELIMITMB_TIP);
		if (m_hti_AllowedIPs) m_ctrlTreeOptions.SetEditLabel(m_hti_AllowedIPs, GetResString(IDS_ALLOWEDIPS));
		SetTool(m_hti_AllowedIPs,IDS_ALLOWEDIPS_TIP);
		if (m_hti_DebugSearchResultDetailLevel) m_ctrlTreeOptions.SetEditLabel(m_hti_DebugSearchResultDetailLevel, GetResString(IDS_DEBUGSEARCHDETAILLEVEL));
		SetTool(m_hti_DebugSearchResultDetailLevel,IDS_DEBUGSEARCHRESULTDETAILLEVEL_TIP); 
		if (m_htiCryptTCPPaddingLength) m_ctrlTreeOptions.SetEditLabel(m_htiCryptTCPPaddingLength, GetResString(IDS_CRYPTTCPPADDINGLENGTH));
		SetTool(m_htiCryptTCPPaddingLength,IDS_CRYPTTCPPADDINGLENGTH_TIP);
		if (m_htiAdjustNTFSDaylightFileTime) m_ctrlTreeOptions.SetItemText(m_htiAdjustNTFSDaylightFileTime, GetResString(IDS_X_ADJUSTNTFSDAYLIGHTFILETIME));
		SetTool(m_htiAdjustNTFSDaylightFileTime, IDS_X_ADJUSTNTFSDAYLIGHTFILETIME_TIP);
		if (m_htidatetimeformat) m_ctrlTreeOptions.SetEditLabel(m_htidatetimeformat, GetResString(IDS_X_DATETIMEFORMAT));
		SetTool(m_htidatetimeformat,IDS_DATETIMEFORMAT_TIP);
		if (m_htidatetimeformat4log) m_ctrlTreeOptions.SetEditLabel(m_htidatetimeformat4log, GetResString(IDS_X_DATETIMEFORMAT4LOG));
		SetTool(m_htidatetimeformat4log ,IDS_DATETIMEFORMAT_TIP);
		if (m_htidatetimeformat4list) m_ctrlTreeOptions.SetEditLabel(m_htidatetimeformat4list, GetResString(IDS_X_DATETIMEFORMAT4LIST));
		SetTool(m_htidatetimeformat4list ,IDS_DATETIMEFORMAT_TIP);
		if (m_htiLogError) m_ctrlTreeOptions.SetEditLabel(m_htiLogError, GetResString(IDS_X_LOGERROR));
		SetTool(m_htiLogError  ,  IDS_X_LOGERROR_TIP );
		if (m_htiLogWarning) m_ctrlTreeOptions.SetEditLabel(m_htiLogWarning, GetResString(IDS_X_LOGWARNING));
		SetTool(m_htiLogWarning ,IDS_X_LOGERROR_TIP );
		if (m_htiLogSuccess) m_ctrlTreeOptions.SetEditLabel(m_htiLogSuccess, GetResString(IDS_X_LOGSUCCESS));
		SetTool(m_htiLogSuccess,IDS_X_LOGERROR_TIP );
		if (m_htiShowVerticalHourMarkers) m_ctrlTreeOptions.SetItemText(m_htiShowVerticalHourMarkers, GetResString(IDS_X_SHOWVERTICALHOURMARKERS));
		SetTool(m_htiShowVerticalHourMarkers,IDS_X_SHOWVERTICALHOURMARKERS_TIP);
		if (m_htiReBarToolbar) m_ctrlTreeOptions.SetItemText(m_htiReBarToolbar, GetResString(IDS_X_REBARTOOLBAR));
		SetTool(m_htiReBarToolbar ,IDS_X_REBARTOOLBAR_TIP);
		if (m_htiIconflashOnNewMessage) m_ctrlTreeOptions.SetItemText(m_htiIconflashOnNewMessage, GetResString(IDS_X_ICON_FLASH_ON_NEW_MESSAGE));
		SetTool(m_htiIconflashOnNewMessage,IDS_X_ICON_FLASH_ON_NEW_MESSAGE_TIP);
		if (m_htiShowCopyEd2kLinkCmd) m_ctrlTreeOptions.SetItemText(m_htiShowCopyEd2kLinkCmd, GetResString(IDS_X_SHOWCOPYED2KLINK));
		SetTool(m_htiShowCopyEd2kLinkCmd, IDS_X_SHOWCOPYED2KLINK_TIP);
		if (m_htidontcompressavi) m_ctrlTreeOptions.SetItemText(m_htidontcompressavi, GetResString(IDS_X_DONTCOMPRESSAVI));
		SetTool(m_htidontcompressavi,IDS_X_DONTCOMPRESSAVI_TIP);
		if (m_htiFileBufferTimeLimit) m_ctrlTreeOptions.SetEditLabel(m_htiFileBufferTimeLimit, GetResString(IDS_X_FILE_BUFFER_TIME_LIMIT));
		SetTool(m_htiFileBufferTimeLimit,IDS_X_FILE_BUFFER_TIME_LIMIT_TIP);
		if (m_htiRearrangeKadSearchKeywords) m_ctrlTreeOptions.SetItemText(m_htiRearrangeKadSearchKeywords, GetResString(IDS_X_REARRANGEKADSEARCH));
		SetTool(m_htiRearrangeKadSearchKeywords,IDS_X_REARRANGEKADSEARCH_TIP);
		if (m_htiUpdateQueue) m_ctrlTreeOptions.SetItemText(m_htiUpdateQueue, GetResString(IDS_UPDATEQUEUE));
		SetTool(m_htiUpdateQueue,IDS_UPDATEQUEUE_TIP);
		if (m_htiRepaint) m_ctrlTreeOptions.SetItemText(m_htiRepaint, GetResString(IDS_REPAINTGRAPHS));
		SetTool(m_htiRepaint,IDS_REPAINTGRAPHS_TIP);
		if (m_htiBeeper) m_ctrlTreeOptions.SetItemText(m_htiBeeper, GetResString(IDS_PW_BEEP));
		SetTool(m_htiBeeper,IDS_PW_BEEP_TIP);
		if (m_htiMsgOnlySec) m_ctrlTreeOptions.SetItemText(m_htiMsgOnlySec, GetResString(IDS_MSGONLYSEC));
		SetTool(m_htiMsgOnlySec,IDC_MSGONLYSEC_SEC);
		if (m_htiDisablePeerCache) m_ctrlTreeOptions.SetItemText(m_htiDisablePeerCache, GetResString(IDS_DISABLEPEERACHE));
		SetTool(m_htiDisablePeerCache,IDS_DISABLEPEERACHE_TIP);
		if (m_htiExtraPreviewWithMenu) m_ctrlTreeOptions.SetItemText(m_htiExtraPreviewWithMenu, GetResString(IDS_EXTRAPREVIEWWITHMENU));
		SetTool(m_htiExtraPreviewWithMenu,IDS_EXTRAPREVIEWWITHMENU_TIP);
		if (m_htiShowUpDownIconInTaskbar) m_ctrlTreeOptions.SetItemText(m_htiShowUpDownIconInTaskbar, GetResString(IDS_SHOWUPDOWNICONINTASKBAR));
		SetTool(m_htiShowUpDownIconInTaskbar,IDS_SHOWUPDOWNICONINTASKBAR_TIP);
		if (m_htiKeepUnavailableFixedSharedDirs) m_ctrlTreeOptions.SetItemText(m_htiKeepUnavailableFixedSharedDirs, GetResString(IDS_KEEPUNAVAILABLEFIXEDSHAREDDIRS));
		SetTool(m_htiKeepUnavailableFixedSharedDirs,IDS_KEEPUNAVAILABLEFIXEDSHAREDDIRS_TIP);
		if (m_htiForceSpeedsToKB) m_ctrlTreeOptions.SetItemText(m_htiForceSpeedsToKB, GetResString(IDS_FORCESPEEDSTOKB));
		SetTool(m_htiForceSpeedsToKB,IDS_FORCESPEEDSTOKB_TIP);
		//MORPH END leuk_he tooltipped
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
	//MORPH START - Added by SiRoB, WebCache 1.2f
	m_htiLogICHEvents = NULL;//JP log ICH events
	//MORPH END   - Added by SiRoB, WebCache 1.2f
	m_htiCreditSystem = NULL;
	m_htiLog2Disk = NULL;
	m_htiDebug2Disk = NULL;
	m_htiDateFileNameLog = NULL;//Morph - added by AndCycle, Date File Name Log
	m_htiCommit = NULL;
	m_htiCommitNever = NULL;
	m_htiCommitOnShutdown = NULL;
	m_htiCommitAlways = NULL;
	m_htiFilterLANIPs = NULL;
	m_htiExtControls = NULL;
	m_htiServerKeepAliveTimeout = NULL;
	m_htiBindAddr = NULL; //MORPH leuk_he bindaddr
	m_htiSparsePartFiles = NULL;
	m_htiFullAlloc = NULL;
	m_htiCheckDiskspace = NULL;
	m_htiMinFreeDiskSpace = NULL;
	m_htiYourHostname = NULL;
	// Removed by MoNKi [MoNKi: -Improved ICS-Firewall support-]
	/* Moved to PPgEmulespana
	m_htiFirewallStartup = NULL;
	*/
	// End emulEspaña
	//MORPH START - Removed by Stulle, Removed dupe USS settings
	/*
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
	*/
	//MORPH END   - Removed by Stulle, Removed dupe USS settings
    m_htiA4AFSaveCpu = NULL;
	m_htiExtractMetaData = NULL;
	m_htiExtractMetaDataNever = NULL;
	m_htiExtractMetaDataID3Lib = NULL;
	m_htiAutoArch = NULL;
	m_htiShareeMule = NULL;
	m_htiShareeMuleMultiUser = NULL;
	m_htiShareeMulePublicUser = NULL;
	m_htiShareeMuleOldStyle = NULL;
	//m_htiExtractMetaDataMediaDet = NULL;
	m_htiResolveShellLinks = NULL;
    
	// continue extra official preferences....
	m_hti_advanced=NULL;
	m_hti_bMiniMuleAutoClose=NULL;
	m_hti_iMiniMuleTransparency=NULL;
	m_hti_bCreateCrashDump=NULL;
	m_hti_bCheckComctl32 =NULL;
	m_hti_bCheckShell32=NULL;
	m_hti_bIgnoreInstances=NULL;
	m_hti_sNotifierMailEncryptCertName=NULL;
	m_hti_sMediaInfo_MediaInfoDllPath=NULL;
	m_hti_bMediaInfo_RIFF=NULL;
	m_hti_bMediaInfo_ID3LIB=NULL;
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
	m_hti_DebugSearchResultDetailLevel=NULL;
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
	m_htiDisablePeerCache = NULL;
	m_htiExtraPreviewWithMenu = NULL;
	m_htiShowUpDownIconInTaskbar = NULL;
	m_htiKeepUnavailableFixedSharedDirs = NULL;
	m_htiForceSpeedsToKB = NULL;
	//MORPH END  leuk_he Advanced official preferences.

    CPropertyPage::OnDestroy();
}

LRESULT CPPgTweaks::OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam)
{
	if (wParam == IDC_EXT_OPTS)
	{
		TREEOPTSCTRLNOTIFY* pton = (TREEOPTSCTRLNOTIFY*)lParam;
		if (m_htiVerbose && pton->hItem == m_htiVerbose)
		{
			BOOL bCheck;
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
				//MORPH START - Added by SiRoB, WebCache 1.2f
				if (m_htiLogICHEvents)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogICHEvents, bCheck);//JP log ICH events
				//MORPH END   - Added by SiRoB, WebCache 1.2f
			}
		}
		else if ((m_htiShareeMuleMultiUser && pton->hItem == m_htiShareeMuleMultiUser)
			|| (m_htiShareeMulePublicUser && pton->hItem == m_htiShareeMulePublicUser)
			|| (m_htiShareeMuleOldStyle && pton->hItem == m_htiShareeMuleOldStyle))
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
		SetModified();
	}
	return 0;
}

void CPPgTweaks::OnHelp()
{
	theApp.ShowHelp(eMule_FAQ_Preferences_Extended_Settings);
}

BOOL CPPgTweaks::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgTweaks::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}

void CPPgTweaks::OnBnClickedOpenprefini()
{
	ShellOpenFile(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("preferences.ini"));
}
