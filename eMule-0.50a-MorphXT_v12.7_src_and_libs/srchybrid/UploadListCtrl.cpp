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
#include "Ntservice.h" //Morph

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
	//MORPH START leuk_he:run as ntservice v1..
	/*
	m_tooltip = new CToolTipCtrlX;
	*/
	// workaround running MFC as service
	if (!theApp.IsRunningAsService())
		m_tooltip = new CToolTipCtrlX;
	else
		m_tooltip = NULL;
	//MORPH END leuk_he:run as ntservice v1..
	SetGeneralPurposeFind(true);
	SetSkinKey(L"UploadsLv");
}

CUploadListCtrl::~CUploadListCtrl()
{
	if (!theApp.IsRunningAsService()) // MORPH leuk_he:run as ntservice v1.. (worksaround for MFC as a service) 
		delete m_tooltip;
}

void CUploadListCtrl::Init()
{
	SetPrefsKey(_T("UploadListCtrl"));
	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	//MORPH START leuk_he:run as ntservice v1..
	// workaround running MFC as service
	if (!theApp.IsRunningAsService())
	{
	//MORPH END leuk_he:run as ntservice v1..
		CToolTipCtrl* tooltip = GetToolTips();
		if (tooltip) {
			m_tooltip->SubclassWindow(tooltip->m_hWnd);
			tooltip->ModifyStyle(0, TTS_NOPREFIX);
			tooltip->SetDelayTime(TTDT_AUTOPOP, 20000);
			tooltip->SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1000);
		}
	} //MORPH leuk_he:run as ntservice v1..

	InsertColumn(0, GetResString(IDS_QL_USERNAME),	LVCFMT_LEFT,  DFLT_CLIENTNAME_COL_WIDTH);
	InsertColumn(1, GetResString(IDS_FILE),			LVCFMT_LEFT,  DFLT_FILENAME_COL_WIDTH);
	InsertColumn(2, GetResString(IDS_DL_SPEED),		LVCFMT_RIGHT, DFLT_DATARATE_COL_WIDTH);
	InsertColumn(3, GetResString(IDS_DL_TRANSF),	LVCFMT_RIGHT, DFLT_DATARATE_COL_WIDTH);
	InsertColumn(4, GetResString(IDS_WAITED),		LVCFMT_LEFT,   60);
	InsertColumn(5, GetResString(IDS_UPLOADTIME),	LVCFMT_LEFT,   80);
	InsertColumn(6, GetResString(IDS_STATUS),		LVCFMT_LEFT,  100);
	InsertColumn(7, GetResString(IDS_UPSTATUS),		LVCFMT_LEFT,  DFLT_PARTSTATUS_COL_WIDTH);
	//MORPH START - Added by SiRoB, Client Software
	InsertColumn(8,GetResString(IDS_CD_CSOFT),LVCFMT_LEFT,100);
	//MORPH END - Added by SiRoB, Client Software
	InsertColumn(9,GetResString(IDS_UPL_DL),LVCFMT_LEFT,100); //Total up down
	InsertColumn(10,GetResString(IDS_CL_DOWNLSTATUS),LVCFMT_LEFT,100); //Yun.SF3 Remote Queue Status
	//MORPH START - Added by SiRoB, ZZ Upload System 20030724-0336
	InsertColumn(11,GetResString(IDS_UPSLOTNUMBER),LVCFMT_LEFT,100);
	//MORPH END - Added by SiRoB, ZZ Upload System 20030724-0336
	//MORPH START - Added by SiRoB, Show Compression by Tarod
	InsertColumn(12,GetResString(IDS_COMPRESSIONGAIN),LVCFMT_LEFT,50);
	//MORPH END - Added by SiRoB, Show Compression by Tarod

	// Mighty Knife: Community affiliation
	if (thePrefs.IsCommunityEnabled())
		InsertColumn(13,GetResString(IDS_COMMUNITY),LVCFMT_LEFT,100);
	else
		InsertColumn(13,GetResString(IDS_COMMUNITY),LVCFMT_LEFT,100,-1,true);
	// [end] Mighty Knife

	// EastShare - Added by Pretender, Friend Tab
	InsertColumn(14,GetResString(IDS_FRIENDLIST),LVCFMT_LEFT,75);
	// EastShare - Added by Pretender, Friend Tab

	// Commander - Added: IP2Country column - Start
	if (thePrefs.GetIP2CountryNameMode() == IP2CountryName_DISABLE)
		InsertColumn(15,GetResString(IDS_COUNTRY),LVCFMT_LEFT,100,-1,true);
	else
		InsertColumn(15,GetResString(IDS_COUNTRY),LVCFMT_LEFT,100);
	// Commander - Added: IP2Country column - End
	
	//MORPH START - Added by SiRoB, Display current uploading chunk
	InsertColumn(16,GetResString(IDS_CHUNK),LVCFMT_LEFT,100);
	//MORPH END   - Added by SiRoB, Display current uploading chunk
		
	SetAllIcons();
	Localize();
	LoadSettings();
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
		
	//MORPH START - Modified by SiRoB, Client Software
	strRes = GetResString(IDS_CD_CSOFT);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(8, &hdi);
	//MORPH END - Modified by SiRoB, Client Software

	//MORPH START - Modified by IceCream, Total up down
	strRes = GetResString(IDS_UPL_DL);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(9, &hdi);
	//MORPH END - Modified by IceCream, Total up down

	//MORPH START - Modified by IceCream, Remote Status
	strRes = GetResString(IDS_CL_DOWNLSTATUS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(10, &hdi);
	//MORPH END - Modified by IceCream, Remote Status

	//MORPH START - Added by SiRoB, ZZ Missing
	strRes = GetResString(IDS_UPSLOTNUMBER);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(11, &hdi);
	//MORPH END - Added by SiRoB, ZZ Missing

	//MORPH START - Added by SiRoB, Show Compression by Tarod
	strRes = GetResString(IDS_COMPRESSIONGAIN);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(12, &hdi);
	//MORPH END - Added by SiRoB, Show Compression by Tarod

	// Mighty Knife: Community affiliation
	strRes = GetResString(IDS_COMMUNITY);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(13, &hdi);
	// [end] Mighty Knife

	// EastShare - Added by Pretender, Friend Tab
	strRes = GetResString(IDS_FRIENDLIST);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(14, &hdi);
	// EastShare - Added by Pretender, Friend Tab

	// Commander - Added: IP2Country column - Start
	strRes = GetResString(IDS_COUNTRY);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(15, &hdi);
	// Commander - Added: IP2Country column - End
		
	//MORPH START - Added by SiRoB, Display current uploading chunk
	strRes = GetResString(IDS_CHUNK);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(16, &hdi);
	//MORPH START - Added by SiRoB, Display current uploading chunk
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
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkey")));
	m_ImageList.Add(CTempIconLoader(_T("ClientCompatible")));
	//MORPH START - Modified by SiRoB, More client
	//m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyPlus")));
	//m_ImageList.Add(CTempIconLoader(_T("ClientCompatiblePlus")));
	m_ImageList.Add(CTempIconLoader(_T("Friend")));
	m_ImageList.Add(CTempIconLoader(_T("ClientMLDonkey")));
	//m_ImageList.Add(CTempIconLoader(_T("ClientMLDonkeyPlus")));
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyHybrid")));
	//m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyHybridPlus")));
	m_ImageList.Add(CTempIconLoader(_T("ClientShareaza")));
	//m_ImageList.Add(CTempIconLoader(_T("ClientShareazaPlus")));
	m_ImageList.Add(CTempIconLoader(_T("ClientAMule")));
	//m_ImageList.Add(CTempIconLoader(_T("ClientAMulePlus")));
	m_ImageList.Add(CTempIconLoader(_T("ClientLPhant")));
	//m_ImageList.Add(CTempIconLoader(_T("ClientLPhantPlus")));
	m_ImageList.Add(CTempIconLoader(_T("ClientRightEdonkey")));
	m_ImageList.Add(CTempIconLoader(_T("Morph")));
	m_ImageList.Add(CTempIconLoader(_T("SCARANGEL")));
	m_ImageList.Add(CTempIconLoader(_T("STULLE")));
	m_ImageList.Add(CTempIconLoader(_T("XTREME")));
	m_ImageList.Add(CTempIconLoader(_T("EASTSHARE")));
	m_ImageList.Add(CTempIconLoader(_T("EMF")));
	m_ImageList.Add(CTempIconLoader(_T("NEO")));
	m_ImageList.Add(CTempIconLoader(_T("MEPHISTO")));
	m_ImageList.Add(CTempIconLoader(_T("XRAY")));
	m_ImageList.Add(CTempIconLoader(_T("MAGIC")));
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("ClientSecureOvl"))), 1);
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("OverlayObfu"))), 2);
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("OverlaySecureObfu"))), 3);
	//MORPH END   - Modified by SiRoB, More client

	// Mighty Knife: Community icon
	m_overlayimages.DeleteImageList();
	m_overlayimages.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	m_overlayimages.SetBkColor(CLR_NONE);
	m_overlayimages.Add(CTempIconLoader(_T("Community")));
	// [end] Mighty Knife
	//MORPH START - Addded by SiRoB, Friend Addon
	m_overlayimages.Add(CTempIconLoader(_T("ClientFriendOvl")));
	m_overlayimages.Add(CTempIconLoader(_T("ClientFriendSlotOvl")));
	//MORPH END   - Addded by SiRoB, Friend Addon
	//MORPH START - Credit Overlay Icon
	m_overlayimages.Add(CTempIconLoader(_T("ClientCreditOvl")));
	m_overlayimages.Add(CTempIconLoader(_T("ClientCreditSecureOvl")));
	//MORPH END   - Credit Overlay Icon
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

	//MORPH START - Changed by Stulle, Visual Studio 2010 Compatibility
	/*
	CMemDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	*/
	CMemoryDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	//MORPH END   - Changed by Stulle, Visual Studio 2010 Compatibility
	BOOL bCtrlFocused;
	InitItemMemDC(dc, lpDrawItemStruct, bCtrlFocused);
	CRect cur_rec(lpDrawItemStruct->rcItem);
	CRect rcClient;
	GetClientRect(&rcClient);
	const CUpDownClient *client = (CUpDownClient *)lpDrawItemStruct->itemData;

	//MORPH START - Upload code
	/*
    if (client->GetSlotNumber() > theApp.uploadqueue->GetActiveUploadsCount())
	*/
	if(client->IsScheduledForRemoval())
		dc.SetTextColor(RGB(255,50,50));
	else if(client->GetSlotNumber() > theApp.uploadqueue->GetActiveUploadsCount(client->GetClassID())) //MORPH - Upload Splitting Class
	//MORPH END   - Upload code
        dc.SetTextColor(::GetSysColor(COLOR_GRAYTEXT));

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
						//MORPH START - Modified by SiRoB, More client & Credit overlay icon
						int iImage;
						//MORPH - Removed by SiRoB, Friend Addon
						/*
						if (client->IsFriend())
							iImage = 2;
						else*/ if (client->GetClientSoft() == SO_MLDONKEY )
							iImage = 3;
						else if (client->GetClientSoft() == SO_EDONKEYHYBRID )
							iImage = 4;
						else if (client->GetClientSoft() == SO_SHAREAZA )
							iImage = 5;
						else if (client->GetClientSoft() == SO_AMULE)
							iImage = 6;
						else if (client->GetClientSoft() == SO_LPHANT)
							iImage = 7;
						else if (client->GetClientSoft() == SO_EDONKEY )
							iImage = 8;
						else if (client->ExtProtocolAvailable())
						//MORPH START - Added by SiRoB, More client icon
						{
							if(client->GetModClient() == MOD_NONE)
								iImage = 1;
							else
								iImage = (uint8)(client->GetModClient() + 8);
						}
						//MORPH END   - Added by SiRoB, More client icon
						else
							iImage = 0;

						UINT nOverlayImage = 0;
						if ((client->Credits() && client->Credits()->GetCurrentIdentState(client->GetIP()) == IS_IDENTIFIED))
							nOverlayImage |= 1;
						if (client->IsObfuscatedConnectionEstablished())
							nOverlayImage |= 2;
						int iIconPosY = (cur_rec.Height() > 16) ? ((cur_rec.Height() - 16) / 2) : 1;
						POINT point = { cur_rec.left, cur_rec.top + iIconPosY };
						m_ImageList.Draw(dc, iImage, point, ILD_NORMAL | INDEXTOOVERLAYMASK(nOverlayImage));

						//MORPH START - Credit Overlay Icon
						if (client->Credits() && client->Credits()->GetHasScore(client->GetIP())) {
							if (nOverlayImage & 1)
								m_overlayimages.Draw(dc, 4, point, ILD_TRANSPARENT);
							else
								m_overlayimages.Draw(dc, 3, point, ILD_TRANSPARENT);
						}
						// Mighty Knife: Community visualization
						if (client->IsCommunity())
							m_overlayimages.Draw(dc,0, point, ILD_TRANSPARENT);
						// [end] Mighty Knife

						//MORPH START - Added by SiRoB, Friend Addon
						if (client->IsFriend())
							m_overlayimages.Draw(dc,client->GetFriendSlot()?2:1, point, ILD_TRANSPARENT);
						//MORPH END   - Added by SiRoB, Friend Addon

						//EastShare Start - added by AndCycle, IP to Country, modified by Commander
						if(theApp.ip2country->ShowCountryFlag() && IsColumnHidden(15)){
							cur_rec.left += 20;
							POINT point2= {cur_rec.left,cur_rec.top+1};
							//theApp.ip2country->GetFlagImageList()->DrawIndirect(dc, client->GetCountryFlagIndex(), point2, CSize(18,16), CPoint(0,0), ILD_NORMAL);
							theApp.ip2country->GetFlagImageList()->DrawIndirect(&theApp.ip2country->GetFlagImageDrawParams(dc,client->GetCountryFlagIndex(),point2));
							cur_rec.left += sm_iLabelOffset;
						}
						//EastShare End - added by AndCycle, IP to Country

						cur_rec.left += 16 + sm_iLabelOffset;
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
						cur_rec.left -= 16;
						cur_rec.right -= sm_iSubItemInset;

						//EastShare Start - added by AndCycle, IP to Country
						if(theApp.ip2country->ShowCountryFlag() && IsColumnHidden(15)){
							cur_rec.left -= 20;
							cur_rec.left -= sm_iLabelOffset;
						}
						//EastShare End - added by AndCycle, IP to Country

						break;
					}

					case 7:
						{
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
					// Commander - Added: IP2Country column - Start
					case 15:
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
					//MORPH START - Added by SiRoB, Display current uploading chunk
					case 16:
							cur_rec.bottom--;
							cur_rec.top++;
							client->DrawUpStatusBarChunk(dc,&cur_rec,false,thePrefs.UseFlatBar());
							client->DrawUpStatusBarChunkText(dc,&cur_rec); //Fafner: part number - 080317
							cur_rec.bottom++;
							cur_rec.top--;
						break;
					//MORPH END   - Added by SiRoB, Display current uploading chunk

					default:
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
						break;
				}
			}
			cur_rec.left += iColumnWidth;
		}
	}

	DrawFocusRect(dc, lpDrawItemStruct->rcItem, lpDrawItemStruct->itemState & ODS_FOCUS, bCtrlFocused, lpDrawItemStruct->itemState & ODS_SELECTED);
	if (!theApp.IsRunningAsService(SVC_LIST_OPT)) // MORPH leuk_he:run as ntservice v1..
		m_updatethread->AddItemUpdated((LPARAM)client); //MORPH - UpdateItemThread
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
			//MORPH START - Adde by SiRoB, Optimization requpfile
			/*
			const CKnownFile *file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
			*/
			const CKnownFile *file = client->CheckAndGetReqUpFile();
			//MORPH END - Adde by SiRoB, Optimization requpfile
			//Morph Start - added by AndCycle, Equal Chance For Each File
			//Morph - added by AndCycle, more detail...for debug?
			if(thePrefs.IsEqualChanceEnable() && file != NULL)
				_sntprintf(pszText, cchTextMax, _T("%s :%s"), file->statistic.GetEqualChanceValueString(false),file->GetFileName());
			else
			//Morph - added by AndCycle, more detail...for debug?
			//Morph End - added by AndCycle, Equal Chance For Each File
			_tcsncpy(pszText, file != NULL ? file->GetFileName() : _T(""), cchTextMax);
			break;
		}

		case 2:
			_tcsncpy(pszText, CastItoXBytes(client->GetDatarate(), false, true), cchTextMax);
			break;

		case 3:
			// NOTE: If you change (add/remove) anything which is displayed here, update also the sorting part..
			//Morph - modified by AndCycle, more uploading session info to show full chunk transfer
			/*
			if (thePrefs.m_bExtControls)
				_sntprintf(pszText, cchTextMax, _T("%s (%s)"), CastItoXBytes(client->GetSessionUp(), false, false), CastItoXBytes(client->GetQueueSessionPayloadUp(), false, false));
			else
				_tcsncpy(pszText, CastItoXBytes(client->GetSessionUp(), false, false), cchTextMax);
			*/
			if(client->GetSessionUp() == client->GetQueueSessionUp())
				_sntprintf(pszText, cchTextMax, _T("%s (%s)"), CastItoXBytes(client->GetQueueSessionPayloadUp(), false, false), CastItoXBytes(client->GetQueueSessionUp(), false, false));
			else
				_sntprintf(pszText, cchTextMax, _T("%s=%s+%s (%s=%s+%s)"), CastItoXBytes(client->GetQueueSessionPayloadUp()), CastItoXBytes(client->GetSessionPayloadUp()), CastItoXBytes(client->GetQueueSessionPayloadUp()-client->GetSessionPayloadUp()), CastItoXBytes(client->GetQueueSessionUp()), CastItoXBytes(client->GetSessionUp()), CastItoXBytes(client->GetQueueSessionUp()-client->GetSessionUp()));
			//Morph - modified by AndCycle, more uploading session info to show full chunk transfer
			break;

		case 4:
			if (client->HasLowID())
				//MORPH START - ZZ LowID handling
				/*
				_sntprintf(pszText, cchTextMax, _T("%s (%s)"), CastSecondsToHM(client->GetWaitTime() / 1000), GetResString(IDS_IDLOW));
				*/
				if(client->m_dwWouldHaveGottenUploadSlotIfNotLowIdTick)
					_sntprintf(pszText, cchTextMax, GetResString(IDS_UP_LOWID_DELAYED),CastSecondsToHM(client->GetWaitTime()/1000), CastSecondsToHM((::GetTickCount()-client->GetUpStartTimeDelay()-client->m_dwWouldHaveGottenUploadSlotIfNotLowIdTick)/1000));
				else
					_sntprintf(pszText, cchTextMax, GetResString(IDS_UP_LOWID),CastSecondsToHM(client->GetWaitTime()/1000));
				//MORPH END   - ZZ LowID handling
			else
				_tcsncpy(pszText, CastSecondsToHM(client->GetWaitTime() / 1000), cchTextMax);
			break;

		case 5:
			//MORPH START - modified by AndCycle, upRemain
			/*
			_tcsncpy(pszText, CastSecondsToHM(client->GetUpStartTimeDelay() / 1000), cchTextMax);
			*/
			{
				//MORPH START - Adde by SiRoB, Optimization requpfile
				/*
				const CKnownFile *file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
				*/
				const CKnownFile *file = client->CheckAndGetReqUpFile();
				//MORPH END - Adde by SiRoB, Optimization requpfile
				sint32 timeleft = -1;
				uint32 UpDatarate = client->GetDatarate();
				// Mighty Knife: Check for credits!=NULL
				if ((UpDatarate == 0) || (client->Credits()==NULL))
					timeleft = -1;
				else if(file)
					if(client->IsMoreUpThanDown(file) && client->GetQueueSessionUp() > SESSIONMAXTRANS)
						timeleft = (sint32)((client->Credits()->GetDownloadedTotal() - client->Credits()->GetUploadedTotal())/UpDatarate);
				// [end] Mighty Knife
					else if(file->GetPowerShared() && client->GetQueueSessionUp() > SESSIONMAXTRANS)
						timeleft = -1;
					else if (file->GetFileSize() > (uint64)SESSIONMAXTRANS)
						timeleft = (sint32)((SESSIONMAXTRANS - client->GetQueueSessionUp())/UpDatarate);
					else
						timeleft = (sint32)(((uint64)file->GetFileSize() - client->GetQueueSessionUp())/UpDatarate);
				_sntprintf(pszText, cchTextMax, _T("%s (+%s)"), CastSecondsToHM((client->GetUpStartTimeDelay())/1000), (timeleft>=0)?CastSecondsToHM(timeleft):_T("?"));
			}
			//MORPH END   - modified by AndCycle, upRemain
			break;

		case 6:
			_tcsncpy(pszText, client->GetUploadStateDisplayString(), cchTextMax);
			break;

		case 7:
			_tcsncpy(pszText, GetResString(IDS_UPSTATUS), cchTextMax);
			break;
		//MORPH START - Added by SiRoB, Client Software
		case 8:			
			_tcsncpy(pszText, client->GetClientSoftVer(), cchTextMax);
			break;
		//MORPH END - Added by SiRoB, Client Software

		//MORPH START - Added By Yun.SF3, Upload/Download
		case 9: //LSD Total UP/DL
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

		//MORPH START - Added By Yun.SF3, Remote Status
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
		//MORPH END - Added By Yun.SF3, Remote Status

		//MORPH START - Added by SiRoB, Upload Bandwidth Splited by class
		case 11:{
				CString Sbuffer;
				//MORPH START - Adde by SiRoB, Optimization requpfile
				/*
				const CKnownFile *file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
				*/
				const CKnownFile *file = client->CheckAndGetReqUpFile();
				//MORPH END - Adde by SiRoB, Optimization requpfile
				Sbuffer.Format(_T("%i"), client->GetSlotNumber());
				if (client->GetClassID()==0){
					Sbuffer.Append(_T(" FS"));
				} else
				//Morph - modified by AndCycle, take PayBackFirst have same class with PowerShare
				if (file && client->GetClassID()==1){
					if (client->IsMoreUpThanDown(file))
					{
						Sbuffer.Append(_T(",PBF"));
						if (client->Credits() && client->Credits()->GetDownloadedTotal() > client->Credits()->GetUploadedTotal())
							Sbuffer.AppendFormat( _T("(%s)"),
							CastItoXBytes((float)client->Credits()->GetDownloadedTotal()-
										(float)client->Credits()->GetUploadedTotal(),false,false));
					}
					//EastShare	Start - FairPlay by AndCycle
					if (!file->IsPartFile() && file->statistic.GetFairPlay()) {
						Sbuffer.Append(_T(",FairPlay"));
					}
					//EastShare	End   - FairPlay by AndCycle
					if (file->GetPowerShared())
						Sbuffer.Append(_T(",PS"));

					CString tempFilePrio;
					switch (file->GetUpPriority()) {
						case PR_VERYLOW : {
							tempFilePrio = GetResString(IDS_PRIOVERYLOW);
							break; }
						case PR_LOW : {
							if( file->IsAutoUpPriority() )
								tempFilePrio = GetResString(IDS_PRIOAUTOLOW);
							else
								tempFilePrio = GetResString(IDS_PRIOLOW);
							break; }
						case PR_NORMAL : {
							if( file->IsAutoUpPriority() )
								tempFilePrio = GetResString(IDS_PRIOAUTONORMAL);
							else
								tempFilePrio = GetResString(IDS_PRIONORMAL);
							break; }
						case PR_HIGH : {
							if( file->IsAutoUpPriority() )
								tempFilePrio = GetResString(IDS_PRIOAUTOHIGH);
							else
								tempFilePrio = GetResString(IDS_PRIOHIGH);
							break; }
						case PR_VERYHIGH : {
							tempFilePrio = GetResString(IDS_PRIORELEASE);
							break; }
						default:
							tempFilePrio.Empty();
					}
					Sbuffer.Append(_T(",") + tempFilePrio);
				}
				_tcsncpy(pszText, Sbuffer, cchTextMax);
				break;
			}
			//MORPH END   - Added by SiRoB, Upload Bandwidth Splited by class

			//MORPH START - Added by SiRoB, Show Compression by Tarod
			case 12:
				if (client->GetCompression() < 0.1f)
					_tcsncpy(pszText, _T("-"), cchTextMax);
				else
					_sntprintf(pszText, cchTextMax, _T("%.1f%%"), client->GetCompression());
				break;
			//MORPH END - Added by SiRoB, Show Compression by Tarod

			// Mighty Knife: Community affiliation
			case 13:
				_tcsncpy(pszText, client->IsCommunity() ? GetResString(IDS_YES) : _T(""), cchTextMax);
				break;
			// [end] Mighty Knife

			// EastShare - Added by Pretender, Friend Tab
			case 14:
				_tcsncpy(pszText, client->IsFriend() ? GetResString(IDS_YES) : _T(""), cchTextMax);
				break;
			// EastShare - Added by Pretender, Friend Tab

			case 15:
				_tcsncpy(pszText, client->GetCountryName(), cchTextMax);
				break;

			case 16:
				_tcsncpy(pszText, _T("Chunk Details"), cchTextMax);
				break;
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

			//MORPH START - Adde by SiRoB, Optimization requpfile
			/*
			const CKnownFile* file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
			*/
			const CKnownFile* file = client->CheckAndGetReqUpFile();
			//MORPH END   - Adde by SiRoB, Optimization requpfile
			// build info text and display it
			//MORPH START - Extra User Infos
			strInfo += GetResString(IDS_CD_CSOFT) + _T(": ") + client->GetClientSoftVer() + _T("\n");
			strInfo += GetResString(IDS_COUNTRY) + _T(": ") + client->GetCountryName(true) + _T("\n");
			//MORPH END   - Extra User Infos
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

		case 2:
			iResult = CompareUnsigned(item1->GetDatarate(), item2->GetDatarate());
			break;

		//Morph - modified by AndCycle, more uploading session info to show full chunk transfer
		/*
		case 3:
			iResult = CompareUnsigned(item1->GetSessionUp(), item2->GetSessionUp());
			if (iResult == 0 && thePrefs.m_bExtControls)
				iResult = CompareUnsigned(item1->GetQueueSessionPayloadUp(), item2->GetQueueSessionPayloadUp());
		*/
		case 3: 
			iResult=CompareUnsigned(item1->GetQueueSessionUp(), item2->GetQueueSessionUp());
		//Morph - modified by AndCycle, more uploading session info to show full chunk transfer
			break;

		case 4:
			iResult = CompareUnsigned(item1->GetWaitTime(), item2->GetWaitTime());
			break;

		case 5:
			iResult = CompareUnsigned(item1->GetUpStartTimeDelay() ,item2->GetUpStartTimeDelay());
			break;

		case 6:
			//MORPH START - Added by Stulle, Improved upload state sorting for additional information
			/*
			iResult = item1->GetUploadState() - item2->GetUploadState();
			*/
			iResult = CompareUnsigned(item1->GetUploadStateExtended() ,item2->GetUploadStateExtended());
			//MORPH END   - Added by Stulle, Improved upload state sorting for additional information
			break;

		case 7:
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
		case 8:
			/*
			iResult=item2->GetClientSoftVer().CompareNoCase(item1->GetClientSoftVer());
			*/
			if (item1->GetClientSoft() == item2->GetClientSoft())
				if (item2->GetVersion() == item1->GetVersion() && (item1->GetClientSoft() == SO_EMULE || item1->GetClientSoft() == SO_AMULE)){
					iResult= CompareOptLocaleStringNoCase(item2->GetClientSoftVer(), item1->GetClientSoftVer());
				}
				else {
					iResult= item1->GetVersion() - item2->GetVersion();
				}
			else
				iResult=-(item1->GetClientSoft() - item2->GetClientSoft());
			break;
		//MORPH END - Modified by SiRoB, Client Software

		//MORPH START - Added By Yun.SF3, Upload/Download
		case 9: // UP-DL TOTAL
			iResult=CompareUnsigned64(item2->Credits()->GetUploadedTotal(), item1->Credits()->GetUploadedTotal());
			break;
		//MORPH END - Added By Yun.SF3, Upload/Download

		//MORPH START - Added by SiRoB, ZZ Upload System
		case 11:
			iResult=CompareUnsigned(item1->GetSlotNumber(), item2->GetSlotNumber());
			break;
		//MORPH END - Added by SiRoB, ZZ Upload System 20030724-0336

		//MORPH START - Added by SiRoB, Show Compression by Tarod
		case 12:
			if (item1->GetCompression() == item2->GetCompression())
				iResult=0;
			else
				iResult=item1->GetCompression() > item2->GetCompression()?1:-1;
			break;
		//MORPH END - Added by SiRoB, Show Compression by Tarod
		
		// Mighty Knife: Community affiliation
		case 13:
			iResult=item1->IsCommunity() - item2->IsCommunity();
			break;
		// [end] Mighty Knife

		// EastShare - Added by Pretender, Friend Tab
		case 14:
			iResult=item1->IsFriend() - item2->IsFriend();
			break;
		// EastShare - Added by Pretender, Friend Tab

		// Commander - Added: IP2Country column - Start
		case 15:
			if(item1->GetCountryName(true) && item2->GetCountryName(true))
				iResult=CompareLocaleStringNoCase(item1->GetCountryName(true), item2->GetCountryName(true));
			else if(item1->GetCountryName(true))
				iResult=1;
			else
				iResult=-1;
			break;
		// Commander - Added: IP2Country column - End

		//MORPH START - Display current uploading chunk
		case 16:
			if (item1->GetUpChunkProgressPercent() == item2->GetUpChunkProgressPercent())
				iResult=0;
			else
				iResult=item1->GetUpChunkProgressPercent() > item2->GetUpChunkProgressPercent()?1:-1;
			break;
		//MORPH END   - Display current uploading chunk
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
	// SLUGFILLER: multiSort remove - handled in parent class

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
	if(!thePrefs.IsLessControls()){ //MORPH show less controls
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && !client->IsFriend()) ? MF_ENABLED : MF_GRAYED), MP_ADDFRIEND, GetResString(IDS_ADDFRIEND), _T("ADDFRIEND"));
	//MORPH START - Added by SiRoB, Friend Addon
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->IsFriend()) ? MF_ENABLED : MF_GRAYED), MP_REMOVEFRIEND, GetResString(IDS_REMOVEFRIEND), _T("DELETEFRIEND"));
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->IsFriend()) ? MF_ENABLED  | ((!client->HasLowID() && client->IsFriend() && client->GetFriendSlot())?MF_CHECKED : MF_UNCHECKED) : MF_GRAYED), MP_FRIENDSLOT, GetResString(IDS_FRIENDSLOT), _T("FRIENDSLOT"));
	//MORPH END - Added by SiRoB, Friend Addon
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED), MP_MESSAGE, GetResString(IDS_SEND_MSG), _T("SENDMESSAGE"));
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED), MP_SHOWLIST, GetResString(IDS_VIEWFILES), _T("VIEWFILES"));
	if (Kademlia::CKademlia::IsRunning() && !Kademlia::CKademlia::IsConnected())
		ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetKadPort()!=0 && client->GetKadVersion() > 1) ? MF_ENABLED : MF_GRAYED), MP_BOOT, GetResString(IDS_BOOTSTRAP));
	ClientMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED), MP_FIND, GetResString(IDS_FIND), _T("Search"));
	
	//MORPH START - Added by Yun.SF3, List Requested Files
	ClientMenu.AppendMenu(MF_SEPARATOR); // Added by sivka
	ClientMenu.AppendMenu(MF_STRING | (client ? MF_ENABLED : MF_GRAYED),MP_LIST_REQUESTED_FILES, GetResString(IDS_LISTREQUESTED), _T("FILEREQUESTED")); // Added by sivka
	//MORPH END - Added by Yun.SF3, List Requested Files
	//MORPH START show less controls
	}
	else
	{
		ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED), MP_MESSAGE, GetResString(IDS_SEND_MSG), _T("SENDMESSAGE"));
		ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED), MP_SHOWLIST, GetResString(IDS_VIEWFILES), _T("VIEWFILES"));
		ClientMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED), MP_FIND, GetResString(IDS_FIND), _T("Search"));
	}
	//MORPH END show less controls
	GetPopupMenuPos(*this, point);
	ClientMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
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
			//MORPH START - Addded by SiRoB, Friend Addon
			case MP_REMOVEFRIEND:{//LSD
				if (client && client->IsFriend())
				{
					theApp.friendlist->RemoveFriend(client->m_Friend);
					RefreshClient(client);
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
					RefreshClient(client);
				}
				//MORPH END - Modified by SIRoB, Added by Yun.SF3, ZZ Upload System
				break;
			}
			//Xman end
			//MORPH END   - Added by SiRoB, Friend Addon
			//MORPH START - Added by Yun.SF3, List Requested Files
			case MP_LIST_REQUESTED_FILES: { // added by sivka
				if (client != NULL)
				{
					client->ShowRequestedFiles(); //Changed by SiRoB
				}
				break;
			}
			//MORPH END - Added by Yun.SF3, List Requested Files
		}
	}
	return true;
}

void CUploadListCtrl::AddClient(const CUpDownClient *client)
{
	if (theApp.IsRunningAsService(SVC_LIST_OPT)) return;// MORPH leuk_he:run as ntservice v1..

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
	if (theApp.IsRunningAsService(SVC_LIST_OPT)) return;// MORPH leuk_he:run as ntservice v1..

	if (!theApp.emuledlg->IsRunning())
		return;

	if (theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd || !theApp.emuledlg->transferwnd->m_pwndTransfer->uploadlistctrl.IsWindowVisible())
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

