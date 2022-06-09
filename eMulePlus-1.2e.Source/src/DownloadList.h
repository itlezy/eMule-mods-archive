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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	MOREVIT - Note: The functionality in this class should really be integrated into the download queue class,
//		but it's tough enough separating it out of the list control without modifying two classes at once.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "Loggable.h"
#include "MuleCtrlItem.h"
#include "Category.h"

#pragma warning(push)
#pragma warning(disable:4702) // unreachable code
#include <map>
#include <hash_map>
#include <vector>
#pragma warning(pop)

using namespace std;
#ifndef VS2002
	using namespace stdext;
#endif

class CPartFile;
class CUpDownClient;

class CDownloadList : public CLoggable
{
public:
	friend class CDownloadListCtrl;

	typedef vector<CPartFile*>					PartFileVector;

protected:
	CDownloadListCtrl						   *m_pctlDownloadList;

	typedef map<CPartFile*, CPartFileDLItem*>	PartFileMap;
	typedef PartFileMap::iterator				PFIter;
	typedef pair<PFIter,PFIter>					PFRange;
	typedef const pair<PFIter,bool>				FileInserted;

	typedef hash_multimap<CUpDownClient*, CSourceDLItem*>		SourceMap;
	typedef SourceMap::iterator					SourceIter;
	typedef pair<SourceIter,SourceIter>			SourceRange;
	typedef const pair<SourceIter,bool>			SourceInserted;

	typedef vector<CPartFileDLItem*>			PartFileItemVector;
	typedef vector<CSourceDLItem*>				SourceItemVector;

	PartFileMap									m_mapFiles;
	SourceMap									m_mapSources;	// non-ownership - source items owned by part items
	PartFileItemVector						   *m_pvecDirtyFiles;
	SourceItemVector						   *m_pvecDirtySources;

public:
						CDownloadList();
					   ~CDownloadList();

	void				SetDownloadListCtrl(CDownloadListCtrl *pctlList) { m_pctlDownloadList = pctlList; }
	void				AddFile(CPartFile *pPartFile);
	void				RemoveFile(CPartFile *pPartFile);
	void				UpdateFile(CPartFile *pPartFile);
	void				RemoveAllFiles();
	void				AddDirtyFile(CPartFileDLItem *pFileItem);
	PartFileItemVector *GetDirtyFiles();
	void				AddDirtySource(CSourceDLItem *pSourceItem);
	SourceItemVector   *GetDirtySources();
	void				AddSource(CPartFile *pParentFile,CUpDownClient *pSource,bool bSourceNotAvailable);
	void				RemoveSource(CUpDownClient *pSource,CPartFile *pParentFile = NULL);
	void				UpdateSource(CUpDownClient *pSource,CPartFile *pParentFile = NULL);
	PartFileVector	   *GetFiles();
	PartFileItemVector *GetFileItems();

	bool				IsValidIterator(const PFIter &itFile) { return itFile != m_mapFiles.end(); }
	bool				IsValidIterator(const SourceIter &itSource) { return itSource != m_mapSources.end(); }
	CPartFileDLItem	   *GetFileItem(const PFIter &itFile) { return itFile->second; }
	CSourceDLItem	   *GetSourceItem(const SourceIter &itSource) { return itSource->second; }
	PFIter				FindFileItem(CPartFile *pPartFile) {	return(m_mapFiles.find(pPartFile)); }
	SourceRange			FindSourceItems(CUpDownClient *pSource);
	SourceItemVector   *GetSourceItems(CUpDownClient *pSource = NULL);
	FileInserted		InsertFileItem(CPartFile *pPartFile, CPartFileDLItem *pFileItem)
							{ return m_mapFiles.insert(make_pair(pPartFile, pFileItem)); }
	SourceIter			InsertSourceItem(CUpDownClient *pSource,CSourceDLItem *pSourceItem)
							{ return m_mapSources.insert(make_pair(pSource, pSourceItem)); }
	void				RemoveSourceItem(CSourceDLItem *pSourceItem);
	void				ClearCompleted(EnumCategories eCategoryID = CAT_NONE); // CAT_NONE means use GUI current cat.
	void				ClearCompleted(CPartFile *pPartFile);
	void				ClearCompleted(const uchar *pFileHash = NULL);
	CString				GetPartFilesStatusString();
};
