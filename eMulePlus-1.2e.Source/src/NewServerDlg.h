#pragma once

#include "resource.h"

class CServerWnd;

// CNewServerDlg dialog

class CNewServerDlg : public CDialog
{
	DECLARE_DYNAMIC(CNewServerDlg)

protected:
	CPoint		m_cpPosition;
	int			m_iCorner;
	bool		m_bUsePos;
	
	CServerWnd *m_pParent;
	CString		m_strServerAddr;
	CString		m_strPort;
	CString		m_strServerName;
	bool		m_bServerEditMode;	// true if we are editing a server, false if we are adding a server

public:
	CNewServerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNewServerDlg();

// Dialog Data
	enum { IDD = IDD_NEWSERVER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	void SetInitialPos(CPoint pos, int iCorner = 0);
	void SetParent(CServerWnd* pParent);
	void SetServerEditMode(bool bServerEditMode = true)	{ m_bServerEditMode = bServerEditMode; }
	void SetLabels(CString strAddress, CString strPort, CString strName, CString strAuxPort);
	void Localize();
	afx_msg void OnBnClickedAddserver();

	BOOL m_bAddAuxPort;
	CString m_strAuxPort;
	afx_msg void OnBnClickedAuxportCheckbox();
};
