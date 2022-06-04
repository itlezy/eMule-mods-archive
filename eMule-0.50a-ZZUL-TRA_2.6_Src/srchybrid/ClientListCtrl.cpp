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
#include "ClientListCtrl.h"
#include "otherfunctions.h"
#include "MenuCmds.h"
#include "ClientDetailDialog.h"
#include "KademliaWnd.h"
#include "ClientList.h"
#include "emuledlg.h"
#include "FriendList.h"
#include "TransferDlg.h"
#include "MemDC.h"
#include "UpDownClient.h"
#include "ClientCredits.h"
#include "ListenSocket.h"
#include "ChatWnd.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "Kademlia/net/KademliaUDPListener.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


static const UINT colStrID[]={
	 IDS_QL_USERNAME
	,IDS_CL_UPLOADSTATUS
	,IDS_CL_TRANSFUP
	,IDS_CL_DOWNLSTATUS
	,IDS_CL_TRANSFDOWN
	,IDS_CD_CSOFT
	,IDS_CONNECTED
	,IDS_CD_UHASH
    ,IDS_IPNUMBER // <CB Mod : Client Extended Info : Date 11.07.07>
    ,IDS_COUNTRY // ZZUL-TRA :: IP2Country
};

IMPLEMENT_DYNAMIC(CClientListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CClientListCtrl, CMuleListCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnClick)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnLvnGetDispInfo)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNmDblClk)
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

CClientListCtrl::CClientListCtrl()
	: CListCtrlItemWalk(this)
{
	SetGeneralPurposeFind(true);
	SetSkinKey(L"ClientsLv");
}

void CClientListCtrl::Init()
{
	SetPrefsKey(_T("ClientListCtrl"));
	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

    static const int colWidth[]={DFLT_CLIENTNAME_COL_WIDTH,
								100,
								DFLT_SIZE_COL_WIDTH,
								100,
								DFLT_SIZE_COL_WIDTH,
								DFLT_CLIENTSOFT_COL_WIDTH,
								50,
								DFLT_HASH_COL_WIDTH,
								100, // <CB Mod : Client Extended Info : Date 11.07.07>
								100}; // ZZUL-TRA :: IP2Country

	CString strRes;
	for(int icol = 0; icol < _countof(colStrID); ++icol){
		strRes=GetResString(colStrID[icol]);
		if(icol == 7)
			strRes.Remove(_T(':'));
		InsertColumn(icol,strRes,LVCFMT_LEFT,colWidth[icol]);
	}

	SetAllIcons();
	Localize();
	LoadSettings();
	SetSortArrow();
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0 : 100));
}

void CClientListCtrl::Localize()
{
	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;

    CString strRes;
	for (int icol=0;icol<pHeaderCtrl->GetItemCount();++icol) {
		strRes=GetResString(colStrID[icol]);
		if(icol == 7)
			strRes.Remove(_T(':'));
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(icol, &hdi);
	}
}

void CClientListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
}

void CClientListCtrl::SetAllIcons()
{
	ApplyImageList(NULL);
	m_ImageList.DeleteImageList();
	m_ImageList.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkey")));
	m_ImageList.Add(CTempIconLoader(_T("ClientCompatible")));
	m_ImageList.Add(CTempIconLoader(_T("Friend")));
	m_ImageList.Add(CTempIconLoader(_T("ZZULTRA")));// ZZUL-TRA :: SameModVersionDetection 
#ifdef CLIENTANALYZER
	m_ImageList.Add(CTempIconLoader(_T("BADGUY")));
#endif
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("ClientSecureOvl"))), 1);
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("OverlayObfu"))), 2);
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("OverlaySecureObfu"))), 3);
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("SHAREVISIBLE"))), 4); //morph4u share visible
	// Apply the image list also to the listview control, even if we use our own 'DrawItem'.
	// This is needed to give the listview control a chance to initialize the row height.
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) != 0 );
	VERIFY( ApplyImageList(m_ImageList) == NULL );
}

void CClientListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	if (!lpDrawItemStruct->itemData)
		return;
	
	const CUpDownClient *client = (CUpDownClient *)lpDrawItemStruct->itemData;
#ifdef EVENLINE	
        CMemoryDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
    InitItemMemDC(dc, lpDrawItemStruct->rcItem, (client->IsFriend())?((client->GetFriendSlot())?RGB(210,240,255):RGB(200,255,200)):((lpDrawItemStruct->itemID % 2)?m_crEvenLine:m_crWindow), lpDrawItemStruct->itemState);
#else
	CMemoryDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	BOOL bCtrlFocused;
	InitItemMemDC(dc, lpDrawItemStruct, bCtrlFocused);
#endif
	CRect cur_rec(lpDrawItemStruct->rcItem);
	CRect rcClient;
	GetClientRect(&rcClient);

	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	cur_rec.right = cur_rec.left - sm_iLabelOffset;
	cur_rec.left += sm_iIconOffset;
	for (int iCurrent = 0; iCurrent < iCount; iCurrent++)
	{
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
		if (!IsColumnHidden(iColumn))
		{
			UINT uDrawTextAlignment;
			int iColumnWidth = GetColumnWidth(iColumn, uDrawTextAlignment);
			cur_rec.right += iColumnWidth;
			if (cur_rec.left < cur_rec.right && HaveIntersection(rcClient, cur_rec))
			{
				TCHAR szItem[1024];
				GetItemDisplayText(client, iColumn, szItem, _countof(szItem));
				switch (iColumn)
				{
					case 0:{
						int iImage;
// ZZUL-TRA :: SameModVersionDetection :: Start
						if(client->IsZZULTRA())
					         iImage = 3;
                        else
// ZZUL-TRA :: SameModVersionDetection :: End
#ifdef CLIENTANALYZER
					    if (client->IsBadGuy())
					         iImage = 4;
                        else
#endif	
                        if (client->IsFriend())
							iImage = 2;
						else if (client->GetClientSoft() == SO_EMULE)
							iImage = 0;
                        else
							iImage = 1;

						UINT nOverlayImage = 0;
						if ((client->Credits() && client->Credits()->GetCurrentIdentState(client->GetIP()) == IS_IDENTIFIED))
							nOverlayImage |= 1;
						if (client->IsObfuscatedConnectionEstablished())
							nOverlayImage |= 2;
						//share visible +
						if (client->GetUserName() && client->GetViewSharedFilesSupport())
							nOverlayImage |= 4;
						//share visible -
						int iIconPosY = (cur_rec.Height() > 16) ? ((cur_rec.Height() - 16) / 2) : 1;
						POINT point = { cur_rec.left, cur_rec.top + iIconPosY };
						m_ImageList.Draw(dc, iImage, point, ILD_NORMAL | INDEXTOOVERLAYMASK(nOverlayImage));

						// ZZUL-TRA :: IP2Country :: Start
						if(theApp.ip2country->ShowCountryFlag()){
                            cur_rec.left+=18;
							POINT point2 = {cur_rec.left, cur_rec.top + 1};
							int index = client->GetCountryFlagIndex();
							theApp.ip2country->GetFlagImageList()->DrawIndirect(dc, index , point2, CSize(18,16), CPoint(0,0), ILD_NORMAL);
							cur_rec.left += sm_iLabelOffset;				
	                        }
						// ZZUL-TRA :: IP2Country :: End

						cur_rec.left += 16 + sm_iLabelOffset;
#ifndef EVENLINE	
                     // ZZUL-TRA :: Colors :: Start
                        if (client->GetUserName()){
						COLORREF crOldBackColor = dc.GetBkColor();
                        if (client->IsFriend() && client->GetFriendSlot())
			             	dc.SetBkColor(RGB(185,220,255)); //friendslot blue
						else if(client->IsFriend())
							dc.SetBkColor(RGB(200,250,200)); //friend green
						_tcsncpy(szItem, client->GetUserName(), _countof(szItem) - 1);
						szItem[_countof(szItem) - 1] = _T('\0');
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT);
						dc.SetBkColor(crOldBackColor);
					    }else
					// ZZUL-TRA :: Colors :: End
#endif
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
						cur_rec.left -= 16;
						cur_rec.right -= sm_iSubItemInset;

                        // ZZUL-TRA :: IP2Country :: Start
						if(theApp.ip2country->ShowCountryFlag())
                           cur_rec.left -=18;
						// ZZUL-TRA :: IP2Country :: End
						break;
					}

					default:
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
						break;
				}
			}
			cur_rec.left += iColumnWidth;
		}
	}
#ifndef EVENLINE	
	DrawFocusRect(dc, lpDrawItemStruct->rcItem, lpDrawItemStruct->itemState & ODS_FOCUS, bCtrlFocused, lpDrawItemStruct->itemState & ODS_SELECTED);
#endif
}

void CClientListCtrl::GetItemDisplayText(const CUpDownClient *client, int iSubItem, LPTSTR pszText, int cchTextMax)
{
	if (pszText == NULL || cchTextMax <= 0) {
		ASSERT(0);
		return;
	}
	pszText[0] = _T('\0');
	switch (iSubItem)
	{
		case 0:
			if (client->GetUserName() == NULL)
				_sntprintf(pszText, cchTextMax, _T("(%s)"), GetResString(IDS_UNKNOWN));
			else
				_tcsncpy(pszText, client->GetUserName(), cchTextMax);
			break;

		case 1:
			_tcsncpy(pszText, client->GetUploadStateDisplayString(), cchTextMax);
			break;

		case 2:
			_tcsncpy(pszText, client->credits != NULL ? CastItoXBytes(client->credits->GetUploadedTotal(), false, false) : _T(""), cchTextMax);
			break;

		case 3:
			_tcsncpy(pszText, client->GetDownloadStateDisplayString(), cchTextMax);
			break;

		case 4:
			_tcsncpy(pszText, client->credits != NULL ? CastItoXBytes(client->credits->GetDownloadedTotal(), false, false) : _T(""), cchTextMax);
			break;

//>>> WiZaRd::Show ModVer
		case 5:
		{
                        CString sBuffer = client->DbgGetFullClientSoftVer(); 
			if (sBuffer.IsEmpty())
				sBuffer = GetResString(IDS_UNKNOWN);
			_tcsncpy(pszText, sBuffer, cchTextMax);
			break;
		}
//>>> WiZaRd::Show ModVer

		case 6:
			_tcsncpy(pszText, GetResString((client->socket && client->socket->IsConnected()) ? IDS_YES : IDS_NO), cchTextMax);
			break;

		case 7:
			_tcsncpy(pszText, md4str(client->GetUserHash()), cchTextMax);
			break;
	// <CB Mod : Client Extended Info : Date 11.07.07>
					case 8:
						_tcsncpy(pszText, ipstr (client->GetIP()), cchTextMax);
						break;
						// </CB Mod : Client Extended Info>
// ZZUL-TRA :: IP2Country :: Start
	    case 9:
			_tcsncpy(pszText, client->GetCountryName(), cchTextMax);
			break;
		// ZZUL-TRA :: IP2Country :: End
}
	pszText[cchTextMax - 1] = _T('\0');
}

void CClientListCtrl::OnLvnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (theApp.emuledlg->IsRunning()) {
		// Although we have an owner drawn listview control we store the text for the primary item in the listview, to be
		// capable of quick searching those items via the keyboard. Because our listview items may change their contents,
		// we do this via a text callback function. The listview control will send us the LVN_DISPINFO notification if
		// it needs to know the contents of the primary item.
		//
		// But, the listview control sends this notification all the time, even if we do not search for an item. At least
		// this notification is only sent for the visible items and not for all items in the list. Though, because this
		// function is invoked *very* often, do *NOT* put any time consuming code in here.
		//
		// Vista: That callback is used to get the strings for the label tips for the sub(!) items.
		//
		NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
		if (pDispInfo->item.mask & LVIF_TEXT) {
			const CUpDownClient* pClient = reinterpret_cast<CUpDownClient*>(pDispInfo->item.lParam);
			if (pClient != NULL)
				GetItemDisplayText(pClient, pDispInfo->item.iSubItem, pDispInfo->item.pszText, pDispInfo->item.cchTextMax);
		}
	}
	*pResult = 0;
}

void CClientListCtrl::OnLvnColumnClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLISTVIEW *pNMListView = (NMLISTVIEW *)pNMHDR;
	bool sortAscending;
	if (GetSortItem() != pNMListView->iSubItem)
	{
		switch (pNMListView->iSubItem)
		{
			case 1: // Upload State
			case 2: // Uploaded Total
			case 4: // Downloaded Total
			case 5: // Client Software
			case 6: // Connected
				sortAscending = false;
				break;
			default:
				sortAscending = true;
				break;
		}
	}
	else
		sortAscending = !GetSortAscending();

	// Sort table
	UpdateSortHistory(pNMListView->iSubItem + (sortAscending ? 0 : 100));
	SetSortArrow(pNMListView->iSubItem, sortAscending);
	SortItems(SortProc, pNMListView->iSubItem + (sortAscending ? 0 : 100));

	*pResult = 0;
}

int CClientListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	const CUpDownClient *item1 = (CUpDownClient *)lParam1;
	const CUpDownClient *item2 = (CUpDownClient *)lParam2;
	int iColumn = (lParamSort >= 100) ? lParamSort - 100 : lParamSort;
	int iResult = 0;
	switch (iColumn)
	{
		case 0:
			if (item1->GetUserName() && item2->GetUserName())
				iResult = CompareLocaleStringNoCase(item1->GetUserName(), item2->GetUserName());
			else if (item1->GetUserName() == NULL)
				iResult = 1; // place clients with no usernames at bottom
			else if (item2->GetUserName() == NULL)
				iResult = -1; // place clients with no usernames at bottom
			break;

		case 1:
		    iResult = item1->GetUploadState() - item2->GetUploadState();
			break;

		case 2:
			if (item1->credits && item2->credits)
				iResult = CompareUnsigned64(item1->credits->GetUploadedTotal(), item2->credits->GetUploadedTotal());
			else if (item1->credits)
			    iResult = 1;
			else
				iResult = -1;
			break;

		case 3:
		    if (item1->GetDownloadState() == item2->GetDownloadState())
			{
			    if (item1->IsRemoteQueueFull() && item2->IsRemoteQueueFull())
				    iResult = 0;
			    else if (item1->IsRemoteQueueFull())
				    iResult = 1;
			    else if (item2->IsRemoteQueueFull())
				    iResult = -1;
		    }
			else
				iResult = item1->GetDownloadState() - item2->GetDownloadState();
			break;

		case 4:
			if (item1->credits && item2->credits)
				iResult = CompareUnsigned64(item1->credits->GetDownloadedTotal(), item2->credits->GetDownloadedTotal());
		    else if (item1->credits)
			    iResult = 1;
		    else
				iResult = -1;
			break;

//>>> WiZaRd::Show ModVer
		case 5:
			//Proper sorting ;)
			iResult = item1->GetClientSoft() - item2->GetClientSoft();
			if(iResult == 0)
				iResult = CompareLocaleStringNoCase(item1->DbgGetFullClientSoftVer(), item2->DbgGetFullClientSoftVer());
			break;
//<<< WiZaRd::Show ModVer

		case 6:
			if (item1->socket && item2->socket)
				iResult = item1->socket->IsConnected() - item2->socket->IsConnected();
			else if (item1->socket)
				iResult = 1;
			else
				iResult = -1;
			break;

		case 7:
			iResult = memcmp(item1->GetUserHash(), item2->GetUserHash(), 16);
			break;
// <CB Mod : Client Extended Info : Date 11.07.07>>
		case 8:
		// Sort by IP number
		{
			uint32 ip1 = ntohl(item1->GetIP());
			uint32 ip2 = ntohl(item2->GetIP());
			if(ip1 > ip2)
				return 1;
			else if (ip1 < ip2)
				return -1;
			else
				return 0;
		}
		break;
		// </CB Mod : Client Extended Info>
// ZZUL-TRA :: IP2Country :: Start
		case 9:
			iResult = CompareLocaleStringNoCase(item1->GetCountryName(true), item2->GetCountryName(true));
			break;
			// ZZUL-TRA :: IP2Country :: End
	}

	if (lParamSort >= 100)
		iResult = -iResult;

	//call secondary sortorder, if this one results in equal
	int dwNextSort;
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->transferwnd->GetClientList()->GetNextSortOrder(lParamSort)) != -1)
		iResult = SortProc(lParam1, lParam2, dwNextSort);

	return iResult;
}

void CClientListCtrl::OnNmDblClk(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1) {
		CUpDownClient* client = (CUpDownClient*)GetItemData(iSel);
		if (client){
			CClientDetailDialog dialog(client, this);
			dialog.DoModal();
		}
	}
	*pResult = 0;
}

void CClientListCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	const CUpDownClient* client = (iSel != -1) ? (CUpDownClient*)GetItemData(iSel) : NULL;

	CTitleMenu ClientMenu;
	ClientMenu.CreatePopupMenu();
	ClientMenu.AddMenuTitle(GetResString(IDS_CLIENTS), true);
	ClientMenu.AppendMenu(MF_STRING | (client ? MF_ENABLED : MF_GRAYED), MP_DETAIL, GetResString(IDS_SHOWDETAILS)/*, _T("CLIENTDETAILS")*/);
	ClientMenu.SetDefaultItem(MP_DETAIL);
        ClientMenu.AppendMenu(MF_SEPARATOR);
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && !client->IsFriend()) ? MF_ENABLED : MF_GRAYED), MP_ADDFRIEND, GetResString(IDS_ADDFRIEND)/*, _T("ADDFRIEND")*/);

	// <CB Mod : Friend Management : Easy Remove : Date 11.07.07>
#ifdef CB_MOD_EASYFRIENDSLOT_ENABLED
	ClientMenu.AppendMenu(MF_STRING | (client && client->IsFriend() ? MF_ENABLED : MF_GRAYED), MP_REMOVEFRIEND, GetResString(IDS_REMOVEFRIEND)/*, _T("DELETEFRIEND")*/);
	ClientMenu.AppendMenu(MF_STRING | (client && client->IsFriend() ? MF_ENABLED : MF_GRAYED), MP_FRIENDSLOT, GetResString(IDS_FRIENDSLOT)/*, _T("FRIENDSLOT")*/);
	ClientMenu.CheckMenuItem(MP_FRIENDSLOT, (client && client->GetFriendSlot()) ? MF_CHECKED : MF_UNCHECKED);
	ClientMenu.AppendMenu(MF_SEPARATOR); 
#endif
	// </CB Mod : Friend Management : Easy Remove>
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED), MP_MESSAGE, GetResString(IDS_SEND_MSG)/*, _T("SENDMESSAGE")*/);
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED), MP_SHOWLIST, GetResString(IDS_VIEWFILES)/*, _T("VIEWFILES")*/);
 	ClientMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED), MP_FIND, GetResString(IDS_FIND)/*, _T("Search")*/);

	if (Kademlia::CKademlia::IsRunning() && !Kademlia::CKademlia::IsConnected())
		ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetKadPort()!=0 && client->GetKadVersion() > 1) ? MF_ENABLED : MF_GRAYED), MP_BOOT, GetResString(IDS_BOOTSTRAP));

       if (thePrefs.IsExtControlsEnabled()) {
		ClientMenu.AppendMenu(MF_SEPARATOR);
		ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->IsBanned()) ? MF_ENABLED : MF_GRAYED), MP_UNBAN, GetResString(IDS_UNBAN)/*, _T("UNBAN")*/);
	}
	GetPopupMenuPos(*this, point);
	ClientMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
}

BOOL CClientListCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	wParam = LOWORD(wParam);

	switch (wParam)
	{
		case MP_FIND:
			OnFindStart();
			return TRUE;
	}

	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1){
		CUpDownClient* client = (CUpDownClient*)GetItemData(iSel);
		switch (wParam){
			case MP_SHOWLIST:
				client->RequestSharedFileList();
				break;
			case MP_MESSAGE:
				theApp.emuledlg->chatwnd->StartSession(client);
				break;
			case MP_ADDFRIEND:
				if (theApp.friendlist->AddFriend(client))
					Update(iSel);
				break;
			case MP_UNBAN:
				if (client->IsBanned()){
					client->UnBan();
					Update(iSel);
				}
				break;
			case MP_DETAIL:
			case MPG_ALTENTER:
			case IDA_ENTER:
			{
				CClientDetailDialog dialog(client, this);
				dialog.DoModal();
				break;
			}
			case MP_BOOT:
				if (client->GetKadPort() && client->GetKadVersion() > 1)
					Kademlia::CKademlia::Bootstrap(ntohl(client->GetIP()), client->GetKadPort());
				break;
			
	// <CB Mod : Friend Management : Easy FriendSlot : Date 11.07.07>
#ifdef CB_MOD_EASYFRIENDSLOT_ENABLED
			case MP_FRIENDSLOT:
				theApp.friendlist->AddFriend(client);
				if (client->GetFriendSlot())
					client->SetFriendSlot(false);
				else
					client->SetFriendSlot(true);
				Update(iSel);
				break;
			case MP_REMOVEFRIEND:
				if (client && client->IsFriend() && client->m_Friend != NULL) {
					theApp.friendlist->RemoveFriend(client->m_Friend);
					Update(iSel);
				}
				break;
#endif
			// </CB Mod : Friend Management : Easy Remove>
		}
	}
	return true;
}

void CClientListCtrl::AddClient(const CUpDownClient *client)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	if (thePrefs.IsKnownClientListDisabled())
		return;

	int iItemCount = GetItemCount();
	InsertItem(LVIF_TEXT | LVIF_PARAM, iItemCount, LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)client);
	theApp.emuledlg->transferwnd->UpdateListCount(CTransferDlg::wnd2Clients, iItemCount + 1);
}

void CClientListCtrl::RemoveClient(const CUpDownClient *client)
{
	if (!theApp.emuledlg->IsRunning())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	int result = FindItem(&find);
	if (result != -1) {
		DeleteItem(result);
		theApp.emuledlg->transferwnd->UpdateListCount(CTransferDlg::wnd2Clients);
	}
}

void CClientListCtrl::RefreshClient(const CUpDownClient *client)
{
	if (!theApp.emuledlg->IsRunning())
		return;

	if (theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd || !theApp.emuledlg->transferwnd->GetClientList()->IsWindowVisible())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	int result = FindItem(&find);
	if (result != -1)
		Update(result);
}

void CClientListCtrl::ShowSelectedUserDetails()
{
	POINT point;
	::GetCursorPos(&point);
	CPoint p = point; 
    ScreenToClient(&p); 
    int it = HitTest(p); 
    if (it == -1)
		return;

	SetItemState(-1, 0, LVIS_SELECTED);
	SetItemState(it, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	SetSelectionMark(it);   // display selection mark correctly!

	CUpDownClient* client = (CUpDownClient*)GetItemData(GetSelectionMark());
	if (client){
		CClientDetailDialog dialog(client, this);
		dialog.DoModal();
	}
}

void CClientListCtrl::ShowKnownClients()
{
	DeleteAllItems();
	int iItemCount = 0;
	for (POSITION pos = theApp.clientlist->list.GetHeadPosition(); pos != NULL; ) {
		const CUpDownClient *cur_client = theApp.clientlist->list.GetNext(pos);
		int iItem = InsertItem(LVIF_TEXT | LVIF_PARAM, iItemCount, LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)cur_client);
		Update(iItem);
		iItemCount++;
	}
	theApp.emuledlg->transferwnd->UpdateListCount(CTransferDlg::wnd2Clients, iItemCount);
}
