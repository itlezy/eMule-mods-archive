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

#define NETUPDATESPEED	1000	// Value for the statistics timer
#define GRIDUPDATESPEED 200		// Value vor the grid timer
#define GRIDTIMER		1		// ID for the grid timer

#define GRIDXRESOLUTION	10		// Distance for grid in x direction
#define GRIDYRESOLUTION	10		// Distance for grid in y direction
#define GRIDSCROLLXSPEED -1		// How fast grid scrolls in x direction
#define GRIDSCROLLYSPEED 0		// How fast grid scrolls in y direction


// CSpeedGraph 대화 상자입니다.
typedef VOID (CALLBACK* INTERFACECHANCEDPROC)(int);

class CSpeedGraph : public CDialog
{
	DECLARE_DYNAMIC(CSpeedGraph)

public:
	CSpeedGraph(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CSpeedGraph();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_SPEEDGRAPH };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
protected:
	virtual void PreSubclassWindow();


public:
	void Init_Graph(CString s_type,UINT i_maxvalue);
	void Set_TrafficValue(ULONGLONG ul_value) ;

	CString m_s_Type;

	// Generated message map functions
protected:
	//{{AFX_MSG(CTrafficGraph)
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG

private:
	void InterfaceHasChanged();

	INTERFACECHANCEDPROC interfaceCallBack;

	CFont	smallFont;
	CBrush	colorbrush;

	COLORREF red, green, back, darkblue, darkgray, white, black, lightgreen, gray, orange;
	CBitmap	colorbrushbmp;
	CPen	GridPen;
	CSize	TGSize;

	CBrush	greenbrush;
	CBrush	redbrush;
	CPoint	orgBrushOrigin; 

	RECT	TrafficDrawRectangle;
	RECT	TrafficDrawUpdateRectangle;

	CString AllTraffic;

	DWORD	TrafficEntries;

	BOOL	initalized;
	BOOL	brushInitalized;

	CRgn	ShapeWNDRegion;
	CRgn	ShapeDCRegion;

	double	m_ui_MaxAmount;

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
	int gridupdatespeed;


	virtual BOOL OnInitDialog();
	virtual BOOL Create(UINT nIDTemplate,RECT &rect,CWnd* pParentWnd = NULL);

	RECT m_rect;
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
