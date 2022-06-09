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
#include "IconStatic.h"
#include "ThemeHelperST.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIconStatic

CIconStatic::CIconStatic()
	: m_pTheme(NULL), m_uiIconID(0)
{
}

CIconStatic::~CIconStatic()
{
	if (m_MemBMP.GetSafeHandle() != NULL)
		m_MemBMP.DeleteObject();
}

BEGIN_MESSAGE_MAP(CIconStatic, CStatic)
	ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
void CIconStatic::Init(UINT uiIconID, CThemeHelperST *pTheme)
{
	m_uiIconID = uiIconID;
	m_pTheme = pTheme;

	Draw();
}

/////////////////////////////////////////////////////////////////////////////
void CIconStatic::SetText(const CString &strText)
{
	m_strText = strText;

	Draw();
}

/////////////////////////////////////////////////////////////////////////////
void CIconStatic::Draw()
{
//	If this function is called for the first time and we did not yet call 'SetWindowText', we take
//	the window label which is already specified for the window (the label which comes from the resource)
	CString	strText;

	GetWindowText(strText);
	SetWindowText(_T(""));
	if (!strText.IsEmpty() && m_strText.IsEmpty())
		m_strText = strText;

	CRect	rRect;

	GetClientRect(rRect);

	CDC		MemDC, *pDC = GetDC();
	CBitmap	*pOldBMP;

	MemDC.CreateCompatibleDC(pDC);

	CFont		*pOldFont = MemDC.SelectObject(GetFont());
	CRect		rCaption(0, 0, 0, 0);
	HTHEME		hTheme = NULL;
	LPOLESTR	oleTxt = NULL;
	int			iTxtLen = 0;

	if (m_pTheme != NULL && m_pTheme->IsThemeActive() && m_pTheme->IsAppThemed())
	{
		if ((hTheme = m_pTheme->OpenThemeData(GetSafeHwnd(), L"BUTTON")) != NULL)
		{
			USES_CONVERSION;

			oleTxt = CT2OLE(m_strText);
			iTxtLen = ocslen(oleTxt);
			m_pTheme->GetThemeTextExtent( hTheme, MemDC.m_hDC, BP_GROUPBOX, GBS_NORMAL, oleTxt, iTxtLen, 
				DT_SINGLELINE | DT_LEFT | DT_NOPREFIX, NULL, &rCaption );
		}
	}
	else
		MemDC.DrawText(m_strText, rCaption, DT_CALCRECT | DT_SINGLELINE | DT_LEFT | DT_NOPREFIX);

	if (rCaption.Height() < 16)
		rCaption.bottom = rCaption.top + 16;
	rCaption.right += 25;
	if (rRect.Width() >= 16 && rCaption.Width() > rRect.Width() - 16)
		rCaption.right = rCaption.left + rRect.Width() - 16;

	if (m_MemBMP.GetSafeHandle())
		m_MemBMP.DeleteObject();
	m_MemBMP.CreateCompatibleBitmap(pDC, rCaption.Width(), rCaption.Height());
	pOldBMP = MemDC.SelectObject(&m_MemBMP);

	MemDC.FillSolidRect(rCaption, GetSysColor(COLOR_BTNFACE));

	if (m_uiIconID != 0)
	{
		HICON	hIcon = (HICON)::LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(m_uiIconID), IMAGE_ICON, 16,16, 0);

		DrawState(MemDC.m_hDC, NULL, NULL, (LPARAM)hIcon, NULL, 3, 0, 16, 16, DST_ICON | DSS_NORMAL);
		DestroyIcon(hIcon);

		rCaption.left += 22;
	}
	
	if (hTheme != NULL)
	{
		m_pTheme->DrawThemeText( hTheme, MemDC.m_hDC, BP_GROUPBOX, GBS_NORMAL, oleTxt, iTxtLen, 
			DT_SINGLELINE | DT_LEFT | DT_NOPREFIX | DT_END_ELLIPSIS, NULL, &rCaption ); 
		m_pTheme->CloseThemeData(hTheme);
	}
	else
	{
		MemDC.SetTextColor(pDC->GetTextColor());
		MemDC.DrawText(m_strText, rCaption, DT_SINGLELINE | DT_LEFT | DT_NOPREFIX | DT_END_ELLIPSIS);
	}

	MemDC.SelectObject(pOldBMP);
	MemDC.SelectObject(pOldFont);

	if (m_wndPicture.m_hWnd == NULL)
		m_wndPicture.Create(NULL, WS_CHILD | WS_VISIBLE | SS_BITMAP, CRect(0, 0, 0, 0), this);
	m_wndPicture.SetWindowPos(NULL, rRect.left + 8, rRect.top, rCaption.Width() + 22, rCaption.Height(), SWP_SHOWWINDOW);
	m_wndPicture.SetBitmap(m_MemBMP);

	CWnd	*pParent = GetParent();

	if (pParent == NULL)
		pParent = GetDesktopWindow();

	CRect	r;

	GetWindowRect(r);
	r.bottom = r.top + 20;
	pParent->ScreenToClient(&r);
	pParent->RedrawWindow(r);

	ReleaseDC(pDC);
}

void CIconStatic::OnSysColorChange()
{
	CStatic::OnSysColorChange();
	if (m_uiIconID != 0)
		Draw();
}
