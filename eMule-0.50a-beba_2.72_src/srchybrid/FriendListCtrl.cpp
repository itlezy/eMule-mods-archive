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
#include "emule.h"
#include "FriendListCtrl.h"
#include "friend.h"
#include "ClientDetailDialog.h"
#include "Addfriend.h"
#include "FriendList.h"
#include "emuledlg.h"
#include "ClientList.h"
#include "OtherFunctions.h"
#include "UpDownClient.h"
#include "ListenSocket.h"
#include "MenuCmds.h"
#include "ChatWnd.h"
// Tux: Feature: List all requested files [start]
#include "UploadQueue.h"
#include "DownloadQueue.h"
#include "Clientlist.h"
#include "PartFile.h"
#include "opcodes.h"
// Tux: Feature: List all requested files [end]

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CFriendListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CFriendListCtrl, CMuleListCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNmDblClk)
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

CFriendListCtrl::CFriendListCtrl()
{
	SetGeneralPurposeFind(true);
	SetSkinKey(L"FriendsLv");
}

CFriendListCtrl::~CFriendListCtrl()
{
}

void CFriendListCtrl::Init()
{
	SetPrefsKey(_T("FriendListCtrl"));

	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	RECT rcWindow;
	GetWindowRect(&rcWindow);
	InsertColumn(0, GetResString(IDS_QL_USERNAME), LVCFMT_LEFT, rcWindow.right - rcWindow.left - 4);

	SetAllIcons();
	theApp.friendlist->SetWindow(this);
	LoadSettings();
	SetSortArrow();
}

void CFriendListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
}

void CFriendListCtrl::SetAllIcons()
{
	CImageList iml;
	iml.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);
	iml.Add(CTempIconLoader(_T("FriendNoClient")));
	iml.Add(CTempIconLoader(_T("FriendWithClient")));
	iml.Add(CTempIconLoader(_T("FriendConnected")));
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	HIMAGELIST himlOld = ApplyImageList(iml.Detach());
	if (himlOld)
		ImageList_Destroy(himlOld);
}

void CFriendListCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;

	strRes = GetResString(IDS_QL_USERNAME);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(0, &hdi);

	int iItems = GetItemCount();
	for (int i = 0; i < iItems; i++)
		UpdateFriend(i, (CFriend*)GetItemData(i));
}

void CFriendListCtrl::UpdateFriend(int iItem, const CFriend* pFriend)
{
	SetItemText(iItem, 0, pFriend->m_strName.IsEmpty() ? _T('(') + GetResString(IDS_UNKNOWN) + _T(')') : pFriend->m_strName);

	int iImage;
    if (!pFriend->GetLinkedClient())
		iImage = 0;
	else if (pFriend->GetLinkedClient()->socket && pFriend->GetLinkedClient()->socket->IsConnected())
		iImage = 2;
	else
		iImage = 1;
	SetItem(iItem, 0, LVIF_IMAGE, 0, iImage, 0, 0, 0, 0);
}

void CFriendListCtrl::AddFriend(const CFriend* pFriend)
{
	// Tux: Fix: Crash on shutdown fix [SiRoB] [start]
	if (!theApp.emuledlg->IsRunning())
		return;
	// Tux: Fix: Crash on shutdown fix [SiRoB] [end]

	int iItem = InsertItem(LVIF_TEXT | LVIF_PARAM, GetItemCount(), pFriend->m_strName, 0, 0, 0, (LPARAM)pFriend);
	if (iItem >= 0)
		UpdateFriend(iItem, pFriend);
	theApp.emuledlg->chatwnd->UpdateFriendlistCount(theApp.friendlist->GetCount());
}

void CFriendListCtrl::RemoveFriend(const CFriend* pFriend)
{
	// Tux: Fix: Crash on shutdown fix [SiRoB] [start]
	if (!theApp.emuledlg->IsRunning())
		return;
	// Tux: Fix: Crash on shutdown fix [SiRoB] [end]

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)pFriend;
	int iItem = FindItem(&find);
	if (iItem != -1)
		DeleteItem(iItem);
	theApp.emuledlg->chatwnd->UpdateFriendlistCount(theApp.friendlist->GetCount());
}

void CFriendListCtrl::RefreshFriend(const CFriend* pFriend)
{
	// Tux: Fix: Crash on shutdown fix [SiRoB] [start]
	if (!theApp.emuledlg->IsRunning())
		return;
	// Tux: Fix: Crash on shutdown fix [SiRoB] [end]

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)pFriend;
	int iItem = FindItem(&find);
	if (iItem != -1)
		UpdateFriend(iItem, pFriend);
}

void CFriendListCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CTitleMenu ClientMenu;
	ClientMenu.CreatePopupMenu();
	ClientMenu.AddMenuTitle(GetResString(IDS_FRIENDLIST), true);

	const CFriend* cur_friend = NULL;
	const CUpDownClient* client = NULL;	// Tux: Improvement: added LowID check
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1) {
		cur_friend = (CFriend*)GetItemData(iSel);
		ClientMenu.AppendMenu(MF_STRING,MP_DETAIL, GetResString(IDS_SHOWDETAILS), _T("CLIENTDETAILS"));
		ClientMenu.SetDefaultItem(MP_DETAIL);
		if (cur_friend)	client = cur_friend->GetLinkedClient(true);	// Tux: Improvement: added LowID check
	}
	const bool can_connect = (client && client->GetDownloadState()!=DS_LOWTOLOWIP);	// Tux: Improvement: added LowID check

	ClientMenu.AppendMenu(MF_STRING, MP_ADDFRIEND, GetResString(IDS_ADDAFRIEND), _T("ADDFRIEND"));
	ClientMenu.AppendMenu(MF_STRING | (cur_friend ? MF_ENABLED : MF_GRAYED), MP_REMOVEFRIEND, GetResString(IDS_REMOVEFRIEND), _T("DELETEFRIEND"));
	ClientMenu.AppendMenu(MF_STRING | (can_connect ? MF_ENABLED : MF_GRAYED), MP_MESSAGE, GetResString(IDS_SEND_MSG), _T("SENDMESSAGE"));	// Tux: Improvement: added LowID check
	ClientMenu.AppendMenu(MF_STRING | ((can_connect && client->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED), MP_SHOWLIST, GetResString(IDS_VIEWFILES) , _T("VIEWFILES"));	// Tux: Improvement: added LowID check
	ClientMenu.AppendMenu(MF_STRING | (cur_friend && cur_friend->GetFriendSlot() ? MF_GRAYED : MF_ENABLED), MP_FRIENDSLOT, GetResString(IDS_FRIENDSLOT), _T("FRIENDSLOT")); // Tux: Fix: crash fix - added cur_friend
	ClientMenu.AppendMenu(MF_STRING, MP_DROP_ALL_FRIENDSLOTS, GetResString(IDS_DROP_ALL_FRIENDSLOTS), _T("FRIENDSLOTREMOVE"));	// Tux: Feature: Remove all friend slots
	ClientMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED), MP_FIND, GetResString(IDS_FIND), _T("SEARCHMENU"));	// Tux: changed search icon

    ClientMenu.EnableMenuItem(MP_FRIENDSLOT, (cur_friend)?MF_ENABLED : MF_GRAYED);
	ClientMenu.CheckMenuItem(MP_FRIENDSLOT, (cur_friend && cur_friend->GetFriendSlot()) ? MF_CHECKED : MF_UNCHECKED);

	// Tux: Feature: List all requested files [start]
	ClientMenu.AppendMenu(MF_SEPARATOR);
	ClientMenu.AppendMenu(MF_STRING | (cur_friend ? MF_ENABLED : MF_GRAYED), MP_LIST_REQUESTED_FILES, GetResString(IDS_LIST_REQFILES), _T("FILEINFO"));
	// Tux: Feature: List all requested files [end]

	GetPopupMenuPos(*this, point);
	ClientMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
}

BOOL CFriendListCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	wParam = LOWORD(wParam);

	CFriend* cur_friend = NULL;
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1) 
		cur_friend = (CFriend*)GetItemData(iSel);
	
	switch (wParam)
	{
		case MP_MESSAGE:
			if (cur_friend)
			{
				theApp.emuledlg->chatwnd->StartSession(cur_friend->GetClientForChatSession());
			}
			break;
		case MP_REMOVEFRIEND:
			if (cur_friend)
			{
				theApp.friendlist->RemoveFriend(cur_friend);
				// auto select next item after deleted one.
				if (iSel < GetItemCount()) {
					SetSelectionMark(iSel);
					SetItemState(iSel, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				}
				theApp.emuledlg->chatwnd->UpdateSelectedFriendMsgDetails();
			}
			break;
		case MP_ADDFRIEND:{
			// Tux: Feature: Modeless dialogs [start]
			/*
			CAddFriend dialog2; 
			dialog2.DoModal();
			*/
			CAddFriend* dialog2 = new CAddFriend(); 
			dialog2->OpenDialog();
			// Tux: Feature: Modeless dialogs [end]
			break;
		}
		case MP_DETAIL:
		case MPG_ALTENTER:
		case IDA_ENTER:
			if (cur_friend)
				ShowFriendDetails(cur_friend);
			break;
		case MP_SHOWLIST:
			if (cur_friend)
			{
				if (cur_friend->GetLinkedClient(true))
					cur_friend->GetLinkedClient()->RequestSharedFileList();
				else
				{
					CUpDownClient* newclient = new CUpDownClient(0, cur_friend->m_nLastUsedPort, cur_friend->m_dwLastUsedIP, 0, 0, true);
					newclient->SetUserName(cur_friend->m_strName);
					theApp.clientlist->AddClient(newclient);
					newclient->RequestSharedFileList();
				}
			}
			break;
		case MP_FRIENDSLOT:
			if (cur_friend)
			{
				bool bIsAlready = cur_friend->GetFriendSlot();
				theApp.friendlist->RemoveAllFriendSlots();
				if (!bIsAlready)
                    cur_friend->SetFriendSlot(true);
			}
			break;
		// Tux: Feature: List all requested files [start]
		case MP_LIST_REQUESTED_FILES: {
			if(theApp.clientlist->IsValidClient(cur_friend->GetLinkedClient()))
			{
				CString fileList;
				fileList += GetResString(IDS_LORQ_DOWNLOADING);
				if (theApp.downloadqueue->IsPartFile(cur_friend->GetLinkedClient()->GetRequestFile()))
				{
					fileList += cur_friend->GetLinkedClient()->GetRequestFile()->GetFileName(); 
					fileList += "\n" ; 
					for(POSITION pos2, pos1 = cur_friend->GetLinkedClient()->m_OtherRequests_list.GetHeadPosition(); (pos2=pos1)!=NULL; )
					{
						if(theApp.downloadqueue->IsPartFile(cur_friend->GetLinkedClient()->m_OtherRequests_list.GetNext(pos1)))
						{
							fileList += cur_friend->GetLinkedClient()->m_OtherRequests_list.GetAt(pos2)->GetFileName(); 
							fileList += "\n" ;
						}
					}
				}
				else
					fileList += GetResString(IDS_LORQ_NODL);

				fileList += "\n\n\n";
				fileList += GetResString(IDS_LORQ_UPLOADING);
				if(cur_friend->GetLinkedClient()->R4AF_List.IsEmpty())
					fileList += GetResString(IDS_LORQ_NOUL);
				else
				{
					for(POSITION pos2, pos1 = cur_friend->GetLinkedClient()->R4AF_List.GetHeadPosition(); (pos2=pos1)!=NULL; )
					{
						if(cur_friend->GetLinkedClient()->R4AF_List.GetNext(pos1))
						{
							fileList += cur_friend->GetLinkedClient()->R4AF_List.GetAt(pos2)->GetFileName(); 
							fileList += "\n" ;
						}
						else
							cur_friend->GetLinkedClient()->R4AF_List.RemoveAt(pos2);
					}
				}
					
				AfxMessageBox(fileList,MB_OK);
			}
			break;
		}
		// Tux: Feature: List all requested files [end]
		// Tux: Feature: Remove all friend slots [start]
		case MP_DROP_ALL_FRIENDSLOTS:
		{
			theApp.friendlist->RemoveAllFriendSlots();
			break;
		}
		// Tux: Feature: Remove all friend slots [end]
		case MP_FIND:
			OnFindStart();
			break;
	}
	return true;
}

void CFriendListCtrl::OnNmDblClk(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1) 
		ShowFriendDetails((CFriend*)GetItemData(iSel));
	*pResult = 0;
}

void CFriendListCtrl::ShowFriendDetails(const CFriend* pFriend)
{
	if (pFriend)
	{
		// Tux: Feature: Modeless dialogs [start]
		// -> recoded :-)
		if (pFriend->GetLinkedClient(true))
			pFriend->GetLinkedClient()->GetDetailDialogInterface()->OpenDetailDialog();
		else
		{
			CAddFriend* dlg = new CAddFriend();
			dlg->m_pShowFriend = const_cast<CFriend*>(pFriend);
			dlg->OpenDialog();
		}
		// Tux: Feature: Modeless dialogs [end]
	}
}

BOOL CFriendListCtrl::PreTranslateMessage(MSG* pMsg) 
{
   	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_DELETE)
		PostMessage(WM_COMMAND, MP_REMOVEFRIEND, 0);
	else if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_INSERT)
		PostMessage(WM_COMMAND, MP_ADDFRIEND, 0);

	return CMuleListCtrl::PreTranslateMessage(pMsg);
}

void CFriendListCtrl::OnLvnColumnClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLISTVIEW *pNMListView = (NMLISTVIEW *)pNMHDR;

	// Determine ascending based on whether already sorted on this column
	int iSortItem = GetSortItem();
	bool bOldSortAscending = GetSortAscending();
	bool bSortAscending = (iSortItem != pNMListView->iSubItem) ? true : !bOldSortAscending;

	// Item is column clicked
	iSortItem = pNMListView->iSubItem;

	// Sort table
	SetSortArrow(iSortItem, bSortAscending);
	SortItems(SortProc, MAKELONG(iSortItem, (bSortAscending ? 0 : 0x0001)));

	*pResult = 0;
}

int CFriendListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	const CFriend *item1 = (CFriend *)lParam1;
	const CFriend *item2 = (CFriend *)lParam2; 
	if (item1 == NULL || item2 == NULL)
		return 0;

	int iResult;
	switch (LOWORD(lParamSort))
	{
		case 0:
			iResult = CompareLocaleStringNoCase(item1->m_strName, item2->m_strName);
			break;

		default:
			return 0;
	}
	if (HIWORD(lParamSort))
		iResult = -iResult;
	return iResult;
}

void CFriendListCtrl::UpdateList()
{
	theApp.emuledlg->chatwnd->UpdateFriendlistCount(theApp.friendlist->GetCount());
	SortItems(SortProc, MAKELONG(GetSortItem(), (GetSortAscending() ? 0 : 0x0001)));
}
