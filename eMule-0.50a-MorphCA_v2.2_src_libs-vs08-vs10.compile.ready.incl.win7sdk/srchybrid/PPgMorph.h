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

	bool m_bEnableDownloadInRed; //MORPH - Added by IceCream, show download in red
	bool m_bEnableDownloadInBold; //MORPH - Added by SiRoB, show download in Bold
	bool m_bShowClientPercentage;
	//MORPH START - Added by Stulle, Global Source Limit
	bool m_bGlobalHL;
	int m_iGlobalHL;
	//MORPH END   - Added by Stulle, Global Source Limit

	bool m_bShowSessionDown; 
	bool m_bShowModstring; 
	bool m_bShowFileStatusIcons; 
    bool  m_bDropBlocking;
    bool  m_bShowBlocking;
    bool  m_bShowCpuRam;
    bool  m_bShowMinQR;
	bool m_bDropSystem; 

	bool m_bInfiniteQueue;	//Morph - added by AndCycle, SLUGFILLER: infiniteQueue
#ifdef SPARE_TRICKLE
	bool m_bDontRemoveSpareTrickleSlot; //Morph - added by AndCycle, Dont Remove Spare Trickle Slot
#endif

#ifdef DOWN_OVERHEAD
	bool m_bUseDownloadOverhead; //// MORPH leuk_he include download overhead in upload stats
#endif
    int  m_iCompressLevel; // MORPH compresslevel
	bool m_bUseCompression; // Use compression. 
	bool m_bAutoCompress; 
	bool m_bFunnyNick; //MORPH - Added by SiRoB, Optionnal funnynick display

	// NEO: QS - [QuickStart]
	bool	m_uQuickStart;
	int		m_iQuickStartTime;
	int		m_iQuickStartTimePerFile;
	int		m_iQuickMaxConperFive;
	int		m_iQuickMaxHalfOpen;
	int		m_iQuickMaxConnections;
	// NEO: QS END
/*
	//MORPH START - Added by SiRoB, Datarate Average Time Management
	int m_iDownloadDataRateAverageTime;
	int m_iUploadDataRateAverageTime;
	//MORPH END   - Added by SiRoB, Datarate Average Time Management
*/	
	//MORPH START - Added by SiRoB, Upload Splitting Class
	int m_iGlobalDataRateFriend;
	int m_iMaxGlobalDataRateFriend;
	int m_iMaxClientDataRateFriend;
	int m_iGlobalDataRatePowerShare;
	int m_iMaxGlobalDataRatePowerShare;
	int m_iMaxClientDataRatePowerShare;
	int m_iMaxClientDataRate;
	//MORPH END  - Added by SiRoB, Upload Splitting Class
	
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
	CString m_sBrokenURLs; //MORPH - Added by WiZaRd, Fix broken HTTP downloads
	bool m_bMMOpen;
	HTREEITEM m_htiMMOpen;

	CTreeOptionsCtrlEx m_ctrlTreeOptions;
	bool m_bInitializedTreeOpts;
	HTREEITEM m_htiDM;
	HTREEITEM m_htiUM;
	
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
	HTREEITEM m_htiShowFileStatusIcons; 
    HTREEITEM m_htiDropBlocking; 
	HTREEITEM m_htiShowBlocking; 
	HTREEITEM m_htiShowCpuRam; 
	HTREEITEM m_htiShowMinQR; 
	HTREEITEM m_htiDropSystem; 
	HTREEITEM m_htiShowSessionDown; 

	HTREEITEM m_htiInfiniteQueue;	//Morph - added by AndCycle, SLUGFILLER: infiniteQueue
#ifdef SPARE_TRICKLE
	HTREEITEM m_htiDontRemoveSpareTrickleSlot; //Morph - added by AndCycle, Dont Remove Spare Trickle Slot
#endif

#ifdef DOWN_OVERHEAD
	HTREEITEM m_htiUseDownloadOverhead; //// MORPH leuk_he include download overhead in upload stats
#endif
	HTREEITEM m_htiCompressLevel; //Morph - compresslevel
	HTREEITEM m_htiUseCompression; 
	HTREEITEM m_htiUseAutoCompression; 
	HTREEITEM m_htiDisplayFunnyNick;//MORPH - Added by SiRoB, Optionnal funnynick display

	/*
	//MORPH START - Added by SiRoB, Datarate Average Time Management
	HTREEITEM m_htiDownloadDataRateAverageTime;
	HTREEITEM m_htiUploadDataRateAverageTime;
	//MORPH END   - Added by SiRoB, Datarate Average Time Management
	*/
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
	HTREEITEM m_htiBrokenURLs; //MORPH - Added by WiZaRd, Fix broken HTTP downloads

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
		
// Added by MoNKi [ SlugFiller: -lowIdRetry- ]
    HTREEITEM	m_htiLowIdRetry;
	int			m_iLowIdRetry;
	// End SlugFiller
// Tux: Feature: Automatic shared files updater [start]
	HTREEITEM m_htiDirWatcher;
	bool m_bDirWatcher;
	// Tux: Feature: Automatic shared files updater [end]
	
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
    
	HTREEITEM m_htiMisc;

	// NEO: QS - [QuickStart]
	HTREEITEM m_htiQuickStart;
	HTREEITEM m_htiQuickStartEnable;
	HTREEITEM m_htiQuickStartTime;
	HTREEITEM m_htiQuickStartTimePerFile;
	HTREEITEM m_htiQuickMaxConperFive;
	HTREEITEM m_htiQuickMaxHalfOpen;
	HTREEITEM m_htiQuickMaxConnections;
	// NEO: QS END

	// EastShare START - Added by linekin, new creditsystem by [lovelace]
	HTREEITEM m_htiCreditSystem;
	// EastShare START - Added by linekin, new creditsystem by [lovelace]

	//Morph - added by AndCycle, Equal Chance For Each File
	bool	m_bEnableEqualChanceForEachFile;
	HTREEITEM m_htiEnableEqualChanceForEachFile;
	//Morph - added by AndCycle, Equal Chance For Each File

        int m_iPowershareMode; //MORPH - Added by SiRoB, Avoid misusing of powersharing
	int	m_iSpreadbar; //MORPH	- Added by AndCycle, SLUGFILLER: Spreadbars - per file
	int m_iHideOS;	//MORPH - Added by SiRoB, SLUGFILLER: hideOS
	bool m_bSelectiveShare;  //MORPH - Added by SiRoB, SLUGFILLER: hideOS
	int m_iShareOnlyTheNeed; //MORPH - Added by SiRoB, SHARE_ONLY_THE_NEED
	int m_iPowerShareLimit; //MORPH - Added by SiRoB, POWERSHARE Limit
#ifdef COMMUNITY	
	// Mighty Knife: Community visualization
	CString   m_sCommunityName;
	// [end] Mighty Knife
#endif
	bool m_bPowershareInternalPrio; //Morph - added by AndCyle, selective PS internal Prio
	int m_iPsAmountLimit; // Limit PS by amount of data uploaded - Stulle

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
	HTREEITEM m_htiPsAmountLimit; // Limit PS by amount of data uploaded - Stulle
	HTREEITEM m_htiPowershareInternalPrio; //Morph - added by AndCyle, selective PS internal Prio
#ifdef COMMUNITY	
	HTREEITEM m_htiCommunityName;
#endif
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
