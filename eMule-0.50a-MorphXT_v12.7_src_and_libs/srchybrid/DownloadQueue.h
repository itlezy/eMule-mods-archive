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
#include "MenuCmds.h" //Morph

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

// khaos::categorymod+
#include "SelCategoryDlg.h"
// khaos::categorymod-

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
	friend class CImportPartsFileThread;
	friend class CServerSocket;

public:
	CDownloadQueue();
	~CDownloadQueue();

	void	Process();
	void	Init();
	
	// add/remove entries
	void	AddPartFilesToShare();
	void	AddDownload(CPartFile* newfile, bool paused);
	//MORPH START - Changed by SiRoB, Selection category support khaos::categorymod+
	//Modified these three functions by adding and in some cases removing params.
	void	AddSearchToDownload(CSearchFile* toadd, uint8 paused = 2, int cat = 0, uint16 useOrder = 0);
	void	AddSearchToDownload(CString link,uint8 paused = 2, int cat = 0, uint16 useOrder = 0);
	void	AddFileLinkToDownload(class CED2KFileLink* pLink, int cat = 0, bool AllocatedLink = false,bool bFromClipboard = false);
	//MORPH END   - Changed by SiRoB, Selection category support khaos::categorymod-
	void	RemoveFile(CPartFile* toremove);
	void	DeleteAll();

	int		GetFileCount() const { return filelist.GetCount(); }
	UINT	GetDownloadingFileCount() const;
	UINT	GetPausedFileCount() const;

	bool	IsFileExisting(const uchar* fileid, bool bLogWarnings = true) const;
	bool	IsPartFile(const CKnownFile* file) const;
	bool	IsTempFile(const CString& rstrDirectory, const CString& rstrName) const;	// SLUGFILLER: SafeHash

	CPartFile* GetFileByID(const uchar* filehash) const;
	CPartFile* GetFileByIndex(int index) const;
	CPartFile* GetFileByKadFileSearchID(uint32 ID) const;

    void    StartNextFileIfPrefs(int cat);
	// khaos::categorymod+
	bool	StartNextFile(int cat=-1,bool force=false);
	void	StopPauseLastFile(int Mode = MP_PAUSE, int Category = -1);
	UINT	GetMaxCatResumeOrder(UINT iCategory = 0);
	void	GetCategoryFileCounts(UINT iCategory, int cntFiles[]);
	UINT	GetCategoryFileCount(UINT iCategory);
	UINT	GetHighestAvailableSourceCount(int nCat = -1);
	UINT	GetCatActiveFileCount(UINT iCategory);
	UINT	GetAutoCat(CString sFullName, EMFileSize nFileSize);
	bool	ApplyFilterMask(CString sFullName, UINT nCat);
	// khaos::categorymod-

	void	RefilterAllComments();	

	// sources
	CUpDownClient* GetDownloadClientByIP(uint32 dwIP);
	CUpDownClient* GetDownloadClientByIP_UDP(uint32 dwIP, uint16 nUDPPort, bool bIgnorePortOnUniqueIP, bool* pbMultipleIPs = NULL);
	bool	IsInList(const CUpDownClient* client) const;

	bool    CheckAndAddSource(CPartFile* sender,CUpDownClient* source);
	bool    CheckAndAddKnownSource(CPartFile* sender,CUpDownClient* source, bool bIgnoreGlobDeadList = false, bool doThrow = false);
	bool	RemoveSource(CUpDownClient* toremove, bool bDoStatsUpdate = true);

	// statistics
	typedef struct{
		int	a[23];
	} SDownloadStats;
	void	GetDownloadSourcesStats(SDownloadStats& results);
	int		GetDownloadFilesStats(uint64 &ui64TotalFileSize, uint64 &ui64TotalLeftToTransfer, uint64 &ui64TotalAdditionalNeededSpace);
	uint32	GetDatarate() {return datarate;}

	void	AddUDPFileReasks()								{m_nUDPFileReasks++;}
	uint32	GetUDPFileReasks() const						{return m_nUDPFileReasks;}
	void	AddFailedUDPFileReasks()						{m_nFailedUDPFileReasks++;}
	uint32	GetFailedUDPFileReasks() const					{return m_nFailedUDPFileReasks;}

	// categories
	// khaos::categorymod+	
	void	ResetCatParts(UINT cat, UINT useCat = 0);
	// khaos::categorymod-
	void	SetCatPrio(UINT cat, uint8 newprio);
    void    RemoveAutoPrioInCat(UINT cat, uint8 newprio); // ZZ:DownloadManager
	void	SetCatStatus(UINT cat, int newstatus);
	void	MoveCat(UINT from, UINT to);
	//MORPH START - Removed by SiRoB, Due to Khaos Categorie
	/*
	void	SetAutoCat(CPartFile* newfile);
	*/
	//MORPH END   - Removed by SiRoB, Due to Khaos Categorie
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

	CServer* cur_udpserver;
	bool	IsFilesPowershared(); //MORPH - Added by SiRoB, ZZ Ratio
	// khaos::kmod+ Advanced A4AF: Brute Force
	CPartFile* forcea4af_file;
	// khaos::kmod-

	//MORPH START - Added by SiRoB, ZZ Ratio in Work
	bool	IsZZRatioInWork() {return m_bIsZZRatioInWork;}
	//MORPH END   - Added by SiRoB, ZZ Ratio in Work
        
        
protected:
	bool	SendNextUDPPacket();
	void	ProcessLocalRequests();
	bool	IsMaxFilesPerUDPServerPacketReached(uint32 nFiles, uint32 nIncludedLargeFiles) const;
	bool	SendGlobGetSourcesUDPPacket(CSafeMemFile* data, bool bExt2Packet, uint32 nFiles, uint32 nIncludedLargeFiles);

private:
	bool	CompareParts(POSITION pos1, POSITION pos2);
	void	SwapParts(POSITION pos1, POSITION pos2);
	void	HeapSort(UINT first, UINT last);
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
	//MORPH START - Removed by SiRoB, sum datarate calculated for each file
	/*
	CList<TransferredData> avarage_dr_list;
	*/
	//MORPH END   - Removed by SiRoB, sum datarate calculated for each file
	
	CSourceHostnameResolveWnd m_srcwnd;

    DWORD       m_dwLastA4AFtime; // ZZ:DownloadManager
	// khaos::categorymod+ For queuing ED2K link additions.
	bool		m_bBusyPurgingLinks;
	bool		PurgeED2KLinkQueue();
	uint32		m_iLastLinkQueuedTick;
	bool        m_bClipboardLinkInQueue;
       public: // public for category selection box:
	CTypedPtrList<CPtrList, CED2KFileLink*> m_ED2KLinkQueue;
	// khaos::categorymod-

	//MORPH START - Added by SiRoB, ZZ Ratio in Work
	bool	m_bIsZZRatioInWork;
	//MORPH START - Added by SiRoB, ZZ Ratio in Work

	//MORPH START - Added by Stulle, Global Source Limit
public:
	void SetHardLimits();
	void SetUpdateHlTime(DWORD in){m_dwUpdateHlTime = in;}
	bool GetPassiveMode() const {return m_bPassiveMode;}
	void SetPassiveMode(bool in){m_bPassiveMode=in;}
	bool GetGlobalHLSrcReqAllowed() const {return m_bGlobalHLSrcReqAllowed;}
	uint16 GetGlobalSourceCount();
protected:
	DWORD m_dwUpdateHL;
	DWORD m_dwUpdateHlTime;
	bool m_bPassiveMode;
	bool m_bGlobalHLSrcReqAllowed;
	//MORPH END   - Added by Stulle, Global Source Limit

	//MORPH START - Added by schnulli900, count failed TCP/IP connections [Xman]
public:
	void	AddTCPFileReask()		{m_uTCPFileReask++;}
	uint32	GetTCPFileReasks() const	{return m_uTCPFileReask;}
	void	AddFailedTCPFileReask()		{m_uFailedTCPFileReask++;}
	uint32	GetFailedTCPFileReasks() const	{return m_uFailedTCPFileReask;}
private:
	uint32	m_uTCPFileReask;
	uint32	m_uFailedTCPFileReask;
        //MORPH END   - Added by schnulli900, count failed TCP/IP connections [Xman]
};
