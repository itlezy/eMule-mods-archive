#pragma once

#include "TabCtrl.hpp"
#define _TAB_BASECLASS	TabControl

// CTabs

class CTabs : public _TAB_BASECLASS
{
//	DECLARE_DYNAMIC(CTabs)

public:
	CTabs();
	virtual ~CTabs();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	virtual void DrawItem(LPDRAWITEMSTRUCT);
	virtual void DrawItem(CDC *pDC, int iItem);

	COLORREF GetRGBColorTabs();
	COLORREF GetRGBColorXP();
	COLORREF GetRGBColorGrayText();

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
};


