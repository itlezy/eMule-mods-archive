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
	InsertColumn(10,GetResString(IDS_CD_CSOFT),		LVCFMT_LEFT,  90);	//Xman version see clientversion in every window
	InsertColumn(11, GetResString(IDS_UPDOWNUPLOADLIST),	LVCFMT_LEFT,  90); //Xman show complete up/down in queuelist

	// ==> push small files [sivka] - Stulle
	InsertColumn(12,GetResString(IDS_SMALL),LVCFMT_LEFT,40,12);
	// <== push small files [sivka] - Stulle

	// ==> push rare file - Stulle
	InsertColumn(13,GetResString(IDS_RARE),LVCFMT_LEFT,40,13);
	// <== push rare file - Stulle

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

	// ==> push small files [sivka] - Stulle
	strRes = GetResString(IDS_SMALL);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(12, &hdi);
	// <== push small files [sivka] - Stulle

	// ==> push rare file - Stulle
	strRes = GetResString(IDS_RARE);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(13, &hdi);
	// <== push rare file - Stulle

	// ==> Design Settings [eWombat/Stulle] - Stulle
	theApp.emuledlg->transferwnd->SetBackgroundColor(style_b_queuelist);
	// <== Design Settings [eWombat/Stulle] - Stulle
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

	//Xman end

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

void CQueueListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
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
	InitItemMemDC(dc, lpDrawItemStruct, bCtrlFocused, true, style_b_queuelist);
	// <== Design Settings [eWombat/Stulle] - Stulle
	CRect cur_rec(lpDrawItemStruct->rcItem);
	CRect rcClient;
	GetClientRect(&rcClient);
	const CUpDownClient *client = (CUpDownClient *)lpDrawItemStruct->itemData;

	// ==> Design Settings [eWombat/Stulle] - Stulle
	/*
	COLORREF crOldBackColor = dc->GetBkColor(); //Xman PowerRelease //Xman show LowIDs
	*/
	// <== Design Settings [eWombat/Stulle] - Stulle
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
						if (((client->credits)?client->credits->GetScoreRatio(client):0) > 1)
							// ==> Mod Icons - Stulle
							// ==> CreditSystems [EastShare/ MorphXT] - Stulle
							/*
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
							nOverlayImage |= 2;
						int iIconPosY = (cur_rec.Height() > 16) ? ((cur_rec.Height() - 16) / 2) : 1;
						POINT point = { cur_rec.left, cur_rec.top + iIconPosY };
						m_ImageList.Draw(dc, iImage, point, ILD_NORMAL | INDEXTOOVERLAYMASK(nOverlayImage));

						// ==> Mod Icons - Stulle
						if(client->Credits() && client->credits->GetHasScore(client) && client->GetModClient() != MOD_NONE)
						{
							if (nOverlayImage & 1)
								m_overlayimages.Draw(dc,1, point, ILD_TRANSPARENT);
							else
								m_overlayimages.Draw(dc,0, point, ILD_TRANSPARENT);
						}
						// <== Mod Icons - Stulle

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
					case 9:
						if (client->GetUpPartCount()) {
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
						}
						break;
					default:
						// ==> Design Settings [eWombat/Stulle] - Stulle
						/*
						//Xman PowerRelease //Xman show LowIDs
						if(iColumn == 1) { 
							const CKnownFile *file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
							if(file && file->GetUpPriority()==PR_POWER)
							dc.SetBkColor(RGB(255,225,225));
						}
						else if(iColumn == 10 && client->HasLowID()) 
							dc.SetBkColor(RGB(255,250,200));
						//Xman end
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
				// ==> PowerShare [ZZ/MorphXT] - Stulle
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
					//Xman PowerRelease
					case PR_POWER:
						_tcsncpy(pszText, GetResString(IDS_POWERRELEASE), cchTextMax);
						break;
					//Xman end
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
					//Xman PowerRelease
					case PR_POWER: {
						Sbuffer = GetResString(IDS_POWERRELEASE);
						break; }
					//Xman end
					default:
						Sbuffer.Empty();
				}
				if(client->GetPowerShared(file)) {
					CString tempString = GetResString(IDS_POWERSHARE_PREFIX);
					tempString.Append(_T(","));
					tempString.Append(Sbuffer);
					Sbuffer.Empty();
					Sbuffer = tempString;
				}
				// ==> Fair Play [AndCycle/Stulle] - Stulle
				if (!file->IsPartFile() && file->statistic.GetFairPlay()) {
					Sbuffer.Append(_T(",FairPlay"));
				}
				// <== Fair Play [AndCycle/Stulle] - Stulle
				_tcsncpy(pszText, Sbuffer, cchTextMax);
				// <== PowerShare [ZZ/MorphXT] - Stulle
			}
			break;
		}
		
		case 3:
			_sntprintf(pszText, cchTextMax, _T("%i"), client->GetScore(false, false, true));
			break;
		
		case 4:
			// ==> Display reason for zero score [Stulle] - Stulle
			/*
			if (client->HasLowID()) {
				if (client->m_bAddNextConnect)
					_sntprintf(pszText, cchTextMax, _T("%i ****"),client->GetScore(false));
				else
					_sntprintf(pszText, cchTextMax, _T("%i (%s)"),client->GetScore(false), GetResString(IDS_IDLOW));
			}
			//Xman uploading problem client
			else if(client->isupprob && client->m_bAddNextConnect)
			{
				if(client->socket && client->socket->IsConnected())
					_sntprintf(pszText, cchTextMax, _T("%i #~~"),client->GetScore(false));
				else
					_sntprintf(pszText, cchTextMax, _T("%i ~~~"),client->GetScore(false));
			}
			//Xman end
			else
				_sntprintf(pszText, cchTextMax, _T("%i"), client->GetScore(false));
			*/
			{
				CString Sbuffer;
				uint32 uScore = client->GetScore(false);
				if (client->HasLowID()){
					if (client->m_bAddNextConnect)
						Sbuffer.Format(_T("%i ****"),uScore);
					else
						Sbuffer.Format(_T("%i (%s)"),uScore, GetResString(IDS_IDLOW));
				}
				//Xman uploading problem client
				else if(client->isupprob && client->m_bAddNextConnect)
				{
					if(client->socket && client->socket->IsConnected())
						Sbuffer.Format(_T("%i #~~"),uScore);
					else
						Sbuffer.Format(_T("%i ~~~"),uScore);
				}
				//Xman end
				else
					Sbuffer.Format(_T("%i"),uScore);

				if(uScore == 0)
					Sbuffer.AppendFormat(_T(" (%s)"),client->GetZeroScoreString());

				// ==> Pay Back First [AndCycle/SiRoB/Stulle] - Stulle
				if (client->IsPBFClient())
				{
					CString tempStr;
					if (client->IsSecure())
						tempStr.Format(_T("%s %s"), _T("PBF"), Sbuffer);
					else
						tempStr.Format(_T("%s %s"), _T("PBF II"), Sbuffer);
					Sbuffer = tempStr;
				}
				// <== Pay Back First [AndCycle/SiRoB/Stulle] - Stulle

				_tcsncpy(pszText, Sbuffer, cchTextMax);
			}
			// <== Display reason for zero score [Stulle] - Stulle
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
			//Xman Code Improvement
			/*
			_tcsncpy(pszText, GetResString(client->IsBanned() ? IDS_YES : IDS_NO), cchTextMax);
			*/
			_tcsncpy(pszText, GetResString(client->GetUploadState() == US_BANNED ? IDS_YES : IDS_NO), cchTextMax);
			//Xman end
			break;

		case 9:
			_tcsncpy(pszText, GetResString(IDS_UPSTATUS), cchTextMax);
			break;
		//Xman version see clientversion in every window
		case 10:
			_tcsncpy(pszText, client->DbgGetFullClientSoftVer(), cchTextMax); //Xman // Maella -Support for tag ET_MOD_VERSION 0x55
			break;
		//Xman end

		//Xman show complete up/down in queuelist
		case 11:
			if(client->Credits() )
				_sntprintf(pszText, cchTextMax, _T("%s/ %s"), CastItoXBytes(client->credits->GetUploadedTotal()), CastItoXBytes(client->credits->GetDownloadedTotal()));
			break;
		//Xman end

		// ==> push small files [sivka] - Stulle
		case 12:
		{
			if (client->GetSmallFilePush())
				_tcsncpy(pszText, GetResString(IDS_YES), cchTextMax);
			else
				_tcsncpy(pszText, GetResString(IDS_NO), cchTextMax);
			break;
		}
		// <== push small files [sivka] - Stulle

		// ==> push rare file - Stulle
		case 13:
		{
			_sntprintf(pszText, cchTextMax, _T("%.1f"), client->GetRareFilePushRatio());
			break;
		}
		// <== push rare file - Stulle
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
			if (file1 != NULL && file2 != NULL)
			// ==> PowerShare [ZZ/MorphXT] - Stulle
			// ==> Fair Play [AndCycle/Stulle] - Stulle
			{
				if (!file1->GetPowerShared() && !file1->statistic.GetFairPlay() && (file2->GetPowerShared() || file2->statistic.GetFairPlay()))
					iResult=-1;			
				else if ((file1->GetPowerShared() || file1->statistic.GetFairPlay()) && !file2->GetPowerShared() && !file2->statistic.GetFairPlay())
					iResult=1;
				else
			// <== Fair Play [AndCycle/Stulle] - Stulle
			// <== PowerShare [ZZ/MorphXT] - Stulle
				iResult = (file1->GetUpPriority() == PR_VERYLOW ? -1 : file1->GetUpPriority()) - (file2->GetUpPriority() == PR_VERYLOW ? -1 : file2->GetUpPriority());
			} // PowerShare [ZZ/MorphXT] - Stulle
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
			// ==> Superior Client Handling [Stulle] - Stulle
			if(!item1->IsSuperiorClient() && item2->IsSuperiorClient())
					iResult=-1;
			else if(item1->IsSuperiorClient() && !item2->IsSuperiorClient())
					iResult=1;
			else
			// <== Superior Client Handling [Stulle] - Stulle
			iResult = CompareUnsigned(item1->GetScore(false), item2->GetScore(false));
			break;

		case 5:
			iResult = CompareUnsigned(item1->GetAskedCount(), item2->GetAskedCount());
			break;

		case 6:
			iResult = CompareUnsigned(item1->GetLastUpRequest(), item2->GetLastUpRequest());
			break;

		case 7:
			// ==> SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
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
				break;
			}
			// <== SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
			break;

		case 8:
			iResult = item1->IsBanned() - item2->IsBanned();
			break;
		
		case 9: 
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
		case 10:
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
		//Xman show complete up/down in queuelist
		case 11:
			if(item1->Credits() && item2->Credits())
				iResult=CompareUnsigned64(item1->credits->GetUploadedTotal(), item2->credits->GetUploadedTotal());
			else
				iResult=0;
			break;
		//Xman end

		// ==> push small files [sivka] - Stulle
		case 12:
			iResult=item1->GetSmallFilePush() - item2->GetSmallFilePush();
			break;
		// <== push small files [sivka] - Stulle

		// ==> push rare file - Stulle
		case 13:
			iResult=CompareFloat(item1->GetRareFilePushRatio(),item2->GetRareFilePushRatio());
			break;
		// <== push rare file - Stulle
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
	// SLUGFILLER: multiSort remove - handled in parent class

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
	if (thePrefs.IsExtControlsEnabled())
		ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->IsBanned()) ? MF_ENABLED : MF_GRAYED), MP_UNBAN, GetResString(IDS_UNBAN));
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

void CQueueListCtrl::AddClient(/*const*/ CUpDownClient *client, bool resetclient)
{
	if (resetclient && client){
		// ==> SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
		/*
		client->SetWaitStartTime();
		*/
		// <== SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
		client->SetAskedCount(1);
	}

	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	if (theApp.IsRunningAsService(SVC_LIST_OPT))
		return;
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle

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
	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	if (theApp.IsRunningAsService(SVC_LIST_OPT))
		return;
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle

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
	//Xman unreachable
	/*
	try
	*/
	//Xman end
	{
		if (   !theApp.emuledlg->IsRunning() // Don't do anything if the app is shutting down - can cause unhandled exceptions
			|| !thePrefs.GetUpdateQueueList()
			|| theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd
			|| !theApp.emuledlg->transferwnd->GetQueueList()->IsWindowVisible() )
			return;

		//Xman faster Updating of Queuelist
		/*
		const CUpDownClient* update = theApp.uploadqueue->GetNextClient(NULL);
		while( update )
		{
			theApp.emuledlg->transferwnd->GetQueueList()->RefreshClient(update);
			update = theApp.uploadqueue->GetNextClient(update);
		}
		*/
		if (theApp.emuledlg->transferwnd->GetQueueList()->GetItemCount()>1)
		{

			theApp.emuledlg->transferwnd->GetQueueList()->UpdateAll();
		}
		//Xman end
	}
	//Xman unreachable
	/*
	CATCH_DFLT_EXCEPTIONS(_T("CQueueListCtrl::QueueUpdateTimer"))
	*/
	//Xman end
}//Xman faster Updating of Queuelist
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
//Xman end