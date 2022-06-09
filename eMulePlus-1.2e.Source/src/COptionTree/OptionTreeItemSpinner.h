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


#ifndef OT_ITEMSPINNER
#define OT_ITEMSPINNER

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionTreeItemSpinner.h : header file
//


// Added Headers
#include "CommonRes.h"
#include "OptionTreeDef.h"
#include "OptionTreeItem.h"
#include "OptionTreeSpinnerButton.h"

// Classes
class COptionTree;


/////////////////////////////////////////////////////////////////////////////
// COptionTreeItemSpinner window

class COptionTreeItemSpinner : public COptionTreeSpinnerButton, public COptionTreeItem
{
// Construction
public:
	COptionTreeItemSpinner();
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
	//{{AFX_VIRTUAL(COptionTreeItemSpinner)
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL CreateSpinnerItem(DWORD dwOptions, double dValue, double dRangeBottom, double dRangeTop);
	virtual ~COptionTreeItemSpinner();

protected:
	LRESULT WM_CommitChanges(WPARAM wParam, LPARAM lParam);

	// Generated message map functions
protected:
	BOOL m_bFocus;

	//{{AFX_MSG(COptionTreeItemSpinner)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !OT_ITEMSPINNER
