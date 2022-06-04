//this file is part of eMule
//Copyright (C)2006-2007 David Xanatos ( XanatosDavid@googlemail.com / http://neomule.sourceforge.net )
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
#include "emuledlg.h"
#include "RequestedFilesCtrl.h"
#include "ClientDetailDialog.h"
#include "MemDC.h"
#include "MenuCmds.h"
#include "FriendList.h"
#include "TransferDlg.h"
#include "ChatWnd.h"
#include "UpDownClient.h"
#include "UploadQueue.h"
#include "ClientCredits.h"
#include "PartFile.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "SharedFileList.h"
#include "KnownFileList.h"
#include "FileDetailDialog.h"
#include "DownloadQueue.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

IMPLEMENT_DYNAMIC(CReqFileItem, CObject)

enum EColsF
{
	colFileName = 0,
	colParts,
	colStats
};

IMPLEMENT_DYNAMIC(CRequestedFilesCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CRequestedFilesCtrl, CMuleListCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnClick)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnLvnGetDispInfo)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNmDblClk)
   	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

CRequestedFilesCtrl::CRequestedFilesCtrl()
	: CRequestedFilesCtrlItemWalk(this)
{
}

CRequestedFilesCtrl::~CRequestedFilesCtrl()
{
	DeleteAllItems(true);
}

void CRequestedFilesCtrl::OnDestroy()
{
	CMuleListCtrl::OnDestroy(); 
}

BOOL CRequestedFilesCtrl::DeleteAllItems(bool bOnDestroy)
{
	for(int i = GetItemCount(); i > 0; i--)
	{
		CReqFileItem* pItem = (CReqFileItem*)m_pRequestedFilesCtrl->GetItemData(i-1);
		if(!bOnDestroy)
			SetItemData(i-1, NULL);
		if(pItem)
			delete pItem;
	}

	return bOnDestroy ? TRUE : CListCtrl::DeleteAllItems();
}

void CRequestedFilesCtrl::Init()
{
	SetPrefsKey(_T("RequestedFilesCtrl"));
	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	InsertColumn(colFileName, GetResString(IDS_DL_FILENAME), LVCFMT_LEFT, 300, -1); 
	InsertColumn(colParts, GetResString(IDS_UPSTATUS), LVCFMT_LEFT, 150, -1); 
	InsertColumn(colStats, GetResString(IDS_STATUS), LVCFMT_LEFT, 110, 1); 

	Localize();
	LoadSettings();
	SetSortArrow();
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0 : 100));
}

void CRequestedFilesCtrl::Localize()
{
	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;

	CString strRes;
	strRes = GetResString(IDS_DL_FILENAME);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(0, &hdi);

	strRes = GetResString(IDS_UPSTATUS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(1, &hdi);

	strRes = GetResString(IDS_STATUS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(2, &hdi);
}

void CRequestedFilesCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
}

void CRequestedFilesCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	if (!lpDrawItemStruct->itemData)
		return;

#ifdef EVENLINE
	uint32 iItem = lpDrawItemStruct->itemID;
        CMemoryDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem, m_crWindow);	
	InitItemMemDC(dc, lpDrawItemStruct->rcItem, (iItem % 2)?m_crEvenLine:m_crWindow, lpDrawItemStruct->itemState);
#else
	CMemoryDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	BOOL bCtrlFocused;
	InitItemMemDC(dc, lpDrawItemStruct, bCtrlFocused);
#endif
	CRect cur_rec(lpDrawItemStruct->rcItem);
	CRect rcClient;
	GetClientRect(&rcClient);
	CReqFileItem *pItem = (CReqFileItem *)lpDrawItemStruct->itemData;

	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	cur_rec.right = cur_rec.left - sm_iLabelOffset;
	cur_rec.left += sm_iIconOffset;
	for (int iCurrent = 0; iCurrent < iCount; iCurrent++)
	{
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
		if (!IsColumnHidden(iColumn))
		{
			UINT uDrawTextAlignment;
			int iColumnWidth = GetColumnWidth(iColumn, uDrawTextAlignment);
			cur_rec.right += iColumnWidth;
			if (cur_rec.left < cur_rec.right && HaveIntersection(rcClient, cur_rec))
			{
				TCHAR szItem[1024];
				GetItemDisplayText(pItem, iColumn, szItem, _countof(szItem));
				switch (iColumn)
				{
					case 1:
						cur_rec.bottom--;
						cur_rec.top++;
						if(m_eDataType == KNOWN_TYPE && pItem->m_pKnownFile)
                              m_pClient->DrawStatusBar(dc, &cur_rec, false, thePrefs.UseFlatBar());
						else if(m_eDataType == STRUCT_TYPE && pItem->m_pFileID)
                            m_pClient->DrawUpStatusBar(dc, &cur_rec, false, thePrefs.UseFlatBar());
						cur_rec.bottom++;
						cur_rec.top--;
						break;

					default:
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
						break;
				}
			}
			cur_rec.left += iColumnWidth;
		}
	}
#ifndef EVENLINE
	DrawFocusRect(dc, lpDrawItemStruct->rcItem, lpDrawItemStruct->itemState & ODS_FOCUS, bCtrlFocused, lpDrawItemStruct->itemState & ODS_SELECTED);
#endif
}

void CRequestedFilesCtrl::GetItemDisplayText(CReqFileItem *pItem, int iSubItem, LPTSTR pszText, int cchTextMax)
{
	if (pszText == NULL || cchTextMax <= 0)
	{
		ASSERT(0);
		return;
	}

	pszText[0] = _T('\0');
	switch (iSubItem)
	{
		case 0:
			_tcsncpy(pszText, pItem->m_strFileName, cchTextMax);
			break;

		case 2:
			if(m_eDataType == KNOWN_TYPE)
			{
				if(pItem->m_bNNP)
					_tcsncpy(pszText, GetResString(IDS_NONEEDEDPARTS), cchTextMax);
				else if(pItem->m_bA4AF)
					_tcsncpy(pszText, GetResString(IDS_ASKED4ANOTHERFILE), cchTextMax);
				else
					_tcsncpy(pszText, m_pClient->GetDownloadStateDisplayString(), cchTextMax);
			}
			else if(m_eDataType == STRUCT_TYPE)
			{
				if(pItem->m_bA4AF)
					_tcsncpy(pszText, GetResString(IDS_ASKED4ANOTHERFILE), cchTextMax);
				else
					_tcsncpy(pszText, m_pClient->GetUploadStateDisplayString(), cchTextMax);
			}
			break;
	}
	pszText[cchTextMax - 1] = _T('\0');
}

void CRequestedFilesCtrl::OnLvnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (theApp.emuledlg->IsRunning())
	{
		// Although we have an owner drawn listview control we store the text for the primary item in the listview, to be
		// capable of quick searching those items via the keyboard. Because our listview items may change their contents,
		// we do this via a text callback function. The listview control will send us the LVN_DISPINFO notification if
		// it needs to know the contents of the primary item.
		//
		// But, the listview control sends this notification all the time, even if we do not search for an item. At least
		// this notification is only sent for the visible items and not for all items in the list. Though, because this
		// function is invoked *very* often, do *NOT* put any time consuming code in here.
		//
		// Vista: That callback is used to get the strings for the label tips for the sub(!) items.
		//
		NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
		if (pDispInfo->item.mask & LVIF_TEXT)
		{
			CReqFileItem* pItem = reinterpret_cast<CReqFileItem*>(pDispInfo->item.lParam);
			if (pItem != NULL)
				GetItemDisplayText(pItem, pDispInfo->item.iSubItem, pDispInfo->item.pszText, pDispInfo->item.cchTextMax);
		}
	}
	*pResult = 0;
}

void CRequestedFilesCtrl::OnLvnColumnClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLISTVIEW *pNMListView = (NMLISTVIEW *)pNMHDR;
	bool sortAscending;
	if (GetSortItem() != pNMListView->iSubItem)
	{
		switch (pNMListView->iSubItem)
		{
			case 2: // Complete
				sortAscending = false;
				break;
			default:
				sortAscending = true;
				break;
		}
	}
	else
		sortAscending = !GetSortAscending();

	// Sort table
	UpdateSortHistory(pNMListView->iSubItem + (sortAscending ? 0 : 100));
	SetSortArrow(pNMListView->iSubItem, sortAscending);
	SortItems(SortProc, pNMListView->iSubItem + (sortAscending ? 0 : 100));

	*pResult = 0;
}

int CRequestedFilesCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	const CReqFileItem *item1 = (CReqFileItem *)lParam1;
	const CReqFileItem *item2 = (CReqFileItem *)lParam2;
	int iColumn = (lParamSort >= 100) ? lParamSort - 100 : lParamSort;
	int iResult = 0;

	switch(iColumn)
	{
		case 0: 
			if (item1->m_strFileName.IsEmpty())
				iResult = 1;
			else if (item2->m_strFileName.IsEmpty())
				iResult = -1;
			else if (item1->m_strFileName == item2->m_strFileName)
				iResult = 0;
			else
				iResult = CompareLocaleStringNoCase(item1->m_strFileName, item2->m_strFileName);
			break;

		case 1:
			// TODOXRAY: Sort by Complete
			iResult = 0;
			break;

		case 2:
			iResult = (item2->m_bNNP ? 3 : (item2->m_bA4AF ? 2 : 1)) - (item1->m_bNNP ? 3 : (item1->m_bA4AF ? 2 : 1));
			break;

	}

	if (lParamSort >= 100)
		iResult = -iResult;

	return iResult;
}

void CRequestedFilesCtrl::OnNmDblClk(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	CSimpleArray<CAbstractFile*> abstractFileList;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos != NULL)
	{
		int iIndex = GetNextSelectedItem(pos);
		if (iIndex >= 0)
		{
			CReqFileItem* pItem = (CReqFileItem*)GetItemData(iIndex);
			if(pItem && (theApp.knownfiles->IsKnownFile(pItem->m_pKnownFile) || theApp.downloadqueue->IsPartFile(pItem->m_pKnownFile)))
				abstractFileList.Add((CAbstractFile*)pItem->m_pKnownFile);
		}
	}

	if(abstractFileList.GetSize() > 0){		
	}

	*pResult = 0;
}

void CRequestedFilesCtrl::AddFile(CKnownFile* reqfile, bool bNNP, bool bA4AF){
	ASSERT(m_eDataType == KNOWN_TYPE);

	CReqFileItem* pItemData = FindItem(reqfile);
	if(pItemData)
	{
		if(pItemData->m_bNNP == bNNP && pItemData->m_bA4AF == bA4AF)
			return;

		pItemData->m_bNNP = bNNP;
		pItemData->m_bA4AF = bA4AF;
		UpdateItem(pItemData);
		return;
	}

	pItemData = new CReqFileItem;
	
	pItemData->m_bNNP = bNNP;
	pItemData->m_bA4AF = bA4AF;
	pItemData->m_pKnownFile = reqfile;
	pItemData->m_pFileID = NULL;
	pItemData->m_strFileName = reqfile->GetFileName();

	int iItem = InsertItem(LVIF_TEXT | LVIF_PARAM, GetItemCount(), LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)pItemData);
	if (iItem >= 0)
		Update(iItem);
}

void CRequestedFilesCtrl::AddFile(Requested_File_Struct* requpfile, bool bA4AF){
	ASSERT(m_eDataType == STRUCT_TYPE);

	CReqFileItem* pItemData = FindItem(requpfile);
	if(pItemData)
	{
		if(pItemData->m_bA4AF == bA4AF)
			return;

		pItemData->m_bA4AF = bA4AF;
		UpdateItem(pItemData);
		return;
	}
	
	pItemData = new CReqFileItem;
	
	pItemData->m_bNNP = false;
	pItemData->m_bA4AF = bA4AF;
	pItemData->m_pKnownFile = theApp.sharedfiles->GetFileByID(requpfile->fileid);
	if(pItemData->m_pKnownFile == NULL)
		pItemData->m_pKnownFile = theApp.downloadqueue->GetFileByID(requpfile->fileid);
	if(pItemData->m_pKnownFile == NULL)
		pItemData->m_pKnownFile = theApp.knownfiles->FindKnownFileByID(requpfile->fileid);
	pItemData->m_pFileID = requpfile;
	pItemData->m_strFileName = pItemData->m_pKnownFile ? pItemData->m_pKnownFile->GetFileName() : md4str(requpfile->fileid);

	int iItem = InsertItem(LVIF_TEXT | LVIF_PARAM, GetItemCount(), LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)pItemData);
	if (iItem >= 0)
		Update(iItem);
}

CReqFileItem* CRequestedFilesCtrl::FindItem(CKnownFile* reqfile)
{
	CReqFileItem* pItemData;
	for(int iItem = 0; iItem < GetItemCount(); iItem++)
	{
		pItemData = (CReqFileItem*)GetItemData(iItem);
		if(pItemData->m_pKnownFile == reqfile)
			return pItemData;
	}

	return NULL;
}

CReqFileItem* CRequestedFilesCtrl::FindItem(Requested_File_Struct* requpfile)
{
	CReqFileItem* pItemData;
	for(int iItem = 0; iItem < GetItemCount(); iItem++)
	{
		pItemData = (CReqFileItem*)GetItemData(iItem);
		if(pItemData->m_pFileID == requpfile)
			return pItemData;
	}

	return NULL;
}

void CRequestedFilesCtrl::UpdateItem(CReqFileItem* pItem)
{
	if (!pItem || !theApp.emuledlg->IsRunning())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)pItem;
	int iItem = CMuleListCtrl::FindItem(&find);
	if (iItem != -1)
		Update(iItem);
}

CRequestedFilesCtrlItemWalk::CRequestedFilesCtrlItemWalk(CRequestedFilesCtrl* pListCtrl)
	: CListCtrlItemWalk(pListCtrl)
{
	m_pRequestedFilesCtrl = pListCtrl;
}

CObject* CRequestedFilesCtrlItemWalk::GetPrevSelectableItem()
{
	ASSERT( m_pRequestedFilesCtrl != NULL );
	if (m_pRequestedFilesCtrl == NULL)
		return NULL;

	int iItemCount = m_pRequestedFilesCtrl->GetItemCount();
	if (iItemCount >= 2)
	{
		POSITION pos = m_pRequestedFilesCtrl->GetFirstSelectedItemPosition();
		if (pos)
		{
			int iItem = m_pRequestedFilesCtrl->GetNextSelectedItem(pos);
			int iCurSelItem = iItem;
			while (iItem-1 >= 0)
			{
				iItem--;

				CReqFileItem* Item = (CReqFileItem*)m_pRequestedFilesCtrl->GetItemData(iItem);
				if(theApp.knownfiles->IsKnownFile(Item->m_pKnownFile) || theApp.downloadqueue->IsPartFile(Item->m_pKnownFile)){
					m_pRequestedFilesCtrl->SetItemState(iCurSelItem, 0, LVIS_SELECTED | LVIS_FOCUSED);
					m_pRequestedFilesCtrl->SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
					m_pRequestedFilesCtrl->SetSelectionMark(iItem);
					m_pRequestedFilesCtrl->EnsureVisible(iItem, FALSE);
					return STATIC_DOWNCAST(CObject, (CObject*)Item->m_pKnownFile);
				}
			}
		}
	}
	return NULL;
}

CObject* CRequestedFilesCtrlItemWalk::GetNextSelectableItem()
{
	ASSERT( m_pRequestedFilesCtrl != NULL );
	if (m_pRequestedFilesCtrl == NULL)
		return NULL;

	int iItemCount =m_pRequestedFilesCtrl->GetItemCount();
	if (iItemCount >= 2)
	{
		POSITION pos = m_pRequestedFilesCtrl->GetFirstSelectedItemPosition();
		if (pos)
		{
			int iItem = m_pRequestedFilesCtrl->GetNextSelectedItem(pos);
			int iCurSelItem = iItem;
			while (iItem+1 < iItemCount)
			{
				iItem++;

				CReqFileItem* Item = (CReqFileItem*)m_pRequestedFilesCtrl->GetItemData(iItem);
				if(theApp.knownfiles->IsKnownFile(Item->m_pKnownFile) || theApp.downloadqueue->IsPartFile(Item->m_pKnownFile)){
					m_pRequestedFilesCtrl->SetItemState(iCurSelItem, 0, LVIS_SELECTED | LVIS_FOCUSED);
					m_pRequestedFilesCtrl->SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
					m_pRequestedFilesCtrl->SetSelectionMark(iItem);
					m_pRequestedFilesCtrl->EnsureVisible(iItem, FALSE);
					return STATIC_DOWNCAST(CObject, (CObject*)Item->m_pKnownFile);
				}
			}
		}
	}
	return NULL;
}