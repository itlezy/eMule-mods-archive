#pragma once

#include "TreeOptionsCtrlEx.h"
static const LPCTSTR ModKey[]={
	_T(""),
	_T("Alt"),
	_T("Ctrl"),
	_T("Ctrl + Alt"),
	_T("Shift"),
	_T("Shift + Alt"),
	_T("Ctrl + Shift"),
	_T("Ctrl + Shift + Alt")
} ;
class CTreeOptionsInvisibleModCombo : public CTreeOptionsCombo
{
public:
	CTreeOptionsInvisibleModCombo();
	virtual ~CTreeOptionsInvisibleModCombo();
	
protected:
	bool m_bSelf;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnCbnSelchange();

	DECLARE_MESSAGE_MAP()

  DECLARE_DYNCREATE(CTreeOptionsInvisibleModCombo)
};