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
#include "ClientListCtrl.h"
#include "otherfunctions.h"
#include "MenuCmds.h"
#include "ClientDetailDialog.h"
#include "KademliaWnd.h"
#include "ClientList.h"
#include "emuledlg.h"
#include "FriendList.h"
#include "TransferWnd.h"
#include "MemDC.h"
#include "UpDownClient.h"
#include "ClientCredits.h"
#include "ListenSocket.h"
#include "ChatWnd.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "Kademlia/net/KademliaUDPListener.h"
#include "Neo/NeoPreferences.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --
#include "Neo\GUI\XPMenu\MenuXP.h" // NEO: NMX - [NeoMenuXP] <-- Xanatos --

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CClientListCtrl

IMPLEMENT_DYNAMIC(CClientListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CClientListCtrl, CMuleListCtrl)
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOLORCHANGE()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
END_MESSAGE_MAP()

CClientListCtrl::CClientListCtrl()
	: CListCtrlItemWalk(this)
{
	SetGeneralPurposeFind(true, false);
}

void CClientListCtrl::Init()
{
	SetName(_T("ClientListCtrl"));

	CImageList ilDummyImageList; //dummy list for getting the proper height of listview entries
	ilDummyImageList.Create(1, theApp.GetSmallSytemIconSize().cy,theApp.m_iDfltImageListColorFlags|ILC_MASK, 1, 1); 
	SetImageList(&ilDummyImageList, LVSIL_SMALL);
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	ilDummyImageList.Detach();

	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	InsertColumn(0,GetResString(IDS_QL_USERNAME),LVCFMT_LEFT,150,0);
	InsertColumn(1,GetResString(IDS_CL_UPLOADSTATUS),LVCFMT_LEFT,150,1);
	InsertColumn(2,GetResString(IDS_CL_TRANSFUP),LVCFMT_LEFT,150,2);
	InsertColumn(3,GetResString(IDS_CL_DOWNLSTATUS),LVCFMT_LEFT,150,3);
	InsertColumn(4,GetResString(IDS_CL_TRANSFDOWN),LVCFMT_LEFT,150,4);
	InsertColumn(5,GetResString(IDS_CD_CSOFT),LVCFMT_LEFT,150,5);
	InsertColumn(6,GetResString(IDS_CONNECTED),LVCFMT_LEFT,150,6);
	CString coltemp;
	coltemp=GetResString(IDS_CD_UHASH);coltemp.Remove(':');
	InsertColumn(7,coltemp,LVCFMT_LEFT,150,7);
#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
	InsertColumn(8,GetResString(IDS_X_COUNTRY),LVCFMT_LEFT,100,8);
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --	

	SetAllIcons();
	Localize();
	LoadSettings();
	SetSortArrow();
	SortItems(SortProc, GetSortItem()+ (GetSortAscending()? 0:100));

#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
	if (NeoPrefs.GetIP2CountryNameMode() == IP2CountryName_DISABLE)
	    HideColumn(8);
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --
}

CClientListCtrl::~CClientListCtrl()
{
}

void CClientListCtrl::OnSysColorChange()
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

void CClientListCtrl::SetAllIcons()
{
	imagelist.DeleteImageList();
	imagelist.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	imagelist.SetBkColor(CLR_NONE);
	//imagelist.Add(CTempIconLoader(_T("ClientEDonkey")));
	//imagelist.Add(CTempIconLoader(_T("ClientCompatible")));
	//imagelist.Add(CTempIconLoader(_T("Friend")));
	//imagelist.Add(CTempIconLoader(_T("ClientMLDonkey")));
	//imagelist.Add(CTempIconLoader(_T("ClientEDonkeyHybrid")));
	//imagelist.Add(CTempIconLoader(_T("ClientShareaza")));
	//imagelist.Add(CTempIconLoader(_T("Server")));
	//imagelist.Add(CTempIconLoader(_T("ClientAMule")));
	//imagelist.Add(CTempIconLoader(_T("ClientLPhant")));
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

void CClientListCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;

	if(pHeaderCtrl->GetItemCount() != 0) {
		CString strRes;

		strRes = GetResString(IDS_QL_USERNAME);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(0, &hdi);

		strRes = GetResString(IDS_CL_UPLOADSTATUS);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(1, &hdi);

		strRes = GetResString(IDS_CL_TRANSFUP);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(2, &hdi);

		strRes = GetResString(IDS_CL_DOWNLSTATUS);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(3, &hdi);

		strRes = GetResString(IDS_CL_TRANSFDOWN);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(4, &hdi);

		strRes = GetResString(IDS_CD_CSOFT);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(5, &hdi);

		strRes = GetResString(IDS_CONNECTED);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(6, &hdi);

		strRes = GetResString(IDS_CD_UHASH);
		strRes.Remove(':');
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(7, &hdi);

#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
		strRes = GetResString(IDS_X_COUNTRY);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(8, &hdi);
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --
	}
}

void CClientListCtrl::ShowKnownClients()
{
	DeleteAllItems();
	int iItemCount = 0;
	for(POSITION pos = theApp.clientlist->list.GetHeadPosition(); pos != NULL;){
		const CUpDownClient* cur_client = theApp.clientlist->list.GetNext(pos);
		int iItem = InsertItem(LVIF_TEXT|LVIF_PARAM,iItemCount,LPSTR_TEXTCALLBACK,0,0,0,(LPARAM)cur_client);
		Update(iItem);
		iItemCount++;
	}
	theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Clients, iItemCount);
}

void CClientListCtrl::AddClient(const CUpDownClient* client)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	if (thePrefs.IsKnownClientListDisabled())
		return;

	int iItemCount = GetItemCount();
	int iItem = InsertItem(LVIF_TEXT|LVIF_PARAM,iItemCount,LPSTR_TEXTCALLBACK,0,0,0,(LPARAM)client);
	Update(iItem);
	theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Clients, iItemCount+1);
}

void CClientListCtrl::RemoveClient(const CUpDownClient* client)
{
	if (!theApp.emuledlg->IsRunning())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	int result = FindItem(&find);
	if (result != -1) {
		DeleteItem(result);
		theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Clients);
	}
}

void CClientListCtrl::RefreshClient(const CUpDownClient* client)
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

void CClientListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
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
					//	image = 2;
					//else if (client->GetClientSoft() == SO_EDONKEYHYBRID)
					//	image = 4;
					//else if (client->GetClientSoft() == SO_MLDONKEY)
					//	image = 3;
					//else if (client->GetClientSoft() == SO_SHAREAZA)
					//	image = 5;
					//else if (client->GetClientSoft() == SO_URL)
					//	image = 6;
					//else if (client->GetClientSoft() == SO_AMULE)
					//	image = 7;
					//else if (client->GetClientSoft() == SO_LPHANT)
					//	image = 8;
					//else if (client->ExtProtocolAvailable())
					//	image = 1;
					//else
					//	image = 0;
					//
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
										&& (NeoPrefs.IsIP2CountryShowFlag() == 2 || IsColumnHidden(8));
					if(bShowFlags){
						cur_rec.left+=20;
						POINT point2= {cur_rec.left,cur_rec.top+1};
						int index = client->GetCountryFlagIndex();
						theApp.ip2country->GetFlagImageList()->DrawIndirect(dc, index , point2, CSize(18,16), CPoint(0,0), ILD_NORMAL);
					}
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --
					if (client->GetUserName()==NULL)
						Sbuffer.Format(_T("(%s)"), GetResString(IDS_UNKNOWN));
					else
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
				case 1:{
					Sbuffer = client->GetUploadStateDisplayString();
					break;
				}
				case 2:{
					if(client->credits)
						Sbuffer = CastItoXBytes(client->credits->GetUploadedTotal(), false, false);
					else
						Sbuffer.Empty();
					break;
				}
				case 3:{
					Sbuffer = client->GetDownloadStateDisplayString();
					break;
				}
				case 4:{
					if(client->credits)
						Sbuffer = CastItoXBytes(client->credits->GetDownloadedTotal(), false, false);
					else
						Sbuffer.Empty();
					break;
				}
				case 5:{
					//Sbuffer = client->GetClientSoftVer();
					Sbuffer = client->DbgGetFullClientSoftVer(); // NEO: MIDI - [ModIDInfo] <-- Xanatos --
					if (Sbuffer.IsEmpty())
						Sbuffer = GetResString(IDS_UNKNOWN);
					break;
				}
				case 6:{
					if(client->socket){
						if(client->socket->IsConnected()){
							Sbuffer = GetResString(IDS_YES);
							break;
						}
					}
					Sbuffer = GetResString(IDS_NO);
					break;
				}
				case 7:
					Sbuffer = md4str(client->GetUserHash());
					break;
#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
				case 8:
					const bool bShowFlags = NeoPrefs.IsIP2CountryShowFlag() == 1 && theApp.ip2country->ShowCountryFlag();
					if(bShowFlags){
						POINT point2= {cur_rec.left,cur_rec.top+1};
						int index = client->GetCountryFlagIndex();
						theApp.ip2country->GetFlagImageList()->DrawIndirect(dc, index , point2, CSize(18,16), CPoint(0,0), ILD_NORMAL);
						cur_rec.left+=20;
					}
					Sbuffer.Format(_T("%s"), client->GetCountryName());
					dc->DrawText(Sbuffer,Sbuffer.GetLength(),&cur_rec,DLC_DT_TEXT);
					if(bShowFlags)
						cur_rec.left-=20;
					break;
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --
			}
#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
			if( iColumn != 0 && iColumn != 8 )
#else
			if( iColumn != 0)
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --
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
	
void CClientListCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
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
	if (Kademlia::CKademlia::IsRunning() && !Kademlia::CKademlia::IsConnected())
		ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetKadPort()!=0) ? MF_ENABLED : MF_GRAYED), MP_BOOT, GetResString(IDS_BOOTSTRAP));
	ClientMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED), MP_FIND, GetResString(IDS_FIND), _T("Search"));
	GetPopupMenuPos(*this, point);
	ClientMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	VERIFY( ClientMenu.DestroyMenu() ); // NEO: FIX - [DestroyMenu] <-- Xanatos --
}

BOOL CClientListCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
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

void CClientListCtrl::OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult){

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

int CClientListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	const CUpDownClient* item1 = (CUpDownClient*)lParam1;
	const CUpDownClient* item2 = (CUpDownClient*)lParam2;
	
	int iResult=0;
	switch (lParamSort) {
	    case 0:
			if (item1->GetUserName() && item2->GetUserName())
				iResult=CompareLocaleStringNoCase(item1->GetUserName(), item2->GetUserName());
			else if (!item1->GetUserName() && !item2->GetUserName())
				iResult=0;
			else {
				// place clients with no usernames at bottom
				if (!item1->GetUserName())
					iResult=1;
				else
					iResult=-1;
			}
			break;
	    case 100:
			if (item1->GetUserName() && item2->GetUserName())
				iResult=CompareLocaleStringNoCase(item2->GetUserName(), item1->GetUserName());
			else if (!item1->GetUserName() && !item2->GetUserName())
				iResult=0;
			else {
				// place clients with no usernames at bottom
				if (!item1->GetUserName())
					iResult=1;
				else
					iResult=-1;
			}
			break;

		case 1:
		    iResult=item1->GetUploadState() - item2->GetUploadState();
			break;
	    case 101:
		    iResult=item2->GetUploadState() - item1->GetUploadState();
			break;

		case 2:
			if (item1->credits && item2->credits)
				iResult=CompareUnsigned64(item1->credits->GetUploadedTotal(), item2->credits->GetUploadedTotal());
			else if (!item1->credits)
			    iResult=1;
			else
				iResult=-1;
			break;
	    case 102:
			if (item1->credits && item2->credits)
				iResult=CompareUnsigned64(item2->credits->GetUploadedTotal(), item1->credits->GetUploadedTotal());
			else if (!item1->credits)
				iResult=1;
			else
			    iResult=-1;
			break;

		case 3:
			// NEO: FIX - [SourceCount] -- Xanatos --
		    /*if (item1->GetDownloadState() == item2->GetDownloadState()) {
			    if (item1->IsRemoteQueueFull() && item2->IsRemoteQueueFull())
				    iResult=0;
			    else if (item1->IsRemoteQueueFull())
				    iResult=1;
			    else if (item2->IsRemoteQueueFull())
				    iResult=-1;
			    else
				    iResult=0;
		    } else*/
				iResult=item1->GetDownloadState() - item2->GetDownloadState();
			break;
	    case 103:
			// NEO: FIX - [SourceCount] -- Xanatos --
		    /*if (item2->GetDownloadState() == item1->GetDownloadState()) {
			    if (item2->IsRemoteQueueFull() && item1->IsRemoteQueueFull())
				    iResult=0;
			    else if (item2->IsRemoteQueueFull())
				    iResult=1;
			    else if (item1->IsRemoteQueueFull())
				    iResult=-1;
			    else
				    iResult=0;
		    } else*/
				iResult=item2->GetDownloadState() - item1->GetDownloadState();
			break;

		case 4:
		    if (item1->credits && item2->credits)
				iResult=CompareUnsigned64(item1->credits->GetDownloadedTotal(), item2->credits->GetDownloadedTotal());
		    else if (!item1->credits)
			    iResult=1;
		    else
				iResult=-1;
			break;
	    case 104:
		    if (item1->credits && item2->credits)
				iResult=CompareUnsigned64(item2->credits->GetDownloadedTotal(), item1->credits->GetDownloadedTotal());
		    else if (!item1->credits)
			    iResult=1;
		    else
			    iResult=-1;
			break;

		case 5:
		    /*if (item1->GetClientSoft() == item2->GetClientSoft())
			    iResult=item2->GetVersion() - item1->GetVersion();
		    else 
				iResult=item1->GetClientSoft() - item2->GetClientSoft();*/
			iResult=CompareLocaleStringNoCase(item1->DbgGetFullClientSoftVer(), item2->DbgGetFullClientSoftVer()); // NEO: MIDI - [ModIDInfo] <-- Xanatos --
			break;
	    case 105:
		    /*if (item1->GetClientSoft() == item2->GetClientSoft())
			    iResult=item1->GetVersion() - item2->GetVersion();
		    else
				iResult=item2->GetClientSoft() - item1->GetClientSoft();*/
			iResult=CompareLocaleStringNoCase(item2->DbgGetFullClientSoftVer(), item1->DbgGetFullClientSoftVer()); // NEO: MIDI - [ModIDInfo] <-- Xanatos --
			break;

		case 6:
		    if (item1->socket && item2->socket)
			    iResult=item1->socket->IsConnected() - item2->socket->IsConnected();
		    else if (!item1->socket)
			    iResult=-1;
		    else
			    iResult=1;
			break;
	    case 106:
		    if (item1->socket && item2->socket)
			    iResult=item2->socket->IsConnected() - item1->socket->IsConnected();
		    else if (!item2->socket)
			    iResult=-1;
		    else
			    iResult=1;
			break;

		case 7:
		    iResult=memcmp(item1->GetUserHash(), item2->GetUserHash(), 16);
			break;
	    case 107:
		    iResult=memcmp(item2->GetUserHash(), item1->GetUserHash(), 16);
			break;

#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
        case 8:
			if(item1->GetCountryName(true) && item2->GetCountryName(true))
				iResult=CompareLocaleStringNoCase(item1->GetCountryName(true), item2->GetCountryName(true));
			else if(item1->GetCountryName(true))
				iResult=1;
			else
				iResult=-1;
			break;

		case 108:
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
	}

	// NEO: SE - [SortExtension] -- Xanatos --
	/*int dwNextSort;
	//call secondary sortorder, if this one results in equal
	//(Note: yes I know this call is evil OO wise, but better than changing a lot more code, while we have only one instance anyway - might be fixed later)
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->transferwnd->clientlistctrl.GetNextSortOrder(lParamSort)) != (-1)){
		iResult= SortProc(lParam1, lParam2, dwNextSort);
	}*/

	return iResult;
}

void CClientListCtrl::ShowSelectedUserDetails()
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

void CClientListCtrl::OnNMDblclk(NMHDR* /*pNMHDR*/, LRESULT* pResult)
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

void CClientListCtrl::OnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
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
