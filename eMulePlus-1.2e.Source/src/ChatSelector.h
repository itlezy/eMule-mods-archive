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

#include "types.h"
#include "HTRichEditCtrl.h"
#include "ClosableTabCtrl.h"
#include "Loggable.h"

class CUpDownClient;

class CChatItem
{
public:
	CChatItem();
	~CChatItem()		{safe_delete(m_pLog);}
	CUpDownClient		*m_pClient;
	CHTRichEditCtrl		*m_pLog;
	CString				m_strMessagePending;
	bool				m_bNotify;
	CStringArray		m_strHistoryArray;
	int					m_iHistoryIndex;
};

class CChatSelector : public CClosableTabCtrl, public CLoggable
{
	DECLARE_DYNAMIC(CChatSelector)

public:
	CChatSelector();
	virtual			~CChatSelector();
	void			Init();
	CChatItem		*StartSession(CUpDownClient* client, bool bForceFocus = true);
	void			EndSession(CUpDownClient* client = 0, bool bForceFocus = true);
	int				GetTabByClient(CUpDownClient *pClient);
	CChatItem		*GetItemByClient(CUpDownClient* client);
	void			ProcessMessage(CUpDownClient* pSender, const CString &strMessage);
	bool			SendMessage(const CString& message);
	bool			SendAwayMessage(CChatItem* ci);
	void			DeleteAllItems();
	void			ShowChat();
	void			ConnectingResult(CUpDownClient* sender, bool bSuccess);
	void			Send();
	void			UpdateFont();
	CChatItem		*GetCurrentChatItem();

	afx_msg void 	OnSize(UINT nType, int cx, int cy);
	virtual BOOL 	PreTranslateMessage(MSG* pMsg);
	BOOL 			RemoveItem(int nItem)		{ return DeleteItem(nItem); }
	afx_msg void 	OnEnLinkMessageBox(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void 	OnDestroy();

protected:
	void			OnTimer(UINT_PTR nIDEvent);
	afx_msg void 	OnTcnSelchangeChatsel(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()
	virtual INT		InsertItem(int nItem,TCITEM* pTabCtrlItem);
	virtual BOOL	DeleteItem(int nItem);
	void			SendMessagePacket(CChatItem& ci, const CString& msg);

private:
	CHTRichEditCtrl	chatout;
	UINT_PTR		m_Timer;
	bool			blinkstate;
	bool			lastemptyicon;
};
