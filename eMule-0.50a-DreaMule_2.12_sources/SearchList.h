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
#pragma once
#include "KnownFile.h"
#include "SearchFile.h"
#include "QArray.h"

enum ESearchType;

typedef struct
{
	CString	m_strFileName;
	CString	m_strFileType;
	CString	m_strFileHash;
	CString	m_strIndex;
	uint64	m_uFileSize;
	uint32	m_uSourceCount;
	uint32	m_dwCompleteSourceCount;
} SearchFileStruct;


class CFileDataIO;
class CAbstractFile;


__inline bool __stdcall operator==(const CSearchFile::SServer& s1, const CSearchFile::SServer& s2)
{
	return s1.m_nIP==s2.m_nIP && s1.m_nPort==s2.m_nPort;
}

__inline bool __stdcall operator==(const CSearchFile::SClient& c1, const CSearchFile::SClient& c2)
{
	return c1.m_nIP==c2.m_nIP && c1.m_nPort==c2.m_nPort &&
		   c1.m_nServerIP==c2.m_nServerIP && c1.m_nServerPort==c2.m_nServerPort;
}


class CSearchList
{
	friend class CSearchListCtrl;
public:
	CSearchList();
	~CSearchList();

	void	Clear();
	void	NewSearch(CSearchListCtrl* in_wnd, CStringA strResultFileType, uint32 nSearchID, ESearchType eType, bool bMobilMuleSearch = false);
	UINT	ProcessSearchAnswer(const uchar* packet, uint32 size, CUpDownClient* Sender, bool* pbMoreResultsAvailable, LPCTSTR pszDirectory = NULL);
	UINT	ProcessSearchAnswer(const uchar* packet, uint32 size, bool bOptUTF8, uint32 nServerIP, uint16 nServerPort, bool* pbMoreResultsAvailable);
	UINT	ProcessUDPSearchAnswer(CFileDataIO& packet, bool bOptUTF8, uint32 nServerIP, uint16 nServerPort);
	UINT	GetED2KResultCount() const;
	UINT	GetResultCount(uint32 nSearchID) const;
	void	AddResultCount(uint32 nSearchID, const uchar* hash, UINT nCount);
	void	SetOutputWnd(CSearchListCtrl* in_wnd) { outputwnd = in_wnd; }
	void	RemoveResults(uint32 nSearchID);
	void	RemoveResult(CSearchFile* todel);
	void	ShowResults(uint32 nSearchID);
	void	GetWebList(CQArray<SearchFileStruct, SearchFileStruct> *SearchFileArray, int iSortBy) const;
	void	AddFileToDownloadByHash(const uchar* hash)		{AddFileToDownloadByHash(hash,0);}
	void	AddFileToDownloadByHash(const uchar* hash, int cat);
	bool	AddToList(CSearchFile* toadd, bool bClientResponse = false);
	CSearchFile* GetSearchFileByHash(const uchar* hash) const;
	void	KademliaSearchKeyword(uint32 searchID, const Kademlia::CUInt128* pfileID, LPCTSTR name, uint64 size, LPCTSTR type, UINT numProperties, ...);
	bool	AddNotes(Kademlia::CEntry* entry, const uchar* hash);
	void	SetNotesSearchStatus(const uchar* pFileHash, bool bSearchRunning);

	UINT GetFoundFiles(uint32 searchID) const {
		UINT returnVal = 0;
		VERIFY( m_foundFilesCount.Lookup(searchID, returnVal) );
		return returnVal;
	}
	// mobilemule
	CSearchFile*	DetachNextFile(uint32 nSearchID);

private:
	CTypedPtrList<CPtrList, CSearchFile*> list;
	CMap<uint32, uint32, UINT, UINT> m_foundFilesCount;
	CMap<uint32, uint32, UINT, UINT> m_foundSourcesCount;

	CSearchListCtrl* outputwnd;
	CString m_strResultFileType;
	uint32	m_nCurED2KSearchID;
	bool	m_MobilMuleSearch;
};
