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
#include "SharedFilesCtrl.h"
#include "OtherFunctions.h"
#include "MetaDataDlg.h"
#include "ED2kLinkDlg.h"
#include "HighColorTab.hpp"
#include "ListViewWalkerPropertySheet.h"
#include "UserMsgs.h"
#include "ResizableLib/ResizableSheet.h"
#include "KnownFile.h"
#include "MapKey.h"
#include "SharedFileList.h"
#include "MemDC.h"
#include "PartFile.h"
#include "MenuCmds.h"
#include "Opcodes.h"
#include "InputBox.h"
#include "TransferWnd.h"
#include "ClientList.h"
#include "UpDownClient.h"
#include "Collection.h"
#include "CollectionViewDialog.h"
#include "SearchParams.h"
#include "SearchDlg.h"
#include "SearchResultsWnd.h"
#include "ToolTipCtrlX.h"
#include "kademlia/kademlia/kademlia.h"
#include "kademlia/kademlia/UDPFirewallTester.h"
#include "Log.h"
#include "KnownFileList.h"
#include "VisualStylesXP.h"
#include "VolumeInfo.h" // X: [FSFS] - [FileSystemFeaturesSupport]
#include <list>
#include <algorithm>
// ShellContextMenu :: Start
#include "ShellContextMenu.h"
// ShellContextMenu :: End

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////////
// CSharedFileDetailsSheet

class CSharedFileDetailsSheet : public CListViewWalkerPropertySheet
{
	DECLARE_DYNAMIC(CSharedFileDetailsSheet)

public:
	CSharedFileDetailsSheet(CAtlList<CShareableFile*>& aFiles, UINT uPshInvokePage = 0, CListCtrlItemWalk* pListCtrl = NULL);
	virtual ~CSharedFileDetailsSheet();

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

LPCTSTR CSharedFileDetailsSheet::m_pPshStartPage;

IMPLEMENT_DYNAMIC(CSharedFileDetailsSheet, CListViewWalkerPropertySheet)

BEGIN_MESSAGE_MAP(CSharedFileDetailsSheet, CListViewWalkerPropertySheet)
	ON_WM_DESTROY()
	ON_MESSAGE(UM_DATA_CHANGED, OnDataChanged)
END_MESSAGE_MAP()

CSharedFileDetailsSheet::CSharedFileDetailsSheet(CAtlList<CShareableFile*>& aFiles, UINT uPshInvokePage, CListCtrlItemWalk* pListCtrl)
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

CSharedFileDetailsSheet::~CSharedFileDetailsSheet()
{
}

void CSharedFileDetailsSheet::OnDestroy()
{
	if (m_uPshInvokePage == 0)
		m_pPshStartPage = GetPage(GetActiveIndex())->m_psp.pszTemplate;
	CListViewWalkerPropertySheet::OnDestroy();
}

BOOL CSharedFileDetailsSheet::OnInitDialog()
{
	EnableStackedTabs(FALSE);
	BOOL bResult = CListViewWalkerPropertySheet::OnInitDialog();
	HighColorTab::UpdateImageList(*this);
	InitWindowStyles(this);
	EnableSaveRestore(_T("SharedFileDetailsSheet"), !thePrefs.prefReadonly); // call this after(!) OnInitDialog // X: [ROP] - [ReadOnlyPreference]
	UpdateTitle();
	return bResult;
}

LRESULT CSharedFileDetailsSheet::OnDataChanged(WPARAM, LPARAM)
{
	UpdateTitle();
	return 1;
}

void CSharedFileDetailsSheet::UpdateTitle()
{
	CString text = GetResString(IDS_DETAILS);
	if(m_aItems.GetSize() == 1)
		//text.AppendFormat(_T(": %s"), STATIC_DOWNCAST(CAbstractFile, m_aItems[0])->GetFileName());
		text.AppendFormat(_T(": %s"), ((CAbstractFile*)m_aItems[0])->GetFileName());
	SetWindowText(text);
}

BOOL CSharedFileDetailsSheet::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_APPLY_NOW)
	{
		CSharedFilesCtrl* pSharedFilesCtrl = DYNAMIC_DOWNCAST(CSharedFilesCtrl, m_pListCtrl->GetListCtrl());
		if (pSharedFilesCtrl)
		{
			for (int i = 0; i < m_aItems.GetSize(); i++) {
				// so, and why does this not(!) work while the sheet is open ??
//>>> WiZaRd::Optimization
				//pSharedFilesCtrl->UpdateFile(DYNAMIC_DOWNCAST(CKnownFile, m_aItems[i]));
				//theApp.sharedfiles->UpdateFile(DYNAMIC_DOWNCAST(CKnownFile, m_aItems[i])); 
//<<< WiZaRd::Optimization				
				theApp.sharedfiles->UpdateFile((CKnownFile*)m_aItems[i]); 
			}
		}
	}

	return CListViewWalkerPropertySheet::OnCommand(wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////////
// CSharedFilesCtrl
static const UINT colStrID[]={
	 IDS_DL_FILENAME
	,IDS_DL_SIZE
	,IDS_TYPE
	,IDS_PRIORITY
	,IDS_FILEID
	,IDS_SF_REQUESTS
	,IDS_SF_ACCEPTS
	,IDS_SF_TRANSFERRED
	,IDS_SF_UPLOADED_PARTS // ==> Spread bars [Slugfiller/MorphXT] - Stulle
	,IDS_FOLDER
	,IDS_COMPLSOURCES
	,IDS_SHAREDTITLE
	,IDS_ARTIST
	,IDS_ALBUM
	,IDS_TITLE
	,IDS_LENGTH
	,IDS_BITRATE
	,IDS_CODEC
	,IDS_ONQUEUE
};

IMPLEMENT_DYNAMIC(CSharedFilesCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CSharedFilesCtrl, CMuleListCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnClick)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnLvnGetDispInfo)
	ON_NOTIFY_REFLECT(LVN_GETINFOTIP, OnLvnGetInfoTip)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNmDblClk)
	ON_NOTIFY_REFLECT_EX(NM_CLICK, OnNMClick)
	ON_WM_CONTEXTMENU()
	ON_WM_KEYDOWN()
	//ON_WM_SYSCOLORCHANGE()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

CSharedFilesCtrl::CSharedFilesCtrl()
	: CListCtrlItemWalk(this),m_pDirectoryFilter(NULL)
// ShellContextMenu :: Start
	,m_pSCM(NULL)
// ShellContextMenu :: End
{
	memset(&m_aSortBySecondValue, 0, sizeof(m_aSortBySecondValue));
	m_pToolTip = new CToolTipCtrlX;
	m_pHighlightedItem = NULL;
}

CSharedFilesCtrl::~CSharedFilesCtrl()
{
	while (!liTempShareableFilesInDir.IsEmpty())	// delete shareble files
		delete liTempShareableFilesInDir.RemoveHead();

	// Tux: Fix: memleak fix [WiZaRd] [start]
	if (m_PrioMenu) VERIFY( m_PrioMenu.DestroyMenu() );
	if (m_CollectionsMenu) VERIFY( m_CollectionsMenu.DestroyMenu() );
	if (m_SharedFilesMenu) VERIFY( m_SharedFilesMenu.DestroyMenu() );
	// Tux: Fix: memleak fix [WiZaRd] [end]

// ShellContextMenu :: Start
	delete m_pSCM;
// ShellContextMenu :: End
	delete m_pToolTip;
	m_ctlListHeader.Detach();
}

void CSharedFilesCtrl::SetGridLine()
{
 if (thePrefs.UseGridlines())
	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_GRIDLINES); 
   else
     SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
}

void CSharedFilesCtrl::Init()
{
	SetPrefsKey(_T("SharedFilesCtrl"));
	SetGridLine();
	ASSERT( (GetStyle() & LVS_SINGLESEL) == 0 );

	InsertColumn(0, GetResString(IDS_DL_FILENAME),		LVCFMT_LEFT,  DFLT_FILENAME_COL_WIDTH);
	InsertColumn(1, GetResString(IDS_DL_SIZE),			LVCFMT_RIGHT, DFLT_SIZE_COL_WIDTH);
	InsertColumn(2, GetResString(IDS_TYPE),				LVCFMT_LEFT,  DFLT_FILETYPE_COL_WIDTH);
	InsertColumn(3, GetResString(IDS_PRIORITY),			LVCFMT_LEFT,  DFLT_PRIORITY_COL_WIDTH);
	InsertColumn(4, GetResString(IDS_FILEID),			LVCFMT_LEFT,  DFLT_HASH_COL_WIDTH,		-1, true);
	InsertColumn(5, GetResString(IDS_SF_REQUESTS),		LVCFMT_RIGHT, 100);
	InsertColumn(6, GetResString(IDS_SF_ACCEPTS),		LVCFMT_RIGHT, 100,						-1, true);
	InsertColumn(7, GetResString(IDS_SF_TRANSFERRED),	LVCFMT_RIGHT, 120);
 // ==> Spread bars [Slugfiller/MorphXT] - Stulle
	InsertColumn(8,GetResString(IDS_SF_UPLOADED_PARTS),     LVCFMT_LEFT,  100);
 // <== Spread bars [Slugfiller/MorphXT] - Stulle
	InsertColumn(9, GetResString(IDS_FOLDER),			LVCFMT_LEFT,  DFLT_FOLDER_COL_WIDTH,	-1, true);
	InsertColumn(10,GetResString(IDS_COMPLSOURCES),		LVCFMT_RIGHT, 100);
	InsertColumn(11,GetResString(IDS_SHAREDTITLE),		LVCFMT_LEFT,  100);
	InsertColumn(12,GetResString(IDS_ARTIST),			LVCFMT_LEFT,  DFLT_ARTIST_COL_WIDTH,	-1, true);
	InsertColumn(13,GetResString(IDS_ALBUM),			LVCFMT_LEFT,  DFLT_ALBUM_COL_WIDTH,		-1, true);
	InsertColumn(14,GetResString(IDS_TITLE),			LVCFMT_LEFT,  DFLT_TITLE_COL_WIDTH,		-1, true);
	InsertColumn(15,GetResString(IDS_LENGTH),			LVCFMT_RIGHT, DFLT_LENGTH_COL_WIDTH,	-1, true);
	InsertColumn(16,GetResString(IDS_BITRATE),			LVCFMT_RIGHT, DFLT_BITRATE_COL_WIDTH,	-1, true);
	InsertColumn(17,GetResString(IDS_CODEC),			LVCFMT_LEFT,  DFLT_CODEC_COL_WIDTH,		-1, true);
	InsertColumn(18,GetResString(IDS_ONQUEUE),          LVCFMT_LEFT,  80); //Xman see OnUploadqueue

	SetAllIcons();
	CreateMenues();
	LoadSettings();

	m_aSortBySecondValue[0] = true; // Requests:			Sort by 2nd value by default
	m_aSortBySecondValue[1] = true; // Accepted Requests:	Sort by 2nd value by default
	m_aSortBySecondValue[2] = true; // Transferred Data:	Sort by 2nd value by default
	m_aSortBySecondValue[3] = false; // Shared ED2K|Kad:	Sort by 1st value by default
	if (GetSortItem() >= 5 && GetSortItem() <= 7)
		m_aSortBySecondValue[GetSortItem() - 5] = GetSortSecondValue();
	else if (GetSortItem() == 11)
		m_aSortBySecondValue[3] = GetSortSecondValue();
	SetSortArrow();
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0 : 20) + (GetSortSecondValue() ? 100 : 0));

	CToolTipCtrl* tooltip = GetToolTips();
	if (tooltip){
		m_pToolTip->SubclassWindow(*tooltip);
		tooltip->ModifyStyle(0, TTS_NOPREFIX);
		tooltip->SetDelayTime(TTDT_AUTOPOP, 20000);
		tooltip->SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1000);
	}
	IgnoredColums = 0x904;	//3: type, 8: shared parts, 11:shared ed2k/kad
	m_ctlListHeader.Attach(GetHeaderCtrl()->Detach());

	m_ShareDropTarget.SetParent(this);
	VERIFY( m_ShareDropTarget.Register(this) );
}
/*
void CSharedFilesCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
	CreateMenues();
}
*/
void CSharedFilesCtrl::SetAllIcons()
{
	ApplyImageList(NULL);
	m_ImageList.DeleteImageList();
	m_ImageList.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);
	//m_ImageList.Add(CTempIconLoader(_T("EMPTY")));
	m_ImageList.Add(CTempIconLoader(_T("FileSharedServer")));
	m_ImageList.Add(CTempIconLoader(_T("FileSharedKad")));
	// Apply the image list also to the listview control, even if we use our own 'DrawItem'.
	// This is needed to give the listview control a chance to initialize the row height.
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) != 0 );
	VERIFY( ApplyImageList(m_ImageList) == NULL );
}

void CSharedFilesCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;
	for (int icol=0;icol<pHeaderCtrl->GetItemCount();++icol) {
		strRes=GetResString(colStrID[icol]);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(icol, &hdi);
	}

	CreateMenues();

	Invalidate();// X: [CI] - [Code Improvement]
	/*size_t iItems = GetItemCount();
	for (size_t i = 0; i < iItems; i++)
		Update(i);*/
}

void CSharedFilesCtrl::AddFile(const CShareableFile* file)
{
	if (!CemuleDlg::IsRunning())
		return;
	// check filter conditions if we should show this file right now
	if ((filter>0 && (filter-1) != GetED2KFileTypeID(file->GetFileName())) || IsFilteredItem(file))
		return;
	if (FindFile(file) != -1)
	{
		// in the filesystem view the shared status might have changed so we need to update the item to redraw the checkbox
		if (thePrefs.showShareableFile/*m_pDirectoryFilter != NULL && m_pDirectoryFilter->m_eItemType == SDI_UNSHAREDDIRECTORY*/)
			UpdateFile(file);
		return;
	}

	// if we are in the filesystem view, this might be a CKnownFile which has to replace a CShareableFile
	// (in case we start sharing this file), so make sure to replace the old one instead of adding a new
	if (thePrefs.showShareableFile/*m_pDirectoryFilter != NULL && m_pDirectoryFilter->m_eItemType == SDI_UNSHAREDDIRECTORY*/ && IsKindOfCKnownFile(file)/*file->IsKindOf(RUNTIME_CLASS(CKnownFile))*/)
	{
		for (POSITION pos = liTempShareableFilesInDir.GetHeadPosition(); pos != NULL; )
		{
			CShareableFile* pFile = liTempShareableFilesInDir.GetNext(pos);
			if (pFile->GetFilePath().CompareNoCase(file->GetFilePath()) == 0)
			{
				int iOldFile = FindFile(pFile);
				if (iOldFile != (-1))
				{
					SetItemData(iOldFile, (LPARAM)file);
					Update(iOldFile);
					//theApp.emuledlg->sharedfileswnd->ShowFilesCount();
					return;
				}
			}
		}
	}

	/*int iItem = */InsertItem(LVIF_TEXT | LVIF_PARAM, GetItemCount(), LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)file);
	//if (iItem >= 0)
		//Update(iItem);
}

void CSharedFilesCtrl::RemoveFile(const CShareableFile* file, bool bDeletedFromDisk)
{
	int iItem = FindFile(file);
	if (iItem != -1)
	{
		if (!bDeletedFromDisk && thePrefs.showShareableFile/*m_pDirectoryFilter != NULL && m_pDirectoryFilter->m_eItemType == SDI_UNSHAREDDIRECTORY*/)
			// in the file system view we usally dont need to remove a file, if it becomes unshared it will
			// still be visible as its still in the file system and the knownfile object doesn't gets deleted neither
			// so to avoid having to reload the whole list we just update it instead of removing and refinding
			UpdateFile(file);
		else
			DeleteItem(iItem);
		theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Shared,GetItemCount());
	}
}

void CSharedFilesCtrl::UpdateFile(const CShareableFile* file/*, bool force*/) //Xman advanced upload-priority
{
	if (!file || !CemuleDlg::IsRunning())
		return;

	int iItem = FindFile(file);
	if (iItem != -1)
		Update(iItem);
}

int CSharedFilesCtrl::FindFile(const CShareableFile* pFile)
{
	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)pFile;
	return FindItem(&find);
}

void CSharedFilesCtrl::ReloadFileList()
{
	DeleteAllItems();
	
#ifdef REPLACE_ATLMAP
	for (CKnownFilesMap::const_iterator it = theApp.sharedfiles->m_Files_map.begin(); it != theApp.sharedfiles->m_Files_map.end(); ++it)
		AddFile(it->second);
#else
	CCKey bufKey;
	CKnownFile* cur_file;
	for (POSITION pos = theApp.sharedfiles->m_Files_map.GetStartPosition(); pos != 0; ){
		theApp.sharedfiles->m_Files_map.GetNextAssoc(pos, bufKey, cur_file);
		AddFile(cur_file);
	}
#endif

	if(thePrefs.showShareableFile){
		while (!liTempShareableFilesInDir.IsEmpty())	// cleanup old filelist
			delete liTempShareableFilesInDir.RemoveHead();

		std::vector<LPCTSTR> l_sAdded;
		LPCTSTR tempDir = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);

		l_sAdded.push_back( g_VolumeInfo.GetRealPath(tempDir) ); // X: [FSFS] - [FileSystemFeaturesSupport]

		AddShareableFiles(tempDir);
		for (size_t ix=1;ix<thePrefs.GetCatCount();ix++)
		{
			tempDir = thePrefs.GetCatPath(ix);
			LPCTSTR realDir = g_VolumeInfo.GetRealPath(tempDir); // X: [FSFS] - [FileSystemFeaturesSupport]
			if(std::find(l_sAdded.begin(), l_sAdded.end(), realDir) ==  l_sAdded.end()) {
				l_sAdded.push_back(realDir);
				AddShareableFiles(tempDir);
			}
		}

		for (POSITION pos = thePrefs.shareddir_list.GetHeadPosition();pos != 0;)
		{
			tempDir = thePrefs.shareddir_list.GetNext(pos);
			LPCTSTR realDir = g_VolumeInfo.GetRealPath(tempDir); // X: [FSFS] - [FileSystemFeaturesSupport]
			if(std::find(l_sAdded.begin(), l_sAdded.end(), realDir) ==  l_sAdded.end()) {
				l_sAdded.push_back(realDir);
				AddShareableFiles(tempDir);
			}
		}
	}
	theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Shared,GetItemCount());
}

void CSharedFilesCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CRect cur_rec(lpDrawItemStruct->rcItem);
	/*const*/ CShareableFile* file = (CShareableFile*)lpDrawItemStruct->itemData;
	CKnownFile* pKnownFile = NULL;
	if (IsKindOfCKnownFile(file)/*file->IsKindOf(RUNTIME_CLASS(CKnownFile))*/)
		pKnownFile = (CKnownFile*)file;
	CMemoryDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	InitItemMemDC(dc, lpDrawItemStruct->rcItem, pKnownFile && pKnownFile->IsPartFile()? RGB(255,250,200)/*Partfile*/: pKnownFile && pKnownFile->GetUpPriority()==PR_POWER ? RGB(255,225,225)/*Power*/:((lpDrawItemStruct->itemID % 2)?m_crEvenLine:m_crWindow), lpDrawItemStruct->itemState);

	RECT rcClient;// X: [DDHC] - [Don't Draw Hidden Column]
	GetClientRect(&rcClient);

	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	cur_rec.right = cur_rec.left - sm_iSubItemInset;
	cur_rec.left += sm_iSubItemInset;
	for(int iCurrent = 0; iCurrent < iCount; iCurrent++){
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
		if(IsColumnHidden(iColumn)) continue;
		UINT uDrawTextAlignment;
		int iColumnWidth = GetColumnWidth(iColumn, uDrawTextAlignment);
		cur_rec.right += iColumnWidth;
		if(iColumnWidth > 2*sm_iSubItemInset && cur_rec.right>0){// X: [DDHC] - [Don't Draw Hidden Column]
			TCHAR szItem[1024];
			GetItemDisplayText(file, iColumn, szItem, _countof(szItem));
			switch(iColumn){
                      	case 0:{
					int iCheckboxDrawWidth = 0;
					if (thePrefs.showShareableFile)
					{
						int iState = (file == m_pHighlightedItem) ? CBS_UNCHECKEDHOT : CBS_UNCHECKEDNORMAL;
						int iNoStyleState = (file == m_pHighlightedItem) ? DFCS_PUSHED : 0;
						// no interacting with shell linked files or default shared directories
						if (theApp.sharedfiles->ShouldBeShared(file->GetSharedDirectory(), file->GetFilePath(), false)  && IsKindOfCKnownFile(file))// X: TODO
						{
							if(file->IsShellLinked()){
							iState = CBS_CHECKEDDISABLED;
							iNoStyleState = DFCS_CHECKED | DFCS_INACTIVE;
						}
							else if((theApp.sharedfiles->ShouldBeShared(file->GetSharedDirectory(), _T("")/*file->GetFilePath()*/, true)))
							{
								if(IsKindOfCKnownFile(file)/*file->IsKindOf(RUNTIME_CLASS(CKnownFile))*/){ // X: [QOH] - [QueryOnHashing]
								iState = CBS_CHECKEDDISABLED;
								iNoStyleState = DFCS_CHECKED | DFCS_INACTIVE;
							}
							}
							else{
								iState = (file == m_pHighlightedItem) ? CBS_CHECKEDHOT : CBS_CHECKEDNORMAL;
								iNoStyleState = (file == m_pHighlightedItem) ? (DFCS_PUSHED | DFCS_CHECKED) : DFCS_CHECKED;
							}
						}
						// SLUGFILLER: SafeHash remove - removed installation dir unsharing
						/*
						else if (!thePrefs.IsShareableDirectory(file->GetPath())){
							iState = CBS_DISABLED;
							iNoStyleState = DFCS_INACTIVE;
						}
						*/

						HTHEME hTheme = (g_xpStyle.IsThemeActive() && g_xpStyle.IsAppThemed()) ? g_xpStyle.OpenThemeData(NULL, L"BUTTON") : NULL;

						RECT recCheckBox;
						recCheckBox.left = cur_rec.left;
						recCheckBox.right = recCheckBox.left + 16;
						recCheckBox.top = cur_rec.top;
						if(cur_rec.Height() > 16)
							recCheckBox.top += (cur_rec.Height() - 16) / 2;
						recCheckBox.bottom = recCheckBox.top + 16;
						if (hTheme != NULL)
							g_xpStyle.DrawThemeBackground(hTheme, dc.GetSafeHdc(), BP_CHECKBOX, iState, &recCheckBox, NULL);
						else
							dc.DrawFrameControl(&recCheckBox, DFC_BUTTON, DFCS_BUTTONCHECK | iNoStyleState | DFCS_FLAT);
						iCheckboxDrawWidth = 16 + sm_i2IconOffset;
						cur_rec.left += iCheckboxDrawWidth;
					}

					int iIconDrawWidth = theApp.GetSmallSytemIconSize().cx;
					int iImage = theApp.GetFileTypeSystemImageIdx(szItem/*file->GetFileName()*/);
					if (theApp.GetSystemImageList() != NULL){
						int iIconPosY = (cur_rec.Height() > 19) ? ((cur_rec.Height() - 16) / 2) : 1;
						ImageList_Draw(theApp.GetSystemImageList(), iImage, dc.GetSafeHdc(), cur_rec.left, cur_rec.top + iIconPosY, ILD_TRANSPARENT);
					}
					cur_rec.left += iIconDrawWidth + sm_iLabelOffset;
					dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT);
					cur_rec.left -= iIconDrawWidth + iCheckboxDrawWidth + sm_iLabelOffset;
					break;
				    }
					// ==> Spread bars 
					case 8:
						if(pKnownFile == NULL)
							break;
						cur_rec.bottom--;
						cur_rec.top++;
						pKnownFile->statistic.DrawSpreadBar(dc,&cur_rec,thePrefs.UseFlatBar());
						{
                          if (thePrefs.IsExtControlsEnabled())
						    {
							COLORREF oldclr = dc.SetTextColor(RGB(0,0,0));
							int iOMode = dc.SetBkMode(TRANSPARENT);
							_sntprintf(szItem, _countof(szItem), _T("%.2f"), pKnownFile->statistic.GetSpreadSortValue()); 
							//_sntprintf(szItem, _countof(szItem), _T("%i%%"), pKnownFile->statistic.GetSpreadSortValue()*100); 
							dc.DrawText(szItem, -1, &cur_rec, (MLC_DT_TEXT & ~DT_LEFT) | DT_CENTER);
							dc.SetBkMode(iOMode);
							dc.SetTextColor(oldclr);
						        }
						}
						cur_rec.bottom++;
						cur_rec.top--;
						break;
					// <== Spread bars 
				case 11:{
					if(pKnownFile){
						POINT pt = {cur_rec.left, cur_rec.top};
						if (pKnownFile->GetPublishedED2K()){
							m_ImageList.Draw(dc, 0, pt, ILD_NORMAL);
							pt.x += 16;
						}
						if (IsSharedInKad(pKnownFile))
							m_ImageList.Draw(dc, 1, pt, ILD_NORMAL);
					}
					break;
				}
				default:
					if(szItem[0] != 0)
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
					break;
			}
		}
		cur_rec.left += iColumnWidth;
		if(cur_rec.left>=rcClient.right)// X: [DDHC] - [Don't Draw Hidden Column]
			break;
	}
}

void CSharedFilesCtrl::GetItemDisplayText(const CAbstractFile* file, int iSubItem, LPTSTR pszText, int cchTextMax) const
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
			_tcsncpy(pszText, CastItoXBytes(file->GetFileSize(), false, false), cchTextMax);
			break;
		
		case 2:
			_tcsncpy(pszText, file->GetFileTypeDisplayStr(), cchTextMax);
			break;
		
		case 9:
			_tcsncpy(pszText, reinterpret_cast<const CShareableFile*>(file)->GetPath(), cchTextMax);
			pszText[cchTextMax - 1] = _T('\0');
			PathRemoveBackslash(pszText);
			break;
		default:
		if (IsKindOfCKnownFile(file)/*file->IsKindOf(RUNTIME_CLASS(CKnownFile))*/){
			CKnownFile* pKnownFile = (CKnownFile*)file;
			switch (iSubItem)
			{			
				case 3:
					_tcsncpy(pszText, pKnownFile->GetUpPriorityDisplayString(), cchTextMax);
					break;
			
				case 4:
					md4str(pKnownFile->GetFileHash(), pszText);
					break;
			
				case 5:
					_sntprintf(pszText, cchTextMax, _T("%u (%u)"), pKnownFile->statistic.GetRequests(), pKnownFile->statistic.GetAllTimeRequests());
					break;
			
				case 6:
					_sntprintf(pszText, cchTextMax, _T("%u (%u)"), pKnownFile->statistic.GetAccepts(), pKnownFile->statistic.GetAllTimeAccepts());
					break;
			
				case 7:
					_sntprintf(pszText, cchTextMax, _T("%s (%s)"), CastItoXBytes(pKnownFile->statistic.GetTransferred(), false, false), CastItoXBytes(pKnownFile->statistic.GetAllTimeTransferred(), false, false));
					break;
// ==> Spread bars [Slugfiller/MorphXT] - Stulle
			case 8:
				_sntprintf(pszText, cchTextMax, _T("%.2f"), pKnownFile->statistic.GetSpreadSortValue()); 
				//_tcsncpy(pszText, _T("Spreadbars"), cchTextMax);
				break;
			// <== Spread bars [Slugfiller/MorphXT] - Stulle			
				case 10:
					//Xman show virtual sources (morph) + virtualUploadsources
					if(pKnownFile->IsPartFile()==false || thePrefs.UseAdvancedAutoPtio()==false)
					{
						if (pKnownFile->m_nCompleteSourcesCountLo == pKnownFile->m_nCompleteSourcesCountHi)
							_sntprintf(pszText, cchTextMax, _T("%u (%u)"), pKnownFile->m_nCompleteSourcesCountLo, pKnownFile->m_nVirtualCompleteSourcesCount);
						else if (pKnownFile->m_nCompleteSourcesCountLo == 0)
							_sntprintf(pszText, cchTextMax, _T("< %u (%u)"), pKnownFile->m_nCompleteSourcesCountHi, pKnownFile->m_nVirtualCompleteSourcesCount);
						else
							_sntprintf(pszText, cchTextMax, _T("%u - %u (%u)"), pKnownFile->m_nCompleteSourcesCountLo, pKnownFile->m_nCompleteSourcesCountHi, pKnownFile->m_nVirtualCompleteSourcesCount);
					}
					else
					{
						//Xman advanced upload-priority
						if (pKnownFile->m_nCompleteSourcesCountLo == pKnownFile->m_nCompleteSourcesCountHi)
							_sntprintf(pszText, cchTextMax, _T("%u (%u/%u)"), pKnownFile->m_nCompleteSourcesCountLo, pKnownFile->m_nVirtualCompleteSourcesCount, pKnownFile->m_nVirtualUploadSources);
						else if (pKnownFile->m_nCompleteSourcesCountLo == 0)
							_sntprintf(pszText, cchTextMax, _T("< %u (%u/%u)"), pKnownFile->m_nCompleteSourcesCountHi, pKnownFile->m_nVirtualCompleteSourcesCount, pKnownFile->m_nVirtualUploadSources);
						else
							_sntprintf(pszText, cchTextMax, _T("%u - %u (%u/%u)"), pKnownFile->m_nCompleteSourcesCountLo, pKnownFile->m_nCompleteSourcesCountHi, pKnownFile->m_nVirtualCompleteSourcesCount, pKnownFile->m_nVirtualUploadSources);
						//Xman end
					}
					//Xman end
					break;
			
				case 11:
					//_sntprintf(pszText, cchTextMax, _T("%s|%s"), GetResString(pKnownFile->GetPublishedED2K() ? IDS_YES : IDS_NO), GetResString(IsSharedInKad(pKnownFile) ? IDS_YES : IDS_NO));
					break;
			
				case 12:
					_tcsncpy(pszText, pKnownFile->GetStrTagValue(FT_MEDIA_ARTIST), cchTextMax);
					break;
			
				case 13:
					_tcsncpy(pszText, pKnownFile->GetStrTagValue(FT_MEDIA_ALBUM), cchTextMax);
					break;
			
				case 14:
					_tcsncpy(pszText, pKnownFile->GetStrTagValue(FT_MEDIA_TITLE), cchTextMax);
					break;
			
				case 15:{
					uint_ptr nMediaLength = pKnownFile->GetIntTagValue(FT_MEDIA_LENGTH);
					if (nMediaLength){
						CString buffer;
						SecToTimeLength(nMediaLength, buffer);
						_tcsncpy(pszText, buffer, cchTextMax);
					}
					break;
				}
			
				case 16:{
					uint32 nBitrate = pKnownFile->GetIntTagValue(FT_MEDIA_BITRATE);
					if (nBitrate)
						_sntprintf(pszText, cchTextMax, _T("%u %s"), nBitrate, GetResString(IDS_KBITSSEC));
					break;
				}
			
				case 17:
					_tcsncpy(pszText, GetCodecDisplayName(pKnownFile->GetStrTagValue(FT_MEDIA_CODEC)), cchTextMax);
					break;

				//Xman see OnUploadqueue
				case 18:
					_ultot_s(pKnownFile->GetOnUploadqueue(), pszText, cchTextMax, 10);// X: [CI] - [Code Improvement]
			    break;
				//Xman end
			}
		}
          break;
	}

	pszText[cchTextMax - 1] = _T('\0');
}

void CSharedFilesCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
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

// ShellContextMenu :: Start
	delete m_pSCM;
	m_pSCM = NULL;
	CMenu ShellContextMenu;
// ShellContextMenu :: End

	// get merged settings
	bool bFirstItem = true;
	bool bContainsShareableFiles = false;
	bool bContainsOnlyShareableFile = true;
	bool bContainsUnshareableFile = false; 
	size_t iSelectedItems = GetSelectedCount();
	sint_ptr iCompleteFileSelected = -1;
	UINT uPrioMenuItem = 0;
	const CShareableFile* pSingleSelFile = NULL;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos)
	{
		const CShareableFile* pFile = (CShareableFile*)GetItemData(GetNextSelectedItem(pos));
		if (bFirstItem)
			pSingleSelFile = pFile;
		else
			pSingleSelFile = NULL;

		sint_ptr iCurCompleteFile = pFile->IsPartFile() ? 0 : 1;
		if (bFirstItem)
			iCompleteFileSelected = iCurCompleteFile;
		else if (iCompleteFileSelected != iCurCompleteFile)
			iCompleteFileSelected = -1;

		bContainsUnshareableFile = !pFile->IsShellLinked() && !pFile->IsPartFile() && (bContainsUnshareableFile || (theApp.sharedfiles->ShouldBeShared(pFile->GetSharedDirectory(), pFile->GetFilePath(), false)
			&& !theApp.sharedfiles->ShouldBeShared(pFile->GetSharedDirectory(), _T("")/*pFile->GetFilePath()*/, true)));

// ShellContextMenu :: Start
	   bool bSingleCompleteFileSelected = (iSelectedItems == 1 && (iCompleteFileSelected == 1 || bContainsOnlyShareableFile));
	   ShellContextMenu.CreateMenu();
	   if (bSingleCompleteFileSelected && !pFile->IsPartFile())
	   {
	        CString	strBuffer = ConcatFullPath(pFile->GetPath(), pFile->GetFileName());
					m_pSCM = new CShellContextMenu(m_hWnd, strBuffer);
	   m_pSCM->SetMenu(&ShellContextMenu);
	   m_SharedFilesMenu.AppendMenu(MF_STRING | MF_POPUP, (UINT_PTR)ShellContextMenu.m_hMenu, GetResString(IDS_SHELLCONTEXT));	
	   }
// ShellContextMenu :: End

		if (IsKindOfCKnownFile(pFile)/*pFile->IsKindOf(RUNTIME_CLASS(CKnownFile))*/)
		{
			bContainsOnlyShareableFile = false;

			UINT uCurPrioMenuItem = 0;
			if (((CKnownFile*)pFile)->IsAutoUpPriority())
				uCurPrioMenuItem = MP_PRIOAUTO;
			else if (((CKnownFile*)pFile)->GetUpPriority() == PR_VERYLOW)
				uCurPrioMenuItem = MP_PRIOVERYLOW;
			else if (((CKnownFile*)pFile)->GetUpPriority() == PR_LOW)
				uCurPrioMenuItem = MP_PRIOLOW;
			else if (((CKnownFile*)pFile)->GetUpPriority() == PR_NORMAL)
				uCurPrioMenuItem = MP_PRIONORMAL;
			else if (((CKnownFile*)pFile)->GetUpPriority() == PR_HIGH)
				uCurPrioMenuItem = MP_PRIOHIGH;
			else if (((CKnownFile*)pFile)->GetUpPriority() == PR_VERYHIGH)
				uCurPrioMenuItem = MP_PRIOVERYHIGH;
			//Xman PowerRelease
			else if (((CKnownFile*)pFile)->GetUpPriority()==PR_POWER)
				uCurPrioMenuItem = MP_PRIOPOWER;
			//Xman end
			else
				ASSERT(0);

			if (bFirstItem)
				uPrioMenuItem = uCurPrioMenuItem;
			else if (uPrioMenuItem != uCurPrioMenuItem)
				uPrioMenuItem = 0;
		}
		else
			bContainsShareableFiles = true;

		bFirstItem = false;
	}

	m_SharedFilesMenu.EnableMenuItem((UINT)m_PrioMenu.m_hMenu, (!bContainsOnlyShareableFile && iSelectedItems > 0) ? MF_ENABLED : MF_GRAYED);
	//m_PrioMenu.CheckMenuRadioItem(MP_PRIOVERYLOW, MP_PRIOAUTO, uPrioMenuItem, 0);
	m_PrioMenu.CheckMenuRadioItem(MP_PRIOVERYLOW, MP_PRIOPOWER, uPrioMenuItem, 0); //Xman PowerRelease 

	bool bSingleCompleteFileSelected = (iSelectedItems == 1 && (iCompleteFileSelected == 1 || bContainsOnlyShareableFile));
// ShellContextMenu :: Start
	//m_SharedFilesMenu.EnableMenuItem(MP_OPEN, bSingleCompleteFileSelected ? MF_ENABLED : MF_GRAYED);
// ShellContextMenu :: End
	//Xman PowerRelease
	m_SharedFilesMenu.EnableMenuItem(MP_PRIOPOWER, /*iCompleteFileSelected*/iSelectedItems >0 ? MF_ENABLED : MF_GRAYED);
	//Xman end
	m_SharedFilesMenu.EnableMenuItem(MP_OPENFOLDER, bSingleCompleteFileSelected ? MF_ENABLED : MF_GRAYED);
// ShellContextMenu :: Start
	//m_SharedFilesMenu.EnableMenuItem(MP_RENAME, (!bContainsShareableFiles && bSingleCompleteFileSelected) ? MF_ENABLED : MF_GRAYED);
// ShellContextMenu :: End
	m_SharedFilesMenu.EnableMenuItem(MP_REMOVE, iCompleteFileSelected > 0 ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(MP_UNSHAREFILE, bContainsUnshareableFile ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(MP_SPREADBAR_RESET, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED); // Spread bars [Slugfiller/MorphXT] - Stulle
// ShellContextMenu :: Start
	//m_SharedFilesMenu.SetDefaultItem(bSingleCompleteFileSelected ? MP_OPEN : -1);
// ShellContextMenu :: End
	m_SharedFilesMenu.EnableMenuItem(MP_DETAIL, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(thePrefs.GetShowCopyEd2kLinkCmd() ? MP_GETED2KLINK : MP_SHOWED2KLINK, (!bContainsOnlyShareableFile && iSelectedItems > 0) ? MF_ENABLED : MF_GRAYED);
	//m_SharedFilesMenu.EnableMenuItem(MP_SEARCHRELATED, theApp.emuledlg->searchwnd->CanSearchRelatedFiles() ? MF_ENABLED : MF_GRAYED);

	m_CollectionsMenu.EnableMenuItem(MP_VIEWCOLLECTION, (!bContainsShareableFiles && pSingleSelFile != NULL && ((CKnownFile*)pSingleSelFile)->m_pCollection != NULL ) ? MF_ENABLED : MF_GRAYED);
	m_CollectionsMenu.EnableMenuItem(MP_SEARCHAUTHOR, (!bContainsShareableFiles && pSingleSelFile != NULL && ((CKnownFile*)pSingleSelFile)->m_pCollection != NULL 
		&& !((CKnownFile*)pSingleSelFile)->m_pCollection->GetAuthorKeyHashString().IsEmpty()) ? MF_ENABLED : MF_GRAYED);
#if defined(_DEBUG)
	if (thePrefs.IsExtControlsEnabled()){
	//JOHNTODO: Not for release as we need kad lowID users in the network to see how well this work work. Also, we do not support these links yet.
		m_SharedFilesMenu.EnableMenuItem(MP_GETKADSOURCELINK, (iSelectedItems > 0 && theApp.IsConnected() && theApp.IsFirewalled() && theApp.clientlist->GetBuddy()) ? MF_ENABLED : MF_GRAYED);
	}
#endif
	m_SharedFilesMenu.CheckMenuItem(MP_SHOWSHAREABLEFILES, thePrefs.showShareableFile?MF_CHECKED:MF_UNCHECKED);
	GetPopupMenuPos(*this, point);
	m_SharedFilesMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON,point.x,point.y,this);
// ShellContextMenu :: Start
	if (m_pSCM != NULL){
		m_pSCM->CleanUp();
	    m_SharedFilesMenu.RemoveMenu(m_SharedFilesMenu.GetMenuItemCount()-1,MF_BYPOSITION);
	}
// ShellContextMenu :: End
}

BOOL CSharedFilesCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	wParam = LOWORD(wParam);
	if(wParam == MP_SHOWSHAREABLEFILES){
		thePrefs.showShareableFile = !thePrefs.showShareableFile;
		ReloadFileList();
		return TRUE;
	}

	CAtlList<CShareableFile*> selectedList;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos != NULL){
		int index = GetNextSelectedItem(pos);
		if (index >= 0)
			selectedList.AddTail((CShareableFile*)GetItemData(index));
	}

	if (selectedList.GetCount() > 0)
	{
		CShareableFile* file = NULL;
		if (selectedList.GetCount() == 1)
			file = selectedList.GetHead();

		CKnownFile* pKnownFile = NULL;
		if (file != NULL && IsKindOfCKnownFile(file)/*file->IsKindOf(RUNTIME_CLASS(CKnownFile))*/)
			pKnownFile = (CKnownFile*)file;

		switch (wParam){
			case MP_GETED2KLINK:{
				CString str;
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					CShareableFile* file = selectedList.GetNext(pos);
					if (file != NULL && IsKindOfCKnownFile(file)/*file->IsKindOf(RUNTIME_CLASS(CKnownFile))*/){
						if (!str.IsEmpty())
							str += _T("\r\n");
						str += ((CKnownFile*)file)->GetED2kLink();
					}
				}
				theApp.CopyTextToClipboard(str);
				break;
			}
#if defined(_DEBUG)
			//JOHNTODO: Not for release as we need kad lowID users in the network to see how well this work work. Also, we do not support these links yet.
			case MP_GETKADSOURCELINK:{
				CString str;
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					CShareableFile* file = selectedList.GetNext(pos);
					if (IsKindOfCKnownFile(file)/*file->IsKindOf(RUNTIME_CLASS(CKnownFile))*/){
						if (!str.IsEmpty())
							str += _T("\r\n");
						str += theApp.CreateKadSourceLink((CKnownFile*)file);
					}
				}
				theApp.CopyTextToClipboard(str);
				break;
			}
#endif
			// file operations
// ShellContextMenu :: Start
			//case MP_OPEN:
// ShellContextMenu :: End
			case IDA_ENTER:
				if (file && !file->IsPartFile())
					OpenFile(file);
				break; 
			case MP_OPENFOLDER:
				if (file && !file->IsPartFile())
					ShellExecute(NULL, _T("open"), _T("explorer"), _T("/select,\"") + file->GetFilePath() + _T("\""), NULL, SW_SHOW);
				break; 
// ShellContextMenu :: Start
			//case MP_RENAME:
// ShellContextMenu :: End
			case MPG_F2:
				if (pKnownFile && !pKnownFile->IsPartFile()){
					InputBox inputbox;
					CString title = GetResString(IDS_RENAME);
					title.Remove(_T('&'));
					inputbox.SetLabels(title, GetResString(IDS_DL_FILENAME), pKnownFile->GetFileName());
					inputbox.SetEditFilenameMode();
					inputbox.DoModal();
					CString newname = inputbox.GetInput();
					if (!inputbox.WasCancelled() && newname.GetLength()>0)
					{
						// at least prevent users from specifying something like "..\dir\file"
						static const TCHAR _szInvFileNameChars[] = _T("\\/:*?\"<>|");
						if (newname.FindOneOf(_szInvFileNameChars) != -1){
							AfxMessageBox(GetErrorMessage(ERROR_BAD_PATHNAME));
							break;
						}

						CString newpath;
						PathCombine(newpath.GetBuffer(MAX_PATH), file->GetPath(), newname);
						newpath.ReleaseBuffer();
						if (_trename(pKnownFile->GetFilePath(), newpath) != 0){
							CString strError;
							strError.Format(GetResString(IDS_ERR_RENAMESF), file->GetFilePath(), newpath, _tcserror(errno));
							AfxMessageBox(strError);
							break;
						}

						if (IsCPartFile(pKnownFile)/*pKnownFile->IsKindOf(RUNTIME_CLASS(CPartFile))*/)
						{
							pKnownFile->SetFileName(newname);
							//STATIC_DOWNCAST(CPartFile, pKnownFile)->SetFullName(newpath); 
							((CPartFile*)pKnownFile)->SetFullName(newpath); 
						}
						else
						{
							//theApp.sharedfiles->RemoveKeywords(pKnownFile);// X: [BF] - [Bug Fix] Don't need to remove & add
							pKnownFile->SetFileName(newname);
							//theApp.sharedfiles->AddKeywords(pKnownFile);
						}
						pKnownFile->SetFilePath(newpath);
						UpdateFile(pKnownFile);
					}
				}
				else
					MessageBeep(MB_OK);
				break;
			case MP_REMOVE:
			case MPG_DELETE:
			{
				if (IDNO == AfxMessageBox(GetResString(IDS_CONFIRM_FILEDELETE),MB_ICONWARNING | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_YESNO))
					return TRUE;

				SetRedraw(FALSE);
				bool bRemovedItems = false;
				while (!selectedList.IsEmpty())
				{
					CShareableFile* myfile = selectedList.RemoveHead();
					if (!myfile || myfile->IsPartFile())
						continue;
					
					bool delsucc = ShellDeleteFile(myfile->GetFilePath());
					if (delsucc){
						if (IsKindOfCKnownFile(myfile)/*myfile->IsKindOf(RUNTIME_CLASS(CKnownFile))*/){
							theApp.sharedfiles->RemoveFile((CKnownFile*)myfile, true);
							if(!thePrefs.m_bDisableHistoryList && !thePrefs.m_bHistoryShowShared){
								theApp.emuledlg->transferwnd->historylistctrl.AddFile((CKnownFile*)myfile);
								//theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2History);
							}
						}
						else
							RemoveFile(myfile, true);
						bRemovedItems = true;
					}
					else{
						CString strError;
						strError.Format( GetResString(IDS_ERR_DELFILE) + _T("\r\n\r\n%s"), myfile->GetFilePath(), GetErrorMessage(GetLastError()));
						AfxMessageBox(strError);
					}
				}
				SetRedraw(TRUE);
				if (bRemovedItems) {
					AutoSelectItem();
					// Depending on <no-idea> this does not always cause a
					// LVN_ITEMACTIVATE message sent. So, explicitly redraw
					// the item.
				}
				break; 
			}
			case MP_UNSHAREFILE:
			{
				SetRedraw(FALSE);
				bool bUnsharedItems = false;
				while (!selectedList.IsEmpty())
				{
					CShareableFile* myfile = selectedList.RemoveHead();
					if (!myfile || myfile->IsPartFile() || !theApp.sharedfiles->ShouldBeShared(myfile->GetPath(), myfile->GetFilePath(), false)
						|| theApp.sharedfiles->ShouldBeShared(myfile->GetPath(), _T("")/*myfile->GetFilePath()*/, true))
					{
						continue; 
					}

					if(theApp.sharedfiles->ExcludeFile(myfile->GetFilePath())){
						bUnsharedItems = true;
						if(!thePrefs.m_bDisableHistoryList && !thePrefs.m_bHistoryShowShared){
							theApp.emuledlg->transferwnd->historylistctrl.AddFile((CKnownFile*)myfile);
							//theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2History);
						}
					}
					else
						ASSERT(0);
				}
				SetRedraw(TRUE);
				if (bUnsharedItems) {
					if (GetFirstSelectedItemPosition() == NULL)
						AutoSelectItem();
				}
				break; 
			}
			case MPG_ALTENTER:
			case MP_DETAIL:
				ShowFileDialog(selectedList);
				break;
			case MP_SEARCHAUTHOR:
				if (pKnownFile && pKnownFile->m_pCollection)
				{
					SSearchParams* pParams = new SSearchParams;
					pParams->strExpression = pKnownFile->m_pCollection->GetCollectionAuthorKeyString();
					pParams->eType = SearchTypeKademlia;
					pParams->strFileType = ED2KFTSTR_EMULECOLLECTION;
					pParams->strSpecialTitle = pKnownFile->m_pCollection->m_sCollectionAuthorName;
					if (pParams->strSpecialTitle.GetLength() > 50){
						pParams->strSpecialTitle = pParams->strSpecialTitle.Left(50) + _T("...");
					}
					theApp.emuledlg->searchwnd->m_pwndResults->StartSearch(pParams);
				}
				break;
			case MP_VIEWCOLLECTION:
				if (pKnownFile && pKnownFile->m_pCollection)
				{
					// NEO: MLD - [ModelesDialogs] -- Xanatos -->
					CCollectionViewDialog* dialog = new CCollectionViewDialog(); 
					dialog->SetCollection(pKnownFile->m_pCollection);
					dialog->OpenDialog();
					// NEO: MLD END <-- Xanatos --
					//CCollectionViewDialog dialog;
					//dialog.SetCollection(pKnownFile->m_pCollection);
					//dialog.DoModal();
				}
				break;
			case MP_SHOWED2KLINK:
				ShowFileDialog(selectedList, IDD_ED2KLINK);
				break;
			case MP_PRIOVERYLOW:
			case MP_PRIOLOW:
			case MP_PRIONORMAL:
			case MP_PRIOHIGH:
			case MP_PRIOVERYHIGH:
			case MP_PRIOPOWER:  //Xman PowerRelease
			case MP_PRIOAUTO:
			{
				SetRedraw(FALSE); //Xman Code Improvement
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					if (!IsKindOfCKnownFile(selectedList.GetAt(pos))/*!selectedList.GetAt(pos)->IsKindOf(RUNTIME_CLASS(CKnownFile))*/){
						selectedList.GetNext(pos);
						continue;
					}
					CKnownFile* file = (CKnownFile*)selectedList.GetNext(pos);
					switch (wParam) {
						case MP_PRIOVERYLOW:
							file->SetAutoUpPriority(false);
							file->SetUpPriority(PR_VERYLOW);
							UpdateFile(file);
							break;
						case MP_PRIOLOW:
							file->SetAutoUpPriority(false);
							file->SetUpPriority(PR_LOW);
							UpdateFile(file);
							break;
						case MP_PRIONORMAL:
							file->SetAutoUpPriority(false);
							file->SetUpPriority(PR_NORMAL);
							UpdateFile(file);
							break;
						case MP_PRIOHIGH:
							file->SetAutoUpPriority(false);
							file->SetUpPriority(PR_HIGH);
							UpdateFile(file);
							break;
						case MP_PRIOVERYHIGH:
							file->SetAutoUpPriority(false);
							file->SetUpPriority(PR_VERYHIGH);
							UpdateFile(file);
							break;	
						//Xman PowerRelease
						case MP_PRIOPOWER:
							/*
							if(file->IsPartFile()) //only to be sure
								break;
							*/
							file->SetAutoUpPriority(false);
							file->SetUpPriority(PR_POWER);
							UpdateFile(file);
							break;
						//Xman end
						case MP_PRIOAUTO:
							file->SetAutoUpPriority(true);
							//Xman advanced upload-priority
							if (thePrefs.UseAdvancedAutoPtio())
#ifdef _DEBUG
								file->CalculateAndSetUploadPriority2(); 
#else
								file->CalculateAndSetUploadPriority(); 
#endif
							else
								file->UpdateAutoUpPriority();
							//Xman end
							UpdateFile(file); 
							break;
					}
				}
				SetRedraw(TRUE); //Xman Code Improvement
			
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
// ==> Spread bars [Slugfiller/MorphXT] - Stulle
			case MP_SPREADBAR_RESET:
			{
				SetRedraw(FALSE);
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					CShareableFile* file = selectedList.GetNext(pos);
					((CKnownFile*)file)->statistic.ResetSpreadBar();	
				UpdateFile(file);
                }
				SetRedraw(TRUE);
				break;
			}
			// <== Spread bars [Slugfiller/MorphXT] - Stulle
// ShellContextMenu :: Start
	        default:
		    {
			if ((m_pSCM != NULL) && m_pSCM->IsMenuCommand(wParam))
				m_pSCM->InvokeCommand(wParam, pKnownFile);
			break;
		    }
// ShellContextMenu :: End
		}
	}
	return TRUE;
}

void CSharedFilesCtrl::OnLvnColumnClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLISTVIEW *pNMListView = (NMLISTVIEW *)pNMHDR;
	bool sortAscending;
	if (GetSortItem() != pNMListView->iSubItem)
	{
		switch (pNMListView->iSubItem)
		{
			case 3:  // Priority
			case 10: // Complete Sources
			case 11: // Shared
				//sortAscending = false;
				//break;
			case 5:  // Requests
			case 6:  // Accepted Requests
			case 7:  // Transferred Data
				// Keep the current 'm_aSortBySecondValue' for that column, but reset to 'descending'
				sortAscending = false;
				break;
			default:
				sortAscending = true;
				break;
		}
	}
	else
		sortAscending = !GetSortAscending();

	// Ornis 4-way-sorting
	int adder = 0;
	if (pNMListView->iSubItem >= 5 && pNMListView->iSubItem <= 7) // 5=IDS_SF_REQUESTS, 6=IDS_SF_ACCEPTS, 7=IDS_SF_TRANSFERRED
	{
		ASSERT( pNMListView->iSubItem - 5 < _countof(m_aSortBySecondValue) );
		if (GetSortItem() == pNMListView->iSubItem && !sortAscending) // check for 'descending' because the initial sort order is also 'descending'
			m_aSortBySecondValue[pNMListView->iSubItem - 5] = !m_aSortBySecondValue[pNMListView->iSubItem - 5];
		adder = m_aSortBySecondValue[pNMListView->iSubItem - 5] ? 100 : 0;
	}
	else if (pNMListView->iSubItem == 11) // 11=IDS_SHAREDTITLE
	{
		ASSERT( 3 < _countof(m_aSortBySecondValue) );
		if (GetSortItem() == pNMListView->iSubItem && !sortAscending) // check for 'descending' because the initial sort order is also 'descending'
			m_aSortBySecondValue[3] = !m_aSortBySecondValue[3];
		adder = m_aSortBySecondValue[3] ? 100 : 0;
	}

	// Sort table
	if (adder == 0)
		SetSortArrow(pNMListView->iSubItem, sortAscending);
	else
		SetSortArrow(pNMListView->iSubItem, sortAscending ? arrowDoubleUp : arrowDoubleDown);
	UpdateSortHistory(pNMListView->iSubItem + (sortAscending ? 0 : 20) + adder, 20);
	SortItems(SortProc, pNMListView->iSubItem + (sortAscending ? 0 : 20) + adder);

	*pResult = 0;
}

int CSharedFilesCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	const CShareableFile* item1 = (CShareableFile*)lParam1;
	const CShareableFile* item2 = (CShareableFile*)lParam2;

	bool bSortAscending;
	LPARAM iColumn;
	if (lParamSort >= 100) {
		bSortAscending = lParamSort < 120;
		iColumn = bSortAscending ? lParamSort : lParamSort - 20;
	}
	else {
		bSortAscending = lParamSort < 20;
		iColumn = bSortAscending ? lParamSort : lParamSort - 20;
	}
	
	int iResult = 0;
	bool bExtColumn = false;
	switch (iColumn)
	{
		case 0: //filename
			iResult = CompareLocaleStringNoCase(item1->GetFileName(), item2->GetFileName());
			break;

		case 1: //filesize
			iResult = CompareUnsigned64(item1->GetFileSize(), item2->GetFileSize());
			break;

		case 2: //filetype
			iResult = item1->GetFileTypeDisplayStr().Compare(item2->GetFileTypeDisplayStr());
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

		case 9: //folder
			iResult = CompareLocaleStringNoCase(item1->GetPath(), item2->GetPath());
			break;
		default:
			bExtColumn = true;
			break;
	}

	if (bExtColumn)
		{
		if (IsKindOfCKnownFile(item1) && !IsKindOfCKnownFile(item2)/*item1->IsKindOf(RUNTIME_CLASS(CKnownFile)) && !item2->IsKindOf(RUNTIME_CLASS(CKnownFile))*/)
				iResult = (-1);
		else if (!IsKindOfCKnownFile(item1) && IsKindOfCKnownFile(item2)/*!item1->IsKindOf(RUNTIME_CLASS(CKnownFile)) && item2->IsKindOf(RUNTIME_CLASS(CKnownFile))*/)
				iResult = 1;
		else if (IsKindOfCKnownFile(item1) && IsKindOfCKnownFile(item2)/*item1->IsKindOf(RUNTIME_CLASS(CKnownFile)) && item2->IsKindOf(RUNTIME_CLASS(CKnownFile))*/)
			{
				CKnownFile* kitem1 = (CKnownFile*)item1;
				CKnownFile* kitem2 = (CKnownFile*)item2;

				switch (iColumn)
				{
					case 3:{//prio
						uint8 p1 = kitem1->GetUpPriority() + 1;
						if (p1 == 5)
							p1 = 0;
						uint8 p2 = kitem2->GetUpPriority() + 1;
						if (p2 == 5)
							p2 = 0;
						iResult = p1 - p2;
						break;
					}

					case 4: //fileID
						iResult = memcmp(kitem1->GetFileHash(), kitem2->GetFileHash(), 16);
						break;

					case 5: //requests
						iResult = CompareUnsigned(kitem1->statistic.GetRequests(), kitem2->statistic.GetRequests());
						break;
				
					case 6: //acc requests
						iResult = CompareUnsigned(kitem1->statistic.GetAccepts(), kitem2->statistic.GetAccepts());
						break;
				
					case 7: //all transferred
						iResult = CompareUnsigned64(kitem1->statistic.GetTransferred(), kitem2->statistic.GetTransferred());
						break;

// ==> Spread bars [Slugfiller/MorphXT] - Stulle
				case 8: //spread asc
					iResult=CompareFloat(kitem1->statistic.GetSpreadSortValue(),kitem2->statistic.GetSpreadSortValue());
					break;
				// <== Spread bars [Slugfiller/MorphXT] - Stulle

				case 10: //complete sources
						iResult = CompareUnsigned(kitem1->m_nCompleteSourcesCount, kitem2->m_nCompleteSourcesCount);
						break;

				case 11: //ed2k shared
						iResult = kitem1->GetPublishedED2K() - kitem2->GetPublishedED2K();
						break;

				case 12:
						iResult = CompareOptLocaleStringNoCaseUndefinedAtBottom(kitem1->GetStrTagValue(FT_MEDIA_ARTIST), kitem2->GetStrTagValue(FT_MEDIA_ARTIST), bSortAscending);
						break;
			
				case 13:
						iResult = CompareOptLocaleStringNoCaseUndefinedAtBottom(kitem1->GetStrTagValue(FT_MEDIA_ALBUM), kitem2->GetStrTagValue(FT_MEDIA_ALBUM), bSortAscending);
						break;

				case 14:
						iResult = CompareOptLocaleStringNoCaseUndefinedAtBottom(kitem1->GetStrTagValue(FT_MEDIA_TITLE), kitem2->GetStrTagValue(FT_MEDIA_TITLE), bSortAscending);
						break;

				case 15:
						iResult = CompareUnsignedUndefinedAtBottom(kitem1->GetIntTagValue(FT_MEDIA_LENGTH), kitem2->GetIntTagValue(FT_MEDIA_LENGTH), bSortAscending);
						break;

				case 16:
						iResult = CompareUnsignedUndefinedAtBottom(kitem1->GetIntTagValue(FT_MEDIA_BITRATE), kitem2->GetIntTagValue(FT_MEDIA_BITRATE), bSortAscending);
						break;

				case 17:
						iResult = CompareOptLocaleStringNoCaseUndefinedAtBottom(GetCodecDisplayName(kitem1->GetStrTagValue(FT_MEDIA_CODEC)), GetCodecDisplayName(kitem2->GetStrTagValue(FT_MEDIA_CODEC)), bSortAscending);
						break;
					//Xman see OnUploadqueue
				case 18:
					iResult = CompareUnsigned(kitem1->GetOnUploadqueue(), kitem2->GetOnUploadqueue());
						break;
					//Xman end

					case 105: //all requests
						iResult = CompareUnsigned(kitem1->statistic.GetAllTimeRequests(), kitem2->statistic.GetAllTimeRequests());
						break;

					case 106: //all acc requests
						iResult = CompareUnsigned(kitem1->statistic.GetAllTimeAccepts(), kitem2->statistic.GetAllTimeAccepts());
						break;

					case 107: //all transferred
						iResult = CompareUnsigned64(kitem1->statistic.GetAllTimeTransferred(), kitem2->statistic.GetAllTimeTransferred());
						break;

				case 111:{ //kad shared
						uint64 tNow = time(NULL);
						int i1 = (tNow < kitem1->GetLastPublishTimeKadSrc()) ? 1 : 0;
						int i2 = (tNow < kitem2->GetLastPublishTimeKadSrc()) ? 1 : 0;
						iResult = i1 - i2;
						break;
					}
				}
			}
		}

	if (!bSortAscending)
		return -iResult;
	// SLUGFILLER: multiSort remove - handled in parent class
	/*
	//call secondary sortorder, if this one results in equal
	int dwNextSort;
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->sharedfileswnd->sharedfilesctrl.GetNextSortOrder(lParamSort)) != (-1)){
		iResult= SortProc(lParam1, lParam2, dwNextSort);
	}
	*/
	return iResult;
}

void CSharedFilesCtrl::OpenFile(const CShareableFile* file)
{
	if(IsKindOfCKnownFile(file)/*file->IsKindOf(RUNTIME_CLASS(CKnownFile))*/ && ((CKnownFile*)file)->m_pCollection)
	{
		// NEO: MLD - [ModelesDialogs] -- Xanatos -->
		CCollectionViewDialog* dialog = new CCollectionViewDialog(); 
		dialog->SetCollection(((CKnownFile*)file)->m_pCollection);
		dialog->OpenDialog();
		// NEO: MLD END <-- Xanatos --
		//CCollectionViewDialog dialog;
		//dialog.SetCollection(((CKnownFile*)file)->m_pCollection);
		//dialog.DoModal();
	}
	else
		ShellOpenFile(file->GetFilePath(), NULL);
}

void CSharedFilesCtrl::OnNmDblClk(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1)
	{
		CShareableFile* file = (CShareableFile*)GetItemData(iSel);
		if (file)
		{
			if (GetKeyState(VK_MENU) & 0x8000)
			{
				CAtlList<CShareableFile*> aFiles;
				aFiles.AddHead(file);
				ShowFileDialog(aFiles);
			}
			else if (!file->IsPartFile())
				OpenFile(file);
		}
	}
	*pResult = 0;
}

void CSharedFilesCtrl::CreateMenues()
{
	if (m_PrioMenu) VERIFY( m_PrioMenu.DestroyMenu() );
	if (m_CollectionsMenu) VERIFY( m_CollectionsMenu.DestroyMenu() );
	if (m_SharedFilesMenu) VERIFY( m_SharedFilesMenu.DestroyMenu() );


	m_PrioMenu.CreateMenu();
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOVERYLOW,GetResString(IDS_PRIOVERYLOW));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOLOW,GetResString(IDS_PRIOLOW));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIONORMAL,GetResString(IDS_PRIONORMAL));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOHIGH, GetResString(IDS_PRIOHIGH));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOVERYHIGH, GetResString(IDS_PRIORELEASE));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOPOWER, GetResString(IDS_POWERRELEASE)); //Xman PowerRelease
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOAUTO, GetResString(IDS_PRIOAUTO));//UAP

	m_CollectionsMenu.CreateMenu();
	m_CollectionsMenu.AppendMenu(MF_STRING,MP_VIEWCOLLECTION, GetResString(IDS_VIEWCOLLECTION));
	m_CollectionsMenu.AppendMenu(MF_STRING,MP_SEARCHAUTHOR, GetResString(IDS_SEARCHAUTHORCOLLECTION));

	m_SharedFilesMenu.CreatePopupMenu();

// ShellContextMenu :: Start
	//m_SharedFilesMenu.AppendMenu(MF_STRING,MP_OPEN, GetResString(IDS_OPENFILE));
// ShellContextMenu :: End
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_OPENFOLDER, GetResString(IDS_OPENFOLDER));
// ShellContextMenu :: Start
	//m_SharedFilesMenu.AppendMenu(MF_STRING,MP_RENAME, GetResString(IDS_RENAME) + _T("..."));
// ShellContextMenu :: End
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_REMOVE, GetResString(IDS_DELETE));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_UNSHAREFILE, GetResString(IDS_UNSHARE));
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_SHOWSHAREABLEFILES,GetResString(IDS_SHOWSHAREABLE));
    m_SharedFilesMenu.AppendMenu(MF_STRING,MP_SPREADBAR_RESET, GetResString(IDS_SPREAD_RESET)); // Spread bars [Slugfiller/MorphXT] - Stulle
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_PrioMenu.m_hMenu, GetResString(IDS_PRIORITY) + _T(" (") + GetResString(IDS_PW_CON_UPLBL) + _T(')'));
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
    m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_CollectionsMenu.m_hMenu, GetResString(IDS_META_COLLECTION));
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR); 	

	if(thePrefs.IsExtControlsEnabled())
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_DETAIL, GetResString(IDS_SHOWDETAILS));
	if (thePrefs.GetShowCopyEd2kLinkCmd())
		m_SharedFilesMenu.AppendMenu(MF_STRING,MP_GETED2KLINK, GetResString(IDS_DL_LINK1) );
	else
		m_SharedFilesMenu.AppendMenu(MF_STRING,MP_SHOWED2KLINK, GetResString(IDS_DL_SHOWED2KLINK) );
	//m_SharedFilesMenu.AppendMenu(MF_STRING, MP_SEARCHRELATED, GetResString(IDS_SEARCHRELATED));

#if defined(_DEBUG)
	if (thePrefs.IsExtControlsEnabled()){
		//JOHNTODO: Not for release as we need kad lowID users in the network to see how well this work work. Also, we do not support these links yet.
		m_SharedFilesMenu.AppendMenu(MF_STRING,MP_GETKADSOURCELINK, _T("Copy eD2K Links To Clipboard (Kad)"));
	}
#endif
}

void CSharedFilesCtrl::ShowSelectedFileInfo()
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

	CShareableFile* file=(CShareableFile*)GetItemData(GetSelectionMark());
	if (file)
	{
		CAtlList<CShareableFile*> aFiles;
		aFiles.AddHead(file);
		ShowFileDialog(aFiles);
	}
}

void CSharedFilesCtrl::OnLvnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
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

void CSharedFilesCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == 'C' && (GetKeyState(VK_CONTROL) & 0x8000))
	{
		// Ctrl+C: Copy listview items to clipboard
		SendMessage(WM_COMMAND, MP_GETED2KLINK);
		return;
	}
	else if (nChar == VK_F5)
		ReloadFileList();
	else if (nChar == VK_SPACE && thePrefs.showShareableFile)
	{
		// Toggle Checkboxes
		// selection and item position might change during processing (shouldn't though, but lets make sure), so first get all pointers instead using the selection pos directly
		SetRedraw(FALSE);
		CAtlList<CShareableFile*> selectedList;
		POSITION pos = GetFirstSelectedItemPosition();
		while (pos != NULL){
			int index = GetNextSelectedItem(pos);
			if (index >= 0)
				selectedList.AddTail((CShareableFile*)GetItemData(index));
		}
		while (!selectedList.IsEmpty())
		{
			CheckBoxClicked(FindFile(selectedList.RemoveHead()));
		}
		SetRedraw(TRUE);
	}

	CMuleListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CSharedFilesCtrl::ShowFileDialog(CAtlList<CShareableFile*>& aFiles, UINT uPshInvokePage)
{
	if (aFiles.GetCount() > 0)
	{
		CSharedFileDetailsSheet dialog(aFiles, uPshInvokePage, this);
		dialog.DoModal();
	}
}

void CSharedFilesCtrl::SetDirectoryFilter(CDirectoryItem* pNewFilter, bool bRefresh){
	if (m_pDirectoryFilter == pNewFilter)
		return;
	m_pDirectoryFilter = pNewFilter;
	if (bRefresh)
		ReloadFileList();
}

void CSharedFilesCtrl::OnLvnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMLVGETINFOTIP>(pNMHDR);
	if (pGetInfoTip->iSubItem == 0)
	{
		LVHITTESTINFO hti = {0};
		::GetCursorPos(&hti.pt);
		ScreenToClient(&hti.pt);
		if (SubItemHitTest(&hti) == -1 || hti.iItem != pGetInfoTip->iItem || hti.iSubItem != 0){
			// don' show the default label tip for the main item, if the mouse is not over the main item
			if ((pGetInfoTip->dwFlags & LVGIT_UNFOLDED) == 0 && pGetInfoTip->cchTextMax > 0 && pGetInfoTip->pszText[0] != _T('\0'))
				pGetInfoTip->pszText[0] = _T('\0');
			return;
		}

		const CShareableFile* pFile = (CShareableFile*)GetItemData(pGetInfoTip->iItem);
		if (pFile && pGetInfoTip->pszText && pGetInfoTip->cchTextMax > 0)
		{
			CString strInfo = pFile->GetInfoSummary();
			strInfo += TOOLTIP_AUTOFORMAT_SUFFIX_CH;
			_tcsncpy(pGetInfoTip->pszText, strInfo, pGetInfoTip->cchTextMax);
			pGetInfoTip->pszText[pGetInfoTip->cchTextMax-1] = _T('\0');
		}
	}
	*pResult = 0;
}

void CSharedFilesCtrl::SetToolTipsDelay(DWORD dwDelay)
{
	CToolTipCtrl* tooltip = GetToolTips();
	if (tooltip)
		tooltip->SetDelayTime(TTDT_INITIAL, dwDelay);
}

bool CSharedFilesCtrl::IsSharedInKad(const CKnownFile *file) const
{
	return (uint64)time(NULL) < file->GetLastPublishTimeKadSrc() &&
		(!(theApp.IsFirewalled() && theApp.IsConnected()))
			|| ((theApp.clientlist->GetBuddy() && (file->GetLastPublishBuddy() == theApp.clientlist->GetBuddy()->GetIP()))
				|| (Kademlia::CKademlia::IsRunning() && !Kademlia::CUDPFirewallTester::IsFirewalledUDP(true) && Kademlia::CUDPFirewallTester::IsVerified()));
}

void CSharedFilesCtrl::AddShareableFiles(CString strFromDir)
{
	CString strSearchPath(strFromDir);
	PathAddBackslash(strSearchPath.GetBuffer(strFromDir.GetLength() + 1));
	strSearchPath.ReleaseBuffer();
	strSearchPath += _T("*");
	CFileFind ff;
	bool end = !ff.FindFile(strSearchPath, 0);
	if (end) {
		DWORD dwError = GetLastError();
		if (dwError != ERROR_FILE_NOT_FOUND)
			DebugLogError(_T("Failed to find files for SharedFilesListCtrl in %s, %s"), strFromDir, GetErrorMessage(dwError));
		return;
	}

	SetRedraw(FALSE);
	while (!end)
	{
		end = !ff.FindNextFile();
		if (ff.IsDirectory() || ff.IsDots() || ff.IsSystem() || ff.IsTemporary() || ff.GetLength()==0 || ff.GetLength()>MAX_EMULE_FILE_SIZE
			||(thePrefs.dontsharext&&checkExt(ff.GetFileName(), thePrefs.shareExt)))// X: [DSE] - [DontShareExt]
			continue;

		CString strFoundFileName(ff.GetFileName());
		CString strFoundFilePath(ff.GetFilePath());
		CString strFoundDirectory(strFoundFilePath.Left(ff.GetFilePath().ReverseFind('\\') + 1));
		ULONGLONG ullFoundFileSize = ff.GetLength();
		CTime tFoundFileTime;
		try{
			ff.GetLastWriteTime(tFoundFileTime);
		}
		catch(CException* ex){
			ex->Delete();
		}

		// ignore real(!) LNK files
		TCHAR szExt[_MAX_EXT];
		_tsplitpath_s(strFoundFileName, NULL, 0, NULL, 0, NULL, 0, szExt, _countof(szExt));
		if (_tcsicmp(szExt, _T(".lnk")) == 0){
			SHFILEINFO info;
			if (SHGetFileInfo(strFoundFilePath, 0, &info, sizeof(info), SHGFI_ATTRIBUTES) && (info.dwAttributes & SFGAO_LINK)){
				continue;
			}
		}

		// ignore real(!) thumbs.db files -- seems that lot of ppl have 'thumbs.db' files without the 'System' file attribute
		if (strFoundFileName.CompareNoCase(_T("thumbs.db")) == 0)
		{
			// if that's a valid 'Storage' file, we declare it as a "thumbs.db" file.
			CComPtr<IStorage> pStorage;
			if (StgOpenStorage(strFoundFilePath, NULL, STGM_READ | STGM_SHARE_DENY_WRITE, NULL, 0, &pStorage) == S_OK)
			{
				CComPtr<IEnumSTATSTG> pEnumSTATSTG;
				if (SUCCEEDED(pStorage->EnumElements(0, NULL, 0, &pEnumSTATSTG)))
				{
					STATSTG statstg = {0};
					if (pEnumSTATSTG->Next(1, &statstg, 0) == S_OK)
					{
						CoTaskMemFree(statstg.pwcsName);
						statstg.pwcsName = NULL;
						continue;
					}
				}
			}
		}

		uint64 fdate = tFoundFileTime.GetTime();// X: [64T] - [64BitTime]
		if (fdate == 0){
			if (thePrefs.GetVerbose())
				AddDebugLogLine(false, _T("Failed to get file date of \"%s\""), strFoundFilePath);
		}
		else
			AdjustNTFSDaylightFileTime(fdate, strFoundFilePath);


		CKnownFile* toadd = theApp.knownfiles->FindKnownFile(strFoundFileName, fdate, ullFoundFileSize);
		if (toadd != NULL && theApp.sharedfiles->GetFileByID(toadd->GetFileHash()) != NULL) // check for shared
		{
			// this file is already shared and should be on the list, nothing to do
		}
		else if (toadd != NULL) // for known
		{
			toadd->SetFilePath(strFoundFilePath);
			toadd->SetPath(strFoundDirectory);
			AddFile(toadd); // known, could be on the list already
		}
		else // not known or shared, create
		{
			CShareableFile* pNewTempFile = new CShareableFile();
			pNewTempFile->SetFilePath(strFoundFilePath);
			pNewTempFile->SetFileName(strFoundFileName);
			pNewTempFile->SetPath(strFoundDirectory);
			pNewTempFile->SetFileSize(ullFoundFileSize);
			uchar aucMD4[16];
			md4clr(aucMD4);
			pNewTempFile->SetFileHash(aucMD4);
			liTempShareableFilesInDir.AddTail(pNewTempFile);
			AddFile(pNewTempFile);
		}
	}
	SetRedraw(TRUE);
	ff.Close();
}

BOOL CSharedFilesCtrl::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (thePrefs.showShareableFile) // do we have checkboxes?
	{
		NMLISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

		int iItem = HitTest(pNMListView->ptAction);
		if (iItem != -1){
			// determine if the checkbox was clicked
			CRect recItem;
			if(GetItemRect(iItem, recItem, LVIR_BOUNDS))
			{
				CPoint pointHit = pNMListView->ptAction;
				ASSERT( recItem.PtInRect(pointHit) );
				recItem.left += sm_iSubItemInset;
				recItem.right = recItem.left + 16;
				recItem.top += (recItem.Height() > 16) ? ((recItem.Height() - 16) / 2) : 0;
				recItem.bottom = recItem.top + 16;
				if (recItem.PtInRect(pointHit)){
					// user clicked on the checkbox
					CheckBoxClicked(iItem);
				}
			}

		}
	}

	*pResult = 0;
	return FALSE; // pass on to parent window
}

void CSharedFilesCtrl::CheckBoxClicked(int iItem)
{
	if (iItem == (-1))
	{
		ASSERT( false );
		return;
	}
	// check which state the checkbox (should) currently have
	const CShareableFile* pFile = (CShareableFile*)GetItemData(iItem); 
	if (pFile->IsShellLinked()) 
		return; // no interacting with shelllinked files 
	if (theApp.sharedfiles->ShouldBeShared(pFile->GetPath(), pFile->GetFilePath(), false)){// file should be shared
		if (theApp.sharedfiles->ShouldBeShared(pFile->GetPath(), _T("")/*pFile->GetFilePath()*/, true)){// file must be shared
			if(IsKindOfCKnownFile(pFile)/*pFile->IsKindOf(RUNTIME_CLASS(CKnownFile))*/ ||// X: [QOH] - [QueryOnHashing] not allowed to unshare this known file
				!theApp.sharedfiles->CheckAndAddSingleFile(pFile->GetFilePath()))
				return;
			// this is shared just now
			if(!thePrefs.m_bDisableHistoryList && !thePrefs.m_bHistoryShowShared){
				theApp.emuledlg->transferwnd->historylistctrl.RemoveFileFromView((CKnownFile*)pFile);
				//theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2History);
			}
		}
		else if(IsKindOfCKnownFile(pFile)){// file was shared
			// this is currently shared so unshare it
	VERIFY( theApp.sharedfiles->ExcludeFile(pFile->GetFilePath()) );
			if(!thePrefs.m_bDisableHistoryList && !thePrefs.m_bHistoryShowShared){
				theApp.emuledlg->transferwnd->historylistctrl.AddFile((CKnownFile*)pFile);
				//theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2History);
			}
		}
		else if(!theApp.sharedfiles->CheckAndAddSingleFile(pFile->GetFilePath()))// file was unshared
			return;
	}
	else// file was excluded
	{
		// SLUGFILLER: SafeHash remove - removed installation dir unsharing
		/*
		if (!thePrefs.IsShareableDirectory(pFile->GetPath()))
			return; // not allowed to share
		*/
		if(!theApp.sharedfiles->AddSingleSharedFile(pFile->GetFilePath()))// X: [QOH] - [QueryOnHashing]
			return;
		UpdateFile(pFile);
		if(!thePrefs.m_bDisableHistoryList && !thePrefs.m_bHistoryShowShared){
			theApp.emuledlg->transferwnd->historylistctrl.RemoveFileFromView((CKnownFile*)pFile);
			//theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2History);
		}
	}
	// update GUI stuff
	theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Shared,GetItemCount());
}

void CSharedFilesCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	// highlighting Checkboxes
	if (thePrefs.showShareableFile)
	{
		// are we currently on any checkbox?
		int iItem = HitTest(point);
		if (iItem != (-1))
		{
			CRect recItem;
			if(GetItemRect(iItem, recItem, LVIR_BOUNDS))
			{
				ASSERT( recItem.PtInRect(point) );
				recItem.left += sm_iSubItemInset;
				recItem.right = recItem.left + 16;
				recItem.top += (recItem.Height() > 16) ? ((recItem.Height() - 16) / 2) : 0;
				recItem.bottom = recItem.top + 16;
				if (recItem.PtInRect(point)){
					// is this checkbox already hot?
					if (m_pHighlightedItem != (CShareableFile*)GetItemData(iItem))
					{
						// update old highlighted item
						CShareableFile* pOldItem = m_pHighlightedItem;
						m_pHighlightedItem = (CShareableFile*)GetItemData(iItem);
						UpdateFile(pOldItem);
						// highlight current item
						InvalidateRect(recItem);
					}
					CMuleListCtrl::OnMouseMove(nFlags, point);
					return;
				}
			}
		}
		// no checkbox should be hot
		if (m_pHighlightedItem != NULL)
		{
			CShareableFile* pOldItem = m_pHighlightedItem;
			m_pHighlightedItem = NULL;
			UpdateFile(pOldItem);
		}
	}
	CMuleListCtrl::OnMouseMove(nFlags, point);
}

CSharedFilesCtrl::CShareDropTarget::CShareDropTarget()
{
	m_piDropHelper = NULL;
	m_pParent = NULL;
    m_bUseDnDHelper = SUCCEEDED(CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, IID_IDropTargetHelper, (void**) &m_piDropHelper));
}

CSharedFilesCtrl::CShareDropTarget::~CShareDropTarget()
{
    if (m_piDropHelper != NULL)
        m_piDropHelper->Release();
}

DROPEFFECT CSharedFilesCtrl::CShareDropTarget::OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD /*dwKeyState*/, CPoint point)
{
	DROPEFFECT dwEffect = DROPEFFECT_NONE;

	if (pDataObject->IsDataAvailable(CF_HDROP))
		dwEffect = DROPEFFECT_COPY;

    if (m_bUseDnDHelper)
    {
        IDataObject* piDataObj = pDataObject->GetIDataObject(FALSE); 
        m_piDropHelper->DragEnter (pWnd->GetSafeHwnd(), piDataObj, &point, dwEffect);
    }

    return dwEffect;
}

DROPEFFECT CSharedFilesCtrl::CShareDropTarget::OnDragOver(CWnd* /*pWnd*/, COleDataObject* pDataObject, DWORD /*dwKeyState*/, CPoint point)
{
	DROPEFFECT dwEffect = DROPEFFECT_NONE;

	if (pDataObject->IsDataAvailable(CF_HDROP))
		dwEffect = DROPEFFECT_COPY;

    if (m_bUseDnDHelper)
    {
		m_piDropHelper->DragOver(&point, dwEffect);
    }

    return dwEffect;
}

BOOL CSharedFilesCtrl::CShareDropTarget::OnDrop(CWnd* /*pWnd*/, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
    HGLOBAL hGlobal = pDataObject->GetGlobalData(CF_HDROP);
	if (hGlobal != NULL)
	{
		HDROP hDrop = (HDROP)GlobalLock(hGlobal);
		if (hDrop != NULL)
		{
			CString strFilePath;
			CFileFind ff;
			CAtlList<CString> liToAddFiles; // all files too add
			CAtlList<CString> liToAddDirs; // all directories to add
			
			UINT nFileCount = DragQueryFile(hDrop, (UINT)(-1), NULL, 0);
			for (UINT nFile = 0; nFile < nFileCount; nFile++ )
			{
				if (DragQueryFile(hDrop, nFile, strFilePath.GetBuffer(MAX_PATH), MAX_PATH) > 0 )
				{
					strFilePath.ReleaseBuffer();
					if (ff.FindFile(strFilePath, 0))
					{
						ff.FindNextFile();
						// just a quick pre check, complete check is done later in the share function itself
						if (ff.IsDots() || ff.IsSystem() || ff.IsTemporary()
							|| (!ff.IsDirectory() && (ff.GetLength()==0 || ff.GetLength()>MAX_EMULE_FILE_SIZE))
							// SLUGFILLER: SafeHash remove - removed installation dir unsharing
							/*
							|| (ff.IsDirectory() && !thePrefs.IsShareableDirectory(ff.GetFilePath() + _T('\\')))
							*/
							|| (ff.IsDirectory() && theApp.sharedfiles->ShouldBeShared(ff.GetFilePath()+ _T('\\'), _T(""), false))
							|| (!ff.IsDirectory() && theApp.sharedfiles->ShouldBeShared(ff.GetFilePath(), ff.GetFilePath().Left(ff.GetFilePath().ReverseFind('\\') + 1), false)) )
						{
							DebugLog(_T("Drag&Drop'ed shared File ignored (%s)"), ff.GetFilePath()); 
							ff.Close();
							continue;
						}
						if (ff.IsDirectory())
						{
							DEBUG_ONLY( DebugLog(_T("Drag'n'Drop'ed directory: %s"), ff.GetFilePath()+ _T('\\'))  );
							liToAddDirs.AddTail(ff.GetFilePath() + _T('\\'));
						}
						else
						{
							DEBUG_ONLY( DebugLog(_T("Drag'n'Drop'ed file: %s"), ff.GetFilePath()) );
							liToAddFiles.AddTail(ff.GetFilePath());
						}
					}
					else
					{
						DebugLogError(_T("Drag&Drop'ed shared File not found (%s)"), strFilePath); 
					}
					ff.Close();

				}
				else
				{
					ASSERT( false );
					strFilePath.ReleaseBuffer();
				}
			}
			if (!liToAddFiles.IsEmpty() || !liToAddDirs.IsEmpty())
			{
				// add the directories first as they could
				// make single file adds invalid if they are contained in one of those dirs already 
				for (POSITION pos = liToAddDirs.GetHeadPosition(); pos != NULL; )
					VERIFY( theApp.sharedfiles->AddSingleSharedDirectory(liToAddDirs.GetNext(pos)) ); // should always succeed

				while (!liToAddFiles.IsEmpty())
					theApp.sharedfiles->AddSingleSharedFile(liToAddFiles.RemoveHead()); // could fail, due to the dirs added above

				// GUI updates
				theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Shared);
			}
			GlobalUnlock(hGlobal);
		}
		GlobalFree(hGlobal);
	}

    if (m_bUseDnDHelper)
    {
        IDataObject* piDataObj = pDataObject->GetIDataObject(FALSE); 
        m_piDropHelper->Drop(piDataObj, &point, dropEffect);
    }
    
    return TRUE;
}

void CSharedFilesCtrl::CShareDropTarget::OnDragLeave(CWnd* /*pWnd*/)
{
    if (m_bUseDnDHelper)
        m_piDropHelper->DragLeave();
}