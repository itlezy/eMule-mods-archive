// OScopeCtrl.h : header file
//

#ifndef __OScopeCtrl_H__
#define __OScopeCtrl_H__

/////////////////////////////////////////////////////////////////////////////
// COScopeCtrl window

class COScopeCtrl : public CWnd
{
	// Construction
public:
	COScopeCtrl(int NTrends = 1);

	// Attributes
	void AppendPoints(double dNewPoint[], bool bInvalidate = true, bool bAdd2List = true);
	void AppendEmptyPoints(double dNewPoint[], bool bInvalidate = true, bool bAdd2List = true);
	void SetRange(double dLower, double dUpper, int iTrend = 0);
	void SetRanges(double dLower, double dUpper);
	void SetXUnits(CString string, CString XMin = _T(""), CString XMax = _T(""));
	void SetYUnits(CString string, CString YMin = _T(""), CString YMax = _T(""));
	void SetGridColor(COLORREF color);
	void SetPlotColor(COLORREF color, int iTrend = 0);
	COLORREF GetPlotColor(int iTrend = 0);
	void SetBackgroundColor(COLORREF color);
	void InvalidateCtrl(bool deleteGraph = true);
	void DrawPoint();
	void Reset();

	// Operations

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COScopeCtrl)
public:
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID = NULL);
	//}}AFX_VIRTUAL

	// Implementation
	bool autofitYscale;
	int m_nXGrids;
	int m_nYGrids;
	int m_nShiftPixels;         // amount to shift with each new point 
	int m_nTrendPoints;			// when you set this to > 0, then plot will
	int m_nMaxPointCnt;
	// contain that much points (regardless of the
	// Trend/control) width drawn on screen !!!

	// Otherwise, if this is -1 (which is default),
	// m_nShiftPixels will be in use
	int m_nYDecimals;

	typedef struct m_str_struct
	{
		CString XUnits, XMin, XMax;
		CString YUnits, YMin, YMax;
	} m_str_t;
	m_str_t m_str;

	typedef struct PlotDataStruct 
	{
		COLORREF crPlotColor;       // data plot color
		CPen   penPlot;
		double dCurrentPosition;    // current position
		double dPreviousPosition;   // previous position
		int nPrevY;
		double dLowerLimit;         // lower bounds
		double dUpperLimit;         // upper bounds
		double dRange;				// = UpperLimit - LowerLimit
		double dVerticalFactor;

		CList<double,double> lstPoints;
	} PlotData_t;

	virtual ~COScopeCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(COScopeCtrl)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	int ReCreateGraph(void);
	afx_msg void OnTimer(UINT nIDEvent);
	void ReSizePlot(int iTrend,double scale);	//Cax2 resize just one line..

	void GetPlotRect(CRect &rPlotRect) const
	{
		rPlotRect = m_rectPlot;
	}
private:
	int m_oldcx;
	int m_oldcy;

	COLORREF m_crBackColor;        // background color
	COLORREF m_crGridColor;        // grid color

	int m_NTrends;

	struct CustShiftStruct
	{		// when m_nTrendPoints > 0, this structure will contain needed vars
		int m_nRmndr;				// reminder after dividing m_nWidthToDo/m_nPointsToDo
		int m_nWidthToDo;
		int m_nPointsToDo;
	} CustShift;

	PlotData_t *m_PlotData;

	int m_nClientHeight;
	int m_nClientWidth;
	int m_nPlotHeight;
	int m_nPlotWidth;

	CRect  m_rectClient;
	CRect  m_rectPlot;

	CDC     m_dcGrid;
	CDC     m_dcPlot;
	CBitmap *m_pbitmapOldGrid;
	CBitmap *m_pbitmapOldPlot;
	CBitmap m_bitmapGrid;
	CBitmap m_bitmapPlot;

	UINT m_nRedrawTimer;
	bool m_bDoUpdate;
};

/////////////////////////////////////////////////////////////////////////////
#endif
