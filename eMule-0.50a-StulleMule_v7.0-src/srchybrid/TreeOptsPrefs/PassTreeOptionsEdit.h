// emulEspaña: Added by MoNKi [ MoNKi: -Pass Edit on TreeOptionsCtrl- ]

#pragma once
#include "..\TreeOptionsCtrl.h"

class CPassTreeOptionsEdit :
	public CTreeOptionsEdit
{
public:
	CPassTreeOptionsEdit(void);
	virtual ~CPassTreeOptionsEdit(void);
	virtual DWORD GetWindowStyle();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_DYNCREATE(CPassTreeOptionsEdit)
};
