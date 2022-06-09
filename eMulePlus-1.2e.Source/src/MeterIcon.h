// MeterIcon.h: interface for the CMeterIcon class.
//
// Created: 04/02/2001 {mm/dm/yyyyy}
// Written by: Anish Mistry http://am-productions.yi.org/
/* This code is licensed under the GNU GPL.  See License.txt or (http://www.gnu.org/copyleft/gpl.html). */
//////////////////////////////////////////////////////////////////////
#pragma once

class CMeterIcon
{
public:
	CMeterIcon();
	virtual ~CMeterIcon();

	bool SetColorLevels(int *pLimits, COLORREF *pColors, int nEntries);
	COLORREF SetBorderColor(COLORREF crColor);
	int SetNumBars(int nNum);
	int SetMaxValue(int nVal);
	int SetWidth(int nWidth);
	SIZE SetDimensions(int nWidth, int nHeight);
	bool Init(HICON hFrame, int nMaxVal, int nNumBars, int nSpacingWidth, int nWidth, int nHeight, COLORREF crColor);
	HICON Create(int *pBarData);
	HICON SetFrame(HICON hIcon);

protected:
	HICON CreateMeterIcon(int *pBarData);
	COLORREF GetMeterColor(int nLevel);

	bool DrawIconMeterPic(CDC &DestDC, int nLevel, int nPos);
	bool DrawMeterMask(CDC &DestDCMask, int nLevel, int nPos);

protected:
	int m_nEntries;
	int m_nSpacingWidth;
	int m_nMaxVal;
	int m_nNumBars;
	HICON m_hFrame;
	COLORREF m_crBorderColor;
	int *m_pLimits;
	COLORREF *m_pColors;
	SIZE m_sDimensions;
	bool m_bInit;
};
