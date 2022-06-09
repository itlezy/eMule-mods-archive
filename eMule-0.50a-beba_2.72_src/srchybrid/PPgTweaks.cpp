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
// Tux: LiteMule: Remove Scheduler
#include "DownloadQueue.h"
#include "Preferences.h"
#include "OtherFunctions.h"
#include "TransferDlg.h"
#include "emuledlg.h"
#include "SharedFilesWnd.h"
#include "ServerWnd.h"
// Tux: LiteMule: Remove Help
#include "Log.h"
#include "UserMsgs.h"
#include "PreferencesDlg.h"	// Tux: Improvement: repaint prefs wnd if needed :-)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Tux: Feature: Hidden prefs [start]
static const LPCTSTR _asNetSecurityZones[] = {
	_T("Untrusted"),
	_T("Internet"),
	_T("Intranet"),
	_T("Trusted"),
	_T("LocalMachine")
};
// Tux: Feature: Hidden prefs [end]

/* Tux TODO:
 *
 * Add the Log File Format option...
 *   Unicode . . . -> 0
 *   UTF-8 . . . . -> 1
 * (see Log.cpp and Log.h)
 *
 * ... the point is that I don't know how to convert
 * an ELogFileFormat value into an integer... maybe
 * I should use the enum values... well, in fact, no-
 * one really needs to change the log format (Unicode
 * by default), so I've put this option on a very
 * long queue... by the way, hi folks. :p
 */

#define	DFLT_MAXCONPERFIVE	20
#define DFLT_MAXHALFOPEN	9

///////////////////////////////////////////////////////////////////////////////
// CPPgTweaks dialog

IMPLEMENT_DYNAMIC(CPPgTweaks, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgTweaks, CPropertyPage)
	ON_WM_HSCROLL()
	ON_WM_DESTROY()
	ON_MESSAGE(UM_TREEOPTSCTRL_NOTIFY, OnTreeOptsCtrlNotify)
	// Tux: LiteMule: Remove Help
        ON_BN_CLICKED(IDC_OPENPREFINI, OnBnClickedOpenprefini)
END_MESSAGE_MAP()

CPPgTweaks::CPPgTweaks()
	: CPropertyPage(CPPgTweaks::IDD)
	, m_ctrlTreeOptions(theApp.m_iDfltImageListColorFlags)
{
	// m_iFileBufferSize = 0;	// Tux: Feature: IntelliFlush
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
	m_bLogSUQWT = false;	// Tux: Feature: SUQWT
	m_bLogKadSecurityEvents = false;	// Tux: Improvement: log Kad security events
	m_bLogAICHEvents = false;	// Tux: Improvement: log AICH events
	m_bLogSLSEvents = false;	// Tux: Improvement: log SLS events
	m_bLogCryptEvents = false;	// Tux: Improvement: log CryptLayer events
	m_bLogAnalyzerEvents = false;	// Tux: Feature: Client Analyzer
	m_bCreditSystem = false;
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
	m_iCryptTCPPaddingLength = 128;	// Tux: Feature: Hidden prefs

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
	m_htiLogSUQWT = NULL;	// Tux: Feature: SUQWT
	m_htiLogKadSecurityEvents = NULL;	// Tux: Improvements: log Kad security events
	m_htiLogAICHEvents = NULL;	// Tux: Improvement: log AICH events
	m_htiLogSLSEvents = NULL;	// Tux: Improvement: log SLS events
	m_htiLogCryptEvents = NULL;	// Tux: Improvement: log CryptLayer events
	m_htiLogAnalyzerEvents = NULL;	// Tux: Feature: Client Analyzer
//	m_htiCreditSystem = NULL;
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

	// Tux: Feature: Hidden prefs [start]
	m_htiHiddenPrefs = NULL;
	m_htiRemove2bin = NULL;
	m_htiShowCopyEd2kLinkCmd = NULL;
	m_htiHighresTimer = NULL;
	m_htiIconflashOnNewMessage = NULL;
	m_htiBeepOnError = NULL;
	m_htiServerUDPPort = NULL;
	m_htiRestoreLastMainWndDlg = NULL;
	m_htiRestoreLastLogPane = NULL;
	m_htiDisplayTweaks = NULL;
	m_htiPreviewTweaks = NULL;
	m_htiPreviewSmallBlocks = NULL;
	m_htiPreviewCopiedArchives = NULL;
	m_htiPreviewOnIconDblClk = NULL;
	m_htiExtraPreviewWithMenu = NULL;
	m_htiShowActiveDownloadsBold = NULL;
	m_htiRTLWindowsLayout = NULL;
	m_htiStraightWindowStyles = NULL;
	m_htiUseSystemFontForMainControls = NULL;
#ifdef HAVE_WIN7_SDK_H
	m_htiShowUpDownIconInTaskbar = NULL;
#endif
	m_htiUpdateQueueList = NULL;
	m_htiMaxChatHistory = NULL;
	m_htiMessageFromValidSourcesOnly = NULL;
	m_htiPreferRestrictedOverUser = NULL;
	m_htiTrustEveryHash = NULL;
	m_htiPartiallyPurgeOldKnownFiles = NULL;
	m_htiStatisticsTweaks = NULL;
	m_htiShowVerticalHourMarkers = NULL;
	m_htiDontRecreateStatGraphsOnResize = NULL;
	m_htiInspectAllFileTypes = NULL;
	m_htiCryptTCPPaddingLength = NULL;
	m_htiTxtEditor = NULL;
	m_htiWebServerTweaks = NULL;
	m_htiWebFileUploadSizeLimitMB = NULL;
	m_htiAllowAdminHiLevFunc = NULL;
	m_htiAllowedRemoteAccessIPs = NULL;
	m_htiLogTweaks = NULL;
	m_htiLogError = NULL;
	m_htiLogWarning = NULL;
	m_htiLogSuccess = NULL;
	m_htiInternetSecurityZone = NULL;
	m_htiMiniMuleAutoClose = NULL;
	m_htiMiniMuleTransparency = NULL;
	m_htiAdjustNTFSDaylightFileTime = NULL;
	m_htiKeepUnavailableFixedSharedDirs = NULL;
	m_htiUPnPExt = NULL;
	m_htiUPnPDisableLib = NULL;
	m_htiUPnPDisableServ = NULL;
	m_htiMaxLogFileSize = NULL;
	// Tux: Feature: Hidden prefs [end]

	bReopenPrefs = false;	// Tux: Improvement: repaint prefs wnd if needed :-)
}

CPPgTweaks::~CPPgTweaks()
{
}

void CPPgTweaks::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	// DDX_Control(pDX, IDC_FILEBUFFERSIZE, m_ctlFileBuffSize);	// Tux: Feature: IntelliFlush
	DDX_Control(pDX, IDC_QUEUESIZE, m_ctlQueueSize);
	DDX_Control(pDX, IDC_EXT_OPTS, m_ctrlTreeOptions);
	if (!m_bInitializedTreeOpts)
	{
		int iImgBackup = 8; // default icon
		int iImgLog = 8;
		int iImgDynyp = 8;
		int iImgConnection = 8;
		//int iImgA4AF = 8;	// Tux: Fix: removed unused icon
		int iImgMetaData = 8;
		int iImgUPnP = 8;
		int iImgShareeMule = 8;
		// Tux: Feature: Hidden prefs [start]
		int iImgExtended = 8;
		int iImgMiniMule = 8;
		int iImgDisplay = 8;
		int iImgPreview = 8;
		int iImgStatistics = 8;
		int iImgWebServer = 8;
		// Tux: Feature: Hidden prefs [end]
		CImageList* piml = m_ctrlTreeOptions.GetImageList(TVSIL_NORMAL);
		if (piml){
			iImgBackup =	piml->Add(CTempIconLoader(_T("Harddisk")));
			iImgLog =		piml->Add(CTempIconLoader(_T("Log")));
			iImgDynyp =		piml->Add(CTempIconLoader(_T("upload")));
			iImgConnection=	piml->Add(CTempIconLoader(_T("connection")));
			//iImgA4AF =		piml->Add(CTempIconLoader(_T("Download")));	// Tux: Improvement: removed unused icon
			iImgMetaData =	piml->Add(CTempIconLoader(_T("MediaInfo")));
			iImgUPnP	=	piml->Add(CTempIconLoader(_T("UPNP")));	// Tux: others: changed icon
			iImgShareeMule =piml->Add(CTempIconLoader(_T("viewfiles")));
			// Tux: Feature: Hidden prefs [start]
			iImgExtended =	piml->Add(CTempIconLoader(_T("TWEAK")));
			iImgMiniMule =	piml->Add(CTempIconLoader(_T("MINIMULE")));
			iImgDisplay =	piml->Add(CTempIconLoader(_T("DISPLAY")));
			iImgPreview =   piml->Add(CTempIconLoader(_T("PREVIEW")));
			iImgStatistics =piml->Add(CTempIconLoader(_T("STATSDETAIL")));
			iImgWebServer =	piml->Add(CTempIconLoader(_T("Web")));
			// Tux: Feature: Hidden prefs [end]
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

		/////////////////////////////////////////////////////////////////////////////
		// Miscellaneous group
		//
		m_htiAutoTakeEd2kLinks = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_AUTOTAKEED2KLINKS), TVI_ROOT, m_bAutoTakeEd2kLinks);
		// m_htiCreditSystem = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_USECREDITSYSTEM), TVI_ROOT, m_bCreditSystem);	// Tux: Newbie security: remove UseCreditSystem
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
			m_htiLogSUQWT = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOG_SUQWT_EVENTS), m_htiVerboseGroup, m_bLogSUQWT);	// Tux: Feature: SUQWT
			m_htiLogKadSecurityEvents = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOG_KADSEC_EVENTS), m_htiVerboseGroup, m_bLogKadSecurityEvents);	// Tux: Improvement: log Kad security events
			m_htiLogAICHEvents = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOG_AICH), m_htiVerboseGroup, m_bLogAICHEvents);	// Tux: Improvement: log AICH events
			m_htiLogSLSEvents = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOG_SLS), m_htiVerboseGroup, m_bLogSLSEvents);	// Tux: Improvement: log SLS events
			m_htiLogCryptEvents = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOG_CRYPT), m_htiVerboseGroup, m_bLogCryptEvents);	// Tux: Improvement: log CryptLayer events
			m_htiLogAnalyzerEvents = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOG_ANALYZER_EVENTS), m_htiVerboseGroup, m_bLogAnalyzerEvents);	// Tux: Feature: Client Analyzer
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
		// eMule Shared User	// Tux: Fix: Prefs fix [TimDzang]
		//
		m_htiShareeMule = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_SHAREEMULELABEL), iImgShareeMule, TVI_ROOT);
		m_htiShareeMuleMultiUser = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SHAREEMULEMULTI), m_htiShareeMule, m_iShareeMule == 0);
		m_htiShareeMulePublicUser = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SHAREEMULEPUBLIC), m_htiShareeMule, m_iShareeMule == 1);
		m_htiShareeMuleOldStyle = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SHAREEMULEOLD), m_htiShareeMule, m_iShareeMule == 2);

		// Tux: Feature: Hidden prefs [start]
		if (thePrefs.IsExtControlsEnabled()) { // Tux: Improvement: hide more extended controls
			m_htiHiddenPrefs = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_MORETWEAKS), iImgExtended, TVI_ROOT);
			m_htiRemove2bin = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_REMOVE2BIN), m_htiHiddenPrefs, m_bRemove2bin);
			m_htiShowCopyEd2kLinkCmd = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_COPYED2KLINK), m_htiHiddenPrefs, m_bShowCopyEd2kLinkCmd);
			m_htiHighresTimer = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_USE_HIGHRES_TIMER), m_htiHiddenPrefs, m_bHighresTimer);
			m_htiServerUDPPort = m_ctrlTreeOptions.InsertItem(GetResString(IDS_UDPSERVERPORT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiHiddenPrefs);
			m_ctrlTreeOptions.AddEditBox(m_htiServerUDPPort, RUNTIME_CLASS(CNumTreeOptionsEdit));
			m_htiMiniMule = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_MINIMULE), iImgMiniMule, m_htiHiddenPrefs);
			m_htiMiniMuleAutoClose = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MINIMULE_AUTOCLOSE), m_htiMiniMule, m_bMiniMuleAutoClose);
			m_htiMiniMuleTransparency = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MINIMULE_TRANSPARENCY), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiMiniMule);
			m_ctrlTreeOptions.AddEditBox(m_htiMiniMuleTransparency, RUNTIME_CLASS(CNumTreeOptionsEdit));
			m_htiDisplayTweaks = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_DISPLAY_TWEAKS), iImgDisplay, m_htiHiddenPrefs);
			m_htiIconflashOnNewMessage = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SYSTRAY_FLASH), m_htiDisplayTweaks, m_bIconflashOnNewMessage);
			m_htiBeepOnError = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_BEEP_ON_ERROR), m_htiDisplayTweaks, m_bBeepOnError);
			m_htiRestoreLastMainWndDlg = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_RESTLASTMAINWINDOW), m_htiDisplayTweaks, m_bRestoreLastMainWndDlg);
			m_htiRestoreLastLogPane = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_RESTLASTLOGPANE), m_htiDisplayTweaks, m_bRestoreLastLogPane);
			m_htiShowActiveDownloadsBold = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SHOW_DL_IN_BOLD), m_htiDisplayTweaks, m_bShowActiveDownloadsBold);
			m_htiRTLWindowsLayout = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_RTL_WINDOW_LAYOUT), m_htiDisplayTweaks, m_bRTLWindowsLayout);
			m_htiStraightWindowStyles = m_ctrlTreeOptions.InsertItem(GetResString(IDS_STRAIGHT_WINDOW_STYLES), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDisplayTweaks);
			m_ctrlTreeOptions.AddEditBox(m_htiStraightWindowStyles, RUNTIME_CLASS(CNumTreeOptionsEdit));
			m_htiUseSystemFontForMainControls = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_USE_SYSFONT), m_htiDisplayTweaks, m_bUseSystemFontForMainControls);
#ifdef HAVE_WIN7_SDK_H
			m_htiShowUpDownIconInTaskbar = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SHOW_UPDOWN_ICON), m_htiDisplayTweaks, m_bShowUpDownIconInTaskbar);
#endif
			m_htiUpdateQueueList = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_UPDATE_QUEUE_LIST), m_htiDisplayTweaks, m_bUpdateQueueList);
			m_htiPreviewTweaks = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_PREVIEW_TWEAKS), iImgPreview, m_htiHiddenPrefs);
			m_htiPreviewSmallBlocks = m_ctrlTreeOptions.InsertItem(GetResString(IDS_PREVIEW_SMALL_BLOCKS), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiPreviewTweaks);
			m_ctrlTreeOptions.AddEditBox(m_htiPreviewSmallBlocks, RUNTIME_CLASS(CTreeOptionsEdit));
			m_htiPreviewCopiedArchives = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PREVIEW_COPIED_ARCHIVES), m_htiPreviewTweaks, m_bPreviewCopiedArchives);
			m_htiPreviewOnIconDblClk = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PREVIEW_ON_ICON_DBLCLICK), m_htiPreviewTweaks, m_bPreviewOnIconDblClk);
			m_htiExtraPreviewWithMenu = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PREVIEW_WITH_MENU), m_htiPreviewTweaks, m_bExtraPreviewWithMenu);
			m_htiMaxChatHistory = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MAX_CHAT_HISTORY), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiHiddenPrefs);
			m_ctrlTreeOptions.AddEditBox(m_htiMaxChatHistory, RUNTIME_CLASS(CNumTreeOptionsEdit));
			m_htiMessageFromValidSourcesOnly = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MSG_FROM_VALID_SRC_ONLY), m_htiHiddenPrefs, m_bMessageFromValidSourcesOnly);
			m_htiPreferRestrictedOverUser = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PREFER_RESTRICTED_USER), m_htiHiddenPrefs, m_bPreferRestrictedOverUser);
			m_htiTrustEveryHash = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_TRUST_EVERY_AICH_HASH), m_htiHiddenPrefs, m_bTrustEveryHash);
			m_htiPartiallyPurgeOldKnownFiles = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PARTIALLY_PURGE_OLD_KNOWN_FILES), m_htiHiddenPrefs, m_bPartiallyPurgeOldKnownFiles);
			m_htiStatisticsTweaks = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_STATISTICS_TWEAKS), iImgStatistics, m_htiHiddenPrefs);
			m_htiShowVerticalHourMarkers = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SHOW_VERTICAL_HOUR_MARKERS), m_htiStatisticsTweaks, m_bShowVerticalHourMarkers);
			m_htiDontRecreateStatGraphsOnResize = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DONT_RECREATE_STAT_GRAPHS), m_htiStatisticsTweaks, m_bDontRecreateStatGraphsOnResize);
			m_htiInspectAllFileTypes = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_INSPECT_ALL_FILE_TYPES), m_htiHiddenPrefs);
			m_htiCryptTCPPaddingLength = m_ctrlTreeOptions.InsertItem(GetResString(IDS_OBFUS_PADDING_LEN), TREEOPTSCTRLIMG_EDIT,TREEOPTSCTRLIMG_EDIT, m_htiHiddenPrefs);
			m_ctrlTreeOptions.AddEditBox(m_htiCryptTCPPaddingLength, RUNTIME_CLASS(CNumTreeOptionsEdit));
			m_htiTxtEditor = m_ctrlTreeOptions.InsertItem(GetResString(IDS_TXTEDITOR), TREEOPTSCTRLIMG_EDIT,TREEOPTSCTRLIMG_EDIT, m_htiHiddenPrefs);
			m_ctrlTreeOptions.AddFileEditBox(m_htiTxtEditor, RUNTIME_CLASS(CTreeOptionsEdit), RUNTIME_CLASS(CTreeOptionsBrowseButton));
			m_htiWebServerTweaks = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_PW_WS), iImgWebServer, m_htiHiddenPrefs);
			m_htiWebFileUploadSizeLimitMB = m_ctrlTreeOptions.InsertItem(GetResString(IDS_WEB_FILE_UL_LIMIT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiWebServerTweaks);
			m_ctrlTreeOptions.AddEditBox(m_htiWebFileUploadSizeLimitMB, RUNTIME_CLASS(CNumTreeOptionsEdit));
			m_htiAllowAdminHiLevFunc = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_WEB_ADMIN_HILEV), m_htiWebServerTweaks, m_bAllowAdminHiLevFunc);
			m_htiAllowedRemoteAccessIPs = m_ctrlTreeOptions.InsertItem(GetResString(IDS_WEB_ALLOWED_RA_IPS), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiWebServerTweaks);
			m_ctrlTreeOptions.AddEditBox(m_htiAllowedRemoteAccessIPs, RUNTIME_CLASS(CTreeOptionsEdit));
			m_htiLogTweaks = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_LOG_TWEAKS), iImgLog, m_htiHiddenPrefs);
			m_htiMaxLogFileSize = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MAX_LOG_FILE_SIZE), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiLogTweaks);
			m_ctrlTreeOptions.AddEditBox(m_htiMaxLogFileSize, RUNTIME_CLASS(CNumTreeOptionsEdit));
			m_htiLogError = m_ctrlTreeOptions.InsertItem(GetResString(IDS_LOG_ERROR_COLOR), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiLogTweaks);
			m_ctrlTreeOptions.AddColorSelector(m_htiLogError, RUNTIME_CLASS(CTreeOptionsBrowseButton));
			m_htiLogWarning = m_ctrlTreeOptions.InsertItem(GetResString(IDS_LOG_WARNING_COLOR), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiLogTweaks);
			m_ctrlTreeOptions.AddColorSelector(m_htiLogWarning, RUNTIME_CLASS(CTreeOptionsBrowseButton));
			m_htiLogSuccess = m_ctrlTreeOptions.InsertItem(GetResString(IDS_LOG_SUCCESS_COLOR), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiLogTweaks);
			m_ctrlTreeOptions.AddColorSelector(m_htiLogSuccess, RUNTIME_CLASS(CTreeOptionsBrowseButton));
			m_htiAdjustNTFSDaylightFileTime = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ADJUST_NTFS_DLFT), m_htiHiddenPrefs, m_bAdjustNTFSDaylightFileTime);
			m_htiKeepUnavailableFixedSharedDirs = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_KEEP_UNAVL_FIXED_SHARED_DIRS), m_htiHiddenPrefs, m_bKeepUnavailableFixedSharedDirs);
			m_htiUPnPExt =  m_ctrlTreeOptions.InsertGroup(GetResString(IDS_UPNP), iImgUPnP, m_htiHiddenPrefs);
			m_htiUPnPDisableLib = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DISABLE_MINIUPNP), m_htiUPnPExt, m_bIsMinilibImplDisabled);
			m_htiUPnPDisableServ = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DISABLE_UPNPWIN), m_htiUPnPExt, m_bIsWinServImplDisabled);
			m_htiInternetSecurityZone = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_SECURITYZONE), TREEOPTSCTRLIMG_EDIT, m_htiHiddenPrefs);
			for (int i = 0; i < ARRSIZE(_asNetSecurityZones); i++)
				m_ctrlTreeOptions.InsertRadioButton(_asNetSecurityZones[i], m_htiInternetSecurityZone, m_iInternetSecurityZone == i);
		} // ..
		// Tux: Feature: Hidden prefs [end]

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
		// Tux: Feature: Hidden prefs [start]
		if (thePrefs.IsExtControlsEnabled()) { // Tux: Improvement: hide more extended controls
			// m_ctrlTreeOptions.Expand(m_htiHiddenPrefs, TVE_EXPAND);
			m_ctrlTreeOptions.Expand(m_htiMiniMule, TVE_EXPAND);
			m_ctrlTreeOptions.Expand(m_htiDisplayTweaks, TVE_EXPAND);
			m_ctrlTreeOptions.Expand(m_htiPreviewTweaks, TVE_EXPAND);
			m_ctrlTreeOptions.Expand(m_htiStatisticsTweaks, TVE_EXPAND);
			m_ctrlTreeOptions.Expand(m_htiWebServerTweaks, TVE_EXPAND);
			m_ctrlTreeOptions.Expand(m_htiLogTweaks, TVE_EXPAND);
			m_ctrlTreeOptions.Expand(m_htiUPnPExt, TVE_EXPAND);
			m_ctrlTreeOptions.Expand(m_htiInternetSecurityZone, TVE_EXPAND);
		} // ..
		// Tux: Feature: Hidden prefs [end]
        m_ctrlTreeOptions.SendMessage(WM_VSCROLL, SB_TOP);
        m_bInitializedTreeOpts = true;
		
		// Tux: Improvement: Bold options [start]
		m_ctrlTreeOptions.SetItemState(m_htiTCPGroup, TVIS_BOLD, TVIS_BOLD);
		m_ctrlTreeOptions.SetItemState(m_htiVerboseGroup, TVIS_BOLD, TVIS_BOLD);
		m_ctrlTreeOptions.SetItemState(m_htiCommit, TVIS_BOLD, TVIS_BOLD);
		m_ctrlTreeOptions.SetItemState(m_htiCheckDiskspace, TVIS_BOLD, TVIS_BOLD);
		m_ctrlTreeOptions.SetItemState(m_htiDynUp, TVIS_BOLD, TVIS_BOLD);
		m_ctrlTreeOptions.SetItemState(m_htiDynUpPingToleranceGroup, TVIS_BOLD, TVIS_BOLD);
		m_ctrlTreeOptions.SetItemState(m_htiExtractMetaData, TVIS_BOLD, TVIS_BOLD);
		m_ctrlTreeOptions.SetItemState(m_htiUPnP, TVIS_BOLD, TVIS_BOLD);
		m_ctrlTreeOptions.SetItemState(m_htiShareeMule, TVIS_BOLD, TVIS_BOLD);
		// Tux: Feature: Hidden prefs [start]
		if (thePrefs.IsExtControlsEnabled()) { // Tux: Improvement: hide more extended controls
			m_ctrlTreeOptions.SetItemState(m_htiHiddenPrefs, TVIS_BOLD, TVIS_BOLD);
			m_ctrlTreeOptions.SetItemState(m_htiMiniMule, TVIS_BOLD, TVIS_BOLD);
			m_ctrlTreeOptions.SetItemState(m_htiDisplayTweaks, TVIS_BOLD, TVIS_BOLD);
			m_ctrlTreeOptions.SetItemState(m_htiPreviewTweaks, TVIS_BOLD, TVIS_BOLD);
			m_ctrlTreeOptions.SetItemState(m_htiStatisticsTweaks, TVIS_BOLD, TVIS_BOLD);
			m_ctrlTreeOptions.SetItemState(m_htiWebServerTweaks, TVIS_BOLD, TVIS_BOLD);
			m_ctrlTreeOptions.SetItemState(m_htiLogTweaks, TVIS_BOLD, TVIS_BOLD);
			m_ctrlTreeOptions.SetItemState(m_htiUPnPExt, TVIS_BOLD, TVIS_BOLD);
			m_ctrlTreeOptions.SetItemState(m_htiInternetSecurityZone, TVIS_BOLD, TVIS_BOLD);
		} // ..
		// Tux: Feature: Hidden prefs [end]
		// Tux: Improvement: Bold options [end]
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

	/////////////////////////////////////////////////////////////////////////////
	// Miscellaneous group
	//
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiAutoTakeEd2kLinks, m_bAutoTakeEd2kLinks);
	//DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiCreditSystem, m_bCreditSystem);	// Tux: Newbie security: remove UseCreditSystem
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
	// Tux: Feature: SUQWT [start]
	if (m_htiLogSUQWT)              DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiLogSUQWT, m_bLogSUQWT);
	if (m_htiLogSUQWT)              m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogSUQWT, m_bVerbose);
	// Tux: Feature: SUQWT [end]
	// Tux: Improvement: log Kad security events [start]
	if (m_htiLogKadSecurityEvents)	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiLogKadSecurityEvents, m_bLogKadSecurityEvents);
	if (m_htiLogKadSecurityEvents)	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogKadSecurityEvents, m_bVerbose);
	// Tux: Improvement: log Kad security events [end]
	// Tux: Improvement: log AICH events [start]
	if (m_htiLogAICHEvents)			DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiLogAICHEvents, m_bLogAICHEvents);
	if (m_htiLogAICHEvents)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogAICHEvents, m_bVerbose);
	// Tux: Improvement: log AICH events [end]
	// Tux: Improvement: log SLS events [start]
	if (m_htiLogSLSEvents)			DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiLogSLSEvents, m_bLogSLSEvents);
	if (m_htiLogSLSEvents)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogSLSEvents, m_bVerbose);
	// Tux: Improvement: log SLS events [end]
	// Tux: Improvement: log CryptLayer events [start]
	if (m_htiLogCryptEvents)		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiLogCryptEvents, m_bLogCryptEvents);
	if (m_htiLogCryptEvents)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogCryptEvents, m_bVerbose);
	// Tux: Improvement: log CryptLayer events [end]
	// Tux: Feature: Client Analyzer [start]
	if (m_htiLogAnalyzerEvents)		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiLogAnalyzerEvents, m_bLogAnalyzerEvents);
	if (m_htiLogAnalyzerEvents)     m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogAnalyzerEvents, m_bVerbose);
	// Tux: Feature: Client Analyzer [end]

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

	/////////////////////////////////////////////////////////////////////////////
	// eMule Shared User
	//
	DDX_TreeRadio(pDX, IDC_EXT_OPTS, m_htiShareeMule, m_iShareeMule);
	m_ctrlTreeOptions.SetRadioButtonEnable(m_htiShareeMulePublicUser, thePrefs.GetWindowsVersion() >= _WINVER_VISTA_);

	// Tux: Feature: Hidden prefs [start]
	if (thePrefs.m_bExtControls) {	// Tux: Improvement: hide more extended controls
		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiRemove2bin, m_bRemove2bin);
		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiShowCopyEd2kLinkCmd, m_bShowCopyEd2kLinkCmd);
		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiHighresTimer, m_bHighresTimer);
		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiIconflashOnNewMessage, m_bIconflashOnNewMessage);
		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiBeepOnError, m_bBeepOnError);
		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiRestoreLastMainWndDlg, m_bRestoreLastMainWndDlg);
		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiRestoreLastLogPane, m_bRestoreLastLogPane);
		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiPreviewCopiedArchives, m_bPreviewCopiedArchives);
		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiPreviewOnIconDblClk, m_bPreviewOnIconDblClk);
		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiExtraPreviewWithMenu, m_bExtraPreviewWithMenu);
		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiShowActiveDownloadsBold, m_bShowActiveDownloadsBold);
		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiRTLWindowsLayout, m_bRTLWindowsLayout);
		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiPreferRestrictedOverUser, m_bPreferRestrictedOverUser);
		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiTrustEveryHash, m_bTrustEveryHash);
		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiPartiallyPurgeOldKnownFiles, m_bPartiallyPurgeOldKnownFiles);
		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiShowVerticalHourMarkers, m_bShowVerticalHourMarkers);
		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiDontRecreateStatGraphsOnResize, m_bDontRecreateStatGraphsOnResize);
		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiAllowAdminHiLevFunc, m_bAllowAdminHiLevFunc);
		DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiServerUDPPort, m_nServerUDPPort);
		DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiPreviewSmallBlocks, m_iPreviewSmallBlocks);
		DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiStraightWindowStyles, m_iStraightWindowStyles);
		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiUseSystemFontForMainControls, m_bUseSystemFontForMainControls);
#ifdef HAVE_WIN7_SDK_H
		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiShowUpDownIconInTaskbar, m_bShowUpDownIconInTaskbar);
#endif
		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiUpdateQueueList, m_bUpdateQueueList);
		DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiMaxChatHistory, m_iMaxChatHistory);
		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiMessageFromValidSourcesOnly, m_bMessageFromValidSourcesOnly);
		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiInspectAllFileTypes, m_bInspectAllFileTypes);
		DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiCryptTCPPaddingLength, m_iCryptTCPPaddingLength);
		DDV_MinMaxInt(pDX, m_iCryptTCPPaddingLength, 10, 254);
		DDX_TreeFileEdit(pDX, IDC_EXT_OPTS, m_htiTxtEditor, m_strTxtEditor);
		DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiWebFileUploadSizeLimitMB, m_iWebFileUploadSizeLimitMB);
		DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiAllowedRemoteAccessIPs, m_sAllowedRemoteAccessIPs);
		DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiMaxLogFileSize, m_uMaxLogFileSize);
		DDX_TreeColor(pDX, IDC_EXT_OPTS, m_htiLogError, m_crLogError);
		DDX_TreeColor(pDX, IDC_EXT_OPTS, m_htiLogWarning, m_crLogWarning);
		DDX_TreeColor(pDX, IDC_EXT_OPTS, m_htiLogSuccess, m_crLogSuccess);

		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiMiniMuleAutoClose, m_bMiniMuleAutoClose);
		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiAdjustNTFSDaylightFileTime, m_bAdjustNTFSDaylightFileTime);
		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiKeepUnavailableFixedSharedDirs, m_bKeepUnavailableFixedSharedDirs);
		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiUPnPDisableLib, m_bIsMinilibImplDisabled);
		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiUPnPDisableServ, m_bIsWinServImplDisabled);
		DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiMiniMuleTransparency, m_iMiniMuleTransparency);
		DDX_TreeRadio(pDX, IDC_EXT_OPTS, m_htiInternetSecurityZone, m_iInternetSecurityZone);
		
#ifdef HAVE_WIN7_SDK_H
		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiShowUpDownIconInTaskbar, thePrefs.GetWindowsVersion() >= _WINVER_7_);
#endif
	} // ..
	// Tux: Feature: Hidden prefs [end]
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
		m_bLogSUQWT = thePrefs.m_bLogSUQWT;	// Tux: Feature: SUQWT
		m_bLogKadSecurityEvents = thePrefs.m_bLogKadSecurityEvents;	// Tux: Improvement: log Kad security events
		m_bLogAICHEvents = thePrefs.m_bLogAICHEvents;	// Tux: Improvement: log AICH events
		m_bLogSLSEvents = thePrefs.m_bLogSLSEvents;	// Tux: Improvement: log SLS events
		m_bLogCryptEvents = thePrefs.m_bLogCryptEvents;	// Tux: Improvement: log CryptLayer events
		m_bLogAnalyzerEvents = thePrefs.m_bLogAnalyzerEvents;	// Tux: Feature: Client Analyzer
		m_iLogLevel = 5 - thePrefs.m_byLogLevel;
	}
	m_bLog2Disk = thePrefs.log2disk;
	// m_bCreditSystem = thePrefs.m_bCreditSystem;	// Tux: Newbie security: remove UseCreditSystem
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

	// Tux: Feature: Hidden prefs [start]
	m_bRemove2bin = thePrefs.m_bRemove2bin;
	m_bShowCopyEd2kLinkCmd = thePrefs.m_bShowCopyEd2kLinkCmd;
	m_bHighresTimer = thePrefs.m_bHighresTimer;
	m_bIconflashOnNewMessage = thePrefs.m_bIconflashOnNewMessage;
	m_bBeepOnError = thePrefs.beepOnError;
	m_nServerUDPPort = thePrefs.nServerUDPPort;
	m_bRestoreLastMainWndDlg = thePrefs.m_bRestoreLastMainWndDlg;
	m_bRestoreLastLogPane = thePrefs.m_bRestoreLastLogPane;
	m_iPreviewSmallBlocks = thePrefs.m_iPreviewSmallBlocks;
	m_bPreviewCopiedArchives = thePrefs.m_bPreviewCopiedArchives;
	m_bPreviewOnIconDblClk = thePrefs.m_bPreviewOnIconDblClk;
	m_bExtraPreviewWithMenu = thePrefs.GetExtraPreviewWithMenu();
	m_bShowActiveDownloadsBold = thePrefs.m_bShowActiveDownloadsBold;
	m_bRTLWindowsLayout = thePrefs.m_bRTLWindowsLayout;
	m_iStraightWindowStyles = thePrefs.m_iStraightWindowStyles;
	m_bUseSystemFontForMainControls = thePrefs.GetUseSystemFontForMainControls();
#ifdef HAVE_WIN7_SDK_H
	m_bShowUpDownIconInTaskbar = thePrefs.IsShowUpDownIconInTaskbar();
#endif
	m_bUpdateQueueList = thePrefs.GetUpdateQueueList();
	m_iMaxChatHistory = thePrefs.m_iMaxChatHistory;
	m_bMessageFromValidSourcesOnly = thePrefs.MsgOnlySecure();
	m_bPreferRestrictedOverUser = thePrefs.m_bPreferRestrictedOverUser;
	m_bTrustEveryHash = thePrefs.m_bTrustEveryHash;
	m_bPartiallyPurgeOldKnownFiles = thePrefs.m_bPartiallyPurgeOldKnownFiles;
	m_bShowVerticalHourMarkers = thePrefs.m_bShowVerticalHourMarkers;
	m_bDontRecreateStatGraphsOnResize = thePrefs.dontRecreateGraphs;
	m_bInspectAllFileTypes = thePrefs.m_bInspectAllFileTypes;
	m_iCryptTCPPaddingLength = thePrefs.GetCryptTCPPaddingLength();
	m_strTxtEditor = thePrefs.m_strTxtEditor;
	m_dwServerKeepAliveTimeout = thePrefs.m_dwServerKeepAliveTimeout;
	m_iWebFileUploadSizeLimitMB = thePrefs.m_iWebFileUploadSizeLimitMB;
	m_bAllowAdminHiLevFunc = thePrefs.m_bAllowAdminHiLevFunc;
	m_sAllowedRemoteAccessIPs = _T("");
	m_uMaxLogFileSize = thePrefs.uMaxLogFileSize;
	m_crLogError = thePrefs.m_crLogError;
	m_crLogWarning = thePrefs.m_crLogWarning;
	m_crLogSuccess = thePrefs.m_crLogSuccess;
	m_iInternetSecurityZone = 0;
	m_bMiniMuleAutoClose = theApp.GetProfileInt(_T("eMule"), _T("MiniMuleAutoClose"), 0)!=0;
	m_iMiniMuleTransparency = theApp.GetProfileInt(_T("eMule"), _T("MiniMuleTransparency"), 0);
	m_bAdjustNTFSDaylightFileTime = theApp.GetProfileInt(_T("eMule"), _T("AdjustNTFSDaylightFileTime"), 0)!=0;
	m_bKeepUnavailableFixedSharedDirs = thePrefs.m_bKeepUnavailableFixedSharedDirs;
	m_bIsMinilibImplDisabled = thePrefs.m_bIsMinilibImplDisabled;
	m_bIsWinServImplDisabled = thePrefs.m_bIsWinServImplDisabled;
	// Tux: Feature: Hidden prefs [end]
	
	m_ctrlTreeOptions.SetImageListColorFlags(theApp.m_iDfltImageListColorFlags);
    CPropertyPage::OnInitDialog();
	InitWindowStyles(this);
	m_ctrlTreeOptions.SetItemHeight(m_ctrlTreeOptions.GetItemHeight() + 2);

	// Tux: Feature: IntelliFlush: removed code block

	m_iQueueSize = thePrefs.m_iQueueSize;
	// m_ctlQueueSize.SetRange(20, 100, TRUE);
	m_ctlQueueSize.SetRange(10, 300, TRUE);	// Tux: others: changed queue size
	m_ctlQueueSize.SetPos(m_iQueueSize/100);
	m_ctlQueueSize.SetTicFreq(10);
	m_ctlQueueSize.SetPageSize(10);
	
	// Tux: Feature: Infinite Queue [start]
	if (thePrefs.infiniteQueue) {
		GetDlgItem(IDC_QUEUESIZE_STATIC)->EnableWindow(FALSE);
		GetDlgItem(IDC_QUEUESIZE)->EnableWindow(FALSE);
	}
	// Tux: Feature: Infinite Queue [end]

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
	// Tux: LiteMule: Remove Scheduler
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

	// Tux: Feature: Hidden prefs [start]
	// if we change the CopyED2KLink parameter, we should also recreate the DownloadListCtrl
	// menu, so the command will be grayed out/enabled accordingly
	if (thePrefs.m_bShowCopyEd2kLinkCmd != m_bShowCopyEd2kLinkCmd)
		theApp.emuledlg->transferwnd->GetDownloadList()->CreateMenues();
	// Tux: Feature: Hidden prefs [end]

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
		thePrefs.m_bLogUlDlEvents = m_bLogUlDlEvents;
		thePrefs.m_bLogSUQWT = m_bLogSUQWT;	// Tux: Feature: SUQWT
		thePrefs.m_bLogKadSecurityEvents = m_bLogKadSecurityEvents;	// Tux: Improvement: log Kad security events
		thePrefs.m_bLogAICHEvents = m_bLogAICHEvents;	// Tux: Improvement: log AICH events
		thePrefs.m_bLogSLSEvents = m_bLogSLSEvents;	// Tux: Improvement: log SLS events
		thePrefs.m_bLogCryptEvents = m_bLogCryptEvents;	// Tux: Improvement: log CryptLayer events
		thePrefs.m_bLogAnalyzerEvents = m_bLogAnalyzerEvents;	// Tux: Feature: Client Analyzer
		thePrefs.m_byLogLevel = 5 - m_iLogLevel;

		thePrefs.m_bVerbose = m_bVerbose; // store after related options were stored!
	}

	// thePrefs.m_bCreditSystem = m_bCreditSystem;	// Tux: Newbie security: remove UseCreditSystem
	thePrefs.m_iCommitFiles = m_iCommitFiles;
	thePrefs.m_iExtractMetaData = m_iExtractMetaData;
	thePrefs.filterLANIPs = m_bFilterLANIPs;
	// thePrefs.m_iFileBufferSize = m_iFileBufferSize;	// Tux: Feature: IntelliFlush
	thePrefs.m_iQueueSize = m_iQueueSize;
	if (thePrefs.m_bExtControls != m_bExtControls) {
		thePrefs.m_bExtControls = m_bExtControls;
		theApp.emuledlg->transferwnd->GetDownloadList()->CreateMenues();
		theApp.emuledlg->searchwnd->CreateMenus();
		theApp.emuledlg->sharedfileswnd->sharedfilesctrl.CreateMenues();
		bReopenPrefs = true;	// Tux: Improvement: repaint prefs wnd if needed :-)
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

	// Tux: Feature: Hidden prefs [start]
	thePrefs.m_bRemove2bin = m_bRemove2bin;
	thePrefs.m_bShowCopyEd2kLinkCmd = m_bShowCopyEd2kLinkCmd;
	thePrefs.m_bHighresTimer = m_bHighresTimer;
	thePrefs.m_bIconflashOnNewMessage = m_bIconflashOnNewMessage;
	thePrefs.beepOnError = m_bBeepOnError;
	thePrefs.nServerUDPPort = (uint16)m_nServerUDPPort;
	thePrefs.m_bRestoreLastMainWndDlg = m_bRestoreLastMainWndDlg;
	thePrefs.m_bRestoreLastLogPane = m_bRestoreLastLogPane;
	thePrefs.m_iPreviewSmallBlocks = m_iPreviewSmallBlocks;
	thePrefs.m_bPreviewCopiedArchives = m_bPreviewCopiedArchives;
	thePrefs.m_bPreviewOnIconDblClk = m_bPreviewOnIconDblClk;
	thePrefs.m_bExtraPreviewWithMenu = m_bExtraPreviewWithMenu;
	thePrefs.m_bShowActiveDownloadsBold = m_bShowActiveDownloadsBold;
	thePrefs.m_bRTLWindowsLayout = m_bRTLWindowsLayout;
	thePrefs.m_iStraightWindowStyles = m_iStraightWindowStyles;
	thePrefs.m_bUseSystemFontForMainControls = m_bUseSystemFontForMainControls;
#ifdef HAVE_WIN7_SDK_H
	thePrefs.m_bShowUpDownIconInTaskbar = m_bShowUpDownIconInTaskbar;
#endif
	thePrefs.m_bupdatequeuelist = m_bUpdateQueueList;
	thePrefs.m_iMaxChatHistory = m_iMaxChatHistory;
	thePrefs.msgsecure = m_bMessageFromValidSourcesOnly;
	thePrefs.m_bPreferRestrictedOverUser = m_bPreferRestrictedOverUser;
	thePrefs.m_bTrustEveryHash = m_bTrustEveryHash;
	thePrefs.m_bPartiallyPurgeOldKnownFiles = m_bPartiallyPurgeOldKnownFiles;
	thePrefs.m_bShowVerticalHourMarkers = m_bShowVerticalHourMarkers;
	thePrefs.dontRecreateGraphs = m_bDontRecreateStatGraphsOnResize;
	thePrefs.m_bInspectAllFileTypes = m_bInspectAllFileTypes;
	thePrefs.m_byCryptTCPPaddingLength = (uint8)m_iCryptTCPPaddingLength;
	thePrefs.m_strTxtEditor = m_strTxtEditor;
	thePrefs.m_dwServerKeepAliveTimeout = m_dwServerKeepAliveTimeout;
	thePrefs.m_iWebFileUploadSizeLimitMB = m_iWebFileUploadSizeLimitMB;
	thePrefs.m_bAllowAdminHiLevFunc = m_bAllowAdminHiLevFunc;
	thePrefs.uMaxLogFileSize = m_uMaxLogFileSize;
	thePrefs.m_crLogError = m_crLogError;
	thePrefs.m_crLogWarning = m_crLogWarning;
	thePrefs.m_crLogSuccess = m_crLogSuccess;

	thePrefs.m_aAllowedRemoteAccessIPs.RemoveAll();
	int iPos = 0;
	CString strIP = m_sAllowedRemoteAccessIPs.Tokenize(_T(";"), iPos);
	while (!strIP.IsEmpty())
	{
		u_long nIP = inet_addr(CStringA(strIP));
		if (nIP != INADDR_ANY && nIP != INADDR_NONE)
			thePrefs.m_aAllowedRemoteAccessIPs.Add(nIP);
		strIP = m_sAllowedRemoteAccessIPs.Tokenize(_T(";"), iPos);
	}

	theApp.WriteProfileString(_T("eMule"), _T("InternetSecurityZone"), _asNetSecurityZones[m_iInternetSecurityZone]);
	theApp.WriteProfileInt(_T("eMule"), _T("MiniMuleAutoClose"), m_bMiniMuleAutoClose ? 1 : 0);
	theApp.WriteProfileInt(_T("eMule"), _T("MiniMuleTransparency"), m_iMiniMuleTransparency);
	theApp.WriteProfileInt(_T("eMule"), _T("AdjustNTFSDaylightFileTime"), m_bAdjustNTFSDaylightFileTime ? 1 : 0);
	thePrefs.m_bKeepUnavailableFixedSharedDirs = m_bKeepUnavailableFixedSharedDirs;
	thePrefs.m_bIsMinilibImplDisabled = m_bIsMinilibImplDisabled;
	thePrefs.m_bIsWinServImplDisabled = m_bIsWinServImplDisabled;
	// Tux: Feature: Hidden prefs [end]

	if (thePrefs.GetEnableVerboseOptions())
	{
	    theApp.emuledlg->serverwnd->ToggleDebugWindow();
		theApp.emuledlg->serverwnd->UpdateLogTabSelection();
	}
	theApp.downloadqueue->CheckDiskspace();

	// Tux: Improvement: repaint prefs wnd if needed :-) [start]
	if (bReopenPrefs) {
		bReopenPrefs = false;
		theApp.emuledlg->preferenceswnd->CloseDialog();
		theApp.emuledlg->preferenceswnd->OpenDialog();
	}
	// Tux: Improvement: repaint prefs wnd if needed :-) [end]

	SetModified(FALSE);
	return CPropertyPage::OnApply();
}

void CPPgTweaks::OnHScroll(UINT /*nSBCode*/, UINT /*nPos*/, CScrollBar* pScrollBar) 
{
	// Tux: Feature: IntelliFlush: removed code block
	// ... else ...
	if (pScrollBar->GetSafeHwnd() == m_ctlQueueSize.m_hWnd)
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
		// if (m_htiCreditSystem) m_ctrlTreeOptions.SetItemText(m_htiCreditSystem, GetResString(IDS_USECREDITSYSTEM));	// Tux: Newbie security: remove UseCreditSystem
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
		if (m_htiLogSUQWT)     m_ctrlTreeOptions.SetItemText(m_htiLogSUQWT, GetResString(IDS_LOG_SUQWT_EVENTS));	// Tux: Feature: SUQWT
		if (m_htiLogKadSecurityEvents) m_ctrlTreeOptions.SetItemText(m_htiLogKadSecurityEvents, GetResString(IDS_LOG_KADSEC_EVENTS));	// Tux: Improvement: log Kad security events
		if (m_htiLogAICHEvents) m_ctrlTreeOptions.SetItemText(m_htiLogAICHEvents, GetResString(IDS_LOG_AICH));	// Tux: Improvement: log AICH events
		if (m_htiLogSLSEvents) m_ctrlTreeOptions.SetItemText(m_htiLogSLSEvents, GetResString(IDS_LOG_SLS));	// Tux: Improvement: log SLS events
		if (m_htiLogCryptEvents) m_ctrlTreeOptions.SetItemText(m_htiLogCryptEvents, GetResString(IDS_LOG_CRYPT));	// Tux: Improvement: log CryptLayer events
		if (m_htiLogAnalyzerEvents) m_ctrlTreeOptions.SetItemText(m_htiLogAnalyzerEvents, GetResString(IDS_LOG_ANALYZER_EVENTS));	// Tux: Feature: Client Analyzer
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

        CString temp;
		// Tux: Feature: IntelliFlush: removed two lines
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
	m_htiLogSUQWT = NULL;	// Tux: Feature: SUQWT
	m_htiLogKadSecurityEvents = NULL;	// Tux: Improvement: log Kad security events
	m_htiLogAICHEvents = NULL;	// Tux: Improvement: log AICH events
	m_htiLogSLSEvents = NULL;	// Tux: Improvement: log SLS events
	m_htiLogCryptEvents = NULL;	// Tux: Improvement: log CryptLayer events
	m_htiLogAnalyzerEvents = NULL;	// Tux: Feature: Client Analyzer
//	m_htiCreditSystem = NULL;
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

	// Tux: Feature: Hidden prefs [start]
	m_htiHiddenPrefs = NULL;
	m_htiRemove2bin = NULL;
	m_htiShowCopyEd2kLinkCmd = NULL;
	m_htiHighresTimer = NULL;
	m_htiIconflashOnNewMessage = NULL;
	m_htiBeepOnError = NULL;
	m_htiServerUDPPort = NULL;
	m_htiRestoreLastMainWndDlg = NULL;
	m_htiRestoreLastLogPane = NULL;
	m_htiDisplayTweaks = NULL;
	m_htiMiniMule = NULL;
	m_htiPreviewTweaks = NULL;
	m_htiPreviewSmallBlocks = NULL;
	m_htiPreviewCopiedArchives = NULL;
	m_htiPreviewOnIconDblClk = NULL;
	m_htiExtraPreviewWithMenu = NULL;
	m_htiShowActiveDownloadsBold = NULL;
	m_htiRTLWindowsLayout = NULL;
	m_htiStraightWindowStyles = NULL;
	m_htiUseSystemFontForMainControls = NULL;
#ifdef HAVE_WIN7_SDK_H
	m_htiShowUpDownIconInTaskbar = NULL;
#endif
	m_htiUpdateQueueList = NULL;
	m_htiMaxChatHistory = NULL;
	m_htiMessageFromValidSourcesOnly = NULL;
	m_htiPreferRestrictedOverUser = NULL;
	m_htiTrustEveryHash = NULL;
	m_htiPartiallyPurgeOldKnownFiles = NULL;
	m_htiStatisticsTweaks = NULL;
	m_htiShowVerticalHourMarkers = NULL;
	m_htiDontRecreateStatGraphsOnResize = NULL;
	m_htiInspectAllFileTypes = NULL;
	m_htiCryptTCPPaddingLength = NULL;
	m_htiTxtEditor = NULL;
	m_htiWebServerTweaks = NULL;
	m_htiWebFileUploadSizeLimitMB = NULL;
	m_htiAllowAdminHiLevFunc = NULL;
	m_htiAllowedRemoteAccessIPs = NULL;
	m_htiLogTweaks = NULL;
	m_htiLogError = NULL;
	m_htiLogWarning = NULL;
	m_htiLogSuccess = NULL;
	m_htiInternetSecurityZone = NULL;
	m_htiMiniMuleAutoClose = NULL;
	m_htiMiniMuleTransparency = NULL;
	m_htiAdjustNTFSDaylightFileTime = NULL;
	m_htiKeepUnavailableFixedSharedDirs = NULL;
	m_htiUPnPExt = NULL;
	m_htiUPnPDisableLib = NULL;
	m_htiUPnPDisableServ = NULL;
	m_htiMaxLogFileSize = NULL;
	// Tux: Feature: Hidden prefs [end]

	bReopenPrefs = false;	// Tux: Improvement: repaint prefs wnd if needed :-)

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
				if (m_htiDebugSourceExchange)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiDebugSourceExchange, bCheck);
				if (m_htiLogBannedClients)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogBannedClients, bCheck);
				if (m_htiLogRatingDescReceived)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogRatingDescReceived, bCheck);
				if (m_htiLogSecureIdent)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogSecureIdent, bCheck);
				if (m_htiLogFilteredIPs)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogFilteredIPs, bCheck);
				if (m_htiLogFileSaving)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogFileSaving, bCheck);
				if (m_htiLogA4AF)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogA4AF, bCheck);
				if (m_htiLogUlDlEvents)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogUlDlEvents, bCheck);
				if (m_htiLogSUQWT)              m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogSUQWT, bCheck);	// Tux: Feature: SUQWT
				if (m_htiLogKadSecurityEvents)	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogKadSecurityEvents, bCheck);	// Tux: Improvement: log Kad security events
				if (m_htiLogAICHEvents)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogAICHEvents, bCheck);	// Tux: Improvement: log AICH events
				if (m_htiLogSLSEvents)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogSLSEvents, bCheck);		// Tux: Improvement: log SLS events
				if (m_htiLogCryptEvents)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogCryptEvents, bCheck);	// Tux: Improvement: log CryptLayer events
				if (m_htiLogAnalyzerEvents)     m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogAnalyzerEvents, bCheck);	// Tux: Feature: Client Analyzer
			}
		}
		else if (   (m_htiShareeMuleMultiUser  && pton->hItem == m_htiShareeMuleMultiUser)
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

// Tux: LiteMule: Remove Help: removed code block

BOOL CPPgTweaks::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// Tux: LiteMule: Remove Help: removed code block
	return __super::OnCommand(wParam, lParam);
}

// Tux: LiteMule: Remove Help: removed code block

void CPPgTweaks::OnBnClickedOpenprefini()
{
	ShellOpenFile(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("preferences.ini"));
}