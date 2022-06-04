#pragma once

#include "TreeOptionsCtrlEx.h"

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