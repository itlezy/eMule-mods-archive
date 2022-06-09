//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#include "stdafx.h"
#include <math.h>
#include "emule.h"
#include "barshader.h"
#include "Preferences.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// Why does _USE_MATH_DEFINES work in debug builds, but not in release builds??
#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

#define HALF(X) (((X) + 1) / 2)

// X: [CI] - [Code Improvement] BarShader
CRectShader::CRectShader(uint32 height) 
: m_used3dlevel(0)
, m_iHeight(height)
, m_Modifiers(NULL)
{
}

CRectShader::~CRectShader() {
	delete[] m_Modifiers;
}

void CRectShader::BuildModifiers() {
	delete[] m_Modifiers;
	m_Modifiers = NULL; // 'new' may throw an exception

	//if (!m_bIsPreview) 
		//m_used3dlevel=thePrefs.Get3DDepth();

	// Barry - New property page slider to control depth of gradient

	// Depth must be at least 2
	// 2 gives greatest depth, the higher the value, the flatter the appearance
	// m_Modifiers[count-1] will always be 1, m_Modifiers[0] depends on the value of depth
	
	int depth = (7-m_used3dlevel);
	int count = HALF(m_iHeight);
	double piOverDepth = M_PI/depth;
	double base = piOverDepth * ((depth / 2.0) - 1);
	double increment = piOverDepth / (count - 1);

	m_Modifiers = new float[count];
	for (int i = 0; i < count; i++)
		m_Modifiers[i] = (float)(sin(base + i * increment));
}

void CRectShader::SetHeight(int height) {
	if(m_iHeight != height) {
		m_iHeight = height;
		m_used3dlevel = thePrefs.Get3DDepth();
		BuildModifiers();
	}
}

 // X: [CI] - [Code Improvement]
void CRectShader::FillRect(CDC *dc, LPRECT rectSpan, float fRed, float fGreen, float fBlue) {
	RECT rect = *rectSpan;
	int iMax = HALF(m_iHeight);
	for(int i = 0; i < iMax; i++) {
		//Xman Code Improvement: FillSolidRect
		//CBrush cbNew(RGB((int)(fRed * m_Modifiers[i] + .5f), (int)(fGreen * m_Modifiers[i] + .5f), (int)(fBlue * m_Modifiers[i] + .5f)));
		const COLORREF crNew = RGB((int)(fRed * m_Modifiers[i] + .5f), (int)(fGreen * m_Modifiers[i] + .5f), (int)(fBlue * m_Modifiers[i] + .5f));
		dc->FillSolidRect(&rect, crNew);
		++rect.top;
		--rect.bottom;
		//Xman end
	}
}

void CRectShader::DrawPreview(CDC* dc, int iLeft, int iTop, int iWidth, COLORREF color, UINT previewLevel)		//Cax2 aqua bar
{
	//m_bIsPreview=true;
	m_used3dlevel = previewLevel;
	BuildModifiers();
	m_used3dlevel = thePrefs.Get3DDepth();
	DrawRect(dc, iLeft, iTop, iWidth, color, (previewLevel==0));
	//m_bIsPreview=false;
}

void CRectShader::DrawRect(CDC* dc, int iLeft, int iTop, int iWidth, COLORREF color, bool bFlat){
	if(iWidth <= 0) return;
	if (m_Modifiers == NULL || (m_used3dlevel!=thePrefs.Get3DDepth())){
		m_used3dlevel = thePrefs.Get3DDepth();
		BuildModifiers();
	}
	RECT rectSpan = {
		iLeft,
		iTop,
		iLeft + iWidth,
		iTop + m_iHeight
	};
	if (bFlat||g_bLowColorDesktop){
		//Xman Code Improvement: FillSolidRect
		//dc->FillRect(rectSpan, &CBrush(color));
		dc->FillSolidRect(&rectSpan, color);
	}
	else
		FillRect(dc, &rectSpan, GetRValue(color), GetGValue(color), GetBValue(color));
}

CBarShader::CBarShader(uint32 height) 
: CRectShader(height)
, m_Spans(100) // Maella -Code Improvement (CPU load)- 
{
	m_uFileSize = (uint64)1;
	m_Spans.SetAt(0, 0);	// SLUGFILLER: speedBarShader
}

void CBarShader::Reset() {
	Fill(0);
}

void CBarShader::SetFileSize(EMFileSize fileSize) {
	if(m_uFileSize != fileSize)
		m_uFileSize = fileSize;

}

void CBarShader::FillRange(uint64 start, uint64 end, COLORREF color) {
	if(end > m_uFileSize)
		end = m_uFileSize;

	if(start >= end)
		return;

	// SLUGFILLER: speedBarShader
	POSITION endpos = m_Spans.FindFirstKeyAfter(end+1);

	if (endpos)
		m_Spans.GetPrev(endpos);
	else
		endpos = m_Spans.GetTailPosition();

	// ==> Corrupted barshaderinfo? [fafner] - Stulle
	if (endpos == NULL) {
		ASSERT(0);
		return;
	}
	// <== Corrupted barshaderinfo? [fafner] - Stulle

	COLORREF endcolor = m_Spans.GetValueAt(endpos);
	endpos = m_Spans.SetAt(end, endcolor);

	// ==> Make code VS 2005 and VS 2008 ready [MorphXT] - Stulle
	/*
	for (POSITION pos = m_Spans.FindFirstKeyAfter(start+1); pos != endpos; ) {
	*/
	//Fafner: fix vs2005 chunk detail - 080317
	//Fafner: fix vs2005 freeze - 080317
	//Fafner: note: FindFirstKeyAfter seems to work differently under vs2005 than under vs2003
	//Fafner: note: the original code 'start+1' lead to not working chunk details
	//Fafner: note: for some reason pos can evaluate to NULL and then the loop continues forever
	//Fafner: note: see also similar code in CStatisticFile::AddBlockTransferred
	//Fafner: note: also look for the keywords 'spreadbarinfo', 'barshaderinfo'
#if _MSC_VER < 1400
	for (POSITION pos = m_Spans.FindFirstKeyAfter(start+1); pos != endpos && pos != NULL; ) {
#else
	for (POSITION pos = m_Spans.FindFirstKeyAfter(start); pos != endpos && pos != NULL; ) {
#endif
	// <== Make code VS 2005 and VS 2008 ready [MorphXT] - Stulle
		POSITION pos1 = pos;
		m_Spans.GetNext(pos);
		m_Spans.RemoveAt(pos1);
	}
	
	m_Spans.GetPrev(endpos);

	if (m_Spans.GetValueAt(endpos) != color)
		m_Spans.SetAt(start, color);
	// SLUGFILLER: speedBarShader
}

void CBarShader::Fill(COLORREF color) {
	// SLUGFILLER: speedBarShader
	m_Spans.RemoveAll();
	m_Spans.SetAt(0, color);
	m_Spans.SetAt(m_uFileSize, 0);
	// SLUGFILLER: speedBarShader
}

void CBarShader::Draw(CDC* dc, int iLeft, int iTop, int iWidth, bool bFlat){
	if(iWidth <= 0 || m_uFileSize == (uint64)0) return;

	if (m_Modifiers == NULL || (m_used3dlevel!=thePrefs.Get3DDepth())){
		m_used3dlevel = thePrefs.Get3DDepth();
		BuildModifiers();
	}

	// SLUGFILLER: speedBarShader
	POSITION pos = m_Spans.GetHeadPosition();

	double m_dPixelsPerByte = (double)iWidth / (uint64)m_uFileSize;
	double m_dBytesPerPixel = (double)m_uFileSize / iWidth;

	RECT rectSpan;
	rectSpan.top = iTop;
	rectSpan.bottom = iTop + m_iHeight;
	rectSpan.right = iLeft;

	uint64 uBytesInOnePixel = (uint64)(m_dBytesPerPixel + 0.5f);
	uint64 start = 0;//bsCurrent->start;
	// SLUGFILLER: speedBarShader
	COLORREF color = m_Spans.GetNextValue(pos); // X: [CI] - [Code Improvement]
	// SLUGFILLER: speedBarShader
	while(pos != NULL && rectSpan.right < (iLeft + iWidth)) {	// SLUGFILLER: speedBarShader
		uint64 uSpan = m_Spans.GetKeyAt(pos) - start;	// SLUGFILLER: speedBarShader
		uint64 uPixels = (uint64)(uSpan * m_dPixelsPerByte + 0.5f);
		if (uPixels > 0) {
			rectSpan.left = rectSpan.right;
			rectSpan.right += (int)uPixels;
			if (bFlat||g_bLowColorDesktop||!color){
				//Xman Code Improvement: FillSolidRect
				//dc->FillRect(rectSpan, &CBrush(color));
				dc->FillSolidRect(&rectSpan, color);
			}
			else
				FillRect(dc, &rectSpan, GetRValue(color), GetGValue(color), GetBValue(color));
			start += (uint64)(uPixels * m_dBytesPerPixel + 0.5f);
		} else {
			float fRed = 0;
			float fGreen = 0;
			float fBlue = 0;
			uint64 iEnd = start + uBytesInOnePixel;
			uint64 iCur; // X: [CI] - [Code Improvement]
			uint64 iLast = start;
			// SLUGFILLER: speedBarShader
			do {
				iCur = m_Spans.GetKeyAt(pos);
				float fWeight = (float)((min(iCur, iEnd) - iLast) * m_dPixelsPerByte);
				fRed   += GetRValue(color) * fWeight;
				fGreen += GetGValue(color) * fWeight;
				fBlue  += GetBValue(color) * fWeight;
				if(iCur > iEnd)
					break;
				iLast = iCur;
				color = m_Spans.GetNextValue(pos);
			} while(pos != NULL);
			// SLUGFILLER: speedBarShader
			rectSpan.left = rectSpan.right;
			rectSpan.right++;
			if (bFlat||g_bLowColorDesktop){
				//Xman Code Improvement: FillSolidRect
				//dc->FillRect(rectSpan, &CBrush(color));
				dc->FillSolidRect(&rectSpan, color);
			}
			else
				FillRect(dc, &rectSpan, fRed, fGreen, fBlue);
			start += uBytesInOnePixel;
		}
		// SLUGFILLER: speedBarShader
		while(pos != NULL && m_Spans.GetKeyAt(pos) < start) {
			color = m_Spans.GetNextValue(pos);
		}
		// SLUGFILLER: speedBarShader
	}
}
