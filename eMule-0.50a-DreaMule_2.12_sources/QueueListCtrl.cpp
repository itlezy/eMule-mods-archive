//this file is part of eMule
//Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "TransferWnd.h"
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

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



IMPLEMENT_DYNAMIC(CQueueListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CQueueListCtrl, CMuleListCtrl)
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOLORCHANGE()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
END_MESSAGE_MAP()

CQueueListCtrl::CQueueListCtrl()
	: CListCtrlItemWalk(this)
{
	SetGeneralPurposeFind(true, false);

	// Barry - Refresh the queue every 10 secs
	VERIFY( (m_hTimer = ::SetTimer(NULL, NULL, 10000, QueueUpdateTimer)) != NULL );
	if (thePrefs.GetVerbose() && !m_hTimer)
		AddDebugLogLine(true,_T("Failed to create 'queue list control' timer - %s"),GetErrorMessage(GetLastError()));
}

void CQueueListCtrl::Init()
{
	SetName(_T("QueueListCtrl"));
	CImageList ilDummyImageList; //dummy list for getting the proper height of listview entries
	ilDummyImageList.Create(1, theApp.GetSmallSytemIconSize().cy,theApp.m_iDfltImageListColorFlags|ILC_MASK, 1, 1); 
	SetImageList(&ilDummyImageList, LVSIL_SMALL);
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	ilDummyImageList.Detach();

	SetExtendedStyle(LVS_EX_FULLROWSELECT);
	InsertColumn(0,GetResString(IDS_QL_USERNAME),LVCFMT_LEFT,150,0);
	InsertColumn(1,GetResString(IDS_FILE),LVCFMT_LEFT,275,1);
	InsertColumn(2,GetResString(IDS_FILEPRIO),LVCFMT_LEFT,110,2);
	InsertColumn(3,GetResString(IDS_QL_RATING),LVCFMT_LEFT,60,3);
	InsertColumn(4,GetResString(IDS_SCORE),LVCFMT_LEFT,60,4);
	InsertColumn(5,GetResString(IDS_ASKED),LVCFMT_LEFT,60,5);
	InsertColumn(6,GetResString(IDS_LASTSEEN),LVCFMT_LEFT,110,6);
	InsertColumn(7,GetResString(IDS_ENTERQUEUE),LVCFMT_LEFT,110,7);
	InsertColumn(8,GetResString(IDS_BANNED),LVCFMT_LEFT,60,8);
	InsertColumn(9,GetResString(IDS_UPSTATUS),LVCFMT_LEFT,100,9);
	InsertColumn(10,GetResString(IDS_CD_CSOFT), LVCFMT_LEFT, 90, 10);	//Xman version see clientversion in every window
	InsertColumn(11, GetResString(IDS_UPDOWNUPLOADLIST), LVCFMT_LEFT, 90, 11); //Xman show complete up/down in queuelist

	SetAllIcons();
	Localize();
	LoadSettings();

	//Xman client percentage
	CFont* pFont = GetFont();
	LOGFONT lfFont = {0};
	pFont->GetLogFont(&lfFont);
	lfFont.lfHeight = 11;
	m_fontBoldSmaller.CreateFontIndirect(&lfFont);
	//Xman end

	// Barry - Use preferred sort order from preferences
	SetSortArrow();
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0:100));
}

CQueueListCtrl::~CQueueListCtrl()
{
	if (m_hTimer)
		VERIFY( ::KillTimer(NULL, m_hTimer) );
}

void CQueueListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
}

void CQueueListCtrl::SetAllIcons()
{
	imagelist.DeleteImageList();
	imagelist.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	imagelist.SetBkColor(CLR_NONE);
	//Xman Show correct Icons	
	imagelist.Add(CTempIconLoader(_T("ClientDefault")));		//0
	imagelist.Add(CTempIconLoader(_T("ClientDefaultPlus")));	//1
	imagelist.Add(CTempIconLoader(_T("ClientEDonkey")));		//2
	imagelist.Add(CTempIconLoader(_T("ClientEDonkeyPlus")));	//3
	imagelist.Add(CTempIconLoader(_T("ClientCompatible")));		//4
	imagelist.Add(CTempIconLoader(_T("ClientCompatiblePlus")));	//5
	imagelist.Add(CTempIconLoader(_T("ClientFriend")));			//6
	imagelist.Add(CTempIconLoader(_T("ClientFriendPlus")));		//7
	imagelist.Add(CTempIconLoader(_T("ClientMLDonkey")));		//8
	imagelist.Add(CTempIconLoader(_T("ClientMLDonkeyPlus")));	//9
	imagelist.Add(CTempIconLoader(_T("ClientEDonkeyHybrid")));	//10
	imagelist.Add(CTempIconLoader(_T("ClientEDonkeyHybridPlus")));//11
	imagelist.Add(CTempIconLoader(_T("ClientShareaza")));		//12
	imagelist.Add(CTempIconLoader(_T("ClientShareazaPlus")));	//13
	imagelist.Add(CTempIconLoader(_T("ClientAMule")));			//14
	imagelist.Add(CTempIconLoader(_T("ClientAMulePlus")));		//15
	imagelist.Add(CTempIconLoader(_T("ClientLPhant")));			//16
	imagelist.Add(CTempIconLoader(_T("ClientLPhantPlus")));		//17
	imagelist.Add(CTempIconLoader(_T("LEECHER")));				//18 //Xman Anti-Leecher

	//Xman friend visualization
	imagelist.Add(CTempIconLoader(_T("ClientFriendSlotOvl"))); //19
	//Xman end


	//Xman end	
	imagelist.SetOverlayImage(imagelist.Add(CTempIconLoader(_T("ClientSecureOvl"))), 1);
	imagelist.SetOverlayImage(imagelist.Add(CTempIconLoader(_T("OverlayObfu"))), 2);
	imagelist.SetOverlayImage(imagelist.Add(CTempIconLoader(_T("OverlaySecureObfu"))), 3);
}

void CQueueListCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;

	if(pHeaderCtrl->GetItemCount() != 0) {
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

	}
}

void CQueueListCtrl::AddClient(/*const*/ CUpDownClient* client, bool resetclient)
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
	int iItem = InsertItem(LVIF_TEXT|LVIF_PARAM,iItemCount,LPSTR_TEXTCALLBACK,0,0,0,(LPARAM)client);
	Update(iItem);
	theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2OnQueue, iItemCount+1);
}

void CQueueListCtrl::RemoveClient(const CUpDownClient* client)
{
	if (!theApp.emuledlg->IsRunning())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	int result = FindItem(&find);
	if (result != -1){
		DeleteItem(result);
		theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2OnQueue);
	}
}

void CQueueListCtrl::RefreshClient(const CUpDownClient* client)
{
	// There is some type of timing issue here.. If you click on item in the queue or upload and leave
	// the focus on it when you exit the cient, it breaks on line 854 of emuleDlg.cpp.. 
	// I added this IsRunning() check to this function and the DrawItem method and
	// this seems to keep it from crashing. This is not the fix but a patch until
	// someone points out what is going wrong.. Also, it will still assert in debug mode..
	if (!theApp.emuledlg->IsRunning())
		return;

	//MORPH START - SiRoB, Don't Refresh item if not needed
	if( theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd || theApp.emuledlg->transferwnd->queuelistctrl.IsWindowVisible() == false )
		return;
	//MORPH END   - SiRoB, Don't Refresh item if not needed

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	int result = FindItem(&find);
	if (result != -1)
		Update(result);
}

#define DLC_DT_TEXT (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS)

void CQueueListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	if (!lpDrawItemStruct->itemData)
		return;

	//MORPH START - Added by SiRoB, Don't draw hidden Rect
	RECT clientRect;
	GetClientRect(&clientRect);
	RECT cur_rec = lpDrawItemStruct->rcItem;
	if (cur_rec.top >= clientRect.bottom || cur_rec.bottom <= clientRect.top)
		return;
	//MORPH END   - Added by SiRoB, Don't draw hidden Rect

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
	COLORREF crOldBackColor = odc->GetBkColor(); //Xman PowerRelease //Xman show LowIDs

	const CUpDownClient* client = (CUpDownClient*)lpDrawItemStruct->itemData;
	CMemDC dc(odc, &lpDrawItemStruct->rcItem);
	CFont* pOldFont = dc.SelectObject(thePrefs.UseNarrowFont() ? &m_fontNarrow : GetFont()); //Xman narrow font at transferwindow
	//CRect cur_rec(lpDrawItemStruct->rcItem); //MORPH - Moved by SiRoB, Don't draw hidden Rect
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
				////Xman Show correct Icons
				uint8 image;
				if (client->IsFriend())
					image = 6;
				else if (client->GetClientSoft() == SO_EDONKEYHYBRID){
					image = 10;
				}
				else if (client->GetClientSoft() == SO_EDONKEY){
					image = 2;
				}
				else if (client->GetClientSoft() == SO_MLDONKEY){
					image = 8;
				}
				else if (client->GetClientSoft() == SO_SHAREAZA){
					image = 12;
				}
				else if (client->GetClientSoft() == SO_AMULE){
					image = 14;
				}
				else if (client->GetClientSoft() == SO_LPHANT){
					image = 16;
				}
				else if (client->ExtProtocolAvailable()){
					image = 4;
				}
				else{
					image = 0;
				}
				//Xman Anti-Leecher
				if(client->IsLeecher()>0)
					image=18;
				else
				//Xman end
				if (((client->credits)?client->credits->GetScoreRatio(client):0) > 1)
					image++;
				//Xman end

				uint32 nOverlayImage = 0;
				if ((client->Credits() && client->Credits()->GetCurrentIdentState(client->GetIP()) == IS_IDENTIFIED))
					nOverlayImage |= 1;
				//Xman changed: display the obfuscation icon for all clients which enabled it
				if(client->IsObfuscatedConnectionEstablished() 
					|| (!(client->socket != NULL && client->socket->IsConnected())
					&& (client->SupportsCryptLayer() && thePrefs.IsClientCryptLayerSupported() && (client->RequestsCryptLayer() || thePrefs.IsClientCryptLayerRequested()))))
					nOverlayImage |= 2;

					POINT point = {cur_rec.left, cur_rec.top+1};
					imagelist.Draw(dc,image, point, ILD_NORMAL | INDEXTOOVERLAYMASK(nOverlayImage));
					Sbuffer = client->GetUserName();

					//Xman friend visualization
					if (client->IsFriend() && client->GetFriendSlot())
						imagelist.Draw(dc,19, point, ILD_NORMAL);
					//Xman end

					//EastShare Start - added by AndCycle, IP to Country, modified by Commander
					if(theApp.ip2country->ShowCountryFlag() ){
						cur_rec.left+=20;
						POINT point2= {cur_rec.left,cur_rec.top+1};
						theApp.ip2country->GetFlagImageList()->Draw(dc, client->GetCountryFlagIndex(), point2, ILD_NORMAL);
					}
					//EastShare End - added by AndCycle, IP to Country

					cur_rec.left +=20;
					dc.DrawText(Sbuffer,Sbuffer.GetLength(),&cur_rec,DLC_DT_TEXT);
					cur_rec.left -=20;

					//EastShare Start - added by AndCycle, IP to Country
					if(theApp.ip2country->ShowCountryFlag() ){
						cur_rec.left-=20;
					}
					//EastShare End - added by AndCycle, IP to Country

					break;
				}
				case 1:
					if(file)
					{
						Sbuffer = file->GetFileName();
						//Xman PowerRelease
						if(file->GetUpPriority()==PR_POWER)
							dc.SetBkColor(RGB(255,225,225));
						//Xman end
					}
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
						   //Xman PowerRelease
							case PR_POWER:
								Sbuffer = GetResString(IDS_POWERRELEASE);
								break;
							//Xman end

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
							Sbuffer.Format(_T("%i (%s)"),client->GetScore(false), GetResString(IDS_IDLOW));
					}
					//Xman uploading problem client
					else if(client->isupprob && client->m_bAddNextConnect)
					{
						if(client->socket && client->socket->IsConnected())
							Sbuffer.Format(_T("%i #~~"),client->GetScore(false));
						else
							Sbuffer.Format(_T("%i ~~~"),client->GetScore(false));
					}
					//Xman end
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
				case 9:
					if( client->GetUpPartCount()){
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
						//Xman end
						cur_rec.bottom++;
						cur_rec.top--;
					}
					break;
				//Xman version see clientversion in every window
				case 10:
					Sbuffer.Format(_T("%s"), client->DbgGetFullClientSoftVer()); //Xman // Maella -Support for tag ET_MOD_VERSION 0x55
					if(client->HasLowID()) dc.SetBkColor(RGB(255,250,200));//Xman show LowIDs
					break;
				//Xman end

				//Xman show complete up/down in queuelist
				case 11:
					if(client->Credits() )
					{
						Sbuffer.Format(_T("%s/ %s"), CastItoXBytes(client->credits->GetUploadedTotal()), CastItoXBytes(client->credits->GetDownloadedTotal()));
					}
					break;
				//Xman end

		   	}
			if( iColumn != 9 && iColumn != 0)
				dc.DrawText(Sbuffer,Sbuffer.GetLength(),&cur_rec,DLC_DT_TEXT);
			dc.SetBkColor(crOldBackColor); //Xman PowerRelease //Xman show LowIDs
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

void CQueueListCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
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
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && !client->IsFriend()) ? MF_ENABLED : MF_GRAYED), MP_ADDFRIEND, GetResString(IDS_ADDFRIEND), _T("ADDFRIEND"));
	ClientMenu.AppendMenu(MF_STRING | (client && client->IsFriend() ? MF_ENABLED : MF_GRAYED), MP_REMOVEFRIEND, GetResString(IDS_REMOVEFRIEND), _T("DELETEFRIEND"));
	ClientMenu.AppendMenu(MF_STRING | (client && client->IsFriend() ? MF_ENABLED : MF_GRAYED), MP_FRIENDSLOT, GetResString(IDS_FRIENDSLOT), _T("FRIENDSLOT"));
	ClientMenu.CheckMenuItem(MP_FRIENDSLOT, (client && client->GetFriendSlot()) ? MF_CHECKED : MF_UNCHECKED);
	ClientMenu.AppendMenu(MF_SEPARATOR); 
	//Xman end

	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED), MP_MESSAGE, GetResString(IDS_SEND_MSG), _T("SENDMESSAGE"));
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED), MP_SHOWLIST, GetResString(IDS_VIEWFILES), _T("VIEWFILES"));
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->IsBanned()) ? MF_ENABLED : MF_GRAYED), MP_UNBAN, GetResString(IDS_UNBAN));
	if (Kademlia::CKademlia::IsRunning() && !Kademlia::CKademlia::IsConnected())
		ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetKadPort()!=0) ? MF_ENABLED : MF_GRAYED), MP_BOOT, GetResString(IDS_BOOTSTRAP));
	ClientMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED), MP_FIND, GetResString(IDS_FIND), _T("Search"));
	// - show requested files (sivka/Xman)
	ClientMenu.AppendMenu(MF_SEPARATOR); 
	ClientMenu.AppendMenu(MF_STRING,MP_LIST_REQUESTED_FILES, GetResString(IDS_LISTREQUESTED), _T("FILEREQUESTED")); 
	//Xman end

	GetPopupMenuPos(*this, point);
	ClientMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
}

BOOL CQueueListCtrl::OnCommand(WPARAM wParam,LPARAM /*lParam*/)
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
					theApp.friendlist->RemoveAllFriendSlots();
					if( !IsAlready )
						client->SetFriendSlot(true);
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
				if (client->GetKadPort())
					Kademlia::CKademlia::Bootstrap(ntohl(client->GetIP()), client->GetKadPort(), (client->GetKadVersion() > 1));
				break;
			// - show requested files (sivka/Xman)
			case MP_LIST_REQUESTED_FILES: { 
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

void CQueueListCtrl::OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult){

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// Barry - Store sort order in preferences
	// Determine ascending based on whether already sorted on this column
	bool sortAscending = (GetSortItem()!= pNMListView->iSubItem) ? true : !GetSortAscending();

	// Sort table
	UpdateSortHistory(pNMListView->iSubItem + (sortAscending ? 0:100), 100);
	SetSortArrow(pNMListView->iSubItem, sortAscending);
	SortItems(SortProc, pNMListView->iSubItem + (sortAscending ? 0:100));

	*pResult = 0;
}

int CQueueListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	const CUpDownClient* item1 = (CUpDownClient*)lParam1;
	const CUpDownClient* item2 = (CUpDownClient*)lParam2;
	int iResult=0;
	switch(lParamSort){
		case 0: 
			if(item1->GetUserName() && item2->GetUserName())
				iResult=CompareLocaleStringNoCase(item1->GetUserName(), item2->GetUserName());
			else if(item1->GetUserName())
				iResult=1;
			else
				iResult=-1;
			break;
		case 100:
			if(item2->GetUserName() && item1->GetUserName())
				iResult=CompareLocaleStringNoCase(item2->GetUserName(), item1->GetUserName());
			else if(item2->GetUserName())
				iResult=1;
			else
				iResult=-1;
			break;
		
		case 1: {
			CKnownFile* file1 = theApp.sharedfiles->GetFileByID(item1->GetUploadFileID());
			CKnownFile* file2 = theApp.sharedfiles->GetFileByID(item2->GetUploadFileID());
			if( (file1 != NULL) && (file2 != NULL))
				iResult=CompareLocaleStringNoCase(file1->GetFileName(), file2->GetFileName());
			else if( file1 == NULL )
				iResult=1;
			else
				iResult=-1;
			break;
		}
		case 101: {
			CKnownFile* file1 = theApp.sharedfiles->GetFileByID(item1->GetUploadFileID());
			CKnownFile* file2 = theApp.sharedfiles->GetFileByID(item2->GetUploadFileID());
			if( (file1 != NULL) && (file2 != NULL))
				iResult=CompareLocaleStringNoCase(file2->GetFileName(), file1->GetFileName());
			else if( file1 == NULL )
				iResult=1;
			else
				iResult=-1;
			break;
		}
		
		case 2: {
			CKnownFile* file1 = theApp.sharedfiles->GetFileByID(item1->GetUploadFileID());
			CKnownFile* file2 = theApp.sharedfiles->GetFileByID(item2->GetUploadFileID());
			if( (file1 != NULL) && (file2 != NULL))
				iResult=((file1->GetUpPriority()==PR_VERYLOW) ? -1 : file1->GetUpPriority()) - ((file2->GetUpPriority()==PR_VERYLOW) ? -1 : file2->GetUpPriority());
			else if( file1 == NULL )
				iResult=1;
			else
				iResult=-1;
			break;
		}
		case 102:{
			CKnownFile* file1 = theApp.sharedfiles->GetFileByID(item1->GetUploadFileID());
			CKnownFile* file2 = theApp.sharedfiles->GetFileByID(item2->GetUploadFileID());
			if( (file1 != NULL) && (file2 != NULL))
				iResult=((file2->GetUpPriority()==PR_VERYLOW) ? -1 : file2->GetUpPriority()) - ((file1->GetUpPriority()==PR_VERYLOW) ? -1 : file1->GetUpPriority());
			else if( file1 == NULL )
				iResult=1;
			else
				iResult=-1;
			break;
		}

		case 3: 
			iResult=CompareUnsigned(item1->GetScore(false,false,true), item2->GetScore(false,false,true));
			break;
		case 103: 
			iResult=CompareUnsigned(item2->GetScore(false,false,true), item1->GetScore(false,false,true));
			break;

		case 4: 
			iResult=CompareUnsigned(item1->GetScore(false), item2->GetScore(false));
			break;
		case 104: 
			iResult=CompareUnsigned(item2->GetScore(false), item1->GetScore(false));
			break;

		case 5: 
			iResult=item1->GetAskedCount() - item2->GetAskedCount();
			break;
		case 105: 
			iResult=item2->GetAskedCount() - item1->GetAskedCount();
			break;
		
		case 6: 
			iResult=item1->GetLastUpRequest() - item2->GetLastUpRequest();
			break;
		case 106: 
			iResult=item2->GetLastUpRequest() - item1->GetLastUpRequest();
			break;
		
		case 7: 
			iResult=item1->GetWaitStartTime() - item2->GetWaitStartTime();
			break;
		case 107: 
			iResult=item2->GetWaitStartTime() - item1->GetWaitStartTime();
			break;
		
		case 8: 
			iResult=item1->IsBanned() - item2->IsBanned();
			break;
		case 108: 
			iResult=item2->IsBanned() - item1->IsBanned();
			break;
		
		case 9: 
			iResult=item1->GetUpPartCount() - item2->GetUpPartCount();
			break;
		case 109: 
			iResult=item2->GetUpPartCount() - item1->GetUpPartCount();
			break;
		//Xman version see clientversion in every window
		case 10:
			// Maella -Support for tag ET_MOD_VERSION 0x55-
			if( item1->GetClientSoft() == item2->GetClientSoft() )
				if(item2->GetVersion() == item1->GetVersion() && item1->GetClientSoft() == SO_EMULE){
					iResult= item2->DbgGetFullClientSoftVer().CompareNoCase( item1->DbgGetFullClientSoftVer());
				}
				else {
					iResult= item2->GetVersion() - item1->GetVersion();
				}
			else
				iResult= item1->GetClientSoft() - item2->GetClientSoft();
			break;
		case 110:
			if( item1->GetClientSoft() == item2->GetClientSoft() )
				if(item2->GetVersion() == item1->GetVersion() && item2->GetClientSoft() == SO_EMULE){
					iResult= item1->DbgGetFullClientSoftVer().CompareNoCase( item2->DbgGetFullClientSoftVer());
				}
				else {
					iResult= item1->GetVersion() - item2->GetVersion();
				}
			else
				iResult= item2->GetClientSoft() - item1->GetClientSoft();
			break;
		//Xman end

		//Xman show complete up/down in queuelist
		case 11:
			if(item1->Credits() && item2->Credits())
			{
				iResult=CompareUnsigned64(item1->credits->GetUploadedTotal(), item2->credits->GetUploadedTotal());
			}
			else
				iResult=0;
			break;
		case 111:
			if(item1->Credits() && item2->Credits())
			{
				iResult=CompareUnsigned64(item2->credits->GetUploadedTotal(), item1->credits->GetUploadedTotal());
			}
			else
				iResult=0;
			break;
		//Xman end

		default:
			iResult=0;
			break;
	}
	// SLUGFILLER: multiSort remove - handled in parent class
	/*
	int dwNextSort;
	//call secondary sortorder, if this one results in equal
	//(Note: yes I know this call is evil OO wise, but better than changing a lot more code, while we have only one instance anyway - might be fixed later)
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->transferwnd->queuelistctrl.GetNextSortOrder(lParamSort)) != (-1)){
		iResult= SortProc(lParam1, lParam2, dwNextSort);
	}
	*/
	return iResult;

}

//Xman faster Updating of Queuelist
void CQueueListCtrl::UpdateAll()
{
	if(theApp.emuledlg->IsRunning())
	{
		RedrawItems(0,GetItemCount());
		//CWnd::UpdateWindow(); //not needed because of sorting
		// Sort table
		SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0:100));
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
			|| !theApp.emuledlg->transferwnd->queuelistctrl.IsWindowVisible() )
			return;

		//Xman faster Updating of Queuelist
		if (theApp.emuledlg->transferwnd->queuelistctrl.GetItemCount()>1)
		{

			theApp.emuledlg->transferwnd->queuelistctrl.UpdateAll();
		}
		//Xman end
	}
	CATCH_DFLT_EXCEPTIONS(_T("CQueueListCtrl::QueueUpdateTimer"))
		// Maella -Code Improvement-
		// Remark: The macro CATCH_DFLT_EXCEPTIONS will not catch all types of exception.
		//         The exceptions thrown in callback function are not intercepted by the dbghelp.dll (e.g. eMule Dump, crashRpt, etc...)
		catch(...) {
			if(theApp.emuledlg != NULL)
				AddLogLine(true, _T("Unknown exception in %s"), __FUNCTION__);
		}
		// Maella end
}
//Xman end

void CQueueListCtrl::ShowQueueClients()
{
	DeleteAllItems(); 
	CUpDownClient* update = theApp.uploadqueue->GetNextClient(NULL);
	while( update )
	{
		AddClient(update, false);
		update = theApp.uploadqueue->GetNextClient(update);
	}
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

void CQueueListCtrl::OnNMDblclk(NMHDR* /*pNMHDR*/, LRESULT *pResult)
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

void CQueueListCtrl::OnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

	if (theApp.emuledlg->IsRunning()){
		// Although we have an owner drawn listview control we store the text for the primary item in the listview, to be
		// capable of quick searching those items via the keyboard. Because our listview items may change their contents,
		// we do this via a text callback function. The listview control will send us the LVN_DISPINFO notification if
		// it needs to know the contents of the primary item.
		//
		// But, the listview control sends this notification all the time, even if we do not search for an item. At least
		// this notification is only sent for the visible items and not for all items in the list. Though, because this
		// function is invoked *very* often, no *NOT* put any time consuming code here in.

		if (pDispInfo->item.mask & LVIF_TEXT){
			const CUpDownClient* pClient = reinterpret_cast<CUpDownClient*>(pDispInfo->item.lParam);
			if (pClient != NULL){
				switch (pDispInfo->item.iSubItem){
					case 0:
						if (pClient->GetUserName() != NULL && pDispInfo->item.cchTextMax > 0){
							_tcsncpy(pDispInfo->item.pszText, pClient->GetUserName(), pDispInfo->item.cchTextMax);
							pDispInfo->item.pszText[pDispInfo->item.cchTextMax-1] = _T('\0');
						}
						break;
					default:
						// shouldn't happen
						pDispInfo->item.pszText[0] = _T('\0');
						break;
				}
			}
		}
	}
	*pResult = 0;
}
