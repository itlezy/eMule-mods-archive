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
#pragma once

#include "Loggable.h"
#include "MuleCtrlItem.h"
#include "DownloadList.h"
#include "MuleListCtrl.h"
#pragma warning(push)
#pragma warning(disable:4702) // unreachable code
#include <map>
#pragma warning(pop)

#define DL_OVERRIDESORT			99

class CPartFile;
class CUpDownClient;

enum EnumDownloadListIcons
{
	DL_ICON_DCS1,
	DL_ICON_DCS2,
	DL_ICON_DCS3,
	DL_ICON_DCS4,
	DL_ICON_DCS5,
	DL_ICON_LISTMINUS,
	DL_ICON_LISTNONE,
	DL_ICON_LISTPLUS,
	DL_ICON_RATING_NO,
	DL_ICON_RATING_EXCELLENT,
	DL_ICON_RATING_GOOD,
	DL_ICON_RATING_FAIR,
	DL_ICON_RATING_POOR,
	DL_ICON_RATING_FAKE,
	DL_ICON_A4AFAUTO,

	DL_ICON_STATUS_COMPLETE,
	DL_ICON_STATUS_COMPLETING,
	DL_ICON_STATUS_DOWNLOADING,
	DL_ICON_STATUS_ERRONEOUS,
	DL_ICON_STATUS_HASHING,
	DL_ICON_STATUS_PAUSED,
	DL_ICON_STATUS_STALLED,
	DL_ICON_STATUS_STOPPED,
	DL_ICON_STATUS_WAITING,
	DL_ICON_STATUS_WAITINGHASH
};

// CDownloadListCtrl
class CDownloadListCtrl : public CMuleListCtrl, public CLoggable
{
	DECLARE_DYNAMIC(CDownloadListCtrl)

public:
	CDownloadListCtrl();
	virtual ~CDownloadListCtrl();

	void	Init();

	void	AddFileItem(CPartFileDLItem* pFileItem);
	void	AddSourceItem(CSourceDLItem *pSourceItem);
	void	RemoveFileItem(CPartFileDLItem *pFileItem);
	void	RemoveSourceItem(CSourceDLItem *pSourceItem);

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	void	UpdateSourceItem(CSourceDLItem *pSourceItem);
	void	UpdateFileItem(CPartFileDLItem *pPartFileItem);

	void	SetStyle();
	void	Localize();

	void	HideFileItem(CPartFileDLItem *pFileItem);
	void	ShowFileItem(CPartFileDLItem *pFileItem);
	void	HideSources(CPartFile* pFileItem/*,bool isShift = false,bool isCtrl = false,bool isAlt = false*/);
	void	HideSourceItem(CSourceDLItem *pSourceItem);
	void	ShowSourceItem(CSourceDLItem *pSourceItem);
	bool	IsSourceFiltered(CSourceDLItem *pSourceItem);
	void	AutoSetSourceFilters(CPartFileDLItem *pFileItem);
	void	ResetSourceFiltersForAllFiles();

	void	ShowFilesCount();
	void	ShowSelectedFileOrUserDetails();
	void	ShowAllUploadingSources();


	void	ExpandCollapseItem(int iItem, enum EnumExpandType expand,bool bCollapseSource = false);
	bool	GetDisplayedFiles(CArray<CPartFile*,CPartFile*> *list);

	void	ChangeCategoryByID(EnumCategories eNewCatID);
	void	ChangeCategoryByIndex(int iNewCatIdx);
	EnumCategories	GetCurTabCat() { return m_eCurTabCat; }
	byte	GetCurTabIndex() { return static_cast<byte>(m_iCurTabIndex); }

	virtual BOOL OnCommand(WPARAM wParam,LPARAM lParam);

	void	RestartWaitingDownloads();
	void	RefreshList();
	void	SortInit(int override);
	bool	IsShowingSources() {return m_bShowSrc;}

	void	PostRefreshMessage();
	void	FilterNoSources();
	void	FilterAllSources();
	void	UpdateSourceItems(CPartFileDLItem *pFileItem = NULL);
	void	UpdateFileItems();
	virtual BOOL PreTranslateMessage(MSG *pMsg);

protected:
	void	DrawFileItem(CDC *dc, int nColumn, LPRECT lpRect, CPartFileDLItem *pFileItem);
	void	DrawSourceItem(CDC *dc, int nColumn, LPRECT lpRect, CSourceDLItem *lpSourceItem);

	afx_msg void OnItemActivate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnListModified(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkDownloadlist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);

	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static int Compare(CPartFile* file1, CPartFile* file2, LPARAM lParamSortItem, WPARAM wParamSortMod);
	static int Compare(CUpDownClient* client1,CUpDownClient* client2, LPARAM lParamSort, bool bDisambiguate = true);

	DECLARE_MESSAGE_MAP()

	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKillFocus(CWnd *pNewWnd);

	virtual void OnNMDividerDoubleClick(NMHEADER *pNMHDR);
	virtual BOOL OnWndMsg(UINT iMessage, WPARAM wParam, LPARAM lParam, LRESULT *pResult);

	void RefreshInfo(void);

private:
	EnumCategories	m_eCurTabCat;
	int				m_iCurTabIndex;
	int				m_iColumnMaxWidths[DLCOL_NUMCOLUMNS];
	int				m_iMeasuringColumn;
	struct	//anonymous struct - allows the use of bit fields as if they were direct members
	{
		bool		m_bShowUploadingSources : 1;
		bool		m_bShowOnQueueSources : 1;
		bool		m_bShowFullQueueSources : 1;
		bool		m_bShowConnectedSources : 1;
		bool		m_bShowConnectingSources : 1;
		bool		m_bShowNNPSources : 1;
		bool		m_bShowWaitForFileReqSources : 1;
		bool		m_bShowLowToLowIDSources : 1;
		bool		m_bShowLowIDOnOtherSrvSources : 1;
		bool		m_bShowBannedSources : 1;
		bool		m_bShowErrorSources : 1;
		bool		m_bShowA4AFSources : 1;
		bool		m_bShowUnknownSources : 1;
		bool		m_bSmartFilter : 1;
	};

	CImageList		m_imageList;

	byte			m_byteSortAscending[DLCOL_NUMCOLUMNS];	// bit 0 - 1st sort direction, bit 1 - 2nd sort direction, etc.
	int				m_iCurrentSortItem;
	bool			m_bSortAscending;
	int				m_iSourceSortItem;
	bool			m_bSortSourcesAscending;
	int				m_iSourceSortItem2;			// Secondary sort column
	bool			m_bSortSourcesAscending2;
	bool			m_bShowSrc;

private:
//	Map methods
	CPartFileDLItem	   *GetFileItem(const CDownloadList::PFIter &itFile) { return itFile->second; }
	CSourceDLItem	   *GetSourceItem(const CDownloadList::SourceIter &itSource) { return itSource->second; }

//	List methods
	void			ListInsertFileItem(CPartFileDLItem *pFileItem,int iPos);
	void			ListInsertSourceItem(CSourceDLItem *pSourceItem, int iPos);
	int				ListGetFileItemIndex(CPartFileDLItem *pFileItem);
	int				ListGetSourceItemIndex(CSourceDLItem *pSourceItem);
	CMuleCtrlItem  *ListGetItemAt(int iIndex);
	bool			ListSelectionIsEmpty() { return GetSelectionMark() == -1 || GetSelectedCount() < 1; }
};
