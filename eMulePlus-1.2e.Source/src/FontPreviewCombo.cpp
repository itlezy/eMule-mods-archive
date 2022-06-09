/*********************************************************************

   Copyright (C) 2002 Smaller Animals Software, Inc.

   This software is provided 'as-is', without any express or implied
   warranty.  In no event will the authors be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.

   3. This notice may not be removed or altered from any source distribution.

   http://www.smalleranimals.com
   smallest@smalleranimals.com

   --------

   This code is based, in part, on:
   "A WTL-based Font preview combo box", Ramon Smits
   http://www.codeproject.com/wtl/rsprevfontcmb.asp

**********************************************************************/

#include "stdafx.h"
#include "resource.h"
#include "FontPreviewCombo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static CFontPreviewCombo *m_pComboBox = 0;
#define SPACING      10
#define GLYPH_WIDTH  15

// set the "sample" text string, defaults to "abcdeABCDE"
#define FONT_PREVIEW_SAMPLE		_T("abcdeABCDE")

/////////////////////////////////////////////////////////////////////////////
// CFontPreviewCombo

CFontPreviewCombo::CFontPreviewCombo()
{
	m_iFontHeight = 16;
	m_iMaxNameWidth = 0;
	m_iMaxSampleWidth = 0;
	m_style = NAME_THEN_SAMPLE;
	m_clrSample = GetSysColor(COLOR_WINDOWTEXT);
	m_clrSample = RGB(60,0,0);
	m_img.Create(IDB_TTF_BMP, GLYPH_WIDTH, 1, RGB(255,255,255));
}

CFontPreviewCombo::~CFontPreviewCombo()
{
}


BEGIN_MESSAGE_MAP(CFontPreviewCombo, CComboBox)
	ON_WM_MEASUREITEM()
	ON_CONTROL_REFLECT(CBN_DROPDOWN, OnDropdown)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFontPreviewCombo message handlers


static BOOL CALLBACK EnumFontProc (LPLOGFONT lplf, LPTEXTMETRIC lptm, DWORD dwType, LPARAM lpData)	
{
	NOPRM(lptm);
	CFontPreviewCombo *pThis = reinterpret_cast<CFontPreviewCombo*>(lpData);
	int index = pThis->AddString(lplf->lfFaceName);
	ASSERT(index!=-1);

	pThis->SetItemData(index, dwType);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

void CFontPreviewCombo::Init()
{
	CClientDC dc(this);

	EnumFonts (dc, 0,(FONTENUMPROC) EnumFontProc,(LPARAM)this); //Enumerate font

	SetCurSel(0);
}

/////////////////////////////////////////////////////////////////////////////

void CFontPreviewCombo::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	ASSERT(lpDIS->CtlType == ODT_COMBOBOX);

	CRect rc = lpDIS->rcItem;

	CDC dc;
	dc.Attach(lpDIS->hDC);

	if (lpDIS->itemState & ODS_FOCUS)
		dc.DrawFocusRect(&rc);

	if (lpDIS->itemID == -1)
		return;

	int nIndexDC = dc.SaveDC();
	CBrush br;
	COLORREF clrSample = m_clrSample;

	if (lpDIS->itemState & ODS_SELECTED)
	{
		br.CreateSolidBrush(::GetSysColor(COLOR_HIGHLIGHT));
		dc.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
		clrSample = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
	}
	else
	{
		br.CreateSolidBrush(dc.GetBkColor());
	}

	dc.SetBkMode(TRANSPARENT);
	dc.FillRect(&rc, &br);

	// which one are we working on?
	CString csCurFontName;
	GetLBText(lpDIS->itemID, csCurFontName);

	// draw the cute TTF glyph
	DWORD dwData = GetItemData(lpDIS->itemID);
	if (dwData & TRUETYPE_FONTTYPE)
	{
		m_img.Draw(&dc, 0, CPoint(rc.left+5, rc.top+4), ILD_TRANSPARENT);
	}
	rc.left += GLYPH_WIDTH;

	int iOffsetX = SPACING;

	// i feel bad creating this font on each draw. but i can't think of a better way (other than creating ALL fonts at once and saving them - yuck
	CFont cf;
	if (m_style != NAME_GUI_FONT)
	{
		if (!cf.CreateFont(m_iFontHeight,0,0,0,FW_NORMAL,FALSE, FALSE, FALSE,DEFAULT_CHARSET ,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH, csCurFontName))
		{
			ASSERT(0);
			return;
		}
	}

	// draw the text
	CSize sz;
	int iPosY = 0;
	HFONT hf = NULL;
	switch (m_style)
	{
	case NAME_GUI_FONT:
		{
			// font name in GUI font
			sz = dc.GetTextExtent(csCurFontName);
			iPosY = (rc.Height() - sz.cy) / 2;
			dc.TextOut(rc.left+iOffsetX, rc.top + iPosY,csCurFontName);
		}
		break;
	case NAME_ONLY:
		{
			// font name in current font
			hf = (HFONT)dc.SelectObject(cf);
			sz = dc.GetTextExtent(csCurFontName);
			iPosY = (rc.Height() - sz.cy) / 2;
			dc.TextOut(rc.left+iOffsetX, rc.top + iPosY,csCurFontName);
			dc.SelectObject(hf);
		}
		break;
	case NAME_THEN_SAMPLE:
		{
			// font name in GUI font
			sz = dc.GetTextExtent(csCurFontName);
			iPosY = (rc.Height() - sz.cy) / 2;
			dc.TextOut(rc.left+iOffsetX, rc.top + iPosY, csCurFontName);

			// condense, for edit
			int iSep = m_iMaxNameWidth;
			if ((lpDIS->itemState & ODS_COMBOBOXEDIT) == ODS_COMBOBOXEDIT)
			{
				iSep = sz.cx;
			}

			// sample in current font
			hf = (HFONT)dc.SelectObject(cf);
			sz = dc.GetTextExtent(FONT_PREVIEW_SAMPLE, CSTRLEN(FONT_PREVIEW_SAMPLE));
			iPosY = (rc.Height() - sz.cy) / 2;
			COLORREF clr = dc.SetTextColor(clrSample);
			dc.TextOut(rc.left + iOffsetX + iSep + iOffsetX, rc.top + iPosY, FONT_PREVIEW_SAMPLE, CSTRLEN(FONT_PREVIEW_SAMPLE));
			dc.SetTextColor(clr);
			dc.SelectObject(hf);
		}
		break;
	case SAMPLE_THEN_NAME:
		{
			// sample in current font
			hf = (HFONT)dc.SelectObject(cf);
			sz = dc.GetTextExtent(FONT_PREVIEW_SAMPLE, CSTRLEN(FONT_PREVIEW_SAMPLE));
			iPosY = (rc.Height() - sz.cy) / 2;
			COLORREF clr = dc.SetTextColor(clrSample);
			dc.TextOut(rc.left+iOffsetX, rc.top + iPosY, FONT_PREVIEW_SAMPLE, CSTRLEN(FONT_PREVIEW_SAMPLE));
			dc.SetTextColor(clr);
			dc.SelectObject(hf);

			// condense, for edit
			int iSep = m_iMaxSampleWidth;
			if ((lpDIS->itemState & ODS_COMBOBOXEDIT) == ODS_COMBOBOXEDIT)
			{
				iSep = sz.cx;
			}

			// font name in GUI font
			sz = dc.GetTextExtent(csCurFontName);
			iPosY = (rc.Height() - sz.cy) / 2;
			dc.TextOut(rc.left + iOffsetX + iSep + iOffsetX, rc.top + iPosY, csCurFontName);
		}
		break;
	case SAMPLE_ONLY:
		{
			// sample in current font
			hf = (HFONT)dc.SelectObject(cf);
			sz = dc.GetTextExtent(FONT_PREVIEW_SAMPLE, CSTRLEN(FONT_PREVIEW_SAMPLE));
			iPosY = (rc.Height() - sz.cy) / 2;
			dc.TextOut(rc.left+iOffsetX, rc.top + iPosY, FONT_PREVIEW_SAMPLE, CSTRLEN(FONT_PREVIEW_SAMPLE));
			dc.SelectObject(hf);
		}
		break;
	}

	dc.RestoreDC(nIndexDC);
	dc.Detach();
}

/////////////////////////////////////////////////////////////////////////////

void CFontPreviewCombo::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	// ok, how big is this ?

	CString csFontName;
	GetLBText(lpMeasureItemStruct->itemID, csFontName);

	CFont cf;
	if (!cf.CreateFont(m_iFontHeight,0,0,0,FW_NORMAL,FALSE, FALSE, FALSE,DEFAULT_CHARSET ,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH, csFontName))
	{
		ASSERT(0);
		return;
	}

	LOGFONT lf;
	cf.GetLogFont(&lf);

	if ((m_style == NAME_ONLY) || (m_style == SAMPLE_ONLY) || (m_style == NAME_GUI_FONT))
	{
		m_iMaxNameWidth = 0;
		m_iMaxSampleWidth = 0;
	}
	else
	{
		CClientDC dc(this);

		// measure font name in GUI font
		HFONT hFont = ((HFONT)GetStockObject( DEFAULT_GUI_FONT ));
		HFONT hf = (HFONT)dc.SelectObject(hFont);
		CSize sz = dc.GetTextExtent(csFontName);
		m_iMaxNameWidth = max(m_iMaxNameWidth, sz.cx);
		dc.SelectObject(hf);

		// measure sample in cur font
		hf = (HFONT)dc.SelectObject(cf);
		if (hf)
		{
			sz = dc.GetTextExtent(FONT_PREVIEW_SAMPLE, CSTRLEN(FONT_PREVIEW_SAMPLE));
			m_iMaxSampleWidth = max(m_iMaxSampleWidth, sz.cx);
			dc.SelectObject(hf);
		}
	}

	lpMeasureItemStruct->itemHeight = lf.lfHeight + 4;
}

/////////////////////////////////////////////////////////////////////////////

void CFontPreviewCombo::OnDropdown()
{
	m_pComboBox = this;

	int nScrollWidth = ::GetSystemMetrics(SM_CXVSCROLL);
	int nWidth = nScrollWidth;
	nWidth += GLYPH_WIDTH;

	switch (m_style)
	{
		case NAME_GUI_FONT:
			nWidth += m_iMaxNameWidth;
			break;
		case NAME_ONLY:
			nWidth += m_iMaxNameWidth;
			break;
		case NAME_THEN_SAMPLE:
			nWidth += m_iMaxNameWidth;
			nWidth += m_iMaxSampleWidth;
			nWidth += SPACING * 2;
			break;
		case SAMPLE_THEN_NAME:
			nWidth += m_iMaxNameWidth;
			nWidth += m_iMaxSampleWidth;
			nWidth += SPACING * 2;
			break;
		case SAMPLE_ONLY:
			nWidth += m_iMaxSampleWidth;
			break;
	}

	SetDroppedWidth(nWidth);
}
