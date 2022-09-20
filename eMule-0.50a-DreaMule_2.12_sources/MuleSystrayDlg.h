#pragma once

class CInputBox : public CEdit
{
protected:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

    DECLARE_MESSAGE_MAP()
};


#include "TrayMenuBtn.h"		// Added by ClassView
#include "GradientStatic.h"	// Added by ClassView
#include "resource.h"


/////////////////////////////////////////////////////////////////////////////
// CMuleSystrayDlg dialog

class CMuleSystrayDlg : public CDialog
{
// Construction
public:
	CMuleSystrayDlg(CWnd* pParent, CPoint pt, int iMaxUp, int iMaxDown, int iCurUp, int iCurDown);
	~CMuleSystrayDlg();
    
// Dialog Data
	//{{AFX_DATA(CMuleSystrayDlg)
	enum { IDD = IDD_MULETRAYDLG };
	CGradientStatic	m_ctrlSidebar;
	//}}AFX_DATA
	/* Xman
	CStatic	m_ctrlUpArrow;
	CStatic	m_ctrlDownArrow;
	CSliderCtrl	m_ctrlUpSpeedSld;
	CSliderCtrl	m_ctrlDownSpeedSld;
	CInputBox	m_DownSpeedInput;
	CInputBox	m_UpSpeedInput;
	int		m_nDownSpeedTxt;
	int		m_nUpSpeedTxt;
	*/


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMuleSystrayDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	/* Xman
	CTrayMenuBtn m_ctrlSpeed;
	CTrayMenuBtn m_ctrlAllToMax;
	CTrayMenuBtn m_ctrlAllToMin;
	*/
	CTrayMenuBtn m_ctrlRestore;
	CTrayMenuBtn m_ctrlDisconnect;
	CTrayMenuBtn m_ctrlConnect;
	CTrayMenuBtn m_ctrlExit;
	CTrayMenuBtn m_ctrlPreferences;

	bool m_bClosingDown;
	
	/* Xman
	int m_iMaxUp;
	int m_iMaxDown;
	*/
	CPoint m_ptInitialPosition;

	/* Xman
	HICON m_hUpArrow;
	HICON m_hDownArrow;
	*/

	UINT m_nExitCode;

	// Generated message map functions
	//{{AFX_MSG(CMuleSystrayDlg)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	//}}AFX_MSG
	/* Xman
	afx_msg void OnChangeDowntxt();
	afx_msg void OnChangeUptxt();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	*/

	DECLARE_MESSAGE_MAP()
};
