// TrayMenuBtn.h : header file
//
#pragma once

/////////////////////////////////////////////////////////////////////////////
// CTrayMenuBtn window

class CTrayMenuBtn : public CWnd
{
public:
	CTrayMenuBtn();
	virtual ~CTrayMenuBtn();

private:
	bool	m_bMouseOver;

public:
	bool	m_bNoHover;
	bool	m_bUseIcon;
	bool	m_bParentCapture;
	UINT	m_nBtnID;
	CSize	m_sIcon;
	HICON	m_hIcon;
	CString m_strText;
	CFont	m_cfFont;

	//{{AFX_MSG(CTrayMenuBtn)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
