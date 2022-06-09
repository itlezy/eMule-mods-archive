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

class CUpDownClient;

class CClientListCtrl : public CMuleListCtrl
{
	DECLARE_DYNAMIC(CClientListCtrl)

public:
					CClientListCtrl();
	virtual		   ~CClientListCtrl();
	virtual void	DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	void			Init();
	void			AddClient(CUpDownClient* client);
	void			RemoveClient(CUpDownClient* client);
	void			UpdateClient(CUpDownClient* client);
	void			Localize();
	void			ShowKnownClients();
	void			SortInit(int sortCode);
	virtual BOOL	OnCommand(WPARAM wParam,LPARAM lParam);
	void			ShowSelectedUserDetails();

protected:
	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static int		SortClient(CUpDownClient* item1, CUpDownClient* item2, int sortMod);
	afx_msg	void	OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void	OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void	OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void	OnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void	OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void	OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

	virtual void	OnNMDividerDoubleClick(NMHEADER *pNMHDR);
	virtual BOOL	OnWndMsg(UINT iMessage, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	virtual BOOL	PreTranslateMessage(MSG *pMsg);

	DECLARE_MESSAGE_MAP()

private:
	typedef vector<CUpDownClient*>		ClientVector;

	bool			m_bSortAscending[CLCOL_NUMCOLUMNS];
	int			m_iColumnMaxWidths[CLCOL_NUMCOLUMNS];
	int			m_iMeasuringColumn;
	ClientVector		*m_pvecDirtyClients;	// List of clients that need client list ctrl refresh

private:
	bool				AddDirtyClient(CUpDownClient *pClient);
	ClientVector*		GetDirtyClients();
	void				RefreshInfo(void);
};
