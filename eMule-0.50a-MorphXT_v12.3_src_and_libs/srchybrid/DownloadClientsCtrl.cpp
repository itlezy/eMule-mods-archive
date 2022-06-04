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
#include "emuledlg.h"
#include "DownloadClientsCtrl.h"
#include "ClientDetailDialog.h"
#include "MemDC.h"
#include "MenuCmds.h"
#include "FriendList.h"
#include "TransferDlg.h"
#include "ChatWnd.h"
#include "UpDownClient.h"
#include "UploadQueue.h"
#include "ClientCredits.h"
#include "PartFile.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "SharedFileList.h"
#include "IP2Country.h" //EastShare - added by AndCycle, IP to Country

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CDownloadClientsCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CDownloadClientsCtrl, CMuleListCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnClick)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnLvnGetDispInfo)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNmDblClk)
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

CDownloadClientsCtrl::CDownloadClientsCtrl()
	: CListCtrlItemWalk(this)
{
	SetGeneralPurposeFind(true);
	SetSkinKey(L"DownloadingLv");
}

void CDownloadClientsCtrl::Init()
{
	SetPrefsKey(_T("DownloadClientsCtrl"));
	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	InsertColumn(0,	GetResString(IDS_QL_USERNAME),		LVCFMT_LEFT,  DFLT_CLIENTNAME_COL_WIDTH);
	InsertColumn(1,	GetResString(IDS_CD_CSOFT),			LVCFMT_LEFT,  DFLT_CLIENTSOFT_COL_WIDTH);
	InsertColumn(2,	GetResString(IDS_FILE),				LVCFMT_LEFT,  DFLT_FILENAME_COL_WIDTH);
	InsertColumn(3,	GetResString(IDS_DL_SPEED),			LVCFMT_RIGHT, DFLT_DATARATE_COL_WIDTH);
	InsertColumn(4, GetResString(IDS_AVAILABLEPARTS),	LVCFMT_LEFT,  DFLT_PARTSTATUS_COL_WIDTH);
	InsertColumn(5,	GetResString(IDS_CL_TRANSFDOWN),	LVCFMT_RIGHT, DFLT_SIZE_COL_WIDTH);
	InsertColumn(6,	GetResString(IDS_CL_TRANSFUP),		LVCFMT_RIGHT, DFLT_SIZE_COL_WIDTH);
	InsertColumn(7,	GetResString(IDS_META_SRCTYPE),		LVCFMT_LEFT,  100);
	InsertColumn(8,	GetResString(IDS_DL_ULDL),		LVCFMT_LEFT, 60);
	//MORPH START - Added by SiRoB, Display current uploading chunk
	InsertColumn(9,	GetResString(IDS_CHUNK),			LVCFMT_LEFT, 100);
	//MORPH END   - Added by SiRoB, Display current uploading chunk
	InsertColumn(10, GetResString(IDS_LAST_ASKED),		LVCFMT_LEFT, 100); //SLAHAM: ADDED Last Asked 
	InsertColumn(11, GetResString(IDS_DOWNL_TIME),		LVCFMT_LEFT, 100); //SLAHAM: ADDED Downloading Time
	InsertColumn(12, GetResString(IDS_KNOWN_SINCE),		LVCFMT_LEFT, 100); //SLAHAM: ADDED Known Since

	// EastShare - Added by Pretender: IP2Country column
	if (thePrefs.GetIP2CountryNameMode() == IP2CountryName_DISABLE)
		InsertColumn(13,GetResString(IDS_COUNTRY),		LVCFMT_LEFT, 50, -1, true);
	else
		InsertColumn(13,GetResString(IDS_COUNTRY),		LVCFMT_LEFT, 50);
	// EastShare - Added by Pretender: IP2Country column

	SetAllIcons();
	Localize();
	LoadSettings();
	SetSortArrow();
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0 : 100));
}

void CDownloadClientsCtrl::Localize()
{
	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;

	CString strRes;
	strRes = GetResString(IDS_QL_USERNAME);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(0, &hdi);

	strRes = GetResString(IDS_CD_CSOFT);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(1, &hdi);

	strRes = GetResString(IDS_FILE);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(2, &hdi);

	strRes = GetResString(IDS_DL_SPEED);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(3, &hdi);
	
	strRes = GetResString(IDS_AVAILABLEPARTS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(4, &hdi);

	strRes = GetResString(IDS_CL_TRANSFDOWN);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(5, &hdi);

	strRes = GetResString(IDS_CL_TRANSFUP);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(6, &hdi);

	strRes = GetResString(IDS_META_SRCTYPE);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(7, &hdi);

	strRes = GetResString(IDS_DL_ULDL);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(8, &hdi);

	//MORPH START - Added by SiRoB, Display current uploading chunk
	strRes = GetResString(IDS_CHUNK);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(9, &hdi);
	//MORPH START - Added by SiRoB, Display current uploading chunk

	//SLAHAM: ADDED Last Asked =>
	strRes = GetResString(IDS_LAST_ASKED);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(10, &hdi);
	//SLAHAM: ADDED Last Asked <=

	//SLAHAM: ADDED Downloading Time =>
	strRes = GetResString(IDS_DOWNL_TIME);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(11, &hdi);
	//SLAHAM: ADDED Downloading Time <=

	//SLAHAM: ADDED Known Since =>
	strRes = GetResString(IDS_KNOWN_SINCE);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(12, &hdi);
	//SLAHAM: ADDED Known Since <=

	//MORPH START - Added by SiRoB, IP2Country column
	strRes = GetResString(IDS_COUNTRY);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(13, &hdi);
	//MORPH END   - Added by SiRoB, IP2Country column
}

void CDownloadClientsCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
}

void CDownloadClientsCtrl::SetAllIcons()
{
	ApplyImageList(NULL);
	m_ImageList.DeleteImageList();
	m_ImageList.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkey")));
	m_ImageList.Add(CTempIconLoader(_T("ClientCompatible")));
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkey")));
	m_ImageList.Add(CTempIconLoader(_T("ClientCompatible")));
	m_ImageList.Add(CTempIconLoader(_T("Friend")));
	m_ImageList.Add(CTempIconLoader(_T("ClientMLDonkey")));
	m_ImageList.Add(CTempIconLoader(_T("ClientMLDonkey")));
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyHybrid")));
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyHybrid")));
	m_ImageList.Add(CTempIconLoader(_T("ClientShareaza")));
	m_ImageList.Add(CTempIconLoader(_T("ClientShareaza")));
	m_ImageList.Add(CTempIconLoader(_T("ClientAMule")));
	m_ImageList.Add(CTempIconLoader(_T("ClientAMule")));
	m_ImageList.Add(CTempIconLoader(_T("ClientLPhant")));
	m_ImageList.Add(CTempIconLoader(_T("ClientLPhant")));
	//MORPH START - Added by SiRoB, More client & Credit Overlay Icon
	m_ImageList.Add(CTempIconLoader(_T("ClientRightEdonkey")));
	m_ImageList.Add(CTempIconLoader(_T("Server")));
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
	//MORPH END   - Added by SiRoB, More client & Credit Overlay Icon
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("ClientSecureOvl"))), 1);
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("OverlayObfu"))), 2);
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("OverlaySecureObfu"))), 3);
	
	// Mighty Knife: Community icon
	m_overlayimages.DeleteImageList ();
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

void CDownloadClientsCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	if (!lpDrawItemStruct->itemData)
		return;

	CMemDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	BOOL bCtrlFocused;
	InitItemMemDC(dc, lpDrawItemStruct, bCtrlFocused);
	CRect cur_rec(lpDrawItemStruct->rcItem);
	CRect rcClient;
	GetClientRect(&rcClient);
	const CUpDownClient *client = (CUpDownClient *)lpDrawItemStruct->itemData;

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
						
						//MORPH - Removed by SiRoB, Friend Addon
						/*
						/*if (client->IsFriend())
							iImage = 4;
						else*/ if (client->GetClientSoft() == SO_EDONKEYHYBRID){
							iImage = 7;
						}
						else if (client->GetClientSoft() == SO_MLDONKEY){
							iImage = 5;
						}
						else if (client->GetClientSoft() == SO_SHAREAZA){
							iImage = 9;
						}
						else if (client->GetClientSoft() == SO_AMULE){
							iImage = 11;
						}
						else if (client->GetClientSoft() == SO_LPHANT){
							iImage = 13;
						}
						//MORPH START - Added by Stulle, Server Icon
						else if(client->GetClientSoft() == SO_URL){
							iImage = 16;
						}
						//MORPH END   - Added by Stulle, Server Icon
						else if (client->ExtProtocolAvailable()){
							//MORPH START - Added by SiRoB, More client icon
							if(client->GetModClient() == MOD_NONE)
								iImage = 1;
							else
								iImage = (uint8)(client->GetModClient() + 16);
							//MORPH END   - Added by SiRoB, More client icon
						}
						else if (client->GetClientSoft() == SO_EDONKEY){
							iImage = 15;
						}
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
						if (client->Credits() && client->Credits()->GetMyScoreRatio(client->GetIP()) > 1) {
							if (nOverlayImage & 1)
								m_overlayimages.Draw(dc, 4, point, ILD_TRANSPARENT);
							else
								m_overlayimages.Draw(dc, 3, point, ILD_TRANSPARENT);
						}
						//MORPH END - Credit Overlay Icon
						//MORPH START - Added by SiRoB, Friend Addon
						if (client->IsFriend())
							m_overlayimages.Draw(dc, client->GetFriendSlot()?2:1,point, ILD_TRANSPARENT);
						//MORPH END   - Added by SiRoB, Friend Addon
						// Mighty Knife: Community visualization
						if (client->IsCommunity())
							m_overlayimages.Draw(dc,0, point, ILD_TRANSPARENT);
						// [end] Mighty Knife
						//MORPH END - Modified by SiRoB, More client & ownCredits overlay icon

						//EastShare Start - added by AndCycle, IP to Country 
						if(theApp.ip2country->ShowCountryFlag() && IsColumnHidden(13)){
							cur_rec.left+=20;
							POINT point2= {cur_rec.left,cur_rec.top+1};
							int index = client->GetCountryFlagIndex();
							theApp.ip2country->GetFlagImageList()->DrawIndirect(dc, index , point2, CSize(18,16), CPoint(0,0), ILD_NORMAL);
							cur_rec.left += sm_iLabelOffset;
						}
						//EastShare End - added by AndCycle, IP to Country

						cur_rec.left += 16 + sm_iLabelOffset;
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
						cur_rec.left -= 16;
						cur_rec.right -= sm_iSubItemInset;

						//EastShare Start - added by AndCycle, IP to Country
						if(theApp.ip2country->ShowCountryFlag() && IsColumnHidden(13)){
							cur_rec.left-=20;
							cur_rec.left -= sm_iLabelOffset;
						}
						//EastShare End - added by AndCycle, IP to Country
						break;
					}

					case 4:
						{
						cur_rec.bottom--;
						cur_rec.top++;

						client->DrawStatusBar(dc, &cur_rec, client->GetRequestFile(), thePrefs.UseFlatBar());
						CString buffer;

						//SLAHAM: ADDED Client Percentage =>
						if (thePrefs.GetUseClientPercentage() && client->GetPartStatus())
						{
							float percent = (float)client->GetAvailablePartCount() / (float)client->GetPartCount()* 100.0f;
							buffer.Format(_T("%.1f%%"), percent);

							if (percent > 0.05f)
							{
								//Commander - Added: Draw Client Percentage xored, caching before draw - Start
								COLORREF oldclr = dc.SetTextColor(RGB(0,0,0));
								int iOMode = dc.SetBkMode(TRANSPARENT);
								buffer.Format(_T("%.1f%%"), percent);
								CFont *pOldFont = dc.SelectObject(&theApp.emuledlg->transferwnd->GetDownloadList()->m_fontBoldSmaller);
#define	DrawClientPercentText		dc.DrawText(buffer, buffer.GetLength(),&cur_rec, ((MLC_DT_TEXT | DT_RIGHT) & ~DT_LEFT) | DT_CENTER)
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
								//Commander - Added: Draw Client Percentage xored, caching before draw - End	
							}
							//SLAHAM: ADDED Client Percentage <=

							cur_rec.bottom++;
							cur_rec.top--;
						}
						break;
					}	

					//MORPH START - Added by SiRoB, Display current uploading chunk
					case 9:
					{
						cur_rec.bottom--;
						cur_rec.top++;
						client->DrawStatusBarChunk(dc,&cur_rec,client->GetRequestFile(),thePrefs.UseFlatBar());
						CString buffer;
						COLORREF oldclr = dc.SetTextColor(RGB(0,0,0));
						int iOMode = dc.SetBkMode(TRANSPARENT);
						if (client->GetCurrentDownloadingChunk()==(UINT)-1) {
							if (client->m_lastPartAsked==(uint16)-1)
								buffer = _T("?");
							else
								buffer.Format(_T("%u"), client->m_lastPartAsked);
						} else
							buffer.Format(_T("%u"), client->GetCurrentDownloadingChunk());
						buffer.AppendFormat(_T(" @ %.1f%%"), client->GetDownChunkProgressPercent());
						CFont *pOldFont = dc.SelectObject(&theApp.emuledlg->transferwnd->GetDownloadList()->m_fontSmaller);
#define	DrawClientPercentTextLeft		dc.DrawText(buffer, buffer.GetLength(),&cur_rec, MLC_DT_TEXT)
						cur_rec.top-=1;cur_rec.bottom-=1;
						cur_rec.left+=1;cur_rec.right-=3;
						DrawClientPercentTextLeft;cur_rec.left+=1;cur_rec.right+=1;
						DrawClientPercentTextLeft;cur_rec.left+=1;cur_rec.right+=1;
						DrawClientPercentTextLeft;cur_rec.top+=1;cur_rec.bottom+=1;
						DrawClientPercentTextLeft;cur_rec.top+=1;cur_rec.bottom+=1;
						DrawClientPercentTextLeft;cur_rec.left-=1;cur_rec.right-=1;
						DrawClientPercentTextLeft;cur_rec.left-=1;cur_rec.right-=1;
						DrawClientPercentTextLeft;cur_rec.top-=1;cur_rec.bottom-=1;
						DrawClientPercentTextLeft;cur_rec.left++;cur_rec.right++;
						dc.SetTextColor(RGB(255,255,255));
						DrawClientPercentTextLeft;
						
						dc.SetTextColor(RGB(0,0,0));
						buffer.Format(_T("%s"), CastItoXBytes(client->GetSessionPayloadDown(), false, false));
#define	DrawClientPercentTextRight		dc.DrawText(buffer, buffer.GetLength(),&cur_rec, MLC_DT_TEXT | DT_RIGHT)
						cur_rec.top-=1;cur_rec.bottom-=1;
						DrawClientPercentTextRight;cur_rec.left+=1;cur_rec.right+=1;
						DrawClientPercentTextRight;cur_rec.left+=1;cur_rec.right+=1;
						DrawClientPercentTextRight;cur_rec.top+=1;cur_rec.bottom+=1;
						DrawClientPercentTextRight;cur_rec.top+=1;cur_rec.bottom+=1;
						DrawClientPercentTextRight;cur_rec.left-=1;cur_rec.right-=1;
						DrawClientPercentTextRight;cur_rec.left-=1;cur_rec.right-=1;
						DrawClientPercentTextRight;cur_rec.top-=1;cur_rec.bottom-=1;
						DrawClientPercentTextRight;cur_rec.left++;cur_rec.right++;
						dc.SetTextColor(RGB(255,255,255));
						DrawClientPercentTextRight;

						dc.SelectObject(pOldFont);
						dc.SetBkMode(iOMode);
						dc.SetTextColor(oldclr);
						cur_rec.bottom++;
						cur_rec.top--;
					}
					break;
					//MORPH END   - Added by SiRoB, Display current uploading chunk

					// EastShare - Added by Pretender: IP2Country column
					case 13:
						if(theApp.ip2country->ShowCountryFlag()){
							POINT point2= {cur_rec.left,cur_rec.top+1};
							int index = client->GetCountryFlagIndex();
							theApp.ip2country->GetFlagImageList()->DrawIndirect(dc, index , point2, CSize(18,16), CPoint(0,0), ILD_NORMAL);
							cur_rec.left+=20;
						}
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
						if(theApp.ip2country->ShowCountryFlag()){
							cur_rec.left-=20;
						}
						break;
					// EastShare - Added by Pretender: IP2Country column

					default:
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
						break;
				}
			}
			cur_rec.left += iColumnWidth;
		}
	}

	DrawFocusRect(dc, lpDrawItemStruct->rcItem, lpDrawItemStruct->itemState & ODS_FOCUS, bCtrlFocused, lpDrawItemStruct->itemState & ODS_SELECTED);
}

void CDownloadClientsCtrl::GetItemDisplayText(const CUpDownClient *client, int iSubItem, LPTSTR pszText, int cchTextMax)
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

		case 1:
			_tcsncpy(pszText, client->GetClientSoftVer(), cchTextMax);
			break;

		case 2:
			_tcsncpy(pszText, client->GetRequestFile()->GetFileName(), cchTextMax);
			break;
		
		case 3:
			_tcsncpy(pszText, CastItoXBytes((float)client->GetDownloadDatarate(), false, true), cchTextMax);
			break;
		
		case 4:
			_tcsncpy(pszText, GetResString(IDS_AVAILABLEPARTS), cchTextMax);
			break;

		case 5:
			if (client->credits && client->GetSessionDown() < client->credits->GetDownloadedTotal())
				_sntprintf(pszText, cchTextMax, _T("%s (%s)"), CastItoXBytes(client->GetSessionDown()), CastItoXBytes(client->credits->GetDownloadedTotal()));
			else
				_tcsncpy(pszText, CastItoXBytes(client->GetSessionDown()), cchTextMax);
			break;
		
		case 6:
			if (client->credits && client->GetSessionUp() < client->credits->GetUploadedTotal())
				_sntprintf(pszText, cchTextMax, _T("%s (%s)"), CastItoXBytes(client->GetSessionUp()), CastItoXBytes(client->credits->GetUploadedTotal()));
			else
				_tcsncpy(pszText, CastItoXBytes(client->GetSessionUp()), cchTextMax);
			break;
		
		case 7:
			switch (client->GetSourceFrom())
			{
				case SF_SERVER:
					_tcsncpy(pszText, _T("eD2K Server"), cchTextMax);
					break;
				case SF_KADEMLIA:
					_tcsncpy(pszText, GetResString(IDS_KADEMLIA), cchTextMax);
					break;
				case SF_SOURCE_EXCHANGE:
					_tcsncpy(pszText, GetResString(IDS_SE), cchTextMax);
					break;
				case SF_PASSIVE:
					_tcsncpy(pszText, GetResString(IDS_PASSIVE), cchTextMax);
					break;
				case SF_LINK:
					_tcsncpy(pszText, GetResString(IDS_SW_LINK), cchTextMax);
					break;
				//MORPH - Source cache
				case SF_CACHE_SERVER:
					_tcsncpy(pszText, _T("eD2K Server (SC)"), cchTextMax);
					break;
				//MORPH - Source cache
				//MORPH - Source cache
				case SF_CACHE_SOURCE_EXCHANGE:
					_tcsncpy(pszText, GetResString(IDS_SE) + _T(" (SC)"), cchTextMax);
					break;
				//MORPH - Source cache
				//MORPH START - Added by SiRoB, Source Loader Saver [SLS]
				case SF_SLS:
					_tcsncpy(pszText, GetResString(IDS_SOURCE_LOADER_SAVER), cchTextMax);
					break;
				//MORPH END   - Added by SiRoB, Source Loader Saver [SLS]
				default:
					_tcsncpy(pszText, GetResString(IDS_UNKNOWN), cchTextMax);
					break;
			}
			break;

		case 8:
			if(client->Credits())
				_sntprintf(pszText, cchTextMax, _T("%.1f/%.1f"),client->credits->GetScoreRatio(client->GetIP()), client->credits->GetMyScoreRatio(client->GetIP()));
			else
				_tcsncpy(pszText, _T(""), cchTextMax);
			break;

			//MORPH START - Display Chunk Detail
		case 9:
			_tcsncpy(pszText, GetResString(IDS_CHUNK), cchTextMax);
			break;
			//MORPH END - Display Chunk Detail

			//SLAHAM: ADDED Last Asked Counter =>
		case 10:
			if( client->GetLastAskedTime() )
				_sntprintf(pszText, cchTextMax, _T(" %s [%u]"), CastSecondsToHM((::GetTickCount()-client->GetLastAskedTime())/1000), client->uiDLAskingCounter);
			else
				_sntprintf(pszText, cchTextMax, _T("Reseted [%u]"), client->uiDLAskingCounter);
			break;
			//SLAHAM: ADDED Last Asked Counter <=

			//SLAHAM: ADDED Show Downloading Time =>
		case 11:	
			_sntprintf(pszText, cchTextMax, _T(" %s (%s)[%u]"), CastSecondsToHM(client->dwSessionDLTime/1000), CastSecondsToHM(client->dwTotalDLTime/1000), client->uiStartDLCount);
			break;
			//SLAHAM: ADDED Show Downloading Time <=

			//SLAHAM: ADDED Known Since =>
		case 12: 
			if( client->dwThisClientIsKnownSince )
				_sntprintf(pszText, cchTextMax, _T(" %s"), CastSecondsToHM((::GetTickCount()-client->dwThisClientIsKnownSince)/1000));
			else
				_tcsncpy(pszText, _T("WHO IS THAT???"), cchTextMax);
			break;
			//SLAHAM: ADDED Known Since <=

			// EastShare - Added by Pretender: IP2Country column
		case 13:
			_tcsncpy(pszText, client->GetCountryName(), cchTextMax);
			break;
			// EastShare - Added by Pretender: IP2Country column
	}
	pszText[cchTextMax - 1] = _T('\0');
}

void CDownloadClientsCtrl::OnLvnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
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

void CDownloadClientsCtrl::OnLvnColumnClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLISTVIEW *pNMListView = (NMLISTVIEW *)pNMHDR;
	bool sortAscending;
	if (GetSortItem() != pNMListView->iSubItem)
	{
		switch (pNMListView->iSubItem)
		{
			case 1: // Client Software
			case 3: // Download Rate
			case 4: // Part Count
			case 5: // Session Down
			case 6: // Session Up
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

int CDownloadClientsCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
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

		case 1:
			//MORPH START - Added by IceCream, ET_MOD_VERSION
			/*
			if (item1->GetClientSoft() == item2->GetClientSoft())
			    iResult = item1->GetVersion() - item2->GetVersion();
		    else 
				iResult = -(item1->GetClientSoft() - item2->GetClientSoft()); // invert result to place eMule's at top
			break;
			*/
			if (item1->GetClientSoft() == item2->GetClientSoft())
				if (item2->GetVersion() == item1->GetVersion() && item1->GetClientSoft() == SO_EMULE){
					iResult= CompareOptLocaleStringNoCase(item2->GetClientSoftVer(), item1->GetClientSoftVer());
				}
				else {
					iResult= item1->GetVersion() - item2->GetVersion();
				}
			else
				iResult=-(item1->GetClientSoft() - item2->GetClientSoft());
			break;
			//MORPH END   - Added by IceCream, ET_MOD_VERSION

		case 2: {
			const CKnownFile *file1 = item1->GetRequestFile();
			const CKnownFile *file2 = item2->GetRequestFile();
			if( (file1 != NULL) && (file2 != NULL))
				iResult = CompareLocaleStringNoCase(file1->GetFileName(), file2->GetFileName());
			else if (file1 == NULL)
				iResult = 1;
			else
				iResult = -1;
			break;
		}

		case 3:
			iResult = CompareUnsigned(item1->GetDownloadDatarate(), item2->GetDownloadDatarate());
			break;

		case 4:
			//MORPH START - Changed by Stulle, Sort by percentage
			/*
			iResult = CompareUnsigned(item1->GetPartCount(), item2->GetPartCount());
			*/
		{
			float percent1 = (float)item1->GetAvailablePartCount() / (float)item1->GetPartCount()* 100.0f;
			float percent2 = (float)item2->GetAvailablePartCount() / (float)item2->GetPartCount()* 100.0f;
			if (percent1 == percent2)
				iResult=0;
			else
				iResult=percent1 > percent2?1:-1;
		}
			//MORPH END   - Changed by Stulle, Sort by percentage
			break;

		case 5:
			iResult = CompareUnsigned(item1->GetSessionDown(), item2->GetSessionDown());
			break;

		case 6:
			iResult = CompareUnsigned(item1->GetSessionUp(), item2->GetSessionUp());
			break;

		case 7:
			iResult = item1->GetSourceFrom() - item2->GetSourceFrom();
			break;

		case 8:
			{   
				if (!item1->Credits()) 
					iResult=1; 
				else if (!item2->Credits())   
					iResult=-1;
				float r1=item2->credits->GetScoreRatio(item2->GetIP());
				float r2=item1->credits->GetScoreRatio(item1->GetIP());
				if (r1==r2)
					iResult=0;
				else if (r1<r2)
					iResult=-1;
				else
					iResult=1;
				break;
			}

		//MORPH START - Display Chunk Detail
		case 9:
			if (item1->GetDownChunkProgressPercent() == item2->GetDownChunkProgressPercent())
				iResult=0;
			else
				iResult=item1->GetDownChunkProgressPercent() > item2->GetDownChunkProgressPercent()?1:-1;
			break;
		//MORPH END - Display Chunk Detail

		//SLAHAM: ADDED Last Asked =>
		case 10: 
			{
				uint32 lastAskedTime1 = item2->GetLastAskedTime();
				uint32 lastAskedTime2 = item1->GetLastAskedTime();
				iResult=lastAskedTime1==lastAskedTime2? 0 : lastAskedTime1<lastAskedTime2? -1 : 1;
				break;
			}
		//SLAHAM: ADDED Last Asked <=

		//SLAHAM: ADDED Show Downloading Time =>
		case 11:
			iResult=CompareUnsigned(item2->dwSessionDLTime, item1->dwSessionDLTime);
			break;
		//SLAHAM: ADDED Show Downloading Time <=

		//SLAHAM: ADDED Known Since =>
		case 12:
			{
				uint32 known1 = item2->dwThisClientIsKnownSince;
				uint32 known2 = item1->dwThisClientIsKnownSince;
				iResult=known1==known2? 0 : known1<known2? -1 : 1;
				break;
			}
			//SLAHAM: ADDED Known Since <=

		// EastShare - Added by Pretender: IP2Country column
		case 13:
			if(item1->GetCountryName(true) && item2->GetCountryName(true))
				iResult=CompareLocaleStringNoCase(item1->GetCountryName(true), item2->GetCountryName(true));
			else if(item1->GetCountryName(true))
				iResult=1;
			else
				iResult=-1;
			break;
		// EastShare - Added by Pretender: IP2Country column		
	}

	if (lParamSort >= 100)
		iResult = -iResult;

	// SLUGFILLER: multiSort remove - handled in parent class
	/*
	//call secondary sortorder, if this one results in equal
	int dwNextSort;
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->transferwnd->GetDownloadClientsList()->GetNextSortOrder(lParamSort)) != -1)
		iResult = SortProc(lParam1, lParam2, dwNextSort);
	*/
	// SLUGFILLER: multiSort remove - handled in parent class

	return iResult;
}

void CDownloadClientsCtrl::OnNmDblClk(NMHDR* /*pNMHDR*/, LRESULT* pResult)
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

void CDownloadClientsCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
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
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->IsFriend()) ? MF_ENABLED  | ((!client->HasLowID() && client->GetFriendSlot())?MF_CHECKED : MF_UNCHECKED) : MF_GRAYED), MP_FRIENDSLOT, GetResString(IDS_FRIENDSLOT), _T("FRIENDSLOT"));
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

BOOL CDownloadClientsCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
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
			//MORPH START - Modified by SiRoB, Added by Yun.SF3, ZZ Upload System
			case MP_REMOVEFRIEND:{
				if (client && client->IsFriend()){					
					theApp.friendlist->RemoveFriend(client->m_Friend);
					theApp.emuledlg->transferwnd->GetDownloadList()->UpdateItem(client);
				}
				break;
			 }

			case MP_FRIENDSLOT:{
				if (client){
					bool IsAlready;
					IsAlready = client->GetFriendSlot();
					//theApp.friendlist->RemoveAllFriendSlots();
					if( IsAlready ) {
						client->SetFriendSlot(false);
					}else 
					{
						client->SetFriendSlot(true);
					}
					theApp.friendlist->ShowFriends();
					theApp.emuledlg->transferwnd->GetDownloadList()->UpdateItem(client);
				}
				break;
			}
			//MORPH END - Modified by SiRoB, Added by Yun.SF3, ZZ Upload System
			//MORPH START - Added by Yun.SF3, List Requested Files
			case MP_LIST_REQUESTED_FILES:
				if (client != NULL)
					client->ShowRequestedFiles(); //Changed by SiRoB
				break;
			//MORPH END - Added by Yun.SF3, List Requested Files
		}
	}
	return true;
}

void CDownloadClientsCtrl::AddClient(const CUpDownClient *client)
{
	if (theApp.IsRunningAsService(SVC_LIST_OPT)) return;// MORPH leuk_he:run as ntservice v1..

	if (!theApp.emuledlg->IsRunning())
		return;

	int iItemCount = GetItemCount();
	InsertItem(LVIF_TEXT | LVIF_PARAM, iItemCount, LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)client);
	theApp.emuledlg->transferwnd->UpdateListCount(CTransferDlg::wnd2Downloading, iItemCount + 1);
}

void CDownloadClientsCtrl::RemoveClient(const CUpDownClient *client)
{
	if (!theApp.emuledlg->IsRunning())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	int result = FindItem(&find);
	if (result != -1) {
		DeleteItem(result);
		theApp.emuledlg->transferwnd->UpdateListCount(CTransferDlg::wnd2Downloading, GetItemCount()); 
	}
}

void CDownloadClientsCtrl::RefreshClient(const CUpDownClient *client)
{
	if (theApp.IsRunningAsService(SVC_LIST_OPT)) return;// MORPH leuk_he:run as ntservice v1..

	if (!theApp.emuledlg->IsRunning())
		return;

	if (theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd || !theApp.emuledlg->transferwnd->GetDownloadClientsList()->IsWindowVisible())
		return;

	//MORPH START - UpdateItemThread
	/*
	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	int result = FindItem(&find);
	if (result != -1)
		Update(result);
	*/
	m_updatethread->AddItemToUpdate((LPARAM)client);
	//MORPH END - UpdateItemThread
}

void CDownloadClientsCtrl::ShowSelectedUserDetails()
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