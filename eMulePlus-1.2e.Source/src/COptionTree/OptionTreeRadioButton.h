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


#ifndef OT_RADIOBUTTON
#define OT_RADIOBUTTON

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionTreeRadioButton.h : header file
//


// Added Headers
#include "CommonRes.h"
#include "OptionTreeDef.h"
#include "OptionTreeItem.h"


// Radio Button Node
struct OT_RADIO_NODE
{
	CString m_strText;

	BOOL m_bChecked;

	CRect m_rcHitRect;

	OT_RADIO_NODE *m_nNextNode;
};


// Classes
class COptionTree;

/////////////////////////////////////////////////////////////////////////////
// COptionTreeRadioButton window

class COptionTreeRadioButton : public CWnd
{
// Construction
public:
	COptionTreeRadioButton();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionTreeRadioButton)
	//}}AFX_VIRTUAL

// Implementation
public:
	int Node_GetChecked();
	void Node_UnCheckAll();
	void SetRadioOptionsOwner(COptionTree *otOption);
	OT_RADIO_NODE *Node_FindNode(CString strText);
	OT_RADIO_NODE *Node_FindNode(int nIndex);
	void Node_DeleteAll();
	void Node_Insert(CString strText, BOOL bChecked);
	virtual ~COptionTreeRadioButton();

protected:
	

	// Generated message map functions
protected:
	OT_RADIO_NODE *m_nAllNodes;
	COptionTree *m_otRadioOption;
	//{{AFX_MSG(COptionTreeRadioButton)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !OT_RADIOBUTTON
