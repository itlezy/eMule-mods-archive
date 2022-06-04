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
#include "KademliaWnd.h"
#include "KadContactListCtrl.h"
#include "Ini2.h"
#include "OtherFunctions.h"
#include "emuledlg.h"
#include "MemDC.h"//from eMuleFuture :: IP2Country
#include "IP2Country.h" //from eMuleFuture :: IP2Country

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CONContactListCtrl

enum ECols
{
	colID = 0,
	colType,
	colDistance
};

IMPLEMENT_DYNAMIC(CKadContactListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CKadContactListCtrl, CMuleListCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnClick)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnLvnGetDispInfo) //from eMuleFuture :: IP2Country 
	ON_WM_DESTROY()
	ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

CKadContactListCtrl::CKadContactListCtrl()
{
	SetGeneralPurposeFind(true);
	SetSkinKey(L"KadContactsLv");
}

CKadContactListCtrl::~CKadContactListCtrl()
{
}

void CKadContactListCtrl::Init()
{
	SetPrefsKey(_T("ONContactListCtrl"));
	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	InsertColumn(colID,		  GetResString(IDS_ID),				LVCFMT_LEFT, 16 + DFLT_HASH_COL_WIDTH);
	InsertColumn(colType,	  GetResString(IDS_TYPE) ,			LVCFMT_LEFT,  50);
	InsertColumn(colDistance, GetResString(IDS_KADDISTANCE),	LVCFMT_LEFT, 600);

	SetAllIcons();
	Localize();

	LoadSettings();
	int iSortItem = GetSortItem();
	bool bSortAscending = GetSortAscending();

	SetSortArrow(iSortItem, bSortAscending);
	SortItems(SortProc, MAKELONG(iSortItem, (bSortAscending ? 0 : 0x0001)));
}

void CKadContactListCtrl::SaveAllSettings()
{
	SaveSettings();
}

void CKadContactListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
}

void CKadContactListCtrl::SetAllIcons()
{
	//from eMuleFuture :: IP2Country :: Start
	/*
	CImageList iml;
	iml.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);
	iml.Add(CTempIconLoader(_T("Contact0")));
	iml.Add(CTempIconLoader(_T("Contact1")));
	iml.Add(CTempIconLoader(_T("Contact2")));
	iml.Add(CTempIconLoader(_T("Contact3")));
	iml.Add(CTempIconLoader(_T("Contact4")));
	iml.Add(CTempIconLoader(_T("SrcUnknown"))); // replace
	*/
	ApplyImageList(NULL);
	m_ImageList.DeleteImageList();
	m_ImageList.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);
	m_ImageList.Add(CTempIconLoader(_T("Contact0")));
	m_ImageList.Add(CTempIconLoader(_T("Contact1")));
	m_ImageList.Add(CTempIconLoader(_T("Contact2")));
	m_ImageList.Add(CTempIconLoader(_T("Contact3")));
	m_ImageList.Add(CTempIconLoader(_T("Contact4")));
	m_ImageList.Add(CTempIconLoader(_T("SrcUnknown"))); // replace
	//from eMuleFuture :: IP2Country :: End

	//from eMuleFuture :: IP2Country :: Start
	/*
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	HIMAGELIST himl = ApplyImageList(iml.Detach());
	if (himl)
		ImageList_Destroy(himl);
	*/
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	VERIFY( ApplyImageList(m_ImageList) == NULL );
	//from eMuleFuture :: IP2Country :: End
}

void CKadContactListCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;

	for (int icol = 0; icol < pHeaderCtrl->GetItemCount(); icol++) 
	{
		switch (icol) 
		{
			case colID: strRes = GetResString(IDS_ID); break;
			case colType: strRes = GetResString(IDS_TYPE); break;
			case colDistance: strRes = GetResString(IDS_KADDISTANCE); break;
			default: strRes.Empty(); break;
		}
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(icol, &hdi);
	}
}

//from eMuleFuture :: IP2Country :: Start
/*
void CKadContactListCtrl::UpdateContact(int iItem, const Kademlia::CContact *contact)
{
	CString id;
	contact->GetClientID(&id);
	SetItemText(iItem, colID, id);

	id.Format(_T("%i(%u)"), contact->GetType(), contact->GetVersion());
	SetItemText(iItem, colType, id);

	contact->GetDistance(&id);
	SetItemText(iItem, colDistance, id);

	UINT nImageShown = contact->GetType() > 4 ? 4 : contact->GetType();
	if (nImageShown < 3 && !contact->IsIpVerified())
		nImageShown = 5; // if we have an active contact, which is however not IP verified (and therefore not used), show this icon instead
	SetItem(iItem, 0, LVIF_IMAGE, 0, nImageShown, 0, 0, 0, 0);
}
*/
//from eMuleFuture :: IP2Country :: End

void CKadContactListCtrl::UpdateKadContactCount()
{
	theApp.emuledlg->kademliawnd->UpdateContactCount();
}

bool CKadContactListCtrl::ContactAdd(const Kademlia::CContact *contact)
{
	bool bResult = false;
	try
	{
		ASSERT( contact != NULL );
		int iItem = InsertItem(LVIF_TEXT | LVIF_PARAM, GetItemCount(), NULL, 0, 0, 0, (LPARAM)contact);
		if (iItem >= 0)
		{
			bResult = true;
	//		Trying to update all the columns causes one of the connection freezes in win98
	//		ContactRef(contact);
			// If it still doesn't work under Win98, uncomment the '!afxData.bWin95' term
			// ==> Drop Win95 support [MorphXT] - Stulle
			/*
			if (!afxIsWin95() && iItem >= 0)
			*/
			//if (iItem >= 0)
			// <== Drop Win95 support [MorphXT] - Stulle
				//UpdateContact(iItem, contact);
			UpdateKadContactCount();
		}
	}
	catch(...){ASSERT(0);}
	return bResult;
}

void CKadContactListCtrl::ContactRem(const Kademlia::CContact *contact)
{
	try
	{
		ASSERT( contact != NULL );
		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)contact;
		int iItem = FindItem(&find);
		if (iItem != -1)
		{
			DeleteItem(iItem);
			UpdateKadContactCount();
		}
	}
	catch(...){ASSERT(0);}
}

void CKadContactListCtrl::ContactRef(const Kademlia::CContact *contact)
{
	try
	{
		ASSERT( contact != NULL );
		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)contact;
		int iItem = FindItem(&find);
		if (iItem != -1)
			//from eMuleFuture :: IP2Country :: Start
			/*
			UpdateContact(iItem, contact);
			*/
			Update(iItem);
			//from eMuleFuture :: IP2Country :: Start
	}
	catch(...){ASSERT(0);}
}

BOOL CKadContactListCtrl::OnCommand(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	// ???
	return TRUE;
}

void CKadContactListCtrl::OnLvnColumnClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLISTVIEW *pNMListView = (NMLISTVIEW *)pNMHDR;

	// Determine ascending based on whether already sorted on this column
	int iSortItem = GetSortItem();
	bool bOldSortAscending = GetSortAscending();
	bool bSortAscending = (iSortItem != pNMListView->iSubItem) ? true : !bOldSortAscending;

	// Item is column clicked
	iSortItem = pNMListView->iSubItem;

	// Sort table
	UpdateSortHistory(MAKELONG(iSortItem, (bSortAscending ? 0 : 0x0001)));
	SetSortArrow(iSortItem, bSortAscending);
	SortItems(SortProc, MAKELONG(iSortItem, (bSortAscending ? 0 : 0x0001)));

	*pResult = 0;
}

int CKadContactListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	const Kademlia::CContact *item1 = (Kademlia::CContact *)lParam1;
	const Kademlia::CContact *item2 = (Kademlia::CContact *)lParam2; 
	if (item1 == NULL || item2 == NULL)
		return 0;

	int iResult;
	switch (LOWORD(lParamSort))
	{
		case colID: {
			Kademlia::CUInt128 i1;
			Kademlia::CUInt128 i2;
			item1->GetClientID(&i1);
			item2->GetClientID(&i2);
			iResult = i1.CompareTo(i2);
			break;
		}

		case colType:
			iResult = item1->GetType() - item2->GetType();
			if (iResult == 0)
				iResult = item1->GetVersion() - item2->GetVersion();
			break;

		case colDistance: {
			Kademlia::CUInt128 distance1, distance2;
			item1->GetDistance(&distance1);
			item2->GetDistance(&distance2);
			iResult = distance1.CompareTo(distance2);
			break;
		}

		default:
			return 0;
	}
	if (HIWORD(lParamSort))
		iResult = -iResult;
	return iResult;
}

//from eMuleFuture :: IP2Country :: Start
void CKadContactListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	if (!lpDrawItemStruct->itemData)
		return;

	CMemDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	BOOL bCtrlFocused;
	InitItemMemDC(dc, lpDrawItemStruct, bCtrlFocused);
	CRect cur_rec(lpDrawItemStruct->rcItem);
	CRect rcClient;
	GetClientRect(&rcClient);

	Kademlia::CContact* contact = (Kademlia::CContact*)lpDrawItemStruct->itemData;

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
				GetItemDisplayText(contact, iColumn, szItem, _countof(szItem));
				switch (iColumn)
				{
					case colID:
					{
						int iIconPosY = (cur_rec.Height() > 16) ? ((cur_rec.Height() - 16) / 2) : 1;
						POINT point = {cur_rec.left, cur_rec.top + iIconPosY};
						uint32 nImageShown = contact->GetType() > 4 ? 4 : contact->GetType();
						if (nImageShown < 3 && !contact->IsIpVerified())
							nImageShown = 5; // if we have an active contact, which is however not IP verified (and therefore not used), show this icon instead
						m_ImageList.Draw(dc, nImageShown, point, ILD_NORMAL); 

						cur_rec.left += 16 + sm_iLabelOffset;

						if(theApp.ip2country->ShowCountryFlag() && thePrefs.m_bShowCountryFlagInKad){
							POINT point2 = {cur_rec.left,cur_rec.top + iIconPosY + 1};
							Country_Struct* tmpCountryStruct = theApp.ip2country->GetCountryFromIP(contact->GetIPAddress());
							theApp.ip2country->GetFlagImageList()->Draw(dc, tmpCountryStruct->FlagIndex, point2, ILD_NORMAL);
							cur_rec.left += 18 + sm_iLabelOffset;
						}

						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT);
						cur_rec.left -= 16;
						cur_rec.right -= sm_iSubItemInset;

						if(theApp.ip2country->ShowCountryFlag() && thePrefs.m_bShowCountryFlagInKad) {
							cur_rec.left -= 18 + sm_iLabelOffset;
						}
						break;
					}
				default:
					if(szItem[0] != 0)
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT);
				}
			}
			cur_rec.left += iColumnWidth;
		}
	}

	DrawFocusRect(dc, lpDrawItemStruct->rcItem, lpDrawItemStruct->itemState & ODS_FOCUS, bCtrlFocused, lpDrawItemStruct->itemState & ODS_SELECTED);
}

void CKadContactListCtrl::GetItemDisplayText(const Kademlia::CContact* contact, int iSubItem, LPTSTR pszText, int cchTextMax) const
{
	if (pszText == NULL || cchTextMax <= 0) {
		ASSERT(0);
		return;
	}
	pszText[0] = _T('\0');
	CString sBuffer;
	switch(iSubItem){
		case colID:
			contact->GetClientID(&sBuffer);
			break;
		case colType:
			sBuffer.Format(_T("%i(%u)"),contact->GetType(), contact->GetVersion());
			break;
		case colDistance:
			contact->GetDistance(&sBuffer);
			break;
	}
	_tcsncpy(pszText, sBuffer, cchTextMax);
	pszText[cchTextMax - 1] = _T('\0');
}

void CKadContactListCtrl::OnLvnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
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
			const Kademlia::CContact* pContact = reinterpret_cast<Kademlia::CContact*>(pDispInfo->item.lParam);
			if (pContact != NULL)
				GetItemDisplayText(pContact, pDispInfo->item.iSubItem, pDispInfo->item.pszText, pDispInfo->item.cchTextMax);
		}
	}
	*pResult = 0;
}
//from eMuleFuture :: IP2Country :: End
