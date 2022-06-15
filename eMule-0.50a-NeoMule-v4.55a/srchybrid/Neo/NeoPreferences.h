//this file is part of NeoMule
//Copyright (C)2007 David Xanatos ( XanatosDavid@googlemail.com / http://NeoMule.sf.net )
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
#pragma once
#include "PrefFunctions.h"
#include "FilePreferences.h"
#include "FilePreferences.h"
// NEO: NCFG - [NeoConfiguration] -- Xanatos -->

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
enum ePingMode;
#endif // NEO_BC // NEO: NBC END

#define AUTO_TEMPDIR 255 // NEO: MTD - [MultiTempDirectories]
class CNeoPreferences
{
public:

	static	DWORD	m_dAppPriority; // NEO: MOD - [AppPriority]
	static	int		m_iPreferredTempDir; // NEO: MTD - [MultiTempDirectories]
	static	bool	m_bPauseOnFileComplete; // NEO: POFC - [PauseOnFileComplete]

	static  UINT	m_uIncompletePartStatus;	// NEO: ICS - [InteligentChunkSelection] 
	static  UINT	m_uSubChunkTransfer; // NEO: SCT - [SubChunkTransfer]
	static  bool	m_bPartStatusHistory; // NEO: PSH - [PartStatusHistory]
	static  bool	m_bLowID2HighIDAutoCallback; // NEO: L2HAC - [LowID2HighIDAutoCallback]
	static  bool	m_bSaveComments; // NEO: XCs - [SaveComments]
	static  bool	m_bKnownComments; // NEO: XCk - [KnownComments]	
	
	static  bool	m_bPreferShareAll; // NEO: PSA - [PreferShareAll]
	static  UINT	m_uLugdunumCredits; // NEO: KLC - [KhaosLugdunumCredits]
	static  bool	m_bDontRemoveStaticServers; // NEO: MOD - [DontRemoveStaticServers]

	static  bool	m_bShowBanner; // NEO: NPB - [PrefsBanner]
	static	bool	m_bCollorShareFiles; // NEO: NSC - [NeoSharedCategories]
	static	bool	m_bSmoothStatisticsGraphs; // NEO: NBC - [NeoBandwidthControl] 

	static	UINT	m_uDisableAutoSort; // NEO: SE - [SortExtension]

	// NEO: NSTI - [NewSystemTrayIcon]
	static	bool	m_bShowSystemTrayUpload;
	static	bool	m_bThinSystemTrayBars;
	static	int		m_iTrayBarsMaxCollor;
	// NEO: NSTI END

	static	bool	m_bStaticTrayIcon; // NEO: STI - [StaticTray]

	// NEO: IM - [InvisibelMode]
	static	bool	m_bInvisibleMode;
	static	UINT	m_iInvisibleModeHotKeyModifier;
	static	TCHAR	m_cInvisibleModeHotKey;
	// NEO: IM END

	// NEO: TPP - [TrayPasswordProtection]
	static	bool	m_bTrayPasswordProtection;
	static	CString	m_sTrayPassword;
	// NEO: TPP END

	static	bool	m_bUsePlusSpeedMeter; // NEO: PSM - [PlusSpeedMeter]

	static	bool	m_bUseRelativeChunkDisplay; // NEO: MOD - [RelativeChunkDisplay]

	// NEO: SI - [SysInfo]
	static	bool	m_bDrawSysInfoGraph;
	static	bool	m_bShowSysInfoOnTitle;
	// NEO: SI END
	
	static	bool	m_bUseChunkDots;	// NEO: MOD - [ChunkDots]

	static	bool	m_bUseTreeStyle; 	// NEO: NTS - [NeoTreeStyle]

	static	bool	m_bShowClientPercentage; // NEO: MOD - [Percentage]

	// NEO: MOD - [RefreshShared]
	static	bool	m_bRefreshShared;
	static	int		m_iRefreshSharedIntervals;
	// NEO: MOD END

	// NEO: OCF - [OnlyCompleetFiles]
	static bool		m_bOnlyCompleteFiles;
	static int		m_iToOldComplete;
	// NEO: OCF END
	
	// NEO: OCC - [ObelixConnectionControl]
	static bool		m_bConnectionControl;
	static int		m_iConnectionControlValue;
	// NEO: OCC END

	// NEO: SCM - [SmartConnectionManagement]
	static	bool	m_bManageConnections;
	static	float	m_fManageConnectionsFactor;
	// NEO: SCM END

	// NEO: NCC - [NeoConnectionChecker]
	static	bool	m_bAutoConnectionChecker;
	static	int		m_iAutoConnectionCheckerValue;
	// NEO: NCC END

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
	static	UINT	m_uNAFCEnabled;
	static	bool	m_bISPCustomIP;
	static	DWORD	m_uISPZone;
	static	DWORD	m_uISPMask;
	static	bool	m_bBindToAdapter;


	static	bool	m_bCheckConnection;
	static	bool	m_bStaticLowestPing;
	static	int		m_iPingMode;
	static	bool	m_bNoTTL;
	static	bool	m_bManualHostToPing;
	static	CString m_sPingServer;

	static	bool	m_bAutoMSS;
	static	int		m_iMSS;

	static	bool	m_bUseDoubleSendSize;


	static	bool	m_bUseDownloadBandwidthThrottler;
	static	UINT	m_uUseHyperDownload;
	static	int		m_iBCTimeDown;
	static	int		m_iBCPriorityDown;
	static	UINT	m_uSetDownloadBuffer;
	static	int		m_iDownloadBufferSize;

	static	UINT	m_uUseBlockedQueue;
	static	int		m_iBCTimeUp;
	static	int		m_iBCPriorityUp;
	static	UINT	m_uSetUploadBuffer;
	static	int		m_iUploadBufferSize;

	static	int		m_iDatarateSamples;


	static	bool	m_bIncludeOverhead;
	static	bool	m_bIncludeTCPAck;
	static	bool	m_bConnectionsOverHead;
	static	bool	m_bSessionRatio;


	static	int		m_iDownloadControl;
	static	float	m_fMinBCDownload;
	static	float	m_fMaxBCDownload;


	static	int		m_iUploadControl;
	static	float	m_fMinBCUpload;
	static	float	m_fMaxBCUpload;


	static	float	m_fMaxDownStream;
	static	float	m_fMaxUpStream;


	static	bool	m_bMinimizeOpenedSlots;
	static	UINT	m_uCumulateBandwidth;
	static	UINT	m_uBadwolfsUpload;

	static	int		m_iMaxReservedSlots;
	
	static	bool	m_bIncreaseTrickleSpeed;
	static	float	m_fIncreaseTrickleSpeed;

	static int		m_iMaxUploadSlots;
	static int		m_iMinUploadSlots;
	static float	m_fUploadPerSlots;

 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
	static	UINT	m_uSeparateReleaseBandwidth;
	static	float	m_fReleaseSlotSpeed;
	static	float	m_fReleaseBandwidthPercentage;

	static	UINT	m_uSeparateFriendBandwidth;
	static	float	m_fFriendSlotSpeed;
	static	float	m_fFriendBandwidthPercentage;
 #endif // BW_MOD // NEO: BM END

	static	bool	m_bOpenMoreSlotsWhenNeeded;
	static	bool	m_bCheckSlotDatarate;

	static	bool	m_bIsTrickleBlocking;
	static	bool	m_bIsDropBlocking;


	static	bool	m_bDynUpGoingDivider;
	static	int		m_iDynUpGoingUpDivider;
	static	int		m_iDynUpGoingDownDivider;

	static	int		m_iUpMaxPingMethod;
	static	int		m_iBasePingUp;
	static	int		m_iPingUpTolerance;
	static	int		m_iPingUpProzent;


	static	bool	m_bDynDownGoingDivider;
	static	int		m_iDynDownGoingUpDivider;
	static	int		m_iDynDownGoingDownDivider;
	
	static	int		m_iDownMaxPingMethod;
	static	int		m_iBasePingDown;
	static	int		m_iPingDownTolerance;
	static	int		m_iPingDownProzent;
#endif // NEO_BC // NEO: NBC END

	static	bool	m_bTCPDisableNagle;

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	static	bool	m_bLancastEnabled;

	static	uint16	m_iMaxLanDownload;
	static	bool	m_bSetLanDownloadBuffer;
	static	int		m_iLanDownloadBufferSize;

	static	uint16	m_iMaxLanUpload;
	static	bool	m_bSetLanUploadBuffer;
	static	int		m_iLanUploadBufferSize;

	static	int		m_iMaxLanUploadSlots;

	static	bool	m_bCustomizedLanCast;
	static	CString	m_sLanCastGroup;
	static	uint16	m_uLanCastPort;

	static	bool	m_bCustomLanCastAdapter;
	static	DWORD	m_uLanCastAdapterIPAdress;
	static	DWORD	m_uLanCastAdapterSubNet;

	static	bool	m_bAutoBroadcastLanFiles;
	static	int		m_iAutoBroadcastLanFiles;

	static	bool	m_bUseLanMultiTransfer;
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	static	bool	m_bUseVoodooTransfer;
	static	bool	m_bSlaveAllowed;
	static	bool	m_bSlaveHosting;

	static	CString	m_sVoodooSpell;
	static	uint16	m_nVoodooPort;

	static	UINT	m_uAutoConnectVoodoo;
	static	int		m_iVoodooReconectTime;

	static	UINT	m_uUseVirtualVoodooFiles;

	static	UINT	m_uVoodooCastEnabled;

	static	UINT	m_uSearchForSlaves;
	static	UINT	m_uSearchForMaster;
	static	int		m_iVoodooSearchIntervals;

	static	bool	m_bHideVoodooFiles; // NEO: PP - [PasswordProtection]
#endif // VOODOO // NEO: VOODOO END

	// NEO: QS - [QuickStart]
	static	UINT	m_uQuickStart;
	static	int		m_iQuickStartTime;
	static	int		m_iQuickStartTimePerFile;
	static	UINT	m_iQuickMaxConperFive;
	static	UINT	m_iQuickMaxHalfOpen;
	static	UINT	m_iQuickMaxConnections;

	static	bool	m_bOnQuickStart;
	// NEO: QS END

	// NEO: RIC - [ReaskOnIDChange]
	static	UINT	m_uCheckIPChange;
	static	bool	m_bInformOnIPChange;
	static	bool	m_bInformOnBuddyChange;
	static	bool	m_bReAskOnIPChange;
	static	bool	m_bQuickStartOnIPChange; // NEO: QS - [QuickStart]
	static	bool	m_bCheckL2HIDChange;
	static	bool	m_bReconnectKadOnIPChange;
	static	bool	m_bRebindSocketsOnIPChange;
	// NEO: RIC END

	// NEO: RKF - [RecheckKadFirewalled]
	static	bool	m_bRecheckKadFirewalled;
	static	int		m_iRecheckKadFirewalled;
	// NEO: RKF END

	// NEO: RLD - [ReconnectOnLowID]
	static	bool	m_bReConnectOnLowID;
	static	int		m_iReConnectOnLowID;
	// NEO: RLD END

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
	static bool		m_bNatTraversalEnabled;
	static UINT		m_uLowIDUploadCallBack; // NEO: LUC - [LowIDUploadCallBack]
	static bool		m_bReuseTCPPort; // NEO: RTP - [ReuseTCPPort]
#endif //NATTUNNELING // NEO: NATT END

	// NEO: NPT - [NeoPartTraffic]
	static UINT		m_uUsePartTraffic;
	static int		m_iPartTrafficCollors;
	// NEO: NPT END
	static bool		m_bUseClassicShareStatusBar; // NEO: MOD - [ClassicShareStatusBar]
	static bool		m_bUseShowSharePermissions; // NEO: SSP - [ShowSharePermissions]
	
	// NEO: SRS - [SmartReleaseSharing]
	static int		m_iReleaseChunks;
	static UINT		m_uReleaseSlotLimit;
	static int		m_iReleaseSlotLimit;
	// NEO: SRS END

	// NEO: NMFS - [NiceMultiFriendSlots]
	static UINT		m_uFriendSlotLimit;
	static int		m_iFriendSlotLimit;
	// NEO: NMFS END

	static bool		m_bSaveUploadQueueWaitTime; // NEO: SQ - [SaveUploadQueue]
	static bool		m_bUseMultiQueue; // NEO: MQ - [MultiQueue]
	static UINT		m_uUseRandomQueue; // NEO: RQ - [RandomQueue]

	static bool		m_bNeoScoreSystem; // NEO: NFS - [NeoScoreSystem]
	static bool		m_bNeoCreditSystem;  // NEO: NCS - [NeoCreditSystem]
	static int		m_iOtherCreditSystem; // NEO: OCS - [OtherCreditSystems]

	// NEO: TQ - [TweakUploadQueue]
	static bool		m_bUseInfiniteQueue;

	static UINT		m_uQueueOverFlowDef;
	static int		m_iQueueOverFlowDef;
	static UINT		m_uQueueOverFlowEx;
	static int		m_iQueueOverFlowEx;
	static UINT		m_uQueueOverFlowRelease;
	static int		m_iQueueOverFlowRelease;
	static UINT		m_uQueueOverFlowCF;
	static int		m_iQueueOverFlowCF;
	// NEO: TQ END

	// NEO: PRSF - [PushSmallRareFiles]
	static bool		m_bPushSmallFiles;
	static int		m_iPushSmallFilesSize;
	static bool		m_bPushRareFiles;
	static int		m_iPushRareFilesValue;
	static bool		m_bPushRatioFiles;
	static int		m_iPushRatioFilesValue;
	// NEO: PRSF END


	// NEO: NXC - [NewExtendedCategories]
	static	bool	m_bShowCatNames;
	static	UINT	m_uShowCategoryFlags;
	static	bool	m_bSelCatOnAdd;
	static	bool	m_bActiveCatDefault;
	static	bool	m_bAutoSetResumeOrder;
	static	bool	m_bSmallFileDLPush;
	static	int 	m_iSmallFileDLPush;
	static	bool	m_bStartDLInEmptyCats; 
	static	int 	m_iStartDLInEmptyCats; // 0 = disabled, otherwise num to resume
	static	bool	m_bUseAutoCat;
	static	bool	m_bCheckAlreadyDownloaded;

	static	bool	m_bStartNextFileByPriority;

	static	bool	m_bSmartA4AFSwapping; // only for NNP swaps and file completes, stops, cancels, etc.
	static	int 	m_iAdvancedA4AFMode; // 0 = disabled, 1 = balance, 2 = stack -- controls the balancing routines for on queue sources
	// NEO: NXC END

	// NEO: NTB - [NeoToolbarButtons]
	static	UINT	m_uNeoToolbar;
	static	int		m_iNeoToolbarButtonCount;
	static  CArray<UINT> m_NeoToolbarButtons;
	// NEO: NTB END

#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase]
	static	bool	m_bEnableSourceList;
	static	int		m_iTableAmountToStore;
	// cleanup
	static	UINT	m_uSourceListRunTimeCleanUp;
	static	int		m_iSourceListExpirationTime;
	// NEO: SFL - [SourceFileList]
	// seen files
	static	bool	m_bSaveSourceFileList;
	static	int		m_iFileListExpirationTime;
	// NEO: SFL END
	// security
	static	bool	m_bUseIPZoneCheck;
	static	bool	m_bSourceHashMonitor; // NEO: SHM - [SourceHashMonitor]
	// aditional obtions
	static	int		m_iIgnoreUndefinedIntervall;
	static	int		m_iIgnoreUnreachableInterval;
#endif // NEO_CD // NEO: NCD END
#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	static	bool	m_bEnableSourceAnalizer;
	static	int		m_iAnalyzeIntervals;
	static	int		m_iTableAmountToAnalyze;
	// gap handling
	static	bool	m_bHandleTableGaps;
	static	int		m_fPriorityGapRatio;
	static	int		m_iMaxGapSize;
	static	int		m_iMaxGapTime;
	// analyze obtions
	static	int		m_fMaxMidleDiscrepanceHigh;
	static	int		m_fMaxMidleDiscrepanceLow;
	static	bool	m_bDualLinkedTableGravity;
	static	int		m_iDualLinkedTableGravity;
	// calculation obtions
	static	int		m_fEnhancedFactor;
	static	int		m_iFreshSourceTreshold;
	static	int		m_iTempralIPBorderLine;
	// additional obtions
	static	int		m_fLastSeenDurationThreshold;
	static	bool	m_bLinkTimePropability;
	static	int		m_iLinkTimeThreshold;
	static	bool	m_bScaleReliableTime;
	static	int		m_iMaxReliableTime;
#endif // NEO_SA // NEO: NSA END

#ifdef ARGOS // NEO: NA - [NeoArgos] 
	static	bool	m_bZeroScoreGPLBreaker;
	static	int		m_iBanTime;
	static	bool	m_bCloseMaellaBackdoor;

	// DLP Groupe
	static	bool	m_bLeecherModDetection;
	static	bool	m_bLeecherNickDetection;
	static	bool	m_bLeecherHashDetection;
	static	int		m_iDetectionLevel;

	// Behavioural groupe
	static	UINT	m_uAgressionDetection;
	static	UINT	m_uHashChangeDetection;

	static	bool	m_bUploadFakerDetection;
	static	bool	m_bFileFakerDetection;
	static	bool	m_bRankFloodDetection;
	static	bool	m_bXsExploitDetection;
	static	bool	m_bFileScannerDetection;
	static	UINT	m_uSpamerDetection;

	static	bool	m_bHashThiefDetection;
	static	UINT	m_uNickThiefDetection;
	static	bool	m_bModThiefDetection;
#endif //ARGOS // NEO: NA END

#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry]
	static	int		m_iIP2CountryNameMode; 
	static	UINT	m_uIP2CountryShowFlag; 
#endif // IP2COUNTRY // NEO: IP2C END

	// NEO: NMX - [NeoMenuXP]
	static	bool	m_bShowXPSideBar;
	static	bool	m_bShowXPBitmap;
	static  int 	m_iXPMenuStyle;
	static	bool	m_bGrayMenuIcon;
	// NEO: NMX END

	static CKnownPreferences KnownPrefs;
	static CPartPreferences PartPrefs;

	static	DWORD	GetAppPriority()				{return m_dAppPriority;} // NEO: MOD - [AppPriority]
	// NEO: MTD - [MultiTempDirectories]
	static  int		GetUsedTempDir()				{return m_iPreferredTempDir;}
	static  void	SetUsedTempDir(int iTDir)		{m_iPreferredTempDir = iTDir;}
	// NEO: MTD END
	static	bool	IsPauseOnFileComplete()			{return m_bPauseOnFileComplete;} // NEO: POFC - [PauseOnFileComplete]

	static  UINT	UseIncompletePartStatus()		{return m_uIncompletePartStatus;}	// NEO: ICS - [InteligentChunkSelection] 
	static  UINT	UseSubChunkTransfer()			{return m_uSubChunkTransfer;} // NEO: SCT - [SubChunkTransfer]
	static  bool	UsePartStatusHistory()			{return m_bPartStatusHistory;} // NEO: PSH - [PartStatusHistory]
	static  bool	UseLowID2HighIDAutoCallback()	{return m_bLowID2HighIDAutoCallback;} // NEO: L2HAC - [LowID2HighIDAutoCallback]
	static  bool	UseSaveComments()				{return m_bSaveComments;} // NEO: XCs - [SaveComments]
	static  bool	UseKnownComments()				{return m_bKnownComments;} // NEO: XCk - [KnownComments]	
	
	static  bool	IsPreferShareAll()				{return m_bPreferShareAll;} // NEO: PSA - [PreferShareAll]
	static  UINT	UseLugdunumCredits()			{return m_uLugdunumCredits;} // NEO: KLC - [KhaosLugdunumCredits]
	static  bool	IsDontRemoveStaticServers()		{return m_bDontRemoveStaticServers;} // NEO: MOD - [DontRemoveStaticServers]

	static  bool	ShowBanner()					{return m_bShowBanner;} // NEO: NPB - [PrefsBanner]
	static	bool	CollorShareFiles()				{return m_bCollorShareFiles;} // NEO: NSC - [NeoSharedCategories]
	static	bool	SmoothStatisticsGraphs()		{return m_bSmoothStatisticsGraphs;} // NEO: NBC - [NeoBandwidthControl] 

	static	UINT	DisableAutoSort()				{return m_uDisableAutoSort;} // NEO: SE - [SortExtension]

	// NEO: NSTI - [NewSystemTrayIcon]
	static	bool	IsShowSystemTrayUpload()		{return m_bShowSystemTrayUpload;}
	static	bool	UseThinSystemTrayBars()			{return m_bThinSystemTrayBars;}
	static	int		GetTrayBarsMaxCollor()			{return m_iTrayBarsMaxCollor;}
	// NEO: NSTI END

	static	bool	UseStaticTrayIcon()				{return m_bStaticTrayIcon;} // NEO: STI - [StaticTray]

	// NEO: IM - [InvisibelMode]
	static	bool	GetInvisibleMode()				{return m_bInvisibleMode;}
	static	UINT	GetInvisibleModeHKKeyModifier()	{return m_iInvisibleModeHotKeyModifier;}
	static	TCHAR	GetInvisibleModeHKKey()			{return m_cInvisibleModeHotKey;}
	// NEO: IM END

	// NEO: TPP - [TrayPasswordProtection]
	static	bool	IsTrayPasswordProtection()		{return m_bTrayPasswordProtection;}
	static	CString	GetTrayPassword()				{return m_sTrayPassword;}
	// NEO: TPP END

	static	bool	UsePlusSpeedMeter()				{return m_bUsePlusSpeedMeter;} // NEO: PSM - [PlusSpeedMeter]

	static	bool	UseRelativeChunkDisplay()		{return m_bUseRelativeChunkDisplay;} // NEO: MOD - [RelativeChunkDisplay]

	// NEO: SI - [SysInfo]
	static	bool	DrawSysInfoGraph()				{return m_bDrawSysInfoGraph;} 
	static	bool	ShowSysInfoOnTitle()			{return m_bShowSysInfoOnTitle;} 
	// NEO: SI END
	
	static	bool	UseChunkDots()					{return m_bUseChunkDots;} 	// NEO: MOD - [ChunkDots]
	static	bool	UseTreeStyle()					{return m_bUseTreeStyle;} 	// NEO: NTS - [NeoTreeStyle]

	static	bool	ShowClientPercentage()			{return m_bShowClientPercentage;} // NEO: MOD - [Percentage]

	// NEO: MOD - [RefreshShared]
	static	bool	IsRefreshShared()				{return m_bRefreshShared;}
	static	int		GetRefreshShared()				{return m_iRefreshSharedIntervals;}
	static	uint32	GetRefreshSharedMs()			{return MIN2MS(GetRefreshShared());}
	// NEO: MOD END

	// NEO: OCF - [OnlyCompleetFiles]
	static  bool	OnlyCompleteFiles()				{return m_bOnlyCompleteFiles;}
	static  int		GetToOldComplete()				{return m_iToOldComplete;}
	// NEO: OCF END
	
	// NEO: OCC - [ObelixConnectionControl]
	static  bool	UseConnectionControl()			{return m_bConnectionControl;}
	static  int		GetConnectionControlValue()		{return m_iConnectionControlValue;}
	// NEO: OCC END

	// NEO: SCM - [SmartConnectionManagement]
	static	bool	IsManageConnections()			{return m_bManageConnections;} 
	static	void	SetManageConnections(bool manage){m_bManageConnections = manage;}
	static	float	GetManageConnectionsFactor()	{return m_fManageConnectionsFactor;}
	static	void	SetManageConnectionsFactor(float Factor){m_fManageConnectionsFactor = Factor;}
	// NEO: SCM END

	// NEO: NCC - [NeoConnectionChecker]
	static	bool	UseAutoConnectionChecker()		{return m_bAutoConnectionChecker;}
	static	int		GetAutoConnectionCheckerValue()	{return m_iAutoConnectionCheckerValue;}
	// NEO: NCC END

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
	static	bool	IsNAFCEnabled()					{return I2B(m_uNAFCEnabled);}
	static	bool	IsCheckAdapter()				{return (m_uNAFCEnabled == 1 && m_bISPCustomIP);} // The default adapter is heare useles
	static	bool	IsISPCustomIP()					{return m_bISPCustomIP;}
	static	DWORD	GetISPZone()					{return m_uISPZone;}
	static	DWORD	GetISPMask()					{return m_uISPMask;}
	static	bool	IsBindToAdapter()				{return m_bBindToAdapter;}

	static	bool	IsCheckConnection()				{return m_bCheckConnection;}
	static	bool	IsStaticLowestPing()			{return m_bStaticLowestPing;}
	static	int		GetPingMode()					{return m_iPingMode;}
	static	bool	IsNoTTL()						{return m_bNoTTL;}
	static	bool	IsAutoHostToPing()				{return (!m_bManualHostToPing);}
	static	CString GetURLPing()					{return m_sPingServer;}

	static	bool	IsAutoMSS()						{return m_bAutoMSS;}
	static	int		GetMSS()						{return m_iMSS;}

	static	bool	UseDoubleSendSize()				{return m_bUseDoubleSendSize;}

	static	bool	UseDownloadBandwidthThrottler()	{return m_bUseDownloadBandwidthThrottler;}
	static	UINT	UseHyperDownload()				{return m_uUseHyperDownload;}
	static	int		GetBCTimeDown()					{return m_iBCTimeDown;}
	static	int		GetBCPriorityDown()				{return m_iBCPriorityDown;}
	static	UINT	IsSetDownloadBuffer()			{return m_uSetDownloadBuffer;}
	static	int		GetDownloadBufferSize()			{return m_iDownloadBufferSize;}

	static	UINT	UseBlockedQueue()				{return m_uUseBlockedQueue;}
	static	int		GetBCTimeUp()					{return m_iBCTimeUp;}
	static	int		GetBCPriorityUp()				{return m_iBCPriorityUp;}
	static	UINT	IsSetUploadBuffer()				{return m_uSetUploadBuffer;}
	static	int		GetUploadBufferSize()			{return m_iUploadBufferSize;}
	
	static	int		GetDatarateSamples()			{return m_iDatarateSamples;}

	static	bool	IsIncludeOverhead()				{return m_bIncludeOverhead;}
	static	void	SetIncludeOverhead(bool include){m_bIncludeOverhead = include;}
	static	bool	IsIncludeTCPAck()				{return m_bIncludeTCPAck;}
	static	bool	IsConnectionsOverHead()			{return m_bConnectionsOverHead;}

	static	bool	IsSessionRatio()				{return m_bSessionRatio;}
	
	static	void	SetDownloadControl(int ctrl)	{m_iDownloadControl = ctrl;}
	static	bool	IsNAFCDownload()				{return (m_iDownloadControl == 1);}
	static	bool	IsDSSEnabled()					{return (m_iDownloadControl == 2);}
	static	int		GetDownloadControl()			{return m_iDownloadControl;}
	static	float	GetMinBCDownload()				{return m_fMinBCDownload;}
	static	void	SetMinBCDownload(float down)	{m_fMinBCDownload = down;}
	static	float	GetMaxBCDownload()				{return m_fMaxBCDownload;}
	static	void	SetMaxBCDownload(float down)	{m_fMaxBCDownload = down;}
	static	bool	IsAutoMaxDownload()				{return (GetMaxBCDownload() == 0);}


	static	void	SetUploadControl(int ctrl)		{m_iUploadControl = ctrl;}
	static	bool	IsNAFCUpload()					{return (m_iUploadControl == 1);}
	static	bool	IsUSSEnabled()					{return (m_iUploadControl == 2);}
	static	int		GetUploadControl()				{return m_iUploadControl;}
	static	float	GetMinBCUpload()				{return m_fMinBCUpload;}
	static	void	SetMinBCUpload(float up)		{m_fMinBCUpload = up;}
	static	float	GetMaxBCUpload()				{return m_fMaxBCUpload;}
	static	void	SetMaxBCUpload(float up)		{m_fMaxBCUpload = up;}
	static	bool	IsAutoMaxUpload()				{return (GetMaxBCUpload() == 0);}

	static	void	SetMaxDownStream(float maxdown)	{m_fMaxDownStream = maxdown;}
	static	float	GetMaxDownStream()				{return m_fMaxDownStream;}
	static	void	SetMaxUpStream(float maxup)		{m_fMaxUpStream = maxup;}
	static	float	GetMaxUpStream()				{return m_fMaxUpStream;}

	static	bool	IsMinimizeOpenedSlots()			{return m_bMinimizeOpenedSlots;}
	static	UINT	IsCumulateBandwidth()			{return m_uCumulateBandwidth;}
	static	UINT	IsBadwolfsUpload()				{return m_uBadwolfsUpload;}

	static	int		GetMaxReservedSlots()			{return m_iMaxReservedSlots;}

	static	bool	IsIncreaseTrickleSpeed()		{return m_bIncreaseTrickleSpeed;}
	static	float	GetIncreaseTrickleSpeed()		{return m_fIncreaseTrickleSpeed;}

 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
	static	UINT	IsSeparateReleaseBandwidth()	{return m_uSeparateReleaseBandwidth;}
	static	float	GetReleaseSlotSpeed()			{return m_fReleaseSlotSpeed;}
	static	float	GetReleaseBandwidthPercentage()	{return m_fReleaseBandwidthPercentage;}

	static	UINT	IsSeparateFriendBandwidth()		{return m_uSeparateFriendBandwidth;}
	static	float	GetFriendSlotSpeed()			{return m_fFriendSlotSpeed;}
	static	float	GetFriendBandwidthPercentage()	{return m_fFriendBandwidthPercentage;}
 #endif // BW_MOD // NEO: BM END

	static int		GetMaxUploadSlots()				{return m_iMaxUploadSlots;}
	static int		GetMinUploadSlots()				{return m_iMinUploadSlots;}
	static float	GetUploadPerSlots()				{return m_fUploadPerSlots;}
	static uint32	GetUploadPerSlotsB()			{return (uint32)KB2B(GetUploadPerSlots());}
	static uint32	GetUploadPerSlotsDrB()			{return (uint32)KB2B((GetUploadPerSlots()*2)/3);}

	static	bool	IsOpenMoreSlotsWhenNeeded()		{return m_bOpenMoreSlotsWhenNeeded;}
	static	bool	IsCheckSlotDatarate()			{return m_bCheckSlotDatarate;}

	static	bool	IsTrickleBlocking()				{return m_bIsTrickleBlocking;}
	static	bool	IsDropBlocking()				{return m_bIsDropBlocking;}

	static	bool	IsDynUpGoingDivider()			{return m_bDynUpGoingDivider;}
	static	int		GetDynUpGoingUpDivider()		{return m_iDynUpGoingUpDivider;}
	static	int		GetDynUpGoingDownDivider()		{return m_iDynUpGoingDownDivider;}

	static	int		GetUpMaxPingMethod()			{return m_iUpMaxPingMethod;}
	static	int		GetBasePingUp()					{return m_iBasePingUp;}
	static	void	SetBasePingUp(int ping)			{m_iBasePingUp = ping;}
	static	int		GetPingUpTolerance()			{return m_iPingUpTolerance;}
	static	void	SetPingUpTolerance(int tolerance){m_iPingUpTolerance = tolerance;}
	static	int		GetPingUpProzent()				{return m_iPingUpProzent;}
	static	void	SetPingUpProzent(int prozent)	{m_iPingUpProzent = prozent;}


	static	bool	IsDynDownGoingDivider()			{return m_bDynDownGoingDivider;}
	static	int		GetDynDownGoingUpDivider()		{return m_iDynDownGoingUpDivider;}
	static	int		GetDynDownGoingDownDivider()	{return m_iDynDownGoingDownDivider;}
	
	static	int		GetDownMaxPingMethod()			{return m_iDownMaxPingMethod;}
	static	int		GetBasePingDown()				{return m_iBasePingDown;}
	static	void	SetBasePingDown(int ping)		{m_iBasePingDown = ping;}
	static	int		GetPingDownTolerance()			{return m_iPingDownTolerance;}
	static	void	SetPingDownTolerance(int tolerance){m_iPingDownTolerance = tolerance;}
	static	int		GetPingDownProzent()			{return m_iPingDownProzent;}
	static	void	SetPingDownProzent(int prozent)	{m_iPingDownProzent = prozent;}
#endif // NEO_BC // NEO: NBC END

	static	bool	IsTCPDisableNagle()				{return m_bTCPDisableNagle;}

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
 #ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	static	bool	IsLanSupportEnabled()			{return IsLancastEnabled() || IsVoodooEnabled();}
 #else
	static	bool	IsLanSupportEnabled()			{return IsLancastEnabled();}
 #endif // VOODOO // NEO: VOODOO END

	static	bool	IsLancastEnabled()				{return m_bLancastEnabled;}

	static	uint16	GetMaxLanDownload()				{return m_iMaxLanDownload ? m_iMaxLanDownload : UNLIMITED;}
	static	bool	IsSetLanDownloadBuffer()		{return m_bSetLanDownloadBuffer;}
	static	int		GetLanDownloadBufferSize()		{return m_iLanDownloadBufferSize;}

	static	uint16	GetMaxLanUpload()				{return m_iMaxLanUpload ? m_iMaxLanUpload : UNLIMITED;}
	static	bool	IsSetLanUploadBuffer()			{return m_bSetLanUploadBuffer;}
	static	int		GetLanUploadBufferSize()		{return m_iLanUploadBufferSize;}

	static	int		GetMaxLanUploadSlots()			{return m_iMaxLanUploadSlots;}

	static	bool	IsCustomizedLanCast()			{return m_bCustomizedLanCast;}
	static	CString	GetLanCastGroup();
	static	uint16	GetLanCastPort();

	static	bool	IsCustomLanCastAdapter()		{return m_bCustomLanCastAdapter;}
	static	DWORD	GetLanCastAdapterIPAdress()		{return m_uLanCastAdapterIPAdress;}
	static	DWORD	GetLanCastAdapterSubNet()		{return m_uLanCastAdapterSubNet;}

	static	bool	IsAutoBroadcastLanFiles()		{return m_bAutoBroadcastLanFiles;}
	static	int		GetAutoBroadcastLanFiles()		{return m_iAutoBroadcastLanFiles;}
	static	uint32	GetAutoBroadcastLanFilesMs()	{return SEC2MS(GetAutoBroadcastLanFiles());}

	static	bool	UseLanMultiTransfer()			{return m_bUseLanMultiTransfer;}
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	static	bool	IsVoodooEnabled()				{return UseVoodooTransfer() || IsSlaveAllowed();} // Voodoo generaly enabled
	static	bool	IsVoodooAllowed()				{return IsSlaveAllowed() || IsSlaveHosting();} // Allow incomming voodoo requests
	static	bool	UseVoodooTransfer()				{return m_bUseVoodooTransfer;} // use voodoo download
	static	bool	IsSlaveAllowed()				{return m_bSlaveAllowed;} // Allow using this clinet as slave
	static	bool	IsSlaveHosting()				{return m_bSlaveHosting;} // Allow slave to connect this master

	static	CString	GetVoodooSpell()				{return m_sVoodooSpell;} // voodoo spell, somethink like password
	static	uint16	GetVoodooPort()					{return m_nVoodooPort;} // Voodoo Port

	static	UINT	IsAutoConnectVoodoo()			{return m_uAutoConnectVoodoo;} // Connect to Voodl clinets ftom list
	static	int		GetVoodooReconectTime()			{return m_iVoodooReconectTime;} // retry connection in this intervals
	static	uint32	GetVoodooReconectTimeMs()		{return SEC2MS(GetVoodooReconectTime());}

	static	UINT	UseVirtualVoodooFiles()			{return m_uUseVirtualVoodooFiles;} // dont create local copies of the voodoo data file

	static	bool	UseVoodooSearch(bool bSlaveSide = false) {return IsVoodooEnabled() && (!bSlaveSide || m_bSlaveAllowed); } // voodoo search forwarding

	static	UINT	IsVoodooCastEnabled()			{if(!IsVoodooAllowed()) return FALSE; return m_uVoodooCastEnabled;} // Enable Voofo lancast extension
	static	UINT	SearchForSlaves()				{if(!IsSlaveAllowed()) return FALSE; return m_uSearchForSlaves;} // Search lan for slavs
	static	UINT	SearchForMaster()				{if(!IsSlaveHosting()) return FALSE; return m_uSearchForMaster;} // Search lan for master
	static	int		VoodooSearchIntervals()			{return m_iVoodooSearchIntervals;}
	static	uint32	VoodooSearchIntervalsMs()		{return SEC2MS(VoodooSearchIntervals());}

	static	bool	IsHideVoodooFiles()				{return m_bHideVoodooFiles;} // NEO: PP - [PasswordProtection]
#endif // VOODOO // NEO: VOODOO END

	// NEO: QS - [QuickStart]
	static	UINT	UseQuickStart()					{return m_uQuickStart;}
	static	int		GetQuickStartMaxTime()			{return m_iQuickStartTime;}
	static	int		GetQuickStartMaxTimeMs()		{return MIN2MS(m_iQuickStartTime);}
	static	int		GetQuickStartTimePerFile()		{return m_iQuickStartTimePerFile;}
	static	int		GetQuickStartTimePerFileMs()	{return SEC2MS(m_iQuickStartTimePerFile);}

	static	bool	OnQuickStart()					{return m_bOnQuickStart;}
	static	void	SetOnQuickStart(bool in)		{m_bOnQuickStart = in;}
	// NEO: QS END

	// NEO: RIC - [ReaskOnIDChange]
	static	UINT	IsCheckIPChange()				{return m_uCheckIPChange;}
	static	bool	IsInformOnIPChange()			{return m_bInformOnIPChange;}
	static	bool	IsInformOnBuddyChange()			{return IsInformOnIPChange() && m_bInformOnBuddyChange;}
	static	bool	IsReAskOnIPChange()				{return m_bReAskOnIPChange;}
	static	bool	UseQuickStartOnIPChange()		{return m_bQuickStartOnIPChange;} // NEO: QS - [QuickStart]
	static	bool	IsCheckL2HIDChange()			{return m_bCheckL2HIDChange;}
	static	bool	IsReconnectKadOnIPChange()		{return m_bReconnectKadOnIPChange;}
	static  bool	IsRebindSocketsOnIPChange()		{return m_bRebindSocketsOnIPChange;}
	// NEO: RIC END

	// NEO: RKF - [RecheckKadFirewalled]
	static	bool	IsRecheckKadFirewalled()		{return m_bRecheckKadFirewalled;}
	static	int		GetRecheckKadFirewalled()		{return m_iRecheckKadFirewalled;}
	static	uint32	GetRecheckKadFirewalledMs()		{return MIN2MS(GetRecheckKadFirewalled());}
	// NEO: RKF END

	// NEO: RLD - [ReconnectOnLowID]
	static bool		ReConnectOnLowID()				{return m_bReConnectOnLowID;}
	static int		GetLowIDRetrys()				{return m_iReConnectOnLowID;}
	// NEO: RLD END

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
	static bool		IsNatTraversalEnabled()		{return m_bNatTraversalEnabled;} 
	static UINT		UseLowIDUploadCallBack()	{return m_uLowIDUploadCallBack;} // NEO: LUC - [LowIDUploadCallBack]
	static bool		ReuseTCPPort()				{return m_bReuseTCPPort;} // NEO: RTP - [ReuseTCPPort]
#endif //NATTUNNELING // NEO: NATT END

	// NEO: NPT - [NeoPartTraffic]
	static UINT		UsePartTraffic()			{return m_uUsePartTraffic;}
	static int		GetPartTrafficCollors()		{return m_iPartTrafficCollors;}
	// NEO: NPT END
	static bool		UseClassicShareStatusBar()	{return m_bUseClassicShareStatusBar;} // NEO: MOD - [ClassicShareStatusBar]
	static bool		UseShowSharePermissions()	{return m_bUseShowSharePermissions;} // NEO: SSP - [ShowSharePermissions]
	
	// NEO: SRS - [SmartReleaseSharing]
	static int		GetReleaseChunks()			{return m_iReleaseChunks;}
	static UINT		IsReleaseSlotLimit()		{return m_uReleaseSlotLimit;}
	static int		GetReleaseSlotLimit()		{return m_iReleaseSlotLimit;}
	// NEO: SRS END

	// NEO: NMFS - [NiceMultiFriendSlots]
	static UINT		IsFriendSlotLimit()			{return m_uFriendSlotLimit;}
	static int		GetFriendSlotLimit()		{return m_iFriendSlotLimit;}
	// NEO: NMFS END

	static bool		SaveUploadQueueWaitTime()	{return m_bSaveUploadQueueWaitTime;} // NEO: SQ - [SaveUploadQueue]
	static bool		UseMultiQueue()				{return m_bUseMultiQueue;} // NEO: MQ - [MultiQueue]
	static UINT		UseRandomQueue()			{return m_uUseRandomQueue;} // NEO: RQ - [RandomQueue]

	static	bool	UseNeoScoreSystem()			{return m_bNeoScoreSystem;} // NEO: NFS - [NeoScoreSystem]
	static	bool	UseNeoCreditSystem()		{return m_bNeoCreditSystem;} // NEO: NCS - [NeoCreditSystem]
	static	int		GetCreditSystem()			{return m_iOtherCreditSystem;} // NEO: OCS - [OtherCreditSystems]

	// NEO: TQ - [TweakUploadQueue]
	static bool		UseInfiniteQueue()			{return m_bUseInfiniteQueue;}

	static UINT		IsQueueOverFlowDef()		{return m_uQueueOverFlowDef;}
	static int		GetQueueOverFlowDef()		{return m_iQueueOverFlowDef;}
	static UINT		IsQueueOverFlowEx()			{return m_uQueueOverFlowEx;}
	static int		GetQueueOverFlowEx()		{return m_iQueueOverFlowEx;}
	static UINT		IsQueueOverFlowRelease()	{return m_uQueueOverFlowRelease;}
	static int		GetQueueOverFlowRelease()	{return m_iQueueOverFlowRelease;}
	static UINT		IsQueueOverFlowCF()			{return m_uQueueOverFlowCF;}
	static int		GetQueueOverFlowCF()		{return m_iQueueOverFlowCF;}
	// NEO: TQ END

	// NEO: PRSF - [PushSmallRareFiles]
	static bool		IsPushSmallFiles()			{return m_bPushSmallFiles;}
	static int		GetPushSmallFilesSize()		{return m_iPushSmallFilesSize;}
	static bool		IsPushRareFiles()			{return m_bPushRareFiles;}
	static int		GetPushRareFilesValue()		{return m_iPushRareFilesValue;}
	static bool		IsPushRatioFiles()			{return m_bPushRatioFiles;}
	static int		GetPushRatioFilesValue()	{return m_iPushRatioFilesValue;}
	// NEO: PRSF END

	// NEO: NXC - [NewExtendedCategories]
	static	bool	ShowCatNameInDownList()			{ return m_bShowCatNames; }
	static	UINT	ShowCategoryFlags()				{ return m_uShowCategoryFlags; }
	static	bool	SelectCatForNewDL()				{ return m_bSelCatOnAdd; }
	static	bool	UseActiveCatForLinks()			{ return m_bActiveCatDefault; }
	static	bool	AutoSetResumeOrder()			{ return m_bAutoSetResumeOrder; }
	static	bool	SmallFileDLPush()				{ return m_bSmallFileDLPush; }
	static	int 	SmallFileDLPushSize()			{ return m_iSmallFileDLPush; }
	static	EMFileSize 	SmallFileDLPushSizeB()		{ return KB2B((uint64)SmallFileDLPushSize()); }
	static	bool	StartDLInEmptyCats()			{ return m_bStartDLInEmptyCats; } 
	static	int 	StartDLInEmptyCatsAmount()		{ return m_iStartDLInEmptyCats; } // 0 = disabled, otherwise num to resume
	static	bool	UseAutoCat()					{ return m_bUseAutoCat; }
	static	bool	CheckAlreadyDownloaded()		{ return m_bCheckAlreadyDownloaded; }

	static	bool	IsStartNextFileByPriority()		{ return m_bStartNextFileByPriority; }

	static	bool	UseSmartA4AFSwapping()			{ return m_bSmartA4AFSwapping; } // only for NNP swaps and file completes, stops, cancels, etc.
	static	int 	AdvancedA4AFMode()				{ return m_iAdvancedA4AFMode; } // 0 = disabled, 1 = balance, 2 = stack -- controls the balancing routines for on queue sources
	// NEO: NXC END
	
	// NEO: NTB - [NeoToolbarButtons]
	static	UINT	UseNeoToolbar()					{return m_uNeoToolbar;} 
	static	int		GetNeoToolbarButtonCount()		{return m_iNeoToolbarButtonCount;} 
	static	UINT	GetNeoToolbarButton(int btn)	{if(btn<m_NeoToolbarButtons.GetCount()) return m_NeoToolbarButtons[btn]; else return 0;} 
	// NEO: NTB END

#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase]
	static	bool	EnableSourceList()				{return m_bEnableSourceList;}
	static	int		GetTableAmountToStore()			{return m_iTableAmountToStore;}
	// cleanup
	static	UINT	IsSourceListRunTimeCleanUp()	{return m_uSourceListRunTimeCleanUp;}
	static	int		GetSourceListExpirationTime()	{return m_iSourceListExpirationTime;}
	// NEO: SFL - [SourceFileList]
	// seen files
	static	bool	SaveSourceFileList()			{return m_bSaveSourceFileList && EnableSourceList();}
	static	int		GetFileListExpirationTime()		{return m_iFileListExpirationTime;}
	// NEO: SFL END
	// security
	static	bool	UseIPZoneCheck()				{return m_bUseIPZoneCheck;}
	static	bool	UseSourceHashMonitor()			{return m_bSourceHashMonitor /*&& EnableSourceList()*/;} // NEO: SHM - [SourceHashMonitor]
	// aditional obtions
	static	int		GetIgnoreUndefinedIntervall()	{return m_iIgnoreUndefinedIntervall;}
	static	int		GetIgnoreUnreachableInterval()	{return m_iIgnoreUnreachableInterval;}
#endif // NEO_CD // NEO: NCD END
#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	static	bool	EnableSourceAnalizer()			{return m_bEnableSourceAnalizer && EnableSourceList();}
	static	int		GetAnalyzeIntervals()			{return m_iAnalyzeIntervals;}
	static	int		GetTableAmountToAnalyze()		{return m_iTableAmountToAnalyze;}
	// gap handling
	static	bool	HandleTableGaps()				{return m_bHandleTableGaps;}
	static	int		GetPriorityGapRatio()			{return m_fPriorityGapRatio;}
	static	int		GetMaxGapSize()					{return m_iMaxGapSize;}
	static	int		GetMaxGapTime()					{return m_iMaxGapTime;}
	// analyze obtions
	static	int		GetMaxMidleDiscrepanceHigh()	{return m_fMaxMidleDiscrepanceHigh;}
	static	int		GetMaxMidleDiscrepanceLow()		{return m_fMaxMidleDiscrepanceLow;}
	static	bool	UseDualLinkedTableGravity()		{return m_bDualLinkedTableGravity;}
	static	int		GetDualLinkedTableGravity()		{return m_iDualLinkedTableGravity;}
	// calculation obtions
	static	int		GetEnhancedFactor()				{return m_fEnhancedFactor;}
	static	int		GetFreshSourceTreshold()		{return m_iFreshSourceTreshold;}
	static	int		GetTempralIPBorderLine()		{return m_iTempralIPBorderLine;}
	// additional obtions
	static	int		GetLastSeenDurationThreshold()	{return m_fLastSeenDurationThreshold;}
	static	bool	UseLinkTimePropability()		{return m_bLinkTimePropability;}
	static	int		GetLinkTimeThreshold()			{return m_iLinkTimeThreshold;}
	static	bool	ScaleReliableTime()				{return m_bScaleReliableTime;}
	static	int		GetMaxReliableTime()			{return m_iMaxReliableTime;}
#endif // NEO_SA // NEO: NSA END

#ifdef ARGOS // NEO: NA - [NeoArgos] 
	static	bool	ZeroScoreGPLBreaker()			{return m_bZeroScoreGPLBreaker;}
	static	int		GetBanTime()					{return m_iBanTime;}
	static	uint32	GetBanTimeMs()					{return MIN2MS(GetBanTime());}
	static	bool	CloseMaellaBackdoor()			{return m_bCloseMaellaBackdoor;}

	// DLP Groupe
	static	bool	IsLeecherModDetection()			{return m_bLeecherModDetection;}
	static	bool	IsLeecherNickDetection()		{return m_bLeecherNickDetection;}
	static	bool	IsLeecherHashDetection()		{return m_bLeecherHashDetection;}
	static	bool	UseDLPScanner()					{return IsLeecherModDetection() || IsLeecherNickDetection() || IsLeecherHashDetection();}
	static	int		GetDetectionLevel()				{return m_iDetectionLevel;}

	// Behavioural groupe
	static	UINT	IsAgressionDetection()			{return m_uAgressionDetection;}
	static	UINT	IsHashChangeDetection()			{
 #ifdef NEO_CD // NEO: NCD - [NeoClientDatabase]
														if(m_uHashChangeDetection == 2 && UseSourceHashMonitor())
															return 2;
 #endif // NEO_CD // NEO: NCD END
														return m_uHashChangeDetection != 0;
													}

	static	bool	IsUploadFakerDetection()		{return m_bUploadFakerDetection;}
	static	bool	IsFileFakerDetection()			{return m_bFileFakerDetection;}
	static	bool	IsRankFloodDetection()			{return m_bRankFloodDetection;}
	static	bool	IsXsExploitDetection()			{return m_bXsExploitDetection;}
	static	bool	IsFileScannerDetection()		{return m_bFileScannerDetection;}
	static	UINT	IsSpamerDetection()				{return m_uSpamerDetection;}

	static	bool	IsHashThiefDetection()			{return m_bHashThiefDetection;}
	static	UINT	IsNickThiefDetection()			{return m_uNickThiefDetection;}
	static	bool	IsModThiefDetection()			{return m_bModThiefDetection;}
#endif //ARGOS // NEO: NA END

#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry]
	static	int		GetIP2CountryNameMode()			{return m_iIP2CountryNameMode;}
	static	UINT	IsIP2CountryShowFlag()			{return m_uIP2CountryShowFlag;}
#endif // IP2COUNTRY // NEO: IP2C END

	// NEO: NMX - [NeoMenuXP]
	static	bool	GetXPSideBar()					{return m_bShowXPSideBar;}
	static	bool	GetXPBitmap()					{return	m_bShowXPBitmap;}
	static  int 	GetXPMenuStyle()				{return	m_iXPMenuStyle;}
	static	bool	UseGrayMenuIcons()				{return m_bGrayMenuIcon;}
	// NEO: NMX END

	CNeoPreferences();
	//~CNeoPreferences();
	
	void			Init();
	void			Save();

	friend class CPPgNeo;
	friend class CPPgSources;
	friend class CPPgSourceStorage;
	friend class CPPgNetwork;
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
	friend class CPPgBandwidth;
#endif // NEO_BC // NEO: NBC END
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	friend class CPPgLancast;
#endif //LANCAST // NEO: NLC END
	friend class CPPgInterface;

protected:
	static void LoadNeoPreferences();
	static void SaveNeoPreferences();
	static void CheckNeoPreferences();
};

extern CNeoPreferences NeoPrefs; 
// NEO: NCFG END <-- Xanatos --