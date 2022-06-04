#pragma once
#include "ResizableLib\ResizableDialog.h"

#define	IDT_BALLON			101

class CTrayDialog : public CResizableDialog
{
public:
	CTrayDialog(UINT uIDD, CWnd* pParent = NULL);   // standard constructor

	BOOL TrayShow();
	BOOL TrayHide();
	void TraySetToolTip(LPCTSTR lpszToolTip);
	void TraySetBalloonToolTip(LPCTSTR lpszTitle, LPCTSTR lpszInfo, UINT infoflag = NIIF_INFO);
	void TraySetIcon(HICON hIcon);
	BOOL TrayIsVisible(){return m_bTrayIconVisible;}

protected:
	BOOL m_bTrayIconVisible;
	NOTIFYICONDATA m_nidIconData;
	UINT_PTR m_uBallonTimer;

	void KillBallonTimer();

	DECLARE_MESSAGE_MAP()	
	afx_msg LRESULT OnTaskBarCreated(WPARAM wParam, LPARAM lParam);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
