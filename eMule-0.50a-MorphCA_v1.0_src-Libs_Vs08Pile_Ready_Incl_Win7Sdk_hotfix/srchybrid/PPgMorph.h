#pragma once
#include "Preferences.h"
#include "TreeOptionsCtrlEx.h"
// CPPgMorph dialog

class CPPgMorph : public CPropertyPage
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

	bool m_bShowModstring; 

	bool m_bInfiniteQueue;	//Morph - added by AndCycle, SLUGFILLER: infiniteQueue
	bool m_bDontRemoveSpareTrickleSlot; //Morph - added by AndCycle, Dont Remove Spare Trickle Slot
	bool m_bUseDownloadOverhead; //// MORPH leuk_he include download overhead in upload stats
    int  m_iCompressLevel; // MORPH compresslevel
	bool  m_bUseCompression; // Use compression. 
	bool m_bFunnyNick; //MORPH - Added by SiRoB, Optionnal funnynick display

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
	bool m_bSelectCat;
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

	CTreeOptionsCtrlEx m_ctrlTreeOptions;
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
       
	HTREEITEM m_htiShowModstring; 

	HTREEITEM m_htiInfiniteQueue;	//Morph - added by AndCycle, SLUGFILLER: infiniteQueue
	HTREEITEM m_htiDontRemoveSpareTrickleSlot; //Morph - added by AndCycle, Dont Remove Spare Trickle Slot
	HTREEITEM m_htiUseDownloadOverhead; //// MORPH leuk_he include download overhead in upload stats
	HTREEITEM m_htiCompressLevel; //Morph - compresslevel
	HTREEITEM m_htiUseCompression; //Morph - compresslevel
	HTREEITEM m_htiDisplayFunnyNick;//MORPH - Added by SiRoB, Optionnal funnynick display

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

	//MORPH START - Added by SiRoB, khaos::categorymod+
	HTREEITEM m_htiShowCatNames;
	HTREEITEM m_htiSelectCat;
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


// Added by MoNKi [ SlugFiller: -lowIdRetry- ]
    HTREEITEM	m_htiLowIdRetry;
	int			m_iLowIdRetry;
	// End SlugFiller

	bool m_bEnablePreferShareAll;//EastShare - PreferShareAll by AndCycle
	HTREEITEM m_htiEnablePreferShareAll;//EastShare - PreferShareAll by AndCycle

	bool m_bIsPayBackFirst;//EastShare - added by AndCycle, Pay Back First
	HTREEITEM m_htiIsPayBackFirst; //EastShare - added by AndCycle, Pay Back First
	
	int m_iPayBackFirstLimit; //MORPH - Added by SiRoB, Pay Back First Tweak
	HTREEITEM m_htiPayBackFirstLimit; //MORPH - Added by SiRoB, Pay Back First Tweak

	bool m_bOnlyDownloadCompleteFiles;//EastShare - Added by AndCycle, Only download complete files v2.1 (shadow)
	HTREEITEM m_htiOnlyDownloadCompleteFiles;//EastShare - Added by AndCycle, Only download complete files v2.1 (shadow)

	bool m_bSaveUploadQueueWaitTime;//Morph - added by AndCycle, Save Upload Queue Wait Time (MSUQWT)
	HTREEITEM m_htiSaveUploadQueueWaitTime;//Morph - added by AndCycle, Save Upload Queue Wait Time (MSUQWT)

	//EastShare - Added by Pretender, Option for ChunkDots
	bool m_bEnableChunkDots;
	HTREEITEM m_htiEnableChunkDots;
	//EastShare - Added by Pretender, Option for ChunkDots

	//EastShare - added by AndCycle, IP to Country
	int	m_iIP2CountryName;
	HTREEITEM m_htiIP2CountryName;
	HTREEITEM m_htiIP2CountryName_DISABLE;
	HTREEITEM m_htiIP2CountryName_SHORT;
	HTREEITEM m_htiIP2CountryName_MID;
	HTREEITEM m_htiIP2CountryName_LONG;
	bool m_bIP2CountryShowFlag;
	HTREEITEM m_htiIP2CountryShowFlag;
	//EastShare - added by AndCycle, IP to Country
    bool m_bIP2CountryAutoUpdate;
	HTREEITEM m_htiIP2CountryAutoUpdate;

	HTREEITEM m_htiMisc;

	// EastShare START - Added by linekin, new creditsystem by [lovelace]
	HTREEITEM m_htiCreditSystem;
	// EastShare START - Added by linekin, new creditsystem by [lovelace]

	//Morph - added by AndCycle, Equal Chance For Each File
	bool	m_bEnableEqualChanceForEachFile;
	HTREEITEM m_htiEnableEqualChanceForEachFile;
	//Morph - added by AndCycle, Equal Chance For Each File

	bool m_bStaticIcon;
	HTREEITEM m_htiStaticIcon; //MORPH - Added, Static Tray Icon

int m_iPowershareMode; //MORPH - Added by SiRoB, Avoid misusing of powersharing
	int	m_iSpreadbar; //MORPH	- Added by AndCycle, SLUGFILLER: Spreadbars - per file
	int m_iHideOS;	//MORPH - Added by SiRoB, SLUGFILLER: hideOS
	bool m_bSelectiveShare;  //MORPH - Added by SiRoB, SLUGFILLER: hideOS
	int m_iShareOnlyTheNeed; //MORPH - Added by SiRoB, SHARE_ONLY_THE_NEED
	int m_iPowerShareLimit; //MORPH - Added by SiRoB, POWERSHARE Limit
	int m_iPermissions; //MORPH - Added by SiRoB, Show Permissions
	// Mighty Knife: Community visualization
	CString   m_sCommunityName;
	// [end] Mighty Knife
	bool m_bPowershareInternalPrio; //Morph - added by AndCyle, selective PS internal Prio

protected:	
	HTREEITEM m_htiSFM;
	//MORPH START - Added by SiRoB, Avoid misusing of powersharing
	HTREEITEM m_htiPowershareMode;
	HTREEITEM m_htiPowershareDisabled;
	HTREEITEM m_htiPowershareActivated;
	HTREEITEM m_htiPowershareAuto;
	HTREEITEM m_htiPowershareLimited;
	//MORPH END   - Added by SiRoB, Avoid misusing of powersharing

	//MORPH	Start	- Added by AndCycle, SLUGFILLER: Spreadbars - per file
	HTREEITEM m_htiSpreadbar;
	//MORPH	End	- Added by AndCycle, SLUGFILLER: Spreadbars - per file

	HTREEITEM m_htiHideOS;	//MORPH - Added by SiRoB, SLUGFILLER: hideOS
	HTREEITEM m_htiSelectiveShare; //MORPH - Added by SiRoB, SLUGFILLER: hideOS
	HTREEITEM m_htiShareOnlyTheNeed; //MORPH - Added by SiRoB, SHARE_ONLY_THE_NEED
	HTREEITEM m_htiPowerShareLimit; //MORPH - Added by SiRoB, POWERSHARE Limit
	HTREEITEM m_htiPowershareInternalPrio; //Morph - added by AndCyle, selective PS internal Prio
	//MORPH START - Added by SiRoB, Show Permission
	HTREEITEM m_htiPermissions;
	HTREEITEM m_htiPermAll;
	HTREEITEM m_htiPermFriend;
	HTREEITEM m_htiPermNone;
	// Mighty Knife: Community visible filelist
	HTREEITEM m_htiPermCommunity;
	HTREEITEM m_htiCommunityName;
	// [end] Mighty Knife
	//MORPH END   - Added by SiRoB, Show Permission

	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam);
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
