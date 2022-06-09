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
#include "QueueListCtrl.h"
#include "OtherFunctions.h"
#include "MenuCmds.h"
#include "ClientDetailDialog.h"
#include "Exceptions.h"
#include "KademliaWnd.h"
#include "emuledlg.h"
#include "FriendList.h"
#include "UploadQueue.h"
#include "UpDownClient.h"
#include "TransferDlg.h"
#include "MemDC.h"
#include "SharedFileList.h"
#include "ClientCredits.h"
#include "PartFile.h"
#include "ChatWnd.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "Kademlia/Kademlia/Prefs.h"
#include "kademlia/net/KademliaUDPListener.h"
#include "Log.h"
//Xman
#include "ListenSocket.h" 
#ifdef CLIENTANALYZER
#include "Addons/AntiLeech/ClientAnalyzer.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


static const UINT colStrID[]={
	 IDS_QL_USERNAME
	,IDS_FILE
	,IDS_FILEPRIO
	,IDS_QL_RATING
	,IDS_SCORE
	,IDS_ASKED
	,IDS_LASTSEEN
	,IDS_ENTERQUEUE
	//,IDS_BANNED
	,IDS_UPSTATUS
	,IDS_CD_CSOFT
	,IDS_UPDOWNUPLOADLIST
};

IMPLEMENT_DYNAMIC(CQueueListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CQueueListCtrl, CMuleListCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnClick)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnLvnGetDispInfo)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNmDblClk)
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

CQueueListCtrl::CQueueListCtrl()
	: CListCtrlItemWalk(this)
{
	SetGeneralPurposeFind(true);
	SetSkinKey(L"QueuedLv");

	// Barry - Refresh the queue every 10 secs
	VERIFY( (m_hTimer = ::SetTimer(NULL, NULL, 10000, QueueUpdateTimer)) != NULL );
	if (thePrefs.GetVerbose() && !m_hTimer)
		AddDebugLogLine(true,_T("Failed to create 'queue list control' timer - %s"),GetErrorMessage(GetLastError()));
}

CQueueListCtrl::~CQueueListCtrl()
{
	if (m_hTimer)
		VERIFY( ::KillTimer(NULL, m_hTimer) );
}

void CQueueListCtrl::Init()
{
	SetPrefsKey(_T("QueueListCtrl"));
	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
/*	InsertColumn(0, GetResString(IDS_QL_USERNAME),	LVCFMT_LEFT, DFLT_CLIENTNAME_COL_WIDTH);
	InsertColumn(1, GetResString(IDS_FILE),			LVCFMT_LEFT, DFLT_FILENAME_COL_WIDTH);
	InsertColumn(2, GetResString(IDS_FILEPRIO),		LVCFMT_LEFT, DFLT_PRIORITY_COL_WIDTH);
	InsertColumn(3, GetResString(IDS_QL_RATING),	LVCFMT_LEFT,  60);
	InsertColumn(4, GetResString(IDS_SCORE),		LVCFMT_LEFT,  60);
	InsertColumn(5, GetResString(IDS_ASKED),		LVCFMT_LEFT,  60);
	InsertColumn(6, GetResString(IDS_LASTSEEN),		LVCFMT_LEFT, 110);
	InsertColumn(7, GetResString(IDS_ENTERQUEUE),	LVCFMT_LEFT, 110);
	InsertColumn(8, GetResString(IDS_BANNED),		LVCFMT_LEFT,  60);
	InsertColumn(9, GetResString(IDS_UPSTATUS),		LVCFMT_LEFT, DFLT_PARTSTATUS_COL_WIDTH);
	InsertColumn(10,GetResString(IDS_CD_CSOFT), LVCFMT_LEFT, DFLT_CLIENTSOFT_COL_WIDTH);	//Xman version see clientversion in every window
	InsertColumn(11, GetResString(IDS_UPDOWNUPLOADLIST), LVCFMT_LEFT, 90); //Xman show complete up/down in queuelist
*/
	static const int colWidth[]={DFLT_CLIENTNAME_COL_WIDTH,
								DFLT_FILENAME_COL_WIDTH,
								DFLT_PRIORITY_COL_WIDTH,
								60,
								60,
								60,
								110,
								110,
								//60,
								DFLT_PARTSTATUS_COL_WIDTH,
								DFLT_CLIENTSOFT_COL_WIDTH,
								90};
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

	// Barry - Use preferred sort order from preferences
	SetSortArrow();
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0 : 100));
}

void CQueueListCtrl::Localize()
{
	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;

/*	if(pHeaderCtrl->GetItemCount() != 0) {
		CString strRes;

		strRes = GetResString(IDS_QL_USERNAME);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(0, &hdi);

		strRes = GetResString(IDS_FILE);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(1, &hdi);

		strRes = GetResString(IDS_FILEPRIO);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(2, &hdi);

		strRes = GetResString(IDS_QL_RATING);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(3, &hdi);

		strRes = GetResString(IDS_SCORE);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(4, &hdi);

		strRes = GetResString(IDS_ASKED);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(5, &hdi);

		strRes = GetResString(IDS_LASTSEEN);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(6, &hdi);

		strRes = GetResString(IDS_ENTERQUEUE);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(7, &hdi);

		strRes = GetResString(IDS_BANNED);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(8, &hdi);
		
		strRes = GetResString(IDS_UPSTATUS);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(9, &hdi);

		//Xman version see clientversion in every window
		strRes = GetResString(IDS_CD_CSOFT);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(10, &hdi);
		//Xman end

		//Xman show complete up/down in queuelist
		strRes = GetResString(IDS_UPDOWNUPLOADLIST);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(11, &hdi);
		//Xman end
*/
	CString strRes;
	for (int icol=0;icol<pHeaderCtrl->GetItemCount();++icol) {
		strRes=GetResString(colStrID[icol]);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(icol, &hdi);
	}
}

void CQueueListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
}

void CQueueListCtrl::SetAllIcons()
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

void CQueueListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CRect cur_rec(lpDrawItemStruct->rcItem);
	const CUpDownClient *client = (CUpDownClient *)lpDrawItemStruct->itemData;
	CMemoryDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	//InitItemMemDC(dc, lpDrawItemStruct->rcItem, /*client->HasLowID() ? RGB(255,250,200) :*/ ((lpDrawItemStruct->itemID % 2)?m_crEvenLine:m_crWindow), lpDrawItemStruct->itemState); //Xman show LowIDs
    InitItemMemDC(dc, lpDrawItemStruct->rcItem, (client->IsFriend())?((client->GetFriendSlot())?RGB(210,240,255):RGB(200,255,200)):((lpDrawItemStruct->itemID % 2)?m_crEvenLine:m_crWindow), lpDrawItemStruct->itemState);
	RECT rcClient;// X: [DDHC] - [Don't Draw Hidden Column]
	GetClientRect(&rcClient);

	CKnownFile* file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
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
					int image;
#ifdef CLIENTANALYZER		
					if(client->IsBadGuy())
						image = 9;
					else
#endif					
					{
						image = client->GetImageIndex();					
						if (((client->credits)?client->credits->GetScoreRatio(client->GetIP()):0) > 1)
							image++;					
					}
					m_ImageList.Draw(dc, image, point, ILD_NORMAL | INDEXTOOVERLAYMASK(nOverlayImage));
					cur_rec.left += 16;

					//EastShare Start - added by AndCycle, IP to Country, modified by Commander
					if(theApp.ip2country->ShowCountryFlag() ){
						point.x = cur_rec.left += sm_i2IconOffset;
						theApp.ip2country->GetFlagImageList()->Draw(dc, client->GetCountryFlagIndex(), point, ILD_NORMAL);
						cur_rec.left += 19;
					}
					//EastShare End - added by AndCycle, IP to Country

					cur_rec.left += sm_iLabelOffset;
					if (client->GetUserName()){
						_tcsncpy(szItem, client->GetUserName(), _countof(szItem) - 1);
						szItem[_countof(szItem) - 1] = _T('\0');
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT);
					}
					cur_rec.left -= 16 + sm_iLabelOffset;

					//EastShare Start - added by AndCycle, IP to Country
					if(theApp.ip2country->ShowCountryFlag() )
						cur_rec.left -= 19 + sm_i2IconOffset;
					//EastShare End - added by AndCycle, IP to Country

					break;
				}
				case 1:{
					const CKnownFile *file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
					if (file)
					{
						//Xman PowerRelease
						COLORREF crOldBackColor = dc.GetBkColor();
                        //PBF +
                        if (client->IsPBFClient())
					        dc.SetBkColor(RGB(230,230,200));
						//PBF -
						else if(file->GetUpPriority()==PR_POWER)
							dc.SetBkColor(RGB(255,225,225));
						_tcsncpy(szItem, file->GetFileName(), _countof(szItem) - 1);
						szItem[_countof(szItem) - 1] = _T('\0');
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT);
						dc.SetBkColor(crOldBackColor);
						//Xman end
					}
					break;
				}
                    // ZZUL-TRA :: TotalUpDown :: Start
					case 10:{
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
							_sntprintf(szItem, _countof(szItem), _T("%s/ %s"), CastItoXBytes(uUploadedTotal), CastItoXBytes(uDownloadedTotal));
							szItem[_countof(szItem) - 1] = _T('\0');
							dc->DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT);
							dc->SetTextColor(crOldTxtColor);
							}
						break;
					}
					// ZZUL-TRA :: TotalUpDown :: End
				case 8:
				//case 9:
					if( client->GetUpPartCount()){
						cur_rec.bottom--;
						cur_rec.top++;
						COLORREF crOldBackColor = dc.GetBkColor();
						client->DrawUpStatusBar(dc,&cur_rec,false,thePrefs.UseFlatBar());
						dc.SetBkColor(crOldBackColor); //Xman Code Improvement: FillSolidRect
						//Xman client percentage (font idea by morph)
						if (thePrefs.GetUseDwlPercentage() && client->GetHisCompletedPartsPercent_UP() >=0){
/*							COLORREF oldclr = dc.SetTextColor(RGB(0,0,0));
							int iOMode = dc.SetBkMode(TRANSPARENT);
							buffer.Format(_T("%i%%"), client->GetHisCompletedPartsPercent_UP());
							CFont *pOldFont = dc.SelectObject(&m_fontBoldSmaller);
#define	DrawClientPercentText	dc.DrawText(buffer, buffer.GetLength(),&cur_rec, ((MLC_DT_TEXT | DT_RIGHT) & ~DT_LEFT) | DT_CENTER)
							--cur_rec.top;--cur_rec.bottom;
							DrawClientPercentText;++cur_rec.left;++cur_rec.right;
							DrawClientPercentText;++cur_rec.left;++cur_rec.right;
							DrawClientPercentText;++cur_rec.top;++cur_rec.bottom;
							DrawClientPercentText;++cur_rec.top;++cur_rec.bottom;
							DrawClientPercentText;--cur_rec.left;--cur_rec.right;
							DrawClientPercentText;--cur_rec.left;--cur_rec.right;
							DrawClientPercentText;--cur_rec.top;--cur_rec.bottom;
							DrawClientPercentText;++cur_rec.left;++cur_rec.right;
							dc.SetTextColor(RGB(255,255,255));
							DrawClientPercentText;--cur_rec.left;--cur_rec.right;
							dc.SelectObject(pOldFont);
							dc.SetBkMode(iOMode);
							dc.SetTextColor(oldclr);*/

							COLORREF oldclr = dc.SetTextColor(RGB(0, 0, 255));
							int iOMode = dc.SetBkMode(TRANSPARENT);
							//CFont *pOldFont = dc.SelectObject(GetFont());
							_sntprintf(szItem, _countof(szItem), _T("%i%%"), client->GetHisCompletedPartsPercent_UP());
							dc.DrawText(szItem, -1, &cur_rec, (MLC_DT_TEXT & ~DT_LEFT) | DT_CENTER);
							//dc.SelectObject(pOldFont);
							dc.SetBkMode(iOMode);
							dc.SetTextColor(oldclr);
						}
						//Xman end
						cur_rec.bottom++;
						cur_rec.top--;
					}
					break;
				default:
					GetItemDisplayText(client, iColumn, szItem, _countof(szItem));
					if(szItem[0] != 0)
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT);
					break;
	   		}
		}
		cur_rec.left += iColumnWidth;
		if(cur_rec.left>=rcClient.right)// X: [DDHC] - [Don't Draw Hidden Column]
			break;
	}
}

void CQueueListCtrl::GetItemDisplayText(const CUpDownClient *client, int iSubItem, LPTSTR pszText, int cchTextMax)
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

		case 1: {
			const CKnownFile *file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
			if(file != NULL)
				_tcsncpy(pszText, file->GetFileName(), cchTextMax);
			break;
		}

		case 2: {
			const CKnownFile *file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
			if (file)
			{
				switch (file->GetUpPriority()) {
					case PR_VERYLOW :
						_tcsncpy(pszText, GetResString(IDS_PRIOVERYLOW), cchTextMax);
						break;
					case PR_LOW :
						_tcsncpy(pszText, GetResString(file->IsAutoUpPriority()?IDS_PRIOAUTOLOW:IDS_PRIOLOW), cchTextMax);
						break;
					case PR_NORMAL :
						_tcsncpy(pszText, GetResString(file->IsAutoUpPriority()?IDS_PRIOAUTONORMAL:IDS_PRIONORMAL), cchTextMax);
						break;
					case PR_HIGH :
						_tcsncpy(pszText, GetResString(file->IsAutoUpPriority()?IDS_PRIOAUTOHIGH:IDS_PRIOHIGH), cchTextMax);
						break;
					case PR_VERYHIGH :
						_tcsncpy(pszText, GetResString(IDS_PRIORELEASE), cchTextMax);
						break;
				   //Xman PowerRelease
					case PR_POWER:
						_tcsncpy(pszText, GetResString(IDS_POWERRELEASE), cchTextMax);
						break;
					//Xman end
				}
			}
			break;
		}

		case 3:
            switch (thePrefs.UseCreditSystem())
			{
			case 1:
			   _sntprintf(pszText, cchTextMax, _T("%i (%.1f)"), client->GetScore(false, false, true), client->Credits()->GetScoreRatio(client->GetIP()));//morph4u see multiplier
  			   //_ultot_s_ptr(client->GetScore(false, false, true), pszText, cchTextMax, 10);// X: [CI] - [Code Improvement]
                  break;
            case 2:
                  if(client->GetAntiLeechData())
			   _sntprintf(pszText, cchTextMax, _T("%i (%.1f)"), client->GetScore(false, false, true), client->GetAntiLeechData()->GetScore());//morph4u see multiplier CA
                  break;
			}
			break;

		case 4:
			if (client->HasLowID()) {
				if (client->m_bAddNextConnect)
					_sntprintf(pszText, cchTextMax, _T("%i ****"),client->GetScore(false));
				else
					_sntprintf(pszText, cchTextMax, _T("%i (%s)"),client->GetScore(false), GetResString(IDS_IDLOW));
			}
			else
				_sntprintf(pszText, cchTextMax, 
				//Xman uploading problem client
					(client->isupprob && client->m_bAddNextConnect) ? 
						((client->socket && client->socket->IsConnected())?_T("%i #~~"):_T("%i ~~~"))
				//Xman end
					:
						_T("%i")
					,client->GetScore(false));
			break;

		case 5:
			_ultot_s(client->GetAskedCount(), pszText, cchTextMax, 10);// X: [CI] - [Code Improvement]
			break;
		
		case 6:
			_tcsncpy(pszText, CastSecondsToHM((GetTickCount() - client->GetLastUpRequest()) / 1000), cchTextMax);
			break;

		case 7:
			_tcsncpy(pszText, CastSecondsToHM((GetTickCount() - client->GetWaitStartTime()) / 1000), cchTextMax);
			break;

		case 8:
			//_tcsncpy(pszText, GetResString(client->IsBanned() ? IDS_YES : IDS_NO), cchTextMax);
			//break;

		//case 9:
			if(thePrefs.GetUseDwlPercentage() && client->GetHisCompletedPartsPercent_UP() >=0)
				_sntprintf(pszText, cchTextMax, _T("%i%%"), client->GetHisCompletedPartsPercent_UP());
			break;
		//Xman version see clientversion in every window
		case 9://10:
			_tcsncpy(pszText, client->DbgGetFullClientSoftVer(), cchTextMax); //Xman // Maella -Support for tag ET_MOD_VERSION 0x55
			break;
		//Xman end

		//Xman show complete up/down in queuelist
		case 10://11:
			if(client->Credits() )
				_sntprintf(pszText, cchTextMax, _T("%s/ %s"), CastItoXBytes(client->credits->GetUploadedTotal()), CastItoXBytes(client->credits->GetDownloadedTotal()));
			break;
		//Xman end
	}
	pszText[cchTextMax - 1] = _T('\0');
}

void CQueueListCtrl::OnLvnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
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

void CQueueListCtrl::OnLvnColumnClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLISTVIEW *pNMListView = (NMLISTVIEW *)pNMHDR;
	bool sortAscending;
	if (GetSortItem() != pNMListView->iSubItem)
	{
		switch (pNMListView->iSubItem)
		{
			case 2: // Up Priority
			case 3: // Rating
			case 4: // Score
			case 5: // Ask Count
			//case 8: // Banned
			case 8: // Part Count
			case 9:
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

int CQueueListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
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

		case 2: {
			const CKnownFile *file1 = theApp.sharedfiles->GetFileByID(item1->GetUploadFileID());
			const CKnownFile *file2 = theApp.sharedfiles->GetFileByID(item2->GetUploadFileID());
			if (file1 != NULL && file2 != NULL)
				iResult = (file1->GetUpPriority() == PR_VERYLOW ? -1 : file1->GetUpPriority()) - (file2->GetUpPriority() == PR_VERYLOW ? -1 : file2->GetUpPriority());
			else if (file1 == NULL)
				iResult = 1;
			else
				iResult = -1;
			break;
		}

		case 3:
			iResult = CompareUnsigned_PTR(item1->GetScore(false, false, true), item2->GetScore(false, false, true));
			break;

		case 4:
	// ==> Superior Client Handling [Stulle] - Stulle
			if(!item1->IsSuperiorClient() && item2->IsSuperiorClient())
					iResult=-1;
			else if(item1->IsSuperiorClient() && !item2->IsSuperiorClient())
					iResult=1;
			else
			// <== Superior Client Handling [Stulle] - Stulle
			iResult = CompareUnsigned_PTR(item1->GetScore(false), item2->GetScore(false));
			break;

		case 5:
			iResult = CompareUnsigned(item1->GetAskedCount(), item2->GetAskedCount());
			break;

		case 6:
			iResult = CompareUnsigned(item1->GetLastUpRequest(), item2->GetLastUpRequest());
			break;

		case 7:
			iResult = CompareUnsigned(item1->GetWaitStartTime(), item2->GetWaitStartTime());
			break;
		
		case 8: 
			//iResult = item1->IsBanned() - item2->IsBanned();
			//break;
		
		//case 9: 
			iResult = CompareUnsigned(item1->GetUpPartCount(), item2->GetUpPartCount());
			break;
		//Xman version see clientversion in every window
		case 9://10:
			// Maella -Support for tag ET_MOD_VERSION 0x55-
			if(item1->GetClientSoft() == item2->GetClientSoft())
				if(item1->GetVersion() == item2->GetVersion() && (item1->GetClientSoft() == SO_EMULE || item1->GetClientSoft() == SO_AMULE)){
					iResult= item1->DbgGetFullClientSoftVer().CompareNoCase( item2->DbgGetFullClientSoftVer());
				}
				else {
					iResult = item1->GetVersion() - item2->GetVersion();
				}
			else
				iResult= item2->GetClientSoft() - item1->GetClientSoft();
			break;
		//Xman end

		//Xman show complete up/down in queuelist
		case 10://11:
			if(item1->Credits() && item2->Credits())
			{
				iResult=CompareUnsigned64(item1->credits->GetUploadedTotal(), item2->credits->GetUploadedTotal());
			}
			else
				return 0;
			break;
		//Xman end

	}
	if (lParamSort >= 100)
		return -iResult;
	// SLUGFILLER: multiSort remove - handled in parent class
	/*
	//call secondary sortorder, if this one results in equal
	int dwNextSort;
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->transferwnd->GetQueueList()->GetNextSortOrder(lParamSort)) != (-1)){
		iResult= SortProc(lParam1, lParam2, dwNextSort);
	}
	*/
	return iResult;

}

//Xman faster Updating of Queuelist
void CQueueListCtrl::UpdateAll()
{
	if(CemuleDlg::IsRunning())
	{
		RedrawItems(0,GetItemCount());
		//CWnd::UpdateWindow(); //not needed because of sorting
		// Sort table
		SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0:100));
	}
}

void CQueueListCtrl::OnNmDblClk(NMHDR* /*pNMHDR*/, LRESULT *pResult)
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

void CQueueListCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
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

BOOL CQueueListCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
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
			case MP_LIST_REQUESTED_FILES: { 
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

void CQueueListCtrl::AddClient(/*const*/ CUpDownClient *client, bool resetclient)
{
	if (resetclient && client){
		client->SetWaitStartTime();
		client->SetAskedCount(1);
	}

	if (!CemuleDlg::IsRunning())
		return;
	if (thePrefs.IsQueueListDisabled())
		return;

	size_t iItemCount = GetItemCount();
	/*int iItem = */InsertItem(LVIF_TEXT | LVIF_PARAM, iItemCount, LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)client);
	//Update(iItem);
	theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2OnQueue, iItemCount + 1);
}

void CQueueListCtrl::RemoveClient(const CUpDownClient *client)
{
	if (!CemuleDlg::IsRunning())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	int result = FindItem(&find);
	if (result != -1) {
		DeleteItem(result);
		theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2OnQueue);
	}
}

void CQueueListCtrl::RefreshClient(const CUpDownClient *client)
{
	if (!CemuleDlg::IsRunning())
		return;

	if (theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd || !/*theApp.emuledlg->transferwnd->GetQueueList()->*/IsWindowVisible())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	int result = FindItem(&find);
	if (result != -1)
		Update(result);
}

void CQueueListCtrl::ShowSelectedUserDetails()
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

void CQueueListCtrl::ShowQueueClients()
{
	DeleteAllItems(); 
	CUpDownClient *update = theApp.uploadqueue->GetNextClient(NULL);
	while( update )
	{
		AddClient(update, false);
		update = theApp.uploadqueue->GetNextClient(update);
	}
}

// Barry - Refresh the queue every 10 secs
void CALLBACK CQueueListCtrl::QueueUpdateTimer(HWND /*hwnd*/, UINT /*uiMsg*/, UINT_PTR /*idEvent*/, DWORD /*dwTime*/)
{
	// NOTE: Always handle all type of MFC exceptions in TimerProcs - otherwise we'll get mem leaks
	//Xman unreachable
	//try
	{
		if (   !CemuleDlg::IsRunning() // Don't do anything if the app is shutting down - can cause unhandled exceptions
			|| !thePrefs.GetUpdateQueueList()
			|| theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd
			|| !theApp.emuledlg->transferwnd->GetQueueList()->IsWindowVisible() )
			return;

		//Xman faster Updating of Queuelist
		if (theApp.emuledlg->transferwnd->GetQueueList()->GetItemCount()>1)
		{

			theApp.emuledlg->transferwnd->GetQueueList()->UpdateAll();
		}
		//Xman end
	}
	//Xman unreachable
	/*
	CATCH_DFLT_EXCEPTIONS(_T("CQueueListCtrl::QueueUpdateTimer"))
		// Maella -Code Improvement-
		// Remark: The macro CATCH_DFLT_EXCEPTIONS will not catch all types of exception.
		//         The exceptions thrown in callback function are not intercepted by the dbghelp.dll (e.g. eMule Dump, crashRpt, etc...)
		catch(...) {
			if(theApp.emuledlg != NULL)
				AddLogLine(true, _T("Unknown exception in %s"), __FUNCTION__);
		}
		// Maella end
	*/
}
//Xman end
