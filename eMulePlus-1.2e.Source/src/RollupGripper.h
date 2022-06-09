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
#pragma once

#define USRMSG_GRIPPERMOVE	WM_USER + 2

#define RUP_GRIPPERWIDTH	60
#define RUP_GRIPPERHEIGHT	15

#define GRIPPER_STATE_NORMAL	0
#define GRIPPER_STATE_MOUSEOVER	1
#define GRIPPER_STATE_PRESSED	2

#define GRIPPER_ROWCOUNT		3


#undef  RUP_GRIPPERHEIGHT
#define RUP_GRIPPERHEIGHT	10
#undef  GRIPPER_ROWCOUNT
#define GRIPPER_ROWCOUNT	2


class CRollupGripper : public CWnd
{
	DECLARE_DYNAMIC(CRollupGripper)

public:
	CRollupGripper();
	virtual ~CRollupGripper();

	int GetLastMove() const		{ return m_iLastMove; }

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnCancelMode();

protected:
	CPoint m_cpLastPoint;
	CDC		MemDC;
	CBitmap	MemBMP;
	CBitmap	*pOldMemBMP;

	int		m_iState;
	int		m_iLastMove;

	bool	m_bMouseOver;
	bool	m_bLButtonDown;
	bool	m_bInit;

protected:
	void DrawDot(CDC *pDC, int x, int y, COLORREF hicolor, COLORREF locolor, bool invert = FALSE);

	DECLARE_MESSAGE_MAP()
};

// (p) 2002 by FoRcHa (a.k.a. NO)  [seppforcher38@hotmail.com]
