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
	UINT m_iFileBufferSize;
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
#ifdef CLIENTANALYZER
	int m_iCreditSystem; //>>> WiZaRd::ClientAnalyzer
	bool m_bLogAnalyzerEvents;
#else
	bool m_bCreditSystem;
#endif
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

// ZZUL-TRA :: Advanced official preferences :: Start
	bool bMiniMuleAutoClose;
	int iMiniMuleTransparency;
	bool bCreateCrashDump;
	bool bIgnoreInstances;
	CString  sMediaInfo_MediaInfoDllPath;
	int iMaxLogBuff;
	int m_iMaxChatHistory;
	int m_iPreviewSmallBlocks;
	bool m_bRestoreLastMainWndDlg;
	bool m_bRestoreLastLogPane;
	bool m_bPreviewCopiedArchives;
	int m_iStraightWindowStyles;
	int m_iLogFileFormat;
	bool m_bRTLWindowsLayout;
	bool m_bPreviewOnIconDblClk;
	CString sInternetSecurityZone;
	CString sTxtEditor;
	int iServerUDPPort; 
	bool m_bRemoveFilesToBin;
    bool m_bHighresTimer;
	bool m_bTrustEveryHash;
    int m_iInspectAllFileTypes;
    int  m_umaxmsgsessions;
    bool m_bPreferRestrictedOverUser;
	bool m_bUseUserSortedServerList;
    int m_iWebFileUploadSizeLimitMB;
    CString m_sAllowedIPs;
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	int m_iDebugSearchResultDetailLevel;
#endif
	int m_iCryptTCPPaddingLength ;
	bool m_bAdjustNTFSDaylightFileTime; 
	CString m_strDateTimeFormat;
	CString m_strDateTimeFormat4Log;
	CString m_strDateTimeFormat4List;
	COLORREF m_crLogError;
	COLORREF m_crLogWarning;
	COLORREF m_crLogSuccess;
	bool m_bShowVerticalHourMarkers;
	bool m_bReBarToolbar;
	bool m_bIconflashOnNewMessage;
	bool m_bShowCopyEd2kLinkCmd;
	bool m_dontcompressavi;
	bool m_ICH;
	int m_iFileBufferTimeLimit;
	bool m_bRearrangeKadSearchKeywords;
	bool m_bUpdateQueue;
	bool m_bRepaint;
	bool m_bBeeper;
	bool m_bMsgOnlySec;
	bool m_bExtraPreviewWithMenu;
	bool m_bShowUpDownIconInTaskbar;
	bool m_bKeepUnavailableFixedSharedDirs;
	bool m_bForceSpeedsToKB;

	HTREEITEM m_hti_advanced;
    HTREEITEM m_hti_bMiniMuleAutoClose;
	HTREEITEM m_hti_iMiniMuleTransparency;
	HTREEITEM m_hti_bCreateCrashDump;
	HTREEITEM m_hti_bIgnoreInstances;
	HTREEITEM m_hti_sMediaInfo_MediaInfoDllPath;
	HTREEITEM m_hti_iMaxLogBuff;
	HTREEITEM m_hti_m_iMaxChatHistory;
	HTREEITEM m_hti_m_iPreviewSmallBlocks;
	HTREEITEM m_hti_m_bRestoreLastMainWndDlg;
	HTREEITEM m_hti_m_bRestoreLastLogPane;
	HTREEITEM m_hti_m_bPreviewCopiedArchives;
	HTREEITEM m_hti_m_iStraightWindowStyles;
	HTREEITEM m_hti_m_iLogFileFormat;
	HTREEITEM m_hti_m_bRTLWindowsLayout;
	HTREEITEM m_hti_m_bPreviewOnIconDblClk;
	HTREEITEM m_hti_sInternetSecurityZone;
	HTREEITEM m_hti_sTxtEditor;
	HTREEITEM m_hti_iServerUDPPort;
	HTREEITEM m_hti_m_bRemoveFilesToBin;
	HTREEITEM m_hti_HighresTimer;
	HTREEITEM m_hti_TrustEveryHash;
	HTREEITEM m_hti_InspectAllFileTypes;
	HTREEITEM m_hti_maxmsgsessions;
	HTREEITEM m_hti_PreferRestrictedOverUser;
	HTREEITEM m_hti_WebFileUploadSizeLimitMB ;
	HTREEITEM m_hti_AllowedIPs;
	HTREEITEM m_hti_UseUserSortedServerList;
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	HTREEITEM m_hti_DebugSearchResultDetailLevel;
#endif
	HTREEITEM m_htiCryptTCPPaddingLength;
	HTREEITEM m_htiAdjustNTFSDaylightFileTime;	  
	HTREEITEM m_htidatetimeformat;
	HTREEITEM m_htidatetimeformat4log;
	HTREEITEM m_htidatetimeformat4list;
	HTREEITEM m_htiLogError;
	HTREEITEM m_htiLogWarning;
	HTREEITEM m_htiLogSuccess;
	HTREEITEM m_htiShowVerticalHourMarkers;
	HTREEITEM m_htiReBarToolbar;
	HTREEITEM m_htiIconflashOnNewMessage;
	HTREEITEM m_htiShowCopyEd2kLinkCmd;
	HTREEITEM m_htidontcompressavi;
	HTREEITEM m_htiICH;
	HTREEITEM m_htiFileBufferTimeLimit;
	HTREEITEM m_htiRearrangeKadSearchKeywords;
	HTREEITEM m_htiUpdateQueue;
	HTREEITEM m_htiRepaint;
	HTREEITEM m_htiBeeper;
	HTREEITEM m_htiMsgOnlySec;
	HTREEITEM m_htiExtraPreviewWithMenu;
	HTREEITEM m_htiShowUpDownIconInTaskbar;
	HTREEITEM m_htiKeepUnavailableFixedSharedDirs;
	HTREEITEM m_htiForceSpeedsToKB;	
// ZZUL-TRA :: Advanced official preferences :: End	

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
	HTREEITEM m_htiCreditSystem;
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

//////////////////////////////////////////////////////////
// ZZUL-TRA :: Start

#ifdef CLIENTANALYZER
//>>> WiZaRd::ClientAnalyzer
	HTREEITEM m_htiCSNone;
	HTREEITEM m_htiCSOffi;
	HTREEITEM m_htiCSAnalyzer;
//<<< WiZaRd::ClientAnalyzer
	HTREEITEM m_htiLogAnalyzerEvents;
#endif
    HTREEITEM m_htiLogEmulator; // ZZUL-TRA :: EmulateOthers
// NEO: QS - [QuickStart]
	bool	m_uQuickStart;
	int		m_iQuickStartTime;
	int		m_iQuickStartTimePerFile;
	int		m_iQuickMaxConperFive;
	int		m_iQuickMaxHalfOpen;
	int		m_iQuickMaxConnections;
	// NEO: QS END
// ==>ZZUL-TRA :: InvisibleMode :: Start
	bool		m_bInvisibleMode;
	CString		m_sInvisibleModeMod;
	CString		m_sInvisibleModeKey;
	// ZZUL-TRA :: InvisibleMode :: End
    bool m_bLogEmulator; // ZZUL-TRA :: EmulateOthers

    bool m_bIsPayBackFirst;//PayBack First
	HTREEITEM m_htiIsPayBackFirst; //PayBack First
	
	int m_iPayBackFirstLimit; //PayBack First
	HTREEITEM m_htiPayBackFirstLimit; //PayBack First

	// ZZUL-TRA :: SysInfo :: Start
	bool m_bGetCPU;
	HTREEITEM m_htiGetCPU; 
	// ZZUL-TRA :: SysInfo :: End

	// ZZUL-TRA :: Global Hardlimit :: Start
	bool m_bIsGlobalHardlimit;
	HTREEITEM m_htiIsGlobalHardlimit; 
	int m_iGlobalHardlimit;
	HTREEITEM m_htiGlobalHardlimit; 
	// ZZUL-TRA :: Global Hardlimit :: End

//>>> shadow2004::IP2Country [EastShare]
	HTREEITEM	m_htiI2CGroup;
	HTREEITEM	m_htiI2C2Letter;
	HTREEITEM	m_htiI2C3Letter;
	HTREEITEM	m_htiI2CFullCaption;
	HTREEITEM	m_htiI2CShowFlag;
	int			m_iI2CCaption;
	bool		m_bI2CShowFlag;
    //HTREEITEM   m_htiIP2CUpdate;
	//bool		m_bIP2CUpdate;
//<<< shadow2004::IP2Country [EastShare]

	HTREEITEM m_htiZZULTRA; //ZZUL-TRA

	bool m_bShowActiveDownloadsBold;
	HTREEITEM m_htiShowActiveDownloadsBold;

	bool m_bShowRuntimeOnTitle;
	HTREEITEM m_htiShowRuntimeOnTitle;

	bool m_bShowDownloadColor;
	HTREEITEM m_htiShowDownloadColor;

	bool m_bShowFileStatusIcons;
	HTREEITEM m_htiShowFileStatusIcons;

	bool m_bDirectoryWatcher;
	HTREEITEM m_htiDirectoryWatcher;

	bool m_bAutoDropSystem;
	HTREEITEM m_htiAutoDropSystem;

	bool m_bFunnyNick;
	HTREEITEM m_htiFunnyNick;

	bool m_bShowSessionDownload;
	HTREEITEM m_htiShowSessionDownload;

// ZZUL-TRA :: PercentBar :: Start
	bool m_bUsePercentBar;
	HTREEITEM m_htiUsePercentBar;
// ZZUL-TRA :: PercentBar :: End

    int m_iDisconnectTime;
    HTREEITEM m_htiDisconnectTime;

//ZZUL-TRA :: PowerShare :: Start
    int m_iPowershareMode; 
	int m_iPowerShareLimit; 
	int m_iPsAmountLimit; 
	HTREEITEM m_htiPowershareMode;
	HTREEITEM m_htiPowershareDisabled;
	HTREEITEM m_htiPowershareActivated;
	HTREEITEM m_htiPowershareAuto;
	HTREEITEM m_htiPowershareLimited;
	HTREEITEM m_htiPowerShareLimit; 
	HTREEITEM m_htiPsAmountLimit; 
//ZZUL-TRA :: PowerShare :: End

// ZZUL-TRA :: InvisibleMode :: Start
	HTREEITEM	m_htiInvisibleMode;
	HTREEITEM	m_htiInvisibleModeMod;
	HTREEITEM	m_htiInvisibleModeKey;
	// ZZUL-TRA :: InvisibleMode :: End
 
    // NEO: QS - [QuickStart]
    HTREEITEM m_htiQuickStartEnable;
	HTREEITEM m_htiQuickStartTime;
	HTREEITEM m_htiQuickStartTimePerFile;
	HTREEITEM m_htiQuickMaxConperFive;
	HTREEITEM m_htiQuickMaxHalfOpen;
	HTREEITEM m_htiQuickMaxConnections;
	// NEO: QS END

//ZZUL-TRA :: End
/////////////////////////////////////////////////////////////////

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnKillActive();

	DECLARE_MESSAGE_MAP()
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedOpenprefini();
};
