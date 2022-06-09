#pragma once
////////////////////////////////////////////////////////////////////////////////////////
// describes properties of the file-object that does not have data part (i.e. was downloaded & deleted)
// 
////////////////////////////////////////////////////////////////////////////////////////
/*
		  CKnownFile - CSharedFile - CPartFile
		/
CAbstractFile
		\
		  CSearchFile

*/
////////////////////////////////////////////////////////////////////////////////////////
#include "../EmEngine.h"
#include "AbstractFile.h"
#include "../Other/TagStream.h"
////////////////////////////////////////////////////////////////////////////////////////
// The keys include basic properties of file in ed2k-network (exclude filename)
struct FileKey {
	UCHAR	fileHash[16];
	DWORD	dwFileSize;
};
////////////////////////////////////////////////////////////////////////////////////////
enum FileDBTags
{
	//abstract file properties
	DBF_FILESIZE = 1,
	DBF_FILENAME,
	//known file properties
	DBT_LAST_WRITE_DATE,
	DBF_REQUESTED,
	DBF_ACCEPTED,
	DBF_TRANSFERRED,
	//shared file properties
	DBF_SHARED_STATUS,
	DBF_JUMPSTART,
	DBF_PRIORITY,
	DBF_AUTO_PRIORITY,
	DBF_VIEW_PERMISSION,
	DBF_SOURCE_SHARE
};
////////////////////////////////////////////////////////////////////////////////////////
class CUpDownClient;

class CKnownFile : public CAbstractFile
{
public:
	CKnownFile();
	~CKnownFile();

//	time properies
	uint32	GetFileDate()						{return m_timetLastWriteDate;}

//	statistical values
	void		AddRequest();
	uint32	GetSessionRequests()			{return m_dwSessionRequested;}
	uint32	GetCumulativeRequests()			{return m_dwSessionRequested;}
	void		SetCumulativeRequests(uint32 dwV)	{m_dwSessionRequested  = dwV;}
	
	void		AddAccepted();
	uint32	GetSessionAccepts()				{return m_dwSessionAccepted;}
	uint32	GetCumulativeAccepts()			{return m_dwCumulativeRequested;}
	void		SetCumulativeAccepts(uint32 dwV)		{m_dwCumulativeRequested = dwV;}

	void		AddTransferred(uint32 dwBytes);
	uint64	GetSessionTransferred()			{return m_qwSessionTransferred;}
	uint64	GetCumulativeTransferred()		{return m_qwCumulativeTransferred;}
	void		SetCumulativeTransferred(uint64 qwV)		{m_qwCumulativeTransferred = qwV;}

	void	SetPath(const CString& path) 	{ m_strKnownFileDirectory = path; }
	CString	GetPath() const					{return m_strKnownFileDirectory;}

	const CString& GetFilePath() const { return m_strFilePath; }
	void	SetFilePath(LPCTSTR pszFilePath);

	bool	LoadHashsetFromFile(CFile& file, bool checkhash);
	bool	LoadDateFromFile(CFile& file);

	void	CreateHashFromFile(CFile *file, uint32 dwLength, uchar *pbyteHash)	{CreateHashFromInput(file, dwLength, pbyteHash, 0);}

	void	CreateHashFromInput(CFile *file, uint32 dwLength, uchar *pbyteHash, uchar *pbyteMem);


//	Index callbacks
	//static int FileHashIndex(Db *pDb, const Dbt *pKey, const Dbt *pData, Dbt *pNewKey);

//	load & save the file properties
	virtual bool		Load();
	virtual bool		Save();

	virtual void 	GetFileTagsFromStream(CTagStream* pTagStream);
	virtual void 	PutFileTagsInStream(CTagStream* pTagStream);

protected:
	uint32	m_timetLastWriteDate;
	CString	m_strKnownFileDirectory;
	CString	m_strFilePath;

	uint32	lastseencomplete;

	vector<CFileTag*>	m_tagArray;
	vector<uchar*>		m_partHashArray;
	
private:
//	statistical variables
	uint32	m_dwSessionRequested;
	uint32	m_dwCumulativeRequested;

	uint32	m_dwSessionAccepted;
	uint32	m_dwCumulativeAccepted;

	uint64	m_qwSessionTransferred;
	uint64	m_qwCumulativeTransferred;
};
