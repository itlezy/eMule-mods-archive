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

#include "stdafx.h"
#include <io.h>
#include <share.h>
#include <iphlpapi.h>
#include "emule.h"
#include "Preferences.h"
#include "NeoPreferences.h"
#include "Opcodes.h"
#include "OtherFunctions.h"
#include "Neo/Ini2.h" // NEO: INI - [PlusIniClass] <-- Xanatos --
//#include "Ini2.h"
// NEO: NCFG - [NeoConfiguration] -- Xanatos -->
#include "Neo/functions.h" 
#include "Neo/defaults.h" 
#include "Neo/IniStrings.h" 
// NEO: NCFG END <-- Xanatos --
#include "Neo/NeoOpcodes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// NEO: NCFG - [NeoConfiguration] -- Xanatos -->
CNeoPreferences NeoPrefs; 

CKnownPreferences CNeoPreferences::KnownPrefs;
CPartPreferences CNeoPreferences::PartPrefs;

/* Xanatos:
* All Neo Preferences variables:
*/

DWORD	CNeoPreferences::m_dAppPriority; // NEO: MOD - [AppPriority]
int		CNeoPreferences::m_iPreferredTempDir; // NEO: MTD - [MultiTempDirectories]
bool	CNeoPreferences::m_bPauseOnFileComplete; // NEO: POFC - [PauseOnFileComplete]

UINT	CNeoPreferences::m_uIncompletePartStatus;	// NEO: ICS - [InteligentChunkSelection] 
UINT	CNeoPreferences::m_uSubChunkTransfer; // NEO: SCT - [SubChunkTransfer]
bool	CNeoPreferences::m_bPartStatusHistory; // NEO: PSH - [PartStatusHistory]
bool	CNeoPreferences::m_bLowID2HighIDAutoCallback; // NEO: L2HAC - [LowID2HighIDAutoCallback]
bool	CNeoPreferences::m_bSaveComments; // NEO: XCs - [SaveComments]
bool	CNeoPreferences::m_bKnownComments; // NEO: XCk - [KnownComments]	
	
bool	CNeoPreferences::m_bPreferShareAll; // NEO: PSA - [PreferShareAll]
UINT	CNeoPreferences::m_uLugdunumCredits; // NEO: KLC - [KhaosLugdunumCredits]
bool	CNeoPreferences::m_bDontRemoveStaticServers; // NEO: MOD - [DontRemoveStaticServers]

bool	CNeoPreferences::m_bShowBanner; // NEO: NPB - [PrefsBanner]
bool	CNeoPreferences::m_bCollorShareFiles; // NEO: NSC - [NeoSharedCategories]
bool	CNeoPreferences::m_bSmoothStatisticsGraphs; // NEO: NBC - [NeoBandwidthControl] 

UINT	CNeoPreferences::m_uDisableAutoSort; // NEO: SE - [SortExtension]

// NEO: NSTI - [NewSystemTrayIcon]
bool	CNeoPreferences::m_bShowSystemTrayUpload;
bool	CNeoPreferences::m_bThinSystemTrayBars;
int		CNeoPreferences::m_iTrayBarsMaxCollor;
// NEO: NSTI END

bool	CNeoPreferences::m_bStaticTrayIcon; // NEO: STI - [StaticTray]

// NEO: IM - [InvisibelMode]
bool	CNeoPreferences::m_bInvisibleMode;
UINT	CNeoPreferences::m_iInvisibleModeHotKeyModifier;
TCHAR	CNeoPreferences::m_cInvisibleModeHotKey;
// NEO: IM END

// NEO: TPP - [TrayPasswordProtection]
bool	CNeoPreferences::m_bTrayPasswordProtection;
CString	CNeoPreferences::m_sTrayPassword;
// NEO: TPP END

bool	CNeoPreferences::m_bUsePlusSpeedMeter; // NEO: PSM - [PlusSpeedMeter]

bool	CNeoPreferences::m_bUseRelativeChunkDisplay; // NEO: MOD - [RelativeChunkDisplay]

// NEO: SI - [SysInfo]
bool	CNeoPreferences::m_bDrawSysInfoGraph;
bool	CNeoPreferences::m_bShowSysInfoOnTitle;
// NEO: SI END

bool	CNeoPreferences::m_bUseChunkDots;	// NEO: MOD - [ChunkDots]

bool	CNeoPreferences::m_bUseTreeStyle; 	// NEO: NTS - [NeoTreeStyle]

bool	CNeoPreferences::m_bShowClientPercentage; // NEO: MOD - [Percentage]

// NEO: MOD - [RefreshShared]
bool	CNeoPreferences::m_bRefreshShared;
int		CNeoPreferences::m_iRefreshSharedIntervals;
// NEO: MOD END

// NEO: OCF - [OnlyCompleetFiles]
bool	CNeoPreferences::m_bOnlyCompleteFiles;
int		CNeoPreferences::m_iToOldComplete;
// NEO: OCF END
	
// NEO: OCC - [ObelixConnectionControl]
bool	CNeoPreferences::m_bConnectionControl;
int		CNeoPreferences::m_iConnectionControlValue;
// NEO: OCC END

// NEO: SCM - [SmartConnectionManagement]
bool	CNeoPreferences::m_bManageConnections;
float	CNeoPreferences::m_fManageConnectionsFactor;
// NEO: SCM END

// NEO: NCC - [NeoConnectionChecker]
bool	CNeoPreferences::m_bAutoConnectionChecker;
int		CNeoPreferences::m_iAutoConnectionCheckerValue;
// NEO: NCC END

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
UINT	CNeoPreferences::m_uNAFCEnabled;
bool	CNeoPreferences::m_bISPCustomIP;
DWORD	CNeoPreferences::m_uISPZone;
DWORD	CNeoPreferences::m_uISPMask;
bool	CNeoPreferences::m_bBindToAdapter;


bool	CNeoPreferences::m_bCheckConnection;
bool	CNeoPreferences::m_bStaticLowestPing;
int		CNeoPreferences::m_iPingMode;
bool	CNeoPreferences::m_bNoTTL;
bool	CNeoPreferences::m_bManualHostToPing;
CString CNeoPreferences::m_sPingServer;

bool	CNeoPreferences::m_bAutoMSS;
int		CNeoPreferences::m_iMSS;

bool	CNeoPreferences::m_bUseDoubleSendSize;


bool	CNeoPreferences::m_bUseDownloadBandwidthThrottler;
UINT	CNeoPreferences::m_uUseHyperDownload;
int		CNeoPreferences::m_iBCTimeDown;
int		CNeoPreferences::m_iBCPriorityDown;
UINT	CNeoPreferences::m_uSetDownloadBuffer;
int		CNeoPreferences::m_iDownloadBufferSize;

UINT	CNeoPreferences::m_uUseBlockedQueue;
int		CNeoPreferences::m_iBCTimeUp;
int		CNeoPreferences::m_iBCPriorityUp;
UINT	CNeoPreferences::m_uSetUploadBuffer;
int		CNeoPreferences::m_iUploadBufferSize;

int		CNeoPreferences::m_iDatarateSamples;

bool	CNeoPreferences::m_bIncludeOverhead;
bool	CNeoPreferences::m_bIncludeTCPAck;
bool	CNeoPreferences::m_bConnectionsOverHead;
bool	CNeoPreferences::m_bSessionRatio;


int		CNeoPreferences::m_iDownloadControl;
float	CNeoPreferences::m_fMinBCDownload;
float	CNeoPreferences::m_fMaxBCDownload;


int		CNeoPreferences::m_iUploadControl;
float	CNeoPreferences::m_fMinBCUpload;
float	CNeoPreferences::m_fMaxBCUpload;


float	CNeoPreferences::m_fMaxDownStream;
float	CNeoPreferences::m_fMaxUpStream;


bool	CNeoPreferences::m_bMinimizeOpenedSlots;
UINT	CNeoPreferences::m_uCumulateBandwidth;
UINT	CNeoPreferences::m_uBadwolfsUpload;

int		CNeoPreferences::m_iMaxReservedSlots;

bool	CNeoPreferences::m_bIncreaseTrickleSpeed;
float	CNeoPreferences::m_fIncreaseTrickleSpeed;

 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
UINT	CNeoPreferences::m_uSeparateReleaseBandwidth;
float	CNeoPreferences::m_fReleaseSlotSpeed;
float	CNeoPreferences::m_fReleaseBandwidthPercentage;

UINT	CNeoPreferences::m_uSeparateFriendBandwidth;
float	CNeoPreferences::m_fFriendSlotSpeed;
float	CNeoPreferences::m_fFriendBandwidthPercentage;
 #endif // BW_MOD // NEO: BM END

int		CNeoPreferences::m_iMaxUploadSlots;
int		CNeoPreferences::m_iMinUploadSlots;
float	CNeoPreferences::m_fUploadPerSlots;

bool	CNeoPreferences::m_bOpenMoreSlotsWhenNeeded;
bool	CNeoPreferences::m_bCheckSlotDatarate;

bool	CNeoPreferences::m_bIsTrickleBlocking;
bool	CNeoPreferences::m_bIsDropBlocking;


bool	CNeoPreferences::m_bDynUpGoingDivider;
int		CNeoPreferences::m_iDynUpGoingUpDivider;
int		CNeoPreferences::m_iDynUpGoingDownDivider;

int		CNeoPreferences::m_iUpMaxPingMethod;
int		CNeoPreferences::m_iBasePingUp;
int		CNeoPreferences::m_iPingUpTolerance;
int		CNeoPreferences::m_iPingUpProzent;


bool	CNeoPreferences::m_bDynDownGoingDivider;
int		CNeoPreferences::m_iDynDownGoingUpDivider;
int		CNeoPreferences::m_iDynDownGoingDownDivider;
	
int		CNeoPreferences::m_iDownMaxPingMethod;
int		CNeoPreferences::m_iBasePingDown;
int		CNeoPreferences::m_iPingDownTolerance;
int		CNeoPreferences::m_iPingDownProzent;
#endif // NEO_BC // NEO: NBC END

bool	CNeoPreferences::m_bTCPDisableNagle;

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
bool	CNeoPreferences::m_bLancastEnabled;

uint16	CNeoPreferences::m_iMaxLanDownload;
bool	CNeoPreferences::m_bSetLanDownloadBuffer;
int		CNeoPreferences::m_iLanDownloadBufferSize;

uint16	CNeoPreferences::m_iMaxLanUpload;
bool	CNeoPreferences::m_bSetLanUploadBuffer;
int		CNeoPreferences::m_iLanUploadBufferSize;

int		CNeoPreferences::m_iMaxLanUploadSlots;

bool	CNeoPreferences::m_bCustomizedLanCast;
CString	CNeoPreferences::m_sLanCastGroup;
uint16	CNeoPreferences::m_uLanCastPort;

bool	CNeoPreferences::m_bCustomLanCastAdapter;
DWORD	CNeoPreferences::m_uLanCastAdapterIPAdress;
DWORD	CNeoPreferences::m_uLanCastAdapterSubNet;

bool	CNeoPreferences::m_bAutoBroadcastLanFiles;
int		CNeoPreferences::m_iAutoBroadcastLanFiles;

bool	CNeoPreferences::m_bUseLanMultiTransfer;
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
bool	CNeoPreferences::m_bUseVoodooTransfer;
bool	CNeoPreferences::m_bSlaveAllowed;
bool	CNeoPreferences::m_bSlaveHosting;

CString	CNeoPreferences::m_sVoodooSpell;
uint16	CNeoPreferences::m_nVoodooPort;

UINT	CNeoPreferences::m_uAutoConnectVoodoo;
int		CNeoPreferences::m_iVoodooReconectTime;

UINT	CNeoPreferences::m_uUseVirtualVoodooFiles;

UINT	CNeoPreferences::m_uVoodooCastEnabled;

UINT	CNeoPreferences::m_uSearchForSlaves;
UINT	CNeoPreferences::m_uSearchForMaster;
int		CNeoPreferences::m_iVoodooSearchIntervals;

bool	CNeoPreferences::m_bHideVoodooFiles; // NEO: PP - [PasswordProtection]
#endif // VOODOO // NEO: VOODOO END

// NEO: QS - [QuickStart]
UINT	CNeoPreferences::m_uQuickStart;
int		CNeoPreferences::m_iQuickStartTime;
int		CNeoPreferences::m_iQuickStartTimePerFile;
UINT	CNeoPreferences::m_iQuickMaxConperFive;
UINT	CNeoPreferences::m_iQuickMaxHalfOpen;
UINT	CNeoPreferences::m_iQuickMaxConnections;

bool	CNeoPreferences::m_bOnQuickStart;
// NEO: QS END

// NEO: RIC - [ReaskOnIDChange]
UINT	CNeoPreferences::m_uCheckIPChange;
bool	CNeoPreferences::m_bInformOnIPChange;
bool	CNeoPreferences::m_bInformOnBuddyChange;
bool	CNeoPreferences::m_bReAskOnIPChange;
bool	CNeoPreferences::m_bQuickStartOnIPChange; // NEO: QS - [QuickStart]
bool	CNeoPreferences::m_bCheckL2HIDChange;
bool	CNeoPreferences::m_bReconnectKadOnIPChange;
bool	CNeoPreferences::m_bRebindSocketsOnIPChange;
// NEO: RIC END

// NEO: RLD - [ReconnectOnLowID]
bool	CNeoPreferences::m_bReConnectOnLowID;
int		CNeoPreferences::m_iReConnectOnLowID;
// NEO: RLD END

// NEO: RKF - [RecheckKadFirewalled]
bool	CNeoPreferences::m_bRecheckKadFirewalled;
int		CNeoPreferences::m_iRecheckKadFirewalled;
// NEO: RKF END

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
bool	CNeoPreferences::m_bNatTraversalEnabled;
UINT	CNeoPreferences::m_uLowIDUploadCallBack; // NEO: LUC - [LowIDUploadCallBack]
bool	CNeoPreferences::m_bReuseTCPPort; // NEO: RTP - [ReuseTCPPort]
#endif //NATTUNNELING // NEO: NATT END

// NEO: NPT - [NeoPartTraffic]
UINT	CNeoPreferences::m_uUsePartTraffic;
int		CNeoPreferences::m_iPartTrafficCollors;
// NEO: NPT END
bool	CNeoPreferences::m_bUseClassicShareStatusBar; // NEO: MOD - [ClassicShareStatusBar]
bool	CNeoPreferences::m_bUseShowSharePermissions; // NEO: SSP - [ShowSharePermissions]

// NEO: SRS - [SmartReleaseSharing]
int		CNeoPreferences::m_iReleaseChunks;
UINT	CNeoPreferences::m_uReleaseSlotLimit;
int		CNeoPreferences::m_iReleaseSlotLimit;
// NEO: SRS END

// NEO: NMFS - [NiceMultiFriendSlots]
UINT	CNeoPreferences::m_uFriendSlotLimit;
int		CNeoPreferences::m_iFriendSlotLimit;
// NEO: NMFS END

bool	CNeoPreferences::m_bSaveUploadQueueWaitTime; // NEO: SQ - [SaveUploadQueue]
bool	CNeoPreferences::m_bUseMultiQueue; // NEO: MQ - [MultiQueue]
UINT	CNeoPreferences::m_uUseRandomQueue; // NEO: RQ - [RandomQueue]

bool	CNeoPreferences::m_bNeoScoreSystem; // NEO: NFS - [NeoScoreSystem]
bool	CNeoPreferences::m_bNeoCreditSystem; // NEO: NCS - [NeoCreditSystem]
int		CNeoPreferences::m_iOtherCreditSystem; // NEO: OCS - [OtherCreditSystems]

// NEO: TQ - [TweakUploadQueue]
bool	CNeoPreferences::m_bUseInfiniteQueue;

UINT	CNeoPreferences::m_uQueueOverFlowDef;
int		CNeoPreferences::m_iQueueOverFlowDef;
UINT	CNeoPreferences::m_uQueueOverFlowEx;
int		CNeoPreferences::m_iQueueOverFlowEx;
UINT	CNeoPreferences::m_uQueueOverFlowRelease;
int		CNeoPreferences::m_iQueueOverFlowRelease;
UINT	CNeoPreferences::m_uQueueOverFlowCF;
int		CNeoPreferences::m_iQueueOverFlowCF;
// NEO: TQ END

// NEO: PRSF - [PushSmallRareFiles]
bool	CNeoPreferences::m_bPushSmallFiles;
int		CNeoPreferences::m_iPushSmallFilesSize;
bool	CNeoPreferences::m_bPushRareFiles;
int		CNeoPreferences::m_iPushRareFilesValue;
bool	CNeoPreferences::m_bPushRatioFiles;
int		CNeoPreferences::m_iPushRatioFilesValue;
// NEO: PRSF END

// NEO: NXC - [NewExtendedCategories]
bool	CNeoPreferences::m_bShowCatNames;
UINT	CNeoPreferences::m_uShowCategoryFlags;
bool	CNeoPreferences::m_bSelCatOnAdd;
bool	CNeoPreferences::m_bActiveCatDefault;
bool	CNeoPreferences::m_bAutoSetResumeOrder;
bool	CNeoPreferences::m_bSmallFileDLPush;
int 	CNeoPreferences::m_iSmallFileDLPush;
bool	CNeoPreferences::m_bStartDLInEmptyCats; 
int 	CNeoPreferences::m_iStartDLInEmptyCats; // 0 = disabled, otherwise num to resume
bool	CNeoPreferences::m_bUseAutoCat;
bool	CNeoPreferences::m_bCheckAlreadyDownloaded;

bool	CNeoPreferences::m_bStartNextFileByPriority;

bool	CNeoPreferences::m_bSmartA4AFSwapping; // only for NNP swaps and file completes, stops, cancels, etc.
int 	CNeoPreferences::m_iAdvancedA4AFMode; // 0 = disabled, 1 = balance, 2 = stack -- controls the balancing routines for on queue sources
// NEO: NXC END

// NEO: NTB - [NeoToolbarButtons]
UINT	CNeoPreferences::m_uNeoToolbar;
int		CNeoPreferences::m_iNeoToolbarButtonCount;
CArray<UINT> CNeoPreferences::m_NeoToolbarButtons;
// NEO: NTB END

#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase]
bool	CNeoPreferences::m_bEnableSourceList;
int		CNeoPreferences::m_iTableAmountToStore;
// cleanup
UINT	CNeoPreferences::m_uSourceListRunTimeCleanUp;
int		CNeoPreferences::m_iSourceListExpirationTime;
// NEO: SFL - [SourceFileList]
// seen files
bool	CNeoPreferences::m_bSaveSourceFileList;
int		CNeoPreferences::m_iFileListExpirationTime;
// NEO: SFL END
// security
bool	CNeoPreferences::m_bUseIPZoneCheck;
bool	CNeoPreferences::m_bSourceHashMonitor; // NEO: SHM - [SourceHashMonitor]
// aditional obtions
int		CNeoPreferences::m_iIgnoreUndefinedIntervall;
int		CNeoPreferences::m_iIgnoreUnreachableInterval;
#endif // NEO_CD // NEO: NCD END
#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
bool	CNeoPreferences::m_bEnableSourceAnalizer;
int		CNeoPreferences::m_iAnalyzeIntervals;
int		CNeoPreferences::m_iTableAmountToAnalyze;
// gap handling
bool	CNeoPreferences::m_bHandleTableGaps;
int		CNeoPreferences::m_fPriorityGapRatio;
int		CNeoPreferences::m_iMaxGapSize;
int		CNeoPreferences::m_iMaxGapTime;
// analyze obtions
int		CNeoPreferences::m_fMaxMidleDiscrepanceHigh;
int		CNeoPreferences::m_fMaxMidleDiscrepanceLow;
bool	CNeoPreferences::m_bDualLinkedTableGravity;
int		CNeoPreferences::m_iDualLinkedTableGravity;
// calculation obtions
int		CNeoPreferences::m_fEnhancedFactor;
int		CNeoPreferences::m_iFreshSourceTreshold;
int		CNeoPreferences::m_iTempralIPBorderLine;
// additional obtions
int		CNeoPreferences::m_fLastSeenDurationThreshold;
bool	CNeoPreferences::m_bLinkTimePropability;
int		CNeoPreferences::m_iLinkTimeThreshold;
bool	CNeoPreferences::m_bScaleReliableTime;
int		CNeoPreferences::m_iMaxReliableTime;
#endif // NEO_SA // NEO: NSA END

#ifdef ARGOS // NEO: NA - [NeoArgos] 
bool	CNeoPreferences::m_bZeroScoreGPLBreaker;
int		CNeoPreferences::m_iBanTime;
bool	CNeoPreferences::m_bCloseMaellaBackdoor;

// DLP Groupe
bool	CNeoPreferences::m_bLeecherModDetection;
bool	CNeoPreferences::m_bLeecherNickDetection;
bool	CNeoPreferences::m_bLeecherHashDetection;
int		CNeoPreferences::m_iDetectionLevel;

// Behavioural groupe
UINT	CNeoPreferences::m_uAgressionDetection;
UINT	CNeoPreferences::m_uHashChangeDetection;

bool	CNeoPreferences::m_bUploadFakerDetection;
bool	CNeoPreferences::m_bFileFakerDetection;
bool	CNeoPreferences::m_bRankFloodDetection;
bool	CNeoPreferences::m_bXsExploitDetection;
bool	CNeoPreferences::m_bFileScannerDetection;
UINT	CNeoPreferences::m_uSpamerDetection;

bool	CNeoPreferences::m_bHashThiefDetection;
UINT	CNeoPreferences::m_uNickThiefDetection;
bool	CNeoPreferences::m_bModThiefDetection;
#endif //ARGOS // NEO: NA END

#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry]
int		CNeoPreferences::m_iIP2CountryNameMode;
UINT	CNeoPreferences::m_uIP2CountryShowFlag;
#endif // IP2COUNTRY // NEO: IP2C END

// NEO: NMX - [NeoMenuXP]
bool	CNeoPreferences::m_bShowXPSideBar;
bool	CNeoPreferences::m_bShowXPBitmap;
int 	CNeoPreferences::m_iXPMenuStyle;
bool	CNeoPreferences::m_bGrayMenuIcon;
// NEO: NMX END

/* Xanatos:
* All Neo Preferences Functions:
*/

CNeoPreferences::CNeoPreferences()
{
}

void CNeoPreferences::Init()
{
	LoadNeoPreferences();

	CIni ini(thePrefs.GetConfigFile());
	ini.SetSection(_T("KnownPrefs"));
	KnownPrefs.Load(ini);
	ini.SetSection(_T("PartPrefs"));
	PartPrefs.Load(ini);
}

void CNeoPreferences::Save()
{
	SaveNeoPreferences();

	CIni ini(thePrefs.GetConfigFile(), L"NeoPrefs");
	ini.SetSection(_T("KnownPrefs"));
	KnownPrefs.Save(ini);
	ini.SetSection(_T("PartPrefs"));
	PartPrefs.Save(ini);
}

void CNeoPreferences::LoadNeoPreferences()
{
	CIni ini(thePrefs.GetConfigFile(), L"NeoPrefs");

	m_dAppPriority = ini.GetInt(L"dAppPriority", NORMAL_PRIORITY_CLASS); // NEO: MOD - [AppPriority]
	m_iPreferredTempDir = ini.GetInt(L"PreferredTempDir", AUTO_TEMPDIR); // NEO: MTD - [MultiTempDirectories]
	m_bPauseOnFileComplete = ini.GetBool(L"m_bPauseOnFileComplete", false); // NEO: POFC - [PauseOnFileComplete]

	m_uIncompletePartStatus = ini.GetInt(L"IncompletePartStatus", 2); // NEO: ICS - [InteligentChunkSelection] 
	m_uSubChunkTransfer = ini.GetInt(L"SubChunkTransfer", 2); // NEO: SCT - [SubChunkTransfer]
	m_bPartStatusHistory = ini.GetBool(L"PartStatusHistory", false); // NEO: PSH - [PartStatusHistory]
	m_bLowID2HighIDAutoCallback = ini.GetBool(L"LowID2HighIDAutoCallback", true); // NEO: L2HAC - [LowID2HighIDAutoCallback]
	m_bSaveComments = ini.GetBool(L"SaveComments", false); // NEO: XCs - [SaveComments]
	m_bKnownComments = ini.GetBool(L"KnownComments ", false); // NEO: XCk - [KnownComments]	
	
	m_bPreferShareAll = ini.GetBool(L"PreferShareAll", false); // NEO: PSA - [PreferShareAll]
	m_uLugdunumCredits = ini.GetInt(L"LugdunumCredits", TRUE); // NEO: KLC - [KhaosLugdunumCredits]
	m_bDontRemoveStaticServers = ini.GetBool(L"DontRemoveStaticServers", true); // NEO: MOD - [DontRemoveStaticServers]

	m_bShowBanner = ini.GetBool(L"ShowBanner", true); // NEO: NPB - [PrefsBanner]
	m_bCollorShareFiles = ini.GetBool(L"CollorShareFiles", true); // NEO: NSC - [NeoSharedCategories]
	m_bSmoothStatisticsGraphs = ini.GetBool(L"SmoothStatisticsGraphs", true); // NEO: NBC - [NeoBandwidthControl] 

	m_uDisableAutoSort = ini.GetInt(L"DisableAutoSort", FALSE); // NEO: SE - [SortExtension]

	// NEO: NSTI - [NewSystemTrayIcon]
	m_bShowSystemTrayUpload = ini.GetBool(L"ShowSystemTrayUpload", true);
	m_bThinSystemTrayBars = ini.GetBool(L"ThinSystemTrayBars", false);
	m_iTrayBarsMaxCollor = ini.GetInt(L"TrayBarsMaxCollor", 1);
	// NEO: NSTI END

	m_bStaticTrayIcon = ini.GetBool(L"StaticTrayIcon", false); // NEO: STI - [StaticTray]

	// NEO: IM - [InvisibelMode]
	m_bInvisibleMode = ini.GetBool(L"InvisibleMode", false);
	m_iInvisibleModeHotKeyModifier = ini.GetInt(L"InvisibleModeHotKeyModifier", MOD_CONTROL | MOD_SHIFT | MOD_ALT);
	m_cInvisibleModeHotKey = ini.GetString(L"InvisibleModeHotKey", L"E").GetAt(0);
	// NEO: IM END

	// NEO: TPP - [TrayPasswordProtection]
	m_bTrayPasswordProtection = ini.GetBool(L"TrayPasswordProtection", false);
	m_sTrayPassword = ini.GetString(L"TrayPassword", L"");
	// NEO: TPP END

	m_bUsePlusSpeedMeter = ini.GetBool(L"UsePlusSpeedMeter", false); // NEO: PSM - [PlusSpeedMeter]

	m_bUseRelativeChunkDisplay = ini.GetBool(L"UseRelativeChunkDisplay", false); // NEO: MOD - [RelativeChunkDisplay]

	// NEO: SI - [SysInfo]
	m_bDrawSysInfoGraph = ini.GetBool(L"DrawSysInfoGraph", true);
	m_bShowSysInfoOnTitle = ini.GetBool(L"ShowSysInfoOnTitle", false);
	// NEO: SI END
	
	m_bUseChunkDots = ini.GetBool(L"UseChunkDots", false);	// NEO: MOD - [ChunkDots]

	m_bUseTreeStyle = ini.GetBool(L"UseTreeStyle", true); 	// NEO: NTS - [NeoTreeStyle]

	m_bShowClientPercentage = ini.GetBool(L"ShowClientPercentage", false); // NEO: MOD - [Percentage]

	// NEO: MOD - [RefreshShared]
	m_bRefreshShared = ini.GetBool(L"RefreshShared", false);
	m_iRefreshSharedIntervals = ini.GetInt(L"RefreshSharedIntervals", 120); // min
	// NEO: MOD END

	// NEO: OCF - [OnlyCompleetFiles]
	m_bOnlyCompleteFiles = ini.GetBool(L"OnlyCompleteFiles", false);
	m_iToOldComplete = ini.GetInt(L"ToOldComplete", 30);
	// NEO: OCF END
	
	// NEO: OCC - [ObelixConnectionControl]
	m_bConnectionControl = ini.GetBool(L"ConnectionControl", false);
	m_iConnectionControlValue = ini.GetInt(L"ConnectionControlValue", VAL_CON_CTR_DEF);
	// NEO: OCC END

	// NEO: SCM - [SmartConnectionManagement]
	m_bManageConnections = ini.GetBool(L"ManageConnections", false);
	m_fManageConnectionsFactor = ini.GetFloat(L"ManageConnectionsFactor", VAL_CON_MAN_DEF);
	// NEO: SCM END

	// NEO: NCC - [NeoConnectionChecker]
	m_bAutoConnectionChecker = ini.GetBool(L"AutoConnectionChecker", true);
	m_iAutoConnectionCheckerValue = ini.GetInt(L"AutoConnectionCheckerValue", VAL_CON_CHK_DEF);
	// NEO: NCC END

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
	m_uNAFCEnabled = ini.GetInt(L"NAFCEnabled", 1);
	m_bISPCustomIP = ini.GetBool(L"ISPCustomIP", false);
	m_uISPZone = ini.GetInt(L"ISPZone", 0);
	m_uISPMask = ini.GetInt(L"ISPMask", 0x0000FFFF);
	m_bBindToAdapter = ini.GetBool(L"BindToAdapter", false);


	m_bCheckConnection = ini.GetBool(L"CheckConnection", false);
	m_bStaticLowestPing = ini.GetBool(L"StaticLowestPing", false);
	m_iPingMode = ini.GetInt(L"PingMode", 0);
	m_bNoTTL = ini.GetBool(L"NoTTL", false);
	m_bManualHostToPing = ini.GetBool(L"ManualHostToPing", false);
	m_sPingServer = ini.GetString(L"PingServer", L"");

	m_bAutoMSS = ini.GetBool(L"AutoMSS", false);
	m_iMSS = ini.GetInt(L"MSS", 1300);

	m_bUseDoubleSendSize = ini.GetBool(L"UseDoubleSendSize", true);


	m_bUseDownloadBandwidthThrottler = ini.GetBool(L"UseDownloadBandwidthThrottler", false);
	m_uUseHyperDownload = ini.GetInt(L"UseHyperDownload", false);
	m_iBCTimeDown = ini.GetInt(L"BCTimeDown", VAL_BC_TIME_DOWN_DEF);
	m_iBCPriorityDown = ini.GetInt(L"BCPriorityDown", 0);
	m_uSetDownloadBuffer = ini.GetInt(L"UseDownloadBuffer", TRUE);
	m_iDownloadBufferSize = ini.GetInt(L"DownloadBufferLimit", VAL_DOWNLOAD_BUFFER_DEF);
	
	m_uUseBlockedQueue = ini.GetInt(L"UseBlockedQueue", TRUE);
	m_iBCTimeUp = ini.GetInt(L"BCTimeUp", VAL_BC_TIME_UP_DEF);
	m_iBCPriorityUp = ini.GetInt(L"BCPriorityUp", 0);
	m_uSetUploadBuffer = ini.GetInt(L"UseUploadBuffer", 2);
	m_iUploadBufferSize = ini.GetInt(L"UploadBufferLimit", VAL_UPLOAD_BUFFER_DEF);

	m_iDatarateSamples = ini.GetInt(L"DatarateSamples", 10);


	m_bIncludeOverhead = ini.GetBool(L"IncludeOverhead", false);
	m_bIncludeTCPAck = ini.GetBool(L"IncludeTCPAck", false);
	m_bConnectionsOverHead = ini.GetBool(L"ConnectionsOverHead", false);
	m_bSessionRatio = ini.GetBool(L"SessionRatio", false);


	m_iDownloadControl = ini.GetInt(L"DownloadControl", 0);
	m_fMinBCDownload = ini.GetFloat(L"MinBCDownload", 40);
	m_fMaxBCDownload = ini.GetFloat(L"MaxBCDownload", 0);


	m_iUploadControl = ini.GetInt(L"UploadControl", 0);
	m_fMinBCUpload = ini.GetFloat(L"MinBCUpload",10);
	m_fMaxBCUpload = ini.GetFloat(L"MaxBCUpload",0);


	m_fMaxDownStream = ini.GetFloat(L"MaxDownStream",64);
	m_fMaxUpStream = ini.GetFloat(L"MaxUpStream",16);


	m_bMinimizeOpenedSlots = ini.GetBool(L"MinimizeOpenedSlots", false);
	m_uCumulateBandwidth = ini.GetInt(L"CumulateBandwidth", FALSE);
	m_uBadwolfsUpload = ini.GetInt(L"BadwolfsUpload", 2);

	m_iMaxReservedSlots = ini.GetInt(L"MaxReservedSlots", VAL_MAX_RESERVED_SLOTS_DEF);
	
	m_bIncreaseTrickleSpeed = ini.GetBool(L"IncreaseTrickleSpeed", true);
	m_fIncreaseTrickleSpeed = ini.GetFloat(L"IncreaseTrickleSpeedValue", VAL_INCREASE_TRICKLE_SPEED_DEF);

 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
	m_uSeparateReleaseBandwidth = ini.GetInt(L"SeparateReleaseBandwidth", FALSE);
	m_fReleaseSlotSpeed = ini.GetFloat(L"ReleaseSlotSpeed", VAL_RELEASE_SLOT_SPEED_DEF);
	m_fReleaseBandwidthPercentage = ini.GetFloat(L"ReleaseBandwidthPercentage", VAL_RELEASE_BANDWIDTH_PERCENTAGE_DEF);

	m_uSeparateFriendBandwidth = ini.GetInt(L"SeparateFriendBandwidth", FALSE);
	m_fFriendSlotSpeed = ini.GetFloat(L"FriendSlotSpeed", VAL_FRIEND_SLOT_SPEED_DEF);
	m_fFriendBandwidthPercentage = ini.GetFloat(L"FriendBandwidthPercentage", VAL_FRIEND_BANDWIDTH_PERCENTAGE_DEF);
 #endif // BW_MOD // NEO: BM END

	m_iMaxUploadSlots = ini.GetInt(L"MaxUploadSlots",VAL_MAX_UL_SLOTS_DEF);
	m_iMinUploadSlots = ini.GetInt(L"MinUploadSlots",VAL_MIN_UL_SLOTS_DEF);
	m_fUploadPerSlots = ini.GetFloat(L"MaxUploadPerSlot",VAL_UL_PER_SLOTS_DEF);

	m_bOpenMoreSlotsWhenNeeded = ini.GetBool(L"OpenMoreSlotsWhenNeeded", true);
	m_bCheckSlotDatarate = ini.GetBool(L"CheckSlotDatarate", true);

	m_bIsTrickleBlocking = ini.GetBool(L"IsTrickleBlocking", false);
	m_bIsDropBlocking = ini.GetBool(L"IsDropBlocking", true);


	m_bDynUpGoingDivider = ini.GetBool(L"DynUpGoingDivider", false);
	m_iDynUpGoingUpDivider = ini.GetInt(L"DynUpGoingUpDivider", VAL_DYN_UP_GOING_UP_DIVIDER_DEF);
	m_iDynUpGoingDownDivider = ini.GetInt(L"DynUpGoingDownDivider", VAL_DYN_UP_GOING_DOWN_DIVIDER_DEF);

	m_iUpMaxPingMethod = ini.GetInt(L"UpMaxPingMethod", 1);
	m_iBasePingUp = ini.GetInt(L"BasePingUp", VAL_BASE_PING_UP_DEF);
	m_iPingUpTolerance = ini.GetInt(L"PingUpTolerance", VAL_PING_UP_TOLERANCE_DEF);
	m_iPingUpProzent = ini.GetInt(L"PingUpProzent", VAL_PING_UP_PROZENT_DEF);


	m_bDynDownGoingDivider = ini.GetBool(L"DynDownGoingDivider", false);
	m_iDynDownGoingUpDivider = ini.GetInt(L"DynDownGoingDivider", VAL_DYN_DOWN_GOING_UP_DIVIDER_DEF);
	m_iDynDownGoingDownDivider = ini.GetInt(L"DynDownGoingDownDivider", VAL_DYN_DOWN_GOING_DOWN_DIVIDER_DEF);
	
	m_iDownMaxPingMethod = ini.GetInt(L"DownMaxPingMethod", 1);
	m_iBasePingDown = ini.GetInt(L"BasePingDown", VAL_BASE_PING_DOWN_DEF);
	m_iPingDownTolerance = ini.GetInt(L"PingDownTolerance", VAL_PING_DOWN_TOLERANCE_DEF);
	m_iPingDownProzent = ini.GetInt(L"PingDownProzent", VAL_PING_DOWN_PROZENT_DEF);
#endif // NEO_BC // NEO: NBC END

	m_bTCPDisableNagle = ini.GetBool(L"TCPDisableNagle", false);

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	m_bLancastEnabled = ini.GetBool(L"LancastEnabled", false);

	m_iMaxLanDownload = (uint16)ini.GetInt(L"MaxLanDownload" , 0); 
	m_bSetLanDownloadBuffer = ini.GetBool(L"SetLanDownloadBuffer", true);
	m_iLanDownloadBufferSize = ini.GetInt(L"LanDownloadBufferSize", VAL_LAN_DOWNLOAD_BUFFER_DEF); 

	m_iMaxLanUpload = (uint16)ini.GetInt(L"MaxLanUpload", 0); 
	m_bSetLanUploadBuffer = ini.GetBool(L"SetLanUploadBuffer", true);
	m_iLanUploadBufferSize = ini.GetInt(L"LanUploadBufferSize", VAL_LAN_UPLOAD_BUFFER_DEF); 

	m_iMaxLanUploadSlots = ini.GetInt(L"MaxLanUploadSlots", VAL_MAX_LAN_UPLOAD_SLOTS_DEF); 

	m_bCustomizedLanCast = ini.GetBool(L"CustomizedLanCast", false);
	m_sLanCastGroup = ini.GetString(L"LanCastGroup", LANCAST_GROUP);
	m_uLanCastPort = (uint16)ini.GetInt(L"LanCastPort", LANCAST_PORT);

	m_bCustomLanCastAdapter = ini.GetBool(L"CustomLanCastAdapter", false);
	m_uLanCastAdapterIPAdress = ini.GetInt(L"LanCastAdapterIPAdress", 0);
	m_uLanCastAdapterSubNet = ini.GetInt(L"LanCastAdapterSubNet", 0x0000FFFF);

	m_bAutoBroadcastLanFiles = ini.GetBool(L"AutoBroadcastLanFiles", false);
	m_iAutoBroadcastLanFiles = ini.GetInt(L"AutoBroadcastLanFilesTimer" , DEF_AUTO_BROADCAST_LAN_FILES); 

	m_bUseLanMultiTransfer = ini.GetBool(L"UseLanMultiTransfer", true);
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	m_bUseVoodooTransfer = ini.GetBool(L"UseVoodooTransfer", false);
	m_bSlaveAllowed = ini.GetBool(L"SlaveAllowed", false);
	m_bSlaveHosting = ini.GetBool(L"SlaveHosting", false);

	m_sVoodooSpell = ini.GetString(L"VoodooSpell",L"Voodoo");
	m_nVoodooPort = (uint16)ini.GetInt(L"VoodooPort", 666); 

	m_uAutoConnectVoodoo = ini.GetInt(L"AutoConnectVoodoo", TRUE);
	m_iVoodooReconectTime = ini.GetInt(L"VoodooReconectTime", DEF_VOODOO_RECONNECT_TIME); 

	m_uUseVirtualVoodooFiles = ini.GetInt(L"UseVirtualVoodooFiles", TRUE);

	m_uVoodooCastEnabled = ini.GetInt(L"VoodooCastEnabled", 2);

	m_uSearchForSlaves = ini.GetInt(L"SearchForSlaves", FALSE); 
	m_uSearchForMaster = ini.GetInt(L"SearchForMaster", FALSE); 
	m_iVoodooSearchIntervals = ini.GetInt(L"VoodooSearchIntervals", DEF_VOODOO_SEARCH); 

	m_bHideVoodooFiles = ini.GetBool(L"HideVoodooFiles", false); // NEO: PP - [PasswordProtection]
#endif // VOODOO // NEO: VOODOO END

	// NEO: QS - [QuickStart]
	m_uQuickStart = ini.GetInt(L"UseQuickStart", FALSE);
	m_iQuickStartTime = ini.GetInt(L"QuickStartTime", TIM_QUICK_START_DEF);
	m_iQuickStartTimePerFile = ini.GetInt(L"QuickStartTimePerFile", VAL_QUICK_START_PF_DEF);

	m_iQuickMaxConperFive = ini.GetInt(L"QuickMaxConperFive", (int)(thePrefs.MaxConperFive*1.5));
	m_iQuickMaxHalfOpen = ini.GetInt(L"QuickMaxHalfOpen", (int)(thePrefs.maxhalfconnections));
	m_iQuickMaxConnections = ini.GetInt(L"QuickMaxConnections", (int)(thePrefs.maxconnections*1.5));
	// NEO: QS END

	// NEO: RIC - [ReaskOnIDChange]
	m_uCheckIPChange = ini.GetInt(L"CheckIDChanges", 2);
	m_bInformOnIPChange = ini.GetBool(L"InformClientsOnIDChange", false);
	m_bInformOnBuddyChange = ini.GetBool(L"InformOnBuddyChange", true);
	m_bReAskOnIPChange = ini.GetBool(L"ReaskSourcesOnIDChange", false);
	m_bQuickStartOnIPChange = ini.GetBool(L"RedoQuickStartOnIDChange", false); // NEO: QS - [QuickStart]
	m_bCheckL2HIDChange = ini.GetBool(L"CheckLowID2HidhIDChanges", false);
	m_bReconnectKadOnIPChange = ini.GetBool(L"ReconnectKadOnIPChange", true);
	m_bRebindSocketsOnIPChange = ini.GetBool(L"RebindSocketsOnIPChange", false);
	// NEO: RIC END

	// NEO: RKF - [RecheckKadFirewalled]
	m_bRecheckKadFirewalled = ini.GetBool(L"RecheckKadFirewalled", true);
	m_iRecheckKadFirewalled = ini.GetInt(L"RecheckKadFirewalledTime", TIM_RECHECK_KAD_FIREWALLED_DEF);
	// NEO: RKF END

	// NEO: RLD - [ReconnectOnLowID]
	m_bReConnectOnLowID = ini.GetBool(L"ReconnectOnLowId" , false);
	m_iReConnectOnLowID = ini.GetInt(L"ReconnectOnLowIdRetries", VAL_RECONNECT_ON_LOWID_TRYS_DEF); 
	// NEO: RLD END

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
	m_bNatTraversalEnabled = ini.GetBool(L"NatTraversalEnabled", true);
	m_uLowIDUploadCallBack = ini.GetInt(L"LowIDUploadCallBack", 2); // NEO: LUC - [LowIDUploadCallBack]
	m_bReuseTCPPort = ini.GetBool(L"ReuseTCPPort", false); // NEO: RTP - [ReuseTCPPort]
#endif //NATTUNNELING // NEO: NATT END

	// NEO: NPT - [NeoPartTraffic]
	m_uUsePartTraffic = ini.GetInt(L"UsePartTraffic", TRUE);
	m_iPartTrafficCollors = ini.GetInt(L"PartTrafficCollors", 1);
	// NEO: NPT END
	m_bUseClassicShareStatusBar = ini.GetBool(L"UseClassicShareStatusBar", true); // NEO: MOD - [ClassicShareStatusBar]
	m_bUseShowSharePermissions = ini.GetBool(L"UseShowSharePermissions", false); // NEO: SSP - [ShowSharePermissions]
	
	// NEO: SRS - [SmartReleaseSharing]
	m_iReleaseChunks = ini.GetInt(L"ReleaseChunks", DEF_RELEASE_CHUNKS);
	m_uReleaseSlotLimit = ini.GetInt(L"LimitReleaseSlots", TRUE);
	m_iReleaseSlotLimit = ini.GetInt(L"ReleaseSlotLimit", DEF_RELEASE_SLOT_LIMIT);
	// NEO: SRS END

	// NEO: NMFS - [NiceMultiFriendSlots]
	m_uFriendSlotLimit = ini.GetInt(L"LimitFriendSlots", FALSE);
	m_iFriendSlotLimit = ini.GetInt(L"FriendSlotLimit", DEF_FRIEND_SLOT_LIMIT);
	// NEO: NMFS END

	m_bSaveUploadQueueWaitTime = ini.GetBool(L"SaveUploadQueueWaitTime", true); // NEO: SQ - [SaveUploadQueue]
	m_bUseMultiQueue = ini.GetBool(L"UseMultiQueue", false); // NEO: MQ - [MultiQueue]
	m_uUseRandomQueue = ini.GetInt(L"UseRandomQueue", FALSE); // NEO: RQ - [RandomQueue]

	m_bNeoScoreSystem = ini.GetBool(_T("NeoScoreSystem"),true); // NEO: NFS - [NeoScoreSystem]
	m_bNeoCreditSystem = ini.GetBool(_T("NeoCreditSystem"),true); // NEO: NCS - [NeoCreditSystem]
	m_iOtherCreditSystem = ini.GetInt(_T("OtherCreditSystem"),0); // NEO: OCS - [OtherCreditSystems]

	// NEO: TQ - [TweakUploadQueue]
	m_bUseInfiniteQueue = ini.GetBool(L"UseInfiniteQueue", false);

	m_uQueueOverFlowDef = ini.GetInt(L"UseQueueOverFlowDef", TRUE);
	m_iQueueOverFlowDef = ini.GetInt(L"QueueOverFlowDef", DEF_QUEUE_OVERFLOW);
	m_uQueueOverFlowEx = ini.GetInt(L"UseQueueOverFlowEx", FALSE);
	m_iQueueOverFlowEx = ini.GetInt(L"QueueOverFlowEx", DEF_QUEUE_OVERFLOW);
	m_uQueueOverFlowRelease = ini.GetInt(L"UseQueueOverFlowRelease", FALSE);
	m_iQueueOverFlowRelease = ini.GetInt(L"QueueOverFlowRelease",DEF_QUEUE_OVERFLOW);
	m_uQueueOverFlowCF = ini.GetInt(L"UseQueueOverFlowCF", FALSE);
	m_iQueueOverFlowCF = ini.GetInt(L"QueueOverFlowCF", DEF_QUEUE_OVERFLOW);
	// NEO: TQ END

	// NEO: PRSF - [PushSmallRareFiles]
	m_bPushSmallFiles = ini.GetBool(L"PushSmallFiles", true);
	m_iPushSmallFilesSize = ini.GetInt(L"PushSmallFilesSize", DEF_PUSH_SMALL_FILES);
	m_bPushRareFiles = ini.GetBool(L"PushRareFiles", true);
	m_iPushRareFilesValue = ini.GetInt(L"PushRareFilesValue", DEF_PUSH_RARE_FILES);
	m_bPushRatioFiles = ini.GetBool(L"PushRatioFiles", true);
	m_iPushRatioFilesValue = ini.GetInt(L"PushRatioFilesValue", DEF_PUSH_RATIO_FILES);
	// NEO: PRSF END

	// NEO: NXC - [NewExtendedCategories]
	m_bShowCatNames = ini.GetBool(_T("ShowCatNames"),true);
	m_uShowCategoryFlags = ini.GetInt(_T("ShowCategoryFlags"),TRUE);
	m_bActiveCatDefault = ini.GetBool(_T("ActiveCatDefault"), true);
	m_bSelCatOnAdd = ini.GetBool(_T("SelCatOnAdd"), false);
	m_bAutoSetResumeOrder = ini.GetBool(_T("AutoSetResumeOrder"), true);
	m_bSmallFileDLPush = ini.GetBool(_T("SmallFileDLPush"), true);
	m_iSmallFileDLPush = ini.GetInt(_T("SmallFileDLPushSize"), 150);
	m_bStartDLInEmptyCats = ini.GetBool(_T("StartDLInEmptyCats"), false);
	m_iStartDLInEmptyCats = ini.GetInt(_T("StartDLInEmptyCatsAmount"), 5);
	m_bUseAutoCat = ini.GetBool(_T("UseAutoCat"), true);
	m_bCheckAlreadyDownloaded = ini.GetBool(_T("CheckAlreadyDownloaded"), false);
	
	m_bStartNextFileByPriority = ini.GetBool(_T("StartNextFileByPriority"), false);

	m_bSmartA4AFSwapping = ini.GetBool(_T("SmartA4AFSwapping"), true);
	m_iAdvancedA4AFMode = ini.GetInt(_T("AdvancedA4AFMode"), 1);
	// NEO: NXC END

	// NEO: NTB - [NeoToolbarButtons]
	m_uNeoToolbar = ini.GetInt(_T("NeoToolbar"), FALSE);
	m_iNeoToolbarButtonCount = ini.GetInt(_T("NeoToolbarButtonCount"), 8);
	m_NeoToolbarButtons.RemoveAll();
	m_NeoToolbarButtons.SetSize(m_iNeoToolbarButtonCount);
	for (int i=0; i<m_iNeoToolbarButtonCount; i++)
		m_NeoToolbarButtons[i] = (ini.GetInt(StrLine(L"%s_%u",_T("NeoToolbarButtons"),i), 0));
	// NEO: NTB END

#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase]
	m_bEnableSourceList = ini.GetBool(L"EnableSourceList", true);
	m_iTableAmountToStore = ini.GetInt(L"TableAmountToStore", DEF_TABLE_AMOUT_TO_STORE);
	// cleanup
	m_uSourceListRunTimeCleanUp = ini.GetInt(L"SourceListRunTimeCleanUp", TRUE);
	m_iSourceListExpirationTime = ini.GetInt(L"SourceListExpirationTime", DEF_SOURCE_EXPIRATION_TIME);
	// NEO: SFL - [SourceFileList]
	// seen files
	m_bSaveSourceFileList = ini.GetBool(L"SaveSourceFileList", true);
	m_iFileListExpirationTime = ini.GetInt(L"FileListExpirationTime", DEF_FILE_EXPIRATION_TIME);
	// NEO: SFL END
	// security
	m_bUseIPZoneCheck = ini.GetBool(L"UseIPZoneCheck", true);
	m_bSourceHashMonitor = ini.GetBool(L"SourceHashMonitor", false); // NEO: SHM - [SourceHashMonitor]
	// aditional obtions
	m_iIgnoreUndefinedIntervall = ini.GetInt(L"IgnoreUndefinedIntervall", DEF_IGNORE_UNREACHABLE_INTERVAL);
	m_iIgnoreUnreachableInterval = ini.GetInt(L"IgnoreUnreachableInterval", DEF_IGNORE_UNDEFINED_INTERVAL);
#endif // NEO_CD // NEO: NCD END
#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	m_bEnableSourceAnalizer = ini.GetBool(L"EnableSourceAnalizer", true);
	m_iAnalyzeIntervals = ini.GetInt(L"AnalyzeIntervals", DEF_ANALISIS_INTERVALS);
	m_iTableAmountToAnalyze = ini.GetInt(L"TableAmountToAnalyze", DEF_TABLE_AMOUT_TO_ANALISE);
	// gap handling
	m_bHandleTableGaps = ini.GetBool(L"HandleTableGaps", true);
	m_fPriorityGapRatio = ini.GetInt(L"PriorityGapRatio", DEF_PRIORITY_GAP_RATIO);
	m_iMaxGapSize = ini.GetInt(L"MaxGapSize", DEF_MAX_GAP_SIZE);
	m_iMaxGapTime = ini.GetInt(L"MaxGapTime", DEF_MAX_GAP_TIME);
	// analyze obtions
	m_fMaxMidleDiscrepanceHigh = ini.GetInt(L"MaxMidleDiscrepanceHigh", DEF_MAX_MIDLE_DISCREPANCE_HIGH);
	m_fMaxMidleDiscrepanceLow = ini.GetInt(L"MaxMidleDiscrepanceLow", DEF_MAX_MIDLE_DISCREPANCE_LOW);
	m_bDualLinkedTableGravity = ini.GetBool(L"UseDualLinkedTableGravity", true);
	m_iDualLinkedTableGravity = ini.GetInt(L"DualLinkedTableGravity", DEF_DUAL_LINKED_TABLE_GRAVITY);
	// calculation obtions
	m_fEnhancedFactor = ini.GetInt(L"EnhancedFactor", DEF_VAL_ENHANCED_FACTOR);
	m_iFreshSourceTreshold = ini.GetInt(L"FreshSourceTreshold", DEF_FRESH_SOURCE_TRESHOLD);
	m_iTempralIPBorderLine = ini.GetInt(L"TempralIPBorderLine", DEF_TEMPORAL_IP_BORDERLINE);
	// additional obtions
	m_fLastSeenDurationThreshold = ini.GetInt(L"LastSeenDurationThreshold", DEF_LAST_SEEN_DURATION_THRESHOLD);
	m_bLinkTimePropability = ini.GetBool(L"LinkTimePropability", true);
	m_iLinkTimeThreshold = ini.GetInt(L"LinkTimeThreshold", DEF_LINK_TIME_THRESHOLD);
	m_bScaleReliableTime = ini.GetBool(L"ScaleReliableTime", true);
	m_iMaxReliableTime = ini.GetInt(L"MaxReliableTime", DEF_RELIABLE_TIME);
#endif // NEO_SA // NEO: NSA END

#ifdef ARGOS // NEO: NA - [NeoArgos] 
	m_bZeroScoreGPLBreaker = ini.GetBool(L"ZeroScoreGPLBreaker", true);
	m_iBanTime = ini.GetInt(L"BanTime", MS2MIN(CLIENTBANTIME));
	m_bCloseMaellaBackdoor = ini.GetBool(L"CloseMaellaBackdoor", false);

	// DLP Groupe
	m_bLeecherModDetection = ini.GetBool(L"LeecherModDetection", true);
	m_bLeecherNickDetection = ini.GetBool(L"LeecherNickDetection", false);
	m_bLeecherHashDetection = ini.GetBool(L"LeecherHashDetection", true);
	m_iDetectionLevel = ini.GetInt(L"DetectionLevel", 1);

	// Behavioural groupe
	m_uAgressionDetection = ini.GetInt(L"AgressionDetection", TRUE);
	m_uHashChangeDetection = ini.GetInt(L"HashChangeDetection", 2);

	m_bUploadFakerDetection = ini.GetBool(L"UploadFakerDetection", false);
	m_bFileFakerDetection = ini.GetBool(L"FileFakerDetection", true);
	m_bRankFloodDetection = ini.GetBool(L"RankFloodDetection", true);
	m_bXsExploitDetection = ini.GetBool(L"XsExploitDetection", true);
	m_bFileScannerDetection = ini.GetBool(L"FileScannerDetection", true);
	m_uSpamerDetection = ini.GetInt(L"SpamerDetection", TRUE);

	m_bHashThiefDetection = ini.GetBool(L"HashThiefDetection", true);
	m_uNickThiefDetection = ini.GetInt(L"NickThiefDetection", FALSE);
	m_bModThiefDetection = ini.GetBool(L"ModThiefDetection", true);
#endif //ARGOS // NEO: NA END

#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry]
	m_iIP2CountryNameMode = ini.GetInt(L"IP2CountryNameMode", 0);
	m_uIP2CountryShowFlag = ini.GetInt(L"IP2CountryShowFlag", FALSE);
#endif // IP2COUNTRY // NEO: IP2C END

	// NEO: NMX - [NeoMenuXP]
	m_bShowXPSideBar = ini.GetBool(L"ShowXPSideBar", true);
	m_bShowXPBitmap = ini.GetBool(L"ShowXPBitmap", true);
	m_iXPMenuStyle = ini.GetInt(L"XPMenuStyle", 1);
	m_bGrayMenuIcon = ini.GetBool(L"GrayMenuIcon", false);
	// NEO: NMX END 

	CheckNeoPreferences();
}

void CNeoPreferences::CheckNeoPreferences()
{
	MinMax(&m_iToOldComplete,3,300); // NEO: OCF - [OnlyCompleetFiles]
	
	MinMax(&m_iConnectionControlValue,VAL_CON_CTR_MIN,VAL_CON_CTR_MAX); // NEO: OCC - [ObelixConnectionControl]

	MinMax(&m_fManageConnectionsFactor,VAL_CON_MAN_MIN,VAL_CON_MAN_MAX); // NEO: SCM - [SmartConnectionManagement]

	MinMax(&m_iAutoConnectionCheckerValue,VAL_CON_CHK_MIN,VAL_CON_CHK_MAX); // NEO: NCC - [NeoConnectionChecker]

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
	MinMax(&m_iBCTimeDown,VAL_BC_TIME_DOWN_MIN,VAL_BC_TIME_DOWN_MAX);
	MinMax(&m_iBCTimeUp,VAL_BC_TIME_UP_MIN,VAL_BC_TIME_UP_MAX);

	MinMax(&m_iDownloadBufferSize,VAL_DOWNLOAD_BUFFER_MIN,VAL_DOWNLOAD_BUFFER_MAX);
	MinMax(&m_iUploadBufferSize,VAL_UPLOAD_BUFFER_MIN,VAL_UPLOAD_BUFFER_MAX);

	MinMax(&m_iDatarateSamples,5,20);


	MinMax(&m_iMaxReservedSlots,VAL_MAX_RESERVED_SLOTS_MIN,VAL_MAX_RESERVED_SLOTS_MAX);

	MinMax(&m_fIncreaseTrickleSpeed,VAL_INCREASE_TRICKLE_SPEED_MIN,VAL_INCREASE_TRICKLE_SPEED_MAX);

 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
	MinMax(&m_fReleaseSlotSpeed,VAL_RELEASE_SLOT_SPEED_MIN,VAL_RELEASE_SLOT_SPEED_MAX);
	MinMax(&m_fReleaseBandwidthPercentage,VAL_RELEASE_BANDWIDTH_PERCENTAGE_MIN,VAL_RELEASE_BANDWIDTH_PERCENTAGE_MAX);

	MinMax(&m_fFriendSlotSpeed,VAL_FRIEND_SLOT_SPEED_MIN,VAL_FRIEND_SLOT_SPEED_MAX);
	MinMax(&m_fFriendBandwidthPercentage,VAL_FRIEND_BANDWIDTH_PERCENTAGE_MIN,VAL_FRIEND_BANDWIDTH_PERCENTAGE_MAX);
 #endif // BW_MOD // NEO: BM END

	MinMax(&m_iMaxUploadSlots,VAL_MAX_UL_SLOTS_MIN,VAL_MAX_UL_SLOTS_MAX);
	MinMax(&m_iMinUploadSlots,VAL_MIN_UL_SLOTS_MIN,VAL_MIN_UL_SLOTS_MAX);
	Maximal(&m_iMinUploadSlots,m_iMaxUploadSlots);
	MinMax(&m_fUploadPerSlots,VAL_UL_PER_SLOTS_MIN,VAL_UL_PER_SLOTS_MAX);

	MinMax(&m_iDynUpGoingUpDivider,VAL_DYN_UP_GOING_UP_DIVIDER_MIN,VAL_DYN_UP_GOING_UP_DIVIDER_MAX);
	MinMax(&m_iDynUpGoingDownDivider,VAL_DYN_UP_GOING_DOWN_DIVIDER_MIN,VAL_DYN_UP_GOING_DOWN_DIVIDER_MAX);

	MinMax(&m_iBasePingUp,VAL_BASE_PING_UP_MIN,VAL_BASE_PING_UP_MAX);
	MinMax(&m_iPingUpTolerance,VAL_PING_UP_TOLERANCE_MIN,VAL_PING_UP_TOLERANCE_MAX);
	MinMax(&m_iPingUpProzent,VAL_PING_UP_PROZENT_MIN,VAL_PING_UP_PROZENT_MAX);

	MinMax(&m_iDynDownGoingUpDivider,VAL_DYN_DOWN_GOING_UP_DIVIDER_MIN,VAL_DYN_DOWN_GOING_UP_DIVIDER_MAX);
	MinMax(&m_iDynDownGoingDownDivider,VAL_DYN_DOWN_GOING_DOWN_DIVIDER_MIN,VAL_DYN_DOWN_GOING_DOWN_DIVIDER_MAX);

	MinMax(&m_iBasePingDown,VAL_BASE_PING_DOWN_MIN,VAL_BASE_PING_DOWN_MAX);
	MinMax(&m_iPingDownTolerance,VAL_PING_DOWN_TOLERANCE_MIN,VAL_PING_DOWN_TOLERANCE_MAX);
	MinMax(&m_iPingDownProzent,VAL_PING_DOWN_PROZENT_MIN,VAL_PING_DOWN_PROZENT_MAX);
#endif // NEO_BC // NEO: NBC END

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	MinMax(&m_iLanDownloadBufferSize,VAL_LAN_DOWNLOAD_BUFFER_MIN,VAL_LAN_DOWNLOAD_BUFFER_MAX);
	MinMax(&m_iLanUploadBufferSize,VAL_LAN_UPLOAD_BUFFER_MIN,VAL_LAN_UPLOAD_BUFFER_MAX);

	MinMax(&m_iMaxLanUploadSlots,VAL_MAX_LAN_UPLOAD_SLOTS_MIN,VAL_MAX_LAN_UPLOAD_SLOTS_MAX);

	MinMax(&m_iAutoBroadcastLanFiles,MIN_AUTO_BROADCAST_LAN_FILES,MAX_AUTO_BROADCAST_LAN_FILES);
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	MinMax(&m_iVoodooSearchIntervals,MIN_VOODOO_SEARCH,MAX_VOODOO_SEARCH);
	MinMax(&m_iVoodooReconectTime,MIN_VOODOO_RECONNECT_TIME,MAX_VOODOO_RECONNECT_TIME);
#endif // VOODOO // NEO: VOODOO END

	// NEO: QS - [QuickStart]
	MinMax(&m_iQuickStartTime,TIM_QUICK_START_MIN, TIM_QUICK_START_MAX);
	MinMax(&m_iQuickStartTimePerFile,VAL_QUICK_START_PF_MIN, VAL_QUICK_START_PF_MAX);
	// NEO: QS END

	MinMax(&m_iRecheckKadFirewalled,TIM_RECHECK_KAD_FIREWALLED_MIN,TIM_RECHECK_KAD_FIREWALLED_MAX); // NEO: RKF - [RecheckKadFirewalled]
	MinMax(&m_iReConnectOnLowID,VAL_RECONNECT_ON_LOWID_TRYS_MIN,VAL_RECONNECT_ON_LOWID_TRYS_MAX); // NEO: RLD - [ReconnectOnLowID]

	// NEO: SRS - [SmartReleaseSharing]
	MinMax(&m_iReleaseChunks,MIN_RELEASE_CHUNKS,MAX_RELEASE_CHUNKS);
	MinMax(&m_iReleaseSlotLimit,MIN_RELEASE_SLOT_LIMIT,MAX_RELEASE_SLOT_LIMIT); 
	// NEO: SRS END

	MinMax(&m_iFriendSlotLimit,MIN_FRIEND_SLOT_LIMIT,MAX_FRIEND_SLOT_LIMIT); // NEO: NMFS - [NiceMultiFriendSlots]

#ifdef BW_MOD // NEO: BM - [BandwidthModeration]
	MinMax(&m_fReleaseSlotSpeed,MIN_RELEASE_SLOT_SPEED,MAX_RELEASE_SLOT_SPEED);
	MinMax(&m_fReleaseBandwidthPercentage,MIN_RELEASE_BANDWIDTH_PERCENTAGE,MAX_RELEASE_BANDWIDTH_PERCENTAGE);

	MinMax(&m_fFriendSlotSpeed,MIN_FRIEND_SLOT_SPEED,MAX_FRIEND_SLOT_SPEED);
	MinMax(&m_fFriendBandwidthPercentage,MIN_FRIEND_BANDWIDTH_PERCENTAGE,MAX_FRIEND_BANDWIDTH_PERCENTAGE);
#endif // BW_MOD // NEO: BM END

	// NEO: TQ - [TweakUploadQueue]
	MinMax(&m_iQueueOverFlowDef,MIN_QUEUE_OVERFLOW,MAX_QUEUE_OVERFLOW);
	MinMax(&m_iQueueOverFlowEx,MIN_QUEUE_OVERFLOW,MAX_QUEUE_OVERFLOW);
	MinMax(&m_iQueueOverFlowRelease,MIN_QUEUE_OVERFLOW,MAX_QUEUE_OVERFLOW);
	MinMax(&m_iQueueOverFlowCF,MIN_QUEUE_OVERFLOW,MAX_QUEUE_OVERFLOW);
	// NEO: TQ END

	// NEO: PRSF - [PushSmallRareFiles]
	MinMax(&m_iPushSmallFilesSize,MIN_PUSH_SMALL_FILES,MAX_PUSH_SMALL_FILES);
	MinMax(&m_iPushRareFilesValue,MIN_PUSH_RARE_FILES,MAX_PUSH_RARE_FILES);
	MinMax(&m_iPushRatioFilesValue,MIN_PUSH_RATIO_FILES,MAX_PUSH_RATIO_FILES);
	// NEO: PRSF END

#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase]
	MinMax(&m_iTableAmountToStore, MIN_TABLE_AMOUT_TO_STORE, MAX_TABLE_AMOUT_TO_STORE);
	// cleanup
	MinMax(&m_iSourceListExpirationTime,MIN_SOURCE_EXPIRATION_TIME, MAX_SOURCE_EXPIRATION_TIME);
	// NEO: SFL - [SourceFileList]
	// seen files
	MinMax(&m_iFileListExpirationTime,MIN_FILE_EXPIRATION_TIME, MAX_FILE_EXPIRATION_TIME);
	// NEO: SFL END
	// aditional obtions
	MinMax(&m_iIgnoreUndefinedIntervall, MIN_IGNORE_UNREACHABLE_INTERVAL, MAX_IGNORE_UNREACHABLE_INTERVAL);
	MinMax(&m_iIgnoreUnreachableInterval, MIN_IGNORE_UNDEFINED_INTERVAL, MAX_IGNORE_UNDEFINED_INTERVAL);
#endif // NEO_CD // NEO: NCD END
#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	MinMax(&m_iAnalyzeIntervals, MIN_ANALISIS_INTERVALS, MAX_ANALISIS_INTERVALS);
	MinMax(&m_iTableAmountToAnalyze, MIN_TABLE_AMOUT_TO_ANALISE, MAX_TABLE_AMOUT_TO_ANALISE);
	// gap handling
	MinMax(&m_fPriorityGapRatio, MIN_PRIORITY_GAP_RATIO, MAX_PRIORITY_GAP_RATIO);
	MinMax(&m_iMaxGapSize, MIN_MAX_GAP_SIZE, MAX_MAX_GAP_SIZE);
	MinMax(&m_iMaxGapTime, MIN_MAX_GAP_TIME, MAX_MAX_GAP_TIME);
	// analyze obtions
	MinMax(&m_fMaxMidleDiscrepanceHigh, MIN_MAX_MIDLE_DISCREPANCE_HIGH, MAX_MAX_MIDLE_DISCREPANCE_HIGH);
	MinMax(&m_fMaxMidleDiscrepanceLow, MIN_MAX_MIDLE_DISCREPANCE_LOW, MAX_MAX_MIDLE_DISCREPANCE_LOW);
	MinMax(&m_iDualLinkedTableGravity, MIN_DUAL_LINKED_TABLE_GRAVITY, MAX_DUAL_LINKED_TABLE_GRAVITY);
	// calculation obtions
	MinMax(&m_fEnhancedFactor, MIN_VAL_ENHANCED_FACTOR, MAX_VAL_ENHANCED_FACTOR);
	MinMax(&m_iFreshSourceTreshold, MIN_FRESH_SOURCE_TRESHOLD, MAX_FRESH_SOURCE_TRESHOLD);
	MinMax(&m_iTempralIPBorderLine, MIN_TEMPORAL_IP_BORDERLINE, MAX_TEMPORAL_IP_BORDERLINE);
	// additional obtions
	MinMax(&m_fLastSeenDurationThreshold, MIN_LAST_SEEN_DURATION_THRESHOLD, MAX_LAST_SEEN_DURATION_THRESHOLD);
	MinMax(&m_iLinkTimeThreshold, MIN_LINK_TIME_THRESHOLD, MAX_LINK_TIME_THRESHOLD);
	MinMax(&m_iMaxReliableTime, MIN_RELIABLE_TIME, MAX_RELIABLE_TIME);
#endif // NEO_SA // NEO: NSA END
}

void CNeoPreferences::SaveNeoPreferences()
{
	CIni ini(thePrefs.GetConfigFile(), L"NeoPrefs");

	ini.WriteInt(L"dAppPriority", m_dAppPriority); // NEO: MOD - [AppPriority]
	ini.WriteInt(L"PreferredTempDir", m_iPreferredTempDir); // NEO: MTD - [MultiTempDirectories]
	ini.WriteBool(L"m_bPauseOnFileComplete", m_bPauseOnFileComplete); // NEO: POFC - [PauseOnFileComplete]
	
	ini.WriteInt(L"IncompletePartStatus", m_uIncompletePartStatus); // NEO: ICS - [InteligentChunkSelection] 
	ini.WriteInt(L"SubChunkTransfer", m_uSubChunkTransfer); // NEO: SCT - [SubChunkTransfer]
	ini.WriteBool(L"PartStatusHistory", m_bPartStatusHistory); // NEO: PSH - [PartStatusHistory]
	ini.WriteBool(L"LowID2HighIDAutoCallback", m_bLowID2HighIDAutoCallback); // NEO: L2HAC - [LowID2HighIDAutoCallback]
	ini.WriteBool(L"SaveComments", m_bSaveComments); // NEO: XCs - [SaveComments]
	ini.WriteBool(L"KnownComments ", m_bKnownComments); // NEO: XCk - [KnownComments]	
	
	ini.WriteBool(L"PreferShareAll", m_bPreferShareAll); // NEO: PSA - [PreferShareAll]
	ini.WriteInt(L"LugdunumCredits", m_uLugdunumCredits); // NEO: KLC - [KhaosLugdunumCredits]
	ini.WriteBool(L"DontRemoveStaticServers", m_bDontRemoveStaticServers); // NEO: MOD - [DontRemoveStaticServers]

	ini.WriteBool(L"ShowBanner", m_bShowBanner); // NEO: NPB - [PrefsBanner]
	ini.WriteBool(L"CollorShareFiles", m_bCollorShareFiles); // NEO: NSC - [NeoSharedCategories]
	ini.WriteBool(L"SmoothStatisticsGraphs", m_bSmoothStatisticsGraphs); // NEO: NBC - [NeoBandwidthControl] 

	ini.WriteInt(L"DisableAutoSort", m_uDisableAutoSort); // NEO: SE - [SortExtension]

	// NEO: NSTI - [NewSystemTrayIcon]
	ini.WriteBool(L"ShowSystemTrayUpload", m_bShowSystemTrayUpload);
	ini.WriteBool(L"ThinSystemTrayBars", m_bThinSystemTrayBars);
	ini.WriteInt(L"TrayBarsMaxCollor", m_iTrayBarsMaxCollor);
	// NEO: NSTI END

	ini.WriteBool(L"StaticTrayIcon", m_bStaticTrayIcon); // NEO: STI - [StaticTray]

	// NEO: IM - [InvisibelMode]
	ini.WriteBool(L"InvisibleMode", m_bInvisibleMode);
	ini.WriteInt(L"InvisibleModeHotKeyModifier", m_iInvisibleModeHotKeyModifier);
	ini.WriteString(L"InvisibleModeHotKey", CString(m_cInvisibleModeHotKey));
	// NEO: IM END

	// NEO: TPP - [TrayPasswordProtection]
	ini.WriteBool(L"TrayPasswordProtection", m_bTrayPasswordProtection);
	ini.WriteString(L"TrayPassword", m_sTrayPassword);
	// NEO: TPP END

	ini.WriteBool(L"UsePlusSpeedMeter", m_bUsePlusSpeedMeter); // NEO: PSM - [PlusSpeedMeter]

	ini.WriteBool(L"UseRelativeChunkDisplay", m_bUseRelativeChunkDisplay); // NEO: MOD - [RelativeChunkDisplay]

	// NEO: SI - [SysInfo]
	ini.WriteBool(L"DrawSysInfoGraph", m_bDrawSysInfoGraph);
	ini.WriteBool(L"ShowSysInfoOnTitle", m_bShowSysInfoOnTitle);
	// NEO: SI END
	
	ini.WriteBool(L"UseChunkDots", m_bUseChunkDots);	// NEO: MOD - [ChunkDots]

	ini.WriteBool(L"UseTreeStyle", m_bUseTreeStyle); 	// NEO: NTS - [NeoTreeStyle]

	ini.WriteBool(L"ShowClientPercentage", m_bShowClientPercentage); // NEO: MOD - [Percentage]

	// NEO: MOD - [RefreshShared]
	ini.WriteBool(L"RefreshShared", m_bRefreshShared);
	ini.WriteInt(L"RefreshSharedIntervals", m_iRefreshSharedIntervals); // min
	// NEO: MOD END

	// NEO: OCF - [OnlyCompleetFiles]
	ini.WriteBool(L"OnlyCompleteFiles", m_bOnlyCompleteFiles);
	ini.WriteInt(L"ToOldComplete", m_iToOldComplete);
	// NEO: OCF END
	
	// NEO: OCC - [ObelixConnectionControl]
	ini.WriteBool(L"ConnectionControl", m_bConnectionControl);
	ini.WriteInt(L"ConnectionControlValue", m_iConnectionControlValue);
	// NEO: OCC END

	// NEO: SCM - [SmartConnectionManagement]
	ini.WriteBool(L"ManageConnections", m_bManageConnections);
	ini.WriteFloat(L"ManageConnectionsFactor", m_fManageConnectionsFactor);
	// NEO: SCM END

	// NEO: NCC - [NeoConnectionChecker]
	ini.WriteBool(L"AutoConnectionChecker", m_bAutoConnectionChecker);
	ini.WriteInt(L"AutoConnectionCheckerValue", m_iAutoConnectionCheckerValue);
	// NEO: NCC END

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
	ini.WriteInt(L"NAFCEnabled", m_uNAFCEnabled);
	ini.WriteBool(L"ISPCustomIP", m_bISPCustomIP);
	ini.WriteInt(L"ISPZone", m_uISPZone);
	ini.WriteInt(L"ISPMask", m_uISPMask);
	ini.WriteBool(L"BindToAdapter", m_bBindToAdapter);


	ini.WriteBool(L"CheckConnection", m_bCheckConnection);
	ini.WriteBool(L"StaticLowestPing", m_bStaticLowestPing);
	ini.WriteInt(L"PingMode", m_iPingMode);
	ini.WriteBool(L"NoTTL", m_bNoTTL);
	ini.WriteBool(L"ManualHostToPing", m_bManualHostToPing);
	ini.WriteString(L"PingServer", m_sPingServer);

	ini.WriteBool(L"AutoMSS", m_bAutoMSS);
	ini.WriteInt(L"MSS", m_iMSS);

	ini.WriteBool(L"UseDoubleSendSize", m_bUseDoubleSendSize);


	ini.WriteBool(L"UseDownloadBandwidthThrottler", m_bUseDownloadBandwidthThrottler);
	ini.WriteInt(L"UseHyperDownload", m_uUseHyperDownload);
	ini.WriteInt(L"BCTimeDown", m_iBCTimeDown);
	ini.WriteInt(L"BCPriorityDown", m_iBCPriorityDown);
	ini.WriteInt(L"UseDownloadBuffer", m_uSetDownloadBuffer);
	ini.WriteInt(L"DownloadBufferLimit", m_iDownloadBufferSize);

	ini.WriteInt(L"UseBlockedQueue", m_uUseBlockedQueue);
	ini.WriteInt(L"BCTimeUp", m_iBCTimeUp);
	ini.WriteInt(L"BCPriorityUp", m_iBCPriorityUp);
	ini.WriteInt(L"UseUploadBuffer", m_uSetUploadBuffer);
	ini.WriteInt(L"UploadBufferLimit", m_iUploadBufferSize);

	ini.WriteInt(L"DatarateSamples", m_iDatarateSamples);


	ini.WriteBool(L"IncludeOverhead", m_bIncludeOverhead);
	ini.WriteBool(L"IncludeTCPAck", m_bIncludeTCPAck);
	ini.WriteBool(L"ConnectionsOverHead", m_bConnectionsOverHead);
	ini.WriteBool(L"SessionRatio", m_bSessionRatio);


	ini.WriteInt(L"DownloadControl", m_iDownloadControl);
	ini.WriteFloat(L"MinBCDownload", m_fMinBCDownload);
	ini.WriteFloat(L"MaxBCDownload", m_fMaxBCDownload);


	ini.WriteInt(L"UploadControl", m_iUploadControl);
	ini.WriteFloat(L"MinBCUpload",m_fMinBCUpload);
	ini.WriteFloat(L"MaxBCUpload",m_fMaxBCUpload);


	ini.WriteFloat(L"MaxDownStream", m_fMaxDownStream);
	ini.WriteFloat(L"MaxUpStream", m_fMaxUpStream);


	ini.WriteBool(L"MinimizeOpenedSlots", m_bMinimizeOpenedSlots);
	ini.WriteInt(L"CumulateBandwidth", m_uCumulateBandwidth);
	ini.WriteInt(L"BadwolfsUpload", m_uBadwolfsUpload);

	ini.WriteInt(L"MaxReservedSlots", m_iMaxReservedSlots);
	
	ini.WriteBool(L"IncreaseTrickleSpeed", m_bIncreaseTrickleSpeed);
	ini.WriteFloat(L"IncreaseTrickleSpeedValue", m_fIncreaseTrickleSpeed);

 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
	ini.WriteInt(L"SeparateReleaseBandwidth", m_uSeparateReleaseBandwidth);
	ini.WriteFloat(L"ReleaseSlotSpeed", m_fReleaseSlotSpeed);
	ini.WriteFloat(L"ReleaseBandwidthPercentage", m_fReleaseBandwidthPercentage);

	ini.WriteInt(L"SeparateFriendBandwidth", m_uSeparateFriendBandwidth);
	ini.WriteFloat(L"FriendSlotSpeed", m_fFriendSlotSpeed);
	ini.WriteFloat(L"FriendBandwidthPercentage", m_fFriendBandwidthPercentage);
 #endif // BW_MOD // NEO: BM END

	ini.WriteInt(L"MaxUploadSlots",m_iMaxUploadSlots);
	ini.WriteInt(L"MinUploadSlots",m_iMinUploadSlots);
	ini.WriteFloat(L"MaxUploadPerSlot",m_fUploadPerSlots);

	ini.WriteBool(L"OpenMoreSlotsWhenNeeded", m_bOpenMoreSlotsWhenNeeded);
	ini.WriteBool(L"CheckSlotDatarate", m_bCheckSlotDatarate);

	ini.WriteBool(L"IsTrickleBlocking", m_bIsTrickleBlocking);
	ini.WriteBool(L"IsDropBlocking", m_bIsDropBlocking);


	ini.WriteBool(L"DynUpGoingDivider", m_bDynUpGoingDivider);
	ini.WriteInt(L"DynUpGoingUpDivider", m_iDynUpGoingUpDivider);
	ini.WriteInt(L"DynUpGoingDownDivider", m_iDynUpGoingDownDivider);

	ini.WriteInt(L"UpMaxPingMethod", m_iUpMaxPingMethod);
	ini.WriteInt(L"BasePingUp", m_iBasePingUp);
	ini.WriteInt(L"PingUpTolerance", m_iPingUpTolerance);
	ini.WriteInt(L"PingUpProzent", m_iPingUpProzent);


	ini.WriteBool(L"DynDownGoingDivider", m_bDynDownGoingDivider);
	ini.WriteInt(L"DynDownGoingDivider", m_iDynDownGoingUpDivider);
	ini.WriteInt(L"DynDownGoingDownDivider", m_iDynDownGoingDownDivider);
	
	ini.WriteInt(L"DownMaxPingMethod", m_iDownMaxPingMethod);
	ini.WriteInt(L"BasePingDown", m_iBasePingDown);
	ini.WriteInt(L"PingDownTolerance", m_iPingDownTolerance);
	ini.WriteInt(L"PingDownProzent", m_iPingDownProzent);
#endif // NEO_BC // NEO: NBC END

	ini.WriteBool(L"TCPDisableNagle", m_bTCPDisableNagle);

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	ini.WriteBool(L"LancastEnabled", m_bLancastEnabled);

	ini.WriteInt(L"MaxLanDownload" , m_iMaxLanDownload); 
	ini.WriteBool(L"SetLanDownloadBuffer", m_bSetLanDownloadBuffer);
	ini.WriteInt(L"LanDownloadBufferSize", m_iLanDownloadBufferSize); 

	ini.WriteInt(L"MaxLanUpload", m_iMaxLanUpload); 
	ini.WriteBool(L"SetLanUploadBuffer", m_bSetLanUploadBuffer);
	ini.WriteInt(L"LanUploadBufferSize", m_iLanUploadBufferSize); 

	ini.WriteInt(L"MaxLanUploadSlots", m_iMaxLanUploadSlots); 

	ini.WriteBool(L"CustomizedLanCast", m_bCustomizedLanCast);
	ini.WriteString(L"LanCastGroup", m_sLanCastGroup);
	ini.WriteInt(L"LanCastPort", m_uLanCastPort);

	ini.WriteBool(L"CustomLanCastAdapter", m_bCustomLanCastAdapter);
	ini.WriteInt(L"LanCastAdapterIPAdress", m_uLanCastAdapterIPAdress);
	ini.WriteInt(L"LanCastAdapterSubNet", m_uLanCastAdapterSubNet);

	ini.WriteBool(L"AutoBroadcastLanFiles", m_bAutoBroadcastLanFiles);
	ini.WriteInt(L"AutoBroadcastLanFilesTimer" , m_iAutoBroadcastLanFiles); 

	ini.WriteBool(L"UseLanMultiTransfer", m_bUseLanMultiTransfer);
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	ini.WriteBool(L"UseVoodooTransfer", m_bUseVoodooTransfer);
	ini.WriteBool(L"SlaveAllowed", m_bSlaveAllowed);
	ini.WriteBool(L"SlaveHosting", m_bSlaveHosting);

	ini.WriteString(L"VoodooSpell",m_sVoodooSpell);
	ini.WriteInt(L"VoodooPort", m_nVoodooPort); 

	ini.WriteInt(L"AutoConnectVoodoo", m_uAutoConnectVoodoo);
	ini.WriteInt(L"VoodooReconectTime", m_iVoodooReconectTime);

	ini.WriteInt(L"UseVirtualVoodooFiles", m_uUseVirtualVoodooFiles);

	ini.WriteInt(L"VoodooCastEnabled", m_uVoodooCastEnabled);

	ini.WriteInt(L"SearchForSlaves", m_uSearchForSlaves); 
	ini.WriteInt(L"SearchForMaster", m_uSearchForMaster); 
	ini.WriteInt(L"VoodooSearchIntervals", m_iVoodooSearchIntervals); 

	ini.WriteBool(L"HideVoodooFiles", m_bHideVoodooFiles); // NEO: PP - [PasswordProtection]
#endif // VOODOO // NEO: VOODOO END

	// NEO: QS - [QuickStart]
	ini.WriteInt(L"UseQuickStart", m_uQuickStart);
	ini.WriteInt(L"QuickStartTime", m_iQuickStartTime);
	ini.WriteInt(L"QuickStartTimePerFile", m_iQuickStartTimePerFile);

	ini.WriteInt(L"QuickMaxConperFive", m_iQuickMaxConperFive);
	ini.WriteInt(L"QuickMaxHalfOpen", m_iQuickMaxHalfOpen);
	ini.WriteInt(L"QuickMaxConnections", m_iQuickMaxConnections);
	// NEO: QS END

	// NEO: RIC - [ReaskOnIDChange]
	ini.WriteInt(L"CheckIDChanges", m_uCheckIPChange);
	ini.WriteBool(L"InformClientsOnIDChange", m_bInformOnIPChange);
	ini.WriteBool(L"InformOnBuddyChange", m_bInformOnBuddyChange);
	ini.WriteBool(L"ReaskSourcesOnIDChange", m_bReAskOnIPChange);
	ini.WriteBool(L"RedoQuickStartOnIDChange", m_bQuickStartOnIPChange); // NEO: QS - [QuickStart]
	ini.WriteBool(L"CheckLowID2HidhIDChanges", m_bCheckL2HIDChange);
	ini.WriteBool(L"ReconnectKadOnIPChange", m_bReconnectKadOnIPChange);
	ini.WriteBool(L"RebindSocketsOnIPChange", m_bRebindSocketsOnIPChange);
	// NEO: RIC END

	// NEO: RKF - [RecheckKadFirewalled]
	ini.WriteBool(L"RecheckKadFirewalled", m_bRecheckKadFirewalled);
	ini.WriteInt(L"RecheckKadFirewalledTime", m_iRecheckKadFirewalled);
	// NEO: RKF END

	// NEO: RLD - [ReconnectOnLowID]
	ini.WriteBool(L"ReconnectOnLowId" , m_bReConnectOnLowID);
	ini.WriteInt(L"ReconnectOnLowIdRetries", m_iReConnectOnLowID); 
	// NEO: RLD END

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
	ini.WriteBool(L"NatTraversalEnabled", m_bNatTraversalEnabled);
	ini.WriteInt(L"LowIDUploadCallBack", m_uLowIDUploadCallBack); // NEO: LUC - [LowIDUploadCallBack]
	ini.WriteBool(L"ReuseTCPPort", m_bReuseTCPPort); // NEO: RTP - [ReuseTCPPort]
#endif //NATTUNNELING // NEO: NATT END

	// NEO: NPT - [NeoPartTraffic]
	ini.WriteInt(L"UsePartTraffic", m_uUsePartTraffic);
	ini.WriteInt(L"PartTrafficCollors", m_iPartTrafficCollors);
	// NEO: NPT END
	ini.WriteBool(L"UseClassicShareStatusBar", m_bUseClassicShareStatusBar); // NEO: MOD - [ClassicShareStatusBar]
	ini.WriteBool(L"UseShowSharePermissions", m_bUseShowSharePermissions); // NEO: SSP - [ShowSharePermissions]
	
	// NEO: SRS - [SmartReleaseSharing]
	ini.WriteInt(L"ReleaseChunks", m_iReleaseChunks);
	ini.WriteInt(L"LimitReleaseSlots", m_uReleaseSlotLimit);
	ini.WriteInt(L"ReleaseSlotLimit", m_iReleaseSlotLimit);
	// NEO: SRS END

	// NEO: NMFS - [NiceMultiFriendSlots]
	ini.WriteInt(L"LimitFriendSlots", m_uFriendSlotLimit);
	ini.WriteInt(L"FriendSlotLimit", m_iFriendSlotLimit);
	// NEO: NMFS END

	ini.WriteBool(L"SaveUploadQueueWaitTime", m_bSaveUploadQueueWaitTime); // NEO: SQ - [SaveUploadQueue]
	ini.WriteBool(L"UseMultiQueue", m_bUseMultiQueue); // NEO: MQ - [MultiQueue]
	ini.WriteInt(L"UseRandomQueue", m_uUseRandomQueue); // NEO: RQ - [RandomQueue]

	ini.WriteBool(_T("NeoScoreSystem"),m_bNeoScoreSystem); // NEO: NFS - [NeoScoreSystem]
	ini.WriteBool(_T("NeoCreditSystem"),m_bNeoCreditSystem); // NEO: NCS - [NeoCreditSystem]
	ini.WriteInt(_T("OtherCreditSystem"),m_iOtherCreditSystem); // NEO: OCS - [OtherCreditSystems]

	// NEO: TQ - [TweakUploadQueue]
	ini.WriteBool(L"UseInfiniteQueue", m_bUseInfiniteQueue);

	ini.WriteInt(L"UseQueueOverFlowDef", m_uQueueOverFlowDef);
	ini.WriteInt(L"QueueOverFlowDef", m_iQueueOverFlowDef);
	ini.WriteInt(L"UseQueueOverFlowEx", m_uQueueOverFlowEx);
	ini.WriteInt(L"QueueOverFlowEx", m_iQueueOverFlowEx);
	ini.WriteInt(L"UseQueueOverFlowRelease", m_uQueueOverFlowRelease);
	ini.WriteInt(L"QueueOverFlowRelease",m_iQueueOverFlowRelease);
	ini.WriteInt(L"UseQueueOverFlowCF", m_uQueueOverFlowCF);
	ini.WriteInt(L"QueueOverFlowCF", m_iQueueOverFlowCF);
	// NEO: TQ END

	// NEO: PRSF - [PushSmallRareFiles]
	ini.WriteBool(L"PushSmallFiles", m_bPushSmallFiles);
	ini.WriteInt(L"PushSmallFilesSize", m_iPushSmallFilesSize);
	ini.WriteBool(L"PushRareFiles", m_bPushRareFiles);
	ini.WriteInt(L"PushRareFilesValue", m_iPushRareFilesValue);
	ini.WriteBool(L"PushRatioFiles", m_bPushRatioFiles);
	ini.WriteInt(L"PushRatioFilesValue", m_iPushRatioFilesValue);
	// NEO: PRSF END

	// NEO: NXC - [NewExtendedCategories]
	ini.WriteBool(_T("ShowCatNames"),m_bShowCatNames);
	ini.WriteInt(_T("ShowCategoryFlags"),m_uShowCategoryFlags);
	ini.WriteBool(_T("ActiveCatDefault"), m_bActiveCatDefault);
	ini.WriteBool(_T("SelCatOnAdd"), m_bSelCatOnAdd);
	ini.WriteBool(_T("AutoSetResumeOrder"), m_bAutoSetResumeOrder);
	ini.WriteBool(_T("SmallFileDLPush"), m_bSmallFileDLPush);
	ini.WriteInt(_T("SmallFileDLPushSize"), m_iSmallFileDLPush);
	ini.WriteBool(_T("StartDLInEmptyCats"), m_bStartDLInEmptyCats);
	ini.WriteInt(_T("StartDLInEmptyCatsAmount"), m_iStartDLInEmptyCats);
	ini.WriteBool(_T("UseAutoCat"), m_bUseAutoCat);
	ini.WriteBool(_T("CheckAlreadyDownloaded"), m_bCheckAlreadyDownloaded);
	
	ini.WriteBool(_T("StartNextFileByPriority"), m_bStartNextFileByPriority);

	ini.WriteBool(_T("SmartA4AFSwapping"), m_bSmartA4AFSwapping);
	ini.WriteInt(_T("AdvancedA4AFMode"), m_iAdvancedA4AFMode);
	// NEO: NXC END

	// NEO: NTB - [NeoToolbarButtons]
	ini.WriteInt(_T("NeoToolbar"), m_uNeoToolbar);
	m_iNeoToolbarButtonCount = m_NeoToolbarButtons.GetCount(); 
	ini.WriteInt(_T("NeoToolbarButtonCount"), m_iNeoToolbarButtonCount);
	for (int i=0; i<m_iNeoToolbarButtonCount; i++)
		ini.WriteInt(StrLine(L"%s_%u",_T("NeoToolbarButtons"),i), m_NeoToolbarButtons[i]);
	// NEO: NTB END

#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase]
	ini.WriteBool(L"EnableSourceList", m_bEnableSourceList);
	ini.WriteInt(L"TableAmountToStore", m_iTableAmountToStore);
	// cleanup
	ini.WriteInt(L"SourceListRunTimeCleanUp", m_uSourceListRunTimeCleanUp);
	ini.WriteInt(L"SourceListExpirationTime", m_iSourceListExpirationTime);
	// NEO: SFL - [SourceFileList]
	// seen files
	ini.WriteBool(L"SaveSourceFileList", m_bSaveSourceFileList);
	ini.WriteInt(L"FileListExpirationTime", m_iFileListExpirationTime);
	// NEO: SFL END
	// security
	ini.WriteBool(L"UseIPZoneCheck", m_bUseIPZoneCheck);
	ini.WriteBool(L"SourceHashMonitor", m_bSourceHashMonitor); // NEO: SHM - [SourceHashMonitor]
	// aditional obtions
	ini.WriteInt(L"IgnoreUndefinedIntervall", m_iIgnoreUndefinedIntervall);
	ini.WriteInt(L"IgnoreUnreachableInterval", m_iIgnoreUnreachableInterval);
#endif // NEO_CD // NEO: NCD END
#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	ini.WriteBool(L"EnableSourceAnalizer", m_bEnableSourceAnalizer);
	ini.WriteInt(L"AnalyzeIntervals", m_iAnalyzeIntervals);
	ini.WriteInt(L"TableAmountToAnalyze", m_iTableAmountToAnalyze);
	// gap handling
	ini.WriteBool(L"HandleTableGaps", m_bHandleTableGaps);
	ini.WriteInt(L"PriorityGapRatio", m_fPriorityGapRatio);
	ini.WriteInt(L"MaxGapSize", m_iMaxGapSize);
	ini.WriteInt(L"MaxGapTime", m_iMaxGapTime);
	// analyze obtions
	ini.WriteInt(L"MaxMidleDiscrepanceHigh", m_fMaxMidleDiscrepanceHigh);
	ini.WriteInt(L"MaxMidleDiscrepanceLow", m_fMaxMidleDiscrepanceLow);
	ini.WriteBool(L"UseDualLinkedTableGravity", m_bDualLinkedTableGravity);
	ini.WriteInt(L"DualLinkedTableGravity", m_iDualLinkedTableGravity);
	// calculation obtions
	ini.WriteInt(L"EnhancedFactor", m_fEnhancedFactor);
	ini.WriteInt(L"FreshSourceTreshold", m_iFreshSourceTreshold);
	ini.WriteInt(L"TempralIPBorderLine", m_iTempralIPBorderLine);
	// additional obtions
	ini.WriteInt(L"LastSeenDurationThreshold", m_fLastSeenDurationThreshold);
	ini.WriteBool(L"LinkTimePropability", m_bLinkTimePropability);
	ini.WriteInt(L"LinkTimeThreshold", m_iLinkTimeThreshold);
	ini.WriteBool(L"ScaleReliableTime", m_bScaleReliableTime);
	ini.WriteInt(L"MaxReliableTime", m_iMaxReliableTime);
#endif // NEO_SA // NEO: NSA END

#ifdef ARGOS // NEO: NA - [NeoArgos] 
	ini.WriteBool(L"ZeroScoreGPLBreaker", m_bZeroScoreGPLBreaker);
	ini.WriteInt(L"BanTime", m_iBanTime);
	ini.WriteBool(L"CloseMaellaBackdoor", m_bCloseMaellaBackdoor);

	// DLP Groupe
	ini.WriteBool(L"LeecherModDetection", m_bLeecherModDetection);
	ini.WriteBool(L"LeecherNickDetection", m_bLeecherNickDetection);
	ini.WriteBool(L"LeecherHashDetection", m_bLeecherHashDetection);
	ini.WriteInt(L"DetectionLevel", m_iDetectionLevel);

	// Behavioural groupe
	ini.WriteInt(L"AgressionDetection", m_uAgressionDetection);
	ini.WriteInt(L"HashChangeDetection", m_uHashChangeDetection);

	ini.WriteBool(L"UploadFakerDetection", m_bUploadFakerDetection);
	ini.WriteBool(L"FileFakerDetection", m_bFileFakerDetection);
	ini.WriteBool(L"RankFloodDetection", m_bRankFloodDetection);
	ini.WriteBool(L"XsExploitDetection", m_bXsExploitDetection);
	ini.WriteBool(L"FileScannerDetection", m_bFileScannerDetection);
	ini.WriteInt(L"SpamerDetection", m_uSpamerDetection);

	ini.WriteBool(L"HashThiefDetection", m_bHashThiefDetection);
	ini.WriteInt(L"NickThiefDetection", m_uNickThiefDetection);
	ini.WriteBool(L"ModThiefDetection", m_bModThiefDetection);
#endif //ARGOS // NEO: NA END

#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry]
	ini.WriteInt(L"IP2CountryNameMode", m_iIP2CountryNameMode);
	ini.WriteInt(L"IP2CountryShowFlag", m_uIP2CountryShowFlag);
#endif // IP2COUNTRY // NEO: IP2C END

	// NEO: NMX - [NeoMenuXP]
	ini.WriteBool(L"ShowXPSideBar", m_bShowXPSideBar);
	ini.WriteBool(L"ShowXPBitmap", m_bShowXPBitmap);
	ini.WriteInt(L"XPMenuStyle", m_iXPMenuStyle);
	ini.WriteBool(L"GrayMenuIcon", m_bGrayMenuIcon);
	// NEO: NMX END 
}

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
CString CNeoPreferences::GetLanCastGroup()
{ 
	return (IsCustomizedLanCast() ? m_sLanCastGroup : LANCAST_GROUP); 
}
uint16 CNeoPreferences::GetLanCastPort()
{ 
	return (IsCustomizedLanCast() ? m_uLanCastPort : LANCAST_PORT); 
}
#endif //LANCAST // NEO: NLC END
// NEO: NCFG END <-- Xanatos --