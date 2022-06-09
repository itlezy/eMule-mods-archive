#include "stdafx.h"
#include "SharedFile.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum _EnumTagTypes
{
	TAGTYPE_STRING	= 2,
	TAGTYPE_INT		= 3,
	TAGTYPE_FLOAT	= 4,
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CSharedFile::CSharedFile()
{
	m_MD4Parts.clear();
	m_pSourceList = NULL;
#ifdef JUMPSTART
	m_pJumpstarter = NULL;
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CSharedFile::~CSharedFile()
{
	CFileMD4SharedPart*		pMD4Part = NULL;

	while (!m_MD4Parts.empty())
	{
		pMD4Part  = m_MD4Parts.back();
		m_MD4Parts.pop_back();
		safe_delete(pMD4Part);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uchar* CSharedFile::GetPartHash(ULONG ulPartIdx)
{
	EMULE_TRY
	CFileMD4SharedPart*		pMD4Part = NULL;

	if (ulPartIdx < m_MD4Parts.size())
	{
		pMD4Part = m_MD4Parts.at(ulPartIdx);
		if (pMD4Part)
			return pMD4Part->GetHash();
	}

	EMULE_CATCH

	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CSharedFile::GetPartSharedStatus(ULONG ulPartIdx)
{
	EMULE_TRY
	CFileMD4SharedPart*		pMD4Part = NULL;

	if (ulPartIdx < m_MD4Parts.size())
	{
		pMD4Part = m_MD4Parts.at(ulPartIdx);
		if (pMD4Part)
			return pMD4Part->GetSharedStatus();
	}

	EMULE_CATCH

	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSharedFile::SetPartSharedStatus(ULONG ulPartIdx, BOOL bStatus)
{
	EMULE_TRY
	CFileMD4Part*		pMD4Part = NULL;

	if (ulPartIdx < m_MD4Parts.size())
	{
		pMD4Part = m_MD4Parts.at(ulPartIdx);
		if (pMD4Part && pMD4Part->GetHash() != NULL)
			pMD4Part->SetSharedStatus(bStatus);
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// check for shared parts
BOOL CSharedFile::HasSharedParts()
{
	EMULE_TRY

	if (!m_MD4Parts.empty())
	{
		PartVector::const_iterator PartsIter = m_MD4Parts.begin();

		while (PartsIter != m_MD4Parts.end())
		{
			CFileMD4SharedPart* pMD4Part = *PartsIter;
			if ( pMD4Part != NULL && pMD4Part->GetSharedStatus())
			{
				return FALSE;
			}
			PartsIter++;
		}
	}

	EMULE_CATCH

	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef JUMPSTART
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// obaldin
bool CSharedFile::IsJumpstartComplete()
{
     return CJumpstarter::IsJsCompleteForFile(this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSharedFile::SetJumpstartEnabled(bool bEnabled)
{
	if(!bEnabled && m_pJumpstarter)
	{
		m_pJumpstarter->Disable();
		delete m_pJumpstarter;
		m_pJumpstarter = NULL;
	}
	else if(bEnabled && !m_pJumpstarter)
	{
		CJumpstarter::EnableForFile(this);
		m_pJumpstarter = new CJumpstarter(this);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSharedFile::AddSentBlock(CUpDownClient* client, uint32 start_offset, uint32 togo)
{
	m_pJumpstarter->AddSentBlock(client, start_offset, togo);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSharedFile::WriteJumpstartPartStatus(CUpDownClient* client, CMemFile* data)
{
	m_pJumpstarter->WriteJumpstartPartStatus(client, data);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSharedFile::AllowChunkForClient(uint32 partNo, CUpDownClient* client)
{
	return (!m_pJumpstarter) || m_pJumpstarter->AllowChunkForClient(partNo, client);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSharedFile::Load()
{
	Dbt key, data;
	FileKey stKey;

	try
	{
		memset(&stKey, 0, sizeof(stKey));

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

		GetFileTagsFromStream(pTagStream);

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
bool CSharedFile::Save()
{
	FileKey stKey;
	Dbt key, data;
	CTagStream* pTagStream;

	try
	{
		pTagStream = new CTagStream();

		PutFileTagsInStream(pTagStream);

		memset(&stKey, 0, sizeof(stKey));
		stKey.dwFileSize = m_dwFileSize;
		memcpy(&stKey.fileHash, GetFileHash(), 16);

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
void CSharedFile::GetFileTagsFromStream(CTagStream* pTagStream)
{
	BYTE byteTemp;

	CKnownFile::GetFileTagsFromStream(pTagStream);
	//set read pointer to the begining
	pTagStream->MoveReadPointer(0);

	while (pTagStream->GetTagName() != 0)
	{
		switch(pTagStream->GetTagName())
		{
			case DBF_SHARED_STATUS:
				pTagStream->GetTagValue(&byteTemp);
				if (byteTemp != 0)
					m_bSharedFile = true;
				else
					m_bSharedFile = false;
				break;
#ifdef JUMPSTART
			case DBF_JUMPSTART:
				pTagStream->GetTagValue(&byteTemp);
				if (byteTemp != 0)
					SetJumpstartEnabled(true);
				break;
#endif
			case DBF_PRIORITY:
				pTagStream->GetTagValue(&m_uPriority);
				break;
			case DBF_AUTO_PRIORITY:
				pTagStream->GetTagValue(&byteTemp);
				if (byteTemp != 0)
					m_bAutoPriority= true;
				else
					m_bAutoPriority = false;
				break;
			case DBF_VIEW_PERMISSION:
				pTagStream->GetTagValue(&m_byteViewPermissions);
				break;
			case DBF_SOURCE_SHARE:
				pTagStream->GetTagValue(&byteTemp);
				if (byteTemp != 0)
					m_pSourceList = new SourceList;
				else
					m_bSharedFile = NULL;
				break;
			default:
				break;
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSharedFile::PutFileTagsInStream(CTagStream* pTagStream)
{
	BYTE byteTemp;

	CKnownFile::PutFileTagsInStream(pTagStream);

	if (m_bSharedFile)
		byteTemp = 0x5A;
	else
		byteTemp = 0;
	pTagStream->PutTag(DBF_SHARED_STATUS, sizeof(byteTemp), &byteTemp);
#ifdef JUMPSTART
	if (m_pJumpstarter)
		byteTemp = 0x5A;
	else
		byteTemp = 0;
	pTagStream->PutTag(DBF_JUMPSTART, sizeof(byteTemp), &byteTemp);
#endif
	pTagStream->PutTag(DBF_PRIORITY, sizeof(m_uPriority), &m_uPriority);
	if (m_bAutoPriority)
		byteTemp = 0x5A;
	else
		byteTemp = 0;
	pTagStream->PutTag(DBF_AUTO_PRIORITY, sizeof(byteTemp), &byteTemp);
	pTagStream->PutTag(DBF_VIEW_PERMISSION, sizeof(m_byteViewPermissions), &m_byteViewPermissions);
	if (m_pSourceList)
		byteTemp = 0x5A;
	else
		byteTemp = 0;
	pTagStream->PutTag(DBF_SOURCE_SHARE, sizeof(byteTemp), &byteTemp);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	create the filehashset by using member variables
bool CSharedFile::CreateHashsetFromFile(const CString &strDirectory,const CString &strFileName)
{
	EMULE_TRY

	CFile					pFile;
	uchar					tempHash[16];
	CString					strNameBuffer;

	SetPath(strDirectory);
	SetFileName(stEngine, strFileName);

	strNameBuffer.Format(_T("%s\\%s"), strDirectory, strFileName);
	SetFilePath(strNameBuffer);

//	If we failed to open the specified pFile, return false
	if ( !pFile.Open(strNameBuffer) )
		return false;

//	AddDebugLogLine(RGB_LOG_DIMMED + _T("File %s is being hashed"), strFileName/*m_strFileName*/);

	//set member variables
	SetFileSize(stEngine, pFile.GetSize());

//	after filesize(m_lFileSize) was defined we can get number of hashs
	const ULONG dwHashCount = GetED2KPartHashCount();

//	Create hashset
	if (dwHashCount > 1)
	{
	//	allocate memory for hashset
		uchar*	pPartsHashSet = new uchar[16*dwHashCount];
	//	allocate memory for hash verctor & fill the value with NULL
		m_MD4Parts.resize (dwHashCount, NULL);

		for (ULONG i = 0; i < dwHashCount; i++)
		{
		//	get hash over global pointer
			uchar* pNewPartHash = pPartsHashSet + 16*i;

		//	Create a hash for the next part
			if (i == dwHashCount -1)
			{
				_CreateHashFromFile(&pFile, (GetFileSize() - PARTSIZE*i), pNewPartHash);
			}
			else
			{
				_CreateHashFromFile(&pFile, PARTSIZE, pNewPartHash);
			}
		//	create a part & set hash
			m_MD4Parts[i] = new CFileMD4SharedPart(m_fileHash, static_cast<USHORT>(i));
			m_MD4Parts.at(i)->SetHash(pNewPartHash);
		}
		_CreateHashFromString(pPartsHashSet, dwHashCount*16, tempHash);

		delete pPartsHashSet;
	}
	else if (dwHashCount == 0)
	{
		_CreateHashFromFile(&pFile, GetFileSize(), tempHash);
	}
	SetFileHash(tempHash);
	pFile.Close();

	return true;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	LoadHashsetFromFile() loads the KnownFile with a hashset from the file 'file' in the format of "known.met".
//		If 'bCheckHash' is true, the file hash in 'file' must match a file hash calculated from the part hashes
//		'file'. false is returned on failure, true on success.
bool CSharedFile::ImportHashsetFromFile(CFile &file)
{
	EMULE_TRY

	uchar					tempHash[16];
	uchar* 					pPartsHashSet = NULL;
	uint16					uNumParts;

//	Read the file hashif required, or just skip it if we need just check a hash
	file.Read(&tempHash,16);			// <HASH> file hash
	SetFileHash(tempHash);

//	Read the number of parts
	file.Read(&uNumParts,2);		// <count:WORD> number of parts

	INT_PTR		iParts = static_cast<uint16>(uNumParts);


	//load hash array in to local buffer
	if (uNumParts == 0)
	{
		return true;	// load was successfully
	}
	else		// need to load hashset
	{
	//	allocate memory for hashset
		pPartsHashSet = new uchar[16*uNumParts];

	//	Read the part hashes.
		file.Read(pPartsHashSet, (16*uNumParts));		// <HASH>[count] part hashes

	//	recreate a file hash from hash set
		_CreateHashFromString(pPartsHashSet, uNumParts*16, tempHash);

	//	If the file hash from 'file' matches the calculated hash create parts,
	//	otherwise ignore hashset data
		if (memcmp(m_fileHash, tempHash, 16) == 0)
		{
			for (int i = 0; i < static_cast<int>(uNumParts); i++)
			{
				//	get hash over global pointer
				uchar*	pPartHash = pPartsHashSet + 16*i;

			//	create a part & set hash
				m_MD4Parts[i] = new CFileMD4SharedPart(m_fileHash, static_cast<USHORT>(i));
				m_MD4Parts.at(i)->SetHash(pPartHash);
			}
		}
	}

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	LoadTagsFromFile() parses what tags we know and stores the rest in 'm_tagArray' so they can be rewritten.
bool CSharedFile::ImportTagsFromFile(CFile &file)
{
	EMULE_TRY

	uint32		dwNumTags;
	BYTE		byteTagType, byteTagName;
	CString		m_strTagName;
	USHORT		uLength;
	CString 	strValue;
	int			iValue;
	float		fltValue;

	file.Read(&dwNumTags,4);							// {<TAGCOUNT:DWORD>

	for (uint32 j = 0; j != dwNumTags; j++)
	{
	//	Read the tag identifier; Either a special tag code or a tag name
		file.Read(&byteTagType,1);
		file.Read(&uLength,2);
		if (uLength == 1)
		{
			file.Read(&byteTagName,1);
		}
		else
		{
			file.Read(m_strTagName.GetBuffer(uLength+1),uLength);
			m_strTagName.ReleaseBuffer(uLength);
		}

	//	Read the tag data value
		if (byteTagType == TAGTYPE_STRING)
		{
			file.Read(&uLength,2);		
			file.Read(strValue.GetBuffer(uLength+1),uLength);
			strValue.ReleaseBuffer(uLength);
		}
		else if (byteTagType == TAGTYPE_INT)
		{
			file.Read(&iValue,4);
		}
		else if (byteTagType == TAGTYPE_FLOAT) // (used by Hybrid 0.48)
		{
			file.Read(&fltValue,4);
		}

		switch (byteTagName)
		{
			case FT_FILENAME:							// (FT_FILENAME:string) file name
				SetFileName(stEngine, strValue);
				break;
			case FT_FILESIZE:							// (FT_FILESIZE:int) file size (bytes)
				SetFileSize(stEngine, iValue);
				break;
			case FT_ATTRANSFERRED:						// (FT_ATTRANSFERRED:int) all time transferred (low long)
				SetCumulativeTransferred(iValue);
				break;
			case FT_ATTRANSFERREDHI:					// (FT_ATTRANSFERREDHI:int) all time transferred (high long)
			{
				uint64	qwActual, qwHigh = 0;
				
				qwHigh = iValue;
				qwHigh <<= 32;
				qwActual = GetCumulativeTransferred();
				qwActual &= qwHigh;
				SetCumulativeTransferred(qwActual);
				break;
			}
			case FT_ATREQUESTED:						// (FT_ATREQUESTED:int) all time requested
				SetCumulativeRequests(iValue);
				break;
 			case FT_ATACCEPTED:							// (FT_ATACCEPTED:int) all time accepted
 				SetCumulativeAccepts(iValue);
				break;
			case FT_PERMISSIONS:						// (FT_PERMISSIONS:int) view permissions
			{
				m_byteViewPermissions = static_cast<UCHAR>(iValue);
				break;
			}
			case FT_PRIORITY:							// (FT_PRIORITY:int) upload priority
			default:
				break;
		}
	}
	return true;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSharedFile::ImportFromFile(CFile& file)
{
	EMULE_TRY
	
	bool	bResult = true;

// <DATE:time_t> last write date/time
	file.Read(&m_timetLastWriteDate,4);
// <FILEHASH> & <HASHSET>
	bResult &= ImportHashsetFromFile(stEngine, file);
// <Tag_set>
	bResult &= ImportTagsFromFile(stEngine, file);

#ifdef JUMPSTART
	if (result && CJumpstarter::ShouldBeEnabledForFile(this))
	{
		m_pJumpstarter = new CJumpstarter(this);
		AddLogLine(false, IDS_JS_ENABLED, m_strFileName);
	}
#endif

	return bResult;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
