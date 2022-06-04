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
#include "kademlia/routing/contact.h"

class CKademliaWnd;

class CKadContactListCtrl : public CMuleListCtrl
{
	DECLARE_DYNAMIC(CKadContactListCtrl)

public:
	CKadContactListCtrl();
	virtual ~CKadContactListCtrl();

protected:
	friend class CKademliaWnd;

	bool ContactAdd(const Kademlia::CContact *contact);
	void ContactRem(const Kademlia::CContact *contact);
	void ContactRef(const Kademlia::CContact *contact);

	void Init();
	void Localize();
	void SaveAllSettings();
	void UpdateKadContactCount();
	//void UpdateContact(int iItem, const Kademlia::CContact *contact); //from eMuleFuture :: IP2Country
	void SetAllIcons();

	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

	virtual BOOL OnCommand(WPARAM wParam,LPARAM lParam);
	//from eMuleFuture :: IP2Country :: Start
	virtual void	DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	CImageList	m_ImageList;
	void GetItemDisplayText(const Kademlia::CContact* contact, int iSubItem, LPTSTR pszText, int cchTextMax) const;
	//from eMuleFuture :: IP2Country :: End

	DECLARE_MESSAGE_MAP()
	afx_msg void OnLvnColumnClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult); //from eMuleFuture :: IP2Country 
	afx_msg void OnNmDblClk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSysColorChange();
};
