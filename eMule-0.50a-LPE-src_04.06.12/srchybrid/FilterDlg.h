#pragma once
#include "EditDelayed.h"

class CFilterDlg{// X: [FI] - [FilterItem]
public:
	CFilterDlg():filter(0){};

	void CreateFilterMenu(CMenu&CatMenu) const;
	CString GetFilterLabel() const;

	virtual void	UpdateFilterLabel()=0;
	CEditDelayed	m_ctlFilter;
	size_t			filter;
};