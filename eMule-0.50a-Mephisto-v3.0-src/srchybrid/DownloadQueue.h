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
struct SUnresolvedHostname;

namespace Kademlia 
{
	class CUInt128;
};

// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
#include "SelCategoryDlg.h"
#include "MenuCmds.h"
// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
#include "SettingsSaver.h" // File Settings [sivka/Stulle] - Stulle

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

// ==> File Settings [sivka/Stulle] - Stulle
class CSaveSettingsThread : public CWinThread
{
public:
    CSaveSettingsThread(void);
    ~CSaveSettingsThread(void);

    void EndThread();
    void Pause(bool paused);
	void KeepWaiting() {m_dwLastWait = ::GetTickCount();}

private:
    static UINT RunProc(LPVOID pParam);
    UINT RunInternal();

	CSettingsSaver m_SettingsSaver;
    CEvent* threadEndedEvent;
    CEvent* pauseEvent;
	CEvent* waitEvent;
	volatile bool bDoRun;
	volatile bool bDoWait;
	DWORD m_dwLastWait;
};
// <== File Settings [sivka/Stulle] - Stulle

// ==> Enforce Ratio [Stulle] - Stulle
enum DownLimitReason
{
	DLR_NONE	= 0,	// no limit
	DLR_NOUL	= 1,	// unlimited because no UL possible
	DLR_13RATIO	= 2,	// unlimited because < 1:3 ratio
	DLR_NAFC	= 4,	// NAFC limit
	DLR_SESLIM	= 8,	// Will be session limited by sources
	DLR_ENFLIM	= 16,	// Will be session limited by enforce
	// ==> Multiple friendslots [ZZ] - Mephisto
	/*
	DLR_SOURCE	= 32,	// Forced 1:3 by sources
	DLR_ENFORCE	= 64,	// Enforce
	*/
	DLR_FRILIM	= 32,	// Will be session limited by friendslots
	DLR_SOURCE	= 64,	// Forced 1:3 by sources
	DLR_ENFORCE	= 128,	// Enforce
	DLR_FRIEND	= 256,	// Friendslots
	// <== Multiple friendslots [ZZ] - Mephisto
};
// <== Enforce Ratio [Stulle] - Stulle

class CDownloadQueue
{
	friend class CAddFileThread;
	friend class CServerSocket;

public:
	CDownloadQueue();
	~CDownloadQueue();

	void	Process();
	void	Init();
	
	// add/remove entries
	void	AddPartFilesToShare();
	void	AddDownload(CPartFile* newfile, bool paused);
	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	/*
	void	AddSearchToDownload(CSearchFile* toadd, uint8 paused = 2, int cat = 0);
	void	AddSearchToDownload(CString link, uint8 paused = 2, int cat = 0);
	void	AddFileLinkToDownload(class CED2KFileLink* pLink, int cat = 0);
	*/
	//Modified these three functions by adding and in some cases removing params.
	void	AddSearchToDownload(CSearchFile* toadd, uint8 paused = 2, int cat = 0, uint16 useOrder = 0);
	void	AddSearchToDownload(CString link,uint8 paused = 2, int cat = 0, uint16 useOrder = 0);
	void	AddFileLinkToDownload(class CED2KFileLink* pLink, int cat = 0, bool AllocatedLink = false);
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	void	RemoveFile(CPartFile* toremove);
	void	DeleteAll();

	int		GetFileCount() const { return filelist.GetCount();}
	UINT	GetDownloadingFileCount() const;
	UINT	GetPausedFileCount() const;

	bool	IsFileExisting(const uchar* fileid, bool bLogWarnings = true) const;
	bool	IsPartFile(const CKnownFile* file) const;

	//Xman
	bool	IsTempFile(const CString& rstrDirectory, const CString& rstrName) const;	// SLUGFILLER: SafeHash

	CPartFile* GetFileByID(const uchar* filehash) const;
	CPartFile* GetFileByIndex(int index) const;
	CPartFile* GetFileByKadFileSearchID(uint32 ID) const;

    void    StartNextFileIfPrefs(int cat);
	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	/*
	void	StartNextFile(int cat=-1,bool force=false);
	*/
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

	void	RefilterAllComments();	

	// sources
	CUpDownClient* GetDownloadClientByIP(uint32 dwIP);
	CUpDownClient* GetDownloadClientByIP_UDP(uint32 dwIP, uint16 nUDPPort, bool bIgnorePortOnUniqueIP, bool* pbMultipleIPs = NULL);
	bool	IsInList(const CUpDownClient* client) const;

	bool    CheckAndAddSource(CPartFile* sender,CUpDownClient* source);
	bool    CheckAndAddKnownSource(CPartFile* sender,CUpDownClient* source, bool bIgnoreGlobDeadList = false);
	bool	RemoveSource(CUpDownClient* toremove, bool bDoStatsUpdate = true);

	// statistics
	typedef struct{
		int	a[23];
	} SDownloadStats;
	void	GetDownloadSourcesStats(SDownloadStats& results);
	int		GetDownloadFilesStats(uint64 &ui64TotalFileSize, uint64 &ui64TotalLeftToTransfer, uint64 &ui64TotalAdditionalNeededSpace);
	//Xman
	/*
	uint32	GetDatarate() {return datarate;}
	*/
	//Xman end

	void	AddUDPFileReasks()								{m_nUDPFileReasks++;}
	uint32	GetUDPFileReasks() const						{return m_nUDPFileReasks;}
	void	AddFailedUDPFileReasks()						{m_nFailedUDPFileReasks++;}
	uint32	GetFailedUDPFileReasks() const					{return m_nFailedUDPFileReasks;}
	//Xman Xtreme Mod
	void	AddTCPFileReask()								{m_TCPFileReask++;}
	uint32	GetTCPFileReasks() const							{return m_TCPFileReask;}
	void	AddFailedTCPFileReask()							{m_FailedTCPFileReask++;}
	uint32	GetFailedTCPFileReasks() const					{return m_FailedTCPFileReask;}
	//Xman end

	// categories
	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	/*
	void	ResetCatParts(UINT cat);
	*/
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	void	SetCatPrio(UINT cat, uint8 newprio);
    void    RemoveAutoPrioInCat(UINT cat, uint8 newprio); // ZZ:DownloadManager
	void	SetCatStatus(UINT cat, int newstatus);
	void	MoveCat(UINT from, UINT to);
	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	/*
	void	SetAutoCat(CPartFile* newfile);
	*/
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

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
	//zz_fly :: remove useless code :: Enig123 :: start
	//note: not needed, because priority have been handled in CDownloadQueue::Process(), thanks Enig123
	/*
	void	SortByPriority();
	*/
	//zz_fly :: end
	void	CheckDiskspace(bool bNotEnoughSpaceLeft = false);
	void	CheckDiskspaceTimed();

	void	ExportPartMetFilesOverview() const;
	void	OnConnectionState(bool bConnected);

	void	AddToResolved( CPartFile* pFile, SUnresolvedHostname* pUH );

	CString GetOptimalTempDir(UINT nCat, EMFileSize nFileSize);

	CServer* cur_udpserver;

	//Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	void	CompDownloadRate();

	//Xman askfordownload priority
	uint16	GetTooManyConnections(bool recalc=false);
	uint8	GetMaxDownPrio(void) const {return m_maxdownprio;}
	void	SetMaxDownPrioNew(uint8 newprio) {m_maxdownprionew=newprio;}
	uint8	GetMaxDownPrioNew(void) const {return m_maxdownprionew;}
	//Xman end

	//Xman GlobalMaxHarlimit for fairness
	void	IncGlobSources() {m_uGlobsources++;	}
	void	DecGlobSources() {m_uGlobsources--;	}
	uint32	GetGlobalSources() const {return m_uGlobsources;	}
	// ==> Multiple friendslots [ZZ] - Mephisto
	/*
	uint8	GetLimitState() const {return m_limitstate;}
	*/
	uint16	GetLimitState() const {return m_limitstate;}
	// <== Multiple friendslots [ZZ] - Mephisto
	uint8	GetLimitRatio() const {return m_limitratio;} // Enforce Ratio [Stulle] - Stulle
	//Xman end

#ifdef PRINT_STATISTIC
	void	PrintStatistic();
#endif

protected:
	bool	SendNextUDPPacket();
	void	ProcessLocalRequests();
	bool	IsMaxFilesPerUDPServerPacketReached(uint32 nFiles, uint32 nIncludedLargeFiles) const;
	bool	SendGlobGetSourcesUDPPacket(CSafeMemFile* data, bool bExt2Packet, uint32 nFiles, uint32 nIncludedLargeFiles);

	//zz_fly :: remove useless code :: Enig123 :: start
	//note: not needed, because priority have been handled in CDownloadQueue::Process(), thanks Enig123
	/*
private:
	bool	CompareParts(POSITION pos1, POSITION pos2);
	void	SwapParts(POSITION pos1, POSITION pos2);
	void	HeapSort(UINT first, UINT last);
	*/
	//zz_fly :: end

//Xman see all sources
public:
	CTypedPtrList<CPtrList, CPartFile*> filelist;
private:
//Xman end
	
	CTypedPtrList<CPtrList, CPartFile*> m_localServerReqQueue;
	uint16	filesrdy;

	// Maella -Pseudo overhead datarate control-
	/*
	uint32	datarate;
	*/
	uint32 m_lastProcessTime;
	uint64 m_lastOverallReceivedBytes;	
	uint64 m_lastReceivedBytes;	
	sint64 m_nDownloadSlopeControl; //Xman changed to sint64
	// Maella end

	//Xman askfordownload priority
	uint16		m_toomanyconnections;
	//uint32	m_toomanytimestamp;
	uint8 m_maxdownprio;
	uint8 m_maxdownprionew;
	//Xman end

	CPartFile*	lastfile;
	uint32		lastcheckdiskspacetime;
	uint32		lastudpsearchtime;
	uint32		lastudpstattime;
	UINT		udcounter;
	UINT		m_cRequestsSentToServer;
	uint32		m_dwNextTCPSrcReq;
	int			m_iSearchedServers;
	uint32		lastkademliafilerequest;

	//Xman
	/*
	uint64		m_datarateMS;
	*/
	//Xman end
	uint32		m_nUDPFileReasks;
	uint32		m_nFailedUDPFileReasks;
	//Xman Xtreme Mod
	uint32		m_TCPFileReask;
	uint32		m_FailedTCPFileReask;
	//Xman end

	//Xman
	/*
	// By BadWolf - Accurate Speed Measurement
	typedef struct TransferredData {
		uint32	datalen;
		DWORD	timestamp;
	};
	CList<TransferredData> avarage_dr_list;
	// END By BadWolf - Accurate Speed Measurement
	*/
	//Xman end

	//Xman GlobalMaxHarlimit for fairness
	uint32		m_uGlobsources;
	// ==> Multiple friendslots [ZZ] - Mephisto
	/*
	uint8		m_limitstate;
	*/
	uint16		m_limitstate;
	// <== Multiple friendslots [ZZ] - Mephisto
	uint8		m_limitratio; // Enforce Ratio [Stulle] - Stulle

	CSourceHostnameResolveWnd m_srcwnd;

	//is it necessary to use these codes to fix such a minor bug?
	//CCriticalSection srcLock;	//zz_fly :: make source add action thread safe :: Enig123

    //Xman
    /*
    DWORD       m_dwLastA4AFtime; // ZZ:DownloadManager
    */
    //Xman end

	uint32 GlobalHardLimitTemp; // show global HL - Stulle

	// ==> File Settings [sivka/Stulle] - Stulle
public:
	void InitTempVariables(CPartFile* file);
	void UpdateFileSettings(CPartFile* file);
	void SaveFileSettings(bool bStart = true);
protected:
	CSettingsSaver m_SettingsSaver;
	CSaveSettingsThread* m_SaveSettingsThread;
	bool m_bSaveAgain;
	DWORD m_dwLastSave;
	// <== File Settings [sivka/Stulle] - Stulle

	// ==> Global Source Limit [Max/Stulle] - Stulle
public:
	void SetHardLimits();
	void SetUpdateHlTime(DWORD in){m_dwUpdateHlTime = in;}
	bool GetPassiveMode() const {return m_bPassiveMode;}
	void SetPassiveMode(bool in){m_bPassiveMode=in;}
	bool GetGlobalHLSrcReqAllowed() const {return m_bGlobalHLSrcReqAllowed;}
protected:
	DWORD m_dwUpdateHL;
	DWORD m_dwUpdateHlTime;
	bool m_bPassiveMode;
	bool m_bGlobalHLSrcReqAllowed;
	// <== Global Source Limit [Max/Stulle] - Stulle

	DWORD m_dwResTimer; // CPU/MEM usage [$ick$/Stulle] - Max

public:
	uint16 GetGlobalSourceCount(); // Show sources on title - Stulle

	// ==> Quick start [TPT] - Max
	int quickflag;
	int quickflags;
	// <== Quick start [TPT] - Max

	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	bool	StartNextFile(int cat=-1,bool force=false);
	void	StopPauseLastFile(int Mode = MP_PAUSE, int Category = -1);
	UINT	GetMaxCatResumeOrder(UINT iCategory = 0);
	void	GetCategoryFileCounts(UINT iCategory, int cntFiles[]);
	UINT	GetCategoryFileCount(UINT iCategory);
	UINT	GetHighestAvailableSourceCount(int nCat = -1);
	UINT	GetCatActiveFileCount(UINT iCategory);
	UINT	GetAutoCat(CString sFullName, EMFileSize nFileSize);
	bool	ApplyFilterMask(CString sFullName, UINT nCat);
	void	ResetCatParts(UINT cat, UINT useCat = 0);

private:
	bool		m_bBusyPurgingLinks;
	bool		PurgeED2KLinkQueue();
	uint32		m_iLastLinkQueuedTick;

	CTypedPtrList<CPtrList, CED2KFileLink*> m_ED2KLinkQueue;
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
};
