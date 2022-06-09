// COptionTree
//
// License
// -------
// This code is provided "as is" with no expressed or implied warranty.
// 
// You may use this code in a commercial product with or without acknowledgement.
// However you may not sell this code or any modification of this code, this includes 
// commercial libraries and anything else for profit.
//
// I would appreciate a notification of any bugs or bug fixes to help the control grow.
//
// History:
// --------
//	See License.txt for full history information.
//
//
// Copyright (c) 1999-2002 
// ComputerSmarts.net 
// mattrmiller@computersmarts.net


#ifndef OT_ITEMCOMBOBOX
#define OT_ITEMCOMBOBOX

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionTreeItemComboBox.h : header file
//

// Added Headers
#include "OptionTreeItem.h"

// Classes
class COptionTree;

/////////////////////////////////////////////////////////////////////////////
// COptionTreeItemComboBox window

class COptionTreeItemComboBox : public CComboBox, public COptionTreeItem
{
// Construction
public:
	COptionTreeItemComboBox();
	virtual void OnMove();
	virtual void OnRefresh();
	virtual void OnCommit();
	virtual void OnActivate();
	virtual void CleanDestroyWindow();
	virtual void OnDeSelect();
	virtual void OnSelect();
	virtual void DrawAttribute(CDC *pDC, const RECT &rcRect);
// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionTreeItemComboBox)
	//}}AFX_VIRTUAL

// Implementation
public:
	long GetDropDownHeight();
	void SetDropDownHeight(long lHeight);
	BOOL CreateComboItem(DWORD dwAddStyle = 0);
	virtual ~COptionTreeItemComboBox();

	// Generated message map functions
protected:
	BOOL m_bFocus;
	long m_lDropDownHeight;
	//{{AFX_MSG(COptionTreeItemComboBox)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !OT_ITEMCOMBOBOX
