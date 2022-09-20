//this file is part of eMule
//Copyright (C)2002-2007 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "AudioListCtrl.h"
#include "SharedFilesCtrl.h"
#include "OtherFunctions.h"
//#include "FileInfoDialog.h"
//#include "MetaDataDlg.h"
//#include "ED2kLinkDlg.h"
//#include "ArchivePreviewDlg.h"
//#include "CommentDialog.h"
#include "HighColorTab.hpp"
//#include "ListViewWalkerPropertySheet.h"
#include "UserMsgs.h"
#include "ResizableLib/ResizableSheet.h"
#include "KnownFile.h"
#include "MapKey.h"
#include "SharedFileList.h"
#include "MemDC.h"
#include "PartFile.h"
#include "MenuCmds.h"
#include "IrcWnd.h"
#include "SharedFilesWnd.h"
#include "Opcodes.h"
#include "InputBox.h"
#include "WebServices.h"
#include "TransferWnd.h"
#include "ClientList.h"
#include "UpDownClient.h"
#include "Collection.h"
#include "CollectionCreateDialog.h"
#include "CollectionViewDialog.h"
#include "SharedDirsTreeCtrl.h"
#include "SearchParams.h"
#include "SearchDlg.h"
#include "SearchResultsWnd.h"
#include "MassRename.h" //Xman Mass Rename (Morph)
#include "Log.h" //Xman Mass Rename (Morph)
#include "./Addons/MediaPlayerWnd.h" //>>> WiZaRd::MediaPlayer

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////////
// CAudioListCtrl

IMPLEMENT_DYNAMIC(CAudioListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CAudioListCtrl, CMuleListCtrl)
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

CAudioListCtrl::CAudioListCtrl()
: CListCtrlItemWalk(this)
{	
	memset(&sortstat, 0, sizeof(sortstat));
	SetGeneralPurposeFind(true, false);
}

CAudioListCtrl::~CAudioListCtrl()
{
	// Tux: Fix: WiZaRd memleak fix [start]
	if (m_PrioMenu) VERIFY( m_PrioMenu.DestroyMenu() );
	if (m_CollectionsMenu) VERIFY( m_CollectionsMenu.DestroyMenu() );
	if (m_SharedFilesMenu) VERIFY( m_SharedFilesMenu.DestroyMenu() );
	// Tux: Fix: WiZaRd memleak fix [end]
}

void CAudioListCtrl::Init()
{
	SetName(_T("AudioListCtrl"));
	CImageList ilDummyImageList; //dummy list for getting the proper height of listview entries
	ilDummyImageList.Create(1, theApp.GetSmallSytemIconSize().cy,theApp.m_iDfltImageListColorFlags|ILC_MASK, 1, 1); 
	SetImageList(&ilDummyImageList, LVSIL_SMALL);
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	ilDummyImageList.Detach();

	SetExtendedStyle(LVS_EX_FULLROWSELECT);
	ModifyStyle(LVS_SINGLESEL,0);

	InsertColumn(0, GetResString(IDS_DL_FILENAME) ,LVCFMT_LEFT,250,0);
	InsertColumn(1,GetResString(IDS_ARTIST),LVCFMT_LEFT,100);
	InsertColumn(2,GetResString(IDS_ALBUM),LVCFMT_LEFT,100);
	InsertColumn(3,GetResString(IDS_TITLE),LVCFMT_LEFT,100);
	InsertColumn(4,GetResString(IDS_LENGTH),LVCFMT_LEFT,50);
	InsertColumn(5,GetResString(IDS_BITRATE),LVCFMT_LEFT,50);
	InsertColumn(6,GetResString(IDS_COMMENT),LVCFMT_LEFT,150);

	SetAllIcons();
	CreateMenues();
	LoadSettings();

	// Barry - Use preferred sort order from preferences
	SetSortArrow();
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0:20));
}

void CAudioListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
	CreateMenues();
}

void CAudioListCtrl::SetAllIcons()
{
	m_ImageList.DeleteImageList();
	m_ImageList.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	m_ImageList.SetBkColor(CLR_NONE);
	m_ImageList.Add(CTempIconLoader(_T("EMPTY")));
	m_ImageList.Add(CTempIconLoader(_T("FileSharedServer"), 16, 16));
	m_ImageList.Add(CTempIconLoader(_T("FileSharedKad"), 16, 16));
	m_ImageList.Add(CTempIconLoader(_T("Rating_NotRated")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Fake")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Poor")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Fair")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Good")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Excellent")));
	m_ImageList.Add(CTempIconLoader(_T("Collection_Search"))); // rating for comments are searched on kad
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("FileCommentsOvl"))), 1);
}

void CAudioListCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;

	for(int i = 0; i != pHeaderCtrl->GetItemCount(); ++i)
	{
		switch (i) 
		{
			case 0: strRes = GetResString(IDS_DL_FILENAME); break;
			case 1: strRes = GetResString(IDS_ARTIST); break;
			case 2: strRes = GetResString(IDS_ALBUM); break;
			case 3: strRes = GetResString(IDS_TITLE); break;
			case 4: strRes = GetResString(IDS_LENGTH); break;
			case 5: strRes = GetResString(IDS_BITRATE); break;
			case 6: strRes = GetResString(IDS_COMMENT); break;
		}

		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(i, &hdi);
	}

	CreateMenues();

	int iItems = GetItemCount();
	for (int i = 0; i < iItems; i++)
		Update(i);
}

void CAudioListCtrl::AddFile(const CKnownFile* file)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	if (FindFile(file) != -1)
		return;
	if(GetED2KFileTypeID(file->GetFileName()) != ED2KFT_AUDIO)
		return;
//>>> WiZaRd::ShareFilter
	const CStringArray& rastrFilter = theApp.emuledlg->sharedfileswnd->m_astrFilter;
	if (!rastrFilter.IsEmpty())
	{
		// filtering is done by text only for all columns to keep it consistent and simple for the user
		TCHAR szFilterTarget[256];
		GetItemDisplayText(file, theApp.emuledlg->sharedfileswnd->GetFilterColumn(), szFilterTarget, _countof(szFilterTarget));
		for (int i = 0; i < rastrFilter.GetSize(); ++i)
		{
			const CString& rstrExpr = rastrFilter.GetAt(i);
			bool bAnd = true;
			LPCTSTR pszText = (LPCTSTR)rstrExpr;
			if (pszText[0] == L'-') 
			{
				pszText += 1;
				bAnd = false;
			}

			const bool bFound = (stristr(szFilterTarget, pszText) != NULL);
			if ((bAnd && !bFound) || (!bAnd && bFound))
				return;
		}
	}
//<<< WiZaRd::ShareFilter
	int iItem = InsertItem(LVIF_TEXT|LVIF_PARAM, GetItemCount(), LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)file);
	if (iItem >= 0)
		Update(iItem);
	ShowFilesCount(); //Xman Code Improvement for ShowFilesCount
}

void CAudioListCtrl::RemoveFile(const CKnownFile* file)
{
//search anyways... some ppl might give a wrong extension...
//	if(GetED2KFileTypeID(file->GetFileName()) != ED2KFT_AUDIO)
//		return;
	int iItem = FindFile(file);
	if (iItem != -1)
	{
		DeleteItem(iItem);		
		ShowFilesCount();
	}
}

void CAudioListCtrl::UpdateFile(const CKnownFile* file)
{
	if (!file || !theApp.emuledlg->IsRunning())
		return;

	//MORPH START - SiRoB, Don't Refresh item if not needed
	if( theApp.emuledlg->activewnd != theApp.emuledlg->sharedfileswnd)
		return;
	//MORPH END   - SiRoB, Don't Refresh item if not needed

	if(GetED2KFileTypeID(file->GetFileName()) != ED2KFT_AUDIO)
		return;

	int iItem = FindFile(file);
	if (iItem != -1)
	{
		Update(iItem);
		if (GetItemState(iItem, LVIS_SELECTED) && IsWindowVisible()) //Xman [MoNKi: -Downloaded History-]
			theApp.emuledlg->sharedfileswnd->ShowSelectedFilesSummary();
	}
}

int CAudioListCtrl::FindFile(const CKnownFile* pFile)
{
	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)pFile;
	return FindItem(&find);
}

void CAudioListCtrl::ShowFilesCount()
{
	if(!theApp.emuledlg->sharedfileswnd->historylistctrl.IsWindowVisible() && !theApp.emuledlg->sharedfileswnd->sharedfilesctrl.IsWindowVisible())
	{	
		CString str;
		if (theApp.sharedfiles->GetHashingCount())
			str.Format(_T(" (%i, %s %i)"), theApp.sharedfiles->GetCount(), GetResString(IDS_HASHING), theApp.sharedfiles->GetHashingCount());
		else
			str.Format(_T(" (%i)"), theApp.sharedfiles->GetCount());
		theApp.emuledlg->sharedfileswnd->GetDlgItem(IDC_TRAFFIC_TEXT)->SetWindowText(GetResString(IDS_SF_FILES) + str);
	}
}

#define DLC_DT_TEXT (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS)

void CAudioListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	if (!lpDrawItemStruct->itemData)
		return;

	//MORPH START - Added by SiRoB, Don't draw hidden Rect
	RECT clientRect;
	GetClientRect(&clientRect);
	RECT cur_rec = lpDrawItemStruct->rcItem;
	if (cur_rec.top >= clientRect.bottom || cur_rec.bottom <= clientRect.top)
		return;
	//MORPH END   - Added by SiRoB, Don't draw hidden Rect

	CDC* odc = CDC::FromHandle(lpDrawItemStruct->hDC);
	BOOL bCtrlFocused = ((GetFocus() == this) || (GetStyle() & LVS_SHOWSELALWAYS));
	if (lpDrawItemStruct->itemState & ODS_SELECTED) {
		if (bCtrlFocused)
			odc->SetBkColor(m_crHighlight);
		else
			odc->SetBkColor(m_crNoHighlight);
	}
	else
	{
		//Xman PowerRelease
		if(((CKnownFile*)lpDrawItemStruct->itemData)->GetUpPriority()==PR_POWER)
			odc->SetBkColor(RGB(255,210,210));
		else
			odc->SetBkColor(m_crWindow);
		//Xman end
	}

	COLORREF crOldBackColor = odc->GetBkColor(); //Xman Code Improvement: FillSolidRect

	/*const*/ CKnownFile* file = (CKnownFile*)lpDrawItemStruct->itemData;
	CMemDC dc(odc, &lpDrawItemStruct->rcItem);
	CFont* pOldFont = dc.SelectObject(GetFont());
	//CRect cur_rec(lpDrawItemStruct->rcItem); //MORPH - Moved by SiRoB, Don't draw hidden Rect
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
	const int iMarginX = 4;
	cur_rec.right = cur_rec.left - iMarginX*2;
	cur_rec.left += iMarginX;
	CString buffer;
	int iIconDrawWidth = theApp.GetSmallSytemIconSize().cx + 3;
	for(int iCurrent = 0; iCurrent < iCount; iCurrent++){
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
		if( !IsColumnHidden(iColumn) ){
			UINT uDTFlags = DLC_DT_TEXT;
			cur_rec.right += GetColumnWidth(iColumn);
			switch(iColumn)
			{
				case 0:{
					int iImage = theApp.GetFileTypeSystemImageIdx(file->GetFileName());
					if (theApp.GetSystemImageList() != NULL)
						::ImageList_Draw(theApp.GetSystemImageList(), iImage, dc.GetSafeHdc(), cur_rec.left, cur_rec.top, ILD_NORMAL|ILD_TRANSPARENT);
					if (!file->GetFileComment().IsEmpty() || file->GetFileRating())
						m_ImageList.Draw(dc, 0, CPoint(cur_rec.left, cur_rec.top), ILD_NORMAL | ILD_TRANSPARENT | INDEXTOOVERLAYMASK(1));
					cur_rec.left += (iIconDrawWidth - 3);

					if (thePrefs.ShowRatingIndicator() && (file->HasComment() || file->HasRating() || file->IsKadCommentSearchRunning()))
					{
						m_ImageList.Draw(dc, file->UserRating(true)+3, CPoint(cur_rec.left, cur_rec.top), ILD_NORMAL);
						cur_rec.left += 16;
						iIconDrawWidth += 16;
					}

					cur_rec.left += 3;

					buffer = file->GetFileName();
					break;
				}
				case 1:
					buffer = file->GetStrTagValue(FT_MEDIA_ARTIST);
					break;
				case 2:
					buffer = file->GetStrTagValue(FT_MEDIA_ALBUM);
					break;
				case 3:
					buffer = file->GetStrTagValue(FT_MEDIA_TITLE);
					break;
				case 4:{
					uint32 nMediaLength = file->GetIntTagValue(FT_MEDIA_LENGTH);
					if (nMediaLength)
						SecToTimeLength(nMediaLength, buffer);
					else 
						buffer.Empty();
					break;
					   }
				case 5:{
					uint32 nBitrate = file->GetIntTagValue(FT_MEDIA_BITRATE);
					if (nBitrate)
						buffer.Format(_T("%u %s"), nBitrate, GetResString(IDS_KBITSSEC));
					else
						buffer.Empty();
					break;
						}
				case 6:
					buffer = file->GetFileComment();
					if(buffer.IsEmpty())
						buffer = GetResString(IDS_COMMENTME);
					break;
			}

			dc.DrawText(buffer, buffer.GetLength(), &cur_rec, uDTFlags);
			if (iColumn == 0)
				cur_rec.left -= iIconDrawWidth;
			dc.SetBkColor(crOldBackColor); //Xman Code Improvement: FillSolidRect
			cur_rec.left += GetColumnWidth(iColumn);
		}
	}
	//ShowFilesCount(); //Xman Code Improvement for ShowFilesCount
	if (lpDrawItemStruct->itemState & ODS_SELECTED)
	{
		RECT outline_rec = lpDrawItemStruct->rcItem;

		outline_rec.top--;
		outline_rec.bottom++;
		dc.FrameRect(&outline_rec, &CBrush(m_crWindow));
		outline_rec.top++;
		outline_rec.bottom--;
		outline_rec.left++;
		outline_rec.right--;

		if (lpDrawItemStruct->itemID > 0 && GetItemState(lpDrawItemStruct->itemID - 1, LVIS_SELECTED))
			outline_rec.top--;

		if (lpDrawItemStruct->itemID + 1 < (UINT)GetItemCount() && GetItemState(lpDrawItemStruct->itemID + 1, LVIS_SELECTED))
			outline_rec.bottom++;

		if(bCtrlFocused)
			dc.FrameRect(&outline_rec, &CBrush(m_crFocusLine));
		else
			dc.FrameRect(&outline_rec, &CBrush(m_crNoFocusLine));
	}
	if (m_crWindowTextBk == CLR_NONE)
		dc.SetBkMode(iOldBkMode);
	dc.SelectObject(pOldFont);
	dc.SetTextColor(crOldTextColor);
}

void CAudioListCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	// get merged settings
	bool bFirstItem = true;
	int iSelectedItems = GetSelectedCount();
	int iCompleteFileSelected = -1;
	UINT uPrioMenuItem = 0;
	const CKnownFile* pSingleSelFile = NULL;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos)
	{
		const CKnownFile* pFile = (CKnownFile*)GetItemData(GetNextSelectedItem(pos));
		if (bFirstItem)
			pSingleSelFile = pFile;
		else
			pSingleSelFile = NULL;

		int iCurCompleteFile = pFile->IsPartFile() ? 0 : 1;
		if (bFirstItem)
			iCompleteFileSelected = iCurCompleteFile;
		else if (iCompleteFileSelected != iCurCompleteFile)
			iCompleteFileSelected = -1;

		UINT uCurPrioMenuItem = 0;
		if (pFile->IsAutoUpPriority())
			uCurPrioMenuItem = MP_PRIOAUTO;
		else if (pFile->GetUpPriority() == PR_VERYLOW)
			uCurPrioMenuItem = MP_PRIOVERYLOW;
		else if (pFile->GetUpPriority() == PR_LOW)
			uCurPrioMenuItem = MP_PRIOLOW;
		else if (pFile->GetUpPriority() == PR_NORMAL)
			uCurPrioMenuItem = MP_PRIONORMAL;
		else if (pFile->GetUpPriority() == PR_HIGH)
			uCurPrioMenuItem = MP_PRIOHIGH;
		else if (pFile->GetUpPriority() == PR_VERYHIGH)
			uCurPrioMenuItem = MP_PRIOVERYHIGH;
		//Xman PowerRelease
		else if (pFile->GetUpPriority()==PR_POWER)
			uCurPrioMenuItem = MP_PRIOPOWER;
		//Xman end
		else
			ASSERT(0);

		if (bFirstItem)
			uPrioMenuItem = uCurPrioMenuItem;
		else if (uPrioMenuItem != uCurPrioMenuItem)
			uPrioMenuItem = 0;

		bFirstItem = false;
	}

	m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_PrioMenu.m_hMenu, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	//m_PrioMenu.CheckMenuRadioItem(MP_PRIOVERYLOW, MP_PRIOAUTO, uPrioMenuItem, 0);
	m_PrioMenu.CheckMenuRadioItem(MP_PRIOVERYLOW, MP_PRIOPOWER, uPrioMenuItem, 0); //Xman PowerRelease 

	bool bSingleCompleteFileSelected = (iSelectedItems == 1 && iCompleteFileSelected == 1);
	m_SharedFilesMenu.EnableMenuItem(MP_OPEN, bSingleCompleteFileSelected ? MF_ENABLED : MF_GRAYED);
//>>> WiZaRd::MediaPlayer
	//if not available, try to reinitialize
	m_SharedFilesMenu.EnableMenuItem(MP_VLCVIEW, iSelectedItems == 1 && theApp.emuledlg->mediaplayerwnd->Init() && theApp.emuledlg->mediaplayerwnd->CanPlayFileType(pSingleSelFile) ? MF_ENABLED : MF_GRAYED);
//<<< WiZaRd::MediaPlayer
	UINT uInsertedMenuItem = 0;

	// X-Ray :: Toolbar :: Cleanup :: Start
	/*
	static const TCHAR _szSkinPkgSuffix1[] = _T(".") EMULSKIN_BASEEXT _T(".zip");
	static const TCHAR _szSkinPkgSuffix2[] = _T(".") EMULSKIN_BASEEXT _T(".rar");
	if (bSingleCompleteFileSelected 
	&& pSingleSelFile 
	&& (   pSingleSelFile->GetFilePath().Right(ARRSIZE(_szSkinPkgSuffix1)-1).CompareNoCase(_szSkinPkgSuffix1) == 0
	|| pSingleSelFile->GetFilePath().Right(ARRSIZE(_szSkinPkgSuffix2)-1).CompareNoCase(_szSkinPkgSuffix2) == 0))
	{
	MENUITEMINFO mii = {0};
	mii.cbSize = sizeof mii;
	mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED;
	mii.wID = MP_INSTALL_SKIN;
	CString strBuff(GetResString(IDS_INSTALL_SKIN));
	mii.dwTypeData = const_cast<LPTSTR>((LPCTSTR)strBuff);
	if (::InsertMenuItem(m_SharedFilesMenu, MP_OPENFOLDER, FALSE, &mii))
	uInsertedMenuItem = mii.wID;
	}
	*/
	// X-Ray :: Toolbar :: Cleanup :: End

	//Xman PowerRelease
	m_SharedFilesMenu.EnableMenuItem(MP_PRIOPOWER, iCompleteFileSelected >0 ? MF_ENABLED : MF_GRAYED);
	//Xman end
	m_SharedFilesMenu.EnableMenuItem(MP_OPENFOLDER, bSingleCompleteFileSelected ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(MP_RENAME, bSingleCompleteFileSelected ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(MP_REMOVE, iCompleteFileSelected > 0 ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.SetDefaultItem(bSingleCompleteFileSelected ? MP_OPEN : -1);
	m_SharedFilesMenu.EnableMenuItem(MP_CMT, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(MP_DETAIL, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(thePrefs.GetShowCopyEd2kLinkCmd() ? MP_GETED2KLINK : MP_SHOWED2KLINK, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);

	// Xman: IcEcRacKer Copy UL-feedback
	m_SharedFilesMenu.EnableMenuItem(MP_ULFEEDBACK, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	//Xman end

	m_SharedFilesMenu.EnableMenuItem(MP_FIND, GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED);

	//Xman Mass Rename (Morph)
	m_SharedFilesMenu.EnableMenuItem(MP_MASSRENAME, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	//Xman end

	m_CollectionsMenu.EnableMenuItem(MP_MODIFYCOLLECTION, ( pSingleSelFile != NULL && pSingleSelFile->m_pCollection != NULL ) ? MF_ENABLED : MF_GRAYED);
	m_CollectionsMenu.EnableMenuItem(MP_VIEWCOLLECTION, ( pSingleSelFile != NULL && pSingleSelFile->m_pCollection != NULL ) ? MF_ENABLED : MF_GRAYED);
	m_CollectionsMenu.EnableMenuItem(MP_SEARCHAUTHOR, ( pSingleSelFile != NULL && pSingleSelFile->m_pCollection != NULL && !pSingleSelFile->m_pCollection->GetAuthorKeyHashString().IsEmpty()) ? MF_ENABLED : MF_GRAYED);
#if defined(_DEBUG)
	if (thePrefs.IsExtControlsEnabled()){
		//JOHNTODO: Not for release as we need kad lowID users in the network to see how well this work work. Also, we do not support these links yet.
		if (iSelectedItems > 0 && theApp.IsConnected() && theApp.IsFirewalled() && theApp.clientlist->GetBuddy())
			m_SharedFilesMenu.EnableMenuItem(MP_GETKADSOURCELINK, MF_ENABLED);
		else
			m_SharedFilesMenu.EnableMenuItem(MP_GETKADSOURCELINK, MF_GRAYED);
	}
#endif
	m_SharedFilesMenu.EnableMenuItem(Irc_SetSendLink, iSelectedItems == 1 && theApp.emuledlg->ircwnd->IsConnected() ? MF_ENABLED : MF_GRAYED);


	CTitleMenu WebMenu;
	WebMenu.CreateMenu();
	WebMenu.AddMenuTitle(NULL, true);
	int iWebMenuEntries = theWebServices.GetFileMenuEntries(&WebMenu);
	UINT flag2 = (iWebMenuEntries == 0 || iSelectedItems != 1) ? MF_GRAYED : MF_STRING;
	m_SharedFilesMenu.AppendMenu(flag2 | MF_POPUP, (UINT_PTR)WebMenu.m_hMenu, GetResString(IDS_WEBSERVICES), _T("WEB"));


	GetPopupMenuPos(*this, point);
	m_SharedFilesMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON,point.x,point.y,this);

	m_SharedFilesMenu.RemoveMenu(m_SharedFilesMenu.GetMenuItemCount()-1,MF_BYPOSITION);
	VERIFY( WebMenu.DestroyMenu() );
	if (uInsertedMenuItem)
		VERIFY( m_SharedFilesMenu.RemoveMenu(uInsertedMenuItem, MF_BYCOMMAND) );
}

BOOL CAudioListCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	wParam = LOWORD(wParam);

	CTypedPtrList<CPtrList, CKnownFile*> selectedList;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos != NULL){
		int index = GetNextSelectedItem(pos);
		if (index >= 0)
			selectedList.AddTail((CKnownFile*)GetItemData(index));
	}

	if (   wParam == MP_CREATECOLLECTION
		|| wParam == MP_FIND
		|| selectedList.GetCount() > 0)
	{
		CKnownFile* file = NULL;
		if (selectedList.GetCount() == 1)
			file = selectedList.GetHead();

		switch (wParam)
		{
case Irc_SetSendLink:
	if (file)
		theApp.emuledlg->ircwnd->SetSendFileString(CreateED2kLink(file));
	break;
case MP_GETED2KLINK:{
	CString str;
	POSITION pos = selectedList.GetHeadPosition();
	while (pos != NULL)
	{
		file = selectedList.GetNext(pos);
		if (!str.IsEmpty())
			str += _T("\r\n");
		str += CreateED2kLink(file);
	}
	theApp.CopyTextToClipboard(str);
	break;
					}
#if defined(_DEBUG)
					//JOHNTODO: Not for release as we need kad lowID users in the network to see how well this work work. Also, we do not support these links yet.
case MP_GETKADSOURCELINK:{
	CString str;
	POSITION pos = selectedList.GetHeadPosition();
	while (pos != NULL)
	{
		file = selectedList.GetNext(pos);
		if (!str.IsEmpty())
			str += _T("\r\n");
		str += theApp.CreateKadSourceLink(file);
	}
	theApp.CopyTextToClipboard(str);
	break;
						 }
#endif
						 // file operations
case MP_OPEN:
case IDA_ENTER:
	if (file && !file->IsPartFile())
		OpenFile(file);
	break; 
	// X-Ray :: Toolbar :: Cleanup :: Start
	/*
	case MP_INSTALL_SKIN:
	if (file && !file->IsPartFile())
	InstallSkin(file->GetFilePath());
	break;
	*/
	// X-Ray :: Toolbar :: Cleanup :: End
case MP_OPENFOLDER:
	if (file && !file->IsPartFile())
		ShellExecute(NULL, _T("open"), file->GetPath(), NULL, NULL, SW_SHOW);
	break; 
case MP_RENAME:
case MPG_F2:
	if (file && !file->IsPartFile()){
		InputBox inputbox;
		CString title = GetResString(IDS_RENAME);
		title.Remove(_T('&'));
		inputbox.SetLabels(title, GetResString(IDS_DL_FILENAME), file->GetFileName());
		inputbox.SetEditFilenameMode();
		inputbox.DoModal();
		CString newname = inputbox.GetInput();
		if (!inputbox.WasCancelled() && newname.GetLength()>0)
		{
			// at least prevent users from specifying something like "..\dir\file"
			static const TCHAR _szInvFileNameChars[] = _T("\\/:*?\"<>|");
			if (newname.FindOneOf(_szInvFileNameChars) != -1){
				AfxMessageBox(GetErrorMessage(ERROR_BAD_PATHNAME));
				break;
			}

			CString newpath;
			PathCombine(newpath.GetBuffer(MAX_PATH), file->GetPath(), newname);
			newpath.ReleaseBuffer();
			if (_trename(file->GetFilePath(), newpath) != 0){
				CString strError;
				strError.Format(GetResString(IDS_ERR_RENAMESF), file->GetFilePath(), newpath, _tcserror(errno));
				AfxMessageBox(strError);
				break;
			}

			if (file->IsKindOf(RUNTIME_CLASS(CPartFile)))
			{
				file->SetFileName(newname);
				STATIC_DOWNCAST(CPartFile, file)->SetFullName(newpath); 
			}
			else
			{
				theApp.sharedfiles->RemoveKeywords(file);
				file->SetFileName(newname);
				theApp.sharedfiles->AddKeywords(file);
			}
			file->SetFilePath(newpath);
			UpdateFile(file);
		}
	}
	else
		MessageBeep(MB_OK);
	break;
case MP_REMOVE:
case MPG_DELETE:
	{
		if (IDNO == AfxMessageBox(GetResString(IDS_CONFIRM_FILEDELETE),MB_ICONWARNING | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_YESNO))
			return TRUE;

		SetRedraw(FALSE);
		bool bRemovedItems = false;
		while (!selectedList.IsEmpty())
		{
			CKnownFile* myfile = selectedList.RemoveHead();
			if (!myfile || myfile->IsPartFile())
				continue;

			BOOL delsucc = FALSE;
			if (!PathFileExists(myfile->GetFilePath()))
				delsucc = TRUE;
			else{
				// Delete
				if (!thePrefs.GetRemoveToBin()){
					delsucc = DeleteFile(myfile->GetFilePath());
				}
				else{
					// delete to recycle bin :(
					TCHAR todel[MAX_PATH+1];
					memset(todel, 0, sizeof todel);
					_tcsncpy(todel, myfile->GetFilePath(), ARRSIZE(todel)-2);

					SHFILEOPSTRUCT fp = {0};
					fp.wFunc = FO_DELETE;
					fp.hwnd = theApp.emuledlg->m_hWnd;
					fp.pFrom = todel;
					fp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;// | FOF_NOERRORUI
					delsucc = (SHFileOperation(&fp) == 0);
				}
			}
			if (delsucc){
				theApp.sharedfiles->RemoveFile(myfile);
				bRemovedItems = true;
				if (myfile->IsKindOf(RUNTIME_CLASS(CPartFile)))
					theApp.emuledlg->transferwnd->downloadlistctrl.ClearCompleted(static_cast<CPartFile*>(myfile));
			}
			else{
				CString strError;
				strError.Format( GetResString(IDS_ERR_DELFILE) + _T("\r\n\r\n%s"), myfile->GetFilePath(), GetErrorMessage(GetLastError()));
				AfxMessageBox(strError);
			}
		}
		SetRedraw(TRUE);
		if (bRemovedItems)
		{
			AutoSelectItem();
			// Depending on <no-idea> this does not always cause a
			// LVN_ITEMACTIVATE message sent. So, explicitly redraw
			// the item.
			theApp.emuledlg->sharedfileswnd->ShowSelectedFilesSummary();
		}
		break; 
	}
case MP_CMT:
	ShowFileDialog(selectedList, IDD_COMMENT);
	break; 
case MPG_ALTENTER:
case MP_DETAIL:
	ShowFileDialog(selectedList);
	break;
case MP_FIND:
	OnFindStart();
	break;
case MP_CREATECOLLECTION:
	{
		CCollection* pCollection = new CCollection();
		POSITION pos = selectedList.GetHeadPosition();
		while (pos != NULL)
		{
			pCollection->AddFileToCollection(selectedList.GetNext(pos),true);
		}
		CCollectionCreateDialog dialog;
		dialog.SetCollection(pCollection,true);
		dialog.DoModal();
		//We delete this collection object because when the newly created
		//collection file is added to the sharedfile list, it is read and verified
		//and which creates the colleciton object that is attached to that file..
		delete pCollection;
		break;
	}
case MP_SEARCHAUTHOR:
	{
		if (selectedList.GetCount() == 1 && file->m_pCollection)
		{
			SSearchParams* pParams = new SSearchParams;
			pParams->strExpression = file->m_pCollection->GetCollectionAuthorKeyString();
			pParams->eType = SearchTypeKademlia;
			pParams->strFileType = ED2KFTSTR_EMULECOLLECTION;
			pParams->strSpecialTitle = file->m_pCollection->m_sCollectionAuthorName;
			if (pParams->strSpecialTitle.GetLength() > 50){
				pParams->strSpecialTitle = pParams->strSpecialTitle.Left(50) + _T("...");
			}
			theApp.emuledlg->searchwnd->m_pwndResults->StartSearch(pParams);
		}
		break;
	}
case MP_VIEWCOLLECTION:
	{
		if (selectedList.GetCount() == 1 && file->m_pCollection)
		{
			CCollectionViewDialog dialog;
			dialog.SetCollection(file->m_pCollection);
			dialog.DoModal();
		}
		break;
	}
case MP_MODIFYCOLLECTION:
	{
		if (selectedList.GetCount() == 1 && file->m_pCollection)
		{
			CCollectionCreateDialog dialog;
			CCollection* pCollection = new CCollection(file->m_pCollection);
			dialog.SetCollection(pCollection,false);
			dialog.DoModal();
			delete pCollection;				
		}
		break;
	}
case MP_SHOWED2KLINK:
	ShowFileDialog(selectedList, IDD_ED2KLINK);
	break;
case MP_PRIOVERYLOW:
case MP_PRIOLOW:
case MP_PRIONORMAL:
case MP_PRIOHIGH:
case MP_PRIOVERYHIGH:
case MP_PRIOPOWER:  //Xman PowerRelease
case MP_PRIOAUTO:
	{
		SetRedraw(FALSE); //Xman Code Improvement
		POSITION pos = selectedList.GetHeadPosition();
		while (pos != NULL)
		{
			CKnownFile* file = selectedList.GetNext(pos);
			switch (wParam) {
case MP_PRIOVERYLOW:
	file->SetAutoUpPriority(false);
	file->SetUpPriority(PR_VERYLOW);
	UpdateFile(file);
	break;
case MP_PRIOLOW:
	file->SetAutoUpPriority(false);
	file->SetUpPriority(PR_LOW);
	UpdateFile(file);
	break;
case MP_PRIONORMAL:
	file->SetAutoUpPriority(false);
	file->SetUpPriority(PR_NORMAL);
	UpdateFile(file);
	break;
case MP_PRIOHIGH:
	file->SetAutoUpPriority(false);
	file->SetUpPriority(PR_HIGH);
	UpdateFile(file);
	break;
case MP_PRIOVERYHIGH:
	file->SetAutoUpPriority(false);
	file->SetUpPriority(PR_VERYHIGH);
	UpdateFile(file);
	break;	
	//Xman PowerRelease
case MP_PRIOPOWER:
	if(file->IsPartFile()) //only to be sure
		break;
	file->SetAutoUpPriority(false);
	file->SetUpPriority(PR_POWER);
	UpdateFile(file);
	break;
	//Xman end
case MP_PRIOAUTO:
	file->SetAutoUpPriority(true);
	//Xman advanced upload-priority
	if (thePrefs.UseAdvancedAutoPtio())
#ifdef _BETA
		file->CalculateAndSetUploadPriority2(); 
#else
		file->CalculateAndSetUploadPriority(); 
#endif
	else
		file->UpdateAutoUpPriority();
	//Xman end
	UpdateFile(file); 
	break;
			}
		}
		SetRedraw(TRUE); //Xman Code Improvement

		break;
	}
	// Xman: idea: IcEcRacKer Copy UL-feedback
case MP_ULFEEDBACK: 
	{ 
		CString feed; 

		bool morefiles = selectedList.GetCount() > 1;
		uint64 sumTransferred=0;
		uint64 sumAllTimeTransferred=0;

		if(!selectedList.IsEmpty())
		{
			feed.AppendFormat(_T("%s: %s \r\n"), GetResString(IDS_SF_STATISTICS),thePrefs.GetUserNick()); 
			feed.AppendFormat(_T("Mod: %s%s[%s] \r\n"),_T("DreaMule"), theApp.m_strCurVersionLong, MOD_VERSION);  
		}

		while (!selectedList.IsEmpty())
		{
			CKnownFile* file = selectedList.RemoveHead();
			sumTransferred += file->statistic.GetTransferred();
			sumAllTimeTransferred += file->statistic.GetAllTimeTransferred();

			feed.AppendFormat(_T("%s: %s \r\n"),GetResString(IDS_DL_FILENAME),file->GetFileName()); 
			feed.AppendFormat(_T("%s: %s \r\n"),GetResString(IDS_TYPE),file->GetFileType()); 
			feed.AppendFormat(_T("%s: %s\r\n"),GetResString(IDS_DL_SIZE), CastItoXBytes(file->GetFileSize(), false, false)); 
			CPartFile* pfile = (CPartFile*)file; 
			if(pfile && pfile->IsPartFile()) 
				feed.AppendFormat(_T("%s %.1f%%\r\n"), GetResString(IDS_FD_COMPSIZE), pfile->GetPercentCompleted()); 
			else 
				feed.AppendFormat(_T("%s 100%%\r\n"), GetResString(IDS_FD_COMPSIZE)); 
			feed.AppendFormat(_T("%s: %s (%s) \r\n"),GetResString(IDS_SF_TRANSFERRED), CastItoXBytes(file->statistic.GetTransferred(), false, false), CastItoXBytes(file->statistic.GetAllTimeTransferred(), false, false));   
			feed.AppendFormat(_T("%s: %u (%u)\r\n"),GetResString(IDS_COMPLSOURCES),file->m_nCompleteSourcesCountLo, file->m_nVirtualCompleteSourcesCount); 
			feed.AppendFormat(_T("%s: %u \r\n"),GetResString(IDS_ONQUEUE),(file->GetOnUploadqueue()));  //Xman see OnUploadqueue
			feed.AppendFormat(_T("%s: %u (%u) \r\n\r\n"),GetResString(IDS_SF_ACCEPTS),file->statistic.GetAccepts(),(file->statistic.GetAllTimeAccepts())); 

		}
		if(morefiles)
			feed.AppendFormat(_T("sum: %s: %s (%s) \r\n\r\n"),GetResString(IDS_SF_TRANSFERRED), CastItoXBytes(sumTransferred, false, false), CastItoXBytes(sumAllTimeTransferred, false, false));   

		theApp.CopyTextToClipboard(feed); 
		break; 
	} 
	//Xman end

	//Xman Mass Rename (Morph)
case MP_MASSRENAME: 
	{
		CMassRenameDialog MRDialog;
		// Add the files to the dialog
		POSITION pos = selectedList.GetHeadPosition();
		while (pos != NULL) {
			CKnownFile*  file = selectedList.GetAt (pos);
			MRDialog.m_FileList.AddTail (file);
			selectedList.GetNext (pos);
		}
		int result = MRDialog.DoModal ();
		if (result == IDOK) {
			// The user has successfully entered new filenames. Now we have
			// to rename all the files...
			POSITION pos = selectedList.GetHeadPosition();
			int i=0;
			while (pos != NULL) {
				CString newname = MRDialog.m_NewFilenames.at (i);
				CString newpath = MRDialog.m_NewFilePaths.at (i);
				CKnownFile* file = selectedList.GetAt (pos);
				// .part files could be renamed by simply changing the filename
				// in the CKnownFile object.
				if ((!file->IsPartFile()) && (_trename(file->GetFilePath(), newpath) != 0)){
					// Use the "Format"-Syntax of AddLogLine here instead of
					// CString.Format+AddLogLine, because if "%"-characters are
					// in the string they would be misinterpreted as control sequences!
					AddLogLine(false,_T("Não foi possivel renomear '%s' para '%s', Erro: %hs"), file->GetFilePath(), newpath, _tcserror(errno));
				} else {
					CString strres;
					if (!file->IsPartFile()) {
						// Use the "Format"-Syntax of AddLogLine here instead of
						// CString.Format+AddLogLine, because if "%"-characters are
						// in the string they would be misinterpreted as control sequences!
						AddLogLine(false,_T("Renomeado com sucesso '%s' para '%s'"), file->GetFilePath(), newpath);
						file->SetFileName(newname);
						if (file->IsKindOf(RUNTIME_CLASS(CPartFile)))
							((CPartFile*) file)->SetFullName(newpath);
					} else {
						// Use the "Format"-Syntax of AddLogLine here instead of
						// CString.Format+AddLogLine, because if "%"-characters are
						// in the string they would be misinterpreted as control sequences!
						AddLogLine(false,_T("Renomeado com sucesso .part file '%s' para '%s'"), file->GetFileName(), newname);
						file->SetFileName(newname, true); 
						((CPartFile*) file)->UpdateDisplayedInfo();
						((CPartFile*) file)->SavePartFile(); 
					}
					file->SetFilePath(newpath);
					UpdateFile(file);
				}

				// Next item
				selectedList.GetNext (pos);
				i++;
			}
		}
		break;
	}
	//Xman end
//>>> WiZaRd::MediaPlayer
				case MP_VLCVIEW:
					{
						if(file)
							theApp.emuledlg->mediaplayerwnd->StartPlayback(file);
						break;
					}
//<<< WiZaRd::MediaPlayer
default:
	if (wParam>=MP_WEBURL && wParam<=MP_WEBURL+256){
		theWebServices.RunURL(file, wParam);
	}
	break;
		}
	}
	return TRUE;
}

void CAudioListCtrl::OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// Barry - Store sort order in preferences
	// Determine ascending based on whether already sorted on this column

	bool sortAscending = (GetSortItem() != pNMListView->iSubItem) ? true : !GetSortAscending();

	// Sort table
	SetSortArrow(pNMListView->iSubItem, sortAscending); 

	UpdateSortHistory(pNMListView->iSubItem + (sortAscending ? 0:20),20);
	SortItems(SortProc, pNMListView->iSubItem + (sortAscending ? 0:20));

	*pResult = 0;
}

int CAudioListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CKnownFile* item1 = (CKnownFile*)lParam1;
	CKnownFile* item2 = (CKnownFile*)lParam2;

	int iResult=0;
	switch(lParamSort)	
	{
		case 0: //filename asc
			iResult = CompareLocaleStringNoCase(item1->GetFileName(), item2->GetFileName());
			break;
		case 20: //filename desc
			iResult=CompareLocaleStringNoCase(item2->GetFileName(), item1->GetFileName());
			break;

		case 1:
			iResult = CompareOptLocaleStringNoCase(item1->GetStrTagValue(FT_MEDIA_ARTIST), item2->GetStrTagValue(FT_MEDIA_ARTIST));
			break;
		case 21:
			iResult = CompareOptLocaleStringNoCase(item2->GetStrTagValue(FT_MEDIA_ARTIST), item1->GetStrTagValue(FT_MEDIA_ARTIST));
			break;

		case 2:
			iResult = CompareOptLocaleStringNoCase(item1->GetStrTagValue(FT_MEDIA_ALBUM), item2->GetStrTagValue(FT_MEDIA_ALBUM));
			break;
		case 22:
			iResult = CompareOptLocaleStringNoCase(item2->GetStrTagValue(FT_MEDIA_ALBUM), item1->GetStrTagValue(FT_MEDIA_ALBUM));
			break;

		case 3:
			iResult = CompareOptLocaleStringNoCase(item1->GetStrTagValue(FT_MEDIA_TITLE), item2->GetStrTagValue(FT_MEDIA_TITLE));
			break;
		case 23:
			iResult = CompareOptLocaleStringNoCase(item2->GetStrTagValue(FT_MEDIA_TITLE), item1->GetStrTagValue(FT_MEDIA_TITLE));
			break;

		case 4:
			iResult = CompareUnsigned(item1->GetIntTagValue(FT_MEDIA_LENGTH), item2->GetIntTagValue(FT_MEDIA_LENGTH));
			break;
		case 24:
			iResult = CompareUnsigned(item2->GetIntTagValue(FT_MEDIA_LENGTH), item1->GetIntTagValue(FT_MEDIA_LENGTH));
			break;

		case 5:
			iResult = CompareUnsigned(item1->GetIntTagValue(FT_MEDIA_BITRATE), item2->GetIntTagValue(FT_MEDIA_BITRATE));
			break;
		case 25:
			iResult = CompareUnsigned(item2->GetIntTagValue(FT_MEDIA_BITRATE), item1->GetIntTagValue(FT_MEDIA_BITRATE));
			break;

		case 6:
			iResult = CompareOptLocaleStringNoCase(item1->GetFileComment(), item2->GetFileComment());
			break;
		case 26:
			iResult = CompareOptLocaleStringNoCase(item2->GetFileComment(), item1->GetFileComment());
			break;

		default: 
			iResult=0;
			break;
	}

	// SLUGFILLER: multiSort remove - handled in parent class
	/*
	int dwNextSort;
	//call secondary sortorder, if this one results in equal
	//(Note: yes I know this call is evil OO wise, but better than changing a lot more code, while we have only one instance anyway - might be fixed later)
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->sharedfileswnd->sharedfilesctrl.GetNextSortOrder(lParamSort)) != (-1)){
	iResult= SortProc(lParam1, lParam2, dwNextSort);
	}
	*/
	return iResult;
}

void CAudioListCtrl::OpenFile(const CKnownFile* file)
{
	if(file->m_pCollection)
	{
		CCollectionViewDialog dialog;
		dialog.SetCollection(file->m_pCollection);
		dialog.DoModal();
	}
	else
		ShellOpenFile(file->GetFilePath(), NULL);
}

void CAudioListCtrl::OnNMDblclk(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1)
	{
		CKnownFile* file = (CKnownFile*)GetItemData(iSel);
		if (file)
		{
			if (GetKeyState(VK_MENU) & 0x8000)
			{
				CTypedPtrList<CPtrList, CKnownFile*> aFiles;
				aFiles.AddHead(file);
				ShowFileDialog(aFiles);
			}
			else if (!file->IsPartFile())
				OpenFile(file);
		}
	}
	*pResult = 0;
}

void CAudioListCtrl::CreateMenues()
{
	if (m_PrioMenu) VERIFY( m_PrioMenu.DestroyMenu() );
	if (m_CollectionsMenu) VERIFY( m_CollectionsMenu.DestroyMenu() );
	if (m_SharedFilesMenu) VERIFY( m_SharedFilesMenu.DestroyMenu() );


	m_PrioMenu.CreateMenu();
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOVERYLOW,GetResString(IDS_PRIOVERYLOW));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOLOW,GetResString(IDS_PRIOLOW));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIONORMAL,GetResString(IDS_PRIONORMAL));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOHIGH, GetResString(IDS_PRIOHIGH));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOVERYHIGH, GetResString(IDS_PRIORELEASE));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOPOWER, GetResString(IDS_POWERRELEASE)); //Xman PowerRelease
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOAUTO, GetResString(IDS_PRIOAUTO));//UAP

	m_CollectionsMenu.CreateMenu();
	m_CollectionsMenu.AddMenuTitle(NULL, true);
	m_CollectionsMenu.AppendMenu(MF_STRING,MP_CREATECOLLECTION, GetResString(IDS_CREATECOLLECTION), _T("COLLECTION_ADD"));
	m_CollectionsMenu.AppendMenu(MF_STRING,MP_MODIFYCOLLECTION, GetResString(IDS_MODIFYCOLLECTION), _T("COLLECTION_EDIT"));
	m_CollectionsMenu.AppendMenu(MF_STRING,MP_VIEWCOLLECTION, GetResString(IDS_VIEWCOLLECTION), _T("COLLECTION_VIEW"));
	m_CollectionsMenu.AppendMenu(MF_STRING,MP_SEARCHAUTHOR, GetResString(IDS_SEARCHAUTHORCOLLECTION), _T("COLLECTION_SEARCH"));

	m_SharedFilesMenu.CreatePopupMenu();
	m_SharedFilesMenu.AddMenuTitle(GetResString(IDS_SHAREDFILES), true);

	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_OPEN, GetResString(IDS_OPENFILE), _T("OPENFILE"));
//>>> WiZaRd::MediaPlayer
	m_SharedFilesMenu.AppendMenu(MF_SEPARATOR);
	m_SharedFilesMenu.AppendMenu(MF_STRING, MP_VLCVIEW, GetResString(IDS_VLC_PREVIEW), _T("PREVIEW"));
//<<< WiZaRd::MediaPlayer
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_OPENFOLDER, GetResString(IDS_OPENFOLDER), _T("OPENFOLDER"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_RENAME, GetResString(IDS_RENAME) + _T("..."), _T("FILERENAME"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_REMOVE, GetResString(IDS_DELETE), _T("DELETE"));
	if (thePrefs.IsExtControlsEnabled())
		m_SharedFilesMenu.AppendMenu(MF_STRING,Irc_SetSendLink,GetResString(IDS_IRC_ADDLINKTOIRC), _T("IRCCLIPBOARD"));

	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_PrioMenu.m_hMenu, GetResString(IDS_PRIORITY) + _T(" (") + GetResString(IDS_PW_CON_UPLBL) + _T(")"), _T("FILEPRIORITY"));
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);

	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_CollectionsMenu.m_hMenu, GetResString(IDS_META_COLLECTION), _T("COLLECTION"));
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR); 	

	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_DETAIL, GetResString(IDS_SHOWDETAILS), _T("FILEINFO"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_CMT, GetResString(IDS_CMT_ADD), _T("FILECOMMENTS")); 
	if (thePrefs.GetShowCopyEd2kLinkCmd())
		m_SharedFilesMenu.AppendMenu(MF_STRING,MP_GETED2KLINK, GetResString(IDS_DL_LINK1), _T("ED2KLINK") );
	else
		m_SharedFilesMenu.AppendMenu(MF_STRING,MP_SHOWED2KLINK, GetResString(IDS_DL_SHOWED2KLINK), _T("ED2KLINK") );
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_FIND, GetResString(IDS_FIND), _T("Search"));
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR); 

	// Xman: IcEcRacKer Copy UL-feedback
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_ULFEEDBACK, _T("Copiar Feedback de UL"), _T("FILECOMMENTS")); 
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR); 
	//Xman end

	//Xman Mass Rename (Morph)
	if (thePrefs.IsExtControlsEnabled())
	{
		m_SharedFilesMenu.AppendMenu(MF_STRING,MP_MASSRENAME,GetResString(IDS_MR), _T("FILEMASSRENAME"));
		m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR); 
	}
	//Xman end

#if defined(_DEBUG)
	if (thePrefs.IsExtControlsEnabled()){
		//JOHNTODO: Not for release as we need kad lowID users in the network to see how well this work work. Also, we do not support these links yet.
		m_SharedFilesMenu.AppendMenu(MF_STRING,MP_GETKADSOURCELINK, _T("Copy eD2K Links To Clipboard (Kad)"));
		m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	}
#endif
}

void CAudioListCtrl::ShowComments(CKnownFile* file)
{
	if (file)
	{
		CTypedPtrList<CPtrList, CKnownFile*> aFiles;
		aFiles.AddHead(file);
		ShowFileDialog(aFiles, IDD_COMMENT);
	}
}

void CAudioListCtrl::OnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

	if (theApp.emuledlg->IsRunning()){
		// Although we have an owner drawn listview control we store the text for the primary item in the listview, to be
		// capable of quick searching those items via the keyboard. Because our listview items may change their contents,
		// we do this via a text callback function. The listview control will send us the LVN_DISPINFO notification if
		// it needs to know the contents of the primary item.
		//
		// But, the listview control sends this notification all the time, even if we do not search for an item. At least
		// this notification is only sent for the visible items and not for all items in the list. Though, because this
		// function is invoked *very* often, no *NOT* put any time consuming code here in.

		if (pDispInfo->item.mask & LVIF_TEXT){
			const CKnownFile* pFile = reinterpret_cast<CKnownFile*>(pDispInfo->item.lParam);
			if (pFile != NULL){
				switch (pDispInfo->item.iSubItem){
case 0:
	if (pDispInfo->item.cchTextMax > 0){
		_tcsncpy(pDispInfo->item.pszText, pFile->GetFileName(), pDispInfo->item.cchTextMax);
		pDispInfo->item.pszText[pDispInfo->item.cchTextMax-1] = _T('\0');
	}
	break;
default:
	// shouldn't happen
	pDispInfo->item.pszText[0] = _T('\0');
	break;
				}
			}
		}
	}
	*pResult = 0;
}

void CAudioListCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == 'C' && (GetKeyState(VK_CONTROL) & 0x8000))
	{
		// Ctrl+C: Copy listview items to clipboard
		SendMessage(WM_COMMAND, MP_GETED2KLINK);
		return;
	}
	else if (nChar == VK_F5)
		theApp.emuledlg->sharedfileswnd->sharedfilesctrl.ReloadFileList();

	CMuleListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CAudioListCtrl::ShowFileDialog(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage)
{
	if (aFiles.GetSize() > 0)
	{
		CSharedFileDetailsSheet dialog(aFiles, uPshInvokePage, this);
		dialog.DoModal();
	}
}

//>>> WiZaRd::ShareFilter
void CAudioListCtrl::GetItemDisplayText(const CKnownFile* file, const int iSubItem, LPTSTR pszText, const int cchTextMax) const
{
	if (pszText == NULL || cchTextMax <= 0) {
		ASSERT(0);
		return;
	}
	pszText[0] = _T('\0');
	CString buffer = L"";
	switch (iSubItem)
	{
		case 0:
			buffer = file->GetFileName();
			break;
		case 1:
			buffer = file->GetStrTagValue(FT_MEDIA_ARTIST);
			break;
		case 2:
			buffer = file->GetStrTagValue(FT_MEDIA_ALBUM);
			break;
		case 3:
			buffer = file->GetStrTagValue(FT_MEDIA_TITLE);
			break;
		case 4:
		{
			uint32 nMediaLength = file->GetIntTagValue(FT_MEDIA_LENGTH);
			if (nMediaLength)
				SecToTimeLength(nMediaLength, buffer);
			else 
				buffer.Empty();
			break;
		}
		case 5:
		{
			uint32 nBitrate = file->GetIntTagValue(FT_MEDIA_BITRATE);
			if (nBitrate)
				buffer.Format(_T("%u %s"), nBitrate, GetResString(IDS_KBITSSEC));
			else
				buffer.Empty();
			break;
		}
		case 6:
			buffer = ((CKnownFile*)file)->GetFileComment();
			if(buffer.IsEmpty())
				buffer = GetResString(IDS_COMMENTME);
			break;
		default:
			break;
	}
	_tcsncpy(pszText, buffer, cchTextMax);
	pszText[cchTextMax - 1] = _T('\0');
}
//<<< WiZaRd::ShareFilter