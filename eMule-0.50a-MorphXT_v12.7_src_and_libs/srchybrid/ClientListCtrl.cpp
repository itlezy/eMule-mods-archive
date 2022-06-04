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

	InsertColumn(0, GetResString(IDS_QL_USERNAME),		LVCFMT_LEFT,  DFLT_CLIENTNAME_COL_WIDTH);
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
	
	// Mighty Knife: Community affiliation
	if (thePrefs.IsCommunityEnabled ())//ShowColumn (8); //Removed by SiRoB, some people may prefere disable it
		InsertColumn(8,GetResString(IDS_COMMUNITY),LVCFMT_LEFT,100);
	else
		InsertColumn(8,GetResString(IDS_COMMUNITY),LVCFMT_LEFT,100,-1,true);
	// [end] Mighty Knife

	// EastShare - Added by Pretender, Friend Tab
	InsertColumn(9,GetResString(IDS_FRIENDLIST),LVCFMT_LEFT,75);
	// EastShare - Added by Pretender, Friend Tab
	// Commander - Added: IP2Country column - Start
	if (thePrefs.GetIP2CountryNameMode() == IP2CountryName_DISABLE)
		InsertColumn(10,GetResString(IDS_COUNTRY),LVCFMT_LEFT,100,-1,true);
	else
		InsertColumn(10,GetResString(IDS_COUNTRY),LVCFMT_LEFT,100);
	// Commander - Added: IP2Country column - End

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

	strRes = GetResString(IDS_CD_CSOFT);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(5, &hdi);

	strRes = GetResString(IDS_CONNECTED);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(6, &hdi);

	strRes = GetResString(IDS_CD_UHASH);
	strRes.Remove(_T(':'));
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(7, &hdi);

	// Mighty Knife: Community affiliation
	strRes=GetResString(IDS_COMMUNITY);
	strRes.Remove(_T(':'));
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(8, &hdi);
	// [end] Mighty Knife

	// EastShare - Added by Pretender, Friend Tab
	strRes=GetResString(IDS_FRIENDLIST);
	strRes.Remove(_T(':'));
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(9, &hdi);
	// EastShare - Added by Pretender, Friend Tab

	// Commander - Added: IP2Country column - Start
	strRes=GetResString(IDS_COUNTRY);
	strRes.Remove(_T(':'));
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(10, &hdi);
	// Commander - Added: IP2Country column - End
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
	m_ImageList.Add(CTempIconLoader(_T("ClientMLDonkey")));
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyHybrid")));
	m_ImageList.Add(CTempIconLoader(_T("ClientShareaza")));
	m_ImageList.Add(CTempIconLoader(_T("Server")));
	m_ImageList.Add(CTempIconLoader(_T("ClientAMule")));
	m_ImageList.Add(CTempIconLoader(_T("ClientLPhant")));
	//MORPH START - Added by SiRoB, More client icon & Credit ovelay icon
	m_ImageList.Add(CTempIconLoader(_T("ClientRightEdonkey")));
	m_ImageList.Add(CTempIconLoader(_T("Morph")));
	m_ImageList.Add(CTempIconLoader(_T("SCARANGEL")));
	m_ImageList.Add(CTempIconLoader(_T("STULLE")));
	m_ImageList.Add(CTempIconLoader(_T("XTREME")));
	m_ImageList.Add(CTempIconLoader(_T("EASTSHARE")));
	m_ImageList.Add(CTempIconLoader(_T("EMF")));
	m_ImageList.Add(CTempIconLoader(_T("NEO")));
	m_ImageList.Add(CTempIconLoader(_T("MEPHISTO")));
	m_ImageList.Add(CTempIconLoader(_T("XRAY")));
	m_ImageList.Add(CTempIconLoader(_T("MAGIC")));
	//MORPH END   - Added by SiRoB, More client icon & Credit ovelay icon
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("ClientSecureOvl"))), 1);
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("OverlayObfu"))), 2);
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("OverlaySecureObfu"))), 3);
	// Mighty Knife: Community icon
	m_overlayimages.DeleteImageList ();
	m_overlayimages.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	m_overlayimages.SetBkColor(CLR_NONE);
	m_overlayimages.Add(CTempIconLoader(_T("Community")));
	// [end] Mighty Knife
	//MORPH START - Addded by SiRoB, Friend Addon
	m_overlayimages.Add(CTempIconLoader(_T("ClientFriendOvl")));
	m_overlayimages.Add(CTempIconLoader(_T("ClientFriendSlotOvl")));
	//MORPH END   - Addded by SiRoB, Friend Addon
	//MORPH START - Credit Overlay Icon
	m_overlayimages.Add(CTempIconLoader(_T("ClientCreditOvl")));
	m_overlayimages.Add(CTempIconLoader(_T("ClientCreditSecureOvl")));
	//MORPH END   - Credit Overlay Icon
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
	
	//MORPH START - Changed by Stulle, Visual Studio 2010 Compatibility
	/*
	CMemDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	*/
	CMemoryDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	//MORPH END   - Changed by Stulle, Visual Studio 2010 Compatibility
	BOOL bCtrlFocused;
	InitItemMemDC(dc, lpDrawItemStruct, bCtrlFocused);
	CRect cur_rec(lpDrawItemStruct->rcItem);
	CRect rcClient;
	GetClientRect(&rcClient);
	const CUpDownClient *client = (CUpDownClient *)lpDrawItemStruct->itemData;

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
						//MORPH - Removed by SiRoB, Friend Addon
						/*
						if (client->IsFriend())
							iImage = 2;
						else if (client->GetClientSoft() == SO_EDONKEYHYBRID)
						*/
						if (client->GetClientSoft() == SO_EDONKEYHYBRID)
						//MORPH - Removed by SiRoB, Friend Addon
							iImage = 4;
						else if (client->GetClientSoft() == SO_MLDONKEY)
							iImage = 3;
						else if (client->GetClientSoft() == SO_SHAREAZA)
							iImage = 5;
						else if (client->GetClientSoft() == SO_URL)
							iImage = 6;
						else if (client->GetClientSoft() == SO_AMULE)
							iImage = 7;
						else if (client->GetClientSoft() == SO_LPHANT)
							iImage = 8;
						else if (client->ExtProtocolAvailable())
						//MORPH START - Modified by SiRoB, More client icon & Credit overlay icon
						{
							if(client->GetModClient() == MOD_NONE)
								iImage = 1;
							else
								iImage = (int)(client->GetModClient() + 9);
						}
						//MORPH END   - Modified by SiRoB, More client icon & Credit overlay icon
						else if (client->GetClientSoft() == SO_EDONKEY)
							iImage = 9;
						else
							iImage = 0;

						UINT nOverlayImage = 0;
						if ((client->Credits() && client->Credits()->GetCurrentIdentState(client->GetIP()) == IS_IDENTIFIED))
							nOverlayImage |= 1;
						if (client->IsObfuscatedConnectionEstablished())
							nOverlayImage |= 2;
						int iIconPosY = (cur_rec.Height() > 16) ? ((cur_rec.Height() - 16) / 2) : 1;
						POINT point = { cur_rec.left, cur_rec.top + iIconPosY };
						m_ImageList.Draw(dc, iImage, point, ILD_NORMAL | INDEXTOOVERLAYMASK(nOverlayImage));

						//MORPH START - Credit Overlay Icon
						if (client->Credits() && client->Credits()->GetHasScore(client->GetIP())) {
							if (nOverlayImage & 1)
								m_overlayimages.Draw(dc, 4, point, ILD_TRANSPARENT);
							else 
								m_overlayimages.Draw(dc, 3, point, ILD_TRANSPARENT);
						}
						// Mighty Knife: Community visualization
						if (client->IsCommunity())
							m_overlayimages.Draw(dc,0, point, ILD_TRANSPARENT);
						// [end] Mighty Knife
						//MORPH END   - Credit Overlay Icon
						if (client->IsFriend())
							m_overlayimages.Draw(dc,client->GetFriendSlot()?2:1, point, ILD_TRANSPARENT);
						//MORPH END - Modified by SiRoB, More client icon

						//Commander: There is a column now to show the country name
						if(theApp.ip2country->ShowCountryFlag() && IsColumnHidden(10)){
							cur_rec.left+=20;
							POINT point2= {cur_rec.left,cur_rec.top+1};
							//int index = client->GetCountryFlagIndex();
							//theApp.ip2country->GetFlagImageList()->DrawIndirect(dc, index , point2, CSize(18,16), CPoint(0,0), ILD_NORMAL);
							theApp.ip2country->GetFlagImageList()->DrawIndirect(&theApp.ip2country->GetFlagImageDrawParams(dc,client->GetCountryFlagIndex(),point2));
							cur_rec.left += sm_iLabelOffset;
						}
						//EastShare End - added by AndCycle, IP to Country

						cur_rec.left += 16 + sm_iLabelOffset;
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
						cur_rec.left -= 16;
						cur_rec.right -= sm_iSubItemInset;
						//EastShare Start - added by AndCycle, IP to Country
						if(theApp.ip2country->ShowCountryFlag() && IsColumnHidden(10)){
							cur_rec.left-=20;
						}
						//EastShare End - added by AndCycle, IP to Country
						break;
					}
					// Commander - Added: IP2Country column - Start
					case 10:
						//Commander: There is a column now to show the country name
						if(theApp.ip2country->ShowCountryFlag()){
							POINT point2= {cur_rec.left,cur_rec.top+1};
							//int index = client->GetCountryFlagIndex();
							//theApp.ip2country->GetFlagImageList()->DrawIndirect(dc, index , point2, CSize(18,16), CPoint(0,0), ILD_NORMAL);
							theApp.ip2country->GetFlagImageList()->DrawIndirect(&theApp.ip2country->GetFlagImageDrawParams(dc,client->GetCountryFlagIndex(),point2));
							cur_rec.left+=20;
						}
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
						if(theApp.ip2country->ShowCountryFlag()){
							cur_rec.left-=20;
							cur_rec.left -= sm_iLabelOffset;
						}
						break;
					// Commander - Added: IP2Country column - End

					default:
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
						break;
				}
			}
			cur_rec.left += iColumnWidth;
		}
	}

	DrawFocusRect(dc, lpDrawItemStruct->rcItem, lpDrawItemStruct->itemState & ODS_FOCUS, bCtrlFocused, lpDrawItemStruct->itemState & ODS_SELECTED);
	if (!theApp.IsRunningAsService(SVC_LIST_OPT)) // MORPH leuk_he:run as ntservice v1..
		m_updatethread->AddItemUpdated((LPARAM)client); //MORPH - UpdateItemThread
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

		case 5:
			//MORPH START
			/*
			_tcsncpy(pszText, client->GetClientSoftVer(), cchTextMax);
			*/
			_sntprintf(pszText, cchTextMax, _T("%s%s"), client->GetClientSoftVer(), client->GetClientModTag());
			//MORPH END
			if (pszText[0] == _T('\0'))
				_tcsncpy(pszText, GetResString(IDS_UNKNOWN), cchTextMax);
			break;

		case 6:
			_tcsncpy(pszText, GetResString((client->socket && client->socket->IsConnected()) ? IDS_YES : IDS_NO), cchTextMax);
			break;

		case 7:
			_tcsncpy(pszText, md4str(client->GetUserHash()), cchTextMax);
			break;
		// Mighty Knife: Community affiliation
		case 8:
			_tcsncpy(pszText, client->IsCommunity () ? GetResString(IDS_YES) : _T(""), cchTextMax);
			break;
		// [end] Mighty Knife
		// EastShare - Added by Pretender, Friend Tab
		case 9:
			_tcsncpy(pszText, client->IsFriend () ? GetResString(IDS_YES) : _T(""), cchTextMax);
			break;
		// EastShare - Added by Pretender, Friend Tab
		// Commander - Added: IP2Country column - Start
		case 10:
			_tcsncpy(pszText, client->GetCountryName(), cchTextMax);
			break;
		// Commander - Added: IP2Country column - End
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
			//MORPH START - Added by Stulle, Improved upload state sorting for additional information
			/*
		    iResult = item1->GetUploadState() - item2->GetUploadState();
			*/
			iResult = CompareUnsigned(item1->GetUploadStateExtended() ,item2->GetUploadStateExtended());
			//MORPH END   - Added by Stulle, Improved upload state sorting for additional information
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
			//MORPH START - Changed by Stulle, Improved sorting for Download State in ClientListCtrl [TAHO]
			/*
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
			*/
		{
			EDownloadState clientState1 = item1->GetDownloadState();
			EDownloadState clientState2 = item2->GetDownloadState();

			if ( clientState1 == DS_DOWNLOADING ){
				if ( clientState2 == DS_DOWNLOADING) {
					iResult = CompareUnsigned(item1->GetDownloadDatarate(), item2->GetDownloadDatarate());
					break;
				}
				iResult = 1;
				break;
			} else if ( clientState2 == DS_DOWNLOADING) {
				iResult = -1;
				break;
			}

			if ( clientState1 == DS_ONQUEUE ){
				if ( clientState2 == DS_ONQUEUE ) {
					if ( item1->IsRemoteQueueFull() ){
						iResult = (item2->IsRemoteQueueFull()) ? 0 : -1;
						break;
					}
					else if ( item2->IsRemoteQueueFull() ){
						iResult = 1;
						break;
					}

					if ( item1->GetRemoteQueueRank() ){
						iResult = (item2->GetRemoteQueueRank()) ? item2->GetRemoteQueueRank() - item1->GetRemoteQueueRank() : 1;
						break;
					}
					iResult = (item2->GetRemoteQueueRank()) ? -1 : 0;
					break;
				}
				iResult = 1;
				break;
			} else if ( clientState2 == DS_ONQUEUE ){
				iResult = -1;
				break;
			}

			if ( clientState1 == DS_NONEEDEDPARTS && clientState2 != DS_NONEEDEDPARTS){
				iResult = 1;
				break;
			}

			if ( clientState1 == DS_TOOMANYCONNS && clientState2 != DS_TOOMANYCONNS){
				iResult = -1;
				break;
			}

			iResult = 0;
		}
			//MORPH START - Changed by Stulle, Improved sorting for Download State in ClientListCtrl [TAHO]
			break;

		case 4:
			if (item1->credits && item2->credits)
				iResult = CompareUnsigned64(item1->credits->GetDownloadedTotal(), item2->credits->GetDownloadedTotal());
		    else if (item1->credits)
			    iResult = 1;
		    else
				iResult = -1;
			break;

		//MORPH START - Added by IceCream, ET_MOD_VERSION
		/*
		case 5:
			if (item1->GetClientSoft() == item2->GetClientSoft())
			    iResult = item1->GetVersion() - item2->GetVersion();
		    else 
				iResult = -(item1->GetClientSoft() - item2->GetClientSoft()); // invert result to place eMule's at top
			break;
		*/
		case 5:
			if (item1->GetClientSoft() == item2->GetClientSoft())
				if (item2->GetVersion() == item1->GetVersion() && (item1->GetClientSoft() == SO_EMULE || item1->GetClientSoft() == SO_AMULE)){
					iResult= CompareOptLocaleStringNoCase(item2->GetClientSoftVer(), item1->GetClientSoftVer());
				}
				else {
					iResult= item1->GetVersion() - item2->GetVersion();
				}
			else
				iResult=-(item1->GetClientSoft() - item2->GetClientSoft());
			break;
		//MORPH END   - Added by IceCream, ET_MOD_VERSION

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
		// Mighty Knife: Community affiliation
		case 8:
			iResult = item1->IsCommunity() - item2->IsCommunity();
			break;
		// [end] Mighty Knife
		// EastShare - Added by Pretender, Friend Tab
		case 9:
			iResult=item1->IsFriend() - item2->IsFriend();
			break;
		// EastShare - Added by Pretender, Friend Tab
	        // Commander - Added: IP2Country column - Start
	        case 10:
			if(item1->GetCountryName(true) && item2->GetCountryName(true))
				iResult=CompareLocaleStringNoCase(item1->GetCountryName(true), item2->GetCountryName(true));
			else if(item1->GetCountryName(true))
				iResult=1;
			else
				iResult=-1;
			break;
		// Commander - Added: IP2Country column - End		
	}

	if (lParamSort >= 100)
		iResult = -iResult;

	// SLUGFILLER: multiSort remove - handled in parent class
	/*
	//call secondary sortorder, if this one results in equal
	int dwNextSort;
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->transferwnd->GetClientList()->GetNextSortOrder(lParamSort)) != -1)
		iResult = SortProc(lParam1, lParam2, dwNextSort);
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
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	const CUpDownClient* client = (iSel != -1) ? (CUpDownClient*)GetItemData(iSel) : NULL;

	CTitleMenu ClientMenu;
	ClientMenu.CreatePopupMenu();
	ClientMenu.AddMenuTitle(GetResString(IDS_CLIENTS), true);
	ClientMenu.AppendMenu(MF_STRING | (client ? MF_ENABLED : MF_GRAYED), MP_DETAIL, GetResString(IDS_SHOWDETAILS), _T("CLIENTDETAILS"));
	ClientMenu.SetDefaultItem(MP_DETAIL);
	if(!thePrefs.IsLessControls()){ //MORPH show less controls
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && !client->IsFriend()) ? MF_ENABLED : MF_GRAYED), MP_ADDFRIEND, GetResString(IDS_ADDFRIEND), _T("ADDFRIEND"));
	//MORPH START - Added by SiRoB, Friend Addon
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->IsFriend()) ? MF_ENABLED : MF_GRAYED), MP_REMOVEFRIEND, GetResString(IDS_REMOVEFRIEND), _T("DELETEFRIEND"));
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->IsFriend()) ? MF_ENABLED  | ((!client->HasLowID() && client->IsFriend() && client->GetFriendSlot())?MF_CHECKED : MF_UNCHECKED) : MF_GRAYED), MP_FRIENDSLOT, GetResString(IDS_FRIENDSLOT), _T("FRIENDSLOT"));
	//MORPH END - Added by SiRoB, Friend Addon
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED), MP_MESSAGE, GetResString(IDS_SEND_MSG), _T("SENDMESSAGE"));
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED), MP_SHOWLIST, GetResString(IDS_VIEWFILES), _T("VIEWFILES"));
	if (Kademlia::CKademlia::IsRunning() && !Kademlia::CKademlia::IsConnected())
		ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetKadPort()!=0 && client->GetKadVersion() > 1) ? MF_ENABLED : MF_GRAYED), MP_BOOT, GetResString(IDS_BOOTSTRAP));
	ClientMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED), MP_FIND, GetResString(IDS_FIND), _T("Search"));
	//MORPH START - Added by Yun.SF3, List Requested Files
	ClientMenu.AppendMenu(MF_SEPARATOR); // Added by sivka [sivka: -listing all requested files from user-]
	ClientMenu.AppendMenu(MF_STRING,MP_LIST_REQUESTED_FILES, GetResString(IDS_LISTREQUESTED), _T("FILEREQUESTED")); // Added by sivka
	//MORPH END - Added by Yun.SF3, List Requested Files
	//MORPH START show less controls
	}
	else
	{
		ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED), MP_MESSAGE, GetResString(IDS_SEND_MSG), _T("SENDMESSAGE"));
		ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED), MP_SHOWLIST, GetResString(IDS_VIEWFILES), _T("VIEWFILES"));
		ClientMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED), MP_FIND, GetResString(IDS_FIND), _T("Search"));
	}
	//MORPH END show less controls
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
			//MORPH START - Added by SIRoB, Friend Addon
            		//Xman friendhandling
			case MP_FRIENDSLOT:{
				//MORPH START - Modified by SIRoB, Added by Yun.SF3, ZZ Upload System
				if (client){
					bool IsAlready;
					IsAlready = client->GetFriendSlot();
					//theApp.friendlist->RemoveAllFriendSlots();
					if( IsAlready ) {
						client->SetFriendSlot(false);
					} else {
						client->SetFriendSlot(true);
					}
					theApp.friendlist->ShowFriends();
				}
				//MORPH END - Modified by SIRoB, Added by Yun.SF3, ZZ Upload System
				break;
			}
			case MP_REMOVEFRIEND:{//LSD
				if (client && client->IsFriend())
				{					
					theApp.friendlist->RemoveFriend(client->m_Friend);
					//this->UpdateItem(client);
				}
				break;
			}
			//Xman end
			//MORPH END   - Added by SIRoB, Friend Addon
			//MORPH START - Added by Yun.SF3, List Requested Files
			case MP_LIST_REQUESTED_FILES: { // added by sivka
				if (client != NULL)
				{
					client->ShowRequestedFiles(); //Changed by SiRoB
				}
				break;
			}
			//MORPH END - Added by Yun.SF3, List Requested Files
		}
	}
	return true;
}

void CClientListCtrl::AddClient(const CUpDownClient *client)
{
	if (theApp.IsRunningAsService(SVC_LIST_OPT)) return;// MORPH leuk_he:run as ntservice v1..

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
	if (theApp.IsRunningAsService(SVC_LIST_OPT)) return;// MORPH leuk_he:run as ntservice v1..

	if (!theApp.emuledlg->IsRunning())
		return;

	if (theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd || !theApp.emuledlg->transferwnd->GetClientList()->IsWindowVisible())
		return;

	//MORPH START- UpdateItemThread
	/*
	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	int result = FindItem(&find);
	if (result != -1)
		Update(result);
	*/
	m_updatethread->AddItemToUpdate((LPARAM)client);
	//MORPH END - UpdateItemThread
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
