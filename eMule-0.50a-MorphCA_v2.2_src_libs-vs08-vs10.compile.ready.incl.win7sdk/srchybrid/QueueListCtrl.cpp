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
	//MORPH START - Added by SiRoB, Client Software
	InsertColumn(10,GetResString(IDS_CD_CSOFT),LVCFMT_LEFT,100);
	//MORPH END - Added by SiRoB, Client Software
#ifdef COMMUNITY	
	// Mighty Knife: Community affiliation
	if (thePrefs.IsCommunityEnabled ())
		InsertColumn(14,GetResString(IDS_COMMUNITY),LVCFMT_LEFT,100);
	else
		InsertColumn(14,GetResString(IDS_COMMUNITY),LVCFMT_LEFT,100, -1, true);
	// [end] Mighty Knife
#endif
	// EastShare - Added by Pretender, Friend Tab
	InsertColumn(11,GetResString(IDS_FRIENDLIST),LVCFMT_LEFT,75);
	// EastShare - Added by Pretender, Friend Tab

	// Commander - Added: IP2Country column - Start
	if (thePrefs.GetIP2CountryNameMode() == IP2CountryName_DISABLE)
		InsertColumn(12,GetResString(IDS_COUNTRY),LVCFMT_LEFT,100, -1, true);
	else
		InsertColumn(12,GetResString(IDS_COUNTRY),LVCFMT_LEFT,100);
	// Commander - Added: IP2Country column - End
	InsertColumn(13,GetResString(IDS_UPL_DL),LVCFMT_LEFT,100); //Total up down

	SetAllIcons();
	Localize();
	LoadSettings();
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

	//MORPH START - Added by SiRoB, Client Software
	strRes = GetResString(IDS_CD_CSOFT);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(10, &hdi);
	//MORPH END - Added by SiRoB, Client Software
#ifdef COMMUNITY	
	// Mighty Knife: Community affiliation
	strRes = GetResString(IDS_COMMUNITY);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(14, &hdi);
	// [end] Mighty Knife
#endif
	// EastShare - Added by Pretender, Friend Tab
	strRes = GetResString(IDS_FRIENDLIST);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(11, &hdi);
	// EastShare - Added by Pretender, Friend Tab

	// Commander - Added: IP2Country column - Start
	strRes = GetResString(IDS_COUNTRY);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(12, &hdi);
	// Commander - Added: IP2Country column - End

    	//MORPH START - Modified by IceCream, Total up down
	strRes = GetResString(IDS_UPL_DL);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(13, &hdi);
	//MORPH END - Modified by IceCream, Total up down
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
#ifdef CLIENTANALYZER
	morphca = m_ImageList.Add(CTempIconLoader(_T("MORPH")));
	badguy = m_ImageList.Add(CTempIconLoader(_T("BADGUY")));
#endif
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("ClientSecureOvl"))), 1);
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("OverlayObfu"))), 2);
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("OverlaySecureObfu"))), 3);
    //share visible +
	m_overlayimages.DeleteImageList ();
	m_overlayimages.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	m_overlayimages.SetBkColor(CLR_NONE);
        m_overlayimages.Add(CTempIconLoader(_T("SHAREVISIBLE")));
	//share visible -
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

	//MORPH START - Changed by Stulle, Visual Studio 2010 Compatibility
	/*
	CMemDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	*/
#ifdef EVENLINE	
        CMemoryDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	InitItemMemDC(dc, lpDrawItemStruct->rcItem, /*client->HasLowID() ? RGB(255,250,200) :*/ ((lpDrawItemStruct->itemID % 2)?m_crEvenLine:m_crWindow), lpDrawItemStruct->itemState); //Xman show LowIDs
#else
	CMemoryDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	//MORPH END   - Changed by Stulle, Visual Studio 2010 Compatibility
	BOOL bCtrlFocused;
	InitItemMemDC(dc, lpDrawItemStruct, bCtrlFocused);
#endif
	CRect cur_rec(lpDrawItemStruct->rcItem);
	CRect rcClient;
	GetClientRect(&rcClient);
	const CUpDownClient *client = (CUpDownClient *)lpDrawItemStruct->itemData;

//leecher greyed out +
    if(client->IsBadGuy())
      dc.SetTextColor(::GetSysColor(COLOR_GRAYTEXT));
	//leecher greyed out -

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
                        if (client->IsFriend())
							iImage = 4;
#ifdef CLIENTANALYZER
                                          else if (morphca != -1 && client->IsMorphCA())
					         iImage = morphca;
					    else if (badguy != -1 && client->IsBadGuy())
					         iImage = badguy;
#endif     
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
						int iIconPosY = (cur_rec.Height() > 16) ? ((cur_rec.Height() - 16) / 2) : 1;
						POINT point = { cur_rec.left, cur_rec.top + iIconPosY };
						m_ImageList.Draw(dc, iImage, point, ILD_NORMAL | INDEXTOOVERLAYMASK(nOverlayImage));
                        
						//share visible +
                        if (client->GetUserName() && client->GetViewSharedFilesSupport())
						    m_overlayimages.Draw(dc,0, point, ILD_TRANSPARENT);
                        //share visible -
						
						//EastShare Start - added by AndCycle, IP to Country, modified by Commander
						if(theApp.ip2country->ShowCountryFlag() && IsColumnHidden(12)){
							cur_rec.left+=20;
							POINT point2= {cur_rec.left,cur_rec.top+1};
							//theApp.ip2country->GetFlagImageList()->DrawIndirect(dc, client->GetCountryFlagIndex(), point2, CSize(18,16), CPoint(0,0), ILD_NORMAL);
							theApp.ip2country->GetFlagImageList()->DrawIndirect(&theApp.ip2country->GetFlagImageDrawParams(dc,client->GetCountryFlagIndex(),point2));
							cur_rec.left += sm_iLabelOffset;
						}
						//EastShare End - added by AndCycle, IP to Country

						cur_rec.left += 16 + sm_iLabelOffset;
	
                     // colors +
                        if (client->GetUserName()){
						COLORREF crOldBackColor = dc.GetBkColor();
                        if (client->IsFriend() && client->GetFriendSlot())
			             	dc.SetBkColor(RGB(185,220,255)); //friendslot blue
						else if(client->IsFriend())
							dc.SetBkColor(RGB(200,250,200)); //friend green
#ifdef COMMUNITY	
                        else if(client->IsCommunity())
							dc.SetBkColor(RGB(250,230,200)); //community orange                     
#endif
						_tcsncpy(szItem, client->GetUserName(), _countof(szItem) - 1);
						szItem[_countof(szItem) - 1] = _T('\0');
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT);
						dc.SetBkColor(crOldBackColor);
					    }else
					//colors -

						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
						cur_rec.left -= 16;
						cur_rec.right -= sm_iSubItemInset;

						//EastShare Start - added by AndCycle, IP to Country
						if(theApp.ip2country->ShowCountryFlag() && IsColumnHidden(12)){
							cur_rec.left-=20;
							cur_rec.left -= sm_iLabelOffset;
						}
						//EastShare End - added by AndCycle, IP to Country

						break;
					}

					case 9:
						if (client->GetUpPartCount()) {
							cur_rec.bottom--;
							cur_rec.top++;
							client->DrawUpStatusBar(dc, &cur_rec, false, thePrefs.UseFlatBar());
							// MORPH START
							//MORPH START - Adde by SiRoB, Optimization requpfile
							/*
							const CKnownFile *file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
							*/
							const CKnownFile* file = client->CheckAndGetReqUpFile();
							//MORPH END   - Adde by SiRoB, Optimization requpfile
							if (file)  // protect against deleted file
								client->DrawCompletedPercent(dc,&cur_rec); //Fafner: client percentage - 080325
							// MORPH END
							cur_rec.bottom++;
							cur_rec.top--;
						}
						break;
 #ifdef LOWIDCOLOR
                //colors +
                    case 10:{
					COLORREF crOldBackColor = dc.GetBkColor();
					if(client->HasLowID()) 
						dc.SetBkColor(RGB(255,250,200));//lowid yellow
					dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT);
					dc.SetBkColor(crOldBackColor);
					break;
				    }
               //colors -
#endif
                 //colors +
                   case 1:{
					const CKnownFile *file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
					if (file)
					{
						COLORREF crOldBackColor = dc.GetBkColor();
                        //PBF +
                        if(file && client->IsMoreUpThanDown(file))
					        dc.SetBkColor(RGB(230,230,200));
						//PBF -
						else if(file->GetPowerShared())
							dc.SetBkColor(RGB(255,225,225));
						_tcsncpy(szItem, file->GetFileName(), _countof(szItem) - 1);
						szItem[_countof(szItem) - 1] = _T('\0');
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT);
						dc.SetBkColor(crOldBackColor);
					}
				}
					break;
				//colors -

					// Commander - Added: IP2Country column - Start
					case 13:
						if(theApp.ip2country->ShowCountryFlag()){
							POINT point2= {cur_rec.left,cur_rec.top+1};
							//theApp.ip2country->GetFlagImageList()->DrawIndirect(dc, client->GetCountryFlagIndex(), point2, CSize(18,16), CPoint(0,0), ILD_NORMAL);
							theApp.ip2country->GetFlagImageList()->DrawIndirect(&theApp.ip2country->GetFlagImageDrawParams(dc,client->GetCountryFlagIndex(),point2));
							cur_rec.left+=20;
						}
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
						if(theApp.ip2country->ShowCountryFlag()){
							cur_rec.left-=20;
						}
						break;
					// Commander - Added: IP2Country column - End

                    // X-Ray :: TotalUpDown :: Start
					case 14:{
						if(client->Credits())
						{
							const COLORREF crOldTxtColor = dc->GetTextColor();
							const uint64 uUploadedTotal = client->Credits()->GetUploadedTotal();
							const uint64 uDownloadedTotal = client->Credits()->GetDownloadedTotal();

							if (uUploadedTotal > 0 || uDownloadedTotal > 0)
							{
								if (uUploadedTotal > uDownloadedTotal)
									dc->SetTextColor(RGB(200, 0, 0));
								else
									dc->SetTextColor(RGB(0, 200, 0));
							}
							dc->DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
							dc->SetTextColor(crOldTxtColor);
						}
						break;
					}
					// X-Ray :: TotalUpDown :: End

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
		m_updatethread->AddItemUpdated((LPARAM)client); //MORPH - UpdateItemThread
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
			//MORPH START - Adde by SiRoB, Optimization requpfile
			/*
			const CKnownFile *file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
			*/
			const CKnownFile* file = client->CheckAndGetReqUpFile();
			//MORPH END   - Adde by SiRoB, Optimization requpfile
			_tcsncpy(pszText, file != NULL ? file->GetFileName() : _T(""), cchTextMax);
			break;
		}

		case 2: {
			//MORPH START - Adde by SiRoB, Optimization requpfile
			/*
			const CKnownFile *file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
			*/
			const CKnownFile* file = client->CheckAndGetReqUpFile();
			//MORPH END   - Adde by SiRoB, Optimization requpfile
			if (file)
			{
				//MORPH START - various features
				/*
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
				*/
				CString Sbuffer;
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

				//Morph Start - added by AndCycle, Equal Chance For Each File
				if(thePrefs.IsEqualChanceEnable()){
					if(file->GetPowerShared()){
						Sbuffer.Append(_T(" "));
						Sbuffer.Append(file->statistic.GetEqualChanceValueString());
					}
					else{
						Sbuffer = file->statistic.GetEqualChanceValueString();
					}
				}
				//Morph End - added by AndCycle, Equal Chance For Each File

				//MORPH START - Added by SiRoB, ZZ Upload System
				if(file->GetPowerShared()) {
					CString tempString = GetResString(IDS_POWERSHARE_PREFIX);
					tempString.Append(_T(","));
					tempString.Append(Sbuffer);
					Sbuffer.Empty(); //MORPH - HotFix by SiRoB, ZZ Upload System
					Sbuffer = tempString;
				}
				//MORPH END - Added by SiRoB, ZZ Upload System
				_tcsncpy(pszText, Sbuffer, cchTextMax);
				//MORPH END   - various features
			}
			break;
		}
		
		case 3:
			_sntprintf(pszText, cchTextMax, _T("%i"), client->GetScore(false, false, true));
			break;
		
		case 4:
			//MORPH START - various features
			/*
			if (client->HasLowID()) {
				if (client->m_bAddNextConnect)
					_sntprintf(pszText, cchTextMax, _T("%i ****"),client->GetScore(false));
				else
					_sntprintf(pszText, cchTextMax, _T("%i (%s)"),client->GetScore(false), GetResString(IDS_IDLOW));
			}
			else
				_sntprintf(pszText, cchTextMax, _T("%i"), client->GetScore(false));
			*/
			{
				CString Sbuffer;
				if (client->HasLowID()){
					//MORPH START - ZZ LowID handling code
					if (client->m_dwWouldHaveGottenUploadSlotIfNotLowIdTick)
						Sbuffer.Format(GetResString(IDS_UP_LOWID_AWAITED),client->GetScore(false), CastSecondsToHM((::GetTickCount()-client->m_dwWouldHaveGottenUploadSlotIfNotLowIdTick)/1000));
					else
						Sbuffer.Format(GetResString(IDS_UP_LOWID2),client->GetScore(false));
					//MORPH END   - ZZ LowID handling code
				}
				else
					Sbuffer.Format(_T("%i"),client->GetScore(false));

				//MORPH START - Adde by SiRoB, Optimization requpfile
				/*
				const CKnownFile *file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
				*/
				const CKnownFile* file = client->CheckAndGetReqUpFile();
				//MORPH END   - Adde by SiRoB, Optimization requpfile
				//EastShare START - Added by TAHO, Pay Back First
				if(file && client->IsMoreUpThanDown(file)) {
					CString tempStr;
					tempStr.Format(_T("%s %s"), _T("PBF"), Sbuffer);
					Sbuffer = tempStr;
				}
				//EastShare END - Added by TAHO, Pay Back First

				//Morph Start - added by AndCycle, show out keep full chunk transfer
				if(client->GetQueueSessionUp() > 0){
					Sbuffer.Append(_T(" F"));
				}
				//Morph End - added by AndCycle, show out keep full chunk transfer
				_tcsncpy(pszText, Sbuffer, cchTextMax);
				//MORPH END   - various features
			}
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
			//MORPH START - Changed by SiRoB, Code Optimization
			/*
			_tcsncpy(pszText, GetResString(client->IsBanned() ? IDS_YES : IDS_NO), cchTextMax);
			*/
			_tcsncpy(pszText, GetResString((client->GetUploadState() == US_BANNED) ? IDS_YES : IDS_NO), cchTextMax);
			//MORPH END   - Changed by SiRoB, Code Optimization
			break;

		case 9:
			_tcsncpy(pszText, GetResString(IDS_UPSTATUS), cchTextMax);
			break;

		//MORPH START - Added by SiRoB, Client Software
		case 10:
			_tcsncpy(pszText, client->DbgGetFullClientSoftVer(), cchTextMax);
			break;
		//MORPH END - Added by SiRoB, Client Software
#ifdef COMMUNITY	
		// Mighty Knife: Community affiliation
		case 14:
			_tcsncpy(pszText, client->IsCommunity () ? GetResString(IDS_YES) : _T(""), cchTextMax);
			break;
		// [end] Mighty Knife
#endif
		// EastShare - Added by Pretender, Friend Tab
		case 11:
			_tcsncpy(pszText, client->IsFriend() ? GetResString(IDS_YES) : _T(""), cchTextMax);
			break;
		// EastShare - Added by Pretender, Friend Tab
		// Commander - Added: IP2Country column - Start
		case 12:
			_tcsncpy(pszText, client->GetCountryName(), cchTextMax);
			break;
		// Commander - Added: IP2Country column - End

	   //MORPH START - Added By Yun.SF3, Upload/Download
		case 13: //LSD Total UP/DL
			{
				if (client->Credits()){
					_sntprintf(pszText, cchTextMax, _T("%s/%s"),
					CastItoXBytes(client->Credits()->GetUploadedTotal(),false,false),
					CastItoXBytes(client->Credits()->GetDownloadedTotal(),false,false));
					//(float)client->Credits()->GetScoreRatio() );
				}
				else
					_sntprintf(pszText, cchTextMax, _T("%s/%s"),
						_T("?"),_T("?"));//,"?" );
				break;	
			}
		//MORPH END - Added By Yun.SF3, Upload/Download
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
			//MORPH START - Adde by SiRoB, Optimization requpfile
			/*
			const CKnownFile *file1 = theApp.sharedfiles->GetFileByID(item1->GetUploadFileID());
			const CKnownFile *file2 = theApp.sharedfiles->GetFileByID(item2->GetUploadFileID());
			*/
			const CKnownFile *file1 = item1->CheckAndGetReqUpFile();
			const CKnownFile *file2 = item2->CheckAndGetReqUpFile();
			//MORPH END   - Adde by SiRoB, Optimization requpfile

			if (file1 != NULL && file2 != NULL)
				iResult = CompareLocaleStringNoCase(file1->GetFileName(), file2->GetFileName());
			else if (file1 == NULL)
				iResult = 1;
			else
				iResult = -1;
			break;
		}

		case 2: {
			//MORPH START - Changed by SiRoB, ZZ Upload System
			/*
			const CKnownFile *file1 = theApp.sharedfiles->GetFileByID(item1->GetUploadFileID());
			const CKnownFile *file2 = theApp.sharedfiles->GetFileByID(item2->GetUploadFileID());
			if (file1 != NULL && file2 != NULL)
				iResult = (file1->GetUpPriority() == PR_VERYLOW ? -1 : file1->GetUpPriority()) - (file2->GetUpPriority() == PR_VERYLOW ? -1 : file2->GetUpPriority());
	   		*/
			//MORPH START - Added by SiRoB, Optimization requpfile
			/*
			const CKnownFile *file1 = theApp.sharedfiles->GetFileByID(item1->GetUploadFileID());
			const CKnownFile *file2 = theApp.sharedfiles->GetFileByID(item2->GetUploadFileID());
			*/
			CKnownFile *file1 = item1->CheckAndGetReqUpFile();
			CKnownFile *file2 = item2->CheckAndGetReqUpFile();
			//MORPH END   - Added by SiRoB, Optimization requpfile
			if( (file1 != NULL) && (file2 != NULL)){
				//only file priority
				if (file1->GetPowerShared()) ++iResult;
 				if (file2->GetPowerShared()) --iResult;
				//Morph Start - added by AndCycle, Equal Chance For Each File
				if(iResult == 0 && (!thePrefs.IsEqualChanceEnable() || (file1->GetPowerShared() && file2->GetPowerShared())))
					iResult = ((file1->GetUpPriority()==PR_VERYLOW) ? -1 : file1->GetUpPriority()) - ((file2->GetUpPriority()==PR_VERYLOW) ? -1 : file2->GetUpPriority());
				if (iResult == 0 && file1 != file2 && thePrefs.IsEqualChanceEnable()){
					iResult =
						file1->statistic.GetEqualChanceValue() < file2->statistic.GetEqualChanceValue() ? 1 :
						file1->statistic.GetEqualChanceValue() > file2->statistic.GetEqualChanceValue() ? -1 :
						0;
				}
				//Morph End - added by AndCycle, Equal Chance For Each File
			}
			//MORPH END - Changed by SiRoB, ZZ Upload System
			else if (file1 == NULL)
				iResult = 1;
			else
				iResult = -1;
			break;
		}

		case 3:
			iResult = CompareUnsigned(item1->GetScore(false, false, true), item2->GetScore(false, false, true));
			break;

		case 4:
		//MORPH START - Changed by SiRoB, ZZ Upload System
		/*
			iResult = CompareUnsigned(item1->GetScore(false), item2->GetScore(false));
		*/
		{
//			//MORPH START - Added by SiRoB, Optimization requpfile
//			/*
//			const CKnownFile *file1 = theApp.sharedfiles->GetFileByID(item1->GetUploadFileID());
//			const CKnownFile *file2 = theApp.sharedfiles->GetFileByID(item2->GetUploadFileID());
//			*/
//			const CKnownFile *file1 = item1->CheckAndGetReqUpFile();
//			const CKnownFile *file2 = item2->CheckAndGetReqUpFile();
//			//MORPH END   - Added by SiRoB, Optimization requpfile
			
//			if( (file1 != NULL) && (file2 != NULL)){

				//Morph - modified by AndCycle, definitely a correct compare to show queue
				CUpDownClient *lClient = (CUpDownClient*)item2, *rClient = (CUpDownClient*)item1;

				uint32 lScore = lClient->GetScore(false), rScore = rClient->GetScore(false);
				iResult = 
					theApp.uploadqueue->RightClientIsBetter(lClient, lScore, rClient, rScore) ? 1 :
					theApp.uploadqueue->RightClientIsBetter(rClient, rScore, lClient, lScore) ? -1 :
					0;

//			}
//			else if( file1 == NULL )
//				iResult = 1;
//			else
//				iResult = -1;
		}
		//MORPH END - Changed by SiRoB, ZZ Upload System
			break;

		case 5:
			iResult = CompareUnsigned(item1->GetAskedCount(), item2->GetAskedCount());
			break;

		case 6:
			iResult = CompareUnsigned(item1->GetLastUpRequest(), item2->GetLastUpRequest());
			break;

		case 7:
 			//EastShare START - Modified by TAHO, modified SUQWT
			/*
			iResult = CompareUnsigned(item1->GetWaitStartTime(), item2->GetWaitStartTime());
			*/
			{
				sint64 time1 = item1->GetWaitStartTime();
				sint64 time2 = item2->GetWaitStartTime();
				if ( time1 == time2 ) {
					iResult = 0;
				} else if ( time1 > time2 ) {
					iResult = 1;
				} else {
					iResult = -1;
				}
			}
			//EastShare END - Modified by TAHO, modified SUQWT
			break;

		case 8:
			//MORPH - Changed by SiRoB, Code Optimization
			/*
			iResult = item1->IsBanned() - item2->IsBanned();
			*/
			iResult=(item1->GetUploadState() == US_BANNED) - (item2->GetUploadState() == US_BANNED);
			break;
		
		case 9: 
			//Fafner: client percentage - 080325
			/*
			iResult = CompareUnsigned(item1->GetUpPartCount(), item2->GetUpPartCount());
			*/
			if (item1->GetCompletedPercent() == item2->GetCompletedPercent())
				iResult=0;
			else
				iResult=item1->GetCompletedPercent() > item2->GetCompletedPercent()?1:-1;
			break;

		//MORPH START - Modified by SiRoB, Client Software
		case 10:
			/*
			iResult=item2->GetClientSoftVer().CompareNoCase(item1->GetClientSoftVer());
			*/
			if (item1->GetClientSoft() == item2->GetClientSoft())
				if (item2->GetVersion() == item1->GetVersion() && (item1->GetClientSoft() == SO_EMULE || item1->GetClientSoft() == SO_AMULE)){
					iResult= CompareOptLocaleStringNoCase(item2->DbgGetFullClientSoftVer(), item1->DbgGetFullClientSoftVer());
				}
				else {
					iResult= item1->GetVersion() - item2->GetVersion();
				}
			else
				iResult=-(item1->GetClientSoft() - item2->GetClientSoft());
			break;
		//MORPH END - Modified by SiRoB, Client Software
#ifdef COMMUNITY	
		// Mighty Knife: Community affiliation
		case 14:
			iResult=item1->IsCommunity() - item2->IsCommunity();
			break;
		// [end] Mighty Knife
#endif
		// EastShare - Added by Pretender, Friend Tab
		case 11:
			iResult=item1->IsFriend() - item2->IsFriend();
			break;
		// EastShare - Added by Pretender, Friend Tab
		// Commander - Added: IP2Country column - Start
		case 12:
			if(item1->GetCountryName(true) && item2->GetCountryName(true))
				iResult=CompareLocaleStringNoCase(item1->GetCountryName(true), item2->GetCountryName(true));
			else if(item1->GetCountryName(true))
				iResult=1;
			else
				iResult=-1;
			break;
		// Commander - Added: IP2Country column - End

       //MORPH START - Added By Yun.SF3, Upload/Download
		case 13: // UP-DL TOTAL
			iResult=CompareUnsigned64(item2->Credits()->GetUploadedTotal(), item1->Credits()->GetUploadedTotal());
			break;
		//MORPH END - Added By Yun.SF3, Upload/Download
	}

	if (lParamSort >= 100)
		iResult = -iResult;

	// SLUGFILLER: multiSort remove - handled in parent class
	/*
	//call secondary sortorder, if this one results in equal
	int dwNextSort;
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->transferwnd->GetQueueList()->GetNextSortOrder(lParamSort)) != -1)
		iResult = SortProc(lParam1, lParam2, dwNextSort);
	*/

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
	ClientMenu.AppendMenu(MF_STRING | (client ? MF_ENABLED : MF_GRAYED), MP_DETAIL, GetResString(IDS_SHOWDETAILS));
	ClientMenu.SetDefaultItem(MP_DETAIL);
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && !client->IsFriend()) ? MF_ENABLED : MF_GRAYED), MP_ADDFRIEND, GetResString(IDS_ADDFRIEND));
	//MORPH START - Added by SiRoB, Friend Addon
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->IsFriend()) ? MF_ENABLED : MF_GRAYED), MP_REMOVEFRIEND, GetResString(IDS_REMOVEFRIEND));
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->IsFriend()) ? MF_ENABLED  | ((!client->HasLowID() && client->IsFriend() && client->GetFriendSlot())?MF_CHECKED : MF_UNCHECKED) : MF_GRAYED), MP_FRIENDSLOT, GetResString(IDS_FRIENDSLOT));
	//MORPH END - Added by SiRoB, Friend Addon
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED), MP_MESSAGE, GetResString(IDS_SEND_MSG));
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED), MP_SHOWLIST, GetResString(IDS_VIEWFILES));
	if (thePrefs.IsExtControlsEnabled())
		ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->IsBanned()) ? MF_ENABLED : MF_GRAYED), MP_UNBAN, GetResString(IDS_UNBAN));
	if (Kademlia::CKademlia::IsRunning() && !Kademlia::CKademlia::IsConnected())
		ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetKadPort()!=0 && client->GetKadVersion() > 1) ? MF_ENABLED : MF_GRAYED), MP_BOOT, GetResString(IDS_BOOTSTRAP));
	ClientMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED), MP_FIND, GetResString(IDS_FIND));
	//MORPH START - Added by Yun.SF3, List Requested Files
	ClientMenu.AppendMenu(MF_SEPARATOR); // Added by sivka
	ClientMenu.AppendMenu(MF_STRING | (client ? MF_ENABLED : MF_GRAYED),MP_LIST_REQUESTED_FILES, GetResString(IDS_LISTREQUESTED)); // Added by sivka
	//MORPH END - Added by Yun.SF3, List Requested Files
	
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
  			//MORPH START - Added by SiRoB, Friend Addon
			case MP_REMOVEFRIEND:{//LSD
				if (client && client->IsFriend())
				{
					theApp.friendlist->RemoveFriend(client->m_Friend);
					Update(iSel);
				}
				break;
			}
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
					Update(iSel);
				}
				//MORPH END - Modified by SIRoB, Added by Yun.SF3, ZZ Upload System
				break;
			}
			//Xman end
			//MORPH END  - Added by SiRoB, Friend Addon
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
			//MORPH START - Added by Yun.SF3, List Requested Files
			case MP_LIST_REQUESTED_FILES: { // added by sivka
				if (client != NULL)
				{
					client->ShowRequestedFiles(); //Changed by SiRoB
				}
				break;
			}
			//MORPH START - Added by Yun.SF3, List Requested Files
		}
	}
	return true;
}

void CQueueListCtrl::AddClient(/*const*/ CUpDownClient *client, bool resetclient)
{
	if (resetclient && client){
		// EastShare START - Marked by TAHO, modified SUQWT
		/*
		client->SetWaitStartTime();
		*/
		// EastShare END - Marked by TAHO, modified SUQWT
		client->SetAskedCount(1);
	//MORPH START - Added by SiRoB, ZZ Upload System
	} else if( client ) {
		// Clients that have been put back "first" on queue (that is, they
		// get to keep its waiting time since before they started upload), are
		// recognized by having an ask count of 0.
		client->SetAskedCount(0);
	//MORPH END - Added by SiRoB, ZZ Upload System
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
	//MORPH END- UpdateItemThread
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
