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
#include "afxdlgs.h"
#include "HTRichEditCtrl.h"
#include "ChatSelector.h"
#include "ResizableLib\ResizableDialog.h"
#include "FriendListCtrl.h"
#include "SplitterControl.h"

// CChatWnd dialog
class CChatWnd : public CResizableDialog
{
	DECLARE_DYNAMIC(CChatWnd)
public:
	void ScrollHistory(bool down);
	CChatSelector m_ctlChatSelector;
	CChatWnd(CWnd* pParent = NULL);   // standard constructor
	virtual ~CChatWnd();

// Dialog Data
	enum { IDD = IDD_CHAT };
	void StartSession(CUpDownClient* client);
	void Localize();
	CFriendListCtrl m_FriendListCtrl;
	CEdit			m_ctlInputText;
	CButton			m_ctlCloseButton;
	CButton			m_ctlSendButton;
	CPPToolTip		m_ttip;

	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

	afx_msg void OnBnClickedCsend();
	afx_msg void OnBnClickedCclose();
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
protected:
	LRESULT		OnCloseTab(WPARAM wparam, LPARAM lparam);
	LRESULT		OnTabProperties(WPARAM wparam, LPARAM lparam);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL	PreTranslateMessage(MSG* pMsg);
	virtual LRESULT DefWindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
private:
	HICON oldMessageIcon;
	HICON oldFriendsIcon;
	CSplitterControl m_wndSplitterChat;

	CStatic* GetMessageStatic() const { return (CStatic*)GetDlgItem(IDC_MESSAGEICON); }
	CStatic* GetFriendsStatic() const { return (CStatic*)GetDlgItem(IDC_FRIENDSICON); }
	HICON GetMessageIcon() const { return (HICON)::LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_MESSAGE), IMAGE_ICON, 16, 16, 0); }
	HICON GetFriendsIcon() const { return (HICON)::LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_FRIEND_LBL), IMAGE_ICON, 16, 16, 0); }
	void DoResize(int iDelta);
	void GetInfo4ToolTip(NM_PPTOOLTIP_DISPLAY	*pNotify);
};
