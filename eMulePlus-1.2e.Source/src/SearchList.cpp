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

#include "stdafx.h"
#include "updownclient.h"
#include "packets.h"
#include "emule.h"
#include "Category.h"
#include "KnownFile.h"
#include "SearchList.h"
#include "Preferences.h"
#include "QArray.h"
#include "MMServer.h"
#include "otherfunctions.h"
#include "SharedFileList.h"
#include "SafeFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//SyruS CQArray-Sorting operators for SearchFileStruct
bool operator > (SearchFileStruct &first, SearchFileStruct &second)
{
	return (first.strIndex.CompareNoCase(second.strIndex) > 0);
}

bool operator < (SearchFileStruct &first, SearchFileStruct &second)
{
	return (first.strIndex.CompareNoCase(second.strIndex) < 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	ConvertED2KTag() converts media tag type and value if required
static void ConvertED2KTag(CTag **ppTag)
{
	CTag *pTag = *ppTag;

	if ((pTag->GetTagID() == 0) && (pTag->GetTagName() != NULL))
	{
		static const struct
		{
			byte	byteID;
			byte	byteED2KType;
			LPCSTR	pszED2KName;
		} s_aEmuleToED2KMetaTagsMap[] =
		{
			// Artist, Album and Title are disabled because they should be already part of the filename
			// and would therefore be redundant information sent to the servers.. and the servers count the
			// amount of sent data!
			{ FT_MEDIA_ARTIST,  TAGTYPE_STRING, FT_ED2K_MEDIA_ARTIST },
			{ FT_MEDIA_ALBUM,   TAGTYPE_STRING, FT_ED2K_MEDIA_ALBUM },
			{ FT_MEDIA_TITLE,   TAGTYPE_STRING, FT_ED2K_MEDIA_TITLE },
			{ FT_MEDIA_LENGTH,  TAGTYPE_STRING, FT_ED2K_MEDIA_LENGTH },
			{ FT_MEDIA_LENGTH,  TAGTYPE_UINT32, FT_ED2K_MEDIA_LENGTH },
			{ FT_MEDIA_BITRATE, TAGTYPE_UINT32, FT_ED2K_MEDIA_BITRATE },
			{ FT_MEDIA_CODEC,   TAGTYPE_STRING, FT_ED2K_MEDIA_CODEC }
		};

		for (uint32 j = 0; j < ARRSIZE(s_aEmuleToED2KMetaTagsMap); j++)
		{
			if ( CmpED2KTagName(pTag->GetTagName(), s_aEmuleToED2KMetaTagsMap[j].pszED2KName) == 0
				&& ( (pTag->IsStr() && s_aEmuleToED2KMetaTagsMap[j].byteED2KType == TAGTYPE_STRING)
					|| (pTag->IsInt() && s_aEmuleToED2KMetaTagsMap[j].byteED2KType == TAGTYPE_UINT32) ) )
			{
				if (pTag->IsStr())
				{
					if (s_aEmuleToED2KMetaTagsMap[j].byteID == FT_MEDIA_LENGTH)
					{
						CString	strVal = pTag->GetStringValue();
						uint32	dwHour, dwMin, dwSec, dwMediaLength = 0;
						int		iRc = _stscanf(strVal, _T("%u : %u : %u"), &dwHour, &dwMin, &dwSec);

						if ((iRc > 0) && (iRc <= 3))
						{
							if (iRc < 3)
							{
								if (iRc < 2)
								{
									dwSec = dwHour;
									dwMin = 0;
								}
								else
								{
									dwSec = dwMin;
									dwMin = dwHour;
								}
								dwHour = 0;
							}
							if ((dwSec > 59) || (dwMin > 59))
								dwMediaLength = 0;
							else
								dwMediaLength = dwHour * 3600 + dwMin * 60 + dwSec;
						}

						*ppTag = NULL;
						delete pTag;

						if (dwMediaLength != 0)
							*ppTag = new CTag(s_aEmuleToED2KMetaTagsMap[j].byteID, dwMediaLength);
					}
					else if (pTag->IsStringValueEmpty())	// delete useless tag
					{
						*ppTag = NULL;
						delete pTag;
					}
					else
						pTag->ChangeTagID(s_aEmuleToED2KMetaTagsMap[j].byteID);	// change tag ID keeping the same value
				}
				else if (pTag->IsInt())
				{
					if (pTag->GetIntValue() == 0)	// delete useless tag
					{
						*ppTag = NULL;
						delete pTag;
					}
					else
						pTag->ChangeTagID(s_aEmuleToED2KMetaTagsMap[j].byteID);	// change tag ID keeping the same value
				}
				break;
			}
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CSearchFile::CSearchFile(CFile &in_stream, ECodingFormat eCF, uint32 dwSearchID, CUpDownClient *pClient, LPCTSTR pszDirectory)
{
	EMULE_TRY

	m_nSearchID = dwSearchID;
	m_iSrvRating = PF_RATING_NONE;
	in_stream.Read(&m_fileHash, 16);		// <filehash 16>
	in_stream.Read(&m_dwClientID, 4);		// <clientip 4>
	in_stream.Read(&m_uClientPort, 2);		// <clientport 2>

	uint32		dwTagCnt, dwFileSzHi;

	in_stream.Read(&dwTagCnt, 4);			// <tagcount 4>

	CTag	*pTag = NULL;
	try
	{
		for (uint32 i = 0; i < dwTagCnt; i++)
		{
			pTag = new CTag();				// (tag) * tagcount

			pTag->FillFromStream(in_stream, eCF);
			ConvertED2KTag(&pTag);		// convert format of media tags
			if (pTag != NULL)
			{
			// Convert ED2K-server file rating tag
				if ((pTag->GetTagID() == FT_FILERATING) && pTag->IsInt())
				{
					uint32	dwPackedRating = pTag->GetIntValue();
					uint32	dwAvgRating = dwPackedRating & 0xFF;	//	Average rating used by clients

				//	Percentage of clients (related to 'Availability') who rated this file
					m_dwVoters = (dwPackedRating >> 8) & 0xFF;
					m_iSrvRating = dwAvgRating / (255 / 5/*RatingExcellent*/);
					m_dSrvRating = static_cast<double>(dwAvgRating) / (255.0 / 5.0);

					delete pTag;
				}
				else
					m_tagArray.Add(pTag);
				pTag = NULL;
			}
		}
	}
	catch(...)
	{
		delete pTag;
		throw;
	}

	CString strFileName = GetStrTagValue(FT_FILENAME);

	if (!strFileName.IsEmpty())
		SetFileName(strFileName);

	if (m_iSrvRating != PF_RATING_NONE)	// Valid rating tag was received
	{
		if (m_dwVoters > 100)
			m_dwVoters = 100;	// Check the range, as it is a percentage
	//	Server sends a percentage of clients who rated this file and total number of
	//	file sources ('Availability'), based on that we can estimate number of voters
		m_dwVoters = (m_dwVoters * GetIntTagValue(FT_SOURCES) + 50) / 100;
		if (m_dwVoters == 0)
			m_dwVoters = 1;	// If a server send rating tag, there should be at least 1 user
	}

	uint64	qwFileSz = GetInt64TagValue(FT_FILESIZE);

	if (((dwFileSzHi = GetIntTagValue(FT_FILESIZE_HI)) != 0) && (qwFileSz <= 0xFFFFFFFF))
		qwFileSz += (static_cast<uint64>(dwFileSzHi) << 32ui64);
	if (qwFileSz > MAX_EMULE_FILE_SIZE)
	{
		g_App.m_pMDlg->AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("Search file size larger than supported %#I64x '%s'"), qwFileSz, strFileName);
		qwFileSz = 0;
	}
	SetFileSize(qwFileSz);

//	Modify some tags for easier and correct future processing
	if (GetFileSize() <= PARTSIZE)
	{
		SetIntTagValue(FT_COMPLETE_SOURCES, GetSourceCount());
		SetIntTagValue(FT_LASTSEENCOMPLETE, 0);	//currently available
	}
	else if (GetCompleteSourceCount() != 0)
		SetIntTagValue(FT_LASTSEENCOMPLETE, 0);	//currently available
	else
		AddExistentIntTag(FT_LASTSEENCOMPLETE, 1);	//modify value to distinguish from 'available'

	if (pClient != NULL)
	{
	//	To be capable of adding the client from which we got that search result
	//	as a source, we have to explicitly store that client's data in the search result.
	//	NOTE: Do *NOT* store the 'pClient' ptr here! At the time when using the 'toadd'
	//	struct, the 'pClient' may have been deleted (just if the user has waited too long before
	//	using one of the client's search results) -> Crash!
		m_eType = SFT_CLIENT;

	//	Explicitly overwrite the already available client's IP, Port with the already validated UserID and Port.
	//	So, we have proper (low)UserID+Server pair.
		m_dwClientIDHybrid = pClient->GetUserIDHybrid();
		m_uClientPort = pClient->GetUserPort();

		m_nClientServerIP = pClient->GetServerIP();
		m_nClientServerPort = pClient->GetServerPort();
	}
	else
	{
		m_eType = SFT_SERVER;
		m_dwClientIDHybrid = (IsLowID(m_dwClientID)) ? m_dwClientID : ntohl(m_dwClientID);
		m_nClientServerIP = 0;
		m_nClientServerPort = 0;
	}

	m_strSearchFileDirectory = pszDirectory ? pszDirectory : _T("");

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CSearchFile::~CSearchFile()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32 CSearchFile::GetIntTagValue(byte tagname)
{
	EMULE_TRY
	for (int i = 0; i < m_tagArray.GetSize(); i++)
	{
		const CTag *pTag = m_tagArray[i];

		if ((pTag->GetTagID() == tagname) && pTag->IsInt())
			return pTag->GetIntValue();
	}
	EMULE_CATCH
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetIntTagValue() returns true if tag was found (in this case pdwOut will contain a valid value on exit)
bool CSearchFile::GetIntTagValue(byte tagname, uint32 *pdwOut)
{
	EMULE_TRY
	for (int i = 0; i < m_tagArray.GetSize(); i++)
	{
		const CTag *pTag = m_tagArray[i];

		if ((pTag->GetTagID() == tagname) && pTag->IsInt())
		{
			*pdwOut = pTag->GetIntValue();
			return true;
		}
	}
	EMULE_CATCH
	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint64 CSearchFile::GetInt64TagValue(byte tagname)
{
	EMULE_TRY
	for (int i = 0; i < m_tagArray.GetSize(); i++)
	{
		const CTag *pTag = m_tagArray[i];

		if ((pTag->GetTagID() == tagname) && pTag->IsAnyInt())
			return pTag->GetInt64Value();
	}
	EMULE_CATCH
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchFile::SetIntTagValue(byte tagname, uint32 dwVal)
{
	EMULE_TRY

	for (int i = 0; i < m_tagArray.GetSize(); i++)
	{
		CTag *pTag = m_tagArray[i];

		if ((pTag->GetTagID() == tagname) && pTag->IsInt())
		{
			pTag->SetIntValue(dwVal);
			return;
		}
	}
//	if tag was not found, add it
	CTag*	pTag = new CTag(tagname, dwVal);
	m_tagArray.Add(pTag);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString CSearchFile::GetStrTagValue(byte tagname)
{
	EMULE_TRY
	for (int i = 0; i < m_tagArray.GetSize(); i++)
	{
		const CTag *pTag = m_tagArray[i];

		if ((pTag->GetTagID() == tagname) && pTag->IsStr())
			return pTag->GetStringValue();
	}
	EMULE_CATCH
	return _T("");
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchFile::AddExistentIntTag(byte tagname, uint32 dwInc)
{
	EMULE_TRY
	for (int i = 0; i < m_tagArray.GetSize(); i++)
	{
		CTag *pTag = m_tagArray[i];

		if ((pTag->GetTagID() == tagname) && pTag->IsInt())
		{
			pTag->SetIntValue(pTag->GetIntValue() + dwInc);
			break;
		}
	}
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchFile::AddCompleteSources(uint32 in_dwCount)
{
	EMULE_TRY
	for (int i = 0; i < m_tagArray.GetSize(); i++)
	{
		CTag *pTag = m_tagArray[i];

		if ((pTag->GetTagID() == FT_COMPLETE_SOURCES) && pTag->IsInt())
		{
			pTag->SetIntValue(pTag->GetIntValue() + in_dwCount);
			return;
		}
	}
//	if tag was not found, add it
	CTag*	pTag = new CTag(FT_COMPLETE_SOURCES, in_dwCount);
	m_tagArray.Add(pTag);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchFile::UpdateLastSeenComplete(uint32 dwSeconds)
{
	EMULE_TRY
	for (int i = 0; i < m_tagArray.GetSize(); i++)
	{
		CTag *pTag = m_tagArray[i];

		if ((pTag->GetTagID() == FT_LASTSEENCOMPLETE) && pTag->IsInt())
		{
			if (dwSeconds < pTag->GetIntValue())
				pTag->SetIntValue(dwSeconds);
			return;
		}
	}
//	if tag was not found, add it
	CTag*	pTag = new CTag(FT_LASTSEENCOMPLETE, dwSeconds);
	m_tagArray.Add(pTag);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchFile::UpdateSrvRating(CSearchFile *pAdd)
{
	if (pAdd->m_iSrvRating != PF_RATING_NONE)	// Valid rating tag was received
	{
		if (m_iSrvRating != PF_RATING_NONE)	// Update previous value
		{
			uint32	dwNewVoters = m_dwVoters + pAdd->m_dwVoters;

		//	Estimate an overall rating for cases when several servers provided us with it
		//	The value is not precise as we don't know exact number of voters
			m_dSrvRating = ( m_dSrvRating * static_cast<double>(m_dwVoters) +
				pAdd->m_dSrvRating * static_cast<double>(pAdd->m_dwVoters) ) / static_cast<double>(dwNewVoters);
			m_dwVoters = dwNewVoters;
		//	Make sure we still within the range
			if (m_dSrvRating > 5.0)
			{
				m_dSrvRating = 5.0;
				m_iSrvRating = 5;
			}
			else if ((m_iSrvRating = static_cast<int>(m_dSrvRating)) < 1)
			{
				m_dSrvRating = 1.0;
				m_iSrvRating = 1;
			}
		}
		else
		{
			m_dSrvRating = pAdd->m_dSrvRating;
			m_iSrvRating = pAdd->m_iSrvRating;
			m_dwVoters = pAdd->m_dwVoters;
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CSearchList::CSearchList()
{
	m_pctlSearchList = NULL;
	m_dwCurrentSearchCount = 0;
	m_bMobilMuleSearch = false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CSearchList::~CSearchList()
{
	Clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchList::Clear()
{
	EMULE_TRY

	while (!list.IsEmpty())
		delete list.RemoveHead();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchList::RemoveResults(uint32 nSearchID)
{
	EMULE_TRY
	// this will not delete the item from the window, make sure your code does it if you call this
	ASSERT( m_pctlSearchList );

	POSITION		pos1, pos2;
	CSearchFile	   *pSearchFile;

	for (pos1 = list.GetHeadPosition();( pos2 = pos1 ) != NULL; )
	{
		pSearchFile = list.GetNext(pos1);
		if (pSearchFile->GetSearchID() == nSearchID)
		{
			list.RemoveAt(pos2);
			delete pSearchFile;
		}
	}
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchList::ShowResults(uint32 nSearchID)
{
	EMULE_TRY
	ASSERT( m_pctlSearchList );
	m_pctlSearchList->SetRedraw(false);

	CSearchFile	   *pSearchFile;

	for (POSITION pos = list.GetHeadPosition(); pos != NULL; )
	{
		pSearchFile = list.GetNext(pos);
		if( pSearchFile->GetSearchID() == nSearchID )
		{
			m_pctlSearchList->AddResult(pSearchFile);
		}
	}
	m_pctlSearchList->SetRedraw(true);
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchList::RemoveResults(CSearchFile *todel)
{
	EMULE_TRY

	POSITION remove_pos = list.Find(todel);
	if (remove_pos != NULL)
	{
		g_App.m_pMDlg->m_dlgSearch.m_ctlSearchList.RemoveResult( todel );
		list.RemoveAt(remove_pos);
		delete todel;
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchList::NewSearch(CSearchListCtrl *in_wnd, const CString &strTypes, uint32 dwSearchID, bool MobilMuleSearch)
{
	if(in_wnd)
		m_pctlSearchList = in_wnd;

	m_strResultType = strTypes;
	m_dwCurrentSearchCount = dwSearchID;
	m_bMobilMuleSearch = MobilMuleSearch;

	foundFilesCount.SetAt(dwSearchID, 0);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint16 CSearchList::ProcessSearchAnswer(char *pPacketBuffer, uint32 dwSize, ECodingFormat eCF, bool* pbIsMoreResultsAvailable)
{
	EMULE_TRY

	if ( g_App.m_pMDlg->m_dlgSearch.IsLastSearchCanceled() &&
		 !g_App.m_pMDlg->m_dlgSearch.IsMoreEnabled() )
		return 0;

	CSafeMemFile	packetStream(reinterpret_cast<BYTE*>(pPacketBuffer), dwSize, 0);
	uint32			dwNumResults;

	packetStream.Read(&dwNumResults, 4);		// <numresults 4>

	for (uint32 i = 0; i < dwNumResults; i++)
	{
		CSearchFile	*pSearchFile = new CSearchFile(packetStream, eCF, m_dwCurrentSearchCount);

		AddToList(pSearchFile, false);
	}
	if (m_bMobilMuleSearch)
	{
#ifdef OLD_SOCKETS_ENABLED
		g_App.m_pMMServer->SearchFinished(false);
#endif OLD_SOCKETS_ENABLED
		m_bMobilMuleSearch = false;
	}

	if (pbIsMoreResultsAvailable != NULL)
	{
		*pbIsMoreResultsAvailable = false;
		if (packetStream.GetLength() - packetStream.GetPosition() == 1)
		{
			byte	byteMore;

			packetStream.Read(&byteMore, 1);
			if (byteMore == 0x01)
			{
				*pbIsMoreResultsAvailable = true;
			}
		}
	}

	EMULE_CATCH

	return GetFoundFiles(m_dwCurrentSearchCount);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchList::ProcessSharedFileListAnswer(byte *pbytePacket, uint32 dwSize, CUpDownClient *pClient, LPCTSTR pszDirectory, bool bFirstDir)
{
	EMULE_TRY

	CSafeMemFile	packetStream(pbytePacket, dwSize, 0);
	uint32			dwNumResults;

	packetStream.Read(&dwNumResults, 4);		// <numresults 4>

	uint32			dwMySearchID = reinterpret_cast<uint32>(pClient);

	if (bFirstDir)
	{
		g_App.m_pMDlg->m_dlgSearch.DeleteSearch(dwMySearchID);
		g_App.m_pMDlg->m_dlgSearch.CreateNewTab(pClient->GetUserName(), dwMySearchID);
		foundFilesCount.SetAt(dwMySearchID,0);
	}

	for (uint32 i = 0; i < dwNumResults; i++)
	{
		CSearchFile		*pSearchFile = new CSearchFile(packetStream, pClient->GetStrCodingFormat(), dwMySearchID, pClient, pszDirectory);

		if (pSearchFile->IsLargeFile() && !pClient->SupportsLargeFiles())
		{
		//	Client offers large file but didn't announced support for that
			delete pSearchFile;
			continue;
		}
		AddToList(pSearchFile, true);
	}
#ifdef OLD_SOCKETS_ENABLED
	if (m_bMobilMuleSearch)
	{
		g_App.m_pMMServer->SearchFinished(false);
	}
#endif OLD_SOCKETS_ENABLED
	m_bMobilMuleSearch = false;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSearchList::AllowUDPSearchAnswer()
{
	return ( !g_App.m_pMDlg->m_dlgSearch.IsLastSearchCanceled() ||
		g_App.m_pMDlg->m_dlgSearch.IsMoreEnabled() );
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint16 CSearchList::ProcessUDPSearchAnswer(CMemFile &pckStream, ECodingFormat eCF)
{
	CSearchFile	   *toadd = new CSearchFile(pckStream, eCF, m_dwCurrentSearchCount);

	AddToList(toadd, false);

	return GetFoundFiles(m_dwCurrentSearchCount);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSearchList::AddToList(CSearchFile *pAddedFile, bool bClientResponse)
{
	EMULE_TRY

	CString		thisType = pAddedFile->GetFileTypeString();

	if ( (!bClientResponse &&
		!( thisType == m_strResultType
			|| m_strResultType == GetResString(IDS_SEARCH_ANY)
			|| m_strResultType == GetResString(IDS_SEARCH_DOC)
			|| (m_strResultType == GetResString(IDS_SEARCH_PRG) && thisType == GetResString(IDS_SEARCH_ARC))) )
		|| ( g_App.m_pMDlg->m_dlgSearch.IsDocumentSearch(pAddedFile->GetSearchID())
			&& thisType != GetResString(IDS_SEARCH_DOC) )
		|| ((pAddedFile->GetFileSize() == 0) || pAddedFile->IsFileNameEmpty()) )
	{
		delete pAddedFile;
		return false;
	}

	CString	sNotSearch = g_App.m_pMDlg->m_dlgSearch.GetNotSearch(pAddedFile->GetSearchID());

	//	Combine words from filter list and except box and exclude any searches
	//	with those words in the file name
	sNotSearch += g_App.m_pPrefs->GetFilterWords();

	if (!sNotSearch.IsEmpty())
	{
		CString	strNotSearchWord, strFileName = pAddedFile->GetFileName();
		int		iNotSearchPos = 0;

		strFileName.MakeLower();

		for (;;)
		{
			strNotSearchWord = sNotSearch.Tokenize(_T(" "), iNotSearchPos);
			if (strNotSearchWord.IsEmpty())
				break;
			if (strFileName.Find(strNotSearchWord) >= 0)
			{
				delete pAddedFile;
				return false;
			}
		}
	}

	CSearchFile	   *pSearchFile;

	for (POSITION pos = list.GetHeadPosition(); pos != NULL; )
	{
		pSearchFile = list.GetNext(pos);

		if ( (md4cmp(pAddedFile->GetFileHash(), pSearchFile->GetFileHash()) == 0) &&
			(pSearchFile->GetSearchID() == pAddedFile->GetSearchID()) )
		{
			uint32	dwTmp, dwAvailableSources = pAddedFile->GetIntTagValue(FT_SOURCES);

		//	Add complete sources tag only if it exists
			if (pAddedFile->GetIntTagValue(FT_COMPLETE_SOURCES, &dwTmp))
			{
				pSearchFile->AddCompleteSources(dwTmp);
				pSearchFile->SetIntTagValue(FT_LASTSEENCOMPLETE, 0);	//currently available
			}
			else if (pAddedFile->GetIntTagValue(FT_LASTSEENCOMPLETE, &dwTmp))
			{
			//	FT_LASTSEENCOMPLETE shouldn't be sent together with FT_COMPLETE_SOURCES
				pSearchFile->UpdateLastSeenComplete(dwTmp);
			}

			pSearchFile->AddExistentIntTag(FT_SOURCES, dwAvailableSources);
			pSearchFile->UpdateSrvRating(pAddedFile);

			if (m_pctlSearchList && !m_bMobilMuleSearch)
				m_pctlSearchList->UpdateChangingColumns(pSearchFile);

			delete pAddedFile;
			return true;
		}
	}

	if (list.AddTail(pAddedFile))
	{
		uint16	uVal;

		foundFilesCount.Lookup(pAddedFile->GetSearchID(), uVal);
		foundFilesCount.SetAt(pAddedFile->GetSearchID(), uVal + 1);
	}
	if (m_pctlSearchList && !m_bMobilMuleSearch)
		m_pctlSearchList->AddResult(pAddedFile);

	return true;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString CSearchList::GetWebList(const CString &strLinePattern, int iSortBy, bool bAscending, bool bShowColumn1, bool bShowColumn2, bool bShowColumn3, bool bShowColumn4, bool bShowColumn5) const
{
	EMULE_TRY

	static const TCHAR s_apcRatingFile[][13] =
	{
		_T("is_none"),
		_T("l_fake"),
		_T("l_sources_5"),
		_T("l_sources_10"),
		_T("l_sources_25"),
		_T("l_sources_50")
	};
#ifdef _DEBUG
	DWORD dwStart = ::GetTickCount();
	DWORD dwSortTime;
#endif

	CSearchFile* pFile;
	SearchFileStruct structFile, *pFData;
	CQArray<SearchFileStruct, SearchFileStruct> SearchFileArray;

	for (POSITION pos = list.GetHeadPosition(); pos != NULL;)
	{
		pFile = list.GetNext(pos);

		if ((pFile == NULL) || (pFile->GetFileSize() == 0) || pFile->IsFileNameEmpty())
			continue;

		structFile.strFileName = pFile->GetFileName();
		structFile.pcFileType = GetFileTypeForWebServer(structFile.strFileName);
		structFile.strFileHash = HashToString(pFile->GetFileHash());
		structFile.dwSourceCount = pFile->GetSourceCount();
		structFile.dwCompleteSourceCount = pFile->GetCompleteSourceCount();
		structFile.qwFileSize = pFile->GetFileSize();
		g_App.m_pFakeCheck->GetFakeComment(structFile.strFileHash, structFile.qwFileSize, &structFile.strFakeCheck);
		structFile.dwSrvRating = pFile->GetSrvFileRating();
		if (structFile.dwSrvRating >= ARRSIZE(s_apcRatingFile))
			structFile.dwSrvRating = 0;

		switch (iSortBy)
		{
			case WS_SLCOL_FILENAME:
			default:
				structFile.strIndex = structFile.strFileName;
				break;
			case WS_SLCOL_SIZE:
				structFile.strIndex.Format(_T("%13I64u"), structFile.qwFileSize);
				break;
			case WS_SLCOL_HASH:
				structFile.strIndex = structFile.strFileHash;
				break;
			case WS_SLCOL_SOURCES:
				structFile.strIndex.Format(_T("%09u"), structFile.dwSourceCount);
				break;
			case WS_SLCOL_FAKECHECK:
				structFile.strIndex = structFile.strFakeCheck;
				break;
			case WS_SLCOL_TYPE:
				structFile.strIndex = structFile.pcFileType;
				break;
			case WS_SLCOL_RATING:
				structFile.strIndex.Format(_T("%u"), structFile.dwSrvRating);
				break;
		}
		SearchFileArray.Add(structFile);
	}

	SearchFileArray.QuickSort(bAscending);

#ifdef _DEBUG
	DWORD dwEnd = ::GetTickCount();
	dwSortTime = dwEnd - dwStart;
#endif

	byte	abyteFileHash[16];
	uchar	nRed, nGreen, nBlue;
	CKnownFile	*pSameFile;
	const TCHAR	*pcOverlayImage;
	CString strColorPrefix;
	CString strColorSuffix = _T("</font>");
	CString strSources;
	CString strFilename;
	CString strTemp2;
	CString strOutput;

	pFData = SearchFileArray.GetData();
	for (int i = 0; i < SearchFileArray.GetCount(); i++)
	{
		nRed = nGreen = nBlue = 255;

		if ((pSameFile = g_App.m_pSharedFilesList->GetFileByID(StringToHash(pFData[i].strFileHash, abyteFileHash))) == NULL)
			pSameFile = g_App.m_pDownloadQueue->GetFileByID(abyteFileHash);

		if (pSameFile == NULL)
			pcOverlayImage = _T("none");
		else
		{
			if (pSameFile->IsPartFile())
			{
				pcOverlayImage = _T("jumpstart");
				nBlue = 0;
				nRed = 0;
			}
			else
			{
				pcOverlayImage = _T("release");
				nBlue = 0;
			}
		}
		strColorPrefix.Format(_T("<font color=\"#%02x%02x%02x\">"), nRed, nGreen, nBlue);

		strSources.Format(_T("%u(%u)"), pFData[i].dwSourceCount, pFData[i].dwCompleteSourceCount);
		strFilename = pFData[i].strFileName;
		strFilename.Replace(_T("'"),_T("\\'"));

		strTemp2.Format(_T("ed2k://|file|%s|%I64u|%s|/"),
			strFilename, pFData[i].qwFileSize, pFData[i].strFileHash);

		strOutput.AppendFormat( strLinePattern,
			pFData[i].pcFileType, strTemp2, pcOverlayImage,
			reinterpret_cast<const TCHAR*>(&s_apcRatingFile[pFData[i].dwSrvRating]),
			(bShowColumn1) ? strColorPrefix + StringLimit(pFData[i].strFileName, 70) + strColorSuffix : _T(""),
			(bShowColumn2) ? strColorPrefix + CastItoXBytes(pFData[i].qwFileSize) + strColorSuffix : _T(""),
			(bShowColumn3) ? strColorPrefix + pFData[i].strFileHash + strColorSuffix : _T(""),
			(bShowColumn4) ? strColorPrefix + strSources + strColorSuffix : _T(""),
			(bShowColumn5) ? strColorPrefix + pFData[i].strFakeCheck + strColorSuffix : _T(""),
			pFData[i].strFileHash );
	}

#ifdef _DEBUG
	g_App.m_pMDlg->AddLogLine(LOG_FL_DBG, _T("WebServer: Searchlist with %u elements sorted in %u ms, output generated in %u ms"), SearchFileArray.GetSize(), dwSortTime, ::GetTickCount() - dwEnd);
#endif

	return strOutput;

	EMULE_CATCH
	return _T("");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchList::AddFileToDownloadByHash(uchar* hash,EnumCategories eCatID)
{
	EMULE_TRY

	CSearchFile		*pSearchFile;

	for (POSITION pos = list.GetHeadPosition(); pos != NULL; )
	{
		pSearchFile = list.GetNext(pos);
		if (!md4cmp(hash,pSearchFile->GetFileHash()))
		{
			g_App.m_pDownloadQueue->AddSearchToDownload(pSearchFile, eCatID, g_App.m_pPrefs->StartDownloadPaused());
			break;
		}
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CSearchFile* CSearchList::DetachNextFile(uint32 nSearchID)
{
// mobilemule
	EMULE_TRY
	// the files are NOT deleted, make sure you do this if you call this function
	// find, removes and returns the searchresult with most Sources
	uint32			nHighSource = 0;
	POSITION		resultpos = 0;
	CSearchFile	   *pSearchFile;

	for (POSITION pos = list.GetHeadPosition(); pos != NULL; )
	{
		pSearchFile = list.GetNext(pos);
		if (pSearchFile->GetSearchID() == nSearchID)
		{
			if (pSearchFile->GetIntTagValue(FT_SOURCES) > nHighSource)
			{
				nHighSource = pSearchFile->GetIntTagValue(FT_SOURCES);
				resultpos = pos;
			}
		}
	}
	if (resultpos == NULL)
	{
		ASSERT ( false );
		return NULL;
	}
	CSearchFile* result = list.GetAt(resultpos);
	list.RemoveAt(resultpos);
	return result;
	EMULE_CATCH
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
