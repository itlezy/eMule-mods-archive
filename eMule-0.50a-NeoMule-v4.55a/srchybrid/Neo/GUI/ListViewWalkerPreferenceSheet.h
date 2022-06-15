//this file is part of NeoMule
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
#include "ResizableLib/ResizableSheet.h"
#include "ListCtrlItemWalk.h"

#define IDC_PREV	100
#define IDC_NEXT	101
// NEO: FCFG - [FileConfiguration]
#define IDC_CPY		102
#define IDC_PST		103
#define IDC_RST		104
// NEO: FCFG END

// CListViewWalkerPreferenceSheet

class CListViewWalkerPreferenceSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CListViewWalkerPreferenceSheet)

public:
	//CListViewWalkerPreferenceSheet(CListCtrlItemWalk* pListCtrl) 
	CListViewWalkerPreferenceSheet(CListCtrlItemWalk* pListCtrl = NULL) // NEO: MLD - [ModelesDialogs] <-- Xanatos -- 
	{
		m_pListCtrl = pListCtrl;
	}
	CListViewWalkerPreferenceSheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CListViewWalkerPreferenceSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~CListViewWalkerPreferenceSheet();

protected:
	CListCtrlItemWalk* m_pListCtrl;
	CSimpleArray<CObject*> m_aItems;
	CButton m_ctlPrev;
	CButton m_ctlNext;
	// NEO: FCFG - [FileConfiguration]
	CButton m_ctlCopy;
	CButton m_ctlPaste;
	CButton m_ctlReset;
	// NEO: FCFG END

	void ChangeData(CObject* pObj);

	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnNext();
	afx_msg void OnPrev();
};
