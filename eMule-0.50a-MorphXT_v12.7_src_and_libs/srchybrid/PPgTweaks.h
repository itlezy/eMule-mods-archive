#pragma once
#include "TreeOptionsCtrlEx.h"

// MORPH START leuk_he tooltipped
/*
class CPPgTweaks : public CPropertyPage
*/
class CPPgTweaks : public CPPgtooltipped  
// MORPH END leuk_he tooltipped
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
	DWORD m_dwBindAddr; //MORPH leuk_he bindaddr
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
	//MORPH START - Added by SiRoB, WebCache 1.2f
	bool m_bLogICHEvents;//JP log ICH events
	//MORPH END   - Added by SiRoB, WebCache 1.2f
	bool m_bCreditSystem;
	bool m_bLog2Disk;
	bool m_bDebug2Disk;
	bool m_bDateFileNameLog;//Morph - added by AndCycle, Date File Name Log
	int m_iCommitFiles;
	bool m_bFilterLANIPs;
	bool m_bExtControls;
	UINT m_uServerKeepAliveTimeout;
	bool m_bSparsePartFiles;
	bool m_bFullAlloc;
	bool m_bCheckDiskspace;
	float m_fMinFreeDiskSpaceMB;
	CString m_sYourHostname;
	// Removed by MoNKi [MoNKi: -Improved ICS-Firewall support-]
	/* Moved to PPgEmulespana
	bool m_bFirewallStartup;
	*/
	int m_iLogLevel;
	//MORPH START - Removed by Stulle, Removed dupe USS settings
	/*
    bool m_bDynUpEnabled;
    int m_iDynUpMinUpload;
    int m_iDynUpPingTolerance;
    int m_iDynUpPingToleranceMilliseconds;
    int m_iDynUpRadioPingTolerance;
    int m_iDynUpGoingUpDivider;
    int m_iDynUpGoingDownDivider;
    int m_iDynUpNumberOfPings;
	*/
	//MORPH END   - Removed by Stulle, Removed dupe USS settings
    bool m_bA4AFSaveCpu;
	bool m_bAutoArchDisable;
	int m_iExtractMetaData;
	int m_iShareeMule;
	bool bShowedWarning;
	bool m_bResolveShellLinks;

	// MORPH START leuk_he Advanced official preferences.
	bool bMiniMuleAutoClose;
	int iMiniMuleTransparency;
	bool bCreateCrashDump;
	bool bCheckComctl32 ;
	bool bCheckShell32;
	bool bIgnoreInstances;
	CString sNotifierMailEncryptCertName;
	CString  sMediaInfo_MediaInfoDllPath;
	bool bMediaInfo_RIFF;
	bool bMediaInfo_ID3LIB;
#ifdef HAVE_QEDIT_H
	bool m_bMediaInfo_MediaDet;
#endif//HAVE_QEDIT_H
	bool m_bMediaInfo_RM;
#ifdef HAVE_WMSDK_H
	bool m_bMediaInfo_WM;
#endif//HAVE_WMSDK_H
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
	int iServerUDPPort; // really a unsigned int 16
	bool m_bRemoveFilesToBin;
    bool m_bHighresTimer;
	bool m_bTrustEveryHash;
    int m_iInspectAllFileTypes;
    int  m_umaxmsgsessions;
    bool m_bPreferRestrictedOverUser;
	bool m_bUseUserSortedServerList;
    int m_iWebFileUploadSizeLimitMB;
    CString m_sAllowedIPs;
	int m_iDebugSearchResultDetailLevel;
	int m_iCryptTCPPaddingLength ;
	bool m_bAdjustNTFSDaylightFileTime; //neo offcial pref II
	CString m_strDateTimeFormat;
	CString m_strDateTimeFormat4Log;
	CString m_strDateTimeFormat4List;
	COLORREF m_crLogError;
	COLORREF m_crLogWarning;
	COLORREF m_crLogSuccess;
	COLORREF m_crLogUSC;
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
public: //MORPH leuk_he:run as ntservice v1.. 
	bool m_bBeeper;
protected: //MORPH leuk_he:run as ntservice v1.. 
	bool m_bMsgOnlySec;
	bool m_bDisablePeerCache;
	bool m_bExtraPreviewWithMenu;
	bool m_bShowUpDownIconInTaskbar;
	bool m_bKeepUnavailableFixedSharedDirs;
	bool m_bForceSpeedsToKB;

    // continue extra official preferences....
	HTREEITEM m_hti_advanced;

	HTREEITEM m_hti_MiniMule;
    HTREEITEM m_hti_bMiniMuleAutoClose;
	HTREEITEM m_hti_iMiniMuleTransparency;

	HTREEITEM m_htiMediInfo;
	HTREEITEM m_hti_InspectAllFileTypes;
	HTREEITEM m_hti_sMediaInfo_MediaInfoDllPath;
	HTREEITEM m_hti_bMediaInfo_RIFF;
	HTREEITEM m_hti_bMediaInfo_ID3LIB;
#ifdef HAVE_QEDIT_H
	HTREEITEM m_hti_MediaInfo_MediaDet;
#endif//HAVE_QEDIT_H
	HTREEITEM m_hti_MediaInfo_RM;
#ifdef HAVE_WMSDK_H
	HTREEITEM m_hti_MediaInfo_WM;
#endif//HAVE_WMSDK_H

	HTREEITEM m_hti_Display;
	HTREEITEM m_hti_m_bRestoreLastMainWndDlg;
	HTREEITEM m_hti_m_bRestoreLastLogPane;
	HTREEITEM m_hti_m_iStraightWindowStyles;
	HTREEITEM m_hti_m_bRTLWindowsLayout;
	HTREEITEM m_hti_m_iMaxChatHistory;
	HTREEITEM m_hti_maxmsgsessions;
	HTREEITEM m_htidatetimeformat;
	HTREEITEM m_htidatetimeformat4list;
	HTREEITEM m_htiShowVerticalHourMarkers;
	HTREEITEM m_htiReBarToolbar;
	HTREEITEM m_htiIconflashOnNewMessage;
	HTREEITEM m_htiShowCopyEd2kLinkCmd;
	HTREEITEM m_htiUpdateQueue;
	HTREEITEM m_htiRepaint;
	HTREEITEM m_htiExtraPreviewWithMenu;
	HTREEITEM m_htiShowUpDownIconInTaskbar;
	HTREEITEM m_htiForceSpeedsToKB;

	HTREEITEM m_hti_Log;
	HTREEITEM m_hti_iMaxLogBuff;
	HTREEITEM m_hti_m_iLogFileFormat;
	HTREEITEM m_htidatetimeformat4log;
	HTREEITEM m_htiLogError;
	HTREEITEM m_htiLogWarning;
	HTREEITEM m_htiLogSuccess;
	HTREEITEM m_htiLogUSC;

	HTREEITEM m_hti_bCreateCrashDump;
	HTREEITEM m_hti_bCheckComctl32 ;
	HTREEITEM m_hti_bCheckShell32;
	HTREEITEM m_hti_bIgnoreInstances;
	HTREEITEM m_hti_sNotifierMailEncryptCertName;
	HTREEITEM m_hti_m_iPreviewSmallBlocks;
	HTREEITEM m_hti_m_bPreviewCopiedArchives;
	HTREEITEM m_hti_m_bPreviewOnIconDblClk;
	HTREEITEM m_hti_sInternetSecurityZone;
	HTREEITEM m_hti_sTxtEditor;
	HTREEITEM m_hti_iServerUDPPort;
	HTREEITEM m_hti_m_bRemoveFilesToBin;
	HTREEITEM m_hti_HighresTimer;
	HTREEITEM m_hti_TrustEveryHash;
	HTREEITEM m_hti_PreferRestrictedOverUser;
	HTREEITEM m_hti_WebFileUploadSizeLimitMB ;
	HTREEITEM m_hti_AllowedIPs;
	HTREEITEM m_hti_UseUserSortedServerList;
	HTREEITEM m_hti_DebugSearchResultDetailLevel;
	HTREEITEM m_htiCryptTCPPaddingLength;
	HTREEITEM m_htiAdjustNTFSDaylightFileTime;	  //neo offcial pref II
	HTREEITEM m_htidontcompressavi;
	HTREEITEM m_htiICH;
	HTREEITEM m_htiFileBufferTimeLimit;
	HTREEITEM m_htiRearrangeKadSearchKeywords;
public: //MORPH leuk_he:run as ntservice v1.. 
	HTREEITEM m_htiBeeper;
protected: //MORPH leuk_he:run as ntservice v1.. 
	HTREEITEM m_htiMsgOnlySec;
	HTREEITEM m_htiDisablePeerCache;
	HTREEITEM m_htiKeepUnavailableFixedSharedDirs;
	// MORPH END  leuk_he Advanced official preferences. 
	

	CSliderCtrl m_ctlFileBuffSize;
	CSliderCtrl m_ctlQueueSize;
public: //MORPH leuk_he:run as ntservice v1.. 
    CTreeOptionsCtrlEx m_ctrlTreeOptions;
protected: //MORPH leuk_he:run as ntservice v1.. 
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
	//MORPH START - Added by SiRoB, WebCache 1.2f
	HTREEITEM m_htiLogICHEvents; //JP log ICH events
	//MORPH END   - Added by SiRoB, WebCache 1.2f

	HTREEITEM m_htiCreditSystem;
	HTREEITEM m_htiLog2Disk;
	HTREEITEM m_htiDebug2Disk;
	HTREEITEM m_htiDateFileNameLog;//Morph - added by AndCycle, Date File Name Log
	HTREEITEM m_htiCommit;
	HTREEITEM m_htiCommitNever;
	HTREEITEM m_htiCommitOnShutdown;
	HTREEITEM m_htiCommitAlways;
	HTREEITEM m_htiFilterLANIPs;
	HTREEITEM m_htiExtControls;
	HTREEITEM m_htiServerKeepAliveTimeout;
	HTREEITEM  m_htiBindAddr;	//MORPH leuk_he bindaddr
	HTREEITEM m_htiSparsePartFiles;
	HTREEITEM m_htiFullAlloc;
	HTREEITEM m_htiCheckDiskspace;
	HTREEITEM m_htiMinFreeDiskSpace;
	HTREEITEM m_htiYourHostname;
	// Removed by MoNKi [MoNKi: -Improved ICS-Firewall support-]
	/* Moved to PPgEmulespana
	HTREEITEM m_htiFirewallStartup;
	*/
	// End emulEspaña
	HTREEITEM m_htiLogLevel;
	//MORPH START - Removed by Stulle, Removed dupe USS settings
	/*
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
	*/
	//MORPH END   - Removed by Stulle, Removed dupe USS settings
    HTREEITEM m_htiA4AFSaveCpu;
	HTREEITEM m_htiExtractMetaData;
	HTREEITEM m_htiExtractMetaDataNever;
	HTREEITEM m_htiExtractMetaDataID3Lib;
	HTREEITEM m_htiAutoArch;
	HTREEITEM m_htiShareeMule;
	HTREEITEM m_htiShareeMuleMultiUser;
	HTREEITEM m_htiShareeMulePublicUser;
	HTREEITEM m_htiShareeMuleOldStyle;
	//HTREEITEM m_htiExtractMetaDataMediaDet;
	HTREEITEM m_htiResolveShellLinks;

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnKillActive();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnBnClickedOpenprefini();
	// Added by MoNKi [MoNKi: -UPnPNAT Support-]
protected:
	bool			m_bLogUPnP;
	HTREEITEM	m_htiLogUPnP;
	// End -UPnPNAT Support-
};
