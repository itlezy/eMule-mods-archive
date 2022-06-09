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

class CFriend;

class CFriendListCtrl : public CMuleListCtrl
{
	DECLARE_DYNAMIC(CFriendListCtrl)
	friend class CFriendList;

public:
	CFriendListCtrl();
	virtual ~CFriendListCtrl();

	void	Init();
	void	Localize();
	virtual BOOL PreTranslateMessage(MSG *pMsg);
	CImageList	m_imageList;

protected:
	DECLARE_MESSAGE_MAP()

	void	AddFriend(CFriend* toadd);
	void	RemoveFriend(CFriend* toremove);
	void	RefreshFriend(CFriend* toupdate);
	BOOL	OnCommand(WPARAM wParam, LPARAM lParam);
	void	OnDoubleclick(NMHDR *pNMHDR, LRESULT *pResult);
	static	int	CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	afx_msg void OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnContextMenu(CWnd *pWnd, CPoint point);

private:
	bool	m_bSortAscending[FRIENDCOL_NUMCOLUMNS];

	void	ShowFriendCount();
	void	SortInit(int iSortCode);
};
