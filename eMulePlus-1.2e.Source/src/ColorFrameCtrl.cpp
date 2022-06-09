// ColorFrameCtrl.cpp : implementation file

#include "stdafx.h"
#include "ColorFrameCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorFrameCtrl
CColorFrameCtrl::CColorFrameCtrl()
{
	m_crBackColor  = RGB(0,   0,   0);  // see also SetBackgroundColor
	m_brushFrame.CreateSolidBrush(RGB(0, 255, 255));	// frame color
}

/////////////////////////////////////////////////////////////////////////////
CColorFrameCtrl::~CColorFrameCtrl()
{
}


BEGIN_MESSAGE_MAP(CColorFrameCtrl, CWnd)
	//{{AFX_MSG_MAP(CColorFrameCtrl)
	ON_WM_PAINT()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CColorFrameCtrl message handlers

/////////////////////////////////////////////////////////////////////////////
BOOL CColorFrameCtrl::Create(DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID)
{
	BOOL result;
	static CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW);

	result = CWnd::CreateEx( WS_EX_STATICEDGE,
		                      className, NULL, dwStyle,
		                      rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top,
		                      pParentWnd->GetSafeHwnd(), (HMENU)nID);
	if (result != 0)
		Invalidate();

	return result;
}

/////////////////////////////////////////////////////////////////////////////
void CColorFrameCtrl::SetFrameColor(COLORREF color)
{
	m_brushFrame.DeleteObject();
	m_brushFrame.CreateSolidBrush(color);

	// clear out the existing garbage, re-start with a clean plot
	Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
void CColorFrameCtrl::SetBackgroundColor(COLORREF color)
{
	m_crBackColor = color;

	// clear out the existing garbage, re-start with a clean plot
	Invalidate();
}

////////////////////////////////////////////////////////////////////////////
void CColorFrameCtrl::OnPaint()
{
	CPaintDC dc(this);  // device context for painting

	dc.FillSolidRect(m_rectClient, m_crBackColor);
	dc.FrameRect(m_rectClient, &m_brushFrame);
}

/////////////////////////////////////////////////////////////////////////////
void CColorFrameCtrl::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	// NOTE: OnSize automatically gets called during the setup of the control

	GetClientRect(m_rectClient);
}
