#pragma once

#include "EnBitmap.h"
#include "CreditsCtrl.h"
#include "LayeredWindowHelperST.h"

class CSplashScreen : public CDialog
{
	DECLARE_DYNAMIC(CSplashScreen)

public:
	CSplashScreen(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSplashScreen();
	
	enum { IDD = IDD_ABOUTBOX };

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);

private:
	CLayeredWindowHelperST	m_Layered;
	UINT			m_nTimeOut;
	CEnBitmap		m_imgSplash;
	CCreditsCtrl	m_ctrlText;
	int				m_iTranslucency;
	bool			m_bLButtonDown;
};
