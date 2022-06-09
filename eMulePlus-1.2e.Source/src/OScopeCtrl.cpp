// OScopeCtrl.cpp : implementation file

#include "stdafx.h"
#include "emule.h"
#include "math.h"

#include "OScopeCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COScopeCtrl
COScopeCtrl::COScopeCtrl(int NTrends)
{
	EMULE_TRY

	int i;
	static const COLORREF aPresetColor[16] =
	{
		RGB(0xFF, 0x00, 0x00),
		RGB(0xFF, 0xC0, 0xC0),

		RGB(0xFF, 0xFF, 0x00),
		RGB(0xFF, 0xA0, 0x00),
		RGB(0xA0, 0x60, 0x00),

		RGB(0x00, 0xFF, 0x00),
		RGB(0x00, 0xA0, 0x00),

		RGB(0x00, 0x00, 0xFF),
		RGB(0x00, 0xA0, 0xFF),
		RGB(0x00, 0xFF, 0xFF),
		RGB(0x00, 0xA0, 0xA0),

		RGB(0xC0, 0xC0, 0xFF),
		RGB(0xFF, 0x00, 0xFF),
		RGB(0xA0, 0x00, 0xA0),

		RGB(0xFF, 0xFF, 0xFF),
		RGB(0x80, 0x80, 0x80)
	};
	// since plotting is based on a LineTo for each new point
	// we need a starting point (i.e. a "previous" point)
	// use 0.0 as the default first point.
	// these are public member variables, and can be changed outside
	// (after construction).
	// G.Hayduk: NTrends is the number of trends that will be drawn on
	// the plot. First 15 plots have predefined colors, but others will
	// be drawn with white, unless you call SetPlotColor
	m_PlotData = new PlotData_t[NTrends];
	m_NTrends = NTrends;

	for(i = 0; i < m_NTrends; i++)
	{
		if(i < 15)
			m_PlotData[i].crPlotColor = aPresetColor[i];  // see also SetPlotColor
		else
			m_PlotData[i].crPlotColor  = RGB(255, 255, 255);  // see also SetPlotColor
		m_PlotData[i].penPlot.CreatePen(PS_SOLID, 0, m_PlotData[i].crPlotColor);
		m_PlotData[i].dPreviousPosition = 0.0;
		m_PlotData[i].nPrevY = -1;
		m_PlotData[i].dLowerLimit = -10.0;
		m_PlotData[i].dUpperLimit =  10.0;
		m_PlotData[i].dRange      =   m_PlotData[i].dUpperLimit -
			m_PlotData[i].dLowerLimit;   // protected member variable
		m_PlotData[i].lstPoints.AddTail(0.0);
	}

	// public variable for the number of decimal places on the y axis
	// G.Hayduk: I've deleted the possibility of changing this parameter
	// in SetRange, so change it after constructing the plot
	m_nYDecimals = 1;

	// set some initial values for the scaling until "SetRange" is called.
	// these are protected varaibles and must be set with SetRange
	// in order to ensure that m_dRange is updated accordingly

	// m_nShiftPixels determines how much the plot shifts (in terms of pixels)
	// with the addition of a new data point
	autofitYscale = false;
	m_nShiftPixels = 1;
	m_nTrendPoints = 0;
	m_nMaxPointCnt = 600;
	CustShift.m_nPointsToDo = 0;
	// G.Hayduk: actually I needed an OScopeCtrl to draw specific number of
	// data samples and stretch them on the plot ctrl. Now, OScopeCtrl has
	// two modes of operation: fixed Shift (when m_nTrendPoints=0,
	// m_nShiftPixels is in use), or fixed number of Points in the plot width
	// (when m_nTrendPoints>0)
	// When m_nTrendPoints>0, CustShift structure is in use

	// background, grid and data colors
	// these are public variables and can be set directly
	m_crBackColor  = RGB(0,   0,   0);  // see also SetBackgroundColor
	m_crGridColor  = RGB(0, 255, 255);  // see also SetGridColor

	// public member variables, can be set directly
	m_str.XUnits.Format(_T("Samples"));  // can also be set with SetXUnits
	m_str.YUnits.Format(_T("Y units"));  // can also be set with SetYUnits

	// protected bitmaps to restore the memory DC's
	m_pbitmapOldGrid = NULL;
	m_pbitmapOldPlot = NULL;

	// G.Hayduk: configurable number of grids init
	// you are free to change those between contructing the object
	// and calling Create
	m_nXGrids = 6;
	m_nYGrids = 5;
	m_nTrendPoints = -1;

	m_bDoUpdate = true;
	m_nRedrawTimer = 0;

	m_oldcx = 0;
	m_oldcy = 0;

	EMULE_CATCH2
}  // COScopeCtrl

/////////////////////////////////////////////////////////////////////////////
COScopeCtrl::~COScopeCtrl()
{
	EMULE_TRY

	// just to be picky restore the bitmaps for the two memory dc's
	// (these dc's are being destroyed so there shouldn't be any leaks)
	if (m_pbitmapOldGrid != NULL)
		m_dcGrid.SelectObject(m_pbitmapOldGrid);
	if (m_pbitmapOldPlot != NULL)
		m_dcPlot.SelectObject(m_pbitmapOldPlot);
	delete[] m_PlotData;
	// G.Hayduk: If anyone notices that I'm not freeing or deleting
	// something, please let me know: hayduk@hello.to

	EMULE_CATCH2
} // ~COScopeCtrl


BEGIN_MESSAGE_MAP(COScopeCtrl, CWnd)
	//{{AFX_MSG_MAP(COScopeCtrl)
	ON_WM_PAINT()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_WM_TIMER()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COScopeCtrl message handlers

/////////////////////////////////////////////////////////////////////////////
BOOL COScopeCtrl::Create(DWORD dwStyle, const RECT& rect,
		                     CWnd* pParentWnd, UINT nID)
{
	BOOL result;

	EMULE_TRY

	static CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW);

	result = CWnd::CreateEx(WS_EX_CLIENTEDGE | WS_EX_STATICEDGE,
		className, NULL, dwStyle,
		rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
		pParentWnd->GetSafeHwnd(), (HMENU)nID);
	if(result != 0)
		InvalidateCtrl();

	EMULE_CATCH

	return result;
} // Create

/////////////////////////////////////////////////////////////////////////////
void COScopeCtrl::SetRange(double dLower, double dUpper, int iTrend)
{
	EMULE_TRY

	ASSERT(dUpper > dLower);

	m_PlotData[iTrend].dLowerLimit     = dLower;
	m_PlotData[iTrend].dUpperLimit     = dUpper;
	m_PlotData[iTrend].dRange          = m_PlotData[iTrend].dUpperLimit - m_PlotData[iTrend].dLowerLimit;
	m_PlotData[iTrend].dVerticalFactor = (double)m_nPlotHeight / m_PlotData[iTrend].dRange;

	// clear out the existing garbage, re-start with a clean plot
	InvalidateCtrl();

	EMULE_CATCH
}  // SetRange

void COScopeCtrl::SetRanges(double dLower, double dUpper)
{
	EMULE_TRY

	int iTrend;
	ASSERT(dUpper > dLower);

	for(iTrend = 0; iTrend < m_NTrends; iTrend ++)
	{
		m_PlotData[iTrend].dLowerLimit     = dLower;
		m_PlotData[iTrend].dUpperLimit     = dUpper;
		m_PlotData[iTrend].dRange          = m_PlotData[iTrend].dUpperLimit - m_PlotData[iTrend].dLowerLimit;
		m_PlotData[iTrend].dVerticalFactor = (double)m_nPlotHeight / m_PlotData[iTrend].dRange;
	}

	// clear out the existing garbage, re-start with a clean plot
	InvalidateCtrl();

	EMULE_CATCH
}  // SetRange
/////////////////////////////////////////////////////////////////////////////
// G.Hayduk: Apart from setting title of axis, now you can optionally set
// the limits strings
// (string which will be placed on the left and right of axis)
void COScopeCtrl::SetXUnits(CString string, CString XMin, CString XMax)
{
	m_str.XUnits = string;
	m_str.XMin = XMin;
	m_str.XMax = XMax;

	InvalidateCtrl(false);
}  // SetXUnits


/////////////////////////////////////////////////////////////////////////////
// G.Hayduk: Apart from setting title of axis, now you can optionally set
// the limits strings
// (string which will be placed on the bottom and top of axis)
void COScopeCtrl::SetYUnits(CString string, CString YMin, CString YMax)
{
	m_str.YUnits = string;
	m_str.YMin = YMin;
	m_str.YMax = YMax;

	// clear out the existing garbage, re-start with a clean plot
	InvalidateCtrl();
}  // SetYUnits

/////////////////////////////////////////////////////////////////////////////
void COScopeCtrl::SetGridColor(COLORREF color)
{
	m_crGridColor = color;

	// clear out the existing garbage, re-start with a clean plot
	InvalidateCtrl();
}  // SetGridColor


/////////////////////////////////////////////////////////////////////////////
void COScopeCtrl::SetPlotColor(COLORREF color, int iTrend)
{
	EMULE_TRY

//	Black is used as a masked background, modify plot color if it's black
	if (color == RGB(0, 0, 0))
		color++;
	m_PlotData[iTrend].crPlotColor = color;

	m_PlotData[iTrend].penPlot.DeleteObject();
	m_PlotData[iTrend].penPlot.CreatePen(PS_SOLID, 0, m_PlotData[iTrend].crPlotColor);

	// clear out the existing garbage, re-start with a clean plot
	//	InvalidateCtrl() ;

	EMULE_CATCH
}  // SetPlotColor

COLORREF COScopeCtrl::GetPlotColor(int iTrend)
{
	EMULE_TRY

	return m_PlotData[iTrend].crPlotColor;

	EMULE_CATCH

	return RGB(0,0,0);
}  // GetPlotColor

/////////////////////////////////////////////////////////////////////////////
void COScopeCtrl::SetBackgroundColor(COLORREF color)
{
	m_crBackColor = color;

	// clear out the existing garbage, re-start with a clean plot
	InvalidateCtrl();
}  // SetBackgroundColor

/////////////////////////////////////////////////////////////////////////////
void COScopeCtrl::InvalidateCtrl(bool deleteGraph)
{
	EMULE_TRY

	// There is a lot of drawing going on here - particularly in terms of
	// drawing the grid.  Don't panic, this is all being drawn (only once)
	// to a bitmap.  The result is then BitBlt'd to the control whenever needed.
	int i, j, GridPos;
	int nCharacters;

	CPen *oldPen;
	CPen solidPen(PS_SOLID, 0, m_crGridColor);
	CFont axisFont, yUnitFont, *oldFont;
	CString strTemp;

	// in case we haven't established the memory dc's
	CClientDC dc(this);

	// if we don't have one yet, set up a memory dc for the grid
	if(m_dcGrid.GetSafeHdc() == NULL)
	{
		m_dcGrid.CreateCompatibleDC(&dc);
		m_bitmapGrid.DeleteObject();
		m_bitmapGrid.CreateCompatibleBitmap(&dc, m_nClientWidth, m_nClientHeight);
		m_pbitmapOldGrid = m_dcGrid.SelectObject(&m_bitmapGrid);
	}

//	Fill the grid background (background color, previously set using SetBkColor, is changed inside)
	m_dcGrid.FillSolidRect(m_rectClient, m_crBackColor);

	// draw the plot rectangle:
	// determine how wide the y axis scaling values are
	nCharacters = abs((int)log10(fabs(m_PlotData[0].dUpperLimit)));

	// add the units digit, decimal point and a minus sign, and an extra space
	// as well as the number of decimal places to display
	nCharacters = nCharacters + 4 + m_nYDecimals;

	// adjust the plot rectangle dimensions
	// assume 6 pixels per character (this may need to be adjusted)
	m_rectPlot.left = m_rectClient.left + 8*7+4;//(nCharacters) ; // DonGato 8 was 6
	m_nPlotWidth    = m_rectPlot.Width();

	// draw the plot rectangle
	oldPen = m_dcGrid.SelectObject(&solidPen);
	m_dcGrid.MoveTo(m_rectPlot.left, m_rectPlot.top);
	m_dcGrid.LineTo(m_rectPlot.right + 1, m_rectPlot.top);
	m_dcGrid.LineTo(m_rectPlot.right + 1, m_rectPlot.bottom + 1);
	m_dcGrid.LineTo(m_rectPlot.left, m_rectPlot.bottom + 1);
	m_dcGrid.LineTo(m_rectPlot.left, m_rectPlot.top);
	m_dcGrid.SelectObject(oldPen);

	// draw the dotted lines,
	// use SetPixel instead of a dotted pen - this allows for a
	// finer dotted line and a more "technical" look
	// G.Hayduk: added configurable number of grids
	for(j = 1; j < (m_nYGrids + 1); j++)
	{
		GridPos = m_rectPlot.Height()*j/ (m_nYGrids + 1) + m_rectPlot.top;
		for(i = m_rectPlot.left; i < m_rectPlot.right; i += 4)
			m_dcGrid.SetPixel(i, GridPos, m_crGridColor);
	}

	// create some fonts (horizontal and vertical)
	// use a height of 14 pixels and 300 weight
	// (these may need to be adjusted depending on the display)
	axisFont.CreateFont(14, 0, 0, 0, FW_LIGHT,
		FALSE, FALSE, 0, DEFAULT_CHARSET, // EC
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, _T("Arial"));
	yUnitFont.CreateFont(14, 0, 900, 0, FW_LIGHT,
		FALSE, FALSE, 0, DEFAULT_CHARSET, // EC
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, _T("Arial"));

	// grab the horizontal font
	oldFont = m_dcGrid.SelectObject(&axisFont);

	// y max
	m_dcGrid.SetTextColor(m_crGridColor);
	m_dcGrid.SetTextAlign(TA_RIGHT | TA_TOP);
	if(m_str.YMax.IsEmpty())
		strTemp.Format(_T("%.*lf"), m_nYDecimals, m_PlotData[0].dUpperLimit);
	else
		strTemp = m_str.YMax;
	m_dcGrid.TextOut(m_rectPlot.left - 4, m_rectPlot.top - 7, strTemp);

	// y/2
	strTemp.Format(_T("%.*lf"), m_nYDecimals, m_PlotData[0].dUpperLimit / 2);
	m_dcGrid.TextOut(m_rectPlot.left - 2, m_rectPlot.bottom+ ((m_rectPlot.top - m_rectPlot.bottom)/2) - 7 , strTemp);


	// y min
	m_dcGrid.SetTextAlign(TA_RIGHT | TA_BASELINE);
	if(m_str.YMin.IsEmpty())
		strTemp.Format(_T("%.*lf"), m_nYDecimals, m_PlotData[0].dLowerLimit);
	else
		strTemp = m_str.YMin;
	m_dcGrid.TextOut(m_rectPlot.left - 4, m_rectPlot.bottom, strTemp);
	/*
	// x min
	m_dcGrid.SetTextAlign(TA_LEFT | TA_TOP);
	if(m_str.XMin.IsEmpty())
	m_dcGrid.TextOut(m_rectPlot.left, m_rectPlot.bottom + 4, "0");
	else
	m_dcGrid.TextOut(m_rectPlot.left, m_rectPlot.bottom + 4, (LPCTSTR)m_str.XMin);

	  // x max
	  m_dcGrid.SetTextAlign(TA_RIGHT | TA_TOP);
	  if(m_str.XMax.IsEmpty())
	  {
	  if(m_nTrendPoints < 0)
	  strTemp.Format("%d", m_nPlotWidth/m_nShiftPixels);
	  else
	  strTemp.Format("%d", m_nTrendPoints - 1);
	  }
	  else
	  strTemp = m_str.XMax;
	  m_dcGrid.TextOut(m_rectPlot.right, m_rectPlot.bottom + 4, strTemp);
	*/
	// x units
	m_dcGrid.SetTextAlign(TA_CENTER | TA_TOP);
	m_dcGrid.TextOut((m_rectPlot.left + m_rectPlot.right)/2,
		m_rectPlot.bottom + 4, m_str.XUnits);

	// restore the font
	m_dcGrid.SelectObject(oldFont);

	// y units
	oldFont = m_dcGrid.SelectObject(&yUnitFont);
	m_dcGrid.SetTextAlign(TA_CENTER | TA_BASELINE);

	CRect rText(0,0,0,0);
	m_dcGrid.DrawText(m_str.YUnits, rText, DT_CALCRECT);
	m_dcGrid.TextOut((m_rectClient.left+m_rectPlot.left+4)/2-rText.Height() / 2,
		((m_rectPlot.bottom+m_rectPlot.top)/2)-rText.Height()/2, m_str.YUnits);
	m_dcGrid.SelectObject(oldFont);

	// at this point we are done filling the the grid bitmap,
	// no more drawing to this bitmap is needed until the setting are changed

	// if we don't have one yet, set up a memory dc for the plot
	if(m_dcPlot.GetSafeHdc() == NULL)
	{
		m_dcPlot.CreateCompatibleDC(&dc);
		m_bitmapPlot.DeleteObject();
		m_bitmapPlot.CreateCompatibleBitmap(&dc, m_nClientWidth, m_nClientHeight);
		m_pbitmapOldPlot = m_dcPlot.SelectObject(&m_bitmapPlot);
	//	Must be black to make possible fast bitmap masking
		m_dcPlot.SetBkColor(RGB(0, 0, 0));
	}

	// make sure the plot bitmap is cleared
	if(deleteGraph)
	{
	//	Fast way to fill a rectangular
		m_dcPlot.ExtTextOut(0, 0, ETO_OPAQUE, m_rectClient, NULL, 0, NULL);
	}

	int iNewSize = m_rectClient.Width() / m_nShiftPixels + 10;		// +10 just in case :)
	if(m_nMaxPointCnt < iNewSize)
		m_nMaxPointCnt = iNewSize;									// keep the bigest value
	m_bDoUpdate = false;

	if (g_App.m_pMDlg->IsRunning())
	{
		if (m_nRedrawTimer != 0)
			KillTimer(m_nRedrawTimer);
		m_nRedrawTimer = SetTimer(1612, 200, NULL);	// Reduce flickering
	}

	// finally, force the plot area to redraw
	InvalidateRect(m_rectClient);

	EMULE_CATCH
} // InvalidateCtrl


/////////////////////////////////////////////////////////////////////////////
// G.Hayduk: now, there are two methods: AppendPoints and AppendEmptyPoints
void COScopeCtrl::AppendPoints(double dNewPoint[], bool bInvalidate, bool bAdd2List)
{
	EMULE_TRY
	if (::IsWindow(m_hWnd)==false) return;
	int iTrend;

	// append a data point to the plot
	for(iTrend = 0; iTrend < m_NTrends; iTrend ++)
	{
		m_PlotData[iTrend].dCurrentPosition = dNewPoint[iTrend];
		if(bAdd2List)
		{
			m_PlotData[iTrend].lstPoints.AddTail(dNewPoint[iTrend]);
			while(m_PlotData[iTrend].lstPoints.GetCount() > m_nMaxPointCnt)
				m_PlotData[iTrend].lstPoints.RemoveHead();
		}
	}

	if(m_nTrendPoints > 0)
	{
		if(CustShift.m_nPointsToDo == 0)
		{
			CustShift.m_nPointsToDo = m_nTrendPoints - 1;
			CustShift.m_nWidthToDo = m_nPlotWidth;
			CustShift.m_nRmndr = 0;
		}

		// a little bit tricky setting m_nShiftPixels in "fixed number of points through plot width" mode
		m_nShiftPixels = (CustShift.m_nWidthToDo + CustShift.m_nRmndr) / CustShift.m_nPointsToDo;
		CustShift.m_nRmndr = (CustShift.m_nWidthToDo + CustShift.m_nRmndr) % CustShift.m_nPointsToDo;
		if(CustShift.m_nPointsToDo == 1)
			m_nShiftPixels = CustShift.m_nWidthToDo;
		CustShift.m_nWidthToDo -= m_nShiftPixels;
		CustShift.m_nPointsToDo--;
	}
	DrawPoint();

	if (bInvalidate && m_bDoUpdate && GetSafeHwnd())
		Invalidate();

	EMULE_CATCH
	return;
} // AppendPoint

/////////////////////////////////////////////////////////////////////////////
// G.Hayduk:
// AppendEmptyPoints adds a vector of data points, without drawing them
// (but shifting the plot), this way you can do a "hole" (space) in the plot
// i.e. indicating "no data here". When points are available, call AppendEmptyPoints
// for first valid vector of data points, and then call AppendPoints again and again
// for valid points

void COScopeCtrl::AppendEmptyPoints(double dNewPoint[], bool bInvalidate, bool bAdd2List)
{
	EMULE_TRY

	int iTrend, currY;
	// append a data point to the plot
	// return the previous point
	for(iTrend = 0; iTrend < m_NTrends; iTrend ++)
	{
		m_PlotData[iTrend].dCurrentPosition = dNewPoint[iTrend];
		if(bAdd2List)
			m_PlotData[iTrend].lstPoints.AddTail(dNewPoint[iTrend]);
	}
	if(m_nTrendPoints > 0)
	{
		if(CustShift.m_nPointsToDo == 0)
		{
			CustShift.m_nPointsToDo = m_nTrendPoints - 1;
			CustShift.m_nWidthToDo = m_nPlotWidth;
			CustShift.m_nRmndr = 0;
		}
		m_nShiftPixels = (CustShift.m_nWidthToDo + CustShift.m_nRmndr) / CustShift.m_nPointsToDo;
		CustShift.m_nRmndr = (CustShift.m_nWidthToDo + CustShift.m_nRmndr) % CustShift.m_nPointsToDo;
		if(CustShift.m_nPointsToDo == 1)
			m_nShiftPixels = CustShift.m_nWidthToDo;
		CustShift.m_nWidthToDo -= m_nShiftPixels;
		CustShift.m_nPointsToDo--;
	}

	// now comes DrawPoint's shift process

	if(m_dcPlot.GetSafeHdc() != NULL)
	{
		if (m_nShiftPixels > 0)
		{
			CRect	rectUpdatedRect = m_rectPlot;

			rectUpdatedRect.right++;
			m_dcPlot.ScrollDC(-m_nShiftPixels, 0, &rectUpdatedRect, &rectUpdatedRect, NULL, NULL);

		//	Establish a rectangle over the right side of plot
		//	which now needs to be cleaned up prior to adding the new point
			rectUpdatedRect.left = rectUpdatedRect.right - m_nShiftPixels;
		//	Fast way to fill the cleanup area with the background
			m_dcPlot.ExtTextOut(0, 0, ETO_OPAQUE, rectUpdatedRect, NULL, 0, NULL);
		}

		// draw the next line segment
		for(iTrend = 0; iTrend < m_NTrends; iTrend ++)
		{
			currY = m_rectPlot.bottom -
				(long)((m_PlotData[iTrend].dCurrentPosition - m_PlotData[iTrend].dLowerLimit) * m_PlotData[iTrend].dVerticalFactor);
			m_PlotData[iTrend].nPrevY = currY;

			// store the current point for connection to the next point
			m_PlotData[iTrend].dPreviousPosition = m_PlotData[iTrend].dCurrentPosition;
		}
	}

	// -----------------------------------------

	if(bInvalidate && m_bDoUpdate)
		Invalidate();

	EMULE_CATCH
	return;
} // AppendEmptyPoint

////////////////////////////////////////////////////////////////////////////
void COScopeCtrl::OnPaint()
{
	EMULE_TRY

	CPaintDC dc(this);  // device context for painting
	CDC memDC, dcMask;
	CBitmap memBitmap, maskBitmap;
	CBitmap *oldBitmap, *pOldMaskBitmap;

	// no real plotting work is performed here,
	// just putting the existing bitmaps on the client

	// to avoid flicker, establish a memory dc, draw to it
	// and then BitBlt it to the client
	memDC.CreateCompatibleDC(&dc);
	memBitmap.CreateCompatibleBitmap(&dc, m_nClientWidth, m_nClientHeight);
	oldBitmap = (CBitmap *)memDC.SelectObject(&memBitmap);

	dcMask.CreateCompatibleDC(&dc);
//	Monochrome DC
	maskBitmap.CreateBitmap(m_nClientWidth, m_nClientHeight, 1, 1, NULL);
	pOldMaskBitmap = dcMask.SelectObject(&maskBitmap);

	if ((memDC.GetSafeHdc() != NULL) && (dcMask.GetSafeHdc() != NULL))
	{
	//	Create the object mask by performing BitBlt from the source bitmap to a monochrome bitmap
	//	The background is already set to transparent color (black)
		dcMask.BitBlt(0, 0, m_nClientWidth, m_nClientHeight, &m_dcPlot, 0, 0, SRCCOPY);
	// First drop the grid on the memory dc
		memDC.BitBlt(0, 0, m_nClientWidth, m_nClientHeight, &m_dcGrid, 0, 0, SRCCOPY);
	//	Now add the plot on top as a "pattern"
		memDC.BitBlt(0, 0, m_nClientWidth, m_nClientHeight, &dcMask, 0, 0, SRCAND);
		memDC.BitBlt(0, 0, m_nClientWidth, m_nClientHeight, &m_dcPlot, 0, 0, SRCPAINT);
	//	Finally send the result to the display (only one on-screen copy to reduce flicker)
		dc.BitBlt(0, 0, m_nClientWidth, m_nClientHeight, &memDC, 0, 0, SRCCOPY);
	}
	memDC.SelectObject(oldBitmap);
	dcMask.SelectObject(pOldMaskBitmap);
	memBitmap.DeleteObject();
	maskBitmap.DeleteObject();

	EMULE_CATCH
} // OnPaint

/////////////////////////////////////////////////////////////////////////////
void COScopeCtrl::DrawPoint()
{
	EMULE_TRY

	// this does the work of "scrolling" the plot to the left
	// and appending a new data point all of the plotting is
	// directed to the memory based bitmap associated with m_dcPlot
	// the will subsequently be BitBlt'd to the client in OnPaint

	if (m_dcPlot.GetSafeHdc() != NULL)
	{
		if (m_nShiftPixels > 0)
		{
			CRect	rectUpdatedRect = m_rectPlot;

			rectUpdatedRect.right++;
			rectUpdatedRect.left++;
			m_dcPlot.ScrollDC(-m_nShiftPixels, 0, &rectUpdatedRect, &rectUpdatedRect, NULL, NULL);

		//	Establish a rectangle over the right side of plot
		//	which now needs to be cleaned up prior to adding the new point
			rectUpdatedRect.left = rectUpdatedRect.right - m_nShiftPixels;
		//	Fast way to fill the cleanup area with the background
			m_dcPlot.ExtTextOut(0, 0, ETO_OPAQUE, rectUpdatedRect, NULL, 0, NULL);
		}

		CPen	   *pOldPen = m_dcPlot.GetCurrentPen();
		int			currX, prevX, currY, prevY, iTrend;

		// draw the next line segment
		for(iTrend = 0; iTrend < m_NTrends; iTrend ++)
		{
			// grab the plotting pen
			m_dcPlot.SelectObject(&m_PlotData[iTrend].penPlot);

			// move to the previous point
			prevX = m_rectPlot.right - m_nShiftPixels;
			if(m_PlotData[iTrend].nPrevY > 0)
			{
				prevY = m_PlotData[iTrend].nPrevY;
			}
			else
			{
				prevY = m_rectPlot.bottom -
				(long)((m_PlotData[iTrend].dPreviousPosition - m_PlotData[iTrend].dLowerLimit) * m_PlotData[iTrend].dVerticalFactor);
			}
			m_dcPlot.MoveTo(prevX, prevY);

			// draw to the current point
			currX = m_rectPlot.right;
			currY = m_rectPlot.bottom -
				(long)((m_PlotData[iTrend].dCurrentPosition - m_PlotData[iTrend].dLowerLimit) * m_PlotData[iTrend].dVerticalFactor);
			m_PlotData[iTrend].nPrevY = currY;

			if (abs(prevX - currX) > abs(prevY - currY))
			{
				currX += (prevX - currX > 0) ? -1 : 1;
			}
			else
			{
				currY += (prevY - currY > 0) ? -1 : 1;
			}
			m_dcPlot.LineTo(currX, currY);

			// if the data leaks over the upper or lower plot boundaries
			// fill the upper and lower leakage with the background
			// this will facilitate clipping on an as needed basis
			// as opposed to always calling IntersectClipRect
			if((prevY <= m_rectPlot.top) || (currY <= m_rectPlot.top))
				m_dcPlot.ExtTextOut( 0, 0, ETO_OPAQUE,
					CRect(prevX - 1, m_rectClient.top, currX + 5, m_rectPlot.top + 1), NULL, 0, NULL );
			if((prevY >= m_rectPlot.bottom) || (currY >= m_rectPlot.bottom))
				m_dcPlot.ExtTextOut( 0, 0, ETO_OPAQUE,
					CRect(prevX - 1, m_rectPlot.bottom + 1, currX + 5, m_rectClient.bottom + 1), NULL, 0, NULL );

			// store the current point for connection to the next point
			m_PlotData[iTrend].dPreviousPosition = m_PlotData[iTrend].dCurrentPosition;
		}
		// restore the pen
		m_dcPlot.SelectObject(pOldPen);
	}

	EMULE_CATCH
} // end DrawPoint

/////////////////////////////////////////////////////////////////////////////
void COScopeCtrl::OnSize(UINT nType, int cx, int cy)
{
	EMULE_TRY

	CWnd::OnSize(nType, cx, cy);
	if ((!cx && !cy) || (cx==m_oldcx && cy==m_oldcy)) return;
	m_oldcx=cx;m_oldcy=cy;

	int iTrend;

	// NOTE: OnSize automatically gets called during the setup of the control

	GetClientRect(m_rectClient);

	// set some member variables to avoid multiple function calls
	m_nClientHeight = m_rectClient.Height();
	m_nClientWidth  = m_rectClient.Width();

	// the "left" coordinate and "width" will be modified in
	// InvalidateCtrl to be based on the width of the y axis scaling
	m_rectPlot.left   = 20;
	m_rectPlot.top    = 10;
	m_rectPlot.right  = m_rectClient.right - 10;
	m_rectPlot.bottom = m_rectClient.bottom - 25;

	// set some member variables to avoid multiple function calls
	m_nPlotHeight = m_rectPlot.Height();
	m_nPlotWidth  = m_rectPlot.Width();

	// set the scaling factor for now, this can be adjusted
	// in the SetRange functions
	for(iTrend = 0; iTrend < m_NTrends; iTrend ++)
		m_PlotData[iTrend].dVerticalFactor = (double)m_nPlotHeight / m_PlotData[iTrend].dRange;

	// destroy and recreate the grid bitmap
	CClientDC dc(this);
	if(m_pbitmapOldGrid && m_bitmapGrid.GetSafeHandle() && m_dcGrid.GetSafeHdc())
	{
		m_dcGrid.SelectObject(m_pbitmapOldGrid);
		m_bitmapGrid.DeleteObject();
		m_bitmapGrid.CreateCompatibleBitmap(&dc, m_nClientWidth, m_nClientHeight);
		m_pbitmapOldGrid = m_dcGrid.SelectObject(&m_bitmapGrid);
	}

	// destroy and recreate the plot bitmap
	if(m_pbitmapOldPlot && m_bitmapPlot.GetSafeHandle() && m_dcPlot.GetSafeHdc())
	{
		m_dcPlot.SelectObject(m_pbitmapOldPlot);
		m_bitmapPlot.DeleteObject();
		m_bitmapPlot.CreateCompatibleBitmap(&dc, m_nClientWidth, m_nClientHeight);
		m_pbitmapOldPlot = m_dcPlot.SelectObject(&m_bitmapPlot);
	}

	InvalidateCtrl();

	EMULE_CATCH
} // OnSize


/////////////////////////////////////////////////////////////////////////////
void COScopeCtrl::Reset()
{
	// to clear the existing data (in the form of a bitmap)
	// simply invalidate the entire control
	InvalidateCtrl();
}

int COScopeCtrl::ReCreateGraph(void)
{
	EMULE_TRY

	int i;
	for(i = 0; i < m_NTrends; i++)
	{
		m_PlotData[i].dPreviousPosition = 0.0;
		m_PlotData[i].nPrevY = -1;
	}

	double *pAddPoints = new double[m_NTrends];

	int iCnt = m_PlotData[0].lstPoints.GetCount();
	for(i = 0; i < iCnt; i++)
	{
		for(int iTrend = 0; iTrend < m_NTrends; iTrend++)
		{
			POSITION pos = m_PlotData[iTrend].lstPoints.FindIndex(i);
			if(pos)
				pAddPoints[iTrend] = m_PlotData[iTrend].lstPoints.GetAt(pos);
			else
				pAddPoints[iTrend] = 0;
		}
		AppendPoints(pAddPoints, false, false);
	}

	delete[] pAddPoints;

	Invalidate();	// Draw the new graph without waiting for the next AppendPoints()

	EMULE_CATCH

	return 0;
}

void COScopeCtrl::ReSizePlot(int iTrend,double scale)		//Cax2 - Resize one line of the graph, don't touch the other lines...
{
	EMULE_TRY

	m_PlotData[iTrend].dPreviousPosition = 0.0;
	m_PlotData[iTrend].nPrevY = -1;

	int iCnt = m_PlotData[0].lstPoints.GetCount();
	for(int i = 0; i < iCnt; i++)
	{
		POSITION pos = m_PlotData[iTrend].lstPoints.FindIndex(i);
		if(pos)
			m_PlotData[iTrend].lstPoints.SetAt(pos,m_PlotData[iTrend].lstPoints.GetAt(pos)*scale);
	}

	EMULE_CATCH
}

void COScopeCtrl::OnTimer(UINT nIDEvent)
{
	if (nIDEvent == m_nRedrawTimer)
	{
		KillTimer(m_nRedrawTimer);
		m_nRedrawTimer = 0;
		m_bDoUpdate = true;
		ReCreateGraph();
	}

	CWnd::OnTimer(nIDEvent);
}
