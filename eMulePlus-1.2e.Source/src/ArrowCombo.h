/*********************************************************************
	ArrowCombo: a combo box with up & down arrows, CopyLeft by Cax2.
	Displays 2 different fonts in the same combo box line. 
	
	This code is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.

	Inspired by the FontPreviewCombo code by
	http://www.smalleranimals.com
	smallest@smalleranimals.com

	which is based, in part, on:
	"A WTL-based Font preview combo box", Ramon Smits
	http://www.codeproject.com/wtl/rsprevfontcmb.asp

**********************************************************************/
#pragma once

class CArrowCombo : public CComboBox
{
public:
	CArrowCombo();
	virtual ~CArrowCombo();
	int m_iFontHeight;
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) {NOPRM(nIDCtl); NOPRM(lpMeasureItemStruct);}
protected:
	DECLARE_MESSAGE_MAP()
};
