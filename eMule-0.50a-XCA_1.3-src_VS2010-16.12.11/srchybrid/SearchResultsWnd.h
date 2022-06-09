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
#include "ResizableLib\ResizableFormView.h"
#include "SearchListCtrl.h"
#include "ClosableTabCtrl.h"
#include "IconStatic.h"
#include "EditX.h"
#include "ComboBoxEx2.h"
#include "ListCtrlEditable.h"
#include "FilterDlg.h"// X: [FI] - [FilterItem]

class CCustomAutoComplete;
class Packet;
class CSafeMemFile;
class CSearchParamsWnd;
struct SSearchParams;
class CDropDownButton;
class CButtonsTabCtrl;
class CAbstractFile;

///////////////////////////////////////////////////////////////////////////////
// CSearchResultsSelector

class CSearchResultsSelector : public CClosableTabCtrl
{
public:
	CSearchResultsSelector(){}

protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
};

///////////////////////////////////////////////////////////////////////////////
// CSearchResultsWnd dialog

class CSearchResultsWnd : public CResizableFormView, public CFilterDlg
{
	DECLARE_DYNCREATE(CSearchResultsWnd)

public:
	CSearchResultsWnd(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSearchResultsWnd();
	
	enum { IDD = IDD_SEARCH };

	CSearchListCtrl searchlistctrl;
	CSearchResultsSelector searchselect;
	CSearchParamsWnd* m_pwndParams;

	void	Localize();
	void	LocalizeAll();// X: [RUL] - [Remove Useless Localize]

	void	StartSearch(SSearchParams* pParams);
	bool	SearchMore();
	void	CancelSearch(UINT uSearchID = 0);

	bool	DoNewEd2kSearch(SSearchParams* pParams);
	void	CancelEd2kSearch();
	bool	IsLocalEd2kSearchRunning() const { return (m_uTimerLocalServer != 0); }
	bool	IsGlobalEd2kSearchRunning() const { return (global_search_timer != 0); }
	void	LocalEd2kSearchEnd(UINT count, bool bMoreResultsAvailable);
	void	AddGlobalEd2kSearchResults(UINT count);
	void	SetNextSearchID(uint32 uNextID)				{m_nEd2kSearchID = uNextID;}

	bool	DoNewKadSearch(SSearchParams* pParams);
	//void	CancelKadSearch(UINT uSearchID);
	void	SearchCanceled(UINT uSearchID);// X: move up

	bool	CanSearchRelatedFiles() const;
	void	SearchRelatedFiles(CAtlList<CAbstractFile*>& listFiles);

	void	DownloadSelected();
	void	DownloadSelected(bool bPaused);

	bool	CanDeleteSearch(uint32 nSearchID) const;
	bool	CanDeleteAllSearches() const;
	void	DeleteSearch(uint32 nSearchID);
	void	DeleteAllSearches();
	void	DeleteSelectedSearch();

	bool	CreateNewTab(SSearchParams* pParams, bool bActiveIcon = true);
	void	ShowSearchSelector(bool visible);
	int		GetSelectedCat();
	void	SetSelectedCat(int iSel);// X: [UIC] - [UIChange]
	void	UpdateCatTabs();
	void	Reload();
	virtual void	UpdateFilterLabel();
	SSearchParams* GetSearchResultsParams(UINT uSearchID) const;

protected:
	Packet*		searchpacket;
	bool		m_b64BitSearchPacket;
	UINT_PTR	global_search_timer;
	UINT_PTR		m_uTimerLocalServer;
	CProgressCtrl searchprogress;
	CButton		m_ctlOpenParamsWnd;
	bool		canceld;
	uint16		servercount;
	bool		globsearch;
	uint32		m_nEd2kSearchID;
	CImageList	m_imlSearchResults;
	CButtonsTabCtrl	*m_cattabs;
	CDropDownButton* m_btnSearchListMenu;
	int			m_iSentMoreReq;

	bool StartNewSearch(SSearchParams* pParams);
	void SearchStarted();
	void ShowResults(const SSearchParams* pParams);
	void SetAllIcons();
	void SetSearchResultsIcon(UINT uSearchID, int iImage);
	void SetActiveSearchResultsIcon(UINT uSearchID);
	void SetInactiveSearchResultsIcon(UINT uSearchID);

	virtual void OnInitialUpdate();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDblClkSearchList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSelChangeTab(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnCloseTab(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDblClickTab(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy();
	afx_msg void OnSysColorChange();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//afx_msg void OnBnClickedDownloadSelected();
	afx_msg void OnBnClickedClearAll();
	afx_msg void OnClose();
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedOpenParamsWnd();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg LRESULT OnChangeFilter(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSearchListMenuBtnDropDown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
