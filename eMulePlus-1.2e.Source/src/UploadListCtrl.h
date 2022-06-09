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
#include <vector>

#ifndef USRMSG_SWITCHUPLOADLIST
#define USRMSG_SWITCHUPLOADLIST	WM_USER + 12
#endif

class CUpDownClient;

class CUploadListCtrl : public CMuleListCtrl
{
	DECLARE_DYNAMIC(CUploadListCtrl)

public:
	CUploadListCtrl();
	virtual ~CUploadListCtrl();
	void	Init();
	void	AddClient(CUpDownClient* client);
	void	RemoveClient(CUpDownClient* client);
	void	UpdateClient(CUpDownClient* client);
	void	SortInit(int sortCode);
	void	Localize();
	virtual BOOL OnCommand(WPARAM wParam,LPARAM lParam );
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	void	ShowSelectedUserDetails();

	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkUploadlist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	afx_msg	void OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);

	virtual void OnNMDividerDoubleClick(NMHEADER *pNMHDR);
	virtual BOOL OnWndMsg(UINT iMessage, WPARAM wParam, LPARAM lParam, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()

private:
	typedef vector<CUpDownClient*>		ClientVector;

	bool			m_bSortAscending[ULCOL_NUMCOLUMNS];
	int				m_iColumnMaxWidths[ULCOL_NUMCOLUMNS];
	int				m_iMeasuringColumn;
	ClientVector*		m_pvecDirtyClients;

private:
	void RefreshInfo(void);
	bool AddDirtyClient(CUpDownClient* pClientItem);
	ClientVector* GetDirtyClients();
};
