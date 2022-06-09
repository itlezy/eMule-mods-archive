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


#ifndef OT_ITEMCOLOR
#define OT_ITEMCOLOR

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionTreeItemColor.h : header file
//

// Added Headers
#include "CommonRes.h"
#include "OptionTreeDef.h"
#include "OptionTreeItem.h"
#include "OptionTreeColorPopUp.h"

// Classes
class COptionTree;

/////////////////////////////////////////////////////////////////////////////
// COptionTreeItemColor window

class COptionTreeItemColor : public CWnd, public COptionTreeItem
{
// Construction
public:
	COptionTreeItemColor();
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
	//{{AFX_VIRTUAL(COptionTreeItemColor)
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL GetOption(DWORD dwOption);
	void SetOption(DWORD dwOption, BOOL bSet);
	COLORREF GetAutomaticColor();
	void SetAutomaticColor(COLORREF crAutomatic);
	BOOL CreateColorItem(DWORD dwOptions, COLORREF rcColor, COLORREF rcAutomatic);
	void SetColor(COLORREF rcColor);
	COLORREF GetColor();
	virtual ~COptionTreeItemColor();

	// Generated message map functions
protected:
	void DrawControl(CDC *pDC, const RECT &rcRect);
	BOOL m_bFocus;
	COLORREF m_crColor;
	COLORREF m_crAutomatic;
	DWORD m_dwOptions;
	//{{AFX_MSG(COptionTreeItemColor)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	//}}AFX_MSG
	afx_msg long OnSelEndOK(UINT lParam, long wParam);
    afx_msg long OnSelEndCancel(UINT lParam, long wParam);
    afx_msg long OnSelChange(UINT lParam, long wParam);
	afx_msg long OnCloseColorPopUp(UINT lParam, long wParam);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !OT_ITEMCOLOR
