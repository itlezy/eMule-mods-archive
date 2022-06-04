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
#include "MuleListCtrl.h"
#include "ListCtrlItemWalk.h"
#include "FilterItem.h"// X: [FI] - [FilterItem]
#include "map_inc.h"

#define AVBLYSHADECOUNT 13

class CSearchList;
class CSearchFile;
class CToolTipCtrlX;

struct SearchCtrlItem_Struct{
   CSearchFile*		value;
   CSearchFile*     owner;
   uchar			filehash[16];
   uint16			childcount;
};

class CSortSelectionState{
public:
	uint32	m_nSortItem;
	bool	m_bSortAscending;
	uint32	m_nScrollPosition;
	CAtlArray<int>	m_aSelectedItems;
	CAtlList<int> m_liSortHistory; // SLUGFILLER: multiSort
};

class CSearchListCtrl : public CMuleListCtrl, public CListCtrlItemWalk, public CFilterItem
{
	DECLARE_DYNAMIC(CSearchListCtrl)

public:
	CSearchListCtrl();
	virtual ~CSearchListCtrl();

	void	Init(CSearchList* in_searchlist);
	void	CreateMenues();
	void	UpdateSources(const CSearchFile* toupdate);
	void	AddResult(const CSearchFile* toshow);
	void	RemoveResult(const CSearchFile* toremove);
	void	Localize();
	void	ShowResults(uint32 nResultsID);
	void	ClearResultViewState(uint32 nResultsID);
	void	NoTabs() { m_nResultsID = 0; }
	void	UpdateSearch(CSearchFile* toupdate);
	void	SetFileSizeFormat(EFileSizeFormat eFormat);
	virtual void	ReloadFileList();
	void	SetGridLine();

protected:
	uint32		m_nResultsID;
	CMenu	m_SearchFileMenu;
	CSearchList* searchlist;
	CToolTipCtrlX* m_tooltip;
	CImageList	m_ImageList;
	COLORREF	m_crSearchResultDownloading;
	COLORREF	m_crSearchResultDownloadStopped;
	COLORREF	m_crSearchResultKnown;
	COLORREF	m_crSearchResultShareing;
	COLORREF	m_crSearchResultCancelled;
	COLORREF	m_crShades[AVBLYSHADECOUNT];

#ifdef REPLACE_ATLMAP
	unordered_map<int, CSortSelectionState*> 	m_mapSortSelectionStates;
#else
	CAtlMap<int, CSortSelectionState*> 	m_mapSortSelectionStates;
#endif

	COLORREF GetSearchItemColor(/*const*/ CSearchFile* src);
	bool IsComplete(const CSearchFile *pFile, UINT uSources) const;
	CString GetCompleteSourcesDisplayString(const CSearchFile* pFile, UINT uSources/*, bool* pbComplete = NULL*/) const;
	void	ExpandCollapseItem(int iItem, int iAction);
	void	HideSources(CSearchFile* toCollapse);
	void	SetHighlightColors();
	void	SetAllIcons();
	CString	FormatFileSize(ULONGLONG ullFileSize) const;
	virtual void	GetItemDisplayText(const CAbstractFile* src, int iSubItem, LPTSTR pszText, int cchTextMax) const;

	void	DrawSourceParent(CDC *dc, int nColumn, LPRECT lpRect, UINT uDrawTextAlignment, /*const*/ CSearchFile* src);
	void	DrawSourceChild(CDC *dc, int nColumn, LPRECT lpRect, UINT uDrawTextAlignment, /*const*/ CSearchFile* src);

	static int Compare(const CSearchFile* item1, const CSearchFile* item2, LPARAM lParamSort, bool bSortMod);
	static int CompareChild(const CSearchFile* file1, const CSearchFile* file2, LPARAM lParamSort);
	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLvnColumnClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnDeleteAllItems(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnKeyDown(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNmClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNmDblClk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSysColorChange();
};
