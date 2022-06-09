#if !defined(AFX_SCROLLSTATIC_H__6EE7A28A_205D_44DA_A9C4_9EE34154EA52__INCLUDED_)
#define AFX_SCROLLSTATIC_H__6EE7A28A_205D_44DA_A9C4_9EE34154EA52__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ScrollStatic.h : header file
// (p) 2003 by FoRcHa

/////////////////////////////////////////////////////////////////////////////
// CScrollStatic window

class CScrollStatic : public CStatic
{
// Construction
public:
	CScrollStatic();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScrollStatic)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CScrollStatic();
	void SetWindowText(LPCTSTR szText);

	// Generated message map functions
//protected:
	int		m_nOffset;
	int		m_nTextWidth;
	
	CString m_strText;

	CPoint  m_cpLastPoint;
	
	BOOL	m_bMouseIn;
	BOOL	m_bLButtonDown;
	
	HCURSOR m_hOldCursor;
		
	//{{AFX_MSG(CScrollStatic)
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCROLLSTATIC_H__6EE7A28A_205D_44DA_A9C4_9EE34154EA52__INCLUDED_)
