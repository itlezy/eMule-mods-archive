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
#include "Preferences.h"
#include "emuledlg.h"
#include "MemDC.h"
#include "knownfilelist.h"
#include "SharedFileList.h"
#include "menucmds.h"
#include "OtherFunctions.h"
#include "IrcWnd.h"
#include "WebServices.h"
#include "HistoryListCtrl.h"
#include "CommentDialog.h"
#include "FileInfoDialog.h"
#include "MetaDataDlg.h"
#include "ResizableLib/ResizableSheet.h"
#include "ED2kLinkDlg.h"
#include "Log.h"
#include "ListViewWalkerPropertySheet.h"
#include "UserMsgs.h"
#include "SharedFilesWnd.h"
#include "HighColorTab.hpp"
#include "PartFile.h"
#include "TransferDlg.h"
#include "AbstractFile.h"

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
	CHistoryFileDetailsSheet(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage = 0, CListCtrlItemWalk* pListCtrl = NULL);
	virtual ~CHistoryFileDetailsSheet();

protected:
	CFileInfoDialog		m_wndMediaInfo;
	CMetaDataDlg		m_wndMetaData;
	CED2kLinkDlg		m_wndFileLink;
	CCommentDialog		m_wndFileComments;

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

CHistoryFileDetailsSheet::CHistoryFileDetailsSheet(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage, CListCtrlItemWalk* pListCtrl)
	: CListViewWalkerPropertySheet(pListCtrl)
{
	m_uPshInvokePage = uPshInvokePage;
	POSITION pos = aFiles.GetHeadPosition();
	while (pos)
		m_aItems.Add(aFiles.GetNext(pos));
	m_psh.dwFlags &= ~PSH_HASHELP;
	
	m_wndMediaInfo.m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndMediaInfo.m_psp.dwFlags |= PSP_USEICONID;
	m_wndMediaInfo.m_psp.pszIcon = _T("MEDIAINFO");
	m_wndMediaInfo.SetFiles(&m_aItems);
	AddPage(&m_wndMediaInfo);

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

	m_wndFileComments.m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndFileComments.m_psp.dwFlags |= PSP_USEICONID;
	m_wndFileComments.m_psp.pszIcon = _T("FileComments");
	m_wndFileComments.SetFiles(&m_aItems);
	AddPage(&m_wndFileComments);

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
	EnableSaveRestore(_T("HistoryFileDetailsSheet")); // call this after(!) OnInitDialog
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
	if (m_aItems.GetSize() == 1)
		SetWindowText(GetResString(IDS_DETAILS) + _T(": ") + STATIC_DOWNCAST(CKnownFile, m_aItems[0])->GetFileName());
	else
		SetWindowText(GetResString(IDS_DETAILS));
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
				pHistoryListCtrl->UpdateFile(DYNAMIC_DOWNCAST(CKnownFile, m_aItems[i]));
			}
		}
	}

	return CListViewWalkerPropertySheet::OnCommand(wParam, lParam);
}

//////////////////////////////
// CHistoryListCtrl

IMPLEMENT_DYNAMIC(CHistoryListCtrl, CListCtrl)
CHistoryListCtrl::CHistoryListCtrl()
	: CListCtrlItemWalk(this)
{
	SetGeneralPurposeFind(true);
	SetSkinKey(L"HistoryListCtrl");
}

CHistoryListCtrl::~CHistoryListCtrl()
{
	if (m_HistoryOpsMenu) VERIFY( m_HistoryOpsMenu.DestroyMenu() );
	if (m_HistoryMenu)  VERIFY( m_HistoryMenu.DestroyMenu() );
}


BEGIN_MESSAGE_MAP(CHistoryListCtrl, CMuleListCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnClick)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnLvnGetDispInfo)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()


void CHistoryListCtrl::Init(void)
{
	SetPrefsKey(_T("HistoryListCtrl"));
	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
	ModifyStyle(LVS_SINGLESEL,0);
	
	InsertColumn(0,GetResString(IDS_DL_FILENAME),LVCFMT_LEFT, 260);
	InsertColumn(1,GetResString(IDS_DL_SIZE),LVCFMT_RIGHT,70);
	InsertColumn(2,GetResString(IDS_TYPE),LVCFMT_LEFT,100);
	InsertColumn(3,GetResString(IDS_FILEID),LVCFMT_LEFT, 220);
	InsertColumn(4,GetResString(IDS_DATE),LVCFMT_LEFT, 120);
	InsertColumn(5,GetResString(IDS_DOWNHISTORY_SHARED),LVCFMT_LEFT, 65);
	InsertColumn(6,GetResString(IDS_COMMENT),LVCFMT_LEFT, 260);
	//EastShare START - Added by Pretender
	InsertColumn(7,GetResString(IDS_SF_TRANSFERRED),LVCFMT_RIGHT,120);
	InsertColumn(8,GetResString(IDS_SF_REQUESTS),LVCFMT_RIGHT,100);
	InsertColumn(9,GetResString(IDS_SF_ACCEPTS),LVCFMT_RIGHT,100);
	//EastShare END

	LoadSettings();

	Reload();

	SetSortArrow();
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0:20));
}

void CHistoryListCtrl::AddFile(CKnownFile* toadd){
	uint32 itemnr = GetItemCount();

	LVFINDINFO info;
	info.flags = LVFI_PARAM;
	info.lParam = (LPARAM)toadd;
	int nItem = FindItem(&info);
	if(nItem == -1){
		if(!theApp.sharedfiles->IsFilePtrInList(toadd) || (theApp.sharedfiles->IsFilePtrInList(toadd) && thePrefs.GetShowSharedInHistory()))
		{
			InsertItem(LVIF_PARAM|LVIF_TEXT,itemnr,toadd->GetFileName(),0,0,0,(LPARAM)toadd);
			if(IsWindowVisible())
			{
				CString str;
				str.Format(_T(" (%i)"),this->GetItemCount());
				theApp.emuledlg->sharedfileswnd->GetDlgItem(IDC_TRAFFIC_TEXT)->SetWindowText(GetResString(IDS_DOWNHISTORY) + str);
			}
		}
	}
}

void CHistoryListCtrl::Reload(void)
{
	CKnownFile * cur_file;

	SetRedraw(false);

	DeleteAllItems();

	theApp.emuledlg->sharedfileswnd->ShowSelectedFilesDetails(false, true);

	//Xman 4.8
	//don't know exactly what happend, but a few users (with old known.met) had a crash
	if(theApp.knownfiles->GetKnownFiles().IsEmpty()==false)
	{
		if(thePrefs.GetShowSharedInHistory()){
			POSITION pos = theApp.knownfiles->GetKnownFiles().GetStartPosition();					
			while(pos){
				CCKey key;
				theApp.knownfiles->GetKnownFiles().GetNextAssoc( pos, key, cur_file );
				InsertItem(LVIF_PARAM|LVIF_TEXT,GetItemCount(),cur_file->GetFileName(),0,0,0,(LPARAM)cur_file);
			}		
		}
		else{
			CKnownFilesMap *files = NULL;
			files=theApp.knownfiles->GetDownloadedFiles();
			POSITION pos = files->GetStartPosition();					
			while(pos){
				CCKey key;
				files->GetNextAssoc( pos, key, cur_file );
				InsertItem(LVIF_PARAM|LVIF_TEXT,GetItemCount(),cur_file->GetFileName(),0,0,0,(LPARAM)cur_file);
			}		
			delete files;
		}
	}
	//Xman end
      
	SetRedraw(true);

	if(IsWindowVisible())
	{
		CString str;
		str.Format(_T(" (%i)"),this->GetItemCount());
		theApp.emuledlg->sharedfileswnd->GetDlgItem(IDC_TRAFFIC_TEXT)->SetWindowText(GetResString(IDS_DOWNHISTORY) + str);
	}
}

void CHistoryListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	if (!lpDrawItemStruct->itemData)
		return;

	// ==> Visual Studio 2010 Compatibility [Stulle/Avi-3k/ied] - Stulle
	/*
	CMemDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	*/
	CMemoryDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	// <== Visual Studio 2010 Compatibility [Stulle/Avi-3k/ied] - Stulle
	BOOL bCtrlFocused;
	InitItemMemDC(dc, lpDrawItemStruct, bCtrlFocused);
	CRect cur_rec(lpDrawItemStruct->rcItem);
	CRect rcClient;
	GetClientRect(&rcClient);

	//Fafner: possible exception in history - 070626
	//Fafner: note: I got this when replacing known.met (e.g., with backup) and some of
	//Fafner: note: the files are still shared. After return and reload it is fine.
	CString sText;
	try {
		CKnownFile* file = (CKnownFile*)lpDrawItemStruct->itemData;

		CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
		int iCount = pHeaderCtrl->GetItemCount();
		cur_rec.right = cur_rec.left - sm_iLabelOffset;
		cur_rec.left += sm_iIconOffset;
		int iIconDrawWidth = theApp.GetSmallSytemIconSize().cx;
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
					GetItemDisplayText(file, iColumn, szItem, _countof(szItem));
					switch (iColumn)
					{
						case 0:
						{
							int iIconPosY = (cur_rec.Height() > theApp.GetSmallSytemIconSize().cy) ? ((cur_rec.Height() - theApp.GetSmallSytemIconSize().cy) / 2) : 0;
							int iImage = theApp.GetFileTypeSystemImageIdx(file->GetFileName());
							if (theApp.GetSystemImageList() != NULL)
								::ImageList_Draw(theApp.GetSystemImageList(), iImage, dc.GetSafeHdc(), cur_rec.left, cur_rec.top + iIconPosY, ILD_TRANSPARENT);
							cur_rec.left += iIconDrawWidth;

							cur_rec.left += sm_iLabelOffset;
							dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
							cur_rec.left -= iIconDrawWidth;
							cur_rec.right -= sm_iSubItemInset;
							break;
						}

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
	catch (...) {
		if (!theApp.knownfiles->bReloadHistory)
			LogError(LOG_STATUSBAR, _T("CHistoryListCtrl::DrawItem: exception - %s."), sText); //just once
		theApp.knownfiles->bReloadHistory = true;
	}
}

void CHistoryListCtrl::GetItemDisplayText(CKnownFile* file, int iSubItem, LPTSTR pszText, int cchTextMax) const
{
	if (pszText == NULL || cchTextMax <= 0) {
		ASSERT(0);
		return;
	}
	pszText[0] = _T('\0');
	switch (iSubItem)
	{
	case 0:
		_tcsncpy(pszText, file->GetFileName(), cchTextMax);
		break;
	case 1:
		_tcsncpy(pszText, CastItoXBytes(file->GetFileSize()), cchTextMax);
		break;
	case 2:
		_tcsncpy(pszText, file->GetFileTypeDisplayStr(), cchTextMax);
		break;
	case 3:
		_tcsncpy(pszText, EncodeBase16(file->GetFileHash(),16), cchTextMax);
		break;
	case 4:
		_tcsncpy(pszText, file->GetUtcCFileDate().Format("%x %X"), cchTextMax);
		break;
	case 5:
		if (theApp.sharedfiles->IsFilePtrInList(file))
			_tcsncpy(pszText, GetResString(IDS_YES), cchTextMax);
		else
			_tcsncpy(pszText, GetResString(IDS_NO), cchTextMax);
		break;
	case 6:
		_tcsncpy(pszText, file->GetFileComment(), cchTextMax);
		break;
	//EastShare START - Added by Pretender
	case 7:
		_tcsncpy(pszText, CastItoXBytes(file->statistic.GetAllTimeTransferred()), cchTextMax);
		break;
	case 8:
		_sntprintf(pszText, cchTextMax, _T("%u"), file->statistic.GetAllTimeRequests());
		break;
	case 9:
		_sntprintf(pszText, cchTextMax, _T("%u"), file->statistic.GetAllTimeAccepts());
		break;
	//EastShare END
	}
	pszText[cchTextMax - 1] = _T('\0');
}

void CHistoryListCtrl::OnLvnColumnClick( NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLISTVIEW *pNMListView = (NMLISTVIEW *)pNMHDR;
	bool sortAscending;
	if (GetSortItem() != pNMListView->iSubItem)
	{
		switch (pNMListView->iSubItem)
		{
			case 7:
			case 8:
			case 9:
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
	int iColumn = bSortAscending ? lParamSort : lParamSort - 20;

	try { //Fafner: possible exception in history - 070626
		switch(iColumn){
			case 0: //filename ascf
				iResult = CompareLocaleStringNoCase(item1->GetFileName(), item2->GetFileName());
				break;

			case 1: //filesize asc
				iResult = CompareUnsigned64(item1->GetFileSize(), item2->GetFileSize());
				break;

			case 2: //filetype asc
			iResult= item1->GetFileType().CompareNoCase(item2->GetFileType());
			// if the type is equal, subsort by extension
			if (iResult == 0)
			{
				LPCTSTR pszExt1 = PathFindExtension(item1->GetFileName());
				LPCTSTR pszExt2 = PathFindExtension(item2->GetFileName());
				if ((pszExt1 == NULL) ^ (pszExt2 == NULL))
					iResult = pszExt1 == NULL ? 1 : (-1);
				else
					iResult = pszExt1 != NULL ? _tcsicmp(pszExt1, pszExt2) : 0;
			}
			break;

			case 3: //file ID
				iResult= memcmp(item1->GetFileHash(),item2->GetFileHash(),16);
				break;

			case 4: //date
				iResult= CompareUnsigned(item1->GetUtcFileDate(),item2->GetUtcFileDate());
				break;

			case 5: //Shared?
				{
					bool shared1, shared2;
					shared1 = theApp.sharedfiles->IsFilePtrInList(item1);
					shared2 = theApp.sharedfiles->IsFilePtrInList(item2);
					iResult= shared1==shared2 ? 0 : (shared1 && !shared2 ? 1 : -1);
				}
				break;

			case 6: //comment
				iResult= _tcsicmp(item1->GetFileComment(),item2->GetFileComment());
				break;

			//EastShare START - Added by Pretender
			case 7: //all transferred asc
				iResult=item1->statistic.GetAllTimeTransferred()==item2->statistic.GetAllTimeTransferred()?0:(item1->statistic.GetAllTimeTransferred()>item2->statistic.GetAllTimeTransferred()?1:-1);
				break;

			case 8: //acc requests asc
				iResult=item1->statistic.GetAllTimeRequests() - item2->statistic.GetAllTimeRequests();
				break;
		
			case 9: //acc accepts asc
				iResult=item1->statistic.GetAllTimeAccepts() - item2->statistic.GetAllTimeAccepts();
				break;
			//EastShare END
		}
	}
	catch (...) {
		if (!theApp.knownfiles->bReloadHistory)
			LogError(LOG_STATUSBAR, _T("CHistoryListCtrl::SortProc: exception.")); //just once
		theApp.knownfiles->bReloadHistory = true;
		iResult = 0;
	}

	if (!bSortAscending)
		iResult = -iResult;

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

	for(int i=0; i<=6;i++){
		switch(i){
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
			case 9:
				strRes = GetResString(IDS_SF_ACCEPTS);
				break;
			//EastShare
			default:
				strRes = "No Text!!";
		}

		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(i, &hdi);
	}

	CreateMenues();
}

void CHistoryListCtrl::CreateMenues()
{
	if (m_HistoryOpsMenu) VERIFY( m_HistoryOpsMenu.DestroyMenu() );
	if (m_HistoryMenu) VERIFY(m_HistoryMenu.DestroyMenu());

	m_HistoryOpsMenu.CreateMenu();
	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	m_HistoryOpsMenu.AddMenuTitle(NULL, true);
	*/
	m_HistoryOpsMenu.AddMenuTitle(GetResString(IDS_HISTORY), true, false);
	// <== XP Style Menu [Xanatos] - Stulle
	m_HistoryOpsMenu.AppendMenu(MF_STRING,MP_CLEARHISTORY,GetResString(IDS_DOWNHISTORY_CLEAR), _T("CLEARCOMPLETE"));

	m_HistoryMenu.CreatePopupMenu();
	m_HistoryMenu.AddMenuTitle(GetResString(IDS_DOWNHISTORY), true);
	m_HistoryMenu.AppendMenu(MF_STRING,MP_OPEN, GetResString(IDS_OPENFILE), _T("OPENFILE"));
	
	m_HistoryMenu.AppendMenu(MF_STRING,MP_CMT, GetResString(IDS_CMT_ADD), _T("FILECOMMENTS")); 
	m_HistoryMenu.AppendMenu(MF_STRING,MP_DETAIL, GetResString(IDS_SHOWDETAILS), _T("FILEINFO"));
	m_HistoryMenu.AppendMenu(MF_STRING|MF_SEPARATOR); 

	m_HistoryMenu.AppendMenu(MF_STRING,MP_VIEWSHAREDFILES,GetResString(IDS_DOWNHISTORY_SHOWSHARED));
	m_HistoryMenu.AppendMenu(MF_STRING|MF_SEPARATOR); 

	m_HistoryMenu.AppendMenu(MF_STRING,MP_SHOWED2KLINK, GetResString(IDS_DL_SHOWED2KLINK), _T("ED2KLINK"));
	m_HistoryMenu.AppendMenu(MF_STRING|MF_SEPARATOR); 
	
	m_HistoryMenu.AppendMenu(MF_STRING,MP_REMOVESELECTED, GetResString(IDS_DOWNHISTORY_REMOVE), _T("DELETESELECTED"));
	m_HistoryMenu.AppendMenu(MF_STRING|MF_SEPARATOR); 

	m_HistoryMenu.AppendMenu(MF_STRING,Irc_SetSendLink,GetResString(IDS_IRC_ADDLINKTOIRC), _T("IRCCLIPBOARD"));
	m_HistoryMenu.AppendMenu(MF_STRING|MF_SEPARATOR); 

	m_HistoryMenu.AppendMenu(MF_POPUP,(UINT_PTR)m_HistoryOpsMenu.m_hMenu, GetResString(IDS_DOWNHISTORY_ACTIONS));
	m_HistoryMenu.AppendMenu(MF_STRING|MF_SEPARATOR); 
}

void CHistoryListCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CKnownFile* file = NULL;

	int iSelectedItems = GetSelectedCount();
	if (GetSelectionMark()!=-1) file=(CKnownFile*)GetItemData(GetSelectionMark());

	if(file && theApp.sharedfiles->IsFilePtrInList(file)){
		m_HistoryMenu.EnableMenuItem(MP_OPEN, MF_ENABLED);
		m_HistoryMenu.EnableMenuItem(MP_REMOVESELECTED, MF_GRAYED);
	}
	else {
		m_HistoryMenu.EnableMenuItem(MP_OPEN, MF_GRAYED);
		if(file && GetSelectedCount()>0)
            m_HistoryMenu.EnableMenuItem(MP_REMOVESELECTED, MF_ENABLED);
        else
		    m_HistoryMenu.EnableMenuItem(MP_REMOVESELECTED, MF_GRAYED);
    }

	if(thePrefs.GetShowSharedInHistory())
		m_HistoryMenu.CheckMenuItem(MP_VIEWSHAREDFILES, MF_CHECKED);
	else
		m_HistoryMenu.CheckMenuItem(MP_VIEWSHAREDFILES, MF_UNCHECKED);

	m_HistoryMenu.EnableMenuItem(Irc_SetSendLink, iSelectedItems == 1 && theApp.emuledlg->ircwnd->IsConnected() ? MF_ENABLED : MF_GRAYED);

	CTitleMenu WebMenu;
	WebMenu.CreateMenu();
	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	WebMenu.AddMenuTitle(NULL, true);
	*/
	WebMenu.AddMenuTitle(GetResString(IDS_WEBSERVICES), true, false);
	// <== XP Style Menu [Xanatos] - Stulle
	int iWebMenuEntries = theWebServices.GetFileMenuEntries(&WebMenu);
	UINT flag = (iWebMenuEntries == 0 || iSelectedItems != 1) ? MF_GRAYED : MF_ENABLED;
	m_HistoryMenu.AppendMenu(MF_POPUP | flag, (UINT_PTR)WebMenu.m_hMenu, GetResString(IDS_WEBSERVICES), _T("SEARCHMETHOD_GLOBAL"));

	m_HistoryMenu.AppendMenu(MF_SEPARATOR); 
	m_HistoryMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED), MP_FIND, GetResString(IDS_FIND), _T("Search"));

	GetPopupMenuPos(*this, point);
	m_HistoryMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON,point.x,point.y,this);

	m_HistoryMenu.RemoveMenu(m_HistoryMenu.GetMenuItemCount()-1,MF_BYPOSITION); //find menu
	m_HistoryMenu.RemoveMenu(m_HistoryMenu.GetMenuItemCount()-1,MF_BYPOSITION); //separator
	m_HistoryMenu.RemoveMenu(m_HistoryMenu.GetMenuItemCount()-1,MF_BYPOSITION); //web menu
	VERIFY( WebMenu.DestroyMenu() );
}

void CHistoryListCtrl::OnNMDblclk(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1)
	{
		CKnownFile* file = (CKnownFile*)GetItemData(iSel);
		if (file)
		{
			if (GetKeyState(VK_MENU) & 0x8000)
			{
				CTypedPtrList<CPtrList, CKnownFile*> aFiles;
				aFiles.AddHead(file);
				ShowFileDialog(aFiles);
			}
			else if (!file->IsPartFile())
				OpenFile(file);
		}
	}
	*pResult = 0;
}

BOOL CHistoryListCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	UINT selectedCount = this->GetSelectedCount(); 
	int iSel = GetSelectionMark();

	switch (wParam)
	{
	case MP_FIND:
		OnFindStart();
		return TRUE;
	}

	CTypedPtrList<CPtrList, CKnownFile*> selectedList;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos != NULL){
		int index = GetNextSelectedItem(pos);
		if (index >= 0)
			selectedList.AddTail((CKnownFile*)GetItemData(index));
	}

	if (selectedCount>0){
		CKnownFile* file = (CKnownFile*)GetItemData(iSel);
		if (wParam>=MP_WEBURL && wParam<=MP_WEBURL+256) {
			theWebServices.RunURL(file, wParam);
		}

		switch (wParam){
			case Irc_SetSendLink:
			{
				theApp.emuledlg->ircwnd->SetSendFileString(file->GetED2kLink());
				break;
			}
			case MP_SHOWED2KLINK:
			{
				ShowFileDialog(selectedList, IDD_ED2KLINK);
				break;
			}
			case MP_OPEN:
				if(theApp.sharedfiles->IsFilePtrInList(file))
					OpenFile(file);
				break; 
			case MP_CMT: 
				ShowFileDialog(selectedList, IDD_COMMENT);
                break; 
			case MPG_ALTENTER:
			case MP_DETAIL:{
				ShowFileDialog(selectedList);
				break;
			}
			case MP_COPYSELECTED:{
				CString str;
				while (!selectedList.IsEmpty()){
					if (!str.IsEmpty())
						str += _T("\r\n");
					str += selectedList.GetHead()->GetED2kLink();
					selectedList.RemoveHead();
				}
				theApp.CopyTextToClipboard(str);
				break;
			}
			case MPG_DELETE:
			case MP_REMOVESELECTED:
				{
					UINT i, uSelectedCount = GetSelectedCount();
					int  nItem = -1;

					if (uSelectedCount > 1)
					{
						if(MessageBox(GetResString(IDS_DOWNHISTORY_REMOVE_QUESTION_MULTIPLE),NULL,MB_YESNO) == IDYES){
							for (i=0;i < uSelectedCount;i++)
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
		}
	}

	switch(wParam){
		case MP_CLEARHISTORY:
			ClearHistory();
			break;
		case MP_VIEWSHAREDFILES:
			thePrefs.SetShowSharedInHistory(!thePrefs.GetShowSharedInHistory());
			Reload();
			break;
	}

	return true;
}

void CHistoryListCtrl::ShowComments(CKnownFile* file) {
	if (file)
	{
		CTypedPtrList<CPtrList, CKnownFile*> aFiles;
		aFiles.AddHead(file);
		ShowFileDialog(aFiles, IDD_COMMENT);
	}
}

void CHistoryListCtrl::OpenFile(CKnownFile* file){
	TCHAR* buffer = new TCHAR[MAX_PATH];
	_sntprintf(buffer,MAX_PATH,_T("%s\\%s"),file->GetPath(),file->GetFileName());
	AddLogLine( false, _T("%s\\%s"),file->GetPath(),file->GetFileName());
	ShellOpenFile(buffer, NULL);
	delete[] buffer;
}

void CHistoryListCtrl::RemoveFile(CKnownFile *toRemove) {
	if(theApp.sharedfiles->IsFilePtrInList(toRemove))
		return;

	if (toRemove->IsKindOf(RUNTIME_CLASS(CPartFile)))
		theApp.emuledlg->transferwnd->GetDownloadList()->ClearCompleted(static_cast<CPartFile*>(toRemove));

	if(theApp.knownfiles->RemoveKnownFile(toRemove)){
		LVFINDINFO info;
		info.flags = LVFI_PARAM;
		info.lParam = (LPARAM)toRemove;
		int nItem = FindItem(&info);
		if(nItem != -1)
		{
			DeleteItem(nItem);
			if(IsWindowVisible())
			{
				CString str;
				str.Format(_T(" (%i)"),this->GetItemCount());
				theApp.emuledlg->sharedfileswnd->GetDlgItem(IDC_TRAFFIC_TEXT)->SetWindowText(GetResString(IDS_DOWNHISTORY) + str);
			}
		}
	}
}
//Xman
//only used for removing duplicated files to avoid a crash
void CHistoryListCtrl::RemoveFileFromView(CKnownFile* toRemove)
{
	LVFINDINFO info;
	info.flags = LVFI_PARAM;
	info.lParam = (LPARAM)toRemove;
	int nItem = FindItem(&info);
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
		Reload();
	}
}


void CHistoryListCtrl::UpdateFile(const CKnownFile* file)
{
	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	if (theApp.IsRunningAsService(SVC_LIST_OPT))
		return;
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle

	if (!file || !theApp.emuledlg->IsRunning())
		return;
	int iItem = FindFile(file);
	if (iItem != -1)
	{
		Update(iItem);
		if (GetItemState(iItem, LVIS_SELECTED))
			theApp.emuledlg->sharedfileswnd->ShowSelectedFilesDetails(false,true);
	}
}

int CHistoryListCtrl::FindFile(const CKnownFile* pFile)
{
	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)pFile;
	return FindItem(&find);
}

void CHistoryListCtrl::ShowFileDialog(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage)
{
	if (aFiles.GetSize() > 0)
	{
		CHistoryFileDetailsSheet dialog(aFiles, uPshInvokePage, this);
		dialog.DoModal();
	}
}

void CHistoryListCtrl::OnLvnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
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
			CKnownFile* pFile = reinterpret_cast<CKnownFile*>(pDispInfo->item.lParam);
			if (pFile != NULL)
				GetItemDisplayText(pFile, pDispInfo->item.iSubItem, pDispInfo->item.pszText, pDispInfo->item.cchTextMax);
		}
	}
	*pResult = 0;
}