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
	,IDS_BANNED
	,IDS_UPSTATUS
	,IDS_CD_CSOFT //>>> WiZaRd::Show ModVer
	,IDS_TOTALUPDOWN // ZZUL-TRA :: TotalUpDown
    ,IDS_CL_DOWNLSTATUS //Download-Status
    ,IDS_COUNTRY // ZZUL-TRA :: IP2Country
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

	static const int colWidth[]={DFLT_CLIENTNAME_COL_WIDTH,
								DFLT_FILENAME_COL_WIDTH,
								DFLT_PRIORITY_COL_WIDTH,
								60,
								60,
								60,
								110,
								110,
								60,
								DFLT_PARTSTATUS_COL_WIDTH,
								DFLT_CLIENTSOFT_COL_WIDTH, //>>> WiZaRd::Show ModVer
								120, // ZZUL-TRA :: TotalUpDown
								100, //Download-Status
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

void CQueueListCtrl::Localize()
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

void CQueueListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
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
							theApp.ip2country->GetFlagImageList()->DrawIndirect(dc, client->GetCountryFlagIndex(), point2, CSize(18,16), CPoint(0,0), ILD_NORMAL);
							cur_rec.left += sm_iLabelOffset;
						}
						// ZZUL-TRA :: IP2Country :: End

						cur_rec.left += 16 + sm_iLabelOffset;
#ifndef EVENLINE		
                     //  ZZUL-TRA :: Colors :: Start
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

					case 9:
						if (client->GetUpPartCount()) {
							cur_rec.bottom--;
							cur_rec.top++;
							client->DrawUpStatusBar(dc, &cur_rec, false, thePrefs.UseFlatBar());
							//ZZUL-TRA :: ClientPercentage :: Start
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
							//ZZUL-TRA :: ClientPercentage :: End
							cur_rec.bottom++;
							cur_rec.top--;
						}
						break;
                 // ZZUL-TRA :: Colors :: Start
                   case 1:{
                    const uint8 iSuperiorStates = client->GetSuperiorClientStates();//PBF
					const CKnownFile *file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
					if (file)
					{
						COLORREF crOldBackColor = dc.GetBkColor();
                        //PBF +
                        if (iSuperiorStates & SCS_PBF_SUI)
					        dc.SetBkColor(RGB(230,230,200));
						//PBF -
						else if(file->GetPowerSharedMode()==3)
							dc.SetBkColor(RGB(250,230,200));
						else if(file->GetPowerShared())
							dc.SetBkColor(RGB(255,225,225));
						_tcsncpy(szItem, file->GetFileName(), _countof(szItem) - 1);
						szItem[_countof(szItem) - 1] = _T('\0');
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT);
						dc.SetBkColor(crOldBackColor);
					}
				}
					break;
				// ZZUL-TRA :: Colors :: End

	// ZZUL-TRA :: TotalUpDown :: Start
					case 11:{
						if(client->Credits())
						{
							const COLORREF crOldTxtColor = dc->GetTextColor();
							const uint64 uUploadedTotal = client->Credits()->GetUploadedTotal();
							const uint64 uDownloadedTotal = client->Credits()->GetDownloadedTotal();

							if (uUploadedTotal > 0 || uDownloadedTotal > 0)
							{
								if (uUploadedTotal > uDownloadedTotal)
									dc->SetTextColor(RGB(192, 0, 0));
								else
									dc->SetTextColor(RGB(0, 192, 0));
							}
							dc->DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
							dc->SetTextColor(crOldTxtColor);
						}
						break;
					}
					// ZZUL-TRA :: TotalUpDown :: End

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

		case 2: {
			const CKnownFile *file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
			if (file)
			{
				switch (file->GetUpPriority())
				{
					case PR_VERYLOW:
						_tcsncpy(pszText, GetResString(IDS_PRIOVERYLOW), cchTextMax);
						break;
					
					case PR_LOW:
						if (file->IsAutoUpPriority())
							_tcsncpy(pszText, GetResString(IDS_PRIOAUTOLOW), cchTextMax);
						else
							_tcsncpy(pszText, GetResString(IDS_PRIOLOW), cchTextMax);
						break;
					
					case PR_NORMAL:
						if (file->IsAutoUpPriority())
							_tcsncpy(pszText, GetResString(IDS_PRIOAUTONORMAL), cchTextMax);
						else
							_tcsncpy(pszText, GetResString(IDS_PRIONORMAL), cchTextMax);
						break;

					case PR_HIGH:
						if (file->IsAutoUpPriority())
							_tcsncpy(pszText, GetResString(IDS_PRIOAUTOHIGH), cchTextMax);
						else
							_tcsncpy(pszText, GetResString(IDS_PRIOHIGH), cchTextMax);
						break;

					case PR_VERYHIGH:
						_tcsncpy(pszText, GetResString(IDS_PRIORELEASE), cchTextMax);
						break;
				}

                if(file->GetPowerShared()) {
					_tcsncat(pszText, _T(" "), cchTextMax);
					_tcsncat(pszText, GetResString(IDS_POWERSHARE_PREFIX), cchTextMax);
                }

			}
			break;
		}

//morph4u +
		case 3:
            switch (thePrefs.UseCreditSystem())
			{
			case 1:
			   _sntprintf(pszText, cchTextMax, _T("%i (%.1f)"), client->GetScore(false, false, true), client->Credits()->GetScoreRatio(client->GetIP()));//morph4u see multiplier
                  break;
#ifdef CLIENTANALYZER
            case 2:
                  if(client->GetAntiLeechData())
			   _sntprintf(pszText, cchTextMax, _T("%i (%.1f)"), client->GetScore(false, false, true), client->GetAntiLeechData()->GetScore());//morph4u see multiplier CA
                  break;
#endif
			}
			break;
//morph4u  -		

		// ZZUL-TRA :: PaybackFirst :: Start
		/*
		case 4:
			if (client->HasLowID()) {
				if (client->m_dwWouldHaveGottenUploadSlotIfNotLowIdTick)
					_sntprintf(pszText, cchTextMax, _T("%i **** Awaited reconnect %s"),client->GetScore(false), CastSecondsToHM((::GetTickCount()-client->m_dwWouldHaveGottenUploadSlotIfNotLowIdTick)/1000));
				else
					_sntprintf(pszText, cchTextMax, _T("%i (%s)"),client->GetScore(false), GetResString(IDS_IDLOW));
			}
			else
				_sntprintf(pszText, cchTextMax, _T("%i"), client->GetScore(false));
			break;
         */
		case 4: {
			CString strBuffer;
			if (client->HasLowID())
			{
		     if (client->m_dwWouldHaveGottenUploadSlotIfNotLowIdTick)
				strBuffer.Format(_T("%i **** Awaited reconnect %s"),client->GetScore(false), CastSecondsToHM((::GetTickCount()-client->m_dwWouldHaveGottenUploadSlotIfNotLowIdTick)/1000));
			 else
                strBuffer.Format(_T("%i (%s)"),client->GetScore(false), GetResString(IDS_IDLOW));
			}
			else
				strBuffer.Format(_T("%i"),client->GetScore(false));

			const uint8 iSuperiorStates = client->GetSuperiorClientStates();
			if(iSuperiorStates & SCS_PBF_SUI)
				strBuffer.Append(_T(", PBF"));
			_tcsncpy(pszText, strBuffer, cchTextMax);
			break;
		}
		// ZZUL-TRA :: PaybackFirst :: End

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
//>>> WiZaRd::Show ModVer
		case 10:
			_tcsncpy(pszText, client->DbgGetFullClientSoftVer(), cchTextMax);
			break;
//<<< WiZaRd::Show ModVer
	// ZZUL-TRA :: TotalUpDown :: Start
		case 11: {
			if (client->Credits())
			{
				const uint64 uUploadedTotal = client->Credits()->GetUploadedTotal();
				const uint64 uDownloadedTotal = client->Credits()->GetDownloadedTotal();
				if (uUploadedTotal > 0 || uDownloadedTotal > 0)
					_sntprintf(pszText, cchTextMax, L"%s/%s", CastItoXBytes(uUploadedTotal), CastItoXBytes(uDownloadedTotal));
			}
			break;
		}
		// ZZUL-TRA :: TotalUpDown :: End
//Download-Status
	case 12: //Yun.SF3 remote queue status
					{	
                        CString buffer;
						int qr = client->GetRemoteQueueRank();
						if (client->GetDownloadDatarate() > 0){
							buffer.Format(_T("%.1f %s"),(float)client->GetDownloadDatarate()/1024, GetResString(IDS_KBYTESPERSEC));
						}
						else if (qr)
								buffer.Format(_T("QR: %u"),qr);
						
						else if(client->IsRemoteQueueFull())
							buffer.Format(_T("%s"),GetResString(IDS_QUEUEFULL));
						else
							buffer.Format(_T("%s"),GetResString(IDS_UNKNOWN));
					_tcsncpy(pszText, buffer, cchTextMax);
					}
					break;
//Download-Status
// ZZUL-TRA :: IP2Country :: Start
		case 13:
			_tcsncpy(pszText, client->GetCountryName(), cchTextMax);
			break;
		// ZZUL-TRA :: IP2Country :: End	
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
			if (file1 != NULL && file2 != NULL) {
				if(item1->GetPowerShared() == true && item2->GetPowerShared() == false)
                    iResult = 1;
                else if(item1->GetPowerShared() == false && item2->GetPowerShared() == true)
                    iResult = -1;
			    else
				iResult = (file1->GetUpPriority() == PR_VERYLOW ? -1 : file1->GetUpPriority()) - (file2->GetUpPriority() == PR_VERYLOW ? -1 : file2->GetUpPriority());
			} else if (file1 == NULL)
				iResult = 1;
			else
				iResult = -1;
			break;
		}

		case 3:
			iResult = CompareUnsigned(item1->GetScore(false, false, true), item2->GetScore(false, false, true));
			break;

		case 4: {
				CKnownFile* file1 = theApp.sharedfiles->GetFileByID(item1->GetUploadFileID());
				CKnownFile* file2 = theApp.sharedfiles->GetFileByID(item2->GetUploadFileID());
				if( (file1 != NULL) && (file2 != NULL)) {
					if(item1->GetPowerShared() == true && item2->GetPowerShared() == false)
						iResult = 1;
					else if(item1->GetPowerShared() == false && item2->GetPowerShared() == true)
						iResult = -1;
					else if(item1->GetPowerShared() == true && item2->GetPowerShared() == true)
						iResult = ((file1->GetUpPriority()==PR_VERYLOW) ? -1 : file1->GetUpPriority()) - ((file2->GetUpPriority()==PR_VERYLOW) ? -1 : file2->GetUpPriority());
					else
						iResult = 0;
				} else if( file1 == NULL )
					iResult = 1;
				else
					iResult = -1;

				if(iResult == 0) {
			iResult = CompareUnsigned(item1->GetScore(false), item2->GetScore(false));
				}
			}
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
			iResult = CompareUnsigned(item1->GetUpPartCount(), item2->GetUpPartCount());
			break;

//>>> WiZaRd::Show ModVer
		case 10:
			//Proper sorting ;)
			iResult = item1->GetClientSoft() - item2->GetClientSoft();
			if(iResult == 0)
				iResult = CompareLocaleStringNoCase(item1->DbgGetFullClientSoftVer(), item2->DbgGetFullClientSoftVer());
			break;
//<<< WiZaRd::Show ModVer
// ZZUL-TRA :: TotalUpDown :: Start
		case 11:
			if (!item1->Credits())
				iResult = -1;
			else if (!item2->Credits())
				iResult = 1;
			else
				iResult = CompareUnsigned64(item1->Credits()->GetDownloadedTotal(), item2->Credits()->GetDownloadedTotal());
			break;
		// ZZUL-TRA :: TotalUpDown :: End
//Download-Status
        case 12:
			iResult=CompareUnsigned(item1->GetRemoteQueueRank(), item2->GetRemoteQueueRank());
			break;
//Download-Status
// ZZUL-TRA :: IP2Country :: Start
	case 13:
			iResult = CompareLocaleStringNoCase(item1->GetCountryName(true), item2->GetCountryName(true));
			break;
			// ZZUL-TRA :: IP2Country :: End
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
	ClientMenu.AppendMenu(MF_STRING | (client ? MF_ENABLED : MF_GRAYED), MP_DETAIL, GetResString(IDS_SHOWDETAILS)/*, _T("CLIENTDETAILS")*/);
	ClientMenu.SetDefaultItem(MP_DETAIL);
        ClientMenu.AppendMenu(MF_SEPARATOR);
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && !client->IsFriend()) ? MF_ENABLED : MF_GRAYED), MP_ADDFRIEND, GetResString(IDS_ADDFRIEND)/*, _T("ADDFRIEND")*/);
		
	// <CB Mod : Friend Management : Easy Remove>
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

	if (thePrefs.IsExtControlsEnabled())
		ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->IsBanned()) ? MF_ENABLED : MF_GRAYED), MP_UNBAN, GetResString(IDS_UNBAN));
	if (Kademlia::CKademlia::IsRunning() && !Kademlia::CKademlia::IsConnected())
		ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetKadPort()!=0 && client->GetKadVersion() > 1) ? MF_ENABLED : MF_GRAYED), MP_BOOT, GetResString(IDS_BOOTSTRAP));

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

	// <CB Mod : Friend Management : Easy FriendSlot>
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

void CQueueListCtrl::AddClient(/*const*/ CUpDownClient *client, bool resetclient)
{
	if (resetclient && client){
		client->SetWaitStartTime();
		client->SetAskedCount(1);
    } else if( client ) {
        // Clients that have been put back "first" on queue (that is, they
        // get to keep its waiting time since before they started upload), are
        // recognized by having an ask count of 0.
        client->SetAskedCount(0);
	}

	if (!theApp.emuledlg->IsRunning())
		return;
	if (thePrefs.IsQueueListDisabled())
		return;

	int iItemCount = GetItemCount();
	int iItem = InsertItem(LVIF_TEXT | LVIF_PARAM, iItemCount, LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)client);
	Update(iItem);
	theApp.emuledlg->transferwnd->UpdateListCount(CTransferDlg::wnd2OnQueue, iItemCount + 1);
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
		theApp.emuledlg->transferwnd->UpdateListCount(CTransferDlg::wnd2OnQueue);
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
		if (   !theApp.emuledlg->IsRunning() // Don't do anything if the app is shutting down - can cause unhandled exceptions
			|| !thePrefs.GetUpdateQueueList()
			|| theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd
			|| !theApp.emuledlg->transferwnd->GetQueueList()->IsWindowVisible() )
			return;

		const CUpDownClient* update = theApp.uploadqueue->GetNextClient(NULL);
		while( update )
		{
			theApp.emuledlg->transferwnd->GetQueueList()->RefreshClient(update);
			update = theApp.uploadqueue->GetNextClient(update);
		}
	}
	CATCH_DFLT_EXCEPTIONS(_T("CQueueListCtrl::QueueUpdateTimer"))
}