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
#include "QueueListCtrl.h"
#include "ClientListCtrl.h"
#include "DownloadClientsCtrl.h"
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
#include "Neo\GUI\ToolTips\PPToolTip.h"

#define UPLOAD_WND		0
#define	QUEUE_WND		1
#define KNOWN_WND		2
#define TRANSF_WND		3
#define UPDOWN_WND		4
#define DOWNLOAD_WND	5
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

class CDropDownButton;
class CToolTipCtrlX;

class CTransferWnd : public CResizableDialog
{
	DECLARE_DYNAMIC(CTransferWnd)

public:
	friend class CSharedDirsTreeCtrl;// NEO: NSC - [NeoSharedCategories] <-- Xanatos --

	CTransferWnd(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTransferWnd();

	enum EWnd1Icon {
		w1iSplitWindow = 0,
		w1iDownloadFiles,
		w1iUploading,
		w1iDownloading,
		w1iOnQueue,
		w1iClientsKnown
	};

	enum EWnd2Icon {
		w2iUploading = 0,
		w2iDownloading,
		w2iOnQueue,
		w2iClientsKnown
	};

	enum EWnd2 {
		wnd2Downloading = 0,
		wnd2Uploading = 1,
		wnd2OnQueue = 2,
		wnd2Clients = 3
	};

	void ShowQueueCount(uint32 number);
	void UpdateListCount(EWnd2 listindex, int iCount = -1);
	//void UpdateFilesCount(int iCount);
	void UpdateFilesCount(int iCount, float Percentage); // NEO: MOD - [Percentage] <-- Xanatos --
	void Localize();
	void UpdateCatTabTitles(bool force = true);
	void UpdateCatTabIcons(bool force = false); // NEO: CCF - [ColloredCategoryFlags] <-- Xanatos --
	void VerifyCatTabSize();
	//int	 AddCategory(CString newtitle,CString newincoming,CString newcomment,CString newautocat,bool addTab=true);
	int	 AddCategory(CString newtitle,CString newincoming,CString newcomment,CString newautocat,bool addTab=true, bool share = false); // NEO: NSC - [NeoSharedCategories] <-- Xanatos --
	void SwitchUploadList();
	void ResetTransToolbar(bool bShowToolbar, bool bResetLists = true);
	int GetActiveCategory()			{ return m_dlTab.GetCurSel(); } // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
	void NeoCommand(uint8 uNeoCmdL, uint8 uNeoCmdW); // NEO: NTB - [NeoToolbarButtons] <-- Xanatos --
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	CImageList* GetImageList() { return &m_ImageList; }
	void DrawClientImage(CDC *dc, POINT &point, CUpDownClient* client);
	void SetTTDelay();
	void SetDlgItemFocus(int nID);
#else
	void SetToolTipsDelay(DWORD dwDelay);
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
	void OnDisableList();

	// Dialog Data
	enum { IDD = IDD_TRANSFER };
	CUploadListCtrl			uploadlistctrl;
	CDownloadListCtrl		downloadlistctrl;
	CQueueListCtrl			queuelistctrl;
	CClientListCtrl			clientlistctrl;
	CDownloadClientsCtrl	downloadclientsctrl;

protected:
	CSplitterControl m_wndSplitter;
	EWnd2		m_uWnd2;
	bool		downloadlistactive;
	CDropDownButton* m_btnWnd1;
	CDropDownButton* m_btnWnd2;
	CDropDownButton* m_btnWndN; // NEO: NTB - [NeoToolbarButtons] <-- Xanatos --
	TabControl	m_dlTab;
	int			rightclickindex;
	int			m_nDragIndex;
	int			m_nDropIndex;
	int			m_nLastCatTT;
	int			m_isetcatmenu;
	bool		m_bIsDragging;
	CImageList* m_pDragImage;
	POINT		m_pLastMousePoint;
	uint32		m_dwShowListIDC;
#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
	CToolTipCtrlX* m_tooltipCats;
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
	// NEO: CCF - [ColloredCategoryFlags] -- Xanatos -->
	CImageList	m_imagelistCat;
	HICON		m_hIconCatTab;
	HICON		m_hIconCatTabHi;
	HICON		m_hIconCatTabLo;
	HICON		GetColorIcon(COLORREF color, UINT prio);
	// NEO: CCF END <-- Xanatos --

	// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
	//CTitleMenu	m_mnuCategory; 
	CMenuXP		m_mnuCategory; // NEO: NMX - [NeoMenuXP]
	CTitleMenu	m_mnuCatPriority;
	CTitleMenu	m_mnuCatViewFilter;
	CTitleMenu	m_mnuCatA4AF;
	// NEO: NXC END <-- Xanatos --

	void	ShowWnd2(EWnd2 uList);
	void	SetWnd2(EWnd2 uWnd2);
	void	DoResize(int delta);
	void	UpdateSplitterRange();
	void	DoSplitResize(int delta);
	void	SetAllIcons();
	void	SetWnd1Icons();
	void	SetWnd2Icons();
	void	SetWndNIcons(); // NEO: NTB - [NeoToolbarButtons] <-- Xanatos --
#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
	void	UpdateTabToolTips() {UpdateTabToolTips(-1);}
	void	UpdateTabToolTips(int tab);
	CString	GetTabStatistic(int tab);
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
	int		GetTabUnderMouse(CPoint* point);
#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
	int		GetItemUnderMouse(CListCtrl* ctrl);
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
	CString	GetCatTitle(int catid);
	void	EditCatTabLabel(int index,CString newlabel);
	void	EditCatTabLabel(int index);
	void	ShowList(uint32 dwListIDC);
	void	SetWnd1Icon(EWnd1Icon iIcon);
	void	SetWnd2Icon(EWnd2Icon iIcon);
	void	ShowSplitWindow(bool bReDraw = false);
	void	LocalizeToolbars();
	void	CreateCategoryMenus(); // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
	void	NeoCommand(uint8 button); // NEO: NTB - [NeoToolbarButtons] <-- Xanatos --

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	
	DECLARE_MESSAGE_MAP()
	afx_msg void OnHoverUploadList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHoverDownloadList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTcnSelchangeDltab(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRclickDltab(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTabMovement(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLvnKeydownDownloadlist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSysColorChange();
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnDblclickDltab();
	afx_msg void OnBnClickedQueueRefreshButton();
	afx_msg void OnBnClickedChangeView();
	afx_msg void OnWnd1BtnDropDown(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnWnd2BtnDropDown(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnWndNBtnDropDown(NMHDR *pNMHDR, LRESULT *pResult);// NEO: NTB - [NeoToolbarButtons] <-- Xanatos --
	afx_msg void OnSplitterMoved(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct); // NEO: NMX - [NeoMenuXP] <-- Xanatos --
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	afx_msg BOOL OnToolTipNotify(UINT id, NMHDR *pNMH, LRESULT *pResult);

private:
	CMuleListCtrl* lists_list[6];	
	CImageList m_ImageList;
	CPPToolTip m_ttip;
	CPPToolTip m_tabtip;
	//CPPToolTip m_othertips;
	CPPToolTip m_btttp;

	void	UpdateTabToolTips();
	void	UpdateToolTips();
	int		m_iOldToolTipItem[DOWNLOAD_WND];
	int		GetClientImage(CUpDownClient* client);
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
};
