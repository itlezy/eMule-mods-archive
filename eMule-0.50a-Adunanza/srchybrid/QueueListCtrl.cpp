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
#include "ListenSocket.h" //Anis

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


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

	InsertColumn(0, GetResString(IDS_QL_USERNAME),	LVCFMT_LEFT, DFLT_CLIENTNAME_COL_WIDTH);
	InsertColumn(1, GetResString(IDS_FILE),			LVCFMT_LEFT, DFLT_FILENAME_COL_WIDTH);
	InsertColumn(2, GetResString(IDS_FILEPRIO),		LVCFMT_LEFT, DFLT_PRIORITY_COL_WIDTH);
	InsertColumn(3, GetResString(IDS_QL_RATING),	LVCFMT_LEFT,  60);
	InsertColumn(4, GetResString(IDS_SCORE),		LVCFMT_LEFT,  60);
	InsertColumn(5, GetResString(IDS_ASKED),		LVCFMT_LEFT,  60);
	InsertColumn(6, GetResString(IDS_LASTSEEN),		LVCFMT_LEFT, 110);
	InsertColumn(7, GetResString(IDS_ENTERQUEUE),	LVCFMT_LEFT, 110);
	InsertColumn(8, GetResString(IDS_BANNED),		LVCFMT_LEFT,  60);
	InsertColumn(9, GetResString(IDS_UPSTATUS),		LVCFMT_LEFT, DFLT_PARTSTATUS_COL_WIDTH);

	SetAllIcons();
	Localize();
	LoadSettings();
	//Anis Percentuale client
	CFont* pFont = GetFont();
	LOGFONT lfFont = {0};
	pFont->GetLogFont(&lfFont);
	lfFont.lfHeight = 11;
	m_fontBoldSmaller.CreateFontIndirect(&lfFont);
	SetSortArrow();
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0 : 100));
}

void CQueueListCtrl::Localize()
{
	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;

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
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkey")));
	m_ImageList.Add(CTempIconLoader(_T("ClientCompatible")));
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyPlus")));
	m_ImageList.Add(CTempIconLoader(_T("ClientCompatiblePlus")));
	m_ImageList.Add(CTempIconLoader(_T("Friend")));
	m_ImageList.Add(CTempIconLoader(_T("ClientMLDonkey")));
	m_ImageList.Add(CTempIconLoader(_T("ClientMLDonkeyPlus")));
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyHybrid")));
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyHybridPlus")));
	m_ImageList.Add(CTempIconLoader(_T("ClientShareaza")));
	m_ImageList.Add(CTempIconLoader(_T("ClientShareazaPlus")));
	m_ImageList.Add(CTempIconLoader(_T("ClientAMule")));
	m_ImageList.Add(CTempIconLoader(_T("ClientAMulePlus")));
	m_ImageList.Add(CTempIconLoader(_T("ClientLPhant")));
	m_ImageList.Add(CTempIconLoader(_T("ClientLPhantPlus")));
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("ClientSecureOvl"))), 1);
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("OverlayObfu"))), 2);
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("OverlaySecureObfu"))), 3);
	m_ImageList.Add(CTempIconLoader(_T("ClientAdunanzA"))); // 16
	m_ImageList.Add(CTempIconLoader(_T("ClientFastWeb")));  // 17
	// Stefano Picerno: Mod adunanza - aggiungo le icone Adunanza

	// fine mod Adu
	// Apply the image list also to the listview control, even if we use our own 'DrawItem'.
	// This is needed to give the listview control a chance to initialize the row height.
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) != 0 );
	VERIFY( ApplyImageList(m_ImageList) == NULL );
}

#define DLC_DT_TEXT (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS)

void CQueueListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	if (!lpDrawItemStruct->itemData)
		return;
	CDC* odc = CDC::FromHandle(lpDrawItemStruct->hDC);
	BOOL bCtrlFocused = ((GetFocus() == this) || (GetStyle() & LVS_SHOWSELALWAYS));
	if (lpDrawItemStruct->itemState & ODS_SELECTED) {
		if (bCtrlFocused)
			odc->SetBkColor(m_crHighlight);
		else
			odc->SetBkColor(m_crNoHighlight);
	}
	else
		odc->SetBkColor(GetBkColor());
	const CUpDownClient* client = (CUpDownClient*)lpDrawItemStruct->itemData;
	CMemoryDC dc(odc, &lpDrawItemStruct->rcItem);
	CFont* pOldFont = dc.SelectObject(GetFont());
	CRect cur_rec(lpDrawItemStruct->rcItem);
	COLORREF crOldTextColor = dc.SetTextColor((lpDrawItemStruct->itemState & ODS_SELECTED) ? m_crHighlightText : m_crWindowText);

	int iOldBkMode;
	if (m_crWindowTextBk == CLR_NONE){
		DefWindowProc(WM_ERASEBKGND, (WPARAM)(HDC)dc, 0);
		iOldBkMode = dc.SetBkMode(TRANSPARENT);
	}
	else
		iOldBkMode = OPAQUE;

	CKnownFile* file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	cur_rec.right = cur_rec.left - 8;
	cur_rec.left += 4;
	CString Sbuffer;
	for(int iCurrent = 0; iCurrent < iCount; iCurrent++){
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
		if( !IsColumnHidden(iColumn) ){
			cur_rec.right += GetColumnWidth(iColumn);
			switch(iColumn){
				case 0:{
					int iImage;
					// Inizio modifiche AdunanzA
					DWORD aduType = client->GetClientAduType();
							if (client->IsFriend())
								iImage = 4;
							else if (aduType == ADUNANZA_ICON_ADU)
								iImage = 18;
							else if (aduType == ADUNANZA_ICON_FW)
								iImage = 19;
							else if (client->GetClientSoft() == SO_EDONKEYHYBRID) {
								if (((client->credits)?client->credits->GetScoreRatio(client):0) > 1)
									iImage = 8;
								else
									iImage = 7;
							}
							else if (client->GetClientSoft() == SO_MLDONKEY) {
								if (((client->credits)?client->credits->GetScoreRatio(client):0) > 1)
									iImage = 6;
								else
									iImage = 5;
							}
							else if (client->GetClientSoft() == SO_SHAREAZA) {
								if (((client->credits)?client->credits->GetScoreRatio(client):0) > 1)
									iImage = 10;
								else
									iImage = 9;
							}
							else if (client->GetClientSoft() == SO_AMULE) {
								if (((client->credits)?client->credits->GetScoreRatio(client):0) > 1)
									iImage = 12;
								else
									iImage = 11;
							}
							else if (client->GetClientSoft() == SO_LPHANT) {
								if (((client->credits)?client->credits->GetScoreRatio(client):0) > 1)
									iImage = 14;
								else
									iImage = 13;
							}
							else if (client->ExtProtocolAvailable()) {
								if (((client->credits)?client->credits->GetScoreRatio(client):0) > 1)
									iImage = 3;
								else
									iImage = 1;
							}
							else {
								if (((client->credits)?client->credits->GetScoreRatio(client):0) > 1)
									iImage = 2;
								else
									iImage = 0;
							}
					
		
						UINT nOverlayImage = 0;
						if ((client->Credits() && client->Credits()->GetCurrentIdentState(client->GetIP()) == IS_IDENTIFIED))
							nOverlayImage |= 1;
			
						if(client->IsObfuscatedConnectionEstablished() 
							|| (!(client->socket != NULL && client->socket->IsConnected())
							&& (client->SupportsCryptLayer() && thePrefs.IsClientCryptLayerSupported() && (client->RequestsCryptLayer() || thePrefs.IsClientCryptLayerRequested()))))
							nOverlayImage |= 2;

					int iIconPosY = (cur_rec.Height() > 16) ? ((cur_rec.Height() - 16) / 2) : 1;	
					POINT point = { cur_rec.left, cur_rec.top + iIconPosY };
							m_ImageList.Draw(dc, iImage, point, ILD_NORMAL | INDEXTOOVERLAYMASK(nOverlayImage));

					Sbuffer = client->GetUserName();
					cur_rec.left +=20;
					dc.DrawText(Sbuffer,Sbuffer.GetLength(),&cur_rec,DLC_DT_TEXT);
					cur_rec.left -=20;
					break;
				}
				case 1:
					if(file)
						Sbuffer = file->GetFileName();
					else
						Sbuffer = _T("?");
					break;
				case 2:
					if(file){
						switch (file->GetUpPriority()) {
							case PR_VERYLOW : {
								Sbuffer = GetResString(IDS_PRIOVERYLOW);
								break; }
							case PR_LOW : {
								if( file->IsAutoUpPriority() )
									Sbuffer = GetResString(IDS_PRIOAUTOLOW);
								else
									Sbuffer = GetResString(IDS_PRIOLOW);
								break; }
							case PR_NORMAL : {
								if( file->IsAutoUpPriority() )
									Sbuffer = GetResString(IDS_PRIOAUTONORMAL);
								else
									Sbuffer = GetResString(IDS_PRIONORMAL);
								break; }
							case PR_HIGH : {
								if( file->IsAutoUpPriority() )
									Sbuffer = GetResString(IDS_PRIOAUTOHIGH);
								else
									Sbuffer = GetResString(IDS_PRIOHIGH);
								break; }
							case PR_VERYHIGH : {
								Sbuffer = GetResString(IDS_PRIORELEASE);
								break; }
							default:
								Sbuffer.Empty();
						}
					}
					else
						Sbuffer = _T("?");
					break;
				case 3:
					Sbuffer.Format(_T("%i"),client->GetScore(false,false,true));
					break;
				case 4:
					if (client->HasLowID()){
						if (client->m_bAddNextConnect)
							Sbuffer.Format(_T("%i ****"),client->GetScore(false));
						else
						{
							// Anis
							if(client->HasFastwebIP())
								Sbuffer.Format(_T("%i (%s)"),client->GetScore(false), GetResString(IDS_IDLOW));
							else
								Sbuffer.Format(_T("%i (%s)"),client->GetScore(false), _T("ID Basso"));
						}
					}
					else
						Sbuffer.Format(_T("%i"),client->GetScore(false));
					break;
				case 5:
					Sbuffer.Format(_T("%i"),client->GetAskedCount());
					break;
				case 6:
					Sbuffer = CastSecondsToHM((::GetTickCount() - client->GetLastUpRequest())/1000);
					break;
				case 7:
					Sbuffer = CastSecondsToHM((::GetTickCount() - client->GetWaitStartTime())/1000);
					break;
				case 8:
					if(client->IsBanned())
						Sbuffer = GetResString(IDS_YES);
					else
						Sbuffer = GetResString(IDS_NO);
					break;
				case 9: {
					if (client->GetUpPartCount()) {
							cur_rec.bottom--;
							cur_rec.top++;
							client->DrawUpStatusBar(dc,&cur_rec,false,thePrefs.UseFlatBar());
							//Xman client percentage (font idea by morph)
							CString buffer;
							if (thePrefs.GetUseDwlPercentage())
							{
								if(client->GetHisCompletedPartsPercent_UP() >=0)
								{
									COLORREF oldclr = dc.SetTextColor(RGB(0,0,0));
									int iOMode = dc.SetBkMode(TRANSPARENT);
									buffer.Format(_T("%i%%"), client->GetHisCompletedPartsPercent_UP());
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
						}
							//Xman end
							COLORREF crOldBackColor = dc->GetBkColor(); //Anis
							dc.SetBkColor(crOldBackColor); //zz_fly
							cur_rec.bottom++;
							cur_rec.top--;
						}
						break;
		   	}
			if( iColumn != 9 && iColumn != 0)
				dc.DrawText(Sbuffer,Sbuffer.GetLength(),&cur_rec,DLC_DT_TEXT);
			cur_rec.left += GetColumnWidth(iColumn);
		}
	}

	// draw rectangle around selected item(s)
	if (lpDrawItemStruct->itemState & ODS_SELECTED)
	{
		RECT outline_rec = lpDrawItemStruct->rcItem;

		outline_rec.top--;
		outline_rec.bottom++;
		dc.FrameRect(&outline_rec, &CBrush(GetBkColor()));
		outline_rec.top++;
		outline_rec.bottom--;
		outline_rec.left++;
		outline_rec.right--;

		if(bCtrlFocused)
			dc.FrameRect(&outline_rec, &CBrush(m_crFocusLine));
		else
			dc.FrameRect(&outline_rec, &CBrush(m_crNoFocusLine));
	}
	
	if (m_crWindowTextBk == CLR_NONE)
		dc.SetBkMode(iOldBkMode);
	dc.SelectObject(pOldFont);
	dc.SetTextColor(crOldTextColor);
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
			if (client->GetUserName() == NULL)
				_sntprintf(pszText, cchTextMax, _T("(%s)"), GetResString(IDS_UNKNOWN));
			else
				_tcsncpy(pszText, client->GetUserName(), cchTextMax);
			break;

		case 1: {
			const CKnownFile *file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
			_tcsncpy(pszText, file != NULL ? file->GetFileName() : _T(""), cchTextMax);
			break;
		}
		
		case 3:
			_sntprintf(pszText, cchTextMax, _T("%i"), client->GetScore(false, false, true));
			break;
		
		case 4:
			if (client->HasLowID()) {
				if (client->m_bAddNextConnect)
					_sntprintf(pszText, cchTextMax, _T("%i ****"),client->GetScore(false));
				else 
				{
					if(client->HasFastwebIP())
						_sntprintf(pszText, cchTextMax, _T("%i (%s)"),client->GetScore(false), GetResString(IDS_IDLOW));
					else
						_sntprintf(pszText, cchTextMax, _T("%i (%s)"),client->GetScore(false), _T("ID Basso"));
				}

			}
			else
				_sntprintf(pszText, cchTextMax, _T("%i"), client->GetScore(false));
			break;

		case 5:
			_sntprintf(pszText, cchTextMax, _T("%i"), client->GetAskedCount());
			break;
		
		case 6:
			_tcsncpy(pszText, CastSecondsToHM((GetTickCount() - client->GetLastUpRequest()) / 1000), cchTextMax);
			break;

		case 7:
			_tcsncpy(pszText, CastSecondsToHM((GetTickCount() - client->GetWaitStartTime()) / 1000), cchTextMax);
			break;

		case 8:
			_tcsncpy(pszText, GetResString(client->IsBanned() ? IDS_YES : IDS_NO), cchTextMax);
			break;

		case 9:
			_tcsncpy(pszText, GetResString(IDS_UPSTATUS), cchTextMax);
			break;
	}
	pszText[cchTextMax - 1] = _T('\0');
}

void CQueueListCtrl::OnLvnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
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
			case 8: // Banned
			case 9: // Part Count
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

		case 1: {
			const CKnownFile *file1 = theApp.sharedfiles->GetFileByID(item1->GetUploadFileID());
			const CKnownFile *file2 = theApp.sharedfiles->GetFileByID(item2->GetUploadFileID());
			if (file1 != NULL && file2 != NULL)
			{
				//>>> Anis::PowerShare
				iResult = file1->IsPowerShared()-file2->IsPowerShared();
				if(iResult == 0)
				//### Anis::PowerShare
				iResult = CompareLocaleStringNoCase(file1->GetFileName(), file2->GetFileName());
			}
			
			else if (file1 == NULL)
				iResult = 1;
			else
				iResult = -1;
			break;
		}

		case 2: {
			const CKnownFile *file1 = theApp.sharedfiles->GetFileByID(item1->GetUploadFileID());
			const CKnownFile *file2 = theApp.sharedfiles->GetFileByID(item2->GetUploadFileID());
			if (file1 != NULL && file2 != NULL) {
				//>>> Anis::PowerShare
				iResult = file1->IsPowerShared()-file2->IsPowerShared();
				if(iResult == 0)
				//### Anis::PowerShare
				iResult = (file1->GetUpPriority() == PR_VERYLOW ? -1 : file1->GetUpPriority()) - (file2->GetUpPriority() == PR_VERYLOW ? -1 : file2->GetUpPriority());
			}
			else if (file1 == NULL)
				iResult = 1;
			else
				iResult = -1;
			break;
		}

		case 3:
//>>> Anis::PowerShare
			iResult = item1->GetFriendSlot() - item2->GetFriendSlot();
			if(iResult == 0)
			{
				CKnownFile* file1 = theApp.sharedfiles->GetFileByID(item1->GetUploadFileID());
				CKnownFile* file2 = theApp.sharedfiles->GetFileByID(item2->GetUploadFileID());
				bool bIsPS1 = file1 && file1->IsPowerShared();
				bool bIsPS2 = file2 && file2->IsPowerShared();
				if(bIsPS1 && bIsPS2)
					iResult = (file1->GetUpPriority() == PR_VERYLOW ? -1 : file1->GetUpPriority()) - (file2->GetUpPriority() == PR_VERYLOW ? -1 : file2->GetUpPriority());
				else
					iResult = bIsPS1 - bIsPS2;
			}
			if(iResult == 0)
//### Anis::PowerShare
				iResult = CompareUnsigned(item1->GetScore(false, false, true), item2->GetScore(false, false, true));
			break;

		case 4:
			iResult = CompareUnsigned(item1->GetScore(false), item2->GetScore(false));
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
			iResult = item1->IsBanned() - item2->IsBanned();
			break;
		
		case 9: 
            if (item1->GetHisCompletedPartsPercent_UP() == item2->GetHisCompletedPartsPercent_UP())  
                iResult=0;  
            else  
                iResult=item1->GetHisCompletedPartsPercent_UP() > item2->GetHisCompletedPartsPercent_UP()?1:-1;  
			break;
	}

	if (lParamSort >= 100)
		iResult = -iResult;

	//call secondary sortorder, if this one results in equal
	int dwNextSort;
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->transferwnd->GetQueueList()->GetNextSortOrder(lParamSort)) != -1)
		iResult = SortProc(lParam1, lParam2, dwNextSort);

	return iResult;
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
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	const CUpDownClient* client = (iSel != -1) ? (CUpDownClient*)GetItemData(iSel) : NULL;

	CTitleMenu ClientMenu;
	ClientMenu.CreatePopupMenu();
	ClientMenu.AddMenuTitle(GetResString(IDS_CLIENTS), true);
	ClientMenu.AppendMenu(MF_STRING | (client ? MF_ENABLED : MF_GRAYED), MP_DETAIL, GetResString(IDS_SHOWDETAILS), _T("CLIENTDETAILS"));
	ClientMenu.SetDefaultItem(MP_DETAIL);
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && !client->IsFriend()) ? MF_ENABLED : MF_GRAYED), MP_ADDFRIEND, GetResString(IDS_ADDFRIEND), _T("ADDFRIEND"));
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED), MP_MESSAGE, GetResString(IDS_SEND_MSG), _T("SENDMESSAGE"));
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED), MP_SHOWLIST, GetResString(IDS_VIEWFILES), _T("VIEWFILES"));
	if (thePrefs.IsExtControlsEnabled())
		ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->IsBanned()) ? MF_ENABLED : MF_GRAYED), MP_UNBAN, GetResString(IDS_UNBAN));
	if (Kademlia::CKademlia::IsRunning() && !Kademlia::CKademlia::IsConnected())
		ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetKadPort()!=0 && client->GetKadVersion() > 1) ? MF_ENABLED : MF_GRAYED), MP_BOOT, GetResString(IDS_BOOTSTRAP));
	ClientMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED), MP_FIND, GetResString(IDS_FIND), _T("Search"));
	GetPopupMenuPos(*this, point);
	ClientMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
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

	if (!theApp.emuledlg->IsRunning())
		return;
	if (thePrefs.IsQueueListDisabled())
		return;

	int iItemCount = GetItemCount();
	int iItem = InsertItem(LVIF_TEXT | LVIF_PARAM, iItemCount, LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)client);
	Update(iItem);
	theApp.emuledlg->transferwnd->UpdateListCount(wnd2OnQueue);
}

void CQueueListCtrl::RemoveClient(const CUpDownClient *client)
{
	if (!theApp.emuledlg->IsRunning())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	int result = FindItem(&find);
	if (result != -1) {
		DeleteItem(result);
		theApp.emuledlg->transferwnd->UpdateListCount(wnd2OnQueue);
	}
}

void CQueueListCtrl::RefreshClient(const CUpDownClient *client)
{
	if (!theApp.emuledlg->IsRunning())
		return;

	if (theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd || !theApp.emuledlg->transferwnd->GetQueueList()->IsWindowVisible())
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
void CALLBACK CQueueListCtrl::QueueUpdateTimer(HWND /*hwnd*/, UINT /*uiMsg*/, UINT /*idEvent*/, DWORD /*dwTime*/)
{
	// NOTE: Always handle all type of MFC exceptions in TimerProcs - otherwise we'll get mem leaks
	try
	{
		if (!theApp.emuledlg->IsRunning() || !thePrefs.GetUpdateQueueList() || theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd || !theApp.emuledlg->transferwnd->GetQueueList()->IsWindowVisible() )
			return;

		if (theApp.emuledlg->transferwnd->GetQueueList()->GetItemCount()>1)
			theApp.emuledlg->transferwnd->GetQueueList()->UpdateAll();
	}
	CATCH_DFLT_EXCEPTIONS(_T("CQueueListCtrl::QueueUpdateTimer"))
}

//coda ultra veloce
void CQueueListCtrl::UpdateAll()
{
	if(theApp.emuledlg->IsRunning())
	{
		RedrawItems(0,GetItemCount());
		SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0:100));
	}
}
