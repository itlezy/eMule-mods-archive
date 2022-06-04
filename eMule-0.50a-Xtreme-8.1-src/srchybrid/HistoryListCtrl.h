//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
//
// HistoryListCtrl. emulEspaña Mod: Added by MoNKi
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

#include "emule.h"
#include "types.h"
#include "MuleListCtrl.h"
#include "KnownFile.h"
#include "ListCtrlItemWalk.h"

// CHistoryListCtrl

class CHistoryListCtrl : public CMuleListCtrl, public CListCtrlItemWalk
{
	DECLARE_DYNAMIC(CHistoryListCtrl)

public:
	CHistoryListCtrl();
	virtual ~CHistoryListCtrl();
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	void	Init(void);
	void	AddFile(CKnownFile* toadd);
	void	Localize();
	void	CreateMenues();
	void	Reload(void);
	void	ShowComments(CKnownFile* file);
	void	RemoveFile(CKnownFile* toremove);
	void	RemoveFileFromView(CKnownFile* toremove); //only used for removing duplicated files
	void	ClearHistory();
	void	UpdateFile(const CKnownFile* file);

protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnLvnColumnClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
	void ShowFileDialog(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage = 0);
	void GetItemDisplayText(CKnownFile* file, int iSubItem, LPTSTR pszText, int cchTextMax) const;
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	int FindFile(const CKnownFile* pFile);

private:
	CTitleMenu	m_HistoryMenu;
	CTitleMenu	m_HistoryOpsMenu;
	void		OpenFile(CKnownFile* file);
};
