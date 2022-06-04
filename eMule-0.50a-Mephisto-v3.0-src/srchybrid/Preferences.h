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

#include "Ini2.h" // Design Settings [eWombat/Stulle] - Stulle

const CString strDefaultToolbar = _T("0099010203040506070899091011");

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


enum EToolbarLabelType;
enum ELogFileFormat;

//Xman process prio
// [TPT] - Select process priority 
/*
#define PROCESSPRIORITYNUMBER 6
static const PriorityClasses[] = { REALTIME_PRIORITY_CLASS, HIGH_PRIORITY_CLASS, ABOVE_NORMAL_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS, BELOW_NORMAL_PRIORITY_CLASS, IDLE_PRIORITY_CLASS };
*/
// [TPT] - Select process priority 

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

// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
/*
struct Category_Struct{
	CString	strIncomingPath;
	CString	strTitle;
	CString	strComment;
	DWORD	color;
	UINT	prio;
	CString autocat;
	CString	regexp;
	int		filter;
	bool	filterNeg;
	bool	care4all;
	bool	ac_regexpeval;
	bool	downloadInAlphabeticalOrder; // ZZ:DownloadManager
};
*/
// View Filter Struct
#pragma pack(1)
struct CategoryViewFilter_Struct{
	//		General View Filters
	UINT	nFromCats;  // 0 == All; 1 == Unassigned; 2 == This Cat Only
	bool	bSuspendFilters;
	//		File Type View Filters
	bool	bVideo;
	bool	bAudio;
	bool	bArchives;
	bool	bImages;
	//		File State View Filters
	bool	bWaiting;
	bool	bTransferring;
	bool	bPaused;
	bool	bStopped;
	bool	bComplete;
	bool	bHashing;
	bool	bErrorUnknown;
	bool	bCompleting;
	bool	bSeenComplet;
	//		File Size View Filters
	uint64	nFSizeMin;
	uint64	nFSizeMax;
	uint64	nRSizeMin;
	uint64	nRSizeMax;
	//		Time Remaining Filters
	uint32	nTimeRemainingMin;
	uint32	nTimeRemainingMax;
	//		Source Count Filters
	UINT	nSourceCountMin;
	UINT	nSourceCountMax;
	UINT	nAvailSourceCountMin;
	UINT	nAvailSourceCountMax;
	//		Advanced Filter Mask
	CString	sAdvancedFilterMask;
};
#pragma pack()

// Criteria Selection Struct
#pragma pack(1)
struct CategorySelectionCriteria_Struct{
	bool	bFileSize;
	bool	bAdvancedFilterMask;
};
#pragma pack()

#pragma pack(1)
struct Category_Struct{
	CString	strIncomingPath;
	CString	strTitle;
	CString	strComment;
	DWORD	color;
	UINT	prio;
	bool	bResumeFileOnlyInSameCat;
	// View Filter Struct
	CategoryViewFilter_Struct viewfilters;
	CategorySelectionCriteria_Struct selectioncriteria;
	UINT	m_iDlMode; // 0 = NONE, 1 = alphabetical, 2 = LP
};
#pragma pack()
// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

// ==> Design Settings [eWombat/Stulle] - Stulle
#define STYLE_VERSION 3
struct StylesStruct 
{
	short nOnOff;
	DWORD nFlags;
	COLORREF nFontColor;
	COLORREF nBackColor;
};

#define STYLE_BOLD		0x00000001
#define STYLE_UNDERLINE	0x00000002
#define STYLE_ITALIC	0x00000004
#define STYLE_FONTMASK	0x00000007
#define STYLE_USED		0x80000000

// master styles
enum eMasterStyles
{
	client_styles = 0,
	download_styles,
	share_styles,
	server_styles,
	background_styles,
	window_styles,
	feedback_styles, // Feedback personalization [Stulle] - Stulle
	master_count
};

// client styles
enum eClientStyles
{
	style_c_default = 0,
	style_c_friend,
	style_c_powershare,
	style_c_powerrelease,
	style_c_downloading,
	style_c_uploading,
	style_c_leecher,
	style_c_lowid,
	style_c_credits,
	style_c_count
};

// download styles
enum eDownloadStyles
{
	style_d_default = 0,
	style_d_downloading,
	style_d_complete,
	style_d_completing,
	style_d_hashing,
	style_d_paused,
	style_d_stopped,
	style_d_errunk,
	style_d_count
};

// share styles
enum eShareStyles
{
	style_s_default = 0,
	style_s_incomplete,
	style_s_powershare,
	style_s_auto,
	style_s_verylow,
	style_s_low,
	style_s_normal,
	style_s_high,
	style_s_release,
	style_s_powerrelease,
	style_s_shareable,
	style_s_count
};

// server styles
enum eServerStyles
{
	style_se_default = 0,
	style_se_connected,
	style_se_static,
	style_se_filtered,
	style_se_dead,
	style_se_unreliable,
	style_se_count
};

// background styles
enum eBackgroundStyles
{
	style_b_default = 0,
	style_b_clientlist,
	style_b_dlclientlist,
	style_b_queuelist,
	style_b_uploadlist,
	style_b_downloadlist,
	style_b_sharedlist,
	style_b_serverwnd,
	style_b_count
};

// window styles
enum eWindowStyles
{
	style_w_default = 0,
	style_w_kademlia,
	style_w_server,
	style_w_transfer,
	style_w_search,
	style_w_shared,
	style_w_messages,
	style_w_irc,
	style_w_statistic,
	style_w_statusbar,
	style_w_toolbar,
	style_w_count
};

// ==> Feedback personalization [Stulle] - Stulle
// feedback styles
enum eFeedBackStyles
{
	style_f_default = 0,
	style_f_label,
	style_f_names,
	style_f_fileinfo,
	style_f_filestate,
	style_f_transferred,
	style_f_requests,
	style_f_sources,
	style_f_clientsonqueue,
	style_f_compeltesrc,
	style_f_count
};
// <== Feedback personalization [Stulle] - Stulle
// <== Design Settings [eWombat/Stulle] - Stulle

// ==> Multiple Part Transfer [Stulle] - Mephisto
enum eChunkModes
{
	CHUNK_SCORE = 0,
	CHUNK_XMAN,
	CHUNK_FINISH,
	CHUNK_FULL
};
// <== Multiple Part Transfer [Stulle] - Mephisto

class CPreferences
{
public:
	static	CString	strNick;
	// ZZ:UploadSpeedSense -->
	static	uint16	minupload;
	// ZZ:UploadSpeedSense <--
	//Xman
	/*
	static	uint16	maxupload;
	static	uint16	maxdownload;
	*/
	//Xman end
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
	static	CStringArray	tempdir;
	static	bool	ICH;
	static	bool	m_bAutoUpdateServerList;
	static	bool	updatenotify;
	static	bool	mintotray;
	static	bool	autoconnect;
	static	bool	m_bAutoConnectToStaticServersOnly; // Barry
	static	bool	autotakeed2klinks;	   // Barry
	static	bool	addnewfilespaused;	   // Barry
	static	UINT	depth3D;			   // Barry
	static	bool	m_bEnableMiniMule;
	static	int		m_iStraightWindowStyles;
	static  bool	m_bUseSystemFontForMainControls;
	static	bool	m_bRTLWindowsLayout;
	static	CString	m_strSkinProfile;
	static	CString	m_strSkinProfileDir;
	static	bool	m_bAddServersFromServer;
	static	bool	m_bAddServersFromClients;
	static	UINT	maxsourceperfile;
	static	UINT	trafficOMeterInterval;
	static	UINT	statsInterval;
	static	bool	m_bFillGraphs;
	static	uchar	userhash[16];
	static	WINDOWPLACEMENT EmuleWindowPlacement;
	//Xman
	/*
	static	int		maxGraphDownloadRate;
	static	int		maxGraphUploadRate;
	static	uint32	maxGraphUploadRateEstimated;
	*/
	//Xman end
	static	bool	beepOnError;
	static	bool	confirmExit;

	//Xman Xtreme Upload: this graph isn't shown at xtreme
	//Maella Bandwidth control
	/*
	static	DWORD	m_adwStatsColors[15];
	*/
	// ==> Source Graph - Stulle
	/*
	static	DWORD	m_adwStatsColors[14];
	//Xman end
	*/
	static	DWORD	m_adwStatsColors[15];
	// <== Source Graph - Stulle
	static	bool	bHasCustomTaskIconColor;
	static  bool	m_bIconflashOnNewMessage;

	static	bool	splashscreen;
	static	bool	filterLANIPs;
	static	bool	m_bAllocLocalHostIP;
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
	static	uint64	cumUpData_EDONKEY;
	static	uint64	cumUpData_EDONKEYHYBRID;
	static	uint64	cumUpData_EMULE;
	static	uint64	cumUpData_MLDONKEY;
	static	uint64	cumUpData_AMULE;
	static	uint64	cumUpData_EMULECOMPAT;
	static	uint64	cumUpData_SHAREAZA;
	// Session client breakdown stats for sent bytes...
	static	uint64	sesUpData_EDONKEY;
	static	uint64	sesUpData_EDONKEYHYBRID;
	static	uint64	sesUpData_EMULE;
	static	uint64	sesUpData_MLDONKEY;
	static	uint64	sesUpData_AMULE;
	static	uint64	sesUpData_EMULECOMPAT;
	static	uint64	sesUpData_SHAREAZA;

	// Cumulative port breakdown stats for sent bytes...
	static	uint64	cumUpDataPort_4662;
	static	uint64	cumUpDataPort_OTHER;
	static	uint64	cumUpDataPort_PeerCache;
	// Session port breakdown stats for sent bytes...
	static	uint64	sesUpDataPort_4662;
	static	uint64	sesUpDataPort_OTHER;
	static	uint64	sesUpDataPort_PeerCache;

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
	static	uint64	cumDownData_EDONKEY;
	static	uint64	cumDownData_EDONKEYHYBRID;
	static	uint64	cumDownData_EMULE;
	static	uint64	cumDownData_MLDONKEY;
	static	uint64	cumDownData_AMULE;
	static	uint64	cumDownData_EMULECOMPAT;
	static	uint64	cumDownData_SHAREAZA;
	static	uint64	cumDownData_URL;
	// Session client breakdown stats for received bytes...
	static	uint64	sesDownData_EDONKEY;
	static	uint64	sesDownData_EDONKEYHYBRID;
	static	uint64	sesDownData_EMULE;
	static	uint64	sesDownData_MLDONKEY;
	static	uint64	sesDownData_AMULE;
	static	uint64	sesDownData_EMULECOMPAT;
	static	uint64	sesDownData_SHAREAZA;
	static	uint64	sesDownData_URL;

	// Cumulative port breakdown stats for received bytes...
	static	uint64	cumDownDataPort_4662;
	static	uint64	cumDownDataPort_OTHER;
	static	uint64	cumDownDataPort_PeerCache;
	// Session port breakdown stats for received bytes...
	static	uint64	sesDownDataPort_4662;
	static	uint64	sesDownDataPort_OTHER;
	static	uint64	sesDownDataPort_PeerCache;

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
	//MORPH START - Added by SiRoB, Splitting Bar [O²]
	static	UINT	splitterbarPositionStat;
	static	UINT	splitterbarPositionStat_HL;
	static	UINT	splitterbarPositionStat_HR;
	static	UINT	splitterbarPositionFriend;
	static	UINT	splitterbarPositionIRC;
	static	UINT	splitterbarPositionShared;
	//MORPH END - Added by SiRoB, Splitting Bar [O²]
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

	static	CString	m_strIRCServer;
	static	CString	m_strIRCNick;
	static	CString	m_strIRCChannelFilter;
	static	bool	m_bIRCAddTimeStamp;
	static	bool	m_bIRCUseChannelFilter;
	static	UINT	m_uIRCChannelUserFilter;
	static	CString	m_strIRCPerformString;
	static	bool	m_bIRCUsePerform;
	static	bool	m_bIRCGetChannelsOnConnect;
	static	bool	m_bIRCAcceptLinks;
	static	bool	m_bIRCAcceptLinksFriendsOnly;
	static	bool	m_bIRCPlaySoundEvents;
	static	bool	m_bIRCIgnoreMiscMessages;
	static	bool	m_bIRCIgnoreJoinMessages;
	static	bool	m_bIRCIgnorePartMessages;
	static	bool	m_bIRCIgnoreQuitMessages;
	static	bool	m_bIRCIgnoreEmuleAddFriendMsgs;
	static	bool	m_bIRCAllowEmuleAddFriend;
	static	bool	m_bIRCIgnoreEmuleSendLinkMsgs;
	static	bool	m_bIRCJoinHelpChannel;
	static	bool	m_bIRCEnableSmileys;
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
	static	bool	m_bLogSecureIdent;
	static	bool	m_bLogFilteredIPs;
	static	bool	m_bLogFileSaving;
    static  bool    m_bLogA4AF; // ZZ:DownloadManager
	static	bool	m_bLogDrop; //Xman Xtreme Downloadmanager
	static	bool	m_bLogpartmismatch; //Xman Log part/size-mismatch
	static	bool	m_bLogUlDlEvents;
	static	bool	m_bUseDebugDevice;
	static	int		m_iDebugServerTCPLevel;
	static	int		m_iDebugServerUDPLevel;
	static	int		m_iDebugServerSourcesLevel;
	static	int		m_iDebugServerSearchesLevel;
	static	int		m_iDebugClientTCPLevel;
	static	int		m_iDebugClientUDPLevel;
	static	int		m_iDebugClientKadUDPLevel;
	static	int		m_iDebugSearchResultDetailLevel;
	static	bool	m_bupdatequeuelist;
	static	bool	m_bManualAddedServersHighPriority;
	static	bool	m_btransferfullchunks;
	static	int		m_istartnextfile;
	static	bool	m_bshowoverhead;
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
	// ==> CreditSystems [EastShare/ MorphXT] - Stulle
	/*
	static	bool	m_bCreditSystem;
	*/
	// <== CreditSystems [EastShare/ MorphXT] - Stulle

	static	bool	log2disk;
	static	bool	debug2disk;
	static	int		iMaxLogBuff;
	static	UINT	uMaxLogFileSize;
	static	ELogFileFormat m_iLogFileFormat;
	static	bool	scheduler;
	static	bool	dontcompressavi;
	static	bool	msgonlyfriends;
	static	bool	msgsecure;
	static	bool	m_bUseChatCaptchas;

	static	UINT	filterlevel;
	static	UINT	m_iFileBufferSize;
	static	UINT	m_iQueueSize;
	static	int		m_iCommitFiles;
	static	UINT	m_uFileBufferTimeLimit;

	static	UINT	maxmsgsessions;
	// ==> Make code VS 2005 and VS 2008 ready [MorphXT] - Stulle
	/*
	static	uint32	versioncheckLastAutomatic;
	*/
	static	time_t	versioncheckLastAutomatic;
	// <== Make code VS 2005 and VS 2008 ready [MorphXT] - Stulle
	//Xman versions check
	static	uint32	mversioncheckLastAutomatic;
	//Xman end
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
	static	int		m_nWebPageRefresh;
	static	bool	m_bWebLowEnabled;
	static	int		m_iWebTimeoutMins;
	static	int		m_iWebFileUploadSizeLimitMB;
	static	CString	m_strTemplateFile;
	static	ProxySettings proxy; // deadlake PROXYSUPPORT
	static  bool	m_bAllowAdminHiLevFunc;
	static	CUIntArray m_aAllowedRemoteAccessIPs;

	static	bool	showCatTabInfos;
	static	bool	resumeSameCat;
	static	bool	dontRecreateGraphs;
	static	bool	autofilenamecleanup;
	//static	int		allcatType;
	//static	bool	allcatTypeNeg;
	static	bool	m_bUseAutocompl;
	static	bool	m_bShowDwlPercentage;
	static	bool	m_bRemoveFinishedDownloads;
	static	UINT	m_iMaxChatHistory;
	static	bool	m_bShowActiveDownloadsBold;

	static	int		m_iSearchMethod;
	static	bool	m_bAdvancedSpamfilter;
	static	bool	m_bUseSecureIdent;
	// mobilemule
	static	CString	m_strMMPassword;
	static	bool	m_bMMEnabled;
	static	uint16	m_nMMPort;

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
	static	bool	m_bDynUpEnabled;
	static	int		m_iDynUpPingTolerance;
	static	int		m_iDynUpGoingUpDivider;
	static	int		m_iDynUpGoingDownDivider;
	static	int		m_iDynUpNumberOfPings;
	static  int		m_iDynUpPingToleranceMilliseconds;
	static  bool	m_bDynUpUseMillisecondPingTolerance;
	// ZZ:UploadSpeedSense <--

	//Xman
	/*
    static bool     m_bA4AFSaveCpu; // ZZ:DownloadManager
	*/
	//Xman end

    static bool     m_bHighresTimer;

	static	bool	m_bResolveSharedShellLinks;
	static	CStringList shareddir_list;
	static	CStringList addresses_list;
	static	bool	m_bKeepUnavailableFixedSharedDirs;

	static	int		m_iDbgHeap;
	static	UINT	m_nWebMirrorAlertLevel;
	static	bool	m_bRunAsUser;
	static	bool	m_bPreferRestrictedOverUser;

	static  bool	m_bUseOldTimeRemaining;

	// PeerCache
	static	uint32	m_uPeerCacheLastSearch;
	static	bool	m_bPeerCacheWasFound;
	static	bool	m_bPeerCacheEnabled;
	static	uint16	m_nPeerCachePort;
	static	bool	m_bPeerCacheShow;

	// Firewall settings
	static bool		m_bOpenPortsOnStartUp;

	//AICH Options
	static bool		m_bTrustEveryHash;
	
	// files
	static bool		m_bRememberCancelledFiles;
	static bool		m_bRememberDownloadedFiles;
	static bool		m_bPartiallyPurgeOldKnownFiles;

	//emil notifier
	static bool		m_bNotifierSendMail;
	static CString	m_strNotifierMailServer;
	static CString	m_strNotifierMailSender;
	static CString	m_strNotifierMailReceiver;

	// encryption / obfuscation / verification
	static bool		m_bCryptLayerRequested;
	static bool		m_bCryptLayerSupported;
	static bool		m_bCryptLayerRequired;
	static uint8	m_byCryptTCPPaddingLength;
	static uint32   m_dwKadUDPKey;

	// ==> UPnP support [MoNKi] - leuk_he
	/*
	// UPnP
	static bool		m_bSkipWANIPSetup;
	static bool		m_bSkipWANPPPSetup;
	static bool		m_bEnableUPnP;
	static bool		m_bCloseUPnPOnExit;
	static bool		m_bIsWinServImplDisabled;
	static bool		m_bIsMinilibImplDisabled;
	static int		m_nLastWorkingImpl;

#ifdef DUAL_UPNP //zz_fly :: dual upnp
	//UPnP chooser
	static bool		m_bUseACATUPnPCurrent;
	static bool		m_bUseACATUPnPNextStart;

	//ACAT UPnP
	static	bool m_bUPnPNat; // UPnP On/Off
	static	bool m_bUPnPTryRandom; //UPnP use random ports
	static	uint16 m_iUPnPTCPExternal; // TCP External Port
	static	uint16 m_iUPnPUDPExternal; // UDP External Port
	static	bool GetUPnPNat()    { return m_bUPnPNat; }
	static	void SetUPnPNat(bool on)    { m_bUPnPNat = on; }
	static	void SetUPnPTCPExternal(uint16 port) { m_iUPnPTCPExternal = port; }
	static	void SetUPnPUDPExternal(uint16 port) { m_iUPnPUDPExternal = port; }
	static	bool GetUPnPNatTryRandom() { return m_bUPnPTryRandom; }
	static	void SetUPnPNatTryRandom(bool on) { m_bUPnPTryRandom = on; }
#endif //zz_fly :: dual upnp

	//zz_fly :: Rebind UPnP on IP-change
	static  bool m_bUPnPRebindOnIPChange;
	static  bool GetUPnPNatRebind() { return m_bUPnPRebindOnIPChange; }
	static	void SetUPnPNatRebind(bool on) { m_bUPnPRebindOnIPChange = on; }
	//zz_fy :: end
	*/
	// <== UPnP support [MoNKi] - leuk_he

	// Spam
	static bool		m_bEnableSearchResultFilter;

	static BOOL		m_bIsRunningAeroGlass;
	static bool		m_bPreventStandby;
	static bool		m_bStoreSearches;


	// ==> Advanced Options [Official/MorphXT] - Stulle
	static bool bMiniMuleAutoClose;
	static int  iMiniMuleTransparency ;
	static bool bCheckComctl32 ;
	static bool bCheckShell32;
	static bool bIgnoreInstances;
	static CString sNotifierMailEncryptCertName;
	static CString sMediaInfo_MediaInfoDllPath ;
	static bool bMediaInfo_RIFF ;
	static bool bMediaInfo_ID3LIB; 
#ifdef HAVE_QEDIT_H
	static bool m_bMediaInfo_MediaDet;
#endif//HAVE_QEDIT_H
	static bool m_bMediaInfo_RM;
#ifdef HAVE_WMSDK_H
	static bool m_bMediaInfo_WM;
#endif//HAVE_WMSDK_H
	static CString sInternetSecurityZone;
	// <== Advanced Options [Official/MorphXT] - Stulle

	// ==> Global Source Limit [Max/Stulle] - Stulle
    static  UINT	m_uGlobalHL;
	static	bool	m_bGlobalHL;
	static	bool	m_bGlobalHlAll;
    static  bool	m_bGlobalHlDefault;
	// <== Global Source Limit [Max/Stulle] - Stulle

	// ==> push small files [sivka] - Stulle
    static  bool	enablePushSmallFile;
    static	uint32	m_iPushSmallFiles;
	static	uint16	m_iPushSmallBoost;
	// <== push small files [sivka] - Stulle
    static  bool enablePushRareFile; // push rare file - Stulle

	static bool	showSrcInTitle; // Show sources on title - Stulle
	static bool	showOverheadInTitle; // show overhead on title - Stulle
	static bool ShowGlobalHL; // show global HL - Stulle
	static bool ShowFileHLconst; // show HL per file constantly - Stulle
	static bool m_bShowInMSN7; //Show in MSN7 [TPT] - Stulle
    static bool m_bClientQueueProgressBar; // Client queue progress bar [Commander] - Stulle
	static bool m_bShowClientPercentage; // Show Client Percentage optional [Stulle] - Stulle
	// ==> CPU/MEM usage [$ick$/Stulle] - Max
	static bool m_bSysInfo;
	static bool m_bSysInfoGlobal;
	// <== CPU/MEM usage [$ick$/Stulle] - Max
	static bool m_bShowSpeedMeter; // High resolution speedmeter on toolbar [eFMod/Stulle] - Myth88
	// ==> Static Tray Icon [MorphXT] - MyTh88
	static bool m_bStaticIcon; 
	// <== Static Tray Icon [MorphXT] - MyTh88

	static uint8 creditSystemMode; // CreditSystems [EastShare/ MorphXT] - Stulle

	static bool	m_bSaveUploadQueueWaitTime; // SUQWT [Moonlight/EastShare/ MorphXT] - Stulle

	// ==> File Settings [sivka/Stulle] - Stulle
	static	uint16	m_MaxSourcesPerFileTemp;
	static	bool	m_EnableAutoDropNNSTemp;
	static	DWORD	m_AutoNNS_TimerTemp;
	static	uint16  m_MaxRemoveNNSLimitTemp;
	static	bool	m_EnableAutoDropFQSTemp;
	static	DWORD	m_AutoFQS_TimerTemp;
	static	uint16  m_MaxRemoveFQSLimitTemp;
	static	bool	m_EnableAutoDropQRSTemp;
	static	DWORD	m_AutoHQRS_TimerTemp;
	static	uint16  m_MaxRemoveQRSTemp;
	static	uint16  m_MaxRemoveQRSLimitTemp;
	static	bool	m_bHQRXmanTemp;
	static	bool	m_bGlobalHlTemp; // Global Source Limit (customize for files) - Stulle
	static	bool	m_EnableAutoDropNNSDefault;
	static	DWORD	m_AutoNNS_TimerDefault;
	static	uint16  m_MaxRemoveNNSLimitDefault;
	static	bool	m_EnableAutoDropFQSDefault;
	static	DWORD	m_AutoFQS_TimerDefault;
	static	uint16  m_MaxRemoveFQSLimitDefault;
	static	bool	m_EnableAutoDropQRSDefault;
	static	DWORD	m_AutoHQRS_TimerDefault;
	static	uint16  m_MaxRemoveQRSDefault;
	static	uint16  m_MaxRemoveQRSLimitDefault;
	static	bool	m_bHQRXmanDefault;
	static	bool	m_MaxSourcesPerFileTakeOver;
	static	bool	m_EnableAutoDropNNSTakeOver;
	static	bool	m_AutoNNS_TimerTakeOver;
	static	bool	m_MaxRemoveNNSLimitTakeOver;
	static	bool	m_EnableAutoDropFQSTakeOver;
	static	bool	m_AutoFQS_TimerTakeOver;
	static	bool	m_MaxRemoveFQSLimitTakeOver;
	static	bool	m_EnableAutoDropQRSTakeOver;
	static	bool	m_AutoHQRS_TimerTakeOver;
	static	bool	m_MaxRemoveQRSTakeOver;
	static	bool	m_MaxRemoveQRSLimitTakeOver;
	static	bool	m_bHQRXmanTakeOver;
	static	bool	m_bGlobalHlTakeOver; // Global Source Limit (customize for files) - Stulle
	static	bool	m_TakeOverFileSettings;
	// <== File Settings [sivka/Stulle] - Stulle

	// ==> Source Graph - Stulle
	static bool	m_bSrcGraph;
	static uint16  m_iStatsHLMin;
	static uint16  m_iStatsHLMax;
	static uint16  m_iStatsHLDif;
	// <== Source Graph - Stulle

	// ==> FunnyNick [SiRoB/Stulle] - Stulle
	static uint8	FnTagMode;
	static TCHAR	m_sFnCustomTag [256];
	static bool		m_bFnTagAtEnd;
	// <== FunnyNick [SiRoB/Stulle] - Stulle

	static bool		m_bACC; // ACC [Max/WiZaRd] - Max

	static uint32	m_uScarVerCheckLastAutomatic; // ScarAngel Version Check - Stulle

	// ==> Quick start [TPT] - Max
	static	bool	m_bQuickStart;
	static	uint16  m_iQuickStartMaxTime;
	static	UINT    m_iQuickStartMaxConn;
	static	uint16  m_iQuickStartMaxConnPerFive;
	static	UINT    m_iQuickStartMaxConnBack;
	static	uint16  m_iQuickStartMaxConnPerFiveBack;
	static	bool	m_bQuickStartAfterIPChange;
	// <== Quick start [TPT] - Max

	// ==> TBH: Backup [TBH/EastShare/MorphXT] - Stulle
	static	bool	m_bAutoBackup;
	static	bool	m_bAutoBackup2;
	// <== TBH: Backup [TBH/EastShare/MorphXT] - Stulle

	// ==> TBH: minimule - Max
	static  int		speedmetermin;
	static  int		speedmetermax;
	static  bool	m_bMiniMule;
	static  uint32	m_iMiniMuleUpdate;
	static  bool	m_bMiniMuleLives;
	static  uint8	m_iMiniMuleTransparency;
	static	bool	m_bMMCompl;
	static	bool	m_bMMOpen;
	// <== TBH: minimule - Max

	// ==> Simple cleanup [MorphXT] - Stulle
	static int      m_SimpleCleanupOptions;
	static CString  m_SimpleCleanupSearch;
	static CString  m_SimpleCleanupReplace;
	static CString  m_SimpleCleanupSearchChars;
	static CString  m_SimpleCleanupReplaceChars;
	// <== Simple cleanup [MorphXT] - Stulle

	static	bool	startupsound; // Startupsound [Commander] - mav744

	static uint8	m_uCompressLevel; // Adjust Compress Level [Stulle] - Stulle

	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	static bool		m_bValidSrcsOnly;
	static bool		m_bShowCatNames;
	static bool		m_bActiveCatDefault;
	static bool		m_bSelCatOnAdd;
	static bool		m_bAutoSetResumeOrder;
	static bool		m_bSmallFileDLPush;
	static uint8	m_iStartDLInEmptyCats;
	static bool		m_bRespectMaxSources;
	static bool		m_bUseAutoCat;
	static uint8	dlMode;
	static bool		m_bAddRemovedInc;
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

	static bool		m_bSpreadbarSetStatus; // Spread bars [Slugfiller/MorphXT] - Stulle
	// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	static int	hideOS;
	static bool	selectiveShare;
	static int	ShareOnlyTheNeed;
	// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle

	// ==> PowerShare [ZZ/MorphXT] - Stulle
	static int	m_iPowershareMode;
	static int	PowerShareLimit;
	// <== PowerShare [ZZ/MorphXT] - Stulle

	static	uint8	m_uReleaseBonus; // Release Bonus [sivka] - Stulle
	static	bool	m_bReleaseScoreAssurance; // Release Score Assurance [Stulle] - Stulle

	static	int		GetPsAmountLimit() {return PsAmountLimit;} // Limit PS by amount of data uploaded [Stulle] - Stulle

	// ==> Design Settings [eWombat/Stulle] - Stulle
	// client styles
	static DWORD		nClientStyleFlags[style_c_count]; 
	static COLORREF		nClientStyleFontColor[style_c_count];
	static COLORREF		nClientStyleBackColor[style_c_count];
	static short		nClientStyleOnOff[style_c_count];

	// download styles
	static DWORD		nDownloadStyleFlags[style_d_count]; 
	static COLORREF		nDownloadStyleFontColor[style_d_count];
	static COLORREF		nDownloadStyleBackColor[style_d_count];
	static short		nDownloadStyleOnOff[style_d_count];

	// share styles
	static DWORD		nShareStyleFlags[style_s_count]; 
	static COLORREF		nShareStyleFontColor[style_s_count];
	static COLORREF		nShareStyleBackColor[style_s_count];
	static short		nShareStyleOnOff[style_s_count];

	// server styles
	static DWORD		nServerStyleFlags[style_se_count]; 
	static COLORREF		nServerStyleFontColor[style_se_count];
	static COLORREF		nServerStyleBackColor[style_se_count];
	static short		nServerStyleOnOff[style_se_count];

	// background styles
	static DWORD		nBackgroundStyleFlags[style_b_count]; 
	static COLORREF		nBackgroundStyleFontColor[style_b_count];
	static COLORREF		nBackgroundStyleBackColor[style_b_count];
	static short		nBackgroundStyleOnOff[style_b_count];

	// window styles
	static DWORD		nWindowStyleFlags[style_w_count]; 
	static COLORREF		nWindowStyleFontColor[style_w_count];
	static COLORREF		nWindowStyleBackColor[style_w_count];
	static short		nWindowStyleOnOff[style_w_count];

	// ==> Feedback personalization [Stulle] - Stulle
	// feedback styles
	static DWORD		nFeedBackStyleFlags[style_f_count]; 
	static COLORREF		nFeedBackStyleFontColor[style_f_count];
	static COLORREF		nFeedBackStyleBackColor[style_f_count];
	static short		nFeedBackStyleOnOff[style_f_count];
	// <== Feedback personalization [Stulle] - Stulle
	// <== Design Settings [eWombat/Stulle] - Stulle

	// ==> Enforce Ratio [Stulle] - Stulle
	static bool		m_bEnforceRatio;
	static uint8	m_uRatioValue;
	// <== Enforce Ratio [Stulle] - Stulle

	// ==> Improved ICS-Firewall support [MoNKi]-Max
	static bool		m_bICFSupport;
	static bool		m_bICFSupportFirstTime;
	static bool		m_bICFSupportStatusChanged;
	static bool		m_bICFSupportServerUDP;
	// <== Improved ICS-Firewall support [MoNKi]-Max

	// ==> Invisible Mode [TPT/MoNKi] - Stulle
	static bool		m_bInvisibleMode;		
	static UINT		m_iInvisibleModeHotKeyModifier;
	static char		m_cInvisibleModeHotKey;
	static bool		m_bInvisibleModeStart;
	// <== Invisible Mode [TPT/MoNKi] - Stulle

	//==> UPnP support [MoNKi] - leuk_he
	static bool		m_bUPnPNat;
	static bool		m_bUPnPNatWeb;
	static bool		m_bUPnPVerboseLog;
	static uint16	m_iUPnPPort;
	static bool		m_bUPnPLimitToFirstConnection;
	static bool		m_bUPnPClearOnClose;
	static int    m_iDetectuPnP; //leuk_he autodetect in startup wizard
	static DWORD	 m_dwUpnpBindAddr;
	static bool      m_bBindAddrIsDhcp;
	static bool     m_bUPnPForceUpdate;
	//<== UPnP support [MoNKi] - leuk_he

	// ==> Random Ports [MoNKi] - Stulle
	static bool		m_bRndPorts;
	static uint16	m_iMinRndPort;
	static uint16	m_iMaxRndPort;
	static bool		m_bRndPortsResetOnRestart;
	static uint16	m_iRndPortsSafeResetOnRestartTime;
	static uint16	m_iCurrentTCPRndPort;
	static uint16	m_iCurrentUDPRndPort;
	// <== Random Ports [MoNKi] - Stulle

	// ==> Automatic shared files updater [MoNKi] - Stulle
	static bool		m_bDirectoryWatcher;
	static bool		m_bSingleSharedDirWatcher;
	static uint32	m_uTimeBetweenReloads;
	// <== Automatic shared files updater [MoNKi] - Stulle

	// ==> Anti Uploader Ban [Stulle] - Stulle
	static uint16 m_uAntiUploaderBanLimit;
	static uint8 AntiUploaderBanCaseMode;
	// <== Anti Uploader Ban [Stulle] - Stulle

	// ==> Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	static	bool	m_bEmuMLDonkey;
	static	bool	m_bEmueDonkey;
	static	bool	m_bEmueDonkeyHybrid;
	static	bool	m_bEmuShareaza;
	static  bool    m_bEmuLphant;
	static	bool	m_bLogEmulator;
	// <== Emulate others [WiZaRd/Spike/shadow2004] - Stulle

	// ==> Spread Credits Slot [Stulle] - Stulle
	static bool	SpreadCreditsSlot;
	static uint16 SpreadCreditsSlotCounter;
	// <== Spread Credits Slot [Stulle] - Stulle

	// ==> Pay Back First [AndCycle/SiRoB/Stulle] - Stulle
	static bool		m_bPayBackFirst;
	static uint8	m_iPayBackFirstLimit;
	static bool		m_bPayBackFirst2;
	static uint16	m_iPayBackFirstLimit2;
	// <== Pay Back First [AndCycle/SiRoB/Stulle] - Stulle

	static bool		m_bIgnoreThird; // Do not reserve 1/3 of your uploadlimit for emule [Stulle] - Stulle

	static bool		m_bDisableUlThres; // Disable accepting only clients who asked within last 30min [Stulle] - Stulle

	static bool		m_bFollowTheMajority; // Follow The Majority [AndCycle/Stulle] - Stulle

	static int		m_iFairPlay; // Fair Play [AndCycle/Stulle] - Stulle

	static bool		m_bMaxSlotSpeed; // Alwasy maximize slot speed [Stulle] - Stulle

	static uint32	m_uReAskTimeDif; // Timer for ReAsk File Sources [Stulle] - Stulle

	// ==> Advanced Updates [MorphXT/Stulle] - Stulle
	static bool			m_bAutoUpdateAntiLeech;
	static CString		m_strAntiLeechURL;
	static uint32		m_uIPFilterVersionNum;
	static SYSTEMTIME	m_IP2CountryVersion;
	static bool			AutoUpdateIP2Country;
	static CString		UpdateURLIP2Country;
	// <== Advanced Updates [MorphXT/Stulle] - Stulle

	static bool		m_bFineCS; // Modified FineCS [CiccioBastardo/Stulle] - Stulle

	static bool		m_bTrayComplete; // Completed in Tray [Stulle] - Stulle

	static bool		m_bColorFeedback; // Feedback personalization [Stulle] - Stulle

	static bool		m_bSplitWindow; // Advanced Transfer Window Layout [Stulle] - Stulle

	static bool		m_bDateFileNameLog; // Date File Name Log [AndCycle] - Stulle

	static bool		m_bIonixWebsrv; // Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he/Stulle] - Stulle

	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	static int		m_iServiceStartupMode;
	static int		m_iServiceOptLvl;
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle
	// ==> Adjustable NT Service Strings [Stulle] - Stulle
	static CString	m_strServiceName;
	static CString	m_strServiceDispName;
	static CString	m_strServiceDescr;
	static bool		m_bServiceStringsLoaded;
	// <== Adjustable NT Service Strings [Stulle] - Stulle

	static bool		m_bCloseEasteregg; // Diabolic Easteregg [Stulle] - Mephisto

	// ==> Mephisto Upload - Mephisto
	static uint8	m_uMinSlots;
	static uint8	m_uNoNewSlotTimer;
	static uint8	m_uFullLoops;
	static uint8	m_uMonitorLoops;
	static uint8	m_uNotReachedBW;
	static uint8	m_uNoTrickleTimer;
	static uint16	m_uMoveDownKB;
	// <== Mephisto Upload - Mephisto

	// ==> Multiple Part Transfer [Stulle] - Mephisto
	static uint8	m_uChunksMode;
	static uint8	m_uChunksToFinish;
	static uint8	m_uChunksToUpload;
	// <== Multiple Part Transfer [Stulle] - Mephisto
	static uint16	m_uMaxUpMinutes; // Adjust max upload time [Stulle] - Mephisto

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
		tableIrcMain,
		tableIrcChannels,
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
	friend class CPPgIRC;
	friend class Wizard;
	friend class CPPgTweaks;
	friend class CPPgDisplay;
	friend class CPPgSecurity;
	friend class CPPgScheduler;
	friend class CPPgDebug;

	friend class CPPgScar; // ScarAngel Preferences window - Stulle
	friend class CSivkaFileSettings; // File Settings [sivka/Stulle] - Stulle
	friend class CPPgWebServer; // Run eMule as NT Service [leuk_he/Stulle] - Stulle

	CPreferences();
	~CPreferences();

	static	void	Init();
	static	void	Uninit();

	static	LPCTSTR GetTempDir(int id = 0)				{return (LPCTSTR)tempdir.GetAt((id < tempdir.GetCount()) ? id : 0);}
	static	int		GetTempDirCount()					{return tempdir.GetCount();}
	static	bool	CanFSHandleLargeFiles(int nForCat);
	static	LPCTSTR GetConfigFile();
	static	const CString& GetFileCommentsFilePath()	{return m_strFileCommentsFilePath;}
	static	CString	GetMuleDirectory(EDefaultDirectory eDirectory, bool bCreate = true);
	static	void	SetMuleDirectory(EDefaultDirectory eDirectory, CString strNewDir);
	static	void	ChangeUserDirMode(int nNewMode);

	// SLUGFILLER: SafeHash remove - removed installation dir unsharing
	/*
	static	bool	IsTempFile(const CString& rstrDirectory, const CString& rstrName);
	static	bool	IsShareableDirectory(const CString& rstrDirectory);
	static	bool	IsInstallationDirectory(const CString& rstrDir);
	*/
	static	bool	IsConfigFile(const CString& rstrDirectory, const CString& rstrName);
	// SLUGFILLER: SafeHash remove - removed installation dir unsharing

	static	bool	Save();
	static	void	SaveCats();

	static	bool	GetUseServerPriorities()			{return m_bUseServerPriorities;}
	static	bool	GetUseUserSortedServerList()		{return m_bUseUserSortedServerList;}
	static	bool	Reconnect()							{return reconnect;}
	static	const CString& GetUserNick()				{return strNick;}
	static	void	SetUserNick(LPCTSTR pszNick);
	static	int		GetMaxUserNickLength()				{return 50;}

	static	LPCSTR	GetBindAddrA()						{return m_pszBindAddrA; }
	static	LPCWSTR	GetBindAddrW()						{return m_pszBindAddrW; }

	// ==> UPnP support [MoNKi] - leuk_he
	/*
#ifdef DUAL_UPNP //zz_fly :: dual upnp
	//ACAT UPnP
	static	uint16	GetPort();
	static	uint16	GetUDPPort();
#else //zz_fly :: dual upnp
	static	uint16	GetPort()							{return port;}
	static	uint16	GetUDPPort()						{return udpport;}
#endif //zz_fly :: dual upnp
	*/
	// <== UPnP support [MoNKi] - leuk_he

	static	uint16	GetServerUDPPort()					{return nServerUDPPort;}
	static	uchar*	GetUserHash()						{return userhash;}
	// ZZ:UploadSpeedSense -->
	static	uint16	GetMinUpload()						{return minupload;}
	// ZZ:UploadSpeedSense <--
	//Xman
	/*
	static	uint16	GetMaxUpload()						{return maxupload;}
	*/
	//Xman end
	static	bool	IsICHEnabled()						{return ICH;}
	static	bool	GetAutoUpdateServerList()			{return m_bAutoUpdateServerList;}
	static	bool	UpdateNotify()						{return updatenotify;}
	static	bool	GetMinToTray()						{return mintotray;}
	static	bool	DoAutoConnect()						{return autoconnect;}
	static	void	SetAutoConnect(bool inautoconnect)	{autoconnect = inautoconnect;}
	static	bool	GetAddServersFromServer()			{return m_bAddServersFromServer;}
	static	bool	GetAddServersFromClients()			{return m_bAddServersFromClients;}
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
	static float	m_slotspeed;
	static bool		m_openmoreslots;
	static bool		m_bandwidthnotreachedslots;
	static void CheckSlotSpeed();
	static int		m_sendbuffersize;
	static int		GetSendbuffersize()	{return m_sendbuffersize;}
	static void		SetSendbuffersize(int in) {m_sendbuffersize=in;}
	static int		m_internetdownreactiontime;

	//Xman process prio
	// [TPT] - Select process priority 
	const DWORD GetMainProcessPriority() const { return m_MainProcessPriority; }
	void SetMainProcessPriority(DWORD in) {m_MainProcessPriority=in; }
	static	uint32	m_MainProcessPriority;
	// [TPT] - Select process priority 	


	//Xman Anti-Leecher
	static bool m_antileecher;
	static bool m_antileechername;
	static bool	m_antighost;
	static bool	m_antileecherbadhello;
	static bool	m_antileechersnafu;
	static bool	m_antileechermod;
	static bool	m_antileecherthief;
	static bool	m_antileecherspammer;
	static bool	m_antileecherxsexploiter;
	static bool m_antileecheremcrypt;
	static bool m_antileecheruserhash;
	static bool	m_antileechercommunity_action;
	static bool	m_antileecherghost_action;
	static bool	m_antileecherthief_action;
	static bool GetAntiLeecher() {return m_antileecher;}
	static bool GetAntiLeecherName() {return m_antileecher && m_antileechername;}
	static bool GetAntiGhost() {return m_antileecher && m_antighost;}
	static bool GetAntiLeecherBadHello() {return m_antileecher && m_antileecherbadhello;}
	static bool GetAntiLeecherSnafu() {return m_antileecher && m_antileechersnafu;}
	static bool GetAntiLeecherMod() {return m_antileecher && m_antileechermod;}
	static bool GetAntiLeecherThief() {return m_antileecher && m_antileecherthief;}
	static bool GetAntiLeecherspammer() {return m_antileecher && m_antileecherspammer;}
	static bool GetAntiLeecherXSExploiter() {return m_antileecher && m_antileecherxsexploiter;}
	static bool GetAntiLeecheremcrypt() {return m_antileecher && m_antileecheremcrypt;}
	static bool GeTAntiLeecheruserhash(){return m_antileecher && m_antileecheruserhash;}
	static bool GetAntiLeecherCommunity_Action() {return m_antileechercommunity_action;}
	static bool GetAntiLeecherGhost_Action() {return m_antileecherghost_action;}
	static bool GetAntiLeecherThief_Action() {return m_antileecherthief_action;}
	static void SetAntiLeecher(bool in)  {m_antileecher=in;}
	static void SetAntiLeecherName(bool in) {m_antileechername=in;}
	static void SetAntiGhost(bool in) {m_antighost=in;}
	static void SetAntiLeecherBadHello(bool in) {m_antileecherbadhello=in;}
	static void SetAntiLeecherSnafu(bool in) {m_antileechersnafu=in;}
	static void SetAntiLeecherMod(bool in) {m_antileechermod=in;}
	static void SetAntiLeecherThief(bool in) {m_antileecherthief=in;}
	static void SetAntiLeecherSpammer(bool in) {m_antileecherspammer=in;}
	static void SetAntiLeecherXSExploiter(bool in) {m_antileecherxsexploiter=in;}
	static void SetAntiLeecheremcrypt(bool in) {m_antileecheremcrypt=in;}
	static void SetAntiLeecheruserhash(bool in) {m_antileecheruserhash=in;}
	static void SetAntiLeecherCommunity_Action(bool in) {m_antileechercommunity_action=in;}
	static void SetAntiLeecherGhost_Action(bool in) {m_antileecherghost_action=in;}
	static void SetAntiLeecherThief_Action(bool in) {m_antileecherthief_action=in;}
	//X-Ray :: Fincan Hash Detection :: Start
	static bool m_antileecherFincan;
	static bool GetAntiLeecherFincan() {return m_antileecher && m_antileecherFincan;}
	static CString m_antileecherFincanURL;
	//X-Ray :: Fincan Hash Detection :: End
	//Xman end

	//Xman narrow font at transferwindow
	static bool m_bUseNarrowFont;
	static bool UseNarrowFont() {return m_bUseNarrowFont;}
	static void SetNarrowFont(bool in) {m_bUseNarrowFont=in;}
	//Xman end

	//Xman 1:3 Ratio
	static bool m_13ratio;
	static bool Is13Ratio() {return m_13ratio;}
	static void Set13Ratio(bool in) {m_13ratio=in;}
	//Xman end

	// ==> Superior Client Handling [Stulle] - Stulle
	/*
	//Xman always one release-slot
	static bool m_onerealeseslot;
	static bool UseReleasseSlot() {return m_onerealeseslot;}
	static void SetUseReleaseSlot(bool in) {m_onerealeseslot=in;}
	//Xman end
	*/
	// <== Superior Client Handling [Stulle] - Stulle

	//Xman advanced upload-priority
	static	bool m_AdvancedAutoPrio;
	static bool UseAdvancedAutoPtio() {return m_AdvancedAutoPrio;}
	static void SetAdvancedAutoPrio(bool in) {m_AdvancedAutoPrio=in;}
	//Xman end

	//Xman chunk chooser
	static uint8 m_chunkchooser;
	static uint8 GetChunkChooseMethod()	{return m_chunkchooser;}

	//Xman disable compression
	static bool m_bUseCompression;


	//Xman auto update IPFilter
	static bool	m_bautoupdateipfilter;
	static bool AutoUpdateIPFilter() {return m_bautoupdateipfilter;}
	static void SetAutoUpdateIPFilter(bool in) {m_bautoupdateipfilter=in;}
	static CString m_strautoupdateipfilter_url;
	static CString GetAutoUpdateIPFilter_URL() {return m_strautoupdateipfilter_url;}
	static void SetAutoUpdateIPFilter_URL(CString in) {m_strautoupdateipfilter_url=in;}
	static SYSTEMTIME		m_IPfilterVersion;
	// ==> Advanced Updates [MorphXT/Stulle] - Stulle
	/*
	static uint32 m_last_ipfilter_check;
	*/
	// <== Advanced Updates [MorphXT/Stulle] - Stulle
	//Xman end

	//Xman count block/success send
	static bool m_showblockratio;
	static bool ShowBlockRatio() {return m_showblockratio;}
	static void SetShowBlockRatio(bool in) {m_showblockratio=in;}

	static bool m_dropblockingsockets;
	static bool DropBlockingSockets() {return m_dropblockingsockets;}
	static void SetDropBlockingSockets(bool in) {m_dropblockingsockets=in;}
	//Xman end

	//Xman Funny-Nick (Stulle/Morph)
	static bool	m_bFunnyNick;
	static bool DisplayFunnyNick() 	{return m_bFunnyNick;}
	static void SetDisplayFunnyNick(bool in) {m_bFunnyNick=in;}
	//Xman end

	//Xman remove unused AICH-hashes
	static bool m_rememberAICH;
	static bool GetRememberAICH() {return m_rememberAICH;}
	static void SetRememberAICH(bool in) {m_rememberAICH=in;}
	//Xman end

	//Xman smooth-accurate-graph
	static bool usesmoothgraph;

	static bool retryconnectionattempts; //Xman 

	// Maella 
	static float	maxupload;                    // [FAF] -Allow Bandwidth Settings in <1KB Incremements-
	static float	maxdownload;
	static float	maxGraphDownloadRate;
	static float	maxGraphUploadRate;

	static uint8	zoomFactor;                   // -Graph: display zoom-

	static uint16	MTU;                          // -MTU Configuration-
	static bool		usedoublesendsize;
	static bool		retrieveMTUFromSocket; // netfinity: Maximum Segment Size (MSS - Vista only) //added by zz_fly

	static bool	NAFCFullControl;	          // -Network Adapter Feedback Control-
	static uint32 forceNAFCadapter;	
	static uint8	datarateSamples;              // -Accurate measure of bandwidth: eDonkey data + control, network adapter-

	static bool    enableMultiQueue;             // -One-queue-per-file- (idea bloodymad)
	//static bool    enableReleaseMultiQueue;
	// Maella end

	// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
	static float	GetMaxUpload()  {return maxupload;}
	static void	SetMaxUpload(float in)	{maxupload = in;}

	static float	GetMaxDownload() ; // rate limited
	static void	SetMaxDownload(float in) {maxdownload = in;}

	static float	GetMaxGraphUploadRate()  {return maxGraphUploadRate;}
	static void	SetMaxGraphUploadRate(float in) {maxGraphUploadRate=in;}

	static float	GetMaxGraphDownloadRate()  {return maxGraphDownloadRate;}
	static void	SetMaxGraphDownloadRate(float in) {maxGraphDownloadRate=in;}
	// Maella end

	// Maella -Graph: display zoom-
	static uint8	GetZoomFactor()  { return zoomFactor; }
	static void	SetZoomFactor(uint8 zoom) { zoomFactor = zoom; }
	// Maella end

	// Maella -MTU Configuration-
	static uint16	GetMTU()  { return MTU; }
	static void	SetMTU(uint16 MTUin) { MTU = MTUin; }
	// Maella end

	// Maella -Network Adapter Feedback Control-
	static bool	GetNAFCFullControl()  { return NAFCFullControl; }
	//static void	SetNAFCFullControl(bool flag) { NAFCFullControl = flag; }
	static void	SetNAFCFullControl(bool flag);
	static uint32 GetForcedNAFCAdapter() { return forceNAFCadapter;}
	// Maella end

	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	static uint8 GetDatarateSamples()  { return datarateSamples; }
	static void  SetDatarateSamples(uint8 samples) { datarateSamples = samples; }
	// Maella end

	// Maella -One-queue-per-file- (idea bloodymad)
	static bool GetEnableMultiQueue()  { return enableMultiQueue; }
	static void SetEnableMultiQueue(bool state) { enableMultiQueue = state; }
	//static bool GetEnableReleaseMultiQueue()  { return enableReleaseMultiQueue; }
	//static void SetEnableReleaseMultiQueue(bool state) { enableReleaseMultiQueue = state; }
	// Maella end

	// Mighty Knife: Static server handling
	static bool		m_bDontRemoveStaticServers;
	static	bool    GetDontRemoveStaticServers ()			  { return m_bDontRemoveStaticServers; }
	static	void	SetDontRemoveStaticServers (bool _b)	  { m_bDontRemoveStaticServers = _b; }
	// [end] Mighty Knife

	//Xman [MoNKi: -Downloaded History-]
	static bool		m_bHistoryShowShared;
	static bool		GetShowSharedInHistory()		{ return m_bHistoryShowShared; }
	static void		SetShowSharedInHistory(bool on)	{ m_bHistoryShowShared = on; }
	//Xman end

	//Xman GlobalMaxHarlimit for fairness
	static uint32	m_uMaxGlobalSources;
	static bool		m_bAcceptsourcelimit;

	//Xman show additional graph lines
	static bool		m_bShowAdditionalGraph;

	//Xman versions check
	static bool		updatenotifymod;
	static bool		UpdateNotifyMod()	{return updatenotifymod;}

	//Xman don't overwrite bak files if last sessions crashed
	static bool		m_this_session_aborted_in_an_unnormal_way;
	static bool		m_last_session_aborted_in_an_unnormal_way;
	static bool		eMuleChrashedLastSession()		{ return m_last_session_aborted_in_an_unnormal_way;}

	static bool		m_bShowCountryFlagInKad; //zz_fly :: show country flag in KAD
	static bool		m_bKnown2Buffer; //zz_fly :: known2 buffer
	//zz_fly :: known2 split :: start
	static bool		m_bKnown2Split;
	static bool		m_bKnown2Split_next;
	static bool		IsKnown2SplitEnabled()	{return m_bKnown2Split && GetRememberAICH() && IsRememberingDownloadedFiles();} //this feature only available when user want to remember unused AichHashSet.
	//zz_fly :: known2 split :: end
	static uint64		m_uAutoPreviewLimit; //zz_fly :: do not auto preview big archive

	//MORPH START - Added by WiZaRd, Fix broken HTTP downloads
	static	CString	m_strBrokenURLs;
	static	CString	GetBrokenURLs()						{return m_strBrokenURLs;}
	static	void	SetBrokenURLs(const CString& str)	{m_strBrokenURLs = str;}
	//MORPH END   - Added by WiZaRd, Fix broken HTTP downloads
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
	static	uint64	GetUpTotalClientData()				{return   GetCumUpData_EDONKEY()
																+ GetCumUpData_EDONKEYHYBRID()
																+ GetCumUpData_EMULE()
																+ GetCumUpData_MLDONKEY()
																+ GetCumUpData_AMULE()
																+ GetCumUpData_EMULECOMPAT()
																+ GetCumUpData_SHAREAZA();}
	static	uint64	GetCumUpData_EDONKEY()				{return (cumUpData_EDONKEY +		sesUpData_EDONKEY );}
	static	uint64	GetCumUpData_EDONKEYHYBRID()		{return (cumUpData_EDONKEYHYBRID +	sesUpData_EDONKEYHYBRID );}
	static	uint64	GetCumUpData_EMULE()				{return (cumUpData_EMULE +			sesUpData_EMULE );}
	static	uint64	GetCumUpData_MLDONKEY()				{return (cumUpData_MLDONKEY +		sesUpData_MLDONKEY );}
	static	uint64	GetCumUpData_AMULE()				{return (cumUpData_AMULE +			sesUpData_AMULE );}
	static	uint64	GetCumUpData_EMULECOMPAT()			{return (cumUpData_EMULECOMPAT +	sesUpData_EMULECOMPAT );}
	static	uint64	GetCumUpData_SHAREAZA()				{return (cumUpData_SHAREAZA +		sesUpData_SHAREAZA );}
	
	// Session client breakdown stats for sent bytes
	static	uint64	GetUpSessionClientData()			{return   sesUpData_EDONKEY 
																+ sesUpData_EDONKEYHYBRID 
																+ sesUpData_EMULE 
																+ sesUpData_MLDONKEY 
																+ sesUpData_AMULE
																+ sesUpData_EMULECOMPAT
																+ sesUpData_SHAREAZA;}
	static	uint64	GetUpData_EDONKEY()					{return sesUpData_EDONKEY;}
	static	uint64	GetUpData_EDONKEYHYBRID()			{return sesUpData_EDONKEYHYBRID;}
	static	uint64	GetUpData_EMULE()					{return sesUpData_EMULE;}
	static	uint64	GetUpData_MLDONKEY()				{return sesUpData_MLDONKEY;}
	static	uint64	GetUpData_AMULE()					{return sesUpData_AMULE;}
	static	uint64	GetUpData_EMULECOMPAT()				{return sesUpData_EMULECOMPAT;}
	static	uint64	GetUpData_SHAREAZA()				{return sesUpData_SHAREAZA;}

	// Cumulative port breakdown stats for sent bytes...
	static	uint64	GetUpTotalPortData()				{return   GetCumUpDataPort_4662() 
																+ GetCumUpDataPort_OTHER()
																+ GetCumUpDataPort_PeerCache();}
	static	uint64	GetCumUpDataPort_4662()				{return (cumUpDataPort_4662 +		sesUpDataPort_4662 );}
	static	uint64	GetCumUpDataPort_OTHER()			{return (cumUpDataPort_OTHER +		sesUpDataPort_OTHER );}
	static	uint64	GetCumUpDataPort_PeerCache()		{return (cumUpDataPort_PeerCache +	sesUpDataPort_PeerCache );}

	// Session port breakdown stats for sent bytes...
	static	uint64	GetUpSessionPortData()				{return   sesUpDataPort_4662 
																+ sesUpDataPort_OTHER
																+ sesUpDataPort_PeerCache;}
	static	uint64	GetUpDataPort_4662()				{return sesUpDataPort_4662;}
	static	uint64	GetUpDataPort_OTHER()				{return sesUpDataPort_OTHER;}
	static	uint64	GetUpDataPort_PeerCache()			{return sesUpDataPort_PeerCache;}

	// Cumulative DS breakdown stats for sent bytes...
	static	uint64	GetUpTotalDataFile()				{return (GetCumUpData_File() +		GetCumUpData_Partfile() );}
	static	uint64	GetCumUpData_File()					{return (cumUpData_File +			sesUpData_File );}
	static	uint64	GetCumUpData_Partfile()				{return (cumUpData_Partfile +		sesUpData_Partfile );}
	// Session DS breakdown stats for sent bytes...
	static	uint64	GetUpSessionDataFile()				{return (sesUpData_File +			sesUpData_Partfile );}
	static	uint64	GetUpData_File()					{return sesUpData_File;}
	static	uint64	GetUpData_Partfile()				{return sesUpData_Partfile;}

	// Cumulative client breakdown stats for received bytes
	static	uint64	GetDownTotalClientData()			{return   GetCumDownData_EDONKEY() 
																+ GetCumDownData_EDONKEYHYBRID() 
																+ GetCumDownData_EMULE() 
																+ GetCumDownData_MLDONKEY() 
																+ GetCumDownData_AMULE()
																+ GetCumDownData_EMULECOMPAT()
																+ GetCumDownData_SHAREAZA()
																+ GetCumDownData_URL();}
	static	uint64	GetCumDownData_EDONKEY()			{return (cumDownData_EDONKEY +			sesDownData_EDONKEY);}
	static	uint64	GetCumDownData_EDONKEYHYBRID()		{return (cumDownData_EDONKEYHYBRID +	sesDownData_EDONKEYHYBRID);}
	static	uint64	GetCumDownData_EMULE()				{return (cumDownData_EMULE +			sesDownData_EMULE);}
	static	uint64	GetCumDownData_MLDONKEY()			{return (cumDownData_MLDONKEY +			sesDownData_MLDONKEY);}
	static	uint64	GetCumDownData_AMULE()				{return (cumDownData_AMULE +			sesDownData_AMULE);}
	static	uint64	GetCumDownData_EMULECOMPAT()		{return (cumDownData_EMULECOMPAT +		sesDownData_EMULECOMPAT);}
	static	uint64	GetCumDownData_SHAREAZA()			{return (cumDownData_SHAREAZA +			sesDownData_SHAREAZA);}
	static	uint64	GetCumDownData_URL()				{return (cumDownData_URL +				sesDownData_URL);}
	
	// Session client breakdown stats for received bytes
	static	uint64	GetDownSessionClientData()			{return   sesDownData_EDONKEY 
																+ sesDownData_EDONKEYHYBRID 
																+ sesDownData_EMULE 
																+ sesDownData_MLDONKEY 
																+ sesDownData_AMULE
																+ sesDownData_EMULECOMPAT
																+ sesDownData_SHAREAZA
																+ sesDownData_URL;}
	static	uint64	GetDownData_EDONKEY()				{return sesDownData_EDONKEY;}
	static	uint64	GetDownData_EDONKEYHYBRID()			{return sesDownData_EDONKEYHYBRID;}
	static	uint64	GetDownData_EMULE()					{return sesDownData_EMULE;}
	static	uint64	GetDownData_MLDONKEY()				{return sesDownData_MLDONKEY;}
	static	uint64	GetDownData_AMULE()					{return sesDownData_AMULE;}
	static	uint64	GetDownData_EMULECOMPAT()			{return sesDownData_EMULECOMPAT;}
	static	uint64	GetDownData_SHAREAZA()				{return sesDownData_SHAREAZA;}
	static	uint64	GetDownData_URL()					{return sesDownData_URL;}

	// Cumulative port breakdown stats for received bytes...
	static	uint64	GetDownTotalPortData()				{return   GetCumDownDataPort_4662() 
																+ GetCumDownDataPort_OTHER()
																+ GetCumDownDataPort_PeerCache();}
	static	uint64	GetCumDownDataPort_4662()			{return cumDownDataPort_4662		+ sesDownDataPort_4662;}
	static	uint64	GetCumDownDataPort_OTHER()			{return cumDownDataPort_OTHER		+ sesDownDataPort_OTHER;}
	static	uint64	GetCumDownDataPort_PeerCache()		{return cumDownDataPort_PeerCache	+ sesDownDataPort_PeerCache;}

	// Session port breakdown stats for received bytes...
	static	uint64	GetDownSessionDataPort()			{return   sesDownDataPort_4662 
																+ sesDownDataPort_OTHER
																+ sesDownDataPort_PeerCache;}
	static	uint64	GetDownDataPort_4662()				{return sesDownDataPort_4662;}
	static	uint64	GetDownDataPort_OTHER()				{return sesDownDataPort_OTHER;}
	static	uint64	GetDownDataPort_PeerCache()			{return sesDownDataPort_PeerCache;}

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
	static	bool	GetAllowLocalHostIP()				{return m_bAllocLocalHostIP;}
	static	bool	IsOnlineSignatureEnabled()			{return onlineSig;}
	//Xman
	/*
	static	int		GetMaxGraphUploadRate(bool bEstimateIfUnlimited);
	static	int		GetMaxGraphDownloadRate()			{return maxGraphDownloadRate;}
	static	void	SetMaxGraphUploadRate(int in);
	static	void	SetMaxGraphDownloadRate(int in)		{maxGraphDownloadRate=(in)?in:96;}

	static	uint16	GetMaxDownload();
	static	uint64	GetMaxDownloadInBytesPerSec(bool dynamic = false);
	*/
	static	uint64	GetMaxDownloadInBytesPerSec();
	//Xman end
	static	UINT	GetMaxConnections()					{return maxconnections;}
	static	UINT	GetMaxHalfConnections()				{return maxhalfconnections;}
	static	UINT	GetMaxSourcePerFileDefault()		{return maxsourceperfile;}
	static	UINT	GetDeadServerRetries()				{return m_uDeadServerRetries;}
	static	DWORD	GetServerKeepAliveTimeout()			{return m_dwServerKeepAliveTimeout;}
	static	bool	GetConditionalTCPAccept()			{return m_bConditionalTCPAccept;}

	static	WORD	GetLanguageID();
	static	void	SetLanguageID(WORD lid);
	static	void	GetLanguages(CWordArray& aLanguageIDs);
	static	void	SetLanguage();
	static	bool	IsLanguageSupported(LANGID lidSelected, bool bUpdateBefore);
	static	CString GetLangDLLNameByID(LANGID lidSelected);
	static	void	InitThreadLocale();
	static	void	SetRtlLocale(LCID lcid);
	static	CString GetHtmlCharset();

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
	//MORPH START - Added by SiRoB, Splitting Bar [O²]
	static	UINT	GetSplitterbarPositionStat()		{return splitterbarPositionStat;}
	static	void	SetSplitterbarPositionStat(UINT pos) {splitterbarPositionStat=pos;}
	static	UINT	GetSplitterbarPositionStat_HL()		{return splitterbarPositionStat_HL;}
	static	void	SetSplitterbarPositionStat_HL(UINT pos) {splitterbarPositionStat_HL=pos;}
	static	UINT	GetSplitterbarPositionStat_HR()		{return splitterbarPositionStat_HR;}
	static	void	SetSplitterbarPositionStat_HR(UINT pos) {splitterbarPositionStat_HR=pos;}
	static	UINT	GetSplitterbarPositionFriend()		{return splitterbarPositionFriend;}
	static	void	SetSplitterbarPositionFriend(UINT pos) {splitterbarPositionFriend=pos;}
	static	UINT	GetSplitterbarPositionIRC()			{return splitterbarPositionIRC;}
	static	void	SetSplitterbarPositionIRC(UINT pos) {splitterbarPositionIRC=pos;}
	static	UINT	GetSplitterbarPositionShared()		{return splitterbarPositionShared;}
	static	void	SetSplitterbarPositionShared(UINT pos) {splitterbarPositionShared=pos;}
	//MORPH END   - Added by SiRoB, Splitting Bar [O²]
	// -khaos--+++> Changed datatype to avoid overflows
	static	UINT	GetStatsMax()						{return statsMax;}
	// <-----khaos-
	static	bool	UseFlatBar()						{return (depth3D==0);}
	static	int		GetStraightWindowStyles()			{return m_iStraightWindowStyles;}
	static  bool	GetUseSystemFontForMainControls()	{return m_bUseSystemFontForMainControls;}

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

	static	CString GetIRCNick()						{return m_strIRCNick;}
	static	void	SetIRCNick(LPCTSTR pszNick)			{m_strIRCNick = pszNick;}
	static	CString GetIRCServer()						{return m_strIRCServer;}
	static	bool	GetIRCAddTimeStamp()				{return m_bIRCAddTimeStamp;}
	static	bool	GetIRCUseChannelFilter()			{return m_bIRCUseChannelFilter;}
	static	CString GetIRCChannelFilter()				{return m_strIRCChannelFilter;}
	static	UINT	GetIRCChannelUserFilter()			{return m_uIRCChannelUserFilter;}
	static	bool	GetIRCUsePerform()					{return m_bIRCUsePerform;}
	static	CString GetIRCPerformString()				{return m_strIRCPerformString;}
	static	bool	GetIRCJoinHelpChannel()				{return m_bIRCJoinHelpChannel;}
	static	bool	GetIRCGetChannelsOnConnect()		{return m_bIRCGetChannelsOnConnect;}
	static	bool	GetIRCPlaySoundEvents()				{return m_bIRCPlaySoundEvents;}
	static	bool	GetIRCIgnoreMiscMessages()			{return m_bIRCIgnoreMiscMessages;}
	static	bool	GetIRCIgnoreJoinMessages()			{return m_bIRCIgnoreJoinMessages;}
	static	bool	GetIRCIgnorePartMessages()			{return m_bIRCIgnorePartMessages;}
	static	bool	GetIRCIgnoreQuitMessages()			{return m_bIRCIgnoreQuitMessages;}
	static	bool	GetIRCIgnoreEmuleAddFriendMsgs()	{return m_bIRCIgnoreEmuleAddFriendMsgs;}
	static	bool	GetIRCIgnoreEmuleSendLinkMsgs()		{return m_bIRCIgnoreEmuleSendLinkMsgs;}
	static	bool	GetIRCAllowEmuleAddFriend()			{return m_bIRCAllowEmuleAddFriend;}
	static	bool	GetIRCAcceptLinks()					{return m_bIRCAcceptLinks;}
	static	bool	GetIRCAcceptLinksFriendsOnly()		{return m_bIRCAcceptLinksFriendsOnly;}
	static	bool	GetIRCEnableSmileys()				{return m_bIRCEnableSmileys;}
	static	bool	GetMessageEnableSmileys()			{return m_bMessageEnableSmileys;}

	static	WORD	GetWindowsVersion();
	static  bool	IsRunningAeroGlassTheme();
	static	bool	GetStartMinimized()					{return startMinimized;}
	static	void	SetStartMinimized( bool instartMinimized) {startMinimized = instartMinimized;}
	static	bool	GetAutoStart()						{return m_bAutoStart;}
	static	void	SetAutoStart( bool val)				{m_bAutoStart = val;}

	static	bool	GetRestoreLastMainWndDlg()			{return m_bRestoreLastMainWndDlg;}
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
	// ==> Multiple Part Transfer [Stulle] - Mephisto
	/*
	static	bool	TransferFullChunks()				{return m_btransferfullchunks;}
	*/
	static	bool	TransferFullChunks()				{return m_uChunksMode != CHUNK_SCORE;}
	// <== Multiple Part Transfer [Stulle] - Mephisto
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
	// ==> CreditSystems [EastShare/ MorphXT] - Stulle
	/*
	static	bool	UseCreditSystem()					{return m_bCreditSystem;}
	static	void	SetCreditSystem(bool m_bInCreditSystem) {m_bCreditSystem = m_bInCreditSystem;}
	*/
	// <== CreditSystems [EastShare/ MorphXT] - Stulle

	static	const CString& GetTxtEditor()				{return m_strTxtEditor;}
	static	const CString& GetVideoPlayer()				{return m_strVideoPlayer;}
	static	const CString& GetVideoPlayerArgs()			{return m_strVideoPlayerArgs;}

	static	UINT	GetFileBufferSize()					{return m_iFileBufferSize;}
	static	UINT	GetFileBufferTimeLimit()			{return m_uFileBufferTimeLimit;}
	static	UINT	GetQueueSize()						{return m_iQueueSize;}
	static	int		GetCommitFiles()					{return m_iCommitFiles;}
	static	bool	GetShowCopyEd2kLinkCmd()			{return m_bShowCopyEd2kLinkCmd;}

	// Barry
	static	UINT	Get3DDepth()						{return depth3D;}
	static	bool	AutoTakeED2KLinks()					{return autotakeed2klinks;}
	static	bool	AddNewFilesPaused()					{return addnewfilespaused;}

	static	bool	TransferlistRemainSortStyle()		{return m_bTransflstRemain;}
	static	void	TransferlistRemainSortStyle(bool in){m_bTransflstRemain=in;}

	static	DWORD	GetStatsColor(int index)			{return m_adwStatsColors[index];}
	static	void	SetStatsColor(int index, DWORD value){m_adwStatsColors[index] = value;}
	static	int		GetNumStatsColors()					{return ARRSIZE(m_adwStatsColors);}
	static	void	GetAllStatsColors(int iCount, LPDWORD pdwColors);
	static	bool	SetAllStatsColors(int iCount, const DWORD* pdwColors);
	static	void	ResetStatsColor(int index);
	static	bool	HasCustomTaskIconColor()			{return bHasCustomTaskIconColor;}

	static	void	SetMaxConsPerFive(UINT in)			{MaxConperFive=in;}
	static	LPLOGFONT GetHyperTextLogFont()				{return &m_lfHyperText;}
	static	void	SetHyperTextFont(LPLOGFONT plf)		{m_lfHyperText = *plf;}
	static	LPLOGFONT GetLogFont()						{return &m_lfLogText;}
	static	void	SetLogFont(LPLOGFONT plf)			{m_lfLogText = *plf;}
	static	COLORREF GetLogErrorColor()					{return m_crLogError;}
	static	COLORREF GetLogWarningColor()				{return m_crLogWarning;}
	static	COLORREF GetLogSuccessColor()				{return m_crLogSuccess;}

	static	UINT	GetMaxConperFive()					{return MaxConperFive;}
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

	//Xman
	/*
	static	void	SetMaxUpload(UINT in);
	static	void	SetMaxDownload(UINT in);
	*/
	//Xman end

	static	WINDOWPLACEMENT GetEmuleWindowPlacement()	{return EmuleWindowPlacement;}
	static	void	SetWindowLayout(WINDOWPLACEMENT in) {EmuleWindowPlacement=in;}

	static	bool	GetAutoConnectToStaticServersOnly() {return m_bAutoConnectToStaticServersOnly;}
	static	UINT	GetUpdateDays()						{return versioncheckdays;}
	static	uint32	GetLastVC()							{return versioncheckLastAutomatic;}
	static	void	UpdateLastVC();
	//Xman versions check
	static	uint32	GetLastMVC()				{return mversioncheckLastAutomatic;}
	static	void	UpdateLastMVC();
	//Xman end
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
	static	int		AddCat(Category_Struct* cat)		{catMap.Add(cat); return catMap.GetCount()-1;}
	static	bool	MoveCat(UINT from, UINT to);
	static	void	RemoveCat(int index);
	static	int		GetCatCount()						{return catMap.GetCount();}
	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	/*
	static  bool	SetCatFilter(int index, int filter);
	static  int		GetCatFilter(int index);
	static	bool	GetCatFilterNeg(int index);
	static	void	SetCatFilterNeg(int index, bool val);
	*/
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	static	Category_Struct* GetCategory(int index)		{if (index>=0 && index<catMap.GetCount()) return catMap.GetAt(index); else return NULL;}
	static	const CString &GetCatPath(int index)		{return catMap.GetAt(index)->strIncomingPath;}
	static	DWORD	GetCatColor(int index, int nDefault = COLOR_BTNTEXT);

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
	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	/*
	static	uint16	GetWSPort()							{return m_nWebPort;}
	*/
	// may be required before init! 
	static	uint16	GetWSPort();
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle
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
	static  const CUIntArray& GetAllowedRemoteAccessIPs(){return m_aAllowedRemoteAccessIPs;}
	static	uint32	GetMaxWebUploadFileSizeMB()			{return m_iWebFileUploadSizeLimitMB;}

	static	void	SetMaxSourcesPerFile(UINT in)		{maxsourceperfile=in;}
	static	void	SetMaxConnections(UINT in)			{maxconnections =in;}
	static	void	SetMaxHalfConnections(UINT in)		{maxhalfconnections =in;}
	static	bool	IsSchedulerEnabled()				{return scheduler;}
	static	void	SetSchedulerEnabled(bool in)		{scheduler=in;}
	static	bool	GetDontCompressAvi()				{return dontcompressavi;}

	static	bool	MsgOnlyFriends()					{return msgonlyfriends;}
	static	bool	MsgOnlySecure()						{return msgsecure;}
	static	UINT	GetMsgSessionsMax()					{return maxmsgsessions;}
	static	bool	IsSecureIdentEnabled()				{return m_bUseSecureIdent;} // use clientcredits->CryptoAvailable() to check if crypting is really available and not this function
	static	bool	IsAdvSpamfilterEnabled()			{return m_bAdvancedSpamfilter;}
	static	bool	IsChatCaptchaEnabled()				{return IsAdvSpamfilterEnabled() && m_bUseChatCaptchas;}
	static	const CString& GetTemplate()				{return m_strTemplateFile;}
	static	void	SetTemplate(CString in)				{m_strTemplateFile = in;}
	static	bool	GetNetworkKademlia()				{return networkkademlia && udpport > 0;}
	static	void	SetNetworkKademlia(bool val);
	static	bool	GetNetworkED2K()					{return networked2k;}
	static	void	SetNetworkED2K(bool val)			{networked2k = val;}

	// mobileMule
	static	const CString& GetMMPass()					{return m_strMMPassword;}
	static	void	SetMMPass(CString strNewPass);
	static	bool	IsMMServerEnabled()					{return m_bMMEnabled;}
	static	void	SetMMIsEnabled(bool bEnable)		{m_bMMEnabled=bEnable;}
	static	uint16	GetMMPort()							{return m_nMMPort;}
	static	void	SetMMPort(uint16 uPort)				{m_nMMPort=uPort;}

	// deadlake PROXYSUPPORT
	static	const ProxySettings& GetProxySettings()		{return proxy;}
	static	void	SetProxySettings(const ProxySettings& proxysettings) {proxy = proxysettings;}

	static	bool	ShowCatTabInfos()					{return showCatTabInfos;}
	static	void	ShowCatTabInfos(bool in)			{showCatTabInfos=in;}

	static	bool	AutoFilenameCleanup()				{return autofilenamecleanup;}
	static	void	AutoFilenameCleanup(bool in)		{autofilenamecleanup=in;}
	static	void	SetFilenameCleanups(CString in)		{filenameCleanups=in;}

	static	bool	GetResumeSameCat()					{return resumeSameCat;}
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
	//Xman
	/*
	static	bool	IsDynUpEnabled();
	*/
	static	bool	IsDynUpEnabled()					{ return m_bDynUpEnabled; }
	//Xman end
	static	void	SetDynUpEnabled(bool newValue)		{m_bDynUpEnabled = newValue;}
	static	int		GetDynUpPingTolerance()				{return m_iDynUpPingTolerance;}
	static	int		GetDynUpGoingUpDivider()			{return m_iDynUpGoingUpDivider;}
	static	int		GetDynUpGoingDownDivider()			{return m_iDynUpGoingDownDivider;}
	static	int		GetDynUpNumberOfPings()				{return m_iDynUpNumberOfPings;}
    static  bool	IsDynUpUseMillisecondPingTolerance(){return m_bDynUpUseMillisecondPingTolerance;} // EastShare - Added by TAHO, USS limit
	static  int		GetDynUpPingToleranceMilliseconds() {return m_iDynUpPingToleranceMilliseconds;} // EastShare - Added by TAHO, USS limit
	static  void	SetDynUpPingToleranceMilliseconds(int in){m_iDynUpPingToleranceMilliseconds = in;}
	// ZZ:UploadSpeedSense <--

	//Xman
	/*
    static bool     GetA4AFSaveCpu()                    {return m_bA4AFSaveCpu;} // ZZ:DownloadManager
	*/
	//Xman end

    static bool     GetHighresTimer()                   {return m_bHighresTimer;}

	static	CString	GetHomepageBaseURL()				{return GetHomepageBaseURLForLevel(GetWebMirrorAlertLevel());}
	static	CString	GetVersionCheckBaseURL();					
	static	void	SetWebMirrorAlertLevel(uint8 newValue){m_nWebMirrorAlertLevel = newValue;}
	static	bool	IsDefaultNick(const CString strCheck);
	static	UINT	GetWebMirrorAlertLevel();
	static	bool	UseSimpleTimeRemainingComputation()	{return m_bUseOldTimeRemaining;}

	static	bool	IsRunAsUserEnabled();
	static	bool	IsPreferingRestrictedOverUser()		{return m_bPreferRestrictedOverUser;}

	// PeerCache
	static	bool	IsPeerCacheDownloadEnabled()		{return (m_bPeerCacheEnabled && !IsClientCryptLayerRequested());}
	static	uint32	GetPeerCacheLastSearch()			{return m_uPeerCacheLastSearch;}
	static	bool	WasPeerCacheFound()					{return m_bPeerCacheWasFound;}
	static	void	SetPeerCacheLastSearch(uint32 dwLastSearch) {m_uPeerCacheLastSearch = dwLastSearch;}
	static	void	SetPeerCacheWasFound(bool bFound)	{m_bPeerCacheWasFound = bFound;}
	static	uint16	GetPeerCachePort()					{return m_nPeerCachePort;}
	static	void	SetPeerCachePort(uint16 nPort)		{m_nPeerCachePort = nPort;}
	static	bool	GetPeerCacheShow()					{return m_bPeerCacheShow;}

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
	static	bool	GetUseDebugDevice()					{return m_bUseDebugDevice;}
	static	int		GetDebugServerTCPLevel()			{return m_iDebugServerTCPLevel;}
	static	int		GetDebugServerUDPLevel() 			{return m_iDebugServerUDPLevel;}
	static	int		GetDebugServerSourcesLevel()		{return m_iDebugServerSourcesLevel;}
	static	int		GetDebugServerSearchesLevel()		{return m_iDebugServerSearchesLevel;}
	static	int		GetDebugClientTCPLevel()			{return m_iDebugClientTCPLevel;}
	static	int		GetDebugClientUDPLevel()			{return m_iDebugClientUDPLevel;}
	static	int		GetDebugClientKadUDPLevel()			{return m_iDebugClientKadUDPLevel;}
	static	int		GetDebugSearchResultDetailLevel()	{return m_iDebugSearchResultDetailLevel;}
	static	int		GetVerboseLogPriority()				{return	m_byLogLevel;}

	// Firewall settings
	static  bool	IsOpenPortsOnStartupEnabled()		{return m_bOpenPortsOnStartUp;}
	
	//AICH Hash
	static	bool	IsTrustingEveryHash()				{return m_bTrustEveryHash;} // this is a debug option

	static	bool	IsRememberingDownloadedFiles()		{return m_bRememberDownloadedFiles;}
	static	bool	IsRememberingCancelledFiles()		{return m_bRememberCancelledFiles;}
	static  bool	DoPartiallyPurgeOldKnownFiles()		{return m_bPartiallyPurgeOldKnownFiles;}
	static	void	SetRememberDownloadedFiles(bool nv)	{m_bRememberDownloadedFiles = nv;}
	static	void	SetRememberCancelledFiles(bool nv)	{m_bRememberCancelledFiles = nv;}
	// mail notifier
	static	bool	IsNotifierSendMailEnabled()			{return m_bNotifierSendMail;}
	static	CString	GetNotifierMailServer()				{return m_strNotifierMailServer;}
	static	CString	GetNotifierMailSender()				{return m_strNotifierMailSender;}
	static	CString	GetNotifierMailReceiver()			{return m_strNotifierMailReceiver;}

	static	void	SetNotifierSendMail(bool nv)		{m_bNotifierSendMail = nv;}
	static  bool	DoFlashOnNewMessage()				{return m_bIconflashOnNewMessage;}
	static  void	IniCopy(CString si, CString di);

	static	void	EstimateMaxUploadCap(uint32 nCurrentUpload);
	static  bool	GetAllocCompleteMode()				{return m_bAllocFull;}
	static  void	SetAllocCompleteMode(bool in)		{m_bAllocFull=in;}

	// encryption
	static bool		IsClientCryptLayerSupported()		{return m_bCryptLayerSupported;}
	static bool		IsClientCryptLayerRequested()		{return IsClientCryptLayerSupported() && m_bCryptLayerRequested;}
	static bool		IsClientCryptLayerRequired()		{return IsClientCryptLayerRequested() && m_bCryptLayerRequired;}
	static bool		IsClientCryptLayerRequiredStrict()	{return false;} // not even incoming test connections will be answered
	static bool		IsServerCryptLayerUDPEnabled()		{return IsClientCryptLayerSupported();}
	static bool		IsServerCryptLayerTCPRequested()	{return IsClientCryptLayerRequested();}
	static uint32	GetKadUDPKey()						{return m_dwKadUDPKey;}
	static uint8	GetCryptTCPPaddingLength()			{return m_byCryptTCPPaddingLength;}
	static void		SetCryptTCPPaddingLength(int in)	{m_byCryptTCPPaddingLength = (uint8)((in>=10 && in<=254) ? in : 128);} //zz_fly :: hardlimit on CryptTCPPaddingLength

	// ==> UPnP support [MoNKi] - leuk_he
	/*
	// UPnP
	static bool		GetSkipWANIPSetup()					{return m_bSkipWANIPSetup;}
	static bool		GetSkipWANPPPSetup()				{return m_bSkipWANPPPSetup;}
	static bool		IsUPnPEnabled()						{return m_bEnableUPnP;}
	static void		SetSkipWANIPSetup(bool nv)			{m_bSkipWANIPSetup = nv;}
	static void		SetSkipWANPPPSetup(bool nv)			{m_bSkipWANPPPSetup = nv;}
	static bool		CloseUPnPOnExit()					{return m_bCloseUPnPOnExit;}
	static bool		IsWinServUPnPImplDisabled()			{return m_bIsWinServImplDisabled;}
	static bool		IsMinilibUPnPImplDisabled()			{return m_bIsMinilibImplDisabled;}
	static int		GetLastWorkingUPnPImpl()			{return m_nLastWorkingImpl;}
	static void		SetLastWorkingUPnPImpl(int val)		{m_nLastWorkingImpl = val;}
	*/
	// <== UPnP support [MoNKi] - leuk_he

	// Spamfilter
	static bool		IsSearchSpamFilterEnabled()			{return m_bEnableSearchResultFilter;}
	
	static bool		IsStoringSearchesEnabled()			{return m_bStoreSearches;}
	static bool		GetPreventStandby()					{return m_bPreventStandby;}
	static uint16	GetRandomTCPPort();
	static uint16	GetRandomUDPPort();

	// ==> Global Source Limit [Max/Stulle] - Stulle
	static UINT		GetGlobalHL()				{return m_uGlobalHL;} 
	static bool		IsUseGlobalHL()				{return m_bGlobalHL;} 
	static bool		GetGlobalHlAll()			{return m_bGlobalHlAll;}
	// <== Global Source Limit [Max/Stulle] - Stulle

	// ==> push small files [sivka] - Stulle
	static bool		GetEnablePushSmallFile()	{return enablePushSmallFile;}
	static uint32	GetPushSmallFileSize()		{return m_iPushSmallFiles;}
	static uint16	GetPushSmallFileBoost()		{return m_iPushSmallBoost;}
	// <== push small files [sivka] - Stulle
    static bool		GetEnablePushRareFile()		{return enablePushRareFile;} // push rare file - Stulle

	static	bool	ShowSrcOnTitle()		{ return showSrcInTitle;} // Show sources on title - Stulle
	static	bool	ShowOverheadOnTitle()	{ return showOverheadInTitle;} // show overhead on title - Stulle
	static	bool	GetShowGlobalHL()		{ return ShowGlobalHL; } // show global HL - Stulle
	static	bool	GetShowFileHLconst()	{ return ShowFileHLconst; } // show HL per file constantly - Stulle
	static	bool	GetShowMSN7()			{ return m_bShowInMSN7;} // Show in MSN7 [TPT] - Stulle
	static	bool	ShowClientQueueProgressBar()	{ return m_bClientQueueProgressBar;} // Client queue progress bar [Commander] - Stulle
	static	bool	GetShowClientPercentage()	{ return m_bShowClientPercentage;}  // Show Client Percentage optional [Stulle] - Stulle
	// ==> CPU/MEM usage [$ick$/Stulle] - Max
	static	bool	GetSysInfo()			{ return m_bSysInfo; }
	static	bool	GetSysInfoGlobal()		{ return m_bSysInfoGlobal; }
	// <== CPU/MEM usage [$ick$/Stulle] - Max
	static	bool	GetShowSpeedMeter()		{ return m_bShowSpeedMeter; } // High resolution speedmeter on toolbar [eFMod/Stulle] - Myth88
	// ==> Static Tray Icon [MorphXT] - MyTh88
	static bool		GetStaticIcon()				{return m_bStaticIcon;}
	// <== Static Tray Icon [MorphXT] - MyTh88

	static	uint8	GetCreditSystem()		{return creditSystemMode;} // CreditSystems [EastShare/ MorphXT] - Stulle

	static	bool	SaveUploadQueueWaitTime()			{return m_bSaveUploadQueueWaitTime;} // SUQWT [Moonlight/EastShare/ MorphXT] - Stulle

	// ==> File Settings [sivka/Stulle] - Stulle
	static uint16	GetMaxSourcesPerFileTemp(){return m_MaxSourcesPerFileTemp;}
	static bool		GetEnableAutoDropNNSTemp(){return m_EnableAutoDropNNSTemp;}
	static DWORD	GetAutoNNS_TimerTemp(){return m_AutoNNS_TimerTemp;}
	static uint16	GetMaxRemoveNNSLimitTemp(){return m_MaxRemoveNNSLimitTemp;}
	static bool		GetEnableAutoDropFQSTemp(){return m_EnableAutoDropFQSTemp;}
	static DWORD	GetAutoFQS_TimerTemp(){return m_AutoFQS_TimerTemp;}
	static uint16	GetMaxRemoveFQSLimitTemp(){return m_MaxRemoveFQSLimitTemp;}
	static bool		GetEnableAutoDropQRSTemp(){return m_EnableAutoDropQRSTemp;}
	static DWORD	GetAutoHQRS_TimerTemp(){return m_AutoHQRS_TimerTemp;}
	static uint16	GetMaxRemoveQRSTemp(){return m_MaxRemoveQRSTemp;}
	static uint16	GetMaxRemoveQRSLimitTemp(){return m_MaxRemoveQRSLimitTemp;}
	static bool		GetHQRXmanTemp(){return m_bHQRXmanTemp;}
	static bool		GetGlobalHlTemp(){return m_bGlobalHlTemp;} // Global Source Limit (customize for files) - Stulle
	static void		SetMaxSourcesPerFileTemp(uint16 in){m_MaxSourcesPerFileTemp=in;}
	static void		SetEnableAutoDropNNSTemp(bool in){m_EnableAutoDropNNSTemp=in;}
	static void		SetAutoNNS_TimerTemp(DWORD in){m_AutoNNS_TimerTemp=in;}
	static void		SetMaxRemoveNNSLimitTemp(uint16 in){m_MaxRemoveNNSLimitTemp=in;}
	static void		SetEnableAutoDropFQSTemp(bool in){m_EnableAutoDropFQSTemp=in;}
	static void		SetAutoFQS_TimerTemp(DWORD in){m_AutoFQS_TimerTemp=in;}
	static void		SetMaxRemoveFQSLimitTemp(uint16 in){m_MaxRemoveFQSLimitTemp=in;}
	static void		SetEnableAutoDropQRSTemp(bool in){m_EnableAutoDropQRSTemp=in;}
	static void		SetAutoHQRS_TimerTemp(DWORD in){m_AutoHQRS_TimerTemp=in;}
	static void		SetMaxRemoveQRSTemp(uint16 in){m_MaxRemoveQRSTemp=in;}
	static void		SetMaxRemoveQRSLimitTemp(uint16 in){m_MaxRemoveQRSLimitTemp=in;}
	static void		SetHQRXmanTemp(bool in){m_bHQRXmanTemp=in;}
	static void		SetGlobalHlTemp(bool in){m_bGlobalHlTemp=in;} // Global Source Limit (customize for files) - Stulle
	static void		SetTakeOverFileSettings(bool in) {m_TakeOverFileSettings=in;}
	static bool		GetEnableAutoDropNNSDefault(){return m_EnableAutoDropNNSDefault;}
	static DWORD	GetAutoNNS_TimerDefault(){return m_AutoNNS_TimerDefault;}
	static uint16	GetMaxRemoveNNSLimitDefault(){return m_MaxRemoveNNSLimitDefault;}
	static bool		GetEnableAutoDropFQSDefault(){return m_EnableAutoDropFQSDefault;}
	static DWORD	GetAutoFQS_TimerDefault(){return m_AutoFQS_TimerDefault;}
	static uint16	GetMaxRemoveFQSLimitDefault(){return m_MaxRemoveFQSLimitDefault;}
	static bool		GetEnableAutoDropQRSDefault(){return m_EnableAutoDropQRSDefault;}
	static DWORD	GetAutoHQRS_TimerDefault(){return m_AutoHQRS_TimerDefault;}
	static uint16	GetMaxRemoveQRSDefault(){return m_MaxRemoveQRSDefault;}
	static uint16	GetMaxRemoveQRSLimitDefault(){return m_MaxRemoveQRSLimitDefault;}
	static bool		GetHQRXmanDefault(){return m_bHQRXmanDefault;}
	static bool		GetGlobalHlDefault(){return m_bGlobalHlDefault;} // Global Source Limit (customize for files) - Stulle
	static bool		GetMaxSourcesPerFileTakeOver(){return m_MaxSourcesPerFileTakeOver;}
	static bool		GetEnableAutoDropNNSTakeOver(){return m_EnableAutoDropNNSTakeOver;}
	static bool		GetAutoNNS_TimerTakeOver(){return m_AutoNNS_TimerTakeOver;}
	static bool		GetMaxRemoveNNSLimitTakeOver(){return m_MaxRemoveNNSLimitTakeOver;}
	static bool		GetEnableAutoDropFQSTakeOver(){return m_EnableAutoDropFQSTakeOver;}
	static bool		GetAutoFQS_TimerTakeOver(){return m_AutoFQS_TimerTakeOver;}
	static bool		GetMaxRemoveFQSLimitTakeOver(){return m_MaxRemoveFQSLimitTakeOver;}
	static bool		GetEnableAutoDropQRSTakeOver(){return m_EnableAutoDropQRSTakeOver;}
	static bool		GetAutoHQRS_TimerTakeOver(){return m_AutoHQRS_TimerTakeOver;}
	static bool		GetMaxRemoveQRSTakeOver(){return m_MaxRemoveQRSTakeOver;}
	static bool		GetMaxRemoveQRSLimitTakeOver(){return m_MaxRemoveQRSLimitTakeOver;}
	static bool		GetHQRXmanTakeOver(){return m_bHQRXmanTakeOver;}
	static bool		GetGlobalHlTakeOver(){return m_bGlobalHlTakeOver;} // Global Source Limit (customize for files) - Stulle
	static bool		GetTakeOverFileSettings() {return m_TakeOverFileSettings;}
	// <== File Settings [sivka/Stulle] - Stulle

	// ==> Source Graph - Stulle
	static	bool GetSrcGraph()			{ return m_bSrcGraph; }
	static uint16  GetStatsHLMin()		{ return m_iStatsHLMin; }
	static uint16  GetStatsHLMax()		{ return m_iStatsHLMax; }
	static uint16  GetStatsHLDif()		{ return m_iStatsHLDif; }
	// <== Source Graph - Stulle

	// ==> FunnyNick [SiRoB/Stulle] - Stulle
	static	uint8	GetFnTag()			{return FnTagMode;}
	static	CString GetFnCustomTag ()	{ return m_sFnCustomTag; }
	static	bool	GetFnTagAtEnd()		{return m_bFnTagAtEnd;}
	// <== FunnyNick [SiRoB/Stulle] - Stulle

	static	bool	GetACC()			{ return m_bACC; } // ACC [Max/WiZaRd] - Max

	// ==> ScarAngel Version Check - Stulle
	static	uint32	GetLastSVC()				{return m_uScarVerCheckLastAutomatic;}
	static	void	UpdateLastSVC();
	// <== ScarAngel Version Check - Stulle

	// ==> Quick start [TPT] - Max
	static	bool	GetQuickStart()						{return m_bQuickStart;}
	static	void	SetMaxCon(int in)					{maxconnections=in;} 
	static	UINT	GetMaxCon()							{ return maxconnections;}
	static	uint16	GetQuickStartMaxTime()				{ return m_iQuickStartMaxTime; }
	static	UINT	GetQuickStartMaxConn()				{ return m_iQuickStartMaxConn; }
	static	uint16	GetQuickStartMaxConnPerFive()		{ return m_iQuickStartMaxConnPerFive; }
	static	UINT	GetQuickStartMaxConnBack()			{ return m_iQuickStartMaxConnBack; }
	static	uint16	GetQuickStartMaxConnPerFiveBack()	{ return m_iQuickStartMaxConnPerFiveBack; }
	static	bool	GetQuickStartAfterIPChange()		{ return m_bQuickStartAfterIPChange;}
	// <== Quick start [TPT] - Max

	// ==> TBH: Backup [TBH/EastShare/MorphXT] - Stulle
	static	bool	GetAutoBackup()				{ return m_bAutoBackup; }
	static	bool	GetAutoBackup2()			{ return m_bAutoBackup2; }
	// <== TBH: Backup [TBH/EastShare/MorphXT] - Stulle

	// ==> TBH: minimule - Max
	static	int		GetSpeedMeterMin()		{return speedmetermin;}
	static	int		GetSpeedMeterMax()		{return speedmetermax;}
	static	void	SetSpeedMeterMin(int in)	{speedmetermin = in;}
	static	void	SetSpeedMeterMax(int in)    {speedmetermax = in;}
	static	bool	IsMiniMuleEnabled() {return m_bMiniMule;}
	static	void	SetMiniMuleEnabled(bool in) {m_bMiniMule = in;}
	static	uint32	GetMiniMuleUpdate()	{return m_iMiniMuleUpdate;}
	static	void	SetMiniMuleUpdate(uint32 in)  {m_iMiniMuleUpdate = in;}
	static	void	SetMiniMuleLives(bool in) {m_bMiniMuleLives = in;}
	static	bool	GetMiniMuleLives()	{return m_bMiniMuleLives;}
	static	void	SetMiniMuleTransparency(uint8 in) {m_iMiniMuleTransparency = in;}
	static	uint8	GetMiniMuleTransparency() {return m_iMiniMuleTransparency;}
	static	bool	GetMMCompl()			{ return m_bMMCompl; }
	static	bool	GetMMOpen()				{ return m_bMMOpen; }
	// <== TBH: minimule - Max

	// ==> Simple cleanup [MorphXT] - Stulle
	static	void	SetSimpleCleanupOptions (int _i)	      { m_SimpleCleanupOptions = _i; }
	static	int 	GetSimpleCleanupOptions ()			      { return m_SimpleCleanupOptions; }
	static	void	SetSimpleCleanupSearch (CString _s)	      { m_SimpleCleanupSearch = _s; }
	static	CString	GetSimpleCleanupSearch ()			      { return m_SimpleCleanupSearch; }
	static	void	SetSimpleCleanupReplace (CString _s)	  { m_SimpleCleanupReplace = _s; }
	static	CString	GetSimpleCleanupReplace ()				  { return m_SimpleCleanupReplace; }
	static	void	SetSimpleCleanupSearchChars (CString _s)  { m_SimpleCleanupSearchChars = _s; }
	static	CString	GetSimpleCleanupSearchChars ()			  { return m_SimpleCleanupSearchChars; }
	static	void	SetSimpleCleanupReplaceChars (CString _s) { m_SimpleCleanupReplaceChars = _s; }
	static	CString	GetSimpleCleanupReplaceChars ()			  { return m_SimpleCleanupReplaceChars; }
	// <== Simple cleanup [MorphXT] - Stulle

	static  bool	UseStartupSound()			{return startupsound;} // Startupsound [Commander] - mav744

	static	uint8	GetCompressLevel()			{return m_uCompressLevel;} // Adjust Compress Level [Stulle] - Stulle

	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	static	bool	ShowValidSrcsOnly()		{ return m_bValidSrcsOnly; }
	static	bool	ShowCatNameInDownList()	{ return m_bShowCatNames; }
	static	bool	SelectCatForNewDL()		{ return m_bSelCatOnAdd; }
	static	bool	UseActiveCatForLinks()	{ return m_bActiveCatDefault; }
	static	bool	AutoSetResumeOrder()	{ return m_bAutoSetResumeOrder; }
	static	bool	SmallFileDLPush()		{ return m_bSmallFileDLPush; }
	static	uint8	StartDLInEmptyCats()	{ return m_iStartDLInEmptyCats; } // 0 = disabled, otherwise num to resume
	static	bool	UseAutoCat()			{ return m_bUseAutoCat; }
	static	uint8	GetDlMode()				{ return dlMode;}
	static	bool	UseAddRemoveInc()		{ return m_bAddRemovedInc; }
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

	static	bool	GetSpreadbarSetStatus()	{return m_bSpreadbarSetStatus;} // Spread bars [Slugfiller/MorphXT] - Stulle
	// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	static	int		GetHideOvershares()		{return hideOS;}
	static	bool	IsSelectiveShareEnabled()	{return selectiveShare;}
	static	int		GetShareOnlyTheNeed()	{return ShareOnlyTheNeed;}
	// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle

	// ==> PowerShare [ZZ/MorphXT] - Stulle
	static	int		GetPowerShareMode()	{return m_iPowershareMode;}
	static	int		GetPowerShareLimit() {return PowerShareLimit;}
	// <== PowerShare [ZZ/MorphXT] - Stulle

	static	uint8	GetReleaseBonus()			{return m_uReleaseBonus;} // Release Bonus [sivka] - Stulle
	static	bool	GetReleaseScoreAssurance()	{return m_bReleaseScoreAssurance;} // Release Score Assurance [Stulle] - Stulle

	static int	PsAmountLimit; // Limit PS by amount of data uploaded [Stulle] - Stulle

	// ==> Design Settings [eWombat/Stulle] - Stulle
	static void		InitStyles();
	static bool		GetStyle(int nMaster, int nStyle, StylesStruct *style=NULL);
	static bool		SetStyle(int nMaster, int nStyle, StylesStruct *style=NULL);
	static DWORD	GetStyleFlags(int nMaster, int nStyle);
	static void		SetStyleFlags(int nMaster, int nStyle, DWORD dwNew);
	static COLORREF	GetStyleFontColor(int nMaster, int nStyle);
	static void		SetStyleFontColor(int nMaster, int nStyle, COLORREF crNew);
	static COLORREF	GetStyleBackColor(int nMaster, int nStyle);
	static void		SetStyleBackColor(int nMaster, int nStyle, COLORREF crNew);
	static short	GetStyleOnOff(int nMaster, int nStyle);
	static void		SetStyleOnOff(int nMaster, int nStyle, short sNew);
	static void		SaveStylePrefs(CIni &ini);
	static void		LoadStylePrefs(CIni &ini);
	static void		LoadStylePrefsV2(CIni &ini);
	// <== Design Settings [eWombat/Stulle] - Stulle

	// ==> Enforce Ratio [Stulle] - Stulle
	static	bool	GetEnforceRatio()	{ return m_bEnforceRatio; }
	static	uint8	GetRatioValue()		{ return m_uRatioValue; }
	// <== Enforce Ratio [Stulle] - Stulle

	// ==> Improved ICS-Firewall support [MoNKi]-Max
	static	bool	GetICFSupport() { return m_bICFSupport; }
	static	void	SetICFSupport(bool on) { m_bICFSupport = on; }
	static	bool	GetICFSupportFirstTime() { return m_bICFSupportFirstTime; }
	static	void	SetICFSupportFirstTime(bool on) { m_bICFSupportFirstTime = on; }
	static	bool	GetICFSupportServerUDP() { return m_bICFSupportServerUDP; }
	static	void	SetICFSupportServerUDP(bool on) { m_bICFSupportServerUDP = on; }
	// <== Improved ICS-Firewall support [MoNKi]-Max

	// ==> Invisible Mode [TPT/MoNKi] - Stulle
    static	bool GetInvisibleMode() { return m_bInvisibleMode; }
	static	UINT GetInvisibleModeHKKeyModifier() { return m_iInvisibleModeHotKeyModifier; }
	static	char GetInvisibleModeHKKey() { return m_cInvisibleModeHotKey; }
	static	void SetInvisibleMode(bool on, UINT keymodifier, char key);
	static	bool GetInvisibleModeStart() { return m_bInvisibleModeStart; }
	// <== Invisible Mode [TPT/MoNKi] - Stulle

	// ==> UPnP support [MoNKi] - leuk_he  upnp bindaddr
	static DWORD	 GetUpnpBindAddr()				{return m_dwUpnpBindAddr; }
	static void      SetUpnpBindAddr(DWORD bindip); 
	static void      SetUpnpBindDhcp(bool BindAddrIsDhcp) {m_bBindAddrIsDhcp=BindAddrIsDhcp;};
	static bool      GetUpnpBindDhcp() {return m_bBindAddrIsDhcp;};
	static	bool	IsUPnPEnabled()						{ return m_bUPnPNat; }
	// Note: setting upnp to disbale/enable is in theApp.m_UPnP_IGDControlPoint->SetUPnPNat to actually stop and start upnp. 
	static	bool	GetUPnPNatWeb()						{ return m_bUPnPNatWeb; }
	static	void	SetUPnPNatWeb(bool on)				{ m_bUPnPNatWeb = on; }
	static	void	SetUPnPVerboseLog(bool on)			{ m_bUPnPVerboseLog = on; }
	static	bool	GetUPnPVerboseLog()					{ return m_bUPnPVerboseLog; }
	static	void	SetUPnPPort(uint16 port)			{ m_iUPnPPort = port; }
	static	uint16	GetUPnPPort()						{ return m_iUPnPPort; }
	static	void	SetUPnPClearOnClose(bool on)		{ m_bUPnPClearOnClose = on; }
	static	bool	GetUPnPClearOnClose()				{ return m_bUPnPClearOnClose; }
	static	bool	SetUPnPLimitToFirstConnection(bool on)	{ m_bUPnPLimitToFirstConnection = on; }
	static	bool	GetUPnPLimitToFirstConnection()		{ return m_bUPnPLimitToFirstConnection; }
	static	int  	GetUpnpDetect()					{ return m_iDetectuPnP; } //leuk_he autodetect upnp in wizard
	static void     SetUpnpDetect(int on);
	#define UPNP_DO_AUTODETECT 2
	#define UPNP_DETECTED 0
	#define UPNP_NOT_DETECTED -1 
	#define UPNP_NO_DETECTEDTION -2 
	#define UPNP_NOT_NEEDED -10
	// <== UPnP support [MoNKi] - leuk_he

	// ==> Random Ports [MoNKi] - Stulle
	static	uint16	GetPort(bool newPort = false, bool original = false, bool reset = false);
	static	uint16	GetUDPPort(bool newPort = false, bool original = false, bool reset = false);
	static	bool	GetUseRandomPorts()				{ return m_bRndPorts; }
	static	void	SetUseRandomPorts(bool on)		{ m_bRndPorts = on; }
	static	uint16	GetMinRandomPort()				{ return m_iMinRndPort; }
	static	void	SetMinRandomPort(uint16 min)	{ m_iMinRndPort = min; }
	static	uint16	GetMaxRandomPort()				{ return m_iMaxRndPort; }
	static	void	SetMaxRandomPort(uint16 max)	{ m_iMaxRndPort = max; }
	static	bool	GetRandomPortsResetOnRestart()	{ return m_bRndPortsResetOnRestart; }
	static	void	SetRandomPortsResetOnRestart(bool on)	{ m_bRndPortsResetOnRestart = on; }
	static	uint16	GetRandomPortsSafeResetOnRestartTime(){ return m_iRndPortsSafeResetOnRestartTime; }
	static	void	SetRandomPortsSafeResetOnRestartTime(uint16 time){ m_iRndPortsSafeResetOnRestartTime = time; }
	// <== Random Ports [MoNKi] - Stulle

	// ==> Automatic shared files updater [MoNKi] - Stulle
	static	bool	GetDirectoryWatcher()				{ return m_bDirectoryWatcher; }
	static	void	SetDirectoryWatcher(bool on)		{ m_bDirectoryWatcher = on; }
	static	bool	GetSingleSharedDirWatcher()			{ return m_bSingleSharedDirWatcher; }
	static	void	SetSingleSharedDirWatcher(bool in)	{ m_bSingleSharedDirWatcher = in; }
	static	uint32	GetTimeBetweenReloads()				{ return m_uTimeBetweenReloads; }
	static	void	SetTimeBetweenReloads(uint32 in)	{ m_uTimeBetweenReloads = in; }
	// <== Automatic shared files updater [MoNKi] - Stulle

	// ==> Control download priority [tommy_gun/iONiX] - MyTh88
	static	uint8	m_uiBowlfishMode;
	static	uint8	GetBowlfishMode()					{return m_uiBowlfishMode;}

	static	bool	GetBowlfishPrioPercent()			{return (m_uiBowlfishMode == 1);}
	static	void	SetBowlfishPrioPercent()			{m_uiBowlfishMode = 1;}

	static	bool	GetBowlfishPrioSize()				{return (m_uiBowlfishMode == 2);}
	static	void	SetBowlfishPrioSize()				{m_uiBowlfishMode = 2;}

	static	uint8	m_nBowlfishPrioPercentValue;
	static	uint8	GetBowlfishPrioPercentValue()		{return m_nBowlfishPrioPercentValue;}
	static	void	SetBowlfishPrioPercentValue(uint8 value)	{m_nBowlfishPrioPercentValue = value;}

	static	uint16	m_nBowlfishPrioSizeValue;
	static	uint16	GetBowlfishPrioSizeValue()			{return m_nBowlfishPrioSizeValue;}
	static	void	SetBowlfishPrioSizeValue(uint16 value)	{m_nBowlfishPrioSizeValue = value;}

	static	uint8	m_nBowlfishPrioNewValue;
	static	uint8	GetBowlfishPrioNewValue()			{return m_nBowlfishPrioNewValue;}
	static	void	SetBowlfishPrioNewValue(uint8 value)	{m_nBowlfishPrioNewValue = value;}
	// <== Control download priority [tommy_gun/iONiX] - MyTh88

	// ==> Anti Uploader Ban [Stulle] - Stulle
	static	uint16	GetAntiUploaderBanLimit()	{return m_uAntiUploaderBanLimit;}
	static	uint8	GetAntiUploaderBanCase()	{return AntiUploaderBanCaseMode;}
	// <== Anti Uploader Ban [Stulle] - Stulle

	// ==> Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	static	bool	IsEmuMLDonkey()			    {return m_bEmuMLDonkey;}
	static	bool	IsEmueDonkey()			    {return m_bEmueDonkey;}
	static	bool	IsEmueDonkeyHybrid()		{return m_bEmueDonkeyHybrid;}
	static	bool	IsEmuShareaza()				{return m_bEmuShareaza;}
	static  bool    IsEmuLphant()				{return m_bEmuLphant;}
	static	bool	IsEmuLog()					{return m_bLogEmulator;}
	// <== Emulate others [WiZaRd/Spike/shadow2004] - Stulle

	// ==> Spread Credits Slot [Stulle] - Stulle
	static	bool	GetSpreadCreditsSlot()			{return SpreadCreditsSlot;}
	static  void    SetSpreadCreditsSlotCounter		(uint16 in) { SpreadCreditsSlotCounter = in; }
	static	uint16	GetSpreadCreditsSlotCounter()	{return SpreadCreditsSlotCounter;}
	// <== Spread Credits Slot [Stulle] - Stulle

	// ==> Pay Back First [AndCycle/SiRoB/Stulle] - Stulle
	static	bool	IsPayBackFirst()		{return m_bPayBackFirst;}
	static	uint8	GetPayBackFirstLimit()	{return m_iPayBackFirstLimit;}
	static	bool	IsPayBackFirst2()		{return m_bPayBackFirst2;}
	static	uint16	GetPayBackFirstLimit2()	{return m_iPayBackFirstLimit2;}
	// <== Pay Back First [AndCycle/SiRoB/Stulle] - Stulle

	static	bool	GetIgnoreThird()		{return m_bIgnoreThird;} // Do not reserve 1/3 of your uploadlimit for emule [Stulle] - Stulle

	static	bool	GetDisableUlThres()		{return m_bDisableUlThres;} // Disable accepting only clients who asked within last 30min [Stulle] - Stulle

	static	bool	IsFollowTheMajorityEnabled() { return m_bFollowTheMajority;} // Follow The Majority [AndCycle/Stulle] - Stulle

	static	int		GetFairPlay()			{ return m_iFairPlay; } //Fair Play [AndCycle/Stulle] - Stulle

	static	bool	GetMaxSlotSpeed()		{ return m_bMaxSlotSpeed; } // Alwasy maximize slot speed [Stulle] - Stulle

	static uint32	GetReAskTimeDif()		{return m_uReAskTimeDif;} // Timer for ReAsk File Sources [Stulle] - Stulle

	// ==> Advanced Updates [MorphXT/Stulle] - Stulle
	static bool			IsAutoUpdateAntiLeech()			{return m_bAutoUpdateAntiLeech;}
	static CString		GetAntiLeechURL()				{return m_strAntiLeechURL;}
	static LPSYSTEMTIME	GetIPfilterVersion()			{return &m_IPfilterVersion;}
	static uint32		GetIPFilterVersionNum()			{return m_uIPFilterVersionNum;}
	static LPSYSTEMTIME	GetIP2CountryVersion()			{return &m_IP2CountryVersion;}
	static bool			IsAutoUPdateIP2CountryEnabled()	{return AutoUpdateIP2Country;}
	static CString		GetUpdateURLIP2Country()		{return UpdateURLIP2Country;}
	static bool			IsIPFilterViaDynDNS(CString strURL = NULL);
	// <== Advanced Updates [MorphXT/Stulle] - Stulle

	static void		SetBindAddr(CStringW bindip); // Advanced Options [Official/MorphXT] - Stulle

	static bool		FineCS()			{return m_bFineCS;} // Modified FineCS [CiccioBastardo/Stulle] - Stulle

	static bool		GetTrayComplete()		{ return m_bTrayComplete; } // Completed in Tray [Stulle] - Stulle

	static bool		GetColorFeedback()		{ return m_bColorFeedback; } // Feedback personalization [Stulle] - Stulle

	// ==> Advanced Transfer Window Layout [Stulle] - Stulle
	static	bool	GetSplitWindow()		{ return m_bSplitWindow; }
	static	void	SetSplitWindow(bool in)	{ m_bSplitWindow = in; }
	// <== Advanced Transfer Window Layout [Stulle] - Stulle

	static	bool	DateFileNameLog()		{ return m_bDateFileNameLog;} // Date File Name Log [AndCycle] - Stulle

	static	bool	UseIonixWebsrv()		{ return m_bIonixWebsrv; } // Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he/Stulle] - Stulle

	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	static int      GetServiceStartupMode();
	static int		GetServiceOptLvl()		{ return m_iServiceOptLvl; }
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle
	// ==> Adjustable NT Service Strings [Stulle] - Stulle
	static	CString	GetServiceName();
	static	void	SetServiceName(CString in)		{ m_strServiceName = in; }
	static	CString	GetServiceDispName()			{ return m_strServiceDispName; }
	static	void	SetServiceDispName(CString in)	{ m_strServiceDispName = in; }
	static	CString	GetServiceDescr()				{ return m_strServiceDescr; }
	static	void	SetServiceDescr(CString in)		{ m_strServiceDescr = in; }
	// <== Adjustable NT Service Strings [Stulle] - Stulle

	// ==> Mephisto Upload - Mephisto
	static uint8	GetMinSlots()			{return m_uMinSlots;}
	static uint8	GetNoNewSlotTimer()		{return m_uNoNewSlotTimer;}
	static uint8	GetFullLoops()			{return m_uFullLoops;}
	static uint8	GetMonitorLoops()		{return m_uMonitorLoops;}
	static uint8	GetNotReachedBW()		{return m_uNotReachedBW;}
	static uint8	GetNoTrickleTimer()		{return m_uNoTrickleTimer;}
	static uint16	GetMoveDownKB()			{return m_uMoveDownKB;}
	// <== Mephisto Upload - Mephisto

	// ==> Multiple Part Transfer [Stulle] - Mephisto
	static uint8	GetChunksMode()			{return m_uChunksMode;}
	static uint8	GetChunksToFinish()		{return m_uChunksToFinish;}
	static uint8	GetChunksToUpload()		{return m_uChunksToUpload;}
	// <== Multiple Part Transfer [Stulle] - Mephisto
	static uint16	GetMaxUpMinutes()		{return m_uMaxUpMinutes;} // Adjust max upload time [Stulle] - Mephisto

protected:
	static	CString m_strFileCommentsFilePath;
	static	Preferences_Ext_Struct* prefsExt;
	static	WORD m_wWinVer;
	static	CArray<Category_Struct*,Category_Struct*> catMap;
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
