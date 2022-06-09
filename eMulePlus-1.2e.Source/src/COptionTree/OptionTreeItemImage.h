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


#ifndef OT_ITEMIMAGE
#define OT_ITEMIMAGE

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionTreeItemImage.h : header file
//

// Added Headers
#include "CommonRes.h"
#include "OptionTreeDef.h"
#include "OptionTreeItem.h"
#include "OptionTreeImagePopUp.h"

// Classes
class COptionTree;

/////////////////////////////////////////////////////////////////////////////
// COptionTreeItemImage window

class COptionTreeItemImage : public CWnd, public COptionTreeItem
{
// Construction
public:
	COptionTreeItemImage();
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
	//{{AFX_VIRTUAL(COptionTreeItemImage)
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetSelection(int nSelection);
	int GetSelection();
	void AddIcon(CString strIcon, CString strText);
	void AddBitmap(CString strBitmap, COLORREF crMask, CString strText);
	void AddBitmap(UINT uBitmap, COLORREF crMask, CString strText);
	void AddBitmap(CBitmap &bBitmap, COLORREF crMask, CString strText);
	void AddIcon(UINT uIcon, CString strText);
	void AddIcon(HICON hIcon, CString strText);
	BOOL GetOption(DWORD dwOption);
	void SetOption(DWORD dwOption, BOOL bSet);
	void SetNumberColumns(int nNum);
	CSize GetImageSize();
	void SetImageSize(CSize sSize);
	BOOL CreateImageItem(DWORD dwOptions, CSize sImageSizes, int nNumberColumns);
	virtual ~COptionTreeItemImage();

	// Generated message map functions
protected:
	HICON CreateGreyScaleIcon(HICON hIcon);
	void DitherBlt(CDC *drawdc, int nXDest, int nYDest, int nWidth, int nHeight, CBitmap &bmp, int nXSrc, int nYSrc);
	BOOL GetBitmapFromImageList(CDC *pDC, HICON hIcon, CBitmap &bmp);
	void DrawControl(CDC *pDC, const RECT &rcRect);
	BOOL m_bFocus;
	int m_nNumColumns;
	CSize m_sImageSizes;
	int m_nSelected;
	CImageList m_imgImages;
	DWORD m_dwOptions;
	CStringArray m_strText;
	//{{AFX_MSG(COptionTreeItemImage)
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	afx_msg LONG OnPopUpClose(UINT lParam, long wParam);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !OT_ITEMIMAGE
