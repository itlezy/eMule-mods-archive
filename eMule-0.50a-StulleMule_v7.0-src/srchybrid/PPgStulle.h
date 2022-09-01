#pragma once
#include "preferences.h"
#include "TreeOptionsCtrlEx.h"
// CPPgStulle dialog

class CPPgStulle : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgStulle)

public:
	CPPgStulle();
	virtual ~CPPgStulle();

// Dialog Data
	enum { IDD = IDD_PPG_STULLE };
protected:
	
	// ==> Sivka-Ban - Stulle
	bool m_bEnableSivkaBan;
	int m_iSivkaAskTime;
	int m_iSivkaAskCounter;
	bool m_bSivkaAskLog;
	// <== Sivka-Ban - Stulle
	// ==> ban systems optional - Stulle
	bool m_bEnableAntiLeecher; //MORPH - Added by IceCream, enable Anti-leecher
	bool m_bBadModString;
	bool m_bBadNickBan;
	bool m_bGhostMod;
	bool m_bAntiModIdFaker;
	bool m_bAntiNickThief; // AntiNickThief - Stulle
	bool m_bEmptyNick;
	bool m_bFakeEmule;
	bool m_bLeecherName;
	bool m_bCommunityCheck;
	bool m_bHexCheck;
	bool m_bEmcrypt;
	bool m_bBadInfo;
	bool m_bBadHello;
	bool m_bSnafu;
	bool m_bExtraBytes;
	bool m_bNickChanger;
	bool m_bFileFaker;
	bool m_bVagaa;
	// <== ban systems optional - Stulle
	// ==> Reduce Score for leecher - Stulle
	int m_iReduceScore;
	int m_iReduceFactor;
	// <== Reduce Score for leecher - Stulle
	bool m_bNoBadPushing; // Disable PS/PBF for leechers [Stulle/idea by sfrqlxert] - Stulle
	bool m_bEnableAntiCreditHack; //MORPH - Added by IceCream, enable Anti-CreditHack
	bool m_bAntiXsExploiter; // Anti-XS-Exploit [Xman] - Stulle
	bool m_bSpamBan; // Spam Ban [Xman] - Stulle
	//MORPH START - Added by schnulli900, filter clients with failed downloads [Xman]
	bool m_bFilterClientFailedDown; 
	//MORPH END   - Added by schnulli900, filter clients with failed downloads [Xman]
	int m_iClientBanTime; // adjust ClientBanTime - Stulle

	// ==> push small files [sivka] - Stulle
	bool m_bEnablePushSmallFile;
	int  m_iPushSmallFileBoost;
	// <== push small files [sivka] - Stulle
	bool m_bEnablePushRareFile; // push rare file - Stulle

	// ==> FunnyNick Tag - Stulle
	int m_iFnTag;
	CString   m_sFnCustomTag;
	bool m_bFnTagAtEnd;
	// <== FunnyNick Tag - Stulle

	// ==> Quick start [TPT] - Stulle
	bool m_bQuickStart;
	int m_iQuickStartMaxTime;
	int m_iQuickStartMaxConnPerFive;
	int m_iQuickStartMaxConn;
	int m_iQuickStartMaxConnPerFiveBack;
	int m_iQuickStartMaxConnBack;
	bool m_bQuickStartAfterIPChange;
	// <== Quick start [TPT] - Stulle
	// ==> Connection Checker [eWombat/WiZaRd] - Stulle
	bool m_bCheckCon;
    bool m_bICMP;
    int m_uiPingTimeOut;
	int m_uiPingTTL;
	// <== Connection Checker [eWombat/WiZaRd] - Stulle
	// ==> Enforce Ratio - Stulle
	bool m_bEnforceRatio;
	int m_iRatioValue;
	// <== Enforce Ratio - Stulle
	// ==> Inform Clients after IP Change - Stulle
	bool m_bIsreaskSourceAfterIPChange;
	bool m_bInformQueuedClientsAfterIPChange;
	// <== Inform Clients after IP Change - Stulle
	int m_iReAskFileSrc; // Timer for ReAsk File Sources - Stulle

	// ==> Anti Uploader Ban - Stulle
	int m_iAntiUploaderBanLimit;
	int m_iAntiUploaderBanCase;
	// <== Anti Uploader Ban - Stulle

	// ==> CPU/MEM usage [$ick$/Stulle] - Stulle
	bool m_bSysInfo;
	bool m_bSysInfoGlobal;
	// <== CPU/MEM usage [$ick$/Stulle] - Stulle
	bool showSrcInTitle; // Show sources on title - Stulle
	bool m_bShowGlobalHL; // show global HL - Stulle
	bool m_bShowFileHLconst; // show HL per file constantly
	bool m_bShowInMSN7; // Show in MSN7 [TPT] - Stulle
	bool m_bTrayComplete; // Completed in Tray - Stulle
	bool m_bShowSpeedMeter; // High resulution speedmeter on toolbar [eFMod/Stulle] - Stulle

	// ==> drop sources - Stulle
	bool m_bEnableAutoDropNNSDefault;
	int m_iAutoNNS_TimerDefault;
	int m_iMaxRemoveNNSLimitDefault;
	bool m_bEnableAutoDropFQSDefault;
	int m_iAutoFQS_TimerDefault;
	int m_iMaxRemoveFQSLimitDefault;
	bool m_bEnableAutoDropQRSDefault;
	int m_iAutoHQRS_TimerDefault;
	int m_iMaxRemoveQRSDefault;
	int m_iMaxRemoveQRSLimitDefault;
	// <== drop sources - Stulle

	// ==> TBH: minimule - Stulle
	bool m_bShowMM;
	bool m_bMMLives;
	int m_iMMUpdateTime;
	int m_iMMTrans;
	bool m_bMMBanner;
	bool m_bMMCompl;
	bool m_bMMOpen;
	// <== TBH: minimule - Stulle

	// ==> Control download priority [tommy_gun/iONiX] - Stulle
	int m_iAutoDownPrio;
	int m_iAutoDownPrioPerc;
	int m_iAutoDownPrioSize;
	int m_iAutoDownPrioVal;
	// <== Control download priority [tommy_gun/iONiX] - Stulle

	// ==> Spread Credits Slot - Stulle
	bool m_bSpreadCreditsSlot;
	int m_iSpreadCreditsSlotCounter;
	bool m_bSpreadCreditsSlotPS;
	// <== Spread Credits Slot - Stulle
	// ==> Global Source Limit - Stulle
	bool m_bGlobalHL;
	int m_iGlobalHL;
	bool m_bGlobalHlAll;
	bool m_bGlobalHlDefault;
	// <== Global Source Limit - Stulle
	// ==> Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	bool m_bEmuMLDonkey;
	bool m_bEmueDonkey;
	bool m_bEmueDonkeyHybrid;
	bool m_bEmuShareaza;
	bool m_bEmuLphant;
	bool m_bLogEmulator;
	// <== Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	// ==> Release Bonus [sivka] - Stulle
	int m_iReleaseBonus;
	int m_iReleaseBonusDays;
	// <== Release Bonus [sivka] - Stulle
	bool m_bReleaseScoreAssurance; // Release Score Assurance - Stulle
	// ==> Automatic shared files updater [MoNKi] - Stulle
#ifdef ASFU
	bool m_bAutoSharedUpdater;
	bool m_bSingleSharedDirUpdater;
	int m_iTimeBetweenReloads;
#endif
	// <== Automatic shared files updater [MoNKi] - Stulle

	CTreeOptionsCtrlEx m_ctrlTreeOptions;
	bool m_bInitializedTreeOpts;
	HTREEITEM m_htiSecu;
	// ==> Sivka-Ban - Stulle
	HTREEITEM m_htiSivkaBanGroup;
	HTREEITEM m_htiEnableSivkaBan;
	HTREEITEM m_htiSivkaAskTime;
	HTREEITEM m_htiSivkaAskCounter;
	HTREEITEM m_htiSivkaAskLog;
	// <== Sivka-Ban - Stulle
	// ==> ban systems optional - Stulle
	HTREEITEM m_htiAntiLeecherGroup;
	HTREEITEM m_htiEnableAntiLeecher; //MORPH - Added by IceCream, enable Anti-leecher
	HTREEITEM m_htiBadModString;
	HTREEITEM m_htiBadNickBan;
	HTREEITEM m_htiGhostMod;
	HTREEITEM m_htiAntiModIdFaker;
	HTREEITEM m_htiAntiNickThief; // AntiNickThief - Stulle
	HTREEITEM m_htiEmptyNick;
	HTREEITEM m_htiFakeEmule;
	HTREEITEM m_htiLeecherName;
	HTREEITEM m_htiCommunityCheck;
	HTREEITEM m_htiHexCheck;
	HTREEITEM m_htiEmcrypt;
	HTREEITEM m_htiBadInfo;
	HTREEITEM m_htiBadHello;
	HTREEITEM m_htiSnafu;
	HTREEITEM m_htiExtraBytes;
	HTREEITEM m_htiNickChanger;
	HTREEITEM m_htiFileFaker;
	HTREEITEM m_htiVagaa;
	// <== ban systems optional - Stulle
	// ==> Reduce Score for leecher - Stulle
	HTREEITEM m_htiPunishmentGroup;
	HTREEITEM m_htiBanAll;
	HTREEITEM m_htiReduce;
	HTREEITEM m_htiReduceFactor;
	// <== Reduce Score for leecher - Stulle
	HTREEITEM m_htiNoBadPushing; // Disable PS/PBF for leechers [Stulle/idea by sfrqlxert] - Stulle
	HTREEITEM m_htiEnableAntiCreditHack; //MORPH - Added by IceCream, enable Anti-CreditHack
	HTREEITEM m_htiAntiXsExploiter; // Anti-XS-Exploit [Xman] - Stulle
	HTREEITEM m_htiSpamBan; // Spam Ban [Xman] - Stulle
	//MORPH START - Added by schnulli900, filter clients with failed downloads [Xman]
	HTREEITEM m_htiFilterClientFailedDown; 
	//MORPH END   - Added by schnulli900, filter clients with failed downloads [Xman]
	HTREEITEM m_htiClientBanTime; // adjust ClientBanTime - Stulle

	HTREEITEM m_htiPush; // push files - Stulle
	// ==> push small files [sivka] - Stulle
	HTREEITEM m_htiEnablePushSmallFile;
	HTREEITEM m_htiPushSmallFileBoost;
	// <== push small files [sivka] - Stulle
	HTREEITEM m_htiEnablePushRareFile; // push rare file - Stulle

	// ==> FunnyNick Tag - Stulle
	HTREEITEM m_htiFnTag;
	HTREEITEM m_htiNoTag;
	HTREEITEM m_htiShortTag;
	HTREEITEM m_htiFullTag;
	HTREEITEM m_htiCustomTag;
	HTREEITEM m_htiFnCustomTag;
	HTREEITEM m_htiFnTagAtEnd;
	// <== FunnyNick Tag - Stulle

	HTREEITEM m_htiConTweaks;
	// ==> Quick start [TPT] - Stulle
	HTREEITEM m_htiQuickStartGroup;
	HTREEITEM m_htiQuickStart;
	HTREEITEM m_htiQuickStartMaxTime;
	HTREEITEM m_htiQuickStartMaxConnPerFive;
	HTREEITEM m_htiQuickStartMaxConn;
	HTREEITEM m_htiQuickStartMaxConnPerFiveBack;
	HTREEITEM m_htiQuickStartMaxConnBack;
	HTREEITEM m_htiQuickStartAfterIPChange;
	// <== Quick start [TPT] - Stulle
	// ==> Connection Checker [eWombat/WiZaRd] - Stulle
	HTREEITEM	m_htiCheckConGroup;
	HTREEITEM	m_htiCheckCon;
	HTREEITEM	m_htiICMP;
	HTREEITEM	m_htiPingTimeOut;
	HTREEITEM	m_htiPingTTL;
	// <== Connection Checker [eWombat/WiZaRd] - Stulle
	// ==> Enforce Ratio - Stulle
	HTREEITEM m_htiRatioGroup;
	HTREEITEM m_htiEnforceRatio;
	HTREEITEM m_htiRatioValue;
	// <== Enforce Ratio - Stulle
	// ==> Inform Clients after IP Change - Stulle
	HTREEITEM m_htiIsreaskSourceAfterIPChange;
	HTREEITEM m_htiInformQueuedClientsAfterIPChange;
	// <== Inform Clients after IP Change - Stulle
	HTREEITEM m_htiReAskFileSrc; // Timer for ReAsk File Sources - Stulle

	// ==> Anti Uploader Ban - Stulle
	HTREEITEM m_htiAntiUploaderBanLimit;
	HTREEITEM m_htiAntiCase1;
	HTREEITEM m_htiAntiCase2;
	HTREEITEM m_htiAntiCase3;
	// <== Anti Uploader Ban - Stulle

	HTREEITEM m_htiDisplay;
	// ==> CPU/MEM usage [$ick$/Stulle] - Stulle
	HTREEITEM m_htiSysInfoGroup;
	HTREEITEM m_htiSysInfo;
	HTREEITEM m_htiSysInfoGlobal;
	// <== CPU/MEM usage [$ick$/Stulle] - Stulle
	HTREEITEM m_htiShowSrcOnTitle; // Show sources on title - Stulle
	HTREEITEM m_htiShowGlobalHL; // show global HL - Stulle
	HTREEITEM m_htiShowFileHLconst; // show HL per file constantly
	HTREEITEM m_htiShowInMSN7; // Show in MSN7 [TPT] - Stulle
	HTREEITEM m_htiTrayComplete; // Completed in Tray - Stulle
	HTREEITEM m_htiShowSpeedMeter; // High resulution speedmeter on toolbar [eFMod/Stulle] - Stulle

	// ==> drop sources - Stulle
	HTREEITEM m_htiDropDefaults;
	HTREEITEM m_htiAutoNNS;
	HTREEITEM m_htiAutoNNSTimer;
	HTREEITEM m_htiAutoNNSLimit;
	HTREEITEM m_htiAutoFQS;
	HTREEITEM m_htiAutoFQSTimer;
	HTREEITEM m_htiAutoFQSLimit;
	HTREEITEM m_htiAutoQRS;
	HTREEITEM m_htiAutoQRSTimer;
	HTREEITEM m_htiAutoQRSMax;
	HTREEITEM m_htiAutoQRSLimit;
	// <== drop sources - Stulle

	// ==> TBH: minimule - Stulle
	HTREEITEM m_htiMMGroup;
	HTREEITEM m_htiShowMM;
	HTREEITEM m_htiMMLives;
	HTREEITEM m_htiMMUpdateTime;
	HTREEITEM m_htiMMTrans;
	HTREEITEM m_htiMMBanner;
	HTREEITEM m_htiMMCompl;
	HTREEITEM m_htiMMOpen;
	// <== TBH: minimule - Stulle

	// ==> Control download priority [tommy_gun/iONiX] - Stulle
	HTREEITEM m_htiAutoDownPrioGroup;
	HTREEITEM m_htiAutoDownPrioOff;
	HTREEITEM m_htiAutoDownPrioPerc;
	HTREEITEM m_htiAutoDownPrioPercVal;
	HTREEITEM m_htiAutoDownPrioSize;
	HTREEITEM m_htiAutoDownPrioSizeVal;
	HTREEITEM m_htiAutoDownPrioValGroup;
	HTREEITEM m_htiAutoDownPrioLow;
	HTREEITEM m_htiAutoDownPrioNormal;
	HTREEITEM m_htiAutoDownPrioHigh;
	// <== Control download priority [tommy_gun/iONiX] - Stulle

	HTREEITEM m_htiMisc;
	// ==> Spread Credits Slot - Stulle
	HTREEITEM m_htiSpreadCreditsSlotGroup;
	HTREEITEM m_htiSpreadCreditsSlot;
	HTREEITEM m_htiSpreadCreditsSlotCounter;
	HTREEITEM m_htiSpreadCreditsSlotPS;
	// <== Spread Credits Slot - Stulle
	// ==> Global Source Limit - Stulle
	HTREEITEM m_htiGlobalHlGroup;
	HTREEITEM m_htiGlobalHL;
	HTREEITEM m_htiGlobalHlLimit;
	HTREEITEM m_htiGlobalHlAll;
	HTREEITEM m_htiGlobalHlDefault;
	// <== Global Source Limit - Stulle
	// ==> Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	HTREEITEM m_htiEmulatorGroup;
	HTREEITEM m_htiEmuMLDonkey;
	HTREEITEM m_htiEmueDonkey;
	HTREEITEM m_htiEmueDonkeyHybrid;
	HTREEITEM m_htiEmuShareaza;
	HTREEITEM m_htiEmuLphant;
	HTREEITEM m_htiLogEmulator;
	// <== Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	// ==> Release Bonus [sivka] - Stulle
	HTREEITEM m_htiReleaseBonusGroup;
	HTREEITEM m_htiReleaseBonus0;
	HTREEITEM m_htiReleaseBonus1;
	HTREEITEM m_htiReleaseBonusDays;
	HTREEITEM m_htiReleaseBonusDaysEdit;
	// <== Release Bonus [sivka] - Stulle
	HTREEITEM m_htiReleaseScoreAssurance; // Release Score Assurance - Stulle
	// ==> Automatic shared files updater [MoNKi] - Stulle
#ifdef ASFU
	HTREEITEM m_htiAutoSharedGroup;
	HTREEITEM m_htiAutoSharedUpdater;
	HTREEITEM m_htiSingleSharedDirUpdater;
	HTREEITEM m_htiTimeBetweenReloads;
#endif
	// <== Automatic shared files updater [MoNKi] - Stulle

	// ==> push small files [sivka] - Stulle
	uint32 m_iPushSmallFiles;
	void ShowPushSmallFileValues();
	CSliderCtrl m_ctlPushSmallSize;
	// <== push small files [sivka] - Stulle

	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnApply();
	virtual BOOL OnInitDialog();
	virtual BOOL OnKillActive();
	afx_msg void OnSettingsChange()			{ SetModified(); }
	afx_msg void OnEnChangeModified() { SetModified();}
public:
	void Localize(void);	
//	void LoadSettings(void);
};