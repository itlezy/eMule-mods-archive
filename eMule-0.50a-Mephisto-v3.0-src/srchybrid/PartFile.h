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
#include "KnownFile.h"
#include "DeadSourceList.h"
#include "CorruptionBlackBox.h"
#include "SourceSaver.h"	// Xman -New Save/load Sources- enkeyDEV(Ottavio84)

enum EPartFileStatus{
	PS_READY			= 0,
	PS_EMPTY			= 1,
	PS_WAITINGFORHASH	= 2,
	PS_HASHING			= 3,
	PS_ERROR			= 4,
	PS_INSUFFICIENT		= 5,
	PS_UNKNOWN			= 6,
	PS_PAUSED			= 7,
	PS_COMPLETING		= 8,
	PS_COMPLETE			= 9
};

#define PR_VERYLOW			4 // I Had to change this because it didn't save negative number correctly.. Had to modify the sort function for this change..
#define PR_LOW				0 //*
#define PR_NORMAL			1 // Don't change this - needed for edonkey clients and server!
#define PR_HIGH				2 //*
#define PR_VERYHIGH			3
#define PR_AUTO				5 //UAP Hunter
#define PR_POWER			6 //Xman PowerRelease


//#define BUFFER_SIZE_LIMIT 500000 // Max bytes before forcing a flush

#define	PARTMET_BAK_EXT	_T(".bak")
#define	PARTMET_TMP_EXT	_T(".backup")

#define STATES_COUNT		17

enum EPartFileFormat{
	PMT_UNKNOWN			= 0,
	PMT_DEFAULTOLD,
	PMT_SPLITTED,
	PMT_NEWOLD,
	PMT_SHAREAZA,
	PMT_BADFORMAT	
};

enum EPartFileLoadResult{
	PLR_LOADSUCCESS = 1,
	PLR_CHECKSUCCESS = 2,
	PLR_FAILED_METFILE_CORRUPT = -1,
	PLR_FAILED_METFILE_NOACCESS = -2,
	PLR_FAILED_OTHER   = 0
};

#define	FILE_COMPLETION_THREAD_FAILED	0x0000
#define	FILE_COMPLETION_THREAD_SUCCESS	0x0001
#define	FILE_COMPLETION_THREAD_RENAMED	0x0002

enum EPartFileOp{
	PFOP_NONE = 0,
	PFOP_HASHING,
	PFOP_COPYING,
	PFOP_UNCOMPRESSING
	,PFOP_SR13_IMPORTPARTS //MORPH - Added by SiRoB, Import Parts - added by zz_fly
};

class CSearchFile;
class CUpDownClient;
enum EDownloadState;
class CxImage;
class CSafeMemFile;

#pragma pack(1)
struct Requested_Block_Struct
{
	uint64	StartOffset;
	uint64	EndOffset;
	uchar	FileID[16];
	uint64  transferred; // Barry - This counts bytes completed
};
#pragma pack()

struct Gap_Struct
{
	uint64 start;
	uint64 end;
};

struct PartFileBufferedData
{
	BYTE *data;						// Barry - This is the data to be written
	uint64 start;					// Barry - This is the start offset of the data
	uint64 end;						// Barry - This is the end offset of the data
	Requested_Block_Struct *block;	// Barry - This is the requested block that this data relates to
};

//Xman
// BEGIN SiRoB: Flush Thread
struct FlushDone_Struct
{
	bool	bIncreasedFile;
	bool	bForceICH;
	bool*	changedPart;
};
// END SiRoB: Flush Thread

//Xman sourcecache
enum ESourceFrom;
struct PartfileSourceCache
{
	uint16			nPort;
	uint32			dwID;
	uint32			dwServerIP;
	uint16			nServerPort;
	uchar			achUserHash[16];
	bool			withuserhash;
	bool			ed2kIDFlag;
	uint8			byCryptOptions;
	ESourceFrom		sourcefrom;
	uint32			expires;
};
#define	SOURCECACHEPROCESSLOOP	MIN2MS(1)	//every one minute
#define SOURCECACHELIFETIME		MIN2MS(30)	//expires after 30 minutes
//Xman end


typedef CTypedPtrList<CPtrList, CUpDownClient*> CUpDownClientPtrList;

class CPartFile : public CKnownFile
{
	DECLARE_DYNAMIC(CPartFile)

	friend class CPartFileConvert;
	//Xman
	friend class CPartHashThread;	// SLUGFILLER: SafeHash

	friend class CPartFileFlushThread; //Xman Flush Thread

public:
	CPartFile(UINT cat = 0);
	CPartFile(CSearchFile* searchresult, UINT cat = 0);
	CPartFile(CString edonkeylink, UINT cat = 0);
	CPartFile(class CED2KFileLink* fileLink, UINT cat = 0);
	virtual ~CPartFile();

	bool	IsPartFile() const { return !(status == PS_COMPLETE); }

	// eD2K filename
	virtual void SetFileName(LPCTSTR pszFileName, bool bReplaceInvalidFileSystemChars = false, bool bRemoveControlChars = false); // 'bReplaceInvalidFileSystemChars' is set to 'false' for backward compatibility!

	// part.met filename (without path!)
	const CString& GetPartMetFileName() const { return m_partmetfilename; }

	// full path to part.met file or completed file
	const CString& GetFullName() const { return m_fullname; }
	void	SetFullName(CString name) { m_fullname = name; }
	CString	GetTempPath() const;

	// local file system related properties
	bool	IsNormalFile() const { return (m_dwFileAttributes & (FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_SPARSE_FILE)) == 0; }
	const bool	IsAllocating() const { return m_AllocateThread != NULL; }
	EMFileSize	GetRealFileSize() const;
	void	GetLeftToTransferAndAdditionalNeededSpace(uint64 &ui64LeftToTransfer, uint64 &pui32AdditionalNeededSpace) const;
	uint64	GetNeededSpace() const;
	virtual void SetFileSize(EMFileSize nFileSize);

	// last file modification time (NT's version of UTC), to be used for stats only!
	CTime	GetCFileDate() const { return CTime(m_tLastModified); }
	uint32	GetFileDate() const { return m_tLastModified; }

	// file creation time (NT's version of UTC), to be used for stats only!
	CTime	GetCrCFileDate() const { return CTime(m_tCreated); }
	uint32	GetCrFileDate() const { return m_tCreated; }

	void	InitializeFromLink(CED2KFileLink* fileLink, UINT cat = 0);
	// Maella -New bandwidth control-
	/*
	uint32	Process(uint32 reducedownload, UINT icounter);
	*/
	uint32	Process(uint32 maxammount, bool isLimited, bool fullProcess); // in byte, not in percent
	// Maella end
	//Xman end	

	EPartFileLoadResult	LoadPartFile(LPCTSTR in_directory, LPCTSTR filename, EPartFileFormat* pOutCheckFileFormat = NULL); //filename = *.part.met
	EPartFileLoadResult	ImportShareazaTempfile(LPCTSTR in_directory,LPCTSTR in_filename, EPartFileFormat* pOutCheckFileFormat = NULL);

	bool	SavePartFile(bool bDontOverrideBak = false);
	void	PartFileHashFinished(CKnownFile* result);
	bool	HashSinglePart(UINT partnumber, bool* pbAICHReportedOK = NULL); // true = ok , false = corrupted

	//Xman
	// BEGIN SLUGFILLER: SafeHash - replaced old handlers, full hash checker remains for file completion
	void	PartHashFinished(UINT partnumber, bool bAICHAgreed, bool corrupt);
	void	PartHashFinishedAICHRecover(UINT partnumber, bool corrupt);
	bool	IsPartShareable(UINT partnumber) const;
	bool	IsRangeShareable(uint64 start, uint64 end) const;
	// END SLUGFILLER: SafeHash

	void	AddGap(uint64 start, uint64 end);
	void	FillGap(uint64 start, uint64 end);
	void	DrawStatusBar(CDC* dc, LPCRECT rect, bool bFlat) /*const*/;
	virtual void	DrawShareStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool	 bFlat) const;
	bool	IsComplete(uint64 start, uint64 end, bool bIgnoreBufferedData) const;
	bool	IsPureGap(uint64 start, uint64 end) const;
	bool	IsAlreadyRequested(uint64 start, uint64 end, bool bCheckBuffers = false) const;
    bool    ShrinkToAvoidAlreadyRequested(uint64& start, uint64& end) const;
	bool	IsCorruptedPart(UINT partnumber) const;
	
	//Xman Dynamic block request (netfinity/morph/Xman)
	//zz_fly :: remove unused code :: start
	/*
	uint64	GetRemainingAvailableData(const uint8* srcstatus) const;
	uint64	GetRemainingAvailableData(const CUpDownClient* sender) const;
	*/
	//zz_fly :: remove unused code :: end
	uint32  GetDownloadSpeedInPart(uint16 forpart, CUpDownClient* current_source) const;
	//Xman end

	uint64	GetTotalGapSizeInRange(uint64 uRangeStart, uint64 uRangeEnd) const;
	uint64	GetTotalGapSizeInPart(UINT uPart) const;
	void	UpdateCompletedInfos();
	void	UpdateCompletedInfos(uint64 uTotalGaps);
	virtual void	UpdatePartsInfo();

	//Xman chunk chooser
	//bool	GetNextRequestedBlock(CUpDownClient* sender, Requested_Block_Struct** newblocks, uint16* count) /*const*/;
	bool	GetNextRequestedBlock_Maella(CUpDownClient* sender, Requested_Block_Struct** newblocks, uint16* count) /*const*/;
	bool	GetNextRequestedBlock_zz(CUpDownClient* sender, Requested_Block_Struct** newblocks, uint16* count) /*const*/;
	//Xman end
	void	WritePartStatus(CSafeMemFile* file) const;
	void	WriteCompleteSourcesCount(CSafeMemFile* file) const;
	void	AddSources(CSafeMemFile* sources,uint32 serverip, uint16 serverport, bool bWithObfuscationAndHash);
	void	AddSource(LPCTSTR pszURL, uint32 nIP);
	static bool CanAddSource(uint32 userid, uint16 port, uint32 serverip, uint16 serverport, UINT* pdebug_lowiddropped = NULL, bool Ed2kID = true);
	
	EPartFileStatus	GetStatus(bool ignorepause = false) const;
	void	SetStatus(EPartFileStatus eStatus);		// set status and update GUI
	void	_SetStatus(EPartFileStatus eStatus);	// set status and do *not* update GUI
	void	NotifyStatusChange();
	bool	IsStopped() const { return stopped; }
	bool	GetCompletionError() const { return m_bCompletionError; }
	EMFileSize  GetCompletedSize() const { return completedsize; }
	CString getPartfileStatus() const;
	int		getPartfileStatusRang() const;
	void	SetActive(bool bActive);

	uint8	GetDownPriority() const { return m_iDownPriority; }
	void	SetDownPriority(uint8 iNewDownPriority, bool resort = true);
	bool	IsAutoDownPriority(void) const { return m_bAutoDownPriority; }
	void	SetAutoDownPriority(bool NewAutoDownPriority) { m_bAutoDownPriority = NewAutoDownPriority; }
	void	UpdateAutoDownPriority();

	UINT	GetSourceCount() const { return srclist.GetCount(); }
	UINT	GetSrcA4AFCount() const { return A4AFsrclist.GetCount(); }
	UINT	GetSrcStatisticsValue(EDownloadState nDLState) const;
	UINT	GetTransferringSrcCount() const;
	uint64	GetTransferred() const { return m_uTransferred; }
	//Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	/*
	uint32	GetDatarate() const { return datarate; }
	*/
	//Xman end
	float	GetPercentCompleted() const { return percentcompleted; }
	UINT	GetNotCurrentSourcesCount() const;
	int		GetValidSourcesCount() const;
	bool	IsArchive(bool onlyPreviewable = false) const; // Barry - Also want to preview archives
    bool    IsPreviewableFileType() const;
	time_t	getTimeRemaining() const;
	time_t	getTimeRemainingSimple() const;
	// ==> Make code VS 2005 and VS 2008 ready [MorphXT] - Stulle
	/*
	uint32	GetDlActiveTime() const;
	*/
	time_t	GetDlActiveTime() const;
	// <== Make code VS 2005 and VS 2008 ready [MorphXT] - Stulle

	// Barry - Added as replacement for BlockReceived to buffer data before writing to disk
	uint32	WriteToBuffer(uint64 transize, const BYTE *data, uint64 start, uint64 end, Requested_Block_Struct *block, const CUpDownClient* client);
	void	FlushBuffer(bool forcewait=false, bool bForceICH = false, bool bNoAICH = false);
	//Xman
	// BEGIN SiRoB: Flush Thread
	void	WriteToDisk();
	void	FlushDone();
	// END SiRoB: Flush Thread
	// Barry - This will invert the gap list, up to caller to delete gaps when done
	// 'Gaps' returned are really the filled areas, and guaranteed to be in order
	void	GetFilledList(CTypedPtrList<CPtrList, Gap_Struct*> *filled) const;

	// Barry - Added to prevent list containing deleted blocks on shutdown
	void	RemoveAllRequestedBlocks(void);
	bool	RemoveBlockFromList(uint64 start, uint64 end);
	bool	IsInRequestedBlockList(const Requested_Block_Struct* block) const;
	void	RemoveAllSources(bool bTryToSwap);

	bool	CanOpenFile() const;
	bool	IsReadyForPreview() const;
	bool	CanStopFile() const;
	bool	CanPauseFile() const;
	bool	CanResumeFile() const;
	bool	IsPausingOnPreview() const										{ return m_bPauseOnPreview && IsPreviewableFileType() && CanPauseFile(); }

	void	OpenFile() const;
	void	PreviewFile();
	void	DeleteFile();
	void	StopFile(bool bCancel = false, bool resort = true);
	void	PauseFile(bool bInsufficient = false, bool resort = true);
	void	StopPausedFile();
	void	ResumeFile(bool resort = true);
	void	ResumeFileInsufficient();
	void	SetPauseOnPreview(bool bVal)									{ m_bPauseOnPreview = bVal; }

	virtual Packet* CreateSrcInfoPacket(const CUpDownClient* forClient, uint8 byRequestedVersion, uint16 nRequestedOptions) const;
	void	AddClientSources(CSafeMemFile* sources, uint8 sourceexchangeversion, bool bSourceExchange2, const CUpDownClient* pClient = NULL);

	UINT	GetAvailablePartCount() const { return availablePartsCount; }
	void	UpdateAvailablePartsCount();

	uint32	GetLastAnsweredTime() const	{ return m_ClientSrcAnswered; }
	void	SetLastAnsweredTime() { m_ClientSrcAnswered = ::GetTickCount(); }
	void	SetLastAnsweredTimeTimeout();

	uint64	GetCorruptionLoss() const { return m_uCorruptionLoss; }
	uint64	GetCompressionGain() const { return m_uCompressionGain; }
	uint32	GetRecoveredPartsByICH() const { return m_uPartsSavedDueICH; }

	virtual void	UpdateFileRatingCommentAvail(bool bForceUpdate = false);
	virtual void	RefilterFileComments();

	void	AddDownloadingSource(CUpDownClient* client);
	void	RemoveDownloadingSource(CUpDownClient* client);

	//Xman Xtreme Downloadmanager: Auto-A4AF-check
	bool	IsA4AFAuto() const { return m_is_A4AF_auto; }
	void	SetA4AFAuto(bool in) { m_is_A4AF_auto = in; }
	//Xman end

	CString GetProgressString(uint16 size) const;
	CString GetInfoSummary(bool bNoFormatCommands = false) const;

//	int		GetCommonFilePenalty() const;
	void	UpdateDisplayedInfo(bool force = false);

	// Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	UINT	GetCategory() const;
	UINT	GetConstCategory() const;  //Xman checkmark to catogory at contextmenu of downloadlist
	void	SetCategory(UINT cat);
	bool	HasDefaultCategory() const;
	bool	CheckShowItemInGivenCat(int inCategory) /*const*/;

	uint8*	MMCreatePartStatus();

	//preview
	virtual bool GrabImage(uint8 nFramesToGrab, double dStartTime, bool bReduceColor, uint16 nMaxWidth,void* pSender);
	virtual void GrabbingFinished(CxImage** imgResults, uint8 nFramesGrabbed, void* pSender);

	void	FlushBuffersExceptionHandler(CFileException* error);
	void	FlushBuffersExceptionHandler();

	void	PerformFileCompleteEnd(DWORD dwResult);
	//Xman
	void	PerformFirstHash();		// SLUGFILLER: SafeHash


	void	SetFileOp(EPartFileOp eFileOp);
	EPartFileOp GetFileOp() const							{ return m_eFileOp; }
	void	SetFileOpProgress(UINT uProgress);
	UINT	GetFileOpProgress() const						{ return m_uFileOpProgress; }

	CAICHRecoveryHashSet* GetAICHRecoveryHashSet()							{ return m_pAICHRecoveryHashSet; }
	void	RequestAICHRecovery(UINT nPart);
	void	AICHRecoveryDataAvailable(UINT nPart);
	bool	IsAICHPartHashSetNeeded() const									{ return m_FileIdentifier.HasAICHHash() && !m_FileIdentifier.HasExpectedAICHHashCount() && m_bAICHPartHashsetNeeded; }
	void	SetAICHHashSetNeeded(bool bVal)									{ m_bAICHPartHashsetNeeded = bVal; }

	uint32	m_LastSearchTime;
	uint32	m_LastSearchTimeKad;
	uint64	m_iAllocinfo;
	CUpDownClientPtrList srclist;
	CUpDownClientPtrList A4AFsrclist; //<<-- enkeyDEV(Ottavio84) -A4AF-
	CTime	lastseencomplete;
	CFile	m_hpartfile;				// permanent opened handle to avoid write conflicts
	CMutex 	m_FileCompleteMutex;		// Lord KiRon - Mutex for file completion
	uint16	src_stats[4];
	uint16  net_stats[3];
	volatile bool m_bPreviewing;
	volatile bool m_bRecoveringArchive; // Is archive recovery in progress
	bool	m_bLocalSrcReqQueued;
	bool	srcarevisible;				// used for downloadlistctrl
	bool	m_bMD4HashsetNeeded;
	uint8	m_TotalSearchesKad;
	//Xman Xtreme Downloadmanager
	/*
    bool    AllowSwapForSourceExchange()					{ return ::GetTickCount()-lastSwapForSourceExchangeTick > 30*1000; } // ZZ:DownloadManager
    void    SetSwapForSourceExchangeTick()					{ lastSwapForSourceExchangeTick = ::GetTickCount(); } // ZZ:DownloadManager
	*/
	//Xman end
	
	UINT	SetPrivateMaxSources(uint32 in)					{ return m_uMaxSources = in; } 
	UINT	GetPrivateMaxSources() const					{ return m_uMaxSources; } 
	UINT	GetMaxSources() const;
	UINT	GetMaxSourcePerFileSoft() const;
	UINT	GetMaxSourcePerFileUDP() const;

    bool    GetPreviewPrio() const							{ return m_bpreviewprio; }
	void    SetPreviewPrio(bool in)							{ m_bpreviewprio=in; }

	//Xman Xtreme Downloadmanager
	/*
    static bool RightFileHasHigherPrio(CPartFile* left, CPartFile* right);
	*/
	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	/*
	static bool RightFileHasHigherPrio(CPartFile* left, CPartFile* right, bool allow_go_over_hardlimit=false);
	*/
	static bool RightFileHasHigherPrio(const CPartFile* left, const CPartFile* right, bool allow_go_over_hardlimit=false);
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	//Xman end

	CDeadSourceList	m_DeadSourceList;

	//Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	void	CompDownloadRate();
	uint32	GetDownloadDatarate() const {return m_nDownDatarate;}	
	uint32	GetDownloadDatarate10() const {return m_nDownDatarate10;}	
	// Maella end

	//Xman Xtreme Downloadmanager
	void CalcAvgQr(UINT inqr,UINT oldqr);
	UINT GetAvgQr() const {return m_avgqr;}
	void	RemoveNoNeededPartsSources();
	void	RemoveQueueFullSources();
	void	RemoveLeecherSources(); //Xman Anti-Leecher
	//Xman end

	//Xman sourcecache
	void	ProcessSourceCache();
	void	AddToSourceCache(uint16 nPort, uint32 dwID, uint32 dwServerIP,uint16 nServerPort,ESourceFrom sourcefrom, bool ed2kIDFlag=false,  const uchar* achUserHash=NULL, uint8 byCryptOptions=0);
	void	ClearSourceCache();
	uint32	GetSourceCacheAmount() const { return m_sourcecache.GetCount();}
	//Xman end

	//Xman GlobalMaxHarlimit for fairness
	bool	IsGlobalSourceAddAllowed();
	bool	IsSourceSearchAllowed();
	//Xman end

	//Xman manual file allocation (Xanatos)
	void	AllocateNeededSpace();
	//zz_fly :: disable preallocate while eMule allocating, otherwise gui will freeze :: DolphinX :: start
	/*
	const bool	IncompleteAllocateSpace() const	{ return ((m_hpartfile.m_hFile != INVALID_HANDLE_VALUE) && m_hpartfile.GetLength() < GetFileSize()); } 
	*/
	const bool  IncompleteAllocateSpace() const { return ((!IsAllocating()) && (m_hpartfile.m_hFile != INVALID_HANDLE_VALUE) && m_hpartfile.GetLength() < GetFileSize()); }
	//zz_fly :: disable preallocate while eMule allocating :: end
	//Xman end

	//zz_fly :: Drop stalled downloads :: netfinity :: start
	bool	FindAndDropStalledDownload(CUpDownClient* ignore_client) const;
	uint64	GetUnrequestedSize() const;
	//zz_fly :: Drop stalled downloads :: netfinity :: end

#ifdef PRINT_STATISTIC
	uint32	GetSavedSources()	{return m_sourcesaver.GetSavedSources();}
	uint32	GetGapList()		{return gaplist.GetSize();}
	uint32  GetRequestedBlocklist() {return requestedblocks_list.GetCount();}
	uint32	GetSrcpartFrequency()	{return m_SrcpartFrequency.GetSize();}
	uint32	GetBufferedData()		{return m_BufferedData_list.GetSize();}
	uint32	GetA4AFsrclist()		{return A4AFsrclist.GetSize();}
#endif


#ifdef _DEBUG
	// Diagnostic Support
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	//Xman Dynamic block request (netfinity/Xman)
	/*
	bool	GetNextEmptyBlockInPart(UINT partnumber, Requested_Block_Struct* result) const;
	*/
	bool	GetNextEmptyBlockInPart(UINT partnumber, Requested_Block_Struct* result, uint64 bytesToRequest = EMBLOCKSIZE) const;
	//Xman end
	void	CompleteFile(bool hashingdone);
	void	CreatePartFile(UINT cat = 0);
	void	Init();

private:
	//Xman Xtreme Downloadmanager
	UINT	m_avgqr;
	uint32	m_sumqr;
	uint16	m_countqr;
	//Xman end
	BOOL 		PerformFileComplete(); // Lord KiRon
	static UINT CompleteThreadProc(LPVOID pvParams); // Lord KiRon - Used as separate thread to complete file
	static UINT AFX_CDECL AllocateSpaceThread(LPVOID lpParam);
	void		CharFillRange(CString* buffer,uint32 start, uint32 end, char color) const;
	//Xman
	void		ParseICHResult();	// SLUGFILLER: SafeHash


	CCorruptionBlackBox	m_CorruptionBlackBox;
	static CBarShader s_LoadBar;
	//Xman
	/*
	static CBarShader s_ChunkBar;
	uint32	m_iLastPausePurge;
	uint16	count;
	*/
	// ==> Make code VS 2005 and VS 2008 ready [MorphXT] - Stulle
	/*
	uint32	m_iLastPausePurge;
	*/
	time_t	m_iLastPausePurge;
	// <== Make code VS 2005 and VS 2008 ready [MorphXT] - Stulle
	//Xman end
	UINT	m_anStates[STATES_COUNT];
	EMFileSize	completedsize;
	uint64	m_uCorruptionLoss;
	uint64	m_uCompressionGain;
	uint32	m_uPartsSavedDueICH;
	//Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	/*
	uint32	datarate;
	*/
	//Xman end
	CString m_fullname;
	CString m_partmetfilename;
	uint64	m_uTransferred;
	UINT	m_uMaxSources;
	bool	paused;
	bool	m_bPauseOnPreview;
	bool	stopped;
	bool	insufficient;
	bool	m_bCompletionError;
	bool	m_bAICHPartHashsetNeeded;
	uint8	m_iDownPriority;
	bool	m_bAutoDownPriority;
	EPartFileStatus	status;
	bool	newdate;	// indicates if there was a writeaccess to the .part file
	uint32	lastpurgetime;
	uint32	m_LastNoNeededCheck;
	bool	m_is_A4AF_auto; //Xman Xtreme Downloadmanager: Auto-A4AF-check
	CTypedPtrList<CPtrList, Gap_Struct*> gaplist;
	CTypedPtrList<CPtrList, Requested_Block_Struct*> requestedblocks_list;
	CArray<uint16,uint16> m_SrcpartFrequency;
	//Xman
	// BEGIN SLUGFILLER: SafeHash
	CArray<bool,bool> m_PartsShareable;
	int	m_PartsHashing;
	CMutex	ICH_mut;	// ICH locks the file
	CList<UINT,UINT>	m_ICHPartsComplete;
	// END SLUGFILLER: SafeHash
	float	percentcompleted;
	CList<UINT,UINT>	corrupted_list;
	uint32	m_ClientSrcAnswered;
	UINT	availablePartsCount;
	CWinThread* m_AllocateThread;
	DWORD	m_lastRefreshedDLDisplay;
	CUpDownClientPtrList m_downloadingSourceList;
	//zz_fly :: delayed deletion of downloading source :: Enig123 :: Start
	CUpDownClientPtrList m_downloadingDeleteList;
	void	DoDelayedDeletion();
	volatile bool	m_sourceListChange; //Xman // Maella -New bandwidth control-
	//zz_fly :: delayed deletion of downloading source :: Enig123 :: End
	bool	m_bDeleteAfterAlloc;
    bool	m_bpreviewprio;
	// Barry - Buffered data to be written
	CTypedPtrList<CPtrList, PartFileBufferedData*> m_BufferedData_list;
	uint64	m_nTotalBufferData;
	uint32	m_nLastBufferFlushTime;
	UINT	m_category;
	DWORD	m_dwFileAttributes;
	time_t	m_tActivated;
	// ==> Make code VS 2005 and VS 2008 ready [MorphXT] - Stulle
	/*
	uint32	m_nDlActiveTime;
	*/
	time_t	m_nDlActiveTime;
	// <== Make code VS 2005 and VS 2008 ready [MorphXT] - Stulle
	uint32	m_tLastModified;	// last file modification time (NT's version of UTC), to be used for stats only!
	uint32	m_tCreated;			// file creation time (NT's version of UTC), to be used for stats only!
    uint32	m_random_update_wait;	
	volatile EPartFileOp m_eFileOp;
	volatile UINT m_uFileOpProgress;
	CAICHRecoveryHashSet*	m_pAICHRecoveryHashSet;

	//Xman Xtreme Downloadmanager
	/*
    DWORD   lastSwapForSourceExchangeTick; // ZZ:DownloadManaager

};
	*/
	//Xman end

	//Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	uint32 m_nDownDatarate;
	uint32 m_nDownDatarate10;
	// Maella end

	// Xman -New Save/load Sources- enkeyDEV(Ottavio84)
	CSourceSaver m_sourcesaver; 
	// Xman end

	//Xman sourcecache
	CList<PartfileSourceCache>	m_sourcecache;
	uint32						m_lastSoureCacheProcesstime;
	//Xman end

	//Xman
	//MORPH Added by SiRoB, Flush Thread
public:
	FlushDone_Struct* m_FlushSetting;
private:
	CWinThread* m_FlushThread; 
	CCriticalSection m_BufferedData_list_Locker;
	//Xman end

	// ==> Global Source Limit [Max/Stulle] - Stulle
private: 
	UINT	m_uFileHardLimit; 
public: 
	void	IncrHL(UINT m_uSourcesDif);
	void	DecrHL()	{m_uFileHardLimit = GetSourceCount();}
	void	SetPassiveHL(UINT m_uSourcesDif)	{m_uFileHardLimit = GetSourceCount() + m_uSourcesDif;};
	UINT	GetFileHardLimit() const {return m_uFileHardLimit;}
	void	InitHL();
	// <== Global Source Limit [Max/Stulle] - Stulle

	// ==> File Settings [sivka/Stulle] - Stulle
	bool	GetEnableAutoDropNNS() const {return m_EnableAutoDropNNS;}
	DWORD	GetAutoNNS_Timer() const {return m_AutoNNS_Timer;}
	uint16	GetMaxRemoveNNSLimit() const {return m_MaxRemoveNNSLimit;}
	bool	GetEnableAutoDropFQS() const {return m_EnableAutoDropFQS;}
	DWORD	GetAutoFQS_Timer() const {return m_AutoFQS_Timer;}
	uint16	GetMaxRemoveFQSLimit() const {return m_MaxRemoveFQSLimit;}
	bool	GetEnableAutoDropQRS() const {return m_EnableAutoDropQRS;}
	DWORD	GetAutoHQRS_Timer() const {return m_AutoHQRS_Timer;}
	uint16	GetMaxRemoveQRS() const {return m_MaxRemoveQRS;}
	uint16	GetMaxRemoveQRSLimit() const {return m_MaxRemoveQRSLimit;}
	bool	GetHQRXman() const {return m_bHQRXman;}
	bool	GetGlobalHL() const {return m_bGlobalHL;} // Global Source Limit (customize for files) - Stulle
	void	SetEnableAutoDropNNS(bool in){m_EnableAutoDropNNS=in;}
	void	SetAutoNNS_Timer(DWORD in){m_AutoNNS_Timer=in;}
	void	SetMaxRemoveNNSLimit(uint16 in){m_MaxRemoveNNSLimit=in;}
	void	SetEnableAutoDropFQS(bool in){m_EnableAutoDropFQS=in;}
	void	SetAutoFQS_Timer(DWORD in){m_AutoFQS_Timer=in;}
	void	SetMaxRemoveFQSLimit(uint16 in){m_MaxRemoveFQSLimit=in;}
	void	SetEnableAutoDropQRS(bool in){m_EnableAutoDropQRS=in;}
	void	SetAutoHQRS_Timer(DWORD in){m_AutoHQRS_Timer=in;}
	void	SetMaxRemoveQRS(uint16 in){m_MaxRemoveQRS=in;}
	void	SetMaxRemoveQRSLimit(uint16 in){m_MaxRemoveQRSLimit=in;}
	void	SetHQRXman(bool in){m_bHQRXman=in;}
	void	SetGlobalHL(bool in){m_bGlobalHL=in;} // Global Source Limit (customize for files) - Stulle

private:
	bool	m_EnableAutoDropNNS;
	DWORD	m_AutoNNS_Timer;
	uint16	m_MaxRemoveNNSLimit;
	bool	m_EnableAutoDropFQS;
	DWORD	m_AutoFQS_Timer;
	uint16	m_MaxRemoveFQSLimit;
	bool	m_EnableAutoDropQRS;
	DWORD	m_AutoHQRS_Timer;
	uint16	m_MaxRemoveQRS;
	uint16	m_MaxRemoveQRSLimit;
	bool	m_bHQRXman;
	bool	m_bGlobalHL; // Global Source Limit (customize for files) - Stulle
	// <== File Settings [sivka/Stulle] - Stulle

	// ==> customized source dropping - Stulle
	DWORD	m_TimerForAutoNNS;
	DWORD	m_TimerForAutoFQS;
	DWORD	m_TimerForAutoHQRS;
	uint16	m_RemoteQueueRank_Old;
	uint16	m_RemoteQueueRank_New;
	// <== customized source dropping - Stulle

	// ==> show # of dropped sources - Stulle
public:
	uint16	GetShowDroppedSrc() const {return m_ShowDroppedSrc;}
private:
	uint16	m_ShowDroppedSrc;
	// <== show # of dropped sources - Stulle

	// ==> advanced manual dropping - Stulle
public:
	void	RemoveLow2LowIPSourcesManual();
	void	RemoveUnknownErrorBannedSourcesManual();
	void	RemoveHighQRSourcesManualSivka();
	void	RemoveHighQRSourcesManualXman();
	void	CleanUp_NNS_FQS_NONE_ERROR_BANNED_LOWTOLOWIP_Sources();
	// <== advanced manual dropping - Stulle

	// ==> Source Counts Are Cached derivated from Khaos [SiRoB] - Stulle
	UINT	GetAvailableSrcCount() const;
private:
	UINT	m_anStatesTemp[STATES_COUNT];
	// <== Source Counts Are Cached derivated from Khaos [SiRoB] - Stulle

	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	UINT	m_catResumeOrder;
public:
	void	SetCatResumeOrder(UINT order)	{ m_catResumeOrder = order; SavePartFile(); }
	UINT	GetCatResumeOrder() const				{ return m_catResumeOrder; }
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

	int		GetPfStyle() const; // Design Settings [eWombat/Stulle] - Stulle

	// ==> Follow The Majority [AndCycle/Stulle] - Stulle
private:
	int	m_iFollowTheMajority;
	CMap<CUpDownClient*, CUpDownClient*, CString, CString> m_mapSrcFilename;
	CMap<CString, LPCTSTR, int, int> m_mapFilenameCount;
public:
	void	UpdateSourceFileName(CUpDownClient* src);
	void	RemoveSourceFileName(CUpDownClient* src);
	int		GetFollowTheMajority() const { return m_iFollowTheMajority; }
	void	SetFollowTheMajority(int val) { m_iFollowTheMajority = val; }
	// <== Follow The Majority [AndCycle/Stulle] - Stulle
};

//Xman
// BEGIN SLUGFILLER: SafeHash
class CPartHashThread : public CWinThread
{
	DECLARE_DYNCREATE(CPartHashThread)
protected:
	CPartHashThread()	{}
public:
	virtual	BOOL	InitInstance() {return true;}
	virtual int		Run();
	int	SetFirstHash(CPartFile* pOwner);
	void	SetSinglePartHash(CPartFile* pOwner, UINT part, bool ICHused = false, bool AICHRecover = false);
private:
	CPartFile*				m_pOwner;
	bool					m_ICHused;
	bool					m_AICHRecover;
	CString					directory;
	CString					filename;
	CArray<UINT,UINT>	m_PartsToHash;
	CArray<uchar*,uchar*>	m_DesiredHashes;
	CArray<CAICHHashTree*,CAICHHashTree*>	m_phtAICHPartHash;
	CArray<CAICHHash,CAICHHash>	m_DesiredAICHHashes;
};
// END SLUGFILLER: SafeHash

// BEGIN SiRoB: Flush Thread
class CPartFileFlushThread : public CWinThread
{
	DECLARE_DYNCREATE(CPartFileFlushThread)
protected:
	CPartFileFlushThread()	{}
public:
	virtual	BOOL	InitInstance() {return true;}
	virtual int		Run();
	void	SetPartFile(CPartFile* pOwner);
	void	StopFlush();
private:
	CPartFile*				m_partfile;
	CEvent					pauseEvent;
	volatile bool			doRun;
};
// END SiRoB: Flush Thread
