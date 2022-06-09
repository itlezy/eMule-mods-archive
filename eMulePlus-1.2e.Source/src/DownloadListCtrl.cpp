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
#include "DownloadListCtrl.h"
#include "MemDC.h"
#include "TitleMenu.h"
#include "SharedFileList.h"
#include "otherfunctions.h"
#include "updownclient.h"
#include "opcodes.h"
#include "Details\ClientDetails.h"
#include "Details\FileDetails.h"
#include "CommentDialogLst.h"
#include "KeyboardShortcut.h"
#include "InputBox.h"
#include "IP2Country.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define DLC_DT_TEXT (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS)
#define DLC_BARUPDATE 512

BEGIN_MESSAGE_MAP(CDownloadListCtrl, CMuleListCtrl)
	ON_NOTIFY_REFLECT(LVN_ITEMACTIVATE, OnItemActivate)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnListModified)
	ON_NOTIFY_REFLECT(LVN_INSERTITEM, OnListModified)
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, OnListModified)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclkDownloadlist)
	ON_NOTIFY_REFLECT(NM_CLICK, OnNMClick)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
	ON_WM_SIZE()
	ON_WM_KEYUP()
	ON_WM_KEYDOWN()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CDownloadListCtrl, CMuleListCtrl)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDownloadListCtrl::CDownloadListCtrl()
{
	m_iCurrentSortItem = DLCOL_FILENAME;
	m_bSortAscending = true;
	m_iSourceSortItem = DLCOL_FILENAME;
	m_bSortSourcesAscending = true;
	m_iSourceSortItem2 = -1;
	m_bSortSourcesAscending2 = true;
	m_bShowSrc = false;
	memzero(&m_byteSortAscending, sizeof(m_byteSortAscending));
	SetGeneralPurposeFind(true);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDownloadListCtrl::~CDownloadListCtrl()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::Init()
{
	static const uint16 s_auIconResID[] =
	{
		IDI_DCS1,
		IDI_DCS2,
		IDI_DCS3,
		IDI_DCS4,
		IDI_DCS5,
		IDI_LISTMINUS,
		IDI_LISTNONE,
		IDI_LISTPLUS,
		IDI_RATING_NO,
		IDI_RATING_EXCELLENT,
		IDI_RATING_GOOD,
		IDI_RATING_FAIR,
		IDI_RATING_POOR,
		IDI_RATING_FAKE,
		IDI_A4AFAUTO,

		IDI_STATUS_COMPLETE,
		IDI_STATUS_COMPLETING,
		IDI_STATUS_DOWNLOADING,
		IDI_STATUS_ERRONEOUS,
		IDI_STATUS_HASHING,
		IDI_STATUS_PAUSED,
		IDI_STATUS_STALLED,
		IDI_STATUS_STOPPED,
		IDI_STATUS_WAITING,
		IDI_STATUS_WAITINGHASH
	};
	static const uint16 s_auColHdr[][2] =
	{
		{ LVCFMT_LEFT,  260 },	// DLCOL_FILENAME
		{ LVCFMT_RIGHT,  60 },	// DLCOL_SIZE
		{ LVCFMT_RIGHT,  65 },	// DLCOL_TRANSFERRED
		{ LVCFMT_RIGHT,  65 },	// DLCOL_COMPLETED
		{ LVCFMT_RIGHT,  65 },	// DLCOL_SPEED
		{ LVCFMT_LEFT,  170 },	// DLCOL_PROGRESS
		{ LVCFMT_RIGHT,  50 },	// DLCOL_NUMSOURCES
		{ LVCFMT_LEFT,   55 },	// DLCOL_PRIORITY
		{ LVCFMT_LEFT,   70 },	// DLCOL_STATUS
		{ LVCFMT_LEFT,  110 },	// DLCOL_REMAINING
		{ LVCFMT_LEFT,  110 },	// DLCOL_REMAININGTIME
		{ LVCFMT_RIGHT,  60 },	// DLCOL_ULDLRATIO
		{ LVCFMT_RIGHT,  60 },	// DLCOL_QLRATING
		{ LVCFMT_LEFT,  110 },	// DLCOL_LASTSEENCOMPLETE
		{ LVCFMT_LEFT,  220 },	// DLCOL_LASTRECEIVED
		{ LVCFMT_LEFT,  100 },	// DLCOL_CATEGORY
		{ LVCFMT_LEFT,  110 },	// DLCOL_WAITED
		{ LVCFMT_RIGHT, 100 },	// DLCOL_AVGSPEED
		{ LVCFMT_RIGHT, 100 },	// DLCOL_AVGREMTIME
		{ LVCFMT_RIGHT, 100 },	// DLCOL_ETA
		{ LVCFMT_RIGHT, 100 }	// DLCOL_AVGETA
	};

	EMULE_TRY

	CImageList		ilDummyImageList;

	ilDummyImageList.Create(1, 17, g_App.m_iDfltImageListColorFlags | ILC_MASK, 1, 1);
	SetImageList(&ilDummyImageList, LVSIL_SMALL);
	ASSERT((GetStyle() & LVS_SHAREIMAGELISTS) == 0);
	ilDummyImageList.Detach();

	SetStyle();
	SetColors();

	m_bSmartFilter = false;

	FilterNoSources();

	ModifyStyle(LVS_SINGLESEL, 0);

	memzero(m_iColumnMaxWidths, sizeof(m_iColumnMaxWidths));
//	Index of the column being measured. -1 for none.
	m_iMeasuringColumn = -1;

	for (unsigned ui = 0; ui < ARRSIZE(s_auColHdr); ui++)
		InsertColumn(ui, _T(""), static_cast<int>(s_auColHdr[ui][0]), static_cast<int>(s_auColHdr[ui][1]));

	m_imageList.Create(16, 16, g_App.m_iDfltImageListColorFlags|ILC_MASK, ARRSIZE(s_auIconResID), 0);
	m_imageList.SetBkColor(CLR_NONE);
	FillImgLstWith16x16Icons(&m_imageList, s_auIconResID, ARRSIZE(s_auIconResID));

	Localize();

	LoadSettings(CPreferences::TABLE_DOWNLOAD);

//	Sort dialog
	if (g_App.m_pPrefs->DoUseSort())
	{
		SortInit(DL_OVERRIDESORT);
	}
	else
	{
	//	Use preferred sort order from preferences
		m_iCurrentSortItem = g_App.m_pPrefs->GetColumnSortItem(CPreferences::TABLE_DOWNLOAD);
		m_bSortAscending = g_App.m_pPrefs->GetColumnSortAscending(CPreferences::TABLE_DOWNLOAD);
		m_iSourceSortItem = g_App.m_pPrefs->GetColumnSortItem(CPreferences::TABLE_DOWNLOAD2);
		m_bSortSourcesAscending = g_App.m_pPrefs->GetColumnSortAscending(CPreferences::TABLE_DOWNLOAD2);

		m_byteSortAscending[m_iCurrentSortItem] =
			(m_byteSortAscending[m_iCurrentSortItem] & ~1) | ((m_bSortAscending) ? 1 : 0);
		m_byteSortAscending[m_iSourceSortItem] =
			(m_byteSortAscending[m_iSourceSortItem] & ~2) | ((m_bSortSourcesAscending) ? 2 : 0);

		SetSortArrow(m_iCurrentSortItem, m_bSortAscending);

		uint32	dwSortCode =
			m_iCurrentSortItem | (m_bSortAscending ? MLC_SORTASC : MLC_SORTDESC) |
			((m_iSourceSortItem | (m_bSortSourcesAscending ? MLC_SORTASC : MLC_SORTDESC)) << 16);

		SortItems(SortProc, dwSortCode);
	}

	m_iCurTabIndex = 0; // All tab
	m_eCurTabCat = CAT_ALL;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::Localize()
{
	static const uint16 s_auResTbl[] =
	{
		IDS_DL_FILENAME,
		IDS_DL_SIZE,
		IDS_DL_TRANSF,
		IDS_SF_COMPLETED,
		IDS_DL_SPEED,
		IDS_DL_PROGRESS,
		IDS_DL_SOURCES,
		IDS_PRIORITY,
		IDS_STATUS,
		IDS_DL_REMAINS,
		IDS_DLCOL_REMAININGTIME,
		IDS_DL_ULDL,
		IDS_RATING,
		IDS_LASTSEENCOMPLETE,
		IDS_LASTRECEPTION,
		IDS_CAT,
		IDS_WAITED,
		IDS_DLCOL_AVGSPEED,
		IDS_DLCOL_AVGREMTIME,
		IDS_DLCOL_ETA,
		IDS_DLCOL_AVGETA
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

		ShowFilesCount();
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	AddFileItem() adds part file item 'pFileItem' to the display it if it should be visible.
void CDownloadListCtrl::AddFileItem(CPartFileDLItem *pFileItem)
{
	EMULE_TRY

//	TODO: Make this asynchronous

//	If the file belongs to the currently displayed category, add it to the end of the list control (emule .30a)
	if (CCat::FileBelongsToGivenCat(pFileItem->GetFile(),m_eCurTabCat))
	{
		ListInsertFileItem(pFileItem, GetItemCount());
	}

	if (m_bSmartFilter)
	{
		AutoSetSourceFilters(pFileItem);
	}

	ShowFilesCount();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	AddSourceItem() adds source item 'pSourceItem' to the display it if it should be visible.
void CDownloadListCtrl::AddSourceItem(CSourceDLItem *pSourceItem)
{
	EMULE_TRY

	CPartFileDLItem	   *pParentFileItem = pSourceItem->GetParentFileItem();

	if (pParentFileItem == NULL || !pParentFileItem->m_bSrcsAreVisible)
		return;

//
//	Find the position of the next file item displayed or the end of the list
//
	int		iItem = ListGetFileItemIndex(pParentFileItem);
	int		iNumListItems = GetItemCount();

	if (iItem < 0)
		iItem = 0;
	while (iItem+1 < iNumListItems)
	{
		CPartFileDLItem	   *pFileItem = dynamic_cast<CPartFileDLItem*>(ListGetItemAt(iItem+1));

		if (pFileItem != NULL)
			break;
		iItem++;
	}

	if (!IsSourceFiltered(pSourceItem))
		ListInsertSourceItem(pSourceItem,iItem+1);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	RemoveSourceItem() removes the source item 'pSourceItem' from the list. If an error occurs,
//		the method does nothing.
void CDownloadListCtrl::RemoveSourceItem(CSourceDLItem *pSourceItem)
{
	EMULE_TRY

	if (pSourceItem != NULL && ::IsWindow(GetSafeHwnd()) && g_App.m_pMDlg->IsRunning())
	{
		const int		iItem = ListGetSourceItemIndex(pSourceItem);

		if (iItem != -1)
		{
			pSourceItem->SetVisibility(false);
			DeleteItem(iItem);
		}
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	IsSourceFiltered() returns true if the spec'd source should be removed
//		from the DownloadList window according to the current source filter.
bool CDownloadListCtrl::IsSourceFiltered(CSourceDLItem *pSourceItem)
{
#ifdef OLD_SOCKETS_ENABLED
	bool				bShowNewSource = true;
	CPartFileDLItem	   *pFileItem = pSourceItem->GetParentFileItem();

//	If the source item's parent part file isn't expanded, don't show it.
	if (!pFileItem->m_bSrcsAreVisible)
	{
		bShowNewSource = false;
	}
	else
	{
		CUpDownClient	*pSource = pSourceItem->GetSource();

		EnumDLQState	ds = pSource->GetDownloadState();

	//	If we're filtering out uploading sources and the source is uploading...
		if ( !(pFileItem->m_bShowUploadingSources && m_bShowUploadingSources)
		  && ds == DS_DOWNLOADING )
		{
			bShowNewSource = false;
		}
	//	If we're filtering out sources which have us on their upload queues
	//		and the source has issued us an upload queue rank...
		if ( !(pFileItem->m_bShowOnQueueSources && m_bShowOnQueueSources)
		  && ds == DS_ONQUEUE && pSource->GetRemoteQueueRank() > 0 )
		{
			bShowNewSource = false;
		}
	//	If we're NOT filtering out sources which has us on their upload queues
	//		but we're smart filtering and the source has issued us an upload
	//		rank above the preference...
		if ( m_bSmartFilter
		  && (pFileItem->m_bShowOnQueueSources && m_bShowOnQueueSources)
		  && ds == DS_ONQUEUE && pSource->GetRemoteQueueRank() > g_App.m_pPrefs->GetSmartFilterMaxQueueRank())
		{
			bShowNewSource = false;
		}
	//	If we're filtering out sources which report a full upload queue
	//		and the source returned 0 for our upload queue rank...
		if ( !(pFileItem->m_bShowFullQueueSources && m_bShowFullQueueSources)
		  && ds == DS_ONQUEUE && pSource->GetRemoteQueueRank() == 0 )
		{
			bShowNewSource = false;
		}
	//	If we're filtering out sources that are connected and this source is connected...
		if ( !(pFileItem->m_bShowConnectedSources && m_bShowConnectedSources)
		  && ds == DS_CONNECTED)
		{
			bShowNewSource = false;
		}
	//	If we're filtering out sources that are connecting and this source is connecting or connecting via server...
		if ( !(pFileItem->m_bShowConnectingSources && m_bShowConnectingSources)
		  && (ds == DS_CONNECTING || ds == DS_WAITCALLBACK) )
		{
			bShowNewSource = false;
		}
	//	If we're filtering out sources with no needed parts and this source has
	//		no needed parts...
		if ( !(pFileItem->m_bShowNNPSources && m_bShowNNPSources)
		  && ds == DS_NONEEDEDPARTS )
		{
			bShowNewSource = false;
		}
	//	If we're filtering out sources we couldn't connect to because we didn't
	//		have a free connection and that's the case for this source...
		if ( !(pFileItem->m_bShowWaitForFileReqSources && m_bShowWaitForFileReqSources)
		  && ds == DS_WAIT_FOR_FILE_REQUEST )
		{
			bShowNewSource = false;
		}
	//	If we have a Low ID and we're filtering out sources with a Low ID and
	//		this source does...
		if ( !(pFileItem->m_bShowLowToLowIDSources && m_bShowLowToLowIDSources)
		  && ds == DS_LOWTOLOWID )
		{
			bShowNewSource = false;
		}
	//	If we're filtering out LowID client on another server
		if ( !(pFileItem->m_bShowLowIDOnOtherSrvSources && m_bShowLowIDOnOtherSrvSources)
		  && ds == DS_LOWID_ON_OTHER_SERVER )
		{
			bShowNewSource = false;
		}
	//	If we're filtering out banned clients and this source has been banned, i.e. we banned him in
	//	our upload queue...
		if ( !(pFileItem->m_bShowBannedSources && m_bShowBannedSources)
		  && pSource->IsBanned() )
		{
			bShowNewSource = false;
		}
	//	If we're filtering out sources with an error condition (?) and this
	//		source has one...
		if ( !(pFileItem->m_bShowErrorSources && m_bShowErrorSources)
		  && ds == DS_ERROR )
		{
			bShowNewSource = false;
		}
	//	If we're filtering out sources for which we have an active request for
	//		a file other than the source item's parent...
		if ( !(pFileItem->m_bShowA4AFSources && m_bShowA4AFSources)
		  && pSourceItem->IsAskedForAnotherFile() )
		{
			bShowNewSource = false;
		}
	//	If we're filtering out unknown (?) sources and this source is unknown...
		if ( !(pFileItem->m_bShowUnknownSources && m_bShowUnknownSources)
		  && ds == DS_NONE )
		{
			bShowNewSource = false;
		}
	}

	return !bShowNewSource;
#else
	return false;
#endif //OLD_SOCKETS_ENABLED
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::RemoveFileItem(CPartFileDLItem *pFileItem)
{
	EMULE_TRY

	if (!g_App.m_pMDlg->IsRunning() || pFileItem == NULL)
		return;

//	If the InfoList is displaying this file...
	if ( g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.GetType() == INFOLISTTYPE_FILE
	  && g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.GetFile() == pFileItem->GetFile() )
	{
	//	...Set it to display nothing
		g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.SetList(INFOLISTTYPE_NONE);
	}

	int			iFilePos = ListGetFileItemIndex(pFileItem);

//	Remove the file item from the list
	if (iFilePos != -1)
	{
		pFileItem->SetVisibility(false);
		DeleteItem(iFilePos);
	}

	ShowFilesCount();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::UpdateSourceItem(CSourceDLItem *pSourceItem)
{
	EMULE_TRY

	if (pSourceItem != NULL && ::IsWindow(GetSafeHwnd()) && g_App.m_pMDlg->IsRunning())
	{
		int		iSourcePos = ListGetSourceItemIndex(pSourceItem);

		if (iSourcePos != -1)
		{
			bool	bDontUpdate = false;

			if (pSourceItem != NULL && IsSourceFiltered(pSourceItem))
			{
				HideSourceItem(pSourceItem);
				bDontUpdate = true;
			}
			if (!bDontUpdate)
			{
				if (pSourceItem != NULL)
					pSourceItem->ResetUpdateTimer();
				Update(iSourcePos);
			}
		}
	//	If we didn't find the source item in the list...
		else
		{
		//	If it's a source, check to see if it should be shown now
			if (pSourceItem != NULL)
			{
				if (!IsSourceFiltered(pSourceItem))
					ShowSourceItem(pSourceItem);
			}
		}
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::UpdateFileItems()
{
	CDownloadList::PartFileItemVector	*pvecFileItems = g_App.m_pDownloadList->GetFileItems();

	for (uint32 i = 0; i < pvecFileItems->size(); i++)
	{
		UpdateFileItem((*pvecFileItems)[i]);
	}

	delete pvecFileItems;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::UpdateSourceItems(CPartFileDLItem *pFileItem/*=NULL*/)
{
	CDownloadList::SourceItemVector	*pvecSourceItems;

//	If no part file item was specified, update all source items
	if (pFileItem == NULL)
		pvecSourceItems = g_App.m_pDownloadList->GetSourceItems();
	else
		pvecSourceItems = pFileItem->GetSources();

	for (uint32 i = 0; i < pvecSourceItems->size(); i++)
		UpdateSourceItem((*pvecSourceItems)[i]);

	delete pvecSourceItems;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::UpdateFileItem(CPartFileDLItem *pFileItem)
{
	EMULE_TRY

//	MOREVIT - The "running" check is probably superfluous here now that we're checking the window
//	If the file item isn't null and the window hasn't been destroyed (e.g. during shutdown)...
	if (pFileItem != NULL && ::IsWindow(GetSafeHwnd()) && g_App.m_pMDlg->IsRunning())
	{
		int		iFilePos = ListGetFileItemIndex(pFileItem);

	//	If we found the file item in the list...
		if (iFilePos != -1)
		{
			pFileItem->ResetUpdateTimer();
			Update(iFilePos);
			if (m_bSmartFilter)
				AutoSetSourceFilters(pFileItem);
		}
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	DrawFileItem() draws column 'nColumn' of the list row for part file 'pFileItem' in the
//		rectangle 'lpRect' in device context 'dc'. If 'm_iMeasuringColumn' is nonzero then
//		the width of drawing is measured and 'm_iColumnMaxWidths' is updated for that column
//		but nothing is drawn.
void CDownloadListCtrl::DrawFileItem(CDC *dc, int nColumn, LPRECT lpRect, CPartFileDLItem *pFileItem)
{
//	TODO: Put these constants somewhere sensible!
#define OFFSET_PLUSMINUS	18
#define OFFSET_STATUSICON	18
#define OFFSET_FILETYPEICON	19
#define OFFSET_RATINGICON	10
	EMULE_TRY

	if (IsColumnHidden(nColumn))
		return;

	RECT	r = *lpRect;
	int		iPadding = dc->GetTextExtent(_T(" "), 1).cx * 2;
	int		iWidth = (3 * iPadding)/2;
	bool	bMeasuring = (m_iMeasuringColumn >= 0);
	UINT	iCalcFlag = (bMeasuring) ? ((DLC_DT_TEXT | DT_CALCRECT) & ~DT_END_ELLIPSIS) : DLC_DT_TEXT;

	if (IsRightToLeftLanguage())
		iCalcFlag |= DT_RTLREADING;

	if (lpRect->left < lpRect->right)
	{
		CString			buffer;
		CPartFile		*pPartFile = pFileItem->GetFile();

		switch (nColumn)
		{
			case DLCOL_FILENAME:
			{
				POINT	point = { r.left, r.top + 1 };
				int		iIcon;
				EnumPartFileStatuses	eFileStatus = pPartFile->GetStatus();

			//	Draw the old-style +/- box
				iIcon = ( pPartFile->GetSourceCount() > 0 || pPartFile->GetSrcA4AFCount() > 0
						  &&  !(eFileStatus == PS_COMPLETING || eFileStatus == PS_COMPLETE || eFileStatus == PS_STOPPED) )
					  ? (pFileItem->m_bSrcsAreVisible ? DL_ICON_LISTMINUS : DL_ICON_LISTPLUS)
					  : DL_ICON_LISTNONE;
				if (!bMeasuring)
					m_imageList.Draw(dc, iIcon, point, ILD_NORMAL);
				else
					iWidth += OFFSET_PLUSMINUS;
				r.left += OFFSET_PLUSMINUS;
				point.x += OFFSET_PLUSMINUS;

				if (g_App.m_pPrefs->ShowFullFileStatusIcons())
				{
				//	Draw the status icon
					switch (pPartFile->GetPartFileStatusID())
					{
						case PS_COMPLETING:
							iIcon = DL_ICON_STATUS_COMPLETING;
							break;
						case PS_COMPLETE:
							iIcon = DL_ICON_STATUS_COMPLETE;
							break;
						case PS_DOWNLOADING:
							iIcon = DL_ICON_STATUS_DOWNLOADING;
							break;
						case PS_WAITINGFORSOURCE:
							iIcon = DL_ICON_STATUS_WAITING;
							break;
						case PS_WAITINGFORHASH:
							iIcon = DL_ICON_STATUS_WAITINGHASH;
							break;
						case PS_HASHING:
							iIcon = DL_ICON_STATUS_HASHING;
							break;
						case PS_PAUSED:
							iIcon = DL_ICON_STATUS_PAUSED;
							break;
						case PS_STALLED:
							iIcon = DL_ICON_STATUS_STALLED;
							break;
						case PS_STOPPED:
							iIcon = DL_ICON_STATUS_STOPPED;
							break;
						case PS_ERROR:
						default:
							iIcon = DL_ICON_STATUS_ERRONEOUS;
							break;
					}
					if (!bMeasuring)
						m_imageList.Draw(dc, iIcon, point, ILD_NORMAL);
					else
						iWidth += OFFSET_STATUSICON;
					r.left += OFFSET_STATUSICON;

				//	Show that a file is A4AF auto
					if (pPartFile == g_App.m_pDownloadQueue->GetA4AFAutoFile() && !bMeasuring)
						m_imageList.Draw(dc, DL_ICON_A4AFAUTO, point, ILD_TRANSPARENT);
				}

			//	File Type
				if (g_App.m_pPrefs->ShowFileTypeIcon())
				{
					if (!bMeasuring)
					{
						int		iImage = g_App.GetFileTypeSystemImageIdx(pPartFile->GetFileName());

						if (g_App.GetSystemImageList() != NULL)
							::ImageList_Draw(g_App.GetSystemImageList(), iImage, dc->GetSafeHdc(), r.left, r.top+1, ILD_TRANSPARENT);
						r.left += OFFSET_FILETYPEICON;
					}
					else
						iWidth += OFFSET_FILETYPEICON;
				}

			//	Comments/ratings
				if (g_App.m_pPrefs->ShowRatingIcons())
				{
					if ((pPartFile->HasComment() || pPartFile->HasRating()) && !bMeasuring)
					{
						iIcon = DL_ICON_RATING_NO;

						if (pPartFile->HasRating())
						{
							switch (pPartFile->GetRating())
							{
								case PF_RATING_NONE:
								default:
									iIcon = DL_ICON_RATING_NO;
									break;
								case PF_RATING_FAKE:
									iIcon = DL_ICON_RATING_FAKE;
									break;
								case PF_RATING_POOR:
									iIcon = DL_ICON_RATING_POOR;
									break;
								case PF_RATING_GOOD:
									iIcon = DL_ICON_RATING_GOOD;
									break;
								case PF_RATING_FAIR:
									iIcon = DL_ICON_RATING_FAIR;
									break;
								case PF_RATING_EXCELLENT:
									iIcon = DL_ICON_RATING_EXCELLENT;
									break;
							}
						}
						m_imageList.Draw(dc, iIcon, CPoint(r.left - 4, r.top + 2), ILD_NORMAL);
					}
					r.left += OFFSET_RATINGICON;
					iWidth += OFFSET_RATINGICON;
				}

			//	Finally, the file title. First set the color
				COLORREF	crOldTxtColor;
				COLORREF	cr = (pPartFile->IsFakesDotRar() ? g_App.m_pPrefs->GetFakeListDownloadColor() : CCat::GetCatColorByID(pPartFile->GetCatID()));
				bool		bRestoreColor = true;	// Not restoring the color _may_ make the rest of the
													//	row the same color. Don't count on it.
				cr = (cr > 0) ? cr : (::GetSysColor(COLOR_WINDOWTEXT));

				if ((eFileStatus == PS_PAUSED) || (eFileStatus == PS_STOPPED))
				{
					if (g_App.m_pPrefs->ShowPausedGray())
					{
						cr = ::GetSysColor(COLOR_GRAYTEXT);
						bRestoreColor = false;
					}
					else
					{
					//	TODO: This might be worth pulling out into its own function (as I see others have already done)
						uint32	dwR, dwG, dwB, dwGray = 0x60;
						const uint32	dwGrayBuffer = 0x40, dwGrayLimit = 0xFF - dwGrayBuffer;

					//	"gray" the color
						dwR = GetRValue(cr);
						dwB = GetBValue(cr);
						dwG = GetGValue(cr);

					//	Make sure we don't wash the color out TOO much
						if (dwR + dwGray >= dwGrayLimit && dwB + dwGray >= dwGrayLimit && dwG + dwGray >= dwGrayLimit)
							dwGray -= dwGrayBuffer;
						if ((dwR += dwGray)> 0xFF)
							dwR = 0xFF;
						if ((dwB += dwGray) > 0xFF)
							dwB = 0xFF;
						if ((dwG += dwGray) > 0xFF)
							dwG = 0xFF;

						cr = RGB(dwR, dwG, dwB);
					}
				}

				crOldTxtColor = dc->SetTextColor(cr);

			//	Draw the file name
				buffer = pPartFile->GetFileName();
				dc->DrawText(buffer, &r, iCalcFlag);
				if (bMeasuring)
					iWidth += r.right - r.left + 1;

			//	Restore the original color
				if (bRestoreColor)
					dc->SetTextColor(crOldTxtColor);
				break;
			}
			case DLCOL_SIZE:
				buffer = CastItoXBytes(pPartFile->GetFileSize());
				dc->DrawText(buffer, &r, DT_RIGHT | iCalcFlag);
				if (bMeasuring)
					iWidth += r.right - r.left + 1;
				break;

			case DLCOL_TRANSFERRED:
				buffer = CastItoXBytes(pPartFile->GetTransferred());
				dc->DrawText(buffer, &r, DT_RIGHT | iCalcFlag);
				if (bMeasuring)
					iWidth += r.right - r.left + 1;
				break;

			case DLCOL_COMPLETED:
				buffer = CastItoXBytes(pPartFile->GetCompletedSize());
				dc->DrawText(buffer, &r, DT_RIGHT | iCalcFlag);
				if (bMeasuring)
					iWidth += r.right - r.left + 1;
				break;

			case DLCOL_SPEED:
			{
				EnumPartFileStatuses	eFileStatus = pPartFile->GetStatus();

				if ((eFileStatus != PS_COMPLETE) && (eFileStatus != PS_COMPLETING))
				{
					uint32		dwAvgDataRate, dwDataRate = pPartFile->GetDataRate();

					if (dwDataRate > 10)
						buffer.Format(_T("%.1f"), dwDataRate / 1024.0f);
					if (g_App.m_pPrefs->GetShowAverageDataRate())
					{
						dwAvgDataRate = pPartFile->GetAvgDataRate(true);
						if (dwAvgDataRate > 10)
							buffer.AppendFormat(_T(" (%.2f)"), dwAvgDataRate / 1024.0f);
					}
				}
				else if (g_App.m_pPrefs->GetShowAverageDataRate())
					buffer.Format(_T("[%.2f]"), pPartFile->GetAvgDataRate(false)/1024.0f);

				dc->DrawText(buffer, &r, DT_RIGHT | iCalcFlag);
				if (bMeasuring && buffer.GetLength() != 0)
					iWidth += r.right - r.left + 1;
				break;
			}
			case DLCOL_PROGRESS:
			{
				if (!bMeasuring)
				{
					r.bottom--;
					r.top++;

					int		iBarWidth = r.right - r.left;
					int		iBarHeight = r.bottom - r.top;

					if (pFileItem->GetBitmap() == (HBITMAP)NULL)
						VERIFY(pFileItem->GetBitmap().CreateBitmap(1, 1, 1, 8, NULL));

					CDC			cdcStatus;
					HGDIOBJ		hOldBitmap;

					cdcStatus.CreateCompatibleDC(dc);

					int		cx = pFileItem->GetBitmap().GetBitmapDimension().cx;
					DWORD	dwTicks = GetTickCount();

					if (pFileItem->GetUpdateTimer() + DLC_BARUPDATE < dwTicks || cx !=  iBarWidth || pFileItem->GetUpdateTimer() != 0)
					{
						pFileItem->GetBitmap().DeleteObject();
						pFileItem->GetBitmap().CreateCompatibleBitmap(dc,  iBarWidth, iBarHeight);
						pFileItem->GetBitmap().SetBitmapDimension(iBarWidth,  iBarHeight);
						hOldBitmap = cdcStatus.SelectObject(pFileItem->GetBitmap());

						RECT rec_status;
						rec_status.left = 0;
						rec_status.top = 0;
						rec_status.bottom = iBarHeight;
						rec_status.right = iBarWidth;
						pPartFile->DrawStatusBar(&cdcStatus,  &rec_status, g_App.m_pPrefs->UseFlatBar());

						pFileItem->SetUpdateTimer(dwTicks + (rand() % 128));
					}
					else
						hOldBitmap = cdcStatus.SelectObject(pFileItem->GetBitmap());

					dc->BitBlt(r.left, r.top, iBarWidth, iBarHeight, &cdcStatus, 0, 0, SRCCOPY);
					cdcStatus.SelectObject(hOldBitmap);

					if (g_App.m_pPrefs->GetUseDwlPercentage())
					{
					//	BEGIN Display percent in progress bar
						COLORREF	crOld = dc->SetTextColor(RGB(255,255,255));
						int			iOldBkMode = dc->SetBkMode(TRANSPARENT);
						double		dblPercentCompleted = floor(pPartFile->GetPercentCompleted() * 10.0) / 10.0;
						int			iOldLeft = r.left;

						buffer.Format( (floor(dblPercentCompleted) == dblPercentCompleted) ?
							_T("%.f%%") : _T("%.1f%%"), dblPercentCompleted);
						r.left += ((iBarWidth - dc->GetTextExtent(buffer).cx) / 2);

						EnumPartFileStatuses	eFileStatus = pPartFile->GetStatus();

						if ((eFileStatus != PS_COMPLETE) && (eFileStatus != PS_COMPLETING))
							r.top += (g_App.m_pPrefs->UseFlatBar()) ? 3 : 2; // Just a little down

						dc->DrawText(buffer, &r, DLC_DT_TEXT);
						r.left = iOldLeft;
						dc->SetBkMode(iOldBkMode);
						dc->SetTextColor(crOld);
					}
				}
			//	Since the progress bar drawing is conformant, we'll just hardcode a reasonable width
				else
					iWidth += 300;
				break;
			}
			case DLCOL_NUMSOURCES:
			{
				uint32		dwSourcesCount = static_cast<uint32>(pPartFile->GetSourceCount());
				uint32		dwNotCurrentSourcesCount = pPartFile->GetNotCurrentSourcesCount();

				if (dwNotCurrentSourcesCount != 0)
					buffer.Format(_T("%u/%u"), dwSourcesCount - dwNotCurrentSourcesCount, dwSourcesCount);
				else
					buffer.Format(_T("%u"), dwSourcesCount);

				uint32		dwA4AFSourcesCount = static_cast<uint32>(pPartFile->GetSrcA4AFCount());

				if (g_App.m_pPrefs->IsA4AFCountEnabled() && (dwA4AFSourcesCount != 0))
				{
					buffer.AppendFormat(_T("+%u"), dwA4AFSourcesCount);
				}
				buffer.AppendFormat(_T(" (%u)"), pPartFile->GetTransferringSrcCount());

				dc->DrawText(buffer, &r, DT_RIGHT | iCalcFlag);
				if (bMeasuring)
					iWidth += r.right - r.left + 1;
				break;
			}
			case DLCOL_PRIORITY:
			{
				UINT		dwResStrId;

				if (pPartFile->IsAutoPrioritized())
				{
					pPartFile->UpdateDownloadAutoPriority();

					switch (pPartFile->GetPriority())
					{
						default:
						case PR_LOW:
							dwResStrId = IDS_PRIOAUTOLOW;
							break;
						case PR_NORMAL:
							dwResStrId = IDS_PRIOAUTONORMAL;
							break;
						case PR_HIGH:
							dwResStrId = IDS_PRIOAUTOHIGH;
							break;
					}
				}
				else
				{
					switch (pPartFile->GetPriority())
					{
						default:
						case PR_LOW:
							dwResStrId = IDS_PRIOLOW;
							break;
						case PR_NORMAL:
							dwResStrId = IDS_PRIONORMAL;
							break;
						case PR_HIGH:
							dwResStrId = IDS_PRIOHIGH;
							break;
					}
				}
				GetResString(&buffer, dwResStrId);
				dc->DrawText(buffer, &r, iCalcFlag);
				if (bMeasuring)
					iWidth += r.right - r.left + 1;
				break;
			}
			case DLCOL_STATUS:
				buffer = pPartFile->GetPartFileStatus();
				dc->DrawText(buffer, &r, iCalcFlag);
				if (bMeasuring)
					iWidth += r.right - r.left + 1;
				break;

			case DLCOL_REMAINING:
			{
				uint64	qwRemaining = pPartFile->GetFileSize() - pPartFile->GetCompletedSize();

				buffer = CastItoXBytes(qwRemaining);

				EnumPartFileStatuses	eFileStatus = pPartFile->GetStatus();

				if ((eFileStatus != PS_COMPLETE) && (eFileStatus != PS_COMPLETING))
				{
					double		dRemainingPercent = 100.1 - pPartFile->GetPercentCompleted();

					if (dRemainingPercent > 100.0)
						dRemainingPercent = 100.0;
					else if (qwRemaining == 0)
						dRemainingPercent = 0.0;

					dRemainingPercent = floor(dRemainingPercent * 10.0) / 10.0;

					buffer.AppendFormat( (floor(dRemainingPercent) == dRemainingPercent) ?
						_T(" [%.f%%]") : _T(" [%.1f%%]"), dRemainingPercent );
				}
				dc->DrawText(buffer, &r, iCalcFlag);
				if (bMeasuring)
					iWidth += r.right - r.left + 1;
				break;
			}
			case DLCOL_REMAININGTIME:
			{
				sint32		restTime;
				EnumPartFileStatuses	eFileStatus = pPartFile->GetStatus();

				if ((eFileStatus != PS_COMPLETE) && (eFileStatus != PS_COMPLETING))
				{
					restTime = pPartFile->GetTimeRemaining();
					buffer.Format(_T("%s"), CastSecondsToHM(restTime));

					if (g_App.m_pPrefs->GetShowAverageDataRate())
					{
						restTime = pPartFile->GetTimeRemaining(true);
						buffer.AppendFormat(_T(" (%s)"), CastSecondsToHM(restTime));
					}
				}
				else
				{
					restTime = static_cast<sint32>(pPartFile->GetFlushTimeSpan().GetTotalSeconds());
					buffer.Format(_T("[%s / %s]"), CastItoXBytes(pPartFile->GetSessionTransferred()), CastSecondsToHM(restTime));
				}
				dc->DrawText(buffer, &r, iCalcFlag);
				if (bMeasuring)
					iWidth += r.right - r.left + 1;
				break;
			}
			case DLCOL_LASTSEENCOMPLETE:
				if (pPartFile->lastseencomplete == NULL)
				{
					GetResString(&buffer, IDS_NEVER);
					dc->DrawText(buffer, &r, iCalcFlag);
				}
				else
				{
					CTime			ctTime(pPartFile->lastseencomplete);
					SYSTEMTIME		st;

					ctTime.GetAsSystemTime(st);
					COleDateTime    odtTime(st);
					dc->DrawText(odtTime.Format(_T("%c")), &r, iCalcFlag);
				}
				if (bMeasuring)
					iWidth += r.right - r.left + 1;
				break;

			case DLCOL_LASTRECEIVED:
				if (pPartFile->GetTransferred() == 0)
				{
					GetResString(&buffer, IDS_NEVER);
					dc->DrawText(buffer, &r, iCalcFlag);
				}
				else
				{
					CTime			ctTime(pPartFile->GetLastDownTransfer());
					SYSTEMTIME		st;

					ctTime.GetAsSystemTime(st);
					COleDateTime    odtTime(st);
					dc->DrawText(odtTime.Format(_T("%c")), &r, iCalcFlag);
				}
				if (bMeasuring)
					iWidth += r.right - r.left + 1;
				break;

			case DLCOL_CATEGORY:
				if (pPartFile->GetCatID() == 0)
					GetResString(&buffer, IDS_CAT_UNCATEGORIZED);
				else
				{
					CCat	*pCat = CCat::GetCatByID(pPartFile->GetCatID());

					if (pCat != NULL)
						buffer = pCat->GetTitle();
					else
					{
						pPartFile->SetCatID(CAT_NONE);
						GetResString(&buffer, IDS_CAT_UNCATEGORIZED);
					}
				}
				dc->DrawText(buffer, &r, iCalcFlag);
				if (bMeasuring)
					iWidth += r.right - r.left + 1;
				break;

			case DLCOL_WAITED:
			{
				CTimeSpan	ts = CTime::GetCurrentTime() - pPartFile->GetLastDownTransfer();
				uint32		dwWaitedSecs = static_cast<uint32>(ts.GetTotalSeconds());

				if (dwWaitedSecs >= 15)
					buffer = ::CastSecondsToHM(dwWaitedSecs);
				dc->DrawText(buffer, &r, iCalcFlag);
				if (bMeasuring && !buffer.IsEmpty())
					iWidth += r.right - r.left + 1;
				break;
			}
			case DLCOL_AVGSPEED:
			{
				EnumPartFileStatuses	eFileStatus = pPartFile->GetStatus();

				if ((eFileStatus != PS_COMPLETE) && (eFileStatus != PS_COMPLETING))
				{
					uint32		iDataRate = pPartFile->GetAvgDataRate(true);

					if (iDataRate > 10)
						buffer.Format(_T("%.2f"), iDataRate / 1024.0f);
				}
				else
					buffer.Format(_T("%.2f"), pPartFile->GetAvgDataRate(false) / 1024.0f);
				dc->DrawText(buffer, &r, DT_RIGHT | iCalcFlag);
				if (bMeasuring && buffer.GetLength() != 0)
					iWidth += r.right - r.left + 1;
				break;
			}
			case DLCOL_AVGREMTIME:
			{
				sint32		iRemainingTime;
				EnumPartFileStatuses	eFileStatus = pPartFile->GetStatus();

				if ((eFileStatus != PS_COMPLETE) && (eFileStatus != PS_COMPLETING))
				{
					iRemainingTime = pPartFile->GetTimeRemaining(true);
					buffer.AppendFormat(_T("%s"), CastSecondsToHM(iRemainingTime));
				}
				dc->DrawText(buffer, &r, DT_RIGHT | iCalcFlag);
				if (bMeasuring)
					iWidth += r.right - r.left + 1;
				break;
			}
			case DLCOL_ETA:
			{
				sint32		iRemainingTime;
				EnumPartFileStatuses	eFileStatus = pPartFile->GetStatus();

				if ((eFileStatus != PS_COMPLETE) && (eFileStatus != PS_COMPLETING))
				{
					iRemainingTime = pPartFile->GetTimeRemaining();

					if (iRemainingTime != -1)
					{
						CTime			ctTime;
						SYSTEMTIME		st;

						ctTime = CTime::GetCurrentTime();
						ctTime += CTimeSpan(iRemainingTime);
						ctTime.GetAsSystemTime(st);

						COleDateTime	odtTime(st);

						dc->DrawText(odtTime.Format(_T("%c")), &r, DT_RIGHT | iCalcFlag);
						if (bMeasuring)
							iWidth += r.right - r.left + 1;
					}
				}
				else
					dc->DrawText(buffer, &r, iCalcFlag);
				break;
			}
			case DLCOL_AVGETA:
			{
				sint32		iRemainingTime;
				EnumPartFileStatuses	eFileStatus = pPartFile->GetStatus();

				if ((eFileStatus != PS_COMPLETE) && (eFileStatus != PS_COMPLETING))
				{
					iRemainingTime = pPartFile->GetTimeRemaining(true);

					if (iRemainingTime != -1)
					{
						CTime			ctTime;
						SYSTEMTIME		st;

						ctTime = CTime::GetCurrentTime();
						ctTime += CTimeSpan(iRemainingTime);
						ctTime.GetAsSystemTime(st);

						COleDateTime	odtTime(st);

						dc->DrawText(odtTime.Format(_T("%c")), &r, DT_RIGHT | iCalcFlag);
						if (bMeasuring)
							iWidth += r.right - r.left + 1;
					}
				}
				else
					dc->DrawText(buffer, &r, iCalcFlag);
				break;
			}
			default:
				break;
		}
		if (bMeasuring)
		{
		//	Pin the column widths at some reasonable value
			if (iWidth < 40 && iWidth != 0)
				iWidth = 40;
			if (iWidth > m_iColumnMaxWidths[m_iMeasuringColumn])
				m_iColumnMaxWidths[m_iMeasuringColumn] = iWidth;
		}
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	DrawSourceItem() draws column 'nColumn' of the list row for source 'pSourceItem' in the
//		rectangle 'lpRect' in device context 'dc'. If 'm_iMeasuringColumn' is nonzero then
//		the width of drawing is measured and 'm_iColumnMaxWidths' is updated for that column
//		but nothing is drawn.
void CDownloadListCtrl::DrawSourceItem(CDC *dc, int nColumn, LPRECT lpRect, CSourceDLItem *pSourceItem)
{
	EMULE_TRY

#ifdef OLD_SOCKETS_ENABLED
	if (IsColumnHidden(nColumn))
		return;

	int		iPadding = dc->GetTextExtent(_T(" "), 1).cx * 2;
	int		iWidth = (3 * iPadding)/2;
	bool	bMeasuring = (m_iMeasuringColumn >= 0);
	UINT	iCalcFlag = (bMeasuring) ? ((DLC_DT_TEXT | DT_CALCRECT) & ~DT_END_ELLIPSIS) : DLC_DT_TEXT;

	if (lpRect->left < lpRect->right)
	{
		CString				buffer;
		CUpDownClient	   *pSource = pSourceItem->GetSource();
		CString				sDownloadState;
		try
		{
			sDownloadState = pSource->GetDownloadStateAsString();
		}
		catch(...)
		{
		}
		EnumDLQState	eDownloadState = pSource->GetDownloadState();
		CString			sClientName;

		try
		{
			sClientName = pSource->GetUserName();
		}
		catch(...)
		{
		}
		uint32				nTransferredDown = pSource->GetTransferredDown();
		bool				bCredits = pSource->m_pCredits != NULL && pSource->m_pCredits != (CClientCredits*)-1;
		bool				bIsA4AF = pSourceItem->IsAskedForAnotherFile();
		RECT				r = *lpRect;

 		CString		status;

		switch (nColumn)
		{
			case DLCOL_FILENAME:
			{
				RECT			r2 = r;

				r2.left += OFFSET_PLUSMINUS;
				iWidth += OFFSET_PLUSMINUS;
				POINT		point = {r2.left, r2.top + 1};

				if (!bMeasuring)
				{
					if (!bIsA4AF)
					{
						switch (eDownloadState)
						{
							case DS_CONNECTING:
							case DS_CONNECTED:
							case DS_WAITCALLBACK:
							case DS_WAIT_FOR_FILE_REQUEST:
								m_imageList.Draw(dc, DL_ICON_DCS3, point, ILD_NORMAL);
								break;
							case DS_ONQUEUE:
							case DS_LOWID_ON_OTHER_SERVER:
								m_imageList.Draw(dc, DL_ICON_DCS2, point, ILD_NORMAL);
								break;
							case DS_DOWNLOADING:
							case DS_REQHASHSET:
								m_imageList.Draw(dc, DL_ICON_DCS1, point, ILD_NORMAL);
								break;
							case DS_NONEEDEDPARTS:
							case DS_LOWTOLOWID:
								m_imageList.Draw(dc, DL_ICON_DCS4, point, ILD_NORMAL);
								break;
							default:
								m_imageList.Draw(dc, DL_ICON_DCS5, point, ILD_NORMAL);
						}
					}
					else
					{
						m_imageList.Draw(dc, DL_ICON_DCS4, point, ILD_NORMAL);
						if (g_App.m_pPrefs->IsA4AFStringEnabled())
						{
							try
							{
								status = pSource->m_pReqPartFile->GetFileName();
							}
							catch(...)
							{
							}
						}
						else
							GetResString(&status, IDS_ASKED4ANOTHERFILE);
					}
				}
				r2.left += 20;
				point.x += 20;
				iWidth += 20;

				if (!bMeasuring)
				{
					int		iImgLstIdx = CLIENT_IMGLST_PLAIN;

				//	Select corresponding image list depending on client properties
					if (pSource->IsBanned())
						iImgLstIdx = CLIENT_IMGLST_BANNED;
					else if (pSource->IsFriend())
						iImgLstIdx = CLIENT_IMGLST_FRIEND;
					else if (pSource->GetRemoteBaseModifier() >= 1.1)
						iImgLstIdx = CLIENT_IMGLST_CREDITDOWN;

				//	Display Client icon
					g_App.m_pMDlg->m_clientImgLists[iImgLstIdx].Draw(dc, pSource->GetClientIconIndex(), point, ILD_NORMAL);

					if (g_App.m_pPrefs->ShowRatingIcons() && !g_App.m_pIP2Country->ShowCountryFlag())
					{
						r2.left += OFFSET_RATINGICON;
						iWidth += OFFSET_RATINGICON;
					}
				}

				CString strBuffer1;
				if (sClientName.IsEmpty())
					strBuffer1.Format(_T("[%s]"), GetResString(IDS_UNKNOWN));
				else
					strBuffer1 = sClientName;
				if (!status.IsEmpty())
					strBuffer1.AppendFormat(_T(" (%s)"), status);

			//	Different color for A4AF sources
				COLORREF crOldTxtColor;

				if (bIsA4AF)
					crOldTxtColor = dc->SetTextColor(GetSysColor(COLOR_GRAYTEXT));

				r2.left += 20;
				iWidth += 20;
				if (g_App.m_pIP2Country->ShowCountryFlag())
				{
					point.x += 20;
					point.y += 2;
					g_App.m_pIP2Country->GetFlagImageList()->Draw(dc, pSource->GetCountryIndex(), point, ILD_NORMAL);
					r2.left += 22;
					iWidth += 22;
				}
				dc->DrawText(strBuffer1, &r2, iCalcFlag);

				if (bMeasuring)
					iWidth += r2.right - r2.left + 1;
				if (bIsA4AF)
					dc->SetTextColor(crOldTxtColor);
				break;
			}
			case DLCOL_SIZE:
				break;

			case DLCOL_TRANSFERRED:
				if (!bIsA4AF && nTransferredDown && !g_App.m_pPrefs->IsTransferredOnCompleted())
				{
					buffer = CastItoXBytes(nTransferredDown);
					dc->DrawText(buffer, &r, iCalcFlag | DT_RIGHT);
					if (bMeasuring)
						iWidth += r.right - r.left + 1;
				}
				break;

			case DLCOL_COMPLETED:
				if (!bIsA4AF && nTransferredDown && g_App.m_pPrefs->IsTransferredOnCompleted())
				{
					buffer = CastItoXBytes(nTransferredDown);
					dc->DrawText(buffer, &r, iCalcFlag | DT_RIGHT);
					if (bMeasuring)
						iWidth += r.right - r.left + 1;
				}
				break;

			case DLCOL_SPEED:
				if (!bIsA4AF)
				{
					uint32	dwDownloadDataRate = pSource->GetDownloadDataRate();

					if (dwDownloadDataRate != 0)
						buffer.Format(_T("%.1f"), dwDownloadDataRate / 1024.0);
					dc->DrawText(buffer, &r, iCalcFlag | DT_RIGHT);
					if (bMeasuring && !buffer.IsEmpty())
						iWidth += r.right - r.left + 1;
				}
				break;

			case DLCOL_PROGRESS:
			{
				if (!bMeasuring)
				{
					lpRect->bottom--;
					lpRect->top++;

					int iWidth = lpRect->right - lpRect->left;
					int iHeight = lpRect->bottom - lpRect->top;
					if (pSourceItem->GetBitmap() == (HBITMAP)NULL)
						VERIFY(pSourceItem->GetBitmap().CreateBitmap(1, 1, 1, 8, NULL));
					CDC cdcStatus;
					HGDIOBJ hOldBitmap;
					cdcStatus.CreateCompatibleDC(dc);
					int cx = pSourceItem->GetBitmap().GetBitmapDimension().cx;
					DWORD dwTicks = GetTickCount();
					if (pSourceItem->GetUpdateTimer() + DLC_BARUPDATE < dwTicks || cx !=  iWidth  || !pSourceItem->GetUpdateTimer())
					{
						pSourceItem->GetBitmap().DeleteObject();
						pSourceItem->GetBitmap().CreateCompatibleBitmap(dc,  iWidth, iHeight);
						pSourceItem->GetBitmap().SetBitmapDimension(iWidth,  iHeight);
						hOldBitmap = cdcStatus.SelectObject(pSourceItem->GetBitmap());

						RECT rec_status;
						rec_status.left = 0;
						rec_status.top = 0;
						rec_status.bottom = iHeight;
						rec_status.right = iWidth;
						try
						{
							pSource->DrawStatusBar(&cdcStatus,  &rec_status,(pSourceItem->IsAskedForAnotherFile()), g_App.m_pPrefs->UseFlatBar());
						}
						catch(...)
						{
						//	in case client has been deleted meanwhile
						}

						pSourceItem->SetUpdateTimer(dwTicks + (rand() % 128));
					} else
						hOldBitmap = cdcStatus.SelectObject(pSourceItem->GetBitmap());

					dc->BitBlt(lpRect->left, lpRect->top, iWidth, iHeight,  &cdcStatus, 0, 0, SRCCOPY);
					cdcStatus.SelectObject(hOldBitmap);

					lpRect->bottom++;
					lpRect->top--;
				}
				else
					iWidth = 300;
				break;
			}
			case DLCOL_NUMSOURCES:
				buffer = pSource->GetFullSoftVersionString();
				dc->DrawText(buffer, &r, iCalcFlag);
				if (bMeasuring)
					iWidth += r.right - r.left + 1;
				break;

			case DLCOL_PRIORITY:
			{
				bool		bIsColorChanged = false;
				uint32		dwRemoteQueueRank = static_cast<uint32>(pSource->GetRemoteQueueRank());

				if ( pSource->IsEmuleClient() && !pSourceItem->IsAskedForAnotherFile() &&
					(eDownloadState == DS_ONQUEUE || eDownloadState == DS_LOWID_ON_OTHER_SERVER) )
				{
					if (dwRemoteQueueRank == 0)
					{
						GetResString(&buffer, IDS_QUEUEFULL);
					}
					else
					{
						int		iDifference = pSource->GetDifference();

						if (iDifference == static_cast<int>(dwRemoteQueueRank))
						{
						//	Initial QR -- just one QR was received
							buffer.Format(_T("QR: %u"), dwRemoteQueueRank);
						}
						else if (iDifference == 0)
						{
							dc->SetTextColor((COLORREF)RGB(5,65,195));
							bIsColorChanged = true;
							buffer.Format(_T("QR: %u"), dwRemoteQueueRank);
						}
						else
						{
							dc->SetTextColor((COLORREF)((iDifference < 0) ? RGB(10,160,70) : RGB(190,60,60)));
							bIsColorChanged = true;
							buffer.Format(_T("QR: %u (%+i)"), dwRemoteQueueRank, iDifference);
						}
					}
				}
				dc->DrawText(buffer, &r, iCalcFlag);
				if (bIsColorChanged)
					dc->SetTextColor(m_crWindowText);
				if (bMeasuring && !buffer.IsEmpty())
					iWidth += r.right - r.left + 1;
				break;
			}
			case DLCOL_STATUS:
			{
				r.left += 6;

				dc->DrawText(sDownloadState, &r, iCalcFlag);
				if (bMeasuring && !sDownloadState.IsEmpty())
					iWidth += r.right - r.left + 1 + 6;
				break;
			}
			case DLCOL_REMAINING:
			{
				uint32		dwCurrPart, dwRemainingSize = pSource->GetRemainingSizeForCurrentPart(&dwCurrPart);

				if (dwRemainingSize != 0 && pSourceItem->GetParentFile() == pSource->m_pReqPartFile)
				{
					buffer.Format(_T("%u: %s"), dwCurrPart, CastItoXBytes(dwRemainingSize));
				}
				dc->DrawText(buffer, &r, iCalcFlag | DT_LEFT);
				if (bMeasuring && !buffer.IsEmpty())
					iWidth += r.right - r.left + 1;
				break;
			}
			case DLCOL_REMAININGTIME:
			{
				uint32		dwSeconds = pSource->GetRemainingTimeForCurrentPart();

				if (dwSeconds != 0 && pSourceItem->GetParentFile() == pSource->m_pReqPartFile)
				{
					buffer = ::CastSecondsToHM(dwSeconds);
				}
				dc->DrawText(buffer, &r, iCalcFlag | DT_LEFT);
				if (bMeasuring && !buffer.IsEmpty())
					iWidth += r.right - r.left + 1;
				break;
			}
			case DLCOL_ULDLRATIO:
				if (bCredits)
					buffer.Format(_T("%0.1f"), pSource->GetRemoteBaseModifier());
				dc->DrawText(buffer, &r, iCalcFlag | DT_RIGHT);
				if (bMeasuring && !buffer.IsEmpty())
					iWidth += r.right - r.left + 1;
				break;

			case DLCOL_QLRATING:
				if (bCredits)
					buffer.Format(_T("%u"), pSource->GetRemoteRatio());
				dc->DrawText(buffer, &r, iCalcFlag | DT_RIGHT);
				if (bMeasuring && !buffer.IsEmpty())
					iWidth += r.right - r.left + 1;
				break;

			case DLCOL_LASTSEENCOMPLETE:
			case DLCOL_LASTRECEIVED:
			case DLCOL_CATEGORY:
				break;

			case DLCOL_WAITED:
			{
				if ( ((eDownloadState == DS_ONQUEUE) || (eDownloadState == DS_LOWID_ON_OTHER_SERVER))
					&& pSource->GetRemoteQueueRank() != 0 )
				{
					buffer = ::CastSecondsToHM(pSource->GetDLQueueWaitTime() / 1000);
				}
				else
					buffer = _T("-");

				dc->DrawText(buffer, &r, iCalcFlag | DT_RIGHT);
				if (bMeasuring && !buffer.IsEmpty())
					iWidth += r.right - r.left + 1;
				break;
			}
			case DLCOL_AVGSPEED:
			case DLCOL_AVGREMTIME:
			case DLCOL_ETA:
			case DLCOL_AVGETA:
				break;
		}
		if (bMeasuring)
		{
		//	Pin the column widths at some reasonable value
			if (iWidth < 40 && iWidth != 0)
				iWidth = 40;
			if (iWidth > m_iColumnMaxWidths[m_iMeasuringColumn])
				m_iColumnMaxWidths[m_iMeasuringColumn] = iWidth;
		}
	}
#endif //OLD_SOCKETS_ENABLED
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	EMULE_TRY

	if (!g_App.m_pMDlg->IsRunning() || !lpDrawItemStruct->itemData)
		return;

	CDC				   *odc = CDC::FromHandle(lpDrawItemStruct->hDC);
	CMuleCtrlItem	   *pItem = reinterpret_cast<CMuleCtrlItem*>(lpDrawItemStruct->itemData);
	CPartFileDLItem	   *pFileItem = dynamic_cast<CPartFileDLItem*>(pItem);
	CSourceDLItem	   *pSourceItem = dynamic_cast<CSourceDLItem*>(pItem);
	BOOL				bCtrlFocused = ((GetFocus() == this) || (GetStyle() & LVS_SHOWSELALWAYS));
	COLORREF			crBk, crWinBk;

	crWinBk = crBk = GetBkColor();
	if (pFileItem != NULL && (lpDrawItemStruct->itemState & ODS_SELECTED))
		crBk = (bCtrlFocused) ? m_crHighlight : m_crNoHighlight;

	CMemDC			dc(odc, &lpDrawItemStruct->rcItem, crWinBk, crBk);
	CFont			*pOldFont = dc->SelectObject(GetFont());
	COLORREF		crOldTextColor = dc->SetTextColor(m_crWindowText);
	CHeaderCtrl		*pHeaderCtrl = GetHeaderCtrl();

	RECT				cur_rec = lpDrawItemStruct->rcItem;

//	If we're just measuring...
	if (m_iMeasuringColumn >= 0)
	{
		if (pFileItem != NULL)
		{
		//	MOREVIT - This is a tough one. We REALLY ought to figure some way
		//		to separate the measuring and drawing routines but as they
		//		require largely the same code and we want to avoid duplication
		//		at all costs, there's no easy way ATM.
			DrawFileItem(dc, m_iMeasuringColumn, &cur_rec, pFileItem);
		}
		else if (pSourceItem != NULL)
		{
			DrawSourceItem(dc, m_iMeasuringColumn, &cur_rec, pSourceItem);
		}
		return;
	}

	bool		bNotLast = (lpDrawItemStruct->itemID + 1) != static_cast<ULONG>(GetItemCount());
	bool		bNotFirst = lpDrawItemStruct->itemID != 0;
	int			tree_start=0;
	int			tree_end=0;

	int			iOffset = dc->GetTextExtent(_T(" "), 1).cx * 2;
	int			iNumColumns = pHeaderCtrl->GetItemCount();

	cur_rec.right = cur_rec.left - iOffset;
	cur_rec.left += iOffset/2;

	if (pFileItem != NULL)
	{
		for (int iCurrent = 0; iCurrent < iNumColumns; iCurrent++)
		{
			int		iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
			int		cx = CListCtrl::GetColumnWidth(iColumn);

			if (iColumn == DLCOL_PROGRESS)
			{
				int		iNextLeft = cur_rec.left + cx;

			//	Set up tree vars
				cur_rec.left = cur_rec.right + iOffset;
				cur_rec.right = cur_rec.left + min(8, cx);
				tree_start = cur_rec.left + 1;
				tree_end = cur_rec.right;

			//	Normal column stuff
				cur_rec.left = cur_rec.right + 1;
				cur_rec.right = tree_start + cx - iOffset;
				DrawFileItem(dc, DLCOL_PROGRESS, &cur_rec, pFileItem);
				cur_rec.left = iNextLeft;
			}
			else
			{
				cur_rec.right += cx;
				DrawFileItem(dc, iColumn, &cur_rec, pFileItem);
				cur_rec.left += cx;
			}
		}
	}
	else if (pSourceItem != NULL)
	{
		for (int iCurrent = 0; iCurrent < iNumColumns; iCurrent++)
		{
			int		iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
			int		cx = CListCtrl::GetColumnWidth(iColumn);

			if (iColumn == DLCOL_PROGRESS)
			{
				int		iNextLeft = cur_rec.left + cx;

			//	Set up tree vars
				cur_rec.left = cur_rec.right + iOffset;
				cur_rec.right = cur_rec.left + min(8, cx);
				tree_start = cur_rec.left + 1;
				tree_end = cur_rec.right;
			//	Normal column stuff
				cur_rec.left = cur_rec.right + 1;
				cur_rec.right = tree_start + cx - iOffset;
				DrawSourceItem(dc, DLCOL_PROGRESS, &cur_rec, pSourceItem);
				cur_rec.left = iNextLeft;
			}
			else
			{
			//	Space of two columns can be used for source name
				if (iColumn == DLCOL_FILENAME)
				{
					int		iNextColumn = pHeaderCtrl->OrderToIndex(iCurrent + 1);

					if (iNextColumn == DLCOL_SIZE)
					{
						cx += CListCtrl::GetColumnWidth(iNextColumn);
						iCurrent++;
					}
				}

				cur_rec.right += cx;
				DrawSourceItem(dc, iColumn, &cur_rec, pSourceItem);
				cur_rec.left += cx;
			}
		}
	}

//	Draw rectangle around selected item(s)
	if ((lpDrawItemStruct->itemState & ODS_SELECTED) && (pFileItem != NULL))
	{
		RECT	rOutline = lpDrawItemStruct->rcItem;
		CBrush	FrmBrush((bCtrlFocused) ? m_crFocusLine : m_crNoFocusLine);

		rOutline.left++;
		rOutline.right--;

	//	This code tries to draw correct cursor frame around several selected items;
	//	This method works incorrectly when a user selected files and later deselected something...
		if (bNotFirst && (GetItemState(lpDrawItemStruct->itemID - 1, LVIS_SELECTED)))
		{
			CMuleCtrlItem	   *pPrevItem = ListGetItemAt(lpDrawItemStruct->itemID - 1);

			if (typeid(*pPrevItem) == typeid(CPartFileDLItem))
				rOutline.top--;
		}
		if (bNotLast && (GetItemState(lpDrawItemStruct->itemID + 1, LVIS_SELECTED)))
		{
			CMuleCtrlItem	   *pNextItem = ListGetItemAt(lpDrawItemStruct->itemID + 1);

			if (typeid(*pNextItem) == typeid(CPartFileDLItem))
				rOutline.bottom++;
		}
		dc->FrameRect(&rOutline, &FrmBrush);
	}
//	Draw focus rectangle around non-highlightable items when they have the focus
	else if (((lpDrawItemStruct->itemState & ODS_FOCUS) == ODS_FOCUS) && (GetFocus() == this))
	{
		RECT	focus_rec = lpDrawItemStruct->rcItem;
		CBrush	FrmBrush(m_crNoFocusLine);

		focus_rec.left++;
		focus_rec.right--;
		dc->FrameRect(&focus_rec, &FrmBrush);
	}

//	Draw tree last so it draws over selected and focus (looks better)
	if (tree_start < tree_end)
	{
	//	Set new bounds
		RECT tree_rect;
		tree_rect.top    = lpDrawItemStruct->rcItem.top;
		tree_rect.bottom = lpDrawItemStruct->rcItem.bottom;
		tree_rect.left   = tree_start;
		tree_rect.right  = tree_end;
		dc->SetBoundsRect(&tree_rect, DCB_DISABLE);

	//	Gather some information
		CMuleCtrlItem	   *pItem2 = ListGetItemAt(lpDrawItemStruct->itemID + 1);
		CSourceDLItem	   *pSourceItem2 = dynamic_cast<CSourceDLItem*>(pItem2);

		bool hasNext = bNotLast && pSourceItem2 != NULL;
		bool isOpenRoot = hasNext && pFileItem != NULL;
		bool isChild = pSourceItem != NULL;
	//	Might as well calculate these now
		int treeCenter = tree_start + 3;
		int middle = (cur_rec.top + cur_rec.bottom + 1) / 2;

	//	Set up a new pen for drawing the tree
		CPen pn, *oldpn;
		pn.CreatePen(PS_SOLID, 1, dc->GetTextColor());
		oldpn = dc->SelectObject(&pn);

		if (isChild)
		{
		//	Draw the line to the status bar
			dc->MoveTo(tree_end, middle);
			dc->LineTo(tree_start + 3, middle);

		//	Draw the line to the child node
			if (hasNext)
			{
				dc->MoveTo(treeCenter, middle);
				dc->LineTo(treeCenter, cur_rec.bottom + 1);
			}
		}
		else if (isOpenRoot)
		{
		//	Draw circle
			RECT	circle_rec;
			CBrush	FrmBrush(dc->GetTextColor());
			COLORREF crBk = dc->GetBkColor();
			circle_rec.top    = middle - 2;
			circle_rec.bottom = middle + 3;
			circle_rec.left   = treeCenter - 2;
			circle_rec.right  = treeCenter + 3;
			dc->FrameRect(&circle_rec, &FrmBrush);
			dc->SetPixelV(circle_rec.left,      circle_rec.top,    crBk);
			dc->SetPixelV(circle_rec.right - 1, circle_rec.top,    crBk);
			dc->SetPixelV(circle_rec.left,      circle_rec.bottom - 1, crBk);
			dc->SetPixelV(circle_rec.right - 1, circle_rec.bottom - 1, crBk);
		//	Draw the line to the child node
			if (hasNext)
			{
				dc->MoveTo(treeCenter, middle + 3);
				dc->LineTo(treeCenter, cur_rec.bottom + 1);
			}
		}
	//	Draw the line back up to parent node
		if (bNotFirst && isChild)
		{
			dc->MoveTo(treeCenter, middle);
			dc->LineTo(treeCenter, cur_rec.top - 1);
		}
		dc->SelectObject(oldpn);	//	Put the old pen back
	}

//	Put the original objects back
	if (pOldFont)
		dc->SelectObject(pOldFont);
	if (crOldTextColor)
		dc->SetTextColor(crOldTextColor);
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::HideSources(CPartFile *pPartFile)
{
	EMULE_TRY

	SetRedraw(false);

//	For each item in the list control...
	for (int i = 0; i < GetItemCount();)
	{
		CMuleCtrlItem		*pItem = ListGetItemAt(i);
		CSourceDLItem		*pSourceItem = dynamic_cast<CSourceDLItem*>(pItem);

	//	If it's a source item belonging to 'pPartFile'...
		if (pSourceItem != NULL && pSourceItem->GetParentFile() == pPartFile)
		{
			pSourceItem->GetParentFileItem()->m_bSrcsAreVisible = false;
			pSourceItem->ResetUpdateTimer();
			pSourceItem->GetBitmap().DeleteObject();
			pSourceItem->SetVisibility(false);
			DeleteItem(i);
		}
		else
		{
			i++;
		}
	}
	if (m_bShowSrc)
	{
		m_bShowSrc = false;
	//	For each item in the list...
		for (int i = 0; i < GetItemCount(); i++)
		{
			CMuleCtrlItem	   *pItem = ListGetItemAt(i);

		//	If there's a source item left...
			if (typeid(*pItem) == typeid(CSourceDLItem))
			{
				m_bShowSrc = true;
				break;
			}
		}
	}
	SetRedraw(true);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::OnItemActivate(NMHDR *pNMHDR, LRESULT *pResult)
{
	EMULE_TRY

	LPNMITEMACTIVATE	pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	ExpandCollapseItem(pNMIA->iItem, EXPAND_COLLAPSE);
	*pResult = 0;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::ExpandCollapseItem(int iItem,enum EnumExpandType expand,bool bCollapseSource)
{
	EMULE_TRY

	if (iItem == -1)
		return;

	CMuleCtrlItem	   *pItem = ListGetItemAt(iItem);
	CSourceDLItem	   *pSourceItem = dynamic_cast<CSourceDLItem*>(pItem);
	CPartFileDLItem	   *pFileItem = dynamic_cast<CPartFileDLItem*>(pItem);

	if (bCollapseSource && pSourceItem != NULL && pSourceItem->GetParentFileItem() != NULL)
	{
	//	To collapse/expand files when one of its source is selected
		CPartFileDLItem	   *pParentFileItem = pSourceItem->GetParentFileItem();

		iItem = ListGetFileItemIndex(pParentFileItem);

		if (iItem == -1)
			return;

		pFileItem = pParentFileItem;
	}

	if (pFileItem != NULL)
	{
		CPartFile		*pPartFile = pFileItem->GetFile();

		if (pPartFile == NULL)
			return;

	//	If the source branch is disabled and the parent file isn't completing...
		if (pFileItem->m_bSrcsAreVisible == false && pPartFile->GetStatus() != PS_COMPLETING)
		{
			if (expand == EXPAND_ONLY || expand == EXPAND_COLLAPSE)
			{
				m_bShowSrc = true;

				SetRedraw(false);

				pFileItem->m_bSrcsAreVisible = true;

			//	Get the sources items for this file item
				CPartFileDLItem::SourceItemVector	   *pvecSourceItems = pFileItem->GetSources();

				int		iNumSourceItems = pvecSourceItems->size();
				int		iNumAdded = 0;

				for (int i = 0; i < iNumSourceItems; i++)
				{
					if (!IsSourceFiltered((*pvecSourceItems)[i]))
					{
						iNumAdded++;
						ListInsertSourceItem((*pvecSourceItems)[i], iItem + 1);
					}
				}

				delete pvecSourceItems;

				pFileItem->m_bSrcsAreVisible = iNumAdded > 0;

				SetRedraw(true);
			}
		}
	//	If the file is currently showing sources, has no sources, or is completing...
		else
		{
			if (expand == EXPAND_COLLAPSE || expand == COLLAPSE_ONLY)
			{
				if (GetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED) != (LVIS_SELECTED | LVIS_FOCUSED))
				{
					SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
					SetSelectionMark(iItem);
					EnsureVisible(iItem, FALSE /*bPartialOK*/);
				}
				HideSources(pPartFile);
			}
		}

		UpdateFileItem(pFileItem);
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CDownloadListCtrl::OnWndMsg(UINT iMessage,WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	BOOL		bHandled = TRUE;

	EMULE_TRY

	switch (iMessage)
	{
		case WM_DL_REFRESH:
		{
			static DWORD		dwNextRefreshTime = 0;
			uint32		dwCurrTick = ::GetTickCount();

			if (dwCurrTick > dwNextRefreshTime)
			{
				dwNextRefreshTime = dwCurrTick + 1000;

				CDownloadList::PartFileItemVector	   *pvecDirtyFileItems = g_App.m_pDownloadList->GetDirtyFiles();

				if (pvecDirtyFileItems != NULL)
				{
					int		iNumDirtyFileItems = pvecDirtyFileItems->size();

					for (int i = 0; i < iNumDirtyFileItems; i++)
					{
						UpdateFileItem((*pvecDirtyFileItems)[i]);
					}

					delete pvecDirtyFileItems;
				}

				CDownloadList::SourceItemVector	   *pvecDirtySources = g_App.m_pDownloadList->GetDirtySources();

				if (pvecDirtySources != NULL)
				{
					int		iNumDirtySources = pvecDirtySources->size();

					for (int i = 0; i < iNumDirtySources; i++)
					{
						UpdateSourceItem((*pvecDirtySources)[i]);
					}

					delete pvecDirtySources;
				}
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
void CDownloadListCtrl::OnContextMenu(CWnd *pWnd, CPoint point)
{
	NOPRM(pWnd);
	EMULE_TRY

#ifdef OLD_SOCKETS_ENABLED
	CTitleMenu		menuFile;
	CTitleMenu		menuSourceFilter;
	CMenu			menuAdvanced;
	CMenu			menuPriority;
	CMenu			menuED2K;
	CMenu			menuWeb;

//	If at least one item is selected...
	if (!ListSelectionIsEmpty())
	{
		POSITION			pos = GetFirstSelectedItemPosition();
		CMuleCtrlItem		*pItem = reinterpret_cast<CMuleCtrlItem*>(GetItemData(GetNextSelectedItem(pos)));
		CPartFileDLItem		*pFileItem = dynamic_cast<CPartFileDLItem*>(pItem);
		CPartFile			*pPartFile = NULL;

	//	If the selected item is a file...
		if (pFileItem != NULL)
			pPartFile = pFileItem->GetFile();

		if (pPartFile != NULL)
		{
		//	Switch the info list header to display the selected file.
			g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.SetList(INFOLISTTYPE_FILE, pPartFile);

		//	Create the "File" menu
			menuFile.CreatePopupMenu();
			menuFile.AddMenuTitle(GetResString(IDS_DOWNLOADMENUTITLE));
		//	Create/build the priority sub-menu
			menuPriority.CreateMenu();

			EnumPartFileStatuses	eFileStatus = pPartFile->GetStatus();
			bool		bFileDone = ((eFileStatus == PS_COMPLETE) || (eFileStatus == PS_COMPLETING));
			UINT		uFlagIfDone = MF_STRING | ((bFileDone) ? MF_GRAYED : MF_ENABLED);
			bool		bJustOne = (GetSelectedCount() == 1);

			if (!bFileDone)
			{
				bool		bTmpFlag = (bJustOne && !pPartFile->IsAutoPrioritized());

				menuPriority.AppendMenu( MF_STRING |
					((bTmpFlag && pPartFile->GetPriority() == PR_LOW) ? MF_CHECKED : MF_UNCHECKED),
					MP_PRIOLOW, GetResString(IDS_PRIOLOW) );
				menuPriority.AppendMenu( MF_STRING |
					((bTmpFlag && pPartFile->GetPriority() == PR_NORMAL) ? MF_CHECKED : MF_UNCHECKED),
					MP_PRIONORMAL, GetResString(IDS_PRIONORMAL) );
				menuPriority.AppendMenu( MF_STRING |
					((bTmpFlag && pPartFile->GetPriority() == PR_HIGH) ? MF_CHECKED : MF_UNCHECKED),
					MP_PRIOHIGH, GetResString(IDS_PRIOHIGH) );
				menuPriority.AppendMenu( MF_STRING |
					((bJustOne && pPartFile->IsAutoPrioritized()) ? MF_CHECKED : MF_UNCHECKED),
					MP_PRIOAUTO, GetResString(IDS_PRIOAUTO) );
			}

		//	Create/build the ED2K sub-menu
			menuED2K.CreateMenu();
			menuED2K.AppendMenu(MF_STRING,MP_GETED2KLINK, GetStringFromShortcutCode(IDS_DL_LINK1, SCUT_LINK, SSP_TAB_PREFIX));
			menuED2K.AppendMenu(MF_STRING,MP_GETHTMLED2KLINK, GetStringFromShortcutCode(IDS_DL_LINK2, SCUT_LINK_HTML, SSP_TAB_PREFIX));
			menuED2K.AppendMenu(MF_STRING,MP_GETSOURCEED2KLINK, GetStringFromShortcutCode(IDS_CREATESOURCELINK, SCUT_LINK_SOURCE, SSP_TAB_PREFIX));
			menuED2K.AppendMenu(MF_STRING, MP_GETHASH, GetStringFromShortcutCode(IDS_COPYHASH, SCUT_LINK_HASH, SSP_TAB_PREFIX));

		//	Create/build the source filter sub-menu
			menuSourceFilter.CreateMenu();
			menuSourceFilter.AddMenuTitle(GetResString(IDS_SRCFILTERMENU_SHOWING));
			menuSourceFilter.AppendMenu(MF_STRING|(pFileItem->m_bShowUploadingSources?MF_CHECKED:0),MP_SRCFILTER_UPLOADING,GetResString(IDS_SRCFILTERMENU_UPLOADING));
			menuSourceFilter.AppendMenu(MF_STRING|(pFileItem->m_bShowOnQueueSources?MF_CHECKED:0),MP_SRCFILTER_ONQUEUE,GetResString(IDS_SRCFILTERMENU_ONQUEUE));
			menuSourceFilter.AppendMenu(MF_STRING|(pFileItem->m_bShowFullQueueSources?MF_CHECKED:0),MP_SRCFILTER_FULLQUEUE,GetResString(IDS_SRCFILTERMENU_FULLQUEUE));
			menuSourceFilter.AppendMenu(MF_STRING|(pFileItem->m_bShowConnectedSources?MF_CHECKED:0),MP_SRCFILTER_CONNECTED,GetResString(IDS_SRCFILTERMENU_CONNECTED));
			menuSourceFilter.AppendMenu(MF_STRING|(pFileItem->m_bShowConnectingSources?MF_CHECKED:0),MP_SRCFILTER_CONNECTING,GetResString(IDS_SRCFILTERMENU_CONNECTING));
			menuSourceFilter.AppendMenu(MF_STRING|(pFileItem->m_bShowNNPSources?MF_CHECKED:0),MP_SRCFILTER_NNP,GetResString(IDS_NONEEDEDPARTS));
			menuSourceFilter.AppendMenu(MF_STRING|(pFileItem->m_bShowWaitForFileReqSources?MF_CHECKED:0),MP_SRCFILTER_WAITFILEREQ,GetResString(IDS_WAITFILEREQ));
			menuSourceFilter.AppendMenu(MF_STRING|(pFileItem->m_bShowLowToLowIDSources?MF_CHECKED:0),MP_SRCFILTER_LOWTOLOWID,GetResString(IDS_SRCFILTERMENU_LOWIDTOLOWID));
			menuSourceFilter.AppendMenu(MF_STRING|(pFileItem->m_bShowLowIDOnOtherSrvSources?MF_CHECKED:0),MP_SRCFILTER_OTHERSRVLOWID,GetResString(IDS_ANOTHER_SERVER_LOWID));
			menuSourceFilter.AppendMenu(MF_STRING|(pFileItem->m_bShowBannedSources?MF_CHECKED:0),MP_SRCFILTER_BANNED,GetResString(IDS_SRCFILTERMENU_BANNED));
			menuSourceFilter.AppendMenu(MF_STRING|(pFileItem->m_bShowErrorSources?MF_CHECKED:0),MP_SRCFILTER_ERROR,GetResString(IDS_SRCFILTERMENU_ERROR));
			menuSourceFilter.AppendMenu(MF_STRING|(pFileItem->m_bShowA4AFSources?MF_CHECKED:0),MP_SRCFILTER_A4AF,GetResString(IDS_SRCFILTERMENU_A4AF));
			menuSourceFilter.AppendMenu(MF_STRING|(pFileItem->m_bShowUnknownSources?MF_CHECKED:0),MP_SRCFILTER_UNKNOWN,GetResString(IDS_UNKNOWN));

			menuSourceFilter.AppendMenu(MF_SEPARATOR);

			menuSourceFilter.AppendMenu(MF_STRING,MP_SRCFILTER_SHOWALL,GetResString(IDS_SRCFILTERMENU_SHOWALL));
			menuSourceFilter.AppendMenu(MF_STRING,MP_SRCFILTER_HIDEALL,GetResString(IDS_SRCFILTERMENU_HIDEALL));

		//	Create/build the advanced sub-menu
			menuAdvanced.CreateMenu();
			menuAdvanced.AppendMenu(MF_STRING | ((bJustOne && bFileDone) ? MF_GRAYED : MF_ENABLED),
				MP_DOCLEANUP, GetStringFromShortcutCode(IDS_DOCLEANUP, SCUT_FILE_NAMECLEANUP, SSP_TAB_PREFIX));
			menuAdvanced.AppendMenu(MF_STRING|
				(!bFileDone ? ( (pPartFile->IsAlternativeOutputDir()) ? MF_CHECKED : MF_UNCHECKED) : MF_GRAYED),
				MP_CHANGEDIR, GetResString(IDS_CHANGEDIR));
			menuAdvanced.AppendMenu( MF_STRING |
				((bJustOne && !pPartFile->IsLastBlockComplete() && !pPartFile->IsPreallocated()) ? MF_ENABLED : MF_GRAYED),
				MP_PREALLOCATE, GetStringFromShortcutCode(IDS_DL_PREALLOCATE, SCUT_DL_PREALLOC, SSP_TAB_PREFIX) );
			menuAdvanced.AppendMenu(MF_STRING|(bFileDone && !g_App.m_pPrefs->GetAVPath().IsEmpty() && g_App.m_pPrefs->IsAVEnabled() ? MF_ENABLED:MF_GRAYED), MP_AV_SCAN, GetResString(IDS_AV_SCAN));

			bool	bOneNotDone = (bJustOne && !bFileDone);

			menuAdvanced.AppendMenu( MF_STRING |
				( (bOneNotDone && pPartFile->AllowGet1stLast()) ?
				((pPartFile->GetMovieMode() != 0) ? MF_CHECKED : MF_UNCHECKED) : MF_GRAYED ),
				MP_MOVIE, GetResString(IDS_MOVIE) );

			menuAdvanced.AppendMenu(MF_SEPARATOR);
			menuAdvanced.AppendMenu(MF_STRING|((bOneNotDone) ? MF_ENABLED : MF_GRAYED), MP_ALL_A4AF_TO_HERE, GetStringFromShortcutCode(IDS_ALL_A4AF_TO_HERE, SCUT_DL_A4AF, SSP_TAB_PREFIX));
			menuAdvanced.AppendMenu(MF_STRING|((bOneNotDone) ? MF_ENABLED : MF_GRAYED), MP_ALL_A4AF_SAMECAT, GetStringFromShortcutCode(IDS_ALL_A4AF_SAMECAT, SCUT_DL_A4AFSAMECAT, SSP_TAB_PREFIX));
			menuAdvanced.AppendMenu( MF_STRING | ( (bOneNotDone) ?
				( (pPartFile == g_App.m_pDownloadQueue->GetA4AFAutoFile()) ?
				MF_CHECKED : MF_UNCHECKED ) : (MF_GRAYED | MF_UNCHECKED) ), MP_ALL_A4AF_AUTO, GetStringFromShortcutCode(IDS_TREE_DL_A4AF_AUTO, SCUT_DL_A4AFAUTO, SSP_TAB_PREFIX) );
			menuAdvanced.AppendMenu( MF_STRING | ((bOneNotDone) ? MF_ENABLED : MF_GRAYED),
				MP_ALL_A4AF_TO_OTHER, GetStringFromShortcutCode(IDS_ALL_A4AF_TO_OTHER, SCUT_DL_A4AFOTHER, SSP_TAB_PREFIX) );

		//	Create/build the web services sub-menu
			menuWeb.CreateMenu();

			UINT		dwWebServicesEmptyFlag = MF_STRING | ((UpdateURLMenu(menuWeb) == 0) ? MF_GRAYED : 0);
		//	Create/build the category assignment sub-menu
			CTitleMenu	catMenu;

			catMenu.CreatePopupMenu();
			catMenu.AddMenuTitle(GetResString(IDS_CAT));

		//	If there are only predefined cats, gray the assign menu
			UINT	dwNoUserCatsFlag = MF_STRING | MF_GRAYED;

			if (!bFileDone && (CCat::GetNumCats() > CCat::GetNumPredefinedCats()))
			{
				CString	strBuffer;
				int		iFileCatIndex = CCat::GetUserCatIndexByID(pPartFile->GetCatID());

				dwNoUserCatsFlag = MF_STRING;
				for (int i = 0; i < CCat::CatIndexToUserCatIndex(CCat::GetNumCats()); i++)
				{
					if (i == 0)
						GetResString(&strBuffer, IDS_CAT_UNASSIGN);
					else
					{
						strBuffer = CCat::GetCatByUserIndex(i)->GetTitle();
						strBuffer.Replace(_T("&"), _T("&&"));
					}
					catMenu.AppendMenu( MF_STRING | (iFileCatIndex == i ? MF_CHECKED : 0),
						MP_ASSIGNCAT + i, strBuffer );
				}
			}

		//	Build the File menu
			menuFile.AppendMenu(MF_STRING|((eFileStatus == PS_COMPLETE) ? MF_ENABLED : MF_GRAYED), MP_CLEARCOMPLETED, GetStringFromShortcutCode(IDS_DL_CLEAR, SCUT_DL_CLEAR, SSP_TAB_PREFIX));
			menuFile.AppendMenu(MF_SEPARATOR);
		//	Add the category sub-menu
			menuFile.AppendMenu(dwNoUserCatsFlag|MF_POPUP, (UINT_PTR)catMenu.m_hMenu, GetResString(IDS_CAT_ASSIGN));
		//	Add the priority sub-menu
			menuFile.AppendMenu(uFlagIfDone|MF_POPUP, (UINT_PTR)menuPriority.m_hMenu, GetResString(IDS_PRIORITY));

			menuFile.AppendMenu(MF_STRING|(!bFileDone ? MF_ENABLED : MF_GRAYED),MP_CANCEL, GetStringFromShortcutCode(IDS_MAIN_BTN_CANCEL, SCUT_DL_CANCEL, SSP_TAB_PREFIX));

			menuFile.AppendMenu( MF_STRING |
				((eFileStatus != PS_STOPPED && eFileStatus != PS_ERROR && !bFileDone) ? MF_ENABLED:MF_GRAYED),
				MP_STOP, GetStringFromShortcutCode(IDS_STOP_VERB, SCUT_DL_STOP, SSP_TAB_PREFIX) );

			menuFile.AppendMenu( MF_STRING |
				((eFileStatus != PS_PAUSED && eFileStatus != PS_ERROR && !bFileDone) ? MF_ENABLED:MF_GRAYED),
				MP_PAUSE, GetStringFromShortcutCode(IDS_PAUSE_VERB, SCUT_DL_PAUSE, SSP_TAB_PREFIX) );

			menuFile.AppendMenu( MF_STRING |
				((eFileStatus == PS_PAUSED || eFileStatus == PS_STOPPED) ? MF_ENABLED:MF_GRAYED),
				MP_RESUME, GetStringFromShortcutCode(IDS_RESUME, SCUT_DL_RESUME, SSP_TAB_PREFIX) );

			if (bJustOne && eFileStatus == PS_ERROR)
				menuFile.AppendMenu(MF_STRING, MP_INITIALIZE, GetResString(IDS_DL_INITIALIZE));

			menuFile.AppendMenu(MF_SEPARATOR);

			menuFile.AppendMenu( MF_STRING |
				((bJustOne && eFileStatus == PS_COMPLETE) ? MF_ENABLED : MF_GRAYED),
				MP_OPEN, GetStringFromShortcutCode(IDS_OPENFILE, SCUT_FILE_OPEN, SSP_TAB_PREFIX) );

			menuFile.AppendMenu( MF_STRING |
				((bJustOne && eFileStatus == PS_COMPLETE) ? MF_ENABLED : MF_GRAYED),
				MP_OPENFOLDER, GetStringFromShortcutCode(IDS_OPENFOLDER, SCUT_FILE_OPENDIR, SSP_TAB_PREFIX) );

			menuFile.AppendMenu( MF_STRING |
				((bJustOne && pPartFile->PreviewAvailable()) ? MF_ENABLED : MF_GRAYED),
				MP_PREVIEW, GetStringFromShortcutCode(IDS_PREVIEW_VERB, SCUT_DL_PREVIEW, SSP_TAB_PREFIX) );

			menuFile.AppendMenu( MF_STRING | ((bJustOne) ? MF_ENABLED : MF_GRAYED), MP_METINFO,
				GetStringFromShortcutCode(IDS_DL_INFO, SCUT_FILE_DETAILS, SSP_TAB_PREFIX) );

			menuFile.AppendMenu( MF_STRING |
				((bJustOne && (pPartFile->HasComment() || pPartFile->HasRating())) /*&& !bFileDone*/ ? MF_ENABLED : MF_GRAYED),
				MP_VIEWFILECOMMENTS, GetStringFromShortcutCode(IDS_CMT_SHOWALL, SCUT_FILE_COMMENTS, SSP_TAB_PREFIX) );

			menuFile.AppendMenu(MF_SEPARATOR);
		//	Add the source-filter sub-menu
			menuFile.AppendMenu(MF_STRING|MF_POPUP,reinterpret_cast<UINT_PTR>(menuSourceFilter.m_hMenu), GetResString(IDS_SRCFILTERMENU_TITLE));
		//	Add the advanced sub-menu
			menuFile.AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)menuAdvanced.m_hMenu, GetResString(IDS_PW_ADVANCED));
		//	Add the ED2K sub-menu
			menuFile.AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)menuED2K.m_hMenu, GetResString(IDS_ED2KLINKFIX));
		//	Add the web services sub-menu
			menuFile.AppendMenu(dwWebServicesEmptyFlag|MF_POPUP, (UINT_PTR)menuWeb.m_hMenu, GetResString(IDS_WEBSERVICES));

			menuFile.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this, NULL);
		}
	//	If the selected item isn't a file...
		else
		{
			CSourceDLItem			*pSourceItem = dynamic_cast<CSourceDLItem*>(pItem);
			const CUpDownClient		*pSource = pSourceItem->GetSource();
			CTitleMenu		menuClient;

			if (pSource != NULL)
				g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.SetList(INFOLISTTYPE_SOURCE, const_cast<CUpDownClient*>(pSource));

		//	Create the client menu
			menuClient.CreatePopupMenu();
			menuClient.AddMenuTitle(GetResString(IDS_CLIENTS));

			menuClient.AppendMenu(MF_STRING, MP_DETAIL, GetStringFromShortcutCode(IDS_SHOWDETAILS, SCUT_SRC_DETAILS, SSP_TAB_PREFIX));
			if (pSource->IsFriend())
				menuClient.AppendMenu(MF_STRING, MP_REMOVEFRIEND, GetStringFromShortcutCode(IDS_REMOVEFRIEND, SCUT_SRC_FRIEND, SSP_TAB_PREFIX));
			else
				menuClient.AppendMenu(MF_STRING, MP_ADDFRIEND, GetStringFromShortcutCode(IDS_ADDFRIEND, SCUT_SRC_FRIEND, SSP_TAB_PREFIX));
			menuClient.AppendMenu(MF_STRING, MP_MESSAGE, GetStringFromShortcutCode(IDS_SEND_MSG, SCUT_SRC_MSG, SSP_TAB_PREFIX));
			menuClient.AppendMenu( MF_STRING | ((pSource && pSource->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED),
				MP_SHOWLIST, GetStringFromShortcutCode(IDS_VIEWFILES, SCUT_SRC_SHAREDFILES, SSP_TAB_PREFIX) );
			if (pSourceItem->IsAskedForAnotherFile())
				menuClient.AppendMenu(MF_STRING, MP_DOWNNOW, GetResString(IDS_DOWNLOAD_A4AF));

			menuClient.SetDefaultItem((g_App.m_pPrefs->GetDetailsOnClick()) ? MP_DETAIL : MP_MESSAGE);
			menuClient.TrackPopupMenuEx(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this, NULL);
		}
	}
//	If there's no selection (i.e. right-click on empty space in download list)...
	else
	{
		menuFile.CreatePopupMenu();
		menuFile.AddMenuTitle(GetResString(IDS_DOWNLOADMENUTITLE));

		menuFile.AppendMenu(MF_STRING, MP_CLEARALLCOMPLETED, GetStringFromShortcutCode(IDS_TREE_DL_CLEAR_ALL_COMPLETED, SCUT_DL_CLEARALL, SSP_TAB_PREFIX));
		menuFile.AppendMenu(MF_SEPARATOR);

	//	Create/build the source filter sub-menu
		menuSourceFilter.CreateMenu();
		menuSourceFilter.AddMenuTitle(GetResString(IDS_SRCFILTERMENU_SHOWING));
		menuSourceFilter.AppendMenu(MF_STRING|(m_bShowUploadingSources?MF_CHECKED:0),MP_SRCFILTER_UPLOADING,GetResString(IDS_SRCFILTERMENU_UPLOADING));
		menuSourceFilter.AppendMenu(MF_STRING|(m_bShowOnQueueSources?MF_CHECKED:0),MP_SRCFILTER_ONQUEUE,GetResString(IDS_SRCFILTERMENU_ONQUEUE));
		menuSourceFilter.AppendMenu(MF_STRING|(m_bShowFullQueueSources?MF_CHECKED:0),MP_SRCFILTER_FULLQUEUE,GetResString(IDS_SRCFILTERMENU_FULLQUEUE));
		menuSourceFilter.AppendMenu(MF_STRING|(m_bShowConnectedSources?MF_CHECKED:0),MP_SRCFILTER_CONNECTED,GetResString(IDS_SRCFILTERMENU_CONNECTED));
		menuSourceFilter.AppendMenu(MF_STRING|(m_bShowConnectingSources?MF_CHECKED:0),MP_SRCFILTER_CONNECTING,GetResString(IDS_SRCFILTERMENU_CONNECTING));
		menuSourceFilter.AppendMenu(MF_STRING|(m_bShowNNPSources?MF_CHECKED:0),MP_SRCFILTER_NNP,GetResString(IDS_NONEEDEDPARTS));
		menuSourceFilter.AppendMenu(MF_STRING|(m_bShowWaitForFileReqSources?MF_CHECKED:0),MP_SRCFILTER_WAITFILEREQ,GetResString(IDS_WAITFILEREQ));
		menuSourceFilter.AppendMenu(MF_STRING|(m_bShowLowToLowIDSources?MF_CHECKED:0),MP_SRCFILTER_LOWTOLOWID,GetResString(IDS_SRCFILTERMENU_LOWIDTOLOWID));
		menuSourceFilter.AppendMenu(MF_STRING|(m_bShowLowIDOnOtherSrvSources?MF_CHECKED:0),MP_SRCFILTER_OTHERSRVLOWID,GetResString(IDS_ANOTHER_SERVER_LOWID));
		menuSourceFilter.AppendMenu(MF_STRING|(m_bShowBannedSources?MF_CHECKED:0),MP_SRCFILTER_BANNED,GetResString(IDS_SRCFILTERMENU_BANNED));
		menuSourceFilter.AppendMenu(MF_STRING|(m_bShowErrorSources?MF_CHECKED:0),MP_SRCFILTER_ERROR,GetResString(IDS_SRCFILTERMENU_ERROR));
		menuSourceFilter.AppendMenu(MF_STRING|(m_bShowA4AFSources?MF_CHECKED:0),MP_SRCFILTER_A4AF,GetResString(IDS_SRCFILTERMENU_A4AF));
		menuSourceFilter.AppendMenu(MF_STRING|(m_bShowUnknownSources?MF_CHECKED:0),MP_SRCFILTER_UNKNOWN,GetResString(IDS_UNKNOWN));

		menuSourceFilter.AppendMenu(MF_SEPARATOR);

		menuSourceFilter.AppendMenu(MF_STRING,MP_SRCFILTER_SHOWALL,GetResString(IDS_SRCFILTERMENU_SHOWALL));
		menuSourceFilter.AppendMenu(MF_STRING,MP_SRCFILTER_HIDEALL,GetResString(IDS_SRCFILTERMENU_HIDEALL));

		menuSourceFilter.AppendMenu(MF_SEPARATOR);

		menuSourceFilter.AppendMenu(MF_STRING|(m_bSmartFilter?MF_CHECKED:0),MP_SMARTFILTER,GetResString(IDS_SRCFILTERMENU_SMARTFILTER));
	//	Add the source-filter sub-menu
		menuFile.AppendMenu(MF_STRING|MF_POPUP,reinterpret_cast<UINT_PTR>(menuSourceFilter.m_hMenu), GetResString(IDS_SRCFILTERMENU_TITLE));

		menuFile.TrackPopupMenuEx(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this, NULL);
	}

//	Menu objects are destroyed in their destructor

#endif //OLD_SOCKETS_ENABLED

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CDownloadListCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	NOPRM(lParam);
	EMULE_TRY

//	Commands which don't depend on selection
	switch (wParam)
	{
		case MP_CLEARALLCOMPLETED:
		{
			SetRedraw(false);
			g_App.m_pDownloadList->ClearCompleted(CAT_NONE);
			SetRedraw(true);
			return true;
		}
	}

	if (!ListSelectionIsEmpty())
	{
	//	Construct a list of all the selected part files
		CTypedPtrList<CPtrList, CPartFile*>			selectedList;
		CTypedPtrList<CPtrList, CPartFileDLItem*>	selectedItemList;
		UINT		iNumSelected = GetSelectedCount();
		int			iIndex;
		POSITION	pos = GetFirstSelectedItemPosition();
		CMuleCtrlItem		*pItem = NULL;

		if (pos != NULL)
		{
			POSITION	posTmp = pos;

			if ((iIndex = GetNextSelectedItem(posTmp)) >= 0)
				pItem = reinterpret_cast<CMuleCtrlItem*>(GetItemData(iIndex));
		}

		CPartFileDLItem		*pFileItem = dynamic_cast<CPartFileDLItem*>(pItem);

		while (pos != NULL)
		{
			if ((iIndex = GetNextSelectedItem(pos)) >= 0)
			{
				if (typeid(*ListGetItemAt(iIndex)) == typeid(CPartFileDLItem))
				{
					selectedItemList.AddTail(dynamic_cast<CPartFileDLItem*>(ListGetItemAt(iIndex)));
					selectedList.AddTail(ListGetItemAt(iIndex)->GetFile());
				}
			}
		}
	//	If there's at least one part file item selected...
		if (pFileItem != NULL)
		{
			CPartFile		*pPartFile = pItem->GetFile();

			switch (wParam)
			{
				case MP_CANCEL:
				//	For multiple selections
					if (iNumSelected > 0)
					{
						CString		strFileList = GetResString((iNumSelected == 1) ? IDS_Q_CANCELDL2 : IDS_Q_CANCELDL);
						bool		bIsValidDelete = false;

						for (pos = selectedList.GetHeadPosition(); pos != NULL; selectedList.GetNext(pos))
						{
							EnumPartFileStatuses	eFileStatus = selectedList.GetAt(pos)->GetStatus();

							if ((eFileStatus != PS_COMPLETE) && (eFileStatus != PS_COMPLETING))
							{
								bIsValidDelete = true;
								if (iNumSelected < 50)	// a number is used to limit messagebox size
								{
									strFileList += _T("\n");
									strFileList += selectedList.GetAt(pos)->GetFileName();
								}
							}
						}

						if (bIsValidDelete && AfxMessageBox(strFileList, MB_ICONQUESTION|MB_YESNO) == IDYES)
						{
							SetRedraw(false);

							while (!selectedList.IsEmpty())
							{
								HideSources(selectedList.GetHead());
								switch(selectedList.GetHead()->GetStatus())
								{
									case PS_WAITINGFORHASH:
									case PS_HASHING:
									case PS_COMPLETING:
									case PS_COMPLETE:
										break;
									case PS_STOPPED:
									case PS_PAUSED:
									default:
										if (selectedList.GetHead()->IsFakesDotRar())
										{
											g_App.m_pPrefs->SetDLingFakeListVersion(0);
											g_App.m_pPrefs->SetDLingFakeListLink(_T(""));
										}
										selectedList.GetHead()->DeleteFile();
										break;
								}
								selectedList.RemoveHead();
							}

							SetRedraw(true);
							g_App.m_pMDlg->m_wndTransfer.UpdateCatTabTitles();
						}
					}
					break;

				case MP_PRIOHIGH:
					if (iNumSelected > 1)
					{
						SetRedraw(false);
						while(!selectedList.IsEmpty())
						{
							selectedList.GetHead()->SetAutoPriority(false);
							selectedList.GetHead()->SetPriority(PR_HIGH);
							selectedList.RemoveHead();
						}
						SetRedraw(true);
					}
					else
					{
						pPartFile->SetAutoPriority(false);
						pPartFile->SetPriority(PR_HIGH);
					}
					break;

				case MP_PRIOLOW:
					if (iNumSelected > 1)
					{
						SetRedraw(false);
						while(!selectedList.IsEmpty())
						{
							selectedList.GetHead()->SetAutoPriority(false);
							selectedList.GetHead()->SetPriority(PR_LOW);
							selectedList.RemoveHead();
						}
						SetRedraw(true);
					}
					else
					{
						pPartFile->SetAutoPriority(false);
						pPartFile->SetPriority(PR_LOW);
					}
					break;

				case MP_PRIONORMAL:
					if (iNumSelected > 1)
					{
						SetRedraw(false);
						while(!selectedList.IsEmpty())
						{
							selectedList.GetHead()->SetAutoPriority(false);
							selectedList.GetHead()->SetPriority(PR_NORMAL);
							selectedList.RemoveHead();
						}
						SetRedraw(true);
					}
					else
					{
						pPartFile->SetAutoPriority(false);
						pPartFile->SetPriority(PR_NORMAL);
					}
					break;

				case MP_PRIOAUTO:
					if (iNumSelected > 1)
					{
						SetRedraw(false);
						while(!selectedList.IsEmpty())
						{
							selectedList.GetHead()->SetAutoPriority(true);
							selectedList.GetHead()->SetPriority(PR_HIGH);
							selectedList.RemoveHead();
						}
						SetRedraw(true);
					}
					else
					{
						pPartFile->SetAutoPriority(true);
						pPartFile->SetPriority(PR_HIGH);
					}
					break;

				case MP_PAUSE:
					if (iNumSelected > 1)
					{
						SetRedraw(false);
						while(!selectedList.IsEmpty())
						{
							selectedList.GetHead()->PauseFile();
							selectedList.RemoveHead();
						}
						SetRedraw(true);
					}
					else
						pPartFile->PauseFile();
					break;

				case MP_RESUME:
					if (iNumSelected > 1)
					{
						SetRedraw(false);
						while(!selectedList.IsEmpty())
						{
							selectedList.GetHead()->ResumeFile();
							selectedList.RemoveHead();
						}
						SetRedraw(true);
					}
					else
						pPartFile->ResumeFile();
					break;

				case MP_INITIALIZE:
					if (iNumSelected == 1)
					{
						pPartFile->LoadPartFile(pPartFile->GetTempDir(), pPartFile->GetPartMetFileName());
						if (pPartFile->GetRawStatus() == PS_READY)
							g_App.m_pSharedFilesList->SafeAddKnownFile(pPartFile);
					}
					break;

				case MP_PREALLOCATE:
					if (iNumSelected == 1)
					{
						if (!pPartFile->IsLastBlockComplete())
							pPartFile->AllocateNeededSpace();
					}
					break;

				case MP_RENAME:
				{
					CString		strTemp;

					GetResString(&strTemp, IDS_RENAME);

					InputBox	inputbox(strTemp, pPartFile->GetFileName(), true);

					inputbox.DoModal();
					strTemp = inputbox.GetInput();
					if (!inputbox.WasCancelled() && !strTemp.IsEmpty())
					{
						EnumPartFileStatuses	eFileStatus = pPartFile->GetStatus();

					//	As the dialog could be opened for a while we need to check status once again
						if ((eFileStatus != PS_COMPLETE) && (eFileStatus != PS_COMPLETING))
						{
							pPartFile->SetFileName(strTemp);
							pPartFile->SavePartFile();
							pPartFile->UpdateDisplayedInfo();
							g_App.m_pSharedFilesList->UpdateItem((CKnownFile*)pPartFile);
							g_App.m_pMDlg->m_wndTransfer.UpdateInfoHeader();
						}
					}
					break;
				}
				case MP_STOP:
					if (iNumSelected > 1)
					{
						SetRedraw(false);
						while(!selectedList.IsEmpty())
						{
							CPartFile *selected = selectedList.GetHead();
							HideSources(selected);
							selected->StopFile();
							selectedList.RemoveHead();
						}
						SetRedraw(true);
					}
					else
					{
						HideSources(pPartFile);
						pPartFile->StopFile();
					}
					break;

				case MP_CLEARCOMPLETED:
					SetRedraw(false);
					if (iNumSelected > 1)
					{
						while(!selectedList.IsEmpty())
						{
							CPartFile *pSelected = selectedList.GetHead();
							g_App.m_pDownloadList->ClearCompleted(pSelected);
							selectedList.RemoveHead();
						}
					}
					else
						g_App.m_pDownloadList->ClearCompleted(pPartFile);
					SetRedraw(true);
					break;

				case MP_METINFO:
				{
					CFileDetails		dialog(IDS_FD_TITLE, pPartFile, this, 0);

					dialog.DoModal();
					break;
				}
				case MP_METINFOSOURCES:
				{
					CFileDetails		dialog(IDS_FD_TITLE, pPartFile, this, 2);

					dialog.DoModal();
					break;
				}
				case MP_GETED2KLINK:
					if (iNumSelected > 1)
					{
						CString str;
						while(!selectedList.IsEmpty())
						{
							str += selectedList.GetHead()->CreateED2kLink();
							selectedList.RemoveHead();
							if (!selectedList.IsEmpty())
								str += _T("\r\n");
						}
						g_App.CopyTextToClipboard(str);
					}
					else
						g_App.CopyTextToClipboard(pPartFile->CreateED2kLink());
					break;

				case MP_GETHTMLED2KLINK:
					if (iNumSelected > 1)
					{
						CString str;
						while(!selectedList.IsEmpty())
						{
							str += selectedList.GetHead()->CreateHTMLED2kLink();
							selectedList.RemoveHead();
							if (!selectedList.IsEmpty())
								str += _T("\r\n");
						}
						g_App.CopyTextToClipboard(str);
					}
					else
						g_App.CopyTextToClipboard(pPartFile->CreateHTMLED2kLink());
					break;

				case MP_GETSOURCEED2KLINK:
					if (iNumSelected > 1)
					{
						CString str;
						while(!selectedList.IsEmpty())
						{
							str += selectedList.GetHead()->CreateED2KSourceLink(7, 20);
							selectedList.RemoveHead();
							if (!selectedList.IsEmpty())
								str += _T("\r\n");
						}
						g_App.CopyTextToClipboard(str);
					}
					else
						g_App.CopyTextToClipboard(pPartFile->CreateED2KSourceLink(7, 20));
					break;

				case MP_GETHASH:
					if (iNumSelected > 0)
					{
						CString str;

						while (!selectedList.IsEmpty())
						{
							str += HashToString(selectedList.RemoveHead()->GetFileHash());
							if (!selectedList.IsEmpty())
								str += _T("\r\n");
						}
						g_App.CopyTextToClipboard(str);
					}
					break;

				case MP_OPEN:
					if (iNumSelected == 1)
						ShellOpenFile(pPartFile->GetFilePath());
					break;

				case MP_PREVIEW:
					if (iNumSelected == 1)
						pPartFile->PreviewFile();
					break;

				case MP_VIEWFILECOMMENTS:
				{
					CCommentDialogLst dialog(pPartFile);
					dialog.DoModal();
					break;
				}
				case MP_CHANGEDIR:
				{
					CString		strDir = pPartFile->GetOutputDir();

					if (strDir.GetLength() == 2)
						strDir += _T('\\');
					CString newpath = BrowseFolder(g_App.m_pMDlg->m_hWnd, GetResString(IDS_SELECTOUTPUTDIR), strDir);
					if (newpath.GetLength() == 3)
						newpath.Remove(_T('\\'));
					if (iNumSelected > 1)
					{
						while (!selectedList.IsEmpty())
						{
							CPartFile		*pPFTmp = selectedList.RemoveHead();

							if (newpath.CompareNoCase(pPFTmp->GetOutputDir()) != 0)
							{
								pPFTmp->SetAlternativeOutputDir(&newpath);
								pPFTmp->SaveSettingsFile();
							}
						}
					}
					if (newpath.CompareNoCase(strDir) != 0)
					{
						pPartFile->SetAlternativeOutputDir(&newpath);
						pPartFile->SaveSettingsFile();
						pPartFile->GetOutputDir();
					}
					break;
				}
				case MP_DOCLEANUP:
					while (!selectedList.IsEmpty())
					{
						CPartFile* selFile = selectedList.GetHead();
						EnumPartFileStatuses	eFileStatus = selFile->GetStatus();

						if ((eFileStatus != PS_COMPLETE) && (eFileStatus != PS_COMPLETING))
						{
							selFile->SetFileName(CleanupFilename(selFile->GetFileName()));
							selFile->SavePartFile();
							selFile->UpdateDisplayedInfo();
							g_App.m_pSharedFilesList->UpdateItem((CKnownFile*)selFile);
						}
						selectedList.RemoveHead();
					}
					break;

				case MP_ALL_A4AF_TO_HERE:
					pPartFile->DownloadAllA4AF();
					break;

				case MP_ALL_A4AF_SAMECAT:
					pPartFile->DownloadAllA4AF(true);
					break;

				case MP_ALL_A4AF_AUTO:
					if (g_App.m_pDownloadQueue->GetA4AFAutoFile() == pPartFile)
					//	Current pPartFile is A4AF auto => User want to switch it off
						g_App.m_pDownloadQueue->SetA4AFAutoFile(NULL);
					else
					//	Another pPartFile is A4AF auto => switch to new pPartFile
						g_App.m_pDownloadQueue->SetA4AFAutoFile(pPartFile);
					break;

				case MP_ALL_A4AF_TO_OTHER:
				{
					ClientList	clientListCopy;

					SetRedraw(false);

					pPartFile->GetCopySourceLists(SLM_ALLOWED_TO_A4AF_SWAP, &clientListCopy);
					for (ClientList::const_iterator cIt = clientListCopy.begin(); cIt != clientListCopy.end(); cIt++)
					{
						(*cIt)->SwapToAnotherFile(NULL);
					}
					SetRedraw(true);
					break;
				}
				case MP_MOVIE:
				{
					EnumPartFileStatuses	eFileStatus = pPartFile->GetStatus();

					if ((eFileStatus != PS_COMPLETE) && (eFileStatus != PS_COMPLETING))
						pPartFile->GetFirstLastChunk4Preview();
					break;
				}
				case MP_OPENFOLDER:
					if (iNumSelected == 1)
						ShellOpenFile(pPartFile->GetPath());
					break;

				case MP_AV_SCAN:
				{
					CString	strBuffer = g_App.m_pPrefs->GetAVParams();

					while (!selectedList.IsEmpty())
					{
						if (!selectedList.GetHead()->IsPartFile())
						{
							strBuffer += _T(" \"");
							strBuffer += ConcatFullPath(selectedList.GetHead()->GetPath(), selectedList.GetHead()->GetFileName());
							strBuffer += _T('\"');
						}
						selectedList.RemoveHead();
					}
					ShellExecute(NULL, _T("open"), g_App.m_pPrefs->GetAVPath(), strBuffer, NULL, SW_SHOW);
					break;
				}
				case MP_SRCFILTER_UPLOADING:
					if (iNumSelected > 0)
					{
						while (!selectedItemList.IsEmpty())
						{
							selectedItemList.GetHead()->m_bShowUploadingSources = !selectedItemList.GetHead()->m_bShowUploadingSources;
							selectedItemList.RemoveHead();
						}
						UpdateSourceItems();
					}
					break;

				case MP_SRCFILTER_ONQUEUE:
					if (iNumSelected > 0)
					{
						while (!selectedItemList.IsEmpty())
						{
							selectedItemList.GetHead()->m_bShowOnQueueSources = !selectedItemList.GetHead()->m_bShowOnQueueSources;
							selectedItemList.RemoveHead();
						}
						UpdateSourceItems();
					}
					break;

				case MP_SRCFILTER_FULLQUEUE:
					if (iNumSelected > 0)
					{
						while (!selectedItemList.IsEmpty())
						{
							selectedItemList.GetHead()->m_bShowFullQueueSources = !selectedItemList.GetHead()->m_bShowFullQueueSources;
							selectedItemList.RemoveHead();
						}
						UpdateSourceItems();
					}
					break;

				case MP_SRCFILTER_CONNECTED:
					if (iNumSelected > 0)
					{
						while (!selectedItemList.IsEmpty())
						{
							selectedItemList.GetHead()->m_bShowConnectedSources = !selectedItemList.GetHead()->m_bShowConnectedSources;
							selectedItemList.RemoveHead();
						}
						UpdateSourceItems();
					}
					break;

				case MP_SRCFILTER_CONNECTING:
					if (iNumSelected > 0)
					{
						while (!selectedItemList.IsEmpty())
						{
							selectedItemList.GetHead()->m_bShowConnectingSources = !selectedItemList.GetHead()->m_bShowConnectingSources;
							selectedItemList.RemoveHead();
						}
						UpdateSourceItems();
					}
					break;

				case MP_SRCFILTER_NNP:
					if (iNumSelected > 0)
					{
						while (!selectedItemList.IsEmpty())
						{
							selectedItemList.GetHead()->m_bShowNNPSources = !selectedItemList.GetHead()->m_bShowNNPSources;
							selectedItemList.RemoveHead();
						}
						UpdateSourceItems();
					}
					break;

				case MP_SRCFILTER_WAITFILEREQ:
					if (iNumSelected > 0)
					{
						while (!selectedItemList.IsEmpty())
						{
							selectedItemList.GetHead()->m_bShowWaitForFileReqSources = !selectedItemList.GetHead()->m_bShowWaitForFileReqSources;
							selectedItemList.RemoveHead();
						}
						UpdateSourceItems();
					}
					break;

				case MP_SRCFILTER_LOWTOLOWID:
					if (iNumSelected > 0)
					{
						while (!selectedItemList.IsEmpty())
						{
							selectedItemList.GetHead()->m_bShowLowToLowIDSources = !selectedItemList.GetHead()->m_bShowLowToLowIDSources;
							selectedItemList.RemoveHead();
						}
						UpdateSourceItems();
					}
					break;

				case MP_SRCFILTER_OTHERSRVLOWID:
					if (iNumSelected > 0)
					{
						while (!selectedItemList.IsEmpty())
						{
							selectedItemList.GetHead()->m_bShowLowIDOnOtherSrvSources = !selectedItemList.GetHead()->m_bShowLowIDOnOtherSrvSources;
							selectedItemList.RemoveHead();
						}
						UpdateSourceItems();
					}
					break;

				case MP_SRCFILTER_BANNED:
					if (iNumSelected > 0)
					{
						while (!selectedItemList.IsEmpty())
						{
							selectedItemList.GetHead()->m_bShowBannedSources = !selectedItemList.GetHead()->m_bShowBannedSources;
							selectedItemList.RemoveHead();
						}
						UpdateSourceItems();
					}
					break;

				case MP_SRCFILTER_ERROR:
					if (iNumSelected > 0)
					{
						while (!selectedItemList.IsEmpty())
						{
							selectedItemList.GetHead()->m_bShowErrorSources = !selectedItemList.GetHead()->m_bShowErrorSources;
							selectedItemList.RemoveHead();
						}
						UpdateSourceItems();
					}
					break;

				case MP_SRCFILTER_A4AF:
					if (iNumSelected > 0)
					{
						while (!selectedItemList.IsEmpty())
						{
							selectedItemList.GetHead()->m_bShowA4AFSources = !selectedItemList.GetHead()->m_bShowA4AFSources;
							selectedItemList.RemoveHead();
						}
						UpdateSourceItems();
					}
					break;

				case MP_SRCFILTER_UNKNOWN:
					if (iNumSelected > 0)
					{
						while (!selectedItemList.IsEmpty())
						{
							selectedItemList.GetHead()->m_bShowUnknownSources = !selectedItemList.GetHead()->m_bShowUnknownSources;
							selectedItemList.RemoveHead();
						}
						UpdateSourceItems();
					}
					break;

				case MP_SRCFILTER_HIDEALL:
					if (iNumSelected > 0)
					{
						while (!selectedItemList.IsEmpty())
						{
							selectedItemList.GetHead()->FilterAllSources();
							selectedItemList.RemoveHead();
						}
						UpdateSourceItems();
					}
					break;

				case MP_SRCFILTER_SHOWALL:
					if (iNumSelected > 0)
					{
						while (!selectedItemList.IsEmpty())
						{
							selectedItemList.GetHead()->FilterNoSources();
							selectedItemList.RemoveHead();
						}
						UpdateSourceItems();
					}
					break;

				default:
				//	Web services
					if (wParam <= MP_WEBURL + 64 && wParam >= MP_WEBURL)
						RunURL(pPartFile, g_App.m_strWebServiceURLArray.GetAt(wParam - MP_WEBURL));
				//	Assign/Unassign to/from Category
					else if (wParam >= MP_ASSIGNCAT && wParam <= MP_LASTASSIGNCAT)
					{
						while (!selectedList.IsEmpty())
						{
							CPartFile	*pSelectedFile = selectedList.GetHead();
							int			iUserCatIdx = wParam - MP_ASSIGNCAT;

							if (iUserCatIdx != 0)
								pSelectedFile->SetCatID(CCat::GetCatIDByUserIndex(iUserCatIdx));
							else
								pSelectedFile->SetCatID(CAT_NONE);
							selectedList.RemoveHead();
						}
					//	Hide the pPartFile if it's not in the displayed category
						ChangeCategoryByIndex(m_iCurTabIndex);
					//	Redraw the pPartFile in its new color assuming it is visible
						g_App.m_pDownloadList->UpdateFile(pPartFile);
					//	Update the category file counts
						g_App.m_pMDlg->m_wndTransfer.UpdateCatTabTitles();
					}
					break;
			}
		}
	//	If a source item is selected...
		else
		{
			CSourceDLItem		*pSourceItem = dynamic_cast<CSourceDLItem*>(pItem);
#ifdef OLD_SOCKETS_ENABLED
			CUpDownClient		*pSource = pSourceItem->GetSource();

			switch (wParam)
			{
				case MP_SHOWLIST:
					pSource->RequestSharedFileList();
					break;
				case MP_MESSAGE:
					g_App.m_pMDlg->m_wndChat.StartSession(pSource);
					break;
				case MP_ADDFRIEND:
					g_App.m_pFriendList->AddFriend(pSource);
					break;
				case MP_REMOVEFRIEND:
					g_App.m_pFriendList->RemoveFriend(pSource);
					break;
				case MP_DETAIL:
				{
					CClientDetails		dialog(IDS_CD_TITLE, pSource, this, 0);

					dialog.DoModal();
					break;
				}
				case MP_DOWNNOW:
				{
					CPartFile		*pParentFile = pSourceItem->GetParentFile();

					pSource->SwapToAnotherFile(pParentFile);
					break;
				}
			}
#endif //OLD_SOCKETS_ENABLED
		}
	}
//	If nothing was selected in the list...
	else
	{
		switch (wParam)
		{
			case MP_SRCFILTER_UPLOADING:
				m_bShowUploadingSources = !m_bShowUploadingSources;
				UpdateSourceItems();
				break;

			case MP_SRCFILTER_ONQUEUE:
				m_bShowOnQueueSources = !m_bShowOnQueueSources;
				UpdateSourceItems();
				break;

			case MP_SRCFILTER_FULLQUEUE:
				m_bShowFullQueueSources = !m_bShowFullQueueSources;
				UpdateSourceItems();
				break;

			case MP_SRCFILTER_CONNECTED:
				m_bShowConnectedSources = !m_bShowConnectedSources;
				UpdateSourceItems();
				break;

			case MP_SRCFILTER_CONNECTING:
				m_bShowConnectingSources = !m_bShowConnectingSources;
				UpdateSourceItems();
				break;

			case MP_SRCFILTER_NNP:
				m_bShowNNPSources = !m_bShowNNPSources;
				UpdateSourceItems();
				break;

			case MP_SRCFILTER_WAITFILEREQ:
				m_bShowWaitForFileReqSources = !m_bShowWaitForFileReqSources;
				UpdateSourceItems();
				break;

			case MP_SRCFILTER_LOWTOLOWID:
				m_bShowLowToLowIDSources = !m_bShowLowToLowIDSources;
				UpdateSourceItems();
				break;

			case MP_SRCFILTER_OTHERSRVLOWID:
				m_bShowLowIDOnOtherSrvSources = !m_bShowLowIDOnOtherSrvSources;
				UpdateSourceItems();
				break;

			case MP_SRCFILTER_BANNED:
				m_bShowBannedSources = !m_bShowBannedSources;
				UpdateSourceItems();
				break;

			case MP_SRCFILTER_ERROR:
				m_bShowErrorSources = !m_bShowErrorSources;
				UpdateSourceItems();
				break;

			case MP_SRCFILTER_A4AF:
				m_bShowA4AFSources = !m_bShowA4AFSources;
				UpdateSourceItems();
				break;

			case MP_SRCFILTER_UNKNOWN:
				m_bShowUnknownSources = !m_bShowUnknownSources;
				UpdateSourceItems();
				break;

			case MP_SRCFILTER_HIDEALL:
				FilterAllSources();
				UpdateSourceItems();
				break;

			case MP_SRCFILTER_SHOWALL:
				FilterNoSources();
				UpdateSourceItems();
				break;

			case MP_SMARTFILTER:
				m_bSmartFilter = !m_bSmartFilter;
			//	If we're turning smart filtering on...
				if (m_bSmartFilter)
				{
				//	Just update file items so the appropriate sources are shown.
					UpdateFileItems();
				}
			//	If we're turning smart filtering off...
				else
				{
				//	Reset the source filters on all file items and update them.
					ResetSourceFiltersForAllFiles();
				}
				break;
		}
	}

	return true;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::OnNMDividerDoubleClick(NMHEADER *pHeader)
{
	EMULE_TRY

	int		iColumn = pHeader->iItem;

	m_iColumnMaxWidths[iColumn] = 0;
	m_iMeasuringColumn = iColumn;
	Invalidate();
	UpdateWindow();
	m_iMeasuringColumn = -1;
	if (m_iColumnMaxWidths[iColumn] != 0)
		SetColumnWidth(iColumn, m_iColumnMaxWidths[iColumn]);
	else
		CMuleListCtrl::OnNMDividerDoubleClick(pHeader);
	Invalidate();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::OnColumnClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	EMULE_TRY

	NM_LISTVIEW	*pNMListView = reinterpret_cast<NM_LISTVIEW*>(pNMHDR);
	int			iSubItem = pNMListView->iSubItem;
	byte		byteSortOrder = m_byteSortAscending[iSubItem];
	bool		bIsCtrl = GetAsyncKeyState(VK_CONTROL) < 0;

	if (!bIsCtrl)
	{
	// Reverse sorting direction for the same column and keep the same if column was changed
		if (static_cast<int>(m_dwParamSort & MLC_COLUMNMASK) == iSubItem)
			m_byteSortAscending[iSubItem] = byteSortOrder = static_cast<byte>(~(byteSortOrder ^ ~1));

		m_bSortAscending = (byteSortOrder & 1) ? true : false;
		SetSortArrow(m_iCurrentSortItem = iSubItem, m_bSortAscending);

		g_App.m_pPrefs->SetColumnSortItem(CPreferences::TABLE_DOWNLOAD, iSubItem);
		g_App.m_pPrefs->SetColumnSortAscending(CPreferences::TABLE_DOWNLOAD, m_bSortAscending);
	}
	else
	{
	// Reverse sorting direction for the same column and keep the same if column was changed
		if (static_cast<int>((m_dwParamSort >> 16) & MLC_COLUMNMASK) == iSubItem)
			m_byteSortAscending[iSubItem] = byteSortOrder = static_cast<byte>(~(byteSortOrder ^ ~2));

		m_bSortSourcesAscending = (byteSortOrder & 2) ? true : false;
		SetSortArrow(m_iSourceSortItem = iSubItem, m_bSortSourcesAscending);

		g_App.m_pPrefs->SetColumnSortItem(CPreferences::TABLE_DOWNLOAD2, iSubItem);
		g_App.m_pPrefs->SetColumnSortAscending(CPreferences::TABLE_DOWNLOAD2, m_bSortSourcesAscending);
	}
	m_iSourceSortItem2 = -1;

	uint32	dwSortCode =
		m_iCurrentSortItem | (m_bSortAscending ? MLC_SORTASC : MLC_SORTDESC) |
		((m_iSourceSortItem | (m_bSortSourcesAscending ? MLC_SORTASC : MLC_SORTDESC)) << 16) |
		(MLC_DONTSORT << 24);

	SortItems(SortProc, dwSortCode);

	EMULE_CATCH

	*pResult = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CDownloadListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSortCode)
{
	EMULE_TRY

	CMuleCtrlItem		*pItem1 = reinterpret_cast<CMuleCtrlItem*>(lParam1);
	CPartFileDLItem		*pFileItem1 = dynamic_cast<CPartFileDLItem*>(pItem1);
	CSourceDLItem		*pSourceItem1 = dynamic_cast<CSourceDLItem*>(pItem1);
	CMuleCtrlItem		*pItem2 = reinterpret_cast<CMuleCtrlItem*>(lParam2);
	CPartFileDLItem		*pFileItem2 = dynamic_cast<CPartFileDLItem*>(pItem2);
	CSourceDLItem		*pSourceItem2 = dynamic_cast<CSourceDLItem*>(pItem2);
	int					iFileSortItem = 0;
	int					iSourceSortItem = 0;
	int					iSourceSortItem2 = -1;
	int					iFileSortMod = 1;
	int					iSourceSortMod = 1;
	int					iSourceSortMod2 = 1;
	DWORD				dwFilesSortCode = lParamSortCode & 0xFF;
	DWORD				dwSourceSortCode = (lParamSortCode >> 16) & 0xFF;
	DWORD				dwSourceSortCode2 = (lParamSortCode >> 24) & 0xFF;
	bool				bSortSourceCol2 = false;

//	Extract the sort items and sort flags from the sort code
	if ((dwFilesSortCode & MLC_SORTDESC) != 0)
	{
		iFileSortMod = -1;
	}
	iFileSortItem = dwFilesSortCode & MLC_COLUMNMASK;

	if ((dwSourceSortCode & MLC_SORTDESC) != 0)
	{
		iSourceSortMod = -1;
	}
	iSourceSortItem = dwSourceSortCode & MLC_COLUMNMASK;

	if (dwSourceSortCode2 != MLC_DONTSORT)
	{
		bSortSourceCol2 = true;
		if ((dwSourceSortCode2 & MLC_SORTDESC) != 0)
		{
			iSourceSortMod2 = -1;
		}
		iSourceSortItem2 = dwSourceSortCode2 & MLC_COLUMNMASK;
	}

	int		iCompare;

//	Files vs source?
	if (pFileItem1 != NULL && pSourceItem2 != NULL)
	{
		CPartFile	*pPartFile1 = pFileItem1->GetFile();
		CPartFile	*pPartFile2 = pSourceItem2->GetParentFile();

		if (pPartFile1 != pPartFile2)
			iCompare = Compare(pPartFile1, pPartFile2, iFileSortItem, iFileSortMod);
		else
			iCompare = -1;
	}
	else if (pSourceItem1 != NULL && pFileItem2 != NULL)
	{
		CPartFile	*pPartFile1 = pSourceItem1->GetParentFile();
		CPartFile	*pPartFile2 = pFileItem2->GetFile();

		if (pPartFile1 != pPartFile2)
			iCompare = Compare(pPartFile1, pPartFile2, iFileSortItem, iFileSortMod);
		else
			iCompare = 1;
	}
	else
	{
	//	Both files? (!)
		if (pFileItem1 != NULL)
		{
			CPartFile	*pPartFile1 = pFileItem1->GetFile();
			CPartFile	*pPartFile2 = pFileItem2->GetFile();

			iCompare = Compare(pPartFile1, pPartFile2, iFileSortItem, iFileSortMod);
		}
	//	Both sources...
		else
		{
			iCompare = Compare(pSourceItem1->GetParentFile(), pSourceItem2->GetParentFile(),iFileSortItem, iFileSortMod);

		//	If the sources aren't for the same file...
			if (iCompare != 0)
			{
				return iCompare;
			}
		//	A4AF clients kept separate, unless sorting for name
			if ( (pSourceItem1->IsAskedForAnotherFile() || pSourceItem2->IsAskedForAnotherFile())
			  && iSourceSortItem != DLCOL_FILENAME )
			{
				int		n = 0;

				if (pSourceItem1->IsAskedForAnotherFile() && !pSourceItem2->IsAskedForAnotherFile())
				{
					n = 1;
				}
				if (!pSourceItem1->IsAskedForAnotherFile() && pSourceItem2->IsAskedForAnotherFile())
				{
					n = -1;
				}
			//	Always at the end, unless sort by source - then bIsA4AF always at the top
				if (n != 0)
				{
					iCompare = /*(iFileSortItem == DLCOL_PROGRESS) ? -n : n*/ n;
				}
				else
				{
#ifdef OLD_SOCKETS_ENABLED
					iCompare = Compare(pSourceItem1->GetSource(), pSourceItem2->GetSource(), iSourceSortItem, !bSortSourceCol2) * iSourceSortMod;
					if (bSortSourceCol2 && iCompare == 0)
					{
						iCompare = Compare(pSourceItem1->GetSource(), pSourceItem2->GetSource(), iSourceSortItem2) * iSourceSortMod2;
					}
#endif //OLD_SOCKETS_ENABLED
				}
			}
			else
			{
#ifdef OLD_SOCKETS_ENABLED
				iCompare = Compare(pSourceItem1->GetSource(), pSourceItem2->GetSource(), iSourceSortItem, !bSortSourceCol2) * iSourceSortMod;
				if (bSortSourceCol2 && iCompare == 0)
				{
					iCompare = Compare(pSourceItem1->GetSource(), pSourceItem2->GetSource(), iSourceSortItem2) * iSourceSortMod2;
				}
#endif //OLD_SOCKETS_ENABLED
			}
		}
	}

	return iCompare;

	EMULE_CATCH

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::SetStyle()
{
	EMULE_TRY

	SetExtendedStyle(LVS_EX_FULLROWSELECT);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::SortInit(int override)
{
	EMULE_TRY

	uint32	dwSortCode = m_dwParamSort;

//	Major override, wipe old sorting order...
	if (override)
	{
		DWORD		dwFilesSortCode = g_App.m_pPrefs->GetDownloadSortCol();
		DWORD		dwSourceSortCode = g_App.m_pPrefs->GetSrcSortCol1();
		bool		bUseSourceSort2 = g_App.m_pPrefs->DoUseSrcSortCol2();
		DWORD		dwSourceSortCode2 = bUseSourceSort2 ? g_App.m_pPrefs->GetSrcSortCol2() : MLC_DONTSORT;

		dwSortCode = (dwSourceSortCode2 << 24) | (dwSourceSortCode << 16) | dwFilesSortCode;
		m_bSortAscending = (dwFilesSortCode & MLC_SORTDESC) == 0;
		m_bSortSourcesAscending = (dwSourceSortCode & MLC_SORTDESC) == 0;
		m_iCurrentSortItem = dwFilesSortCode & MLC_COLUMNMASK;
		m_iSourceSortItem = dwSourceSortCode & MLC_COLUMNMASK;
		m_iSourceSortItem2 = bUseSourceSort2 ? (dwSourceSortCode2 & MLC_COLUMNMASK) : -1;

		if (bUseSourceSort2)
		{
			m_bSortSourcesAscending2 = (dwSourceSortCode2 & MLC_SORTDESC) == 0;
			m_byteSortAscending[m_iSourceSortItem2] =
				(m_byteSortAscending[m_iSourceSortItem2] & ~2) | ((m_bSortSourcesAscending2) ? 2 : 0);
		}
		m_byteSortAscending[m_iCurrentSortItem] =
			(m_byteSortAscending[m_iCurrentSortItem] & ~1) | ((m_bSortAscending) ? 1 : 0);
		m_byteSortAscending[m_iSourceSortItem] =
			(m_byteSortAscending[m_iSourceSortItem] & ~2) | ((m_bSortSourcesAscending) ? 2 : 0);

		SetSortArrow(m_iCurrentSortItem, m_bSortAscending);
	}
	SortItems(SortProc, dwSortCode);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::OnListModified(NMHDR *pNMHDR, LRESULT *pResult)
{
	NOPRM(pResult);
	EMULE_TRY

	NM_LISTVIEW *pNMListView = (NM_LISTVIEW*)pNMHDR;

//	This works because true is equal to 1 and false equal to 0
	BOOL notLast = pNMListView->iItem + 1 != GetItemCount();
	BOOL notFirst = pNMListView->iItem != 0;

	CRect rcViewRect;
	GetViewRect(&rcViewRect);
	CRect rcItemRect;
	GetItemRect(pNMListView->iItem,&rcItemRect,LVIR_BOUNDS);
	CRect rcIntersection;
	rcIntersection.IntersectRect(rcViewRect,rcItemRect);
	if (!rcIntersection.IsRectEmpty())
	{
		RedrawItems(pNMListView->iItem - notFirst, pNMListView->iItem + notLast);
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Compare() compares the two part files 'file1' and 'file2' by column 'lParamSort' and returns
//		-1, 0, or 1 for <, ==, or > respectively.
/*static*/ int CDownloadListCtrl::Compare(CPartFile *file1, CPartFile *file2, LPARAM lParamSortItem, WPARAM wParamSortMod)
{
	int		iCompare = 0;

	EMULE_TRY

	if (file1 == NULL || file2 == NULL)
		return 0;

	if (g_App.m_pPrefs->DoPausedStoppedLast())
	{
		EnumPartFileStatuses	eFileStatus;
		int	iRank1 = 0, iRank2 = 0;

		eFileStatus = file1->GetStatus();
		iRank1 |= (eFileStatus == PS_PAUSED) ? 1 : 0;
		iRank1 |= (eFileStatus == PS_STOPPED) ? 2 : 0;

		eFileStatus = file2->GetStatus();
		iRank2 |= (eFileStatus == PS_PAUSED) ? 1 : 0;
		iRank2 |= (eFileStatus == PS_STOPPED) ? 2 : 0;

		iCompare = iRank1 - iRank2;
		if (iCompare != 0)
			return iCompare;
	}

	switch (lParamSortItem)
	{
		case DLCOL_FILENAME:
		{
		//	Funny, but we might not have a filename yet..
		//	See below
			break;
		}
		case DLCOL_SIZE:
			iCompare = CompareInt64(file1->GetFileSize(), file2->GetFileSize());
			break;

		case DLCOL_TRANSFERRED:
			iCompare = CompareInt64(file1->GetTransferred(), file2->GetTransferred());
			break;

		case DLCOL_COMPLETED:
			iCompare = CompareInt64(file1->GetCompletedSize(), file2->GetCompletedSize());
			break;

		case DLCOL_SPEED:
			iCompare = CompareUnsigned(file1->GetDataRate(), file2->GetDataRate());
			if (iCompare == 0)
				iCompare = CompareUnsigned(file1->GetAvgDataRate(), file2->GetAvgDataRate());
			break;

		case DLCOL_PROGRESS:
		{
			double		dblComp = file1->GetPercentCompleted() - file2->GetPercentCompleted();

			if (dblComp > .00000000001)
				iCompare = 1;
			else if (dblComp < -.00000000001)
				iCompare = -1;
			break;
		}
		case DLCOL_NUMSOURCES:
			iCompare = file1->GetSourceCount() - file2->GetSourceCount();
			if (iCompare == 0)
				iCompare = file2->GetNotCurrentSourcesCount() - file1->GetNotCurrentSourcesCount();
			break;

		case DLCOL_PRIORITY:
			iCompare = file1->GetPriority() - file2->GetPriority();
			if (iCompare == 0)
				iCompare = static_cast<int>(file1->IsAutoPrioritized()) - static_cast<int>(file2->IsAutoPrioritized());
			break;

		case DLCOL_STATUS:
			iCompare = file1->GetPartFileStatusRang() - file2->GetPartFileStatusRang();
			break;

		case DLCOL_REMAINING:
			iCompare = CompareInt64( file1->GetFileSize() - file1->GetCompletedSize(),
				file2->GetFileSize() - file2->GetCompletedSize() );
			break;

		case DLCOL_REMAININGTIME:
		case DLCOL_ETA:
		{
			int		time1 = file1->GetTimeRemaining();
			int		time2 = file2->GetTimeRemaining();

			time1 = time1 == -1 ? INT_MAX : time1;	// -1 time indicates "unknown". Compare as if "never".
			time2 = time2 == -1 ? INT_MAX : time2;
			iCompare = (time1 > time2) ? 1 : ((time1 < time2) ? -1 : 0);

		//	If no remaining time sort by remaining average time asc.
			if (iCompare == 0)
			{
				time1 = file1->GetTimeRemaining(true);
				time2 = file2->GetTimeRemaining(true);

				time1 = time1 == -1 ? INT_MAX : time1;	// -1 time indicates "unknown". Compare as if "never".
				time2 = time2 == -1 ? INT_MAX : time2;

				iCompare = (time1 > time2) ? 1 : ((time1 < time2) ? -1 : 0);
			}
		//	If no remaining average time sort by remaining size asc. Leave completed files last...
			if (iCompare == 0)
			{
				EnumPartFileStatuses	eFileStatus = file1->GetStatus();

				time1 = ((eFileStatus == PS_COMPLETING) || (eFileStatus == PS_COMPLETE));
				eFileStatus = file2->GetStatus();
				time2 = ((eFileStatus == PS_COMPLETING) || (eFileStatus == PS_COMPLETE));

				if (!time1 && time2)
					iCompare = -1;
				else if (time1 && !time2)
					iCompare = 1;
				else if (!time1 && !time2)
					iCompare = CompareInt64( file1->GetFileSize() - file1->GetCompletedSize(),
						file2->GetFileSize() - file2->GetCompletedSize() );
			}
			break;
		}
		case DLCOL_LASTSEENCOMPLETE:
			if (file1->lastseencomplete > file2->lastseencomplete)
				iCompare = 1;
			else if (file1->lastseencomplete < file2->lastseencomplete)
				iCompare = -1;
			break;

		case DLCOL_LASTRECEIVED:
		{
			bool	bTransferred2 = (file2->GetTransferred() != 0);

			if (file1->GetTransferred() != 0)
			{
				if (bTransferred2)
				{
					CTime	ct1 = file1->GetLastDownTransfer();
					CTime	ct2 = file2->GetLastDownTransfer();

					if (ct1 > ct2)
						iCompare = 1;
					else if (ct1 < ct2)
						iCompare = -1;
				}
				else
					iCompare = 1;
			}
			else if (bTransferred2)
				iCompare = -1;
			break;
		}
		case DLCOL_CATEGORY:
		{
			CCat		*pCat1 = CCat::GetCatByID(file1->GetCatID());
			CCat		*pCat2 = CCat::GetCatByID(file2->GetCatID());
			CString		strFile1CatTitle = pCat1 == NULL ? _T("") : pCat1->GetTitle();
			CString		strFile2CatTitle = pCat2 == NULL ? _T("") : pCat2->GetTitle();

			if (strFile1CatTitle > strFile2CatTitle)
				iCompare = 1;
			else if (strFile1CatTitle < strFile2CatTitle)
				iCompare = -1;
			break;
		}
		case DLCOL_WAITED:
		{
			CTimeSpan	ts1 = CTime::GetCurrentTime() - file1->GetLastDownTransfer();
			CTimeSpan	ts2 = CTime::GetCurrentTime() - file2->GetLastDownTransfer();

			iCompare = (ts1 > ts2) ? 1 : ((ts1 < ts2) ? -1 : 0);
			break;
		}
		case DLCOL_AVGSPEED:
			iCompare = CompareUnsigned(file1->GetAvgDataRate(), file2->GetAvgDataRate());
			if (iCompare == 0)
				iCompare = CompareUnsigned(file1->GetDataRate(), file2->GetDataRate());
			break;

		case DLCOL_AVGREMTIME:
		case DLCOL_AVGETA:
		{
			int		time1 = file1->GetTimeRemaining(true);
			int		time2 = file2->GetTimeRemaining(true);

			time1 = time1 == -1 ? INT_MAX : time1;	// -1 time indicates "unknown". Compare as if "never".
			time2 = time2 == -1 ? INT_MAX : time2;
			iCompare = (time1 > time2) ? 1 : ((time1 < time2) ? -1 : 0);

		//	If no remaining average time sort by remaining average time asc.
			if (iCompare == 0)
			{
				time1 = file1->GetTimeRemaining();
				time2 = file2->GetTimeRemaining();

				time1 = time1 == -1 ? INT_MAX : time1;	// -1 time indicates "unknown". Compare as if "never".
				time2 = time2 == -1 ? INT_MAX : time2;

				iCompare = (time1 > time2) ? 1 : ((time1 < time2) ? -1 : 0);
			}
		//	If no remaining time sort by remaining size asc. Leave completed files last...
			if (iCompare == 0)
			{
				time1 = (file1->IsCompleting() || !file1->IsPartFile());
				time2 = (file2->IsCompleting() || !file2->IsPartFile());

				if (!time1 && time2)
					iCompare = -1;
				else if (time1 && !time2)
					iCompare = 1;
				else if (!time1 && !time2)
					iCompare = CompareInt64( file1->GetFileSize() - file1->GetCompletedSize(),
						file2->GetFileSize() - file2->GetCompletedSize() );
			}
			break;
		}
	}

	if (iCompare == 0)
	{
		iCompare = file1->CmpFileNames(file2->GetFileName());
		if (lParamSortItem == DLCOL_FILENAME)
			iCompare *= wParamSortMod;
	}
	else
	{
		iCompare *= wParamSortMod;
	}

	EMULE_CATCH

	return iCompare;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Compare() compares the two sources 'pClient1' and 'pClient2' by column 'lParamSort' and returns
//		-1, 0, or 1 for <, ==, or > respectively.
/*static*/ int CDownloadListCtrl::Compare( CUpDownClient *pClient1,CUpDownClient *pClient2, LPARAM lParamSort,
										   bool bDisambiguate/*=true*/ )
{
	int		iCompare = 0;

	EMULE_TRY

	if (pClient1==NULL||pClient2==NULL)
		return 0;

	switch (lParamSort)
	{
		case DLCOL_FILENAME:
			break;	//	See below

		case DLCOL_TRANSFERRED:
			if (!g_App.m_pPrefs->IsTransferredOnCompleted())
				iCompare = CompareUnsigned(pClient1->GetTransferredDown(), pClient2->GetTransferredDown());
			break;

		case DLCOL_COMPLETED:
			if (g_App.m_pPrefs->IsTransferredOnCompleted())
			    iCompare = CompareUnsigned(pClient1->GetTransferredDown(), pClient2->GetTransferredDown());
			break;

		case DLCOL_SPEED:
			iCompare = CompareUnsigned(pClient1->GetDownloadDataRate(), pClient2->GetDownloadDataRate());
			break;

		case DLCOL_PROGRESS:
			iCompare = (pClient1->GetAvailablePartCount() - pClient2->GetAvailablePartCount());
			break;

		case DLCOL_NUMSOURCES:
			if ((iCompare = (pClient1->GetClientSoft() - pClient2->GetClientSoft())) == 0)
			{
				if ((iCompare = (pClient2->GetVersion() - pClient1->GetVersion())) == 0)
				{
					if (!pClient1->IsModStringEmpty())
						iCompare = (!pClient2->IsModStringEmpty()) ?
							_tcsicmp(pClient1->GetModString(), pClient2->GetModString()) : -1;
					else if (!pClient2->IsModStringEmpty())
						iCompare = 1;
				}
			}
			break;

		case DLCOL_PRIORITY:
		{
			bool	bShowQRForClient1 = pClient1->GetDownloadState() == DS_ONQUEUE || pClient1->GetDownloadState() == DS_LOWID_ON_OTHER_SERVER;
			bool	bShowQRForClient2 = pClient2->GetDownloadState() == DS_ONQUEUE || pClient2->GetDownloadState() == DS_LOWID_ON_OTHER_SERVER;

			if (!bShowQRForClient1 || !bShowQRForClient2)
			{
				if (pClient1->GetClientSoft() == SO_UNKNOWN || pClient2->GetClientSoft() == SO_UNKNOWN)
				{
					iCompare = (pClient1->GetClientSoft() - pClient2->GetClientSoft());
				}
				else if (pClient1->GetDownloadState()==DS_DOWNLOADING || pClient2->GetDownloadState()==DS_DOWNLOADING)
				{
					iCompare = (pClient1->GetDownloadState()==DS_DOWNLOADING)?((pClient2->GetDownloadState()==DS_DOWNLOADING)?0:-1):2;
				}
				else
				{
					iCompare = (bShowQRForClient1) ? -1 : ((bShowQRForClient2) ? 1 : 0);
				}
			}
			else if (pClient1->IsEmuleClient() && pClient2->IsEmuleClient())
			{
				if (!pClient1->IsRemoteQueueFull())
					iCompare = (!pClient2->IsRemoteQueueFull()) ? (pClient1->GetRemoteQueueRank() - pClient2->GetRemoteQueueRank()) : -1;
				else
					iCompare = (pClient2->IsRemoteQueueFull()) ? 0 : 1;
			}
			else
				iCompare = (pClient1->IsEmuleClient()) ? -1:((pClient2->IsEmuleClient()) ? 1:0);
			break;
		}
		case DLCOL_STATUS:
			iCompare = pClient1->GetDownloadStateAsString().Compare(pClient2->GetDownloadStateAsString());
			break;

		case DLCOL_ULDLRATIO:
		case DLCOL_QLRATING:
		{
			if (pClient1->GetClientSoft() == SO_UNKNOWN || pClient2->GetClientSoft() == SO_UNKNOWN)
			{
				iCompare = (pClient1->GetClientSoft() - pClient2->GetClientSoft());
			}
			else
			{
				if (pClient1->m_pCredits != NULL)
				{
					if (pClient2->m_pCredits != NULL)
					{
						if (lParamSort == DLCOL_ULDLRATIO)
							iCompare = (pClient1->GetRemoteBaseModifier() < pClient2->GetRemoteBaseModifier()) ? -1 : 1;
						else
							iCompare = pClient1->GetRemoteRatio() - pClient2->GetRemoteRatio();
					}
					else
					{
						iCompare = 1;
					}
				}
				else if (pClient2->m_pCredits != NULL)
				{
					iCompare = -1;
				}
			}
			break;
		}
		case DLCOL_LASTSEENCOMPLETE:
		case DLCOL_LASTRECEIVED:
		case DLCOL_CATEGORY:
			break;

		case DLCOL_WAITED:
		{
			uint32	dwWaited1, dwWaited2;

			if ((pClient1->GetDownloadState() == DS_ONQUEUE) || (pClient1->GetDownloadState() == DS_LOWID_ON_OTHER_SERVER))
				dwWaited1 = pClient1->GetDLQueueWaitTime();
			else
				dwWaited1 = INT_MAX;
			if ((pClient2->GetDownloadState() == DS_ONQUEUE) || (pClient2->GetDownloadState() == DS_LOWID_ON_OTHER_SERVER))
				dwWaited2 = pClient2->GetDLQueueWaitTime();
			else
				dwWaited2 = INT_MAX;
			iCompare = ((dwWaited1 < dwWaited2) ? -1 : ((dwWaited1 > dwWaited2) ? 1 : 0));
			break;
		}
		case DLCOL_AVGSPEED:
		case DLCOL_AVGREMTIME:
		case DLCOL_ETA:
		case DLCOL_AVGETA:
			break;

		case DLCOL_REMAINING:
		{
			uint32	dwDummy;

			iCompare = pClient1->GetRemainingSizeForCurrentPart(&dwDummy) - pClient2->GetRemainingSizeForCurrentPart(&dwDummy);
			break;
		}
		case DLCOL_REMAININGTIME:
			iCompare = pClient1->GetRemainingTimeForCurrentPart() - pClient2->GetRemainingTimeForCurrentPart();
			break;
	}

	if (iCompare == 0 && bDisambiguate)
	{
		CString		strName1, strName2;

		try
		{
			strName1 = pClient1->GetUserName();
		}
		catch(...)
		{
			strName1.Empty();
		}
		try
		{
			strName2 = pClient2->GetUserName();
		}
		catch(...)
		{
			strName2.Empty();
		}
		if (strName1.IsEmpty() || strName2.IsEmpty())
			iCompare = (strName1.IsEmpty()) ? ((strName2.IsEmpty()) ? 0 : 1) : -1;
		else
			iCompare = _tcsicmp(strName1, strName2);
	}
	EMULE_CATCH
	return iCompare;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	EMULE_TRY

//	Reset selection in UL, CL or QL list
	POSITION posSelClient = NULL;

	switch (g_App.m_pMDlg->m_wndTransfer.m_nActiveWnd)
	{
		case MPW_UPLOADLIST:
			posSelClient = g_App.m_pMDlg->m_wndTransfer.m_ctlUploadList.GetFirstSelectedItemPosition();
			while (posSelClient != NULL)
			{
				int iSelClientListIndex = g_App.m_pMDlg->m_wndTransfer.m_ctlUploadList.GetNextSelectedItem(posSelClient);
				g_App.m_pMDlg->m_wndTransfer.m_ctlUploadList.SetItemState(iSelClientListIndex, 0, LVIS_SELECTED);
			}
			break;

		case MPW_UPLOADQUEUELIST:
			posSelClient = g_App.m_pMDlg->m_wndTransfer.m_ctlQueueList.GetFirstSelectedItemPosition();
			while (posSelClient != NULL)
			{
				int iSelClientListIndex = g_App.m_pMDlg->m_wndTransfer.m_ctlQueueList.GetNextSelectedItem(posSelClient);
				g_App.m_pMDlg->m_wndTransfer.m_ctlQueueList.SetItemState(iSelClientListIndex, 0, LVIS_SELECTED);
			}
			break;

		case MPW_UPLOADCLIENTLIST:
			posSelClient = g_App.m_pMDlg->m_wndTransfer.m_ctlClientList.GetFirstSelectedItemPosition();
			while (posSelClient != NULL)
			{
				int iSelClientListIndex = g_App.m_pMDlg->m_wndTransfer.m_ctlClientList.GetNextSelectedItem(posSelClient);
				g_App.m_pMDlg->m_wndTransfer.m_ctlClientList.SetItemState(iSelClientListIndex, 0, LVIS_SELECTED);
			}
			break;
	}
	RefreshInfo();

	DWORD pos = GetMessagePos();
	CPoint pt((int)(short)LOWORD(pos), (int)(short)HIWORD(pos));
	ScreenToClient(&pt);

	posSelClient = GetFirstSelectedItemPosition();

	if (posSelClient != NULL)
	{
		int		iItemIndex = GetNextSelectedItem(posSelClient);
		CRect	r;

		GetItemRect(iItemIndex, &r, LVIR_BOUNDS);

		if (r.PtInRect(pt))
		{
			CMuleCtrlItem	*pListItem = ListGetItemAt(iItemIndex);
			CPartFileDLItem	*pFileItem = dynamic_cast<CPartFileDLItem*>(pListItem);

		//	Process only if it is a file item
			if (pFileItem != NULL)
			{
				bool	pProcessed = false;
				CRect	rTestedArea(r.left + 3, r.top + 1, r.left + 18, r.top + 16);

			//	[+]/[-]
				if (rTestedArea.PtInRect(pt))
				{
					OnItemActivate(pNMHDR, pResult);
				}
				rTestedArea.SetRect(r.left + OFFSET_PLUSMINUS + 3, r.top, r.left + OFFSET_PLUSMINUS + 19, r.bottom);
			//	File Status
				if (g_App.m_pPrefs->ShowFullFileStatusIcons())
				{
					if (rTestedArea.PtInRect(pt))
					{
						CPartFile *pPartFile = pListItem->GetFile();

						if (pPartFile != NULL)
						{
							switch (pPartFile->GetStatus())
							{
								case PS_PAUSED:
								case PS_STOPPED:
									pPartFile->ResumeFile();
									break;
								case PS_COMPLETE:
									g_App.m_pDownloadList->ClearCompleted(pPartFile);
									break;
								case PS_ERROR:
								{
									pPartFile->LoadPartFile(pPartFile->GetTempDir(), pPartFile->GetPartMetFileName());
									if (pPartFile->GetRawStatus() == PS_READY)
										g_App.m_pSharedFilesList->SafeAddKnownFile(pPartFile);
									break;
								}
							}
							pProcessed = true;
						}
					}
					rTestedArea.OffsetRect(OFFSET_STATUSICON, 0);
				}
			//	File Type Icon
				if (!pProcessed && g_App.m_pPrefs->ShowFileTypeIcon())
				{
					if (rTestedArea.PtInRect(pt))
					{
						CPartFile	*pPartFile = pListItem->GetFile();

						if (pPartFile != NULL)
						{
							CFileDetails FileDetailsDlg(IDS_FD_TITLE, pPartFile, this, 0);
							FileDetailsDlg.DoModal();
						}
						pProcessed = true;
					}
					rTestedArea.OffsetRect(OFFSET_FILETYPEICON, 0);
				}
			//	Rating
				if (!pProcessed)
				{
					rTestedArea.SetRect(rTestedArea.left - 1, rTestedArea.top + 2, rTestedArea.left + 7, r.bottom - 2);
					if (rTestedArea.PtInRect(pt))
					{
						CPartFile	*pPartFile = pListItem->GetFile();

						if (pPartFile != NULL && (pPartFile->HasComment() || pPartFile->HasRating()))
						{
							CCommentDialogLst CommentDlg(pPartFile);
							CommentDlg.DoModal();
						}
					}
				}
			}
		}
	}

	*pResult = 0;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::OnNMDblclkDownloadlist(NMHDR *pNMHDR, LRESULT *pResult)
{
	NOPRM(pNMHDR);
	EMULE_TRY

	POSITION	posSelClient = GetFirstSelectedItemPosition();

	if (posSelClient != NULL)
	{
		CMuleCtrlItem		*pItem = reinterpret_cast<CMuleCtrlItem*>(GetItemData(GetNextSelectedItem(posSelClient)));
		CPartFileDLItem	*pFileItem = dynamic_cast<CPartFileDLItem*>(pItem);
		CSourceDLItem	*pSourceItem = dynamic_cast<CSourceDLItem*>(pItem);

	// 	Doubleclick opens completed files
		if (pFileItem != NULL)
		{
			CPartFile		*pPartFile = pFileItem->GetFile();

			if (pPartFile->GetStatus() == PS_COMPLETE)
				ShellOpenFile(pPartFile->GetFilePath());
		}
		else if (pSourceItem != NULL)
		{
#ifdef OLD_SOCKETS_ENABLED
			CUpDownClient		*pSource = pSourceItem->GetSource();

			if (g_App.m_pPrefs->GetDetailsOnClick())
			{
				CClientDetails		dialog(IDS_CD_TITLE, pSource, this, 0);

				dialog.DoModal();
			}
			else
			{
				g_App.m_pMDlg->m_wndChat.StartSession(pSource);
			}
#endif //OLD_SOCKETS_ENABLED
		}
	}

	*pResult = 0;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::OnSize(UINT nType, int cx, int cy)
{
	CListCtrl::OnSize(nType, cx, cy);
	Invalidate();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::RestartWaitingDownloads()
{
	EMULE_TRY

	CTypedPtrList<CPtrList, CPartFile*>		selectedList;

	int			iNumItems = GetItemCount();

//	Get initial sources from server if we don´t have our MaxSourcesPerFile Limit reached
	for (int i = 0; i < iNumItems; i++)
	{
		CMuleCtrlItem		*pItem = ListGetItemAt(i);
		CPartFileDLItem		*pFileItem = dynamic_cast<CPartFileDLItem*>(pItem);

		if ( pFileItem != NULL && !pFileItem->GetFile()->IsPaused()
		     && pFileItem->GetFile()->GetSourceCount() <= g_App.m_pPrefs->GetMaxSourcePerFile() )
		{
			selectedList.AddTail(pFileItem->GetFile());
	 	}
	}

	for (POSITION pos = selectedList.GetHeadPosition(); pos != NULL; )
		selectedList.GetNext(pos)->GetSourcesAfterServerConnect();

	AddLogLine(LOG_FL_SBAR, IDS_NEWSERVERCONNECT, g_App.m_pPrefs->GetMaxSourcePerFile());

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	EMULE_TRY

	RefreshInfo();

	if (nChar == VK_CONTROL)
	{
		SetSortArrow(m_iCurrentSortItem, m_bSortAscending);
	}

	CMuleListCtrl::OnKeyUp(nChar, nRepCnt, nFlags);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::ShowAllUploadingSources()
{
	EMULE_TRY

	CDownloadList::PartFileItemVector	   *pvecFileItems = g_App.m_pDownloadList->GetFileItems();

	for (uint32 i = 0; i < pvecFileItems->size(); i++)
	{
		CPartFileDLItem		*pFileItem = (*pvecFileItems)[i];

		if (pFileItem != NULL)
		{
			pFileItem->m_bSrcsAreVisible = true;
			pFileItem->FilterAllSources();
		//	Unfilter just uploading sources
			pFileItem->m_bShowUploadingSources = true;
			UpdateSourceItems();
		}
	}

	delete pvecFileItems;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	EMULE_TRY

	bool	bHandled = false;

	POSITION		posSelClient = GetFirstSelectedItemPosition();
	int				iClientListIndex = GetNextSelectedItem(posSelClient);

	switch (nChar)
	{
		case VK_ADD:
		{
		//	Update header arrow only on the first press and don't on repeated events
			if ((nFlags & 0x4000) == 0)
			{
				if (::GetAsyncKeyState(VK_CONTROL) >= 0)
				{
					ExpandCollapseItem(iClientListIndex, EXPAND_ONLY, true);
					bHandled = true;
				}
			}
			break;
		}
		case VK_SUBTRACT:
		{
			ExpandCollapseItem(iClientListIndex, COLLAPSE_ONLY, true);
			bHandled = true;
			break;
		}
		case VK_CONTROL:
		{
		//	Update header arrow only on the first press and don't on repeated events
			if ((nFlags & 0x4000) == 0)
			{
				if (m_iSourceSortItem2 == -1)
					SetSortArrow(m_iSourceSortItem, m_bSortSourcesAscending);
				else
				{
					SetSortArrow(m_iSourceSortItem, m_bSortSourcesAscending, 1);
					SetSortArrow(m_iSourceSortItem2, m_bSortSourcesAscending2, 2);
				}
			}
			break;
		}
	}

	if (!bHandled)
		CMuleListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::OnKillFocus(CWnd *pNewWnd)
{
//	If Ctrl is pressed return back the list header arrow
	if (::GetAsyncKeyState(VK_CONTROL) < 0)
	{
		SetSortArrow(m_iCurrentSortItem, m_bSortAscending);
	}

	CWnd::OnKillFocus(pNewWnd);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::RefreshInfo(void)
{
	EMULE_TRY

	if (GetFocus() == this)
	{
		POSITION	posSelClient = GetFirstSelectedItemPosition();
		if (posSelClient == NULL)
		{
	 		g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.SetList(INFOLISTTYPE_NONE);
		}
		else
		{
			CMuleCtrlItem		*pItem = reinterpret_cast<CMuleCtrlItem*>(GetItemData(GetNextSelectedItem(posSelClient)));
			CPartFileDLItem	*pFileItem = dynamic_cast<CPartFileDLItem*>(pItem);
			CSourceDLItem	*pSourceItem = dynamic_cast<CSourceDLItem*>(pItem);

			if (pFileItem != NULL)
			{
				const CPartFile		*pPartFile = pFileItem->GetFile();

	 			g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.SetList(INFOLISTTYPE_FILE,const_cast<CPartFile*>(pPartFile));
			}
			else if (pSourceItem != NULL)
			{
#ifdef OLD_SOCKETS_ENABLED
				const CUpDownClient	*pSource = pSourceItem->GetSource();

	 			g_App.m_pMDlg->m_wndTransfer.m_ctlInfoList.SetList(INFOLISTTYPE_SOURCE,const_cast<CUpDownClient*>(pSource));
#endif //OLD_SOCKETS_ENABLED
			}
		}
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	ShowFilesCount() updates the display of the number of downloading files.
void CDownloadListCtrl::ShowFilesCount()
{
	g_App.m_pMDlg->m_wndTransfer.UpdateDownloadHeader();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::ShowSelectedFileOrUserDetails()
{
	EMULE_TRY

	POINT		point;

	::GetCursorPos(&point);

	CPoint		p = point;

	ScreenToClient(&p);

	int			it = HitTest(p);

	SetSelectionMark(it);

	if (it == -1)
		return;

	CMuleCtrlItem		*pItem = reinterpret_cast<CMuleCtrlItem*>(GetItemData(GetSelectionMark()));
	CPartFileDLItem		*pFileItem = dynamic_cast<CPartFileDLItem*>(pItem);
	CSourceDLItem		*pSourceItem = dynamic_cast<CSourceDLItem*>(pItem);

	if (pFileItem != NULL)
	{
		CPartFile		*pPartFile = pFileItem->GetFile();
		CFileDetails		dialog(IDS_FD_TITLE, pPartFile, this, 1);

		dialog.DoModal();
	}
	else if (pSourceItem != NULL)
	{
#ifdef OLD_SOCKETS_ENABLED
		CUpDownClient		*pSource = pSourceItem->GetSource();
		CClientDetails		dialog(IDS_CD_TITLE, pSource, this, 0);
		dialog.DoModal();
#endif //OLD_SOCKETS_ENABLED
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	ChangeCategory() changes the currently displayed category given a category ID.
void CDownloadListCtrl::ChangeCategoryByID(EnumCategories eNewCatID)
{
	int		iNewCatIndex = 0;

	iNewCatIndex = CCat::GetCatIndexByID(eNewCatID);

	if (iNewCatIndex == -1)
		iNewCatIndex = m_iCurTabIndex;

	ChangeCategoryByIndex(iNewCatIndex);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	ChangeCategoryByIndex() changes the currently displayed category given a category (i.e. tab) index.
void CDownloadListCtrl::ChangeCategoryByIndex(int iNewCatIdx)
{
	SetRedraw(FALSE);

	m_iCurTabIndex = iNewCatIdx;
	m_eCurTabCat = CCat::GetCatIDByIndex(iNewCatIdx);

//	Remove all displayed files with a different cat and show the correct ones
	CDownloadList::PartFileItemVector	   *pvecPartFileItems = g_App.m_pDownloadList->GetFileItems();
	uint32									iNumFiles = pvecPartFileItems->size();

	for (uint32 i = 0; i < iNumFiles; i++)
	{
		CPartFileDLItem		*pFileItem = (*pvecPartFileItems)[i];

		if (!CCat::FileBelongsToGivenCat(pFileItem->GetFile(),m_eCurTabCat))
			HideFileItem(pFileItem);
		else
			ShowFileItem(pFileItem);
	}
	delete pvecPartFileItems;

	SortInit(0);
	SetRedraw(TRUE);

	ShowFilesCount();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::HideSourceItem(CSourceDLItem *pSourceItem)
{
//	Find entry in list and update object
	int		iSourceIndex = ListGetSourceItemIndex(pSourceItem);

	if (iSourceIndex != -1)
	{
		pSourceItem->SetVisibility(false);
		DeleteItem(iSourceIndex);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::HideFileItem(CPartFileDLItem *pFileItem)
{
	EMULE_TRY

	HideSources(pFileItem->GetFile());

	int		iFileIndex = ListGetFileItemIndex(pFileItem);

	if (iFileIndex != -1)
	{
		pFileItem->SetVisibility(false);
		DeleteItem(iFileIndex);
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::ShowFileItem(CPartFileDLItem *pFileItem)
{
	EMULE_TRY

	int		iFileIndex = ListGetFileItemIndex(pFileItem);

	if (iFileIndex == -1)
	{
		ListInsertFileItem(pFileItem,GetItemCount());
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::ShowSourceItem(CSourceDLItem *pSourceItem)
{
	EMULE_TRY

	int		iSourceIndex = ListGetSourceItemIndex(pSourceItem);

//	If the source item isn't already in the list...
	if (iSourceIndex == -1)
	{
		CPartFileDLItem	   *pParentFileItem = pSourceItem->GetParentFileItem();
	//	Find the position of the next file item or the end of the list
		int					iParentFileIndex = ListGetFileItemIndex(pParentFileItem);

	// if parent file exists in the list then insert(attach) the sources
		if (iParentFileIndex > -1)
		{
			int				iInsertionIndex = iParentFileIndex;

		//	Until we hit the end of the list or the next item is a file item...
			do
			{
				iInsertionIndex++;
			}
			while ( iInsertionIndex < GetItemCount()
					&& typeid(*ListGetItemAt(iInsertionIndex)) != typeid(CPartFileDLItem));

			ListInsertSourceItem(pSourceItem,iInsertionIndex);
		}
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVDISPINFO	*pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

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
			CMuleCtrlItem		*pItem = reinterpret_cast<CMuleCtrlItem*>(pDispInfo->item.lParam);
			CPartFileDLItem		*pFileItem = dynamic_cast<CPartFileDLItem*>(pItem);

			if (pFileItem != NULL && pFileItem->GetFile() != NULL)
			{
				switch (pDispInfo->item.iSubItem)
				{
					case DLCOL_FILENAME:
						_tcsncpy(pDispInfo->item.pszText, pFileItem->GetFile()->GetFileName(), pDispInfo->item.cchTextMax);
						pDispInfo->item.pszText[pDispInfo->item.cchTextMax - 1] = _T('\0');
						break;
					default:
						pDispInfo->item.pszText[0] = _T('\0');
						break;
				}
			}
			else
			{
#ifdef OLD_SOCKETS_ENABLED
				CSourceDLItem	*pSourceItem = dynamic_cast<CSourceDLItem*>(pItem);

				if (pSourceItem != NULL && pSourceItem->GetParentFile() != NULL)
				{
					switch (pDispInfo->item.iSubItem)
					{
						case DLCOL_FILENAME:
							_tcsncpy(pDispInfo->item.pszText, pSourceItem->GetSource()->GetUserName(), pDispInfo->item.cchTextMax);
							pDispInfo->item.pszText[pDispInfo->item.cchTextMax - 1] = _T('\0');
							break;

						case DLCOL_NUMSOURCES:
							_tcsncpy(pDispInfo->item.pszText, pSourceItem->GetSource()->GetFullSoftVersionString(), pDispInfo->item.cchTextMax);
							pDispInfo->item.pszText[pDispInfo->item.cchTextMax - 1] = _T('\0');
							break;

						default:
							pDispInfo->item.pszText[0] = _T('\0');
							break;
					}
				}
#else
				pDispInfo->item.pszText[0] = _T('\0');
#endif //OLD_SOCKETS_ENABLED
			}
		}
		else if (pDispInfo->item.cchTextMax != 0)
			pDispInfo->item.pszText[0] = _T('\0');
	}
	*pResult = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	List methods
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::ListInsertFileItem(CPartFileDLItem *pFileItem, int iPos)
{
	InsertItem(LVIF_PARAM | LVIF_TEXT, iPos, LPSTR_TEXTCALLBACK, 0, 0, 0, reinterpret_cast<LPARAM>(pFileItem));
	pFileItem->SetVisibility(true);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::ListInsertSourceItem(CSourceDLItem *pSourceItem, int iPos)
{
	InsertItem(LVIF_PARAM | LVIF_TEXT, iPos, LPSTR_TEXTCALLBACK, 0, 0, 0, reinterpret_cast<LPARAM>(pSourceItem));
	pSourceItem->SetVisibility(true);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	ListGetFileItemIndex() returns the list index of 'pFileItem' or -1 if it isn't in the list.
int CDownloadListCtrl::ListGetFileItemIndex(CPartFileDLItem *pFileItem)
{
	LVFINDINFO		find;

	find.flags = LVFI_PARAM;
	find.lParam = reinterpret_cast<LPARAM>(pFileItem);

	return FindItem(&find);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	ListGetSourceItemIndex() returns the list index of 'pSourceItem' or -1 if it isn't in the list.
int CDownloadListCtrl::ListGetSourceItemIndex(CSourceDLItem *pSourceItem)
{
	LVFINDINFO		find;

	find.flags = LVFI_PARAM;
	find.lParam = reinterpret_cast<LPARAM>(pSourceItem);

	return FindItem(&find);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	ListGetItemAt() returns the list item at index 'iIndex'.
CMuleCtrlItem *CDownloadListCtrl::ListGetItemAt(int iIndex)
{
	return reinterpret_cast<CMuleCtrlItem*>(GetItemData(iIndex));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	PostRefreshMessage() puts a message into the list control's message queue telling it it needs to
//	refresh dirty files and sources and then immediately returns.
void CDownloadListCtrl::PostRefreshMessage()
{
	if (::IsWindow(GetSafeHwnd()) && g_App.m_app_state != CEmuleApp::APP_STATE_SHUTTINGDOWN)
	{
		MSG		msg;

	//	If there's no refresh message already in the message queue... (don't want to flood it)
		if (!::PeekMessage(&msg, m_hWnd, WM_DL_REFRESH, WM_DL_REFRESH, PM_NOREMOVE))
		{
			PostMessage(WM_DL_REFRESH, 0, 0);
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	FilterNoSources() turns off filtering for all source types.
void CDownloadListCtrl::FilterNoSources()
{
	m_bShowUploadingSources = true;
	m_bShowOnQueueSources = true;
	m_bShowFullQueueSources = true;
	m_bShowConnectedSources = true;
	m_bShowConnectingSources = true;
	m_bShowNNPSources = true;
	m_bShowWaitForFileReqSources = true;
	m_bShowLowToLowIDSources = true;
	m_bShowLowIDOnOtherSrvSources = true;
	m_bShowBannedSources = true;
	m_bShowErrorSources = true;
	m_bShowA4AFSources = true;
	m_bShowUnknownSources = true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	FilterAllSources() turns on filtering for all source types.
void CDownloadListCtrl::FilterAllSources()
{
	m_bShowUploadingSources = false;
	m_bShowOnQueueSources = false;
	m_bShowFullQueueSources = false;
	m_bShowConnectedSources = false;
	m_bShowConnectingSources = false;
	m_bShowNNPSources = false;
	m_bShowWaitForFileReqSources = false;
	m_bShowLowToLowIDSources = false;
	m_bShowLowIDOnOtherSrvSources = false;
	m_bShowBannedSources = false;
	m_bShowErrorSources = false;
	m_bShowA4AFSources = false;
	m_bShowUnknownSources = false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CDownloadListCtrl::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN)
	{
		int			iMessage = 0;
		short		nCode = GetCodeFromPressedKeys(pMsg);

		if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_DL_DEFSORT))
		{
			if (g_App.m_pPrefs->DoUseSort())
				SortInit(DL_OVERRIDESORT);
		}
		else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_DL_CLEARALL))
		{
			iMessage = MP_CLEARALLCOMPLETED;
		}
		else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_DL_SHOWALL))
		{
			ShowAllUploadingSources();
		}
		else
		{
			POSITION	posSelClient = GetFirstSelectedItemPosition();

			if (posSelClient != NULL)
			{
				CMuleCtrlItem	   *pItem = reinterpret_cast<CMuleCtrlItem*>(GetItemData(GetNextSelectedItem(posSelClient)));
				CPartFileDLItem	   *pFileItem = dynamic_cast<CPartFileDLItem*>(pItem);

			//	If the selected item is a file...
				if (pFileItem != NULL)
				{
					CPartFile			   *pPartFile = pFileItem->GetFile();
					EnumPartFileStatuses	eFileStatus = pPartFile->GetStatus();
					bool					bJustOne  = (GetSelectedCount() == 1);
					bool					bFileDone = (eFileStatus == PS_COMPLETE || eFileStatus == PS_COMPLETING);

					if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_DL_CANCEL))
					{
						if (!bFileDone)
							iMessage = MP_CANCEL;
					}
					else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_DL_STOP))
					{
						if ((eFileStatus != PS_STOPPED) && (eFileStatus != PS_ERROR) && !bFileDone)
							iMessage = MP_STOP;
					}
					else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_DL_PAUSE))
					{
						if ((eFileStatus != PS_PAUSED) && (eFileStatus != PS_STOPPED) && (eFileStatus != PS_ERROR) && !bFileDone)
							iMessage = MP_PAUSE;
					}
					else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_DL_RESUME))
					{
						if ((eFileStatus == PS_PAUSED || eFileStatus == PS_STOPPED))
							iMessage = MP_RESUME;
					}
					else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_FILE_OPEN))
					{
						if (bJustOne && (eFileStatus == PS_COMPLETE))
							iMessage = MP_OPEN;
					}
					else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_FILE_OPENDIR))
					{
						if (bJustOne && (eFileStatus == PS_COMPLETE))
							iMessage = MP_OPENFOLDER;
					}
					else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_DL_PREVIEW))
					{
						if (bJustOne && pPartFile->PreviewAvailable())
							iMessage = MP_PREVIEW;
					}
					else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_FILE_RENAME))
					{
						if (bJustOne && !bFileDone)
							iMessage = MP_RENAME;
					}
					else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_FILE_COMMENTS))
					{
						if (bJustOne && (pPartFile->HasComment() || pPartFile->HasRating()))
							iMessage = MP_VIEWFILECOMMENTS;
					}
					else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_FILE_DETAILS))
					{
						if (bJustOne)
							iMessage = MP_METINFO;
					}
					else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_DL_FD_SOURCES))
					{
						if (bJustOne)
							iMessage = MP_METINFOSOURCES;
					}
					else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_DL_CLEAR))
					{
						iMessage = MP_CLEARCOMPLETED;
					}
					else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_DL_PREALLOC))
					{
						if (bJustOne && !pPartFile->IsLastBlockComplete() && !pPartFile->IsPreallocated())
							iMessage = MP_PREALLOCATE;
					}
					else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_DL_A4AF))
					{
						if (bJustOne && !bFileDone)
							iMessage = MP_ALL_A4AF_TO_HERE;
					}
					else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_DL_A4AFAUTO))
					{
						if (bJustOne && !bFileDone)
							iMessage = MP_ALL_A4AF_AUTO;
					}
					else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_DL_A4AFOTHER))
					{
						if (bJustOne && !bFileDone)
							iMessage = MP_ALL_A4AF_TO_OTHER;
					}
					else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_LINK))
						iMessage = MP_GETED2KLINK;
					else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_LINK_HTML))
						iMessage = MP_GETHTMLED2KLINK;
					else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_LINK_SOURCE))
						iMessage = MP_GETSOURCEED2KLINK;
					else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_LINK_HASH))
						iMessage = MP_GETHASH;
					else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_FILE_NAMECLEANUP))
					{
						if (!bJustOne || !bFileDone)
							iMessage = MP_DOCLEANUP;
					}
					else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_DL_A4AFSAMECAT))
					{
						if (bJustOne && !bFileDone)
							iMessage = MP_ALL_A4AF_SAMECAT;
					}
				}
			//	If the selected item is a source...
				else
				{
					CSourceDLItem	*pSourceItem = dynamic_cast<CSourceDLItem*>(pItem);

					if (pSourceItem != NULL)
						iMessage = GetClientListActionFromShortcutCode(nCode, pSourceItem->GetSource());
				}
			}
		}
		if (iMessage > 0)
		{
			PostMessage(WM_COMMAND, static_cast<WPARAM>(iMessage));
			return TRUE;
		}
	}

	return CMuleListCtrl::PreTranslateMessage(pMsg);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	AutoSetSourceFilters() automatically sets the source filters for a file based on its current state.
//	It is used for the "smart filtering" feature.
void CDownloadListCtrl::AutoSetSourceFilters(CPartFileDLItem *pFileItem)
{
	CPartFile			   *pPartFile = pFileItem->GetFile();
	EnumPartFileStatuses	eStatus = static_cast<_EnumPartFileStatuses>(pPartFile->GetPartFileStatusID());

//	If the file is downloading, show only uploading sources.
	if (eStatus == PS_DOWNLOADING)
	{
		pFileItem->FilterAllSources();
		pFileItem->m_bShowUploadingSources = true;
		if (g_App.m_pPrefs->GetSmartFilterShowOnQueue())
		{	//additionaly show "on queue" sources if enabled
			pFileItem->m_bShowOnQueueSources = true;
		}
	}
//	If the file is waiting, show only the sources whose upload queue we're on...
	else if (eStatus == PS_WAITINGFORSOURCE || eStatus == PS_STALLED)
	{
		pFileItem->FilterAllSources();
	//	If the file has no "on queue" sources, show only connecting/asking sources
		if (pPartFile->GetOnQueueSrcCount() == 0)
		{
			pFileItem->m_bShowConnectingSources = true;
			pFileItem->m_bShowConnectedSources = true;
		}
		else
		{
			pFileItem->m_bShowOnQueueSources = true;
		}
	}
	else
	{
		pFileItem->FilterNoSources();
	}
	UpdateSourceItems(pFileItem);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadListCtrl::ResetSourceFiltersForAllFiles()
{
	CDownloadList::PartFileItemVector	*pvecFileItems = g_App.m_pDownloadList->GetFileItems();

	for (uint32 i = 0; i < pvecFileItems->size(); i++)
	{
		(*pvecFileItems)[i]->FilterNoSources();
		UpdateFileItem((*pvecFileItems)[i]);
	}

	delete pvecFileItems;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
