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
#include "Runnable.h"

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
	PS_COMPLETE			= 9,
// X-Ray :: FileStatusIcons :: Start
	PS_WAITINGFORSOURCE	= 10,
	PS_DOWNLOADING		= 11,
	PS_STOPPED			= 12
// X-Ray :: FileStatusIcons :: End
};

#define PR_VERYLOW			4 // I Had to change this because it didn't save negative number correctly.. Had to modify the sort function for this change..
#define PR_LOW				0 //*
#define PR_NORMAL			1 // Don't change this - needed for edonkey clients and server!
#define PR_HIGH				2 //*
#define PR_VERYHIGH			3
#define PR_AUTO				5 //UAP Hunter
#define PR_POWER			6 //Xman PowerRelease

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
};

class CSearchFile;
class CUpDownClient;
enum EDownloadState;
class CxImage;
class CSafeMemFile;
struct FileFmt_Struct;

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

typedef CAtlList<CUpDownClient*> CUpDownClientPtrList;

class CPartFile : public CKnownFile
{
	//DECLARE_DYNAMIC(CPartFile)

	friend class CPartFileConvert;
	//Xman
	friend class CPartHashThread;	// SLUGFILLER: SafeHash

	friend class CPartFileFlushThread; //Xman Flush Thread

public:
	CPartFile(size_t cat = 0);
	CPartFile(CSearchFile* searchresult, size_t cat = 0);
	CPartFile(CString edonkeylink, size_t cat = 0);
	CPartFile(class CED2KFileLink* fileLink, size_t cat = 0, LPCTSTR filename = NULL);// X: [IP] - [Import Parts]
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
	CTime	GetCFileDate() const { return CTime(m_tLastModified); }// X: [64T] - [64BitTime]
	uint64	GetFileDate() const { return m_tLastModified; }// X: [64T] - [64BitTime]

	// file creation time (NT's version of UTC), to be used for stats only!
	CTime	GetCrCFileDate() const { return CTime(m_tCreated); }// X: [64T] - [64BitTime]
	uint64	GetCrFileDate() const { return m_tCreated; }// X: [64T] - [64BitTime]

	void	InitializeFromLink(CED2KFileLink* fileLink, size_t cat = 0, LPCTSTR filename = NULL);// X: [IP] - [Import Parts]
	//uint32	Process(uint32 reducedownload, UINT icounter);
	// Maella -New bandwidth control-
	uint32	Process(uint32 maxammount, bool isLimited, bool fullProcess); // in byte, not in percent
	// Maella end
	//Xman end	
	EPartFileLoadResult	LoadPartFile(LPCTSTR in_directory, LPCTSTR filename, EPartFileFormat* pOutCheckFileFormat = NULL); //filename = *.part.met
	EPartFileLoadResult	ImportShareazaTempfile(LPCTSTR in_directory,LPCTSTR in_filename, EPartFileFormat* pOutCheckFileFormat = NULL);

	bool	SavePartFile(bool bDontOverrideBak = false);
	void	PartFileHashFinished(CKnownFile* result);
	bool	HashSinglePart(uint_ptr partnumber, bool* pbAICHReportedOK = NULL); // true = ok , false = corrupted
	//Xman
	// BEGIN SLUGFILLER: SafeHash - replaced old handlers, full hash checker remains for file completion
	void	PartHashFinished(uint_ptr partnumber, bool bAICHAgreed, bool corrupt);
	void	PartHashFinishedAICHRecover(uint_ptr partnumber, bool corrupt);
	bool	IsPartShareable(uint_ptr partnumber) const;
	bool	IsRangeShareable(uint64 start, uint64 end) const;
	// END SLUGFILLER: SafeHash
	void	AddGap(uint64 start, uint64 end);
	void	FillGap(uint64 start, uint64 end);
	void	DrawStatusBar(CDC* dc, LPCRECT rect, bool bFlat)/* const*/;
	virtual void	DrawShareStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool	 bFlat) const;
	uint64	GetHeaderSizeOnDisk() const;// X: [HP] - [HeaderPercent]
	bool	IsComplete(uint64 start, uint64 end, bool bIgnoreBufferedData) const;
	bool	IsPureGap(uint64 start, uint64 end) const;
	bool	IsAlreadyRequested(uint64 start, uint64 end, bool bCheckBuffers = false) const;
    bool    ShrinkToAvoidAlreadyRequested(uint64& start, uint64& end) const;
	bool	IsCorruptedPart(uint_ptr partnumber) const;
	//Xman Dynamic block request (netfinity/morph/Xman)
//	uint64	GetRemainingAvailableData(const uint8* srcstatus) const;
//	uint64	GetRemainingAvailableData(const CUpDownClient* sender) const;
	uint32  GetDownloadSpeedInPart(uint16 forpart, CUpDownClient* current_source) const;
	//Xman end
	uint64	GetTotalGapSizeInRange(uint64 uRangeStart, uint64 uRangeEnd) const;
	uint64	GetTotalGapSizeInPart(uint_ptr uPart) const;
	void	UpdateCompletedInfos();
	void	UpdateCompletedInfos(uint64 uTotalGaps);
	virtual void	UpdatePartsInfo();
	//Xman chunk chooser
	//bool	GetNextRequestedBlock_Maella(CUpDownClient* sender, Requested_Block_Struct** newblocks, uint16* count) /*const*/;
	//bool	GetNextRequestedBlock_zz(CUpDownClient* sender, Requested_Block_Struct** newblocks, uint16* count) /*const*/;
	//Xman end
	bool	GetNextRequestedBlock(CUpDownClient* sender, Requested_Block_Struct** newblocks, uint16* count) /*const*/;// X: [Ord] - [Order]
	void	WritePartStatus(CSafeMemFile* file) const;
	void	WriteCompleteSourcesCount(CSafeMemFile* file) const;
	void	AddSources(CSafeMemFile* sources,uint32 serverip, uint16 serverport, bool bWithObfuscationAndHash);
	void	AddSource(LPCTSTR pszURL, uint32 nIP);
	static bool CanAddSource(uint32 userid, uint16 port, uint32 serverip, uint16 serverport, uint_ptr* pdebug_lowiddropped = NULL, bool Ed2kID = true);
	
	EPartFileStatus	GetStatus(bool ignorepause = false) const;
	EPartFileStatus	GetPartFileStatus(); // X-Ray :: FileStatusIcons
	void	SetStatus(EPartFileStatus eStatus);		// set status and update GUI
	void	_SetStatus(EPartFileStatus eStatus);	// set status and do *not* update GUI
	void	NotifyStatusChange();
	void	ClearClient();// X: [C0SC] - [Clear0SpeedClient]
	bool	IsStopped() const												{ return stopped; }
	bool	GetCompletionError() const										{ return m_bCompletionError; }
	EMFileSize  GetCompletedSize() const									{ return completedsize; }
	CString getPartfileStatus() const;
	size_t	getPartfileStatusRang() const;
	void	SetActive(bool bActive);

	uint8	GetDownPriority() const											{ return m_iDownPriority; }
	void	SetDownPriority(uint8 iNewDownPriority, bool resort = true);
	bool	IsAutoDownPriority(void) const									{ return m_bAutoDownPriority; }
	void	SetAutoDownPriority(bool NewAutoDownPriority)					{ m_bAutoDownPriority = NewAutoDownPriority; }
	void	UpdateAutoDownPriority();

//morph4u min. queuerank downloadlistctrl + 
	UINT    CPartFile::GetMinQR() const; 
//morph4u min. queuerank in downloadlistctrl - 

	size_t	GetSourceCount() const { return srclist.GetCount(); }
	size_t	GetSrcA4AFCount() const { return A4AFsrclist.GetCount(); }
	UINT	GetSrcStatisticsValue(EDownloadState nDLState) const;
	UINT	GetTransferringSrcCount() const;
	uint64	GetTransferred() const											{ return m_uTransferred; }
	//uint32	GetDatarate() const { return datarate; } //Xman
	float	GetPercentCompleted() const										{ return percentcompleted; }
	size_t	GetNotCurrentSourcesCount() const;
	UINT	GetValidSourcesCount() const;
	bool	IsArchive(bool onlyPreviewable = false) const; // Barry - Also want to preview archives
///    bool    IsPreviewableFileType() const;
	uint_ptr	getTimeRemaining() const;
	uint_ptr	getTimeRemainingSimple() const;
	uint_ptr	GetDlActiveTime() const;// X: [64T] - [64BitTime]

	// Barry - Added as replacement for BlockReceived to buffer data before writing to disk
	uint32	WriteToBuffer(uint64 transize, const BYTE *data, uint64 start, uint64 end, Requested_Block_Struct *block, const CUpDownClient* client);
	void	FlushBuffer(bool forcewait=false, bool bForceICH = false, bool bOnlyCompletedPart = false);// X: [GB] - [Global Buffer]
	//Xman
	// BEGIN SiRoB: Flush Thread
	void	FlushDone();
	// END SiRoB: Flush Thread
	// Barry - This will invert the gap list, up to caller to delete gaps when done
	// 'Gaps' returned are really the filled areas, and guaranteed to be in order
	void	GetFilledList(CAtlList<Gap_Struct*> *filled) const;

	// Barry - Added to prevent list containing deleted blocks on shutdown
	void	RemoveAllRequestedBlocks(void);
	bool	RemoveBlockFromList(const Requested_Block_Struct* pblock);//Enig123::Optimizations - from MORPH
	bool	RemoveBlockFromList(uint64 start, uint64 end);
	bool	IsInRequestedBlockList(const Requested_Block_Struct* block) const;
	void	RemoveAllSources(bool bTryToSwap);

	bool	CanOpenFile() const;
	bool	IsReadyForPreview() const;
	bool	CanStopFile() const;
	bool	CanPauseFile() const;
	bool	CanResumeFile() const;
	bool	IsPausingOnPreview() const										{ return m_bPauseOnPreview/* && IsPreviewableFileType()*/ && CanPauseFile(); }

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

	UINT	GetAvailablePartCount() const									{ return availablePartsCount; }
	void	UpdateAvailablePartsCount();

	uint32	GetLastAnsweredTime() const										{ return m_ClientSrcAnswered; }
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

	size_t	GetCategory() /*const*/;
	size_t	GetConstCategory() const;  //Xman checkmark to catogory at contextmenu of downloadlist
	void	SetCategory(size_t cat);
	//bool	HasDefaultCategory() const;
	bool	CheckShowItemInGivenCat(size_t inCategory) /*const*/;

	uint8*	MMCreatePartStatus();

	//preview
	virtual bool GrabImage(uint8 nFramesToGrab, double dStartTime, bool bReduceColor, uint16 nMaxWidth,void* pSender);
	virtual void GrabbingFinished(CxImage** imgResults, uint8 nFramesGrabbed, void* pSender);

	void	FlushBuffersExceptionHandler(CFileException* error);
	void	FlushBuffersExceptionHandler();

	void	PerformFileCompleteEnd(DWORD dwResult);
	//Xman
	void	PerformFirstHash();		// SLUGFILLER: SafeHash

	void	SetFileOp(EPartFileOp eFileOp){m_eFileOp = eFileOp;}
	EPartFileOp GetFileOp() const											{ return m_eFileOp; }
	void	SetFileOpProgress(UINT uProgress){ASSERT( uProgress <= 100 );m_uFileOpProgress = uProgress;}
	UINT	GetFileOpProgress() const										{ return m_uFileOpProgress; }

	CAICHRecoveryHashSet* GetAICHRecoveryHashSet()							{ return m_pAICHRecoveryHashSet; }
	void	RequestAICHRecovery(uint_ptr nPart, bool failed); // X: [IP] - [Import Parts] & [IPR] - [Improved Part Recovery]
	void	AICHRecoveryDataAvailable(uint_ptr nPart);
	bool	IsAICHPartHashSetNeeded() const									{ return m_FileIdentifier.HasAICHHash() && !m_FileIdentifier.HasExpectedAICHHashCount() && m_bAICHPartHashsetNeeded; }
	void	SetAICHHashSetNeeded(bool bVal)									{ m_bAICHPartHashsetNeeded = bVal; }

	FileFmt_Struct*GetFileFormat();// X: [FV] - [FileVerify]
	bool	CanDropClient();// X: [DSC] - [Drop Slow Client]
	bool	DropSlowClient(CUpDownClient* client, uint_ptr DownloadDatarate10) const;// X: [DSC] - [Drop Slow Client]
	uint32	m_LastSearchTime;
	uint32	m_LastSearchTimeKad;
	uint64	m_nTotalBufferData;
	CUpDownClientPtrList srclist;
	CUpDownClientPtrList A4AFsrclist; //<<-- enkeyDEV(Ottavio84) -A4AF-
	CTime	lastseencomplete;
	CFile	m_hpartfile;				// permanent opened handle to avoid write conflicts
	Poco::FastMutex	m_FileCompleteMutex;		// Lord KiRon - Mutex for file completion
	uint16	src_stats[4];
	uint16  net_stats[3];
	volatile bool m_bPreviewing;
	volatile bool m_bRecoveringArchive; // Is archive recovery in progress
	bool	m_bLocalSrcReqQueued;
	bool	srcarevisible;				// used for downloadlistctrl
	bool	m_bMD4HashsetNeeded;
	uint8	m_TotalSearchesKad;
	//bool    AllowSwapForSourceExchange() { return ::GetTickCount()-lastSwapForSourceExchangeTick > 30*1000; } // ZZ:DownloadManager //Xman
    //void    SetSwapForSourceExchangeTick() { lastSwapForSourceExchangeTick = ::GetTickCount(); } // ZZ:DownloadManager //Xman
	void	SetPrivateMaxSources(uint32 in)	{ m_uMaxSources = in; } 
	UINT	GetPrivateMaxSources() const	{ return m_uMaxSources; } 
	UINT	GetMaxSources() const;
	UINT_PTR	GetMaxSourcePerFileSoft() const;
	UINT_PTR	GetMaxSourcePerFileUDP() const;

    bool    GetPreviewPrio() const { return m_bpreviewprio; }
	void    SetPreviewPrio(bool in) { m_bpreviewprio=in; }

	static bool RightFileHasHigherPrio(CPartFile* left, CPartFile* right, bool allow_go_over_hardlimit=false); //Xman Xtreme Downloadmanager
	//static bool RightFileHasHigherPrio(CPartFile* left, CPartFile* right);

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
	size_t	GetSourceCacheAmount() const { return m_sourcecache.GetCount();}
	//Xman end

	//Xman GlobalMaxHarlimit for fairness
	bool	IsGlobalSourceAddAllowed();
	bool	IsSourceSearchAllowed();
	//Xman end

	//Xman manual file allocation (Xanatos)
	void	AllocateNeededSpace();
	const bool	IncompleteAllocateSpace() const	{ return ((m_AllocateThread == NULL)// X: [BF] - [Bug Fix] fix gui dead, don't check filesize when allocating
		&& (m_hpartfile.m_hFile != INVALID_HANDLE_VALUE) && m_hpartfile.GetLength() < GetFileSize()); }
	//Xman end
	uint32	xState;// X: [IP] - [Import Parts],[POFC] - [PauseOnFileComplete]
	uint32	lastAICHRequestFailed;// X: [IPR] - [Improved Part Recovery]
	uint8	verifystatus;// X: [FV] - [FileVerify]
	bool	datareceived;// X: [GB] - [Global Buffer]
	uint64	headerSize;// X: [HP] - [HeaderPercent]
	uint64 leftsize;
	uint32	m_nNextFlushBufferTime;// X: [GB] - [Global Buffer]

	// Xman -New Save/load Sources- enkeyDEV(Ottavio84)
	CSourceSaver m_sourcesaver; // X: [ISS] - [Improved Source Save] move up
	// Xman end

// X-Ray :: SessionDownload :: Start
	float	GetPercentCompletedInitial() const {return percentcompletedinitial;}
	uint64	GetTransferredSession() const { return m_uTransferredSession; }
	// X-Ray :: SessionDownload :: End

#ifdef _DEBUG
	size_t	GetSavedSources()	{return m_sourcesaver.GetSavedSources();}
	size_t	GetGapList()		{return gaplist.GetCount();}
	size_t  GetRequestedBlocklist() {return requestedblocks_list.GetCount();}
	size_t	GetSrcpartFrequency()	{return m_SrcpartFrequency.GetCount();}
	size_t	GetBufferedData()		{return m_BufferedData_list.GetCount();}
	size_t	GetA4AFsrclist()		{return A4AFsrclist.GetCount();}
#endif

#ifdef _DEBUG
	// Diagnostic Support
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	bool	GetNextEmptyBlockInPart(uint_ptr partnumber, Requested_Block_Struct* result, uint64 bytesToRequest = EMBLOCKSIZE) const; //Xman Dynamic block request (netfinity/Xman)
	//bool	GetNextEmptyBlockInPart(uint_ptr partnumber, Requested_Block_Struct* result) const;
	void	CompleteFile(bool hashingdone);
	void	CreatePartFile(LPCTSTR fileName = NULL);// X: [IP] - [Import Parts]
	void	Init(size_t ucat);// X: [IP] - [Import Parts]
	void	SetAutoCat();

private:
	//Xman Xtreme Downloadmanager
	UINT	m_avgqr;
	uint32	m_sumqr;
	uint16	m_countqr;
	//Xman end
	BOOL 		PerformFileComplete(); // Lord KiRon
	class CompleteThread : public Poco::Thread
	{
	public:
		CompleteThread(CPartFile* _pFile) : Thread(true, PRIO_LOW), pFile(_pFile){}
		virtual void run(); // Lord KiRon - Used as separate thread to complete file
	private:
		CPartFile* pFile;
	};
	class AllocateSpaceThread : public Poco::Thread
	{
	public:
		AllocateSpaceThread(CPartFile* _pFile, uint64 iAllocinfo) : Thread(true, PRIO_LOWEST), myfile(_pFile), m_iAllocinfo(iAllocinfo){}
		virtual void run();
	private:
		CPartFile* myfile;
		uint64	m_iAllocinfo;
	};
	void		CharFillRange(CString* buffer,uint32 start, uint32 end, char color) const;
	//Xman
	void		ParseICHResult();	// SLUGFILLER: SafeHash
	void StopOnFileComplete();// X: [POFC] - [PauseOnFileComplete]

	CCorruptionBlackBox	m_CorruptionBlackBox;
	static CRectShader s_LoadBar/*,s_CacheBar*/;// X: [CI] - [Code Improvement] BarShader
	//static CBarShader s_ChunkBar; //Xman
	uint64	m_iLastPausePurge;// X: [64T] - [64BitTime]
	//uint16	count; //Xman not used
	UINT	m_anStates[STATES_COUNT];
	EMFileSize	completedsize;
	uint64	m_uCorruptionLoss;
	uint64	m_uCompressionGain;
	uint32	m_uPartsSavedDueICH;
	//uint32	datarate;
	CString m_fullname;
	CString m_partmetfilename;
	uint64	m_uTransferred;
	UINT	m_uMaxSources;
	bool	paused;
	bool	m_bPauseOnPreview;
	bool	stopped;
	bool	insufficient;
	bool	bIncreasedFile;
	bool	m_bCompletionError;
	bool	m_bAICHPartHashsetNeeded;
	uint8	m_iDownPriority;
	bool	m_bAutoDownPriority;
	EPartFileStatus	status;
	//bool	newdate;	// indicates if there was a writeaccess to the .part file
	uint32	lastpurgetime;
	uint32	m_LastNoNeededCheck; //Xman Xtreme Downloadmanager: Auto-A4AF-check
	bool	m_is_A4AF_auto; //Xman Xtreme Downloadmanager: Auto-A4AF-check
	CAtlList<Gap_Struct*> gaplist;
	CAtlList<Requested_Block_Struct*> requestedblocks_list;
	CAtlArray<uint16> m_SrcpartFrequency;
	//Xman
	// BEGIN SLUGFILLER: SafeHash
	CAtlArray<bool> m_PartsShareable;
	uint16	m_PartsHashing;
	Poco::FastMutex	ICH_mut;	// ICH locks the file
	CAtlList<uint16>	m_ICHPartsComplete;
	// END SLUGFILLER: SafeHash
	float	percentcompleted;
	CAtlList<uint16> corrupted_list;
	uint32	m_ClientSrcAnswered;
	UINT	availablePartsCount;
	AllocateSpaceThread* m_AllocateThread;
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
	CAtlList<PartFileBufferedData*> m_BufferedData_list;
	//uint32	m_nLastBufferFlushTime;// X: [GB] - [Global Buffer]
	size_t	m_category;
	DWORD	m_dwFileAttributes;
	time_t	m_tActivated;
	uint_ptr	m_nDlActiveTime;// X: [64T] - [64BitTime]
	uint64	m_tLastModified;	// last file modification time (NT's version of UTC), to be used for stats only!
	uint64	m_tCreated;			// file creation time (NT's version of UTC), to be used for stats only!
//    uint32	m_random_update_wait;	
	volatile EPartFileOp m_eFileOp;
	volatile UINT m_uFileOpProgress;
	CAICHRecoveryHashSet*	m_pAICHRecoveryHashSet;
	//Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	uint32 m_nDownDatarate;
	uint32 m_nDownDatarate10;
	// Maella end

    //DWORD   lastSwapForSourceExchangeTick; // ZZ:DownloadManaager

	//Xman sourcecache
	CAtlList<PartfileSourceCache>	m_sourcecache;
	uint32						m_lastSoureCacheProcesstime;
	//Xman end

	// X-Ray :: SessionDownload :: Start
	float   percentcompletedinitial;
	uint64	m_uTransferredSession;
	// X-Ray :: SessionDownload :: End

	//Xman
	//MORPH Added by SiRoB, Flush Thread
	CPartFileFlushThread* m_FlushThread; 
	//Xman end
	FileFmt_Struct*formatInfo;// X: [FV] - [FileVerify]
	uint64 completedHeaderX;// X: [FV] - [FileVerify]

// ZZUL-TRA :: SOTN :: Start
public:
	uint16	GetSrcPartFrequency(const uint16& part)	{return m_SrcpartFrequency[part];}
// ZZUL-TRA :: SOTN :: End
};

//Xman
// BEGIN SLUGFILLER: SafeHash
class CPartHashThread : public Poco::Thread
{
public:
	CPartHashThread(CPartFile* pOwner, uint16 part = -1, bool ICHused = false, bool AICHRecover = false);
	virtual void run();
private:
	CPartFile*				m_pOwner;
	bool					m_ICHused;
	bool					m_AICHRecover;
	CString					directory;
	CString					filename;
	CAtlArray<uint16>	m_PartsToHash;
};
// END SLUGFILLER: SafeHash

// BEGIN SiRoB: Flush Thread
class CPartFileFlushThread : public Runnable
{
public:
	CPartFileFlushThread(CPartFile* pOwner);
	~CPartFileFlushThread();
	virtual bool run();
	bool	bForceICH;
	bool*	changedPart;
	Poco::Event evTerminated;
private:
	CPartFile*				m_partfile;
};
// END SiRoB: Flush Thread
