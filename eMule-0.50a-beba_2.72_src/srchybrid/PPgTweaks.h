#pragma once
#include "TreeOptionsCtrlEx.h"

class CPPgTweaks : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgTweaks)

public:
	CPPgTweaks();
	virtual ~CPPgTweaks();

// Dialog Data
	enum { IDD = IDD_PPG_TWEAKS };

	void Localize(void);

protected:
	// UINT m_iFileBufferSize;	// Tux: Feature: IntelliFlush
	UINT m_iQueueSize;
	int m_iMaxConnPerFive;
	int m_iMaxHalfOpen;
	bool m_bConditionalTCPAccept;
	bool m_bAutoTakeEd2kLinks;
	bool m_bVerbose;
	bool m_bDebugSourceExchange;
	bool m_bLogBannedClients;
	bool m_bLogRatingDescReceived;
	bool m_bLogSecureIdent;
	bool m_bLogFilteredIPs;
	bool m_bLogFileSaving;
	bool m_bLogA4AF;
	bool m_bLogUlDlEvents;
	bool m_bLogSUQWT;	// Tux: Feature: SUQWT
	bool m_bLogKadSecurityEvents;	// Tux: Improvement: log Kad security events
	bool m_bLogAICHEvents;	// Tux: Improvement: log AICH events
	bool m_bLogSLSEvents;	// Tux: Improvement: log SLS events
	bool m_bLogCryptEvents;	// Tux: Improvement: log CryptLayer events
	bool m_bLogAnalyzerEvents;	// Tux: Feature: Client Analyzer
	bool m_bCreditSystem;
	bool m_bLog2Disk;
	bool m_bDebug2Disk;
	int m_iCommitFiles;
	bool m_bFilterLANIPs;
	bool m_bExtControls;
	UINT m_uServerKeepAliveTimeout;
	bool m_bSparsePartFiles;
	bool m_bFullAlloc;
	bool m_bCheckDiskspace;
	float m_fMinFreeDiskSpaceMB;
	CString m_sYourHostname;
	bool m_bFirewallStartup;
	int m_iLogLevel;
    bool m_bDynUpEnabled;
    int m_iDynUpMinUpload;
    int m_iDynUpPingTolerance;
    int m_iDynUpPingToleranceMilliseconds;
    int m_iDynUpRadioPingTolerance;
    int m_iDynUpGoingUpDivider;
    int m_iDynUpGoingDownDivider;
    int m_iDynUpNumberOfPings;
    bool m_bA4AFSaveCpu;
	bool m_bAutoArchDisable;
	int m_iExtractMetaData;
	bool m_bCloseUPnPOnExit;
	bool m_bSkipWANIPSetup;
	bool m_bSkipWANPPPSetup;
	int m_iShareeMule;
	bool bShowedWarning;
	bool m_bResolveShellLinks;


	CSliderCtrl m_ctlFileBuffSize;
	CSliderCtrl m_ctlQueueSize;
    CTreeOptionsCtrlEx m_ctrlTreeOptions;
	bool m_bInitializedTreeOpts;
	HTREEITEM m_htiTCPGroup;
	HTREEITEM m_htiMaxCon5Sec;
	HTREEITEM m_htiMaxHalfOpen;
	HTREEITEM m_htiConditionalTCPAccept;
	HTREEITEM m_htiAutoTakeEd2kLinks;
	HTREEITEM m_htiVerboseGroup;
	HTREEITEM m_htiVerbose;
	HTREEITEM m_htiDebugSourceExchange;
	HTREEITEM m_htiLogBannedClients;
	HTREEITEM m_htiLogRatingDescReceived;
	HTREEITEM m_htiLogSecureIdent;
	HTREEITEM m_htiLogFilteredIPs;
	HTREEITEM m_htiLogFileSaving;
    HTREEITEM m_htiLogA4AF;
	HTREEITEM m_htiLogUlDlEvents;
	HTREEITEM m_htiLogSUQWT;	// Tux: Feature: SUQWT
	HTREEITEM m_htiLogKadSecurityEvents;	// Tux: Improvement: log Kad security events
	HTREEITEM m_htiLogAICHEvents;	// Tux: Improvement: log AICH events
	HTREEITEM m_htiLogSLSEvents;	// Tux: Improvement: log SLS events
	HTREEITEM m_htiLogCryptEvents;	// Tux: Improvement: log CryptLayer events
	HTREEITEM m_htiLogAnalyzerEvents;	// Tux: Feature: Client Analyzer
//	HTREEITEM m_htiCreditSystem;
	HTREEITEM m_htiLog2Disk;
	HTREEITEM m_htiDebug2Disk;
	HTREEITEM m_htiCommit;
	HTREEITEM m_htiCommitNever;
	HTREEITEM m_htiCommitOnShutdown;
	HTREEITEM m_htiCommitAlways;
	HTREEITEM m_htiFilterLANIPs;
	HTREEITEM m_htiExtControls;
	HTREEITEM m_htiServerKeepAliveTimeout;
	HTREEITEM m_htiSparsePartFiles;
	HTREEITEM m_htiFullAlloc;
	HTREEITEM m_htiCheckDiskspace;
	HTREEITEM m_htiMinFreeDiskSpace;
	HTREEITEM m_htiYourHostname;
	HTREEITEM m_htiFirewallStartup;
	HTREEITEM m_htiLogLevel;
    HTREEITEM m_htiDynUp;
	HTREEITEM m_htiDynUpEnabled;
    HTREEITEM m_htiDynUpMinUpload;
    HTREEITEM m_htiDynUpPingTolerance;
    HTREEITEM m_htiDynUpPingToleranceMilliseconds;
    HTREEITEM m_htiDynUpPingToleranceGroup;
    HTREEITEM m_htiDynUpRadioPingTolerance;
    HTREEITEM m_htiDynUpRadioPingToleranceMilliseconds;
    HTREEITEM m_htiDynUpGoingUpDivider;
    HTREEITEM m_htiDynUpGoingDownDivider;
    HTREEITEM m_htiDynUpNumberOfPings;
    HTREEITEM m_htiA4AFSaveCpu;
	HTREEITEM m_htiExtractMetaData;
	HTREEITEM m_htiExtractMetaDataNever;
	HTREEITEM m_htiExtractMetaDataID3Lib;
	HTREEITEM m_htiAutoArch;
	HTREEITEM m_htiUPnP;
	HTREEITEM m_htiCloseUPnPPorts;
	HTREEITEM m_htiSkipWANIPSetup;
	HTREEITEM m_htiSkipWANPPPSetup;
	HTREEITEM m_htiShareeMule;
	HTREEITEM m_htiShareeMuleMultiUser;
	HTREEITEM m_htiShareeMulePublicUser;
	HTREEITEM m_htiShareeMuleOldStyle;
	//HTREEITEM m_htiExtractMetaDataMediaDet;
	HTREEITEM m_htiResolveShellLinks;

	// Tux: Feature: Hidden prefs [start]
	bool m_bRemove2bin;
	bool m_bShowCopyEd2kLinkCmd;
	bool m_bHighresTimer;
	bool m_bIconflashOnNewMessage;
	bool m_bBeepOnError;
	int m_nServerUDPPort;
	bool m_bRestoreLastMainWndDlg;
	bool m_bRestoreLastLogPane;
	int m_iPreviewSmallBlocks;
	bool m_bPreviewCopiedArchives;
	bool m_bPreviewOnIconDblClk;
	bool m_bExtraPreviewWithMenu;
	bool m_bShowActiveDownloadsBold;
	bool m_bRTLWindowsLayout;
	int m_iStraightWindowStyles;
	bool m_bUseSystemFontForMainControls;
#ifdef HAVE_WIN7_SDK_H
	bool m_bShowUpDownIconInTaskbar;
#endif
	bool m_bUpdateQueueList;
	int m_iMaxChatHistory;
	bool m_bMessageFromValidSourcesOnly;
	bool m_bPreferRestrictedOverUser;
	bool m_bTrustEveryHash;
	bool m_bUsePeerCache;
	bool m_bPartiallyPurgeOldKnownFiles;
	bool m_bShowVerticalHourMarkers;
	bool m_bDontRecreateStatGraphsOnResize;
	bool m_bInspectAllFileTypes;
	int m_iCryptTCPPaddingLength;
	CString m_strTxtEditor;
	int m_dwServerKeepAliveTimeout;
	int m_iWebFileUploadSizeLimitMB;
	bool m_bAllowAdminHiLevFunc;
	CString m_sAllowedRemoteAccessIPs;
	int m_uMaxLogFileSize;
	COLORREF m_crLogError;
	COLORREF m_crLogWarning;
	COLORREF m_crLogSuccess;
	int m_iInternetSecurityZone;
	bool m_bMiniMuleAutoClose;
	int m_iMiniMuleTransparency;
	bool m_bAdjustNTFSDaylightFileTime;
	bool m_bKeepUnavailableFixedSharedDirs;
	bool m_bIsMinilibImplDisabled;
	bool m_bIsWinServImplDisabled;

	HTREEITEM m_htiHiddenPrefs;
	HTREEITEM m_htiRemove2bin;
	HTREEITEM m_htiShowCopyEd2kLinkCmd;
	HTREEITEM m_htiHighresTimer;
	HTREEITEM m_htiIconflashOnNewMessage;
	HTREEITEM m_htiBeepOnError;
	HTREEITEM m_htiServerUDPPort;
	HTREEITEM m_htiRestoreLastMainWndDlg;
	HTREEITEM m_htiRestoreLastLogPane;
	HTREEITEM m_htiMiniMule;
	HTREEITEM m_htiEnableMiniMule;
	HTREEITEM m_htiDisplayTweaks;
	HTREEITEM m_htiPreviewTweaks;
	HTREEITEM m_htiPreviewSmallBlocks;
	HTREEITEM m_htiPreviewCopiedArchives;
	HTREEITEM m_htiPreviewOnIconDblClk;
	HTREEITEM m_htiExtraPreviewWithMenu;
	HTREEITEM m_htiShowActiveDownloadsBold;
	HTREEITEM m_htiRTLWindowsLayout;
	HTREEITEM m_htiStraightWindowStyles;
	HTREEITEM m_htiUseSystemFontForMainControls;
#ifdef HAVE_WIN7_SDK_H
	HTREEITEM m_htiShowUpDownIconInTaskbar;
#endif
	HTREEITEM m_htiUpdateQueueList;
	HTREEITEM m_htiMaxChatHistory;
	HTREEITEM m_htiMessageFromValidSourcesOnly;
	HTREEITEM m_htiPreferRestrictedOverUser;
	HTREEITEM m_htiTrustEveryHash;
	HTREEITEM m_htiPartiallyPurgeOldKnownFiles;
	HTREEITEM m_htiStatisticsTweaks;
	HTREEITEM m_htiShowVerticalHourMarkers;
	HTREEITEM m_htiDontRecreateStatGraphsOnResize;
	HTREEITEM m_htiInspectAllFileTypes;
	HTREEITEM m_htiCryptTCPPaddingLength;
	HTREEITEM m_htiTxtEditor;
	HTREEITEM m_htiWebServerTweaks;
	HTREEITEM m_htiWebFileUploadSizeLimitMB;
	HTREEITEM m_htiAllowAdminHiLevFunc;
	HTREEITEM m_htiAllowedRemoteAccessIPs;
	HTREEITEM m_htiLogTweaks;
	HTREEITEM m_htiLogError;
	HTREEITEM m_htiLogWarning;
	HTREEITEM m_htiLogSuccess;
	HTREEITEM m_htiInternetSecurityZone;
	HTREEITEM m_htiMiniMuleAutoClose;
	HTREEITEM m_htiMiniMuleTransparency;
	HTREEITEM m_htiAdjustNTFSDaylightFileTime;
	HTREEITEM m_htiKeepUnavailableFixedSharedDirs;
	HTREEITEM m_htiUPnPExt;
	HTREEITEM m_htiUPnPDisableLib;
	HTREEITEM m_htiUPnPDisableServ;
	HTREEITEM m_htiMaxLogFileSize;
	// Tux: Feature: Hidden prefs [end]

	bool bReopenPrefs;	// Tux: Improvement: repaint prefs wnd if needed :-)

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnKillActive();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam);
	// Tux: LiteMule: Remove Help
        afx_msg void OnBnClickedOpenprefini();
};
