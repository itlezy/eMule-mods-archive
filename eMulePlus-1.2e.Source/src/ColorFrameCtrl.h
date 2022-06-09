// ColorFrameCtrl.h : header file
//

#ifndef __ColorFrameCtrl_H__
#define __ColorFrameCtrl_H__

/////////////////////////////////////////////////////////////////////////////
// CColorFrameCtrl window

class CColorFrameCtrl : public CWnd
{
public:
	CColorFrameCtrl();
	virtual ~CColorFrameCtrl();

	void SetFrameColor(COLORREF color);
	void SetBackgroundColor(COLORREF color);

	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID=NULL);

protected:
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()

private:
	COLORREF m_crBackColor;        // background color
	CRect  m_rectClient;
	CBrush 	m_brushFrame;
};

/////////////////////////////////////////////////////////////////////////////
#endif
