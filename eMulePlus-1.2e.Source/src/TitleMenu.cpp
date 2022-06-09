//this file is part of eMule
//Copyright (C)2002-2007 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "TitleMenu.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CTitleMenu::CTitleMenu()
{
	HFONT hfont = CreatePopupMenuTitleFont();
	ASSERT(hfont);
	m_Font.Attach(hfont);
	clLeft=::GetSysColor(COLOR_ACTIVECAPTION);
	clRight=::GetSysColor(27);  //COLOR_GRADIENTACTIVECAPTION
	clText=::GetSysColor(COLOR_CAPTIONTEXT);
	
	hinst_msimg32 = LoadLibrary(_T("msimg32.dll"));
	m_bCanDoGradientFill = FALSE;
	
	if(hinst_msimg32)
	{
		m_bCanDoGradientFill = TRUE;
		dllfunc_GradientFill = ((LPFNDLLFUNC1) GetProcAddress( hinst_msimg32, "GradientFill" ));
	}
	bDrawEdge=false;
	flag_edge=BDR_SUNKENINNER;
}

CTitleMenu::~CTitleMenu()
{
	m_Font.DeleteObject();
	FreeLibrary( hinst_msimg32 );
}



/////////////////////////////////////////////////////////////////////////////
// CTitleMenu message handlers

HFONT CTitleMenu::CreatePopupMenuTitleFont()
{
	// start by getting the stock menu font
	HFONT hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	if (hfont)
	{ 
		LOGFONT lf; //get the complete LOGFONT describing this font
		if (GetObject(hfont, sizeof(LOGFONT), &lf))
		{
			lf.lfWeight = FW_BOLD;	// set the weight to bold
			// recreate this font with just the weight changed
			return ::CreateFontIndirect(&lf);
		}
	}
	return NULL;
}


//
// This function adds a title entry to a popup menu
//
void CTitleMenu::AddMenuTitle(CString strTitle)
{
	// insert an empty owner-draw item at top to serve as the title
	// note: item is not selectable (disabled) but not grayed
	m_strTitle = strTitle;
	InsertMenu(0, MF_BYPOSITION | MF_OWNERDRAW | MF_STRING | MF_DISABLED, 0);
}


void CTitleMenu::MeasureItem(LPMEASUREITEMSTRUCT mi)
{
	// get the screen dc to use for retrieving size information
	CDC dc;
	dc.Attach(::GetDC(HWND_DESKTOP));
	// select the title font
	HFONT hfontOld = (HFONT)SelectObject(dc.m_hDC, (HFONT)m_Font);
	// compute the size of the title
	CSize size = dc.GetTextExtent(m_strTitle);
	// deselect the title font
	::SelectObject(dc.m_hDC, hfontOld);
	// add in the left margin for the menu item
	size.cx += GetSystemMetrics(SM_CXMENUCHECK)+8;


	//Return the width and height
	//+ include space for border
	const int nBorderSize = 2;
	mi->itemWidth = size.cx + nBorderSize;
	mi->itemHeight = size.cy + nBorderSize;

	// cleanup
	::ReleaseDC(NULL, dc.Detach());
}


void CTitleMenu::DrawItem(LPDRAWITEMSTRUCT di)
{
	COLORREF crOldBk = ::SetBkColor(di->hDC, clLeft);

	if(m_bCanDoGradientFill&&(clLeft!=clRight))
	{
		TRIVERTEX rcVertex[2];
		di->rcItem.right--; // exclude this point, like FillRect does 
		di->rcItem.bottom--;
		rcVertex[0].x=di->rcItem.left;
		rcVertex[0].y=di->rcItem.top;
		rcVertex[0].Red=GetRValue(clLeft)<<8;	// color values from 0x0000 to 0xff00 !!!!
		rcVertex[0].Green=GetGValue(clLeft)<<8;
		rcVertex[0].Blue=GetBValue(clLeft)<<8;
		rcVertex[0].Alpha=0x0000;
		rcVertex[1].x=di->rcItem.right; 
		rcVertex[1].y=di->rcItem.bottom;
		rcVertex[1].Red=GetRValue(clRight)<<8;
		rcVertex[1].Green=GetGValue(clRight)<<8;
		rcVertex[1].Blue=GetBValue(clRight)<<8;
		rcVertex[1].Alpha=0;
		GRADIENT_RECT rect;
		rect.UpperLeft=0;
		rect.LowerRight=1;

		// fill the area
		GradientFill( di->hDC,rcVertex,2,&rect,1,GRADIENT_FILL_RECT_H);
	}
	else
	{
		::ExtTextOut(di->hDC, 0, 0, ETO_OPAQUE, &di->rcItem, NULL, 0, NULL);
	}
	if(bDrawEdge)
		::DrawEdge(di->hDC, &di->rcItem, flag_edge, BF_RECT);


	int modeOld = ::SetBkMode(di->hDC, TRANSPARENT);
	COLORREF crOld = ::SetTextColor(di->hDC, clText);
	// select font into the dc
	HFONT hfontOld = (HFONT)SelectObject(di->hDC, (HFONT)m_Font);

	// add the menu margin offset
	di->rcItem.left += GetSystemMetrics(SM_CXMENUCHECK) + 8;

	// draw the text left aligned and vertically centered
	::DrawText(di->hDC, m_strTitle, m_strTitle.GetLength(), &di->rcItem, DT_SINGLELINE|DT_VCENTER|DT_LEFT|DT_NOPREFIX|DT_NOCLIP);

	//Restore font and colors...
	::SelectObject(di->hDC, hfontOld);
	::SetBkMode(di->hDC, modeOld);
	::SetBkColor(di->hDC, crOldBk);
	::SetTextColor(di->hDC, crOld);
}


BOOL CTitleMenu::GradientFill(HDC hdc, PTRIVERTEX pVertex, DWORD dwNumVertex, PVOID pMesh, DWORD dwNumMesh, DWORD dwMode)
{
	ASSERT(m_bCanDoGradientFill); 
	return dllfunc_GradientFill(hdc,pVertex,dwNumVertex,pMesh,dwNumMesh,dwMode); 
}
