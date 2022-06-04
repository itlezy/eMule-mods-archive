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
#include "UploadListCtrl.h"
#include "TransferWnd.h"
#include "otherfunctions.h"
#include "MenuCmds.h"
#include "ClientDetailDialog.h"
#include "emuledlg.h"
#include "MemDC.h"
#include "KnownFile.h"
#include "SharedFileList.h"
#include "UpDownClient.h"
#include "ClientCredits.h"
#include "kademlia/kademlia/Kademlia.h"
#include "kademlia/net/KademliaUDPListener.h"
#include "UploadQueue.h"
#include "ToolTipCtrlX.h"
#include "ThrottledSocket.h" //Xman Xtreme Upload
#include "UploadBandwidthThrottler.h" //Xman Xtreme Upload
//#include "EMsocket.h"
#include "ListenSocket.h" //Xman changed: display the obfuscation icon for all clients which enabled it
#include "PartFile.h" //Xman PowerRelease

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


static const UINT colStrID[]={
	 IDS_IP
	,IDS_FILE
	,IDS_DL_SPEED
	,IDS_DL_TRANSF
	,IDS_WAITED
	,IDS_UPLOADTIME
	,IDS_STATUS
	,IDS_FD_PARTS
	,IDS_CD_CSOFT
	,IDS_UPDOWNUPLOADLIST
	,IDS_CL_DOWNLSTATUS//Download-Status
	,IDS_DL_TRANSFCOMPL
};	

IMPLEMENT_DYNAMIC(CUploadListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CUploadListCtrl, CMuleListCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnClick)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnLvnGetDispInfo)
	ON_NOTIFY_REFLECT(LVN_GETINFOTIP, OnLvnGetInfoTip)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNmDblClk)
	ON_WM_CONTEXTMENU()
	//ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

CUploadListCtrl::CUploadListCtrl()
	: CListCtrlItemWalk(this)
{
	m_tooltip = new CToolTipCtrlX;
}

CUploadListCtrl::~CUploadListCtrl()
{
	delete m_tooltip;
}

void CUploadListCtrl::SetGridLine()
{
 if (thePrefs.UseGridlines())
	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_GRIDLINES); 
   else
     SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
}

void CUploadListCtrl::Init()
{
	SetPrefsKey(_T("UploadListCtrl"));
    SetGridLine();
	CToolTipCtrl* tooltip = GetToolTips();
	if (tooltip) {
		m_tooltip->SubclassWindow(tooltip->m_hWnd);
		tooltip->ModifyStyle(0, TTS_NOPREFIX);
		tooltip->SetDelayTime(TTDT_AUTOPOP, 20000);
		tooltip->SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1000);
	}
	static const int colWidth[]={DFLT_CLIENTNAME_COL_WIDTH,
								DFLT_FILENAME_COL_WIDTH,
								DFLT_DATARATE_COL_WIDTH,
								80,
								60,
								80,
								100,
								DFLT_PARTSTATUS_COL_WIDTH,
								DFLT_CLIENTSOFT_COL_WIDTH,
								90,
                                100, //Download-Status
	                            DFLT_SIZE_COL_WIDTH}; 
	for(int icol = 0; icol < _countof(colWidth); ++icol)
		InsertColumn(icol,GetResString(colStrID[icol]),LVCFMT_LEFT,colWidth[icol]);

	SetAllIcons();
	//Localize();// X: [RUL] - [Remove Useless Localize]
	LoadSettings();

	//Xman client percentage
//	CFont* pFont = GetFont();
//	LOGFONT lfFont = {0};
//	pFont->GetLogFont(&lfFont);
//	lfFont.lfHeight = 11;
//	m_fontBoldSmaller.CreateFontIndirect(&lfFont);
	//Xman end

	SetSortArrow();
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0 : 100));
}

void CUploadListCtrl::Localize()
{
	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;
	for (int icol=0;icol<pHeaderCtrl->GetItemCount();++icol) {
		strRes=GetResString(colStrID[icol]);
		if(icol > 6 || icol < 8)
			strRes.Remove(_T(':'));
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(icol, &hdi);
	}
}
/*
void CUploadListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
}
*/
void CUploadListCtrl::SetAllIcons()
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

void CUploadListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CRect cur_rec(lpDrawItemStruct->rcItem);
	const CUpDownClient *client = (CUpDownClient *)lpDrawItemStruct->itemData;
	CMemoryDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	InitItemMemDC(dc, lpDrawItemStruct->rcItem, /*client->HasLowID() ? RGB(255,250,200) :*/ ((lpDrawItemStruct->itemID % 2)?m_crEvenLine:m_crWindow), lpDrawItemStruct->itemState); //Xman show LowIDs
	RECT rcClient;// X: [DDHC] - [Don't Draw Hidden Column]
	GetClientRect(&rcClient);

	//Xman Xtreme Upload 
	const ThrottledFileSocket* socket=(client->socket);
	if( socket!=NULL)
	{
		if (socket->IsFull())
			dc.SetTextColor(RGB(0,0,0));
		else if (socket->IsTrickle())
			dc.SetTextColor(::GetSysColor(COLOR_GRAYTEXT));
#ifdef _DEBUG
		//Xman this is used for testing purpose
		else if(socket->isready)
			dc.SetTextColor(RGB(0,0,255));
		else
			dc.SetTextColor(RGB(0,128,128));
#endif
	}
#ifdef _DEBUG
	//Xman this is used for testing purpose
	else
		dc.SetTextColor(RGB(255,0,0));
	//Xman end
#endif

	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	cur_rec.right = cur_rec.left - sm_iSubItemInset;
	cur_rec.left += sm_iSubItemInset;
	for (int iCurrent = 0; iCurrent < iCount; iCurrent++)
	{
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
		if(IsColumnHidden(iColumn)) continue;
		int iColumnWidth = CListCtrl::GetColumnWidth(iColumn);
		cur_rec.right += iColumnWidth;
		if(iColumnWidth > 2*sm_iSubItemInset && cur_rec.right>0){// X: [DDHC] - [Don't Draw Hidden Column]
		    TCHAR szItem[1024];
		    switch (iColumn)
			{
				case 0:{
					int iIconPosY = (cur_rec.Height() > 19) ? ((cur_rec.Height() - 16) / 2) : 1;
					POINT point = {cur_rec.left, cur_rec.top + iIconPosY};
		
				    if (client->GetUserName()){
					//EastShare Start - added by AndCycle, IP to Country, modified by Commander
					if(theApp.ip2country->ShowCountryFlag() ){
						theApp.ip2country->GetFlagImageList()->Draw(dc, client->GetCountryFlagIndex(), point, ILD_NORMAL);
						cur_rec.left += 21;
					}
					//EastShare End - added by AndCycle, IP to Country

                        //morph4u share visible +
                        if (client->GetViewSharedFilesSupport())
			               _sntprintf(szItem, _countof(szItem), _T("%s*"), client->GetUserIPString());
                        else
                        //morph4u share visible -
						// khaos::kmod+ Show IP
			               _sntprintf(szItem, _countof(szItem), _T("%s"), client->GetUserIPString());
		                // khaos::kmod- 
						//_tcsncpy(szItem, client->GetUserName(), _countof(szItem) - 1);
						szItem[_countof(szItem) - 1] = _T('\0');
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT);

					//EastShare Start - added by AndCycle, IP to Country
					if(theApp.ip2country->ShowCountryFlag() )
						cur_rec.left -= 21;
					//EastShare End - added by AndCycle, IP to Country
					   }
					break;
				}
				case 1:{
					const CKnownFile *file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
					if (file)
					{
						//Xman PowerRelease
						COLORREF crOldBackColor = dc.GetBkColor();
						if(file->GetUpPriority()==PR_POWER)
							dc.SetBkColor(RGB(255,225,225));
						_tcsncpy(szItem, file->GetFileName(), _countof(szItem) - 1);
						szItem[_countof(szItem) - 1] = _T('\0');
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT);
						dc.SetBkColor(crOldBackColor);
						//Xman end
					}
				}
					break;
                    // morph4u :: TotalUpDown :: Start
					case 9:{
						if(client->Credits())
						{
							const COLORREF crOldTxtColor = dc->GetTextColor();
							const uint64 uUploadedTotal = client->Credits()->GetUploadedTotal();
							const uint64 uDownloadedTotal = client->Credits()->GetDownloadedTotal();

							if (uUploadedTotal > 0 || uDownloadedTotal > 0)
							{
								if (uUploadedTotal > uDownloadedTotal)
									dc->SetTextColor(RGB(192,0,0));
								else
									dc->SetTextColor(RGB(0,192,0));
							}
                            _sntprintf(szItem, _countof(szItem), _T("%s/ %s"), CastItoXBytes(client->credits->GetUploadedTotal()), CastItoXBytes(client->credits->GetDownloadedTotal()));
                            szItem[_countof(szItem) - 1] = _T('\0');
							dc->DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT);
							dc->SetTextColor(crOldTxtColor);
						}
						break;
					}
					// morph4u :: TotalUpDown :: End
				case 7:
				{
					cur_rec.bottom--;
					cur_rec.top++;
					COLORREF crOldBackColor = dc.GetBkColor();
					client->DrawUpStatusBar(dc, &cur_rec, false, thePrefs.UseFlatBar());
					dc.SetBkColor(crOldBackColor); //Xman Code Improvement: FillSolidRect
					cur_rec.bottom++;
					cur_rec.top--;
					break;
				}
				default:
					GetItemDisplayText(client, iColumn, szItem, _countof(szItem));
					if(szItem[0] != 0)
						dc.DrawText(szItem, -1, &cur_rec, (iColumn== 2 || iColumn == 3 || iColumn == 11) ? MLC_DT_TEXT | DT_RIGHT : MLC_DT_TEXT);
					break;
			}
		}
		cur_rec.left += iColumnWidth;
		if(cur_rec.left>=rcClient.right)// X: [DDHC] - [Don't Draw Hidden Column]
			break;
	}
}

void CUploadListCtrl::GetItemDisplayText(const CUpDownClient *client, int iSubItem, LPTSTR pszText, int cchTextMax)
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
                //morph4u share visible +
                if (client->GetViewSharedFilesSupport())
			      _sntprintf(pszText, cchTextMax, _T("%s*"), client->GetUserIPString());
                else
                //morph4u share visible -
				// khaos::kmod+ Show IP
			      _sntprintf(pszText, cchTextMax, _T("%s"), client->GetUserIPString());
		        // khaos::kmod- 
				//_tcsncpy(pszText, client->GetUserName(), cchTextMax);
			}
			break;

		case 1: {
			const CKnownFile *file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
			if(file != NULL)
				_tcsncpy(pszText, file->GetFileName(), cchTextMax);
			break;
		}

		case 2:
			if(thePrefs.ShowBlockRatio())
				_sntprintf(pszText, cchTextMax, _T("%s, %0.0f%%"), CastItoXBytes(client->GetUploadDatarate(), false, true), client->socket->GetBlockRatio());
			else
				_tcsncpy(pszText, CastItoXBytes(client->GetUploadDatarate(), false, true), cchTextMax);
			break;

		case 3:
			// ==> Pay Back First
		 	if(client->IsPBFClient()) 
				_sntprintf(pszText, cchTextMax, _T("%s (+%s)"), CastItoXBytes(client->GetSessionUp(), false, false), CastItoXBytes(client->credits->GetDownloadedTotal()-client->credits->GetUploadedTotal()));
			else
            // <== Pay Back First
			    _tcsncpy(pszText, CastItoXBytes(client->GetSessionUp(), false, false), cchTextMax);
			break;

		case 4:
			if (client->HasLowID())
				_sntprintf(pszText, cchTextMax, _T("%s (%s)"), CastSecondsToHM(client->GetWaitTime() / 1000), GetResString(IDS_IDLOW));
			else
				_tcsncpy(pszText, CastSecondsToHM(client->GetWaitTime() / 1000), cchTextMax);
			break;

		case 5:
			_tcsncpy(pszText, CastSecondsToHM(client->GetUpStartTimeDelay() / 1000), cchTextMax);
			break;

		case 6:
				_tcsncpy(pszText, client->GetUploadStateDisplayString(), cchTextMax);
			break;

		case 7:
			//display part bar
			break;
		//Xman version see clientversion in every window
		case 8:
			_tcsncpy(pszText, client->DbgGetFullClientSoftVer(), cchTextMax); //Xman // Maella -Support for tag ET_MOD_VERSION 0x55
			break;
		//Xman end
			
		//Xman show complete up/down in uploadlist
		case 9:
			if(client->Credits() )
				_sntprintf(pszText, cchTextMax, _T("%s/ %s"), CastItoXBytes(client->credits->GetUploadedTotal()), CastItoXBytes(client->credits->GetDownloadedTotal()));
			break;
		//Xman end
		//Download-Status+
		case 10: //Yun.SF3 remote queue status
			{	
				int qr = client->GetRemoteQueueRank();
				if (client->GetDownloadDatarate() > 0){
					_tcsncpy(pszText, CastItoXBytes(client->GetDownloadDatarate(),false,true), cchTextMax);
				}
				else if (qr)
					_sntprintf(pszText, cchTextMax, _T("QR: %u"),qr);
				//Dia+ Show NoNeededParts
				else if (client->GetDownloadState()==DS_NONEEDEDPARTS)
					_tcsncpy(pszText, GetResString(IDS_NONEEDEDPARTS), cchTextMax);
				//Dia- Show NoNeededParts							
				else if(client->IsRemoteQueueFull())
					_tcsncpy(pszText, GetResString(IDS_QUEUEFULL), cchTextMax);
				else
					_tcsncpy(pszText, GetResString(IDS_UNKNOWN), cchTextMax);
				break;	
			}
//Download-Status-	
		case 11:
			if(client->GetHisCompletedPartsPercent_UP() >=0)
				_sntprintf(pszText, cchTextMax, _T("%i%%"), client->GetHisCompletedPartsPercent_UP());
			break;
	}
	pszText[cchTextMax - 1] = _T('\0');
}

void CUploadListCtrl::OnLvnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
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

void CUploadListCtrl::OnLvnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult)
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
			CString strInfo;
			strInfo.Format(GetResString(IDS_USERINFO), client->GetUserName());
			const CKnownFile* file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
			if (file)
			{
				strInfo += GetResString(IDS_SF_REQUESTED) + _T(' ') + file->GetFileName() + _T('\n');
				strInfo.AppendFormat(GetResString(IDS_FILESTATS_SESSION) + GetResString(IDS_FILESTATS_TOTAL),
					file->statistic.GetAccepts(), file->statistic.GetRequests(), CastItoXBytes(file->statistic.GetTransferred(), false, false),
					file->statistic.GetAllTimeAccepts(), file->statistic.GetAllTimeRequests(), CastItoXBytes(file->statistic.GetAllTimeTransferred(), false, false));
			}
			else
			{
				strInfo += GetResString(IDS_REQ_UNKNOWNFILE);
			}
			strInfo += TOOLTIP_AUTOFORMAT_SUFFIX_CH;
			_tcsncpy(pGetInfoTip->pszText, strInfo, pGetInfoTip->cchTextMax);
			pGetInfoTip->pszText[pGetInfoTip->cchTextMax-1] = _T('\0');
		}
	}
	*pResult = 0;
}

void CUploadListCtrl::OnLvnColumnClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLISTVIEW *pNMListView = (NMLISTVIEW *)pNMHDR;
	bool sortAscending;
	if (GetSortItem() != pNMListView->iSubItem)
	{
		switch (pNMListView->iSubItem)
		{
			/*case 2: // Datarate
			case 3: // Session Up
			case 4: // Wait Time
			case 7: // Part Count
				sortAscending = false;
				break;
			default:
				sortAscending = true;
				break;*/
			case 0:
			case 1:
			case 5:
			case 6:
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

int CUploadListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	const CUpDownClient *item1 = (CUpDownClient *)lParam1;
	const CUpDownClient *item2 = (CUpDownClient *)lParam2;
	LPARAM iColumn = (lParamSort >= 100) ? lParamSort - 100 : lParamSort;
	int iResult = 0;
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

		case 1: {
			const CKnownFile *file1 = theApp.sharedfiles->GetFileByID(item1->GetUploadFileID());
			const CKnownFile *file2 = theApp.sharedfiles->GetFileByID(item2->GetUploadFileID());
			if (file1 != NULL && file2 != NULL)
				iResult = CompareLocaleStringNoCase(file1->GetFileName(), file2->GetFileName());
			else if (file1 == NULL)
				iResult = 1;
			else
				iResult = -1;
			break;
		}

		// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
		case 2: 
			iResult = item1->GetUploadDatarate() - item2->GetUploadDatarate();
			break;
		// Maella end

		//Xman don't show too many values
		case 3:
			iResult = CompareUnsigned(item1->GetSessionUp(), item2->GetSessionUp());
			/*
			if (iResult == 0 && thePrefs.m_bExtControls) {
				iResult = CompareUnsigned(item1->m_nCurQueueSessionPayloadUp, item2->m_nCurQueueSessionPayloadUp);
			}*/
			break;
		//Xman end
		case 4:
			iResult = CompareUnsigned(item1->GetWaitTime(), item2->GetWaitTime());
			break;

		case 5:
			iResult = CompareUnsigned(item1->GetUpStartTimeDelay() ,item2->GetUpStartTimeDelay());
			break;

		case 6:
			iResult = item1->GetUploadState() - item2->GetUploadState();
			break;

		case 7:
			// ==> Sort progress bars by percentage [Fafner/Xman] - Stulle  
            /* 
			iResult = CompareUnsigned(item1->GetUpPartCount(), item2->GetUpPartCount());
			*/  
            if (item1->GetHisCompletedPartsPercent_UP() == item2->GetHisCompletedPartsPercent_UP())  
                iResult=0;  
            else  
                iResult=item1->GetHisCompletedPartsPercent_UP() > item2->GetHisCompletedPartsPercent_UP()?1:-1;  
            // <== Sort progress bars by percentage [Fafner/Xman] - Stulle  
			break;

//Xman version see clientversion in every window
		case 8:
			// Maella -Support for tag ET_MOD_VERSION 0x55-
			if(item1->GetClientSoft() == item2->GetClientSoft())
				if(item1->GetVersion() == item2->GetVersion() && item1->GetClientSoft() == SO_EMULE){
					iResult= item1->DbgGetFullClientSoftVer().CompareNoCase( item2->DbgGetFullClientSoftVer());
				}
				else {
					iResult = item1->GetVersion() - item2->GetVersion();
				}
			else
				iResult= item2->GetClientSoft() - item1->GetClientSoft();
			break;
		//Xman end

		//Xman show complete up/down in uploadlist
		case 9:
			if(item1->Credits() && item2->Credits())
			{
				iResult=CompareUnsigned64(item1->credits->GetUploadedTotal(), item2->credits->GetUploadedTotal());
			}
			else
				return 0;
			break;
		//Xman end
//Download-Status
        case 10:
			iResult=CompareUnsigned(item1->GetRemoteQueueRank(), item2->GetRemoteQueueRank());
			break;
//Download-Status
		case 11:
			// ==> Sort progress bars by percentage [Fafner/Xman] - Stulle  
            /* 
			iResult = CompareUnsigned(item1->GetUpPartCount(), item2->GetUpPartCount());
			*/  
            if (item1->GetHisCompletedPartsPercent_UP() == item2->GetHisCompletedPartsPercent_UP())  
                iResult=0;  
            else  
                iResult=item1->GetHisCompletedPartsPercent_UP() > item2->GetHisCompletedPartsPercent_UP()?1:-1;  
            // <== Sort progress bars by percentage [Fafner/Xman] - Stulle  
			break;
	}
	if (lParamSort>=100)
		return -iResult;
	// SLUGFILLER: multiSort remove - handled in parent class
	/*
	//call secondary sortorder, if this one results in equal
	int dwNextSort;
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->transferwnd->uploadlistctrl.GetNextSortOrder(lParamSort)) != (-1)){
		iResult= SortProc(lParam1, lParam2, dwNextSort);
	}
	*/
	return iResult;
}

void CUploadListCtrl::OnNmDblClk(NMHDR* /*pNMHDR*/, LRESULT* pResult)
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

void CUploadListCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
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

	if(client){
		CMenu ClientMenu;
		ClientMenu.CreatePopupMenu();
		ClientMenu.AppendMenu(MF_STRING, MP_DETAIL, GetResString(IDS_SHOWDETAILS));
		ClientMenu.SetDefaultItem(MP_DETAIL);
		ClientMenu.AppendMenu(MF_STRING | ((client->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED), MP_SHOWLIST, GetResString(IDS_VIEWFILES));
               if (thePrefs.IsExtControlsEnabled())
		    ClientMenu.AppendMenu(MF_STRING | ((client) ? MF_ENABLED : MF_GRAYED), MP_REMOVEUPLOAD, GetResString(IDS_REMOVE));
		if (Kademlia::CKademlia::IsRunning() && !Kademlia::CKademlia::IsConnected())
			ClientMenu.AppendMenu(MF_STRING | ((client->GetKadPort()!=0 && client->GetKadVersion() > 1) ? MF_ENABLED : MF_GRAYED), MP_BOOT, GetResString(IDS_BOOTSTRAP));
		
		GetPopupMenuPos(*this, point);
		ClientMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
	VERIFY( ClientMenu.DestroyMenu() );
}
}

BOOL CUploadListCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	wParam = LOWORD(wParam);

	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1){
		CUpDownClient* client = (CUpDownClient*)GetItemData(iSel);

		// Tux: Fix: context menu fix by CB [start]
		// It may happen that the client is removed from the queue when the a selection has been done
		if (!client)
			return true;
		// Tux: Fix: context menu fix by CB [end]

		switch (wParam){
			case MP_SHOWLIST:
				client->RequestSharedFileList();
				break;
                        case MP_REMOVEUPLOAD:{
				theApp.uploadqueue->RemoveFromUploadQueue(client);
				break;
	               	}
			//Xman end
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
		}
	}
	return true;
}

void CUploadListCtrl::AddClient(const CUpDownClient *client)
{
	if (!CemuleDlg::IsRunning())
		return;

	size_t iItemCount = GetItemCount();
	/*int iItem = */InsertItem(LVIF_TEXT | LVIF_PARAM, iItemCount, LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)client);
	//Update(iItem);
	theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Uploading, iItemCount + 1);
}

void CUploadListCtrl::RemoveClient(const CUpDownClient *client)
{
	if (!CemuleDlg::IsRunning())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	int result = FindItem(&find);
	if (result != -1) {
		DeleteItem(result);
		theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Uploading);
	}
}

void CUploadListCtrl::RefreshClient(const CUpDownClient *client)
{
	if (!CemuleDlg::IsRunning())
		return;

	if (theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd || !/*theApp.emuledlg->transferwnd->uploadlistctrl.*/IsWindowVisible())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	int result = FindItem(&find);
	if (result != -1)
		Update(result);
}

void CUploadListCtrl::ShowSelectedUserDetails()
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
