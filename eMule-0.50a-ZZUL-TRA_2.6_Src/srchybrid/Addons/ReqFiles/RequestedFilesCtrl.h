//this file is part of eMule
//Copyright (C)2006-2007 David Xanatos ( XanatosDavid@googlemail.com / http://neomule.sourceforge.net )
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

class CKnownFile;
struct Requested_File_Struct;
class CRequestedFilesCtrl;
class CUpDownClient;

///////////////////////////////////////////////////////////////////////////////
// CReqFileItem
//
class CReqFileItem: public CObject
{
	DECLARE_DYNAMIC(CReqFileItem)
public:
	CKnownFile*	m_pKnownFile;
	Requested_File_Struct* m_pFileID;
	bool m_bNNP;
	bool m_bA4AF;
	CString	m_strFileName; // For faster Sorting;
};

///////////////////////////////////////////////////////////////////////////////
// CSharedFilesListCtrlItemWalk

enum ReqItemType {KNOWN_TYPE = 1, STRUCT_TYPE = 2};

class CRequestedFilesCtrlItemWalk : public CListCtrlItemWalk
{
public:
	CRequestedFilesCtrlItemWalk(CRequestedFilesCtrl* pListCtrl);

	virtual CObject* GetNextSelectableItem();
	virtual CObject* GetPrevSelectableItem();

	virtual void SetDataType(ReqItemType eDataType) { m_eDataType = eDataType; }

protected:
	CRequestedFilesCtrl* m_pRequestedFilesCtrl;
	ReqItemType m_eDataType;
};

class CRequestedFilesCtrl : public CMuleListCtrl, public CRequestedFilesCtrlItemWalk
{
	DECLARE_DYNAMIC(CRequestedFilesCtrl)

public:
	CRequestedFilesCtrl();
	virtual ~CRequestedFilesCtrl();

	void Init();
	void Localize();

	void SetClient(CUpDownClient* pClient) {m_pClient = pClient;}

	void AddFile(CKnownFile* reqfile, bool bNNP, bool bA4AF);
	void AddFile(Requested_File_Struct* requpfile, bool bA4AF);

	BOOL	DeleteAllItems(bool bOnDestroy = false);

protected:
	CReqFileItem* FindItem(CKnownFile* reqfile);
	CReqFileItem* FindItem(Requested_File_Struct* requpfile);
	void UpdateItem(CReqFileItem* pItem);
	void GetItemDisplayText(CReqFileItem *pItem, int iSubItem, LPTSTR pszText, int cchTextMax);
	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	DECLARE_MESSAGE_MAP()

	afx_msg	void OnLvnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNmDblClk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSysColorChange();
	afx_msg void OnDestroy();

	CUpDownClient* m_pClient;
};