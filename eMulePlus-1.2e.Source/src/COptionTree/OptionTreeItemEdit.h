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


#ifndef OT_ITEMEDIT
#define OT_ITEMEDIT

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionTreeItemEdit.h : header file
//

// Added Headers
#include "CommonRes.h"
#include "OptionTreeDef.h"
#include "OptionTreeItem.h"

// Classes
class COptionTree;

/////////////////////////////////////////////////////////////////////////////
// COptionTreeItemEdit window

class COptionTreeItemEdit : public CEdit, public COptionTreeItem
{
// Construction
public:
	COptionTreeItemEdit();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionTreeItemEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL GetOption(DWORD dwOption);
	void SetOption(DWORD dwOption, BOOL bSet);
	void SetEditLong(long lValue);
	void SetEditDword(DWORD dwValue);
	void SetEditFloat(float fValue);
	void SetEditInt(int nValue);
	void SetEditDouble(double dValue);
	BOOL CreateEditItem(DWORD dwOptions, DWORD dwAddStyle = 0);
	virtual void OnMove();
	virtual void OnRefresh();
	virtual void OnCommit();
	virtual void OnActivate();
	virtual void CleanDestroyWindow();
	virtual void OnDeSelect();
	virtual void OnSelect();
	virtual void DrawAttribute(CDC *pDC, const RECT &rcRect);

	BOOL GetEditDword(DWORD &dwReturn);
	BOOL GetEditInt(int &nReturn);
	BOOL GetEditLong(long &lReturn);
	BOOL GetEditDouble(double &dReturn);
	BOOL GetEditFloat(float &fReturn);
	virtual ~COptionTreeItemEdit();

	// Generated message map functions
protected:
	BOOL IsStringNumeric(CString strString);
	//{{AFX_MSG(COptionTreeItemEdit)
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	void OptionsChanges();
	DWORD m_dwOptions;
	BOOL m_bFocus;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !OT_ITEMEDIT
