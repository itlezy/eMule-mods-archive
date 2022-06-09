//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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
#include "updownclient.h"
#include "UploadListCtrl.h"
#include "SharedFileList.h"
#include "TitleMenu.h"
#include "otherfunctions.h"
#include "opcodes.h"
#include "Details\clientdetails.h"
#include "IP2Country.h"
#include "MemDC.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CUploadListCtrl, CMuleListCtrl)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(NM_CLICK, OnNMClick)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclkUploadlist)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
	ON_WM_KEYUP()
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CUploadListCtrl, CMuleListCtrl)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CUploadListCtrl::CUploadListCtrl()
{
	memset(&m_bSortAscending, true, sizeof(m_bSortAscending));
	SetGeneralPurposeFind(true);

	m_pvecDirtyClients = new ClientVector();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUploadListCtrl::Init()
{
	static const uint16 s_auColHdr[][2] =
	{
		{ LVCFMT_LEFT, 150 },	//ULCOL_USERNAME
		{ LVCFMT_LEFT, 275 },	//ULCOL_FILENAME
		{ LVCFMT_RIGHT, 60 },	//ULCOL_SPEED
		{ LVCFMT_RIGHT, 65 },	//ULCOL_TRANSFERRED
		{ LVCFMT_LEFT,  60 },	//ULCOL_WAITED
		{ LVCFMT_LEFT,  60 },	//ULCOL_UPLOADTIME
		{ LVCFMT_LEFT, 110 },	//ULCOL_STATUS
		{ LVCFMT_LEFT,  60 },	//ULCOL_PARTS
		{ LVCFMT_LEFT, 110 },	//ULCOL_PROGRESS
		{ LVCFMT_LEFT, 110 },	//ULCOL_COMPRESSION
		{ LVCFMT_LEFT, 150 }	//ULCOL_COUNTRY
	};

	EMULE_TRY

	CImageList		ilDummyImageList;

	ilDummyImageList.Create(1, 17, g_App.m_iDfltImageListColorFlags | ILC_MASK, 1, 1);
	SetImageList(&ilDummyImageList, LVSIL_SMALL);
	ilDummyImageList.Detach();

	memzero(m_iColumnMaxWidths, sizeof(m_iColumnMaxWidths));
//	Index of the column being measured. -1 for none.
	m_iMeasuringColumn = -1;

	SetExtendedStyle(LVS_EX_FULLROWSELECT);
	for (unsigned ui = 0; ui < ARRSIZE(s_auColHdr); ui++)
		InsertColumn(ui, _T(""), static_cast<int>(s_auColHdr[ui][0]), static_cast<int>(s_auColHdr[ui][1]), ui);

	Localize();

	LoadSettings(CPreferences::TABLE_UPLOAD);
	if (g_App.m_pPrefs->DoUseSort())
		SortInit(g_App.m_pPrefs->GetUploadSortCol());
	else
	{
	//	Use preferred sort order from preferences
		int		iSortCode = g_App.m_pPrefs->GetColumnSortItem(CPreferences::TABLE_UPLOAD);

		iSortCode |= g_App.m_pPrefs->GetColumnSortAscending(CPreferences::TABLE_UPLOAD) ? MLC_SORTASC : MLC_SORTDESC;
		SortInit(iSortCode);
	}

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CUploadListCtrl::~CUploadListCtrl()
{
	safe_delete(m_pvecDirtyClients);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUploadListCtrl::SortInit(int iSortCode)
{
	EMULE_TRY

//	Get the sort column
	int iSortColumn = (iSortCode & MLC_COLUMNMASK);
//	Get the sort order
	bool bSortAscending = (iSortCode & MLC_SORTDESC) == 0;

	SetSortArrow(iSortColumn, bSortAscending);
	SortItems(SortProc, iSortCode);
	m_bSortAscending[iSortColumn] = bSortAscending;

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUploadListCtrl::Localize()
{
	static const uint16 s_auResTbl[] =
	{
		IDS_QL_USERNAME,	//ULCOL_USERNAME
		IDS_FILE,			//ULCOL_FILENAME
		IDS_DL_SPEED,		//ULCOL_SPEED
		IDS_DL_TRANSF,		//ULCOL_TRANSFERRED
		IDS_WAITED,			//ULCOL_WAITED
		IDS_UPLOADTIME,		//ULCOL_UPLOADTIME
		IDS_STATUS,			//ULCOL_STATUS
		IDS_UP_PARTS,		//ULCOL_PARTS
		IDS_DL_PROGRESS,	//ULCOL_PROGRESS
		IDS_COMPRESSION,	//ULCOL_COMPRESSION
		IDS_COUNTRY			//ULCOL_COUNTRY
	};

	EMULE_TRY

	if (GetSafeHwnd())
	{
		CHeaderCtrl	*pHeaderCtrl = GetHeaderCtrl();
		CString		strRes;
		HDITEM		hdi;

		hdi.mask = HDI_TEXT;

		for (unsigned ui = 0; ui < ARRSIZE(s_auResTbl); ui++)
		{
			::GetResString(&strRes, static_cast<UINT>(s_auResTbl[ui]));
			hdi.pszText = const_cast<LPTSTR>(strRes.GetString());
			pHeaderCtrl->SetItem(static_cast<int>(ui), &hdi);
		}
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUploadListCtrl::AddClient(CUpDownClient* pClient)
{
	EMULE_TRY

	uint32		iNumItems = GetItemCount();

	iNumItems = InsertItem(LVIF_TEXT|LVIF_PARAM,iNumItems,LPSTR_TEXTCALLBACK,0,0,1,reinterpret_cast<LPARAM>(pClient));

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUploadListCtrl::RemoveClient(CUpDownClient *pClient)
{
	EMULE_TRY

	if (g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.GetType() == INFOLISTTYPE_SOURCE)
	{
		if (g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.GetClient() == pClient)
		{
 			g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.SetList(INFOLISTTYPE_NONE);
		}
	}

	LVFINDINFO		find;

	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)pClient;

	sint32		result = FindItem(&find);

	if (result != (-1))
		DeleteItem(result);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUploadListCtrl::UpdateClient(CUpDownClient *pClient)
{
	EMULE_TRY

	if (pClient != NULL)
	{
		if (AddDirtyClient(pClient))
			PostUniqueMessage(WM_UL_REFRESH);
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DLC_DT_TEXT (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUploadListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
#define LIST_CELL_PADDING	6		//should be even number
	EMULE_TRY

	if (!g_App.m_pMDlg->IsRunning() || !lpDrawItemStruct->itemData)
		return;

	CDC		*odc = CDC::FromHandle(lpDrawItemStruct->hDC);
	BOOL	bCtrlFocused = ((GetFocus() == this ) || (GetStyle() & LVS_SHOWSELALWAYS));
	COLORREF	crBk, crWinBk;

	crWinBk = crBk = GetBkColor();
	if (lpDrawItemStruct->itemState & ODS_SELECTED)
		crBk = (bCtrlFocused) ? m_crHighlight : m_crNoHighlight;

	CUpDownClient	*pClient = reinterpret_cast<CUpDownClient*>(lpDrawItemStruct->itemData);
	CMemDC			dc(odc, &lpDrawItemStruct->rcItem, crWinBk, crBk);
	CFont			*pOldFont = dc.SelectObject(GetFont());
	COLORREF		crOldTextColor = dc->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
	int				iWidth, iColumn;
	bool			bMeasuring = (m_iMeasuringColumn >= 0);
	UINT			iCalcFlag = bMeasuring ? (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_NOCLIP|DT_CALCRECT) : (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_NOCLIP|DT_END_ELLIPSIS);

	if (IsRightToLeftLanguage())
		iCalcFlag |= DT_RTLREADING;

	RECT			r = lpDrawItemStruct->rcItem;
	CString			strBuffer;
	CKnownFile		*pKnownFile = g_App.m_pSharedFilesList->GetFileByID(pClient->m_reqFileHash);
	CHeaderCtrl		*pHeaderCtrl = GetHeaderCtrl();
	int				iNumColumns = pHeaderCtrl->GetItemCount();

	r.right = r.left - LIST_CELL_PADDING / 2;
	r.left += LIST_CELL_PADDING / 2;
	iWidth = LIST_CELL_PADDING;

	for (int iCurrent = 0; iCurrent < iNumColumns; iCurrent++)
	{
		iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
		if (IsColumnHidden(iColumn) || (bMeasuring && iColumn != m_iMeasuringColumn))
			continue;

		r.right += CListCtrl::GetColumnWidth(iColumn);
		switch (iColumn)
		{
			case ULCOL_USERNAME:
			{
				if (!bMeasuring)
				{
					POINT		point = {r.left, r.top + 1};
					int			iImgLstIdx = CLIENT_IMGLST_PLAIN;

				//	Select corresponding image list depending on client properties
					if (pClient->IsBanned())
						iImgLstIdx = CLIENT_IMGLST_BANNED;
					else if (pClient->IsFriend())
						iImgLstIdx = CLIENT_IMGLST_FRIEND;
					else if (pClient->m_pCredits->HasHigherScoreRatio(pClient->GetIP()))
						iImgLstIdx = CLIENT_IMGLST_CREDITUP;

				//	Display Client icon
					g_App.m_pMDlg->m_clientImgLists[iImgLstIdx].Draw(dc, pClient->GetClientIconIndex(), point, ILD_NORMAL);

					r.left += 20;
					if (g_App.m_pIP2Country->ShowCountryFlag())
					{
						point.x += 20;
						point.y += 2;
						g_App.m_pIP2Country->GetFlagImageList()->Draw(dc, pClient->GetCountryIndex(), point, ILD_NORMAL);
						r.left += 22;
					}
				}
				else
				{
					iWidth += 20;
					if (g_App.m_pIP2Country->ShowCountryFlag())
						iWidth += 22;
				}

				strBuffer = pClient->GetUserName();
				break;
			}
			case ULCOL_FILENAME:
				if (pKnownFile != NULL)
					strBuffer = pKnownFile->GetFileName();
				else
					strBuffer = _T("?");
				break;

			case ULCOL_SPEED:
				strBuffer.Format(_T("%.1f"), static_cast<double>(pClient->GetUpDataRate()) / 1024.0);
				break;

			case ULCOL_TRANSFERRED:
				strBuffer = CastItoXBytes(pClient->GetSessionUp());
				break;

			case ULCOL_WAITED:
				strBuffer = CastSecondsToHM(pClient->GetWaitTime() / 1000);
				break;

			case ULCOL_UPLOADTIME:
				strBuffer = CastSecondsToHM((pClient->GetUpStartTimeDelay()) / 1000);
				break;

			case ULCOL_STATUS:
				GetStatusULQueueString(&strBuffer, pClient->GetUploadState());
				if (pClient->HasLowID())
				{
					strBuffer += _T('/');
					strBuffer += GetResString(IDS_LOWID);
				}
				break;

			case ULCOL_PARTS:
			{
				uint32		iPartNum = pClient->GetCurrentlyUploadingPart();

				strBuffer.Truncate(0);
				if (pClient->GetUpPartCount())
					strBuffer.Format(_T("%u/%u"), pClient->GetAvailUpPartCount(), pClient->GetUpPartCount());
				if (iPartNum != 0xFFFF)
					strBuffer.AppendFormat(_T(" (%u)"), iPartNum);
				break;
			}
			case ULCOL_PROGRESS:
				if (pClient->GetUpPartCount() && g_App.m_pPrefs->IsUploadPartsEnabled())
				{
					if (!bMeasuring)
					{
						RECT	r2;

						r2.bottom = r.bottom - 1;
						r2.top = r.top + 1;
						r2.right = r.right + LIST_CELL_PADDING / 2;
						r2.left = r.left - LIST_CELL_PADDING / 2;
						pClient->DrawUpStatusBar(dc, &r2, g_App.m_pPrefs->UseFlatBar());
					}
					iWidth = 300;
				}
				break;

			case ULCOL_COMPRESSION:
			{
				double		dblCompression = pClient->GetCompression();

				if (dblCompression < 0.1)
					strBuffer = _T("-");
				else
					strBuffer.Format(_T("%.1f"), dblCompression);
				break;
			}
			case ULCOL_COUNTRY:
				strBuffer = pClient->GetCountryName();
				break;
		}
		if (iColumn != ULCOL_PROGRESS)
		{
			UINT		dwCalcFlag = iCalcFlag;

			if ((iColumn == ULCOL_TRANSFERRED) || (iColumn == ULCOL_SPEED))
				dwCalcFlag |= DT_RIGHT;
			dc->DrawText(strBuffer, &r, dwCalcFlag);
			if (bMeasuring && !strBuffer.IsEmpty())
				iWidth += r.right - r.left + 1;
		}
		r.left = r.right + LIST_CELL_PADDING;

		if (bMeasuring)
		{
		//	Pin the column widths at some reasonable value
			if (iWidth < 40)
				iWidth = 40;
			if (iWidth > m_iColumnMaxWidths[m_iMeasuringColumn])
				m_iColumnMaxWidths[m_iMeasuringColumn] = iWidth;
		}
	}
//	Draw rectangle around selected item(s)
	if (!bMeasuring && (lpDrawItemStruct->itemState & ODS_SELECTED))
	{
		RECT	rOutline = lpDrawItemStruct->rcItem;
		CBrush	FrmBrush((bCtrlFocused) ? m_crFocusLine : m_crNoFocusLine);

		rOutline.left++;
		rOutline.right--;
		dc->FrameRect(&rOutline, &FrmBrush);
	}
	if (pOldFont)
		dc.SelectObject(pOldFont);
	if (crOldTextColor)
		dc.SetTextColor(crOldTextColor);

	EMULE_CATCH
#undef LIST_CELL_PADDING
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CUploadListCtrl::OnWndMsg(UINT iMessage,WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	BOOL		bHandled = TRUE;

	EMULE_TRY

	switch (iMessage)
	{
		case WM_UL_REFRESH:
		{
			CUploadListCtrl::ClientVector	   *pvecDirtyClients = GetDirtyClients();

			if (pvecDirtyClients != NULL)
			{
				SetRedraw(FALSE);

				int		iNumDirtySources = pvecDirtyClients->size();

				for (int i = 0; i < iNumDirtySources; i++)
				{
					LVFINDINFO		find;

					find.flags = LVFI_PARAM;
					find.lParam = (LPARAM)(*pvecDirtyClients)[i];

					int	iResult = FindItem(&find);

					if (iResult != -1)
						Update(iResult);
				}

				delete pvecDirtyClients;

				SetRedraw(TRUE);
			}
			break;
		}
		default:
		{
			bHandled = FALSE;
			break;
		}
	}

	EMULE_CATCH

	if (!bHandled)
		bHandled = CMuleListCtrl::OnWndMsg(iMessage, wParam, lParam, pResult);

	return bHandled;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUploadListCtrl::OnNMDividerDoubleClick(NMHEADER *pHeader)
{
	EMULE_TRY

	int		iColumn = pHeader->iItem;

	m_iColumnMaxWidths[iColumn] = 0;
	m_iMeasuringColumn = iColumn;
	Invalidate();
	UpdateWindow();
	m_iMeasuringColumn = -1;
	if (m_iColumnMaxWidths[iColumn] > 0)
		SetColumnWidth(iColumn, m_iColumnMaxWidths[iColumn]);
	else
		CMuleListCtrl::OnNMDividerDoubleClick(pHeader);
	Invalidate();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// CUploadListCtrl message handlers

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUploadListCtrl::OnContextMenu(CWnd *pWnd, CPoint point)
{
	NOPRM(pWnd);
	EMULE_TRY

	CTitleMenu		menuClient;
	POSITION		posSelClient = GetFirstSelectedItemPosition();
	UINT			dwFlags = MF_STRING | MF_GRAYED;
	CUpDownClient	*pClient = NULL;

	if (posSelClient)
	{
		dwFlags = MF_STRING;
		pClient = reinterpret_cast<CUpDownClient*>(GetItemData(GetNextSelectedItem(posSelClient)));

		if (pClient)
		{
			g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.SetList(INFOLISTTYPE_SOURCE, pClient);
		}
	}

	menuClient.CreatePopupMenu();
	menuClient.AddMenuTitle(GetResString(IDS_CLIENTS));
	menuClient.AppendMenu(dwFlags, MP_DETAIL, GetStringFromShortcutCode(IDS_SHOWDETAILS, SCUT_SRC_DETAILS, SSP_TAB_PREFIX));

	UINT_PTR	dwRes = MP_ADDFRIEND;
	UINT		dwResStrId = IDS_ADDFRIEND;

	if (pClient && pClient->IsFriend())
	{
		dwRes = MP_REMOVEFRIEND;
		dwResStrId = IDS_REMOVEFRIEND;
	}
	menuClient.AppendMenu(dwFlags, dwRes, GetStringFromShortcutCode(dwResStrId, SCUT_SRC_FRIEND, SSP_TAB_PREFIX));
	menuClient.AppendMenu(dwFlags, MP_MESSAGE, GetStringFromShortcutCode(IDS_SEND_MSG, SCUT_SRC_MSG, SSP_TAB_PREFIX));
	menuClient.AppendMenu( dwFlags | ((pClient && pClient->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED),
		MP_SHOWLIST, GetStringFromShortcutCode(IDS_VIEWFILES, SCUT_SRC_SHAREDFILES, SSP_TAB_PREFIX) );

	menuClient.SetDefaultItem((g_App.m_pPrefs->GetDetailsOnClick()) ? MP_DETAIL : MP_MESSAGE);
	menuClient.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this, NULL);

//	Menu objects are destroyed in their destructor

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CUploadListCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	NOPRM(lParam);
	EMULE_TRY
	POSITION posSelClient = GetFirstSelectedItemPosition();

	if (posSelClient)
	{
		CUpDownClient* 	pClient = (CUpDownClient*)GetItemData(GetNextSelectedItem(posSelClient));

		switch (wParam)
		{
			case MP_SHOWLIST:
				pClient->RequestSharedFileList();
				break;
			case MP_MESSAGE:
				g_App.m_pMDlg->m_wndChat.StartSession(pClient);
				break;
			case MP_ADDFRIEND:
				g_App.m_pFriendList->AddFriend(pClient);
				break;
			case MP_REMOVEFRIEND:
				g_App.m_pFriendList->RemoveFriend(pClient);
				break;
			case MP_DETAIL:
				{
					CClientDetails dialog(IDS_CD_TITLE, pClient, this, 0);
					dialog.DoModal();
				}
				break;
		}
	}

	EMULE_CATCH

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUploadListCtrl::OnColumnClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	EMULE_TRY

	NM_LISTVIEW	*pNMListView = reinterpret_cast<NM_LISTVIEW*>(pNMHDR);
	int			iSubItem = pNMListView->iSubItem;
	bool		bSortOrder = m_bSortAscending[iSubItem];

// Reverse sorting direction for the same column and keep the same if column was changed
	if (static_cast<int>(m_dwParamSort & MLC_COLUMNMASK) == iSubItem)
		m_bSortAscending[iSubItem] = bSortOrder = !bSortOrder;

	SetSortArrow(iSubItem, bSortOrder);
	SortItems(SortProc, iSubItem + ((bSortOrder) ? MLC_SORTASC : MLC_SORTDESC));
	g_App.m_pPrefs->SetColumnSortItem(CPreferences::TABLE_UPLOAD, iSubItem);
	g_App.m_pPrefs->SetColumnSortAscending(CPreferences::TABLE_UPLOAD, bSortOrder);
	*pResult = 0;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CUploadListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CUpDownClient* item1 = (CUpDownClient*)lParam1;
	CUpDownClient* item2 = (CUpDownClient*)lParam2;

	//check parameters
	if (item1 == NULL || item2 == NULL)
		return 0;

	CKnownFile* file1 = g_App.m_pSharedFilesList->GetFileByID(item1->m_reqFileHash);
	CKnownFile* file2 = g_App.m_pSharedFilesList->GetFileByID(item2->m_reqFileHash);

	int	iCompare = 0;
	bool	bSortAscending = (lParamSort & MLC_SORTDESC) == 0;
	int	iSortMod = bSortAscending ? 1 : -1;
	int	iSortColumn = (lParamSort & MLC_COLUMNMASK);

	switch(iSortColumn)
	{
		case ULCOL_USERNAME:
		{
			iCompare = item1->CmpUserNames(item2->GetUserName());
			break;
		}
		case ULCOL_COUNTRY:
		{
			CString strCountry1 = item1->GetCountryName();
			CString strCountry2 = item2->GetCountryName();

			if (!strCountry1.IsEmpty() && !strCountry2.IsEmpty())
			{
				iCompare = _tcsicmp(strCountry1, strCountry2);
			}
			else
			{
				iCompare = strCountry1.IsEmpty() ? (strCountry2.IsEmpty() ? 0 : 1) : -1;
				iSortMod = 1;		//empty entries at the bottom in any case
			}

			if (iCompare == 0)
			{
				iCompare = SortProc(lParam1,lParam2,ULCOL_USERNAME);
				iSortMod = 1;		//sort allways in ascending order
			}
			break;
		}
		case ULCOL_FILENAME:
			if ((file1 != NULL) && (file2 != NULL))
				iCompare = _tcsicmp(file1->GetFileName(),file2->GetFileName());
			else
				iCompare = ( file1 == NULL ) ? ((file2 == NULL) ? 0 : 1) : -1;
			if (iCompare == 0)
			{
				iCompare = SortProc(lParam1,lParam2, ULCOL_USERNAME);
				iSortMod = 1;		//eklmn: sort allways in ascending order
			}
			break;
		case ULCOL_SPEED:
			iCompare = item1->GetUpDataRate() - item2->GetUpDataRate();
			break;
		case ULCOL_TRANSFERRED:
			iCompare = item1->GetSessionUp() - item2->GetSessionUp();
			break;
		case ULCOL_WAITED:
			iCompare = item1->GetWaitTime() - item2->GetWaitTime();
			break;
		case ULCOL_UPLOADTIME:
			iCompare = item1->GetUpStartTimeDelay() - item2->GetUpStartTimeDelay();
			break;
		case ULCOL_STATUS:
			iCompare = item1->GetUploadState() - item2->GetUploadState();
			break;
		case ULCOL_PARTS:
		case ULCOL_PROGRESS:
		{  // obaldin: sort by upload progress
			uint32 partcnt1 = item1->GetUpPartCount();
			uint32 partcnt2 = item2->GetUpPartCount();

			if((partcnt1!=0) && (partcnt2!=0))
			{
				double	dblProgress1 = static_cast<double>(item1->GetAvailUpPartCount())/static_cast<double>(partcnt1);
				double	dblProgress2 = static_cast<double>(item2->GetAvailUpPartCount())/static_cast<double>(partcnt2);

				iCompare = (dblProgress1 > dblProgress2) ? 1 : ((dblProgress1 < dblProgress2) ? -1: 0);
			}
			else
				iCompare = ( partcnt1 == 0 ) ? ((partcnt2 == 0) ? 0 : 1) : -1;
			break;
		}
		case ULCOL_COMPRESSION: // Add sort compression
			iCompare = (item1->GetCompression() < item2->GetCompression())?-1:1;
			break;
		default:
			iCompare = 0;
	}

	return iCompare * iSortMod;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUploadListCtrl::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	NOPRM(pNMHDR);
	EMULE_TRY

//	Reset selection in DL list
	POSITION posSelClient = g_App.m_pMDlg->m_wndTransfer.m_ctlDownloadList.GetFirstSelectedItemPosition();

	while (posSelClient != NULL)
	{
		int iSelClientListIndex = g_App.m_pMDlg->m_wndTransfer.m_ctlDownloadList.GetNextSelectedItem(posSelClient);
		g_App.m_pMDlg->m_wndTransfer.m_ctlDownloadList.SetItemState(iSelClientListIndex, 0, LVIS_SELECTED);
	}

	RefreshInfo();
	*pResult = 0;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUploadListCtrl::OnNMDblclkUploadlist(NMHDR *pNMHDR, LRESULT *pResult)
{
	NOPRM(pNMHDR);
	EMULE_TRY
	POSITION posSelClient = GetFirstSelectedItemPosition();

	if (posSelClient)
	{
		CUpDownClient* pClient = reinterpret_cast<CUpDownClient*>(GetItemData(GetNextSelectedItem(posSelClient)));

		if (g_App.m_pPrefs->GetDetailsOnClick())
		{
			CClientDetails dialog(IDS_CD_TITLE, pClient, this, 0);
			dialog.DoModal();
		}
		else
		{
			g_App.m_pMDlg->m_wndChat.StartSession(pClient);
		}
	}
	*pResult = 0;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUploadListCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	EMULE_TRY

	RefreshInfo();
	CMuleListCtrl::OnKeyUp(nChar, nRepCnt, nFlags);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUploadListCtrl::RefreshInfo(void)
{
	EMULE_TRY

	if (GetFocus() == this)
	{
		POSITION posSelClient = GetFirstSelectedItemPosition();

		if(posSelClient == 0)
		{
	 		g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.SetList(INFOLISTTYPE_NONE);
		}
		else
		{
			CUpDownClient* pClient = reinterpret_cast<CUpDownClient*>(GetItemData(GetNextSelectedItem(posSelClient)));

	 		g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.SetList(INFOLISTTYPE_SOURCE,pClient);
		}
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUploadListCtrl::OnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO	*pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

	if (g_App.m_pMDlg->IsRunning())
	{
		// Although we have an owner drawn listview control we store the text for the primary item in the listview, to be
		// capable of quick searching those items via the keyboard. Because our listview items may change their contents,
		// we do this via a text callback function. The listview control will send us the LVN_DISPINFO notification if
		// it needs to know the contents of the primary item.
		//
		// But, the listview control sends this notification all the time, even if we do not search for an item. At least
		// this notification is only sent for the visible items and not for all items in the list. Though, because this
		// function is invoked *very* often, no *NOT* put any time consuming code here in.

		if ((pDispInfo->item.mask & LVIF_TEXT) != 0)
		{
		//	Check for own search request, the rest of the flood comes from list control
		//	and isn't used as list is drawn by us
			if (pDispInfo->item.cchTextMax == ML_SEARCH_SZ)
			{
				CUpDownClient	*pClient = reinterpret_cast<CUpDownClient*>(pDispInfo->item.lParam);

				if (pClient != NULL)
				{
					switch (pDispInfo->item.iSubItem)
					{
						case ULCOL_USERNAME:
							_tcsncpy(pDispInfo->item.pszText, pClient->GetUserName(), pDispInfo->item.cchTextMax);
							pDispInfo->item.pszText[pDispInfo->item.cchTextMax - 1] = _T('\0');
							break;

						case ULCOL_COUNTRY:
							_tcsncpy(pDispInfo->item.pszText, pClient->GetCountryName(), pDispInfo->item.cchTextMax);
							pDispInfo->item.pszText[pDispInfo->item.cchTextMax - 1] = _T('\0');
							break;

						case ULCOL_FILENAME:
						{
							CKnownFile	*pKnownFile = g_App.m_pSharedFilesList->GetFileByID(pClient->m_reqFileHash);

							if (pKnownFile != NULL)
							{
								_tcsncpy(pDispInfo->item.pszText, pKnownFile->GetFileName(), pDispInfo->item.cchTextMax);
								pDispInfo->item.pszText[pDispInfo->item.cchTextMax - 1] = _T('\0');
								break;
							}
						}
						default:
							pDispInfo->item.pszText[0] = _T('\0');
							break;
					}
				}
			}
			else if (pDispInfo->item.cchTextMax != 0)
				pDispInfo->item.pszText[0] = _T('\0');
		}
	}
	*pResult = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CUploadListCtrl::PreTranslateMessage(MSG *pMsg)
{
	if ((pMsg->message == WM_KEYDOWN) || (pMsg->message == WM_SYSKEYDOWN))
	{
		int		 iMessage	  = 0;
		POSITION posSelClient = GetFirstSelectedItemPosition();

		if (posSelClient != NULL)
		{
			short			nCode = GetCodeFromPressedKeys(pMsg);
			CUpDownClient	*pSource = reinterpret_cast<CUpDownClient*>(GetItemData(GetNextSelectedItem(posSelClient)));

			iMessage = GetClientListActionFromShortcutCode(nCode, pSource);

			if (iMessage > 0)
			{
				PostMessage(WM_COMMAND, static_cast<WPARAM>(iMessage));
				return TRUE;
			}
		}
	}

	return CMuleListCtrl::PreTranslateMessage(pMsg);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUploadListCtrl::ShowSelectedUserDetails()
{
	POINT point;

	::GetCursorPos(&point);

	CPoint p = point;

	ScreenToClient(&p);

	int it = HitTest(p);

	if (it == -1)
		return;

	SetSelectionMark(it);

	CUpDownClient* pClient = reinterpret_cast<CUpDownClient*>(GetItemData(GetSelectionMark()));

	if (pClient != NULL)
	{
			CClientDetails dialog(IDS_CD_TITLE, pClient, this, 0);
			dialog.DoModal();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUploadListCtrl::AddDirtyClient(CUpDownClient* pClientItem)
{
	bool bResult = false;

//	If 'pClient' is not already in the dirty client list...
	if (m_pvecDirtyClients != NULL
		&& ::find(m_pvecDirtyClients->begin(),m_pvecDirtyClients->end(), pClientItem) == m_pvecDirtyClients->end())
	{
		m_pvecDirtyClients->push_back(pClientItem);
		bResult = true;
	}

	return bResult;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetDirtySources() detaches the current dirty source list and returns it to the caller (who's responsible
//		for destroying it). This allows the download list control to update without locking down the download list.
CUploadListCtrl::ClientVector* CUploadListCtrl::GetDirtyClients()
{
	ClientVector		*pDirtyClients = NULL;

	if (!m_pvecDirtyClients->empty())
	{
		pDirtyClients = m_pvecDirtyClients;
		m_pvecDirtyClients = new ClientVector();
	}

	return pDirtyClients;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
