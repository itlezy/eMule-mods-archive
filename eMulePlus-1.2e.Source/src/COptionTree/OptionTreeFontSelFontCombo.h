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


#ifndef OT_FONTSEL_FONTCOMBO
#define OT_FONTSEL_FONTCOMBO

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FontSelFontCombo.h : header file
//

// Added Headers
#include "CommonRes.h"

/////////////////////////////////////////////////////////////////////////////
// COptionTreeFontSelFontCombo window

// Structures
struct OT_FONTSEL_ITEM
{
	DWORD dwFontType;
	LOGFONT lf;
};

class COptionTreeFontSelFontCombo : public CComboBox
{
// Construction
public:
	COptionTreeFontSelFontCombo();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionTreeFontSelFontCombo)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
	virtual void DeleteItem(LPDELETEITEMSTRUCT lpDIS);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~COptionTreeFontSelFontCombo();
	int AddFont(LOGFONT *pLF, DWORD FontType);
	CString GetCurrentName();

	OT_FONTSEL_ITEM* GetFontItem(int nSel = -1);
	LPLOGFONT GetLogFont(int nSel = -1);
	DWORD GetFontType(int nSel = -1);
	// Generated message map functions
protected:
	//{{AFX_MSG(COptionTreeFontSelFontCombo)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

public:
	BOOL SelectFontName(CString strName);


protected:
	CBitmap m_bmpTrueType;
	CBitmap m_bmpMask;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !OT_FONTSEL_FONTCOMBO
