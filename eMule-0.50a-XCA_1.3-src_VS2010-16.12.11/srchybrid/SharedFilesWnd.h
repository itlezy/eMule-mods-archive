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
#include "SharedFilesCtrl.h"
#include "SharedDirsTreeCtrl.h"
#include "SplitterControl.h"
#include "HistoryListCtrl.h" //Xman [MoNKi: -Downloaded History-]
#include "FilterDlg.h"// X: [FI] - [FilterItem]
#include "ListViewWalkerPropertySheet.h"

/////////////////////////////////////////////////////////////////////////////////////////////
// CSharedFileDetailsModelessSheet
class CFileDetailDlgStatistics;
class CED2kLinkDlg;
class CArchivePreviewDlg;
class CFileInfoDialog;
class CMetaDataDlg;
class CSharedFileDetailsModelessSheet : public CListViewPropertySheet
{
	DECLARE_DYNAMIC(CSharedFileDetailsModelessSheet)

public:
	CSharedFileDetailsModelessSheet();
	virtual ~CSharedFileDetailsModelessSheet();
	void SetFiles(CAtlList<CShareableFile*>& aFiles);
	void LocalizeAll();
	void UpdateMetaDataPage(); //>>> WiZaRd::FiX

protected:
	CFileDetailDlgStatistics*	m_wndStatistics;
	CED2kLinkDlg*				m_wndFileLink;
	CArchivePreviewDlg*			m_wndArchiveInfo;
	CFileInfoDialog*			m_wndMediaInfo;
	CMetaDataDlg*				m_wndMetaData;

	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnDataChanged(WPARAM, LPARAM);
};

class CDropDownButton;
class CSharedFilesWnd : public CResizableDialog, public CFilterDlg
{
	DECLARE_DYNAMIC(CSharedFilesWnd)

public:
	CSharedFilesWnd(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSharedFilesWnd();

	void Localize();
	void LocalizeAll();// X: [RUL] - [Remove Useless Localize]
	void LocalizeToolbars(); // X: AKF - [AllKnownFiles] <-- Xanatos --
	void UpdateMetaDataPage(); //>>> WiZaRd::FiX
	void SetToolTipsDelay(DWORD dwDelay);
	void Reload(bool bForceTreeReload = false);
	void OnVolumesChanged()						{ m_ctlSharedDirTree.OnVolumesChanged(); }
	void OnSingleFileShareStatusChanged()		{ m_ctlSharedDirTree.FileSystemTreeUpdateBoldState(NULL); }
	void ShowSelectedFilesDetails(bool bForce = false); //Xman [MoNKi: -Downloaded History-]
	void ShowDetailsPanel(bool bShow);

	void ResetShareToolbar(bool bShowToolbar); // X: AKF - [AllKnownFiles] <-- Xanatos --
	void ShowFilesCount(bool history=false);
	virtual void UpdateFilterLabel();
	// Dialog Data
	enum { IDD = IDD_FILES };

	CSharedFilesCtrl sharedfilesctrl;
	CSharedDirsTreeCtrl m_ctlSharedDirTree;
	CHistoryListCtrl historylistctrl;
	CDropDownButton* m_btnWndS; 
	volatile size_t nAICHHashing;

private:

	HICON icon_files;
	CSplitterControl m_wndSplitter;
	bool			m_ShowAllKnow; // NEO: AKF - [AllKnownFiles] <-- Xanatos --
	bool			m_bDetailsVisible;
	CSharedFileDetailsModelessSheet	m_dlgDetails;

protected:
	void SetAllIcons();
	void DoResize(int delta);
	void SetWndIcons(); // X: AKF - [AllKnownFiles] <-- Xanatos --
	void SetWndIcon(); // X: AKF - [AllKnownFiles] <-- Xanatos --
	void ChangeView(bool changFilter); // X: AKF - [AllKnownFiles] <-- Xanatos --
	void OnWndSBtnDropDown(NMHDR* , LRESULT*); // X: AKF - [AllKnownFiles] <-- Xanatos --

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM );

	DECLARE_MESSAGE_MAP()
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnChangeFilter(WPARAM wParam, LPARAM lParam);
	//afx_msg void OnBnClickedReloadSharedFiles();// NEO: AKF - [AllKnownFiles] -- Xanatos -->
	afx_msg void OnLvnItemActivateFiles(NMHDR *pNMHDR, LRESULT *pResult);
/*	afx_msg void OnLvnItemActivateHistorylist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickHistorylist(NMHDR *pNMHDR, LRESULT *pResult);*/
//	afx_msg void OnStnDblClickFilesIco();
	afx_msg void OnSysColorChange();
	afx_msg void OnTvnSelChangedSharedDirsTree(NMHDR *pNMHDR, LRESULT *pResult);
//	afx_msg void OnShowWindow( BOOL bShow,UINT nStatus  ); //Xman [MoNKi: -Downloaded History-]
	//Xman [MoNKi: -Downloaded History-]
	//afx_msg void OnBnClickedSfHideshowdetails();// NEO: AKF - [AllKnownFiles] -- Xanatos -->
	afx_msg void OnLvnItemchangedSflist(NMHDR *pNMHDR, LRESULT *pResult);
};
