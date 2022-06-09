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
#include "Loggable.h"
#include <vector>

#ifndef USRMSG_SWITCHUPLOADLIST
#define USRMSG_SWITCHUPLOADLIST	WM_USER + 12
#endif

enum EnumClientFilter
{
	CLI_FILTER_NONE		= 0,
	CLI_FILTER_BANNED	= 1,
	CLI_FILTER_FRIEND	= 2,
	CLI_FILTER_CREDIT	= 3
};

class CUpDownClient;

class CQueueListCtrl : public CMuleListCtrl, public CLoggable
{
	DECLARE_DYNAMIC(CQueueListCtrl)

public:
	CQueueListCtrl();
	virtual ~CQueueListCtrl();
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	void	Init();
	void	AddClient(CUpDownClient* pClient);
	void	RemoveClient(CUpDownClient* client);
	void	UpdateClient(CUpDownClient* client);
	void	SortInit(int sortCode);
	void	Localize();
	void	ShowFilteredList();
	int		GetClientFilterType(CUpDownClient* client);
	int		m_iClientFilter;
	void	ShowSelectedUserDetails();

	virtual BOOL OnCommand(WPARAM wParam,LPARAM lParam );
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkQueuelist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

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

	bool			m_bSortAscending[QLCOL_NUMCOLUMNS];
	int			m_iColumnMaxWidths[QLCOL_NUMCOLUMNS];
	int			m_iMeasuringColumn;
	ClientVector*		m_pvecDirtyClients;

private:
	void RefreshInfo(void);
	bool AddDirtyClient(CUpDownClient* pClientItem);
	ClientVector* GetDirtyClients();
};
