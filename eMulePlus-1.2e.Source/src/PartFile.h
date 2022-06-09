//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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

#include "KnownFile.h"
#include "opcodes.h"
#include "SourceSaver.h"
#include "PastComment.h"
#include "Category.h"

enum _EnumPartFileStatuses
{
	PS_READY			= 0,
	PS_EMPTY			= 1,
	PS_WAITINGFORHASH	= 2,
	PS_HASHING			= 3,
	PS_ERROR			= 4,
	PS_UNKNOWN			= 6,
	PS_PAUSED			= 7,
	PS_COMPLETING		= 8,
	PS_COMPLETE			= 9,
	PS_STOPPED			= 10,
	PS_DOWNLOADING		= 20,	//MOREVIT - Dummy status
	PS_WAITINGFORSOURCE	= 21,	//MOREVIT - Dummy status
	PS_STALLED			= 22	//MOREVIT - Dummy status
};
typedef EnumDomain<_EnumPartFileStatuses>	EnumPartFileStatuses;

//	The number of days without a transfer before a download is considered stalled
#define WARN_PERIOD_OF_NO_PROGRESS	7

#define BUFFER_TIME_LIMIT			MIN2MS(5)	// File data flush interval

// Defines for status of filled parts
#define PART_CORRUPTED				0x01000000
#define PART_VERIFIED				0x02000000
#define NOT_AVAILABLE				0x00FFFFFF

#define PARTIALLY_BLOCKED			0x01
#define FULLY_BLOCKED				0x02
#define FREE_TO_DL					0x00

class CSearchFile;
class CUpDownClient;
class CClientSource;
struct Gap_Struct;
struct Requested_Block_Struct;

struct PartFileBufferedData
{
	BYTE				*pbyteBuffer;	// This is the data to be written
	uint64				qwStartOffset;	// This is the start offset of the data
	uint64				qwEndOffset;	// This is the end offset of the data
};

class CPartFile : public CKnownFile
{
public:
	CPartFile();
	CPartFile(CSearchFile* searchresult, EnumCategories eCatID);
	CPartFile(CString edonkeylink, EnumCategories eCatID);
	CPartFile(class CED2KFileLink* fileLink, EnumCategories eCatID);
	void	InitializeFromLink(CED2KFileLink* fileLink, EnumCategories eCatID);
	virtual	~CPartFile();

//	file status functions (get status)
	EnumPartFileStatuses	GetStatus() const;
	EnumPartFileStatuses	GetRawStatus() const { return m_eStatus; }
	void	SetStatus(EnumPartFileStatuses eNewStatus);
	int		GetPartFileStatusID() const;
	int		GetPartFileStatusRang();
	bool	IsPartFile()				{return !(m_eStatus == PS_COMPLETE);}
	bool	IsPaused() const			{return m_bPaused;}
	bool	IsStopped() const			{return m_bStopped;}
	bool	IsStalled() const;
	bool	IsCompleting() const		{return (m_eStatus == PS_COMPLETING);}
	bool	IsPreallocated() const		{ return m_bIsPreallocated; }
	bool	PreviewAvailable();
	bool	AllowGet1stLast();
	bool	IsArchive() const;
	bool	IsArchivePreviewable() const	{ return m_bIsPreviewableArchive; }
	bool	IsMovie() const				{ return (m_cIsMovie != 0); }
	bool	IsAviMovie() const			{ return (m_cIsMovie == 1); }
	bool	IsAviKindMovie() const		{ return (m_cIsMovie > 0); }
	bool	IsMpgMovie() const			{ return m_bIsMpgMovie; }
	bool	IsMpgAudio() const			{ return m_bIsMpgAudio; }

//	file status functions (set & change status)
	void	DeleteFile();
	void	StopFile(bool bUpdateDisplay = true);
	void	PauseFile();
	void	ResumeFile();
	void	PreviewFile();
	void	AllocateNeededSpace();

//	file property & statistic functions
	uint64	GetRealFileSize();
	uint64	GetSizeToTransfer() const				{ return m_qwGapsSum; }

//	file name & dirs related
	void	SetFileName(const CString& NewName, bool bClearName = true);
	const CString&	GetFullName() const						{return m_strFullName;}
	const CString&	GetPartMetFileName() const				{return m_strPartMetFileName;}
	const CString&	GetTempDir() const						{return m_strTempDir;}
	CString	GetOutputDir();
	void	SetAlternativeOutputDir(CString* path);
	bool	IsAlternativeOutputDir() const			{return !m_strAlternativePath.IsEmpty();}

//	file priority
	bool	IsAutoPrioritized() 					{ return m_bAutoPriority ; }
	void	SetAutoPriority(bool newAutoPriority) 	{ m_bAutoPriority = newAutoPriority ; }
	void	SetPriority(byte byteNewPriority, bool bSaveSettings = true);
	byte	GetPriority()							{return priority;}
	void	UpdateDownloadAutoPriority(void);

//	file comments
	bool	HasComment() const		{return m_bHasComment;}
	void	SetHasComment(bool in)			{m_bHasComment=in;}
	void	CheckAndAddPastComment(CUpDownClient *pClient);
	void	RemovePastComment(CUpDownClient *pClient, bool bRestore= true);
	CPastCommentList &GetPastCommentList()			{ return m_pastCommentList; }

//	file category
	EnumCategories	GetCatID() const;
	void			SetCatID(EnumCategories eCatID);

//	file rating
	EnumPartFileRating		GetRating();
	bool	HasRating() const			{return m_bHasRating;}
	void	SetHasRating(bool in)			{m_bHasRating=in;}
	void	UpdateFileRatingCommentAvail();


//	part status function (get functions)
	bool	IsComplete(uint64 qwStart, uint64 qwEnd);
	bool	IsPartComplete(uint32 dwPart) const;
	bool	IsPartComplete(uint32 dwPart, uint64 *pqwStart, uint64 *pqwEnd) const;
	bool	IsPartFull(uint32 dwPart) const;
	bool	IsPartDownloading(uint32 dwPart) const;
	bool	IsCorruptedPart(uint32 dwPartNum) const {return ((PART_CORRUPTED & m_PartsStatusVector[dwPartNum]) != 0);}

	uint32	GetPartLeftToDLSize(uint32 dwPartNum) const {return (0x00FFFFFF & m_PartsStatusVector[dwPartNum]);}

	uint16	GetNextRequiredPart(CUpDownClient* sender);
	uint16	GetSrcPartFrequency(uint32 part) { return m_srcPartFrequencies[part]; }
	uint32	GetAvailablePartCount() const	{return m_dwAvailablePartsCount;}
	void	GetFirstLastChunk4Preview();

//	part status functions (set & change status)
	void	FileRehashingStarted();
	void	PartFileHashFinished(CKnownFile *pKnownFile);
	bool	HashSinglePart(uint32 dwPartNum); // true = ok, false = corrupted
	void	NewSrcPartsInfo();

//	source functions
	void	AddClientSource(CClientSource *pSource, int iSource=0, bool bExchanged=false, byte byteSXVer = 0);
	void	AddClientSources(CTypedPtrList<CPtrList, CClientSource*>*);
	void	AddClientSources(CMemFile *sources, byte byteSXVer, bool bSX2, const CUpDownClient *pClient = NULL);
	void	AddServerSources(CMemFile &pSources, uint32 dwSrvIP, uint16 uSrvPort, bool bWithObfuscationAndHash);
	virtual	Packet* CreateSrcInfoPacket(const CUpDownClient *pForClient, byte byteRequestedVer, uint16 uRequestedOpt);
	void	GetSourcesAfterServerConnect();
	void	RemoveNoNeededSources(uint32 dwNumberSources2Remove);
	void	RemoveAllSources();
	void	DownloadAllA4AF(bool bSameCat = false);
	void	GetCompleteSourcesRange(uint16 *lo, uint16 *hi)	{ *lo = *hi = GetCompleteSourcesCount(); }

	void	GetCopySourceList(EnumDLQState eClientDS, ClientList *pCopy, bool bClearSourceList = false);
	void	GetCopySourceLists(uint32 dwListsMask, ClientList *pCopy, bool bClearSourceLists = false);
	bool	RemoveClientFromDLSourceList(CUpDownClient *pClient);
	void	AddClientToSourceList(CUpDownClient *pClient, EnumDLQState eClientDS);
	bool	IsClientInSourceList(CUpDownClient *pClient);
	void	ClearSourceLists();
	void	SwapClientBetweenSourceLists(CUpDownClient *pClient, EnumDLQState eSourceDS, EnumDLQState eTargetDS);

	void	AddClientToA4AFSourceList(CUpDownClient *pClient);
	void	RemoveClientFromA4AFSourceList(CUpDownClient *pClient);
	void	GetCopyA4AFSourceList(ClientList *pCopy, bool bClearSourceList = false);
	void	ClearA4AFSourceList();

//	source statistic functions
	uint16	GetSourceCount();
	uint16	GetTransferringSrcCount() const			{ return m_uNumTransferringSrcs; }
	uint16	GetNoNeededPartsSrcCount() const		{ return m_uSrcNNP; }
	uint16	GetOnQueueSrcCount() const				{ return m_uSrcOnQueue; }
	uint16	GetHighQRSrcCount() const				{ return m_uSrcHighQR; }
	uint16	GetConnectingSrcCount() const			{ return m_uSrcConnecting; }
	uint16	GetWaitForFileReqSrcCount() const		{ return m_uSrcWaitForFileReq; }
	uint16	GetConnectedSrcCount() const			{ return m_uSrcConnected; }
	uint16	GetConnectingViaServerSrcCount() const	{ return m_uSrcConnViaServer; }
	uint16	GetLow2LowSrcCount() const				{ return m_uSrcLowToLow; }
	uint16	GetLowIDOnOtherServer() const			{ return m_uSrcLowIDOnOtherServer; }
	uint16	GetQueueFullSrcCount() const			{ return m_uSrcQueueFull; }
	uint16	GetSrcA4AFCount() const					{ return m_uSrcA4AF; }
	uint32	GetNotCurrentSourcesCount();
	uint32	GetValidSourcesCount();
	uint16	GetCompleteSourcesCount()				{return m_uLastCompleteSrcCount;}

//	gap functions
	void	AddGap(uint64 qwStart, uint64 qwEnd);
	void	FillGap(uint64 qwStart, uint64 qwEnd);
	uint32	GetGapsInPart(uint32 dwPartNum, Requested_Block_Struct **ppNewBlocks, uint32 dwCount);
	// Barry - This will invert the gap list, up to caller to delete gaps when done
	// 'Gaps' returned are really the filled areas, and guaranteed to be in order
	void	GetFilledList(CTypedPtrList<CPtrList, Gap_Struct*> *filled);

//	block functions
	bool	IsLastBlockComplete();
	void	RemoveBlockFromList(const uint64 &qwStart, const uint64 &qwEnd);
	// Barry - Added to prevent list containing deleted blocks on shutdown
	void	RemoveAllRequestedBlocks(void);

#define PF_WR2BUF_FL_FREEBUFFER		0x01
#define PF_WR2BUF_FL_ENDOFBLOCK		0x02
	uint32	WriteToBuffer(sint32 iComprGain, BYTE *data, uint64 qwStart, uint64 qwEnd, int iFlags);

//	time function
	void	SetStartTimeReset(bool bEnable)		{m_StartTimeReset = bEnable;}
	bool	GetStartTimeReset()					{return m_StartTimeReset;}
	CTimeSpan GetFlushTimeSpan()			{return (m_timeLastDownTransfer - m_SessionStartTime);}
	CTimeSpan GetSessionTimeSpan()			{return (CTime::GetCurrentTime() - m_SessionStartTime);}
	uint32	GetLastPurgeTime()					{ return m_dwLastPurgeTime; }
	sint32	GetTimeRemaining(bool bAvgerage = false);
	uint32	GetLastAnsweredTime()			{ return m_ClientSrcAnswered; }
	const CTime&	GetLastDownTransfer() const			{ return m_timeLastDownTransfer; }
	void	SetLastAnsweredTime()			{ m_ClientSrcAnswered = ::GetTickCount(); }
	void	SetLastAnsweredTimeTimeout()	{ m_ClientSrcAnswered = 2 * CONNECTION_LATENCY + ::GetTickCount() - SOURCECLIENTREASK; }

//	File statistics (session & cumulative)
	uint64	GetTransferred() const					{ return m_qwBytesTransferred; }
	uint64	GetCompletedSize() const				{ return m_qwCompletedSize; }
	uint32	GetDataRate()							{return m_dwDataRate;}
	void	AddRxAmount(uint32 dwRxBytes)			{ m_qwBytesTransferred += static_cast<uint64>(dwRxBytes); }
	void	AddRxCorruptedAmount(uint32 dwBadBytes);
	uint32	GetAvgDataRate(bool bUpdated = false);
	uint64	GetSessionTransferred()					{ return ((m_qwCompletedSize > m_qwSessionStartSize) ? (m_qwCompletedSize - m_qwSessionStartSize) : 0); }
	uint64	GetLostDueToCorruption() const			{return m_qwLostDueToCorruption;}
	uint64	GetGainDueToCompression() const			{return m_qwGainDueToCompression;}
	uint32	TotalPacketsSavedDueToICH() const		{return m_iTotalPacketsSavedDueToICH;}
	void	UpdateCompletedInfos();
	double	GetPercentCompleted() const				{return m_dblPercentCompleted;}
//	Don't return more than 99.99 for 2 digits display as it will be rounded to 100.00 during number to string conversion
	double	GetPercentCompleted2() const			{return ((m_dblPercentCompleted > 99.99) && (m_dblPercentCompleted < 100.0)) ? 99.99 : m_dblPercentCompleted;}

//	I/O functions
	void	FlushBuffer(void);
	CFile&	GetPartFileHandle()				{return m_hPartFileRead;}
	bool	LoadPartFile(LPCTSTR in_directory, LPCTSTR filename);
	bool	TryToRecoverPartFile(LPCTSTR in_directory, LPCTSTR in_filename);
	bool	SavePartFile();
	void	WritePartStatus(CFile* file);
	void	WriteCompleteSourcesCount(CFile* file);
	void	LoadSettingsFile();
	void	SaveSettingsFile();
	bool	SavePartFileStats();
	int	ReadFileForUpload(uint64 qwOffset, uint32 dwBytesToRead, byte *pbyteBuffer);


//	GUI related functions
	void	DrawStatusBar(CDC* dc, RECT* rect, bool bFlat);
	void	UpdateDisplayedInfo();
	CString	GetPartFileStatus();
	void	GetProgressString(CString *pstrChunkBar, uint32 dwSize);
	CString	GetDownloadFileInfo();
	CString	GetDownloadFileInfo4Tooltips();
	CString	LocalizeLastSeenComplete()	{return DtLocale(lastseencomplete);}
	CString	LocalizeLastDownTransfer()	{return DtLocale(m_timeLastDownTransfer);}

//	unsorted
	uint32	Process(uint32 dwReduceDownload, uint32 dwIteration);
	byte*	MMCreatePartStatus();
	static UINT AllocateNeededSpaceProc(LPVOID lpParameter);
	CString	CreateED2KSourceLink(uint32 dwExpireIn, int iSourceCnt);

	bool	IsFakesDotRar() const						{ return m_bIsFakesDotRar; }
	void	SetFakesDotRar(bool bIsFakesDotRar = true)	{ m_bIsFakesDotRar = bIsFakesDotRar; }

protected:
	bool	IsAlreadyRequested(uint64 qwStart, uint64 qwEnd);
	void	CompleteFile(bool hashingdone);
	void	CreatePartFile();
	void	Init();

private:
	CString	DtLocale(CTime dt);
	BOOL	PerformFileComplete();
	static UINT CompleteThreadProc(CPartFile* pFile);
	CString	GetStatsFullPath() const;
	bool	LoadPartFileStats();
	void	RemovePartFileStats();
	void	ClosePartFile();
	void	UpdateGUIAfterStateChange();
	void	GetGapListCopy(CArray<Gap_Struct, Gap_Struct> *pGapArray);
	void	FillGapInParts(uint64 qwGapStart, uint64 qwGapEnd);
	void	AddGapToParts(uint64 qwGapStart, uint64 qwGapEnd);

// --- variable sections ---

public:
	// Barry - Is archive recovery in progress
	volatile bool	m_bRecoveringArchive;
	volatile bool	m_bPreviewing;

	bool		m_bHashSetNeeded;
	CTime		lastseencomplete;

protected:
	CSourceSaver		m_sourcesaver;

private:
//	Separate read and write handles to eliminate I/O collisions
	CFile	m_hPartFileWrite;
	CFile	m_hPartFileRead;

	EnumCategories	m_eCategoryID;
	bool			m_bDataFlushReq;
	uint16			m_uProcessCounter;
//	Statistics
	uint16	m_uNumTransferringSrcs;
	uint16	m_uSrcNNP;
	uint16	m_uSrcOnQueue;
	uint16	m_uSrcHighQR;
	uint16	m_uSrcConnecting;
	uint16	m_uSrcWaitForFileReq;
	uint16	m_uSrcConnected;
	uint16	m_uSrcConnViaServer;
	uint16	m_uSrcLowToLow;
	uint16	m_uSrcLowIDOnOtherServer;
	uint16	m_uSrcQueueFull;
	uint16	m_uSrcA4AF;
	uint64	m_qwCompletedSize;
	uint64	m_qwLostDueToCorruption;
	uint64	m_qwGainDueToCompression;
	uint32	m_iTotalPacketsSavedDueToICH;
	uint32	m_dwDataRate;
	CString	m_strFullName;
	CString	m_strTempDir;
	CString	m_strPartMetFileName;
	uint64	m_qwBytesTransferred;
	bool	m_bPaused;
	bool	m_bStopped;
	byte	priority;
	EnumPartFileStatuses	m_eStatus;
	uint32	m_dwLastFileSourcesRequestTime;
	uint32	m_dwLastPurgeTime;
	uint32	m_LastNoNeededCheck;

	CTypedPtrList<CPtrList, Gap_Struct*> gaplist;
	CTypedPtrList<CPtrList, Requested_Block_Struct*> requestedblocks_list;
	CArray<uint16,uint16> m_srcPartFrequencies;
	uint16	m_uMaxSrcPartFrequency;
	std::vector<uint32> m_PartsStatusVector;
	CRITICAL_SECTION	m_csSourceLists;
	ClientList			m_SourceLists[DS_LAST_QUEUED_STATE];
	ClientList			m_A4AFSourceLists;
	double	m_dblPercentCompleted;
	uint32	m_dwAvailablePartsCount;
	uint32	m_ClientSrcAnswered;
	bool	m_bAutoPriority;
	bool	m_bIsBeingDeleted;
	bool	m_bIsPreallocated;

	CTime	m_SessionStartTime;
	uint64	m_qwSessionStartSize;
	uint32	m_AvgDataRate;
	uint64	m_qwCompletedPartsSize;
	bool	m_StartTimeReset;

	bool	m_bIsFakesDotRar;
	bool	m_bHasRating;
	bool	m_bHasComment;

	CTypedPtrList<CPtrList, PartFileBufferedData*> m_BufferedData_list;
	uint32	m_nTotalBufferData;
	uint32	m_nLastBufferFlushTime;
	uint16	m_uLastCompleteSrcCount;

	CCriticalSection	m_csSavePartFile;
	CCriticalSection	m_csGapListAndPartStatus;
	CCriticalSection	m_csFileCompletion;

	CTime	m_timeLastDownTransfer;
	CString	m_strAlternativePath;

	CPastCommentList m_pastCommentList;

	uint64	m_qwGapsSum;

	bool	m_bIsPreviewableArchive;
	char	m_cIsMovie;	// 0: non-video, 1: AVI, 2: AVI kind video, < 0: another format
	bool	m_bIsMpgMovie;
	bool	m_bIsMpgAudio;
};
