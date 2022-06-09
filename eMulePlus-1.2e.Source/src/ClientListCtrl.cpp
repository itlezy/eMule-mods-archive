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
#include "ClientListCtrl.h"
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

BEGIN_MESSAGE_MAP(CClientListCtrl, CMuleListCtrl)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_CLICK, OnNMClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
	ON_WM_KEYUP()
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CClientListCtrl, CMuleListCtrl)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CClientListCtrl::CClientListCtrl()
{
	memset(&m_bSortAscending, true, sizeof(m_bSortAscending));
	SetGeneralPurposeFind(true);

	m_pvecDirtyClients = new ClientVector();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CClientListCtrl::~CClientListCtrl()
{
	safe_delete(m_pvecDirtyClients);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientListCtrl::Init()
{
	static const uint16 s_auColHdr[][2] =
	{
		{ LVCFMT_LEFT, 150 },	// CLCOL_USERNAME
		{ LVCFMT_LEFT, 100 },	// CLCOL_UPLOADSTATUS
		{ LVCFMT_LEFT, 100 },	// CLCOL_TRANSFERREDUP
		{ LVCFMT_LEFT, 100 },	// CLCOL_DOWNLOADSTATUS
		{ LVCFMT_LEFT, 100 },	// CLCOL_TRANSFERREDDOWN
		{ LVCFMT_LEFT, 120 },	// CLCOL_CLIENTSOFTWARE
		{ LVCFMT_LEFT,  80 },	// CLCOL_CONNECTEDTIME
		{ LVCFMT_LEFT, 210 },	// CLCOL_USERHASH
		{ LVCFMT_LEFT, 150 }	// CLCOL_COUNTRY
	};
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

	LoadSettings(CPreferences::TABLE_CLIENTLIST);

	if (g_App.m_pPrefs->DoUseSort())
	{
		SortInit(g_App.m_pPrefs->GetClientListSortCol());
	}
	else
	{
		int		iSortCode = g_App.m_pPrefs->GetColumnSortItem(CPreferences::TABLE_CLIENTLIST);

		iSortCode |= g_App.m_pPrefs->GetColumnSortAscending(CPreferences::TABLE_CLIENTLIST) ? MLC_SORTASC : MLC_SORTDESC;
		SortInit(iSortCode);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientListCtrl::SortInit(int iSortCode)
{
//	Get the sort column
	int		iSortColumn = (iSortCode & MLC_COLUMNMASK);
//	Get the sort order
	bool	bSortAscending = (iSortCode & MLC_SORTDESC) == 0;

	SetSortArrow(iSortColumn,bSortAscending);
	SortItems(SortProc, iSortCode);
	m_bSortAscending[iSortColumn] = bSortAscending;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientListCtrl::Localize()
{
	static const uint16 s_auResTbl[] =
	{
		IDS_QL_USERNAME,				// CLCOL_USERNAME
		IDS_CL_UPLOADSTATUS,			// CLCOL_UPLOADSTATUS
		IDS_CL_TRANSFUP,				// CLCOL_TRANSFERREDUP
		IDS_CL_DOWNLSTATUS,				// CLCOL_DOWNLOADSTATUS
		IDS_CL_TRANSFDOWN,				// CLCOL_TRANSFERREDDOWN
		IDS_INFLST_USER_CLIENTSOFTWARE,	// CLCOL_CLIENTSOFTWARE
		IDS_CONNECTED,					// CLCOL_CONNECTEDTIME
		IDS_INFLST_USER_USERHASH,		// CLCOL_USERHASH
		IDS_COUNTRY						// CLCOL_COUNTRY
	};

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
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientListCtrl::ShowKnownClients()
{
	DeleteAllItems();

	int			i = 0;
	CUpDownClient		*pClient;

	SetRedraw(FALSE);
	for (POSITION pos = g_App.m_pClientList->m_clientList.GetHeadPosition(); pos != NULL; )
	{
		pClient = g_App.m_pClientList->m_clientList.GetNext(pos);

		InsertItem(LVIF_TEXT|LVIF_PARAM,i,LPSTR_TEXTCALLBACK,0,0,0,reinterpret_cast<LPARAM>(pClient));
		i++;
	}
	SetRedraw(TRUE);
	g_App.m_pMDlg->m_wndTransfer.UpdateKnown();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientListCtrl::AddClient(CUpDownClient *pClient)
{
	LVFINDINFO		find;

	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)pClient;

	if ((FindItem(&find) < 0) && (pClient != NULL))
	{
		InsertItem(LVIF_TEXT|LVIF_PARAM, GetItemCount(), LPSTR_TEXTCALLBACK, 0, 0, 1, reinterpret_cast<LPARAM>(pClient));
		g_App.m_pMDlg->m_wndTransfer.UpdateKnown();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientListCtrl::RemoveClient(CUpDownClient* pClient)
{
	if (!g_App.m_pMDlg->IsRunning() || (pClient == NULL))
		return;

	if (g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.GetType() == INFOLISTTYPE_SOURCE)
	{
		if (g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.GetClient() == pClient)
			g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.SetList(INFOLISTTYPE_NONE);
	}
	sint32		iResult;
	LVFINDINFO	find;

	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)pClient;
	
	if ((iResult = FindItem(&find)) >= 0)
	{
		DeleteItem(iResult);
		g_App.m_pMDlg->m_wndTransfer.UpdateKnown();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientListCtrl::UpdateClient(CUpDownClient* pClient)
{
	if (pClient != NULL)
	{
		if (AddDirtyClient(pClient))
			PostUniqueMessage(WM_CL_REFRESH);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DLC_DT_TEXT (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
#define LIST_CELL_PADDING	6		//should be even number
	EMULE_TRY

	if (!g_App.m_pMDlg->IsRunning() || !lpDrawItemStruct->itemData)
		return;

	CDC			   *odc = CDC::FromHandle(lpDrawItemStruct->hDC);
	BOOL			bCtrlFocused = ((GetFocus() == this) || (GetStyle() & LVS_SHOWSELALWAYS));
	COLORREF		crBk, crWinBk;

	crWinBk = crBk = GetBkColor();
	if (lpDrawItemStruct->itemState & ODS_SELECTED)
		crBk = (bCtrlFocused) ? m_crHighlight : m_crNoHighlight;

	CUpDownClient	   *pClient = (CUpDownClient*)lpDrawItemStruct->itemData;
	CMemDC				dc(odc, &lpDrawItemStruct->rcItem, crWinBk, crBk);
	CFont			   *pOldFont = dc.SelectObject(GetFont());
	COLORREF			crOldTextColor = dc.SetTextColor(m_crWindowText);
	int					iWidth, iColumn;
	bool				bMeasuring = (m_iMeasuringColumn >= 0);
	UINT				iCalcFlag = bMeasuring ? (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_CALCRECT) : (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS);
	UINT				dwResStrId;

	if (IsRightToLeftLanguage())
		iCalcFlag |= DT_RTLREADING;

	RECT			r = lpDrawItemStruct->rcItem;
	CString			strBuffer;
	CHeaderCtrl	   *pHeaderCtrl = GetHeaderCtrl();
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
			case CLCOL_USERNAME:
			{
				if (!bMeasuring)
				{
					POINT			point = {r.left, r.top + 1};
					int				iImgLstIdx = CLIENT_IMGLST_PLAIN;

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

				try
				{
					strBuffer = pClient->GetUserName();
				}
				catch(...)
				{	//	can crash here if name string object is wrong, probably because
				}	//	client was just deleted or user name object is currently updating
				if (strBuffer.IsEmpty())
					strBuffer.Format(_T("[%s]"), GetResString(IDS_UNKNOWN));
				break;
			}
			case CLCOL_UPLOADSTATUS:
			{
				switch (pClient->GetUploadState())
				{
					case US_ONUPLOADQUEUE:
						dwResStrId = IDS_ONQUEUE;
						break;
					case US_BANNED:
						dwResStrId = IDS_BANNED;
						break;
					case US_CONNECTING:
						dwResStrId = IDS_CONNECTING;
						break;
					case US_UPLOADING:
						dwResStrId = IDS_TRANSFERRING;
						break;
					default:
						strBuffer.Truncate(0);
						dwResStrId = 0;
				}
				if (dwResStrId != 0)
					GetResString(&strBuffer, dwResStrId);
				break;
			}
			case CLCOL_TRANSFERREDUP:
				if (pClient->m_pCredits)
					strBuffer = CastItoXBytes(pClient->m_pCredits->GetUploadedTotal());
				else
					strBuffer = _T("");
				break;

			case CLCOL_DOWNLOADSTATUS:
			{
				switch (pClient->GetDownloadState())
				{
					case DS_CONNECTING:
						dwResStrId = IDS_CONNECTING;
						break;
					case DS_CONNECTED:
						dwResStrId = IDS_ASKING;
						break;
					case DS_WAITCALLBACK:
						dwResStrId = IDS_CONNVIASERVER;
						break;
					case DS_ONQUEUE:
						dwResStrId = pClient->IsRemoteQueueFull()  ? IDS_QUEUEFULL : IDS_ONQUEUE;
						break;
					case DS_DOWNLOADING:
						dwResStrId = IDS_TRANSFERRING;
						break;
					case DS_REQHASHSET:
						dwResStrId = IDS_RECHASHSET;
						break;
					case DS_NONEEDEDPARTS:
						dwResStrId = IDS_NONEEDEDPARTS;
						break;
					case DS_LOWTOLOWID:
						dwResStrId = IDS_NOCONNECTLOW2LOW;
						break;
					case DS_LOWID_ON_OTHER_SERVER:
						dwResStrId = IDS_ANOTHER_SERVER_LOWID;
						break;
					case DS_WAIT_FOR_FILE_REQUEST:
						dwResStrId = IDS_WAITFILEREQ;
						break;
					default:
						strBuffer.Truncate(0);
						dwResStrId = 0;
				}
				if (dwResStrId != 0)
					GetResString(&strBuffer, dwResStrId);
				break;
			}
			case CLCOL_TRANSFERREDDOWN:
				if(pClient->m_pCredits)
					strBuffer = CastItoXBytes(pClient->m_pCredits->GetDownloadedTotal());
				else
					strBuffer = _T("");
				break;

			case CLCOL_CLIENTSOFTWARE:
				strBuffer = pClient->GetFullSoftVersionString();
				break;

			case CLCOL_CONNECTEDTIME:
#ifdef OLD_SOCKETS_ENABLED
				strBuffer = YesNoStr(pClient->m_pRequestSocket != NULL && pClient->m_pRequestSocket->IsConnected());
#endif //OLD_SOCKETS_ENABLED
				break;

			case CLCOL_USERHASH:
				strBuffer = HashToString(pClient->GetUserHash());
				break;

			case CLCOL_COUNTRY:
				strBuffer = pClient->GetCountryName();
				break;
		}
		{
			dc->DrawText(strBuffer, &r, iCalcFlag);
			if (bMeasuring && !strBuffer.IsEmpty())
				iWidth += r.right - r.left + 1;
		}
		r.left = r.right + LIST_CELL_PADDING;

		if (bMeasuring)
		{
		//	Pin the column widths at some reasonable value.
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
BOOL CClientListCtrl::OnWndMsg(UINT iMessage,WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	BOOL		bHandled = TRUE;

	EMULE_TRY

	switch (iMessage)
	{
		case WM_CL_REFRESH:
		{
			CClientListCtrl::ClientVector	   *pvecDirtyClients = GetDirtyClients();

			if (pvecDirtyClients != NULL)
			{
				SetRedraw(FALSE);

				int		iNumDirtySources = pvecDirtyClients->size();

				for (int i = 0; i < iNumDirtySources; i++)
				{
					LVFINDINFO		find;
					int			iResult;

					find.flags = LVFI_PARAM;
					find.lParam = (LPARAM)(*pvecDirtyClients)[i];

					iResult = FindItem(&find);
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
void CClientListCtrl::OnNMDividerDoubleClick(NMHEADER *pHeader)
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
void CClientListCtrl::OnContextMenu(CWnd *pWnd, CPoint point)
{
	NOPRM(pWnd);
	EMULE_TRY

	CTitleMenu		menuClient;
	POSITION		posSelClient = GetFirstSelectedItemPosition();
	UINT			dwMenuFlags = MF_STRING | MF_GRAYED;
	CUpDownClient	*pClient = NULL;

	if (posSelClient)
	{
		dwMenuFlags = MF_STRING;
		pClient = reinterpret_cast<CUpDownClient*>(GetItemData(GetNextSelectedItem(posSelClient)));

		if (pClient)
		{
			g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.SetList(INFOLISTTYPE_SOURCE,pClient);
		}
	}

	menuClient.CreatePopupMenu();
	menuClient.AddMenuTitle(GetResString(IDS_CLIENTLIST));
	menuClient.AppendMenu(dwMenuFlags, MP_DETAIL, GetStringFromShortcutCode(IDS_SHOWDETAILS, SCUT_SRC_DETAILS, SSP_TAB_PREFIX));

	UINT_PTR	dwRes = MP_ADDFRIEND;
	UINT		dwResStrId = IDS_ADDFRIEND;

	if (pClient && pClient->IsFriend())
	{
		dwRes = MP_REMOVEFRIEND;
		dwResStrId = IDS_REMOVEFRIEND;
	}
	menuClient.AppendMenu(dwMenuFlags, dwRes, GetStringFromShortcutCode(dwResStrId, SCUT_SRC_FRIEND, SSP_TAB_PREFIX));
	menuClient.AppendMenu(dwMenuFlags, MP_MESSAGE, GetStringFromShortcutCode(IDS_SEND_MSG, SCUT_SRC_MSG, SSP_TAB_PREFIX));
	menuClient.AppendMenu( dwMenuFlags | ((pClient && pClient->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED),
		MP_SHOWLIST, GetStringFromShortcutCode(IDS_VIEWFILES, SCUT_SRC_SHAREDFILES, SSP_TAB_PREFIX) );
	menuClient.AppendMenu(dwMenuFlags | ((pClient && pClient->IsBanned()) ? MF_ENABLED : MF_GRAYED), MP_UNBAN, GetResString(IDS_UNBAN) );

	menuClient.SetDefaultItem((g_App.m_pPrefs->GetDetailsOnClick()) ? MP_DETAIL : MP_MESSAGE);
	menuClient.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this, NULL);

//	Menu objects are destroyed in their destructor

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CClientListCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	POSITION posSelClient = GetFirstSelectedItemPosition();
	NOPRM(lParam);

	if (posSelClient)
	{
		CUpDownClient* 	pClient = reinterpret_cast<CUpDownClient*>(GetItemData(GetNextSelectedItem(posSelClient)));

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
				UpdateClient(pClient);
				break;

			case MP_REMOVEFRIEND:
				g_App.m_pFriendList->RemoveFriend(pClient);
				UpdateClient(pClient);
				break;

			case MP_UNBAN:
				if (pClient->IsBanned())
				{
					pClient->UnBan();
					UpdateClient(pClient);
				}
				break;

			case MP_DETAIL:
			{
				CClientDetails		dialog(IDS_CD_TITLE, pClient, this, 0);
				dialog.DoModal();
				break;
			}
		}
	}
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientListCtrl::OnColumnClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_LISTVIEW	*pNMListView = reinterpret_cast<NM_LISTVIEW*>(pNMHDR);
	int			iSubItem = pNMListView->iSubItem;
	bool		bSortOrder = m_bSortAscending[iSubItem];

// Reverse sorting direction for the same column and keep the same if column was changed
	if (static_cast<int>(m_dwParamSort & MLC_COLUMNMASK) == iSubItem)
		m_bSortAscending[iSubItem] = bSortOrder = !bSortOrder;

	SetSortArrow(iSubItem, bSortOrder);
	SortItems(SortProc, iSubItem + ((bSortOrder) ? MLC_SORTASC : MLC_SORTDESC));
	g_App.m_pPrefs->SetColumnSortItem(CPreferences::TABLE_CLIENTLIST, iSubItem);
	g_App.m_pPrefs->SetColumnSortAscending(CPreferences::TABLE_CLIENTLIST, bSortOrder);
	*pResult = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CClientListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CUpDownClient*	pClient1 = reinterpret_cast<CUpDownClient*>(lParam1);
	CUpDownClient*	pClient2 = reinterpret_cast<CUpDownClient*>(lParam2);

	if (pClient1 == NULL || pClient2 == NULL)
		return 0;

	int	iCompare = 0;
	int	iSortMod = ((lParamSort & MLC_SORTDESC) == 0) ? 1 : -1;
	int	iSortColumn = (lParamSort & MLC_COLUMNMASK);

	switch (iSortColumn)
	{
		case CLCOL_USERNAME:
		{
			iCompare = pClient1->CmpUserNames(pClient2->GetUserName());
			break;
		}
		case CLCOL_COUNTRY:
		{
			CString strCountry1 = pClient1->GetCountryName();
			CString strCountry2 = pClient2->GetCountryName();

			if (!strCountry1.IsEmpty() && !strCountry2.IsEmpty())
			{
				iCompare = _tcsicmp(strCountry1, strCountry2);
			}
			else
			{
				iCompare = strCountry1.IsEmpty() ? (strCountry2.IsEmpty() ? 0 : 1) : -1;
			//	Empty entries at the bottom in any case
				iSortMod = 1;
			}

			if (iCompare == 0)
			{
				iCompare = SortProc(lParam1, lParam2, CLCOL_USERNAME);
			//	Sort always in ascending order
				iSortMod = 1;
			}
			break;
		}
		case CLCOL_UPLOADSTATUS:
		{
			iCompare = pClient1->GetUploadState()-pClient2->GetUploadState();
			if ((pClient1->GetUploadState() == US_NONE) || (pClient2->GetUploadState() == US_NONE))
				iSortMod = 1;
			break;
		}
		case CLCOL_TRANSFERREDUP:
		{
		//	We are using a 64 bit variables for the credits
			if (pClient1->m_pCredits && pClient2->m_pCredits)
			{
				uint64	qwClient1Credits = pClient1->m_pCredits->GetUploadedTotal();
				uint64	qwClient2Credits = pClient2->m_pCredits->GetUploadedTotal();

				if (qwClient1Credits > qwClient2Credits)
					iCompare = 1;
				else if (qwClient1Credits < qwClient2Credits)
					iCompare = -1;
				else
					iCompare = 0;
			}
			else
			{
				iCompare = (!pClient1->m_pCredits) ? ( (!pClient2->m_pCredits)? 0:1) : -1;
				iSortMod = 1;
			}
			break;
		}
		case CLCOL_DOWNLOADSTATUS:
		{
			if (pClient1->GetDownloadState() == pClient2->GetDownloadState())
			{
				if (pClient1->GetDownloadState() == DS_ONQUEUE)
					iCompare = pClient1->IsRemoteQueueFull() ? 1 : (pClient2->IsRemoteQueueFull() ? -1:0);
			}
			else
				iCompare = pClient1->GetDownloadState() - pClient2->GetDownloadState();
			if ((pClient1->GetDownloadState() == DS_WAIT_FOR_FILE_REQUEST) || (pClient2->GetDownloadState() == DS_WAIT_FOR_FILE_REQUEST))
				iSortMod = 1;
			break;
		}
		case CLCOL_TRANSFERREDDOWN:
		{
		//	We are using a 64 bit variables for the credits
			if (pClient1->m_pCredits && pClient2->m_pCredits)
			{
				uint64	qwClient1Credits = pClient1->m_pCredits->GetDownloadedTotal();
				uint64	qwClient2Credits = pClient2->m_pCredits->GetDownloadedTotal();

				if (qwClient1Credits > qwClient2Credits)
					iCompare = 1;
				else if (qwClient1Credits < qwClient2Credits)
					iCompare = -1;
				else
					iCompare = 0;
			}
			else
			{
				iCompare = (!pClient1->m_pCredits) ? ( (!pClient2->m_pCredits)? 0:1) : -1;
				iSortMod = 1;
			}
			break;
		}
		case CLCOL_CLIENTSOFTWARE:
		{
			iCompare = SortClient(pClient1, pClient2, iSortMod);
			if (iCompare == 0)
			{
				iCompare = SortProc(lParam1, lParam2, CLCOL_USERNAME);
			//	Sort always in ascending order
				iSortMod = 1;
			}
			break;
		}
		case CLCOL_CONNECTEDTIME:
		{
#ifdef OLD_SOCKETS_ENABLED
			bool	bConnected1 = false, bConnected2 = false;

			if (pClient1->m_pRequestSocket != NULL)
				bConnected1 = pClient1->m_pRequestSocket->IsConnected();
			if (pClient2->m_pRequestSocket != NULL)
				bConnected2 = pClient2->m_pRequestSocket->IsConnected();

			iCompare = bConnected1 - bConnected2;
#endif //OLD_SOCKETS_ENABLED
			if (iCompare == 0)
			{
				iCompare = SortProc(lParam1, lParam2, CLCOL_USERNAME);
				iSortMod = 1;	//	Sort always in ascending order
			}
			break;
		}
		case CLCOL_USERHASH:
		{
			iCompare = memcmp(pClient1->GetUserHash(), pClient2->GetUserHash(), 16);
			break;
		}
	}

	return iCompare * iSortMod;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientListCtrl::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	NOPRM(pNMHDR);
//	Reset selection in DL list
	POSITION posSelClient = g_App.m_pMDlg->m_wndTransfer.m_ctlDownloadList.GetFirstSelectedItemPosition();

	while (posSelClient != NULL)
	{
		int iSelClientListIndex = g_App.m_pMDlg->m_wndTransfer.m_ctlDownloadList.GetNextSelectedItem(posSelClient);
		g_App.m_pMDlg->m_wndTransfer.m_ctlDownloadList.SetItemState(iSelClientListIndex, 0, LVIS_SELECTED);
	}

	RefreshInfo();
	*pResult = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientListCtrl::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	POSITION posSelClient = GetFirstSelectedItemPosition();
	NOPRM(pNMHDR);

	if (posSelClient)
	{
		CUpDownClient* pClient = reinterpret_cast<CUpDownClient*>(GetItemData(GetNextSelectedItem(posSelClient)));

		if (g_App.m_pPrefs->GetDetailsOnClick())
		{
			CClientDetails			dialog(IDS_CD_TITLE, pClient, this, 0);
			dialog.DoModal();
		}
		else
		{
			g_App.m_pMDlg->m_wndChat.StartSession(pClient);
		}
	}
	*pResult = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientListCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	RefreshInfo();
	CMuleListCtrl::OnKeyUp(nChar, nRepCnt, nFlags);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientListCtrl::RefreshInfo(void)
{
	if (GetFocus() == this)
	{
		POSITION		posSelClient = GetFirstSelectedItemPosition();

		if (posSelClient == 0)
		{
	 		g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.SetList(INFOLISTTYPE_NONE);
		}
		else
		{
			CUpDownClient* pClient = reinterpret_cast<CUpDownClient*>(GetItemData(GetNextSelectedItem(posSelClient)));

			g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.SetList(INFOLISTTYPE_SOURCE, pClient);
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientListCtrl::OnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
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
						case CLCOL_USERNAME:
							_tcsncpy(pDispInfo->item.pszText, pClient->GetUserName(), pDispInfo->item.cchTextMax);
							pDispInfo->item.pszText[pDispInfo->item.cchTextMax - 1] = _T('\0');
							break;

						case CLCOL_CLIENTSOFTWARE:
							_tcsncpy(pDispInfo->item.pszText, pClient->GetFullSoftVersionString(), pDispInfo->item.cchTextMax);
							pDispInfo->item.pszText[pDispInfo->item.cchTextMax - 1] = _T('\0');
							break;

						case CLCOL_USERHASH:
						{
							TCHAR	acHashStr[MAX_HASHSTR_SIZE];

						// As hash is short and has a fixed size, not buffer size check is performed
							_tcscpy(pDispInfo->item.pszText, md4str(pClient->GetUserHash(), acHashStr));
							break;
						}
						case CLCOL_COUNTRY:
							_tcsncpy(pDispInfo->item.pszText, pClient->GetCountryName(), pDispInfo->item.cchTextMax);
							pDispInfo->item.pszText[pDispInfo->item.cchTextMax - 1] = _T('\0');
							break;

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
//	SortClient() compares two clients by software type/version/mod and returns -1,0,1. If 'iSortMode' is
//		1 then the sort is ascending, if -1 then descending. If the two clients can't be sorted by
//		software then they're sorted by user name.
int CClientListCtrl::SortClient(CUpDownClient *pClient1, CUpDownClient *pClient2, int iSortMode)
{
	int		iCompare = 0;

//	If either client is using unknown software, sort them by software ID (putting them at the end)
	if (pClient1->GetClientSoft() == SO_UNKNOWN || pClient2->GetClientSoft() == SO_UNKNOWN)
	{
		iCompare = (pClient1->GetClientSoft() - pClient2->GetClientSoft()) * iSortMode;	//append all unknown ones at the end
	}
//	If the clients are using the same software...
	else
	{
		if ((iCompare = (pClient1->GetClientSoft() - pClient2->GetClientSoft())) == 0)
		{
		//	Sort by mod version
			if ((iCompare = (pClient2->GetVersion() - pClient1->GetVersion())) == 0)
			{
				if (!pClient1->IsModStringEmpty())
					iCompare = (!pClient2->IsModStringEmpty()) ?
						_tcsicmp(pClient1->GetModString(), pClient2->GetModString()) : -1;
				else if (!pClient2->IsModStringEmpty())
					iCompare = 1;
			}
		}
	}
	return iCompare;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CClientListCtrl::AddDirtyClient(CUpDownClient *pClient)
{
	bool bResult = false;

//	If 'pClient' is not already in the dirty client list...
	if (m_pvecDirtyClients != NULL
		&& ::find(m_pvecDirtyClients->begin(),m_pvecDirtyClients->end(),pClient) == m_pvecDirtyClients->end())
	{
		m_pvecDirtyClients->push_back(pClient);
		bResult = true;
	}

	return bResult;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetDirtyClients() detaches the current dirty client list and returns it to the caller (who's responsible
//	for destroying it). This allows the client list control to update without locking down the client list.
CClientListCtrl::ClientVector* CClientListCtrl::GetDirtyClients()
{
	ClientVector		*pvecDirtyClients = NULL;

	if (!m_pvecDirtyClients->empty())
	{
		pvecDirtyClients = m_pvecDirtyClients;
		m_pvecDirtyClients = new ClientVector();
	}

	return pvecDirtyClients;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CClientListCtrl::PreTranslateMessage(MSG *pMsg)
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
void CClientListCtrl::ShowSelectedUserDetails()
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
