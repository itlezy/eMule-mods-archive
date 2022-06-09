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

#include "ServerListCtrl.h"
#include "ResizableLib\ResizableDialog.h"
#include "XPStyleButtonST.h"
#include "Loggable.h"
#include "ToolTips\PPToolTip.h"

class CHTRichEditCtrl;

class CNewTabCtrl : public CTabCtrl
{
protected:
	virtual void  DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
};

// CServerWnd dialog

class CServerWnd : public CResizableDialog, public CLoggable
{
	DECLARE_DYNAMIC(CServerWnd)

public:
	CServerWnd(CWnd* pParent = NULL);   // standard constructor
	virtual ~CServerWnd();
	void Localize();
	void ToggleDebugWindow();
	void InitFont();
    // Dialog Data
	enum { IDD = IDD_SERVER };

	void AddServer(const CString &strAddress, const CString &strPort, const CString &strName, const TCHAR *pcAuxPort = _T(""), bool bChangeServerInfo = false);
	void UpdateServerMetFromURL(CString strURL);
	afx_msg void OnStnClickedServinfIco();
	afx_msg void OnBnClickedUpdateservermet();
	afx_msg void OnBnClickedNewserver();
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnLinkServerBox(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedResetLog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()

public:
	CServerListCtrl		m_ctlServerList;
	CXPStyleButtonST	m_ctrlNewServerBtn;
	CXPStyleButtonST	m_ctrlUpdateServerMetBtn;
	CXPStyleButtonST	m_ctrlResetLogBtn;
	CPPToolTip			m_ttip;
	CHTRichEditCtrl		*m_pctlServerMsgBox;
	CHTRichEditCtrl		*m_pctlLogBox;
	CHTRichEditCtrl		*m_pctlDebugBox;
	CImageList			m_imageList;
	CNewTabCtrl			m_ctrlBoxSwitcher;

private:
	CStatic*	GetServerLstStatic() const { return (CStatic*)GetDlgItem(IDC_SERVLST_ICO); }
	HICON		GetServerLstIcon() const { return (HICON)::LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_SERVERLIST), IMAGE_ICON, 16, 16, 0); }
	void		GetInfo4ToolTip(NM_PPTOOLTIP_DISPLAY	*pNotify);

private:
	HICON	oldServerListIcon;
	bool	bDebug;
};
