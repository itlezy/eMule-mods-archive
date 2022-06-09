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

#include "stdafx.h"
#include "emule.h"
#include "updownclient.h"
#include "ChatWnd.h"
#include "Friend.h"
#include "IP2Country.h"
#include "Details\clientdetails.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define	SPLITTER_RANGE_WIDTH	200
#define	SPLITTER_RANGE_HEIGHT	686

#define	SPLITTER_MARGIN			2
#define	SPLITTER_WIDTH			4

IMPLEMENT_DYNAMIC(CChatWnd, CDialog)
CChatWnd::CChatWnd(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CChatWnd::IDD, pParent)
{
}

CChatWnd::~CChatWnd()
{
}

void CChatWnd::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHATSEL, m_ctlChatSelector);
	DDX_Control(pDX, IDC_LIST2, m_FriendListCtrl);
	DDX_Control(pDX, IDC_CMESSAGE, m_ctlInputText);
	DDX_Control(pDX, IDC_CSEND, m_ctlSendButton);
	DDX_Control(pDX, IDC_CCLOSE, m_ctlCloseButton);
}

BEGIN_MESSAGE_MAP(CChatWnd, CResizableDialog)
	ON_WM_KEYDOWN()
	ON_WM_SHOWWINDOW()
	ON_MESSAGE(WM_CLOSETAB, OnCloseTab)
	ON_MESSAGE(WM_TAB_PROPERTIES, OnTabProperties)
	ON_BN_CLICKED(IDC_CCLOSE, OnBnClickedCclose)
	ON_BN_CLICKED(IDC_CSEND, OnBnClickedCsend)
	ON_WM_DESTROY()
END_MESSAGE_MAP()
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CChatWnd message handlers
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CChatWnd::OnInitDialog()
{
	CResizableDialog::OnInitDialog();

	m_ctlInputText.SetLimitText(MAX_CLIENT_MSG_LEN);
	oldMessageIcon = GetMessageStatic()->SetIcon(GetMessageIcon());
	oldFriendsIcon = GetFriendsStatic()->SetIcon(GetFriendsIcon());

//	Create window splitter
	CRect rcSpl;

	GetDlgItem(IDC_LIST2)->GetWindowRect(rcSpl);
	ScreenToClient(rcSpl);

	rcSpl.left = rcSpl.right + SPLITTER_MARGIN;
	rcSpl.right = rcSpl.left + SPLITTER_WIDTH;
	m_wndSplitterChat.Create(WS_CHILD | WS_VISIBLE, rcSpl, this, IDC_SPLITTER_FRIEND);

	uint32	dwPosStatVinit = rcSpl.left;
	uint32	dwPosStatVnew = g_App.m_pPrefs->GetSplitterbarPositionFriend();

	if (dwPosStatVnew > SPLITTER_RANGE_HEIGHT)
		dwPosStatVnew = SPLITTER_RANGE_HEIGHT;
	else if (dwPosStatVnew < SPLITTER_RANGE_WIDTH)
		dwPosStatVnew = SPLITTER_RANGE_WIDTH;
	rcSpl.left = dwPosStatVnew;
	rcSpl.right = dwPosStatVnew + SPLITTER_WIDTH;
	m_wndSplitterChat.MoveWindow(rcSpl);

	m_ctlChatSelector.Init();
	m_FriendListCtrl.Init();

	DoResize(dwPosStatVnew - dwPosStatVinit);

	AddAnchor(IDC_CSEND, BOTTOM_RIGHT);
	AddAnchor(IDC_CCLOSE, BOTTOM_RIGHT);
	AddAnchor(IDC_FRIENDS_LBL,TOP_LEFT);
	AddAnchor(IDC_FRIENDSICON,TOP_LEFT);

	m_ttip.Create(this);
	m_ttip.SetDelayTime(TTDT_AUTOPOP, 15000);
	m_ttip.SetDelayTime(TTDT_INITIAL, g_App.m_pPrefs->GetToolTipDelay()*1000);
	m_ttip.SendMessage(TTM_SETMAXTIPWIDTH, 0, SHRT_MAX);
	m_ttip.SetBehaviour(PPTOOLTIP_MULTIPLE_SHOW);
	m_ttip.SetNotify(m_hWnd);
	m_ttip.AddTool(&m_FriendListCtrl, _T(""));

	Localize();

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChatWnd::DoResize(int iDelta)
{
	CSplitterControl::ChangeWidth(this, IDC_LIST2, iDelta);
	CSplitterControl::ChangeWidth(this, IDC_CHATSEL, -iDelta, CW_RIGHTALIGN);
	CSplitterControl::ChangeWidth(this, IDC_CMESSAGE, -iDelta, CW_RIGHTALIGN);
	CSplitterControl::ChangePos(this, IDC_MESSAGES_LBL, -iDelta, 0);
	CSplitterControl::ChangePos(this, IDC_MESSAGEICON, -iDelta, 0);

	CRect rc;

	GetDlgItem(IDC_LIST2)->GetWindowRect(rc);
	ScreenToClient(rc);

	g_App.m_pPrefs->SetSplitterbarPositionFriend(rc.right + SPLITTER_MARGIN);

	GetClientRect(rc);

	RemoveAnchor(m_wndSplitterChat);
	AddAnchor(m_wndSplitterChat, TOP_LEFT);

	RemoveAnchor(IDC_LIST2);
	AddAnchor(IDC_LIST2, TOP_LEFT, BOTTOM_LEFT);

	RemoveAnchor(IDC_CHATSEL);
	AddAnchor(IDC_CHATSEL, TOP_LEFT, BOTTOM_RIGHT);

	RemoveAnchor(IDC_MESSAGES_LBL);
	AddAnchor(IDC_MESSAGES_LBL, TOP_LEFT);

	RemoveAnchor(IDC_MESSAGEICON);
	AddAnchor(IDC_MESSAGEICON, TOP_LEFT);

	RemoveAnchor(IDC_CMESSAGE);
	AddAnchor(IDC_CMESSAGE, BOTTOM_LEFT, BOTTOM_RIGHT);

	m_wndSplitterChat.SetRange(rc.left + SPLITTER_RANGE_WIDTH, rc.left + SPLITTER_RANGE_HEIGHT);

	Invalidate();
	UpdateWindow();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
LRESULT CChatWnd::DefWindowProc(UINT Msg, WPARAM wParam, LPARAM lParam) 
{
	CRect rc;

	switch (Msg)
	{
		case WM_PAINT:
			GetClientRect(rc);
			if (rc.Width() > 0)
			{
				GetDlgItem(IDC_LIST2)->GetWindowRect(rc);
				ScreenToClient(rc);

				rc.left = rc.right + SPLITTER_MARGIN;
				rc.right = rc.left + SPLITTER_WIDTH;

				m_wndSplitterChat.MoveWindow(rc, TRUE);
			}
			break;

		case WM_NOTIFY:
			if (wParam == IDC_SPLITTER_FRIEND)
				DoResize((reinterpret_cast<SPC_NMHDR*>(lParam))->delta);
			break;

		case WM_WINDOWPOSCHANGED:
			GetClientRect(rc);
			if (rc.Width() > 0)
				Invalidate();
			break;

		case WM_SIZE:
			GetClientRect(rc);
			m_wndSplitterChat.SetRange(rc.left + SPLITTER_RANGE_WIDTH, rc.left + SPLITTER_RANGE_HEIGHT);
			break;
	}

	return CResizableDialog::DefWindowProc(Msg, wParam, lParam);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChatWnd::StartSession(CUpDownClient* client)
{
	g_App.m_pMDlg->SetActiveDialog(this);
	m_ctlChatSelector.StartSession(client);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChatWnd::OnShowWindow(BOOL bShow, UINT nStatus)
{
	NOPRM(nStatus);
	if (bShow)
		m_ctlChatSelector.ShowChat();
	else
		m_wndSplitterChat.CancelTracking();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChatWnd::OnBnClickedCclose()
{
	m_ctlChatSelector.EndSession();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChatWnd::OnBnClickedCsend()
{
	CString			strNewMessage;

	m_ctlInputText.GetWindowText(strNewMessage);

	strNewMessage.Trim();
	if (!strNewMessage.IsEmpty() && m_ctlChatSelector.SendMessage(strNewMessage))
		m_ctlInputText.SetWindowText(_T(""));
	m_ctlInputText.SetFocus();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CChatWnd::PreTranslateMessage(MSG* pMsg)
{
	EMULE_TRY

	if (pMsg->message == WM_KEYDOWN && (pMsg->hwnd == m_ctlInputText.m_hWnd))
	{
		if (pMsg->wParam == VK_RETURN)
		{
			OnBnClickedCsend();
			return TRUE;
		}

		if (pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN)
		{
			ScrollHistory(pMsg->wParam == VK_DOWN);
			return TRUE;
		}
	}

	if (g_App.m_pPrefs->GetToolTipDelay() != 0)
		m_ttip.RelayEvent(pMsg);

	return CResizableDialog::PreTranslateMessage(pMsg);

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CChatWnd::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR* pNMHDR = (NMHDR*)lParam;

	switch(pNMHDR->code)
	{
		case UDM_TOOLTIP_DISPLAY:
		{
			NM_PPTOOLTIP_DISPLAY *pNotify = (NM_PPTOOLTIP_DISPLAY*)lParam;

			GetInfo4ToolTip(pNotify);
			return TRUE;
		}
		case UDM_TOOLTIP_POP:
		{
			m_ttip.Pop();
			return TRUE;
		}
	}

	return CResizableDialog::OnNotify(wParam, lParam, pResult);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChatWnd::GetInfo4ToolTip(NM_PPTOOLTIP_DISPLAY *pNotify)
{ 
	int						iControlId = CWnd::FromHandle(pNotify->ti->hWnd)->GetDlgCtrlID();

	if (iControlId == IDC_LIST2)
	{
		int					iSel = GetItemUnderMouse(&m_FriendListCtrl);

		if (iSel < 0 || iSel == 65535)
			return;

		CFriend             *pFriend = reinterpret_cast<CFriend*>(m_FriendListCtrl.GetItemData(iSel));

		if (pFriend == NULL)
			return;

		CString				strUserName = pFriend->m_strName, strLaseSeen, strLastChat, strCountry;

		strUserName.Replace(_T("<"), _T("<<"));

		if (g_App.m_pIP2Country->IsIP2Country())
				strCountry.Format(_T(" (<b>%s</b>)"), g_App.m_pIP2Country->GetCountryNameByIndex(g_App.m_pIP2Country->GetCountryFromIP(pFriend->m_dwLastUsedIP)));

		if (pFriend->GetLastSeen() == 0)
			GetResString(&strLaseSeen, IDS_NEVER);
		else
		{
			SYSTEMTIME		st;

			CTime(pFriend->GetLastSeen()).GetAsSystemTime(st);
			strLaseSeen = COleDateTime(st).Format(_T("%c"));
		}

		if (pFriend->m_dwLastChatted == NULL)
			GetResString(&strLastChat, IDS_NEVER);
		else
		{
			SYSTEMTIME		st;

			CTime(pFriend->m_dwLastChatted).GetAsSystemTime(st);
			strLastChat = COleDateTime(st).Format(_T("%c"));
		}

		pNotify->ti->sTooltip.Format(_T("<t=1><b>%s</b><br><hr=100%%><br><b>%s<t></b>%s<br><b>%s<t></b>%s:%u%s<br><b>%s:<t></b>%s<br><b>%s:<t></b>%s"),
			strUserName,
			GetResString(IDS_CD_UHASH), HashToString(pFriend->GetUserHash()),
			GetResString(IDS_CD_UIP), ipstr(pFriend->m_dwLastUsedIP), pFriend->m_nLastUsedPort, strCountry,
			GetResString(IDS_LASTSEEN), strLaseSeen,
			GetResString(IDS_TT_LAST_CHAT), strLastChat);

		int			iImageIndex;

		if (pFriend->GetLinkedClient() == NULL)
			iImageIndex = 0;
#ifdef OLD_SOCKETS_ENABLED
		else if (pFriend->GetLinkedClient()->m_pRequestSocket && pFriend->GetLinkedClient()->m_pRequestSocket->IsConnected())
			iImageIndex = 2;
#endif //OLD_SOCKETS_ENABLED
		else
			iImageIndex = 1;

		pNotify->ti->hIcon = m_FriendListCtrl.m_imageList.ExtractIcon(iImageIndex);
	}
} 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
void CChatWnd::Localize()
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_MESSAGES_LBL, IDS_MESSAGES },
		{ IDC_CSEND, IDS_CW_SEND },
		{ IDC_CCLOSE, IDS_CW_CLOSE }
	};

	if (m_hWnd)
	{
		for (unsigned ui = 0; ui < ARRSIZE(s_auResTbl); ui++)
			SetDlgItemText(s_auResTbl[ui][0], GetResString(s_auResTbl[ui][1]));
		m_FriendListCtrl.Localize();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CChatWnd::OnCloseTab(WPARAM wparam, LPARAM lparam)
{
	TCITEM		tcItem;
	int			iItem = (int)wparam;
	NOPRM(lparam);

	tcItem.mask = TCIF_PARAM;
	m_ctlChatSelector.GetItem(iItem, &tcItem);
	
	m_ctlChatSelector.EndSession(((CChatItem*)tcItem.lParam)->m_pClient);

	if (iItem > 0)
		iItem--;

	if (iItem >= 0)
	{
		m_ctlChatSelector.SetCurSel(iItem);
		m_ctlChatSelector.ShowChat();
	}

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CChatWnd::OnTabProperties(WPARAM wparam, LPARAM lparam)
{
	TCITEM		tcItem;
	int			iItem = (int)wparam;
	NOPRM(lparam);

	tcItem.mask = TCIF_PARAM;
	m_ctlChatSelector.GetItem(iItem, &tcItem);

	CClientDetails		dialog(IDS_CD_TITLE, reinterpret_cast<CChatItem*>(tcItem.lParam)->m_pClient, this, 0);
	dialog.DoModal();

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChatWnd::ScrollHistory(bool down)
{
	CChatItem		*pChatItem = m_ctlChatSelector.GetCurrentChatItem();

	if ((pChatItem == NULL) || ((pChatItem->m_iHistoryIndex == 0 && !down)
		|| (pChatItem->m_iHistoryIndex == pChatItem->m_strHistoryArray.GetCount() && down)))
		return;

	if (down)
		++pChatItem->m_iHistoryIndex;
	else
		--pChatItem->m_iHistoryIndex;

	CString			strBuffer;

	if (pChatItem->m_iHistoryIndex != pChatItem->m_strHistoryArray.GetCount())
		strBuffer = pChatItem->m_strHistoryArray.GetAt(pChatItem->m_iHistoryIndex);

	m_ctlInputText.SetWindowText(strBuffer);
	m_ctlInputText.SetSel(strBuffer.GetLength(), strBuffer.GetLength());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChatWnd::OnDestroy()
{
	CResizableDialog::OnDestroy();
	DestroyIcon(GetMessageStatic()->SetIcon(oldMessageIcon));
	DestroyIcon(GetFriendsStatic()->SetIcon(oldFriendsIcon));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
