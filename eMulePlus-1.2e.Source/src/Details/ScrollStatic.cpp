// ScrollStatic.cpp : implementation file
// (p) 2003 by FoRcHa

#include "stdafx.h"
#include "ScrollStatic.h"

#include "..\resource.h"
#include ".\scrollstatic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScrollStatic

CScrollStatic::CScrollStatic()
{
	m_nOffset = 0;
	m_nTextWidth = 0;
	m_bMouseIn = FALSE;
	m_bLButtonDown = FALSE;
	m_hOldCursor = NULL;
}

CScrollStatic::~CScrollStatic()
{
}


BEGIN_MESSAGE_MAP(CScrollStatic, CStatic)
	//{{AFX_MSG_MAP(CScrollStatic)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScrollStatic message handlers

void CScrollStatic::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect rClient;
	GetClientRect(rClient);

	CDC MemDC;
	CBitmap MemBMP;

	MemDC.CreateCompatibleDC(&dc);
	MemBMP.CreateCompatibleBitmap(&dc, rClient.Width(), rClient.Height());

	CBitmap *pOldBMP = MemDC.SelectObject(&MemBMP);
	CGdiObject* pOldFont = MemDC.SelectStockObject(DEFAULT_GUI_FONT);

	CStatic::DefWindowProc(WM_PAINT, (WPARAM)MemDC.m_hDC, NULL);
	MemDC.SetBkMode(TRANSPARENT);

	CRect rText = rClient;
	rText.left -= m_nOffset;

	MemDC.DrawText(m_strText, rText, DT_NOPREFIX|DT_NOCLIP|DT_SINGLELINE|DT_LEFT|DT_VCENTER);

	CRect rTest = rClient;
	MemDC.DrawText(m_strText, rTest, DT_CALCRECT|DT_NOPREFIX|DT_NOCLIP|DT_SINGLELINE|DT_LEFT|DT_VCENTER);
	m_nTextWidth = rTest.Width();

	dc.BitBlt(0, 0, rClient.Width(), rClient.Height(), &MemDC, 0, 0, SRCCOPY);

	MemDC.SelectObject(pOldFont);
	MemDC.SelectObject(pOldBMP);
}

BOOL CScrollStatic::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

void CScrollStatic::OnMouseMove(UINT nFlags, CPoint point)
{
	CRect rClient;
	GetClientRect(rClient);
	ClientToScreen(&rClient);

	CPoint cpPoint = point;
	ClientToScreen(&cpPoint);

	if(rClient.PtInRect(cpPoint))
	{
		if(!m_bMouseIn)
		{
			m_bMouseIn = TRUE;
			SetCapture();

			if(m_nTextWidth > rClient.Width())
			{
				m_hOldCursor = ::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
			}
		}
	}
	else
	{
		if(!m_bLButtonDown)
		{
			ReleaseCapture();

			if(m_hOldCursor != NULL)
			{
				::SetCursor(m_hOldCursor);
				m_hOldCursor = NULL;
			}
		}

		m_bMouseIn = FALSE;
	}

	if(m_bLButtonDown)
	{
		int nLastOffset = m_nOffset;
		CPoint cpDiff = m_cpLastPoint - point;
		m_cpLastPoint = point;
		m_nOffset = min(m_nOffset + cpDiff.x, m_nTextWidth - rClient.Width());
		m_nOffset = max(0, m_nOffset);
		if(nLastOffset != m_nOffset)
		{
			Invalidate();
		}
	}

	CStatic::OnMouseMove(nFlags, point);
}

void CScrollStatic::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_cpLastPoint = point;
	m_bLButtonDown = TRUE;
	CStatic::OnLButtonDown(nFlags, point);
}

void CScrollStatic::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bLButtonDown = FALSE;

	if(!m_bMouseIn)
	{
		ReleaseCapture();

		if(m_hOldCursor != NULL)
		{
			::SetCursor(m_hOldCursor);
			m_hOldCursor = NULL;
		}
	}

	CStatic::OnLButtonUp(nFlags, point);
}

void CScrollStatic::SetWindowText(LPCTSTR lpszString)
{
	m_strText = lpszString;
	CStatic::SetWindowText(_T(""));
	if (GetSafeHwnd() != NULL)
		Invalidate();
}
