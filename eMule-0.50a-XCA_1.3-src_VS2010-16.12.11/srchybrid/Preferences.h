//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#pragma once

const CString strDefaultToolbar = _T("009901020304050607990809"); 

enum EViewSharedFilesAccess{
	vsfaEverybody = 0,
	vsfaFriends = 1,
	vsfaNobody = 2
};

enum ENotifierSoundType{
	ntfstNoSound = 0,
	ntfstSoundFile = 1,
	ntfstSpeech = 2
};

enum EDefaultDirectory{
	EMULE_CONFIGDIR = 0,
	EMULE_TEMPDIR = 1,
	EMULE_INCOMINGDIR = 2,
	EMULE_LOGDIR = 3,
	EMULE_ADDLANGDIR = 4, // directories with languages installed by the eMule (parent: EMULE_EXPANSIONDIR)
	EMULE_INSTLANGDIR = 5, // directories with languages installed by the user or installer (parent: EMULE_EXECUTEABLEDIR)
	EMULE_WEBSERVERDIR = 6,
	EMULE_SKINDIR = 7,
	EMULE_DATABASEDIR = 8, // the parent directory of the incoming/temp folder
	EMULE_CONFIGBASEDIR = 9, // the parent directory of the config folder 
	EMULE_EXECUTEABLEDIR = 10, // assumed to be not writeable (!)
	EMULE_TOOLBARDIR = 11,
	EMULE_EXPANSIONDIR = 12 // this is a base directory accessable for all users for things eMule installs
};
enum EPort{
	PORT_4662,
	PORT_OTHER,
	PORT_PEERCACHE,
	PORT_COUNT
};
enum ESoft{
	SOFT_EMULE,
	SOFT_EDONKEYHYBRID,
	SOFT_EDONKEY,
	SOFT_AMULE,
	SOFT_MLDONKEY,
	SOFT_SHAREAZA,
	SOFT_EMULECOMPAT,
	SOFT_URL
};
enum EFileSizeFormat {
	fsizeDefault,
	fsizeKByte,
	fsizeMByte
};

#define DOWN_SOFT_COUNT 8
#define UP_SOFT_COUNT (DOWN_SOFT_COUNT-1)
enum EToolbarLabelType;
enum ELogFileFormat;

//Xman process prio
// [TPT] - Select process priority 
/*
#define PROCESSPRIORITYNUMBER 6
static	const DWORD PriorityClasses[] = { REALTIME_PRIORITY_CLASS, HIGH_PRIORITY_CLASS, ABOVE_NORMAL_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS, BELOW_NORMAL_PRIORITY_CLASS, IDLE_PRIORITY_CLASS };
*/
// [TPT] - Select process priority 
#define MAXUSERNICKLENGTH 50
// DO NOT EDIT VALUES like making a uint16 to uint32, or insert any value. ONLY append new vars
#pragma pack(1)
struct Preferences_Ext_Struct{
	uint8	version;
	uchar	userhash[16];
	WINDOWPLACEMENT EmuleWindowPlacement;
};
#pragma pack()

// deadlake PROXYSUPPORT
struct ProxySettings{
	uint16		type;
	uint16		port;
	CStringA	name;
	CStringA	user;
	CStringA	password;
	bool		EnablePassword;
	bool		UseProxy;
};

struct Category_Struct{
	CString	strIncomingPath;
	CString	strTempPath;// X: [TD] - [TempDir]
	CString	strTitle;
	CString	strComment;
	DWORD	color;
	UINT	prio;
	CString autocat;
	CString	regexp;
	size_t	filter;
	bool	filterNeg;
	bool	care4all;
	bool	ac_regexpeval;
	bool	downloadInAlphabeticalOrder; // ZZ:DownloadManager
};

class CPreferences
{
public:
	static	CString	strNick;
	// ZZ:UploadSpeedSense -->
//	static	uint16	minupload;
	// ZZ:UploadSpeedSense <--
	static	LPCSTR	m_pszBindAddrA;
	static	CStringA m_strBindAddrA;
	static	LPCWSTR	m_pszBindAddrW;
	static	CStringW m_strBindAddrW;
	static	uint16	port;
	static	uint16	udpport;
	static	uint16	nServerUDPPort;
	static	UINT	maxconnections;
	static	UINT	maxhalfconnections;
	static	bool	m_bConditionalTCPAccept;
	static	bool	reconnect;
	static	bool	m_bUseServerPriorities;
	static	bool	m_bUseUserSortedServerList;
	static	CString	m_strIncomingDir;
	static	CAtlArray<CString>	tempdir;
	static	bool	ICH;
	static	bool	m_bAutoUpdateServerList;
	static	bool	updatenotify;
	static	bool	mintotray;
	static	bool	autoconnect;
	static	bool	m_bAutoConnectToStaticServersOnly; // Barry
	static	bool	autotakeed2klinks;	   // Barry
	static	bool	addnewfilespaused;	   // Barry
	static	UINT	depth3D;			   // Barry
	static	int		m_iStraightWindowStyles;
	static	bool	m_bEnableMiniMule;
	static	bool	m_bUseSystemFontForMainControls;
	static	bool	m_bRTLWindowsLayout;
	static	bool	m_bFillGraphs;
	static	bool	beepOnError;
	static	bool	confirmExit;
	static	CString	m_strSkinProfile;
	static	CString	m_strSkinProfileDir;
	static	UINT	maxsourceperfile;
	static	UINT	trafficOMeterInterval;
	static	UINT	statsInterval;
	static	uchar	userhash[16];
	static	WINDOWPLACEMENT EmuleWindowPlacement;
	static	DWORD	m_adwStatsColors[14];
	static	bool	bHasCustomTaskIconColor;
	static	bool	m_bIconflashOnNewMessage;

	static	bool	splashscreen;
	static	bool	filterLANIPs;
#ifdef _DEBUG
	static	bool	m_bAllocLocalHostIP;
#endif
	static	bool	onlineSig;

	// -khaos--+++> Struct Members for Storing Statistics

	// Saved stats for cumulative downline overhead...
	static	uint64	cumDownOverheadTotal;
	static	uint64	cumDownOverheadFileReq;
	static	uint64	cumDownOverheadSrcEx;
	static	uint64	cumDownOverheadServer;
	static	uint64	cumDownOverheadKad;
	static	uint64	cumDownOverheadTotalPackets;
	static	uint64	cumDownOverheadFileReqPackets;
	static	uint64	cumDownOverheadSrcExPackets;
	static	uint64	cumDownOverheadServerPackets;
	static	uint64	cumDownOverheadKadPackets;

	// Saved stats for cumulative upline overhead...
	static	uint64	cumUpOverheadTotal;
	static	uint64	cumUpOverheadFileReq;
	static	uint64	cumUpOverheadSrcEx;
	static	uint64	cumUpOverheadServer;
	static	uint64	cumUpOverheadKad;
	static	uint64	cumUpOverheadTotalPackets;
	static	uint64	cumUpOverheadFileReqPackets;
	static	uint64	cumUpOverheadSrcExPackets;
	static	uint64	cumUpOverheadServerPackets;
	static	uint64	cumUpOverheadKadPackets;

	// Saved stats for cumulative upline data...
	static	uint32	cumUpSuccessfulSessions;
	static	uint32	cumUpFailedSessions;
	static	uint32	cumUpAvgTime;
	// Cumulative client breakdown stats for sent bytes...
	static	uint64	cumUpData[UP_SOFT_COUNT];
	// Session client breakdown stats for sent bytes...
	static	uint64	sesUpData[UP_SOFT_COUNT];

	// Cumulative port breakdown stats for sent bytes...
	static	uint64	cumUpDataPort[PORT_COUNT];
	// Session port breakdown stats for sent bytes...
	static	uint64	sesUpDataPort[PORT_COUNT];

	// Cumulative source breakdown stats for sent bytes...
	static	uint64	cumUpData_File;
	static	uint64	cumUpData_Partfile;
	// Session source breakdown stats for sent bytes...
	static	uint64	sesUpData_File;
	static	uint64	sesUpData_Partfile;

	// Saved stats for cumulative downline data...
	static	uint32	cumDownCompletedFiles;
	static	uint32	cumDownSuccessfulSessions;
	static	uint32	cumDownFailedSessions;
	static	uint32	cumDownAvgTime;

	// Cumulative statistics for saved due to compression/lost due to corruption
	static	uint64	cumLostFromCorruption;
	static	uint64	cumSavedFromCompression;
	static	uint32	cumPartsSavedByICH;

	// Session statistics for download sessions
	static	uint32	sesDownSuccessfulSessions;
	static	uint32	sesDownFailedSessions;
	static	uint32	sesDownAvgTime;
	static	uint32	sesDownCompletedFiles;
	static	uint64	sesLostFromCorruption;
	static	uint64	sesSavedFromCompression;
	static	uint32	sesPartsSavedByICH;

	// Cumulative client breakdown stats for received bytes...
	static	uint64	cumDownData[DOWN_SOFT_COUNT];
	// Session client breakdown stats for received bytes...
	static	uint64	sesDownData[DOWN_SOFT_COUNT];

	// Cumulative port breakdown stats for received bytes...
	static	uint64	cumDownDataPort[PORT_COUNT];
	// Session port breakdown stats for received bytes...
	static	uint64	sesDownDataPort[PORT_COUNT];

	// Saved stats for cumulative connection data...
	static	float	cumConnAvgDownRate;
	static	float	cumConnMaxAvgDownRate;
	static	float	cumConnMaxDownRate;
	static	float	cumConnAvgUpRate;
	static	float	cumConnMaxAvgUpRate;
	static	float	cumConnMaxUpRate;
	static	time_t	cumConnRunTime;
	static	uint32	cumConnNumReconnects;
	static	uint32	cumConnAvgConnections;
	static	uint32	cumConnMaxConnLimitReached;
	static	uint32	cumConnPeakConnections;
	static	uint32	cumConnTransferTime;
	static	uint32	cumConnDownloadTime;
	static	uint32	cumConnUploadTime;
	static	uint32	cumConnServerDuration;

	// Saved records for servers / network...
	static	uint32	cumSrvrsMostWorkingServers;
	static	uint32	cumSrvrsMostUsersOnline;
	static	uint32	cumSrvrsMostFilesAvail;

	// Saved records for shared files...
	static	uint32	cumSharedMostFilesShared;
	static	uint64	cumSharedLargestShareSize;
	static	uint64	cumSharedLargestAvgFileSize;
	static	uint64	cumSharedLargestFileSize;

	// Save the date when the statistics were last reset...
	static	time_t	stat_datetimeLastReset;

	// Save new preferences for PPgStats
	static	UINT	statsConnectionsGraphRatio; // This will store the divisor, i.e. for 1:3 it will be 3, for 1:20 it will be 20.
	// Save the expanded branches of the stats tree
	static	CString	m_strStatsExpandedTreeItems;

	static	UINT	statsSaveInterval;
	static  bool	m_bShowVerticalHourMarkers;
	// <-----khaos- End Statistics Members


	// Original Stats Stuff
	static	uint64	totalDownloadedBytes;
	static	uint64	totalUploadedBytes;
	// End Original Stats Stuff
	static	WORD	m_wLanguageID;
	static	bool	transferDoubleclick;
	static	EViewSharedFilesAccess m_iSeeShares;
	static	UINT	m_iToolDelayTime;	// tooltip delay time in seconds
	static	bool	bringtoforeground;
	static	UINT	splitterbarPosition;
	static	UINT	splitterbarPositionSvr;

	static	UINT	m_uTransferWnd1;
	static	UINT	m_uTransferWnd2;
	//MORPH START - Added by SiRoB, Splitting Bar [O?]
	static	UINT	splitterbarPositionStat;
	static	UINT	splitterbarPositionStat_HL;
	static	UINT	splitterbarPositionStat_HR;
	static	UINT	splitterbarPositionFriend;
	static	UINT	splitterbarPositionShared;
	//MORPH END - Added by SiRoB, Splitting Bar [O?]
	static	UINT	m_uDeadServerRetries;
	static	DWORD	m_dwServerKeepAliveTimeout;
	// -khaos--+++> Changed data type to avoid overflows
	static	UINT	statsMax;
	// <-----khaos-
	static	UINT	statsAverageMinutes;

	static	CString	notifierConfiguration;
	static	bool	notifierOnDownloadFinished;
	static	bool	notifierOnNewDownload;
	static	bool	notifierOnChat;
	static	bool	notifierOnLog;
	static	bool	notifierOnImportantError;
	static	bool	notifierOnEveryChatMsg;
	static	bool	notifierOnNewVersion;
	static	ENotifierSoundType notifierSoundType;
	static	CString	notifierSoundFile;

	static	bool	m_bMessageEnableSmileys;

	static	bool	m_bRemove2bin;
	static	bool	m_bShowCopyEd2kLinkCmd;
	static	bool	m_bpreviewprio;
	static	bool	m_bSmartServerIdCheck;
	static	uint8	smartidstate;
	static	bool	m_bSafeServerConnect;
	static	bool	startMinimized;
	static	bool	m_bAutoStart;
	static	bool	m_bRestoreLastMainWndDlg;
	static	int		m_iLastMainWndDlgID;
	static	bool	m_bRestoreLastLogPane;
	static	int		m_iLastLogPaneID;
	static	UINT	MaxConperFive;
	static	bool	checkDiskspace;
	static	UINT	m_uMinFreeDiskSpace;
	static	bool	m_bSparsePartFiles;
	static	CString	m_strYourHostname;
	static	bool	m_bEnableVerboseOptions;
	static	bool	m_bVerbose;
	static	bool	m_bFullVerbose;
	static  int		m_byLogLevel;
	static	bool	m_bDebugSourceExchange; // Sony April 23. 2003, button to keep source exchange msg out of verbose log
	static	bool	m_bLogBannedClients;
	static	bool	m_bLogRatingDescReceived;
#ifdef CLIENTANALYZER
	static	bool	m_bLogAnalyzerEvents;
#endif
	static	bool	m_bLogSecureIdent;
	static	bool	m_bLogFilteredIPs;
	static	bool	m_bLogFileSaving;
	static	bool    m_bLogA4AF; // ZZ:DownloadManager
	static	bool	m_bLogDrop; //Xman Xtreme Downloadmanager
	static	bool	m_bLogpartmismatch; //Xman Log part/size-mismatch
	static	bool	m_bLogUlDlEvents;
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	static	bool	m_bUseDebugDevice;
	static	int		m_iDebugServerTCPLevel;
	static	int		m_iDebugServerUDPLevel;
	static	int		m_iDebugServerSourcesLevel;
	static	int		m_iDebugServerSearchesLevel;
	static	int		m_iDebugClientTCPLevel;
	static	int		m_iDebugClientUDPLevel;
	static	int		m_iDebugClientKadUDPLevel;
	static	int		m_iDebugSearchResultDetailLevel;
#endif
	static	bool	m_bupdatequeuelist;
	static	bool	m_bManualAddedServersHighPriority;
	static	bool	m_btransferfullchunks;
	static	bool	m_bshowoverhead;
	static	int		m_istartnextfile;
	static	bool	m_bDAP;
	static	bool	m_bUAP;
	static	bool	m_bDisableKnownClientList;
	static	bool	m_bDisableQueueList;
	static	bool	m_bExtControls;
	static	bool	m_bTransflstRemain;

	static	UINT	versioncheckdays;
	static	bool	showRatesInTitle;

	static	CString	m_strTxtEditor;
	static	CString	m_strVideoPlayer;
	static	CString	m_strVideoPlayerArgs;
	static	bool	moviePreviewBackup;
	static	int		m_iPreviewSmallBlocks;
	static	bool	m_bPreviewCopiedArchives;
	static	int		m_iInspectAllFileTypes;
	static	bool	m_bPreviewOnIconDblClk;
	static	bool	m_bCheckFileOpen;
	static	bool	indicateratings;
	static	bool	watchclipboard;
	static	bool	filterserverbyip;
	static	bool	m_bFirstStart;
#ifdef CLIENTANALYZER
	static	uint8	m_uiCreditSystem;
#else
	static	bool	m_bCreditSystem;
#endif

	static	bool	log2disk;
	static	bool	debug2disk;
	static	int		iMaxLogBuff;
	static	UINT	uMaxLogFileSize;
	static	ELogFileFormat m_iLogFileFormat;
	static	bool	scheduler;
	static	bool	msgonlyfriends;
	static	bool	msgsecure;
	static	bool	m_bUseChatCaptchas;

	static	UINT	filterlevel;
	//static	UINT	m_uFileBufferSize;// X: [GB] - [Global Buffer]
	static	UINT	m_iQueueSize;
	static	int		m_iCommitFiles;
	//static	UINT	m_uFileBufferTimeLimit;// X: [GB] - [Global Buffer]

	static	UINT	maxmsgsessions;	
	static	uint64	versioncheckLastAutomatic;
	
	static	CString	messageFilter;
	static	CString	commentFilter;
	static	CString	filenameCleanups;
	static	CString	m_strDateTimeFormat;
	static	CString	m_strDateTimeFormat4Log;
	static	CString	m_strDateTimeFormat4Lists;
	static	LOGFONT m_lfHyperText;
	static	LOGFONT m_lfLogText;
	static	COLORREF m_crLogError;
	static	COLORREF m_crLogWarning;
	static	COLORREF m_crLogSuccess;
	static	int		m_iExtractMetaData;
	static	bool	m_bAdjustNTFSDaylightFileTime;
	static	bool	m_bRearrangeKadSearchKeywords;
	static  bool    m_bAllocFull;
	static	bool	m_bShowSharedFilesDetails;
	static  bool	m_bShowWin7TaskbarGoodies;
	static  bool	m_bShowUpDownIconInTaskbar;
	static	bool	m_bForceSpeedsToKB;
	static	bool	m_bAutoShowLookups;
	static	bool	m_bExtraPreviewWithMenu;


	// Web Server [kuchin]
	static	CString	m_strWebPassword;
	static	CString	m_strWebLowPassword;
	static	uint16	m_nWebPort;
	static	bool	m_bWebUseUPnP;
	static	bool	m_bWebEnabled;
	static	bool	m_bWebUseGzip;
	static	bool	m_bWebLowEnabled;
	static	int		m_nWebPageRefresh;
	static	int		m_iWebTimeoutMins;
	static	int		m_iWebFileUploadSizeLimitMB;
	static	CString	m_strTemplateFile;
	static	ProxySettings proxy; // deadlake PROXYSUPPORT
	static	bool	m_bAllowAdminHiLevFunc;
	static	CAtlArray<UINT> m_aAllowedRemoteAccessIPs;

	static	bool	showCatTabInfos;
//	static	bool	resumeSameCat;
	static	bool	dontRecreateGraphs;
	static	bool	autofilenamecleanup;
	//static	int		allcatType;
	//static	bool	allcatTypeNeg;
	static	bool	m_bUseAutocompl;
	static	bool	m_bShowDwlPercentage;
	static	bool	m_bRemoveFinishedDownloads;
	static	bool	m_bShowActiveDownloadsBold;
	static	UINT	m_iMaxChatHistory;

	static	int		m_iSearchMethod;
	static	bool	m_bAdvancedSpamfilter;
#ifndef CLIENTANALYZER
	static	bool	m_bUseSecureIdent;
#endif
	static	bool	networkkademlia;
	static	bool	networked2k;

	// toolbar
	static	EToolbarLabelType m_nToolbarLabels;
	static	CString	m_sToolbarBitmap;
	static	CString	m_sToolbarBitmapFolder;
	static	CString	m_sToolbarSettings;
	static	bool	m_bReBarToolbar;
	static	CSize	m_sizToolbarIconSize;

	static	bool	m_bWinaTransToolbar;
	static	bool	m_bShowDownloadToolbar;

	//preview
	static	bool	m_bPreviewEnabled;
	static	bool	m_bAutomaticArcPreviewStart;

	// ZZ:UploadSpeedSense -->
//	static	bool	m_bDynUpEnabled;
//	static	int		m_iDynUpPingTolerance;
//	static	int		m_iDynUpGoingUpDivider;
//	static	int		m_iDynUpGoingDownDivider;
//	static	int		m_iDynUpNumberOfPings;
//	static	int		m_iDynUpPingToleranceMilliseconds;
//	static	bool	m_bDynUpUseMillisecondPingTolerance;
	// ZZ:UploadSpeedSense <--

	//static	bool     m_bA4AFSaveCpu; // ZZ:DownloadManager

	static	bool     m_bHighresTimer;

	static	bool	m_bResolveSharedShellLinks;
	static	CAtlList<CString> shareddir_list;
	static	CAtlList<CString> addresses_list;
	static	bool	m_bKeepUnavailableFixedSharedDirs;

#ifdef _DEBUG// X: [RDL] - [Remove Debug Log]
	static	int		m_iDbgHeap;
#endif
	static	UINT	m_nWebMirrorAlertLevel;
	static	bool	m_bRunAsUser;
	static	bool	m_bPreferRestrictedOverUser;

	static	bool	m_bUseOldTimeRemaining;

	// X: [RPC] - [Remove PeerCache]
	// PeerCache
	/*static	uint64	m_uPeerCacheLastSearch;
	static	bool	m_bPeerCacheWasFound;
	static	bool	m_bPeerCacheEnabled;
	static	uint16	m_nPeerCachePort;
//	static	bool	m_bPeerCacheShow;
*/
	// Firewall settings
	static bool		m_bOpenPortsOnStartUp;

	//AICH Options
	static bool		m_bTrustEveryHash;

	// files
	static bool		m_bRememberCancelledFiles;
	static bool		m_bRememberDownloadedFiles;
	static bool		m_bPartiallyPurgeOldKnownFiles;

	
	// encryption / obfuscation / verification
	static bool		m_bCryptLayerRequested;
	static bool		m_bCryptLayerSupported;
	static bool		m_bCryptLayerRequired;
	static uint8	m_byCryptTCPPaddingLength;
	static uint32   m_dwKadUDPKey;

	static bool		m_bSkipWANIPSetup;
	static bool		m_bSkipWANPPPSetup;
	static bool		m_bEnableUPnP;
	static bool		m_bCloseUPnPOnExit;
	//static bool		m_bIsWinServImplDisabled;
	static bool		m_bIsMinilibImplDisabled;
	static bool		m_bIsACATImplDisabled;
	static bool		m_bUPnPTryRandom; // Try to use random external port if already in use On/Off
	static int		m_nLastWorkingImpl;

	static	bool GetUPnPNatTryRandom()  { return m_bUPnPTryRandom; }
	static	void SetUPnPNatTryRandom(bool on) { m_bUPnPTryRandom = on; }
	//zz_fly :: Rebind UPnP on IP-change
	static  bool m_bUPnPRebindOnIPChange;
	static  bool GetUPnPNatRebind() { return m_bUPnPRebindOnIPChange; }
	static	void SetUPnPNatRebind(bool on) { m_bUPnPRebindOnIPChange = on; }
	//zz_fly :: Rebind UPnP on IP-change end

	// Spam
	static bool		m_bEnableSearchResultFilter;

	static BOOL		m_bIsRunningAeroGlass;
	static bool		m_bPreventStandby;
	static bool		m_bStoreSearches;
	//X
	static	bool	m_bEnable64BitTime;// X: [E64T] - [Enable64BitTime]
	static	bool	refuseupload;// X: [RU] - [RefuseUpload]
	static	bool	m_bPauseOnFileComplete; // NEO: POFC - [PauseOnFileComplete]
	static	bool	dontcompressext;// X: [DCE] - [DontCompressExt]

	static	CString	compressExt;// X: [DCE] - [DontCompressExt]
	static	uint32	rumax;// X: [RU] - [RefuseUpload]

	static	bool	prefReadonly; // X: [ROP] - [ReadOnlyPreference]
	static	uint8	queryOnHashing; // X: [QOH] - [QueryOnHashing]
	static	uint8	lastTranWndCatID;// X: [RCI] - [Remember Catalog ID]
	static	bool	dontsharext;// X: [DSE] - [DontShareExt]
	static	CString	shareExt;// X: [DSE] - [DontShareExt]

	static	bool	noIPFilterDesc;// X: [NIPFD] - [No IPFilter Description]

	static	CString	speedgraphwindowConfiguration; // X: [SGW] - [SpeedGraphWnd]
	static	int		speedgraphwindowLeft;
	static	int		speedgraphwindowTop;
	static	bool	showSpeedGraph;
	static	bool	randomPortOnStartup; // X: [RPOS] - [RandomPortOnStartup]
	// NEO: QS - [QuickStart]
	static	bool	m_uQuickStart;
	static	int		m_iQuickStartTime;
	static	int		m_iQuickStartTimePerFile;
	static	UINT	m_iQuickMaxConperFive;
	static	UINT	m_iQuickMaxHalfOpen;
	static	UINT	m_iQuickMaxConnections;

	static	bool	m_bOnQuickStart;
	// NEO: QS END

// ==> Pay Back First [AndCycle/SiRoB/Stulle] - Stulle
	static bool		m_bPayBackFirst;
	static uint8	m_iPayBackFirstLimit;
	//static bool		m_bPayBackFirst2;
	//static uint16	m_iPayBackFirstLimit2;
	// <== Pay Back First [AndCycle/SiRoB/Stulle] - Stulle

	// ==> Invisible Mode [TPT/MoNKi] - Stulle
	static	bool		m_bInvisibleMode;		
	static	char		m_cInvisibleModeHotKey;
	static	bool		m_bInvisibleModeStart;
	static	int			m_iInvisibleModeHotKeyModifier;
	// <== Invisible Mode [TPT/MoNKi] - Stulle
	static	UINT		m_uGlobalBufferSize;// X: [GB] - [Global Buffer]

	static	EFileSizeFormat m_eFileSizeFormat;
	static	bool		m_bMediaInfo_RIFF;
	static	bool		m_bMediaInfo_RM;
	static	bool		m_bMediaInfo_ID3LIB;
	static	bool		m_bMediaInfo_WM;
	static	bool		m_bMediaInfo_MediaDet;
	static	bool		m_bMiniMuleAutoClose;
	static	UINT		m_uMiniMuleTransparency;

	static	CString		m_strMediaInfoDllPath;
	static	CString		m_strNotifierMailEncryptCertName;
	static	CString		m_strInternetSecurityZone;

	enum Table
	{
		tableDownload, 
		tableUpload, 
		tableQueue, 
		tableSearch,
		tableShared, 
		tableServer, 
		tableClientList,
		tableFilenames,
		tableDownloadClients,
		tableHistory		//Xman [MoNKi: -Downloaded History-]
	};

	friend class CPreferencesWnd;
	friend class CPPgGeneral;
	friend class CPPgConnection;
	friend class CPPgServer;
	friend class CPPgDirectories;
	friend class CPPgFiles;
	friend class CPPgNotify;
	friend class CPPgTweaks;
	friend class CPPgDisplay;
	friend class CPPgSecurity;
	friend class CPPgScheduler;
	friend class CPPgDebug;

	CPreferences();
	~CPreferences();

	static	void	Init();
	static	void	Uninit();

	static	LPCTSTR GetTempDir(size_t id = 0)				{return (LPCTSTR)tempdir.GetAt((id < tempdir.GetCount()) ? id : 0);}
	static	size_t	GetTempDirCount()					{return tempdir.GetCount();}
	static	bool	CanFSHandleLargeFiles(size_t cat); // X: [TD] - [TempDir]
	static	LPCTSTR GetConfigFile();
	static	const CString& GetFileCommentsFilePath()	{return m_strFileCommentsFilePath;}
	static	CString	GetMuleDirectory(EDefaultDirectory eDirectory, bool bCreate = true);
	static	void	SetMuleDirectory(EDefaultDirectory eDirectory, CString strNewDir);
	static	void	ChangeUserDirMode(int nNewMode);

	// SLUGFILLER: SafeHash remove - removed installation dir unsharing
	//static	bool	IsTempFile(const CString& rstrDirectory, const CString& rstrName); 
	//static	bool	IsShareableDirectory(const CString& rstrDirectory);
	//static	bool	IsInstallationDirectory(const CString& rstrDir);
	// SLUGFILLER: SafeHash remove - removed installation dir unsharing

	static	bool	Save();
	static	void	SaveCats();

	static	bool	GetUseServerPriorities()			{return m_bUseServerPriorities;}
	static	bool	GetUseUserSortedServerList()		{return m_bUseUserSortedServerList;}
	static	bool	Reconnect()							{return reconnect;}
	static	const CString& GetUserNick()				{return strNick;}
	static	void	SetUserNick(LPCTSTR pszNick);
///	static	int		GetMaxUserNickLength()				{return 50;}

	static	LPCSTR	GetBindAddrA()						{return m_pszBindAddrA; }
	static	LPCWSTR	GetBindAddrW()						{return m_pszBindAddrW; }
	static	uint16	GetPort()							{return port;}
	static	uint16	GetUDPPort()						{return udpport;}

	static	uint16	GetServerUDPPort()					{return nServerUDPPort;}
	static	uchar*	GetUserHash()						{return userhash;}
	// ZZ:UploadSpeedSense -->
///	static	uint16	GetMinUpload()						{return minupload;}
	// ZZ:UploadSpeedSense <--
	static	bool	IsICHEnabled()						{return ICH;}
	static	bool	GetAutoUpdateServerList()			{return m_bAutoUpdateServerList;}
	static	bool	UpdateNotify()						{return updatenotify;}
	static	bool	GetMinToTray()						{return mintotray;}
	static	bool	DoAutoConnect()						{return autoconnect;}
	static	void	SetAutoConnect(bool inautoconnect)	{autoconnect = inautoconnect;}
	static	bool	IsAcceptUpload();// X: [RU] - [RefuseUpload]
	static	bool*	GetMinTrayPTR()						{return &mintotray;}
	static	UINT	GetTrafficOMeterInterval()			{return trafficOMeterInterval;}
	static	void	SetTrafficOMeterInterval(UINT in)	{trafficOMeterInterval=in;}
	static	UINT	GetStatsInterval()					{return statsInterval;}
	static	void	SetStatsInterval(UINT in)			{statsInterval=in;}
	static	bool	GetFillGraphs()						{return m_bFillGraphs;}
	static	void	SetFillGraphs(bool bFill)			{m_bFillGraphs = bFill;}

	// -khaos--+++> Many, many, many, many methods.
	static	void	SaveStats(int bBackUp = 0);
	static	void	SetRecordStructMembers();
	static	void	SaveCompletedDownloadsStat();
	static	bool	LoadStats(int loadBackUp = 0);
	static	void	ResetCumulativeStatistics();

	//--------------------------------------------------------------------------------------
	//Xman Xtreme Mod:

	//Xman Xtreme Upload
	static	float	m_slotspeed;
	static	bool		m_openmoreslots;
	static	bool		m_bandwidthnotreachedslots;
	//static	void CheckSlotSpeed();
	// X: [DSRB] - [Dynamic Send and Receive Buffer]
	//static	int		m_sendbuffersize;
	//static	int		GetSendbuffersize()	{return m_sendbuffersize;}
	//static	void		SetSendbuffersize(int in) {m_sendbuffersize=in;}
	static	int		m_internetdownreactiontime;

	//Xman process prio
	// [TPT] - Select process priority 
	static	DWORD GetMainProcessPriority() { return m_MainProcessPriority; }
	static	void SetMainProcessPriority(DWORD in) {m_MainProcessPriority=in; }
	static	uint32	m_MainProcessPriority;
	// [TPT] - Select process priority 	


	//Xman 1:3 Ratio
	static	bool m_13ratio;
	static	bool Is13Ratio() {return m_13ratio;}
	static	void Set13Ratio(bool in) {m_13ratio=in;}
	//Xman end
// ==> Superior Client Handling [Stulle] - Stulle
	/*
	//Xman always one release-slot
	static	bool m_onerealeseslot;
	static	bool UseReleasseSlot() {return m_onerealeseslot;}
	static	void SetUseReleaseSlot(bool in) {m_onerealeseslot=in;}
	//Xman end
	*/
	// <== Superior Client Handling [Stulle] - Stulle

	//Xman advanced upload-priority
	static	bool m_AdvancedAutoPrio;
	static	bool UseAdvancedAutoPtio() {return m_AdvancedAutoPrio;}
	static	void SetAdvancedAutoPrio(bool in) {m_AdvancedAutoPrio=in;}
	//Xman end

	//Xman chunk chooser
	static	uint8 m_chunkchooser;
	static	uint8 GetChunkChooseMethod()	{return m_chunkchooser;}

	//Xman disable compression
	static	bool m_bUseCompression;


	//Xman auto update IPFilter
	static	bool	m_bautoupdateipfilter;
	static	bool AutoUpdateIPFilter() {return m_bautoupdateipfilter;}
	static	void SetAutoUpdateIPFilter(bool in) {m_bautoupdateipfilter=in;}
	static	CString m_strautoupdateipfilter_url;
	static	CString GetAutoUpdateIPFilter_URL() {return m_strautoupdateipfilter_url;}
	static	void SetAutoUpdateIPFilter_URL(CString in) {m_strautoupdateipfilter_url=in;}
	static	SYSTEMTIME		m_IPfilterVersion;
	static	uint64 m_last_ipfilter_check;
	//Xman end

	//Xman count block/success send
	static	bool m_showblockratio;
	static	bool ShowBlockRatio() {return m_showblockratio;}
	static	void SetShowBlockRatio(bool in) {m_showblockratio=in;}

	static	bool m_dropblockingsockets;
	static	bool DropBlockingSockets() {return m_dropblockingsockets;}
	static	void SetDropBlockingSockets(bool in) {m_dropblockingsockets=in;}
	//Xman end

	//Xman Funny-Nick (Stulle/Morph)
	static	bool	m_bFunnyNick;
	static	bool DisplayFunnyNick() 	{return m_bFunnyNick;}
	static	void SetDisplayFunnyNick(bool in) {m_bFunnyNick=in;}
	//Xman end

	//Xman remove unused AICH-hashes
	static	bool m_rememberAICH;
	static	bool GetRememberAICH() {return m_rememberAICH;}
	static	void SetRememberAICH(bool in) {m_rememberAICH=in;}
	//Xman end

	//Xman smooth-accurate-graph
	static	bool usesmoothgraph;

	static	bool retryconnectionattempts; //Xman 

	// Maella 
	static	float	maxupload;                    // [FAF] -Allow Bandwidth Settings in <1KB Incremements-
	static	float	maxdownload;
	static	float	maxGraphDownloadRate;
	static	float	maxGraphUploadRate;

	static	uint16	MTU;                          // -MTU Configuration-
	static	bool		usedoublesendsize;
	static	bool		retrieveMTUFromSocket; // netfinity: Maximum Segment Size (MSS - Vista only) //added by zz_fly

	static	bool	NAFCFullControl;	          // -Network Adapter Feedback Control-
	static	uint32 forceNAFCadapter;	
	static	uint8	datarateSamples;              // -Accurate measure of bandwidth: eDonkey data + control, network adapter-

	static	bool    enableMultiQueue;             // -One-queue-per-file- (idea bloodymad)
	//static	bool    enableReleaseMultiQueue;
	// Maella end

	// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
	static	float	GetMaxUpload()  {return maxupload;}
	static	void	SetMaxUpload(float in);

	static	float	GetMaxDownload() ; // rate limited
	static	void	SetMaxDownload(float in) {maxdownload = in;}

	static	float	GetMaxGraphUploadRate()  {return maxGraphUploadRate;}
	static	void	SetMaxGraphUploadRate(float in) {maxGraphUploadRate=in;}

	static	float	GetMaxGraphDownloadRate()  {return maxGraphDownloadRate;}
	static	void	SetMaxGraphDownloadRate(float in) {maxGraphDownloadRate=in;}
	// Maella end

	// Maella -MTU Configuration-
	static	uint16	GetMTU()  { return MTU; }
	static	void	SetMTU(uint16 MTUin) { MTU = MTUin; }
	// Maella end

	// Maella -Network Adapter Feedback Control-
	static	bool	GetNAFCFullControl()  { return NAFCFullControl; }
	//static	void	SetNAFCFullControl(bool flag) { NAFCFullControl = flag; }
	static	void	SetNAFCFullControl(bool flag);
	static	uint32 GetForcedNAFCAdapter() { return forceNAFCadapter;}
	// Maella end

	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	static	uint8 GetDatarateSamples()  { return datarateSamples; }
	static	void  SetDatarateSamples(uint8 samples) { datarateSamples = samples; }
	// Maella end

	// Maella -One-queue-per-file- (idea bloodymad)
	static	bool GetEnableMultiQueue()  { return enableMultiQueue; }
	static	void SetEnableMultiQueue(bool state) { enableMultiQueue = state; }
	//static	bool GetEnableReleaseMultiQueue()  { return enableReleaseMultiQueue; }
	//static	void SetEnableReleaseMultiQueue(bool state) { enableReleaseMultiQueue = state; }
	// Maella end

	// Mighty Knife: Static server handling
	static	bool	m_bDontRemoveStaticServers;
	static	bool    GetDontRemoveStaticServers ()			  { return m_bDontRemoveStaticServers; }
	static	void	SetDontRemoveStaticServers (bool _b)	  { m_bDontRemoveStaticServers = _b; }
	// [end] Mighty Knife

	//Xman [MoNKi: -Downloaded History-]
	static	bool		m_bHistoryShowShared;
	static	bool		GetShowSharedInHistory()		{ return m_bHistoryShowShared; }
	static	void		SetShowSharedInHistory(bool on)	{ m_bHistoryShowShared = on; }
	//Xman end

	//Xman GlobalMaxHarlimit for fairness
	static	uint32	m_uMaxGlobalSources;
	static	bool		m_bAcceptsourcelimit;

	//Xman show additional graph lines
	static	bool		m_bShowAdditionalGraph;

	//Xman don't overwrite bak files if last sessions crashed
	//static	bool		m_this_session_aborted_in_an_unnormal_way;// X: [CI] - [Code Improvement]
	static	bool		m_last_session_aborted_in_an_unnormal_way;
	static	bool		eMuleCrashedLastSession()		{ return m_last_session_aborted_in_an_unnormal_way;}

	static bool		m_bKnown2Buffer; //zz_fly :: known2 buffer
	//Xman end
	//-----------------------------------------------------------------------------

	static	void	Add2DownCompletedFiles()			{cumDownCompletedFiles++;}
	static	void	SetConnMaxAvgDownRate(float in)		{cumConnMaxAvgDownRate = in;}
	static	void	SetConnMaxDownRate(float in)		{cumConnMaxDownRate = in;}
	static	void	SetConnAvgUpRate(float in)			{cumConnAvgUpRate = in;}
	static	void	SetConnMaxAvgUpRate(float in)		{cumConnMaxAvgUpRate = in;}
	static	void	SetConnMaxUpRate(float in)			{cumConnMaxUpRate = in;}
	static	void	SetConnPeakConnections(int in)		{cumConnPeakConnections = in;}
	static	void	SetUpAvgTime(int in)				{cumUpAvgTime = in;}
	static	void	Add2DownSAvgTime(int in)			{sesDownAvgTime += in;}
	static	void	SetDownCAvgTime(int in)				{cumDownAvgTime = in;}
	static	void	Add2ConnTransferTime(int in)		{cumConnTransferTime += in;}
	static	void	Add2ConnDownloadTime(int in)		{cumConnDownloadTime += in;}
	static	void	Add2ConnUploadTime(int in)			{cumConnUploadTime += in;}
	static	void	Add2DownSessionCompletedFiles()		{sesDownCompletedFiles++;}
	static	void	Add2SessionTransferData(UINT uClientID, UINT uClientPort, BOOL bFromPF, BOOL bUpDown, uint32 bytes, bool sentToFriend = false);
	static	void	Add2DownSuccessfulSessions()		{sesDownSuccessfulSessions++;
														 cumDownSuccessfulSessions++;}
	static	void	Add2DownFailedSessions()			{sesDownFailedSessions++;
														 cumDownFailedSessions++;}
	static	void	Add2LostFromCorruption(uint64 in)	{sesLostFromCorruption += in;}
	static	void	Add2SavedFromCompression(uint64 in) {sesSavedFromCompression += in;}
	static	void	Add2SessionPartsSavedByICH(int in)	{sesPartsSavedByICH += in;}

	// Saved stats for cumulative downline overhead
	static	uint64	GetDownOverheadTotal()				{return cumDownOverheadTotal;}
	static	uint64	GetDownOverheadFileReq()			{return cumDownOverheadFileReq;}
	static	uint64	GetDownOverheadSrcEx()				{return cumDownOverheadSrcEx;}
	static	uint64	GetDownOverheadServer()				{return cumDownOverheadServer;}
	static	uint64	GetDownOverheadKad()				{return cumDownOverheadKad;}
	static	uint64	GetDownOverheadTotalPackets()		{return cumDownOverheadTotalPackets;}
	static	uint64	GetDownOverheadFileReqPackets()		{return cumDownOverheadFileReqPackets;}
	static	uint64	GetDownOverheadSrcExPackets()		{return cumDownOverheadSrcExPackets;}
	static	uint64	GetDownOverheadServerPackets()		{return cumDownOverheadServerPackets;}
	static	uint64	GetDownOverheadKadPackets()			{return cumDownOverheadKadPackets;}

	// Saved stats for cumulative upline overhead
	static	uint64	GetUpOverheadTotal()				{return cumUpOverheadTotal;}
	static	uint64	GetUpOverheadFileReq()				{return cumUpOverheadFileReq;}
	static	uint64	GetUpOverheadSrcEx()				{return cumUpOverheadSrcEx;}
	static	uint64	GetUpOverheadServer()				{return cumUpOverheadServer;}
	static	uint64	GetUpOverheadKad()					{return cumUpOverheadKad;}
	static	uint64	GetUpOverheadTotalPackets()			{return cumUpOverheadTotalPackets;}
	static	uint64	GetUpOverheadFileReqPackets()		{return cumUpOverheadFileReqPackets;}
	static	uint64	GetUpOverheadSrcExPackets()			{return cumUpOverheadSrcExPackets;}
	static	uint64	GetUpOverheadServerPackets()		{return cumUpOverheadServerPackets;}
	static	uint64	GetUpOverheadKadPackets()			{return cumUpOverheadKadPackets;}

	// Saved stats for cumulative upline data
	static	uint32	GetUpSuccessfulSessions()			{return cumUpSuccessfulSessions;}
	static	uint32	GetUpFailedSessions()				{return cumUpFailedSessions;}
	static	uint32	GetUpAvgTime()						{return cumUpAvgTime;}

	// Saved stats for cumulative downline data
	static	uint32	GetDownCompletedFiles()				{return cumDownCompletedFiles;}
	static	uint32	GetDownC_SuccessfulSessions()		{return cumDownSuccessfulSessions;}
	static	uint32	GetDownC_FailedSessions()			{return cumDownFailedSessions;}
	static	uint32	GetDownC_AvgTime()					{return cumDownAvgTime;}
	
	// Session download stats
	static	uint32	GetDownSessionCompletedFiles()		{return sesDownCompletedFiles;}
	static	uint32	GetDownS_SuccessfulSessions()		{return sesDownSuccessfulSessions;}
	static	uint32	GetDownS_FailedSessions()			{return sesDownFailedSessions;}
	static	uint32	GetDownS_AvgTime()					{return GetDownS_SuccessfulSessions() ? sesDownAvgTime / GetDownS_SuccessfulSessions() : 0;}

	// Saved stats for corruption/compression
	static	uint64	GetCumLostFromCorruption()			{return cumLostFromCorruption;}
	static	uint64	GetCumSavedFromCompression()		{return cumSavedFromCompression;}
	static	uint64	GetSesLostFromCorruption()			{return sesLostFromCorruption;}
	static	uint64	GetSesSavedFromCompression()		{return sesSavedFromCompression;}
	static	uint32	GetCumPartsSavedByICH()				{return cumPartsSavedByICH;}
	static	uint32	GetSesPartsSavedByICH()				{return sesPartsSavedByICH;}

	// Cumulative client breakdown stats for sent bytes
	static	uint64	GetUpTotalClientData(){
		uint64 res=0;
		for(INT_PTR i=0;i<UP_SOFT_COUNT;i++)
			res += cumUpData[i] + sesUpData[i];
		return res;
	}
	static	uint64	GetCumUpData(ESoft i)				{return (cumUpData[i] +		sesUpData[i] );}
	
	// Session client breakdown stats for sent bytes
	static	uint64	GetUpSessionClientData(){
		uint64 res=0;
		for(INT_PTR i=0;i<UP_SOFT_COUNT;i++)
			res += sesUpData[i];
		return res;
	}

	// Cumulative port breakdown stats for sent bytes...
	static	uint64	GetUpTotalPortData(){
		uint64 res=0;
		for(INT_PTR i=0;i<PORT_COUNT;i++)
			res+=cumUpDataPort[i] +		sesUpDataPort[i];
		return res;
	}
	static	uint64	GetCumUpDataPort(EPort i)				{return (cumUpDataPort[i] +		sesUpDataPort[i] );}

	// Session port breakdown stats for sent bytes...
	static	uint64	GetUpSessionPortData(){		return (sesUpDataPort[0]+sesUpDataPort[1]+sesUpDataPort[2]);	}

	// Cumulative DS breakdown stats for sent bytes...
	static	uint64	GetUpTotalDataFile()				{return (GetCumUpData_File() +		GetCumUpData_Partfile() );}
	static	uint64	GetCumUpData_File()					{return (cumUpData_File +			sesUpData_File );}
	static	uint64	GetCumUpData_Partfile()				{return (cumUpData_Partfile +		sesUpData_Partfile );}
	// Session DS breakdown stats for sent bytes...
	static	uint64	GetUpSessionDataFile()				{return (sesUpData_File +			sesUpData_Partfile );}
	static	uint64	GetUpData_File()					{return sesUpData_File;}
	static	uint64	GetUpData_Partfile()				{return sesUpData_Partfile;}

	// Cumulative client breakdown stats for received bytes
	static	uint64	GetDownTotalClientData(){
		uint64 res=0;
		for(INT_PTR i=0;i<DOWN_SOFT_COUNT;i++)
			res+=cumDownData[i] +sesDownData[i];
		return res;
	}
	static	uint64	GetCumDownData(ESoft i)			{return (cumDownData[i] +			sesDownData[i]);}
	
	// Session client breakdown stats for received bytes
	static	uint64	GetDownSessionClientData(){
		uint64 res=0;
		for(INT_PTR i=0;i<DOWN_SOFT_COUNT;i++)
			res+=sesDownData[i];
		return res;
	}

	// Cumulative port breakdown stats for received bytes...
	static	uint64	GetDownTotalPortData(){
		uint64 res=0;
		for(INT_PTR i=0;i<PORT_COUNT;i++)
			res+=cumDownDataPort[i]+sesDownDataPort[i];
		return res;
	}
	static	uint64	GetCumDownDataPort(EPort i)			{return (cumDownDataPort[i]		+ sesDownDataPort[i]);}

	// Session port breakdown stats for received bytes...
	static	uint64	GetDownSessionDataPort(){		return (sesDownDataPort[0]+sesDownDataPort[1]+sesDownDataPort[2]);	}
	// Saved stats for cumulative connection data
	static	float	GetConnAvgDownRate()				{return cumConnAvgDownRate;}
	static	float	GetConnMaxAvgDownRate()				{return cumConnMaxAvgDownRate;}
	static	float	GetConnMaxDownRate()				{return cumConnMaxDownRate;}
	static	float	GetConnAvgUpRate()					{return cumConnAvgUpRate;}
	static	float	GetConnMaxAvgUpRate()				{return cumConnMaxAvgUpRate;}
	static	float	GetConnMaxUpRate()					{return cumConnMaxUpRate;}
	static	time_t	GetConnRunTime()					{return cumConnRunTime;}
	static	uint32	GetConnNumReconnects()				{return cumConnNumReconnects;}
	static	uint32	GetConnAvgConnections()				{return cumConnAvgConnections;}
	static	uint32	GetConnMaxConnLimitReached()		{return cumConnMaxConnLimitReached;}
	static	uint32	GetConnPeakConnections()			{return cumConnPeakConnections;}
	static	uint32	GetConnTransferTime()				{return cumConnTransferTime;}
	static	uint32	GetConnDownloadTime()				{return cumConnDownloadTime;}
	static	uint32	GetConnUploadTime()					{return cumConnUploadTime;}
	static	uint32	GetConnServerDuration()				{return cumConnServerDuration;}

	// Saved records for servers / network
	static	uint32	GetSrvrsMostWorkingServers()		{return cumSrvrsMostWorkingServers;}
	static	uint32	GetSrvrsMostUsersOnline()			{return cumSrvrsMostUsersOnline;}
	static	uint32	GetSrvrsMostFilesAvail()			{return cumSrvrsMostFilesAvail;}

	// Saved records for shared files
	static	uint32	GetSharedMostFilesShared()			{return cumSharedMostFilesShared;}
	static	uint64	GetSharedLargestShareSize()			{return cumSharedLargestShareSize;}
	static	uint64	GetSharedLargestAvgFileSize()		{return cumSharedLargestAvgFileSize;}
	static	uint64	GetSharedLargestFileSize()			{return cumSharedLargestFileSize;}

	// Get the long date/time when the stats were last reset
	static	time_t GetStatsLastResetLng()				{return stat_datetimeLastReset;}
	static	CString GetStatsLastResetStr(bool formatLong = true);
	static	UINT	GetStatsSaveInterval()				{return statsSaveInterval;}

	// Get and Set our new preferences
	static	void	SetStatsMax(UINT in)				{statsMax = in;}
	static	void	SetStatsConnectionsGraphRatio(UINT in) {statsConnectionsGraphRatio = in;}
	static	UINT	GetStatsConnectionsGraphRatio()		{return statsConnectionsGraphRatio;}
	static	void	SetExpandedTreeItems(CString in)	{m_strStatsExpandedTreeItems = in;}
	static	const CString &GetExpandedTreeItems()		{return m_strStatsExpandedTreeItems;}

	static	uint64	GetTotalDownloaded()				{return totalDownloadedBytes;}
	static	uint64	GetTotalUploaded()					{return totalUploadedBytes;}

	static	bool	IsErrorBeepEnabled()				{return beepOnError;}
	static	bool	IsConfirmExitEnabled()				{return confirmExit;}
	static	void	SetConfirmExit(bool bVal)			{confirmExit = bVal;} 
	static	bool	UseSplashScreen()					{return splashscreen;}
	static	bool	FilterLANIPs()						{return filterLANIPs;}
#ifdef _DEBUG
	static	bool	GetAllowLocalHostIP()				{return m_bAllocLocalHostIP;}
#endif
	static	bool	IsOnlineSignatureEnabled()			{return onlineSig;}

	static	uint64	GetMaxDownloadInBytesPerSec(); //Xman Xtreme Mod

	// NEO: QS - [QuickStart] -- Xanatos -->
	//static	UINT	GetMaxConnections()					{return maxconnections;}
	//static	UINT	GetMaxHalfConnections()				{return maxhalfconnections;}
	static	UINT	GetMaxConnections();
	static	UINT	GetMaxHalfConnections();
	// NEO: QS END <-- Xanatos --
	static	UINT	GetMaxSourcePerFileDefault()		{return maxsourceperfile;}
	static	UINT	GetDeadServerRetries()				{return m_uDeadServerRetries;}
	static	DWORD	GetServerKeepAliveTimeout()			{return m_dwServerKeepAliveTimeout;}
	static	bool	GetConditionalTCPAccept()			{return m_bConditionalTCPAccept;}

	static	WORD	GetLanguageID();
	static	void	SetLanguageID(WORD lid);
	static	void	GetLanguages(CAtlArray<WORD>& aLanguageIDs);
	static	void	SetLanguage();
	static	bool	IsLanguageSupported(LANGID lidSelected, bool bUpdateBefore);
	static	CString GetLangNameByID(LANGID lidSelected);
#ifdef _DEBUG
	static	void	InitThreadLocale();
	static	void	SetRtlLocale(LCID lcid);
#endif
	//static	CString GetHtmlCharset();

	static	bool	IsDoubleClickEnabled()				{return transferDoubleclick;}
	static	EViewSharedFilesAccess CanSeeShares(void)	{return m_iSeeShares;}
	static	UINT	GetToolTipDelay(void)				{return m_iToolDelayTime;}
	static	bool	IsBringToFront()					{return bringtoforeground;}

	static	UINT	GetSplitterbarPosition()			{return splitterbarPosition;}
	static	void	SetSplitterbarPosition(UINT pos)	{splitterbarPosition=pos;}
	static	UINT	GetSplitterbarPositionServer()		{return splitterbarPositionSvr;}
	static	void	SetSplitterbarPositionServer(UINT pos)	{splitterbarPositionSvr=pos;}
	static	UINT	GetTransferWnd1()					{return m_uTransferWnd1;}
	static	void	SetTransferWnd1(UINT uWnd1)			{m_uTransferWnd1 = uWnd1;}
	static	UINT	GetTransferWnd2()					{return m_uTransferWnd2;}
	static	void	SetTransferWnd2(UINT uWnd2)			{m_uTransferWnd2 = uWnd2;}
	//MORPH START - Added by SiRoB, Splitting Bar [O?]
	static	UINT	GetSplitterbarPositionStat()		{return splitterbarPositionStat;}
	static	void	SetSplitterbarPositionStat(UINT pos) {splitterbarPositionStat=pos;}
	static	UINT	GetSplitterbarPositionStat_HL()		{return splitterbarPositionStat_HL;}
	static	void	SetSplitterbarPositionStat_HL(UINT pos) {splitterbarPositionStat_HL=pos;}
	static	UINT	GetSplitterbarPositionStat_HR()		{return splitterbarPositionStat_HR;}
	static	void	SetSplitterbarPositionStat_HR(UINT pos) {splitterbarPositionStat_HR=pos;}
	static	UINT	GetSplitterbarPositionFriend()		{return splitterbarPositionFriend;}
	static	void	SetSplitterbarPositionFriend(UINT pos) {splitterbarPositionFriend=pos;}
	static	UINT	GetSplitterbarPositionShared()		{return splitterbarPositionShared;}
	static	void	SetSplitterbarPositionShared(UINT pos) {splitterbarPositionShared=pos;}
	//MORPH END   - Added by SiRoB, Splitting Bar [O?]
	// -khaos--+++> Changed datatype to avoid overflows
	static	UINT	GetStatsMax()						{return statsMax;}
	// <-----khaos-
	static	bool	UseFlatBar()						{return (depth3D==0);}
	static	int		GetStraightWindowStyles()			{return m_iStraightWindowStyles;}
	static	bool	GetUseSystemFontForMainControls()	{return m_bUseSystemFontForMainControls;}

	static	const CString& GetSkinProfile()				{return m_strSkinProfile;}
	static	void	SetSkinProfile(LPCTSTR pszProfile)	{m_strSkinProfile = pszProfile;}

	static	UINT	GetStatsAverageMinutes()			{return statsAverageMinutes;}
	static	void	SetStatsAverageMinutes(UINT in)	{statsAverageMinutes=in;}

	static	const CString& GetNotifierConfiguration()	{return notifierConfiguration;}
	static	void	SetNotifierConfiguration(LPCTSTR pszConfigPath) {notifierConfiguration = pszConfigPath;}
	static	bool	GetNotifierOnDownloadFinished()		{return notifierOnDownloadFinished;}
	static	bool	GetNotifierOnNewDownload()			{return notifierOnNewDownload;}
	static	bool	GetNotifierOnChat()					{return notifierOnChat;}
	static	bool	GetNotifierOnLog()					{return notifierOnLog;}
	static	bool	GetNotifierOnImportantError()		{return notifierOnImportantError;}
	static	bool	GetNotifierOnEveryChatMsg()			{return notifierOnEveryChatMsg;}
	static	bool	GetNotifierOnNewVersion()			{return notifierOnNewVersion;}
	static	ENotifierSoundType GetNotifierSoundType()	{return notifierSoundType;}
	static	const CString& GetNotifierSoundFile()		{return notifierSoundFile;}

	static	bool	GetEnableMiniMule()					{return m_bEnableMiniMule;}
	static	bool	GetRTLWindowsLayout()				{return m_bRTLWindowsLayout;}

	static	bool	GetMessageEnableSmileys()			{return m_bMessageEnableSmileys;}

	static	WORD	InitWinVersion();
	static	WORD	GetWindowsVersion(){return m_wWinVer;}
	static	bool	IsRunningAeroGlassTheme();
	static	bool	GetStartMinimized()					{return startMinimized;}
	static	void	SetStartMinimized( bool instartMinimized) {startMinimized = instartMinimized;}
	static	bool	GetAutoStart()						{return m_bAutoStart;}
	static	void	SetAutoStart( bool val)				{m_bAutoStart = val;}

	static	bool	GetRestoreLastMainWndDlg()			{return m_bRestoreLastMainWndDlg;}
	static	void	SetRestoreLastMainWndDlg( bool bo)				{m_bRestoreLastMainWndDlg = bo;}

	static	int		GetLastMainWndDlgID()				{return m_iLastMainWndDlgID;}
	static	void	SetLastMainWndDlgID(int iID)		{m_iLastMainWndDlgID = iID;}

	static	bool	GetRestoreLastLogPane()				{return m_bRestoreLastLogPane;}
	static	int		GetLastLogPaneID()					{return m_iLastLogPaneID;}
	static	void	SetLastLogPaneID(int iID)			{m_iLastLogPaneID = iID;}

	static	bool	GetSmartIdCheck()					{return m_bSmartServerIdCheck;}
	static	void	SetSmartIdCheck(bool in_smartidcheck) {m_bSmartServerIdCheck = in_smartidcheck;}
	static	uint8	GetSmartIdState()					{return smartidstate;}
	static	void	SetSmartIdState(uint8 in_smartidstate) {smartidstate = in_smartidstate;}
	static	bool	GetPreviewPrio()					{return m_bpreviewprio;}
	static	void	SetPreviewPrio(bool in)				{m_bpreviewprio=in;}
	static	bool	GetUpdateQueueList()				{return m_bupdatequeuelist;}
	static	bool	GetManualAddedServersHighPriority()	{return m_bManualAddedServersHighPriority;}
	static	bool	TransferFullChunks()				{return m_btransferfullchunks;}
	static	void	SetTransferFullChunks( bool m_bintransferfullchunks )				{m_btransferfullchunks = m_bintransferfullchunks;}
	static	int		StartNextFile()						{return m_istartnextfile;}
	static	bool	ShowOverhead()						{return m_bshowoverhead;}
	static	void	SetNewAutoUp(bool m_bInUAP)			{m_bUAP = m_bInUAP;}
	static	bool	GetNewAutoUp()						{return m_bUAP;}
	static	void	SetNewAutoDown(bool m_bInDAP)		{m_bDAP = m_bInDAP;}
	static	bool	GetNewAutoDown()					{return m_bDAP;}
	static	bool	IsKnownClientListDisabled()			{return m_bDisableKnownClientList;}
	static	bool	IsQueueListDisabled()				{return m_bDisableQueueList;}
	static	bool	IsFirstStart()						{return m_bFirstStart;}
#ifdef CLIENTANALYZER
	static	uint8	UseCreditSystem()					{return m_uiCreditSystem;}
	static	void	SetCreditSystem(const uint8& m_uiInCreditSystem) {m_uiCreditSystem = m_uiInCreditSystem;}
#else
	static	bool	UseCreditSystem()					{return m_bCreditSystem;}
	static	void	SetCreditSystem(bool m_bInCreditSystem) {m_bCreditSystem = m_bInCreditSystem;}
#endif

	static	const CString& GetTxtEditor()				{return m_strTxtEditor;}
	static	const CString& GetVideoPlayer()				{return m_strVideoPlayer;}
	static	const CString& GetVideoPlayerArgs()			{return m_strVideoPlayerArgs;}

	//static	UINT	GetFileBufferSize()					{return m_uFileBufferSize;}// X: [GB] - [Global Buffer]
	//static	UINT	GetFileBufferTimeLimit()			{return m_uFileBufferTimeLimit;}// X: [GB] - [Global Buffer]
	static	UINT	GetQueueSize()						{return m_iQueueSize;}
	static	int		GetCommitFiles()					{return m_iCommitFiles;}
	static	bool	GetShowCopyEd2kLinkCmd()			{return m_bShowCopyEd2kLinkCmd;}

	// Barry
	static	UINT	Get3DDepth()						{return depth3D;}
	static	bool	AutoTakeED2KLinks()					{return autotakeed2klinks;}
	static	bool	AddNewFilesPaused()					{return addnewfilespaused;}

	static	bool	TransferlistRemainSortStyle()		{return m_bTransflstRemain;}
	static	void	TransferlistRemainSortStyle(bool in){m_bTransflstRemain=in;}

	static	DWORD	GetStatsColor(size_t index)			{return m_adwStatsColors[index];}
	static	void	SetStatsColor(size_t index, DWORD value){m_adwStatsColors[index] = value;}
	static	size_t	GetNumStatsColors()					{return ARRSIZE(m_adwStatsColors);}
	static	void	GetAllStatsColors(size_t iCount, LPDWORD pdwColors);
	static	bool	SetAllStatsColors(size_t iCount, const DWORD* pdwColors);
	static	void	ResetStatsColor(size_t index);
	static	bool	HasCustomTaskIconColor()			{return bHasCustomTaskIconColor;}

	static	void	SetMaxConsPerFive(UINT in)			{MaxConperFive=in;}
	static	LPLOGFONT GetHyperTextLogFont()				{return &m_lfHyperText;}
	static	void	SetHyperTextFont(LPLOGFONT plf)		{m_lfHyperText = *plf;}
	static	LPLOGFONT GetLogFont()						{return &m_lfLogText;}
	static	void	SetLogFont(LPLOGFONT plf)			{m_lfLogText = *plf;}
	static	COLORREF GetLogErrorColor()					{return m_crLogError;}
	static	COLORREF GetLogWarningColor()				{return m_crLogWarning;}
	static	COLORREF GetLogSuccessColor()				{return m_crLogSuccess;}

	//static	UINT	GetMaxConperFive()					{return MaxConperFive;}
	static	UINT	GetMaxConperFive(); // NEO: QS - [QuickStart] <-- Xanatos --
	static	UINT	GetDefaultMaxConperFive();

	static	bool	IsSafeServerConnectEnabled()		{return m_bSafeServerConnect;}
	static	void	SetSafeServerConnectEnabled(bool in){m_bSafeServerConnect=in;}
	static	bool	IsMoviePreviewBackup()				{return moviePreviewBackup;}
	static	int		GetPreviewSmallBlocks()				{return m_iPreviewSmallBlocks;}
	static	bool	GetPreviewCopiedArchives()			{return m_bPreviewCopiedArchives;}
	static	int		GetInspectAllFileTypes()			{return m_iInspectAllFileTypes;}
	static	int		GetExtractMetaData()				{return m_iExtractMetaData;}
	static	bool	GetAdjustNTFSDaylightFileTime()		{return m_bAdjustNTFSDaylightFileTime;}
	static	bool	GetRearrangeKadSearchKeywords()		{return m_bRearrangeKadSearchKeywords;}

	static	const CString& GetYourHostname()			{return m_strYourHostname;}
	static	void	SetYourHostname(LPCTSTR pszHostname){m_strYourHostname = pszHostname;}
	static	bool	IsCheckDiskspaceEnabled()			{return checkDiskspace;}
	static	UINT	GetMinFreeDiskSpace()				{return m_uMinFreeDiskSpace;}
	static	bool	GetSparsePartFiles();
	static	void	SetSparsePartFiles(bool bEnable)	{m_bSparsePartFiles = bEnable;}
	static	bool	GetResolveSharedShellLinks()		{return m_bResolveSharedShellLinks;}
	static  bool	IsShowUpDownIconInTaskbar()			{return m_bShowUpDownIconInTaskbar;}
	static  bool	IsWin7TaskbarGoodiesEnabled()				{return m_bShowWin7TaskbarGoodies;}
	static  void    SetWin7TaskbarGoodiesEnabled(bool flag)	{m_bShowWin7TaskbarGoodies = flag;}


	static	WINDOWPLACEMENT GetEmuleWindowPlacement()	{return EmuleWindowPlacement;}
	static	void	SetWindowLayout(WINDOWPLACEMENT in) {EmuleWindowPlacement=in;}

	static	bool	GetAutoConnectToStaticServersOnly() {return m_bAutoConnectToStaticServersOnly;}	
	static	UINT	GetUpdateDays()						{return versioncheckdays;}
	static	uint64	GetLastVC()							{return versioncheckLastAutomatic;}
	static	void	UpdateLastVC();
	
	static	int		GetIPFilterLevel()					{return filterlevel;}
	static	const CString& GetMessageFilter()			{return messageFilter;}
	static	const CString& GetCommentFilter()			{return commentFilter;}
	static	void	SetCommentFilter(const CString& strFilter) {commentFilter = strFilter;}
	static	const CString& GetFilenameCleanups()		{return filenameCleanups;}

	static	bool	ShowRatesOnTitle()					{return showRatesInTitle;}
	static	void	LoadCats();
	static	const CString& GetDateTimeFormat()			{return m_strDateTimeFormat;}
	static	const CString& GetDateTimeFormat4Log()		{return m_strDateTimeFormat4Log;}
	static	const CString& GetDateTimeFormat4Lists()	{return m_strDateTimeFormat4Lists;}

	// Download Categories (Ornis)
	static	size_t	AddCat(Category_Struct* cat)		{catMap.Add(cat); return catMap.GetCount()-1;}
	static	bool	MoveCat(size_t from, size_t to);
	static	void	RemoveCat(size_t index);
	static	size_t	GetCatCount()						{return catMap.GetCount();}
	static	bool	SetCatFilter(size_t index, sint_ptr filter);
	static	size_t	GetCatFilter(size_t index);
	static	bool	GetCatFilterNeg(size_t index);
	static	void	SetCatFilterNeg(size_t index, bool val);
	static	Category_Struct* GetCategory(size_t index)		{if (index<catMap.GetCount()) return catMap.GetAt(index); else return NULL;}
	static	const CString &GetCatPath(size_t index)		{return catMap.GetAt(index)->strIncomingPath;}
	static	DWORD	GetCatColor(size_t index, int nDefault = COLOR_BTNTEXT);

	static	bool	GetPreviewOnIconDblClk()			{return m_bPreviewOnIconDblClk;}
	static	bool	GetCheckFileOpen()					{return m_bCheckFileOpen;}
	static	bool	ShowRatingIndicator()				{return indicateratings;}
	static	bool	WatchClipboard4ED2KLinks()			{return watchclipboard;}
	static	bool	GetRemoveToBin()					{return m_bRemove2bin;}
	static	bool	GetFilterServerByIP()				{return filterserverbyip;}

	static	bool	GetLog2Disk()						{return log2disk;}
	static	bool	GetDebug2Disk()						{return m_bVerbose && debug2disk;}
	static	int		GetMaxLogBuff()						{return iMaxLogBuff;}
	static	UINT	GetMaxLogFileSize()					{return uMaxLogFileSize;}
	static	ELogFileFormat GetLogFileFormat()			{return m_iLogFileFormat;}

	// WebServer
	static	uint16	GetWSPort()							{return m_nWebPort;}
	static	bool	GetWSUseUPnP()						{return m_bWebUseUPnP && GetWSIsEnabled();}
	static	void	SetWSPort(uint16 uPort)				{m_nWebPort=uPort;}
	static	const CString& GetWSPass()					{return m_strWebPassword;}
	static	void	SetWSPass(CString strNewPass);
	static	bool	GetWSIsEnabled()					{return m_bWebEnabled;}
	static	void	SetWSIsEnabled(bool bEnable)		{m_bWebEnabled=bEnable;}
	static	bool	GetWebUseGzip()						{return m_bWebUseGzip;}
	static	void	SetWebUseGzip(bool bUse)			{m_bWebUseGzip=bUse;}
	static	int		GetWebPageRefresh()					{return m_nWebPageRefresh;}
	static	void	SetWebPageRefresh(int nRefresh)		{m_nWebPageRefresh=nRefresh;}
	static	bool	GetWSIsLowUserEnabled()				{return m_bWebLowEnabled;}
	static	void	SetWSIsLowUserEnabled(bool in)		{m_bWebLowEnabled=in;}
	static	const CString& GetWSLowPass()				{return m_strWebLowPassword;}
	static	int		GetWebTimeoutMins()					{return m_iWebTimeoutMins;}
	static  bool	GetWebAdminAllowedHiLevFunc()		{return m_bAllowAdminHiLevFunc;}
	static	void	SetWSLowPass(CString strNewPass);
	static  const CAtlArray<UINT>& GetAllowedRemoteAccessIPs(){return m_aAllowedRemoteAccessIPs;}
	static	uint32	GetMaxWebUploadFileSizeMB()			{return m_iWebFileUploadSizeLimitMB;}

	static	void	SetMaxSourcesPerFile(UINT in)		{maxsourceperfile=in;}
	static	void	SetMaxConnections(UINT in)			{maxconnections =in;}
	static	void	SetMaxHalfConnections(UINT in)		{maxhalfconnections =in;}
	static	bool	IsSchedulerEnabled()				{return scheduler;}
	static	void	SetSchedulerEnabled(bool in)		{scheduler=in;}

	static	bool	MsgOnlyFriends()					{return msgonlyfriends;}
	static	bool	MsgOnlySecure()						{return msgsecure;}
	static	UINT	GetMsgSessionsMax()					{return maxmsgsessions;}
#ifndef CLIENTANALYZER
	static	bool	IsSecureIdentEnabled()				{return m_bUseSecureIdent;} // use clientcredits->CryptoAvailable() to check if crypting is really available and not this function
#endif
	static	bool	IsAdvSpamfilterEnabled()			{return m_bAdvancedSpamfilter;}
	static	bool	IsChatCaptchaEnabled()				{return IsAdvSpamfilterEnabled() && m_bUseChatCaptchas;}
	static	const CString& GetTemplate()				{return m_strTemplateFile;}
	static	void	SetTemplate(CString in)				{m_strTemplateFile = in;}
	static	bool	GetNetworkKademlia()				{return networkkademlia && udpport > 0;}
	static	void	SetNetworkKademlia(bool val){networkkademlia = val;}
	static	bool	GetNetworkED2K()					{return networked2k;}
	static	void	SetNetworkED2K(bool val)			{networked2k = val;}

	// deadlake PROXYSUPPORT
	static	const ProxySettings& GetProxySettings()		{return proxy;}
	static	void	SetProxySettings(const ProxySettings& proxysettings) {proxy = proxysettings;}

	static	bool	ShowCatTabInfos()					{return showCatTabInfos;}
	static	void	ShowCatTabInfos(bool in)			{showCatTabInfos=in;}

	static	bool	AutoFilenameCleanup()				{return autofilenamecleanup;}
	static	void	AutoFilenameCleanup(bool in)		{autofilenamecleanup=in;}
	static	void	SetFilenameCleanups(CString in)		{filenameCleanups=in;}

///	static	bool	GetResumeSameCat()					{return resumeSameCat;}
	static	bool	IsGraphRecreateDisabled()			{return dontRecreateGraphs;}
	static	bool	IsExtControlsEnabled()				{return m_bExtControls;}
	static	void	SetExtControls(bool in)				{m_bExtControls=in;}
	static	bool	GetRemoveFinishedDownloads()		{return m_bRemoveFinishedDownloads;}

	static	UINT	GetMaxChatHistoryLines()			{return m_iMaxChatHistory;}
	static	bool	GetUseAutocompletion()				{return m_bUseAutocompl;}
	static	bool	GetUseDwlPercentage()				{return m_bShowDwlPercentage;}
	static	void	SetUseDwlPercentage(bool in)		{m_bShowDwlPercentage=in;}
	static	bool	GetShowActiveDownloadsBold()		{return m_bShowActiveDownloadsBold;}
	static	bool	GetShowSharedFilesDetails()			{return m_bShowSharedFilesDetails;}
	static	void	SetShowSharedFilesDetails(bool bIn) {m_bShowSharedFilesDetails = bIn;}
	static	bool	GetAutoShowLookups()				{return m_bAutoShowLookups;}
	static	void	SetAutoShowLookups(bool bIn)		{m_bAutoShowLookups = bIn;}
	static	bool	GetForceSpeedsToKB()				{return m_bForceSpeedsToKB;}
	static	bool	GetExtraPreviewWithMenu()			{return m_bExtraPreviewWithMenu;}

	//Toolbar
	static	const CString& GetToolbarSettings()					{return m_sToolbarSettings;}
	static	void	SetToolbarSettings(const CString& in)		{m_sToolbarSettings = in;}
	static	const CString& GetToolbarBitmapSettings()			{return m_sToolbarBitmap;}
	static	void	SetToolbarBitmapSettings(const CString& path){m_sToolbarBitmap = path;}
	static	EToolbarLabelType GetToolbarLabelSettings()			{return m_nToolbarLabels;}
	static	void	SetToolbarLabelSettings(EToolbarLabelType eLabelType) {m_nToolbarLabels = eLabelType;}
	static	bool	GetReBarToolbar()							{return m_bReBarToolbar;}
	static	bool	GetUseReBarToolbar();
	static	CSize	GetToolbarIconSize()				{return m_sizToolbarIconSize;}
	static	void	SetToolbarIconSize(CSize siz)		{m_sizToolbarIconSize = siz;}

	static	bool	IsTransToolbarEnabled()				{return m_bWinaTransToolbar;}
	static	bool	IsDownloadToolbarEnabled()			{return m_bShowDownloadToolbar;}
	static	void	SetDownloadToolbar(bool bShow)		{m_bShowDownloadToolbar = bShow;}

	static	int		GetSearchMethod()					{return m_iSearchMethod;}
	static	void	SetSearchMethod(int iMethod)		{m_iSearchMethod = iMethod;}

	// ZZ:UploadSpeedSense -->
//	static	bool	IsDynUpEnabled()					{ return m_bDynUpEnabled; } //Xman
//	static	void	SetDynUpEnabled(bool newValue)		{m_bDynUpEnabled = newValue;}
//	static	int		GetDynUpPingTolerance()				{return m_iDynUpPingTolerance;}
//	static	int		GetDynUpGoingUpDivider()			{return m_iDynUpGoingUpDivider;}
//	static	int		GetDynUpGoingDownDivider()			{return m_iDynUpGoingDownDivider;}
//	static	int		GetDynUpNumberOfPings()				{return m_iDynUpNumberOfPings;}
//	static	bool	IsDynUpUseMillisecondPingTolerance(){return m_bDynUpUseMillisecondPingTolerance;} // EastShare - Added by TAHO, USS limit
//	static	int		GetDynUpPingToleranceMilliseconds() {return m_iDynUpPingToleranceMilliseconds;} // EastShare - Added by TAHO, USS limit
//	static	void	SetDynUpPingToleranceMilliseconds(int in){m_iDynUpPingToleranceMilliseconds = in;}
	// ZZ:UploadSpeedSense <--

    //static	bool     GetA4AFSaveCpu()                    {return m_bA4AFSaveCpu;} // ZZ:DownloadManager

    static bool     GetHighresTimer()                   {return m_bHighresTimer;}

	static	CString	GetHomepageBaseURL()				{return GetHomepageBaseURLForLevel(GetWebMirrorAlertLevel());}
	static	CString	GetVersionCheckBaseURL();					
	static	void	SetWebMirrorAlertLevel(uint8 newValue){m_nWebMirrorAlertLevel = newValue;}
	static	bool	IsDefaultNick(const CString strCheck);
	static	UINT	GetWebMirrorAlertLevel();
	static	bool	UseSimpleTimeRemainingComputation()	{return m_bUseOldTimeRemaining;}

	static	bool	IsRunAsUserEnabled();
	static	bool	IsPreferingRestrictedOverUser()		{return m_bPreferRestrictedOverUser;}

	// X: [RPC] - [Remove PeerCache]
	// PeerCache
/*	static	bool	IsPeerCacheDownloadEnabled()		{return (m_bPeerCacheEnabled && !IsClientCryptLayerRequested());}
	static	uint64	GetPeerCacheLastSearch()			{return m_uPeerCacheLastSearch;}
	static	bool	WasPeerCacheFound()					{return m_bPeerCacheWasFound;}
	static	void	SetPeerCacheLastSearch(uint64 dwLastSearch) {m_uPeerCacheLastSearch = dwLastSearch;}
	static	void	SetPeerCacheWasFound(bool bFound)	{m_bPeerCacheWasFound = bFound;}
	static	uint16	GetPeerCachePort()					{return m_nPeerCachePort;}
	static	void	SetPeerCachePort(uint16 nPort)		{m_nPeerCachePort = nPort;}
///	static	bool	GetPeerCacheShow()					{return m_bPeerCacheShow;}
*/
	// Verbose log options
	static	bool	GetEnableVerboseOptions()			{return m_bEnableVerboseOptions;}
	static	bool	GetVerbose()						{return m_bVerbose;}
	static	bool	GetFullVerbose()					{return m_bVerbose && m_bFullVerbose;}
	static	bool	GetDebugSourceExchange()			{return m_bVerbose && m_bDebugSourceExchange;}
	static	bool	GetLogBannedClients()				{return m_bVerbose && m_bLogBannedClients;}
	static	bool	GetLogRatingDescReceived()			{return m_bVerbose && m_bLogRatingDescReceived;}
	static	bool	GetLogSecureIdent()					{return m_bVerbose && m_bLogSecureIdent;}
	static	bool	GetLogFilteredIPs()					{return m_bVerbose && m_bLogFilteredIPs;}
	static	bool	GetLogFileSaving()					{return m_bVerbose && m_bLogFileSaving;}
    static	bool	GetLogA4AF()    					{return m_bVerbose && m_bLogA4AF;} // ZZ:DownloadManager
	static	bool	GetLogDrop()						{return m_bVerbose && m_bLogDrop;} //Xman Xtreme Downloadmanager
	static	bool	GetLogPartmismatch()				{return m_bVerbose && m_bLogpartmismatch;} //Xman Log part/size-mismatch
	static	bool	GetLogUlDlEvents()					{return m_bVerbose && m_bLogUlDlEvents;}
	static	bool	GetLogKadSecurityEvents()			{return m_bVerbose && true;}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	static	bool	GetUseDebugDevice()					{return m_bUseDebugDevice;}
	static	int		GetDebugServerTCPLevel()			{return m_iDebugServerTCPLevel;}
	static	int		GetDebugServerUDPLevel() 			{return m_iDebugServerUDPLevel;}
	static	int		GetDebugServerSourcesLevel()		{return m_iDebugServerSourcesLevel;}
	static	int		GetDebugServerSearchesLevel()		{return m_iDebugServerSearchesLevel;}
	static	int		GetDebugClientTCPLevel()			{return m_iDebugClientTCPLevel;}
	static	int		GetDebugClientUDPLevel()			{return m_iDebugClientUDPLevel;}
	static	int		GetDebugClientKadUDPLevel()			{return m_iDebugClientKadUDPLevel;}
	static	int		GetDebugSearchResultDetailLevel()	{return m_iDebugSearchResultDetailLevel;}
#endif
	static	int		GetVerboseLogPriority()				{return	m_byLogLevel;}

	// Firewall settings
	static	bool	IsOpenPortsOnStartupEnabled()		{return m_bOpenPortsOnStartUp;}
	
	//AICH Hash
	static	bool	IsTrustingEveryHash()				{return m_bTrustEveryHash;} // this is a debug option

	static	bool	IsRememberingDownloadedFiles()		{return m_bRememberDownloadedFiles;}
	static	bool	IsRememberingCancelledFiles()		{return m_bRememberCancelledFiles;}
	static	bool	DoPartiallyPurgeOldKnownFiles()		{return m_bPartiallyPurgeOldKnownFiles;}
	static	void	SetRememberDownloadedFiles(bool nv)	{m_bRememberDownloadedFiles = nv;}
	static	void	SetRememberCancelledFiles(bool nv)	{m_bRememberCancelledFiles = nv;}
	
	static	bool	DoFlashOnNewMessage()				{return m_bIconflashOnNewMessage;}
	//static	void	IniCopy(CString si, CString di);

	static	void	EstimateMaxUploadCap(uint32 nCurrentUpload);
	static	bool	GetAllocCompleteMode()				{return m_bAllocFull;}
	static	void	SetAllocCompleteMode(bool in)		{m_bAllocFull=in;}

	// encryption
	static	bool		IsClientCryptLayerSupported()		{return m_bCryptLayerSupported;}
	static	bool		IsClientCryptLayerRequested()		{return IsClientCryptLayerSupported() && m_bCryptLayerRequested;}
	static bool		IsClientCryptLayerRequired()		{return IsClientCryptLayerRequested() && m_bCryptLayerRequired;}
	static bool		IsClientCryptLayerRequiredStrict()	{return false;} // not even incoming test connections will be answered
	static	bool		IsServerCryptLayerUDPEnabled()		{return IsClientCryptLayerSupported();}
	static	bool		IsServerCryptLayerTCPRequested()	{return IsClientCryptLayerRequested();}
	static	uint32	GetKadUDPKey()						{return m_dwKadUDPKey;}
	static	uint8	GetCryptTCPPaddingLength()			{return m_byCryptTCPPaddingLength;}
///	static	void		SetCryptTCPPaddingLength(int in)	{m_byCryptTCPPaddingLength = (uint8)((in>=10 && in<=254) ? in : 128);} //zz_fly :: hardlimit on CryptTCPPaddingLength

	// UPnP
	static bool		GetSkipWANIPSetup()					{return m_bSkipWANIPSetup;}
	static bool		GetSkipWANPPPSetup()				{return m_bSkipWANPPPSetup;}
	static bool		IsUPnPEnabled()						{return m_bEnableUPnP;}
	static void		SetSkipWANIPSetup(bool nv)			{m_bSkipWANIPSetup = nv;}
	static void		SetSkipWANPPPSetup(bool nv)			{m_bSkipWANPPPSetup = nv;}
	static bool		CloseUPnPOnExit()					{return m_bCloseUPnPOnExit;}
	//static bool		IsWinServUPnPImplDisabled()			{return m_bIsWinServImplDisabled;}
	static bool		IsMinilibUPnPImplDisabled()			{return m_bIsMinilibImplDisabled;}
	static bool		IsACATUPnPImplDisabled()			{return m_bIsACATImplDisabled;}
	static int		GetLastWorkingUPnPImpl()			{return m_nLastWorkingImpl;}
	static void		SetLastWorkingUPnPImpl(int val)		{m_nLastWorkingImpl = val;}

	// Spamfilter
	static	bool		IsSearchSpamFilterEnabled()			{return m_bEnableSearchResultFilter;}
	
	static	bool		IsStoringSearchesEnabled()			{return m_bStoreSearches;}
	static	bool		GetPreventStandby()					{return m_bPreventStandby;}
	static uint16	GetRandomTCPPort();
	static uint16	GetRandomUDPPort();
	// ==> Invisible Mode [TPT/MoNKi] - Stulle
	static	void SetInvisibleMode(bool on, int keymodifier, char key);
	// <== Invisible Mode [TPT/MoNKi] - Stulle
#ifdef CLIENTANALYZER
	static	bool	GetLogAnalyzerEvents()				{return m_bLogAnalyzerEvents;}
#endif

// X-Ray :: FileStatusIcons :: Start
public:
	static	bool	m_bShowFileStatusIcons;
	static	bool	IsFileStatusIcons()					{return m_bShowFileStatusIcons;}
	static	void	SetFileStatusIcons(const bool& in)	{m_bShowFileStatusIcons = in;}
// X-Ray :: FileStatusIcons :: End
public:
// X-Ray :: SessionDownload :: Start
	static	bool	m_bShowSessionDownload;
	static	bool	GetUseSessionDownload()					{return m_bShowSessionDownload;}
// X-Ray :: SessionDownload :: End
// ==> Pay Back First [AndCycle/SiRoB/Stulle] - Stulle
	static	bool	IsPayBackFirst()		{return m_bPayBackFirst;}
	static	uint8	GetPayBackFirstLimit()	{return m_iPayBackFirstLimit;}
	//static	bool	IsPayBackFirst2()		{return m_bPayBackFirst2;}
	//static	uint16	GetPayBackFirstLimit2()	{return m_iPayBackFirstLimit2;}
	// <== Pay Back First [AndCycle/SiRoB/Stulle] - Stulle
	static	bool  m_bShowMinQR;
	static	bool  GetMinQR()					{return m_bShowMinQR;}

	static	bool  m_bShowDownloadColor;
	static	bool  GetDownloadColor()					{return m_bShowDownloadColor;}

protected:
	static	CString m_strFileCommentsFilePath;
	static	WORD m_wWinVer;
	static	CAtlArray<Category_Struct*> catMap;
	static	CString	m_astrDefaultDirs[13];
	static	bool	m_abDefaultDirsCreated[13];
	static	int		m_nCurrentUserDirMode; // Only for PPgTweaks

	static void	CreateUserHash();
	static void	SetStandartValues();
	static int	GetRecommendedMaxConnections();
	static void LoadPreferences();
	static void SavePreferences();
	static CString	GetHomepageBaseURLForLevel(int nLevel);
	static CString	GetDefaultDirectory(EDefaultDirectory eDirectory, bool bCreate = true);
};

extern CPreferences thePrefs;
extern bool g_bLowColorDesktop;
