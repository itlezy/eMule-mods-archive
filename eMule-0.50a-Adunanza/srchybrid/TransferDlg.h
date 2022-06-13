//this file is part of eMule
//Copyright (C)2002-2010 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "UploadListCtrl.h"
#include "DownloadListCtrl.h"
#include "QueueListCtrl.h"
#include "ClientListCtrl.h"
#include "DownloadClientsCtrl.h"

#define wnd2Uploading	0
#define wnd2Downloading 1
#define wnd2OnQueue	2
#define wnd2Clients	3

class CTransferWnd;
class CToolbarWnd;

class CTransferDlg : public CFrameWnd
{
	DECLARE_DYNCREATE(CTransferDlg)

public:
	CTransferDlg();           // protected constructor used by dynamic creation
	virtual ~CTransferDlg();
	CTransferWnd* m_pwndTransfer;

	BOOL Create(CWnd* pParent);
	
	//Wrappers
	void Localize();
	void ShowQueueCount(); // Anis
	void UpdateFilesCount(int iCount);
	void UpdateCatTabTitles(bool force = true);
	void VerifyCatTabSize();
	int	 AddCategoryInteractive();
	void SwitchUploadList();
	void ResetTransToolbar(bool bShowToolbar, bool bResetLists = true);
	void SetToolTipsDelay(DWORD dwDelay);
	void OnDisableList();
	void UpdateListCount(const uint8 listindex);
	int	 AddCategory(CString newtitle,CString newincoming,CString newcomment,CString newautocat,bool addTab=true);
	void ShowToolbar(bool bShow);

	CUploadListCtrl*		GetUploadList();
	CDownloadListCtrl*		GetDownloadList();
	CQueueListCtrl*			GetQueueList();
	CClientListCtrl*		GetClientList();
	CDownloadClientsCtrl*	GetDownloadClientsList();
	CDownloadListCtrl		downloadlistctrl; //Fix by Anis
protected:
	CToolbarWnd* m_pwndToolbar;

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void DockToolbarWnd();

	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnClose();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
};