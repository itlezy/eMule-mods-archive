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


#ifndef OT_TREEINFO
#define OT_TREEINFO

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionTreeInfo.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionTreeInfo window

// Classes
class COptionTree;

class COptionTreeInfo : public CStatic
{
// Construction
public:
	COptionTreeInfo();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionTreeInfo)
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetOptionsOwner(COptionTree *otOption);
	virtual ~COptionTreeInfo();

	// Generated message map functions
protected:
	//{{AFX_MSG(COptionTreeInfo)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	COptionTree *m_otOption;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !OT_TREEINFO
