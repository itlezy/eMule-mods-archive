//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
//
// HistoryListCtrl. emulEspaña Mod: Added by MoNKi
//	modified and adapted by Xman-Xtreme Mod
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

// HistoryListCtrl.cpp: archivo de implementación
//

#include "stdafx.h"
#include "emule.h"
#include "KnownFile.h"
#include "Preferences.h"
#include "emuledlg.h"
#include "MemDC.h"
#include "knownfilelist.h"
#include "SharedFileList.h"
#include "menucmds.h"
#include "OtherFunctions.h"
#include "HistoryListCtrl.h"
#include "MetaDataDlg.h"
#include "ResizableLib/ResizableSheet.h"
#include "ED2kLinkDlg.h"
#include "Log.h"
#include "ListViewWalkerPropertySheet.h"
#include "UserMsgs.h"
#include "HighColorTab.hpp"
#include "PartFile.h"
#include "TransferWnd.h"
#include "DropDownButton.h"
#include "SearchDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////////
// CHistoryFileDetailsSheet

class CHistoryFileDetailsSheet : public CListViewWalkerPropertySheet
{
	DECLARE_DYNAMIC(CHistoryFileDetailsSheet)

public:
	CHistoryFileDetailsSheet(CAtlList<CKnownFile*>& aFiles, UINT uPshInvokePage = 0, CListCtrlItemWalk* pListCtrl = NULL);
	virtual ~CHistoryFileDetailsSheet();

protected:
	CMetaDataDlg		m_wndMetaData;
	CED2kLinkDlg		m_wndFileLink;

	UINT m_uPshInvokePage;
	static LPCTSTR m_pPshStartPage;

	void UpdateTitle();

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg LRESULT OnDataChanged(WPARAM, LPARAM);
};

LPCTSTR CHistoryFileDetailsSheet::m_pPshStartPage;

IMPLEMENT_DYNAMIC(CHistoryFileDetailsSheet, CListViewWalkerPropertySheet)

BEGIN_MESSAGE_MAP(CHistoryFileDetailsSheet, CListViewWalkerPropertySheet)
	ON_WM_DESTROY()
	ON_MESSAGE(UM_DATA_CHANGED, OnDataChanged)
END_MESSAGE_MAP()

CHistoryFileDetailsSheet::CHistoryFileDetailsSheet(CAtlList<CKnownFile*>& aFiles, UINT uPshInvokePage, CListCtrlItemWalk* pListCtrl)
	: CListViewWalkerPropertySheet(pListCtrl)
{
	m_uPshInvokePage = uPshInvokePage;
	POSITION pos = aFiles.GetHeadPosition();
	while (pos)
		m_aItems.Add(aFiles.GetNext(pos));
	m_psh.dwFlags &= ~PSH_HASHELP;
	
	m_wndMetaData.m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndMetaData.m_psp.dwFlags |= PSP_USEICONID;
	m_wndMetaData.m_psp.pszIcon = _T("METADATA");
	if (m_aItems.GetSize() == 1 && thePrefs.IsExtControlsEnabled()) {
		m_wndMetaData.SetFiles(&m_aItems);
		AddPage(&m_wndMetaData);
	}

	m_wndFileLink.m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndFileLink.m_psp.dwFlags |= PSP_USEICONID;
	m_wndFileLink.m_psp.pszIcon = _T("ED2KLINK");
	m_wndFileLink.SetFiles(&m_aItems);
	AddPage(&m_wndFileLink);

	LPCTSTR pPshStartPage = m_pPshStartPage;
	if (m_uPshInvokePage != 0)
		pPshStartPage = MAKEINTRESOURCE(m_uPshInvokePage);
	for (int i = 0; i < m_pages.GetSize(); i++)
	{
		CPropertyPage* pPage = GetPage(i);
		if (pPage->m_psp.pszTemplate == pPshStartPage)
		{
			m_psh.nStartPage = i;
			break;
		}
	}
}

CHistoryFileDetailsSheet::~CHistoryFileDetailsSheet()
{
}

void CHistoryFileDetailsSheet::OnDestroy()
{
	if (m_uPshInvokePage == 0)
		m_pPshStartPage = GetPage(GetActiveIndex())->m_psp.pszTemplate;
	CListViewWalkerPropertySheet::OnDestroy();
}

BOOL CHistoryFileDetailsSheet::OnInitDialog()
{		
	EnableStackedTabs(FALSE);
	BOOL bResult = CListViewWalkerPropertySheet::OnInitDialog();
	HighColorTab::UpdateImageList(*this);
	InitWindowStyles(this);
	EnableSaveRestore(_T("HistoryFileDetailsSheet"), !thePrefs.prefReadonly); // call this after(!) OnInitDialog // X: [ROP] - [ReadOnlyPreference]
	UpdateTitle();
	return bResult;
}

LRESULT CHistoryFileDetailsSheet::OnDataChanged(WPARAM, LPARAM)
{
	UpdateTitle();
	return 1;
}

void CHistoryFileDetailsSheet::UpdateTitle()
{
	CString text = GetResString(IDS_DETAILS);
	if(m_aItems.GetSize() == 1)
		//text.AppendFormat(_T(": %s"), STATIC_DOWNCAST(CKnownFile, m_aItems[0])->GetFileName());
		text.AppendFormat(_T(": %s"), ((CKnownFile*)m_aItems[0])->GetFileName());
	SetWindowText(text);
}

BOOL CHistoryFileDetailsSheet::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_APPLY_NOW)
	{
		CHistoryListCtrl* pHistoryListCtrl = DYNAMIC_DOWNCAST(CHistoryListCtrl, m_pListCtrl->GetListCtrl());
		if (pHistoryListCtrl)
			{
			for (int i = 0; i < m_aItems.GetSize(); i++) {
				// so, and why does this not(!) work while the sheet is open ??
				//pHistoryListCtrl->UpdateFile(DYNAMIC_DOWNCAST(CKnownFile, m_aItems[i]));
				pHistoryListCtrl->UpdateFile((CKnownFile*)m_aItems[i]);
			}
		}
	}

	return CListViewWalkerPropertySheet::OnCommand(wParam, lParam);
}

//////////////////////////////
// CHistoryListCtrl
static const UINT colStrID[]={
	 IDS_DL_FILENAME
	,IDS_DL_SIZE
	,IDS_TYPE
	,IDS_FILEID
	,IDS_DATE
	,IDS_DOWNHISTORY_SHARED
    ,IDS_SF_TRANSFERRED
	,IDS_SF_REQUESTS
	,IDS_SF_ACCEPTS
};

IMPLEMENT_DYNAMIC(CHistoryListCtrl, CListCtrl)
CHistoryListCtrl::CHistoryListCtrl()
	: CListCtrlItemWalk(this)
{
}

CHistoryListCtrl::~CHistoryListCtrl()
{
	if (m_HistoryMenu)  VERIFY( m_HistoryMenu.DestroyMenu() );
	m_ctlListHeader.Detach();
}


BEGIN_MESSAGE_MAP(CHistoryListCtrl, CMuleListCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnClick)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnLvnGetDispInfo)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNmDblClk)
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

void CHistoryListCtrl::SetGridLine()
{
 if (thePrefs.UseGridlines())
	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_GRIDLINES); 
   else
     SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
}

void CHistoryListCtrl::Init(void)
{
	SetPrefsKey(_T("HistoryListCtrl"));
	CImageList ilDummyImageList; //dummy list for getting the proper height of listview entries
	ilDummyImageList.Create(16, 16,theApp.m_iDfltImageListColorFlags|ILC_MASK, 1, 1); 
	CImageList *pOldStates = SetImageList(&ilDummyImageList, LVSIL_SMALL);
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	ilDummyImageList.Detach();
	if (pOldStates)
		pOldStates->DeleteImageList();
    SetGridLine();
	ASSERT( (GetStyle() & LVS_SINGLESEL) == 0 );

	InsertColumn(0,GetResString(IDS_DL_FILENAME),	LVCFMT_LEFT,		DFLT_FILENAME_COL_WIDTH);
	InsertColumn(1,GetResString(IDS_DL_SIZE),		LVCFMT_RIGHT,		DFLT_SIZE_COL_WIDTH);
	InsertColumn(2,GetResString(IDS_TYPE),			LVCFMT_LEFT,		DFLT_FILETYPE_COL_WIDTH);
	InsertColumn(3,GetResString(IDS_FILEID),		LVCFMT_LEFT,		DFLT_HASH_COL_WIDTH,		-1, true);
	InsertColumn(4,GetResString(IDS_DATE),			LVCFMT_LEFT,		120);
	InsertColumn(5,GetResString(IDS_DOWNHISTORY_SHARED),LVCFMT_LEFT,	65);
//EastShare START - Added by Pretender
	InsertColumn(6,GetResString(IDS_SF_TRANSFERRED),  LVCFMT_RIGHT,120);
	InsertColumn(7,GetResString(IDS_SF_REQUESTS),     LVCFMT_RIGHT,80);
	InsertColumn(8,GetResString(IDS_SF_ACCEPTS),      LVCFMT_RIGHT,120);
//EastShare END

	LoadSettings();
	CreateMenues();// X: [RUL] - [Remove Useless Localize]

	SetSortArrow();
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0:20));
	IgnoredColums = 0x4;	//3: type
	m_ctlListHeader.Attach(GetHeaderCtrl()->Detach());
}

void CHistoryListCtrl::AddFile(CKnownFile* toadd){
	if (!CemuleDlg::IsRunning())
		return;
	// check filter conditions if we should show this file right now
	if ((filter>0 && (filter-1) != GetED2KFileTypeID(toadd->GetFileName())) || IsFilteredItem(toadd))
		return;

	if (FindFile(toadd) != -1)
		return;
	if(!theApp.sharedfiles->IsFilePtrInList(toadd) || (/*theApp.sharedfiles->IsFilePtrInList(toadd) && */thePrefs.m_bHistoryShowShared))
		InsertItem(LVIF_PARAM|LVIF_TEXT,GetItemCount(),toadd->GetFileName(),0,0,0,(LPARAM)toadd);
}

void CHistoryListCtrl::ReloadFileList(void)
{
	if (thePrefs.IsHistoryListDisabled())
		return;
	SetRedraw(false);

	DeleteAllItems();

	//Xman 4.8
	//don't know exactly what happend, but a few users (with old known.met) had a crash
#ifdef REPLACE_ATLMAP
	for (CKnownFilesMap::const_iterator it = theApp.knownfiles->GetKnownFiles().begin(); it != theApp.knownfiles->GetKnownFiles().end(); ++it)
	{
		CKnownFile* cur_file = it->second;
		if(thePrefs.m_bHistoryShowShared||!theApp.sharedfiles->IsFilePtrInList(cur_file))
			AddFile(cur_file);
	}
#else
	CKnownFile * cur_file;
	if(theApp.knownfiles->GetKnownFiles().IsEmpty()==false)
	{
			POSITION pos = theApp.knownfiles->GetKnownFiles().GetStartPosition();					
			while(pos){
				CCKey key;
				theApp.knownfiles->GetKnownFiles().GetNextAssoc( pos, key, cur_file );
			if(thePrefs.m_bHistoryShowShared||!theApp.sharedfiles->IsFilePtrInList(cur_file))
				AddFile(cur_file);
			}		
		}
	//Xman end
#endif

	SetRedraw(true);

	theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2History, GetItemCount());
}

void CHistoryListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	uint32 iItem = lpDrawItemStruct->itemID;
	CRect cur_rec(lpDrawItemStruct->rcItem);
	CKnownFile* file = (CKnownFile*)lpDrawItemStruct->itemData;
	CMemoryDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem, m_crWindow);	
	InitItemMemDC(dc, lpDrawItemStruct->rcItem, (iItem % 2)?m_crEvenLine:m_crWindow, lpDrawItemStruct->itemState);

	RECT rcClient;// X: [DDHC] - [Don't Draw Hidden Column]
	GetClientRect(&rcClient);

	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	cur_rec.right = cur_rec.left - sm_iSubItemInset;
	cur_rec.left += sm_iSubItemInset;
	for(int iCurrent = 0; iCurrent < iCount; iCurrent++) 
	{
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
		if(IsColumnHidden(iColumn)) continue;

		UINT uDrawTextAlignment;
		int iColumnWidth = GetColumnWidth(iColumn, uDrawTextAlignment);

		cur_rec.right += iColumnWidth;
		if(iColumnWidth > 2*sm_iSubItemInset && cur_rec.right>0){// X: [DDHC] - [Don't Draw Hidden Column]
			TCHAR szItem[1024];
			GetItemDisplayText(file, iColumn, szItem, _countof(szItem));
			if(iColumn == 0){
				//draw the item's icon
				int iImage = theApp.GetFileTypeSystemImageIdx( szItem/*file->GetFileName()*/ );
				if (theApp.GetSystemImageList() != NULL){
					int iIconPosY = (cur_rec.Height() > 19) ? ((cur_rec.Height() - 16) / 2) : 1;
					::ImageList_Draw(theApp.GetSystemImageList(), iImage, dc, cur_rec.left, cur_rec.top + iIconPosY, ILD_NORMAL|ILD_TRANSPARENT);
                }

	            cur_rec.left += 16 + sm_iLabelOffset;
				dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT);
				cur_rec.left -= 16 + sm_iLabelOffset;
			   }
                else if (szItem[0] != 0)
				dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
		}
		cur_rec.left += iColumnWidth;
		if(cur_rec.left>=rcClient.right)// X: [DDHC] - [Don't Draw Hidden Column]
			break;
	}
}

void CHistoryListCtrl::GetItemDisplayText(const CAbstractFile* file, int iSubItem, LPTSTR pszText, int cchTextMax) const
{
	if (pszText == NULL || cchTextMax <= 0) {
		ASSERT(0);
		return;
	}
	pszText[0] = _T('\0');
	const CKnownFile* pKnownFile=reinterpret_cast<const CKnownFile*>(file);
	switch(iSubItem){
		case 0:
			_tcsncpy(pszText, pKnownFile->GetFileName(), cchTextMax);
			break;
		case 1:
			_tcsncpy(pszText, CastItoXBytes(pKnownFile->GetFileSize()), cchTextMax);
			break;
		case 2:
			_tcsncpy(pszText, pKnownFile->GetFileTypeDisplayStr(), cchTextMax);
			break;
		case 3:
			md4str(pKnownFile->GetFileHash(), pszText);
			break;
		case 4:
			_tcsncpy(pszText, ((pKnownFile->GetUtcFileDate() != 0) ? pKnownFile->GetUtcCFileDate().Format(thePrefs.GetDateTimeFormat4Lists()) : GetResString(IDS_NEVER)), cchTextMax);
			break;
		case 5:
			_tcsncpy(pszText, GetResString(theApp.sharedfiles->IsFilePtrInList(pKnownFile)?IDS_YES:IDS_NO), cchTextMax);
			break;
//EastShare START - Added by Pretender
	case 6:
		_tcsncpy(pszText, CastItoXBytes(pKnownFile->statistic.GetAllTimeTransferred()), cchTextMax);
		break;
	case 7:
		_sntprintf(pszText, cchTextMax, _T("%u"), pKnownFile->statistic.GetAllTimeRequests());
		break;
	case 8:
		_sntprintf(pszText, cchTextMax, _T("%u"), pKnownFile->statistic.GetAllTimeAccepts());
		break;
//EastShare
	}
	pszText[cchTextMax - 1] = _T('\0');
}

void CHistoryListCtrl::OnLvnColumnClick( NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLISTVIEW *pNMListView = (NMLISTVIEW *)pNMHDR;

	// Barry - Store sort order in preferences
	// Determine ascending based on whether already sorted on this column
	bool sortAscending = (GetSortItem()!= pNMListView->iSubItem) ? true : !GetSortAscending();

	// Sort table
	UpdateSortHistory(pNMListView->iSubItem + (sortAscending ? 0 : 20),20);
	SetSortArrow(pNMListView->iSubItem, sortAscending);
	SortItems(SortProc, pNMListView->iSubItem + (sortAscending ? 0 : 20));

	*pResult = 0;
}

int CHistoryListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort){
	/*const*/ CKnownFile* item1 = (CKnownFile*)lParam1;
	/*const*/ CKnownFile* item2 = (CKnownFile*)lParam2;	

	int iResult=0;
	bool bSortAscending = lParamSort < 20;
	LPARAM iColumn = bSortAscending ? lParamSort : lParamSort - 20;

	switch(iColumn){
		case 0: //filename asc
			iResult = CompareLocaleStringNoCase(item1->GetFileName(),item2->GetFileName());
			break;
		case 1: //filesize asc
			iResult = CompareUnsigned64(item1->GetFileSize(), item2->GetFileSize());
			break;
		case 2: //filetype asc
			iResult = item1->GetFileType().CompareNoCase(item2->GetFileType());
			// if the type is equal, subsort by extension
			if (iResult == 0)
			{
				LPCTSTR pszExt1 = PathFindExtension(item1->GetFileName());
				LPCTSTR pszExt2 = PathFindExtension(item2->GetFileName());
				if ((pszExt1 == NULL) ^ (pszExt2 == NULL)) // moloko+
					iResult = pszExt1 == NULL ? 1 : (-1);
				else
					iResult = pszExt1 != NULL ? _tcsicmp(pszExt1, pszExt2) : 0;
			}
			break;
		case 3: //file ID
			iResult = memcmp(item1->GetFileHash(),item2->GetFileHash(),16);
			break;
		case 4: //date
			iResult = CompareUnsigned64(item1->GetUtcFileDate(),item2->GetUtcFileDate());
			break;
//EastShare START - Added by Pretender
			case 6: //all transferred asc
				iResult=item1->statistic.GetAllTimeTransferred()==item2->statistic.GetAllTimeTransferred()?0:(item1->statistic.GetAllTimeTransferred()>item2->statistic.GetAllTimeTransferred()?1:-1);
				break;
         
			case 7: //acc requests asc
				iResult=item1->statistic.GetAllTimeRequests() - item2->statistic.GetAllTimeRequests();
				break;
			
			case 8: //acc accepts asc
				iResult=item1->statistic.GetAllTimeAccepts() - item2->statistic.GetAllTimeAccepts();
				break;
//EastShare END
		case 5: //Shared?
		{
			bool shared1, shared2;
			shared1 = theApp.sharedfiles->IsFilePtrInList(item1);
			shared2 = theApp.sharedfiles->IsFilePtrInList(item2);
			if(shared1==shared2)
				return 0;
			iResult = (shared1 && !shared2 ? 1 : -1);
			break;
		}
			break;
	}

	if (!bSortAscending)
		return -iResult;
	// SLUGFILLER: multiSort remove - handled in parent class
	/*
	int dwNextSort;
	//call secondary sortorder, if this one results in equal
	//(Note: yes I know this call is evil OO wise, but better than changing a lot more code, while we have only one instance anyway - might be fixed later)
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->sharedfileswnd->historylistctrl.GetNextSortOrder(lParamSort)) != (-1)){
		iResult= SortProc(lParam1, lParam2, dwNextSort);
	}
	*/
	return iResult;
}

void CHistoryListCtrl::Localize() {
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;
	for (int icol=0;icol<pHeaderCtrl->GetItemCount();++icol) {
/*		switch(i){
			case 0:
				strRes = GetResString(IDS_DL_FILENAME);
				break;
			case 1:
				strRes = GetResString(IDS_DL_SIZE);
				break;
			case 2:
				strRes = GetResString(IDS_TYPE);
				break;
			case 3:
				strRes = GetResString(IDS_FILEID);
				break;
			case 4:
				strRes = GetResString(IDS_DATE);
				break;
			case 5:
				strRes = GetResString(IDS_DOWNHISTORY_SHARED);
				break;
			case 6:
				strRes = GetResString(IDS_COMMENT);
				break;

			//EastShare
			case 7:
				strRes = GetResString(IDS_SF_TRANSFERRED);
				break;
			case 8:
				strRes = GetResString(IDS_SF_REQUESTS);
				break;
			
			//EastShare

			default:
				strRes = "No Text!!";
				break;
		}*/
		strRes=GetResString(colStrID[icol]);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(icol, &hdi);
	}

	CreateMenues();
}

void CHistoryListCtrl::CreateMenues()
{
	if (m_HistoryMenu) VERIFY(m_HistoryMenu.DestroyMenu());

	m_HistoryMenu.CreatePopupMenu();
	m_HistoryMenu.AppendMenu(MF_STRING,MP_OPEN, GetResString(IDS_OPENFILE));
	
	if(thePrefs.IsExtControlsEnabled())
	m_HistoryMenu.AppendMenu(MF_STRING,MP_DETAIL, GetResString(IDS_SHOWDETAILS));
	m_HistoryMenu.AppendMenu(MF_STRING,MP_SHOWED2KLINK, GetResString(IDS_DL_SHOWED2KLINK));
	//m_HistoryMenu.AppendMenu(MF_STRING, MP_SEARCHRELATED, GetResString(IDS_SEARCHRELATED));
	m_HistoryMenu.AppendMenu(MF_STRING|MF_SEPARATOR); 

	if(thePrefs.IsExtControlsEnabled()){
	m_HistoryMenu.AppendMenu(MF_STRING,MP_VIEWSHAREDFILES,GetResString(IDS_DOWNHISTORY_SHOWSHARED));
	m_HistoryMenu.AppendMenu(MF_STRING|MF_SEPARATOR); 
	}
	
	m_HistoryMenu.AppendMenu(MF_STRING,MP_REMOVESELECTED, GetResString(IDS_DOWNHISTORY_REMOVE));
	m_HistoryMenu.AppendMenu(MF_STRING,MP_CLEARHISTORY,GetResString(IDS_DOWNHISTORY_CLEAR));
}

void CHistoryListCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
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
	CKnownFile* file = NULL;

	size_t iSelectedItems = GetSelectedCount();
	if (GetSelectionMark()!=-1) file=(CKnownFile*)GetItemData(GetSelectionMark());

	if(file && theApp.sharedfiles->IsFilePtrInList(file)){
		m_HistoryMenu.EnableMenuItem(MP_OPEN, MF_ENABLED);
		m_HistoryMenu.EnableMenuItem(MP_REMOVESELECTED, MF_GRAYED);
	}
	else {
		m_HistoryMenu.EnableMenuItem(MP_OPEN, MF_GRAYED);
		m_HistoryMenu.EnableMenuItem(MP_REMOVESELECTED, (file && iSelectedItems>0)?MF_ENABLED:MF_GRAYED);
    }

	m_HistoryMenu.EnableMenuItem(MP_SHOWED2KLINK, (iSelectedItems>0)?MF_ENABLED:MF_GRAYED);
	//m_HistoryMenu.EnableMenuItem(MP_SEARCHRELATED, theApp.emuledlg->searchwnd->CanSearchRelatedFiles() ? MF_ENABLED : MF_GRAYED);

	if(thePrefs.IsExtControlsEnabled()){
	m_HistoryMenu.EnableMenuItem(MP_DETAIL, (iSelectedItems>0)?MF_ENABLED:MF_GRAYED);
	m_HistoryMenu.CheckMenuItem(MP_VIEWSHAREDFILES, thePrefs.m_bHistoryShowShared?MF_CHECKED:MF_UNCHECKED);
	}

	GetPopupMenuPos(*this, point);
	m_HistoryMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON,point.x,point.y,this);
}

void CHistoryListCtrl::OnNmDblClk(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1)
	{
		CKnownFile* file = (CKnownFile*)GetItemData(iSel);
		if (file)
		{
			if ((GetKeyState(VK_MENU) & 0x8000) || file->GetFilePath().IsEmpty())
			{
				CAtlList<CKnownFile*> aFiles;
				aFiles.AddHead(file);
				ShowFileDialog(aFiles);
			}
			else if (!file->IsPartFile())
				ShellOpenFile(file->GetFilePath(), NULL);
		}
	}
	*pResult = 0;
}

BOOL CHistoryListCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	size_t selectedCount = GetSelectedCount(); 
	int iSel = GetSelectionMark();

	CAtlList<CKnownFile*> selectedList;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos != NULL){
		int index = GetNextSelectedItem(pos);
		if (index >= 0)
			selectedList.AddTail((CKnownFile*)GetItemData(index));
	}

	if (selectedCount>0){
		CKnownFile* file = (CKnownFile*)GetItemData(iSel);

		switch (wParam){
			case MP_SHOWED2KLINK:
			{
				ShowFileDialog(selectedList, IDD_ED2KLINK);
				break;
			}
			case MP_OPEN:
				if(theApp.sharedfiles->IsFilePtrInList(file))
					ShellOpenFile(file->GetFilePath(), NULL);
				break; 
			case MPG_ALTENTER:
			case MP_DETAIL:{
				ShowFileDialog(selectedList);
				break;
			}
		    //zz_fly :: from Morph, handled Ctrl+C and Detele :: start
			case MP_COPYSELECTED:{
				CString str;
				while (!selectedList.IsEmpty()){
					if (!str.IsEmpty())
						str += _T("\r\n");
					str += selectedList.RemoveHead()->GetED2kLink();
				}
				theApp.CopyTextToClipboard(str);
				break;
			}
			case MPG_DELETE:
			//zz_fly :: end
			case MP_REMOVESELECTED:
				{
					if (selectedCount > 1)
					{
						if(MessageBox(GetResString(IDS_DOWNHISTORY_REMOVE_QUESTION_MULTIPLE),NULL,MB_YESNO) == IDYES){
							int  nItem = -1;
							for (size_t i = 0;i < selectedCount;i++)
							{
								nItem = GetNextItem(nItem, LVNI_SELECTED);
								ASSERT(nItem != -1);
								CKnownFile *item_File = (CKnownFile *)GetItemData(nItem);
								if(item_File && theApp.sharedfiles->GetFileByID(item_File->GetFileHash())==NULL ){
									RemoveFile(item_File);
									nItem--;
								}
							}
						}
					}
					else
					{
						if(file && theApp.sharedfiles->GetFileByID(file->GetFileHash())==NULL){ //Xman 4.2 crashfix
							CString msg;
							msg.Format(GetResString(IDS_DOWNHISTORY_REMOVE_QUESTION),file->GetFileName());
							if(MessageBox(msg,NULL,MB_YESNO) == IDYES)
								RemoveFile(file);
						}
					}
				}
				break;
			/*case MP_SEARCHRELATED:{
				CAtlList<CAbstractFile*> abstractFileList;
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
					abstractFileList.AddTail(selectedList.GetNext(pos));
				theApp.emuledlg->searchwnd->SearchRelatedFiles(abstractFileList);
				theApp.emuledlg->SetActiveDialog(theApp.emuledlg->searchwnd);
				break;
			}*/
		}
	}

	switch(wParam){
		case MP_CLEARHISTORY:
			ClearHistory();
			break;
		case MP_VIEWSHAREDFILES:
			thePrefs.m_bHistoryShowShared = !thePrefs.m_bHistoryShowShared;
			ReloadFileList();
			break;
	}

	return true;
}

void CHistoryListCtrl::ShowSelectedFileInfo() {
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

	CKnownFile* file = (CKnownFile*)GetItemData(GetSelectionMark());
	if (file)
	{
		CAtlList<CKnownFile*> aFiles;
		aFiles.AddHead(file);
		ShowFileDialog(aFiles);
	}
}

void CHistoryListCtrl::RemoveFile(CKnownFile *toRemove) {
	if(theApp.sharedfiles->IsFilePtrInList(toRemove))
		return;

	if(theApp.knownfiles->RemoveKnownFile(toRemove)){
		int nItem = FindFile(toRemove);
		if(nItem != -1)
		{
			DeleteItem(nItem);
			theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2History, GetItemCount());
		}
	}
}
//Xman
//only used for removing duplicated files to avoid a crash
void CHistoryListCtrl::RemoveFileFromView(CKnownFile* toRemove)
{
	int nItem = FindFile(toRemove);
	if(nItem != -1)
	{
		DeleteItem(nItem);
	}
	//remark: no need to update the count-info, because we only replace files
}
//Xman end

void CHistoryListCtrl::ClearHistory() {
	if(MessageBox(GetResString(IDS_DOWNHISTORY_CLEAR_QUESTION),GetResString(IDS_DOWNHISTORY),MB_YESNO)==IDYES)
	{
		theApp.knownfiles->ClearHistory();
		ReloadFileList();
	}
}

void CHistoryListCtrl::UpdateFile(const CKnownFile* file)
{
	if (!file || !CemuleDlg::IsRunning())
		return;

	//MORPH START - SiRoB, Don't Refresh item if not needed
	if(theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd  || !IsWindowVisible())
		return;
	//MORPH END   - SiRoB, Don't Refresh item if not needed

	int iItem = FindFile(file);
	if (iItem != -1)
		Update(iItem);
	}

int CHistoryListCtrl::FindFile(const CKnownFile* pFile)
{
	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)pFile;
	return FindItem(&find);
}

void CHistoryListCtrl::ShowFileDialog(CAtlList<CKnownFile*>& aFiles, UINT uPshInvokePage)
{
	if (aFiles.GetCount() > 0)
	{
		CHistoryFileDetailsSheet dialog(aFiles, uPshInvokePage, this);
		dialog.DoModal();
	}
}

void CHistoryListCtrl::OnLvnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
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
		NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
		if (pDispInfo->item.mask & LVIF_TEXT) {
			const CShareableFile* pFile = reinterpret_cast<CShareableFile*>(pDispInfo->item.lParam);
			if (pFile != NULL)
				GetItemDisplayText(pFile, pDispInfo->item.iSubItem, pDispInfo->item.pszText, pDispInfo->item.cchTextMax);
		}
	}
	*pResult = 0;
}
