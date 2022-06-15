//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "ToolTipCtrlX.h"
#include "OtherFunctions.h"
#include "emule.h"
#include "log.h"
#include "VisualStylesXP.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DFLT_DRAWTEXT_FLAGS	(DT_NOPREFIX | DT_END_ELLIPSIS)


IMPLEMENT_DYNAMIC(CToolTipCtrlX, CToolTipCtrl)

BEGIN_MESSAGE_MAP(CToolTipCtrlX, CToolTipCtrl)
	ON_WM_SYSCOLORCHANGE()
	ON_WM_SETTINGCHANGE()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnNMCustomDraw)
	ON_NOTIFY_REFLECT(NM_THEMECHANGED, OnNMThemeChanged)
END_MESSAGE_MAP()

CToolTipCtrlX::CToolTipCtrlX()
{
	memset(&m_lfNormal, 0, sizeof(m_lfNormal));
	m_bCol1Bold = true;
	m_bShowFileIcon = false;
	ResetSystemMetrics();
	m_dwCol1DrawTextFlags = DFLT_DRAWTEXT_FLAGS | DT_LEFT;
	m_dwCol2DrawTextFlags = DFLT_DRAWTEXT_FLAGS | DT_LEFT;
}

CToolTipCtrlX::~CToolTipCtrlX()
{
}

void CToolTipCtrlX::SetCol1DrawTextFlags(DWORD dwFlags)
{
	m_dwCol1DrawTextFlags = DFLT_DRAWTEXT_FLAGS | dwFlags;
}

void CToolTipCtrlX::SetCol2DrawTextFlags(DWORD dwFlags)
{
	m_dwCol2DrawTextFlags = DFLT_DRAWTEXT_FLAGS | dwFlags;
}

void CToolTipCtrlX::ResetSystemMetrics()
{
	m_fontBold.DeleteObject();
	m_fontNormal.DeleteObject();

	memset(&m_lfNormal, 0, sizeof(m_lfNormal));
    NONCLIENTMETRICS ncm;
    ncm.cbSize = sizeof(NONCLIENTMETRICS);
	if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0))
		memcpy(&m_lfNormal, &ncm.lfStatusFont, sizeof(m_lfNormal));

	m_crTooltipBkColor = GetSysColor(COLOR_INFOBK);
	m_crTooltipTextColor = GetSysColor(COLOR_INFOTEXT);
	m_rcScreen.left = 0;
	m_rcScreen.top = 0;
	m_rcScreen.right = GetSystemMetrics(SM_CXSCREEN);
	m_rcScreen.bottom = GetSystemMetrics(SM_CYSCREEN);
	m_iScreenWidth4 = m_rcScreen.Width() / 4;
}

void CToolTipCtrlX::OnNMThemeChanged(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	ResetSystemMetrics();
	*pResult = 0;
}

void CToolTipCtrlX::OnSysColorChange()
{
	ResetSystemMetrics();
	CToolTipCtrl::OnSysColorChange();
}

void CToolTipCtrlX::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	ResetSystemMetrics();
	CToolTipCtrl::OnSettingChange(uFlags, lpszSection);
}

void CToolTipCtrlX::OnNMCustomDraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTTCUSTOMDRAW pNMCD = reinterpret_cast<LPNMTTCUSTOMDRAW>(pNMHDR);
	if (pNMCD->nmcd.dwDrawStage == CDDS_PREPAINT)
	{
		CWnd* pwnd = CWnd::FromHandle(pNMCD->nmcd.hdr.hwndFrom);
		CDC* pdc = CDC::FromHandle(pNMCD->nmcd.hdc);

		// Windows Vista (General)
		// -----------------------
		// *Need* to use (some aspects) of the 'TOOLTIP' theme to get typical Vista tooltips.
		//
		// Windows Vista *without* SP1
		// ---------------------------
		// The Vista 'TOOLTIP' theme offers a bold version of the standard tooltip font via 
		// the TTP_STANDARDTITLE part id. Furthermore TTP_STANDARDTITLE is the same font as
		// the standard tooltip font (TTP_STANDARD). So, the 'TOOLTIP' theme can get used
		// thoroughly.
		//
		// Windows Vista *with* SP1
		// ------------------------
		// The Vista SP1(!) 'TOOLTIP' theme does though *not* offer a bold font. Keep
		// in mind that TTP_STANDARDTITLE does not return a bold font. Keep also in mind
		// that TTP_STANDARDTITLE is even a *different* font than TTP_STANDARD!
		// Which means, that TTP_STANDARDTITLE should *not* be used within the same line
		// as TTP_STANDARD. It just looks weird. TTP_STANDARDTITLE could be used for a
		// single line (the title line), but it would though not give us a bold font.
		// So, actually we can use the Vista 'TOOLTIP' theme only for getting the proper
		// tooltip background.
		//
		// Windows XP
		// ----------
		// Can *not* use the 'TOOLTIP' theme at all because it would give us only a (non-bold)
		// black font on a white tooltip window background. Seems that the 'TOOLTIP' theme under 
		// WinXP is just using the default Window values (black+white) and does not
		// use any of the tooltip specific Window metrics...
		//
		bool bUseEmbeddedThemeFonts = false;
		HTHEME hTheme = NULL;
		if (theApp.IsVistaThemeActive()) {
			hTheme = g_xpStyle.OpenThemeData(*pwnd, L"TOOLTIP");
			// Using the theme's fonts works only under Vista without SP1. When SP1
			// is installed the fonts which are used for TTP_STANDARDTITLE and TTP_STANDARD
			// do no longer match (should not get displayed within the same text line).
			if (hTheme) bUseEmbeddedThemeFonts = true;
		}

		CString strText;
		pwnd->GetWindowText(strText);

		CRect rcWnd;
		pwnd->GetWindowRect(&rcWnd);

		CFont* pOldDCFont = NULL;
		if (hTheme == NULL || !bUseEmbeddedThemeFonts)
		{
			// If we have a theme, we try to query the correct fonts from it
			if (m_fontNormal.m_hObject == NULL && hTheme) {
				// Windows Vista Ultimate, 6.0.6001 SP1 Build 6001, English with German Language Pack
				// ----------------------------------------------------------------------------------
				// TTP_STANDARD, TTSS_NORMAL
				//  Height   -12
				//  Weight   400
				//  CharSet  1
				//  Quality  5
				//  FaceName "Segoe UI"
				//
				// TTP_STANDARDTITLE, TTSS_NORMAL
				//  Height   -12
				//  Weight   400
				//  CharSet  1
				//  Quality  5
				//  FaceName "Segoe UI Fett" (!!!) Note: "Fett" (that is German !?)
				//
				// That font name ("Segoe UI *Fett*") looks quite suspicious. I would not be surprised
				// if that eventually leads to a problem within the font mapper which may not be capable
				// of finding the (english) version of that font and thus falls back to the standard
				// system font. At least this would explain why the TTP_STANDARD and TTP_STANDARDTITLE
				// fonts are *different* on that particular Windows Vista system.
				LOGFONT lf = {0};
				if (g_xpStyle.GetThemeFont(hTheme, pdc->m_hDC, TTP_STANDARD, TTSS_NORMAL, TMT_FONT, &lf) == S_OK) {
					VERIFY( m_fontNormal.CreateFontIndirect(&lf) );

					if (m_bCol1Bold && m_fontBold.m_hObject == NULL) {
						memset(&lf, 0, sizeof(lf));
						if (g_xpStyle.GetThemeFont(hTheme, pdc->m_hDC, TTP_STANDARDTITLE, TTSS_NORMAL, TMT_FONT, &lf) == S_OK)
							VERIFY( m_fontBold.CreateFontIndirect(&lf) );
					}

					// Get the tooltip font color
					COLORREF crText;
					if (g_xpStyle.GetThemeColor(hTheme, TTP_STANDARD, TTSS_NORMAL, TMT_TEXTCOLOR, &crText) == S_OK)
						m_crTooltipTextColor = crText;
				}
			}

			// If needed, create the standard tooltip font which was queried from the system metrics
			if (m_fontNormal.m_hObject == NULL && m_lfNormal.lfHeight != 0)
				VERIFY( m_fontNormal.CreateFontIndirect(&m_lfNormal) );

			// Select the tooltip font
			if (m_fontNormal.m_hObject != NULL) {
				pOldDCFont = pdc->SelectObject(&m_fontNormal);

				// If needed, create the bold version of the tooltip font by deriving it from the standard font
				if (m_bCol1Bold && m_fontBold.m_hObject == NULL) {
					LOGFONT lf;
					m_fontNormal.GetLogFont(&lf);
					lf.lfWeight = FW_BOLD;
					VERIFY( m_fontBold.CreateFontIndirect(&lf) );
				}
			}

			// Set the font color (queried from system metrics or queried from theme)
			pdc->SetTextColor(m_crTooltipTextColor);
		}

		// Auto-format the text only if explicitly requested. Otherwise we would also format
		// single line tooltips for regular list items, and if those list items contain ':'
		// characters they would be shown partially in bold. For performance reasons, the
		// auto-format is to be requested by appending the TOOLTIP_AUTOFORMAT_SUFFIX_CH 
		// character. Appending, because we can remove that character efficiently without
		// re-allocating the entire string.
		bool bAutoFormatText = strText.GetLength() > 0 && strText[strText.GetLength() - 1] == TOOLTIP_AUTOFORMAT_SUFFIX_CH;
		if (bAutoFormatText)
			strText.Truncate(strText.GetLength() - 1); // truncate the TOOLTIP_AUTOFORMAT_SUFFIX_CH char by setting it to NUL
		bool bShowFileIcon = m_bShowFileIcon && bAutoFormatText;
		if (bShowFileIcon) {
			int iPosNL = strText.Find(_T('\n'));
			if (iPosNL > 0) {
				int iPosColon = strText.Find(_T(':'));
				if (iPosColon < iPosNL)
					bShowFileIcon = false; // 1st line does not contain a filename
			}
		}
		int iTextHeight = 0;
		int iMaxCol1Width = 0;
		int iMaxCol2Width = 0;
		int iMaxSingleLineWidth = 0;
		CSize sizText(0);
		int iPos = 0;
		int iCaptionEnd = bShowFileIcon ? max(strText.Find(_T("\n<br_head>\n")), 0) : 0; // special tooltip with file icon
		int iCaptionHeight = 0;
		int iIconMinYBorder = bShowFileIcon ? 3 : 0;
		int iIconWidth = bShowFileIcon ? theApp.GetBigSytemIconSize().cx : 0;
		int iIconHeight = bShowFileIcon ? theApp.GetBigSytemIconSize().cy : 0;
		int iIconDrawingWidth = bShowFileIcon ? (iIconWidth + 9) : 0;
		while (iPos != -1)
		{
			CString strLine = GetNextString(strText, _T('\n'), iPos);
			int iColon = bAutoFormatText ? strLine.Find(_T(':')) : -1;
			if (iColon != -1) {
				CSize siz;
				if (hTheme && bUseEmbeddedThemeFonts) {
					CRect rcExtent;
					CRect rcBounding(0, 0, 32767, 32767);
					g_xpStyle.GetThemeTextExtent(hTheme, *pdc, m_bCol1Bold ? TTP_STANDARDTITLE : TTP_STANDARD, TTSS_NORMAL, strLine, iColon + 1, m_dwCol1DrawTextFlags, &rcBounding, &rcExtent);
					siz.cx = rcExtent.Width();
					siz.cy = rcExtent.Height();
				}
				else {
					CFont* pOldFont = m_bCol1Bold ? pdc->SelectObject(&m_fontBold) : NULL;
					siz = pdc->GetTextExtent(strLine, iColon + 1);
					if (pOldFont)
						pdc->SelectObject(pOldFont);
				}
				iMaxCol1Width = max(iMaxCol1Width, siz.cx + ((bShowFileIcon && iPos <= iCaptionEnd + strLine.GetLength()) ? iIconDrawingWidth : 0));
				iTextHeight = siz.cy + 1; // update height with 'col1' string, because 'col2' string might be empty and therefore has no height
				if (iPos <= iCaptionEnd)
					iCaptionHeight += siz.cy + 1;
				else
					sizText.cy += siz.cy + 1;

				LPCTSTR pszCol2 = (LPCTSTR)strLine + iColon + 1;
				while (_istspace((_TUCHAR)*pszCol2))
					pszCol2++;
				if (*pszCol2 != _T('\0')) {
					if (hTheme && bUseEmbeddedThemeFonts) {
						CRect rcExtent;
						CRect rcBounding(0, 0, 32767, 32767);
						g_xpStyle.GetThemeTextExtent(hTheme, *pdc, TTP_STANDARD, TTSS_NORMAL, pszCol2, ((LPCTSTR)strLine + strLine.GetLength()) - pszCol2, m_dwCol2DrawTextFlags, &rcBounding, &rcExtent);
						siz.cx = rcExtent.Width();
						siz.cy = rcExtent.Height();
					}
					else {
						siz = pdc->GetTextExtent(pszCol2, ((LPCTSTR)strLine + strLine.GetLength()) - pszCol2);
					}
					iMaxCol2Width = max(iMaxCol2Width, siz.cx);
				}
			}
			else if (bShowFileIcon && iPos <= iCaptionEnd && iPos == strLine.GetLength() + 1){
				// file name, printed bold on top without any tabbing or desc
				CSize siz;
				if (hTheme && bUseEmbeddedThemeFonts) {
					CRect rcExtent;
					CRect rcBounding(0, 0, 32767, 32767);
					g_xpStyle.GetThemeTextExtent(hTheme, *pdc, m_bCol1Bold ? TTP_STANDARDTITLE : TTP_STANDARD, TTSS_NORMAL, strLine, strLine.GetLength(), m_dwCol1DrawTextFlags, &rcBounding, &rcExtent);
					siz.cx = rcExtent.Width();
					siz.cy = rcExtent.Height();
				}
				else {
					CFont* pOldFont = m_bCol1Bold ? pdc->SelectObject(&m_fontBold) : NULL;
					siz = pdc->GetTextExtent(strLine);
					if (pOldFont)
						pdc->SelectObject(pOldFont);
				}
				iMaxSingleLineWidth = max(iMaxSingleLineWidth, siz.cx + iIconDrawingWidth);
				iCaptionHeight += siz.cy + 1;
			}
			else if (!strLine.IsEmpty() && strLine.Compare(_T("<br>")) != 0 && strLine.Compare(_T("<br_head>")) != 0) {
				CSize siz;
				if (hTheme && bUseEmbeddedThemeFonts) {
					CRect rcExtent;
					CRect rcBounding(0, 0, 32767, 32767);
					g_xpStyle.GetThemeTextExtent(hTheme, *pdc, TTP_STANDARD, TTSS_NORMAL, strLine, strLine.GetLength(), m_dwCol2DrawTextFlags, &rcBounding, &rcExtent);
					siz.cx = rcExtent.Width();
					siz.cy = rcExtent.Height();
				}
				else {
					siz = pdc->GetTextExtent(strLine);
				}
				iMaxSingleLineWidth = max(iMaxSingleLineWidth, siz.cx + ((bShowFileIcon && iPos <= iCaptionEnd) ? iIconDrawingWidth : 0));
				if (bShowFileIcon && iPos <= iCaptionEnd + strLine.GetLength())
					iCaptionHeight += siz.cy + 1;
				else
					sizText.cy += siz.cy + 1;
			}
			else{
				CSize siz;
				if (hTheme && bUseEmbeddedThemeFonts) {
					CRect rcExtent;
					CRect rcBounding(0, 0, 32767, 32767);
					g_xpStyle.GetThemeTextExtent(hTheme, *pdc, TTP_STANDARD, TTSS_NORMAL, _T(" "), 1, m_dwCol2DrawTextFlags, &rcBounding, &rcExtent);
					siz.cx = rcExtent.Width();
					siz.cy = rcExtent.Height();
				}
				else {
					// TODO: Would need to use 'GetTabbedTextExtent' here, but do we actually use 'tabbed' text here at all ??
					siz = pdc->GetTextExtent(_T(" "), 1);
				}
				sizText.cy += siz.cy + 1;
			}
		}
		if (bShowFileIcon && iCaptionEnd > 0)
			iCaptionHeight = max(iCaptionHeight, theApp.GetBigSytemIconSize().cy + (2*iIconMinYBorder));
		sizText.cy += iCaptionHeight;
		if (hTheme && theApp.m_ullComCtrlVer >= MAKEDLLVERULL(6,16,0,0))
			sizText.cy += 2; // extra bottom margin for Vista/Theme

		iMaxCol1Width = min(m_iScreenWidth4, iMaxCol1Width);
		iMaxCol2Width = min(m_iScreenWidth4*2, iMaxCol2Width);

		const int iMiddleMargin = 6;
		iMaxSingleLineWidth = max(iMaxSingleLineWidth, iMaxCol1Width + iMiddleMargin + iMaxCol2Width);
		sizText.cx = iMaxSingleLineWidth;
	
		CRect rcNewSize(rcWnd.left, rcWnd.top, rcWnd.left + sizText.cx, rcWnd.top + sizText.cy);
		AdjustRect(&rcNewSize);		
		rcWnd.right = rcWnd.left + rcNewSize.Width();
		rcWnd.bottom = rcWnd.top + rcNewSize.Height();

		if (rcWnd.left >= m_rcScreen.left) {
			if (rcWnd.right > m_rcScreen.right && rcWnd.Width() <= m_rcScreen.Width())
				rcWnd.OffsetRect(-(rcWnd.right - m_rcScreen.right), 0);
		}
		if (rcWnd.top >= m_rcScreen.top) {
			if (rcWnd.bottom > m_rcScreen.bottom && rcWnd.Height() <= m_rcScreen.Height())
				rcWnd.OffsetRect(0, -(rcWnd.bottom - m_rcScreen.bottom));
		}
		CPoint ptText(pNMCD->nmcd.rc.left, pNMCD->nmcd.rc.top);

		pwnd->MoveWindow(&rcWnd);
		pwnd->ScreenToClient(&rcWnd);
		pNMCD->nmcd.rc.right = rcWnd.right;
		pNMCD->nmcd.rc.bottom = rcWnd.bottom;
		if (hTheme) {
			int iPartId = TTP_STANDARD;
			int iStateId = TTSS_NORMAL;
			if (g_xpStyle.IsThemeBackgroundPartiallyTransparent(hTheme, iPartId, iStateId))
				g_xpStyle.DrawThemeParentBackground(m_hWnd, pdc->m_hDC, &rcWnd);
			g_xpStyle.DrawThemeBackground(hTheme, pdc->m_hDC, iPartId, iStateId, &rcWnd, NULL);
		}
		else {
			pdc->FillSolidRect(&rcWnd, m_crTooltipBkColor);
			// Vista: Need to draw the window border explicitly !?
			if (theApp.m_ullComCtrlVer >= MAKEDLLVERULL(6,16,0,0)) {
				CPen pen;
				pen.CreatePen(0, 1, m_crTooltipTextColor);
				CPen *pOP = pdc->SelectObject(&pen);
				pdc->MoveTo(rcWnd.left, rcWnd.top);
				pdc->LineTo(rcWnd.right - 1, rcWnd.top);
				pdc->LineTo(rcWnd.right - 1, rcWnd.bottom - 1);
				pdc->LineTo(rcWnd.left, rcWnd.bottom - 1);
				pdc->LineTo(rcWnd.left, rcWnd.top);
				pdc->SelectObject(pOP);
				pen.DeleteObject();
			}
		}

		int iOldBkMode = 0;
		if (hTheme && !bUseEmbeddedThemeFonts)
			iOldBkMode = pdc->SetBkMode(TRANSPARENT);

		iPos = 0;
		while (iPos != -1)
		{
			CString strLine = GetNextString(strText, _T('\n'), iPos);
			int iColon = bAutoFormatText ? strLine.Find(_T(':')) : -1;
			CRect rcDT;
			if (!bShowFileIcon || (unsigned)iPos > (unsigned)iCaptionEnd + strLine.GetLength())
				rcDT.SetRect(ptText.x, ptText.y, ptText.x + iMaxCol1Width, ptText.y + iTextHeight);
			else
				rcDT.SetRect(ptText.x + iIconDrawingWidth, ptText.y, ptText.x + iMaxCol1Width, ptText.y + iTextHeight);
			if (iColon != -1) {
				// don't draw empty <col1> strings (they are still handy to use for skipping the <col1> space)
				if (iColon > 0) {
					if (hTheme && bUseEmbeddedThemeFonts)
						g_xpStyle.DrawThemeText(hTheme, pdc->m_hDC, m_bCol1Bold ? TTP_STANDARDTITLE : TTP_STANDARD, TTSS_NORMAL, strLine, iColon + 1, m_dwCol1DrawTextFlags, 0, &rcDT);
					else {
						CFont* pOldFont = m_bCol1Bold ? pdc->SelectObject(&m_fontBold) : NULL;
						pdc->DrawText(strLine, iColon + 1, &rcDT, m_dwCol1DrawTextFlags);
						if (pOldFont)
							pdc->SelectObject(pOldFont);
					}
				}

				LPCTSTR pszCol2 = (LPCTSTR)strLine + iColon + 1;
				while (_istspace((_TUCHAR)*pszCol2))
					pszCol2++;
				if (*pszCol2 != _T('\0')) {
					rcDT.left = ptText.x + iMaxCol1Width + iMiddleMargin;
					rcDT.right = rcDT.left + iMaxCol2Width;
					if (hTheme && bUseEmbeddedThemeFonts)
						g_xpStyle.DrawThemeText(hTheme, pdc->m_hDC, TTP_STANDARD, TTSS_NORMAL, pszCol2, ((LPCTSTR)strLine + strLine.GetLength()) - pszCol2, m_dwCol2DrawTextFlags, 0, &rcDT);
					else
						pdc->DrawText(pszCol2, ((LPCTSTR)strLine + strLine.GetLength()) - pszCol2, &rcDT, m_dwCol2DrawTextFlags);
				}

				ptText.y += iTextHeight;
			}
			else if (bShowFileIcon && iPos <= iCaptionEnd && iPos == strLine.GetLength() + 1){
				// first line on special fileicon tab - draw icon and bold filename
				if (hTheme && bUseEmbeddedThemeFonts)
					g_xpStyle.DrawThemeText(hTheme, pdc->m_hDC, m_bCol1Bold ? TTP_STANDARDTITLE : TTP_STANDARD, TTSS_NORMAL, strLine, strLine.GetLength(), m_dwCol1DrawTextFlags, 0, &CRect(ptText.x  + iIconDrawingWidth, ptText.y, ptText.x + iMaxSingleLineWidth, ptText.y + iTextHeight));
				else {
					CFont* pOldFont = m_bCol1Bold ? pdc->SelectObject(&m_fontBold) : NULL;
					pdc->DrawText(strLine, CRect(ptText.x  + iIconDrawingWidth, ptText.y, ptText.x + iMaxSingleLineWidth, ptText.y + iTextHeight), m_dwCol1DrawTextFlags);
					if (pOldFont)
						pdc->SelectObject(pOldFont);
				}

				ptText.y += iTextHeight;
				int iImage = theApp.GetFileTypeSystemImageIdx(strLine, -1, true);
				if (theApp.GetBigSystemImageList() != NULL) {
					int iPosY = rcDT.top;
					if (iCaptionHeight > iIconHeight)
						iPosY += (iCaptionHeight - iIconHeight) / 2;
					::ImageList_Draw(theApp.GetBigSystemImageList(), iImage, pdc->GetSafeHdc(), ptText.x, iPosY, ILD_NORMAL | ILD_TRANSPARENT);
				}
			}
			else {
				bool bIsBrHeadLine = false;
				if (bAutoFormatText && (strLine.Compare(_T("<br>")) == 0 || (bIsBrHeadLine = strLine.Compare(_T("<br_head>")) == 0) == true)){
					CPen pen;
					pen.CreatePen(0, 1, m_crTooltipTextColor);
					CPen *pOP = pdc->SelectObject(&pen);
					if (bIsBrHeadLine)
						ptText.y = iCaptionHeight;
					pdc->MoveTo(ptText.x, ptText.y + ((iTextHeight - 2) / 2)); 
					pdc->LineTo(ptText.x + iMaxSingleLineWidth, ptText.y + ((iTextHeight - 2) / 2));
					ptText.y += iTextHeight;
					pdc->SelectObject(pOP);
					pen.DeleteObject();
				}
				else{
					if (hTheme && bUseEmbeddedThemeFonts) {
						CRect rcLine(ptText.x, ptText.y, 32767, 32767);
						g_xpStyle.DrawThemeText(hTheme, pdc->m_hDC, TTP_STANDARD, TTSS_NORMAL, strLine, strLine.GetLength(), DT_EXPANDTABS | m_dwCol2DrawTextFlags, 0, &rcLine);
						ptText.y += iTextHeight;
					}
					else {
						// Text is written in the currently selected font. If 'nTabPositions' is 0 and 'lpnTabStopPositions' is NULL,
						// tabs are expanded to eight times the average character width.
						CSize siz = pdc->TabbedTextOut(ptText.x, ptText.y, strLine, 0, NULL, 0);
						ptText.y += siz.cy + 1;
					}
				}
			}
		}
		if (pOldDCFont)
			pdc->SelectObject(pOldDCFont);
		if (hTheme && !bUseEmbeddedThemeFonts)
			pdc->SetBkMode(iOldBkMode);
		if (hTheme)
			g_xpStyle.CloseThemeData(hTheme);
		*pResult = CDRF_SKIPDEFAULT;
		return;
	}
	*pResult = CDRF_DODEFAULT;
}