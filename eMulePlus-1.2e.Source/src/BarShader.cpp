#include "stdafx.h"
#include "emule.h"
#include "BarShader.h"
#include "math.h"
#include "otherfunctions.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifndef M_PI_2
#define M_PI_2     1.57079632679489661923
#endif

#ifndef PI
#define PI 3.14159265358979323846264338328
#endif

#define HALF(X) (((X) + 1) / 2)

CBarShader::CBarShader(uint32 dwHeight, uint32 dwWidth, COLORREF crColor /*= 0*/, uint64 qwFileSize /*= 1*/)
{
	m_iWidth = dwWidth;
	m_iHeight = dwHeight;
	m_qwFileSize = 0;
	m_Spans.SetAt(0, crColor);
	m_Spans.SetAt(qwFileSize, 0);
	m_pdblModifiers = NULL;
	m_bIsPreview = false;
	m_used3dlevel = 0;
	SetFileSize(qwFileSize);
}

CBarShader::~CBarShader(void)
{
	delete[] m_pdblModifiers;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Must not be called when m_used3dlevel = 0 (flat)
void CBarShader::BuildModifiers()
{
	if (m_pdblModifiers != NULL)
	{
		delete[] m_pdblModifiers;
		m_pdblModifiers = NULL; // 'new' may throw an exception
	}

	// New property page slider to control depth of gradient

	// Depth must be at least 2
	// 2 gives greatest depth, the higher the value, the flatter the appearance
	// m_pdblModifiers[dwCount-1] will always be 1, m_pdblModifiers[0] depends on the value of depth
	static const double dDepths[5] = { 5.5, 4.0, 3.0, 2.50, 2.25 };		//aqua bar - smoother gradient jumps...
	double	depth = dDepths[((m_used3dlevel > 5) ? (256 - m_used3dlevel) : m_used3dlevel) - 1];
	uint32	dwCount = HALF(static_cast<uint32>(m_iHeight));
	double piOverDepth = PI / depth;
	double base = PI / 2 - piOverDepth;
	double increment = piOverDepth / (dwCount - 1);

	m_pdblModifiers = new double[dwCount];
	for (uint32 i = 0; i < dwCount; i++, base += increment)
		m_pdblModifiers[i] = sin(base);
}

void CBarShader::SetWidth(int width)
{
	if(m_iWidth != width) {
		m_iWidth = width;
		if (m_qwFileSize)
			m_dblPixelsPerByte = (double)m_iWidth / m_qwFileSize;
		else
			m_dblPixelsPerByte = 0.0;
		if (m_iWidth)
			m_dblBytesPerPixel = (double)m_qwFileSize / m_iWidth;
		else
			m_dblBytesPerPixel = 0.0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBarShader::SetFileSize(uint64 qwFileSize)
{
	if (m_qwFileSize != qwFileSize)
	{
		m_qwFileSize = qwFileSize;
		if (m_qwFileSize)
			m_dblPixelsPerByte = static_cast<double>(m_iWidth) / m_qwFileSize;
		else
			m_dblPixelsPerByte = 0.0;
		if (m_iWidth)
			m_dblBytesPerPixel = static_cast<double>(m_qwFileSize) / m_iWidth;
		else
			m_dblBytesPerPixel = 0.0;
	}
}

void CBarShader::SetHeight(int height)
{
	if(m_iHeight != height)
 	{
		m_iHeight = height;

		if ((m_used3dlevel = g_App.m_pPrefs->Get3DDepth()) != 0)
			BuildModifiers();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBarShader::FillRange(uint64 qwStart, uint64 qwEnd, COLORREF crColor)
{
	if(qwEnd > m_qwFileSize)
		qwEnd = m_qwFileSize;

	if(qwStart >= qwEnd)
		return;
	POSITION endprev, endpos = m_Spans.FindFirstKeyAfter(qwEnd + 1ui64);

	if ((endprev = endpos) != NULL)
		m_Spans.GetPrev(endpos);
	else
		endpos = m_Spans.GetTailPosition();

	ASSERT(endpos != NULL);

	COLORREF endcolor = m_Spans.GetValueAt(endpos);

	if ((endcolor == crColor) && (m_Spans.GetKeyAt(endpos) <= qwEnd) && (endprev != NULL))
		endpos = endprev;
	else
		endpos = m_Spans.SetAt(qwEnd, endcolor);

	for (POSITION pos = m_Spans.FindFirstKeyAfter(qwStart); pos != endpos;)
	{
		POSITION pos1 = pos;
		m_Spans.GetNext(pos);
		m_Spans.RemoveAt(pos1);
	}

	m_Spans.GetPrev(endpos);

	if ((endpos == NULL) || (m_Spans.GetValueAt(endpos) != crColor))
		m_Spans.SetAt(qwStart, crColor);
}

void CBarShader::Fill(COLORREF crColor)
{
	m_Spans.RemoveAll();
	m_Spans.SetAt(0, crColor);
	m_Spans.SetAt(m_qwFileSize, 0);
}

void CBarShader::DrawPreview(CDC* dc, int iLeft, int iTop, byte previewLevel)
{
	m_bIsPreview = true;
	m_used3dlevel = previewLevel;
	if (previewLevel != 0)
		BuildModifiers();
	Draw(dc, iLeft, iTop, (previewLevel == 0));
	m_bIsPreview = false;
}

void CBarShader::Draw(CDC* dc, int iLeft, int iTop, bool bFlat)
{
	COLORREF crLastColor = ~0u, crPrevBkColor = dc->GetBkColor();
	POSITION pos = m_Spans.GetHeadPosition();
	RECT rectSpan;
	rectSpan.top = iTop;
	rectSpan.bottom = iTop + m_iHeight;
	rectSpan.right = iLeft;

	sint64 iBytesInOnePixel = static_cast<sint64>(m_dblBytesPerPixel + 0.5);
	uint64 qwStart = 0;
	COLORREF crColor = m_Spans.GetNextValue(pos);

	iLeft += m_iWidth;
	while ((pos != NULL) && (rectSpan.right < iLeft)) {
		uint64 qwSpan = m_Spans.GetKeyAt(pos) - qwStart;
		int iPixels = static_cast<int>(qwSpan * m_dblPixelsPerByte + 0.5);

		if(iPixels > 0) {
			rectSpan.left = rectSpan.right;
			rectSpan.right += iPixels;
			FillRect(dc, &rectSpan, crLastColor = crColor, bFlat);

			qwStart += static_cast<uint64>(iPixels * m_dblBytesPerPixel + 0.5);
		} else {
			double dblWeight, dRed = 0, dGreen = 0, dBlue = 0;
			uint32 dwRed, dwGreen, dwBlue;
			uint64 qwLast = qwStart, qwEnd = qwStart + iBytesInOnePixel;

			do {
				dblWeight = (min(m_Spans.GetKeyAt(pos), qwEnd) - qwLast) * m_dblPixelsPerByte;
				dRed   += GetRValue(crColor) * dblWeight;
				dGreen += GetGValue(crColor) * dblWeight;
				dBlue  += GetBValue(crColor) * dblWeight;
				if ((qwLast = m_Spans.GetKeyAt(pos)) >= qwEnd)
					break;
				crColor = m_Spans.GetValueAt(pos);
				m_Spans.GetNext(pos);
			} while(pos != NULL);
			rectSpan.left = rectSpan.right;
			rectSpan.right++;

		//	Saturation
			dwRed = static_cast<uint32>(dRed);
			if (dwRed > 255)
				dwRed = 255;
			dwGreen = static_cast<uint32>(dGreen);
			if (dwGreen > 255)
				dwGreen = 255;
			dwBlue = static_cast<uint32>(dBlue);
			if (dwBlue > 255)
				dwBlue = 255;

			FillRect(dc, &rectSpan, crLastColor = RGB(dwRed, dwGreen, dwBlue), bFlat);
			qwStart += iBytesInOnePixel;
		}
		while((pos != NULL) && (m_Spans.GetKeyAt(pos) <= qwStart))
			crColor = m_Spans.GetNextValue(pos);
	}
	if ((rectSpan.right < iLeft) && (crLastColor != ~0))
	{
		rectSpan.left = rectSpan.right;
		rectSpan.right = iLeft;
		FillRect(dc, &rectSpan, crLastColor, bFlat);
	}
	dc->SetBkColor(crPrevBkColor);	//restore previous background color
}

void CBarShader::FillRect(CDC *dc, LPCRECT rectSpan, COLORREF crColor, bool bFlat)
{
	if(!crColor || bFlat)
		dc->FillSolidRect(rectSpan, crColor);
	else
	{
		if ((m_used3dlevel != g_App.m_pPrefs->Get3DDepth() && !m_bIsPreview) || m_pdblModifiers == NULL)
		{
			m_used3dlevel = g_App.m_pPrefs->Get3DDepth();
			BuildModifiers();
		}

		double	dblRed = GetRValue(crColor), dblGreen = GetGValue(crColor), dblBlue = GetBValue(crColor);
		double	dAdd, dMod;

		if (m_used3dlevel > 5)		//Cax2 aqua bar
		{
			dMod = 1.0 - .025 * (256 - m_used3dlevel);		//variable central darkness - from 97.5% to 87.5% of the original colour...
			dAdd = 255;

			dblRed = dMod * dblRed - dAdd;
			dblGreen = dMod * dblGreen - dAdd;
			dblBlue = dMod * dblBlue - dAdd;
		}
		else
			dAdd = 0;

		RECT		rect;
		int			iTop = rectSpan->top, iBot = rectSpan->bottom;
		double		*pdCurr = m_pdblModifiers, *pdLimit = pdCurr + HALF(m_iHeight);

		rect.right = rectSpan->right;
		rect.left = rectSpan->left;

		for (; pdCurr < pdLimit; pdCurr++)
		{
			crColor = RGB( static_cast<int>(dAdd + dblRed * *pdCurr),
				static_cast<int>(dAdd + dblGreen * *pdCurr),
				static_cast<int>(dAdd + dblBlue * *pdCurr) );
			rect.top = iTop++;
			rect.bottom = iTop;
			dc->FillSolidRect(&rect, crColor);

			rect.bottom = iBot--;
			rect.top = iBot;
		//	Fast way to fill, background color is already set inside previous FillSolidRect
			dc->ExtTextOut(0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static TCHAR ColorToChar(COLORREF crColor)
{
	unsigned	uiComponent;

	uiComponent = GetGValue(crColor);	//yellow (highest priority from user point of view)
	if (uiComponent != 0)
		return _T('8');
	uiComponent = GetRValue(crColor);	//red (high priority to stress it among blue ones)
	if (uiComponent > 100)
		return _T('7');
	return static_cast<TCHAR>(GetBValue(crColor) + _T('0'));	//0 - black, 1-6 blue
}

void CBarShader::GenerateWSBar(CString *pstrBar)
{
	POSITION	pos = m_Spans.GetHeadPosition();
	TCHAR		cColCode, *pcBarPos = pstrBar->GetBuffer(m_iWidth);
	const TCHAR	*const pcBarEnd = pcBarPos + m_iWidth;
	sint64		iBytesInOnePixel = static_cast<sint64>(m_dblBytesPerPixel + 0.5);
	uint64		qwStart = 0;
	COLORREF	crLastColor = ~0u, crColor = m_Spans.GetNextValue(pos);

	while ((pos != NULL) && (pcBarPos < pcBarEnd))
	{
		uint64 qwSpan = m_Spans.GetKeyAt(pos) - qwStart;
		int iPixels = static_cast<int>(qwSpan * m_dblPixelsPerByte + 0.5);

		if (iPixels > 0)
		{
			qwStart += static_cast<uint64>(iPixels * m_dblBytesPerPixel + 0.5);
			cColCode = ColorToChar(crLastColor = crColor);
			do {
				*pcBarPos++ = cColCode;
			} while(--iPixels > 0);
		}
		else
		{
			double dblWeight, dRed = 0, dGreen = 0, dBlue = 0;
			uint32 dwRed, dwGreen, dwBlue;
			uint64 qwLast = qwStart, qwEnd = qwStart + iBytesInOnePixel;

			do {
				dblWeight = (min(m_Spans.GetKeyAt(pos), qwEnd) - qwLast) * m_dblPixelsPerByte;
				dRed   += GetRValue(crColor) * dblWeight;
				dGreen += GetGValue(crColor) * dblWeight;
				dBlue  += GetBValue(crColor) * dblWeight;
				if ((qwLast = m_Spans.GetKeyAt(pos)) >= qwEnd)
					break;
				crColor = m_Spans.GetValueAt(pos);
				m_Spans.GetNext(pos);
			} while(pos != NULL);

		//	Saturation
			dwRed = static_cast<uint32>(dRed);
			if (dwRed > 255)
				dwRed = 255;
			dwGreen = static_cast<uint32>(dGreen);
			if (dwGreen > 255)
				dwGreen = 255;
			dwBlue = static_cast<uint32>(dBlue + 0.5);
			if (dwBlue > 6)
				dwBlue = 6;

			*pcBarPos++ = ColorToChar(crLastColor = RGB(dwRed, dwGreen, dwBlue));
			qwStart += iBytesInOnePixel;
		}
		while((pos != NULL) && (m_Spans.GetKeyAt(pos) <= qwStart))
			crColor = m_Spans.GetNextValue(pos);
	}
	if ((pcBarPos < pcBarEnd) && (crLastColor != ~0))
	{
		cColCode = ColorToChar(crLastColor);
		do {
			*pcBarPos = cColCode;
		} while(++pcBarPos < pcBarEnd);
	}
	pstrBar->ReleaseBufferSetLength(m_iWidth);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
