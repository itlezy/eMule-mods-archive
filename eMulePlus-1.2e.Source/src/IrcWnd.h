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
#include "IrcMain.h"
#include "MuleListCtrl.h"
#include "ClosableTabCtrl.h"
#include "HTRichEditCtrl.h"
#include "XPStyleButtonST.h"
#include "IrcNickListCtrl.h"

enum EnumConnectStatus
{
	IRC_DISCONNECTED = 0,
	IRC_CONNECTING,
	IRC_CONNECTED
};

class CHTRichEditCtrl;

// CIrcWnd dialog
struct ChannelList
{
	CString name;
	CString users;
	CString desc;
};

struct Channel
{
	CString	name;
	CString title;
	CPtrList nicks;
	CHTRichEditCtrl log;
	CStringArray history;
	uint16 history_pos;
	byte type;
	// Type is mainly so that we can use this for IRC and the eMule Messages..
	// 1-Status, 2-Channel list, 4-Channel, 5-Private Channel, 6-eMule Message(Add later)
};

class CIrcWnd : public CResizableDialog
{
	DECLARE_DYNAMIC(CIrcWnd)
	friend class CIrcNickListCtrl;

public:
	CIrcWnd(CWnd* pParent = NULL);   // standard constructor
	virtual ~CIrcWnd();

	afx_msg void OnDestroy();

	enum { IDD = IDD_IRC };

	void		SortInit(int iSortCode);
	void		Localize();
	bool		GetLoggedIn()						{ return m_bLoggedIn; }
	void		SetLoggedIn(bool bLoggedIn)			{ m_bLoggedIn = bLoggedIn; }
	void		UpdateFont();
	void		ResetServerChannelList();
	void		AddChannelToList(CString strName, CString strUser, CString strDescription);
	void		ScrollHistory(bool bDown);
	void		ChangeAllNick(const CString &strOldNick, const CString &strNewNick);
	void		AddMessage(const CString &strChannel, const TCHAR *pcNick, COLORREF crTextColor, const CString &strInputLine);
	void		AddMessageF(const CString &strChannel, const TCHAR *pcNick, COLORREF crTextColor, const TCHAR *pcLine, ...);
	void		SetConnectStatus(EnumConnectStatus eConnectStatus);
	void		NoticeMessage(CString strSource, CString strMessage);
	void		SetTitle(CString strChannel, CString strTitle);
	void		SetActivity(CString strChannel);
	void		SendString(const CString &strSend);
	Channel*	FindChannelByName(const CString &strName);
	Channel*	NewChannel(const CString &strName, byte byteType);
	void		RemoveChannel(CString strChannel);
	void		DeleteAllChannel();
	void		JoinChannels();

	Channel*	GetCurrentChannel() const			{ return m_pCurrentChannel; }

	CMuleListCtrl		serverChannelList;
	CIrcNickListCtrl	m_ctlNickList;

protected:
	virtual BOOL	OnInitDialog();
	virtual void	OnSize(UINT nType, int cx, int cy);
	virtual int		OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void	DoDataExchange(CDataExchange* pDX);
	virtual BOOL	OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL	PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()

	afx_msg void	OnBnClickedBnIrcconnect();
	afx_msg void	OnBnClickedClosechat(int nItem=-1);
	afx_msg void	OnTcnSelchangeTab2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg	void	OnColumnClickChanL( NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg	void	OnNMRclickChanL(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void	OnNMDblclkserverChannelList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void	OnBnClickedTextBold();
	afx_msg void	OnBnClickedTextUnderline();
	afx_msg void	OnBnClickedTextColor();
	afx_msg void	OnBnClickedChatsend();
	afx_msg LONG	OnColorSelEndOK(WPARAM wParam, LPARAM lParam);
	afx_msg LONG	OnColorSelEndCancel(WPARAM wParam, LPARAM lParam);
	LRESULT			OnCloseTab(WPARAM wparam, LPARAM lparam);

	static	int	CALLBACK SortProcChanL(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

private:
	bool	IsConnected() const		{ return m_eConnectStatus == IRC_CONNECTED; }
	bool	IsDisconnected() const	{ return m_eConnectStatus == IRC_DISCONNECTED; }
	CStatic*	GetUserLstStatic() const { return (CStatic*)GetDlgItem(IDC_IRC_USERS_ICO); }
	HICON	GetUserLstIcon() const { return (HICON)::LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_USERS), IMAGE_ICON, 16, 16, 0); }

	Channel					*m_pCurrentChannel;
	CClosableTabCtrl		channelselect;
	CIrcMain				*m_pIrcMain;
	CHTRichEditCtrl			titleWindow;
	CEdit					inputWindow;
	CXPStyleButtonST		m_ctrlTextBoldBtn;
	CXPStyleButtonST		m_ctrlTextUnderlineBtn;
	CXPStyleButtonST		m_ctrlTextColorBtn;
	CPtrList				channelLPtrList;
	CPtrList				channelPtrList;
	HICON					m_hOldIcon;
	EnumConnectStatus		m_eConnectStatus;
	bool					m_bLoggedIn;
	bool					m_bSortAscendingChanList[IRC2COL_NUMCOLUMNS];
};
