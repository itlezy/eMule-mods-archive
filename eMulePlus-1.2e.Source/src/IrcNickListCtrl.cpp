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

#include "stdafx.h"
#include "IrcNickListCtrl.h"
#include "IrcWnd.h"
#include "otherfunctions.h"
#include "TitleMenu.h"
#include "emule.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CIrcNickListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CIrcNickListCtrl, CMuleListCtrl)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblClk)
END_MESSAGE_MAP()

CIrcNickListCtrl::CIrcNickListCtrl()
{
	m_pParent = NULL;
	memset(&m_bSortAscending, true, sizeof(m_bSortAscending));
	SetGeneralPurposeFind(true);
}

void CIrcNickListCtrl::Init()
{
	static const uint16 s_auColHdr[][2] =
	{
		{ LVCFMT_LEFT, 90 },	// IRC1COL_NICK
		{ LVCFMT_LEFT, 70 }		// IRC1COL_STATUS
	};

	for (unsigned ui = 0; ui < ARRSIZE(s_auColHdr); ui++)
		InsertColumn(ui, _T(""), static_cast<int>(s_auColHdr[ui][0]), static_cast<int>(s_auColHdr[ui][1]));

	Localize();
	LoadSettings(CPreferences::TABLE_IRCNICK);

	int		iSortCode = g_App.m_pPrefs->GetColumnSortItem(CPreferences::TABLE_IRCNICK);

	iSortCode |= g_App.m_pPrefs->GetColumnSortAscending(CPreferences::TABLE_IRCNICK) ? MLC_SORTASC : MLC_SORTDESC;
	SortInit(iSortCode);
}

void CIrcNickListCtrl::Localize()
{
	static const uint16 s_auResTbl[] =
	{
		IDS_NICK,		// IRC1COL_NICK
		IDS_STATUS	// IRC1COL_STATUS
	};
	CHeaderCtrl	*pHeaderCtrl = GetHeaderCtrl();
	CString		strRes;
	HDITEM		hdi;

	hdi.mask = HDI_TEXT;

	for (unsigned ui = 0; ui < ARRSIZE(s_auResTbl); ui++)
	{
		::GetResString(&strRes, static_cast<UINT>(s_auResTbl[ui]));
		hdi.pszText = const_cast<LPTSTR>(strRes.GetString());
		pHeaderCtrl->SetItem(static_cast<int>(ui), &hdi);
	}
	UpdateNickCount();
}

void CIrcNickListCtrl::UpdateNickCount()
{
	CString	strTitle;

	strTitle.Format(_T("%s (%u)"), GetResString(IDS_UUSERS), GetItemCount());
	g_App.m_pMDlg->m_wndIRC.SetDlgItemText(IDC_IRC_USERS_LBL, strTitle);
}

void CIrcNickListCtrl::SortInit(int iSortCode)
{
	int		iSortColumn = (iSortCode & MLC_COLUMNMASK);	//	Get the sort column
	bool	bSortAscending = (iSortCode & MLC_SORTDESC) == 0;	//	Get the sort order

	SetSortArrow(iSortColumn, bSortAscending);
	SortItems(SortProc, iSortCode);
	m_bSortAscending[iSortColumn] = bSortAscending;
}

void CIrcNickListCtrl::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW		*pNMListView = reinterpret_cast<NM_LISTVIEW*>(pNMHDR);
	int				iSubItem = pNMListView->iSubItem;
	bool			bSortOrder = m_bSortAscending[iSubItem];

// Reverse sorting direction for the same column and keep the same if column was changed
	if (static_cast<int>(m_dwParamSort & MLC_COLUMNMASK) == iSubItem)
		m_bSortAscending[iSubItem] = bSortOrder = !bSortOrder;

	SetSortArrow(iSubItem, bSortOrder);
	SortItems(SortProc, iSubItem + ((bSortOrder) ? MLC_SORTASC : MLC_SORTDESC));
	g_App.m_pPrefs->SetColumnSortItem(CPreferences::TABLE_IRCNICK, iSubItem);
	g_App.m_pPrefs->SetColumnSortAscending(CPreferences::TABLE_IRCNICK, bSortOrder);
	*pResult = 0;
}

void CIrcNickListCtrl::OnNMDblClk(NMHDR *pNMHDR, LRESULT *pResult)
{
	int		iNickItem = GetSelectionMark();
	NOPRM(pNMHDR);

	if (iNickItem != -1)
	{
		Nick	*pNick = (Nick*)GetItemData(iNickItem);

		if (pNick != NULL)
			m_pParent->AddMessage(pNick->nick, _T(""), RGB(0, 147, 0), GetResString(IDS_IRC_PRIVATECHANSTART));
	}
	*pResult = 0;
}

void CIrcNickListCtrl::OnContextMenu(CWnd *pWnd, CPoint point)
{
	UINT		dwFlags = MF_STRING | ((GetSelectionMark() != -1) ? MF_ENABLED : MF_GRAYED);
	CTitleMenu	menuNick;
	NOPRM(pWnd);

	menuNick.CreatePopupMenu();
	menuNick.AddMenuTitle(GetResString(IDS_NICK));
	menuNick.AppendMenu(dwFlags, Irc_Priv, GetResString(IDS_IRC_PRIVMESSAGE));
	menuNick.AppendMenu(dwFlags, Irc_WhoIs, _T("WhoIs"));
	menuNick.AppendMenu(dwFlags, Irc_Slap, GetResString(IDS_IRC_SLAP));
	menuNick.AppendMenu(dwFlags, Irc_Owner, _T("Owner"));
	menuNick.AppendMenu(dwFlags, Irc_DeOwner, _T("DeOwner"));
	menuNick.AppendMenu(dwFlags, Irc_Op, GetResString(IDS_IRC_OP));
	menuNick.AppendMenu(dwFlags, Irc_DeOp, GetResString(IDS_IRC_DEOP));
	menuNick.AppendMenu(dwFlags, Irc_HalfOp, GetResString(IDS_IRC_HALFOP));
	menuNick.AppendMenu(dwFlags, Irc_DeHalfOp, GetResString(IDS_IRC_DEHALFOP));
	menuNick.AppendMenu(dwFlags, Irc_Voice, GetResString(IDS_IRC_VOICE));
	menuNick.AppendMenu(dwFlags, Irc_DeVoice, GetResString(IDS_IRC_DEVOICE));
	menuNick.AppendMenu(dwFlags, Irc_Protect, _T("Protect"));
	menuNick.AppendMenu(dwFlags, Irc_DeProtect, _T("DeProtect"));
	menuNick.AppendMenu(dwFlags, Irc_Kick, GetResString(IDS_IRC_KICK));
	menuNick.SetDefaultItem(Irc_Priv);
	menuNick.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this, NULL);

//	Menu objects are destroyed in their destructor
}

BOOL CIrcNickListCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int			iNickItem = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	int			iChanItem = m_pParent->channelselect.GetCurSel();
	Nick		*pNick = (Nick*)GetItemData(iNickItem);
	TCITEM		tcitem;
	Channel		*pChannel;
	CString		strSend;
	CIrcMain	*pIrcMain = m_pParent->m_pIrcMain;
	NOPRM(lParam);

	tcitem.mask = TCIF_PARAM;
	m_pParent->channelselect.GetItem(iChanItem, &tcitem);
	pChannel = (Channel*)tcitem.lParam;

	switch (wParam)
	{
		case Irc_Priv:
			if (pNick != NULL && m_pParent->FindChannelByName(pNick->nick) == NULL)
				m_pParent->AddMessage(pNick->nick, _T(""), RGB(0, 147, 0), GetResString(IDS_IRC_PRIVATECHANSTART));
			break;

		case Irc_WhoIs:
			if (pNick != NULL)
			{
				strSend.Format(_T("WHOIS %s"), pNick->nick);
				pIrcMain->SendString(strSend);
			}
			break;

		case Irc_Owner:
			if (pNick != NULL && pChannel != NULL)
			{
				strSend.Format(_T("PRIVMSG chanserv owner %s %s"), pChannel->name, pNick->nick);
				pIrcMain->SendString(strSend);
			}
			break;

		case Irc_DeOwner:
			if (pNick != NULL && pChannel != NULL)
			{
				strSend.Format(_T("PRIVMSG chanserv deowner %s %s"), pChannel->name, pNick->nick);
				pIrcMain->SendString(strSend);
			}
			break;

		case Irc_Op:
			if (pNick != NULL && pChannel != NULL)
			{
				strSend.Format(_T("MODE %s +o %s"), pChannel->name, pNick->nick);
				pIrcMain->SendString(strSend);
			}
			break;

		case Irc_DeOp:
			if (pNick != NULL && pChannel != NULL)
			{
				strSend.Format(_T("MODE %s -o %s"), pChannel->name, pNick->nick);
				pIrcMain->SendString(strSend);
			}
			break;

		case Irc_HalfOp:
			if (pNick != NULL && pChannel != NULL)
			{
				strSend.Format(_T("MODE %s +h %s"), pChannel->name, pNick->nick);
				pIrcMain->SendString(strSend);
			}
			break;

		case Irc_DeHalfOp:
			if (pNick != NULL && pChannel != NULL)
			{
				strSend.Format(_T("MODE %s -h %s"), pChannel->name, pNick->nick);
				pIrcMain->SendString(strSend);
			}
			break;

		case Irc_Voice:
			if (pNick != NULL && pChannel != NULL)
			{
				strSend.Format(_T("MODE %s +v %s"), pChannel->name, pNick->nick);
				pIrcMain->SendString(strSend);
			}
			break;

		case Irc_DeVoice:
			if (pNick != NULL && pChannel != NULL)
			{
				strSend.Format(_T("MODE %s -v %s"), pChannel->name, pNick->nick);
				pIrcMain->SendString(strSend);
			}
			break;

		case Irc_Protect:
			if (pNick != NULL && pChannel != NULL)
			{
				strSend.Format(_T("PRIVMSG chanserv protect %s %s"), pChannel->name, pNick->nick);
				pIrcMain->SendString(strSend);
			}
			break;

		case Irc_DeProtect:
			if (pNick != NULL && pChannel != NULL)
			{
				strSend.Format(_T("PRIVMSG chanserv deprotect %s %s"), pChannel->name, pNick->nick);
				pIrcMain->SendString(strSend);
			}
			break;

		case Irc_Kick:
			if (pNick != NULL && pChannel != NULL)
			{
				strSend.Format(_T("KICK %s %s"), pChannel->name, pNick->nick);
				pIrcMain->SendString(strSend);
			}
			break;

		case Irc_Slap:
			if (pNick != NULL && pChannel != NULL)
			{
				strSend.Format(GetResString(IDS_IRC_SLAPMSGSEND), pChannel->name, pNick->nick);
				m_pParent->AddMessageF(pChannel->name, _T(""), RGB(252, 127, 0), GetResString(IDS_IRC_SLAPMSG), pIrcMain->GetNick(), pNick->nick);
				pIrcMain->SendString(strSend);
			}
			break;
	}
	return true;
}

int CIrcNickListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	Nick	*pItem1 = (Nick*)lParam1;
	Nick	*pItem2 = (Nick*)lParam2;

	int		iCompare = 0;
	int		iSortColumn = (lParamSort & MLC_COLUMNMASK);
	int		iSortMod = ((lParamSort & MLC_SORTDESC) == 0) ? 1 : -1;

	for (;;)
	{
		switch (iSortColumn)
		{
			case IRC1COL_NICK:
				iCompare = pItem1->nick.CompareNoCase(pItem2->nick);
				break;

			case IRC1COL_STATUS:
				if (pItem1->owner == _T("!"))
				{
					if (pItem2->owner != _T("!"))
						iCompare = -1;
				}
				else if (pItem2->owner == _T("!"))
					iCompare = 1;
				else if (pItem1->protect == _T("*"))
				{
					if (pItem2->protect != _T("*"))
						iCompare = -1;
				}
				else if (pItem2->protect == _T("*"))
					iCompare = 1;
				else if (pItem1->op == _T("@"))
				{
					if (pItem2->op != _T("@"))
						iCompare = -1;
				}
				else if (pItem2->op == _T("@"))
					iCompare = 1;
				else if (pItem1->hop == _T("%"))
				{
					if (pItem2->hop != _T("%"))
						iCompare = -1;
				}
				else if (pItem2->hop == _T("%"))
					iCompare = 1;
				else if (pItem1->voice == _T("+"))
				{
					if (pItem2->voice != _T("+"))
						iCompare = -1;
				}
				else if (pItem2->voice == _T("+"))
					iCompare = 1;
				else if (pItem1->uop == _T("-"))
				{
					if (pItem2->uop != _T("-"))
						iCompare = -1;
				}
				else if (pItem2->uop == _T("-"))
					iCompare = 1;

				if (iCompare == 0)
				{
					iSortColumn = IRC1COL_NICK;
					continue;
				}
				break;
		}
		break;
	}
	return iCompare * iSortMod;
}

Nick* CIrcNickListCtrl::NewNick(CString strChannel, CString nick)
{
	Channel		*pToAddChan = m_pParent->FindChannelByName(strChannel);
	TCHAR		cFCh;

	if (pToAddChan == NULL)
		return NULL;

	if (FindNickByName(strChannel, nick) != NULL)
		return NULL;

	Nick	   *pToAddNick = new Nick;

	if (nick.GetAt(0) == _T('~'))
		nick.SetAt(0, _T('!'));

	if (nick.GetAt(0) == _T('!'))
	{
		nick = nick.Mid(1);
		pToAddNick->owner = _T("!");
	}
	else
		pToAddNick->owner.Empty();

	if (nick.GetAt(0) == _T('&'))
		nick.SetAt(0, _T('*'));

	if (nick.GetAt(0) == _T('*'))
	{
		nick = nick.Mid(1);
		pToAddNick->protect = _T("*");
	}
	else
		pToAddNick->protect.Empty();

	if ((cFCh = nick.GetAt(0)) == _T('@'))
	{
		pToAddNick->op = _T("@");
		pToAddNick->hop.Empty();
		pToAddNick->voice.Empty();
		pToAddNick->uop.Empty();
		pToAddNick->nick = nick.Mid(1);
	}
	else if (cFCh == _T('%'))
	{
		pToAddNick->op.Empty();
		pToAddNick->hop = _T("%");
		pToAddNick->voice.Empty();
		pToAddNick->uop.Empty();
		pToAddNick->nick = nick.Mid(1);
	}
	else if (cFCh == _T('+'))
	{
		pToAddNick->op.Empty();
		pToAddNick->hop.Empty();
		pToAddNick->voice = _T("+");
		pToAddNick->uop.Empty();
		pToAddNick->nick = nick.Mid(1);
	}
	else if (cFCh == _T('-'))
	{
		pToAddNick->op.Empty();
		pToAddNick->hop.Empty();
		pToAddNick->voice.Empty();
		pToAddNick->uop = _T("-");
		pToAddNick->nick = nick.Mid(1);
	}
	else
	{
		pToAddNick->op.Empty();
		pToAddNick->hop.Empty();
		pToAddNick->voice.Empty();
		pToAddNick->nick = nick;
	}
	pToAddNick->nick.TrimLeft(_T("*@%+-"));
	pToAddChan->nicks.AddTail(pToAddNick);
	if (pToAddChan == m_pParent->GetCurrentChannel())
	{
		CString		strMode;
		int			iItems = InsertItem(LVIF_TEXT | LVIF_PARAM, GetItemCount(), pToAddNick->nick, 0, 0, 0, (LPARAM)pToAddNick);

		strMode.Format(_T("%s%s%s%s%s%s"), pToAddNick->owner, pToAddNick->protect, pToAddNick->op, pToAddNick->hop, pToAddNick->voice, pToAddNick->uop);
		SetItemText(iItems, IRC1COL_STATUS, strMode);
		UpdateNickCount();
	}
	return pToAddNick;
}

Nick* CIrcNickListCtrl::FindNickByName(CString channel, const CString &strName)
{
	Channel		*pCurChannel = m_pParent->FindChannelByName(channel);
	Nick		*pCurNick;

	if (pCurChannel == NULL)
		return 0;

	for (POSITION pos = pCurChannel->nicks.GetHeadPosition(); pos != NULL;)
	{
		pCurNick = (Nick*)pCurChannel->nicks.GetNext(pos);
		if (pCurNick->nick == strName)
			return pCurNick;
	}
	return 0;
}

void CIrcNickListCtrl::RefreshNickList(CString channel)
{
	DeleteAllItems();

	Channel		*pRefresh = m_pParent->FindChannelByName(channel);

	if (pRefresh == NULL)
		return;

	Nick		*pCurNick;
	int			iItems;
	CString		strMode;

	for (POSITION pos = pRefresh->nicks.GetHeadPosition(); pos != NULL;)
	{
		pCurNick = (Nick*)pRefresh->nicks.GetNext(pos);
		iItems = InsertItem(LVIF_TEXT | LVIF_PARAM, GetItemCount(), pCurNick->nick, 0, 0, 0, (LPARAM)pCurNick);
		strMode.Format(_T("%s%s%s%s%s%s"), pCurNick->owner, pCurNick->protect, pCurNick->op, pCurNick->hop, pCurNick->voice, pCurNick->uop);
		SetItemText(iItems, IRC1COL_STATUS, strMode);
	}
	UpdateNickCount();
}

bool CIrcNickListCtrl::RemoveNick(CString channel, const CString &strNick)
{
	Channel		*pUpdate = m_pParent->FindChannelByName(channel);

	if (pUpdate == NULL)
		return false;

	POSITION		pos1, pos2;
	Nick			*pCurNick;
	LVFINDINFO		lvdfind;
	sint32			iResult;

	for (pos1 = pUpdate->nicks.GetHeadPosition(); (pos2 = pos1) != NULL;)
	{
		pCurNick = (Nick*)pUpdate->nicks.GetNext(pos1);
		if (pCurNick->nick == strNick)
		{
			if (pUpdate == m_pParent->GetCurrentChannel())
			{
				lvdfind.flags = LVFI_PARAM;
				lvdfind.lParam = (LPARAM)pCurNick;
				iResult = FindItem(&lvdfind);
				if (iResult != -1)
				{
					DeleteItem(iResult);
					UpdateNickCount();
				}
			}
			pUpdate->nicks.RemoveAt(pos2);
			delete pCurNick;
			return true;
		}
	}
	return false;
}

void CIrcNickListCtrl::DeleteAllNick(CString channel)
{
	Channel		*pCurChannel = m_pParent->FindChannelByName(channel);

	if (pCurChannel == NULL)
		return;

	POSITION	pos1, pos2;
	Nick		*pCurNick;

	for (pos1 = pCurChannel->nicks.GetHeadPosition(); (pos2 = pos1) != NULL;)
	{
		pCurNick = (Nick*)pCurChannel->nicks.GetNext(pos1);
		pCurChannel->nicks.RemoveAt(pos2);
		delete pCurNick;
	}
}

void CIrcNickListCtrl::DeleteNickInAll(const CString &strNick, const CString &strMsg)
{
	Channel		*pCurChannel;
	CString		strMessage(strMsg), strLine;

	strMessage.Trim();
	for (POSITION pos = m_pParent->channelPtrList.GetHeadPosition(); pos != NULL;)
	{
		pCurChannel = (Channel*)m_pParent->channelPtrList.GetNext(pos);
		if (RemoveNick(pCurChannel->name, strNick))
		{
			if (!g_App.m_pPrefs->GetIrcIgnoreInfoMessage())
			{
				strLine.Format(GetResString(IDS_IRC_HASQUIT), strNick, strMessage);
				if (strMessage.IsEmpty())
					strLine.Truncate(strLine.GetLength() - 3);
				else
					strLine.Insert(strLine.GetLength() - 1, _T('\x0F'));
				m_pParent->AddMessage(pCurChannel->name, _T(""), RGB(51, 102, 255), strLine);
			}
		}
	}
}

bool CIrcNickListCtrl::ChangeNick(CString channel, const CString &strOldNick, const CString &strNewNick)
{
	Channel	*pUpdate = m_pParent->FindChannelByName(channel);

	if (pUpdate == NULL)
		return false;

	Nick		*pCurNick;
	LVFINDINFO	lvfind;
	sint32		iResult;

	for (POSITION pos = pUpdate->nicks.GetHeadPosition(); pos != NULL;)
	{
		pCurNick = (Nick*)pUpdate->nicks.GetNext(pos);
		if (pCurNick->nick == strOldNick)
		{
			if (pUpdate == m_pParent->GetCurrentChannel())
			{
				lvfind.flags = LVFI_PARAM;
				lvfind.lParam = (LPARAM)pCurNick;
				iResult = FindItem(&lvfind);
				if (iResult != -1)
					SetItemText(iResult, IRC1COL_NICK, strNewNick);
			}
			pCurNick->nick = strNewNick;
			return true;
		}
	}
	return false;
}

bool CIrcNickListCtrl::ChangeMode(CString channel, const CString &strNick, const CString &strMode)
{
	Channel		*pUpdate = m_pParent->FindChannelByName(channel);

	if (pUpdate == NULL)
		return false;

	Nick			*pCurNick;
	LVFINDINFO		lvfind;
	sint32			iResult;
	CString			strTmp;

	for (POSITION pos = pUpdate->nicks.GetHeadPosition(); pos != NULL;)
	{
		pCurNick = (Nick*)pUpdate->nicks.GetNext(pos);
		if (pCurNick->nick == strNick)
		{
			if (strMode == _T("+a"))
				pCurNick->protect = _T("*");
			else if (strMode == _T("-a"))
				pCurNick->protect.Empty();
			else if (strMode == _T("+h"))
				pCurNick->hop = _T("%");
			else if (strMode == _T("-h"))
				pCurNick->hop.Empty();
			else if (strMode == _T("+o"))
				pCurNick->op = _T("@");
			else if (strMode == _T("-o"))
				pCurNick->op.Empty();
			else if (strMode == _T("+q"))
				pCurNick->owner = _T("!");
			else if (strMode == _T("-q"))
				pCurNick->owner.Empty();
			else if (strMode == _T("+u"))
				pCurNick->uop = _T("-");
			else if (strMode == _T("-u"))
				pCurNick->uop.Empty();
			else if (strMode == _T("+v"))
				pCurNick->voice = _T("+");
			else if (strMode == _T("-v"))
				pCurNick->voice.Empty();
			if (pUpdate == m_pParent->GetCurrentChannel())
			{
				lvfind.flags = LVFI_PARAM;
				lvfind.lParam = (LPARAM)pCurNick;
				iResult = FindItem(&lvfind);
				if (iResult != -1)
				{
					strTmp.Format(_T("%s%s%s%s%s%s"), pCurNick->owner, pCurNick->protect, pCurNick->op, pCurNick->hop, pCurNick->voice, pCurNick->uop);
					SetItemText(iResult, IRC1COL_STATUS, strTmp);
				}
			}
		}
	}
	return true;
}

void CIrcNickListCtrl::ParseChangeMode(CString channel, const CString &strChanger, const CString &strCommands, const CString &strNames)
{
	try
	{
		if (strCommands.GetLength() == 2)
		{
			if (ChangeMode(channel, strNames, strCommands))
				if (!g_App.m_pPrefs->GetIrcIgnoreInfoMessage())
					m_pParent->AddMessageF(channel, _T(""), RGB(0, 147, 0), GetResString(IDS_IRC_SETSMODE), strChanger, strCommands, strNames);
			return;
		}
		else
		{
			CString		strDir;

			strDir = strCommands[0];
			if (strDir == _T("+") || strDir == _T("-"))
			{
				int		iCurMode = 1;
				int		iCurName = 0;
				int		iCurNameBack = strNames.Find(_T(' '), iCurName);

				while (iCurMode < strCommands.GetLength())
				{
					CString		strTest;

					if(iCurNameBack > iCurName)
					{
						strTest = strNames.Mid(iCurName, iCurNameBack - iCurName);
						iCurName = iCurNameBack + 1;
					}
					if (ChangeMode(channel, strTest, strDir + strCommands[iCurMode]) && !g_App.m_pPrefs->GetIrcIgnoreInfoMessage())
						m_pParent->AddMessageF(channel, _T(""), RGB(0, 147, 0), GetResString(IDS_IRC_SETSMODE), strChanger, strDir + strCommands[iCurMode], strTest);
					iCurNameBack = strNames.Find(_T(' '), iCurName + 1);
					if (iCurNameBack == -1)
						iCurNameBack = strNames.GetLength();
					iCurMode++;
				}
			}
		}
	}
	catch (...)
	{
		m_pParent->AddMessage(channel, _T(""), RGB(252, 127, 0), GetResString(IDS_IRC_NOTSUPPORTED));
	}
}
