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
#include "Neo/BlockMaps.h" // NEO: SCV - [SubChunkVerification] <-- Xanatos --
class CClientFileStatus; // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --

enum EPartFileStatus{
	PS_READY			= 0,
	PS_EMPTY			= 1,
	//PS_WAITINGFORHASH	= 2, // NEO: SSH - [SlugFillerSafeHash] <-- Xanatos --
	PS_HASHING			= 3,
	PS_ERROR			= 4,
	PS_INSUFFICIENT		= 5,
	PS_UNKNOWN			= 6,
	PS_PAUSED			= 7,
	PS_COMPLETING		= 8,
	PS_COMPLETE			= 9,
	PS_IMPORTING		= 10, // NEO: PIX - [PartImportExport] <-- Xanatos --
	PS_MOVING			= 11 // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
};

#define PR_VERYLOW			4 // I Had to change this because it didn't save negative number correctly.. Had to modify the sort function for this change..
#define PR_LOW				0 //*
#define PR_NORMAL			1 // Don't change this - needed for edonkey clients and server!
#define PR_HIGH				2 //*
#define PR_VERYHIGH			3
#define PR_AUTO				5 //UAP Hunter

//#define BUFFER_SIZE_LIMIT 500000 // Max bytes before forcing a flush
#define BUFFER_TIME_LIMIT	60000	// Max milliseconds before forcing a flush

#define	PARTMET_BAK_EXT	_T(".bak")
#define	PARTMET_TMP_EXT	_T(".backup")
#define	PARTMET_EXT		_T(".met") // NEO: MOD <-- Xanatos --

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
#define	PARTSRC_EXT		_T(".src")
#define	PARTSRC_BAK_EXT	_T(".bak")
#define	PARTSRC_TMP_EXT	_T(".backup")
#endif // NEO_SS // NEO: NSS END <-- Xanatos --

#define STATES_COUNT		21 // NEO: FIX - [SourceCount] <-- Xanatos --

enum EPartFileFormat{
	PMT_UNKNOWN			= 0,
	PMT_DEFAULTOLD,
	PMT_SPLITTED,
	PMT_NEWOLD,
	PMT_SHAREAZA,
	PMT_BADFORMAT	
};

#define	FILE_COMPLETION_THREAD_FAILED	0x0000
#define	FILE_COMPLETION_THREAD_SUCCESS	0x0001
#define	FILE_COMPLETION_THREAD_RENAMED	0x0002
// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
#define	FILE_MOVE_THREAD_FAILED			0x0000
#define	FILE_MOVE_THREAD_SUCCESS		0x0001
// NEO: MTD END <-- Xanatos --

enum EPartFileOp{
	PFOP_NONE = 0,
	PFOP_HASHING,
	PFOP_COPYING,
	PFOP_IMPORTING, // NEO: PIX - [PartImportExport] <-- Xanatos --
	PFOP_UNCOMPRESSING
};

class CSearchFile;
class CUpDownClient;
enum EDownloadState;
class CxImage;
class CSafeMemFile;
struct ImportInfo; // NEO: PIX - [PartImportExport] <-- Xanatos --
class CPartPreferences; // NEO: FCFG - [FileConfiguration] <-- Xanatos --

#pragma pack(1)
struct Requested_Block_Struct
{
	uint64	StartOffset;
	uint64	EndOffset;
	uchar	FileID[16];
	uint64  transferred; // Barry - This counts bytes completed
	byte*	filedata; // NEO: RBT - [ReadBlockThread] <-- Xanatos --
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	uint32	timeout;
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
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
	//Requested_Block_Struct *block;	// Barry - This is the requested block that this data relates to  // NEO: SSH - [SlugFillerSafeHash] <-- Xanatos --
};

//MORPH START - Added by SiRoB, Flush Thread // NEO: FFT - [FileFlushThread] -- Xanatos -->
struct FlushDone_Struct
{
	bool	bIncreasedFile;
	bool	bForceICH;
	//bool	bNoAICH; // SLUGFILLER: SafeHash // NEO: SSH - [SlugFillerSafeHash] <-- Xanatos --
	bool*	changedPart;
};
//MORPH END   - Added by SiRoB, Flush Thread // NEO: FFT END <-- Xanatos --

// BEGIN SLUGFILLER: SafeHash // NEO: SSH - [SlugFillerSafeHash] -- Xanatos -->
struct PartHashOrder
{
	PartHashOrder(uint16 uPart_,const uchar* Hash_,bool ICHused_ = false, bool AICHRecover_ = false, bool AICHok_ = false)
	{
		uPart = uPart_;
		md4cpy(&Hash[0],Hash_);
		ICHused = ICHused_;
		AICHRecover = AICHRecover_;
		AICHok = AICHok_;
	}
	uint16	uPart;
	uchar	Hash[16];
	bool	ICHused;
	bool	AICHRecover;
	bool	AICHok;
};

struct PartHashResult
{
	PartHashResult(uint16 uPart_,bool corrupted_,bool AICHRecover_, bool AICHok_) 
	{
		uPart = uPart_; 
		corrupted = corrupted_; 
		AICHRecover = AICHRecover_;
		AICHok = AICHok_;
	}
	uint16	uPart;
	bool	corrupted;
	bool	AICHRecover;
	bool	AICHok;
};
// END SLUGFILLER: SafeHash // NEO: SSH END <-- Xanatos --

// NEO: SCV - [SubChunkVerification] -- Xanatos -->
struct BlockHashOrder
{
	BlockHashOrder(uint16 uPart_,bool AICHRecover_ = false, bool ForceMD4_ = false)
	{
		uPart = uPart_;
		AICHRecover = AICHRecover_;
		ForceMD4 = ForceMD4_;
	}
	uint16	uPart;
	CList<uint8,uint8> BlocksToHash;
	bool	AICHRecover;
	bool	ForceMD4;
};

struct BlockHashResult
{
	BlockHashResult(uint16 uPart_,CMap<uint8,uint8,BOOL,BOOL>* resultMap_,bool AICHRecover_, bool ForceMD4_) 
	{
		resultMap = resultMap_; 
		uPart = uPart_; 
		AICHRecover = AICHRecover_;
		ForceMD4 = ForceMD4_;
	}
	CMap<uint8,uint8,BOOL,BOOL>* resultMap;
	uint16					uPart;
	bool					AICHRecover;
	bool					ForceMD4;
};
// NEO: SCV END <-- Xanatos --

// NEO: PIX - [PartImportExport] -- Xanatos -->
struct ImportPart_Struct {
	uint64 start;
	uint64 end;
	BYTE* data;
};
// NEO: PIX END <-- Xanatos --

typedef CTypedPtrList<CPtrList, CUpDownClient*> CUpDownClientPtrList;

class CPartFile : public CKnownFile
{
	DECLARE_DYNAMIC(CPartFile)

	friend class CPartFileConvert;
	friend class CPartHashThread;	// SLUGFILLER: SafeHash // NEO: SSH - [SlugFillerSafeHash] <-- Xanatos --
	friend class CBlockHashThread;	// NEO: SafeHash // NEO: SCV - [SubChunkVerification] <-- Xanatos --
	friend class CPartFileFlushThread;//MORPH START - Added by SiRoB, Flush Thread // NEO: FFT - [FileFlushThread] <-- Xanatos --
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	friend class CVoodooSocket;
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

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

	// last file modification time (NT's version of UTC), to be used for stats only!
	CTime	GetCFileDate() const { return CTime(m_tLastModified); }
	uint32	GetFileDate() const { return m_tLastModified; }

	// file creation time (NT's version of UTC), to be used for stats only!
	CTime	GetCrCFileDate() const { return CTime(m_tCreated); }
	uint32	GetCrFileDate() const { return m_tCreated; }

	void	InitializeFromLink(CED2KFileLink* fileLink, UINT cat = 0);
	uint32	Process(uint32 reducedownload, UINT icounter);
	uint32	CalculateDownloadRate(); // NEO: ASM - [AccurateSpeedMeasure] <-- Xanatos --
	uint8	LoadPartFile(LPCTSTR in_directory, LPCTSTR filename, bool getsizeonly = false); //filename = *.part.met
	uint8	ImportShareazaTempfile(LPCTSTR in_directory,LPCTSTR in_filename , bool getsizeonly);
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	bool	LoadFromTempFile(CFileDataIO* file);
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
	bool	LoadTagsFromTempFile(CFileDataIO* fileptr, bool getsizeonly = false, bool isnewstyle = false); // NEO: VOODOO - [UniversalPartfileInterface] <-- Xanatos --

	bool	SavePartFile();
	void	WriteToTempFile(CFileDataIO* file); // NEO: VOODOO - [UniversalPartfileInterface] <-- Xanatos --
	void	PartFileHashFinished(CKnownFile* result);
	bool	HashSinglePart(UINT partnumber); // true = ok , false = corrupted
	// NEO: SSH - [SlugFillerSafeHash] -- Xanatos -->
	bool	SetSinglePartHash(uint16 part, bool ICHused = false, bool AICHRecover = false, bool AICHok = false);
	void	PartHashFinished(UINT partnumber, bool corrupt, bool AICHRecover = false, bool AICHok = false);
	uint16	GetPartsHashing(bool bNoBlocks = true);
	bool	IsPartShareable(UINT partnumber) const;
	//bool	IsRangeShareable(uint64 start, uint64 end) const;
	// NEO: SSH END <-- Xanatos --
	// NEO: SCV - [SubChunkVerification] -- Xanatos -->
	uint8	GetBlocksInPart(uint16 part);
	bool	SetBlockPartHash(uint16 part, bool AICHRecover = false, bool ForceMD4 = false);
	void	BlockHashFinished(UINT partnumber, CMap<uint8,uint8,BOOL,BOOL>* resultMap, bool AICHRecover = false, bool ForceMD4 = false);
	bool	GetBlockMap(uint16 part, tBlockMap** map);
	CMutex*	GetSCV_mut()	{return &SCV_mut; }
	// NEO: SCV END <-- Xanatos --
	// NEO: PIX - [PartImportExport] -- Xanatos -->
	bool	ImportParts(); 
	BOOL	PerformImportParts(ImportInfo* Instructions); 
	// NEO: PIX END <-- Xanatos --
	// NEO: MOD - [SpaceAllocate] -- Xanatos -->
	void	AllocateNeededSpace();
	bool	IncompleteAllocateSpace()	{ return ((m_hpartfile.m_hFile != INVALID_HANDLE_VALUE) && m_hpartfile.GetLength() < GetFileSize()); } 
	// NEO: MOD END <-- Xanatos --

	// NEO: FCFG - [FileConfiguration] -- Xanatos -->
	bool	LoadNeoFile();
	bool	LoadNeoFile(CFileDataIO* file);
	bool	SaveNeoFile();
	bool	SaveNeoFile(CFileDataIO* file);
	virtual bool HasPreferences();
	// NEO: FCF END <-- Xanatos --

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
	bool	LoadSources();
	bool	LoadSources(CFileDataIO* file);
	bool	SaveSources();
	bool	SaveSources(CFileDataIO* file);
#endif // NEO_SS // NEO: NSS END <-- Xanatos --

	void	AddGap(uint64 start, uint64 end);
	void	FillGap(uint64 start, uint64 end);
	void	DrawStatusBar(CDC* dc, LPCRECT rect, bool bFlat) /*const*/;
	virtual void	DrawShareStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool	 bFlat) const;
	virtual void	DrawClassicShareStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool	 bFlat) const; // NEO: MOD - [ClassicShareStatusBar] <-- Xanatos --
	bool	IsComplete(uint64 start, uint64 end, bool bIgnoreBufferedData) const;
	bool	IsPureGap(uint64 start, uint64 end) const;
	bool	IsAlreadyRequested(uint64 start, uint64 end, bool bCheckBuffers = false) const;
    bool    ShrinkToAvoidAlreadyRequested(uint64& start, uint64& end) const;
	bool	IsCorruptedPart(UINT partnumber) const;
	uint64	GetTotalGapSizeInRange(uint64 uRangeStart, uint64 uRangeEnd) const;
	uint64	GetTotalGapSizeInPart(UINT uPart) const;
	void	UpdateCompletedInfos();
	void	UpdateCompletedInfos(uint64 uTotalGaps);
	virtual void	UpdatePartsInfo();
	virtual void	UpdatePartsInfoEx(EPartStatus type); // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --

	//bool	GetNextRequestedBlock(CUpDownClient* sender, Requested_Block_Struct** newblocks, uint16* count) /*const*/;
	// NEO: DBR - [DynamicBlockRequest] -- Xanatos -->
	bool	GetNextRequestedBlock(CUpDownClient* sender, Requested_Block_Struct** newblocks, uint16* count, uint32 blocksize) /*const*/; 
	bool	GetNextRequestedBlockICS(CUpDownClient* sender, Requested_Block_Struct** newblocks, uint16* count, uint32 blocksize) /*const*/; // NEO: ICS - [InteligentChunkSelection]
	// NEO: DBR END <-- Xanatos --
	void	WriteSubChunkMaps(CSafeMemFile* file, CClientFileStatus* status = NULL) const; // NEO: SCT - [SubChunkTransfer] <-- Xanatos --
	void	WriteIncPartStatus(CSafeMemFile* file) const; // NEO: ICS - [InteligentChunkSelection] <-- Xanatos --
	//void	WritePartStatus(CSafeMemFile* file) const;
	void	WritePartStatus(CSafeMemFile* file, CUpDownClient* client) const; // NEO: IPS - [InteligentPartSharing] <-- Xanatos --
	void	WriteCompleteSourcesCount(CSafeMemFile* file) const;
	void	AddSources(CSafeMemFile* sources,uint32 serverip, uint16 serverport, bool bWithObfuscationAndHash);
	void	AddSource(LPCTSTR pszURL, uint32 nIP);
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
	static bool CanAddSource(uint32 userid, uint16 port, uint32 serverip, uint16 serverport, UINT* pdebug_lowiddropped = NULL, bool Ed2kID = true, bool Low2Low = false);
#else
	static bool CanAddSource(uint32 userid, uint16 port, uint32 serverip, uint16 serverport, UINT* pdebug_lowiddropped = NULL, bool Ed2kID = true);
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
	static bool CheckSourceID(uint32 dwID); // NEO: CI#6 - [CodeImprovement] <-- Xanatos --
	
	EPartFileStatus	GetStatus(bool ignorepause = false) const;
	void	SetStatus(EPartFileStatus eStatus);		// set status and update GUI
	void	_SetStatus(EPartFileStatus eStatus);	// set status and do *not* update GUI
	void	NotifyStatusChange();
	bool	IsStopped() const { return stopped; }
	bool	IsPaused() const { return paused; } // NEO: MOD - [IsPaused] <-- Xanatos --
	bool	GetCompletionError() const { return m_bCompletionError; }
	bool	GetCompletionBreak() const { return m_bCompletionBreak; } // NEO: POFC - [PauseOnFileComplete] <-- Xanatos --
	EMFileSize  GetCompletedSize() const { return completedsize; }
	CString getPartfileStatus() const;
	int		getPartfileStatusRang() const;
	void	SetActive(bool bActive);

	uint8	GetDownPriority() const { return m_iDownPriority; }
	void	SetDownPriority(uint8 iNewDownPriority, bool resort = true);
	bool	IsAutoDownPriority(void) const { return m_bAutoDownPriority; }
	void	SetAutoDownPriority(bool NewAutoDownPriority) { m_bAutoDownPriority = NewAutoDownPriority; }
	void	UpdateAutoDownPriority();

	//UINT	GetSourceCount() const { return srclist.GetCount(); }
	UINT	GetSourceCount() const { return srclist.GetCount() - GetInactiveSourceCount(); } // NEO: FIX - [SourceCount] <-- Xanatos --
	UINT	GetSrcA4AFCount() const { return A4AFsrclist.GetCount(); }
	UINT	GetSrcStatisticsValue(EDownloadState nDLState) const;
	// NEO: FIX - [SourceCount] -- Xanatos -->
	void	IncrSrcStatisticsValue(EDownloadState nDLState); 
	void	DecrSrcStatisticsValue(EDownloadState nDLState); 
	UINT	GetInactiveSourceCount() const;
	UINT	GetAvailableSrcCount() const;
	// NEO: FIX END <-- Xanatos --
	UINT	GetTransferringSrcCount() const;
	uint64	GetTransferred() const { return m_uTransferred; }
	uint64	GetTransferredSession() const { return m_uTransferredSession; } // MOD - [SessionDL] <-- Xanatos --
	uint32	GetDatarate() const { return datarate; }
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	uint32	GetLanDatarate() const { return landatarate; }
#endif //LANCAST // NEO: NLC END <-- Xanatos --
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	uint32	GetVoodooDatarate() const { return voodoodatarate; }
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
	float	GetPercentCompleted() const { return percentcompleted; }
	float	GetPercentCompletedInitial() const { return percentcompletedinitial; } // NEO: MOD - [Percentage] <-- Xanatos --
	UINT	GetNotCurrentSourcesCount() const;
	int		GetValidSourcesCount() const;
	bool	IsArchive(bool onlyPreviewable = false) const; // Barry - Also want to preview archives
    bool    IsPreviewableFileType() const;
	time_t	getTimeRemaining() const;
	time_t	getTimeRemainingSimple() const;
	uint32	GetDlActiveTime() const;

	// Barry - Added as replacement for BlockReceived to buffer data before writing to disk
	//uint32	WriteToBuffer(uint64 transize, const BYTE *data, uint64 start, uint64 end, Requested_Block_Struct *block, const CUpDownClient* client);
	uint32	WriteToBuffer(uint64 transize, const BYTE *data, uint64 start, uint64 end, Requested_Block_Struct *block, uint32 clientIP); // NEO: VOODOO - [UniversalPartfileInterface] <-- Xanatos --
	void	FlushBuffer(bool forcewait=false, bool bForceICH = false, bool bNoAICH = false);
	//MORPH START - Added by SiRoB, Flush Thread // NEO: FFT - [FileFlushThread] -- Xanatos -->
	void	WriteToDisk();
	void	FlushDone();
	//MORPH END   - Added by SiRoB, Flush Thread // NEO: FFT END <-- Xanatos --

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

	void	OpenFile() const;
	void	PreviewFile();
	void	MovePartFile(CString newTempDir); // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
	//void	DeleteFile(); 
	void	DeleteFile(bool bUnloadOnly = false, bool resort = true); // NEO: MTD - [MultiTempDirectories] // NEO: VOODOO - [UniversalPartfileInterface] <-- Xanatos --
	void	CleanUpFiles(); // NEO: MOD - [CleanUpFiles] <-- Xanatos --
	void	StopFile(bool bCancel = false, bool resort = true);
	void	PauseFile(bool bInsufficient = false, bool resort = true);
	void	StopPausedFile();
	void	ResumeFile(bool resort = true);
	void	ResumeFileInsufficient();

	virtual Packet* CreateSrcInfoPacket(const CUpDownClient* forClient, uint8 byRequestedVersion, uint16 nRequestedOptions) const;
	void	AddClientSources(CSafeMemFile* sources, uint8 sourceexchangeversion, bool bSourceExchange2, const CUpDownClient* pClient = NULL);

	UINT	GetAvailablePartCount() const { return availablePartsCount; }
	void	UpdateAvailablePartsCount();

	uint32	GetLastAnsweredTime() const	{ return m_ClientSrcAnswered; }
	//void	SetLastAnsweredTime() { m_ClientSrcAnswered = ::GetTickCount(); }
	void	SetLastAnsweredTime() { 
				m_ClientSrcAnswered = ::GetTickCount(); 
				// NEO: MSR - [ManualSourceRequest] -- Xanatos -->
				if(m_iCollectingXSSources > 0)
					m_iCollectingXSSources--;
				// NEO: MSR END <-- Xanatos --
			}
	void	SetLastAnsweredTimeTimeout();

	uint64	GetCorruptionLoss() const { return m_uCorruptionLoss; }
	uint64	GetCompressionGain() const { return m_uCompressionGain; }
	uint32	GetRecoveredPartsByICH() const { return m_uPartsSavedDueICH; }

	virtual void	UpdateFileRatingCommentAvail(bool bForceUpdate = false);
	virtual void	RefilterFileComments();

	void	AddDownloadingSource(CUpDownClient* client);
	void	RemoveDownloadingSource(CUpDownClient* client);

	CString GetProgressString(uint16 size) const;
	CString GetInfoSummary() const;

//	int		GetCommonFilePenalty() const;
	void	UpdateDisplayedInfo(bool force = false);

	// NEO: NSC - [NeoSharedCategories] -- Xanatos --
	//UINT	GetCategory() /*const*/;
	//void	SetCategory(UINT cat);
	virtual void SetCategory(UINT cat, uint8 init = 0); // NEO: MOD - [SetCategory] <-- Xanatos --
	void	UpdatePartPrefs(CPartPreferences* cfgPartPrefs); // NEO: FCFG - [FileConfiguration] <-- Xanatos --
	bool	CheckShowItemInGivenCat(int inCategory) /*const*/;

	uint8*	MMCreatePartStatus();

	//preview
	virtual bool GrabImage(uint8 nFramesToGrab, double dStartTime, bool bReduceColor, uint16 nMaxWidth,void* pSender);
	virtual void GrabbingFinished(CxImage** imgResults, uint8 nFramesGrabbed, void* pSender);

	void	FlushBuffersExceptionHandler(CFileException* error);
	void	FlushBuffersExceptionHandler();

	void	PerformFileCompleteEnd(DWORD dwResult);

	void	SetFileOp(EPartFileOp eFileOp);
	EPartFileOp GetFileOp() const							{ return m_eFileOp; }
	void	SetFileOpProgress(UINT uProgress);
	UINT	GetFileOpProgress() const						{ return m_uFileOpProgress; }

	//void	RequestAICHRecovery(UINT nPart);
	void	RequestAICHRecovery(UINT nPart, bool* pFaild = NULL); // NEO: SCV - [SubChunkVerification] <-- Xanatos --
	void	AICHRecoveryDataAvailable(UINT nPart);

	bool	Publishable(); // NEO: SAFS - [ShowAllFilesInShare] <-- Xanatos --

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	CTypedPtrList<CPtrList, Gap_Struct*>& GetGapList() {return gaplist;}
	void	ReCombinateGapList();
	void	SetVoodoo() {m_isVoodooFile = true;}
	bool	IsVoodooFile() const;
	void	AddMaster(CVoodooSocket* Master);
	void	RemoveMaster(CVoodooSocket* Master);
	void	VoodooComplete(bool bFinal = false);
	void	AddThrottleBlock(uint64 start, uint64 end);
	void	RemoveThrottleBlock(uint64 start, uint64 end);

	// NEO: VOODOOx - [VoodooSourceExchange]
	bool	IsVoodooXS(bool bBoot = false) const; 
	// NEO: VOODOOx END
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

	uint32	m_LastSearchTime;
	uint32	m_LastSearchTimeUdp; // NEO: SRT - [SourceRequestTweaks] <-- Xanatos --
	uint32	m_LastSearchTimeKad;
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	uint32	m_LastLanSearchTime;
#endif //LANCAST // NEO: NLC END <-- Xanatos --
	uint64	m_iAllocinfo;
	CUpDownClientPtrList srclist;
	CUpDownClientPtrList A4AFsrclist; //<<-- enkeyDEV(Ottavio84) -A4AF-
	CTime	lastseencomplete;
	CFile	m_hpartfile;				// permanent opened handle to avoid write conflicts
	CMutex 	m_FileCompleteMutex;		// Lord KiRon - Mutex for file completion
	uint16	src_stats[7];				// NEO: MOD - [srcStats] <-- Xanatos --
	uint16  net_stats[3];
	volatile bool m_bPreviewing;
	volatile bool m_bRecoveringArchive; // Is archive recovery in progress
	bool	m_bLocalSrcReqQueued;
	bool	srcarevisible;				// used for downloadlistctrl
	bool	hashsetneeded;
	uint8	m_TotalSearchesKad;
    bool    AllowSwapForSourceExchange()					{ return ::GetTickCount()-lastSwapForSourceExchangeTick > 30*1000; } // ZZ:DownloadManager
    void    SetSwapForSourceExchangeTick()					{ lastSwapForSourceExchangeTick = ::GetTickCount(); } // ZZ:DownloadManager
	
	// NEO: SRT - [SourceRequestTweaks] -- Xanatos --
	//UINT	SetPrivateMaxSources(uint32 in)					{ return m_uMaxSources = in; } 
	//UINT	GetPrivateMaxSources() const					{ return m_uMaxSources; } 
	//UINT	GetMaxSources() const;
	//UINT	GetMaxSourcePerFileSoft() const;
	//UINT	GetMaxSourcePerFileUDP() const;

    bool    GetPreviewPrio() const							{ return m_bpreviewprio; }
	void    SetPreviewPrio(bool in)							{ m_bpreviewprio=in; }

	// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
	static bool NextRightFileHasHigherPrio(const CPartFile* left, const CPartFile* right);
    static bool RightFileHasHigherPrio(const CPartFile* left, const CPartFile* right);
	// NEO: NXC END <-- Xanatos --
	//static bool RightFileHasHigherPrio(CPartFile* left, CPartFile* right);

	virtual uint16	GetPartAvailibility(uint16 i) const		{return (!IsPartFile() ? CKnownFile::GetPartAvailibility(i) : (m_SrcpartFrequency.IsEmpty()			? 0 : m_SrcpartFrequency[i]));} // NEO: NPT - [NeoPartTraffic] <-- Xanatos --

	// NEO: SDT - [SourcesDropTweaks] -- Xanatos -->
	bool	TestAndDropSource(CUpDownClient* src, int DropMode, UINT Time, int LimitMode, UINT Limit, int Mode, UINT &LastTime);
	bool	IsHighQState(CUpDownClient* src);
	UINT	GetAverageQueueRank() {return m_uAverageQueueRank;}
	// NEO: SDT END <-- Xanatos --

	UINT	IsCollectingHalted() const; // NEO: XSC - [ExtremeSourceCache] <-- Xanatos --

	// NEO: ASL - [AutoSoftLock] -- Xanatos -->
	bool	CheckSoftLock();
	bool	GetAutoSoftLocked()	const		{return m_bSoftLocked; } 
	// NEO: ASL END <-- Xanatos --

	void	CalculateAutoHardLimit(); // NEO: AHL - [AutoHardLimit] <-- Xanatos --
	
	void	CalculateCategoryLimit(); // NEO: CSL - [CategorySourceLimit] <-- Xanatos --

	// NEO: GSL - [GlobalSourceLimit] -- Xanatos -->
	void	InitHL();
	void	IncrHL(UINT m_uSourcesDif);
	void	DecrHL()										{SetPassiveHL(0);}
	void	SetPassiveHL(UINT m_uSourcesDif);
	UINT	GetFileHardLimit() const						{return m_uFileHardLimit;}
	// NEO: GSL END <-- Xanatos --

	// NEO: SRT - [SourceRequestTweaks] -- Xanatos -->
	void	ManIncrHL();
	void	ManDecrHL();
	// NEO: SRT END <-- Xanatos --

	CDeadSourceList	m_DeadSourceList;

	// NEO: MCM - [ManualClientManagement] -- Xanatos -->
	uint16	CollectAllA4AF();
	uint16	ReleaseAllA4AF();
	// NEO: MCM END <-- Xanatos --

	// NEO: FCFG - [FileConfiguration] -- Xanatos -->
	CPartPreferences* PartPrefs;

	// NEO: SRT - [SourceRequestTweaks]
	UINT	GetMaxSources(bool bReal = false) const;
	UINT	GetSwapSourceLimit() const;
	UINT	GetXsSourceLimit() const;
	UINT	GetSvrSourceLimit() const;
	UINT	GetKadSourceLimit() const;
	UINT	GetUdpSourceLimit() const;
	// NEO: SRT END

	UINT	GetSourceCacheSourceLimit() const; // NEO: XSC - [ExtremeSourceCache]
	
	// NEO: SDT - [SourcesDropTweaks]
	UINT	GetBadSourceDropLimit() const;
	UINT	GetNNPSourceDropLimit() const;
	UINT	GetFullQSourceDropLimit() const;
	UINT	GetHighQSourceDropLimit() const;
	// NEO: SDT END
	// NEO: FCFG END <-- Xanatos --

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
	UINT	GetSourceStorageSourceLimit() const;
	UINT	GetSourceStorageReaskSourceLimit() const;
#endif // NEO_SS // NEO: NSS END <-- Xanatos --

#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer] -- Xanatos -->
	bool	SortCompare(POSITION pos1, POSITION pos2);
	void	SortSwap(POSITION pos1, POSITION pos2);
	void	HeapSortList(UINT first, UINT last);
	void	SortSourceList();
	void	AnalizerSources();
#endif // NEO_SA // NEO: NSA END <-- Xanatos --

	// NEO: MSR - [ManualSourceRequest] -- Xanatos -->
	void	CollectKADSources();

	void	SetXSSourcesCollecting(int Amount = 3)	{ m_iCollectingXSSources = Amount;}
	bool	IsCollectXSSources()					{ return m_iCollectingXSSources ? TRUE : FALSE; }
	// NEO: MSR END <-- Xanatos --
	
	void	DropSources(EDownloadState nState); // NEO: MSD - [ManualSourcesDrop] <-- Xanatos --

	// NEO: OCF - [OnlyCompleetFiles] -- Xanatos -->
	bool	NotSeenCompleteSource() const; 
	void	SetForced(bool bSet)					{forced = bSet;}
	bool	GetForced() const						{return forced;}
	// NEO: OCF END <-- Xanatos --
	// NEO: SD - [StandByDL] -- Xanatos -->
	void	SetStandBy(bool bSet)					{standby = bSet;}
	bool	GetStandBy() const						{return standby;}
	UINT	IsStandBy() const; 
	// NEO: SD END <-- Xanatos --
	// NEO: SC - [SuspendCollecting] -- Xanatos -->
	void	SetSuspend(bool bSet)					{suspend = bSet;}
	bool	GetSuspend() const						{return suspend;}
	// NEO: SC END <-- Xanatos --

	// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
	void	SetCatResumeOrder(int order)	{ m_catResumeOrder = order; SavePartFile(); }
	int		GetCatResumeOrder() const		{ return m_catResumeOrder; }
	// NEO: NXC END <-- Xanatos --

#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	virtual void GetTooltipFileInfo(CString &info);
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

#ifdef _DEBUG
	// Diagnostic Support
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	//bool	GetNextEmptyBlockInPart(UINT partnumber, Requested_Block_Struct* result) const;
	bool	GetNextEmptyBlockInPart(UINT partnumber, tBlockMap* blockmap, Requested_Block_Struct* result, uint32 blocksize) const; // NEO: SCT - [SubChunkTransfer] // NEO: DBR - [DynamicBlockRequest] <-- Xanatos --
	// NEO: SCV - [SubChunkVerification] -- Xanatos -->
	bool	VerifyIncompleteParts(bool bCheck = false);
	void	VerifyIncompletePart(uint16 partNumber);
	void	SaveAICHMap(CFileDataIO* file);
	bool	LoadAICHMap(CFileDataIO* file);
	// NEO: SCV END <-- Xanatos --
	void	CompleteFile(bool hashingdone);
	void	CreatePartFile(UINT cat = 0);
	void	Init();

	// NEO: ICS - [InteligentChunkSelection] -- Xanatos -->
	uint16* CalcDownloadingParts(CUpDownClient* client);
	uint64	GetPartSizeToDownload(uint16 partNumber);
	// NEO: ICS END <-- Xanatos --

private:
	BOOL 		PerformFileComplete(); // Lord KiRon
	static UINT CompleteThreadProc(LPVOID pvParams); // Lord KiRon - Used as separate thread to complete file
	// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
	BOOL		PerformFileMove(); 
	static UINT MoveThreadProc(LPVOID pvParams);
	// NEO: MTD END <-- Xanatos --
	static UINT AFX_CDECL AllocateSpaceThread(LPVOID lpParam);
	void		CharFillRange(CString* buffer,uint32 start, uint32 end, char color) const;
	void		ParseICHResult();	// SLUGFILLER: SafeHash // NEO: SSH - [SlugFillerSafeHash] <-- Xanatos --

	CCorruptionBlackBox	m_CorruptionBlackBox;
	static CBarShader s_LoadBar;
	static CBarShader s_ChunkBar;
	uint32	m_iLastPausePurge;
	uint16	count;
	UINT	m_anStates[STATES_COUNT];
	EMFileSize	completedsize;
	uint64	m_uCorruptionLoss;
	uint64	m_uCompressionGain;
	uint32	m_uPartsSavedDueICH;
	uint32	datarate;
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	uint32	landatarate;
#endif //LANCAST // NEO: NLC END <-- Xanatos --
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	uint32	voodoodatarate;
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
	CString m_fullname;
	CString m_partmetfilename;
	CString	m_strNewDirectory; // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
	uint64	m_uTransferred;
	uint64	m_uTransferredSession; // MOD - [SessionDL] <-- Xanatos --
	//UINT	m_uMaxSources; // NEO: SRT - [SourceRequestTweaks] -- Xanatos --
	bool	paused;
	bool	stopped;
	bool	forced; // NEO: OCF - [OnlyCompleetFiles] <-- Xanatos --
	bool	standby; // NEO: SD - [StandByDL] <-- Xanatos --
	bool	suspend; // NEO: SC - [SuspendCollecting] <-- Xanatos --
	bool	insufficient;
	bool	m_bCompletionError;
	bool	m_bCompletionBreak; // NEO: POFC - [PauseOnFileComplete] <-- Xanatos --
	uint8	m_iDownPriority;
	bool	m_bAutoDownPriority;
	EPartFileStatus	status;
	bool	newdate;	// indicates if there was a writeaccess to the .part file
	uint32	lastpurgetime;
	uint32	m_LastNoNeededCheck;
	CTypedPtrList<CPtrList, Gap_Struct*> gaplist;
	CTypedPtrList<CPtrList, Requested_Block_Struct*> requestedblocks_list;
	CArray<uint16,uint16> m_SrcpartFrequency;
	CArray<uint16,uint16> m_SrcincpartFrequency; // NEO: ICS - [InteligentChunkSelection] <-- Xanatos --
	float	percentcompleted;
	float   percentcompletedinitial; // NEO: MOD - [Percentage] <-- Xanatos --
	// NEO: SSH - [SlugFillerSafeHash] -- Xanatos -->
	CWinThread* m_HashThread;
	CMap<uint16,uint16,PartHashOrder*,PartHashOrder*> m_PartsToHash;
	CCriticalSection m_PartsToHashLocker;
	CMutex	ICH_mut; // ICH locks the file
	CList<uint16,uint16> m_ICHPartsComplete;
	CArray<bool,bool> m_PartsShareable;
	// NEO: SSH END <-- Xanatos --
	// NEO: SCV - [SubChunkVerification] -- Xanatos -->
	CWinThread* m_AICHHashThread;
	CMap<uint16,uint16,BlockHashOrder*,BlockHashOrder*> m_BlocksToHash;
	CCriticalSection m_BlocksToHashLocker;
	CBlockMaps m_BlockMaps;
	uint32	m_uAICHVeirifcationPending;
	CMutex	SCV_mut; // For updating the block map and for hash AICH set operations
	// NEO: SCV END <-- Xanatos --
	CList<uint16, uint16> corrupted_list;
	uint32	m_ClientSrcAnswered;
	UINT	availablePartsCount;
	CWinThread* m_AllocateThread;
	CWinThread* m_FlushThread; //MORPH - Flush Thread // NEO: FFT - [FileFlushThread] <-- Xanatos --
	DWORD	m_lastRefreshedDLDisplay;
	CUpDownClientPtrList m_downloadingSourceList;
	bool	m_bDeleteAfterAlloc;
    bool	m_bpreviewprio;
	// Barry - Buffered data to be written
	CTypedPtrList<CPtrList, PartFileBufferedData*> m_BufferedData_list;
	uint64	m_nTotalBufferData;
	uint32	m_nLastBufferFlushTime;
	//UINT	m_category; // NEO: NSC - [NeoSharedCategories] <-- Xanatos --
	DWORD	m_dwFileAttributes;
	time_t	m_tActivated;
	uint32	m_nDlActiveTime;
	uint32	m_tLastModified;	// last file modification time (NT's version of UTC), to be used for stats only!
	uint32	m_tCreated;			// file creation time (NT's version of UTC), to be used for stats only!
    uint32	m_random_update_wait;	
	volatile EPartFileOp m_eFileOp;
	volatile UINT m_uFileOpProgress;

    DWORD   lastSwapForSourceExchangeTick; // ZZ:DownloadManaager

	//MORPH START - Added by SiRoB, Flush Thread // NEO: FFT - [FileFlushThread] -- Xanatos -->
	FlushDone_Struct* m_FlushSetting;
	CCriticalSection m_BufferedData_list_Locker;
	//MORPH END   - Added by SiRoB, Flush Thread // NEO: FFT END <-- Xanatos --

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	bool	m_isVoodooFile;
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

	// NEO: SDT - [SourcesDropTweaks] -- Xanatos -->
	uint32	lastBadSourcePurgeTime; 
	uint32	lastNNPSourcePurgeTime; 
	uint32	lastFullQSourcePurgeTime; 
	uint32	lastHighQSourcePurgeTime; 

	UINT	m_uAverageQueueRank;
	// NEO: SDT END <-- Xanatos --

	// NEO: ASL - [AutoSoftLock] -- Xanatos -->
	bool	m_bCollectingHalted;
	bool	m_bSoftLocked;
	// NEO: ASL END <-- Xanatos --

	// NEO: AHL - [AutoHardLimit] -- Xanatos -->
	UINT	m_uAutoHardLimit;
	uint32	m_uLastAutoHardLimit;
	// NEO: AHL END <-- Xanatos --

	// NEO: CSL - [CategorySourceLimit] -- Xanatos -->
	uint32	m_uLastCategoryLimit;
	UINT	m_uFileCategoryLimit;
	// NEO: CSL END <-- Xanatos --

	UINT	m_uFileHardLimit; // NEO: GSL - [GlobalSourceLimit] <-- Xanatos --

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
	uint32	m_uLastSaveSource;
	uint32	m_uLastLoadSource;

	uint32	m_uLastReaskedGroupeTime;
#endif // NEO_SS // NEO: NSS END <-- Xanatos --

	int     m_ics_filemode; // NEO: ICS - [InteligentChunkSelection] <-- Xanatos --

	int		m_iCollectingXSSources;	// NEO: MSR - [ManualSourceRequest] <-- Xanatos --

	int		m_catResumeOrder; // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
};

// NEO: MOD - [CSyncHelper] -- Xanatos --
class CSyncHelper
{
public:
	CSyncHelper()
	{
		m_pObject = NULL;
	}
	~CSyncHelper()
	{
		if (m_pObject)
			m_pObject->Unlock();
	}
	CSyncObject* m_pObject;
};

//MORPH START - Added by SiRoB, Flush Thread // NEO: FFT - [FileFlushThread] -- Xanatos -->
class CPartFileFlushThread : public CWinThread
{
	DECLARE_DYNCREATE(CPartFileFlushThread)
protected:
	CPartFileFlushThread()	{}
public:
	virtual	BOOL	InitInstance() {return true;}
	virtual int		Run();
	void	SetPartFile(CPartFile* pOwner);
private:
	CPartFile*				m_partfile;
};
//MORPH END   - Added by SiRoB, Flush Thread // NEO: FFT END <-- Xanatos --

// BEGIN SLUGFILLER: SafeHash // NEO: SSH - [SlugFillerSafeHash] -- Xanatos -->
class CPartHashThread : public CWinThread
{
	DECLARE_DYNCREATE(CPartHashThread)
protected:
	CPartHashThread()	{}

	friend class CPartFile;
	CFile file;
public:
	virtual	BOOL	InitInstance() {return true;}
	virtual int		Run();
	void			SetPartFile(CPartFile* pOwner);
private:
	CPartFile*		m_pOwner;
	CString			fullname;
};
// END SLUGFILLER: SafeHash // NEO: SSH END <-- Xanatos --

// NEO: SCV - [SubChunkVerification] -- Xanatos -->
class CBlockHashThread : public CWinThread
{
	DECLARE_DYNCREATE(CBlockHashThread)
protected:
	CBlockHashThread()	{}

	friend class CPartFile;
	CFile file;
public:
	virtual	BOOL	InitInstance() {return true;}
	virtual int		Run();
	void			SetPartFile(CPartFile* pOwner);
private:
	CPartFile*		m_pOwner;
	CString			fullname;
};
// NEO: SCV END <-- Xanatos --
