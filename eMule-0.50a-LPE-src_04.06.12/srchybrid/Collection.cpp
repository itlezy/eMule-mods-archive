//this file is part of eMule
//Copyright (C)2002-2005 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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
#include "collection.h"
#include "KnownFile.h"
#include "CollectionFile.h"
#include "SafeFile.h"
#include "Packets.h"
#include "Preferences.h"
#include "SharedFilelist.h"
#include "emule.h"
#include "Log.h"
#include "md5sum.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define COLLECTION_FILE_VERSION1_INITIAL		0x01
#define COLLECTION_FILE_VERSION2_LARGEFILES		0x02

CCollection::CCollection(void)
: m_sCollectionName(_T(""))
, m_sCollectionAuthorName(_T(""))
, m_bTextFormat(false)
{
#ifdef REPLACE_ATLMAP
	CCollectionFilesMap(DEFAULT_FILES_TABLE_SIZE).swap(m_CollectionFilesMap);
#else
	m_CollectionFilesMap.InitHashTable(DEFAULT_FILES_TABLE_SIZE);
#endif
	m_sCollectionName.Format(_T("New Collection-%u"), ::GetTickCount());
	m_pabyCollectionAuthorKey = NULL;
	m_nKeySize = 0;
}

CCollection::CCollection(const CCollection* pCollection)
{
   //MORPH START  CB Mod : Fix : Collection double extension leuk_he
  CString collectionName = pCollection->m_sCollectionName;
  collectionName.Left(collectionName.ReverseFind(L'.')); // no extension should be handled correctly as well
  m_sCollectionName = collectionName;
  //MORPH END  CB Mod : Fix : Collection double extension leuk_he

	if (pCollection->m_pabyCollectionAuthorKey != NULL){
		m_nKeySize = pCollection->m_nKeySize;
		m_pabyCollectionAuthorKey = new BYTE[m_nKeySize];
		memcpy(m_pabyCollectionAuthorKey, pCollection->m_pabyCollectionAuthorKey, m_nKeySize);
		m_sCollectionAuthorName = pCollection->m_sCollectionAuthorName;
	}
	else{
		m_nKeySize = 0;
		m_pabyCollectionAuthorKey = NULL;
	}

	m_bTextFormat = pCollection->m_bTextFormat;

#ifdef REPLACE_ATLMAP
	CCollectionFilesMap(pCollection->m_CollectionFilesMap.bucket_count()).swap(m_CollectionFilesMap);
	for(CCollectionFilesMap::const_iterator it = pCollection->m_CollectionFilesMap.begin(); it != pCollection->m_CollectionFilesMap.end(); ++it)
		AddFileToCollection(it->second, true);
#else
	m_CollectionFilesMap.InitHashTable(pCollection->m_CollectionFilesMap.GetHashTableSize());
	POSITION pos = pCollection->m_CollectionFilesMap.GetStartPosition();
	CCollectionFile* pCollectionFile;
	CSKey key;
	while( pos != NULL )
	{
		pCollection->m_CollectionFilesMap.GetNextAssoc( pos, key, pCollectionFile );
		AddFileToCollection(pCollectionFile, true);
	}
#endif
}

CCollection::~CCollection(void)
{
	delete[] m_pabyCollectionAuthorKey;
#ifdef REPLACE_ATLMAP
	for(CCollectionFilesMap::const_iterator it = m_CollectionFilesMap.begin(); it != m_CollectionFilesMap.end(); ++it)
		delete it->second;
	m_CollectionFilesMap.clear();
#else
	POSITION pos = m_CollectionFilesMap.GetStartPosition();
	CCollectionFile* pCollectionFile;
	CSKey key;
	while( pos != NULL )
	{
		m_CollectionFilesMap.GetNextAssoc( pos, key, pCollectionFile );
	    delete pCollectionFile;
	}
	m_CollectionFilesMap.RemoveAll();
#endif
}

CCollectionFile* CCollection::AddFileToCollection(CAbstractFile* pAbstractFile, bool bCreateClone)
{
	CCollectionFile* pCollectionFile;
	CSKey key(pAbstractFile->GetFileHash());
#ifdef REPLACE_ATLMAP
	CCollectionFilesMap::const_iterator it = m_CollectionFilesMap.find(key);
	if(it != m_CollectionFilesMap.end()){
		ASSERT(0);
		return it->second;
	}
#else
	if (m_CollectionFilesMap.Lookup(key, pCollectionFile))
	{
		ASSERT(0);
		return pCollectionFile;
	}
#endif

	pCollectionFile = NULL;

	if(bCreateClone)
		pCollectionFile = new CCollectionFile(pAbstractFile);
	else if(IsCCollectionFile(pAbstractFile)/*pAbstractFile->IsKindOf(RUNTIME_CLASS(CCollectionFile))*/)
		pCollectionFile = (CCollectionFile*)pAbstractFile;

	if(pCollectionFile)
#ifdef REPLACE_ATLMAP
		m_CollectionFilesMap[key] = pCollectionFile;
#else
		m_CollectionFilesMap.SetAt(key, pCollectionFile);
#endif

	return pCollectionFile;
}

void CCollection::RemoveFileFromCollection(CAbstractFile* pAbstractFile)
{
	CSKey key(pAbstractFile->GetFileHash());
#ifdef REPLACE_ATLMAP
	CCollectionFilesMap::const_iterator it = m_CollectionFilesMap.find(key);
	if(it != m_CollectionFilesMap.end()){
		CCollectionFile* pCollectionFile = it->second;
		m_CollectionFilesMap.erase(it);
		delete pCollectionFile;
	}
#else
	CCollectionFile* pCollectionFile;
	if (m_CollectionFilesMap.Lookup(key, pCollectionFile))
	{
		m_CollectionFilesMap.RemoveKey(key);
		delete pCollectionFile;
	}
#endif
	else
		ASSERT(0);
}

void CCollection::SetCollectionAuthorKey(const byte* abyCollectionAuthorKey, uint32 nSize)
{
	delete[] m_pabyCollectionAuthorKey;
	m_pabyCollectionAuthorKey = NULL;
	m_nKeySize = 0;
	if (abyCollectionAuthorKey != NULL){
		m_pabyCollectionAuthorKey = new BYTE[nSize];
		memcpy(m_pabyCollectionAuthorKey, abyCollectionAuthorKey, nSize);
		m_nKeySize = nSize;
	}
}

bool CCollection::InitCollectionFromFile(const CString& sFilePath, CString sFileName)
{
	DEBUG_ONLY( sFileName.Replace(COLLECTION_FILEEXTENSION, _T("")) );

	bool bCollectionLoaded = false;

	CSafeFile data;
	if(data.Open(sFilePath, CFile::modeRead | CFile::shareDenyWrite | CFile::typeBinary))
	{
		try
		{
			uint32 nVersion = data.ReadUInt32();
			if(nVersion == COLLECTION_FILE_VERSION1_INITIAL || nVersion == COLLECTION_FILE_VERSION2_LARGEFILES)
			{
				uint32 headerTagCount = data.ReadUInt32();
				while(headerTagCount)
				{
					CTag tag(&data, true);
					switch(tag.GetNameID())
					{
						case FT_FILENAME:
						{
							if(tag.IsStr())
								m_sCollectionName = tag.GetStr();
							break;
						}
						case FT_COLLECTIONAUTHOR:
						{
							if(tag.IsStr())
								m_sCollectionAuthorName = tag.GetStr();
							break;
						}
						case FT_COLLECTIONAUTHORKEY:
						{
							if(tag.IsBlob())
							{
								SetCollectionAuthorKey(tag.GetBlob(), tag.GetBlobSize());
							}
							break;
						}
					}
					headerTagCount--;
				}
				uint32 fileCount = data.ReadUInt32();
				while(fileCount)
				{
					CCollectionFile* pCollectionFile = new CCollectionFile(&data);
					if(pCollectionFile)
						AddFileToCollection(pCollectionFile, false);
					fileCount--;
				}
				bCollectionLoaded = true;
			}
			if (m_pabyCollectionAuthorKey != NULL){
					bool bResult = false;
					if (data.GetLength() > data.GetPosition()){
						using namespace CryptoPP;

						uint32 nPos = (uint32)data.GetPosition();
						data.SeekToBegin();
						BYTE* pMessage = new BYTE[nPos];
						VERIFY( data.Read(pMessage, nPos) == nPos);

						StringSource ss_Pubkey(m_pabyCollectionAuthorKey, m_nKeySize, true, 0);
						RSASSA_PKCS1v15_SHA_Verifier pubkey(ss_Pubkey);
		
						int nSignLen = (int)(data.GetLength() - data.GetPosition());
						BYTE* pSignature = new BYTE[nSignLen ];
						VERIFY( data.Read(pSignature, nSignLen) == (UINT)nSignLen);

						bResult = pubkey.VerifyMessage(pMessage, nPos, pSignature, nSignLen);

						delete[] pMessage;
						delete[] pSignature;
					}	
					if (!bResult){
						DebugLogWarning(_T("Collection %s: Verifying of public key failed!"), m_sCollectionName);
						delete[] m_pabyCollectionAuthorKey;
						m_pabyCollectionAuthorKey = NULL;
						m_nKeySize = 0;
						m_sCollectionAuthorName.Empty();// X: [CI] - [Code Improvement]
					}
					else
						DebugLog(_T("Collection %s: Public key verified"), m_sCollectionName);
					
			}
			else
				m_sCollectionAuthorName.Empty();// X: [CI] - [Code Improvement]
			data.Close();
		}
		catch(CFileException* error)
		{
			error->Delete();
			return false;
		}
		catch(...)
		{
			ASSERT( false );
			data.Close();
			return false;
		}
	}
	else
		return false;

	if(!bCollectionLoaded)
	{
		bool bHasBOM = IsUnicodeFile(sFilePath); // Avi3k: fix coll utf8
		CStdioFile data;
		if(data.Open(sFilePath, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText))
		{
			try
			{
				if (bHasBOM) // Avi3k: fix coll utf8
					data.Seek(sizeof(WORD), CFile::begin);
				CString sLink;
				while(data.ReadString(sLink))
				{
					//Ignore all lines that start with #.
					//These lines can be used for future features..
					if(sLink.Find(_T('#')) != 0)
					{
						try
						{
							CCollectionFile* pCollectionFile = new CCollectionFile();
							if (pCollectionFile->InitFromLink(sLink))
								AddFileToCollection(pCollectionFile, false);
							else
								delete pCollectionFile;
						}
						catch(...)
						{
							ASSERT( false );
							data.Close();
							return false;
						}
					}
				}
				data.Close();
				m_sCollectionName = sFileName;
				bCollectionLoaded = true;
				m_bTextFormat = true;
			}
			catch(CFileException* error)
			{
				error->Delete();
				return false;
			}
			catch(...)
			{
				ASSERT( false );
				data.Close();
				return false;
			}
		}
	}

	return bCollectionLoaded;
}

void CCollection::WriteToFileAddShared(CryptoPP::RSASSA_PKCS1v15_SHA_Signer* pSignKey)
{
	using namespace CryptoPP;
	CString sFileName;
	sFileName.Format(_T("%s%s"), m_sCollectionName, COLLECTION_FILEEXTENSION);
	
	CString sFilePath;
	sFilePath.Format(_T("%s\\%s"), thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR), sFileName);

	if(m_bTextFormat)
	{
		CStdioFile data;
		if(data.Open(sFilePath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite | CFile::typeText))
		{
			try
			{
#ifdef REPLACE_ATLMAP
				for(CCollectionFilesMap::const_iterator it = m_CollectionFilesMap.begin(); it != m_CollectionFilesMap.end(); ++it)
				{
					CString sLink;
					sLink.Format(_T("%s\n"), it->second->GetED2kLink()); 
					data.WriteString(sLink);
				}
#else
				POSITION pos = m_CollectionFilesMap.GetStartPosition();
				CCollectionFile* pCollectionFile;
				CSKey key;
				while( pos != NULL )
				{
					m_CollectionFilesMap.GetNextAssoc( pos, key, pCollectionFile );
					CString sLink;
					sLink.Format(_T("%s\n"), pCollectionFile->GetED2kLink()); 
					data.WriteString(sLink);
				}
#endif
				data.Close();
			}
			catch(CFileException* error)
			{
				error->Delete();
				return;
			}
			catch(...)
			{
				ASSERT( false );
				data.Close();
				return;
			}
		}
	}
	else
	{
		CSafeFile data;
		if(data.Open(sFilePath, CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyWrite | CFile::typeBinary))
		{
			try
			{
				//Version
				// check first if we have any large files in the map - write use lowest version possible
				uint32 dwVersion = COLLECTION_FILE_VERSION1_INITIAL;
#ifdef REPLACE_ATLMAP
				for(CCollectionFilesMap::const_iterator it = m_CollectionFilesMap.begin(); it != m_CollectionFilesMap.end(); ++it)
				{
					if (it->second->IsLargeFile()){
						dwVersion = COLLECTION_FILE_VERSION2_LARGEFILES;
						break;
					}
				}
#else
				POSITION pos = m_CollectionFilesMap.GetStartPosition();
				CCollectionFile* pCollectionFile;
				CSKey key;
				while( pos != NULL ) {
					m_CollectionFilesMap.GetNextAssoc( pos, key, pCollectionFile );
					if (pCollectionFile->IsLargeFile()){
						dwVersion = COLLECTION_FILE_VERSION2_LARGEFILES;
						break;
					}
				}
#endif
				data.WriteUInt32(dwVersion);

				uint32 uTagCount = 1;

				//NumberHeaderTags
				if(m_pabyCollectionAuthorKey != NULL)
					uTagCount += 2;


				data.WriteUInt32(uTagCount);

				CTag collectionName(FT_FILENAME, m_sCollectionName);
				collectionName.WriteTagToFile(&data, utf8strRaw);

				if(m_pabyCollectionAuthorKey != NULL){
					CTag collectionAuthor(FT_COLLECTIONAUTHOR, m_sCollectionAuthorName);
					collectionAuthor.WriteTagToFile(&data, utf8strRaw);

					CTag collectionAuthorKey(FT_COLLECTIONAUTHORKEY, m_nKeySize, m_pabyCollectionAuthorKey);
					collectionAuthorKey.WriteTagToFile(&data, utf8strRaw);
				}

				//Total Files
#ifdef REPLACE_ATLMAP
				data.WriteUInt32(m_CollectionFilesMap.size());

				for(CCollectionFilesMap::const_iterator it = m_CollectionFilesMap.begin(); it != m_CollectionFilesMap.end(); ++it)
					it->second->WriteCollectionInfo(&data);
#else
				data.WriteUInt32((uint32)m_CollectionFilesMap.GetCount());

				pos = m_CollectionFilesMap.GetStartPosition();
				while( pos != NULL ) {
					m_CollectionFilesMap.GetNextAssoc( pos, key, pCollectionFile );
					pCollectionFile->WriteCollectionInfo(&data);
				}
#endif

				if (pSignKey != NULL){
					uint32 nPos = (uint32)data.GetPosition();
					data.SeekToBegin();
					BYTE* pBuffer = new BYTE[nPos];
					VERIFY( data.Read(pBuffer, nPos) == nPos);

					SecByteBlock sbbSignature(pSignKey->SignatureLength());
					AutoSeededRandomPool rng;
					pSignKey->SignMessage(rng, pBuffer ,nPos , sbbSignature.begin());
					BYTE abyBuffer2[500];
					ArraySink asink(abyBuffer2, 500);
					asink.Put(sbbSignature.begin(), sbbSignature.size());
					int nResult = (uint8)asink.TotalPutLength();
					data.Write(abyBuffer2, nResult);

					delete[] pBuffer;
				}
				data.Close();
			}
			catch(CFileException* error)
			{
				error->Delete();
				return;
			}
			catch(...)
			{
				ASSERT( false );
				data.Close();
				return;
			}
		}
	}

	theApp.sharedfiles->CheckAndAddSingleFile(sFilePath);
}

bool CCollection::HasCollectionExtention(const CString& sFileName)
{
	if(sFileName.Find(COLLECTION_FILEEXTENSION) == -1)
		return false;
	return true;
}

CString CCollection::GetCollectionAuthorKeyString(){
	if (m_pabyCollectionAuthorKey != NULL)
		return EncodeBase16(m_pabyCollectionAuthorKey, m_nKeySize);
	else
		return _T("");
}

CString	CCollection::GetAuthorKeyHashString(){
	if (m_pabyCollectionAuthorKey != NULL){
		MD5Sum md5(m_pabyCollectionAuthorKey, m_nKeySize);
		return md5.GetHash(); // X: [CI] - [Code Improvement]
	}
	return _T("");
}
