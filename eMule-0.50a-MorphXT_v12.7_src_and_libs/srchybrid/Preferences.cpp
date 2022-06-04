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
#include "VistaDefines.h"
#include "LastCommonRouteFinder.h" //MORPH - Added by SiRoB
#include "friendlist.h" //MORPH - Added by SiRoB, There is one slot friend or more

#pragma warning(disable:4516) // access-declarations are deprecated; member using-declarations provide a better alternative
#pragma warning(disable:4244) // conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable:4100) // unreferenced formal parameter
#pragma warning(disable:4702) // unreachable code
#pragma warning(disable:4189) // local variable is initialized but not referenced
#include <cryptopp/osrng.h>
#pragma warning(default:4189) // local variable is initialized but not referenced
#pragma warning(default:4702) // unreachable code
#pragma warning(default:4100) // unreferenced formal parameter
#pragma warning(default:4244) // conversion from 'type1' to 'type2', possible loss of data
#pragma warning(default:4516) // access-declarations are deprecated; member using-declarations provide a better alternative


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CPreferences thePrefs;

bool CPreferences::m_bUseCompression;//Xman disable compression
//Xman disable compression
/* MORPH : more dirs
CString CPreferences::m_astrDefaultDirs[13];
bool	CPreferences::m_abDefaultDirsCreated[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
*/
CString CPreferences::m_astrDefaultDirs[EMULE_FEEDSDIR+1];
// note, the following needs to be updated to the total number of dirs when changing that number
// currently it's 15
bool	CPreferences::m_abDefaultDirsCreated[EMULE_FEEDSDIR+1] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// MORPH END : more dirs
int		CPreferences::m_nCurrentUserDirMode = -1;
int		CPreferences::m_iDbgHeap;
CString	CPreferences::strNick;
UINT     CPreferences::minupload;  //MORPH  uint16 is not enough
UINT	CPreferences::maxupload; //MORPH  uint16 is not enough
UINT	CPreferences::maxdownload; //MORPH  uint16 is not enough
LPCSTR	CPreferences::m_pszBindAddrA;
CStringA CPreferences::m_strBindAddrA;
LPCWSTR	CPreferences::m_pszBindAddrW;
CStringW CPreferences::m_strBindAddrW;
// MORPH START leuk_he upnp bindaddr
DWORD	 CPreferences::m_dwUpnpBindAddr;
bool     CPreferences::m_bBindAddrIsDhcp;
// MORPH End leuk_he upnp bindaddr

uint16	CPreferences::port;
uint16	CPreferences::udpport;
uint16	CPreferences::nServerUDPPort;
UINT	CPreferences::maxconnections;
UINT	CPreferences::maxhalfconnections;
bool	CPreferences::m_bConditionalTCPAccept;
bool	CPreferences::reconnect;
bool	CPreferences::m_bUseServerPriorities;
bool	CPreferences::m_bUseUserSortedServerList;
CString	CPreferences::m_strIncomingDir;
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
bool	CPreferences::m_bUseSystemFontForMainControls;
bool	CPreferences::m_bRTLWindowsLayout;
CString	CPreferences::m_strSkinProfile;
CString	CPreferences::m_strSkinProfileDir;
bool	CPreferences::m_bAddServersFromServer;
bool	CPreferences::m_bAddServersFromClients;
UINT	CPreferences::maxsourceperfile;
UINT	CPreferences::trafficOMeterInterval;
UINT	CPreferences::statsInterval;
bool	CPreferences::m_bFillGraphs;
uchar	CPreferences::userhash[16];
WINDOWPLACEMENT CPreferences::EmuleWindowPlacement;
int		CPreferences::maxGraphDownloadRate;
int		CPreferences::maxGraphUploadRate;
uint32	CPreferences::maxGraphUploadRateEstimated = 0;
bool	CPreferences::beepOnError;
bool	CPreferences::m_bIconflashOnNewMessage;
bool	CPreferences::confirmExit;
DWORD	CPreferences::m_adwStatsColors[16]; //MORPH - Changed by SiRoB, Powershare display
bool	CPreferences::bHasCustomTaskIconColor;
bool	CPreferences::splashscreen;
bool	CPreferences::startupsound;//Commander - Added: Enable/Disable Startupsound
bool	CPreferences::sidebanner;//Commander - Added: Side Banner
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
CString	CPreferences::m_strStatsExpandedTreeItems;
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
CString CPreferences::m_strIRCServer;
CString	CPreferences::m_strIRCNick;
CString	CPreferences::m_strIRCChannelFilter;
bool	CPreferences::m_bIRCAddTimeStamp;
bool	CPreferences::m_bIRCUseChannelFilter;
UINT	CPreferences::m_uIRCChannelUserFilter;
CString	CPreferences::m_strIRCPerformString;
bool	CPreferences::m_bIRCUsePerform;
bool	CPreferences::m_bIRCGetChannelsOnConnect;
bool	CPreferences::m_bIRCAcceptLinks;
bool	CPreferences::m_bIRCAcceptLinksFriendsOnly;
bool	CPreferences::m_bIRCPlaySoundEvents;
bool	CPreferences::m_bIRCIgnoreMiscMessages;
bool	CPreferences::m_bIRCIgnoreJoinMessages;
bool	CPreferences::m_bIRCIgnorePartMessages;
bool	CPreferences::m_bIRCIgnoreQuitMessages;
bool	CPreferences::m_bIRCIgnoreEmuleAddFriendMsgs;
bool	CPreferences::m_bIRCAllowEmuleAddFriend;
bool	CPreferences::m_bIRCIgnoreEmuleSendLinkMsgs;
bool	CPreferences::m_bIRCJoinHelpChannel;
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
bool	CPreferences::m_bLogUlDlEvents;
// MORPH START - Added by Commander, WebCache 1.2e
bool	CPreferences::m_bLogICHEvents;//JP log ICH events
// MORPH END - Added by Commander, WebCache 1.2e
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
// MORPH START show less controls
bool	CPreferences::m_bShowLessControls;
// MORPH END  show less controls
bool	CPreferences::m_bTransflstRemain;
UINT	CPreferences::versioncheckdays;
bool	CPreferences::showRatesInTitle;
CString	CPreferences::m_strTxtEditor;
CString	CPreferences::m_strVideoPlayer;
CString CPreferences::m_strVideoPlayerArgs;
bool	CPreferences::moviePreviewBackup;
int		CPreferences::m_iPreviewSmallBlocks;
bool	CPreferences::m_bPreviewCopiedArchives;
int		CPreferences::m_iInspectAllFileTypes;
bool	CPreferences::m_bPreviewOnIconDblClk;
bool	CPreferences::m_bCheckFileOpen;
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
int   CPreferences::m_iCompressLevel;   // MORPH setable compresslevel [leuk_he]
bool	CPreferences::msgonlyfriends;
bool	CPreferences::msgsecure;
bool	CPreferences::m_bUseChatCaptchas;
UINT	CPreferences::filterlevel;
UINT	CPreferences::m_iFileBufferSize;
UINT	CPreferences::m_uFileBufferTimeLimit;
UINT	CPreferences::m_iQueueSize;
int		CPreferences::m_iCommitFiles;
UINT	CPreferences::maxmsgsessions;
time_t	CPreferences::versioncheckLastAutomatic; //vs2005
//MORPH START - Added by SiRoB, New Version check
uint32	CPreferences::mversioncheckLastAutomatic;
//MORPH START - Added by SiRoB, New Version check
CString	CPreferences::messageFilter;
CString	CPreferences::commentFilter;
CString	CPreferences::filenameCleanups;
CString	CPreferences::m_strDateTimeFormat;
CString	CPreferences::m_strDateTimeFormat4Log;
CString	CPreferences::m_strDateTimeFormat4Lists;
LOGFONT CPreferences::m_lfHyperText;
LOGFONT CPreferences::m_lfLogText;
COLORREF CPreferences::m_crLogError = RGB(255, 0, 0);
COLORREF CPreferences::m_crLogWarning = RGB(128, 0, 128);
COLORREF CPreferences::m_crLogSuccess = RGB(0, 0, 255);
//MORPH START - Added by SiRoB, Upload Splitting Class
COLORREF CPreferences::m_crLogUSC = RGB(10,180,50);
//MORPH END   - Added by SiRoB, Upload Splitting Class
	
int		CPreferences::m_iExtractMetaData;
bool	CPreferences::m_bAdjustNTFSDaylightFileTime = true;
bool	CPreferences::m_bRearrangeKadSearchKeywords;
//>>> [ionix] - iONiX::Advanced WebInterface Account Management
bool	CPreferences::m_bIonixWebsrv;
//<<< [ionix] - iONiX::Advanced WebInterface Account Management
CString	CPreferences::m_strWebPassword;
CString	CPreferences::m_strWebLowPassword;
CUIntArray CPreferences::m_aAllowedRemoteAccessIPs;
uint16	CPreferences::m_nWebPort;
#ifdef USE_OFFICIAL_UPNP
bool	CPreferences::m_bWebUseUPnP;
#endif
bool	CPreferences::m_bWebEnabled;
bool	CPreferences::m_bWebUseGzip;
int		CPreferences::m_nWebPageRefresh;
bool	CPreferences::m_bWebLowEnabled;
int		CPreferences::m_iWebTimeoutMins;
int		CPreferences::m_iWebFileUploadSizeLimitMB;
bool	CPreferences::m_bAllowAdminHiLevFunc;
CString	CPreferences::m_strTemplateFile;
ProxySettings CPreferences::proxy;
bool	CPreferences::showCatTabInfos;
bool	CPreferences::resumeSameCat;
bool	CPreferences::dontRecreateGraphs;
bool	CPreferences::autofilenamecleanup;
bool	CPreferences::m_bUseAutocompl;
bool	CPreferences::m_bShowDwlPercentage;
bool    CPreferences::m_bShowClientPercentage;  //Commander - Added: Client Percentage
bool	CPreferences::m_bRemoveFinishedDownloads;
UINT	CPreferences::m_iMaxChatHistory;
bool	CPreferences::m_bShowActiveDownloadsBold;
int		CPreferences::m_iSearchMethod;
bool	CPreferences::m_bAdvancedSpamfilter;
bool	CPreferences::m_bUseSecureIdent;
CString	CPreferences::m_strMMPassword;
bool	CPreferences::m_bMMEnabled;
uint16	CPreferences::m_nMMPort;
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
//MORPH START - Added by SiRoB, USSLog
bool	CPreferences::m_bDynUpLog;
//MORPH END   - Added by SiRoB, USSLog
bool	CPreferences::m_bUSSUDP; //MORPH - Added by SiRoB, USS UDP preferency
short   CPreferences::m_sPingDataSize;  //MORPH leuk_he ICMP ping datasize <> 0 setting
//MORPH START - Added by Commander, ClientQueueProgressBar
bool CPreferences::m_bClientQueueProgressBar;
//MORPH END - Added by Commander, ClientQueueProgressBar

//MORPH START - Added by Commander, FolderIcons
bool CPreferences::m_bShowFolderIcons;
//MORPH END - Added by Commander, FolderIcons

// ==> Slot Limit - Stulle
bool	CPreferences::m_bSlotLimitThree;
bool	CPreferences::m_bSlotLimitNum;
uint8	CPreferences::m_iSlotLimitNum;
// <== Slot Limit - Stulle

bool	CPreferences::enableHighProcess;//MORPH - Added by IceCream, high process priority
bool	CPreferences::enableDownloadInRed; //MORPH - Added by IceCream, show download in red
//MORPH START - Added by schnulli900, filter clients with failed downloads [Xman]
bool	CPreferences::m_bFilterClientFailedDown; 
//MORPH END   - Added by schnulli900, filter clients with failed downloads [Xman]
bool	CPreferences::enableAntiLeecher; //MORPH - Added by IceCream, enableAntiLeecher
bool	CPreferences::enableAntiCreditHack; //MORPH - Added by IceCream, enableAntiCreditHack
int	CPreferences::creditSystemMode; // EastShare - Added by linekin, creditsystem integration
bool	CPreferences::m_bEnableEqualChanceForEachFile;//Morph - added by AndCycle, Equal Chance For Each File
bool	CPreferences::m_bFollowTheMajority; // EastShare       - FollowTheMajority by AndCycle
int		CPreferences::m_iFairPlay; //EastShare	- FairPlay by AndCycle
bool	CPreferences::isautodynupswitching;//MORPH - Added by Yun.SF3, Auto DynUp changing
int	CPreferences::m_iPowershareMode; //MORPH - Added by SiRoB, Avoid misusing of powersharing
bool	CPreferences::m_bPowershareInternalPrio; //Morph - added by AndCyle, selective PS internal Prio
int	CPreferences::maxconnectionsswitchborder;
//EastShare Start- Added by Pretender, TBH-AutoBackup
bool	CPreferences::autobackup;
bool	CPreferences::autobackup2;
//EastShare End - Added by Pretender, TBH-AutoBackup

//MORPH START - Added by SiRoB, Datarate Average Time Management
int	CPreferences::m_iDownloadDataRateAverageTime;
int	CPreferences::m_iUploadDataRateAverageTime;
//MORPH END   - Added by SiRoB, Datarate Average Time Management

//MORPH START - Added by SiRoB, Upload Splitting Class
int	CPreferences::globaldataratefriend;
int	CPreferences::maxglobaldataratefriend;
int	CPreferences::globaldataratepowershare;
int	CPreferences::maxglobaldataratepowershare;
int	CPreferences::maxclientdataratefriend;
int	CPreferences::maxclientdataratepowershare;
int	CPreferences::maxclientdatarate;
//MORPH END   - Added by SiRoB, Upload Splitting Class
//MORPH START - Added by SiRoB, SLUGFILLER: lowIdRetry
int	CPreferences::LowIdRetries;
int	CPreferences::LowIdRetried;
//MORPH END   - Added by SiRoB, SLUGFILLER: lowIdRetry
//MORPH	Start	- Added by AndCycle, SLUGFILLER: Spreadbars - per file
int	CPreferences::m_iSpreadbarSetStatus;
//MORPH	End	- Added by AndCycle, SLUGFILLER: Spreadbars - per file
//MORPH START - Added by SiRoB, SLUGFILLER: hideOS
int	CPreferences::hideOS;
bool	CPreferences::selectiveShare;
//MORPH END   - Added by SiRoB, SLUGFILLER: hideOS

bool	CPreferences::infiniteQueue;	//Morph - added by AndCycle, SLUGFILLER: infiniteQueue

int	CPreferences::ShareOnlyTheNeed; //MORPH - Added by SiRoB, SHARE_ONLY_THE_NEED Wistily idea
int	CPreferences::PowerShareLimit; //MORPH - Added by SiRoB, POWERSHARE Limit
int	CPreferences::permissions;//MORPH - Added by SiRoB, Show Permissions

//EastShare Start - PreferShareAll by AndCycle
bool	CPreferences::shareall;	// SLUGFILLER: preferShareAll
//EastShare End - PreferShareAll by AndCycle

bool	CPreferences::m_bEnableChunkDots;
//EastShare - Added by Pretender, Option for ChunkDots




TCHAR	CPreferences::UpdateURLFakeList[256];//MORPH START - Added by milobac and Yun.SF3, FakeCheck, FakeReport, Auto-updating
TCHAR	CPreferences::UpdateURLIPFilter[256];//MORPH START added by Yun.SF3: Ipfilter.dat update
TCHAR	CPreferences::UpdateURLIP2Country[256];//Commander - Added: IP2Country auto-updating

bool	CPreferences::m_bPayBackFirst;//EastShare - added by AndCycle, Pay Back First
uint8	CPreferences::m_iPayBackFirstLimit;//MORPH - Added by SiRoB, Pay Back First Tweak
bool	CPreferences::m_bOnlyDownloadCompleteFiles;//EastShare - Added by AndCycle, Only download complete files v2.1 (shadow)
bool	CPreferences::m_bSaveUploadQueueWaitTime;//Morph - added by AndCycle, Save Upload Queue Wait Time (MSUQWT)
// EastShare START - Added by TAHO, .met file control
int		CPreferences::m_iKnownMetDays;
bool	CPreferences::m_bCompletlyPurgeOldKnownFiles;
bool	CPreferences::m_bRemoveAichImmediatly;
// EastShare END   - Added by TAHO, .met file control
bool	CPreferences::m_bDateFileNameLog;//Morph - added by AndCycle, Date File Name Log
bool	CPreferences::m_bDontRemoveSpareTrickleSlot;//Morph - added by AndCycle, Dont Remove Spare Trickle Slot
bool	CPreferences::m_bUseDownloadOverhead;//Morph - leuk_he use download overhead in upload
bool	CPreferences::m_bFunnyNick;//MORPH - Added by SiRoB, Optionnal funnynick display

//MORPH START - Added by milobac, FakeCheck, FakeReport, Auto-updating
SYSTEMTIME	CPreferences::m_FakesDatVersion;
bool	CPreferences::UpdateFakeStartup;
//MORPH END - Added by milobac, FakeCheck, FakeReport, Auto-updating
//MORPH START added by Yun.SF3: Ipfilter.dat update
bool	CPreferences::AutoUpdateIPFilter; //added by milobac: Ipfilter.dat update
SYSTEMTIME	CPreferences::m_IPfilterVersion; //added by milobac: Ipfilter.dat update
uint32	CPreferences::m_uIPFilterVersionNum; //MORPH - Added by Stulle, New IP Filter by Ozzy [Stulle/Ozzy]
//MORPH END added by Yun.SF3: Ipfilter.dat update

//Commander - Added: IP2Country Auto-updating - Start
bool	CPreferences::AutoUpdateIP2Country;
SYSTEMTIME	CPreferences::m_IP2CountryVersion;
//Commander - Added: IP2Country Auto-updating - End

//EastShare - added by AndCycle, IP to Country
int	CPreferences::m_iIP2CountryNameMode;
bool	CPreferences::m_bIP2CountryShowFlag;
//EastShare - added by AndCycle, IP to Country

// khaos::categorymod+
bool	CPreferences::m_bValidSrcsOnly;
bool	CPreferences::m_bShowCatNames;
bool	CPreferences::m_bActiveCatDefault;
bool	CPreferences::m_bSelCatOnAdd;
bool	CPreferences::m_bAutoSetResumeOrder;
bool	CPreferences::m_bSmallFileDLPush;
uint8	CPreferences::m_iStartDLInEmptyCats;
bool	CPreferences::m_bRespectMaxSources;
bool	CPreferences::m_bUseAutoCat;
bool	CPreferences::m_bAddRemovedInc;
// khaos::categorymod-

// MORPH START leuk_he disable catcolor
bool   CPreferences::m_bDisableCatColors;
// MORPH END   leuk_he disable catcolor
	

// khaos::kmod+
bool	CPreferences::m_bShowA4AFDebugOutput;
bool	CPreferences::m_bSmartA4AFSwapping;
uint8	CPreferences::m_iAdvancedA4AFMode; // 0 = disabled, 1 = balance, 2 = stack
bool	CPreferences::m_bUseSaveLoadSources;
// khaos::categorymod-
// khaos::accuratetimerem+
uint8	CPreferences::m_iTimeRemainingMode; // 0 = both, 1 = real time, 2 = average
// khaos::accuratetimerem-

//MORPH START - Added by SiRoB, ICS Optional
bool	CPreferences::m_bUseIntelligentChunkSelection;
//MORPH END   - Added by SiRoB, ICS Optional

// Mighty Knife: Community Visualization, Report hashing files, Log friendlist activities
TCHAR	CPreferences::m_sCommunityName [256];
bool	CPreferences::m_bReportHashingFiles;
bool	CPreferences::m_bLogFriendlistActivities;
// [end] Mighty Knife

// Mighty Knife: CRC32-Tag - not accessible in preferences dialog !
bool	CPreferences::m_bDontAddCRCToFilename;
bool	CPreferences::m_bCRC32ForceUppercase;
bool	CPreferences::m_bCRC32ForceAdding;
TCHAR	CPreferences::m_sCRC32Prefix [256];
TCHAR	CPreferences::m_sCRC32Suffix [256];
// [end] Mighty Knife

// Mighty Knife: Simple cleanup options
int      CPreferences::m_SimpleCleanupOptions;
CString  CPreferences::m_SimpleCleanupSearch;
CString  CPreferences::m_SimpleCleanupReplace;
CString  CPreferences::m_SimpleCleanupSearchChars;
CString  CPreferences::m_SimpleCleanupReplaceChars;
// [end] Mighty Knife

// Mighty Knife: Static server handling
bool	CPreferences::m_bDontRemoveStaticServers;
// [end] Mighty Knife

//MORPH START - Added by SiRoB, Smart Upload Control v2 (SUC) [lovelace]
bool	CPreferences::m_bSUCEnabled;
int		CPreferences::m_iSUCHigh;
int		CPreferences::m_iSUCLow;
int		CPreferences::m_iSUCPitch;
int		CPreferences::m_iSUCDrift;
bool	CPreferences::m_bSUCLog;
//MORPH END - Added by SiRoB, Smart Upload Control v2 (SUC) [lovelace]
//Commander - Added: Invisible Mode [TPT] - Start
bool	CPreferences::m_bInvisibleMode;		
UINT	CPreferences::m_iInvisibleModeHotKeyModifier;
char	CPreferences::m_cInvisibleModeHotKey;
//Commander - Added: Invisible Mode [TPT] - End

bool    CPreferences::m_bAllocFull;
bool	CPreferences::m_bShowSharedFilesDetails;
bool	CPreferences::m_bShowUpDownIconInTaskbar;
bool	CPreferences::m_bShowWin7TaskbarGoodies;
bool	CPreferences::m_bForceSpeedsToKB;
bool	CPreferences::m_bAutoShowLookups;
bool	CPreferences::m_bExtraPreviewWithMenu;

// ZZ:DownloadManager -->
bool    CPreferences::m_bA4AFSaveCpu;
// ZZ:DownloadManager <--
bool    CPreferences::m_bHighresTimer;
bool	CPreferences::m_bResolveSharedShellLinks;
bool	CPreferences::m_bKeepUnavailableFixedSharedDirs;
CStringList CPreferences::shareddir_list;
CStringList CPreferences::sharedsubdir_list;	// SLUGFILLER: shareSubdir
CStringList CPreferences::inactive_shareddir_list;	  // inactive sharesubdier
CStringList CPreferences::inactive_sharedsubdir_list;	// inactive sharesubdier

CStringList CPreferences::addresses_list;
CString CPreferences::m_strFileCommentsFilePath;
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
bool	CPreferences::m_bPartiallyPurgeOldKnownFiles;

bool	CPreferences::m_bNotifierSendMail;
CString	CPreferences::m_strNotifierMailServer;
CString	CPreferences::m_strNotifierMailSender;
CString	CPreferences::m_strNotifierMailReceiver;

bool	CPreferences::m_bWinaTransToolbar;
bool	CPreferences::m_bShowDownloadToolbar;

bool	CPreferences::m_bCryptLayerRequested;
bool	CPreferences::m_bCryptLayerSupported;
bool	CPreferences::m_bCryptLayerRequired;
uint32	CPreferences::m_dwKadUDPKey;
uint8	CPreferences::m_byCryptTCPPaddingLength;

#ifdef USE_OFFICIAL_UPNP
bool	CPreferences::m_bSkipWANIPSetup;
bool	CPreferences::m_bSkipWANPPPSetup;
bool	CPreferences::m_bEnableUPnP;
bool	CPreferences::m_bCloseUPnPOnExit;
bool	CPreferences::m_bIsWinServImplDisabled;
bool	CPreferences::m_bIsMinilibImplDisabled;
int		CPreferences::m_nLastWorkingImpl;
#endif

bool	CPreferences::m_bEnableSearchResultFilter;

bool	CPreferences::m_bIRCEnableSmileys;
bool	CPreferences::m_bMessageEnableSmileys;

BOOL	CPreferences::m_bIsRunningAeroGlass;
bool	CPreferences::m_bPreventStandby;
bool	CPreferences::m_bStoreSearches;

bool    CPreferences::m_bCryptLayerRequiredStrictServer; // MORPH lh require obfuscated server connection 

//MORPH START - Added by SiRoB, XML News [O²]
bool	CPreferences::enableNEWS;
//MORPH END   - Added by SiRoB, XML News [O²]

//MORPH START - Added by SiRoB, [MoNKi: -UPnPNAT Support-]
	bool	CPreferences::m_bUPnPNat;
	bool	CPreferences::m_bUPnPNatWeb;
	bool	CPreferences::m_bUPnPVerboseLog;
	uint16	CPreferences::m_iUPnPPort;
	bool	CPreferences::m_bUPnPLimitToFirstConnection;
	bool	CPreferences::m_bUPnPClearOnClose;
	int     CPreferences::m_iDetectuPnP; //leuk_he autodetect in startup wizard
	bool    CPreferences::m_bUPnPForceUpdate;
	// End MoNKi
//MORPH END   - Added by SiRoB, [MoNKi: -UPnPNAT Support-]

//MORPH START - Added by SiRoB, [MoNKi: -Random Ports-]
	bool	CPreferences::m_bRndPorts;
	uint16	CPreferences::m_iMinRndPort;
	uint16	CPreferences::m_iMaxRndPort;
	bool	CPreferences::m_bRndPortsResetOnRestart;
	uint16	CPreferences::m_iRndPortsSafeResetOnRestartTime;
	uint16	CPreferences::m_iCurrentTCPRndPort;
	uint16	CPreferences::m_iCurrentUDPRndPort;
//MORPH END   - Added by SiRoB, [MoNKi: -Random Ports-]

//MORPH START - Added by SiRoB, [MoNKi: -Improved ICS-Firewall support-]
	bool	CPreferences::m_bICFSupport;
	bool	CPreferences::m_bICFSupportFirstTime;
	bool	CPreferences::m_bICFSupportStatusChanged;
	bool	CPreferences::m_bICFSupportServerUDP;
//MORPH END   - Added by SiRoB, [MoNKi: -Improved ICS-Firewall support-]

//MORPH START - Added by SiRoB / Commander, Wapserver [emulEspaña]
	TCHAR	CPreferences::m_sWapTemplateFile[MAX_PATH];
	bool	CPreferences::m_bWapEnabled;
	uint16	CPreferences::m_nWapPort;
	int		CPreferences::m_iWapGraphWidth;
	int		CPreferences::m_iWapGraphHeight;
	bool	CPreferences::m_bWapFilledGraphs;
	int		CPreferences::m_iWapMaxItemsInPages;
	bool	CPreferences::m_bWapSendImages;
	bool	CPreferences::m_bWapSendGraphs;
	bool	CPreferences::m_bWapSendProgressBars;
	bool	CPreferences::m_bWapAllwaysSendBWImages;
	UINT	CPreferences::m_iWapLogsSize;
	CString CPreferences::m_strWapServerDir;
	CString	CPreferences::m_sWapPassword;
	CString	CPreferences::m_sWapLowPassword;
	bool	CPreferences::m_bWapLowEnabled;
	//MORPH END - Added by SiRoB / Commander, Wapserver [emulEspaña]

//MORPH START - Added by Stulle, Global Source Limit
UINT	CPreferences::m_uGlobalHL; 
bool	CPreferences::m_bGlobalHL;
//MORPH END   - Added by Stulle, Global Source Limit

// MORPH START leuk_he Advanced official preferences.
bool CPreferences::bMiniMuleAutoClose;
int  CPreferences::iMiniMuleTransparency; 
bool CPreferences::bCreateCrashDump;
bool CPreferences::bCheckComctl32 ;
bool CPreferences::bCheckShell32;
bool CPreferences::bIgnoreInstances;
CString CPreferences::sNotifierMailEncryptCertName;
CString CPreferences::sMediaInfo_MediaInfoDllPath ;
bool CPreferences::bMediaInfo_RIFF;
bool CPreferences::bMediaInfo_ID3LIB ;
#ifdef HAVE_QEDIT_H
bool CPreferences::m_bMediaInfo_MediaDet;
#endif//HAVE_QEDIT_H
bool CPreferences::m_bMediaInfo_RM;
#ifdef HAVE_WMSDK_H
bool CPreferences::m_bMediaInfo_WM;
#endif//HAVE_WMSDK_H
CString CPreferences::sInternetSecurityZone;
// MORPH END  leuk_he Advanced official preferences. 

//MORPH START - Added, Downloaded History [Monki/Xman]
bool	CPreferences::m_bHistoryShowShared;
//MORPH END   - Added, Downloaded History [Monki/Xman]

//MORPH START leuk_he:run as ntservice v1..
int		CPreferences::m_iServiceStartupMode;
int		CPreferences::m_iServiceOptLvl;
//MORPH END leuk_he:run as ntservice v1..
//MORPH START - Added by Stulle, Adjustable NT Service Strings
bool	CPreferences::m_bServiceStringsLoaded = false;
CString	CPreferences::m_strServiceName;
CString	CPreferences::m_strServiceDispName;
CString	CPreferences::m_strServiceDescr;
//MORPH END   - Added by Stulle, Adjustable NT Service Strings

bool CPreferences::m_bStaticIcon; //MORPH - Added, Static Tray Icon

bool CPreferences::m_bFakeAlyzerIndications; //MORPH - Added by Stulle, Fakealyzer [netfinity]

CString CPreferences::m_strBrokenURLs; //MORPH - Added by WiZaRd, Fix broken HTTP downloads

// ==> [MoNKi: -USS initial TTL-] - Stulle
uint8	CPreferences::m_iUSSinitialTTL;
// <== [MoNKi: -USS initial TTL-] - Stulle

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
	// khaos::kmod+ We need _better_ random numbers...  Sometimes.
	InitRandGen();
	// khaos::kmod-

	prefsExt = new Preferences_Ext_Struct;
	memset(prefsExt, 0, sizeof *prefsExt);

	m_strFileCommentsFilePath = GetMuleDirectory(EMULE_CONFIGDIR) + L"fileinfo.ini";

	//MORPH START - Added by SiRoB, XML News [O²]
	(void)GetDefaultDirectory(EMULE_FEEDSDIR, true); // just to create it
	//MORPH END   - Added by SiRoB, XML News [O²]

	///////////////////////////////////////////////////////////////////////////
	// Move *.log files from application directory into 'log' directory
	//
	CFileFind ff;
	BOOL bFoundFile = ff.FindFile(GetMuleDirectory(EMULE_EXECUTEABLEDIR) + L"eMule*.log", 0);
	while (bFoundFile)
	{
		bFoundFile = ff.FindNextFile();
		if (ff.IsDots() || ff.IsSystem() || ff.IsDirectory() || ff.IsHidden())
			continue;
		MoveFile(ff.GetFilePath(), GetMuleDirectory(EMULE_LOGDIR) + ff.GetFileName());
	}
	ff.Close();

	///////////////////////////////////////////////////////////////////////////
	// Move 'downloads.txt/bak' files from application and/or data-base directory
	// into 'config' directory
	//
	if (PathFileExists(GetMuleDirectory(EMULE_DATABASEDIR) + L"downloads.txt"))
		MoveFile(GetMuleDirectory(EMULE_DATABASEDIR) + L"downloads.txt", GetMuleDirectory(EMULE_CONFIGDIR) + L"downloads.txt");
	if (PathFileExists(GetMuleDirectory(EMULE_DATABASEDIR) + L"downloads.bak"))
		MoveFile(GetMuleDirectory(EMULE_DATABASEDIR) + L"downloads.bak", GetMuleDirectory(EMULE_CONFIGDIR) + L"downloads.bak");
	if (PathFileExists(GetMuleDirectory(EMULE_EXECUTEABLEDIR) + L"downloads.txt"))
		MoveFile(GetMuleDirectory(EMULE_EXECUTEABLEDIR) + L"downloads.txt", GetMuleDirectory(EMULE_CONFIGDIR) + L"downloads.txt");
	if (PathFileExists(GetMuleDirectory(EMULE_EXECUTEABLEDIR) + L"downloads.bak"))
		MoveFile(GetMuleDirectory(EMULE_EXECUTEABLEDIR) + L"downloads.bak", GetMuleDirectory(EMULE_CONFIGDIR) + L"downloads.bak");

	CreateUserHash();

	// load preferences.dat or set standart values
	CString strFullPath;
	strFullPath = GetMuleDirectory(EMULE_CONFIGDIR) + L"preferences.dat";
	FILE* preffile = _tfsopen(strFullPath, L"rb", _SH_DENYWR);

	LoadPreferences();

	if (!preffile){
		SetStandartValues();
	}
	else{
		if (fread(prefsExt,sizeof(Preferences_Ext_Struct),1,preffile) != 1 || ferror(preffile))
			SetStandartValues();

		md4cpy(userhash, prefsExt->userhash);
		EmuleWindowPlacement = prefsExt->EmuleWindowPlacement;

		fclose(preffile);
		smartidstate = 0;
	}

	// shared directories
	strFullPath = GetMuleDirectory(EMULE_CONFIGDIR) + L"shareddir.dat";
	CStdioFile* sdirfile = new CStdioFile();
	bool bIsUnicodeFile = IsUnicodeFile(strFullPath); // check for BOM
	// open the text file either in ANSI (text) or Unicode (binary), this way we can read old and new files
	// with nearly the same code..
	if (sdirfile->Open(strFullPath, CFile::modeRead | CFile::shareDenyWrite | (bIsUnicodeFile ? CFile::typeBinary : 0)))
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

				// SLUGFILLER: SafeHash remove - removed installation dir unsharing
				/*
				if (!IsShareableDirectory(toadd))
					continue;
				*/

				// Skip non-existing directories from fixed disks only
				int iDrive = PathGetDriveNumber(toadd);
				if (iDrive >= 0 && iDrive <= 25) {
					WCHAR szRootPath[4] = L" :\\";
					szRootPath[0] = (WCHAR)(L'A' + iDrive);
					if (GetDriveType(szRootPath) == DRIVE_FIXED && !m_bKeepUnavailableFixedSharedDirs) {
						if (_taccess(toadd, 0) != 0)
						// MORPH START sharesubdir 
						{
							theApp.QueueLogLine(false,_T("Dir %s Added to inaccesable directories") , toadd);  // Note thate queue need to be used because logwindow is not initialized (logged time is wrong)
							inactive_shareddir_list.AddHead(toadd); // sharedsubdir inactive
						// MORPH END sharesubdir 
							continue;
						} // MORPH sharesubdir 
					}
				}

				if (toadd.Right(1) != L'\\')
					toadd.Append(L"\\");
				shareddir_list.AddHead(toadd);
			}
			sdirfile->Close();
		}
		catch (CFileException* ex) {
			ASSERT(0);
			ex->Delete();
		}
	}
	delete sdirfile;

	// SLUGFILLER START: shareSubdir
	// shared directories with subdirectories
    TCHAR * fullpath = new TCHAR[_tcslen(GetMuleDirectory(EMULE_CONFIGDIR) ) + MAX_PATH];
	_stprintf(fullpath, _T("%s\\sharedsubdir.ini"), GetMuleDirectory(EMULE_CONFIGDIR) );
	sdirfile = new CStdioFile();
	bIsUnicodeFile = IsUnicodeFile(fullpath); // check for BOM
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
				toadd.Trim(_T("\r\n")); // need to trim '\r' in binary mode
				TCHAR szFullPath[MAX_PATH];
				if (PathCanonicalize(szFullPath, toadd))
					toadd = szFullPath;

				if (_taccess(toadd, 0) == 0) { // only add directories which still exist
					if (toadd.Right(1) != _T('\\'))
						toadd.Append(_T("\\"));
					sharedsubdir_list.AddHead(toadd);
				}
				else  {
					   theApp.QueueLogLine(false,_T("Dir %s Added to inaccesable directories with subdir") , toadd); 
					inactive_sharedsubdir_list.AddHead(toadd);	// sharedsubdir	inactive 
				}

			}
			sdirfile->Close();
		}
		catch (CFileException* ex) {
			ASSERT(0);
			ex->Delete();
		}
	}
	delete sdirfile;
	delete[] fullpath;
	// SLUGFILLER END: shareSubdir
	
	// serverlist addresses
	// filename update to reasonable name
	if (PathFileExists(GetMuleDirectory(EMULE_CONFIGDIR) + L"adresses.dat") ) {
		if (PathFileExists(GetMuleDirectory(EMULE_CONFIGDIR) + L"addresses.dat") )
			DeleteFile(GetMuleDirectory(EMULE_CONFIGDIR) + L"adresses.dat");
		else 
			MoveFile(GetMuleDirectory(EMULE_CONFIGDIR) + L"adresses.dat", GetMuleDirectory(EMULE_CONFIGDIR) + L"addresses.dat");
	}

	strFullPath = GetMuleDirectory(EMULE_CONFIGDIR) + L"addresses.dat";
	sdirfile = new CStdioFile();
	bIsUnicodeFile = IsUnicodeFile(strFullPath);
	if (sdirfile->Open(strFullPath, CFile::modeRead | CFile::shareDenyWrite | (bIsUnicodeFile ? CFile::typeBinary : 0)))
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

	userhash[5] = 14;
	userhash[14] = 111;

	// Explicitly inform the user about errors with incoming/temp folders!
	if (!PathFileExists(GetMuleDirectory(EMULE_INCOMINGDIR)) && !::CreateDirectory(GetMuleDirectory(EMULE_INCOMINGDIR),0)) {
		CString strError;
		strError.Format(GetResString(IDS_ERR_CREATE_DIR), GetResString(IDS_PW_INCOMING), GetMuleDirectory(EMULE_INCOMINGDIR), GetErrorMessage(GetLastError()));
		AfxMessageBox(strError, MB_ICONERROR);

		m_strIncomingDir = GetDefaultDirectory(EMULE_INCOMINGDIR, true); // will also try to create it if needed
		if (!PathFileExists(GetMuleDirectory(EMULE_INCOMINGDIR))){
			strError.Format(GetResString(IDS_ERR_CREATE_DIR), GetResString(IDS_PW_INCOMING), GetMuleDirectory(EMULE_INCOMINGDIR), GetErrorMessage(GetLastError()));
			AfxMessageBox(strError, MB_ICONERROR);
		}
	}
	if (!PathFileExists(GetTempDir()) && !::CreateDirectory(GetTempDir(),0)) {
		CString strError;
		strError.Format(GetResString(IDS_ERR_CREATE_DIR), GetResString(IDS_PW_TEMP), GetTempDir(), GetErrorMessage(GetLastError()));
		AfxMessageBox(strError, MB_ICONERROR);

		tempdir.SetAt(0, GetDefaultDirectory(EMULE_TEMPDIR, true)); // will also try to create it if needed);
		if (!PathFileExists(GetTempDir())){
			strError.Format(GetResString(IDS_ERR_CREATE_DIR), GetResString(IDS_PW_TEMP), GetTempDir(), GetErrorMessage(GetLastError()));
			AfxMessageBox(strError, MB_ICONERROR);
		}
	}
	// khaos::kmod+ Source Lists directory
	for (int i=0;i<tempdir.GetCount();i++) { // leuk_he: multiple temp dirs for save sources. 
	   CString sSourceListsPath = CString(thePrefs.GetTempDir(i)) + _T("\\Source Lists");
	   if (UseSaveLoadSources() && !PathFileExists(sSourceListsPath.GetBuffer()) && !::CreateDirectory(sSourceListsPath.GetBuffer(), 0)) {
	  	  CString strError;
		  strError.Format(_T("Failed to create source lists directory \"%s\" - %s"), sSourceListsPath, GetErrorMessage(GetLastError()));
		  AfxMessageBox(strError, MB_ICONERROR);
	   }
	}
	// khaos::kmod-

	// Create 'skins' directory
	if (!PathFileExists(GetMuleDirectory(EMULE_SKINDIR)) && !CreateDirectory(GetMuleDirectory(EMULE_SKINDIR), 0)) {
		m_strSkinProfileDir = GetDefaultDirectory(EMULE_SKINDIR, true); // will also try to create it if needed
	}

	// Create 'toolbars' directory
	if (!PathFileExists(GetMuleDirectory(EMULE_TOOLBARDIR)) && !CreateDirectory(GetMuleDirectory(EMULE_TOOLBARDIR), 0)) {
		m_sToolbarBitmapFolder = GetDefaultDirectory(EMULE_TOOLBARDIR, true); // will also try to create it if needed;
	}


	if (isnulmd4(userhash))
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

	//MORPH START - Added by SiRoB, New Version check
	mversioncheckLastAutomatic=0;
	//MORPH END   - Added by SiRoB, New Version check
//	Save();
}

// SLUGFILLER: SafeHash remove - global form of IsTempFile unnececery
/*
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
	for (int i = 0; i < _countof(_apszNotSharedExts); i++){
		UINT uNum;
		TCHAR iChar;
		// "misuse" the 'scanf' function for a very simple pattern scanning.
		if (_stscanf(strNameLower, _apszNotSharedExts[i], &uNum, &iChar) == 2 && iChar == L'|')
			return true;
	}

	return false;
}
*/

//MORPH - Added by SiRoB, ZZ Ratio
uint8 CPreferences::IsZZRatioDoesWork(){
	uint8 ret = 0;
	if (theApp.downloadqueue->IsFilesPowershared())
		ret |= 1;
	if (theApp.friendlist->IsFriendSlot() && (thePrefs.GetGlobalDataRateFriend() > 3*1024 || thePrefs.GetGlobalDataRateFriend() == 0))
		ret |= 2;
	if (GetMaxUpload()<10)
		ret |= 4;
	if (theStats.GetAvgUploadRate(0)<10)
		ret |= 8;
	return ret;
}
//MORPH - Added by SiRoB, ZZ ratio

UINT CPreferences::GetMaxDownload(){	 //MORPH  uint16 is not enough
    return ((UINT)GetMaxDownloadInBytesPerSec()/1024); //MORPH  uint16 is not enough
}

//MORPH START - Removed by Stulle, Official ul/dl ratio restrictions
/*
uint64 CPreferences::GetMaxDownloadInBytesPerSec(bool dynamic){
	//dont be a Lam3r :)
	UINT maxup;
	if (dynamic && thePrefs.IsDynUpEnabled() && theApp.uploadqueue->GetWaitingUserCount() != 0 && theApp.uploadqueue->GetDatarate() != 0) {
		maxup = theApp.uploadqueue->GetDatarate();
	} else {
		maxup = GetMaxUpload()*1024;
	}

	if (maxup < 4*1024)
		return (((maxup < 10*1024) && ((uint64)maxup*3 < maxdownload*1024)) ? (uint64)maxup*3 : maxdownload*1024);
	return (((maxup < 10*1024) && ((uint64)maxup*4 < maxdownload*1024)) ? (uint64)maxup*4 : maxdownload*1024);
}
*/
uint64 CPreferences::GetMaxDownloadInBytesPerSec(bool){
	return maxdownload*1024;
}
//MORPH END   - Removed by Stulle, Official ul/dl ratio restrictions

//MORPH START - Added by SiRoB, Upload Splitting Class
uint32	CPreferences::GetGlobalDataRateFriend()
{
	return globaldataratefriend*1024;//_UI32_MAX;
}
uint32	CPreferences::GetMaxGlobalDataRateFriend()
{
	return maxglobaldataratefriend;
}
uint32	CPreferences::GetMaxClientDataRateFriend()
{
	return maxclientdataratefriend*1024;
}
uint32	CPreferences::GetGlobalDataRatePowerShare()
{
	return globaldataratepowershare*1024;//_UI32_MAX;
}
uint32	CPreferences::GetMaxGlobalDataRatePowerShare()
{
	return maxglobaldataratepowershare;
}
uint32	CPreferences::GetMaxClientDataRatePowerShare()
{
	return maxclientdataratepowershare*1024;
}
uint32	CPreferences::GetMaxClientDataRate()
{
	return maxclientdatarate*1024;
}
//MORPH END   - Added by SiRoB, Upload Splitting Class
// -khaos--+++> A whole bunch of methods!  Keep going until you reach the end tag.
void CPreferences::SaveStats(int bBackUp){
	// This function saves all of the new statistics in my addon.  It is also used to
	// save backups for the Reset Stats function, and the Restore Stats function (Which is actually LoadStats)
	// bBackUp = 0: DEFAULT; save to statistics.ini
	// bBackUp = 1: Save to statbkup.ini, which is used to restore after a reset
	// bBackUp = 2: Save to statbkuptmp.ini, which is temporarily created during a restore and then renamed to statbkup.ini

	CString strFullPath(GetMuleDirectory(EMULE_CONFIGDIR));
	if (bBackUp == 1)
		strFullPath += L"statbkup.ini";
	else if (bBackUp == 2)
		strFullPath += L"statbkuptmp.ini";
	else
		strFullPath += L"statistics.ini";
	
	CIni ini(strFullPath, L"Statistics");

	// Save cumulative statistics to preferences.ini, going in order as they appear in CStatisticsDlg::ShowStatistics.
	// We do NOT SET the values in prefs struct here.

    // Save Cum Down Data
	ini.WriteUInt64(L"TotalDownloadedBytes", theStats.sessionReceivedBytes + GetTotalDownloaded());
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
	ini.WriteUInt64(L"TotalUploadedBytes", theStats.sessionSentBytes + GetTotalUploaded());
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

	// Download Rate Average
	tempRate = theStats.GetAvgDownloadRate(AVG_TOTAL);
	ini.WriteFloat(L"ConnAvgDownRate", tempRate);
	
	// Max Download Rate Average
	if (tempRate > GetConnMaxAvgDownRate())
		SetConnMaxAvgDownRate(tempRate);
	ini.WriteFloat(L"ConnMaxAvgDownRate", GetConnMaxAvgDownRate());
	
	// Max Download Rate
	tempRate = (float)theApp.downloadqueue->GetDatarate() / 1024;
	if (tempRate > GetConnMaxDownRate())
		SetConnMaxDownRate(tempRate);
	ini.WriteFloat(L"ConnMaxDownRate", GetConnMaxDownRate());
	
	// Upload Rate Average
	tempRate = theStats.GetAvgUploadRate(AVG_TOTAL);
	ini.WriteFloat(L"ConnAvgUpRate", tempRate);
	
	// Max Upload Rate Average
	if (tempRate > GetConnMaxAvgUpRate())
		SetConnMaxAvgUpRate(tempRate);
	ini.WriteFloat(L"ConnMaxAvgUpRate", GetConnMaxAvgUpRate());
	
	// Max Upload Rate
	tempRate = (float)theApp.uploadqueue->GetDatarate() / 1024;
	if (tempRate > GetConnMaxUpRate())
		SetConnMaxUpRate(tempRate);
	ini.WriteFloat(L"ConnMaxUpRate", GetConnMaxUpRate());
	
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

	CIni ini(GetMuleDirectory(EMULE_CONFIGDIR) + L"statistics.ini", L"Statistics" );

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
		case 0:
			// for transition...
			if (PathFileExists(GetMuleDirectory(EMULE_CONFIGDIR) + L"statistics.ini"))
				sINI.Format(L"%sstatistics.ini", GetMuleDirectory(EMULE_CONFIGDIR));
			else
				sINI.Format(L"%spreferences.ini", GetMuleDirectory(EMULE_CONFIGDIR));
			break;
		case 1:
			sINI.Format(L"%sstatbkup.ini", GetMuleDirectory(EMULE_CONFIGDIR));
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
	cumConnMaxAvgDownRate			= ini.GetFloat(L"ConnMaxAvgDownRate");
	cumConnMaxDownRate				= ini.GetFloat(L"ConnMaxDownRate");
	cumConnAvgUpRate				= ini.GetFloat(L"ConnAvgUpRate");
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
		sINIBackUp.Format(L"%sstatbkuptmp.ini", GetMuleDirectory(EMULE_CONFIGDIR));
		if (findBackUp.FindFile(sINIBackUp)){
			::DeleteFile(sINI);				// Remove the backup that we just restored from
			::MoveFile(sINIBackUp, sINI);	// Rename our temporary backup to the normal statbkup.ini filename.
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
			_tcsftime(szDateReset, _countof(szDateReset), formatLong ? GetDateTimeFormat() : L"%c", statsReset);
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
	CString strFullPath;
	strFullPath = GetMuleDirectory(EMULE_CONFIGDIR) + L"preferences.dat";

	FILE* preffile = _tfsopen(strFullPath, L"wb", _SH_DENYWR);
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
	// khaos::categorymod+ We need to SaveCats() each time we exit eMule.
	SaveCats();
	// khaos::categorymod-

	strFullPath = GetMuleDirectory(EMULE_CONFIGDIR) + L"shareddir.dat";
	CStdioFile sdirfile;
	if (sdirfile.Open(strFullPath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite | CFile::typeBinary))
	{
		try{
			// write Unicode byte-order mark 0xFEFF
			WORD wBOM = 0xFEFF;
			sdirfile.Write(&wBOM, sizeof(wBOM));

			for (POSITION pos = shareddir_list.GetHeadPosition();pos != 0;){
				sdirfile.WriteString(shareddir_list.GetNext(pos));
				sdirfile.Write(L"\r\n", sizeof(TCHAR)*2);
			}
      // MORPH START sharesubdir inactive shares 
			for (POSITION pos = inactive_shareddir_list.GetHeadPosition();pos != 0;){  // inactive sharedir
				sdirfile.WriteString(inactive_shareddir_list.GetNext(pos).GetBuffer());
				sdirfile.Write(_T("\r\n"), sizeof(TCHAR)*2);
			}
       // MORPH EEND sharesubdir 
			if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !theApp.emuledlg->IsRunning())){
				sdirfile.Flush(); // flush file stream buffers to disk buffers
				if (_commit(_fileno(sdirfile.m_pStream)) != 0) // commit disk buffers to disk
					AfxThrowFileException(CFileException::hardIO, GetLastError(), sdirfile.GetFileName());
			}
			sdirfile.Close();
		}
		catch(CFileException* error){
			TCHAR buffer[MAX_CFEXP_ERRORMSG];
			error->GetErrorMessage(buffer,_countof(buffer));
			if (thePrefs.GetVerbose())
				AddDebugLogLine(true, L"Failed to save %s - %s", strFullPath, buffer);
			error->Delete();
		}
	}
	else
		error = true;

	// SLUGFILLER START: shareSubdir
	TCHAR *fullpath = new TCHAR[_tcslen(GetMuleDirectory(EMULE_CONFIGDIR)) + 19];
	_stprintf(fullpath, _T("%s\\sharedsubdir.ini"), GetMuleDirectory(EMULE_CONFIGDIR));
	if (sdirfile.Open(fullpath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite | CFile::typeBinary))
	{
		try{
			// write Unicode byte-order mark 0xFEFF
			WORD wBOM = 0xFEFF;
			sdirfile.Write(&wBOM, sizeof(wBOM));

			for (POSITION pos = sharedsubdir_list.GetHeadPosition();pos != 0;){
				sdirfile.WriteString(sharedsubdir_list.GetNext(pos).GetBuffer());
				sdirfile.Write(_T("\r\n"), sizeof(TCHAR)*2);
			}
			if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !theApp.emuledlg->IsRunning())){
				sdirfile.Flush(); // flush file stream buffers to disk buffers
			}
			for (POSITION pos = inactive_sharedsubdir_list.GetHeadPosition();pos != 0;){  // inactive sharesubdir
				sdirfile.WriteString(inactive_sharedsubdir_list.GetNext(pos).GetBuffer());
				sdirfile.Write(_T("\r\n"), sizeof(TCHAR)*2);
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
				AddDebugLogLine(true,_T("Failed to save %s - %s"), fullpath, buffer);
			error->Delete();
		}
	}
	else
		error = true;
	delete[] fullpath;
	fullpath=NULL;
	// SLUGFILLER END: shareSubdir
	::CreateDirectory(GetMuleDirectory(EMULE_INCOMINGDIR), 0);
	::CreateDirectory(GetTempDir(), 0);
	return error;
}

void CPreferences::CreateUserHash()
{
	CryptoPP::AutoSeededRandomPool rng;
	rng.GenerateBlock(userhash, 16);
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
	//MORPH START - Added by SiRoB, [itsonlyme: -modname-]
	/*
	ini.WriteString(L"AppVersion", theApp.m_strCurVersionLong);
	*/
	ini.WriteString(L"AppVersion", theApp.m_strCurVersionLong + L" [" + theApp.m_strModLongVersion + L"]");
	//MORPH END   - Added by SiRoB, [itsonlyme: -modname-]
	//---

#ifdef _DEBUG
	ini.WriteInt(L"DebugHeap", m_iDbgHeap);
#endif

	ini.WriteStringUTF8(L"Nick", strNick);
	ini.WriteString(L"IncomingDir", m_strIncomingDir);
	
	ini.WriteString(L"TempDir", tempdir.GetAt(0));

	CString tempdirs;
	for (int i=1;i<tempdir.GetCount();i++) {
		tempdirs.Append(tempdir.GetAt(i) );
		if (i+1<tempdir.GetCount())
			tempdirs.Append(L"|");
	}
	ini.WriteString(L"TempDirs", tempdirs);

    ini.WriteInt(L"MinUpload", minupload);
	ini.WriteInt(L"MaxUpload",maxupload);
	ini.WriteInt(L"MaxDownload",maxdownload);
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
	ini.WriteBool(L"StatsFillGraphs",m_bFillGraphs);
	ini.WriteInt(L"DownloadCapacity",maxGraphDownloadRate);
	ini.WriteInt(L"UploadCapacityNew",maxGraphUploadRate);
	ini.WriteInt(L"DeadServerRetry",m_uDeadServerRetries);
	ini.WriteInt(L"ServerKeepAliveTimeout",m_dwServerKeepAliveTimeout);
	ini.WriteString(L"BindAddr",m_pszBindAddrW); //MORPH leuk_he bindaddr
	// MORPH START leuk_he upnp bindaddr
	ini.WriteString(L"UpnpBindAddr", ipstr(htonl(GetUpnpBindAddr())));
	ini.WriteBool(L"UpnpBindAddrDhcp",GetUpnpBindDhcp());
    // MORPH END leuk_he upnp bindaddr
	ini.WriteInt(L"SplitterbarPosition",splitterbarPosition);
	ini.WriteInt(L"SplitterbarPositionServer",splitterbarPositionSvr);
	ini.WriteInt(L"SplitterbarPositionStat",splitterbarPositionStat+1);
	ini.WriteInt(L"SplitterbarPositionStat_HL",splitterbarPositionStat_HL/*+1 BSB prevent moving bars down*/);
	ini.WriteInt(L"SplitterbarPositionStat_HR",splitterbarPositionStat_HR/*+1 BSB*/ );
	ini.WriteInt(L"SplitterbarPositionFriend",splitterbarPositionFriend);
	ini.WriteInt(L"SplitterbarPositionIRC",splitterbarPositionIRC);
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
	if (IsRunningAeroGlassTheme())
		ini.WriteBool(L"MinToTray_Aero",mintotray);
	else
		ini.WriteBool(L"MinToTray",mintotray);
	ini.WriteBool(L"PreventStandby", m_bPreventStandby);
	ini.WriteBool(L"StoreSearches", m_bStoreSearches);
	ini.WriteBool(L"AddServersFromServer",m_bAddServersFromServer);
	ini.WriteBool(L"AddServersFromClient",m_bAddServersFromClients);
	ini.WriteBool(L"Splashscreen",splashscreen);
	ini.WriteBool(L"Startupsound",startupsound);//Commander - Added: Enable/Disable Startupsound
	ini.WriteBool(L"Sidebanner",sidebanner);//Commander - Added: Side Banner
	ini.WriteBool(L"BringToFront",bringtoforeground);
	ini.WriteBool(L"TransferDoubleClick",transferDoubleclick);
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
	ini.WriteBool(L"ResolveSharedShellLinks",m_bResolveSharedShellLinks);
	ini.WriteString(L"YourHostname",m_strYourHostname);
	ini.WriteBool(L"CheckFileOpen",m_bCheckFileOpen);
	ini.WriteBool(L"ShowWin7TaskbarGoodies", m_bShowWin7TaskbarGoodies );

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

	ini.WriteString(L"TxtEditor",m_strTxtEditor);
	ini.WriteString(L"VideoPlayer",m_strVideoPlayer);
	ini.WriteString(L"VideoPlayerArgs",m_strVideoPlayerArgs);
	ini.WriteString(L"MessageFilter",messageFilter);
	ini.WriteString(L"CommentFilter",commentFilter);
	ini.WriteString(L"DateTimeFormat",GetDateTimeFormat());
	ini.WriteString(L"DateTimeFormat4Log",GetDateTimeFormat4Log());
	ini.WriteString(L"DateTimeFormat4Lists",GetDateTimeFormat4Lists());
	ini.WriteString(L"WebTemplateFile",m_strTemplateFile);
	ini.WriteString(L"FilenameCleanups",filenameCleanups);
	ini.WriteInt(L"ExtractMetaData",m_iExtractMetaData);

	ini.WriteString(L"DefaultIRCServerNew", m_strIRCServer);
	ini.WriteString(L"IRCNick", m_strIRCNick);
	ini.WriteBool(L"IRCAddTimestamp", m_bIRCAddTimeStamp);
	ini.WriteString(L"IRCFilterName", m_strIRCChannelFilter);
	ini.WriteInt(L"IRCFilterUser", m_uIRCChannelUserFilter);
	ini.WriteBool(L"IRCUseFilter", m_bIRCUseChannelFilter);
	ini.WriteString(L"IRCPerformString", m_strIRCPerformString);
	ini.WriteBool(L"IRCUsePerform", m_bIRCUsePerform);
	ini.WriteBool(L"IRCListOnConnect", m_bIRCGetChannelsOnConnect);
	ini.WriteBool(L"IRCAcceptLink", m_bIRCAcceptLinks);
	ini.WriteBool(L"IRCAcceptLinkFriends", m_bIRCAcceptLinksFriendsOnly);
	ini.WriteBool(L"IRCSoundEvents", m_bIRCPlaySoundEvents);
	ini.WriteBool(L"IRCIgnoreMiscMessages", m_bIRCIgnoreMiscMessages);
	ini.WriteBool(L"IRCIgnoreJoinMessages", m_bIRCIgnoreJoinMessages);
	ini.WriteBool(L"IRCIgnorePartMessages", m_bIRCIgnorePartMessages);
	ini.WriteBool(L"IRCIgnoreQuitMessages", m_bIRCIgnoreQuitMessages);
	ini.WriteBool(L"IRCIgnoreEmuleAddFriendMsgs", m_bIRCIgnoreEmuleAddFriendMsgs);
	ini.WriteBool(L"IRCAllowEmuleAddFriend", m_bIRCAllowEmuleAddFriend);
	ini.WriteBool(L"IRCIgnoreEmuleSendLinkMsgs", m_bIRCIgnoreEmuleSendLinkMsgs);
	ini.WriteBool(L"IRCHelpChannel", m_bIRCJoinHelpChannel);
	ini.WriteBool(L"IRCEnableSmileys",m_bIRCEnableSmileys);
	ini.WriteBool(L"MessageEnableSmileys",m_bMessageEnableSmileys);

	ini.WriteBool(L"SmartIdCheck", m_bSmartServerIdCheck);
	ini.WriteBool(L"Verbose", m_bVerbose);
	ini.WriteBool(L"DebugSourceExchange", m_bDebugSourceExchange);	// do *not* use the according 'Get...' function here!
	ini.WriteBool(L"LogBannedClients", m_bLogBannedClients);			// do *not* use the according 'Get...' function here!
	ini.WriteBool(L"LogRatingDescReceived", m_bLogRatingDescReceived);// do *not* use the according 'Get...' function here!
	ini.WriteBool(L"LogSecureIdent", m_bLogSecureIdent);				// do *not* use the according 'Get...' function here!
	ini.WriteBool(L"LogFilteredIPs", m_bLogFilteredIPs);				// do *not* use the according 'Get...' function here!
	ini.WriteBool(L"LogFileSaving", m_bLogFileSaving);				// do *not* use the according 'Get...' function here!
    ini.WriteBool(L"LogA4AF", m_bLogA4AF);                           // do *not* use the according 'Get...' function here!
	ini.WriteBool(L"LogUlDlEvents", m_bLogUlDlEvents);
	// MORPH START - Added by Commander, WebCache 1.2f
	ini.WriteBool(L"LogICHEvents", m_bLogICHEvents);//JP log ICH events
	// MORPH END - Added by Commander, WebCache 1.2f
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
	ini.WriteBool(L"ManualHighPrio", m_bManualAddedServersHighPriority);
	ini.WriteBool(L"FullChunkTransfers", m_btransferfullchunks);
	ini.WriteBool(L"ShowOverhead", m_bshowoverhead);
	ini.WriteBool(L"VideoPreviewBackupped", moviePreviewBackup);
	ini.WriteInt(L"StartNextFile", m_istartnextfile);

	ini.DeleteKey(L"FileBufferSizePref"); // delete old 'file buff size' setting
	ini.WriteInt(L"FileBufferSize", m_iFileBufferSize);

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
	ini.WriteBool(L"MessageUseCaptchas", m_bUseChatCaptchas);
	ini.WriteBool(L"ShowInfoOnCatTabs",showCatTabInfos);
	ini.WriteBool(L"AutoFilenameCleanup",autofilenamecleanup);
	ini.WriteBool(L"ShowExtControls",m_bExtControls);
	// MORPH START show less controls
	ini.WriteBool(L"ShowLessControls",m_bShowLessControls);
    // MORPH END  show less controls
	ini.WriteBool(L"UseAutocompletion",m_bUseAutocompl);
	ini.WriteBool(L"NetworkKademlia",networkkademlia);
	ini.WriteBool(L"NetworkED2K",networked2k);
	ini.WriteBool(L"AutoClearCompleted",m_bRemoveFinishedDownloads);
	ini.WriteBool(L"TransflstRemainOrder",m_bTransflstRemain);
	ini.WriteBool(L"UseSimpleTimeRemainingcomputation",m_bUseOldTimeRemaining);
	ini.WriteBool(L"AllocateFullFile",m_bAllocFull);
	ini.WriteBool(L"ShowSharedFilesDetails", m_bShowSharedFilesDetails);
	ini.WriteBool(L"AutoShowLookups", m_bAutoShowLookups);

	ini.WriteInt(L"VersionCheckLastAutomatic", versioncheckLastAutomatic);
	//MORPH START - Added by SiRoB, New Version check
	ini.WriteInt(L"MVersionCheckLastAutomatic", mversioncheckLastAutomatic);
	//MORPH END   - Added by SiRoB, New Version check
	ini.WriteInt(L"FilterLevel",filterlevel);

	ini.WriteBool(L"SecureIdent", m_bUseSecureIdent);// change the name in future version to enable it by default
	ini.WriteBool(L"AdvancedSpamFilter",m_bAdvancedSpamfilter);
	ini.WriteBool(L"ShowDwlPercentage",m_bShowDwlPercentage);
	ini.WriteBool(L"RemoveFilesToBin",m_bRemove2bin);
	ini.WriteBool(L"ShowCopyEd2kLinkCmd",m_bShowCopyEd2kLinkCmd); // morph enable this saving. advanced offficial settings
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

    ini.WriteBool(L"A4AFSaveCpu", m_bA4AFSaveCpu); // ZZ:DownloadManager
    ini.WriteBool(L"HighresTimer", m_bHighresTimer);
	ini.WriteInt(L"WebMirrorAlertLevel", m_nWebMirrorAlertLevel);
	ini.WriteBool(L"RunAsUnprivilegedUser", m_bRunAsUser);
	ini.WriteBool(L"OpenPortsOnStartUp", m_bOpenPortsOnStartUp);
	ini.WriteInt(L"DebugLogLevel", m_byLogLevel);
	ini.WriteInt(L"WinXPSP2OrHigher", IsRunningXPSP2OrHigher());
	ini.WriteBool(L"RememberCancelledFiles", m_bRememberCancelledFiles);
	ini.WriteBool(L"RememberDownloadedFiles", m_bRememberDownloadedFiles);

	ini.WriteBool(L"NotifierSendMail", m_bNotifierSendMail);
	ini.WriteString(L"NotifierMailSender", m_strNotifierMailSender);
	ini.WriteString(L"NotifierMailServer", m_strNotifierMailServer);
	ini.WriteString(L"NotifierMailRecipient", m_strNotifierMailReceiver);

	ini.WriteBool(L"WinaTransToolbar", m_bWinaTransToolbar);
	ini.WriteBool(L"ShowDownloadToolbar", m_bShowDownloadToolbar);

	ini.WriteBool(L"CryptLayerRequested", m_bCryptLayerRequested);
	ini.WriteBool(L"CryptLayerRequired", m_bCryptLayerRequired);
	ini.WriteBool(L"CryptLayerSupported", m_bCryptLayerSupported);
	ini.WriteInt(L"KadUDPKey", m_dwKadUDPKey);

	ini.WriteBool(L"EnableSearchResultSpamFilter", m_bEnableSearchResultFilter);
	
	ini.WriteBool(L"CryptLayerRequiredStrictServer",IsServerCryptLayerRequiredStrict()); // MORPH lh require obfuscated server connection 

	// ==> Slot Limit - Stulle
	ini.WriteBool(L"SlotLimitThree", m_bSlotLimitThree);
	ini.WriteBool(L"SlotLimitNumB", m_bSlotLimitNum);
	ini.WriteInt(L"SlotLimitNum", m_iSlotLimitNum);
	// <== Slot Limit - Stulle

	// MORPH START leuk_he Advanced official preferences.
	ini.WriteBool(L"MiniMuleAutoClose",bMiniMuleAutoClose);
	ini.WriteInt(L"MiniMuleTransparency",iMiniMuleTransparency);
	ini.WriteBool(L"CreateCrashDump",bCreateCrashDump);
	ini.WriteBool(L"CheckComctl32",bCheckComctl32 );
	ini.WriteBool(L"CheckShell32",bCheckShell32);
	ini.WriteBool(L"IgnoreInstance",bIgnoreInstances);
	ini.WriteString(L"NotifierMailEncryptCertName",sNotifierMailEncryptCertName);
	ini.WriteString(L"MediaInfo_MediaInfoDllPath",sMediaInfo_MediaInfoDllPath);
	if (theApp.GetProfileInt(L"eMule", L"MediaInfo_RIFF_FIX", 1)==1){ // fix morph 9.3 bad default once.
		bMediaInfo_RIFF=true;
		bMediaInfo_ID3LIB=true;
		ini.WriteInt(L"MediaInfo_RIFF_FIX",0); //once
	}
	ini.WriteBool(L"MediaInfo_RIFF",bMediaInfo_RIFF);
	ini.WriteBool(L"MediaInfo_ID3LIB",bMediaInfo_ID3LIB);
#ifdef HAVE_QEDIT_H
	ini.WriteBool(_T("MediaInfo_MediaDet"),m_bMediaInfo_MediaDet);
#endif//HAVE_QEDIT_H
	ini.WriteBool(_T("MediaInfo_RM"),m_bMediaInfo_RM);
#ifdef HAVE_WMSDK_H
	ini.WriteBool(_T("MediaInfo_WM"),m_bMediaInfo_WM);
#endif//HAVE_WMSDK_H
	ini.WriteInt(L"MaxLogBuff",iMaxLogBuff/1024);
	ini.WriteInt(L"MaxChatHistoryLines",m_iMaxChatHistory);
	ini.WriteInt(L"PreviewSmallBlocks",m_iPreviewSmallBlocks);
	ini.WriteBool(L"RestoreLastMainWndDlg",m_bRestoreLastMainWndDlg);
	ini.WriteBool(L"RestoreLastLogPane",m_bRestoreLastLogPane);
	ini.WriteBool(L"PreviewCopiedArchives",m_bPreviewCopiedArchives);
	ini.WriteInt(L"StraightWindowStyles",m_iStraightWindowStyles);
	ini.WriteInt(L"LogFileFormat",m_iLogFileFormat);
	ini.WriteBool(L"RTLWindowsLayout",m_bRTLWindowsLayout);
	ini.WriteBool(L"PreviewOnIconDblClk",m_bPreviewOnIconDblClk);
	ini.WriteString(L"InternetSecurityZone",sInternetSecurityZone);
	ini.WriteInt(L"InspectAllFileTypes",m_iInspectAllFileTypes);
    ini.WriteInt(L"MaxMessageSessions",maxmsgsessions);
    ini.WriteBool(L"PreferRestrictedOverUser",m_bPreferRestrictedOverUser);
	ini.WriteBool(L"UserSortedServerList",m_bUseUserSortedServerList);
	ini.WriteInt(L"CryptTCPPaddingLength",m_byCryptTCPPaddingLength);
    ini.WriteBool(L"AdjustNTFSDaylightFileTime",m_bAdjustNTFSDaylightFileTime); 
    ini.WriteBool(L"DontCompressAvi",dontcompressavi);
    ini.WriteBool(L"ShowCopyEd2kLinkCmd",m_bShowCopyEd2kLinkCmd);
    ini.WriteBool(L"IconflashOnNewMessage",m_bIconflashOnNewMessage);
    ini.WriteBool(L"ReBarToolbar",m_bReBarToolbar);
	ini.WriteBool(L"ICH",IsICHEnabled());	// 10.5
	ini.WriteInt(L"FileBufferTimeLimit", m_uFileBufferTimeLimit/1000);
	ini.WriteBool(L"RearrangeKadSearchKeywords",m_bRearrangeKadSearchKeywords);
	ini.WriteBool(L"UpdateQueueListPref", m_bupdatequeuelist);
	ini.WriteBool(L"DontRecreateStatGraphsOnResize",dontRecreateGraphs);
	ini.WriteBool(L"BeepOnError",beepOnError);
	ini.WriteBool(L"MessageFromValidSourcesOnly",msgsecure);
	ini.WriteBool(L"ShowUpDownIconInTaskbar",m_bShowUpDownIconInTaskbar);
	ini.WriteBool(L"ForceSpeedsToKB",m_bForceSpeedsToKB);
	ini.WriteBool(L"ExtraPreviewWithMenu",m_bExtraPreviewWithMenu);
	ini.WriteBool(L"KeepUnavailableFixedSharedDirs",m_bKeepUnavailableFixedSharedDirs);
	ini.WriteColRef(L"LogErrorColor",m_crLogError);
	ini.WriteColRef(L"LogWarningColor",m_crLogWarning);
	ini.WriteColRef(L"LogSuccessColor",m_crLogSuccess);
	//MORPH START - Added by SiRoB, Upload Splitting Class
	ini.WriteColRef(L"LogUploadSplittingClassColor",m_crLogUSC);
	//MORPH END   - Added by SiRoB, Upload Splitting Class

	ini.WriteInt(L"MaxFileUploadSizeMB",m_iWebFileUploadSizeLimitMB, L"WebServer" );//section WEBSERVER start
	CString WriteAllowedIPs ;
	if (GetAllowedRemoteAccessIPs().GetCount() > 0)
		for (int i = 0; i <  GetAllowedRemoteAccessIPs().GetCount(); i++)
           WriteAllowedIPs = WriteAllowedIPs  + L";" + ipstr(GetAllowedRemoteAccessIPs()[i]);
    ini.WriteString(L"AllowedIPs",WriteAllowedIPs);  // End Seciotn Webserver
    ini.WriteBool(L"ShowVerticalHourMarkers",m_bShowVerticalHourMarkers,L"Statistics");
	ini.WriteBool(L"EnabledDeprecated", m_bPeerCacheEnabled, L"PeerCache");
	// MORPH END  leuk_he Advanced official preferences. 

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
	ini.WriteString(L"statsExpandedTreeItems", m_strStatsExpandedTreeItems);
	CString buffer2;
	for (int i=0;i<GetNumStatsColors();i++) { //MORPH - Changed by SiRoB, Powershare display
		buffer.Format(L"0x%06x",GetStatsColor(i));
		buffer2.Format(L"StatColor%i",i);
		ini.WriteString(buffer2,buffer,L"Statistics" );
	}
	ini.WriteBool(L"HasCustomTaskIconColor", bHasCustomTaskIconColor, L"Statistics");


	///////////////////////////////////////////////////////////////////////////
	// Section: "WebServer"
	//
	ini.WriteString(L"Password", GetWSPass(), L"WebServer");
	ini.WriteString(L"PasswordLow", GetWSLowPass());
	ini.WriteInt(L"Port", m_nWebPort);
#ifdef USE_OFFICIAL_UPNP
	ini.WriteBool(L"WebUseUPnP", m_bWebUseUPnP);
#endif
	ini.WriteBool(L"Enabled", m_bWebEnabled);
	ini.WriteBool(L"UseGzip", m_bWebUseGzip);
	ini.WriteInt(L"PageRefreshTime", m_nWebPageRefresh);
	ini.WriteBool(L"UseLowRightsUser", m_bWebLowEnabled);
	ini.WriteBool(L"AllowAdminHiLevelFunc",m_bAllowAdminHiLevFunc);
	ini.WriteInt(L"WebTimeoutMins", m_iWebTimeoutMins);
	//>>> [ionix] - iONiX::Advanced WebInterface Account Management
	ini.WriteBool(L"UseIonixWebsrv", m_bIonixWebsrv);
	//<<< [ionix] - iONiX::Advanced WebInterface Account Management



	///////////////////////////////////////////////////////////////////////////
	// Section: "MobileMule"
	//
	ini.WriteString(L"Password", GetMMPass(), L"MobileMule");
	ini.WriteBool(L"Enabled", m_bMMEnabled);
	ini.WriteInt(L"Port", m_nMMPort);


	///////////////////////////////////////////////////////////////////////////
	// Section: "PeerCache"
	//
	ini.WriteInt(L"LastSearch", m_uPeerCacheLastSearch, L"PeerCache");
	ini.WriteBool(L"Found", m_bPeerCacheWasFound);
	ini.WriteInt(L"PCPort", m_nPeerCachePort);

#ifdef USE_OFFICIAL_UPNP
	///////////////////////////////////////////////////////////////////////////
	// Section: "UPnP"
	//
	ini.WriteBool(L"EnableUPnP", m_bEnableUPnP, L"UPnP");
	ini.WriteBool(L"SkipWANIPSetup", m_bSkipWANIPSetup);
	ini.WriteBool(L"SkipWANPPPSetup", m_bSkipWANPPPSetup);
	ini.WriteBool(L"CloseUPnPOnExit", m_bCloseUPnPOnExit);
	ini.WriteInt(L"LastWorkingImplementation", m_nLastWorkingImpl);
#endif
	
	//MORPH START - Added by SiRoB, [MoNKi: -UPnPNAT Support-]
	ini.WriteBool(L"UPnPNAT", m_bUPnPNat, L"eMule");
	ini.WriteBool(L"UPnPNAT_Web", m_bUPnPNatWeb);
	ini.WriteBool(L"UPnPVerbose", m_bUPnPVerboseLog);
	ini.WriteInt(L"UPnPPort", m_iUPnPPort);
	ini.WriteBool(L"UPnPClearOnClose", m_bUPnPClearOnClose);
	ini.WriteBool(L"UPnPLimitToFirstConnection", m_bUPnPLimitToFirstConnection);
	ini.WriteInt(L"UPnPDetect", m_iDetectuPnP); // 
	ini.WriteBool(L"UPnPForceUpdate", m_bUPnPForceUpdate);
		//MORPH END   - Added by SiRoB, [MoNKi: -UPnPNAT Support-]
  
	//MORPH START - Added by SiRoB, [MoNKi: -Random Ports-]
	ini.WriteBool(L"RandomPorts", m_bRndPorts, L"eMule");
	ini.WriteInt(L"MinRandomPort", m_iMinRndPort, L"eMule");
	ini.WriteInt(L"MaxRandomPort", m_iMaxRndPort, L"eMule");
	ini.WriteBool(L"RandomPortsReset", m_bRndPortsResetOnRestart, L"eMule");
	ini.WriteInt(L"RandomPortsSafeResetOnRestartTime", m_iRndPortsSafeResetOnRestartTime, L"eMule");

	ini.WriteInt(L"OldTCPRandomPort", m_iCurrentTCPRndPort, L"eMule");
	ini.WriteInt(L"OldUDPRandomPort", m_iCurrentUDPRndPort, L"eMule");
	ini.WriteUInt64(L"RandomPortsLastRun", CTime::GetCurrentTime().GetTime() , L"eMule");
	//MORPH END   - Added by SiRoB, [MoNKi: -Random Ports-]

	//MORPH START - Added by SiRoB, [MoNKi: -Improved ICS-Firewall support-]
	ini.WriteBool(L"ICFSupportFirstTime", m_bICFSupportFirstTime, L"eMule");
	ini.WriteBool(L"ICFSupport", m_bICFSupport, L"eMule");
	ini.WriteBool(L"ICFSupportServerUDP", m_bICFSupportServerUDP , L"eMule");
	//MORPH END   - Added by SiRoB, [MoNKi: -Improved ICS-Firewall support-]

    //MORPH START - Added by SiRoB / Commander, Wapserver [emulEspaña]
	ini.WriteBool(L"WapEnabled", m_bWapEnabled, L"WapServer");
	ini.WriteString(L"WapTemplateFile",m_sWapTemplateFile, L"WapServer");
	ini.WriteInt(L"WapPort", m_nWapPort, L"WapServer");
	ini.WriteInt(L"WapGraphWidth", m_iWapGraphWidth, L"WapServer");
	ini.WriteInt(L"WapGraphHeight", m_iWapGraphHeight, L"WapServer");
	ini.WriteBool(L"WapFilledGraphs", m_bWapFilledGraphs, L"WapServer");
	ini.WriteInt(L"WapMaxItemsInPage", m_iWapMaxItemsInPages, L"WapServer");
	ini.WriteBool(L"WapSendImages", m_bWapSendImages, L"WapServer");
	ini.WriteBool(L"WapSendGraphs", m_bWapSendGraphs, L"WapServer");
	ini.WriteBool(L"WapSendProgressBars", m_bWapSendProgressBars, L"WapServer");
	ini.WriteBool(L"WapSendBWImages", m_bWapAllwaysSendBWImages, L"WapServer");
	ini.WriteInt(L"WapLogsSize", m_iWapLogsSize, L"WapServer");
	ini.WriteString(L"WapPassword", m_sWapPassword, L"WapServer");
	ini.WriteString(L"WapPasswordLow", m_sWapLowPassword, L"WapServer");
	ini.WriteBool(L"WapLowEnable", m_bWapLowEnabled, L"WapServer");
	//MORPH END - Added by SiRoB / Commander, Wapserver [emulEspaña]        

	//MORPH START - Added by Commander, ClientQueueProgressBar  
	ini.WriteBool(L"ClientQueueProgressBar",m_bClientQueueProgressBar, L"eMule");
	//MORPH END - Added by Commander, ClientQueueProgressBar

	//MORPH START - Added by Commander, FolderIcons  
	ini.WriteBool(L"ShowFolderIcons",m_bShowFolderIcons, L"eMule");
	//MORPH END - Added by Commander, FolderIcons

	ini.WriteBool(L"InfiniteQueue",infiniteQueue,L"eMule");	//Morph - added by AndCycle, SLUGFILLER: infiniteQueue

	ini.WriteBool(L"AutoDynUpSwitching",isautodynupswitching,L"eMule");//MORPH - Added by Yun.SF3, Auto DynUp changing
	ini.WriteInt(L"PowershareMode",m_iPowershareMode,L"eMule"); //MORPH - Added by SiRoB, Avoid misusing of powersharing
	ini.WriteBool(L"PowershareInternalPrio",m_bPowershareInternalPrio,L"eMule");//Morph - added by AndCyle, selective PS internal Prio

	ini.WriteBool(L"EnableHighProcess", enableHighProcess,L"eMule"); //MORPH - Added by IceCream, high process priority

	ini.WriteBool(L"EnableDownloadInRed", enableDownloadInRed,L"eMule"); //MORPH - Added by IceCream, show download in red
	ini.WriteBool(L"EnableDownloadInBold", m_bShowActiveDownloadsBold,L"eMule"); //MORPH - Added by SiRoB, show download in Bold
        //MORPH START - Added by schnulli900, filter clients with failed downloads [Xman]
	ini.WriteBool(L"FilterClientFailedDown", m_bFilterClientFailedDown,L"eMule"); 
        //MORPH END   - Added by schnulli900, filter clients with failed downloads [Xman]
	ini.WriteBool(L"EnableAntiLeecher", enableAntiLeecher,L"eMule"); //MORPH - Added by IceCream, enable AntiLeecher
	ini.WriteBool(L"EnableAntiCreditHack", enableAntiCreditHack,L"eMule"); //MORPH - Added by IceCream, enable AntiCreditHack
	ini.WriteInt(L"CreditSystemMode", creditSystemMode,L"eMule");// EastShare - Added by linekin, ES CreditSystem
	ini.WriteBool(L"EqualChanceForEachFile", m_bEnableEqualChanceForEachFile, L"eMule");	//Morph - added by AndCycle, Equal Chance For Each File
	ini.WriteBool(L"FollowTheMajority", m_bFollowTheMajority, L"eMule"); // EastShare       - FollowTheMajority by AndCycle
	ini.WriteInt(L"FairPlay", m_iFairPlay, L"eMule"); //EastShare	- FairPlay by AndCycle

	//MORPH START - Added by SiRoB, Datarate Average Time Management
	ini.WriteInt(L"DownloadDataRateAverageTime",max(1,m_iDownloadDataRateAverageTime/1000),L"eMule");
	ini.WriteInt(L"UPloadDataRateAverageTime",max(1,m_iUploadDataRateAverageTime/1000),L"eMule");
	//MORPH END   - Added by SiRoB, Datarate Average Time Management

	//MORPH START - Added by SiRoB, Upload Splitting Class
	ini.WriteInt(L"GlobalDataRateFriend",globaldataratefriend,L"eMule");
	ini.WriteInt(L"MaxGlobalDataRateFriend",maxglobaldataratefriend,L"eMule");
	ini.WriteInt(L"GlobalDataRatePowerShare",globaldataratepowershare,L"eMule");
	ini.WriteInt(L"MaxGlobalDataRatePowerShare",maxglobaldataratepowershare,L"eMule");
	ini.WriteInt(L"MaxClientDataRateFriend",maxclientdataratefriend,L"eMule");
	ini.WriteInt(L"MaxClientDataRatePowerShare",maxclientdataratepowershare,L"eMule");
	ini.WriteInt(L"MaxClientDataRate",maxclientdatarate,L"eMule");
	//MORPH END   - Added by SiRoB, Upload Splitting Class

	//MORPH START - Added by SiRoB, SLUGFILLER: lowIdRetry
	ini.WriteInt(L"ReconnectOnLowIdRetries",LowIdRetries,L"eMule");	// SLUGFILLER: lowIdRetry
	//MORPH END   - Added by SiRoB, SLUGFILLER: lowIdRetry
	//MORPH	Start	- Added by AndCycle, SLUGFILLER: Spreadbars - per file
	ini.WriteInt(L"SpreadbarSetStatus", m_iSpreadbarSetStatus, L"eMule");
	//MORPH	End	- Added by AndCycle, SLUGFILLER: Spreadbars - per file
	//MORPH START - Added by SiRoB, SLUGFILLER: hideOS
	ini.WriteInt(L"HideOvershares",hideOS,L"eMule");
	ini.WriteBool(L"SelectiveShare",selectiveShare,L"eMule");
	//MORPH END   - Added by SiRoB, SLUGFILLER: hideOS
	//MORPH START - Added by SiRoB, SHARE_ONLY_THE_NEED
	ini.WriteInt(L"ShareOnlyTheNeed",ShareOnlyTheNeed,L"eMule");
	//MORPH END   - Added by SiRoB, SHARE_ONLY_THE_NEED
	//MORPH START - Added by SiRoB, POWERSHARE Limit
	ini.WriteInt(L"PowerShareLimit",PowerShareLimit,L"eMule");
	//MORPH END   - Added by SiRoB, POWERSHARE Limit
	//MORPH START - Added by SiRoB, Show Permissions
	ini.WriteInt(L"ShowSharePermissions",permissions,L"eMule");
	//MORPH END   - Added by SiRoB, Show Permissions

    //MORPH START added by Yun.SF3: Ipfilter.dat update
	ini.WriteBinary(L"IPfilterVersion", (LPBYTE)&m_IPfilterVersion, sizeof(m_IPfilterVersion),L"eMule"); 
	ini.WriteBool(L"AutoUPdateIPFilter",AutoUpdateIPFilter,L"eMule");
	ini.WriteInt(L"IPFilterVersionNum", m_uIPFilterVersionNum,L"eMule");
    //MORPH END added by Yun.SF3: Ipfilter.dat update

	//Commander - Added: IP2Country Auto-updating - Start
	ini.WriteBinary(L"IP2CountryVersion", (LPBYTE)&m_IP2CountryVersion, sizeof(m_IP2CountryVersion),L"eMule"); 
	ini.WriteBool(L"AutoUPdateIP2Country",AutoUpdateIP2Country,L"eMule");
	//Commander - Added: IP2Country Auto-updating - End

	//MORPH START - Added by milobac, FakeCheck, FakeReport, Auto-updating
	ini.WriteBinary(L"FakesDatVersion", (LPBYTE)&m_FakesDatVersion, sizeof(m_FakesDatVersion),L"eMule"); 
	ini.WriteBool(L"UpdateFakeStartup",UpdateFakeStartup,L"eMule");
	//MORPH END - Added by milobac, FakeCheck, FakeReport, Auto-updating

	ini.WriteString(L"UpdateURLFakeList",UpdateURLFakeList,L"eMule");		//MORPH START - Added by milobac and Yun.SF3, FakeCheck, FakeReport, Auto-updating
	ini.WriteString(L"UpdateURLIPFilter",UpdateURLIPFilter,L"eMule");//MORPH START added by Yun.SF3: Ipfilter.dat update
    ini.WriteString(L"UpdateURLIP2Country",UpdateURLIP2Country,L"eMule");//Commander - Added: IP2Country auto-updating

	//EastShare Start - PreferShareAll by AndCycle
	ini.WriteBool(L"ShareAll",shareall,L"eMule");	// SLUGFILLER: preferShareAll
	//EastShare END - PreferShareAll by AndCycle
	// EastShare START - Added by TAHO, .met file control
	ini.WriteInt(L"KnownMetDays", m_iKnownMetDays,L"eMule");
	ini.WriteBool(L"PartiallyPurgeOldKnownFiles",m_bPartiallyPurgeOldKnownFiles,L"eMule");
	ini.WriteBool(L"CompletlyPurgeOldKnownFiles",m_bCompletlyPurgeOldKnownFiles,L"eMule");
	ini.WriteBool(L"RemoveAichImmediatly",m_bRemoveAichImmediatly,L"eMule");
	// EastShare END - Added by TAHO, .met file control
	//EastShare - Added by Pretender, Option for ChunkDots
	ini.WriteInt(L"EnableChunkDots", m_bEnableChunkDots,L"eMule");
	//EastShare - Added by Pretender, Option for ChunkDots

	//EastShare - added by AndCycle, IP to Country
	ini.WriteInt(L"IP2Country", m_iIP2CountryNameMode,L"eMule"); 
	ini.WriteBool(L"IP2CountryShowFlag", m_bIP2CountryShowFlag,L"eMule");
	//EastShare - added by AndCycle, IP to Country

	// khaos::categorymod+ Save Preferences
	ini.WriteBool(L"ValidSrcsOnly", m_bValidSrcsOnly,L"eMule");
	ini.WriteBool(L"ShowCatName", m_bShowCatNames,L"eMule");
	ini.WriteBool(L"ActiveCatDefault", m_bActiveCatDefault,L"eMule");
	ini.WriteBool(L"SelCatOnAdd", m_bSelCatOnAdd,L"eMule");
	ini.WriteBool(L"AutoSetResumeOrder", m_bAutoSetResumeOrder,L"eMule");
	ini.WriteBool(L"SmallFileDLPush", m_bSmallFileDLPush,L"eMule");
	ini.WriteInt(L"StartDLInEmptyCats", m_iStartDLInEmptyCats,L"eMule");
	ini.WriteBool(L"UseAutoCat", m_bUseAutoCat,L"eMule");
	ini.WriteBool(L"AddRemovedInc", m_bAddRemovedInc,L"eMule");
	// khaos::categorymod-
	// MORPH START leuk_he disable catcolor
	ini.WriteBool(L"DisableCatColors", m_bDisableCatColors,L"eMule");
	// MORPH END   leuk_he disable catcolor
	// khaos::kmod+
	ini.WriteBool(L"SmartA4AFSwapping", m_bSmartA4AFSwapping,L"eMule");
	ini.WriteInt(L"AdvancedA4AFMode", m_iAdvancedA4AFMode,L"eMule");
	ini.WriteBool(L"ShowA4AFDebugOutput", m_bShowA4AFDebugOutput,L"eMule");
	ini.WriteBool(L"RespectMaxSources", m_bRespectMaxSources,L"eMule");
	ini.WriteBool(L"UseSaveLoadSources", m_bUseSaveLoadSources,L"eMule");
	// khaos::categorymod-
	// khaos::accuratetimerem+
	ini.WriteInt(L"TimeRemainingMode", m_iTimeRemainingMode,L"eMule");
	// khaos::accuratetimerem-
	//MORPH START - Added by SiRoB, ICS Optional
	ini.WriteBool(L"UseIntelligentChunkSelection", m_bUseIntelligentChunkSelection,L"eMule");
	//MORPH END   - Added by SiRoB, ICS Optional
	//MORPH START - Added by SiRoB, Smart Upload Control v2 (SUC) [lovelace]
	ini.WriteBool(L"SUCEnabled",m_bSUCEnabled,L"eMule");
	ini.WriteInt(L"SUCLog",m_bSUCLog,L"eMule");
	ini.WriteInt(L"SUCHigh",m_iSUCHigh,L"eMule");
	ini.WriteInt(L"SUCLow",m_iSUCLow,L"eMule");
	ini.WriteInt(L"SUCDrift",m_iSUCDrift,L"eMule");
	ini.WriteInt(L"SUCPitch",m_iSUCPitch,L"eMule");
	//MORPH END - Added by SiRoB, Smart Upload Control v2 (SUC) [lovelace]
	ini.WriteInt(L"MaxConnectionsSwitchBorder",maxconnectionsswitchborder,L"eMule");//MORPH - Added by Yun.SF3, Auto DynUp changing

	ini.WriteBool(L"IsPayBackFirst",m_bPayBackFirst,L"eMule");//EastShare - added by AndCycle, Pay Back First
	ini.WriteInt(L"PayBackFirstLimit",m_iPayBackFirstLimit,L"eMule");//MORPH - Added by SiRoB, Pay Back First Tweak
	ini.WriteBool(L"OnlyDownloadCompleteFiles", m_bOnlyDownloadCompleteFiles,L"eMule");//EastShare - Added by AndCycle, Only download complete files v2.1 (shadow)
	ini.WriteBool(L"SaveUploadQueueWaitTime", m_bSaveUploadQueueWaitTime,L"eMule");//Morph - added by AndCycle, Save Upload Queue Wait Time (MSUQWT)
	ini.WriteBool(L"DateFileNameLog", m_bDateFileNameLog,L"eMule");//Morph - added by AndCycle, Date File Name Log
	ini.WriteBool(L"DontRemoveSpareTrickleSlot", m_bDontRemoveSpareTrickleSlot,L"eMule");//Morph - added by AndCycle, Dont Remove Spare Trickle Slot
	ini.WriteBool(L"UseDownloadOverhead",m_bUseDownloadOverhead,L"eMule");//Morph - added by AndCycle, Dont Remove Spare Trickle Slot
	ini.WriteBool(L"DisplayFunnyNick", m_bFunnyNick,L"eMule");//MORPH - Added by SiRoB, Optionnal funnynick display
	//EastShare Start - Added by Pretender, TBH-AutoBackup
	ini.WriteBool(L"AutoBackup",autobackup,L"eMule");
	ini.WriteBool(L"AutoBackup2",autobackup2,L"eMule");
	//EastShare End - Added by Pretender, TBH-AutoBackup

	// Mighty Knife: Community visualization, Report hashing files, Log friendlist activities
	ini.WriteString(L"CommunityName", m_sCommunityName,L"eMule");
	ini.WriteBool (L"ReportHashingFiles",m_bReportHashingFiles,L"eMule");
	ini.WriteBool (L"LogFriendlistActivities",m_bLogFriendlistActivities,L"eMule");
	// [end] Mighty Knife

	// Mighty Knife: CRC32-Tag
	ini.WriteBool (L"DontAddCRC32ToFilename",m_bDontAddCRCToFilename,L"eMule");
	ini.WriteBool (L"ForceCRC32Uppercase",m_bCRC32ForceUppercase,L"eMule");
	ini.WriteBool (L"ForceCRC32Adding",m_bCRC32ForceAdding,L"eMule");
	CString temp;
	// Encapsule these strings by "" because space characters are allowed at the
	// beginning/end of the prefix/suffix !
	temp.Format (L"\"%s\"",m_sCRC32Prefix);
	ini.WriteString(L"LastCRC32Prefix",temp,L"eMule");
	temp.Format (L"\"%s\"",m_sCRC32Suffix);
	ini.WriteString(L"LastCRC32Suffix",temp,L"eMule");
	// [end] Mighty Knife

	// Mighty Knife: Simple cleanup options
	ini.WriteInt (L"SimpleCleanupOptions",m_SimpleCleanupOptions);
	// Enclose the strings with '"' before writing them to the file.
	// These will be filtered if the string is read again
	ini.WriteString (L"SimpleCleanupSearch",CString ('\"')+m_SimpleCleanupSearch+'\"');
	ini.WriteString (L"SimpleCleanupReplace",CString ('\"')+m_SimpleCleanupReplace+'\"');
	ini.WriteString (L"SimpleCleanupSearchChars",CString ('\"')+m_SimpleCleanupSearchChars+'\"');
	ini.WriteString (L"SimpleCleanupReplaceChars",CString ('\"')+m_SimpleCleanupReplaceChars+'\"');
	// [end] Mighty Knife

	// Mighty Knife: Static server handling
	ini.WriteBool (L"DontRemoveStaticServers",m_bDontRemoveStaticServers,L"eMule");
	// [end] Mighty Knife

	//MORPH START - Added by SiRoB,  ZZ dynamic upload (USS)
	ini.WriteBool(L"USSLog", m_bDynUpLog,L"eMule");
	//MORPH END    - Added by SiRoB,  ZZ dynamic upload (USS)
	ini.WriteBool(L"USSUDP_FORCE", m_bUSSUDP,L"eMule"); //MORPH - Added by SiRoB, USS UDP preferency
	ini.WriteInt(L"USSPingDataSize", (int)m_sPingDataSize); // MORPH leuk_he ICMP ping datasize <> 0 setting
    ini.WriteBool(L"ShowClientPercentage",m_bShowClientPercentage);  //Commander - Added: Client Percentage

    //Commander - Added: Invisible Mode [TPT] - Start
    ini.WriteBool(L"InvisibleMode", m_bInvisibleMode);
	ini.WriteInt(L"InvisibleModeHKKey", (int)m_cInvisibleModeHotKey);
	ini.WriteInt(L"InvisibleModeHKKeyModifier", m_iInvisibleModeHotKeyModifier);
    //Commander - Added: Invisible Mode [TPT] - End        

	//MORPH START - Added by Stulle, Global Source Limit
	ini.WriteBool(L"GlobalHL", m_bGlobalHL);
	ini.WriteInt(L"GlobalHLvalue", m_uGlobalHL);
	//MORPH END   - Added by Stulle, Global Source Limit

	//MORPH START - Added, Downloaded History [Monki/Xman]
	ini.WriteBool(L"ShowSharedInHistory", m_bHistoryShowShared);
	//MORPH END   - Added, Downloaded History [Monki/Xman]
	//MORPH START leuk_he:run as ntservice v1..
	ini.WriteInt(L"ServiceStartupMode",m_iServiceStartupMode);
	if(theApp.GetProfileInt(_T("eMule"), _T("ServiceOptLvlNew"),-1) == 4 && m_iServiceOptLvl == SVC_BASIC_OPT) // Import from future StulleMule versions
		ini.WriteInt(L"ServiceOptLvlNew",4);
	else
		ini.WriteInt(L"ServiceOptLvlNew",m_iServiceOptLvl);
	//MORPH END leuk_he:run as ntservice v1..
	ini.WriteBool(L"StaticIcon",m_bStaticIcon); //MORPH - Added, Static Tray Icon
	ini.WriteBool(L"FakeAlyzerIndications",m_bFakeAlyzerIndications); //MORPH - Added by Stulle, Fakealyzer [netfinity]
	ini.WriteString(L"BrokenURLs", m_strBrokenURLs); //MORPH - Added by WiZaRd, Fix broken HTTP downloads
	// ==> [MoNKi: -USS initial TTL-] - Stulle
	ini.WriteInt(L"USSInitialTTL", m_iUSSinitialTTL, L"StulleMule");
	// <== [MoNKi: -USS initial TTL-] - Stulle
	//MORPH START - Added by Stulle, Adjustable NT Service Strings
	ini.WriteString(L"ServiceName", m_strServiceName);
	ini.WriteString(L"ServiceDispName", m_strServiceDispName);
	ini.WriteString(L"ServiceDescr", m_strServiceDescr);
	//MORPH END   - Added by Stulle, Adjustable NT Service Strings
}

void CPreferences::ResetStatsColor(int index)
{
	switch(index)
	{
		case 0 : m_adwStatsColors[0]=RGB(0,0,0);break;  //MORPH - HotFix by SiRoB & IceCream, Default Black color for BackGround
		case 1 : m_adwStatsColors[1]=RGB(192,192,255);break;
		case 2 : m_adwStatsColors[2]=RGB(128, 255, 128);break;
		case 3 : m_adwStatsColors[3]=RGB(0, 255, 255);break;
		case 4 : m_adwStatsColors[4]=RGB(255, 255, 255);break;
		case 5 : m_adwStatsColors[5]=RGB(255, 0, 0);break;
		case 6 : m_adwStatsColors[6]=RGB(0, 255, 255);break;
		case 7 : m_adwStatsColors[7]=RGB(255, 255, 255);break;
		case 8 : m_adwStatsColors[8]=RGB(150, 150, 255);break;
		case 9 : m_adwStatsColors[9]=RGB(255, 255, 128);break; //MORPH - Added by Yun.SF3, ZZ Upload System
		case 10 : m_adwStatsColors[10]=RGB(0, 255, 0);break;
		case 11 : m_adwStatsColors[11]=RGB(0, 0, 0); bHasCustomTaskIconColor = false; break; //MORPH - HotFix by SiRoB & IceCream, Default Black color for SystrayBar
		case 12 : m_adwStatsColors[12]=RGB(192,   0, 192);break; //MORPH - Added by Yun.SF3, ZZ Upload System
		case 13 : m_adwStatsColors[13]=RGB(128, 128, 255);break; //MORPH - Added by Yun.SF3, ZZ Upload System
		case 14 : m_adwStatsColors[14]=RGB(192, 192, 0);break;
		case 15 : m_adwStatsColors[15]=RGB(255, 0, 255);break; //MORPH - Added by SiRoB, Powershare display
	}
}

void CPreferences::GetAllStatsColors(int iCount, LPDWORD pdwColors)
{
	memset(pdwColors, 0, sizeof(*pdwColors) * iCount);
	memcpy(pdwColors, m_adwStatsColors, sizeof(*pdwColors) * min(_countof(m_adwStatsColors), iCount));
}

bool CPreferences::SetAllStatsColors(int iCount, const DWORD* pdwColors)
{
	bool bModified = false;
	int iMin = min(_countof(m_adwStatsColors), iCount);
	for (int i = 0; i < iMin; i++)
	{
		if (m_adwStatsColors[i] != pdwColors[i])
		{
			m_adwStatsColors[i] = pdwColors[i];
			bModified = true;
			if (i == 11)
				bHasCustomTaskIconColor = true;
		}
	}
	return bModified;
}

void CPreferences::IniCopy(CString si, CString di)
{
	CIni ini(GetConfigFile(), L"eMule");
	CString s = ini.GetString(si);
	// Do NOT write empty settings, this will mess up reading of default settings in case
	// there were no settings (fresh emule install) at all available!
	if (!s.IsEmpty())
	{
		ini.SetSection(L"ListControlSetup");
		ini.WriteString(di,s);
	}
}

void CPreferences::LoadPreferences()
{
	CIni ini(GetConfigFile(), L"eMule");
	ini.SetSection(L"eMule");

	CString strCurrVersion, strPrefsVersion;

	//MORPH START - Added by SiRoB, [itsonlyme: -modname-]
	/*
	strCurrVersion = theApp.m_strCurVersionLong;
	*/
	strCurrVersion = theApp.m_strCurVersionLong + _T(" [") + theApp.m_strModLongVersion + _T("]");
	//MORPH END   - Added by SiRoB, [itsonlyme: -modname-]
	
	strPrefsVersion = ini.GetString(L"AppVersion");

	m_bFirstStart = false;

	if (strPrefsVersion.IsEmpty()){
		m_bFirstStart = true;
	}

#ifdef _DEBUG
	m_iDbgHeap = ini.GetInt(L"DebugHeap", 1);
#else
	m_iDbgHeap = 0;
#endif

	m_nWebMirrorAlertLevel = ini.GetInt(L"WebMirrorAlertLevel",0);
	updatenotify=ini.GetBool(L"UpdateNotifyTestClient",true);

	SetUserNick(ini.GetStringUTF8(L"Nick", DEFAULT_NICK));
	if (strNick.IsEmpty() || IsDefaultNick(strNick))
		SetUserNick(DEFAULT_NICK);

	m_strIncomingDir = ini.GetString(L"IncomingDir", _T(""));
	if (m_strIncomingDir.IsEmpty()) // We want GetDefaultDirectory to also create the folder, so we have to know if we use the default or not
		m_strIncomingDir = GetDefaultDirectory(EMULE_INCOMINGDIR, true);
	MakeFoldername(m_strIncomingDir);

	// load tempdir(s) setting
	CString tempdirs;
	tempdirs = ini.GetString(L"TempDir", _T(""));
	if (tempdirs.IsEmpty()) // We want GetDefaultDirectory to also create the folder, so we have to know if we use the default or not
		tempdirs = GetDefaultDirectory(EMULE_TEMPDIR, true);
	tempdirs += L"|" + ini.GetString(L"TempDirs");

	int curPos=0;
	bool doubled;
	CString atmp=tempdirs.Tokenize(L"|", curPos);
	while (!atmp.IsEmpty())
	{
		atmp.Trim();
		if (!atmp.IsEmpty()) {
			MakeFoldername(atmp);
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

	maxGraphDownloadRate=ini.GetInt(L"DownloadCapacity",96);
	if (maxGraphDownloadRate==0)
		maxGraphDownloadRate=96;
	
	maxGraphUploadRate = ini.GetInt(L"UploadCapacityNew",-1);
	if (maxGraphUploadRate == 0)
		maxGraphUploadRate = UNLIMITED;
	else if (maxGraphUploadRate == -1){
		// converting value from prior versions
		int nOldUploadCapacity = ini.GetInt(L"UploadCapacity", 16);
		if (nOldUploadCapacity == 16 && ini.GetInt(L"MaxUpload",12) == 12){
			// either this is a complete new install, or the prior version used the default value
			// in both cases, set the new default values to unlimited
			maxGraphUploadRate = UNLIMITED;
			ini.WriteInt(L"MaxUpload",UNLIMITED, L"eMule");
		}
		else
			maxGraphUploadRate = nOldUploadCapacity; // use old custoum value
	}

	minupload=ini.GetInt(L"MinUpload", 5);    // also used for unlimited... 

	//MORPH START - Added by SiRoB, (SUC) & (USS)
	minupload = min(max(minupload,1),maxGraphUploadRate); //MORPH uint16 is not enough
	//MORPH END   - Added by SiRoB, (SUC) & (USS)
	maxupload=ini.GetInt(L"MaxUpload",UNLIMITED); //MORPH uint16 is not enough
	if (maxupload > maxGraphUploadRate && maxupload != UNLIMITED)
		maxupload = (maxGraphUploadRate  / 5 * 4); //MORPH uint16 is not enough
	
	maxdownload=ini.GetInt(L"MaxDownload", UNLIMITED); //MORPH uint16 is not enough
	if (maxdownload > maxGraphDownloadRate && maxdownload != UNLIMITED)
		maxdownload = (maxGraphDownloadRate /5 * 4); //MORPH uint16 is not enoug
	maxconnections=ini.GetInt(L"MaxConnections",GetRecommendedMaxConnections());
	maxhalfconnections=ini.GetInt(L"MaxHalfConnections",9);
	m_bConditionalTCPAccept = ini.GetBool(L"ConditionalTCPAccept", false);

	// reset max halfopen to a default if OS changed to SP2 (or higher) or away
	int dwSP2OrHigher = ini.GetInt(L"WinXPSP2OrHigher", -1);
	int dwCurSP2OrHigher = IsRunningXPSP2OrHigher();
	if (dwSP2OrHigher != dwCurSP2OrHigher){
		if (dwCurSP2OrHigher == 0)
			maxhalfconnections = 50;
		else if (dwCurSP2OrHigher == 1)
			maxhalfconnections = 9;
	}
  // MORPH START leuk_he upnp bindaddr
	// abuse m_strBindAddrW will be overwriten in a sec...
	m_strBindAddrW = ini.GetString(L"upnpBindAddr");
	m_strBindAddrW.Trim();

  	SetUpnpBindAddr(ntohl(inet_addr((LPCSTR)(CStringA)m_strBindAddrW  )));
  // MORPH END leuk_he upnp bindaddr

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
	m_wLanguageID=ini.GetWORD(L"Language",0);
	m_iSeeShares=(EViewSharedFilesAccess)ini.GetInt(L"SeeShare",vsfaNobody);
	m_iToolDelayTime=ini.GetInt(L"ToolTipDelay",1);
	trafficOMeterInterval=ini.GetInt(L"StatGraphsInterval",3);
	statsInterval=ini.GetInt(L"statsInterval",5);
	m_bFillGraphs=ini.GetBool(L"StatsFillGraphs");
	dontcompressavi=ini.GetBool(L"DontCompressAvi",false);
	// MORPH setable compresslevel [leuk_he]
	m_iCompressLevel=(short) ini.GetInt(L"CompressLevel",9,L"eMule" );
	if ((m_iCompressLevel > 9 )||(m_iCompressLevel < 1 )) m_iCompressLevel=9 ; // 1 = worst, but saves cpu, 9 = best, emule default
	// MORPH setable compresslevel [leuk_he]
	
	m_uDeadServerRetries=ini.GetInt(L"DeadServerRetry",3);	// morph 1 --> 3 for no fallback from obfuscated -> normal. 
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
	splitterbarPositionFriend=ini.GetInt(L"SplitterbarPositionFriend",170);
	splitterbarPositionShared=ini.GetInt(L"SplitterbarPositionShared",179);
	splitterbarPositionIRC=ini.GetInt(L"SplitterbarPositionIRC",170);
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
	
	// since the minimize to tray button is not working under Aero (at least not at this point),
	// we enable map the minimize to tray on the minimize button by default if Aero is running
	if (IsRunningAeroGlassTheme())
		mintotray=ini.GetBool(L"MinToTray_Aero", true);
	else
		mintotray=ini.GetBool(L"MinToTray", false);

	m_bPreventStandby = ini.GetBool(L"PreventStandby", false);
	m_bStoreSearches = ini.GetBool(L"StoreSearches", true);
	m_bAddServersFromServer=ini.GetBool(L"AddServersFromServer",false);
	m_bAddServersFromClients=ini.GetBool(L"AddServersFromClient",false);
	splashscreen=ini.GetBool(L"Splashscreen",true);
	startupsound=ini.GetBool(L"Startupsound",true);//Commander - Added: Enable/Disable Startupsound
	sidebanner=ini.GetBool(L"Sidebanner",true);//Commander - Added: Side Banner
	bringtoforeground=ini.GetBool(L"BringToFront",true);
	transferDoubleclick=ini.GetBool(L"TransferDoubleClick",true);
	beepOnError=ini.GetBool(L"BeepOnError",true);
	confirmExit=ini.GetBool(L"ConfirmExit",true);
	filterLANIPs=ini.GetBool(L"FilterBadIPs",true);
	m_bAllocLocalHostIP=ini.GetBool(L"AllowLocalHostIP",false);
	autoconnect=ini.GetBool(L"Autoconnect",false);
	showRatesInTitle=ini.GetBool(L"ShowRatesOnTitle",false);
	m_bIconflashOnNewMessage=ini.GetBool(L"IconflashOnNewMessage",true); //MORPH

	onlineSig=ini.GetBool(L"OnlineSignature",false);
	startMinimized=ini.GetBool(L"StartupMinimized",false);
	m_bAutoStart=ini.GetBool(L"AutoStart",false);
	m_bRestoreLastMainWndDlg=ini.GetBool(L"RestoreLastMainWndDlg",false);
	m_iLastMainWndDlgID=ini.GetInt(L"LastMainWndDlgID",0);
	m_bRestoreLastLogPane=ini.GetBool(L"RestoreLastLogPane",false);
	m_iLastLogPaneID=ini.GetInt(L"LastLogPaneID",0);
	m_bSafeServerConnect =ini.GetBool(L"SafeServerConnect",false);

	m_bTransflstRemain =ini.GetBool(L"TransflstRemainOrder",false);
	filterserverbyip=ini.GetBool(L"FilterServersByIP",true); //MORPH leuk_he Changed default from false to true to fight fake servers
	filterlevel=ini.GetInt(L"FilterLevel",127);
	checkDiskspace=ini.GetBool(L"CheckDiskspace",false);
	m_uMinFreeDiskSpace=ini.GetInt(L"MinFreeDiskSpace",20*1024*1024);
	m_bSparsePartFiles=ini.GetBool(L"SparsePartFiles",false);
	m_bResolveSharedShellLinks=ini.GetBool(L"ResolveSharedShellLinks",false);
	m_bKeepUnavailableFixedSharedDirs = ini.GetBool(L"KeepUnavailableFixedSharedDirs", false);
	m_strYourHostname=ini.GetString(L"YourHostname", L"");

	// Barry - New properties...
	m_bAutoConnectToStaticServersOnly = ini.GetBool(L"AutoConnectStaticOnly",false); 
	autotakeed2klinks = ini.GetBool(L"AutoTakeED2KLinks",true); 
	addnewfilespaused = ini.GetBool(L"AddNewFilesPaused",false); 
	depth3D = ini.GetInt(L"3DDepth", 5);
	m_bEnableMiniMule = ini.GetBool(L"MiniMule", true);

	// Notifier
	notifierConfiguration = ini.GetString(L"NotifierConfiguration", GetMuleDirectory(EMULE_CONFIGDIR) + L"Notifier.ini");
    notifierOnDownloadFinished = ini.GetBool(L"NotifyOnDownload");
	notifierOnNewDownload = ini.GetBool(L"NotifyOnNewDownload");
    notifierOnChat = ini.GetBool(L"NotifyOnChat");
    notifierOnLog = ini.GetBool(L"NotifyOnLog");
	notifierOnImportantError = ini.GetBool(L"NotifyOnImportantError");
	notifierOnEveryChatMsg = ini.GetBool(L"NotifierPopEveryChatMessage");
	notifierOnNewVersion = ini.GetBool(L"NotifierPopNewVersion");
    notifierSoundType = (ENotifierSoundType)ini.GetInt(L"NotifierUseSound", ntfstNoSound);
	notifierSoundFile = ini.GetString(L"NotifierSoundPath");

	m_strDateTimeFormat = ini.GetString(L"DateTimeFormat", L"%A, %c");
	m_strDateTimeFormat4Log = ini.GetString(L"DateTimeFormat4Log", L"%c");
	m_strDateTimeFormat4Lists = ini.GetString(L"DateTimeFormat4Lists", L"%c");

	m_strIRCServer = ini.GetString(L"DefaultIRCServerNew", L"ircchat.emule-project.net");
	m_strIRCNick = ini.GetString(L"IRCNick");
	m_bIRCAddTimeStamp = ini.GetBool(L"IRCAddTimestamp", true);
	m_bIRCUseChannelFilter = ini.GetBool(L"IRCUseFilter", true);
	m_strIRCChannelFilter = ini.GetString(L"IRCFilterName", L"");
	if (m_strIRCChannelFilter.IsEmpty())
		m_bIRCUseChannelFilter = false;
	m_uIRCChannelUserFilter = ini.GetInt(L"IRCFilterUser", 0);
	m_strIRCPerformString = ini.GetString(L"IRCPerformString");
	m_bIRCUsePerform = ini.GetBool(L"IRCUsePerform", false);
	m_bIRCGetChannelsOnConnect = ini.GetBool(L"IRCListOnConnect", true);
	m_bIRCAcceptLinks = ini.GetBool(L"IRCAcceptLink", true);
	m_bIRCAcceptLinksFriendsOnly = ini.GetBool(L"IRCAcceptLinkFriends", true);
	m_bIRCPlaySoundEvents = ini.GetBool(L"IRCSoundEvents", false);
	m_bIRCIgnoreMiscMessages = ini.GetBool(L"IRCIgnoreMiscMessages", false);
	m_bIRCIgnoreJoinMessages = ini.GetBool(L"IRCIgnoreJoinMessages", true);
	m_bIRCIgnorePartMessages = ini.GetBool(L"IRCIgnorePartMessages", true);
	m_bIRCIgnoreQuitMessages = ini.GetBool(L"IRCIgnoreQuitMessages", true);
	m_bIRCIgnoreEmuleAddFriendMsgs = ini.GetBool(L"IRCIgnoreEmuleAddFriendMsgs", false);
	m_bIRCAllowEmuleAddFriend = ini.GetBool(L"IRCAllowEmuleAddFriend", true);
	m_bIRCIgnoreEmuleSendLinkMsgs = ini.GetBool(L"IRCIgnoreEmuleSendLinkMsgs", false);
	m_bIRCJoinHelpChannel = ini.GetBool(L"IRCHelpChannel", true);
	m_bIRCEnableSmileys = ini.GetBool(L"IRCEnableSmileys", true);
	m_bMessageEnableSmileys = ini.GetBool(L"MessageEnableSmileys", true);

	m_bSmartServerIdCheck = ini.GetBool(L"SmartIdCheck",true);
	log2disk = ini.GetBool(L"SaveLogToDisk",false);
	uMaxLogFileSize = ini.GetInt(L"MaxLogFileSize", 1024*1024);
	iMaxLogBuff = ini.GetInt(L"MaxLogBuff",64) * 1024;
	// MORPH START leuk_he Advanced official preferences.
	if (iMaxLogBuff  < 64*1024)  iMaxLogBuff =  64*1024;
	if (iMaxLogBuff  > 512*1024) iMaxLogBuff =512*1024;
	// MORPH END leuk_he Advanced official preferences.
	m_iLogFileFormat = (ELogFileFormat)ini.GetInt(L"LogFileFormat", Unicode);
	m_bEnableVerboseOptions=ini.GetBool(L"VerboseOptions", true);
	if (m_bEnableVerboseOptions)
	{
		m_bVerbose=ini.GetBool(L"Verbose",false);
		m_bFullVerbose=ini.GetBool(L"FullVerbose",false);
		debug2disk=ini.GetBool(L"SaveDebugToDisk",false);
		m_bDebugSourceExchange=ini.GetBool(L"DebugSourceExchange",false);
		m_bLogBannedClients=ini.GetBool(L"LogBannedClients", true);
		m_bLogRatingDescReceived=ini.GetBool(L"LogRatingDescReceived",true);
		m_bLogSecureIdent=ini.GetBool(L"LogSecureIdent",true);
		m_bLogFilteredIPs=ini.GetBool(L"LogFilteredIPs",true);
		m_bLogFileSaving=ini.GetBool(L"LogFileSaving",false);
        m_bLogA4AF=ini.GetBool(L"LogA4AF",false); // ZZ:DownloadManager
		m_bLogUlDlEvents=ini.GetBool(L"LogUlDlEvents",true);

		// MORPH START - Added by Commander, WebCache 1.2e
		m_bLogICHEvents=ini.GetBool(_T("LogICHEvents"),true);//JP log ICH events
		// MORPH END - Added by Commander, WebCache 1.2e
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
	m_bupdatequeuelist=ini.GetBool(L"UpdateQueueListPref",false);
	m_bManualAddedServersHighPriority=ini.GetBool(L"ManualHighPrio",false);
	m_btransferfullchunks=ini.GetBool(L"FullChunkTransfers",true);
	m_istartnextfile=ini.GetInt(L"StartNextFile",0);
	m_bshowoverhead=ini.GetBool(L"ShowOverhead",false);
	moviePreviewBackup=ini.GetBool(L"VideoPreviewBackupped",true);
	m_iPreviewSmallBlocks=ini.GetInt(L"PreviewSmallBlocks", 0);
	m_bPreviewCopiedArchives=ini.GetBool(L"PreviewCopiedArchives", true);
	m_iInspectAllFileTypes=ini.GetInt(L"InspectAllFileTypes", 0);
	m_bAllocFull=ini.GetBool(L"AllocateFullFile",0);
	m_bAutomaticArcPreviewStart=ini.GetBool(L"AutoArchivePreviewStart", true);
	m_bShowSharedFilesDetails = ini.GetBool(L"ShowSharedFilesDetails", true);
	m_bAutoShowLookups = ini.GetBool(L"AutoShowLookups", true);
	m_bShowUpDownIconInTaskbar = ini.GetBool(L"ShowUpDownIconInTaskbar", false );
	m_bShowWin7TaskbarGoodies  = ini.GetBool(L"ShowWin7TaskbarGoodies", true);
	m_bForceSpeedsToKB = ini.GetBool(L"ForceSpeedsToKB", false);
	m_bExtraPreviewWithMenu = ini.GetBool(L"ExtraPreviewWithMenu", false);

	// read file buffer size (with backward compatibility)
	m_iFileBufferSize=ini.GetInt(L"FileBufferSizePref",0); // old setting
	if (m_iFileBufferSize == 0)
		m_iFileBufferSize = 256*1024;
	else
		m_iFileBufferSize = ((m_iFileBufferSize*15000 + 512)/1024)*1024;
	m_iFileBufferSize=ini.GetInt(L"FileBufferSize",m_iFileBufferSize);
	m_uFileBufferTimeLimit = SEC2MS(ini.GetInt(L"FileBufferTimeLimit", 60));

	// read queue size (with backward compatibility)
	m_iQueueSize=ini.GetInt(L"QueueSizePref",0); // old setting
	if (m_iQueueSize == 0)
		m_iQueueSize = 50*100;
	else
		m_iQueueSize = m_iQueueSize*100;
	m_iQueueSize=ini.GetInt(L"QueueSize",m_iQueueSize);

	m_iCommitFiles=ini.GetInt(L"CommitFiles", 1); // 1 = "commit" on application shut down; 2 = "commit" on each file saveing
	versioncheckdays=ini.GetInt(L"Check4NewVersionDelay",5);
	m_bDAP=ini.GetBool(L"DAPPref",true);
	m_bUAP=ini.GetBool(L"UAPPref",true);
	m_bPreviewOnIconDblClk=ini.GetBool(L"PreviewOnIconDblClk",false);
	m_bCheckFileOpen=ini.GetBool(L"CheckFileOpen",true);
	indicateratings=ini.GetBool(L"IndicateRatings",true);
	watchclipboard=ini.GetBool(L"WatchClipboard4ED2kFilelinks",false);
	m_iSearchMethod=ini.GetInt(L"SearchMethod",0);

	showCatTabInfos=ini.GetBool(L"ShowInfoOnCatTabs",false);
//	resumeSameCat=ini.GetBool(L"ResumeNextFromSameCat",false);
	dontRecreateGraphs =ini.GetBool(L"DontRecreateStatGraphsOnResize",false);
	m_bExtControls =ini.GetBool(L"ShowExtControls",false);
	// MORPH START show less controls
	m_bShowLessControls =ini.GetBool(L"ShowLessControls",false);
    // MORPH END  show less controls
	versioncheckLastAutomatic=ini.GetInt(L"VersionCheckLastAutomatic",0);
	//MORPH START - Added by SiRoB, New Version check
	mversioncheckLastAutomatic=ini.GetInt(L"MVersionCheckLastAutomatic",0);
	//MORPH END   - Added by SiRoB, New Version check
	m_bDisableKnownClientList=ini.GetBool(L"DisableKnownClientList",false);
	m_bDisableQueueList=ini.GetBool(L"DisableQueueList",false);
	/*
	m_bCreditSystem=ini.GetBool(L"UseCreditSystem",true);
	*/
	m_bCreditSystem=true; //MORPH - Changed by SiRoB, CreditSystem allways used
	scheduler=ini.GetBool(L"EnableScheduler",false);
	msgonlyfriends=ini.GetBool(L"MessagesFromFriendsOnly",false);
	msgsecure=ini.GetBool(L"MessageFromValidSourcesOnly",true);
	m_bUseChatCaptchas = ini.GetBool(L"MessageUseCaptchas", true);
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
	if (m_iMaxChatHistory > 2048)  m_iMaxChatHistory = 2048;// MORPH leuk_he Advanced official preferences.
	maxmsgsessions=ini.GetInt(L"MaxMessageSessions",50);
	if (maxmsgsessions > 6000)  maxmsgsessions = 6000;// MORPH leuk_he Advanced official preferences.
	if (maxmsgsessions < 0 )  maxmsgsessions = 0;     // MORPH leuk_he Advanced official preferences.

	m_bShowActiveDownloadsBold = ini.GetBool(L"ShowActiveDownloadsBold", false);

	m_strTxtEditor = ini.GetString(L"TxtEditor", L"notepad.exe");
	m_strVideoPlayer = ini.GetString(L"VideoPlayer", L"");
	m_strVideoPlayerArgs = ini.GetString(L"VideoPlayerArgs",L"");
	
	m_strTemplateFile = ini.GetString(L"WebTemplateFile", GetMuleDirectory(EMULE_EXECUTEABLEDIR) + L"eMule.tmpl");
	// if emule is using the default, check if the file is in the config folder, as it used to be in prior version
	// and might be wanted by the user when switching to a personalized template
	if (m_strTemplateFile.Compare(GetMuleDirectory(EMULE_EXECUTEABLEDIR) + L"eMule.tmpl") == 0){
		CFileFind ff;
		if (ff.FindFile(GetMuleDirectory(EMULE_CONFIGDIR) + L"eMule.tmpl"))
			m_strTemplateFile = GetMuleDirectory(EMULE_CONFIGDIR) + L"eMule.tmpl";
		ff.Close();
	}

	messageFilter=ini.GetStringLong(L"MessageFilter",L"fastest download speed|fastest eMule|DI-Emule|eMule FX|ZamBoR 2|HyperMu|Ultra"); // leuk_he: add some known spammers
	/* MORPH START modified commentfilter
	commentFilter = ini.GetStringLong(L"CommentFilter",L"http://|https://|ftp://|www.|ftp.");
	*/
	commentFilter = ini.GetStringLong(L"CommentFilter",L"http://|https://|ftp://|www.|ftp.|hypermu");
	// MORPH END modified commentfilter 

	commentFilter.MakeLower();
	filenameCleanups=ini.GetStringLong(L"FilenameCleanups",L"http|www.|.com|.de|.org|.net|shared|powered|sponsored|sharelive|filedonkey|saugstube|eselfilme|eseldownloads|emulemovies|spanishare|eselpsychos.de|saughilfe.de|goldesel.6x.to|freedivx.org|elitedivx|deviance|-ftv|ftv|-flt|flt");
	m_iExtractMetaData = ini.GetInt(L"ExtractMetaData", 1); // 0=disable, 1=mp3, 2=MediaDet
	if (m_iExtractMetaData > 1)
		m_iExtractMetaData = 1;
	m_bAdjustNTFSDaylightFileTime=ini.GetBool(L"AdjustNTFSDaylightFileTime", true);
	m_bRearrangeKadSearchKeywords = ini.GetBool(L"RearrangeKadSearchKeywords", true);

	m_bUseSecureIdent=ini.GetBool(L"SecureIdent",true);
	m_bAdvancedSpamfilter=ini.GetBool(L"AdvancedSpamFilter",true);
	m_bRemoveFinishedDownloads=ini.GetBool(L"AutoClearCompleted",false);
	m_bUseOldTimeRemaining= ini.GetBool(L"UseSimpleTimeRemainingcomputation",false);

	// Toolbar
	m_sToolbarSettings = ini.GetString(L"ToolbarSetting", strDefaultToolbar);
	m_sToolbarBitmap = ini.GetString(L"ToolbarBitmap", L"");
	m_sToolbarBitmapFolder = ini.GetString(L"ToolbarBitmapFolder", _T(""));
	if (m_sToolbarBitmapFolder.IsEmpty()) // We want GetDefaultDirectory to also create the folder, so we have to know if we use the default or not
		m_sToolbarBitmapFolder = GetDefaultDirectory(EMULE_TOOLBARDIR, true);
	m_nToolbarLabels = (EToolbarLabelType)ini.GetInt(L"ToolbarLabels", CMuleToolbarCtrl::GetDefaultLabelType());
	m_bReBarToolbar = ini.GetBool(L"ReBarToolbar", 1);
	m_sizToolbarIconSize.cx = m_sizToolbarIconSize.cy = ini.GetInt(L"ToolbarIconSize", 32);
	m_iStraightWindowStyles=ini.GetInt(L"StraightWindowStyles",0);
	m_bUseSystemFontForMainControls=ini.GetBool(L"UseSystemFontForMainControls",0);
	m_bRTLWindowsLayout = ini.GetBool(L"RTLWindowsLayout");
	m_strSkinProfile = ini.GetString(L"SkinProfile", L"");
	m_strSkinProfileDir = ini.GetString(L"SkinProfileDir", _T(""));
	if (m_strSkinProfileDir.IsEmpty()) // We want GetDefaultDirectory to also create the folder, so we have to know if we use the default or not
		m_strSkinProfileDir = GetDefaultDirectory(EMULE_SKINDIR, true);

    //Commander - Added: Invisible Mode [TPT] - Start
    m_bInvisibleMode = ini.GetBool(L"InvisibleMode", false);
	m_iInvisibleModeHotKeyModifier = ini.GetInt(L"InvisibleModeHKKeyModifier", MOD_CONTROL | MOD_SHIFT | MOD_ALT);
	m_cInvisibleModeHotKey = (char)ini.GetInt(L"InvisibleModeHKKey",(int)'E');
    SetInvisibleMode(m_bInvisibleMode  ,m_iInvisibleModeHotKeyModifier ,m_cInvisibleModeHotKey );
	//Commander - Added: Invisible Mode [TPT] - End

    //MORPH START - Added by Commander, ClientQueueProgressBar
	m_bClientQueueProgressBar=ini.GetBool(L"ClientQueueProgressBar",false);
    //MORPH END - Added by Commander, ClientQueueProgressBar
	
	//MORPH START - Added by Commander, FolderIcons
	m_bShowFolderIcons=ini.GetBool(L"ShowFolderIcons",false);
	//MORPH END - Added by Commander, FolderIcons

    m_bShowClientPercentage=ini.GetBool(L"ShowClientPercentage",false);  //Commander - Added: Client Percentage
	enableDownloadInRed = ini.GetBool(L"EnableDownloadInRed", true); //MORPH - Added by IceCream, show download in red
	m_bShowActiveDownloadsBold = ini.GetBool(L"EnableDownloadInBold", true); //MORPH - Added by SiRoB, show download in Bold
        //MORPH START - Added by schnulli900, filter clients with failed downloads [Xman]
	m_bFilterClientFailedDown = ini.GetBool(L"FilterClientFailedDown", true);
        //MORPH END   - Added by schnulli900, filter clients with failed downloads [Xman]
	enableAntiLeecher = ini.GetBool(L"EnableAntiLeecher", true); //MORPH - Added by IceCream, enable AntiLeecher
	enableAntiCreditHack = ini.GetBool(L"EnableAntiCreditHack", true); //MORPH - Added by IceCream, enable AntiCreditHack
	enableHighProcess = ini.GetBool(L"EnableHighProcess", false); //MORPH - Added by IceCream, high process priority
	creditSystemMode = ini.GetInt(L"CreditSystemMode", 0/*Official*/); // EastShare - Added by linekin, ES CreditSystem
	if (    (creditSystemMode <0 )  || (creditSystemMode >3)) // MORPH leuk_he only valid credit systems in morph
       creditSystemMode =0;  // MORPH leuk_he only valid credit systems in morph 
	m_bEnableEqualChanceForEachFile = ini.GetBool(L"EqualChanceForEachFile", false);//Morph - added by AndCycle, Equal Chance For Each File
	m_bFollowTheMajority = ini.GetBool(L"FollowTheMajority", false); // EastShare       - FollowTheMajority by AndCycle
	m_iFairPlay = ini.GetInt(L"FairPlay", 0); //EastShare	- FairPlay by AndCycle
        
	//MORPH START added by Yun.SF3: Ipfilter.dat update
	m_uIPFilterVersionNum = ini.GetInt(L"IPFilterVersionNum",0); //MORPH - Added by Stulle, New IP Filter by Ozzy [Stulle/Ozzy]
	LPBYTE pst = NULL;
	UINT usize = sizeof m_IPfilterVersion;
	if (ini.GetBinary(L"IPfilterVersion", &pst, &usize) && usize == sizeof m_IPfilterVersion)
		memcpy(&m_IPfilterVersion, pst, sizeof m_IPfilterVersion);
	else
		memset(&m_IPfilterVersion, 0, sizeof m_IPfilterVersion);
	delete[] pst;
	AutoUpdateIPFilter=ini.GetBool(L"AutoUPdateIPFilter",false); //added by milobac: Ipfilter.dat update
	//MORPH END added by Yun.SF3: Ipfilter.dat update
    
    //Commander - Added: IP2Country Auto-updating - Start
	pst = NULL;
	usize = sizeof m_IP2CountryVersion;
	if (ini.GetBinary(L"IP2CountryVersion", &pst, &usize) && usize == sizeof m_IP2CountryVersion)
		memcpy(&m_IP2CountryVersion, pst, sizeof m_IP2CountryVersion);
	else
		memset(&m_IP2CountryVersion, 0, sizeof m_IP2CountryVersion);
	delete[] pst;
	AutoUpdateIP2Country=ini.GetBool(L"AutoUPdateIP2Country",false);
    //Commander - Added: IP2Country Auto-updating - End

	//MORPH START - Added by milobac, FakeCheck, FakeReport, Auto-updating
	pst = NULL;
	usize = sizeof m_FakesDatVersion;
	if (ini.GetBinary(L"FakesDatVersion", &pst, &usize) && usize == sizeof m_FakesDatVersion)
		memcpy(&m_FakesDatVersion, pst, sizeof m_FakesDatVersion);
	else
		memset(&m_FakesDatVersion, 0, sizeof m_FakesDatVersion);
	delete[] pst;
	UpdateFakeStartup=ini.GetBool(L"UpdateFakeStartup",false);
	//MORPH END - Added by milobac, FakeCheck, FakeReport, Auto-updating

	//EastShare - added by AndCycle, IP to Country
	m_iIP2CountryNameMode = ini.GetInt(L"IP2Country", IP2CountryName_DISABLE); 
	m_bIP2CountryShowFlag = ini.GetBool(L"IP2CountryShowFlag", false);
	//EastShare - added by AndCycle, IP to Country
	
	//MORPH START - Added by SiRoB, Datarate Average Time Management
	m_iDownloadDataRateAverageTime = 1000*max(1, (uint8)ini.GetInt(L"DownloadDataRateAverageTime",30));
	m_iUploadDataRateAverageTime = 1000*max(1, (uint8)ini.GetInt(L"UploadDataRateAverageTime",30));
	//MORPH END   - Added by SiRoB, Datarate Average Time Management

	//MORPH START - Added by SiRoB, Upload Splitting Class
	globaldataratefriend=ini.GetInt(L"GlobalDataRateFriend",3);
	maxglobaldataratefriend=ini.GetInt(L"MaxGlobalDataRateFriend",100);
	globaldataratepowershare=ini.GetInt(L"GlobalDataRatePowerShare",0);
	maxglobaldataratepowershare=ini.GetInt(L"MaxGlobalDataRatePowerShare",85); 
	maxclientdataratefriend=ini.GetInt(L"MaxClientDataRateFriend",0);
	maxclientdataratepowershare=ini.GetInt(L"MaxClientDataRatePowerShare",0);
	maxclientdatarate=ini.GetInt(L"MaxClientDataRate",0);
	//MORPH END   - Added by SiRoB, Upload Splitting Class
    // ==> Slot Limit - Stulle
	m_bSlotLimitThree = ini.GetBool(L"SlotLimitThree",true); // default
	if (!m_bSlotLimitThree)
		m_bSlotLimitNum = ini.GetBool(L"SlotLimitNumB",false);
	else
		m_bSlotLimitNum = false;
	int temp = ini.GetInt(L"SlotLimitNum",100);
	m_iSlotLimitNum = (uint8)((temp >= 60 && temp <= 255) ? temp : 100);
	// <== Slot Limit - Stulle


	//MORPH START - Added by SiRoB, SLUGFILLER: lowIdRetry
	LowIdRetries=ini.GetInt(L"ReconnectOnLowIdRetries",3);	// SLUGFILLER: lowIdRetry
	//MORPH END   - Added by SiRoB, SLUGFILLER: lowIdRetry
	//MORPH	Start	- Added by AndCycle, SLUGFILLER: Spreadbars - per file
	m_iSpreadbarSetStatus = ini.GetInt(L"SpreadbarSetStatus", 0);
	//MORPH	End	- Added by AndCycle, SLUGFILLER: Spreadbars - per file
	//MORPH START - Added by SiRoB, SLUGFILLER: hideOS
	hideOS=ini.GetInt(L"HideOvershares",0/*5*/);
	selectiveShare=ini.GetBool(L"SelectiveShare",false);
	//MORPH END   - Added by SiRoB, SLUGFILLER: hideOS
	//MORPH START - Added by SiRoB, SHARE_ONLY_THE_NEED
	ShareOnlyTheNeed=ini.GetInt(L"ShareOnlyTheNeed",false);
	//MORPH END   - Added by SiRoB, SHARE_ONLY_THE_NEED
	//MORPH START - Added by SiRoB, POWERSHARE Limit
	PowerShareLimit=ini.GetInt(L"PowerShareLimit",0);
	//MORPH END   - Added by SiRoB, POWERSHARE Limit
	//MORPH START - Added by SiRoB, Show Permissions
	permissions=ini.GetInt(L"ShowSharePermissions",0);
	//MORPH END   - Added by SiRoB, Show Permissions
	//EastShare - Added by Pretender, TBH-AutoBackup
	autobackup = ini.GetBool(L"AutoBackup",true);
	autobackup2 = ini.GetBool(L"AutoBackup2",true);
	//EastShare - Added by Pretender, TBH-AutoBackup
	infiniteQueue=ini.GetBool(L"InfiniteQueue",false);	//Morph - added by AndCycle, SLUGFILLER: infiniteQueue
	//MORPH START - Added by SiRoB, Avoid misusing of powersharing
	m_iPowershareMode=ini.GetInt(L"PowershareMode",2);
	//MORPH END   - Added by SiRoB, Avoid misusing of powersharing
	m_bPowershareInternalPrio = ini.GetBool(L"PowershareInternalPrio",false);//Morph - added by AndCyle, selective PS internal Prio

	//MORPH START - Added & Modified by SiRoB, Smart Upload Control v2 (SUC) [lovelace]
	m_bSUCEnabled = ini.GetBool(L"SUCEnabled",false);
	m_bSUCLog =  ini.GetBool(L"SUCLog",false);
	m_iSUCHigh = ini.GetInt(L"SUCHigh",900);
	m_iSUCHigh = min(max(m_iSUCHigh,350),1000);
	m_iSUCLow = ini.GetInt(L"SUCLow",600);
	m_iSUCLow = min(max(m_iSUCLow,350),m_iSUCHigh);
	m_iSUCPitch = ini.GetInt(L"SUCPitch",3000);
	m_iSUCPitch = min(max(m_iSUCPitch,2500),10000);
	m_iSUCDrift = ini.GetInt(L"SUCDrift",50);
	m_iSUCDrift = min(max(m_iSUCDrift,0),100);
	//MORPH END - Added & Modified by SiRoB, Smart Upload Control v2 (SUC) [lovelace]
	maxconnectionsswitchborder = ini.GetInt(L"MaxConnectionsSwitchBorder",100);//MORPH - Added by Yun.SF3, Auto DynUp changing
	maxconnectionsswitchborder = min(max(maxconnectionsswitchborder,50),60000);//MORPH - Added by Yun.SF3, Auto DynUp changing

	//EastShare Start - PreferShareAll by AndCycle
	shareall=ini.GetBool(L"ShareAll",true);	// SLUGFILLER: preferShareAll
	//EastShare END - PreferShareAll by AndCycle
	// EastShare START - Added by TAHO, .met file control
	m_iKnownMetDays = ini.GetInt(L"KnownMetDays", 0);
	m_bCompletlyPurgeOldKnownFiles = ini.GetBool(L"CompletlyPurgeOldKnownFiles",false);
	if(m_bPartiallyPurgeOldKnownFiles && m_bCompletlyPurgeOldKnownFiles)
		m_bCompletlyPurgeOldKnownFiles = false;
	m_bRemoveAichImmediatly = ini.GetBool(L"RemoveAichImmediatly",false);
	// EastShare END - Added by TAHO, .met file control
	//EastShare - Added by Pretender, Option for ChunkDots
	m_bEnableChunkDots=ini.GetBool(L"EnableChunkDots",true);
	//EastShare - Added by Pretender, Option for ChunkDots

	isautodynupswitching=ini.GetBool(L"AutoDynUpSwitching",false);
	m_bDateFileNameLog=ini.GetBool(L"DateFileNameLog", true);//Morph - added by AndCycle, Date File Name Log
	m_bPayBackFirst=ini.GetBool(L"IsPayBackFirst",false);//EastShare - added by AndCycle, Pay Back First
	m_iPayBackFirstLimit=(uint8)min(ini.GetInt(L"PayBackFirstLimit",10),255);//MORPH - Added by SiRoB, Pay Back First Tweak
	m_bOnlyDownloadCompleteFiles = ini.GetBool(L"OnlyDownloadCompleteFiles", false);//EastShare - Added by AndCycle, Only download complete files v2.1 (shadow)
	m_bSaveUploadQueueWaitTime = ini.GetBool(L"SaveUploadQueueWaitTime", true );//Morph - added by AndCycle, Save Upload Queue Wait Time (MSUQWT)
	m_bDontRemoveSpareTrickleSlot = ini.GetBool(L"DontRemoveSpareTrickleSlot", false);//Morph - added by AndCycle, Dont Remove Spare Trickle Slot --> default is now false because it disables slotfocus
	m_bUseDownloadOverhead= ini.GetBool(L"UseDownloadOverhead", true); //Morph - leuk_he use download overhead in upload
	
	m_bFunnyNick = ini.GetBool(L"DisplayFunnyNick", true);//MORPH - Added by SiRoB, Optionnal funnynick display
	_stprintf(UpdateURLFakeList,L"%s",ini.GetString(L"UpdateURLFakeList",L"http://emulepawcio.sourceforge.net/fakes.zip"));		//MORPH START - Added by milobac and Yun.SF3, FakeCheck, FakeReport, Auto-updating
	_stprintf(UpdateURLIPFilter,L"%s",ini.GetString(L"UpdateURLIPFilter",L"http://downloads.sourceforge.net/scarangel/ipfilter.rar"));//MORPH START added by Yun.SF3: Ipfilter.dat update
	_stprintf(UpdateURLIP2Country,L"%s",ini.GetString(L"UpdateURLIP2Country",L"http://geolite.maxmind.com/download/geoip/database/GeoIPCountryCSV.zip"));//Commander - Added: IP2Country auto-updating

	// khaos::categorymod+ Load Preferences
	m_bShowCatNames=ini.GetBool(L"ShowCatName",true);
	m_bValidSrcsOnly=ini.GetBool(L"ValidSrcsOnly", false);
	m_bActiveCatDefault=ini.GetBool(L"ActiveCatDefault", true);
	m_bSelCatOnAdd=ini.GetBool(L"SelCatOnAdd", true);
	m_bAutoSetResumeOrder=ini.GetBool(L"AutoSetResumeOrder", true);
	m_bSmallFileDLPush=ini.GetBool(L"SmallFileDLPush", true);
	m_iStartDLInEmptyCats=(uint8)ini.GetInt(L"StartDLInEmptyCats", 0);
	m_bUseAutoCat=ini.GetBool(L"UseAutoCat", true);
	m_bAddRemovedInc=ini.GetBool(L"AddRemovedInc",true);
	// khaos::categorymod-
	// MORPH START leuk_he disable catcolor
	m_bDisableCatColors=ini.GetBool(L"DisableCatColors", false);
	// MORPH END   leuk_he disable catcolor
	
	// khaos::kmod+
	m_bUseSaveLoadSources=ini.GetBool(L"UseSaveLoadSources", true);
	m_bRespectMaxSources=ini.GetBool(L"RespectMaxSources", true);
	m_bSmartA4AFSwapping=ini.GetBool(L"SmartA4AFSwapping", true);
	m_iAdvancedA4AFMode=(uint8)ini.GetInt(L"AdvancedA4AFMode", 1);
	m_bShowA4AFDebugOutput=ini.GetBool(L"ShowA4AFDebugOutput", false);
	// khaos::accuratetimerem+
	m_iTimeRemainingMode=(uint8)ini.GetInt(L"TimeRemainingMode", 0);
	// khaos::accuratetimerem-
	//MORPH START - Added by SiRoB, ICS Optional
	m_bUseIntelligentChunkSelection=ini.GetBool(L"UseIntelligentChunkSelection", true);
	//MORPH END   - Added by SiRoB, ICS Optional
	//MORPH START - Added by SiRoB, XML News [O²]
	enableNEWS=ini.GetBool(L"ShowNews", 1);
	//MORPH END   - Added by SiRoB, XML News [O²]

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
	//MORPH START - Added by SiRoB, Upload Splitting Class
	m_crLogUSC = ini.GetColRef(L"LogUploadSplittingClassColor", m_crLogUSC);
	//MORPH END   - Added by SiRoB, Upload Splitting Class

	if (statsAverageMinutes < 1)
		statsAverageMinutes = 5;

	// ZZ:UploadSpeedSense -->
	if (!m_bSUCEnabled) 	//MORPH START - Added by SiRoB,  Morph parameter transfer (USS)
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

	//MORPH START - Added by SiRoB,  USS log flag
	m_bDynUpLog = ini.GetBool(L"USSLog", true);
	//MORPH END   - Added by SiRoB,  USS log flag
	m_bUSSUDP = ini.GetBool(L"USSUDP_FORCE", false); //MORPH - Added by SiRoB, USS UDP preferency
    m_sPingDataSize = (short)ini.GetInt(L"USSPingDataSize", 0); //MORPH leuk_he ICMP ping datasize <> 0 setting
	m_bA4AFSaveCpu = ini.GetBool(L"A4AFSaveCpu", false); // ZZ:DownloadManager
    m_bHighresTimer = ini.GetBool(L"HighresTimer", false);
	m_bRunAsUser = ini.GetBool(L"RunAsUnprivilegedUser", false);
	m_bPreferRestrictedOverUser = ini.GetBool(L"PreferRestrictedOverUser", false);
	m_bOpenPortsOnStartUp = ini.GetBool(L"OpenPortsOnStartUp", false);
	m_byLogLevel = ini.GetInt(L"DebugLogLevel", DLP_VERYLOW);
	m_bTrustEveryHash = ini.GetBool(L"AICHTrustEveryHash", false);
	m_bRememberCancelledFiles = ini.GetBool(L"RememberCancelledFiles", true);
	m_bRememberDownloadedFiles = ini.GetBool(L"RememberDownloadedFiles", true);
	m_bPartiallyPurgeOldKnownFiles = ini.GetBool(L"PartiallyPurgeOldKnownFiles", true);

	m_bNotifierSendMail = ini.GetBool(L"NotifierSendMail", false);
#if _ATL_VER >= 0x0710
	if (!IsRunningXPSP2OrHigher())
		m_bNotifierSendMail = false;
#endif
	m_strNotifierMailSender = ini.GetString(L"NotifierMailSender", L"");
	m_strNotifierMailServer = ini.GetString(L"NotifierMailServer", L"");
	m_strNotifierMailReceiver = ini.GetString(L"NotifierMailRecipient", L"");

	m_bWinaTransToolbar = ini.GetBool(L"WinaTransToolbar", true);
	m_bShowDownloadToolbar = ini.GetBool(L"ShowDownloadToolbar", true);

	m_bCryptLayerRequested = ini.GetBool(L"CryptLayerRequested", false);
	m_bCryptLayerRequired = ini.GetBool(L"CryptLayerRequired", false);
	m_bCryptLayerSupported = ini.GetBool(L"CryptLayerSupported", true);
	m_dwKadUDPKey = ini.GetInt(L"KadUDPKey", GetRandomUInt32());

	uint32 nTmp = ini.GetInt(L"CryptTCPPaddingLength", 128);
	m_byCryptTCPPaddingLength = (uint8)min(nTmp, 254);

	m_bEnableSearchResultFilter = ini.GetBool(L"EnableSearchResultSpamFilter", true);

  m_bCryptLayerRequiredStrictServer = ini.GetBool(L"CryptLayerRequiredStrictServer",false); // MORPH lh require obfuscated server connection 

	// Mighty Knife: Community visualization, Report hashing files, Log friendlist activities
	_stprintf (m_sCommunityName,L"%s",ini.GetString (L"CommunityName"));
	m_bReportHashingFiles = ini.GetBool (L"ReportHashingFiles",true);
	m_bLogFriendlistActivities = ini.GetBool (L"LogFriendlistActivities",true);
	// [end] Mighty Knife

	// Mighty Knife: CRC32-Tag
	SetDontAddCRCToFilename (ini.GetBool (L"DontAddCRC32ToFilename",false));
	SetCRC32ForceUppercase (ini.GetBool (L"ForceCRC32Uppercase",false));
	SetCRC32ForceAdding (ini.GetBool (L"ForceCRC32Adding",false));
	// From the prefix/suffix delete the leading/trailing "".
	SetCRC32Prefix (ini.GetString(L"LastCRC32Prefix",L"\" [\"").Trim (L"\""));
	SetCRC32Suffix (ini.GetString(L"LastCRC32Suffix",L"\"]\"").Trim (L"\""));
	// [end] Mighty Knife

	// Mighty Knife: Simple cleanup options
	SetSimpleCleanupOptions (ini.GetInt (L"SimpleCleanupOptions",3));
	SetSimpleCleanupSearch (ini.GetString (L"SimpleCleanupSearch"));
	SetSimpleCleanupReplace (ini.GetString (L"SimpleCleanupReplace"));
	// Format of the preferences string for character replacement:
	//      "str";"str";"str";...;"str"
	// Every "str" in SimpleCleanupSearchChars corresponds to a "str"
	// in SimpleCleanupReplaceChars at the same position.
	SetSimpleCleanupSearchChars (ini.GetString (L"SimpleCleanupSearchChars",
								 L"\"\xE4\";\"\xF6\";\"\xFC\";\"\xC4\";\"\xD6\";\"\xDC\";\"\xDF\""));/*ISO 8859-4*/
	SetSimpleCleanupReplaceChars (ini.GetString (L"SimpleCleanupReplaceChars",
								 L"\"ae\";\"oe\";\"ue\";\"Ae\";\"Oe\";\"Ue\";\"ss\""));
	// [end] Mighty Knife

	// Mighty Knife: Static server handling
	SetDontRemoveStaticServers (ini.GetBool (L"DontRemoveStaticServers",false));
	// [end] Mighty Knife

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
	m_strStatsExpandedTreeItems = ini.GetString(L"statsExpandedTreeItems",L"111000000100000110000010000011110000010010",L"Statistics");
	CString buffer2;
	for (int i = 0; i < _countof(m_adwStatsColors); i++) {
		buffer2.Format(L"StatColor%i", i);
		m_adwStatsColors[i] = 0;
		if (_stscanf(ini.GetString(buffer2, L"", L"Statistics"), L"%i", &m_adwStatsColors[i]) != 1)
			ResetStatsColor(i);
	}
	bHasCustomTaskIconColor = ini.GetBool(L"HasCustomTaskIconColor",false, L"Statistics");
	m_bShowVerticalHourMarkers = ini.GetBool(L"ShowVerticalHourMarkers", true, L"Statistics");

	// -khaos--+++> Load Stats
	// I changed this to a seperate function because it is now also used
	// to load the stats backup and to load stats from preferences.ini.old.
	LoadStats();
	// <-----khaos-

	///////////////////////////////////////////////////////////////////////////
	// Section: "WebServer"
	//
	m_strWebPassword = ini.GetString(L"Password", L"", L"WebServer");
	m_strWebLowPassword = ini.GetString(L"PasswordLow", L"");
	m_nWebPort=(uint16)ini.GetInt(L"Port", 4711);
#ifdef USE_OFFICIAL_UPNP
	m_bWebUseUPnP = ini.GetBool(L"WebUseUPnP", false);
#endif
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
	//>>> [ionix] - iONiX::Advanced WebInterface Account Management
	m_bIonixWebsrv=ini.GetBool(L"UseIonixWebsrv", false);
	//<<< [ionix] - iONiX::Advanced WebInterface Account Management
	

	///////////////////////////////////////////////////////////////////////////
	// Section: "MobileMule"
	//
	m_strMMPassword = ini.GetString(L"Password", L"", L"MobileMule");
	m_bMMEnabled = ini.GetBool(L"Enabled", false);
	m_nMMPort = (uint16)ini.GetInt(L"Port", 80);

	///////////////////////////////////////////////////////////////////////////
	// Section: "PeerCache"
	//
	m_uPeerCacheLastSearch = ini.GetInt(L"LastSearch", 0, L"PeerCache");
	m_bPeerCacheWasFound = ini.GetBool(L"Found", false);
	m_bPeerCacheEnabled = ini.GetBool(L"EnabledDeprecated", false);
	m_nPeerCachePort = (uint16)ini.GetInt(L"PCPort", 0);
	m_bPeerCacheShow = true; //ini.GetBool(L"Show", false); //allways see peercache

#ifdef USE_OFFICIAL_UPNP
	///////////////////////////////////////////////////////////////////////////
	// Section: "UPnP"
	//
	m_bEnableUPnP = ini.GetBool(L"EnableUPnP", false, L"UPnP");
	m_bSkipWANIPSetup = ini.GetBool(L"SkipWANIPSetup", false);
	m_bSkipWANPPPSetup = ini.GetBool(L"SkipWANPPPSetup", false);
	m_bCloseUPnPOnExit = ini.GetBool(L"CloseUPnPOnExit", true);
	m_nLastWorkingImpl = ini.GetInt(L"LastWorkingImplementation", 1 /*MiniUPnPLib*/);
	m_bIsMinilibImplDisabled = ini.GetBool(L"DisableMiniUPNPLibImpl", false);
	m_bIsWinServImplDisabled = ini.GetBool(L"DisableWinServImpl", false);
#endif

    //MORPH START - Added by SiRoB, [MoNKi: -UPnPNAT Support-]
	m_bUPnPNat = ini.GetBool(L"UPnPNAT", false, L"eMule");
	m_bUPnPNatWeb = ini.GetBool(L"UPnPNAT_Web", false, L"eMule");
	m_bUPnPVerboseLog = ini.GetBool(L"UPnPVerbose", true, L"eMule");
	m_iUPnPPort = (uint16)ini.GetInt(L"UPnPPort", 0, L"eMule");
	m_bUPnPLimitToFirstConnection = ini.GetBool(L"UPnPLimitToFirstConnection", false, L"eMule");
	m_bUPnPClearOnClose = ini.GetBool(L"UPnPClearOnClose", true, L"eMule");
    SetUpnpDetect(ini.GetInt(L"uPnPDetect", UPNP_DO_AUTODETECT, L"eMule")); //leuk_he autodetect upnp in wizard
    m_bUPnPForceUpdate=ini.GetBool(L"UPnPForceUpdate", false);
	//MORPH END   - Added by SiRoB, [MoNKi: -UPnPNAT Support-]

	//MORPH START - Added by SiRoB, [MoNKi: -Random Ports-]
	m_bRndPorts = ini.GetBool(L"RandomPorts", false, L"eMule");
	m_iMinRndPort = (uint16)ini.GetInt(L"MinRandomPort", 3000, L"eMule");
	m_iMaxRndPort = (uint16)ini.GetInt(L"MaxRandomPort", 0xFFFF, L"eMule");
	m_bRndPortsResetOnRestart = ini.GetBool(L"RandomPortsReset", false, L"eMule");
	m_iRndPortsSafeResetOnRestartTime = (uint16)ini.GetInt(L"RandomPortsSafeResetOnRestartTime", 0, L"eMule");
	
	uint16 iOldRndTCPPort = (uint16)ini.GetInt(L"OldTCPRandomPort", 0, L"eMule");
	uint16 iOldRndUDPPort = (uint16)ini.GetInt(L"OldUDPRandomPort", 0, L"eMule");
	__time64_t iRndPortsLastRun = ini.GetUInt64(L"RandomPortsLastRun", 0, L"eMule");
	
	m_iCurrentTCPRndPort = 0;
	m_iCurrentUDPRndPort = 0;

	if(m_bRndPorts && !m_bRndPortsResetOnRestart &&
		m_iRndPortsSafeResetOnRestartTime != 0)
	{
		CTime tNow = CTime::GetCurrentTime();
		CTime tOld = CTime(iRndPortsLastRun);
		CTimeSpan ts = tNow - tOld;
		if(ts.GetTimeSpan() <= m_iRndPortsSafeResetOnRestartTime){
			m_iCurrentTCPRndPort = iOldRndTCPPort;
			m_iCurrentUDPRndPort = iOldRndUDPPort;
		}
	}
	m_bRndPortsResetOnRestart = false;
	//MORPH END   - Added by SiRoB, [MoNKi: -Random Ports-]

	//MORPH START - Added by by SiRoB, [MoNKi: -Improved ICS-Firewall support-]
	m_bICFSupportStatusChanged = false;
	m_bICFSupport = ini.GetBool(L"ICFSupport", false, L"eMule");
	m_bICFSupportFirstTime = ini.GetBool(L"ICFSupportFirstTime", true, L"eMule");
	m_bICFSupportServerUDP = ini.GetBool(L"ICFSupportServerUDP", false, L"eMule");
	//MORPH END   - Added by by SiRoB, [MoNKi: -Improved ICS-Firewall support-]

	///////////////////////////////////////////////////////////////////////////
	// Section: "WapServer"
	//
    //MORPH START - Added by SiRoB / Commander, Wapserver [emulEspaña]
	m_bWapEnabled=ini.GetBool(L"WapEnabled", false, L"WapServer");
	_stprintf(m_sWapTemplateFile,L"%s",ini.GetString(L"WapTemplateFile",L"eMule_Wap.tmpl",L"WapServer"));
	m_nWapPort=(uint16)ini.GetInt(L"WapPort", 80, L"WapServer");
	m_iWapGraphWidth=ini.GetInt(L"WapGraphWidth", 60, L"WapServer");
	m_iWapGraphHeight=ini.GetInt(L"WapGraphHeight", 45, L"WapServer");
	m_bWapFilledGraphs=ini.GetBool(L"WapFilledGraphs", false, L"WapServer");
	m_iWapMaxItemsInPages = ini.GetInt(L"WapMaxItemsInPage", 5, L"WapServer");
	m_bWapSendImages=ini.GetBool(L"WapSendImages", true, L"WapServer");
	m_bWapSendGraphs=ini.GetBool(L"WapSendGraphs", true, L"WapServer");
	m_bWapSendProgressBars=ini.GetBool(L"WapSendProgressBars", true, L"WapServer");
	m_bWapAllwaysSendBWImages=ini.GetBool(L"WapSendBWImages", true, L"WapServer");
	m_iWapLogsSize=ini.GetInt(L"WapLogsSize", 1024, L"WapServer");
	m_sWapPassword = ini.GetString(L"WapPassword", L"WapServer", L"WapServer");
	m_sWapLowPassword = ini.GetString(L"WapPasswordLow", L"", L"WapServer");
	m_bWapLowEnabled = ini.GetBool(L"WapLowEnable", false, L"WapServer");
	//MORPH END - Added by SiRoB / Commander, Wapserver [emulEspaña]

	//MORPH START - Added by Stulle, Global Source Limit
	m_bGlobalHL = ini.GetBool(L"GlobalHL", true, L"eMule");
	uint32 m_uGlobalHlStandard = 3500;
	if (maxGraphUploadRate != UNLIMITED)
	{
		m_uGlobalHlStandard = (uint32)(maxGraphUploadRate*0.9f);
		m_uGlobalHlStandard = (uint32)((m_uGlobalHlStandard*400 - (m_uGlobalHlStandard-10.0f)*100)*0.65f);
		m_uGlobalHlStandard = max(1000,min(m_uGlobalHlStandard,MAX_GSL));
	}
	uint32 m_uTemp = ini.GetInt(L"GlobalHLvalue", m_uGlobalHlStandard);
	m_uGlobalHL = (m_uTemp >= 1000 && m_uTemp <= MAX_GSL) ? m_uTemp : m_uGlobalHlStandard;
	//MORPH END   - Added by Stulle, Global Source Limit

	//MORPH START leuk_he Advanced official preferences.
	bMiniMuleAutoClose=ini.GetBool(L"MiniMuleAutoClose",0,L"eMule");
	iMiniMuleTransparency=min(ini.GetInt(L"MiniMuleTransparency",0),100); // range 0..100
	bCreateCrashDump=ini.GetBool(L"CreateCrashDump",0);
	bCheckComctl32 =ini.GetBool(L"CheckComctl32",true);
	bCheckShell32=ini.GetBool(L"CheckShell32",true);
	bIgnoreInstances=ini.GetBool(L"IgnoreInstance",false);
	sNotifierMailEncryptCertName=ini.GetString(L"NotifierMailEncryptCertName",L"");
	sMediaInfo_MediaInfoDllPath=ini.GetString(L"MediaInfo_MediaInfoDllPath",L"MEDIAINFO.DLL") ;
	bMediaInfo_RIFF=ini.GetBool(L"MediaInfo_RIFF",true);
	bMediaInfo_ID3LIB =ini.GetBool(L"MediaInfo_ID3LIB",true);
#ifdef HAVE_QEDIT_H
	m_bMediaInfo_MediaDet =ini.GetBool(_T("MediaInfo_MediaDet"),true);
#endif//HAVE_QEDIT_H
	m_bMediaInfo_RM =ini.GetBool(_T("MediaInfo_RM"),true);
#ifdef HAVE_WMSDK_H
	m_bMediaInfo_WM =ini.GetBool(_T("MediaInfo_WM"),true);
#endif//HAVE_WMSDK_H
	sInternetSecurityZone=ini.GetString(L"InternetSecurityZone",L"Untrusted");
    m_iDebugSearchResultDetailLevel = ini.GetInt(L"DebugSearchResultDetailLevel", 0); // NOTE: this variable is also initialized to 0 above! 
	// MORPH END  leuk_he Advanced official preferences. 


 	//MORPH START - Added, Downloaded History [Monki/Xman]
	m_bHistoryShowShared = ini.GetBool(L"ShowSharedInHistory", false);
	//MORPH END   - Added, Downloaded History [Monki/Xman]
   	m_bUseCompression=ini.GetBool(L"UseCompression",true);//Xman disable compression
	//MORPH START leuk_he:run as ntservice v1..
	GetServiceName();
	m_iServiceOptLvl = ini.GetInt(L"ServiceOptLvlNew",-1);
	if(m_iServiceOptLvl < 0) // import old settings
	{
		m_iServiceOptLvl = ini.GetInt(L"ServiceOptLvl",-1);
		switch(m_iServiceOptLvl)
		{
		case 0:
			m_iServiceOptLvl = SVC_NO_OPT;
			break;
		case 4:
			m_iServiceOptLvl = SVC_BASIC_OPT;
			break;
		case 6:
			m_iServiceOptLvl = SVC_LIST_OPT;
			break;
		case 10:
			m_iServiceOptLvl = SVC_FULL_OPT;
			break;
		default: // this will be the default in case there was nothing to import
			m_iServiceOptLvl = SVC_LIST_OPT;
			break;
		}
	}
	else if(m_iServiceOptLvl == 4) // Import from future StulleMule versions
		m_iServiceOptLvl = SVC_BASIC_OPT;
	//MORPH END leuk_he:run as ntservice v1..
	m_bStaticIcon = ini.GetBool(L"StaticIcon",false); //MORPH - Added, Static Tray Icon
	m_bFakeAlyzerIndications = ini.GetBool(L"FakeAlyzerIndications",false); //MORPH - Added by Stulle, Fakealyzer [netfinity]
	m_strBrokenURLs = ini.GetStringLong(L"BrokenURLs", L"sourceforge"); //MORPH - Added by WiZaRd, Fix broken HTTP downloads

	// ==> [MoNKi: -USS initial TTL-] - Stulle
	m_iUSSinitialTTL = (uint8)ini.GetInt(L"USSInitialTTL", 1,L"StulleMule");
	// <== [MoNKi: -USS initial TTL-] - Stulle
	//MORPH START - Added by Stulle, Adjustable NT Service Strings
	m_strServiceName = ini.GetString(L"ServiceName",NULL);
	m_strServiceDispName = ini.GetString(L"ServiceDispName",NULL);
	m_strServiceDescr = ini.GetString(L"ServiceDescr",NULL);
	m_bServiceStringsLoaded = true;
	//MORPH END   - Added by Stulle, Adjustable NT Service Strings

	LoadCats();
	SetLanguage();
}

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

void CPreferences::SaveCats()
{
	CString strCatIniFilePath;
	strCatIniFilePath.Format(L"%sCategory.ini", GetMuleDirectory(EMULE_CONFIGDIR));
	(void)_tremove(strCatIniFilePath);
	CIni ini(strCatIniFilePath);
	ini.WriteInt(L"Count", catMap.GetCount() - 1, L"General");
	ini.WriteInt(L"CategoryVersion", 2, L"General"); // khaos::categorymod+
	for (int i = 0; i < catMap.GetCount(); i++)
	{
		CString strSection;
		strSection.Format(L"Cat#%i", i);
		ini.SetSection(strSection);

		ini.WriteStringUTF8(L"Title", catMap.GetAt(i)->strTitle);
		ini.WriteStringUTF8(L"Incoming", catMap.GetAt(i)->strIncomingPath);
		ini.WriteStringUTF8(L"Comment", catMap.GetAt(i)->strComment);
		/*Removed by Stulle
		ini.WriteStringUTF8(L"RegularExpression", catMap.GetAt(i)->regexp);
		*/
		ini.WriteInt(L"Color", catMap.GetAt(i)->color);
		ini.WriteInt(L"a4afPriority", catMap.GetAt(i)->prio); // ZZ:DownloadManager
		ini.WriteInt(L"AdvancedA4AFMode", catMap.GetAt(i)->iAdvA4AFMode);
		/*Removed by SiRoB
		ini.WriteStringUTF8(L"AutoCat", catMap.GetAt(i)->autocat);
		ini.WriteInt(L"Filter", catMap.GetAt(i)->filter);
		ini.WriteBool(L"FilterNegator", catMap.GetAt(i)->filterNeg);
		ini.WriteBool(L"AutoCatAsRegularExpression", catMap.GetAt(i)->ac_regexpeval);
		*/
		ini.WriteBool(L"downloadInAlphabeticalOrder", catMap.GetAt(i)->downloadInAlphabeticalOrder!=FALSE);
		// removed care4all
		// khaos::categorymod+ Save View Filters
		ini.WriteInt(L"vfFromCats", catMap.GetAt(i)->viewfilters.nFromCats);
		ini.WriteBool(L"vfVideo", catMap.GetAt(i)->viewfilters.bVideo!=FALSE);
		ini.WriteBool(L"vfAudio", catMap.GetAt(i)->viewfilters.bAudio!=FALSE);
		ini.WriteBool(L"vfArchives", catMap.GetAt(i)->viewfilters.bArchives!=FALSE);
		ini.WriteBool(L"vfImages", catMap.GetAt(i)->viewfilters.bImages!=FALSE);
		ini.WriteBool(L"vfWaiting", catMap.GetAt(i)->viewfilters.bWaiting!=FALSE);
		ini.WriteBool(L"vfTransferring", catMap.GetAt(i)->viewfilters.bTransferring!=FALSE);
		ini.WriteBool(L"vfPaused", catMap.GetAt(i)->viewfilters.bPaused!=FALSE);
		ini.WriteBool(L"vfStopped", catMap.GetAt(i)->viewfilters.bStopped!=FALSE);
		ini.WriteBool(L"vfComplete", catMap.GetAt(i)->viewfilters.bComplete!=FALSE);
		ini.WriteBool(L"vfHashing", catMap.GetAt(i)->viewfilters.bHashing!=FALSE);
		ini.WriteBool(L"vfErrorUnknown", catMap.GetAt(i)->viewfilters.bErrorUnknown!=FALSE);
		ini.WriteBool(L"vfCompleting", catMap.GetAt(i)->viewfilters.bCompleting!=FALSE);
		ini.WriteBool(L"vfSeenComplet", catMap.GetAt(i)->viewfilters.bSeenComplet!=FALSE); //MORPH - Added by SiRoB, Seen Complet filter
		ini.WriteUInt64(L"vfFSizeMin", catMap.GetAt(i)->viewfilters.nFSizeMin);
		ini.WriteUInt64(L"vfFSizeMax", catMap.GetAt(i)->viewfilters.nFSizeMax);
		ini.WriteUInt64(L"vfRSizeMin", catMap.GetAt(i)->viewfilters.nRSizeMin);
		ini.WriteUInt64(L"vfRSizeMax", catMap.GetAt(i)->viewfilters.nRSizeMax);
		ini.WriteInt(L"vfTimeRemainingMin", catMap.GetAt(i)->viewfilters.nTimeRemainingMin);
		ini.WriteInt(L"vfTimeRemainingMax", catMap.GetAt(i)->viewfilters.nTimeRemainingMax);
		ini.WriteInt(L"vfSourceCountMin", catMap.GetAt(i)->viewfilters.nSourceCountMin);
		ini.WriteInt(L"vfSourceCountMax", catMap.GetAt(i)->viewfilters.nSourceCountMax);
		ini.WriteInt(L"vfAvailSourceCountMin", catMap.GetAt(i)->viewfilters.nAvailSourceCountMin);
		ini.WriteInt(L"vfAvailSourceCountMax", catMap.GetAt(i)->viewfilters.nAvailSourceCountMax);
		ini.WriteString(L"vfAdvancedFilterMask", catMap.GetAt(i)->viewfilters.sAdvancedFilterMask);
		// Save Selection Criteria
		ini.WriteBool(L"scFileSize", catMap.GetAt(i)->selectioncriteria.bFileSize!=FALSE);
		ini.WriteBool(L"scAdvancedFilterMask", catMap.GetAt(i)->selectioncriteria.bAdvancedFilterMask!=FALSE);
		// khaos::categorymod-
		ini.WriteBool(L"ResumeFileOnlyInSameCat", catMap.GetAt(i)->bResumeFileOnlyInSameCat!=FALSE); //MORPH - Added by SiRoB, Resume file only in the same category
	}
}
// khaos::categorymod+
void CPreferences::LoadCats()
{
	CString strCatIniFilePath;
	strCatIniFilePath.Format(L"%sCategory.ini", GetMuleDirectory(EMULE_CONFIGDIR));
	CIni ini(strCatIniFilePath);
	
	bool bCreateDefault = false;
	bool bSkipLoad = false;
	if (!PathFileExists(strCatIniFilePath))
	{
		bCreateDefault = true;
		bSkipLoad = true;
	}
	else
	{
		//ini.SetFileName(strCatIniFilePath);
		if (ini.GetInt(L"CategoryVersion", 0, L"General") == 0)
			bCreateDefault = true;
	}

	if (bCreateDefault)
	{
		Category_Struct* defcat=new Category_Struct;

		defcat->strTitle=L"Default";
    	defcat->prio=PR_NORMAL; // ZZ:DownloadManager
		defcat->iAdvA4AFMode = 0;
		defcat->strIncomingPath = GetMuleDirectory(EMULE_INCOMINGDIR);
		defcat->strComment = L"The default category.  It can't be merged or deleted.";
		defcat->color = 0;
		defcat->viewfilters.bArchives = true;
		defcat->viewfilters.bAudio = true;
		defcat->viewfilters.bComplete = true;
		defcat->viewfilters.bCompleting = true;
		defcat->viewfilters.bSeenComplet = true; //MORPH - Added by SiRoB, Seen Complet filter
		defcat->viewfilters.bErrorUnknown = true;
		defcat->viewfilters.bHashing = true;
		defcat->viewfilters.bImages = true;
		defcat->viewfilters.bPaused = true;
		defcat->viewfilters.bStopped = true;
		defcat->viewfilters.bSuspendFilters = false;
		defcat->viewfilters.bTransferring = true;
		defcat->viewfilters.bVideo = true;
		defcat->viewfilters.bWaiting = true;
		defcat->viewfilters.nAvailSourceCountMax = 0;
		defcat->viewfilters.nAvailSourceCountMin = 0;
		defcat->viewfilters.nFromCats = 2;
		defcat->viewfilters.nFSizeMax = 0;
		defcat->viewfilters.nFSizeMin = 0;
		defcat->viewfilters.nRSizeMax = 0;
		defcat->viewfilters.nRSizeMin = 0;
		defcat->viewfilters.nSourceCountMax = 0;
		defcat->viewfilters.nSourceCountMin = 0;
		defcat->viewfilters.nTimeRemainingMax = 0;
		defcat->viewfilters.nTimeRemainingMin = 0;
		defcat->viewfilters.sAdvancedFilterMask = "";
		defcat->selectioncriteria.bAdvancedFilterMask = true;
		defcat->selectioncriteria.bFileSize = true;
		defcat->bResumeFileOnlyInSameCat = false; //MORPH - Added by SiRoB, Resume file only in the same category
		AddCat(defcat);
		if (bSkipLoad)
		{
			SaveCats();
			return;
		}
	}

	int iNumCategories = ini.GetInt(L"Count", 0, L"General");
	for (int i = bCreateDefault ? 1 : 0; i <= iNumCategories; i++)
	{
		CString strSection;
		strSection.Format(L"Cat#%i", i);
		ini.SetSection(strSection);

		Category_Struct* newcat = new Category_Struct;
		newcat->strTitle = ini.GetStringUTF8(L"Title");
		newcat->strIncomingPath = ini.GetStringUTF8(L"Incoming");
		MakeFoldername(newcat->strIncomingPath);
		// SLUGFILLER: SafeHash remove - removed installation dir unsharing
		newcat->strComment = ini.GetStringUTF8(L"Comment");
		newcat->prio =ini.GetInt(L"a4afPriority",PR_NORMAL); // ZZ:DownloadManager
		/* Stulle
		newcat->filter = ini.GetInt(L"Filter", 0);
		newcat->filterNeg = ini.GetBool(L"FilterNegator", FALSE);
		newcat->ac_regexpeval = ini.GetBool(L"AutoCatAsRegularExpression", FALSE);
		newcat->care4all = ini.GetBool(L"Care4All", FALSE);
		newcat->regexp = ini.GetStringUTF8(L"RegularExpression";
		newcat->autocat = ini.GetStringUTF8(L"Autocat";
		*/
		newcat->downloadInAlphabeticalOrder = ini.GetBool(L"downloadInAlphabeticalOrder", FALSE); // ZZ:DownloadManager
		newcat->color = ini.GetInt(L"Color", (DWORD)-1 );

		// khaos::kmod+ Category Advanced A4AF Mode
		newcat->iAdvA4AFMode = ini.GetInt(L"AdvancedA4AFMode", 0);
		// khaos::kmod-
		// Load View Filters
		newcat->viewfilters.nFromCats = ini.GetInt(L"vfFromCats", i==0?0:2);
		newcat->viewfilters.bSuspendFilters = false;
		newcat->viewfilters.bVideo = ini.GetBool(L"vfVideo", true);
		newcat->viewfilters.bAudio = ini.GetBool(L"vfAudio", true);
		newcat->viewfilters.bArchives = ini.GetBool(L"vfArchives", true);
		newcat->viewfilters.bImages = ini.GetBool(L"vfImages", true);
		newcat->viewfilters.bWaiting = ini.GetBool(L"vfWaiting", true);
		newcat->viewfilters.bTransferring = ini.GetBool(L"vfTransferring", true);
		newcat->viewfilters.bPaused = ini.GetBool(L"vfPaused", true);
		newcat->viewfilters.bStopped = ini.GetBool(L"vfStopped", true);
		newcat->viewfilters.bComplete = ini.GetBool(L"vfComplete", true);
		newcat->viewfilters.bHashing = ini.GetBool(L"vfHashing", true);
		newcat->viewfilters.bErrorUnknown = ini.GetBool(L"vfErrorUnknown", true);
		newcat->viewfilters.bCompleting = ini.GetBool(L"vfCompleting", true);
		newcat->viewfilters.bSeenComplet = ini.GetBool(L"vfSeenComplet", true); //MORPH - Added by SiRoB, Seen Complet filter
		newcat->viewfilters.nFSizeMin = ini.GetInt(L"vfFSizeMin", 0);
		newcat->viewfilters.nFSizeMax = ini.GetInt(L"vfFSizeMax", 0);
		newcat->viewfilters.nRSizeMin = ini.GetInt(L"vfRSizeMin", 0);
		newcat->viewfilters.nRSizeMax = ini.GetInt(L"vfRSizeMax", 0);
		newcat->viewfilters.nTimeRemainingMin = ini.GetInt(L"vfTimeRemainingMin", 0);
		newcat->viewfilters.nTimeRemainingMax = ini.GetInt(L"vfTimeRemainingMax", 0);
		newcat->viewfilters.nSourceCountMin = ini.GetInt(L"vfSourceCountMin", 0);
		newcat->viewfilters.nSourceCountMax = ini.GetInt(L"vfSourceCountMax", 0);
		newcat->viewfilters.nAvailSourceCountMin = ini.GetInt(L"vfAvailSourceCountMin", 0);
		newcat->viewfilters.nAvailSourceCountMax = ini.GetInt(L"vfAvailSourceCountMax", 0);
		newcat->viewfilters.sAdvancedFilterMask = ini.GetString(L"vfAdvancedFilterMask", L"");
		// Load Selection Criteria
		newcat->selectioncriteria.bFileSize = ini.GetBool(L"scFileSize", true);
		newcat->selectioncriteria.bAdvancedFilterMask = ini.GetBool(L"scAdvancedFilterMask", true);
		newcat->bResumeFileOnlyInSameCat = ini.GetBool(L"ResumeFileOnlyInSameCat", false); //MORPH - Added by SiRoB, Resume file only in the same category

		AddCat(newcat);
		if (!PathFileExists(newcat->strIncomingPath))
			//MORPH START: test if directory was succesfuly create else yell in log
			if (::CreateDirectory(newcat->strIncomingPath, 0)!= 0 ){
				newcat->strIncomingPath = GetMuleDirectory(EMULE_INCOMINGDIR); // MORPH
			    theApp.QueueLogLine(true,L"incoming directory  %s of category %s not found " , newcat->strIncomingPath, newcat->strTitle );  // Note that  queue need to be used because logwindow is not initialized (logged time is wrong)
			}
			//MORPH	END
	}
}
// khaos::categorymod-

void CPreferences::RemoveCat(int index)
{
	if (index >= 0 && index < catMap.GetCount())
	{
		Category_Struct* delcat = catMap.GetAt(index); 
		catMap.RemoveAt(index);
		delete delcat;
	}
}

/*
bool CPreferences::SetCatFilter(int index, int filter)
{
	if (index >= 0 && index < catMap.GetCount())
	{
		catMap.GetAt(index)->filter = filter;
		return true;
	}
	return false;
}

int CPreferences::GetCatFilter(int index)
{
	if (index >= 0 && index < catMap.GetCount())
		return catMap.GetAt(index)->filter;
    return 0;
}

bool CPreferences::GetCatFilterNeg(int index)
{
	if (index >= 0 && index < catMap.GetCount())
		return catMap.GetAt(index)->filterNeg;
    return false;
}

void CPreferences::SetCatFilterNeg(int index, bool val)
{
	if (index >= 0 && index < catMap.GetCount())
		catMap.GetAt(index)->filterNeg = val;
}
*/

bool CPreferences::MoveCat(UINT from, UINT to)
{
	if (from >= (UINT)catMap.GetCount() || to >= (UINT)catMap.GetCount() + 1 || from == to)
		return false;

	Category_Struct* tomove = catMap.GetAt(from);
	if (from < to) {
		catMap.RemoveAt(from);
		catMap.InsertAt(to - 1, tomove);
	} else {
		catMap.InsertAt(to, tomove);
		catMap.RemoveAt(from + 1);
	}
	SaveCats();
	return true;
}


DWORD CPreferences::GetCatColor(int index, int nDefault) {
	if (index>=0 && index<catMap.GetCount()) {
		DWORD c=catMap.GetAt(index)->color;
		if (c!=(DWORD)-1)
			return catMap.GetAt(index)->color; 
	}

	return GetSysColor(nDefault);
}


///////////////////////////////////////////////////////

// SLUGFILLER: SafeHash remove - global form of IsTempFile unnececery
/*
bool CPreferences::IsInstallationDirectory(const CString& rstrDir)
{
	CString strFullPath;
	if (PathCanonicalize(strFullPath.GetBuffer(MAX_PATH), rstrDir))
		strFullPath.ReleaseBuffer();
	else
		strFullPath = rstrDir;
	
	// skip sharing of several special eMule folders
	if (!CompareDirectories(strFullPath, GetMuleDirectory(EMULE_EXECUTEABLEDIR)))
		return true;
	if (!CompareDirectories(strFullPath, GetMuleDirectory(EMULE_CONFIGDIR)))
		return true;
	if (!CompareDirectories(strFullPath, GetMuleDirectory(EMULE_WEBSERVERDIR)))
		return true;
	if (!CompareDirectories(strFullPath, GetMuleDirectory(EMULE_INSTLANGDIR)))
		return true;
	if (!CompareDirectories(strFullPath, GetMuleDirectory(EMULE_LOGDIR)))
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
*/
// SLUGFILLER: SafeHash remove - global form of IsTempFile unnececery

void CPreferences::UpdateLastVC()
{
	struct tm tmTemp;
	versioncheckLastAutomatic = safe_mktime(CTime::GetCurrentTime().GetLocalTm(&tmTemp));
}
//MORPH START - Added by SiRoB, New Version check
void CPreferences::UpdateLastMVC()
{
	struct tm tmTemp;
	mversioncheckLastAutomatic = safe_mktime(CTime::GetCurrentTime().GetLocalTm(&tmTemp));
}
//MORPH END   - Added by SiRoB, New Version check

void CPreferences::SetWSPass(CString strNewPass)
{
	m_strWebPassword = MD5Sum(strNewPass).GetHash();
}

void CPreferences::SetWSLowPass(CString strNewPass)
{
	m_strWebLowPassword = MD5Sum(strNewPass).GetHash();
}

void CPreferences::SetMMPass(CString strNewPass)
{
	m_strMMPassword = MD5Sum(strNewPass).GetHash();
}

void CPreferences::SetMaxUpload(UINT in)
{
	UINT  oldMaxUpload = in; //MORPH uint16 is not enough
	maxupload = (oldMaxUpload) ? oldMaxUpload : UNLIMITED; //MORPH uint16 is not enough
}

void CPreferences::SetMaxDownload(UINT in)
{
	uint16 oldMaxDownload = (uint16)in;
	maxdownload = (oldMaxDownload) ? oldMaxDownload : (uint16)UNLIMITED;
}

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
	return (GetWindowsVersion() == _WINVER_XP_ || GetWindowsVersion() == _WINVER_2K_ || GetWindowsVersion() == _WINVER_2003_) 
		&& m_bRunAsUser
		&& m_nCurrentUserDirMode == 2;
}

bool CPreferences::GetUseReBarToolbar()
{
	return GetReBarToolbar() && theApp.m_ullComCtrlVer >= MAKEDLLVERULL(5,8,0,0);
}

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

bool CPreferences::CanFSHandleLargeFiles(int nForCat)	{
	bool bResult = false;
	for (int i = 0; i != tempdir.GetCount(); i++){
		if (!IsFileOnFATVolume(tempdir.GetAt(i))){
			bResult = true;
			break;
		}
	}
	return bResult && !IsFileOnFATVolume((nForCat > 0) ? GetCatPath(nForCat) : GetMuleDirectory(EMULE_INCOMINGDIR));
}

uint16 CPreferences::GetRandomTCPPort()
{
	// Get table of currently used TCP ports.
	PMIB_TCPTABLE pTCPTab = NULL;
	// Couple of crash dmp files are showing that we may crash somewhere in 'iphlpapi.dll' when doing the 2nd call
	__try {
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
	}
	__except(EXCEPTION_EXECUTE_HANDLER) {
		free(pTCPTab);
		pTCPTab = NULL;
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
	// Couple of crash dmp files are showing that we may crash somewhere in 'iphlpapi.dll' when doing the 2nd call
	__try {
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
	}
	__except(EXCEPTION_EXECUTE_HANDLER) {
		free(pUDPTab);
		pUDPTab = NULL;
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

// General behavior:
//
// WinVer < Vista
// Default: ApplicationDir if preference.ini exists there. If not: user specific dirs if preferences.ini exits there. If not: again ApplicationDir
// Default overwritten by Registry value (see below)
// Fallback: ApplicationDir
//
// WinVer >= Vista:
// Default: User specific Dir if preferences.ini exists there. If not: All users dir, if preferences.ini exists there. If not user specific dirs again
// Default overwritten by Registry value (see below)
// Fallback: ApplicationDir
CString CPreferences::GetDefaultDirectory(EDefaultDirectory eDirectory, bool bCreate){
	
	//MORPH START - Added by Stulle, Copy files from bin package config to used config dir
	bool bMoveConfigFiles = false;
	CString strProgDirConfigPath;
	//MORPH END   - Added by Stulle, Copy files from bin package config to used config dir
	if (m_astrDefaultDirs[0].IsEmpty()){ // already have all directories fetched and stored?	
		
		// Get out exectuable starting directory which was our default till Vista
		TCHAR tchBuffer[MAX_PATH];
		::GetModuleFileName(NULL, tchBuffer, _countof(tchBuffer));
		tchBuffer[_countof(tchBuffer) - 1] = _T('\0');
		LPTSTR pszFileName = _tcsrchr(tchBuffer, L'\\') + 1;
		*pszFileName = L'\0';
		m_astrDefaultDirs[EMULE_EXECUTEABLEDIR] = tchBuffer;

		// set our results to old default / fallback values
		// those 3 dirs are the base for all others
		CString strSelectedDataBaseDirectory = m_astrDefaultDirs[EMULE_EXECUTEABLEDIR];
		CString strSelectedConfigBaseDirectory = m_astrDefaultDirs[EMULE_EXECUTEABLEDIR];
		CString strSelectedExpansionBaseDirectory = m_astrDefaultDirs[EMULE_EXECUTEABLEDIR];
		m_nCurrentUserDirMode = 2; // To let us know which "mode" we are using in case we want to switch per options

		// check if preferences.ini exists already in our default / fallback dir
		CFileFind ff;
		bool bConfigAvailableExecuteable = ff.FindFile(strSelectedConfigBaseDirectory + CONFIGFOLDER + _T("preferences.ini"), 0) != 0;
		ff.Close();

		strProgDirConfigPath = strSelectedConfigBaseDirectory + CONFIGFOLDER; //MORPH - Added by Stulle, Copy files from bin package config to used config dir
		
		// check if our registry setting is present which forces the single or multiuser directories
		// and lets us ignore other defaults
		// 0 = Multiuser, 1 = Publicuser, 2 = ExecuteableDir. (on Winver < Vista 1 has the same effect as 2)
		DWORD nRegistrySetting = (DWORD)-1;
		CRegKey rkEMuleRegKey;
		if (rkEMuleRegKey.Open(HKEY_CURRENT_USER, _T("Software\\eMule"), KEY_READ) == ERROR_SUCCESS){
			rkEMuleRegKey.QueryDWORDValue(_T("UsePublicUserDirectories"), nRegistrySetting);
			rkEMuleRegKey.Close();
		}
		if (nRegistrySetting != -1 && nRegistrySetting != 0 && nRegistrySetting != 1 && nRegistrySetting != 2)
			nRegistrySetting = (DWORD)-1;

		// Do we need to get SystemFolders or do we use our old Default anyway? (Executable Dir)
		if (   nRegistrySetting == 0
			|| (nRegistrySetting == 1 && GetWindowsVersion() >= _WINVER_VISTA_)
			|| (nRegistrySetting == -1 && (!bConfigAvailableExecuteable || GetWindowsVersion() >= _WINVER_VISTA_)))
		{
			HMODULE hShell32 = LoadLibrary(_T("shell32.dll"));
			if (hShell32){
				if (GetWindowsVersion() >= _WINVER_VISTA_){
					
					PWSTR pszLocalAppData = NULL;
					PWSTR pszPersonalDownloads = NULL;
					PWSTR pszPublicDownloads = NULL;
					PWSTR pszProgrammData = NULL;

					// function not available on < WinVista
					HRESULT (WINAPI *pfnSHGetKnownFolderPath)(REFKNOWNFOLDERID, DWORD, HANDLE, PWSTR*);
					(FARPROC&)pfnSHGetKnownFolderPath = GetProcAddress(hShell32, "SHGetKnownFolderPath");
					
					if (pfnSHGetKnownFolderPath != NULL
						&& (*pfnSHGetKnownFolderPath)(FOLDERID_LocalAppData, 0, NULL, &pszLocalAppData) == S_OK
						&& (*pfnSHGetKnownFolderPath)(FOLDERID_Downloads, 0, NULL, &pszPersonalDownloads) == S_OK
						&& (*pfnSHGetKnownFolderPath)(FOLDERID_PublicDownloads, 0, NULL, &pszPublicDownloads) == S_OK
						&& (*pfnSHGetKnownFolderPath)(FOLDERID_ProgramData, 0, NULL, &pszProgrammData) == S_OK)
					{
						if (_tcsclen(pszLocalAppData) < MAX_PATH - 30 && _tcsclen(pszPersonalDownloads) < MAX_PATH - 40
							&& _tcsclen(pszProgrammData) < MAX_PATH - 30 && _tcsclen(pszPublicDownloads) < MAX_PATH - 40)
						{
							CString strLocalAppData  = pszLocalAppData;
							CString strPersonalDownloads = pszPersonalDownloads;
							CString strPublicDownloads = pszPublicDownloads;
							CString strProgrammData = pszProgrammData;
							if (strLocalAppData.Right(1) != _T("\\"))
								strLocalAppData += _T("\\");
							if (strPersonalDownloads.Right(1) != _T("\\"))
								strPersonalDownloads += _T("\\");
							if (strPublicDownloads.Right(1) != _T("\\"))
								strPublicDownloads += _T("\\");
							if (strProgrammData.Right(1) != _T("\\"))
								strProgrammData += _T("\\");

							if (nRegistrySetting == -1){
								// no registry default, check if we find a preferences.ini to use
								bool bRes =  ff.FindFile(strLocalAppData + _T("eMule\\") + CONFIGFOLDER + _T("preferences.ini"), 0) != 0;
								ff.Close();
								if (bRes)
									m_nCurrentUserDirMode = 0;
								else{
									bRes =  ff.FindFile(strProgrammData + _T("eMule\\") + CONFIGFOLDER + _T("preferences.ini"), 0) != 0;
									ff.Close();
									if (bRes)
										m_nCurrentUserDirMode = 1;
									else if (bConfigAvailableExecuteable)
										m_nCurrentUserDirMode = 2;
									else
									{ //MORPH - Added by Stulle, Copy files from bin package config to used config dir
										m_nCurrentUserDirMode = 0; // no preferences.ini found, use the default
									//MORPH START - Added by Stulle, Copy files from bin package config to used config dir
										bMoveConfigFiles = true;
									}
									//MORPH END   - Added by Stulle, Copy files from bin package config to used config dir
								}
							}
							else
								m_nCurrentUserDirMode = nRegistrySetting;
							
							if (m_nCurrentUserDirMode == 0){
								// multiuser
								strSelectedDataBaseDirectory = strPersonalDownloads + _T("eMule\\");
								strSelectedConfigBaseDirectory = strLocalAppData + _T("eMule\\");
								strSelectedExpansionBaseDirectory = strProgrammData + _T("eMule\\");
							}
							else if (m_nCurrentUserDirMode == 1){
								// public user
								strSelectedDataBaseDirectory = strPublicDownloads + _T("eMule\\");
								strSelectedConfigBaseDirectory = strProgrammData + _T("eMule\\");
								strSelectedExpansionBaseDirectory = strProgrammData + _T("eMule\\");
							}
							else if (m_nCurrentUserDirMode == 2){
								// programm directory
							}
							else
								ASSERT( false );
						}
						else
							ASSERT( false );
						}

						CoTaskMemFree(pszLocalAppData);
						CoTaskMemFree(pszPersonalDownloads);
						CoTaskMemFree(pszPublicDownloads);
						CoTaskMemFree(pszProgrammData);
				}
				else { // GetWindowsVersion() >= _WINVER_VISTA_

					CString strAppData = ShellGetFolderPath(CSIDL_APPDATA);
					CString strPersonal = ShellGetFolderPath(CSIDL_PERSONAL);
					if (!strAppData.IsEmpty() && !strPersonal.IsEmpty())
					{
						if (strAppData.GetLength() < MAX_PATH - 30 && strPersonal.GetLength() < MAX_PATH - 40){
							if (strPersonal.Right(1) != _T("\\"))
								strPersonal += _T("\\");
							if (strAppData.Right(1) != _T("\\"))
								strAppData += _T("\\");
							if (nRegistrySetting == 0){
								// registry setting overwrites, use these folders
								strSelectedDataBaseDirectory = strPersonal + _T("eMule Downloads\\");
								strSelectedConfigBaseDirectory = strAppData + _T("eMule\\");
								m_nCurrentUserDirMode = 0;
								// strSelectedExpansionBaseDirectory stays default
							}
							else if (nRegistrySetting == -1 && !bConfigAvailableExecuteable){
								if (ff.FindFile(strAppData + _T("eMule\\") + CONFIGFOLDER + _T("preferences.ini"), 0)){
									// preferences.ini found, so we use this as default
									strSelectedDataBaseDirectory = strPersonal + _T("eMule Downloads\\");
									strSelectedConfigBaseDirectory = strAppData + _T("eMule\\");
									m_nCurrentUserDirMode = 0;
									bMoveConfigFiles = true; //MORPH - Added by Stulle, Copy files from bin package config to used config dir
								}
								ff.Close();
							}
							else
								ASSERT( false );
						}
						else
							ASSERT( false );
					}
				}
				FreeLibrary(hShell32);
			}
			else{
				DebugLogError(_T("Unable to load shell32.dll to retrieve the systemfolder locations, using fallbacks"));
				ASSERT( false );
			}
		}

		// the use of ending backslashes is inconsitent, would need a rework throughout the code to fix this
		m_astrDefaultDirs[EMULE_CONFIGDIR] = strSelectedConfigBaseDirectory + CONFIGFOLDER;
		m_astrDefaultDirs[EMULE_TEMPDIR] = strSelectedDataBaseDirectory + _T("Temp");
		m_astrDefaultDirs[EMULE_INCOMINGDIR] = strSelectedDataBaseDirectory + _T("Incoming");
		m_astrDefaultDirs[EMULE_LOGDIR] = strSelectedConfigBaseDirectory + _T("logs\\");
		m_astrDefaultDirs[EMULE_ADDLANGDIR] = strSelectedExpansionBaseDirectory + _T("lang\\");
		m_astrDefaultDirs[EMULE_INSTLANGDIR] = m_astrDefaultDirs[EMULE_EXECUTEABLEDIR] + _T("lang\\");
		m_astrDefaultDirs[EMULE_WEBSERVERDIR] = m_astrDefaultDirs[EMULE_EXECUTEABLEDIR] + _T("webserver\\");
		m_astrDefaultDirs[EMULE_SKINDIR] = strSelectedExpansionBaseDirectory + _T("skins");
		m_astrDefaultDirs[EMULE_DATABASEDIR] = strSelectedDataBaseDirectory; // has ending backslashes
		m_astrDefaultDirs[EMULE_CONFIGBASEDIR] = strSelectedConfigBaseDirectory; // has ending backslashes
		//                EMULE_EXECUTEABLEDIR
		m_astrDefaultDirs[EMULE_TOOLBARDIR] = strSelectedExpansionBaseDirectory + _T("skins");
		m_astrDefaultDirs[EMULE_EXPANSIONDIR] = strSelectedExpansionBaseDirectory; // has ending backslashes
		m_astrDefaultDirs[EMULE_WAPSERVERDIR] = m_astrDefaultDirs[EMULE_EXECUTEABLEDIR] + _T("wapserver\\");
		m_astrDefaultDirs[EMULE_FEEDSDIR] = strSelectedConfigBaseDirectory + _T("feeds\\");

		/*CString strDebug;
		for (int i = 0; i < 12; i++)
			strDebug += m_astrDefaultDirs[i] + _T("\n");
		AfxMessageBox(strDebug, MB_ICONINFORMATION);*/
	}
	if (bCreate && !m_abDefaultDirsCreated[eDirectory]){
		switch (eDirectory){ // create the underlying directory first - be sure to adjust this if changing default directories
			case EMULE_CONFIGDIR:
			case EMULE_LOGDIR:
				::CreateDirectory(m_astrDefaultDirs[EMULE_CONFIGBASEDIR], NULL);
				break;
			case EMULE_TEMPDIR:
			case EMULE_INCOMINGDIR:
				::CreateDirectory(m_astrDefaultDirs[EMULE_DATABASEDIR], NULL);
				break;
			case EMULE_ADDLANGDIR:
			case EMULE_SKINDIR:
			case EMULE_TOOLBARDIR:
				::CreateDirectory(m_astrDefaultDirs[EMULE_EXPANSIONDIR], NULL);
				break;
		}
		::CreateDirectory(m_astrDefaultDirs[eDirectory], NULL);
		m_abDefaultDirsCreated[eDirectory] = true;
	}
	//MORPH START - Added by Stulle, Copy files from bin package config to used config dir
	if (bMoveConfigFiles)
	{
		//NOTE: Can't log right here and making a popup here would be really annoying!
		CopyFile(strProgDirConfigPath + _T("countryflag.dll"), m_astrDefaultDirs[EMULE_CONFIGDIR] + _T("countryflag.dll"),TRUE);
		CopyFile(strProgDirConfigPath + _T("countryflag32.dll"), m_astrDefaultDirs[EMULE_CONFIGDIR] + _T("countryflag32.dll"),TRUE);
		CopyFile(strProgDirConfigPath + _T("eMule Light.tmpl"), m_astrDefaultDirs[EMULE_CONFIGDIR] + _T("eMule Light.tmpl"),TRUE);
		CopyFile(strProgDirConfigPath + _T("eMule.tmpl"), m_astrDefaultDirs[EMULE_CONFIGDIR] + _T("eMule.tmpl"),TRUE);
		CopyFile(strProgDirConfigPath + _T("Multiuser eMule.tmpl"), m_astrDefaultDirs[EMULE_CONFIGDIR] + _T("Multiuser eMule.tmpl"),TRUE);
		CopyFile(strProgDirConfigPath + _T("nodes.dat"), m_astrDefaultDirs[EMULE_CONFIGDIR] + _T("nodes.dat"),TRUE);
		CopyFile(strProgDirConfigPath + _T("server.met"), m_astrDefaultDirs[EMULE_CONFIGDIR] + _T("server.met"),TRUE);
		CopyFile(strProgDirConfigPath + _T("startup.wav"), m_astrDefaultDirs[EMULE_CONFIGDIR] + _T("startup.wav"),TRUE);
		CopyFile(strProgDirConfigPath + _T("webservices.dat"), m_astrDefaultDirs[EMULE_CONFIGDIR] + _T("webservices.dat"),TRUE);
		CopyFile(strProgDirConfigPath + _T("XMLNews.dat"), m_astrDefaultDirs[EMULE_CONFIGDIR] + _T("XMLNews.dat"),TRUE);
	}
	//MORPH END   - Added by Stulle, Copy files from bin package config to used config dir
	return m_astrDefaultDirs[eDirectory];
}

CString	CPreferences::GetMuleDirectory(EDefaultDirectory eDirectory, bool bCreate){
	switch (eDirectory){
		case EMULE_INCOMINGDIR:
			return m_strIncomingDir;
		case EMULE_TEMPDIR:
			ASSERT( false ); // use GetTempDir() instead! This function can only return the first tempdirectory
			return GetTempDir(0);
		case EMULE_SKINDIR:
			return m_strSkinProfileDir;
		case EMULE_TOOLBARDIR:
			return m_sToolbarBitmapFolder;
		default:
			return GetDefaultDirectory(eDirectory, bCreate);
	}
}

void CPreferences::SetMuleDirectory(EDefaultDirectory eDirectory, CString strNewDir){
	switch (eDirectory){
		case EMULE_INCOMINGDIR:
			m_strIncomingDir = strNewDir;
			break;
		case EMULE_SKINDIR:
			m_strSkinProfileDir = strNewDir;
			break;
		case EMULE_TOOLBARDIR:
			m_sToolbarBitmapFolder = strNewDir;
			break;
		default:
			ASSERT( false );
	}
}

void CPreferences::ChangeUserDirMode(int nNewMode){
	if (m_nCurrentUserDirMode == nNewMode)
		return;
	if (nNewMode == 1 && GetWindowsVersion() < _WINVER_VISTA_)
	{
		ASSERT( false );
		return;
	}
	// check if our registry setting is present which forces the single or multiuser directories
	// and lets us ignore other defaults
	// 0 = Multiuser, 1 = Publicuser, 2 = ExecuteableDir.
	CRegKey rkEMuleRegKey;
	if (rkEMuleRegKey.Create(HKEY_CURRENT_USER, _T("Software\\eMule")) == ERROR_SUCCESS){
		if (rkEMuleRegKey.SetDWORDValue(_T("UsePublicUserDirectories"), nNewMode) != ERROR_SUCCESS)
			DebugLogError(_T("Failed to write registry key to switch UserDirMode"));
		else
			m_nCurrentUserDirMode = nNewMode;
		rkEMuleRegKey.Close();
	}
}

bool CPreferences::GetSparsePartFiles()	{
	// Vistas Sparse File implemenation seems to be buggy as far as i can see
	// If a sparsefile exceeds a given limit of write io operations in a certain order (or i.e. end to beginning)
	// in its lifetime, it will at some point throw out a FILE_SYSTEM_LIMITATION error and deny any writing
	// to this file.
	// It was suggested that Vista might limits the dataruns, which would lead to such a behavior, but wouldn't
	// make much sense for a sparse file implementation nevertheless.
	// Due to the fact that eMule wirtes a lot small blocks into sparse files and flushs them every 6 seconds,
	// this problem pops up sooner or later for all big files. I don't see any way to walk arround this for now
	// Update: This problem seems to be fixed on Win7, possibly on earlier Vista ServicePacks too
	//		   In any case, we allow sparse files for vesions earlier and later than Vista
	return m_bSparsePartFiles && (GetWindowsVersion() != _WINVER_VISTA_);
}

bool CPreferences::IsRunningAeroGlassTheme(){
	// This is important for all functions which need to draw in the NC-Area (glass style)
	// Aero by default does not allow this, any drawing will not be visible. This can be turned off,
	// but Vista will not deliver the Glass style then as background when calling the default draw function
	// in other words, its draw all or nothing yourself - eMule chooses currently nothing
	static bool bAeroAlreadyDetected = false;
	if (!bAeroAlreadyDetected){
		bAeroAlreadyDetected = true;
		m_bIsRunningAeroGlass = FALSE;
		if (GetWindowsVersion() >= _WINVER_VISTA_){
			HMODULE hDWMAPI = LoadLibrary(_T("dwmapi.dll"));
			if (hDWMAPI){
				HRESULT (WINAPI *pfnDwmIsCompositionEnabled)(BOOL*);
				(FARPROC&)pfnDwmIsCompositionEnabled = GetProcAddress(hDWMAPI, "DwmIsCompositionEnabled");
				if (pfnDwmIsCompositionEnabled != NULL)
					pfnDwmIsCompositionEnabled(&m_bIsRunningAeroGlass);
				FreeLibrary(hDWMAPI);
			}
		}
	}
	return m_bIsRunningAeroGlass == TRUE ? true : false;
}

//MORPH START - Added by IceCream, high process priority
void CPreferences::SetEnableHighProcess(bool enablehigh) 			
{
	enableHighProcess = enablehigh;
	if (enablehigh)
		SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	else
		SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
}
//MORPH END   - Added by IceCream, high process priority

//MORPH START - Added & Modified by SiRoB, Smart Upload Control v2 (SUC) [lovelace]
bool	CPreferences::IsSUCDoesWork()
{
	return minupload<maxupload && maxupload != UNLIMITED && m_bSUCEnabled;
}
//MORPH END  - Added & Modified by SiRoB, Smart Upload Control v2 (SUC) [lovelace]

//MORPH START - Added by SiRoB, (SUC) & (USS)
void	CPreferences::SetMinUpload(UINT in)
{
	minupload = (uint16)((in) ? in : UNLIMITED);
}
//MORPH END  - Added by SiRoB, (SUC) & (USS)

//Commander - Added: Invisible Mode [TPT] - Start
void CPreferences::SetInvisibleMode(bool on, UINT keymodifier, char key) 
{
	m_bInvisibleMode = on;
	m_iInvisibleModeHotKeyModifier = keymodifier;
	m_cInvisibleModeHotKey = key;
	if(theApp.emuledlg!=NULL){
		//Always unregister, the keys could be different.
		theApp.emuledlg->UnRegisterInvisibleHotKey();
		if(m_bInvisibleMode)	theApp.emuledlg->RegisterInvisibleHotKey();
	}
}
//Commander - Added: Invisible Mode [TPT] - End

//MORPH START - Added by SiRoB, [MoNKi: -UPnPNAT Support-]
// emulEspaña: Modified by MoNKi [MoNKi: -Random Ports-]
/*
uint16	CPreferences::GetPort(){
*/
uint16	CPreferences::GetPort(bool newPort, bool original, bool reset){
// End -Random Ports-

	// emulEspaña: Added by MoNKi [MoNKi: -Random Ports-]
	if(original)
		return port;

	if(reset){
		m_iCurrentTCPRndPort = 0;
	}
// End -Random Ports-

	// emulEspaña: Added by MoNKi [MoNKi: -Random Ports-]
	if (m_iCurrentTCPRndPort == 0 || newPort){
		if(GetUseRandomPorts())
			do{
				m_iCurrentTCPRndPort = GetMinRandomPort() + (uint16)((((float)rand() / RAND_MAX) * (GetMaxRandomPort() - GetMinRandomPort())));
			}while(m_iCurrentTCPRndPort==GetUDPPort() && ((GetMaxRandomPort() - GetMinRandomPort())>0));
		else
			m_iCurrentTCPRndPort = port;
	}
	return m_iCurrentTCPRndPort;
	// End -Random Ports-
}

// emulEspaña: Modified by MoNKi [MoNKi: -Random Ports-]
/*
uint16	CPreferences::GetUDPPort(){
*/
uint16	CPreferences::GetUDPPort(bool newPort, bool original, bool reset){
// End -Random Ports-

	// emulEspaña: Added by MoNKi [MoNKi: -Random Ports-]
	if(original)
		return udpport;

	if(reset){
		m_iCurrentUDPRndPort = 0;
	}
// End -Random Ports-

	if(udpport == 0)
		return 0;
	
	// emulEspaña: Added by MoNKi [MoNKi: -Random Ports-]
	if (m_iCurrentUDPRndPort == 0 || newPort){
		if(GetUseRandomPorts())
			do{
				m_iCurrentUDPRndPort = GetMinRandomPort() + (uint16)(((float)rand() / RAND_MAX) * (GetMaxRandomPort() - GetMinRandomPort()));
			}while(m_iCurrentUDPRndPort==GetPort() && ((GetMaxRandomPort() - GetMinRandomPort())>0));
		else
			m_iCurrentUDPRndPort = udpport;
	}
	return m_iCurrentUDPRndPort;
	// End -Random Ports-
}
//MORPH END   - Added by SiRoB, [MoNKi: -UPnPNAT Support-]

//MORPH START - Added by SiRoB / Commander, Wapserver [emulEspaña]
void CPreferences::SetWapPass(CString strNewPass)
{
	m_sWapPassword = MD5Sum(strNewPass).GetHash();
}

void CPreferences::SetWapLowPass(CString strNewPass)
{
	m_sWapLowPassword = MD5Sum(strNewPass).GetHash();
}
//MORPH END - Added by SiRoB / Commander, Wapserver [emulEspaña]

 // MORPH START leuk_he bindaddr
void CPreferences::SetBindAddr(CStringW bindip)
{
	m_strBindAddrW = bindip;
	m_strBindAddrW.Trim();
	m_pszBindAddrW = m_strBindAddrW.IsEmpty() ? NULL : (LPCWSTR)m_strBindAddrW;
	m_strBindAddrA = m_strBindAddrW;
	m_pszBindAddrA = m_strBindAddrA.IsEmpty() ? NULL : (LPCSTR)m_strBindAddrA;
}
 // MORPH END leuk_he bindaddr
 // MORPH START leuk_he upnp bindaddr
void CPreferences::SetUpnpBindAddr(DWORD bindip) {
   /* wine workarround		if ( GetBindAddrA() != NULL && bindip== ntohl(inet_addr(GetBindAddrA())))
			m_dwUpnpBindAddr =0;
		else 
		*/
	    	m_dwUpnpBindAddr= bindip;
	}
	// MORPH END leuk_he upnp bindaddr
 
// MORPH leuk_he:run as ntservice v1. START (startup and ws port) 
int CPreferences::GetServiceStartupMode(){
	//MORPH START - Added by Stulle, Adjustable NT Service Strings
	if (m_strServiceName.IsEmpty()) // may be called before LoadPreferences()
		m_strServiceName = theApp.GetProfileStringW(L"StulleMule",L"ServiceName",NULL);
	//MORPH END   - Added by Stulle, Adjustable NT Service Strings
	if (m_iServiceStartupMode == 0) // may be called before LoadPreferences()
	   m_iServiceStartupMode=theApp.GetProfileInt(_T("eMule"), _T("ServiceStartupMode"),2); // default = stop service and start
   return m_iServiceStartupMode;
}
uint16	CPreferences::GetWSPort()							
{	if (m_nWebPort==0)
		m_nWebPort=(WORD) theApp.GetProfileInt(_T("WebServer"), _T("Port"),4711);
	return m_nWebPort; 
}
// MORPH leuk_he:run as ntservice v1.. (startup and ws port) 

// MORPH START show less controls
bool CPreferences::SetLessControls(bool newvalue)
{
	if (newvalue ==  m_bShowLessControls)
		return m_bShowLessControls;  // no change
	m_bShowLessControls = newvalue ; 
	theApp.emuledlg->ShowLessControls(newvalue);
	return m_bShowLessControls;  //  change
}
// MORPH END  show less controls

// SLUGFILLER: SafeHash
bool CPreferences::IsConfigFile(const CString& rstrDirectory, const CString& rstrName)
{
	if (CompareDirectories(rstrDirectory, GetMuleDirectory(EMULE_CONFIGDIR)))
		return false;

	// do not share a file from the config directory, if it contains one of the following extensions
	static const LPCTSTR _apszNotSharedExts[] = { L".met.bak", L".ini.old" };
	for (int i = 0; i < _countof(_apszNotSharedExts); i++){
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
		L"cancelled.met", //MORPH
		L"category.ini",
		L"clients.met",
		L"cryptkey.dat",
		L"emfriends.met",
		L"fileinfo.ini",
		L"ipfilter.dat",
		L"known.met",
		L"known2_64.met", //MORPH
		L"preferences.dat",
		L"preferences.ini",
		L"server.met",
		L"server.met.new",
		L"server_met.download",
		L"server_met.old",
		L"shareddir.dat",
		L"sharedsubdir.dat",
		L"sharedsubdir.ini", //MORPH
		L"staticservers.dat",
		L"StoredSearches.met", //MORPH
		L"webservices.dat"
	};
	for (int i = 0; i < _countof(_apszNotSharedFiles); i++){
		if (rstrName.CompareNoCase(_apszNotSharedFiles[i])==0)
			return true;
	}

	return false;
}
// SLUGFILLER: SafeHash

//MORPH START - Added by Stulle, New IP Filter by Ozzy [Stulle/Ozzy]
bool CPreferences::IsIPFilterViaDynDNS(CString strURL)
{
	if(strURL.IsEmpty())
		strURL = _T("http://downloads.sourceforge.net/scarangel/ipfilter.rar");

	if(StrStr(GetUpdateURLIPFilter(),strURL)!=0)
		return true;
	return false;
}
//MORPH END   - Added by Stulle, New IP Filter by Ozzy [Stulle/Ozzy]

//MORPH START - Added by Stulle, Adjustable NT Service Strings
CString CPreferences::GetServiceName()
{
	// if the strings have not been loaded yet we need to load the name directly from the .ini
	if (!m_bServiceStringsLoaded)
		m_strServiceName = theApp.GetProfileString(_T("StulleMule"), _T("ServiceName"), NULL);
	return m_strServiceName;
}
//MORPH END   - Added by Stulle, Adjustable NT Service Strings