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
#include "TransferWnd.h"
#include "UpDownClient.h"
//#include "UploadQueue.h" not needed
#include "ClientCredits.h"
#include "PartFile.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "SharedFileList.h"
#include "ListenSocket.h" //Xman changed: display the obfuscation icon for all clients which enabled it
#include "ToolTipCtrlX.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DLC_BARUPDATE 1024

static const UINT colStrID[]={
	 IDS_IP
	,IDS_META_SRCTYPE
	,IDS_CL_TRANSFDOWN
	,IDS_DL_SPEED
	,IDS_SEARCHAVAIL
	,IDS_FD_PARTS
	,IDS_CD_CSOFT
	,IDS_CL_DOWNLSTATUS
	,IDS_CL_TRANSFUP
	,IDS_UL_SPEED
};

IMPLEMENT_DYNAMIC(CDownloadClientsCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CDownloadClientsCtrl, CMuleListCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnClick)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnLvnGetDispInfo)
	ON_NOTIFY_REFLECT(LVN_GETINFOTIP, OnLvnGetInfoTip)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNmDblClk)
	ON_WM_CONTEXTMENU()
	//ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

CDownloadClientsCtrl::CDownloadClientsCtrl()
	: CListCtrlItemWalk(this)
	, curPartfile(NULL)
{
	m_tooltip = new CToolTipCtrlX;
}

void CDownloadClientsCtrl::SetGridLine()
{
 if (thePrefs.UseGridlines())
	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_GRIDLINES); 
   else
     SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
}

void CDownloadClientsCtrl::Init()
{
	SetPrefsKey(_T("DownloadClientsCtrl"));
	SetGridLine();

CToolTipCtrl* tooltip = GetToolTips();
	if (tooltip){
		m_tooltip->SubclassWindow(*tooltip);
		tooltip->ModifyStyle(0, TTS_NOPREFIX);
		tooltip->SetDelayTime(TTDT_AUTOPOP, 20000);
		tooltip->SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1000);
	}

	static const int colWidth[]={DFLT_CLIENTNAME_COL_WIDTH,
								100,
								DFLT_CLIENTSOFT_COL_WIDTH,
								DFLT_DATARATE_COL_WIDTH,
								DFLT_DATARATE_COL_WIDTH,
								DFLT_PARTSTATUS_COL_WIDTH,
								DFLT_CLIENTSOFT_COL_WIDTH,
								115,
								DFLT_CLIENTSOFT_COL_WIDTH,
								DFLT_DATARATE_COL_WIDTH};
	for(int icol = 0; icol < _countof(colWidth); ++icol)
		InsertColumn(icol,GetResString(colStrID[icol]),LVCFMT_LEFT,colWidth[icol]);

	SetAllIcons();
	//Localize();// X: [RUL] - [Remove Useless Localize]
	LoadSettings();

	SetSortArrow();
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0 : 100));
}

CDownloadClientsCtrl::~CDownloadClientsCtrl()
{
	for(ClientList::const_iterator it=clientlist.begin();it!=clientlist.end();++it)
		delete it->second;
	clientlist.clear();
	delete m_tooltip;
}

void CDownloadClientsCtrl::Localize()
{
	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;
	for (int icol=0;icol<pHeaderCtrl->GetItemCount();++icol) {
		strRes=GetResString(colStrID[icol]);
		if(icol > 4 || icol < 7) 
			strRes.Remove(_T(':'));
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(icol, &hdi);
	}
}
/*
void CDownloadClientsCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
}
*/
void CDownloadClientsCtrl::SetAllIcons() 
{
	ApplyImageList(NULL);
	m_ImageList.DeleteImageList();
	m_ImageList.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);	
	//theApp.SetClientIcon(m_ImageList);
	// Apply the image list also to the listview control, even if we use our own 'DrawItem'.
	// This is needed to give the listview control a chance to initialize the row height.
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) != 0 );
	VERIFY( ApplyImageList(m_ImageList) == NULL );
}

void CDownloadClientsCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CRect cur_rec(lpDrawItemStruct->rcItem);
	CUpDownClient *client = (CUpDownClient *)lpDrawItemStruct->itemData;
	CMemoryDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	InitItemMemDC(dc, lpDrawItemStruct->rcItem, /*client->HasLowID() ? RGB(255,250,200) :*/ ((lpDrawItemStruct->itemID % 2)?m_crEvenLine:m_crWindow), lpDrawItemStruct->itemState); //Xman show LowIDs 

	RECT rcClient;// X: [DDHC] - [Don't Draw Hidden Column]
	GetClientRect(&rcClient);

	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	cur_rec.right = cur_rec.left - sm_iSubItemInset;
	cur_rec.left += sm_iSubItemInset;
	ClientList::const_iterator it = clientlist.find(client);
	ASSERT(it != clientlist.end());
	CtrlItem_Struct* cur_item = it->second;
	bool isAvailable = (client->GetRequestFile() == curPartfile);
	for(int iCurrent = 0; iCurrent < iCount; iCurrent++){
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
		if(IsColumnHidden(iColumn)) continue;
		int iColumnWidth = CListCtrl::GetColumnWidth(iColumn);
		cur_rec.right += iColumnWidth;
		if(iColumnWidth > 2*sm_iSubItemInset && cur_rec.right>0){// X: [DDHC] - [Don't Draw Hidden Column]
			TCHAR szItem[1024];
			GetItemDisplayText(client, iColumn, szItem, _countof(szItem));
			switch(iColumn){
				case 0:{
			        int iIconPosY = (cur_rec.Height() > 19) ? ((cur_rec.Height() - 16) / 2) : 1;
					POINT point = {cur_rec.left, cur_rec.top + iIconPosY};
					
                    //EastShare Start - added by AndCycle, IP to Country, modified by Commander
					if(theApp.ip2country->ShowCountryFlag() && client->GetUserName()){
						theApp.ip2country->GetFlagImageList()->Draw(dc, client->GetCountryFlagIndex(), point, ILD_NORMAL);
						cur_rec.left += 21;
					}
					//EastShare End - added by AndCycle, IP to Country

					dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT);

					//EastShare Start - added by AndCycle, IP to Country
					if(theApp.ip2country->ShowCountryFlag() && client->GetUserName())
						cur_rec.left -= 21;
					//EastShare End - added by AndCycle, IP to Country
				    }
	                             break;
					case 5:
					{
						cur_rec.bottom--;
						cur_rec.top++;
					COLORREF crOldBackColor = dc.GetBkColor();
					client->DrawStatusBar(dc, &cur_rec, false, thePrefs.UseFlatBar());
					dc.SetBkColor(crOldBackColor); //Xman Code Improvement: FillSolidRect
						cur_rec.bottom++;
						cur_rec.top--;
					break;
					}
				default:
					if(szItem[0] != 0)
						dc.DrawText(szItem, -1, &cur_rec, (iColumn== 1 || iColumn == 6) ? MLC_DT_TEXT : MLC_DT_TEXT | DT_RIGHT);
					break;
			}
		}
		cur_rec.left += iColumnWidth;
		if(cur_rec.left>=rcClient.right)// X: [DDHC] - [Don't Draw Hidden Column]
			break;
	}
}

void CDownloadClientsCtrl::GetItemDisplayText(CUpDownClient *client, int iSubItem, LPTSTR pszText, int cchTextMax)
{
	if (pszText == NULL || cchTextMax <= 0) {
		ASSERT(0);
		return;
	}
	pszText[0] = _T('\0');
	switch (iSubItem)
	{
		case 0:
			if (client->GetUserName()){
				//_tcsncpy(pszText, client->GetUserName(), cchTextMax);
                //morph4u share visible +
                if (client->GetViewSharedFilesSupport())
			       _sntprintf(pszText, cchTextMax, _T("%s*"), client->GetUserIPString());
                else
                //morph4u share visible -
		        // khaos::kmod+ Show IP
			       _sntprintf(pszText, cchTextMax, _T("%s"), client->GetUserIPString());
		        // khaos::kmod- 
			}
            break;

		case 1:
			{
				static const UINT StrID[]={IDS_SERVER,IDS_KADEMLIA,IDS_SE,IDS_PASSIVE,IDS_SLS};
				_tcsncpy(pszText, GetResString(StrID[client->GetSourceFrom()]), cchTextMax);
			}
			break;

		case 2:
			if (client->GetTransferredDown())
				_tcsncpy(pszText, CastItoXBytes(client->GetTransferredDown()), cchTextMax);
			break;
		
		case 3:
			if(client->GetDownloadDatarate())
				_tcsncpy(pszText, CastItoXBytes(client->GetDownloadDatarate(), false, true), cchTextMax);
			break;
		
		case 4:
			if(client->GetRequestFile() == curPartfile && client->GetHisCompletedPartsPercent_Down() >=0)
				_sntprintf(pszText, cchTextMax, _T("%i%%"), client->GetHisCompletedPartsPercent_Down());
			break;

		case 5:
			//display part bar
			break;

		case 6:
			_tcsncpy(pszText, client->DbgGetFullClientSoftVer(), cchTextMax);
			break;

		case 7:{
			CString Sbuffer;
			if (client->GetRequestFile() == curPartfile){
				Sbuffer = client->GetDownloadStateDisplayString();
				//Xman
				//Xman only intern
				//if(client->GetDownloadState()==DS_TOOMANYCONNS)
				//	Sbuffer.Format(_T("P:%u,M:%u,Q:%u, %s"), client->m_downloadpriority, theApp.downloadqueue->m_maxdownprio,client->GetRemoteQueueRank() ,GetResString(IDS_TOOMANYCONNS));
			    //Xman Xtreme Downloadmanager
				if(thePrefs.m_bExtControls && !(client->m_OtherRequests_list.IsEmpty() && client->m_OtherNoNeeded_list.IsEmpty())) { //Xman Xtreme Downloadmanager
					Sbuffer.AppendChar(_T('*'));
				}

			}
			else {
				Sbuffer.Format(_T("A4AF")); //= GetResString(IDS_ASKED4ANOTHERFILE);
			//Xman end

			// ZZ:DownloadManager -->
				if(thePrefs.m_bExtControls) {
					if(client->IsInNoNeededList(curPartfile)) {
						Sbuffer += _T(" (") + GetResString(IDS_NONEEDEDPARTS) + _T(')');
					} else if(client->GetDownloadState() == DS_DOWNLOADING) {
						Sbuffer += _T(" (") + GetResString(IDS_TRANSFERRING) + _T(')');
					} else if(client->IsSwapSuspended(curPartfile)) { //Xman 0.46b Bugfix
						Sbuffer += _T(" (") + GetResString(IDS_SOURCESWAPBLOCKED) + _T(')');
					}

					if (client && client->GetRequestFile() && client->GetRequestFile()->GetFileName()){
						Sbuffer.AppendFormat(_T(": \"%s\""),client->GetRequestFile()->GetFileName());
					}
				}
			}
			// ZZ:DownloadManager <--
			_tcsncpy(pszText, Sbuffer, cchTextMax);
		}
			break;

		case 8:
			if(client->GetTransferredUp())
				_tcsncpy(pszText, CastItoXBytes(client->GetTransferredUp()), cchTextMax);
			break;
		case 9:
			if(client->GetUploadDatarate())
				_tcsncpy(pszText, CastItoXBytes(client->GetUploadDatarate(), false, true), cchTextMax);
			break;
	}
	pszText[cchTextMax - 1] = _T('\0');
}

void CDownloadClientsCtrl::OnLvnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
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
			CUpDownClient* pClient = reinterpret_cast<CUpDownClient*>(pDispInfo->item.lParam);
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
			case 0:
			case 1:
			case 7:
				sortAscending = true;
				break;
			default:
				sortAscending = false;
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
	CPartFile* partfile = theApp.emuledlg->transferwnd->downloadclientsctrl.curPartfile;
	if(item1->GetRequestFile() != partfile && item2->GetRequestFile() == partfile)
		return 1;
	else if(item1->GetRequestFile() == partfile && item2->GetRequestFile() != partfile)
		return -1;
	else{
		LPARAM iColumn = (lParamSort >= 100) ? lParamSort - 100 : lParamSort;
		int iResult=0;
		switch (iColumn)
		{
		case 0:
				if (ntohl(item1->GetIP()) && ntohl(item2->GetIP()))
					//iResult = CompareLocaleStringNoCase(item1->GetUserName(), item2->GetUserName());
					// khaos::kmod+ Show IP
			        iResult = CompareUnsigned(ntohl(item1->GetIP()), ntohl(item2->GetIP()));
		             // khaos::kmod- Show IP
				else if (ntohl(item1->GetIP()) == NULL)
					iResult = 1; // place clients with no usernames at bottom
				else if (ntohl(item2->GetIP()) == NULL)
					iResult = -1; // place clients with no usernames at bottom
				break;

			case 1:
				iResult = CompareUnsigned(item1->GetSourceFrom(), item2->GetSourceFrom());
				break;
			case 2: 
				iResult = CompareUnsigned(item1->GetTransferredDown(), item2->GetTransferredDown());
				break;
			case 3:
				iResult = CompareUnsigned(item1->GetDownloadDatarate()/(2*1024), item2->GetDownloadDatarate()/(2*1024));
				break;
			case 4:
				if(item1->GetHisCompletedPartsPercent_Down() == item2->GetHisCompletedPartsPercent_Down()){
					if(item1->GetPartStatus() == NULL)
						return 0;
					ASSERT(item2->GetPartStatus() != NULL);
					iResult=CompareUnsigned(item1->GetAvailablePartCount(), item2->GetAvailablePartCount());
					if(iResult == 0){
						if(item1->GetPartStatus()[item1->GetPartCount()-1] == item2->GetPartStatus()[item2->GetPartCount()-1])
							return 0;
						if(item1->GetPartStatus()[item1->GetPartCount()-1])
							iResult=-1;
						else
							iResult=1;
					}
				}
				else
					iResult=(item1->GetHisCompletedPartsPercent_Down() > item2->GetHisCompletedPartsPercent_Down())?1:-1;
				break;
			case 5:
				if(item1->GetHisCompletedPartsPercent_Down() == item2->GetHisCompletedPartsPercent_Down()){
					if(item1->GetPartStatus() == NULL)
						return 0;
					ASSERT(item2->GetPartStatus() != NULL);
					iResult=CompareUnsigned(item1->GetAvailablePartCount(), item2->GetAvailablePartCount());
					if(iResult == 0){
						if(item1->GetPartStatus()[item1->GetPartCount()-1] == item2->GetPartStatus()[item2->GetPartCount()-1])
							return 0;
						if(item1->GetPartStatus()[item1->GetPartCount()-1])
							iResult=-1;
						else
							iResult=1;
					}
				}
				else
					iResult=(item1->GetHisCompletedPartsPercent_Down() > item2->GetHisCompletedPartsPercent_Down())?1:-1;
				break;
			case 6:
				if( item1->GetClientSoft()  == item2->GetClientSoft()  ){
					if(item2->GetVersion() == item1->GetVersion() && item1->GetClientSoft()  == SO_EMULE)
						iResult= item1->DbgGetFullClientSoftVer().CompareNoCase( item2->DbgGetFullClientSoftVer());
					else
						iResult= CompareUnsigned(item1->GetVersion(), item2->GetVersion());
				}
				else
					iResult = CompareUnsigned(item2->GetClientSoft(), item1->GetClientSoft());
				break;
			case 7:
			if (item1->GetDownloadState() == item2->GetDownloadState()){
				if(item1->GetDownloadState() == DS_ONQUEUE){
					if (item1->IsRemoteQueueFull() && item2->IsRemoteQueueFull())
						return 0;
					if (item1->IsRemoteQueueFull()||item1->GetRemoteQueueRank() == 0)
						iResult=1;
					else if (item2->IsRemoteQueueFull()||item2->GetRemoteQueueRank() == 0)
						iResult=-1;
					else
						iResult=CompareUnsigned(item1->GetRemoteQueueRank(), item2->GetRemoteQueueRank());
				}
				else
					return 0;
			}
			else
				iResult=item1->GetDownloadState() - item2->GetDownloadState();
			break;
			case 8: 
				iResult=CompareUnsigned(item1->GetTransferredUp(), item2->GetTransferredUp());
			break;
			case 9: 
				iResult=CompareUnsigned(item1->GetUploadDatarate(), item2->GetUploadDatarate());
			break;

		}
		if (lParamSort>=100)
			return -iResult;

		// SLUGFILLER: multiSort remove - handled in parent class
		/*
		int dwNextSort;
		//call secondary sortorder, if this one results in equal
		//(Note: yes I know this call is evil OO wise, but better than changing a lot more code, while we have only one instance anyway - might be fixed later)
		if (iResult == 0 && (dwNextSort = GetNextSortOrder(lParamSort)) != (-1)){
			iResult= SortProc(lParam1, lParam2, dwNextSort);
		}
		*/
		return iResult;
	}
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
	CUpDownClient* client = (iSel != -1) ? (CUpDownClient*)GetItemData(iSel) : NULL;

	if(client){
		CMenu ClientMenu;
		ClientMenu.CreatePopupMenu();
		ClientMenu.AppendMenu(MF_STRING, MP_DETAIL, GetResString(IDS_SHOWDETAILS));
		ClientMenu.SetDefaultItem(MP_DETAIL);

		//Xman Xtreme Downloadmanager
		if (client->GetDownloadState() == DS_DOWNLOADING)
			ClientMenu.AppendMenu(MF_STRING,MP_STOP_CLIENT,GetResString(IDS_STOP_CLIENT));
		//xman end

        if (thePrefs.IsExtControlsEnabled()){
		   ClientMenu.AppendMenu(MF_SEPARATOR); 
		   ClientMenu.AppendMenu(MF_STRING,MP_RE_ASK,GetResString(IDS_ASKING)); 
        }

		if (Kademlia::CKademlia::IsRunning() && !Kademlia::CKademlia::IsConnected())
			ClientMenu.AppendMenu(MF_STRING | ((client->GetKadPort()!=0 && client->GetKadVersion() > 1) ? MF_ENABLED : MF_GRAYED), MP_BOOT, GetResString(IDS_BOOTSTRAP));
		CMenu A4AFMenu;
		A4AFMenu.CreateMenu();
		if (thePrefs.m_bExtControls) {
			//Xman Xtreme Downloadmanager
			//if (content->type == UNAVAILABLE_SOURCE) {
			//A4AFMenu.AppendMenu(MF_STRING,MP_A4AF_CHECK_THIS_NOW,GetResString(IDS_A4AF_CHECK_THIS_NOW));
			if (client->GetRequestFile() != curPartfile)
				A4AFMenu.AppendMenu(MF_STRING,MP_SWAP_A4AF_TO_THIS,GetResString(IDS_SWAP_A4AF_TO_THIS)); // Added by sivka [Ambdribant]
			else if (!(client->m_OtherNoNeeded_list.IsEmpty() && client->m_OtherRequests_list.IsEmpty()))
				A4AFMenu.AppendMenu(MF_STRING,MP_SWAP_A4AF_TO_OTHER,GetResString(IDS_SWAP_A4AF_TO_OTHER)); // Added by sivka

			//}
			//Xman end
			if (A4AFMenu.GetMenuItemCount()>0)
				ClientMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)A4AFMenu.m_hMenu, GetResString(IDS_A4AF));
		}
		ClientMenu.AppendMenu(MF_SEPARATOR); 
		ClientMenu.AppendMenu(MF_STRING | ((client->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED), MP_SHOWLIST, GetResString(IDS_VIEWFILES));
		
		GetPopupMenuPos(*this, point);
		ClientMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);

		VERIFY( A4AFMenu.DestroyMenu() );
		VERIFY( ClientMenu.DestroyMenu() );
	}
	}

BOOL CDownloadClientsCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	wParam = LOWORD(wParam);

	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1){
		CUpDownClient* client = (CUpDownClient*)GetItemData(iSel);
		if(!client) return true;
		switch (wParam){

				case MP_RE_ASK:	 
					if (client){
                    client->UDPReaskForDownload();
					client->AskForDownload();
					}
                   break;

			case MP_SHOWLIST:
				client->RequestSharedFileList();
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
			//Xman Xtreme Downloadmanager
			case MP_STOP_CLIENT: 
				client->StopClient();
				break;		

			case MP_SWAP_A4AF_TO_THIS: { 
				if(curPartfile->GetStatus(false) == PS_READY || curPartfile->GetStatus(false) == PS_EMPTY)
				{
					if(!client->GetDownloadState() == DS_DOWNLOADING)
					{
						if(client->SwapToAnotherFile(true, true, false, curPartfile,true)){
							curPartfile->UpdateDisplayedInfo(true);
							theApp.emuledlg->transferwnd->partstatusctrl.Refresh(true);
					}
				}
                            }
                           break;
									   }
			case MP_SWAP_A4AF_TO_OTHER:
				if (/*(client != NULL) && */!(client->GetDownloadState() == DS_DOWNLOADING)){
					if(client->SwapToAnotherFile(true, true, false, NULL,true)){
						curPartfile->UpdateDisplayedInfo(true);
						theApp.emuledlg->transferwnd->partstatusctrl.Refresh(true);
					}
				}
				break;
			//Xman end				
		}
	}
	return true;
}

void CDownloadClientsCtrl::AddClient(CUpDownClient *client,const CPartFile* partfile)
{
	if(curPartfile && curPartfile == partfile){
		_AddClient(client);
		theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Downloading, GetItemCount()); 
	}
}

void CDownloadClientsCtrl::_AddClient(CUpDownClient *client)
{
	if (!CemuleDlg::IsRunning())
		return;

	clientlist.insert(ClientPair(client,new CtrlItem_Struct()));
	//Xman Code Improvement
	InsertItem(LVIF_TEXT|LVIF_PARAM, GetItemCount(), client->GetUserName(), 0, 0, 1, (LPARAM)client);
	//RefreshClient(client);
}

void CDownloadClientsCtrl::RemoveClient(CUpDownClient *client, bool force)
{
	if (!CemuleDlg::IsRunning() || curPartfile==NULL || (!force && client->GetRequestFile() != curPartfile))
		return;

	ClientList::iterator it = clientlist.find(client);
	if (it != clientlist.end()) {
		CtrlItem_Struct* del_item = it->second;
		clientlist.erase(it);
		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)client;
		int result = FindItem(&find);
		if (result != -1)
			DeleteItem(result);
		theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Downloading, GetItemCount()); 
		delete del_item;
	}
}

void CDownloadClientsCtrl::RefreshClient(const CUpDownClient *client,const CPartFile* partfile)
{
	if (!CemuleDlg::IsRunning())
		return;

	if (theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd || !IsWindowVisible())
		return;

	if(curPartfile && curPartfile == partfile){
		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)client;
		int result = FindItem(&find);
		if (result != -1)
			Update(result);
	}
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

void CDownloadClientsCtrl::RemoveAllClients()
{
	DeleteAllItems();
	for(ClientList::const_iterator it=clientlist.begin();it!=clientlist.end();++it)
		delete it->second;
	clientlist.clear();
}

void CDownloadClientsCtrl::Reload(CPartFile* partfile, bool show){
	if(show){
		if(curPartfile != partfile){
			SetRedraw(false);
			curPartfile = partfile;
			RemoveAllClients();
			if(partfile){
				for(POSITION pos=partfile->srclist.GetHeadPosition();pos!=NULL;)
					_AddClient(partfile->srclist.GetNext(pos));
				for(POSITION pos=partfile->A4AFsrclist.GetHeadPosition();pos!=NULL;)
					_AddClient(partfile->A4AFsrclist.GetNext(pos));
			}
			SetRedraw(true);
			theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Downloading, GetItemCount());
			theApp.emuledlg->transferwnd->partstatusctrl.Refresh();
		}
	}
	else if(curPartfile && curPartfile == partfile){
		SetRedraw(false);
		RemoveAllClients();
		SetRedraw(true);
		theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Downloading, GetItemCount());
		theApp.emuledlg->transferwnd->partstatusctrl.Refresh();
	}

}

void CDownloadClientsCtrl::OnLvnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMLVGETINFOTIP>(pNMHDR);
	if (pGetInfoTip->iSubItem == 0)
	{
		LVHITTESTINFO hti = {0};
		::GetCursorPos(&hti.pt);
		ScreenToClient(&hti.pt);
		if (SubItemHitTest(&hti) == -1 || hti.iItem != pGetInfoTip->iItem || hti.iSubItem != 0){
			// don't show the default label tip for the main item, if the mouse is not over the main item
			if ((pGetInfoTip->dwFlags & LVGIT_UNFOLDED) == 0 && pGetInfoTip->cchTextMax > 0 && pGetInfoTip->pszText[0] != _T('\0'))
				pGetInfoTip->pszText[0] = _T('\0');
			return;
		}

		const CUpDownClient* client = (CUpDownClient*)GetItemData(pGetInfoTip->iItem);
		if (client && pGetInfoTip->pszText && pGetInfoTip->cchTextMax > 0)
		{
			// build info text and display it
			in_addr server;
			server.S_un.S_addr = client->GetServerIP();

			//Xman Xtreme Downloadmanager
			CString askbuffer;
			uint32 uJitteredFileReaskTime=client->GetJitteredFileReaskTime();
			if(client->GetDownloadState()==DS_NONEEDEDPARTS)
				uJitteredFileReaskTime *=2;

			if(client->HasTooManyFailedUDP() || client->GetDownloadState()==DS_NONEEDEDPARTS || (client->HasLowID() && !(client->GetBuddyIP() && client->GetBuddyPort() && client->HasValidBuddyID())))
			{
				if ( (uJitteredFileReaskTime+client->GetLastAskedTime()) < ::GetTickCount() )
					askbuffer=_T(": 0\n");
				else if(client->GetDownloadState()==DS_NONEEDEDPARTS || uJitteredFileReaskTime+client->GetLastAskedTime() <= client->GetNextTCPAskedTime())
					askbuffer.Format(_T(": %s\n"),CastSecondsToHM((uJitteredFileReaskTime+client->GetLastAskedTime()-::GetTickCount())/1000));
				else
					askbuffer.Format(_T(": %s (%s)\n"), CastSecondsToHM((uJitteredFileReaskTime+client->GetLastAskedTime()-::GetTickCount())/1000), CastSecondsToHM((client->GetNextTCPAskedTime()-::GetTickCount())/1000));
			}
			else
			{
				if (client->GetNextTCPAskedTime()<=::GetTickCount() || (uJitteredFileReaskTime+client->GetLastAskedTime()) < ::GetTickCount() )
					askbuffer=_T(": 0\n");
				else
					askbuffer.Format(_T(": %s (%s)\n"), CastSecondsToHM((uJitteredFileReaskTime+client->GetLastAskedTime()-::GetTickCount())/1000), CastSecondsToHM((client->GetNextTCPAskedTime()-::GetTickCount())/1000));
			}
			//Xman end

			CString info;
			info.Format(GetResString(IDS_USERINFO)
						+ GetResString(IDS_SERVER) + _T(": %s:%u\n\n")
						+ GetResString(IDS_NEXT_REASK) + askbuffer, //Xman Xtreme Downloadmanager
						client->GetUserName() ? client->GetUserName() : (_T('(') + GetResString(IDS_UNKNOWN) + _T(')')),
						ipstr(server), client->GetServerPort()); //Xman Xtreme Downloadmanager
			info += _T('\n');
			info.AppendFormat(GetResString(IDS_SOURCEINFO), client->GetAskedCountDown(), client->GetAvailablePartCount());
			//Xman Xtreme Downloadmanager
			info.AppendFormat(_T("\nUDP reask possible: %s"), client->HasTooManyFailedUDP() || (client->HasLowID() && !(client->GetBuddyIP() && client->GetBuddyPort() && client->HasValidBuddyID())) ? _T("no") : _T("yes")); //Xman Xtreme-Downloadmanager 
			if(client->HasLowID())
			{
				info.Append((client->GetBuddyIP() && client->GetBuddyPort() && client->HasValidBuddyID())?_T("\nclient has buddy"):_T("\nclient has no buddy"));
				if (client->GetLowIDReaskPening())
					info.Append(_T(", reask pending"));
			}
			//Xman end

			info += _T('\n');

			if (client->GetRequestFile() == curPartfile)
				info += GetResString(IDS_CLIENTSOURCENAME) + (!client->GetClientFilename().IsEmpty() ? client->GetClientFilename() : _T("-"));
			else
			{
				// client asked twice
				info += GetResString(IDS_ASKEDFAF);
                if (client->GetRequestFile() && client->GetRequestFile()->GetFileName())
                    info.AppendFormat(_T(": %s"), client->GetRequestFile()->GetFileName());
			}

            if (thePrefs.IsExtControlsEnabled() && !client->m_OtherRequests_list.IsEmpty())
			{
				CSimpleArray<const CString*> apstrFileNames;
				POSITION pos = client->m_OtherRequests_list.GetHeadPosition();
				while (pos)
					apstrFileNames.Add(&client->m_OtherRequests_list.GetNext(pos)->GetFileName());
				Sort(apstrFileNames);
				if (client->GetRequestFile() == curPartfile)
					info += _T('\n');
				info += _T('\n');
				info += GetResString(IDS_A4AF_FILES);
				info += _T(':');
				for (int i = 0; i < apstrFileNames.GetSize(); i++)
				{
					const CString* pstrFileName = apstrFileNames[i];
					if (info.GetLength() + (i > 0 ? 2 : 0) + pstrFileName->GetLength() >= pGetInfoTip->cchTextMax) {
						static const TCHAR szEllipses[] = _T("\n:...");
						if (info.GetLength() + (int)ARRSIZE(szEllipses) - 1 < pGetInfoTip->cchTextMax)
							info += szEllipses;
						break;
					}
					if (i > 0)
						info += _T("\n:");
					info += *pstrFileName;
				}
			}

			info.AppendChar(TOOLTIP_AUTOFORMAT_SUFFIX_CH);
			_tcsncpy(pGetInfoTip->pszText, info, pGetInfoTip->cchTextMax);
			pGetInfoTip->pszText[pGetInfoTip->cchTextMax-1] = _T('\0');
		}
	}
	*pResult = 0;
}
