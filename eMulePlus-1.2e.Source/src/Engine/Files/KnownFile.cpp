// parts of this file are based on work from pan One (http://home-3.tiscali.nl/~meost/pms/)

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

#include "StdAfx.h"
#include "../../packets.h"
#include "KnownFile.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CKnownFile::CKnownFile()
{
	m_timetLastWriteDate = 0;

	m_dwSessionRequested = 0;
	m_dwCumulativeRequested = 0;

	m_dwSessionAccepted = 0;
	m_dwCumulativeAccepted = 0;

	m_qwSessionTransferred = 0;
	m_qwCumulativeTransferred = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CKnownFile::~CKnownFile()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CKnownFile::AddRequest()
{
	m_dwSessionRequested++;
	m_dwCumulativeRequested++;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CKnownFile::AddAccepted()
{
	m_dwSessionAccepted++;
	m_dwCumulativeAccepted++;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CKnownFile::AddTransferred(uint32 dwBytes)
{
	m_qwSessionTransferred += dwBytes;
	m_qwCumulativeTransferred += dwBytes;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
int CKnownFile::FileHashIndex(Db *pDb, const Dbt *pKey, const Dbt *pData, Dbt *pNewKey)
{
	FileKey* pFileKey = (FileKey*)(pKey->get_data());
	FileKey* pTempKey = new FileKey;

	memset(pTempKey, 0, sizeof(FileKey));
	memcpy(pTempKey->fileHash, pFileKey->fileHash, 16);
	
	*pNewKey = Dbt();
	pNewKey->set_data(pTempKey);
	pNewKey->set_size(sizeof(FileKey));
	pNewKey->set_flags(DB_DBT_APPMALLOC);
	return 0;
}
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CKnownFile::Load()
{
/*	Dbt key, data;

	try
	{
		FileKey stKey;
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
*/
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CKnownFile::Save()
{
/*	FileKey stKey;
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
	}*/
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CKnownFile::GetFileTagsFromStream(CTagStream* pTagStream)
{
	CString strFileName;

	while (pTagStream->GetTagName() != 0)
	{
		switch(pTagStream->GetTagName())
		{
			case DBF_FILENAME:
				pTagStream->GetTagValue(strFileName.GetBuffer(pTagStream->GetTagSize()));
				break;
			case DBT_LAST_WRITE_DATE:
				pTagStream->GetTagValue(&m_timetLastWriteDate);
				break;
			case DBF_REQUESTED:
				pTagStream->GetTagValue(&m_dwCumulativeRequested);
				break;
			case DBF_ACCEPTED:
				pTagStream->GetTagValue(&m_dwCumulativeAccepted);
				break;
			case DBF_TRANSFERRED:
				pTagStream->GetTagValue(&m_qwCumulativeTransferred);
				break;
			default:
				break;
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CKnownFile::PutFileTagsInStream(CTagStream* pTagStream)
{
	pTagStream->PutTag(DBF_FILENAME, (GetFileName().GetLength()*sizeof(TCHAR)), GetFileName().GetBuffer(0));
	pTagStream->PutTag(DBT_LAST_WRITE_DATE, sizeof(m_timetLastWriteDate), &m_timetLastWriteDate);
	pTagStream->PutTag(DBF_REQUESTED, sizeof(m_dwCumulativeRequested), &m_dwCumulativeRequested);
	pTagStream->PutTag(DBF_ACCEPTED, sizeof(m_dwCumulativeAccepted), &m_dwCumulativeAccepted);
	pTagStream->PutTag(DBF_TRANSFERRED, sizeof(m_qwCumulativeTransferred), &m_qwCumulativeTransferred);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
