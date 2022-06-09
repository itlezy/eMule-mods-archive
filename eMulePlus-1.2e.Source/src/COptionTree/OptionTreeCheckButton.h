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


#ifndef OT_CHECKBUTTON
#define OT_CHECKBUTTON

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionTreeCheckButton.h : header file
//

// Added Headers
#include "OptionTreeDef.h"
#include "OptionTreeItem.h"

// Classes
class COptionTree;

/////////////////////////////////////////////////////////////////////////////
// COptionTreeCheckButton window

class COptionTreeCheckButton : public CWnd
{
// Construction
public:
	COptionTreeCheckButton();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionTreeCheckButton)
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL GetOption(DWORD dwOption);
	void SetOption(DWORD dwOption, BOOL bSet);
	void SetCheckOptionsOwner(COptionTree *otOption);
	BOOL GetCheck();
	void SetCheck(BOOL bCheck);
	CString GetCheckedText();
	CString GetUnCheckedText();
	void SetCheckText(CString strChecked, CString strUnChecked);
	virtual ~COptionTreeCheckButton();

	// Generated message map functions
protected:
	CString m_strUnChecked;
	CString m_strChecked;
	BOOL m_bShowText;
	BOOL m_bShowCheck;
	BOOL m_bCheck;
	CRect m_rcCheck;
	COptionTree *m_otCheckOption;
	DWORD m_dwOptions;
	//{{AFX_MSG(COptionTreeCheckButton)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !OT_CHECKBUTTON
