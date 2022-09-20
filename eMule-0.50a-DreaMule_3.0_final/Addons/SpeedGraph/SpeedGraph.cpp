// SpeedGraph.cpp
//

#include "stdafx.h"
#include "emule.h"
#include "SpeedGraph.h"
#include "MemDC.h"

#include <commctrl.h>
#include <intshcut.h>
#include <wininet.h> 

CSpeedGraph::CSpeedGraph()
{
	brushInitalized = FALSE;
	initalized = FALSE;
	m_ui_MaxAmount = 0.0;
	m_ui_MaxValue = 0.0;
	m_ui_MaxValueDelay = 0;
	interfaceCallBack = NULL;
	TrafficStats=NULL;
	gridxstartpos = 0;
	gridystartpos = 0;
	gridxresolution		=	GRIDXRESOLUTION;
	gridyresolution		=	GRIDYRESOLUTION;
	gridscrollxspeed	=	GRIDSCROLLXSPEED;
	gridscrollyspeed	=	GRIDSCROLLYSPEED; 
	plotgranularity		=	PLOTGRANULATRITY;
	Color1 = RGB(0,255,0);
	Color2 = RGB(255,0,0);
	ColorGrid = RGB(75, 75, 75);
	ColorBack = RGB(0, 0, 105);
	ColorText = RGB(255, 255, 255);
}

CSpeedGraph::~CSpeedGraph()
{
	if(TrafficStats)
		delete[] TrafficStats; 

}

BEGIN_MESSAGE_MAP(CSpeedGraph, CWnd)
	ON_WM_PAINT()
ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


void CSpeedGraph::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CDC * pDC   = CDC::FromHandle( dc );
	int nSavedDC = pDC -> SaveDC( );

	// Create the brush for the color bar
	if(brushInitalized == FALSE){
		CBitmap bmp;
		CMemDC *memDC = new CMemDC(pDC);
		
		RECT clipRect;
		memDC->GetClipBox(&clipRect);

		if(clipRect.right - clipRect.left > 1){
			bmp.CreateCompatibleBitmap(memDC,plotgranularity, TGSize.cy);
			CBitmap *pOld = memDC->SelectObject(&bmp);
			
			CSize bmps = bmp.GetBitmapDimension();		
			
			colorbrush.DeleteObject();

			// Need for scaling the color to the size of button
			BYTE r,g,b;
			for(int x = 0; x<TGSize.cy; x++){
				r = (BYTE)((GetRValue(Color1)*x + GetRValue(Color2)*(TGSize.cy-x))/TGSize.cy);
				g = (BYTE)((GetGValue(Color1)*x + GetGValue(Color2)*(TGSize.cy-x))/TGSize.cy);
				b = (BYTE)((GetBValue(Color1)*x + GetBValue(Color2)*(TGSize.cy-x))/TGSize.cy);

				memDC->SetPixelV(0,x,RGB(r,g,b));
				memDC->SetPixelV(1,x,RGB(r,g,b));
			}

			memDC->SelectObject(pOld);

			colorbrush.CreatePatternBrush(&bmp);	
			brushInitalized = TRUE;
		}

		delete memDC;
	}

	if(initalized == TRUE){
		COLORREF backcolor = GetSysColor(COLOR_BTNFACE);
		
		CBrush brush;
		CMemDC *memDC = new CMemDC(pDC);
		
		RECT clipRect;
		memDC->GetClipBox(&clipRect);
		memDC->FillSolidRect(&clipRect,backcolor);
		
		CFont *oldFont;
		int xp, yp, xx, yy;
		CPoint orgBrushOrigin = memDC->GetBrushOrg();
		
		oldFont = memDC->SelectObject(&smallFont);
		
		ASSERT(m_ui_MaxAmount);
		double scale = (double)TGSize.cy / m_ui_MaxAmount;
		
		yp = TrafficDrawRectangle.bottom;
		xp = TrafficDrawRectangle.left;
		
		RECT fillrect;
		
		// Fill the background
		brush.CreateSolidBrush(ColorBack);
		memDC->FillRect(&TrafficDrawRectangle, &brush);

		// draw the grid
		int xgridlines, ygridlines;
		
		xgridlines = TGSize.cx / gridxresolution;
		ygridlines = TGSize.cy / gridyresolution;
		CPen* oldPen = memDC->SelectObject(&GridPen);
		// Create the vertical lines
		for (int x=0; x<= xgridlines; x++){
			memDC->MoveTo(x*gridxresolution + gridxstartpos, 0			);
			memDC->LineTo(x*gridxresolution + gridxstartpos, TGSize.cy	);
		}
		// And the horizontal lines
		for (int y=0; y<= ygridlines; y++){
			memDC->MoveTo(0			, gridystartpos + TGSize.cy - y*gridyresolution - 2);
			memDC->LineTo(TGSize.cx	, gridystartpos + TGSize.cy - y*gridyresolution - 2);
		}

		gridxstartpos += gridscrollxspeed;
		gridystartpos += gridscrollyspeed;
		if(gridxstartpos < 0)
			gridxstartpos = gridxresolution;
		if(gridxstartpos > gridxresolution)
			gridxstartpos = 0;
		if(gridystartpos < 0)
			gridystartpos = gridyresolution;
		if(gridystartpos > gridyresolution)
			gridystartpos = 0;

		memDC->SelectObject(oldPen );

		for(DWORD cnt=0; cnt<TrafficEntries; cnt++){
			xx = xp + cnt*plotgranularity;
			yy = yp - (int)((double)TrafficStats[cnt].value * scale);
			
			// Just paint if we are connected...
			fillrect.bottom = yp;
			fillrect.top	= yy;
			fillrect.left	= xx;
			fillrect.right	= xx+plotgranularity;
			memDC->SetBrushOrg(xx,yp);
			if(TrafficStats[cnt].value > 0.0) {
				memDC->FillRect(&fillrect, &colorbrush);
				memDC->SetPixelV(xx, yy, Color1);
			}
		}

		// last print the textual statistic
		COLORREF textcolor = memDC->GetTextColor();
		int bkmode = memDC->GetBkMode();
		memDC->SetBkMode(TRANSPARENT);
		memDC->SetTextColor(ColorBack);
		memDC->TextOut(6,9,AllTraffic);
		memDC->SetTextColor(ColorText);
		memDC->TextOut(5,9,AllTraffic); 
		memDC->SetTextColor(textcolor);
		memDC->SetBkMode(bkmode);
		memDC->SelectObject(oldFont);
		memDC->SetBrushOrg(orgBrushOrigin.x, orgBrushOrigin.y);
		delete memDC;
	}	
	pDC -> RestoreDC( nSavedDC );
}

void CSpeedGraph::PreSubclassWindow()
{
	// make sure we are an owner draw button
	CWnd::PreSubclassWindow();
}

void CSpeedGraph::Set_TrafficValue(ULONGLONG ul_value) 
{
	double traffic = (double)ul_value;
	double delta1;
	delta1 = (double)(traffic) / 1024.0;
	
	// Shift whole array 1 step to left and calculate local maximum
	for(DWORD x=0; x<TrafficEntries-1; x++){
		TrafficStats[x].value	= TrafficStats[x+1].value;
	}

	TrafficStats[TrafficEntries-1].value = traffic;
	AllTraffic.Format(_T("%s %.1f kB/s"),m_s_Type,delta1);
	
	Invalidate(FALSE);
}

void CSpeedGraph::Init_Graph(CString s_type,UINT i_maxvalue)
{
	m_s_Type.Format(_T("%s"),s_type);
	m_ui_MaxAmount = i_maxvalue*1024;
}

void CSpeedGraph::Init_Color(COLORREF c_Color1, COLORREF c_Color2, COLORREF c_ColorBack, COLORREF c_ColorGrid, COLORREF c_ColorText)
{
	Color1 = c_Color1;
	Color2 = c_Color2;
	ColorGrid = c_ColorGrid;
	ColorBack = c_ColorBack;
	ColorText = c_ColorText;
	if(initalized){
		GridPen.DeleteObject();
		GridPen.CreatePen(PS_SOLID ,1 , ColorGrid);
	}
	brushInitalized = FALSE;
}

void CSpeedGraph::Init() 
{
	this->GetWindowRect(&TrafficDrawRectangle);
	this->GetWindowRect(&TrafficDrawUpdateRectangle);
	ScreenToClient(&TrafficDrawUpdateRectangle);
	ScreenToClient(&TrafficDrawRectangle);

	TGSize.cx = TrafficDrawRectangle.right - TrafficDrawRectangle.left;
	TGSize.cy = TrafficDrawRectangle.bottom - TrafficDrawRectangle.top;

	smallFont.DeleteObject();
	smallFont.CreateFont(-10,0,0,0,FW_THIN, FALSE, FALSE, FALSE, DEFAULT_CHARSET,OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH, _T("Arial"));

	GetWindowRect(&TrafficDrawRectangle);
	ScreenToClient(&TrafficDrawRectangle);
	
	TGSize.cx = TrafficDrawRectangle.right - TrafficDrawRectangle.left;
	TGSize.cy = TrafficDrawRectangle.bottom - TrafficDrawRectangle.top;
	
	TrafficEntries = TGSize.cx / plotgranularity;
	delete[] TrafficStats;
	TrafficStats = new TRAFFICENTRY[TrafficEntries];

	for(DWORD x=0; x<TrafficEntries; x++){
		TrafficStats[x].value = 0.0;//(float)x * (float)TrafficEntries*0.05;//;10.0 + 10.0*(sin(5.0*(float)x*3.14/180.0));
	}

	CRgn rectRgn, ellRgn, finalRgn;
	rectRgn.CreateRectRgn(0,0,TGSize.cx, TGSize.cy);
	ShapeWNDRegion.DeleteObject();
	ShapeWNDRegion.CreateRectRgn(0,0,TGSize.cx, TGSize.cy);;
	ShapeDCRegion.DeleteObject();
	ShapeDCRegion.CreateRectRgn(0,0,TGSize.cx, TGSize.cy);;


	int x1,x2,y1,y2,xe,ye,xs,ys;
	int xof, yof;
	int r;
	xs = TGSize.cx;
	ys = TGSize.cy;
	x1 = 0;
	y1 = 0;
	x2 = xs;
	y2 = ys;
	xe = 5;//Radius of edge
	ye = 5;//Radius of edge
	xof = (int)( (float)xs*0.0);
	yof = (int)( (float)ys*0.0);
	ellRgn.DeleteObject();
	r = ellRgn.CreateRoundRectRgn(x1,y1,x2,y2,xe,ye);
	r = ellRgn.OffsetRgn(-xof, -yof);
	r = ShapeWNDRegion.CombineRgn(&rectRgn, &ellRgn,RGN_AND );
	r = ShapeDCRegion.CombineRgn(&rectRgn, &ellRgn,RGN_AND );

	this->SetWindowRgn((HRGN)ShapeWNDRegion, TRUE);

	initalized = TRUE;
}

BOOL CSpeedGraph::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}