//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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

class CSafeMemFile;
class CSearchFile;
class CUpDownClient;
class CServer;
class CPartFile;
class CSharedFileList;
class CKnownFile;
// NEO: FCFG - [FileConfiguration] -- Xanatos -->
class CPartPreferences; 
class CKnownPreferences; 
// NEO: FCFG END <-- Xanatos --
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
class ThrottledControlSocket;
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
class CFileDataIO;
#endif // NEO_SS // NEO: NSS END <-- Xanatos --
// NEO: XUC - [ExtendedUdpCache] -- Xanatos -->
struct SServer;
#include "MapKey.h"
// NEO: XUC END <-- Xanatos --
struct SUnresolvedHostname;


namespace Kademlia 
{
	class CUInt128;
};

class CSourceHostnameResolveWnd : public CWnd
{
// Construction
public:
	CSourceHostnameResolveWnd();
	virtual ~CSourceHostnameResolveWnd();

	void AddToResolve(const uchar* fileid, LPCSTR pszHostname, uint16 port, LPCTSTR pszURL = NULL);

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnHostnameResolved(WPARAM wParam, LPARAM lParam);

private:
	struct Hostname_Entry {
		uchar fileid[16];
		CStringA strHostname;
		uint16 port;
		CString strURL;
	};
	CTypedPtrList<CPtrList, Hostname_Entry*> m_toresolve;
	char m_aucHostnameBuffer[MAXGETHOSTSTRUCT];
};


class CDownloadQueue
{
	friend class CAddFileThread;
	friend class CServerSocket;

public:
	CDownloadQueue();
	~CDownloadQueue();

	void	Process();
	void	CalculateDownloadRate(); // NEO: ASM - [AccurateSpeedMeasure] <-- Xanatos --
	void	Init();
	void	Init(CString tempPath); // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
	void	LoadSources();
	void	SaveSources();
#endif // NEO_SS // NEO: NSS END <-- Xanatos --

	// add/remove entries
	void	AddPartFilesToShare();
	void	AddDownload(CPartFile* newfile, bool paused);
	void	AddSearchToDownload(CSearchFile* toadd, uint8 paused = 2, int cat = 0);
	void	AddSearchToDownload(CString link, uint8 paused = 2, int cat = 0);
	void	AddFileLinkToDownload(class CED2KFileLink* pLink, int cat = 0);
	void	RemoveFile(CPartFile* toremove);
	void	DeleteAll();

	int		GetFileCount() const { return filelist.GetCount(); }
	CTypedPtrList<CPtrList, CPartFile*>* GetFileList() { return &filelist; } // NEO: MOD - [GetFileList] <-- Xanatos --
	UINT	GetDownloadingFileCount() const;
	UINT	GetPausedFileCount() const;

	bool	IsFileExisting(const uchar* fileid, bool bLogWarnings = true) const;
	bool	IsPartFile(const CKnownFile* file) const;
	bool	IsTempFile(const CString& rstrDirectory, const CString& rstrName) const; // SLUGFILLER: SafeHash // NEO: SSH - [SlugFillerSafeHash] <-- Xanatos --

	CPartFile* GetFileByID(const uchar* filehash) const;
	CPartFile* GetFileByIndex(int index) const;
	CPartFile* GetFileByKadFileSearchID(uint32 ID) const;

    void    StartNextFileIfPrefs(int cat);
	//void	StartNextFile(int cat=-1,bool force=false);

	void	RefilterAllComments();	
	// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
	bool	StartNextFile(int cat=-1,bool force=false);
	void	StopPauseLastFile(int Mode = 0, int Category = -1);
	int		GetMaxCatResumeOrder(int iCategory = 0);
	UINT	GetCategoryFileCount(int iCategory);
	UINT	GetHighestAvailableSourceCount(int nCat = -1);
	int		GetAutoCat(CString sFullName, EMFileSize nFileSize);
	bool	ApplyFilterMask(CString sFullName, int nCat);
	void	StartDLInEmptyCats(int useCat);
	// NEO: NXC END <-- Xanatos --

	// sources
	CUpDownClient* GetDownloadClientByIP(uint32 dwIP);
	CUpDownClient* GetDownloadClientByIP_UDP(uint32 dwIP, uint16 nUDPPort, bool bIgnorePortOnUniqueIP, bool* pbMultipleIPs = NULL);
	bool	IsInList(const CUpDownClient* client) const;

	bool    CheckAndAddSource(CPartFile* sender,CUpDownClient* source);
	bool    CheckAndAddKnownSource(CPartFile* sender,CUpDownClient* source, bool bIgnoreGlobDeadList = false);
	bool	RemoveSource(CUpDownClient* toremove, bool bDoStatsUpdate = true);

	// statistics
	typedef struct{
		int	a[31]; // NEO: MOD - [SDownloadStats] <-- Xanatos --
	} SDownloadStats;
	void	GetDownloadSourcesStats(SDownloadStats& results);
	int		GetDownloadFilesStats(uint64 &ui64TotalFileSize, uint64 &ui64TotalLeftToTransfer, uint64 &ui64TotalAdditionalNeededSpace);
	uint32	GetDatarate() {return datarate;}

	void	AddUDPFileReasks()								{m_nUDPFileReasks++;}
	uint32	GetUDPFileReasks() const						{return m_nUDPFileReasks;}
	void	AddFailedUDPFileReasks()						{m_nFailedUDPFileReasks++;}
	uint32	GetFailedUDPFileReasks() const					{return m_nFailedUDPFileReasks;}

	// categories
	void	ResetCatParts(UINT cat);
	void	SetCatPrio(UINT cat, uint8 newprio);
    void    RemoveAutoPrioInCat(UINT cat, uint8 newprio); // ZZ:DownloadManager
	void	SetCatStatus(UINT cat, int newstatus);
	void	MoveCat(UINT from, UINT to);
	//void	SetAutoCat(CPartFile* newfile); // NEO: NXC - [NewExtendedCategories] <-- Xanatos --

	// searching on local server
	void	SendLocalSrcRequest(CPartFile* sender);
	void	RemoveLocalServerRequest(CPartFile* pFile);
	void	ResetLocalServerRequests();

	// searching in Kad
	void	SetLastKademliaFileRequest()				{lastkademliafilerequest = ::GetTickCount();}
	bool	DoKademliaFileRequest();
	void	KademliaSearchFile(uint32 searchID, const Kademlia::CUInt128* pcontactID, const Kademlia::CUInt128* pkadID, uint8 type, uint32 ip, uint16 tcp, uint16 udp, uint32 dwBuddyIP, uint16 dwBuddyPort, uint8 byCryptOptions);

	// searching on global servers
	void	StopUDPRequests();

	// check diskspace
	void	SortByPriority();
	void	CheckDiskspace(bool bNotEnoughSpaceLeft = false);
	void	CheckDiskspaceTimed();

	void	ExportPartMetFilesOverview() const;
	void	OnConnectionState(bool bConnected);

	void	AddToResolved( CPartFile* pFile, SUnresolvedHostname* pUH );

	CString GetOptimalTempDir(UINT nCat, EMFileSize nFileSize);

#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
	void	ProcessReceiving();

	void	AddToProcessQueue(ThrottledControlSocket* sock);
	void	RemoveFromProcessQueue(ThrottledControlSocket* sock);

	void	ReSortDownloadSlots(CUpDownClient* client);
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
	uint32 	GetDownloadQueueLength(){return numberOfDownloading;}
	void 	IncreaseDownloadQueueLength(){numberOfDownloading++;}
	void 	DecreaseDownloadQueueLength(){if(numberOfDownloading) numberOfDownloading--;}
#endif // NEO_BC // NEO: NBC END <-- Xanatos --

	// NEO: QS - [QuickStart] -- Xanatos -->
	void	ProcessQuickStart();
	void	DoQuickStart();
	void	StopQuickStart();
	// NEO: QS END <-- Xanatos --

	// NEO: FCFG - [FileConfiguration] -- Xanatos -->
	void	UpdatePartPrefs(CPartPreferences* PartPrefs, CKnownPreferences* KnownPrefs, UINT cat);
	static void UpdatePartPrefs(CPartFile* cur_file, CPartPreferences* PartPrefs, UINT cat);
	// NEO: FCFG END <-- Xanatos --
	
	// NEO: GSL - [GlobalSourceLimit] -- Xanatos -->
	void SetGlobalSourceLimits();
	bool GetPassiveMode() const {return m_bPassiveMode;}
	void SetPassiveMode(bool in){m_bPassiveMode=in;}
	bool GetGlobalHLSrcReqAllowed() const {return m_bGlobalHLSrcReqAllowed;}
	UINT GetGlobalSourceCount(UINT cat = (UINT)-1); // NEO: CSL - [CategorySourceLimit]
	// NEO: GSL END <-- Xanatos --

	// NEO: MSR - [ManualSourceRequest] -- Xanatos -->
	void	ManualLocalRequests(CTypedPtrList<CPtrList, CPartFile*>& ServerReqQueue);
	void	ManualGlobalRequests(CTypedPtrList<CPtrList, CPartFile*>& GlobalReqQueue);
	void	CahceUDPServer(CSearchFile* toadd); // NEO: XUC - [ExtendedUdpCache]
	// NEO: MSR END <-- Xanatos --

	void CDownloadQueue::ExecuteNeoCommand(CTypedPtrList<CPtrList, CPartFile*>& selectedList, uint8 uNeoCmdL, uint8 uNeoCmdW); // NEO: MOD - [NeoDownloadCommands] <-- Xanatos --

#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	void	GetTransferTipInfo(CString &info);
	void	GetTipInfoByCat(uint8 cat, CString &info);
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

	CServer* cur_udpserver;

	CPartFile* forcea4af_file; // NEO: NXC - [NewExtendedCategories] <-- Xanatos --

protected:
	bool	SendNextUDPPacket();
	void	ProcessLocalRequests();
	bool	IsMaxFilesPerUDPServerPacketReached(uint32 nFiles, uint32 nIncludedLargeFiles) const;
	bool	SendGlobGetSourcesUDPPacket(CSafeMemFile* data, bool bExt2Packet, uint32 nFiles, uint32 nIncludedLargeFiles);

private:
	bool	CompareParts(POSITION pos1, POSITION pos2);
	void	SwapParts(POSITION pos1, POSITION pos2);
	void	HeapSort(UINT first, UINT last);
	void	PurgeED2KLinkQueue(); // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
	CTypedPtrList<CPtrList, CPartFile*> filelist;
	CTypedPtrList<CPtrList, CPartFile*> m_localServerReqQueue;
	uint16	filesrdy;
	uint32	datarate;
	
	CPartFile*	lastfile;
	uint32		lastcheckdiskspacetime;
	uint32		lastudpsearchtime;
	uint32		lastudpstattime;
	UINT		udcounter;
	UINT		m_cRequestsSentToServer;
	uint32		m_dwNextTCPSrcReq;
	int			m_iSearchedServers;
	uint32		lastkademliafilerequest;

	uint64		m_datarateMS;
	uint32		m_nUDPFileReasks;
	uint32		m_nFailedUDPFileReasks;

	// By BadWolf - Accurate Speed Measurement
	typedef struct TransferredData {
		uint32	datalen;
		DWORD	timestamp;
	};
	CList<TransferredData> avarage_dr_list;
	// END By BadWolf - Accurate Speed Measurement

	CSourceHostnameResolveWnd m_srcwnd;

    DWORD       m_dwLastA4AFtime; // ZZ:DownloadManager

	// NEO: GSL - [GlobalSourceLimit] -- Xanatos -->
	DWORD m_uLastGlobalSourceLimit;
	bool m_bPassiveMode;
	bool m_bGlobalHLSrcReqAllowed;
	// NEO: GSL END <-- Xanatos --

	// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
	uint32		m_iNextLinkQueuedTick;
	CTypedPtrList<CPtrList, CED2KFileLink*> m_ED2KLinkQueue;
	// NEO: NXC END <-- Xanatos --

#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
	CTypedPtrList<CPtrList, ThrottledControlSocket*> ProcessQueue;
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
	uint32	numberOfDownloading;
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
	// NEO: QS - [QuickStart] -- Xanatos -->
	bool		m_bQuickStartDone;
	DWORD		m_dwQuickStartEndTime;
	// NEO: QS END <-- Xanatos --

	CMap <CSKey,const CSKey&, void*, void*> m_CachedServers; // NEO: XUC - [ExtendedUdpCache] <-- Xanatos --
};

// NEO: MOD - [NeoDownloadCommands] -- Xanatos -->
///////////////////////////////////////////////////////////////////////
// Command ID's for neo downlaod commands

// Group1
#define	INST_COLLECT				0xA0

#define	INST_COLLECT_ALL_SOURCES	0xA1
	
#define	INST_COLLECT_XS_SOURCES		0xA2
#define	INST_COLLECT_SVR_SOURCES	0xA3
#define	INST_COLLECT_KAD_SOURCES	0xA4
#define	INST_COLLECT_UDP_SOURCES	0xA5
#ifdef VOODOO // NEO: VOODOOx - [VoodooSourceExchange]
#define	INST_COLLECT_VOODOO_SOURCES	0xA6
#endif // VOODOOx // NEO: VOODOOn END

#define	INST_AHL_INCREASE			0xAA
#define	INST_AHL_DECREASE			0xAB

// Group 2
#define	INST_DROP					0xB0

#define	INST_DROP_NNP				0xB1
#define	INST_DROP_FULLQ				0xB2
#define	INST_DROP_HIGHQ				0xB3
#define	INST_DROP_WAITINGRETRY		0xB4
#define	INST_DROP_OUTOFDATE			0xB5
#define	INST_DROP_LOADED			0xB6
#define	INST_DROP_LOADEDWAITING		0xB7
#define	INST_DROP_RESERVE			0xB8
#define	INST_DROP_RETIRED			0xB9
#define	INST_DROP_UNREACHABLE		0xBA
#define	INST_DROP_TOMANY			0xBB
#define	INST_DROP_UNKNOWN			0xBC
#define	INST_DROP_BAD				0xBD
#define	INST_DROP_LOW2LOW			0xBE
#define	INST_DROP_CACHED			0xBF

// Group 3
#define	INST_REASK					0xC0

#define	INST_REASK_SRC				0xC1
#define	INST_REASK_FULLQ			0xC2
#define	INST_REASK_NNP				0xC3
#define	INST_REASK_WAITINGRETRY		0xC4
#define	INST_REASK_LOADEDGROUP		0xC5
#define	INST_REASK_LOADED			0xC6
#define	INST_REASK_LOADEDWAITING	0xC7
#define	INST_REASK_RESERVE			0xC8
#define	INST_REASK_RETIRED			0xC9
#define	INST_REASK_UNREACHABLE		0xCA
#define	INST_REASK_CACHED			0xCB

// Group 4
#define	INST_STORAGE				0xD0

#define	INST_STORAGE_LOAD			0xD1
#define	INST_STORAGE_SAVE			0xD2
#define	INST_STORAGE_IMPORT			0xD3
#define	INST_STORAGE_EXPORT			0xD4
#define	INST_STORAGE_FIND			0xD5

// Group 5
#define	INST_OTHER					0xE0

// NEO: PP - [PasswordProtection]
#define	INST_OTHER_PROTECT_SHOW		0xE1
#define	INST_OTHER_PROTECT_HIDE		0xE2
#define	INST_OTHER_PROTECT_SET		0xE3
#define	INST_OTHER_PROTECT_CHANGE	0xE4
#define	INST_OTHER_PROTECT_UNSET	0xE5
// NEO: PP END

// NEO: FCFG - [FileConfiguration]
#define	INST_OTHER_PROPERTIES		0xE6
#define	INST_OTHER_PREFERENCES		0xE7
// NEO: FCFG END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
#define	INST_OTHER_VOODOO_LIST		0xE8
#endif // VOODOO // NEO: VOODOO END

#define	INST_SEP					0xEF

// NEO: MOD END <-- Xanatos --