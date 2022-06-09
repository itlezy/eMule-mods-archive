//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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
#include "SharedFilesCtrl.h"
#include "ResizableLib\ResizableDialog.h"
#include "ProgressCtrlX.h"
#include "afxwin.h"
#include "IconStatic.h"

class CSharedFilesWnd : public CResizableDialog
{
	DECLARE_DYNAMIC(CSharedFilesWnd)

public:
	CSharedFilesWnd(CWnd* pParent = NULL);
	virtual ~CSharedFilesWnd();
	void Localize();
	void ShowDetails(CKnownFile* cur_file);
// Dialog Data
	enum { IDD = IDD_FILES };

	CSharedFilesCtrl	m_ctlSharedFilesList;
	CPPToolTip			m_ttip;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedReloadsharedfiles();
	afx_msg void OnBnClickedOpenincomingfolder();
	afx_msg void OnBnClickedSwitchAllKnown();
	afx_msg void OnLvnItemActivateSflist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSflist(NMHDR *pNMHDR, LRESULT *pResult);

	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

private:
	CProgressCtrlX pop_bar;
	CProgressCtrlX pop_baraccept;
	CProgressCtrlX pop_bartrans;
	CFont bold;
	CIconStatic m_ctrlStatisticsFrm;
//	Resources cleanup
	HICON oldFilesIcon;
	CStatic* GetFilesStatic() const { return (CStatic*)GetDlgItem(IDC_FILES_ICO); }
	HICON GetFilesIcon() const { return (HICON)::LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_SHAREDFILES), IMAGE_ICON, 16, 16, 0); }
	void 		GetInfo4ToolTip(NM_PPTOOLTIP_DISPLAY	*pNotify);
public:
	afx_msg void OnDestroy();
};
