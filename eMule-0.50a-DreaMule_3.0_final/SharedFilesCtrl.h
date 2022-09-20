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
#pragma once
#include "MuleListCtrl.h"
#include "TitleMenu.h"
#include "ListCtrlItemWalk.h"
//>>> WiZaRd::Artist Filter
#include "QArray.h"
#include "SharedDirsTreeCtrl.h"
//<<< WiZaRd::Artist Filter

class CSharedFileList;
class CKnownFile;
class CDirectoryItem;
class CToolTipCtrlX;

class CSharedDirsTreeCtrl; //>>> WiZaRd::Artist Filter

//////////////////////////////////////////////////////////////////////////////
// CSharedFileDetailsSheet

//>>> WiZaRd::SharedFiles Redesign
#include "ListViewWalkerPropertySheet.h"
#include "FileInfoDialog.h"
#include "MetaDataDlg.h"
#include "ED2kLinkDlg.h"
#include "CommentDialog.h"
#include "ArchivePreviewDlg.h"
class CSharedFileDetailsSheet : public CListViewWalkerPropertySheet
{
	DECLARE_DYNAMIC(CSharedFileDetailsSheet)

public:
	CSharedFileDetailsSheet(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage = 0, CListCtrlItemWalk* pListCtrl = NULL);
	virtual ~CSharedFileDetailsSheet();

protected:
	CFileInfoDialog		m_wndMediaInfo;
	CMetaDataDlg		m_wndMetaData;
	CED2kLinkDlg		m_wndFileLink;
	CCommentDialog		m_wndFileComments;
	CArchivePreviewDlg	m_wndArchiveInfo;

	UINT m_uPshInvokePage;
	static LPCTSTR m_pPshStartPage;

	void UpdateTitle();

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg LRESULT OnDataChanged(WPARAM, LPARAM);
};
//<<< WiZaRd::SharedFiles Redesign

class CSharedFilesCtrl : public CMuleListCtrl, public CListCtrlItemWalk
{
	friend class CSharedDirsTreeCtrl;
	DECLARE_DYNAMIC(CSharedFilesCtrl)

public:
	CSharedFilesCtrl();
	virtual ~CSharedFilesCtrl();

	void	Init();
	void	CreateMenues();
//>>> WiZaRd::Artist Filter
//	void	ReloadFileList();
	void	ReloadFileList(const bool bReloadArtists = true); 
//<<< WiZaRd::Artist Filter
	void	AddFile(const CKnownFile* file);
	void	RemoveFile(const CKnownFile* file);
	void	UpdateFile(const CKnownFile* file, bool force=false); //Xman advanced upload-priority
	void	Localize();
	void	ShowFilesCount();
	void	ShowComments(CKnownFile* file);
	void	SetAICHHashing(uint32 nVal)				{ nAICHHashing = nVal; }
	void	SetDirectoryFilter(CDirectoryItem* pNewFilter, bool bRefresh = true);

protected:
	CTitleMenu		m_SharedFilesMenu;
	CTitleMenu		m_CollectionsMenu;
	CMenu			m_PrioMenu;
	bool			sortstat[4];
	CImageList		m_ImageList;
	CDirectoryItem*	m_pDirectoryFilter;
	volatile uint32 nAICHHashing;
	CToolTipCtrlX*	m_pToolTip;

	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	void OpenFile(const CKnownFile* file);
	void ShowFileDialog(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage = 0);
	void SetAllIcons();
	int FindFile(const CKnownFile* pFile);

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSysColorChange();
	afx_msg	void OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLvnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult);

//>>> WiZaRd::Artist Filter
private:
	CQArray<CString, const CString&> m_arrArtists;
	CSharedDirsTreeCtrl*	pParentTree;
public:
	void	SetParentTree(CSharedDirsTreeCtrl* parent)	{pParentTree = parent;}
	void	FillTreeItemList(CDirectoryItem* pCurrent, const int i, const ESpecialDirectoryItems eParentType);
	void	UpdateArtistList(const CKnownFile* file = NULL);
	bool	CheckFileCat(const CKnownFile* file);
//<<< WiZaRd::Artist Filter
//>>> WiZaRd::ShareFilter
private:
	void	GetItemDisplayText(const CKnownFile* file, const int iSubItem, LPTSTR pszText, const int cchTextMax) const;
//<<< WiZaRd::ShareFilter
};
