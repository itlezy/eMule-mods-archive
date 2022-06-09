//this file is part of eMule
//Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "emule.h"
#include "ClosableTabCtrl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(CClosableTabCtrl, CTabCtrl)
CClosableTabCtrl::CClosableTabCtrl()
{
	static const uint16 s_auIconResID[] = { IDI_CLOSE };

	m_pImgLst.Create(16, 16, ILC_COLOR8 | ILC_MASK, ARRSIZE(s_auIconResID), 0);
	m_pImgLst.SetBkColor(CLR_NONE);
	FillImgLstWith16x16Icons(&m_pImgLst, s_auIconResID, ARRSIZE(s_auIconResID));
}

CClosableTabCtrl::~CClosableTabCtrl()
{
}

BEGIN_MESSAGE_MAP(CClosableTabCtrl, CTabCtrl)
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()


void CClosableTabCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	int			iTabs = GetItemCount();
	CRect		rect;

	for (int i = 0; i < iTabs; i++)
	{
		GetItemRect(i, rect);
		rect.DeflateRect(rect.Width() - 16, 0, 0, 0);

		if (rect.PtInRect(point))
		{
			GetParent()->SendMessage(WM_CLOSETAB, (WPARAM)i);
			return; 
		}
	}
	
	CTabCtrl::OnLButtonUp(nFlags, point);
}

void CClosableTabCtrl::OnMButtonUp(UINT nFlags, CPoint point)
{
	int			iTabs = GetItemCount();
	CRect		rect;

	for (int i = 0; i < iTabs; i++)
	{
		GetItemRect(i, rect);
		if (rect.PtInRect(point))
		{
			GetParent()->SendMessage(WM_CLOSETAB, (WPARAM)i);
			return;
		}
	}

	CTabCtrl::OnMButtonUp(nFlags, point);
}

void CClosableTabCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	int			iTabs = GetItemCount();
	CRect		rect;

	for (int i = 0; i < iTabs; i++)
	{
		GetItemRect(i, rect);
		if (rect.PtInRect(point))
		{
			GetParent()->SendMessage(WM_TAB_PROPERTIES, (WPARAM)i);
			return;
		}
	}

	CTabCtrl::OnRButtonUp(nFlags, point);
}

void CClosableTabCtrl::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	CRect		rect = lpDIS->rcItem;
	IMAGEINFO	info;
	bool		bSelected = lpDIS->itemState & ODS_SELECTED;
	int			iTabIndex = lpDIS->itemID;

	if (iTabIndex < 0)
		return;

	TCHAR		szLabel[256];
	TC_ITEM		tci;

	tci.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_STATE;
	tci.pszText = szLabel;
	tci.cchTextMax = ARRSIZE(szLabel);
	tci.dwStateMask = TCIS_HIGHLIGHTED;
	if (!GetItem(iTabIndex, &tci))
		return;

	CDC			dc;

	dc.Attach(lpDIS->hDC);

	int		iCX, iOldBkMode = dc.SetBkMode(TRANSPARENT);
	LONG	lTop = rect.top;

	iCX = ::GetSystemMetrics(SM_CXEDGE);
	rect.DeflateRect(iCX, ::GetSystemMetrics(SM_CYEDGE), iCX, 0);

	if (!g_App.m_pMDlg->m_themeHelper.IsAppThemed())
		dc.FillSolidRect(&rect, GetSysColor(COLOR_BTNFACE));

//	Draw image
	if (tci.iImage >= 0)
	{
		// Get height of image 
		m_pImgLst.GetImageInfo(0, &info);

		CRect	ImageRect(info.rcImage), rIcon = rect;

		rIcon.right -= bSelected ? 6 : 4;
		rIcon.top += 2;
		rIcon.left = rIcon.right - ImageRect.Width();
		rect.right = rIcon.left - 3;

		m_pImgLst.Draw(&dc, 0, CPoint(rIcon.left, rIcon.top), ILD_TRANSPARENT);
	}

	COLORREF	crOldColor = dc.SetTextColor((tci.dwState & TCIS_HIGHLIGHTED) ? RGB(196, 0, 0) : GetSysColor(COLOR_BTNTEXT));
	UINT		dwFormat = DT_SINGLELINE | DT_BOTTOM | DT_CENTER | DT_NOPREFIX | DT_NOCLIP;

	if (bSelected)
	{
		rect.top = lTop;
		dwFormat = DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_NOPREFIX | DT_NOCLIP;
	}
	dc.DrawText(szLabel, -1, rect, dwFormat);
	dc.SetTextColor(crOldColor);
	dc.SetBkMode(iOldBkMode);
	dc.Detach();
}

void CClosableTabCtrl::PreSubclassWindow()
{
	CTabCtrl::PreSubclassWindow();
	SetImageList(&m_pImgLst);	//required only to automatically extend tab width for image size
	ModifyStyle(0, TCS_OWNERDRAWFIXED);
#ifdef _UNICODE
//	Win9x: Explicitly set to Unicode to receive Unicode notifications.
	SendMessage(CCM_SETUNICODEFORMAT, TRUE);
#endif
}
