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

struct Nick
{
	CString nick;
	CString op;
	CString hop;
	CString voice;
	CString uop;
	CString owner;
	CString protect;
};

class CIrcWnd;

class CIrcNickListCtrl : public CMuleListCtrl
{
	DECLARE_DYNAMIC(CIrcNickListCtrl)

public:
	CIrcNickListCtrl();
	void	Init();
	void	Localize();
	void	SetParent(CIrcWnd *pParent)		{ m_pParent = pParent; };

	void	UpdateNickCount();
	Nick*	NewNick(CString strChannel, CString strNick);
	Nick*	FindNickByName(CString strChannel, const CString &strName);
	void	RefreshNickList(CString strChannel);
	bool	RemoveNick(CString strChannel, const CString &strNick);
	void	DeleteAllNick(CString strChannel);
	void	DeleteNickInAll(const CString &strNick, const CString &strMsg);
	void	ParseChangeMode(CString strChannel, const CString &strChanger, const CString &strCommands, const CString &strNames);
	bool	ChangeNick(CString strChannel, const CString &strOldNick, const CString &strNewNick);

protected:
	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	afx_msg	void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnNMDblClk(NMHDR *pNMHDR, LRESULT *pResult);

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

private:
	void	SortInit(int iSortCode);
	bool	ChangeMode(CString strChannel, const CString &strNick, const CString &strMode);

private:
	CIrcWnd					*m_pParent;
	bool					m_bSortAscending[IRC1COL_NUMCOLUMNS];
};
