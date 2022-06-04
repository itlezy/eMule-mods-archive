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
#include "SHAHashset.h"

class CKnownFile;
typedef CMap<CCKey,const CCKey&,CKnownFile*,CKnownFile*> CKnownFilesMap;
typedef CMap<CSKey,const CSKey&,int,int> CancelledFilesMap;
typedef CMap<CAICHHash, const CAICHHash&, const CKnownFile*, const CKnownFile*> KnonwFilesByAICHMap;

// ==> Threaded Known Files Saving [Stulle] - Stulle
class CSaveKnownThread : public CWinThread
{
public:
    CSaveKnownThread(void);
    ~CSaveKnownThread(void);

    void EndThread();
    void Pause(bool paused);

private:
    static UINT RunProc(LPVOID pParam);
    UINT RunInternal();

    CEvent* threadEndedEvent;
    CEvent* pauseEvent;
	volatile bool bDoRun;
};
// <== Threaded Known Files Saving [Stulle] - Stulle

class CKnownFileList 
{
	friend class CFileDetailDlgStatistics;
	friend class CStatisticFile;
public:
	CKnownFileList();
	~CKnownFileList();

	bool	SafeAddKFile(CKnownFile* toadd);
	bool	Init();
	void	Save();
	void	Clear();
	void	Process();

	CKnownFile* FindKnownFile(LPCTSTR filename, uint32 date, uint64 size) const;
	CKnownFile* FindKnownFileByID(const uchar* hash) const;
	CKnownFile* FindKnownFileByPath(const CString& sFilePath) const;
	bool	IsKnownFile(const CKnownFile* file) const;
	bool	IsFilePtrInList(const CKnownFile* file) const;

	void	AddCancelledFileID(const uchar* hash);
	bool	IsCancelledFileByID(const uchar* hash) const;

	const CKnownFilesMap& GetKnownFiles() const { return m_Files_map; }
	void	CopyKnownFileMap(CMap<CCKey,const CCKey&,CKnownFile*,CKnownFile*> &Files_Map);

	bool	ShouldPurgeAICHHashset(const CAICHHash& rAICHHash) const;
	void	AICHHashChanged(const CAICHHash* pOldAICHHash, const CAICHHash& rNewAICHHash, CKnownFile* pFile);

	uint32 	m_nRequestedTotal;
	uint32 	m_nAcceptedTotal;
	uint64 	m_nTransferredTotal;

	//Xman [MoNKi: -Check already downloaded files-]
	int CheckAlreadyDownloadedFile(const uchar* hash, CString filename=_T(""), CArray<CKnownFile*,CKnownFile*> *files = NULL);
	bool CheckAlreadyDownloadedFileQuestion(const uchar* hash, CString filename);
	//Xman end

	//Xman [MoNKi: -Downloaded History-]
	CKnownFilesMap* GetDownloadedFiles();
	bool RemoveKnownFile(CKnownFile *toRemove);
	void ClearHistory();
	bool	bReloadHistory; //Fafner: possible exception in history - 070626
	//Xman end

private:
	bool	LoadKnownFiles();
	bool	LoadCancelledFiles();

	uint16 	requested;
	uint16 	accepted;
	uint64 	transferred;
	uint32 	m_nLastSaved;
	CKnownFilesMap		m_Files_map;
	CancelledFilesMap	m_mapCancelledFiles;
	// for faster access, map of files indexed by AICH-hash, not garantueed to be complete at this point (!)
	// (files which got AICH hashed later will not be added yet, because we don't need them, make sure to change this if needed)
	KnonwFilesByAICHMap m_mapKnownFilesByAICH;
	uint32	m_dwCancelledFilesSeed;

public:
	uint32	GetTotalRequested() {return requested;} // push rare file - Stulle

	// ==> Threaded Known Files Saving [Stulle] - Stulle
	void SaveKnown(bool bStart = true);
	CSaveKnownThread* m_SaveKnownThread;
protected:
	bool m_bSaveAgain;
	// <== Threaded Known Files Saving [Stulle] - Stulle
};
