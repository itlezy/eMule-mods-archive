#pragma once
#include "resource.h"
#include "ResizableLib\ResizableDialog.h"
#include "AduBrowser.h"

//Anis Hireche modules - AdunanzA 03/10/11

// finestra di dialogo AduWebBrowser

inline void AfxEnableDlgItem( CWnd *pDlg, int nIDDlgItem, BOOL bEnabled = TRUE ) //Anis function Afx.
{
    HWND hDlgItem = ::GetDlgItem( pDlg->GetSafeHwnd(), nIDDlgItem );
    EnableWindow( hDlgItem, bEnabled );
}

class AduWebBrowser : public CResizableDialog
{
	DECLARE_DYNAMIC(AduWebBrowser)
public:
	CWebBrowser2	m_browser;
	AduWebBrowser(CWnd* pParent = NULL);   // costruttore standard
	virtual ~AduWebBrowser();
	virtual BOOL OnInitDialog();
	enum { IDD = IDD_WEB_BROWSER };

protected:
	DECLARE_EVENTSINK_MAP()
	void TitleChangeBrowser(LPCTSTR Text);
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange(CDataExchange* pDX);    // Supporto DDX/DDV
	BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnEnSetfocusEdit2();
	afx_msg void OnEnKillfocusEdit2();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnTimer( UINT nIDEvent );
	afx_msg void StartTimer();
};