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
#include "DownloadListCtrl.h"
#include "otherfunctions.h" 
#include "updownclient.h"
#include "MenuCmds.h"
#include "ClientDetailDialog.h"
#include "FileDetailDialog.h"
#include "MetaDataDlg.h"
#include "InputBox.h"
#include "emuledlg.h"
#include "DownloadQueue.h"
#include "PartFile.h"
#include "ClientCredits.h"
#include "MemDC.h"
#include "TransferWnd.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "Kademlia/Kademlia/Prefs.h"
#include "Kademlia/net/KademliaUDPListener.h"
#include "Preview.h"
#include "StringConversion.h"
#include "AddSourceDlg.h"
#include "ToolTipCtrlX.h"
#include "CollectionViewDialog.h"
#include "SearchDlg.h"
#include "SharedFileList.h"
#include "ListenSocket.h" //Xman changed: display the obfuscation icon for all clients which enabled it
#include "Defaults.h"// X: [IP] - [Import Parts],[POFC] - [PauseOnFileComplete]
#include "shahashset.h"// X: [IP] - [Import Parts]
#include<algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CDownloadListCtrl

static const UINT colStrID[]={
	 IDS_DL_FILENAME
	,IDS_DL_SIZE
	,IDS_DL_TRANSF
	,IDS_DL_TRANSFCOMPL
	,IDS_DL_SPEED
	,IDS_DL_PROGRESS
	,IDS_DL_SOURCES
	,IDS_PRIORITY
	,IDS_STATUS
	,IDS_DL_REMAINS
	,IDS_LASTSEENCOMPL
	,IDS_FD_LASTCHANGE
	,IDS_CAT
	,IDS_ADDEDON
	,IDS_SEARCHAVAIL
};
IMPLEMENT_DYNAMIC(CDownloadListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CDownloadListCtrl, CMuleListCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnClick)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnLvnGetDispInfo)
	ON_NOTIFY_REFLECT(LVN_GETINFOTIP, OnLvnGetInfoTip)
	ON_NOTIFY_REFLECT(NM_CLICK, OnListModified)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNmDblClk)
	ON_WM_CONTEXTMENU()
	//ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

CDownloadListCtrl::CDownloadListCtrl()
	: CListCtrlItemWalk(this)
{
	m_tooltip = new CToolTipCtrlX;
}

CDownloadListCtrl::~CDownloadListCtrl()
{
	if (m_DropMenu) 
        VERIFY( m_DropMenu.DestroyMenu() ); //Xman Xtreme Downloadmanager
	if (m_PrioMenu)
		VERIFY( m_PrioMenu.DestroyMenu() );
    if (m_SourcesMenu)
		VERIFY( m_SourcesMenu.DestroyMenu() );
	if (m_FileMenu)
		VERIFY( m_FileMenu.DestroyMenu() );
	delete m_tooltip;
	m_ctlListHeader.Detach();
}

void CDownloadListCtrl::SetGridLine()
{
 if (thePrefs.UseGridlines())
	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_GRIDLINES); 
   else
     SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
}

void CDownloadListCtrl::Init()
{
	SetPrefsKey(_T("DownloadListCtrl"));
        SetGridLine();
	ASSERT( (GetStyle() & LVS_SINGLESEL) == 0 );
	
	CToolTipCtrl* tooltip = GetToolTips();
	if (tooltip){
		m_tooltip->SubclassWindow(*tooltip);
		tooltip->ModifyStyle(0, TTS_NOPREFIX);
		tooltip->SetDelayTime(TTDT_AUTOPOP, 20000);
		tooltip->SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1000);
	}

	InsertColumn(0, GetResString(IDS_DL_FILENAME),		LVCFMT_LEFT,  DFLT_FILENAME_COL_WIDTH);
	InsertColumn(1, GetResString(IDS_DL_SIZE),			LVCFMT_RIGHT, DFLT_SIZE_COL_WIDTH);
	InsertColumn(2, GetResString(IDS_DL_TRANSF),		LVCFMT_RIGHT, DFLT_SIZE_COL_WIDTH,		-1, true);
	InsertColumn(3, GetResString(IDS_DL_TRANSFCOMPL),	LVCFMT_RIGHT, DFLT_SIZE_COL_WIDTH);
	InsertColumn(4, GetResString(IDS_DL_SPEED),			LVCFMT_RIGHT, DFLT_DATARATE_COL_WIDTH);
	InsertColumn(5, GetResString(IDS_DL_PROGRESS),		LVCFMT_LEFT,  DFLT_SIZE_COL_WIDTH);
	InsertColumn(6, GetResString(IDS_DL_SOURCES),		LVCFMT_RIGHT,  60);
	InsertColumn(7, GetResString(IDS_PRIORITY),			LVCFMT_LEFT,  DFLT_PRIORITY_COL_WIDTH);
	InsertColumn(8, GetResString(IDS_STATUS),			LVCFMT_LEFT,   70);
	InsertColumn(9, GetResString(IDS_DL_REMAINS),		LVCFMT_LEFT,  110);
	CString lsctitle = GetResString(IDS_LASTSEENCOMPL);
	lsctitle.Remove(_T(':'));
	InsertColumn(10, lsctitle,							LVCFMT_LEFT,  150,						-1, true);
	lsctitle = GetResString(IDS_FD_LASTCHANGE);
	lsctitle.Remove(_T(':'));
	InsertColumn(11, lsctitle,							LVCFMT_LEFT,  120,						-1, true);
	InsertColumn(12, GetResString(IDS_CAT),				LVCFMT_LEFT,  100,						-1, true);
	InsertColumn(13, GetResString(IDS_ADDEDON),			LVCFMT_LEFT,  120);
	InsertColumn(14, GetResString(IDS_SEARCHAVAIL),			LVCFMT_LEFT,   70);
	
	SetAllIcons();
	CreateMenues();//Localize();// X: [RUL] - [Remove Useless Localize]

	LoadSettings();
	curTab = thePrefs.lastTranWndCatID; // X: [RCI] - [Remember Catalog ID]

	// Barry - Use preferred sort order from preferences
	m_bRemainSort = thePrefs.TransferlistRemainSortStyle();
	int adder = 0;
	if (GetSortItem() != 9 || !m_bRemainSort)
		SetSortArrow();
	else {
		SetSortArrow(GetSortItem(), GetSortAscending() ? arrowDoubleUp : arrowDoubleDown);
		adder = 81;
	}
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0 : 100) + adder);
	IgnoredColums = 0x4E00; //9,10,11,14
	m_ctlListHeader.Attach(GetHeaderCtrl()->Detach());
}
/*
void CDownloadListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	CreateMenues();
}
*/
//Xman end

void CDownloadListCtrl::SetAllIcons()
{
	ApplyImageList(NULL);
	m_ImageList.DeleteImageList();
	m_ImageList.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);
	// Status Icons :: Start
	m_ImageList.Add(CTempIconLoader(_T("STATUS_DOWNLOADING"))); //0 green
	m_ImageList.Add(CTempIconLoader(_T("STATUS_PAUSED"))); //1 orange
	m_ImageList.Add(CTempIconLoader(_T("STATUS_STOPPED"))); //2 red
	m_ImageList.Add(CTempIconLoader(_T("STATUS_WAITING"))); //3 gray
	m_ImageList.Add(CTempIconLoader(_T("STATUS_COMPLETING"))); //4 blue
	m_ImageList.Add(CTempIconLoader(_T("PREVIEW_DOWN")));  //5
	m_ImageList.Add(CTempIconLoader(_T("PREVIEW_WAIT")));  //6
  	m_ImageList.Add(CTempIconLoader(_T("STATUS_TICK"))); //7
    // Status Icons :: End
	m_ImageList.Add(CTempIconLoader(_T("HEALTH_1"))); //8
	m_ImageList.Add(CTempIconLoader(_T("HEALTH_2"))); //9
	m_ImageList.Add(CTempIconLoader(_T("HEALTH_3"))); //10
	m_ImageList.Add(CTempIconLoader(_T("HEALTH_4"))); //11
	m_ImageList.Add(CTempIconLoader(_T("HEALTH_5"))); //12
	m_ImageList.Add(CTempIconLoader(_T("HEALTH_6"))); //13
	m_ImageList.Add(CTempIconLoader(_T("HEALTH_7"))); //14
	m_ImageList.Add(CTempIconLoader(_T("HEALTH_8"))); //15
	//theApp.SetClientIcon(m_ImageList);
	// Apply the image list also to the listview control, even if we use our own 'DrawItem'.
	// This is needed to give the listview control a chance to initialize the row height.
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) != 0 );
	VERIFY( ApplyImageList(m_ImageList) == NULL );
}

void CDownloadListCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;
	for (int icol=0;icol<pHeaderCtrl->GetItemCount();++icol) {
		strRes=GetResString(colStrID[icol]);
		if(icol > 9 || icol < 12)
			strRes.Remove(_T(':'));
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(icol, &hdi);
	}

	CreateMenues();
}

void CDownloadListCtrl::AddFile(CPartFile* toadd)
{
	// The same file shall be added only once
	if(toadd->CheckShowItemInGivenCat(curTab) && !IsFilteredItem(toadd))
		InsertItem(LVIF_PARAM | LVIF_TEXT, GetItemCount(), LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)toadd);
}

void CDownloadListCtrl::RemoveFile(const CPartFile* toremove)
{
	//Refresh clientlist
	if (!CemuleDlg::IsRunning())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)toremove;
	int result = FindItem(&find);
	if (result != -1){
		DeleteItem(result);
		theApp.emuledlg->transferwnd->downloadclientsctrl.Reload(NULL, true);
		}
	theApp.emuledlg->transferwnd->UpdateFilesCount();
}

void CDownloadListCtrl::GetItemDisplayText(const CAbstractFile* file, int iSubItem, LPTSTR pszText, int cchTextMax) const
{
	if (pszText == NULL || cchTextMax <= 0) {
		ASSERT(0);
		return;
	}
	const CPartFile* lpPartFile = reinterpret_cast<const CPartFile*>(file);
	pszText[0] = _T('\0');
	switch (iSubItem)
	{
		case 0: 	// file name
			_tcsncpy(pszText, lpPartFile->GetFileName(), cchTextMax);
			break;

		case 1:		// size
			_tcsncpy(pszText, CastItoXBytes(lpPartFile->GetFileSize(), false, false), cchTextMax);
			break;

		case 2:		// transferred
			_tcsncpy(pszText, CastItoXBytes(lpPartFile->GetTransferred(), false, false), cchTextMax);
			break;

		case 3:		// transferred complete
			_tcsncpy(pszText, CastItoXBytes(lpPartFile->GetCompletedSize(), false, false), cchTextMax);
			break;

		case 4:		// speed
			if (lpPartFile->GetTransferringSrcCount())
				_tcsncpy(pszText, CastItoXBytes(lpPartFile->GetDownloadDatarate(), false, true), cchTextMax);
			break;

		case 5: 	// progress
			_sntprintf(pszText, cchTextMax, _T("%.2f%%"), lpPartFile->GetPercentCompleted());
			break;

		case 6:	{	// sources
			CString strBuffer;
			size_t sc = lpPartFile->GetSourceCount();
// ZZ:DownloadManager -->
			if (!(lpPartFile->GetStatus() == PS_PAUSED && sc == 0) && lpPartFile->GetStatus() != PS_COMPLETE)
			{
				size_t ncsc = lpPartFile->GetNotCurrentSourcesCount();
				strBuffer.Format(_T("%i"), sc - ncsc);
				if (ncsc > 0)
					strBuffer.AppendFormat(_T("/%i"), sc);
				if (thePrefs.IsExtControlsEnabled() && lpPartFile->GetSrcA4AFCount() > 0)
					strBuffer.AppendFormat(_T("+%i"), lpPartFile->GetSrcA4AFCount());
				if (lpPartFile->GetTransferringSrcCount() > 0)
					strBuffer.AppendFormat(_T(" (%i)"), lpPartFile->GetTransferringSrcCount());
			}
// <-- ZZ:DownloadManager
			if (thePrefs.IsExtControlsEnabled() && lpPartFile->GetPrivateMaxSources() != 0)
				strBuffer.AppendFormat(_T(" [%i]"), lpPartFile->GetPrivateMaxSources());
			_tcsncpy(pszText, strBuffer, cchTextMax);
			break;
		}

		case 7:		// prio
		{
			static const UINT StrID[]={IDS_PRIOAUTOLOW,IDS_PRIOLOW,IDS_PRIOAUTONORMAL,IDS_PRIONORMAL,IDS_PRIOAUTOHIGH,IDS_PRIOHIGH};
			INT_PTR downpriority=lpPartFile->GetDownPriority()<<1;
			ASSERT(	PR_LOW==0 && PR_NORMAL== 1 && PR_HIGH == 2 && downpriority>=PR_LOW && downpriority<=2*PR_HIGH);
			if(!lpPartFile->IsAutoDownPriority())
				++downpriority;
			_tcsncpy(pszText, GetResString(StrID[downpriority]), cchTextMax);
		}
			break;

		case 8:
			_tcsncpy(pszText, lpPartFile->getPartfileStatus(), cchTextMax);
			break;

		case 9:		// remaining time & size
			if (lpPartFile->GetStatus() != PS_COMPLETING && lpPartFile->GetStatus() != PS_COMPLETE)
			{
				uint_ptr restTime;
				if (!thePrefs.UseSimpleTimeRemainingComputation())
					restTime = lpPartFile->getTimeRemaining();
				else
					restTime = lpPartFile->getTimeRemainingSimple();
				_sntprintf(pszText, cchTextMax, _T("%s (%s)"), CastSecondsToHM(restTime), CastItoXBytes(lpPartFile->leftsize, false, false));
			}
			break;

		case 10: {	// last seen complete
			CString strBuffer;
			if (lpPartFile->m_nCompleteSourcesCountLo == 0)
				strBuffer.Format(_T("< %u"), lpPartFile->m_nCompleteSourcesCountHi);
			else if (lpPartFile->m_nCompleteSourcesCountLo == lpPartFile->m_nCompleteSourcesCountHi)
				strBuffer.Format(_T("%u"), lpPartFile->m_nCompleteSourcesCountLo);
			else
				strBuffer.Format(_T("%u - %u"), lpPartFile->m_nCompleteSourcesCountLo, lpPartFile->m_nCompleteSourcesCountHi);

			_sntprintf(pszText, cchTextMax, _T("%s (%s)"), ((lpPartFile->lastseencomplete == NULL)?GetResString(IDS_NEVER):lpPartFile->lastseencomplete.Format(thePrefs.GetDateTimeFormat4Lists())), strBuffer);
			break;
		}

		case 11: // last receive
			_tcsncpy(pszText, ((lpPartFile->GetFileDate() != 0 && lpPartFile->GetCompletedSize() > (uint64)0) ? lpPartFile->GetCFileDate().Format(thePrefs.GetDateTimeFormat4Lists()) : GetResString(IDS_NEVER)), cchTextMax);
			break;

		case 12: // cat
			if (const_cast<CPartFile*>(lpPartFile)->GetCategory() != 0)
				_tcsncpy(pszText,  thePrefs.GetCategory(const_cast<CPartFile*>(lpPartFile)->GetCategory())->strTitle, cchTextMax);
			break;
		case 13: // added on
			_tcsncpy(pszText, ((lpPartFile->GetCrFileDate() != NULL) ? lpPartFile->GetCrCFileDate().Format(thePrefs.GetDateTimeFormat4Lists()) : _T("?")), cchTextMax);
			break;
	    case 14: //health
			break;
	}
	pszText[cchTextMax - 1] = _T('\0');
}

void CDownloadListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	RECT cur_rec = lpDrawItemStruct->rcItem;
	CMemoryDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	CPartFile* lpPartFile = (CPartFile*)lpDrawItemStruct->itemData;
	InitItemMemDC(dc, lpDrawItemStruct->rcItem, (lpPartFile->xState > PFS_NORMAL)?((lpPartFile->xState == POFC_WAITING)?RGB(200,255,200)/*POFC*/:RGB(255,250,200)/*Import Parts*/):((lpDrawItemStruct->itemID % 2)?m_crEvenLine:m_crWindow), lpDrawItemStruct->itemState);

	RECT rcClient;// X: [DDHC] - [Don't Draw Hidden Column]
	GetClientRect(&rcClient);

	CtrlItem_Struct *content = (CtrlItem_Struct *)lpDrawItemStruct->itemData;

	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();

	cur_rec.right = cur_rec.left - sm_iSubItemInset;
	cur_rec.left += sm_iSubItemInset;
	if (!g_bLowColorDesktop && (lpDrawItemStruct->itemState & ODS_SELECTED) == 0) {
		DWORD dwCatColor = thePrefs.GetCatColor(lpPartFile->GetCategory(), COLOR_WINDOWTEXT);
		if (dwCatColor > 0)
			dc.SetTextColor(dwCatColor);
	}

	for (int iCurrent = 0; iCurrent < iCount; iCurrent++)
	{
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
		if(IsColumnHidden(iColumn)) continue;
		UINT uDrawTextAlignment;
		int iColumnWidth = GetColumnWidth(iColumn, uDrawTextAlignment);
		cur_rec.right += iColumnWidth;
		if(iColumnWidth > 2*sm_iSubItemInset && cur_rec.right>0){// X: [DDHC] - [Don't Draw Hidden Column]
			TCHAR szItem[1024];
			GetItemDisplayText(lpPartFile, iColumn, szItem, _countof(szItem));
			switch(iColumn){
				case 0:{	// file name
                    CRect rcDraw(cur_rec);
					int iIconPosY = (rcDraw.Height() > 19) ? ((rcDraw.Height() - 16) / 2) : 1;
					//int iImage = theApp.GetFileTypeSystemImageIdx(szItem/*lpPartFile->GetFileName()*/);
					//if (theApp.GetSystemImageList() != NULL) //morph4u no system image 
					//	::ImageList_Draw(theApp.GetSystemImageList(), iImage, dc.GetSafeHdc(), rcDraw.left, rcDraw.top + iIconPosY, ILD_TRANSPARENT);
					//rcDraw.left += theApp.GetSmallSytemIconSize().cx + sm_iLabelOffset;
            // Status Icons :: Start
				uint8 iIcon;
                switch (lpPartFile->GetPartFileStatus()){
					case PS_DOWNLOADING:
						if (lpPartFile->IsReadyForPreview() && lpPartFile->IsMovie())
                        iIcon = 5;
						else
						iIcon = 0;
						break;
                    case PS_TICK:
						iIcon = 7;
						break;
                    case PS_COMPLETING:
						iIcon = 4;
						break;
					case PS_WAITINGFORSOURCE:
						if (lpPartFile->IsReadyForPreview() && lpPartFile->IsMovie())
                        iIcon = 6;
						else
						iIcon = 3;
						break;
					case PS_PAUSED:
						iIcon = 1;
						break;			
					case PS_STOPPED:
					default:
						iIcon = 2;
						break;
				}

				POINT ipoint= {rcDraw.left-2, rcDraw.top+1}; 
				m_ImageList.Draw(dc, iIcon, ipoint, ILD_TRANSPARENT); 
				rcDraw.left += 18; 
			// Status Icons :: End
					dc->DrawText(szItem, -1, &rcDraw, MLC_DT_TEXT);
					break;
				}

// morph4u :: PercentBar :: Start
        case 5: {	// progress
			    cur_rec.bottom--;
			    cur_rec.top++;
				COLORREF crOldBackColor = dc.GetBkColor();
			 if (thePrefs.IsProgressBar()){
				lpPartFile->DrawProgressBar(dc, &cur_rec, thePrefs.UseFlatBar()); 
				}
				dc->SetBkColor(crOldBackColor); //Xman Code Improvement: FillSolidRect
			 if (szItem[0] != 0) {
				    COLORREF oldclr = thePrefs.IsProgressBar()?dc->SetTextColor(RGB(99, 99, 99)):dc->SetTextColor(RGB(0, 0, 0));
				int iOMode = dc->SetBkMode(TRANSPARENT);
				_sntprintf(szItem, _countof(szItem), _T("%.2f%%"), lpPartFile->GetPercentCompleted());
				szItem[_countof(szItem) - 1] = _T('\0');
			 if (thePrefs.IsProgressBar()){
			    dc->DrawText(szItem, -1, &cur_rec, (MLC_DT_TEXT & ~DT_LEFT) | DT_CENTER);
			 }else{
			    dc->DrawText(szItem, -1, &cur_rec, (MLC_DT_TEXT & ~DT_LEFT) | DT_RIGHT);
				}
				dc->SetBkMode(iOMode);
				dc->SetTextColor(oldclr);
			    }
			    cur_rec.bottom++;
			    cur_rec.top--;
			    break;
		}
// morph4u :: PercentBar :: End
		
				case 14:{ //health
					int available = lpPartFile->GetAvailablePartCount()*100/lpPartFile->GetPartCount(); //get available parts in %
					POINT pt = {cur_rec.left, cur_rec.top};
					if (available >= 25) // 25%-49% display first bar
					{
						    m_ImageList.Draw(dc, 8, pt, ILD_NORMAL);
							pt.x += 10;
					}
					else
					{
						    m_ImageList.Draw(dc, 12, pt, ILD_NORMAL);
						    pt.x += 10;
					}
				    if (available >= 50) // 50%-74% display second bar
					{
							m_ImageList.Draw(dc, 9, pt, ILD_NORMAL);
						    pt.x += 10;
					}
					else
					{
						    m_ImageList.Draw(dc, 13, pt, ILD_NORMAL);
						    pt.x += 10;
					}
					if (available >= 75) // 75%-99% display third bar
					{
						    m_ImageList.Draw(dc, 10, pt, ILD_NORMAL);
						    pt.x += 10;
					}
					else
					{
						    m_ImageList.Draw(dc, 14, pt, ILD_NORMAL);
						    pt.x += 10;
					}
					if (available == 100) // 100% display all fours
						    m_ImageList.Draw(dc, 11, pt, ILD_NORMAL);
					else
						    m_ImageList.Draw(dc, 15, pt, ILD_NORMAL);

					break;
			   }
		
				default:
					if(szItem[0] != 0)
						dc->DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
					break;
			}
		}
		cur_rec.left += iColumnWidth;
		if(cur_rec.left>=rcClient.right)// X: [DDHC] - [Don't Draw Hidden Column]
			break;
	}
}

void CDownloadListCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if (point.x != -1 || point.y != -1) {// X: [BF] - [Bug Fix]
		RECT rcClient;
		GetClientRect(&rcClient);
		ClientToScreen(&rcClient);
		if (!PtInRect(&rcClient,point)) {
			Default();
			return;
		}
	}
	int iSel = GetNextItem(-1, LVIS_SELECTED);
	if (iSel != -1)
	{
		// get merged settings
		bool bFirstItem = true;
		size_t iSelectedItems = 0;
		size_t iFilesNotDone = 0;
		size_t iFilesToPause = 0;
		size_t iFilesToStop = 0;
		size_t iFilesToResume = 0;
		size_t iFilesToOpen = 0;
        size_t iFilesGetPreviewParts = 0;
           size_t iFilesPreviewType = 0;
		size_t iFilesToPreview = 0;
		size_t iFilesToCancel = 0;
        size_t iFilesCanPauseOnPreview = 0;
	    size_t iFilesDoPauseOnPreview = 0;
		size_t iFilesInCats = 0;
		size_t iFilesA4AFAuto = 0; //Xman Xtreme Downloadmanager: Auto-A4AF-check
		UINT uPrioMenuItem = 0;
		const CPartFile* file1 = NULL;
		POSITION pos = GetFirstSelectedItemPosition();
		while (pos)
		{
			const CPartFile* pFile = (CPartFile*)GetItemData(GetNextSelectedItem(pos));
			if (bFirstItem)
				file1 = pFile;
			iSelectedItems++;

			bool bFileDone = (pFile->GetStatus()==PS_COMPLETE || pFile->GetStatus()==PS_COMPLETING);
			iFilesToCancel += pFile->GetStatus() != PS_COMPLETING ? 1 : 0;
			iFilesNotDone += !bFileDone ? 1 : 0;
			iFilesToStop += pFile->CanStopFile() ? 1 : 0;
			iFilesToPause += pFile->CanPauseFile() ? 1 : 0;
			iFilesToResume += pFile->CanResumeFile() ? 1 : 0;
			//iFilesToOpen += pFile->CanOpenFile() ? 1 : 0;
            iFilesGetPreviewParts += pFile->GetPreviewPrio() ? 1 : 0;
            iFilesPreviewType += pFile->IsPreviewableFileType() ? 1 : 0;
			iFilesToPreview += pFile->IsReadyForPreview() ? 1 : 0;
	        iFilesCanPauseOnPreview += (pFile->IsPreviewableFileType() && !pFile->IsReadyForPreview() && pFile->CanPauseFile()) ? 1 : 0;
			iFilesDoPauseOnPreview += (pFile->IsPausingOnPreview()) ? 1 : 0;
			iFilesInCats += (pFile->GetConstCategory() != 0) ? 1 : 0; 
			iFilesA4AFAuto += (!bFileDone && pFile->IsA4AFAuto()) ? 1 : 0; //Xman Xtreme Downloadmanager: Auto-A4AF-check

			UINT uCurPrioMenuItem = 0;
			if (pFile->IsAutoDownPriority())
				uCurPrioMenuItem = MP_PRIOAUTO;
			else if (pFile->GetDownPriority() == PR_HIGH)
				uCurPrioMenuItem = MP_PRIOHIGH;
			else if (pFile->GetDownPriority() == PR_NORMAL)
				uCurPrioMenuItem = MP_PRIONORMAL;
			else if (pFile->GetDownPriority() == PR_LOW)
				uCurPrioMenuItem = MP_PRIOLOW;
			else
				ASSERT(0);

				if (bFirstItem){
				uPrioMenuItem = uCurPrioMenuItem;
					bFirstItem = false;
				}
				else if (uPrioMenuItem != uCurPrioMenuItem)
					uPrioMenuItem = 0;

		}
		//Xman from Stulle
		m_FileMenu.EnableMenuItem((UINT)m_DropMenu.m_hMenu, (iSelectedItems > 0 && iFilesToStop > 0) ? MF_ENABLED : MF_GRAYED); // enable only when it makes sense - Stulle
		//Xman end
		m_FileMenu.EnableMenuItem((UINT)m_PrioMenu.m_hMenu, iFilesNotDone > 0 ? MF_ENABLED : MF_GRAYED);
		m_PrioMenu.CheckMenuRadioItem(MP_PRIOLOW, MP_PRIOAUTO, uPrioMenuItem, 0);

		// enable commands if there is at least one item which can be used for the action
		m_FileMenu.EnableMenuItem(MP_CANCEL, iFilesToCancel > 0 ? MF_ENABLED : MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_STOP, iFilesToStop > 0 ? MF_ENABLED : MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_PAUSE, iFilesToPause > 0 ? MF_ENABLED : MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_RESUME, iFilesToResume > 0 ? MF_ENABLED : MF_GRAYED);
		
		//bool bOpenEnabled = (iSelectedItems == 1 && iFilesToOpen == 1);
		//m_FileMenu.EnableMenuItem(MP_OPEN, bOpenEnabled ? MF_ENABLED : MF_GRAYED);
		
		bool bDetailsEnabled = (iSelectedItems > 0);
		m_FileMenu.EnableMenuItem(MP_METINFO, bDetailsEnabled ? MF_ENABLED : MF_GRAYED);
		m_FileMenu.SetDefaultItem(bDetailsEnabled?MP_METINFO:(UINT)-1);

		m_FileMenu.EnableMenuItem(thePrefs.m_bShowCopyEd2kLinkCmd ? MP_GETED2KLINK : MP_SHOWED2KLINK, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_PASTE, theApp.IsEd2kFileLinkInClipboard() ? MF_ENABLED : MF_GRAYED);
		//m_FileMenu.EnableMenuItem(MP_SEARCHRELATED, theApp.emuledlg->searchwnd->CanSearchRelatedFiles() ? MF_ENABLED : MF_GRAYED);

			CMenu PreviewWithMenu;
			PreviewWithMenu.CreateMenu();
			size_t iPreviewMenuEntries = thePreviewApps.GetAllMenuEntries(PreviewWithMenu, (iSelectedItems == 1) ? file1 : NULL);

		if( thePrefs.IsExtControlsEnabled()){
			if(!thePrefs.GetPreviewPrio()) {
					m_FileMenu.EnableMenuItem(MP_TRY_TO_GET_PREVIEW_PARTS, (iSelectedItems == 1 && iFilesPreviewType == 1 && iFilesToPreview == 0 && iFilesNotDone == 1) ? MF_ENABLED : MF_GRAYED);
				m_FileMenu.CheckMenuItem(MP_TRY_TO_GET_PREVIEW_PARTS, (iSelectedItems == 1 && iFilesGetPreviewParts == 1) ? MF_CHECKED : MF_UNCHECKED);
			}
				m_FileMenu.EnableMenuItem(MP_PREVIEW, (iSelectedItems == 1 && iFilesToPreview == 1) ? MF_ENABLED : MF_GRAYED);
				m_FileMenu.EnableMenuItem(MP_PAUSEONPREVIEW, iFilesCanPauseOnPreview > 0 ? MF_ENABLED : MF_GRAYED);
				m_FileMenu.CheckMenuItem(MP_PAUSEONPREVIEW, (iSelectedItems > 0 && iFilesDoPauseOnPreview == iSelectedItems) ? MF_CHECKED : MF_UNCHECKED);
				if (iPreviewMenuEntries > 0 /*&& !thePrefs.GetExtraPreviewWithMenu()*/)
					m_FileMenu.InsertMenu(1, MF_POPUP | MF_BYPOSITION | (iSelectedItems == 1 ? MF_ENABLED : MF_GRAYED), (UINT_PTR)PreviewWithMenu.m_hMenu, GetResString(IDS_PREVIEWWITH));
				/*else if (iPreviewMenuEntries > 0)
					m_FileMenu.InsertMenu(MP_METINFO, MF_POPUP | MF_BYCOMMAND | (iSelectedItems == 1 ? MF_ENABLED : MF_GRAYED), (UINT_PTR)PreviewWithMenu.m_hMenu, GetResString(IDS_PREVIEWWITH));*/
			if (m_SourcesMenu) {
				m_FileMenu.EnableMenuItem((UINT)m_SourcesMenu.m_hMenu, MF_ENABLED);
				m_SourcesMenu.CheckMenuItem(MP_ALL_A4AF_AUTO, (iSelectedItems == 1 && iFilesNotDone == 1 && iFilesA4AFAuto == 1) ? MF_CHECKED : MF_UNCHECKED); //Xman Xtreme Downloadmanager: Auto-A4AF-check
				m_SourcesMenu.EnableMenuItem(MP_ADDSOURCE, (iSelectedItems == 1 && iFilesToStop == 1) ? MF_ENABLED : MF_GRAYED);
				m_SourcesMenu.EnableMenuItem(MP_SETSOURCELIMIT, (iFilesNotDone == iSelectedItems) ? MF_ENABLED : MF_GRAYED);
				m_SourcesMenu.EnableMenuItem(MP_C0SC, iFilesToStop > 0 ? MF_ENABLED : MF_GRAYED);// X: [C0SC] - [Clear0SpeedClient]
			}
			m_FileMenu.EnableMenuItem(MP_FLUSHBUFFER, (iFilesToStop > 0 && !file1->IsAllocating()) ? MF_ENABLED : MF_GRAYED); // X: [FB] - [FlushBuffer]
			m_FileMenu.EnableMenuItem(MP_PREALOCATE, (iFilesNotDone > 0 && file1->IncompleteAllocateSpace()) ? MF_ENABLED : MF_GRAYED); 
			m_FileMenu.AppendMenu(MF_STRING | (iSelectedItems == 1 && iFilesToStop ==1 && ((CPartFile*)file1)->GetAICHRecoveryHashSet()->CanTrustMajority()) ? MF_ENABLED : MF_GRAYED, MP_AICHHASH, _T("Trust AICH Hash"));
		}
			else {
				m_FileMenu.EnableMenuItem(MP_PREVIEW, (iSelectedItems == 1 && iFilesToPreview == 1) ? MF_ENABLED : MF_GRAYED);
				if (iPreviewMenuEntries)
					m_FileMenu.InsertMenu(MP_METINFO, MF_POPUP | MF_BYCOMMAND | (iSelectedItems == 1 ? MF_ENABLED : MF_GRAYED), (UINT_PTR)PreviewWithMenu.m_hMenu, GetResString(IDS_PREVIEWWITH));
			}


		// create cat-submenue
		CMenu CatsMenu;
		CatsMenu.CreateMenu();
	    FillCatsMenu(CatsMenu, iFilesInCats);
		m_FileMenu.AppendMenu(MF_POPUP, (UINT_PTR)CatsMenu.m_hMenu, GetResString(IDS_TOCAT));
//Xman checkmark to catogory at contextmenu of downloadlist
			if(iSelectedItems == 1)
				CatsMenu.CheckMenuItem(MP_ASSIGNCAT+file1->GetConstCategory(),MF_CHECKED);
			//Xman end
		GetPopupMenuPos(*this, point);
		m_FileMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
		if( thePrefs.IsExtControlsEnabled())
			VERIFY( m_FileMenu.RemoveMenu(m_FileMenu.GetMenuItemCount() - 1, MF_BYPOSITION) );// X: [IP] - [Import Parts]
		    VERIFY( m_FileMenu.RemoveMenu(m_FileMenu.GetMenuItemCount() - 1, MF_BYPOSITION) );
        if (iPreviewMenuEntries && thePrefs.IsExtControlsEnabled())
			VERIFY( m_FileMenu.RemoveMenu((UINT)PreviewWithMenu.m_hMenu, MF_BYCOMMAND) );
		else if (iPreviewMenuEntries)
			VERIFY( m_FileMenu.RemoveMenu((UINT)PreviewWithMenu.m_hMenu, MF_BYCOMMAND) );
			VERIFY( CatsMenu.DestroyMenu() );
			VERIFY( PreviewWithMenu.DestroyMenu() );
       	    }
	    else{	// nothing selected
		//Xman from Stulle
		m_FileMenu.EnableMenuItem((UINT)m_DropMenu.m_hMenu, MF_GRAYED); // enable only when it makes sense - Stulle
		//Xman end
		m_FileMenu.EnableMenuItem((UINT)m_PrioMenu.m_hMenu, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_CANCEL, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_PAUSE, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_STOP, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_RESUME, MF_GRAYED);
		//m_FileMenu.EnableMenuItem(MP_OPEN, MF_GRAYED);

		if (thePrefs.IsExtControlsEnabled()) {
			if (!thePrefs.GetPreviewPrio())
			{
				m_FileMenu.EnableMenuItem(MP_TRY_TO_GET_PREVIEW_PARTS, MF_GRAYED);
				m_FileMenu.CheckMenuItem(MP_TRY_TO_GET_PREVIEW_PARTS, MF_UNCHECKED);
			}
			m_FileMenu.EnableMenuItem(MP_PREVIEW, MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_PAUSEONPREVIEW, MF_GRAYED);
        }
		else {
		m_FileMenu.EnableMenuItem(MP_PREVIEW, MF_GRAYED);
		}
		m_FileMenu.EnableMenuItem(MP_METINFO, MF_GRAYED);
		m_FileMenu.EnableMenuItem(thePrefs.GetShowCopyEd2kLinkCmd() ? MP_GETED2KLINK : MP_SHOWED2KLINK, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_PASTE, theApp.IsEd2kFileLinkInClipboard() ? MF_ENABLED : MF_GRAYED);
        //m_FileMenu.EnableMenuItem(MP_SEARCHRELATED, MF_GRAYED);
		m_FileMenu.SetDefaultItem((UINT)-1);
		if (m_SourcesMenu){
				m_FileMenu.EnableMenuItem((UINT)m_SourcesMenu.m_hMenu, MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_FLUSHBUFFER, MF_GRAYED);// X: [FB] - [FlushBuffer]
			m_FileMenu.EnableMenuItem(MP_PREALOCATE, MF_GRAYED);
		}

		GetPopupMenuPos(*this, point);
		m_FileMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
	}
}

void CDownloadListCtrl::FillCatsMenu(CMenu& rCatsMenu, size_t iFilesInCats)
{
	ASSERT(rCatsMenu.m_hMenu);
	if (iFilesInCats == (-1))
	{
		iFilesInCats = 0;
		int iSel = GetNextItem(-1, LVIS_SELECTED);
		if (iSel != -1)
		{
			const CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(iSel);
			//if (content != NULL && content->type == FILE_TYPE)
			//{
                CAtlList<CPartFile*> selectedList; //morph4u added instead FILE_TYPE
				POSITION pos = GetFirstSelectedItemPosition();
				while (pos)
				{
					const CtrlItem_Struct* pItemData = (CtrlItem_Struct*)GetItemData(GetNextSelectedItem(pos));
	            if (pItemData == NULL/*|| pItemData->type != FILE_TYPE*/)
						continue;
					const CPartFile* pFile = (CPartFile*)pItemData;
					iFilesInCats += (pFile->GetConstCategory() != 0) ? 1 : 0; 
				}
			//}
		}
	}
	rCatsMenu.AppendMenu(MF_STRING, MP_NEWCAT, GetResString(IDS_NEW) + _T("..."));	
	CString label = GetResString(IDS_CAT_UNASSIGN);
	label.Remove('(');
	label.Remove(')'); // Remove brackets without having to put a new/changed ressource string in
	rCatsMenu.AppendMenu(MF_STRING | ((iFilesInCats == 0) ? MF_GRAYED : MF_ENABLED), MP_ASSIGNCAT, label);
	if (thePrefs.GetCatCount() > 1)
	{
		rCatsMenu.AppendMenu(MF_SEPARATOR);
			for (size_t i = 1; i < thePrefs.GetCatCount(); i++){
				label=thePrefs.GetCategory(i)->strTitle;
				label.Replace(_T("&"), _T("&&") );
			rCatsMenu.AppendMenu(MF_STRING, MP_ASSIGNCAT + i, label);
		}
			}
		}

BOOL CDownloadListCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	wParam = LOWORD(wParam);

	switch (wParam)
	{
		case MP_PASTE:
			if (theApp.IsEd2kFileLinkInClipboard())
				theApp.PasteClipboard(curTab);// X: [AC] - [ActionChange] use transferwnd's select cat as default cat
			return TRUE;
	}

	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel == -1)
		iSel = GetNextItem(-1, LVIS_SELECTED);
	if (iSel != -1)
	{
		CPartFile* file = (CPartFile*)GetItemData(iSel);
		//for multiple selections 
		size_t selectedCount = 0;
		CAtlList<CPartFile*> selectedList; 
		POSITION pos = GetFirstSelectedItemPosition();
		while(pos != NULL) 
		{ 
			int index = GetNextSelectedItem(pos);
			if(index > -1) 
			{
				selectedCount++;
				selectedList.AddTail((CPartFile*)GetItemData(index));
			} 
		} 

		switch (wParam)
		{
			case MP_CANCEL:
			case MPG_DELETE: // keyboard del will continue to remove completed files from the screen while cancel will now also be available for complete files
			{
				if (selectedCount > 0)
				{
					SetRedraw(false);
					CString fileList;
					bool validdelete = false;
					bool removecompl = false;
					int cFiles = 0;
					const int iMaxDisplayFiles = 10;
					for (pos = selectedList.GetHeadPosition(); pos != 0; )
					{
						CPartFile* cur_file = selectedList.GetNext(pos);
						if (cur_file->GetStatus() != PS_COMPLETING && (cur_file->GetStatus() != PS_COMPLETE || wParam == MP_CANCEL)){
							validdelete = true;
							cFiles++;
							if (cFiles < iMaxDisplayFiles)
								fileList.Append(_T('\n') + CString(cur_file->GetFileName()));
							else if(cFiles == iMaxDisplayFiles && pos != NULL)
								fileList.Append(_T("\n..."));
						}
						else if (cur_file->GetStatus() == PS_COMPLETE)
							removecompl = true;
					}
					CString quest = GetResString((selectedCount == 1)?IDS_Q_CANCELDL2:IDS_Q_CANCELDL);// X: [CI] - [Code Improvement]
					if ((removecompl && !validdelete) || (validdelete && AfxMessageBox(quest + fileList, MB_DEFBUTTON2 | MB_ICONQUESTION | MB_YESNO) == IDYES))
					{
						bool bRemovedItems = false;
						while (!selectedList.IsEmpty())
						{
							CPartFile* partfile = selectedList.RemoveHead();// X: [CI] - [Code Improvement]
							switch (partfile->GetStatus())
							{
									//case PS_WAITINGFORHASH:
								//case PS_HASHING:
								case PS_COMPLETING:
									//bRemovedItems = true;
									break;
								case PS_COMPLETE:
									if (wParam == MP_CANCEL){
										bool delsucc = ShellDeleteFile(partfile->GetFilePath());
										if (delsucc){
											theApp.sharedfiles->RemoveFile(partfile, true);
											if(!thePrefs.m_bDisableHistoryList && !thePrefs.m_bHistoryShowShared){
												theApp.emuledlg->transferwnd->historylistctrl.AddFile(partfile);
												//theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2History);
											}
										}
										else{
											CString strError;
											strError.Format( GetResString(IDS_ERR_DELFILE) + _T("\r\n\r\n%s"), partfile->GetFilePath(), GetErrorMessage(GetLastError()));
											AfxMessageBox(strError);
										}
									}
									RemoveFile(partfile);
									bRemovedItems = true;
									break;
								case PS_PAUSED:
									partfile->DeleteFile();
									bRemovedItems = true;
									break;
								default:
									if (/*partfile->GetCategory() && */partfile->xState!=POFC_WAITING)// X: [POFC] - [PauseOnFileComplete]
										theApp.downloadqueue->StartNextFileIfPrefs(partfile->GetCategory());
									partfile->DeleteFile();
									bRemovedItems = true;
									break;
							}
						}
						if (bRemovedItems)
						{
							AutoSelectItem();
							theApp.emuledlg->transferwnd->UpdateCatTabTitles();
						}
					}
					SetRedraw(true);
				}
				break;
			}
			case MP_PRIOHIGH:
				SetRedraw(false);
				while (!selectedList.IsEmpty()){
					CPartFile* partfile = selectedList.RemoveHead();// X: [CI] - [Code Improvement]
					partfile->SetAutoDownPriority(false);
					partfile->SetDownPriority(PR_HIGH);
				}
				SetRedraw(true);
				break;
			case MP_PRIOLOW:
				SetRedraw(false);
				while (!selectedList.IsEmpty()){
					CPartFile* partfile = selectedList.RemoveHead();// X: [CI] - [Code Improvement]
					partfile->SetAutoDownPriority(false);
					partfile->SetDownPriority(PR_LOW);
				}
				SetRedraw(true);
				break;
			case MP_PRIONORMAL:
				SetRedraw(false);
				while (!selectedList.IsEmpty()){
					CPartFile* partfile = selectedList.RemoveHead();// X: [CI] - [Code Improvement]
					partfile->SetAutoDownPriority(false);
					partfile->SetDownPriority(PR_NORMAL);
				}
				SetRedraw(true);
				break;
			case MP_PRIOAUTO:
				SetRedraw(false);
				while (!selectedList.IsEmpty()){
					CPartFile* partfile = selectedList.RemoveHead();// X: [CI] - [Code Improvement]
					partfile->SetAutoDownPriority(true);
					partfile->SetDownPriority(PR_HIGH);
				}
				SetRedraw(true);
				break;
			case MP_C0SC:// X: [C0SC] - [Clear0SpeedClient]
				SetRedraw(false);
				while (!selectedList.IsEmpty()){
					CPartFile* partfile = selectedList.RemoveHead();
					partfile->ClearClient();
				}
				SetRedraw(true);
				break;

			case MP_PAUSE:
				SetRedraw(false);
				while (!selectedList.IsEmpty()){
					CPartFile* partfile = selectedList.RemoveHead();// X: [CI] - [Code Improvement]
					if (partfile->CanPauseFile()){
						partfile->PauseFile();
					}
				}
				SetRedraw(true);
				break;
			case MP_RESUME:
				SetRedraw(false);
				while (!selectedList.IsEmpty()){
					CPartFile* partfile = selectedList.RemoveHead();// X: [CI] - [Code Improvement]
					if (partfile->CanResumeFile()){
						if (partfile->GetStatus() == PS_INSUFFICIENT)
							partfile->ResumeFileInsufficient();
						else
							partfile->ResumeFile();
					}
				}
				SetRedraw(true);
				break;
			case MP_STOP:
				SetRedraw(false);
				while (!selectedList.IsEmpty()){
					CPartFile *partfile = selectedList.RemoveHead();// X: [CI] - [Code Improvement]
					if (partfile->CanStopFile()){
						partfile->StopFile();
					}
				}
				SetRedraw(true);
				theApp.emuledlg->transferwnd->UpdateCatTabTitles();
				break;

			case MP_AICHHASH:// X: [IP] - [Import Parts]
				if (selectedCount == 1)
					file->GetAICHRecoveryHashSet()->TrustMajority();
				break;
			case MP_FLUSHBUFFER:{// X: [FB] - [FlushBuffer]
				SetRedraw(false);
				DWORD curTick = ::GetTickCount();
				while (!selectedList.IsEmpty()){
					CPartFile *partfile = selectedList.RemoveHead();
					// Avoid flushing while copying preview file
						if((partfile->m_nTotalBufferData > 0) && !partfile->m_bPreviewing){
						partfile->m_nNextFlushBufferTime = curTick;
						curTick += 1000;
					}
				}
				SetRedraw(true);
				break;
			}
			//Xman manual file allocation (Xanatos)
			case MP_PREALOCATE:
				while (!selectedList.IsEmpty()){
					CPartFile *partfile = selectedList.RemoveHead();// X: [CI] - [Code Improvement]
					if(partfile->IncompleteAllocateSpace())
						partfile->AllocateNeededSpace();
				}
				break;
			//Xman end

			//Xman Xtreme Downloadmanager
			case MP_ALL_A4AF_TO_THIS:
				{
					SetRedraw(false);
					bool redraw = false;
					if (selectedCount == 1 
						&& (file->GetStatus(false) == PS_READY || file->GetStatus(false) == PS_EMPTY))
					{
						//theApp.downloadqueue->DisableAllA4AFAuto();

						for (POSITION pos = file->A4AFsrclist.GetHeadPosition();pos!=NULL;){ // X: [CI] - [Code Improvement]
							CUpDownClient *cur_source = file->A4AFsrclist.GetNext(pos);
							if( cur_source->GetDownloadState() != DS_DOWNLOADING
								&& cur_source->GetRequestFile() 
								&& ( (!cur_source->GetRequestFile()->IsA4AFAuto()) || cur_source->GetDownloadState() == DS_NONEEDEDPARTS) //Xman Xtreme Downloadmanager: Auto-A4AF-check
								&& !cur_source->IsSwapSuspended(file) )
							{
								redraw = cur_source->SwapToAnotherFile(true, false, false, file,true);
							}
						}

					}
					SetRedraw(true);
					if(redraw)
					RefreshFile(file);						
					break;
				}
			case MP_DROPNONEEDEDSRCS: { 
				if(selectedCount > 1){
					while (!selectedList.IsEmpty()) {
						selectedList.RemoveHead()->RemoveNoNeededPartsSources();//DS_NONEEDEDPARTS DL-6// X: [CI] - [Code Improvement]
					}
					break;
				}
				file->RemoveNoNeededPartsSources();
				break;					
									  }
			case MP_DROPQUEUEFULLSRCS: { 
				if(selectedCount > 1){
					while (!selectedList.IsEmpty()) {
						selectedList.RemoveHead()->RemoveQueueFullSources();// X: [CI] - [Code Improvement]
					}
					break;
				}
				file->RemoveQueueFullSources();
				break;
		   }
		   
			case MP_ALL_A4AF_TO_OTHER:
				{
					SetRedraw(false);

					if (selectedCount == 1 
						&& (file->GetStatus(false) == PS_READY || file->GetStatus(false) == PS_EMPTY))
					{
						//theApp.downloadqueue->DisableAllA4AFAuto();
						for(POSITION pos = file->srclist.GetHeadPosition(); pos != NULL; ){
							CUpDownClient* cur_src = file->srclist.GetNext(pos);
							if(cur_src->GetDownloadState() != DS_DOWNLOADING)// X: skip downloading source
								cur_src->SwapToAnotherFile(false, false, false, NULL,true);
						}
					}
					SetRedraw(true);
					break;
				}
			//Xman end
			//Xman Xtreme Downloadmanager: Auto-A4AF-check
			case MP_ALL_A4AF_AUTO:
				file->SetA4AFAuto(!file->IsA4AFAuto());
				break;
			//Xman end

			case MPG_F2:
				if (GetAsyncKeyState(VK_CONTROL) < 0 || selectedCount > 1) {
					// when ctrl is pressed -> filename cleanup
					if (IDYES==AfxMessageBox(GetResString(IDS_MANUAL_FILENAMECLEANUP),MB_YESNO))
						while (!selectedList.IsEmpty()){
							CPartFile *partfile = selectedList.RemoveHead();// X: [CI] - [Code Improvement]
							if (partfile->IsPartFile()) {
								partfile->SetFileName(CleanupFilename(partfile->GetFileName()));
							}
						}
				} else {
					if (file->GetStatus() != PS_COMPLETE && file->GetStatus() != PS_COMPLETING)
					{
						InputBox inputbox;
						CString title = GetResString(IDS_RENAME);
						title.Remove(_T('&'));
						inputbox.SetLabels(title, GetResString(IDS_DL_FILENAME), file->GetFileName());
						inputbox.SetEditFilenameMode();
						if (inputbox.DoModal()==IDOK && !inputbox.GetInput().IsEmpty() && IsValidEd2kString(inputbox.GetInput()))
						{
							file->SetFileName(inputbox.GetInput(), true);
							//file->UpdateDisplayedInfo();
							file->SavePartFile();
						}
					}
					else
						MessageBeep(MB_OK);
				}
				break;
			case MP_METINFO:
			case MPG_ALTENTER:
				ShowFileDialog(0);
				break;
			case MP_COPYSELECTED:
			case MP_GETED2KLINK:{
				CString str;
				while (!selectedList.IsEmpty()){
					if (!str.IsEmpty())
						str += _T("\r\n");
					str += ((CAbstractFile*)selectedList.RemoveHead())->GetED2kLink();// X: [CI] - [Code Improvement]
				}
				theApp.CopyTextToClipboard(str);
				break;
			}
			/*case MP_SEARCHRELATED:{
				CAtlList<CAbstractFile*> abstractFileList;
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
					abstractFileList.AddTail(selectedList.GetNext(pos));
				theApp.emuledlg->searchwnd->SearchRelatedFiles(abstractFileList);
				theApp.emuledlg->SetActiveDialog(theApp.emuledlg->searchwnd);
				break;
			}*/
			//case MP_OPEN:
			case IDA_ENTER:
				if (selectedCount > 1)
					break;
				if (file->CanOpenFile())
					file->OpenFile();
				break;
			case MP_TRY_TO_GET_PREVIEW_PARTS:
				if (selectedCount > 1)
					break;
                   file->SetPreviewPrio(!file->GetPreviewPrio());
                break;
			
				case MP_PREVIEW:
					if (selectedCount > 1)
						break;
					file->PreviewFile();
					break;
				case MP_PAUSEONPREVIEW:
				{
					bool bAllPausedOnPreview = true;
					for (pos = selectedList.GetHeadPosition(); pos != 0; )
						bAllPausedOnPreview = ((CPartFile*)selectedList.GetNext(pos))->IsPausingOnPreview() && bAllPausedOnPreview;
					while (!selectedList.IsEmpty()){
						CPartFile* pPartFile = selectedList.RemoveHead();
						if (pPartFile->IsPreviewableFileType() && !pPartFile->IsReadyForPreview())
							pPartFile->SetPauseOnPreview(!bAllPausedOnPreview);
						
					}					
					break;
				}		
			case MP_SHOWED2KLINK:
				ShowFileDialog(IDD_ED2KLINK);
				break;
			case MP_SETSOURCELIMIT: {
				CString temp;
				temp.Format(_T("%u"),file->GetPrivateMaxSources());
				InputBox inputbox;
				CString title = GetResString(IDS_SETPFSLIMIT);
				inputbox.SetLabels(title, GetResString(IDS_SETPFSLIMITEXPLAINED), temp );

				if (inputbox.DoModal() == IDOK)
				{
					temp = inputbox.GetInput();
					int newlimit = _tstoi(temp);
					while (!selectedList.IsEmpty()){
						CPartFile *partfile = selectedList.RemoveHead();// X: [CI] - [Code Improvement]
						partfile->SetPrivateMaxSources(newlimit);
						partfile->UpdateDisplayedInfo(true);
					}
				}
				break;
			}
			case MP_ADDSOURCE: {
				if (selectedCount > 1)
					break;
				// NEO: MLD - [ModelesDialogs] -- Xanatos -->
				CAddSourceDlg* dlg = new CAddSourceDlg();
				dlg->SetFile(file);
				dlg->OpenDialog(); 
				// NEO: MLD END <-- Xanatos --
				//CAddSourceDlg as;
				//as.SetFile(file);
				//as.DoModal();
				break;
			}
			default:
					 if ((wParam >= MP_ASSIGNCAT && wParam<=MP_ASSIGNCAT+99) || wParam == MP_NEWCAT){
						int nCatNumber;
						if (wParam == MP_NEWCAT)
						{
						nCatNumber = theApp.emuledlg->transferwnd->AddCategoryInteractive();
						if (nCatNumber == 0) // Creation canceled
						break;
						}
						else
						nCatNumber = (UINT)wParam - MP_ASSIGNCAT;
					SetRedraw(FALSE);
					while (!selectedList.IsEmpty()){
						CPartFile *partfile = selectedList.RemoveHead();// X: [CI] - [Code Improvement]
						partfile->SetCategory(nCatNumber);
						partfile->UpdateDisplayedInfo(true);
					}
					SetRedraw(TRUE);
					ReloadFileList();
					if (thePrefs.ShowCatTabInfos())
						theApp.emuledlg->transferwnd->UpdateCatTabTitles();
				}
                    else if (wParam>=MP_PREVIEW_APP_MIN && wParam<=MP_PREVIEW_APP_MAX){
					thePreviewApps.RunApp(file, (UINT)wParam);
					}
				break;
		}
	}

	return TRUE;
}

void CDownloadListCtrl::OnLvnColumnClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLISTVIEW *pNMListView = (NMLISTVIEW *)pNMHDR;
	bool sortAscending;
	if (GetSortItem() != pNMListView->iSubItem)
	{
		switch (pNMListView->iSubItem)
		{
			case 2: // Transferred
			case 3: // Completed
			case 4: // Download rate
			case 5: // Progress
			case 6: // Sources / Client Software
				sortAscending = false;
				break;
			/*case 9:
				// Keep the current 'm_bRemainSort' for that column, but reset to 'ascending'
				sortAscending = true;
				break;*/
			default:
				sortAscending = true;
				break;
		}
	}
	else
		sortAscending = !GetSortAscending();

	// Ornis 4-way-sorting
	int adder = 0;
	if (pNMListView->iSubItem == 9)
	{
		if (GetSortItem() == 9 && sortAscending) // check for 'ascending' because the initial sort order is also 'ascending'
			m_bRemainSort = !m_bRemainSort;
		adder = !m_bRemainSort ? 0 : 81;
	}

	// Sort table
	if (adder == 0)
		SetSortArrow(pNMListView->iSubItem, sortAscending);
	else
		SetSortArrow(pNMListView->iSubItem, sortAscending ? arrowDoubleUp : arrowDoubleDown);
	UpdateSortHistory(pNMListView->iSubItem + (sortAscending ? 0 : 100) + adder);
	SortItems(SortProc, pNMListView->iSubItem + (sortAscending ? 0 : 100) + adder);

	// Save new preferences
	thePrefs.TransferlistRemainSortStyle(m_bRemainSort);

	*pResult = 0;
}

int CDownloadListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	//int dwOrgSort = lParamSort; // SLUGFILLER: multiSort remove - handled in parent class
	LPARAM iColumn = (lParamSort >= 100) ? lParamSort - 100 : lParamSort;

	CPartFile* file1 = (CPartFile*)lParam1;
	CPartFile* file2 = (CPartFile*)lParam2;
	int iResult = 0;
	switch(iColumn)
	{
		case 0: //filename asc
			iResult = CompareLocaleStringNoCase(file1->GetFileName(),file2->GetFileName());
			break;
		case 1: //size asc
			iResult = CompareUnsigned64(file1->GetFileSize(), file2->GetFileSize());
			break;
		case 2: //transferred asc
			iResult = CompareUnsigned64(file1->GetTransferred(), file2->GetTransferred());
			break;
		case 3: //completed asc
			iResult = CompareUnsigned64(file1->GetCompletedSize(), file2->GetCompletedSize());
			break;
		case 4: //speed asc
			iResult = CompareUnsigned(file1->GetDownloadDatarate()/(2*1024), file2->GetDownloadDatarate()/(2*1024)); //Xman // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
			break;
		case 5: //progress asc
			iResult = CompareFloat(file1->GetPercentCompleted(), file2->GetPercentCompleted());
			break;
		case 6: //sources asc
			iResult = CompareUnsigned_PTR(file1->GetSourceCount(), file2->GetSourceCount());
			break;
		case 7: //priority asc
			iResult = CompareUnsigned(file1->GetDownPriority(), file2->GetDownPriority());
			break;
		case 8: //Status asc 
			iResult = CompareUnsigned_PTR(file1->getPartfileStatusRang(),file2->getPartfileStatusRang());
			break;

		case 9: //Remaining Time asc
		{
			//Make ascending sort so we can have the smaller remaining time on the top 
			//instead of unknowns so we can see which files are about to finish better..
			size_t f1, f2;
			if (!thePrefs.UseSimpleTimeRemainingComputation()){
				f1 = file1->getTimeRemaining();
				f2 = file2->getTimeRemaining();
			}
			else{
				f1 = file1->getTimeRemainingSimple();
				f2 = file2->getTimeRemainingSimple();
			}
			//Same, do nothing.
			if (f1 == f2)
				return 0;

			//If descending, put first on top as it is unknown
			//If ascending, put first on bottom as it is unknown
			if (f1 == (size_t)-1)
				iResult = 1;
			//If descending, put second on top as it is unknown
			//If ascending, put second on bottom as it is unknown
			if (f2 == (size_t)-1)
				iResult=-1;
			//If descending, put first on top as it is bigger.
			//If ascending, put first on bottom as it is bigger.
			else
				iResult = CompareUnsigned_PTR(f1, f2);
			break;
		}

		case 90: //Remaining SIZE asc
			iResult = CompareUnsigned64(file1->leftsize, file2->leftsize);
			break;
		case 10: //last seen complete asc
			if (file1->lastseencomplete > file2->lastseencomplete)
				iResult = 1;
			else if(file1->lastseencomplete < file2->lastseencomplete)
				iResult = -1;
			else
				return 0;
			break;
		case 11: //last received Time asc
			if (file1->GetFileDate() > file2->GetFileDate())
				iResult=1;
			else if(file1->GetFileDate() < file2->GetFileDate())
				iResult=-1;
			else
				return 0;
			break;
		case 12:
			//TODO: 'GetCategory' SHOULD be a 'const' function and 'GetResString' should NOT be called..
			iResult = CompareLocaleStringNoCase(	thePrefs.GetCategory(const_cast<CPartFile*>(file1)->GetCategory())->strTitle,
											thePrefs.GetCategory(const_cast<CPartFile*>(file2)->GetCategory())->strTitle );
			break;

		case 13: // addeed on asc
			if (file1->GetCrCFileDate() > file2->GetCrCFileDate())
				iResult = 1;
			else if(file1->GetCrCFileDate() < file2->GetCrCFileDate())
				iResult = -1;
			else
				return 0;
               case 14: //health
			iResult = CompareUnsigned64(file1->GetAvailablePartCount()*100/file1->GetPartCount(), file2->GetAvailablePartCount()*100/file1->GetPartCount());
			break;
	}
	if (lParamSort>=100)
		return -iResult;
	return iResult;
}

void CDownloadListCtrl::OnListModified(NMHDR *pNMHDR, LRESULT * /*pResult*/)
{
	NMLISTVIEW *pNMListView = (NMLISTVIEW *)pNMHDR;

	//this works because true is equal to 1 and false equal to 0
	/*BOOL notLast = pNMListView->iItem + 1 != GetItemCount();
	BOOL notFirst = pNMListView->iItem != 0;
	RedrawItems(pNMListView->iItem - notFirst, pNMListView->iItem + notLast)*/;
	//if((pNMListView->uChanged & LVIF_STATE) && (pNMListView->uNewState & LVIS_SELECTED)){
	int iSel = pNMListView->iItem;//GetNextItem(-1, LVIS_FOCUSED);
	if (iSel != -1) {
		CPartFile* partfile = (CPartFile*)GetItemData(iSel);
		ASSERT(partfile);
		if (partfile)
			theApp.emuledlg->transferwnd->downloadclientsctrl.Reload(partfile,true);
	}
	else
		theApp.emuledlg->transferwnd->downloadclientsctrl.Reload(NULL,true);
	//}
}

void CDownloadListCtrl::OnNmDblClk(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1)
	{
		const CPartFile* partfile = (CPartFile*)GetItemData(iSel);
		if (partfile)
			ShowFileDialog(0);
	}
	
	*pResult = 0;
}

void CDownloadListCtrl::CreateMenues()
{
	if (m_DropMenu) 
          VERIFY( m_DropMenu.DestroyMenu() ); //Xman Xtreme Downloadmanager

	if (m_PrioMenu)
		VERIFY( m_PrioMenu.DestroyMenu() );
	if (m_SourcesMenu)
		VERIFY( m_SourcesMenu.DestroyMenu() );
	if (m_FileMenu)
		VERIFY( m_FileMenu.DestroyMenu() );

	m_FileMenu.CreatePopupMenu();

	//Xman Xtreme Downloadmanager
	m_DropMenu.CreateMenu();
	m_DropMenu.AppendMenu(MF_STRING, MP_DROPNONEEDEDSRCS, GetResString(IDS_DROPNONEEDEDSRCS)); 
	m_DropMenu.AppendMenu(MF_STRING, MP_DROPQUEUEFULLSRCS, GetResString(IDS_DROPQUEUEFULLSRCS)); 


	m_FileMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_DropMenu.m_hMenu, GetResString(IDS_SubMenu_Drop));
	m_FileMenu.AppendMenu(MF_SEPARATOR);

	//Xman end

	// Add 'Download Priority' sub menu
	//
	m_PrioMenu.CreateMenu();
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIOLOW, GetResString(IDS_PRIOLOW));
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIONORMAL, GetResString(IDS_PRIONORMAL));
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIOHIGH, GetResString(IDS_PRIOHIGH));
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIOAUTO, GetResString(IDS_PRIOAUTO));
	m_FileMenu.AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)m_PrioMenu.m_hMenu, GetResString(IDS_PRIORITY) + _T(" (") + GetResString(IDS_DOWNLOAD) + _T(')'));

	// Add file commands
	//
	m_FileMenu.AppendMenu(MF_STRING, MP_PAUSE, GetResString(IDS_DL_PAUSE));
	m_FileMenu.AppendMenu(MF_STRING, MP_STOP, GetResString(IDS_DL_STOP));
	m_FileMenu.AppendMenu(MF_STRING, MP_RESUME, GetResString(IDS_DL_RESUME));
	m_FileMenu.AppendMenu(MF_STRING, MP_CANCEL, GetResString(IDS_MAIN_BTN_CANCEL));
	m_FileMenu.AppendMenu(MF_SEPARATOR);
	//m_FileMenu.AppendMenu(MF_STRING, MP_OPEN, GetResString(IDS_DL_OPEN));

	// Extended: Submenu with Preview options, Normal: Preview and possibly 'Preview with' item 
	if (thePrefs.IsExtControlsEnabled())
	{
		m_FileMenu.AppendMenu(MF_STRING, MP_PREVIEW, GetResString(IDS_DL_PREVIEW));
		m_FileMenu.AppendMenu(MF_STRING, MP_PAUSEONPREVIEW, GetResString(IDS_PAUSEONPREVIEW));
		if (!thePrefs.GetPreviewPrio())
    		m_FileMenu.AppendMenu(MF_STRING, MP_TRY_TO_GET_PREVIEW_PARTS, GetResString(IDS_DL_TRY_TO_GET_PREVIEW_PARTS));
	}
	else
	m_FileMenu.AppendMenu(MF_STRING, MP_PREVIEW, GetResString(IDS_DL_PREVIEW));
    m_FileMenu.AppendMenu(MF_SEPARATOR);
	m_FileMenu.AppendMenu(MF_STRING, MP_METINFO, GetResString(IDS_DL_INFO));


	// Add (extended user mode) 'Source Handling' sub menu
	//
	if (thePrefs.IsExtControlsEnabled()) {
		m_FileMenu.AppendMenu(MF_SEPARATOR);
		m_SourcesMenu.CreateMenu();
		//Xman Xtreme Downloadmanager
		m_SourcesMenu.AppendMenu(MF_STRING, MP_ALL_A4AF_AUTO, GetResString(IDS_ALL_A4AF_AUTO)); //Xman Xtreme Downloadmanager: Auto-A4AF-check
		m_SourcesMenu.AppendMenu(MF_STRING, MP_ALL_A4AF_TO_THIS, GetResString(IDS_ALL_A4AF_TO_THIS)); 
		m_SourcesMenu.AppendMenu(MF_STRING, MP_ALL_A4AF_TO_OTHER, GetResString(IDS_ALL_A4AF_TO_OTHER)); 
		//Xman end
		m_SourcesMenu.AppendMenu(MF_STRING, MP_ADDSOURCE, GetResString(IDS_ADDSRCMANUALLY));
		m_SourcesMenu.AppendMenu(MF_STRING, MP_SETSOURCELIMIT, GetResString(IDS_SETPFSLIMIT));
		m_SourcesMenu.AppendMenu(MF_STRING, MP_C0SC, GetResString(IDS_CLEAR0KCLIENT));// X: [C0SC] - [Clear0SpeedClient]
		m_FileMenu.AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)m_SourcesMenu.m_hMenu, GetResString(IDS_A4AF));
	}
	m_FileMenu.AppendMenu(MF_SEPARATOR);

	// Add 'Copy & Paste' commands
	//
	if (thePrefs.GetShowCopyEd2kLinkCmd())
		m_FileMenu.AppendMenu(MF_STRING, MP_GETED2KLINK, GetResString(IDS_DL_LINK1));
	else
		m_FileMenu.AppendMenu(MF_STRING, MP_SHOWED2KLINK, GetResString(IDS_DL_SHOWED2KLINK));
	m_FileMenu.AppendMenu(MF_STRING, MP_PASTE, GetResString(IDS_SW_DIRECTDOWNLOAD));
	//m_FileMenu.AppendMenu(MF_SEPARATOR);

	// Search commands
	//
	//m_FileMenu.AppendMenu(MF_STRING, MP_SEARCHRELATED, GetResString(IDS_SEARCHRELATED));
	if( thePrefs.IsExtControlsEnabled()){
		m_FileMenu.AppendMenu(MF_STRING, MP_FLUSHBUFFER, GetResString(IDS_TRY2FLUSH)); // X: [FB] - [FlushBuffer]
		m_FileMenu.AppendMenu(MF_STRING, MP_PREALOCATE, GetResString(IDS_PREALLDISCSPACE)); 
	}

	// Web-services and categories will be added on-the-fly..
}

void CDownloadListCtrl::ShowSelectedFileDetails()
{
	POINT point;
	::GetCursorPos(&point);
	CPoint pt = point; 
    ScreenToClient(&pt); 
    int it = HitTest(pt);
    if (it == -1)
		return;

	SetItemState(-1, 0, LVIS_SELECTED);
	SetItemState(it, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	SetSelectionMark(it);   // display selection mark correctly! 

	ShowFileDialog(0);
}

void CDownloadListCtrl::ReloadFileList(){
	ChangeCategory(curTab);
}

void CDownloadListCtrl::UpdateCurrentCategoryView(CPartFile* thisfile) {
	if (theApp.downloadqueue->filelist.Find(thisfile) != NULL){
		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)thisfile;
		int result = FindItem(&find);
		if (!thisfile->CheckShowItemInGivenCat(curTab) || IsFilteredItem(thisfile)){
			if (result != -1)
				DeleteItem(result);
		}
		else if (result == -1)
			InsertItem(LVIF_PARAM|LVIF_TEXT,GetItemCount(),LPSTR_TEXTCALLBACK,0,0,0,(LPARAM)thisfile);
	}
}

void CDownloadListCtrl::ChangeCategory(size_t newsel){

	SetRedraw(FALSE);

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	for (POSITION pos = theApp.downloadqueue->filelist.GetHeadPosition();pos != 0;){
		CPartFile* file = theApp.downloadqueue->filelist.GetNext(pos);
		// Check if entry is already in the List
		find.lParam = (LPARAM)file;
		int result = FindItem(&find);
		if (!file->CheckShowItemInGivenCat(newsel) || IsFilteredItem(file)){// X: [FI] - [FilterItem]
			if (result != -1)
				DeleteItem(result);
		}
		else if (result == -1)
			InsertItem(LVIF_PARAM|LVIF_TEXT,GetItemCount(),LPSTR_TEXTCALLBACK,0,0,0,(LPARAM)file);
	}

	SetRedraw(TRUE);
	thePrefs.lastTranWndCatID = curTab = newsel; // X: [RCI] - [Remember Catalog ID]
	theApp.emuledlg->transferwnd->UpdateFilterLabel();
	theApp.emuledlg->transferwnd->UpdateFilesCount();
}

void CDownloadListCtrl::OnLvnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (CemuleDlg::IsRunning()) {
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
		NMLVDISPINFO *pDispInfo = (NMLVDISPINFO *)pNMHDR;
		/*TRACE("CDownloadListCtrl::OnLvnGetDispInfo iItem=%d iSubItem=%d", pDispInfo->item.iItem, pDispInfo->item.iSubItem);
		if (pDispInfo->item.mask & LVIF_TEXT)
			TRACE(" LVIF_TEXT");
		if (pDispInfo->item.mask & LVIF_IMAGE)
			TRACE(" LVIF_IMAGE");
		if (pDispInfo->item.mask & LVIF_STATE)
			TRACE(" LVIF_STATE");
		TRACE("\n");*/
		if (pDispInfo->item.mask & LVIF_TEXT) {
			const CPartFile* file = reinterpret_cast<CPartFile*>(pDispInfo->item.lParam);
			if (file != NULL)
				GetItemDisplayText(file, pDispInfo->item.iSubItem, pDispInfo->item.pszText, pDispInfo->item.cchTextMax);
			else
				ASSERT(0);
		}
    }
    *pResult = 0;
}

void CDownloadListCtrl::OnLvnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult)
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

		const CPartFile* partfile = (CPartFile*)GetItemData(pGetInfoTip->iItem);
		if (partfile && pGetInfoTip->pszText && pGetInfoTip->cchTextMax > 0)
		{
			// build info text and display it
			CString info = partfile->GetInfoSummary();

			info.AppendChar(TOOLTIP_AUTOFORMAT_SUFFIX_CH);
			_tcsncpy(pGetInfoTip->pszText, info, pGetInfoTip->cchTextMax);
			pGetInfoTip->pszText[pGetInfoTip->cchTextMax-1] = _T('\0');
		}
	}
	*pResult = 0;
}

void CDownloadListCtrl::ShowFileDialog(UINT uInvokePage)
{
	CSimpleArray<CPartFile*> aFiles;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos != NULL)
	{
		int iItem = GetNextSelectedItem(pos);
		if (iItem != -1)
			aFiles.Add((CPartFile*)GetItemData(iItem));
	}

	if (aFiles.GetSize() > 0)
	{
		CFileDetailDialog dialog(&aFiles, uInvokePage, this);
		dialog.DoModal();
	}
}

void CDownloadListCtrl::RefreshFile(CPartFile* file)
{
	if( !CemuleDlg::IsRunning() )
		return;
	if( theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd || !/*theApp.emuledlg->transferwnd->downloadlistctrl.*/IsWindowVisible())
		return; 
	
	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)file;
	int result = FindItem(&find);
	if(result != -1)
		Update(result);
	return;
}

//Xman end

static const int FilterDStrID[]={
	 IDS_ALL
	,IDS_ALLOTHERS
	,0
	,-1
	,IDS_WAITING
	,IDS_DOWNLOADING
	,IDS_ERRORLIKE
	,IDS_PAUSED
	,IDS_SEENCOMPL
	,-1
	,IDS_VIDEO
	,IDS_AUDIO
	,IDS_SEARCH_ARC
	,IDS_SEARCH_CDIMG
	,IDS_SEARCH_DOC
	,IDS_SEARCH_PICS
	,IDS_SEARCH_PRG
	,0
	,0
	,0
	,IDS_SEARCH_EMULECOLLECTION
};

CString CDownloadListCtrl::GetFilterLabelCat(){
	CString newlabel;
	if (thePrefs.GetCatFilterNeg(curTab))
		newlabel.AppendChar(_T('!'));

	UINT catfilter =(UINT) thePrefs.GetCatFilter(curTab);
	if(catfilter<21 && FilterDStrID[catfilter]>0)
		newlabel.Append(GetResString(FilterDStrID[catfilter]));
	else if (catfilter==18)
		newlabel.Append( _T('\"') + thePrefs.GetCategory(curTab)->regexp + _T('\"') );
	return newlabel;
}

void CDownloadListCtrl::CreateFilterMenuCat(CMenu&CatMenu)
{
	CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0,GetResString(IDS_ALL) );
	CatMenu.AppendMenu((curTab && !thePrefs.GetCategory(curTab)->care4all ) ? MF_GRAYED : MF_STRING ,MP_CAT_SET0+1,GetResString(IDS_ALLOTHERS) );

	// selector for regular expression view filter
	if (curTab) {
		if (thePrefs.IsExtControlsEnabled()){
			CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+18, GetResString(IDS_REGEXPRESSION) );
			CatMenu.AppendMenu((thePrefs.GetCategory(curTab)->care4all)?MF_STRING:MF_STRING|MF_CHECKED | MF_BYCOMMAND ,MP_CAT_SET0+17,GetResString(IDS_CARE4ALL) );
		}
	}

	for(INT_PTR i=3;i<sizeof(FilterDStrID)/sizeof(int);++i){
		if(FilterDStrID[i]>0)
			CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+i,GetResString(FilterDStrID[i]) );
		else if(FilterDStrID[i]==-1)
			CatMenu.AppendMenu(MF_SEPARATOR);
	}

	if (thePrefs.IsExtControlsEnabled()) {
		CatMenu.AppendMenu(MF_SEPARATOR);
		CatMenu.AppendMenu( thePrefs.GetCatFilter(curTab)>0?MF_STRING:MF_GRAYED,MP_CAT_SET0+19,GetResString(IDS_NEGATEFILTER) );
		if ( thePrefs.GetCatFilterNeg(curTab))
			CatMenu.CheckMenuItem( MP_CAT_SET0+19 ,MF_CHECKED | MF_BYCOMMAND);
	}
	
	CatMenu.CheckMenuItem( MP_CAT_SET0+thePrefs.GetCatFilter(curTab) ,MF_CHECKED | MF_BYCOMMAND);
}

CImageList *CDownloadListCtrl::CreateDragImage(int /*iItem*/, LPPOINT lpPoint)
{
	const size_t iMaxSelectedItems = 30;
	size_t iSelectedItems = 0;
	CRect rcSelectedItems;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos && iSelectedItems < iMaxSelectedItems)
	{
		int iItem = GetNextSelectedItem(pos);
		CRect rcLabel;
		GetItemRect(iItem, rcLabel, LVIR_LABEL);
		if (iSelectedItems == 0)
		{
			rcSelectedItems.left = sm_iSubItemInset;
			rcSelectedItems.top = rcLabel.top;
			rcSelectedItems.right = rcLabel.right;
			rcSelectedItems.bottom = rcLabel.bottom;
		}
		rcSelectedItems.UnionRect(rcSelectedItems, rcLabel);
		iSelectedItems++;
	}
	if (iSelectedItems == 0)
		return NULL;

	CClientDC dc(this);
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC(&dc))
		return NULL;

	CBitmap bmpMem;
	if (!bmpMem.CreateCompatibleBitmap(&dc, rcSelectedItems.Width(), rcSelectedItems.Height()))
		return NULL;

	CBitmap *pOldBmp = dcMem.SelectObject(&bmpMem);
	CFont *pOldFont = dcMem.SelectObject(GetFont());

	COLORREF crBackground = GetSysColor(COLOR_WINDOW);
	dcMem.FillSolidRect(0, 0, rcSelectedItems.Width(), rcSelectedItems.Height(), crBackground);
	dcMem.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));

	iSelectedItems = 0;
	pos = GetFirstSelectedItemPosition();
	while (pos && iSelectedItems < iMaxSelectedItems)
	{
		int iItem = GetNextSelectedItem(pos);
		const CPartFile* pPartFile = (CPartFile*)GetItemData(iItem);
		CRect rcLabel;
		GetItemRect(iItem, rcLabel, LVIR_LABEL);

		CRect rcItem;
		rcItem.left = 0;
		rcItem.top = rcLabel.top - rcSelectedItems.top;
		rcItem.right = rcLabel.right;
		rcItem.bottom = rcItem.top + rcLabel.Height();

		if (theApp.GetSystemImageList())
		{
			int iImage = theApp.GetFileTypeSystemImageIdx(pPartFile->GetFileName());
			ImageList_Draw(theApp.GetSystemImageList(), iImage, dcMem, rcItem.left, rcItem.top, ILD_TRANSPARENT);
		}

		rcItem.left += 16 + sm_iLabelOffset;
		dcMem.DrawText(pPartFile->GetFileName(), -1, rcItem, MLC_DT_TEXT);
		rcItem.left -= 16 + sm_iLabelOffset;

		iSelectedItems++;
	}
	dcMem.SelectObject(pOldBmp);
	dcMem.SelectObject(pOldFont);

	// At this point the bitmap in 'bmpMem' may or may not contain alpha data and we have to take special
	// care about passing such a bitmap further into Windows (GDI). Strange things can happen due to that
	// not all GDI functions can deal with RGBA bitmaps. Thus, create an image list with ILC_COLORDDB.
	CImageList *pimlDrag = new CImageList();
	pimlDrag->Create(rcSelectedItems.Width(), rcSelectedItems.Height(), ILC_COLORDDB | ILC_MASK, 1, 0);
	pimlDrag->Add(&bmpMem, crBackground);
	bmpMem.DeleteObject();

	if (lpPoint)
	{
		CPoint ptCursor;
		GetCursorPos(&ptCursor);
		ScreenToClient(&ptCursor);
		lpPoint->x = ptCursor.x - rcSelectedItems.left;
		lpPoint->y = ptCursor.y - rcSelectedItems.top;
	}

	return pimlDrag;
}
