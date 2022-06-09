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

#ifndef OT_ITEMIPADDRESS
#define OT_ITEMIPADDRESS

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionTreeItemFont.h : header file
//

// Added Headers
#include "CommonRes.h"
#include "OptionTreeDef.h"
#include "OptionTreeItem.h"
#include "OptionTreeIPAddressEdit.h"

// Classes
class COptionTree;


/////////////////////////////////////////////////////////////////////////////
// COptionTreeItemIPAddress window

class COptionTreeItemIPAddress : public CIPAddressCtrl, public COptionTreeItem
{
// Construction
public:
	COptionTreeItemIPAddress();
	virtual void OnMove();
	virtual void OnRefresh();
	virtual void OnCommit();
	virtual void OnActivate();
	virtual void CleanDestroyWindow();
	virtual void OnDeSelect();
	virtual void OnSelect();
	virtual void DrawAttribute(CDC *pDC, const RECT &rcRect);
// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionTreeItemIPAddress)
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL CreateIPAddressItem(DWORD dwAddStyle);
	virtual ~COptionTreeItemIPAddress();

	// Generated message map functions
protected:
	afx_msg long OnEditLooseFocus(UINT lParam, long wParam);
	afx_msg long OnEditNextEdit(UINT lParam, long wParam);	
	BOOL m_bFocus;
	COptionTreeIPAddressEdit m_ctlEdit4;
	COptionTreeIPAddressEdit m_ctlEdit3;
	COptionTreeIPAddressEdit m_ctlEdit2;
	COptionTreeIPAddressEdit m_ctlEdit1;
	//{{AFX_MSG(COptionTreeItemIPAddress)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !OT_ITEMIPADDRESS
