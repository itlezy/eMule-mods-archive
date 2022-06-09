//	This file is part of eMule Plus
//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//	as published by the Free Software Foundation; either
//	version 2 of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// CRollupCtrl & CRollupHeader
// (c) 2002 by FoRcHa (a.k.a. NO)  [seppforcher38@hotmail.com]
//
// I would appreciate a notification of any bugs or bug fixes to help the control grow.
///////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "emule.h"
#include "RollupGripper.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


extern COLORREF LightenColor(COLORREF crColor, int iDiv);

IMPLEMENT_DYNAMIC(CRollupGripper, CWnd)
CRollupGripper::CRollupGripper()
{
	m_iState = GRIPPER_STATE_NORMAL;
	m_bMouseOver = false;
	m_bLButtonDown = false;
	m_bInit = true;

	MemDC.m_hDC = NULL;
	MemBMP.m_hObject = NULL;
	pOldMemBMP = NULL;

	m_iLastMove = 0;
}

CRollupGripper::~CRollupGripper()
{
	if(MemDC.m_hDC)
		MemDC.SelectObject(pOldMemBMP);
}


BEGIN_MESSAGE_MAP(CRollupGripper, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_WM_CANCELMODE()
END_MESSAGE_MAP()


void CRollupGripper::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	if(m_bInit)
	{
		if(MemDC.m_hDC != NULL)
		{
			if(pOldMemBMP)
			{
				MemDC.SelectObject(pOldMemBMP);
#ifdef _DEBUG
				pOldMemBMP = NULL;
#endif
			}
			MemDC.DeleteDC();
		}
		MemDC.CreateCompatibleDC((CDC*)&dc);
		if(MemBMP.m_hObject != NULL)
			MemBMP.DeleteObject();
		MemBMP.CreateCompatibleBitmap((CDC*)&dc, RUP_GRIPPERWIDTH, RUP_GRIPPERHEIGHT);
		pOldMemBMP = MemDC.SelectObject(&MemBMP);

		m_bInit = false;
	}

	MemDC.FillSolidRect(0,0,RUP_GRIPPERWIDTH,RUP_GRIPPERHEIGHT,GetSysColor(COLOR_BTNFACE));

	switch(m_iState)
	{
		case GRIPPER_STATE_NORMAL:
		{
		//	COLORREF hicolor = GetSysColor(COLOR_3DSHADOW);
		//	COLORREF locolor = GetSysColor(COLOR_3DLIGHT);
			COLORREF hicolor = LightenColor(GetSysColor(COLOR_BTNFACE), -0x50);
			COLORREF locolor = LightenColor(GetSysColor(COLOR_BTNFACE),  0x10);

			int yp = 0;
			for(int yc = 0; yc < GRIPPER_ROWCOUNT; yc++, yp+=5)
			{
				int xp = 0;
				for(int xc = 0; xc < 12; xc++, xp+=5)
					DrawDot(&MemDC, xp, yp, locolor, hicolor);
			}
			break;
		}
		case GRIPPER_STATE_MOUSEOVER:
		{
		//	COLORREF hicolor = GetSysColor(COLOR_3DDKSHADOW);
		//	COLORREF locolor = GetSysColor(COLOR_3DHILIGHT);
			COLORREF hicolor = LightenColor(GetSysColor(COLOR_BTNFACE), -0x80);
			COLORREF locolor = LightenColor(GetSysColor(COLOR_BTNFACE),  0x80);

			int yp = 0;
			for(int yc = 0; yc < GRIPPER_ROWCOUNT; yc++, yp+=5)
			{
				int xp = 0;
				for(int xc = 0; xc < 12; xc++, xp+=5)
					DrawDot(&MemDC, xp, yp, locolor, hicolor);
			}
			break;
		}
		case GRIPPER_STATE_PRESSED: //pressed 
		{
			COLORREF hicolor = LightenColor(GetSysColor(COLOR_BTNFACE), -0x50);
			COLORREF locolor = LightenColor(GetSysColor(COLOR_BTNFACE),  0x80);

			int yp = 0;
			for(int yc = 0; yc < GRIPPER_ROWCOUNT; yc++, yp+=5)
			{
				int xp = 0;
				for(int xc = 0; xc < 12; xc++, xp+=5)
					DrawDot(&MemDC, xp, yp, hicolor, locolor);
			}
			break;
		}
		default: break;
	}

	dc.BitBlt(0, 0, RUP_GRIPPERWIDTH, RUP_GRIPPERHEIGHT, &MemDC, 0, 0, SRCCOPY);
}

void CRollupGripper::DrawDot(CDC *pDC, int x, int y, COLORREF hicolor, COLORREF locolor, bool invert)
{
	pDC->SetPixel(x,   y,   hicolor);
	pDC->SetPixel(x+1, y,   hicolor);
	pDC->SetPixel(x,   y+1, hicolor);
	pDC->SetPixel(x+1, y+1, invert ? locolor : hicolor);
	pDC->SetPixel(x+2, y+1, locolor);
	pDC->SetPixel(x+1, y+2, locolor);
	pDC->SetPixel(x+2, y+2, locolor);
}

BOOL CRollupGripper::OnEraseBkgnd(CDC* pDC)
{
	NOPRM(pDC);
	return TRUE;
}

void CRollupGripper::OnLButtonDown(UINT nFlags, CPoint point)
{
	CWnd* pParent = GetParent();
	if(!pParent) pParent = GetDesktopWindow();

	ClientToScreen(&point);	
	pParent->ScreenToClient(&point);

	m_cpLastPoint = point;
	m_bLButtonDown = true;
	
	if(m_iState < GRIPPER_STATE_PRESSED)
	{
		m_iState = GRIPPER_STATE_PRESSED;
		Invalidate();
	}

	if(!m_bMouseOver)
		SetCapture();

	CWnd::OnLButtonDown(nFlags, point);
}

void CRollupGripper::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bLButtonDown = false;

	if(!m_bMouseOver)
	{
		m_iState = GRIPPER_STATE_NORMAL;
		ReleaseCapture();
		Invalidate();
	}
	else
	{
		m_iState = GRIPPER_STATE_MOUSEOVER;
		Invalidate();
	}

	CWnd::OnLButtonUp(nFlags, point);
}

void CRollupGripper::OnMouseMove(UINT nFlags, CPoint point)
{
	CRect rClientRect;
	GetClientRect(&rClientRect);

	if(point.x >= rClientRect.left && point.x <= rClientRect.right &&
		point.y >= rClientRect.top && point.y <= rClientRect.bottom)
	{
		if(!m_bMouseOver)
		{
			m_bMouseOver = true;
			SetCapture();
			if(m_iState < GRIPPER_STATE_MOUSEOVER)
			{
				m_iState = GRIPPER_STATE_MOUSEOVER;
				Invalidate();
			}
		}
	}
	else
	{
		m_bMouseOver = false;

		if(!m_bLButtonDown)
		{
			m_iState = GRIPPER_STATE_NORMAL;
			ReleaseCapture();
			Invalidate();
		}
	}

	if(m_bLButtonDown)
	{
		CWnd* pParent = GetParent();
		if(!pParent) pParent = GetDesktopWindow();

		CRect ParentRect;							// Parent client area (Parent coords)
		pParent->GetClientRect(ParentRect);

		ClientToScreen(&point);						// Convert point to parent coords
		pParent->ScreenToClient(&point);

		CRect ButtonRect;							// Button Dimensions (Parent coords)
		GetWindowRect(ButtonRect);
		pParent->ScreenToClient(ButtonRect);

		if(point != m_cpLastPoint)
		{
			m_iLastMove = m_cpLastPoint.y - point.y;

			if(m_iLastMove != 0)
				GetParent()->SendMessage(WM_COMMAND, USRMSG_GRIPPERMOVE, (long)m_hWnd);

			m_cpLastPoint = point;
		}
	}
	
	CWnd::OnMouseMove(nFlags, point);
}

void CRollupGripper::OnCancelMode()
{
	m_bLButtonDown = false;
	m_bMouseOver = false;
	m_iState = GRIPPER_STATE_NORMAL;
	CWnd::OnCancelMode();
}
