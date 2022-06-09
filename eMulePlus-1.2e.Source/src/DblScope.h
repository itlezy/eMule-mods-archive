#if !defined(AFX_DBLSCOPE_H__D7EDB6CB_03EF_4C2D_9094_DC04B4825EFD__INCLUDED_)
#define AFX_DBLSCOPE_H__D7EDB6CB_03EF_4C2D_9094_DC04B4825EFD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DblScope.h : header file
//

#include <afxtempl.h>

// !!! ATTENTION !!!: this control is about 10% done, if you want to work on it, 
//					  then pleaase send me a message, and maybe the finished code *g*, 
//					  otherwise i will continue working on it on my own. ...sometime :D  
//					  [FoRcHa]

/////////////////////////////////////////////////////////////////////////////
// CDblScope window

#define _ALPHA_BLEND_
//#define _OUTLINE_
#define _SCANLINES_

class CDblScope : public CWnd
{
// Construction
public:
	CDblScope();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDblScope)
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetRange(UINT nMin, UINT nMax, bool bDelete = false);
	void GetRange(UINT& nMax, UINT& nMin)
	{
		nMax = m_nMax;
		nMin = m_nMin;
	}
	void ResetGraph(bool bDelete = true);
	void AddValues(UINT nVal1, UINT nVal2, bool bRedraw = true);
	virtual ~CDblScope();

	// Generated message map functions
protected:
	//{{AFX_MSG(CDblScope)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void AddValues2Graph(UINT nVal1, UINT nVal2);
	void ReCreateGraph(CDC *pDC);
	
	CBitmap *m_pOldBMP; // FoRcHa
	CBitmap m_MemBMP;
	CDC		m_MemDC;
	bool	m_bInit;

	UINT m_nMax;
	UINT m_nMin;
	UINT m_nStep;

	CList<UINT,UINT> m_List1;
	CList<UINT,UINT> m_List2;

	COLORREF m_crColor1;
	COLORREF m_crColor2;
	COLORREF m_crBackColor;

	bool m_bScanLines;
	CPen m_ScanPen;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DBLSCOPE_H__D7EDB6CB_03EF_4C2D_9094_DC04B4825EFD__INCLUDED_)
