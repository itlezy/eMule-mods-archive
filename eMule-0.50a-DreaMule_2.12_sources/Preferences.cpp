//this file is part of eMule
//Copyright (C)2002-2005 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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
#include "Opcodes.h"
#include "OtherFunctions.h"
#include "Ini2.h"
#include "DownloadQueue.h"
#include "UploadQueue.h"
#include "Statistics.h"
#include "MD5Sum.h"
#include "PartFile.h"
#include "Sockets.h"
#include "ListenSocket.h"
#include "ServerList.h"
#include "SharedFileList.h"
#include "UpDownClient.h"
#include "SafeFile.h"
#include "emuledlg.h"
#include "StatisticsDlg.h"
#include "Log.h"
#include "MuleToolbarCtrl.h"
//Xman
#include "BandWidthControl.h" // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
//boizaum
#include "AdunanzA.h"
//boizaum
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CPreferences thePrefs;
//-------------------------------------------------------------------------------
//Xman Xtreme Mod:

//upnp_start
bool	CPreferences::m_bUPnPNat; // UPnP On/Off
bool	CPreferences::m_bUPnPTryRandom; // Try to use random external port if already in use On/Off
uint16	CPreferences::m_iUPnPTCPExternal = 0; // TCP External Port
uint16	CPreferences::m_iUPnPUDPExternal = 0; // UDP External Port
//upnp_end

//Xman Xtreme Upload
float	CPreferences::m_slotspeed;
bool	CPreferences::m_openmoreslots;
bool	CPreferences::m_bandwidthnotreachedslots;
int		CPreferences::m_sendbuffersize;
int		CPreferences::m_internetdownreactiontime;

//Xman process prio
uint32	CPreferences::m_MainProcessPriority; // [TPT] - Select process priority

//Xman Anti-Leecher
bool CPreferences::m_antileecher;
bool CPreferences::m_antileechername;
bool CPreferences::m_antighost;
bool CPreferences::m_antileecherbadhello;
bool CPreferences::m_antileechersnafu;
bool CPreferences::m_antileechermod;
bool CPreferences::m_antileecherthief;
bool CPreferences::m_antileecherspammer;
bool CPreferences::m_antileecherxsexploiter;
bool CPreferences::m_antileecheremcrypt;
bool CPreferences::m_antileechercommunity_action;
bool CPreferences::m_antileecherghost_action;
bool CPreferences::m_antileecherthief_action;
//Xman end

//Xman narrow font at transferwindow
bool CPreferences::m_bUseNarrowFont;
//Xman end

//Xman 1:3 Ratio
bool CPreferences::m_13ratio;
//Xman end

//Xman chunk chooser
uint8 CPreferences::m_chunkchooser;

//Xman disable compression
bool CPreferences::m_bUseCompression;


//Xman Funny-Nick (Stulle/Morph)
bool CPreferences::m_bFunnyNick;
//Xman end

//Xman count block/success send
bool CPreferences::m_showblockratio;
bool CPreferences::m_dropblockingsockets;

//Xman remove unused AICH-hashes
bool CPreferences::m_rememberAICH;

//Xman smooth-accurate-graph
bool CPreferences::usesmoothgraph;

bool CPreferences::retryconnectionattempts; //Xman

// Maella
float	CPreferences::maxupload;                    // [FAF] -Allow Bandwidth Settings in <1KB Incremements-
float	CPreferences::maxdownload;
float	CPreferences::maxGraphDownloadRate;
float	CPreferences::maxGraphUploadRate;

uint8	CPreferences::zoomFactor;                   // -Graph: display zoom-

uint16	CPreferences::MTU;                          // -MTU Configuration-
bool	CPreferences::usedoublesendsize;

bool	CPreferences::NAFCFullControl;	          // -Network Adapter Feedback Control-
uint32	CPreferences::forceNAFCadapter;
uint8	CPreferences::datarateSamples;              // -Accurate measure of bandwidth: eDonkey data + control, network adapter-

bool    CPreferences::enableMultiQueue;             // -One-queue-per-file- (idea bloodymad)
bool    CPreferences::enableReleaseMultiQueue;
// Maella end

// Mighty Knife: Static server handling
bool	CPreferences::m_bDontRemoveStaticServers;
// [end] Mighty Knife


//Xman [MoNKi: -Downloaded History-]
bool	CPreferences::m_bHistoryShowShared;
//Xman end

//Xman GlobalMaxHarlimit for fairness
uint32	CPreferences::m_uMaxGlobalSources;
bool	CPreferences::m_bAcceptsourcelimit;

//Xman show additional graph lines
bool	CPreferences::m_bShowAdditionalGraph;



//Xman don't overwrite bak files if last sessions crashed
bool	CPreferences::m_this_session_aborted_in_an_unnormal_way;
bool	CPreferences::m_last_session_aborted_in_an_unnormal_way;


//Xman end
//-------------------------------------------------------------------------------

int		CPreferences::m_iDbgHeap;
CString	CPreferences::strNick;
uint16	CPreferences::minupload;
LPCSTR	CPreferences::m_pszBindAddrA;
CStringA CPreferences::m_strBindAddrA;
LPCWSTR	CPreferences::m_pszBindAddrW;
CStringW CPreferences::m_strBindAddrW;
uint16	CPreferences::port;
uint16	CPreferences::udpport;
uint16	CPreferences::nServerUDPPort;
UINT	CPreferences::maxconnections;
UINT	CPreferences::maxhalfconnections;
bool	CPreferences::m_bConditionalTCPAccept;
bool	CPreferences::reconnect;
bool	CPreferences::m_bUseServerPriorities;
bool	CPreferences::m_bUseUserSortedServerList;
TCHAR	CPreferences::incomingdir[MAX_PATH];
CStringArray CPreferences::tempdir;
bool	CPreferences::ICH;
bool	CPreferences::m_bAutoUpdateServerList;
bool	CPreferences::updatenotify;
bool	CPreferences::mintotray;
bool	CPreferences::autoconnect;
bool	CPreferences::m_bAutoConnectToStaticServersOnly;
bool	CPreferences::autotakeed2klinks;
bool	CPreferences::addnewfilespaused;
UINT	CPreferences::depth3D;
bool	CPreferences::m_bEnableMiniMule;
int		CPreferences::m_iStraightWindowStyles;
bool	CPreferences::m_bRTLWindowsLayout;
CString	CPreferences::m_strSkinProfile;
CString	CPreferences::m_strSkinProfileDir;
bool	CPreferences::m_bAddServersFromServer;
bool	CPreferences::m_bAddServersFromClients;
UINT	CPreferences::maxsourceperfile;
UINT	CPreferences::trafficOMeterInterval;
UINT	CPreferences::statsInterval;
uchar	CPreferences::userhash[16];
WINDOWPLACEMENT CPreferences::EmuleWindowPlacement;
bool	CPreferences::beepOnError;
bool	CPreferences::m_bIconflashOnNewMessage;
bool	CPreferences::confirmExit;
DWORD	CPreferences::m_adwStatsColors[15];
bool	CPreferences::splashscreen;
bool	CPreferences::filterLANIPs;
bool	CPreferences::m_bAllocLocalHostIP;
bool	CPreferences::onlineSig;
uint64	CPreferences::cumDownOverheadTotal;
uint64	CPreferences::cumDownOverheadFileReq;
uint64	CPreferences::cumDownOverheadSrcEx;
uint64	CPreferences::cumDownOverheadServer;
uint64	CPreferences::cumDownOverheadKad;
uint64	CPreferences::cumDownOverheadTotalPackets;
uint64	CPreferences::cumDownOverheadFileReqPackets;
uint64	CPreferences::cumDownOverheadSrcExPackets;
uint64	CPreferences::cumDownOverheadServerPackets;
uint64	CPreferences::cumDownOverheadKadPackets;
uint64	CPreferences::cumUpOverheadTotal;
uint64	CPreferences::cumUpOverheadFileReq;
uint64	CPreferences::cumUpOverheadSrcEx;
uint64	CPreferences::cumUpOverheadServer;
uint64	CPreferences::cumUpOverheadKad;
uint64	CPreferences::cumUpOverheadTotalPackets;
uint64	CPreferences::cumUpOverheadFileReqPackets;
uint64	CPreferences::cumUpOverheadSrcExPackets;
uint64	CPreferences::cumUpOverheadServerPackets;
uint64	CPreferences::cumUpOverheadKadPackets;
uint32	CPreferences::cumUpSuccessfulSessions;
uint32	CPreferences::cumUpFailedSessions;
uint32	CPreferences::cumUpAvgTime;
uint64	CPreferences::cumUpData_EDONKEY;
uint64	CPreferences::cumUpData_EDONKEYHYBRID;
uint64	CPreferences::cumUpData_EMULE;
uint64	CPreferences::cumUpData_MLDONKEY;
uint64	CPreferences::cumUpData_AMULE;
uint64	CPreferences::cumUpData_EMULECOMPAT;
uint64	CPreferences::cumUpData_SHAREAZA;
uint64	CPreferences::sesUpData_EDONKEY;
uint64	CPreferences::sesUpData_EDONKEYHYBRID;
uint64	CPreferences::sesUpData_EMULE;
uint64	CPreferences::sesUpData_MLDONKEY;
uint64	CPreferences::sesUpData_AMULE;
uint64	CPreferences::sesUpData_EMULECOMPAT;
uint64	CPreferences::sesUpData_SHAREAZA;
uint64	CPreferences::cumUpDataPort_4662;
uint64	CPreferences::cumUpDataPort_OTHER;
uint64	CPreferences::cumUpDataPort_PeerCache;
uint64	CPreferences::sesUpDataPort_4662;
uint64	CPreferences::sesUpDataPort_OTHER;
uint64	CPreferences::sesUpDataPort_PeerCache;
uint64	CPreferences::cumUpData_File;
uint64	CPreferences::cumUpData_Partfile;
uint64	CPreferences::sesUpData_File;
uint64	CPreferences::sesUpData_Partfile;
uint32	CPreferences::cumDownCompletedFiles;
uint32	CPreferences::cumDownSuccessfulSessions;
uint32	CPreferences::cumDownFailedSessions;
uint32	CPreferences::cumDownAvgTime;
uint64	CPreferences::cumLostFromCorruption;
uint64	CPreferences::cumSavedFromCompression;
uint32	CPreferences::cumPartsSavedByICH;
uint32	CPreferences::sesDownSuccessfulSessions;
uint32	CPreferences::sesDownFailedSessions;
uint32	CPreferences::sesDownAvgTime;
uint32	CPreferences::sesDownCompletedFiles;
uint64	CPreferences::sesLostFromCorruption;
uint64	CPreferences::sesSavedFromCompression;
uint32	CPreferences::sesPartsSavedByICH;
uint64	CPreferences::cumDownData_EDONKEY;
uint64	CPreferences::cumDownData_EDONKEYHYBRID;
uint64	CPreferences::cumDownData_EMULE;
uint64	CPreferences::cumDownData_MLDONKEY;
uint64	CPreferences::cumDownData_AMULE;
uint64	CPreferences::cumDownData_EMULECOMPAT;
uint64	CPreferences::cumDownData_SHAREAZA;
uint64	CPreferences::cumDownData_URL;
uint64	CPreferences::sesDownData_EDONKEY;
uint64	CPreferences::sesDownData_EDONKEYHYBRID;
uint64	CPreferences::sesDownData_EMULE;
uint64	CPreferences::sesDownData_MLDONKEY;
uint64	CPreferences::sesDownData_AMULE;
uint64	CPreferences::sesDownData_EMULECOMPAT;
uint64	CPreferences::sesDownData_SHAREAZA;
uint64	CPreferences::sesDownData_URL;
uint64	CPreferences::cumDownDataPort_4662;
uint64	CPreferences::cumDownDataPort_OTHER;
uint64	CPreferences::cumDownDataPort_PeerCache;
uint64	CPreferences::sesDownDataPort_4662;
uint64	CPreferences::sesDownDataPort_OTHER;
uint64	CPreferences::sesDownDataPort_PeerCache;
float	CPreferences::cumConnAvgDownRate;
float	CPreferences::cumConnMaxAvgDownRate;
float	CPreferences::cumConnMaxDownRate;
float	CPreferences::cumConnAvgUpRate;
float	CPreferences::cumConnMaxAvgUpRate;
float	CPreferences::cumConnMaxUpRate;
time_t	CPreferences::cumConnRunTime;
uint32	CPreferences::cumConnNumReconnects;
uint32	CPreferences::cumConnAvgConnections;
uint32	CPreferences::cumConnMaxConnLimitReached;
uint32	CPreferences::cumConnPeakConnections;
uint32	CPreferences::cumConnTransferTime;
uint32	CPreferences::cumConnDownloadTime;
uint32	CPreferences::cumConnUploadTime;
uint32	CPreferences::cumConnServerDuration;
uint32	CPreferences::cumSrvrsMostWorkingServers;
uint32	CPreferences::cumSrvrsMostUsersOnline;
uint32	CPreferences::cumSrvrsMostFilesAvail;
uint32	CPreferences::cumSharedMostFilesShared;
uint64	CPreferences::cumSharedLargestShareSize;
uint64	CPreferences::cumSharedLargestAvgFileSize;
uint64	CPreferences::cumSharedLargestFileSize;
time_t	CPreferences::stat_datetimeLastReset;
UINT	CPreferences::statsConnectionsGraphRatio;
UINT	CPreferences::statsSaveInterval;
TCHAR	CPreferences::statsExpandedTreeItems[256];
bool	CPreferences::m_bShowVerticalHourMarkers;
uint64	CPreferences::totalDownloadedBytes;
uint64	CPreferences::totalUploadedBytes;
WORD	CPreferences::m_wLanguageID;
bool	CPreferences::transferDoubleclick;
EViewSharedFilesAccess CPreferences::m_iSeeShares;
UINT	CPreferences::m_iToolDelayTime;
bool	CPreferences::bringtoforeground;
UINT	CPreferences::splitterbarPosition;
UINT	CPreferences::splitterbarPositionSvr;
UINT	CPreferences::splitterbarPositionStat;
UINT	CPreferences::splitterbarPositionStat_HL;
UINT	CPreferences::splitterbarPositionStat_HR;
UINT	CPreferences::splitterbarPositionFriend;
UINT	CPreferences::splitterbarPositionIRC;
UINT	CPreferences::splitterbarPositionShared;
UINT	CPreferences::m_uTransferWnd1;
UINT	CPreferences::m_uTransferWnd2;
UINT	CPreferences::m_uDeadServerRetries;
DWORD	CPreferences::m_dwServerKeepAliveTimeout;
UINT	CPreferences::statsMax;
UINT	CPreferences::statsAverageMinutes;
CString	CPreferences::notifierConfiguration;
bool	CPreferences::notifierOnDownloadFinished;
bool	CPreferences::notifierOnNewDownload;
bool	CPreferences::notifierOnChat;
bool	CPreferences::notifierOnLog;
bool	CPreferences::notifierOnImportantError;
bool	CPreferences::notifierOnEveryChatMsg;
bool	CPreferences::notifierOnNewVersion;
ENotifierSoundType CPreferences::notifierSoundType = ntfstNoSound;
CString	CPreferences::notifierSoundFile;
TCHAR	CPreferences::m_sircserver[50];
TCHAR	CPreferences::m_sircnick[30];
TCHAR	CPreferences::m_sircchannamefilter[50];
bool	CPreferences::m_bircaddtimestamp;
bool	CPreferences::m_bircusechanfilter;
UINT	CPreferences::m_iircchanneluserfilter;
TCHAR	CPreferences::m_sircperformstring[255];
bool	CPreferences::m_bircuseperform;
bool	CPreferences::m_birclistonconnect;
bool	CPreferences::m_bircacceptlinks;
bool	CPreferences::m_bircacceptlinksfriends;
bool	CPreferences::m_bircsoundevents;
bool	CPreferences::m_bircignoremiscmessage;
bool	CPreferences::m_bircignorejoinmessage;
bool	CPreferences::m_bircignorepartmessage;
bool	CPreferences::m_bircignorequitmessage;
bool	CPreferences::m_bircignoreemuleprotoaddfriend;
bool	CPreferences::m_bircallowemuleprotoaddfriend;
bool	CPreferences::m_bircignoreemuleprotosendlink;
bool	CPreferences::m_birchelpchannel;
bool	CPreferences::m_bRemove2bin;
bool	CPreferences::m_bShowCopyEd2kLinkCmd;
bool	CPreferences::m_bpreviewprio;
bool	CPreferences::m_bSmartServerIdCheck;
uint8	CPreferences::smartidstate;
bool	CPreferences::m_bSafeServerConnect;
bool	CPreferences::startMinimized;
bool	CPreferences::m_bAutoStart;
bool	CPreferences::m_bRestoreLastMainWndDlg;
int		CPreferences::m_iLastMainWndDlgID;
bool	CPreferences::m_bRestoreLastLogPane;
int		CPreferences::m_iLastLogPaneID;
UINT	CPreferences::MaxConperFive;
bool	CPreferences::checkDiskspace;
UINT	CPreferences::m_uMinFreeDiskSpace;
bool	CPreferences::m_bSparsePartFiles;
CString	CPreferences::m_strYourHostname;
bool	CPreferences::m_bEnableVerboseOptions;
bool	CPreferences::m_bVerbose;
bool	CPreferences::m_bFullVerbose;
bool	CPreferences::m_bDebugSourceExchange;
bool	CPreferences::m_bLogBannedClients;
bool	CPreferences::m_bLogRatingDescReceived;
bool	CPreferences::m_bLogSecureIdent;
bool	CPreferences::m_bLogFilteredIPs;
bool	CPreferences::m_bLogFileSaving;
bool	CPreferences::m_bLogA4AF; // ZZ:DownloadManager
bool	CPreferences::m_bLogDrop; //Xman Xtreme Downloadmanager
bool	CPreferences::m_bLogpartmismatch; //Xman Log part/size-mismatch
bool	CPreferences::m_bLogUlDlEvents;
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
bool	CPreferences::m_bUseDebugDevice = true;
#else
bool	CPreferences::m_bUseDebugDevice = false;
#endif
int		CPreferences::m_iDebugServerTCPLevel;
int		CPreferences::m_iDebugServerUDPLevel;
int		CPreferences::m_iDebugServerSourcesLevel;
int		CPreferences::m_iDebugServerSearchesLevel;
int		CPreferences::m_iDebugClientTCPLevel;
int		CPreferences::m_iDebugClientUDPLevel;
int		CPreferences::m_iDebugClientKadUDPLevel;
int		CPreferences::m_iDebugSearchResultDetailLevel;
bool	CPreferences::m_bupdatequeuelist;
bool	CPreferences::m_bManualAddedServersHighPriority;
bool	CPreferences::m_btransferfullchunks;
int		CPreferences::m_istartnextfile;
bool	CPreferences::m_bshowoverhead;
bool	CPreferences::m_bDAP;
bool	CPreferences::m_bUAP;
bool	CPreferences::m_bDisableKnownClientList;
bool	CPreferences::m_bDisableQueueList;
bool	CPreferences::m_bExtControls;
bool	CPreferences::m_bTransflstRemain;
UINT	CPreferences::versioncheckdays;
bool	CPreferences::showRatesInTitle;
TCHAR	CPreferences::TxtEditor[MAX_PATH];
CString	CPreferences::m_strVideoPlayer;
CString CPreferences::m_strVideoPlayerArgs;
bool	CPreferences::moviePreviewBackup;
int		CPreferences::m_iPreviewSmallBlocks;
bool	CPreferences::m_bPreviewCopiedArchives;
int		CPreferences::m_iInspectAllFileTypes;
bool	CPreferences::m_bPreviewOnIconDblClk;
bool	CPreferences::indicateratings;
bool	CPreferences::watchclipboard;
bool	CPreferences::filterserverbyip;
bool	CPreferences::m_bFirstStart;
bool	CPreferences::m_bCreditSystem;
bool	CPreferences::log2disk;
bool	CPreferences::debug2disk;
int		CPreferences::iMaxLogBuff;
UINT	CPreferences::uMaxLogFileSize;
ELogFileFormat CPreferences::m_iLogFileFormat = Unicode;
bool	CPreferences::scheduler;
bool	CPreferences::dontcompressavi;
bool	CPreferences::msgonlyfriends;
bool	CPreferences::msgsecure;
UINT	CPreferences::filterlevel;
UINT	CPreferences::m_iFileBufferSize;
bool	CPreferences::m_bIntelliFlush; //>>> WiZaRd::IntelliFlush
UINT	CPreferences::m_iQueueSize;
int		CPreferences::m_iCommitFiles;
UINT	CPreferences::maxmsgsessions;
uint32	CPreferences::versioncheckLastAutomatic;

CString	CPreferences::messageFilter;
CString	CPreferences::commentFilter;
CString	CPreferences::filenameCleanups;
TCHAR	CPreferences::datetimeformat[64];
TCHAR	CPreferences::datetimeformat4log[64];
LOGFONT CPreferences::m_lfHyperText;
LOGFONT CPreferences::m_lfLogText;
COLORREF CPreferences::m_crLogError = RGB(255, 0, 0);
//>>> WiZaRd::"Proper" Colors
//showing warnings in blue and success in green makes more sense :P
COLORREF CPreferences::m_crLogWarning = RGB(0, 0, 255);
COLORREF CPreferences::m_crLogSuccess = RGB(0, 192, 0);
//COLORREF CPreferences::m_crLogWarning = RGB(128, 0, 128);
//COLORREF CPreferences::m_crLogSuccess = RGB(0, 0, 255);
//<<< WiZaRd::"Proper" Colors
int		CPreferences::m_iExtractMetaData;
bool	CPreferences::m_bAdjustNTFSDaylightFileTime = true;
TCHAR	CPreferences::m_sWebPassword[256];
TCHAR	CPreferences::m_sWebLowPassword[256];
CUIntArray CPreferences::m_aAllowedRemoteAccessIPs;
uint16	CPreferences::m_nWebPort;
bool	CPreferences::m_bWebEnabled;
bool	CPreferences::m_bWebUseGzip;
int		CPreferences::m_nWebPageRefresh;
bool	CPreferences::m_bWebLowEnabled;
TCHAR	CPreferences::m_sWebResDir[MAX_PATH];
int		CPreferences::m_iWebTimeoutMins;
int		CPreferences::m_iWebFileUploadSizeLimitMB;
bool	CPreferences::m_bAllowAdminHiLevFunc;
TCHAR	CPreferences::m_sTemplateFile[MAX_PATH];
ProxySettings CPreferences::proxy;
bool	CPreferences::showCatTabInfos;
bool	CPreferences::resumeSameCat;
bool	CPreferences::dontRecreateGraphs;
bool	CPreferences::autofilenamecleanup;
bool	CPreferences::m_bUseAutocompl;
bool	CPreferences::m_bShowDwlPercentage;
bool	CPreferences::m_bRemoveFinishedDownloads;
UINT	CPreferences::m_iMaxChatHistory;
bool	CPreferences::m_bShowActiveDownloadsBold;
int		CPreferences::m_iSearchMethod;
bool	CPreferences::m_bAdvancedSpamfilter;
bool	CPreferences::m_bUseSecureIdent;
//remove moblie
bool	CPreferences::networkkademlia;
bool	CPreferences::networked2k;
EToolbarLabelType CPreferences::m_nToolbarLabels;
CString	CPreferences::m_sToolbarBitmap;
CString	CPreferences::m_sToolbarBitmapFolder;
CString	CPreferences::m_sToolbarSettings;
bool	CPreferences::m_bReBarToolbar;
CSize	CPreferences::m_sizToolbarIconSize;
bool	CPreferences::m_bPreviewEnabled;
bool	CPreferences::m_bAutomaticArcPreviewStart;
bool	CPreferences::m_bDynUpEnabled;
int		CPreferences::m_iDynUpPingTolerance;
int		CPreferences::m_iDynUpGoingUpDivider;
int		CPreferences::m_iDynUpGoingDownDivider;
int		CPreferences::m_iDynUpNumberOfPings;
int		CPreferences::m_iDynUpPingToleranceMilliseconds;
bool	CPreferences::m_bDynUpUseMillisecondPingTolerance;
bool    CPreferences::m_bAllocFull;
// ZZ:DownloadManager -->
//bool    CPreferences::m_bA4AFSaveCpu;
// ZZ:DownloadManager <--
bool    CPreferences::m_bHighresTimer;
CStringList CPreferences::shareddir_list;
CStringList CPreferences::addresses_list;
CString CPreferences::appdir;
CString CPreferences::configdir;
CString CPreferences::m_strWebServerDir;
CString CPreferences::m_strLangDir;
CString CPreferences::m_strFileCommentsFilePath;
CString	CPreferences::m_strLogDir;
Preferences_Ext_Struct* CPreferences::prefsExt;
WORD	CPreferences::m_wWinVer;
CArray<Category_Struct*,Category_Struct*> CPreferences::catMap;
UINT	CPreferences::m_nWebMirrorAlertLevel;
bool	CPreferences::m_bRunAsUser;
bool	CPreferences::m_bPreferRestrictedOverUser;
bool	CPreferences::m_bUseOldTimeRemaining;
uint32	CPreferences::m_uPeerCacheLastSearch;
bool	CPreferences::m_bPeerCacheWasFound;
bool	CPreferences::m_bPeerCacheEnabled;
uint16	CPreferences::m_nPeerCachePort;
bool	CPreferences::m_bPeerCacheShow;

bool	CPreferences::m_bOpenPortsOnStartUp;
int		CPreferences::m_byLogLevel;
bool	CPreferences::m_bTrustEveryHash;
bool	CPreferences::m_bRememberCancelledFiles;
bool	CPreferences::m_bRememberDownloadedFiles;

bool	CPreferences::m_bNotifierSendMail;
CString	CPreferences::m_strNotifierMailServer;
CString	CPreferences::m_strNotifierMailSender;
CString	CPreferences::m_strNotifierMailReceiver;

bool	CPreferences::m_bWinaTransToolbar;

bool	CPreferences::m_bCryptLayerRequested;
bool	CPreferences::m_bCryptLayerSupported;
bool	CPreferences::m_bCryptLayerRequired;


//boiuzam
uint32	CPreferences::m_AduTips;
//boizaum
bool    CPreferences::hotkey_enabled = false;						 // TK4 mod
int     CPreferences::hotkey_mod = MOD_CONTROL + MOD_ALT;			 // TK4 mod
int     CPreferences::hotkey_key = (int)'Y';						 // TK4 mod
HWND    CPreferences::HideWnd;							   		     // TK4 mod
bool	CPreferences::BadNameCheck = true;						 	 // TK4 mod - [FDC]
bool    CPreferences::doubleFDC = true;                             // TK4 mod - [FDC]  double detection if 'true' alert on 2 bad names
int     CPreferences::FDCSensitivity = 92;						     // TK4 mod - [FDC] System sensitivity

CPreferences::CPreferences()
{
#ifdef _DEBUG
	m_iDbgHeap = 1;
#endif
}

CPreferences::~CPreferences()
{
	delete prefsExt;
}

LPCTSTR CPreferences::GetConfigFile()
{
	return theApp.m_pszProfileName;
}

void CPreferences::Init()
{
	srand((uint32)time(0)); // we need random numbers sometimes

	prefsExt = new Preferences_Ext_Struct;
	memset(prefsExt, 0, sizeof *prefsExt);

	//get application start directory
	TCHAR buffer[490];
	::GetModuleFileName(0, buffer, 490);
	LPTSTR pszFileName = _tcsrchr(buffer, L'\\') + 1;
	*pszFileName = L'\0';

	appdir = buffer;
	configdir = appdir + CONFIGFOLDER;
	m_strWebServerDir = appdir + L"webserver\\";
	m_strLangDir = appdir + L"lang\\";
	m_strFileCommentsFilePath = configdir + L"fileinfo.ini";
	m_strLogDir = appdir + L"logs\\";

	///////////////////////////////////////////////////////////////////////////
	// Create 'config' directory (and optionally move files from application directory)
	//
	::CreateDirectory(GetConfigDir(), 0);

	///////////////////////////////////////////////////////////////////////////
	// Create 'logs' directory (and optionally move files from application directory)
	//
	::CreateDirectory(GetLogDir(), 0);
	CFileFind ff;
	BOOL bFoundFile = ff.FindFile(GetAppDir() + L"eMule*.log", 0);
	while (bFoundFile)
	{
		bFoundFile = ff.FindNextFile();
		if (ff.IsDots() || ff.IsSystem() || ff.IsDirectory() || ff.IsHidden())
			continue;
		MoveFile(ff.GetFilePath(), GetLogDir() + ff.GetFileName());
	}


	CreateUserHash();

	// load preferences.dat or set standart values
	TCHAR* fullpath = new TCHAR[_tcslen(configdir)+16];
	_stprintf(fullpath,L"%spreferences.dat",configdir);
	FILE* preffile = _tfsopen(fullpath,L"rb", _SH_DENYWR);
	delete[] fullpath;

	LoadPreferences();

	if (!preffile){
		SetStandartValues();
	}
	else{
		fread(prefsExt,sizeof(Preferences_Ext_Struct),1,preffile);
		if (ferror(preffile))
			SetStandartValues();

		md4cpy(userhash, prefsExt->userhash);
		EmuleWindowPlacement = prefsExt->EmuleWindowPlacement;

		fclose(preffile);
		smartidstate = 0;
	}

	// shared directories
	fullpath = new TCHAR[_tcslen(configdir) + MAX_PATH];
	_stprintf(fullpath, L"%sshareddir.dat", configdir);
	CStdioFile* sdirfile = new CStdioFile();
	bool bIsUnicodeFile = IsUnicodeFile(fullpath); // check for BOM
	// open the text file either in ANSI (text) or Unicode (binary), this way we can read old and new files
	// with nearly the same code..
	if (sdirfile->Open(fullpath, CFile::modeRead | CFile::shareDenyWrite | (bIsUnicodeFile ? CFile::typeBinary : 0)))
	{
		try {
			if (bIsUnicodeFile)
				sdirfile->Seek(sizeof(WORD), SEEK_CUR); // skip BOM

			CString toadd;
			while (sdirfile->ReadString(toadd))
			{
				toadd.Trim(L" \t\r\n"); // need to trim '\r' in binary mode
				if (toadd.IsEmpty())
					continue;

				TCHAR szFullPath[MAX_PATH];
				if (PathCanonicalize(szFullPath, toadd))
					toadd = szFullPath;

				if (!IsShareableDirectory(toadd) )
					continue;

				if (_taccess(toadd, 0) == 0) { // only add directories which still exist
					if (toadd.Right(1) != L'\\')
						toadd.Append(L"\\");
					shareddir_list.AddHead(toadd);
				}
			}
		}
		catch (CFileException* ex) {
			ASSERT(0);
			ex->Delete();
		}
		sdirfile->Close();
	}
	delete sdirfile;
	delete[] fullpath;

	// serverlist addresses
	// filename update to reasonable name
	if (PathFileExists( configdir + L"adresses.dat") ) {
		if (PathFileExists( configdir + L"addresses.dat") )
			DeleteFile( configdir + L"adresses.dat");
		else
			MoveFile( configdir + L"adresses.dat", configdir + L"addresses.dat");
	}

	fullpath = new TCHAR[_tcslen(configdir) + 20];
	_stprintf(fullpath, L"%saddresses.dat", configdir);
	sdirfile = new CStdioFile();
	bIsUnicodeFile = IsUnicodeFile(fullpath);
	if (sdirfile->Open(fullpath, CFile::modeRead | CFile::shareDenyWrite | (bIsUnicodeFile ? CFile::typeBinary : 0)))
	{
		try {
			if (bIsUnicodeFile)
				sdirfile->Seek(sizeof(WORD), SEEK_CUR); // skip BOM

			CString toadd;
			while (sdirfile->ReadString(toadd))
			{
				toadd.Trim(L" \t\r\n"); // need to trim '\r' in binary mode
				if (toadd.IsEmpty())
					continue;
				addresses_list.AddHead(toadd);
			}
		}
		catch (CFileException* ex) {
			ASSERT(0);
			ex->Delete();
		}
		sdirfile->Close();
	}
	delete sdirfile;
	delete[] fullpath;
	fullpath = NULL;

	userhash[5] = 14;
	userhash[14] = 111;

	// Explicitly inform the user about errors with incoming/temp folders!
	if (!PathFileExists(GetIncomingDir()) && !::CreateDirectory(GetIncomingDir(),0)) {
		CString strError;
		strError.Format(GetResString(IDS_ERR_CREATE_DIR), GetResString(IDS_PW_INCOMING), GetIncomingDir(), GetErrorMessage(GetLastError()));
		AfxMessageBox(strError, MB_ICONERROR);
		_stprintf(incomingdir,L"%sincoming",appdir);
		if (!PathFileExists(GetIncomingDir()) && !::CreateDirectory(GetIncomingDir(),0)){
			strError.Format(GetResString(IDS_ERR_CREATE_DIR), GetResString(IDS_PW_INCOMING), GetIncomingDir(), GetErrorMessage(GetLastError()));
			AfxMessageBox(strError, MB_ICONERROR);
		}
	}
	if (!PathFileExists(GetTempDir()) && !::CreateDirectory(GetTempDir(),0)) {
		CString strError;
		strError.Format(GetResString(IDS_ERR_CREATE_DIR), GetResString(IDS_PW_TEMP), GetTempDir(), GetErrorMessage(GetLastError()));
		AfxMessageBox(strError, MB_ICONERROR);

		tempdir.SetAt(0,appdir + L"temp" );
		if (!PathFileExists(GetTempDir()) && !::CreateDirectory(GetTempDir(),0)){
			strError.Format(GetResString(IDS_ERR_CREATE_DIR), GetResString(IDS_PW_TEMP), GetTempDir(), GetErrorMessage(GetLastError()));
			AfxMessageBox(strError, MB_ICONERROR);
		}
	}

	// Create 'skins' directory
	if (!PathFileExists(GetSkinProfileDir()) && !CreateDirectory(GetSkinProfileDir(), 0)) {
		m_strSkinProfileDir = appdir + L"skins";
		CreateDirectory(GetSkinProfileDir(), 0);
	}

	// Create 'toolbars' directory
	if (!PathFileExists(GetToolbarBitmapFolderSettings()) && !CreateDirectory(GetToolbarBitmapFolderSettings(), 0)) {
		m_sToolbarBitmapFolder = appdir + L"skins";
		CreateDirectory(GetToolbarBitmapFolderSettings(), 0);
	}


	if (((int*)userhash[0]) == 0 && ((int*)userhash[1]) == 0 && ((int*)userhash[2]) == 0 && ((int*)userhash[3]) == 0)
		CreateUserHash();
}

void CPreferences::Uninit()
{
	while (!catMap.IsEmpty())
	{
		Category_Struct* delcat = catMap.GetAt(0);
		catMap.RemoveAt(0);
		delete delcat;
	}
}

void CPreferences::SetStandartValues()
{
	CreateUserHash();

	WINDOWPLACEMENT defaultWPM;
	defaultWPM.length = sizeof(WINDOWPLACEMENT);
	defaultWPM.rcNormalPosition.left=10;defaultWPM.rcNormalPosition.top=10;
	defaultWPM.rcNormalPosition.right=700;defaultWPM.rcNormalPosition.bottom=500;
	defaultWPM.showCmd=0;
	EmuleWindowPlacement=defaultWPM;
	versioncheckLastAutomatic=0;


//	Save();
}

bool CPreferences::IsTempFile(const CString& rstrDirectory, const CString& rstrName)
{
	bool bFound = false;
	for (int i=0;i<tempdir.GetCount() && !bFound;i++)
		if (CompareDirectories(rstrDirectory, GetTempDir(i))==0)
			bFound = true; //ok, found a directory

	if(!bFound) //found nowhere - not a tempfile...
		return false;

	// do not share a file from the temp directory, if it matches one of the following patterns
	CString strNameLower(rstrName);
	strNameLower.MakeLower();
	strNameLower += L"|"; // append an EOS character which we can query for
	static const LPCTSTR _apszNotSharedExts[] = {
		L"%u.part" L"%c",
		L"%u.part.met" L"%c",
		L"%u.part.met" PARTMET_BAK_EXT L"%c",
		L"%u.part.met" PARTMET_TMP_EXT L"%c"
	};
	for (int i = 0; i < ARRSIZE(_apszNotSharedExts); i++){
		UINT uNum;
		TCHAR iChar;
		// "misuse" the 'scanf' function for a very simple pattern scanning.
		if (_stscanf(strNameLower, _apszNotSharedExts[i], &uNum, &iChar) == 2 && iChar == L'|')
			return true;
	}

	return false;
}

// SLUGFILLER: SafeHash
bool CPreferences::IsConfigFile(const CString& rstrDirectory, const CString& rstrName)
{
	if (CompareDirectories(rstrDirectory, configdir))
		return false;

	// do not share a file from the config directory, if it contains one of the following extensions
	static const LPCTSTR _apszNotSharedExts[] = { L".met.bak", L".ini.old" };
	for (int i = 0; i < ARRSIZE(_apszNotSharedExts); i++){
		int iLen = _tcslen(_apszNotSharedExts[i]);
		if (rstrName.GetLength()>=iLen && rstrName.Right(iLen).CompareNoCase(_apszNotSharedExts[i])==0)
			return true;
	}

	// do not share following files from the config directory
	static const LPCTSTR _apszNotSharedFiles[] =
	{
		L"AC_SearchStrings.dat",
		L"AC_ServerMetURLs.dat",
		L"addresses.dat",
		L"category.ini",
		L"clients.met",
		L"cryptkey.dat",
		L"emfriends.met",
		L"fileinfo.ini",
		L"ipfilter.dat",
		L"known.met",
		L"preferences.dat",
		L"preferences.ini",
		L"server.met",
		L"server.met.new",
		L"server_met.download",
		L"server_met.old",
		L"shareddir.dat",
		L"sharedsubdir.dat",
		L"staticservers.dat",
		L"webservices.dat"
	};
	for (int i = 0; i < ARRSIZE(_apszNotSharedFiles); i++){
		if (rstrName.CompareNoCase(_apszNotSharedFiles[i])==0)
			return true;
	}

	return false;
}
// SLUGFILLER: SafeHash

//Xman Xtreme Mod
// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
float CPreferences::GetMaxDownload() {
	//dont be a Lam3r :)
	const float maxUpload = (GetMaxUpload() >= UNLIMITED) ? GetMaxGraphUploadRate() : GetMaxUpload();
	if(maxUpload < 4.0f)
		return (3.0f * maxUpload < maxdownload) ? 3.0f * maxUpload : maxdownload;
	else if(maxUpload < 11.0f) //Xman changed to 11
		return (4.0f * maxUpload < maxdownload) ? 4.0f * maxUpload : maxdownload;
	else
		return maxdownload;
}
// Maella end

uint64 CPreferences::GetMaxDownloadInBytesPerSec() {
	//dont be a Lam3r :)
	return ((uint64)GetMaxDownload() * 1024);
}
//Xman end

// -khaos--+++> A whole bunch of methods!  Keep going until you reach the end tag.
void CPreferences::SaveStats(int bBackUp){
	// This function saves all of the new statistics in my addon.  It is also used to
	// save backups for the Reset Stats function, and the Restore Stats function (Which is actually LoadStats)
	// bBackUp = 0: DEFAULT; save to statistics.ini
	// bBackUp = 1: Save to statbkup.ini, which is used to restore after a reset
	// bBackUp = 2: Save to statbkuptmp.ini, which is temporarily created during a restore and then renamed to statbkup.ini

	CString fullpath(configdir);
	if (bBackUp == 1)
		fullpath += L"statbkup.ini";
	else if (bBackUp == 2)
		fullpath += L"statbkuptmp.ini";
	else
		fullpath += L"statistics.ini";

	CIni ini(fullpath, L"Statistics");

	// Save cumulative statistics to preferences.ini, going in order as they appear in CStatisticsDlg::ShowStatistics.
	// We do NOT SET the values in prefs struct here.

    // Save Cum Down Data
	ini.WriteUInt64(L"TotalDownloadedBytes", theApp.pBandWidthControl->GeteMuleIn()+GetTotalDownloaded()); // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	ini.WriteInt(L"DownSuccessfulSessions", cumDownSuccessfulSessions);
	ini.WriteInt(L"DownFailedSessions", cumDownFailedSessions);
	ini.WriteInt(L"DownAvgTime", (GetDownC_AvgTime() + GetDownS_AvgTime()) / 2);
	ini.WriteUInt64(L"LostFromCorruption", cumLostFromCorruption + sesLostFromCorruption);
	ini.WriteUInt64(L"SavedFromCompression", sesSavedFromCompression + cumSavedFromCompression);
	ini.WriteInt(L"PartsSavedByICH", cumPartsSavedByICH + sesPartsSavedByICH);

	ini.WriteUInt64(L"DownData_EDONKEY", GetCumDownData_EDONKEY());
	ini.WriteUInt64(L"DownData_EDONKEYHYBRID", GetCumDownData_EDONKEYHYBRID());
	ini.WriteUInt64(L"DownData_EMULE", GetCumDownData_EMULE());
	ini.WriteUInt64(L"DownData_MLDONKEY", GetCumDownData_MLDONKEY());
	ini.WriteUInt64(L"DownData_LMULE", GetCumDownData_EMULECOMPAT());
	ini.WriteUInt64(L"DownData_AMULE", GetCumDownData_AMULE());
	ini.WriteUInt64(L"DownData_SHAREAZA", GetCumDownData_SHAREAZA());
	ini.WriteUInt64(L"DownData_URL", GetCumDownData_URL());
	ini.WriteUInt64(L"DownDataPort_4662", GetCumDownDataPort_4662());
	ini.WriteUInt64(L"DownDataPort_OTHER", GetCumDownDataPort_OTHER());
	ini.WriteUInt64(L"DownDataPort_PeerCache", GetCumDownDataPort_PeerCache());

	ini.WriteUInt64(L"DownOverheadTotal",theStats.GetDownDataOverheadFileRequest() +
										theStats.GetDownDataOverheadSourceExchange() +
										theStats.GetDownDataOverheadServer() +
										theStats.GetDownDataOverheadKad() +
										theStats.GetDownDataOverheadOther() +
										GetDownOverheadTotal());
	ini.WriteUInt64(L"DownOverheadFileReq", theStats.GetDownDataOverheadFileRequest() + GetDownOverheadFileReq());
	ini.WriteUInt64(L"DownOverheadSrcEx", theStats.GetDownDataOverheadSourceExchange() + GetDownOverheadSrcEx());
	ini.WriteUInt64(L"DownOverheadServer", theStats.GetDownDataOverheadServer() + GetDownOverheadServer());
	ini.WriteUInt64(L"DownOverheadKad", theStats.GetDownDataOverheadKad() + GetDownOverheadKad());

	ini.WriteUInt64(L"DownOverheadTotalPackets", theStats.GetDownDataOverheadFileRequestPackets() +
												theStats.GetDownDataOverheadSourceExchangePackets() +
												theStats.GetDownDataOverheadServerPackets() +
												theStats.GetDownDataOverheadKadPackets() +
												theStats.GetDownDataOverheadOtherPackets() +
												GetDownOverheadTotalPackets());
	ini.WriteUInt64(L"DownOverheadFileReqPackets", theStats.GetDownDataOverheadFileRequestPackets() + GetDownOverheadFileReqPackets());
	ini.WriteUInt64(L"DownOverheadSrcExPackets", theStats.GetDownDataOverheadSourceExchangePackets() + GetDownOverheadSrcExPackets());
	ini.WriteUInt64(L"DownOverheadServerPackets", theStats.GetDownDataOverheadServerPackets() + GetDownOverheadServerPackets());
	ini.WriteUInt64(L"DownOverheadKadPackets", theStats.GetDownDataOverheadKadPackets() + GetDownOverheadKadPackets());

	// Save Cumulative Upline Statistics
	ini.WriteUInt64(L"TotalUploadedBytes", theApp.pBandWidthControl->GeteMuleOut()+GetTotalUploaded()); // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	ini.WriteInt(L"UpSuccessfulSessions", theApp.uploadqueue->GetSuccessfullUpCount() + GetUpSuccessfulSessions());
	ini.WriteInt(L"UpFailedSessions", theApp.uploadqueue->GetFailedUpCount() + GetUpFailedSessions());
	ini.WriteInt(L"UpAvgTime", (theApp.uploadqueue->GetAverageUpTime() + GetUpAvgTime())/2);
	ini.WriteUInt64(L"UpData_EDONKEY", GetCumUpData_EDONKEY());
	ini.WriteUInt64(L"UpData_EDONKEYHYBRID", GetCumUpData_EDONKEYHYBRID());
	ini.WriteUInt64(L"UpData_EMULE", GetCumUpData_EMULE());
	ini.WriteUInt64(L"UpData_MLDONKEY", GetCumUpData_MLDONKEY());
	ini.WriteUInt64(L"UpData_LMULE", GetCumUpData_EMULECOMPAT());
	ini.WriteUInt64(L"UpData_AMULE", GetCumUpData_AMULE());
	ini.WriteUInt64(L"UpData_SHAREAZA", GetCumUpData_SHAREAZA());
	ini.WriteUInt64(L"UpDataPort_4662", GetCumUpDataPort_4662());
	ini.WriteUInt64(L"UpDataPort_OTHER", GetCumUpDataPort_OTHER());
	ini.WriteUInt64(L"UpDataPort_PeerCache", GetCumUpDataPort_PeerCache());
	ini.WriteUInt64(L"UpData_File", GetCumUpData_File());
	ini.WriteUInt64(L"UpData_Partfile", GetCumUpData_Partfile());

	ini.WriteUInt64(L"UpOverheadTotal", theStats.GetUpDataOverheadFileRequest() +
										theStats.GetUpDataOverheadSourceExchange() +
										theStats.GetUpDataOverheadServer() +
										theStats.GetUpDataOverheadKad() +
										theStats.GetUpDataOverheadOther() +
										GetUpOverheadTotal());
	ini.WriteUInt64(L"UpOverheadFileReq", theStats.GetUpDataOverheadFileRequest() + GetUpOverheadFileReq());
	ini.WriteUInt64(L"UpOverheadSrcEx", theStats.GetUpDataOverheadSourceExchange() + GetUpOverheadSrcEx());
	ini.WriteUInt64(L"UpOverheadServer", theStats.GetUpDataOverheadServer() + GetUpOverheadServer());
	ini.WriteUInt64(L"UpOverheadKad", theStats.GetUpDataOverheadKad() + GetUpOverheadKad());

	ini.WriteUInt64(L"UpOverheadTotalPackets", theStats.GetUpDataOverheadFileRequestPackets() +
										theStats.GetUpDataOverheadSourceExchangePackets() +
										theStats.GetUpDataOverheadServerPackets() +
										theStats.GetUpDataOverheadKadPackets() +
										theStats.GetUpDataOverheadOtherPackets() +
										GetUpOverheadTotalPackets());
	ini.WriteUInt64(L"UpOverheadFileReqPackets", theStats.GetUpDataOverheadFileRequestPackets() + GetUpOverheadFileReqPackets());
	ini.WriteUInt64(L"UpOverheadSrcExPackets", theStats.GetUpDataOverheadSourceExchangePackets() + GetUpOverheadSrcExPackets());
	ini.WriteUInt64(L"UpOverheadServerPackets", theStats.GetUpDataOverheadServerPackets() + GetUpOverheadServerPackets());
	ini.WriteUInt64(L"UpOverheadKadPackets", theStats.GetUpDataOverheadKadPackets() + GetUpOverheadKadPackets());

	// Save Cumulative Connection Statistics
	float tempRate = 0.0F;

	// Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	uint32 eMuleIn;
	uint32 eMuleOut;
	uint32 notUsed;
	theApp.pBandWidthControl->GetDatarates(GetDatarateSamples(),
		eMuleIn, notUsed,
		eMuleOut, notUsed,
		notUsed, notUsed);

	// Download Rate Average
	tempRate = theStats.GetSessionAvgDownloadRate();
	ini.WriteFloat(L"ConnAvgDownRate", tempRate);

	// Max Download Rate Average
	if (tempRate > GetConnMaxAvgDownRate())
		SetConnMaxAvgDownRate(tempRate);
	ini.WriteFloat(L"ConnMaxAvgDownRate", GetConnMaxAvgDownRate());

	// Max Download Rate
	tempRate = (float)eMuleIn / 1024.0f;
	if (tempRate > GetConnMaxDownRate())
		SetConnMaxDownRate(tempRate);
	ini.WriteFloat(L"ConnMaxDownRate", GetConnMaxDownRate());

	// Upload Rate Average
	tempRate = theStats.GetSessionAvgUploadRate();
	ini.WriteFloat(L"ConnAvgUpRate", tempRate);

	// Max Upload Rate Average
	if (tempRate > GetConnMaxAvgUpRate())
		SetConnMaxAvgUpRate(tempRate);
	ini.WriteFloat(L"ConnMaxAvgUpRate", GetConnMaxAvgUpRate());

	// Max Upload Rate
	tempRate = (float)eMuleOut / 1024.0f;
	if (tempRate > GetConnMaxUpRate())
		SetConnMaxUpRate(tempRate);
	ini.WriteFloat(L"ConnMaxUpRate", GetConnMaxUpRate());
	//Xman end

	// Overall Run Time
	ini.WriteInt(L"ConnRunTime", (UINT)((GetTickCount() - theStats.starttime)/1000 + GetConnRunTime()));

	// Number of Reconnects
	ini.WriteInt(L"ConnNumReconnects", (theStats.reconnects>0) ? (theStats.reconnects - 1 + GetConnNumReconnects()) : GetConnNumReconnects());

	// Average Connections
	if (theApp.serverconnect->IsConnected())
		ini.WriteInt(L"ConnAvgConnections", (UINT)((theApp.listensocket->GetAverageConnections() + cumConnAvgConnections)/2));

	// Peak Connections
	if (theApp.listensocket->GetPeakConnections() > cumConnPeakConnections)
		cumConnPeakConnections = theApp.listensocket->GetPeakConnections();
	ini.WriteInt(L"ConnPeakConnections", cumConnPeakConnections);

	// Max Connection Limit Reached
	if (theApp.listensocket->GetMaxConnectionReached() + cumConnMaxConnLimitReached > cumConnMaxConnLimitReached)
		ini.WriteInt(L"ConnMaxConnLimitReached", theApp.listensocket->GetMaxConnectionReached() + cumConnMaxConnLimitReached);

	// Time Stuff...
	ini.WriteInt(L"ConnTransferTime", GetConnTransferTime() + theStats.GetTransferTime());
	ini.WriteInt(L"ConnUploadTime", GetConnUploadTime() + theStats.GetUploadTime());
	ini.WriteInt(L"ConnDownloadTime", GetConnDownloadTime() + theStats.GetDownloadTime());
	ini.WriteInt(L"ConnServerDuration", GetConnServerDuration() + theStats.GetServerDuration());

	// Compare and Save Server Records
	uint32 servtotal, servfail, servuser, servfile, servlowiduser, servtuser, servtfile;
	float servocc;
	theApp.serverlist->GetStatus(servtotal, servfail, servuser, servfile, servlowiduser, servtuser, servtfile, servocc);

	if (servtotal - servfail > cumSrvrsMostWorkingServers)
		cumSrvrsMostWorkingServers = servtotal - servfail;
	ini.WriteInt(L"SrvrsMostWorkingServers", cumSrvrsMostWorkingServers);

	if (servtuser > cumSrvrsMostUsersOnline)
		cumSrvrsMostUsersOnline = servtuser;
	ini.WriteInt(L"SrvrsMostUsersOnline", cumSrvrsMostUsersOnline);

	if (servtfile > cumSrvrsMostFilesAvail)
		cumSrvrsMostFilesAvail = servtfile;
	ini.WriteInt(L"SrvrsMostFilesAvail", cumSrvrsMostFilesAvail);

	// Compare and Save Shared File Records
	if ((UINT)theApp.sharedfiles->GetCount() > cumSharedMostFilesShared)
		cumSharedMostFilesShared = theApp.sharedfiles->GetCount();
	ini.WriteInt(L"SharedMostFilesShared", cumSharedMostFilesShared);

	uint64 bytesLargestFile = 0;
	uint64 allsize = theApp.sharedfiles->GetDatasize(bytesLargestFile);
	if (allsize > cumSharedLargestShareSize)
		cumSharedLargestShareSize = allsize;
	ini.WriteUInt64(L"SharedLargestShareSize", cumSharedLargestShareSize);
	if (bytesLargestFile > cumSharedLargestFileSize)
		cumSharedLargestFileSize = bytesLargestFile;
	ini.WriteUInt64(L"SharedLargestFileSize", cumSharedLargestFileSize);

	if (theApp.sharedfiles->GetCount() != 0) {
		uint64 tempint = allsize/theApp.sharedfiles->GetCount();
		if (tempint > cumSharedLargestAvgFileSize)
			cumSharedLargestAvgFileSize = tempint;
	}

	ini.WriteUInt64(L"SharedLargestAvgFileSize", cumSharedLargestAvgFileSize);
	ini.WriteInt(L"statsDateTimeLastReset", stat_datetimeLastReset);

	// If we are saving a back-up or a temporary back-up, return now.
	if (bBackUp != 0)
		return;
}

void CPreferences::SetRecordStructMembers() {

	// The purpose of this function is to be called from CStatisticsDlg::ShowStatistics()
	// This was easier than making a bunch of functions to interface with the record
	// members of the prefs struct from ShowStatistics.

	// This function is going to compare current values with previously saved records, and if
	// the current values are greater, the corresponding member of prefs will be updated.
	// We will not write to INI here, because this code is going to be called a lot more often
	// than SaveStats()  - Khaos

	CString buffer;

	// Servers
	uint32 servtotal, servfail, servuser, servfile, servlowiduser, servtuser, servtfile;
	float servocc;
	theApp.serverlist->GetStatus( servtotal, servfail, servuser, servfile, servlowiduser, servtuser, servtfile, servocc );
	if ((servtotal-servfail)>cumSrvrsMostWorkingServers) cumSrvrsMostWorkingServers = (servtotal-servfail);
	if (servtuser>cumSrvrsMostUsersOnline) cumSrvrsMostUsersOnline = servtuser;
	if (servtfile>cumSrvrsMostFilesAvail) cumSrvrsMostFilesAvail = servtfile;

	// Shared Files
	if ((UINT)theApp.sharedfiles->GetCount() > cumSharedMostFilesShared)
		cumSharedMostFilesShared = theApp.sharedfiles->GetCount();
	uint64 bytesLargestFile = 0;
	uint64 allsize=theApp.sharedfiles->GetDatasize(bytesLargestFile);
	if (allsize>cumSharedLargestShareSize) cumSharedLargestShareSize = allsize;
	if (bytesLargestFile>cumSharedLargestFileSize) cumSharedLargestFileSize = bytesLargestFile;
	if (theApp.sharedfiles->GetCount() != 0) {
		uint64 tempint = allsize/theApp.sharedfiles->GetCount();
		if (tempint>cumSharedLargestAvgFileSize) cumSharedLargestAvgFileSize = tempint;
	}
} // SetRecordStructMembers()

void CPreferences::SaveCompletedDownloadsStat(){

	// This function saves the values for the completed
	// download members to INI.  It is called from
	// CPartfile::PerformFileComplete ...   - Khaos

	TCHAR* fullpath = new TCHAR[_tcslen(configdir)+MAX_PATH]; // i_a
	_stprintf(fullpath,L"%sstatistics.ini",configdir);

	CIni ini( fullpath, L"Statistics" );

	delete[] fullpath;

	ini.WriteInt(L"DownCompletedFiles",			GetDownCompletedFiles());
	ini.WriteInt(L"DownSessionCompletedFiles",	GetDownSessionCompletedFiles());
} // SaveCompletedDownloadsStat()

void CPreferences::Add2SessionTransferData(UINT uClientID, UINT uClientPort, BOOL bFromPF,
										   BOOL bUpDown, uint32 bytes, bool sentToFriend)
{
	//	This function adds the transferred bytes to the appropriate variables,
	//	as well as to the totals for all clients. - Khaos
	//	PARAMETERS:
	//	uClientID - The identifier for which client software sent or received this data, eg SO_EMULE
	//	uClientPort - The remote port of the client that sent or received this data, eg 4662
	//	bFromPF - Applies only to uploads.  True is from partfile, False is from non-partfile.
	//	bUpDown - True is Up, False is Down
	//	bytes - Number of bytes sent by the client.  Subtract header before calling.

	switch (bUpDown){
		case true:
			//	Upline Data
			switch (uClientID){
				// Update session client breakdown stats for sent bytes...
				case SO_EMULE:
				case SO_OLDEMULE:		sesUpData_EMULE+=bytes;			break;
				case SO_EDONKEYHYBRID:	sesUpData_EDONKEYHYBRID+=bytes;	break;
				case SO_EDONKEY:		sesUpData_EDONKEY+=bytes;		break;
				case SO_MLDONKEY:		sesUpData_MLDONKEY+=bytes;		break;
				case SO_AMULE:			sesUpData_AMULE+=bytes;			break;
				case SO_SHAREAZA:		sesUpData_SHAREAZA+=bytes;		break;
				case SO_CDONKEY:
				case SO_LPHANT:
				case SO_XMULE:			sesUpData_EMULECOMPAT+=bytes;	break;
			}

			switch (uClientPort){
				// Update session port breakdown stats for sent bytes...
				case 4662:				sesUpDataPort_4662+=bytes;		break;
				case (UINT)-1:			sesUpDataPort_PeerCache+=bytes;	break;
				//case (UINT)-2:		sesUpDataPort_URL+=bytes;		break;
				default:				sesUpDataPort_OTHER+=bytes;		break;
			}

			if (bFromPF)				sesUpData_Partfile+=bytes;
			else						sesUpData_File+=bytes;

			//	Add to our total for sent bytes...
			theApp.UpdateSentBytes(bytes, sentToFriend);

			break;

		case false:
			// Downline Data
			switch (uClientID){
                // Update session client breakdown stats for received bytes...
				case SO_EMULE:
				case SO_OLDEMULE:		sesDownData_EMULE+=bytes;		break;
				case SO_EDONKEYHYBRID:	sesDownData_EDONKEYHYBRID+=bytes;break;
				case SO_EDONKEY:		sesDownData_EDONKEY+=bytes;		break;
				case SO_MLDONKEY:		sesDownData_MLDONKEY+=bytes;	break;
				case SO_AMULE:			sesDownData_AMULE+=bytes;		break;
				case SO_SHAREAZA:		sesDownData_SHAREAZA+=bytes;	break;
				case SO_CDONKEY:
				case SO_LPHANT:
				case SO_XMULE:			sesDownData_EMULECOMPAT+=bytes;	break;
				case SO_URL:			sesDownData_URL+=bytes;			break;
			}

			switch (uClientPort){
				// Update session port breakdown stats for received bytes...
				// For now we are only going to break it down by default and non-default.
				// A statistical analysis of all data sent from every single port/domain is
				// beyond the scope of this add-on.
				case 4662:				sesDownDataPort_4662+=bytes;	break;
				case (UINT)-1:			sesDownDataPort_PeerCache+=bytes;break;
				//case (UINT)-2:		sesDownDataPort_URL+=bytes;		break;
				default:				sesDownDataPort_OTHER+=bytes;	break;
			}

			//	Add to our total for received bytes...
			theApp.UpdateReceivedBytes(bytes);
	}
}

// Reset Statistics by Khaos

void CPreferences::ResetCumulativeStatistics(){

	// Save a backup so that we can undo this action
	SaveStats(1);

	// SET ALL CUMULATIVE STAT VALUES TO 0  :'-(

	totalDownloadedBytes=0;
	totalUploadedBytes=0;
	cumDownOverheadTotal=0;
	cumDownOverheadFileReq=0;
	cumDownOverheadSrcEx=0;
	cumDownOverheadServer=0;
	cumDownOverheadKad=0;
	cumDownOverheadTotalPackets=0;
	cumDownOverheadFileReqPackets=0;
	cumDownOverheadSrcExPackets=0;
	cumDownOverheadServerPackets=0;
	cumDownOverheadKadPackets=0;
	cumUpOverheadTotal=0;
	cumUpOverheadFileReq=0;
	cumUpOverheadSrcEx=0;
	cumUpOverheadServer=0;
	cumUpOverheadKad=0;
	cumUpOverheadTotalPackets=0;
	cumUpOverheadFileReqPackets=0;
	cumUpOverheadSrcExPackets=0;
	cumUpOverheadServerPackets=0;
	cumUpOverheadKadPackets=0;
	cumUpSuccessfulSessions=0;
	cumUpFailedSessions=0;
	cumUpAvgTime=0;
	cumUpData_EDONKEY=0;
	cumUpData_EDONKEYHYBRID=0;
	cumUpData_EMULE=0;
	cumUpData_MLDONKEY=0;
	cumUpData_AMULE=0;
	cumUpData_EMULECOMPAT=0;
	cumUpData_SHAREAZA=0;
	cumUpDataPort_4662=0;
	cumUpDataPort_OTHER=0;
	cumUpDataPort_PeerCache=0;
	cumDownCompletedFiles=0;
	cumDownSuccessfulSessions=0;
	cumDownFailedSessions=0;
	cumDownAvgTime=0;
	cumLostFromCorruption=0;
	cumSavedFromCompression=0;
	cumPartsSavedByICH=0;
	cumDownData_EDONKEY=0;
	cumDownData_EDONKEYHYBRID=0;
	cumDownData_EMULE=0;
	cumDownData_MLDONKEY=0;
	cumDownData_AMULE=0;
	cumDownData_EMULECOMPAT=0;
	cumDownData_SHAREAZA=0;
	cumDownData_URL=0;
	cumDownDataPort_4662=0;
	cumDownDataPort_OTHER=0;
	cumDownDataPort_PeerCache=0;
	cumConnAvgDownRate=0;
	cumConnMaxAvgDownRate=0;
	cumConnMaxDownRate=0;
	cumConnAvgUpRate=0;
	cumConnRunTime=0;
	cumConnNumReconnects=0;
	cumConnAvgConnections=0;
	cumConnMaxConnLimitReached=0;
	cumConnPeakConnections=0;
	cumConnDownloadTime=0;
	cumConnUploadTime=0;
	cumConnTransferTime=0;
	cumConnServerDuration=0;
	cumConnMaxAvgUpRate=0;
	cumConnMaxUpRate=0;
	cumSrvrsMostWorkingServers=0;
	cumSrvrsMostUsersOnline=0;
	cumSrvrsMostFilesAvail=0;
    cumSharedMostFilesShared=0;
	cumSharedLargestShareSize=0;
	cumSharedLargestAvgFileSize=0;

	// Set the time of last reset...
	time_t timeNow;
	time(&timeNow);
	stat_datetimeLastReset = timeNow;

	// Save the reset stats
	SaveStats();
	theApp.emuledlg->statisticswnd->ShowStatistics(true);
}


// Load Statistics
// This used to be integrated in LoadPreferences, but it has been altered
// so that it can be used to load the backup created when the stats are reset.
// Last Modified: 2-22-03 by Khaos
bool CPreferences::LoadStats(int loadBackUp)
{
	// loadBackUp is 0 by default
	// loadBackUp = 0: Load the stats normally like we used to do in LoadPreferences
	// loadBackUp = 1: Load the stats from statbkup.ini and create a backup of the current stats.  Also, do not initialize session variables.
	CString sINI;
	CFileFind findBackUp;

	switch (loadBackUp) {
		case 0:{
			// for transition...
			if(PathFileExists(configdir+L"statistics.ini"))
				sINI.Format(L"%sstatistics.ini", configdir);
			else
				sINI.Format(L"%spreferences.ini", configdir);

			break;
			   }
		case 1:
			sINI.Format(L"%sstatbkup.ini", configdir);
			if (!findBackUp.FindFile(sINI))
				return false;
			SaveStats(2); // Save our temp backup of current values to statbkuptmp.ini, we will be renaming it at the end of this function.
			break;
	}

	BOOL fileex = PathFileExists(sINI);
	CIni ini(sINI, L"Statistics");

	totalDownloadedBytes			= ini.GetUInt64(L"TotalDownloadedBytes");
	totalUploadedBytes				= ini.GetUInt64(L"TotalUploadedBytes");

	// Load stats for cumulative downline overhead
	cumDownOverheadTotal			= ini.GetUInt64(L"DownOverheadTotal");
	cumDownOverheadFileReq			= ini.GetUInt64(L"DownOverheadFileReq");
	cumDownOverheadSrcEx			= ini.GetUInt64(L"DownOverheadSrcEx");
	cumDownOverheadServer			= ini.GetUInt64(L"DownOverheadServer");
	cumDownOverheadKad				= ini.GetUInt64(L"DownOverheadKad");
	cumDownOverheadTotalPackets		= ini.GetUInt64(L"DownOverheadTotalPackets");
	cumDownOverheadFileReqPackets	= ini.GetUInt64(L"DownOverheadFileReqPackets");
	cumDownOverheadSrcExPackets		= ini.GetUInt64(L"DownOverheadSrcExPackets");
	cumDownOverheadServerPackets	= ini.GetUInt64(L"DownOverheadServerPackets");
	cumDownOverheadKadPackets		= ini.GetUInt64(L"DownOverheadKadPackets");

	// Load stats for cumulative upline overhead
	cumUpOverheadTotal				= ini.GetUInt64(L"UpOverHeadTotal");
	cumUpOverheadFileReq			= ini.GetUInt64(L"UpOverheadFileReq");
	cumUpOverheadSrcEx				= ini.GetUInt64(L"UpOverheadSrcEx");
	cumUpOverheadServer				= ini.GetUInt64(L"UpOverheadServer");
	cumUpOverheadKad				= ini.GetUInt64(L"UpOverheadKad");
	cumUpOverheadTotalPackets		= ini.GetUInt64(L"UpOverHeadTotalPackets");
	cumUpOverheadFileReqPackets		= ini.GetUInt64(L"UpOverheadFileReqPackets");
	cumUpOverheadSrcExPackets		= ini.GetUInt64(L"UpOverheadSrcExPackets");
	cumUpOverheadServerPackets		= ini.GetUInt64(L"UpOverheadServerPackets");
	cumUpOverheadKadPackets			= ini.GetUInt64(L"UpOverheadKadPackets");

	// Load stats for cumulative upline data
	cumUpSuccessfulSessions			= ini.GetInt(L"UpSuccessfulSessions");
	cumUpFailedSessions				= ini.GetInt(L"UpFailedSessions");
	cumUpAvgTime					= ini.GetInt(L"UpAvgTime");

	// Load cumulative client breakdown stats for sent bytes
	cumUpData_EDONKEY				= ini.GetUInt64(L"UpData_EDONKEY");
	cumUpData_EDONKEYHYBRID			= ini.GetUInt64(L"UpData_EDONKEYHYBRID");
	cumUpData_EMULE					= ini.GetUInt64(L"UpData_EMULE");
	cumUpData_MLDONKEY				= ini.GetUInt64(L"UpData_MLDONKEY");
	cumUpData_EMULECOMPAT			= ini.GetUInt64(L"UpData_LMULE");
	cumUpData_AMULE					= ini.GetUInt64(L"UpData_AMULE");
	cumUpData_SHAREAZA				= ini.GetUInt64(L"UpData_SHAREAZA");

	// Load cumulative port breakdown stats for sent bytes
	cumUpDataPort_4662				= ini.GetUInt64(L"UpDataPort_4662");
	cumUpDataPort_OTHER				= ini.GetUInt64(L"UpDataPort_OTHER");
	cumUpDataPort_PeerCache			= ini.GetUInt64(L"UpDataPort_PeerCache");

	// Load cumulative source breakdown stats for sent bytes
	cumUpData_File					= ini.GetUInt64(L"UpData_File");
	cumUpData_Partfile				= ini.GetUInt64(L"UpData_Partfile");

	// Load stats for cumulative downline data
	cumDownCompletedFiles			= ini.GetInt(L"DownCompletedFiles");
	cumDownSuccessfulSessions		= ini.GetInt(L"DownSuccessfulSessions");
	cumDownFailedSessions			= ini.GetInt(L"DownFailedSessions");
	cumDownAvgTime					= ini.GetInt(L"DownAvgTime");

	// Cumulative statistics for saved due to compression/lost due to corruption
	cumLostFromCorruption			= ini.GetUInt64(L"LostFromCorruption");
	cumSavedFromCompression			= ini.GetUInt64(L"SavedFromCompression");
	cumPartsSavedByICH				= ini.GetInt(L"PartsSavedByICH");

	// Load cumulative client breakdown stats for received bytes
	cumDownData_EDONKEY				= ini.GetUInt64(L"DownData_EDONKEY");
	cumDownData_EDONKEYHYBRID		= ini.GetUInt64(L"DownData_EDONKEYHYBRID");
	cumDownData_EMULE				= ini.GetUInt64(L"DownData_EMULE");
	cumDownData_MLDONKEY			= ini.GetUInt64(L"DownData_MLDONKEY");
	cumDownData_EMULECOMPAT			= ini.GetUInt64(L"DownData_LMULE");
	cumDownData_AMULE				= ini.GetUInt64(L"DownData_AMULE");
	cumDownData_SHAREAZA			= ini.GetUInt64(L"DownData_SHAREAZA");
	cumDownData_URL					= ini.GetUInt64(L"DownData_URL");

	// Load cumulative port breakdown stats for received bytes
	cumDownDataPort_4662			= ini.GetUInt64(L"DownDataPort_4662");
	cumDownDataPort_OTHER			= ini.GetUInt64(L"DownDataPort_OTHER");
	cumDownDataPort_PeerCache		= ini.GetUInt64(L"DownDataPort_PeerCache");

	// Load stats for cumulative connection data
	cumConnAvgDownRate				= ini.GetFloat(L"ConnAvgDownRate");
	if(cumConnAvgDownRate<0) cumConnAvgDownRate=0; //Xman prevent floating point underun
	cumConnMaxAvgDownRate			= ini.GetFloat(L"ConnMaxAvgDownRate");
	cumConnMaxDownRate				= ini.GetFloat(L"ConnMaxDownRate");
	cumConnAvgUpRate				= ini.GetFloat(L"ConnAvgUpRate");
	if(cumConnAvgUpRate<0) cumConnAvgUpRate=0; //Xman prevent floating point underun
	cumConnMaxAvgUpRate				= ini.GetFloat(L"ConnMaxAvgUpRate");
	cumConnMaxUpRate				= ini.GetFloat(L"ConnMaxUpRate");
	cumConnRunTime					= ini.GetInt(L"ConnRunTime");
	cumConnTransferTime				= ini.GetInt(L"ConnTransferTime");
	cumConnDownloadTime				= ini.GetInt(L"ConnDownloadTime");
	cumConnUploadTime				= ini.GetInt(L"ConnUploadTime");
	cumConnServerDuration			= ini.GetInt(L"ConnServerDuration");
	cumConnNumReconnects			= ini.GetInt(L"ConnNumReconnects");
	cumConnAvgConnections			= ini.GetInt(L"ConnAvgConnections");
	cumConnMaxConnLimitReached		= ini.GetInt(L"ConnMaxConnLimitReached");
	cumConnPeakConnections			= ini.GetInt(L"ConnPeakConnections");

	// Load date/time of last reset
	stat_datetimeLastReset			= ini.GetInt(L"statsDateTimeLastReset");

	// Smart Load For Restores - Don't overwrite records that are greater than the backed up ones
	if (loadBackUp == 1)
	{
		// Load records for servers / network
		if ((UINT)ini.GetInt(L"SrvrsMostWorkingServers") > cumSrvrsMostWorkingServers)
			cumSrvrsMostWorkingServers = ini.GetInt(L"SrvrsMostWorkingServers");

		if ((UINT)ini.GetInt(L"SrvrsMostUsersOnline") > cumSrvrsMostUsersOnline)
			cumSrvrsMostUsersOnline = ini.GetInt(L"SrvrsMostUsersOnline");

		if ((UINT)ini.GetInt(L"SrvrsMostFilesAvail") > cumSrvrsMostFilesAvail)
			cumSrvrsMostFilesAvail = ini.GetInt(L"SrvrsMostFilesAvail");

		// Load records for shared files
		if ((UINT)ini.GetInt(L"SharedMostFilesShared") > cumSharedMostFilesShared)
			cumSharedMostFilesShared =	ini.GetInt(L"SharedMostFilesShared");

		uint64 temp64 = ini.GetUInt64(L"SharedLargestShareSize");
		if (temp64 > cumSharedLargestShareSize)
			cumSharedLargestShareSize = temp64;

		temp64 = ini.GetUInt64(L"SharedLargestAvgFileSize");
		if (temp64 > cumSharedLargestAvgFileSize)
			cumSharedLargestAvgFileSize = temp64;

		temp64 = ini.GetUInt64(L"SharedLargestFileSize");
		if (temp64 > cumSharedLargestFileSize)
			cumSharedLargestFileSize = temp64;

		// Check to make sure the backup of the values we just overwrote exists.  If so, rename it to the backup file.
		// This allows us to undo a restore, so to speak, just in case we don't like the restored values...
		CString sINIBackUp;
		sINIBackUp.Format(L"%sstatbkuptmp.ini", configdir);
		if (findBackUp.FindFile(sINIBackUp)){
			CFile::Remove(sINI);				// Remove the backup that we just restored from
			CFile::Rename(sINIBackUp, sINI);	// Rename our temporary backup to the normal statbkup.ini filename.
		}

		// Since we know this is a restore, now we should call ShowStatistics to update the data items to the new ones we just loaded.
		// Otherwise user is left waiting around for the tick counter to reach the next automatic update (Depending on setting in prefs)
		theApp.emuledlg->statisticswnd->ShowStatistics();
	}
	// Stupid Load -> Just load the values.
	else
	{
		// Load records for servers / network
		cumSrvrsMostWorkingServers	= ini.GetInt(L"SrvrsMostWorkingServers");
		cumSrvrsMostUsersOnline		= ini.GetInt(L"SrvrsMostUsersOnline");
		cumSrvrsMostFilesAvail		= ini.GetInt(L"SrvrsMostFilesAvail");

		// Load records for shared files
		cumSharedMostFilesShared	= ini.GetInt(L"SharedMostFilesShared");
		cumSharedLargestShareSize	= ini.GetUInt64(L"SharedLargestShareSize");
		cumSharedLargestAvgFileSize = ini.GetUInt64(L"SharedLargestAvgFileSize");
		cumSharedLargestFileSize	= ini.GetUInt64(L"SharedLargestFileSize");

		// Initialize new session statistic variables...
		sesDownCompletedFiles		= 0;

		sesUpData_EDONKEY			= 0;
		sesUpData_EDONKEYHYBRID		= 0;
		sesUpData_EMULE				= 0;
		sesUpData_MLDONKEY			= 0;
		sesUpData_AMULE				= 0;
		sesUpData_EMULECOMPAT		= 0;
		sesUpData_SHAREAZA			= 0;
		sesUpDataPort_4662			= 0;
		sesUpDataPort_OTHER			= 0;
		sesUpDataPort_PeerCache		= 0;

		sesDownData_EDONKEY			= 0;
		sesDownData_EDONKEYHYBRID	= 0;
		sesDownData_EMULE			= 0;
		sesDownData_MLDONKEY		= 0;
		sesDownData_AMULE			= 0;
		sesDownData_EMULECOMPAT		= 0;
		sesDownData_SHAREAZA		= 0;
		sesDownData_URL				= 0;
		sesDownDataPort_4662		= 0;
		sesDownDataPort_OTHER		= 0;
		sesDownDataPort_PeerCache	= 0;

		sesDownSuccessfulSessions	= 0;
		sesDownFailedSessions		= 0;
		sesPartsSavedByICH			= 0;
	}

	if (!fileex || (stat_datetimeLastReset==0 && totalDownloadedBytes==0 && totalUploadedBytes==0))
	{
		time_t timeNow;
		time(&timeNow);
		stat_datetimeLastReset = timeNow;
	}

	return true;
}

// This formats the UTC long value that is saved for stat_datetimeLastReset
// If this value is 0 (Never reset), then it returns Unknown.
CString CPreferences::GetStatsLastResetStr(bool formatLong)
{
	// formatLong dictates the format of the string returned.
	// For example...
	// true: DateTime format from the .ini
	// false: DateTime format from the .ini for the log
	CString	returnStr;
	if (GetStatsLastResetLng()) {
		tm *statsReset;
		TCHAR szDateReset[128];
		time_t lastResetDateTime = (time_t) GetStatsLastResetLng();
		statsReset = localtime(&lastResetDateTime);
		if (statsReset){
			_tcsftime(szDateReset, ARRSIZE(szDateReset), formatLong ? GetDateTimeFormat() : L"%c", statsReset);
			returnStr = szDateReset;
		}
	}
	if (returnStr.IsEmpty())
		returnStr = GetResString(IDS_UNKNOWN);
	return returnStr;
}

// <-----khaos-

bool CPreferences::Save(){

	bool error = false;
	TCHAR* fullpath = new TCHAR[_tcslen(configdir)+MAX_PATH]; // i_a
	_stprintf(fullpath,L"%spreferences.dat",configdir);

	FILE* preffile = _tfsopen(fullpath,L"wb", _SH_DENYWR);
	delete[] fullpath;
	prefsExt->version = PREFFILE_VERSION;
	if (preffile){
		prefsExt->version=PREFFILE_VERSION;
		prefsExt->EmuleWindowPlacement=EmuleWindowPlacement;
		md4cpy(prefsExt->userhash, userhash);

		error = fwrite(prefsExt,sizeof(Preferences_Ext_Struct),1,preffile)!=1;
		if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !theApp.emuledlg->IsRunning())){
			fflush(preffile); // flush file stream buffers to disk buffers
			(void)_commit(_fileno(preffile)); // commit disk buffers to disk
		}
		fclose(preffile);
	}
	else
		error = true;

	SavePreferences();
	SaveStats();

	fullpath = new TCHAR[_tcslen(configdir) + 14];
	_stprintf(fullpath, L"%sshareddir.dat", configdir);
	CStdioFile sdirfile;
	if (sdirfile.Open(fullpath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite | CFile::typeBinary))
	{
		try{
			// write Unicode byte-order mark 0xFEFF
			WORD wBOM = 0xFEFF;
			sdirfile.Write(&wBOM, sizeof(wBOM));

			for (POSITION pos = shareddir_list.GetHeadPosition();pos != 0;){
				sdirfile.WriteString(shareddir_list.GetNext(pos));
				sdirfile.Write(L"\r\n", sizeof(TCHAR)*2);
			}
			if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !theApp.emuledlg->IsRunning())){
				sdirfile.Flush(); // flush file stream buffers to disk buffers
				if (_commit(_fileno(sdirfile.m_pStream)) != 0) // commit disk buffers to disk
					AfxThrowFileException(CFileException::hardIO, GetLastError(), sdirfile.GetFileName());
			}
			sdirfile.Close();
		}
		catch(CFileException* error){
			TCHAR buffer[MAX_CFEXP_ERRORMSG];
			error->GetErrorMessage(buffer,ARRSIZE(buffer));
			if (thePrefs.GetVerbose())
				AddDebugLogLine(true,L"Failed to save %s - %s", fullpath, buffer);
			error->Delete();
		}
	}
	else
		error = true;
	delete[] fullpath;
	fullpath=NULL;

	::CreateDirectory(GetIncomingDir(),0);
	::CreateDirectory(GetTempDir(),0);
	return error;
}

void CPreferences::CreateUserHash()
{
	for (int i = 0; i < 8; i++)
	{
		uint16 random = GetRandomUInt16();
		memcpy(&userhash[i*2], &random, 2);
	}

	// mark as emule client. that will be need in later version
	userhash[5] = 14;
	userhash[14] = 111;
}

int CPreferences::GetRecommendedMaxConnections() {
	int iRealMax = ::GetMaxWindowsTCPConnections();
	if(iRealMax == -1 || iRealMax > 520)
		return 500;

	if(iRealMax < 20)
		return iRealMax;

	if(iRealMax <= 256)
		return iRealMax - 10;

	return iRealMax - 20;
}

void CPreferences::SavePreferences()
{
	CString buffer;

	CIni ini(GetConfigFile(), L"eMule");
	//---
	ini.WriteString(L"AppVersion", theApp.m_strCurVersionLong);
	//---

#ifdef _DEBUG
	ini.WriteInt(L"DebugHeap", m_iDbgHeap);
#endif

	ini.WriteStringUTF8(L"Nick", strNick);
	ini.WriteString(L"IncomingDir", incomingdir);

	ini.WriteString(L"TempDir", tempdir.GetAt(0));

	CString tempdirs;
	for (int i=1;i<tempdir.GetCount();i++) {
		tempdirs.Append(tempdir.GetAt(i) );
		if (i+1<tempdir.GetCount())
			tempdirs.Append(L"|");
	}
	ini.WriteString(L"TempDirs", tempdirs);

    ini.WriteInt(L"MinUpload", minupload);
	//Xman
	// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
	ini.WriteFloat(L"MaxUpload", maxupload);
	ini.WriteFloat(L"MaxDownload", maxdownload);
	ini.WriteFloat(L"DownloadCapacity", maxGraphDownloadRate);
	ini.WriteFloat(L"UploadCapacity", maxGraphUploadRate);
	// Maella end

	ini.WriteInt(L"MaxConnections",maxconnections);
	ini.WriteInt(L"MaxHalfConnections",maxhalfconnections);
	ini.WriteBool(L"ConditionalTCPAccept", m_bConditionalTCPAccept);
	ini.WriteInt(L"Port",port);
	ini.WriteInt(L"UDPPort",udpport);
	ini.WriteInt(L"ServerUDPPort", nServerUDPPort);
	ini.WriteInt(L"MaxSourcesPerFile",maxsourceperfile );
	ini.WriteWORD(L"Language",m_wLanguageID);
	ini.WriteInt(L"SeeShare",m_iSeeShares);
	ini.WriteInt(L"ToolTipDelay",m_iToolDelayTime);
	ini.WriteInt(L"StatGraphsInterval",trafficOMeterInterval);
	ini.WriteInt(L"StatsInterval",statsInterval);
	ini.WriteInt(L"DeadServerRetry",m_uDeadServerRetries);
	ini.WriteInt(L"ServerKeepAliveTimeout",m_dwServerKeepAliveTimeout);
	ini.WriteInt(L"SplitterbarPosition",splitterbarPosition+2);
	ini.WriteInt(L"SplitterbarPositionServer",splitterbarPositionSvr);
	ini.WriteInt(L"SplitterbarPositionStat",splitterbarPositionStat+1);
	ini.WriteInt(L"SplitterbarPositionStat_HL",splitterbarPositionStat_HL); //Xman BlueSonicBoy-Stats-Fix
	ini.WriteInt(L"SplitterbarPositionStat_HR",splitterbarPositionStat_HR); //Xman BlueSonicBoy-Stats-Fix
	ini.WriteInt(L"SplitterbarPositionFriend",splitterbarPositionFriend);
	ini.WriteInt(L"SplitterbarPositionIRC",splitterbarPositionIRC+2);
	ini.WriteInt(L"SplitterbarPositionShared",splitterbarPositionShared);
	ini.WriteInt(L"TransferWnd1",m_uTransferWnd1);
	ini.WriteInt(L"TransferWnd2",m_uTransferWnd2);
	ini.WriteInt(L"VariousStatisticsMaxValue",statsMax);
	ini.WriteInt(L"StatsAverageMinutes",statsAverageMinutes);
	ini.WriteInt(L"MaxConnectionsPerFiveSeconds",MaxConperFive);
	ini.WriteInt(L"Check4NewVersionDelay",versioncheckdays);

	ini.WriteBool(L"Reconnect",reconnect);
	ini.WriteBool(L"Scoresystem",m_bUseServerPriorities);
	ini.WriteBool(L"Serverlist",m_bAutoUpdateServerList);
	ini.WriteBool(L"UpdateNotifyTestClient",updatenotify);
	ini.WriteBool(L"MinToTray",mintotray);
	ini.WriteBool(L"AddServersFromServer",m_bAddServersFromServer);
	ini.WriteBool(L"AddServersFromClient",m_bAddServersFromClients);
	ini.WriteBool(L"Splashscreen",splashscreen);
	ini.WriteBool(L"BringToFront",bringtoforeground);
	ini.WriteBool(L"TransferDoubleClick",transferDoubleclick);
	ini.WriteBool(L"BeepOnError",beepOnError);
	ini.WriteBool(L"ConfirmExit",confirmExit);
	ini.WriteBool(L"FilterBadIPs",filterLANIPs);
    ini.WriteBool(L"Autoconnect",autoconnect);
	ini.WriteBool(L"OnlineSignature",onlineSig);
	ini.WriteBool(L"StartupMinimized",startMinimized);
	ini.WriteBool(L"AutoStart",m_bAutoStart);
	ini.WriteInt(L"LastMainWndDlgID",m_iLastMainWndDlgID);
	ini.WriteInt(L"LastLogPaneID",m_iLastLogPaneID);
	ini.WriteBool(L"SafeServerConnect",m_bSafeServerConnect);
	ini.WriteBool(L"ShowRatesOnTitle",showRatesInTitle);
	ini.WriteBool(L"IndicateRatings",indicateratings);
	ini.WriteBool(L"WatchClipboard4ED2kFilelinks",watchclipboard);
	ini.WriteInt(L"SearchMethod",m_iSearchMethod);
	ini.WriteBool(L"CheckDiskspace",checkDiskspace);
	ini.WriteInt(L"MinFreeDiskSpace",m_uMinFreeDiskSpace);
	ini.WriteBool(L"SparsePartFiles",m_bSparsePartFiles);
	ini.WriteString(L"YourHostname",m_strYourHostname);

	// Barry - New properties...
    ini.WriteBool(L"AutoConnectStaticOnly", m_bAutoConnectToStaticServersOnly);
	ini.WriteBool(L"AutoTakeED2KLinks", autotakeed2klinks);
    ini.WriteBool(L"AddNewFilesPaused", addnewfilespaused);
    ini.WriteInt (L"3DDepth", depth3D);
	ini.WriteBool(L"MiniMule", m_bEnableMiniMule);

	ini.WriteString(L"NotifierConfiguration", notifierConfiguration);
	ini.WriteBool(L"NotifyOnDownload", notifierOnDownloadFinished);
	ini.WriteBool(L"NotifyOnNewDownload", notifierOnNewDownload);
	ini.WriteBool(L"NotifyOnChat", notifierOnChat);
	ini.WriteBool(L"NotifyOnLog", notifierOnLog);
	ini.WriteBool(L"NotifyOnImportantError", notifierOnImportantError);
	ini.WriteBool(L"NotifierPopEveryChatMessage", notifierOnEveryChatMsg);
	ini.WriteBool(L"NotifierPopNewVersion", notifierOnNewVersion);
	ini.WriteInt(L"NotifierUseSound", (int)notifierSoundType);
	ini.WriteString(L"NotifierSoundPath", notifierSoundFile);

	ini.WriteBool(L"ShowActiveDownloadsBold",m_bShowActiveDownloadsBold); //Xman Show active downloads bold

	ini.WriteString(L"TxtEditor",TxtEditor);
	ini.WriteString(L"VideoPlayer",m_strVideoPlayer);
	ini.WriteString(L"VideoPlayerArgs",m_strVideoPlayerArgs);
	ini.WriteString(L"MessageFilter",messageFilter);
	ini.WriteString(L"CommentFilter",commentFilter);
	ini.WriteString(L"DateTimeFormat",GetDateTimeFormat());
	ini.WriteString(L"DateTimeFormat4Log",GetDateTimeFormat4Log());
	ini.WriteString(L"WebTemplateFile",m_sTemplateFile);
	ini.WriteString(L"FilenameCleanups",filenameCleanups);
	ini.WriteInt(L"ExtractMetaData",m_iExtractMetaData);

	ini.WriteString(L"DefaultIRCServerNew",m_sircserver);
	ini.WriteString(L"IRCNick",m_sircnick);
	ini.WriteBool(L"IRCAddTimestamp", m_bircaddtimestamp);
	ini.WriteString(L"IRCFilterName", m_sircchannamefilter);
	ini.WriteInt(L"IRCFilterUser", m_iircchanneluserfilter);
	ini.WriteBool(L"IRCUseFilter", m_bircusechanfilter);
	ini.WriteString(L"IRCPerformString", m_sircperformstring);
	ini.WriteBool(L"IRCUsePerform", m_bircuseperform);
	ini.WriteBool(L"IRCListOnConnect", m_birclistonconnect);
	ini.WriteBool(L"IRCAcceptLink", m_bircacceptlinks);
	ini.WriteBool(L"IRCAcceptLinkFriends", m_bircacceptlinksfriends);
	ini.WriteBool(L"IRCSoundEvents", m_bircsoundevents);
	ini.WriteBool(L"IRCIgnoreMiscMessages", m_bircignoremiscmessage);
	ini.WriteBool(L"IRCIgnoreJoinMessages", m_bircignorejoinmessage);
	ini.WriteBool(L"IRCIgnorePartMessages", m_bircignorepartmessage);
	ini.WriteBool(L"IRCIgnoreQuitMessages", m_bircignorequitmessage);
	ini.WriteBool(L"IRCIgnoreEmuleProtoAddFriend", m_bircignoreemuleprotoaddfriend);
	ini.WriteBool(L"IRCAllowEmuleProtoAddFriend", m_bircallowemuleprotoaddfriend);
	ini.WriteBool(L"IRCIgnoreEmuleProtoSendLink", m_bircignoreemuleprotosendlink);
	ini.WriteBool(L"IRCHelpChannel", m_birchelpchannel);
	ini.WriteBool(L"SmartIdCheck", m_bSmartServerIdCheck);
	ini.WriteBool(L"Verbose", m_bVerbose);
	ini.WriteBool(L"DebugSourceExchange", m_bDebugSourceExchange);	// do *not* use the according 'Get...' function here!
	ini.WriteBool(L"LogBannedClients", m_bLogBannedClients);			// do *not* use the according 'Get...' function here!
	ini.WriteBool(L"LogRatingDescReceived", m_bLogRatingDescReceived);// do *not* use the according 'Get...' function here!
	ini.WriteBool(L"LogSecureIdent", m_bLogSecureIdent);				// do *not* use the according 'Get...' function here!
	ini.WriteBool(L"LogFilteredIPs", m_bLogFilteredIPs);				// do *not* use the according 'Get...' function here!
	ini.WriteBool(L"LogFileSaving", m_bLogFileSaving);				// do *not* use the according 'Get...' function here!
    ini.WriteBool(L"LogA4AF", m_bLogA4AF);                           // do *not* use the according 'Get...' function here!
	ini.WriteBool(L"LogDrop", m_bLogDrop); //Xman Xtreme Downloadmanager
	ini.WriteBool(L"Logpartmismatch", m_bLogpartmismatch); //Xman Log part/size-mismatch
	ini.WriteBool(L"LogUlDlEvents", m_bLogUlDlEvents);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	// following options are for debugging or when using an external debug device viewer only.
	ini.WriteInt(L"DebugServerTCP",m_iDebugServerTCPLevel);
	ini.WriteInt(L"DebugServerUDP",m_iDebugServerUDPLevel);
	ini.WriteInt(L"DebugServerSources",m_iDebugServerSourcesLevel);
	ini.WriteInt(L"DebugServerSearches",m_iDebugServerSearchesLevel);
	ini.WriteInt(L"DebugClientTCP",m_iDebugClientTCPLevel);
	ini.WriteInt(L"DebugClientUDP",m_iDebugClientUDPLevel);
	ini.WriteInt(L"DebugClientKadUDP",m_iDebugClientKadUDPLevel);
#endif
	ini.WriteBool(L"PreviewPrio", m_bpreviewprio);
	ini.WriteBool(L"UpdateQueueListPref", m_bupdatequeuelist);
	ini.WriteBool(L"ManualHighPrio", m_bManualAddedServersHighPriority);
	ini.WriteBool(L"FullChunkTransfers", m_btransferfullchunks);
	ini.WriteBool(L"ShowOverhead", m_bshowoverhead);
	ini.WriteBool(L"VideoPreviewBackupped", moviePreviewBackup);
	ini.WriteInt(L"StartNextFile", m_istartnextfile);

	ini.DeleteKey(L"FileBufferSizePref"); // delete old 'file buff size' setting
	ini.WriteInt(L"FileBufferSize", m_iFileBufferSize);
//>>> WiZaRd::IntelliFlush
	ini.WriteBool(L"IntelliFlush", m_bIntelliFlush);
//<<< WiZaRd::IntelliFlush

	ini.DeleteKey(L"QueueSizePref"); // delete old 'queue size' setting
	ini.WriteInt(L"QueueSize", m_iQueueSize);

	ini.WriteInt(L"CommitFiles", m_iCommitFiles);
	ini.WriteBool(L"DAPPref", m_bDAP);
	ini.WriteBool(L"UAPPref", m_bUAP);
	ini.WriteBool(L"FilterServersByIP",filterserverbyip);
	ini.WriteBool(L"DisableKnownClientList",m_bDisableKnownClientList);
	ini.WriteBool(L"DisableQueueList",m_bDisableQueueList);
	ini.WriteBool(L"UseCreditSystem",m_bCreditSystem);
	ini.WriteBool(L"SaveLogToDisk",log2disk);
	ini.WriteBool(L"SaveDebugToDisk",debug2disk);
	ini.WriteBool(L"EnableScheduler",scheduler);
	ini.WriteBool(L"MessagesFromFriendsOnly",msgonlyfriends);
	ini.WriteBool(L"MessageFromValidSourcesOnly",msgsecure);
	ini.WriteBool(L"ShowInfoOnCatTabs",showCatTabInfos);
	ini.WriteBool(L"DontRecreateStatGraphsOnResize",dontRecreateGraphs);
	ini.WriteBool(L"AutoFilenameCleanup",autofilenamecleanup);
	ini.WriteBool(L"ShowExtControls",m_bExtControls);
	ini.WriteBool(L"UseAutocompletion",m_bUseAutocompl);
	ini.WriteBool(L"NetworkKademlia",networkkademlia);
	ini.WriteBool(L"NetworkED2K",networked2k);
	ini.WriteBool(L"AutoClearCompleted",m_bRemoveFinishedDownloads);
	ini.WriteBool(L"TransflstRemainOrder",m_bTransflstRemain);
	ini.WriteBool(L"UseSimpleTimeRemainingcomputation",m_bUseOldTimeRemaining);
	ini.WriteBool(L"AllocateFullFile",m_bAllocFull);

	ini.WriteInt(L"VersionCheckLastAutomatic", versioncheckLastAutomatic);
	ini.WriteInt(L"FilterLevel",filterlevel);

	ini.WriteBool(L"SecureIdent", m_bUseSecureIdent);// change the name in future version to enable it by default
	ini.WriteBool(L"AdvancedSpamFilter",m_bAdvancedSpamfilter);
	ini.WriteBool(L"ShowDwlPercentage",m_bShowDwlPercentage);
	ini.WriteBool(L"RemoveFilesToBin",m_bRemove2bin);
	//ini.WriteBool(L"ShowCopyEd2kLinkCmd",m_bShowCopyEd2kLinkCmd);
	ini.WriteBool(L"AutoArchivePreviewStart", m_bAutomaticArcPreviewStart);

	// Toolbar
	ini.WriteString(L"ToolbarSetting", m_sToolbarSettings);
	ini.WriteString(L"ToolbarBitmap", m_sToolbarBitmap );
	ini.WriteString(L"ToolbarBitmapFolder", m_sToolbarBitmapFolder);
	ini.WriteInt(L"ToolbarLabels", m_nToolbarLabels);
	ini.WriteInt(L"ToolbarIconSize", m_sizToolbarIconSize.cx);
	ini.WriteString(L"SkinProfile", m_strSkinProfile);
	ini.WriteString(L"SkinProfileDir", m_strSkinProfileDir);

	ini.WriteBinary(L"HyperTextFont", (LPBYTE)&m_lfHyperText, sizeof m_lfHyperText);
	ini.WriteBinary(L"LogTextFont", (LPBYTE)&m_lfLogText, sizeof m_lfLogText);

	// ZZ:UploadSpeedSense -->
    ini.WriteBool(L"USSEnabled", m_bDynUpEnabled);
    ini.WriteBool(L"USSUseMillisecondPingTolerance", m_bDynUpUseMillisecondPingTolerance);
    ini.WriteInt(L"USSPingTolerance", m_iDynUpPingTolerance);
	ini.WriteInt(L"USSPingToleranceMilliseconds", m_iDynUpPingToleranceMilliseconds); // EastShare - Add by TAHO, USS limit
    ini.WriteInt(L"USSGoingUpDivider", m_iDynUpGoingUpDivider);
    ini.WriteInt(L"USSGoingDownDivider", m_iDynUpGoingDownDivider);
    ini.WriteInt(L"USSNumberOfPings", m_iDynUpNumberOfPings);
	// ZZ:UploadSpeedSense <--

    //ini.WriteBool(L"A4AFSaveCpu", m_bA4AFSaveCpu); // ZZ:DownloadManager
    ini.WriteBool(L"HighresTimer", m_bHighresTimer);
	ini.WriteInt(L"WebMirrorAlertLevel", m_nWebMirrorAlertLevel);
	ini.WriteBool(L"RunAsUnprivilegedUser", m_bRunAsUser);
	ini.WriteBool(L"OpenPortsOnStartUp", m_bOpenPortsOnStartUp);
	ini.WriteInt(L"DebugLogLevel", m_byLogLevel);
	ini.WriteInt(L"WinXPSP2", IsRunningXPSP2());
	ini.WriteBool(L"RememberCancelledFiles", m_bRememberCancelledFiles);
	ini.WriteBool(L"RememberDownloadedFiles", m_bRememberDownloadedFiles);

	ini.WriteBool(L"NotifierSendMail", m_bNotifierSendMail);
	ini.WriteString(L"NotifierMailSender", m_strNotifierMailSender);
	ini.WriteString(L"NotifierMailServer", m_strNotifierMailServer);
	ini.WriteString(L"NotifierMailRecipient", m_strNotifierMailReceiver);

	ini.WriteBool(L"WinaTransToolbar", m_bWinaTransToolbar);

	ini.WriteBool(L"CryptLayerRequested", m_bCryptLayerRequested);
	ini.WriteBool(L"CryptLayerRequired", m_bCryptLayerRequired);
	ini.WriteBool(L"CryptLayerSupported", m_bCryptLayerSupported);


	///////////////////////////////////////////////////////////////////////////
	// Section: "Proxy"
	//
	ini.WriteBool(L"ProxyEnablePassword",proxy.EnablePassword,L"Proxy");
	ini.WriteBool(L"ProxyEnableProxy",proxy.UseProxy,L"Proxy");
	ini.WriteString(L"ProxyName",CStringW(proxy.name),L"Proxy");
	ini.WriteString(L"ProxyPassword",CStringW(proxy.password),L"Proxy");
	ini.WriteString(L"ProxyUser",CStringW(proxy.user),L"Proxy");
	ini.WriteInt(L"ProxyPort",proxy.port,L"Proxy");
	ini.WriteInt(L"ProxyType",proxy.type,L"Proxy");


	///////////////////////////////////////////////////////////////////////////
	// Section: "Statistics"
	//
	ini.WriteInt(L"statsConnectionsGraphRatio", statsConnectionsGraphRatio,L"Statistics");
	ini.WriteString(L"statsExpandedTreeItems", statsExpandedTreeItems);
	CString buffer2;
	for (int i=0;i<15;i++) {
		buffer.Format(L"0x%06x",GetStatsColor(i));
		buffer2.Format(L"StatColor%i",i);
		ini.WriteString(buffer2,buffer,L"Statistics" );
	}


	///////////////////////////////////////////////////////////////////////////
	// Section: "WebServer"
	//
	ini.WriteString(L"Password", GetWSPass(), L"WebServer");
	ini.WriteString(L"PasswordLow", GetWSLowPass());
	ini.WriteInt(L"Port", m_nWebPort);
	ini.WriteBool(L"Enabled", m_bWebEnabled);
	ini.WriteBool(L"UseGzip", m_bWebUseGzip);
	ini.WriteInt(L"PageRefreshTime", m_nWebPageRefresh);
	ini.WriteBool(L"UseLowRightsUser", m_bWebLowEnabled);
	ini.WriteBool(L"AllowAdminHiLevelFunc",m_bAllowAdminHiLevFunc);
	ini.WriteInt(L"WebTimeoutMins", m_iWebTimeoutMins);


	///////////////////////////////////////////////////////////////////////////
//remove mobile
	// Section: "PeerCache"
	//
	ini.WriteInt(L"LastSearch", m_uPeerCacheLastSearch, L"PeerCache");
	ini.WriteBool(L"Found", m_bPeerCacheWasFound);
	ini.WriteBool(L"Enabled", m_bPeerCacheEnabled);
	ini.WriteInt(L"PCPort", m_nPeerCachePort);
	//End Section
    //////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// Section: "TK4 Additional settings" - TK4 Mod  
	// 	
	ini.WriteBool(L"HiddenHotKeyEnabled",hotkey_enabled,_T("TK4 Additional"));
	ini.WriteInt(L"HiddenHotKeyMod",hotkey_mod,_T("TK4 Additional"));
	ini.WriteInt(L"HiddenHotKeyKey",hotkey_key,_T("TK4 Additional"));
 	ini.WriteBool(L"BadNameCheckEnabled",BadNameCheck,_T("TK4 Additional"));    //[FDC]	 enable state
	ini.WriteBool(L"DoubleFDC",doubleFDC,_T("TK4 Additional"));                 //[FDC]	 require 2 bad names for alert
	ini.WriteInt(L"FDCSensitivity",FDCSensitivity,_T("TK4 Additional"));        //[FDC]  save FDC sensitivity
 

	//Xman Xtreme Mod:
	//--------------------------------------------------------------------------

	//upnp_start
	ini.WriteBool(L"UPnPNAT", m_bUPnPNat, L"UPnP");
	ini.WriteBool(L"UPnPNAT_TryRandom", m_bUPnPTryRandom, L"UPnP");
	//upnp_end
//boiuzaum
	ini.WriteInt(_T("AduTips"), m_AduTips, _T("AdunanzA"));
//boizaum
	//Xman Xtreme Upload
	ini.WriteFloat(L"uploadslotspeed",m_slotspeed,L"Xtreme");
	ini.WriteBool(L"openmoreslots",m_openmoreslots);
	ini.WriteBool(L"bandwidthnotreachedslots",m_bandwidthnotreachedslots);
	ini.WriteInt(L"sendbuffersize", m_sendbuffersize);

	ini.WriteBool(L"retryconnectionattempts", retryconnectionattempts);

	//Xman GlobalMaxHarlimit for fairness
	ini.WriteBool(L"Acceptsourcelimit", m_bAcceptsourcelimit);

	//Xman show additional graph lines
	ini.WriteBool(L"ShowAdditionalGraph", m_bShowAdditionalGraph);

	//Xman process prio
	ini.WriteInt(L"MainProcessPriority", m_MainProcessPriority); // [TPT] - Select process priority

	//Xman Anti-Leecher
	ini.WriteBool(L"AntiLeecher",m_antileecher);
	ini.WriteBool(L"AntiLeecherName", m_antileechername);
	ini.WriteBool(L"AntiGhost", m_antighost);
	ini.WriteBool(L"AntiLeecherBadHello", m_antileecherbadhello);
	ini.WriteBool(L"AntiLeecherSnafu", m_antileechersnafu);
	ini.WriteBool(L"AntiLeecherMod", m_antileechermod);
	ini.WriteBool(L"AntiLeecherThief", m_antileecherthief);
	ini.WriteBool(L"AntiLeecherSpammer", m_antileecherspammer);
	ini.WriteBool(L"AntiLeecherXSExploiter", m_antileecherxsexploiter);
	ini.WriteBool(L"AntiLeecheremcrypt", m_antileecheremcrypt);
	ini.WriteBool(L"AntiLeecherCommunity_Action", m_antileechercommunity_action);
	ini.WriteBool(L"AntiLeecherGhost_Action", m_antileecherghost_action);
	ini.WriteBool(L"AntiLeecherThief_Action", m_antileecherthief_action);
	//Xman end

	//Xman narrow font at transferwindow
	ini.WriteBool(L"UseNarrowFont", m_bUseNarrowFont);
	//Xman end

	//Xman 1:3 Ratio
	ini.WriteBool(L"amountbasedratio",m_13ratio);
	//Xman end

	//Xman chunk chooser
	ini.WriteInt(L"chunkchooser", m_chunkchooser);


	//Xman count block/success send
	ini.WriteBool(L"ShowSocketBlockRatio", m_showblockratio);
	ini.WriteBool(L"DropBlockingSockets", m_dropblockingsockets);

	//Xman Funny-Nick (Stulle/Morph)
	ini.WriteBool(_T("DisplayFunnyNick"), m_bFunnyNick);
	//Xman end

	//Xman remove unused AICH-hashes
	ini.WriteBool(L"rememberAICH", m_rememberAICH);

	//Xman smooth-accurate-graph
	ini.WriteBool(L"usesmoothgraph",usesmoothgraph);

	// Maella -Graph: display zoom-
	ini.WriteInt(L"ZoomFactor", zoomFactor);
	// Maella end

	// Maella -MTU Configuration-
	if (MTU<500)
		MTU=500;
	if (MTU>1500)
		MTU=1500;
	ini.WriteInt(L"MTU", MTU);
	// Maella end

	ini.WriteBool(L"usedoublesendsize",usedoublesendsize);

	// Maella -Network Adapter Feedback Control-
	ini.WriteBool(L"NAFCFullControl", NAFCFullControl);
	ini.WriteInt(L"ForceNAFCAdapter",forceNAFCadapter);
	// Maella end

	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	ini.WriteInt(L"DatarateSamples", datarateSamples);
	// Maella end

	// Maella -One-queue-per-file- (idea bloodymad)
	ini.WriteBool(L"EnableMultiQueue", enableMultiQueue);
	ini.WriteBool(L"EnableReleaseMultiQueue", enableReleaseMultiQueue);
	// Maella end

	// Mighty Knife: Static server handling
	ini.WriteBool (L"DontRemoveStaticServers",m_bDontRemoveStaticServers);
	// [end] Mighty Knife

	//Xman [MoNKi: -Downloaded History-]
	ini.WriteBool(L"ShowSharedInHistory", m_bHistoryShowShared);
	//Xman end


	//Xman don't overwrite bak files if last sessions crashed
	ini.WriteBool(L"last_session_aborted_in_an_unnormal_way", m_this_session_aborted_in_an_unnormal_way);

	//Xman end
	//--------------------------------------------------------------------------

}

void CPreferences::ResetStatsColor(int index)
{
	switch(index)
	{
		case  0: m_adwStatsColors[ 0]=RGB(  0,  0, 64);break;
		case  1: m_adwStatsColors[ 1]=RGB(192,192,255);break;
		case  2: m_adwStatsColors[ 2]=RGB(128,255,128);break;
		case  3: m_adwStatsColors[ 3]=RGB(  0,210,  0);break;
		case  4: m_adwStatsColors[ 4]=RGB(  0,128,  0);break;
		case  5: m_adwStatsColors[ 5]=RGB(255,128,128);break;
		case  6: m_adwStatsColors[ 6]=RGB(200,  0,  0);break;
		case  7: m_adwStatsColors[ 7]=RGB(140,  0,  0);break;
		case  8: m_adwStatsColors[ 8]=RGB(150,150,255);break;
		case  9: m_adwStatsColors[ 9]=RGB(192,  0,192);break;
		case 10: m_adwStatsColors[10]=RGB(255,255,128);break;
		case 11: m_adwStatsColors[11]=RGB(  0,  0,  0);break;
		case 12: m_adwStatsColors[12]=RGB(255,255,255);break;
		case 13: m_adwStatsColors[13]=RGB(255,255,255);break;
		case 14: m_adwStatsColors[14]=RGB(255,190,190);break;
	}
}

void CPreferences::GetAllStatsColors(int iCount, LPDWORD pdwColors)
{
	memset(pdwColors, 0, sizeof(*pdwColors) * iCount);
	memcpy(pdwColors, m_adwStatsColors, sizeof(*pdwColors) * min(ARRSIZE(m_adwStatsColors), iCount));
}

bool CPreferences::SetAllStatsColors(int iCount, const DWORD* pdwColors)
{
	bool bModified = false;
	int iMin = min(ARRSIZE(m_adwStatsColors), iCount);
	for (int i = 0; i < iMin; i++)
	{
		if (m_adwStatsColors[i] != pdwColors[i])
		{
			m_adwStatsColors[i] = pdwColors[i];
			bModified = true;
		}
	}
	return bModified;
}

void CPreferences::IniCopy(CString si, CString di) {
	CIni ini(GetConfigFile(), L"eMule");

	CString s=ini.GetString(si);

	ini.SetSection(L"ListControlSetup");

	ini.WriteString(di,s);
}

// Imports the tablesetups of emuleversions (.ini) <0.46b		- temporary
void CPreferences::ImportOldTableSetup() {

	IniCopy(L"DownloadColumnHidden" ,	L"DownloadListCtrlColumnHidden" );
	IniCopy(L"DownloadColumnWidths" ,	L"DownloadListCtrlColumnWidths" );
	IniCopy(L"DownloadColumnOrder" ,		L"DownloadListCtrlColumnOrders" );
	IniCopy(L"TableSortItemDownload" ,	L"DownloadListCtrlTableSortItem" );
	IniCopy(L"TableSortAscendingDownload" , L"DownloadListCtrlTableSortAscending" );

	IniCopy(L"ONContactListCtrlColumnHidden" ,	L"ONContactListCtrlColumnHidden" );
	IniCopy(L"ONContactListCtrlColumnWidths" ,	L"ONContactListCtrlColumnWidths" );
	IniCopy(L"ONContactListCtrlColumnOrders" ,		L"ONContactListCtrlColumnOrders" );

	IniCopy(L"KadSearchListCtrlColumnHidden" ,	L"KadSearchListCtrlColumnHidden" );
	IniCopy(L"KadSearchListCtrlColumnWidths" ,	L"KadSearchListCtrlColumnWidths" );
	IniCopy(L"KadSearchListCtrlColumnOrders" ,		L"KadSearchListCtrlColumnOrders" );

	IniCopy(L"UploadColumnHidden" ,		L"UploadListCtrlColumnHidden" );
	IniCopy(L"UploadColumnWidths" ,		L"UploadListCtrlColumnWidths" );
	IniCopy(L"UploadColumnOrder" ,		L"UploadListCtrlColumnOrders" );
	IniCopy(L"TableSortItemUpload" ,		L"UploadListCtrlTableSortItem" );
	IniCopy(L"TableSortAscendingUpload", L"UploadListCtrlTableSortAscending" );

	IniCopy(L"QueueColumnHidden" ,		L"QueueListCtrlColumnHidden" );
	IniCopy(L"QueueColumnWidths" ,		L"QueueListCtrlColumnWidths" );
	IniCopy(L"QueueColumnOrder" ,		L"QueueListCtrlColumnOrders" );
	IniCopy(L"TableSortItemQueue" ,		L"QueueListCtrlTableSortItem" );
	IniCopy(L"TableSortAscendingQueue" , L"QueueListCtrlTableSortAscending" );

	IniCopy(L"SearchColumnHidden" ,		L"SearchListCtrlColumnHidden" );
	IniCopy(L"SearchColumnWidths" ,		L"SearchListCtrlColumnWidths" );
	IniCopy(L"SearchColumnOrder" ,		L"SearchListCtrlColumnOrders" );
	IniCopy(L"TableSortItemSearch" ,		L"SearchListCtrlTableSortItem" );
	IniCopy(L"TableSortAscendingSearch", L"SearchListCtrlTableSortAscending" );

	IniCopy(L"SharedColumnHidden" ,		L"SharedFilesCtrlColumnHidden" );
	IniCopy(L"SharedColumnWidths" ,		L"SharedFilesCtrlColumnWidths" );
	IniCopy(L"SharedColumnOrder" ,		L"SharedFilesCtrlColumnOrders" );
	IniCopy(L"TableSortItemShared" ,		L"SharedFilesCtrlTableSortItem" );
	IniCopy(L"TableSortAscendingShared", L"SharedFilesCtrlTableSortAscending" );

	IniCopy(L"ServerColumnHidden" ,		L"ServerListCtrlColumnHidden" );
	IniCopy(L"ServerColumnWidths" ,		L"ServerListCtrlColumnWidths" );
	IniCopy(L"ServerColumnOrder" ,		L"ServerListCtrlColumnOrders" );
	IniCopy(L"TableSortItemServer" ,		L"ServerListCtrlTableSortItem" );
	IniCopy(L"TableSortAscendingServer", L"ServerListCtrlTableSortAscending" );

	IniCopy(L"ClientListColumnHidden" ,		L"ClientListCtrlColumnHidden" );
	IniCopy(L"ClientListColumnWidths" ,		L"ClientListCtrlColumnWidths" );
	IniCopy(L"ClientListColumnOrder" ,		L"ClientListCtrlColumnOrders" );
	IniCopy(L"TableSortItemClientList" ,		L"ClientListCtrlTableSortItem" );
	IniCopy(L"TableSortAscendingClientList", L"ClientListCtrlTableSortAscending" );

	IniCopy(L"FilenamesListColumnHidden" ,	L"FileDetailDlgNameColumnHidden" );
	IniCopy(L"FilenamesListColumnWidths" ,	L"FileDetailDlgNameColumnWidths" );
	IniCopy(L"FilenamesListColumnOrder" ,	L"FileDetailDlgNameColumnOrders" );
	IniCopy(L"TableSortItemFilenames" ,		L"FileDetailDlgNameTableSortItem" );
	IniCopy(L"TableSortAscendingFilenames",  L"FileDetailDlgNameTableSortAscending" );

	IniCopy(L"IrcMainColumnHidden" ,		L"IrcNickListCtrlColumnHidden" );
	IniCopy(L"IrcMainColumnWidths" ,		L"IrcNickListCtrlColumnWidths" );
	IniCopy(L"IrcMainColumnOrder" ,		L"IrcNickListCtrlColumnOrders" );
	IniCopy(L"TableSortItemIrcMain" ,	L"IrcNickListCtrlTableSortItem" );
	IniCopy(L"TableSortAscendingIrcMain",L"IrcNickListCtrlTableSortAscending" );

	IniCopy(L"IrcChannelsColumnHidden" ,		L"IrcChannelListCtrlColumnHidden" );
	IniCopy(L"IrcChannelsColumnWidths" ,		L"IrcChannelListCtrlColumnWidths" );
	IniCopy(L"IrcChannelsColumnOrder" ,		L"IrcChannelListCtrlColumnOrders" );
	IniCopy(L"TableSortItemIrcChannels" ,	L"IrcChannelListCtrlTableSortItem" );
	IniCopy(L"TableSortAscendingIrcChannels",L"IrcChannelListCtrlTableSortAscending" );

	IniCopy(L"DownloadClientsColumnHidden" ,		L"DownloadClientsCtrlColumnHidden" );
	IniCopy(L"DownloadClientsColumnWidths" ,		L"DownloadClientsCtrlColumnWidths" );
	IniCopy(L"DownloadClientsColumnOrder" ,		L"DownloadClientsCtrlColumnOrders" );
}

void CPreferences::LoadPreferences()
{
	TCHAR buffer[256];

	CIni ini(GetConfigFile(), L"eMule");

	// import old (<0.46b) table setups - temporary
	if (ini.GetInt(L"SearchListCtrlTableSortItem",-1,L"ListControlSetup")==-1)
		ImportOldTableSetup();
	ini.SetSection(L"eMule");

	CString strCurrVersion, strPrefsVersion;

	strCurrVersion = theApp.m_strCurVersionLong;
	strPrefsVersion = ini.GetString(L"AppVersion");

	m_bFirstStart = false;

	if (strCurrVersion != strPrefsVersion){
		m_bFirstStart = true;
	}

#ifdef _DEBUG
	m_iDbgHeap = ini.GetInt(L"DebugHeap", 1);
#else
	m_iDbgHeap = 0;
#endif

	m_nWebMirrorAlertLevel = ini.GetInt(L"WebMirrorAlertLevel",0);
	updatenotify=ini.GetBool(L"UpdateNotifyTestClient",false); //Xman

	SetUserNick(ini.GetStringUTF8(L"Nick", DEFAULT_NICK));
	if (strNick.IsEmpty() || IsDefaultNick(strNick))
		SetUserNick(_T("[www.dreamteamshare.com]DreaMule 3")); //(DEFAULT_NICK); //Xman UserNick

	_stprintf(buffer,L"%sIncoming",appdir);
	_stprintf(incomingdir,L"%s",ini.GetString(L"IncomingDir",buffer ));
	MakeFoldername(incomingdir);

	// load tempdir(s) setting
	_stprintf(buffer,L"%sTemp",appdir);

	CString tempdirs;
	tempdirs=ini.GetString(L"TempDir",buffer);
	tempdirs+= L"|" + ini.GetString(L"TempDirs");

	int curPos=0;
	bool doubled;
	CString atmp=tempdirs.Tokenize(L"|", curPos);
	while (!atmp.IsEmpty())
	{
		atmp.Trim();
		if (!atmp.IsEmpty()) {
			MakeFoldername(atmp.GetBuffer(MAX_PATH));
			atmp.ReleaseBuffer();
			doubled=false;
			for (int i=0;i<tempdir.GetCount();i++)	// avoid double tempdirs
				if (atmp.CompareNoCase(GetTempDir(i))==0) {
					doubled=true;
					break;
				}
			if (!doubled) {
				if (PathFileExists(atmp)==FALSE) {
					CreateDirectory(atmp,NULL);
					if (PathFileExists(atmp)==TRUE || tempdir.GetCount()==0)
						tempdir.Add(atmp);
				}
				else
					tempdir.Add(atmp);
			}
		}
		atmp = tempdirs.Tokenize(L"|", curPos);
	}

	//-------------------------------------------------------------------
	//Xman Xtreme Mod
	m_wLanguageID=ini.GetWORD(L"Language",0);	//thx [MoNKi: -FIX: ini.GetFloat needs Language- ]
	SetLanguage();


	//Xman Xtreme Upload
	maxGraphDownloadRate=ini.GetFloat(L"DownloadCapacity",96); // Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
	if (maxGraphDownloadRate==0) maxGraphDownloadRate=96;
	maxGraphUploadRate=ini.GetFloat(L"UploadCapacity",16);// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
	if (maxGraphUploadRate==0) maxGraphUploadRate=16;
	minupload=(uint16)ini.GetInt(L"MinUpload", 1); //Xman not used!
	maxupload=ini.GetFloat(L"MaxUpload",12); // Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-


	if(maxupload<= 0.0f || maxupload >= UNLIMITED)
	{
		maxupload=11.0f;
		::MessageBox(NULL,
			_T("Atenção ! Ilimitado é invalido !\n\n")
			_T("Attention, une bande passante 'illimitée' pour l'émission est invalide\n\n")
			_T("Achtung, unbegrenzte Datarate für das Upload ist ungültig\n\n")
			_T("Advirtiendo, el límite para el datarate del upload es inválido"),
			_T("Warning/Attention/Achtung/Advirtiendo"),
			MB_ICONINFORMATION);
	}
	else if(maxupload<3.0f)
		maxupload=3.0f;
	if(maxGraphUploadRate<= 0.0f || maxGraphUploadRate >= UNLIMITED)
		maxGraphUploadRate=16.0f;
	else if(maxGraphUploadRate<5.0f)
		maxGraphUploadRate=5.0f;


	if (maxupload>maxGraphUploadRate && maxupload!=UNLIMITED) maxupload=maxGraphUploadRate*.8f;
	maxdownload=ini.GetFloat(L"MaxDownload",UNLIMITED); // Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
	if (maxdownload>maxGraphDownloadRate && maxdownload!=UNLIMITED) maxdownload=maxGraphDownloadRate*.8f;
	
	m_internetdownreactiontime=ini.GetInt(L"internetdownreactiontime",2);
	if(m_internetdownreactiontime <1)
		m_internetdownreactiontime=1;
	if(m_internetdownreactiontime>10)
		m_internetdownreactiontime=10;

	//Xman end
	//-------------------------------------------------------------------

	maxconnections=ini.GetInt(L"MaxConnections",GetRecommendedMaxConnections());
	maxhalfconnections=ini.GetInt(L"MaxHalfConnections",9);
	m_bConditionalTCPAccept = ini.GetBool(L"ConditionalTCPAccept", false);

	// reset max halfopen to a default if OS changed to SP2 or away
	int dwSP2 = ini.GetInt(L"WinXPSP2", -1);
	int dwCurSP2 = IsRunningXPSP2();
	if (dwSP2 != dwCurSP2){
		if (dwCurSP2 == 0)
			maxhalfconnections = 50;
		else if (dwCurSP2 == 1)
			maxhalfconnections = 9;
	}

	m_strBindAddrW = ini.GetString(L"BindAddr");
	m_strBindAddrW.Trim();
	m_pszBindAddrW = m_strBindAddrW.IsEmpty() ? NULL : (LPCWSTR)m_strBindAddrW;
	m_strBindAddrA = m_strBindAddrW;
	m_pszBindAddrA = m_strBindAddrA.IsEmpty() ? NULL : (LPCSTR)m_strBindAddrA;
	
	port = (uint16)ini.GetInt(L"Port", 0);
	if (port == 0)
		port = thePrefs.GetRandomTCPPort();

	// 0 is a valid value for the UDP port setting, as it is used for disabling it.
	int iPort = ini.GetInt(L"UDPPort", INT_MAX/*invalid port value*/);
	if (iPort == INT_MAX)
		udpport = thePrefs.GetRandomUDPPort();
	else
		udpport = (uint16)iPort;

	nServerUDPPort = (uint16)ini.GetInt(L"ServerUDPPort", -1); // 0 = Don't use UDP port for servers, -1 = use a random port (for backward compatibility)
	maxsourceperfile=ini.GetInt(L"MaxSourcesPerFile",400 );
	m_iSeeShares=(EViewSharedFilesAccess)ini.GetInt(L"SeeShare",vsfaNobody);
	m_iToolDelayTime=ini.GetInt(L"ToolTipDelay",1);
	trafficOMeterInterval=ini.GetInt(L"StatGraphsInterval",5); //Xman
	statsInterval=ini.GetInt(L"statsInterval",5);
	dontcompressavi=ini.GetBool(L"DontCompressAvi",false);

	m_uDeadServerRetries=ini.GetInt(L"DeadServerRetry",1);
	if (m_uDeadServerRetries > MAX_SERVERFAILCOUNT)
		m_uDeadServerRetries = MAX_SERVERFAILCOUNT;
	m_dwServerKeepAliveTimeout=ini.GetInt(L"ServerKeepAliveTimeout",0);
	splitterbarPosition=ini.GetInt(L"SplitterbarPosition",75);
	if (splitterbarPosition < 9)
		splitterbarPosition = 9;
	else if (splitterbarPosition > 93)
		splitterbarPosition = 93;
	splitterbarPositionStat=ini.GetInt(L"SplitterbarPositionStat",30);
	splitterbarPositionStat_HL=ini.GetInt(L"SplitterbarPositionStat_HL",66);
	splitterbarPositionStat_HR=ini.GetInt(L"SplitterbarPositionStat_HR",33);
	if (splitterbarPositionStat_HR+1>=splitterbarPositionStat_HL){
		splitterbarPositionStat_HL = 66;
		splitterbarPositionStat_HR = 33;
	}
	splitterbarPositionFriend=ini.GetInt(L"SplitterbarPositionFriend",300);
	splitterbarPositionShared=ini.GetInt(L"SplitterbarPositionShared",179);
	splitterbarPositionIRC=ini.GetInt(L"SplitterbarPositionIRC",200);
	splitterbarPositionSvr=ini.GetInt(L"SplitterbarPositionServer",75);
	if (splitterbarPositionSvr>90 || splitterbarPositionSvr<10)
		splitterbarPositionSvr=75;

	m_uTransferWnd1 = ini.GetInt(L"TransferWnd1",0);
	m_uTransferWnd2 = ini.GetInt(L"TransferWnd2",1);

	statsMax=ini.GetInt(L"VariousStatisticsMaxValue",100);
	statsAverageMinutes=ini.GetInt(L"StatsAverageMinutes",5);
	MaxConperFive=ini.GetInt(L"MaxConnectionsPerFiveSeconds",GetDefaultMaxConperFive());

	reconnect = ini.GetBool(L"Reconnect", true);
	m_bUseServerPriorities = ini.GetBool(L"Scoresystem", true);
	m_bUseUserSortedServerList = ini.GetBool(L"UserSortedServerList", false);
	ICH = ini.GetBool(L"ICH", true);
	m_bAutoUpdateServerList = ini.GetBool(L"Serverlist", false);

	mintotray=ini.GetBool(L"MinToTray",false);
	m_bAddServersFromServer=ini.GetBool(L"AddServersFromServer",false); //Xman changed
	m_bAddServersFromClients=ini.GetBool(L"AddServersFromClient",false);
	splashscreen=ini.GetBool(L"Splashscreen",true);
	bringtoforeground=ini.GetBool(L"BringToFront",true);
	transferDoubleclick=ini.GetBool(L"TransferDoubleClick",true);
	beepOnError=ini.GetBool(L"BeepOnError",true);
	confirmExit=ini.GetBool(L"ConfirmExit",true);
	filterLANIPs=ini.GetBool(L"FilterBadIPs",true);
	m_bAllocLocalHostIP=ini.GetBool(L"AllowLocalHostIP",false);
	autoconnect=ini.GetBool(L"Autoconnect",true);
	showRatesInTitle=ini.GetBool(L"ShowRatesOnTitle",false);
	m_bIconflashOnNewMessage=ini.GetBool(L"IconflashOnNewMessage",false);

	onlineSig=ini.GetBool(L"OnlineSignature",false);
	startMinimized=ini.GetBool(L"StartupMinimized",false);
	m_bAutoStart=ini.GetBool(L"AutoStart",false);
	m_bRestoreLastMainWndDlg=ini.GetBool(L"RestoreLastMainWndDlg",false);
	m_iLastMainWndDlgID=ini.GetInt(L"LastMainWndDlgID",0);
	m_bRestoreLastLogPane=ini.GetBool(L"RestoreLastLogPane",false);
	m_iLastLogPaneID=ini.GetInt(L"LastLogPaneID",0);
	m_bSafeServerConnect =ini.GetBool(L"SafeServerConnect",false);

	m_bTransflstRemain =ini.GetBool(L"TransflstRemainOrder",false);
	filterserverbyip=ini.GetBool(L"FilterServersByIP",false);
	filterlevel=ini.GetInt(L"FilterLevel",127);
	checkDiskspace=ini.GetBool(L"CheckDiskspace",false);
	m_uMinFreeDiskSpace=ini.GetInt(L"MinFreeDiskSpace",20*1024*1024);
	m_bSparsePartFiles=ini.GetBool(L"SparsePartFiles",false);
	m_strYourHostname=ini.GetString(L"YourHostname", L"");

	// Barry - New properties...
	m_bAutoConnectToStaticServersOnly = ini.GetBool(L"AutoConnectStaticOnly",true);
	autotakeed2klinks = ini.GetBool(L"AutoTakeED2KLinks",true);
	addnewfilespaused = ini.GetBool(L"AddNewFilesPaused",false);
	depth3D = ini.GetInt(L"3DDepth", 5);
	m_bEnableMiniMule = ini.GetBool(L"MiniMule", true);

	// Notifier
	notifierConfiguration = ini.GetString(L"NotifierConfiguration", GetConfigDir() + L"Notifier.ini");
    notifierOnDownloadFinished = ini.GetBool(L"NotifyOnDownload");
	notifierOnNewDownload = ini.GetBool(L"NotifyOnNewDownload");
    notifierOnChat = ini.GetBool(L"NotifyOnChat");
    notifierOnLog = ini.GetBool(L"NotifyOnLog");
	notifierOnImportantError = ini.GetBool(L"NotifyOnImportantError");
	notifierOnEveryChatMsg = ini.GetBool(L"NotifierPopEveryChatMessage");
	notifierOnNewVersion = ini.GetBool(L"NotifierPopNewVersion");
    notifierSoundType = (ENotifierSoundType)ini.GetInt(L"NotifierUseSound", ntfstNoSound);
	notifierSoundFile = ini.GetString(L"NotifierSoundPath");

	_stprintf(datetimeformat,L"%s",ini.GetString(L"DateTimeFormat",L"%A, %c"));
	if (_tcslen(datetimeformat)==0) _tcscpy(datetimeformat,L"%A, %c");
	_stprintf(datetimeformat4log,L"%s",ini.GetString(L"DateTimeFormat4Log",L"%c"));
	if (_tcslen(datetimeformat4log)==0) _tcscpy(datetimeformat4log,L"%c");

	_stprintf(m_sircserver,L"%s",ini.GetString(L"DefaultIRCServerNew",L"ircchat.emule-project.net"));
	_stprintf(m_sircnick,L"%s",ini.GetString(L"IRCNick"));
	m_bircaddtimestamp=ini.GetBool(L"IRCAddTimestamp",true);
	_stprintf(m_sircchannamefilter,L"%s",ini.GetString(L"IRCFilterName", L"" ));
	m_bircusechanfilter=ini.GetBool(L"IRCUseFilter", false);
	m_iircchanneluserfilter=ini.GetInt(L"IRCFilterUser", 0);
	_stprintf(m_sircperformstring,L"%s",ini.GetString(L"IRCPerformString", L"" ));
	m_bircuseperform=ini.GetBool(L"IRCUsePerform", false);
	m_birclistonconnect=ini.GetBool(L"IRCListOnConnect", true);
	m_bircacceptlinks=ini.GetBool(L"IRCAcceptLink", true);
	m_bircacceptlinksfriends=ini.GetBool(L"IRCAcceptLinkFriends", true);
	m_bircsoundevents=ini.GetBool(L"IRCSoundEvents", false);
	m_bircignoremiscmessage=ini.GetBool(L"IRCIgnoreMiscMessages", false);
	m_bircignorejoinmessage=ini.GetBool(L"IRCIgnoreJoinMessages", true);
	m_bircignorepartmessage=ini.GetBool(L"IRCIgnorePartMessages", true);
	m_bircignorequitmessage=ini.GetBool(L"IRCIgnoreQuitMessages", true);
	m_bircignoreemuleprotoaddfriend=ini.GetBool(L"IRCIgnoreEmuleProtoAddFriend", false);
	m_bircallowemuleprotoaddfriend=ini.GetBool(L"IRCAllowEmuleProtoAddFriend", true);
	m_bircignoreemuleprotosendlink=ini.GetBool(L"IRCIgnoreEmuleProtoSendLink", false);
	m_birchelpchannel=ini.GetBool(L"IRCHelpChannel",true);
	m_bSmartServerIdCheck=ini.GetBool(L"SmartIdCheck",true);

	log2disk = ini.GetBool(L"SaveLogToDisk",false);
	uMaxLogFileSize = ini.GetInt(L"MaxLogFileSize", 1024*1024);
	iMaxLogBuff = ini.GetInt(L"MaxLogBuff",64) * 1024;
	m_iLogFileFormat = (ELogFileFormat)ini.GetInt(L"LogFileFormat", Unicode, 0);
	m_bEnableVerboseOptions=ini.GetBool(L"VerboseOptions", true);
	if (m_bEnableVerboseOptions)
	{
		m_bVerbose=ini.GetBool(L"Verbose",false);
		m_bFullVerbose=ini.GetBool(L"FullVerbose",false);
		debug2disk=ini.GetBool(L"SaveDebugToDisk",false);
		m_bDebugSourceExchange=ini.GetBool(L"DebugSourceExchange",false);
		m_bLogBannedClients=ini.GetBool(L"LogBannedClients", false); //Xman
		m_bLogRatingDescReceived=ini.GetBool(L"LogRatingDescReceived",true);
		m_bLogSecureIdent=ini.GetBool(L"LogSecureIdent",true);
		m_bLogFilteredIPs=ini.GetBool(L"LogFilteredIPs",false); //Xman
		m_bLogFileSaving=ini.GetBool(L"LogFileSaving",false);
        m_bLogA4AF=ini.GetBool(L"LogA4AF",false); // ZZ:DownloadManager
		m_bLogDrop=ini.GetBool(L"LogDrop",false); //Xman Xtreme Downloadmanager
		m_bLogpartmismatch=ini.GetBool(L"Logpartmismatch", true); //Xman Log part/size-mismatch
		m_bLogUlDlEvents=ini.GetBool(L"LogUlDlEvents",true);
	}
	else
	{
		if (m_bRestoreLastLogPane && m_iLastLogPaneID>=2)
			m_iLastLogPaneID = 1;
	}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	// following options are for debugging or when using an external debug device viewer only.
	m_iDebugServerTCPLevel = ini.GetInt(L"DebugServerTCP", 0);
	m_iDebugServerUDPLevel = ini.GetInt(L"DebugServerUDP", 0);
	m_iDebugServerSourcesLevel = ini.GetInt(L"DebugServerSources", 0);
	m_iDebugServerSearchesLevel = ini.GetInt(L"DebugServerSearches", 0);
	m_iDebugClientTCPLevel = ini.GetInt(L"DebugClientTCP", 0);
	m_iDebugClientUDPLevel = ini.GetInt(L"DebugClientUDP", 0);
	m_iDebugClientKadUDPLevel = ini.GetInt(L"DebugClientKadUDP", 0);
	m_iDebugSearchResultDetailLevel = ini.GetInt(L"DebugSearchResultDetailLevel", 0);
#else
	// for normal release builds ensure that those options are all turned off
	m_iDebugServerTCPLevel = 0;
	m_iDebugServerUDPLevel = 0;
	m_iDebugServerSourcesLevel = 0;
	m_iDebugServerSearchesLevel = 0;
	m_iDebugClientTCPLevel = 0;
	m_iDebugClientUDPLevel = 0;
	m_iDebugClientKadUDPLevel = 0;
	m_iDebugSearchResultDetailLevel = 0;
#endif

	m_bpreviewprio=ini.GetBool(L"PreviewPrio",false);
	m_bupdatequeuelist=ini.GetBool(L"UpdateQueueListPref",true); //Xman
	m_bManualAddedServersHighPriority=ini.GetBool(L"ManualHighPrio",false);
	m_btransferfullchunks=ini.GetBool(L"FullChunkTransfers",true);
	m_istartnextfile=ini.GetInt(L"StartNextFile",0);
	m_bshowoverhead=ini.GetBool(L"ShowOverhead",true); //Xman
	moviePreviewBackup=ini.GetBool(L"VideoPreviewBackupped",true);
	m_iPreviewSmallBlocks=ini.GetInt(L"PreviewSmallBlocks", 0);
	m_bPreviewCopiedArchives=ini.GetBool(L"PreviewCopiedArchives", true);
	m_iInspectAllFileTypes=ini.GetInt(L"InspectAllFileTypes", 0);
	m_bAllocFull=ini.GetBool(L"AllocateFullFile",0);
	m_bAutomaticArcPreviewStart=ini.GetBool(L"AutoArchivePreviewStart", true );

	// read file buffer size (with backward compatibility)
	m_iFileBufferSize=ini.GetInt(L"FileBufferSizePref",0); // old setting
	if (m_iFileBufferSize == 0)
		m_iFileBufferSize = 256*1024;
	else
		m_iFileBufferSize = ((m_iFileBufferSize*15000 + 512)/1024)*1024;
	m_iFileBufferSize=ini.GetInt(L"FileBufferSize",m_iFileBufferSize);
//>>> WiZaRd::IntelliFlush
	m_bIntelliFlush = ini.GetBool(L"IntelliFlush", true);
//<<< WiZaRd::IntelliFlush

	// read queue size (with backward compatibility)
	m_iQueueSize=ini.GetInt(L"QueueSizePref",0); // old setting
	if (m_iQueueSize == 0)
		m_iQueueSize = 50*100;
	else
		m_iQueueSize = m_iQueueSize*100;
	m_iQueueSize=ini.GetInt(L"QueueSize",m_iQueueSize);

	m_iCommitFiles=ini.GetInt(L"CommitFiles", 1); // 1 = "commit" on application shut down; 2 = "commit" on each file saveing
	versioncheckdays=ini.GetInt(L"Check4NewVersionDelay",3); //Xman
	m_bDAP=ini.GetBool(L"DAPPref",true);
	m_bUAP=ini.GetBool(L"UAPPref",true);
	m_bPreviewOnIconDblClk=ini.GetBool(L"PreviewOnIconDblClk",false);
	indicateratings=ini.GetBool(L"IndicateRatings",true);
	watchclipboard=ini.GetBool(L"WatchClipboard4ED2kFilelinks",false);
	m_iSearchMethod=ini.GetInt(L"SearchMethod",0);

	showCatTabInfos=ini.GetBool(L"ShowInfoOnCatTabs",false);
//	resumeSameCat=ini.GetBool(L"ResumeNextFromSameCat",false);
	dontRecreateGraphs =ini.GetBool(L"DontRecreateStatGraphsOnResize",false);
	m_bExtControls =ini.GetBool(L"ShowExtControls",false);

	versioncheckLastAutomatic=ini.GetInt(L"VersionCheckLastAutomatic",0);
	m_bDisableKnownClientList=ini.GetBool(L"DisableKnownClientList",true);
	m_bDisableQueueList=ini.GetBool(L"DisableQueueList",false);
	m_bCreditSystem=ini.GetBool(L"UseCreditSystem",true);
	scheduler=ini.GetBool(L"EnableScheduler",false);
	msgonlyfriends=ini.GetBool(L"MessagesFromFriendsOnly",false);
	msgsecure=ini.GetBool(L"MessageFromValidSourcesOnly",true);
	autofilenamecleanup=ini.GetBool(L"AutoFilenameCleanup",false);
	m_bUseAutocompl=ini.GetBool(L"UseAutocompletion",true);
	m_bShowDwlPercentage=ini.GetBool(L"ShowDwlPercentage",false);
	networkkademlia=ini.GetBool(L"NetworkKademlia",true);
	networked2k=ini.GetBool(L"NetworkED2K",true);
	m_bRemove2bin=ini.GetBool(L"RemoveFilesToBin",true);
	m_bShowCopyEd2kLinkCmd=ini.GetBool(L"ShowCopyEd2kLinkCmd",false);

	m_iMaxChatHistory=ini.GetInt(L"MaxChatHistoryLines",100);
	if (m_iMaxChatHistory < 1)
		m_iMaxChatHistory = 100;
	maxmsgsessions=ini.GetInt(L"MaxMessageSessions",50);
	m_bShowActiveDownloadsBold = ini.GetBool(L"ShowActiveDownloadsBold", true);

	_snwprintf(TxtEditor,_countof(TxtEditor),L"%s",ini.GetString(L"TxtEditor",L"notepad.exe"));
	m_strVideoPlayer = ini.GetString(L"VideoPlayer", L"");
	m_strVideoPlayerArgs = ini.GetString(L"VideoPlayerArgs",L"");
	_snwprintf(m_sTemplateFile,_countof(m_sTemplateFile),L"%s",ini.GetString(L"WebTemplateFile", GetConfigDir()+L"eMule.tmpl"));

	messageFilter=ini.GetStringLong(L"MessageFilter",L"Your client has an infinite queue|Your client is connecting too fast|fastest download speed");
	commentFilter = ini.GetStringLong(L"CommentFilter",L"http://|https://|ftp://|www.|ftp.");
	commentFilter.MakeLower();
	filenameCleanups=ini.GetStringLong(L"FilenameCleanups",L"http|www.|.com|.de|.org|.net|shared|powered|sponsored|sharelive|filedonkey|");
	m_iExtractMetaData = ini.GetInt(L"ExtractMetaData", 1); // 0=disable, 1=mp3, 2=MediaDet
	if (m_iExtractMetaData > 1)
		m_iExtractMetaData = 1;
	m_bAdjustNTFSDaylightFileTime=ini.GetBool(L"AdjustNTFSDaylightFileTime", true);

	m_bUseSecureIdent=ini.GetBool(L"SecureIdent",true);
	m_bAdvancedSpamfilter=ini.GetBool(L"AdvancedSpamFilter",true);
	m_bRemoveFinishedDownloads=ini.GetBool(L"AutoClearCompleted",false);
	m_bUseOldTimeRemaining= ini.GetBool(L"UseSimpleTimeRemainingcomputation",false);

	// Toolbar
	m_sToolbarSettings = ini.GetString(L"ToolbarSetting", strDefaultToolbar);
	m_sToolbarBitmap = ini.GetString(L"ToolbarBitmap", L"");
	m_sToolbarBitmapFolder = ini.GetString(L"ToolbarBitmapFolder", appdir + L"skins");
	m_nToolbarLabels = (EToolbarLabelType)ini.GetInt(L"ToolbarLabels", CMuleToolbarCtrl::GetDefaultLabelType());
	m_bReBarToolbar = ini.GetBool(L"ReBarToolbar", 1);
	m_sizToolbarIconSize.cx = m_sizToolbarIconSize.cy = ini.GetInt(L"ToolbarIconSize", 32);
	m_iStraightWindowStyles=ini.GetInt(L"StraightWindowStyles",0);
	m_bRTLWindowsLayout = ini.GetBool(L"RTLWindowsLayout");
	m_strSkinProfile = ini.GetString(L"SkinProfile", L"");
	m_strSkinProfileDir = ini.GetString(L"SkinProfileDir", appdir + L"skins");

	LPBYTE pData = NULL;
	UINT uSize = sizeof m_lfHyperText;
	if (ini.GetBinary(L"HyperTextFont", &pData, &uSize) && uSize == sizeof m_lfHyperText)
		memcpy(&m_lfHyperText, pData, sizeof m_lfHyperText);
	else
		memset(&m_lfHyperText, 0, sizeof m_lfHyperText);
	delete[] pData;

	pData = NULL;
	uSize = sizeof m_lfLogText;
	if (ini.GetBinary(L"LogTextFont", &pData, &uSize) && uSize == sizeof m_lfLogText)
		memcpy(&m_lfLogText, pData, sizeof m_lfLogText);
	else
		memset(&m_lfLogText, 0, sizeof m_lfLogText);
	delete[] pData;

	m_crLogError = ini.GetColRef(L"LogErrorColor", m_crLogError);
	m_crLogWarning = ini.GetColRef(L"LogWarningColor", m_crLogWarning);
	m_crLogSuccess = ini.GetColRef(L"LogSuccessColor", m_crLogSuccess);

	if (statsAverageMinutes < 1)
		statsAverageMinutes = 5;

	// ZZ:UploadSpeedSense -->
    m_bDynUpEnabled = ini.GetBool(L"USSEnabled", false);
    m_bDynUpUseMillisecondPingTolerance = ini.GetBool(L"USSUseMillisecondPingTolerance", false);
    m_iDynUpPingTolerance = ini.GetInt(L"USSPingTolerance", 500);
	m_iDynUpPingToleranceMilliseconds = ini.GetInt(L"USSPingToleranceMilliseconds", 200);
	if( minupload < 1 )
		minupload = 1;
	m_iDynUpGoingUpDivider = ini.GetInt(L"USSGoingUpDivider", 1000);
    m_iDynUpGoingDownDivider = ini.GetInt(L"USSGoingDownDivider", 1000);
    m_iDynUpNumberOfPings = ini.GetInt(L"USSNumberOfPings", 1);
	// ZZ:UploadSpeedSense <--

    //m_bA4AFSaveCpu = ini.GetBool(L"A4AFSaveCpu", false); // ZZ:DownloadManager
    m_bHighresTimer = ini.GetBool(L"HighresTimer", false);
	m_bRunAsUser = ini.GetBool(L"RunAsUnprivilegedUser", false);
	m_bPreferRestrictedOverUser = ini.GetBool(L"PreferRestrictedOverUser", false);
	m_bOpenPortsOnStartUp = ini.GetBool(L"OpenPortsOnStartUp", true);
	m_byLogLevel = ini.GetInt(L"DebugLogLevel", DLP_VERYLOW);
	m_bTrustEveryHash = ini.GetBool(L"AICHTrustEveryHash", false);
	m_bRememberCancelledFiles = ini.GetBool(L"RememberCancelledFiles", true);
	m_bRememberDownloadedFiles = ini.GetBool(L"RememberDownloadedFiles", true);

	m_bNotifierSendMail = ini.GetBool(L"NotifierSendMail", false);
/* Xman removed
#if _ATL_VER >= 0x0710
	if (!IsRunningXPSP2())
		m_bNotifierSendMail = false;
#endif
*/
	m_strNotifierMailSender = ini.GetString(L"NotifierMailSender", L"");
	m_strNotifierMailServer = ini.GetString(L"NotifierMailServer", L"");
	m_strNotifierMailReceiver = ini.GetString(L"NotifierMailRecipient", L"");

	m_bWinaTransToolbar = ini.GetBool(L"WinaTransToolbar", true); //Xman default true

	m_bCryptLayerRequested = ini.GetBool(L"CryptLayerRequested", true);//Boizaum defaut Actived
	m_bCryptLayerRequired = ini.GetBool(L"CryptLayerRequired", false);
	m_bCryptLayerSupported = ini.GetBool(L"CryptLayerSupported", true);

	///////////////////////////////////////////////////////////////////////////
	// Section: "Proxy"
	//
	proxy.EnablePassword = ini.GetBool(L"ProxyEnablePassword",false,L"Proxy");
	proxy.UseProxy = ini.GetBool(L"ProxyEnableProxy",false,L"Proxy");
	proxy.name = CStringA(ini.GetString(L"ProxyName", L"", L"Proxy"));
	proxy.user = CStringA(ini.GetString(L"ProxyUser", L"", L"Proxy"));
	proxy.password = CStringA(ini.GetString(L"ProxyPassword", L"", L"Proxy"));
	proxy.port = (uint16)ini.GetInt(L"ProxyPort",1080,L"Proxy");
	proxy.type = (uint16)ini.GetInt(L"ProxyType",PROXYTYPE_NOPROXY,L"Proxy");


	///////////////////////////////////////////////////////////////////////////
	// Section: "Statistics"
	//
	statsSaveInterval = ini.GetInt(L"SaveInterval", 60, L"Statistics");
	statsConnectionsGraphRatio = ini.GetInt(L"statsConnectionsGraphRatio", 3, L"Statistics");
	_stprintf(statsExpandedTreeItems,L"%s",ini.GetString(L"statsExpandedTreeItems",L"111000000100000110000010000011110000010010",L"Statistics"));
	CString buffer2;
	for (int i = 0; i < ARRSIZE(m_adwStatsColors); i++) {
		buffer2.Format(L"StatColor%i", i);
		_stprintf(buffer, L"%s", ini.GetString(buffer2, L"", L"Statistics"));
		m_adwStatsColors[i] = 0;
		if (_stscanf(buffer, L"%i", &m_adwStatsColors[i]) != 1)
			ResetStatsColor(i);
	}
	m_bShowVerticalHourMarkers = ini.GetBool(L"ShowVerticalHourMarkers", true, L"Statistics");

	// -khaos--+++> Load Stats
	// I changed this to a seperate function because it is now also used
	// to load the stats backup and to load stats from preferences.ini.old.
	LoadStats();
	// <-----khaos-

	///////////////////////////////////////////////////////////////////////////
	// Section: "WebServer"
	//
	_stprintf(m_sWebPassword,L"%s",ini.GetString(L"Password", L"",L"WebServer"));
	_stprintf(m_sWebLowPassword,L"%s",ini.GetString(L"PasswordLow", L""));
	m_nWebPort=(uint16)ini.GetInt(L"Port", 4711);
	m_bWebEnabled=ini.GetBool(L"Enabled", false);
	m_bWebUseGzip=ini.GetBool(L"UseGzip", true);
	m_bWebLowEnabled=ini.GetBool(L"UseLowRightsUser", false);
	m_nWebPageRefresh=ini.GetInt(L"PageRefreshTime", 120);
	m_iWebTimeoutMins=ini.GetInt(L"WebTimeoutMins", 5 );
	m_iWebFileUploadSizeLimitMB=ini.GetInt(L"MaxFileUploadSizeMB", 5 );
	m_bAllowAdminHiLevFunc=ini.GetBool(L"AllowAdminHiLevelFunc", false);
	buffer2 = ini.GetString(L"AllowedIPs");
	int iPos = 0;
	CString strIP = buffer2.Tokenize(L";", iPos);
	while (!strIP.IsEmpty())
	{
		u_long nIP = inet_addr(CStringA(strIP));
		if (nIP != INADDR_ANY && nIP != INADDR_NONE)
			m_aAllowedRemoteAccessIPs.Add(nIP);
		strIP = buffer2.Tokenize(L";", iPos);
	}

	///////////////////////////////////////////////////////////////////////////
//remove moblie mule
	///////////////////////////////////////////////////////////////////////////
	// Section: "PeerCache"
	//
	m_uPeerCacheLastSearch = ini.GetInt(L"LastSearch", 0, L"PeerCache");
	m_bPeerCacheWasFound = ini.GetBool(L"Found", false);
	m_bPeerCacheEnabled = ini.GetBool(L"Enabled", true);
	m_nPeerCachePort = (uint16)ini.GetInt(L"PCPort", 0);
	m_bPeerCacheShow = ini.GetBool(L"Show", false);
//boizaum
	m_AduTips = ini.GetInt(_T("AduTips"), 0, _T("AdunanzA")) & (~(ADUTIP_LOWUPLIMITS|ADUTIP_BROKENKAD));
//boizaum

	LoadCats();
	//SetLanguage(); //Xman done above

	//--------------------------------------------------------------------------
	//Xman Xtreme Mod:

	//upnp_start
	m_bUPnPNat = ini.GetBool(L"UPnPNAT", true, L"UPnP");
	m_bUPnPTryRandom = ini.GetBool(L"UPnPNAT_TryRandom", true, L"UPnP");
	//upnp_end

	//Xman Xtreme Upload
	m_slotspeed=ini.GetFloat(L"uploadslotspeed",3.2f, L"Xtreme");
	CheckSlotSpeed();
	m_openmoreslots=ini.GetBool(L"openmoreslots",true);
	m_bandwidthnotreachedslots=ini.GetBool(L"bandwidthnotreachedslots",false);
	m_sendbuffersize=ini.GetInt(L"sendbuffersize", 8192);
	switch (m_sendbuffersize)
	{
	case 6000:
	case 8192:
	case 12000:
		break;
	default:
		m_sendbuffersize=8192;
	}
	//Xman end

	retryconnectionattempts=ini.GetBool(L"retryconnectionattempts",true);

	//Xman GlobalMaxHarlimit for fairness
	m_bAcceptsourcelimit=ini.GetBool(L"Acceptsourcelimit", true);

	//Xman show additional graph lines
	m_bShowAdditionalGraph=ini.GetBool(L"ShowAdditionalGraph", false);

	//Xman process prio
	m_MainProcessPriority = ini.GetInt(L"MainProcessPriority", NORMAL_PRIORITY_CLASS);
	switch (GetWindowsVersion())
	{
	case _WINVER_98_:
	case _WINVER_95_:
	case _WINVER_ME_:
		//		case _WINVER_NT4_: // same as win95
		if (m_MainProcessPriority == ABOVE_NORMAL_PRIORITY_CLASS || m_MainProcessPriority == BELOW_NORMAL_PRIORITY_CLASS )
			m_MainProcessPriority = NORMAL_PRIORITY_CLASS;
	}
	switch (m_MainProcessPriority)
	{
	case IDLE_PRIORITY_CLASS:
		m_MainProcessPriority=NORMAL_PRIORITY_CLASS;
		break;
	case BELOW_NORMAL_PRIORITY_CLASS:
		m_MainProcessPriority=NORMAL_PRIORITY_CLASS;
		break;
	case NORMAL_PRIORITY_CLASS:
	case ABOVE_NORMAL_PRIORITY_CLASS:
	case HIGH_PRIORITY_CLASS:
		break;
	case REALTIME_PRIORITY_CLASS:
		m_MainProcessPriority=HIGH_PRIORITY_CLASS;
		break;
	default:
		m_MainProcessPriority=NORMAL_PRIORITY_CLASS;
	}
	//Xman end

	// Maella -Graph: display zoom-
	zoomFactor = (uint8)ini.GetInt(L"ZoomFactor", 1);
	if(zoomFactor < 1) zoomFactor = 1;
	// Maella end

	// Maella -MTU Configuration-
	MTU=(uint16)ini.GetInt(L"MTU",1340);
	if (MTU<500)
		MTU=500;
	if (MTU>1500)
		MTU=1500;
	// Maella end

	usedoublesendsize=ini.GetBool(L"usedoublesendsize",false);

	// Maella -Network Adapter Feedback Control-
	NAFCFullControl=ini.GetBool(L"NAFCFullControl", false);
	forceNAFCadapter=ini.GetInt(L"ForceNAFCAdapter",0);
	// Maella end

	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	datarateSamples=(uint8)ini.GetInt(L"DatarateSamples", 10);
	if(datarateSamples < 1) datarateSamples = 1;
	if(datarateSamples > 20) datarateSamples = 20;
	// Maella end

	// Maella -One-queue-per-file- (idea bloodymad)
	enableMultiQueue=ini.GetBool(L"EnableMultiQueue", false);
	enableReleaseMultiQueue=ini.GetBool(L"EnableReleaseMultiQueue", false);
	// Maella end

	//Xman Anti-Leecher
	m_antileecher=ini.GetBool(L"AntiLeecher",true);
	m_antileechername=ini.GetBool(L"AntiLeecherName",true);
	m_antighost=ini.GetBool(L"AntiGhost",true);
	m_antileecherbadhello= ini.GetBool(L"AntiLeecherBadHello", true );
	m_antileechersnafu= ini.GetBool(L"AntiLeecherSnafu", true);
	m_antileechermod= ini.GetBool(L"AntiLeecherMod", true);
	m_antileecherthief=ini.GetBool(L"AntiLeecherThief", true);
	m_antileecherspammer= ini.GetBool(L"AntiLeecherSpammer", true);
	m_antileecherxsexploiter= ini.GetBool(L"AntiLeecherXSExploiter", true);
	m_antileecheremcrypt= ini.GetBool(L"AntiLeecheremcrypt", true);
	m_antileechercommunity_action= ini.GetBool(L"AntiLeecherCommunity_Action", true);
	m_antileecherghost_action= ini.GetBool(L"AntiLeecherGhost_Action", true);
	m_antileecherthief_action= ini.GetBool(L"AntiLeecherThief_Action", true);

	//Xman end

	//Xman narrow font at transferwindow
	m_bUseNarrowFont=ini.GetBool(L"UseNarrowFont",false);
	//Xman end

	//Xman 1:3 Ratio
	m_13ratio=ini.GetBool(L"amountbasedratio",false);
	//Xman end

	//Xman chunk chooser
	m_chunkchooser=(uint8)ini.GetInt(L"chunkchooser",1); //1 = Mealla 2=zz
	if(m_chunkchooser<1 || m_chunkchooser>2)
		m_chunkchooser=1;
	//Xman end

	//Xman disable compression
	m_bUseCompression=ini.GetBool(L"UseCompression",true);



	//Xman count block/success send
	m_showblockratio=ini.GetBool(L"ShowSocketBlockRatio", false);
	m_dropblockingsockets=ini.GetBool(L"DropBlockingSockets", false);

	//Xman Funny-Nick (Stulle/Morph)
	m_bFunnyNick = ini.GetBool(_T("DisplayFunnyNick"), false);
	//Xman end

	//Xman remove unused AICH-hashes
	m_rememberAICH=ini.GetBool(L"rememberAICH",false);
	//Xman end

	//Xman smooth-accurate-graph
	usesmoothgraph=ini.GetBool(L"usesmoothgraph",false);

	// Mighty Knife: Static server handling
	SetDontRemoveStaticServers (ini.GetBool (L"DontRemoveStaticServers",true));
	// [end] Mighty Knife

	//Xman [MoNKi: -Downloaded History-]
	m_bHistoryShowShared = ini.GetBool(L"ShowSharedInHistory", false);
	//Xman end


	//Xman don't overwrite bak files if last sessions crashed
	m_last_session_aborted_in_an_unnormal_way = ini.GetBool(L"last_session_aborted_in_an_unnormal_way",false);

	//Xman end
	//--------------------------------------------------------------------------
	//End Section
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
	// Section: "TK4 additional settings" 
	//	       
	hotkey_enabled = ini.GetBool(L"HiddenHotKeyEnabled", false,L"TK4 Additional");
	hotkey_mod = ini.GetInt(L"HiddenHotKeyMod",MOD_CONTROL + MOD_ALT,L"TK4 Additional");
	hotkey_key = ini.GetInt(L"HiddenHotKeyKey", (int)'Y',L"TK4 Additional");
 	BadNameCheck = ini.GetBool(L"BadNameCheckEnabled", true,L"TK4 Additional");     //[FDC]  enable state
    doubleFDC = ini.GetBool(L"DoubleFDC", True,L"TK4 Additional");                 //[FDC]  enable state
    FDCSensitivity = ini.GetInt(L"FDCSensitivity", 92,L"TK4 Additional");           //[FDC]  load FDC sensitivity
 
}

//Xman Xtreme Upload
void CPreferences::CheckSlotSpeed()
{
	if(m_slotspeed<1.5f)
		m_slotspeed=1.5f;
	float maxSlotSpeed=3.0f;
	if (maxupload<6) maxSlotSpeed=2.0f;
	if (maxupload>=10)
		//insanidade deo boi
        maxSlotSpeed=maxupload/3;
		//insanidade do boi
	if (maxSlotSpeed>XTREME_MAX_SLOTSPEED)
		maxSlotSpeed=XTREME_MAX_SLOTSPEED;
	if(m_slotspeed>maxSlotSpeed)
		m_slotspeed=maxSlotSpeed;

	//Xman GlobalMaxHarlimit for fairness
	m_uMaxGlobalSources=(uint32)(maxupload*400 - (maxupload-10.0f)*100);
}
//Xman end


WORD CPreferences::GetWindowsVersion(){
	static bool bWinVerAlreadyDetected = false;
	if(!bWinVerAlreadyDetected)
	{
		bWinVerAlreadyDetected = true;
		m_wWinVer = DetectWinVersion();
	}
	return m_wWinVer;
}

UINT CPreferences::GetDefaultMaxConperFive(){
	switch (GetWindowsVersion()){
		case _WINVER_98_:
			return 5;
		case _WINVER_95_:
		case _WINVER_ME_:
			return MAXCON5WIN9X;
		case _WINVER_2K_:
		case _WINVER_XP_:
			return MAXCONPER5SEC;
		default:
			return MAXCONPER5SEC;
	}
}

//////////////////////////////////////////////////////////
// category implementations
//////////////////////////////////////////////////////////

void CPreferences::SaveCats(){

	// Cats
	CString catinif,ixStr,buffer;
	catinif.Format(L"%sCategory.ini",configdir);
	_tremove(catinif);

	CIni catini( catinif, L"Category" );
	catini.WriteInt(L"Count",catMap.GetCount()-1,L"General");
	for (int ix=0;ix<catMap.GetCount();ix++){
		ixStr.Format(L"Cat#%i",ix);
		catini.WriteString(L"Title",catMap.GetAt(ix)->title,ixStr);
		catini.WriteString(L"Incoming",catMap.GetAt(ix)->incomingpath,ixStr);
		catini.WriteString(L"Comment",catMap.GetAt(ix)->comment,ixStr);
		catini.WriteString(L"RegularExpression",catMap.GetAt(ix)->regexp,ixStr);
		buffer.Format(L"%lu",catMap.GetAt(ix)->color);
		catini.WriteString(L"Color",buffer,ixStr);
		catini.WriteInt(L"a4afPriority",catMap.GetAt(ix)->prio,ixStr); // ZZ:DownloadManager
		catini.WriteString(L"AutoCat",catMap.GetAt(ix)->autocat,ixStr);
		catini.WriteInt(L"Filter",catMap.GetAt(ix)->filter,ixStr);
		catini.WriteBool(L"FilterNegator",catMap.GetAt(ix)->filterNeg,ixStr);
		catini.WriteBool(L"AutoCatAsRegularExpression",catMap.GetAt(ix)->ac_regexpeval,ixStr);
        catini.WriteBool(L"downloadInAlphabeticalOrder", catMap.GetAt(ix)->downloadInAlphabeticalOrder!=FALSE, ixStr);
		catini.WriteBool(L"Care4All",catMap.GetAt(ix)->care4all,ixStr);
	}
}

void CPreferences::LoadCats() {
	CString ixStr,catinif,cat_a,cat_b,cat_c;
	TCHAR buffer[100];

	catinif.Format(L"%sCategory.ini",configdir);

	CIni catini( catinif, L"Category" );
	int max=catini.GetInt(L"Count",0,L"General");

	for (int ix=0;ix<=max;ix++){
		ixStr.Format(L"Cat#%i",ix);

		Category_Struct* newcat=new Category_Struct;
		newcat->filter=0;
		_stprintf(newcat->title,L"%s",catini.GetString(L"Title",L"",ixStr));
		_stprintf(newcat->incomingpath,L"%s",catini.GetString(L"Incoming",L"",ixStr));
		MakeFoldername(newcat->incomingpath);
		if (!IsShareableDirectory(newcat->incomingpath)){
			_sntprintf(newcat->incomingpath, ARRSIZE(newcat->incomingpath), L"%s", GetIncomingDir());
			MakeFoldername(newcat->incomingpath);
		}
		_stprintf(newcat->comment,L"%s",catini.GetString(L"Comment",L"",ixStr));
		newcat->prio =catini.GetInt(L"a4afPriority",PR_NORMAL,ixStr); // ZZ:DownloadManager
		newcat->filter=catini.GetInt(L"Filter",0,ixStr);
		newcat->filterNeg =catini.GetBool(L"FilterNegator",FALSE,ixStr);
		newcat->ac_regexpeval  =catini.GetBool(L"AutoCatAsRegularExpression",FALSE,ixStr);
		newcat->care4all=catini.GetBool(L"Care4All",FALSE,ixStr);

		newcat->regexp=catini.GetString(L"RegularExpression",L"",ixStr);
		newcat->autocat=catini.GetString(L"Autocat",L"",ixStr);
        newcat->downloadInAlphabeticalOrder = catini.GetBool(L"downloadInAlphabeticalOrder", FALSE, ixStr); // ZZ:DownloadManager

		_stprintf(buffer,L"%s",catini.GetString(L"Color",L"0",ixStr));
		newcat->color = _tstoi(buffer);

		AddCat(newcat);
		if (!PathFileExists(newcat->incomingpath))
			::CreateDirectory(newcat->incomingpath,0);
	}
}
void CPreferences::RemoveCat(int index)	{
	if (index>=0 && index<catMap.GetCount()) {
		Category_Struct* delcat;
		delcat=catMap.GetAt(index);
		catMap.RemoveAt(index);
		delete delcat;
	}
}

bool CPreferences::SetCatFilter(int index, int filter){
	if (index>=0 && index<catMap.GetCount()) {
		Category_Struct* cat;
		cat=catMap.GetAt(index);
		cat->filter=filter;
		return true;
	}

	return false;
}

int CPreferences::GetCatFilter(int index){
	if (index>=0 && index<catMap.GetCount()) {
		return catMap.GetAt(index)->filter;
	}

    return 0;
}

bool CPreferences::GetCatFilterNeg(int index){
	if (index>=0 && index<catMap.GetCount()) {
		return catMap.GetAt(index)->filterNeg;
	}

    return false;
}

void CPreferences::SetCatFilterNeg(int index, bool val) {
	if (index>=0 && index<catMap.GetCount()) {
		catMap.GetAt(index)->filterNeg=val;
	}
}


bool CPreferences::MoveCat(UINT from, UINT to){
	if (from>=(UINT)catMap.GetCount() || to >=(UINT)catMap.GetCount()+1 || from==to) return false;

	Category_Struct* tomove;

	tomove=catMap.GetAt(from);

	if (from < to) {
		catMap.RemoveAt(from);
		catMap.InsertAt(to-1,tomove);
	} else {
		catMap.InsertAt(to,tomove);
		catMap.RemoveAt(from+1);
	}

	SaveCats();

	return true;
}


bool CPreferences::IsInstallationDirectory(const CString& rstrDir)
{
	CString strFullPath;
	if (PathCanonicalize(strFullPath.GetBuffer(MAX_PATH), rstrDir))
		strFullPath.ReleaseBuffer();
	else
		strFullPath = rstrDir;

	// skip sharing of several special eMule folders
	if (!CompareDirectories(strFullPath, GetAppDir()))			// ".\eMule"
		return true;
	if (!CompareDirectories(strFullPath, GetConfigDir()))		// ".\eMule\config"
		return true;
	if (!CompareDirectories(strFullPath, GetWebServerDir()))	// ".\eMule\webserver"
		return true;
	if (!CompareDirectories(strFullPath, GetLangDir()))			// ".\eMule\lang"
		return true;

	return false;
}

bool CPreferences::IsShareableDirectory(const CString& rstrDir)
{
	if (IsInstallationDirectory(rstrDir))
		return false;

	CString strFullPath;
	if (PathCanonicalize(strFullPath.GetBuffer(MAX_PATH), rstrDir))
		strFullPath.ReleaseBuffer();
	else
		strFullPath = rstrDir;

	// skip sharing of several special eMule folders
	for (int i=0;i<GetTempDirCount();i++)
		if (!CompareDirectories(strFullPath, GetTempDir(i)))			// ".\eMule\temp"
			return false;

	return true;
}

void CPreferences::UpdateLastVC()
{
	struct tm tmTemp;
	versioncheckLastAutomatic = safe_mktime(CTime::GetCurrentTime().GetLocalTm(&tmTemp));
}



void CPreferences::SetWSPass(CString strNewPass)
{
	_stprintf(m_sWebPassword, L"%s", MD5Sum(strNewPass).GetHash().GetBuffer(0));
}

void CPreferences::SetWSLowPass(CString strNewPass)
{
	_stprintf(m_sWebLowPassword, L"%s", MD5Sum(strNewPass).GetHash().GetBuffer(0));
}

//remove moblie

/* Xman
void CPreferences::SetMaxUpload(UINT in)
{
	uint16 oldMaxUpload = (uint16)in;
	maxupload = (oldMaxUpload) ? oldMaxUpload : (uint16)UNLIMITED;
}

void CPreferences::SetMaxDownload(UINT in)
{
	uint16 oldMaxDownload = (uint16)in;
	maxdownload = (oldMaxDownload) ? oldMaxDownload : (uint16)UNLIMITED;
}
*/

void CPreferences::SetNetworkKademlia(bool val)
{
	networkkademlia = val;
}

CString CPreferences::GetHomepageBaseURLForLevel(int nLevel){
	CString tmp;
	if (nLevel == 0)
		tmp = L"http://emule-project.net";
	else if (nLevel == 1)
		tmp = L"http://www.emule-project.org";
	else if (nLevel == 2)
		tmp = L"http://www.emule-project.com";
	else if (nLevel < 100)
		tmp.Format(L"http://www%i.emule-project.net",nLevel-2);
	else if (nLevel < 150)
		tmp.Format(L"http://www%i.emule-project.org",nLevel);
	else if (nLevel < 200)
		tmp.Format(L"http://www%i.emule-project.com",nLevel);
	else if (nLevel == 200)
		tmp = L"http://emule.sf.net";
	else if (nLevel == 201)
		tmp = L"http://www.emuleproject.net";
	else if (nLevel == 202)
		tmp = L"http://sourceforge.net/projects/emule/";
	else
		tmp = L"http://www.emule-project.net";
	return tmp;
}

CString CPreferences::GetVersionCheckBaseURL(){
	CString tmp;
	UINT nWebMirrorAlertLevel = GetWebMirrorAlertLevel();
	if (nWebMirrorAlertLevel < 100)
		tmp = L"http://vcheck.emule-project.net";
	else if (nWebMirrorAlertLevel < 150)
		tmp.Format(L"http://vcheck%i.emule-project.org",nWebMirrorAlertLevel);
	else if (nWebMirrorAlertLevel < 200)
		tmp.Format(L"http://vcheck%i.emule-project.com",nWebMirrorAlertLevel);
	else if (nWebMirrorAlertLevel == 200)
		tmp = L"http://emule.sf.net";
	else if (nWebMirrorAlertLevel == 201)
		tmp = L"http://www.emuleproject.net";
	else
		tmp = L"http://vcheck.emule-project.net";
	return tmp;
}

bool CPreferences::IsDefaultNick(const CString strCheck){
	// not fast, but this function is called often
	for (int i = 0; i != 255; i++){
		if (GetHomepageBaseURLForLevel(i) == strCheck)
			return true;
	}
	return ( strCheck == L"http://emule-project.net" );
}

void CPreferences::SetUserNick(LPCTSTR pszNick)
{
	strNick = pszNick;
}

UINT CPreferences::GetWebMirrorAlertLevel(){
	// Known upcoming DDoS Attacks
	if (m_nWebMirrorAlertLevel == 0){
		// no threats known at this time
	}
	// end
	if (UpdateNotify())
		return m_nWebMirrorAlertLevel;
	else
		return 0;
}

bool CPreferences::IsRunAsUserEnabled(){
	return (GetWindowsVersion() == _WINVER_XP_ || GetWindowsVersion() == _WINVER_2K_) && m_bRunAsUser;
}

bool CPreferences::GetUseReBarToolbar()
{
	return GetReBarToolbar() && theApp.m_ullComCtrlVer >= MAKEDLLVERULL(5,8,0,0);
}

/* Xman
int	CPreferences::GetMaxGraphUploadRate(bool bEstimateIfUnlimited){
	if (maxGraphUploadRate != UNLIMITED || !bEstimateIfUnlimited){
		return maxGraphUploadRate;
	}
	else{
		if (maxGraphUploadRateEstimated != 0){
			return maxGraphUploadRateEstimated +4;
		}
		else
			return 16;
	}
}

void CPreferences::EstimateMaxUploadCap(uint32 nCurrentUpload){
	if (maxGraphUploadRateEstimated+1 < nCurrentUpload){
		maxGraphUploadRateEstimated = nCurrentUpload;
		if (maxGraphUploadRate == UNLIMITED && theApp.emuledlg && theApp.emuledlg->statisticswnd)
			theApp.emuledlg->statisticswnd->SetARange(false, thePrefs.GetMaxGraphUploadRate(true));
	}
}

void CPreferences::SetMaxGraphUploadRate(int in){
	maxGraphUploadRate	=(in) ? in : UNLIMITED;
}

bool CPreferences::IsDynUpEnabled()	{
	return m_bDynUpEnabled || maxGraphUploadRate == UNLIMITED;
}
*/

bool CPreferences::CanFSHandleLargeFiles()	{
	bool bResult = false;
	for (int i = 0; i != tempdir.GetCount(); i++){
		if (!IsFileOnFATVolume(tempdir.GetAt(i))){
			bResult = true;
			break;
		}
	}
	return bResult && !IsFileOnFATVolume(GetIncomingDir());
}

uint16 CPreferences::GetRandomTCPPort()
{
	// Get table of currently used TCP ports.
	PMIB_TCPTABLE pTCPTab = NULL;
	HMODULE hIpHlpDll = LoadLibrary(_T("iphlpapi.dll"));
	if (hIpHlpDll)
	{
		DWORD (WINAPI *pfnGetTcpTable)(PMIB_TCPTABLE, PDWORD, BOOL);
		(FARPROC&)pfnGetTcpTable = GetProcAddress(hIpHlpDll, "GetTcpTable");
		if (pfnGetTcpTable)
		{
			DWORD dwSize = 0;
			if ((*pfnGetTcpTable)(NULL, &dwSize, FALSE) == ERROR_INSUFFICIENT_BUFFER)
			{
				// The nr. of TCP entries could change (increase) between
				// the two function calls, allocate some more memory.
				dwSize += sizeof(pTCPTab->table[0]) * 50;
				pTCPTab = (PMIB_TCPTABLE)malloc(dwSize);
				if (pTCPTab)
				{
					if ((*pfnGetTcpTable)(pTCPTab, &dwSize, TRUE) != ERROR_SUCCESS)
					{
						free(pTCPTab);
						pTCPTab = NULL;
					}
				}
			}
		}
		FreeLibrary(hIpHlpDll);
	}

	const UINT uValidPortRange = 61000;
	int iMaxTests = uValidPortRange; // just in case, avoid endless loop
	uint16 nPort;
	bool bPortIsFree;
	do {
		// Get random port
		nPort = 4096 + (GetRandomUInt16() % uValidPortRange);

		// The port is by default assumed to be available. If we got a table of currently
		// used TCP ports, we verify that this port is currently not used in any way.
		bPortIsFree = true;
		if (pTCPTab)
		{
			uint16 nPortBE = htons(nPort);
			for (UINT e = 0; e < pTCPTab->dwNumEntries; e++)
			{
				// If there is a TCP entry in the table (regardless of its state), the port
				// is treated as not available.
				if (pTCPTab->table[e].dwLocalPort == nPortBE)
				{
					bPortIsFree = false;
					break;
				}
			}
		}
	}
	while (!bPortIsFree && --iMaxTests > 0);
	free(pTCPTab);
	return nPort;
}

uint16 CPreferences::GetRandomUDPPort()
{
	// Get table of currently used UDP ports.
	PMIB_UDPTABLE pUDPTab = NULL;
	HMODULE hIpHlpDll = LoadLibrary(_T("iphlpapi.dll"));
	if (hIpHlpDll)
	{
		DWORD (WINAPI *pfnGetUdpTable)(PMIB_UDPTABLE, PDWORD, BOOL);
		(FARPROC&)pfnGetUdpTable = GetProcAddress(hIpHlpDll, "GetUdpTable");
		if (pfnGetUdpTable)
		{
			DWORD dwSize = 0;
			if ((*pfnGetUdpTable)(NULL, &dwSize, FALSE) == ERROR_INSUFFICIENT_BUFFER)
			{
				// The nr. of UDP entries could change (increase) between
				// the two function calls, allocate some more memory.
				dwSize += sizeof(pUDPTab->table[0]) * 50;
				pUDPTab = (PMIB_UDPTABLE)malloc(dwSize);
				if (pUDPTab)
				{
					if ((*pfnGetUdpTable)(pUDPTab, &dwSize, TRUE) != ERROR_SUCCESS)
					{
						free(pUDPTab);
						pUDPTab = NULL;
					}
				}
			}
		}
		FreeLibrary(hIpHlpDll);
	}

	const UINT uValidPortRange = 61000;
	int iMaxTests = uValidPortRange; // just in case, avoid endless loop
	uint16 nPort;
	bool bPortIsFree;
	do {
		// Get random port
		nPort = 4096 + (GetRandomUInt16() % uValidPortRange);

		// The port is by default assumed to be available. If we got a table of currently
		// used UDP ports, we verify that this port is currently not used in any way.
		bPortIsFree = true;
		if (pUDPTab)
		{
			uint16 nPortBE = htons(nPort);
			for (UINT e = 0; e < pUDPTab->dwNumEntries; e++)
			{
				if (pUDPTab->table[e].dwLocalPort == nPortBE)
				{
					bPortIsFree = false;
					break;
				}
			}
		}
	}
	while (!bPortIsFree && --iMaxTests > 0);
	free(pUDPTab);
	return nPort;
}

//Xman
//upnp_start
uint16 CPreferences::GetPort(){
	if (m_iUPnPTCPExternal != 0)
		return m_iUPnPTCPExternal;

	return port;
}

uint16 CPreferences::GetUDPPort(){
	if (udpport == 0)
		return 0;

	if(m_iUPnPUDPExternal != 0)
		return m_iUPnPUDPExternal;

	return udpport;
}
//upnp_end