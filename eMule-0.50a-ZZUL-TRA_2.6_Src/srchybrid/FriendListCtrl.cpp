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
// MORPH START - Added by Commander, Friendlinks [emulEspaa] - added by zz_fly
#include "ED2KLink.h"
#include "Log.h"
// MORPH END - Added by Commander, Friendlinks [emulEspaa]

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
	//MORPH START - Added by SiRoB, Friend Addon
	iml.Add(CTempIconLoader(_T("FriendNoClientSlot")));
	iml.Add(CTempIconLoader(_T("FriendWithClientSlot")));
	iml.Add(CTempIconLoader(_T("FriendConnectedSlot")));
	//MORPH END   - Added by SiRoB, Friend Addon

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
    // Mighty Knife: log friend activities
	CString OldName = GetItemText (iItem,0);
	if ((OldName != pFriend->m_strName) && (thePrefs.GetLogFriendlistActivities())) {
 		
		Log(LOG_INFO, _T("Friend changed his name: '%s'->'%s', hash %s"),
				(LPCTSTR) OldName, (LPCTSTR) pFriend->m_strName, md4str(pFriend->m_abyUserhash));
	}
	// [end] Mighty Knife
	SetItemText(iItem, 0, pFriend->m_strName.IsEmpty() ? _T('(') + GetResString(IDS_UNKNOWN) + _T(')') : pFriend->m_strName);

	int iImage;
    if (!pFriend->GetLinkedClient())
		iImage = 0;
	else if (pFriend->GetLinkedClient()->socket && pFriend->GetLinkedClient()->socket->IsConnected())
		iImage = 2;
	else
		iImage = 1;
	//MORPH START - Added by SiRoB, Friend Addon
	if (pFriend->GetFriendSlot()) iImage += 3;
	//MORPH END   - Added by SiRoB, Friend Addon 

	SetItem(iItem, 0, LVIF_IMAGE, 0, iImage, 0, 0, 0, 0);
}

void CFriendListCtrl::AddFriend(const CFriend* pFriend)
{
	int iItem = InsertItem(LVIF_TEXT | LVIF_PARAM, GetItemCount(), pFriend->m_strName, 0, 0, 0, (LPARAM)pFriend);
	if (iItem >= 0)
		UpdateFriend(iItem, pFriend);
	theApp.emuledlg->chatwnd->UpdateFriendlistCount(theApp.friendlist->GetCount());
}

void CFriendListCtrl::RemoveFriend(const CFriend* pFriend)
{
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
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1) {
		cur_friend = (CFriend*)GetItemData(iSel);
		ClientMenu.AppendMenu(MF_STRING,MP_DETAIL, GetResString(IDS_SHOWDETAILS)/*, _T("CLIENTDETAILS")*/);
		ClientMenu.SetDefaultItem(MP_DETAIL);
	}

	ClientMenu.AppendMenu(MF_STRING, MP_ADDFRIEND, GetResString(IDS_ADDAFRIEND)/*, _T("ADDFRIEND")*/);
	ClientMenu.AppendMenu(MF_STRING | (cur_friend ? MF_ENABLED : MF_GRAYED), MP_REMOVEFRIEND, GetResString(IDS_REMOVEFRIEND)/*, _T("DELETEFRIEND")*/);
	ClientMenu.AppendMenu(MF_STRING | (cur_friend ? MF_ENABLED : MF_GRAYED), MP_MESSAGE, GetResString(IDS_SEND_MSG)/*, _T("SENDMESSAGE")*/);
	ClientMenu.AppendMenu(MF_STRING | ((cur_friend==NULL || (cur_friend && cur_friend->GetLinkedClient(true) && !cur_friend->GetLinkedClient(true)->GetViewSharedFilesSupport())) ? MF_GRAYED : MF_ENABLED), MP_SHOWLIST, GetResString(IDS_VIEWFILES)/* , _T("VIEWFILES")*/);
	ClientMenu.AppendMenu(MF_SEPARATOR);
	ClientMenu.AppendMenu(MF_STRING, MP_FRIENDSLOT, GetResString(IDS_FRIENDSLOT)/*, _T("FRIENDSLOT")*/);
   //friend log +
	ClientMenu.AppendMenu(MF_STRING, MP_LOGFRIENDS, _T("Log Friend activities")/*, _T("FILECOMMENTS")*/);
	if(thePrefs.GetLogFriendlistActivities())
		ClientMenu.CheckMenuItem(MP_LOGFRIENDS,MF_CHECKED);
	else
		ClientMenu.CheckMenuItem(MP_LOGFRIENDS,MF_UNCHECKED);
	//friend log -
   ClientMenu.AppendMenu(MF_SEPARATOR);
 	// MORPH START - Modified by Commander, Friendlinks [emulEspaa] - added by zz_fly
	ClientMenu.AppendMenu(MF_STRING | (cur_friend ? MF_ENABLED : MF_GRAYED), MP_GETFRIENDED2KLINK, _T("Copy link to clipboard"));
	ClientMenu.AppendMenu(MF_STRING, MP_GETMYFRIENDED2KLINK, _T("Copy my link to clipboard"));
    ClientMenu.AppendMenu(MF_STRING | (theApp.IsEd2kFriendLinkInClipboard() ? MF_ENABLED : MF_GRAYED), MP_PASTE, GetResString(IDS_PASTE));
	ClientMenu.AppendMenu(MF_SEPARATOR);
	// MORPH END - Modified by Commander, Friendlinks [emulEspaa]	
	ClientMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED), MP_FIND, GetResString(IDS_FIND)/*, _T("Search")*/);

    ClientMenu.EnableMenuItem(MP_FRIENDSLOT, (cur_friend)?MF_ENABLED : MF_GRAYED);
	ClientMenu.CheckMenuItem(MP_FRIENDSLOT, (cur_friend && cur_friend->GetFriendSlot()) ? MF_CHECKED : MF_UNCHECKED);

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
			CAddFriend dialog2; 
			dialog2.DoModal();
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
// MORPH START - Added by Commander, Friendlinks [emulEspaa] - added by zz_fly
		case MP_PASTE:
		{
				CString link = theApp.CopyTextFromClipboard();
				link.Trim();
				if ( link.IsEmpty() )
					break;
				try{
					CED2KLink* pLink = CED2KLink::CreateLinkFromUrl(link);
		
				if (pLink && pLink->GetKind() == CED2KLink::kFriend )
				{
							// Better with dynamic_cast, but no RTTI enabled in the project
							CED2KFriendLink* pFriendLink = static_cast<CED2KFriendLink*>(pLink);
							uchar userHash[16];
							pFriendLink->GetUserHash(userHash);

						if ( ! theApp.friendlist->IsAlreadyFriend(userHash) )
						theApp.friendlist->AddFriend(userHash, 0U, 0U, 0U, 0U, pFriendLink->GetUserName(), 1U);
					else
					{
						CString msg;
						msg.Format(GetResString(IDS_USER_ALREADY_FRIEND), pFriendLink->GetUserName());
						AddLogLine(true, msg);
						}
					}
				if(pLink) delete pLink; //zz_fly :: memleak :: thanks DolphinX
				}
				catch(CString strError){
					AfxMessageBox(strError);
				}
			}
			break;
        case MP_GETFRIENDED2KLINK:
		{
			CString sCompleteLink;
			if ( cur_friend && cur_friend->HasUserhash() )
			{
				CString sLink;
				CED2KFriendLink friendLink(cur_friend->m_strName, cur_friend->m_abyUserhash);
				friendLink.GetLink(sLink);
				if ( !sCompleteLink.IsEmpty() )
					sCompleteLink.Append(_T("\r\n"));
				sCompleteLink.Append(sLink);
			}

			if ( !sCompleteLink.IsEmpty() )
				theApp.CopyTextToClipboard(sCompleteLink);
		}
			break;
        case MP_GETMYFRIENDED2KLINK:
		{
			CString sLink;
			sLink.Format(_T("ed2k://|friend|%s|%s|/"),
				thePrefs.GetUserNick(),
				EncodeBase16(thePrefs.GetUserHash(),16));
			theApp.CopyTextToClipboard(sLink);
		}
			break;
	// MORPH END - Added by Commander, Friendlinks [emulEspaa]
		case MP_FRIENDSLOT:
			if (cur_friend)
			{
				bool bIsAlready = cur_friend->GetFriendSlot();
				theApp.friendlist->RemoveAllFriendSlots();
				if (!bIsAlready)
                    cur_friend->SetFriendSlot(true);
			}
           //MORPH START - Added by SiRoB, Friend Addon
					UpdateFriend(iSel,cur_friend);
		   //MORPH END   - Added by SiRoB, Friend Addon
			break;
		case MP_FIND:
			OnFindStart();
			break;
	}
//friend log +
		if(wParam == MP_LOGFRIENDS){
			if(thePrefs.GetLogFriendlistActivities())
				thePrefs.SetLogFriendlistActivities(false);
			else
				thePrefs.SetLogFriendlistActivities(true);
		}
		//friend log -
			
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
		if (pFriend->GetLinkedClient(true))
		{
			CClientDetailDialog dialog(pFriend->GetLinkedClient());
			dialog.DoModal();
		}
		else
		{
			CAddFriend dlg;
			dlg.m_pShowFriend = const_cast<CFriend*>(pFriend);
			dlg.DoModal();
		}
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

// MORPH START - Added by Commander, Friendlinks [emulEspaa] - added by zz_fly
bool CFriendListCtrl::AddEmfriendsMetToList(const CString& strFile)
{
	ShowWindow(SW_HIDE);
	bool ret = theApp.friendlist->AddEmfriendsMetToList(strFile);
	theApp.friendlist->ShowFriends();
	UpdateList();
	ShowWindow(SW_SHOW);
	return ret;
}
// MORPH END - Added by Commander, Friendlinks [emulEspaa]
