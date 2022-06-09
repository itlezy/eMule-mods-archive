// TrayMenuBtn.cpp : implementation file
//

#include "stdafx.h"
#include "TrayMenuBtn.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTrayMenuBtn

CTrayMenuBtn::CTrayMenuBtn()
{
	m_bMouseOver = false;
	m_bNoHover = false;
	m_bUseIcon = false;
	m_bParentCapture = false;
	m_hIcon = NULL;
	m_nBtnID = 0;
	m_sIcon.cx = 0;
	m_sIcon.cy = 0;
}

CTrayMenuBtn::~CTrayMenuBtn()
{
	if (m_hIcon != NULL)
		DestroyIcon(m_hIcon);
}


BEGIN_MESSAGE_MAP(CTrayMenuBtn, CWnd)
	//{{AFX_MSG_MAP(CTrayMenuBtn)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTrayMenuBtn message handlers

void CTrayMenuBtn::OnMouseMove(UINT nFlags, CPoint point) 
{
	CRect	rClient;

	GetClientRect(rClient);

	if ( point.x >= rClient.left && point.x <= rClient.right
		&& point.y >= rClient.top && point.y <= rClient.bottom )
	{
		SetCapture();

		if (!m_bNoHover)
			m_bMouseOver = true;
		Invalidate();
	}
	else
	{
		CWnd	*pParent;

		if (m_bParentCapture && (pParent = GetParent()) != NULL)
			pParent->SetCapture();
		else
			ReleaseCapture();

		m_bMouseOver = false;
		Invalidate();
	}
	
	CWnd::OnMouseMove(nFlags, point);
}

void CTrayMenuBtn::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CRect	rClient;

	GetClientRect(rClient);

	if (point.x >= rClient.left && point.x <= rClient.right
		&& point.y >= rClient.top && point.y <= rClient.bottom )
	{
		CWnd	*pParent = GetParent();

		if (pParent != NULL)
			pParent->PostMessage(WM_COMMAND, MAKEWPARAM(m_nBtnID, BN_CLICKED), reinterpret_cast<LPARAM>(m_hWnd));
	}
	else
	{
		ReleaseCapture();
		m_bMouseOver = false;
		Invalidate();
	}		
	
	CWnd::OnLButtonUp(nFlags, point);
}

void CTrayMenuBtn::OnPaint() 
{
	CPaintDC	dc(this); // device context for painting
	CRect		rClient;

	GetClientRect(rClient);

	CDC	MemDC;

	MemDC.CreateCompatibleDC(&dc);

	CBitmap	MemBMP, *pOldBMP;

	MemBMP.CreateCompatibleBitmap(&dc, rClient.Width(), rClient.Height());
	pOldBMP = MemDC.SelectObject(&MemBMP);

	CFont	*pOldFONT = NULL;

	if (m_cfFont.GetSafeHandle())
		pOldFONT = MemDC.SelectObject(&m_cfFont);

	BOOL	bEnabled = IsWindowEnabled();

	if (m_bMouseOver && bEnabled)
	{	
		FillRect(MemDC.m_hDC, rClient, GetSysColorBrush(COLOR_HIGHLIGHT));
		MemDC.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
	}
	else
	{
		FillRect(MemDC.m_hDC, rClient, GetSysColorBrush(COLOR_BTNFACE));
		MemDC.SetTextColor(GetSysColor(COLOR_BTNTEXT));
	}

	int	iLeftOffset = 0;

	if (m_bUseIcon)
	{		
		MemDC.DrawState(
				CPoint(2, (rClient.Height() >> 1) - (m_sIcon.cy >> 1)),
				CSize(16, 16), m_hIcon, DST_ICON | DSS_NORMAL, reinterpret_cast<HBRUSH>(NULL) );
		iLeftOffset = m_sIcon.cx + 4;
	}

	MemDC.SetBkMode(TRANSPARENT);

	CRect	rText(0, 0, 0, 0);

	MemDC.DrawText(m_strText, rText, DT_CALCRECT | DT_SINGLELINE | DT_LEFT);

	CPoint	pt(rClient.left + 2 + iLeftOffset, (rClient.Height() >> 1) - (rText.Height() >> 1));
	CPoint	sz(rText.Width(), rText.Height());

	MemDC.DrawState( pt, sz, m_strText, DST_TEXT | (bEnabled ? DSS_NORMAL : DSS_DISABLED), 
		FALSE, m_strText.GetLength(), reinterpret_cast<HBRUSH>(NULL) );

	dc.BitBlt(0, 0, rClient.Width(), rClient.Height(), &MemDC, 0, 0, SRCCOPY);
	MemDC.SelectObject(pOldBMP);

	if (pOldFONT != NULL)
		MemDC.SelectObject(pOldFONT);
}
