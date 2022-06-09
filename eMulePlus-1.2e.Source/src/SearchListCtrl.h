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

#include "MuleListCtrl.h"

class CSearchList;
class CSearchFile;

class CSearchListCtrl : public CMuleListCtrl
{
	DECLARE_DYNAMIC(CSearchListCtrl)
public:
					CSearchListCtrl();
	virtual		   ~CSearchListCtrl();
	void			Init(CSearchList *pSearchList);
	void			UpdateChangingColumns(CSearchFile *pSearchFile);
	void			AddResult(CSearchFile *pSearchFile);
	void			RemoveResult(CSearchFile *pSearchFile);
	void			SortInit(int iSortCode);
	void			Localize();
	void			ShowResults(uint32 dwResultsID);
	void			NoTabs()							{ m_dwResultsID = 0; }
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

protected:
	static	int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd *pWnd, CPoint point);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG *pMsg);
	virtual void	OnNMDividerDoubleClick(NMHEADER *pNMHDR);

	DECLARE_MESSAGE_MAP()
private:
	void			UpdateLine(int iIndex, CSearchFile *pSearchFile);

private:
	uint32			m_dwResultsID;
	CSearchList		*m_pSearchList;
	CImageList		m_imageList;
	bool			m_bSortAscending[SL_NUMCOLUMNS + 1];
};
