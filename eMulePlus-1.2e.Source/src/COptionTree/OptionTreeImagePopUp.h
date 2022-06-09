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


#ifndef OT_IMAGEPOPUP
#define OT_IMAGEPOPUP

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionTreeImagePopUp.h : header file
//

// Added Headers
#include "CommonRes.h"
#include "OptionTreeDef.h"
#include "OptionTreeItem.h"
#include <afxtempl.h>

// Classes
class COptionTree;

/////////////////////////////////////////////////////////////////////////////
// COptionTreeImagePopUp window

class COptionTreeImagePopUp : public CWnd
{
// Construction
public:
	COptionTreeImagePopUp(CPoint pPoint, CSize sImageSize, int nNumColumns, CImageList *imgImageList, COptionTree *otImageOption, CStringArray &strText, int nSelected, CWnd* pParentWnd);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionTreeImagePopUp)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~COptionTreeImagePopUp();

	// Generated message map functions
protected:
	void SetUpImages();
	void SetUpToolTips();
	void SetWindowSize();
	BOOL Create(CPoint pPoint);
	CToolTipCtrl m_ttToolTip;
	CPoint m_pHover;
	COptionTree *m_otImageOption;
	CImageList *m_pImageList;
	CWnd *m_pParent;
	CSize m_sImageSizes;
	int m_nNumColumns;
	int m_nNumRows;
	CStringArray m_strText;
	CArray<CRect, CRect&> m_rcRectangles;
	int m_nSelected;
	//{{AFX_MSG(COptionTreeImagePopUp)
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNcDestroy();
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !OT_IMAGEPOPUP
