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
#include "stdafx.h"
#include "SearchListCtrl.h"
#include "emule.h"
#include "ResizableLib/ResizableSheet.h"
#include "SearchFile.h"
#include "SearchList.h"
#include "emuledlg.h"
#include "MetaDataDlg.h"
#include "SearchDlg.h"
#include "SearchParams.h"
#include "ClosableTabCtrl.h"
#include "UpDownClient.h"
#include "ClientList.h"
#include "MemDC.h"
#include "SharedFileList.h"
#include "DownloadQueue.h"
#include "PartFile.h"
#include "KnownFileList.h"
#include "MenuCmds.h"
#include "OtherFunctions.h"
#include "Opcodes.h"
#include "Packets.h"
#include "Log.h"
#include "HighColorTab.hpp"
#include "ListViewWalkerPropertySheet.h"
#include "UserMsgs.h"
#include "ToolTipCtrlX.h"
#include "SearchDlg.h"
#include "SearchResultsWnd.h"
#include "sockets.h"
#include "server.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define COLLAPSE_ONLY	0
#define EXPAND_ONLY		1
#define EXPAND_COLLAPSE	2

#define	TREE_WIDTH		10


//////////////////////////////////////////////////////////////////////////////
// CSearchResultFileDetailSheet

class CSearchResultFileDetailSheet : public CListViewWalkerPropertySheet
{
	DECLARE_DYNAMIC(CSearchResultFileDetailSheet)

public:
	CSearchResultFileDetailSheet(CAtlList<CSearchFile*>& paFiles, UINT uPshInvokePage = 0, CListCtrlItemWalk* pListCtrl = NULL);
	virtual ~CSearchResultFileDetailSheet();

protected:
	CMetaDataDlg m_wndMetaData;

	UINT m_uPshInvokePage;
	static LPCTSTR m_pPshStartPage;

	void UpdateTitle();

	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg LRESULT OnDataChanged(WPARAM, LPARAM);
};

LPCTSTR CSearchResultFileDetailSheet::m_pPshStartPage;

IMPLEMENT_DYNAMIC(CSearchResultFileDetailSheet, CListViewWalkerPropertySheet)

BEGIN_MESSAGE_MAP(CSearchResultFileDetailSheet, CListViewWalkerPropertySheet)
	ON_WM_DESTROY()
	ON_MESSAGE(UM_DATA_CHANGED, OnDataChanged)
END_MESSAGE_MAP()

CSearchResultFileDetailSheet::CSearchResultFileDetailSheet(CAtlList<CSearchFile*>& paFiles, UINT uPshInvokePage, CListCtrlItemWalk* pListCtrl)
	: CListViewWalkerPropertySheet(pListCtrl)
{
	m_uPshInvokePage = uPshInvokePage;
	POSITION pos = paFiles.GetHeadPosition();
	while (pos)
		m_aItems.Add(paFiles.GetNext(pos));
	m_psh.dwFlags &= ~PSH_HASHELP;
	m_psh.dwFlags |= PSH_NOAPPLYNOW;
	
	m_wndMetaData.m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndMetaData.m_psp.dwFlags |= PSP_USEICONID;
	//m_wndMetaData.m_psp.pszIcon = _T("METADATA");
	if (m_aItems.GetSize() == 1 && thePrefs.IsExtControlsEnabled()) {
		m_wndMetaData.SetFiles(&m_aItems);
		AddPage(&m_wndMetaData);
	}

	LPCTSTR pPshStartPage = m_pPshStartPage;
	if (m_uPshInvokePage != 0)
		pPshStartPage = MAKEINTRESOURCE(m_uPshInvokePage);
	for (int i = 0; i < m_pages.GetSize(); i++)
	{
		CPropertyPage* pPage = GetPage(i);
		if (pPage->m_psp.pszTemplate == pPshStartPage)
		{
			m_psh.nStartPage = i;
			break;
		}
	}
}

CSearchResultFileDetailSheet::~CSearchResultFileDetailSheet()
{
}

void CSearchResultFileDetailSheet::OnDestroy()
{
	if (m_uPshInvokePage == 0)
		m_pPshStartPage = GetPage(GetActiveIndex())->m_psp.pszTemplate;
	CListViewWalkerPropertySheet::OnDestroy();
}

BOOL CSearchResultFileDetailSheet::OnInitDialog()
{		
	EnableStackedTabs(FALSE);
	BOOL bResult = CListViewWalkerPropertySheet::OnInitDialog();
	HighColorTab::UpdateImageList(*this);
	InitWindowStyles(this);
	EnableSaveRestore(_T("SearchResultFileDetailsSheet"), !thePrefs.prefReadonly); // call this after(!) OnInitDialog // X: [ROP] - [ReadOnlyPreference]
	UpdateTitle();
	return bResult;
}

LRESULT CSearchResultFileDetailSheet::OnDataChanged(WPARAM, LPARAM)
{
	UpdateTitle();
	return 1;
}

void CSearchResultFileDetailSheet::UpdateTitle()
{
	CString text = GetResString(IDS_DETAILS);
	if(m_aItems.GetSize() == 1)
		//text.AppendFormat(_T(": %s"), STATIC_DOWNCAST(CSearchFile, m_aItems[0])->GetFileName());
		text.AppendFormat(_T(": %s"), ((CSearchFile*)m_aItems[0])->GetFileName());
	SetWindowText(text);
}


//////////////////////////////////////////////////////////////////////////////
// CSearchListCtrl
static const UINT colStrID[]={
	 IDS_DL_FILENAME
	,IDS_DL_SIZE
	,IDS_SEARCHAVAIL
	,IDS_COMPLSOURCES
	,IDS_TYPE
	,IDS_FILEID
	,IDS_ARTIST
	,IDS_ALBUM
	,IDS_TITLE
	,IDS_LENGTH
	,IDS_BITRATE
	,IDS_CODEC
	,IDS_FOLDER
	,IDS_KNOWN
	,IDS_AICHHASH
};

IMPLEMENT_DYNAMIC(CSearchListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CSearchListCtrl, CMuleListCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnClick)
	ON_NOTIFY_REFLECT(LVN_DELETEALLITEMS, OnLvnDeleteAllItems)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnLvnGetDispInfo)
	ON_NOTIFY_REFLECT(LVN_GETINFOTIP, OnLvnGetInfoTip)
	ON_NOTIFY_REFLECT(LVN_KEYDOWN, OnLvnKeyDown)
	ON_NOTIFY_REFLECT(NM_CLICK, OnNmClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNmDblClk)
	ON_WM_CONTEXTMENU()
	ON_WM_KEYDOWN()
	ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

CSearchListCtrl::CSearchListCtrl()
	: CListCtrlItemWalk(this)
{
	searchlist = NULL;
	m_nResultsID = 0;
	m_tooltip = new CToolTipCtrlX;
}

void CSearchListCtrl::SetAllIcons()
{
	ApplyImageList(NULL);
	m_ImageList.DeleteImageList();
	m_ImageList.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);
	m_ImageList.Add(CTempIconLoader(_T("Spam"))); // spam indicator
	// Apply the image list also to the listview control, even if we use our own 'DrawItem'.
	// This is needed to give the listview control a chance to initialize the row height.
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) != 0 );
	VERIFY( ApplyImageList(m_ImageList) == NULL );

	// NOTE: There is another image list applied to this particular listview control!
	// See also the 'Init' function.
}

void CSearchListCtrl::SetGridLine()
{
 if (thePrefs.UseGridlines())
	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_GRIDLINES); 
   else
     SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
}

void CSearchListCtrl::Init(CSearchList* in_searchlist)
{
	SetPrefsKey(_T("SearchListCtrl"));
	ASSERT( (GetStyle() & LVS_SINGLESEL) == 0 );
	SetGridLine();

	CToolTipCtrl* tooltip = GetToolTips();
	if (tooltip){
		m_tooltip->SubclassWindow(*tooltip);
		tooltip->ModifyStyle(0, TTS_NOPREFIX);
		tooltip->SetDelayTime(TTDT_AUTOPOP, 20000);
		//tooltip->SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1000);
	}
	searchlist = in_searchlist;

	InsertColumn(0, GetResString(IDS_DL_FILENAME),	LVCFMT_LEFT,  DFLT_FILENAME_COL_WIDTH);
	InsertColumn(1, GetResString(IDS_DL_SIZE),		LVCFMT_RIGHT, DFLT_SIZE_COL_WIDTH);
	InsertColumn(2, GetResString(IDS_SEARCHAVAIL) + (thePrefs.IsExtControlsEnabled() ? _T(" (") + GetResString(IDS_DL_SOURCES) + _T(')') : _T("")), LVCFMT_RIGHT, 60);
	InsertColumn(3, GetResString(IDS_COMPLSOURCES),	LVCFMT_RIGHT,  70);
	InsertColumn(4, GetResString(IDS_TYPE),			LVCFMT_LEFT,  DFLT_FILETYPE_COL_WIDTH);
	InsertColumn(5, GetResString(IDS_FILEID),		LVCFMT_LEFT,  DFLT_HASH_COL_WIDTH	,		-1, true);
	InsertColumn(6, GetResString(IDS_ARTIST),		LVCFMT_LEFT,  DFLT_ARTIST_COL_WIDTH);
	InsertColumn(7, GetResString(IDS_ALBUM),		LVCFMT_LEFT,  DFLT_ALBUM_COL_WIDTH);
	InsertColumn(8, GetResString(IDS_TITLE),		LVCFMT_LEFT,  DFLT_TITLE_COL_WIDTH);
	InsertColumn(9, GetResString(IDS_LENGTH),		LVCFMT_RIGHT, DFLT_LENGTH_COL_WIDTH);
	InsertColumn(10,GetResString(IDS_BITRATE),		LVCFMT_RIGHT, DFLT_BITRATE_COL_WIDTH);
	InsertColumn(11,GetResString(IDS_CODEC),		LVCFMT_LEFT,  DFLT_CODEC_COL_WIDTH);
	InsertColumn(12,GetResString(IDS_FOLDER),		LVCFMT_LEFT,  DFLT_FOLDER_COL_WIDTH,		-1, true);
	InsertColumn(13,GetResString(IDS_KNOWN),		LVCFMT_LEFT,   50);
	InsertColumn(14,GetResString(IDS_AICHHASH),		LVCFMT_LEFT,  DFLT_HASH_COL_WIDTH	,		-1, true);

	SetAllIcons();

	// This states image list with that particular width is only there to let the listview control
	// auto-size the column width properly (double clicking on header divider). The items in the
	// list view contain a file type icon and optionally also a 'tree' icon (in case there are
	// more search entries related to one file hash). The width of that 'tree' icon (even if it is
	// not drawn) has to be known by the default list view control code to determine the total width
	// needed to show a particular item. The image list itself can be even empty, it is used by
	// the listview control just for querying the width of on image in the list, even if that image
	// was never added.
	/*CImageList imlDummyStates;
	imlDummyStates.Create(TREE_WIDTH, 16, ILC_COLOR, 0, 0);
	CImageList *pOldStates = SetImageList(&imlDummyStates, LVSIL_STATE);
	imlDummyStates.Detach();
	if (pOldStates)
		pOldStates->DeleteImageList();*/

	CreateMenues();

	LoadSettings();
	SetHighlightColors();

	// Barry - Use preferred sort order from preferences
	if (GetSortItem()!= -1){// don't force a sorting if '-1' is specified, so we can better see how the search results are arriving
		SetSortArrow();
		SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0:100));
	}
	IgnoredColums = 0x10;
	m_ctlListHeader.Attach(GetHeaderCtrl()->Detach());
}

CSearchListCtrl::~CSearchListCtrl()
{
#ifdef REPLACE_ATLMAP
	for (unordered_map<int, CSortSelectionState*>::const_iterator it = m_mapSortSelectionStates.begin(); it != m_mapSortSelectionStates.end(); ++it)
		delete it->second;
	m_mapSortSelectionStates.clear();
#else
	POSITION pos = m_mapSortSelectionStates.GetStartPosition();
	while (pos != NULL) {
		int nKey;
		CSortSelectionState* pValue;
		m_mapSortSelectionStates.GetNextAssoc(pos, nKey, pValue);
		delete pValue;
	}
	m_mapSortSelectionStates.RemoveAll();
#endif
	delete m_tooltip;
	m_ctlListHeader.Detach();
}

void CSearchListCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;
	for (int icol=0;icol<pHeaderCtrl->GetItemCount();icol++) {
		strRes=GetResString(colStrID[icol]);
		if(icol == 2 && thePrefs.IsExtControlsEnabled())
			strRes.Append(_T(" (") + GetResString(IDS_DL_SOURCES) + _T(')'));
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(icol, &hdi);
	}

	CreateMenues();
}

void CSearchListCtrl::AddResult(const CSearchFile* toshow)
{
	bool bItemFiltered = (filter>0 && (filter-1) != GetED2KFileTypeID(toshow->GetFileName())) || IsFilteredItem(toshow);

	// update tab-counter for the given searchfile
	CClosableTabCtrl& searchselect = theApp.emuledlg->searchwnd->GetSearchSelector();
	int iTabItems = searchselect.GetItemCount();
	if (iTabItems > 0)
	{
		TCITEM ti;
		ti.mask = TCIF_PARAM;
		for (int iItem = 0; iItem < iTabItems; iItem++)
		{
			if (searchselect.GetItem(iItem, &ti) && ti.lParam != NULL)
			{
				const SSearchParams* pSearchParams = (SSearchParams*)ti.lParam;
				if (pSearchParams->dwSearchID == toshow->GetSearchID())
				{
					int iAvailResults = searchlist->GetFoundFiles(toshow->GetSearchID());
					CString strTabLabel;
					if(m_nResultsID == pSearchParams->dwSearchID){
						size_t iFilteredResult = GetItemCount();
						if (!bItemFiltered)
							iFilteredResult++;
						strTabLabel.Format(_T("%s (%u/%u)"), pSearchParams->strSearchTitle, iFilteredResult, iAvailResults);
					}
					else
						strTabLabel.Format(_T("%s (%u)"), pSearchParams->strSearchTitle, iAvailResults);
					strTabLabel.Replace(_T("&"), _T("&&"));
					ti.pszText = const_cast<LPTSTR>((LPCTSTR)strTabLabel);
					ti.mask = TCIF_TEXT;
					searchselect.SetItem(iItem, &ti);
					if (searchselect.GetCurSel() != iItem)
						searchselect.HighlightItem(iItem);
					break;
				}
			}
		}
	}

	if (bItemFiltered || toshow->GetSearchID() != m_nResultsID)
		return;

	// Turn off updates
	EUpdateMode eCurUpdateMode = SetUpdateMode(none);
	// Add item
	int iItem = InsertItem(LVIF_TEXT | LVIF_PARAM, GetItemCount(), toshow->GetFileName(), 0, 0, 0, (LPARAM)toshow);
	// Add all sub items as callbacks and restore updating with last sub item.
	// The callbacks are only needed for 'Find' functionality, not for any drawing.
	const int iSubItems = 13;
	for (int i = 1; i <= iSubItems; i++)
	{
		if (i == iSubItems)
			SetUpdateMode(eCurUpdateMode);
		SetItemText(iItem, i, LPSTR_TEXTCALLBACK);
	}
}

void CSearchListCtrl::UpdateSources(const CSearchFile* toupdate)
{
	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)toupdate;
	int index = FindItem(&find);
	if (index != -1)
	{
		CString strBuffer;
		uint32 nSources = toupdate->GetSourceCount();	
		int iClients = toupdate->GetClientsCount();
		if (thePrefs.IsExtControlsEnabled() && iClients > 0)
			strBuffer.Format(_T("%u (%u)"), nSources, iClients);
		else
			strBuffer.Format(_T("%u"), nSources);
		SetItemText(index, 2, strBuffer);
		SetItemText(index, 3, GetCompleteSourcesDisplayString(toupdate, nSources));

		if (toupdate->IsListExpanded())
		{
			const SearchList* list = theApp.searchlist->GetSearchListForID(toupdate->GetSearchID());
			for (POSITION pos = list->GetHeadPosition(); pos != NULL; )
			{
				const CSearchFile* cur_file = list->GetNext(pos);
				if (cur_file->GetListParent() == toupdate)
				{
					LVFINDINFO find;
					find.flags = LVFI_PARAM;
					find.lParam = (LPARAM)cur_file;
					int index2 = FindItem(&find);
					if (index2 != -1)
						Update(index2);
					else
						InsertItem(LVIF_PARAM | LVIF_TEXT, index+1, cur_file->GetFileName(), 0, 0, 0, (LPARAM)cur_file);
				}
			}
		}
		Update(index);
	}
}

void CSearchListCtrl::UpdateSearch(CSearchFile* toupdate)
{
	if (!toupdate || !CemuleDlg::IsRunning())
		return;
	//MORPH START - SiRoB, Don't Refresh item if not needed
	if( theApp.emuledlg->activewnd != theApp.emuledlg->searchwnd/* || IsWindowVisible() == FALSE*/ )
		return;
	//MORPH END   - SiRoB, Don't Refresh item if not needed

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)toupdate;
	int index = FindItem(&find);
	if (index != -1)
	{
		Update(index);
	}
}

bool CSearchListCtrl::IsComplete(const CSearchFile *pFile, UINT uSources) const
{
	UINT uCompleteSources = pFile->GetIntTagValue(FT_COMPLETE_SOURCES);
	int iComplete = pFile->IsComplete(uSources, uCompleteSources);

	/*if (iComplete < 0)			// '< 0' ... unknown
		return true;			// treat 'unknown' as complete

	if (iComplete > 0)			// '> 0' ... we know it's complete
		return true;

	return false;				// '= 0' ... we know it's not complete*/
	return iComplete != 0;
}

CString CSearchListCtrl::GetCompleteSourcesDisplayString(const CSearchFile* pFile, UINT uSources/*, bool* pbComplete*/) const
{
	UINT uCompleteSources = pFile->GetIntTagValue(FT_COMPLETE_SOURCES);
	int iComplete = pFile->IsComplete(uSources, uCompleteSources);

	// If we have no 'Complete' info at all but the file size is <= PARTSIZE, we though know that the file
	// is complete (otherwise it would not be shared).
	if (iComplete < 0 && pFile->GetFileSize() <= (uint64)PARTSIZE) {
		iComplete = 1;
		// If this search result is from a remote client's shared file list, we know the 'complete' count.
		if (pFile->GetDirectory() != NULL)
			uCompleteSources = 1;
	}

	CString str;
	if (iComplete < 0)			// '< 0' ... unknown
	{
		str = _T('?');
		//if (pbComplete)
			//*pbComplete = true;	// treat 'unknown' as complete
	}
	else if (iComplete > 0)		// '> 0' ... we know it's complete
	{
		if (uSources && uCompleteSources) {
			str.Format(_T("%u%%"), (uCompleteSources*100)/uSources);
			if (thePrefs.IsExtControlsEnabled())
				str.AppendFormat(_T(" (%u)"), uCompleteSources);
		}
		else {
			// we know it's complete, but we don't know the degree.. (for files <= PARTSIZE in Kad searches)
			str = GetResString(IDS_YES);
		}
		//if (pbComplete)
			//*pbComplete = true;
	}
	else						// '= 0' ... we know it's not complete
	{
		str = _T("0%");
		if (thePrefs.IsExtControlsEnabled())
			str.Append(_T(" (0)"));// X: [CI] - [Code Improvement]
		//if (pbComplete)
			//*pbComplete = false;
	}
	return str;
}

void CSearchListCtrl::RemoveResult(const CSearchFile* toremove)
{
	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)toremove;
	int iItem = FindItem(&find);
	if (iItem != -1)
		DeleteItem(iItem);
}

void CSearchListCtrl::ShowResults(uint32 nResultsID)
{
	if (m_nResultsID != 0 && nResultsID != m_nResultsID){
		// store the current state
		CSortSelectionState* pCurState = new CSortSelectionState();
		POSITION pos = GetFirstSelectedItemPosition();
		while (pos != NULL){
			pCurState->m_aSelectedItems.Add(GetNextSelectedItem(pos));
		}
		pCurState->m_nSortItem = GetSortItem();
		pCurState->m_bSortAscending = GetSortAscending();
		pCurState->m_nScrollPosition = GetTopIndex();
		//Xman
		// SLUGFILLER: multiSort - save sort history
		pos = m_liSortHistory.GetHeadPosition();
		while (pos != NULL){
			pCurState->m_liSortHistory.AddTail(m_liSortHistory.GetNext(pos));
		}
		// SLUGFILLER: multiSort
#ifdef REPLACE_ATLMAP
		m_mapSortSelectionStates[m_nResultsID] = pCurState;
#else
		m_mapSortSelectionStates.SetAt(m_nResultsID, pCurState);
#endif
	}
	
	DeleteAllItems();
	
	// recover stored state
#ifdef REPLACE_ATLMAP
	unordered_map<int, CSortSelectionState*>::const_iterator it;
	if (nResultsID != 0 && nResultsID != m_nResultsID && (it = m_mapSortSelectionStates.find(nResultsID)) != m_mapSortSelectionStates.end()){
		CSortSelectionState* pNewState = it->second;
		m_mapSortSelectionStates.erase(it);
#else
	CSortSelectionState* pNewState = NULL;
	if (nResultsID != 0 && nResultsID != m_nResultsID && m_mapSortSelectionStates.Lookup(nResultsID, pNewState)){
		m_mapSortSelectionStates.RemoveKey(nResultsID);		
#endif
		// sort order
//		thePrefs.SetColumnSortItem(CPreferences::tableSearch, pNewState->m_nSortItem);
//		thePrefs.SetColumnSortAscending(CPreferences::tableSearch, pNewState->m_bSortAscending);

		//Xman
		// SLUGFILLER: multiSort - load sort history
		m_liSortHistory.RemoveAll();
		for (POSITION pos = pNewState->m_liSortHistory.GetHeadPosition(); pos != NULL; )
			m_liSortHistory.AddTail(pNewState->m_liSortHistory.GetNext(pos));
		// SLUGFILLER: multiSort

		SetSortArrow(pNewState->m_nSortItem, pNewState->m_bSortAscending);
		SortItems(SortProc, pNewState->m_nSortItem + (pNewState->m_bSortAscending ? 0:100));
		// fill in the items
		m_nResultsID = nResultsID;
		searchlist->ShowResults(m_nResultsID);
		// set stored selectionstates
		for (size_t i = 0; i < pNewState->m_aSelectedItems.GetCount(); i++){
			SetItemState(pNewState->m_aSelectedItems[i], LVIS_SELECTED, LVIS_SELECTED);
		}
		POINT Point;
		if (pNewState->m_nScrollPosition > 0){
			GetItemPosition(pNewState->m_nScrollPosition-1, &Point);
			Point.x = 0;
			Scroll(CSize(Point));
		}
		delete pNewState;
	}
	else{
		m_nResultsID = nResultsID;
		searchlist->ShowResults(m_nResultsID);
	}
}

void CSearchListCtrl::OnLvnColumnClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLISTVIEW *pNMListView = (NMLISTVIEW *)pNMHDR;
	bool sortAscending;
	if (GetSortItem() != pNMListView->iSubItem)
	{
		switch (pNMListView->iSubItem)
		{
			case 2: // Availability
			case 3: // Complete Sources
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

int CSearchListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	const CSearchFile *item1 = (CSearchFile *)lParam1;
	const CSearchFile *item2 = (CSearchFile *)lParam2;
	//Xman
	//int orgSort=lParamSort; // SLUGFILLER: multiSort remove - handled in parent class
	bool sortMod = true;
	if (lParamSort >= 100) {
		sortMod = false;
		lParamSort -= 100;
	}

	int comp;

	if (item1->GetListParent()==NULL && item2->GetListParent()!=NULL){
		if (item1 == item2->GetListParent())
			return -1;
		comp = Compare(item1, item2->m_list_parent, lParamSort, sortMod);
	}
	else if (item2->GetListParent()==NULL && item1->GetListParent()!=NULL){
		if (item1->m_list_parent == item2)
			return 1;
		comp = Compare(item1->GetListParent(), item2, lParamSort, sortMod);
	}
	else if (item1->GetListParent()==NULL){
		comp = Compare(item1, item2, lParamSort, sortMod);
	}
	else{
		comp = Compare(item1->GetListParent(), item2->GetListParent(), lParamSort, sortMod);
		if (comp != 0)
			return (sortMod)?comp:-comp;

		if ((item1->GetListParent()==NULL && item2->GetListParent()!=NULL) || (item2->GetListParent()==NULL && item1->GetListParent()!=NULL)){
			if (item1->GetListParent()==NULL)
				return -1;
			else
				return 1;
		}
		return CompareChild(item1, item2, lParamSort);
	}

	//Xman
	// SLUGFILLER: multiSort remove - handled in parent class
	/*
	int dwNextSort;
	//call secondary sortorder, if this one results in equal
	//(Note: yes I know this call is evil OO wise, but better than changing a lot more code, while we have only one instance anyway - might be fixed later)
	if (comp == 0 && (dwNextSort = theApp.emuledlg->searchwnd->m_pwndResults->searchlistctrl.GetNextSortOrder(orgSort)) != (-1)){
		comp= SortProc(lParam1, lParam2, dwNextSort);
	}
	*/

	return (sortMod)?comp:-comp;
}

int CSearchListCtrl::CompareChild(const CSearchFile *item1, const CSearchFile *item2, LPARAM lParamSort)
{
	LPARAM iColumn = lParamSort >= 100 ? lParamSort - 100 : lParamSort;
	int iResult = 0;
	switch (iColumn)
	{
		case 0:		//filename
			iResult = CompareLocaleStringNoCase(item1->GetFileName(), item2->GetFileName());
			break;

		case 14: // AICH Hash
			iResult = CompareAICHHash(item1->GetFileIdentifierC(), item2->GetFileIdentifierC(), true);
			break;
		default:
			// always sort by descending availability
			iResult = -CompareUnsigned(item1->GetIntTagValue(FT_SOURCES), item2->GetIntTagValue(FT_SOURCES));
			break;
	}
	if (lParamSort >= 100)
		return -iResult;
	return iResult;
}

int CSearchListCtrl::Compare(const CSearchFile *item1, const CSearchFile *item2, LPARAM lParamSort, bool bSortAscending)
{
	if (thePrefs.IsSearchSpamFilterEnabled())
	{
		// files makred as spam are always put to the bottom of the list (maybe as option later)
		if (item1->IsConsideredSpam() ^ item2->IsConsideredSpam())
			return (bSortAscending ^ item1->IsConsideredSpam()) ? -1 : 1;
	}

	switch (lParamSort)
	{
		case 0: //filename asc
			return CompareLocaleStringNoCase(item1->GetFileName(),item2->GetFileName());

		case 1: //size asc
			return CompareUnsigned64(item1->GetFileSize(), item2->GetFileSize());

		case 2: //sources asc
			return CompareUnsigned(item1->GetIntTagValue(FT_SOURCES), item2->GetIntTagValue(FT_SOURCES));

		case 3: // complete sources asc
			if (item1->GetIntTagValue(FT_SOURCES) == 0 || item2->GetIntTagValue(FT_SOURCES) == 0 || item1->IsKademlia() || item2->IsKademlia())
				return 0; // should never happen, just a sanity check
			return CompareUnsigned((item1->GetIntTagValue(FT_COMPLETE_SOURCES)*100)/item1->GetIntTagValue(FT_SOURCES), (item2->GetIntTagValue(FT_COMPLETE_SOURCES)*100)/item2->GetIntTagValue(FT_SOURCES));

		case 4: //type asc
		{
			int iResult = item1->GetFileTypeDisplayStr().Compare(item2->GetFileTypeDisplayStr());
			// if the type is equal, subsort by extension
			if (iResult == 0)
			{
				LPCTSTR pszExt1 = PathFindExtension(item1->GetFileName());
				LPCTSTR pszExt2 = PathFindExtension(item2->GetFileName());
				if ((pszExt1 == NULL) ^ (pszExt2 == NULL))
					return pszExt1 == NULL ? 1 : (-1);
				else
					return  pszExt1 != NULL ? _tcsicmp(pszExt1, pszExt2) : 0;
			}
			else
				return iResult;
		}
		case 5: //filehash asc
			return memcmp(item1->GetFileHash(),item2->GetFileHash(),16);
	
		case 6:
			return CompareOptLocaleStringNoCaseUndefinedAtBottom(item1->GetStrTagValue(FT_MEDIA_ARTIST), item2->GetStrTagValue(FT_MEDIA_ARTIST), bSortAscending);
	
		case 7:
			return CompareOptLocaleStringNoCaseUndefinedAtBottom(item1->GetStrTagValue(FT_MEDIA_ALBUM), item2->GetStrTagValue(FT_MEDIA_ALBUM), bSortAscending);

		case 8:
			return CompareOptLocaleStringNoCaseUndefinedAtBottom(item1->GetStrTagValue(FT_MEDIA_TITLE), item2->GetStrTagValue(FT_MEDIA_TITLE), bSortAscending);

		case 9:
			return CompareUnsignedUndefinedAtBottom(item1->GetIntTagValue(FT_MEDIA_LENGTH), item2->GetIntTagValue(FT_MEDIA_LENGTH), bSortAscending);

		case 10:
			return CompareUnsignedUndefinedAtBottom(item1->GetIntTagValue(FT_MEDIA_BITRATE), item2->GetIntTagValue(FT_MEDIA_BITRATE), bSortAscending);

		case 11:
			return CompareOptLocaleStringNoCaseUndefinedAtBottom(GetCodecDisplayName(item1->GetStrTagValue(FT_MEDIA_CODEC)), GetCodecDisplayName(item2->GetStrTagValue(FT_MEDIA_CODEC)), bSortAscending);

		case 12: //path asc
			return CompareOptLocaleStringNoCaseUndefinedAtBottom(item1->GetDirectory(), item2->GetDirectory(), bSortAscending);

		case 13:
			return item1->GetKnownType() - item2->GetKnownType();

		case 14:
			return CompareAICHHash(item1->GetFileIdentifierC(), item2->GetFileIdentifierC(), bSortAscending);
	}
	return 0;
}

void CSearchListCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if (point.x != -1 || point.y != -1) {// X: [BF] - [Bug Fix]
		RECT rcClient;
		GetClientRect(&rcClient);
		ClientToScreen(&rcClient);
		if (!PtInRect(&rcClient,point)) {
			Default();
			return;
		}
	}
	int iSelected = 0;
	int iToDownload = 0;
	bool bContainsNotSpamFile = false;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos != NULL)
	{
		const CSearchFile* pFile = (CSearchFile*)GetItemData(GetNextSelectedItem(pos));
		if (pFile)
		{
			iSelected++;
			if (!theApp.downloadqueue->IsFileExisting(pFile->GetFileHash(), false))
				iToDownload++;
			if (!pFile->IsConsideredSpam())
				bContainsNotSpamFile = true;
		}
	}

	if(iSelected > 0){
	m_SearchFileMenu.EnableMenuItem(MP_RESUME, iToDownload > 0 ? MF_ENABLED : MF_GRAYED);
	if (thePrefs.IsExtControlsEnabled())
		m_SearchFileMenu.EnableMenuItem(MP_RESUMEPAUSED, iToDownload > 0 ? MF_ENABLED : MF_GRAYED);
	//Xman add search to cancelled
		m_SearchFileMenu.EnableMenuItem(MP_ADDSEARCHCANCELLED, thePrefs.IsRememberingCancelledFiles() ? MF_ENABLED : MF_GRAYED);
	//Xman end
	if (thePrefs.IsExtControlsEnabled())
		m_SearchFileMenu.EnableMenuItem(MP_DETAIL, iSelected == 1 ? MF_ENABLED : MF_GRAYED);
	m_SearchFileMenu.EnableMenuItem(MP_REMOVE, theApp.emuledlg->searchwnd->CanDeleteSearch(m_nResultsID) ? MF_ENABLED : MF_GRAYED);
	m_SearchFileMenu.EnableMenuItem(MP_REMOVEALL, theApp.emuledlg->searchwnd->CanDeleteAllSearches() ? MF_ENABLED : MF_GRAYED);
		//m_SearchFileMenu.EnableMenuItem(MP_SEARCHRELATED, theApp.emuledlg->searchwnd->CanSearchRelatedFiles() ? MF_ENABLED : MF_GRAYED);

	UINT uInsertedMenuItem2 = 0;
		if (thePrefs.IsSearchSpamFilterEnabled() && m_SearchFileMenu.InsertMenu(MP_REMOVESELECTED, MF_STRING | MF_ENABLED, MP_MARKASSPAM, GetResString((bContainsNotSpamFile) ? IDS_MARKSPAM : IDS_MARKNOTSPAM)))
		uInsertedMenuItem2 = MP_MARKASSPAM;
	
	m_SearchFileMenu.SetDefaultItem(
		(iToDownload > 0)?
		(!thePrefs.AddNewFilesPaused() || !thePrefs.IsExtControlsEnabled()) ? MP_RESUME : MP_RESUMEPAUSED
		:(UINT)-1);

	GetPopupMenuPos(*this, point);
	m_SearchFileMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	if (uInsertedMenuItem2)
		VERIFY( m_SearchFileMenu.RemoveMenu(uInsertedMenuItem2, MF_BYCOMMAND) );

	}
}

BOOL CSearchListCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	wParam = LOWORD(wParam);

	CAtlList<CSearchFile*> selectedList;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos != NULL)
	{
		int index = GetNextSelectedItem(pos);
		if (index >= 0)
			selectedList.AddTail((CSearchFile*)GetItemData(index));
	}

	if (selectedList.GetCount() > 0)
	{
		CSearchFile* file = selectedList.GetHead();

		switch (wParam)
		{
			case MP_GETED2KLINK:{
				CWaitCursor curWait;
				CString clpbrd;
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					file = selectedList.GetNext(pos);
					if (!clpbrd.IsEmpty())
						clpbrd += _T("\r\n");
					clpbrd += file->GetED2kLink();
				}
				theApp.CopyTextToClipboard(clpbrd);
				return TRUE;
			}
			//Xman add search to cancelled
			case MP_ADDSEARCHCANCELLED:
				{
					CWaitCursor curWait;
					POSITION pos = GetFirstSelectedItemPosition();
					while (pos!=NULL) 
					{
						int cur_sel= GetNextSelectedItem(pos);
						const uchar* cur_hash=((CSearchFile*)GetItemData(cur_sel))->GetFileHash();
						theApp.knownfiles->AddCancelledFileID(cur_hash);
						Update(cur_sel);
					}
					return TRUE;
				}
			//Xman end

			case MP_RESUME:
				if (thePrefs.IsExtControlsEnabled())
					theApp.emuledlg->searchwnd->DownloadSelected(false);
				else
					theApp.emuledlg->searchwnd->DownloadSelected();
				return TRUE;
			case MP_RESUMEPAUSED:
				theApp.emuledlg->searchwnd->DownloadSelected(true);
				return TRUE;
			case IDA_ENTER:
				theApp.emuledlg->searchwnd->DownloadSelected();
				return TRUE;
			case MP_REMOVESELECTED:
			case MPG_DELETE:
			{
				CWaitCursor curWait;
				SetRedraw(FALSE);
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					file = selectedList.GetNext(pos);
					HideSources(file);
					theApp.searchlist->RemoveResult(file);
				}
				AutoSelectItem();
				SetRedraw(TRUE);
				return TRUE;
			}
			case MP_DETAIL:
			case MPG_ALTENTER:
			{
				CSearchResultFileDetailSheet sheet(selectedList, 0, this);
				sheet.DoModal();
				return TRUE;
			}
			/*case MP_SEARCHRELATED:{
				// just a shortcut for the user typing into the searchfield "related::[filehash]"
				CAtlList<CAbstractFile*> abstractFileList;
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
					abstractFileList.AddTail(selectedList.GetNext(pos));
				theApp.emuledlg->searchwnd->SearchRelatedFiles(abstractFileList);
				return TRUE;
			}*/
			case MP_MARKASSPAM:
			{
				CWaitCursor curWait;
				SetRedraw(FALSE);
				POSITION pos = selectedList.GetHeadPosition();
				bool bContainsNotSpamFile = false;
				while (pos != NULL){
					file = selectedList.GetNext(pos);
					if (!file->IsConsideredSpam()){
						bContainsNotSpamFile = true;
						break;
					}
				}
				pos = selectedList.GetHeadPosition();
				while (pos != NULL){
					file = selectedList.GetNext(pos);
					if (file->IsConsideredSpam() && bContainsNotSpamFile){
						continue;
					}
					else if (file->IsConsideredSpam() && !bContainsNotSpamFile){
						theApp.searchlist->MarkFileAsNotSpam(file, false, true);
					}
					else if (!file->IsConsideredSpam() && bContainsNotSpamFile){
						theApp.searchlist->MarkFileAsSpam(file, false, true);
					}
					else if (!file->IsConsideredSpam() && !bContainsNotSpamFile){
						continue;
					}
				}
				theApp.searchlist->RecalculateSpamRatings(file->GetSearchID(), bContainsNotSpamFile, !bContainsNotSpamFile, true); 
				SetRedraw(TRUE);
				return TRUE;
			}
		}
	}
	switch (wParam){
		case MP_REMOVEALL:{
			CWaitCursor curWait;
			theApp.emuledlg->searchwnd->DeleteAllSearches();
			break;
		}
		case MP_REMOVE:{
			CWaitCursor curWait;
			theApp.emuledlg->searchwnd->DeleteSearch(m_nResultsID);
			break;
		}
	}

	return FALSE;
}

void CSearchListCtrl::OnLvnDeleteAllItems(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	// To suppress subsequent LVN_DELETEITEM notification messages, return TRUE.
	*pResult = TRUE;
}

void CSearchListCtrl::CreateMenues()
{
	if (m_SearchFileMenu)
		VERIFY( m_SearchFileMenu.DestroyMenu() );

	m_SearchFileMenu.CreatePopupMenu();
	m_SearchFileMenu.AppendMenu(MF_STRING, MP_RESUME, GetResString(IDS_DOWNLOAD));
	if (thePrefs.IsExtControlsEnabled())
		m_SearchFileMenu.AppendMenu(MF_STRING, MP_RESUMEPAUSED, GetResString(IDS_DOWNLOAD) + _T(" (") + GetResString(IDS_PAUSED) + _T(')'));
	//Xman add search to cancelled
	m_SearchFileMenu.AppendMenu(MF_STRING, MP_ADDSEARCHCANCELLED, GetResString(IDS_MARKCANCELLED));
	//Xman end
	if (thePrefs.IsExtControlsEnabled())
		m_SearchFileMenu.AppendMenu(MF_STRING, MP_DETAIL, GetResString(IDS_SHOWDETAILS));
	m_SearchFileMenu.AppendMenu(MF_SEPARATOR);
	m_SearchFileMenu.AppendMenu(MF_STRING, MP_GETED2KLINK, GetResString(IDS_DL_LINK1));
	m_SearchFileMenu.AppendMenu(MF_STRING, MP_REMOVESELECTED, GetResString(IDS_REMOVESELECTED));
	//m_SearchFileMenu.AppendMenu(MF_STRING, MP_MARKASSPAM, GetResString(IDS_MARKSPAM));
	m_SearchFileMenu.AppendMenu(MF_SEPARATOR);
	m_SearchFileMenu.AppendMenu(MF_STRING, MP_REMOVE, GetResString(IDS_REMOVESEARCHSTRING));
	m_SearchFileMenu.AppendMenu(MF_STRING, MP_REMOVEALL, GetResString(IDS_REMOVEALLSEARCH));
	m_SearchFileMenu.AppendMenu(MF_SEPARATOR);
	//m_SearchFileMenu.AppendMenu(MF_STRING, MP_SEARCHRELATED, GetResString(IDS_SEARCHRELATED));
}

void CSearchListCtrl::OnLvnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMLVGETINFOTIP>(pNMHDR);
	if (pGetInfoTip->iSubItem == 0)
	{
		LVHITTESTINFO hti = {0};
		::GetCursorPos(&hti.pt);
		ScreenToClient(&hti.pt);
		bool bOverMainItem = (SubItemHitTest(&hti) != -1 && hti.iItem == pGetInfoTip->iItem && hti.iSubItem == 0);

		// those tooltips are very nice for debugging/testing but pretty annoying for general usage
		// enable tooltips only if Ctrl is currently pressed
		bool bShowInfoTip = bOverMainItem && (GetSelectedCount() > 1 || (GetKeyState(VK_CONTROL) & 0x8000));
		if (bShowInfoTip && GetSelectedCount() > 1)
		{
			// Don't show the tooltip if the mouse cursor is not over at least one of the selected items
			bool bInfoTipItemIsPartOfMultiSelection = false;
			POSITION pos = GetFirstSelectedItemPosition();
			while (pos) {
				if (GetNextSelectedItem(pos) == pGetInfoTip->iItem) {
					bInfoTipItemIsPartOfMultiSelection = true;
					break;
				}
			}
			if (!bInfoTipItemIsPartOfMultiSelection)
				bShowInfoTip = false;
		}

		if (!bShowInfoTip) {
			if (!bOverMainItem) {
				// don't show the default label tip for the main item, if the mouse is not over the main item
				if ((pGetInfoTip->dwFlags & LVGIT_UNFOLDED) == 0 && pGetInfoTip->cchTextMax > 0 && pGetInfoTip->pszText[0] != _T('\0'))
					pGetInfoTip->pszText[0] = _T('\0');
			}
			return;
		}

		if (GetSelectedCount() <= 1)
		{
		    const CSearchFile* file = (CSearchFile*)GetItemData(pGetInfoTip->iItem);
			if (file && pGetInfoTip->pszText && pGetInfoTip->cchTextMax > 0){
			    CString strInfo, strHead;
				strHead.Format(_T("%s\n")
					+ GetResString(IDS_FD_HASH) + _T(" %s\n")
					+ GetResString(IDS_FD_SIZE) + _T(" %s\n<br>\n"),
					file->GetFileName(), md4str(file->GetFileHash()), CastItoXBytes(file->GetFileSize(), false, false));

				const CAtlArray<CTag*>& tags = file->GetTags();
			    for (size_t i = 0; i < tags.GetCount(); i++){
				    const CTag* tag = tags[i];
				    if (tag){
					    CString strTag;
					    switch (tag->GetNameID()){
						    /*case FT_FILENAME:
							    strTag.Format(_T("%s: %s"), GetResString(IDS_SW_NAME), tag->GetStr());
							    break;
						    case FT_FILESIZE:
							    strTag.Format(_T("%s: %s"), GetResString(IDS_DL_SIZE), FormatFileSize(tag->GetInt64()));
							    break;*/
						    case FT_FILETYPE:
							    strTag.Format(_T("%s: %s"), GetResString(IDS_TYPE), tag->GetStr());
							    break;
						    case FT_FILEFORMAT:
							    strTag.Format(_T("%s: %s"), GetResString(IDS_SEARCHEXTENTION), tag->GetStr());
							    break;
						    case FT_SOURCES:
							    strTag.Format(_T("%s: %u"), GetResString(IDS_SEARCHAVAIL), tag->GetInt());
							    break;
						    case 0x13: // remote client's upload file priority (tested with Hybrid 0.47)
							    if (tag->GetInt() == 0)
								    strTag = GetResString(IDS_PRIORITY) + _T(": ") + GetResString(IDS_PRIONORMAL);
							    else if (tag->GetInt() == 2)
								    strTag = GetResString(IDS_PRIORITY) + _T(": ") + GetResString(IDS_PRIOHIGH);
							    else if (tag->GetInt() == -2)
								    strTag = GetResString(IDS_PRIORITY) + _T(": ") + GetResString(IDS_PRIOLOW);
						    #ifdef _DEBUG
							    else
								    strTag.Format(_T("%s: %d (***Unknown***)"), GetResString(IDS_PRIORITY), tag->GetInt());
						    #endif
							    break;
						    default:{
							    bool bUnkTag = false;
								bool bSkipTag = false;
							    if (tag->GetNameID() == FT_FILENAME || tag->GetNameID() == FT_FILESIZE)
									bSkipTag = true;
								else if (tag->GetName()){
									strTag.Format(_T("%hs: "), tag->GetName());
									strTag = strTag.Left(1).MakeUpper() + strTag.Mid(1);
							    }
							    else{
								    extern CString GetName(const CTag* pTag);
								    CString strTagName = GetName(tag);
								    if (!strTagName.IsEmpty()){
									    strTag.Format(_T("%s: "), strTagName);
								    }
								    else{
								    #ifdef _DEBUG
									    strTag.Format(_T("Unknown tag #%02X: "), tag->GetNameID());
								    #else
									    bUnkTag = true;
								    #endif
								    }
							    }
							    if (!bUnkTag && !bSkipTag){
								    if (tag->IsStr())
									    strTag += tag->GetStr();
								    else if (tag->IsInt()){
									    if (tag->GetNameID() == FT_MEDIA_LENGTH){
										    CString strTemp;
										    SecToTimeLength(tag->GetInt(), strTemp);
										    strTag += strTemp;
									    }
									    else{
										    TCHAR szBuff[16];
										    _itot(tag->GetInt(), szBuff, 10);
										    strTag += szBuff;
									    }
								    }
								    else if (tag->IsFloat()){
									    TCHAR szBuff[32];
									    _sntprintf(szBuff, _countof(szBuff) - 1, _T("%f"), tag->GetFloat());
										szBuff[_countof(szBuff) - 1] = _T('\0');
									    strTag += szBuff;
								    }
								    else if (!bSkipTag){
								    #ifdef _DEBUG
									    CString strBuff;
									    strBuff.Format(_T("Unknown value type=#%02X"), tag->GetType());
									    strTag += strBuff;
								    #else
									    strTag.Empty();
								    #endif
								    }
							    }
						    }
							break;
					    }
					    if (!strTag.IsEmpty()){
						    if (!strInfo.IsEmpty())
							    strInfo += _T('\n');
						    strInfo += strTag;
						    if (strInfo.GetLength() >= pGetInfoTip->cchTextMax)
							    break;
					    }
				    }
			    }
    
    #ifdef USE_DEBUG_DEVICE
			    if (file->GetClientsCount()){
					bool bFirst = true;
				    if (file->GetClientID() && file->GetClientPort()){
					    uint32 uClientIP = file->GetClientID();
					    uint32 uServerIP = file->GetClientServerIP();
						CString strSource;
						if (bFirst){
							bFirst = false;
							strSource = _T("Sources");
						}
						strSource.AppendFormat(_T(": %u.%u.%u.%u:%u  Server: %u.%u.%u.%u:%u"),
						    (uint8)uClientIP,(uint8)(uClientIP>>8),(uint8)(uClientIP>>16),(uint8)(uClientIP>>24), file->GetClientPort(),
						    (uint8)uServerIP,(uint8)(uServerIP>>8),(uint8)(uServerIP>>16),(uint8)(uServerIP>>24), file->GetClientServerPort());
					    if (!strInfo.IsEmpty())
						    strInfo += _T('\n');
					    strInfo += strSource;
				    }
    
				    // ==> Make code VS 2005 and VS 2008 ready [MorphXT] - Stulle
				    /*
				    const CSimpleArray<CSearchFile::SClient>& aClients = file->GetClients();
				    */
				    const CSimpleArray<CSearchFile::SClient,CSearchFile::CSClientEqualHelper>& aClients = file->GetClients();
				    // <== Make code VS 2005 and VS 2008 ready [MorphXT] - Stulle
				    for (int i = 0; i < aClients.GetSize(); i++){
					    uint32 uClientIP = aClients[i].m_nIP;
					    uint32 uServerIP = aClients[i].m_nServerIP;
						CString strSource;
						if (bFirst){
							bFirst = false;
							strSource = _T("Sources");
						}
						strSource.AppendFormat(_T(": %u.%u.%u.%u:%u  Server: %u.%u.%u.%u:%u"),
						    (uint8)uClientIP,(uint8)(uClientIP>>8),(uint8)(uClientIP>>16),(uint8)(uClientIP>>24), aClients[i].m_nPort,
						    (uint8)uServerIP,(uint8)(uServerIP>>8),(uint8)(uServerIP>>16),(uint8)(uServerIP>>24), aClients[i].m_nServerPort);
					    if (!strInfo.IsEmpty())
						    strInfo += _T('\n');
					    strInfo += strSource;
					    if (strInfo.GetLength() >= pGetInfoTip->cchTextMax)
						    break;
				    }
			    }
    
			    if (file->GetServers().GetSize()){
				    // ==> Make code VS 2005 and VS 2008 ready [MorphXT] - Stulle
				    /*
				    const CSimpleArray<CSearchFile::SServer>& aServers = file->GetServers();
				    */
				    const CSimpleArray<CSearchFile::SServer,CSearchFile::CSServerEqualHelper>& aServers = file->GetServers();
				    // <== Make code VS 2005 and VS 2008 ready [MorphXT] - Stulle
				    for (int i = 0; i < aServers.GetSize(); i++){
					    uint32 uServerIP = aServers[i].m_nIP;
						CString strServer;
						if (i == 0)
							strServer.Format(_T("Servers"));
						strServer.AppendFormat(_T(": %u.%u.%u.%u:%u  Avail: %u"),
						    (uint8)uServerIP,(uint8)(uServerIP>>8),(uint8)(uServerIP>>16),(uint8)(uServerIP>>24), aServers[i].m_nPort, aServers[i].m_uAvail);
					    if (!strInfo.IsEmpty())
						    strInfo += _T('\n');
					    strInfo += strServer;
					    if (strInfo.GetLength() >= pGetInfoTip->cchTextMax)
						    break;
				    }
			    }
    #endif
				strInfo = strHead + strInfo + TOOLTIP_AUTOFORMAT_SUFFIX_CH;
			    _tcsncpy(pGetInfoTip->pszText, strInfo, pGetInfoTip->cchTextMax);
			    pGetInfoTip->pszText[pGetInfoTip->cchTextMax-1] = _T('\0');
		    }
	    }
		else
		{
			int iSelected = 0;
			ULONGLONG ulTotalSize = 0;
			POSITION pos = GetFirstSelectedItemPosition();
			while (pos)
			{
				const CSearchFile* pFile = (CSearchFile*)GetItemData(GetNextSelectedItem(pos));
				if (pFile)
				{
					iSelected++;
					ulTotalSize += (uint64)pFile->GetFileSize();
				}
			}

			if (iSelected > 0)
			{
				CString strInfo;
				strInfo.Format(_T("%s: %u\r\n%s: %s"), GetResString(IDS_FILES), iSelected, GetResString(IDS_DL_SIZE), FormatFileSize(ulTotalSize));
				strInfo += TOOLTIP_AUTOFORMAT_SUFFIX_CH;
				_tcsncpy(pGetInfoTip->pszText, strInfo, pGetInfoTip->cchTextMax);
				pGetInfoTip->pszText[pGetInfoTip->cchTextMax-1] = _T('\0');
			}
		}
	}
	
	*pResult = 0;
}

void CSearchListCtrl::ExpandCollapseItem(int iItem, int iAction)
{
	if (iItem == -1)
		return;

	CSearchFile* searchfile = (CSearchFile*)GetItemData(iItem);
	if (searchfile->GetListParent() != NULL)
	{
		searchfile = searchfile->GetListParent();

		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)searchfile;
		iItem = FindItem(&find);
		if (iItem == -1)
			return;
	}
	if (!searchfile)
		return;

	if (!searchfile->IsListExpanded())
	{
		if (iAction > COLLAPSE_ONLY)
		{
			// only expand when more than one child (more than the original entry itself)
			if (searchfile->GetListChildCount() < 2)
				return;

			// Go through the whole list to find out the sources for this file
			SetRedraw(FALSE);
			const SearchList* list = theApp.searchlist->GetSearchListForID(searchfile->GetSearchID());
			for (POSITION pos = list->GetHeadPosition(); pos != NULL; )
			{
				const CSearchFile* cur_file = list->GetNext(pos);
				if (cur_file->GetListParent() == searchfile)
				{
					searchfile->SetListExpanded(true);
					InsertItem(LVIF_PARAM | LVIF_TEXT, iItem+1, cur_file->GetFileName(), 0, 0, 0, (LPARAM)cur_file);
				}
			}
			SetRedraw(TRUE);
		}
	}
	else{
		if (iAction == EXPAND_COLLAPSE || iAction == COLLAPSE_ONLY)
		{
			if (GetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED) != (LVIS_SELECTED | LVIS_FOCUSED))
			{
				SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				SetSelectionMark(iItem);
			}
			HideSources(searchfile);
		}
	}

	Update(iItem);
}

void CSearchListCtrl::HideSources(CSearchFile* toCollapse)
{
	SetRedraw(FALSE);
	size_t pre = 0;
	size_t post = 0;
	for (size_t i = 0; i < GetItemCount(); i++)
	{
		const CSearchFile* item = (CSearchFile*)GetItemData(i);
		if (item->GetListParent() == toCollapse)
		{
			pre++;
			DeleteItem(i--);
			post++;
		}
	}
	if (pre - post == 0)
		toCollapse->SetListExpanded(false);
	SetRedraw(TRUE);
}

void CSearchListCtrl::OnNmClick(NMHDR *pNMHDR, LRESULT * /*pResult*/)
{
	POINT pt;
	::GetCursorPos(&pt);
    ScreenToClient(&pt);
	if (pt.x < sm_iSubItemInset + TREE_WIDTH && pt.x > sm_iSubItemInset)
	{
		LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
		ExpandCollapseItem(pNMIA->iItem, EXPAND_COLLAPSE);
	}
}

void CSearchListCtrl::OnNmDblClk(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	POINT pt;
	::GetCursorPos(&pt);
    ScreenToClient(&pt);
	if (pt.x > sm_iSubItemInset + TREE_WIDTH)
	{
		if (GetKeyState(VK_MENU) & 0x8000)
		{
			int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
			if (iSel != -1)
			{
				/*const*/ CSearchFile* file = (CSearchFile*)GetItemData(iSel);
				if (file)
				{
					CAtlList<CSearchFile*> aFiles;
					aFiles.AddTail(file);
					CSearchResultFileDetailSheet sheet(aFiles, 0, this);
					sheet.DoModal();
				}
			}
		}
		else
			theApp.emuledlg->searchwnd->DownloadSelected();
	}
}

void CSearchListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	RECT cur_rec(lpDrawItemStruct->rcItem);
	CSearchFile *content = (CSearchFile *)lpDrawItemStruct->itemData;
	CMemoryDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	InitItemMemDC(dc, lpDrawItemStruct->rcItem, (lpDrawItemStruct->itemID % 2)?m_crEvenLine:m_crWindow, lpDrawItemStruct->itemState);
	if (!g_bLowColorDesktop || (lpDrawItemStruct->itemState & ODS_SELECTED) == 0)
		dc.SetTextColor(GetSearchItemColor(content));

	RECT rcClient;// X: [DDHC] - [Don't Draw Hidden Column]
	GetClientRect(&rcClient);

	BOOL notLast = lpDrawItemStruct->itemID + 1 != (UINT)GetItemCount();
	BOOL notFirst = lpDrawItemStruct->itemID != 0;
	int tree_start = 0;
	int tree_end = 0;

	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	cur_rec.right = cur_rec.left - sm_iSubItemInset;
	cur_rec.left += sm_iSubItemInset;

	// Parent entries
	if (content->GetListParent() == NULL)
	{
		for (int iCurrent = 0; iCurrent < iCount; iCurrent++)
		{
			int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
			if(IsColumnHidden(iColumn)) continue;
			UINT uDrawTextAlignment;
			int iColumnWidth = GetColumnWidth(iColumn, uDrawTextAlignment);
			cur_rec.right += iColumnWidth;
			if(iColumnWidth > 2*sm_iSubItemInset && cur_rec.right>0){// X: [DDHC] - [Don't Draw Hidden Column]
				if (iColumn == 0){
					//set up tree vars
					tree_start = cur_rec.left + 1;
					if(iColumnWidth <= 2*sm_iSubItemInset + 8)
						tree_end = cur_rec.left + iColumnWidth - 2*sm_iSubItemInset;
					else{
						tree_end = cur_rec.left + 8;
						if(iColumnWidth > 2*sm_iSubItemInset + 24){
							int iCurLeft = cur_rec.left;
							//normal column stuff
							cur_rec.left = tree_end + 3;
							DrawSourceParent(dc, 0, &cur_rec, uDrawTextAlignment, content);
							cur_rec.left = iCurLeft;
						}
					}
				}
				else
					DrawSourceParent(dc, iColumn, &cur_rec, uDrawTextAlignment, content);
			}
			cur_rec.left += iColumnWidth;
			if(cur_rec.left>=rcClient.right)// X: [DDHC] - [Don't Draw Hidden Column]
				break;
		}
	}
	else{
		for (int iCurrent = 0; iCurrent < iCount; iCurrent++){
			int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
			if(IsColumnHidden(iColumn)) continue;
			UINT uDrawTextAlignment;
			int iColumnWidth = GetColumnWidth(iColumn, uDrawTextAlignment);
			cur_rec.right += iColumnWidth;
			if(iColumnWidth > 2*sm_iSubItemInset && cur_rec.right>0){// X: [DDHC] - [Don't Draw Hidden Column]
				if (iColumn == 0){
					//set up tree vars
					tree_start = cur_rec.left + 1;
					if(iColumnWidth <= 2*sm_iSubItemInset + 8)
						tree_end = cur_rec.left + iColumnWidth - 2*sm_iSubItemInset;
					else{
						tree_end = cur_rec.left + 8;
						if(iColumnWidth > 2*sm_iSubItemInset + 32){
							int iCurLeft = cur_rec.left;
							//normal column stuff
							cur_rec.left = tree_end + 11;
							DrawSourceChild(dc, 0, &cur_rec, uDrawTextAlignment, content);
							cur_rec.left = iCurLeft;
						}
					}
				}
				else
					DrawSourceChild(dc, iColumn, &cur_rec, uDrawTextAlignment, content);
			}
			cur_rec.left += iColumnWidth;
			if(cur_rec.left>=rcClient.right)// X: [DDHC] - [Don't Draw Hidden Column]
				break;
		}
	}

	//draw tree last so it draws over selected and focus (looks better)
	if(tree_start < tree_end)
	{
		//set new bounds
		RECT tree_rect = {
			tree_start,
			lpDrawItemStruct->rcItem.top,
			tree_end,
			lpDrawItemStruct->rcItem.bottom
		};
		dc.SetBoundsRect(&tree_rect, DCB_DISABLE);

		//gather some information
		BOOL hasNext = notLast && ((const CSearchFile*)GetItemData(lpDrawItemStruct->itemID + 1))->GetListParent()!=NULL;
		BOOL isOpenRoot = hasNext && content->GetListParent() == NULL;
		BOOL isChild = content->GetListParent()!= NULL;

		//might as well calculate these now
		int treeCenter = tree_start + 4;
		int middle = (cur_rec.top + cur_rec.bottom + 1) / 2;

		//set up a new pen for drawing the tree
		COLORREF crLine = (!g_bLowColorDesktop || (lpDrawItemStruct->itemState & ODS_SELECTED) == 0) ? RGB(128,128,128) : m_crHighlightText;
		CPen pn, *oldpn;
		pn.CreatePen(PS_SOLID, 1, crLine);
		oldpn = dc.SelectObject(&pn);

		if(isChild)
		{
			//draw the line to the status bar
			dc.MoveTo(tree_end+10, middle);
			dc.LineTo(tree_start + 4, middle);

			//draw the line to the child node
			if(hasNext)
			{
				dc.MoveTo(treeCenter, middle);
				dc.LineTo(treeCenter, cur_rec.bottom + 1);
			}
		}
		else if (isOpenRoot || (content->GetListParent() == NULL && content->GetListChildCount() > 1))
		{
			//draw box
			RECT circle_rec;
			circle_rec.top    = middle - 5;
			circle_rec.bottom = middle + 4;
			circle_rec.left   = treeCenter - 4;
			circle_rec.right  = treeCenter + 5;
			dc.FrameRect(&circle_rec, &CBrush(crLine));
			CPen penBlack;
			penBlack.CreatePen(PS_SOLID, 1, (!g_bLowColorDesktop || (lpDrawItemStruct->itemState & ODS_SELECTED) == 0) ? m_crWindowText : m_crHighlightText);
			CPen* pOldPen2;
			pOldPen2 = dc.SelectObject(&penBlack);
			dc.MoveTo(treeCenter-2,middle - 1);
			dc.LineTo(treeCenter+3,middle - 1);
			
			if (!content->IsListExpanded())
			{
				dc.MoveTo(treeCenter,middle-3);
				dc.LineTo(treeCenter,middle+2);
			}
			dc.SelectObject(pOldPen2);
			penBlack.DeleteObject(); //Xman Code Improvement
			//draw the line to the child node
			if (hasNext)
			{
				dc.MoveTo(treeCenter, middle + 4);
				dc.LineTo(treeCenter, cur_rec.bottom + 1);
			}
		}

		//draw the line back up to parent node
		if (notFirst && isChild)
		{
			dc.MoveTo(treeCenter, middle);
			dc.LineTo(treeCenter, cur_rec.top - 1);
		}

		//put the old pen back
		dc.SelectObject(oldpn);
		pn.DeleteObject();
	}
}

COLORREF CSearchListCtrl::GetSearchItemColor(/*const*/ CSearchFile* src)
{
	const CKnownFile* pFile = theApp.downloadqueue->GetFileByID(src->GetFileHash());

	if (pFile)
	{
		if (pFile->IsPartFile())
		{
			src->SetKnownType(CSearchFile::Downloading);
			if (((CPartFile*)pFile)->GetStatus() == PS_PAUSED)
				return m_crSearchResultDownloadStopped;
			return m_crSearchResultDownloading;
		}
		else
		{
			src->SetKnownType(CSearchFile::Shared);
			return m_crSearchResultShareing;
		}
	}
	else if (theApp.sharedfiles->GetFileByID(src->GetFileHash()))
	{
		src->SetKnownType(CSearchFile::Shared);
		return m_crSearchResultShareing;
	}
	else if (theApp.knownfiles->FindKnownFileByID(src->GetFileHash()))
	{
		src->SetKnownType(CSearchFile::Downloaded);
		return m_crSearchResultKnown;
	}
	else if (theApp.knownfiles->IsCancelledFileByID(src->GetFileHash())){
		src->SetKnownType(CSearchFile::Cancelled);
		return m_crSearchResultCancelled;		
	}

	// Spamcheck
	if (src->IsConsideredSpam() && thePrefs.IsSearchSpamFilterEnabled())
		return ::GetSysColor(COLOR_GRAYTEXT);

	// unknown file -> show shades of a color
	uint32 srccnt = src->GetSourceCount();
	if (srccnt > 0)
		--srccnt;
	return m_crShades[(srccnt >= AVBLYSHADECOUNT) ? AVBLYSHADECOUNT-1 : srccnt];
}

void CSearchListCtrl::DrawSourceChild(CDC *dc, int nColumn, LPRECT lpRect, UINT uDrawTextAlignment, /*const*/ CSearchFile* src)
{
	TCHAR szItem[1024];
	GetItemDisplayText(src, nColumn, szItem, _countof(szItem));
	if (nColumn == 0){// file name
		//UINT uOffset = 30;
		int iImage = theApp.GetFileTypeSystemImageIdx(szItem/*src->GetFileName()*/);
		if (theApp.GetSystemImageList() != NULL){
			int iIconPosY = (lpRect->bottom - lpRect->top > 19) ? ((lpRect->bottom - lpRect->top - 16) / 2) : 1;
		ImageList_Draw(theApp.GetSystemImageList(), iImage, dc->GetSafeHdc(), lpRect->left, lpRect->top + iIconPosY, ILD_NORMAL | ILD_TRANSPARENT);
		}
		lpRect->left += 16 + sm_iLabelOffset;
		dc->DrawText(szItem, -1, lpRect, MLC_DT_TEXT);
		lpRect->left -= 16 + sm_iLabelOffset;
	}
	else if(szItem[0] != 0)
		dc->DrawText(szItem, -1, lpRect, MLC_DT_TEXT | uDrawTextAlignment);
}

void CSearchListCtrl::DrawSourceParent(CDC *dc, int nColumn, LPRECT lpRect, UINT uDrawTextAlignment, /*const*/ CSearchFile* src)
{
	TCHAR szItem[1024];
	GetItemDisplayText(src, nColumn, szItem, _countof(szItem));
	switch (nColumn)
	{
		case 0:			// file name
		{
			// icon
			/*int ofs = (content->GetListParent()!=NULL)?
					14 // indent child items
				:
					6;*/
			int iIconPosY = (lpRect->bottom - lpRect->top > 19) ? ((lpRect->bottom - lpRect->top - 16) / 2) : 1;
			int iImage = theApp.GetFileTypeSystemImageIdx(szItem/*src->GetFileName()*/);
			if (theApp.GetSystemImageList() != NULL){
			ImageList_Draw(theApp.GetSystemImageList(), iImage, dc->GetSafeHdc(), lpRect->left, lpRect->top + iIconPosY, ILD_NORMAL | ILD_TRANSPARENT);
			}
			// spam indicator takes the place of commentsrating icon
			UINT uOffset = 16;
			if (thePrefs.IsSearchSpamFilterEnabled() && src->IsConsideredSpam()){
				uOffset += sm_i2IconOffset;
				POINT point = {lpRect->left + uOffset, lpRect->top + iIconPosY};
				m_ImageList.Draw(dc, 0, point, ILD_NORMAL);
				uOffset += 16;
			}
			uOffset += sm_iLabelOffset;
			lpRect->left += uOffset;
			dc->DrawText(szItem, -1, lpRect, MLC_DT_TEXT);
			lpRect->left -= uOffset;
			break;
		}

		case 3:{		// complete sources
			bool bComplete = IsComplete(src, src->GetSourceCount());
			COLORREF crOldTextColor = 0;
			if (!bComplete)
				crOldTextColor = dc->SetTextColor(RGB(255, 0, 0));
			dc->DrawText(szItem, -1, lpRect, MLC_DT_TEXT | uDrawTextAlignment);
			if (!bComplete)
				dc->SetTextColor(crOldTextColor);
			break;
		}

		default:
			if(szItem[0] != 0)
				dc->DrawText(szItem, -1, lpRect, MLC_DT_TEXT | uDrawTextAlignment);
			break;
	}
}

void CSearchListCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == 'C' && (GetKeyState(VK_CONTROL) & 0x8000))
	{
		// Ctrl+C: Copy listview items to clipboard
		SendMessage(WM_COMMAND, MP_GETED2KLINK);
		return;
	}
	CMuleListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CSearchListCtrl::SetHighlightColors()
{
	// Default colors
	// --------------
	//	Blue:	User does not know that file; shades of blue are used to indicate availability of file
	//  Red:	User already has the file; it is currently downloading or it is currently shared 
	//			-> 'Red' means: User can not add this file
	//	Green:	User 'knows' the file (it was already download once, but is currently not in share)
	COLORREF crSearchResultAvblyBase = RGB(0,0,255);
	m_crSearchResultDownloading		= RGB(255,0,0);
	m_crSearchResultDownloadStopped = RGB(255,0,0);
	m_crSearchResultShareing		= RGB(255,0,0);
	m_crSearchResultKnown			= RGB(0,128,0);
	m_crSearchResultCancelled		= RGB(190,130,0); //Xman changed to brown

	// precalculate sources shades
	COLORREF normFGC=GetTextColor();
	float rdelta = (float)((GetRValue(crSearchResultAvblyBase) - GetRValue(normFGC)) / AVBLYSHADECOUNT);
	float gdelta = (float)((GetGValue(crSearchResultAvblyBase) - GetGValue(normFGC)) / AVBLYSHADECOUNT);
	float bdelta = (float)((GetBValue(crSearchResultAvblyBase) - GetBValue(normFGC)) / AVBLYSHADECOUNT);

	for (int shades=0;shades<AVBLYSHADECOUNT;shades++) {
		m_crShades[shades]=RGB(	GetRValue(normFGC) + (rdelta*shades),
								GetGValue(normFGC) + (gdelta*shades),
								GetBValue(normFGC) + (bdelta*shades));
	}
}

void CSearchListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetHighlightColors();	
}

void CSearchListCtrl::OnLvnKeyDown(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDown = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);

	bool bAltKey = GetAsyncKeyState(VK_MENU) < 0;
	int iAction = EXPAND_COLLAPSE;
	if (pLVKeyDown->wVKey==VK_ADD || (bAltKey && pLVKeyDown->wVKey==VK_RIGHT))
		iAction = EXPAND_ONLY;
	else if (pLVKeyDown->wVKey==VK_SUBTRACT || (bAltKey && pLVKeyDown->wVKey==VK_LEFT))
		iAction = COLLAPSE_ONLY;
	if (iAction < EXPAND_COLLAPSE)
		ExpandCollapseItem(GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED), iAction);
	*pResult = 0;
}

void CSearchListCtrl::ClearResultViewState(uint32 nResultsID){
	// just clean up our stored states for this search
#ifdef REPLACE_ATLMAP
	unordered_map<int, CSortSelectionState*>::const_iterator it = m_mapSortSelectionStates.find(nResultsID);
	if(it != m_mapSortSelectionStates.end()){
		CSortSelectionState* pState = it->second;
		m_mapSortSelectionStates.erase(it);
		delete pState;
	}
#else
	CSortSelectionState* pState = NULL;
	if (m_mapSortSelectionStates.Lookup(nResultsID, pState)){
		m_mapSortSelectionStates.RemoveKey(nResultsID);
		delete pState;
	}
#endif
}

void CSearchListCtrl::GetItemDisplayText(const CAbstractFile* src, int iSubItem, 
										 LPTSTR pszText, int cchTextMax) const
{
	if (pszText == NULL || cchTextMax <= 0) {
		ASSERT(0);
		return;
	}
	pszText[0] = _T('\0');
	const CSearchFile* pSearchFile=reinterpret_cast<const CSearchFile*>(src);
	switch (iSubItem)
	{
		case 0:			// file name
			_tcsncpy(pszText, pSearchFile->GetFileName(), cchTextMax);
			break;
		
		case 1:			// file size
			if (   pSearchFile->GetListParent() == NULL
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
				|| (thePrefs.GetDebugSearchResultDetailLevel() >= 1 && src->GetFileSize() != pSearchFile->GetListParent()->GetFileSize())
#endif
				) {
				_tcsncpy(pszText, FormatFileSize(src->GetFileSize()), cchTextMax);
			}
			break;

		case 2:			// avail
			if (pSearchFile->GetListParent() == NULL)
			{
				CString strBuffer;
				strBuffer.Format(_T("%u"), pSearchFile->GetSourceCount());
				if (thePrefs.IsExtControlsEnabled()) {
					if (pSearchFile->IsKademlia()) {
						uint32 nKnownPublisher = (pSearchFile->GetKadPublishInfo() & 0x00FF0000) >> 16;
						if (nKnownPublisher > 0)
							strBuffer.AppendFormat(_T(" (%u)"), nKnownPublisher);
					}
					else {
						int iClients = pSearchFile->GetClientsCount();
						if (iClients > 0)
							strBuffer.AppendFormat(_T(" (%u)"), iClients);
					}
				}
#ifdef _DEBUG
				if (pSearchFile->GetKadPublishInfo() == 0)
					strBuffer += _T(" | -");
				else
					strBuffer.AppendFormat(_T(" | Names:%u, Pubs:%u, Trust:%0.2f"), 
										   (pSearchFile->GetKadPublishInfo() & 0xFF000000) >> 24,
										   (pSearchFile->GetKadPublishInfo() & 0x00FF0000) >> 16, 
										   (float)(pSearchFile->GetKadPublishInfo() & 0x0000FFFF) / 100.0f);
#endif
				_tcsncpy(pszText, strBuffer, cchTextMax);
			}
			else {
				_ultot_s(pSearchFile->GetListChildCount(), pszText, cchTextMax, 10);// X: [CI] - [Code Improvement]
			}
			break;
		
		case 3:			// complete sources
			if (   pSearchFile->GetListParent() == NULL
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
				|| (thePrefs.GetDebugSearchResultDetailLevel() >= 1 && thePrefs.IsExtControlsEnabled())
#endif
				) {
				_tcsncpy(pszText, GetCompleteSourcesDisplayString(pSearchFile, pSearchFile->GetSourceCount()), cchTextMax);
			}
			break;
		
		case 4:			// file type
			if (pSearchFile->GetListParent() == NULL)
				_tcsncpy(pszText, pSearchFile->GetFileTypeDisplayStr(), cchTextMax);
			break;
		
		case 5:			// file hash
			if (pSearchFile->GetListParent() == NULL)
				md4str(pSearchFile->GetFileHash(), pszText);
			break;
		
		case 6:
			_tcsncpy(pszText, pSearchFile->GetStrTagValue(FT_MEDIA_ARTIST), cchTextMax);
			break;
		
		case 7:
			_tcsncpy(pszText, pSearchFile->GetStrTagValue(FT_MEDIA_ALBUM), cchTextMax);
			break;
		
		case 8:
			_tcsncpy(pszText, pSearchFile->GetStrTagValue(FT_MEDIA_TITLE), cchTextMax);
			break;
		
		case 9:{
			uint_ptr nMediaLength = pSearchFile->GetIntTagValue(FT_MEDIA_LENGTH);
			if (nMediaLength){
				CString buffer;
				SecToTimeLength(nMediaLength, buffer);
				_tcsncpy(pszText, buffer, cchTextMax);
			}
			break;
		}
		
		case 10:{
			uint32 nBitrate = pSearchFile->GetIntTagValue(FT_MEDIA_BITRATE);
			if (nBitrate)
				_sntprintf(pszText, cchTextMax, _T("%u %s"), nBitrate, GetResString(IDS_KBITSSEC));
			break;
		}
		
		case 11:
			_tcsncpy(pszText, GetCodecDisplayName(pSearchFile->GetStrTagValue(FT_MEDIA_CODEC)), cchTextMax);
			break;
		
		case 12:		// dir
			if (pSearchFile->GetDirectory())
				_tcsncpy(pszText, pSearchFile->GetDirectory(), cchTextMax);
			break;
		
		case 13:
			if (pSearchFile->GetListParent() == NULL)
			{
				if (pSearchFile->m_eKnown == CSearchFile::Shared)
					_tcsncpy(pszText, GetResString(IDS_SHARED), cchTextMax);
				else if (pSearchFile->m_eKnown == CSearchFile::Downloading)
					_tcsncpy(pszText, GetResString(IDS_DOWNLOADING), cchTextMax);
				else if (pSearchFile->m_eKnown == CSearchFile::Downloaded)
					_tcsncpy(pszText, GetResString(IDS_DOWNLOADED), cchTextMax);
				else if (pSearchFile->m_eKnown == CSearchFile::Cancelled)
					_tcsncpy(pszText, GetResString(IDS_CANCELLED), cchTextMax);
				else if (pSearchFile->IsConsideredSpam() && thePrefs.IsSearchSpamFilterEnabled())
					_tcsncpy(pszText, GetResString(IDS_SPAM), cchTextMax);
#ifdef _DEBUG
				{
					pszText[cchTextMax - 1] = _T('\0');
					CString strBuffer(pszText);
					if (!strBuffer.IsEmpty())
						strBuffer += _T(' ');
					DEBUG_ONLY(strBuffer.AppendFormat(_T("SR: %u%%"), pSearchFile->GetSpamRating()));
					_tcsncpy(pszText, strBuffer, cchTextMax);
				}
#endif
			break;
			}
		case 14:
			if (src->GetFileIdentifierC().HasAICHHash())
				_tcsncpy(pszText, src->GetFileIdentifierC().GetAICHHash().GetString(), cchTextMax);
			break;
	}
	pszText[cchTextMax - 1] = _T('\0');
}

void CSearchListCtrl::OnLvnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (CemuleDlg::IsRunning()) {
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
		if (pDispInfo->item.mask & LVIF_TEXT) {
			const CSearchFile *pSearchFile = reinterpret_cast<CSearchFile *>(pDispInfo->item.lParam);
			if (pSearchFile != NULL)
				GetItemDisplayText(pSearchFile, pDispInfo->item.iSubItem, pDispInfo->item.pszText, pDispInfo->item.cchTextMax);
		}
	}
	*pResult = 0;
}

CString	CSearchListCtrl::FormatFileSize(ULONGLONG ullFileSize) const
{
	if (thePrefs.m_eFileSizeFormat == fsizeKByte) {
		// Always rond up to next KB (this is same as Windows Explorer is doing)
		return GetFormatedUInt64((ullFileSize + 1024 - 1) / 1024) + _T(" KB");
	}
	if (thePrefs.m_eFileSizeFormat == fsizeMByte) {
		//return GetFormatedUInt64((ullFileSize + 1024*1024 - 1) / (1024*1024)) + _T(" ") + GetResString(IDS_MBYTES);
		double fFileSize = (double)ullFileSize / (1024.0*1024.0);
		if (fFileSize < 0.01)
			fFileSize = 0.01;
		TCHAR szVal[40];
		_sntprintf(szVal, _countof(szVal) - 1, _T("%.2f"), fFileSize);
		szVal[_countof(szVal) - 1] = _T('\0');
		static NUMBERFMT nf;
		if (nf.Grouping == 0) {
			nf.NumDigits = 2;
			nf.LeadingZero = 1;
			nf.Grouping = 3;
			// we are hardcoding the following two format chars by intention because the C-RTL also has the decimal sep hardcoded to '.'
			nf.lpDecimalSep = _T(".");
			nf.lpThousandSep = _T(",");
			nf.NegativeOrder = 0;
		}
		CString strVal;
		const int iBuffSize = _countof(szVal)*2;
		int iResult = GetNumberFormat(LOCALE_SYSTEM_DEFAULT, 0, szVal, &nf, strVal.GetBuffer(iBuffSize), iBuffSize);
		strVal.ReleaseBuffer();
		if (iResult == 0)
			strVal = szVal;
		return strVal + _T(" MB");
	}
		return CastItoXBytes(ullFileSize);
}

void CSearchListCtrl::SetFileSizeFormat(EFileSizeFormat eFormat)
{
	thePrefs.m_eFileSizeFormat = eFormat;
	Invalidate();
	UpdateWindow();
}

void CSearchListCtrl::ReloadFileList(){
	theApp.emuledlg->searchwnd->m_pwndResults->Reload();
}
