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
#include "QueueListCtrl.h"
#include "SharedFileList.h"
#include "otherfunctions.h"
#include "opcodes.h"
#include "Details\ClientDetails.h"
#include "TitleMenu.h"
#include "IP2Country.h"
#include "MemDC.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CQueueListCtrl, CMuleListCtrl)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_CLICK, OnNMClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclkQueuelist)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
	ON_WM_KEYUP()
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CQueueListCtrl, CMuleListCtrl)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CQueueListCtrl::CQueueListCtrl()
{
	memset(&m_bSortAscending, true, sizeof(m_bSortAscending));

	SetGeneralPurposeFind(true);

	m_pvecDirtyClients = new ClientVector();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CQueueListCtrl::~CQueueListCtrl()
{
	safe_delete(m_pvecDirtyClients);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CQueueListCtrl::Init()
{
	static const uint16 s_auColHdr[][2] =
	{
		{ LVCFMT_LEFT, 150 },	// QLCOL_USERNAME
		{ LVCFMT_LEFT, 275 },	// QLCOL_FILENAME
		{ LVCFMT_LEFT, 110 },	// QLCOL_FILEPRIORITY
		{ LVCFMT_LEFT, 110 },	// QLCOL_PARTS
		{ LVCFMT_LEFT, 110 },	// QLCOL_PROGRESS
		{ LVCFMT_LEFT,  60 },	// QLCOL_QLRATING
		{ LVCFMT_LEFT,  60 },	// QLCOL_SCORE
		{ LVCFMT_LEFT,  60 },	// QLCOL_SFRATIO
		{ LVCFMT_LEFT,  60 },	// QLCOL_RFRATIO
		{ LVCFMT_LEFT,  60 },	// QLCOL_TIMESASKED
		{ LVCFMT_LEFT, 110 },	// QLCOL_LASTSEEN
		{ LVCFMT_LEFT, 110 },	// QLCOL_ENTEREDQUEUE
		{ LVCFMT_LEFT,  60 },	// QLCOL_BANNED
		{ LVCFMT_LEFT, 150 }	// QLCOL_COUNTRY
	};
	CImageList		ilDummyImageList;

	ilDummyImageList.Create(1, 17,g_App.m_iDfltImageListColorFlags | ILC_MASK, 1, 1);
	SetImageList(&ilDummyImageList, LVSIL_SMALL);
	ilDummyImageList.Detach();

	m_iClientFilter = CLI_FILTER_NONE;

	memzero(m_iColumnMaxWidths, sizeof(m_iColumnMaxWidths));
//	Index of the column being measured. -1 for none.
	m_iMeasuringColumn = -1;

	SetExtendedStyle(LVS_EX_FULLROWSELECT);
	for (unsigned ui = 0; ui < ARRSIZE(s_auColHdr); ui++)
		InsertColumn(ui, _T(""), static_cast<int>(s_auColHdr[ui][0]), static_cast<int>(s_auColHdr[ui][1]), ui);

	Localize();

	LoadSettings(CPreferences::TABLE_QUEUE);
	if (g_App.m_pPrefs->DoUseSort())
	{
		SortInit(g_App.m_pPrefs->GetQueueSortCol());
	}
	else
	{
		int		iSortCode = g_App.m_pPrefs->GetColumnSortItem(CPreferences::TABLE_QUEUE);

		iSortCode |= g_App.m_pPrefs->GetColumnSortAscending(CPreferences::TABLE_QUEUE) ? MLC_SORTASC : MLC_SORTDESC;
		SortInit(iSortCode);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CQueueListCtrl::SortInit(int iSortCode)
{
//	Get the sort column
	int iSortColumn = (iSortCode & MLC_COLUMNMASK);
//	Get the sort order
	bool bSortAscending = (iSortCode & MLC_SORTDESC) == 0;

	SetSortArrow(iSortColumn,bSortAscending);
	SortItems(SortProc, iSortCode);
	m_bSortAscending[iSortColumn] = bSortAscending;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CQueueListCtrl::Localize()
{
	static const uint16 s_auResTbl[] =
	{
		IDS_QL_USERNAME,	// QLCOL_USERNAME
		IDS_FILE,			// QLCOL_FILENAME
		IDS_FILEPRIO,		// QLCOL_FILEPRIORITY
		IDS_UP_PARTS,		// QLCOL_PARTS
		IDS_DL_PROGRESS,	// QLCOL_PROGRESS
		IDS_RATING,			// QLCOL_QLRATING
		IDS_SCORE,			// QLCOL_SCORE
		IDS_SFRATIO,		// QLCOL_SFRATIO
		IDS_RFRATIO,		// QLCOL_RFRATIO
		IDS_ASKED,			// QLCOL_TIMESASKED
		IDS_LASTSEEN,		// QLCOL_LASTSEEN
		IDS_ENTERQUEUE,		// QLCOL_ENTEREDQUEUE
		IDS_BANNED,			// QLCOL_BANNED
		IDS_COUNTRY			// QLCOL_COUNTRY
	};

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
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CQueueListCtrl::AddClient(CUpDownClient *pClient)
{
	if (!g_App.m_pMDlg->IsRunning())
		return;
	if (!pClient)
		return;

	if (m_iClientFilter != 0)
		if (m_iClientFilter != GetClientFilterType(pClient))
			return;

	uint32		iNumItems = GetItemCount();

	iNumItems = InsertItem(LVIF_TEXT|LVIF_PARAM,iNumItems,LPSTR_TEXTCALLBACK,0,0,1,reinterpret_cast<LPARAM>(pClient));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CQueueListCtrl::RemoveClient(CUpDownClient* pClient)
{
	if (!g_App.m_pMDlg->IsRunning())
		return;
	if (pClient == NULL)
		return;

	if (g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.GetType() == INFOLISTTYPE_SOURCE)
	{
		if (g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.GetClient() == pClient)
		{
 			g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.SetList(INFOLISTTYPE_NONE);
		}
	}

	LVFINDINFO		find;
	int			iResult;

	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)pClient;

	iResult = FindItem(&find);
	if (iResult != (-1))
		DeleteItem(iResult);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CQueueListCtrl::UpdateClient(CUpDownClient *pClient)
{
	if (pClient != NULL)
	{
		if (AddDirtyClient(pClient))
			PostUniqueMessage(WM_QL_REFRESH);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CQueueListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
#define LIST_CELL_PADDING	6		//should be even number
	if (!g_App.m_pMDlg->IsRunning() || !lpDrawItemStruct->itemData)
		return;

	CDC		*odc = CDC::FromHandle(lpDrawItemStruct->hDC);
	BOOL	bCtrlFocused = ((GetFocus() == this) || (GetStyle() & LVS_SHOWSELALWAYS));
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
			case QLCOL_USERNAME:
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
			case QLCOL_FILENAME:
				if (pKnownFile)
					strBuffer = pKnownFile->GetFileName();
				else
					strBuffer = _T("?");
				break;

			case QLCOL_FILEPRIORITY:
			{
				if (pKnownFile)
				{
					UINT		dwResStrId;

					switch (pKnownFile->GetULPriority())
					{
						case PR_RELEASE:
							dwResStrId = IDS_PRIORELEASE;
							break;
						case PR_HIGH:
							dwResStrId = IDS_PRIOHIGH;
							break;
						case PR_LOW:
							dwResStrId = IDS_PRIOLOW;
							break;
						case PR_VERYLOW:
							dwResStrId = IDS_PRIOVERYLOW;
							break;
						default:
							dwResStrId = IDS_PRIONORMAL;
							break;
					}
					GetResString(&strBuffer, dwResStrId);
				}
				else
					strBuffer = _T("?");
				break;
			}
			case QLCOL_PARTS:
				if (pClient->GetUpPartCount())
					strBuffer.Format(_T("%u/%u"), pClient->GetAvailUpPartCount(), pClient->GetUpPartCount());
				else
					strBuffer = _T("");
				break;

			case QLCOL_PROGRESS:
			{
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
			}
			case QLCOL_QLRATING:
				strBuffer.Format(_T("%u"), pClient->GetScore(true));
				break;

			case QLCOL_SCORE:
			//	Note: actually the client, which is downloading from us should be not in WaitingQueue
				if (pClient->IsDownloading())
					strBuffer = _T("-");
				else
					strBuffer.Format((pClient->IsAddNextConnect()) ? _T("%u*") : _T("%u"), pClient->GetScore());
				break;

			case QLCOL_SFRATIO:
				if (pKnownFile != NULL)
					strBuffer.Format(_T("%.2f"), pKnownFile->GetSizeRatio());
				else
					strBuffer = _T("-");
				break;

			case QLCOL_RFRATIO:
				if (pKnownFile != NULL)
					strBuffer.Format(_T("%.2f"), pKnownFile->GetPopularityRatio());
				else
					strBuffer = _T("-");
				break;

			case QLCOL_TIMESASKED:
				strBuffer.Format(_T("%u"), pClient->GetAskedCount());
				break;

			case QLCOL_LASTSEEN:
				strBuffer = CastSecondsToHM((GetTickCount() - pClient->GetLastUpRequest())/1000);
				break;

			case QLCOL_ENTEREDQUEUE:
				strBuffer = CastSecondsToHM((GetTickCount() - pClient->GetWaitStartTime())/1000);
				break;

			case QLCOL_BANNED:
				strBuffer = YesNoStr(pClient->IsBanned());
				break;

			case QLCOL_COUNTRY:
				strBuffer = pClient->GetCountryName();
				break;
		}
		if (iColumn != QLCOL_PROGRESS)
		{
			dc->DrawText(strBuffer, &r, iCalcFlag);
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
	if (lpDrawItemStruct->itemState & ODS_SELECTED)
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
#undef LIST_CELL_PADDING
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CQueueListCtrl::OnWndMsg(UINT iMessage,WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	BOOL		bHandled = TRUE;

	EMULE_TRY

	switch (iMessage)
	{
		case WM_QL_REFRESH:
		{
			CQueueListCtrl::ClientVector	   *pvecDirtyClients = GetDirtyClients();

			if (pvecDirtyClients != NULL)
			{
				SetRedraw(FALSE);

				int		iNumDirtySources = pvecDirtyClients->size();

				for (int i = 0; i < iNumDirtySources; i++)
				{
					LVFINDINFO		find;
					int				iResult;

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
			bHandled = FALSE;
			break;
	}

	EMULE_CATCH

	if (!bHandled)
		bHandled = CMuleListCtrl::OnWndMsg(iMessage, wParam, lParam, pResult);

	return bHandled;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CQueueListCtrl::OnNMDividerDoubleClick(NMHEADER *pHeader)
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

// CQueueListCtrl message handlers

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CQueueListCtrl::OnContextMenu(CWnd *pWnd, CPoint point)
{
	NOPRM(pWnd);
	EMULE_TRY

	CTitleMenu		menuClient;
	CMenu			menuFilter;
	CUpDownClient	*pClient = NULL;
	POSITION		posSelClient = GetFirstSelectedItemPosition();
	UINT			dwFlags = MF_STRING | MF_GRAYED;

	if (posSelClient)
	{
		dwFlags = MF_STRING;
		pClient = reinterpret_cast<CUpDownClient*>(GetItemData(GetNextSelectedItem(posSelClient)));

		if (pClient)
		{
			g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.SetList(INFOLISTTYPE_SOURCE, pClient);
		}
	}

//	Show filtered list of clients
	menuFilter.CreateMenu();
	menuFilter.AppendMenu( MF_STRING | ((m_iClientFilter == CLI_FILTER_NONE) ? MF_CHECKED : MF_UNCHECKED),
		MP_FILTER_NONE, GetResString(IDS_CAT_REINVADD) );
//	menuFilter.AppendMenu( MF_STRING | ((m_iClientFilter == CLI_FILTER_BANNED) ? MF_CHECKED : MF_UNCHECKED),
//		MP_FILTER_BANNED, GetResString(IDS_WEB_SHOW_UPLOAD_QUEUE_BANNED) );
	menuFilter.AppendMenu( MF_STRING | ((m_iClientFilter == CLI_FILTER_FRIEND) ? MF_CHECKED : MF_UNCHECKED),
		MP_FILTER_FRIEND, GetResString(IDS_WEB_SHOW_UPLOAD_QUEUE_FRIEND) );
	menuFilter.AppendMenu( MF_STRING | ((m_iClientFilter == CLI_FILTER_CREDIT) ? MF_CHECKED : MF_UNCHECKED),
		MP_FILTER_CREDIT, GetResString(IDS_WEB_SHOW_UPLOAD_QUEUE_CREDIT) );

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
	menuClient.AppendMenu(dwFlags,MP_MESSAGE, GetStringFromShortcutCode(IDS_SEND_MSG, SCUT_SRC_MSG, SSP_TAB_PREFIX));
	menuClient.AppendMenu( dwFlags | ((pClient != NULL && pClient->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED),
		MP_SHOWLIST, GetStringFromShortcutCode(IDS_VIEWFILES, SCUT_SRC_SHAREDFILES, SSP_TAB_PREFIX) );
	menuClient.AppendMenu(dwFlags | ((pClient && pClient->IsBanned()) ? MF_ENABLED : MF_GRAYED), MP_UNBAN, GetResString(IDS_UNBAN));

	menuClient.AppendMenu(MF_SEPARATOR);
	menuClient.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)menuFilter.m_hMenu, GetResString(IDS_QUEUEFILTER));

	menuClient.SetDefaultItem((g_App.m_pPrefs->GetDetailsOnClick()) ? MP_DETAIL : MP_MESSAGE);
	menuClient.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this, NULL);

//	Menu objects are destroyed in their destructor

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CQueueListCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	POSITION posSelClient = GetFirstSelectedItemPosition();
	NOPRM(lParam);

	if (posSelClient)
	{
		CUpDownClient* pClient = reinterpret_cast<CUpDownClient*>(GetItemData(GetNextSelectedItem(posSelClient)));

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
			case MP_UNBAN:
				if( pClient->IsBanned() )
					pClient->UnBan();
				break;
			case MP_DETAIL:
			{
				CClientDetails dialog(IDS_CD_TITLE, pClient, this, 0);
				dialog.DoModal();
				break;
			}
		}
	}
	switch(wParam)
	{
		case MP_FILTER_NONE:
		{
			m_iClientFilter = CLI_FILTER_NONE;
			ShowFilteredList();
			g_App.m_pMDlg->m_wndTransfer.UpdateQueueFilter();
			break;
		}
		case MP_FILTER_BANNED:
		{
			m_iClientFilter = CLI_FILTER_BANNED;
			ShowFilteredList();
			g_App.m_pMDlg->m_wndTransfer.UpdateQueueFilter();
			break;
		}
		case MP_FILTER_FRIEND:
		{
			m_iClientFilter = CLI_FILTER_FRIEND;
			ShowFilteredList();
			g_App.m_pMDlg->m_wndTransfer.UpdateQueueFilter();
			break;
		}
		case MP_FILTER_CREDIT:
		{
			m_iClientFilter = CLI_FILTER_CREDIT;
			ShowFilteredList();
			g_App.m_pMDlg->m_wndTransfer.UpdateQueueFilter();
			break;
		}
	}
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CQueueListCtrl::OnColumnClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_LISTVIEW	*pNMListView = reinterpret_cast<NM_LISTVIEW*>(pNMHDR);
	int			iSubItem = pNMListView->iSubItem;
	bool		bSortOrder = m_bSortAscending[iSubItem];

// Reverse sorting direction for the same column and keep the same if column was changed
	if (static_cast<int>(m_dwParamSort & MLC_COLUMNMASK) == iSubItem)
		m_bSortAscending[iSubItem] = bSortOrder = !bSortOrder;

	SetSortArrow(iSubItem, bSortOrder);
	SortItems(SortProc, iSubItem + ((bSortOrder) ? MLC_SORTASC : MLC_SORTDESC));
	g_App.m_pPrefs->SetColumnSortItem(CPreferences::TABLE_QUEUE, iSubItem);
	g_App.m_pPrefs->SetColumnSortAscending(CPreferences::TABLE_QUEUE, bSortOrder);
	*pResult = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CQueueListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CUpDownClient* pClient1 = (CUpDownClient*)lParam1;
	CUpDownClient* pClient2 = (CUpDownClient*)lParam2;

	if (pClient1 == NULL || pClient2 == NULL)
		return 0;

	CKnownFile* pSharedFile1 = g_App.m_pSharedFilesList->GetFileByID(pClient1->m_reqFileHash);
	CKnownFile* pSharedFile2 = g_App.m_pSharedFilesList->GetFileByID(pClient2->m_reqFileHash);

	int	iCompare = 0;
	int	iSortMod = ((lParamSort & MLC_SORTDESC) == 0) ? 1 : -1;
	int	iSortColumn = (lParamSort & MLC_COLUMNMASK);

	switch(iSortColumn)
	{
		case QLCOL_USERNAME:
		{
			iCompare = pClient1->CmpUserNames(pClient2->GetUserName());
			break;
		}
		case QLCOL_COUNTRY:
		{
			CString strCountry1 = pClient1->GetCountryName();
			CString strCountry2 = pClient2->GetCountryName();

			if (!strCountry1.IsEmpty() && !strCountry2.IsEmpty())
				iCompare = _tcsicmp(strCountry1, strCountry2);
			else
			{
				iCompare = strCountry1.IsEmpty() ? (strCountry2.IsEmpty() ? 0 : 1) : -1;
				iSortMod = 1;		//empty entries at the bottom in any case
			}

			if (iCompare == 0)
			{
				iCompare = SortProc(lParam1,lParam2,QLCOL_USERNAME);
				iSortMod = 1;		//sort allways in ascending order
			}
			break;
		}
		case QLCOL_FILENAME:
			if ((pSharedFile1 != NULL) && (pSharedFile2 != NULL))
				iCompare = pSharedFile1->CmpFileNames(pSharedFile2->GetFileName());
			else
				iCompare = (pSharedFile1 == NULL) ? ((pSharedFile2 == NULL) ? 0 : 1) : -1;
			if (iCompare == 0)
			{
				iCompare = SortProc(lParam1,lParam2, QLCOL_USERNAME);
				iSortMod = 1;		//eklmn: sort allways in ascending order
			}
			break;
		case QLCOL_FILEPRIORITY:
			if(pSharedFile1 != NULL && pSharedFile2 != NULL)		//Cax2 - right priority sortig
			{
				if (pSharedFile1->GetULPriority() == PR_VERYLOW)
					iCompare = -1;
				else if (pSharedFile2->GetULPriority() == PR_VERYLOW)
					iCompare = 1;
				else
					iCompare = pSharedFile1->GetULPriority() - pSharedFile2->GetULPriority();
			}
			else
				iCompare = ( pSharedFile1 == NULL ) ? ((pSharedFile2 == NULL) ? 0 : 1) : -1;
			//eklmn: additional sorting by filename & username
			if (iCompare == 0)
			{
				iCompare = SortProc(lParam1,lParam2, QLCOL_FILENAME);
				iSortMod = 1;		//eklmn: sort allways in ascending order
			}
			break;
		case QLCOL_PARTS:
		case QLCOL_PROGRESS:
		{  // obaldin: sort by upload progress
			uint32 partcnt1 = pClient1->GetUpPartCount();
			uint32 partcnt2 = pClient2->GetUpPartCount();

			if((partcnt1!=0) && (partcnt2!=0))
			{
				double	dblProgress1 = static_cast<double>(pClient1->GetAvailUpPartCount())/static_cast<double>(partcnt1);
				double	dblProgress2 = static_cast<double>(pClient2->GetAvailUpPartCount())/static_cast<double>(partcnt2);

				iCompare = (dblProgress1 > dblProgress2) ? 1 : ((dblProgress1 < dblProgress2) ? -1: 0);
			}
			else
				iCompare = ( partcnt1 == 0 ) ? ((partcnt2 == 0) ? 0 : 1) : -1;
			break;
		}
		case QLCOL_QLRATING:
			iCompare = pClient1->GetScore(true) - pClient2->GetScore(true);
			break;
		case QLCOL_SCORE:
			iCompare =  pClient1->GetScore() - pClient2->GetScore();
			break;
		case QLCOL_SFRATIO: //small file  asc
			if( (pSharedFile1 != NULL) && (pSharedFile2 != NULL))
				iCompare = static_cast<int>(100.0*(pSharedFile1->GetSizeRatio() - pSharedFile2->GetSizeRatio()) );
			else
				iCompare = ( pSharedFile1 == NULL ) ? ((pSharedFile2 == NULL) ? 0 : 1) : -1;
			break;
		case QLCOL_RFRATIO: //rare file asc
			if( (pSharedFile1 != NULL) && (pSharedFile2 != NULL))
				iCompare = static_cast<int>(100.0 *(pSharedFile1->GetPopularityRatio() - pSharedFile2->GetPopularityRatio()) );
			else
				iCompare = ( pSharedFile1 == NULL ) ? ((pSharedFile2 == NULL) ? 0 : 1) : -1;
			break;
		case QLCOL_TIMESASKED:
			iCompare =  pClient1->GetAskedCount() - pClient2->GetAskedCount();
			break;
		case QLCOL_LASTSEEN:
			iCompare =  pClient2->GetLastUpRequest() - pClient1->GetLastUpRequest();
			break;
		case QLCOL_ENTEREDQUEUE:
			iCompare = pClient2->GetWaitStartTime() - pClient1->GetWaitStartTime();
			break;
		case QLCOL_BANNED:
			iCompare = pClient1->IsBanned() - pClient2->IsBanned();
			//eklmn: additional sorting by filename & username
			if (iCompare == 0)
			{
				iCompare = SortProc(lParam1,lParam2, QLCOL_FILENAME);
				iSortMod = 1;		//eklmn: sort allways in ascending order
			}
			break;
		default:
			iCompare =  0;
	}

	return iCompare*iSortMod;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CQueueListCtrl::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
//	Reset selection in DL list
	POSITION posSelClient = g_App.m_pMDlg->m_wndTransfer.m_ctlDownloadList.GetFirstSelectedItemPosition();
	NOPRM(pNMHDR);

	while (posSelClient != NULL)
	{
		int iSelClientListIndex = g_App.m_pMDlg->m_wndTransfer.m_ctlDownloadList.GetNextSelectedItem(posSelClient);
		g_App.m_pMDlg->m_wndTransfer.m_ctlDownloadList.SetItemState(iSelClientListIndex, 0, LVIS_SELECTED);
	}

	RefreshInfo();
	*pResult = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CQueueListCtrl::OnNMDblclkQueuelist(NMHDR *pNMHDR, LRESULT *pResult)
{
	POSITION posSelClient = GetFirstSelectedItemPosition();
	NOPRM(pNMHDR);

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
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CQueueListCtrl::ShowSelectedUserDetails()
{
	POINT point;

	::GetCursorPos(&point);

	CPoint p = point;

    ScreenToClient(&p);

    int it = HitTest(p);

    if (it == -1)
	{
		return;
	}
	SetSelectionMark(it);

	CUpDownClient* pClient = reinterpret_cast<CUpDownClient*>(GetItemData(GetSelectionMark()));

	if (pClient != NULL)
	{
			CClientDetails dialog(IDS_CD_TITLE, pClient, this, 0);
			dialog.DoModal();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CQueueListCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	RefreshInfo();
	CMuleListCtrl::OnKeyUp(nChar, nRepCnt, nFlags);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CQueueListCtrl::RefreshInfo(void)
{
	if (GetFocus() == this)
	{
		POSITION		posSelClient = GetFirstSelectedItemPosition();

		if(posSelClient == 0)
		{
	 		g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.SetList(INFOLISTTYPE_NONE);
		}
		else
		{
			CUpDownClient		*pClient = reinterpret_cast<CUpDownClient*>(GetItemData(GetNextSelectedItem(posSelClient)));

	 		g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.SetList(INFOLISTTYPE_SOURCE,pClient);
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CQueueListCtrl::OnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
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
						case QLCOL_USERNAME:
							_tcsncpy(pDispInfo->item.pszText, pClient->GetUserName(), pDispInfo->item.cchTextMax);
							pDispInfo->item.pszText[pDispInfo->item.cchTextMax - 1] = _T('\0');
							break;

						case QLCOL_COUNTRY:
							_tcsncpy(pDispInfo->item.pszText, pClient->GetCountryName(), pDispInfo->item.cchTextMax);
							pDispInfo->item.pszText[pDispInfo->item.cchTextMax - 1] = _T('\0');
							break;

						case QLCOL_FILENAME:
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
//DonGato: show filtered list of clients
void CQueueListCtrl::ShowFilteredList()
{
	CUpDownClient	   *pClient;

	SetRedraw(FALSE);
	DeleteAllItems();
	for (POSITION pos = g_App.m_pUploadQueue->GetHeadPosition(); pos != NULL; )
	{
		pClient = g_App.m_pUploadQueue->GetNext(pos);
		if (pClient != NULL)
		{
			AddClient(pClient);
		}
	}
	SetRedraw(TRUE);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Show filtered list of clients
int CQueueListCtrl::GetClientFilterType(CUpDownClient* client)
{
	if (client->IsBanned())
		return 1;
	else if (client->IsFriend())
		return 2;
	else if (client->m_pCredits->HasHigherScoreRatio(client->GetIP()))
		return 3;
	else
		return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CQueueListCtrl::PreTranslateMessage(MSG *pMsg)
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
bool CQueueListCtrl::AddDirtyClient(CUpDownClient* pClientItem)
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
CQueueListCtrl::ClientVector* CQueueListCtrl::GetDirtyClients()
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
