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

#ifndef OT_ITEMHYPERLINK
#define OT_ITEMFONT

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionTreeItemFont.h : header file
//

// Added Headers
#include "CommonRes.h"
#include "OptionTreeDef.h"
#include "OptionTreeItem.h"
#include "OptionTreeFontSel.h"

/////////////////////////////////////////////////////////////////////////////
// COptionTreeItemHyperLink window

class COptionTreeItemHyperLink : public CWnd, public COptionTreeItem
{
// Construction
public:
	COptionTreeItemHyperLink();
	virtual void OnMove();
	virtual void OnRefresh();
	virtual void OnCommit();
	virtual void OnActivate();
	virtual void CleanDestroyWindow();
	virtual void OnDeSelect();
	virtual void OnSelect();
	virtual void DrawAttribute(CDC *pDC, const RECT &rcRect);
	virtual void OnExpand(BOOL bExpand);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionTreeItemHyperLink)
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetVisited(BOOL bVisited);
	BOOL GetOption(DWORD dwOption);
	void SetOption(DWORD dwOption, BOOL bSet);
	void SetLink(CString strLink);
	CString GetLink();
	COLORREF GetVisitedColor();
	COLORREF GetHoverColor();
	COLORREF GetLinkColor();
	void SetVisitedColor(COLORREF crColor);
	void SetHoverColor(COLORREF crColor);
	void SetLinkColor(COLORREF crColor);
	BOOL GetVisited();
	HCURSOR GetLinkCursor();
	void SetLinkCursor(HCURSOR hCursor);
	BOOL CreateHyperlinkItem(DWORD dwOptions, CString strLink, COLORREF crLink, COLORREF crHover = NULL, COLORREF crVisited = NULL);
	virtual ~COptionTreeItemHyperLink();

	// Generated message map functions
protected:
	BOOL m_bFocus;
	void SetDefaultCursor();
	COLORREF m_crLink;
	COLORREF m_crHover;
	COLORREF m_crVisited;
	BOOL m_bVisited;
	DWORD m_dwOptions;
	CString m_strLink;
	HCURSOR  m_hLinkCursor;
	CRect m_rcHover;
	BOOL m_bHover;
	CFont m_fUnderlineFont;
	//{{AFX_MSG(COptionTreeItemHyperLink)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !OT_ITEMHYPERLINK
