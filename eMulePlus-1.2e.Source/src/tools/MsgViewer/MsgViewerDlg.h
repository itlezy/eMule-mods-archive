// MsgViewerDlg.h : header file
//

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CMsgViewerDlg dialog

#include <afxadv.h>
#include <afxtempl.h>

#include "SizeableDlg.h"
#include "ProcessFileDlg.h"

const MAX_VIEWABLE = 500;

#include "Common.h"

class CMsgViewerDlg : public CSizeableDlg
{
// Construction

	MSG_FILE_INFO m_stFileInfo;
	CRecentFileList m_stRecentList;

public:
	static const TCHAR s_szSection[];
	CMsgViewerDlg(CWnd* pParent = NULL);	// standard constructor

	CString m_strTmpFile;
	DWORD m_dwPageViewed;
	BOOL OpenFile(PCTSTR wszFileName);
	void TryOpen(PCTSTR);

// Dialog Data
	//{{AFX_DATA(CMsgViewerDlg)
	enum { IDD = IDD_MSGVIEWER_DIALOG };
	CSpinButtonCtrl	m_wndSpin;
	CEdit	m_wndFilterStats;
	CProgressCtrl	m_wndFilterProgress;
	CComboBox	m_wndFileNameCombo;
	CSliderCtrl	m_wndSlider;
	CListCtrl	m_wndList;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMsgViewerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

	virtual void OnOK();

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CMsgViewerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnLoad();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRefresh();
	afx_msg void OnReleasedcaptureSlider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelendokFilenamecombo();
	afx_msg void OnFilter();
	afx_msg void OnDeltaposSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

