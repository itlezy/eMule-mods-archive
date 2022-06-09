#include "StdAfx.h"
#include "FileMD4Part.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CFileMD4Part::CFileMD4Part(UCHAR* pFileHash, USHORT uNumber)
{
	m_pcFileHash = pFileHash;
	m_uNumber = uNumber;
	m_pcPartHash = NULL;
	m_bSharedPart = FALSE;
	m_dwTransfered = 0;
	m_uFullTrasfer = 0;
	m_uPartTrasfer = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CFileMD4Part::~CFileMD4Part()
{
	if (m_pcPartHash)
		delete m_pcPartHash;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CFileMD4Part::SetHash(UCHAR* pcHash)
{
	//allocate hash if required
	if (m_pcPartHash == NULL)
	{
		m_pcPartHash = new uchar[16];
	}
//	check allocation
	if (m_pcPartHash == NULL)
		return FALSE;

//	copy hash
	memcpy(m_pcPartHash, pcHash, 16);
//	set part as shared
	m_bSharedPart = TRUE;
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFileMD4Part::CountTransferSession(BOOL bFullPartTransfer)
{
	if (bFullPartTransfer)
		m_uFullTrasfer++;
	else
		m_uPartTrasfer++;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFileMD4Part::AddTransferred(uint32 dwBytes)
{
	m_dwTransfered += dwBytes; 
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFileMD4Part::GetPartTagsFromStream(CTagStream* pTagStream)
{
	BYTE byteTemp;

	while (pTagStream->GetTagName() != 0)
	{
		switch(pTagStream->GetTagName())
		{
			case DBMD4PT_HASH:
				//allocate hash if required
				if (m_pcPartHash == NULL)
				{
					m_pcPartHash = new uchar[16];
				}
				pTagStream->GetTagValue(m_pcPartHash);
				break;

			case DBMD4PT_SHARED_STATUS:
				pTagStream->GetTagValue(&byteTemp);
				if (byteTemp != 0)
					m_bSharedPart = TRUE;
				else
					m_bSharedPart = FALSE;
				break;

			case DBMD4PT_TRANSFERRED:
				pTagStream->GetTagValue(&m_dwTransfered);
				break;

			case DBMD4PT_FULL_TRANSFER:
				pTagStream->GetTagValue(&m_uFullTrasfer);
				break;

			case DBMD4PT_PART_TRANSFER:
				pTagStream->GetTagValue(&m_uPartTrasfer);
				break;

			default:
				break;
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFileMD4Part::PutPartTagsInStream(CTagStream* pTagStream)
{
	BYTE byteTemp;

	if (m_pcPartHash)
	{
		pTagStream->PutTag(DBMD4PT_HASH, 16, m_pcPartHash);
	}

	if (m_bSharedPart)
		byteTemp = 0x5A;
	else
		byteTemp = 0x00;
	pTagStream->PutTag(DBMD4PT_SHARED_STATUS, sizeof(byteTemp), &byteTemp);
	pTagStream->PutTag(DBMD4PT_TRANSFERRED, sizeof(m_dwTransfered), &m_dwTransfered);
	pTagStream->PutTag(DBMD4PT_FULL_TRANSFER, sizeof(m_uFullTrasfer), &m_uFullTrasfer);
	pTagStream->PutTag(DBMD4PT_PART_TRANSFER, sizeof(m_uPartTrasfer), &m_uPartTrasfer);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CFileMD4Part::Load()
{
	Dbt key, data;
	MD4PartKey stKey;

	try
	{
		memset(&stKey, 0, sizeof(stKey));
		memcpy(stKey.fileHash, m_pcFileHash, 16);
		stKey.uPartNumber = m_uNumber;

		key.set_data(&stKey);
		key.set_size(sizeof(stKey));
		key.set_ulen(sizeof(stKey));
		key.set_flags(DB_DBT_USERMEM);

		// allow Db allocate the memory for tag stream
		data.set_data(NULL);
		data.set_size(0);
		data.set_ulen(0);
		data.set_flags(DB_DBT_MALLOC);

		int nRet = stEngine.m_pDbFiles->get(NULL, &key, &data, 0);
		if(nRet)
			return false;

		CTagStream* pTagStream = new CTagStream(data.get_data(), data.get_size());

		GetPartTagsFromStream(pTagStream);

		//the memory allocated by Db will be freed inside CTagStream
		safe_delete(pTagStream);
	}
	catch (DbRunRecoveryException &dbe)
	{
		TRACE(CString(dbe.what()));
		return false;
	}
	catch (DbException &dbe)
	{
		TRACE("Problems working with database objects: %s.\n", dbe.what());
		return false;
	}
	catch(...)
	{
		TRACE("Problems working with database objects.\n");
		return false;
	}

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CFileMD4Part::Save()
{
	Dbt key, data;
	MD4PartKey stKey;
	CTagStream* pTagStream;

	try
	{
		pTagStream = new CTagStream();

		PutPartTagsInStream(pTagStream);

		memset(&stKey, 0, sizeof(stKey));
		memcpy(stKey.fileHash, m_pcFileHash, 16);
		stKey.uPartNumber = m_uNumber;

		key.set_data(&stKey);
		key.set_size(sizeof(stKey));

		data.set_data(pTagStream->GetStream());
		data.set_size(pTagStream->GetStreamSize());

		int nRet = stEngine.m_pDbFiles->put(NULL, &key, &data, 0);
		safe_delete(pTagStream);
		if(nRet)
		{
			stEngine.m_pDbFiles->err(nRet, "CKnownFile::SaveTag");
			ASSERT(FALSE);
			return false;
		}
	}
	catch (DbRunRecoveryException &dbe)
	{
		TRACE(CString(dbe.what()));
		safe_delete(pTagStream);
		return false;
	}
	catch (DbException &dbe)
	{
		TRACE("Problems working with database objects: %s.\n", dbe.what());
		safe_delete(pTagStream);
		return false;
	}
	catch(...)
	{
		TRACE("Problems working with database objects.\n");
		safe_delete(pTagStream);
		return false;
	}
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

