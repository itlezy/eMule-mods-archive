// !!! ATTENTION !!!: this control is about 10% done, if you want to work on it, 
//					  then pleaase send me a message, and maybe the finished code *g*, 
//					  otherwise i will continue working on it on my own. ...sometime :D  
//					  [FoRcHa]

#include "stdafx.h"
#include "DblScope.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAINCOLORVALUE	224

/////////////////////////////////////////////////////////////////////////////
// CDblScope

CDblScope::CDblScope()
{
	m_nMin = 0;
	m_nMax = 100;
	m_nStep = 4;
	m_bInit = true;

	m_crColor1		= RGB(MAINCOLORVALUE,0,0);
	m_crColor2		= RGB(0,MAINCOLORVALUE,0);
	m_crBackColor	= RGB(64,64,64); 

	m_bScanLines = true;
	m_ScanPen.CreatePen(PS_SOLID, 1, RGB(0,0,0));
	m_pOldBMP = NULL; // FoRcHa
}

CDblScope::~CDblScope()
{
	// FoRcHa
	if(m_pOldBMP)
		m_MemDC.SelectObject(m_pOldBMP);
}


BEGIN_MESSAGE_MAP(CDblScope, CWnd)
	//{{AFX_MSG_MAP(CDblScope)
	ON_WM_PAINT()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDblScope message handlers

void CDblScope::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	CRect rClient;
	GetClientRect(&rClient);

	if(m_bInit)
	{		
		m_bInit = false;
		ReCreateGraph(&dc);
	}

	dc.BitBlt(0, 0, rClient.Width(), rClient.Height(), &m_MemDC, 0,0, SRCCOPY);
}

void CDblScope::ReCreateGraph(CDC *pDC)
{
	CRect rClient;
	GetClientRect(&rClient);

	m_bScanLines = (UINT)rClient.Height() >= m_nMax >> 2 ? true : false;

	// FoRcHa
	if(m_pOldBMP)
		m_MemDC.SelectObject(m_pOldBMP);
	if(m_MemBMP.GetSafeHandle())
		m_MemBMP.DeleteObject();

	if(m_MemDC.GetSafeHdc())
		m_MemDC.DeleteDC();

	m_MemDC.CreateCompatibleDC(pDC);
	m_MemBMP.CreateCompatibleBitmap(pDC, rClient.Width(), rClient.Height());
	m_pOldBMP = m_MemDC.SelectObject(&m_MemBMP); // FoRcHa
	m_MemDC.FillSolidRect(rClient,m_crBackColor);

	POSITION pos1 = m_List1.GetHeadPosition();
	POSITION pos2 = m_List2.GetHeadPosition();

	while(pos1 != NULL && pos2 != NULL)
		AddValues2Graph(m_List1.GetNext(pos1), m_List2.GetNext(pos2));

	if(m_bScanLines)
	{
		CPen *pOldPen = m_MemDC.SelectObject(&m_ScanPen);
		for(int y = 0; y < rClient.Height(); y += 2)
		{
			m_MemDC.MoveTo(rClient.left, y);
			m_MemDC.LineTo(rClient.right, y);
		}
		m_MemDC.SelectObject(pOldPen);
	}
}

void CDblScope::AddValues(UINT nVal1, UINT nVal2, bool bRedraw)
{
	m_List1.AddTail(nVal1);
	m_List2.AddTail(nVal2);

	if(GetSafeHwnd())
	{
		AddValues2Graph(nVal1, nVal2);

		if(bRedraw)
			Invalidate();
	}
}	

void CDblScope::ResetGraph(bool bDelete)
{
	if(bDelete)
	{
		m_List1.RemoveAll();
		m_List2.RemoveAll();
	}
	m_bInit = true;
	Invalidate();
}

void CDblScope::SoftReset()
{
	m_List1.RemoveAll();
	m_List2.RemoveAll();
}

void CDblScope::SetRange(UINT nMin, UINT nMax, bool bDelete)
{
	m_nMin = nMin;
	m_nMax = nMax;

	ResetGraph(bDelete);
}

void CDblScope::AddValues2Graph(UINT nVal1, UINT nVal2)
{
	if(m_MemDC.GetSafeHdc())
	{
		CRect rClient;
		GetClientRect(&rClient);
		int iWidth = rClient.Width();
		int iHeight = rClient.Height();
		int iNewWidth = iWidth - m_nStep;

		if(nVal1 > m_nMax)
			nVal1 = m_nMax;
		if(nVal2 > m_nMax)
			nVal2 = m_nMax;

		double dVal1 = (double)iHeight * nVal1 / m_nMax;
		double dVal2 = (double)iHeight * nVal2 / m_nMax;
		UINT nRndVal1 = (UINT)dVal1;
		if(dVal1 - nRndVal1 >= 0.5)
			nRndVal1++;
		UINT nRndVal2 = (UINT)dVal2;
		if(dVal2 - nRndVal2 >= 0.5)
			nRndVal2++;


		int iOverlap;
		int iHigher;

		COLORREF crOverlap;
		COLORREF crHigher;

		if(nVal1 > nVal2)
		{
			iHigher		= nRndVal1 - nRndVal2;
			iOverlap	= nRndVal1 - iHigher;
			//	crHigher	= m_crColor1;
			crHigher	= RGB(GetRValue(m_crColor1)-(iHeight-nRndVal1),GetGValue(m_crColor1),GetBValue(m_crColor1));
			crOverlap	= RGB(iHigher > MAINCOLORVALUE ? 0 : MAINCOLORVALUE - iHigher, MAINCOLORVALUE, 0);
		}
		else
		{
			iHigher		= nRndVal2 - nRndVal1;
			iOverlap	= nRndVal2 - iHigher;
			//	crHigher	= m_crColor2;
			crHigher	= RGB(GetRValue(m_crColor2),GetGValue(m_crColor2)-(iHeight-nRndVal2),GetBValue(m_crColor2));
			crOverlap	= RGB(MAINCOLORVALUE, iHigher > MAINCOLORVALUE ? 0 : MAINCOLORVALUE - iHigher, 0);
		}

		m_MemDC.BitBlt(0, 0, iNewWidth, iHeight, &m_MemDC, m_nStep, 0, SRCCOPY);
		m_MemDC.FillSolidRect(iNewWidth, 0, m_nStep, iHeight, m_crBackColor);
		if(iOverlap)
			m_MemDC.FillSolidRect(iNewWidth, iHeight - iOverlap, m_nStep, iOverlap, crOverlap);
		if(iHigher)
			m_MemDC.FillSolidRect(iNewWidth, iHeight - iOverlap - iHigher, m_nStep, iHigher, crHigher);

		if(m_bScanLines)
		{
			CPen *pOldPen = m_MemDC.SelectObject(&m_ScanPen);
			for(int y = 0; y < iHeight; y += 2)
			{
				m_MemDC.MoveTo(iNewWidth, y);
				m_MemDC.LineTo(iNewWidth+m_nStep, y);
			}
			m_MemDC.SelectObject(pOldPen);
		}

		while(m_List1.GetCount() > rClient.Width())
		{
			m_List1.RemoveHead();
			m_List2.RemoveHead();
		}	
	}
}

void CDblScope::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	m_bInit = true;
	Invalidate();	
}