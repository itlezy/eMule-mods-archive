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
	bool m_bLogDrop; //Xman Xtreme Downloadmanager
	bool m_bLogpartmismatch; //Xman Log part/size-mismatch
	bool m_bLogUlDlEvents;
	// ==> CreditSystems [EastShare/ MorphXT] - Stulle
	/*
	bool m_bCreditSystem;
	*/
	// <== CreditSystems [EastShare/ MorphXT] - Stulle
	bool m_bLog2Disk;
	bool m_bDebug2Disk;
	bool m_bDateFileNameLog; // Date File Name Log [AndCycle] - Stulle
	int m_iCommitFiles;
	bool m_bFilterLANIPs;
	bool m_bExtControls;
	UINT m_uServerKeepAliveTimeout;
	bool m_bSparsePartFiles;
	bool m_bFullAlloc;
	bool m_bCheckDiskspace;
	float m_fMinFreeDiskSpaceMB;
	CString m_sYourHostname;
	// ==> Improved ICS-Firewall support [MoNKi] - Max
	/*
	bool m_bFirewallStartup;
	*/
	// <== Improved ICS-Firewall support [MoNKi] - Max
	int m_iLogLevel;
	//Xman
	/*
    bool m_bDynUpEnabled;
    int m_iDynUpMinUpload;
    int m_iDynUpPingTolerance;
    int m_iDynUpPingToleranceMilliseconds;
    int m_iDynUpRadioPingTolerance;
    int m_iDynUpGoingUpDivider;
    int m_iDynUpGoingDownDivider;
    int m_iDynUpNumberOfPings;
    bool m_bA4AFSaveCpu;
	*/
	//Xman end
	bool m_bAutoArchDisable;
	int m_iExtractMetaData;
	// ==> UPnP support [MoNKi] - leuk_he
	/*
	bool m_bIsUPnPEnabled; //zz_fly :: add UPnP option in Tweaks
	bool m_bCloseUPnPOnExit;
	bool m_bSkipWANIPSetup;
	bool m_bSkipWANPPPSetup;
	*/
	// <== UPnP support [MoNKi] - leuk_he
	int m_iShareeMule;
	int m_iCryptTCPPaddingLength; //Xman Added PaddingLength to Extended preferences
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
	HTREEITEM m_htiLogDrop; //Xman Xtreme Downloadmanager
	HTREEITEM m_htiLogpartmismtach; //Xman Log part/size-mismatch
	HTREEITEM m_htiLogUlDlEvents;
	// ==> CreditSystems [EastShare/ MorphXT] - Stulle
	/*
	HTREEITEM m_htiCreditSystem;
	*/
	// <== CreditSystems [EastShare/ MorphXT] - Stulle
	HTREEITEM m_htiLog2Disk;
	HTREEITEM m_htiDebug2Disk;
	HTREEITEM m_htiDateFileNameLog; // Date File Name Log [AndCycle] - Stulle
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
	// ==> Improved ICS-Firewall support [MoNKi] - Max
	/*
	HTREEITEM m_htiFirewallStartup;
	*/
	// <== Improved ICS-Firewall support [MoNKi] - Max
	HTREEITEM m_htiLogLevel;
	//Xman
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
    HTREEITEM m_htiA4AFSaveCpu;
	*/
	//Xman end

	// ==> UPnP support [MoNKi] - leuk_he verbose log
	bool m_bLogUPnP;
	HTREEITEM m_htiLogUPnP;
	// <== UPnP support [MoNKi] - leuk_he 
	HTREEITEM m_htiExtractMetaData;
	HTREEITEM m_htiExtractMetaDataNever;
	HTREEITEM m_htiExtractMetaDataID3Lib;
	HTREEITEM m_htiAutoArch;

	// ==> UPnP support [MoNKi] - leuk_he
	/*
	//UPnP chooser
	HTREEITEM m_htiUseACATUPnPNextStart;
	bool m_iUseACATUPnPNextStart;

	//ACAT UPnP
	HTREEITEM m_htiUPnPNat;
	HTREEITEM m_htiUPnPTryRandom;
	bool m_iUPnPNat;
	bool m_iUPnPTryRandom;

	HTREEITEM m_htiUPnP;
	HTREEITEM m_htiIsUPnPEnabled; //zz_fly :: add UPnP option in Tweaks
	HTREEITEM m_htiCloseUPnPPorts;
	HTREEITEM m_htiSkipWANIPSetup;
	HTREEITEM m_htiSkipWANPPPSetup;

	//zz_fly :: Rebind UPnP on IP-change
	HTREEITEM m_htiUPnPRebindOnIPChange;
	bool m_iUPnPRebindOnIPChange;
	//zz_fly :: end
	*/
	// <== UPnP support [MoNKi] - leuk_he

	HTREEITEM m_htiShareeMule;
	HTREEITEM m_htiShareeMuleMultiUser;
	HTREEITEM m_htiShareeMulePublicUser;
	HTREEITEM m_htiShareeMuleOldStyle;
	//HTREEITEM m_htiExtractMetaDataMediaDet;
	HTREEITEM m_htiResolveShellLinks;

	HTREEITEM m_htiCryptTCPPaddingLength; //Xman Added PaddingLength to Extended preferences

	//zz_fly
	//zz_fly :: known2 buffer
	bool m_bKnown2Buffer; 
	HTREEITEM m_htiKnown2Buffer;
	//zz_fly :: end

	//zz_fly :: known2 split
	bool m_bKnown2Split; 
	HTREEITEM m_htiKnown2Split;
	//zz_fly :: end

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
};
