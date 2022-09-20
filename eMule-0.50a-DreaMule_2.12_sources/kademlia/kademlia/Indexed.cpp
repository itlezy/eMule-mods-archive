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
it will be added to the offical client.. Changing something without knowing
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
{
	m_mapKeyword.InitHashTable(1031);
	m_mapNotes.InitHashTable(1031);
	m_mapLoad.InitHashTable(1031);
	m_mapSources.InitHashTable(1031);
	m_sSourceFileName = CMiscUtils::GetAppDir();
	m_sSourceFileName.Append(CONFIGFOLDER);
	m_sSourceFileName.Append(_T("src_index.dat"));
	m_sKeyFileName = CMiscUtils::GetAppDir();
	m_sKeyFileName.Append(CONFIGFOLDER);
	m_sKeyFileName.Append(_T("key_index.dat"));
	m_sLoadFileName = CMiscUtils::GetAppDir();
	m_sLoadFileName.Append(CONFIGFOLDER);
	m_sLoadFileName.Append(_T("load_index.dat"));
	m_tLastClean = time(NULL) + (60*30);
	m_uTotalIndexSource = 0;
	m_uTotalIndexKeyword = 0;
	m_uTotalIndexNotes = 0;
	m_uTotalIndexLoad = 0;
	ReadFile();
}

void CIndexed::ReadFile(void)
{
	try
	{
		uint32 uTotalLoad = 0;
		uint32 uTotalSource = 0;
		uint32 uTotalKeyword = 0;
		CUInt128 uKeyID, uID, uSourceID;

		CBufferedFileIO fileLoad;
		if(fileLoad.Open(m_sLoadFileName, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite))
		{
			setvbuf(fileLoad.m_pStream, NULL, _IOFBF, 32768);
			uint32 uVersion = fileLoad.ReadUInt32();
			if(uVersion<2)
			{
				/*time_t tSaveTime = */fileLoad.ReadUInt32();
				uint32 uNumLoad = fileLoad.ReadUInt32();
				while(uNumLoad)
				{
					fileLoad.ReadUInt128(&uKeyID);
					if(AddLoad(uKeyID, fileLoad.ReadUInt32()))
						uTotalLoad++;
					uNumLoad--;
				}
			}
			fileLoad.Close();
		}

		CBufferedFileIO fileKey;
		if (fileKey.Open(m_sKeyFileName, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite))
		{
			setvbuf(fileKey.m_pStream, NULL, _IOFBF, 32768);

			uint32 uVersion = fileKey.ReadUInt32();
			if( uVersion < 3 )
			{
				time_t tSaveTime = fileKey.ReadUInt32();
				if( tSaveTime > time(NULL) )
				{
					fileKey.ReadUInt128(&uID);
					if( Kademlia::CKademlia::GetPrefs()->GetKadID() == uID )
					{
						uint32 uNumKeys = fileKey.ReadUInt32();
						while( uNumKeys )
						{
							fileKey.ReadUInt128(&uKeyID);
							uint32 uNumSource = fileKey.ReadUInt32();
							while( uNumSource )
							{
								fileKey.ReadUInt128(&uSourceID);
								uint32 uNumName = fileKey.ReadUInt32();
								while( uNumName )
								{
									CEntry* pToAdd = new Kademlia::CEntry();
									pToAdd->m_bSource = false;
									pToAdd->m_tLifetime = fileKey.ReadUInt32();
									uint32 uTotalTags = fileKey.ReadByte();
									while( uTotalTags )
									{
										CKadTag* pTag = fileKey.ReadTag();
										if(pTag)
										{
											if (!pTag->m_name.Compare(TAG_FILENAME))
											{
												pToAdd->m_fileName = pTag->GetStr();
												KadTagStrMakeLower(pToAdd->m_fileName); // make lowercase, the search code expects lower case strings!
												// NOTE: always add the 'name' tag, even if it's stored separately in 'fileName'. the tag is still needed for answering search request
												pToAdd->m_listTag.push_back(pTag);
											}
											else if (!pTag->m_name.Compare(TAG_FILESIZE))
											{
												pToAdd->m_uSize = pTag->GetInt();
												// NOTE: always add the 'size' tag, even if it's stored separately in 'size'. the tag is still needed for answering search request
												pToAdd->m_listTag.push_back(pTag);
											}
											else if (!pTag->m_name.Compare(TAG_SOURCEIP))
											{
												pToAdd->m_uIP = (uint32)pTag->GetInt();
												pToAdd->m_listTag.push_back(pTag);
											}
											else if (!pTag->m_name.Compare(TAG_SOURCEPORT))
											{
												pToAdd->m_uTCPPort = (uint16)pTag->GetInt();
												pToAdd->m_listTag.push_back(pTag);
											}
											else if (!pTag->m_name.Compare(TAG_SOURCEUPORT))
											{
												pToAdd->m_uUDPPort = (uint16)pTag->GetInt();
												pToAdd->m_listTag.push_back(pTag);
											}
											else
											{
												pToAdd->m_listTag.push_back(pTag);
											}
										}
										uTotalTags--;
									}
									pToAdd->m_uKeyID.SetValue(uKeyID);
									pToAdd->m_uSourceID.SetValue(uSourceID);
									uint8 uLoad;
									if(AddKeyword(uKeyID, uSourceID, pToAdd, uLoad))
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

		CBufferedFileIO fileSource;
		if (fileSource.Open(m_sSourceFileName, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite))
		{
			setvbuf(fileSource.m_pStream, NULL, _IOFBF, 32768);

			uint32 uVersion = fileSource.ReadUInt32();
			if( uVersion < 3 )
			{
				time_t tSaveTime = fileSource.ReadUInt32();
				if( tSaveTime > time(NULL) )
				{
					uint32 uNumKeys = fileSource.ReadUInt32();
					while( uNumKeys )
					{
						fileSource.ReadUInt128(&uKeyID);
						uint32 uNumSource = fileSource.ReadUInt32();
						while( uNumSource )
						{
							fileSource.ReadUInt128(&uSourceID);
							uint32 uNumName = fileSource.ReadUInt32();
							while( uNumName )
							{
								CEntry* pToAdd = new Kademlia::CEntry();
								pToAdd->m_bSource = true;
								pToAdd->m_tLifetime = fileSource.ReadUInt32();
								uint32 uTotalTags = fileSource.ReadByte();
								while( uTotalTags )
								{
									CKadTag* pTag = fileSource.ReadTag();
									if(pTag)
									{
										if (!pTag->m_name.Compare(TAG_SOURCEIP))
										{
											pToAdd->m_uIP = (uint32)pTag->GetInt();
											pToAdd->m_listTag.push_back(pTag);
										}
										else if (!pTag->m_name.Compare(TAG_SOURCEPORT))
										{
											pToAdd->m_uTCPPort = (uint16)pTag->GetInt();
											pToAdd->m_listTag.push_back(pTag);
										}
										else if (!pTag->m_name.Compare(TAG_SOURCEUPORT))
										{
											pToAdd->m_uUDPPort = (uint16)pTag->GetInt();
											pToAdd->m_listTag.push_back(pTag);
										}
										else
										{
											pToAdd->m_listTag.push_back(pTag);
										}
									}
									uTotalTags--;
								}
								pToAdd->m_uKeyID.SetValue(uKeyID);
								pToAdd->m_uSourceID.SetValue(uSourceID);
								uint8 uLoad;
								if(AddSources(uKeyID, uSourceID, pToAdd, uLoad))
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

			m_uTotalIndexSource = uTotalSource;
			m_uTotalIndexKeyword = uTotalKeyword;
			m_uTotalIndexLoad = uTotalLoad;
			AddDebugLogLine( false, _T("Read %u source, %u keyword, and %u load entries"), uTotalSource, uTotalKeyword, uTotalLoad);
		}
	}
	catch ( CIOException *ioe )
	{
		AddDebugLogLine( false, _T("Exception in CIndexed::readFile (IO error(%i))"), ioe->m_iCause);
		ioe->Delete();
	}
	catch (...)
	{
		AddDebugLogLine(false, _T("Exception in CIndexed::readFile"));
	}
}

CIndexed::~CIndexed()
{
	try
	{
		uint32 uTotalSource = 0;
		uint32 uTotalKey = 0;
		uint32 uTotalLoad = 0;

		CBufferedFileIO fileLoad;
		if(fileLoad.Open(m_sLoadFileName, CFile::modeWrite | CFile::modeCreate | CFile::typeBinary | CFile::shareDenyWrite))
		{
			setvbuf(fileLoad.m_pStream, NULL, _IOFBF, 32768);
			uint32 uVersion = 1;
			fileLoad.WriteUInt32(uVersion);
			fileLoad.WriteUInt32(time(NULL));
			fileLoad.WriteUInt32(m_mapLoad.GetCount());
			POSITION pos1 = m_mapLoad.GetStartPosition();
			while( pos1 != NULL )
			{
				Load* pLoad;
				CCKey key1;
				m_mapLoad.GetNextAssoc( pos1, key1, pLoad );
				fileLoad.WriteUInt128(pLoad->uKeyID);
				fileLoad.WriteUInt32(pLoad->uTime);
				uTotalLoad++;
				delete pLoad;
			}
			fileLoad.Close();
		}

		CBufferedFileIO fileSource;
		if (fileSource.Open(m_sSourceFileName, CFile::modeWrite | CFile::modeCreate | CFile::typeBinary | CFile::shareDenyWrite))
		{
			setvbuf(fileSource.m_pStream, NULL, _IOFBF, 32768);
			uint32 uVersion = 2;
			fileSource.WriteUInt32(uVersion);
			fileSource.WriteUInt32(time(NULL)+KADEMLIAREPUBLISHTIMES);
			fileSource.WriteUInt32(m_mapSources.GetCount());
			POSITION pos1 = m_mapSources.GetStartPosition();
			while( pos1 != NULL )
			{
				CCKey key1;
				SrcHash* pCurrSrcHash;
				m_mapSources.GetNextAssoc( pos1, key1, pCurrSrcHash );
				fileSource.WriteUInt128(pCurrSrcHash->uKeyID);
				CKadSourcePtrList* keyHashSrcMap = &pCurrSrcHash->ptrlistSource;
				fileSource.WriteUInt32(keyHashSrcMap->GetCount());
				POSITION pos2 = keyHashSrcMap->GetHeadPosition();
				while( pos2 != NULL )
				{
					Source* pCurrSource = keyHashSrcMap->GetNext(pos2);
					fileSource.WriteUInt128(pCurrSource->uSourceID);
					CKadEntryPtrList* srcEntryList = &pCurrSource->ptrlEntryList;
					fileSource.WriteUInt32(srcEntryList->GetCount());
					for(POSITION pos3 = srcEntryList->GetHeadPosition(); pos3 != NULL; )
					{
						CEntry* pCurrName = srcEntryList->GetNext(pos3);
						fileSource.WriteUInt32(pCurrName->m_tLifetime);
						fileSource.WriteTagList(pCurrName->m_listTag);
						delete pCurrName;
						uTotalSource++;
					}
					delete pCurrSource;
				}
				delete pCurrSrcHash;
			}
			fileSource.Close();
		}

		CBufferedFileIO fileKey;
		if (fileKey.Open(m_sKeyFileName, CFile::modeWrite | CFile::modeCreate | CFile::typeBinary | CFile::shareDenyWrite))
		{
			setvbuf(fileKey.m_pStream, NULL, _IOFBF, 32768);
			uint32 uVersion = 2;
			fileKey.WriteUInt32(uVersion);
			fileKey.WriteUInt32(time(NULL)+KADEMLIAREPUBLISHTIMEK);
			fileKey.WriteUInt128(Kademlia::CKademlia::GetPrefs()->GetKadID());
			fileKey.WriteUInt32(m_mapKeyword.GetCount());
			POSITION pos1 = m_mapKeyword.GetStartPosition();
			while( pos1 != NULL )
			{
				CCKey key1;
				KeyHash* pCurrKeyHash;
				m_mapKeyword.GetNextAssoc( pos1, key1, pCurrKeyHash );
				fileKey.WriteUInt128(pCurrKeyHash->uKeyID);
				CSourceKeyMap* keySrcKeyMap = &pCurrKeyHash->mapSource;
				fileKey.WriteUInt32(keySrcKeyMap->GetCount());
				POSITION pos2 = keySrcKeyMap->GetStartPosition();
				while( pos2 != NULL )
				{
					Source* pCurrSource;
					CCKey key2;
					keySrcKeyMap->GetNextAssoc( pos2, key2, pCurrSource );
					fileKey.WriteUInt128(pCurrSource->uSourceID);
					CKadEntryPtrList* srcEntryList = &pCurrSource->ptrlEntryList;
					fileKey.WriteUInt32(srcEntryList->GetCount());
					for(POSITION pos3 = srcEntryList->GetHeadPosition(); pos3 != NULL; )
					{
						CEntry* pCurrName = srcEntryList->GetNext(pos3);
						fileKey.WriteUInt32(pCurrName->m_tLifetime);
						fileKey.WriteTagList(pCurrName->m_listTag);
						delete pCurrName;
						uTotalKey++;
					}
					delete pCurrSource;
				}
				delete pCurrKeyHash;
			}
			fileKey.Close();
		}
		AddDebugLogLine( false, _T("Wrote %u source, %u keyword, and %u load entries"), uTotalSource, uTotalKey, uTotalLoad);

		POSITION pos1 = m_mapNotes.GetStartPosition();
		while( pos1 != NULL )
		{
			CCKey key1;
			SrcHash* pCurrNoteHash;
			m_mapNotes.GetNextAssoc( pos1, key1, pCurrNoteHash );
			CKadSourcePtrList* keyHashNoteMap = &pCurrNoteHash->ptrlistSource;
			POSITION pos2 = keyHashNoteMap->GetHeadPosition();
			while( pos2 != NULL )
			{
				Source* pCurrNote = keyHashNoteMap->GetNext(pos2);
				CKadEntryPtrList* noteEntryList = &pCurrNote->ptrlEntryList;
				for(POSITION pos3 = noteEntryList->GetHeadPosition(); pos3 != NULL; )
				{
					delete noteEntryList->GetNext(pos3);
				}
				delete pCurrNote;
			}
			delete pCurrNoteHash;
		}
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

void CIndexed::Clean(void)
{
	try
	{
		if( m_tLastClean > time(NULL) )
			return;

		uint32 uRemovedKey = 0;
		uint32 uRemovedSource = 0;
		uint32 uTotalSource = 0;
		uint32 uTotalKey = 0;
		time_t tNow = time(NULL);

		{
			POSITION pos1 = m_mapKeyword.GetStartPosition();
			while( pos1 != NULL )
			{
				CCKey key1;
				KeyHash* pCurrKeyHash;
				m_mapKeyword.GetNextAssoc( pos1, key1, pCurrKeyHash );
				POSITION pos2 = pCurrKeyHash->mapSource.GetStartPosition();
				while( pos2 != NULL )
				{
					CCKey key2;
					Source* pCurrSource;
					pCurrKeyHash->mapSource.GetNextAssoc( pos2, key2, pCurrSource );
					for(POSITION pos3 = pCurrSource->ptrlEntryList.GetHeadPosition(); pos3 != NULL; )
					{
						POSITION pos4 = pos3;
						CEntry* pCurrName = pCurrSource->ptrlEntryList.GetNext(pos3);
						uTotalKey++;
						if( !pCurrName->m_bSource && pCurrName->m_tLifetime < tNow)
						{
							uRemovedKey++;
							pCurrSource->ptrlEntryList.RemoveAt(pos4);
							delete pCurrName;
						}
					}
					if( pCurrSource->ptrlEntryList.IsEmpty())
					{
						pCurrKeyHash->mapSource.RemoveKey(key2);
						delete pCurrSource;
					}
				}
				if( pCurrKeyHash->mapSource.IsEmpty())
				{
					m_mapKeyword.RemoveKey(key1);
					delete pCurrKeyHash;
				}
			}
		}
		{
			POSITION pos1 = m_mapSources.GetStartPosition();
			while( pos1 != NULL )
			{
				CCKey key1;
				SrcHash* pCurrSrcHash;
				m_mapSources.GetNextAssoc( pos1, key1, pCurrSrcHash );
				for(POSITION pos2 = pCurrSrcHash->ptrlistSource.GetHeadPosition(); pos2 != NULL; )
				{
					POSITION pos3 = pos2;
					Source* pCurrSource = pCurrSrcHash->ptrlistSource.GetNext(pos2);
					for(POSITION pos4 = pCurrSource->ptrlEntryList.GetHeadPosition(); pos4 != NULL; )
					{
						POSITION pos5 = pos4;
						CEntry* pCurrName = pCurrSource->ptrlEntryList.GetNext(pos4);
						uTotalSource++;
						if( pCurrName->m_tLifetime < tNow)
						{
							uRemovedSource++;
							pCurrSource->ptrlEntryList.RemoveAt(pos5);
							delete pCurrName;
						}
					}
					if( pCurrSource->ptrlEntryList.IsEmpty())
					{
						pCurrSrcHash->ptrlistSource.RemoveAt(pos3);
						delete pCurrSource;
					}
				}
				if( pCurrSrcHash->ptrlistSource.IsEmpty())
				{
					m_mapSources.RemoveKey(key1);
					delete pCurrSrcHash;
				}
			}
		}

		m_uTotalIndexSource = uTotalSource;
		m_uTotalIndexKeyword = uTotalKey;
		AddDebugLogLine( false, _T("Removed %u keyword out of %u and %u source out of %u"), uRemovedKey, uTotalKey, uRemovedSource, uTotalSource);
		m_tLastClean = time(NULL) + MIN2S(30);
	}
	catch(...)
	{
		AddDebugLogLine(false, _T("Exception in CIndexed::clean"));
		ASSERT(0);
	}
}

bool CIndexed::AddKeyword(const CUInt128& uKeyID, const CUInt128& uSourceID, Kademlia::CEntry* pEntry, uint8& uLoad)
{
	if( !pEntry )
		return false;

	if( m_uTotalIndexKeyword > KADEMLIAMAXENTRIES )
	{
		uLoad = 100;
		return false;
	}

	if( pEntry->m_uSize == 0 || pEntry->m_fileName.IsEmpty() || pEntry->m_listTag.size() == 0 || pEntry->m_tLifetime < time(NULL))
		return false;

	KeyHash* pCurrKeyHash;
	if(!m_mapKeyword.Lookup(CCKey(uKeyID.GetData()), pCurrKeyHash))
	{
		Source* pCurrSource = new Source;
		pCurrSource->uSourceID.SetValue(uSourceID);
		pCurrSource->ptrlEntryList.AddHead(pEntry);
		pCurrKeyHash = new KeyHash;
		pCurrKeyHash->uKeyID.SetValue(uKeyID);
		pCurrKeyHash->mapSource.SetAt(CCKey(pCurrSource->uSourceID.GetData()), pCurrSource);
		m_mapKeyword.SetAt(CCKey(pCurrKeyHash->uKeyID.GetData()), pCurrKeyHash);
		uLoad = 1;
		m_uTotalIndexKeyword++;
		return true;
	}
	else
	{
		uint32 uIndexTotal = pCurrKeyHash->mapSource.GetCount();
		if ( uIndexTotal > KADEMLIAMAXINDEX )
		{
			uLoad = 100;
			//Too many entries for this Keyword..
			return false;
		}
		Source* pCurrSource;
		if(pCurrKeyHash->mapSource.Lookup(CCKey(uSourceID.GetData()), pCurrSource))
		{
			if (pCurrSource->ptrlEntryList.GetCount() > 0)
			{
				if( uIndexTotal > KADEMLIAMAXINDEX - 5000 )
				{
					uLoad = 100;
					//We are in a hot node.. If we continued to update all the publishes
					//while this index is full, popular files will be the only thing you index.
					return false;
				}
				delete pCurrSource->ptrlEntryList.GetHead();
				pCurrSource->ptrlEntryList.RemoveHead();
			}
			else
				m_uTotalIndexKeyword++;
			uLoad = (uint8)((uIndexTotal*100)/KADEMLIAMAXINDEX);
			pCurrSource->ptrlEntryList.AddHead(pEntry);
			return true;
		}
		else
		{
			pCurrSource = new Source;
			pCurrSource->uSourceID.SetValue(uSourceID);
			pCurrSource->ptrlEntryList.AddHead(pEntry);
			pCurrKeyHash->mapSource.SetAt(CCKey(pCurrSource->uSourceID.GetData()), pCurrSource);
			m_uTotalIndexKeyword++;
			uLoad = (uint8)((uIndexTotal*100)/KADEMLIAMAXINDEX);
			return true;
		}
	}
}

bool CIndexed::AddSources(const CUInt128& uKeyID, const CUInt128& uSourceID, Kademlia::CEntry* pEntry, uint8& uLoad)
{
	if( !pEntry )
		return false;
	if( pEntry->m_uIP == 0 || pEntry->m_uTCPPort == 0 || pEntry->m_uUDPPort == 0 || pEntry->m_listTag.size() == 0 || pEntry->m_tLifetime < time(NULL))
		return false;

	SrcHash* pCurrSrcHash;
	if(!m_mapSources.Lookup(CCKey(uKeyID.GetData()), pCurrSrcHash))
	{
		Source* pCurrSource = new Source;
		pCurrSource->uSourceID.SetValue(uSourceID);
		pCurrSource->ptrlEntryList.AddHead(pEntry);
		pCurrSrcHash = new SrcHash;
		pCurrSrcHash->uKeyID.SetValue(uKeyID);
		pCurrSrcHash->ptrlistSource.AddHead(pCurrSource);
		m_mapSources.SetAt(CCKey(pCurrSrcHash->uKeyID.GetData()), pCurrSrcHash);
		m_uTotalIndexSource++;
		uLoad = 1;
		return true;
	}
	else
	{
		uint32 uSize = pCurrSrcHash->ptrlistSource.GetSize();
		for(POSITION pos1 = pCurrSrcHash->ptrlistSource.GetHeadPosition(); pos1 != NULL; )
		{
			Source* pCurrSource = pCurrSrcHash->ptrlistSource.GetNext(pos1);
			if( pCurrSource->ptrlEntryList.GetSize() )
			{
				CEntry* pCurrEntry = pCurrSource->ptrlEntryList.GetHead();
				ASSERT(pCurrEntry!=NULL);
				if( pCurrEntry->m_uIP == pEntry->m_uIP && ( pCurrEntry->m_uTCPPort == pEntry->m_uTCPPort || pCurrEntry->m_uUDPPort == pEntry->m_uUDPPort ))
				{
					delete pCurrSource->ptrlEntryList.RemoveHead();
					pCurrSource->ptrlEntryList.AddHead(pEntry);
					uLoad = (uint8)((uSize*100)/KADEMLIAMAXSOUCEPERFILE);
					return true;
				}
			}
			else
			{
				//This should never happen!
				pCurrSource->ptrlEntryList.AddHead(pEntry);
				ASSERT(0);
				uLoad = (uint8)((uSize*100)/KADEMLIAMAXSOUCEPERFILE);
				m_uTotalIndexSource++;
				return true;
			}
		}
		if( uSize > KADEMLIAMAXSOUCEPERFILE )
		{
			Source* pCurrSource = pCurrSrcHash->ptrlistSource.RemoveTail();
			delete pCurrSource->ptrlEntryList.RemoveTail();
			pCurrSource->uSourceID.SetValue(uSourceID);
			pCurrSource->ptrlEntryList.AddHead(pEntry);
			pCurrSrcHash->ptrlistSource.AddHead(pCurrSource);
			uLoad = 100;
			return true;
		}
		else
		{
			Source* pCurrSource = new Source;
			pCurrSource->uSourceID.SetValue(uSourceID);
			pCurrSource->ptrlEntryList.AddHead(pEntry);
			pCurrSrcHash->ptrlistSource.AddHead(pCurrSource);
			m_uTotalIndexSource++;
			uLoad = (uint8)((uSize*100)/KADEMLIAMAXSOUCEPERFILE);
			return true;
		}
	}
}

bool CIndexed::AddNotes(const CUInt128& uKeyID, const CUInt128& uSourceID, Kademlia::CEntry* pEntry, uint8& uLoad)
{
	if( !pEntry )
		return false;
	if( pEntry->m_uIP == 0 || pEntry->m_listTag.size() == 0 )
		return false;

	SrcHash* pCurrNoteHash;
	if(!m_mapNotes.Lookup(CCKey(uKeyID.GetData()), pCurrNoteHash))
	{
		Source* pCurrNote = new Source;
		pCurrNote->uSourceID.SetValue(uSourceID);
		pCurrNote->ptrlEntryList.AddHead(pEntry);
		SrcHash* pCurrNoteHash = new SrcHash;
		pCurrNoteHash->uKeyID.SetValue(uKeyID);
		pCurrNoteHash->ptrlistSource.AddHead(pCurrNote);
		m_mapNotes.SetAt(CCKey(pCurrNoteHash->uKeyID.GetData()), pCurrNoteHash);
		uLoad = 1;
		m_uTotalIndexNotes++;
		return true;
	}
	else
	{
		uint32 uSize = pCurrNoteHash->ptrlistSource.GetSize();
		for(POSITION pos1 = pCurrNoteHash->ptrlistSource.GetHeadPosition(); pos1 != NULL; )
		{
			Source* pCurrNote = pCurrNoteHash->ptrlistSource.GetNext(pos1);
			if( pCurrNote->ptrlEntryList.GetSize() )
			{
				CEntry* pCurrEntry = pCurrNote->ptrlEntryList.GetHead();
				if(pCurrEntry->m_uIP == pEntry->m_uIP || pCurrEntry->m_uSourceID == pEntry->m_uSourceID)
				{
					delete pCurrNote->ptrlEntryList.RemoveHead();
					pCurrNote->ptrlEntryList.AddHead(pEntry);
					uLoad = (uint8)((uSize*100)/KADEMLIAMAXNOTESPERFILE);
					return true;
				}
			}
			else
			{
				//This should never happen!
				pCurrNote->ptrlEntryList.AddHead(pEntry);
				ASSERT(0);
				uLoad = (uint8)((uSize*100)/KADEMLIAMAXNOTESPERFILE);
				m_uTotalIndexKeyword++;
				return true;
			}
		}
		if( uSize > KADEMLIAMAXNOTESPERFILE )
		{
			Source* pCurrNote = pCurrNoteHash->ptrlistSource.RemoveTail();
			delete pCurrNote->ptrlEntryList.RemoveTail();
			pCurrNote->uSourceID.SetValue(uSourceID);
			pCurrNote->ptrlEntryList.AddHead(pEntry);
			pCurrNoteHash->ptrlistSource.AddHead(pCurrNote);
			uLoad = 100;
			return true;
		}
		else
		{
			Source* pCurrNote = new Source;
			pCurrNote->uSourceID.SetValue(uSourceID);
			pCurrNote->ptrlEntryList.AddHead(pEntry);
			pCurrNoteHash->ptrlistSource.AddHead(pCurrNote);
			uLoad = (uint8)((uSize*100)/KADEMLIAMAXNOTESPERFILE);
			m_uTotalIndexKeyword++;
			return true;
		}
	}
}

bool CIndexed::AddLoad(const CUInt128& uKeyID, uint32 uTime)
{
	//This is needed for when you restart the client.
	if((uint32)time(NULL)>uTime)
		return false;

	Load* pLoad;
	if(m_mapLoad.Lookup(CCKey(uKeyID.GetData()), pLoad))
		return false;

	pLoad = new Load();
	pLoad->uKeyID.SetValue(uKeyID);
	pLoad->uTime = uTime;
	m_mapLoad.SetAt(CCKey(pLoad->uKeyID.GetData()), pLoad);
	m_uTotalIndexLoad++;
	return true;
}

bool SearchTermsMatch(const SSearchTerm* pSearchTerm, const Kademlia::CEntry* pItem)
{
	// boolean operators
	if (pSearchTerm->m_type == SSearchTerm::AND)
		return SearchTermsMatch(pSearchTerm->m_pLeft, pItem) && SearchTermsMatch(pSearchTerm->m_pRight, pItem);

	if (pSearchTerm->m_type == SSearchTerm::OR)
		return SearchTermsMatch(pSearchTerm->m_pLeft, pItem) || SearchTermsMatch(pSearchTerm->m_pRight, pItem);

	if (pSearchTerm->m_type == SSearchTerm::NOT)
		return SearchTermsMatch(pSearchTerm->m_pLeft, pItem) && !SearchTermsMatch(pSearchTerm->m_pRight, pItem);

	// word which is to be searched in the file name (and in additional meta data as done by some ed2k servers???)
	if (pSearchTerm->m_type == SSearchTerm::String)
	{
		int iStrSearchTerms = pSearchTerm->m_pastr->GetCount();
		if (iStrSearchTerms == 0)
			return false;
		// if there are more than one search strings specified (e.g. "aaa bbb ccc") the entire string is handled
		// like "aaa AND bbb AND ccc". search all strings from the string search term in the tokenized list of
		// the file name. all strings of string search term have to be found (AND)
		for (int iSearchTerm = 0; iSearchTerm < iStrSearchTerms; iSearchTerm++)
		{
			// this will not give the same results as when tokenizing the filename string, but it is 20 times faster.
			if (wcsstr(pItem->m_fileName, pSearchTerm->m_pastr->GetAt(iSearchTerm)) == NULL)
				return false;
		}
		return true;
	}

	if (pSearchTerm->m_type == SSearchTerm::MetaTag)
	{
		if (pSearchTerm->m_pTag->m_type == 2) // meta tags with string values
		{
			for (TagList::const_iterator itTagList = pItem->m_listTag.begin(); itTagList != pItem->m_listTag.end(); ++itTagList)
			{
				const CKadTag* pTag = *itTagList;
				if (pTag->IsStr() && pSearchTerm->m_pTag->m_name.Compare(pTag->m_name) == 0)
					return pTag->GetStr().CompareNoCase(pSearchTerm->m_pTag->GetStr()) == 0;
			}
		}
	}
	else if (pSearchTerm->m_type == SSearchTerm::OpGreaterEqual)
	{
		if (pSearchTerm->m_pTag->IsInt()) // meta tags with integer values
		{
			for (TagList::const_iterator itTagList = pItem->m_listTag.begin(); itTagList != pItem->m_listTag.end(); ++itTagList)
			{
				const CKadTag* pTag = *itTagList;
				if (pTag->IsInt() && pSearchTerm->m_pTag->m_name.Compare(pTag->m_name) == 0)
					return pTag->GetInt() >= pSearchTerm->m_pTag->GetInt();
			}
		}
		else if (pSearchTerm->m_pTag->IsFloat()) // meta tags with float values
		{
			for (TagList::const_iterator itTagList = pItem->m_listTag.begin(); itTagList != pItem->m_listTag.end(); ++itTagList)
			{
				const Kademlia::CKadTag* pTag = *itTagList;
				if (pTag->IsFloat() && pSearchTerm->m_pTag->m_name.Compare(pTag->m_name) == 0)
					return pTag->GetFloat() >= pSearchTerm->m_pTag->GetFloat();
			}
		}
	}
	else if (pSearchTerm->m_type == SSearchTerm::OpLessEqual)
	{
		if (pSearchTerm->m_pTag->IsInt()) // meta tags with integer values
		{
			for (TagList::const_iterator itTagList = pItem->m_listTag.begin(); itTagList != pItem->m_listTag.end(); ++itTagList)
			{
				const Kademlia::CKadTag* pTag = *itTagList;
				if (pTag->IsInt() && pSearchTerm->m_pTag->m_name.Compare(pTag->m_name) == 0)
					return pTag->GetInt() <= pSearchTerm->m_pTag->GetInt();
			}
		}
		else if (pSearchTerm->m_pTag->IsFloat()) // meta tags with float values
		{
			for (TagList::const_iterator itTagList = pItem->m_listTag.begin(); itTagList != pItem->m_listTag.end(); ++itTagList)
			{
				const Kademlia::CKadTag* pTag = *itTagList;
				if (pTag->IsFloat() && pSearchTerm->m_pTag->m_name.Compare(pTag->m_name) == 0)
					return pTag->GetFloat() <= pSearchTerm->m_pTag->GetFloat();
			}
		}
	}
	else if (pSearchTerm->m_type == SSearchTerm::OpGreater)
	{
		if (pSearchTerm->m_pTag->IsInt()) // meta tags with integer values
		{
			for (TagList::const_iterator itTagList = pItem->m_listTag.begin(); itTagList != pItem->m_listTag.end(); ++itTagList)
			{
				const Kademlia::CKadTag* pTag = *itTagList;
				if (pTag->IsInt() && pSearchTerm->m_pTag->m_name.Compare(pTag->m_name) == 0)
					return pTag->GetInt() > pSearchTerm->m_pTag->GetInt();
			}
		}
		else if (pSearchTerm->m_pTag->IsFloat()) // meta tags with float values
		{
			for (TagList::const_iterator itTagList = pItem->m_listTag.begin(); itTagList != pItem->m_listTag.end(); ++itTagList)
			{
				const Kademlia::CKadTag* pTag = *itTagList;
				if (pTag->IsFloat() && pSearchTerm->m_pTag->m_name.Compare(pTag->m_name) == 0)
					return pTag->GetFloat() > pSearchTerm->m_pTag->GetFloat();
			}
		}
	}
	else if (pSearchTerm->m_type == SSearchTerm::OpLess)
	{
		if (pSearchTerm->m_pTag->IsInt()) // meta tags with integer values
		{
			for (TagList::const_iterator itTagList = pItem->m_listTag.begin(); itTagList != pItem->m_listTag.end(); ++itTagList)
			{
				const Kademlia::CKadTag* pTag = *itTagList;
				if (pTag->IsInt() && pSearchTerm->m_pTag->m_name.Compare(pTag->m_name) == 0)
					return pTag->GetInt() < pSearchTerm->m_pTag->GetInt();
			}
		}
		else if (pSearchTerm->m_pTag->IsFloat()) // meta tags with float values
		{
			for (TagList::const_iterator itTagList = pItem->m_listTag.begin(); itTagList != pItem->m_listTag.end(); ++itTagList)
			{
				const Kademlia::CKadTag* pTag = *itTagList;
				if (pTag->IsFloat() && pSearchTerm->m_pTag->m_name.Compare(pTag->m_name) == 0)
					return pTag->GetFloat() < pSearchTerm->m_pTag->GetFloat();
			}
		}
	}
	else if (pSearchTerm->m_type == SSearchTerm::OpEqual)
	{
		if (pSearchTerm->m_pTag->IsInt()) // meta tags with integer values
		{
			for (TagList::const_iterator itTagList = pItem->m_listTag.begin(); itTagList != pItem->m_listTag.end(); ++itTagList)
			{
				const Kademlia::CKadTag* pTag = *itTagList;
				if (pTag->IsInt() && pSearchTerm->m_pTag->m_name.Compare(pTag->m_name) == 0)
					return pTag->GetInt() == pSearchTerm->m_pTag->GetInt();
			}
		}
		else if (pSearchTerm->m_pTag->IsFloat()) // meta tags with float values
		{
			for (TagList::const_iterator itTagList = pItem->m_listTag.begin(); itTagList != pItem->m_listTag.end(); ++itTagList)
			{
				const Kademlia::CKadTag* pTag = *itTagList;
				if (pTag->IsFloat() && pSearchTerm->m_pTag->m_name.Compare(pTag->m_name) == 0)
					return pTag->GetFloat() == pSearchTerm->m_pTag->GetFloat();
			}
		}
	}
	else if (pSearchTerm->m_type == SSearchTerm::OpNotEqual)
	{
		if (pSearchTerm->m_pTag->IsInt()) // meta tags with integer values
		{
			for (TagList::const_iterator itTagList = pItem->m_listTag.begin(); itTagList != pItem->m_listTag.end(); ++itTagList)
			{
				const Kademlia::CKadTag* pTag = *itTagList;
				if (pTag->IsInt() && pSearchTerm->m_pTag->m_name.Compare(pTag->m_name) == 0)
					return pTag->GetInt() != pSearchTerm->m_pTag->GetInt();
			}
		}
		else if (pSearchTerm->m_pTag->IsFloat()) // meta tags with float values
		{
			for (TagList::const_iterator itTagList = pItem->m_listTag.begin(); itTagList != pItem->m_listTag.end(); ++itTagList)
			{
				const Kademlia::CKadTag* pTag = *itTagList;
				if (pTag->IsFloat() && pSearchTerm->m_pTag->m_name.Compare(pTag->m_name) == 0)
					return pTag->GetFloat() != pSearchTerm->m_pTag->GetFloat();
			}
		}
	}

	return false;
}

void CIndexed::SendValidKeywordResult(const CUInt128& uKeyID, const SSearchTerm* pSearchTerms, uint32 uIP, uint16 uPort, bool bOldClient, bool bKad2, uint16 uStartPosition)
{
	KeyHash* pCurrKeyHash;
	if(m_mapKeyword.Lookup(CCKey(uKeyID.GetData()), pCurrKeyHash))
	{
		byte byPacket[1024*50];
		CByteIO byIO(byPacket,sizeof(byPacket));
		byIO.WriteByte(OP_KADEMLIAHEADER);
		if(bKad2)
		{
			byIO.WriteByte(KADEMLIA2_SEARCH_RES);
			byIO.WriteUInt128(Kademlia::CKademlia::GetPrefs()->GetKadID());
		}
		else
			byIO.WriteByte(KADEMLIA_SEARCH_RES);
		byIO.WriteUInt128(uKeyID);
		byIO.WriteUInt16(50);
		uint16 uMaxResults = 300;
		int iCount = 0-uStartPosition;
		POSITION pos1 = pCurrKeyHash->mapSource.GetStartPosition();
		while( pos1 != NULL )
		{
			CCKey key1;
			Source* pCurrSource;
			pCurrKeyHash->mapSource.GetNextAssoc( pos1, key1, pCurrSource );
			for(POSITION pos2 = pCurrSource->ptrlEntryList.GetHeadPosition(); pos2 != NULL; )
			{
				CEntry* pCurrName = pCurrSource->ptrlEntryList.GetNext(pos2);
				if ( !pSearchTerms || SearchTermsMatch(pSearchTerms, pCurrName) )
				{
					if( iCount < 0 )
						iCount++;
					else if( (uint16)iCount < uMaxResults )
					{
						if((!bOldClient || pCurrName->m_uSize <= OLD_MAX_EMULE_FILE_SIZE))
						{
							iCount++;
							byIO.WriteUInt128(pCurrName->m_uSourceID);
							byIO.WriteTagList(pCurrName->m_listTag);
							if( iCount % 50 == 0 )
							{
								uint32 uLen = sizeof(byPacket)-byIO.GetAvailable();
								CKademlia::GetUDPListener()->SendPacket(byPacket, uLen, uIP, uPort);
								byIO.Reset();
								byIO.WriteByte(OP_KADEMLIAHEADER);
								if(bKad2)
								{
									if (thePrefs.GetDebugClientKadUDPLevel() > 0)
										DebugSend("KADEMLIA2_SEARCH_RES", uIP, uPort);
									byIO.WriteByte(KADEMLIA2_SEARCH_RES);
									byIO.WriteUInt128(Kademlia::CKademlia::GetPrefs()->GetKadID());
								}
								else
								{
									if (thePrefs.GetDebugClientKadUDPLevel() > 0)
										DebugSend("KADEMLIA_SEARCH_RES", uIP, uPort);
									byIO.WriteByte(KADEMLIA_SEARCH_RES);
								}
								byIO.WriteUInt128(uKeyID);
								byIO.WriteUInt16(50);
							}
						}
					}
					else
					{
						pos1 = NULL;
						break;
					}
				}
			}
		}
		if(iCount > 0)
		{
			uint16 uCountLeft = (uint16)iCount % 50;
			if( uCountLeft )
			{
				uint32 uLen = sizeof(byPacket)-byIO.GetAvailable();
				if(bKad2)
				{
					memcpy(byPacket+18+16, &uCountLeft, 2);
					if (thePrefs.GetDebugClientKadUDPLevel() > 0)
						DebugSend("KADEMLIA2_SEARCH_RES", uIP, uPort);
				}
				else
				{
					memcpy(byPacket+18, &uCountLeft, 2);
					if (thePrefs.GetDebugClientKadUDPLevel() > 0)
						DebugSend("KADEMLIA_SEARCH_RES", uIP, uPort);
				}
				CKademlia::GetUDPListener()->SendPacket(byPacket, uLen, uIP, uPort);
			}
		}
	}
	Clean();
}

void CIndexed::SendValidSourceResult(const CUInt128& uKeyID, uint32 uIP, uint16 uPort, bool bKad2, uint16 uStartPosition, uint64 uFileSize)
{
	SrcHash* pCurrSrcHash;
	if(m_mapSources.Lookup(CCKey(uKeyID.GetData()), pCurrSrcHash))
	{
		byte byPacket[1024*50];
		CByteIO byIO(byPacket,sizeof(byPacket));
		byIO.WriteByte(OP_KADEMLIAHEADER);
		if(bKad2)
		{
			byIO.WriteByte(KADEMLIA2_SEARCH_RES);
			byIO.WriteUInt128(Kademlia::CKademlia::GetPrefs()->GetKadID());
		}
		else
			byIO.WriteByte(KADEMLIA_SEARCH_RES);
		byIO.WriteUInt128(uKeyID);
		byIO.WriteUInt16(50);
		uint16 uMaxResults = 300;
		int iCount = 0-uStartPosition;
		for(POSITION pos1 = pCurrSrcHash->ptrlistSource.GetHeadPosition(); pos1 != NULL; )
		{
			Source* pCurrSource = pCurrSrcHash->ptrlistSource.GetNext(pos1);
			if( pCurrSource->ptrlEntryList.GetSize() )
			{
				CEntry* pCurrName = pCurrSource->ptrlEntryList.GetHead();
				if( iCount < 0 )
					iCount++;
				else if( (uint16)iCount < uMaxResults )
				{
					if( !uFileSize || !pCurrName->m_uSize || pCurrName->m_uSize == uFileSize )
					{
						byIO.WriteUInt128(pCurrName->m_uSourceID);
						byIO.WriteTagList(pCurrName->m_listTag);
						iCount++;
						if( iCount % 50 == 0 )
						{
							uint32 uLen = sizeof(byPacket)-byIO.GetAvailable();
							CKademlia::GetUDPListener()->SendPacket(byPacket, uLen, uIP, uPort);
							byIO.Reset();
							byIO.WriteByte(OP_KADEMLIAHEADER);
							if(bKad2)
							{
								if (thePrefs.GetDebugClientKadUDPLevel() > 0)
									DebugSend("KADEMLIA2_SEARCH_RES", uIP, uPort);
								byIO.WriteByte(KADEMLIA2_SEARCH_RES);
								byIO.WriteUInt128(Kademlia::CKademlia::GetPrefs()->GetKadID());
							}
							else
							{
								if (thePrefs.GetDebugClientKadUDPLevel() > 0)
									DebugSend("KADEMLIA_SEARCH_RES", uIP, uPort);
								byIO.WriteByte(KADEMLIA_SEARCH_RES);
							}
							byIO.WriteUInt128(uKeyID);
							byIO.WriteUInt16(50);
						}
					}
				}
				else
				{
					break;
				}
			}
		}
		if( iCount > 0 )
		{
			uint16 uCountLeft = (uint16)iCount % 50;
			if( uCountLeft )
			{
				uint32 uLen = sizeof(byPacket)-byIO.GetAvailable();
				if(bKad2)
				{
					memcpy(byPacket+18+16, &uCountLeft, 2);
					if (thePrefs.GetDebugClientKadUDPLevel() > 0)
						DebugSend("KADEMLIA2_SEARCH_RES", uIP, uPort);
				}
				else
				{
					memcpy(byPacket+18, &uCountLeft, 2);
					if (thePrefs.GetDebugClientKadUDPLevel() > 0)
						DebugSend("KADEMLIA_SEARCH_RES", uIP, uPort);
				}
				CKademlia::GetUDPListener()->SendPacket(byPacket, uLen, uIP, uPort);
			}
		}
	}
	Clean();
}

void CIndexed::SendValidNoteResult(const CUInt128& uKeyID, uint32 uIP, uint16 uPort, bool bKad2, uint64 uFileSize)
{
	try
	{
		SrcHash* pCurrNoteHash;
		if(m_mapNotes.Lookup(CCKey(uKeyID.GetData()), pCurrNoteHash))
		{
			byte byPacket[1024*50];
			CByteIO byIO(byPacket,sizeof(byPacket));
			byIO.WriteByte(OP_KADEMLIAHEADER);
			if(bKad2)
			{
				byIO.WriteByte(KADEMLIA2_SEARCH_RES);
				byIO.WriteUInt128(Kademlia::CKademlia::GetPrefs()->GetKadID());
			}
			else
				byIO.WriteByte(KADEMLIA_SEARCH_NOTES_RES);
			byIO.WriteUInt128(uKeyID);
			byIO.WriteUInt16(50);
			uint16 uMaxResults = 150;
			uint16 uCount = 0;
			for(POSITION pos1 = pCurrNoteHash->ptrlistSource.GetHeadPosition(); pos1 != NULL; )
			{
				Source* pCurrNote = pCurrNoteHash->ptrlistSource.GetNext(pos1);
				if( pCurrNote->ptrlEntryList.GetSize() )
				{
					CEntry* pCurrName = pCurrNote->ptrlEntryList.GetHead();
					if( uCount < uMaxResults )
					{
						if( !uFileSize || !pCurrName->m_uSize || uFileSize == pCurrName->m_uSize )
						{
							byIO.WriteUInt128(pCurrName->m_uSourceID);
							byIO.WriteTagList(pCurrName->m_listTag);
							uCount++;
							if( uCount % 50 == 0 )
							{
								uint32 uLen = sizeof(byPacket)-byIO.GetAvailable();
								CKademlia::GetUDPListener()->SendPacket(byPacket, uLen, uIP, uPort);
								byIO.Reset();
								byIO.WriteByte(OP_KADEMLIAHEADER);
								if(bKad2)
								{
									if (thePrefs.GetDebugClientKadUDPLevel() > 0)
										DebugSend("KADEMLIA2_SEARCH_RES", uIP, uPort);
									byIO.WriteByte(KADEMLIA2_SEARCH_RES);
									byIO.WriteUInt128(Kademlia::CKademlia::GetPrefs()->GetKadID());
								}
								else
								{
									if (thePrefs.GetDebugClientKadUDPLevel() > 0)
										DebugSend("KADEMLIA_SEARCH_NOTES_RES", uIP, uPort);
									byIO.WriteByte(KADEMLIA_SEARCH_NOTES_RES);
								}
								byIO.WriteUInt128(uKeyID);
								byIO.WriteUInt16(50);
							}
						}
					}
					else
					{
						break;
					}
				}
			}
			uint16 uCountLeft = uCount % 50;
			if( uCountLeft )
			{
				uint32 uLen = sizeof(byPacket)-byIO.GetAvailable();
				if(bKad2)
				{
					memcpy(byPacket+18+16, &uCountLeft, 2);
					if (thePrefs.GetDebugClientKadUDPLevel() > 0)
						DebugSend("KADEMLIA2_SEARCH_RES", uIP, uPort);
				}
				else
				{
					memcpy(byPacket+18, &uCountLeft, 2);
					if (thePrefs.GetDebugClientKadUDPLevel() > 0)
						DebugSend("KADEMLIA_SEARCH_NOTES_RES", uIP, uPort);
				}
				CKademlia::GetUDPListener()->SendPacket(byPacket, uLen, uIP, uPort);
			}
		}
	}
	catch(...)
	{
		AddDebugLogLine(false, _T("Exception in CIndexed::SendValidNoteResult"));
	}
}

bool CIndexed::SendStoreRequest(const CUInt128& uKeyID)
{
	Load* pLoad;
	if(m_mapLoad.Lookup(CCKey(uKeyID.GetData()), pLoad))
	{
		if(pLoad->uTime < (uint32)time(NULL))
		{
			m_mapLoad.RemoveKey(CCKey(uKeyID.GetData()));
			m_uTotalIndexLoad--;
			delete pLoad;
			return true;
		}
		return false;
	}
	return true;
}

uint32 CIndexed::GetFileKeyCount()
{
	return m_mapKeyword.GetCount();
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
