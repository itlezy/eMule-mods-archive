//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#pragma once
#include "ResizableLib\ResizableDialog.h"
#include "ServerListCtrl.h"
#include "RichEditCtrlX.h"
#include "ClosableTabCtrl.h"
#include "SplitterControl.h"
#include "kademlia/routing/contact.h"

//class CKadContactHistogramCtrl;
class CHTRichEditCtrl;

class CServerWnd : public CResizableDialog
{
	DECLARE_DYNAMIC(CServerWnd)

public:
	CServerWnd(CWnd* pParent = NULL);   // standard constructor
	virtual ~CServerWnd();

	// Contacts
	/*void ShowContacts();
	void HideContacts();
	void ContactAdd(const Kademlia::CContact* contact);
	void ContactRem(const Kademlia::CContact* contact);*/

	void Localize();
	void ToggleDebugWindow();
	void UpdateMyInfo();
	void UpdateLogTabSelection();
	void SaveAllSettings();
	//void UpdateControlsState();
	void PasteServerFromClipboard();
	bool AddServer(uint16 uPort, CString strAddress, CString strName = _T(""), bool bShowErrorMB = true);
	CString GetMyInfoString();

// Dialog Data
	enum { IDD = IDD_SERVER };

	enum ELogPaneItems
	{
		PaneLog			= 0, // those are CTabCtrl item indices
		PaneVerboseLog	= 1
	};

	CServerListCtrl serverlistctrl;
	CHTRichEditCtrl* logbox;
	CHTRichEditCtrl* debuglog;
	CClosableTabCtrl StatusSelector;
	CSplitterControl m_wndSplitter;

private:
	void	DoResize(int delta);
	void	UpdateSplitterRange();
	void	InitSplitter();
	void	ReattachAnchors();

	//CKadContactHistogramCtrl* m_contactHistogramCtrl;
	CImageList m_imlLogPanes;
	HICON icon_srvlist;
	bool	debug;
	CRichEditCtrlX m_MyInfo;
	CHARFORMAT m_cfDef;
	CHARFORMAT m_cfBold;
	CString m_strClickNewVersion;
	
protected:
	void SetAllIcons();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	//afx_msg void OnBnClickedAddserver();
	//afx_msg void OnBnClickedUpdateNodesDatFromUrl();
	//afx_msg void OnBnClickedUpdateServerMetFromUrl();
	//afx_msg void OnBnClickedBootstrapbutton();
	//afx_msg void OnBnKADConnect();
	//afx_msg void OnBnClickedFirewallcheckbutton();
	//afx_msg void OnEnSetfocusBootstrapip();
	afx_msg void OnBnClickedResetLog();
	afx_msg void OnTcnSelchangeTab3(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnLinkLogBox(NMHDR *pNMHDR, LRESULT *pResult);
	//afx_msg void OnSysColorChange();
	afx_msg void OnStnDblclickServlstIco();
	afx_msg void OnSplitterMoved(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};