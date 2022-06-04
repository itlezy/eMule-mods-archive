#pragma once
#include "HypertextCtrl.h"
#include "preferences.h"
#include "TreeOptionsCtrlEx.h"
#include "BtnST.h"
#include "ColorButton.h"
// CPPgScar dialog

void SysTimeToStr(LPSYSTEMTIME st, LPTSTR str); // Advanced Updates [MorphXT/Stulle] - Stulle

class CPPgScar : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgScar)

public:
	CPPgScar();
	virtual ~CPPgScar();

// Dialog Data
	enum { IDD = IDD_PPG_SCAR };
protected:
	// ==> push small files [sivka] - Stulle
	bool m_bEnablePushSmallFile;
	int  m_iPushSmallFileBoost;
	// <== push small files [sivka] - Stulle
	bool m_bEnablePushRareFile; // push rare file - Stulle

	// ==> FunnyNick [SiRoB/Stulle] - Stulle
	bool m_bFnActive;
	int m_iFnTag;
	CString   m_sFnCustomTag;
	bool m_bFnTagAtEnd;
	// <== FunnyNick [SiRoB/Stulle] - Stulle

	// ==> Quick start [TPT] - Max
	bool m_bQuickStart;
	int m_iQuickStartMaxTime;
	int m_iQuickStartMaxConnPerFive;
	int m_iQuickStartMaxConn;
	int m_iQuickStartMaxConnPerFiveBack;
	int m_iQuickStartMaxConnBack;
	bool m_bQuickStartAfterIPChange;
	// <== Quick start [TPT] - Max
	// ==> Enforce Ratio [Stulle] - Stulle
	bool m_bEnforceRatio;
	int m_iRatioValue;
	// <== Enforce Ratio [Stulle] - Stulle
	// ==> Improved ICS-Firewall support [MoNKi] - Max
	bool		m_bICFSupport;
	bool		m_bICFSupportClearAtEnd;
	bool		m_bICFSupportServerUDP;
	// <== Improved ICS-Firewall support [MoNKi] - Max
	// ==> UPnP support [MoNKi] - leuk_he
	bool m_bUPnPNat;
	bool m_bUpnPNATwebservice;
	DWORD m_dwUpnpBindAddr;
    bool m_bUPnPForceUpdate;
	// <== UPnP support [MoNKi] - leuk_he
	// ==> Random Ports [MoNKi] - Stulle
	bool m_bRandomports;
	int m_iRandomFirstPort;
	int m_iRandomLastPort;
	int m_iRandomPortsResetTime;
	// <== Random Ports [MoNKi] - Stulle
	// ==> Multiple Part Transfer [Stulle] - Mephisto
	int m_iChunksMode;
	int m_iChunksToFinish;
	int m_iChunksToUpload;
	// <== Multiple Part Transfer [Stulle] - Mephisto
	int m_iMaxUpMinutes; // Adjust max upload time [Stulle] - Mephisto
	int m_iReAskFileSrc; // Timer for ReAsk File Sources [Stulle] - Stulle
	bool m_bACC; // ACC [Max/WiZaRd] - Max
	bool m_bIgnoreThird; // Do not reserve 1/3 of your uploadlimit for emule [Stulle] - Stulle
	bool m_bUlThres; // Disable accepting only clients who asked within last 30min [Stulle] - Stulle
	bool m_bMaxSlotSpeed; // Alwasy maximize slot speed [Stulle] - Stulle
	// ==> Mephisto Upload - Mephisto
	int m_iMinSlots;
	int m_iNoNewSlotTimer;
	int m_iFullLoops;
	int m_iMonitorLoops;
	int m_iNotReachedBW;
	int m_iNoTrickleTimer;
	int m_iMoveDownKB;
	// <== Mephisto Upload - Mephisto

	// ==> Anti Uploader Ban [Stulle] - Stulle
	int m_iAntiUploaderBanLimit;
	int m_iAntiUploaderBanCase;
	// <== Anti Uploader Ban [Stulle] - Stulle

	// ==> CreditSystems [EastShare/ MorphXT] - Stulle
	int m_iCreditSystem;
	// <== CreditSystems [EastShare/ MorphXT] - Stulle
	bool m_bFineCS; // Modified FineCS [CiccioBastardo/Stulle] - Stulle
	// ==> Pay Back First [AndCycle/SiRoB/Stulle] - Stulle
	bool m_bIsPayBackFirst;
	int m_iPayBackFirstLimit;
	bool m_bIsPayBackFirst2;
	int m_iPayBackFirstLimit2;
	// <== Pay Back First [AndCycle/SiRoB/Stulle] - Stulle

	// ==> CPU/MEM usage [$ick$/Stulle] - Max
	bool m_bSysInfo;
	bool m_bSysInfoGlobal;
	// <== CPU/MEM usage [$ick$/Stulle] - Max
	// ==> Invisible Mode [TPT/MoNKi] - Stulle
	bool		m_bInvisibleMode;
	CString		m_sInvisibleModeMod;
	CString		m_sInvisibleModeKey;
	UINT		m_iInvisibleModeActualKeyModifier;
	bool		m_bInvisibleModeStart;
	// <== Invisible Mode [TPT/MoNKi] - Stulle
	bool showSrcInTitle; // Show sources on title - Stulle
	bool m_bShowGlobalHL; // show global HL - Stulle
	bool m_bShowFileHLconst; // show HL per file constantly - Stulle
	bool m_bShowInMSN7; // Show in MSN7 [TPT] - Stulle
	bool m_bQueueProgressBar; // Client queue progress bar [Commander] - Stulle
	bool m_bTrayComplete; // Completed in Tray [Stulle] - Stulle
	bool m_bColorFeedback; // Feedback personalization [Stulle] - Stulle
	bool m_bShowClientPercentage; // Show Client Percentage optional [Stulle] - Stulle
	bool m_bFollowTheMajority; // Follow The Majority [AndCycle/Stulle] - Stulle
	bool m_bShowSpeedMeter; // High resolution speedmeter on toolbar [eFMod/Stulle] - Myth88
	bool m_bStaticIcon; // Static Tray Icon - MyTh88

	// ==> File Settings [sivka/Stulle] - Stulle
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
	int m_iHQRXmanDefault;
	bool m_bGlobalHlDefault;
	// <== File Settings [sivka/Stulle] - Stulle

	// ==> TBH: minimule - Max
	bool m_bShowMM;
	bool m_bMMLives;
	int m_iMMUpdateTime;
	int m_iMMTrans;
	bool m_bMMBanner;
	bool m_bMMCompl;
	bool m_bMMOpen;
	// <== TBH: minimule - Max

	// ==> Control download priority [tommy_gun/iONiX] - MyTh88
	int m_iAutoDownPrio;
	int m_iAutoDownPrioPerc;
	int m_iAutoDownPrioSize;
	int m_iAutoDownPrioVal;
	// <== Control download priority [tommy_gun/iONiX] - MyTh88

	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	int m_iDlMode;
	bool m_bShowCatNames;
	bool m_bSelectCat;
	bool m_bUseActiveCat;
	bool m_bAutoSetResOrder;
	bool m_bSmallFileDLPush;
	int m_iResumeFileInNewCat;
	bool m_bUseAutoCat;
	bool m_bAddRemovedInc;
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

	// ==> PowerShare [ZZ/MorphXT] - Stulle
	int m_iPowershareMode;
	int m_iPowerShareLimit;
	// <== PowerShare [ZZ/MorphXT] - Stulle
	int m_iPsAmountLimit; // Limit PS by amount of data uploaded [Stulle] - Stulle
	// ==> Spread Credits Slot [Stulle] - Stulle
	bool m_bSpreadCreditsSlot;
	int m_iSpreadCreditsSlotCounter;
	// <== Spread Credits Slot [Stulle] - Stulle
	// ==> Release Bonus [sivka] - Stulle
	int m_iReleaseBonus;
	int m_iReleaseBonusDays;
	// <== Release Bonus [sivka] - Stulle
	bool m_bReleaseScoreAssurance; // Release Score Assurance [Stulle] - Stulle
	bool m_bSpreadBars; // Spread bars [Slugfiller/MorphXT] - Stulle
	// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	int m_iHideOS;
	bool m_bSelectiveShare;
	int m_iShareOnlyTheNeed;
	// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	int m_iFairPlay; // Fair Play [AndCycle/Stulle] - Stulle

	// ==> Global Source Limit [Max/Stulle] - Stulle
	bool m_bGlobalHL;
	int m_iGlobalHL;
	bool m_bGlobalHlAll;
	// <== Global Source Limit [Max/Stulle] - Stulle
	// ==> Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	bool m_bEmuMLDonkey;
	bool m_bEmueDonkey;
	bool m_bEmueDonkeyHybrid;
	bool m_bEmuShareaza;
	bool m_bEmuLphant;
	bool m_bLogEmulator;
	// <== Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	// ==> Automatic shared files updater [MoNKi] - Stulle
	bool m_bAutoSharedUpdater;
	bool m_bSingleSharedDirUpdater;
	int m_iTimeBetweenReloads;
	// <== Automatic shared files updater [MoNKi] - Stulle
	bool m_bSUQWT; // SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
	bool m_bStartupSound; // Startupsound [Commander] - mav744
	int m_iCompressLevel; // Adjust Compress Level [Stulle] - Stulle

	CTreeOptionsCtrlEx m_ctrlTreeOptions;
	bool m_bInitializedTreeOpts;

	HTREEITEM m_htiPush; // push files - Stulle
	// ==> push small files [sivka] - Stulle
	HTREEITEM m_htiEnablePushSmallFile;
	HTREEITEM m_htiPushSmallFileBoost;
	// <== push small files [sivka] - Stulle
	HTREEITEM m_htiEnablePushRareFile; // push rare file - Stulle

	// ==> FunnyNick [SiRoB/Stulle] - Stulle
	HTREEITEM m_htiFnTag;
	HTREEITEM m_htiFnActive;
	HTREEITEM m_htiFnTagMode;
	HTREEITEM m_htiNoTag;
	HTREEITEM m_htiShortTag;
	HTREEITEM m_htiFullTag;
	HTREEITEM m_htiCustomTag;
	HTREEITEM m_htiFnCustomTag;
	HTREEITEM m_htiFnTagAtEnd;
	// <== FunnyNick [SiRoB/Stulle] - Stulle

	HTREEITEM m_htiConTweaks;
	// ==> Quick start [TPT] - Max
	HTREEITEM m_htiQuickStartGroup;
	HTREEITEM m_htiQuickStart;
	HTREEITEM m_htiQuickStartMaxTime;
	HTREEITEM m_htiQuickStartMaxConnPerFive;
	HTREEITEM m_htiQuickStartMaxConn;
	HTREEITEM m_htiQuickStartMaxConnPerFiveBack;
	HTREEITEM m_htiQuickStartMaxConnBack;
	HTREEITEM m_htiQuickStartAfterIPChange;
	// <== Quick start [TPT] - Max
	// ==> Enforce Ratio [Stulle] - Stulle
	HTREEITEM m_htiRatioGroup;
	HTREEITEM m_htiEnforceRatio;
	HTREEITEM m_htiRatioValue;
	// <== Enforce Ratio [Stulle] - Stulle
	// ==> Improved ICS-Firewall support [MoNKi] - Max
	HTREEITEM m_htiICFSupportRoot;
	HTREEITEM m_htiICFSupport;
	HTREEITEM m_htiICFSupportClearAtEnd;
	HTREEITEM m_htiICFSupportServerUDP;
	// <== Improved ICS-Firewall support [MoNKi] - Max
	// ==> UPnP support [MoNKi] - leuk_he 
	HTREEITEM m_htiUPnPNatGroup;
	HTREEITEM m_htiUPnPNat;
	HTREEITEM m_htiUpnPNATwebservice;
	HTREEITEM m_htiUpnpBinaddr;
	HTREEITEM	m_htiUPnPForceUpdate;
	// <== UPnP support [MoNKi] - leuk_he
	// ==> Random Ports [MoNKi] - Stulle
	HTREEITEM m_htiRndGrp;
	HTREEITEM m_htiRandomports;
	HTREEITEM m_htiRandomFirstPort;
	HTREEITEM m_htiRandomLastPort;
	HTREEITEM m_htiRandomPortsResetTime;
	// <== Random Ports [MoNKi] - Stulle
	// ==> Mephisto Upload - Mephisto
	HTREEITEM m_htiMephistoUploadGrp;
	HTREEITEM m_htiMephistoWarning1;
	HTREEITEM m_htiMephistoWarning2;
	HTREEITEM m_htiMinSlots;
	HTREEITEM m_htiNoNewSlotTimer;
	HTREEITEM m_htiFullLoops;
	HTREEITEM m_htiMonitorLoops;
	HTREEITEM m_htiNotReachedBW;
	HTREEITEM m_htiNoTrickleTimer;
	HTREEITEM m_htiMoveDownKB;
	// <== Mephisto Upload - Mephisto
	// ==> Multiple Part Transfer [Stulle] - Mephisto
	HTREEITEM m_htiChunksGroup;
	HTREEITEM m_htiChunksScore;
	HTREEITEM m_htiChunksXman;
	HTREEITEM m_htiChunksFinish;
	HTREEITEM m_htiChunksToFinish;
	HTREEITEM m_htiChunksFull;
	HTREEITEM m_htiChunksToUpload;
	// <== Multiple Part Transfer [Stulle] - Mephisto
	HTREEITEM m_htiMaxUpMinutes; // Adjust max upload time [Stulle] - Mephisto
	HTREEITEM m_htiReAskFileSrc; // Timer for ReAsk File Sources [Stulle] - Stulle
	HTREEITEM m_htiACC; // ACC [Max/WiZaRd] - Max
	HTREEITEM m_htiIgnoreThird; // Do not reserve 1/3 of your uploadlimit for emule [Stulle] - Stulle
	HTREEITEM m_htiUlThres; // Disable accepting only clients who asked within last 30min [Stulle] - Stulle
	HTREEITEM m_htiMaxSlotSpeed; // Alwasy maximize slot speed [Stulle] - Stulle

	// ==> Anti Uploader Ban [Stulle] - Stulle
	HTREEITEM m_htiAntiUploaderBanLimit;
	HTREEITEM m_htiAntiCase1;
	HTREEITEM m_htiAntiCase2;
	HTREEITEM m_htiAntiCase3;
	// <== Anti Uploader Ban [Stulle] - Stulle

	// ==> CreditSystems [EastShare/ MorphXT] - Stulle
	HTREEITEM m_htiCreditSystem;
	HTREEITEM m_htiOfficialCredit;
	HTREEITEM m_htiLovelaceCredit;
	HTREEITEM m_htiRatioCredit;
	HTREEITEM m_htiPawcioCredit;
	HTREEITEM m_htiESCredit;
	HTREEITEM m_htiSivkaCredit;
	HTREEITEM m_htiSwatCredit;
	HTREEITEM m_htiXmanCredit;
	HTREEITEM m_htiTk4Credit;
	HTREEITEM m_htiZzulCredit;
	// <== CreditSystems [EastShare/ MorphXT] - Stulle
	HTREEITEM m_htiFineCS; // Modified FineCS [CiccioBastardo/Stulle] - Stulle
	// ==> Pay Back First [AndCycle/SiRoB/Stulle] - Stulle
	HTREEITEM m_htiIsPayBackFirst;
	HTREEITEM m_htiPayBackFirstLimit;
	HTREEITEM m_htiIsPayBackFirst2;
	HTREEITEM m_htiPayBackFirstLimit2;
	// <== Pay Back First [AndCycle/SiRoB/Stulle] - Stulle

	HTREEITEM m_htiDisplay;
	// ==> CPU/MEM usage [$ick$/Stulle] - Max
	HTREEITEM m_htiSysInfoGroup;
	HTREEITEM m_htiSysInfo;
	HTREEITEM m_htiSysInfoGlobal;
	// <== CPU/MEM usage [$ick$/Stulle] - Max
	// ==> Invisible Mode [TPT/MoNKi] - Stulle
	HTREEITEM	m_htiInvisibleModeRoot;
	HTREEITEM	m_htiInvisibleMode;
	HTREEITEM	m_htiInvisibleModeMod;
	HTREEITEM	m_htiInvisibleModeKey;
	HTREEITEM	m_htiInvisibleModeStart;
	// <== Invisible Mode [TPT/MoNKi] - Stulle
	HTREEITEM m_htiShowSrcOnTitle; // Show sources on title - Stulle
	HTREEITEM m_htiShowGlobalHL; // show global HL - Stulle
	HTREEITEM m_htiShowFileHLconst; // show HL per file constantly - Stulle
	HTREEITEM m_htiShowInMSN7; // Show in MSN7 [TPT] - Stulle
	HTREEITEM m_htiQueueProgressBar; // Client queue progress bar [Commander] - Stulle
	HTREEITEM m_htiTrayComplete; // Completed in Tray [Stulle] - Stulle
	HTREEITEM m_htiColorFeedback; // Feedback personalization [Stulle] - Stulle
	HTREEITEM m_htiShowClientPercentage; // Show Client Percentage optional [Stulle] - Stulle
	HTREEITEM m_htiFollowTheMajority; // Follow The Majority [AndCycle/Stulle] - Stulle
	HTREEITEM m_htiShowSpeedMeter; // High resolution speedmeter on toolbar [eFMod/Stulle] - Myth88
	// ==> Static Tray Icon [MorphXT] - MyTh88
	HTREEITEM m_htiStaticIcon;
	// <== Static Tray Icon [MorphXT] - MyTh88

	// ==> File Settings [sivka/Stulle] - Stulle
	HTREEITEM m_htiFileDefaults;
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
	HTREEITEM m_htiAutoQRSWay;
	HTREEITEM m_htiHQRXman;
	HTREEITEM m_htiHQRSivka;
	HTREEITEM m_htiGlobalHlDefault;
	// <== File Settings [sivka/Stulle] - Stulle

	// ==> TBH: minimule - Max
	HTREEITEM m_htiMMGroup;
	HTREEITEM m_htiShowMM;
	HTREEITEM m_htiMMLives;
	HTREEITEM m_htiMMUpdateTime;
	HTREEITEM m_htiMMTrans;
	HTREEITEM m_htiMMBanner;
	HTREEITEM m_htiMMCompl;
	HTREEITEM m_htiMMOpen;
	// <== TBH: minimule - Max

	// ==> Control download priority [tommy_gun/iONiX] - MyTh88
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
	// <== Control download priority [tommy_gun/iONiX] - MyTh88

	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	HTREEITEM m_htiSCC;
	HTREEITEM m_htiDlMode;
	HTREEITEM m_htiDlNone;
	HTREEITEM m_htiDlAlph;
	HTREEITEM m_htiDlLP;
	HTREEITEM m_htiShowCatNames;
	HTREEITEM m_htiSelectCat;
	HTREEITEM m_htiUseActiveCat;
	HTREEITEM m_htiAutoSetResOrder;
	HTREEITEM m_htiSmallFileDLPush;
	HTREEITEM m_htiResumeFileInNewCat;
	HTREEITEM m_htiUseAutoCat;
	HTREEITEM m_htiAddRemovedInc;
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

	HTREEITEM m_htiSharedPrefs; // Shared Files Management [Stulle] - Stulle
	// ==> PowerShare [ZZ/MorphXT] - Stulle
	HTREEITEM m_htiPowershareMode;
	HTREEITEM m_htiPowershareDisabled;
	HTREEITEM m_htiPowershareActivated;
	HTREEITEM m_htiPowershareAuto;
	HTREEITEM m_htiPowerShareLimit;
	HTREEITEM m_htiPowershareLimited;
	// <== PowerShare [ZZ/MorphXT] - Stulle
	HTREEITEM m_htiPsAmountLimit; // Limit PS by amount of data uploaded [Stulle] - Stulle
	// ==> Spread Credits Slot [Stulle] - Stulle
	HTREEITEM m_htiSpreadCreditsSlotGroup;
	HTREEITEM m_htiSpreadCreditsSlot;
	HTREEITEM m_htiSpreadCreditsSlotCounter;
	// <== Spread Credits Slot [Stulle] - Stulle,
	// ==> Release Bonus [sivka] - Stulle
	HTREEITEM m_htiReleaseBonusGroup;
	HTREEITEM m_htiReleaseBonus0;
	HTREEITEM m_htiReleaseBonus1;
	HTREEITEM m_htiReleaseBonusDays;
	HTREEITEM m_htiReleaseBonusDaysEdit;
	// <== Release Bonus [sivka] - Stulle
	HTREEITEM m_htiReleaseScoreAssurance; // Release Score Assurance [Stulle] - Stulle
	HTREEITEM m_htiSpreadBars; // Spread bars [Slugfiller/MorphXT] - Stulle
	// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	HTREEITEM m_htiHideOS;
	HTREEITEM m_htiSelectiveShare;
	HTREEITEM m_htiShareOnlyTheNeed;
	// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	HTREEITEM m_htiFairPlay; // Fair Play [AndCycle/Stulle] - Stulle

	HTREEITEM m_htiMisc;
	// ==> Global Source Limit [Max/Stulle] - Stulle
	HTREEITEM m_htiGlobalHlGroup;
	HTREEITEM m_htiGlobalHL;
	HTREEITEM m_htiGlobalHlLimit;
	HTREEITEM m_htiGlobalHlAll;
	HTREEITEM m_htiGlobalHlAggro;
	// <== Global Source Limit [Max/Stulle] - Stulle
	// ==> Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	HTREEITEM m_htiEmulatorGroup;
	HTREEITEM m_htiEmuMLDonkey;
	HTREEITEM m_htiEmueDonkey;
	HTREEITEM m_htiEmueDonkeyHybrid;
	HTREEITEM m_htiEmuShareaza;
	HTREEITEM m_htiEmuLphant;
	HTREEITEM m_htiLogEmulator;
	// <== Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	// ==> Automatic shared files updater [MoNKi] - Stulle
	HTREEITEM m_htiAutoSharedGroup;
	HTREEITEM m_htiAutoSharedUpdater;
	HTREEITEM m_htiSingleSharedDirUpdater;
	HTREEITEM m_htiTimeBetweenReloads;
	// <== Automatic shared files updater [MoNKi] - Stulle
	HTREEITEM m_htiSUQWT; // SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
	HTREEITEM m_htiStartupSound; // Startupsound [Commander] - mav744
	HTREEITEM m_htiCompressLevel; // Adjust Compress Level [Stulle] - Stulle

	// ==> push small files [sivka] - Stulle
	uint32 m_iPushSmallFiles;
	void ShowPushSmallFileValues();
	// <== push small files [sivka] - Stulle

	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	void LoadSettings(void);
	virtual BOOL OnApply();
	virtual BOOL OnInitDialog();
	virtual BOOL OnKillActive();
	afx_msg void OnSettingsChange()			{ SetModified(); }
	afx_msg void OnEnChangeModified() { SetModified();}
	afx_msg void OnTabSelectionChange(NMHDR *pNMHDR, LRESULT *pResult); // Tabbed Preferences [TPT] - Stulle
public:
	void Localize(void);	

	// ==> Tabbed Preferences [TPT] - Stulle
	enum eTab{
	SCAR,
	BACKUP,
	COLOR,
	ADVANCED,
	UPDATE,
	SUPPORT
	};
private:
	void SetTab(eTab tab);

	void InitTab();
	void InitControl();	


	// Tab	
	CTabCtrl   m_tabCtr;
	eTab       m_currentTab;
	CImageList m_imageList;

	// ScarAngel
	CStatic		m_strWarning;
	CStatic		m_strPushSmall, m_iPushSmallLabel;
	CSliderCtrl m_ctlPushSmallSize;

	// Backup
	CButton		m_BackupBox;
	CButton		m_Dat;
	CButton		m_Met;
	CButton		m_Ini;
	CButton		m_Part;
	CButton		m_PartMet;
	CButton		m_SelectAll;
	CButton		m_BackupNow;
	CButton		m_AutoBackupBox;
	CButton		m_AutoBackup;
	CButton		m_AutoBackup2;
	CButton		m_Note;
	CButton		m_NoteText;

	// Design settings
	CButton		m_ColorBox;
	CComboBox	m_MasterCombo;
	CComboBox	m_SubCombo;
	CButton		m_OnOff;
	CButtonST	m_bold;
	CButtonST	m_underlined;
	CButtonST	m_italic;
	CButton		m_FontColorLabel;
	CColorButton		m_FontColor;
	CButton		m_BackColorLabel;
	CColorButton	m_BackColor;
	CStatic		m_ColorWarning;
	CButton		m_EasterEgg;

	// Advanced
	CButton		m_AntiLeechBox;
	CButton		m_AntiLeechStart;
	CButton		m_AntiLeechWeek;
	CButton		m_AntiLeechURLStatic;
	CEdit		m_AntiLeechURL;
public:
	CButton		m_AntiLeechVersion;
private:
	CButton		m_AntiLeechReset;
	CButton		m_AntiLeechUpdate;
	CButton		m_IpFilterBox;
	CButton		m_IpFilterStart;
	CButton		m_IpFilterWeek;
	CButton		m_IpFilterURLStatic;
	CEdit		m_IpFilterURL;
public:
	CButton		m_IpFilterTime;
private:
	CButton		m_IpFilterReset;
	CButton		m_IpFilterUpdate;
	CButton		m_CountryBox;
	CButton		m_CountryStart;
	CButton		m_CountryURLStatic;
	CEdit		m_CountryURL;
	CButton		m_CountryTime;
	CButton		m_CountryReset;
	CButton		m_CountryUpdate;

	// Support
	CHyperTextCtrl	m_HpLink;
	CHyperTextCtrl	m_BoardGerLink;
	CHyperTextCtrl	m_BoardEngLink;
	CHyperTextCtrl	m_RateLink;
	/*
	CHyperTextCtrl	m_XtremeLink;
	*/
	// <== Tabbed Preferences [TPT] - Stulle

	// ==> TBH: Backup [TBH/EastShare/MorphXT] - Stulle
public:
	void Backup(LPCTSTR extensionToBack, BOOL conFirm);
	void Backup3();
	afx_msg void OnBnClickedBackupnow();
	afx_msg void OnBnClickedDat();
	afx_msg void OnBnClickedMet();
	afx_msg void OnBnClickedIni();
	afx_msg void OnBnClickedPart();
	afx_msg void OnBnClickedPartMet();
	afx_msg void OnBnClickedSelectall();
	afx_msg void OnBnClickedAutobackup();
	afx_msg void OnBnClickedAutobackup2();
private:
	void Backup2(LPCTSTR extensionToBack);
	void BackupNowEnable();
	BOOL y2All;
	// <== TBH: Backup [TBH/EastShare/MorphXT] - Stulle

	// ==> Design Settings [eWombat/Stulle] - Stulle
	StylesStruct nClientStyles[style_c_count];
	StylesStruct nDownloadStyles[style_d_count];
	StylesStruct nShareStyles[style_s_count];
	StylesStruct nServerStyles[style_se_count];
	StylesStruct nBackgroundStyles[style_b_count];
	StylesStruct nWindowStyles[style_w_count];
	// ==> Feedback personalization [Stulle] - Stulle
	StylesStruct nFeedBackStyles[style_f_count];
	// <== Feedback personalization [Stulle] - Stulle
	bool m_bFocusWasOnCombo;
	bool m_bDesignChanged;
	bool m_bBold;
	bool m_bUnderlined;
	bool m_bItalic;
public:
	void InitMasterStyleCombo();
	void InitSubStyleCombo();
	void UpdateStyles();
	void OnFontStyle();
	StylesStruct GetStyle(int nMaster, int nStyle);
	void SetStyle(int nMaster, int nStyle, StylesStruct *style=NULL);

	afx_msg LONG OnColorPopupSelChange(UINT lParam, LONG wParam);
	afx_msg void OnBnClickedBold();
	afx_msg void OnBnClickedUnderlined();
	afx_msg void OnBnClickedItalic();
	afx_msg void OnCbnSelchangeStyleselMaster();
	afx_msg void OnCbnSelchangeStyleselSub();
	afx_msg void OnBnClickedOnOff();
	afx_msg void OnEnKillfocusMasterCombo();
	afx_msg void OnEnKillfocusSubCombo();
	afx_msg void OnBnClickedEasteregg(); // Diabolic Easteregg [Stulle] - Mephisto
	// <== Design Settings [eWombat/Stulle] - Stulle

	// ==> Advanced Options [Official/MorphXT] - Stulle
protected:
	bool bMiniMuleAutoClose;
	int iMiniMuleTransparency;

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

	bool m_bRestoreLastMainWndDlg;
	bool m_bRestoreLastLogPane;
	int m_iStraightWindowStyles;
	bool m_bRTLWindowsLayout;
	int m_iMaxChatHistory;
    int  m_umaxmsgsessions;
	CString m_strDateTimeFormat;
	CString m_strDateTimeFormat4Lists;
	bool m_bShowVerticalHourMarkers;
	bool m_bReBarToolbar;
	bool m_bIconflashOnNewMessage;
	bool m_bShowCopyEd2kLinkCmd;
	bool m_bUpdateQueue;
	bool m_bRepaint;
	bool m_bExtraPreviewWithMenu;
	bool m_bShowUpDownIconInTaskbar;
	bool m_bForceSpeedsToKB;

	int m_iLogFileFormat;
	int iMaxLogBuff;
	CString m_strDateTimeFormat4Log;
	COLORREF m_crLogError;
	COLORREF m_crLogWarning;
	COLORREF m_crLogSuccess;

	bool bCheckComctl32 ;
	bool bCheckShell32;
	bool bIgnoreInstances;
	CString sNotifierMailEncryptCertName;
	int m_iPreviewSmallBlocks;
	bool m_bPreviewCopiedArchives;
	bool m_bPreviewOnIconDblClk;
	CString sInternetSecurityZone;
	CString sTxtEditor;
	int iServerUDPPort; // really a unsigned int 16
	bool m_bRemoveFilesToBin;
    bool m_bHighresTimer;
	bool m_bTrustEveryHash;
    int m_iInspectAllFileTypes;
    bool m_bPreferRestrictedOverUser;
	bool m_bUseUserSortedServerList;
    int m_iWebFileUploadSizeLimitMB;
    CString m_sAllowedIPs;
	int m_iDebugSearchResultDetailLevel;
	bool m_bAdjustNTFSDaylightFileTime;
	bool m_dontcompressavi;
	bool m_ICH;
	DWORD m_dwBindAddr;
	int m_iFileBufferTimeLimit;
	bool m_bRearrangeKadSearchKeywords;
	bool m_bBeeper;
	bool m_bMsgOnlySec;
	bool m_bDisablePeerCache;
	bool m_bKeepUnavailableFixedSharedDirs;

	CTreeOptionsCtrlEx m_ctrlAdvTreeOptions;
	bool m_bInitializedAdvTreeOpts;

	HTREEITEM m_hti_AdvMiniMule;
    HTREEITEM m_hti_bMiniMuleAutoClose;
	HTREEITEM m_hti_iMiniMuleTransparency;

	HTREEITEM m_hti_MediaInfo;
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

	HTREEITEM m_hti_AdvDisplay;
	HTREEITEM m_hti_m_bRestoreLastMainWndDlg;
	HTREEITEM m_hti_m_bRestoreLastLogPane;
	HTREEITEM m_hti_m_iStraightWindowStyles;
	HTREEITEM m_hti_m_bRTLWindowsLayout;
	HTREEITEM m_hti_m_iMaxChatHistory;
	HTREEITEM m_hti_maxmsgsessions;
	HTREEITEM m_htidatetimeformat;
	HTREEITEM m_htidatetimeformat4lists;
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
	HTREEITEM m_hti_m_iLogFileFormat;
	HTREEITEM m_hti_iMaxLogBuff;
	HTREEITEM m_htidatetimeformat4log;
	HTREEITEM m_htiLogError;
	HTREEITEM m_htiLogWarning;
	HTREEITEM m_htiLogSuccess;

	HTREEITEM m_hti_bCheckComctl32;
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
	HTREEITEM m_hti_InspectAllFileTypes;
	HTREEITEM m_hti_PreferRestrictedOverUser;
	HTREEITEM m_hti_WebFileUploadSizeLimitMB ;
	HTREEITEM m_hti_AllowedIPs;
	HTREEITEM m_hti_UseUserSortedServerList;
	HTREEITEM m_hti_DebugSearchResultDetailLevel;
	HTREEITEM m_htiAdjustNTFSDaylightFileTime;
	HTREEITEM m_htidontcompressavi;
	HTREEITEM m_htiICH;
	HTREEITEM  m_htiBindAddr;
	HTREEITEM m_htiFileBufferTimeLimit;
	HTREEITEM m_htiRearrangeKadSearchKeywords;
	HTREEITEM m_htiBeeper;
	HTREEITEM m_htiMsgOnlySec;
	HTREEITEM m_htiDisablePeerCache;
	HTREEITEM m_htiKeepUnavailableFixedSharedDirs;
	// <== Advanced Options [Official/MorphXT] - Stulle

	// ==> Advanced Updates [MorphXT/Stulle] - Stulle
public:
	afx_msg void OnBnClickedUpdateALUrl();
	afx_msg void OnBnClickedResetALUrl();
	afx_msg void OnBnClickedUpdateipfurl();
	afx_msg void OnBnClickedResetipfurl();
	afx_msg void OnBnClickedUpdateipcurl();
	afx_msg void OnBnClickedResetipcurl();
	// <== Advanced Updates [MorphXT/Stulle] - Stulle
};