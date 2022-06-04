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
#include "MuleToolbarCtrl.h"
#include "VistaDefines.h"

#pragma warning(disable:4516) // access-declarations are deprecated; member using-declarations provide a better alternative
#pragma warning(disable:4244) // conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable:4100) // unreferenced formal parameter
#pragma warning(disable:4702) // unreachable code
#include <crypto/osrng.h>
#pragma warning(default:4702) // unreachable code
#pragma warning(default:4100) // unreferenced formal parameter
#pragma warning(default:4244) // conversion from 'type1' to 'type2', possible loss of data
#pragma warning(default:4516) // access-declarations are deprecated; member using-declarations provide a better alternative
//Xman
#include "BandWidthControl.h" // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
#include "VolumeInfo.h" // X: [FSFS] - [FileSystemFeaturesSupport]
#include "XMessageBox.h"
#define CONFIGFOLDER			_T("config\\")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#define	MS2SEC(ms)	((ms)/1000)

CPreferences thePrefs;
//-------------------------------------------------------------------------------
//Xman Xtreme Mod:

//Xman Xtreme Upload
float	CPreferences::m_slotspeed;
bool	CPreferences::m_openmoreslots;
bool	CPreferences::m_bandwidthnotreachedslots;
//int		CPreferences::m_sendbuffersize;// X: [DSRB] - [Dynamic Send and Receive Buffer]
int		CPreferences::m_internetdownreactiontime;

//Xman always one release-slot
//bool CPreferences::m_onerealeseslot;
//Xman end

//Xman advanced upload-priority
bool CPreferences::m_AdvancedAutoPrio;
//Xman end

//Xman chunk chooser
uint8 CPreferences::m_chunkchooser;

//Xman disable compression
bool CPreferences::m_bUseCompression;

//Xman auto update IPFilter
bool CPreferences::m_bautoupdateipfilter;
CString CPreferences::m_strautoupdateipfilter_url;
SYSTEMTIME CPreferences::m_IPfilterVersion;
uint64 CPreferences::m_last_ipfilter_check;
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

uint16	CPreferences::MTU;                          // -MTU Configuration-
bool	CPreferences::usedoublesendsize;
bool	CPreferences::retrieveMTUFromSocket; // netfinity: Maximum Segment Size (MSS - Vista only) //added by zz_fly

bool	CPreferences::NAFCFullControl;	          // -Network Adapter Feedback Control-
uint32	CPreferences::forceNAFCadapter;
uint8	CPreferences::datarateSamples;              // -Accurate measure of bandwidth: eDonkey data + control, network adapter-

bool    CPreferences::enableMultiQueue;             // -One-queue-per-file- (idea bloodymad)
// Maella end

// Mighty Knife: Static server handling
bool	CPreferences::m_bDontRemoveStaticServers;
// [end] Mighty Knife


//Xman [MoNKi: -Downloaded History-]
bool	CPreferences::m_bHistoryShowShared;
//Xman end

//Xman show additional graph lines
bool	CPreferences::m_bShowAdditionalGraph;


//Xman don't overwrite bak files if last sessions crashed
//bool	CPreferences::m_this_session_aborted_in_an_unnormal_way;// X: [CI] - [Code Improvement]
bool	CPreferences::m_last_session_aborted_in_an_unnormal_way;


//Xman end
//-------------------------------------------------------------------------------

CString CPreferences::m_astrDefaultDirs[10];
bool	CPreferences::m_abDefaultDirsCreated[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int		CPreferences::m_nCurrentUserDirMode = -1;
#ifdef _DEBUG// X: [RDL] - [Remove Debug Log]
int		CPreferences::m_iDbgHeap;
#endif
//CString	CPreferences::strNick;
//uint16	CPreferences::minupload;
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
CString	CPreferences::m_strIncomingDir;
CAtlArray<CString> CPreferences::tempdir;
bool	CPreferences::ICH;
//bool	CPreferences::autoconnect;
bool	CPreferences::m_bAutoConnectToStaticServersOnly;
bool	CPreferences::autotakeed2klinks;
bool	CPreferences::addnewfilespaused;
UINT	CPreferences::depth3D;
int		CPreferences::m_iStraightWindowStyles;
bool	CPreferences::m_bUseSystemFontForMainControls;
bool	CPreferences::m_bRTLWindowsLayout;
UINT	CPreferences::maxsourceperfile;
UINT	CPreferences::trafficOMeterInterval;
UINT	CPreferences::statsInterval;
bool	CPreferences::m_bFillGraphs;
uchar	CPreferences::userhash[16];
WINDOWPLACEMENT CPreferences::EmuleWindowPlacement;
bool	CPreferences::beepOnError;
bool	CPreferences::confirmExit;
DWORD	CPreferences::m_adwStatsColors[15];
bool	CPreferences::bHasCustomTaskIconColor;
bool	CPreferences::splashscreen;
bool	CPreferences::filterLANIPs;
#ifdef _DEBUG
bool	CPreferences::m_bAllocLocalHostIP;
#endif
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
uint64	CPreferences::cumUpData[UP_SOFT_COUNT];
uint64	CPreferences::sesUpData[UP_SOFT_COUNT];
uint64	CPreferences::cumUpDataPort[PORT_COUNT];
uint64	CPreferences::sesUpDataPort[PORT_COUNT];
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
uint64	CPreferences::cumDownData[DOWN_SOFT_COUNT];
uint64	CPreferences::sesDownData[DOWN_SOFT_COUNT];
uint64	CPreferences::cumDownDataPort[PORT_COUNT];
uint64	CPreferences::sesDownDataPort[PORT_COUNT];
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
UINT	CPreferences::statsSaveInterval;
CString	CPreferences::m_strStatsExpandedTreeItems;
bool	CPreferences::m_bShowVerticalHourMarkers;
uint64	CPreferences::totalDownloadedBytes;
uint64	CPreferences::totalUploadedBytes;
WORD	CPreferences::m_wLanguageID;
EViewSharedFilesAccess CPreferences::m_iSeeShares;
UINT	CPreferences::m_iToolDelayTime;
bool	CPreferences::bringtoforeground;
UINT	CPreferences::splitterbarPosition;
UINT	CPreferences::splitterbarPositionSvr;
UINT	CPreferences::splitterbarPositionStat;
UINT	CPreferences::splitterbarPositionStat_HR;
UINT	CPreferences::m_uTransferWnd1;
UINT	CPreferences::m_uTransferWnd2;
UINT	CPreferences::m_uDeadServerRetries;
DWORD	CPreferences::m_dwServerKeepAliveTimeout;
UINT	CPreferences::statsAverageMinutes;
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
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
bool	CPreferences::m_bUseDebugDevice = true;
int		CPreferences::m_iDebugServerTCPLevel;
int		CPreferences::m_iDebugServerUDPLevel;
int		CPreferences::m_iDebugServerSourcesLevel;
int		CPreferences::m_iDebugServerSearchesLevel;
int		CPreferences::m_iDebugClientTCPLevel;
int		CPreferences::m_iDebugClientUDPLevel;
int		CPreferences::m_iDebugClientKadUDPLevel;
int		CPreferences::m_iDebugSearchResultDetailLevel;
#endif
//bool	CPreferences::m_bManualAddedServersHighPriority;
bool	CPreferences::m_btransferfullchunks;
int		CPreferences::m_istartnextfile;
bool	CPreferences::m_bDAP;
bool	CPreferences::m_bUAP;
bool	CPreferences::m_bExtControls;
bool	CPreferences::m_bTransflstRemain;
bool	CPreferences::showRatesInTitle;
CString	CPreferences::m_strTxtEditor;
CString	CPreferences::m_strVideoPlayer;
CString CPreferences::m_strVideoPlayerArgs;
bool	CPreferences::moviePreviewBackup;
int 	CPreferences::m_iPreviewSmallBlocks;

bool	CPreferences::m_bCheckFileOpen;
bool	CPreferences::watchclipboard;
bool	CPreferences::filterserverbyip;
bool	CPreferences::m_bFirstStart;
bool	CPreferences::log2disk;
bool	CPreferences::debug2disk;
int		CPreferences::iMaxLogBuff;
UINT	CPreferences::uMaxLogFileSize;
ELogFileFormat CPreferences::m_iLogFileFormat = Unicode;
UINT	CPreferences::filterlevel;
//UINT	CPreferences::m_uFileBufferSize;
UINT	CPreferences::m_uGlobalBufferSize;// X: [GB] - [Global Buffer]
UINT	CPreferences::m_iQueueSize;
int		CPreferences::m_iCommitFiles;

CString	CPreferences::filenameCleanups;
CString	CPreferences::m_strDateTimeFormat;
CString	CPreferences::m_strDateTimeFormat4Log;
CString	CPreferences::m_strDateTimeFormat4Lists;
LOGFONT CPreferences::m_lfHyperText;
LOGFONT CPreferences::m_lfLogText;
COLORREF CPreferences::m_crLogError = RGB(255, 0, 0);
COLORREF CPreferences::m_crLogWarning = RGB(128, 0, 128);
COLORREF CPreferences::m_crLogSuccess = RGB(0, 0, 255);
bool	CPreferences::m_bAdjustNTFSDaylightFileTime = true;
bool	CPreferences::m_bRearrangeKadSearchKeywords;
ProxySettings CPreferences::proxy;
bool	CPreferences::showCatTabInfos;
//bool	CPreferences::resumeSameCat;
bool	CPreferences::dontRecreateGraphs;
bool	CPreferences::autofilenamecleanup;
int		CPreferences::m_iSearchMethod;
bool	CPreferences::m_bUseSecureIdent;
//bool	CPreferences::networkkademlia;
bool	CPreferences::networked2k;
bool    CPreferences::m_bAllocFull;
//bool	CPreferences::m_bShowUpDownIconInTaskbar;
bool	CPreferences::m_bShowWin7TaskbarGoodies;
bool    CPreferences::m_bHighresTimer;
bool	CPreferences::m_bResolveSharedShellLinks;
bool	CPreferences::m_bKeepUnavailableFixedSharedDirs;
CAtlList<CString> CPreferences::shareddir_list;
CString CPreferences::m_strFileCommentsFilePath;
WORD	CPreferences::m_wWinVer;
CAtlArray<Category_Struct*> CPreferences::catMap;
UINT	CPreferences::m_nWebMirrorAlertLevel;
bool	CPreferences::m_bRunAsUser;
bool	CPreferences::m_bPreferRestrictedOverUser;
bool	CPreferences::m_bUseOldTimeRemaining;

bool	CPreferences::m_bOpenPortsOnStartUp;
int		CPreferences::m_byLogLevel;
bool	CPreferences::m_bTrustEveryHash;
bool	CPreferences::m_bRememberCancelledFiles;
bool	CPreferences::m_bRememberDownloadedFiles;
bool	CPreferences::m_bPartiallyPurgeOldKnownFiles;

bool	CPreferences::m_bWinaTransToolbar;

bool	CPreferences::m_bCryptLayerRequested;
bool	CPreferences::m_bCryptLayerSupported;
bool	CPreferences::m_bCryptLayerRequired;
uint32	CPreferences::m_dwKadUDPKey;
uint8	CPreferences::m_byCryptTCPPaddingLength;
bool	CPreferences::m_bEnableSearchResultFilter;
EFileSizeFormat	CPreferences::m_eFileSizeFormat;
bool	CPreferences::m_bSkipWANIPSetup;
bool	CPreferences::m_bSkipWANPPPSetup;
bool	CPreferences::m_bEnableUPnP;
bool	CPreferences::m_bCloseUPnPOnExit;
//bool	CPreferences::m_bIsWinServImplDisabled;
bool	CPreferences::m_bIsMinilibImplDisabled;
bool	CPreferences::m_bIsACATImplDisabled;
int		CPreferences::m_nLastWorkingImpl;
bool	CPreferences::m_bUPnPTryRandom; // Try to use random external port if already in use On/Off
bool	CPreferences::m_bUPnPRebindOnIPChange; //zz_fly :: Rebind UPnP on IP-change , On/Off

bool	CPreferences::m_bEnable64BitTime;// X: [E64T] - [Enable64BitTime]
bool	CPreferences::refuseupload;// X: [RU] - [RefuseUpload]
bool	CPreferences::m_bPauseOnFileComplete; // NEO: POFC - [PauseOnFileComplete]
bool	CPreferences::dontcompressext;// X: [DCE] - [DontCompressExt]
CString	CPreferences::compressExt;// X: [DCE] - [DontCompressExt]
uint32	CPreferences::rumax;// X: [RU] - [RefuseUpload]
bool	CPreferences::prefReadonly; // X: [ROP] - [ReadOnlyPreference]
bool	CPreferences::isshowtoolbar;
bool	CPreferences::isshowcatbar;
bool	CPreferences::m_bDisableHistoryList;
bool	CPreferences::showShareableFile;
uint8	CPreferences::queryOnHashing; // X: [QOH] - [QueryOnHashing]
uint8	CPreferences::lastTranWndCatID;// X: [RCI] - [Remember Catalog ID]
bool	CPreferences::dontsharext;// X: [DSE] - [DontShareExt]
CString	CPreferences::shareExt;// X: [DSE] - [DontShareExt]
int		CPreferences::speedgraphwindowLeft; // X: [SGW] - [SpeedGraphWnd]
int		CPreferences::speedgraphwindowTop;
bool	CPreferences::showSpeedGraph;
bool	CPreferences::randomPortOnStartup; // X: [RPOS] - [RandomPortOnStartup]
bool	CPreferences::notifier;

// ==> Invisible Mode [TPT/MoNKi] - Stulle
bool	CPreferences::m_bInvisibleMode;		
int	CPreferences::m_iInvisibleModeHotKeyModifier;
char	CPreferences::m_cInvisibleModeHotKey;
bool	CPreferences::m_bInvisibleModeStart;
// <== Invisible Mode [TPT/MoNKi] - Stulle

//morph4u + ////////////////////////////////////////////////
bool	CPreferences::m_bGridlines;
TCHAR 	CPreferences::m_strServerMetUpdateURL[256];
TCHAR	CPreferences::m_strNodesDatUpdateURL[256];
bool	CPreferences::m_bShowProgressBar; //morph4u :: PercentBar 
bool	CPreferences::m_bShutDownMule;
bool	CPreferences::m_bShutDownPC; 
// ==> Pay Back First
bool	CPreferences::m_bPayBackFirst;
// <== Pay Back First
bool	CPreferences::m_bBufferDisplay;

//morph4u - ////////////////////////////////////////

BOOL	CPreferences::m_bIsRunningAeroGlass;
bool	CPreferences::m_bPreventStandby;
bool	CPreferences::m_bStoreSearches;
bool	CPreferences::m_bKnown2Buffer; //zz_fly :: known2 buffer

CPreferences::CPreferences()
{
#ifdef _DEBUG
	m_iDbgHeap = 1;
#endif
}

CPreferences::~CPreferences()
{
}

LPCTSTR CPreferences::GetConfigFile()
{
	return theApp.m_pszProfileName;
}

void CPreferences::Init()
{
	Preferences_Ext_Struct prefsExt;
	memset(&prefsExt, 0, sizeof prefsExt);

	m_strFileCommentsFilePath = GetMuleDirectory(EMULE_CONFIGDIR) + L"fileinfo.ini";

	///////////////////////////////////////////////////////////////////////////
	// Move *.log files from application directory into 'log' directory
	//
	/*CFileFind ff;
	BOOL bFoundFile = ff.FindFile(GetMuleDirectory(EMULE_EXECUTEABLEDIR) + L"eMule*.log", 0);
	while (bFoundFile)
	{
		bFoundFile = ff.FindNextFile();
		if (ff.IsDots() || ff.IsSystem() || ff.IsDirectory() || ff.IsHidden())
			continue;
		MoveFile(ff.GetFilePath(), GetMuleDirectory(EMULE_LOGDIR) + ff.GetFileName());
	}
	ff.Close();*/

	///////////////////////////////////////////////////////////////////////////
	// Move 'downloads.txt/bak' files from application and/or data-base directory
	// into 'config' directory
	//
	/*if (PathFileExists(GetMuleDirectory(EMULE_DATABASEDIR) + L"downloads.txt"))
		MoveFile(GetMuleDirectory(EMULE_DATABASEDIR) + L"downloads.txt", GetMuleDirectory(EMULE_CONFIGDIR) + L"downloads.txt");
	if (PathFileExists(GetMuleDirectory(EMULE_DATABASEDIR) + L"downloads.bak"))
		MoveFile(GetMuleDirectory(EMULE_DATABASEDIR) + L"downloads.bak", GetMuleDirectory(EMULE_CONFIGDIR) + L"downloads.bak");
	if (PathFileExists(GetMuleDirectory(EMULE_EXECUTEABLEDIR) + L"downloads.txt"))
		MoveFile(GetMuleDirectory(EMULE_EXECUTEABLEDIR) + L"downloads.txt", GetMuleDirectory(EMULE_CONFIGDIR) + L"downloads.txt");
	if (PathFileExists(GetMuleDirectory(EMULE_EXECUTEABLEDIR) + L"downloads.bak"))
		MoveFile(GetMuleDirectory(EMULE_EXECUTEABLEDIR) + L"downloads.bak", GetMuleDirectory(EMULE_CONFIGDIR) + L"downloads.bak");*/
	//CreateUserHash();

	LoadPreferences();

	// -khaos--+++> Load Stats
	// I changed this to a seperate function because it is now also used
	// to load the stats backup and to load stats from preferences.ini.old.
	LoadStats();
	// <-----khaos-

	// load preferences.dat or set standart values
	CString strFullPath;
	strFullPath = GetMuleDirectory(EMULE_CONFIGDIR) + L"preferences.dat";
	FILE* preffile = _tfsopen(strFullPath, L"rb", _SH_DENYWR);

	if (!preffile){
		SetStandartValues();
	}
	else{
		if (fread(&prefsExt,sizeof(Preferences_Ext_Struct),1,preffile) != 1 || ferror(preffile))
			SetStandartValues();
		else{ //Enig123 :: Fix
			md4cpy(userhash, prefsExt.userhash);
			EmuleWindowPlacement = prefsExt.EmuleWindowPlacement;
			//zz_fly :: check userhash after initialized :: Enig123 :: start
			userhash[5] = 0;
			userhash[14] = 0;
			if (isnulmd4(userhash)) //Xman Bugfix by ilmira
				CreateUserHash();
			else{
				userhash[5] = 14;
				userhash[14] = 111;
			}
			//zz_fly :: check userhash after initialized :: Enig123 :: end
		} //Enig123 :: Fix

		fclose(preffile);
		smartidstate = 0;
	}

	// shared directories
	strFullPath = GetMuleDirectory(EMULE_CONFIGDIR) + L"shareddir.dat";
	CStdioFile* sdirfile = new CStdioFile();
	bool bIsUnicodeFile = IsUnicodeFile(strFullPath); // check for BOM
	// open the text file either in ANSI (text) or Unicode (binary), this way we can read old and new files
	// with nearly the same code..
	if (sdirfile->Open(strFullPath, (bIsUnicodeFile ? (CFile::modeRead | CFile::shareDenyWrite | CFile::typeBinary)
													: (CFile::modeRead | CFile::shareDenyWrite))))
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
							continue;
					}
				}

				if (toadd.Right(1) != L'\\')
					toadd.Append(L"\\");
				shareddir_list.AddHead(toadd);
			}
		}
		catch (CFileException* ex) {
			ASSERT(0);
			ex->Delete();
		}
		sdirfile->Close();
	}
	delete sdirfile;


	// Explicitly inform the user about errors with incoming/temp folders!
	if (!PathFileExists(GetMuleDirectory(EMULE_INCOMINGDIR)) && !::CreateDirectory(GetMuleDirectory(EMULE_INCOMINGDIR),0)) {
		XMSGBOXPARAMS params;
		params.nTimeoutSeconds = 5;
		CString strError;
		strError.Format(GetResString(IDS_ERR_CREATE_DIR), GetResString(IDS_PW_INCOMING), GetMuleDirectory(EMULE_INCOMINGDIR), GetErrorMessage(GetLastError()));
		XMessageBox(NULL, strError, _T("eMule"), MB_ICONERROR, &params);

		m_strIncomingDir = GetDefaultDirectory(EMULE_INCOMINGDIR, true); // will also try to create it if needed
		if (!PathFileExists(GetMuleDirectory(EMULE_INCOMINGDIR))){
			strError.Format(GetResString(IDS_ERR_CREATE_DIR), GetResString(IDS_PW_INCOMING), GetMuleDirectory(EMULE_INCOMINGDIR), GetErrorMessage(GetLastError()));
			XMessageBox(NULL, strError, _T("eMule"), MB_ICONERROR, &params);
		}
	}
	if (!PathFileExists(GetTempDir()) && !::CreateDirectory(GetTempDir(),0)) {
		XMSGBOXPARAMS params;
		params.nTimeoutSeconds = 5;
		CString strError;
		strError.Format(GetResString(IDS_ERR_CREATE_DIR), GetResString(IDS_PW_TEMP), GetTempDir(), GetErrorMessage(GetLastError()));
		XMessageBox(NULL, strError, _T("eMule"), MB_ICONERROR, &params);

		tempdir.SetAt(0, GetDefaultDirectory(EMULE_TEMPDIR, true)); // will also try to create it if needed);
		if (!PathFileExists(GetTempDir())){
			strError.Format(GetResString(IDS_ERR_CREATE_DIR), GetResString(IDS_PW_TEMP), GetTempDir(), GetErrorMessage(GetLastError()));
			XMessageBox(NULL, strError, _T("eMule"), MB_ICONERROR, &params);
		}
	}

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

//	Save();
}

// SLUGFILLER: SafeHash remove - global form of IsTempFile unnececery
/*
bool CPreferences::IsTempFile(const CString& rstrDirectory, const CString& rstrName)
{
	bool bFound = false;
	for (size_t i=0;i<tempdir.GetCount() && !bFound;i++)
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
		if (_stscanf_s(strNameLower, _apszNotSharedExts[i], &uNum, &iChar) == 2 && iChar == L'|')
			return true;
	}

	return false;
}
*/

bool CPreferences::IsAcceptUpload()// X: [RU] - [RefuseUpload]
{
	return !(refuseupload&&(rumax<theApp.emuledlg->GetDownloadDatarate()));
}

//Xman Xtreme Mod
// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
float CPreferences::GetMaxDownload() {
	//dont be a Lam3r :)
	const float maxUpload = (GetMaxUpload() >= UNLIMITED) ? GetMaxGraphUploadRate() : GetMaxUpload();
	/* //remove ratio
	if(maxUpload < 4.0f)
	return (3.0f * maxUpload < maxdownload) ? 3.0f * maxUpload : maxdownload;
	else if(maxUpload < 11.0f) //Xman changed to 11
	return (4.0f * maxUpload < maxdownload) ? 4.0f * maxUpload : maxdownload;
	*/ 
	return maxdownload;
}
// Maella end

// remove ratio
/* 
uint64 CPreferences::GetMaxDownloadInBytesPerSec() {
	//dont be a Lam3r :)
	return ((uint64)GetMaxDownload() * 1024);
}
//Xman end
*/ 

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
	ini.WriteUInt64(L"TotalDownloadedBytes", theApp.pBandWidthControl->GeteMuleIn()+GetTotalDownloaded()); // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	ini.WriteInt(L"DownSuccessfulSessions", cumDownSuccessfulSessions);
	ini.WriteInt(L"DownFailedSessions", cumDownFailedSessions);
	ini.WriteInt(L"DownAvgTime", (GetDownC_AvgTime() + GetDownS_AvgTime()) / 2);
	ini.WriteUInt64(L"LostFromCorruption", cumLostFromCorruption + sesLostFromCorruption);
	ini.WriteUInt64(L"SavedFromCompression", sesSavedFromCompression + cumSavedFromCompression);
	ini.WriteInt(L"PartsSavedByICH", cumPartsSavedByICH + sesPartsSavedByICH);

	ini.WriteUInt64(L"DownData_EDONKEY", GetCumDownData(SOFT_EDONKEY));
	ini.WriteUInt64(L"DownData_EDONKEYHYBRID", GetCumDownData(SOFT_EDONKEYHYBRID));
	ini.WriteUInt64(L"DownData_EMULE", GetCumDownData(SOFT_EMULE));
	ini.WriteUInt64(L"DownData_MLDONKEY", GetCumDownData(SOFT_MLDONKEY));
	ini.WriteUInt64(L"DownData_LMULE", GetCumDownData(SOFT_EMULECOMPAT));
	ini.WriteUInt64(L"DownData_AMULE", GetCumDownData(SOFT_AMULE));
	ini.WriteUInt64(L"DownData_SHAREAZA", GetCumDownData(SOFT_SHAREAZA));
	ini.WriteUInt64(L"DownDataPort_4662", GetCumDownDataPort(PORT_4662));
	ini.WriteUInt64(L"DownDataPort_OTHER", GetCumDownDataPort(PORT_OTHER));

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
	ini.WriteUInt64(L"UpData_EDONKEY", GetCumUpData(SOFT_EDONKEY));
	ini.WriteUInt64(L"UpData_EDONKEYHYBRID", GetCumUpData(SOFT_EDONKEYHYBRID));
	ini.WriteUInt64(L"UpData_EMULE", GetCumUpData(SOFT_EMULE));
	ini.WriteUInt64(L"UpData_MLDONKEY", GetCumUpData(SOFT_MLDONKEY));
	ini.WriteUInt64(L"UpData_LMULE", GetCumUpData(SOFT_EMULECOMPAT));
	ini.WriteUInt64(L"UpData_AMULE", GetCumUpData(SOFT_AMULE));
	ini.WriteUInt64(L"UpData_SHAREAZA", GetCumUpData(SOFT_SHAREAZA));
	ini.WriteUInt64(L"UpDataPort_4662", GetCumUpDataPort(PORT_4662));
	ini.WriteUInt64(L"UpDataPort_OTHER", GetCumUpDataPort(PORT_OTHER));
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
	uint32 servtotal, servfail, notused, servtuser, servtfile;
	float servocc;
	theApp.serverlist->GetStatus(servtotal, servfail, notused, notused, notused, servtuser, servtfile, servocc);
	
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
	if ((UINT_PTR)theApp.sharedfiles->GetCount() > cumSharedMostFilesShared)
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
	ini.WriteUInt64(L"statsDateTimeLastReset", stat_datetimeLastReset);

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
	uint32 servtotal, servfail, notused, servtuser, servtfile;
	float servocc;
	theApp.serverlist->GetStatus( servtotal, servfail, notused, notused, notused, servtuser, servtfile, servocc );
	if ((servtotal-servfail)>cumSrvrsMostWorkingServers) cumSrvrsMostWorkingServers = (servtotal-servfail);
	if (servtuser>cumSrvrsMostUsersOnline) cumSrvrsMostUsersOnline = servtuser;
	if (servtfile>cumSrvrsMostFilesAvail) cumSrvrsMostFilesAvail = servtfile;

	// Shared Files
	if ((UINT_PTR)theApp.sharedfiles->GetCount() > cumSharedMostFilesShared)
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
										   BOOL bUpDown, uint32 bytes, bool sentToPBF)// ==> Pay Back First
{
	//	This function adds the transferred bytes to the appropriate variables,
	//	as well as to the totals for all clients. - Khaos
	//	PARAMETERS:
	//	uClientID - The identifier for which client software sent or received this data, eg SO_EMULE
	//	uClientPort - The remote port of the client that sent or received this data, eg 4662
	//	bFromPF - Applies only to uploads.  True is from partfile, False is from non-partfile.
	//	bUpDown - True is Up, False is Down
	//	bytes - Number of bytes sent by the client.  Subtract header before calling.

	if(bUpDown){
			//	Upline Data
			switch (uClientID){
				// Update session client breakdown stats for sent bytes...
				case SO_EMULE:
				case SO_OLDEMULE:		sesUpData[SOFT_EMULE]+=bytes;			break;
				case SO_EDONKEYHYBRID:	sesUpData[SOFT_EDONKEYHYBRID]+=bytes;	break;
				case SO_EDONKEY:		sesUpData[SOFT_EDONKEY]+=bytes;		break;
				case SO_MLDONKEY:		sesUpData[SOFT_MLDONKEY]+=bytes;		break;
				case SO_AMULE:			sesUpData[SOFT_AMULE]+=bytes;			break;
				case SO_SHAREAZA:		sesUpData[SOFT_SHAREAZA]+=bytes;		break;
				// ==> Enhanced Client Recognition [Spike] - Stulle
				case SO_HYDRANODE:
				case SO_EMULEPLUS:
				case SO_TRUSTYFILES:
				// <== Enhanced Client Recognition [Spike] - Stulle
			case SO_EASYMULE2:
				case SO_CDONKEY:
				case SO_LPHANT:
				case SO_XMULE:			sesUpData[SOFT_EMULECOMPAT]+=bytes;	break;
			}

			switch (uClientPort){
				// Update session port breakdown stats for sent bytes...
				case 4662:				sesUpDataPort[PORT_4662]+=bytes;		break;
				//case (UINT)-2:		sesUpDataPort_URL+=bytes;		break;
				default:				sesUpDataPort[PORT_OTHER]+=bytes;		break;
			}

			if (bFromPF)				sesUpData_Partfile+=bytes;
			else						sesUpData_File+=bytes;

			//	Add to our total for sent bytes...
		theApp.UpdateSentBytes(bytes, sentToPBF);// ==> Pay Back First
	}
	else{
			// Downline Data
			switch (uClientID){
                // Update session client breakdown stats for received bytes...
				case SO_EMULE:
				case SO_OLDEMULE:		sesDownData[SOFT_EMULE]+=bytes;		break;
				case SO_EDONKEYHYBRID:	sesDownData[SOFT_EDONKEYHYBRID]+=bytes;break;
				case SO_EDONKEY:		sesDownData[SOFT_EDONKEY]+=bytes;		break;
				case SO_MLDONKEY:		sesDownData[SOFT_MLDONKEY]+=bytes;	break;
				case SO_AMULE:			sesDownData[SOFT_AMULE]+=bytes;		break;
				case SO_SHAREAZA:		sesDownData[SOFT_SHAREAZA]+=bytes;	break;
				// ==> Enhanced Client Recognition [Spike] - Stulle
				case SO_HYDRANODE:
				case SO_EMULEPLUS:
				case SO_TRUSTYFILES:
				// <== Enhanced Client Recognition [Spike] - Stulle
			case SO_EASYMULE2:
				case SO_CDONKEY:
				case SO_LPHANT:
				case SO_XMULE:			sesDownData[SOFT_EMULECOMPAT]+=bytes;	break;
			}

			switch (uClientPort){
				// Update session port breakdown stats for received bytes...
				// For now we are only going to break it down by default and non-default.
				// A statistical analysis of all data sent from every single port/domain is
				// beyond the scope of this add-on.
				case 4662:				sesDownDataPort[PORT_4662]+=bytes;	break;
				//case (UINT)-2:		sesDownDataPort_URL+=bytes;		break;
				default:				sesDownDataPort[PORT_OTHER]+=bytes;	break;
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
	cumDownCompletedFiles=0;
	cumDownSuccessfulSessions=0;
	cumDownFailedSessions=0;
	cumDownAvgTime=0;
	cumLostFromCorruption=0;
	cumSavedFromCompression=0;
	cumPartsSavedByICH=0;
	for(INT_PTR i=0;i<PORT_COUNT;i++){
		cumUpDataPort[i]=0;
		cumDownDataPort[i]=0;
	}
	for(INT_PTR i=0;i<UP_SOFT_COUNT;i++)
		cumUpData[i]=0;
	for(INT_PTR i=0;i<DOWN_SOFT_COUNT;i++)
		cumDownData[i]=0;
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
	stat_datetimeLastReset = time(NULL);

	// Save the reset stats
	SaveStats();
	//theApp.emuledlg->statisticswnd->ShowStatistics(/*true*/);
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

	switch (loadBackUp) {
		case 0:
			// for transition...
				sINI.Format(L"%sstatistics.ini", GetMuleDirectory(EMULE_CONFIGDIR));
			if (_taccess(sINI, 0))
				sINI.Format(L"%spreferences.ini", GetMuleDirectory(EMULE_CONFIGDIR));
			break;
		case 1:
			sINI.Format(L"%sstatbkup.ini", GetMuleDirectory(EMULE_CONFIGDIR));
			if (_taccess(sINI, 0))
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
	cumUpOverheadTotal				= ini.GetUInt64(L"UpOverheadTotal");
	cumUpOverheadFileReq			= ini.GetUInt64(L"UpOverheadFileReq");
	cumUpOverheadSrcEx				= ini.GetUInt64(L"UpOverheadSrcEx");
	cumUpOverheadServer				= ini.GetUInt64(L"UpOverheadServer");
	cumUpOverheadKad				= ini.GetUInt64(L"UpOverheadKad");
	cumUpOverheadTotalPackets		= ini.GetUInt64(L"UpOverheadTotalPackets");
	cumUpOverheadFileReqPackets		= ini.GetUInt64(L"UpOverheadFileReqPackets");
	cumUpOverheadSrcExPackets		= ini.GetUInt64(L"UpOverheadSrcExPackets");
	cumUpOverheadServerPackets		= ini.GetUInt64(L"UpOverheadServerPackets");
	cumUpOverheadKadPackets			= ini.GetUInt64(L"UpOverheadKadPackets");

	// Load stats for cumulative upline data
	cumUpSuccessfulSessions			= ini.GetInt(L"UpSuccessfulSessions");
	cumUpFailedSessions				= ini.GetInt(L"UpFailedSessions");
	cumUpAvgTime					= ini.GetInt(L"UpAvgTime");

	// Load cumulative client breakdown stats for sent bytes
	cumUpData[SOFT_EDONKEY]				= ini.GetUInt64(L"UpData_EDONKEY");
	cumUpData[SOFT_EDONKEYHYBRID]			= ini.GetUInt64(L"UpData_EDONKEYHYBRID");
	cumUpData[SOFT_EMULE]					= ini.GetUInt64(L"UpData_EMULE");
	cumUpData[SOFT_MLDONKEY]				= ini.GetUInt64(L"UpData_MLDONKEY");
	cumUpData[SOFT_EMULECOMPAT]			= ini.GetUInt64(L"UpData_LMULE");
	cumUpData[SOFT_AMULE]					= ini.GetUInt64(L"UpData_AMULE");
	cumUpData[SOFT_SHAREAZA]				= ini.GetUInt64(L"UpData_SHAREAZA");

	// Load cumulative port breakdown stats for sent bytes
	cumUpDataPort[PORT_4662]				= ini.GetUInt64(L"UpDataPort_4662");
	cumUpDataPort[PORT_OTHER]				= ini.GetUInt64(L"UpDataPort_OTHER");

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
	cumDownData[SOFT_EDONKEY]				= ini.GetUInt64(L"DownData_EDONKEY");
	cumDownData[SOFT_EDONKEYHYBRID]		= ini.GetUInt64(L"DownData_EDONKEYHYBRID");
	cumDownData[SOFT_EMULE]				= ini.GetUInt64(L"DownData_EMULE");
	cumDownData[SOFT_MLDONKEY]			= ini.GetUInt64(L"DownData_MLDONKEY");
	cumDownData[SOFT_EMULECOMPAT]			= ini.GetUInt64(L"DownData_LMULE");
	cumDownData[SOFT_AMULE]				= ini.GetUInt64(L"DownData_AMULE");
	cumDownData[SOFT_SHAREAZA]			= ini.GetUInt64(L"DownData_SHAREAZA");

	// Load cumulative port breakdown stats for received bytes
	cumDownDataPort[PORT_4662]			= ini.GetUInt64(L"DownDataPort_4662");
	cumDownDataPort[PORT_OTHER]			= ini.GetUInt64(L"DownDataPort_OTHER");

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
	stat_datetimeLastReset			= ini.GetUInt64(L"statsDateTimeLastReset");

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
		if (_taccess(sINIBackUp, 0) == 0){
			// X-Ray :: FiXeS :: Crashfix :: Start :: WiZaRd
			//calling the "CFile" static functions here is dangerous because they will 
			//throw exceptions if they fail and those aren't caught anywhere...
			/*
			CFile::Remove(sINI);				// Remove the backup that we just restored from
			CFile::Rename(sINIBackUp, sINI);	// Rename our temporary backup to the normal statbkup.ini filename.
			*/
			_tremove(sINI);				// Remove the backup that we just restored from
			_trename(sINIBackUp, sINI);	// Rename our temporary backup to the normal statbkup.ini filename.
			// X-Ray :: FiXeS :: Crashfix :: End :: WiZaRd
		}

		// Since we know this is a restore, now we should call ShowStatistics to update the data items to the new ones we just loaded.
		// Otherwise user is left waiting around for the tick counter to reach the next automatic update (Depending on setting in prefs)
		//theApp.emuledlg->statisticswnd->ShowStatistics(); // X: move to BOOL CStatisticsTree::OnCommand
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

		for(INT_PTR i=0;i<DOWN_SOFT_COUNT;i++)
			sesDownData[i]= 0;

		for(INT_PTR i=0;i<UP_SOFT_COUNT;i++)
			sesUpData[i]=0;
		
		for(INT_PTR i=0;i<PORT_COUNT;i++){
			sesUpDataPort[i]= 0;
			sesDownDataPort[i]=0;
		}

		sesDownSuccessfulSessions	= 0;
		sesDownFailedSessions		= 0;
		sesPartsSavedByICH			= 0;
	}

	if (!fileex || (stat_datetimeLastReset==0 && totalDownloadedBytes==0 && totalUploadedBytes==0))
	{
		stat_datetimeLastReset = time(NULL);
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
	CString strFullPath = GetMuleDirectory(EMULE_CONFIGDIR) + L"preferences.dat";

	FILE* preffile = _tfsopen(strFullPath, L"wb", _SH_DENYWR);
	if (preffile){
		Preferences_Ext_Struct prefsExt;
		prefsExt.version=PREFFILE_VERSION;
		prefsExt.EmuleWindowPlacement=EmuleWindowPlacement;
		md4cpy(prefsExt.userhash, userhash);

		error = fwrite(&prefsExt, sizeof(Preferences_Ext_Struct), 1, preffile) != 1;
		if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !CemuleDlg::IsRunning())){
			fflush(preffile); // flush file stream buffers to disk buffers
			(void)_commit(_fileno(preffile)); // commit disk buffers to disk
		}
		fclose(preffile);
	}
	else
		error = true;

	SavePreferences();
	SaveStats();

	strFullPath = GetMuleDirectory(EMULE_CONFIGDIR) + L"shareddir.dat";
	CStdioFile sdirfile;
	if (sdirfile.Open(strFullPath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite | CFile::typeBinary))
	{
		try{
			// write Unicode byte-order mark 0xFEFF
			WORD wBOM = 0xFEFF;
			sdirfile.Write(&wBOM, sizeof(wBOM));

			for (POSITION pos = shareddir_list.GetTailPosition();pos != 0;){
				sdirfile.WriteString(shareddir_list.GetPrev(pos));
				sdirfile.Write(L"\r\n", sizeof(TCHAR)*2);
			}
			if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !CemuleDlg::IsRunning())){
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

int CPreferences::GetRecommendedMaxConnections(){
	return 500;
}

void CPreferences::SavePreferences()
{
	CString buffer;
	
	CIni ini(GetConfigFile(), L"eMule");
	//---
	//ini.WriteString(L"AppVersion", theApp.m_strCurVersionLong); // X: keep eMule section first
	//---
        
	//Xman auto update IPFilter
	ini.WriteBinary(_T("IPfilterVersion"), (LPBYTE)&m_IPfilterVersion, sizeof(m_IPfilterVersion),L"Xtreme"); 
	ini.WriteUInt64(_T("lastipfiltercheck"),m_last_ipfilter_check);
	//Xman end

	if(prefReadonly) // X: [ROP] - [ReadOnlyPreference]
		return;
	//ini.WriteStringUTF8(L"Nick", strNick, L"eMule");
#ifdef _DEBUG
	ini.WriteInt(L"DebugHeap", m_iDbgHeap);
#endif
	ini.WriteString(L"IncomingDir", m_strIncomingDir, L"eMule");
	
	ini.WriteString(L"TempDir", tempdir.GetAt(0));

	CString tempdirs;
	for (size_t i=1;i<tempdir.GetCount();i++) {
		tempdirs.Append(tempdir.GetAt(i) );
		if (i+1<tempdir.GetCount())
			tempdirs.Append(L"|");
	}
	ini.WriteString(L"TempDirs", tempdirs);

//    ini.WriteInt(L"MinUpload", minupload);
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
	ini.WriteInt(L"Port",randomPortOnStartup?0:port); // X: [RPOS] - [RandomPortOnStartup]
	ini.WriteInt(L"UDPPort",randomPortOnStartup?INT_MAX:udpport);
	ini.WriteInt(L"ServerUDPPort", nServerUDPPort);
	ini.WriteInt(L"MaxSourcesPerFile",maxsourceperfile );
	ini.WriteWORD(L"Language",m_wLanguageID);
	ini.WriteInt(L"SeeShare",m_iSeeShares);
	ini.WriteInt(L"ToolTipDelay",m_iToolDelayTime);
	ini.WriteInt(L"StatGraphsInterval",trafficOMeterInterval);
	ini.WriteInt(L"StatsInterval",statsInterval);
	ini.WriteBool(L"StatsFillGraphs",m_bFillGraphs);
	ini.WriteInt(L"DeadServerRetry",m_uDeadServerRetries);
	ini.WriteInt(L"ServerKeepAliveTimeout",m_dwServerKeepAliveTimeout);
	ini.WriteInt(L"SplitterbarPosition",splitterbarPosition);
	ini.WriteInt(L"SplitterbarPositionServer",splitterbarPositionSvr);
	ini.WriteInt(L"SplitterbarPositionStat",splitterbarPositionStat+1);
	ini.WriteInt(L"SplitterbarPositionStat_HR",splitterbarPositionStat_HR); //Xman BlueSonicBoy-Stats-Fix
	ini.WriteInt(L"TransferWnd1",m_uTransferWnd1);
	ini.WriteInt(L"TransferWnd2",m_uTransferWnd2);
	ini.WriteInt(L"StatsAverageMinutes",statsAverageMinutes);
	ini.WriteInt(L"MaxConnectionsPerFiveSeconds",MaxConperFive);

	ini.WriteBool(L"Reconnect",reconnect);
	ini.WriteBool(L"Scoresystem",m_bUseServerPriorities);	
	ini.WriteBool(L"PreventStandby", m_bPreventStandby);
	ini.WriteBool(L"StoreSearches", m_bStoreSearches);
	ini.WriteBool(L"Splashscreen",splashscreen);
	ini.WriteBool(L"BringToFront",bringtoforeground);
	ini.WriteBool(L"ConfirmExit",confirmExit);
	ini.WriteBool(L"FilterBadIPs",filterLANIPs);
	//ini.WriteBool(L"Autoconnect",autoconnect);
	ini.WriteBool(L"StartupMinimized",startMinimized);
	ini.WriteBool(L"AutoStart",m_bAutoStart);
		ini.WriteInt(L"LastMainWndDlgID",m_iLastMainWndDlgID);
		ini.WriteInt(L"LastLogPaneID",m_iLastLogPaneID);
	ini.WriteBool(L"SafeServerConnect",m_bSafeServerConnect);
	ini.WriteBool(L"ShowRatesOnTitle",showRatesInTitle);
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

	ini.WriteString(L"TxtEditor",m_strTxtEditor);
    ini.WriteString(L"VideoPlayer",m_strVideoPlayer);
	ini.WriteString(L"VideoPlayerArgs",m_strVideoPlayerArgs);
	ini.WriteString(L"DateTimeFormat",GetDateTimeFormat());
	ini.WriteString(L"DateTimeFormat4Log",GetDateTimeFormat4Log());
	ini.WriteString(L"FilenameCleanups",filenameCleanups);

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
	ini.WriteBool(L"LogDrop", m_bLogDrop); //Xman Xtreme Downloadmanager
	ini.WriteBool(L"Logpartmismatch", m_bLogpartmismatch); //Xman Log part/size-mismatch

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
	//ini.WriteBool(L"ManualHighPrio", m_bManualAddedServersHighPriority);
	ini.WriteBool(L"FullChunkTransfers", m_btransferfullchunks);
	ini.WriteBool(L"VideoPreviewBackupped", moviePreviewBackup);
	ini.WriteInt(L"StartNextFile", m_istartnextfile);
	ini.WriteInt(L"PreviewSmallBlocks", m_iPreviewSmallBlocks);

	//ini.DeleteKey(L"FileBufferSizePref"); // delete old 'file buff size' setting
	//ini.WriteInt(L"FileBufferSize", m_uFileBufferSize);

	//ini.DeleteKey(L"QueueSizePref"); // delete old 'queue size' setting
	ini.WriteInt(L"QueueSize", m_iQueueSize);

	ini.WriteInt(L"CommitFiles", m_iCommitFiles);
	ini.WriteBool(L"DAPPref", m_bDAP);
	ini.WriteBool(L"UAPPref", m_bUAP);
	ini.WriteBool(L"FilterServersByIP",filterserverbyip);
	ini.WriteBool(L"SaveLogToDisk",log2disk);
	ini.WriteBool(L"SaveDebugToDisk",debug2disk);
	ini.WriteBool(L"ShowInfoOnCatTabs",showCatTabInfos);
	ini.WriteBool(L"AutoFilenameCleanup",autofilenamecleanup);
	ini.WriteBool(L"ShowExtControls",m_bExtControls);
	//ini.WriteBool(L"NetworkKademlia",networkkademlia);
	ini.WriteBool(L"NetworkED2K",networked2k);
	ini.WriteBool(L"TransflstRemainOrder",m_bTransflstRemain);
	ini.WriteBool(L"UseSimpleTimeRemainingcomputation",m_bUseOldTimeRemaining);
	ini.WriteBool(L"AllocateFullFile",m_bAllocFull);

	ini.WriteInt(L"FilterLevel",filterlevel);

	ini.WriteBool(L"SecureIdent", m_bUseSecureIdent);// change the name in future version to enable it by default
	ini.WriteBool(L"RemoveFilesToBin",m_bRemove2bin);

	// ZZ:UploadSpeedSense -->
//    ini.WriteBool(L"USSEnabled", m_bDynUpEnabled);
//    ini.WriteBool(L"USSUseMillisecondPingTolerance", m_bDynUpUseMillisecondPingTolerance);
//    ini.WriteInt(L"USSPingTolerance", m_iDynUpPingTolerance);
//	ini.WriteInt(L"USSPingToleranceMilliseconds", m_iDynUpPingToleranceMilliseconds); // EastShare - Add by TAHO, USS limit
//    ini.WriteInt(L"USSGoingUpDivider", m_iDynUpGoingUpDivider);
//    ini.WriteInt(L"USSGoingDownDivider", m_iDynUpGoingDownDivider);
//    ini.WriteInt(L"USSNumberOfPings", m_iDynUpNumberOfPings);
	// ZZ:UploadSpeedSense <--

	//ini.WriteBool(L"A4AFSaveCpu", m_bA4AFSaveCpu); // ZZ:DownloadManager
	ini.WriteBool(L"HighresTimer", m_bHighresTimer);
	ini.WriteInt(L"WebMirrorAlertLevel", m_nWebMirrorAlertLevel);
	ini.WriteBool(L"RunAsUnprivilegedUser", m_bRunAsUser);
	ini.WriteBool(L"OpenPortsOnStartUp", m_bOpenPortsOnStartUp);
	ini.WriteInt(L"DebugLogLevel", m_byLogLevel);
	ini.WriteInt(L"WinXPSP2OrHigher", (int)IsRunningXPSP2OrHigher());
	ini.WriteBool(L"RememberCancelledFiles", m_bRememberCancelledFiles);
	ini.WriteBool(L"RememberDownloadedFiles", m_bRememberDownloadedFiles);

	ini.WriteBool(L"WinaTransToolbar", m_bWinaTransToolbar);
	ini.WriteBool(L"CryptLayerRequested", m_bCryptLayerRequested);
	ini.WriteBool(L"CryptLayerRequired", m_bCryptLayerRequired);
	ini.WriteBool(L"CryptLayerSupported", m_bCryptLayerSupported);
	ini.WriteInt(L"KadUDPKey", m_dwKadUDPKey);

	ini.WriteBool(L"EnableSearchResultSpamFilter", m_bEnableSearchResultFilter);
	
	//Xman Added PaddingLength to Extended preferences
	ini.WriteInt(L"CryptTCPPaddingLength",m_byCryptTCPPaddingLength);
	//Xman end
	ini.WriteBool(L"RestoreLastMainWndDlg",m_bRestoreLastMainWndDlg);
	ini.WriteInt(L"SearchResultsFileSizeFormat", m_eFileSizeFormat);

	///////////////////////////////////////////////////////////////////////////
	// Section: "Proxy"
	//
	ini.WriteBool(L"ProxyEnablePassword",proxy.EnablePassword,L"Proxy");
	ini.WriteBool(L"ProxyEnableProxy",proxy.UseProxy);
	ini.WriteString(L"ProxyName",CString(proxy.name));
	ini.WriteString(L"ProxyPassword",CString(proxy.password));
	ini.WriteString(L"ProxyUser",CString(proxy.user));
	ini.WriteInt(L"ProxyPort",proxy.port);
	ini.WriteInt(L"ProxyType",proxy.type);


	///////////////////////////////////////////////////////////////////////////
	// Section: "Statistics"
	//
	ini.WriteString(L"statsExpandedTreeItems", m_strStatsExpandedTreeItems);
	CString buffer2;
	for (size_t i=0; i < _countof(m_adwStatsColors); i++) {
		buffer.Format(L"0x%06x",GetStatsColor(i));
		buffer2.Format(L"StatColor%i",i);
		ini.WriteString(buffer2,buffer);
	}
	ini.WriteBool(L"HasCustomTaskIconColor", bHasCustomTaskIconColor);

	///////////////////////////////////////////////////////////////////////////
	// Section: "UPnP"
	//
	ini.WriteBool(L"EnableUPnP", m_bEnableUPnP, L"UPnP");
	ini.WriteBool(L"SkipWANIPSetup", m_bSkipWANIPSetup);
	ini.WriteBool(L"SkipWANPPPSetup", m_bSkipWANPPPSetup);
	ini.WriteBool(L"CloseUPnPOnExit", m_bCloseUPnPOnExit);
	ini.WriteInt(L"LastWorkingImplementation", m_nLastWorkingImpl);
	ini.WriteBool(L"UPnPNAT_TryRandom", m_bUPnPTryRandom);
	ini.WriteBool(L"UPnPNAT_RebindOnIPChange", m_bUPnPRebindOnIPChange); //zz_fly :: rebind UPnP on ip-change
	//upnp_end

	//Xman Xtreme Upload
	ini.WriteFloat(L"uploadslotspeed",m_slotspeed,L"Xtreme");
	ini.WriteBool(L"openmoreslots",m_openmoreslots);
	ini.WriteBool(L"bandwidthnotreachedslots",m_bandwidthnotreachedslots);
	//ini.WriteInt(L"sendbuffersize", m_sendbuffersize);// X: [DSRB] - [Dynamic Send and Receive Buffer]

	ini.WriteBool(L"retryconnectionattempts", retryconnectionattempts); 
    
	//Xman don't overwrite bak files if last sessions crashed
	ini.WriteBool(L"last_session_aborted_in_an_unnormal_way", m_last_session_aborted_in_an_unnormal_way);// X: [CI] - [Code Improvement]

	//Xman show additional graph lines
	ini.WriteBool(L"ShowAdditionalGraph", m_bShowAdditionalGraph);

	//Xman always one release-slot
	//ini.WriteBool(L"onerealeseslot",m_onerealeseslot);
	//Xman end

	//Xman advanced upload-priority
	ini.WriteBool(L"AdvancedAutoPrio",m_AdvancedAutoPrio);
	//Xman end

	//Xman chunk chooser
	ini.WriteInt(L"chunkchooser", m_chunkchooser);

	//Xman auto update IPFilter
	ini.WriteString(L"AutoUpdateIPFilter_URL", m_strautoupdateipfilter_url);
	ini.WriteBool(L"AutoUpdateIPFilter", m_bautoupdateipfilter);
	//Xman end

	//Xman count block/success send
	ini.WriteBool(L"ShowSocketBlockRatio", m_showblockratio);
	ini.WriteBool(L"DropBlockingSockets", m_dropblockingsockets);

	//Xman remove unused AICH-hashes
	ini.WriteBool(L"rememberAICH", m_rememberAICH);

	//Xman smooth-accurate-graph
	ini.WriteBool(L"usesmoothgraph",usesmoothgraph);

	// Maella -MTU Configuration-
	if (MTU<500)
		MTU=500;
	if (MTU>1500)
		MTU=1500;
	ini.WriteInt(L"MTU", MTU);
	// Maella end

	ini.WriteBool(L"usedoublesendsize",usedoublesendsize);
	// netfinity: Maximum Segment Size (MSS - Vista only) //added by zz_fly
	ini.WriteBool(L"retrieveMTUFromSocket", retrieveMTUFromSocket);

	// Maella -Network Adapter Feedback Control-
	ini.WriteBool(L"NAFCFullControl", NAFCFullControl);
	ini.WriteInt(L"ForceNAFCAdapter",forceNAFCadapter);
	// Maella end

	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	ini.WriteInt(L"DatarateSamples", datarateSamples);
	// Maella end

	// Maella -One-queue-per-file- (idea bloodymad)
	ini.WriteBool(L"EnableMultiQueue", enableMultiQueue);
	// Maella end

	// Mighty Knife: Static server handling
	ini.WriteBool (L"DontRemoveStaticServers",m_bDontRemoveStaticServers);
	// [end] Mighty Knife

	//Xman [MoNKi: -Downloaded History-]
	ini.WriteBool(L"ShowSharedInHistory", m_bHistoryShowShared);
	//Xman end

	ini.WriteBool(L"EnableKnown2Buffer", m_bKnown2Buffer); //zz_fly :: known2 buffer
	//Xman end
	//--------------------------------------------------------------------------
	ini.WriteBool(L"64BitTime", m_bEnable64BitTime,L"Xtreme+");// X: [E64T] - [Enable64BitTime]
	ini.WriteBool(L"RefuseUpload",refuseupload);// X: [RU] - [RefuseUpload]
	ini.WriteInt(L"RUMax", rumax);// X: [RU] - [RefuseUpload]
	ini.WriteInt(L"GlobalBufferSize", m_uGlobalBufferSize);// X: [GB] - [Global Buffer]
	ini.WriteBool(L"DontCompressExt",dontcompressext);// X: [DCE] - [DontCompressExt]
	ini.WriteString(L"CompressExt",compressExt);// X: [DCE] - [DontCompressExt]
	ini.WriteBool(L"PauseOnFileComplete",m_bPauseOnFileComplete); // NEO: POFC - [PauseOnFileComplete]
	ini.WriteBool(L"ShowToolbar",isshowtoolbar);
	ini.WriteBool(L"ShowCatbar",isshowcatbar);
	ini.WriteBool(L"DisableHistoryList",m_bDisableHistoryList);
	ini.WriteBool(L"ShowShareableFile",showShareableFile);
	ini.WriteInt(L"QueryOnHashing",queryOnHashing);// X: [QOH] - [QueryOnHashing]
	ini.WriteInt(L"LastCatID",lastTranWndCatID);// X: [RCI] - [Remember Catalog ID]
	ini.WriteBool(L"DontShareExt",dontsharext);// X: [DSE] - [DontShareExt]
	ini.WriteString(L"ShareExt",shareExt);// X: [DSE] - [DontShareExt]
	ini.WriteBool(L"ShowSpeedGraph",showSpeedGraph); // X: [SGW] - [SpeedGraphWnd]
	ini.WriteInt(L"SpeedGraphX",speedgraphwindowLeft);
	ini.WriteInt(L"SpeedGraphY",speedgraphwindowTop);
	ini.WriteBool(L"randomPortOnStartup",randomPortOnStartup); // X: [RPOS] - [RandomPortOnStartup]
	ini.WriteBool(L"Notifier",notifier);

	// ==> Invisible Mode [TPT/MoNKi] - Stulle
	ini.WriteBool(_T("InvisibleMode"), m_bInvisibleMode);
	ini.WriteInt(_T("InvisibleModeHKKey"), (int)m_cInvisibleModeHotKey);
	ini.WriteInt(_T("InvisibleModeHKKeyModifier"), m_iInvisibleModeHotKeyModifier);
	ini.WriteBool(_T("InvisibleModeStart"), m_bInvisibleModeStart);
	// <== Invisible Mode [TPT/MoNKi] - Stulle

//morph4u + /////////////////////////////////////////////////
    ini.WriteBool(L"GridLines",m_bGridlines);
	ini.WriteBool(L"ShowProgressBar", m_bShowProgressBar); //morph4u :: PercentBar
	ini.WriteString(L"UpdateURLServerMet", m_strServerMetUpdateURL);
	ini.WriteString(L"UpdateURLNodesDat", m_strNodesDatUpdateURL);
// ==> Pay Back First
	ini.WriteBool(_T("IsPayBackFirst"),m_bPayBackFirst);
	// <== Pay Back First
	ini.WriteBool(L"BufferDisplay",m_bBufferDisplay);
//morph4u - //////////////////////////////////////////////
}

void CPreferences::ResetStatsColor(size_t index)
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
		case 11: m_adwStatsColors[11]=RGB(  0,  0,  0); bHasCustomTaskIconColor = false; break;
		case 12: m_adwStatsColors[12]=RGB(255,255,255);break;
		case 13: m_adwStatsColors[13]=RGB(255,255,128);break;
		case 14: m_adwStatsColors[14]=RGB(255,128,128);break;
	}
}

void CPreferences::GetAllStatsColors(size_t iCount, LPDWORD pdwColors)
{
	memset(pdwColors, 0, sizeof(*pdwColors) * iCount);
	memcpy(pdwColors, m_adwStatsColors, sizeof(*pdwColors) * min(_countof(m_adwStatsColors), iCount));
}

bool CPreferences::SetAllStatsColors(size_t iCount, const DWORD* pdwColors)
{
	bool bModified = false;
	size_t iMin = min(_countof(m_adwStatsColors), iCount);
	for (size_t i = 0; i < iMin; i++)
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

void CPreferences::LoadPreferences()
{
	CIni ini(GetConfigFile(), L"eMule");


	CString strCurrVersion, strPrefsVersion;

	strCurrVersion = theApp.m_strCurVersionLong;
	strPrefsVersion = ini.GetString(L"AppVersion");

	m_bFirstStart = strPrefsVersion.IsEmpty();

#ifdef _DEBUG// X: [RDL] - [Remove Debug Log]
	m_iDbgHeap = ini.GetInt(L"DebugHeap", 1);
/*#else
	m_iDbgHeap = 0;*/
#endif

	m_nWebMirrorAlertLevel = ini.GetInt(L"WebMirrorAlertLevel",0);

	/*SetUserNick(ini.GetStringUTF8(L"Nick", DEFAULT_NICK));
	if (strNick.IsEmpty() || IsDefaultNick(strNick))
		SetUserNick(DEFAULT_NICK);*/

	m_strIncomingDir = ini.GetString(L"IncomingDir");
	if (m_strIncomingDir.IsEmpty()) // We want GetDefaultDirectory to also create the folder, so we have to know if we use the default or not
		m_strIncomingDir = GetDefaultDirectory(EMULE_INCOMINGDIR, true);
	MakeFoldername(m_strIncomingDir);

	// load tempdir(s) setting
	CString tempdirs;
	tempdirs = ini.GetString(L"TempDir");
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
			for (size_t i=0;i<tempdir.GetCount();i++)	// avoid double tempdirs
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
//	minupload=(uint16)ini.GetInt(L"MinUpload", 1); //Xman not used!
	maxupload=ini.GetFloat(L"MaxUpload",12); // Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-

	if(maxupload<= 0.0f || maxupload >= UNLIMITED)
	{
		maxupload=11.0f;
		::MessageBox(NULL,
			_T("Warning, an 'unlimited' datarate for the upload limit is invalid\n\n")
			//_T("Attention, une bande passante 'illimitée' pour l'émission est invalide\n\n")
			//_T("Achtung, unbegrenzte Datarate für das Upload ist ungültig\n\n")
			//_T("Advirtiendo, el límite para el datarate del upload es inválido")
			,_T("Warning")//_T("Warning/Attention/Achtung/Advirtiendo")
			, MB_ICONINFORMATION);
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
	else if(m_internetdownreactiontime>10)
		m_internetdownreactiontime=10;

	//Xman end
	//-------------------------------------------------------------------	


	maxconnections=ini.GetInt(L"MaxConnections",GetRecommendedMaxConnections());
	maxhalfconnections=ini.GetInt(L"MaxHalfConnections",9);
	m_bConditionalTCPAccept = ini.GetBool(L"ConditionalTCPAccept", false);

	// reset max halfopen to a default if OS changed to SP2 (or higher) or away
	int dwSP2OrHigher = ini.GetInt(L"WinXPSP2OrHigher", -1);
	sint_ptr dwCurSP2OrHigher = IsRunningXPSP2OrHigher();
	if (dwSP2OrHigher != dwCurSP2OrHigher){
		if (dwCurSP2OrHigher == 0)
			maxhalfconnections = 50;
		else if (dwCurSP2OrHigher == 1)
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
	statsInterval=ini.GetInt(L"StatsInterval",5);//morph4u
	m_bFillGraphs=ini.GetBool(L"StatsFillGraphs");

	m_uDeadServerRetries=ini.GetInt(L"DeadServerRetry",5);
	if (m_uDeadServerRetries > MAX_SERVERFAILCOUNT)
		m_uDeadServerRetries = MAX_SERVERFAILCOUNT;
	m_dwServerKeepAliveTimeout=ini.GetInt(L"ServerKeepAliveTimeout",0);
	splitterbarPosition=ini.GetInt(L"SplitterbarPosition",55);
	if (splitterbarPosition < 9)
		splitterbarPosition = 9;
	else if (splitterbarPosition > 93)
		splitterbarPosition = 93;
	splitterbarPositionStat=ini.GetInt(L"SplitterbarPositionStat",30);
	splitterbarPositionStat_HR=ini.GetInt(L"SplitterbarPositionStat_HR",50);
	splitterbarPositionSvr=ini.GetInt(L"SplitterbarPositionServer",75);
	if (splitterbarPositionSvr>90 || splitterbarPositionSvr<10)
		splitterbarPositionSvr=75;

	m_uTransferWnd1 = ini.GetInt(L"TransferWnd1",0);
	m_uTransferWnd2 = ini.GetInt(L"TransferWnd2",0);

	statsAverageMinutes=ini.GetInt(L"StatsAverageMinutes",5);
	MaxConperFive=ini.GetInt(L"MaxConnectionsPerFiveSeconds",GetDefaultMaxConperFive());

	reconnect = ini.GetBool(L"Reconnect", true);
	m_bUseServerPriorities = ini.GetBool(L"Scoresystem", true);
	m_bUseUserSortedServerList = ini.GetBool(L"UserSortedServerList", false);
	ICH = ini.GetBool(L"ICH", true);
	
	m_bPreventStandby = ini.GetBool(L"PreventStandby", false);
	m_bStoreSearches = ini.GetBool(L"StoreSearches", true);
	splashscreen=ini.GetBool(L"Splashscreen",true);
	bringtoforeground=ini.GetBool(L"BringToFront",true);
	beepOnError=ini.GetBool(L"BeepOnError",true);
	confirmExit=ini.GetBool(L"ConfirmExit",true);
	filterLANIPs=ini.GetBool(L"FilterBadIPs",true);
#ifdef _DEBUG
	m_bAllocLocalHostIP=ini.GetBool(L"AllowLocalHostIP",false);
#endif
	//autoconnect=ini.GetBool(L"Autoconnect",true);
	showRatesInTitle=ini.GetBool(L"ShowRatesOnTitle",true);

	startMinimized=ini.GetBool(L"StartupMinimized",false);
	m_bAutoStart=ini.GetBool(L"AutoStart",false);
	m_bRestoreLastMainWndDlg=ini.GetBool(L"RestoreLastMainWndDlg",true);
	m_iLastMainWndDlgID=ini.GetInt(L"LastMainWndDlgID",134);
	m_bRestoreLastLogPane=ini.GetBool(L"RestoreLastLogPane",false);
		m_iLastLogPaneID=ini.GetInt(L"LastLogPaneID",0);
	m_bSafeServerConnect =ini.GetBool(L"SafeServerConnect",false);

	m_bTransflstRemain =ini.GetBool(L"TransflstRemainOrder",false);
	filterserverbyip=ini.GetBool(L"FilterServersByIP",false);
	filterlevel=ini.GetInt(L"FilterLevel",127);
	checkDiskspace=ini.GetBool(L"CheckDiskspace",false);
	m_uMinFreeDiskSpace=ini.GetInt(L"MinFreeDiskSpace",20*1024*1024);
	m_bSparsePartFiles=ini.GetBool(L"SparsePartFiles",false);
	m_bResolveSharedShellLinks=ini.GetBool(L"ResolveSharedShellLinks",false);
	m_bKeepUnavailableFixedSharedDirs = ini.GetBool(L"KeepUnavailableFixedSharedDirs", false);
	m_strYourHostname=ini.GetString(L"YourHostname");

	// Barry - New properties...
	m_bAutoConnectToStaticServersOnly = ini.GetBool(L"AutoConnectStaticOnly",false); 
	autotakeed2klinks = ini.GetBool(L"AutoTakeED2KLinks",true); 
	addnewfilespaused = ini.GetBool(L"AddNewFilesPaused",false); 
	depth3D = ini.GetInt(L"3DDepth", 5);

	m_strDateTimeFormat = ini.GetString(L"DateTimeFormat", L"%A, %c");
	m_strDateTimeFormat4Log = ini.GetString(L"DateTimeFormat4Log", L"%c");
	m_strDateTimeFormat4Lists = ini.GetString(L"DateTimeFormat4Lists", L"%c");

	m_bSmartServerIdCheck = ini.GetBool(L"SmartIdCheck",true);
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
/*#else// X: [RDL] - [Remove Debug Log]
	// for normal release builds ensure that those options are all turned off
	m_iDebugServerTCPLevel = 0;
	m_iDebugServerUDPLevel = 0;
	m_iDebugServerSourcesLevel = 0;
	m_iDebugServerSearchesLevel = 0;
	m_iDebugClientTCPLevel = 0;
	m_iDebugClientUDPLevel = 0;
	m_iDebugClientKadUDPLevel = 0;
	m_iDebugSearchResultDetailLevel = 0;*/
#endif

	m_bpreviewprio=ini.GetBool(L"PreviewPrio",false);
	//m_bManualAddedServersHighPriority=ini.GetBool(L"ManualHighPrio",false);
	m_btransferfullchunks=ini.GetBool(L"FullChunkTransfers",true);
	m_istartnextfile=ini.GetInt(L"StartNextFile",0);
	moviePreviewBackup=ini.GetBool(L"VideoPreviewBackupped",true);
	m_iPreviewSmallBlocks=ini.GetInt(L"PreviewSmallBlocks", 0);

	m_bAllocFull=ini.GetBool(L"AllocateFullFile",0);
	//m_bShowUpDownIconInTaskbar = ini.GetBool(L"ShowUpDownIconInTaskbar", false );
	m_bShowWin7TaskbarGoodies  = ini.GetBool(L"ShowWin7TaskbarGoodies", true);

	// read file buffer size (with backward compatibility)
	/*m_uFileBufferSize=ini.GetInt(L"FileBufferSizePref",0); // old setting
	if (m_uFileBufferSize < 262144)
		m_uFileBufferSize = 1048576;
	else
		m_uFileBufferSize = ((m_uFileBufferSize*15000 + 512)/1024)*1024;
	m_uFileBufferSize=ini.GetInt(L"FileBufferSize",m_uFileBufferSize);*/

	// read queue size (with backward compatibility)
	m_iQueueSize=ini.GetInt(L"QueueSizePref",0); // old setting
	if (m_iQueueSize == 0)
		m_iQueueSize = 50*100;
	else
		m_iQueueSize = m_iQueueSize*100;
	m_iQueueSize=ini.GetInt(L"QueueSize",m_iQueueSize);

	m_iCommitFiles=ini.GetInt(L"CommitFiles", 1); // 1 = "commit" on application shut down; 2 = "commit" on each file saveing
	m_bDAP=ini.GetBool(L"DAPPref",true);
	m_bUAP=ini.GetBool(L"UAPPref",true);
	m_bCheckFileOpen=ini.GetBool(L"CheckFileOpen",true);
	watchclipboard=ini.GetBool(L"WatchClipboard4ED2kFilelinks",false);
	m_iSearchMethod=ini.GetInt(L"SearchMethod",2); //0->2 default kad

	showCatTabInfos=ini.GetBool(L"ShowInfoOnCatTabs",false);
//	resumeSameCat=ini.GetBool(L"ResumeNextFromSameCat",false);
	dontRecreateGraphs =ini.GetBool(L"DontRecreateStatGraphsOnResize",false);
	m_bExtControls =ini.GetBool(L"ShowExtControls",false);

	autofilenamecleanup=ini.GetBool(L"AutoFilenameCleanup",false);
	//networkkademlia=ini.GetBool(L"NetworkKademlia",true);
	networked2k=ini.GetBool(L"NetworkED2K",false);
	m_bRemove2bin=ini.GetBool(L"RemoveFilesToBin",true);
	m_bShowCopyEd2kLinkCmd=ini.GetBool(L"ShowCopyEd2kLinkCmd",true);
	
	m_strTxtEditor = ini.GetString(L"TxtEditor", L"notepad.exe");
	m_strVideoPlayer = ini.GetString(L"VideoPlayer");
	m_strVideoPlayerArgs = ini.GetString(L"VideoPlayerArgs");
	
	filenameCleanups=ini.GetStringLong(L"FilenameCleanups",L".com|.it|.net|.org|32|33|34|$|€|abc|arica|berlus|cache|cambi|cerco|dagna|destra|downl|dwn|emul|fasci|global|govern|gratis|http|kad|legge|munis|negr|politi|prima|priorit|razor|rinomin|ronzi|slot|soldi|tepay|terron|unipol|usate|veloc|visit|vota|wc|www.|zambor|albania|alema|comuni|dinero|donkey|duce|glion|inter|juve|ladium|milan|moggi|money|napoli|prodi|referend|roia|sinistra|ttana|unione|zapatero|mulo|mule");
	m_bAdjustNTFSDaylightFileTime=ini.GetBool(L"AdjustNTFSDaylightFileTime", true);
	m_bRearrangeKadSearchKeywords = ini.GetBool(L"RearrangeKadSearchKeywords", true);

	m_bUseSecureIdent=ini.GetBool(L"SecureIdent",true);
	m_bUseOldTimeRemaining= ini.GetBool(L"UseSimpleTimeRemainingcomputation",false);

	m_iStraightWindowStyles=ini.GetInt(L"StraightWindowStyles",0);
	m_bUseSystemFontForMainControls=ini.GetBool(L"UseSystemFontForMainControls",0);
	m_bRTLWindowsLayout = ini.GetBool(L"RTLWindowsLayout");

	LPBYTE pData = NULL;
	size_t uSize = sizeof m_lfHyperText;
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
//    m_bDynUpEnabled = ini.GetBool(L"USSEnabled", false);
//    m_bDynUpUseMillisecondPingTolerance = ini.GetBool(L"USSUseMillisecondPingTolerance", false);
//    m_iDynUpPingTolerance = ini.GetInt(L"USSPingTolerance", 500);
//	m_iDynUpPingToleranceMilliseconds = ini.GetInt(L"USSPingToleranceMilliseconds", 200);
//	if( minupload < 1 )
//		minupload = 1;
//	m_iDynUpGoingUpDivider = ini.GetInt(L"USSGoingUpDivider", 1000);
//    m_iDynUpGoingDownDivider = ini.GetInt(L"USSGoingDownDivider", 1000);
//    m_iDynUpNumberOfPings = ini.GetInt(L"USSNumberOfPings", 1);
	// ZZ:UploadSpeedSense <--

    //m_bA4AFSaveCpu = ini.GetBool(L"A4AFSaveCpu", false); // ZZ:DownloadManager
    m_bHighresTimer = ini.GetBool(L"HighresTimer", false);
	m_bRunAsUser = ini.GetBool(L"RunAsUnprivilegedUser", false);
	m_bPreferRestrictedOverUser = ini.GetBool(L"PreferRestrictedOverUser", false);
	m_bOpenPortsOnStartUp = ini.GetBool(L"OpenPortsOnStartUp", false);
	m_byLogLevel = ini.GetInt(L"DebugLogLevel", DLP_VERYLOW);
	m_bTrustEveryHash = ini.GetBool(L"AICHTrustEveryHash", false);
	m_bRememberCancelledFiles = ini.GetBool(L"RememberCancelledFiles", true);
	m_bRememberDownloadedFiles = ini.GetBool(L"RememberDownloadedFiles", true);
	m_bPartiallyPurgeOldKnownFiles = ini.GetBool(L"PartiallyPurgeOldKnownFiles", true);

	m_bWinaTransToolbar = ini.GetBool(L"WinaTransToolbar", true);
	m_bCryptLayerRequested = ini.GetBool(L"CryptLayerRequested", false);
	m_bCryptLayerRequired = ini.GetBool(L"CryptLayerRequired", false);
	m_bCryptLayerSupported = ini.GetBool(L"CryptLayerSupported", true);
	m_dwKadUDPKey = ini.GetInt(L"KadUDPKey", t_rng->getUInt32());
	m_byCryptTCPPaddingLength = (uint8)ini.GetInt(L"CryptTCPPaddingLength", 128);
	if(m_byCryptTCPPaddingLength<10)
		m_byCryptTCPPaddingLength=10;
	else if(m_byCryptTCPPaddingLength>254)
		m_byCryptTCPPaddingLength=254;

	m_bEnableSearchResultFilter = ini.GetBool(L"EnableSearchResultSpamFilter", true);

	m_eFileSizeFormat = (EFileSizeFormat)ini.GetInt(_T("SearchResultsFileSizeFormat"), fsizeDefault);
	///////////////////////////////////////////////////////////////////////////
	// Section: "Proxy"
	//
	proxy.EnablePassword = ini.GetBool(L"ProxyEnablePassword",false,L"Proxy");
	proxy.UseProxy = ini.GetBool(L"ProxyEnableProxy",false);
	proxy.name = CStringA(ini.GetString(L"ProxyName"));
	proxy.user = CStringA(ini.GetString(L"ProxyUser"));
	proxy.password = CStringA(ini.GetString(L"ProxyPassword"));
	proxy.port = (uint16)ini.GetInt(L"ProxyPort",1080);
	proxy.type = (uint16)ini.GetInt(L"ProxyType",PROXYTYPE_NOPROXY);


	///////////////////////////////////////////////////////////////////////////
	// Section: "Statistics"
	//
	statsSaveInterval = ini.GetInt(L"SaveInterval", 60, L"Statistics");
	m_strStatsExpandedTreeItems = ini.GetString(L"statsExpandedTreeItems",L"111000000100000110000010000011110000010010");
	CString buffer2;
	for (size_t i = 0; i < _countof(m_adwStatsColors); i++) {
		buffer2.Format(L"StatColor%i", i);
		m_adwStatsColors[i] = 0;
		if (_stscanf_s(ini.GetString(buffer2), L"%i", &m_adwStatsColors[i]) != 1)
			ResetStatsColor(i);
	}
	bHasCustomTaskIconColor = ini.GetBool(L"HasCustomTaskIconColor", false);
	m_bShowVerticalHourMarkers = ini.GetBool(L"ShowVerticalHourMarkers", true);



	///////////////////////////////////////////////////////////////////////////
	// Section: "UPnP"
	//
	//zz_fly :: keep compatibility :: start
	/*
	m_bEnableUPnP = ini.GetBool(L"EnableUPnP", false, L"UPnP");
	*/
	bool tmpEnableUPnP = ini.GetBool(L"UPnPNAT", false, L"UPnP");
	m_bEnableUPnP = ini.GetBool(L"EnableUPnP", tmpEnableUPnP);
	//zz_fly :: end
	m_bSkipWANIPSetup = ini.GetBool(L"SkipWANIPSetup", false);
	m_bSkipWANPPPSetup = ini.GetBool(L"SkipWANPPPSetup", false);
	m_bCloseUPnPOnExit = ini.GetBool(L"CloseUPnPOnExit", true);
	m_nLastWorkingImpl = ini.GetInt(L"LastWorkingImplementation", 1 /*MiniUPnPLib*/);
	//m_bIsWinServImplDisabled = ini.GetBool(L"DisableWinServImpl", false);
	m_bIsMinilibImplDisabled = ini.GetBool(L"DisableMiniUPNPLibImpl", false);
	m_bIsACATImplDisabled = ini.GetBool(L"DisableACATUPNPImpl", false);
	m_bUPnPTryRandom = ini.GetBool(L"UPnPNAT_TryRandom", false);
	m_bUPnPRebindOnIPChange = ini.GetBool(L"UPnPNAT_RebindOnIPChange", false); //zz_fly :: Rebind UPnP on IP-change

	LoadCats();
	//SetLanguage(); //Xman done above

	//--------------------------------------------------------------------------
	//Xman Xtreme Mod:


	//Xman Xtreme Upload
	m_slotspeed=ini.GetFloat(L"uploadslotspeed",3.4f, L"Xtreme");
	//CheckSlotSpeed();

	if(m_slotspeed<1.5f)
		m_slotspeed=1.5f;
	else
	{
		float maxSlotSpeed=GetMaxSlotSpeed(maxupload);

		if(m_slotspeed>maxSlotSpeed)
			m_slotspeed=maxSlotSpeed;
	}

	m_openmoreslots=ini.GetBool(L"openmoreslots",true);
	m_bandwidthnotreachedslots=ini.GetBool(L"bandwidthnotreachedslots",false);
	// X: [DSRB] - [Dynamic Send and Receive Buffer]
	/*m_sendbuffersize=ini.GetInt(L"sendbuffersize", 8192);
	switch (m_sendbuffersize)
	{
	case 6000:
	case 8192:
	case 12000:
	case 24000: //zz_fly :: support 24k send buffer
		break;
	default:
		m_sendbuffersize=8192;
		break;
	}
*/
	retryconnectionattempts=ini.GetBool(L"retryconnectionattempts",true);

        //Xman show additional graph lines
	m_bShowAdditionalGraph=ini.GetBool(L"ShowAdditionalGraph", false);

	// Maella -MTU Configuration-
	MTU=(uint16)ini.GetInt(L"MTU",1340);
	if (MTU<500)
		MTU=500;
	if (MTU>1500)
		MTU=1500;
	// Maella end

	usedoublesendsize=ini.GetBool(L"usedoublesendsize",false);
	// netfinity: Maximum Segment Size (MSS - Vista only) //added by zz_fly
	retrieveMTUFromSocket = ini.GetBool(L"retrieveMTUFromSocket", true);
	if(retrieveMTUFromSocket && GetWindowsVersion() < _WINVER_VISTA_)
		retrieveMTUFromSocket = false;

	// Maella -Network Adapter Feedback Control-
	NAFCFullControl=ini.GetBool(L"NAFCFullControl", true);
	forceNAFCadapter=ini.GetInt(L"ForceNAFCAdapter",0);
	// Maella end

	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	datarateSamples=(uint8)ini.GetInt(L"DatarateSamples", 10);
	if(datarateSamples < 1) datarateSamples = 1;
	if(datarateSamples > 20) datarateSamples = 20;
	// Maella end

	// Maella -One-queue-per-file- (idea bloodymad)
	enableMultiQueue=ini.GetBool(L"EnableMultiQueue", true);
	// Maella end

	//Xman always one release-slot
	//m_onerealeseslot=ini.GetBool(L"onerealeseslot",true);
	//Xman end

	//Xman advanced upload-priority
	m_AdvancedAutoPrio=ini.GetBool(L"AdvancedAutoPrio",true);
	//Xman end

	//Xman chunk chooser
	m_chunkchooser=(uint8)ini.GetInt(L"chunkchooser",1); // 1 = Maella 2=zz
	if(m_chunkchooser!=1 && m_chunkchooser!=2)
		m_chunkchooser=1;
	//Xman end

	//Xman disable compression
	m_bUseCompression=ini.GetBool(L"UseCompression",true);

	//Xman auto update IPFilter
	m_strautoupdateipfilter_url= ini.GetString(L"AutoUpdateIPFilter_URL", _T("http://upd.emule-security.org/ipfilter.zip"));
	m_bautoupdateipfilter= ini.GetBool(L"AutoUpdateIPFilter", false);
	LPBYTE pst = NULL;
	size_t usize = sizeof m_IPfilterVersion;
	if (ini.GetBinary(L"IPfilterVersion", &pst, &usize) && usize == sizeof m_IPfilterVersion)
		memcpy(&m_IPfilterVersion, pst, sizeof m_IPfilterVersion);
	else
		memset(&m_IPfilterVersion, 0, sizeof m_IPfilterVersion);
	delete[] pst;
	m_last_ipfilter_check= ini.GetUInt64(L"lastipfiltercheck", 0);
	//Xman end

	//Xman count block/success send
	m_showblockratio=ini.GetBool(L"ShowSocketBlockRatio", true);
	m_dropblockingsockets=ini.GetBool(L"DropBlockingSockets", true);

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

	m_bKnown2Buffer = ini.GetBool(L"EnableKnown2Buffer", false); //zz_fly :: known2 buffer
	//Xman end
	//--------------------------------------------------------------------------
	m_bEnable64BitTime = ini.GetBool(L"64BitTime", false, L"Xtreme+");// X: [E64T] - [Enable64BitTime]
	refuseupload=ini.GetBool(L"RefuseUpload",false);// X: [RU] - [RefuseUpload]
	rumax=ini.GetInt(L"RUMax",256000);// X: [RU] - [RefuseUpload]
	m_uGlobalBufferSize=ini.GetInt(L"GlobalBufferSize", 8388608);// X: [GB] - [Global Buffer]
	/*if (m_uGlobalBufferSize < m_uFileBufferSize * 2 || m_uGlobalBufferSize > 1073741824)
		m_uGlobalBufferSize = m_uFileBufferSize * 4;*/


	dontcompressext=ini.GetBool(L"DontCompressExt",true);// X: [DCE] - [DontCompressExt]
	compressExt=ini.GetString(L"CompressExt",L".zip|.rar|.7z|.ace|.ogm|.cbz|.cbr|.ape|.flac|.mkv|.mp4");// X: [DCE] - [DontCompressExt]
	compressExt.MakeLower();
	m_bPauseOnFileComplete=ini.GetBool(L"PauseOnFileComplete",true); // NEO: POFC - [PauseOnFileComplete]
	prefReadonly = ini.GetBool(L"ReadOnly", false); // X: [ROP] - [ReadOnlyPreference]
	isshowtoolbar=ini.GetBool(L"ShowToolbar",true);
	isshowcatbar=ini.GetBool(L"ShowCatbar",false);
	m_bDisableHistoryList=ini.GetBool(L"DisableHistoryList",false);
	showShareableFile=ini.GetBool(L"ShowShareableFile",false);
	queryOnHashing = ini.GetInt(L"QueryOnHashing", 0); // X: [QOH] - [QueryOnHashing]
	if(queryOnHashing>2)
		queryOnHashing=0;
	lastTranWndCatID = ini.GetInt(L"LastCatID", 0); // X: [RCI] - [Remember Catalog ID]
	if(lastTranWndCatID > (thePrefs.GetCatCount()-1) || lastTranWndCatID<0) // X: [RCI] - [Remember Catalog ID]
		lastTranWndCatID = 0;
	dontsharext=ini.GetBool(L"DontShareExt",true);// X: [DSE] - [DontShareExt]
	shareExt=ini.GetString(L"ShareExt",L".bt!|.jc!|.bc!|.td|.part|.met|.bak|.txtsrc");// X: [DSE] - [DontShareExt]
	shareExt.MakeLower();
	showSpeedGraph=ini.GetBool(L"ShowSpeedGraph",true); // X: [SGW] - [SpeedGraphWnd]
	speedgraphwindowLeft=ini.GetInt(L"SpeedGraphX",90);
	speedgraphwindowTop=ini.GetInt(L"SpeedGraphY",10);
	randomPortOnStartup=ini.GetBool(L"randomPortOnStartup",false); // X: [RPOS] - [RandomPortOnStartup]
    notifier=ini.GetBool(L"Notifier",true);

	// ==> Invisible Mode [TPT/MoNKi] - Stulle
    m_bInvisibleMode = ini.GetBool(_T("InvisibleMode"), false);
	m_iInvisibleModeHotKeyModifier = ini.GetInt(_T("InvisibleModeHKKeyModifier"), MOD_CONTROL | MOD_SHIFT | MOD_ALT);
	m_cInvisibleModeHotKey = (char)ini.GetInt(_T("InvisibleModeHKKey"),(int)'E');
    SetInvisibleMode(m_bInvisibleMode  ,m_iInvisibleModeHotKeyModifier ,m_cInvisibleModeHotKey );
	m_bInvisibleModeStart = ini.GetBool(_T("InvisibleModeStart"), false);
	// <== Invisible Mode [TPT/MoNKi] - Stulle

//morph4u + ////////////////////////////////////////////////
	_stprintf(m_strServerMetUpdateURL, L"%s", ini.GetString(L"UpdateURLServerMet", L"http://upd.emule-security.org/server.met"));
	_stprintf(m_strNodesDatUpdateURL, L"%s", ini.GetString(L"UpdateURLNodesDat", L"http://upd.emule-security.org/nodes.dat"));
    m_bGridlines = ini.GetBool(L"GridLines", false);
	m_bShowProgressBar = ini.GetBool(L"ShowProgressBar", true); //morph4u :: PercentBar
	m_bShutDownMule = ini.GetBool(L"ShutDownMule", false); 
	m_bShutDownPC = ini.GetBool(L"ShutDownPC", false); 
// ==> Pay Back First
	m_bPayBackFirst=ini.GetBool(_T("IsPayBackFirst"), true);
// <== Pay Back First
	m_bBufferDisplay = ini.GetBool(L"BufferDisplay", false);
//morph4u - ////////////////////////////////////////////////
}
/*
//Xman Xtreme Upload
void CPreferences::CheckSlotSpeed()
{
	if(m_slotspeed<1.5f)
		m_slotspeed=1.5f;
	else
	{
		float maxSlotSpeed=GetMaxSlotSpeed(maxupload);

	if(m_slotspeed>maxSlotSpeed)
		m_slotspeed=maxSlotSpeed;
}
//Xman end
*/
WORD CPreferences::InitWinVersion(){
	return (m_wWinVer = DetectWinVersion());	
}

UINT CPreferences::GetDefaultMaxConperFive(){
	switch (GetWindowsVersion()){
		/*case _WINVER_98_:
			return 5;
		case _WINVER_95_:	
		case _WINVER_ME_:
			return MAXCON5WIN9X;*/
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
	CIni ini(strCatIniFilePath, L"General"); // X-Ray :: eMulePlusIniClass
	ini.WriteInt(L"Count", (int)(catMap.GetCount() - 1));
	for (size_t i = 0; i < catMap.GetCount(); i++)
	{
		CString strSection;
		strSection.Format(L"Cat#%i", i);
		ini.SetSection(strSection);

		ini.WriteStringUTF8(L"Title", catMap.GetAt(i)->strTitle);
		ini.WriteStringUTF8(L"Temp", catMap.GetAt(i)->strTempPath);// X: [TD] - [TempDir]
		ini.WriteStringUTF8(L"Incoming", catMap.GetAt(i)->strIncomingPath);
		ini.WriteStringUTF8(L"Comment", catMap.GetAt(i)->strComment);
		ini.WriteStringUTF8(L"RegularExpression", catMap.GetAt(i)->regexp);
		ini.WriteInt(L"Color", catMap.GetAt(i)->color);
		ini.WriteInt(L"a4afPriority", catMap.GetAt(i)->prio); // ZZ:DownloadManager
		ini.WriteStringUTF8(L"AutoCat", catMap.GetAt(i)->autocat);
		ini.WriteInt(L"Filter", (int)catMap.GetAt(i)->filter);
		ini.WriteBool(L"FilterNegator", catMap.GetAt(i)->filterNeg);
		ini.WriteBool(L"AutoCatAsRegularExpression", catMap.GetAt(i)->ac_regexpeval);
        ini.WriteBool(L"downloadInAlphabeticalOrder", catMap.GetAt(i)->downloadInAlphabeticalOrder!=FALSE);
		ini.WriteBool(L"Care4All", catMap.GetAt(i)->care4all);
	}
}

void CPreferences::LoadCats()
{
	CString strCatIniFilePath;
	strCatIniFilePath.Format(L"%sCategory.ini", GetMuleDirectory(EMULE_CONFIGDIR));
	CIni ini(strCatIniFilePath, L"General"); // X-Ray :: eMulePlusIniClass
	int iNumCategories = ini.GetInt(L"Count", 0);
	for (int i = 0; i <= iNumCategories; i++)
	{
		CString strSection;
		strSection.Format(L"Cat#%i", i);
		ini.SetSection(strSection);

		Category_Struct* newcat = new Category_Struct;
		newcat->filter = 0;
		if (i != 0) // All category
		{
		newcat->strTitle = ini.GetStringUTF8(L"Title");
		newcat->strTempPath = ini.GetStringUTF8(L"Temp");// X: [TD] - [TempDir]
		newcat->strIncomingPath = ini.GetStringUTF8(L"Incoming");
		MakeFoldername(newcat->strIncomingPath);
		if (
			// SLUGFILLER: SafeHash remove - removed installation dir unsharing
			/*
			!IsShareableDirectory(newcat->strIncomingPath) || 
			*/
		(!PathFileExists(newcat->strIncomingPath) && !::CreateDirectory(newcat->strIncomingPath, 0)))
		{
			newcat->strIncomingPath = GetMuleDirectory(EMULE_INCOMINGDIR);
			MakeFoldername(newcat->strIncomingPath);
		}
			newcat->care4all = ini.GetBool(L"Care4All", FALSE);
		}
		else{
			newcat->strTitle = GetResString(IDS_ALL);// X: [UIC] - [UIChange] change cat0 Title
			newcat->strTempPath.Empty();// X: [TD] - [TempDir]
			newcat->strIncomingPath = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
			newcat->care4all = true;
		}
		newcat->strComment = ini.GetStringUTF8(L"Comment");
		newcat->prio = ini.GetInt(L"a4afPriority", PR_NORMAL); // ZZ:DownloadManager
		newcat->filter = ini.GetInt(L"Filter", 0);
		newcat->filterNeg = ini.GetBool(L"FilterNegator", FALSE);
		newcat->ac_regexpeval = ini.GetBool(L"AutoCatAsRegularExpression", FALSE);
		newcat->regexp = ini.GetStringUTF8(L"RegularExpression");
		newcat->autocat = ini.GetStringUTF8(L"AutoCat");
        newcat->downloadInAlphabeticalOrder = ini.GetBool(L"downloadInAlphabeticalOrder", FALSE); // ZZ:DownloadManager
		newcat->color = ini.GetInt(L"Color", (DWORD)-1 );
		AddCat(newcat);
	}
}

void CPreferences::RemoveCat(size_t index)
{
	if (index < catMap.GetCount())
	{
		Category_Struct* delcat = catMap.GetAt(index); 
		catMap.RemoveAt(index);
		delete delcat;
	}
}

bool CPreferences::SetCatFilter(size_t index, sint_ptr filter)
{
	if (index < catMap.GetCount())
	{
		catMap.GetAt(index)->filter = filter;
		return true;
	}
	return false;
}

size_t CPreferences::GetCatFilter(size_t index)
{
	if (index < catMap.GetCount())
		return catMap.GetAt(index)->filter;
    return 0;
}

bool CPreferences::GetCatFilterNeg(size_t index)
{
	if (index < catMap.GetCount())
		return catMap.GetAt(index)->filterNeg;
    return false;
}

void CPreferences::SetCatFilterNeg(size_t index, bool val)
{
	if (index < catMap.GetCount())
		catMap.GetAt(index)->filterNeg = val;
}

bool CPreferences::MoveCat(size_t from, size_t to)
{
	if (from >= catMap.GetCount() || to >= catMap.GetCount() + 1 || from == to)
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


DWORD CPreferences::GetCatColor(size_t index, int nDefault) {
	if (index<catMap.GetCount()) {
		DWORD c=catMap.GetAt(index)->color;
		if (c!=(DWORD)-1)
			return catMap.GetAt(index)->color; 
	}

	return GetSysColor(nDefault);
}


///////////////////////////////////////////////////////
// SLUGFILLER: SafeHash remove - global form of IsInstallationDirectory unnececery
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
	if (!CompareDirectories(strFullPath, GetMuleDirectory(EMULE_INSTLANGDIR)))
		return true;

	return false;
}
*/

// SLUGFILLER: SafeHash remove - global form of IsShareableDirectory unnececery
/*
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
	for (size_t i=0;i<GetTempDirCount();i++)
		if (!CompareDirectories(strFullPath, GetTempDir(i)))			// ".\eMule\temp"
			return false;

	return true;
}
*/

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

bool CPreferences::IsDefaultNick(const CString strCheck){
	// not fast, but this function is called often
	for (int i = 0; i != 255; i++){
		if (GetHomepageBaseURLForLevel(i) == strCheck)
			return true;
	}
	return ( strCheck == L"http://emule-project.net" );
}

/*void CPreferences::SetUserNick(LPCTSTR pszNick)
{
	strNick = pszNick;
}*/

UINT CPreferences::GetWebMirrorAlertLevel(){
	// Known upcoming DDoS Attacks
	if (m_nWebMirrorAlertLevel == 0){
		// no threats known at this time
	}
	// end
		return 0;
}

bool CPreferences::IsRunAsUserEnabled(){
	return (thePrefs.GetWindowsVersion() == _WINVER_XP_ || thePrefs.GetWindowsVersion() == _WINVER_2K_ || thePrefs.GetWindowsVersion() == _WINVER_2003_) 
		&& m_bRunAsUser
		&& m_nCurrentUserDirMode == 2;
}

//Xman
/*
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

bool CPreferences::CanFSHandleLargeFiles(size_t cat)	{ // X: [TD] - [TempDir]
	Category_Struct* category = GetCategory(cat);
	if(g_VolumeInfo.GetVolumeInfoByPath(category->strIncomingPath)->IsFAT()) // X: [FSFS] - [FileSystemFeaturesSupport]
		return false;
	if(category->strTempPath.IsEmpty()){
		for (size_t i = 0; i != tempdir.GetCount(); i++){
			if (!g_VolumeInfo.GetVolumeInfoByPath(tempdir.GetAt(i))->IsFAT())
			return true;
	}
	}
	else if (!g_VolumeInfo.GetVolumeInfoByPath(category->strTempPath)->IsFAT())
		return true;
	return false;
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
	SFMT&rng = *t_rng;
	do {
		// Get random port
		nPort = 4096 + (rng.getUInt16() % uValidPortRange);

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
	SFMT&rng = *t_rng;
	do {
		// Get random port
		nPort = 4096 + (rng.getUInt16() % uValidPortRange);

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
		bool bConfigAvailableExecuteable = _taccess(strSelectedConfigBaseDirectory + CONFIGFOLDER + _T("preferences.ini"), 0) == 0;
		
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
			if (GetWindowsVersion() >= _WINVER_VISTA_){
			HMODULE hShell32 = LoadLibrary(_T("shell32.dll"));
			if (hShell32){
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
							if (strLocalAppData.Right(1) != _T('\\'))
								strLocalAppData += _T('\\');
							if (strPersonalDownloads.Right(1) != _T('\\'))
								strPersonalDownloads += _T('\\');
							if (strPublicDownloads.Right(1) != _T('\\'))
								strPublicDownloads += _T('\\');
							if (strProgrammData.Right(1) != _T('\\'))
								strProgrammData += _T('\\');

							if (nRegistrySetting == -1){
								// no registry default, check if we find a preferences.ini to use
								bool bRes =  _taccess(strLocalAppData + _T("eMule\\") + CONFIGFOLDER + _T("preferences.ini"), 0) == 0;
								if (bRes)
									m_nCurrentUserDirMode = 0;
								else{
									bRes =  _taccess(strProgrammData + _T("eMule\\") + CONFIGFOLDER + _T("preferences.ini"), 0) == 0;
									if (bRes)
										m_nCurrentUserDirMode = 1;
									else if (bConfigAvailableExecuteable)
										m_nCurrentUserDirMode = 2;
									else
										m_nCurrentUserDirMode = 0; // no preferences.ini found, use the default
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
					FreeLibrary(hShell32);
				}
				else{
					DebugLogError(_T("Unable to load shell32.dll to retrieve the systemfolder locations, using fallbacks"));
					ASSERT( false );
				}
				}
			else { // GetWindowsVersion() >= _WINVER_VISTA_

					CString strAppData = ShellGetFolderPath(CSIDL_APPDATA);
					CString strPersonal = ShellGetFolderPath(CSIDL_PERSONAL);
					if (!strAppData.IsEmpty() && !strPersonal.IsEmpty())
					{
						if (strAppData.GetLength() < MAX_PATH - 30 && strPersonal.GetLength() < MAX_PATH - 40){
							if (strPersonal.Right(1) != _T('\\'))
								strPersonal += _T('\\');
							if (strAppData.Right(1) != _T('\\'))
								strAppData += _T('\\');
							if (nRegistrySetting == 0){
								// registry setting overwrites, use these folders
								strSelectedDataBaseDirectory = strPersonal + _T("eMule Downloads\\");
								strSelectedConfigBaseDirectory = strAppData + _T("eMule\\");
								m_nCurrentUserDirMode = 0;
								// strSelectedExpansionBaseDirectory stays default
							}
							else if (nRegistrySetting == -1 && !bConfigAvailableExecuteable){
							if (_taccess(strAppData + _T("eMule\\") + CONFIGFOLDER + _T("preferences.ini"), 0) == 0){
									// preferences.ini found, so we use this as default
									strSelectedDataBaseDirectory = strPersonal + _T("eMule Downloads\\");
									strSelectedConfigBaseDirectory = strAppData + _T("eMule\\");
									m_nCurrentUserDirMode = 0;
								}
							}
							else
								ASSERT( false );
						}
						else
							ASSERT( false );
					}
				}
		}

		// the use of ending backslashes is inconsitent, would need a rework throughout the code to fix this
		m_astrDefaultDirs[EMULE_CONFIGDIR] = strSelectedConfigBaseDirectory + CONFIGFOLDER;
		m_astrDefaultDirs[EMULE_TEMPDIR] = strSelectedDataBaseDirectory + _T("Temp");
		m_astrDefaultDirs[EMULE_INCOMINGDIR] = strSelectedDataBaseDirectory + _T("Incoming");
		m_astrDefaultDirs[EMULE_LOGDIR] = strSelectedConfigBaseDirectory + _T("logs\\");
		m_astrDefaultDirs[EMULE_ADDLANGDIR] = strSelectedExpansionBaseDirectory + _T("lang\\");
		m_astrDefaultDirs[EMULE_INSTLANGDIR] = m_astrDefaultDirs[EMULE_EXECUTEABLEDIR] + _T("lang\\");
		m_astrDefaultDirs[EMULE_DATABASEDIR] = strSelectedDataBaseDirectory; // has ending backslashes
		m_astrDefaultDirs[EMULE_CONFIGBASEDIR] = strSelectedConfigBaseDirectory; // has ending backslashes
		//                EMULE_EXECUTEABLEDIR
		m_astrDefaultDirs[EMULE_EXPANSIONDIR] = strSelectedExpansionBaseDirectory; // has ending backslashes

		/*CString strDebug;
		for (int i = 0; i < 10; i++)
			strDebug += m_astrDefaultDirs[i] + _T('\n');
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
				::CreateDirectory(m_astrDefaultDirs[EMULE_EXPANSIONDIR], NULL);
				break;
		}
		::CreateDirectory(m_astrDefaultDirs[eDirectory], NULL);
		m_abDefaultDirsCreated[eDirectory] = true;
	}
	return m_astrDefaultDirs[eDirectory];
}

CString	CPreferences::GetMuleDirectory(EDefaultDirectory eDirectory, bool bCreate){
	switch (eDirectory){
		case EMULE_INCOMINGDIR:
			return m_strIncomingDir;
		case EMULE_TEMPDIR:
			ASSERT( false ); // use GetTempDir() instead! This function can only return the first tempdirectory
			return GetTempDir(0);
		default:
			return GetDefaultDirectory(eDirectory, bCreate);
	}
}
/*
void CPreferences::SetMuleDirectory(EDefaultDirectory eDirectory, CString strNewDir){
	switch (eDirectory){
		case EMULE_INCOMINGDIR:
			m_strIncomingDir = strNewDir;
			break;
		default:
			ASSERT( false );
			break;
	}
}*/

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

//Xman NAFC -> Statisticgraph
void CPreferences::SetNAFCFullControl(bool flag)
{
	if(NAFCFullControl!=flag)
	{
		NAFCFullControl = flag;
		if(theApp.emuledlg && theApp.emuledlg->m_hWnd && theApp.m_app_state == APP_STATE_RUNNING)
		theApp.emuledlg->statisticswnd->RepaintMeters();
	}
}
//Xman end

// ==> Invisible Mode [TPT/MoNKi] - Stulle
void CPreferences::SetInvisibleMode(bool on, int keymodifier, char key) 
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
// <== Invisible Mode [TPT/MoNKi] - Stulle
void CPreferences::SetMaxUpload(float in)
{
	float oldmaxSlotSpeed = GetMaxSlotSpeed(maxupload);
	float maxSlotSpeed = GetMaxSlotSpeed(in);
	if(maxSlotSpeed != oldmaxSlotSpeed)
	{
		m_slotspeed = maxSlotSpeed * m_slotspeed / oldmaxSlotSpeed;
		if(m_slotspeed < 1.5f)
			m_slotspeed = 1.5f;
	}
	maxupload = in;
}