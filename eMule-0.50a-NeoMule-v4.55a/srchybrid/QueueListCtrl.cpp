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
#include "ListenSocket.h" // NEO: UPC - [UploadingProblemClient] <-- Xanatos --
#include "Neo/NeoPreferences.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --
#include "Neo\GUI\XPMenu\MenuXP.h" // NEO: NMX - [NeoMenuXP] <-- Xanatos --

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
#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
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

	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

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
	InsertColumn(10,GetResString(IDS_CD_CSOFT),LVCFMT_LEFT,150,10); // NEO: MIDI - [ModIDInfo] <-- Xanatos --
	// NEO: MOD - [TransferCollumn] -- Xanatos -->
	InsertColumn(11,GetResString(IDS_CL_TRANSFUP),LVCFMT_LEFT,150,11);
	InsertColumn(12,GetResString(IDS_CL_TRANSFDOWN),LVCFMT_LEFT,150,12);
	// NEO: MOD END <-- Xanatos --
#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
	InsertColumn(13,GetResString(IDS_X_COUNTRY),LVCFMT_LEFT,100,13);
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --

	SetAllIcons();
	Localize();
	LoadSettings();
	// Barry - Use preferred sort order from preferences
	SetSortArrow();
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0:100));
#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
	if (NeoPrefs.GetIP2CountryNameMode() == IP2CountryName_DISABLE)
		HideColumn (13);
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --
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

// NEO: NCI - [NewClientIcons] -- Xanatos -->
enum EnumClientListIcons
{
	CL_BLANK = 0,

	CL_CLIENT_CDONKEY,
	CL_CLIENT_EDONKEY,
	CL_CLIENT_EMULE,
	CL_CLIENT_HYBRID,
	CL_CLIENT_MLDONKEY,
	CL_CLIENT_OLDEMULE,
	CL_CLIENT_SHAREAZA,
	CL_CLIENT_UNKNOWN,
	CL_CLIENT_XMULE,
	CL_CLIENT_AMULE,
	CL_CLIENT_LPHANT,
	CL_CLIENT_EMULEPLUS,
	CL_CLIENT_TRUSTYFILES,
	CL_CLIENT_HYDRANODE,
	CL_SERVER,

	CL_CLIENT_MOD,
	CL_CLIENT_MOD_NEO,
	CL_CLIENT_MOD_MORPH,
	CL_CLIENT_MOD_SCARANGEL,
	CL_CLIENT_MOD_STULLE,
	CL_CLIENT_MOD_MAXMOD,
	CL_CLIENT_MOD_XTREME,
	CL_CLIENT_MOD_EASTSHARE,
	CL_CLIENT_MOD_IONIX,
	CL_CLIENT_MOD_CYREX,
	CL_CLIENT_MOD_NEXTEMF,
	
	CL_CREDIT_UP,
	CL_CREDIT_DOWN,
	CL_CREDIT_UP_DOWN,

	CL_FRIEND_CLIENT,
	CL_FRIEND_SLOT_CLIENT,

	CL_ARGOS_BANNED,

	CL_SHOW_SHARED,
	CL_LOW_ID_CLIENT,
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	CL_LANCAST_CLIENT,
#endif //LANCAST // NEO: NLC END

	CL_SECUREHASH_BLUE,
	CL_SECUREHASH_GREEN,
	CL_SECUREHASH_YELLOW,
	CL_SECUREHASH_RED,

	CL_ICON_OBFU,
};
// NEO: NCI END <-- Xanatos --

void CQueueListCtrl::SetAllIcons()
{
	imagelist.DeleteImageList();
	imagelist.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	imagelist.SetBkColor(CLR_NONE);
	//imagelist.Add(CTempIconLoader(_T("ClientEDonkey")));
	//imagelist.Add(CTempIconLoader(_T("ClientCompatible")));
	//imagelist.Add(CTempIconLoader(_T("ClientEDonkeyPlus")));
	//imagelist.Add(CTempIconLoader(_T("ClientCompatiblePlus")));
	//imagelist.Add(CTempIconLoader(_T("Friend")));
	//imagelist.Add(CTempIconLoader(_T("ClientMLDonkey")));
	//imagelist.Add(CTempIconLoader(_T("ClientMLDonkeyPlus")));
	//imagelist.Add(CTempIconLoader(_T("ClientEDonkeyHybrid")));
	//imagelist.Add(CTempIconLoader(_T("ClientEDonkeyHybridPlus")));
	//imagelist.Add(CTempIconLoader(_T("ClientShareaza")));
	//imagelist.Add(CTempIconLoader(_T("ClientShareazaPlus")));
	//imagelist.Add(CTempIconLoader(_T("ClientAMule")));
	//imagelist.Add(CTempIconLoader(_T("ClientAMulePlus")));
	//imagelist.Add(CTempIconLoader(_T("ClientLPhant")));
	//imagelist.Add(CTempIconLoader(_T("ClientLPhantPlus")));
	//imagelist.SetOverlayImage(imagelist.Add(CTempIconLoader(_T("ClientSecureOvl"))), 1);
	//imagelist.SetOverlayImage(imagelist.Add(CTempIconLoader(_T("OverlayObfu"))), 2);
	//imagelist.SetOverlayImage(imagelist.Add(CTempIconLoader(_T("OverlaySecureObfu"))), 3);

	// NEO: NCI - [NewClientIcons] -- Xanatos -->
	imagelist.Add(CTempIconLoader(_T("EMPTY")));

	imagelist.Add(CTempIconLoader(_T("CLIENT_CDONKEY")));
	imagelist.Add(CTempIconLoader(_T("CLIENT_EDONKEY")));
	imagelist.Add(CTempIconLoader(_T("CLIENT_EMULE")));
	imagelist.Add(CTempIconLoader(_T("CLIENT_HYBRID")));
	imagelist.Add(CTempIconLoader(_T("CLIENT_MLDONKEY")));
	imagelist.Add(CTempIconLoader(_T("CLIENT_OLDEMULE")));
	imagelist.Add(CTempIconLoader(_T("CLIENT_SHAREAZA")));
	imagelist.Add(CTempIconLoader(_T("CLIENT_UNKNOWN")));
	imagelist.Add(CTempIconLoader(_T("CLIENT_XMULE")));
	imagelist.Add(CTempIconLoader(_T("CLIENT_AMULE")));
	imagelist.Add(CTempIconLoader(_T("CLIENT_LPHANT")));
	imagelist.Add(CTempIconLoader(_T("CLIENT_EMULEPLUS")));
	imagelist.Add(CTempIconLoader(_T("CLIENT_TRUSTYFILES")));
	imagelist.Add(CTempIconLoader(_T("CLIENT_HYDRANODE")));
	imagelist.Add(CTempIconLoader(_T("SERVER")));

	imagelist.Add(CTempIconLoader(_T("CLIENT_MOD")));
	imagelist.Add(CTempIconLoader(_T("CLIENT_NEO")));
	imagelist.Add(CTempIconLoader(_T("CLIENT_MORPH")));
	imagelist.Add(CTempIconLoader(_T("CLIENT_SCARANGEL")));
	imagelist.Add(CTempIconLoader(_T("CLIENT_STULLE")));
	imagelist.Add(CTempIconLoader(_T("CLIENT_MAXMOD")));
	imagelist.Add(CTempIconLoader(_T("CLIENT_XTREME")));
	imagelist.Add(CTempIconLoader(_T("CLIENT_EASTSHARE")));
	imagelist.Add(CTempIconLoader(_T("CLIENT_IONIX")));
	imagelist.Add(CTempIconLoader(_T("CLIENT_CYREX")));
	imagelist.Add(CTempIconLoader(_T("CLIENT_NEXTEMF")));
	
	imagelist.Add(CTempIconLoader(_T("CREDIT_UP")));
	imagelist.Add(CTempIconLoader(_T("CREDIT_DOWN")));
	imagelist.Add(CTempIconLoader(_T("CREDIT_UP_DOWN")));

	imagelist.Add(CTempIconLoader(_T("FRIEND_CLIENT")));
	imagelist.Add(CTempIconLoader(_T("FRIEND_SLOT_CLIENT")));

	imagelist.Add(CTempIconLoader(_T("ARGOS_BANNED")));

	imagelist.Add(CTempIconLoader(_T("SHOW_SHARED")));
	imagelist.Add(CTempIconLoader(_T("LOW_ID_CLIENT")));
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	imagelist.Add(CTempIconLoader(_T("LANCAST_CLIENT")));
#endif //LANCAST // NEO: NLC END

	imagelist.Add(CTempIconLoader(_T("SECUREHASH_BLUE")));
	imagelist.Add(CTempIconLoader(_T("SECUREHASH_GREEN")));
	imagelist.Add(CTempIconLoader(_T("SECUREHASH_YELLOW")));
	imagelist.Add(CTempIconLoader(_T("SECUREHASH_RED")));

	imagelist.Add(CTempIconLoader(_T("OverlayObfu")));
	// NEO: NCI END <-- Xanatos --
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

		// NEO: MIDI - [ModIDInfo] -- Xanatos -->
		strRes = GetResString(IDS_CD_CSOFT);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(10, &hdi);
		// NEO: MIDI END <-- Xanatos --

		// NEO: MOD - [TransferCollumn] -- Xanatos -->
		strRes = GetResString(IDS_CL_TRANSFUP);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(11, &hdi);

		strRes = GetResString(IDS_CL_TRANSFDOWN);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(12, &hdi);
		// NEO: MOD END <-- Xanatos --

#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
		strRes = GetResString(IDS_X_COUNTRY);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(13, &hdi);
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --
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
	if (result != -1) {
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
	CMemDC dc(odc, &lpDrawItemStruct->rcItem);
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
					//uint8 image;
					//if (client->IsFriend())
					//	image = 4;
					//else if (client->GetClientSoft() == SO_EDONKEYHYBRID){
					//	if (client->credits->GetScoreRatio(client->GetIP()) > 1)
					//		image = 8;
					//	else
					//		image = 7;
					//}
					//else if (client->GetClientSoft() == SO_MLDONKEY){
					//	if (client->credits->GetScoreRatio(client->GetIP()) > 1)
					//		image = 6;
					//	else
					//		image = 5;
					//}
					//else if (client->GetClientSoft() == SO_SHAREAZA){
					//	if(client->credits->GetScoreRatio(client->GetIP()) > 1)
					//		image = 10;
					//	else
					//		image = 9;
					//}
					//else if (client->GetClientSoft() == SO_AMULE){
					//	if(client->credits->GetScoreRatio(client->GetIP()) > 1)
					//		image = 12;
					//	else
					//		image = 11;
					//}
					//else if (client->GetClientSoft() == SO_LPHANT){
					//	if(client->credits->GetScoreRatio(client->GetIP()) > 1)
					//		image = 14;
					//	else
					//		image = 13;
					//}
					//else if (client->ExtProtocolAvailable()){
					//	if(client->credits->GetScoreRatio(client->GetIP()) > 1)
					//		image = 3;
					//	else
					//		image = 1;
					//}
					//else{
					//	if (client->credits->GetScoreRatio(client->GetIP()) > 1)
					//		image = 2;
					//	else
					//		image = 0;
					//}
					//uint32 nOverlayImage = 0;
					//if ((client->Credits() && client->Credits()->GetCurrentIdentState(client->GetIP()) == IS_IDENTIFIED))
					//	nOverlayImage |= 1;
					//if (client->IsObfuscatedConnectionEstablished())
					//	nOverlayImage |= 2;
					//POINT point = {cur_rec.left, cur_rec.top+1};
					//imagelist.Draw(dc,image, point, ILD_NORMAL | INDEXTOOVERLAYMASK(nOverlayImage));

					// NEO: NCI - [NewClientIcons] -- Xanatos -->
					POINT point = {cur_rec.left, cur_rec.top+1};
					
					switch (client->GetClientSoft())
					{
						case SO_URL:		imagelist.Draw(dc, CL_SERVER, point, ILD_NORMAL);			break;
						case SO_CDONKEY:	imagelist.Draw(dc, CL_CLIENT_CDONKEY,point, ILD_NORMAL);	break;
						case SO_AMULE: 		imagelist.Draw(dc, CL_CLIENT_AMULE, point, ILD_NORMAL);		break;
						case SO_LPHANT:		imagelist.Draw(dc, CL_CLIENT_LPHANT, point, ILD_NORMAL);	break;
						case SO_EMULEPLUS:	imagelist.Draw(dc, CL_CLIENT_EMULEPLUS, point, ILD_NORMAL);	break;
						case SO_XMULE:		imagelist.Draw(dc, CL_CLIENT_XMULE, point, ILD_NORMAL);		break;
						case SO_HYDRANODE:	imagelist.Draw(dc, CL_CLIENT_HYDRANODE, point, ILD_NORMAL);	break;
						case SO_TRUSTYFILES:imagelist.Draw(dc, CL_CLIENT_TRUSTYFILES, point, ILD_NORMAL);break;
						case SO_SHAREAZA:	imagelist.Draw(dc, CL_CLIENT_SHAREAZA, point, ILD_NORMAL);	break;
						case SO_EDONKEYHYBRID:	imagelist.Draw(dc, CL_CLIENT_HYBRID, point, ILD_NORMAL);break;
						case SO_MLDONKEY:	imagelist.Draw(dc, CL_CLIENT_MLDONKEY, point, ILD_NORMAL);	break;
						case SO_EMULE:
							if(const EModClient Mod = client->GetMod())
								imagelist.Draw(dc, CL_CLIENT_MOD + ((int)Mod - 1), point, ILD_NORMAL);
							else
								imagelist.Draw(dc, CL_CLIENT_EMULE, point, ILD_NORMAL);
							break;
						case SO_OLDEMULE:	imagelist.Draw(dc, CL_CLIENT_OLDEMULE, point, ILD_NORMAL);	break;
						case SO_EDONKEY:	imagelist.Draw(dc, CL_CLIENT_EDONKEY, point, ILD_NORMAL);	break;
						case SO_UNKNOWN:	imagelist.Draw(dc, CL_CLIENT_UNKNOWN, point, ILD_NORMAL);	break;
						default:			imagelist.Draw(dc, CL_CLIENT_UNKNOWN, point, ILD_NORMAL);
					}

					if (client->IsBanned()) 
						imagelist.Draw(dc, CL_ARGOS_BANNED, point, ILD_TRANSPARENT);
					else if (client->GetFriendSlot())
						imagelist.Draw(dc, CL_FRIEND_SLOT_CLIENT, point, ILD_TRANSPARENT);
					else if (client->IsFriend())
						imagelist.Draw(dc, CL_FRIEND_CLIENT, point, ILD_TRANSPARENT);
					else if (client->Credits()){ 
						if (client->credits->GetScoreRatio(client->GetIP()) > 1 && client->credits->GetRemoteScoreRatio() > 1)
							imagelist.Draw(dc, CL_CREDIT_UP_DOWN, point, ILD_TRANSPARENT);
						else if (client->credits->GetScoreRatio(client->GetIP()) > 1)
							imagelist.Draw(dc, CL_CREDIT_UP, point, ILD_TRANSPARENT);
						else if (client->credits->GetRemoteScoreRatio() > 1)
							imagelist.Draw(dc, CL_CREDIT_DOWN, point, ILD_TRANSPARENT);
					}

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
					if(client->IsLanClient()) 
						imagelist.Draw(dc, CL_LANCAST_CLIENT, point, ILD_TRANSPARENT);
					else // Lan Client's dont need SUI
#endif //LANCAST // NEO: NLC END
						if(theApp.clientcredits->CryptoAvailable() && client->Credits())
							switch(client->Credits()->GetCurrentIdentState(client->GetIP())){
								case IS_IDBADGUY:
								case IS_IDFAILED:		imagelist.Draw(dc, CL_SECUREHASH_RED, point, ILD_TRANSPARENT);		break;
								case IS_IDNEEDED:		imagelist.Draw(dc, CL_SECUREHASH_YELLOW, point, ILD_TRANSPARENT); 	break;
								case IS_IDENTIFIED:		imagelist.Draw(dc, CL_SECUREHASH_GREEN, point, ILD_TRANSPARENT); 	break;
								case IS_NOTAVAILABLE:	imagelist.Draw(dc, CL_SECUREHASH_BLUE, point, ILD_TRANSPARENT); 	break;
							}

					if(client->GetViewSharedFilesSupport() && client->GetClientSoft() == SO_EMULE)
						imagelist.Draw(dc, CL_SHOW_SHARED, point, ILD_TRANSPARENT);

					if(client->HasLowID())
						imagelist.Draw(dc, CL_LOW_ID_CLIENT, point, ILD_TRANSPARENT);

					if(client->IsObfuscatedConnectionEstablished())
						imagelist.Draw(dc, CL_ICON_OBFU, point, ILD_TRANSPARENT);
					// END: NCI END <-- Xanatos --

#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
					const bool bShowFlags = NeoPrefs.IsIP2CountryShowFlag() && theApp.ip2country->ShowCountryFlag()
										&& (NeoPrefs.IsIP2CountryShowFlag() == 2 || IsColumnHidden(14));
					if(bShowFlags){
						cur_rec.left+=20;
						POINT point2= {cur_rec.left,cur_rec.top+1};
						theApp.ip2country->GetFlagImageList()->DrawIndirect(dc, client->GetCountryFlagIndex(), point2, CSize(18,16), CPoint(0,0), ILD_NORMAL);
					}
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --
					Sbuffer = client->GetUserName();
					cur_rec.left +=20;
					dc.DrawText(Sbuffer,Sbuffer.GetLength(),&cur_rec,DLC_DT_TEXT);
					cur_rec.left -=20;
#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
					if(bShowFlags)
						cur_rec.left-=20;
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --
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
						// NEO: SRS - [SmartReleaseSharing] -- Xanatos -->
						if(file->IsReleasePriority()){
							CString buffer;
							buffer.Format(_T("%s (%s)"), GetResString(IDS_PRIORELEASE), Sbuffer);
							if(file->GetReleasePriority())
								buffer.AppendFormat(_T(" [%.2f]"),file->GetReleaseModifyer());
							if(file->GetPowerShared())
								buffer.AppendFormat(_T(" PS"));
							Sbuffer = buffer;
						}
						// NEO: SRS <-- Xanatos --
					}
					else
						Sbuffer = _T("?");
					break;
				case 3:
					Sbuffer.Format(_T("%i"),client->GetScore(false,false,true));
					break;
				case 4:
					if (client->HasLowID()){
						//if (client->m_bAddNextConnect)
						if (client->GetUploadState() == US_PENDING) // NEO: MOD - [NewUploadState] <-- Xanatos --
							Sbuffer.Format(_T("%i ****"),client->GetScore(false));
						else
							Sbuffer.Format(_T("%i (%s)"),client->GetScore(false), GetResString(IDS_IDLOW));
					}
					// NEO: UPC - [UploadingProblemClient] -- Xanatos -->
					else if(client->m_fUpIsProblematic)
					{
						if (client->GetUploadState() == US_PENDING) // NEO: MOD - [NewUploadState]
						//if (client->m_bAddNextConnect)
						{
							if(client->socket && client->socket->IsConnected())
								Sbuffer.Format(_T("%i #~~"),client->GetScore(false));
							else
								Sbuffer.Format(_T("%i ~~~"),client->GetScore(false));
						}
						else
							Sbuffer.Format(_T("%i (%s)"),client->GetScore(false), GetResString(IDS_X_PROBLEMATIC));
					}
					// NEO: UPC END <-- Xanatos --
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
					//if( client->GetUpPartCount()) // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
					{
						cur_rec.bottom--;
						cur_rec.top++;
						//client->DrawUpStatusBar(dc,&cur_rec,false,thePrefs.UseFlatBar());
						client->DrawUpStatusBar(dc,&cur_rec,client->GetUploadFileID(),thePrefs.UseFlatBar()); // NEO: MFSB - [MultiFileStatusBars] <-- Xanatos --
						cur_rec.bottom++;
						cur_rec.top--;
					}
					break;
				// NEO: MIDI - [ModIDInfo] -- Xanatos -->
				case 10:
					//Sbuffer.Format(_T("%s"), client->GetClientSoftVer());
					Sbuffer.Format(_T("%s"), client->DbgGetFullClientSoftVer()); 
					break;
				// NEO: MIDI END <-- Xanatos --
				// NEO: MOD - [TransferCollumn] -- Xanatos -->
				case 11:{
					if(client->credits)
						Sbuffer = CastItoXBytes(client->credits->GetUploadedTotal(), false, false);
					else
						Sbuffer.Empty();
					break;
				}
				case 12:{
					if(client->credits)
						Sbuffer = CastItoXBytes(client->credits->GetDownloadedTotal(), false, false);
					else
						Sbuffer.Empty();
					break;
				}
				// NEO: MOD END <-- Xanatos --
#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
				case 13:
					Sbuffer.Format(_T("%s"), client->GetCountryName());
					const bool bShowFlags = NeoPrefs.IsIP2CountryShowFlag() == 1 && theApp.ip2country->ShowCountryFlag();
					if(bShowFlags){
						POINT point2= {cur_rec.left,cur_rec.top+1};
						theApp.ip2country->GetFlagImageList()->DrawIndirect(dc, client->GetCountryFlagIndex(), point2, CSize(18,16), CPoint(0,0), ILD_NORMAL);
						cur_rec.left+=20;
					}
					dc->DrawText(Sbuffer,Sbuffer.GetLength(),&cur_rec,DLC_DT_TEXT);
					if(bShowFlags)
						cur_rec.left-=20;
					break;
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --
		   	}
#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry]
			if( iColumn != 9 && iColumn != 0 && iColumn != 13)
#else
			if( iColumn != 9 && iColumn != 0)
#endif // IP2COUNTRY // NEO: IP2C END
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

void CQueueListCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	const CUpDownClient* client = (iSel != -1) ? (CUpDownClient*)GetItemData(iSel) : NULL;

	//CTitleMenu ClientMenu;
	CMenuXP ClientMenu; // NEO: NMX - [NeoMenuXP] <-- Xanatos --
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
		ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetKadPort()!=0) ? MF_ENABLED : MF_GRAYED), MP_BOOT, GetResString(IDS_BOOTSTRAP));
	ClientMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED), MP_FIND, GetResString(IDS_FIND), _T("Search"));
	GetPopupMenuPos(*this, point);
	ClientMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
	VERIFY( ClientMenu.DestroyMenu() ); // NEO: FIX - [DestroyMenu] <-- Xanatos --
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
				//client->RequestSharedFileList();
				// NEO: XSF - [ExtendedSharedFiles] -- Xanatos -->
				{
					// NEO: MLD - [ModelesDialogs]
					CClientDetailDialog* dlg = new CClientDetailDialog(client, this, IDD_BROWSEFILES);
					dlg->OpenDialog(); 
					// NEO: MLD END
					//CClientDetailDialog dialog(client, this, IDD_BROWSEFILES);
					//dialog.DoModal();
				}
				// NEO: XSF END <-- Xanatos --
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
				// NEO: MLD - [ModelesDialogs] -- Xanatos -->
				CClientDetailDialog* dlg = new CClientDetailDialog(client, this);
				dlg->OpenDialog(); 
				// NEO: MLD END <-- Xanatos --
				//CClientDetailDialog dialog(client, this);
				//dialog.DoModal();
				break;
			}
			case MP_BOOT:
				if (client->GetKadPort())
					Kademlia::CKademlia::Bootstrap(ntohl(client->GetIP()), client->GetKadPort(), (client->GetKadVersion() > 1));
				break;
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
			if( (file1 != NULL) && (file2 != NULL)){
				// NEO: SRS - [SmartReleaseSharing] -- Xanatos -->
				if(file1->IsReleasePriority() != file2->IsReleasePriority())
					iResult = file1->IsReleasePriority() - file2->IsReleasePriority();
				else if(file1->GetPowerShared() != file2->GetPowerShared())
					iResult = file1->GetPowerShared() - file2->GetPowerShared();
				else if(file1->GetReleasePriority() != file2->GetReleasePriority())
					iResult = file1->GetReleasePriority() - file2->GetReleasePriority();
				else if(file1->GetReleasePriority() && file2->GetReleasePriority())
					iResult = (int)(file1->GetReleaseModifyer() - file2->GetReleaseModifyer());
				else
				// NEO: SRS <-- Xanatos --
					iResult=((file1->GetUpPriority()==PR_VERYLOW) ? -1 : file1->GetUpPriority()) - ((file2->GetUpPriority()==PR_VERYLOW) ? -1 : file2->GetUpPriority());
			}else if( file1 == NULL )
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
			//iResult=item1->GetUpPartCount() - item2->GetUpPartCount();
			// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
			{
				CKnownFile* file1 = theApp.sharedfiles->GetFileByID(item1->GetUploadFileID());
				CKnownFile* file2 = theApp.sharedfiles->GetFileByID(item2->GetUploadFileID());
				if(file1 && file2)
					iResult=CompareUnsigned(file1->GetPartCount(), file2->GetPartCount());
				else if(!file1 && !file2)
					iResult=0;
				else if(file1)
					iResult=1;
				else
					iResult=-1;
				break;
			}
			// NEO: SCFS END <-- Xanatos --
			break;
		case 109: 
			//iResult=item2->GetUpPartCount() - item1->GetUpPartCount();
			// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
			{
				CKnownFile* file1 = theApp.sharedfiles->GetFileByID(item1->GetUploadFileID());
				CKnownFile* file2 = theApp.sharedfiles->GetFileByID(item2->GetUploadFileID());
				if(file1 && file2)
					iResult=CompareUnsigned(file2->GetPartCount(), file1->GetPartCount());
				else if(!file1 && !file2)
					iResult=0;
				else if(file2)
					iResult=1;
				else
					iResult=-1;
			}
			// NEO: SCFS END <-- Xanatos --
			break;
			// NEO: MIDI - [ModIDInfo] -- Xanatos -->
		case 10:
	    case 110:
		    /*if (item1->GetClientSoft() == item2->GetClientSoft())
			    iResult=item2->GetVersion() - item1->GetVersion();
		    else 
				iResult=item1->GetClientSoft() - item2->GetClientSoft();*/
			iResult=CompareLocaleStringNoCase(item1->DbgGetFullClientSoftVer(), item2->DbgGetFullClientSoftVer()); 
			break;
			// NEO: MIDI END <-- Xanatos --
		// NEO: MOD - [TransferCollumn] -- Xanatos -->
		case 11:
		    if (item1->credits && item2->credits)
				iResult=CompareUnsigned64(item1->credits->GetUploadedTotal(), item2->credits->GetUploadedTotal());
		    else if (!item1->credits)
			    iResult=1;
		    else
			    iResult=-1;
			break;
	    case 111:
		    if (item1->credits && item2->credits)
				iResult=CompareUnsigned64(item2->credits->GetUploadedTotal(), item1->credits->GetUploadedTotal());
		    else if (!item1->credits)
			    iResult=1;
		    else
			    iResult=-1;
			break;
		case 12:
		    if (item1->credits && item2->credits)
				iResult=CompareUnsigned64(item1->credits->GetDownloadedTotal(), item2->credits->GetDownloadedTotal());
		    else if (!item1->credits)
			    iResult=1;
		    else
			    iResult=-1;
			break;
	    case 112:
		    if (item1->credits && item2->credits)
				iResult=CompareUnsigned64(item2->credits->GetDownloadedTotal(), item1->credits->GetDownloadedTotal());
		    else if (!item1->credits)
			    iResult=1;
		    else
			    iResult=-1;

			break;
		// NEO: MOD END <-- Xanatos --
#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
		case 13:
			if(item1->GetCountryName(true) && item2->GetCountryName(true))
				iResult=CompareLocaleStringNoCase(item1->GetCountryName(true), item2->GetCountryName(true));
			else if(item1->GetCountryName(true))
				iResult=1;
			else
				iResult=-1;
			break;

		case 113:
			if(item1->GetCountryName(true) && item2->GetCountryName(true))
				iResult=CompareLocaleStringNoCase(item2->GetCountryName(true), item1->GetCountryName(true));
			else if(item2->GetCountryName(true))
				iResult=1;
			else
				iResult=-1;
			break;
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --
		default:
			iResult=0;
			break;
	}
	
	// NEO: SE - [SortExtension] -- Xanatos --
	/*int dwNextSort;
	//call secondary sortorder, if this one results in equal
	//(Note: yes I know this call is evil OO wise, but better than changing a lot more code, while we have only one instance anyway - might be fixed later)
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->transferwnd->queuelistctrl.GetNextSortOrder(lParamSort)) != (-1)){
		iResult= SortProc(lParam1, lParam2, dwNextSort);
	}*/

	return iResult;

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

		const CUpDownClient* update = theApp.uploadqueue->GetNextClient(NULL);
		while( update )
		{
			theApp.emuledlg->transferwnd->queuelistctrl.RefreshClient(update);
			update = theApp.uploadqueue->GetNextClient(update);
		}
	}
	CATCH_DFLT_EXCEPTIONS(_T("CQueueListCtrl::QueueUpdateTimer"))
}

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
		// NEO: MLD - [ModelesDialogs] -- Xanatos -->
		CClientDetailDialog* dlg = new CClientDetailDialog(client, this);
		dlg->OpenDialog(); 
		// NEO: MLD END <-- Xanatos --
		//CClientDetailDialog dialog(client, this);
		//dialog.DoModal();
	}
}

void CQueueListCtrl::OnNMDblclk(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1) {
		CUpDownClient* client = (CUpDownClient*)GetItemData(iSel);
		if (client){
			// NEO: MLD - [ModelesDialogs] -- Xanatos -->
			CClientDetailDialog* dlg = new CClientDetailDialog(client, this);
			dlg->OpenDialog(); 
			// NEO: MLD END <-- Xanatos --
			//CClientDetailDialog dialog(client, this);
			//dialog.DoModal();
		}
	}
	*pResult = 0;
}

#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
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
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --