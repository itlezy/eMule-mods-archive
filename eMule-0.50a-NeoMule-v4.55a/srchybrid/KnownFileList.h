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
#include "MapKey.h"

class CKnownFile;
// NEO: FCFG - [FileConfiguration] -- Xanatos -->
class CFileDataIO;
class CKnownPreferences;
// NEO: FCFG END <-- Xanatos --
typedef CMap<CCKey,const CCKey&,CKnownFile*,CKnownFile*> CKnownFilesMap;
typedef CMap<CSKey,const CSKey&,int,int> CancelledFilesMap;

class CKnownFileList 
{
	friend class CSharedFilesWnd;
	friend class CStatisticFile;
	friend class CSharedFilesCtrl; // NEO: AKF - [AllKnownFiles] <-- Xanatos --
public:
	CKnownFileList();
	~CKnownFileList();

	bool	SafeAddKFile(CKnownFile* toadd);
	bool	Init();
	void	Save();
	void	Clear();
	void	Process();
	void	Publish(); // NEO: XCk - [KnownComments] <-- Xanatos --

	// NEO: FCFG - [FileConfiguration] -- Xanatos -->
	bool	LoadKnownPreferences();
	bool	SaveKnownPreferences();
	bool	ClearPreferencesEntry(CFileDataIO* file);
	// NEO: FCFG END <-- Xanatos --
	// NEO: NPT - [NeoPartTraffic] -- Xanatos -->
	bool	IsPartTrafficLoaded() { return m_bPartTrafficLoaded; }
	bool	SavePartTraffic();
	bool	LoadPartTraffic();
	bool	ClearTrafficEntry(CFileDataIO* file);
	// NEO: NPT END <-- Xanatos --

	// NEO: XCs - [SaveComments] -- Xanatos -->
	bool IsCommentsLoaded() { return m_bCommentsLoaded; }
	bool LoadComments();
	bool ClearCommentsEntry(CFileDataIO* file);
	bool SaveComments();
	// NEO: XCs END <-- Xanatos --

	CKnownFile* FindKnownFile(LPCTSTR filename, uint32 date, uint64 size) const;
	CKnownFile* FindKnownFileByID(const uchar* hash) const;
	CKnownFile* FindKnownFileByPath(const CString& sFilePath) const;
	CKnownFile*	GetFileByIndex(int index); // NEO: XCk - [KnownComments] <-- Xanatos --
	bool	IsKnownFile(const CKnownFile* file) const;
	bool	IsFilePtrInList(const CKnownFile* file) const;

	void	AddCancelledFileID(const uchar* hash);
	bool	IsCancelledFileByID(const uchar* hash) const;

	const CKnownFilesMap& GetKnownFiles() const { return m_Files_map; }
	void	CopyKnownFileMap(CMap<CCKey,const CCKey&,CKnownFile*,CKnownFile*> &Files_Map);

	// NEO: AKF - [AllKnownFiles] -- Xanatos -->
	bool	RemoveFile(CKnownFile* toRemove);
	int		GetCount()	{return m_Files_map.GetCount(); }
	// NEO: AKF END <-- Xanatos --

	// NEO: NSC - [NeoSharedCategories] -- Xanatos -->
	void	ResetCatParts(UINT cat);
	void	ShiftCatParts(UINT cat); 
	void	MoveCat(UINT from, uint8 to);
	// NEO: NSC END <-- Xanatos --

	// NEO: FCFG - [FileConfiguration] -- Xanatos -->
	void	UpdateKnownPrefs(CKnownPreferences* KnownPrefs, UINT cat);
	static	void UpdateKnownPrefs(CKnownFile* cur_file, CKnownPreferences* KnownPrefs, UINT cat);
	// NEO: FCFG END <-- Xanatos --

	// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
	int CheckAlreadyDownloadedFile(const uchar* hash, CString filename=_T(""), CArray<CKnownFile*,CKnownFile*> *files = NULL);
	bool CheckAlreadyDownloadedFileQuestion(const uchar* hash, CString filename);
	// NEO: NXC END <-- Xanatos --

private:
	bool	LoadKnownFiles();
	bool	LoadCancelledFiles();

	uint16 	requested;
	uint16 	accepted;
	uint64 	transferred;
	uint32 	m_nLastSaved;
	bool	m_bPartTrafficLoaded; // NEO: NPT - [NeoPartTraffic] <-- Xanatos --
	bool	m_bCommentsLoaded; // NEO: XCs - [SaveComments] <-- Xanatos --
	int		m_currFileNotes; // NEO: XCk - [KnownComments] <-- Xanatos --
	CKnownFilesMap		m_Files_map;
	CancelledFilesMap	m_mapCancelledFiles;
	uint32	m_dwCancelledFilesSeed;
};
