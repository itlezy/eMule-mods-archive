// MeterIcon.cpp: implementation of the CMeterIcon class.
//
// Created: 04/02/2001 {mm/dm/yyyyy}
// Written by: Anish Mistry http://am-productions.yi.org/
/* This code is licensed under the GNU GPL.  See License.txt or (http://www.gnu.org/copyleft/gpl.html). */
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MeterIcon.h"
#include "otherfunctions.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#define _MAXBLOCKS	5	// added by FoRcHa

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMeterIcon::CMeterIcon()
{
	m_nNumBars = 2;
	m_sDimensions.cx = 16;
	m_sDimensions.cy = 16;
	m_nMaxVal = 100;
	m_nSpacingWidth = 0;
	m_hFrame = NULL;
	m_bInit = false;
	m_crBorderColor = RGB(0,0,0);
	m_pLimits = NULL;
	m_pColors = NULL;
	m_nEntries = 0;
}

CMeterIcon::~CMeterIcon()
{
	// free color list memory
	if(m_pLimits)
		delete []m_pLimits;
	if(m_pColors)
		delete []m_pColors;
}

COLORREF CMeterIcon::GetMeterColor(int nLevel)
// it the nLevel is greater than the values defined in m_pLimits the last value in the array is used
{
	for(int i = 0;i < m_nEntries;i++)
	{
		if(nLevel <= m_pLimits[i])
		{
			return m_pColors[i];
		}
	}
	// default to the last entry
	return m_pColors[m_nEntries-1];
}


HICON CMeterIcon::CreateMeterIcon(int *pBarData)
// the returned icon must be cleaned up using DestroyIcon()
{
	ICONINFO iiNewIcon = {0};
	memzero(&iiNewIcon,sizeof(ICONINFO));
	iiNewIcon.fIcon = true;	// set that it is an icon

	CPaintDC dc(CWnd::GetDesktopWindow());
	// create DC's
	CDC IconDC;

	if(!IconDC.CreateCompatibleDC(&dc))
	{
		ASSERT(false);
		return NULL;
	}
	
	// load bitmaps
	//iiNewIcon.hbmColor = ::CreateCompatibleBitmap(dc,m_sDimensions.cx,m_sDimensions.cy);
	CBitmap ColorBitmap;
	if(!ColorBitmap.CreateCompatibleBitmap(&dc,m_sDimensions.cx,m_sDimensions.cy))
	{
		ASSERT(false);
		return NULL;
	}

	CBitmap *pOldIconDC = IconDC.SelectObject(&ColorBitmap);
	if(pOldIconDC == NULL)
	{
		ASSERT(false);
		//DeleteObject(iiNewIcon.hbmColor);
		return NULL;
	}
	//if(!BitBlt(hIconDC,0,0,m_sDimensions.cx,m_sDimensions.cy,NULL,0,0,BLACKNESS))
	if(!IconDC.BitBlt(0,0,m_sDimensions.cx,m_sDimensions.cy,NULL,0,0,BLACKNESS))
	{
		ASSERT(false);
		IconDC.SelectObject(pOldIconDC);
		//DeleteObject(iiNewIcon.hbmColor);
		return NULL;
	}
	if(!DrawIconEx(IconDC,0,0,m_hFrame,m_sDimensions.cx,m_sDimensions.cy,NULL,NULL,DI_NORMAL))
	{
		ASSERT(false);
		IconDC.SelectObject(pOldIconDC);
		//DeleteObject(iiNewIcon.hbmColor);
		return NULL;
	}

	// draw the meters
	for(int i = 0;i < m_nNumBars;i++)
		if(DrawIconMeterPic(IconDC,pBarData[i],i) == false)
		{
			ASSERT(false);
			IconDC.SelectObject(pOldIconDC);
			//DeleteObject(iiNewIcon.hbmColor);
			return false;
		}

	IconDC.SelectObject(pOldIconDC);

	// Now attend the mask 
	CDC MaskDC;
	if(!MaskDC.CreateCompatibleDC(&dc))
	{
		ASSERT(false);
		return NULL;
	}

	iiNewIcon.hbmMask = ::CreateCompatibleBitmap(MaskDC,m_sDimensions.cx,m_sDimensions.cy);
	if(iiNewIcon.hbmMask == NULL)
	{
		ASSERT(false);
		return NULL;
	}

	HGDIOBJ hOldMaskDC = (HGDIOBJ)MaskDC.SelectObject(iiNewIcon.hbmMask);
	if(hOldMaskDC == NULL)
	{
		ASSERT(false);
		DeleteObject(iiNewIcon.hbmMask);
		return NULL;
	}
	// initilize the bitmaps
	if(!MaskDC.BitBlt(0,0,m_sDimensions.cx,m_sDimensions.cy,NULL,0,0,WHITENESS))
	{
		ASSERT(false);
		MaskDC.SelectObject(hOldMaskDC);
		DeleteObject(iiNewIcon.hbmMask);
		return NULL;
	}

	if(!DrawIconEx(MaskDC,0,0,m_hFrame,m_sDimensions.cx,m_sDimensions.cy,NULL,NULL,DI_NORMAL))
	{
		ASSERT(false);
		MaskDC.SelectObject(hOldMaskDC);
		DeleteObject(iiNewIcon.hbmMask);
		return NULL;
	}

	// draw the meters
	for(int i = 0;i < m_nNumBars;i++)
		if(DrawMeterMask(MaskDC,pBarData[i],i) == false)
		{
			ASSERT(false);
			MaskDC.SelectObject(hOldMaskDC);
			DeleteObject(iiNewIcon.hbmMask);
			return false;
		}

	// create icon
	MaskDC.SelectObject(hOldMaskDC);


	iiNewIcon.hbmColor	=	ColorBitmap;
	HICON hNewIcon = CreateIconIndirect(&iiNewIcon);

	// cleanup
	DeleteObject(iiNewIcon.hbmColor);
	DeleteObject(iiNewIcon.hbmMask);

	return hNewIcon;
}


HICON CMeterIcon::SetFrame(HICON hIcon)
// return the old frame icon
{
	HICON hOld = m_hFrame;
	m_hFrame = hIcon;
	return hOld;
}

HICON CMeterIcon::Create(int *pBarData)
// must call init once before calling
{
	if(!m_bInit)
		return NULL;
	return CreateMeterIcon(pBarData);
}

bool CMeterIcon::Init(HICON hFrame, int nMaxVal, int nNumBars, int nSpacingWidth, int nWidth, int nHeight, COLORREF crColor)
// nWidth & nHeight are the dimensions of the icon that you want created
// nSpacingWidth is the space between the bars
// hFrame is the overlay for the bars
// crColor is the outline color for the bars
{
	SetFrame(hFrame);
	SetWidth(nSpacingWidth);
	SetMaxValue(nMaxVal);
	SetDimensions(nWidth,nHeight);
	SetNumBars(nNumBars);
	SetBorderColor(crColor);
	m_bInit = true;
	return m_bInit;
}

SIZE CMeterIcon::SetDimensions(int nWidth, int nHeight)
// return the previous dimension
{
	SIZE sOld = m_sDimensions;
	m_sDimensions.cx = nWidth;
	m_sDimensions.cy = nHeight;
	return sOld;
}

int CMeterIcon::SetNumBars(int nNum)
{
	int nOld = m_nNumBars;
	m_nNumBars = nNum;
	return nOld;
}

int CMeterIcon::SetWidth(int nWidth)
{
	int nOld = m_nSpacingWidth;
	m_nSpacingWidth = nWidth;
	return nOld;
}

int CMeterIcon::SetMaxValue(int nVal)
{
	int nOld = m_nMaxVal;
	m_nMaxVal = nVal;
	return nOld;
}

COLORREF CMeterIcon::SetBorderColor(COLORREF crColor)
{
	COLORREF crOld = m_crBorderColor;
	m_crBorderColor = crColor;
	return crOld;
}

bool CMeterIcon::SetColorLevels(int *pLimits, COLORREF *pColors,int nEntries)
// pLimits is an array of int that contain the upper limit for the corresponding color
{
	// free exsisting memory
	if(m_pLimits)
	{
		delete []m_pLimits;
		m_pLimits = NULL;
	}
	if(m_pColors)
	{
		delete []m_pColors;
		m_pColors = NULL;
	}
	// allocate new memory
	m_pLimits = new int[nEntries];
	m_pColors = new COLORREF[nEntries];
	// copy values
	for(int i = 0;i < nEntries;i++)
	{
		m_pLimits[i] = pLimits[i];
		m_pColors[i] = pColors[i];
	}
	m_nEntries = nEntries;
	return true;
}

bool CMeterIcon::DrawIconMeterPic(CDC &DestDC, int nLevel, int nPos)
{
	NOPRM(nPos);
	// draw meter
	HBRUSH hBrush = CreateSolidBrush(GetMeterColor(nLevel));
	if(hBrush == NULL)
	{
		ASSERT(false);
		return false;
	}
	HGDIOBJ hOldBrush = DestDC.SelectObject(hBrush);
	if(hOldBrush == NULL)
	{
		ASSERT(false);
		DeleteObject(hBrush);
		return(false);
	}
	HPEN hPen = CreatePen(PS_SOLID,1,m_crBorderColor);
	if(hPen == NULL)
	{
		ASSERT(false);	
		DestDC.SelectObject(hOldBrush);
		DeleteObject(hBrush);
		return false;
	}
	HGDIOBJ hOldPen = DestDC.SelectObject(hPen);
	if(hOldPen == NULL)
	{
		ASSERT(false);	
		DestDC.SelectObject(hOldBrush);
		DeleteObject(hBrush);
		DeleteObject(hPen);
		return false;
	}
	
	// FoRcHa start //////////////////////////////////////////////////////////////////
	if(nLevel > 0)
	{
		int iYpos = m_sDimensions.cy - 4;
		int nBlockCnt = nLevel / ((m_nMaxVal == 0 ? 1 : m_nMaxVal) / _MAXBLOCKS) + 1;
		if(nBlockCnt > _MAXBLOCKS)
			nBlockCnt = _MAXBLOCKS;
		for(int i = 0; i < nBlockCnt; i++)
		{
			DestDC.Rectangle( 0, iYpos, 4, iYpos+4);
			iYpos -= 3;
		}
	}
	// FoRcHa end ////////////////////////////////////////////////////////////////////

	DestDC.SelectObject(hOldPen);
	DeleteObject(hPen);
	DestDC.SelectObject(hOldBrush);
	DeleteObject(hBrush);
	return true;
}

bool CMeterIcon::DrawMeterMask(CDC &DestDCMask, int nLevel, int nPos)
{
	NOPRM(nPos);
	// draw meter mask
	HBRUSH hDestDCMaskBrush = CreateSolidBrush(RGB(0,0,0));
	if(hDestDCMaskBrush == NULL)
	{
		ASSERT(false);
		return false;
	}
	HGDIOBJ hOldDestDCMaskBrush = DestDCMask.SelectObject(hDestDCMaskBrush);
	if(hOldDestDCMaskBrush == NULL)
	{
		ASSERT(false);	
		DeleteObject(hDestDCMaskBrush);
		return false;
	}
	HPEN hMaskPen = CreatePen(PS_SOLID,1,RGB(0,0,0));
	if(hMaskPen == NULL)
	{
		ASSERT(false);	
		DestDCMask.SelectObject(hOldDestDCMaskBrush);
		DeleteObject(hDestDCMaskBrush);
		return false;
	}
	HGDIOBJ hOldMaskPen = DestDCMask.SelectObject(hMaskPen);
	if(hOldMaskPen == NULL)
	{
		ASSERT(false);	
		DestDCMask.SelectObject(hOldDestDCMaskBrush);
		DeleteObject(hDestDCMaskBrush);
		DeleteObject(hMaskPen);
		return false;
	}


	// FoRcHa start //////////////////////////////////////////////////////////////////
	if(nLevel > 0)
	{
		int iYpos = m_sDimensions.cy - 4;
		int nBlockCnt = nLevel / ((m_nMaxVal == 0 ? 1 : m_nMaxVal) / _MAXBLOCKS) + 1;
		for(int i = 0; i < nBlockCnt; i++)
		{
			DestDCMask.Rectangle( 0, iYpos, 4, iYpos+4);
			iYpos -= 3;
		}
	}
	// FoRcHa end ////////////////////////////////////////////////////////////////////

	DestDCMask.SelectObject(hOldMaskPen);
	DeleteObject(hMaskPen);
	DestDCMask.SelectObject(hOldDestDCMaskBrush);
	DeleteObject(hDestDCMaskBrush);

	return true;
}
