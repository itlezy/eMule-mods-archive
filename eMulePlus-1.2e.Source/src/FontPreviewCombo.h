/*********************************************************************

   Copyright (C) 2002 Smaller Animals Software, Inc.

   This software is provided 'as-is', without any express or implied
   warranty.  In no event will the authors be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.

   3. This notice may not be removed or altered from any source distribution.

   http://www.smalleranimals.com
   smallest@smalleranimals.com

   --------

   This code is based, in part, on:
   "A WTL-based Font preview combo box", Ramon Smits
   http://www.codeproject.com/wtl/rsprevfontcmb.asp

**********************************************************************/
#pragma once

/////////////////////////////////////////////////////////////////////////////
// CFontPreviewCombo window

class CFontPreviewCombo : public CComboBox
{
public:
	CFontPreviewCombo();
	virtual ~CFontPreviewCombo();

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);

	// call this to load the font strings
	void	Init();

public:
   /*
      All of the following options must be set before you call Init() !!
   */

	// choose the sample color (only applies with NAME_THEN_SAMPLE and SAMPLE_THEN_NAME)
	COLORREF	m_clrSample;

	// choose how the name and sample are displayed
	typedef enum
	{
		NAME_ONLY = 0,		// font name, drawn in font
		NAME_GUI_FONT,		// font name, drawn in GUI font
		NAME_THEN_SAMPLE,	// font name in GUI font, then sample text in font
		SAMPLE_THEN_NAME,	// sample text in font, then font name in GUI font
		SAMPLE_ONLY			// sample text in font
	} PreviewStyle;

	PreviewStyle	m_style;

	// height of the sample	text (doesn't change the font name text)
	int m_iFontHeight;

protected:
	CImageList m_img;

	int m_iMaxNameWidth;
	int m_iMaxSampleWidth;

	afx_msg void OnDropdown();

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
