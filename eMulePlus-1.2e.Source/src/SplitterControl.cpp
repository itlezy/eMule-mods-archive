//this file is part of eMule
//Copyright (C)2002 Merkur ( devs@emule-project.net / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "stdafx.h"
#include "SplitterControl.h"
#include "opcodes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CSplitterControl

HCURSOR CSplitterControl::m_hcurMoveVert = NULL;
HCURSOR CSplitterControl::m_hcurMoveHorz = NULL;

BEGIN_MESSAGE_MAP(CSplitterControl, CStatic)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_CANCELMODE()
END_MESSAGE_MAP()

CSplitterControl::CSplitterControl()
{
	// Mouse is pressed down or not ?
	m_bIsPressed = FALSE;	

	// Min and Max range of the splitter.
	m_nMin = m_nMax = -1;
}

CSplitterControl::~CSplitterControl()
{
}

void CSplitterControl::Create(DWORD dwStyle, const CRect &rect, CWnd *pParent, UINT nID)
{
	CRect rc = rect;
	
	// Determine default type base on it's size.
	m_nType = (rc.Width() < rc.Height()) ? SPS_VERTICAL : SPS_HORIZONTAL;
	
	if (m_nType == SPS_VERTICAL)
		rc.right = rc.left + 5;
	else // SPS_HORIZONTAL
		rc.bottom = rc.top + 5;
	
	CStatic::Create(_T(""), dwStyle | SS_NOTIFY, rc, pParent, nID);
	
	if (!m_hcurMoveVert)
	{
		m_hcurMoveVert = ::LoadCursor(NULL, IDC_SIZEWE);
		m_hcurMoveHorz = ::LoadCursor(NULL, IDC_SIZENS);
	}
	
	// force the splitter not to be split
	SetRange(0, 0, -1);
}

void CSplitterControl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	RECT rcClient, rc;

	GetClientRect(&rcClient);
	rc.left = rcClient.left + 1;
	rc.top = rcClient.top + 1;
	rc.right = rcClient.right - 1;
	rc.bottom = rcClient.bottom - 1;
	dc.FillSolidRect(&rc, ::GetSysColor(COLOR_3DFACE));
	dc.Draw3dRect(&rcClient, GetSysColor(COLOR_BTNHIGHLIGHT), GetSysColor(COLOR_BTNSHADOW));
}

void CSplitterControl::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bIsPressed)
	{
		CWnd		*pParent = GetParent();
		CWindowDC	dc(NULL);

		DrawLine(&dc);
		
		CPoint pt = point;
		ClientToScreen(&pt);
		pParent->ScreenToClient(&pt);

		if (pt.x < m_nMin)
			pt.x = m_nMin;
		if (pt.y < m_nMin)
			pt.y = m_nMin;

		if (pt.x > m_nMax)
			pt.x = m_nMax;
		if (pt.y > m_nMax)
			pt.y = m_nMax;

		pParent->ClientToScreen(&pt);
		m_nX = pt.x;
		m_nY = pt.y;
		DrawLine(&dc);
	}
	CStatic::OnMouseMove(nFlags, point);
}

BOOL CSplitterControl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (nHitTest == HTCLIENT)
	{
		::SetCursor((m_nType == SPS_VERTICAL) ? m_hcurMoveVert : m_hcurMoveHorz);
		return 0;
	}
	else
		return CStatic::OnSetCursor(pWnd, nHitTest, message);
}

void CSplitterControl::OnLButtonDown(UINT nFlags, CPoint point)
{
	CStatic::OnLButtonDown(nFlags, point);
	
	m_bIsPressed = TRUE;
	SetCapture();
	CRect rcWnd;
	GetWindowRect(rcWnd);
	
	if (m_nType == SPS_VERTICAL)
		m_nSavePos = m_nX = rcWnd.left;
	else
		m_nSavePos = m_nY = rcWnd.top;

	CWindowDC dc(NULL);
	DrawLine(&dc);
}

void CSplitterControl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bIsPressed)
	{
		ReleaseCapture();
		ClientToScreen(&point);
		EraseTrace();

		CPoint pt(m_nX, m_nY);
		CWnd *pOwner = GetOwner();
		if (pOwner && IsWindow(pOwner->m_hWnd))
		{
			pOwner->ScreenToClient(&pt);
			MoveWindowTo(pt);

			SPC_NMHDR nmsp;
			nmsp.hdr.hwndFrom = m_hWnd;
			nmsp.hdr.idFrom = GetDlgCtrlID();
			nmsp.hdr.code = UM_SPN_SIZED;
			nmsp.delta = ((m_nType == SPS_VERTICAL) ? m_nX : m_nY) - m_nSavePos;
			pOwner->SendMessage(WM_NOTIFY, nmsp.hdr.idFrom, (LPARAM)&nmsp);
		}
	}

	CStatic::OnLButtonUp(nFlags, point);
}

void CSplitterControl::DrawLine(CDC *pDC)
{
	int nRop = pDC->SetROP2(R2_NOTXORPEN);

	CRect rcWnd;
	GetWindowRect(rcWnd);
	
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
	CPen *pOP = pDC->SelectObject(&pen);
	
	if (m_nType == SPS_VERTICAL)
	{
		pDC->MoveTo(m_nX - 1, rcWnd.top);
		pDC->LineTo(m_nX - 1, rcWnd.bottom);

		pDC->MoveTo(m_nX + 1, rcWnd.top);
		pDC->LineTo(m_nX + 1, rcWnd.bottom);
	}
	else // m_nType == SPS_HORIZONTAL
	{
		pDC->MoveTo(rcWnd.left, m_nY - 1);
		pDC->LineTo(rcWnd.right, m_nY - 1);
		
		pDC->MoveTo(rcWnd.left, m_nY + 1);
		pDC->LineTo(rcWnd.right, m_nY + 1);
	}
	pDC->SetROP2(nRop);
	pDC->SelectObject(pOP);
}

void CSplitterControl::MoveWindowTo(CPoint pt)
{
	CRect rc;
	GetWindowRect(rc);

	CWnd* pParent;
	pParent = GetParent();
	if (!pParent || !::IsWindow(pParent->m_hWnd))
		return;

	pParent->ScreenToClient(rc);
	if (m_nType == SPS_VERTICAL)
	{	
		int nMidX = (rc.left + rc.right) / 2;
		int dx = pt.x - nMidX;
		rc.OffsetRect(dx, 0);
	}
	else
	{	
		int nMidY = (rc.top + rc.bottom) / 2;
		int dy = pt.y - nMidY;
		rc.OffsetRect(0, dy);
	}
	MoveWindow(rc);
}

void CSplitterControl::ChangeWidth(CWnd *pParent, int iCtrlID, int iDx, DWORD dwFlag/*=CW_LEFTALIGN*/)
{
	if (pParent && ::IsWindow(pParent->m_hWnd))
	{
		CWnd	*pWnd = pParent->GetDlgItem(iCtrlID);
		CRect	rcWnd;

		pWnd->GetWindowRect(rcWnd);
		pParent->ScreenToClient(rcWnd);
		if (dwFlag == CW_LEFTALIGN)
			rcWnd.right += iDx;
		else if (dwFlag == CW_RIGHTALIGN)
			rcWnd.left -= iDx;
		pWnd->MoveWindow(rcWnd);
	}
}

void CSplitterControl::ChangeHeight(CWnd *pParent, int iCtrlID, int iDy, DWORD dwFlag/*=CW_TOPALIGN*/)
{
	if (pParent && ::IsWindow(pParent->m_hWnd))
	{
		CWnd	*pWnd = pParent->GetDlgItem(iCtrlID);
		CRect	rcWnd;

		pWnd->GetWindowRect(rcWnd);
		pParent->ScreenToClient(rcWnd);
		if (dwFlag == CW_TOPALIGN)
			rcWnd.bottom += iDy;
		else if (dwFlag == CW_BOTTOMALIGN)
			rcWnd.top -= iDy;
		pWnd->MoveWindow(rcWnd);
	}
	else ASSERT(0);
}

void CSplitterControl::ChangePos(CWnd *pParent, int iCtrlID, int iDx, int iDy)
{
	if (pParent && ::IsWindow(pParent->m_hWnd))
	{
		CWnd	*pWnd = pParent->GetDlgItem(iCtrlID);
		CRect	rcWnd;

		pWnd->GetWindowRect(rcWnd);
		pParent->ScreenToClient(rcWnd);
		rcWnd.OffsetRect(-iDx, iDy);
		pWnd->MoveWindow(rcWnd);
	}	
}

void CSplitterControl::SetRange(int nMin, int nMax)
{
	m_nMin = nMin;
	m_nMax = nMax;
}

// Set splitter range from (nRoot - nSubtraction) to (nRoot + nAddition)
// If (nRoot < 0)
//		nRoot =  <current position of the splitter>
void CSplitterControl::SetRange(int nSubtraction, int nAddition, int nRoot)
{
	if (nRoot < 0)
	{
		CRect rcWnd;
		GetWindowRect(rcWnd);
		GetParent()->ScreenToClient(rcWnd); // need to work in client coordinates
		if (m_nType == SPS_VERTICAL)
			nRoot = rcWnd.left + rcWnd.Width() / 2;
		else // if m_nType == SPS_HORIZONTAL
			nRoot = rcWnd.top + rcWnd.Height() / 2;
	}
	m_nMin = nRoot - nSubtraction;
	m_nMax = nRoot + nAddition;
}

void CSplitterControl::EraseTrace()
{
//	Undo previous drawing
	if (m_bIsPressed)
	{
		CWindowDC dc(NULL);

		DrawLine(&dc);
		m_bIsPressed = FALSE;
	}
}

void CSplitterControl::OnCancelMode()
{
	CWnd *pOwner = GetOwner();

	m_bIsPressed = FALSE;
//	Force redraw to erase last splitter marker
	if (pOwner && IsWindow(pOwner->m_hWnd))
	{
		SPC_NMHDR nmsp;

		nmsp.hdr.hwndFrom = m_hWnd;
		nmsp.hdr.idFrom = GetDlgCtrlID();
		nmsp.hdr.code = UM_SPN_SIZED;
		nmsp.delta = 0;
		pOwner->SendMessage(WM_NOTIFY, nmsp.hdr.idFrom, (LPARAM)&nmsp);
	}
	CWnd::OnCancelMode();
}

void CSplitterControl::CancelTracking()
{
	EraseTrace();
	ReleaseCapture();
}
