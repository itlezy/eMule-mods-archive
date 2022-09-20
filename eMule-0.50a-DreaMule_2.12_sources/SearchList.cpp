//this file is part of eMule
//Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "emule.h"
#include "SearchFile.h"
#include "SearchList.h"
#include "SearchParams.h"
#include "Packets.h"
#include "OtherFunctions.h"
#include "Preferences.h"
#include "UpDownClient.h"
#include "SafeFile.h"
//remove moblie mule
#include "SharedFileList.h"
#include "KnownFileList.h"
#include "DownloadQueue.h"
#include "PartFile.h"
#include "CxImage/xImage.h"
#include "kademlia/utils/uint128.h"
#include "Kademlia/Kademlia/Entry.h"
#include "emuledlg.h"
#include "SearchDlg.h"
#include "SearchListCtrl.h"
#include "Log.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
// CSearchList

CSearchList::CSearchList()
{
	outputwnd = NULL;
	m_MobilMuleSearch = false;
}

CSearchList::~CSearchList()
{
	Clear();
}

void CSearchList::Clear()
{
	for(POSITION pos = list.GetHeadPosition(); pos != NULL; )
		delete list.GetNext(pos);
	list.RemoveAll();
}

void CSearchList::RemoveResults(uint32 nSearchID)
{
	// this will not delete the item from the window, make sure your code does it if you call this
	for (POSITION pos = list.GetHeadPosition(); pos != NULL;)
	{
		POSITION posLast = pos;
		CSearchFile* cur_file =	list.GetNext(pos);
		if (cur_file->GetSearchID() == nSearchID)
		{
			list.RemoveAt(posLast);
			delete cur_file;
		}
	}
}

void CSearchList::ShowResults(uint32 nSearchID)
{
	ASSERT( outputwnd );
	outputwnd->SetRedraw(FALSE);
	CMuleListCtrl::EUpdateMode bCurUpdateMode = outputwnd->SetUpdateMode(CMuleListCtrl::none/*direct*/);

	for (POSITION pos = list.GetHeadPosition(); pos != NULL; )
	{
		const CSearchFile* cur_file = list.GetNext(pos);
		if (cur_file->GetListParent() == NULL && cur_file->GetSearchID() == nSearchID)
		{
			outputwnd->AddResult(cur_file);
			if (cur_file->IsListExpanded() && cur_file->GetListChildCount() > 0)
				outputwnd->UpdateSources(cur_file);
		}
	}

	outputwnd->SetUpdateMode(bCurUpdateMode);
	outputwnd->SetRedraw(TRUE);
}

void CSearchList::RemoveResult(CSearchFile* todel)
{
	for (POSITION pos = list.GetHeadPosition(); pos != NULL; )
	{
		POSITION posLast = pos;
		CSearchFile* cur_file = list.GetNext(pos);
		if (cur_file == todel)
		{
			theApp.emuledlg->searchwnd->RemoveResult(todel);
			list.RemoveAt(posLast);
			delete todel;
			return;
		}
	}
}

void CSearchList::NewSearch(CSearchListCtrl* pWnd, CStringA strResultFileType, uint32 nSearchID, ESearchType eSearchType, bool bMobilMuleSearch)
{
	if (pWnd)
		outputwnd = pWnd;

	m_strResultFileType = strResultFileType;
	if (eSearchType == SearchTypeEd2kServer || eSearchType == SearchTypeEd2kGlobal)
		m_nCurED2KSearchID = nSearchID;
	m_foundFilesCount.SetAt(nSearchID, 0);
	m_foundSourcesCount.SetAt(nSearchID, 0);
	m_MobilMuleSearch = bMobilMuleSearch;
}

UINT CSearchList::ProcessSearchAnswer(const uchar* in_packet, uint32 size,
									  CUpDownClient* Sender, bool* pbMoreResultsAvailable, LPCTSTR pszDirectory)
{
	ASSERT( Sender != NULL );
	uint32 nSearchID = (uint32)Sender;
	SSearchParams* pParams = new SSearchParams;
	pParams->strExpression = Sender->GetUserName();
	pParams->dwSearchID = nSearchID;
	pParams->bClientSharedFiles = true;
	if (theApp.emuledlg->searchwnd->CreateNewTab(pParams)){
		m_foundFilesCount.SetAt(nSearchID, 0);
		m_foundSourcesCount.SetAt(nSearchID, 0);
	}
	else{
		delete pParams;
		pParams = NULL;
	}

	CSafeMemFile packet(in_packet, size);
	UINT results = packet.ReadUInt32();
	for (UINT i = 0; i < results; i++){
		CSearchFile* toadd = new CSearchFile(&packet, Sender ? Sender->GetUnicodeSupport()!=utf8strNone : false, nSearchID, 0, 0, pszDirectory);
		if (toadd->IsLargeFile() && (Sender == NULL || !Sender->SupportsLargeFiles())){
			DebugLogWarning(_T("Client offers large file (%s) but doesn't announced support for it - ignoring file"), toadd->GetFileName());
			continue;
		}
		if (Sender){
			toadd->SetClientID(Sender->GetIP());
			toadd->SetClientPort(Sender->GetUserPort());
			toadd->SetClientServerIP(Sender->GetServerIP());
			toadd->SetClientServerPort(Sender->GetServerPort());
			if (Sender->GetServerIP() && Sender->GetServerPort()){
				CSearchFile::SServer server(Sender->GetServerIP(), Sender->GetServerPort());
				server.m_uAvail = 1;
				toadd->AddServer(server);
			}
			toadd->SetPreviewPossible( Sender->GetPreviewSupport() && ED2KFT_VIDEO == GetED2KFileTypeID(toadd->GetFileName()) );
		}
		AddToList(toadd, true);
	}

	if (pbMoreResultsAvailable)
		*pbMoreResultsAvailable = false;
	int iAddData = (int)(packet.GetLength() - packet.GetPosition());
	if (iAddData == 1){
		uint8 ucMore = packet.ReadUInt8();
		if (ucMore == 0x00 || ucMore == 0x01){
			if (pbMoreResultsAvailable)
				*pbMoreResultsAvailable = ucMore!=0;
			if (thePrefs.GetDebugClientTCPLevel() > 0)
				Debug(_T("  Client search answer(%s): More=%u\n"), Sender->GetUserName(), ucMore);
		}
		else{
			if (thePrefs.GetDebugClientTCPLevel() > 0)
				Debug(_T("*** NOTE: Client ProcessSearchAnswer(%s): ***AddData: 1 byte: 0x%02x\n"), Sender->GetUserName(), ucMore);
		}
	}
	else if (iAddData > 0){
		if (thePrefs.GetDebugClientTCPLevel() > 0){
			Debug(_T("*** NOTE: Client ProcessSearchAnswer(%s): ***AddData: %u bytes\n"), Sender->GetUserName(), iAddData);
			DebugHexDump(in_packet + packet.GetPosition(), iAddData);
		}
	}

	packet.Close();
	return GetResultCount(nSearchID);
}

UINT CSearchList::ProcessSearchAnswer(const uchar* in_packet, uint32 size, bool bOptUTF8,
									  uint32 nServerIP, uint16 nServerPort, bool* pbMoreResultsAvailable)
{
	CSafeMemFile packet(in_packet, size);
	UINT results = packet.ReadUInt32();
	for (UINT i = 0; i < results; i++){
		CSearchFile* toadd = new CSearchFile(&packet, bOptUTF8, m_nCurED2KSearchID);
		toadd->SetClientServerIP(nServerIP);
		toadd->SetClientServerPort(nServerPort);
		if (nServerIP && nServerPort){
			CSearchFile::SServer server(nServerIP, nServerPort);
			server.m_uAvail = toadd->GetIntTagValue(FT_SOURCES);
			toadd->AddServer(server);
		}
		AddToList(toadd, false);
	}


	if (pbMoreResultsAvailable)
		*pbMoreResultsAvailable = false;
	int iAddData = (int)(packet.GetLength() - packet.GetPosition());
	if (iAddData == 1){
		uint8 ucMore = packet.ReadUInt8();
		if (ucMore == 0x00 || ucMore == 0x01){
			if (pbMoreResultsAvailable)
				*pbMoreResultsAvailable = ucMore!=0;
			if (thePrefs.GetDebugServerTCPLevel() > 0)
				Debug(_T("  Search answer(Server %s:%u): More=%u\n"), ipstr(nServerIP), nServerPort, ucMore);
		}
		else{
			if (thePrefs.GetDebugServerTCPLevel() > 0)
				Debug(_T("*** NOTE: ProcessSearchAnswer(Server %s:%u): ***AddData: 1 byte: 0x%02x\n"), ipstr(nServerIP), nServerPort, ucMore);
		}
	}
	else if (iAddData > 0){
		if (thePrefs.GetDebugServerTCPLevel() > 0){
			Debug(_T("*** NOTE: ProcessSearchAnswer(Server %s:%u): ***AddData: %u bytes\n"), ipstr(nServerIP), nServerPort, iAddData);
			DebugHexDump(in_packet + packet.GetPosition(), iAddData);
		}
	}

	packet.Close();
	return GetED2KResultCount();
}

UINT CSearchList::ProcessUDPSearchAnswer(CFileDataIO& packet, bool bOptUTF8, uint32 nServerIP, uint16 nServerPort)
{
	CSearchFile* toadd = new CSearchFile(&packet, bOptUTF8, m_nCurED2KSearchID, nServerIP, nServerPort);
	AddToList(toadd);
	return GetED2KResultCount();
}

UINT CSearchList::GetResultCount(uint32 nSearchID) const
{
	UINT nSources = 0;
	VERIFY( m_foundSourcesCount.Lookup(nSearchID, nSources) );
	return nSources;
}

UINT CSearchList::GetED2KResultCount() const
{
	return GetResultCount(m_nCurED2KSearchID);
}

void CSearchList::GetWebList(CQArray<SearchFileStruct, SearchFileStruct> *SearchFileArray, int iSortBy) const
{
	for (POSITION pos = list.GetHeadPosition(); pos != NULL; )
	{
		const CSearchFile* pFile = list.GetNext(pos);
		if (pFile == NULL || pFile->GetListParent() != NULL || pFile->GetFileSize() == (uint64)0 || pFile->GetFileName().IsEmpty())
			continue;

		SearchFileStruct structFile;
		structFile.m_strFileName = pFile->GetFileName();
		structFile.m_strFileType = pFile->GetFileTypeDisplayStr();
		structFile.m_strFileHash = md4str(pFile->GetFileHash());
		structFile.m_uSourceCount = pFile->GetSourceCount();
		structFile.m_dwCompleteSourceCount = pFile->GetCompleteSourceCount();
		structFile.m_uFileSize = pFile->GetFileSize();

		switch (iSortBy)
		{
			case 0:
				structFile.m_strIndex = structFile.m_strFileName;
				break;
			case 1:
				structFile.m_strIndex.Format(_T("%10u"), structFile.m_uFileSize);
				break;
			case 2:
				structFile.m_strIndex = structFile.m_strFileHash;
				break;
			case 3:
				structFile.m_strIndex.Format(_T("%09u"), structFile.m_uSourceCount);
				break;
			case 4:
				structFile.m_strIndex = structFile.m_strFileType;
				break;
			default:
				structFile.m_strIndex.Empty();
		}
		SearchFileArray->Add(structFile);
	}
}

void CSearchList::AddFileToDownloadByHash(const uchar* hash, int cat)
{
	for (POSITION pos = list.GetHeadPosition(); pos != NULL; )
	{
		CSearchFile* sf = list.GetNext(pos);
		if (!md4cmp(hash, sf->GetFileHash()))
		{
			theApp.downloadqueue->AddSearchToDownload(sf, 2, cat);
			break;
		}
	}
}

// mobilemule
CSearchFile* CSearchList::DetachNextFile(uint32 nSearchID)
{
	// the files are NOT deleted, make sure you do this if you call this function
	// find, removes and returns the searchresult with most Sources
	uint32 nHighSource = 0;
	POSITION resultpos = 0;
	for (POSITION pos = list.GetHeadPosition(); pos != NULL; )
	{
		POSITION cur_pos = pos;
		CSearchFile* cur_file = list.GetNext(pos);
		if (cur_file->GetSearchID() == nSearchID)
		{
			if (cur_file->GetIntTagValue(FT_SOURCES) >= nHighSource)
			{
				nHighSource = cur_file->GetIntTagValue(FT_SOURCES);
				resultpos = cur_pos;
			}
		}
	}
	if (resultpos == 0){
		ASSERT ( false );
		return NULL;
	}
	CSearchFile* result = list.GetAt(resultpos);
	list.RemoveAt(resultpos);
	return result;
}

bool CSearchList::AddToList(CSearchFile* toadd, bool bClientResponse)
{
	//Xman
	// SLUGFILLER: searchCatch
	CPartFile *file = theApp.downloadqueue->GetFileByID(toadd->GetFileHash());
	if (file){
		if (toadd->GetClientID() && toadd->GetClientPort()){
			// pre-filter sources which would be dropped in CPartFile::AddSources
			if (CPartFile::CanAddSource(toadd->GetClientID(), toadd->GetClientPort(), toadd->GetClientServerIP(), toadd->GetClientServerPort())){
				CSafeMemFile sources(1+4+2);
				sources.WriteUInt8(1);
				sources.WriteUInt32(toadd->GetClientID());
				sources.WriteUInt16(toadd->GetClientPort());
				sources.SeekToBegin();
				file->AddSources(&sources,toadd->GetClientServerIP(),toadd->GetClientServerPort(),false);
			}
		}
	}
	// SLUGFILLER: searchCatch

	if (!bClientResponse && !m_strResultFileType.IsEmpty() && _tcscmp(m_strResultFileType, toadd->GetFileType()) != 0)
	{
		delete toadd;
		return false;
	}

	// search for a 'parent' with same filehash and search-id as the new search result entry
	for (POSITION pos = list.GetHeadPosition(); pos != NULL; )
	{
		CSearchFile* parent = list.GetNext(pos);
		if (   parent->GetListParent() == NULL
			&& parent->GetSearchID() == toadd->GetSearchID()
			&& md4cmp(parent->GetFileHash(), toadd->GetFileHash()) == 0)
		{
			// if this parent does not yet have any child entries, create one child entry
			// which is equal to the current parent entry (needed for GUI when expanding the child list).
			if (parent->GetListChildCount() == 0)
			{
				CSearchFile* child = new CSearchFile(parent);
				child->SetListParent(parent);
				int iSources = parent->GetIntTagValue(FT_SOURCES);
				if (iSources == 0)
					iSources = 1;
				child->SetListChildCount(iSources);
				list.AddTail(child);
				parent->SetListChildCount(1);
			}

			// get the 'Availability' of the new search result entry
			UINT uAvail;
			if (bClientResponse) {
				// If this is a response from a client ("View Shared Files"), we set the "Availability" at least to 1.
				if (!toadd->GetIntTagValue(FT_SOURCES, uAvail) || uAvail==0)
					uAvail = 1;
			}
			else
				uAvail = toadd->GetIntTagValue(FT_SOURCES);

			// add the 'Availability' of the new search result entry to the total search result count for this search
			AddResultCount(parent->GetSearchID(), parent->GetFileHash(), uAvail);

			// get 'Complete Sources' of the new search result entry
			uint32 uCompleteSources = (uint32)-1;
			bool bHasCompleteSources = toadd->GetIntTagValue(FT_COMPLETE_SOURCES, uCompleteSources);

			bool bFound = false;
			if (thePrefs.GetDebugSearchResultDetailLevel() >= 1)
			{
				; // for debugging: do not merge search results
			}
			else
			{
				// check if that parent already has a child with same filename as the new search result entry
				for (POSITION pos2 = list.GetHeadPosition(); pos2 != NULL && !bFound; )
				{
					CSearchFile* child = list.GetNext(pos2);
					if (    child != toadd													// not the same object
						&& child->GetListParent() == parent								// is a child of our result (one filehash)
						&& toadd->GetFileName().CompareNoCase(child->GetFileName()) == 0)	// same name
					{
						bFound = true;

						// add properties of new search result entry to the already available child entry (with same filename)
						// ed2k: use the sum of all values, kad: use the max. values
						if (toadd->IsKademlia()) {
							if (uAvail > child->GetListChildCount())
								child->SetListChildCount(uAvail);
						}
						else {
							child->AddListChildCount(uAvail);
						}
						child->AddSources(uAvail);
						if (bHasCompleteSources)
							child->AddCompleteSources(uCompleteSources);

						break;
					}
				}
			}
			if (!bFound)
			{
				// the parent which we had found does not yet have a child with that new search result's entry name,
				// add the new entry as a new child
				//
				toadd->SetListParent(parent);
				toadd->SetListChildCount(uAvail);
				parent->AddListChildCount(1);
				list.AddHead(toadd);
			}

			// copy possible available sources from new search result entry to parent
			if (toadd->GetClientID() && toadd->GetClientPort())
			{
				if (IsValidSearchResultClientIPPort(toadd->GetClientID(), toadd->GetClientPort()))
				{
					// pre-filter sources which would be dropped in CPartFile::AddSources
					if (CPartFile::CanAddSource(toadd->GetClientID(), toadd->GetClientPort(), toadd->GetClientServerIP(), toadd->GetClientServerPort()))
					{
						CSearchFile::SClient client(toadd->GetClientID(), toadd->GetClientPort(),
													toadd->GetClientServerIP(), toadd->GetClientServerPort());
						if (parent->GetClients().Find(client) == -1)
							parent->AddClient(client);
					}
				}
				else
				{
					if (thePrefs.GetDebugServerSearchesLevel() > 1)
					{
						uint32 nIP = toadd->GetClientID();
						Debug(_T("Filtered source from search result %s:%u\n"), DbgGetClientID(nIP), toadd->GetClientPort());
					}
				}
			}

			// copy possible available servers from new search result entry to parent
			// will be used in future
			if (toadd->GetClientServerIP() && toadd->GetClientServerPort())
			{
				CSearchFile::SServer server(toadd->GetClientServerIP(), toadd->GetClientServerPort());
				int iFound = parent->GetServers().Find(server);
				if (iFound == -1) {
					server.m_uAvail = uAvail;
					parent->AddServer(server);
				}
				else
					parent->GetServerAt(iFound).m_uAvail += uAvail;
			}

			UINT uAllChildsSourceCount = 0;			// ed2k: sum of all sources, kad: the max. sources found
			UINT uAllChildsCompleteSourceCount = 0; // ed2k: sum of all sources, kad: the max. sources found
			const CSearchFile* bestEntry = NULL;
			for (POSITION pos2 = list.GetHeadPosition(); pos2 != NULL; )
			{
				const CSearchFile* child = list.GetNext(pos2);
				if (child->GetListParent() == parent)
				{
					if (parent->IsKademlia())
					{
						if (child->GetListChildCount() > uAllChildsSourceCount)
							uAllChildsSourceCount = child->GetListChildCount();
						/*if (child->GetCompleteSourceCount() > uAllChildsCompleteSourceCount) // not yet supported
							uAllChildsCompleteSourceCount = child->GetCompleteSourceCount();*/
					}
					else
					{
						uAllChildsSourceCount += child->GetListChildCount();
						uAllChildsCompleteSourceCount += child->GetCompleteSourceCount();
					}

					if (bestEntry == NULL)
						bestEntry = child;
					else if (child->GetListChildCount() > bestEntry->GetListChildCount())
						bestEntry = child;
				}
			}
			if (bestEntry)
			{
				parent->SetFileSize(bestEntry->GetFileSize());
				parent->SetFileName(bestEntry->GetFileName());
				parent->SetFileType(bestEntry->GetFileType());
				parent->ClearTags();
				parent->CopyTags(bestEntry->GetTags());
				parent->SetIntTagValue(FT_SOURCES, uAllChildsSourceCount);
				parent->SetIntTagValue(FT_COMPLETE_SOURCES, uAllChildsCompleteSourceCount);
			}

			// update parent in GUI
			if (outputwnd && !m_MobilMuleSearch)
				outputwnd->UpdateSources(parent);

			if (bFound)
				delete toadd;
			return true;
		}
	}

	// no bounded result found yet -> add as parent to list
	toadd->SetListParent(NULL);
	if (list.AddTail(toadd))
	{
		UINT tempValue = 0;
		VERIFY( m_foundFilesCount.Lookup(toadd->GetSearchID(), tempValue) );
		m_foundFilesCount.SetAt(toadd->GetSearchID(), tempValue + 1);

		// get the 'Availability' of this new search result entry
		UINT uAvail;
		if (bClientResponse) {
			// If this is a response from a client ("View Shared Files"), we set the "Availability" at least to 1.
			if (!toadd->GetIntTagValue(FT_SOURCES, uAvail) || uAvail==0)
				uAvail = 1;
			toadd->AddSources(uAvail);
		}
		else
			uAvail = toadd->GetIntTagValue(FT_SOURCES);

		// add the 'Availability' of this new search result entry to the total search result count for this search
		AddResultCount(toadd->GetSearchID(), toadd->GetFileHash(), uAvail);
	}

	if (thePrefs.GetDebugSearchResultDetailLevel() >= 1)
		toadd->SetListExpanded(true);

	// add parent in GUI
	if (outputwnd && !m_MobilMuleSearch)
		outputwnd->AddResult(toadd);

	return true;
}

CSearchFile* CSearchList::GetSearchFileByHash(const uchar* hash) const
{
	for (POSITION pos = list.GetHeadPosition(); pos != 0; )
	{
		CSearchFile* sf = list.GetNext(pos);
		if (!md4cmp(hash, sf->GetFileHash()))
			return sf;
	}
	return NULL;
}

bool CSearchList::AddNotes(Kademlia::CEntry* entry, const uchar *hash)
{
	bool flag = false;
	for (POSITION pos = list.GetHeadPosition(); pos != 0; )
	{
		CSearchFile* sf = list.GetNext(pos);
		if (!md4cmp(hash, sf->GetFileHash()))
		{
			Kademlia::CEntry* entryClone = entry->Copy();
			if(sf->AddNote(entryClone))
				flag = true;
			else
				delete entryClone;
		}
	}
	return flag;
}

void CSearchList::SetNotesSearchStatus(const uchar* pFileHash, bool bSearchRunning){
	for (POSITION pos = list.GetHeadPosition(); pos != 0; )
	{
		CSearchFile* sf = list.GetNext(pos);
		if (!md4cmp(pFileHash, sf->GetFileHash()))
		{
			sf->SetKadCommentSearchRunning(bSearchRunning);
		}
	}
}

void CSearchList::AddResultCount(uint32 nSearchID, const uchar* hash, UINT nCount)
{
	// do not count already available or downloading files for the search result limit
	if (theApp.sharedfiles->GetFileByID(hash) || theApp.downloadqueue->GetFileByID(hash))
		return;

	UINT tempValue = 0;
	VERIFY( m_foundSourcesCount.Lookup(nSearchID, tempValue) );
	m_foundSourcesCount.SetAt(nSearchID, tempValue + nCount);
}
// FIXME LARGE FILES
void CSearchList::KademliaSearchKeyword(uint32 searchID, const Kademlia::CUInt128* fileID,
										LPCTSTR name, uint64 size, LPCTSTR type, UINT numProperties, ...)
{
	va_list args;
	va_start(args, numProperties);

	EUtf8Str eStrEncode = utf8strRaw;
	CSafeMemFile* temp = new CSafeMemFile(250);
	uchar fileid[16];
	fileID->ToByteArray(fileid);
	temp->WriteHash16(fileid);

	temp->WriteUInt32(0);	// client IP
	temp->WriteUInt16(0);	// client port

	// write tag list
	UINT uFilePosTagCount = (UINT)temp->GetPosition();
	uint32 tagcount = 0;
	temp->WriteUInt32(tagcount); // dummy tag count, will be filled later

	// standard tags
	CTag tagName(FT_FILENAME, name);
	tagName.WriteTagToFile(temp, eStrEncode);
	tagcount++;

	CTag tagSize(FT_FILESIZE, size, true);
	tagSize.WriteTagToFile(temp, eStrEncode);
	tagcount++;

	if (type != NULL && type[0] != _T('\0'))
	{
		CTag tagType(FT_FILETYPE, type);
		tagType.WriteTagToFile(temp, eStrEncode);
		tagcount++;
	}

	// additional tags
	while (numProperties-- > 0)
	{
		UINT uPropType = va_arg(args, UINT);
		LPCSTR pszPropName = va_arg(args, LPCSTR);
		LPVOID pvPropValue = va_arg(args, LPVOID);
		if (uPropType == 2 /*TAGTYPE_STRING*/)
		{
			if ((LPCTSTR)pvPropValue != NULL && ((LPCTSTR)pvPropValue)[0] != _T('\0'))
			{
				if (strlen(pszPropName) == 1)
				{
					CTag tagProp((uint8)*pszPropName, (LPCTSTR)pvPropValue);
					tagProp.WriteTagToFile(temp, eStrEncode);
				}
				else
				{
					CTag tagProp(pszPropName, (LPCTSTR)pvPropValue);
					tagProp.WriteTagToFile(temp, eStrEncode);
				}
				tagcount++;
			}
		}
		else if (uPropType == 3 /*TAGTYPE_UINT32*/)
		{
			if ((uint32)pvPropValue != 0)
			{
				CTag tagProp(pszPropName, (uint32)pvPropValue);
				tagProp.WriteTagToFile(temp, eStrEncode);
				tagcount++;
			}
		}
		else
		{
			ASSERT(0);
		}
	}
	va_end(args);
	temp->Seek(uFilePosTagCount, SEEK_SET);
	temp->WriteUInt32(tagcount);

	temp->SeekToBegin();
	CSearchFile* tempFile = new CSearchFile(temp, eStrEncode == utf8strRaw, searchID, 0, 0, 0, true);
	AddToList(tempFile);

	delete temp;
}
