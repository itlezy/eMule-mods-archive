#pragma once

#include "../EmEngine.h"
#include "../Other/TagStream.h"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct MD4PartKey {
	UCHAR	fileHash[16];
	USHORT	uPartNumber;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum DBMD4PartTags
{
	DBMD4PT_HASH = 1,
	DBMD4PT_SHARED_STATUS,
	DBMD4PT_TRANSFERRED,
	DBMD4PT_FULL_TRANSFER,
	DBMD4PT_PART_TRANSFER,
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CFileMD4Part
{
public:
	CFileMD4Part(UCHAR* pFileHash, USHORT uNumber);
	~CFileMD4Part();

//	hash
	UCHAR*	GetHash() const			{return m_pcPartHash;}
	BOOL	SetHash(UCHAR* pcHash);

	BOOL	GetSharedStatus()		{return m_bSharedPart;}
	void		SetSharedStatus(BOOL b)	{m_bSharedPart = b;}

	//statistical values
	void		CountTransferSession(BOOL bFullPartTransfer);
	USHORT	GetFullTrasfer()		{return m_uFullTrasfer;}
	USHORT	GetPartTrasfer()		{return m_uPartTrasfer;}

	void		AddTransferred(uint32 dwBytes);
	DWORD	GetTransferred()					{return m_dwTransfered;}

//	load & save the properties
	bool		Load();
	bool		Save();
//	tag processing
	void 	GetPartTagsFromStream(CTagStream* pTagStream);
	void 	PutPartTagsInStream(CTagStream* pTagStream);

protected:
//	pointer to hasht of the file
	UCHAR*		m_pcFileHash;
//	pointer to hasht of the part
	UCHAR*		m_pcPartHash;

//	the number of the part
	USHORT		m_uNumber;

//	is part shared or not
	BOOL		m_bSharedPart;

//	statistical variables
	DWORD		m_dwTransfered;

	USHORT		m_uFullTrasfer;
	USHORT		m_uPartTrasfer;

private:
	

};
