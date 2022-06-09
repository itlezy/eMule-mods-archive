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
#include "server.h"
#include "SearchListCtrl.h"
#include "SharedFileList.h"
#include "otherfunctions.h"
#include "FakeCheck.h"
#include "TitleMenu.h"
#include "MemDC.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CSearchListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CSearchListCtrl, CMuleListCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CSearchListCtrl::CSearchListCtrl()
{
	m_pSearchList = NULL;
	m_dwResultsID = 0;
	SetGeneralPurposeFind(true);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchListCtrl::Init(CSearchList *pSearchList)
{
	static const uint16 s_auColHdr[][2] =
	{
		{ LVCFMT_LEFT,  250 },	//SL_COLUMN_FILENAME
		{ LVCFMT_RIGHT, 100 },	//SL_COLUMN_SIZE
		{ LVCFMT_RIGHT,  50 },	//SL_COLUMN_SOURCES
		{ LVCFMT_LEFT,   65 },	//SL_COLUMN_TYPE
		{ LVCFMT_LEFT,  220 },	//SL_COLUMN_FILEHASH
		{ LVCFMT_LEFT,  220 },	//SL_COLUMN_FAKECHECK
		{ LVCFMT_LEFT,  130 },	//SL_COLUMN_LASTSEENCOMPLETE
		{ LVCFMT_LEFT,  150 },	//SL_COLUMN_FOLDER
		{ LVCFMT_RIGHT,  70 },	//SL_COLUMN_LENGTH
		{ LVCFMT_RIGHT,  70 },	//SL_COLUMN_BITRATE
		{ LVCFMT_LEFT,   60 }	//SL_COLUMN_CODEC
	};
	static const uint16 s_auIconResID[] =
	{
		IDI_RATING_FAKE,
		IDI_RATING_POOR,
		IDI_RATING_FAIR,
		IDI_RATING_GOOD,
		IDI_RATING_EXCELLENT
	};

	CImageList ilDummyImageList;
	ilDummyImageList.Create(1, 17, ILC_COLOR, 1, 1);
	SetImageList(&ilDummyImageList, LVSIL_SMALL);
	ilDummyImageList.Detach();

	SetExtendedStyle(LVS_EX_FULLROWSELECT);
	ModifyStyle(LVS_SINGLESEL, 0);

	m_pSearchList = pSearchList;

	for (unsigned ui = 0; ui < ARRSIZE(s_auColHdr); ui++)
		InsertColumn(ui, _T(""), static_cast<int>(s_auColHdr[ui][0]), static_cast<int>(s_auColHdr[ui][1]), ui);

	m_imageList.Create(16, 16, g_App.m_iDfltImageListColorFlags | ILC_MASK, ARRSIZE(s_auIconResID), 0);
	m_imageList.SetBkColor(CLR_NONE);
	FillImgLstWith16x16Icons(&m_imageList, s_auIconResID, ARRSIZE(s_auIconResID));

	memzero(&m_bSortAscending, SL_NUMCOLUMNS * sizeof(m_bSortAscending[0]));
	m_bSortAscending[SL_NUMCOLUMNS] = true;	//disable double-level sorting by default
	LoadSettings(CPreferences::TABLE_SEARCH);
	if (g_App.m_pPrefs->DoUseSort())
		SortInit(g_App.m_pPrefs->GetSearchSortCol());
	else
	{
		int		iSortCode = g_App.m_pPrefs->GetColumnSortItem(CPreferences::TABLE_SEARCH);

		iSortCode |= g_App.m_pPrefs->GetColumnSortAscending(CPreferences::TABLE_SEARCH) ? MLC_SORTASC : MLC_SORTDESC;
		SortInit(iSortCode);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CSearchListCtrl::~CSearchListCtrl()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchListCtrl::SortInit(int iSortCode)
{
	int		iSortColumn = (iSortCode & MLC_COLUMNMASK);			//the sort column
	bool	bSortAscending = (iSortCode & MLC_SORTDESC) == 0;	//the sort order
	int		iSortAltFlag = (iSortCode & MLC_SORTALT);			//the alternate sort

	if (iSortColumn != SL_COLUMN_SOURCES)
		iSortAltFlag = 0;

	m_bSortAscending[iSortColumn] = bSortAscending;
	if (iSortAltFlag == 0)
	{
		if (iSortColumn == SL_COLUMN_SOURCES)
			m_bSortAscending[SL_NUMCOLUMNS] = true;
		SetSortArrow(iSortColumn, bSortAscending);
	}
	else
	{
		if (iSortColumn == SL_COLUMN_SOURCES)
			m_bSortAscending[SL_NUMCOLUMNS] = false;
		SetSortArrow(iSortColumn, (bSortAscending) ? CMuleListCtrl::arrowDoubleUp : CMuleListCtrl::arrowDoubleDown);
	}
	SortItems(SortProc, iSortCode);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchListCtrl::Localize()
{
	static const uint16 s_auResTbl[] =
	{
		IDS_DL_FILENAME,
		IDS_DL_SIZE,
		IDS_DL_SOURCES,
		IDS_TYPE,
		IDS_FILEHASH,
		IDS_FAKE_CHECK_HEADER,
		IDS_LASTSEENCOMPLETE,
		IDS_SF_FOLDER,
		IDS_LENGTH,
		IDS_BITRATE,
		IDS_CODEC
	};

	if (GetSafeHwnd() != NULL)
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
void CSearchListCtrl::AddResult(CSearchFile *pSearchFile)
{
	EMULE_TRY

//	If there's at least one search tab...
	if (g_App.m_pMDlg->m_dlgSearch.m_ctlSearchTabs.GetItemCount() > 0)
	{
	//	Update tab-iWebServiceCounter for the given search pSearchFile
		CString		strName, strSource;
		int		j;
		int		iTabCounter;
		TCHAR		acText[510];
		TCITEM		tabItem;

		tabItem.mask = TCIF_PARAM;

		for (iTabCounter = 0; iTabCounter < g_App.m_pMDlg->m_dlgSearch.m_ctlSearchTabs.GetItemCount(); iTabCounter++)
		{
			g_App.m_pMDlg->m_dlgSearch.m_ctlSearchTabs.GetItem(iTabCounter, &tabItem);
			if (tabItem.lParam == static_cast<LONG>(pSearchFile->GetSearchID()))
				break;
		}

		if (tabItem.lParam == static_cast<LONG>(pSearchFile->GetSearchID()))
		{
			tabItem.pszText = acText;
			tabItem.cchTextMax = ARRSIZE(acText);
			tabItem.mask = TCIF_TEXT;
			g_App.m_pMDlg->m_dlgSearch.m_ctlSearchTabs.GetItem(iTabCounter, &tabItem);

			strName = acText;

			tabItem.mask = TCIF_PARAM;
			g_App.m_pMDlg->m_dlgSearch.m_ctlSearchTabs.GetItem(iTabCounter, &tabItem);

			strSource.Format(_T(" (%u)"), m_pSearchList->GetFoundFiles(pSearchFile->GetSearchID()));

			j = strName.ReverseFind(_T('('));
			if (j > 0)
				strName.Truncate(j - 1);
			strName.Append(strSource);

			tabItem.mask = TCIF_TEXT;
			tabItem.pszText = strName.LockBuffer();
			g_App.m_pMDlg->m_dlgSearch.m_ctlSearchTabs.SetItem(iTabCounter, &tabItem);
			strName.UnlockBuffer();
		}
	}

	if (pSearchFile->GetSearchID() != m_dwResultsID)
		return;

	uint32		dwTmp, dwIndex = GetItemCount();

	dwIndex = InsertItem(LVIF_TEXT | LVIF_PARAM, dwIndex, pSearchFile->GetFileName(), 0, 0, 0, (LPARAM)pSearchFile);

	CString		strTmp;

	UpdateLine(dwIndex, pSearchFile);

	SetItemText(dwIndex, SL_COLUMN_SIZE, CastItoXBytes(pSearchFile->GetFileSize()));
	SetItemText(dwIndex, SL_COLUMN_TYPE, pSearchFile->GetFileTypeString());
	strTmp = HashToString(pSearchFile->GetFileHash());
	SetItemText(dwIndex, SL_COLUMN_FILEHASH, strTmp);
 //	FakeCheck
	if (IsColumnHidden(SL_COLUMN_FAKECHECK))
		SetItemText(dwIndex, SL_COLUMN_FAKECHECK, _T(""));
	else
	{
		g_App.m_pFakeCheck->GetFakeComment(strTmp, pSearchFile->GetFileSize(), &strTmp);
		SetItemText(dwIndex, SL_COLUMN_FAKECHECK, strTmp);
	}

	SetItemText(dwIndex, SL_COLUMN_FOLDER, pSearchFile->GetSearchFileDir());

	if ((dwTmp = pSearchFile->GetMediaLength()) != 0)
		SetItemText(dwIndex, SL_COLUMN_LENGTH, CastSecondsToHM(dwTmp));
	else
		SetItemText(dwIndex, SL_COLUMN_LENGTH, _T(""));

	if ((dwTmp = pSearchFile->GetMediaBitrate()) != 0)
	{
		strTmp.Format(_T("%u kbps"), dwTmp);
		SetItemText(dwIndex, SL_COLUMN_BITRATE, strTmp);
	}
	else
		SetItemText(dwIndex, SL_COLUMN_BITRATE, _T(""));
	SetItemText(dwIndex, SL_COLUMN_CODEC, pSearchFile->GetMediaCodec());

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchListCtrl::UpdateChangingColumns(CSearchFile *pSearchFile)
{
	EMULE_TRY

	LVFINDINFO		find;

	find.flags = LVFI_PARAM;
	find.lParam = reinterpret_cast<LPARAM>(pSearchFile);

	int		iIndex = FindItem(&find);

	if (iIndex != -1)
	{
		UpdateLine(iIndex, pSearchFile);
		Update(iIndex);
		RedrawItems(iIndex, iIndex);
		SortInit(m_dwParamSort);
	}
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchListCtrl::UpdateLine(int iIndex, CSearchFile *pSearchFile)
{
	CString		strTmp;
	uint32		dwLastSeenVal;

	strTmp.Format(_T("%u (%u)"), pSearchFile->GetSourceCount(), pSearchFile->GetCompleteSourceCount());
	SetItemText(iIndex, SL_COLUMN_SOURCES, strTmp);

	if ((dwLastSeenVal = pSearchFile->GetLastSeenCompleteValue()) == 0)
		SetItemText(iIndex, SL_COLUMN_LASTSEENCOMPLETE, _T(""));	//currently available
	else
	{
		if (dwLastSeenVal == 0x7FFFFFFF)
			GetResString(&strTmp, IDS_NEVER);	//complete file wasn't seen
		else
		{
			COleDateTime lastSeenTime = COleDateTime::GetCurrentTime() - COleDateTimeSpan(0, 0, 0, dwLastSeenVal - 1);

			strTmp = lastSeenTime.Format(_T("%c"));
		}
		SetItemText(iIndex, SL_COLUMN_LASTSEENCOMPLETE, strTmp);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchListCtrl::RemoveResult(CSearchFile *pSearchFile)
{
	EMULE_TRY

	LVFINDINFO		find;

	find.flags = LVFI_PARAM;
	find.lParam = reinterpret_cast<LPARAM>(pSearchFile);

	sint32		lIndex = FindItem(&find);

	if (lIndex != -1)
		DeleteItem(lIndex);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchListCtrl::ShowResults(uint32 dwResultsID)
{
	EMULE_TRY

	DeleteAllItems();
	m_dwResultsID = dwResultsID;
	if (m_pSearchList != NULL)
		m_pSearchList->ShowResults(m_dwResultsID);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	EMULE_TRY

	if (!lpDrawItemStruct->itemData)
		return;

	CSearchFile	*pSearchFile = reinterpret_cast<CSearchFile*>(lpDrawItemStruct->itemData);
	CDC			*odc = CDC::FromHandle(lpDrawItemStruct->hDC);
	UINT		iCalcFlag = (DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);
	COLORREF	crBk, crWinBk;

	if (IsRightToLeftLanguage())
		iCalcFlag |= DT_RTLREADING;

	crWinBk = crBk = GetBkColor();
	if (lpDrawItemStruct->itemState & ODS_SELECTED)
		crBk = (GetFocus() == this) ? m_crHighlight : m_crNoHighlight;

	CMemDC		dc(odc, &lpDrawItemStruct->rcItem, crWinBk, crBk);
	CFont		*pOldFont = dc->SelectObject(GetFont());
	COLORREF	crText = ::GetSysColor(COLOR_WINDOWTEXT);
	uint32		dwRed = GetRValue(crText);
	uint32		dwGreen = GetGValue(crText);
	uint32		dwBlue = GetGValue(crText);
	CKnownFile	*pKnownFile = g_App.m_pSharedFilesList->GetFileByID(pSearchFile->GetFileHash());

//	If it's not a completed shared file, check to see if it's a part file
	if (pKnownFile == NULL)
		pKnownFile = g_App.m_pDownloadQueue->GetFileByID(pSearchFile->GetFileHash());
	if (pKnownFile != NULL)
	{
	//	Currently downloading (-> red)
		if (pKnownFile->IsPartFile())
		{
			dwRed = (pSearchFile->GetSourceCount() + 4) * 20;
			if (dwRed > 255)
				dwRed = 255;
		}
		else	//	Already downloaded and currently shared (-> green)
			dwGreen = 128;
	}
	else
	{
		pKnownFile = g_App.m_pKnownFilesList->FindKnownFileByID(pSearchFile->GetFileHash());

	//	Known but currently not shared (-> Orange)
		if (pKnownFile != NULL)
		{
			dwRed = 255;
			dwGreen = 165;
			dwBlue = 0;
		}
		else	//	Not known at all (-> indicate availability in shades of blue)
		{
			dwBlue = (pSearchFile->GetSourceCount() == 0) ? 0 : ((pSearchFile->GetSourceCount() - 1) * 20);
			if (dwBlue > 255)
				dwBlue = 255;
		}
	}

	COLORREF	crOldText = dc->SetTextColor(RGB(dwRed, dwGreen, dwBlue));
	RECT		r = lpDrawItemStruct->rcItem;
	CHeaderCtrl	*pHeaderCtrl = GetHeaderCtrl();
	int			iImgIdx, iCol, iNumCols = pHeaderCtrl->GetItemCount();

	r.right = r.left - 2;
	r.left += 3;

	for (int iColIdx = 0; iColIdx < iNumCols; iColIdx++)
	{
		iCol = pHeaderCtrl->OrderToIndex(iColIdx);
		if (IsColumnHidden(iCol))
			continue;
		r.right += CListCtrl::GetColumnWidth(iCol);
		switch (iCol)
		{
			case SL_COLUMN_FILENAME:
				// File Type
				if (g_App.m_pPrefs->ShowFileTypeIcon())
				{
					iImgIdx = g_App.GetFileTypeSystemImageIdx(pSearchFile->GetFileName());
					if (g_App.GetSystemImageList() != NULL)
						::ImageList_Draw(g_App.GetSystemImageList(), iImgIdx, dc->GetSafeHdc(), r.left, r.top + 1, ILD_TRANSPARENT);
					r.left += 19;
				}
				// File Rating
				if (g_App.m_pPrefs->ShowRatingIcons())
				{
					if ((iImgIdx = pSearchFile->GetSrvFileRating()) != PF_RATING_NONE)
						m_imageList.Draw(dc, iImgIdx - 1, CPoint(r.left - 4, r.top + 2), ILD_TRANSPARENT);
					r.left += 10;
				}
			default:
				dc->DrawText(GetItemText(lpDrawItemStruct->itemID, iCol), &r, DT_LEFT | iCalcFlag);
				break;

			case SL_COLUMN_SIZE:
			case SL_COLUMN_SOURCES:
			case SL_COLUMN_LENGTH:
			case SL_COLUMN_BITRATE:
				dc->DrawText(GetItemText(lpDrawItemStruct->itemID, iCol), &r, DT_RIGHT | iCalcFlag);
				break;
		}
		r.left = r.right + 5;
	}

	if(lpDrawItemStruct->itemState & ODS_FOCUS)
	{
		RECT	rOutline = lpDrawItemStruct->rcItem;
		CBrush	FrmBrush((GetFocus() == this) ? m_crFocusLine : m_crNoFocusLine);

		rOutline.left++;
		rOutline.right--;
		dc->FrameRect(&rOutline, &FrmBrush);
	}

	dc->SelectObject(pOldFont);
	dc->SetTextColor(crOldText);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchListCtrl::OnColumnClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	EMULE_TRY

	NM_LISTVIEW	*pNMListView = reinterpret_cast<NM_LISTVIEW*>(pNMHDR);
	int			iFlags = 0, iSubItem = pNMListView->iSubItem;
	bool		bSortOrder = m_bSortAscending[iSubItem];

//	For the double-level sorting columns, toggle the Alt column flag when the arrow is facing up
	if (iSubItem == SL_COLUMN_SOURCES)
	{
		if ((static_cast<int>(m_dwParamSort & MLC_COLUMNMASK) == iSubItem) && bSortOrder)
			m_bSortAscending[SL_NUMCOLUMNS] = !m_bSortAscending[SL_NUMCOLUMNS];
		iFlags = m_bSortAscending[SL_NUMCOLUMNS] ? 0 : MLC_SORTALT;
	}

// Reverse sorting direction for the same column and keep the same if column was changed
	if (static_cast<int>(m_dwParamSort & MLC_COLUMNMASK) == iSubItem)
		m_bSortAscending[iSubItem] = bSortOrder = !bSortOrder;

	if (iFlags == 0)
		SetSortArrow(iSubItem, bSortOrder);
	else
		SetSortArrow(iSubItem, (bSortOrder) ? arrowDoubleUp : arrowDoubleDown);
	SortItems(SortProc, iSubItem | iFlags | ((bSortOrder) ? MLC_SORTASC : MLC_SORTDESC));

	g_App.m_pPrefs->SetColumnSortItem(CPreferences::TABLE_SEARCH, iSubItem | iFlags);	//allow to save alternate criterion
	g_App.m_pPrefs->SetColumnSortAscending(CPreferences::TABLE_SEARCH, bSortOrder);

	*pResult = 0;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchListCtrl::OnNMDividerDoubleClick(NMHEADER *pHeader)
{
	int		iWidth, iColumn = pHeader->iItem;

	SetRedraw(false);
	CMuleListCtrl::OnNMDividerDoubleClick(pHeader);
	if ((iColumn == SL_COLUMN_FILENAME) && (g_App.m_pPrefs->ShowRatingIcons() || g_App.m_pPrefs->ShowFileTypeIcon()))
	{
	//	Base class sets max width for a label, adjusting according to used icons
		iWidth = GetColumnWidth(iColumn);
		if (g_App.m_pPrefs->ShowFileTypeIcon())
			iWidth += 19;
		if (g_App.m_pPrefs->ShowRatingIcons())
			iWidth += 10;
		SetColumnWidth(iColumn, iWidth);
	}
	SetRedraw(true);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CSearchListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	EMULE_TRY
	CSearchFile		*pSearchFile1 = reinterpret_cast<CSearchFile*>(lParam1);
	CSearchFile		*pSearchFile2 = reinterpret_cast<CSearchFile*>(lParam2);

	if (pSearchFile1 == NULL || pSearchFile2 == NULL)
		return 0;

	uint32	dwVal1, dwVal2;
	int		iCompare = 0;
	int		iSortMod = ((lParamSort & MLC_SORTDESC) == 0) ? 1 : -1;
	int		iSortAltFlag = (lParamSort & MLC_SORTALT);
	int		iSortCol = lParamSort & MLC_COLUMNMASK;

	for (;;)
	{
		switch (iSortCol)
		{
			case SL_COLUMN_FILENAME:
				iCompare = pSearchFile1->CmpFileNames(pSearchFile2->GetFileName());
				break;

			case SL_COLUMN_SIZE:
				iCompare = CompareInt64(pSearchFile1->GetFileSize(), pSearchFile2->GetFileSize());
				break;

			case SL_COLUMN_SOURCES:
				if (iSortAltFlag == 0)
				{
					iCompare = pSearchFile1->GetSourceCount() - pSearchFile2->GetSourceCount();
					if (iCompare == 0)
						iCompare = pSearchFile1->GetCompleteSourceCount() - pSearchFile2->GetCompleteSourceCount();
				}
				else
				{
					iCompare = pSearchFile1->GetCompleteSourceCount() - pSearchFile2->GetCompleteSourceCount();
					if (iCompare == 0)
						iCompare = pSearchFile1->GetSourceCount() - pSearchFile2->GetSourceCount();
				}
				if (iCompare == 0)
				{
					iSortCol = SL_COLUMN_FILENAME;
					iSortMod = 1;		//sort always in ascending order
					continue;
				}
				break;

			case SL_COLUMN_TYPE:
				iCompare = pSearchFile1->CmpFileTypes(pSearchFile2->GetFileType());
				if (iCompare == 0)
				{
					iSortMod = 1;		//sort always in ascending order
					if ((iCompare = pSearchFile1->GetFileExtension().Compare(pSearchFile2->GetFileExtension())) == 0)
					{
						iSortCol = SL_COLUMN_FILENAME;
						continue;
					}
				}
				break;

			case SL_COLUMN_FILEHASH:
				iCompare = memcmp(pSearchFile1->GetFileHash(), pSearchFile2->GetFileHash(), 16);
				break;

			case SL_COLUMN_FAKECHECK:
			{
				CString strVal1;
				CString strVal2;

				g_App.m_pFakeCheck->GetFakeComment(HashToString(pSearchFile1->GetFileHash()), pSearchFile1->GetFileSize(), &strVal1);
				g_App.m_pFakeCheck->GetFakeComment(HashToString(pSearchFile2->GetFileHash()), pSearchFile2->GetFileSize(), &strVal2);
				iCompare = _tcscmp(strVal1, strVal2);
				break;
			}

			case SL_COLUMN_LASTSEENCOMPLETE:
				iCompare = pSearchFile1->GetLastSeenCompleteValue() - pSearchFile2->GetLastSeenCompleteValue();
				if (iCompare == 0)
				{
					iSortCol = SL_COLUMN_SOURCES;
					iSortMod = -1;		//in descending order
					continue;
				}
				break;

			case SL_COLUMN_FOLDER:
				iCompare = _tcsicmp(pSearchFile1->GetSearchFileDir(), pSearchFile2->GetSearchFileDir());
				if (iCompare == 0)
				{
					iSortCol = SL_COLUMN_FILENAME;
					iSortMod = 1;		//sort always in ascending order
					continue;
				}
				break;

			case SL_COLUMN_LENGTH:
				dwVal1 = pSearchFile1->GetMediaLength();
				dwVal2 = pSearchFile2->GetMediaLength();
				iCompare = CompareUnsigned(dwVal1, dwVal2);
				if ((dwVal1 == 0) || (dwVal2 == 0))
					iSortMod = -1;	// not specified values always at the end
				break;

			case SL_COLUMN_BITRATE:
				dwVal1 = pSearchFile1->GetMediaBitrate();
				dwVal2 = pSearchFile2->GetMediaBitrate();
				iCompare = CompareUnsigned(dwVal1, dwVal2);
				if ((dwVal1 == 0) || (dwVal2 == 0))
					iSortMod = -1;	// not specified values always at the end
				break;

			case SL_COLUMN_CODEC:
			{
				CString strVal1 = pSearchFile1->GetMediaCodec();
				CString strVal2 = pSearchFile2->GetMediaCodec();

				iCompare = _tcsicmp(strVal1, strVal2);
				if (strVal1.IsEmpty() || strVal2.IsEmpty())
					iSortMod = -1;	// not specified values always at the end
				break;
			}
		}
		break;
	}
	return iCompare * iSortMod;

	EMULE_CATCH
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchListCtrl::OnContextMenu(CWnd *pWnd, CPoint point)
{
	NOPRM(pWnd);
	EMULE_TRY

	CTitleMenu	menuSearchFile;
	CMenu		menuED2K, menuWebServices;

	menuSearchFile.CreatePopupMenu();
	menuSearchFile.AddMenuTitle(GetResString(IDS_FILE));

	CServer		*pCurServer;
	int			iSelectionMark = GetSelectionMark();
	UINT		dwFlags2, dwSelectedCount = GetSelectedCount();
	UINT		dwMenuFlags = MF_STRING | (((iSelectionMark != -1) && (dwSelectedCount > 0)) ? MF_ENABLED : MF_GRAYED);

	menuSearchFile.AppendMenu(dwMenuFlags, MP_RESUME, GetResString(IDS_DOWNLOAD_VERB));
	menuSearchFile.AppendMenu(dwMenuFlags, MP_RESUMEPAUSED, GetResString(IDS_DOWNLOAD_VERB) + _T(" (") + GetResString(IDS_PAUSED) + _T(")"));
	menuSearchFile.AppendMenu( MF_STRING |
		((iSelectionMark != -1) && (dwSelectedCount > 0) &&
		g_App.m_pServerConnect->IsConnected() &&
		((pCurServer = g_App.m_pServerConnect->GetCurrentServer()) != NULL) &&
		(pCurServer->GetTCPFlags() & SRV_TCPFLG_RELATEDSEARCH)
		) ? MF_ENABLED : MF_GRAYED, MP_SEARCHRELATED, GetResString(IDS_SEARCHRELATED) );

	menuSearchFile.SetDefaultItem(
		(g_App.m_pPrefs->StartDownloadPaused()) ? MP_RESUMEPAUSED : MP_RESUME );

	menuSearchFile.AppendMenu(MF_SEPARATOR);

	menuSearchFile.AppendMenu(dwMenuFlags, MP_REMOVESELECTED, GetResString(IDS_REMOVESELECTED));
	dwFlags2 = MF_STRING | ((g_App.m_pMDlg->m_dlgSearch.m_ctlSearchTabs.GetItemCount() > 0) ? MF_ENABLED : MF_GRAYED);
	menuSearchFile.AppendMenu(dwFlags2, MP_REMOVEALL, GetResString(IDS_REMOVEALLSEARCH));

	menuSearchFile.AppendMenu(MF_SEPARATOR);

	menuED2K.CreateMenu();
	menuED2K.AppendMenu(MF_STRING, MP_GETED2KLINK, GetStringFromShortcutCode(IDS_DL_LINK1, SCUT_LINK, SSP_TAB_PREFIX));
	menuED2K.AppendMenu(MF_STRING, MP_GETHTMLED2KLINK, GetStringFromShortcutCode(IDS_DL_LINK2, SCUT_LINK_HTML, SSP_TAB_PREFIX));
	menuED2K.AppendMenu(MF_STRING, MP_GETHASH, GetStringFromShortcutCode(IDS_COPYHASH, SCUT_LINK_HASH, SSP_TAB_PREFIX));
	menuSearchFile.AppendMenu(dwMenuFlags | MF_POPUP, (UINT_PTR)menuED2K.m_hMenu, GetResString(IDS_ED2KLINKFIX));

	menuWebServices.CreateMenu();

	dwMenuFlags |= ( ( (UpdateURLMenu(menuWebServices) == 0) ||
		((iSelectionMark != -1) && (dwSelectedCount > 1)) ) ? MF_GRAYED : 0 );

	menuSearchFile.AppendMenu(dwMenuFlags | MF_POPUP, (UINT_PTR)menuWebServices.m_hMenu, GetResString(IDS_WEBSERVICES));

	menuSearchFile.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this, NULL);

//	Menu objects are destroyed in their destructor

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CSearchListCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	NOPRM(lParam);
	EMULE_TRY

//	For multiple selections
	CTypedPtrList<CPtrList, CSearchFile*> selectedList;
	int			iIdx;
	POSITION	pos = GetFirstSelectedItemPosition();

	while (pos != NULL)
	{
		if ((iIdx = GetNextSelectedItem(pos)) >= 0)
			selectedList.AddTail(reinterpret_cast<CSearchFile*>(GetItemData(iIdx)));
	}

	if (selectedList.GetCount() > 0)
	{
		switch (wParam)
		{
			case MP_GETED2KLINK:
			{
				CString		str;

				while (!selectedList.IsEmpty())
				{
					str += selectedList.RemoveHead()->CreateED2kLink();
					if (!selectedList.IsEmpty())
						str += _T("\r\n");
				}
				g_App.CopyTextToClipboard(str);
				return true;
			}
			case MP_GETHTMLED2KLINK:
			{
				CString		str;

				while (!selectedList.IsEmpty())
				{
					str += selectedList.RemoveHead()->CreateHTMLED2kLink();
					if (!selectedList.IsEmpty())
						str += _T("\r\n");
				}
				g_App.CopyTextToClipboard(str);
				return true;
			}
			case MP_GETHASH:
			{
				CString str;

				while (!selectedList.IsEmpty())
				{
					str += HashToString(selectedList.RemoveHead()->GetFileHash());
					if (!selectedList.IsEmpty())
						str += _T("\r\n");
				}
				g_App.CopyTextToClipboard(str);
				return true;
			}
			case MP_RESUMEPAUSED:
			case MP_RESUME:
				g_App.m_pMDlg->m_dlgSearch.DownloadSelected(wParam == MP_RESUMEPAUSED);
				return true;

			case MP_REMOVESELECTED:
				SetRedraw(false);
				while (!selectedList.IsEmpty())
					g_App.m_pSearchList->RemoveResults(selectedList.RemoveHead());
				SetRedraw(true);
				return true;

			case MP_SEARCHRELATED:
			{
				CString	strSearch(_T("related"));

			//	Just a shortcut for a user typing into the search field "related::[filehash]"
			//	Limit is used to protect against very long request
				for (iIdx = 0; !selectedList.IsEmpty() && (iIdx < 10); iIdx++)
				{
					CSearchFile	*pSearchFile = selectedList.RemoveHead();

					strSearch += _T("::");
					strSearch += HashToString(pSearchFile->GetFileHash());
				}
				g_App.m_pMDlg->m_dlgSearch.DoNewEd2kSearch(strSearch, GetResString(IDS_SEARCH_ANY), 0, 0, 0, _T(""), false, _T(""));
				return true;
			}
			default:
				if ((wParam >= MP_WEBURL) && (wParam <= MP_WEBURL + 64))
				{
					RunURL(selectedList.GetHead(), g_App.m_strWebServiceURLArray.GetAt(wParam - MP_WEBURL));
					return true;
				}
				break;
		}
	}
//	Commands which don't depend on selection
	switch (wParam)
	{
		case MP_REMOVEALL:
			g_App.m_pMDlg->m_dlgSearch.DeleteAllSearches();
			break;
	}
	EMULE_CATCH
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CSearchListCtrl::PreTranslateMessage(MSG *pMsg)
{
	if ((pMsg->message == WM_KEYDOWN) || (pMsg->message == WM_SYSKEYDOWN))
	{
		POSITION	posSelClient = GetFirstSelectedItemPosition();

		if (posSelClient != NULL)
		{
			int		iMessage = 0;
			short	nCode = GetCodeFromPressedKeys(pMsg);

			if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_LINK))
				iMessage = MP_GETED2KLINK;
			else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_LINK_HTML))
				iMessage = MP_GETHTMLED2KLINK;
			else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_LINK_HASH))
				iMessage = MP_GETHASH;

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
