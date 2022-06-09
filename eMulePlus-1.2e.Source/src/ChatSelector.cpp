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
#include "ChatSelector.h"
#include "Friend.h"
#include "packets.h"
#include "otherfunctions.h"
#include "IP2Country.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


CChatItem::CChatItem()
{
	m_pClient = NULL;
	m_pLog = NULL;
	m_strMessagePending.Empty();
	m_bNotify = false;
	m_iHistoryIndex = 0;
}

//	CChatSelector

IMPLEMENT_DYNAMIC(CChatSelector, CClosableTabCtrl)
CChatSelector::CChatSelector()
{
	lastemptyicon = false;
	blinkstate = false;
	m_Timer = 0;
}

CChatSelector::~CChatSelector()
{
}


BEGIN_MESSAGE_MAP(CChatSelector, CClosableTabCtrl)
	ON_WM_TIMER()
	ON_NOTIFY_REFLECT(TCN_SELCHANGE, OnTcnSelchangeChatsel)
	ON_WM_SIZE()
	ON_NOTIFY(EN_LINK, IDC_CHATOUT, OnEnLinkMessageBox)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	CChatSelector message handlers
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChatSelector::Init()
{
	CRect	rcRect;

	GetClientRect(&rcRect);
	AdjustRect(false, rcRect);
	rcRect.DeflateRect(7, 7);
	ModifyStyle(0, WS_CLIPCHILDREN);
	chatout.Create(WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_NOHIDESEL, rcRect, this, IDC_CHATOUT);
	chatout.ModifyStyleEx(0, WS_EX_STATICEDGE, SWP_FRAMECHANGED);
	chatout.SetFont(&g_App.m_pMDlg->m_fontDefault);
	chatout.SetTitle(GetResString(IDS_MESSAGES));
	chatout.m_dwFlags |= (HTC_ISLIMITED | HTC_ISWORDWRAP);
	chatout.AppendText(CLIENT_NAME, CSTRLEN(CLIENT_NAME), RGB(153,51,102), CLR_DEFAULT, HTC_LINK);
	chatout.AppendText(_T(" v") CURRENT_VERSION_LONG _T(" - ") + GetResString(IDS_CHAT_WELCOME) + _T('\n'), RGB(153,51,102), CLR_DEFAULT, HTC_HAVENOLINK);

	VERIFY((m_Timer = SetTimer(20, 1500, 0)));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CChatItem* CChatSelector::StartSession(CUpDownClient* pClient, bool bForceFocus /* true */)
{
	if (pClient == NULL || !g_App.m_pMDlg->IsRunning())
		return NULL;

	EMULE_TRY

	if (GetTabByClient(pClient) != -1)
	{
		SetCurSel(GetTabByClient(pClient));
		if (bForceFocus)
			ShowChat();
		return NULL;
	}

	CChatItem	*pChatItem = new CChatItem();
	CRect		rcRect;

	pChatItem->m_pClient = pClient;
	pChatItem->m_pLog = new CHTRichEditCtrl;

	GetClientRect(&rcRect);
	AdjustRect(false, rcRect);
	rcRect.left += 3;
	rcRect.top += 4;
	rcRect.right -= 3;
	rcRect.bottom -= 3;

	if (GetItemCount() == 0)
		rcRect.top += 20;

	pChatItem->m_pLog->Create(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_NOHIDESEL, rcRect, this, (UINT)-1);
	pChatItem->m_pLog->ModifyStyleEx(0, WS_EX_STATICEDGE, SWP_FRAMECHANGED);
	pChatItem->m_pLog->SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(3, 3));
	pChatItem->m_pLog->SetFont(&g_App.m_pMDlg->m_fontDefault);
	pChatItem->m_pLog->m_dwFlags |= HTC_ISWORDWRAP;
	pChatItem->m_pLog->SetTargetDevice(NULL, 0);

	COleDateTime	timelog(COleDateTime::GetCurrentTime());
	CString			strName = pClient->GetUserName(), strCountry, strTemp;

	if (g_App.m_pIP2Country->IsIP2Country())
		strCountry.Format(_T(" (%s)"), pClient->GetCountryName());
	strTemp.Format(_T("*** %s: %s (%s: %s:%u%s) - %s\n"), 
		GetResString(IDS_CHAT_START),
		strName,
		GetResString(IDS_IP), pClient->GetFullIP(), pClient->GetUserPort(), strCountry,
		timelog.Format(_T("%c")));
	pChatItem->m_pLog->AppendText(strTemp, RGB(255, 0, 0));
	pClient->SetChatState(MS_CHATTING);
	if (pClient->IsFriend())
		pClient->m_pFriend->m_dwLastChatted = time(NULL);

	if (strName.GetLength() > 30)
	{
		strName.Truncate(30);
		strName += _T("...");
	}
	else if (strName.IsEmpty())
		strName.Format(_T("[%s]"), GetResString(IDS_UNKNOWN));

	TCITEM	tcitem;

	tcitem.mask = TCIF_PARAM | TCIF_TEXT | TCIF_IMAGE;
	tcitem.lParam = (LPARAM)pChatItem;
	tcitem.pszText = (TCHAR*)strName.GetString();
	tcitem.iImage = 0;
	
	int	iResult = InsertItem(GetItemCount(), &tcitem);

	g_App.m_pMDlg->m_wndChat.m_ctlCloseButton.EnableWindow(true);
	g_App.m_pMDlg->m_wndChat.m_ctlSendButton.EnableWindow(true);

	if (iResult != -1 && IsWindowVisible())
	{
		SetCurSel(iResult);
		pChatItem->m_pLog->SetTitle(pClient->GetUserName());
		if (bForceFocus)
			ShowChat();
	}

	return pChatItem;

	EMULE_CATCH

	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CChatSelector::GetTabByClient(CUpDownClient *pClient)
{
	TCITEM tcitem;

	for (int i = 0; i < GetItemCount(); i++)
	{
		tcitem.mask = TCIF_PARAM;
		if (GetItem(i, &tcitem) && ((CChatItem*)tcitem.lParam)->m_pClient == pClient)
			return i;
	}
	return -1;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CChatItem* CChatSelector::GetItemByClient(CUpDownClient* client)
{
	TCITEM	tcitem;

	for (int i = 0; i < GetItemCount(); i++)
	{
		tcitem.mask = TCIF_PARAM;
		if (GetItem(i, &tcitem) && ((CChatItem*)tcitem.lParam)->m_pClient == client)
			return(CChatItem*)tcitem.lParam;
	}
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChatSelector::ProcessMessage(CUpDownClient* pSender, const CString &strIncomingMessage)
{
	static const TCHAR *s_apcMsgBanFilter[] =
	{	// Add new clowns on top; All strings in lower case
		_T("tyrantmule"),
		_T("rocketmule"),
		_T("speedshare"),
		_T("emule fx"),
		_T("zambor"),
		_T("fastest emule ever"),
		_T("robot from riaa"),
		_T("ketamine"),
		_T("di-emule"),
		_T("http://www.chez.com/theworld/"),
		_T("http://fullspeed.to/mison")
	};
	CString	strMessage(strIncomingMessage);
	int		iCurPos = 0;

	strMessage.MakeLower();

//	Ban spammers
	if (g_App.m_pPrefs->IsCounterMeasures())
	{
		for (unsigned ui = 0; ui < ARRSIZE(s_apcMsgBanFilter); ui++)
		{
			if (strMessage.Find(s_apcMsgBanFilter[ui]) >= 0)
			{
				AddLogLine(LOG_FL_DBG | LOG_RGB_DIMMED, _T("Anti-leecher: Client %s has been banned because of spamming"), pSender->GetClientNameWithSoftware());
				pSender->Ban(BAN_CLIENT_SPAMMING);
				return;
			}
		}
	}

	CString	strResToken, strFilter = g_App.m_pPrefs->GetMessageFilter();

	strFilter.MakeLower();
	for (;;)
	{
		strResToken = strFilter.Tokenize(_T("|"), iCurPos);
		if (strResToken.IsEmpty())
			break;
		if (strMessage.Find(strResToken) >= 0)
		{
			AddLogLine(LOG_FL_DBG | LOG_RGB_DIMMED, _T("Filtered message '%s' from client %s"), strIncomingMessage, pSender->GetClientNameWithSoftware());
			return;
		}
	}

	CChatItem	*pChatItem = GetItemByClient(pSender);
	bool		bIsNewChatWindow = false;

	if (pChatItem == NULL)
	{
		if (GetItemCount() >= 50)
		{
			AddLogLine(LOG_FL_DBG | LOG_RGB_WARNING, _T("Instant Messaging: Messages limit reached"));
			return;
		}

		if (g_App.m_pPrefs->GetAcceptMessagesFrom() == 4) //no messages
			return;

		if ((g_App.m_pPrefs->GetAcceptMessagesFrom() == 2) && !pSender->IsFriend()) //only friends
			return;

		if ((g_App.m_pPrefs->GetAcceptMessagesFrom() == 3) && !pSender->IsFriend()) //log non friends
		{
			AddLogLine(0, GetResString(IDS_IM_MSGFROMCHAT) + _T(" %s: %s"), pSender->GetUserName(), strIncomingMessage);
			pSender->SetChatState(MS_NONE);
			return;
		}

		pChatItem = StartSession(pSender);
		if (pChatItem == NULL)
			return;
		bIsNewChatWindow = true;
	}
	COleDateTime	timelog(COleDateTime::GetCurrentTime());

	strMessage.Format(_T("%s (%s): "), pSender->GetUserName(), timelog.Format(_T("%c")));
	pChatItem->m_pLog->AppendText(strMessage, RGB(50, 200, 250));
	pChatItem->m_pLog->AppendText(strIncomingMessage + _T('\n'));

	if (g_App.m_pPrefs->GetAwayState())
	{
		if ((::GetTickCount() - pChatItem->m_pClient->GetAwayMessageResendCount()) > 3000)
		{ //send again only if 3 secs from last away message
			SendAwayMessage(pChatItem);
			pChatItem->m_pClient->SetAwayMessageResendCount(::GetTickCount());
		}
	}

	if ((iCurPos = GetTabByClient(pSender)) != GetCurSel())
		SetItemState(iCurPos, TCIS_HIGHLIGHTED, TCIS_HIGHLIGHTED);
	if (!g_App.m_pPrefs->GetAwayState())
	{
		BOOL	bVisible = ::IsWindowVisible(::GetParent(m_hWnd));

	//	Show statusbar indicator if Messages window isn't active or our application isn't topmost one
		if (!bVisible || (g_App.m_pMDlg->m_hWnd != ::GetForegroundWindow()))
		{
			pChatItem->m_bNotify = true;
		//	Send Notification is required
			if ( ( g_App.m_pPrefs->GetUseChatNotifier() &&
				(bIsNewChatWindow || g_App.m_pPrefs->GetNotifierPopsEveryChatMsg()) ) )
			{
				strMessage.Format(_T("%s %s:'%s'\n"), GetResString(IDS_TBN_NEWCHATMSG), pSender->GetUserName(), strIncomingMessage);
				g_App.m_pMDlg->SendMail(strMessage, true, g_App.m_pPrefs->IsSMTPInfoEnabled());
				g_App.m_pMDlg->ShowNotifier(strMessage, TBN_CHAT, false, true);
			}
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CChatSelector::SendMessage(const CString& message)
{
	if (!g_App.m_pMDlg->IsRunning())
		return false;

	CChatItem	*pChatItem = GetCurrentChatItem();

	if (pChatItem == NULL || pChatItem->m_pClient == NULL)
		return false;

	if (pChatItem->m_strHistoryArray.GetCount() == g_App.m_pPrefs->GetMaxChatHistoryLines())
		pChatItem->m_strHistoryArray.RemoveAt(0);
	pChatItem->m_strHistoryArray.Add(CString(message));
	pChatItem->m_iHistoryIndex = pChatItem->m_strHistoryArray.GetCount();

	if (pChatItem->m_pClient->GetChatState() == MS_CONNECTING)
		return false;

#ifdef OLD_SOCKETS_ENABLED
	if (pChatItem->m_pClient->IsHandshakeFinished())
	{
		SendMessagePacket(*pChatItem, message);

		COleDateTime	timelog(COleDateTime::GetCurrentTime());
		CString			strTmp;

		strTmp.Format(_T("%s (%s): "), g_App.m_pPrefs->GetUserNick(), timelog.Format(_T("%c")));
		pChatItem->m_pLog->AppendText(strTmp, RGB(1, 180, 20));
		pChatItem->m_pLog->AppendText(message + _T('\n'));
	}
	else
	{
		pChatItem->m_pLog->AppendText(_T("*** ") + GetResString(IDS_CONNECTING), RGB(255, 0, 0), CLR_DEFAULT, HTC_HAVENOLINK);
		pChatItem->m_strMessagePending = message;
		pChatItem->m_pClient->SetChatState(MS_CONNECTING);
		pChatItem->m_pClient->TryToConnect();
	}
#endif //OLD_SOCKETS_ENABLED
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChatSelector::ConnectingResult(CUpDownClient* sender, bool bSuccess)
{
	CChatItem	*pChatItem = GetItemByClient(sender);

	if (pChatItem == NULL || pChatItem->m_pClient == NULL)
		return;

	pChatItem->m_pClient->SetChatState(MS_CHATTING);

	if (!bSuccess)
	{
		if (!pChatItem->m_strMessagePending.IsEmpty())
		{
			pChatItem->m_pLog->AppendText(_T(' ') + GetResString(IDS_FAILED) + _T('\n'), RGB(255, 0, 0), CLR_DEFAULT, HTC_HAVENOLINK);
			pChatItem->m_strMessagePending.Empty();
		}
#ifdef _DEBUG
		else
			pChatItem->m_pLog->AppendText(GetResString(IDS_CHATDISCONNECTED) + _T('\n'), RGB(255, 0, 0), CLR_DEFAULT, HTC_HAVENOLINK);
#endif
	}
	else if (!pChatItem->m_strMessagePending.IsEmpty())
	{
		pChatItem->m_pLog->AppendText(_T(" ok\n"), CSTRLEN(_T(" ok\n")), RGB(255, 0, 0), CLR_DEFAULT, HTC_HAVENOLINK);
		SendMessagePacket(*pChatItem, pChatItem->m_strMessagePending);

		COleDateTime	timelog(COleDateTime::GetCurrentTime());
		CString			strTmp;

		strTmp.Format(_T("%s (%s): "), g_App.m_pPrefs->GetUserNick(), timelog.Format(_T("%c")));
		pChatItem->m_pLog->AppendText(strTmp, RGB(1, 180, 20));
		pChatItem->m_pLog->AppendText(pChatItem->m_strMessagePending + _T('\n'));
		pChatItem->m_strMessagePending.Empty();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChatSelector::DeleteAllItems()
{
	TCITEM	tcitem;

	for (int i = 0; i < GetItemCount(); i++)
	{
		tcitem.mask = TCIF_PARAM;
		if (GetItem(i, &tcitem))
			delete (CChatItem*)tcitem.lParam;
	}
	chatout.ShowWindow(SW_SHOW);
	g_App.m_pMDlg->m_wndChat.m_ctlCloseButton.EnableWindow(false);
	g_App.m_pMDlg->m_wndChat.m_ctlSendButton.EnableWindow(false);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChatSelector::OnTimer(UINT_PTR nIDEvent)
{
	NOPRM(nIDEvent);
	blinkstate = !blinkstate;

	bool	globalnotify = false;
	TCITEM	tcitem;

	for (int i = 0; i < GetItemCount(); i++)
	{
		tcitem.mask = TCIF_PARAM;
		if (!GetItem(i, &tcitem))
			break;

		if (((CChatItem*)tcitem.lParam)->m_bNotify)
		{
			globalnotify = true;
			break;
		}
	}
	if (globalnotify)
	{
		g_App.m_pMDlg->ShowMessageState(((blinkstate) ? 1 : 2));
		lastemptyicon = false;
	}
	else if (!lastemptyicon)
	{
		g_App.m_pMDlg->ShowMessageState(0);
		lastemptyicon = true;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CChatItem* CChatSelector::GetCurrentChatItem()
{
	int	iCurSel = GetCurSel();

	if (iCurSel == -1)
		return NULL;

	TCITEM	tcitem;

	tcitem.mask = TCIF_PARAM;
	if (!GetItem(iCurSel, &tcitem))
		return NULL;
	return (CChatItem*)tcitem.lParam;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChatSelector::ShowChat()
{
	int	iCurSel = GetCurSel();

	if (iCurSel == -1)
		return;

	bool	bWasChanged = (GetItemState(iCurSel, TCIS_HIGHLIGHTED) & TCIS_HIGHLIGHTED) ? true : false;

	SetItemState(iCurSel, TCIS_HIGHLIGHTED, NULL);

	CChatItem	*pChatItem = GetCurrentChatItem(), *pUpdateItem;

	if (pChatItem == NULL)
		return;

	pChatItem->m_pLog->SetRedraw(false);
	pChatItem->m_pLog->ShowWindow(SW_SHOW);
	if ((pChatItem->m_pLog->m_dwFlags & HTC_ISAUTOSCROLL) != 0 && bWasChanged)
		pChatItem->m_pLog->ScrollToLastLine();
	pChatItem->m_pLog->SetRedraw(true);

	TCITEM	tcitem;
	int		i = 0;

	tcitem.mask = TCIF_PARAM;
	while (GetItem(i++, &tcitem))
	{
		pUpdateItem = (CChatItem*)tcitem.lParam;
		if (pUpdateItem != pChatItem)
			pUpdateItem->m_pLog->ShowWindow(SW_HIDE);
	}

	pChatItem->m_pLog->SetTitle(pChatItem->m_pClient->GetUserName());
	pChatItem->m_bNotify = false;
	g_App.m_pMDlg->m_wndChat.m_ctlInputText.SetFocus();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChatSelector::OnTcnSelchangeChatsel(NMHDR *pNMHDR, LRESULT *pResult)
{
	NOPRM(pNMHDR);
	ShowChat();
	*pResult = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
INT	CChatSelector::InsertItem(int nItem, TCITEM* pTabCtrlItem)
{
	if (GetItemCount() == 0)
		chatout.ShowWindow(SW_HIDE);

	int	iResult = CClosableTabCtrl::InsertItem(nItem, pTabCtrlItem);

	RedrawWindow();
	return iResult;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CChatSelector::DeleteItem(int nItem)
{
	CClosableTabCtrl::DeleteItem(nItem);
	if (GetItemCount() == 0)
	{
		chatout.ShowWindow(SW_SHOW);
		g_App.m_pMDlg->m_wndChat.m_ctlCloseButton.EnableWindow(false);
		g_App.m_pMDlg->m_wndChat.m_ctlSendButton.EnableWindow(false);
	}
	RedrawWindow();
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChatSelector::EndSession(CUpDownClient* client, bool bForceFocus /* true */)
{
	if (!g_App.m_pMDlg->IsRunning())
		return;

	int	iCurSel;

	if (client != NULL)
		iCurSel = GetTabByClient(client);
	else
		iCurSel = GetCurSel();

	if (iCurSel == -1)
		return;

	TCITEM	tcitem;
	
	tcitem.mask = TCIF_PARAM;

	if (!GetItem(iCurSel, &tcitem) || tcitem.lParam == 0)
		return;

	CChatItem	*pChatItem = (CChatItem*)tcitem.lParam;

	pChatItem->m_pClient->SetChatState(MS_NONE);

	DeleteItem(iCurSel);
	delete pChatItem;

	int	iTabItems = GetItemCount();

	if (iTabItems > 0)
	{
	//	Select next tab
		if (iCurSel >= iTabItems)
			iCurSel = iTabItems - 1;
		SetCurSel(iCurSel);				// Returns CB_ERR if error or no prev. selection(!)
		iCurSel = GetCurSel();			// get the real current selection
		if (iCurSel == CB_ERR)			// if still error
			iCurSel = SetCurSel(0);
		if (bForceFocus)
			ShowChat();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChatSelector::OnSize(UINT nType, int cx, int cy)
{
	CClosableTabCtrl::OnSize(nType, cx, cy);

	CRect	rcRect;

	GetClientRect(&rcRect);
	AdjustRect(false, rcRect);
	rcRect.left += 3;
	rcRect.top += 4;
	rcRect.right -= 3;
	rcRect.bottom -= 3;

	if (GetItemCount() > 0)
		rcRect.top -= 20;

	chatout.SetWindowPos(NULL, rcRect.left, rcRect.top, rcRect.Width(), rcRect.Height(), SWP_NOZORDER);

	TCITEM		tcitem;
	CChatItem	*pChatItem;
	int			i = 0;

	tcitem.mask = TCIF_PARAM;
	while (GetItem(i++, &tcitem))
	{
		pChatItem = (CChatItem*)tcitem.lParam;
		pChatItem->m_pLog->SetWindowPos(NULL, rcRect.left, rcRect.top + 20, rcRect.Width(), rcRect.Height() - 20, SWP_NOZORDER);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CChatSelector::PreTranslateMessage(MSG* pMsg)
{
	return CClosableTabCtrl::PreTranslateMessage(pMsg);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChatSelector::UpdateFont()
{
	chatout.SetFont(&g_App.m_pMDlg->m_fontDefault);

	TCITEM		tcitem;
	CChatItem	*pChatItem;
	int			i = 0;
	
	tcitem.mask = TCIF_PARAM;
	while (GetItem(i++, &tcitem))
	{
		pChatItem = (CChatItem*)tcitem.lParam;
		pChatItem->m_pLog->SetFont(&g_App.m_pMDlg->m_fontDefault);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChatSelector::OnEnLinkMessageBox(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;

	ENLINK	*pEnLink = reinterpret_cast<ENLINK *>(pNMHDR);

	if (pEnLink && pEnLink->msg == WM_LBUTTONDOWN)
	{
		CString	strUrl;

		chatout.GetTextRange(pEnLink->chrg.cpMin, pEnLink->chrg.cpMax, strUrl);
		if (strUrl == CLIENT_NAME)
			ShellExecute(NULL, NULL, _T("http://emuleplus.info"), NULL, NULL, SW_SHOWDEFAULT);
		*pResult = 1;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChatSelector::OnDestroy()
{
	if (m_Timer != NULL)
		KillTimer(m_Timer);
	CClosableTabCtrl::OnDestroy();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CChatSelector::SendAwayMessage(CChatItem* ci)
{
	if (ci == NULL  || ci->m_pClient == NULL || ci->m_pClient->GetChatState() == MS_CONNECTING) //send AwayMessage only if connected
		return false;

#ifdef OLD_SOCKETS_ENABLED
	if (ci->m_pClient->IsHandshakeFinished())
	{
		CString	strAwayMessage = _T("[AUTO REPLY:AWAY] ") + g_App.m_pPrefs->GetAwayStateMessage();

		SendMessagePacket(*ci, strAwayMessage);

		COleDateTime	timelog(COleDateTime::GetCurrentTime());
		CString			strTmp;

		strTmp.Format(_T("%s (%s): "), g_App.m_pPrefs->GetUserNick(), timelog.Format(_T("%c")));
		ci->m_pLog->AppendText(strTmp, RGB(1, 180, 20));
		ci->m_pLog->AppendText(strAwayMessage + _T('\n'));
	}
#endif //OLD_SOCKETS_ENABLED
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CChatSelector::SendMessagePacket(CChatItem &ci, const CString &strMessage)
{
	CStringA	strEncodedMsg;

	Str2MB(ci.m_pClient->GetStrCodingFormat(), &strEncodedMsg, strMessage);

	uint32		dwMsgLen = strEncodedMsg.GetLength();
	Packet		*pPacket = new Packet(OP_MESSAGE, dwMsgLen + 2);

	POKE_WORD(pPacket->m_pcBuffer, static_cast<uint16>(dwMsgLen));
	memcpy2(pPacket->m_pcBuffer + 2, strEncodedMsg.GetString(), dwMsgLen);
	g_App.m_pUploadQueue->AddUpDataOverheadOther(pPacket->m_dwSize);
#ifdef OLD_SOCKETS_ENABLED
	ci.m_pClient->m_pRequestSocket->SendPacket(pPacket, true, true);
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
