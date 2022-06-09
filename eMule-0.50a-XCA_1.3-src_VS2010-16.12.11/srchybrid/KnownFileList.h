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
//#include <vector>
#include "MapKey.h"
#include "SHAHashset.h"

class CKnownFile;
#ifdef REPLACE_ATLMAP
typedef unordered_map<CSKey, int, CSKey, CSKey> CancelledFilesMap;
typedef unordered_map<CAICHHash, const CKnownFile*, CAICHHash, CAICHHash> KnonwFilesByAICHMap;
#else
typedef CAtlMap<CSKey,int,CSKeyTraits> CancelledFilesMap;
typedef CAtlMap<CAICHHash, const CKnownFile*, CAICHHashTraits> KnonwFilesByAICHMap;
#endif
//typedef std::vector<CKnownFile*> CKnownFilesArray;
class CKnownFileList : public Poco::Thread
{
	friend class CFileDetailDlgStatistics;
	friend class CStatisticFile;
public:
	CKnownFileList();
	virtual ~CKnownFileList();

	bool	SafeAddKFile(CKnownFile* toadd);
	bool	Init();
	void	Save();
	void	Clear();
	void	Process();
	// ==> Threaded Known Files Saving [Stulle] - Stulle
	virtual void run();
	// <== Threaded Known Files Saving [Stulle] - Stulle

	CKnownFile* FindKnownFile(LPCTSTR filename, uint64 date, uint64 size) const;// X: [64T] - [64BitTime]
	CKnownFile* FindKnownFileByID(const uchar* hash) const;
	CKnownFile* FindKnownFileByPath(const CString& sFilePath) const;
	bool	IsKnownFile(const CKnownFile* file) const;
	bool	IsFilePtrInList(const CKnownFile* file) const;

	void	AddCancelledFileID(const uchar* hash);
	bool	IsCancelledFileByID(const uchar* hash) const;

	const CKnownFilesMap& GetKnownFiles() const { return m_Files_map; }
	//void	CopyKnownFileMap(CAtlMap<CCKey,CKnownFile*,CCKeyTraits> &Files_Map);

	uint_ptr	ShouldPurgeAICHHashset(const CAICHHash& rAICHHash) const;
	void	AICHHashChanged(const CAICHHash* pOldAICHHash, const CAICHHash& rNewAICHHash, CKnownFile* pFile);

	uint32 	m_nRequestedTotal;
	uint32 	m_nAcceptedTotal;
	uint64 	m_nTransferredTotal;

	//Xman [MoNKi: -Check already downloaded files-]
	sint_ptr CheckAlreadyDownloadedFile(const uchar* hash, CString filename=_T(""), CAtlArray<CKnownFile*> *files = NULL) const;
	bool CheckAlreadyDownloadedFileQuestion(const uchar* hash, CString filename) const;
	//Xman end

	//Xman [MoNKi: -Downloaded History-]
	//CKnownFilesArray* GetDownloadedFiles();
	bool RemoveKnownFile(CKnownFile *toRemove);
	void ClearHistory();
	//Xman end
private:
	bool	LoadKnownFiles();
	bool	LoadCancelledFiles();

	uint16 	requested;
	uint16 	accepted;
	uint32 	m_nLastSaved;
	uint64 	transferred;
	CKnownFilesMap		m_Files_map;
	CancelledFilesMap	m_mapCancelledFiles;
	// for faster access, map of files indexed by AICH-hash, not garantueed to be complete at this point (!)
	// (files which got AICH hashed later will not be added yet, because we don't need them, make sure to change this if needed)
	KnonwFilesByAICHMap m_mapKnownFilesByAICH;
	uint32	m_dwCancelledFilesSeed;
};
