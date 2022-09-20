/*******************************************

	CTrafficGraph

	Version:	1.0
	Date:		31.10.2001
	Author:		Michael Fatzi
	Mail:		Michael_Fatzi@hotmail.com
	Copyright 1996-1997, Keith Rule

	You may freely use or modify this code provided this
	Copyright is included in all derived versions.
	
	History: 10.2001 Startup

	Handy little button control to display current 
	nettraffic as graph in a button.

********************************************/

// Change Button class to Dialog class by bornbest.

#pragma once

typedef struct _TRAFFIC_ENTRY_
{
	double value;
} TRAFFICENTRY;

#define PLOTGRANULATRITY 2		// Defines the width of the rectangle representing a bar in the diagram
#define GRIDXRESOLUTION	10		// Distance for grid in x direction
#define GRIDYRESOLUTION	10		// Distance for grid in y direction
#define GRIDSCROLLXSPEED -1		// How fast grid scrolls in x direction
#define GRIDSCROLLYSPEED 0		// How fast grid scrolls in y direction


// CSpeedGraph
typedef VOID (CALLBACK* INTERFACECHANCEDPROC)(int);

class CSpeedGraph : public CWnd
{
public:
	CSpeedGraph(); 
	virtual ~CSpeedGraph();

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnPaint();

protected:
	virtual void PreSubclassWindow();

public:
	void Init_Graph(CString s_type,UINT i_maxvalue);
	void Init_Color(COLORREF c_Color1, COLORREF c_Color2, COLORREF c_ColorBack, COLORREF c_ColorGrid, COLORREF c_ColorText);
	void Set_TrafficValue(ULONGLONG ul_value) ;

	CString m_s_Type;

private:
	void InterfaceHasChanged();

	INTERFACECHANCEDPROC interfaceCallBack;

	CFont	smallFont;
	CBrush	colorbrush;
	CBitmap	colorbrushbmp;

	CPen	GridPen;
	CSize	TGSize;

	RECT	TrafficDrawRectangle;
	RECT	TrafficDrawUpdateRectangle;

	CString AllTraffic;

	DWORD	TrafficEntries;

	BOOL	initalized;
	BOOL	brushInitalized;

	CRgn	ShapeWNDRegion;
	CRgn	ShapeDCRegion;

	double	m_ui_MaxAmount;
	double	m_ui_MaxValue;
	uint32	m_ui_MaxValueDelay;

	TRAFFICENTRY* TrafficStats;

	int gridxstartpos;		
	int gridystartpos;
	int plotgranularity;		

	// Public modification variables
public:
	int gridxresolution;		// The size of grid raster
	int gridyresolution;
	int gridscrollxspeed;		// Scroll speed of the grid
	int gridscrollyspeed; 

	COLORREF Color1;
	COLORREF Color2;
	COLORREF ColorGrid;
	COLORREF ColorBack;
	COLORREF ColorText;

	void Init();

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
