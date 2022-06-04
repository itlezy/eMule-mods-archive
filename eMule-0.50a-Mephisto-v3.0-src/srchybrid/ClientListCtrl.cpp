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

	//Xman
	// khaos::kmod+ Show IP
	InsertColumn(8,GetResString(IDS_IP),LVCFMT_LEFT,75,8);
	// khaos::kmod-

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

	//Xman
	// khaos::kmod+
	strRes=GetResString(IDS_IP);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(8, &hdi);
	// khaos::kmod-

	// ==> Design Settings [eWombat/Stulle] - Stulle
	theApp.emuledlg->transferwnd->SetBackgroundColor(style_b_clientlist);
	// <== Design Settings [eWombat/Stulle] - Stulle
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
	//Xman Show correct Icons	
	/*
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkey")));
	m_ImageList.Add(CTempIconLoader(_T("ClientCompatible")));
	m_ImageList.Add(CTempIconLoader(_T("Friend")));
	m_ImageList.Add(CTempIconLoader(_T("ClientMLDonkey")));
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyHybrid")));
	m_ImageList.Add(CTempIconLoader(_T("ClientShareaza")));
	m_ImageList.Add(CTempIconLoader(_T("Server")));
	m_ImageList.Add(CTempIconLoader(_T("ClientAMule")));
	m_ImageList.Add(CTempIconLoader(_T("ClientLPhant")));
	*/
	m_ImageList.Add(CTempIconLoader(_T("ClientDefault")));		//0
	m_ImageList.Add(CTempIconLoader(_T("ClientDefaultPlus")));	//1
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkey")));		//2
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyPlus")));	//3
	m_ImageList.Add(CTempIconLoader(_T("ClientCompatible")));		//4
	m_ImageList.Add(CTempIconLoader(_T("ClientCompatiblePlus")));	//5
	m_ImageList.Add(CTempIconLoader(_T("ClientFriend")));			//6
	m_ImageList.Add(CTempIconLoader(_T("ClientFriendPlus")));		//7
	m_ImageList.Add(CTempIconLoader(_T("ClientMLDonkey")));		//8
	m_ImageList.Add(CTempIconLoader(_T("ClientMLDonkeyPlus")));	//9
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyHybrid")));	//10
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyHybridPlus")));//11
	m_ImageList.Add(CTempIconLoader(_T("ClientShareaza")));		//12
	m_ImageList.Add(CTempIconLoader(_T("ClientShareazaPlus")));	//13
	m_ImageList.Add(CTempIconLoader(_T("ClientAMule")));			//14
	m_ImageList.Add(CTempIconLoader(_T("ClientAMulePlus")));		//15
	m_ImageList.Add(CTempIconLoader(_T("ClientLPhant")));			//16
	m_ImageList.Add(CTempIconLoader(_T("ClientLPhantPlus")));		//17
	m_ImageList.Add(CTempIconLoader(_T("LEECHER")));				//18 //Xman Anti-Leecher

	//Xman friend visualization
	m_ImageList.Add(CTempIconLoader(_T("ClientFriendSlotOvl"))); //19
	//Xman end
	//Xman end

	// ==> Mod Icons - Stulle
	// ==> Mephisto mod [Stulle] - Mephisto
	/*
	m_ImageList.Add(CTempIconLoader(_T("AAAEMULEAPP"))); //20
	*/
	m_ImageList.Add(CTempIconLoader(_T("SCARANGEL"))); //20
	// <== Mephisto mod [Stulle] - Mephisto
	m_ImageList.Add(CTempIconLoader(_T("STULLE"))); //21
	m_ImageList.Add(CTempIconLoader(_T("XTREME"))); //22
	m_ImageList.Add(CTempIconLoader(_T("MORPH"))); //23
	m_ImageList.Add(CTempIconLoader(_T("EASTSHARE"))); //24
	m_ImageList.Add(CTempIconLoader(_T("EMF"))); //25
	m_ImageList.Add(CTempIconLoader(_T("NEO"))); //26
	// ==> Mephisto mod [Stulle] - Mephisto
	/*
	m_ImageList.Add(CTempIconLoader(_T("MEPHISTO"))); //27
	*/
	m_ImageList.Add(CTempIconLoader(_T("AAAEMULEAPP"))); //27
	// <== Mephisto mod [Stulle] - Mephisto
	m_ImageList.Add(CTempIconLoader(_T("XRAY"))); //28
	m_ImageList.Add(CTempIconLoader(_T("MAGIC"))); //29
	// <== Mod Icons - Stulle

	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("ClientSecureOvl"))), 1);
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("OverlayObfu"))), 2);
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("OverlaySecureObfu"))), 3);

	// ==> Mod Icons - Stulle
	m_overlayimages.DeleteImageList ();
	m_overlayimages.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	m_overlayimages.SetBkColor(CLR_NONE);
	m_overlayimages.Add(CTempIconLoader(_T("ClientCreditOvl")));
	m_overlayimages.Add(CTempIconLoader(_T("ClientCreditSecureOvl")));
	// <== Mod Icons - Stulle

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
	
	// ==> Visual Studio 2010 Compatibility [Stulle/Avi-3k/ied] - Stulle
	/*
	CMemDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	*/
	CMemoryDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	// <== Visual Studio 2010 Compatibility [Stulle/Avi-3k/ied] - Stulle
	BOOL bCtrlFocused;
	//Xman narrow font at transferwindow
	/*
	InitItemMemDC(dc, lpDrawItemStruct, bCtrlFocused);
	*/
	// ==> Design Settings [eWombat/Stulle] - Stulle
	/*
	InitItemMemDC(dc, lpDrawItemStruct, bCtrlFocused, true);
	//Xman end
	*/
	InitItemMemDC(dc, lpDrawItemStruct, bCtrlFocused, true, style_b_clientlist);
	// <== Design Settings [eWombat/Stulle] - Stulle
	CRect cur_rec(lpDrawItemStruct->rcItem);
	CRect rcClient;
	GetClientRect(&rcClient);
	const CUpDownClient *client = (CUpDownClient *)lpDrawItemStruct->itemData;

	// ==> Design Settings [eWombat/Stulle] - Stulle
	/*
	COLORREF crOldBackColor = dc->GetBkColor(); //Xman show LowIDs
	*/
	// <== Design Settings [eWombat/Stulle] - Stulle
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
						//Xman Show correct Icons
						/*
						if (client->IsFriend())
							iImage = 2;
						else if (client->GetClientSoft() == SO_EDONKEYHYBRID)
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
							iImage = 1;
						else
							iImage = 0;
						*/
						//Xman Anti-Leecher
						if(client->IsLeecher()>0)
							iImage = 18;
						//Xman end
						else if (client->IsFriend())
							iImage = 6;
						else if (client->GetClientSoft() == SO_EDONKEYHYBRID){
							iImage = 10;
						}
						else if (client->GetClientSoft() == SO_EDONKEY){
							iImage = 2;
						}
						else if (client->GetClientSoft() == SO_MLDONKEY){
							iImage = 8;
						}
						else if (client->GetClientSoft() == SO_SHAREAZA){
							iImage = 12;
						}
						else if (client->GetClientSoft() == SO_AMULE){
							iImage = 14;
						}
						else if (client->GetClientSoft() == SO_LPHANT){
							iImage = 16;
						}
						else if (client->ExtProtocolAvailable()){
							// ==> Mod Icons - Stulle
							/*
								iImage = 4;
							*/
							if(client->GetModClient() == MOD_NONE)
								iImage = 4;
							else
								iImage = (uint8)(client->GetModClient() + 19);
							// <== Mod Icons - Stulle
						}
						else{
							iImage = 0;
						}
						//Xman end

						// ==> Mod Icons - Stulle
						// ==> CreditSystems [EastShare/ MorphXT] - Stulle
						if (((client->credits)?client->credits->GetScoreRatio(client):0) > 1)
							if (client->GetModClient() == MOD_NONE){
								if(client->credits && client->credits->GetHasScore(client))
									iImage++;
							}
						// <== CreditSystems [EastShare/ MorphXT] - Stulle
						// <== Mod Icons - Stulle

						UINT nOverlayImage = 0;
						if ((client->Credits() && client->Credits()->GetCurrentIdentState(client->GetIP()) == IS_IDENTIFIED))
							nOverlayImage |= 1;
						//Xman changed: display the obfuscation icon for all clients which enabled it
						/*
						if (client->IsObfuscatedConnectionEstablished())
						*/
						if(client->IsObfuscatedConnectionEstablished() 
							|| (!(client->socket != NULL && client->socket->IsConnected())
							&& (client->SupportsCryptLayer() && thePrefs.IsClientCryptLayerSupported() && (client->RequestsCryptLayer() || thePrefs.IsClientCryptLayerRequested()))))
						//Xman End
							nOverlayImage |= 2;
						int iIconPosY = (cur_rec.Height() > 16) ? ((cur_rec.Height() - 16) / 2) : 1;
						POINT point = { cur_rec.left, cur_rec.top + iIconPosY };
						m_ImageList.Draw(dc, iImage, point, ILD_NORMAL | INDEXTOOVERLAYMASK(nOverlayImage));

						// ==> Mod Icons - Stulle
						if (client->Credits() && client->Credits()->GetHasScore(client) && client->GetModClient() != MOD_NONE)
						{
							if (nOverlayImage & 1)
								m_overlayimages.Draw(dc, 1, point, ILD_TRANSPARENT);
							else 
								m_overlayimages.Draw(dc, 0, point, ILD_TRANSPARENT);
						}
						// <== Mod Icons - Stulle

						//Xman friend visualization
						if (client->IsFriend() && client->GetFriendSlot())
							m_ImageList.Draw(dc,19, point, ILD_NORMAL);
						//Xman end

						//EastShare Start - added by AndCycle, IP to Country 
						if(theApp.ip2country->ShowCountryFlag() )
						{
							cur_rec.left+=20;
							POINT point2= {cur_rec.left,cur_rec.top+1};
							//theApp.ip2country->GetFlagImageList()->Draw(dc, client->GetCountryFlagIndex(), point2, ILD_NORMAL);
							theApp.ip2country->GetFlagImageList()->DrawIndirect(&theApp.ip2country->GetFlagImageDrawParams(dc,client->GetCountryFlagIndex(),point2));
							cur_rec.left += sm_iLabelOffset;
						}
						//EastShare End - added by AndCycle, IP to Country

						cur_rec.left += 16 + sm_iLabelOffset;
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
						cur_rec.left -= 16;
						cur_rec.right -= sm_iSubItemInset;

						//EastShare Start - added by AndCycle, IP to Country
						if(theApp.ip2country->ShowCountryFlag() )
						{
							cur_rec.left-=20;
						}
						//EastShare End - added by AndCycle, IP to Country
						break;
					}

					default:
						// ==> Design Settings [eWombat/Stulle] - Stulle
						/*
						//Xman show LowIDs
						if(iColumn == 5 && client->HasLowID()) 
							dc.SetBkColor(RGB(255,250,200));
						//Xman End
						*/
						// <== Design Settings [eWombat/Stulle] - Stulle
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
						// ==> Design Settings [eWombat/Stulle] - Stulle
						/*
						dc.SetBkColor(crOldBackColor); //Xman show LowIDs
						*/
						// <== Design Settings [eWombat/Stulle] - Stulle
						break;
				}
			}
			cur_rec.left += iColumnWidth;
		}
	}

	DrawFocusRect(dc, lpDrawItemStruct->rcItem, lpDrawItemStruct->itemState & ODS_FOCUS, bCtrlFocused, lpDrawItemStruct->itemState & ODS_SELECTED);
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
			// Maella -Support for tag ET_MOD_VERSION 0x55
			/*
			_tcsncpy(pszText, client->GetClientSoftVer(), cchTextMax);
			*/
			_tcsncpy(pszText, client->DbgGetFullClientSoftVer(), cchTextMax);
			//Xman end
			if (pszText[0] == _T('\0'))
				_tcsncpy(pszText, GetResString(IDS_UNKNOWN), cchTextMax);
			break;

		case 6:
			_tcsncpy(pszText, GetResString((client->socket && client->socket->IsConnected()) ? IDS_YES : IDS_NO), cchTextMax);
			break;

		case 7:
			_tcsncpy(pszText, md4str(client->GetUserHash()), cchTextMax);
			break;
		//Xman
		// khaos::kmod+ Show IP
		case 8:
			_sntprintf(pszText, cchTextMax, _T("%s:%u"), client->GetUserIPString(), client->GetUserPort());
			break;
		// khaos::kmod- 
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

		case 5:
			//Xman
			// Maella -Support for tag ET_MOD_VERSION 0x55
			/*
			if (item1->GetClientSoft() == item2->GetClientSoft())
			    iResult = item1->GetVersion() - item2->GetVersion();
		    else 
				iResult = -(item1->GetClientSoft() - item2->GetClientSoft()); // invert result to place eMule's at top
			*/
			if(item1->GetClientSoft() == item2->GetClientSoft())
				if(item1->GetVersion() == item2->GetVersion() && (item1->GetClientSoft() == SO_EMULE || item1->GetClientSoft() == SO_AMULE)){
					iResult = item2->DbgGetFullClientSoftVer().CompareNoCase( item1->DbgGetFullClientSoftVer());
				}
				else {
					iResult = item1->GetVersion() - item2->GetVersion();
				}
			else
				iResult = -(item1->GetClientSoft() - item2->GetClientSoft()); // invert result to place eMule's at top
			//Xman end
			break;

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
		// khaos::kmod- Show IP
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
	// SLUGFILLER: multiSort remove - handled in parent class

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
	//Xman friendhandling
	ClientMenu.AppendMenu(MF_SEPARATOR); 
	//Xman end
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && !client->IsFriend()) ? MF_ENABLED : MF_GRAYED), MP_ADDFRIEND, GetResString(IDS_ADDFRIEND), _T("ADDFRIEND"));
	//Xman friendhandling
	ClientMenu.AppendMenu(MF_STRING | (client && client->IsFriend() ? MF_ENABLED : MF_GRAYED), MP_REMOVEFRIEND, GetResString(IDS_REMOVEFRIEND), _T("DELETEFRIEND"));
	ClientMenu.AppendMenu(MF_STRING | (client && client->IsFriend() ? MF_ENABLED : MF_GRAYED), MP_FRIENDSLOT, GetResString(IDS_FRIENDSLOT), _T("FRIENDSLOT"));
	ClientMenu.CheckMenuItem(MP_FRIENDSLOT, (client && client->GetFriendSlot()) ? MF_CHECKED : MF_UNCHECKED);
	ClientMenu.AppendMenu(MF_SEPARATOR); 
	//Xman end
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED), MP_MESSAGE, GetResString(IDS_SEND_MSG), _T("SENDMESSAGE"));
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED), MP_SHOWLIST, GetResString(IDS_VIEWFILES), _T("VIEWFILES"));
	if (Kademlia::CKademlia::IsRunning() && !Kademlia::CKademlia::IsConnected())
		ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetKadPort()!=0 && client->GetKadVersion() > 1) ? MF_ENABLED : MF_GRAYED), MP_BOOT, GetResString(IDS_BOOTSTRAP));
	ClientMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED), MP_FIND, GetResString(IDS_FIND), _T("Search"));

	// - show requested files (sivka/Xman)
	ClientMenu.AppendMenu(MF_SEPARATOR); 
	ClientMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED),MP_LIST_REQUESTED_FILES, GetResString(IDS_LISTREQUESTED), _T("FILEREQUESTED")); 
	//Xman end

	GetPopupMenuPos(*this, point);
	ClientMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	VERIFY( ClientMenu.DestroyMenu() ); // XP Style Menu [Xanatos] - Stulle
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
			//Xman friendhandling
			case MP_REMOVEFRIEND:
				if (client && client->IsFriend())
				{
					theApp.friendlist->RemoveFriend(client->m_Friend);
					Update(iSel);
				}
				break;
			case MP_FRIENDSLOT: 
				if (client)
				{
					bool IsAlready;				
					IsAlready = client->GetFriendSlot();
					// ==> Multiple friendslots [ZZ] - Mephisto
					/*
					theApp.friendlist->RemoveAllFriendSlots();
					*/
					// <== Multiple friendslots [ZZ] - Mephisto
					if( !IsAlready )
						client->SetFriendSlot(true);
					// ==> Multiple friendslots [ZZ] - Mephisto
					else
						client->SetFriendSlot(false);
					// <== Multiple friendslots [ZZ] - Mephisto
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
				if (client != NULL)
				{
					client->ShowRequestedFiles(); 
				}
				break;
			}
			//Xman end
		}
	}
	return true;
}

void CClientListCtrl::AddClient(const CUpDownClient *client)
{
	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	if (theApp.IsRunningAsService(SVC_LIST_OPT))
		return;
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle

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
	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	if (theApp.IsRunningAsService(SVC_LIST_OPT))
		return;
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle

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

//Xman SortingFix for Morph-Code-Improvement Don't Refresh item if not needed
void CClientListCtrl::UpdateAll()
{
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0:100));
}
//Xman end