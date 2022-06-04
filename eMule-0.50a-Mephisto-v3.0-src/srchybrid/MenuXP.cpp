// CustMenu.cpp: implementation of the CMenuXP class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MenuXP.h"
#include "KeyHelper.h"
#include "emule.h"
#include "preferences.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// constants used for drawing
const int CXGAP = 0;				// num pixels between button and text
const int CXTEXTMARGIN = 2;		// num pixels after hilite to start text
const int CXBUTTONMARGIN = 2;	// num pixels wider button is than bitmap
const int CYBUTTONMARGIN = 2;	// ditto for height

// DrawText flags
const int DT_MYSTANDARD = DT_SINGLELINE|DT_LEFT|DT_VCENTER;

// MenuXP Sub Heading [fafner] - MyTh
#define SUB_HEAD_MULTI 0.95

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CMenuXP, CMenu)

CMenuXP::CMenuXP()
{
	//initialize menu font with the default
	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(info), &info, 0);
	VERIFY(m_fontMenu.CreateFontIndirect(&info.lfMenuFont));

	//initialize colors with system default
	m_clrBackGround = ::GetSysColor(COLOR_MENU);
	m_clrSelectedBar = ::GetSysColor(COLOR_HIGHLIGHT);
	m_clrSelectedText = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
	m_clrText = ::GetSysColor(COLOR_MENUTEXT);
	m_clrDisabledText = ::GetSysColor(COLOR_GRAYTEXT);
	m_clrHeadingText = ::GetSysColor(COLOR_MENUTEXT); // MenuXP Sub Heading [fafner] - MyTh
	m_clrIconArea = m_clrBackGround;

	//initialize sidebar colors
	m_clrSideBarStart = RGB(0, 0, 192);
	m_clrSideBarEnd = RGB(0, 0, 0);

	//the default sytle is office style
	m_Style = STYLE_OFFICE;

	// use embosed dissabled icons
	m_EmbossedIcons = TRUE;

	m_bBreak = false;
	m_bBreakBar = false;
	m_hBitmap = NULL;
}

CMenuXP::~CMenuXP()
{
	m_fontMenu.DeleteObject();
	//Clear();
}


void CMenuXP::MeasureItem( LPMEASUREITEMSTRUCT lpms )
{
	if (lpms->CtlType != ODT_MENU)
		return;

	CMenuXPItem	*pItem = (CMenuXPItem *)lpms->itemData;
	//TRACE("pItem: 0x%x",(DWORD)pItem);	//This line prevent boundschecker from issue a resource leak

	if (!pItem || !pItem->IsMyData())
		return;

	if (pItem->m_bSideBar)
	{
		lpms->itemWidth = pItem->m_nSize;
		lpms->itemHeight = 0;
	}
	else if (pItem->m_bSeparator)
	{
		// separator: use half system height and zero width
		lpms->itemHeight = ::GetSystemMetrics(SM_CYMENUCHECK)>>1;
		lpms->itemWidth  = 0;
	}
	else
	{
		//calculate the size needed to draw the text: use DrawText with DT_CALCRECT

		CWindowDC dc(NULL);	// screen DC--I won't actually draw on it
		CRect rcText(0,0,0,0);
		CFont* pOldFont;
		//Calculate the size with bold font, for default item to be correct
		CFont	fontBold;
		LOGFONT	logFont;
		m_fontMenu.GetLogFont(&logFont);
		// ==> MenuXP Sub Heading [fafner] - MyTh
		if (pItem->m_bHeading) {
			logFont.lfHeight = (LONG)(logFont.lfHeight * SUB_HEAD_MULTI);
			logFont.lfWeight = FW_SEMIBOLD;
		}
		else
		// <== MenuXP Sub Heading [fafner] - MyTh
		logFont.lfWeight = FW_BOLD;
		fontBold.CreateFontIndirect(&logFont);
		
		pOldFont = dc.SelectObject(&fontBold);
//		pOldFont= dc.SelectObject(&m_fontMenu);
		dc.DrawText(pItem->m_strText, rcText, DT_MYSTANDARD|DT_CALCRECT);
		dc.SelectObject(pOldFont);

		// the height of the item should be the maximun of the text and the button
		lpms->itemHeight = max(rcText.Height(), pItem->m_nSize + (CYBUTTONMARGIN<<1));

		// ==> MenuXP Sub Heading [fafner] - MyTh
		if (pItem->m_bHeading)
			lpms->itemHeight = (LONG)(lpms->itemHeight * SUB_HEAD_MULTI);
		// <== MenuXP Sub Heading [fafner] - MyTh

		if (pItem->m_bButtonOnly)
		{	//for button only style, we set the item's width to be the same as its height
			lpms->itemWidth = lpms->itemHeight;
		}
		else
		{
			// width is width of text plus a bunch of stuff
			int cx = rcText.Width();	// text width 
			cx += CXTEXTMARGIN<<1;		// L/R margin for readability
			cx += CXGAP;					// space between button and menu text
			cx += (pItem->m_nSize + CYBUTTONMARGIN * 2) <<1;		// button width (L=button; R=empty margin)

			lpms->itemWidth = cx;		// done deal
		}
	}
	
	// whatever value I return in lpms->itemWidth, Windows will add the
	// width of a menu checkmark, so I must subtract to defeat Windows. Argh.
	//
	lpms->itemWidth -= GetSystemMetrics(SM_CXMENUCHECK)-1;

	//TRACE(_T("MeasureItem: ID(%d), Width(%d), Height(%d)\n"), 
	//	lpms->itemID,
	//	lpms->itemWidth, lpms->itemHeight);

}

void CMenuXP::DrawItem( LPDRAWITEMSTRUCT lpds )
{
	ASSERT(lpds);
	if (lpds->CtlType != ODT_MENU)
		return; // not handled by me
	CMenuXPItem * pItem = (CMenuXPItem *)lpds->itemData;
	if (!pItem)
		return;
	
	ASSERT(lpds->itemAction != ODA_FOCUS);
	ASSERT(lpds->hDC);
	CDC dc;
	dc.Attach(lpds->hDC);

	//get the drawing area
	CRect rcItem = lpds->rcItem;

	//TRACE(_T("DrawItem: ID(%d), Widht(%d),  Height(%d)\n"), 
	//	lpds->itemID, rcItem.Width(), rcItem.Height());

	if (pItem->m_bSideBar)
	{
		CRect rcClipBox;
		dc.GetClipBox(rcClipBox);
		//before drawing the sidebar, we must fill the entire menu area with its backgroundcolor,
		//orelse, the breakbar area will remain the the default menu color
		//so, if you want to avoid strange color and don't want a sidebar, just add a sidebar with 
		//zero width
		//dc.FillSolidRect(rcClipBox, m_Style==STYLE_XP? m_clrIconArea : m_clrBackGround);
		
		//draw the side bar
		CRect rc = rcItem;
		rc.top = rcClipBox.top;
		rc.bottom = rcClipBox.bottom;
		DrawSideBar(&dc, rc, pItem->m_hIcon, pItem->m_strText);
	}
	else if (pItem->m_bSeparator) 
	{
		//draw background first
		DrawBackGround(&dc, rcItem, FALSE, FALSE);
		// draw the background
		CRect rc = rcItem;								// copy rect
		rc.top += rc.Height()>>1;						// vertical center
		dc.DrawEdge(&rc, EDGE_ETCHED, BF_TOP);		// draw separator line
		
		// in XP mode, fill the icon area with the iconarea color
		if (m_Style == STYLE_XP)
		{
			CRect rcArea(rcItem.TopLeft(),
				CSize(pItem->m_nSize + (CYBUTTONMARGIN<<1), 
				pItem->m_nSize + (CYBUTTONMARGIN<<1)));
			DrawIconArea(&dc, rcArea, FALSE, FALSE, FALSE);
		}
	} 
	else
	{
		BOOL bDisabled = lpds->itemState & ODS_GRAYED;
		BOOL bSelected = lpds->itemState & ODS_SELECTED;
		BOOL bChecked  = lpds->itemState & ODS_CHECKED;

		// ==> MenuXP Sub Heading [fafner] - MyTh
		if (pItem->m_bHeading)
		{
			HBITMAP	bmpBar = CreateGradientBMP(
				dc.m_hDC, m_clrSideBarStart, m_clrSideBarEnd,
				rcItem.Width(), rcItem.Height(),
				2, 256);
			if (bmpBar)
			{
				CDC memDC;
				memDC.CreateCompatibleDC(&dc);
				HBITMAP hOldBmp = (HBITMAP)::SelectObject(memDC.m_hDC, bmpBar);
				dc.BitBlt(rcItem.left, rcItem.top,
					rcItem.Width(), rcItem.Height(),
					&memDC, 0, 0, SRCCOPY);
				::SelectObject(memDC, hOldBmp);
				::DeleteObject(bmpBar);
			}
		}
		else
		// <== MenuXP Sub Heading [fafner] - MyTh

		//draw the background first
		DrawBackGround(&dc, rcItem, bSelected, bDisabled);
		
		//Draw the icon area for XP style
		if (m_Style == STYLE_XP)
		{
			CRect rcArea(rcItem.TopLeft(), CSize(rcItem.Height(), rcItem.Height()));
			DrawIconArea(&dc, rcArea, bSelected, bDisabled, bChecked);
		}

		//draw the button, not the icon
		CRect rcButton(rcItem.TopLeft(), CSize(rcItem.Height(), rcItem.Height()));
		if (pItem->m_bButtonOnly)
			rcButton = rcItem;
		if (pItem->m_hIcon || bChecked)
		{
			DrawButton(&dc, rcButton, bSelected, bDisabled, bChecked);
		}
		// ==> check mark superior to icon [fafner] - Stulle
		/*
		//draw the icon actually
		if (pItem->m_hIcon)
		{
			CRect	rcIcon = rcButton;
			rcIcon.DeflateRect(2, 2);
			DrawIcon(&dc, rcIcon, pItem->m_hIcon, bSelected, bDisabled, bChecked);
		}
		else if (bChecked)	
		{
			//draw the check mark
			CRect	rcCheck = rcButton;
			rcCheck.DeflateRect(2, 2);
			DrawCheckMark(&dc, rcCheck, bSelected);
		}
		*/
		if (bChecked)	
		{
			//draw the check mark
			CRect	rcCheck = rcButton;
			rcCheck.DeflateRect(2, 2);
			DrawCheckMark(&dc, rcCheck, bSelected);
		}
		//draw the icon actually
		else if (pItem->m_hIcon)
		{
			CRect	rcIcon = rcButton;
			rcIcon.DeflateRect(2, 2);
			DrawIcon(&dc, rcIcon, pItem->m_hIcon, bSelected, bDisabled, bChecked);
		}
		// <== check mark superior to icon [fafner] - Stulle

		//draw text finally
		if (!pItem->m_bButtonOnly)
		{
			CRect rcText = rcItem;				 // start w/whole item
			rcText.left += rcButton.Width() + CXGAP + CXTEXTMARGIN; // left margin
			rcText.right -= pItem->m_nSize;				 // right margin
			// ==> MenuXP Sub Heading [fafner] - MyTh
			if (pItem->m_bHeading)
				DrawText(&dc, rcText, pItem->m_strText, true, false, 0, true);
			else
			// <== MenuXP Sub Heading [fafner] - MyTh			
			DrawText(&dc, rcText, pItem->m_strText, bSelected, bDisabled, lpds->itemState&ODS_DEFAULT ? 1 : 0);
		}

	}

	dc.Detach();
}

//draw background
void CMenuXP::DrawBackGround(CDC *pDC, CRect rect, BOOL bSelected, BOOL bDisabled)
{
	if (m_hBitmap && (!bSelected || bDisabled))
	{
		FillRect(pDC, rect, m_clrBackGround); // Neo Fix
		pDC->BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), &m_memDC,
			0, rect.top, SRCCOPY);
	}
	else if (bSelected)
	{
		FillRect(pDC, rect, bDisabled? ((m_Style==STYLE_XP)?m_clrBackGround:m_clrSelectedBar) : m_clrSelectedBar);
	}
	else
	{
		FillRect(pDC, rect, m_clrBackGround);
	}

	//in XP mode, draw a line rectangle around
	if (m_Style == STYLE_XP && bSelected && !bDisabled)
	{
		CGdiObject *pOldBrush = pDC->SelectStockObject(HOLLOW_BRUSH);
		CGdiObject	*pOldPen = pDC->SelectStockObject(BLACK_PEN);
		pDC->Rectangle(rect);
		pDC->SelectObject(pOldBrush);
		pDC->SelectObject(pOldPen);
	}
}

//draw the icon button, the icon is not included
void CMenuXP::DrawButton(CDC *pDC, CRect rect, BOOL bSelected, BOOL bDisabled, BOOL bChecked)
{
	if (m_Style == STYLE_OFFICE)
	{
		// normal: fill BG depending on state
		if (bChecked && !bSelected)
		{
			FillRect(pDC, rect, GetSysColor(COLOR_3DHILIGHT));
		}
		else
			FillRect(pDC, rect, m_clrBackGround);
	
		// draw pushed-in or popped-out edge
		if (!bDisabled && (bSelected || bChecked) )
		{
			pDC->DrawEdge(rect, bChecked ? BDR_SUNKENOUTER : BDR_RAISEDINNER,
				BF_RECT);
		}
	}
	else if (m_Style == STYLE_XP && !bSelected)
	{
		if (bChecked && !bDisabled)
		{
			DrawBackGround(pDC, rect, TRUE, FALSE);
		}
	}
	
}

//draw the icon area, the icon is not included, only in XP style
void CMenuXP::DrawIconArea(CDC *pDC, CRect rect, BOOL bSelected, BOOL bDisabled, BOOL /*bChecked*/)
{
	if (m_Style != STYLE_XP)
		return;

	// normal: fill BG depending on state
	if (!bSelected || bDisabled)
	{
		FillRect(pDC, rect, m_clrIconArea);
	}
}

// CreateGrayscaleIcon function coded by lit
#define GRAY(r,g,b) (19595*r + 38470*g + 7471*b) >> 16
HICON CreateGrayscaleIcon(CDC *pDC, HICON hIcon)
{
	ICONINFO iinfo;
	GetIconInfo(hIcon,&iinfo);

	// get the bitmap's size and colour information
	BITMAP bm;
	::GetObject(iinfo.hbmColor, sizeof(BITMAP), &bm);

	// create a DIBSection copy of the original bitmap
	HBITMAP hDib = (HBITMAP)::CopyImage(iinfo.hbmColor, IMAGE_BITMAP, 0, 0, LR_COPYRETURNORG | LR_CREATEDIBSECTION);

	if (bm.bmBitsPixel < 16)
	{   // bitmap has a colour table, so we modify the colour table
		CDC memDC;
		memDC.CreateCompatibleDC(pDC);
		int SavedMemDC = memDC.SaveDC();
		memDC.SelectObject(hDib);
		int nColours = 1 << bm.bmBitsPixel;

		RGBQUAD pal[256];

		// Get the colour table
		::GetDIBColorTable(memDC.m_hDC, 0, nColours, pal);

		// modify the colour table
		for (int x = 0; x < nColours; x++)
		{
			BYTE nGray = (BYTE)(GRAY(pal[x].rgbRed, pal[x].rgbGreen, pal[x].rgbBlue)); //Fafner: C4333 - 0529 - MyTh
			pal[x].rgbRed = nGray;
			pal[x].rgbGreen = nGray;
			pal[x].rgbBlue = nGray;
		}

		// set the modified colour tab to the DIBSection bitmap
		::SetDIBColorTable(memDC.m_hDC, 0, nColours, pal);

		memDC.RestoreDC(SavedMemDC);
		memDC.DeleteDC();
		VERIFY(::DeleteObject(iinfo.hbmColor));
		iinfo.hbmColor = hDib;
		HICON hNewIcon = CreateIconIndirect(&iinfo);
		VERIFY(::DeleteObject(iinfo.hbmColor));
		VERIFY(::DeleteObject(iinfo.hbmMask));
		return hNewIcon;
	}
	else
	{   // the bitmap does not have a colour table, so we modify the bitmap bits directly
		int Size = bm.bmHeight * bm.bmWidth;

		BITMAPINFO bmi;
		bmi.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biHeight        = bm.bmHeight;
		bmi.bmiHeader.biWidth         = bm.bmWidth;
		bmi.bmiHeader.biPlanes        = 1;
		bmi.bmiHeader.biBitCount      = bm.bmBitsPixel;
		bmi.bmiHeader.biCompression   = BI_RGB;
		bmi.bmiHeader.biSizeImage     = ((bm.bmWidth * bm.bmBitsPixel + 31) & (~31)) / 8 * bm.bmHeight;
		bmi.bmiHeader.biXPelsPerMeter = 0;
		bmi.bmiHeader.biYPelsPerMeter = 0;
		bmi.bmiHeader.biClrUsed       = 0;
		bmi.bmiHeader.biClrImportant  = 0;

		// Get the bitmaps data bits
		BYTE *pBits = new BYTE[bmi.bmiHeader.biSizeImage];
		VERIFY (::GetDIBits(pDC->m_hDC, hDib, 0, bm.bmHeight, pBits, &bmi, DIB_RGB_COLORS));

		if (bm.bmBitsPixel == 32)
		{
			DWORD *dst=(DWORD *)pBits;

			while (Size--)
			{
				int nGray = GRAY(GetBValue(*dst), GetGValue(*dst), GetRValue(*dst));
				*dst = (*dst & 0xff000000) | ((DWORD)RGB(nGray, nGray, nGray) & 0x00ffffff);
				dst++;
			}
		}

		else if (bm.bmBitsPixel == 24)
		{
			BYTE *dst=(BYTE*)pBits;

			for (int dh = 0; dh < bm.bmHeight; dh++)
			{
				for (int dw = 0; dw < bm.bmWidth; dw++)
				{
					int nGray = GRAY(dst[2], dst[1], dst[0]);
					dst[0]=(BYTE)nGray;
					dst[1]=(BYTE)nGray;
					dst[2]=(BYTE)nGray;
					dst += 3;
				}

				// each row is DWORD aligned, so when we reach the end of a row, we
				// have to realign the pointer to point to the start of the next row
				int pos = (int)dst - (int)pBits;
				int rem = pos % 4;
				if (rem)
					dst += 4 - rem;
			}
		}

		else if (bm.bmBitsPixel == 16)
		{
			WORD *dst=(WORD*)pBits;

			while (Size--)
			{
				BYTE b = (BYTE)((*dst)&(0x1F));
				BYTE g = (BYTE)(((*dst)>>5)&(0x1F));
				BYTE r = (BYTE)(((*dst)>>10)&(0x1F));

				int nGray = GRAY(r, g, b);
				*dst = ((WORD)(((BYTE)(nGray)|((WORD)((BYTE)(nGray))<<5))|(((DWORD)(BYTE)(nGray))<<10)));
				dst++;
			}
		}

		// set the modified bitmap data bits to the DIBSection
		::SetDIBits(pDC->m_hDC, hDib, 0, bm.bmHeight, pBits, &bmi, DIB_RGB_COLORS);
		delete[] pBits;
		VERIFY(::DeleteObject(iinfo.hbmColor));
		iinfo.hbmColor = hDib;
		HICON hNewIcon = CreateIconIndirect(&iinfo);
		VERIFY(::DeleteObject(iinfo.hbmColor));
		VERIFY(::DeleteObject(iinfo.hbmMask));
		return hNewIcon;
	}
}


//draw the icon
void CMenuXP::DrawIcon(CDC *pDC, CRect rect, HICON hIcon, BOOL bSelected, BOOL bDisabled, BOOL bChecked)
{
	if (bDisabled)
	{
		if(m_EmbossedIcons)
		{
			DrawEmbossed(pDC, hIcon, rect);
		}
		else if(HICON hGrayIcon = CreateGrayscaleIcon(pDC,hIcon))
		{
			::DrawIconEx(pDC->m_hDC, rect.left, rect.top, hGrayIcon,
				rect.Width(), rect.Height(), 0, NULL,
				DI_NORMAL);
			::DestroyIcon(hGrayIcon);
		}
	}
	else
	{
		if(m_Style==STYLE_XP && bSelected && !bChecked)
		{
			DrawEmbossed(pDC,hIcon,rect,FALSE, TRUE);
			rect.OffsetRect(-1,-1);
		}
		::DrawIconEx(pDC->m_hDC, rect.left, rect.top, hIcon,
			rect.Width(), rect.Height(), 0, NULL,
			DI_NORMAL);
	}
}

//draw the sidebar
void CMenuXP::DrawSideBar(CDC *pDC, CRect rect, HICON /*hIcon*/, CString strText)
{
	rect.right += 3;	//fill the gap produced by the menubreak

	HBITMAP	bmpBar = CreateGradientBMP(
		pDC->m_hDC, m_clrSideBarStart, m_clrSideBarEnd,
		rect.Width(), rect.Height(),
		0, 256);
	if (bmpBar)
	{
		CDC memDC;
		memDC.CreateCompatibleDC(pDC);
		HBITMAP hOldBmp = (HBITMAP)::SelectObject(memDC.m_hDC, bmpBar);
		pDC->BitBlt(rect.left, rect.top,
			rect.Width(), rect.Height(),
			&memDC, 0, 0, SRCCOPY);
		::SelectObject(memDC, hOldBmp);
		::DeleteObject(bmpBar);
	}
	//Draw Sidebar text
	CFont	vertFont;
	vertFont.CreateFont(16, 0, 900, 900, FW_BOLD,
		0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH, _T("Arial"));
	CFont *pOldFont = pDC->SelectObject(&vertFont);
	COLORREF oldColor = pDC->GetTextColor();
	pDC->SetTextColor(RGB(255, 255, 255));
	pDC->SetBkMode(TRANSPARENT);
	pDC->TextOut(rect.left+2, rect.bottom-4, strText);
	pDC->SetTextColor(oldColor);	
	pDC->SelectObject(pOldFont);
	vertFont.DeleteObject();

}

//draw the check mark
void CMenuXP::DrawCheckMark(CDC *pDC, CRect rect, BOOL /*bSelected*/)
{
/*	//"#define OEMRESOURCE" must be in the begining of your stdafx.h
	//for the LoadOEMBitmap to work
#ifdef OEMRESOURCE
	CBitmap bmp;	//Check mark bitmap
	VERIFY(bmp.LoadOEMBitmap(OBM_CHECK));	

	// center bitmap in caller's rectangle
	BITMAP bm;
	bmp.GetBitmap(&bm);
	int cx = bm.bmWidth;
	int cy = bm.bmHeight;
	CRect rcDest = rect;
	CPoint p(0,0);
	CSize delta(CPoint((rect.Width() - cx)/2, (rect.Height() - cy)/2));
	if (rect.Width() > cx)
		rcDest = CRect(rect.TopLeft() + delta, CSize(cx, cy));
	else
		p -= delta;

	// select checkmark into memory DC
	CDC memdc;
	memdc.CreateCompatibleDC(pDC);
	CBitmap *pOldBmp = memdc.SelectObject(&bmp);

	COLORREF colorOld =
		pDC->SetBkColor(GetSysColor(bSelected ? COLOR_MENU : COLOR_3DLIGHT));
	pDC->BitBlt(rcDest.left, rcDest.top, rcDest.Width(), rcDest.Height(),
		&memdc, p.x, p.y, SRCCOPY);
	pDC->SetBkColor(colorOld);

	memdc.SelectObject(pOldBmp);
	bmp.DeleteObject();
#else
	CRect	rcDest = rect;
	pDC->DrawFrameControl(rcDest, DFC_MENU, DFCS_MENUCHECK);
#endif
*/
	//Draw it myself :(
	const int nCheckDots = 8;
	CPoint pt1, pt2, pt3;	//3 point of the checkmark
	pt1.x = 0;	// 5/18 of the rect width
	pt1.y = 3;	
	pt2.x = 2;
	pt2.y = 5;
	pt3.x = 7;
	pt3.y = 0;

	int xOff = (rect.Width()-nCheckDots)/2 + rect.left ;
	int yOff = (rect.Height()-nCheckDots)/2 + rect.top;
	pt1.Offset(xOff, yOff);
	pt2.Offset(xOff, yOff);
	pt3.Offset(xOff, yOff);

	CPen	pen(PS_SOLID, 1, RGB(0, 0, 0));
	CGdiObject *pOldPen = pDC->SelectObject(&pen);
	pDC->MoveTo(pt1);
	pDC->LineTo(pt2);
	pDC->LineTo(pt3);
	pt1.Offset(0, 1);
	pt2.Offset(0, 1);
	pt3.Offset(0, 1);
	pDC->MoveTo(pt1);
	pDC->LineTo(pt2);
	pDC->LineTo(pt3);
	pt1.Offset(0, 1);
	pt2.Offset(0, 1);
	pt3.Offset(0, 1);
	pDC->MoveTo(pt1);
	pDC->LineTo(pt2);
	pDC->LineTo(pt3);
	pDC->SelectObject(pOldPen);
}

//Draw menu text
void CMenuXP::DrawText(CDC *pDC, CRect rect, CString strText, BOOL bSelected, BOOL bDisabled, BOOL bBold, bool bHeading) // MenuXP Sub Heading [fafner] - MyTh
{
	CFont*	pOldFont;
	CFont	fontBold;
	CFont	fontHead; // MenuXP Sub Heading [fafner] - MyTh

	if (bBold)
	{
		LOGFONT	logFont;
		m_fontMenu.GetLogFont(&logFont);
		logFont.lfWeight = FW_BOLD;
		fontBold.CreateFontIndirect(&logFont);

		pOldFont = pDC->SelectObject(&fontBold);
	}
	// ==> MenuXP Sub Heading [fafner] - MyTh
	else if (bHeading)
	{
		LOGFONT	logFont;
		m_fontMenu.GetLogFont(&logFont);
		logFont.lfHeight = (LONG)(logFont.lfHeight * SUB_HEAD_MULTI);
		logFont.lfWeight = FW_SEMIBOLD;
		fontHead.CreateFontIndirect(&logFont);

		pOldFont = pDC->SelectObject(&fontHead);
	}
	// <== MenuXP Sub Heading [fafner] - MyTh
	else
	{
		pOldFont = pDC->SelectObject(&m_fontMenu);
	}

	pDC->SetBkMode(TRANSPARENT);
	if (bDisabled && (!bSelected || m_Style == STYLE_XP))
	{
		DrawMenuText(*pDC, rect + CPoint(1, 1), strText, m_clrSelectedText);
	}
	if (bDisabled)
	{
		DrawMenuText(*pDC, rect, strText, m_clrDisabledText);
	}
	else
	{
		// ==> MenuXP Sub Heading [fafner] - MyTh
		if(bHeading)
			DrawMenuText(*pDC, rect + CPoint(1, 1), strText, m_clrHeadingText);
		else
		// <== MenuXP Sub Heading [fafner] - MyTh
			DrawMenuText(*pDC, rect, strText, bSelected? m_clrSelectedText : m_clrText);
	}

	pDC->SelectObject(pOldFont);

	if (bBold)
		fontBold.DeleteObject();
	// ==> MenuXP Sub Heading [fafner] - MyTh
	if (bHeading)
		fontHead.DeleteObject();
	// <== MenuXP Sub Heading [fafner] - MyTh
}

//set menu font
BOOL CMenuXP::SetMenuFont(LOGFONT	lgfont)
{
	m_fontMenu.DeleteObject();
	return m_fontMenu.CreateFontIndirect(&lgfont);
}

BOOL CMenuXP::DestroyMenu()
{
	Clear();
	return CMenu::DestroyMenu();
}

//clear all memory and handles
void CMenuXP::Clear(void)
{
	if (m_hBitmap)
	{
		DeleteObject(m_hBitmap);
		m_hBitmap = NULL;
	}
	UINT nCount = GetMenuItemCount();
	for (UINT i=nCount; i>0; i--)
		RemoveMenu(i-1,MF_BYPOSITION);
}

//draw embossed icon for the disabled item
const DWORD		MAGICROP		= 0xb8074a;
const COLORREF CWHITE  = RGB(255,255,255);

void CMenuXP::DrawEmbossed(CDC *pDC, HICON hIcon, CRect rect, BOOL bColor, BOOL bShadow)
{
	CDC	memdc;
	memdc.CreateCompatibleDC(pDC);
	int cx = rect.Width();
	int cy = rect.Height();


	// create mono or color bitmap
	CBitmap bm;
	if (bColor)
		bm.CreateCompatibleBitmap(pDC, cx, cy);
	else
		bm.CreateBitmap(cx, cy, 1, 1, NULL);

	// draw image into memory DC--fill BG white first
	CBitmap* pOldBitmap = memdc.SelectObject(&bm);
	//FillRect(&memdc, CRect(0, 0, cx, cy), m_clrBackGround);
	memdc.PatBlt(0, 0, cx, cy, WHITENESS);
	::DrawIconEx(memdc.m_hDC, 0, 0, hIcon, cx, cy, 1, NULL, DI_NORMAL);

	// This seems to be required. Why, I don't know. ???
	COLORREF colorOldBG = pDC->SetBkColor(CWHITE);

	// Draw using hilite offset by (1,1), then shadow
	CBrush brShadow(GetSysColor(COLOR_3DSHADOW));
	CBrush brHilite(GetSysColor(COLOR_3DHIGHLIGHT));
	CBrush* pOldBrush = pDC->SelectObject(bShadow ? &brShadow : &brHilite);
	pDC->BitBlt(rect.left+1, rect.top+1, cx, cy, &memdc, 0, 0, MAGICROP);
	pDC->SelectObject(&brShadow);
	pDC->BitBlt(rect.left, rect.top, cx, cy, &memdc, 0, 0, MAGICROP);
	pDC->SelectObject(pOldBrush);
	pDC->SetBkColor(colorOldBG);				 // restore
	memdc.SelectObject(pOldBitmap);		 // ...
	bm.DeleteObject();
	brShadow.DeleteObject();
	brHilite.DeleteObject();

}

//////////////////
// Shorthand to fill a rectangle with a solid color.
//
void CMenuXP::FillRect(CDC *pDC, const CRect& rc, COLORREF color)
{
	CBrush brush(color);
	CBrush* pOldBrush = pDC->SelectObject(&brush);
	pDC->PatBlt(rc.left, rc.top, rc.Width(), rc.Height(), PATCOPY);
	pDC->SelectObject(pOldBrush);
	brush.DeleteObject();
}

HBITMAP CMenuXP::CreateGradientBMP(HDC hDC,COLORREF cl1,COLORREF cl2,int nWidth,int nHeight,int nDir,int nNumColors)
{
	if(nNumColors > 256)
		nNumColors = 256;

	COLORREF		PalVal[256];
	ZeroMemory(PalVal, sizeof(COLORREF)*256);

	int nIndex;
	BYTE peRed=0,peGreen=0,peBlue=0;

	int r1=GetRValue(cl1);
	int r2=GetRValue(cl2);
	int g1=GetGValue(cl1);
	int g2=GetGValue(cl2);
	int b1=GetBValue(cl1);
	int b2=GetBValue(cl2);

    for (nIndex = 0; nIndex < nNumColors; nIndex++)
    {
        peRed = (BYTE) (r1 + MulDiv((r2-r1),nIndex,nNumColors-1));
        peGreen = (BYTE) (g1 + MulDiv((g2-g1),nIndex,nNumColors-1));
        peBlue = (BYTE) (b1 + MulDiv((b2-b1),nIndex,nNumColors-1));

		PalVal[nIndex]=(peRed << 16) | (peGreen << 8) | (peBlue);
	}

	int x,y,w,h;
	w=nWidth;
	h=nHeight;
	
	LPDWORD			pGradBits;
	BITMAPINFO		GradBitInfo;

	pGradBits=(DWORD*) malloc(w*h*sizeof(DWORD));
	ZeroMemory(&GradBitInfo, sizeof(BITMAPINFO));

	GradBitInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	GradBitInfo.bmiHeader.biWidth=w;
	GradBitInfo.bmiHeader.biHeight=h;
	GradBitInfo.bmiHeader.biPlanes=1;
	GradBitInfo.bmiHeader.biBitCount=32;
	GradBitInfo.bmiHeader.biCompression=BI_RGB;
	
	if(nDir==0) 
	{
		for(y=0;y<h;y++) 
		{
			for(x=0;x<w;x++) 
			{
				*(pGradBits+(y*w)+x)=PalVal[MulDiv(nNumColors,y,h)];
			}
		}
	}
	else if(nDir==1) 
	{
		for(y=0;y<h;y++)
		{
			int l,r;
			l=MulDiv((nNumColors/2),y,h);
			r=l+(nNumColors/2)-1;
			for(x=0;x<w;x++)
			{
				*(pGradBits+(y*w)+x)=PalVal[l+MulDiv((r-l),x,w)];
			}
		}
	}
	else if(nDir==2)
	{
		for(x=0;x<w;x++)
		{
			for(y=0;y<h;y++)
			{
				*(pGradBits+(y*w)+x)=PalVal[MulDiv(nNumColors,x,w)];
			}
		}
	}
	else if(nDir==3)
	{
		for(y=0;y<h;y++)
		{
			int l,r;
			r=MulDiv((nNumColors/2),y,h);
			l=r+(nNumColors/2)-1;
			for(x=0;x<w;x++)
			{
				*(pGradBits+(y*w)+x)=PalVal[l+MulDiv((r-l),x,w)];
			}
		}
	}

	HBITMAP hBmp = CreateDIBitmap(hDC,&GradBitInfo.bmiHeader,CBM_INIT,
						pGradBits,&GradBitInfo,DIB_RGB_COLORS);

	free(pGradBits);

	return hBmp;
}

//static member for keyboard operation, you can used it in you parent window
//it work with shortcut key
LRESULT CMenuXP::OnMenuChar(UINT nChar, UINT /*nFlags*/, CMenu* pMenu) 
{
	UINT iCurrentItem = (UINT)-1; // guaranteed higher than any command ID
	CUIntArray arItemsMatched;		// items that match the character typed

	UINT nItem = pMenu->GetMenuItemCount();
	UINT i;
	for (i=0; i< nItem; i++) 
	{
		MENUITEMINFO	info;
		ZeroMemory(&info, sizeof(info));
		info.cbSize = sizeof(info);
		info.fMask = MIIM_DATA | MIIM_TYPE | MIIM_STATE;
		::GetMenuItemInfo(*pMenu, i, TRUE, &info);

		CMenuXPItem	*pData = (CMenuXPItem *)info.dwItemData;
		if ((info.fType & MFT_OWNERDRAW) && pData && pData->IsMyData())
		{
			CString	text = pData->m_strText;
			int iAmpersand = text.Find('&');
			if (iAmpersand >=0 && toupper(nChar)==toupper(text[iAmpersand+1]))
				arItemsMatched.Add(i);
		}
		if (info.fState & MFS_HILITE)
			iCurrentItem = i; // note index of current item
	}
	

	// arItemsMatched now contains indexes of items that match the char typed.
	//
	//   * if none: beep
	//   * if one:  execute it
	//   * if more than one: hilite next
	//
	UINT nFound = arItemsMatched.GetSize();
	if (nFound == 0)
		return 0;

	else if (nFound==1)
		return MAKELONG(arItemsMatched[0], MNC_EXECUTE);

	// more than one found--return 1st one past current selected item;
	UINT iSelect = 0;
	for (i=0; i < nFound; i++) {
		if (arItemsMatched[i] > iCurrentItem) {
			iSelect = i;
			break;
		}
	}
	return MAKELONG(arItemsMatched[iSelect], MNC_SELECT);
}

void CMenuXP::DrawMenuText(CDC& dc, CRect rc, CString text,
	COLORREF color)
{
	CString left = text;
	CString right;
	int iTabPos = left.Find('\t');
	if (iTabPos >= 0) {
		right = left.Right(left.GetLength() - iTabPos - 1);
		left  = left.Left(iTabPos);
	}
	dc.SetTextColor(color);
	dc.DrawText(left, &rc, DT_MYSTANDARD);
	if (iTabPos > 0)
		dc.DrawText(right, &rc, DT_MYSTANDARD|DT_RIGHT);
}

//find a popupmenu from a menuitem id
CMenuXP *CMenuXP::FindSubMenuFromID(DWORD dwID)
{
	CMenuXP	*pSubMenu;
	CMenuXP	*pResult;
	UINT i;
	for (i=0; i<GetMenuItemCount(); i++)
	{
		if (GetMenuItemID(i) == dwID)
				return this;
	}

	for (i=0; i<GetMenuItemCount(); i++)
	{
		pSubMenu = (CMenuXP *)GetSubMenu(i);
		if (pSubMenu)
		{
			pResult = pSubMenu->FindSubMenuFromID(dwID);
			if (pResult)
				return pResult;
		}
	}

	return NULL;
}

// add sidebar and background
void CMenuXP::AddMenuTitle(LPCTSTR lpszTitle, bool /*bIsIconMenu*/, bool bIsSidebar)
{
	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	if (thePrefs.GetXPMenuStyle() == 0)
		SetMenuStyle(CMenuXP::STYLE_OFFICE);
	else if (thePrefs.GetXPMenuStyle() == 1)
		SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	else
		SetMenuStyle(CMenuXP::STYLE_XP);

	if(thePrefs.UseGrayMenuIcons())
		SetEmbossedIcons(FALSE);

	if(thePrefs.GetXPSideBar()){
		AddSideBar(new CMenuXPSideBar(17, theApp.GetAppTitle()));
		COLORREF startColor = RGB(10,150,203);
		theApp.LoadSkinColorAlt(_T("MenuLvStart"), _T("XPMenuLvStart"), startColor);
		COLORREF endColor = RGB(85,213,250);
		theApp.LoadSkinColorAlt(_T("MenuLvEnd"), _T("EndMenuLvEnd"), endColor);
		SetSideBarStartColor(startColor);
		SetSideBarEndColor(endColor);
	}

	if(thePrefs.GetXPBitmap()){
		SetBackBitmap(_T("MENUBACK"), _T("JPG"));
		COLORREF color = RGB(0,93,126);
		theApp.LoadSkinColorAlt(_T("MenuLvMark"), _T("XPMenuLvMark"), color);
		SetSelectedBarColor(color);
	}else{
		SetBackColor(::GetSysColor(COLOR_MENU));
		SetSelectedBarColor(::GetSysColor(29/*COLOR_MENUBAR*/ /*30 COLOR_MENUHILIGHT*//*));
	}
	*/
	SetMenuStyle(CMenuXP::STYLE_STARTMENU);

	if(bIsSidebar)
		AddSideBar(new CMenuXPSideBar(17, theApp.m_strModLongVersion));
        if(lpszTitle)
		AppendHeading(lpszTitle);

	SetBackBitmap(_T("MENUBACK"), _T("JPG"));
	// ==> Mephisto mod [Stulle] - Mephisto
	/*
	SetSideBarStartColor(RGB(255,194,189));
	SetSideBarEndColor(RGB(237,237,237));
	SetSelectedBarColor(RGB(252,231,211));
	SetTextColor(RGB(0,0,0));
	SetSelectedTextColor(RGB(0,0,0));
	SetHeadingTextColor(RGB(0,0,0)); // MenuXP Sub Heading [fafner] - MyTh
	*/
	SetSideBarStartColor(RGB(255,0,0));
	SetSideBarEndColor(RGB(0,0,0));
	SetSelectedBarColor(RGB(255,125,125));
	SetTextColor(RGB(0,0,0));
	SetSelectedTextColor(RGB(0,0,0));
	SetHeadingTextColor(RGB(255,255,255));  // MenuXP Sub Heading [fafner] - MyTh
	// <== Mephisto mod [Stulle] - Mephisto
	// <== XP Style Menu [Xanatos] - Stulle
}

// add menu item on the end
BOOL CMenuXP::AppendMenu(UINT nFlags, UINT_PTR nIDNewItem, LPCTSTR lpszNewItem, LPCTSTR lpszIconName)
{
	if(nFlags & MF_SEPARATOR)
		return AppendSeparator();
	return AppendODMenu(nFlags, nIDNewItem, new CMenuXPText((nFlags & MF_POPUP) ? 0 : nIDNewItem, lpszNewItem, lpszIconName ? theApp.LoadIcon(lpszIconName, 16, 16) : NULL));
}

// insert a menu item
BOOL CMenuXP::InsertMenu(UINT nPosition, UINT nFlags, UINT_PTR nIDNewItem, LPCTSTR lpszNewItem, LPCTSTR lpszIconName)
{
	if(nFlags & MF_SEPARATOR)
		return AppendSeparator();
	return AppendODMenu(nFlags, nIDNewItem, new CMenuXPText((nFlags & MF_POPUP) ? 0 : nIDNewItem, lpszNewItem, lpszIconName ? theApp.LoadIcon(lpszIconName, 16, 16) : NULL), nPosition);
}

BOOL CMenuXP::RemoveMenu(UINT nPosition, UINT nFlags)
{
	MENUITEMINFO	info;
	ZeroMemory(&info, sizeof(MENUITEMINFO));
	info.cbSize = sizeof(MENUITEMINFO);
	info.fMask = MIIM_DATA | MIIM_TYPE;
	// ==> mem leak fix [fafner] - Stulle
	/*
	GetMenuItemInfo(nPosition, &info, TRUE);
	*/
	GetMenuItemInfo(nPosition, &info, nFlags & MF_BYPOSITION);
	// <== mem leak fix [fafner] - Stulle
	
	CMenuXPItem *pData = (CMenuXPItem *)info.dwItemData;
	if ((info.fType & MFT_OWNERDRAW) && pData && pData->IsMyData())
		delete pData;

	return CMenu::RemoveMenu(nPosition,nFlags);
}

// change icon
void CMenuXP::ChangeMenuIcon(UINT_PTR nIDNewItem, LPCTSTR /*lpszNewItem*/, LPCTSTR lpszIconName)
{
	MENUITEMINFO	info;
	ZeroMemory(&info, sizeof(MENUITEMINFO));
	info.cbSize = sizeof(MENUITEMINFO);
	info.fMask = MIIM_DATA | MIIM_TYPE;
	GetMenuItemInfo(nIDNewItem, &info);
	
	CMenuXPItem *pData = (CMenuXPItem *)info.dwItemData;
	if ((info.fType & MFT_OWNERDRAW) && pData && pData->IsMyData())
	{
		if (pData->m_hIcon)
			::DestroyIcon(pData->m_hIcon);
		pData->m_hIcon = (/*lpszNewItem, */lpszIconName ? theApp.LoadIcon(lpszIconName, 16, 16) : NULL);
	}
}

//Add a gradient sidebar, it must be the first item in a popupmenu
BOOL CMenuXP::AddSideBar(CMenuXPSideBar *pItem)
{
	ASSERT(pItem);

	m_bBreak = TRUE;
	m_bBreakBar = FALSE;

	return CMenu::AppendMenu(MF_OWNERDRAW | MF_SEPARATOR, pItem->m_dwID, (LPCTSTR)pItem);
}

//add a normal menuitem, an accelerator key could be specified, and the accel text will
//be added automatically
BOOL CMenuXP::AppendODMenu(UINT nFlags, UINT_PTR nIDNewItem, CMenuXPItem* pItem, UINT nPosition, ACCEL *pAccel)
{
	//TRACE("XPMenu Created adress 0x0%x, Name %s\n",pItem,CStringA(pItem->m_strText));
	//TRACE(" - N 0x0%x\n\n",pItem);
	ASSERT(pItem);

	nFlags |= MF_OWNERDRAW;
	if (m_bBreak) 
		nFlags |= MF_MENUBREAK;
	if (m_bBreakBar)
		nFlags |= MF_MENUBARBREAK;
	m_bBreak = m_bBreakBar = FALSE;

	if (pAccel)
	{
		CBCGKeyHelper	keyhelper(pAccel);
		CString	strAccel;
		keyhelper.Format(strAccel);
		if (strAccel.GetLength()>0)
		{
			pItem->m_strText += _T("\t");
			pItem->m_strText += strAccel;
		}
	}

	if(nPosition != UINT_MAX)
		return CMenu::InsertMenu(nPosition, nFlags, nIDNewItem, (LPCTSTR)pItem);
	return CMenu::AppendMenu(nFlags, nIDNewItem, (LPCTSTR)pItem);
}

//Add a separator line
BOOL CMenuXP::AppendSeparator(void)	
{
	m_bBreak = m_bBreakBar = FALSE;

	CMenuXPSeparator *pItem = new CMenuXPSeparator;

	return CMenu::AppendMenu(MF_OWNERDRAW | MF_SEPARATOR, 0, (LPCTSTR)pItem);
}

// ==> MenuXP Sub Heading [fafner] - MyTh
BOOL CMenuXP::AppendHeading(LPCTSTR lpszTitle)	
{
	// Note: We need to add a break if we want to add this item on the top! [Stulle]
	UINT nFlags = MF_OWNERDRAW | MF_DISABLED;
	if (m_bBreak) 
		nFlags |= MF_MENUBREAK;
	if (m_bBreakBar)
		nFlags |= MF_MENUBARBREAK;
	m_bBreak = m_bBreakBar = FALSE;

	CMenuXPHeading *pItem = new CMenuXPHeading(lpszTitle);

	return CMenu::AppendMenu(nFlags, 0, (LPCTSTR)pItem);
}
// <== MenuXP Sub Heading [fafner] - MyTh

//add a popup menu
/*void CMenuXP::AppendODPopup(UINT &nFlags, CMenuXPItem* pItem)
{
	//TRACE(" - P 0x0%x\n\n",pItem);
	ASSERT(pItem);

	nFlags |= MF_OWNERDRAW;
	nFlags |= MF_POPUP;
	if (m_bBreak) 
		nFlags |= MF_MENUBREAK;
	if (m_bBreakBar)
		nFlags |= MF_MENUBARBREAK;
	m_bBreak = m_bBreakBar = FALSE;
}*/

//Change column, the next item added will be in the next column
void CMenuXP::Break(void)
{
	m_bBreak = TRUE;
}

//same as Break(), except that a break line will appear between the two columns
void CMenuXP::BreakBar(void)	
{
	m_bBreakBar = TRUE;
}

//Set background bitmap, null to remove
void CMenuXP::SetBackBitmap(HBITMAP hBmp)
{
	if (hBmp == NULL && m_hBitmap)
	{
		::DeleteObject(m_hBitmap);
		m_hBitmap = NULL;
		m_memDC.DeleteDC();
		return;
	}
	m_hBitmap = hBmp;
	if (!m_memDC.m_hDC)
	{
		CWindowDC	dc(NULL);
		m_memDC.CreateCompatibleDC(&dc);
	}

	ASSERT(m_memDC.m_hDC);

	::SelectObject(m_memDC.m_hDC, m_hBitmap);

}

void CMenuXP::SetBackBitmap(LPCTSTR lpszResourceName, LPCTSTR pszResourceType)
{
	CBitmap bmp;
	HBITMAP hBitmap = NULL;
	if (bmp.Attach(theApp.LoadImage(lpszResourceName,pszResourceType)))
		hBitmap = (HBITMAP)bmp.Detach();
	if(hBitmap != NULL)
		SetBackBitmap(hBitmap);
}