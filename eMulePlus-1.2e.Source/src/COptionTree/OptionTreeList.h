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


#ifndef OT_LIST
#define OT_LIST

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionTreeList.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionTreeList window

// Classes
class COptionTree;


class COptionTreeList : public CWnd
{
// Construction
public:
	COptionTreeList();
	BOOL Create(DWORD dwStyle, RECT rcRect, CWnd* pParentWnd, UINT nID);

// Implementation
public:
	void UpdateResize();
	virtual ~COptionTreeList();

	// Generated message map functions
protected:
	// CPropTree class that this class belongs
	COptionTree *m_otOption;
	long m_lPrevCol;
	BOOL m_bColDrag;
	HCURSOR m_hSplitter;
	HCURSOR m_hHand;
	long m_lColumn;
	//{{AFX_MSG(COptionTreeList)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg UINT OnGetDlgCode();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void GetHandCursor();
	void SetOptionsOwner(COptionTree *otOption);
	void CheckVisibleFocus();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !OT_LIST
