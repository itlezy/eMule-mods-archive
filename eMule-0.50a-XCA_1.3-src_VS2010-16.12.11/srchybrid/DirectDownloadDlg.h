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
#include "ResizableLib/ResizableDialog.h"
//#include "IconStatic.h"
#include "ButtonsTabCtrl.h"
#include "Neo\ModeLess.h" // NEO: MLD - [ModelesDialogs] <-- Xanatos --
#include "ListCtrlX.h"// X: [PL] - [Preview Links]
#include<vector>
class CED2KFileLink;
class CDirectDownloadDlg : public CModResizableDialog // NEO: MLD - [ModelesDialogs] <-- Xanatos --
{
	DECLARE_DYNAMIC(CDirectDownloadDlg)

public:
	CDirectDownloadDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDirectDownloadDlg();
// Dialog Data
	enum { IDD = IDD_DIRECT_DOWNLOAD };
	void ClipboardPromptMode(LPCTSTR pszLinks); // X: [UIC] - [UIChange] allow change cat
	void AddLink(LPCTSTR pszLinks); // X: [UIC] - [UIChange] allow change cat

protected:
	HICON m_icnWnd;
	//CIconStatic m_ctrlDirectDlFrm;
	CString m_LinkText;
	CButtonsTabCtrl	m_cattabs;
	CListCtrlX m_LinkList;
	std::vector<CED2KFileLink*> m_pLinkItems;// X: [PL] - [Preview Links]
	bool isInited;
	bool isShowList;
	bool clipboardPromptMode; // X: [UIC] - [UIChange] allow change cat

	void UpdateCatTabs();
	void InitLink();
	void AddLink2Vector(CString&appendLink);
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnEnKillfocusElink();
	afx_msg void OnLvnGetDispInfoLink(NMHDR *pNMHDR, LRESULT *pResult);// X: [PL] - [Preview Links]
	afx_msg void UpdateControls();//void OnEnUpdateElink();	// X: [CI] - [Code Improvement]
public:
	afx_msg void OnBnClickedPreview();

};
