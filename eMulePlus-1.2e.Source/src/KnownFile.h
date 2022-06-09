//this file is part of eMule
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

#include "opcodes.h"
#include "PastComment.h"
#include "Loggable.h"
#include "ed2k_filetype.h"
#pragma warning(push)
#pragma warning(disable:4702) // unreachable code
#include <list>
#include <vector>
#pragma warning(pop)

//	Known file part statuses
#define KFPS_PART_HIDDEN	0x01

class CUpDownClient;
class CJumpstarter;
class Packet;
class CTag;

class CFileStatistic
{
	friend class CKnownFile;
	friend class CPartFile;
	friend class CSharedFilesCtrl;
	friend class CKnownFileList;

public:
	CFileStatistic();
	~CFileStatistic();

	double	GetCompleteReleases() { return completeReleases; }
	double	GetCompletePartReleases(uint16 part);
	void	AddTraffic(uint32 dwPart, const uint64 &qwStart, uint32 dwBytes);

	void	resetStats(bool all=false);
	void	resetPartTraffic(uint16	part, bool all=false);

	void	AddRequest();
	void	AddAccepted();
	void	AddTransferred(uint32 bytes);
	bool	merge(CFileStatistic *m);

	uint16	GetRequests() const				{return m_iNumRequested;}
	uint16	GetAccepts() const				{return m_iNumAccepted;}
	uint64	GetTransferred()	const		{return m_qwNumTransferred;}
	uint32	GetAllTimeRequests() const		{return m_dwAllTimeRequested;}
	uint32	GetAllTimeAccepts() const		{return m_dwAllTimeAccepted;}
	uint64	GetAllTimeTransferred() const	{return m_qwAllTimeTransferred;}

	uint32	GetPartAccepted(uint16 part, bool session) const;
	CKnownFile* fileParent;

private:
	uint16	m_iNumRequested;
	uint16	m_iNumAccepted;
	uint64	m_qwNumTransferred;
	uint64	m_qwAllTimeTransferred;
	uint32	m_dwAllTimeRequested;
	uint32	m_dwAllTimeAccepted;

	uint32	*partAccepted;				// all time accepted requests on part level
	uint32	*partAcceptedSession;		// session accepted requests on part level

	uint32	*partTraffic;				// all time traffic on part level
	uint32	*partTrafficSession;		// session traffic on part level

	uint32	*blockTraffic;				// all time traffic on block level
	uint32	*blockTrafficSession;		// session traffic on block level

	double	completeReleases;			// how often the complete file was uploaded (basing on all time block-traffic)

	void	initTraffic();				// create the traffic arrays
	void	recalcCompleteReleases();	// recalculate the complete release count, don't do this to often
};

/*
					   CPartFile
					 /
		  CKnownFile
		/
CAbstractFile
		\
		  CSearchFile
*/
class CAbstractFile : public CLoggable
{
public:
	CAbstractFile() : m_strFileName(_T("")), m_strFileExtension(_T(""))
	{
		m_qwFileSize = 0;
		m_eFileType = ED2KFT_ANY;
	}
	virtual ~CAbstractFile();

	const CString&	GetFileName() const				{return m_strFileName;}
	virtual void	SetFileName(const CString &NewName, bool bClearName = true);
	bool			IsFileNameEmpty()				{return m_strFileName.IsEmpty();}
	int				CmpFileNames(const TCHAR *pcName2) const	{return _tcsicmp(m_strFileName, pcName2);}

	const CString&	GetFileExtension() const		{return m_strFileExtension;}

	uint64			GetFileSize() const				{ return m_qwFileSize; }
	virtual void	SetFileSize(uint64 qwFileSize)	{ m_qwFileSize = qwFileSize; }
	bool			IsLargeFile() const				{ return (m_qwFileSize > OLD_MAX_EMULE_FILE_SIZE); }

	const uchar*	GetFileHash() const				{return m_fileHash;}

	CString			GetFileTypeString();
	LPCSTR			GetSearchFileType(uint32 *pdwType);
	int				CmpFileTypes(uint32 dwType2) const;
	EED2KFileType	GetFileType() const				{ return m_eFileType; }

	CString			CreateED2kLink() const;
	CString			CreateED2kSourceLink() const;
	CString			CreateHTMLED2kLink() const;
protected:
	CString		m_strFileName;
	CString		m_strFileExtension;
	uchar		m_fileHash[16];
	CArray<CTag*, CTag*>	m_tagArray;
private:	//	Several data sections to improve data alignment
	uint64		m_qwFileSize;
protected:
	EnumPartFileRating	m_eRating;
private:
	EED2KFileType m_eFileType;
};

typedef std::list<CUpDownClient*> ClientList;

class CKnownFile : public CAbstractFile
{
public:
	CKnownFile();
	virtual ~CKnownFile();
	bool	CreateFromFile(const CString &strDir, const CString &strFileName, bool bPartFile); // create date, hashset and tags from a file
	const CString&	GetPath() const				{return m_strKnownFileDirectory;}
	void		SetPath(const CString& strPath)	{ m_strKnownFileDirectory = strPath; }

//	m_strFilePath contains:
//	1) full file name for known file and complete part file
//	2) full part file name (.part) for incomplete part file
	const CString& GetFilePath() const { return m_strFilePath; }
	void	SetFilePath(const CString& strFilePath)	{ m_strFilePath = strFilePath; }


	void			SetFileSize(uint64 qwFileSize);
	virtual void	SetFileName(const CString& NewName, bool bClearName = true);

	bool	IsCompressedTransferAllowed()	{return m_bIsCompressedTransferAllowed;}
	virtual bool	IsPartFile()			{ return false; }
	virtual bool	IsPaused() const		{ return false; }
	virtual bool	IsStopped() const		{ return false; }
	virtual bool	IsCompleting() const	{ return false; }
	bool	LoadFromFile(CFile &file);	//load date, hashset and tags from a .met file
	bool	WriteToFile(CFile &file);
	uint32	GetFileDate() const				{ return m_timetLastWriteDate; }
	double	GetPopularityRatio();
	uint16	GetHashCount()					{ CSingleLock Lock(&m_csHashList, TRUE); return static_cast<uint16>(m_partHashArray.GetCount()); }
	const uchar*	GetPartHash(uint32 dwPart) const;
	const uchar*	GetHashSet() const		{ return m_pPartsHashSet; }
	uint16	GetPartCount() const			{return m_uPartCount;}
	uint32	GetED2KPartHashCount() const	{return m_dwED2KPartHashCount;}
	uint32	GetED2KPartCount() const		{return m_dwED2KPartCount;}
	uint32	GetLastPartSize() const			{return m_dwLastPartSz;}
	uint32	GetLastBlockSize() const		{return m_dwLastBlkSz;}
	uint32	m_timetLastWriteDate;

	byte	GetULPriority(void) const				{ return m_bytePriority; }
	void	SetULPriority(byte iNewPriority)		{m_bytePriority = iNewPriority;}
	CString	GetKnownFilePriorityString() const;
	void	UpdateUploadAutoPriority(void);
	bool	IsULAutoPrioritized() const				{ return m_bAutoPriority; }
	void	SetAutoULPriority(bool newAutoPriority)	{ m_bAutoPriority=newAutoPriority; }

//	Movie Preview Mode
	byte	GetMovieMode()						{return m_byteMoviePreviewMode;}
	void	SetMovieMode(byte byteNewMode)		{m_byteMoviePreviewMode = byteNewMode;}

//	Shared file view permissions (all, only friends, no one)
	byte	m_bytePermissions;
	byte	GetPermissions(void)			{return m_bytePermissions;}
	void	SetPermissions(byte byteNewPermissions) {m_bytePermissions = byteNewPermissions;}
	CString	GetPermissionString();

	bool	LoadHashsetFromFile(CFile& file, bool checkhash);

	CFileStatistic statistic;

	const CString&	GetFileComment()         {if (!m_bCommentLoaded) LoadComment(); return m_strComment;}
	void	SetFileComment(const CString &strNewComment);
	void	SetFileRating(EnumPartFileRating eNewRating);
	EnumPartFileRating	GetFileRating()         	{if (!m_bCommentLoaded) LoadComment(); return m_eRating;}
	void	RemoveFileCommentAndRating();

	uint16	GetCompleteSourcesCount()		{return m_nCompleteSourcesCount;}
	void	GetCompleteSourcesRange(uint16 *lo, uint16 *hi)	{*lo= m_nCompleteSourcesCountLo; *hi= m_nCompleteSourcesCountHi;}

	uint32	GetTrafficBlock(uint64 qwStart, uint64 qwEnd, bool bSession = false);
	uint32	GetTrafficPart(uint64 qwStart, uint64 qwEnd, bool bSession = false);
	uint32	GetPartTraffic(uint16 part, bool session=false);
	uint32	GetBlockTraffic(uint16 block, bool session=false);
	bool	LoadFromFileTraffic(FILE* file, byte version);
	bool	SaveToFileTraffic(FILE* file);

	uint32	GetPartSize(uint32 dwPart) const;
	uint32	GetBlockSize(uint32 dwBlock) const;

	uint32	GetBlockCount() const				{return m_dwBlockCount;}

	void	SetSharedFile(bool b) { m_sharedFile=b; }
	bool	GetSharedFile() { return m_sharedFile; }

	void	SetPublishedED2K(bool bVal)		{ m_bPublishedED2K = bVal; }
	bool	GetPublishedED2K() const		{ return m_bPublishedED2K; }

	byte*	GetPartStatusArr()		 		{ return &m_PartsStatusVector[0]; }
	bool	IsPartShared(uint32 dwPart)		{ return ((m_PartsStatusVector[dwPart] & KFPS_PART_HIDDEN) == 0); }
	void	SharePart(uint16 uPart)			{m_PartsStatusVector[uPart] &= ~KFPS_PART_HIDDEN;}
	void	UnsharePart(uint16 uPart)		{m_PartsStatusVector[uPart] |= KFPS_PART_HIDDEN;}
	
	bool	HasHiddenParts();
	void	WritePartStatus(CFile* file);

	virtual	Packet*	CreateSrcInfoPacket(const CUpDownClient *pForClient, byte byteRequestedVer, uint16 uRequestedOpt);
	void	CalculateCompleteSources();

	bool	GetJumpstartEnabled() const		{ return m_Jumpstarter!=NULL; }
	void	SetJumpstartEnabled(bool enabled);
	bool	IsJsComplete();
	void	AddSentBlock(CUpDownClient *client, const uint64 &qwStartOffset, uint32 togo);
	void	WriteJumpstartPartStatus(CUpDownClient* client, CMemFile* data);
	bool	AllowChunkForClient(uint32 partNo, CUpDownClient* client);

	void	UpdateSharedFileDisplay();

	virtual	int ReadFileForUpload(uint64 qwOffset, uint32 dwBytesToRead, byte *pbyteBuffer);

	double	GetSizeRatio()		{return m_dblSizeRatio;}

//	Source Exchange
	void	AddClientToSourceList(CUpDownClient* pClient);
	void	RemoveClientFromSourceList(CUpDownClient* pClient);
	void	GetCopySourceList(ClientList *pCopy);

protected:
	CCriticalSection	m_csHashList;

	bool	LoadTagsFromFile(CFile& file);
	bool	LoadDateFromFile(CFile& file);
	void	CreateHashFromFile(CFile *file, uint32 dwLength, uchar *pbyteHash)	{CreateHashFromInput(file, dwLength, pbyteHash, 0);}
	void	CreateHashFromString(uchar *pbyteMem, uint32 dwLength, uchar *pbyteHash)	{CreateHashFromInput(0, dwLength, pbyteHash, pbyteMem);}
	void	LoadComment();

	bool					m_sharedFile;
	CArray<uchar*, uchar*>	m_partHashArray;
	CString					m_strFilePath;
	CString					m_strComment;
	CString					m_strKnownFileDirectory;

private:
	void	CreateHashFromInput(CFile *file, uint32 dwLength, uchar *pbyteHash, uchar *pbyteMem);
	bool	m_bAutoPriority;
	bool	m_bCommentLoaded;
	uint16	m_uPartCount;
	uint32	m_dwED2KPartCount;
	uint32	m_dwED2KPartHashCount;
	uint32	m_dwBlockCount;
	uint32	m_dwLastPartSz;
	uint32	m_dwLastBlkSz;
	uint16	m_nCompleteSourcesCount;
	uint16	m_nCompleteSourcesCountLo;
	uint16	m_nCompleteSourcesCountHi;
	uint32	m_nCompleteSourcesTime;
	CJumpstarter	*m_Jumpstarter;
	uchar*			m_pPartsHashSet;
	double	m_dblSizeRatio;
	byte	m_bytePriority;
	byte	m_byteMoviePreviewMode;
	bool	m_bIsCompressedTransferAllowed;
	bool	m_bPublishedED2K;

	std::vector<byte> m_PartsStatusVector;

	ClientList 	m_SourceList;
	CRITICAL_SECTION m_csSourceList;
};

// Constants for MD4Transform
#define S11 3
#define S12 7
#define S13 11
#define S14 19
#define S21 3
#define S22 5
#define S23 9
#define S24 13
#define S31 3
#define S32 9
#define S33 11
#define S34 15

// Basic MD4 functions
#define MD4_F(x, y, z) ((((y) ^ (z)) & (x)) ^ (z))
#define MD4_G(x, y, z) (((z) & ((x) ^ (y))) | ((x) & (y)))
#define MD4_H(x, y, z) ((x) ^ (y) ^ (z))

// Rotates x left n bits
#ifdef _MSC_VER
#pragma intrinsic(_rotl)
#define MD4_ROTATE_LEFT(x, n) _rotl((x), (n))
#else
#define MD4_ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))
#endif

// Partial transformations
#define MD4_FF(a, b, c, d, x, s) \
{ \
  (a) += MD4_F((b), (c), (d)) + (x); \
  (a) = MD4_ROTATE_LEFT((a), (s)); \
}

#define MD4_GG(a, b, c, d, x, s) \
{ \
  (a) += MD4_G((b), (c), (d)) + (x) + (uint32)0x5A827999; \
  (a) = MD4_ROTATE_LEFT((a), (s)); \
}

#define MD4_HH(a, b, c, d, x, s) \
{ \
  (a) += MD4_H((b), (c), (d)) + (x) + (uint32)0x6ED9EBA1; \
  (a) = MD4_ROTATE_LEFT((a), (s)); \
}

static void __fastcall MD4Transform(uint32 Hash[4], uint32 x[16]);
