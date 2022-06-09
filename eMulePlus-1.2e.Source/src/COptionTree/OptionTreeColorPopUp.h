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

#ifndef OT_COLORPOPUP
#define OT_COLORPOPUP

#pragma once 

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// OptionTreeColorPopUp.h : header file
//

// Added Headers
#include "OptionTreeDef.h"
#include "OptionTreeItem.h"

// Classes
class COptionTree;


// Structures
typedef struct 
{
    COLORREF crColor;
    TCHAR    *szName;
} OT_COLOR_ITEM;

/////////////////////////////////////////////////////////////////////////////
// COptionTreeColorPopUp window

class COptionTreeColorPopUp : public CWnd
{
// Construction
public:
    COptionTreeColorPopUp();
    COptionTreeColorPopUp(CPoint pPoint, COLORREF crColor, COLORREF crDefault, CWnd* pParentWnd, LPCTSTR szDefaultText = NULL, LPCTSTR szCustomText = NULL);
    void Initialize();

// Attributes
public:

// Operations
public:
    BOOL Create(CPoint pPoint, COLORREF crColor, CWnd* pParentWnd, LPCTSTR szDefaultText = NULL, LPCTSTR szCustomText = NULL);

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(COptionTreeColorPopUp)
    public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~COptionTreeColorPopUp();

protected:
    BOOL GetCellRect(int nIndex, const LPRECT& rect);
    void FindCellFromColor(COLORREF crColor);
    void SetWindowSize();
    void CreateToolTips();
    void ChangeSelection(int nIndex);
    void EndSelection(int nMessage);
    void DrawCell(CDC* pDC, int nIndex);

    COLORREF GetColor(int nIndex)              { return m_crColors[nIndex].crColor; }
    LPCTSTR GetColorName(int nIndex)           { return m_crColors[nIndex].szName; }
    int  GetIndex(int nRow, int nCol) const;
    int  GetRow(int nIndex) const;
    int  GetColumn(int nIndex) const;

// protected attributes
protected:
    static OT_COLOR_ITEM m_crColors[];
    int m_nNumColors;
    int m_nNumColumns, m_nNumRows;
    int m_nBoxSize, m_nMargin;
    int m_nCurrentSel;
    int m_nChosenColorSel;
    CString m_strDefaultText;
    CString m_strCustomText;
    CRect m_rcCustomTextRect, m_rcDefaultTextRect, m_rcWindowRect;
    CFont m_fFont;
    CPalette m_plPalette;
    COLORREF m_crInitialColor, m_crColor, m_crDefault;
    CToolTipCtrl m_ttToolTip;
    CWnd* m_wndParent;

    BOOL m_bChildWindowVisible;

    // Generated message map functions
protected:
    //{{AFX_MSG(COptionTreeColorPopUp)
    afx_msg void OnNcDestroy();
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnPaint();
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg BOOL OnQueryNewPalette();
    afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !OT_COLORPOPUP
