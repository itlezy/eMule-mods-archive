//this file is part of NeoMule
//Copyright (C)2006-2008 David Xanatos ( XanatosDavid@googlemail.com / http://NeoMule.sf.net )
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
#include "TransferWnd.h"
#include "ChatWnd.h"
#include "UpDownClient.h"
#include "UploadQueue.h"
#include "ClientCredits.h"
#include "PartFile.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "SharedFileList.h"
#include "KnownFileList.h"
#include "Neo/Functions.h" // NEO: MOD
#include "FileDetailDialog.h"
#include "DownloadQueue.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

// NEO: RFL - [RequestFileList] -- Xanatos -->

IMPLEMENT_DYNAMIC(CReqFileItem, CObject)

enum EColsF
{
	colFileName = 0,
	colParts,
	colStats
};

IMPLEMENT_DYNAMIC(CRequestedFilesCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CRequestedFilesCtrl, CMuleListCtrl)
   	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

CRequestedFilesCtrl::CRequestedFilesCtrl()
	: CRequestedFilesCtrlItemWalk(this)
{
	m_FileDetailDialog = NULL; // NEO: MLD - [ModelesDialogs]
}

CRequestedFilesCtrl::~CRequestedFilesCtrl()
{
	ASSERT(m_FileDetailDialog == NULL); // NEO: MLD - [ModelesDialogs]
	DeleteAllItems(true);
}

void CRequestedFilesCtrl::OnDestroy()
{
	// NEO: MLD - [ModelesDialogs]
	if(m_FileDetailDialog)
		m_FileDetailDialog->DropControl();
	m_FileDetailDialog = NULL;
	// NEO: MLD END
	CMuleListCtrl::OnDestroy(); 
}

BOOL CRequestedFilesCtrl::DeleteAllItems(bool bOnDestroy)
{
	for(int i=GetItemCount(); i>0; i--){
		CReqFileItem* Item = (CReqFileItem*)m_pRequestedFilesCtrl->GetItemData(i-1);
		if(!bOnDestroy)
			SetItemData(i-1, NULL);
		if(Item)
			delete Item;
	}

	return bOnDestroy ? TRUE : CListCtrl::DeleteAllItems();
}

void CRequestedFilesCtrl::Localize(void)
{
}

void CRequestedFilesCtrl::Init(void)
{
	SetName(_T("RequestedFilesCtrl"));
	ModifyStyle(LVS_SINGLESEL,0);
	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
	InsertColumn(colFileName, GetResString(IDS_DL_FILENAME), LVCFMT_LEFT, 300, -1); 
	InsertColumn(colParts, GetResString(IDS_UPSTATUS), LVCFMT_LEFT, 150, -1); 
	InsertColumn(colStats, GetResString(IDS_STATUS), LVCFMT_LEFT, 80, 1); 

	Localize();
	LoadSettings();

	SetSortArrow();
}

void CRequestedFilesCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if(m_eDataType == STRUCT_TYPE)
		return;

	int iSel = GetNextItem(-1, LVIS_SELECTED);
	if (iSel != -1)
	{
		CReqFileItem* Item = (CReqFileItem*)GetItemData(iSel);
		CPartFile* file = NULL;
		if(!theApp.knownfiles->IsKnownFile(Item->KnownFile) && !theApp.downloadqueue->IsPartFile(Item->KnownFile))
			Item->KnownFile = NULL;
		else if(Item->KnownFile->IsPartFile())
			file = (CPartFile*)Item->KnownFile;
		else
			file = NULL;

		// NEO: MCM - [ManualClientManagement]
		CTitleMenu A4AFMenu;
		A4AFMenu.CreatePopupMenu();
		A4AFMenu.AddMenuTitle(GetResString(IDS_X_SRC_MANAGEMENT), true);
		if (thePrefs.IsExtControlsEnabled()) {
			A4AFMenu.AppendMenu(MF_STRING | ((Item->A4AF) ? MF_ENABLED : MF_GRAYED),MP_SWAP_TO_CLIENT,GetResString(IDS_X_SWAP_TO_CLIENT), _T("SWAPTO"));
			A4AFMenu.AppendMenu(MF_STRING | ((!Item->NNP && !Item->A4AF) ? MF_ENABLED : MF_GRAYED),MP_SWAP_FROM_CLIENT,GetResString(IDS_X_SWAP_FROM_CLIENT), _T("SWAPFROM"));
			A4AFMenu.AppendMenu(MF_STRING | (((!Item->NNP && !Item->A4AF) || m_Client->IsSwapingDisabled()) ? MF_ENABLED : MF_GRAYED), MP_LOCK_CLIENT,GetResString((m_Client->IsSwapingDisabled()) ? (file && file == m_Client->GetRequestFile() ? IDS_X_MP_UNLOCK_CLIENT : IDS_X_MP_UNLOCKFROM_CLIENT) : IDS_X_MP_LOCK_CLIENT), (m_Client->IsSwapingDisabled()) ? ((file && file == m_Client->GetRequestFile()) ? _T("UNLOCK") : _T("UNLOCKFROM")) : _T("LOCKTO"));
		}
		// NEO: MCM EN

		GetPopupMenuPos(*this, point);
		A4AFMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
		VERIFY( A4AFMenu.DestroyMenu() );
	}
}

BOOL CRequestedFilesCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	CPartFile* file = NULL;
	CReqFileItem* Item = NULL;
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1)
	{
		Item = (CReqFileItem*)GetItemData(iSel);
		if(!theApp.knownfiles->IsKnownFile(Item->KnownFile) && !theApp.downloadqueue->IsPartFile(Item->KnownFile))
			Item->KnownFile = NULL;
		else if(Item->KnownFile->IsPartFile())
			file = (CPartFile*)Item->KnownFile;
		else
			file = NULL;
	}

	if(file){
		switch (wParam)
		{
		// NEO: MCM - [ManualClientManagement]
		case MP_SWAP_TO_CLIENT:
			if (file->GetStatus(false) == PS_READY || file->GetStatus(false) == PS_EMPTY)
			{
				if (m_Client->GetDownloadState() != DS_DOWNLOADING)
				{
					m_Client->DoSwap(file, false, _T("Manual: Swap to this File"),true); // ZZ:DownloadManager
					UpdateItem(Item);
				}
			}
			break;
		case MP_SWAP_FROM_CLIENT:
			if (file->GetStatus(false) == PS_READY || file->GetStatus(false) == PS_EMPTY)
			{
				if (m_Client->GetDownloadState() != DS_DOWNLOADING)
				{
					m_Client->SwapToAnotherFile(_T("Manual: Swap to an Other File"), false, true, false, NULL, true, true, true); // ZZ:DownloadManager
					UpdateItem(Item);
				}
			}
			break;
		case MP_LOCK_CLIENT:
			if (file->GetStatus(false) == PS_READY || file->GetStatus(false) == PS_EMPTY)
			{
				m_Client->DisableSwaping(!m_Client->IsSwapingDisabled());
				UpdateItem(Item);
			}
			break;
		// NEO: MCM END
		}
	}
	return CMuleListCtrl::OnCommand(wParam, lParam);
}

void CRequestedFilesCtrl::OnNMDblclk(NMHDR* /*pNMHDR*/, LRESULT* pResult) {
	CSimpleArray<CAbstractFile*> abstractFileList;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos != NULL)
	{
		int index = GetNextSelectedItem(pos);
		if (index >= 0){
			CReqFileItem* Item = (CReqFileItem*)GetItemData(index);
			if(Item && (theApp.knownfiles->IsKnownFile(Item->KnownFile) || theApp.downloadqueue->IsPartFile(Item->KnownFile)))
				abstractFileList.Add((CAbstractFile*)Item->KnownFile);
		}
	}

	if(abstractFileList.GetSize() > 0)
	{
		// NEO: MLD - [ModelesDialogs]
		if(m_FileDetailDialog)
			m_FileDetailDialog->DropControl();
		m_FileDetailDialog = new CFileDetailDialog(&abstractFileList, 0, this);
		m_FileDetailDialog->OpenDialog(FALSE); 
		// NEO: MLD END 
	}

	*pResult = 0;
}

void CRequestedFilesCtrl::AddFile(CKnownFile* reqfile, bool bNNP, bool bA4AF){
	ASSERT(m_eDataType == KNOWN_TYPE);

	CReqFileItem* ItemData = FindItem(reqfile);
	if(ItemData){
		if(ItemData->NNP == bNNP
		&& ItemData->A4AF == bA4AF)
			return;

		ItemData->NNP = bNNP;
		ItemData->A4AF = bA4AF;
		UpdateItem(ItemData);
		return;
	}

	ItemData = new CReqFileItem;
	
	ItemData->NNP = bNNP;
	ItemData->A4AF = bA4AF;
	ItemData->KnownFile = reqfile;
	ItemData->FileID = NULL;
	ItemData->FileName = reqfile->GetFileName();

	int iItem = InsertItem(LVIF_TEXT|LVIF_PARAM, GetItemCount(), LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)ItemData);
	if (iItem >= 0)
		Update(iItem);
}

void CRequestedFilesCtrl::AddFile(Requested_File_Struct* requpfile, bool bA4AF){
	ASSERT(m_eDataType == STRUCT_TYPE);

	CReqFileItem* ItemData = FindItem(requpfile);
	if(ItemData){
		if(ItemData->A4AF == bA4AF)
			return;

		ItemData->A4AF = bA4AF;
		UpdateItem(ItemData);
		return;
	}
	
	ItemData = new CReqFileItem;
	
	ItemData->NNP = false;
	ItemData->A4AF = bA4AF;
	ItemData->KnownFile = theApp.sharedfiles->GetFileByID(requpfile->fileid);
	if(ItemData->KnownFile == NULL)
		ItemData->KnownFile = theApp.downloadqueue->GetFileByID(requpfile->fileid);
	if(ItemData->KnownFile == NULL)
		ItemData->KnownFile = theApp.knownfiles->FindKnownFileByID(requpfile->fileid);
	ItemData->FileID = requpfile;
	ItemData->FileName = ItemData->KnownFile ? ItemData->KnownFile->GetFileName() : md4str(requpfile->fileid);

	int iItem = InsertItem(LVIF_TEXT|LVIF_PARAM, GetItemCount(), LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)ItemData);
	if (iItem >= 0)
		Update(iItem);
}

CReqFileItem* CRequestedFilesCtrl::FindItem(CKnownFile* reqfile)
{
	CReqFileItem* ItemData;
	for(int nItem=0; nItem<GetItemCount(); nItem++)
	{
		ItemData=(CReqFileItem*)GetItemData(nItem);
		if(ItemData->KnownFile == reqfile)
			return ItemData;
	}

	return NULL;
}

CReqFileItem* CRequestedFilesCtrl::FindItem(Requested_File_Struct* requpfile)
{
	CReqFileItem* ItemData;
	for(int nItem=0; nItem<GetItemCount(); nItem++)
	{
		ItemData=(CReqFileItem*)GetItemData(nItem);
		if(ItemData->FileID == requpfile)
			return ItemData;
	}

	return NULL;
}

void CRequestedFilesCtrl::UpdateItem(CReqFileItem* ToUpdate)
{
	if (!ToUpdate || !theApp.emuledlg->IsRunning())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)ToUpdate;
	int iItem = CMuleListCtrl::FindItem(&find);
	if (iItem != -1)
		Update(iItem);
}

#define DLC_DT_TEXT (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS)

void CRequestedFilesCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	if (!lpDrawItemStruct->itemData)
		return;
	CDC* odc = CDC::FromHandle(lpDrawItemStruct->hDC);
	BOOL bCtrlFocused = ((GetFocus() == this) || (GetStyle() & LVS_SHOWSELALWAYS));
	if (lpDrawItemStruct->itemState & ODS_SELECTED) {
		if (bCtrlFocused)
			odc->SetBkColor(m_crHighlight);
		else
			odc->SetBkColor(m_crNoHighlight);
	}
	else
		odc->SetBkColor(GetBkColor());

	CReqFileItem* Item = (CReqFileItem*)lpDrawItemStruct->itemData;
	if(!theApp.knownfiles->IsKnownFile(Item->KnownFile) && !theApp.downloadqueue->IsPartFile(Item->KnownFile))
		Item->KnownFile = NULL;

	CMemDC dc(odc, &lpDrawItemStruct->rcItem);
	CFont* pOldFont = dc.SelectObject(GetFont());
	CRect cur_rec(lpDrawItemStruct->rcItem);
	COLORREF crOldTextColor = dc.SetTextColor((lpDrawItemStruct->itemState & ODS_SELECTED) ? m_crHighlightText : m_crWindowText);

	int iOldBkMode;
	if (m_crWindowTextBk == CLR_NONE){
		DefWindowProc(WM_ERASEBKGND, (WPARAM)(HDC)dc, 0);
		iOldBkMode = dc.SetBkMode(TRANSPARENT);
	}
	else
		iOldBkMode = OPAQUE;

	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	cur_rec.right = cur_rec.left - 8;
	cur_rec.left += 4;
	CString Sbuffer;	
	for(int iCurrent = 0; iCurrent < iCount; iCurrent++){
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
		if( !IsColumnHidden(iColumn) ){
			cur_rec.right += GetColumnWidth(iColumn);
			switch(iColumn){
			case 0:
				Sbuffer = Item->FileName;
				break;
			case 1:
				cur_rec.bottom--;
				cur_rec.top++;
				if(m_eDataType == KNOWN_TYPE && Item->KnownFile)
					m_Client->DrawStatusBar(dc, &cur_rec, (CPartFile*)Item->KnownFile, thePrefs.UseFlatBar());
				else if(m_eDataType == STRUCT_TYPE && Item->FileID)
					m_Client->DrawUpStatusBar(dc, &cur_rec, Item->FileID->fileid, thePrefs.UseFlatBar());
				cur_rec.bottom++;
				cur_rec.top--;

				break;
			case 2:
				if(m_eDataType == KNOWN_TYPE){
					if(Item->NNP){
						Sbuffer = GetResString(IDS_NONEEDEDPARTS); 
					}else if(Item->A4AF){
						Sbuffer = GetResString(IDS_ASKED4ANOTHERFILE); 
					}else
						Sbuffer = m_Client->GetDownloadStateDisplayString();
				}else if(m_eDataType == STRUCT_TYPE){
					if(Item->A4AF){
						Sbuffer = GetResString(IDS_ASKED4ANOTHERFILE); 
					}else
						Sbuffer = m_Client->GetUploadStateDisplayString();
				}
				break;
			}
			if( iColumn != 1)
				dc.DrawText(Sbuffer,Sbuffer.GetLength(),&cur_rec,DLC_DT_TEXT);
			cur_rec.left += GetColumnWidth(iColumn);
		}
	}

	//draw rectangle around selected item(s)
	if (lpDrawItemStruct->itemState & ODS_SELECTED)
	{
		RECT outline_rec = lpDrawItemStruct->rcItem;

		outline_rec.top--;
		outline_rec.bottom++;
		dc.FrameRect(&outline_rec, &CBrush(GetBkColor()));
		outline_rec.top++;
		outline_rec.bottom--;
		outline_rec.left++;
		outline_rec.right--;

		if (bCtrlFocused)
			dc.FrameRect(&outline_rec, &CBrush(m_crFocusLine));
		else
			dc.FrameRect(&outline_rec, &CBrush(m_crNoFocusLine));
	}

	if (m_crWindowTextBk == CLR_NONE)
		dc.SetBkMode(iOldBkMode);

	dc.SelectObject(pOldFont);
	dc.SetTextColor(crOldTextColor);
}

void CRequestedFilesCtrl::OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult){

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// Barry - Store sort order in preferences
	// Determine ascending based on whether already sorted on this column
	bool sortAscending = (GetSortItem()!= pNMListView->iSubItem) ? (pNMListView->iSubItem == 0) : !GetSortAscending();	

	// Sort table
	UpdateSortHistory(pNMListView->iSubItem + (sortAscending ? 0:100), 100);
	SetSortArrow(pNMListView->iSubItem, sortAscending);
	SortItems(SortProc, pNMListView->iSubItem + (sortAscending ? 0:100));

	*pResult = 0;
}

int CRequestedFilesCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort){
	const CReqFileItem* item1 = (CReqFileItem*)lParam1;
	const CReqFileItem* item2 = (CReqFileItem*)lParam2;
	switch(lParamSort){
		case 0: 
			if (item1->FileName.IsEmpty())
				return 1;
			else if (item2->FileName.IsEmpty())
				return -1;
			else if (item1->FileName == item2->FileName)
				return 0;
			return CompareLocaleStringNoCase(item1->FileName, item2->FileName);
			break;
		case 100:
			if (item1->FileName.IsEmpty())
				return -1;
			else if (item2->FileName.IsEmpty())
				return 1;
			else if (item1->FileName == item2->FileName)
				return 0;
			return -CompareLocaleStringNoCase(item1->FileName, item2->FileName);
			break;
		case 1:
			return 0;
		    break;
	    case 101:
			return 0;
			break;
		case 2:
			return (item2->NNP ? 3 : (item2->A4AF ? 2 : 1)) - (item1->NNP ? 3 : (item1->A4AF ? 2 : 1));
			break;
		case 102:
			return (item1->NNP ? 3 : (item1->A4AF ? 2 : 1)) - (item2->NNP ? 3 : (item2->A4AF ? 2 : 1));
			break;
		default:
			return 0;
	}
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
				if(theApp.knownfiles->IsKnownFile(Item->KnownFile) || theApp.downloadqueue->IsPartFile(Item->KnownFile)){
					m_pRequestedFilesCtrl->SetItemState(iCurSelItem, 0, LVIS_SELECTED | LVIS_FOCUSED);
					m_pRequestedFilesCtrl->SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
					m_pRequestedFilesCtrl->SetSelectionMark(iItem);
					m_pRequestedFilesCtrl->EnsureVisible(iItem, FALSE);
					return STATIC_DOWNCAST(CObject, (CObject*)Item->KnownFile);
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
				if(theApp.knownfiles->IsKnownFile(Item->KnownFile) || theApp.downloadqueue->IsPartFile(Item->KnownFile)){
					m_pRequestedFilesCtrl->SetItemState(iCurSelItem, 0, LVIS_SELECTED | LVIS_FOCUSED);
					m_pRequestedFilesCtrl->SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
					m_pRequestedFilesCtrl->SetSelectionMark(iItem);
					m_pRequestedFilesCtrl->EnsureVisible(iItem, FALSE);
					return STATIC_DOWNCAST(CObject, (CObject*)Item->KnownFile);
				}
			}
		}
	}
	return NULL;
}

// NEO: RFL END <-- Xanatos --