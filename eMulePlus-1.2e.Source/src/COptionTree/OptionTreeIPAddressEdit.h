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

#ifndef OT_ITEMIPADDRESSEDIT
#define OT_ITEMIPADDRESSEDIT


// Added Headers
#include "CommonRes.h"
#include "OptionTreeDef.h"

// Classes
class COptionTree;

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionTreeIPAddressEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionTreeIPAddressEdit window

class COptionTreeIPAddressEdit : public CEdit
{
// Construction
public:
	COptionTreeIPAddressEdit();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionTreeIPAddressEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetNotifyWnd(HWND hWnd);
	virtual ~COptionTreeIPAddressEdit();

	// Generated message map functions
protected:
	HWND m_wndNotify;
	//{{AFX_MSG(COptionTreeIPAddressEdit)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnMaxText();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !OT_ITEMIPADDRESSEDIT
