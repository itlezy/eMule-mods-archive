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

#include "stdafx.h"
#include "Tabs.h"
#include "otherfunctions.h"

//IMPLEMENT_DYNAMIC(CTabs, _TAB_BASECLASS)
CTabs::CTabs()
{
}

CTabs::~CTabs()
{
}


BEGIN_MESSAGE_MAP(CTabs, _TAB_BASECLASS)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()


int CTabs::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (_TAB_BASECLASS::OnCreate(lpCreateStruct) == -1)
		return -1;

	ModifyStyle(0, TCS_OWNERDRAWFIXED);
#ifdef _UNICODE
//	Win9x: Explicitly set to Unicode to receive Unicode notifications.
	SendMessage(CCM_SETUNICODEFORMAT, TRUE);
#endif

	return 0;
}

void CTabs::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	CDC			dc;

	dc.Attach(lpDIS->hDC);
	DrawItem(&dc, lpDIS->itemID);
	dc.Detach();
}

void CTabs::DrawItem(CDC *pDC, int iItem)
{
	CRect		rItem, rClient;

	GetItemRect(iItem, rItem);
	GetClientRect(rClient);
#ifndef NEW_LOOK
	rItem.top = rClient.top;
	rItem.bottom = rClient.bottom + 1;
#endif

	CGdiObject* pOldFont = pDC->SelectStockObject(DEFAULT_GUI_FONT);

	TCHAR	acText[512];
	TCITEM	tcItem;

	tcItem.mask = TCIF_TEXT;
	tcItem.pszText = acText;
	tcItem.cchTextMax = ARRSIZE(acText);
	GetItem(iItem, &tcItem);

	if (GetCurSel() == iItem)
	{
#ifdef NEW_LOOK
		rItem.top = rClient.top;
		rItem.bottom = rClient.bottom + 1;
#endif
		pDC->FillSolidRect(rItem, GetSysColor(COLOR_BTNFACE));
		pDC->Draw3dRect(rItem, GetSysColor(COLOR_BTNHILIGHT), GetSysColor(COLOR_BTNSHADOW));
		pDC->SetTextColor(GetSysColor(COLOR_BTNTEXT));
		pDC->SetBkColor(GetSysColor(COLOR_BTNFACE));
	}
	else
	{
#ifndef NEW_LOOK
		CPen		cpPen, *pOldPen;

		cpPen.CreatePen(PS_SOLID, 0, LightenColor(GetRGBColorTabs(), -40));
		pOldPen = pDC->SelectObject(&cpPen);
#else
		CPen		cpPen, cpPen2, *pOldPen;

		cpPen2.CreatePen(PS_SOLID, 0, RGB(0, 0, 0));

		pOldPen = pDC->SelectObject(&cpPen);
//		rItem.top = rClient.top;
		rItem.bottom = rClient.bottom + 1;
//		pDC->FillSolidRect(&rItem,GetRGBColorTabs());
//		pDC->Draw3dRect(&rItem,GetRGBColorTabs(),GetSysColor(COLOR_BTNSHADOW));
		pDC->FillSolidRect(rItem, GetSysColor(COLOR_BTNFACE));
		pDC->Draw3dRect(rItem, GetSysColor(COLOR_BTNHILIGHT), GetSysColor(COLOR_BTNSHADOW));
#endif //NEW_LOOK
		if (GetCurSel() != iItem - 1)
		{
			pDC->MoveTo(rItem.left, rItem.top + 2);
			pDC->LineTo(rItem.left, rItem.bottom - 2);
		}
		if (GetCurSel() != iItem + 1)
		{
			pDC->MoveTo(rItem.right, rItem.top + 2);
			pDC->LineTo(rItem.right, rItem.bottom - 2);
		}
#ifdef NEW_LOOK
		pDC->MoveTo(rItem.left + 1, rItem.top);
		pDC->MoveTo(rItem.right - 1, rItem.top);
#endif

		pDC->SelectObject(pOldPen);

		pDC->SetTextColor(GetRGBColorGrayText());
#ifndef NEW_LOOK
		pDC->SetBkColor(GetRGBColorTabs());
#else
		pDC->SetBkColor(GetSysColor(COLOR_BTNFACE));
#endif
	}

	CRect		rText = rItem;

	rText.DeflateRect(2, 2, 2, 2);
	pDC->DrawText(acText, -1, rText, DT_NOPREFIX|DT_NOCLIP|DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);

	CWnd		*pSB = GetWindow(GW_CHILD);

	if (pSB != NULL)
	{
		CRect		rSB;

		pSB->GetWindowRect(rSB);
		ScreenToClient(&rSB);

		if (rSB.PtInRect(rItem.TopLeft()) ||
			rSB.PtInRect(rItem.BottomRight()) ||
			rSB.PtInRect(CPoint(rItem.right, rItem.top)) ||
			rSB.PtInRect(CPoint(rItem.left, rItem.bottom)) ||
			rItem.PtInRect(rSB.TopLeft()) ||
			rItem.PtInRect(rSB.BottomRight()) ||
			rItem.PtInRect(CPoint(rSB.right, rSB.top)) ||
			rItem.PtInRect(CPoint(rSB.left, rSB.bottom)))
		{
			pSB->Invalidate();
		}
	}

	pDC->SelectObject(pOldFont);
}

BOOL CTabs::OnEraseBkgnd(CDC* pDC)
{
	NOPRM(pDC);
	return FALSE;
}

void CTabs::OnPaint()
{
	CPaintDC		dc(this); // device context for painting
	CRect			rClient;

	GetClientRect(rClient);
#ifndef NEW_LOOK
	dc.FillSolidRect(rClient, GetRGBColorTabs());
#else
	dc.FillSolidRect(rClient, ::GetSysColor(COLOR_DESKTOP));
#endif

	int				iItemCount = GetItemCount();

	for(int i = 0; i < iItemCount; i++)
	{
		DrawItem(&dc, i);
	}
}

COLORREF CTabs::GetRGBColorTabs()
{
	int			iRed = GetRValue(GetRGBColorXP());
	int			iGreen = GetGValue(GetRGBColorXP());
	int			iBlue = GetBValue(GetRGBColorXP());
	int			iMax = 255 - max(max(iRed, iGreen), iBlue);
	
	iMax = iMax - (int)(iMax * 0.51);

	return RGB(iRed + iMax, iGreen + iMax, iBlue + iMax);
}

COLORREF CTabs::GetRGBColorXP()
{
	COLORREF		cr = GetSysColor(COLOR_3DFACE);

	return RGB(((3 * GetRValue(cr) + 240) / 4) + 1,
			   ((3 * GetGValue(cr) + 240) / 4) + 1,
			   ((3 * GetBValue(cr) + 240) / 4) + 1);
}

COLORREF CTabs::GetRGBColorGrayText()
{
	int			iRed = 0;
	int			iGreen = 0;
	int			iBlue = 0;

	iRed = iRed >> 1;
	iGreen = iGreen >> 1;
	iBlue = iBlue >> 1;

	int			iMax = 255 - max(max(iRed, iGreen), iBlue);

	iMax = iMax - (int)(iMax * 0.60);

	return RGB(iRed + iMax, iGreen + iMax, iBlue + iMax);
}
