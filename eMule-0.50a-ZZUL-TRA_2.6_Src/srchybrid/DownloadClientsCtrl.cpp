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
#include "emuledlg.h"
#include "DownloadClientsCtrl.h"
#include "ClientDetailDialog.h"
#include "MemDC.h"
#include "MenuCmds.h"
#include "FriendList.h"
#include "TransferDlg.h"
#include "ChatWnd.h"
#include "UpDownClient.h"
#include "UploadQueue.h"
#include "ClientCredits.h"
#include "PartFile.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "SharedFileList.h"
#include "Addons/IP2Country/IP2Country.h" // ZZUL-TRA :: IP2Country

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const UINT colStrID[]={
	 IDS_QL_USERNAME
	,IDS_CD_CSOFT
	,IDS_FILE
	,IDS_DL_SPEED
	,IDS_AVAILABLEPARTS
	,IDS_CL_TRANSFDOWN
	,IDS_CL_TRANSFUP
	,IDS_META_SRCTYPE
    ,IDS_COUNTRY // ZZUL-TRA :: IP2Country
};

IMPLEMENT_DYNAMIC(CDownloadClientsCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CDownloadClientsCtrl, CMuleListCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnClick)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnLvnGetDispInfo)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNmDblClk)
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

CDownloadClientsCtrl::CDownloadClientsCtrl()
	: CListCtrlItemWalk(this)
{
	SetGeneralPurposeFind(true);
	SetSkinKey(L"DownloadingLv");
}

void CDownloadClientsCtrl::Init()
{
	SetPrefsKey(_T("DownloadClientsCtrl"));
	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	static const int colWidth[]={DFLT_CLIENTNAME_COL_WIDTH,
								DFLT_CLIENTSOFT_COL_WIDTH,
								DFLT_FILENAME_COL_WIDTH,
								DFLT_DATARATE_COL_WIDTH,
								DFLT_PARTSTATUS_COL_WIDTH,
								DFLT_SIZE_COL_WIDTH,
								DFLT_SIZE_COL_WIDTH,
								100,
                                100}; // ZZUL-TRA :: IP2Country
	for(int icol = 0; icol < _countof(colWidth); ++icol)
		InsertColumn(icol,GetResString(colStrID[icol]),LVCFMT_LEFT,colWidth[icol]);
   
	SetAllIcons();
	Localize();
	LoadSettings();

	//ZZUL-TRA :: ClientPercentage :: Start
	CFont* pFont = GetFont();
	LOGFONT lfFont = {0};
	pFont->GetLogFont(&lfFont);
	lfFont.lfHeight = 11;
	m_fontBoldSmaller.CreateFontIndirect(&lfFont);
	//ZZUL-TRA :: ClientPercentage :: End

	SetSortArrow();
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0 : 100));
}

void CDownloadClientsCtrl::Localize()
{
	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;

	CString strRes;
	for (int icol=0;icol<pHeaderCtrl->GetItemCount();++icol) {
		strRes=GetResString(colStrID[icol]);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(icol, &hdi);
	}
}

void CDownloadClientsCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
}

void CDownloadClientsCtrl::SetAllIcons()
{
	ApplyImageList(NULL);
	m_ImageList.DeleteImageList();
	m_ImageList.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkey")));
	m_ImageList.Add(CTempIconLoader(_T("ClientCompatible")));
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyPlus")));
	m_ImageList.Add(CTempIconLoader(_T("ClientCompatiblePlus")));
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

void CDownloadClientsCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
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
						if (client->credits != NULL)
						{					
// ZZUL-TRA :: SameModVersionDetection :: Start
                        if (client->IsZZULTRA())
					       iImage = 5;
                        else
// ZZUL-TRA :: SameModVersionDetection :: End
#ifdef CLIENTANALYZER                                      
					    if (client->IsBadGuy())
					         iImage = 6;
                        else
#endif  			
					    if (client->IsFriend())
								iImage = 4;
						else if (client->GetClientSoft() == SO_EMULE) {	
								if (client->credits->GetScoreRatio(client->GetIP()) > 1)
								iImage = 2;
								else
								iImage = 0;
							}
							else {
								if (client->credits->GetScoreRatio(client->GetIP()) > 1)
									iImage = 3;
								else
									iImage = 1;
							}
						}
						else
							iImage = 0;

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
						if(theApp.ip2country->ShowCountryFlag()){
                        cur_rec.left -=18;
						cur_rec.left -= sm_iLabelOffset;
						}        		 
						// ZZUL-TRA :: IP2Country :: End
						break;
					}

					case 4:
						cur_rec.bottom--;
						cur_rec.top++;
						client->DrawStatusBar(dc, &cur_rec, false, thePrefs.UseFlatBar());
						//ZZUL-TRA :: ClientPercentage :: Start
						if (thePrefs.GetUseDwlPercentage())
						{
							if(client->GetHisCompletedPartsPercent_Down() >=0)
							{
								CString buffer;
								COLORREF oldclr = dc.SetTextColor(RGB(0,0,0));
								int iOMode = dc.SetBkMode(TRANSPARENT);
								buffer.Format(_T("%i%%"), client->GetHisCompletedPartsPercent_Down());
								CFont *pOldFont = dc.SelectObject(&m_fontBoldSmaller);
#define DLC_DT_TEXT (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS)
#define	DrawClientPercentText	dc.DrawText(buffer, buffer.GetLength(),&cur_rec, ((DLC_DT_TEXT | DT_RIGHT) & ~DT_LEFT) | DT_CENTER)
								cur_rec.top-=1;cur_rec.bottom-=1;
								DrawClientPercentText;cur_rec.left+=1;cur_rec.right+=1;
								DrawClientPercentText;cur_rec.left+=1;cur_rec.right+=1;
								DrawClientPercentText;cur_rec.top+=1;cur_rec.bottom+=1;
								DrawClientPercentText;cur_rec.top+=1;cur_rec.bottom+=1;
								DrawClientPercentText;cur_rec.left-=1;cur_rec.right-=1;
								DrawClientPercentText;cur_rec.left-=1;cur_rec.right-=1;
								DrawClientPercentText;cur_rec.top-=1;cur_rec.bottom-=1;
								DrawClientPercentText;cur_rec.left++;cur_rec.right++;
								dc.SetTextColor(RGB(255,255,255));
								DrawClientPercentText;
								dc.SelectObject(pOldFont);
								dc.SetBkMode(iOMode);
								dc.SetTextColor(oldclr);
							}
						}
						//ZZUL-TRA :: ClientPercentage :: End
						cur_rec.bottom++;
						cur_rec.top--;
						break;

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

void CDownloadClientsCtrl::GetItemDisplayText(const CUpDownClient *client, int iSubItem, LPTSTR pszText, int cchTextMax)
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
			_tcsncpy(pszText, client->DbgGetFullClientSoftVer(), cchTextMax); //>>> WiZaRd::Show ModVer
			break;

		case 2:
			_tcsncpy(pszText, client->GetRequestFile()->GetFileName(), cchTextMax);
			break;
		
		case 3:
			_tcsncpy(pszText, CastItoXBytes((float)client->GetDownloadDatarate(), false, true), cchTextMax);
			break;
		
		case 4:
			_tcsncpy(pszText, GetResString(IDS_AVAILABLEPARTS), cchTextMax);
			break;

		case 5:
			if (client->credits && client->GetSessionDown() < client->credits->GetDownloadedTotal())
				_sntprintf(pszText, cchTextMax, _T("%s (%s)"), CastItoXBytes(client->GetSessionDown()), CastItoXBytes(client->credits->GetDownloadedTotal()));
			else
				_tcsncpy(pszText, CastItoXBytes(client->GetSessionDown()), cchTextMax);
			break;
		
		case 6:
			if (client->credits && client->GetSessionUp() < client->credits->GetUploadedTotal())
				_sntprintf(pszText, cchTextMax, _T("%s (%s)"), CastItoXBytes(client->GetSessionUp()), CastItoXBytes(client->credits->GetUploadedTotal()));
			else
				_tcsncpy(pszText, CastItoXBytes(client->GetSessionUp()), cchTextMax);
			break;
		
		case 7:
			switch (client->GetSourceFrom())
			{
				case SF_SERVER:
					_tcsncpy(pszText, _T("eD2K Server"), cchTextMax);
					break;
				case SF_KADEMLIA:
					_tcsncpy(pszText, GetResString(IDS_KADEMLIA), cchTextMax);
					break;
				case SF_SOURCE_EXCHANGE:
					_tcsncpy(pszText, GetResString(IDS_SE), cchTextMax);
					break;
				case SF_PASSIVE:
					_tcsncpy(pszText, GetResString(IDS_PASSIVE), cchTextMax);
					break;
				case SF_LINK:
					_tcsncpy(pszText, GetResString(IDS_SW_LINK), cchTextMax);
					break;
							// ZZUL-TRA :: Sourcecache :: Start
				case SF_CACHE:
					_tcsncpy(pszText, GetResString(IDS_SOURCECACHE), cchTextMax);
					break;
							// ZZUL-TRA :: Sourcecache :: End
                           // ZZUL-TRA :: SLS :: Start
				case SF_SLS:
					_tcsncpy(pszText, GetResString(IDS_SLS), cchTextMax);
					break;
			    // ZZUL-TRA :: SLS :: End
				default:
					_tcsncpy(pszText, GetResString(IDS_UNKNOWN), cchTextMax);
					break;
			}
			break;
	    // ZZUL-TRA :: IP2Country :: Start
		case 8:
			_tcsncpy(pszText, client->GetCountryName(), cchTextMax);
			break;
		// ZZUL-TRA :: IP2Country :: End

	}
	pszText[cchTextMax - 1] = _T('\0');
}

void CDownloadClientsCtrl::OnLvnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
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

void CDownloadClientsCtrl::OnLvnColumnClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLISTVIEW *pNMListView = (NMLISTVIEW *)pNMHDR;
	bool sortAscending;
	if (GetSortItem() != pNMListView->iSubItem)
	{
		switch (pNMListView->iSubItem)
		{
			case 1: // Client Software
			case 3: // Download Rate
			case 4: // Part Count
			case 5: // Session Down
			case 6: // Session Up
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

int CDownloadClientsCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
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
//>>> WiZaRd::Show ModVer
			//Proper sorting ;)
			iResult = item1->GetClientSoft() - item2->GetClientSoft();
			if(iResult == 0)
				iResult = CompareLocaleStringNoCase(item1->DbgGetFullClientSoftVer(), item2->DbgGetFullClientSoftVer());
//<<< WiZaRd::Show ModVer	
			break;

		case 2: {
			const CKnownFile *file1 = item1->GetRequestFile();
			const CKnownFile *file2 = item2->GetRequestFile();
			if( (file1 != NULL) && (file2 != NULL))
				iResult = CompareLocaleStringNoCase(file1->GetFileName(), file2->GetFileName());
			else if (file1 == NULL)
				iResult = 1;
			else
				iResult = -1;
			break;
		}

		case 3:
			iResult = CompareUnsigned(item1->GetDownloadDatarate(), item2->GetDownloadDatarate());
			break;

		case 4:
			iResult = CompareUnsigned(item1->GetPartCount(), item2->GetPartCount());
			break;

		case 5:
			iResult = CompareUnsigned(item1->GetSessionDown(), item2->GetSessionDown());
			break;

		case 6:
			iResult = CompareUnsigned(item1->GetSessionUp(), item2->GetSessionUp());
			break;

		case 7:
			iResult = item1->GetSourceFrom() - item2->GetSourceFrom();
			break;
	    // ZZUL-TRA :: IP2Country :: Start
		case 8:
			iResult=CompareLocaleStringNoCase(item1->GetCountryName(true), item2->GetCountryName(true));
			break;
	    // ZZUL-TRA :: IP2Country :: End
	}

	if (lParamSort >= 100)
		iResult = -iResult;

	//call secondary sortorder, if this one results in equal
	int dwNextSort;
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->transferwnd->GetDownloadClientsList()->GetNextSortOrder(lParamSort)) != -1)
		iResult = SortProc(lParam1, lParam2, dwNextSort);

	return iResult;
}

void CDownloadClientsCtrl::OnNmDblClk(NMHDR* /*pNMHDR*/, LRESULT* pResult)
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

void CDownloadClientsCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
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
	GetPopupMenuPos(*this, point);
	ClientMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
}

BOOL CDownloadClientsCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
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
			// </CB Mod : Friend Management : Easy FriendSlot>
			
		}
	}
	return true;
}

void CDownloadClientsCtrl::AddClient(const CUpDownClient *client)
{
	if (!theApp.emuledlg->IsRunning())
		return;

	int iItemCount = GetItemCount();
	InsertItem(LVIF_TEXT | LVIF_PARAM, iItemCount, LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)client);
	theApp.emuledlg->transferwnd->UpdateListCount(CTransferDlg::wnd2Downloading, iItemCount + 1);
}

void CDownloadClientsCtrl::RemoveClient(const CUpDownClient *client)
{
	if (!theApp.emuledlg->IsRunning())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	int result = FindItem(&find);
	if (result != -1) {
		DeleteItem(result);
		theApp.emuledlg->transferwnd->UpdateListCount(CTransferDlg::wnd2Downloading, GetItemCount()); 
	}
}

void CDownloadClientsCtrl::RefreshClient(const CUpDownClient *client)
{
	if (!theApp.emuledlg->IsRunning())
		return;

	if (theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd || !theApp.emuledlg->transferwnd->GetDownloadClientsList()->IsWindowVisible())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	int result = FindItem(&find);
	if (result != -1)
		Update(result);
}

void CDownloadClientsCtrl::ShowSelectedUserDetails()
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
