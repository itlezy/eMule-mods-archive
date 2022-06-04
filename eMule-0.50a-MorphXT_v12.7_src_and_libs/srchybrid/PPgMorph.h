#pragma once
#include "TreeOptionsCtrlEx.h"
// CPPgMorph dialog

//MORPH START leuk_he tooltipped
/*
class CPPgMorph : public CPropertyPage
*/
class CPPgMorph : public CPPgtooltipped  
//MORPH END  leuk_he tooltipped
{
	DECLARE_DYNAMIC(CPPgMorph)

public:
	CPPgMorph();
	virtual ~CPPgMorph();

// Dialog Data
	enum { IDD = IDD_PPG_MORPH };
protected:

	bool m_bSUCLog;
	bool m_bUSSLimit; // EastShare - Added by TAHO, USS limit
	int m_iSUCHigh;
	int m_iSUCLow;
	int m_iSUCPitch;
	int m_iSUCDrift;
	bool m_bUSSLog;
	bool m_bUSSUDP; //MORPH - Added by SiRoB, USS UDP preferency
	int m_sPingDataSize; //MORPH leuk_he ICMP ping datasize <> 0 setting (short but no DX macro for that) 
	int m_iUSSPingLimit; // EastShare - Added by TAHO, USS limit
    int m_iUSSPingTolerance;
    int m_iUSSGoingUpDivider;
    int m_iUSSGoingDownDivider;
    int m_iUSSNumberOfPings;
	// ==> [MoNKi: -USS initial TTL-] - Stulle
	int			m_iUSSTTL;
	// <== [MoNKi: -USS initial TTL-] - Stulle
	int m_iMinUpload;
	bool m_bEnableDownloadInRed; //MORPH - Added by IceCream, show download in red
	bool m_bEnableDownloadInBold; //MORPH - Added by SiRoB, show download in Bold
	bool m_bShowClientPercentage;
	//MORPH START - Added by Stulle, Global Source Limit
	bool m_bGlobalHL;
	int m_iGlobalHL;
	//MORPH END   - Added by Stulle, Global Source Limit
        //MORPH START - Added by schnulli900, filter clients with failed downloads [Xman]
	bool m_bFilterClientFailedDown; 
        //MORPH END   - Added by schnulli900, filter clients with failed downloads [Xman]
	bool m_bEnableAntiLeecher; //MORPH - Added by IceCream, enable Anti-leecher
	bool m_bEnableAntiCreditHack; //MORPH - Added by IceCream, enable Anti-CreditHack
	bool m_bInfiniteQueue;	//Morph - added by AndCycle, SLUGFILLER: infiniteQueue
	bool m_bDontRemoveSpareTrickleSlot; //Morph - added by AndCycle, Dont Remove Spare Trickle Slot
	bool m_bUseDownloadOverhead; //// MORPH leuk_he include download overhead in upload stats
    int  m_iCompressLevel; // MORPH compresslevel
	bool  m_bUseCompression; // Use compression. 
	bool m_bFunnyNick; //MORPH - Added by SiRoB, Optionnal funnynick display
	bool m_bClientQueueProgressBar; // MORPH - Added by Commander, ClientQueueProgressBar

	//MORPH START - Added by SiRoB, Datarate Average Time Management
	int m_iDownloadDataRateAverageTime;
	int m_iUploadDataRateAverageTime;
	//MORPH END   - Added by SiRoB, Datarate Average Time Management
	
	//MORPH START - Added by SiRoB, Upload Splitting Class
	int m_iGlobalDataRateFriend;
	int m_iMaxGlobalDataRateFriend;
	int m_iMaxClientDataRateFriend;
	int m_iGlobalDataRatePowerShare;
	int m_iMaxGlobalDataRatePowerShare;
	int m_iMaxClientDataRatePowerShare;
	int m_iMaxClientDataRate;
	//MORPH END  - Added by SiRoB, Upload Splitting Class
		// ==> Slot Limit - Stulle
	int m_iSlotLimiter;
	int m_iSlotLimitNum;
	// <== Slot Limit - Stulle
	int	m_iDynUpMode;//MORPH - Added by Yun.SF3, Auto DynUp changing
	int	m_iMaxConnectionsSwitchBorder;//MORPH - Added by Yun.SF3, Auto DynUp changing
	//MORPH START - Added by SiRoB, khaos::categorymod+
	bool m_bShowCatNames;
public: //MORPH leuk_he:run as ntservice v1.. 
	bool m_bSelectCat;
protected: //MORPH leuk_he:run as ntservice v1.. 
	bool m_bUseActiveCat;
	bool m_bAutoSetResOrder;
	bool m_bShowA4AFDebugOutput;
	bool m_bSmartA4AFSwapping;
	int m_iAdvA4AFMode;
	bool m_bSmallFileDLPush;
	int m_iResumeFileInNewCat;
	bool m_bUseAutoCat;
	bool m_bAddRemovedInc;
	bool m_bUseSLS;
	// khaos::accuratetimerem+
	int m_iTimeRemainingMode;
	// khaos::accuratetimerem-
	// MORPH START leuk_he disable catcolor
	bool m_bDisableCatColors;
	// MORPH END   leuk_he disable catcolor
    
	//MORPH END - Added by SiRoB, khaos::categorymod+
	bool m_bUseICS; //MORPH - Added by SIRoB, ICS Optional
	CString m_sBrokenURLs; //MORPH - Added by WiZaRd, Fix broken HTTP downloads
	bool m_bHighProcess; //MORPH - Added by IceCream, high process priority

public: //MORPH leuk_he:run as ntservice v1.. 
	CTreeOptionsCtrlEx m_ctrlTreeOptions;
protected: //MORPH leuk_he:run as ntservice v1.. 
	bool m_bInitializedTreeOpts;
	HTREEITEM m_htiDM;
	HTREEITEM m_htiUM;
	HTREEITEM m_htiDYNUP;
	HTREEITEM m_htiDynUpOFF;
	HTREEITEM m_htiDynUpSUC;
	HTREEITEM m_htiDynUpUSS;
	HTREEITEM m_htiDynUpAutoSwitching;//MORPH - Added by Yun.SF3, Auto DynUp changing
	HTREEITEM m_htiMaxConnectionsSwitchBorder;//MORPH - Added by Yun.SF3, Auto DynUp changing
	HTREEITEM m_htiSUCLog;
	HTREEITEM m_htiSUCHigh;
	HTREEITEM m_htiSUCLow;
	HTREEITEM m_htiSUCPitch;
	HTREEITEM m_htiSUCDrift;
	HTREEITEM m_htiUSSLog;
	HTREEITEM m_htiUSSUDP; //MORPH - Added by SiRoB, USS UDP preferency
	HTREEITEM m_htiUSSLimit; // EastShare - Added by TAHO, USS limit
	HTREEITEM m_htiUSSPingLimit; // EastShare - Added by TAHO, USS limit
    HTREEITEM m_htiUSSPingTolerance;
    HTREEITEM m_htiUSSGoingUpDivider;
    HTREEITEM m_htiUSSGoingDownDivider;
    HTREEITEM m_htiUSSNumberOfPings;
	// ==> [MoNKi: -USS initial TTL-] - Stulle
	HTREEITEM	m_htiUSSTTL;
	// <== [MoNKi: -USS initial TTL-] - Stulle
	HTREEITEM m_htiMinUpload;
	HTREEITEM m_htiEnableDownloadInRed; //MORPH - Added by IceCream, show download in red
	HTREEITEM m_htiEnableDownloadInBold; //MORPH - Added by SiRoB, show download in Bold
	HTREEITEM m_htiShowClientPercentage;
	HTREEITEM m_htiPingDataSize;   //MORPH leuk_he ICMP ping datasize <> 0 setting
	//MORPH START - Added by Stulle, Global Source Limit
	HTREEITEM m_htiGlobalHlGroup;
	HTREEITEM m_htiGlobalHL;
	HTREEITEM m_htiGlobalHlLimit;
	//MORPH END   - Added by Stulle, Global Source Limit
        //MORPH START - Added by schnulli900, filter clients with failed downloads [Xman]
	HTREEITEM m_htiFilterClientFailedDown; 
        //MORPH END   - Added by schnulli900, filter clients with failed downloads [Xman]
	HTREEITEM m_htiEnableAntiLeecher; //MORPH - Added by IceCream, enable Anti-leecher
	HTREEITEM m_htiEnableAntiCreditHack; //MORPH - Added by IceCream, enable Anti-CreditHack
	HTREEITEM m_htiInfiniteQueue;	//Morph - added by AndCycle, SLUGFILLER: infiniteQueue
	HTREEITEM m_htiDontRemoveSpareTrickleSlot; //Morph - added by AndCycle, Dont Remove Spare Trickle Slot
	HTREEITEM m_htiUseDownloadOverhead; //// MORPH leuk_he include download overhead in upload stats
	HTREEITEM m_htiCompressLevel; //Morph - compresslevel
	HTREEITEM m_htiUseCompression; //Morph - compresslevel
	HTREEITEM m_htiDisplayFunnyNick;//MORPH - Added by SiRoB, Optionnal funnynick display
	HTREEITEM m_htiClientQueueProgressBar; //MORPH - Added by Commander, ClientQueueProgressBar

	//MORPH START - Added by SiRoB, Datarate Average Time Management
	HTREEITEM m_htiDownloadDataRateAverageTime;
	HTREEITEM m_htiUploadDataRateAverageTime;
	//MORPH END   - Added by SiRoB, Datarate Average Time Management
	
	//MORPH START - Added by SiRoB, Upload Splitting Class
	HTREEITEM m_htiFriend;
	HTREEITEM m_htiGlobalDataRateFriend;
	HTREEITEM m_htiMaxGlobalDataRateFriend;
	HTREEITEM m_htiMaxClientDataRateFriend;
	HTREEITEM m_htiPowerShare;
	HTREEITEM m_htiGlobalDataRatePowerShare;
	HTREEITEM m_htiMaxGlobalDataRatePowerShare;
	HTREEITEM m_htiMaxClientDataRatePowerShare;
	HTREEITEM m_htiMaxClientDataRate;
	//MORPH END   - Added by SiRoB, Upload Splitting Class
	HTREEITEM m_htiUpDisplay; //MORPH - Added by Commander, UM -> Display

	HTREEITEM m_htiSCC;
	HTREEITEM m_htiSAC;
	HTREEITEM m_htiDisp;
	HTREEITEM m_htiDlSecu;
	HTREEITEM m_htiUpSecu;

	//MORPH START - Added by SiRoB, khaos::categorymod+
	HTREEITEM m_htiShowCatNames;
public: //MORPH leuk_he:run as ntservice v1.. 
	HTREEITEM m_htiSelectCat;
protected: //MORPH leuk_he:run as ntservice v1.. 
	HTREEITEM m_htiUseActiveCat;
	HTREEITEM m_htiAutoSetResOrder;
	HTREEITEM m_htiShowA4AFDebugOutput;
	HTREEITEM m_htiSmartA4AFSwapping;
	HTREEITEM m_htiAdvA4AFMode;
	HTREEITEM m_htiBalanceSources;
	HTREEITEM m_htiStackSources;
	HTREEITEM m_htiDisableAdvA4AF;
	HTREEITEM m_htiSmallFileDLPush;
	HTREEITEM m_htiResumeFileInNewCat;
	HTREEITEM m_htiUseAutoCat;
	HTREEITEM m_htiAddRemovedInc;
	HTREEITEM m_htiUseSLS;
	// khaos::accuratetimerem+
	HTREEITEM m_htiTimeRemainingMode;
	HTREEITEM m_htiTimeRemBoth;
	HTREEITEM m_htiTimeRemAverage;
	HTREEITEM m_htiTimeRemRealTime;
	// khaos::accuratetimerem-
	// MORPH START leuk_he disable catcolor
	HTREEITEM m_htiDisableCatColors;
	// MORPH END   leuk_he disable catcolor


	//MORPH END - Added by SiRoB, khaos::categorymod+
	HTREEITEM m_htiUseICS; //MORPH - Added by SIRoB, ICS Optional
	HTREEITEM m_htiBrokenURLs; //MORPH - Added by WiZaRd, Fix broken HTTP downloads
	HTREEITEM m_htiHighProcess; //MORPH - Added by IceCream, high process priority

	// Mighty Knife: Report hashing files, Log friendlist activities
	bool      m_bReportHashingFiles;
	HTREEITEM m_htiReportHashingFiles;
	bool	  m_bLogFriendlistActivities;
	HTREEITEM m_htiLogFriendlistActivities;
	// [end] Mighty Knife

	// Mighty Knife: Static server handling
	bool	  m_bDontRemoveStaticServers;
	HTREEITEM m_htiDontRemoveStaticServers;
	// [end] Mighty Knife
		// ==> Slot Limit - Stulle
	HTREEITEM m_htiSlotLimitGroup;
	HTREEITEM m_htiSlotLimitNone;
	HTREEITEM m_htiSlotLimitThree;
	HTREEITEM m_htiSlotLimitNumB;
	HTREEITEM m_htiSlotLimitNum;
	// <== Slot Limit - Stulle

	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	void Localize(void);	
	void LoadSettings(void);
	virtual BOOL OnApply();
	virtual BOOL OnInitDialog();
	virtual BOOL OnKillActive();
	afx_msg void OnSettingsChange()			{ SetModified(); }
};
//MORPH START leuk_he ask on exit

class CAskExit : public CPPgtooltippedDialog
{
	DECLARE_DYNAMIC(CAskExit)
public:
	CAskExit();   // standard constructor
	virtual ~CAskExit();
	virtual BOOL  OnInitDialog() ;
// Dialog Data
	enum { IDD = IDD_ASKEXIT };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedYes();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedYesservice();
	afx_msg void OnBnClickedNominimize();
};
//MORPH END leuk_he ask on exit
