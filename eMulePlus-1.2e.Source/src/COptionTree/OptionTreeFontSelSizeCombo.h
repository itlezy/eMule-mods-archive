// COptionTree
//
// License
// -------
// This code is provided "as is" with no expressed or implied warranty.
// 
// You may use this code in a commercial product with or without acknowledgement.
// However you may not sell this code or any modification of this code, this includes 
// commercial libraries and anything else for profit.
//
// I would appreciate a notification of any bugs or bug fixes to help the control grow.
//
// History:
// --------
//	See License.txt for full history information.
//
//
// Copyright (c) 1999-2002 
// ComputerSmarts.net 
// mattrmiller@computersmarts.net

#ifndef OT_FONTSEL_SIZECOMBO
#define OT_FONTSEL_SIZECOMBO

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FontSelSizeCombo.h : header file
//
// Added Headers
#include "CommonRes.h"

/////////////////////////////////////////////////////////////////////////////
// COptionTreeFontSelSizeCombo window

class COptionTreeFontSelSizeCombo : public CComboBox
{
// Construction
public:
	COptionTreeFontSelSizeCombo();
	int AddSize(int nPointSize, long lfHeight);
	void GetPointSize(CY& cy);
	long GetHeight(int nSel = -1);
	void UpdateLogFont(LPLOGFONT lpLF, int nSel = -1);
// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionTreeFontSelSizeCombo)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~COptionTreeFontSelSizeCombo();

	// Generated message map functions
protected:
	//{{AFX_MSG(COptionTreeFontSelSizeCombo)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !OT_FONTSEL_SIZECOMBO
