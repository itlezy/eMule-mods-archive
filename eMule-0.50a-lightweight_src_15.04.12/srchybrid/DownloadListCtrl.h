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

// Foward declaration
class CPartFile;
class CUpDownClient;
class CDownloadListCtrl;
class CToolTipCtrlX;

///////////////////////////////////////////////////////////////////////////////
// CDownloadListCtrl

class CDownloadListCtrl : public CMuleListCtrl, public CListCtrlItemWalk, public CFilterItem
{
	DECLARE_DYNAMIC(CDownloadListCtrl)

public:
	CDownloadListCtrl();
	virtual ~CDownloadListCtrl();

	UINT	curTab;

	void	Init();
	void	AddFile(CPartFile* toadd);
	void	RemoveFile(const CPartFile* toremove);
	void	CreateMenues();
	void	Localize();
	void	ChangeCategory(size_t newsel);
	void	ShowSelectedFileDetails();
	void	ReloadFileList();
	void	UpdateCurrentCategoryView(CPartFile* thisfile);
	CImageList *CreateDragImage(int iItem, LPPOINT lpPoint);
	void	RefreshFile(CPartFile* file);
	void	CreateFilterMenuCat(CMenu&CatMenu);
	CString	GetFilterLabelCat();
    void	FillCatsMenu(CMenu& rCatsMenu, size_t iFilesInCats = (-1));
	//CMenu* GetPrioMenu();
	void	SetGridLine();

protected:
	CImageList  m_ImageList;

    void SetAllIcons();

	CMenu	m_PrioMenu;
	CMenu	m_FileMenu;
	CMenu	m_SourcesMenu;
	CMenu	m_DropMenu;//Xman Xtreme Downloadmanager
	bool		m_bRemainSort;
	CFont		m_fontBold; // may contain a locally created bold font
	CFont*		m_pFontBold;// points to the bold font which is to be used (may be the locally created or the default bold font)
	
	CToolTipCtrlX* m_tooltip;

	void ShowFileDialog(UINT uInvokePage);
	//int GetFilesCountInCurCat(); //Xman see all sources
	virtual void GetItemDisplayText(const CAbstractFile* file, int iSubItem, LPTSTR pszText, int cchTextMax) const;

	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnListModified(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnColumnClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNmDblClk(NMHDR *pNMHDR, LRESULT *pResult);
	//afx_msg void OnSysColorChange();
};
