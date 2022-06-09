#pragma once

#include "ProcessFileDlg.h"

struct CFilter {
	struct CEntry {
		PCTSTR m_szName;
		bool m_bAllowed;

		inline CEntry() { }
		void operator = (const CEntry& stAnoter)
		{
			m_szName = stAnoter.m_szName;
			m_bAllowed = stAnoter.m_bAllowed;
		}
	};

	CMap <ULONG, ULONG, CEntry, CEntry&> m_mapFilter;

	BOOL m_bIn;
	BOOL m_bOut;
	BOOL m_bNeither;
	BOOL m_bUnknown;

	BOOL m_bTypeServ;
	BOOL m_bTypePeer;

	CFilter()
	{
		m_bIn = TRUE;
		m_bOut = TRUE;
		m_bNeither = TRUE;
		m_bUnknown = TRUE;
		m_bTypeServ = TRUE;
		m_bTypePeer = TRUE;
	}

	void Open();
	void Save();

	BOOL AllowMsg(MSG_FILE_INFO::MSG_DATA& stData);

};

extern CFilter g_stFilter;

/////////////////////////////////////////////////////////////////////////////
// CFilterDlg dialog

class CFilterDlg : public CDialog
{
// Construction
public:
	void MsgSelect(DWORD, BOOL bSelect);
	BOOL MsgIsSelected(DWORD);
	void ProcessSingleButton(BOOL bApply, int& nButton, const USHORT* pMsg, UINT nCount);
	void ProcessButtons(UINT nButtonID);

	CFilterDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFilterDlg)
	enum { IDD = IDD_FILTER };
	CListBox	m_wndMessages;
	//}}AFX_DATA

	int		m_nSel_KeepAlive;
	int		m_nSel_Disconnect;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFilterDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFilterDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeMessages();
	afx_msg void OnSelectall();
	afx_msg void OnDeselectAll();
	virtual void OnOK();
	//}}AFX_MSG
	afx_msg void OnCheckButton();

	DECLARE_MESSAGE_MAP()
};

