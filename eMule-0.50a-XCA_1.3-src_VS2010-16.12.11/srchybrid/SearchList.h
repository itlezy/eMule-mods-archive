//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#pragma once
#include "KnownFile.h"
#include "SearchFile.h"
#include <vector>
#include "Mapkey.h"

enum ESearchType;

typedef struct
{
	CString sortIndex[3];//m_strFileName, m_strFileType, m_strFileHash
	uint64	m_uFileSize;
	uint32	m_uSourceCount;
	uint32	m_dwCompleteSourceCount;
} SearchFileStruct;

typedef CAtlList<CSearchFile*> SearchList;

typedef struct {
	uint32 m_nSearchID;
	SearchList m_listSearchFiles;
} SearchListsStruct;

typedef struct {
	uint32	m_nResults;
	uint32	m_nSpamResults;
} UDPServerRecord;



class CFileDataIO;
class CAbstractFile;
struct SSearchTerm;

class CSearchList
{
	friend class CSearchListCtrl;
public:
	CSearchList();
	~CSearchList();

	void	Clear();
	void	NewSearch(CSearchListCtrl* in_wnd, CStringA strResultFileType, uint32 nSearchID, ESearchType eType, CString strSearchExpression);
	UINT	ProcessSearchAnswer(const uchar* packet, uint32 size, CUpDownClient* Sender, bool* pbMoreResultsAvailable, LPCTSTR pszDirectory = NULL);
	UINT	ProcessSearchAnswer(const uchar* packet, uint32 size, bool bOptUTF8, uint32 nServerIP, uint16 nServerPort, bool* pbMoreResultsAvailable);
	UINT	ProcessUDPSearchAnswer(CFileDataIO& packet, bool bOptUTF8, uint32 nServerIP, uint16 nServerPort);
	UINT	GetED2KResultCount() const;
	UINT	GetResultCount(uint32 nSearchID) const;
	void	AddResultCount(uint32 nSearchID, const uchar* hash, UINT nCount, bool bSpam);
	void	SetOutputWnd(CSearchListCtrl* in_wnd) { outputwnd = in_wnd; }
	void	RemoveResults(uint32 nSearchID);
	void	RemoveResult(CSearchFile* todel);
	void	ShowResults(uint32 nSearchID);
	void	GetWebList(std::vector<SearchFileStruct>&SearchFileArray) const;
	void	AddFileToDownloadByHash(const uchar* hash)		{AddFileToDownloadByHash(hash,0);}
	void	AddFileToDownloadByHash(const uchar* hash, int cat);
	bool	AddToList(CSearchFile* toadd, bool bClientResponse = false, uint32 dwFromUDPServerIP = 0);
	CSearchFile* GetSearchFileByHash(const uchar* hash) const;
	void	KademliaSearchKeyword(uint32 searchID, const Kademlia::CUInt128* pfileID, LPCTSTR name, uint64 size, LPCTSTR type, UINT uKadPublishInfo, CAtlArray<CAICHHash>& raAICHHashs, CAtlArray<uint8>& raAICHHashPopularity, SSearchTerm* pQueriedSearchTerm, UINT numProperties, ...);
	bool	AddNotes(Kademlia::CEntry* entry, const uchar* hash);
	void	SetNotesSearchStatus(const uchar* pFileHash, bool bSearchRunning);
	void	SentUDPRequestNotification(uint32 nSearchID, uint32 dwServerIP);

	void	StoreSearches();
	void	LoadSearches();
	
	void	DoSpamRating(CSearchFile* pSearchFile, bool bIsClientFile = false, bool bMarkAsNoSpam = false, bool bRecalculateAll = false, bool bUpdateAll = false, uint32 dwFromUDPServerIP = 0);
	void	MarkFileAsSpam(CSearchFile* pSpamFile, bool bRecalculateAll = false, bool bUpdate = false);
	void	MarkFileAsNotSpam(CSearchFile* pSpamFile, bool bRecalculateAll = false, bool bUpdate = false)	{ DoSpamRating(pSpamFile, false, true, bRecalculateAll, bUpdate); } 
	void	RecalculateSpamRatings(uint32 nSearchID, bool bExpectHigher, bool bExpectLower, bool bUpdate);
	void	SaveSpamFilter();

	UINT GetFoundFiles(uint32 searchID) const {
#ifdef REPLACE_ATLMAP
		unordered_map<uint32, UINT>::const_iterator it = m_foundFilesCount.find(searchID);
		ASSERT(it != m_foundFilesCount.end());
		return it->second;
#else
		UINT returnVal = 0;
		VERIFY( m_foundFilesCount.Lookup(searchID, returnVal) );
		return returnVal;
#endif
	}
	// mobilemule
	CSearchFile*	DetachNextFile(uint32 nSearchID);
protected:
	SearchList*		GetSearchListForID(uint32 nSearchID);
	uint32			GetSpamFilenameRatings(const CSearchFile* pSearchFile, bool bMarkAsNoSpam);
	void			LoadSpamFilter();



private:
	CAtlList<SearchListsStruct*> m_listFileLists;
#ifdef REPLACE_ATLMAP
	unordered_map<uint32, UINT> m_foundFilesCount;
	unordered_map<uint32, UINT> m_foundSourcesCount;
	unordered_map<uint32, UINT> m_ReceivedUDPAnswersCount;
	unordered_map<uint32, UINT> m_RequestedUDPAnswersCount;
#else
	CAtlMap<uint32, UINT> m_foundFilesCount;
	CAtlMap<uint32, UINT> m_foundSourcesCount;
	CAtlMap<uint32, UINT> m_ReceivedUDPAnswersCount;
	CAtlMap<uint32, UINT> m_RequestedUDPAnswersCount;
#endif
	CSearchListCtrl* outputwnd;
	CString			m_strResultFileType;
	
	uint32			m_nCurED2KSearchID;

	// spamfilter
	CAtlArray<CString>							m_astrSpamCheckCurSearchExp;
	CAtlArray<CString>							m_astrKnownSpamNames;
	CAtlArray<CString>							m_astrKnownSimilarSpamNames;
#ifdef REPLACE_ATLMAP
	unordered_set<uint32>		m_mapKnownSpamServerIPs;
	unordered_set<uint32>		m_mapKnownSpamSourcesIPs;
	unordered_map<uint32, UDPServerRecord*>	m_aUDPServerRecords;
	typedef unordered_map<CSKey, bool, CSKey, CSKey> CKnownSpamHashsMap;
#else
	CAtlMap<uint32, bool>		m_mapKnownSpamServerIPs;
	CAtlMap<uint32, bool>		m_mapKnownSpamSourcesIPs;
	CAtlMap<uint32, UDPServerRecord*>	m_aUDPServerRecords;
	typedef CAtlMap<CSKey, bool, CSKeyTraits> CKnownSpamHashsMap;
#endif
	CKnownSpamHashsMap	m_mapKnownSpamHashs;
	CAtlArray<uint64>						m_aui64KnownSpamSizes;
	CAtlArray<uint32>						m_aCurED2KSentRequestsIPs;
	CAtlArray<uint32>						m_aCurED2KSentReceivedIPs;
	bool									m_bSpamFilterLoaded;
};
