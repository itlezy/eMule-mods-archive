//this file is part of eMule
//Copyright (C)2002-2005 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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
#include "CollectionListCtrl.h"
#include "ResizableLib\ResizableDialog.h"
#include "Modeless.h"	// Tux: Feature: Modeless dialogs

class CCollection;

// Tux: Feature: Modeless dialogs [start]
//class CCollectionViewDialog : public CResizableDialog
class CCollectionViewDialog : public CModelessResizableDialog
// Tux: Feature: Modeless dialogs [end]
{
	DECLARE_DYNAMIC(CCollectionViewDialog)

public:
	CCollectionViewDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCollectionViewDialog();

	// Dialog Data
	enum { IDD = IDD_COLLECTIONVIEWDIALOG };

	// Tux: Feature: Modeless dialogs [start]
	//void SetCollection(CCollection* pCollection);
	void SetCollection(CCollection* pCollection, bool bDeleteThis = false);
	// Tux: Feature: Modeless dialogs [end]

protected:
	CButton m_AddNewCatagory;
	CStatic m_CollectionViewListLabel;
	CStatic m_CollectionViewListIcon;
	CButton m_CollectionDownload;
	CButton m_CollectionExit;
	CEdit m_CollectionViewAuthor;
	CEdit m_CollectionViewAuthorKey;
	CCollectionListCtrl m_CollectionViewList;
	CCollection* m_pCollection;
	HICON m_icoWnd;
	HICON m_icoColl;
	bool m_bDeleteThis;	// Tux: Feature: Modeless dialogs

	void DownloadSelected(void);

	virtual BOOL OnInitDialog(void);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedViewCollection();
	afx_msg void OnNmDblClkCollectionList(NMHDR *pNMHDR, LRESULT *pResult);
};
