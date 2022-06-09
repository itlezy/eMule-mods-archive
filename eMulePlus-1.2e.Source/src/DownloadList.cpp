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
#include "opcodes.h"
#include "DownloadList.h"
#include "DownloadListCtrl.h"
#include "emule.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDownloadList::CDownloadList()
{
	m_pvecDirtyFiles = new PartFileItemVector();
	m_pvecDirtySources = new SourceItemVector();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDownloadList::~CDownloadList()
{
	RemoveAllFiles();

	delete m_pvecDirtyFiles;
	delete m_pvecDirtySources;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	AddFile() adds part file 'pPartFile' to the download list and notifies the download list ctrl of the addition.
void CDownloadList::AddFile(CPartFile *pPartFile)
{
	EMULE_TRY

	if (pPartFile == NULL)
		return;

//	Create new Item
	CPartFileDLItem	   *pFileItem = new CPartFileDLItem(pPartFile);

//	The same file shall be added only once
	ASSERT(!IsValidIterator(FindFileItem(pPartFile)));
	InsertFileItem(pPartFile, pFileItem);

	m_pctlDownloadList->AddFileItem(pFileItem);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadList::RemoveFile(CPartFile *pPartFile)
{
	if (pPartFile != NULL)
	{
		PFIter		itFile = FindFileItem(pPartFile);

	//	If it was found...
		if (IsValidIterator(itFile))
		{
			CPartFileDLItem		*pFileItem = GetFileItem(itFile);

		//	Remove the file item from the file map
			m_mapFiles.erase(itFile);

		//
		//	If it's in the dirty list, remove it from there too
			PartFileItemVector::iterator	it = ::find(m_pvecDirtyFiles->begin(),m_pvecDirtyFiles->end(),pFileItem);

			if (it != m_pvecDirtyFiles->end())
			{
				m_pvecDirtyFiles->erase(it);
			}

		//	NOTE: This call must remain synchronous as the item is destroyed immediately after.
			m_pctlDownloadList->RemoveFileItem(pFileItem);

		//	Destroy the file item
			delete pFileItem;
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadList::UpdateFile(CPartFile *pPartFile)
{
	if (pPartFile != NULL)
	{
		PFIter		itFileItem = FindFileItem(pPartFile);

	//	For each file item associated with 'pFile'...
		if (IsValidIterator(itFileItem))
		{
			CPartFileDLItem  *pFileItem = GetFileItem(itFileItem);

			if (pFileItem != NULL)
			{
				AddDirtyFile(pFileItem);
			}
		}
		m_pctlDownloadList->PostRefreshMessage();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadList::AddDirtyFile(CPartFileDLItem *pFileItem)
{
//	If 'pFileItem' is not already in the dirty file list...
	if (::find(m_pvecDirtyFiles->begin(),m_pvecDirtyFiles->end(),pFileItem) == m_pvecDirtyFiles->end())
	{
		m_pvecDirtyFiles->push_back(pFileItem);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetDirtyFiles() detaches the current dirty file list and returns it to the caller (who's responsible
//		for destroying it). This allows the download list control to update without locking down the download list.
CDownloadList::PartFileItemVector *CDownloadList::GetDirtyFiles()
{
	PartFileItemVector		*pDirtyFiles = NULL;

	if (!m_pvecDirtyFiles->empty())
	{
		pDirtyFiles = m_pvecDirtyFiles;
		m_pvecDirtyFiles = new PartFileItemVector();
	}

	return pDirtyFiles;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	AddSource() adds Source 'pSource' to the download list and notifies the download list ctrl of the addition.
void CDownloadList::AddSource(CPartFile *pParentFile,CUpDownClient *pSource,bool bSourceNotAvailable)
{
	EMULE_TRY

#ifdef OLD_SOCKETS_ENABLED
	if (pParentFile == NULL || pSource == NULL)
		return;
//
//	Find out if there is already an item for the spec'd source and file. If the source is available,
//		mark any other items unavailable.
	CSourceDLItem	   *pSourceItem = NULL;

	SourceRange			range = FindSourceItems(pSource);

	for (SourceIter itSource = range.first; itSource != range.second; itSource++)
	{
		CSourceDLItem		*pSourceItem2 = itSource->second;

		if (pSourceItem2->GetParentFile() == pParentFile)
		{
			pSourceItem = pSourceItem2;
		}
		else
		{
		//	A Source can only be "Available" for one file at a time
			if (bSourceNotAvailable == false)
				pSourceItem2->SetAvailability(false);
		}
	}

	CPartFileDLItem		*pParentFileItem = NULL;

//	If there's no existing item for 'pSource', create one
	if (pSourceItem == NULL)
	{
	//	Find the part file item we want to add it to
		PFIter		itFileItem = FindFileItem(pParentFile);

	//	If it doesn't exist...
		if (!IsValidIterator(itFileItem))
			return;
		pParentFileItem = GetFileItem(itFileItem);
		pSourceItem = pParentFileItem->CreateSourceItem(pSource,!bSourceNotAvailable);
		InsertSourceItem(pSource,pSourceItem);
	}
//	If there's already an item for 'pSource'...
	else
	{
		pSourceItem->SetAvailability(!bSourceNotAvailable);
		pSourceItem->ResetUpdateTimer();
		return;
	}
#endif //OLD_SOCKETS_ENABLED

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	RemoveSource() removes and destroys the source item for source 'pSource' belonging to
//		part file 'pParentFile'. If 'pParentFile' is NULL then all source items for source 'pSource'
//		are deleted. If an error occurs, the method does nothing.
void CDownloadList::RemoveSource(CUpDownClient *pSource,CPartFile *pParentFile/*=NULL*/)
{
	EMULE_TRY

	if (pSource != NULL)
	{
	//	If the InfoList is displaying this source and it is being completely removed...
		if ( pParentFile == NULL
		  && g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.GetType() == INFOLISTTYPE_SOURCE
		  && g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.GetClient() == pSource
		  && g_App.m_pMDlg->IsRunning())
		{
		//	Set it to display nothing
			g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.SetList(INFOLISTTYPE_NONE);
		}

		CSourceDLItem	   *pSourceItem = NULL;
		SourceRange			range = FindSourceItems(pSource);

	//	For each source item associated with 'pSource' in the multi-map...
		for (SourceIter itSource = range.first; itSource != range.second; )
		{
			pSourceItem = GetSourceItem(itSource);

		//	If no parent file was spec'd or this item belongs to the spec'd parent file...
			if (pSourceItem != NULL && (pParentFile == NULL || pSourceItem->GetParentFile() == pParentFile))
			{
				itSource = m_mapSources.erase(itSource);

			//
			//	If it's in the dirty list, remove it from there too
				SourceItemVector::iterator	it = ::find(m_pvecDirtySources->begin(),m_pvecDirtySources->end(),pSourceItem);

				if (it != m_pvecDirtySources->end())
				{
					m_pvecDirtySources->erase(it);
				}

			//	Remove the source item from the download list ctrl
				m_pctlDownloadList->RemoveSourceItem(pSourceItem);

				CPartFileDLItem		*pParentFileItem = pSourceItem->GetParentFileItem();

				pParentFileItem->DeleteSourceItem(pSourceItem);

			//	If a parent file was spec'd we're done, otherwise continue for the rest of the items for the source.
				if (pParentFile != NULL)
					break;
			}
			else
			{
				itSource++;
			}
		}
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadList::UpdateSource(CUpDownClient *pSource, CPartFile *pParentFile/*=NULL*/)
{
	if (pSource != NULL)
	{
		SourceRange		range = FindSourceItems(pSource);

	//	For each source item associated with 'pSource'...
		for (SourceIter itSource = range.first; itSource != range.second; itSource++)
		{
			CSourceDLItem  *pSourceItem = GetSourceItem(itSource);

			if (pSourceItem != NULL && (pParentFile == NULL || pParentFile == pSourceItem->GetParentFile()))
			{
				AddDirtySource(pSourceItem);
				if (pParentFile != NULL)
					break;
			}
		}
		m_pctlDownloadList->PostRefreshMessage();
		g_App.m_pMDlg->m_wndTransfer.m_ctlClientList.PostUniqueMessage(WM_CL_REFRESH);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadList::AddDirtySource(CSourceDLItem *pSourceItem)
{
//	If 'pSourceItem' is not already in the dirty source list...
	if (::find(m_pvecDirtySources->begin(),m_pvecDirtySources->end(),pSourceItem) == m_pvecDirtySources->end())
	{
		m_pvecDirtySources->push_back(pSourceItem);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetDirtySources() detaches the current dirty source list and returns it to the caller (who's responsible
//		for destroying it). This allows the download list control to update without locking down the download list.
CDownloadList::SourceItemVector *CDownloadList::GetDirtySources()
{
	SourceItemVector		*pDirtySources = NULL;

	if (!m_pvecDirtySources->empty())
	{
		pDirtySources = m_pvecDirtySources;
		m_pvecDirtySources = new SourceItemVector();
	}

	return pDirtySources;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	RemoveAllFiles() removes and destroys all part file items in the window.
void CDownloadList::RemoveAllFiles()
{
	while (m_mapFiles.empty() == false)
	{
		delete GetFileItem(m_mapFiles.begin());
		m_mapFiles.erase(m_mapFiles.begin());
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDownloadList::SourceRange CDownloadList::FindSourceItems(CUpDownClient *pSource)
{
	return m_mapSources.equal_range(pSource);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadList::ClearCompleted(EnumCategories eCategoryID/* = CAT_NONE*/)
{
	EMULE_TRY

	EnumCategories		eCurCat = m_pctlDownloadList->GetCurTabCat();

//	Search for completed file(s)
	for (PFIter itFile = m_mapFiles.begin(); itFile != m_mapFiles.end();)
	{
		CPartFileDLItem		*pFileItem = GetFileItem(itFile);

		itFile++;
		if (pFileItem != NULL)
		{
			CPartFile		*pPartFile = pFileItem->GetFile();

			if ( pPartFile->IsPartFile() == false
			  && ( CCat::FileBelongsToGivenCat(pPartFile,eCategoryID == CAT_NONE ? eCurCat : eCategoryID)
				|| (eCategoryID == CAT_ALL) ) )
			{
				RemoveFile(pPartFile);
			}
		}
	}

	g_App.m_pMDlg->m_wndTransfer.UpdateCatTabTitles();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	ClearCompleted(pPartFile) clears the single completed file with file pointer.
void CDownloadList::ClearCompleted(CPartFile *pPartFile)
{
	EMULE_TRY

	if (pPartFile != NULL && !pPartFile->IsPartFile())
	{
		RemoveFile(pPartFile);
		g_App.m_pMDlg->m_wndTransfer.UpdateCatTabTitles();
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	ClearCompleted(pFileHash) clears the single completed file with file hash.
void CDownloadList::ClearCompleted(const uchar *pFileHash /*=NULL*/)
{
	EMULE_TRY

	CArray<CPartFile*, CPartFile*>	completedFilesList;

	for (PFIter itFile = m_mapFiles.begin(); itFile != m_mapFiles.end(); )
	{
		CPartFileDLItem	   *pFileItem = GetFileItem(itFile);

		itFile++; // Already point to the next file
		if (pFileItem != NULL)
		{
			CPartFile	*pPartFile = pFileItem->GetFile();

			if (!pPartFile->IsPartFile())
			{
				if (pFileHash == NULL)
				{
					completedFilesList.Add(pPartFile);
				}
				else if (md4cmp(pFileHash, pPartFile->GetFileHash()) == 0)
				{
					RemoveFile(pPartFile);
					break;	//we can stop searching if match is found
				}
			}
		}
	}
//	eklmn: clear all file correct way
	if (pFileHash == NULL)
	{
		for (int i = 0; i < completedFilesList.GetCount(); i++)
		{
			RemoveFile(completedFilesList[i]);	//continue clearing all
		}
		completedFilesList.RemoveAll();
	}

	g_App.m_pMDlg->m_wndTransfer.UpdateCatTabTitles();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString CDownloadList::GetPartFilesStatusString()
{
	EMULE_TRY

	CString		strOut;

//	Search for file(s)
	for (PFIter itFile = m_mapFiles.begin(); itFile != m_mapFiles.end(); itFile++)
	{
		CPartFileDLItem		*pFileItem = GetFileItem(itFile);

		if (pFileItem != NULL)
		{
			CPartFile		*pPartFile = pFileItem->GetFile();

			strOut.AppendFormat( _T("\r\n%s\t [%.2f%%] %u/%u - %s"),
							 pPartFile->GetFileName(),
							 pPartFile->GetPercentCompleted2(),
							 pPartFile->GetTransferringSrcCount(),
							 pPartFile->GetSourceCount(),
							 pPartFile->GetPartFileStatus() );
		}
	}
	return strOut;

	EMULE_CATCH

	return _T("");
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDownloadList::PartFileVector *CDownloadList::GetFiles()
{
	PartFileVector		*pvecPartFiles = new PartFileVector();

	EMULE_TRY

	for (PFIter itFile = m_mapFiles.begin(); itFile != m_mapFiles.end(); itFile++)
	{
		pvecPartFiles->push_back(itFile->first);
	}

	EMULE_CATCH

	return pvecPartFiles;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDownloadList::PartFileItemVector *CDownloadList::GetFileItems()
{
	PartFileItemVector		*pvecPartFileItems = new PartFileItemVector();

	EMULE_TRY

	for (PFIter itFile = m_mapFiles.begin(); itFile != m_mapFiles.end(); itFile++)
	{
		pvecPartFileItems->push_back(itFile->second);
	}

	EMULE_CATCH

	return pvecPartFileItems;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDownloadList::SourceItemVector *CDownloadList::GetSourceItems(CUpDownClient *pSource/* = NULL*/)
{
	SourceItemVector	   *pvecSourceItems = new SourceItemVector();

	if (pSource != NULL)
	{
	//	Retrieve all entries matching the source
		SourceRange		range = FindSourceItems(pSource);

		for (SourceIter itSource = range.first; itSource != range.second; itSource++)
		{
			CSourceDLItem		*pSourceItem  = GetSourceItem(itSource);

			pvecSourceItems->push_back(pSourceItem);
		}
	}
	else
	{
		for (SourceIter itSource = m_mapSources.begin(); itSource != m_mapSources.end(); itSource++)
		{
			CSourceDLItem	   *pSourceItem = GetSourceItem(itSource);

			pvecSourceItems->push_back(pSourceItem);
		}
	}

	return pvecSourceItems;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadList::RemoveSourceItem(CSourceDLItem *pSourceItem)
{
#ifdef OLD_SOCKETS_ENABLED
	CSourceDLItem	   *pSourceItem2 = NULL;
	SourceRange			range = FindSourceItems(pSourceItem->GetSource());

//	For each source item associated with 'pSource' in the multi-map...
	for (SourceIter itSource = range.first; itSource != range.second; )
	{
		pSourceItem2 = GetSourceItem(itSource);

	//	If we found 'pSourceItem' in the map...
		if (pSourceItem == pSourceItem2)
		{
			m_mapSources.erase(itSource);

		//
		//	If it's in the dirty list, remove it from there too
			SourceItemVector::iterator	it = ::find(m_pvecDirtySources->begin(),m_pvecDirtySources->end(),pSourceItem);

			if (it != m_pvecDirtySources->end())
			{
				m_pvecDirtySources->erase(it);
			}

		//	Remove the source item from the download list ctrl
			m_pctlDownloadList->RemoveSourceItem(pSourceItem);

			break;
		}
		else
		{
			itSource++;
		}
	}
#endif //OLD_SOCKETS_ENABLED
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
