#pragma once

#include "DialogMinTrayBtn.h"
#include "ResizableLib\ResizableDialog.h"

#define WM_TRAY_ICON_NOTIFY_MESSAGE (WM_USER + 1)


class CTrayDialog : public CDialogMinTrayBtn<CResizableDialog>
{
protected:
	typedef CDialogMinTrayBtn<CResizableDialog> CTrayDialogBase;

public:
	void TraySetMinimizeToTray(bool *pbMinimizeToTray);
	BOOL TraySetMenu(UINT nResourceID, UINT nDefaultPos=0);
	BOOL TraySetMenu(HMENU hMenu, UINT nDefaultPos=0);
	BOOL TraySetMenu(LPCTSTR lpszMenuName, UINT nDefaultPos=0);
	BOOL TrayUpdate();
	BOOL TrayShow();
	BOOL TrayHide();
	void TraySetToolTip(LPCTSTR lpszToolTip);
	void TraySetIcon(HICON hIcon, bool bDelete= false);
	void TrayMinimizeToTrayChanged();

	bool TrayIsVisible();
	CTrayDialog(UINT uIDD,CWnd* pParent = NULL);   // standard constructor

	virtual void OnTrayRButtonUp(CPoint pt);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	
	DECLARE_MESSAGE_MAP()

private:
	afx_msg LRESULT OnTrayNotify(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTaskBarCreated(WPARAM wParam, LPARAM lParam);

	bool			*m_pbMinimizeToTray;
	HICON			m_hPrevIconDelete;
	NOTIFYICONDATA	m_nidIconData;
	CMenu			m_mnuTrayMenu;
	UINT			m_nDefaultMenuItem;
	bool			m_bCurIconDelete;
	bool			m_bLButtonDblClk;
	bool			m_bTrayIconVisible;
};
