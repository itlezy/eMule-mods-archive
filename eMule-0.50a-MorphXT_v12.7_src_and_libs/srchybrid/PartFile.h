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

// khaos::kmod+ Save/Load Sources
#include "SourceSaver.h" //<<-- enkeyDEV(Ottavio84) -New SLS-
// khaos::kmod-

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
	PFOP_UNCOMPRESSING,
	PFOP_SR13_IMPORTPARTS //MORPH - Added by SiRoB, Import Part
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
struct FlushDone_Struct
{
	bool	bIncreasedFile;
	bool	bForceICH;
	bool*	changedPart;
};

//MORPH START - Added by Stulle, Source cache [Xman]
enum ESourceFrom;
struct PartfileSourceCache
{
	uint16			nPort;
	uint32			dwID;
	uint32			dwServerIP;
	uint16			nServerPort;
	uchar			achUserHash[16];
	uint8			byCryptOptions;
	bool			withuserhash;
	bool			ed2kIDFlag;
	ESourceFrom		sourcefrom;
	uint32			expires;
};
#define	SOURCECACHEPROCESSLOOP	MIN2MS(1)	//every one minute
#define SOURCECACHELIFETIME		MIN2MS(30)	//expires after 30 minutes
//MORPH END   - Added by Stulle, Source cache [Xman]

typedef CTypedPtrList<CPtrList, CUpDownClient*> CUpDownClientPtrList;

class CPartFile : public CKnownFile
{
	DECLARE_DYNAMIC(CPartFile)

	friend class CPartFileConvert;
	friend class CPartHashThread;	// SLUGFILLER: SafeHash
	friend class CPartFileFlushThread;	//MORPH - Flush Thread
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
	//MORPH START - Changed by Stulle, No zz ratio for http traffic
	/*
	uint32	Process(uint32 reducedownload, UINT icounter, uint32 friendReduceddownload);
	*/
	uint32	Process(uint32 reducedownload, UINT icounter, uint32 friendReduceddownload,uint32 httpReduceddownload);
	//MORPH END   - Changed by Stulle, No zz ratio for http traffic
	EPartFileLoadResult	LoadPartFile(LPCTSTR in_directory, LPCTSTR filename, EPartFileFormat* pOutCheckFileFormat = NULL); //filename = *.part.met
	EPartFileLoadResult	ImportShareazaTempfile(LPCTSTR in_directory,LPCTSTR in_filename, EPartFileFormat* pOutCheckFileFormat = NULL);

	bool	SavePartFile(bool bDontOverrideBak = false);
	void	PartFileHashFinished(CKnownFile* result);
	bool	HashSinglePart(UINT partnumber, bool* pbAICHReportedOK = NULL); // true = ok , false = corrupted //MORPH - Flush Thread
	// SLUGFILLER: SafeHash - replaced old handlers, full hash checker remains for file completion
	void	PartHashFinished(UINT partnumber, bool bAICHAgreed, bool corrupt);
	void	PartHashFinishedAICHRecover(UINT partnumber, bool corrupt);
	bool	IsPartShareable(UINT partnumber) const;
	bool	IsRangeShareable(uint64 start, uint64 end) const;
	//MORPH END   - Added by SiRoB, SLUGFILLER: SafeHash


	void	AddGap(uint64 start, uint64 end);
	void	FillGap(uint64 start, uint64 end);
	void	DrawStatusBar(CDC* dc, LPCRECT rect, bool bFlat) /*const*/;
	virtual void	DrawShareStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool	 bFlat) /*const*/;
	bool	IsComplete(uint64 start, uint64 end, bool bIgnoreBufferedData) const;
	bool	IsPureGap(uint64 start, uint64 end) const;
	bool	IsAlreadyRequested(uint64 start, uint64 end, bool bCheckBuffers = false) const;
    bool    ShrinkToAvoidAlreadyRequested(uint64& start, uint64& end) const;
	bool	IsCorruptedPart(UINT partnumber) const;
	//MORPH START - Enhanced DBR
	uint64	GetRemainingAvailableData(const uint8* srcstatus) const;
	uint64	GetRemainingAvailableData(const CUpDownClient* sender) const;
	//MORPH END   - Enhanced DBR
	uint64	GetTotalGapSizeInRange(uint64 uRangeStart, uint64 uRangeEnd) const;
	uint64	GetTotalGapSizeInPart(UINT uPart) const;
	void	UpdateCompletedInfos();
	//void	UpdateCompletedInfos(uint64 uTotalGaps); //MORPH - Optimization, completedsize
	virtual void	UpdatePartsInfo();

	bool	GetNextRequestedBlock(CUpDownClient* sender, Requested_Block_Struct** newblocks, uint16* count) /*const*/;
	//MORPH START - Added by SiRoB, ICS Optional
	bool	GetNextRequestedBlockICS(CUpDownClient* sender, Requested_Block_Struct** newblocks, uint16* count);
	//MORPH END   - Added by SiRoB, ICS Optional
	void	WritePartStatus(CSafeMemFile* file, CUpDownClient* client = NULL) /*const*/; // SLUGFILLER: hideOS
	void	WriteCompleteSourcesCount(CSafeMemFile* file) const;
	void	AddSources(CSafeMemFile* sources,uint32 serverip, uint16 serverport, bool bWithObfuscationAndHash);
	void	AddSource(LPCTSTR pszURL, uint32 nIP);
	static bool CanAddSource(uint32 userid, uint16 port, uint32 serverip, uint16 serverport, UINT* pdebug_lowiddropped = NULL, bool Ed2kID = true);
	
	EPartFileStatus	GetStatus(bool ignorepause = false) const;
	void	SetStatus(EPartFileStatus eStatus);		// set status and update GUI
	void	_SetStatus(EPartFileStatus eStatus);	// set status and do *not* update GUI
	void	NotifyStatusChange();
	bool	IsStopped() const												{ return stopped; }
	bool	GetCompletionError() const										{ return m_bCompletionError; }
	EMFileSize  GetCompletedSize() const									{ return completedsize; }
	CString getPartfileStatus() const;
	int		getPartfileStatusRang() const;
	void	SetActive(bool bActive);

	uint8	GetDownPriority() const											{ return m_iDownPriority; }
	void	SetDownPriority(uint8 iNewDownPriority, bool resort = true);
	bool	IsAutoDownPriority(void) const									{ return m_bAutoDownPriority; }
	void	SetAutoDownPriority(bool NewAutoDownPriority)					{ m_bAutoDownPriority = NewAutoDownPriority; }
	void	UpdateAutoDownPriority();

	UINT	GetSourceCount() const											{ return srclist.GetCount(); }
	UINT	GetSrcA4AFCount() const											{ return A4AFsrclist.GetCount(); }
	UINT	GetSrcStatisticsValue(EDownloadState nDLState) const;
	UINT	GetTransferringSrcCount() const;
	uint64	GetTransferred() const											{ return m_uTransferred; }
	uint32	GetDatarate() const												{ return datarate; }
	float	GetPercentCompleted() const										{ return percentcompleted; }
	UINT	GetNotCurrentSourcesCount() const;
	int		GetValidSourcesCount() const;
	//MORPH START - Added by SiRoB, Source Counts Are Cached derivated from Khaos
	UINT	GetAvailableSrcCount() const;
	//MORPH END   - Added by SiRoB, Source Counts Are Cached derivated from Khaos
	bool	IsArchive(bool onlyPreviewable = false) const; // Barry - Also want to preview archives
    bool    IsPreviewableFileType() const;
	time_t	getTimeRemaining() const;
	time_t	getTimeRemainingSimple() const;
	time_t	GetDlActiveTime() const; //vs2005

	// Barry - Added as replacement for BlockReceived to buffer data before writing to disk
	uint32	WriteToBuffer(uint64 transize, const BYTE *data, uint64 start, uint64 end, Requested_Block_Struct *block, const CUpDownClient* client);
	void	FlushBuffer(bool forcewait=false, bool bForceICH = false, bool bNoAICH = false);
	//MORPH START - Added by SiRoB, Flush Thread
	void	WriteToDisk();
	void	FlushDone();
	//MORPH END   - Added by SiRoB, Flush Thread

	// Barry - This will invert the gap list, up to caller to delete gaps when done
	// 'Gaps' returned are really the filled areas, and guaranteed to be in order
	void	GetFilledList(CTypedPtrList<CPtrList, Gap_Struct*> *filled) const;

	// Barry - Added to prevent list containing deleted blocks on shutdown
	void	RemoveAllRequestedBlocks(void);
	bool	RemoveBlockFromList(uint64 start, uint64 end);
	bool	RemoveBlockFromList(const Requested_Block_Struct* block); //MORPH - Optimization
	bool	IsInRequestedBlockList(const Requested_Block_Struct* block) const;
	void	AddRequestedBlock(Requested_Block_Struct* block);
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

	CString GetProgressString(uint16 size) const;
	CString GetInfoSummary(bool bNoFormatCommands = false) const;

//	int		GetCommonFilePenalty() const;
	void	UpdateDisplayedInfo(bool force = false);

	UINT	GetCategory() const;
	void	SetCategory(UINT cat);
	bool	HasDefaultCategory() const;
	bool	CheckShowItemInGivenCat(int inCategory) /*const*/;

	uint8*	MMCreatePartStatus();

	//preview
	virtual bool GrabImage(uint8 nFramesToGrab, double dStartTime, bool bReduceColor, uint16 nMaxWidth,void* pSender);
	virtual void GrabbingFinished(CxImage** imgResults, uint8 nFramesGrabbed, void* pSender);

	void	FlushBuffersExceptionHandler(CFileException* error);
	void	FlushBuffersExceptionHandler();

	void	PerformFirstHash();		// SLUGFILLER: SafeHash
	void	PerformFileCompleteEnd(DWORD dwResult);

	void	SetFileOp(EPartFileOp eFileOp);
	EPartFileOp GetFileOp() const											{ return m_eFileOp; }
	void	SetFileOpProgress(UINT uProgress);
	UINT	GetFileOpProgress() const										{ return m_uFileOpProgress; }

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
    bool    AllowSwapForSourceExchange()					{ return ::GetTickCount()-lastSwapForSourceExchangeTick > 30*1000; } // ZZ:DownloadManager
    void    SetSwapForSourceExchangeTick()					{ lastSwapForSourceExchangeTick = ::GetTickCount(); } // ZZ:DownloadManager
	
	UINT	SetPrivateMaxSources(uint32 in)					{ return m_uMaxSources = in; } 
	UINT	GetPrivateMaxSources() const					{ return m_uMaxSources; } 
	UINT	GetMaxSources() const;
	UINT	GetMaxSourcePerFileSoft() const;
	UINT	GetMaxSourcePerFileUDP() const;

    bool    GetPreviewPrio() const							{ return m_bpreviewprio; }
	void    SetPreviewPrio(bool in)							{ m_bpreviewprio=in; }

    /*MORPH*/static bool RightFileHasHigherPrio(const CPartFile* left, const CPartFile* right);

	CDeadSourceList	m_DeadSourceList;
	//Morph Start - added by AndCycle, ICS
	// enkeyDev: ICS
	uint16* CalcDownloadingParts(const CUpDownClient* client); // Pawcio for enkeyDEV: ICS
	void	WriteIncPartStatus(CSafeMemFile* file);
    void    NewSrcIncPartsInfo();
	uint64	GetPartSizeToDownload(uint16 partNumber);
	// <--- enkeyDev: ICS
	//Morph End - added by AndCycle, ICS

	CArray<uint16,uint16> m_SrcpartFrequency; //MORPH - Added by SiRoB, Share Only The Need

	// khaos::categorymod+
	void	SetCatResumeOrder(UINT order)	{ m_catResumeOrder = order; SavePartFile(); }
	UINT	GetCatResumeOrder() const				{ return m_catResumeOrder; }
	// khaos::categorymod-
	// khaos::accuratetimerem+
	void	SetActivatedTick()				{ m_dwActivatedTick = GetTickCount(); }
	DWORD	GetActivatedTick()				{ return m_dwActivatedTick; }
	time_t	GetTimeRemainingAvg() const;
	// khaos::accuratetimerem-
	// khaos::kmod+ Advanced A4AF: Brute Force Features
	bool	ForceAllA4AF()	const			{ return m_bForceAllA4AF; }
	bool	ForceA4AFOff()	const			{ return m_bForceA4AFOff; }
	void	SetForceAllA4AF(bool in)		{ m_bForceAllA4AF = in; }
	void	SetForceA4AFOff(bool in)		{ m_bForceA4AFOff = in; }
	// khaos::kmod-
	bool	notSeenCompleteSource() const;
#ifdef _DEBUG
	// Diagnostic Support
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	bool	GetNextEmptyBlockInPart(UINT partnumber, Requested_Block_Struct* result, uint64 bytesToRequest = EMBLOCKSIZE) const;
	void	CompleteFile(bool hashingdone);
	void	CreatePartFile(UINT cat = 0);
	void	Init();
	// khaos::kmod+ Save/Load Sources
	CSourceSaver m_sourcesaver; //<<-- enkeyDEV(Ottavio84) -New SLS-
	// khaos::kmod-

private:
	BOOL 		PerformFileComplete(); // Lord KiRon
	static UINT CompleteThreadProc(LPVOID pvParams); // Lord KiRon - Used as separate thread to complete file
	static UINT AFX_CDECL AllocateSpaceThread(LPVOID lpParam);
	void		CharFillRange(CString* buffer,uint32 start, uint32 end, char color) const;
	void		ParseICHResult();	// SLUGFILLER: SafeHash

	CCorruptionBlackBox	m_CorruptionBlackBox;
	static CBarShader s_LoadBar;
	static CBarShader s_ChunkBar;
	time_t	m_iLastPausePurge;
	uint16	count;
	UINT	m_anStates[STATES_COUNT];
	//MORPH START - Added by SiRoB, Cached stat
	UINT	m_anStatesTemp[STATES_COUNT];
	//MORPH END   - Added by SiRoB, Cached stat
	uint64	m_uTotalGaps; //MORPH - Optimization, completedsize
	EMFileSize	completedsize;
	uint64	m_uCorruptionLoss;
	uint64	m_uCompressionGain;
	uint32	m_uPartsSavedDueICH;
	uint32	datarate;
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
public:  //morph
	CTypedPtrList<CPtrList, Gap_Struct*> gaplist;
private:  //morph
	CTypedPtrList<CPtrList, Requested_Block_Struct*> requestedblocks_list;
	//MORPH - Moved by SiRoB, moved in public area du to Share Only The Need
	/*
	CArray<uint16,uint16> m_SrcpartFrequency;
	*/
	// SLUGFILLER: SafeHash
	CArray<bool,bool> m_PartsShareable;
	int	m_PartsHashing;
	CMutex	ICH_mut;	// ICH locks the file
	CList<UINT,UINT>	m_ICHPartsComplete;
	// SLUGFILLER: SafeHash
	float	percentcompleted;
	CList<UINT,UINT>	corrupted_list;
	uint32	m_ClientSrcAnswered;
	UINT	availablePartsCount;
	CWinThread* m_AllocateThread;
	CWinThread* m_FlushThread; //MORPH - Flush Thread
	DWORD	m_lastRefreshedDLDisplay;
	CUpDownClientPtrList m_downloadingSourceList;
	bool	m_bDeleteAfterAlloc;
    bool	m_bpreviewprio;
	// Barry - Buffered data to be written
	CTypedPtrList<CPtrList, PartFileBufferedData*> m_BufferedData_list;
	uint64	m_nTotalBufferData;
	uint32	m_nLastBufferFlushTime;
	UINT	m_category;
	DWORD	m_dwFileAttributes;
	time_t	m_tActivated;
	time_t	m_nDlActiveTime; //vs2005
	uint32	m_tLastModified;	// last file modification time (NT's version of UTC), to be used for stats only!
	uint32	m_tCreated;			// file creation time (NT's version of UTC), to be used for stats only!
    uint32	m_random_update_wait;	
	volatile EPartFileOp m_eFileOp;
	volatile UINT m_uFileOpProgress;
	CAICHRecoveryHashSet*	m_pAICHRecoveryHashSet;

    DWORD   lastSwapForSourceExchangeTick; // ZZ:DownloadManaager
	
	//MORPH START - Added by SiRoB, Flush Thread
public:
	FlushDone_Struct* m_FlushSetting;
private:
	CCriticalSection m_BufferedData_list_Locker;
	//MORPH END   - Added by SiRoB, Flush Thread

	// khaos::categorymod+
	UINT	m_catResumeOrder;
	// khaos::categorymod-
	// khaos::accuratetimerem+
	uint32	m_nSecondsActive;
	uint64	m_I64uInitialBytes;
	DWORD	m_dwActivatedTick;
	// khaos::accuratetimerem-
	// khaos::kmod+ Advanced A4AF: Force A4AF
	bool	m_bForceAllA4AF;
	bool	m_bForceA4AFOff;
	// khaos::kmod-
	//MORPH START - Added by SiRoB,  SharedStatusBar CPU Optimisation
	bool	InChangedSharedStatusBar;
	CBitmap m_bitmapSharedStatusBar;
	int		lastSize;
	bool	lastonlygreyrect;
	bool	lastbFlat;
	//MORPH END - Added by SiRoB,  SharedStatusBar CPU Optimisation

	//Morph Start - added by AndCycle, ICS
    // enkeyDev: ICS
    CArray<uint16,uint16> m_SrcIncPartFrequency;
    // <--- enkeyDev: ICS
    //Morph End - added by AndCycle, ICS

	//MORPH START - Added by Stulle, Global Source Limit
private: 
	UINT	m_uFileHardLimit; 
public: 
	void	IncrHL(UINT m_uSourcesDif);
	void	DecrHL()	{m_uFileHardLimit = GetSourceCount();}
	void	SetPassiveHL(UINT m_uSourcesDif)	{m_uFileHardLimit = GetSourceCount() + m_uSourcesDif;};
	UINT	GetFileHardLimit() const {return m_uFileHardLimit;}
	void	InitHL();
	bool	IsSrcReqOrAddAllowed();
	//MORPH END   - Added by Stulle, Global Source Limit

	//MORPH START - Added by Stulle, Source cache [Xman]
	void	ProcessSourceCache();
	void	AddToSourceCache(uint16 nPort, uint32 dwID, uint32 dwServerIP,uint16 nServerPort,ESourceFrom sourcefrom, bool ed2kIDFlag=false,  const uchar* achUserHash=NULL, const uint8 byCryptOptions = 0);
	void	ClearSourceCache();
	uint32	GetSourceCacheAmount() const { return m_sourcecache.GetCount();}
private:
	CList<PartfileSourceCache>	m_sourcecache;
	uint32						m_lastSoureCacheProcesstime;
	//MORPH END   - Added by Stulle, Source cache [Xman]

	// EastShare Start - FollowTheMajority by AndCycle
private:
	bool	m_bFollowTheMajority;
	CMap<CUpDownClient*, CUpDownClient*, CString, CString> m_mapSrcFilename;
	CMap<CString, LPCTSTR, int, int> m_mapFilenameCount;
public:
	void	UpdateSourceFileName(CUpDownClient* src);
	void	RemoveSourceFileName(CUpDownClient* src);
	bool	DoFollowTheMajority() const { return this->m_bFollowTheMajority; }
	void	SetFollowTheMajority(bool val) { this->m_bFollowTheMajority = val; }
	void	InvertFollowTheMajority() {m_bFollowTheMajority = !m_bFollowTheMajority;}
	// EastShare End   - FollowTheMajority by AndCycle
};

// SLUGFILLER: SafeHash
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
// SLUGFILLER: SafeHash
//MORPH START - Added by SiRoB, Flush Thread
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
	bool			doRun;
};
//MORPH END   - Added by SiRoB, Flush Thread
