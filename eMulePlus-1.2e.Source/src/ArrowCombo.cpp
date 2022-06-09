/*********************************************************************
	ArrowCombo: a combo box with up & down arrows, CopyLeft by Cax2.
	Displays 2 different fonts in the same combo box line. 
	
	This code is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.

	Inspired by the FontPreviewCombo code by
	http://www.smalleranimals.com
	smallest@smalleranimals.com

	which is based, in part, on:
	"A WTL-based Font preview combo box", Ramon Smits
	http://www.codeproject.com/wtl/rsprevfontcmb.asp

**********************************************************************/

#include "stdafx.h"
#include "ArrowCombo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



static CArrowCombo *m_pComboBox = 0;
#define SPACE      4

/////////////////////////////////////////////////////////////////////////////
// CArrowCombo

CArrowCombo::CArrowCombo(){
	m_iFontHeight = 16;
}

CArrowCombo::~CArrowCombo(){
}

BEGIN_MESSAGE_MAP(CArrowCombo, CComboBox)
	ON_WM_DRAWITEM()
	ON_WM_MEASUREITEM()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CArrowCombo message handlers

void CArrowCombo::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	ASSERT(lpDIS->CtlType == ODT_COMBOBOX); 
	CRect rect = lpDIS->rcItem;
	CDC dc;
	dc.Attach(lpDIS->hDC);
	if (lpDIS->itemState & ODS_FOCUS) dc.DrawFocusRect(&rect);
	if (lpDIS->itemID == -1) return;
	int nIndexDC = dc.SaveDC();
	
	CBrush brush;

	if (lpDIS->itemState & ODS_SELECTED)
	{
		brush.CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
		dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
	} 
	else 
		brush.CreateSolidBrush(dc.GetBkColor());
	
	dc.SetBkMode(TRANSPARENT);
	dc.FillRect(&rect, &brush);

	CString itemText;
	GetLBText(lpDIS->itemID, itemText);
	bool bArrowUp = false;
	bool bArrowDouble = false;
	bool ShowArrow = (!_tcsstr(itemText,_T("[-]")));
	if (ShowArrow)
	{
		if (_tcsstr(itemText,_T("[^]")) != NULL)
			bArrowUp = true;
		else if (_tcsstr(itemText,_T("[^^]")) != NULL)
		{
			bArrowUp = true;
			bArrowDouble = true;
		}
		else if (_tcsstr(itemText,_T("[vv]")) != NULL)
			bArrowDouble = true;
	}
	itemText= itemText.SpanExcluding(_T("["));

	CSize sz = dc.GetTextExtent(itemText);
	int txtHeight = (rect.Height() - sz.cy) / 2;

	dc.SetTextColor((GetStyle() & WS_DISABLED)? (COLORREF)RGB(128,128,128):dc.GetTextColor());	//Cax2 - grey them out
	dc.TextOut(rect.left+SPACE, rect.top + txtHeight, itemText);
	if(ShowArrow)
	{
		int txtLen = sz.cx+SPACE;
		//marlett is (AFAIK) available with all windows distributions...
		CFont marlett, *pOldFont; 
		marlett.CreateFont(m_iFontHeight-2,0,0,0,FW_NORMAL,FALSE, FALSE, FALSE,DEFAULT_CHARSET ,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH, _T("Marlett"));
		pOldFont = dc.SelectObject(&marlett);
		sz = dc.GetTextExtent(_T("t"), 1);		//both t(^) & u (v) arrows are the same width...
		dc.SetTextColor((GetStyle() & WS_DISABLED)?(COLORREF)RGB(160,160,160):(COLORREF)RGB(128,128,128));
		dc.TextOut(rect.left + txtLen, rect.top + txtHeight, (bArrowUp) ? _T("t") : _T("u"), 1);
		if (bArrowDouble)
		{
			dc.SetTextColor((GetStyle() & WS_DISABLED)?(COLORREF)RGB(192,192,192):(COLORREF)RGB(255,255,255));
			dc.TextOut(rect.left+txtLen+SPACE-1, rect.top + txtHeight, (bArrowUp) ? _T("t") : _T("u"), 1);
			dc.SetTextColor((GetStyle() & WS_DISABLED)?(COLORREF)RGB(160,160,160):(COLORREF)RGB(128,128,128));
			dc.TextOut(rect.left+txtLen+SPACE, rect.top + txtHeight, (bArrowUp) ? _T("t") : _T("u"), 1);
		}
		dc.SelectObject(pOldFont);
	}
	dc.RestoreDC(nIndexDC);
	dc.Detach();
}
