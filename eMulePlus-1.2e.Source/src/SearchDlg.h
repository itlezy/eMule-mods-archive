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

#include "SearchListCtrl.h"
#include "types.h"
#include "IconStatic.h"
#include "ClosableTabCtrl.h"

class Packet;
class CCustomAutoComplete;

typedef struct
{
	CString sNotSearch;
	bool bDocumentSearch;
} SearchData;
typedef CMap<int, int, SearchData, SearchData> SearchMap;

class CSearchDlg : public CResizableDialog
{
	DECLARE_DYNAMIC(CSearchDlg)

public:
	CSearchDlg(CWnd* pParent = NULL);
	virtual ~CSearchDlg();

	afx_msg void OnBnClickedStarts();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBnClickedCancels();
	afx_msg void OnBnClickedSdownload();
	afx_msg void OnBnClickedClearall();
	afx_msg void OnChangeMax();
	afx_msg void OnChangeMin();
	afx_msg void OnSearchKeyDown();
	afx_msg void OnBnClickedMores();

	enum { IDD = IDD_SEARCH };

	void	Localize();
	void	DownloadSelected(bool bPaused = false);
	void	LocalSearchEnd(uint16 count, bool LocalSearchEnd);
	void	AddGlobalEd2kSearchResults(uint16 count);
	void	DeleteSearch(uint32 nSearchID);
	void	DeleteAllSearches();
	void	CreateNewTab(const CString &strSearchString, uint32 nSearchID);
	void	UpdateCatTabs();
	void	OnChangeSize(bool changedMin);
	void	SearchClipBoard();
	void	IgnoreClipBoardLinks(const CString &strLink)		{m_strLastClipBoard = strLink; }
	void	AddEd2kLinksToDownload(CString strLink,EnumCategories eCatID);
	void	AddEd2kLinksToDownload(CString strLink)		{AddEd2kLinksToDownload(strLink,CCat::GetCatIDByIndex(m_ctrlCatTabs.GetCurSel()));}
	CSearchListCtrl m_ctlSearchList;
	CClosableTabCtrl m_ctlSearchTabs;
	void	DoNewEd2kSearch(const CString &strSearch, const CString &strTypeText, uint64 qwMinSize, uint64 qwMaxSize,
							 uint32 dwAvailability, const CString &strExtension, bool bDoGlobal, const CString &strSearchExclusion, uint16 nSearchID = 0);
	CString	GetNotSearch(int nSearchId);
	bool	IsDocumentSearch(int nSearchId);
	void	SaveSearchStrings();
	void	ShowSearchTabs(int iCmdShow);
	bool	IsLastSearchCanceled() const {return m_bCancelled;}
	bool	IsMoreEnabled() const {return m_bMoreEnabled;}
	void	KeepHistoryChanged();
	CPPToolTip m_ttip;

protected:
	void StartNewSearch();
	LRESULT OnCloseTab(WPARAM wparam, LPARAM lparam);
	LRESULT OnShowWindow(WPARAM wparam, LPARAM lparam);
	CString	CreateWebQuery();
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnNMDblclkSearchlist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeSearchname();
	afx_msg void OnEnChangeMethod();
	afx_msg void OnBnClickedSearchReset();
	afx_msg void OnBnClickedDownloadAllEd2k();
	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()

private:
	Packet		*m_pGlobSearchPck;
	Packet		*m_pGlobSearchPck2;
	UINT_PTR	global_search_timer;
	CProgressCtrl m_ctlGlobalSearchProgress;
	bool		m_bCancelled;
	bool		m_bGlobalSearch;
	bool		m_bMoreEnabled;
	bool		m_bGuardCBPrompt;
	uint32		m_nSearchID;
	CComboBoxEx	methodbox;
	CComboBox	Stypebox;
	CTabCtrl	m_ctrlCatTabs;
	CString		m_strLastClipBoard;
	CImageList	m_BoxImageList;
	CIconStatic	m_ctrlSearchFrm;
	CIconStatic	m_ctrlDirectDlFrm;
	CCustomAutoComplete* m_pacSearchString;

	HICON		oldSearchLstIcon;
	SearchMap	m_SearchMap;

	uint32		m_nMoreRequestCount;
	uint16		m_uServerCount;
	bool		m_b64BitSearchPacket;

private:
	CStatic*	GetSearchLstStatic() const		{ return (CStatic*)GetDlgItem(IDC_SEARCHLST_ICO); }
	HICON		GetSearchLstIcon() const		{ return (HICON)::LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_SEARCHRESULTS), IMAGE_ICON, 16, 16, 0); }
	void 		GetInfo4ToolTip(NM_PPTOOLTIP_DISPLAY	*pNotify);
};
