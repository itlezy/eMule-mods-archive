#pragma once

#include "TreeOptionsCtrlEx.h"

class CTreeOptionsInvisibleKeyCombo : public CTreeOptionsCombo
{
public:
	CTreeOptionsInvisibleKeyCombo();
	virtual ~CTreeOptionsInvisibleKeyCombo();
	
protected:
	bool m_bSelf;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnCbnSelchange();

	DECLARE_MESSAGE_MAP()

  DECLARE_DYNCREATE(CTreeOptionsInvisibleKeyCombo)
};