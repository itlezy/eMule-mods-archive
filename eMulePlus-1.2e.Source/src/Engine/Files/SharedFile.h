#pragma once
////////////////////////////////////////////////////////////////////////////////////////
// describes a shared file
////////////////////////////////////////////////////////////////////////////////////////
/*
		  CKnownFile - CSharedFile - CPartFile
		/
CAbstractFile
		\
		  CSearchFile
*/
////////////////////////////////////////////////////////////////////////////////////////
#include "KnownFile.h"
#include "../Other/MDX_Hash.h"
#include "FileMD4Part.h"
////////////////////////////////////////////////////////////////////////////////////////
// Permission values for shared files
#define PERM_ALL		0
#define PERM_FRIENDS	1
#define PERM_NOONE		2

////////////////////////////////////////////////////////////////////////////////////////
typedef vector<bool> SharedPartsVector;
typedef vector<CFileMD4Part*> PartVector;
typedef list<CUpDownClient*> SourceList;
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
#ifdef JUMPSTART
class CJumpstarter;
#endif

class CSharedFile : public CKnownFile
{
public:
	CSharedFile();
	~CSharedFile();

	// path to the data part of file object
	CString	GetPath() const					{return m_strSharedFileDirectory;}
	void		SetPath(const CString& path) 		{ m_strSharedFileDirectory = path; }

	// filename with path of the data part of file object
	const CString& GetFilePath() const 				{ return m_strFilePath; }
	void			 SetFilePath(LPCTSTR pszFilePath) 	{m_strFilePath = pszFilePath;}

//	share property, is file shared or not
	void	SetFileShared(bool b) 		{ m_bSharedFile = b; }
	bool	IsFileShared() 				{ return m_bSharedFile; }

//	publish status
	void PublishOnCurServer(bool b)			{m_bPublishedOnCurServer = b;}
	bool IsFilePublishedOnCurServer()		{return m_bPublishedOnCurServer;}

	//upload priotity of the file
	USHORT	GetPriority(void)					{return m_uPriority;};
	void		SetPriority(USHORT uNewPriority) 	{m_uPriority = uNewPriority;};

	bool		IsAutoPrioritized() 							{return m_bAutoPriority;}
	void		SetAutoPriorityEnabled(bool bAutoPriority) 	{m_bAutoPriority = bAutoPriority;}

#ifdef JUMPSTART
//	jumpstart
	bool GetJumpstartEnabled(void) {return m_pJumpstarter != NULL;};
	void SetJumpstartEnabled(bool bEnabled);
	bool IsJumstartComplete();
//	jump start subproperties
	void AddSentBlock(CUpDownClient* client, uint32 start_offset, uint32 togo);
	void WriteJumpstartPartStatus(CUpDownClient* client, CMemFile* data);
	bool AllowChunkForClient(uint32 partNo, CUpDownClient* client);
#endif

// vitual status functions
	virtual	bool	IsPartFile()			{return false;}
	virtual	bool	IsPaused()			{return false;}
	virtual	bool	IsStopped()		{return false;}
	virtual	bool	IsCompleting()		{return false;}

//	Shared file view permissions (all, only friends, no one)
	byte		GetViewPermissions(void)			{return m_byteViewPermissions;};
	void		SetViewPermissions(byte byteNew)	{m_byteViewPermissions = byteNew;};

//	save & load operation in Db
	virtual bool	Load();
	virtual bool	Save(CEmEngine& stEngine);
	virtual void 	GetFileTagsFromStream(CTagStream* pTagStream);
	virtual void 	PutFileTagsInStream(CTagStream* pTagStream);

//	file operations 
	virtual bool 	CreateHashsetFromFile(const CString &strDirectory,const CString &strFileName);

//	import file properties (read the file description)
	virtual bool	ImportFromFile(CFile& file);	//load date, hashset and tags from a .met file
	bool			ImportHashsetFromFile(CFile& file);
	bool			ImportTagsFromFile(CFile &file);


//	wrapper fuctions for shared parts properties
	BOOL	HasSharedParts();
	uchar*	GetPartHash(ULONG ulPartIdx);
	BOOL	GetPartSharedStatus(ULONG ulPartIdx);
	VOID	SetPartSharedStatus(ULONG ulPartIdx, BOOL bStatus);

//	public variables
	PartVector	m_MD4Parts;

/*
eklmn: will be implemented later on
	CString	GetFileComment()         
	{
		if (!m_bCommentLoaded) 
			LoadComment(); 
		return m_strComment;
	}
	void		SetFileComment(CString strNewComment);

	void	SetFileRating(EnumPartFileRating eNewRating);
	EnumPartFileRating	GetFileRating()         	{if (!m_bCommentLoaded) LoadComment(); return m_eRating;}
	void	RemoveFileCommentAndRating();

eklmn: 
	uint16	GetHashCount()				{ CSingleLock Lock(&m_mutexHashList,TRUE); return m_partHashArray.GetCount();}

eklmn: should moved to PartFile (cause we can allways view(prieview) the file, which we have)
	//	Movie Preview Mode
	byte	m_iMoviePreviewMode;
	byte	GetMovieMode()						{return m_iMoviePreviewMode;}
	void	SetMovieMode(byte iNewMode) 		{m_iMoviePreviewMode = iNewMode;}

eklmn: should moved in shared file list
	double	GetFileRatio(void) ;

eklmn: will be reimplemented. cause the clients with complete source will never request the file 

	void		CalculateCompleteSources();
	uint16	GetCompleteSourcesCount()		{return m_nCompleteSourcesCount;}
	void		GetCompleteSourcesRange(uint16 *lo, uint16 *hi)	{*lo= m_nCompleteSourcesCountLo; *hi= m_nCompleteSourcesCountHi;}

eklmn: it has nothing to do with file properties
	virtual	Packet*	CreateSrcInfoPacket(CUpDownClient* forClient);


eklmn: the part traffic feature should be reimplmented
	uint32	GetTrafficPart(uint32 start, uint32 end, bool session=false);
	uint32	GetPartTraffic(uint16 part, bool session=false);
	bool	LoadFromFileTraffic(FILE* file, byte version);
	bool	SaveToFileTraffic(FILE* file);
*/

protected:
	CString 			m_strFilePath;
	CString				m_strSharedFileDirectory;


//	current status of the shared file
	bool				m_bSharedFile;
	bool				m_bPublishedOnCurServer;

//	view permission
	byte				m_byteViewPermissions;

//	priority of the shared file number from 1(highest) to 65535(lowest)
	USHORT				m_uPriority;
//	autopriority (priotity calculated automaticaly)
	bool				m_bAutoPriority;

#ifdef JUMPSTART
//	jumpstart Db
	CJumpstarter* 	m_pJumpstarter;
#endif

//	list with sources the requested this file
	SourceList* 		m_pSourceList;


/*
	CMutex	m_mutexHashList;

	bool	LoadTagsFromFile(CFile& file);
	bool	LoadDateFromFile(CFile& file);
	void	LoadComment();
	
	CArray<CFileTag*,CFileTag*>	m_tagArray;
	CMap<uint16, uint16, byte, byte> m_partStatus;
	CString						m_strFilePath;
	CString						m_strComment;
	EnumPartFileRating	m_eRating;
*/

private:

/*
	uint32	m_dwBlockCount;
	uint16	m_nCompleteSourcesCount;
	uint16	m_nCompleteSourcesCountLo;
	uint16	m_nCompleteSourcesCountHi;
	uint32	m_nCompleteSourcesTime;
*/

};
////////////////////////////////////////////////////////////////////////////////////////
