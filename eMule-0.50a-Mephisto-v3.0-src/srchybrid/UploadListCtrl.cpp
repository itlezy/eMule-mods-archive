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
#include "TransferDlg.h"
#include "otherfunctions.h"
#include "MenuCmds.h"
#include "ClientDetailDialog.h"
#include "KademliaWnd.h"
#include "emuledlg.h"
#include "friendlist.h"
#include "MemDC.h"
#include "KnownFile.h"
#include "SharedFileList.h"
#include "UpDownClient.h"
#include "ClientCredits.h"
#include "ChatWnd.h"
#include "kademlia/kademlia/Kademlia.h"
#include "kademlia/net/KademliaUDPListener.h"
#include "UploadQueue.h"
#include "ToolTipCtrlX.h"
#include "ThrottledSocket.h" //Xman Xtreme Upload
#include "UploadBandwidthThrottler.h" //Xman Xtreme Upload
#include "ListenSocket.h" //Xman changed: display the obfuscation icon for all clients which enabled it
#include "PartFile.h" //Xman PowerRelease

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CUploadListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CUploadListCtrl, CMuleListCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnClick)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnLvnGetDispInfo)
	ON_NOTIFY_REFLECT(LVN_GETINFOTIP, OnLvnGetInfoTip)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNmDblClk)
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

CUploadListCtrl::CUploadListCtrl()
	: CListCtrlItemWalk(this)
{
	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	/*
	m_tooltip = new CToolTipCtrlX;
	*/
	// workaround running MFC as service
	if (!theApp.IsRunningAsService())
		m_tooltip = new CToolTipCtrlX;
	else
		m_tooltip = NULL;
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle
	SetGeneralPurposeFind(true);
	SetSkinKey(L"UploadsLv");
}

CUploadListCtrl::~CUploadListCtrl()
{
	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	// workaround running MFC as service
	if (!theApp.IsRunningAsService())
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle
		delete m_tooltip;
}

void CUploadListCtrl::Init()
{
	SetPrefsKey(_T("UploadListCtrl"));
	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	// workaround running MFC as service
	if (!theApp.IsRunningAsService())
	{
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle
		CToolTipCtrl* tooltip = GetToolTips();
		if (tooltip) {
			m_tooltip->SubclassWindow(tooltip->m_hWnd);
			tooltip->ModifyStyle(0, TTS_NOPREFIX);
			tooltip->SetDelayTime(TTDT_AUTOPOP, 20000);
			tooltip->SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1000);
		}
	} // Run eMule as NT Service [leuk_he/Stulle] - Stulle

	InsertColumn(0, GetResString(IDS_QL_USERNAME),	LVCFMT_LEFT,  DFLT_CLIENTNAME_COL_WIDTH);
	InsertColumn(1, GetResString(IDS_FILE),			LVCFMT_LEFT,  DFLT_FILENAME_COL_WIDTH);
	InsertColumn(2, GetResString(IDS_DL_SPEED),		LVCFMT_RIGHT, DFLT_DATARATE_COL_WIDTH);
	InsertColumn(3, GetResString(IDS_DL_TRANSF),	LVCFMT_RIGHT, DFLT_DATARATE_COL_WIDTH);
	InsertColumn(4, GetResString(IDS_WAITED),		LVCFMT_LEFT,   60);
	InsertColumn(5, GetResString(IDS_UPLOADTIME),	LVCFMT_LEFT,   80);
	InsertColumn(6, GetResString(IDS_STATUS),		LVCFMT_LEFT,  100);
	InsertColumn(7, GetResString(IDS_UPSTATUS),		LVCFMT_LEFT,  DFLT_PARTSTATUS_COL_WIDTH);
	InsertColumn(8,	GetResString(IDS_CD_CSOFT),		LVCFMT_LEFT, 90); //Xman version see clientversion in every window
	InsertColumn(9, GetResString(IDS_UPDOWNUPLOADLIST),	LVCFMT_LEFT, 90); //Xman show complete up/down in uploadlist
	

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

	SetSortArrow();
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0 : 100));
}

void CUploadListCtrl::Localize()
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

	strRes = GetResString(IDS_DL_SPEED);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(2, &hdi);

	strRes = GetResString(IDS_DL_TRANSF);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(3, &hdi);

	strRes = GetResString(IDS_WAITED);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(4, &hdi);

	strRes = GetResString(IDS_UPLOADTIME);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(5, &hdi);

	strRes = GetResString(IDS_STATUS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(6, &hdi);

	strRes = GetResString(IDS_UPSTATUS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(7, &hdi);

	//Xman version see clientversion in every window
	strRes = GetResString(IDS_CD_CSOFT);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(8, &hdi);
	//Xman end

	//Xman show complete up/down in uploadlist
	strRes = GetResString(IDS_UPDOWNUPLOADLIST);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(9, &hdi);
	//Xman end

	// ==> Design Settings [eWombat/Stulle] - Stulle
	theApp.emuledlg->transferwnd->SetBackgroundColor(style_b_uploadlist);
	// <== Design Settings [eWombat/Stulle] - Stulle
}

void CUploadListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
}

void CUploadListCtrl::SetAllIcons()
{
	ApplyImageList(NULL);
	m_ImageList.DeleteImageList();
	m_ImageList.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);
	//Xman Show correct Icons	
	/*
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
	*/
	m_ImageList.Add(CTempIconLoader(_T("ClientDefault")));		//0
	m_ImageList.Add(CTempIconLoader(_T("ClientDefaultPlus")));	//1
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkey")));		//2
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyPlus")));	//3
	m_ImageList.Add(CTempIconLoader(_T("ClientCompatible")));		//4
	m_ImageList.Add(CTempIconLoader(_T("ClientCompatiblePlus")));	//5
	m_ImageList.Add(CTempIconLoader(_T("ClientFriend")));			//6
	m_ImageList.Add(CTempIconLoader(_T("ClientFriendPlus")));		//7
	m_ImageList.Add(CTempIconLoader(_T("ClientMLDonkey")));		//8
	m_ImageList.Add(CTempIconLoader(_T("ClientMLDonkeyPlus")));	//9
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyHybrid")));	//10
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyHybridPlus")));//11
	m_ImageList.Add(CTempIconLoader(_T("ClientShareaza")));		//12
	m_ImageList.Add(CTempIconLoader(_T("ClientShareazaPlus")));	//13
	m_ImageList.Add(CTempIconLoader(_T("ClientAMule")));			//14
	m_ImageList.Add(CTempIconLoader(_T("ClientAMulePlus")));		//15
	m_ImageList.Add(CTempIconLoader(_T("ClientLPhant")));			//16
	m_ImageList.Add(CTempIconLoader(_T("ClientLPhantPlus")));		//17
	m_ImageList.Add(CTempIconLoader(_T("LEECHER")));				//18 //Xman Anti-Leecher

	//Xman friend visualization
	m_ImageList.Add(CTempIconLoader(_T("ClientFriendSlotOvl"))); //19
	//Xman end

	//Xman End
	// ==> Mod Icons - Stulle
	// ==> Mephisto mod [Stulle] - Mephisto
	/*
	m_ImageList.Add(CTempIconLoader(_T("AAAEMULEAPP"))); //20
	*/
	m_ImageList.Add(CTempIconLoader(_T("SCARANGEL"))); //20
	// <== Mephisto mod [Stulle] - Mephisto
	m_ImageList.Add(CTempIconLoader(_T("STULLE"))); //21
	m_ImageList.Add(CTempIconLoader(_T("XTREME"))); //22
	m_ImageList.Add(CTempIconLoader(_T("MORPH"))); //23
	m_ImageList.Add(CTempIconLoader(_T("EASTSHARE"))); //24
	m_ImageList.Add(CTempIconLoader(_T("EMF"))); //25
	m_ImageList.Add(CTempIconLoader(_T("NEO"))); //26
	// ==> Mephisto mod [Stulle] - Mephisto
	/*
	m_ImageList.Add(CTempIconLoader(_T("MEPHISTO"))); //27
	*/
	m_ImageList.Add(CTempIconLoader(_T("AAAEMULEAPP"))); //27
	// <== Mephisto mod [Stulle] - Mephisto
	m_ImageList.Add(CTempIconLoader(_T("XRAY"))); //28
	m_ImageList.Add(CTempIconLoader(_T("MAGIC"))); //29
	// <== Mod Icons - Stulle
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("ClientSecureOvl"))), 1);
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("OverlayObfu"))), 2);
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("OverlaySecureObfu"))), 3);
	// ==> Mod Icons - Stulle
	m_overlayimages.DeleteImageList ();
	m_overlayimages.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	m_overlayimages.SetBkColor(CLR_NONE);
	m_overlayimages.Add(CTempIconLoader(_T("ClientCreditOvl")));
	m_overlayimages.Add(CTempIconLoader(_T("ClientCreditSecureOvl")));
	// <== Mod Icons - Stulle
	// Apply the image list also to the listview control, even if we use our own 'DrawItem'.
	// This is needed to give the listview control a chance to initialize the row height.
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) != 0 );
	VERIFY( ApplyImageList(m_ImageList) == NULL );
}

void CUploadListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	if (!lpDrawItemStruct->itemData)
		return;

	// ==> Visual Studio 2010 Compatibility [Stulle/Avi-3k/ied] - Stulle
	/*
	CMemDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	*/
	CMemoryDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	// <== Visual Studio 2010 Compatibility [Stulle/Avi-3k/ied] - Stulle
	BOOL bCtrlFocused;
	//Xman narrow font at transferwindow
	/*
	InitItemMemDC(dc, lpDrawItemStruct, bCtrlFocused);
	*/
	// ==> Design Settings [eWombat/Stulle] - Stulle
	/*
	InitItemMemDC(dc, lpDrawItemStruct, bCtrlFocused, true);
	//Xman end
	*/
	InitItemMemDC(dc, lpDrawItemStruct, bCtrlFocused, true, style_b_uploadlist);
	// <== Design Settings [eWombat/Stulle] - Stulle
	CRect cur_rec(lpDrawItemStruct->rcItem);
	CRect rcClient;
	GetClientRect(&rcClient);
	const CUpDownClient *client = (CUpDownClient *)lpDrawItemStruct->itemData;

	// ==> Design Settings [eWombat/Stulle] - Stulle
	/*
	COLORREF crOldBackColor = dc->GetBkColor(); //Xman PowerRelease
	*/
	// <== Design Settings [eWombat/Stulle] - Stulle
	//Xman Xtreme Upload 
	/*
    if (client->GetSlotNumber() > theApp.uploadqueue->GetActiveUploadsCount())
        dc.SetTextColor(::GetSysColor(COLOR_GRAYTEXT));
    }
	*/
	// ==> Mephisto Upload - Mephisto
	/*
	const ThrottledFileSocket* socket=(client->GetFileUploadSocket());
	if( socket!=NULL)
	{
		// ==> Design Settings [eWombat/Stulle] - Stulle
		/*
		if (socket->IsFull())
			dc.SetTextColor(RGB(0,0,0));
		else if (socket->IsTrickle())
			dc.SetTextColor(::GetSysColor(COLOR_GRAYTEXT));
		*//*
#define MLC_BLEND(A, B, X) ((A + B * (X-1) + ((X+1)/2)) / X)

#define MLC_RGBBLEND(A, B, X) (                   \
	RGB(MLC_BLEND(GetRValue(A), GetRValue(B), X), \
	MLC_BLEND(GetGValue(A), GetGValue(B), X),     \
	MLC_BLEND(GetBValue(A), GetBValue(B), X))     \
)
		if (socket->IsFull())
			;
		else if (socket->IsTrickle())
			dc.SetTextColor(MLC_RGBBLEND(dc.GetTextColor(), dc.GetBkColor(), 2));
		// <== Design Settings [eWombat/Stulle] - Stulle
		//Xman this is used for testing purpose
		else
		{
			if(socket->isready)
				dc.SetTextColor(RGB(0,0,255));
			else
				dc.SetTextColor(RGB(0,128,128));

		}
	}
	//Xman this is used for testing purpose
	else
		dc.SetTextColor(RGB(255,0,0));
	//Xman end
	*/
	if(client->upendsoon)
		dc.SetTextColor(RGB(255,50,50));
	// <== Mephisto Upload - Mephisto

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
						//Xman Show correct Icons
						/*
						if (client->IsFriend())
							iImage = 4;
						else if (client->GetClientSoft() == SO_EDONKEYHYBRID) {
							if (client->credits->GetScoreRatio(client->GetIP()) > 1)
								iImage = 8;
							else
								iImage = 7;
						}
						else if (client->GetClientSoft() == SO_MLDONKEY) {
							if (client->credits->GetScoreRatio(client->GetIP()) > 1)
								iImage = 6;
							else
								iImage = 5;
						}
						else if (client->GetClientSoft() == SO_SHAREAZA) {
							if (client->credits->GetScoreRatio(client->GetIP()) > 1)
								iImage = 10;
							else
								iImage = 9;
						}
						else if (client->GetClientSoft() == SO_AMULE) {
							if (client->credits->GetScoreRatio(client->GetIP()) > 1)
								iImage = 12;
							else
								iImage = 11;
						}
						else if (client->GetClientSoft() == SO_LPHANT) {
							if (client->credits->GetScoreRatio(client->GetIP()) > 1)
								iImage = 14;
							else
								iImage = 13;
						}
						else if (client->ExtProtocolAvailable()) {
							if (client->credits->GetScoreRatio(client->GetIP()) > 1)
								iImage = 3;
							else
								iImage = 1;
						}
						else {
							if (client->credits->GetScoreRatio(client->GetIP()) > 1)
								iImage = 2;
							else
								iImage = 0;
						}
						*/
						if (client->IsFriend())
							iImage = 6;
						else if (client->GetClientSoft() == SO_EDONKEYHYBRID){
							iImage = 10;
						}
						else if (client->GetClientSoft() == SO_EDONKEY){
							iImage = 2;
						}
						else if (client->GetClientSoft() == SO_MLDONKEY){
							iImage = 8;
						}
						else if (client->GetClientSoft() == SO_SHAREAZA){
							iImage = 12;
						}
						else if (client->GetClientSoft() == SO_AMULE){
							iImage = 14;
						}
						else if (client->GetClientSoft() == SO_LPHANT){
							iImage = 16;
						}
						else if (client->ExtProtocolAvailable()){
							// ==> Mod Icons - Stulle
							/*
							iImage = 4;
							*/
							if(client->GetModClient() == MOD_NONE)
								iImage = 4;
							else
								iImage = (uint8)(client->GetModClient() + 19);
							// <== Mod Icons - Stulle
						}
						else{
							iImage = 0;
						}
						//Xman Anti-Leecher
						if(client->IsLeecher()>0)
							iImage=18;
						else
						//Xman end
						// ==> Mod Icons - Stulle
						// ==> CreditSystems [EastShare/ MorphXT] - Stulle
						/*
						if (((client->credits)?client->credits->GetScoreRatio(client):0) > 1)
							iImage++;
						*/
						if (client->GetModClient() == MOD_NONE){
							if(client->credits && client->credits->GetHasScore(client))
								iImage++;
						}
						// <== CreditSystems [EastShare/ MorphXT] - Stulle
						// <== Mod Icons - Stulle
						//Xman end

						UINT nOverlayImage = 0;
						if ((client->Credits() && client->Credits()->GetCurrentIdentState(client->GetIP()) == IS_IDENTIFIED))
							nOverlayImage |= 1;
						//Xman changed: display the obfuscation icon for all clients which enabled it
						/*
						if (client->IsObfuscatedConnectionEstablished())
						*/
						if(client->IsObfuscatedConnectionEstablished() 
							|| (!(client->socket != NULL && client->socket->IsConnected())
							&& (client->SupportsCryptLayer() && thePrefs.IsClientCryptLayerSupported() && (client->RequestsCryptLayer() || thePrefs.IsClientCryptLayerRequested()))))
						//Xman end
							nOverlayImage |= 2;
						int iIconPosY = (cur_rec.Height() > 16) ? ((cur_rec.Height() - 16) / 2) : 1;
						POINT point = { cur_rec.left, cur_rec.top + iIconPosY };

						//Xman friend visualization
						if (client->IsFriend() && client->GetFriendSlot())
							m_ImageList.Draw(dc,19, point, ILD_NORMAL);
						//Xman end

						//EastShare Start - added by AndCycle, IP to Country 
						if(theApp.ip2country->ShowCountryFlag() )
						{
							cur_rec.left+=20;
							POINT point2= {cur_rec.left,cur_rec.top+1};
							//theApp.ip2country->GetFlagImageList()->Draw(dc, client->GetCountryFlagIndex(), point2, ILD_NORMAL);
							theApp.ip2country->GetFlagImageList()->DrawIndirect(&theApp.ip2country->GetFlagImageDrawParams(dc,client->GetCountryFlagIndex(),point2));
							cur_rec.left += sm_iLabelOffset;
						}
						//EastShare End - added by AndCycle, IP to Country

						m_ImageList.Draw(dc, iImage, point, ILD_NORMAL | INDEXTOOVERLAYMASK(nOverlayImage));

						// ==> Mod Icons - Stulle
						if(client->Credits() && client->credits->GetHasScore(client) && client->GetModClient() != MOD_NONE)
						{
							if(nOverlayImage & 1)
								m_overlayimages.Draw(dc,1, point, ILD_TRANSPARENT);
							else
								m_overlayimages.Draw(dc,0, point, ILD_TRANSPARENT);
						}
						// <== Mod Icons - Stulle

						cur_rec.left += 16 + sm_iLabelOffset;
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
						cur_rec.left -= 16;
						cur_rec.right -= sm_iSubItemInset;

						//EastShare Start - added by AndCycle, IP to Country
						if(theApp.ip2country->ShowCountryFlag() )
						{
							cur_rec.left-=20;
						}
						//EastShare End - added by AndCycle, IP to Country

						break;
					}

					case 7:
					{
						cur_rec.bottom--;
						cur_rec.top++;
						COLORREF crOldBackColor = dc->GetBkColor(); //Xman Code Improvement: FillSolidRect
						client->DrawUpStatusBar(dc, &cur_rec, false, thePrefs.UseFlatBar());
						dc.SetBkColor(crOldBackColor); //Xman Code Improvement: FillSolidRect
						//Xman client percentage (font idea by morph)
						CString buffer;
						// ==> Show Client Percentage optional [Stulle] - Stulle
						/*
						if (thePrefs.GetUseDwlPercentage())
						*/
						if (thePrefs.GetShowClientPercentage())
						// <== Show Client Percentage optional [Stulle] - Stulle
						{
							if(client->GetHisCompletedPartsPercent_UP() >=0)
							{
								COLORREF oldclr = dc.SetTextColor(RGB(0,0,0));
								int iOMode = dc.SetBkMode(TRANSPARENT);
								buffer.Format(_T("%i%%"), client->GetHisCompletedPartsPercent_UP());
								CFont *pOldFont = dc.SelectObject(&m_fontBoldSmaller);
#define	DrawClientPercentText	dc.DrawText(buffer, buffer.GetLength(),&cur_rec, ((MLC_DT_TEXT | DT_RIGHT) & ~DT_LEFT) | DT_CENTER)
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
						break;
					}

					default:
						// ==> Design Settings [eWombat/Stulle] - Stulle
						/*
						//Xman PowerRelease //Xman show LowIDs
						if(iColumn == 1){ 
							const CKnownFile *file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
							if(file && file->GetUpPriority()==PR_POWER)
								dc->SetBkColor(RGB(255,225,225));
						}
						else if(iColumn == 8 && client->HasLowID())
							dc->SetBkColor(RGB(255,250,200));
						//Xman End
						*/
						// <== Design Settings [eWombat/Stulle] - Stulle
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
						// ==> Design Settings [eWombat/Stulle] - Stulle
						/*
						dc.SetBkColor(crOldBackColor); //Xman PowerRelease //Xman show LowIDs
						*/
						// <== Design Settings [eWombat/Stulle] - Stulle
						break;
				}
			}
			cur_rec.left += iColumnWidth;
		}
	}

	DrawFocusRect(dc, lpDrawItemStruct->rcItem, lpDrawItemStruct->itemState & ODS_FOCUS, bCtrlFocused, lpDrawItemStruct->itemState & ODS_SELECTED);
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

		case 2:
			//Xman count block/success send
			//Xman // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
			/*
			_tcsncpy(pszText, CastItoXBytes(client->GetDatarate(), false, true), cchTextMax);
			*/
			// ==> Mephisto Upload - Mephisto
			/*
			if(thePrefs.ShowBlockRatio())
				_sntprintf(pszText, cchTextMax, _T("%s, %0.0f%%"),CastItoXBytes(client->GetUploadDatarate(), false, true), client->GetFileUploadSocket()->GetBlockRatio());
			else
				_tcsncpy(pszText, CastItoXBytes(client->GetUploadDatarate(), false, true), cchTextMax);
			*/
			if(thePrefs.ShowBlockRatio())
				_sntprintf(pszText, cchTextMax, _T("%s, %0.0f%%, %s"),CastItoXBytes(client->GetUploadDatarate(), false, true), client->GetFileUploadSocket()->GetBlockRatio(), (client->GetFileUploadSocket()->IsFull()?_T("F"):_T("T")));
			else
				_sntprintf(pszText, cchTextMax, _T("%s, %s"),CastItoXBytes(client->GetUploadDatarate(), false, true), (client->GetFileUploadSocket()->IsFull()?_T("F"):_T("T")));
			// <== Mephisto Upload - Mephisto
			//Xman end
			break;

		case 3:
			// NOTE: If you change (add/remove) anything which is displayed here, update also the sorting part..
			//Xman
			/*
			if (thePrefs.m_bExtControls)
				_sntprintf(pszText, cchTextMax, _T("%s (%s)"), CastItoXBytes(client->GetSessionUp(), false, false), CastItoXBytes(client->GetQueueSessionPayloadUp(), false, false));
			else
				_tcsncpy(pszText, CastItoXBytes(client->GetSessionUp(), false, false), cchTextMax);
			*/
			//Xman only intern
			//if(client->GetFileUploadSocket())
			//	Sbuffer.Format(_T("%s, ready:%u b:%u %u"),CastItoXBytes(client->GetSessionUp(), false, false), client->GetFileUploadSocket()->isready, !client->GetFileUploadSocket()->StandardPacketQueueIsEmpty(),client->GetFileUploadSocket()->blockedsendcount);
			//else
				_tcsncpy(pszText, CastItoXBytes(client->GetSessionUp(), false, false), cchTextMax);
			//Xman end
			break;

		case 4:
			if (client->HasLowID())
				_sntprintf(pszText, cchTextMax, _T("%s (%s)"), CastSecondsToHM(client->GetWaitTime() / 1000), GetResString(IDS_IDLOW));
			else
				_tcsncpy(pszText, CastSecondsToHM(client->GetWaitTime() / 1000), cchTextMax);
			break;

		case 5:
			// ==> Display remaining upload time [Stulle] - Mephisto
			/*
			_tcsncpy(pszText, CastSecondsToHM(client->GetUpStartTimeDelay() / 1000), cchTextMax);
			*/
			_sntprintf(pszText, cchTextMax, _T("%s (+%s)"), CastSecondsToHM((client->GetUpStartTimeDelay())/1000), client->GetRemainingUploadTime());
			// <== Display remaining upload time [Stulle] - Mephisto
			break;

		case 6:
			// ==> PowerShare [ZZ/MorphXT] - Stulle
			/*
			_tcsncpy(pszText, client->GetUploadStateDisplayString(), cchTextMax);
			*/
		{
			CString Sbuffer;
			Sbuffer.Format(client->GetUploadStateDisplayString());
			// ==> Mephisto Upload - Mephisto
			Sbuffer.AppendFormat(_T(",%u"),client->GetSlotNumber());
			// ==> Upload Debuging [Stulle] - Mephisto
#ifdef UPLOAD_DEBUGING
			Sbuffer.AppendFormat(_T(", %I64u / %I64u = %I64u"),client->GetFileUploadSocket()->GetFullBytesCount(), client->GetFileUploadSocket()->GetAllFullTimesCounter(), client->GetFileUploadSocket()->GetAvgFullBytes());
			Sbuffer.AppendFormat(_T(", %u / %i = %.2f < %.2f ?"), client->GetFileUploadSocket()->GetFullTimesCounter(), client->GetFileUploadSocket()->GetFullTimesListCount(), client->GetFileUploadSocket()->GetFullTimesRatio(), (((float)thePrefs.GetFullLoops())/100.0f));
#endif
			// <== Upload Debuging [Stulle] - Mephisto
			// <== Mephisto Upload - Mephisto
			// ==> Display friendslot [Stulle] - Stulle
			if (client->IsFriend() && client->GetFriendSlot())
				Sbuffer.Append(_T(",FS"));
			// <== Display friendslot [Stulle] - Stulle
			if (client->GetPowerShared())
				Sbuffer.Append(_T(",PS"));
			// ==> Fair Play [AndCycle/Stulle] - Stulle
			const CKnownFile *file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
			if (file && file->statistic.GetFairPlay()) {
				Sbuffer.Append(_T(",FairPlay"));
			}
			// <== Fair Play [AndCycle/Stulle] - Stulle
			// ==> Pay Back First [AndCycle/SiRoB/Stulle] - Stulle
			if(client->IsPBFClient()) // client->credits != NULL here
			{
				if (client->IsSecure())
					Sbuffer.Append(_T(",PBF"));
				else
					Sbuffer.Append(_T(",PBF II"));

				Sbuffer.AppendFormat(_T(" (%s)"),CastItoXBytes(client->credits->GetDownloadedTotal()-client->credits->GetUploadedTotal()));
			}
			// <== Pay Back First [AndCycle/SiRoB/Stulle] - Stulle
			_tcsncpy(pszText, Sbuffer, cchTextMax);
		}
			// <== PowerShare [ZZ/MorphXT] - Stulle
			break;

		case 7:
			_tcsncpy(pszText, GetResString(IDS_UPSTATUS), cchTextMax);
			break;

		//Xman version see clientversion in every window
		case 8:
			_tcsncpy(pszText, client->DbgGetFullClientSoftVer(), cchTextMax);
			break;
		//Xman end
				
		//Xman show complete up/down in uploadlist
		case 9:
			if(client->Credits())
				_sntprintf(pszText, cchTextMax, _T("%s/ %s"), CastItoXBytes(client->credits->GetUploadedTotal()), CastItoXBytes(client->credits->GetDownloadedTotal()));
			else
				_tcsncpy(pszText, _T("?"), cchTextMax);
			break;
		//Xman end
	}
	pszText[cchTextMax - 1] = _T('\0');
}

void CUploadListCtrl::OnLvnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
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
			case 2: // Datarate
			case 3: // Session Up
			case 4: // Wait Time
			case 7: // Part Count
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

int CUploadListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
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

		// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
		/*
		case 2:
			iResult = CompareUnsigned(item1->GetDatarate(), item2->GetDatarate());
			break;
		*/
		case 2: 
			iResult = item1->GetUploadDatarate() - item2->GetUploadDatarate();
			break;
		// Maella end

		case 3:
			iResult = CompareUnsigned(item1->GetSessionUp(), item2->GetSessionUp());
			//Xman don't show too many values
			/*
			if (iResult == 0 && thePrefs.m_bExtControls)
				iResult = CompareUnsigned(item1->GetQueueSessionPayloadUp(), item2->GetQueueSessionPayloadUp());
			*/
			//Xman end
			break;

		case 4:
			iResult = CompareUnsigned(item1->GetWaitTime(), item2->GetWaitTime());
			break;

		case 5:
			iResult = CompareUnsigned(item1->GetUpStartTimeDelay() ,item2->GetUpStartTimeDelay());
			break;

		case 6:
			// ==> Mephisto Upload - Mephisto
			/*
			iResult = item1->GetUploadState() - item2->GetUploadState();
			*/
			iResult = CompareUnsigned(item1->GetSlotNumber(), item2->GetSlotNumber());
			// <== Mephisto Upload - Mephisto
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
				if(item1->GetVersion() == item2->GetVersion() && (item1->GetClientSoft() == SO_EMULE || item1->GetClientSoft() == SO_AMULE)){
					iResult = item2->DbgGetFullClientSoftVer().CompareNoCase( item1->DbgGetFullClientSoftVer());
				}
				else {
					iResult = item1->GetVersion() - item2->GetVersion();
				}
			else
				iResult = -(item1->GetClientSoft() - item2->GetClientSoft()); // invert result to place eMule's at top
			break;
		//Xman end

		//Xman show complete up/down in uploadlist
		case 9:
			if(item1->Credits() && item2->Credits())
			{
				iResult=CompareUnsigned64(item1->credits->GetUploadedTotal(), item2->credits->GetUploadedTotal());
			}
			else
				iResult=0;
			break;
		//Xman end
	}

	if (lParamSort >= 100)
		iResult = -iResult;

	// SLUGFILLER: multiSort remove - handled in parent class
	/*
	//call secondary sortorder, if this one results in equal
	int dwNextSort;
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->transferwnd->m_pwndTransfer->uploadlistctrl.GetNextSortOrder(lParamSort)) != -1)
		iResult = SortProc(lParam1, lParam2, dwNextSort);
	*/
	// SLUGFILLER End
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
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	const CUpDownClient* client = (iSel != -1) ? (CUpDownClient*)GetItemData(iSel) : NULL;

	CTitleMenu ClientMenu;
	ClientMenu.CreatePopupMenu();
	ClientMenu.AddMenuTitle(GetResString(IDS_CLIENTS), true);
	ClientMenu.AppendMenu(MF_STRING | (client ? MF_ENABLED : MF_GRAYED), MP_DETAIL, GetResString(IDS_SHOWDETAILS), _T("CLIENTDETAILS"));
	ClientMenu.SetDefaultItem(MP_DETAIL);
	//Xman friendhandling
	ClientMenu.AppendMenu(MF_SEPARATOR); 
	//Xman end
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && !client->IsFriend()) ? MF_ENABLED : MF_GRAYED), MP_ADDFRIEND, GetResString(IDS_ADDFRIEND), _T("ADDFRIEND"));
	//Xman friendhandling
	ClientMenu.AppendMenu(MF_STRING | (client && client->IsFriend() ? MF_ENABLED : MF_GRAYED), MP_REMOVEFRIEND, GetResString(IDS_REMOVEFRIEND), _T("DELETEFRIEND"));
	ClientMenu.AppendMenu(MF_STRING | (client && client->IsFriend() ? MF_ENABLED : MF_GRAYED), MP_FRIENDSLOT, GetResString(IDS_FRIENDSLOT), _T("FRIENDSLOT"));
	ClientMenu.CheckMenuItem(MP_FRIENDSLOT, (client && client->GetFriendSlot()) ? MF_CHECKED : MF_UNCHECKED);
	ClientMenu.AppendMenu(MF_SEPARATOR); 
	//Xman end

	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED), MP_MESSAGE, GetResString(IDS_SEND_MSG), _T("SENDMESSAGE"));
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED), MP_SHOWLIST, GetResString(IDS_VIEWFILES), _T("VIEWFILES"));
	if (Kademlia::CKademlia::IsRunning() && !Kademlia::CKademlia::IsConnected())
		ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetKadPort()!=0 && client->GetKadVersion() > 1) ? MF_ENABLED : MF_GRAYED), MP_BOOT, GetResString(IDS_BOOTSTRAP));
	ClientMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED), MP_FIND, GetResString(IDS_FIND), _T("Search"));

	// - show requested files (sivka/Xman)
	ClientMenu.AppendMenu(MF_SEPARATOR); 
	ClientMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED),MP_LIST_REQUESTED_FILES, GetResString(IDS_LISTREQUESTED), _T("FILEREQUESTED")); 
	//Xman end

	GetPopupMenuPos(*this, point);
	ClientMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	VERIFY( ClientMenu.DestroyMenu() ); // XP Style Menu [Xanatos] - Stulle
}

BOOL CUploadListCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
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
					// ==> Multiple friendslots [ZZ] - Mephisto
					/*
					theApp.friendlist->RemoveAllFriendSlots();
					*/
					// <== Multiple friendslots [ZZ] - Mephisto
					if( !IsAlready )
						client->SetFriendSlot(true);
					// ==> Multiple friendslots [ZZ] - Mephisto
					else
						client->SetFriendSlot(false);
					// <== Multiple friendslots [ZZ] - Mephisto
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

void CUploadListCtrl::AddClient(const CUpDownClient *client)
{
	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	if (theApp.IsRunningAsService(SVC_LIST_OPT))
		return;
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle

	if (!theApp.emuledlg->IsRunning())
		return;

	int iItemCount = GetItemCount();
	int iItem = InsertItem(LVIF_TEXT | LVIF_PARAM, iItemCount, LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)client);
	Update(iItem);
	theApp.emuledlg->transferwnd->m_pwndTransfer->UpdateListCount(CTransferWnd::wnd2Uploading, iItemCount + 1);
}

void CUploadListCtrl::RemoveClient(const CUpDownClient *client)
{
	if (!theApp.emuledlg->IsRunning())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	int result = FindItem(&find);
	if (result != -1) {
		DeleteItem(result);
		theApp.emuledlg->transferwnd->m_pwndTransfer->UpdateListCount(CTransferWnd::wnd2Uploading);
	}
}

void CUploadListCtrl::RefreshClient(const CUpDownClient *client)
{
	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	if (theApp.IsRunningAsService(SVC_LIST_OPT))
		return;
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle

	if (!theApp.emuledlg->IsRunning())
		return;

	if (theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd || !theApp.emuledlg->transferwnd->m_pwndTransfer->uploadlistctrl.IsWindowVisible())
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

