//this file is part of eMule
//Copyright (C)2002-2005 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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
#include "ResizableLib\ResizableDialog.h"
#include "SplitterControl.h"
#include "TabCtrl.hpp"
#include "UploadListCtrl.h"
#include "DownloadListCtrl.h"
#include "HistoryListCtrl.h"
#include "DownloadClientsCtrl.h"
#include "SharedFilesCtrl.h"
#include "PartStatusCtrl.h"
#include "FilterDlg.h"// X: [FI] - [FilterItem]

class CDropDownButton;
class CToolTipCtrlX;

class CTransferWnd : public CResizableDialog, public CFilterDlg
{
	DECLARE_DYNAMIC(CTransferWnd)

public:
	CTransferWnd(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTransferWnd();

	enum EWnd1Icon {
		w1iSplitWindow = 0,
		w1iShared,
		w1iHistory,
		w1iFilter
	};

	enum EWnd2Icon {
		w2iDownloading = 0,
		w2iUploading
	};

	enum EWnd2 {
		wnd2Downloading = 0,
		wnd2Uploading = 1,
		wnd2Shared = 2,
		wnd2History = 3
	};

	void ShowQueueCount();
	void ShowBufferUsage(uint64 totalsize);// X: [GB] - [Global Buffer]
	void UpdateListCount(EWnd2 listindex, size_t iCount = -1);
	void UpdateFilesCount(); //Xman see all sources

	void Localize();
	void LocalizeAll();// X: [RUL] - [Remove Useless Localize]
	void UpdateCatTabTitles(bool force = true);
	virtual void UpdateFilterLabel();
	size_t	 AddCategory(CString newtitle,CString newincoming,CString newtemp,CString newcomment,CString newautocat,bool addTab=true);// X: [TD] - [TempDir]
	size_t	 AddCategoryInteractive();
	void ResetTransToolbar(bool bShowToolbar);
	void SetToolTipsDelay(DWORD dwDelay);
	void OnDisableList();
	void ShowCatTab();

	// Dialog Data
	enum { IDD = IDD_TRANSFER };
	CUploadListCtrl			uploadlistctrl;
	CDownloadListCtrl		downloadlistctrl;
	CDownloadClientsCtrl	downloadclientsctrl;
	CSharedFilesCtrl		sharedfilesctrl;
	CHistoryListCtrl		historylistctrl;
	CPartStatusCtrl partstatusctrl;
	volatile UINT_PTR nAICHHashing;

protected:
	CSplitterControl m_wndSplitter;
	EWnd2		m_uWnd2;
	bool		downloadlistactive;
	CDropDownButton* m_btnWnd1;
	CDropDownButton* m_btnWnd2;
	TabControl	m_dlTab;
	size_t			rightclickindex;
	int			m_nDragIndex;
	int			m_nDropIndex;
	int			m_nLastCatTT;
	bool		m_bIsDragging;
	CImageList* m_pDragImage;
	POINT		m_pLastMousePoint;
	uint_ptr		m_dwShowListIDC;
	CToolTipCtrlX* m_tooltipCats;
	void	ShowWnd2(EWnd2 uList);
	void	DoResize(int delta);
	void	UpdateSplitterRange();
	//void	SetAllIcons();
	void	SetWnd1Icons();
	void	SetWnd2Icons();
	void	UpdateTabToolTips() {UpdateTabToolTips(-1);}
	void	UpdateTabToolTips(int tab);
	CString	GetTabStatistic(int tab);
	int		GetTabUnderMouse(CPoint* point);
	int		GetItemUnderMouse(CListCtrl* ctrl);
	void	EditCatTabLabel(size_t index);
	void	ShowList(uint_ptr dwListIDC);
	void	SetWnd1Icon(EWnd1Icon iIcon);
	void	SetWnd2Icon(EWnd2Icon iIcon);
	void	LocalizeToolbars();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnChangeFilter(WPARAM wParam, LPARAM lParam);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedChangeView();
	afx_msg void OnBnClickedRefreshButton();
	afx_msg void OnDblClickDltab();
	afx_msg void OnHoverDownloadList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHoverUploadList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLvnBeginDragDownloadList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnNmRClickDltab(NMHDR *pNMHDR, LRESULT *pResult);
	//afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnSplitterMoved(NMHDR *pNMHDR, LRESULT *pResult);
	//afx_msg void OnSysColorChange();
	afx_msg void OnTabMovement(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTcnSelchangeDltab(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnWnd1BtnDropDown(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnWnd2BtnDropDown(NMHDR *pNMHDR, LRESULT *pResult);
};
