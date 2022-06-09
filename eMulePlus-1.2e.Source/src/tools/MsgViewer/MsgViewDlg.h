#pragma once

/////////////////////////////////////////////////////////////////////////////
// CMsgViewDlg dialog

#include "SizeableDlg.h"

class CMsgViewDlg : public CSizeableDlg
{
// Construction
public:
	CMsgViewDlg(LPCVOID pData, DWORD dwDataSize, __int64 nPos, ULONG nIdProtoType, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMsgViewDlg)
	enum { IDD = IDD_MSGVIEW };
	CString	m_strMsgData;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMsgViewDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	LPCVOID m_pData; // for export feature
	DWORD m_dwDataSize;

	// Generated message map functions
	//{{AFX_MSG(CMsgViewDlg)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	afx_msg void OnExport();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
