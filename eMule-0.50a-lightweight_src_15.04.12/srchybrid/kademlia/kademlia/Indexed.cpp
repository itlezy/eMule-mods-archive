/*
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// Note To Mods //
/*
Please do not change anything here and release it..
There is going to be a new forum created just for the Kademlia side of the client..
If you feel there is an error or a way to improve something, please
post it in the forum first and let us look at it.. If it is a real improvement,
it will be added to the official client.. Changing something without knowing
what all it does can cause great harm to the network if released in mass form..
Any mod that changes anything within the Kademlia side will not be allowed to advertise
there client on the eMule forum..
*/
#include "stdafx.h"
#include "./Indexed.h"
#include "./Kademlia.h"
#include "./Entry.h"
#include "./Prefs.h"
#include "../net/KademliaUDPListener.h"
#include "../utils/MiscUtils.h"
#include "../io/BufferedFileIO.h"
#include "../io/IOException.h"
#include "../io/ByteIO.h"
#include "../../Preferences.h"
#include "../../Log.h"
#include "../utils/KadUDPKey.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace Kademlia;

void DebugSend(LPCTSTR pszMsg, uint32 uIP, uint16 uPort);

CString CIndexed::m_sKeyFileName;
CString CIndexed::m_sSourceFileName;
CString CIndexed::m_sLoadFileName;

CIndexed::CIndexed()
#ifdef REPLACE_ATLMAP
: m_mapKeyword(DEFAULT_FILES_TABLE_SIZE)/*, m_mapNotes(DEFAULT_FILES_TABLE_SIZE)*/, m_mapLoad(DEFAULT_FILES_TABLE_SIZE), m_mapSources(DEFAULT_FILES_TABLE_SIZE)
#endif
{
#ifndef REPLACE_ATLMAP
	m_mapKeyword.InitHashTable(DEFAULT_FILES_TABLE_SIZE);
	//m_mapNotes.InitHashTable(DEFAULT_FILES_TABLE_SIZE);
	m_mapLoad.InitHashTable(DEFAULT_FILES_TABLE_SIZE);
	m_mapSources.InitHashTable(DEFAULT_FILES_TABLE_SIZE);
#endif
	m_sSourceFileName = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("src_index.dat");
	m_sKeyFileName = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("key_index.dat");
	m_sLoadFileName = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("load_index.dat");
	m_tLastClean = time(NULL) + (60*30);
	m_uTotalIndexSource = 0;
	m_uTotalIndexKeyword = 0;
	//m_uTotalIndexNotes = 0;
	m_uTotalIndexLoad = 0;
	m_bAbortLoading = false;
	m_bDataLoaded = false;
	ReadFile();
}

void CIndexed::ReadFile(void)
{
	m_bAbortLoading = false;
	CLoadDataThread* pLoadDataThread = new CLoadDataThread(this);
	pLoadDataThread->start();
}

CIndexed::~CIndexed()
{ 
	if (!m_bDataLoaded){
		// the user clicked on disconnect/close just after he started kad (on probably just before posting in the forum the emule doenst works :P )
		// while the loading thread is still busy. First tell the thread to abort its loading, afterwards wait for it to terminate
		// and then delete all loaded items without writing them to the files (as they are incomplete and unchanged)
		DebugLogWarning(_T("Kad stopping while still loading CIndexed data, waiting for abort"));
		m_bAbortLoading = true;
		Poco::FastMutex::SingleLock sLock(m_mutSync, true);
		ASSERT( m_bDataLoaded );

		// cleanup without storing
#ifdef REPLACE_ATLMAP
		for (CSrcHashMap::const_iterator it = m_mapSources.begin(); it != m_mapSources.end(); ++it)
		{
			SrcHash* pCurrSrcHash = it->second;
#else
		POSITION pos1 = m_mapSources.GetStartPosition();
		while( pos1 != NULL )
		{
			CCKey key1;
			SrcHash* pCurrSrcHash;
			m_mapSources.GetNextAssoc( pos1, key1, pCurrSrcHash );
#endif
			CKadSourcePtrList& keyHashSrcMap = pCurrSrcHash->ptrlistSource;
			POSITION pos2 = keyHashSrcMap.GetHeadPosition();
			while( pos2 != NULL )
			{
				Source* pCurrSource = keyHashSrcMap.GetNext(pos2);
				CKadEntryPtrList& srcEntryList = pCurrSource->ptrlEntryList;
				for(POSITION pos3 = srcEntryList.GetHeadPosition(); pos3 != NULL; )
				{
					CEntry* pCurrName = srcEntryList.GetNext(pos3);
					delete pCurrName;
				}
				delete pCurrSource;
			}
			delete pCurrSrcHash;
		}

#ifdef REPLACE_ATLMAP
		for (CKeyHashMap::const_iterator it1 = m_mapKeyword.begin(); it1 != m_mapKeyword.end(); ++it1)
		{
			KeyHash* pCurrKeyHash = it1->second;
			CSourceKeyMap& keySrcKeyMap = pCurrKeyHash->mapSource;
			for (CSourceKeyMap::const_iterator it2 = keySrcKeyMap.begin(); it2 != keySrcKeyMap.end(); ++it2)
			{
				Source* pCurrSource = it2->second;
#else
		pos1 = m_mapKeyword.GetStartPosition();
		while( pos1 != NULL )
		{
			CCKey key1;
			KeyHash* pCurrKeyHash;
			m_mapKeyword.GetNextAssoc( pos1, key1, pCurrKeyHash );
			CSourceKeyMap& keySrcKeyMap = pCurrKeyHash->mapSource;
			POSITION pos2 = keySrcKeyMap.GetStartPosition();
			while( pos2 != NULL )
			{
				Source* pCurrSource;
				CCKey key2;
				keySrcKeyMap.GetNextAssoc( pos2, key2, pCurrSource );
#endif
				CKadEntryPtrList& srcEntryList = pCurrSource->ptrlEntryList;
				for(POSITION pos3 = srcEntryList.GetHeadPosition(); pos3 != NULL; )
				{
					CKeyEntry* pCurrName = (CKeyEntry*)srcEntryList.GetNext(pos3);
					ASSERT( pCurrName->IsKeyEntry() );
					pCurrName->DirtyDeletePublishData();
					delete pCurrName;
				}
				delete pCurrSource;
			}
			delete pCurrKeyHash;
		}
		CKeyEntry::ResetGlobalTrackingMap();
	}
	else {
		// standart cleanup with sotring
		try
		{
			uint_ptr uTotalSource = 0;
			uint_ptr uTotalKey = 0;
			uint_ptr uTotalLoad = 0;
			bool I64Time = thePrefs.m_bEnable64BitTime;// X: [E64T] - [Enable64BitTime]
			CBufferedFileIO fileLoad;
			if(fileLoad.Open(m_sLoadFileName, CFile::modeWrite | CFile::modeCreate | CFile::typeBinary | CFile::shareDenyWrite))
			{
				setvbuf(fileLoad.m_pStream, NULL, _IOFBF, 32768);
				if(I64Time){// X: [E64T] - [Enable64BitTime]
					fileLoad.WriteUInt32(LOADINDEX_VERSION|I64TIMEMASK);//uVersion// X: [CI] - [Code Improvement]
					fileLoad.WriteUInt64(time(NULL));
				}
				else{
					fileLoad.WriteUInt32(LOADINDEX_VERSION);//uVersion// X: [CI] - [Code Improvement]
					fileLoad.WriteUInt32((uint32)time(NULL));
				}
#ifdef REPLACE_ATLMAP
				fileLoad.WriteUInt32((uint32) m_mapLoad.size());
				for (CLoadMap::const_iterator it = m_mapLoad.begin(); it != m_mapLoad.end(); ++it)
				{
					Load* pLoad = it->second;
#else
				fileLoad.WriteUInt32((uint32) m_mapLoad.GetCount());
				POSITION pos1 = m_mapLoad.GetStartPosition();
				while( pos1 != NULL )
				{
					Load* pLoad;
					CCKey key1;
					m_mapLoad.GetNextAssoc( pos1, key1, pLoad );
#endif
					fileLoad.WriteUInt128(pLoad->uKeyID);
					I64Time?fileLoad.WriteUInt64(pLoad->uTime):fileLoad.WriteUInt32((uint32)pLoad->uTime);// X: [E64T] - [Enable64BitTime]
					uTotalLoad++;
					delete pLoad;
				}
				fileLoad.Close();
			}
			else
				DebugLogError(_T("Unable to store Kad file: %s"), m_sLoadFileName);

			CBufferedFileIO fileSource;
			if (fileSource.Open(m_sSourceFileName, CFile::modeWrite | CFile::modeCreate | CFile::typeBinary | CFile::shareDenyWrite))
			{
				setvbuf(fileSource.m_pStream, NULL, _IOFBF, 32768);
				if(I64Time){// X: [E64T] - [Enable64BitTime]
					fileSource.WriteUInt32(SRCINDEX_VERSION|I64TIMEMASK);//uVersion// X: [CI] - [Code Improvement]
					fileSource.WriteUInt64(time(NULL)+KADEMLIAREPUBLISHTIMES);
				}
				else{
					fileSource.WriteUInt32(SRCINDEX_VERSION);//uVersion// X: [CI] - [Code Improvement]
					fileSource.WriteUInt32((uint32)(time(NULL)+KADEMLIAREPUBLISHTIMES));
				}
#ifdef REPLACE_ATLMAP
				fileSource.WriteUInt32((uint32) m_mapSources.size());
				for (CSrcHashMap::const_iterator it = m_mapSources.begin(); it != m_mapSources.end(); ++it)
				{
					SrcHash* pCurrSrcHash = it->second;
#else
				fileSource.WriteUInt32((uint32) m_mapSources.GetCount());
				POSITION pos1 = m_mapSources.GetStartPosition();
				while( pos1 != NULL )
				{
					CCKey key1;
					SrcHash* pCurrSrcHash;
					m_mapSources.GetNextAssoc( pos1, key1, pCurrSrcHash );
#endif
					fileSource.WriteUInt128(pCurrSrcHash->uKeyID);
					CKadSourcePtrList& keyHashSrcMap = pCurrSrcHash->ptrlistSource;
					fileSource.WriteUInt32((uint32) keyHashSrcMap.GetCount());
					POSITION pos2 = keyHashSrcMap.GetHeadPosition();
					while( pos2 != NULL )
					{
						Source* pCurrSource = keyHashSrcMap.GetNext(pos2);
						fileSource.WriteUInt128(pCurrSource->uSourceID);
						CKadEntryPtrList& srcEntryList = pCurrSource->ptrlEntryList;
						fileSource.WriteUInt32((uint32) srcEntryList.GetCount());
						for(POSITION pos3 = srcEntryList.GetHeadPosition(); pos3 != NULL; )
						{
							CEntry* pCurrName = srcEntryList.GetNext(pos3);
							I64Time?fileSource.WriteUInt64(pCurrName->m_tLifetime):fileSource.WriteUInt32((uint32)pCurrName->m_tLifetime);// X: [E64T] - [Enable64BitTime]
							pCurrName->WriteTagList(&fileSource);
							delete pCurrName;
							uTotalSource++;
						}
						delete pCurrSource;
					}
					delete pCurrSrcHash;
				}
				fileSource.Close();
			}
			else
				DebugLogError(_T("Unable to store Kad file: %s"), m_sSourceFileName);

			CBufferedFileIO fileKey;
			if (fileKey.Open(m_sKeyFileName, CFile::modeWrite | CFile::modeCreate | CFile::typeBinary | CFile::shareDenyWrite))
			{
				setvbuf(fileKey.m_pStream, NULL, _IOFBF, 32768);
				if(I64Time){// X: [E64T] - [Enable64BitTime]
					fileKey.WriteUInt32(KEYINDEX_VERSION|I64TIMEMASK);//uVersion
					fileKey.WriteUInt64(time(NULL)+KADEMLIAREPUBLISHTIMEK);
				}
				else{
					fileKey.WriteUInt32(KEYINDEX_VERSION);//uVersion
					fileKey.WriteUInt32((uint32)(time(NULL)+KADEMLIAREPUBLISHTIMEK));
				}
				fileKey.WriteUInt128(Kademlia::CKademlia::GetPrefs()->GetKadID());
#ifdef REPLACE_ATLMAP
				fileKey.WriteUInt32((uint32) m_mapKeyword.size());
				for (CKeyHashMap::const_iterator it1 = m_mapKeyword.begin(); it1 != m_mapKeyword.end(); ++it1)
				{
					KeyHash* pCurrKeyHash = it1->second;
					fileKey.WriteUInt128(pCurrKeyHash->uKeyID);
					CSourceKeyMap& keySrcKeyMap = pCurrKeyHash->mapSource;
					fileKey.WriteUInt32((uint32) keySrcKeyMap.size());
					for (CSourceKeyMap::const_iterator it2 = keySrcKeyMap.begin(); it2 != keySrcKeyMap.end(); ++it2)
					{
						Source* pCurrSource = it2->second;
#else
				fileKey.WriteUInt32((uint32) m_mapKeyword.GetCount());
				POSITION pos1 = m_mapKeyword.GetStartPosition();
				while( pos1 != NULL )
				{
					CCKey key1;
					KeyHash* pCurrKeyHash;
					m_mapKeyword.GetNextAssoc( pos1, key1, pCurrKeyHash );
					fileKey.WriteUInt128(pCurrKeyHash->uKeyID);
					CSourceKeyMap& keySrcKeyMap = pCurrKeyHash->mapSource;
					fileKey.WriteUInt32((uint32) keySrcKeyMap.GetCount());
					POSITION pos2 = keySrcKeyMap.GetStartPosition();
					while( pos2 != NULL )
					{
						Source* pCurrSource;
						CCKey key2;
						keySrcKeyMap.GetNextAssoc( pos2, key2, pCurrSource );
#endif
						fileKey.WriteUInt128(pCurrSource->uSourceID);
						CKadEntryPtrList& srcEntryList = pCurrSource->ptrlEntryList;
						fileKey.WriteUInt32((uint32) srcEntryList.GetCount());
						for(POSITION pos3 = srcEntryList.GetHeadPosition(); pos3 != NULL; )
						{
							CKeyEntry* pCurrName = (CKeyEntry*)srcEntryList.GetNext(pos3);
							ASSERT( pCurrName->IsKeyEntry() );
							I64Time?fileKey.WriteUInt64(pCurrName->m_tLifetime):fileKey.WriteUInt32((uint32)pCurrName->m_tLifetime);// X: [E64T] - [Enable64BitTime]
							pCurrName->WritePublishTrackingDataToFile(&fileKey);
							pCurrName->WriteTagList(&fileKey);
							pCurrName->DirtyDeletePublishData();
							delete pCurrName;
							uTotalKey++;
						}
						delete pCurrSource;
					}
					delete pCurrKeyHash;
				}
				CKeyEntry::ResetGlobalTrackingMap();
				fileKey.Close();
			}
			else
				DebugLogError(_T("Unable to store Kad file: %s"), m_sKeyFileName);

			AddDebugLogLine( false, _T("Wrote %u source, %u keyword, and %u load entries"), uTotalSource, uTotalKey, uTotalLoad);
		}
		catch ( CIOException *ioe )
		{
			AddDebugLogLine( false, _T("Exception in CIndexed::~CIndexed (IO error(%i))"), ioe->m_iCause);
			ioe->Delete();
		}
		catch (...)
		{
			AddDebugLogLine(false, _T("Exception in CIndexed::~CIndexed"));
		}
	}
/*
	// leftover cleanup (same for both variants)
#ifdef REPLACE_ATLMAP
	for (CSrcHashMap::const_iterator it = m_mapNotes.begin(); it != m_mapNotes.end(); ++it)
	{
		SrcHash* pCurrNoteHash = it->second;
#else
	POSITION pos1 = m_mapNotes.GetStartPosition();
	while( pos1 != NULL )
	{
		CCKey key1;
		SrcHash* pCurrNoteHash;
		m_mapNotes.GetNextAssoc( pos1, key1, pCurrNoteHash );
#endif
		CKadSourcePtrList& keyHashNoteMap = pCurrNoteHash->ptrlistSource;
		POSITION pos2 = keyHashNoteMap.GetHeadPosition();
		while( pos2 != NULL )
		{
			Source* pCurrNote = keyHashNoteMap.GetNext(pos2);
			CKadEntryPtrList& noteEntryList = pCurrNote->ptrlEntryList;
			for(POSITION pos3 = noteEntryList.GetHeadPosition(); pos3 != NULL; )
			{
				delete noteEntryList.GetNext(pos3);
			}
			delete pCurrNote;
		}
		delete pCurrNoteHash;
	}*/
}

void CIndexed::Clean(void)
{
	try
	{
		if( m_tLastClean > time(NULL) )
			return;

		uint_ptr uRemovedKey = 0;
		uint_ptr uRemovedSource = 0;
		uint_ptr uTotalSource = 0;
		uint_ptr uTotalKey = 0;
		time_t tNow = time(NULL);

		{
#ifdef REPLACE_ATLMAP
			for (CKeyHashMap::const_iterator it1 = m_mapKeyword.begin(); it1 != m_mapKeyword.end();)
			{
				KeyHash* pCurrKeyHash = it1->second;
				CSourceKeyMap& keySrcKeyMap = pCurrKeyHash->mapSource;
				for (CSourceKeyMap::const_iterator it2 = keySrcKeyMap.begin(); it2 != keySrcKeyMap.end();)
				{
					Source* pCurrSource = it2->second;
#else
			POSITION pos1 = m_mapKeyword.GetStartPosition();
			while( pos1 != NULL )
			{
				CCKey key1;
				KeyHash* pCurrKeyHash;
				m_mapKeyword.GetNextAssoc( pos1, key1, pCurrKeyHash );
				CSourceKeyMap& keySrcKeyMap = pCurrKeyHash->mapSource;
				POSITION pos2 = keySrcKeyMap.GetStartPosition();
				while( pos2 != NULL )
				{
					CCKey key2;
					Source* pCurrSource;
					keySrcKeyMap.GetNextAssoc( pos2, key2, pCurrSource );
#endif
					CKadEntryPtrList& srcEntryList = pCurrSource->ptrlEntryList;
					for(POSITION pos3 = srcEntryList.GetHeadPosition(); pos3 != NULL; )
					{
						POSITION pos4 = pos3;
						CKeyEntry* pCurrName = (CKeyEntry*)srcEntryList.GetNext(pos3);
						ASSERT( pCurrName->IsKeyEntry() );
						uTotalKey++;
						if( !pCurrName->m_bSource && pCurrName->m_tLifetime < tNow)
						{
							uRemovedKey++;
							srcEntryList.RemoveAt(pos4);
							delete pCurrName;
						}
						else if (pCurrName->m_bSource)
							ASSERT( false );
						else
							pCurrName->CleanUpTrackedPublishers(); // intern cleanup
					}
#ifdef REPLACE_ATLMAP
					if( srcEntryList.IsEmpty())
					{
						it2 = keySrcKeyMap.erase(it2);
						delete pCurrSource;
					}
					else
						++it2;
#else
					if( srcEntryList.IsEmpty())
					{
						keySrcKeyMap.RemoveKey(key2);
						delete pCurrSource;
					}
#endif
				}
#ifdef REPLACE_ATLMAP
				if( keySrcKeyMap.empty())
				{
					it1 = m_mapKeyword.erase(it1);
					delete pCurrKeyHash;
				}
				else
					++it1;
#else
				if( keySrcKeyMap.IsEmpty())
				{
					m_mapKeyword.RemoveKey(key1);
					delete pCurrKeyHash;
				}
#endif
			}
		}
		{
#ifdef REPLACE_ATLMAP
			for (CSrcHashMap::const_iterator it = m_mapSources.begin(); it != m_mapSources.end();)
			{
				SrcHash* pCurrSrcHash = it->second;
#else
			POSITION pos1 = m_mapSources.GetStartPosition();
			while( pos1 != NULL )
			{
				CCKey key1;
				SrcHash* pCurrSrcHash;
				m_mapSources.GetNextAssoc( pos1, key1, pCurrSrcHash );
#endif
				CKadSourcePtrList& keyHashSrcMap = pCurrSrcHash->ptrlistSource;
				for(POSITION pos2 = keyHashSrcMap.GetHeadPosition(); pos2 != NULL; )
				{
					POSITION pos3 = pos2;
					Source* pCurrSource = keyHashSrcMap.GetNext(pos2);
					CKadEntryPtrList& srcEntryList = pCurrSource->ptrlEntryList;
					for(POSITION pos4 = srcEntryList.GetHeadPosition(); pos4 != NULL; )
					{
						POSITION pos5 = pos4;
						CEntry* pCurrName = srcEntryList.GetNext(pos4);
						uTotalSource++;
						if( pCurrName->m_tLifetime < tNow)
						{
							uRemovedSource++;
							srcEntryList.RemoveAt(pos5);
							delete pCurrName;
						}
					}
					if( srcEntryList.IsEmpty())
					{
						keyHashSrcMap.RemoveAt(pos3);
						delete pCurrSource;
					}
				}
				if( keyHashSrcMap.IsEmpty())
				{
#ifdef REPLACE_ATLMAP
					it = m_mapSources.erase(it);
#else
					m_mapSources.RemoveKey(key1);
#endif
					delete pCurrSrcHash;
				}
#ifdef REPLACE_ATLMAP
				else
					++it;
#endif
			}
		}

		m_uTotalIndexSource = uTotalSource - uRemovedSource; // WiZaRd
		m_uTotalIndexKeyword = uTotalKey - uRemovedKey;
		AddDebugLogLine( false, _T("Removed %u keyword out of %u and %u source out of %u"), uRemovedKey, uTotalKey, uRemovedSource, uTotalSource);
		m_tLastClean = time(NULL) + MIN2S(30);
	}
	catch(...)
	{
		AddDebugLogLine(false, _T("Exception in CIndexed::clean"));
		ASSERT(0);
	}
}

bool CIndexed::AddKeyword(const CUInt128& uKeyID, const CUInt128& uSourceID, Kademlia::CKeyEntry* pEntry, uint8& uLoad, bool bIgnoreThreadLock)
{
	// do not access any data while the loading thread is busy;
	// bIgnoreThreadLock should be only used by CLoadDataThread itself
	if (!bIgnoreThreadLock && !m_bDataLoaded) {
		DEBUG_ONLY( DebugLogWarning(_T("CIndexed Memberfunction call failed because the dataloading still in progress")) );
		return false;
	}

	if( !pEntry )
		return false;

	if (!pEntry->IsKeyEntry()){
		ASSERT( false );
		return false;
	}

	if( m_uTotalIndexKeyword > KADEMLIAMAXENTRIES )
	{
		uLoad = 100;
		return false;
	}

	if( pEntry->m_uSize == 0 || pEntry->GetCommonFileName().IsEmpty() || pEntry->GetTagCount() == 0 || pEntry->m_tLifetime < time(NULL))
		return false;

	KeyHash* pCurrKeyHash;
#ifdef REPLACE_ATLMAP
	CKeyHashMap::const_iterator it = m_mapKeyword.find(uKeyID.GetData());
	if(it == m_mapKeyword.end())
#else
	if(!m_mapKeyword.Lookup(CCKey(uKeyID.GetData()), pCurrKeyHash))
#endif
	{
		Source* pCurrSource = new Source;
		pCurrSource->uSourceID.SetValue(uSourceID);
		pEntry->MergeIPsAndFilenames(NULL); //IpTracking init
		pCurrSource->ptrlEntryList.AddHead(pEntry);
		pCurrKeyHash = new KeyHash;
		pCurrKeyHash->uKeyID.SetValue(uKeyID);
#ifdef REPLACE_ATLMAP
		pCurrKeyHash->mapSource[pCurrSource->uSourceID.GetData()] = pCurrSource;
		m_mapKeyword[pCurrKeyHash->uKeyID.GetData()] = pCurrKeyHash;
#else
		pCurrKeyHash->mapSource.SetAt(CCKey(pCurrSource->uSourceID.GetData()), pCurrSource);
		m_mapKeyword.SetAt(CCKey(pCurrKeyHash->uKeyID.GetData()), pCurrKeyHash);
#endif
		uLoad = 1;
		m_uTotalIndexKeyword++;
		return true;
	}
	else
	{
#ifdef REPLACE_ATLMAP
		pCurrKeyHash = it->second;
		CSourceKeyMap& keySrcKeyMap = pCurrKeyHash->mapSource;
		uint_ptr uIndexTotal = keySrcKeyMap.size();
#else
		CSourceKeyMap& keySrcKeyMap = pCurrKeyHash->mapSource;
		uint_ptr uIndexTotal = (uint_ptr) keySrcKeyMap.GetCount();
#endif
		if ( uIndexTotal > KADEMLIAMAXINDEX )
		{
			uLoad = 100;
			//Too many entries for this Keyword..
			return false;
		}
		Source* pCurrSource;
#ifdef REPLACE_ATLMAP
		CSourceKeyMap::const_iterator it = keySrcKeyMap.find(uSourceID.GetData());
		if(it != keySrcKeyMap.end())
		{
			pCurrSource = it->second;
#else
		if(keySrcKeyMap.Lookup(CCKey(uSourceID.GetData()), pCurrSource))
		{
#endif
			CKadEntryPtrList& srcEntryList = pCurrSource->ptrlEntryList;
			if (srcEntryList.GetCount() > 0)
			{
				if( uIndexTotal > KADEMLIAMAXINDEX - 5000 )
				{
					uLoad = 100;
					//We are in a hot node.. If we continued to update all the publishes
					//while this index is full, popular files will be the only thing you index.
					return false;
				}
				// also check for size match
				CKeyEntry* pOldEntry = NULL;
				for (POSITION pos = srcEntryList.GetHeadPosition(); pos != NULL; srcEntryList.GetNext(pos)){
					CKeyEntry* pCurEntry = (CKeyEntry*)srcEntryList.GetAt(pos);
					ASSERT( pCurEntry->IsKeyEntry() );
					if (pCurEntry->m_uSize == pEntry->m_uSize){
						pOldEntry = pCurEntry;
						srcEntryList.RemoveAt(pos);
						break;
					}
				}
				pEntry->MergeIPsAndFilenames(pOldEntry); // pOldEntry can be NULL, thats ok and we still need todo this call in this case
				if (pOldEntry == NULL){
					m_uTotalIndexKeyword++;
					DebugLogWarning(_T("Kad: Indexing: Keywords: Multiple sizes published for file %s"), pEntry->m_uSourceID.ToHexString());
				}
				DEBUG_ONLY( AddDebugLogLine(DLP_VERYLOW, false, _T("Indexed file %s"), pEntry->m_uSourceID.ToHexString()) );
				delete pOldEntry;
				pOldEntry = NULL;
			}
			else{
				m_uTotalIndexKeyword++;
				pEntry->MergeIPsAndFilenames(NULL); //IpTracking init
			}
			uLoad = (uint8)((uIndexTotal*100)/KADEMLIAMAXINDEX);
			srcEntryList.AddHead(pEntry);
			return true;
		}
		else
		{
			pCurrSource = new Source;
			pCurrSource->uSourceID.SetValue(uSourceID);
			pEntry->MergeIPsAndFilenames(NULL); //IpTracking init
			pCurrSource->ptrlEntryList.AddHead(pEntry);
#ifdef REPLACE_ATLMAP
			keySrcKeyMap[pCurrSource->uSourceID.GetData()] = pCurrSource;
#else
			keySrcKeyMap.SetAt(CCKey(pCurrSource->uSourceID.GetData()), pCurrSource);
#endif
			m_uTotalIndexKeyword++;
			uLoad = (uint8)((uIndexTotal*100)/KADEMLIAMAXINDEX);
			return true;
		}
	}
}

bool CIndexed::AddSources(const CUInt128& uKeyID, const CUInt128& uSourceID, Kademlia::CEntry* pEntry, uint8& uLoad, bool bIgnoreThreadLock)
{
	// do not access any data while the loading thread is busy;
	// bIgnoreThreadLock should be only used by CLoadDataThread itself
	if (!bIgnoreThreadLock && !m_bDataLoaded) {
		DEBUG_ONLY( DebugLogWarning(_T("CIndexed Memberfunction call failed because the dataloading still in progress")) );
		return false;
	}

	if( !pEntry )
		return false;
	if( pEntry->m_uIP == 0 || pEntry->m_uTCPPort == 0 || pEntry->m_uUDPPort == 0 || pEntry->GetTagCount() == 0 || pEntry->m_tLifetime < time(NULL))
		return false;

	SrcHash* pCurrSrcHash;
#ifdef REPLACE_ATLMAP
	CSrcHashMap::const_iterator it = m_mapSources.find(uKeyID.GetData());
	if(it == m_mapSources.end())
#else
	if(!m_mapSources.Lookup(CCKey(uKeyID.GetData()), pCurrSrcHash))
#endif
	{
		Source* pCurrSource = new Source;
		pCurrSource->uSourceID.SetValue(uSourceID);
		pCurrSource->ptrlEntryList.AddHead(pEntry);
		pCurrSrcHash = new SrcHash;
		pCurrSrcHash->uKeyID.SetValue(uKeyID);
		pCurrSrcHash->ptrlistSource.AddHead(pCurrSource);
#ifdef REPLACE_ATLMAP
		m_mapSources[pCurrSrcHash->uKeyID.GetData()] = pCurrSrcHash;
#else
		m_mapSources.SetAt(CCKey(pCurrSrcHash->uKeyID.GetData()), pCurrSrcHash);
#endif
		m_uTotalIndexSource++;
		uLoad = 1;
		return true;
	}
	else
	{
#ifdef REPLACE_ATLMAP
		pCurrSrcHash = it->second;
#endif
		CKadSourcePtrList& keyHashSrcMap = pCurrSrcHash->ptrlistSource;
		size_t uSize = keyHashSrcMap.GetCount();
		for(POSITION pos1 = keyHashSrcMap.GetHeadPosition(); pos1 != NULL; )
		{
			Source* pCurrSource = keyHashSrcMap.GetNext(pos1);
			CKadEntryPtrList& srcEntryList = pCurrSource->ptrlEntryList;
			if( srcEntryList.GetCount() )
			{
				CEntry* pCurrEntry = srcEntryList.GetHead();
				ASSERT(pCurrEntry!=NULL);
				if( pCurrEntry->m_uIP == pEntry->m_uIP && ( pCurrEntry->m_uTCPPort == pEntry->m_uTCPPort || pCurrEntry->m_uUDPPort == pEntry->m_uUDPPort ))
				{
					delete srcEntryList.RemoveHead();
					srcEntryList.AddHead(pEntry);
					uLoad = (uint8)((uSize*100)/KADEMLIAMAXSOUCEPERFILE);
					return true;
				}
			}
			else
			{
				//This should never happen!
				srcEntryList.AddHead(pEntry);
				ASSERT(0);
				uLoad = (uint8)((uSize*100)/KADEMLIAMAXSOUCEPERFILE);
				m_uTotalIndexSource++;
				return true;
			}
		}
		if( uSize > KADEMLIAMAXSOUCEPERFILE )
		{
			Source* pCurrSource = keyHashSrcMap.RemoveTail();
			delete pCurrSource->ptrlEntryList.RemoveTail();
			pCurrSource->uSourceID.SetValue(uSourceID);
			pCurrSource->ptrlEntryList.AddHead(pEntry);
			keyHashSrcMap.AddHead(pCurrSource);
			uLoad = 100;
			return true;
		}
		else
		{
			Source* pCurrSource = new Source;
			pCurrSource->uSourceID.SetValue(uSourceID);
			pCurrSource->ptrlEntryList.AddHead(pEntry);
			keyHashSrcMap.AddHead(pCurrSource);
			m_uTotalIndexSource++;
			uLoad = (uint8)((uSize*100)/KADEMLIAMAXSOUCEPERFILE);
			return true;
		}
	}
}
/*
bool CIndexed::AddNotes(const CUInt128& uKeyID, const CUInt128& uSourceID, Kademlia::CEntry* pEntry, uint8& uLoad, bool bIgnoreThreadLock)
{
	// do not access any data while the loading thread is busy;
	// bIgnoreThreadLock should be only used by CLoadDataThread itself
	if (!bIgnoreThreadLock && !m_bDataLoaded) {
		DEBUG_ONLY( DebugLogWarning(_T("CIndexed Memberfunction call failed because the dataloading still in progress")) );
		return false;
	}

	if( !pEntry )
		return false;
	if( pEntry->m_uIP == 0 || pEntry->GetTagCount() == 0 )
		return false;

	SrcHash* pCurrNoteHash;
#ifdef REPLACE_ATLMAP
	CSrcHashMap::const_iterator it = m_mapNotes.find(uKeyID.GetData());
	if(it == m_mapNotes.end())
#else
	if(!m_mapNotes.Lookup(CCKey(uKeyID.GetData()), pCurrNoteHash))
#endif
	{
		Source* pCurrNote = new Source;
		pCurrNote->uSourceID.SetValue(uSourceID);
		pCurrNote->ptrlEntryList.AddHead(pEntry);
		SrcHash* pCurrNoteHash = new SrcHash;
		pCurrNoteHash->uKeyID.SetValue(uKeyID);
		pCurrNoteHash->ptrlistSource.AddHead(pCurrNote);
#ifdef REPLACE_ATLMAP
		m_mapNotes[pCurrNoteHash->uKeyID.GetData()] = pCurrNoteHash;
#else
		m_mapNotes.SetAt(CCKey(pCurrNoteHash->uKeyID.GetData()), pCurrNoteHash);
#endif
		uLoad = 1;
		++m_uTotalIndexNotes;
		return true;
	}
	else
	{
#ifdef REPLACE_ATLMAP
		pCurrNoteHash = it->second;
#endif
		CKadSourcePtrList& keyHashNoteMap = pCurrNoteHash->ptrlistSource;
		size_t uSize = keyHashNoteMap.GetCount();
		for(POSITION pos1 = keyHashNoteMap.GetHeadPosition(); pos1 != NULL; )
		{
			Source* pCurrNote = keyHashNoteMap.GetNext(pos1);
			CKadEntryPtrList& noteEntryList = pCurrNote->ptrlEntryList;
			if( noteEntryList.GetCount() )
			{
				CEntry* pCurrEntry = noteEntryList.GetHead();
				if(pCurrEntry->m_uIP == pEntry->m_uIP || pCurrEntry->m_uSourceID == pEntry->m_uSourceID)
				{
					delete noteEntryList.RemoveHead();
					noteEntryList.AddHead(pEntry);
					uLoad = (uint8)((uSize*100)/KADEMLIAMAXNOTESPERFILE);
					return true;
				}
			}
			else
			{
				//This should never happen!
				noteEntryList.AddHead(pEntry);
				ASSERT(0);
				uLoad = (uint8)((uSize*100)/KADEMLIAMAXNOTESPERFILE);
				++m_uTotalIndexNotes; // WiZaRd
				return true;
			}
		}
		if( uSize > KADEMLIAMAXNOTESPERFILE )
		{
			Source* pCurrNote = keyHashNoteMap.RemoveTail();
			delete pCurrNote->ptrlEntryList.RemoveTail();
			pCurrNote->uSourceID.SetValue(uSourceID);
			pCurrNote->ptrlEntryList.AddHead(pEntry);
			keyHashNoteMap.AddHead(pCurrNote);
			uLoad = 100;
			return true;
		}
		else
		{
			Source* pCurrNote = new Source;
			pCurrNote->uSourceID.SetValue(uSourceID);
			pCurrNote->ptrlEntryList.AddHead(pEntry);
			keyHashNoteMap.AddHead(pCurrNote);
			uLoad = (uint8)((uSize*100)/KADEMLIAMAXNOTESPERFILE);
			++m_uTotalIndexNotes; // WiZaRd
			return true;
		}
	}
}*/

bool CIndexed::AddLoad(const CUInt128& uKeyID, uint64 uTime, bool bIgnoreThreadLock)// X: [64T] - [64BitTime]
{
	// do not access any data while the loading thread is busy;
	// bIgnoreThreadLock should be only used by CLoadDataThread itself
	if (!bIgnoreThreadLock && !m_bDataLoaded) {
		DEBUG_ONLY( DebugLogWarning(_T("CIndexed Memberfunction call failed because the dataloading still in progress")) );
		return false;
	}

	//This is needed for when you restart the client.
	if((uint64)time(NULL)>uTime)// X: [64T] - [64BitTime]
		return false;

#ifdef REPLACE_ATLMAP
	CLoadMap::const_iterator it = m_mapLoad.find(uKeyID.GetData());
	if(it != m_mapLoad.end())
		return false;
	Load* pLoad;
#else
	Load* pLoad;
	if(m_mapLoad.Lookup(CCKey(uKeyID.GetData()), pLoad))
		return false;
#endif

	pLoad = new Load();
	pLoad->uKeyID.SetValue(uKeyID);
	pLoad->uTime = uTime;
#ifdef REPLACE_ATLMAP
	m_mapLoad[pLoad->uKeyID.GetData()] = pLoad;
#else
	m_mapLoad.SetAt(CCKey(pLoad->uKeyID.GetData()), pLoad);
#endif
	m_uTotalIndexLoad++;
	return true;
}

void CIndexed::SendValidKeywordResult(const CUInt128& uKeyID, const SSearchTerm* pSearchTerms, uint32 uIP, uint16 uPort, bool bOldClient, uint16 uStartPosition, CKadUDPKey senderUDPKey)
{
	// do not access any data while the loading thread is busy;
	if (!m_bDataLoaded) {
		DEBUG_ONLY( DebugLogWarning(_T("CIndexed Memberfunction call failed because the dataloading still in progress")) );
		return;
	}

#ifdef REPLACE_ATLMAP
	CKeyHashMap::const_iterator it = m_mapKeyword.find(uKeyID.GetData());
	if(it != m_mapKeyword.end()){
		KeyHash* pCurrKeyHash = it->second;
#else
	KeyHash* pCurrKeyHash;
	if(m_mapKeyword.Lookup(CCKey(uKeyID.GetData()), pCurrKeyHash))
	{
#endif
		byte byPacket[1024*5];
		byte bySmallBuffer[2048];

		CByteIO byIO(byPacket,sizeof(byPacket));
		byIO.WriteByte(OP_KADEMLIAHEADER);
			byIO.WriteByte(KADEMLIA2_SEARCH_RES);
			byIO.WriteUInt128(Kademlia::CKademlia::GetPrefs()->GetKadID());
		byIO.WriteUInt128(uKeyID);
		
		byte* pbyCountPos = byPacket + byIO.GetUsed();
		ASSERT( byPacket+18+16 == pbyCountPos || byPacket+18 == pbyCountPos);
		byIO.WriteUInt16(0);

		const uint16 uMaxResults = 300;
		sint_ptr iCount = 0-uStartPosition;
		sint_ptr iUnsentCount = 0;
		CByteIO byIOTmp(bySmallBuffer, sizeof(bySmallBuffer));
		// we do 2 loops: In the first one we ignore all results which have a trustvalue below 1
		// in the second one we then also consider those. That way we make sure our 300 max results are not full
		// of spam entries. We could also sort by trustvalue, but we would risk to only send popular files this way
		// on very hot keywords
		bool bOnlyTrusted = true;
#ifdef _DEBUG// X: [RDL] - [Remove Debug Log]
		uint32 dbgResultsTrusted = 0;
		uint32 dbgResultsUntrusted = 0;
#endif
		CSourceKeyMap& keySrcKeyMap = pCurrKeyHash->mapSource;
		do{
#ifdef REPLACE_ATLMAP
			for (CSourceKeyMap::const_iterator it =  keySrcKeyMap.begin(); it !=  keySrcKeyMap.end();)
			{
				Source* pCurrSource = it->second;
				++it;
#else
			POSITION pos1 = keySrcKeyMap.GetStartPosition();
			while( pos1 != NULL )
			{
				CCKey key1;
				Source* pCurrSource;
				keySrcKeyMap.GetNextAssoc( pos1, key1, pCurrSource );
#endif
				CKadEntryPtrList& srcEntryList = pCurrSource->ptrlEntryList;
				for(POSITION pos2 = srcEntryList.GetHeadPosition(); pos2 != NULL; )
				{
					CKeyEntry* pCurrName = (CKeyEntry*)srcEntryList.GetNext(pos2);
					ASSERT( pCurrName->IsKeyEntry() );
					if ( (bOnlyTrusted ^ (pCurrName->GetTrustValue() < 1.0f)) && (!pSearchTerms || pCurrName->StartSearchTermsMatch(pSearchTerms)) )
					{
						if( iCount < 0 )
							iCount++;
						else if( (uint16)iCount < uMaxResults )
						{
							if((!bOldClient || pCurrName->m_uSize <= OLD_MAX_EMULE_FILE_SIZE))
							{
								iCount++;
#ifdef _DEBUG// X: [RDL] - [Remove Debug Log]
								if (bOnlyTrusted)
									dbgResultsTrusted++;
								else
									dbgResultsUntrusted++;
#endif
								byIOTmp.WriteUInt128(pCurrName->m_uSourceID);
									pCurrName->WriteTagListWithPublishInfo(&byIOTmp);
								
								if( byIO.GetUsed() + byIOTmp.GetUsed() > UDP_KAD_MAXFRAGMENT && iUnsentCount > 0)
								{
									uint_ptr uLen = sizeof(byPacket)-byIO.GetAvailable();
									PokeUInt16(pbyCountPos, (uint16)iUnsentCount);
									CKademlia::GetUDPListener()->SendPacket(byPacket, uLen, uIP, uPort, senderUDPKey, NULL);
									byIO.Reset();
									byIO.WriteByte(OP_KADEMLIAHEADER);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
										if (thePrefs.GetDebugClientKadUDPLevel() > 0)
											DebugSend("KADEMLIA2_SEARCH_RES", uIP, uPort);
#endif
										byIO.WriteByte(KADEMLIA2_SEARCH_RES);
										byIO.WriteUInt128(Kademlia::CKademlia::GetPrefs()->GetKadID());
									byIO.WriteUInt128(uKeyID);
									byIO.WriteUInt16(0);
									DEBUG_ONLY(DebugLog(_T("Sent %u keyword search results in one packet to avoid fragmentation"), iUnsentCount)); 
									iUnsentCount = 0;
								}
								ASSERT( byIO.GetUsed() + byIOTmp.GetUsed() <= UDP_KAD_MAXFRAGMENT );
								byIO.WriteArray(bySmallBuffer, byIOTmp.GetUsed());
								byIOTmp.Reset();
								iUnsentCount++;
							}
						}
						else
						{
#ifdef REPLACE_ATLMAP
							it = keySrcKeyMap.end();
#else
							pos1 = NULL;
#endif
							break;
						}
					}
				}
			}
			if (bOnlyTrusted && iCount < (sint_ptr)uMaxResults)
				bOnlyTrusted = false;
			else
				break;
		} while (!bOnlyTrusted);

#ifdef _DEBUG// X: [RDL] - [Remove Debug Log]
		// LOGTODO: Remove Log
		//DebugLog(_T("Kad Keyword search Result Request: Send %u trusted and %u untrusted results"), dbgResultsTrusted, dbgResultsUntrusted);
#endif

		if(iUnsentCount > 0)
		{
			uint_ptr uLen = sizeof(byPacket)-byIO.GetAvailable();
			PokeUInt16(pbyCountPos, (uint16)iUnsentCount);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
			if(thePrefs.GetDebugClientKadUDPLevel() > 0)
			{
				DebugSend("KADEMLIA2_SEARCH_RES", uIP, uPort);
			}
#endif
			CKademlia::GetUDPListener()->SendPacket(byPacket, uLen, uIP, uPort, senderUDPKey, NULL);
			DEBUG_ONLY(DebugLog(_T("Sent %u keyword search results in last packet to avoid fragmentation"), iUnsentCount));
		}
		else if (iCount > 0)
			ASSERT( false );
	}
	Clean();
}

void CIndexed::SendValidSourceResult(const CUInt128& uKeyID, uint32 uIP, uint16 uPort, uint16 uStartPosition, uint64 uFileSize, CKadUDPKey senderUDPKey)
{
	// do not access any data while the loading thread is busy;
	if (!m_bDataLoaded) {
		DEBUG_ONLY( DebugLogWarning(_T("CIndexed Memberfunction call failed because the dataloading still in progress")) );
		return;
	}

#ifdef REPLACE_ATLMAP
	CSrcHashMap::const_iterator it = m_mapSources.find(uKeyID.GetData());
	if(it != m_mapSources.end())
	{
		SrcHash* pCurrSrcHash = it->second;
#else
	SrcHash* pCurrSrcHash;
	if(m_mapSources.Lookup(CCKey(uKeyID.GetData()), pCurrSrcHash))
	{
#endif
		byte byPacket[1024*5];
		byte bySmallBuffer[2048];
		CByteIO byIO(byPacket,sizeof(byPacket));
		byIO.WriteByte(OP_KADEMLIAHEADER);
			byIO.WriteByte(KADEMLIA2_SEARCH_RES);
			byIO.WriteUInt128(Kademlia::CKademlia::GetPrefs()->GetKadID());

		byIO.WriteUInt128(uKeyID);
		byte* pbyCountPos = byPacket + byIO.GetUsed();
		ASSERT( byPacket+18+16 == pbyCountPos || byPacket+18 == pbyCountPos);
		byIO.WriteUInt16(0);
		
		sint_ptr iUnsentCount = 0;
		CByteIO byIOTmp(bySmallBuffer, sizeof(bySmallBuffer));

		uint16 uMaxResults = 300;
		sint_ptr iCount = 0-uStartPosition;
		CKadSourcePtrList& keyHashSrcMap = pCurrSrcHash->ptrlistSource;
		for(POSITION pos1 = keyHashSrcMap.GetHeadPosition(); pos1 != NULL; )
		{
			Source* pCurrSource = keyHashSrcMap.GetNext(pos1);
			CKadEntryPtrList& srcEntryList = pCurrSource->ptrlEntryList;
			if( srcEntryList.GetCount() )
			{
				CEntry* pCurrName = srcEntryList.GetHead();
				if( iCount < 0 )
					iCount++;
				else if( (uint16)iCount < uMaxResults )
				{
					if( !uFileSize || !pCurrName->m_uSize || pCurrName->m_uSize == uFileSize )
					{
						byIOTmp.WriteUInt128(pCurrName->m_uSourceID);
						pCurrName->WriteTagList(&byIOTmp);
						iCount++;
						if( byIO.GetUsed() + byIOTmp.GetUsed() > UDP_KAD_MAXFRAGMENT && iUnsentCount > 0)
						{
							uint_ptr uLen = sizeof(byPacket)-byIO.GetAvailable();
							PokeUInt16(pbyCountPos, (uint16)iUnsentCount);
							CKademlia::GetUDPListener()->SendPacket(byPacket, uLen, uIP, uPort, senderUDPKey, NULL);
							byIO.Reset();
							byIO.WriteByte(OP_KADEMLIAHEADER);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
								if (thePrefs.GetDebugClientKadUDPLevel() > 0)
									DebugSend("KADEMLIA2_SEARCH_RES", uIP, uPort);
#endif
								byIO.WriteByte(KADEMLIA2_SEARCH_RES);
								byIO.WriteUInt128(Kademlia::CKademlia::GetPrefs()->GetKadID());
							byIO.WriteUInt128(uKeyID);
							byIO.WriteUInt16(0);
							//DEBUG_ONLY(DebugLog(_T("Sent %u source search results in one packet to avoid fragmentation"), iUnsentCount)); 
							iUnsentCount = 0;
						}
						ASSERT( byIO.GetUsed() + byIOTmp.GetUsed() <= UDP_KAD_MAXFRAGMENT );
						byIO.WriteArray(bySmallBuffer, byIOTmp.GetUsed());
						byIOTmp.Reset();
						iUnsentCount++;
					}
				}
				else
				{
					break;
				}
			}
		}

		if(iUnsentCount > 0)
		{
			uint_ptr uLen = sizeof(byPacket)-byIO.GetAvailable();
			PokeUInt16(pbyCountPos, (uint16)iUnsentCount);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
			if(thePrefs.GetDebugClientKadUDPLevel() > 0)
			{
				DebugSend("KADEMLIA2_SEARCH_RES", uIP, uPort);
			}
#endif
			CKademlia::GetUDPListener()->SendPacket(byPacket, uLen, uIP, uPort, senderUDPKey, NULL);
			//DEBUG_ONLY(DebugLog(_T("Sent %u source search results in last packet to avoid fragmentation"), iUnsentCount));
		}
		else if (iCount > 0)
			ASSERT( false );
	}
	Clean();
}
/*
void CIndexed::SendValidNoteResult(const CUInt128& uKeyID, uint32 uIP, uint16 uPort, uint64 uFileSize, CKadUDPKey senderUDPKey)
{
	// do not access any data while the loading thread is busy;
	if (!m_bDataLoaded) {
		DEBUG_ONLY( DebugLogWarning(_T("CIndexed Memberfunction call failed because the dataloading still in progress")) );
		return;
	}

	try
	{
#ifdef REPLACE_ATLMAP
		CSrcHashMap::const_iterator it = m_mapNotes.find(uKeyID.GetData());
		if(it != m_mapNotes.end()){
			SrcHash* pCurrNoteHash = it->second;
#else
		SrcHash* pCurrNoteHash;
		if(m_mapNotes.Lookup(CCKey(uKeyID.GetData()), pCurrNoteHash))
		{
#endif
			byte byPacket[1024*5];
			byte bySmallBuffer[2048];
			CByteIO byIO(byPacket,sizeof(byPacket));
			byIO.WriteByte(OP_KADEMLIAHEADER);
				byIO.WriteByte(KADEMLIA2_SEARCH_RES);
				byIO.WriteUInt128(Kademlia::CKademlia::GetPrefs()->GetKadID());
			byIO.WriteUInt128(uKeyID);

			byte* pbyCountPos = byPacket + byIO.GetUsed();
			ASSERT( byPacket+18+16 == pbyCountPos || byPacket+18 == pbyCountPos);
			byIO.WriteUInt16(0);

			sint_ptr iUnsentCount = 0;
			CByteIO byIOTmp(bySmallBuffer, sizeof(bySmallBuffer));
			uint16 uMaxResults = 150;
			uint16 uCount = 0;
			CKadSourcePtrList& keyHashNoteMap = pCurrNoteHash->ptrlistSource;
			for(POSITION pos1 = keyHashNoteMap.GetHeadPosition(); pos1 != NULL; )
			{
				Source* pCurrNote = keyHashNoteMap.GetNext(pos1);
				CKadEntryPtrList& noteEntryList = pCurrNote->ptrlEntryList;
				if( noteEntryList.GetCount() )
				{
					CEntry* pCurrName = noteEntryList.GetHead();
					if( uCount < uMaxResults )
					{
						if( !uFileSize || !pCurrName->m_uSize || uFileSize == pCurrName->m_uSize )
						{
							byIOTmp.WriteUInt128(pCurrName->m_uSourceID);
							pCurrName->WriteTagList(&byIOTmp);
							uCount++;
							if( byIO.GetUsed() + byIOTmp.GetUsed() > UDP_KAD_MAXFRAGMENT && iUnsentCount > 0)
							{
								uint_ptr uLen = sizeof(byPacket)-byIO.GetAvailable();
								PokeUInt16(pbyCountPos, (uint16)iUnsentCount);
								CKademlia::GetUDPListener()->SendPacket(byPacket, uLen, uIP, uPort, senderUDPKey, NULL);
								byIO.Reset();
								byIO.WriteByte(OP_KADEMLIAHEADER);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
									if (thePrefs.GetDebugClientKadUDPLevel() > 0)
										DebugSend("KADEMLIA2_SEARCH_RES", uIP, uPort);
#endif
									byIO.WriteByte(KADEMLIA2_SEARCH_RES);
									byIO.WriteUInt128(Kademlia::CKademlia::GetPrefs()->GetKadID());
								byIO.WriteUInt128(uKeyID);
								byIO.WriteUInt16(0);
								DEBUG_ONLY(DebugLog(_T("Sent %u keyword search results in one packet to avoid fragmentation"), iUnsentCount)); 
								iUnsentCount = 0;
							}
							ASSERT( byIO.GetUsed() + byIOTmp.GetUsed() <= UDP_KAD_MAXFRAGMENT );
							byIO.WriteArray(bySmallBuffer, byIOTmp.GetUsed());
							byIOTmp.Reset();
							iUnsentCount++;
						}
					}
					else
					{
						break;
					}
				}
			}
			if(iUnsentCount > 0)
			{
				uint_ptr uLen = sizeof(byPacket)-byIO.GetAvailable();
				PokeUInt16(pbyCountPos, (uint16)iUnsentCount);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
				if(thePrefs.GetDebugClientKadUDPLevel() > 0)
				{
					DebugSend("KADEMLIA2_SEARCH_RES", uIP, uPort);
				}
#endif
				CKademlia::GetUDPListener()->SendPacket(byPacket, uLen, uIP, uPort, senderUDPKey, NULL);
				DEBUG_ONLY(DebugLog(_T("Sent %u note search results in last packet to avoid fragmentation"), iUnsentCount));
			}
			else if (uCount > 0)
				ASSERT( false );
		}
	}
	catch(...)
	{
		AddDebugLogLine(false, _T("Exception in CIndexed::SendValidNoteResult"));
	}
}*/

bool CIndexed::SendStoreRequest(const CUInt128& uKeyID)
{
	// do not access any data while the loading thread is busy;
	if (!m_bDataLoaded) {
		DEBUG_ONLY( DebugLogWarning(_T("CIndexed Memberfunction call failed because the dataloading still in progress")) );
		return true; // don't report overloaded with a false
	}

#ifdef REPLACE_ATLMAP
	CLoadMap::const_iterator it = m_mapLoad.find(uKeyID.GetData());
	if(it == m_mapLoad.end())
		return true;
	Load* pLoad = it->second;
	if(pLoad->uTime < (uint64)time(NULL))// X: [64T] - [64BitTime]
	{
		m_mapLoad.erase(it);
#else
	Load* pLoad;
	if(!m_mapLoad.Lookup(CCKey(uKeyID.GetData()), pLoad))
		return true;
	if(pLoad->uTime < (uint64)time(NULL))// X: [64T] - [64BitTime]
	{
		m_mapLoad.RemoveKey(CCKey(uKeyID.GetData()));
#endif
		m_uTotalIndexLoad--;
		delete pLoad;
		return true;
	}
	return false;
}

uint_ptr CIndexed::GetFileKeyCount()
{
	// do not access any data while the loading thread is busy;
	if (!m_bDataLoaded) {
		DEBUG_ONLY( DebugLogWarning(_T("CIndexed Memberfunction call failed because the dataloading still in progress")) );
		return 0;
	}

#ifdef REPLACE_ATLMAP
	return m_mapKeyword.size();
#else
	return (uint_ptr) m_mapKeyword.GetCount();
#endif
}

SSearchTerm::SSearchTerm()
{
	m_type = AND;
	m_pTag = NULL;
	m_pLeft = NULL;
	m_pRight = NULL;
}

SSearchTerm::~SSearchTerm()
{
	if (m_type == String)
		delete m_pastr;
	delete m_pTag;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/// CIndexed::CLoadDataThread Implementation
typedef CIndexed::CLoadDataThread CLoadDataThread;

CIndexed::CLoadDataThread::CLoadDataThread(CIndexed* pOwner) : Thread(true, PRIO_LOW), m_pOwner(pOwner)
{
}

void CIndexed::CLoadDataThread::run()
{
	DbgSetThreadName("Kademlia Indexed Load Data");
	if ( !m_pOwner )
		return;

	InitThreadLocale();
	ASSERT( m_pOwner->m_bDataLoaded == false );
	Poco::FastMutex::SingleLock sLock(m_pOwner->m_mutSync, true);

	ASSERT((LOADINDEX_VERSION & I64TIMEMASK) == 0);// X: [E64T] - [Enable64BitTime]
	ASSERT((KEYINDEX_VERSION & I64TIMEMASK) == 0);
	ASSERT((SRCINDEX_VERSION & I64TIMEMASK) == 0);

	try
	{
		uint_ptr uTotalLoad = 0;
		uint_ptr uTotalSource = 0;
		uint_ptr uTotalKeyword = 0;
		CUInt128 uKeyID, uID, uSourceID;
		
		if (!m_pOwner->m_bAbortLoading)
		{
			CBufferedFileIO fileLoad;
			if(fileLoad.Open(m_sLoadFileName, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite))
			{
				setvbuf(fileLoad.m_pStream, NULL, _IOFBF, 32768);
				uint_ptr uVersion = fileLoad.ReadUInt32();
				bool I64Time = ((uVersion & I64TIMEMASK) != 0);// X: [E64T] - [Enable64BitTime]
				uVersion&=~I64TIMEMASK;
				if(uVersion<=LOADINDEX_VERSION)// X: [CI] - [Code Improvement]
				{
					/*time_t tSaveTime = */I64Time?fileLoad.ReadUInt64():fileLoad.ReadUInt32();// X: [E64T] - [Enable64BitTime]
					uint_ptr uNumLoad = fileLoad.ReadUInt32();
					while(uNumLoad && !m_pOwner->m_bAbortLoading)
					{
						fileLoad.ReadUInt128(&uKeyID);
						if(m_pOwner->AddLoad(uKeyID, I64Time?fileLoad.ReadUInt64():fileLoad.ReadUInt32(), true))// X: [E64T] - [Enable64BitTime]
							uTotalLoad++;
						uNumLoad--;
					}
				}
				fileLoad.Close();
			}
			else
				DebugLogWarning(_T("Unable to load Kad file: %s"), m_sLoadFileName);
		}

		if (!m_pOwner->m_bAbortLoading)
		{
			CBufferedFileIO fileKey;
			if (fileKey.Open(m_sKeyFileName, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite))
			{
				setvbuf(fileKey.m_pStream, NULL, _IOFBF, 32768);

				uint_ptr uVersion = fileKey.ReadUInt32();
				bool I64Time = ((uVersion & I64TIMEMASK) != 0);// X: [E64T] - [Enable64BitTime]
				uVersion&=~I64TIMEMASK;
				if( uVersion <=KEYINDEX_VERSION)// X: [CI] - [Code Improvement]
				{
					time_t tSaveTime = I64Time?fileKey.ReadUInt64():fileKey.ReadUInt32();// X: [E64T] - [Enable64BitTime]
					if( tSaveTime > time(NULL) )
					{
						fileKey.ReadUInt128(&uID);
						if( Kademlia::CKademlia::GetPrefs()->GetKadID() == uID )
						{
							uint_ptr uNumKeys = fileKey.ReadUInt32();
							while( uNumKeys && !m_pOwner->m_bAbortLoading )
							{
								fileKey.ReadUInt128(&uKeyID);
								uint_ptr uNumSource = fileKey.ReadUInt32();
								while( uNumSource && !m_pOwner->m_bAbortLoading )
								{
									fileKey.ReadUInt128(&uSourceID);
									uint_ptr uNumName = fileKey.ReadUInt32();
									while( uNumName && !m_pOwner->m_bAbortLoading)
									{
										CKeyEntry* pToAdd = new Kademlia::CKeyEntry();
										pToAdd->m_uKeyID.SetValue(uKeyID);
										pToAdd->m_uSourceID.SetValue(uSourceID);									
										pToAdd->m_bSource = false;
										pToAdd->m_tLifetime = I64Time?fileKey.ReadUInt64():fileKey.ReadUInt32();// X: [E64T] - [Enable64BitTime]
										if (uVersion >= 3)
											pToAdd->ReadPublishTrackingDataFromFile(&fileKey, uVersion >= 4);
										uint_ptr uTotalTags = fileKey.ReadByte();
										while( uTotalTags )
										{
											CKadTag* pTag = fileKey.ReadTag();
											if(pTag)
											{
												if (!pTag->m_name.Compare(TAG_FILENAME))
												{
													if (pToAdd->GetCommonFileName().IsEmpty())
														pToAdd->SetFileName(pTag->GetStr());
													delete pTag;
												}
												else if (!pTag->m_name.Compare(TAG_FILESIZE))
												{
													pToAdd->m_uSize = pTag->GetInt();
													delete pTag;
												}
												else if (!pTag->m_name.Compare(TAG_SOURCEIP))
												{
													pToAdd->m_uIP = (uint32)pTag->GetInt();
													pToAdd->AddTag(pTag);
												}
												else if (!pTag->m_name.Compare(TAG_SOURCEPORT))
												{
													pToAdd->m_uTCPPort = (uint16)pTag->GetInt();
													pToAdd->AddTag(pTag);
												}
												else if (!pTag->m_name.Compare(TAG_SOURCEUPORT))
												{
													pToAdd->m_uUDPPort = (uint16)pTag->GetInt();
													pToAdd->AddTag(pTag);
												}
												else
												{
													pToAdd->AddTag(pTag);
												}
											}
											uTotalTags--;
										}
										uint8 uLoad;
										if(m_pOwner->AddKeyword(uKeyID, uSourceID, pToAdd, uLoad, true))
											uTotalKeyword++;
										else
											delete pToAdd;
										uNumName--;
									}
									uNumSource--;
								}
								uNumKeys--;
							}
						}
					}
				}
				fileKey.Close();
			}
			else
				DebugLogWarning(_T("Unable to load Kad file: %s"), m_sKeyFileName);
		}

		if (!m_pOwner->m_bAbortLoading)
		{
			CBufferedFileIO fileSource;
			if (fileSource.Open(m_sSourceFileName, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite))
			{
				setvbuf(fileSource.m_pStream, NULL, _IOFBF, 32768);

				uint_ptr uVersion = fileSource.ReadUInt32();
				bool I64Time = ((uVersion & I64TIMEMASK) != 0);// X: [E64T] - [Enable64BitTime]
				uVersion&=~I64TIMEMASK;
				if(uVersion<=SRCINDEX_VERSION)// X: [CI] - [Code Improvement]
				{
					time_t tSaveTime = I64Time?fileSource.ReadUInt64():fileSource.ReadUInt32();// X: [E64T] - [Enable64BitTime]
					if( tSaveTime > time(NULL) )
					{
						uint_ptr uNumKeys = fileSource.ReadUInt32();
						while( uNumKeys && !m_pOwner->m_bAbortLoading )
						{
							fileSource.ReadUInt128(&uKeyID);
							uint_ptr uNumSource = fileSource.ReadUInt32();
							while( uNumSource && !m_pOwner->m_bAbortLoading )
							{
								fileSource.ReadUInt128(&uSourceID);
								uint_ptr uNumName = fileSource.ReadUInt32();
								while( uNumName && !m_pOwner->m_bAbortLoading )
								{
									CEntry* pToAdd = new Kademlia::CEntry();
									pToAdd->m_bSource = true;
									pToAdd->m_tLifetime = I64Time?fileSource.ReadUInt64():fileSource.ReadUInt32();// X: [E64T] - [Enable64BitTime]
									uint_ptr uTotalTags = fileSource.ReadByte();
									while( uTotalTags )
									{
										CKadTag* pTag = fileSource.ReadTag();
										if(pTag)
										{
											if (!pTag->m_name.Compare(TAG_SOURCEIP))
											{
												pToAdd->m_uIP = (uint32)pTag->GetInt();
												pToAdd->AddTag(pTag);
											}
											else if (!pTag->m_name.Compare(TAG_SOURCEPORT))
											{
												pToAdd->m_uTCPPort = (uint16)pTag->GetInt();
												pToAdd->AddTag(pTag);
											}
											else if (!pTag->m_name.Compare(TAG_SOURCEUPORT))
											{
												pToAdd->m_uUDPPort = (uint16)pTag->GetInt();
												pToAdd->AddTag(pTag);
											}
											else
											{
												pToAdd->AddTag(pTag);
											}
										}
										uTotalTags--;
									}
									pToAdd->m_uKeyID.SetValue(uKeyID);
									pToAdd->m_uSourceID.SetValue(uSourceID);
									uint8 uLoad;
									if(m_pOwner->AddSources(uKeyID, uSourceID, pToAdd, uLoad, true))
										uTotalSource++;
									else
										delete pToAdd;
									uNumName--;
								}
								uNumSource--;
							}
							uNumKeys--;
						}
					}
				}
				fileSource.Close();

				m_pOwner->m_uTotalIndexSource = uTotalSource;
				m_pOwner->m_uTotalIndexKeyword = uTotalKeyword;
				m_pOwner->m_uTotalIndexLoad = uTotalLoad;
				AddDebugLogLine( false, _T("Read %u source, %u keyword, and %u load entries"), uTotalSource, uTotalKeyword, uTotalLoad);
			}
			else
				DebugLogWarning(_T("Unable to load Kad file: %s"), m_sSourceFileName);
		}
	}
	catch ( CIOException *ioe )
	{
		AddDebugLogLine( false, _T("CIndexed::CLoadDataThread::Run (IO error(%i))"), ioe->m_iCause);
		ioe->Delete();
	}
	catch (...)
	{
		AddDebugLogLine(false, _T("Exception in CIndexed::CLoadDataThread::Run"));
		ASSERT( false );
	}
	if (m_pOwner->m_bAbortLoading)
		AddDebugLogLine(false, _T("Terminating CIndexed::CLoadDataThread - early abort requested"));
	else
		AddDebugLogLine(false, _T("Terminating CIndexed::CLoadDataThread - finished loading data"));

	m_pOwner->m_bDataLoaded = true;
}
