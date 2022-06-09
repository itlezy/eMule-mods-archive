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
#include "UploadQueue.h"


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
	,IDS_IP
	,IDS_STATS_COUNTRIES// X: [IP2L] - [IP2Location]
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

	/*InsertColumn(0, GetResString(IDS_QL_USERNAME),		LVCFMT_LEFT,  DFLT_CLIENTNAME_COL_WIDTH);
	InsertColumn(1, GetResString(IDS_CL_UPLOADSTATUS),	LVCFMT_LEFT,  100);
	InsertColumn(2, GetResString(IDS_CL_TRANSFUP),		LVCFMT_RIGHT, DFLT_SIZE_COL_WIDTH);
	InsertColumn(3, GetResString(IDS_CL_DOWNLSTATUS),	LVCFMT_LEFT,  100);
	InsertColumn(4, GetResString(IDS_CL_TRANSFDOWN),	LVCFMT_RIGHT, DFLT_SIZE_COL_WIDTH);
	InsertColumn(5, GetResString(IDS_CD_CSOFT),			LVCFMT_LEFT,  DFLT_CLIENTSOFT_COL_WIDTH);
	InsertColumn(6, GetResString(IDS_CONNECTED),		LVCFMT_LEFT,   50);
	CString coltemp;
	coltemp = GetResString(IDS_CD_UHASH);
	coltemp.Remove(_T(':'));
	InsertColumn(7, coltemp,							LVCFMT_LEFT,  DFLT_HASH_COL_WIDTH);

	//Xman
	// khaos::kmod+ Show IP
	InsertColumn(8,GetResString(IDS_IP),LVCFMT_LEFT,75);
	// khaos::kmod-
	InsertColumn(9,GetResString(IDS_STATS_COUNTRIES),LVCFMT_LEFT,75);// X: [IP2L] - [IP2Location]
	*/
	static const int colWidth[]={DFLT_CLIENTNAME_COL_WIDTH,
								100,
								DFLT_SIZE_COL_WIDTH,
								100,
								DFLT_PARTSTATUS_COL_WIDTH,
								DFLT_CLIENTSOFT_COL_WIDTH,
								50,
								DFLT_HASH_COL_WIDTH,
								75,
								100};

	CString strRes;
	for(int icol = 0; icol < _countof(colStrID); ++icol){
		strRes=GetResString(colStrID[icol]);
		if(icol == 7)
			strRes.Remove(_T(':'));
		InsertColumn(icol,strRes,LVCFMT_LEFT,colWidth[icol]);
	}
	
	SetAllIcons();
	//Localize();// X: [RUL] - [Remove Useless Localize]
	LoadSettings();
	SetSortArrow();
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0 : 100));
}

void CClientListCtrl::Localize()
{
	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
/*
	if(pHeaderCtrl->GetItemCount() != 0) {
		CString strRes;

		strRes = GetResString(IDS_QL_USERNAME);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(0, &hdi);

		strRes = GetResString(IDS_CL_UPLOADSTATUS);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(1, &hdi);

		strRes = GetResString(IDS_CL_TRANSFUP);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(2, &hdi);

		strRes = GetResString(IDS_CL_DOWNLSTATUS);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(3, &hdi);

		strRes = GetResString(IDS_CL_TRANSFDOWN);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(4, &hdi);

		strRes=GetResString(IDS_CD_CSOFT);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(5, &hdi);

		strRes = GetResString(IDS_CONNECTED);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(6, &hdi);

		strRes = GetResString(IDS_CD_UHASH);
		strRes.Remove(_T(':'));
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(7, &hdi);

		//Xman
		// khaos::kmod+
		strRes=GetResString(IDS_IP);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(8, &hdi);
		// khaos::kmod-
*/
	CString strRes;
	for (int icol=0;icol<pHeaderCtrl->GetItemCount();++icol) {
		strRes=GetResString(colStrID[icol]);
		if(icol == 7)
			strRes.Remove(_T(':'));
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(icol, &hdi);
	}
}

//Xman SortingFix for Morph-Code-Improvement Don't Refresh item if not needed
void CClientListCtrl::UpdateAll()
{
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0:100));
}
//Xman end

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
	theApp.SetClientIcon(m_ImageList);
	// Apply the image list also to the listview control, even if we use our own 'DrawItem'.
	// This is needed to give the listview control a chance to initialize the row height.
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) != 0 );
	VERIFY( ApplyImageList(m_ImageList) == NULL );
}

void CClientListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CRect cur_rec(lpDrawItemStruct->rcItem);
	const CUpDownClient *client = (CUpDownClient *)lpDrawItemStruct->itemData;
	CMemoryDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	//InitItemMemDC(dc, lpDrawItemStruct->rcItem, /*client->HasLowID() ? RGB(255,250,200) :*/ ((lpDrawItemStruct->itemID % 2)?m_crEvenLine:m_crWindow), lpDrawItemStruct->itemState); //Xman show LowIDs
    InitItemMemDC(dc, lpDrawItemStruct->rcItem, (client->IsFriend())?((client->GetFriendSlot())?RGB(210,240,255):RGB(200,255,200)):((lpDrawItemStruct->itemID % 2)?m_crEvenLine:m_crWindow), lpDrawItemStruct->itemState);

	RECT rcClient;// X: [DDHC] - [Don't Draw Hidden Column]
	GetClientRect(&rcClient);

	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	cur_rec.right = cur_rec.left - sm_iSubItemInset;
	cur_rec.left += sm_iSubItemInset;
	for(int iCurrent = 0; iCurrent < iCount; iCurrent++){
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
		if(IsColumnHidden(iColumn)) continue;
		int iColumnWidth = CListCtrl::GetColumnWidth(iColumn);
		cur_rec.right += iColumnWidth;
		if(iColumnWidth > 2*sm_iSubItemInset && cur_rec.right>0){// X: [DDHC] - [Don't Draw Hidden Column]
			TCHAR szItem[1024];
			switch(iColumn){
				case 0:{
					uint32 nOverlayImage = 0;
                    //morph4u share visible +
                    if (client->GetUserName() && client->GetViewSharedFilesSupport())
						nOverlayImage |= 5;
                    //morph4u share visible -
					if ((client->Credits() && client->Credits()->GetCurrentIdentState(client->GetIP()) == IS_IDENTIFIED))
						nOverlayImage |= 1;
					//Xman changed: display the obfuscation icon for all clients which enabled it
					if(client->IsObfuscatedConnectionEstablished() 
						|| (!(client->socket != NULL && client->socket->IsConnected())
						&& (client->SupportsCryptLayer() && thePrefs.IsClientCryptLayerSupported() && (client->RequestsCryptLayer() || thePrefs.IsClientCryptLayerRequested()))))
						nOverlayImage |= 2;
					int iIconPosY = (cur_rec.Height() > 19) ? ((cur_rec.Height() - 16) / 2) : 1;
					POINT point = {cur_rec.left, cur_rec.top + iIconPosY};
#ifdef CLIENTANALYZER			
					m_ImageList.Draw(dc,(client->IsBadGuy())?9:client->GetImageIndex(), point, ILD_NORMAL | INDEXTOOVERLAYMASK(nOverlayImage));					
#else
					m_ImageList.Draw(dc,client->GetImageIndex(), point, ILD_NORMAL | INDEXTOOVERLAYMASK(nOverlayImage));					
#endif
					cur_rec.left += 16;
					//EastShare Start - added by AndCycle, IP to Country 
					if(theApp.ip2country->ShowCountryFlag() )
					{
						point.x = cur_rec.left += sm_i2IconOffset;
						theApp.ip2country->GetFlagImageList()->Draw(dc, client->GetCountryFlagIndex(), point, ILD_NORMAL);
						cur_rec.left += 19;
					}
					//EastShare End - added by AndCycle, IP to Country

					if (client->GetUserName()){
						cur_rec.left += sm_iLabelOffset;
						_tcsncpy(szItem, client->GetUserName(), _countof(szItem) - 1);
						szItem[_countof(szItem) - 1] = _T('\0');
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT);
						cur_rec.left -= sm_iLabelOffset;
					}
					cur_rec.left -= 16;

					//EastShare Start - added by AndCycle, IP to Country
					if(theApp.ip2country->ShowCountryFlag() )
						cur_rec.left -= 19 + sm_i2IconOffset;
					//EastShare End - added by AndCycle, IP to Country
					break;
				}
				default:
					GetItemDisplayText(client, iColumn, szItem, _countof(szItem));
					if(szItem[0] != 0)
						dc.DrawText(szItem, -1, &cur_rec, (iColumn== 2 || iColumn == 4) ? MLC_DT_TEXT | DT_RIGHT : MLC_DT_TEXT);
					break;
			}
		}
		cur_rec.left += iColumnWidth;
		if(cur_rec.left>=rcClient.right)// X: [DDHC] - [Don't Draw Hidden Column]
			break;
	}
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
			if (client->GetUserName())
				_tcsncpy(pszText, client->GetUserName(), cchTextMax);
			break;

		case 1:
			_tcsncpy(pszText, client->GetUploadStateDisplayString(), cchTextMax);
			break;

		case 2:
			if(client->credits != NULL)
				_tcsncpy(pszText, CastItoXBytes(client->credits->GetUploadedTotal(), false, false), cchTextMax);
			break;

		case 3:
			_tcsncpy(pszText, client->GetDownloadStateDisplayString(), cchTextMax);
			break;

		case 4:
			if(client->credits != NULL)
				_tcsncpy(pszText, CastItoXBytes(client->credits->GetDownloadedTotal(), false, false), cchTextMax);
			break;

		case 5:
			_tcsncpy(pszText, client->DbgGetFullClientSoftVer(), cchTextMax);
			break;

		case 6:
			_tcsncpy(pszText, GetResString((client->socket && client->socket->IsConnected()) ? IDS_YES : IDS_NO), cchTextMax);
			break;

		case 7:
			md4str(client->GetUserHash(), pszText);
			break;
		//Xman
		// khaos::kmod+ Show IP
		case 8:
			_sntprintf(pszText, cchTextMax, _T("%s:%u"), client->GetUserIPString(), client->GetUserPort());
			break;
		// khaos::kmod- 
		case 9:// X: [IP2L] - [IP2Location]
			if(theApp.ip2country->IsIP2Country())
				_tcsncpy(pszText, client->GetCountryName(), cchTextMax);
			break;
	}
	pszText[cchTextMax - 1] = _T('\0');
}

void CClientListCtrl::OnLvnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (CemuleDlg::IsRunning()) {
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
	LPARAM iColumn = (lParamSort >= 100) ? lParamSort - 100 : lParamSort;
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
		    if (item1->GetDownloadState() == item2->GetDownloadState()) {
			    if (item1->IsRemoteQueueFull() && item2->IsRemoteQueueFull())
				    return 0;
				if (item1->IsRemoteQueueFull()||item1->GetRemoteQueueRank() == 0)
					iResult = 1;
				else if (item2->IsRemoteQueueFull()||item2->GetRemoteQueueRank() == 0)
					iResult = -1;
				else
					iResult = CompareUnsigned(item1->GetRemoteQueueRank(), item2->GetRemoteQueueRank());
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

		//Xman
		// Maella -Support for tag ET_MOD_VERSION 0x55
		case 5:
			if(item1->GetClientSoft() == item2->GetClientSoft())
				if(item2->GetVersion() == item1->GetVersion() && (item1->GetClientSoft() == SO_EMULE || item1->GetClientSoft() == SO_AMULE)){
					iResult= item1->DbgGetFullClientSoftVer().CompareNoCase( item2->DbgGetFullClientSoftVer());
				}
				else {
					iResult= item1->GetVersion() - item2->GetVersion();
				}
			else
				iResult= item2->GetClientSoft() - item1->GetClientSoft();
			break;

		// Xman end
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

		//Xman
		// khaos::kmod+ Show IP
		case 8:
			iResult = CompareUnsigned(ntohl(item1->GetIP()), ntohl(item2->GetIP()));
			break;
			/*
			if(item2->GetUserIPString() && item1->GetUserIPString())
				iResult= _tcsicmp(item1->GetUserIPString(), item2->GetUserIPString());
			else if(item1->GetUserIPString())
				iResult= 1;
			else
				iResult= -1;
			break;
			*/
		// khaos::kmod- Show IP
		case 9:// X: [IP2L] - [IP2Location]
			if(item1->GetCountryFlagIndex() == item2->GetCountryFlagIndex()){
				if(item1->m_structUserLocation == item2->m_structUserLocation)
					return 0;
				iResult = CompareLocaleString(item1->m_structUserLocation->locationName, item2->m_structUserLocation->locationName);
			}
			else
				iResult = CompareLocaleString(item1->m_structUserLocation->country->LongCountryName, item2->m_structUserLocation->country->LongCountryName);// X: [IP2L] - [IP2Location]
			break;
	}

	if (lParamSort >= 100)
		return -iResult;
	// SLUGFILLER: multiSort remove - handled in parent class
	/*
	//call secondary sortorder, if this one results in equal
	int dwNextSort;
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->transferwnd->GetClientList()->GetNextSortOrder(lParamSort)) != (-1)){
		iResult= SortProc(lParam1, lParam2, dwNextSort);
	}
	*/
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
	if (point.x != -1 || point.y != -1) {// X: [BF] - [Bug Fix]
		RECT rcClient;
		GetClientRect(&rcClient);
		ClientToScreen(&rcClient);
		if (!PtInRect(&rcClient,point)) {
			Default();
			return;
		}
	}
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	const CUpDownClient* client = (iSel != -1) ? (CUpDownClient*)GetItemData(iSel) : NULL;

	CTitleMenu ClientMenu;
	ClientMenu.CreatePopupMenu();
	ClientMenu.AddMenuTitle(GetResString(IDS_CLIENTS), true);
	ClientMenu.AppendMenu(MF_STRING | (client ? MF_ENABLED : MF_GRAYED), MP_DETAIL, GetResString(IDS_SHOWDETAILS));
	ClientMenu.SetDefaultItem(MP_DETAIL);
	//Xman friendhandling
	ClientMenu.AppendMenu(MF_SEPARATOR); 
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && !client->IsFriend()) ? MF_ENABLED : MF_GRAYED), MP_ADDFRIEND, GetResString(IDS_ADDFRIEND));
	ClientMenu.AppendMenu(MF_STRING | (client && client->IsFriend() ? MF_ENABLED : MF_GRAYED), MP_REMOVEFRIEND, GetResString(IDS_REMOVEFRIEND));
	ClientMenu.AppendMenu(MF_STRING | (client && client->IsFriend() ? (client->GetFriendSlot() ? MF_CHECKED : MF_UNCHECKED) : MF_GRAYED), MP_FRIENDSLOT, GetResString(IDS_FRIENDSLOT));
	ClientMenu.AppendMenu(MF_SEPARATOR); 
	//Xman end
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED), MP_MESSAGE, GetResString(IDS_SEND_MSG));
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED), MP_SHOWLIST, GetResString(IDS_VIEWFILES));
	if (client && thePrefs.IsExtControlsEnabled()){
		bool isbanned = client->IsBanned();
		ClientMenu.AppendMenu(MF_STRING | ((client->IsEd2kClient() && isbanned) ? MF_ENABLED : MF_GRAYED), MP_UNBAN, GetResString(IDS_UNBAN));
	}
	if (Kademlia::CKademlia::IsRunning() && !Kademlia::CKademlia::IsConnected())
		ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetKadPort()!=0 && client->GetKadVersion() > 1) ? MF_ENABLED : MF_GRAYED), MP_BOOT, GetResString(IDS_BOOTSTRAP));
	ClientMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED), MP_FIND, GetResString(IDS_FIND));

	// - show requested files (sivka/Xman)
	ClientMenu.AppendMenu(MF_SEPARATOR); 
	ClientMenu.AppendMenu(MF_STRING | (client ? MF_ENABLED : MF_GRAYED), MP_LIST_REQUESTED_FILES, GetResString(IDS_LISTREQUESTED)); 
	//Xman end

	GetPopupMenuPos(*this, point);
	ClientMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	VERIFY( ClientMenu.DestroyMenu() );
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
		if(!client) return true;
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
			//Xman friendhandling
			case MP_REMOVEFRIEND:
				if (/*client && */client->IsFriend())
				{
					theApp.friendlist->RemoveFriend(client->m_Friend);
					Update(iSel);
				}
				break;
			case MP_FRIENDSLOT: 
				//if (client)
				{
					bool IsAlready;				
					IsAlready = client->GetFriendSlot();
					theApp.friendlist->RemoveAllFriendSlots();
					if( !IsAlready )
						client->SetFriendSlot(true);
					Update(iSel);
				}
				break;
			//Xman end
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
				// - show requested files (sivka/Xman)
			case MP_LIST_REQUESTED_FILES: 
				{ 
				//if (client != NULL)
				//{
					client->ShowRequestedFiles(); 
				//}
				break;
				}
				//Xman end
		}
	}
	return true;
}

void CClientListCtrl::AddClient(const CUpDownClient *client)
{
	if (!CemuleDlg::IsRunning())
		return;
	if (thePrefs.IsKnownClientListDisabled())
		return;

	size_t iItemCount = GetItemCount();
	InsertItem(LVIF_TEXT | LVIF_PARAM, iItemCount, LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)client);
	theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Clients, iItemCount + 1);
}

void CClientListCtrl::RemoveClient(const CUpDownClient *client)
{
	if (!CemuleDlg::IsRunning())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	int result = FindItem(&find);
	if (result != -1) {
		DeleteItem(result);
		theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Clients);
	}
}

void CClientListCtrl::RefreshClient(const CUpDownClient *client)
{
	if (!CemuleDlg::IsRunning())
		return;

	if (theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd || !/*theApp.emuledlg->transferwnd->GetClientList()->*/IsWindowVisible())
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
		/*int iItem = */InsertItem(LVIF_TEXT | LVIF_PARAM, iItemCount, LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)cur_client);
		//Update(iItem);
		iItemCount++;
	}
	theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Clients, iItemCount);
}
