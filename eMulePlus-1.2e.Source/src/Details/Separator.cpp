/////////////////////////////////////////////////////////////////////////////
// Separator.cpp : source file for CSeparator.
//
// Written by Michael Dunn (mdunn at inreach dot com).  Copyright and all
// that stuff.  Use however you like but give me credit where it's due.  I'll
// know if you don't. bwa ha ha ha ha!
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Separator.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSeparator

CSeparator::CSeparator()
{
}

CSeparator::~CSeparator()
{
}


BEGIN_MESSAGE_MAP(CSeparator, CStatic)
	//{{AFX_MSG_MAP(CSeparator)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSeparator message handlers

// This function draws the horizontal line, followed by the text. The general
// idea for the line drawing comes from Hans Buehler's article at
// http://www.codeguru.com/staticctrl/draw_bevel.shtml
//
// Note that this class is intended to be just a simple separator, so the
// only static styles I implement here are SS_LEFT, SS_CENTER, SS_RIGHT, and
// SS_NOPREFIX.  Also note that the line is always drawn vertically centered
// in the control, so if you make the control tall enough you can get the 
// text totally above the line.

void CSeparator::OnPaint() 
{
	CPaintDC dc(this);                      // device context for painting
	CRect    rectWnd;                       // RECT of the static control
	CRect    rectText;                      // RECT in which text will be drawn
	CString  cstrText;                      // text to draw
	CFont*   pfontOld;
	DWORD    dwStyle = GetStyle();          // this window's style

	// Get the screen & client coords.
	GetWindowRect ( rectWnd );
	GetClientRect ( rectText );

	// Get the text to be drawn.
	GetWindowText ( cstrText );

	// If the control is taller than it is wide, draw a vertical line.
	// No text is drawn in this case.
	if ( rectWnd.Height() > rectWnd.Width() )
	{
		dc.Draw3dRect( 
			rectWnd.Width()/2, 0,       // upper-left point
			2, rectWnd.Height(),        // width & height  
			::GetSysColor(COLOR_3DSHADOW),
			::GetSysColor(COLOR_3DHIGHLIGHT) );

		/************************************************************
		If you feel adventurous, here is the code to do vertical text
		drawing.  I have it commented out because getting the text
		aligned correctly over the line looks like it'll be a pain.

		// Start by getting this control's font, and then tweak it a bit.

		LOGFONT lf;
		GetFont()->GetObject ( sizeof(LOGFONT), &lf );

		// Text will be rotated counter-clockwise 90 degrees.
		lf.lfOrientation = lf.lfEscapement = 900;

		// We need a TrueType font to get rotated text.  MS Sans Serif
		// won't cut it!
		lstrcpy ( lf.lfFaceName, _T("Tahoma") );

		// Create a font with the tweaked attributes.
		CFont font;
		font.CreateFontIndirect ( &lf );

		pfontOld = dc.SelectObject ( &font );

		dc.SetBkColor ( ::GetSysColor(COLOR_BTNFACE) );

		GetWindowText ( cstrText );

		dc.DrawText ( cstrText, rectText,
		DT_VCENTER | DT_CENTER | DT_SINGLELINE );

		dc.SelectObject ( pfontOld );
		font.DeleteObject();
		************************************************************/
	}
	else
	{

		// Get the font for the text.
		pfontOld = dc.SelectObject ( GetFont() );

		SIZE TextSize;
		GetTextExtentPoint32(dc, cstrText, cstrText.GetLength(), &TextSize);

		// We're drawing a horizontal separator.
		// The text will always be at the top of the control.  Also check
		// if the SS_NOPREFIX style is set.                                        

		// Voila!  One 3D line coming up.

		UINT uFormat = DT_TOP | DT_NOCLIP;

		if ( dwStyle & SS_NOPREFIX )
			uFormat |= DT_NOPREFIX;

		if ( dwStyle & SS_RIGHT )
		{
			uFormat |= DT_RIGHT;
			dc.Draw3dRect (0, rectWnd.Height() / 2, rectWnd.Width() - TextSize.cx - 10, 2,
				::GetSysColor(COLOR_3DSHADOW),
				::GetSysColor(COLOR_3DHIGHLIGHT) );
		}
		else if ( dwStyle & SS_CENTER )
		{
			uFormat |= DT_CENTER;
			dc.Draw3dRect (0, rectWnd.Height() / 2, ((rectWnd.Width() - TextSize.cx) / 2) - 10, 2,
				::GetSysColor(COLOR_3DSHADOW),
				::GetSysColor(COLOR_3DHIGHLIGHT) );
			dc.Draw3dRect (((rectWnd.Width() - TextSize.cx) / 2) + TextSize.cx + 10, rectWnd.Height() / 2, ((rectWnd.Width() - TextSize.cx) / 2) - 10, 2,
				::GetSysColor(COLOR_3DSHADOW),
				::GetSysColor(COLOR_3DHIGHLIGHT) );
		}
		else
		{
			uFormat |= DT_LEFT;
			dc.Draw3dRect (TextSize.cx + 10, rectWnd.Height() / 2,	rectWnd.Width() - TextSize.cx - 10, 2,
				::GetSysColor(COLOR_3DSHADOW),
				::GetSysColor(COLOR_3DHIGHLIGHT) );
		}

		dc.SetBkMode(TRANSPARENT);
		dc.DrawText ( cstrText, rectText, uFormat );

		// Clean up GDI objects like a good lil' programmer.
		dc.SelectObject ( pfontOld );
	}
}
