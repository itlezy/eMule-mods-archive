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
#include "FileInfoDialog.h"
#include "MetaDataDlg.h"
#include "ED2kLinkDlg.h"
#include "ArchivePreviewDlg.h"
#include "CommentDialog.h"
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
#include "IrcWnd.h"
#include "SharedFilesWnd.h"
#include "Opcodes.h"
#include "InputBox.h"
#include "WebServices.h"
#include "TransferDlg.h"
#include "ClientList.h"
#include "UpDownClient.h"
#include "Collection.h"
#include "CollectionCreateDialog.h"
#include "CollectionViewDialog.h"
#include "SearchParams.h"
#include "SearchDlg.h"
#include "SearchResultsWnd.h"
#include "ToolTipCtrlX.h"
#include "kademlia/kademlia/kademlia.h"
#include "kademlia/kademlia/UDPFirewallTester.h"
#include "MediaInfo.h"
#include "Log.h"
#include "KnownFileList.h"
#include "VisualStylesXP.h"
#include "MassRename.h" //Xman Mass Rename (Morph)
#include "Log.h" //Xman Mass Rename (Morph)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

bool NeedArchiveInfoPage(const CSimpleArray<CObject*>* paItems);
void UpdateFileDetailsPages(CListViewPropertySheet *pSheet,
							CResizablePage *pArchiveInfo, CResizablePage *pMediaInfo);


//////////////////////////////////////////////////////////////////////////////
// CSharedFileDetailsSheet

class CSharedFileDetailsSheet : public CListViewWalkerPropertySheet
{
	DECLARE_DYNAMIC(CSharedFileDetailsSheet)

public:
	CSharedFileDetailsSheet(CTypedPtrList<CPtrList, CShareableFile*>& aFiles, UINT uPshInvokePage = 0, CListCtrlItemWalk* pListCtrl = NULL);
	virtual ~CSharedFileDetailsSheet();

protected:
	CFileInfoDialog		m_wndMediaInfo;
	CMetaDataDlg		m_wndMetaData;
	CED2kLinkDlg		m_wndFileLink;
	CCommentDialog		m_wndFileComments;
	CArchivePreviewDlg	m_wndArchiveInfo;

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

CSharedFileDetailsSheet::CSharedFileDetailsSheet(CTypedPtrList<CPtrList, CShareableFile*>& aFiles, UINT uPshInvokePage, CListCtrlItemWalk* pListCtrl)
	: CListViewWalkerPropertySheet(pListCtrl)
{
	m_uPshInvokePage = uPshInvokePage;
	POSITION pos = aFiles.GetHeadPosition();
	while (pos)
		m_aItems.Add(aFiles.GetNext(pos));
	m_psh.dwFlags &= ~PSH_HASHELP;

	m_wndFileComments.m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndFileComments.m_psp.dwFlags |= PSP_USEICONID;
	m_wndFileComments.m_psp.pszIcon = _T("FileComments");
	m_wndFileComments.SetFiles(&m_aItems);
	AddPage(&m_wndFileComments);

	m_wndArchiveInfo.m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndArchiveInfo.m_psp.dwFlags |= PSP_USEICONID;
	m_wndArchiveInfo.m_psp.pszIcon = _T("ARCHIVE_PREVIEW");
	m_wndArchiveInfo.SetFiles(&m_aItems);
	m_wndMediaInfo.m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndMediaInfo.m_psp.dwFlags |= PSP_USEICONID;
	m_wndMediaInfo.m_psp.pszIcon = _T("MEDIAINFO");
	m_wndMediaInfo.SetFiles(&m_aItems);
	if (NeedArchiveInfoPage(&m_aItems))
		AddPage(&m_wndArchiveInfo);
	else
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
	EnableSaveRestore(_T("SharedFileDetailsSheet")); // call this after(!) OnInitDialog
	UpdateTitle();
	return bResult;
}

LRESULT CSharedFileDetailsSheet::OnDataChanged(WPARAM, LPARAM)
{
	UpdateTitle();
	UpdateFileDetailsPages(this, &m_wndArchiveInfo, &m_wndMediaInfo);
	return 1;
}

void CSharedFileDetailsSheet::UpdateTitle()
{
	if (m_aItems.GetSize() == 1)
		SetWindowText(GetResString(IDS_DETAILS) + _T(": ") + STATIC_DOWNCAST(CAbstractFile, m_aItems[0])->GetFileName());
	else
		SetWindowText(GetResString(IDS_DETAILS));
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
				pSharedFilesCtrl->UpdateFile(DYNAMIC_DOWNCAST(CKnownFile, m_aItems[i]));
			}
		}
	}

	return CListViewWalkerPropertySheet::OnCommand(wParam, lParam);
}


//////////////////////////////////////////////////////////////////////////////
// CSharedFilesCtrl

IMPLEMENT_DYNAMIC(CSharedFilesCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CSharedFilesCtrl, CMuleListCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnClick)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnLvnGetDispInfo)
	ON_NOTIFY_REFLECT(LVN_GETINFOTIP, OnLvnGetInfoTip)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNmDblClk)
	ON_NOTIFY_REFLECT_EX(NM_CLICK, OnNMClick)
	ON_WM_CONTEXTMENU()
	ON_WM_KEYDOWN()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

CSharedFilesCtrl::CSharedFilesCtrl()
	: CListCtrlItemWalk(this)
{
	memset(&m_aSortBySecondValue, 0, sizeof(m_aSortBySecondValue));
	nAICHHashing = 0;
	m_pDirectoryFilter = NULL;
	SetGeneralPurposeFind(true);
	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	/*
	m_pToolTip = new CToolTipCtrlX;
	*/
	// workaround running MFC as service
	if (!theApp.IsRunningAsService())
		m_pToolTip = new CToolTipCtrlX;
	else
		m_pToolTip = NULL;
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle
	SetSkinKey(L"SharedFilesLv");
	m_pHighlightedItem = NULL;
}

CSharedFilesCtrl::~CSharedFilesCtrl()
{
	while (!liTempShareableFilesInDir.IsEmpty())	// delete shareble files
		delete liTempShareableFilesInDir.RemoveHead();
	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	// workaround running MFC as service
	if (!theApp.IsRunningAsService())
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle
		delete m_pToolTip;
	// ==> PowerShare [ZZ/MorphXT] - Stulle
	if (m_PowershareMenu) VERIFY( m_PowershareMenu.DestroyMenu() );
	if (m_PowerShareLimitMenu) VERIFY( m_PowerShareLimitMenu.DestroyMenu() );
	// <== PowerShare [ZZ/MorphXT] - Stulle
	// ==> Limit PS by amount of data uploaded [Stulle] - Stulle
	if (m_PsAmountLimitMenu) VERIFY( m_PsAmountLimitMenu.DestroyMenu() );
	// <== Limit PS by amount of data uploaded [Stulle] - Stulle
	// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	if (m_HideOSMenu) VERIFY( m_HideOSMenu.DestroyMenu() );
	if (m_SelectiveChunkMenu) VERIFY( m_SelectiveChunkMenu.DestroyMenu() );
	if (m_ShareOnlyTheNeedMenu) VERIFY( m_ShareOnlyTheNeedMenu.DestroyMenu() );
	// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	// ==> mem leak fix [fafner] - Stulle
	if (m_PrioMenu) VERIFY( m_PrioMenu.DestroyMenu() );
	if (m_CollectionsMenu) VERIFY( m_CollectionsMenu.DestroyMenu() );
	if (m_SharedFilesMenu) VERIFY( m_SharedFilesMenu.DestroyMenu() );
	// <== mem leak fix [fafner] - Stulle
}

void CSharedFilesCtrl::Init()
{
	SetPrefsKey(_T("SharedFilesCtrl"));
	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
	ASSERT( (GetStyle() & LVS_SINGLESEL) == 0 );

	InsertColumn(0, GetResString(IDS_DL_FILENAME),		LVCFMT_LEFT,  DFLT_FILENAME_COL_WIDTH);
	InsertColumn(1, GetResString(IDS_DL_SIZE),			LVCFMT_RIGHT, DFLT_SIZE_COL_WIDTH);
	InsertColumn(2, GetResString(IDS_TYPE),				LVCFMT_LEFT,  DFLT_FILETYPE_COL_WIDTH);
	InsertColumn(3, GetResString(IDS_PRIORITY),			LVCFMT_LEFT,  DFLT_PRIORITY_COL_WIDTH);
	InsertColumn(4, GetResString(IDS_FILEID),			LVCFMT_LEFT,  DFLT_HASH_COL_WIDTH,		-1, true);
	InsertColumn(5, GetResString(IDS_SF_REQUESTS),		LVCFMT_RIGHT, 100);
	InsertColumn(6, GetResString(IDS_SF_ACCEPTS),		LVCFMT_RIGHT, 100,						-1, true);
	InsertColumn(7, GetResString(IDS_SF_TRANSFERRED),	LVCFMT_RIGHT, 120);
	InsertColumn(8, GetResString(IDS_SHARED_STATUS),	LVCFMT_LEFT,  DFLT_PARTSTATUS_COL_WIDTH);
	InsertColumn(9, GetResString(IDS_FOLDER),			LVCFMT_LEFT,  DFLT_FOLDER_COL_WIDTH,	-1, true);
	InsertColumn(10,GetResString(IDS_COMPLSOURCES),		LVCFMT_RIGHT, 60);
	InsertColumn(11,GetResString(IDS_SHAREDTITLE),		LVCFMT_LEFT,  100);
	InsertColumn(12,GetResString(IDS_ARTIST),			LVCFMT_LEFT,  DFLT_ARTIST_COL_WIDTH,	-1, true);
	InsertColumn(13,GetResString(IDS_ALBUM),			LVCFMT_LEFT,  DFLT_ALBUM_COL_WIDTH,		-1, true);
	InsertColumn(14,GetResString(IDS_TITLE),			LVCFMT_LEFT,  DFLT_TITLE_COL_WIDTH,		-1, true);
	InsertColumn(15,GetResString(IDS_LENGTH),			LVCFMT_RIGHT, DFLT_LENGTH_COL_WIDTH,	-1, true);
	InsertColumn(16,GetResString(IDS_BITRATE),			LVCFMT_RIGHT, DFLT_BITRATE_COL_WIDTH,	-1, true);
	InsertColumn(17,GetResString(IDS_CODEC),			LVCFMT_LEFT,  DFLT_CODEC_COL_WIDTH,		-1, true);
	InsertColumn(18,GetResString(IDS_ONQUEUE),			LVCFMT_LEFT,  50); //Xman see OnUploadqueue
	InsertColumn(19,GetResString(IDS_SHAREFACTOR),			LVCFMT_LEFT,  100); //Xman advanced upload-priority
	// ==> Spread bars [Slugfiller/MorphXT] - Stulle
	InsertColumn(20,GetResString(IDS_SF_UPLOADED_PARTS),LVCFMT_LEFT,170,13);
	InsertColumn(21,GetResString(IDS_SF_TURN_PART),LVCFMT_LEFT,100,14);
	InsertColumn(22,GetResString(IDS_SF_TURN_SIMPLE),LVCFMT_LEFT,100,15);
	InsertColumn(23,GetResString(IDS_SF_FULLUPLOAD),LVCFMT_LEFT,100,16);
	// <== Spread bars [Slugfiller/MorphXT] - Stulle
	InsertColumn(24,GetResString(IDS_RARE_RATIO),LVCFMT_LEFT,100,17); // push rare file - Stulle
	// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	InsertColumn(25,GetResString(IDS_HIDEOS),LVCFMT_LEFT,100,18);
	InsertColumn(26,GetResString(IDS_SHAREONLYTHENEED),LVCFMT_LEFT,100,19);
	// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	InsertColumn(27,GetResString(IDS_POWERSHARE_COLUMN_LABEL),LVCFMT_LEFT,70,20); // PowerShare [ZZ/MorphXT] - Stulle

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
	// ==> PowerShare [ZZ/MorphXT] - Stulle
	/*
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0 : 20) + (GetSortSecondValue() ? 100 : 0));
	*/
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0 : 30)  + (GetSortSecondValue() ? 100 : 0));
	// <== PowerShare [ZZ/MorphXT] - Stulle

	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	// workaround running MFC as service
	if (!theApp.IsRunningAsService())
	{
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle
		CToolTipCtrl* tooltip = GetToolTips();
		if (tooltip){
			m_pToolTip->SetFileIconToolTip(true);
			m_pToolTip->SubclassWindow(*tooltip);
			tooltip->ModifyStyle(0, TTS_NOPREFIX);
			tooltip->SetDelayTime(TTDT_AUTOPOP, 20000);
			tooltip->SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1000);
		}
	} // Run eMule as NT Service [leuk_he/Stulle] - Stulle

	m_ShareDropTarget.SetParent(this);
	VERIFY( m_ShareDropTarget.Register(this) );
}

void CSharedFilesCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
	CreateMenues();
}

void CSharedFilesCtrl::SetAllIcons()
{
	ApplyImageList(NULL);
	m_ImageList.DeleteImageList();
	m_ImageList.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);
	m_ImageList.Add(CTempIconLoader(_T("EMPTY")));
	m_ImageList.Add(CTempIconLoader(_T("FileSharedServer")));
	m_ImageList.Add(CTempIconLoader(_T("FileSharedKad")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_NotRated")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Fake")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Poor")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Fair")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Good")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Excellent")));
	m_ImageList.Add(CTempIconLoader(_T("Collection_Search"))); // rating for comments are searched on kad
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("FileCommentsOvl"))), 1);
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

	strRes = GetResString(IDS_DL_FILENAME);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(0, &hdi);

	strRes = GetResString(IDS_DL_SIZE);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(1, &hdi);

	strRes = GetResString(IDS_TYPE);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(2, &hdi);

	strRes = GetResString(IDS_PRIORITY);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(3, &hdi);

	strRes = GetResString(IDS_FILEID);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(4, &hdi);

	strRes = GetResString(IDS_SF_REQUESTS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(5, &hdi);

	strRes = GetResString(IDS_SF_ACCEPTS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(6, &hdi);

	strRes = GetResString(IDS_SF_TRANSFERRED);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(7, &hdi);

	strRes = GetResString(IDS_SHARED_STATUS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(8, &hdi);

	strRes = GetResString(IDS_FOLDER);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(9, &hdi);

	strRes = GetResString(IDS_COMPLSOURCES);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(10, &hdi);

	strRes = GetResString(IDS_SHAREDTITLE);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(11, &hdi);

	strRes = GetResString(IDS_ARTIST);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(12, &hdi);

	strRes = GetResString(IDS_ALBUM);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(13, &hdi);

	strRes = GetResString(IDS_TITLE);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(14, &hdi);

	strRes = GetResString(IDS_LENGTH);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(15, &hdi);

	strRes = GetResString(IDS_BITRATE);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(16, &hdi);

	strRes = GetResString(IDS_CODEC);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(17, &hdi);

	//Xman see OnUploadqueue
	strRes = GetResString(IDS_ONQUEUE);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(18, &hdi);
	//Xman end

	//Xman advanced upload-priority
	strRes = GetResString(IDS_SHAREFACTOR);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(19, &hdi);
	//Xman end

	// ==> Spread bars [Slugfiller/MorphXT] - Stulle
	strRes = GetResString(IDS_SF_UPLOADED_PARTS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(20, &hdi);

	strRes = GetResString(IDS_SF_TURN_PART);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(21, &hdi);

	strRes = GetResString(IDS_SF_TURN_SIMPLE);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(22, &hdi);

	strRes = GetResString(IDS_SF_FULLUPLOAD);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(23, &hdi);
	// <== Spread bars [Slugfiller/MorphXT] - Stulle

	// ==> push rare file - Stulle
	strRes = GetResString(IDS_RARE_RATIO);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(24, &hdi);
	// <== push rare file - Stulle

	// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
    strRes = GetResString(IDS_HIDEOS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(25, &hdi);

	strRes = GetResString(IDS_SHAREONLYTHENEED);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(26, &hdi);
	// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle

	// ==> PowerShare [ZZ/MorphXT] - Stulle
	strRes = GetResString(IDS_POWERSHARE_COLUMN_LABEL);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(27, &hdi);
	// <== PowerShare [ZZ/MorphXT] - Stulle

	// ==> Design Settings [eWombat/Stulle] - Stulle
	COLORREF crTempColor = thePrefs.GetStyleBackColor(background_styles, style_b_sharedlist);

	if(crTempColor == CLR_DEFAULT)
		crTempColor = thePrefs.GetStyleBackColor(background_styles, style_b_default);

	if(crTempColor != CLR_DEFAULT)
		SetBkColor(crTempColor);
	else
		SetBkColor(COLORREF(RGB(255,255,255)));
	// <== Design Settings [eWombat/Stulle] - Stulle

	CreateMenues();

	int iItems = GetItemCount();
	for (int i = 0; i < iItems; i++)
		Update(i);

	ShowFilesCount();
}

void CSharedFilesCtrl::AddFile(const CShareableFile* file)
{
	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	if (theApp.IsRunningAsService(SVC_LIST_OPT))
		return;
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle

	if (!theApp.emuledlg->IsRunning())
		return;
	// check filter conditions if we should show this file right now
	if (m_pDirectoryFilter != NULL){
		ASSERT( file->IsKindOf(RUNTIME_CLASS(CKnownFile)) || m_pDirectoryFilter->m_eItemType == SDI_UNSHAREDDIRECTORY );
		switch(m_pDirectoryFilter->m_eItemType){
			case SDI_ALL:
				// No filter
				break;
			case SDI_FILESYSTEMPARENT:
				return;

			case SDI_UNSHAREDDIRECTORY:
				// Items from the whole filesystem tree
				if (file->IsPartFile())
					return;
			case SDI_NO:
				// some shared directory
			case SDI_CATINCOMING:
				// Categories with special incoming dirs
				if (CompareDirectories(file->GetSharedDirectory(), m_pDirectoryFilter->m_strFullPath) != 0)
					return;
				break;

			case SDI_TEMP:
				// only tempfiles
				if (!file->IsPartFile())
					return;
				else if (m_pDirectoryFilter->m_nCatFilter != -1 && (UINT)m_pDirectoryFilter->m_nCatFilter != ((CPartFile*)file)->GetCategory())
					return;
				break;

			case SDI_DIRECTORY:
				// any userselected shared dir but not incoming or temp
				if (file->IsPartFile())
					return;
				if (CompareDirectories(file->GetSharedDirectory(), thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR)) == 0)
					return;
				break;

			case SDI_INCOMING:
				// Main incoming directory
				if (CompareDirectories(file->GetSharedDirectory(), thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR)) != 0)
					return;
				// Hmm should we show all incoming files dirs or only those from the main incoming dir here?
				// hard choice, will only show the main for now
				break;

			// Avi3k: SharedView Ed2kType
			case SDI_ED2KFILETYPE:
				{
					if (m_pDirectoryFilter->m_nCatFilter == -1 || m_pDirectoryFilter->m_nCatFilter != GetED2KFileTypeID(file->GetFileName()))
						return;
					break;
				}
			// end Avi3k: SharedView Ed2kType
		}
	}
	if (IsFilteredItem(file))
		return;
	if (FindFile(file) != -1)
	{
		// in the filesystem view the shared status might have changed so we need to update the item to redraw the checkbox
		if (m_pDirectoryFilter != NULL && m_pDirectoryFilter->m_eItemType == SDI_UNSHAREDDIRECTORY)
			UpdateFile(file);
		return;
	}
	
	// if we are in the filesystem view, this might be a CKnownFile which has to replace a CShareableFile
	// (in case we start sharing this file), so make sure to replace the old one instead of adding a new
	if (m_pDirectoryFilter != NULL && m_pDirectoryFilter->m_eItemType == SDI_UNSHAREDDIRECTORY && file->IsKindOf(RUNTIME_CLASS(CKnownFile)))
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
					ShowFilesCount();
					return;
				}
			}
		}
	}

	int iItem = InsertItem(LVIF_TEXT | LVIF_PARAM, GetItemCount(), LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)file);
	if (iItem >= 0)
		Update(iItem);
	ShowFilesCount(); //Xman Code Improvement for ShowFilesCount
}

void CSharedFilesCtrl::RemoveFile(const CShareableFile* file, bool bDeletedFromDisk)
{
	int iItem = FindFile(file);
	if (iItem != -1)
	{
		if (!bDeletedFromDisk && m_pDirectoryFilter != NULL && m_pDirectoryFilter->m_eItemType == SDI_UNSHAREDDIRECTORY)
		{
			// in the file system view we usally dont need to remove a file, if it becomes unshared it will
			// still be visible as its still in the file system and the knownfile object doesn't gets deleted neither
			// so to avoid having to reload the whole list we just update it instead of removing and refinding
			UpdateFile(file);
			ShowFilesCount();
		}
		else
		{
			DeleteItem(iItem);
			ShowFilesCount();
		}
	}
}

//Xman advanced upload-priority
/*
void CSharedFilesCtrl::UpdateFile(const CShareableFile* file, bool bUpdateFileSummary)
*/
void CSharedFilesCtrl::UpdateFile(const CShareableFile* file, bool bUpdateFileSummary, bool force)
//Xman end
{
	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	if (theApp.IsRunningAsService(SVC_LIST_OPT))
		return;
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle

	if (!file || !theApp.emuledlg->IsRunning())
		return;

	//MORPH START - SiRoB, Don't Refresh item if not needed
	if( theApp.emuledlg->activewnd != theApp.emuledlg->sharedfileswnd && !force) //Xman advanced upload-priority 
		return;
	//MORPH END   - SiRoB, Don't Refresh item if not needed

	int iItem = FindFile(file);
	if (iItem != -1)
	{
		Update(iItem);
		//Xman [MoNKi: -Downloaded History-]
		/*
		if (bUpdateFileSummary && GetItemState(iItem, LVIS_SELECTED))
		*/
		if (bUpdateFileSummary && GetItemState(iItem, LVIS_SELECTED) && IsWindowVisible())
		//Xman end
			theApp.emuledlg->sharedfileswnd->ShowSelectedFilesDetails();
	}
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
	//Xman [MoNKi: -Downloaded History-]
	if(theApp.emuledlg->sharedfileswnd->historylistctrl.IsWindowVisible())
	{
		theApp.emuledlg->sharedfileswnd->historylistctrl.Reload();
		return;
	}
	//Xman end
	DeleteAllItems();
	theApp.emuledlg->sharedfileswnd->ShowSelectedFilesDetails();
	
	CCKey bufKey;
	CKnownFile* cur_file;
	for (POSITION pos = theApp.sharedfiles->m_Files_map.GetStartPosition(); pos != 0; ){
		theApp.sharedfiles->m_Files_map.GetNextAssoc(pos, bufKey, cur_file);
		AddFile(cur_file);
	}
	if (m_pDirectoryFilter != NULL && m_pDirectoryFilter->m_eItemType == SDI_UNSHAREDDIRECTORY && !m_pDirectoryFilter->m_strFullPath.IsEmpty()){
		AddShareableFiles(m_pDirectoryFilter->m_strFullPath);
	}
	else {
		while (!liTempShareableFilesInDir.IsEmpty())	// cleanup temp filelist
			delete liTempShareableFilesInDir.RemoveHead();
	}
	ShowFilesCount();
}

void CSharedFilesCtrl::ShowFilesCount()
{
	//Xman [MoNKi: -Downloaded History-]
	/*
	CString str;
	if (theApp.sharedfiles->GetHashingCount() + nAICHHashing)
		str.Format(_T(" (%i, %s %i)"), theApp.sharedfiles->GetCount(), GetResString(IDS_HASHING), theApp.sharedfiles->GetHashingCount() + nAICHHashing);
	else
		str.Format(_T(" (%i)"), theApp.sharedfiles->GetCount());
	theApp.emuledlg->sharedfileswnd->GetDlgItem(IDC_TRAFFIC_TEXT)->SetWindowText(GetResString(IDS_SF_FILES) + str);
	*/
	//if(theApp.emuledlg->sharedfileswnd->sharedfilesctrl.IsWindowVisible())
	if(!theApp.emuledlg->sharedfileswnd->historylistctrl.IsWindowVisible()) //Xman Code Improvement for ShowFilesCount
	{	
		CString str;
		if (theApp.sharedfiles->GetHashingCount() + nAICHHashing)
			str.Format(_T(" (%i, %s %i)"), theApp.sharedfiles->GetCount(), GetResString(IDS_HASHING), theApp.sharedfiles->GetHashingCount() + nAICHHashing);
		else
			str.Format(_T(" (%i)"), theApp.sharedfiles->GetCount());
		theApp.emuledlg->sharedfileswnd->GetDlgItem(IDC_TRAFFIC_TEXT)->SetWindowText(GetResString(IDS_SF_FILES) + str);
	}
	//Xman End
}

void CSharedFilesCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
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
	//zz_fly :: we init it ourself :: from DolphinX :: start
	/*
	BOOL bCtrlFocused;
	InitItemMemDC(dc, lpDrawItemStruct, bCtrlFocused);
	*/
	//zz_fly :: end
	// ==> Design Settings [eWombat/Stulle] - Stulle
	BOOL bCtrlFocused;
	InitItemMemDC(dc, lpDrawItemStruct, bCtrlFocused, false, style_b_sharedlist);
	// <== Design Settings [eWombat/Stulle] - Stulle
	CRect cur_rec(lpDrawItemStruct->rcItem);
	CRect rcClient;
	GetClientRect(&rcClient);

	/*const*/ CShareableFile* file = (CShareableFile*)lpDrawItemStruct->itemData;
	CKnownFile* pKnownFile = NULL;
	if (file->IsKindOf(RUNTIME_CLASS(CKnownFile)))
		pKnownFile = (CKnownFile*)file;

	// ==> Design Settings [eWombat/Stulle] - Stulle
	/*
	//zz_fly :: we init it ourself :: from DolphinX :: start
	BOOL bCtrlFocused = ((GetFocus() == this) || (GetStyle() & LVS_SHOWSELALWAYS));
	dc.FillBackground((lpDrawItemStruct->itemState & ODS_SELECTED)?
		((bCtrlFocused)?
			m_crHighlight
		:
			m_crNoHighlight)
	:
		//Xman PowerRelease
		((pKnownFile && pKnownFile->GetUpPriority()==PR_POWER)?
			RGB(255,210,210)
		:
		//Xman end
			m_crWindow));
	dc.SetTextColor((lpDrawItemStruct->itemState & ODS_SELECTED) ? m_crHighlightText : m_crWindowText);
	dc.SetFont(GetFont());
	//zz_fly :: end
	*/
	// <== Design Settings [eWombat/Stulle] - Stulle

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
					case 0: {
						int iCheckboxDrawWidth = 0;
						if (CheckBoxesEnabled())
						{
							int iState = (file == m_pHighlightedItem) ? CBS_UNCHECKEDHOT : CBS_UNCHECKEDNORMAL;
							int iNoStyleState = (file == m_pHighlightedItem) ? DFCS_PUSHED : 0;
							// no interacting with shell linked files or default shared directories
							if ((file->IsShellLinked() && theApp.sharedfiles->ShouldBeShared(file->GetSharedDirectory(), file->GetFilePath(), false)) 
								|| (theApp.sharedfiles->ShouldBeShared(file->GetSharedDirectory(), file->GetFilePath(), true)))
							{
								iState = CBS_CHECKEDDISABLED;
								iNoStyleState = DFCS_CHECKED | DFCS_INACTIVE;
							}
							else if (theApp.sharedfiles->ShouldBeShared(file->GetSharedDirectory(), file->GetFilePath(), false))
							{
								iState = (file == m_pHighlightedItem) ? CBS_CHECKEDHOT : CBS_CHECKEDNORMAL;
								iNoStyleState = (file == m_pHighlightedItem) ? (DFCS_PUSHED | DFCS_CHECKED) : DFCS_CHECKED;
							}
							// SLUGFILLER: SafeHash remove - removed installation dir unsharing
							/*
							else if (!thePrefs.IsShareableDirectory(file->GetPath()))
							{
								iState = CBS_DISABLED;
								iNoStyleState = DFCS_INACTIVE;
							}
							*/
							// SLUGFILLER: SafeHash remove - removed installation dir unsharing							

							HTHEME hTheme = (g_xpStyle.IsThemeActive() && g_xpStyle.IsAppThemed()) ? g_xpStyle.OpenThemeData(NULL, L"BUTTON") : NULL;

							CRect recCheckBox = cur_rec;
							recCheckBox.right = recCheckBox.left + 16;
							recCheckBox.top += (cur_rec.Height() > 16) ? ((cur_rec.Height() - 16) / 2) : 0;
							recCheckBox.bottom = recCheckBox.top + 16;
							if (hTheme != NULL)
								g_xpStyle.DrawThemeBackground(hTheme, dc.GetSafeHdc(), BP_CHECKBOX, iState, &recCheckBox, NULL);
							else
								dc.DrawFrameControl(&recCheckBox, DFC_BUTTON, DFCS_BUTTONCHECK | iNoStyleState | DFCS_FLAT);
							cur_rec.left += 2 + 16;
							iCheckboxDrawWidth += 2 + 16;
						}

						int iIconPosY = (cur_rec.Height() > theApp.GetSmallSytemIconSize().cy) ? ((cur_rec.Height() - theApp.GetSmallSytemIconSize().cy) / 2) : 0;
						int iImage = theApp.GetFileTypeSystemImageIdx(file->GetFileName());
						if (theApp.GetSystemImageList() != NULL)
							::ImageList_Draw(theApp.GetSystemImageList(), iImage, dc.GetSafeHdc(), cur_rec.left, cur_rec.top + iIconPosY, ILD_TRANSPARENT);
						if (!file->GetFileComment().IsEmpty() || file->GetFileRating())
							m_ImageList.Draw(dc, 0, CPoint(cur_rec.left, cur_rec.top + iIconPosY), ILD_NORMAL | INDEXTOOVERLAYMASK(1));
						cur_rec.left += iIconDrawWidth;

						if (thePrefs.ShowRatingIndicator() && (file->HasComment() || file->HasRating() || file->IsKadCommentSearchRunning()))
						{
							m_ImageList.Draw(dc, file->UserRating(true) + 3, CPoint(cur_rec.left + 2, cur_rec.top + iIconPosY), ILD_NORMAL);
							cur_rec.left += 2 + 16;
							iIconDrawWidth += 2 + 16;
						}
						cur_rec.left += sm_iLabelOffset;
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
						cur_rec.left -= iIconDrawWidth + iCheckboxDrawWidth;
						cur_rec.right -= sm_iSubItemInset;
						break;
					}
					
					case 8:
						if (pKnownFile != NULL && pKnownFile->GetPartCount()) {
							cur_rec.bottom--;
							cur_rec.top++;
							COLORREF crOldBackColor = dc->GetBkColor(); //Xman Code Improvement: FillSolidRect
							pKnownFile->DrawShareStatusBar(dc, &cur_rec, false, thePrefs.UseFlatBar());
							dc.SetBkColor(crOldBackColor); //Xman Code Improvement: FillSolidRect
							cur_rec.bottom++;
							cur_rec.top--;
						}
						break;

					case 11:
						if (pKnownFile == NULL)
							break;
						if (pKnownFile->GetPublishedED2K())
							m_ImageList.Draw(dc, 1, cur_rec.TopLeft(), ILD_NORMAL);
						if (IsSharedInKad(pKnownFile))
						{
							cur_rec.left += 16;
							m_ImageList.Draw(dc, IsSharedInKad(pKnownFile) ? 2 : 0, cur_rec.TopLeft(), ILD_NORMAL);
							cur_rec.left -= 16;
						}
						break;

					// ==> Spread bars [Slugfiller/MorphXT] - Stulle
					case 20:
						if(pKnownFile == NULL)
							break;
						cur_rec.bottom--;
						cur_rec.top++;
						pKnownFile->statistic.DrawSpreadBar(dc,&cur_rec,thePrefs.UseFlatBar());
						cur_rec.bottom++;
						cur_rec.top--;
						break;
					// <== Spread bars [Slugfiller/MorphXT] - Stulle

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

void CSharedFilesCtrl::GetItemDisplayText(const CShareableFile* file, int iSubItem, LPTSTR pszText, int cchTextMax) const
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
			_tcsncpy(pszText, file->GetPath(), cchTextMax);
			pszText[cchTextMax - 1] = _T('\0');
			PathRemoveBackslash(pszText);
			break;
	}

	if (file->IsKindOf(RUNTIME_CLASS(CKnownFile))){
		CKnownFile* pKnownFile = (CKnownFile*)file;
		switch (iSubItem)
		{			
			case 3:
				// ==> PowerShare [ZZ/MorphXT] - Stulle
				if(pKnownFile->GetPowerShared())
					_sntprintf(pszText, cchTextMax, _T("%s %s"), GetResString(IDS_POWERSHARE_PREFIX), pKnownFile->GetUpPriorityDisplayString());
				else
				// <== PowerShare [ZZ/MorphXT] - Stulle
				_tcsncpy(pszText, pKnownFile->GetUpPriorityDisplayString(), cchTextMax);
				break;
			
			case 4:
				_tcsncpy(pszText, md4str(pKnownFile->GetFileHash()), cchTextMax);
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
			
			case 8:
				_sntprintf(pszText, cchTextMax, _T("%s: %u"), GetResString(IDS_SHARED_STATUS), pKnownFile->GetPartCount());
				break;
			
			case 10:
				//Xman show virtual sources (morph) + virtualUploadsources
				/*
				if (pKnownFile->m_nCompleteSourcesCountLo == pKnownFile->m_nCompleteSourcesCountHi)
					_sntprintf(pszText, cchTextMax, _T("%u"), pKnownFile->m_nCompleteSourcesCountLo);
				else if (pKnownFile->m_nCompleteSourcesCountLo == 0)
					_sntprintf(pszText, cchTextMax, _T("< %u"), pKnownFile->m_nCompleteSourcesCountHi);
				else
					_sntprintf(pszText, cchTextMax, _T("%u - %u"), pKnownFile->m_nCompleteSourcesCountLo, pKnownFile->m_nCompleteSourcesCountHi);
				*/
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
				_sntprintf(pszText, cchTextMax, _T("%s|%s"), GetResString(pKnownFile->GetPublishedED2K() ? IDS_YES : IDS_NO), GetResString(IsSharedInKad(pKnownFile) ? IDS_YES : IDS_NO));
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
				uint32 nMediaLength = pKnownFile->GetIntTagValue(FT_MEDIA_LENGTH);
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
				_sntprintf(pszText, cchTextMax, _T("%u"),pKnownFile->GetOnUploadqueue());
				break;
			//Xman end

			//Xman advanced upload-priority
			case 19:
			{
				if(thePrefs.UseAdvancedAutoPtio())
					_sntprintf(pszText, cchTextMax, _T("%.0f%% / %.0f%%"), pKnownFile->CalculateUploadPriorityPercent(), pKnownFile->statistic.GetAllTimeTransferred()/(float)pKnownFile->GetFileSize()*100);
				else
					_sntprintf(pszText, cchTextMax, _T("%.0f%%"), pKnownFile->statistic.GetAllTimeTransferred()/(float)pKnownFile->GetFileSize()*100);
				break;
			}
			//Xman end
			// ==> Spread bars [Slugfiller/MorphXT] - Stulle
			case 20:
				_tcsncpy(pszText, _T("Spreadhars"), cchTextMax);
				break;
			case 21:
				_sntprintf(pszText, cchTextMax, _T("%.2f"),pKnownFile->statistic.GetSpreadSortValue());
				break;
			case 22:
				if (pKnownFile->GetFileSize()>(uint64)0)
					_sntprintf(pszText, cchTextMax, _T("%.2f"),((double)pKnownFile->statistic.GetAllTimeTransferred())/((double)pKnownFile->GetFileSize()));
				else
					_sntprintf(pszText, cchTextMax, _T("%.2f"),0.0f);
				break;
			case 23:
				_sntprintf(pszText, cchTextMax, _T("%.2f"),pKnownFile->statistic.GetFullSpreadCount());
				break;
			// <== Spread bars [Slugfiller/MorphXT] - Stulle
			// ==> push rare file - Stulle
			case 24:
				_sntprintf(pszText, cchTextMax, _T("%.1f"),pKnownFile->GetFileRatio());
				break;
			// <== push rare file - Stulle
			// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
			case 25:
			{
				CString buffer;
				UINT hideOSInWork = pKnownFile->HideOSInWork();
				buffer = _T("[") + GetResString((hideOSInWork>0)?IDS_POWERSHARE_ON_LABEL:IDS_POWERSHARE_OFF_LABEL) + _T("] ");
				if(pKnownFile->GetHideOS()<0)
					buffer.Append(_T(" ") + ((CString)GetResString(IDS_DEFAULT)).Left(1) + _T(". "));
				hideOSInWork = (pKnownFile->GetHideOS()>=0)?pKnownFile->GetHideOS():thePrefs.GetHideOvershares();
				if (hideOSInWork>0)
					buffer.AppendFormat(_T("%i"), hideOSInWork);
				else
					buffer.AppendFormat(_T("%s"), GetResString(IDS_DISABLED));
				if (pKnownFile->GetSelectiveChunk()>=0){
					if (pKnownFile->GetSelectiveChunk())
						buffer.Append(_T(" + S"));
				}else
					if (thePrefs.IsSelectiveShareEnabled())
						buffer.Append(_T(" + ") + ((CString)GetResString(IDS_DEFAULT)).Left(1) + _T(". S"));
				_tcsncpy(pszText, buffer, cchTextMax);
				break;
			}
			case 26:
				if(pKnownFile->GetShareOnlyTheNeed()>=0) {
					if (pKnownFile->GetShareOnlyTheNeed())
						_sntprintf(pszText, cchTextMax, _T("%i"), pKnownFile->GetShareOnlyTheNeed());
					else
						_tcsncpy(pszText, GetResString(IDS_DISABLED), cchTextMax);
				} else
					_sntprintf(pszText, cchTextMax, _T("%s. %s"), ((CString)GetResString(IDS_DEFAULT)).Left(1), GetResString((thePrefs.GetShareOnlyTheNeed()>0)?IDS_ENABLED:IDS_DISABLED));
				break;
			// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
			// ==> PowerShare [ZZ/MorphXT] - Stulle
			case 27:
			{
				CString buffer;
				int powersharemode;
				bool powershared = pKnownFile->GetPowerShared();
				buffer = _T("[") + GetResString((powershared)?IDS_POWERSHARE_ON_LABEL:IDS_POWERSHARE_OFF_LABEL) + _T("] ");
				if (pKnownFile->GetPowerSharedMode()>=0)
					powersharemode = pKnownFile->GetPowerSharedMode();
				else {
					powersharemode = thePrefs.GetPowerShareMode();
					buffer.Append(_T(" ") + ((CString)GetResString(IDS_DEFAULT)).Left(1) + _T(". "));
				} //
				if(powersharemode == 2)
					buffer.Append(GetResString(IDS_POWERSHARE_AUTO_LABEL));
				else if (powersharemode == 1)
					buffer.Append(GetResString(IDS_POWERSHARE_ACTIVATED_LABEL));
				else if (powersharemode == 3) {
					buffer.Append(GetResString(IDS_POWERSHARE_LIMITED));
				if (pKnownFile->GetPowerShareLimit()<0)
						buffer.AppendFormat(_T(" %s. %i"), ((CString)GetResString(IDS_DEFAULT)).Left(1), thePrefs.GetPowerShareLimit());
					else
						buffer.AppendFormat(_T(" %i"), pKnownFile->GetPowerShareLimit());
					// ==> Limit PS by amount of data uploaded - Stulle
					if (pKnownFile->GetPsAmountLimit()<0)
						buffer.AppendFormat(_T(" %s. %i%%"), ((CString)GetResString(IDS_DEFAULT)).Left(1), thePrefs.GetPsAmountLimit());
					else
						buffer.AppendFormat(_T(" %i%%"), pKnownFile->GetPsAmountLimit());
					// <== Limit PS by amount of data uploaded - Stulle
				}
				else
					buffer.Append(GetResString(IDS_POWERSHARE_DISABLED_LABEL));
				buffer.Append(_T(" ("));
				if (pKnownFile->GetPowerShareAuto())
					buffer.Append(GetResString(IDS_POWERSHARE_ADVISED_LABEL));
				else if (pKnownFile->GetPowerShareLimited() && (powersharemode == 3))
					buffer.Append(GetResString(IDS_POWERSHARE_LIMITED));
				else if (pKnownFile->GetPowerShareAuthorized())
					buffer.Append(GetResString(IDS_POWERSHARE_AUTHORIZED_LABEL));
				else
					buffer.Append(GetResString(IDS_POWERSHARE_DENIED_LABEL));
				buffer.Append(_T(")"));
				_tcsncpy(pszText, buffer, cchTextMax);
			}
			break;
			// <== PowerShare [ZZ/MorphXT] - Stulle
		}
	}

	pszText[cchTextMax - 1] = _T('\0');
}

void CSharedFilesCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	// get merged settings
	bool bFirstItem = true;
	bool bContainsShareableFiles = false;
	bool bContainsOnlyShareableFile = true;
	bool bContainsUnshareableFile = false;
	int iSelectedItems = GetSelectedCount();
	int iCompleteFileSelected = -1;
	UINT uPrioMenuItem = 0;

	// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	int iHideOS = -1;
	UINT uHideOSMenuItem = 0;
	UINT uSelectiveChunkMenuItem = 0;
	UINT uShareOnlyTheNeedMenuItem = 0;
	CString buffer;
	// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	// ==> PowerShare [ZZ/MorphXT] - Stulle
	int iPowerShareLimit = -1;
	UINT uPowershareMenuItem = 0;
	UINT uPowerShareLimitMenuItem = 0;
	// <== PowerShare [ZZ/MorphXT] - Stulle
	// ==> Limit PS by amount of data uploaded [Stulle] - Stulle
	int iPsAmountLimit = -1;
	UINT uPsAmountLimitMenuItem = 0;
	// <== Limit PS by amount of data uploaded [Stulle] - Stulle

	const CShareableFile* pSingleSelFile = NULL;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos)
	{
		const CShareableFile* pFile = (CShareableFile*)GetItemData(GetNextSelectedItem(pos));
		if (bFirstItem)
			pSingleSelFile = pFile;
		else
			pSingleSelFile = NULL;

		int iCurCompleteFile = pFile->IsPartFile() ? 0 : 1;
		if (bFirstItem)
			iCompleteFileSelected = iCurCompleteFile;
		else if (iCompleteFileSelected != iCurCompleteFile)
			iCompleteFileSelected = -1;

		bContainsUnshareableFile = !pFile->IsShellLinked() && !pFile->IsPartFile() && (bContainsUnshareableFile || (theApp.sharedfiles->ShouldBeShared(pFile->GetSharedDirectory(), pFile->GetFilePath(), false)
			&& !theApp.sharedfiles->ShouldBeShared(pFile->GetSharedDirectory(), pFile->GetFilePath(), true)));

		if (pFile->IsKindOf(RUNTIME_CLASS(CKnownFile)))
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
			else if (((CKnownFile*)pFile)->GetUpPriority() == PR_POWER)
				uCurPrioMenuItem = MP_PRIOPOWER;
			//Xman end
			else
				ASSERT(0);

			if (bFirstItem)
				uPrioMenuItem = uCurPrioMenuItem;
			else if (uPrioMenuItem != uCurPrioMenuItem)
				uPrioMenuItem = 0;

			// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
			UINT uCurHideOSMenuItem = 0;
			int iCurHideOS = ((CKnownFile*)pFile)->GetHideOS();
			if (iCurHideOS == -1)
				uCurHideOSMenuItem = MP_HIDEOS_DEFAULT;
			else
				uCurHideOSMenuItem = MP_HIDEOS_SET;
			if (bFirstItem)
			{
				uHideOSMenuItem = uCurHideOSMenuItem;
				iHideOS = iCurHideOS;
			}
			else if (uHideOSMenuItem != uCurHideOSMenuItem || iHideOS != iCurHideOS)
			{
				uHideOSMenuItem = 0;
				iHideOS = -1;
			}
		
			UINT uCurSelectiveChunkMenuItem = 0;
			if (((CKnownFile*)pFile)->GetSelectiveChunk() == -1)
				uCurSelectiveChunkMenuItem = MP_SELECTIVE_CHUNK;
			else
				uCurSelectiveChunkMenuItem = MP_SELECTIVE_CHUNK+1 + ((CKnownFile*)pFile)->GetSelectiveChunk();
			if (bFirstItem)
				uSelectiveChunkMenuItem = uCurSelectiveChunkMenuItem;
			else if (uSelectiveChunkMenuItem != uCurSelectiveChunkMenuItem)
				uSelectiveChunkMenuItem = 0;

			UINT uCurShareOnlyTheNeedMenuItem = 0;
			if (((CKnownFile*)pFile)->GetShareOnlyTheNeed() == -1)
				uCurShareOnlyTheNeedMenuItem = MP_SHAREONLYTHENEED;
			else
				uCurShareOnlyTheNeedMenuItem = MP_SHAREONLYTHENEED+1 + ((CKnownFile*)pFile)->GetShareOnlyTheNeed();
			if (bFirstItem)
				uShareOnlyTheNeedMenuItem = uCurShareOnlyTheNeedMenuItem ;
			else if (uShareOnlyTheNeedMenuItem != uCurShareOnlyTheNeedMenuItem)
				uShareOnlyTheNeedMenuItem = 0;
			// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle

			// ==> PowerShare [ZZ/MorphXT] - Stulle
			UINT uCurPowershareMenuItem = 0;
			if (((CKnownFile*)pFile)->GetPowerSharedMode()==-1)
				uCurPowershareMenuItem = MP_POWERSHARE_DEFAULT;
			else
				uCurPowershareMenuItem = MP_POWERSHARE_DEFAULT+1 + ((CKnownFile*)pFile)->GetPowerSharedMode();
		
			if (bFirstItem)
				uPowershareMenuItem = uCurPowershareMenuItem;
			else if (uPowershareMenuItem != uCurPowershareMenuItem)
				uPowershareMenuItem = 0;

			UINT uCurPowerShareLimitMenuItem = 0;
			int iCurPowerShareLimit = ((CKnownFile*)pFile)->GetPowerShareLimit();
			if (iCurPowerShareLimit==-1)
				uCurPowerShareLimitMenuItem = MP_POWERSHARE_LIMIT;
			else
				uCurPowerShareLimitMenuItem = MP_POWERSHARE_LIMIT_SET;
		
			if (bFirstItem)
			{
				uPowerShareLimitMenuItem = uCurPowerShareLimitMenuItem;
				iPowerShareLimit = iCurPowerShareLimit;
			}
			else if (uPowerShareLimitMenuItem != uCurPowerShareLimitMenuItem || iPowerShareLimit != iCurPowerShareLimit)
			{
				uPowerShareLimitMenuItem = 0;
				iPowerShareLimit = -1;
			}
			// <== PowerShare [ZZ/MorphXT] - Stulle

			// ==> Limit PS by amount of data uploaded [Stulle] - Stulle
			UINT uCurPsAmountLimitMenuItem = 0;
			int iCurPsAmountLimit = ((CKnownFile*)pFile)->GetPsAmountLimit();
			if (iCurPsAmountLimit==-1)
				uCurPsAmountLimitMenuItem = MP_PS_AMOUNT_LIMIT;
			else
				uCurPsAmountLimitMenuItem = MP_PS_AMOUNT_LIMIT_SET;

			if (bFirstItem)
			{
				uPsAmountLimitMenuItem = uCurPsAmountLimitMenuItem;
				iPsAmountLimit = iCurPsAmountLimit;
			}
			else if (uPsAmountLimitMenuItem != uCurPsAmountLimitMenuItem || iPsAmountLimit != iCurPsAmountLimit)
			{
				uPsAmountLimitMenuItem = 0;
				iPsAmountLimit = -1;
			}
			// <== Limit PS by amount of data uploaded [Stulle] - Stulle
		}
		else
			bContainsShareableFiles = true;

		bFirstItem = false;
	}

	m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_PrioMenu.m_hMenu, (!bContainsShareableFiles && iSelectedItems > 0) ? MF_ENABLED : MF_GRAYED);
	//Xman PowerRelease 
	/*
	m_PrioMenu.CheckMenuRadioItem(MP_PRIOVERYLOW, MP_PRIOAUTO, uPrioMenuItem, 0);
	*/
	m_PrioMenu.CheckMenuRadioItem(MP_PRIOVERYLOW, MP_PRIOPOWER, uPrioMenuItem, 0);
	//Xman end

	bool bSingleCompleteFileSelected = (iSelectedItems == 1 && (iCompleteFileSelected == 1 || bContainsOnlyShareableFile));
	m_SharedFilesMenu.EnableMenuItem(MP_OPEN, bSingleCompleteFileSelected ? MF_ENABLED : MF_GRAYED);
	UINT uInsertedMenuItem = 0;
	static const TCHAR _szSkinPkgSuffix1[] = _T(".") EMULSKIN_BASEEXT _T(".zip");
	static const TCHAR _szSkinPkgSuffix2[] = _T(".") EMULSKIN_BASEEXT _T(".rar");
	if (bSingleCompleteFileSelected 
		&& pSingleSelFile 
		&& (   pSingleSelFile->GetFilePath().Right(ARRSIZE(_szSkinPkgSuffix1)-1).CompareNoCase(_szSkinPkgSuffix1) == 0
		    || pSingleSelFile->GetFilePath().Right(ARRSIZE(_szSkinPkgSuffix2)-1).CompareNoCase(_szSkinPkgSuffix2) == 0))
	{
		MENUITEMINFO mii = {0};
		mii.cbSize = sizeof mii;
		mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
		mii.fType = MFT_STRING;
		mii.fState = MFS_ENABLED;
		mii.wID = MP_INSTALL_SKIN;
		CString strBuff(GetResString(IDS_INSTALL_SKIN));
		mii.dwTypeData = const_cast<LPTSTR>((LPCTSTR)strBuff);
		if (::InsertMenuItem(m_SharedFilesMenu, MP_OPENFOLDER, FALSE, &mii))
			uInsertedMenuItem = mii.wID;
	}
	//Xman PowerRelease
	m_SharedFilesMenu.EnableMenuItem(MP_PRIOPOWER, (!bContainsShareableFiles && iCompleteFileSelected > 0) ? MF_ENABLED : MF_GRAYED);
	//Xman end
	m_SharedFilesMenu.EnableMenuItem(MP_OPENFOLDER, bSingleCompleteFileSelected ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(MP_RENAME, (!bContainsShareableFiles && bSingleCompleteFileSelected) ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(MP_REMOVE, iCompleteFileSelected > 0 ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(MP_UNSHAREFILE, bContainsUnshareableFile ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.SetDefaultItem(bSingleCompleteFileSelected ? MP_OPEN : -1);
	m_SharedFilesMenu.EnableMenuItem(MP_CMT, (!bContainsShareableFiles && iSelectedItems > 0) ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(MP_DETAIL, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(thePrefs.GetShowCopyEd2kLinkCmd() ? MP_GETED2KLINK : MP_SHOWED2KLINK, (!bContainsOnlyShareableFile && iSelectedItems > 0) ? MF_ENABLED : MF_GRAYED);

	// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_HideOSMenu.m_hMenu, (iSelectedItems > 0 && iCompleteFileSelected > 0) ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_SelectiveChunkMenu.m_hMenu, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	if (thePrefs.GetHideOvershares()==0)
		buffer.Format(_T(" (%s)"),GetResString(IDS_DISABLED));
	else
		buffer.Format(_T(" (%u)"),thePrefs.GetHideOvershares());
	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	m_HideOSMenu.ModifyMenu(MP_HIDEOS_DEFAULT, MF_STRING,MP_HIDEOS_DEFAULT, GetResString(IDS_DEFAULT) + buffer);
	*/
	m_HideOSMenu.RemoveMenu(MP_HIDEOS_DEFAULT,MF_BYCOMMAND);
	m_HideOSMenu.InsertMenu(1,MF_STRING|MF_BYPOSITION,MP_HIDEOS_DEFAULT,GetResString(IDS_DEFAULT) + buffer);
	// <== XP Style Menu [Xanatos] - Stulle
	if (iHideOS==-1)
		buffer = GetResString(IDS_EDIT);
	else if (iHideOS==0)
		buffer = GetResString(IDS_DISABLED);
	else
		buffer.Format(_T("%i"), iHideOS);
	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	m_HideOSMenu.ModifyMenu(MP_HIDEOS_SET, MF_STRING,MP_HIDEOS_SET, buffer);
	*/
	m_HideOSMenu.RemoveMenu(MP_HIDEOS_SET,MF_BYCOMMAND);
	m_HideOSMenu.InsertMenu(2,MF_STRING|MF_BYPOSITION,MP_HIDEOS_SET,buffer);
	// <== XP Style Menu [Xanatos] - Stulle
	m_HideOSMenu.CheckMenuRadioItem(MP_HIDEOS_DEFAULT, MP_HIDEOS_SET, uHideOSMenuItem, 0);
	buffer.Format(_T(" (%s)"),thePrefs.IsSelectiveShareEnabled()?GetResString(IDS_ENABLED):GetResString(IDS_DISABLED));
	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	m_SelectiveChunkMenu.ModifyMenu(MP_SELECTIVE_CHUNK, MF_STRING, MP_SELECTIVE_CHUNK, GetResString(IDS_DEFAULT) + buffer);
	*/
	m_SelectiveChunkMenu.RemoveMenu(MP_SELECTIVE_CHUNK,MF_BYCOMMAND);
	m_SelectiveChunkMenu.InsertMenu(1,MF_STRING|MF_BYPOSITION,MP_SELECTIVE_CHUNK,GetResString(IDS_DEFAULT) + buffer);
	// <== XP Style Menu [Xanatos] - Stulle
	m_SelectiveChunkMenu.CheckMenuRadioItem(MP_SELECTIVE_CHUNK, MP_SELECTIVE_CHUNK_1, uSelectiveChunkMenuItem, 0);

	m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_ShareOnlyTheNeedMenu.m_hMenu, (iSelectedItems > 0 && iCompleteFileSelected > 0) ? MF_ENABLED : MF_GRAYED);
	buffer.Format(_T(" (%s)"),thePrefs.GetShareOnlyTheNeed()?GetResString(IDS_ENABLED):GetResString(IDS_DISABLED));
	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	m_ShareOnlyTheNeedMenu.ModifyMenu(MP_SHAREONLYTHENEED, MF_STRING, MP_SHAREONLYTHENEED, GetResString(IDS_DEFAULT) + buffer);
	*/
	m_ShareOnlyTheNeedMenu.RemoveMenu(MP_SHAREONLYTHENEED,MF_BYCOMMAND);
	m_ShareOnlyTheNeedMenu.InsertMenu(1,MF_STRING|MF_BYPOSITION,MP_SHAREONLYTHENEED,GetResString(IDS_DEFAULT) + buffer);
	// <== XP Style Menu [Xanatos] - Stulle
	m_ShareOnlyTheNeedMenu.CheckMenuRadioItem(MP_SHAREONLYTHENEED, MP_SHAREONLYTHENEED_1, uShareOnlyTheNeedMenuItem, 0);
	// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	m_SharedFilesMenu.EnableMenuItem(MP_SPREADBAR_RESET, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED); // Spread bars [Slugfiller/MorphXT] - Stulle
	// ==> PowerShare [ZZ/MorphXT] - Stulle
	m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_PowershareMenu.m_hMenu, (iSelectedItems > 0 && iCompleteFileSelected > 0) ? MF_ENABLED : MF_GRAYED);
	switch (thePrefs.GetPowerShareMode()){
		case 0:
			buffer.Format(_T(" (%s)"),GetResString(IDS_POWERSHARE_DISABLED));
			break;
		case 1:
			buffer.Format(_T(" (%s)"),GetResString(IDS_POWERSHARE_ACTIVATED));
			break;
		case 2:
			buffer.Format(_T(" (%s)"),GetResString(IDS_POWERSHARE_AUTO));
			break;
		case 3:
			buffer.Format(_T(" (%s)"),GetResString(IDS_POWERSHARE_LIMITED));
			break;
		default:
			buffer = _T(" (?)");
			break;
	}
	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	m_PowershareMenu.ModifyMenu(MP_POWERSHARE_DEFAULT, MF_STRING,MP_POWERSHARE_DEFAULT, GetResString(IDS_DEFAULT) + buffer);
	*/
	m_PowershareMenu.RemoveMenu(MP_POWERSHARE_DEFAULT,MF_BYCOMMAND);
	m_PowershareMenu.InsertMenu(1,MF_STRING|MF_BYPOSITION,MP_POWERSHARE_DEFAULT,GetResString(IDS_DEFAULT) + buffer);
	// <== XP Style Menu [Xanatos] - Stulle
	m_PowershareMenu.CheckMenuRadioItem(MP_POWERSHARE_DEFAULT, MP_POWERSHARE_LIMITED, uPowershareMenuItem, 0);
	m_PowershareMenu.EnableMenuItem((UINT_PTR)m_PowerShareLimitMenu.m_hMenu, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	if (thePrefs.GetPowerShareLimit()==0)
		buffer.Format(_T(" (%s)"),GetResString(IDS_DISABLED));
	else
		buffer.Format(_T(" (%u)"),thePrefs.GetPowerShareLimit());
	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	m_PowerShareLimitMenu.ModifyMenu(MP_POWERSHARE_LIMIT, MF_STRING,MP_POWERSHARE_LIMIT, GetResString(IDS_DEFAULT) + buffer);
	*/
	m_PowerShareLimitMenu.RemoveMenu(MP_POWERSHARE_LIMIT,MF_BYCOMMAND);
	m_PowerShareLimitMenu.InsertMenu(1,MF_STRING|MF_BYPOSITION,MP_POWERSHARE_LIMIT,GetResString(IDS_DEFAULT) + buffer);
	// <== XP Style Menu [Xanatos] - Stulle
	if (iPowerShareLimit==-1)
		buffer = GetResString(IDS_EDIT);
	else if (iPowerShareLimit==0)
		buffer = GetResString(IDS_DISABLED);
	else
		buffer.Format(_T("%i"),iPowerShareLimit);
	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	m_PowerShareLimitMenu.ModifyMenu(MP_POWERSHARE_LIMIT_SET, MF_STRING,MP_POWERSHARE_LIMIT_SET, buffer);
	*/
	m_PowerShareLimitMenu.RemoveMenu(MP_POWERSHARE_LIMIT_SET,MF_BYCOMMAND);
	m_PowerShareLimitMenu.InsertMenu(2,MF_STRING|MF_BYPOSITION,MP_POWERSHARE_LIMIT_SET,buffer);
	// <== XP Style Menu [Xanatos] - Stulle
	m_PowerShareLimitMenu.CheckMenuRadioItem(MP_POWERSHARE_LIMIT, MP_POWERSHARE_LIMIT_SET, uPowerShareLimitMenuItem, 0);
	// <== PowerShare [ZZ/MorphXT] - Stulle
	// ==> Limit PS by amount of data uploaded [Stulle] - Stulle
	m_PowershareMenu.EnableMenuItem((UINT_PTR)m_PsAmountLimitMenu.m_hMenu, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	if (iPsAmountLimit==0)
		buffer.Format(_T(" (%s)"),GetResString(IDS_DISABLED));
	else
		buffer.Format(_T(" (%i%%)"),thePrefs.GetPsAmountLimit());
	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	m_PsAmountLimitMenu.ModifyMenu(MP_PS_AMOUNT_LIMIT, MF_STRING,MP_PS_AMOUNT_LIMIT, GetResString(IDS_DEFAULT) + buffer);
	*/
	m_PsAmountLimitMenu.RemoveMenu(MP_PS_AMOUNT_LIMIT,MF_BYCOMMAND);
	m_PsAmountLimitMenu.InsertMenu(1,MF_STRING|MF_BYPOSITION,MP_PS_AMOUNT_LIMIT,GetResString(IDS_DEFAULT) + buffer);
	// <== XP Style Menu [Xanatos] - Stulle
	if (iPsAmountLimit==-1)
		buffer = GetResString(IDS_EDIT);
	else if (iPsAmountLimit==0)
		buffer = GetResString(IDS_DISABLED);
	else
		buffer.Format(_T("%i%%"),iPsAmountLimit);
	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	m_PsAmountLimitMenu.ModifyMenu(MP_PS_AMOUNT_LIMIT_SET, MF_STRING,MP_PS_AMOUNT_LIMIT_SET, buffer);
	*/
	m_PsAmountLimitMenu.RemoveMenu(MP_PS_AMOUNT_LIMIT_SET,MF_BYCOMMAND);
	m_PsAmountLimitMenu.InsertMenu(2,MF_STRING|MF_BYPOSITION,MP_PS_AMOUNT_LIMIT_SET,buffer);
	// <== XP Style Menu [Xanatos] - Stulle
	m_PsAmountLimitMenu.CheckMenuRadioItem(MP_PS_AMOUNT_LIMIT, MP_PS_AMOUNT_LIMIT_SET, uPsAmountLimitMenuItem, 0);
	// <== Limit PS by amount of data uploaded [Stulle] - Stulle
	// ==> Copy feedback feature [MorphXT] - Stulle
	/*
	// Xman: IcEcRacKer Copy UL-feedback
	m_SharedFilesMenu.EnableMenuItem(MP_ULFEEDBACK, (!bContainsShareableFiles && iSelectedItems > 0) ? MF_ENABLED : MF_GRAYED);
	//Xman end
	*/
	m_SharedFilesMenu.EnableMenuItem(MP_COPYFEEDBACK, (!bContainsShareableFiles && iSelectedItems > 0) ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(MP_COPYFEEDBACK_US, (!bContainsShareableFiles && iSelectedItems > 0) ? MF_ENABLED : MF_GRAYED);
	// <== Copy feedback feature [MorphXT] - Stulle

	m_SharedFilesMenu.EnableMenuItem(MP_FIND, GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED);

	//Xman Mass Rename (Morph)
	m_SharedFilesMenu.EnableMenuItem(MP_MASSRENAME, (!bContainsShareableFiles && iSelectedItems > 0) ? MF_ENABLED : MF_GRAYED);
	//Xman end

	m_CollectionsMenu.EnableMenuItem(MP_MODIFYCOLLECTION, (!bContainsShareableFiles && pSingleSelFile != NULL && ((CKnownFile*)pSingleSelFile)->m_pCollection != NULL ) ? MF_ENABLED : MF_GRAYED);
	m_CollectionsMenu.EnableMenuItem(MP_VIEWCOLLECTION, (!bContainsShareableFiles && pSingleSelFile != NULL && ((CKnownFile*)pSingleSelFile)->m_pCollection != NULL ) ? MF_ENABLED : MF_GRAYED);
	m_CollectionsMenu.EnableMenuItem(MP_SEARCHAUTHOR, (!bContainsShareableFiles && pSingleSelFile != NULL && ((CKnownFile*)pSingleSelFile)->m_pCollection != NULL 
		&& !((CKnownFile*)pSingleSelFile)->m_pCollection->GetAuthorKeyHashString().IsEmpty()) ? MF_ENABLED : MF_GRAYED);
#if defined(_DEBUG)
	if (thePrefs.IsExtControlsEnabled()){
	//JOHNTODO: Not for release as we need kad lowID users in the network to see how well this work work. Also, we do not support these links yet.
		if (iSelectedItems > 0 && theApp.IsConnected() && theApp.IsFirewalled() && theApp.clientlist->GetBuddy())
			m_SharedFilesMenu.EnableMenuItem(MP_GETKADSOURCELINK, MF_ENABLED);
		else
			m_SharedFilesMenu.EnableMenuItem(MP_GETKADSOURCELINK, MF_GRAYED);
	}
#endif
	m_SharedFilesMenu.EnableMenuItem(Irc_SetSendLink, (!bContainsOnlyShareableFile && iSelectedItems == 1 && theApp.emuledlg->ircwnd->IsConnected()) ? MF_ENABLED : MF_GRAYED);

	CTitleMenu WebMenu;
	WebMenu.CreateMenu();
	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	WebMenu.AddMenuTitle(NULL, true);
	*/
	WebMenu.AddMenuTitle(GetResString(IDS_WEBSERVICES), true, false);
	// <== XP Style Menu [Xanatos] - Stulle
	int iWebMenuEntries = theWebServices.GetFileMenuEntries(&WebMenu);
	UINT flag2 = (iWebMenuEntries == 0 || iSelectedItems != 1) ? MF_GRAYED : MF_STRING;
	m_SharedFilesMenu.AppendMenu(flag2 | MF_POPUP, (UINT_PTR)WebMenu.m_hMenu, GetResString(IDS_WEBSERVICES), _T("WEB"));

	GetPopupMenuPos(*this, point);
	m_SharedFilesMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON,point.x,point.y,this);

	m_SharedFilesMenu.RemoveMenu(m_SharedFilesMenu.GetMenuItemCount()-1,MF_BYPOSITION);
	VERIFY( WebMenu.DestroyMenu() );
	if (uInsertedMenuItem)
		VERIFY( m_SharedFilesMenu.RemoveMenu(uInsertedMenuItem, MF_BYCOMMAND) );
}

BOOL CSharedFilesCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	wParam = LOWORD(wParam);

	CTypedPtrList<CPtrList, CShareableFile*> selectedList;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos != NULL){
		int index = GetNextSelectedItem(pos);
		if (index >= 0)
			selectedList.AddTail((CShareableFile*)GetItemData(index));
	}

	if (   wParam == MP_CREATECOLLECTION
		|| wParam == MP_FIND
		|| selectedList.GetCount() > 0)
	{
		CShareableFile* file = NULL;
		if (selectedList.GetCount() == 1)
			file = selectedList.GetHead();

		CKnownFile* pKnownFile = NULL;
		if (file != NULL && file->IsKindOf(RUNTIME_CLASS(CKnownFile)))
			pKnownFile = (CKnownFile*)file;

		switch (wParam){
			case Irc_SetSendLink:
				if (pKnownFile != NULL)
					theApp.emuledlg->ircwnd->SetSendFileString(pKnownFile->GetED2kLink());
				break;
			case MP_GETED2KLINK:{
				CString str;
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					CShareableFile* file = selectedList.GetNext(pos);
					if (file != NULL && file->IsKindOf(RUNTIME_CLASS(CKnownFile))){
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
					if (file->IsKindOf(RUNTIME_CLASS(CKnownFile))){
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
			case MP_OPEN:
			case IDA_ENTER:
				if (file && !file->IsPartFile())
					OpenFile(file);
				break; 
			case MP_INSTALL_SKIN:
				if (file && !file->IsPartFile())
					InstallSkin(file->GetFilePath());
				break;
			case MP_OPENFOLDER:
				if (file && !file->IsPartFile())
					ShellExecute(NULL, _T("open"), _T("explorer"), _T("/select,\"") + file->GetFilePath() + _T("\""), NULL, SW_SHOW);
				break; 
			case MP_RENAME:
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

						if (pKnownFile->IsKindOf(RUNTIME_CLASS(CPartFile)))
						{
							pKnownFile->SetFileName(newname);
							STATIC_DOWNCAST(CPartFile, pKnownFile)->SetFullName(newpath); 
						}
						else
						{
							theApp.sharedfiles->RemoveKeywords(pKnownFile);
							pKnownFile->SetFileName(newname);
							theApp.sharedfiles->AddKeywords(pKnownFile);
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
						if (myfile->IsKindOf(RUNTIME_CLASS(CKnownFile))) 
							theApp.sharedfiles->RemoveFile((CKnownFile*)myfile, true);
						else
							RemoveFile(myfile, true);
						bRemovedItems = true;
						if (myfile->IsKindOf(RUNTIME_CLASS(CPartFile)))
							theApp.emuledlg->transferwnd->GetDownloadList()->ClearCompleted(static_cast<CPartFile*>(myfile));
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
					theApp.emuledlg->sharedfileswnd->ShowSelectedFilesDetails();
					theApp.emuledlg->sharedfileswnd->OnSingleFileShareStatusChanged(); // might have been a single shared file
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
						|| theApp.sharedfiles->ShouldBeShared(myfile->GetPath(), myfile->GetFilePath(), true))
					{
						continue;
					}

					bUnsharedItems |= theApp.sharedfiles->ExcludeFile(myfile->GetFilePath());
					ASSERT( bUnsharedItems );
				}
				SetRedraw(TRUE);
				if (bUnsharedItems) {
					theApp.emuledlg->sharedfileswnd->ShowSelectedFilesDetails();
					theApp.emuledlg->sharedfileswnd->OnSingleFileShareStatusChanged();
					if (GetFirstSelectedItemPosition() == NULL)
						AutoSelectItem();
				}
				break; 
			}
			case MP_CMT:
				ShowFileDialog(selectedList, IDD_COMMENT);
                break; 
			case MPG_ALTENTER:
			case MP_DETAIL:
				ShowFileDialog(selectedList);
				break;
			case MP_FIND:
				OnFindStart();
				break;
			case MP_CREATECOLLECTION:
			{
				CCollection* pCollection = new CCollection();
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					CShareableFile* pFile = selectedList.GetNext(pos);
					if (pFile->IsKindOf(RUNTIME_CLASS(CKnownFile)))
						pCollection->AddFileToCollection(pFile, true);
				}
				CCollectionCreateDialog dialog;
				dialog.SetCollection(pCollection,true);
				dialog.DoModal();
				//We delete this collection object because when the newly created
				//collection file is added to the sharedfile list, it is read and verified
				//and which creates the colleciton object that is attached to that file..
				delete pCollection;
				break;
			}
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
					CCollectionViewDialog dialog;
					dialog.SetCollection(pKnownFile->m_pCollection);
					dialog.DoModal();
				}
				break;
			case MP_MODIFYCOLLECTION:
				if (pKnownFile && pKnownFile->m_pCollection)
				{
					CCollectionCreateDialog dialog;
					CCollection* pCollection = new CCollection(pKnownFile->m_pCollection);
					dialog.SetCollection(pCollection,false);
					dialog.DoModal();
					delete pCollection;				
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
						if (!selectedList.GetAt(pos)->IsKindOf(RUNTIME_CLASS(CKnownFile)))
						{
							selectedList.GetNext(pos); //zz_fly :: bug fix :: DolphinX
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
								if(file->IsPartFile()) //only to be sure
									break;
								file->SetAutoUpPriority(false);
								file->SetUpPriority(PR_POWER);
								UpdateFile(file);
								break;
							//Xman end
							case MP_PRIOAUTO:
								file->SetAutoUpPriority(true);
								//Xman advanced upload-priority
								if (thePrefs.UseAdvancedAutoPtio())
#ifdef _BETA
									file->CalculateAndSetUploadPriority2(); 
#else
									file->CalculateAndSetUploadPriority(); 
#endif
								else
								//Xman end
								file->UpdateAutoUpPriority();
								UpdateFile(file); 
								break;
						}
					}
					SetRedraw(TRUE); //Xman Code Improvement

					break;
				}
			// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
			case MP_HIDEOS_DEFAULT:
			case MP_HIDEOS_SET:
			{
				POSITION pos = selectedList.GetHeadPosition();
				int newHideOS = -1;
				if (wParam==MP_HIDEOS_SET)
				{
					InputBox inputbox;
					CString title=GetResString(IDS_HIDEOS);
					CString currHideOS;
					if (pKnownFile)
						currHideOS.Format(_T("%i"), (pKnownFile->GetHideOS()>=0)?pKnownFile->GetHideOS():thePrefs.GetHideOvershares());
					else
						currHideOS = _T("0");
					inputbox.SetLabels(GetResString(IDS_HIDEOS), GetResString(IDS_HIDEOVERSHARES), currHideOS);
					inputbox.SetNumber(true);
					int result = inputbox.DoModal();
					if (result == IDCANCEL || (newHideOS = inputbox.GetInputInt()) < 0)
						break;
				}
				SetRedraw(FALSE);
				while (pos != NULL)
				{
					CShareableFile* file = selectedList.GetNext(pos);
					if (!file->IsKindOf(RUNTIME_CLASS(CKnownFile)))
						continue;
					if  (newHideOS == ((CKnownFile*)file)->GetHideOS())
						continue;
					((CKnownFile*)file)->SetHideOS(newHideOS);
					UpdateFile(file);
				}
				SetRedraw(TRUE);
				break;
			}
			// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
			// ==> PowerShare [ZZ/MorphXT] - Stulle
			case MP_POWERSHARE_ON:
			case MP_POWERSHARE_OFF:
			case MP_POWERSHARE_DEFAULT:
			case MP_POWERSHARE_AUTO:
			case MP_POWERSHARE_LIMITED:
			{
				SetRedraw(FALSE);
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					CShareableFile* file = selectedList.GetNext(pos);
					if (!file->IsKindOf(RUNTIME_CLASS(CKnownFile)))
						continue;
					switch (wParam) {
						case MP_POWERSHARE_DEFAULT:
							((CKnownFile*)file)->SetPowerShared(-1);
							break;
						case MP_POWERSHARE_ON:
							((CKnownFile*)file)->SetPowerShared(1);
							break;
						case MP_POWERSHARE_OFF:
							((CKnownFile*)file)->SetPowerShared(0);
							break;
						case MP_POWERSHARE_AUTO:
							((CKnownFile*)file)->SetPowerShared(2);
							break;
						case MP_POWERSHARE_LIMITED:
							((CKnownFile*)file)->SetPowerShared(3);
							break;
					}
					UpdateFile(file);
				}
				SetRedraw(TRUE);
				break;
			}
			case MP_POWERSHARE_LIMIT:
			case MP_POWERSHARE_LIMIT_SET:
			{
				POSITION pos = selectedList.GetHeadPosition();
				int newPowerShareLimit = -1;
				if (wParam==MP_POWERSHARE_LIMIT_SET)
				{
					InputBox inputbox;
					CString title=GetResString(IDS_POWERSHARE);
					CString currPowerShareLimit;
					if (pKnownFile)
						currPowerShareLimit.Format(_T("%i"), (pKnownFile->GetPowerShareLimit()>=0)?pKnownFile->GetPowerShareLimit():thePrefs.GetPowerShareLimit());
					else
						currPowerShareLimit = _T("0");
					inputbox.SetLabels(GetResString(IDS_POWERSHARE), GetResString(IDS_POWERSHARE_LIMIT), currPowerShareLimit);
					inputbox.SetNumber(true);
					int result = inputbox.DoModal();
					if (result == IDCANCEL || (newPowerShareLimit = inputbox.GetInputInt()) < 0)
						break;
				}
				SetRedraw(FALSE);
				while (pos != NULL)
				{
					CShareableFile* file = selectedList.GetNext(pos);
					if  (newPowerShareLimit == ((CKnownFile*)file)->GetPowerShareLimit())
						break;
					((CKnownFile*)file)->SetPowerShareLimit(newPowerShareLimit);
					if (((CKnownFile*)file)->IsPartFile())
						((CPartFile*)file)->UpdatePartsInfo();
					else
						((CKnownFile*)file)->UpdatePartsInfo();
					UpdateFile(file);
				}
				SetRedraw(TRUE);
				break;
			}
			// <== PowerShare [ZZ/MorphXT] - Stulle
			// ==> Limit PS by amount of data uploaded [Stulle] - Stulle
			case MP_PS_AMOUNT_LIMIT:
			case MP_PS_AMOUNT_LIMIT_SET:
			{
				POSITION pos = selectedList.GetHeadPosition();
				int newPsAmountLimit = -1;
				if (wParam==MP_PS_AMOUNT_LIMIT_SET)
				{
					InputBox inputbox;
					CString title=GetResString(IDS_POWERSHARE);
					CString currPsAmountLimit;
					if (pKnownFile)
						currPsAmountLimit.Format(_T("%i"), ((pKnownFile->GetPsAmountLimit()>=0.0f)?pKnownFile->GetPsAmountLimit():thePrefs.GetPsAmountLimit()));
					else
						currPsAmountLimit = _T("0");
					inputbox.SetLabels(GetResString(IDS_POWERSHARE), GetResString(IDS_PS_AMOUNT_LIMIT_LABEL), currPsAmountLimit);
					inputbox.SetNumber(true);
					int result = inputbox.DoModal();
					if (result == IDCANCEL || (newPsAmountLimit = inputbox.GetInputInt()) < 0)
						break;
					if (newPsAmountLimit > MAX_PS_AMOUNT_LIMIT)
					{
						AfxMessageBox(GetResString(IDS_PS_AMOUNT_LIMIT_WRONG),MB_OK | MB_ICONINFORMATION,0);
						break;
					}
				}
				SetRedraw(FALSE);
				while (pos != NULL)
				{
					CShareableFile* file = selectedList.GetNext(pos);
					if  (newPsAmountLimit == ((CKnownFile*)file)->GetPsAmountLimit())
						break;
					((CKnownFile*)file)->SetPsAmountLimit(newPsAmountLimit);
					if (((CKnownFile*)file)->IsPartFile())
						((CPartFile*)file)->UpdatePartsInfo();
					else
						((CKnownFile*)file)->UpdatePartsInfo();
					UpdateFile(file);
				}
				SetRedraw(TRUE);
				break;
			}
			// <== Limit PS by amount of data uploaded [Stulle] - Stulle
			// ==> Spread bars [Slugfiller/MorphXT] - Stulle
			case MP_SPREADBAR_RESET:
			{
				SetRedraw(FALSE);
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					CShareableFile* file = selectedList.GetNext(pos);
					((CKnownFile*)file)->statistic.ResetSpreadBar();
				}
				SetRedraw(TRUE);
				break;
			}
			// <== Spread bars [Slugfiller/MorphXT] - Stulle
			// ==> Copy feedback feature [MorphXT] - Stulle
			/*
			// Xman: idea: IcEcRacKer Copy UL-feedback
			case MP_ULFEEDBACK: 
				{ 
					CString feed; 

					bool morefiles = selectedList.GetCount() > 1;
					uint64 sumTransferred=0;
					uint64 sumAllTimeTransferred=0;

					if(!selectedList.IsEmpty())
					{
						feed.AppendFormat(_T("%s: %s \r\n"), GetResString(IDS_SF_STATISTICS),thePrefs.GetUserNick()); 
						feed.AppendFormat(_T("Mod: %s%s[%s] \r\n"),_T("eMule"), theApp.m_strCurVersionLong, MOD_VERSION);  
					}

					while (!selectedList.IsEmpty())
					{
						CShareableFile* file = selectedList.RemoveHead();
						if (!file->IsKindOf(RUNTIME_CLASS(CKnownFile)))
							continue;
						sumTransferred += ((CKnownFile*)file)->statistic.GetTransferred();
						sumAllTimeTransferred += ((CKnownFile*)file)->statistic.GetAllTimeTransferred();

						feed.AppendFormat(_T("%s: %s \r\n"),GetResString(IDS_DL_FILENAME),((CKnownFile*)file)->GetFileName()); 
						feed.AppendFormat(_T("%s: %s \r\n"),GetResString(IDS_TYPE),((CKnownFile*)file)->GetFileType()); 
						feed.AppendFormat(_T("%s: %s\r\n"),GetResString(IDS_DL_SIZE), CastItoXBytes(((CKnownFile*)file)->GetFileSize(), false, false)); 
						CPartFile* pfile = (CPartFile*)file; 
						if(pfile && pfile->IsPartFile()) 
							feed.AppendFormat(_T("%s %.1f%%\r\n"), GetResString(IDS_FD_COMPSIZE), pfile->GetPercentCompleted()); 
						else 
							feed.AppendFormat(_T("%s 100%%\r\n"), GetResString(IDS_FD_COMPSIZE)); 
						feed.AppendFormat(_T("%s: %s (%s) \r\n"),GetResString(IDS_SF_TRANSFERRED), CastItoXBytes(((CKnownFile*)file)->statistic.GetTransferred(), false, false), CastItoXBytes(((CKnownFile*)file)->statistic.GetAllTimeTransferred(), false, false));   
						feed.AppendFormat(_T("%s: %u (%u)\r\n"),GetResString(IDS_COMPLSOURCES),((CKnownFile*)file)->m_nCompleteSourcesCountLo, ((CKnownFile*)file)->m_nVirtualCompleteSourcesCount); 
						feed.AppendFormat(_T("%s: %u \r\n"),GetResString(IDS_ONQUEUE),(((CKnownFile*)file)->GetOnUploadqueue()));  //Xman see OnUploadqueue
						feed.AppendFormat(_T("%s: %u (%u) \r\n\r\n"),GetResString(IDS_SF_ACCEPTS),((CKnownFile*)file)->statistic.GetAccepts(),(((CKnownFile*)file)->statistic.GetAllTimeAccepts())); 
					}
					if(morefiles)
						feed.AppendFormat(_T("sum: %s: %s (%s) \r\n\r\n"),GetResString(IDS_SF_TRANSFERRED), CastItoXBytes(sumTransferred, false, false), CastItoXBytes(sumAllTimeTransferred, false, false));   

					theApp.CopyTextToClipboard(feed); 
					break; 
				} 
			//Xman end
			*/
 			case MP_COPYFEEDBACK:
			case MP_COPYFEEDBACK_US:
			{
				CString feed;
				uint64 uTransferredSum = 0;
				uint64 uTransferredAllSum = 0;
				int iCount = 0;
				POSITION pos = selectedList.GetHeadPosition();

				if(wParam == MP_COPYFEEDBACK_US)
				{
					// ==> Feedback personalization [Stulle] - Stulle
					/*
					feed.AppendFormat(_T("Feedback from %s on [%s]\r\n"),thePrefs.GetUserNick(),theApp.m_strModLongVersion);
					*/
					CString tmp;
					tmp.Format(_T("Feedback from %s on [%s]"),GetColoredText(thePrefs.GetUserNick(),style_f_names),GetColoredText(theApp.m_strModLongVersion,style_f_names));
					feed.Append(GetColoredText(tmp,style_f_label));
					feed.Append(_T("\r\n"));
					// <== Feedback personalization [Stulle] - Stulle
				}
				else
				{
					// ==> Feedback personalization [Stulle] - Stulle
					/*
					feed.AppendFormat(GetResString(IDS_FEEDBACK_FROM),thePrefs.GetUserNick(), theApp.m_strModLongVersion);
					feed.Append(_T("\r\n"));
					*/
					CString tmp;
					tmp.Format(GetResString(IDS_FEEDBACK_FROM),GetColoredText(thePrefs.GetUserNick(),style_f_names),GetColoredText(theApp.m_strModLongVersion,style_f_names));
					feed.Append(GetColoredText(tmp,style_f_label));
					feed.Append(_T("\r\n"));
					// <== Feedback personalization [Stulle] - Stulle
				}

				while (pos != NULL)
				{
					CShareableFile* file = selectedList.GetNext(pos);
					if (!file->IsKindOf(RUNTIME_CLASS(CKnownFile)))
						continue;

					feed.Append(((CKnownFile*)file)->GetFeedback(wParam == MP_COPYFEEDBACK_US));
					if(pos != NULL) // Feedback personalization [Stulle] - Stulle
						feed.Append(_T("\r\n"));

					uTransferredSum += ((CKnownFile*)file)->statistic.GetTransferred();
					uTransferredAllSum += ((CKnownFile*)file)->statistic.GetAllTimeTransferred();
					iCount++;
				}

				if(iCount>1)
				{
					feed.Append(_T("\r\n"));
					if(wParam == MP_COPYFEEDBACK_US)
					{
						// ==> Feedback personalization [Stulle] - Stulle
						/*
						feed.AppendFormat(_T("Transferred (all files): %s (%s)\r\n"),CastItoXBytes(uTransferredSum,false,false,3,true),CastItoXBytes(uTransferredAllSum,false,false,3,true));
						*/
						feed.AppendFormat(_T("Transferred (all files): %s (%s)"),GetColoredText(CastItoXBytes(uTransferredSum,false,false,3,true),style_f_transferred),GetColoredText(CastItoXBytes(uTransferredAllSum,false,false,3,true),style_f_transferred));
						// <== Feedback personalization [Stulle] - Stulle
					}
					else
					{
						// ==> Feedback personalization [Stulle] - Stulle
						/*
						feed.AppendFormat(_T("%s: %s (%s)\r\n"),GetResString(IDS_FEEDBACK_ALL_TRANSFERRED),CastItoXBytes(uTransferredSum,false,false,3),CastItoXBytes(uTransferredAllSum,false,false,3));
						*/
						feed.AppendFormat(_T("%s: %s (%s)"),GetResString(IDS_FEEDBACK_ALL_TRANSFERRED),GetColoredText(CastItoXBytes(uTransferredSum,false,false,3,true),style_f_transferred),GetColoredText(CastItoXBytes(uTransferredAllSum,false,false,3,true),style_f_transferred));
						// <== Feedback personalization [Stulle] - Stulle
					}
				}
				feed.Append(GetColoredText(_T(""),-style_f_label)); // Feedback personalization [Stulle] - Stulle
				feed.Append(_T("\r\n"));
				//Todo: copy all the comments too
				theApp.CopyTextToClipboard(feed);
				break;
			}
			// <== Copy feedback feature [MorphXT] - Stulle

			//Xman Mass Rename (Morph)
			case MP_MASSRENAME: 
			{
				CMassRenameDialog MRDialog;
				// Add the files to the dialog
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL) {
					CShareableFile* file = selectedList.GetNext(pos);
					if (!file->IsKindOf(RUNTIME_CLASS(CKnownFile)))
						continue;
					MRDialog.m_FileList.AddTail((CKnownFile*)file);
				}
				int result = MRDialog.DoModal ();
				if (result == IDOK) {
					// The user has successfully entered new filenames. Now we have
					// to rename all the files...
					POSITION pos = selectedList.GetHeadPosition();
					int i=0;
					while (pos != NULL) {
						CString newname = MRDialog.m_NewFilenames.at (i);
						CString newpath = MRDialog.m_NewFilePaths.at (i);
						CShareableFile* file = selectedList.GetNext(pos);
						if (!file->IsKindOf(RUNTIME_CLASS(CKnownFile)))
							continue;
						// .part files could be renamed by simply changing the filename
						// in the CKnownFile object.
							if ((!((CKnownFile*)file)->IsPartFile()) && (_trename(((CKnownFile*)file)->GetFilePath(), newpath) != 0)){
							// Use the "Format"-Syntax of AddLogLine here instead of
							// CString.Format+AddLogLine, because if "%"-characters are
							// in the string they would be misinterpreted as control sequences!
							AddLogLine(false,_T("Failed to rename '%s' to '%s', Error: %hs"), file->GetFilePath(), newpath, _tcserror(errno));
						} else {
								//CString strres; // obsolete
								if (!((CKnownFile*)file)->IsPartFile()) {
								// Use the "Format"-Syntax of AddLogLine here instead of
								// CString.Format+AddLogLine, because if "%"-characters are
								// in the string they would be misinterpreted as control sequences!
									AddLogLine(false,_T("Successfully renamed '%s' to '%s'"), ((CKnownFile*)file)->GetFilePath(), newpath);
									((CKnownFile*)file)->SetFileName(newname);
								if (file->IsKindOf(RUNTIME_CLASS(CPartFile)))
									((CPartFile*) file)->SetFullName(newpath);
							} else {
								// Use the "Format"-Syntax of AddLogLine here instead of
								// CString.Format+AddLogLine, because if "%"-characters are
								// in the string they would be misinterpreted as control sequences!
								AddLogLine(false,_T("Successfully renamed .part file '%s' to '%s'"), ((CKnownFile*)file)->GetFileName(), newname);
								((CPartFile*) file)->SetFollowTheMajority(false); // Follow The Majority [AndCycle/Stulle] - Stulle
								((CKnownFile*)file)->SetFileName(newname, true); 
								((CPartFile*) file)->UpdateDisplayedInfo();
								((CPartFile*) file)->SavePartFile(); 
							}
							((CKnownFile*)file)->SetFilePath(newpath);
							UpdateFile(file);
						}

							// Next item (pos is iterated when retriving the current file)
						i++;
					}
				}
				break;
			}
			//Xman end
			default:
				if (file && wParam>=MP_WEBURL && wParam<=MP_WEBURL+256){
					theWebServices.RunURL(file, wParam);
				}
				// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
				else
				{
					POSITION pos = selectedList.GetHeadPosition();
					while (pos != NULL)
					{
						CShareableFile* file = selectedList.GetNext(pos);
						if (!file->IsKindOf(RUNTIME_CLASS(CKnownFile)))
							continue;
						if (wParam>=MP_SELECTIVE_CHUNK && wParam<=MP_SELECTIVE_CHUNK_1){
							((CKnownFile*)file)->SetSelectiveChunk(wParam==MP_SELECTIVE_CHUNK?-1:wParam-MP_SELECTIVE_CHUNK_0);
							UpdateFile(file);
						}else if (wParam>=MP_SHAREONLYTHENEED && wParam<=MP_SHAREONLYTHENEED_1){
							((CKnownFile*)file)->SetShareOnlyTheNeed(wParam==MP_SHAREONLYTHENEED?-1:wParam-MP_SHAREONLYTHENEED_0);
							UpdateFile(file);
						}
					}
				}
				// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
				break;
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
				sortAscending = false;
				break;
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
	// ==> PowerShare [ZZ/MorphXT] - Stulle
	/*
	UpdateSortHistory(pNMListView->iSubItem + (sortAscending ? 0 : 20) + adder, 20);
	SortItems(SortProc, pNMListView->iSubItem + (sortAscending ? 0 : 20) + adder);
	*/
	UpdateSortHistory(pNMListView->iSubItem + (sortAscending ? 0 : 30) + adder, 30);
	SortItems(SortProc, pNMListView->iSubItem + (sortAscending ? 0 : 30) + adder);
	// <== PowerShare [ZZ/MorphXT] - Stulle


	*pResult = 0;
}

int CSharedFilesCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	const CShareableFile* item1 = (CShareableFile*)lParam1;
	const CShareableFile* item2 = (CShareableFile*)lParam2;

	bool bSortAscending;
	int iColumn;
	// ==> PowerShare [ZZ/MorphXT] - Stulle
	/*
	if (lParamSort >= 100) {
		bSortAscending = lParamSort < 120;
		iColumn = bSortAscending ? lParamSort : lParamSort - 20;
	}
	else {
		bSortAscending = lParamSort < 20;
		iColumn = bSortAscending ? lParamSort : lParamSort - 20;
	}
	*/
	if (lParamSort >= 100) {
		bSortAscending = lParamSort < 130;
		iColumn = bSortAscending ? lParamSort : lParamSort - 30;
	}
	else {
		bSortAscending = lParamSort < 30;
		iColumn = bSortAscending ? lParamSort : lParamSort - 30;
	}

	// all indexes shifted by 10!!!

	// <== PowerShare [ZZ/MorphXT] - Stulle
	
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
	}

	if (bExtColumn)
	{
		if (item1->IsKindOf(RUNTIME_CLASS(CKnownFile)) && !item2->IsKindOf(RUNTIME_CLASS(CKnownFile)))
			iResult = (-1);
		else if (!item1->IsKindOf(RUNTIME_CLASS(CKnownFile)) && item2->IsKindOf(RUNTIME_CLASS(CKnownFile)))
			iResult = 1;
		else if (item1->IsKindOf(RUNTIME_CLASS(CKnownFile)) && item2->IsKindOf(RUNTIME_CLASS(CKnownFile)))
		{
			CKnownFile* kitem1 = (CKnownFile*)item1;
			CKnownFile* kitem2 = (CKnownFile*)item2;

			switch (iColumn)
			{
				case 3:{//prio
					// ==> PowerShare [ZZ/MorphXT] - Stulle
					/*
					uint8 p1 = kitem1->GetUpPriority() + 1;
					if (p1 == 5)
						p1 = 0;
					uint8 p2 = kitem2->GetUpPriority() + 1;
					if (p2 == 5)
						p2 = 0;
					iResult = p1 - p2;
					*/
					if (!kitem1->GetPowerShared() && kitem2->GetPowerShared())
						iResult=-1;			
					else if (kitem1->GetPowerShared() && !kitem2->GetPowerShared())
						iResult=1;
					else			
						if(kitem1->GetUpPriority() == PR_VERYLOW && kitem2->GetUpPriority() != PR_VERYLOW)
							iResult=-1;
						else if (kitem1->GetUpPriority() != PR_VERYLOW && kitem2->GetUpPriority() == PR_VERYLOW)
							iResult=1;
						else
							iResult=kitem1->GetUpPriority()-kitem2->GetUpPriority();
					// <== PowerShare [ZZ/MorphXT] - Stulle
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
					iResult= kitem1->GetOnUploadqueue() -kitem2->GetOnUploadqueue();
					break;
				//Xman end

				//Xman advanced upload-priority
				//sort by the second value-> faster
				case 19:
					{
						float it1value= kitem1->statistic.GetAllTimeTransferred()/(float)kitem1->GetFileSize()*1000; //sort one number after ,
						float it2value= kitem2->statistic.GetAllTimeTransferred()/(float)kitem2->GetFileSize()*1000;
						iResult=(int)(it1value-it2value);
						break;
					}
				//Xman end

				// ==> Spread bars [Slugfiller/MorphXT] - Stulle
				case 20: //spread asc
				case 21:
					iResult=CompareFloat(kitem1->statistic.GetSpreadSortValue(),kitem2->statistic.GetSpreadSortValue());
					break;
				case 22: // VQB:  Simple UL asc
					{
						float x1 = ((float)kitem1->statistic.GetAllTimeTransferred())/((float)kitem1->GetFileSize());
						float x2 = ((float)kitem2->statistic.GetAllTimeTransferred())/((float)kitem2->GetFileSize());
						iResult=CompareFloat(x1,x2);
					break;
					}
				case 23: // SF:  Full Upload Count asc
					iResult=CompareFloat(kitem1->statistic.GetFullSpreadCount(),kitem2->statistic.GetFullSpreadCount());
					break;
				// <== Spread bars [Slugfiller/MorphXT] - Stulle

				// ==> push rare file - Stulle
				case 24:
					iResult=CompareFloat(kitem1->GetFileRatio(),kitem2->GetFileRatio());
					break;
				// <== push rare file - Stulle

				// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
				case 25:
					if (kitem1->GetHideOS() == kitem2->GetHideOS())
						iResult=kitem1->GetSelectiveChunk() - kitem2->GetSelectiveChunk();
					else
						iResult=kitem1->GetHideOS() - kitem2->GetHideOS();
					break;
				case 26:
					iResult=kitem1->GetShareOnlyTheNeed() - kitem2->GetShareOnlyTheNeed();
					break;
				// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle

				// ==> PowerShare [ZZ/MorphXT] - Stulle
				case 27:
					if (!kitem1->GetPowerShared() && kitem2->GetPowerShared())
						iResult=-1;
					else if (kitem1->GetPowerShared() && !kitem2->GetPowerShared())
						iResult=1;
					else
						if (kitem1->GetPowerSharedMode() != kitem2->GetPowerSharedMode())
							iResult=kitem1->GetPowerSharedMode() - kitem2->GetPowerSharedMode();
						else
							if (!kitem1->GetPowerShareAuthorized() && kitem2->GetPowerShareAuthorized())
								iResult=-1;
							else if (kitem1->GetPowerShareAuthorized() && !kitem2->GetPowerShareAuthorized())
								iResult=1;
							else
								if (!kitem1->GetPowerShareAuto() && kitem2->GetPowerShareAuto())
									iResult=-1;
								else if (kitem1->GetPowerShareAuto() && !kitem2->GetPowerShareAuto())
									iResult=1;
								else
									if (!kitem1->GetPowerShareLimited() && kitem2->GetPowerShareLimited())
										iResult=-1;
									else if (kitem1->GetPowerShareLimited() && !kitem2->GetPowerShareLimited())
										iResult=1;
									else
										iResult=0;
					break;
				// <== PowerShare [ZZ/MorphXT] - Stulle

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
					uint32 tNow = time(NULL);
					int i1 = (tNow < kitem1->GetLastPublishTimeKadSrc()) ? 1 : 0;
					int i2 = (tNow < kitem2->GetLastPublishTimeKadSrc()) ? 1 : 0;
					iResult = i1 - i2;
					break;
				}
			}
		}
	}

	if (!bSortAscending)
		iResult = -iResult;

	// SLUGFILLER: multiSort remove - handled in parent class
	/*
	//call secondary sortorder, if this one results in equal
	int dwNextSort;
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->sharedfileswnd->sharedfilesctrl.GetNextSortOrder(lParamSort)) != -1)
		iResult = SortProc(lParam1, lParam2, dwNextSort);
	*/
	// SLUGFILLER End
	return iResult;
}

void CSharedFilesCtrl::OpenFile(const CShareableFile* file)
{
	if(file->IsKindOf(RUNTIME_CLASS(CKnownFile)) && ((CKnownFile*)file)->m_pCollection)
	{
		CCollectionViewDialog dialog;
		dialog.SetCollection(((CKnownFile*)file)->m_pCollection);
		dialog.DoModal();
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
				CTypedPtrList<CPtrList, CShareableFile*> aFiles;
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
	// ==> PowerShare [ZZ/MorphXT] - Stulle
	if (m_PowershareMenu) VERIFY( m_PowershareMenu.DestroyMenu() );
	if (m_PowerShareLimitMenu) VERIFY( m_PowerShareLimitMenu.DestroyMenu() );
	// <== PowerShare [ZZ/MorphXT] - Stulle
	// ==> Limit PS by amount of data uploaded [Stulle] - Stulle
	if (m_PsAmountLimitMenu) VERIFY( m_PsAmountLimitMenu.DestroyMenu() );
	// <== Limit PS by amount of data uploaded [Stulle] - Stulle
	// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	if (m_HideOSMenu) VERIFY( m_HideOSMenu.DestroyMenu() );
	if (m_SelectiveChunkMenu) VERIFY( m_SelectiveChunkMenu.DestroyMenu() );
	if (m_ShareOnlyTheNeedMenu) VERIFY( m_ShareOnlyTheNeedMenu.DestroyMenu() );
	// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	if (m_PrioMenu) VERIFY( m_PrioMenu.DestroyMenu() );
	if (m_CollectionsMenu) VERIFY( m_CollectionsMenu.DestroyMenu() );
	if (m_SharedFilesMenu) VERIFY( m_SharedFilesMenu.DestroyMenu() );

	// ==> PowerShare [ZZ/MorphXT] - Stulle
	m_PowershareMenu.CreateMenu();
	m_PowershareMenu.AddMenuTitle(GetResString(IDS_POWERSHARE), true, false); // XP Style Menu [Xanatos] - Stulle
	m_PowershareMenu.AppendMenu(MF_STRING,MP_POWERSHARE_DEFAULT,GetResString(IDS_DEFAULT));
	m_PowershareMenu.AppendMenu(MF_STRING,MP_POWERSHARE_OFF,GetResString(IDS_POWERSHARE_DISABLED));
	m_PowershareMenu.AppendMenu(MF_STRING,MP_POWERSHARE_ON,GetResString(IDS_POWERSHARE_ACTIVATED));
	m_PowershareMenu.AppendMenu(MF_STRING,MP_POWERSHARE_AUTO,GetResString(IDS_POWERSHARE_AUTO));
	m_PowershareMenu.AppendMenu(MF_STRING,MP_POWERSHARE_LIMITED,GetResString(IDS_POWERSHARE_LIMITED)); 
	m_PowerShareLimitMenu.CreateMenu();
	m_PowerShareLimitMenu.AddMenuTitle(GetResString(IDS_POWERSHARE_LIMITED), true, false); // XP Style Menu [Xanatos] - Stulle
	m_PowerShareLimitMenu.AppendMenu(MF_STRING,MP_POWERSHARE_LIMIT,	GetResString(IDS_DEFAULT));
	m_PowerShareLimitMenu.AppendMenu(MF_STRING,MP_POWERSHARE_LIMIT_SET,	GetResString(IDS_DISABLED));
	// <== PowerShare [ZZ/MorphXT] - Stulle

	// ==> Limit PS by amount of data uploaded [Stulle] - Stulle
	m_PsAmountLimitMenu.CreateMenu();
	m_PsAmountLimitMenu.AddMenuTitle(GetResString(IDS_PS_LIMITED_AMNT), true, false); // XP Style Menu [Xanatos] - Stulle
	m_PsAmountLimitMenu.AppendMenu(MF_STRING,MP_PS_AMOUNT_LIMIT,	GetResString(IDS_DEFAULT));
	m_PsAmountLimitMenu.AppendMenu(MF_STRING,MP_PS_AMOUNT_LIMIT_SET,	GetResString(IDS_DISABLED));
	// <== Limit PS by amount of data uploaded [Stulle] - Stulle

	// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	m_HideOSMenu.CreateMenu();
	m_HideOSMenu.AddMenuTitle(GetResString(IDS_HIDEOS), true, false); // XP Style Menu [Xanatos] - Stulle
	m_HideOSMenu.AppendMenu(MF_STRING,MP_HIDEOS_DEFAULT, GetResString(IDS_DEFAULT));
	m_HideOSMenu.AppendMenu(MF_STRING,MP_HIDEOS_SET, GetResString(IDS_DISABLED));
	m_SelectiveChunkMenu.CreateMenu();
	m_SelectiveChunkMenu.AddMenuTitle(GetResString(IDS_SELECTIVESHARE), true, false); // XP Style Menu [Xanatos] - Stulle
	m_SelectiveChunkMenu.AppendMenu(MF_STRING,MP_SELECTIVE_CHUNK,	GetResString(IDS_DEFAULT));
	m_SelectiveChunkMenu.AppendMenu(MF_STRING,MP_SELECTIVE_CHUNK_0,	GetResString(IDS_DISABLED));
	m_SelectiveChunkMenu.AppendMenu(MF_STRING,MP_SELECTIVE_CHUNK_1,	GetResString(IDS_ENABLED));

	m_ShareOnlyTheNeedMenu.CreateMenu();
	m_ShareOnlyTheNeedMenu.AddMenuTitle(GetResString(IDS_SHAREONLYTHENEED), true, false); // XP Style Menu [Xanatos] - Stulle
	m_ShareOnlyTheNeedMenu.AppendMenu(MF_STRING,MP_SHAREONLYTHENEED,	GetResString(IDS_DEFAULT));
	m_ShareOnlyTheNeedMenu.AppendMenu(MF_STRING,MP_SHAREONLYTHENEED_0,	GetResString(IDS_DISABLED));
	m_ShareOnlyTheNeedMenu.AppendMenu(MF_STRING,MP_SHAREONLYTHENEED_1,	GetResString(IDS_ENABLED));
	// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle

	m_PrioMenu.CreateMenu();
	m_PrioMenu.AddMenuTitle(GetResString(IDS_PRIORITY), true, false); // XP Style Menu [Xanatos] - Stulle
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOVERYLOW,GetResString(IDS_PRIOVERYLOW));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOLOW,GetResString(IDS_PRIOLOW));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIONORMAL,GetResString(IDS_PRIONORMAL));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOHIGH, GetResString(IDS_PRIOHIGH));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOVERYHIGH, GetResString(IDS_PRIORELEASE));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOPOWER, GetResString(IDS_POWERRELEASE)); //Xman PowerRelease
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOAUTO, GetResString(IDS_PRIOAUTO));//UAP

	m_CollectionsMenu.CreateMenu();
	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	m_CollectionsMenu.AddMenuTitle(NULL, true);
	*/
	m_CollectionsMenu.AddMenuTitle(GetResString(IDS_SEARCH_EMULECOLLECTION), true, false);
	// <== XP Style Menu [Xanatos] - Stulle
	m_CollectionsMenu.AppendMenu(MF_STRING,MP_CREATECOLLECTION, GetResString(IDS_CREATECOLLECTION), _T("COLLECTION_ADD"));
	m_CollectionsMenu.AppendMenu(MF_STRING,MP_MODIFYCOLLECTION, GetResString(IDS_MODIFYCOLLECTION), _T("COLLECTION_EDIT"));
	m_CollectionsMenu.AppendMenu(MF_STRING,MP_VIEWCOLLECTION, GetResString(IDS_VIEWCOLLECTION), _T("COLLECTION_VIEW"));
	m_CollectionsMenu.AppendMenu(MF_STRING,MP_SEARCHAUTHOR, GetResString(IDS_SEARCHAUTHORCOLLECTION), _T("COLLECTION_SEARCH"));

	m_SharedFilesMenu.CreatePopupMenu();
	m_SharedFilesMenu.AddMenuTitle(GetResString(IDS_SHAREDFILES), true);

	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_OPEN, GetResString(IDS_OPENFILE), _T("OPENFILE"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_OPENFOLDER, GetResString(IDS_OPENFOLDER), _T("OPENFOLDER"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_RENAME, GetResString(IDS_RENAME) + _T("..."), _T("FILERENAME"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_REMOVE, GetResString(IDS_DELETE), _T("DELETE"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_UNSHAREFILE, GetResString(IDS_UNSHARE), _T("KADBOOTSTRAP")); // TODO: better icon
	if (thePrefs.IsExtControlsEnabled())
		m_SharedFilesMenu.AppendMenu(MF_STRING,Irc_SetSendLink,GetResString(IDS_IRC_ADDLINKTOIRC), _T("IRCCLIPBOARD"));

	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_PrioMenu.m_hMenu, GetResString(IDS_PRIORITY) + _T(" (") + GetResString(IDS_PW_CON_UPLBL) + _T(")"), _T("FILEPRIORITY"));
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);

	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_CollectionsMenu.m_hMenu, GetResString(IDS_META_COLLECTION), _T("AABCollectionFileType"));
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR); 	

	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_DETAIL, GetResString(IDS_SHOWDETAILS), _T("FILEINFO"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_CMT, GetResString(IDS_CMT_ADD), _T("FILECOMMENTS"));
	if (thePrefs.GetShowCopyEd2kLinkCmd())
		m_SharedFilesMenu.AppendMenu(MF_STRING,MP_GETED2KLINK, GetResString(IDS_DL_LINK1), _T("ED2KLINK") );
	else
		m_SharedFilesMenu.AppendMenu(MF_STRING,MP_SHOWED2KLINK, GetResString(IDS_DL_SHOWED2KLINK), _T("ED2KLINK") );
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_FIND, GetResString(IDS_FIND), _T("Search"));
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);

	m_SharedFilesMenu.AppendHeading(GetResString(IDS_RELEASER));
	// ==> PowerShare [ZZ/MorphXT] - Stulle
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_PowershareMenu.m_hMenu, GetResString(IDS_POWERSHARE), _T("FILEPOWERSHARE"));
	m_PowershareMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	m_PowershareMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_PowerShareLimitMenu.m_hMenu, GetResString(IDS_POWERSHARE_LIMIT));
	// <== PowerShare [ZZ/MorphXT] - Stulle
	// ==> Limit PS by amount of data uploaded [Stulle] - Stulle
    m_PowershareMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_PsAmountLimitMenu.m_hMenu, GetResString(IDS_PS_AMOUNT_LIMIT));
	// <== Limit PS by amount of data uploaded [Stulle] - Stulle
	// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_HideOSMenu.m_hMenu, GetResString(IDS_HIDEOS), _T("FILEHIDEOS"));
	m_HideOSMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	m_HideOSMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_SelectiveChunkMenu.m_hMenu, GetResString(IDS_SELECTIVESHARE));

	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_ShareOnlyTheNeedMenu.m_hMenu, GetResString(IDS_SHAREONLYTHENEED), _T("FILESHAREONLYTHENEED"));
	// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_SPREADBAR_RESET, GetResString(IDS_SPREAD_RESET), _T("RESETSPREADBAR")); // Spread bars [Slugfiller/MorphXT] - Stulle

	// ==> Copy feedback feature [MorphXT] - Stulle
	/*
	// Xman: IcEcRacKer Copy UL-feedback
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_ULFEEDBACK, _T("Copy UL-Feedback"), _T("FILECOMMENTS")); 
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR); 
	//Xman end
	*/
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_COPYFEEDBACK, GetResString(IDS_COPYFEEDBACK), _T("COPY"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_COPYFEEDBACK_US, GetResString(IDS_COPYFEEDBACK_US), _T("COPY"));
	m_SharedFilesMenu.AppendMenu(MF_SEPARATOR);
	// <== Copy feedback feature [MorphXT] - Stulle

	//Xman Mass Rename (Morph)
	if (thePrefs.IsExtControlsEnabled())
	{
		m_SharedFilesMenu.AppendMenu(MF_STRING,MP_MASSRENAME,GetResString(IDS_MR), _T("FILEMASSRENAME"));
		m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR); 
	}
	//Xman end

#if defined(_DEBUG)
	if (thePrefs.IsExtControlsEnabled()){
		//JOHNTODO: Not for release as we need kad lowID users in the network to see how well this work work. Also, we do not support these links yet.
		m_SharedFilesMenu.AppendMenu(MF_STRING,MP_GETKADSOURCELINK, _T("Copy eD2K Links To Clipboard (Kad)"));
		m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	}
#endif
}

void CSharedFilesCtrl::ShowComments(CShareableFile* file)
{
	if (file)
	{
		CTypedPtrList<CPtrList, CShareableFile*> aFiles;
		aFiles.AddHead(file);
		ShowFileDialog(aFiles, IDD_COMMENT);
	}
}

void CSharedFilesCtrl::OnLvnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
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
	else if (nChar == VK_SPACE && CheckBoxesEnabled())
	{
		// Toggle Checkboxes
		// selection and item position might change during processing (shouldn't though, but lets make sure), so first get all pointers instead using the selection pos directly
		SetRedraw(FALSE);
		CTypedPtrList<CPtrList, CShareableFile*> selectedList;
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

void CSharedFilesCtrl::ShowFileDialog(CTypedPtrList<CPtrList, CShareableFile*>& aFiles, UINT uPshInvokePage)
{
	if (aFiles.GetSize() > 0)
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

bool CSharedFilesCtrl::IsFilteredItem(const CShareableFile* pFile) const
{
	const CStringArray& rastrFilter = theApp.emuledlg->sharedfileswnd->m_astrFilter;
	if (rastrFilter.GetSize() == 0)
		return false;

	// filtering is done by text only for all colums to keep it consistent and simple for the user even if that
	// doesn't allows complex filters
	TCHAR szFilterTarget[256];
	GetItemDisplayText(pFile, theApp.emuledlg->sharedfileswnd->GetFilterColumn(),
					   szFilterTarget, _countof(szFilterTarget));

	bool bItemFiltered = false;
	for (int i = 0; i < rastrFilter.GetSize(); i++)
	{
		const CString& rstrExpr = rastrFilter.GetAt(i);
		bool bAnd = true;
		LPCTSTR pszText = (LPCTSTR)rstrExpr;
		if (pszText[0] == _T('-')) {
			pszText += 1;
			bAnd = false;
		}

		bool bFound = (stristr(szFilterTarget, pszText) != NULL);
		if ((bAnd && !bFound) || (!bAnd && bFound)) {
			bItemFiltered = true;
			break;
		}
	}
	return bItemFiltered;
}

void CSharedFilesCtrl::SetToolTipsDelay(DWORD dwDelay)
{
	CToolTipCtrl* tooltip = GetToolTips();
	if (tooltip)
		tooltip->SetDelayTime(TTDT_INITIAL, dwDelay);
}

bool CSharedFilesCtrl::IsSharedInKad(const CKnownFile *file) const
{
	bool bSharedInKad;
	if ((uint32)time(NULL) < file->GetLastPublishTimeKadSrc()) {
		if (theApp.IsFirewalled() && theApp.IsConnected()) {
			if ((theApp.clientlist->GetBuddy() && (file->GetLastPublishBuddy() == theApp.clientlist->GetBuddy()->GetIP()))
				|| (Kademlia::CKademlia::IsRunning() && !Kademlia::CUDPFirewallTester::IsFirewalledUDP(true) && Kademlia::CUDPFirewallTester::IsVerified()))
			{
				bSharedInKad = true;
			}
			else
				bSharedInKad = false;
		}
		else
			bSharedInKad = true;
	}
	else
		bSharedInKad = false;
	return bSharedInKad;
}

void CSharedFilesCtrl::AddShareableFiles(CString strFromDir)
{
	while (!liTempShareableFilesInDir.IsEmpty())	// cleanup old filelist
		delete liTempShareableFilesInDir.RemoveHead();

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
		if (ff.IsDirectory() || ff.IsDots() || ff.IsSystem() || ff.IsTemporary() || ff.GetLength()==0 || ff.GetLength()>MAX_EMULE_FILE_SIZE)
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
		_tsplitpath(strFoundFileName, NULL, NULL, NULL, szExt);
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

		// ==> Make code VS 2005 and VS 2008 ready [MorphXT] - Stulle
		/*
		uint32 fdate = (UINT)tFoundFileTime.GetTime();
		*/
		time_t fdate = (time_t)tFoundFileTime.GetTime();
		// <== Make code VS 2005 and VS 2008 ready [MorphXT] - Stulle
		if (fdate == 0)
			fdate = (UINT)-1;
		if (fdate == -1){
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
	if (CheckBoxesEnabled()) // do we have checkboxes?
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
				recItem.left += sm_iIconOffset;
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
	if (theApp.sharedfiles->ShouldBeShared(pFile->GetPath(), pFile->GetFilePath(), false)){
		// this is currently shared so unshare it
		if (theApp.sharedfiles->ShouldBeShared(pFile->GetPath(), pFile->GetFilePath(), true))
			return; // not allowed to unshare this file
		VERIFY( theApp.sharedfiles->ExcludeFile(pFile->GetFilePath()) );
		// update GUI stuff
		ShowFilesCount();
		theApp.emuledlg->sharedfileswnd->ShowSelectedFilesDetails();
		theApp.emuledlg->sharedfileswnd->OnSingleFileShareStatusChanged();
		// no need to update the list itself, will be handled in the RemoveFile function
	}
	else
	{
		// SLUGFILLER: SafeHash remove - removed installation dir unsharing
		/*
		if (!thePrefs.IsShareableDirectory(pFile->GetPath()))
			return; // not allowed to share
		*/
		// SLUGFILLER: SafeHash remove - removed installation dir unsharing
		VERIFY( theApp.sharedfiles->AddSingleSharedFile(pFile->GetFilePath()) );
		ShowFilesCount();
		theApp.emuledlg->sharedfileswnd->ShowSelectedFilesDetails();
		theApp.emuledlg->sharedfileswnd->OnSingleFileShareStatusChanged();
		UpdateFile(pFile);
	}
}

bool CSharedFilesCtrl::CheckBoxesEnabled() const
{
	return m_pDirectoryFilter != NULL && m_pDirectoryFilter->m_eItemType == SDI_UNSHAREDDIRECTORY;
}

void CSharedFilesCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	// highlighting Checkboxes
	if (CheckBoxesEnabled())
	{
		// are we currently on any checkbox?
		int iItem = HitTest(point);
		if (iItem != (-1))
		{
			CRect recItem;
			if(GetItemRect(iItem, recItem, LVIR_BOUNDS))
			{
				ASSERT( recItem.PtInRect(point) );
				recItem.left += sm_iIconOffset;
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
						UpdateFile(pOldItem, false);
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
			UpdateFile(pOldItem, false);
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
			CStringList liToAddFiles; // all files too add
			CStringList liToAddDirs; // all directories to add
			bool bFromSingleDirectory = true; // are all files from within the same directory
			CString strSingleDirectory = _T(""); // which would be this one
			
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
							// SLUGFILLER: SafeHash remove - removed installation dir unsharing
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
							if (bFromSingleDirectory)
							{
								if (strSingleDirectory.IsEmpty())
									strSingleDirectory = ff.GetFilePath().Left(ff.GetFilePath().ReverseFind('\\') + 1);
								else if (strSingleDirectory.CompareNoCase(ff.GetFilePath().Left(ff.GetFilePath().ReverseFind('\\') + 1)) != NULL)
									bFromSingleDirectory = false;
							}
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

				bool bHaveFiles = false;
				// ==> Automatic shared files updater [MoNKi] - Stulle
				/*
				while (!liToAddFiles.IsEmpty())
					bHaveFiles |= theApp.sharedfiles->AddSingleSharedFile(liToAddFiles.RemoveHead()); // could fail, due to the dirs added above
				*/
				int iDoAsfuReset = 0;
				while (!liToAddFiles.IsEmpty())
					bHaveFiles |= theApp.sharedfiles->AddSingleSharedFile(liToAddFiles.RemoveHead(),false,iDoAsfuReset); // could fail, due to the dirs added above
				// <== Automatic shared files updater [MoNKi] - Stulle

				// GUI updates
				if (!liToAddDirs.IsEmpty())
					theApp.emuledlg->sharedfileswnd->m_ctlSharedDirTree.Reload(true);
				if (bHaveFiles)
					theApp.emuledlg->sharedfileswnd->OnSingleFileShareStatusChanged();
				m_pParent->ShowFilesCount();

				if (bHaveFiles && liToAddDirs.IsEmpty() && bFromSingleDirectory)
				{
					// if we added only files from the same directory, show and select this in the filesystem tree
					ASSERT( !strSingleDirectory.IsEmpty() );
					VERIFY( theApp.emuledlg->sharedfileswnd->m_ctlSharedDirTree.ShowFileSystemDirectory(strSingleDirectory) );
				}
				else if (!liToAddDirs.IsEmpty() && !bHaveFiles)
				{
					// only directories added, if only one select the specific shared dir, otherwise the Shared Directories section
					if (liToAddDirs.GetCount() == 1)
						theApp.emuledlg->sharedfileswnd->m_ctlSharedDirTree.ShowSharedDirectory(liToAddDirs.GetHead());
					else
						theApp.emuledlg->sharedfileswnd->m_ctlSharedDirTree.ShowSharedDirectory(_T(""));
				}
				else
				{
					// otherwise select the All Shared Files category
					theApp.emuledlg->sharedfileswnd->m_ctlSharedDirTree.ShowAllSharedFiles();
				}
				// ==> Automatic shared files updater [MoNKi] - Stulle
				if(iDoAsfuReset == 1 || !liToAddDirs.IsEmpty()) // a dropped file caused reset or we added a dir
				{
					if(thePrefs.GetDirectoryWatcher() && (iDoAsfuReset == 0 || thePrefs.GetSingleSharedDirWatcher()))
						theApp.ResetDirectoryWatcher();
				}
				// <== Automatic shared files updater [MoNKi] - Stulle
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