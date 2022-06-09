//	This file is part of eMule Plus
//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//	as published by the Free Software Foundation; either
//	version 2 of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "stdafx.h"
#include "..\PartFile.h"
#include "FDParts.h"
#include "..\emule.h"

#define FD_FILT_COMPLETE	0x01
#define FD_FILT_NOTAVAIL	0x02

#define BLUE_GRANULARITY	8
#define BLUE_MAXGREEN		208
#define BLUE_LEVELS			(BLUE_MAXGREEN / BLUE_GRANULARITY)

//	Zooming is used as a light workaround to avoid artifacts caused by not perfect
//	bar drawing algorithm which is not good when the number of parts is bigger
//	than the number of width pixels
#define BAR_ZOOM_SHIFT		8u

// CFDParts dialog

std::vector<PartInfoType>	CFDParts::s_aPartInfo;

IMPLEMENT_DYNCREATE(CFDParts, CPropertyPage)

CFDParts::CFDParts() : CPropertyPage(CFDParts::IDD)
{
	m_pFile = NULL;
	memset(&m_bSortAscending, true, sizeof(m_bSortAscending));
}

CFDParts::~CFDParts()
{
}

void CFDParts::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_PARTS, m_PartsListCtrl);
}


BEGIN_MESSAGE_MAP(CFDParts, CPropertyPage)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_PARTS, OnLvnColumnclick)
	ON_BN_CLICKED(IDC_HIDECOMPLETE, OnFilterChange)
	ON_BN_CLICKED(IDC_HIDENOTAVAILABLE, OnFilterChange)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFDParts message handlers

BOOL CFDParts::OnInitDialog()
{
	static const uint16 s_auColHdr[][3] =
	{
		{ IDS_PART,							LVCFMT_LEFT,  50 },		// FDPARTSCOL_NUMBER
		{ IDS_INFLST_FILE_COMPLETEDSIZE,	LVCFMT_RIGHT, 110 },	// FDPARTSCOL_COMPLETESIZE
		{ IDS_DL_SOURCES,					LVCFMT_RIGHT, 60 },		// FDPARTSCOL_SOURCES
		{ IDS_STATUS,						LVCFMT_LEFT,  80 }		// FDPARTSCOL_STATUS
	};

	CPropertyPage::OnInitDialog();

	Localize();

	for (unsigned ui = 0; ui < ARRSIZE(s_auColHdr); ui++)
		m_PartsListCtrl.InsertColumn(ui, GetResString(s_auColHdr[ui][0]), static_cast<int>(s_auColHdr[ui][1]), static_cast<int>(s_auColHdr[ui][2]), ui);
	m_PartsListCtrl.LoadSettings(CPreferences::TABLE_PARTSTATUS);

//	Display constant information
	CString	strBuff;

	strBuff.Format(_T("%u %s"), m_pFile->GetLastPartSize(), GetResString(IDS_BYTES));
	SetDlgItemText(IDC_LASTPARTSZ, strBuff);

//	Initialize sorting
	int		iSortColumn = g_App.m_pPrefs->GetColumnSortItem(CPreferences::TABLE_PARTSTATUS);
	bool	bSortAscending = g_App.m_pPrefs->GetColumnSortAscending(CPreferences::TABLE_PARTSTATUS);

	SortInit(iSortColumn | (bSortAscending ? MLC_SORTASC : MLC_SORTDESC));

//	Setup filters
	byte	byteFilter = g_App.m_pPrefs->GetDetailedPartsFilter();

	if ((byteFilter & FD_FILT_COMPLETE) != 0)
		CheckDlgButton(IDC_HIDECOMPLETE, BST_CHECKED);
	if ((byteFilter & FD_FILT_NOTAVAIL) != 0)
		CheckDlgButton(IDC_HIDENOTAVAILABLE, BST_CHECKED);

	s_aPartInfo.reserve(m_pFile->GetPartCount());

	Update();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFDParts::Localize()
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_DLPARTSTATUS_LBL, IDS_DLPARTSTATUS },
		{ IDC_UPPARTSTATUS_LBL, IDS_UPPARTSTATUS },
		{ IDC_PARTCOMPLSTATUS_LBL, IDS_PARTCOMPLETION },
		{ IDC_HIDECOMPLETE, IDS_HIDECOMPLETE },
		{ IDC_HIDENOTAVAILABLE, IDS_HIDENOTAVAILABLE },
		{ IDC_LASTPARTSZ_LBL, IDS_LASTPARTSIZE }
	};

	if (GetSafeHwnd() != NULL)
	{
		for (uint32 i = 0; i < ARRSIZE(s_auResTbl); i++)
			SetDlgItemText(s_auResTbl[i][0], GetResString(static_cast<UINT>(s_auResTbl[i][1])));
	}
}

void CFDParts::Update()
{
	EMULE_TRY

	if (m_pFile == NULL || !::IsWindow(GetSafeHwnd()))
		return;

	CString				astrStatus[FDPARTS_STAT_COUNT];
	double				dPercent;
	int					iIdx;
	bool				bDelete, bFlat, bTmp, bUpdated = false;
	byte				byteFilter;
	TCHAR				acTmp[32];
	LVITEM				lvi;
	PartInfoType		PartItem;
	uint32				ui, dwWidth, dwCompleteSz, dwTmp, dwPartCnt = m_pFile->GetPartCount();
	std::vector<uint32>	adwUpPartStatuses(dwPartCnt, 0), adwDnPartStatuses(dwPartCnt, 0);
	std::vector<uint32>	adwPartsInList(dwPartCnt, 0);
	const COLORREF		crMissing = RGB(255, 0, 0);
	COLORREF			crNoReception, crComplete;
	RECT				rWnd;
	POSITION			pos;

	GetResString(&astrStatus[FDPARTS_STAT_DOWNLOADING], IDS_DOWNLOADING);	// Preload status strings
	GetResString(&astrStatus[FDPARTS_STAT_RECOVERING], IDS_RECOVERING);
	GetResString(&astrStatus[FDPARTS_STAT_CORRUPTED], IDS_CORRUPTED);
	// [FDPARTS_STAT_NONE] is empty string
	GetResString(&astrStatus[FDPARTS_STAT_NOTAVAILABLE], IDS_NOTAVAILABLE);
	GetResString(&astrStatus[FDPARTS_STAT_COMPLETE], IDS_COMPLETE);

	if ((bFlat = g_App.m_pPrefs->UseFlatBar()) == true)
	{
		crComplete = RGB(0, 150, 0);
		crNoReception = RGB(0, 0, 0);
	}
	else
	{
		crComplete = RGB(0, 192, 0);
		crNoReception = RGB(95, 95, 95);
	}
	m_PartsListCtrl.GetWindowRect(&rWnd);
	dwWidth = rWnd.right - rWnd.left;

//	Part counter is used as index instead of actual part offset in a file, because:
//	1) it's faster; 2) to make last part width equal to others for better representation
	CBarShader			DownloadStatusBar(16, dwWidth, crMissing, dwPartCnt << BAR_ZOOM_SHIFT);
	CBarShader			UploadStatusBar(16, dwWidth, crMissing, dwPartCnt << BAR_ZOOM_SHIFT);
	CBarShader			CompletionBar(16, dwWidth, crNoReception, dwPartCnt << BAR_ZOOM_SHIFT);

	g_App.m_pUploadQueue->GetUploadFilePartsAvailability(&adwUpPartStatuses[0], dwPartCnt, m_pFile->GetFileHash());

//	Search for max/min values for dynamic scaling
//	(to provide better snap-shot of parts availability for files with many sources)
	uint32				dwDnMax = 0, dwDnMin = ~0u, dwUpMax = 0, dwUpMin = ~0u, dwDnDiff, dwUpDiff;

	for (ui = 0; ui < dwPartCnt; ui++)
	{
		if ((adwDnPartStatuses[ui] = m_pFile->GetSrcPartFrequency(ui)) != 0)
		{
			if (adwDnPartStatuses[ui] > dwDnMax)
				dwDnMax = adwDnPartStatuses[ui];
			if (adwDnPartStatuses[ui] < dwDnMin)
				dwDnMin = adwDnPartStatuses[ui];
		}
		if (adwUpPartStatuses[ui] != 0)
		{
			if (adwUpPartStatuses[ui] > dwUpMax)
				dwUpMax = adwUpPartStatuses[ui];
			if (adwUpPartStatuses[ui] < dwUpMin)
				dwUpMin = adwUpPartStatuses[ui];
		}
	}
	dwDnDiff = 0;
	if (dwDnMax > BLUE_LEVELS)
		dwDnDiff = ((dwDnMax - dwDnMin) < BLUE_LEVELS) ? (dwDnMax - BLUE_LEVELS) : (dwDnMin - 1);
	dwUpDiff = 0;
	if (dwUpMax > BLUE_LEVELS)
		dwUpDiff = ((dwUpMax - dwUpMin) < BLUE_LEVELS) ? (dwUpMax - BLUE_LEVELS) : (dwUpMin - 1);

	byteFilter = ((IsDlgButtonChecked(IDC_HIDECOMPLETE)) ? FD_FILT_COMPLETE : 0) |
		((IsDlgButtonChecked(IDC_HIDENOTAVAILABLE)) ? FD_FILT_NOTAVAIL : 0);
	g_App.m_pPrefs->SetDetailedPartsFilter(byteFilter);

	lvi.mask = LVIF_TEXT | LVIF_PARAM;
	lvi.iSubItem = 0;

	m_PartsListCtrl.SetRedraw(FALSE);
	m_PartsListCtrl.SetSortProcedure(NULL);	//	Disable list sorting

//	The whole processing here looks quite weird... it's done to make it fast, as
//	otherwise it's very slow for large files. First current list elements are processed
//	then the rest of the chunks is checked to avoid damn slow FindItem() calls
	for (iIdx = 0, pos = m_PartsListCtrl.GetItemDataHeadPos(); pos != NULL; iIdx++)
	{
		ui = static_cast<uint32>(m_PartsListCtrl.GetItemDataByPos(pos, iIdx));
		adwPartsInList[ui] = 1;	//	Mark that a part is/was in the list

		bDelete = false;
		if ((PartItem.uiSrcNum1 = adwDnPartStatuses[ui]) != 0)
		{
			dwTmp = PartItem.uiSrcNum1 - dwDnDiff;
			DownloadStatusBar.FillRange( ui << BAR_ZOOM_SHIFT, (ui + 1u) << BAR_ZOOM_SHIFT,
				RGB(0, (dwTmp > BLUE_LEVELS) ? 0 : (BLUE_MAXGREEN - BLUE_GRANULARITY * dwTmp), 255) );
		}

		if ((PartItem.uiSrcNum2 = adwUpPartStatuses[ui]) != 0)
		{
			dwTmp = PartItem.uiSrcNum2 - dwUpDiff;
			UploadStatusBar.FillRange( ui << BAR_ZOOM_SHIFT, (ui + 1u) << BAR_ZOOM_SHIFT,
				RGB(0, (dwTmp > BLUE_LEVELS) ? 0 : (BLUE_MAXGREEN - BLUE_GRANULARITY * dwTmp), 255) );
		}

		if (m_pFile->IsPartComplete(ui))
		{
		//	Filter complete parts
			CompletionBar.FillRange(ui << BAR_ZOOM_SHIFT, (ui + 1u) << BAR_ZOOM_SHIFT, crComplete);
			if ((byteFilter & FD_FILT_COMPLETE) != 0)
			{
				m_PartsListCtrl.DeleteItem(iIdx);	//	Delete filtered item
				iIdx--;
				continue;
			}
			PartItem.uiStatus = FDPARTS_STAT_COMPLETE;
		}
		else
		{
			PartItem.uiStatus = FDPARTS_STAT_NONE;
			if (PartItem.uiSrcNum1 == 0)
			{
			//	Filter not available parts
				if ((byteFilter & FD_FILT_NOTAVAIL) != 0)
					bDelete = true;
				PartItem.uiStatus = FDPARTS_STAT_NOTAVAILABLE;
			}
			if ((bTmp = m_pFile->IsPartDownloading(ui)) == true)
				PartItem.uiStatus = FDPARTS_STAT_DOWNLOADING;
			if (m_pFile->IsCorruptedPart(ui))
				PartItem.uiStatus = (bTmp) ? FDPARTS_STAT_RECOVERING : FDPARTS_STAT_CORRUPTED;
		}

		dwTmp = m_pFile->GetPartSize(ui);
		PartItem.uiCompleteSz = dwCompleteSz = dwTmp - m_pFile->GetPartLeftToDLSize(ui);
		if (dwCompleteSz == dwTmp)
			dPercent = 100.0;
		else
		{
			dPercent = floor(static_cast<double>(dwCompleteSz) * 10000 / static_cast<double>(dwTmp)) / 100.0;
			if (dwCompleteSz != 0)
			{
			//	Something was received for this part, show how much is filled
				CompletionBar.FillRange(ui << BAR_ZOOM_SHIFT, (ui + 1u) << BAR_ZOOM_SHIFT, RGB(0, (220 - 2.2 * dPercent), 255));
			}
		}

		if (bDelete)
		{
			m_PartsListCtrl.DeleteItem(iIdx);	//	Delete filtered item
			iIdx--;
			continue;
		}
		if (memcmp(&PartItem, &s_aPartInfo[ui], sizeof(PartItem)) != 0)
		{
			bUpdated = true;
			s_aPartInfo[ui] = PartItem;

			_stprintf(acTmp, _T("%u (%.2f%%)"), dwCompleteSz, dPercent);
			m_PartsListCtrl.SetItemText(iIdx, FDPARTSCOL_COMPLETESIZE, acTmp);
			_stprintf(acTmp, _T("%u/%u"), PartItem.uiSrcNum1, PartItem.uiSrcNum2);
			m_PartsListCtrl.SetItemText(iIdx, FDPARTSCOL_SOURCES, acTmp);
			m_PartsListCtrl.SetItemText(iIdx, FDPARTSCOL_STATUS, astrStatus[PartItem.uiStatus]);
		}
	}
	for (ui = 0; ui < dwPartCnt; ui++)
	{
		if (adwPartsInList[ui] != 0)
			continue;

		bDelete = false;
		if ((PartItem.uiSrcNum1 = adwDnPartStatuses[ui]) != 0)
		{
			dwTmp = PartItem.uiSrcNum1 - dwDnDiff;
			DownloadStatusBar.FillRange( ui << BAR_ZOOM_SHIFT, (ui + 1u) << BAR_ZOOM_SHIFT,
				RGB(0, (dwTmp > BLUE_LEVELS) ? 0 : (BLUE_MAXGREEN - BLUE_GRANULARITY * dwTmp), 255) );
		}

		if ((PartItem.uiSrcNum2 = adwUpPartStatuses[ui]) != 0)
		{
			dwTmp = PartItem.uiSrcNum2 - dwUpDiff;
			UploadStatusBar.FillRange( ui << BAR_ZOOM_SHIFT, (ui + 1u) << BAR_ZOOM_SHIFT,
				RGB(0, (dwTmp > BLUE_LEVELS) ? 0 : (BLUE_MAXGREEN - BLUE_GRANULARITY * dwTmp), 255) );
		}

		if (m_pFile->IsPartComplete(ui))
		{
		//	Filter complete parts
			CompletionBar.FillRange(ui << BAR_ZOOM_SHIFT, (ui + 1u) << BAR_ZOOM_SHIFT, crComplete);
			if ((byteFilter & FD_FILT_COMPLETE) != 0)
				continue;
			PartItem.uiStatus = FDPARTS_STAT_COMPLETE;
		}
		else
		{
			PartItem.uiStatus = FDPARTS_STAT_NONE;
			if (PartItem.uiSrcNum1 == 0)
			{
			//	Filter not available parts
				if ((byteFilter & FD_FILT_NOTAVAIL) != 0)
					bDelete = true;
				PartItem.uiStatus = FDPARTS_STAT_NOTAVAILABLE;
			}
			if ((bTmp = m_pFile->IsPartDownloading(ui)) == true)
				PartItem.uiStatus = FDPARTS_STAT_DOWNLOADING;
			if (m_pFile->IsCorruptedPart(ui))
				PartItem.uiStatus = (bTmp) ? FDPARTS_STAT_RECOVERING : FDPARTS_STAT_CORRUPTED;
		}

		dwTmp = m_pFile->GetPartSize(ui);
		PartItem.uiCompleteSz = dwCompleteSz = dwTmp - m_pFile->GetPartLeftToDLSize(ui);
		if (dwCompleteSz == dwTmp)
			dPercent = 100.0;
		else
		{
			dPercent = floor(static_cast<double>(dwCompleteSz) * 10000 / static_cast<double>(dwTmp)) / 100.0;
			if (dwCompleteSz != 0)
			{
			//	Something was received for this part, show how much is filled
				CompletionBar.FillRange(ui << BAR_ZOOM_SHIFT, (ui + 1u) << BAR_ZOOM_SHIFT, RGB(0, (220 - 2.2 * dPercent), 255));
			}
		}
		if (bDelete)
			continue;

		_stprintf(acTmp, _T("%u"), ui);
		lvi.iItem = m_PartsListCtrl.GetItemCount();
		lvi.pszText = acTmp;
		lvi.lParam = static_cast<LPARAM>(ui);
		iIdx = m_PartsListCtrl.InsertItem(&lvi);

		bUpdated = true;
		s_aPartInfo[ui] = PartItem;

		_stprintf(acTmp, _T("%u (%.2f%%)"), dwCompleteSz, dPercent);
		m_PartsListCtrl.SetItemText(iIdx, FDPARTSCOL_COMPLETESIZE, acTmp);
		_stprintf(acTmp, _T("%u/%u"), PartItem.uiSrcNum1, PartItem.uiSrcNum2);
		m_PartsListCtrl.SetItemText(iIdx, FDPARTSCOL_SOURCES, acTmp);
		m_PartsListCtrl.SetItemText(iIdx, FDPARTSCOL_STATUS, astrStatus[PartItem.uiStatus]);
	}
	m_PartsListCtrl.SetSortProcedure(SortProc);	//	Enable list sorting
	if (bUpdated)
		m_PartsListCtrl.SortItems(SortProc, m_PartsListCtrl.GetSortParam());
	m_PartsListCtrl.SetRedraw(TRUE);

//	Draw status bars
	CDC			memDC, *pDC = GetDC();
	CBitmap	   *pOldBitmap;
	HWND		hItemWnd;

	memDC.CreateCompatibleDC(pDC);

//	Prepare and draw Download Availability Status Bar
	if (m_bmpDownloadStatus.m_hObject != NULL)
		m_bmpDownloadStatus.DeleteObject();
	m_bmpDownloadStatus.CreateCompatibleBitmap(pDC, dwWidth, 16);
	pOldBitmap = memDC.SelectObject(&m_bmpDownloadStatus);
	DownloadStatusBar.Draw(&memDC, 0, 0, bFlat);
	GetDlgItem(IDC_DLPARTSTATUS, &hItemWnd);
	::SendMessage(hItemWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)m_bmpDownloadStatus.m_hObject);
	::InvalidateRect(hItemWnd, NULL, FALSE);	// No need to erase background as the whole area is used by a picture

//	Prepare and draw Upload Availability Status Bar
	if (m_bmpUploadStatus.m_hObject != NULL)
		m_bmpUploadStatus.DeleteObject();
	m_bmpUploadStatus.CreateCompatibleBitmap(pDC, dwWidth, 16);
	memDC.SelectObject(&m_bmpUploadStatus);
	UploadStatusBar.Draw(&memDC, 0, 0, bFlat);
	GetDlgItem(IDC_UPPARTSTATUS, &hItemWnd);
	::SendMessage(hItemWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)m_bmpUploadStatus.m_hObject);
	::InvalidateRect(hItemWnd, NULL, FALSE);	// No need to erase background as the whole area is used by a picture

//	Prepare and draw Parts Completion Status Bar
	if (m_bmpCompletionStatus.m_hObject != NULL)
		m_bmpCompletionStatus.DeleteObject();
	m_bmpCompletionStatus.CreateCompatibleBitmap(pDC, dwWidth, 16);
	memDC.SelectObject(&m_bmpCompletionStatus);
	CompletionBar.Draw(&memDC, 0, 0, bFlat);
	GetDlgItem(IDC_PARTCOMPLSTATUS, &hItemWnd);
	::SendMessage(hItemWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)m_bmpCompletionStatus.m_hObject);
	::InvalidateRect(hItemWnd, NULL, FALSE);	// No need to erase background as the whole area is used by a picture

	memDC.SelectObject(pOldBitmap);
	ReleaseDC(pDC);

	EMULE_CATCH
}

void CFDParts::OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	EMULE_TRY

	NM_LISTVIEW	*pNMListView = reinterpret_cast<NM_LISTVIEW*>(pNMHDR);
	int			iFlags = 0, iSubItem = pNMListView->iSubItem;
	bool		bSortOrder = m_bSortAscending[iSubItem];

//	For the double-level sorting columns, toggle the Alt column flag when the arrow is facing up
	if (iSubItem == FDPARTSCOL_SOURCES)
	{
		if (((m_PartsListCtrl.GetSortParam() & MLC_COLUMNMASK) == static_cast<uint32>(iSubItem)) && bSortOrder)
			m_bSortAscending[FDPARTSCOL_NUMCOLUMNS] = !m_bSortAscending[FDPARTSCOL_NUMCOLUMNS];
		iFlags = m_bSortAscending[FDPARTSCOL_NUMCOLUMNS] ? 0 : MLC_SORTALT;
	}

// Reverse sorting direction for the same column and keep the same if column was changed
	if ((m_PartsListCtrl.GetSortParam() & MLC_COLUMNMASK) == static_cast<uint32>(iSubItem))
		m_bSortAscending[iSubItem] = bSortOrder = !bSortOrder;

	if (iFlags == 0)
		m_PartsListCtrl.SetSortArrow(iSubItem, bSortOrder);
	else
		m_PartsListCtrl.SetSortArrow(iSubItem, (bSortOrder) ? CMuleListCtrl::arrowDoubleUp : CMuleListCtrl::arrowDoubleDown);
	m_PartsListCtrl.SortItems(SortProc, iSubItem + iFlags + ((bSortOrder) ? MLC_SORTASC : MLC_SORTDESC));

	g_App.m_pPrefs->SetColumnSortItem(CPreferences::TABLE_PARTSTATUS, iSubItem | iFlags);	// Allow to save alternate criterion
	g_App.m_pPrefs->SetColumnSortAscending(CPreferences::TABLE_PARTSTATUS, bSortOrder);

	*pResult = 0;

	EMULE_CATCH
}

void CFDParts::OnFilterChange()
{
	Update();
}

int CALLBACK CFDParts::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	PartInfoType	*pItem1 = &s_aPartInfo[lParam1];
	PartInfoType	*pItem2 = &s_aPartInfo[lParam2];

	int		iCompare = 0;
	int		iSortColumn = (lParamSort & MLC_COLUMNMASK);
	int		iSortAltFlag = (lParamSort & MLC_SORTALT);
	int		iSortMod = ((lParamSort & MLC_SORTDESC) == 0) ? 1 : -1;

	switch (iSortColumn)
	{
		case FDPARTSCOL_NUMBER:
			iCompare = lParam1 - lParam2;
			break;

		case FDPARTSCOL_COMPLETESIZE:
			iCompare = pItem1->uiCompleteSz - pItem2->uiCompleteSz;
			break;

		case FDPARTSCOL_SOURCES:
			if (iSortAltFlag == 0)
				iCompare = pItem1->uiSrcNum1 - pItem2->uiSrcNum1;
			else
				iCompare = pItem1->uiSrcNum2 - pItem2->uiSrcNum2;
			break;

		case FDPARTSCOL_STATUS:
			iCompare = pItem1->uiStatus - pItem2->uiStatus;
			break;
	}
	if (iCompare == 0)
		return lParam1 - lParam2;	// Compare by part number if no difference by another criterion
	return iCompare * iSortMod;
}

void CFDParts::SortInit(int iSortCode)
{
	int		iSortColumn = (iSortCode & MLC_COLUMNMASK);			// The sort column
	bool	bSortAscending = (iSortCode & MLC_SORTDESC) == 0;	// The sort order
	int		iSortAltFlag = (iSortCode & MLC_SORTALT);			// The alternate sort

	if (iSortColumn != FDPARTSCOL_SOURCES)
		iSortAltFlag = 0;

	m_bSortAscending[iSortColumn] = bSortAscending;
	if (iSortAltFlag == 0)
		m_PartsListCtrl.SetSortArrow(iSortColumn, bSortAscending);
	else
	{
		if (iSortColumn == FDPARTSCOL_SOURCES)
			m_bSortAscending[FDPARTSCOL_NUMCOLUMNS] = false;
		m_PartsListCtrl.SetSortArrow(iSortColumn, (bSortAscending) ? CMuleListCtrl::arrowDoubleUp : CMuleListCtrl::arrowDoubleDown);
	}
	m_PartsListCtrl.SortItems(SortProc, iSortCode);
}

void CFDParts::OnDestroy()
{
	m_PartsListCtrl.SaveSettings(CPreferences::TABLE_PARTSTATUS);
}
