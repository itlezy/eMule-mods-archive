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
#include "Friend.h"
#include "FriendListCtrl.h"
#include "TitleMenu.h"
#include "Details\clientdetails.h"
#include "AddFriend.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CFriendListCtrl, CMuleListCtrl)

CFriendListCtrl::CFriendListCtrl()
{
	memset(&m_bSortAscending, true, sizeof(m_bSortAscending));
	SetGeneralPurposeFind(true);
}

CFriendListCtrl::~CFriendListCtrl()
{
}

BEGIN_MESSAGE_MAP(CFriendListCtrl, CMuleListCtrl)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDoubleclick)
	ON_WM_KEYDOWN()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnclick)
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFriendListCtrl::Init()
{
	static const uint16 s_auIconResID[] =
	{
		IDI_FRIENDS1,
		IDI_FRIENDS2,
		IDI_USERS
	};
	static const uint16 s_auColHdr[][2] =
	{
		{ LVCFMT_LEFT,  140 },	// FRIENDCOL_USERNAME
		{ LVCFMT_LEFT,  110 }	// FRIENDCOL_LASTSEEN
	};

	SetExtendedStyle(LVS_EX_FULLROWSELECT);

	m_imageList.Create(16, 16, g_App.m_iDfltImageListColorFlags | ILC_MASK, ARRSIZE(s_auIconResID), 0);
	m_imageList.SetBkColor(RGB(255, 255, 255));
	FillImgLstWith16x16Icons(&m_imageList, s_auIconResID, ARRSIZE(s_auIconResID));
	SetImageList(&m_imageList, LVSIL_SMALL);

	for (unsigned ui = 0; ui < ARRSIZE(s_auColHdr); ui++)
		InsertColumn(ui, _T(""), static_cast<int>(s_auColHdr[ui][0]), static_cast<int>(s_auColHdr[ui][1]));

	LoadSettings(CPreferences::TABLE_FRIENDLIST);

	g_App.m_pFriendList->SetWindow(this);
	g_App.m_pFriendList->ShowFriends();

	int		iSortCode = g_App.m_pPrefs->GetColumnSortItem(CPreferences::TABLE_FRIENDLIST);

	iSortCode |= g_App.m_pPrefs->GetColumnSortAscending(CPreferences::TABLE_FRIENDLIST) ? MLC_SORTASC : MLC_SORTDESC;
	SortInit(iSortCode);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFriendListCtrl::SortInit(int iSortCode)
{
	int		iSortColumn = (iSortCode & MLC_COLUMNMASK);			//the sort column
	bool	bSortAscending = (iSortCode & MLC_SORTDESC) == 0;	//the sort order

	m_bSortAscending[iSortColumn] = bSortAscending;
	SetSortArrow(iSortColumn, bSortAscending);
	SortItems(SortProc, iSortCode);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFriendListCtrl::Localize()
{
	static const uint16 s_auResTbl[] =
	{
		IDS_QL_USERNAME,	// FRIENDCOL_USERNAME
		IDS_LASTSEEN		// FRIENDCOL_LASTSEEN
	};

	if (GetSafeHwnd() != NULL)
	{
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
	}
	ShowFriendCount();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFriendListCtrl::AddFriend(CFriend* toadd)
{
	uint32 itemnr = GetItemCount();
	itemnr = InsertItem(LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE, itemnr, toadd->m_strName.GetBuffer(), 0, 0, 1, (LPARAM)toadd);
	RefreshFriend(toadd);
	ShowFriendCount();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFriendListCtrl::RemoveFriend(CFriend* toremove)
{
	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)toremove;
	sint32 result = FindItem(&find);
	if (result != ( -1))
		DeleteItem(result);
	ShowFriendCount();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFriendListCtrl::RefreshFriend(CFriend *pFriend)
{
	if (!::IsWindow(GetSafeHwnd()))
		return;

	LVFINDINFO		find;

	find.flags = LVFI_PARAM;
	find.lParam = reinterpret_cast<LPARAM>(pFriend);

	sint32		iIndex = FindItem(&find);

	if (iIndex < 0)
		return;

	int		iImageIndex;
	CString	strTemp;

	if (pFriend->GetLinkedClient() == NULL)
		iImageIndex = 0;
#ifdef OLD_SOCKETS_ENABLED
	else if (pFriend->GetLinkedClient()->m_pRequestSocket && pFriend->GetLinkedClient()->m_pRequestSocket->IsConnected())
		iImageIndex = 2;
#endif //OLD_SOCKETS_ENABLED
	else
		iImageIndex = 1;
	SetItem(iIndex, FRIENDCOL_USERNAME, LVIF_TEXT | LVIF_IMAGE, pFriend->m_strName, iImageIndex, 0, 0, 0, 0);

	if (pFriend->GetLastSeen() == 0)
		GetResString(&strTemp, IDS_NEVER);
	else
	{
		SYSTEMTIME		st;

		CTime(pFriend->GetLastSeen()).GetAsSystemTime(st);
		strTemp = COleDateTime(st).Format(_T("%c"));
	}
	SetItemText(iIndex, FRIENDCOL_LASTSEEN, strTemp);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFriendListCtrl::OnDoubleclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	NOPRM(pNMHDR);
	EMULE_TRY

	int		iSelectionMark = GetSelectionMark();

	if ((iSelectionMark != -1) && (GetSelectedCount() > 0))
	{
		CFriend *cur_friend = reinterpret_cast<CFriend*>(GetItemData(iSelectionMark));
		if (g_App.m_pPrefs->GetDetailsOnClick())
		{
			if (cur_friend->GetLinkedClient() != NULL)
			{
				CClientDetails dialog(IDS_CD_TITLE, cur_friend->GetLinkedClient(), this, 0);
				dialog.DoModal();
			}
		}
		else
			PostMessage(WM_COMMAND, MP_MESSAGE);
	}

	EMULE_CATCH
	*pResult = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFriendListCtrl::OnContextMenu(CWnd *pWnd, CPoint point)
{
//	On right click, we also want to change the current selection like the left click does
	CPoint	p = point;
	int		iIdx;
	NOPRM(pWnd);

	ScreenToClient(&p);
	if ((iIdx = HitTest(p)) >= 0)
		SetSelectionMark(iIdx);

	CTitleMenu	menuClient;

	menuClient.CreatePopupMenu();
	menuClient.AddMenuTitle(GetResString(IDS_FRIENDS));

	int			iSelectionMark = GetSelectionMark();
	bool		bSelected = ((iSelectionMark != -1) && (GetSelectedCount() > 0));
	CFriend		*pFriend = NULL;

	if (bSelected)
	{
		pFriend = reinterpret_cast<CFriend*>(GetItemData(iSelectionMark));
		menuClient.AppendMenu( MF_STRING | ((pFriend->GetLinkedClient() != NULL) ? MF_ENABLED : MF_GRAYED),
			MP_DETAIL, GetStringFromShortcutCode(IDS_SHOWDETAILS, SCUT_SRC_DETAILS, SSP_TAB_PREFIX) );
	}

	menuClient.AppendMenu(MF_STRING, MP_ADDFRIEND, GetResString(IDS_ADDAFRIEND));

	if (bSelected)
	{
		menuClient.AppendMenu(MF_STRING, MP_REMOVEFRIEND, GetResString(IDS_REMOVEFRIEND));
		menuClient.AppendMenu(MF_STRING, MP_MESSAGE, GetStringFromShortcutCode(IDS_SEND_MSG, SCUT_SRC_MSG, SSP_TAB_PREFIX));
		menuClient.AppendMenu( MF_STRING | ((pFriend && ((pFriend->GetLinkedClient() == NULL) || pFriend->GetLinkedClient()->GetViewSharedFilesSupport())) ? MF_ENABLED : MF_GRAYED),
			MP_SHOWLIST, GetStringFromShortcutCode(IDS_VIEWFILES, SCUT_SRC_SHAREDFILES, SSP_TAB_PREFIX) );
	}
	menuClient.SetDefaultItem((g_App.m_pPrefs->GetDetailsOnClick()) ? MP_DETAIL : MP_MESSAGE);
	menuClient.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this, NULL);
//	Menu objects are destroyed in their destructor
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CFriendListCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int		iSelectionMark = GetSelectionMark();
	CFriend	*pFriend = NULL;
	NOPRM(lParam);

	if (iSelectionMark != -1)
		pFriend = reinterpret_cast<CFriend*>(GetItemData(iSelectionMark));
	switch (wParam)
	{
		case MP_MESSAGE:
			if (pFriend != NULL)
			{
				if (pFriend->GetLinkedClient() != NULL)
					g_App.m_pMDlg->m_wndChat.StartSession(pFriend->GetLinkedClient());
				else
				{
					CUpDownClient* pChatClient = new CUpDownClient(static_cast<uint16>(pFriend->m_nLastUsedPort), pFriend->m_dwLastUsedIP, 0, 0, NULL, UID_ED2K);

					pChatClient->SetUserName(pFriend->m_strName);
					if (g_App.m_pClientList->AddClient(pChatClient))
						g_App.m_pMDlg->m_wndChat.StartSession(pChatClient);
					else
						safe_delete(pChatClient);
				}
			}
			break;

		case MP_ADDFRIEND:
		{
			CAddFriend dialog2;
			dialog2.DoModal();
			break;
		}
		case MP_DETAIL:
			if ((pFriend != NULL) && (pFriend->GetLinkedClient() != NULL))
			{
				CClientDetails dialog(IDS_CD_TITLE, pFriend->GetLinkedClient(), this, 0);
				dialog.DoModal();
			}
			break;

		case MP_SHOWLIST:
			if (pFriend != NULL)
			{
				if (pFriend->GetLinkedClient() != NULL)
					pFriend->GetLinkedClient()->RequestSharedFileList();
				else
				{
					CUpDownClient* pNewClient = new CUpDownClient(static_cast<uint16>(pFriend->m_nLastUsedPort), pFriend->m_dwLastUsedIP, 0, 0, NULL, UID_ED2K);

					pNewClient->SetUserName(pFriend->m_strName);
					if (g_App.m_pClientList->AddClient(pNewClient))
						pNewClient->RequestSharedFileList();
					else
						safe_delete(pNewClient);
				}
			}
			break;

		case MP_CANCEL:	// delete through pressing <DEL> key
			if( (pFriend == NULL) || ( g_App.m_pPrefs->IsConfirmFriendDelEnabled() &&
				( MessageBox(GetResString(IDS_BACKUP_SURE), GetResString(IDS_REMOVEFRIEND),
				MB_ICONQUESTION | MB_YESNO) != IDYES ) ) )
			{
				break;
			}
		case MP_REMOVEFRIEND:	// delete through context menu
			if (pFriend != NULL)
			{
				g_App.m_pFriendList->RemoveFriend(pFriend);
			//	Auto-select next item after deleted one or the previous if it was the last
				if ( (iSelectionMark < GetItemCount()) ||
					((--iSelectionMark >= 0) && (iSelectionMark < GetItemCount())) )
				{
					SetSelectionMark(iSelectionMark);
					SetItemState(iSelectionMark, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				}
			}
			break;
	}
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFriendListCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	EMULE_TRY
	
	bool bHandled = false;

	if (nChar == VK_INSERT)
	{
		CAddFriend dialog2;
		dialog2.DoModal();

		bHandled = true;
	}

	if (!bHandled)
		CMuleListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFriendListCtrl::OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_LISTVIEW	*pNMListView = reinterpret_cast<NM_LISTVIEW*>(pNMHDR);
	int			iSubItem = pNMListView->iSubItem;
	bool		bSortOrder = m_bSortAscending[iSubItem];

// Reverse sorting direction for the same column and keep the same if column was changed
	if (static_cast<int>(m_dwParamSort & MLC_COLUMNMASK) == iSubItem)
		m_bSortAscending[iSubItem] = bSortOrder = !bSortOrder;

	SetSortArrow(iSubItem, bSortOrder);
	SortItems(SortProc, iSubItem | ((bSortOrder) ? MLC_SORTASC : MLC_SORTDESC));

	g_App.m_pPrefs->SetColumnSortItem(CPreferences::TABLE_FRIENDLIST, iSubItem);
	g_App.m_pPrefs->SetColumnSortAscending(CPreferences::TABLE_FRIENDLIST, bSortOrder);

	*pResult = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CFriendListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CFriend	*pFriend1 = reinterpret_cast<CFriend*>(lParam1);
	CFriend	*pFriend2 = reinterpret_cast<CFriend*>(lParam2);

	if ((pFriend1 == NULL) || (pFriend2 == NULL))
		return 0;

	int		iCompare = 0;
	int		iSortMod = ((lParamSort & MLC_SORTDESC) == 0) ? 1 : -1;
	int		iSortCol = lParamSort & MLC_COLUMNMASK;

	switch (iSortCol)
	{
		case FRIENDCOL_USERNAME:
			iCompare = _tcsicmp(pFriend1->m_strName, pFriend2->m_strName);
			break;

		case FRIENDCOL_LASTSEEN:
			iCompare = CompareUnsigned(pFriend1->GetLastSeen(), pFriend2->GetLastSeen());
			break;
	}
	return iCompare * iSortMod;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFriendListCtrl::ShowFriendCount()
{
	CString		strTitle;

	strTitle.Format(_T("%s (%u)"), GetResString(IDS_FRIENDS), GetItemCount());
	g_App.m_pMDlg->m_wndChat.SetDlgItemText(IDC_FRIENDS_LBL, strTitle);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CFriendListCtrl::PreTranslateMessage(MSG *pMsg)
{
	if ((pMsg->message == WM_KEYDOWN) || (pMsg->message == WM_SYSKEYDOWN))
	{
		int		 iMessage	  = 0;
		POSITION posSelClient = GetFirstSelectedItemPosition();

		if (posSelClient != NULL)
		{
			short	nCode = GetCodeFromPressedKeys(pMsg);
			CFriend	*pFriend = reinterpret_cast<CFriend*>(GetItemData(GetNextSelectedItem(posSelClient)));

			iMessage = GetClientListActionFromShortcutCode(nCode, (pFriend != NULL) ? pFriend->GetLinkedClient() : NULL);

			if (iMessage > 0)
			{
				PostMessage(WM_COMMAND, static_cast<WPARAM>(iMessage));
				return TRUE;
			}
		}
	}

	return CMuleListCtrl::PreTranslateMessage(pMsg);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
