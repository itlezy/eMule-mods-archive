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

class CSourceHostnameResolveWnd : public CWnd
{
// Construction
public:
	CSourceHostnameResolveWnd();
	virtual ~CSourceHostnameResolveWnd();

	void AddToResolve(const uchar* fileid, LPCSTR pszHostname, uint16 port);

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnHostnameResolved(WPARAM wParam, LPARAM lParam);

private:
	struct Hostname_Entry {
		uchar fileid[16];
		CStringA strHostname;
		uint16 port;
	};
	CAtlList<Hostname_Entry*> m_toresolve;
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
	void	Init();

	// add/remove entries
	void	AddPartFilesToShare();
	void	AddDownload(CPartFile* newfile, bool paused);
	void	AddSearchToDownload(CSearchFile* toadd, uint8 paused = 2, size_t cat = 0);
	void	AddSearchToDownload(CString link, uint8 paused = 2, size_t cat = 0);
	void	AddFileLinkToDownload(class CED2KFileLink* pLink, size_t cat = 0, LPCTSTR filename = NULL);
	void	RemoveFile(CPartFile* toremove);
	void	DeleteAll();

	size_t	GetFileCount() const { return filelist.GetCount();}
	UINT	GetDownloadingFileCount() const;

	bool	IsFileExisting(const uchar* fileid, bool bLogWarnings = true) const;
	bool	IsPartFile(const CKnownFile* file) const;

	CString getTextList();
	CString	GetFilesCount(size_t inCategory);

	//Xman
	bool	IsTempFile(const CString& rstrDirectory, const CString& rstrName) const;	// SLUGFILLER: SafeHash

	CPartFile* GetFileByID(const uchar* filehash) const;
	CPartFile* GetFileByKadFileSearchID(uint32 ID) const;

    void    StartNextFileIfPrefs(size_t cat);
	void	StartNextFile(size_t cat=-1,bool force=false);

	// sources
	CUpDownClient* GetDownloadClientByIP(uint32 dwIP);
	CUpDownClient* GetDownloadClientByIP_UDP(uint32 dwIP, uint16 nUDPPort, bool bIgnorePortOnUniqueIP, bool* pbMultipleIPs = NULL);
	bool	IsInList(const CUpDownClient* client) const;

	bool    CheckAndAddSource(CPartFile* sender,CUpDownClient* source);
	bool    CheckAndAddKnownSource(CPartFile* sender,CUpDownClient* source, bool bIgnoreGlobDeadList = false);
	bool	RemoveSource(CUpDownClient* toremove, bool bDoStatsUpdate = true);

	// statistics
	typedef struct{
		UINT	a[23];
	} SDownloadStats;
	void	GetDownloadSourcesStats(SDownloadStats& results);
	UINT	GetTransferingSources();
	//uint32	GetDatarate() {return datarate;}

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
	void	ResetCatParts(size_t cat);
	void	SetCatPrio(size_t cat, uint8 newprio);
    void    RemoveAutoPrioInCat(size_t cat, uint8 newprio); // ZZ:DownloadManager
	void	SetCatStatus(size_t cat, int newstatus);
	void	MoveCat(size_t from, size_t to);

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
	void	CheckDiskspace(bool bNotEnoughSpaceLeft = false); // SLUGFILLER: checkDiskspace
	void	CheckDiskspaceTimed();

	void	OnConnectionState(bool bConnected);

	CString GetOptimalTempDir(size_t nCat, EMFileSize nFileSize);

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

	void	UpdateCompressible();
#ifdef _DEBUG
	void	PrintStatistic();
#endif

protected:
	bool	SendNextUDPPacket();
	void	ProcessLocalRequests();
	bool	IsMaxFilesPerUDPServerPacketReached(uint32 nFiles, uint32 nIncludedLargeFiles) const;
	bool	SendGlobGetSourcesUDPPacket(CSafeMemFile* data, bool bExt2Packet, uint32 nFiles, uint32 nIncludedLargeFiles);

private:
	// SLUGFILLER: checkDiskspace
	bool	CompareParts(POSITION pos1, POSITION pos2);
	void	SwapParts(POSITION pos1, POSITION pos2);
	void	HeapSort(UINT_PTR first, UINT_PTR last);
	// SLUGFILLER: checkDiskspace
//Xman see all sources
public:
	CAtlList<CPartFile*> filelist;  
private:
//Xman end
	CAtlList<CPartFile*> m_localServerReqQueue;
	uint16	filesrdy;
	//uint32	datarate;

	// Maella -Pseudo overhead datarate control-
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
	uint32		lastcheckdiskspacetime;	// SLUGFILLER: checkDiskspace
	uint32		lastudpsearchtime;
	uint32		lastudpstattime;
	UINT		udcounter;
	UINT		m_cRequestsSentToServer;
	uint32		m_dwNextTCPSrcReq;
	int			m_iSearchedServers;
	uint32		lastkademliafilerequest;

	//uint64		m_datarateMS;
	uint32		m_nUDPFileReasks;
	uint32		m_nFailedUDPFileReasks;
	//Xman Xtreme Mod
	uint32		m_TCPFileReask;
	uint32		m_FailedTCPFileReask;
	//Xman end

	CSourceHostnameResolveWnd m_srcwnd;
	//is it necessary to use these codes to fix such a minor bug?
	//Poco::FastMutex srcLock;	//zz_fly :: make source add action thread safe :: Enig123
    //DWORD       m_dwLastA4AFtime; // ZZ:DownloadManager
};
