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

// MuleCtrlItem.cpp : implementation file
//

#include "stdafx.h"

#include "MuleCtrlItem.h"
#include "updownclient.h"
#include "emule.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CSourceDLItem::~CSourceDLItem()
{
	if (g_App.m_pDownloadList != NULL)
		g_App.m_pDownloadList->RemoveSourceItem(this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSourceDLItem::IsAskedForAnotherFile() const
{
	return (m_pSource ? (m_pSource->m_pReqPartFile != m_pParentFile) : false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPartFileDLItem::CPartFileDLItem(CPartFile *pPartFile)
	: m_pFile(pPartFile)
{
	m_bSrcsAreVisible = false;

	FilterNoSources();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPartFileDLItem::~CPartFileDLItem()
{
	RemoveAllSources();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	RemoveAllSources() removes and destroys all source items in the window associated with this file item.
void CPartFileDLItem::RemoveAllSources(void)
{
	while (m_mapSources.empty() == false)
	{
		delete m_mapSources.begin()->second;
		m_mapSources.erase(m_mapSources.begin());
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CSourceDLItem* CPartFileDLItem::CreateSourceItem(CUpDownClient *pSource,bool bIsAvailable)
{
	CSourceDLItem		*pSourceItem = NULL;

	pSourceItem = new CSourceDLItem(pSource,m_pFile,this);
	pSourceItem->SetAvailability(bIsAvailable);

	m_mapSources.insert(make_pair(pSource, pSourceItem));

	return pSourceItem;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPartFileDLItem::DeleteSourceItem(CSourceDLItem *pSourceItem)
{
	bool		bRemoved = false;

//	If the source belongs to this item...
	if (pSourceItem != NULL && pSourceItem->GetParentFileItem() == this)
	{
	//	Try to find it in our map
		SourceIter		itSource = m_mapSources.find(pSourceItem->GetSource());

	//	If we found it...
		if (itSource != m_mapSources.end())
		{
			m_mapSources.erase(itSource);
			delete pSourceItem;
			bRemoved = true;
		}
	}

	return bRemoved;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	FilterNoSources() turns off filtering for all source types.
void CPartFileDLItem::FilterNoSources()
{
	m_bShowUploadingSources = true;
	m_bShowOnQueueSources = true;
	m_bShowFullQueueSources = true;
	m_bShowConnectedSources = true;
	m_bShowConnectingSources = true;
	m_bShowNNPSources = true;
	m_bShowWaitForFileReqSources = true;
	m_bShowLowToLowIDSources = true;
	m_bShowLowIDOnOtherSrvSources = true;
	m_bShowBannedSources = true;
	m_bShowErrorSources = true;
	m_bShowA4AFSources = true;
	m_bShowUnknownSources = true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	FilterAllSources() turns on filtering for all source types.
void CPartFileDLItem::FilterAllSources()
{
	m_bShowUploadingSources = false;
	m_bShowOnQueueSources = false;
	m_bShowFullQueueSources = false;
	m_bShowConnectedSources = false;
	m_bShowConnectingSources = false;
	m_bShowNNPSources = false;
	m_bShowWaitForFileReqSources = false;
	m_bShowLowToLowIDSources = false;
	m_bShowLowIDOnOtherSrvSources = false;
	m_bShowBannedSources = false;
	m_bShowErrorSources = false;
	m_bShowA4AFSources = false;
	m_bShowUnknownSources = false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPartFileDLItem::SourceItemVector *CPartFileDLItem::GetSources()
{
	SourceItemVector	   *pvecSources = new SourceItemVector();

	for (SourceIter itSource = m_mapSources.begin(); itSource != m_mapSources.end(); itSource++)
	{
		CSourceDLItem	   *pSourceItem = itSource->second;

		pvecSources->push_back(pSourceItem);
	}

	return pvecSources;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
