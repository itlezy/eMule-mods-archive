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


#ifndef OT_ITEMCHECKBOX
#define OT_ITEMCHECKBOX

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionTreeItemCheckBox.h : header file
//

// Added Headers
#include "CommonRes.h"
#include "OptionTreeDef.h"
#include "OptionTreeItem.h"
#include "OptionTreeCheckButton.h"

// Classes
class COptionTree;

/////////////////////////////////////////////////////////////////////////////
// COptionTreeItemCheckBox window

class COptionTreeItemCheckBox : public COptionTreeCheckButton, public COptionTreeItem
{
// Construction
public:
	COptionTreeItemCheckBox();
	virtual void OnMove();
	virtual void OnRefresh();
	virtual void OnCommit();
	virtual void OnActivate();
	virtual void CleanDestroyWindow();
	virtual void DrawAttribute(CDC *pDC, const RECT &rcRect);
	virtual void OnDeSelect();
	virtual void OnSelect();

	BOOL CreateCheckBoxItem(BOOL bChecked, DWORD dwOptions);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionTreeItemCheckBox)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~COptionTreeItemCheckBox();

	// Generated message map functions
protected:
	BOOL m_bFocus;
	CRect m_rcCheck;
	//{{AFX_MSG(COptionTreeItemCheckBox)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !OT_ITEMCHECKBOX
