// ResizableMinMax.cpp: implementation of the CResizableMinMax class.
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2000-2002 by Paolo Messina
// (http://www.geocities.com/ppescher - ppescher@yahoo.com)
//
// The contents of this file are subject to the Artistic License (the "License").
// You may not use this file except in compliance with the License. 
// You may obtain a copy of the License at:
// http://www.opensource.org/licenses/artistic-license.html
//
// If you find this code useful, credits would be nice!
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ResizableMinMax.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CResizableMinMax::CResizableMinMax()
{
	m_bUseMinTrack = FALSE;
	m_bUseMaxTrack = FALSE;
	m_bUseMaxRect = FALSE;
}

CResizableMinMax::~CResizableMinMax()
{

}

void CResizableMinMax::MinMaxInfo(LPMINMAXINFO lpMMI)
{
	if (m_bUseMinTrack)
		lpMMI->ptMinTrackSize = m_ptMinTrackSize;

	if (m_bUseMaxTrack)
		lpMMI->ptMaxTrackSize = m_ptMaxTrackSize;

	if (m_bUseMaxRect)
	{
		lpMMI->ptMaxPosition = m_ptMaxPos;
		lpMMI->ptMaxSize = m_ptMaxSize;
	}
}

void CResizableMinMax::ChainMinMaxInfo(LPMINMAXINFO lpMMI,
									   CWnd* pParentWnd, CWnd* pWnd)
{
	// get the extra size from child to parent
	CRect rectClient, rectWnd;
	if ((pParentWnd->GetStyle() & WS_CHILD) && pParentWnd->IsZoomed())
		pParentWnd->GetClientRect(rectWnd);
	else
		pParentWnd->GetWindowRect(rectWnd);
	pParentWnd->RepositionBars(0, 0xFFFF,
		AFX_IDW_PANE_FIRST, CWnd::reposQuery, rectClient);
	CSize sizeExtra = rectWnd.Size() - rectClient.Size();

	// ask the view for track size
	MINMAXINFO mmiChild = *lpMMI;
	pWnd->SendMessage(WM_GETMINMAXINFO, 0, (LPARAM)&mmiChild);
	mmiChild.ptMaxTrackSize = sizeExtra + mmiChild.ptMaxTrackSize;
	mmiChild.ptMinTrackSize = sizeExtra + mmiChild.ptMinTrackSize;

	// min size is the largest
	lpMMI->ptMinTrackSize.x = __max(lpMMI->ptMinTrackSize.x,
		mmiChild.ptMinTrackSize.x);
	lpMMI->ptMinTrackSize.y = __max(lpMMI->ptMinTrackSize.y,
		mmiChild.ptMinTrackSize.y);

	// max size is the shortest
	lpMMI->ptMaxTrackSize.x = __min(lpMMI->ptMaxTrackSize.x,
		mmiChild.ptMaxTrackSize.x);
	lpMMI->ptMaxTrackSize.y = __min(lpMMI->ptMaxTrackSize.y,
		mmiChild.ptMaxTrackSize.y);
}

void CResizableMinMax::SetMaximizedRect(const CRect& rc)
{
	m_bUseMaxRect = TRUE;

	m_ptMaxPos = rc.TopLeft();
	m_ptMaxSize.x = rc.Width();
	m_ptMaxSize.y = rc.Height();
}

void CResizableMinMax::ResetMaximizedRect()
{
	m_bUseMaxRect = FALSE;
}

void CResizableMinMax::SetMinTrackSize(const CSize& size)
{
	m_bUseMinTrack = TRUE;

	m_ptMinTrackSize.x = size.cx;
	m_ptMinTrackSize.y = size.cy;
}

void CResizableMinMax::ResetMinTrackSize()
{
	m_bUseMinTrack = FALSE;
}

void CResizableMinMax::SetMaxTrackSize(const CSize& size)
{
	m_bUseMaxTrack = TRUE;

	m_ptMaxTrackSize.x = size.cx;
	m_ptMaxTrackSize.y = size.cy;
}

void CResizableMinMax::ResetMaxTrackSize()
{
	m_bUseMaxTrack = FALSE;
}
