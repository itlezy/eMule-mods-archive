#pragma once
#include "afxwin.h"

class CServerWnd;

// CUpdateServerMetDlg dialog

class CUpdateServerMetDlg : public CDialog
{
	DECLARE_DYNAMIC(CUpdateServerMetDlg)

protected:
	CPoint		m_cpPosition;
	int			m_iCorner;
	bool		m_bUsePos;
	CServerWnd *m_pParent;

public:
	CUpdateServerMetDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CUpdateServerMetDlg();

// Dialog Data
	enum { IDD = IDD_UPDATESERVERMET };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	void SetInitialPos(CPoint pos, int iCorner = 0);
	void SetParent(CServerWnd* pParent);
	void Localize();
	afx_msg void OnBnClickedUpdateservermetfromurl();
protected:
	CString m_strServerMetURL;
	virtual void OnOK();
public:
	afx_msg void OnCbnSelchangeServerMetURL();
	CComboBox m_ComboBox;
};
