// GStatic.h															11.12.03
//------------------------------------------------------------------------------
#ifndef __GSTATIC_H__BA571A0E_A7D0_4d6d_9378_CFC0415B84AC__INCLUDED_
#define __GSTATIC_H__BA571A0E_A7D0_4d6d_9378_CFC0415B84AC__INCLUDED_

#include <math.h>

class CGradientStatic : public CWindowImpl<CGradientStatic, CStatic>
{
public:
	BEGIN_MSG_MAP(CGradientStatic)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()

public:
	CGradientStatic()
	{
		m_bInit = true;
		m_bHorizontal = true;
		m_bInvert = false;

		m_crColorRT = GetSysColor(COLOR_ACTIVECAPTION);
		m_crColorLB = m_crColorRT;
		m_crTextColor = GetSysColor(COLOR_CAPTIONTEXT);

		m_Mem.hdc = NULL;
		m_Mem.bmp = NULL;
		m_Mem.oldbmp = NULL;
		m_Mem.cx = 0;
		m_Mem.cy = 0;
	}
	~CGradientStatic()
	{
		EMULE_TRY

		if (m_Mem.hdc)
		{
			if (m_Mem.oldbmp)
				::SelectObject(m_Mem.hdc, m_Mem.oldbmp);
			::DeleteDC(m_Mem.hdc);
		}
		if (m_Mem.bmp)
		{
			::DeleteObject(m_Mem.bmp);
		}

		EMULE_CATCH
	}

	void SetInit(bool bInit)				{ m_bInit = bInit;		 }
	void SetHorizontal(bool bHorz = true)	{ m_bHorizontal = bHorz; }
	void SetInvert(bool bInvert = false)	{ m_bInvert = bInvert;	 }
	void SetColors(COLORREF crText, COLORREF crLB, COLORREF crRT)
	{
		m_crTextColor = crText;
		m_crColorLB = crLB;
		m_crColorRT = crRT;
	}
	void SetFont(CFont *pFont)
	{
		EMULE_TRY

		LOGFONT lfFont;
		pFont->GetLogFont(&lfFont);

		if(m_cfFont.operator HFONT())
			m_cfFont.DeleteObject();
		m_cfFont.CreateFontIndirect(&lfFont);

		EMULE_CATCH
	}

protected:
	bool m_bInit;
	bool m_bHorizontal;
	bool m_bInvert;

	COLORREF m_crColorRT;
	COLORREF m_crColorLB;
	COLORREF m_crTextColor;

	CFont m_cfFont;

	struct _TAG_GRADIENTSTATIC_MEM_
	{
		HDC		hdc;
		HBITMAP	bmp;
		HBITMAP oldbmp;
		int		cx;
		int		cy;

	}m_Mem;

private:
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		EMULE_TRY

		CPaintDC dc(m_hWnd);

		CRect rClient;
		GetClientRect(rClient);

		if (m_bInit)
		{
			CreateGradient(dc.m_hDC, &rClient);
			m_bInit = false;
		}

		::BitBlt(dc.m_hDC, 0, 0, m_Mem.cx, m_Mem.cy, m_Mem.hdc, 0, 0, SRCCOPY);
		return 0;

		EMULE_CATCH
		return -1;
	}

	void DrawRotatedText(HDC hdc, LPCTSTR str, LPRECT rect, double angle, UINT nOptions = 0)
	{
		EMULE_TRY

		// convert angle to radian
		double pi = 3.141592654;
		double radian = pi * 2 / 360 * angle;

		// get the center of a not-rotated text
		SIZE TextSize;
		GetTextExtentPoint32(hdc, str, _tcslen(str), &TextSize);

		POINT center;
		center.x = TextSize.cx / 2;
		center.y = TextSize.cy / 2;

		// now calculate the center of the rotated text
		POINT rcenter;
		rcenter.x = long(cos(radian) * center.x - sin(radian) * center.y);
		rcenter.y = long(sin(radian) * center.x + cos(radian) * center.y);

		// finally draw the text and move it to the center of the rectangle
		SetTextAlign(hdc, TA_BOTTOM);
		SetBkMode(hdc, TRANSPARENT);
		ExtTextOut(hdc, rect->left + (rect->right - rect->left) / 2 - rcenter.x,
					rect->bottom, nOptions, rect, str, _tcslen(str), NULL);

		EMULE_CATCH
	}

	void DrawVerticalText(CRect *pRect)
	{
		EMULE_TRY

		HFONT hOldFont = NULL;

		LOGFONT lfFont;
		if (m_cfFont.operator HFONT())
		{
			m_cfFont.GetLogFont(&lfFont);
		}
		else
		{
			CFont TmpFont = GetFont();
			TmpFont.GetLogFont(&lfFont);
			_tcscpy(lfFont.lfFaceName, _T("Arial"));	// some fonts won't turn :(
		}
		lfFont.lfEscapement = 900;

		CFont Font;
		Font.CreateFontIndirect(&lfFont);
		hOldFont = (HFONT)::SelectObject(m_Mem.hdc, Font.operator HFONT());

		TCHAR szText[256];
		GetWindowText(szText,_countof(szText));

		::SetTextColor(m_Mem.hdc, m_crTextColor);
		::SetBkMode(m_Mem.hdc, TRANSPARENT);
		CRect rText = pRect;
		rText.bottom -= 5;

		DrawRotatedText(m_Mem.hdc, szText, rText, 90);

		if (hOldFont)
		{
			::SelectObject(m_Mem.hdc, hOldFont);
		}

		EMULE_CATCH
	}

	void DrawHorizontalText(CRect *pRect)
	{
		EMULE_TRY

		HFONT hOldFont = NULL;

		if (m_cfFont.operator HFONT())
		{
			hOldFont = (HFONT)::SelectObject(m_Mem.hdc, m_cfFont.operator HFONT());
		}

		TCHAR szText[256];
		GetWindowText(szText,_countof(szText));

		::SetTextColor(m_Mem.hdc, m_crTextColor);
		::SetBkMode(m_Mem.hdc, TRANSPARENT);
		::DrawText(m_Mem.hdc, szText, _tcsclen(szText), pRect,
					DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
		if (hOldFont)
		{
			::SelectObject(m_Mem.hdc, hOldFont);
		}

		EMULE_CATCH
	}

	void DrawVerticalGradient()
	{
		EMULE_TRY

		double dRstep = (GetRValue(m_crColorLB) - GetRValue(m_crColorRT)) / static_cast<double>(m_Mem.cy);
		double dGstep = (GetGValue(m_crColorLB) - GetGValue(m_crColorRT)) / static_cast<double>(m_Mem.cy);
		double dBstep = (GetBValue(m_crColorLB) - GetBValue(m_crColorRT)) / static_cast<double>(m_Mem.cy);
		double r = GetRValue(m_crColorRT);
		double g = GetGValue(m_crColorRT);
		double b = GetBValue(m_crColorRT);

		for(int y = 0; y < m_Mem.cy; y++)
		{
			HPEN hPen, hOldPen = NULL;
			hPen = ::CreatePen(PS_SOLID, 1, RGB(r,g,b));
			hOldPen = (HPEN)::SelectObject(m_Mem.hdc, hPen);

			::MoveToEx(m_Mem.hdc,0,y,NULL);
			::LineTo(m_Mem.hdc,m_Mem.cx,y);
			if (hOldPen)
				::SelectObject(m_Mem.hdc, hOldPen);
			if (hPen)
				::DeleteObject(hPen);

			r += dRstep;
			g += dGstep;
			b += dBstep;
		}

		EMULE_CATCH
	}

	void DrawHorizontalGradient()
	{
		EMULE_TRY

		double dRstep = (GetRValue(m_crColorRT) - GetRValue(m_crColorLB)) / static_cast<double>(m_Mem.cx);
		double dGstep = (GetGValue(m_crColorRT) - GetGValue(m_crColorLB)) / static_cast<double>(m_Mem.cx);
		double dBstep = (GetBValue(m_crColorRT) - GetBValue(m_crColorLB)) / static_cast<double>(m_Mem.cx);
		double r = GetRValue(m_crColorLB);
		double g = GetGValue(m_crColorLB);
		double b = GetBValue(m_crColorLB);

		for(int x = 0; x < m_Mem.cx; x++)
		{
			HPEN hPen, hOldPen = NULL;
			hPen = ::CreatePen(PS_SOLID, 1, RGB(r,g,b));
			hOldPen = (HPEN)::SelectObject(m_Mem.hdc, hPen);

			::MoveToEx(m_Mem.hdc,x,0,NULL);
			::LineTo(m_Mem.hdc,x,m_Mem.cy);
			if (hOldPen)
				::SelectObject(m_Mem.hdc,hOldPen);
			if (hPen)
				::DeleteObject(hPen);

			r += dRstep;
			g += dGstep;
			b += dBstep;
		}

		EMULE_CATCH
	}

	void CreateGradient(HDC hDC, CRect *pRect)
	{
		EMULE_TRY

		m_Mem.cx = pRect->Width();
		m_Mem.cy = pRect->Height();

		if (m_Mem.hdc)
		{
			if (m_Mem.bmp && m_Mem.oldbmp)
				::SelectObject(m_Mem.hdc,m_Mem.oldbmp);
			::DeleteDC(m_Mem.hdc);
		}
		m_Mem.hdc = ::CreateCompatibleDC(hDC);

		if (m_Mem.bmp)
			::DeleteObject(m_Mem.bmp);
		m_Mem.bmp = ::CreateCompatibleBitmap(hDC, m_Mem.cx, m_Mem.cy);
		m_Mem.oldbmp = (HBITMAP)::SelectObject(m_Mem.hdc, m_Mem.bmp);

		if (m_bHorizontal)
		{
			DrawHorizontalGradient();
			DrawHorizontalText(pRect);
		}
		else
		{
			DrawVerticalGradient();
			DrawVerticalText(pRect);
		}

		EMULE_CATCH
	}
};

#endif	// #ifndef __GSTATIC_H__BA571A0E_A7D0_4d6d_9378_CFC0415B84AC__INCLUDED_