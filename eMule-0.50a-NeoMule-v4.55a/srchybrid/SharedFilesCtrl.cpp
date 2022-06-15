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
#include "KnownFileList.h" // NEO: AKF - [AllKnownFiles] <-- Xanatos --
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
#include "TransferWnd.h"
#include "ClientList.h"
#include "UpDownClient.h"
#include "Collection.h"
#include "CollectionCreateDialog.h"
#include "CollectionViewDialog.h"
#include "SharedDirsTreeCtrl.h"
#include "SearchParams.h"
#include "SearchDlg.h"
#include "SearchResultsWnd.h"
#include "ToolTipCtrlX.h"
#include "kademlia/kademlia/kademlia.h"
#include "kademlia/kademlia/UDPFirewallTester.h"
#include "downloadqueue.h" // NEO: SP - [SharedParts] <-- Xanatos --
#include "Neo/CP/FilePreferencesDialog.h" // NEO: FCFG - [FileConfiguration] <-- Xanatos --
#include "FileDetailDialog.h" // NEO: NFDD - [NewFileDetailDialog] <-- Xanatos --
#include "neo/functions.h" // NEO: MOD <-- Xanatos --
#include "Neo/NeoPreferences.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
#include "Neo/VooDoo/voodoo.h"
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
#include "MD5Sum.h" // NEO: PP - [PasswordProtection] <-- Xanatos --
#include "langids.h"// NEO: FB - [FeedBack] <-- Xanatos --
#include "Neo/Ini2.h" // NEO: INI - [PlusIniClass] <-- Xanatos --
#include "Neo/GUI/MassRename.h" // NEO: MMR - [MorphMassRemane] <-- Xanatos --
// NEO: CRC - [MorphCRCTag] -- Xanatos -->
#include "Neo/GUI/AddCRC32TagDialog.h" 
#include "UserMsgs.h"
#include "log.h"
// NEO: CRC END <-- Xanatos --
#include "Neo\GUI\XPMenu\MenuXP.h" // NEO: NMX - [NeoMenuXP] <-- Xanatos --

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

bool NeedArchiveInfoPage(const CSimpleArray<CObject*>* paItems);
void UpdateFileDetailsPages(CListViewWalkerPropertySheet *pSheet,
							CResizablePage *pArchiveInfo, CResizablePage *pMediaInfo);

//////////////////////////////////////////////////////////////////////////////
// CSharedFilesCtrl

IMPLEMENT_DYNAMIC(CSharedItem, CObject) // NEO: SP - [SharedParts] <-- Xanatos --

IMPLEMENT_DYNAMIC(CSharedFilesCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CSharedFilesCtrl, CMuleListCtrl)
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_CLICK, OnClick) // NEO: NTS - [NeoTreeStyle] <-- Xanatos --
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
	ON_NOTIFY_REFLECT(LVN_GETINFOTIP, OnLvnGetInfoTip)
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
	ON_WM_KEYDOWN()
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, OnBegindragFilelist) // NEO: SDD - [ShareDargAndDrop] <-- Xanatos --
	// NEO: CRC - [MorphCRCTag] -- Xanatos -->
	ON_MESSAGE(UM_CRC32_RENAMEFILE,	OnCRC32RenameFile)
	ON_MESSAGE(UM_CRC32_UPDATEFILE, OnCRC32UpdateFile)
	// NEO: CRC END <-- Xanatos --
END_MESSAGE_MAP()

CSharedFilesCtrl::CSharedFilesCtrl()
	: CSharedFilesListCtrlItemWalk(this) // NEO: SP - [SharedParts] <-- Xanatos --
	//: CListCtrlItemWalk(this)
{
	memset(&sortstat, 0, sizeof(sortstat));
	nAICHHashing = 0;
	m_pDirectoryFilter = NULL;
	SetGeneralPurposeFind(true, false);
	m_pToolTip = new CToolTipCtrlX;
	m_ShowAllKnow = false; // NEO: AKF - [AllKnownFiles] <-- Xanatos --
	SetColoring(NeoPrefs.GetPartTrafficCollors()); // NEO: NPT - [NeoPartTraffic] <-- Xanatos --
}

CSharedFilesCtrl::~CSharedFilesCtrl()
{
	// NEO: FIX - [DestroyMenu] -- Xanatos -->
	if (m_PrioMenu) VERIFY( m_PrioMenu.DestroyMenu() );
	if (m_PermMenu) VERIFY( m_PermMenu.DestroyMenu() ); // NEO: SSP - [ShowSharePermissions]
	if (m_VirtualDirMenu) VERIFY (m_VirtualDirMenu.DestroyMenu()); // NEO: VSF - [VirtualSharedFiles]
	if (m_PWProtMenu) VERIFY( m_PWProtMenu.DestroyMenu()); // NEO: PP - [PasswordProtection]
	if (m_CollectionsMenu) VERIFY( m_CollectionsMenu.DestroyMenu() );
	if (m_ReleaseMenu) VERIFY( m_ReleaseMenu.DestroyMenu() ); // NEO: MOD - [ReleaseMenu]
	if (m_SharedFilesMenu) VERIFY( m_SharedFilesMenu.DestroyMenu() );
	if (m_CRC32Menu) VERIFY( m_CRC32Menu.DestroyMenu() ); // NEO: CRC - [MorphCRCTag]
	// NEO: FIX END <-- Xanatos --
	delete m_pToolTip;
}

void CSharedFilesCtrl::Init()
{
	SetName(_T("SharedFilesCtrl"));

	CImageList ilDummyImageList; //dummy list for getting the proper height of listview entries
	ilDummyImageList.Create(1, theApp.GetSmallSytemIconSize().cy,theApp.m_iDfltImageListColorFlags|ILC_MASK, 1, 1); 
	SetImageList(&ilDummyImageList, LVSIL_SMALL);
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	ilDummyImageList.Detach();

	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
	ASSERT( (GetStyle() & LVS_SINGLESEL) == 0 );
	EnableDual(); // NEO: SEa - [SortAltExtension] <-- Xanatos --

	InsertColumn(0, GetResString(IDS_DL_FILENAME) ,LVCFMT_LEFT,250,0);
	InsertColumn(1,GetResString(IDS_DL_SIZE),LVCFMT_LEFT,100,1);
	InsertColumn(2,GetResString(IDS_TYPE),LVCFMT_LEFT,50,2);
	InsertColumn(3,GetResString(IDS_PRIORITY),LVCFMT_LEFT,70,3);
	InsertColumn(4,GetResString(IDS_FILEID),LVCFMT_LEFT,220,4);
	InsertColumn(5,GetResString(IDS_SF_REQUESTS),LVCFMT_LEFT,100,5);
	InsertColumn(6,GetResString(IDS_SF_ACCEPTS),LVCFMT_LEFT,100,6);
	InsertColumn(7,GetResString(IDS_SF_TRANSFERRED),LVCFMT_LEFT,120,7);
	InsertColumn(8,GetResString(IDS_UPSTATUS),LVCFMT_LEFT,100,8);
	InsertColumn(9,GetResString(IDS_FOLDER),LVCFMT_LEFT,200,9);
	InsertColumn(10,GetResString(IDS_COMPLSOURCES),LVCFMT_LEFT,100,10);
	InsertColumn(11,GetResString(IDS_SHAREDTITLE),LVCFMT_LEFT,200,11);
	InsertColumn(12,GetResString(IDS_CAT),LVCFMT_LEFT,100,12); // NEO: NSC - [NeoSharedCategories] <-- Xanatos --
	InsertColumn(13,GetResString(IDS_X_PARTTRAFFIC),LVCFMT_LEFT,200,13); // NEO: NPT - [NeoPartTraffic] <-- Xanatos --
	InsertColumn(14,GetResString(IDS_X_UPLOADS),LVCFMT_LEFT,100,14); // NEO: NPT - [NeoPartTraffic] <-- Xanatos --
	InsertColumn(15,GetResString(IDS_PERMISSION),LVCFMT_LEFT,100,15); // NEO: SSP - [ShowSharePermissions] <-- Xanatos --
	InsertColumn(16,GetResString(IDS_X_VDS_VIRTDIRTITLE),LVCFMT_LEFT,200,16); // NEO: VSF - [VirtualSharedFiles] <-- Xanatos --
	InsertColumn(17,GetResString(IDS_X_CRC),LVCFMT_LEFT,100,17); // NEO: CRC - [MorphCRCTag] <-- Xanatos --

	SetAllIcons();
	CreateMenues();
	LoadSettings();

	// Barry - Use preferred sort order from preferences
	SetSortArrow();
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0 : 20) + (GetSortSecondValue() ? 100 : 0));

	CToolTipCtrl* tooltip = GetToolTips();
	if (tooltip){
		m_pToolTip->SetFileIconToolTip(true);
		m_pToolTip->SubclassWindow(*tooltip);
		tooltip->ModifyStyle(0, TTS_NOPREFIX);
		tooltip->SetDelayTime(TTDT_AUTOPOP, 20000);
		tooltip->SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1000);
	}
}

void CSharedFilesCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
	CreateMenues();
}

void CSharedFilesCtrl::SetAllIcons()
{
	m_ImageList.DeleteImageList();
	m_ImageList.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	m_ImageList.SetBkColor(CLR_NONE);
	m_ImageList.Add(CTempIconLoader(_T("EMPTY")));
	m_ImageList.Add(CTempIconLoader(_T("FileSharedServer"), 16, 16));
	m_ImageList.Add(CTempIconLoader(_T("FileSharedKad"), 16, 16));
	m_ImageList.Add(CTempIconLoader(_T("Rating_NotRated")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Fake")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Poor")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Fair")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Good")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Excellent")));
	m_ImageList.Add(CTempIconLoader(_T("Collection_Search"))); // rating for comments are searched on kad
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("FileCommentsOvl"))), 1);
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

	// NEO: NSC - [NeoSharedCategories] -- Xanatos -->
	strRes = GetResString(IDS_CAT);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(12, &hdi);
	// NEO: NSC END <-- Xanatos --

	// NEO: NPT - [NeoPartTraffic] -- Xanatos -->
	strRes = GetResString(IDS_X_PARTTRAFFIC);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(13, &hdi);

	strRes = GetResString(IDS_X_UPLOADS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(14, &hdi);
	// NEO: NPT END <-- Xanatos --

	// NEO: SSP - [ShowSharePermissions] -- Xanatos -->
	strRes = GetResString(IDS_PERMISSION);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(15, &hdi);
	// NEO: SSP END <-- Xanatos --

	// NEO: VSF - [VirtualSharedFiles] -- Xanatos -->
	strRes = GetResString(IDS_X_VDS_VIRTDIRTITLE);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(16, &hdi);
	// NEO: VSF END <-- Xanatos --

	CreateMenues();

	int iItems = GetItemCount();
	for (int i = 0; i < iItems; i++)
		Update(i);
}

void CSharedFilesCtrl::AddFile(const CKnownFile* file)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	// check filter conditions if we should show this file right now
	if (m_pDirectoryFilter != NULL){
		CString strFilePath = file->GetPath();
		if (strFilePath.Right(1) == _T("\\")){
			strFilePath = strFilePath.Left(strFilePath.GetLength()-1);
		}
		switch(m_pDirectoryFilter->m_eItemType){
			case SDI_ALL:
				// No filter
				break;
			// NEO: EFT - [ed2kFileType] -- Xanatos -->
			//case SDI_ED2KFILETYPE:
			//	if (m_pDirectoryFilter->m_nCatFilter == -1 || m_pDirectoryFilter->m_nCatFilter != GetED2KFileTypeID(file->GetFileName()))
			//		return;
			//	break;
			// NEO: EFT END <-- Xanatos --
			// NEO: NSC - [NeoSharedCategories] -- Xanatos -->
			case SDI_CATEGORIES:
				if (m_pDirectoryFilter->m_nCatFilter != -1 && (UINT)m_pDirectoryFilter->m_nCatFilter != file->GetCategory())
					return;
				break;
			// NEO: NSC END <-- Xanatos --
			case SDI_FILESYSTEMPARENT:
				return; // no files
				break;
			case SDI_NO:
				// some shared directory
			case SDI_CATINCOMING:
				// Categories with special incoming dirs
			case SDI_CATTEMP: // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
				// Files with special temp dirs
			case SDI_UNSHAREDDIRECTORY:
				// Items from the whole filesystem tree
				if (strFilePath.CompareNoCase(m_pDirectoryFilter->m_strFullPath) != 0)
					return;
				break;
			case SDI_TEMP:
				// only tempfiles
				if (!file->IsPartFile())
					return;
				// NEO: MTD - [MultiTempDirectories] -- Xanatos --
				//else if (m_pDirectoryFilter->m_nCatFilter != -1 && (UINT)m_pDirectoryFilter->m_nCatFilter != ((CPartFile*)file)->GetCategory())
				//	return;
				break;
			case SDI_DIRECTORY:
				// any userselected shared dir but not incoming or temp
				if (file->IsPartFile())
					return;
				if (strFilePath.CompareNoCase(thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR)) == 0)
					return;
				break;
			case SDI_INCOMING:
				// Main incoming directory
				if (strFilePath.CompareNoCase(thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR)) != 0)
					return;
				// Hmm should we show all incoming files dirs or only those from the main incoming dir here?
				// hard choice, will only show the main for now
				break;

		}
	}
	if (IsFilteredItem(file))
		return;
	if (FindFile(file) != -1)
		return;
	// NEO: PP - [PasswordProtection] -- Xanatos -->
	if (file->IsPWProtHidden())
		return;
	// NEO: PP END <-- Xanatos --
	// NEO: SP - [SharedParts] -- Xanatos -->
	CSharedItem *ItemData = new CSharedItem ;
	
	ItemData->isFile = true;
	ItemData->isOpen = false;
	ItemData->KnownFile = const_cast <CKnownFile*> (file);
	ItemData->Part = 0;
	ItemData->Parts = file->GetPartCount();

	int iItem = InsertItem(LVIF_TEXT|LVIF_PARAM, GetItemCount(), LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)ItemData); 
	// NEO: SP END <-- Xanatos --
	//int iItem = InsertItem(LVIF_TEXT|LVIF_PARAM, GetItemCount(), LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)file);
	if (iItem >= 0)
		Update(iItem);
}

void CSharedFilesCtrl::RemoveFile(const CKnownFile* file)
{
	// NEO: SP - [SharedParts] -- Xanatos -->
	if(file == NULL)
		return;

	CSharedItem* ItemData;
	for(int nItem=0; nItem<GetItemCount(); nItem++)
	{
		ItemData=(CSharedItem*)GetItemData(nItem);
		if(ItemData->isFile && ItemData->KnownFile==file)
		{
			if(ItemData->isOpen)
			{
				SetAlternate(FALSE); // NEO: SEa - [SortAltExtension]
				for(uint16 Part=0; Part<ItemData->Parts; Part++)
				{
					CSharedItem* ItemDataPart=(CSharedItem*)GetItemData(nItem+1);
					if (ItemDataPart->isFile) 
						break;
					SetItemData(nItem+1, NULL);
					DeleteItem(nItem+1);
					if(ItemDataPart) 
						delete ItemDataPart;
				}
			}

			SetItemData(nItem, NULL);
			DeleteItem(nItem);
			if(ItemData) 
				delete ItemData;
		}
	}
	ShowFilesCount();
	// NEO: SP END <-- Xanatos --
	/*int iItem = FindFile(file);
	if (iItem != -1)
	{
		DeleteItem(iItem);
		ShowFilesCount();
	}*/
}

void CSharedFilesCtrl::UpdateFile(const CKnownFile* file)
{
	if (!file || !theApp.emuledlg->IsRunning())
		return;
	int iItem = FindFile(file);
	if (iItem != -1)
	{
		Update(iItem);
		if (GetItemState(iItem, LVIS_SELECTED))
			theApp.emuledlg->sharedfileswnd->ShowSelectedFilesSummary();
	}
}

int CSharedFilesCtrl::FindFile(const CKnownFile* pFile)
{
	// NEO: SP - [SharedParts] -- Xanatos -->
	CSharedItem* ItemData;
	for(int nItem=0; nItem<GetItemCount(); nItem++)
	{
		ItemData=(CSharedItem*)GetItemData(nItem);
		if(ItemData->isFile && ItemData->KnownFile == pFile)
			return nItem;
	}

	return -1;
	// NEO: SP END <-- Xanatos --

	/*LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)pFile;
	return FindItem(&find);*/
}

// NEO: SP - [SharedParts] -- Xanatos -->
void CSharedFilesCtrl::UpdatePart(const CSharedItem* part)
{
	if (!part || !theApp.emuledlg->IsRunning())
		return;
	int iItem = FindPart(part);
	if (iItem != -1)
	{
		Update(iItem);
		/*if (GetItemState(iItem, LVIS_SELECTED))
			theApp.emuledlg->sharedfileswnd->ShowSelectedFilesSummary();*/
	}
}

int CSharedFilesCtrl::FindPart(const CSharedItem* pPart)
{
	CSharedItem* ItemData;
	for(int nItem=0; nItem<GetItemCount(); nItem++)
	{
		ItemData=(CSharedItem*)GetItemData(nItem);
		if(ItemData==pPart)
			return nItem;
	}

	return -1;
}
// NEO: SP END <-- Xanatos --


void CSharedFilesCtrl::ReloadFileList()
{
	DeleteAllItems();
	theApp.emuledlg->sharedfileswnd->ShowSelectedFilesSummary();
	
	CCKey bufKey;
	CKnownFile* cur_file;
	// NEO: AKF - [AllKnownFiles] -- Xanatos -->
	if(m_ShowAllKnow){
		for (POSITION pos = theApp.knownfiles->m_Files_map.GetStartPosition(); pos != 0; ){
			theApp.knownfiles->m_Files_map.GetNextAssoc(pos, bufKey, cur_file);
			AddFile(cur_file);
		}
	}else
	// NEO: AKF END <-- Xanatos --
	{
		for (POSITION pos = theApp.sharedfiles->m_Files_map.GetStartPosition(); pos != 0; ){
			theApp.sharedfiles->m_Files_map.GetNextAssoc(pos, bufKey, cur_file);
			AddFile(cur_file);
		}
	}

	// NEO: SE - [SortExtension] -- Xanatos -->
	AlterSortArrow();

	SetSortArrow();

	if (NeoPrefs.DisableAutoSort() == 2)
		SortItems(SortProc, GetSortItem() + (GetSortAscending()? 0:100));
	// NEO: SE END <-- Xanatos --

	ShowFilesCount();
}

// NEO: AKF - [AllKnownFiles] -- Xanatos -->
void CSharedFilesCtrl::ChangeView(UINT view)
{
	switch(view)
	{
		case 1:
			m_ShowAllKnow = false;
			break;
		case 2:
			m_ShowAllKnow = true;
			break;
		default:
			m_ShowAllKnow = !m_ShowAllKnow;
	}
	ReloadFileList();
}
// NEO: AKF END <-- Xanatos --

//void CSharedFilesCtrl::ShowFilesCount()
void CSharedFilesCtrl::ShowFilesCount(DWORD Percentage) // NEO: MOD - [HashProgress] <-- Xanatos --
{
	CString str;
	// NEO: AKF - [AllKnownFiles] -- Xanatos -->
	if(IsKnownList()){
		if (theApp.sharedfiles->GetHashingCount() + nAICHHashing){
			// NEO: MOD - [HashProgress]
			if(Percentage > 0)
				str.Format(_T(" (%i | %i, %s %i) %i%%"), theApp.knownfiles->GetCount(), theApp.sharedfiles->GetCount(), GetResString(IDS_HASHING), theApp.sharedfiles->GetHashingCount() + nAICHHashing, Percentage);
			else
			// NEO: MOD END
				str.Format(_T(" (%i | %i, %s %i)"), theApp.knownfiles->GetCount(), theApp.sharedfiles->GetCount(), GetResString(IDS_HASHING), theApp.sharedfiles->GetHashingCount() + nAICHHashing);
		}else
			str.Format(_T(" (%i | %i)"), theApp.knownfiles->GetCount(), theApp.sharedfiles->GetCount());
	}else
	// NEO: AKF END <-- Xanatos --
	{
		if (theApp.sharedfiles->GetHashingCount() + nAICHHashing){
			// NEO: MOD - [HashProgress]
			if(Percentage > 0)
				str.Format(_T(" (%i, %s %i) %i%%"), theApp.sharedfiles->GetCount(), GetResString(IDS_HASHING), theApp.sharedfiles->GetHashingCount() + nAICHHashing, Percentage);
			else
			// NEO: MOD END
				str.Format(_T(" (%i, %s %i)"), theApp.sharedfiles->GetCount(), GetResString(IDS_HASHING), theApp.sharedfiles->GetHashingCount() + nAICHHashing);
		}else
			str.Format(_T(" (%i)"), theApp.sharedfiles->GetCount());
	}
	// NEO: AKF - [AllKnownFiles] -- Xanatos 
	theApp.emuledlg->sharedfileswnd->m_btnWndS->SetWindowText(GetResString(IsKnownList() ? IDS_X_KN_FILES : IDS_SF_FILES) + str);
	theApp.emuledlg->sharedfileswnd->ChangeSIcon(IsKnownList() ? 0 : 1);
	if(IsKnownList())
		theApp.emuledlg->sharedfileswnd->m_btnWndS->CheckButton(MP_VIEWS_KNOWN);
	else
		theApp.emuledlg->sharedfileswnd->m_btnWndS->CheckButton(MP_VIEWS_SHARED);
	// NEO: AKF END <-- Xanatos --
	//theApp.emuledlg->sharedfileswnd->GetDlgItem(IDC_TRAFFIC_TEXT)->SetWindowText(GetResString(IDS_SF_FILES) + str);
}

void CSharedFilesCtrl::GetItemDisplayText(const CKnownFile* file, int iSubItem, LPTSTR pszText, int cchTextMax) const
{
	if (pszText == NULL || cchTextMax <= 0) {
		ASSERT(0);
		return;
	}
	pszText[0] = _T('\0');
	CString buffer;


	// NEO: MOD -- Xanatos -->
	switch(iSubItem){
		case 0:{
			buffer = file->GetFileName();
			break;
		}
		case 1:
			buffer = CastItoXBytes(file->GetFileSize(), false, false);
			break;
		case 2:
			buffer = file->GetFileTypeDisplayStr();
			break;
		case 3:{
			buffer = file->GetUpPriorityDisplayString();
			// NEO: SRS - [SmartReleaseSharing]
			if(file->IsReleasePriority()){
				CString buffer2;
				buffer2.Format(_T("%s (%s)"), GetResString(IDS_PRIORELEASE), buffer);
				if(file->GetReleasePriority())
					buffer2.AppendFormat(_T(" [%.2f]"),file->GetReleaseModifyer());
				if(file->GetPowerShared())
					buffer2.AppendFormat(_T(" PS"));
				buffer = buffer2;
			}
			// NEO: SRS
			break;
		}
		case 4:
			buffer = md4str(file->GetFileHash());
			break;
		case 5:
            buffer.Format(_T("%u (%u)"), file->statistic.GetRequests(), file->statistic.GetAllTimeRequests());
			break;
		case 6:
			buffer.Format(_T("%u (%u)"), file->statistic.GetAccepts(), file->statistic.GetAllTimeAccepts());
			break;
		case 7:
			buffer.Format(_T("%s (%s)"), CastItoXBytes(file->statistic.GetTransferred(), false, false), CastItoXBytes(file->statistic.GetAllTimeTransferred(), false, false));
			break;
		case 8:
			break;
		case 9:
			buffer = file->GetPath();
			PathRemoveBackslash(buffer.GetBuffer());
			buffer.ReleaseBuffer();
			break;
		case 10:
            if (file->m_nCompleteSourcesCountLo == file->m_nCompleteSourcesCountHi)
				buffer.Format(_T("%u"), file->m_nCompleteSourcesCountLo);
            else if (file->m_nCompleteSourcesCountLo == 0)
				buffer.Format(_T("< %u"), file->m_nCompleteSourcesCountHi);
			else
				buffer.Format(_T("%u - %u"), file->m_nCompleteSourcesCountLo, file->m_nCompleteSourcesCountHi);
			break;
		case 11:{
			break;
		}
		// NEO: NSC - [NeoSharedCategories]
		case 12:
			if(file->GetCategory()!=0)
				buffer = thePrefs.GetCategory(file->GetCategory())->strTitle;
			else
				buffer.Empty();
			break;
		// NEO: NSC END
		// NEO: NPT - [NeoPartTraffic]
		case 13:
			break;
		case 14: 
			if(NeoPrefs.UsePartTraffic()){
				double CompHi = (double)file->statistic.GetAllTimeTransferred()/(uint64)file->GetFileSize();
				double CompLo = file->statistic.GetCompleteReleases();
				if (CompLo == 0){
					buffer.Format(_T("< %.2f"), CompHi);
				}else if (CompLo == CompHi){
					buffer.Format(_T("%.2f"), CompLo);
				}else{
					buffer.Format(_T("%.2f - %.2f"), CompLo, CompHi);
				}
			}
			break;
		// NEO: NPT END
		// NEO: SSP - [ShowSharePermissions]
		case 15:{
			int ViewPerm = NeoPrefs.UseShowSharePermissions() ? file->GetPermissions() : thePrefs.CanSeeShares();
			switch (ViewPerm)
			{
				case PERM_NONE: 
					buffer = GetResString(IDS_X_PERM_HIDDEN); 
					break;
				case PERM_FRIENDS: 
					buffer = GetResString(IDS_X_PERM_FRIENDS); 
					break;
				case PERM_ALL: 
					buffer = GetResString(IDS_X_PERM_PUBLIC); 
					break;
				case PERM_DEFAULT:
					buffer = GetResString(IDS_X_PERM_DEFAULT);
					break;
			}

			if(ViewPerm == PERM_DEFAULT)
				switch (thePrefs.CanSeeShares())
				{
					case PERM_NONE: 
						buffer.AppendFormat(_T(" [%s]"),GetResString(IDS_X_PERM_HIDDEN)); 
						break;
					case PERM_FRIENDS: 
						buffer.AppendFormat(_T(" [%s]"),GetResString(IDS_X_PERM_FRIENDS)); 
						break;
					case PERM_ALL: 
						buffer.AppendFormat(_T(" [%s]"),GetResString(IDS_X_PERM_PUBLIC)); 
						break;
				}
			break;
		}
		// NEO: SSP END 
		// NEO: VSF - [VirtualSharedFiles]
		case 16:
			buffer = file->GetPath(true);
			break;
		// NEO: VSF END
		// NEO: CRC - [MorphCRCTag]
		case 17:
			if(file->IsCRC32Calculated()) {
				buffer.Format(_T("%02X%02X%02X%02X"),	(int) file->GetCalculatedCRC32() [3],
														(int) file->GetCalculatedCRC32() [2],
														(int) file->GetCalculatedCRC32() [1],
														(int) file->GetCalculatedCRC32() [0]);
				if(file->IsCRCOk())
					buffer.AppendFormat(_T(" %s"), GetResString(IDS_X_OK));
			}else
				buffer.Empty();
			break;
		// NEO: CRC END
	}
	_tcsncpy(pszText, buffer, cchTextMax);
	// NEO: MOD END <-- Xanatos --

/*	switch(iSubItem){
		case 0:
			_tcsncpy(pszText, file->GetFileName(), cchTextMax);
			break;
		case 1:
			_tcsncpy(pszText, CastItoXBytes(file->GetFileSize(), false, false), cchTextMax);
			break;
		case 2:
			_tcsncpy(pszText, file->GetFileTypeDisplayStr(), cchTextMax);
			break;
		case 3:{
			_tcsncpy(pszText, file->GetUpPriorityDisplayString(), cchTextMax);
			break;
			   }
		case 4:
			_tcsncpy(pszText, md4str(file->GetFileHash()), cchTextMax);
			break;
		case 5:
			buffer.Format(_T("%u (%u)"), file->statistic.GetRequests(), file->statistic.GetAllTimeRequests());
			_tcsncpy(pszText, buffer, cchTextMax);
			break;
		case 6:
			buffer.Format(_T("%u (%u)"), file->statistic.GetAccepts(), file->statistic.GetAllTimeAccepts());
			_tcsncpy(pszText, buffer, cchTextMax);
			break;
		case 7:
			buffer.Format(_T("%s (%s)"), CastItoXBytes(file->statistic.GetTransferred(), false, false), CastItoXBytes(file->statistic.GetAllTimeTransferred(), false, false));
			_tcsncpy(pszText, buffer, cchTextMax);
			break;
		case 8:
			break;
		case 9:
			_tcsncpy(pszText, file->GetPath(), cchTextMax);
			PathRemoveBackslash(pszText);
			break;
		case 10:
			if (file->m_nCompleteSourcesCountLo == file->m_nCompleteSourcesCountHi)
				buffer.Format(_T("%u"), file->m_nCompleteSourcesCountLo);
			else if (file->m_nCompleteSourcesCountLo == 0)
				buffer.Format(_T("< %u"), file->m_nCompleteSourcesCountHi);
			else
				buffer.Format(_T("%u - %u"), file->m_nCompleteSourcesCountLo, file->m_nCompleteSourcesCountHi);
			_tcsncpy(pszText, buffer, cchTextMax);
			break;
		case 11:
			break;
	}*/

	pszText[cchTextMax - 1] = _T('\0');
}
#define DLC_DT_TEXT (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS)

void CSharedFilesCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	if (!lpDrawItemStruct->itemData)
		return;
	CSharedItem* itemdata = (CSharedItem*)lpDrawItemStruct->itemData; // NEO: SP - [SharedParts] <-- Xanatos --
	CDC* odc = CDC::FromHandle(lpDrawItemStruct->hDC);
	BOOL bCtrlFocused = ((GetFocus() == this) || (GetStyle() & LVS_SHOWSELALWAYS));
	if (lpDrawItemStruct->itemState & ODS_SELECTED) {
		if (bCtrlFocused)
			odc->SetBkColor(m_crHighlight);
		else
			odc->SetBkColor(m_crNoHighlight);
	}
	// NEO: NSC - [NeoSharedCategories] -- Xanatos -->
	else if(NeoPrefs.CollorShareFiles())
	{
		COLORREF catcolor = thePrefs.GetCatColor(itemdata->KnownFile->GetCategory());

		// Lit: White category must be white because background can be changed 
		if(catcolor == 0)
			//odc->SetBkColor(GetBkColor());
			odc->SetBkColor(RGB(255,255,255));
		else
			odc->SetBkColor(RGB(
			//	(GetRValue(catcolor)*16+GetRValue(GetBkColor())*240)/256,
			//	(GetGValue(catcolor)*16+GetGValue(GetBkColor())*240)/256,
			//	(GetBValue(catcolor)*16+GetBValue(GetBkColor())*240)/256));
				(GetRValue(catcolor)*16+60945)/255,
				(GetGValue(catcolor)*16+60945)/255,
				(GetBValue(catcolor)*16+60945)/255));

	}
	// NEO: NSC END <-- Xanatos --
	else
		odc->SetBkColor(GetBkColor());

	// NEO: NTS - [NeoTreeStyle] -- Xanatos -->
	BOOL notFirst = lpDrawItemStruct->itemID != 0;
	BOOL notLast = lpDrawItemStruct->itemID + 1 != (UINT)GetItemCount();
	int tree_start = 0;
	int tree_end = 0;
	// NEO: NTS END <-- Xanatos --

	///*const*/ CKnownFile* file = (CKnownFile*)lpDrawItemStruct->itemData; // SP - removed // NEO: SP - [SharedParts] <-- Xanatos --
	CMemDC dc(odc, &lpDrawItemStruct->rcItem);
	CFont* pOldFont = dc.SelectObject(GetFont());
	CRect cur_rec(lpDrawItemStruct->rcItem);
	COLORREF crOldTextColor = dc.SetTextColor((lpDrawItemStruct->itemState & ODS_SELECTED) ? m_crHighlightText : m_crWindowText);

	int iOldBkMode;
	if (m_crWindowTextBk == CLR_NONE){
		DefWindowProc(WM_ERASEBKGND, (WPARAM)(HDC)dc, 0);
		iOldBkMode = dc.SetBkMode(TRANSPARENT);
	}
	else
		iOldBkMode = OPAQUE;

	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	const int iMarginX = 4;
	cur_rec.right = cur_rec.left - iMarginX*2;
	cur_rec.left += iMarginX;
	//CString buffer; // SP - removed // NEO: SP - [SharedParts] <-- Xanatos --
	//int iIconDrawWidth = theApp.GetSmallSytemIconSize().cx + 3;
	for(int iCurrent = 0; iCurrent < iCount; iCurrent++){
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
		if( !IsColumnHidden(iColumn) ){
			//UINT uDTFlags = DLC_DT_TEXT; // SP - removed // NEO: SP - [SharedParts] <-- Xanatos --
			cur_rec.right += GetColumnWidth(iColumn);

			// NEO: NTS - [NeoTreeStyle] -- Xanatos -->
			if (iColumn==0){
				tree_start = cur_rec.left;
				tree_end = cur_rec.left + 12;
			}
			// NEO: NTS END <-- Xanatos --
			// NEO: SP - [SharedParts] -- Xanatos -->
			if(!itemdata->isFile){
				DrawPartItem(dc,iColumn,&cur_rec,itemdata);
			}else{
				DrawFileItem(dc,iColumn,&cur_rec,itemdata->KnownFile);
			}
			// NEO: SP END <-- Xanatos --

			cur_rec.left += GetColumnWidth(iColumn);
		}
	}
	ShowFilesCount();
	if (lpDrawItemStruct->itemState & ODS_SELECTED)
	{
		RECT outline_rec = lpDrawItemStruct->rcItem;

		outline_rec.top--;
		outline_rec.bottom++;
		dc.FrameRect(&outline_rec, &CBrush(m_crWindow));
		outline_rec.top++;
		outline_rec.bottom--;
		outline_rec.left++;
		outline_rec.right--;

		if (lpDrawItemStruct->itemID > 0 && GetItemState(lpDrawItemStruct->itemID - 1, LVIS_SELECTED))
			outline_rec.top--;

		if (lpDrawItemStruct->itemID + 1 < (UINT)GetItemCount() && GetItemState(lpDrawItemStruct->itemID + 1, LVIS_SELECTED))
			outline_rec.bottom++;

		if(bCtrlFocused)
			dc.FrameRect(&outline_rec, &CBrush(m_crFocusLine));
		else
			dc.FrameRect(&outline_rec, &CBrush(m_crNoFocusLine));
	}
	else if (((lpDrawItemStruct->itemState & ODS_FOCUS) == ODS_FOCUS) && (GetFocus() == this))
	{
		RECT focus_rec;
		focus_rec.top    = lpDrawItemStruct->rcItem.top;
		focus_rec.bottom = lpDrawItemStruct->rcItem.bottom;
		focus_rec.left   = lpDrawItemStruct->rcItem.left + 1;
		focus_rec.right  = lpDrawItemStruct->rcItem.right - 1;
		dc.FrameRect(&focus_rec, &CBrush(m_crNoFocusLine));
	}
	
	// NEO: NTS - [NeoTreeStyle] -- Xanatos -->
	//draw tree last so it draws over selected and focus (looks better)
	if(NeoPrefs.UseTreeStyle()){
		if(tree_start < tree_end)
		{
			//set new bounds
			RECT tree_rect;
			tree_rect.top    = lpDrawItemStruct->rcItem.top;
			tree_rect.bottom = lpDrawItemStruct->rcItem.bottom;
			tree_rect.left   = tree_start;
			tree_rect.right  = tree_end;
			dc->SetBoundsRect(&tree_rect, DCB_DISABLE);

			//gather some information
			BOOL hasNext = notLast && !((const CSharedItem*)GetItemData(lpDrawItemStruct->itemID + 1))->isFile;
			BOOL isOpenRoot = hasNext && itemdata->isFile;
			BOOL isChild = !itemdata->isFile;

			//might as well calculate these now
			int treeCenter = tree_start + 4;
			int middle = (cur_rec.top + cur_rec.bottom + 1) / 2;

			//set up a new pen for drawing the tree
			CPen pn, *oldpn;
			pn.CreatePen(PS_SOLID, 1, RGB(128,128,128)/*m_crWindowText*/);
			oldpn = dc->SelectObject(&pn);

			if(isChild)
			{
				//draw the line to the status bar
				dc->MoveTo(tree_end+10, middle);
				dc->LineTo(tree_start + 4, middle);

				//draw the line to the child node
				if(hasNext)
				{
					dc->MoveTo(treeCenter, middle);
					dc->LineTo(treeCenter, cur_rec.bottom + 1);
				}
			}
			else if (isOpenRoot || (itemdata->isFile && itemdata->Parts > 1))
			{
				//draw box
				RECT circle_rec;
				circle_rec.top    = middle - 5;
				circle_rec.bottom = middle + 4;
				circle_rec.left   = treeCenter - 4;
				circle_rec.right  = treeCenter + 5;
				dc->FrameRect(&circle_rec, &CBrush(RGB(128,128,128)/*m_crWindowText*/));
				CPen penBlack;
				penBlack.CreatePen(PS_SOLID, 1, m_crWindowText);
				CPen* pOldPen2;
				pOldPen2 = dc->SelectObject(&penBlack);
				dc->MoveTo(treeCenter-2,middle - 1);
				dc->LineTo(treeCenter+3,middle - 1);
				
				if (!itemdata->isOpen)
				{
					dc->MoveTo(treeCenter,middle-3);
					dc->LineTo(treeCenter,middle+2);
				}
				dc->SelectObject(pOldPen2);
				//draw the line to the child node
				if (hasNext)
				{
					dc->MoveTo(treeCenter, middle + 4);
					dc->LineTo(treeCenter, cur_rec.bottom + 1);
				}
			}

			//draw the line back up to parent node
			if (notFirst && isChild)
			{
				dc->MoveTo(treeCenter, middle);
				dc->LineTo(treeCenter, cur_rec.top - 1);
			}

			//put the old pen back
			dc->SelectObject(oldpn);
			pn.DeleteObject();
		}
	}
	// NEO: NTS END <-- Xanatos --

	if (m_crWindowTextBk == CLR_NONE)
		dc.SetBkMode(iOldBkMode);
	dc.SelectObject(pOldFont);
	dc.SetTextColor(crOldTextColor);
}

// NEO: SP - [SharedParts] -- Xanatos -->
void CSharedFilesCtrl::DrawFileItem(CDC *dc, int iColumn, LPRECT cur_rec, CKnownFile *file)
{
	int iIconDrawWidth = theApp.GetSmallSytemIconSize().cx + 3;

	// NEO: SSP - [ShowSharePermissions]
	if (iColumn == 15 && NeoPrefs.UseShowSharePermissions() && file->GetPermissions() != PERM_DEFAULT){
		int Perm = /*file->GetPermissions() != PERM_DEFAULT ?*/ file->GetPermissions() /*: thePrefs.CanSeeShares()*/;
		if (Perm == PERM_NONE)
			dc->SetTextColor((COLORREF)RGB(0,175,0));
		else if (Perm == PERM_FRIENDS)
			dc->SetTextColor((COLORREF)RGB(208,128,0));
		else //if (Perm == PERM_ALL)
			dc->SetTextColor((COLORREF)RGB(240,0,0));
	}else
	// NEO: SSP END
	// NEO: AKF - [AllKnownFiles]
	if (!theApp.downloadqueue->IsFileExisting(file->GetFileHash(),false)){
		dc->SetTextColor((COLORREF)RGB(128,128,128));
	}
	else
	// NEO: AKF END
	if (file->IsPartFile()){
		dc->SetTextColor((COLORREF)RGB(0,0,192));
	}else{
		dc->SetTextColor((COLORREF)RGB(0,0,0));
	}

	UINT uDTFlags = DLC_DT_TEXT; 
	CString buffer;

	switch(iColumn){
		case 0:{
			// NEO: NTS - [NeoTreeStyle]
			if(NeoPrefs.UseTreeStyle())
				cur_rec->left += 12;
			// NEO: NTS END

			int iImage = theApp.GetFileTypeSystemImageIdx(file->GetFileName());
			if (theApp.GetSystemImageList() != NULL)
				::ImageList_Draw(theApp.GetSystemImageList(), iImage, dc->GetSafeHdc(), cur_rec->left, cur_rec->top, ILD_NORMAL|ILD_TRANSPARENT);
			if (!file->GetFileComment().IsEmpty() || file->GetFileRating())
				m_ImageList.Draw(dc, 0, CPoint(cur_rec->left, cur_rec->top), ILD_NORMAL | ILD_TRANSPARENT | INDEXTOOVERLAYMASK(1));
			cur_rec->left += (iIconDrawWidth - 3);

			if (thePrefs.ShowRatingIndicator() && (file->HasComment() || file->HasRating() || file->IsKadCommentSearchRunning()))
			{
				m_ImageList.Draw(dc, file->UserRating(true)+3, CPoint(cur_rec->left, cur_rec->top), ILD_NORMAL);
				cur_rec->left += 16;
				iIconDrawWidth += 16;
			}

			cur_rec->left += 3;

			buffer = file->GetFileName();
			break;
		}
		case 1:
			buffer = CastItoXBytes(file->GetFileSize(), false, false);
			uDTFlags |= DT_RIGHT;
			break;
		case 2:
			buffer = file->GetFileTypeDisplayStr();
			break;
		case 3:{
			buffer = file->GetUpPriorityDisplayString();
			// NEO: SRS - [SmartReleaseSharing] -- Xanatos -->
			if(file->IsReleasePriority()){
				CString buffer2;
				buffer2.Format(_T("%s (%s)"), GetResString(IDS_PRIORELEASE), buffer);
				if(file->GetReleasePriority())
					buffer2.AppendFormat(_T(" [%.2f]"),file->GetReleaseModifyer());
				if(file->GetPowerShared())
					buffer2.AppendFormat(_T(" PS"));
				buffer = buffer2;
			}
			// NEO: SRS <-- Xanatos --
			break;
		}
		case 4:
			buffer = md4str(file->GetFileHash());
			break;
		case 5:
            buffer.Format(_T("%u (%u)"), file->statistic.GetRequests(), file->statistic.GetAllTimeRequests());
			break;
		case 6:
			buffer.Format(_T("%u (%u)"), file->statistic.GetAccepts(), file->statistic.GetAllTimeAccepts());
			break;
		case 7:
			buffer.Format(_T("%s (%s)"), CastItoXBytes(file->statistic.GetTransferred(), false, false), CastItoXBytes(file->statistic.GetAllTimeTransferred(), false, false));
			break;
		case 8:
			if (file->GetPartCount()){
				cur_rec->bottom--;
				cur_rec->top++;
				// NEO: MOD - [ClassicShareStatusBar] -- Xanatos -->
				if(NeoPrefs.UseClassicShareStatusBar())
					file->DrawClassicShareStatusBar(dc,cur_rec,false,thePrefs.UseFlatBar());
				else
					file->DrawShareStatusBar(dc,cur_rec,false,thePrefs.UseFlatBar());
				// NEO: MOD END <-- Xanatos --
				cur_rec->bottom++;
				cur_rec->top--;
			}
			break;
		case 9:
			buffer = file->GetPath();
			PathRemoveBackslash(buffer.GetBuffer());
			buffer.ReleaseBuffer();
			break;
		case 10:
            if (file->m_nCompleteSourcesCountLo == file->m_nCompleteSourcesCountHi)
				buffer.Format(_T("%u"), file->m_nCompleteSourcesCountLo);
            else if (file->m_nCompleteSourcesCountLo == 0)
				buffer.Format(_T("< %u"), file->m_nCompleteSourcesCountHi);
			else
				buffer.Format(_T("%u - %u"), file->m_nCompleteSourcesCountLo, file->m_nCompleteSourcesCountHi);
			break;
		case 11:{
			CPoint pt(cur_rec->left, cur_rec->top);
			m_ImageList.Draw(dc, file->GetPublishedED2K() ? 1 : 0, pt, ILD_NORMAL | ILD_TRANSPARENT);
			pt.x += 16;
			bool bSharedInKad;
			if ((uint32)time(NULL) < file->GetLastPublishTimeKadSrc())
			{
				if (theApp.IsFirewalled() && theApp.IsConnected())
				{
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
			m_ImageList.Draw(dc, bSharedInKad ? 2 : 0, pt, ILD_NORMAL | ILD_TRANSPARENT);
			buffer.Empty();
			break;
		}
		// NEO: NSC - [NeoSharedCategories]
		case 12:
			if(file->GetCategory()!=0)
				buffer = thePrefs.GetCategory(file->GetCategory())->strTitle;
			else
				buffer.Empty();
			break;
		// NEO: NSC END
		// NEO: NPT - [NeoPartTraffic]
		case 13:
			if(NeoPrefs.UsePartTraffic() == 1)
				if(file->GetPartCount()){
					cur_rec->bottom--;
					cur_rec->top++;
					file->statistic.DrawTrafficStatusBar(dc,cur_rec,false,thePrefs.UseFlatBar(),GetTrafficColor);
					cur_rec->bottom++;
					cur_rec->top--;
				}
			break;
		case 14: 
			if(NeoPrefs.UsePartTraffic()){
				double CompHi = (double)file->statistic.GetAllTimeTransferred()/(uint64)file->GetFileSize();
				double CompLo = file->statistic.GetCompleteReleases();
				if (CompLo == 0){
					buffer.Format(_T("< %.2f"), CompHi);
				}else if (CompLo == CompHi){
					buffer.Format(_T("%.2f"), CompLo);
				}else{
					buffer.Format(_T("%.2f - %.2f"), CompLo, CompHi);
				}
			}
			break;
		// NEO: NPT END
		// NEO: SSP - [ShowSharePermissions]
		case 15:{
			int ViewPerm = NeoPrefs.UseShowSharePermissions() ? file->GetPermissions() : thePrefs.CanSeeShares();
			switch (ViewPerm)
			{
				case PERM_NONE: 
					buffer = GetResString(IDS_X_PERM_HIDDEN); 
					break;
				case PERM_FRIENDS: 
					buffer = GetResString(IDS_X_PERM_FRIENDS); 
					break;
				case PERM_ALL: 
					buffer = GetResString(IDS_X_PERM_PUBLIC); 
					break;
				case PERM_DEFAULT:
					buffer = GetResString(IDS_X_PERM_DEFAULT);
					break;
			}

			if(ViewPerm == PERM_DEFAULT)
				switch (thePrefs.CanSeeShares())
				{
					case PERM_NONE: 
						buffer.AppendFormat(_T(" [%s]"),GetResString(IDS_X_PERM_HIDDEN)); 
						break;
					case PERM_FRIENDS: 
						buffer.AppendFormat(_T(" [%s]"),GetResString(IDS_X_PERM_FRIENDS)); 
						break;
					case PERM_ALL: 
						buffer.AppendFormat(_T(" [%s]"),GetResString(IDS_X_PERM_PUBLIC)); 
						break;
				}
			break;
		}
		// NEO: SSP END 
		// NEO: VSF - [VirtualSharedFiles]
		case 16:
			buffer = file->GetPath(true);
			break;
		// NEO: VSF END
		// NEO: CRC - [MorphCRCTag] -- Xanatos -->
		case 17:
			if(file->IsCRC32Calculated()) {
				buffer.Format(_T("%02X%02X%02X%02X"),	(int) file->GetCalculatedCRC32() [3],
														(int) file->GetCalculatedCRC32() [2],
														(int) file->GetCalculatedCRC32() [1],
														(int) file->GetCalculatedCRC32() [0]);
				if(file->IsCRCOk())
					buffer.AppendFormat(_T(" %s"), GetResString(IDS_X_OK));
			}else
				buffer.Empty();
			break;
		// NEO: CRC END <-- Xanatos --
	}

	if (iColumn != 8 && iColumn != 13) // NEO: NPT - [NeoPartTraffic]
		dc->DrawText(buffer, buffer.GetLength(), cur_rec, uDTFlags);
	if (iColumn == 0){
		cur_rec->left -= iIconDrawWidth;
		// NEO: NTS - [NeoTreeStyle]
		if(NeoPrefs.UseTreeStyle())
			cur_rec->left -= 12;
		// NEO: NTS END
	}
} 

void CSharedFilesCtrl::DrawPartItem(CDC *dc, int iColumn, LPRECT cur_rec, CSharedItem *part)
{
	UINT uDTFlags = DLC_DT_TEXT; 
	CString buffer;

	bool bOC = false;
	// NEO: MCS - [ManualChunkSelection]
	if (iColumn == 3 && (part->KnownFile->IsPartFile()) && ((CPartFile*)part->KnownFile)->PartPrefs->GetWantedPart(part->Part) == PR_PART_WANTED){
		dc->SetTextColor((COLORREF)RGB(5,65,195)); // Blue
		bOC = true;
	}else 
	// NEO: MCS END
	// NEO: MPS - [ManualPartSharing]
	if (iColumn == 15 && part->KnownFile->KnownPrefs->GetManagedPart(part->Part) != PR_PART_NORMAL){
		uint8 PartPerm = part->KnownFile->GetPartState(part->Part); 
		if (PartPerm == PR_PART_ON)
			dc->SetTextColor((COLORREF)RGB(10,160,70)); // Green
		else if (PartPerm == PR_PART_HIDEN)
			dc->SetTextColor((COLORREF)RGB(190,190,60)); // Yellow
		else if (PartPerm == PR_PART_OFF)
			dc->SetTextColor((COLORREF)RGB(190,60,60)); // Red
		bOC = true;
	}
	// NEO: MPS END
	if(!bOC){
		bool PartDone=false;
		if(part->KnownFile->IsPartFile())
			PartDone = ((CPartFile*)part->KnownFile)->IsComplete(PARTSIZE*part->Part, PARTSIZE*part->Part+PARTSIZE-1, true);
		else
			PartDone = theApp.downloadqueue->IsFileExisting(part->KnownFile->GetFileHash(),false); 

		dc->SetTextColor(PartDone ? (COLORREF)RGB(0,0,0) : (COLORREF)RGB(192,192,192));
	}

	switch(iColumn){
		case 0:
			// NEO: NTS - [NeoTreeStyle]
			if(NeoPrefs.UseTreeStyle())
				cur_rec->left += 31;
			// NEO: NTS END

			buffer.Format(GetResString(IDS_X_PART_ENTRY),part->Part);
			break;
		case 1:
			if(part->KnownFile->IsPartFile() && !((CPartFile*)part->KnownFile)->IsComplete(PARTSIZE*part->Part, PARTSIZE*part->Part+PARTSIZE-1, true))
				buffer=GetResString(IDS_X_PART_NOT_DONE);
			else
				buffer=GetResString(IDS_X_PART_DONE);
			break;
		case 2:
			break;
		// NEO: MCS - [ManualChunkSelection]
		case 3:
			if (part->KnownFile->IsPartFile())
				if(((CPartFile*)part->KnownFile)->PartPrefs->GetWantedPart(part->Part))
					buffer=GetResString(IDS_X_PARTPRIO_WANTED);
				else
					buffer=GetResString(IDS_X_PARTPRIO_DEFAULT);
			break;
		// NEO: MCS END
		case 4:
			buffer =  md4str(part->KnownFile->GetPartHash(part->Part));
			break;
		case 5:
			break;
		// NEO: NPT - [NeoPartTraffic]
		case 6:
			//if(NeoPrefs.UsePartTraffic())
			//	buffer.Format(_T("%i (%i)"), part->KnownFile->statistic.GetPartAccepted(part->Part, true), part->KnownFile->statistic.GetPartAccepted(part->Part, false));
			break;
		case 7:
			if(NeoPrefs.UsePartTraffic())
				buffer.Format(_T("%s (%s)"), CastItoXBytes(part->KnownFile->statistic.GetPartTrafficSession(part->Part)), CastItoXBytes(part->KnownFile->statistic.GetPartTraffic(part->Part)));
			break;
		// NEO: NPT END
		case 8:
			break;
		case 9:
			break;
		case 10:{
			int Shown = part->KnownFile->GetPartAvailibility(part->Part);
			buffer.Format(_T("%u"), Shown);
			break;
		}
		case 11:
			break;
		case 12:
			break;
		// NEO: NPT - [NeoPartTraffic]
		case 13:
			if(NeoPrefs.UsePartTraffic() == 1){
				cur_rec->bottom--;
				cur_rec->top++;
				part->KnownFile->statistic.DrawTrafficStatusBar(dc,cur_rec,false,thePrefs.UseFlatBar(),GetTrafficColor,part->Part); 
				cur_rec->bottom++;
				cur_rec->top--;
			}
			break;
		case 14: 
			if(NeoPrefs.UsePartTraffic()){
				double CompHi = (double)part->KnownFile->statistic.GetPartTraffic(part->Part)/part->KnownFile->GetPartSize(part->Part);
				double CompLo = part->KnownFile->statistic.GetPartRelease(part->Part);
				if (CompLo == 0){
					buffer.Format(_T("< %.2f"), CompHi);
				}else if (CompLo == CompHi){
					buffer.Format(_T("%.2f"), CompLo);
				}else{
					buffer.Format(_T("%.2f - %.2f"), CompLo, CompHi);
				}
			}
			break;
		// NEO: NPT END
		// NEO: MPS - [ManualPartSharing]
		case 15:{
			uint8 status = part->KnownFile->KnownPrefs->GetManagedPart(part->Part);
			switch(status) 
			{
			case PR_PART_NORMAL:
				buffer=GetResString(IDS_X_PARTPRIO_NORMAL);
			case PR_PART_ON:
				buffer=GetResString(IDS_X_PARTPRIO_ON);
				break;
			case PR_PART_HIDEN:
				buffer=GetResString(IDS_X_PARTPRIO_HIDEN);
				break;
			case PR_PART_OFF:
				buffer=GetResString(IDS_X_PARTPRIO_BLOCKED);
				break;
			}

			// NEO: IPS - [InteligentPartSharing]
			if(!part->KnownFile->KnownPrefs->UseInteligentPartSharing())
				break;

			uint8 IPSstatus = part->KnownFile->GetIPSPartStatus(part->Part);
			switch(IPSstatus) 
			{
			case PR_PART_ON:
				buffer.AppendFormat(_T(" [%s]"),GetResString(IDS_X_PARTPRIO_ON));
				break;
			case PR_PART_HIDEN:
				buffer.AppendFormat(_T(" [%s]"),GetResString(IDS_X_PARTPRIO_HIDEN));
				break;
			case PR_PART_OFF:
				buffer.AppendFormat(_T(" [%s]"),GetResString(IDS_X_PARTPRIO_BLOCKED));
				break;
			}

			if(status != PR_PART_NORMAL && IPSstatus != status)
				buffer.Append(_T(" !"));
			// NEO: IPS END

			break;
		}
		// NEO: MPS END
	}

	if (iColumn != 8 && iColumn != 13) // NEO: NPT - [NeoPartTraffic]
		dc->DrawText(buffer, buffer.GetLength(), cur_rec, uDTFlags);

	if (iColumn == 0){
		// NEO: NTS - [NeoTreeStyle]
		if(NeoPrefs.UseTreeStyle())
			cur_rec->left -= 31;
		// NEO: NTS END
	}
}
// NEO: SP END <-- Xanatos --

void CSharedFilesCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	// NEO: SP - [SharedParts] -- Xanatos --
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);

	CSharedItem* pSingleSelItem  = (iSel == -1) ? NULL : (CSharedItem*)GetItemData(iSel);
	if (pSingleSelItem && !pSingleSelItem->isFile){

		bool bFirstItem = true; 
		int iCompletePartSelected = 0; // NEO: PIX - [PartImportExport]
		int iInCompletePartSelected = 0; // NEO: MCS - [ManualChunkSelection]

		UINT uPartPermMenuItem = 0; // NEO: MPS - [ManualPartSharing]

		CKnownFile* pSingleSelKnownFile = pSingleSelItem->KnownFile;
		CPartFile* pSingleSelPartFile = NULL;
		if(pSingleSelKnownFile->IsPartFile())
			pSingleSelPartFile = (CPartFile*)pSingleSelKnownFile;

		bool bFileExist = (theApp.downloadqueue->IsFileExisting(pSingleSelKnownFile->GetFileHash(),false));
		POSITION pos = GetFirstSelectedItemPosition();
		while (pos)
		{
			const CSharedItem* pItem = (CSharedItem*)GetItemData(GetNextSelectedItem(pos));

			if(pItem->isFile)
				continue;

			if(pItem->KnownFile != pSingleSelKnownFile){
				pSingleSelKnownFile = NULL;
				pSingleSelPartFile = NULL;
				bFileExist = false;
				break;
			}

			bool PartComplete = (!pSingleSelPartFile || pSingleSelPartFile->IsComplete(PARTSIZE*pItem->Part, PARTSIZE*pItem->Part+PARTSIZE-1, true)); 
			iCompletePartSelected += PartComplete ? 1 : 0; // NEO: PIX - [PartImportExport]  
			iInCompletePartSelected += PartComplete ? 0 : 1; // NEO: MCS - [ManualChunkSelection]

			// NEO: MPS - [ManualPartSharing]
			UINT uCurPartPerm = 0;
			if (pSingleSelKnownFile->KnownPrefs->GetManagedPart(pItem->Part) == PR_PART_NORMAL)
				uCurPartPerm = MP_PARTNORMAL;
			else if (pSingleSelKnownFile->KnownPrefs->GetManagedPart(pItem->Part) == PR_PART_ON)
				uCurPartPerm = MP_PARTON;
			else if (pSingleSelKnownFile->KnownPrefs->GetManagedPart(pItem->Part) == PR_PART_HIDEN)
				uCurPartPerm = MP_PARTHIDE;
			else if (pSingleSelKnownFile->KnownPrefs->GetManagedPart(pItem->Part) == PR_PART_OFF)
				uCurPartPerm = MP_PARTBLOCK;
			else
				ASSERT(0);

			if (bFirstItem)
				uPartPermMenuItem = uCurPartPerm;
			else if (uPartPermMenuItem != uCurPartPerm)
				uPartPermMenuItem = 0;
			// NEO: MPS END

			bFirstItem = false;
		}

		//CTitleMenu PartMenu;
		CMenuXP PartMenu;// NEO: NMX - [NeoMenuXP] <-- Xanatos --
		PartMenu.CreatePopupMenu();
		PartMenu.AddMenuTitle(GetResString(IDS_X_PARTS), true);
		// NEO: PIX - [PartImportExport]
		PartMenu.AppendMenu(MF_STRING, MP_EXPORT, GetResString(IDS_X_EXPORT_PARTS), _T("PARTEXPORT"));
		PartMenu.EnableMenuItem(MP_EXPORT, (bFileExist && iCompletePartSelected > 0) ? MF_ENABLED : MF_GRAYED);
		// NEO: PIX END
		// NEO: MCS - [ManualChunkSelection]
		PartMenu.AppendMenu(MF_STRING, MP_PARTWANTED, GetResString(IDS_X_PART_WANTED), (pSingleSelPartFile && pSingleSelPartFile->PartPrefs->GetWantedPart(pSingleSelItem->Part)) ? _T("PARTWANTED2") : _T("PARTWANTED"));
		//PartMenu.CheckMenuItem(MP_PARTWANTED, (pSingleSelPartFile && pSingleSelPartFile->PartPrefs.GetWantedPart(pSingleSelItem->Part)) ? MF_CHECKED : MF_UNCHECKED);
		PartMenu.EnableMenuItem(MP_PARTWANTED, (pSingleSelPartFile) ? MF_ENABLED : MF_GRAYED);
		PartMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
		// NEO: MCS END

		// NEO: MPS - [ManualPartSharing]
		CTitleMenu PrioMenu;
		PrioMenu.CreateMenu();
		PrioMenu.AddMenuTitle(NULL, true);
		PrioMenu.AppendMenu(MF_STRING, MP_PARTNORMAL, GetResString(IDS_X_PARTPRIO_NORMAL));
		PrioMenu.AppendMenu(MF_STRING, MP_PARTON, GetResString(IDS_X_PARTPRIO_ON));
		PrioMenu.AppendMenu(MF_STRING, MP_PARTHIDE, GetResString(IDS_X_PARTPRIO_HIDEN));
		PrioMenu.AppendMenu(MF_STRING, MP_PARTBLOCK, GetResString(IDS_X_PARTPRIO_BLOCKED));
		PrioMenu.CheckMenuRadioItem(MP_PARTNORMAL,MP_PARTBLOCK, uPartPermMenuItem, 0);
		PartMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)PrioMenu.m_hMenu, GetResString(IDS_X_PARTPRIO), _T("PARTPRIO"));
		// enable _only_ for complete files !
		PartMenu.EnableMenuItem((UINT_PTR)PrioMenu.m_hMenu, (bFileExist && !pSingleSelPartFile) ? MF_ENABLED : MF_GRAYED); 
		// NEO: MPS 

		GetPopupMenuPos(*this, point);
		PartMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);

		VERIFY( PrioMenu.DestroyMenu() ); // NEO: MPS - [ManualPartSharing]
		VERIFY( PartMenu.DestroyMenu() );

		return;
	}
	// NEO: SP END <-- Xanatos --

	// get merged settings
	bool bFirstItem = true;
	int iSelectedItems = GetSelectedCount();
	int iCompleteFileSelected = -1;
	UINT uPrioMenuItem = 0;
	UINT uPermMenuItem = 0; // NEO: SSP - [ShowSharePermissions] <-- Xanatos --
	int iFilesProtected = 0; // NEO: PP - [PasswordProtection] <-- Xanatos --
	bool bVirtRemove = false; // NEO: VSF - [VirtualSharedFiles] <-- Xanatos --
	//const CKnownFile* pSingleSelFile = NULL;
	const CKnownFile* pSingleSelFile = pSingleSelItem ? pSingleSelItem->KnownFile : NULL; // NEO: SP - [SharedParts] <-- Xanatos --
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos)
	{
		// NEO: SP - [SharedParts] -- Xanatos -->
		const CSharedItem* pItem = (CSharedItem*)GetItemData(GetNextSelectedItem(pos));
		if(!pItem->isFile)
			continue;
		const CKnownFile* pFile = pItem->KnownFile;
		// NEO: SP END <-- Xanatos --

		/*const CKnownFile* pFile = (CKnownFile*)GetItemData(GetNextSelectedItem(pos));
		if (bFirstItem)
			pSingleSelFile = pFile;
		else
			pSingleSelFile = NULL;*/

		int iCurCompleteFile = pFile->IsPartFile() ? 0 : 1;
		if (bFirstItem)
			iCompleteFileSelected = iCurCompleteFile;
		else if (iCompleteFileSelected != iCurCompleteFile)
			iCompleteFileSelected = -1;

		UINT uCurPrioMenuItem = 0;
		if (pFile->IsAutoUpPriority())
			uCurPrioMenuItem = MP_PRIOAUTO;
		else if (pFile->GetUpPriority() == PR_VERYLOW)
			uCurPrioMenuItem = MP_PRIOVERYLOW;
		else if (pFile->GetUpPriority() == PR_LOW)
			uCurPrioMenuItem = MP_PRIOLOW;
		else if (pFile->GetUpPriority() == PR_NORMAL)
			uCurPrioMenuItem = MP_PRIONORMAL;
		else if (pFile->GetUpPriority() == PR_HIGH)
			uCurPrioMenuItem = MP_PRIOHIGH;
		else if (pFile->GetUpPriority() == PR_VERYHIGH)
			uCurPrioMenuItem = MP_PRIOVERYHIGH;
		else
			ASSERT(0);

		if (bFirstItem)
			uPrioMenuItem = uCurPrioMenuItem;
		else if (uPrioMenuItem != uCurPrioMenuItem)
			uPrioMenuItem = 0;

		iFilesProtected += (pFile->IsPWProt()); // NEO: PP - [PasswordProtection] <-- Xanatos --

		// NEO: SSP - [ShowSharePermissions] -- Xanatos -->
		UINT uCurPermMenuItem = 0;
		if (pFile->GetPermissions() == PERM_DEFAULT)
			uCurPermMenuItem = MP_PERMDEFAULT;
		else if (pFile->GetPermissions() == PERM_ALL)
			uCurPermMenuItem = MP_PERMALL;
		else if (pFile->GetPermissions() == PERM_FRIENDS)
			uCurPermMenuItem = MP_PERMFRIENDS;
		else if (pFile->GetPermissions() == PERM_NONE)
			uCurPermMenuItem = MP_PERMNONE;
		else
			ASSERT(0);

		if (bFirstItem)
			uPermMenuItem = uCurPermMenuItem;
		else if (uPermMenuItem != uCurPermMenuItem)
			uPermMenuItem = 0;
		// NEO: SSP END <-- Xanatos --

		// NEO: VSF - [VirtualSharedFiles] -- Xanatos -->
		if (!bVirtRemove) {
			CString virt, fileID;
			fileID.Format(_T("%I64u:%s"), (uint64)pFile->GetFileSize(), EncodeBase16(pFile->GetFileHash(),16));
			bVirtRemove = thePrefs.m_fileToVDir_map.Lookup(fileID, virt);
			if (!bVirtRemove) {
				CString path = pFile->GetPath();
				path.MakeLower();
				path.TrimRight(_T('\\'));
				bVirtRemove = thePrefs.m_dirToVDir_map.Lookup(path, virt);
				if (!bVirtRemove)
					bVirtRemove = thePrefs.m_dirToVDirWithSD_map.Lookup(path, virt);
			}
		}
		// NEO: VSF END <-- Xanatos --

		bFirstItem = false;
	}

	m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_PrioMenu.m_hMenu, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	m_PrioMenu.CheckMenuRadioItem(MP_PRIOVERYLOW, MP_PRIOAUTO, uPrioMenuItem, 0);
	m_PrioMenu.CheckMenuItem(MP_PRIORELEASE, pSingleSelFile && pSingleSelFile->IsReleasePriority() ? MF_CHECKED : MF_UNCHECKED); // NEO: SRS - [SmartReleaseSharing] <-- Xanatos --
	// NEO: SSP - [ShowSharePermissions] -- Xanatos -->
	m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_PermMenu.m_hMenu, (iSelectedItems > 0 && NeoPrefs.UseShowSharePermissions()) ? MF_ENABLED : MF_GRAYED);
	m_PermMenu.CheckMenuRadioItem(MP_PERMALL, MP_PERMDEFAULT, uPermMenuItem, 0);
	// NEO: SSP END <-- Xanatos --
	// NEO: PP - [PasswordProtection] -- Xanatos -->
	m_PWProtMenu.EnableMenuItem(MP_PWPROT_SET, (iSelectedItems > 1 || (iSelectedItems == 1 && !iFilesProtected)) ? MF_ENABLED : MF_GRAYED);
	m_PWProtMenu.EnableMenuItem(MP_PWPROT_CHANGE, (iFilesProtected > 0) ? MF_ENABLED : MF_GRAYED);
	m_PWProtMenu.EnableMenuItem(MP_PWPROT_UNSET, (iFilesProtected > 0) ? MF_ENABLED : MF_GRAYED);
	// NEO: PP END <-- Xanatos --
	// NEO: VSF - [VirtualSharedFiles] -- Xanatos -->
	m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_VirtualDirMenu.m_hMenu, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(MP_IOM_VIRTREMOVE, bVirtRemove ? MF_ENABLED : MF_GRAYED);
	// NEO: VSF END <-- Xanatos --

	m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_ReleaseMenu.m_hMenu, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED); // NEO: MOD - [ReleaseMenu] <-- Xanatos --
	m_ReleaseMenu.EnableMenuItem(MP_RECALC_IPS, pSingleSelFile && pSingleSelFile->KnownPrefs->UseInteligentPartSharing() ? MF_ENABLED : MF_GRAYED); // NEO: IPS - [InteligentPartSharing] <-- Xanatos --

	m_ReleaseMenu.EnableMenuItem(MP_CLEARCOMMENTS, pSingleSelFile && !pSingleSelFile->GetCommentList().IsEmpty() ? MF_ENABLED : MF_GRAYED); // NEO: XCs - [SaveComments] <-- Xanatos --
	// NEO: MPS - [ManualPartSharing]-- Xanatos -->
	m_ReleaseMenu.EnableMenuItem(MP_COPYBLOCK, pSingleSelFile && pSingleSelFile->KnownPrefs->HasManagedParts() ? MF_ENABLED : MF_GRAYED);
	m_ReleaseMenu.EnableMenuItem(MP_PASTEBLOCK, pSingleSelFile && !pSingleSelFile->IsPartFile() ? MF_ENABLED : MF_GRAYED);
	m_ReleaseMenu.EnableMenuItem(MP_CLEARBLOCK, pSingleSelFile && pSingleSelFile->KnownPrefs->HasManagedParts() ? MF_ENABLED : MF_GRAYED);
	// NEO: MPS END <-- Xanatos --
	// NEO: MCS - [ManualChunkSelection] -- Xanatos -->
	m_ReleaseMenu.EnableMenuItem(MP_LINEARWANTED, pSingleSelFile && pSingleSelFile->IsPartFile() ? MF_ENABLED : MF_GRAYED);
	m_ReleaseMenu.EnableMenuItem(MP_CLEARWANTED, pSingleSelFile && pSingleSelFile->IsPartFile() && ((CPartFile*)pSingleSelFile)->PartPrefs->HasWantedParts() ? MF_ENABLED : MF_GRAYED);
	// NEO: MCS END <-- Xanatos --

	bool bSingleCompleteFileSelected = (iSelectedItems == 1 && iCompleteFileSelected == 1);
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
	m_SharedFilesMenu.EnableMenuItem(MP_OPENFOLDER, bSingleCompleteFileSelected ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(MP_RENAME, bSingleCompleteFileSelected ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(MP_MASSRENAME, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED); // NEO: MMR - [MorphMassRemane] <-- Xanatos --
	m_SharedFilesMenu.EnableMenuItem(MP_REMOVE, iCompleteFileSelected > 0 ? MF_ENABLED : MF_GRAYED);
	// NEO: SSF - [ShareSingleFiles] -- Xanatos -->
	bool bCanUnshare = false;
	if(pSingleSelFile)
	{
		for (POSITION pos = thePrefs.sharedfile_list.GetHeadPosition(); pos != 0; ) {
			if(thePrefs.sharedfile_list.GetNext(pos).CompareNoCase(MkPath(pSingleSelFile->GetPath(),pSingleSelFile->GetFileName())))
				continue;
			bCanUnshare = true;
			break;
		}
	}
	m_SharedFilesMenu.EnableMenuItem(MP_UNSHARE_FILE, bCanUnshare ? MF_ENABLED : MF_GRAYED);
	// NEO: SSF END <-- Xanatos --
	m_SharedFilesMenu.SetDefaultItem(bSingleCompleteFileSelected ? MP_OPEN : -1);
	m_SharedFilesMenu.EnableMenuItem(MP_CMT, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(MP_DETAIL, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(MP_TWEAKS, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED); // NEO: FCFG - [FileConfiguration] <-- Xanatos --
	m_SharedFilesMenu.EnableMenuItem(thePrefs.GetShowCopyEd2kLinkCmd() ? MP_GETED2KLINK : MP_SHOWED2KLINK, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(MP_COPYFEEDBACK, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED); // NEO: FB - [FeedBack] <-- Xanatos --
	m_SharedFilesMenu.EnableMenuItem(MP_SEARCHHASH, iSelectedItems == 1 && theApp.emuledlg->searchwnd->CanSearchRelatedFiles() ? MF_ENABLED : MF_GRAYED); // NEO: MOD - [SearchHash] <-- Xanatos --
	m_SharedFilesMenu.EnableMenuItem(MP_FIND, GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED);

	m_CollectionsMenu.EnableMenuItem(MP_MODIFYCOLLECTION, ( pSingleSelFile != NULL && pSingleSelFile->m_pCollection != NULL ) ? MF_ENABLED : MF_GRAYED);
	m_CollectionsMenu.EnableMenuItem(MP_VIEWCOLLECTION, ( pSingleSelFile != NULL && pSingleSelFile->m_pCollection != NULL ) ? MF_ENABLED : MF_GRAYED);
	m_CollectionsMenu.EnableMenuItem(MP_SEARCHAUTHOR, ( pSingleSelFile != NULL && pSingleSelFile->m_pCollection != NULL && !pSingleSelFile->m_pCollection->GetAuthorKeyHashString().IsEmpty()) ? MF_ENABLED : MF_GRAYED);

	m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_CRC32Menu.m_hMenu, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED); // NEO: CRC - [MorphCRCTag] <-- Xanatos --

#if defined(_DEBUG)
	if (thePrefs.IsExtControlsEnabled()){
	//JOHNTODO: Not for release as we need kad lowID users in the network to see how well this work work. Also, we do not support these links yet.
		if (iSelectedItems > 0 && theApp.IsConnected() && theApp.IsFirewalled() && theApp.clientlist->GetBuddy())
			m_SharedFilesMenu.EnableMenuItem(MP_GETKADSOURCELINK, MF_ENABLED);
		else
			m_SharedFilesMenu.EnableMenuItem(MP_GETKADSOURCELINK, MF_GRAYED);
	}
#endif
	m_SharedFilesMenu.EnableMenuItem(Irc_SetSendLink, iSelectedItems == 1 && theApp.emuledlg->ircwnd->IsConnected() ? MF_ENABLED : MF_GRAYED);

	// NEO: NSC - [NeoSharedCategories] -- Xanatos -->
	//CTitleMenu CatsMenu;
	CMenuXP CatsMenu;// NEO: NMX - [NeoMenuXP] <-- Xanatos --
	if(pSingleSelFile){
		CatsMenu.CreateMenu();
		CatsMenu.AddMenuTitle(NULL, true);
		UpdateCatMenu(CatsMenu,pSingleSelFile->GetCategory(),true);
		UINT flag = (thePrefs.GetFullCatCount() == 1) ? MF_GRAYED : MF_ENABLED;
		m_SharedFilesMenu.AppendMenu(MF_POPUP | flag, (UINT_PTR)CatsMenu.m_hMenu, GetResString(IDS_TOCAT), _T("CATEGORY"));
	}
	// NEO: NSC END <-- Xanatos --

	CTitleMenu WebMenu;
	WebMenu.CreateMenu();
	WebMenu.AddMenuTitle(NULL, true);
	int iWebMenuEntries = theWebServices.GetFileMenuEntries(&WebMenu);
	UINT flag2 = (iWebMenuEntries == 0 || iSelectedItems != 1) ? MF_GRAYED : MF_STRING;
	m_SharedFilesMenu.AppendMenu(flag2 | MF_POPUP, (UINT_PTR)WebMenu.m_hMenu, GetResString(IDS_WEBSERVICES), _T("WEB"));

	GetPopupMenuPos(*this, point);
	m_SharedFilesMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON,point.x,point.y,this);

	m_SharedFilesMenu.RemoveMenu(m_SharedFilesMenu.GetMenuItemCount()-1,MF_BYPOSITION);
	// NEO: NSC - [NeoSharedCategories] -- Xanatos -->
	if(iSelectedItems > 0)
		m_SharedFilesMenu.RemoveMenu(m_SharedFilesMenu.GetMenuItemCount()-1,MF_BYPOSITION); 
	// NEO: NSC END <-- Xanatos --
	VERIFY( WebMenu.DestroyMenu() );
	// NEO: NSC - [NeoSharedCategories] -- Xanatos -->
	if(pSingleSelFile > 0)
		VERIFY( CatsMenu.DestroyMenu() );
	// NEO: NSC END <-- Xanatos --
	if (uInsertedMenuItem)
		VERIFY( m_SharedFilesMenu.RemoveMenu(uInsertedMenuItem, MF_BYCOMMAND) );
}

BOOL CSharedFilesCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	wParam = LOWORD(wParam);

	// NEO: SP - [SharedParts] -- Xanatos -->
	CSharedItem* pSingleSelItem  = NULL;
	
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel == -1)
		iSel = GetNextItem(-1, LVIS_SELECTED);
	if (iSel != -1)
		pSingleSelItem  = (CSharedItem*)GetItemData(iSel);

	if (pSingleSelItem && !pSingleSelItem->isFile){

		CTypedPtrList<CPtrList, CSharedItem*> selectedList;
		POSITION pos = GetFirstSelectedItemPosition();
		while (pos != NULL){
			int index = GetNextSelectedItem(pos);
			if (index >= 0){
				CSharedItem* pItem = (CSharedItem*)GetItemData(index);
				if(pItem->isFile) 
					continue;
				if(pItem->KnownFile != pSingleSelItem->KnownFile)
					continue;
				selectedList.AddTail(pItem);
			}
		}

		const CSharedItem* part;
		switch (wParam){
			// NEO: PIX - [PartImportExport]
			case MP_EXPORT:{
				CList<uint16>* ExportList = new CList<uint16>;
				while (!selectedList.IsEmpty())
				{
					part = selectedList.GetHead();
					selectedList.RemoveHead();
					ExportList->AddTail(part->Part);
				}

				pSingleSelItem->KnownFile->ExportParts(ExportList);
			}	break;
			// NEO: PIX END
			// NEO: MCS - [ManualChunkSelection]
			case MP_PARTWANTED:{
				if(!pSingleSelItem->KnownFile->IsPartFile())
					break;

				CPartFile* PartFile = (CPartFile*)pSingleSelItem->KnownFile;

				// get ot create Preferences 
				CPartPreferences* PartPrefs = PartFile->PartPrefs;
				if(!PartPrefs->IsFilePrefs())
					PartPrefs = new CPartPreferencesEx(CFP_FILE);

				// save preferences
				uint8 NewState = (PartPrefs->GetWantedPart(pSingleSelItem->Part) == PR_PART_NORMAL) ? PR_PART_WANTED : PR_PART_NORMAL;
				while (!selectedList.IsEmpty())
				{
					part = selectedList.GetHead();
					selectedList.RemoveHead();
					PartPrefs->SetWantedPart(part->Part, NewState);
					UpdatePart(part);
				}

				// check validiti of the tweaks
				PartPrefs->CheckTweaks();

				// valiate pointers and update (!)
				PartFile->UpdatePartPrefs(PartPrefs);
				
				break;
			}
			// NEO: MCS END
			// NEO: MPS - [ManualPartSharing]
			case MP_PARTNORMAL:
			case MP_PARTON:
			case MP_PARTHIDE:
			case MP_PARTBLOCK:{
				uint8 NewState=0;
				switch (wParam){
				case MP_PARTNORMAL:
					NewState=PR_PART_NORMAL;
					break;
				case MP_PARTON:
					NewState=PR_PART_ON;
					break;
				case MP_PARTHIDE:
					NewState=PR_PART_HIDEN;
					break;
				case MP_PARTBLOCK:
					NewState=PR_PART_OFF;
					break;
				}

				CKnownFile* KnownFile = pSingleSelItem->KnownFile;

				// get ot create Preferences 
				CKnownPreferences* KnownPrefs = KnownFile->KnownPrefs;
				if(!KnownPrefs->IsFilePrefs())
					KnownPrefs = new CKnownPreferencesEx(CFP_FILE);

				// save preferences
				while (!selectedList.IsEmpty())
				{
					part = selectedList.GetHead();
					selectedList.RemoveHead();
					KnownPrefs->SetManagedPart(part->Part, NewState);
					UpdatePart(part);
				}

				// check validiti of the tweaks
				KnownPrefs->CheckTweaks();

				// valiate pointers and update (!)
				KnownFile->UpdateKnownPrefs(KnownPrefs);
				break;
			}
			// NEO: MPS END
		}

		return TRUE;
	}
	// NEO: SP END <-- Xanatos --

	CTypedPtrList<CPtrList, CKnownFile*> selectedList;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos != NULL){
		int index = GetNextSelectedItem(pos);
		if (index >= 0)
		// NEO: SP - [SharedParts] -- Xanatos -->
		{
			CSharedItem* pItem = (CSharedItem*)GetItemData(index);
			if(!pItem->isFile) 
				continue;
			selectedList.AddTail(pItem->KnownFile);
		}
		// NEO: SP END <-- Xanatos --
		//	selectedList.AddTail((CKnownFile*)GetItemData(index));
	}

	if (   wParam == MP_CREATECOLLECTION
		|| wParam == MP_FIND
		|| wParam == MP_PWPROT_HIDE || wParam == MP_PWPROT_SHOW // NEO: PP - [PasswordProtection] <-- Xanatos --
		|| selectedList.GetCount() > 0)
	{
		CKnownFile* file = NULL;
		if (selectedList.GetCount() == 1)
			file = selectedList.GetHead();

		switch (wParam){
			case Irc_SetSendLink:
				if (file)
					theApp.emuledlg->ircwnd->SetSendFileString(CreateED2kLink(file));
				break;
			case MP_GETED2KLINK:{
				CString str;
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					file = selectedList.GetNext(pos);
					if (!str.IsEmpty())
						str += _T("\r\n");
					str += CreateED2kLink(file);
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
					file = selectedList.GetNext(pos);
					if (!str.IsEmpty())
						str += _T("\r\n");
					str += theApp.CreateKadSourceLink(file);
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
					ShellExecute(NULL, _T("open"), file->GetPath(), NULL, NULL, SW_SHOW);
				break; 
			case MP_RENAME:
			case MPG_F2:
				if (file && !file->IsPartFile()){
					InputBox inputbox;
					CString title = GetResString(IDS_RENAME);
					title.Remove(_T('&'));
					inputbox.SetLabels(title, GetResString(IDS_DL_FILENAME), file->GetFileName());
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
						// NEO: AKF - [AllKnownFiles] -- Xanatos -->
						bool bShareFile = theApp.sharedfiles->IsFilePtrInList(file);
						if (bShareFile) 
						// NEO: AKF END <-- Xanatos --
							if (_trename(file->GetFilePath(), newpath) != 0){
								CString strError;
								strError.Format(GetResString(IDS_ERR_RENAMESF), file->GetFilePath(), newpath, _tcserror(errno));
								AfxMessageBox(strError);
								break;
							}

						if (file->IsKindOf(RUNTIME_CLASS(CPartFile)))
						{
							file->SetFileName(newname);
							STATIC_DOWNCAST(CPartFile, file)->SetFullName(newpath); 
						}
						else
						{
							theApp.sharedfiles->RemoveKeywords(file);
							file->SetFileName(newname);
							theApp.sharedfiles->AddKeywords(file);
						}
						if (bShareFile) // NEO: AKF - [AllKnownFiles] <-- Xanatos --
							file->SetFilePath(newpath);
						UpdateFile(file);
					}
				}
				else
					MessageBeep(MB_OK);
				break;
			// NEO: MMR - [MorphMassRemane] -- Xanatos -->
			case MP_MASSRENAME: {
				CMassRenameDialog* MRDialog = new CMassRenameDialog;
				// Add the files to the dialog
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL) {
					CKnownFile*  file = selectedList.GetAt (pos);
					MRDialog->m_FileList.AddTail (file);
					selectedList.GetNext (pos);
				}
				MRDialog->OpenDialog();
			}
			break;
			// NEO: MMR END <-- Xanatos --
			case MP_REMOVE:
			case MPG_DELETE:
			{
				if (IDNO == AfxMessageBox(GetResString(IDS_CONFIRM_FILEDELETE),MB_ICONWARNING | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_YESNO))
					return TRUE;

				SetRedraw(FALSE);
				bool bRemovedItems = false;
				while (!selectedList.IsEmpty())
				{
					CKnownFile* myfile = selectedList.RemoveHead();
					if (!myfile || myfile->IsPartFile())
						continue;
					
					BOOL delsucc = FALSE;
					if (!PathFileExists(myfile->GetFilePath()))
						delsucc = TRUE;
					else{
						// Delete
						if (!thePrefs.GetRemoveToBin()){
							delsucc = DeleteFile(myfile->GetFilePath());
						}
						else{
							// delete to recycle bin :(
							TCHAR todel[MAX_PATH+1];
							memset(todel, 0, sizeof todel);
							_tcsncpy(todel, myfile->GetFilePath(), ARRSIZE(todel)-2);

							SHFILEOPSTRUCT fp = {0};
							fp.wFunc = FO_DELETE;
							fp.hwnd = theApp.emuledlg->m_hWnd;
							fp.pFrom = todel;
							fp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;// | FOF_NOERRORUI
							delsucc = (SHFileOperation(&fp) == 0);
						}
					}
					if (delsucc){
						theApp.sharedfiles->RemoveFile(myfile);
						bRemovedItems = true;
						if (myfile->IsKindOf(RUNTIME_CLASS(CPartFile)))
							theApp.emuledlg->transferwnd->downloadlistctrl.ClearCompleted(static_cast<CPartFile*>(myfile));
						// NEO: AKF - [AllKnownFiles] -- Xanatos -->
						if (IsKnownList()){
							RemoveFile(myfile);
							theApp.knownfiles->RemoveFile(myfile);
						}
						// NEO: AKF END <-- Xanatos --
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
					theApp.emuledlg->sharedfileswnd->ShowSelectedFilesSummary();
				}
				break; 
			}
			// NEO: SSF - [ShareSingleFiles] -- Xanatos -->
			case MP_UNSHARE_FILE: {
				if (IDNO == AfxMessageBox(GetResString(IDS_X_CONFIRM_FILEUSHARE),MB_ICONWARNING | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_YESNO))
					return TRUE;

				SetRedraw(FALSE);
				bool bRemovedItems = false;
				while (!selectedList.IsEmpty())
				{
					CKnownFile* myfile = selectedList.RemoveHead();
					if (!myfile || myfile->IsPartFile())
						continue;
					
					CString sFile = MkPath(myfile->GetPath(), myfile->GetFileName());
					bool bCanUnshare = false;
					POSITION remPos = NULL;
					for (POSITION pos = thePrefs.sharedfile_list.GetHeadPosition(); pos != 0; ) {
						remPos = pos;
						if(thePrefs.sharedfile_list.GetNext(pos).CompareNoCase(sFile))
							continue;
						bCanUnshare = true;
						break;
					}

					if (bCanUnshare){
						thePrefs.sharedfile_list.RemoveAt(remPos);
						theApp.sharedfiles->RemoveFile(myfile);
						bRemovedItems = true;
						if (myfile->IsKindOf(RUNTIME_CLASS(CPartFile)))
							theApp.emuledlg->transferwnd->downloadlistctrl.ClearCompleted(static_cast<CPartFile*>(myfile));
						thePrefs.Save();
					}
					else{
						CString strError;
						strError.Format(GetResString(IDS_X_ERR_UNSHARE), myfile->GetFilePath());
						AfxMessageBox(strError);
					}
				}
				SetRedraw(TRUE);
				if (bRemovedItems) {
					AutoSelectItem();
					// Depending on <no-idea> this does not always cause a
					// LVN_ITEMACTIVATE message sent. So, explicitly redraw
					// the item.
					theApp.emuledlg->sharedfileswnd->ShowSelectedFilesSummary();
				}
                break; 
			}
			// NEO: SSF END <-- Xanatos --
			case MP_CMT:
				ShowFileDialog(selectedList, IDD_COMMENT);
                break; 
			case MPG_ALTENTER:
			case MP_DETAIL:
				ShowFileDialog(selectedList);
				break;
			// NEO: FCFG - [FileConfiguration] -- Xanatos -->
			case MP_TWEAKS:
				ShowFileDialog(selectedList,NULL,TRUE);
				break; 
			// NEO: FCFG END <-- Xanatos --
			case MP_FIND:
				OnFindStart();
				break;
			case MP_CREATECOLLECTION:
			{
				CCollection* pCollection = new CCollection();
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					pCollection->AddFileToCollection(selectedList.GetNext(pos),true);
				}
				// NEO: MLD - [ModelesDialogs] -- Xanatos -->
				CCollectionCreateDialog* dialog = new CCollectionCreateDialog(); 
				dialog->SetCollection(pCollection,true);
				dialog->OpenDialog();
				// NEO: MLD END <-- Xanatos --
				//CCollectionCreateDialog dialog;
				//dialog.SetCollection(pCollection,true);
				//dialog.DoModal();
				//We delete this collection object because when the newly created
				//collection file is added to the sharedfile list, it is read and verified
				//and which creates the colleciton object that is attached to that file..
				//delete pCollection;
				break;
			}
			case MP_SEARCHAUTHOR:
				if (selectedList.GetCount() == 1 && file->m_pCollection)
				{
					SSearchParams* pParams = new SSearchParams;
					pParams->strExpression = file->m_pCollection->GetCollectionAuthorKeyString();
					pParams->eType = SearchTypeKademlia;
					pParams->strFileType = ED2KFTSTR_EMULECOLLECTION;
					pParams->strSpecialTitle = file->m_pCollection->m_sCollectionAuthorName;
					if (pParams->strSpecialTitle.GetLength() > 50){
						pParams->strSpecialTitle = pParams->strSpecialTitle.Left(50) + _T("...");
					}
					theApp.emuledlg->searchwnd->m_pwndResults->StartSearch(pParams);
				}
				break;
			case MP_VIEWCOLLECTION:
				if (selectedList.GetCount() == 1 && file->m_pCollection)
				{
					// NEO: MLD - [ModelesDialogs] -- Xanatos -->
					CCollectionViewDialog* dialog = new CCollectionViewDialog(); 
					dialog->SetCollection(file->m_pCollection);
					dialog->OpenDialog();
					// NEO: MLD END <-- Xanatos --
					//CCollectionViewDialog dialog;
					//dialog.SetCollection(file->m_pCollection);
					//dialog.DoModal();
				}
				break;
			case MP_MODIFYCOLLECTION:
				if (selectedList.GetCount() == 1 && file->m_pCollection)
				{
					// NEO: MLD - [ModelesDialogs] -- Xanatos -->
					CCollectionCreateDialog* dialog = new CCollectionCreateDialog(); 
					CCollection* pCollection = new CCollection(file->m_pCollection);
					dialog->SetCollection(pCollection,false);
					dialog->OpenDialog();
					// NEO: MLD END <-- Xanatos --	
					//CCollectionCreateDialog dialog;
					//CCollection* pCollection = new CCollection(file->m_pCollection);
					//dialog.SetCollection(pCollection,false);
					//dialog.DoModal();
					//delete pCollection;				
				}
				break;
			case MP_SHOWED2KLINK:
				ShowFileDialog(selectedList, IDD_ED2KLINK);
				break;
			// NEO: CI#3 - [CodeImprovement]  -- Xanatos -->
			case MP_PRIOVERYLOW:
			case MP_PRIOLOW:
			case MP_PRIONORMAL:
			case MP_PRIOHIGH:
			case MP_PRIOVERYHIGH:
			case MP_PRIOAUTO:
				{
					uint8 uPriority = PR_NORMAL;
					switch (wParam) {
						case MP_PRIOVERYLOW:	uPriority = PR_VERYLOW;		break;
						case MP_PRIOLOW:		uPriority = PR_LOW;			break;
						case MP_PRIONORMAL:		uPriority = PR_NORMAL;		break;
						case MP_PRIOHIGH:		uPriority = PR_HIGH;		break;
						case MP_PRIOVERYHIGH:	uPriority = PR_VERYHIGH;	break;
						case MP_PRIOAUTO:		uPriority = PR_AUTO;		break;
					}

					POSITION pos = selectedList.GetHeadPosition();
					while (pos != NULL)
					{
						CKnownFile* file = selectedList.GetNext(pos);
						if(uPriority == PR_AUTO){
							file->SetAutoUpPriority(true);
							file->UpdateAutoUpPriority();
						}else{
							file->SetAutoUpPriority(false);
							file->SetUpPriority(uPriority);
						}
						UpdateFile(file);
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
						if(!file->IsVoodooFile() && file->KnownPrefs->IsEnableVoodoo())
							theApp.voodoo->ManifestShareInstruction(file,INST_UL_PRIO,uPriority);
#endif // VOODOO // NEO: VOODOO END
					}
					break;
				}
			// NEO: CI#3 EMD <-- Xanatos --
			// NEO: SRS - [SmartReleaseSharing] -- Xanatos -->
			case MP_PRIORELEASE:{
				SetRedraw(false);
				bool bNewState = !selectedList.GetHead()->IsReleasePriority();
				while (!selectedList.IsEmpty()){
					file = selectedList.GetHead();
					file->SetReleasePriority(bNewState);
					selectedList.RemoveHead();
				}
				SetRedraw(true);
				break;
			}
			// NEO: SRS END <-- Xanatos --
			// NEO: SSP - [ShowSharePermissions] -- Xanatos -->
			case MP_PERMNONE:
			case MP_PERMFRIENDS:
			case MP_PERMALL: 
			case MP_PERMDEFAULT: 
			{
				SetRedraw(false);
				while(!selectedList.IsEmpty()) { 
					CKnownFile *file = selectedList.GetHead();
					switch (wParam)
					{
						case MP_PERMNONE:
							file->SetPermissions(PERM_NONE);
							break;
						case MP_PERMFRIENDS:
							file->SetPermissions(PERM_FRIENDS);
							break;
						case MP_PERMALL:
							file->SetPermissions(PERM_ALL);
							break;
						case MP_PERMDEFAULT:
							file->SetPermissions(PERM_DEFAULT);
							break;								
					}
					selectedList.RemoveHead();
				}
				SetRedraw(true);
				break;
			}
			// NEO: SSP END <-- Xanatos --
			// NEO: PP - [PasswordProtection] -- Xanatos -->
			case MP_PWPROT_SET:
				{
					InputBox inputbox1, inputbox2;
					CString pass1 = _T(""), pass2 = _T("");
					MD5Sum makeMD5Sum;

					inputbox1.SetLabels(GetResString(IDS_X_PWPROT_SETIBTITLE),GetResString(IDS_X_PWPROT_SETIBDESC), _T(""));
					//inputbox1.SetPassword(true);
					if (inputbox1.DoModal() == IDOK)
						pass1=inputbox1.GetInput();
					else 
						break;

					while(!selectedList.IsEmpty()) { 
						CKnownFile *file = selectedList.GetHead();
						if ( !file->IsPWProt() ) {
							file->SetPWProtShow(true);
							file->SetPWProt( (pass1.GetLength() == 0) ? _T("") : makeMD5Sum.Calculate( pass1 ) );
						}
					UpdateFile(file);
					selectedList.RemoveHead();
					}
					break;
				}
			case MP_PWPROT_CHANGE:
				{
					InputBox inputbox1, inputbox2;
					CString pass1 = _T(""), pass2 = _T("");
					MD5Sum makeMD5Sum;

					inputbox1.SetLabels(GetResString(IDS_X_PWPROT_OLDPWIBTITLE),GetResString(IDS_X_PWPROT_OLDPWIBDESC), _T(""));
					inputbox1.SetPassword(true);
					if (inputbox1.DoModal() == IDOK) {
						pass1=inputbox1.GetInput();
						if (pass1.GetLength() == 0)
							break;
					} else 
						break;

					inputbox2.SetLabels(GetResString(IDS_X_PWPROT_SETIBTITLE),GetResString(IDS_X_PWPROT_SETIBDESC), _T(""));
					//inputbox2.SetPassword(true);
					if (inputbox2.DoModal() == IDOK){
						pass2=inputbox2.GetInput();
						if (pass2.GetLength() == 0)
							if(AfxMessageBox(GetResString(IDS_X_PWPROT_CLEARPW), MB_YESNO|MB_ICONQUESTION) == IDNO)
								break;
					}

					while(!selectedList.IsEmpty()) { 
						CKnownFile *file = selectedList.GetHead();
						if ( file->IsPWProt() ) {
							if ( makeMD5Sum.Calculate( pass1 ) == file->GetPWProt() ) {
								file->SetPWProtShow(true);
								file->SetPWProt( (pass2.GetLength() == 0) ? _T("") : makeMD5Sum.Calculate( pass2 ) );
							}
						}
					UpdateFile(file);
					selectedList.RemoveHead();
					}
					break;
				}
			case MP_PWPROT_UNSET:
				{
					InputBox inputbox1;
					CString pass1 = _T("");
					MD5Sum makeMD5Sum;

					inputbox1.SetLabels(GetResString(IDS_X_PWPROT_SHOWIBTITLE),GetResString(IDS_X_PWPROT_UNSETIBDESC), _T(""));
					inputbox1.SetPassword(true);
					if (inputbox1.DoModal() == IDOK) {
						pass1=inputbox1.GetInput();
						if (pass1.GetLength() == 0)
							break;
					} else 
						break;

					while(!selectedList.IsEmpty()) { 
						CKnownFile *file = selectedList.GetHead();
						if ( file->IsPWProt() ) {
							if ( makeMD5Sum.Calculate( pass1 ) == file->GetPWProt()) {
								file->SetPWProtShow(true);
								file->SetPWProt( _T("") );
							}
						}
					UpdateFile(file);
					selectedList.RemoveHead();
					}
					break;
				}
				case MP_PWPROT_SHOW:
				{
					SetRedraw(false);
					InputBox inputbox;
					inputbox.SetLabels(GetResString(IDS_X_PWPROT_SHOWIBTITLE),GetResString(IDS_X_PWPROT_SHOWIBDESC), _T(""));
					inputbox.SetPassword(true);
					inputbox.DoModal();
					CString pass=inputbox.GetInput();
					if (!inputbox.WasCancelled() /*&& pass.GetLength()>0*/){ // for voodoo
						MD5Sum makeMD5Sum( pass );
						pass = makeMD5Sum.GetHash();
						CCKey bufKey;
						CKnownFile* cur_file;
						if(!m_ShowAllKnow){ 
							for (POSITION pos = theApp.sharedfiles->m_Files_map.GetStartPosition(); pos != 0; ){
								theApp.sharedfiles->m_Files_map.GetNextAssoc(pos, bufKey, cur_file);
								if (cur_file->GetPWProt() == pass){
									cur_file->SetPWProtShow(true);
									AddFile(cur_file);
									if(cur_file->IsPartFile())
										theApp.emuledlg->transferwnd->downloadlistctrl.ShowFile(((CPartFile*)cur_file));
								}
							}
						}else{
							for (POSITION pos = theApp.knownfiles->m_Files_map.GetStartPosition(); pos != 0; ){
								theApp.knownfiles->m_Files_map.GetNextAssoc(pos, bufKey, cur_file);
								if (cur_file->GetPWProt() == pass){
									cur_file->SetPWProtShow(true);
									AddFile(cur_file);
									if(cur_file->IsPartFile())
										theApp.emuledlg->transferwnd->downloadlistctrl.ShowFile(((CPartFile*)cur_file));
								}
							}
						}
					}
					SetRedraw(true);
					break;
				}
				case MP_PWPROT_HIDE: 
				{
					SetRedraw(false);
					CCKey bufKey;
					CKnownFile* cur_file;
					if(!m_ShowAllKnow){ 
						for (POSITION pos = theApp.sharedfiles->m_Files_map.GetStartPosition(); pos != 0; ){
							theApp.sharedfiles->m_Files_map.GetNextAssoc(pos, bufKey, cur_file);
							if(cur_file->IsPWProt()){
								cur_file->SetPWProtShow(false);
								RemoveFile(cur_file);
								if(cur_file->IsPartFile())
									theApp.emuledlg->transferwnd->downloadlistctrl.HideFile(((CPartFile*)cur_file));
							}
						}
					}else{
						for (POSITION pos = theApp.knownfiles->m_Files_map.GetStartPosition(); pos != 0; ){
							theApp.knownfiles->m_Files_map.GetNextAssoc(pos, bufKey, cur_file);
							if(cur_file->IsPWProt()){
								cur_file->SetPWProtShow(false);
								RemoveFile(cur_file);
								if(cur_file->IsPartFile())
									theApp.emuledlg->transferwnd->downloadlistctrl.HideFile(((CPartFile*)cur_file));
							}
						}
					}
					SetRedraw(true);
					break;
				}
			// NEO: PP END <-- Xanatos --
			// NEO: VSF - [VirtualSharedFiles] -- Xanatos -->
			case MP_IOM_VIRTFILE:
			case MP_IOM_VIRTDIR: 
			case MP_IOM_VIRTSUBDIR:
				{
					InputBox input;
					CString title;
					file = selectedList.GetHead();
					switch (wParam) {
						case MP_IOM_VIRTFILE:
							title.Format(GetResString(IDS_X_VDS_CHANGEMAP), file->GetFileName());
							input.SetLabels(title,GetResString(IDS_X_VDS_VIRTUALFILE),selectedList.GetHead()->GetPath(true));
							break;
						case MP_IOM_VIRTDIR:
							title.Format(GetResString(IDS_X_VDS_CHANGEMAP), file->GetPath());
							input.SetLabels(title,GetResString(IDS_X_VDS_VIRTUALDIR),selectedList.GetHead()->GetPath(true));
							break;
						case MP_IOM_VIRTSUBDIR:
							title.Format(GetResString(IDS_X_VDS_CHANGEMAP), file->GetPath());
							input.SetLabels(title,GetResString(IDS_X_VDS_VIRTUALSUBDIR),selectedList.GetHead()->GetPath(true));
							break;
					}
					input.DoModal();
					CString output = input.GetInput();
					if (!input.WasCancelled() && output.GetLength()>0) {
						output.MakeLower();
						output.TrimRight(_T('\\'));
						POSITION pos = selectedList.GetHeadPosition();
						while (pos != NULL)
						{
							CKnownFile* file = selectedList.GetNext(pos);
							CString fileID;
							CString path = file->GetPath();
							path.MakeLower();
							path.TrimRight(_T('\\'));
							fileID.Format(_T("%I64u:%s"), (uint64)file->GetFileSize(), EncodeBase16(file->GetFileHash(),16));
							if (wParam == MP_IOM_VIRTFILE)
								thePrefs.m_fileToVDir_map.SetAt(fileID, output);
							else if (wParam == MP_IOM_VIRTDIR)
								thePrefs.m_dirToVDir_map.SetAt(path, output);
							else if (wParam == MP_IOM_VIRTSUBDIR)
								thePrefs.m_dirToVDirWithSD_map.SetAt(path, output);
						}
					}
					theApp.emuledlg->sharedfileswnd->Invalidate(false);
					theApp.emuledlg->sharedfileswnd->UpdateWindow();
					if (theApp.sharedfiles) 
						theApp.sharedfiles->ShowLocalFilesDialog(true);
					break;
				}
			case MP_IOM_VIRTREMOVE:
				{
					POSITION pos = selectedList.GetHeadPosition();
					while (pos != NULL)
					{
						CKnownFile* file = selectedList.GetNext(pos);
						CString virt, fileID;
						CString path = file->GetPath();
						path.MakeLower();
						path.TrimRight(_T('\\'));
						fileID.Format(_T("%I64u:%s"), (uint64)file->GetFileSize(), EncodeBase16(file->GetFileHash(),16));
						if (thePrefs.m_fileToVDir_map.Lookup(fileID, virt))
							thePrefs.m_fileToVDir_map.RemoveKey(fileID);
						if (thePrefs.m_dirToVDir_map.Lookup(path, virt))
							thePrefs.m_dirToVDir_map.RemoveKey(path);
						if (thePrefs.m_dirToVDirWithSD_map.Lookup(path, virt))
							thePrefs.m_dirToVDirWithSD_map.RemoveKey(path);
					}
					theApp.emuledlg->sharedfileswnd->Invalidate(false);
					theApp.emuledlg->sharedfileswnd->UpdateWindow();
					if (theApp.sharedfiles) 
						theApp.sharedfiles->ShowLocalFilesDialog(true);
					break;
				}
			//case MP_IOM_VIRTPREFS:
			//	theApp.emuledlg->ShowPreferences(IDD_PPG_VIRTUAL);
			//	break;
			// NEO: VSF END <-- Xanatos --
			// NEO: CRC - [MorphCRCTag] -- Xanatos -->
			case MP_CRC32_RECALCULATE: 
				// Remove existing CRC32 tags from the selected files
				if (!selectedList.IsEmpty()){
					POSITION pos = selectedList.GetHeadPosition();
					while (pos != NULL) {
						CKnownFile* file = selectedList.GetAt (pos);
						memset(file->GetCalculatedCRC32rw(),0,sizeof(BYTE)*4);
						//UpdateFile(file);
						selectedList.GetNext (pos);
					}
				}
				// Repaint the list 
				Invalidate();
				// !!! NO "break;" HERE !!!
				// This case branch must lead into the MP_CRC32_CALCULATE branch - 
				// so after removing the CRC's from the selected files they
				// are immediately recalculated!
			case MP_CRC32_CALCULATE: 
				if (!selectedList.IsEmpty()){
					// For every chosen file create a worker thread and add it
					// to the file processing thread
					POSITION pos = selectedList.GetHeadPosition();
					while (pos != NULL) {
						CCRC32CalcWorker* worker = new CCRC32CalcWorker;
						CKnownFile*  file = selectedList.GetAt (pos);
						const uchar* FileHash = file->GetFileHash ();
						worker->SetFileHashToProcess (FileHash);
						worker->SetFilePath (file->GetFilePath ());
						m_FileProcessingThread.AddFileProcessingWorker (worker);
						selectedList.GetNext (pos);
					}
					// Start the file processing thread to process the files.
					if (!m_FileProcessingThread.IsRunning ()) {
						// (Re-)start the thread, this will do the rest

						// If the thread object already exists, this will result in an
						// ASSERT - but that doesn't matter since we manually take care
						// that the thread does not run at this moment...
						m_FileProcessingThread.CreateThread ();
					}
				}
				break;
			case MP_CRC32_ABORT:
				// Message the File processing thread to stop any pending calculations
				if (m_FileProcessingThread.IsRunning ())
					m_FileProcessingThread.Terminate ();
				break;
			case MP_CRC32_TAG:
				if (!selectedList.IsEmpty()){
					// NEO: MLD - [ModelesDialogs]
					AddCRC32InputBox* AddCRCDialog = new AddCRC32InputBox;
					// Add the files to the dialog
					POSITION pos = selectedList.GetHeadPosition();
					while (pos != NULL) {
						CKnownFile*  file = selectedList.GetAt (pos);
						AddCRCDialog->m_FileList.AddTail (file);
						selectedList.GetNext (pos);
					}
					AddCRCDialog->OpenDialog();
					// NEO: MLD END
				}
				break;
			// NEO: CRC END <-- Xanatos --
			// NEO: MOD - [ReleaseMenu] -- Xanatos -->
			case MP_RECALC_PS:
				{
					while (!selectedList.IsEmpty())
					{
						file = selectedList.GetHead();
						selectedList.RemoveHead();
						file->m_nCompleteSourcesTime = 0; // force update
						file->UpdatePartsInfo();
						UpdateFile(file);
					}
					break;
				} 
			// NEO: MOD END <-- Xanatos --
			// NEO: IPS - [InteligentPartSharing] -- Xanatos -->
			case MP_RECALC_IPS:
				{
					while (!selectedList.IsEmpty())
					{
						file = selectedList.GetHead();
						selectedList.RemoveHead();
						file->CalculateIPS();
						UpdateFile(file);
					}
					break;
				} 
			// NEO: IPS END <-- Xanatos --
			// NEO: NPT - [NeoPartTraffic] -- Xanatos -->
			case MP_CLEARSTATS:
			case MP_CLEARALLSTATS:
				{
					bool all=(wParam==MP_CLEARALLSTATS);
					if (IDNO == AfxMessageBox(GetResString(all ? IDS_X_CONFIRM_RESET_ALL_TRAFFIC : IDS_X_CONFIRM_RESET_TRAFFIC),MB_ICONWARNING | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_YESNO))
						break;

					while (!selectedList.IsEmpty())
					{
						file = selectedList.GetHead();
						selectedList.RemoveHead();
						file->statistic.ResetStats(all);
						UpdateFile(file);
					}
					break;
				} 
			// NEO: NPT END <-- Xanatos --
			// NEO: MPS - [ManualPartSharing] -- Xanatos -->
			case MP_COPYBLOCK:
				{
					if(!selectedList.IsEmpty())
					{
						file = selectedList.GetHead();
						selectedList.RemoveHead();

						CKnownPreferences* KnownPrefs = file->KnownPrefs;
						if(!KnownPrefs->IsFilePrefs())
							break; // no custom prefs

						CIni ini;
						ini.SetSection(_T("BlockedParts"));

						for(UINT Part = 0;Part < file->GetPartCount(); Part++){
							uint8 status = 0;
							if(file->KnownPrefs->m_ManagedParts.Lookup(Part,status))
								ini.WriteInt(StrLine(_T("Part%d"),Part),status);
						}

						theApp.CopyTextToClipboard(ini.GetBuffer());
					}
					break;
				} 
			case MP_PASTEBLOCK:
				{
					while (!selectedList.IsEmpty())
					{
						file = selectedList.GetHead();
						selectedList.RemoveHead();

						CKnownPreferences* KnownPrefs = file->KnownPrefs;
						if(file->IsPartFile())
							continue; // not allowed

						if(!KnownPrefs->IsFilePrefs())
							KnownPrefs = new CKnownPreferencesEx(CFP_FILE);
						else
							KnownPrefs->m_ManagedParts.RemoveAll(); // reset old prefs

						CIni ini;
						ini.SetBuffer(theApp.CopyTextFromClipboard());
						ini.SetSection(_T("BlockedParts"));

						for(UINT Part = 0;Part < file->GetPartCount(); Part++){
							uint8 status = (uint8)ini.GetInt(StrLine(_T("Part%d"),Part),0);
							if(status)
								KnownPrefs->m_ManagedParts.SetAt(Part,status);
						}

						// valiate pointers and update (!)
						file->UpdateKnownPrefs(KnownPrefs);

						UpdateFile(file);
					}
					break;
				} 
			case MP_CLEARBLOCK:
				{
					while (!selectedList.IsEmpty())
					{
						file = selectedList.GetHead();
						selectedList.RemoveHead();

						CKnownPreferences* KnownPrefs = file->KnownPrefs;
						if(!KnownPrefs->IsFilePrefs())
							break; // nothing to do here

						// clear
						KnownPrefs->m_ManagedParts.RemoveAll();

						// valiate pointers and update (!)
						file->UpdateKnownPrefs(KnownPrefs); // may remove the file prtefs

						UpdateFile(file);
					}
					break;
				} 
			// NEO: MPS END <-- Xanatos --
			// NEO: MCS - [ManualChunkSelection] -- Xanatos -->
			case MP_LINEARWANTED:
				{
					while (!selectedList.IsEmpty())
					{
						file = selectedList.GetHead();
						selectedList.RemoveHead();

						if(!file->IsPartFile())
							continue; // not allowed

						CPartPreferences* PartPrefs = ((CPartFile*)file)->PartPrefs;

						if(!PartPrefs->IsFilePrefs())
							PartPrefs = new CPartPreferencesEx(CFP_FILE);
						else
							PartPrefs->m_WantedParts.RemoveAll(); // reset old prefs

						for(UINT Part = 0;Part < file->GetPartCount(); Part++)
							PartPrefs->m_WantedParts.SetAt(Part,PR_PART_WANTED);

						// valiate pointers and update (!)
						((CPartFile*)file)->UpdatePartPrefs(PartPrefs);

						UpdateFile(file);
					}
					break;
				} 
			case MP_CLEARWANTED:
				{
					while (!selectedList.IsEmpty())
					{
						file = selectedList.GetHead();
						selectedList.RemoveHead();

						if(!file->IsPartFile())
							continue; // not allowed

						CPartPreferences* PartPrefs = ((CPartFile*)file)->PartPrefs;
						if(!PartPrefs->IsFilePrefs())
							break; // nothing to do here

						// clear
						PartPrefs->m_WantedParts.RemoveAll();

						// valiate pointers and update (!)
						((CPartFile*)file)->UpdatePartPrefs(PartPrefs); // may remove the file prtefs

						UpdateFile(file);
					}
					break;
				} 
			// NEO: MCS END <-- Xanatos --
			// NEO: XCs - [SaveComments] -- Xanatos -->
			case MP_CLEARCOMMENTS:
				{
					if (IDNO == AfxMessageBox(GetResString(IDS_X_CONFIRM_RESET_CONNENTS),MB_ICONWARNING | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_YESNO))
						break;

					while (!selectedList.IsEmpty())
					{
						file = selectedList.GetHead();
						selectedList.RemoveHead();
						file->ClearCommentList();
						UpdateFile(file);
					}
					break;
				}
			// NEO: XCs END <-- Xanatos --
			// NEO: MOD - [SearchHash] -- Xanatos -->
			case MP_SEARCHHASH:
				if (file == NULL)
					break;
				theApp.emuledlg->searchwnd->SearchRelatedFiles(selectedList, true);
				theApp.emuledlg->SetActiveDialog(theApp.emuledlg->searchwnd);
				break;
			// NEO: MOD <-- Xanatos --
			// NEO: FB - [FeedBack] -- Xanatos -->
			case MP_COPYFEEDBACK:
			{
				CString feed;
				POSITION pos = selectedList.GetHeadPosition();

				WORD wLanguageID = thePrefs.GetLanguageID();
				if(wLanguageID != LANGID_EN_US && IDYES == AfxMessageBox(GetResString(IDS_X_COPYFEEDBACK_US),MB_ICONQUESTION | MB_YESNO))
					wLanguageID = LANGID_EN_US;

				feed.AppendFormat(GetResString(IDS_X_FEEDBACK_FROM,wLanguageID), thePrefs.GetUserNick(), theApp.m_strNeoVersionLong); feed.AppendFormat(_T(" \r\n"));
				while (pos != NULL)
				{
					CKnownFile* file = selectedList.GetNext(pos);
					feed.AppendFormat(GetResString(IDS_X_FEEDBACK_FILENAME,wLanguageID), file->GetFileName()); feed.Append(_T(" \r\n"));
					feed.AppendFormat(GetResString(IDS_X_FEEDBACK_FILETYPE,wLanguageID), file->GetFileType()); feed.Append(_T(" \r\n"));
					feed.AppendFormat(GetResString(IDS_X_FEEDBACK_FILESIZE,wLanguageID), CastItoXBytes(file->GetFileSize(),false,false,3)); feed.Append(_T(" \r\n"));
					feed.AppendFormat(GetResString(IDS_X_FEEDBACK_DOWNLOADED,wLanguageID), (file->IsPartFile()==false)?GetResString(IDS_COMPLETE,wLanguageID):CastItoXBytes(((CPartFile*)file)->GetCompletedSize(),false,false,3));  feed.Append(_T(" \r\n"));
					feed.AppendFormat(GetResString(IDS_X_FEEDBACK_TRANSFERRED,wLanguageID), CastItoXBytes(file->statistic.GetTransferred(),false,false,3),CastItoXBytes(file->statistic.GetAllTimeTransferred(),false,false,3)); feed.Append(_T(" \r\n"));
					feed.AppendFormat(GetResString(IDS_X_FEEDBACK_REQUESTED,wLanguageID), file->statistic.GetRequests(), file->statistic.GetAllTimeRequests()); feed.Append(_T(" \r\n"));
					feed.AppendFormat(GetResString(IDS_X_FEEDBACK_ACCEPTED,wLanguageID), file->statistic.GetAccepts() , file->statistic.GetAllTimeAccepts()); feed.Append(_T(" \r\n"));
					if(file->IsPartFile()){
						feed.AppendFormat(GetResString(IDS_X_FEEDBACK_TOTAL,wLanguageID), ((CPartFile*)file)->GetSourceCount()); feed.Append(_T(" \r\n"));
						feed.AppendFormat(GetResString(IDS_X_FEEDBACK_AVAILABLE,wLanguageID), ((CPartFile*)file)->GetAvailableSrcCount()); feed.Append(_T(" \r\n"));
						feed.AppendFormat(GetResString(IDS_X_FEEDBACK_NONEEDPART,wLanguageID), ((CPartFile*)file)->GetSrcStatisticsValue(DS_NONEEDEDPARTS)); feed.Append(_T(" \r\n"));
					}
					feed.AppendFormat(GetResString(IDS_X_FEEDBACK_COMPLETE,wLanguageID), file->m_nCompleteSourcesCount); feed.Append(_T(" \r\n"));
					feed.Append(_T(" \r\n"));
				}
				//Todo: copy all the comments too
				theApp.CopyTextToClipboard(feed);
				break;
			}
			// NEO: FB END <-- Xanatos --
			default:
				if (wParam>=MP_WEBURL && wParam<=MP_WEBURL+99){ // NEO: FIX - [MP_WEBURL] <-- Xanatos --
					theWebServices.RunURL(file, wParam);
				}
				// NEO: NSC - [NeoSharedCategories] -- Xanatos -->
				else if (wParam>=MP_ASSIGNCAT && wParam<=MP_ASSIGNCAT+99){
					SetRedraw(FALSE);
					while (!selectedList.IsEmpty()){
						CKnownFile *knownfile = selectedList.GetHead();
						knownfile->SetCategory(wParam - MP_ASSIGNCAT);
						//knownfile->UpdateDisplayedInfo(true);
						selectedList.RemoveHead();
					}
					SetRedraw(TRUE);
					ReloadFileList(); //UpdateCurrentCategoryView();
					if (thePrefs.ShowCatTabInfos())
						theApp.emuledlg->transferwnd->UpdateCatTabTitles();
				}
				// NEO: NSC END <-- Xanatos --
				break;
		}
	}
	return TRUE;
}

void CSharedFilesCtrl::OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// Barry - Store sort order in preferences
	// Determine ascending based on whether already sorted on this column

	bool sortAscending = (GetSortItem() != pNMListView->iSubItem) ? true : !GetSortAscending();

	// Ornis 4-way-sorting
	int adder=0;
	if (pNMListView->iSubItem>=5 && pNMListView->iSubItem<=7)
	{
		ASSERT( pNMListView->iSubItem - 5 < ARRSIZE(sortstat) );
		if (!sortAscending)
			sortstat[pNMListView->iSubItem - 5] = !sortstat[pNMListView->iSubItem - 5];
		adder = sortstat[pNMListView->iSubItem-5] ? 0 : 100;
	}
	//else if (pNMListView->iSubItem==11)
	else if (pNMListView->iSubItem==11 || pNMListView->iSubItem==14) // NEO: NPT - [NeoPartTraffic] <-- Xanatos --
	{
		ASSERT( 3 < ARRSIZE(sortstat) );
		if (!sortAscending)
			sortstat[3] = !sortstat[3];
		adder = sortstat[3] ? 0 : 100;
	}

	// Sort table
	if (adder==0)	
		SetSortArrow(pNMListView->iSubItem, sortAscending); 
	else
		SetSortArrow(pNMListView->iSubItem, sortAscending ? arrowDoubleUp : arrowDoubleDown);

	//UpdateSortHistory(pNMListView->iSubItem + adder + (sortAscending ? 0:20),20);
	//SortItems(SortProc, pNMListView->iSubItem + adder + (sortAscending ? 0:20));
	// NEO: SE - [SortExtension] -- Xanatos -->
	UpdateSortHistory(GetSortItem() + (sortAscending ? 0:20), 20);
	SortItems(SortProc, GetSortItem() + adder + (GetSortAscending() ? 0:20) + (IsAlternate()?2000:0));
	// NEO: SE END <-- Xanatos --

	*pResult = 0;
}

int CSharedFilesCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	// NEO: SE - [SortExtension] -- Xanatos -->
	bool showParts=false;  
	if (lParamSort>=2000)
	{ 
		showParts=true; 
		lParamSort-=2000; 
	} 
	// NEO: SE END <-- Xanatos --

	// NEO: SP - [SharedParts] -- Xanatos -->
	const CSharedItem* item1=(CSharedItem*)lParam1;
	const CSharedItem* item2=(CSharedItem*)lParam2;

	int iResult = 0;

	if(item1->isFile != false && item2->isFile != false){
		if (showParts) return 0; // NEO: SEa - [SortAltExtension]
		iResult = Compare(item1->KnownFile,item2->KnownFile,lParamSort);
	}
	else if(item1->isFile == false && item2->isFile == false)
	{
		if(item1->KnownFile == item2->KnownFile) 
			iResult = Compare(item1,item2,lParamSort);
		else // different files
			iResult = 0;
	}
	else // compare file and part
	{
		return 0;
	}
	// NEO: SP END <-- Xanatos --

	// NEO: SE - [SortExtension] -- Xanatos --
	/*int dwNextSort;
	//call secondary sortorder, if this one results in equal
	//(Note: yes I know this call is evil OO wise, but better than changing a lot more code, while we have only one instance anyway - might be fixed later)
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->sharedfileswnd->sharedfilesctrl.GetNextSortOrder(lParamSort)) != (-1)){
		iResult= SortProc(lParam1, lParam2, dwNextSort);
	}*/

	return iResult;
}
	
int CSharedFilesCtrl::Compare(const CKnownFile* item1, const CKnownFile* item2, LPARAM lParamSort) // NEO: SP - [SharedParts] <-- Xanatos --
{
	int iResult=0;
	switch(lParamSort){
		case 0: //filename asc
			iResult=CompareLocaleStringNoCase(item1->GetFileName(),item2->GetFileName());
			break;
		case 20: //filename desc
			iResult=CompareLocaleStringNoCase(item2->GetFileName(),item1->GetFileName());
			break;

		case 1: //filesize asc
			iResult=item1->GetFileSize()==item2->GetFileSize()?0:(item1->GetFileSize()>item2->GetFileSize()?1:-1);
			break;

		case 21: //filesize desc
			iResult=item1->GetFileSize()==item2->GetFileSize()?0:(item2->GetFileSize()>item1->GetFileSize()?1:-1);
			break;

		case 2: //filetype asc
			iResult=item1->GetFileTypeDisplayStr().Compare(item2->GetFileTypeDisplayStr());
			break;
		case 22: //filetype desc
			iResult=item2->GetFileTypeDisplayStr().Compare(item1->GetFileTypeDisplayStr());
			break;

		case 3: //prio asc
		{
			/*uint8 p1=item1->GetUpPriority() +1;
			if(p1==5)
				p1=0;
			uint8 p2=item2->GetUpPriority() +1;
			if(p2==5)
				p2=0;
			iResult=p1-p2;*/
			// NEO: SRS - [SmartReleaseSharing] -- Xanatos -->
			if(item1->IsReleasePriority() != item2->IsReleasePriority())
				iResult = item1->IsReleasePriority() - item2->IsReleasePriority();
			else if(item1->GetPowerShared() != item2->GetPowerShared())
				iResult = item1->GetPowerShared() - item2->GetPowerShared();
			else if(item1->GetReleasePriority() != item2->GetReleasePriority())
				iResult = item1->GetReleasePriority() - item2->GetReleasePriority();
			else if(item1->GetReleasePriority() && item2->GetReleasePriority())
				iResult = (int)(item1->GetReleaseModifyer() - item2->GetReleaseModifyer());
			else
				iResult=((item1->GetUpPriority()==PR_VERYLOW) ? -1 : item1->GetUpPriority()) - ((item2->GetUpPriority()==PR_VERYLOW) ? -1 : item2->GetUpPriority());
			if(iResult == 0)
				iResult = item1->IsAutoUpPriority() - item2->IsAutoUpPriority();
			// NEO: SRS <-- Xanatos --
			break;
		}
		case 23: //prio desc
		{
			/*uint8 p1=item1->GetUpPriority() +1;
			if(p1==5)
				p1=0;
			uint8 p2=item2->GetUpPriority() +1;
			if(p2==5)
				p2=0;
			iResult=p2-p1;*/
			// NEO: SRS - [SmartReleaseSharing] -- Xanatos -->
			if(item2->IsReleasePriority() != item1->IsReleasePriority())
				iResult = item2->IsReleasePriority() - item1->IsReleasePriority();
			else if(item2->GetPowerShared() != item1->GetPowerShared())
				iResult = item2->GetPowerShared() - item1->GetPowerShared();
			else if(item2->GetReleasePriority() != item1->GetReleasePriority())
				iResult = item2->GetReleasePriority() - item1->GetReleasePriority();
			else if(item2->GetReleasePriority() && item1->GetReleasePriority())
				iResult = (int)(item2->GetReleaseModifyer() - item1->GetReleaseModifyer());
			else
				iResult=((item2->GetUpPriority()==PR_VERYLOW) ? -1 : item2->GetUpPriority()) - ((item1->GetUpPriority()==PR_VERYLOW) ? -1 : item1->GetUpPriority());
			if(iResult == 0)
				iResult = item2->IsAutoUpPriority() - item1->IsAutoUpPriority();
			// NEO: SRS <-- Xanatos --
			break;
		}

		case 4: //fileID asc
			iResult=memcmp(item1->GetFileHash(), item2->GetFileHash(), 16);
			break;
		case 24: //fileID desc
			iResult=memcmp(item2->GetFileHash(), item1->GetFileHash(), 16);
			break;

		case 5: //requests asc
			iResult=item1->statistic.GetRequests() - item2->statistic.GetRequests();
			break;
		case 25: //requests desc
			iResult=item2->statistic.GetRequests() - item1->statistic.GetRequests();
			break;
		
		case 6: //acc requests asc
			iResult=item1->statistic.GetAccepts() - item2->statistic.GetAccepts();
			break;
		case 26: //acc requests desc
			iResult=item2->statistic.GetAccepts() - item1->statistic.GetAccepts();
			break;
		
		case 7: //all transferred asc
			iResult=item1->statistic.GetTransferred()==item2->statistic.GetTransferred()?0:(item1->statistic.GetTransferred()>item2->statistic.GetTransferred()?1:-1);
			break;
		case 27: //all transferred desc
			iResult=item1->statistic.GetTransferred()==item2->statistic.GetTransferred()?0:(item2->statistic.GetTransferred()>item1->statistic.GetTransferred()?1:-1);
			break;

		case 9: //folder asc
			iResult=CompareLocaleStringNoCase(item1->GetPath(),item2->GetPath());
			break;
		case 29: //folder desc
			iResult=CompareLocaleStringNoCase(item2->GetPath(),item1->GetPath());
			break;

		case 10: //complete sources asc
			iResult=CompareUnsigned(item1->m_nCompleteSourcesCount, item2->m_nCompleteSourcesCount);
			break;
		case 30: //complete sources desc
			iResult=CompareUnsigned(item2->m_nCompleteSourcesCount, item1->m_nCompleteSourcesCount);
			break;

		case 11: //ed2k shared asc
			iResult=item1->GetPublishedED2K() - item2->GetPublishedED2K();
			break;
		case 31: //ed2k shared desc
			iResult=item2->GetPublishedED2K() - item1->GetPublishedED2K();
			break;

		// NEO: NSC - [NeoSharedCategories] -- Xanatos -->
		case 12:
			iResult=CompareLocaleStringNoCase(	(const_cast<CKnownFile*>(item1)->GetCategory()!=0)?thePrefs.GetCategory(const_cast<CKnownFile*>(item1)->GetCategory())->strTitle:GetResString(IDS_ALL),
											(const_cast<CKnownFile*>(item2)->GetCategory()!=0)?thePrefs.GetCategory(const_cast<CKnownFile*>(item2)->GetCategory())->strTitle:GetResString(IDS_ALL) );
			break;
		case 32:
			iResult=-CompareLocaleStringNoCase( (const_cast<CKnownFile*>(item1)->GetCategory()!=0)?thePrefs.GetCategory(const_cast<CKnownFile*>(item1)->GetCategory())->strTitle:GetResString(IDS_ALL),
											 (const_cast<CKnownFile*>(item2)->GetCategory()!=0)?thePrefs.GetCategory(const_cast<CKnownFile*>(item2)->GetCategory())->strTitle:GetResString(IDS_ALL) );
			break;
		// NEO: NSC END <-- Xanatos --

		// NEO: NPT - [NeoPartTraffic] -- Xanatos -->
		case 14: //upload asc 
			iResult=(int)(100*item1->statistic.GetCompleteReleases()) - (int)(100*item2->statistic.GetCompleteReleases()); 
			break;
		case 34: //upload desc 
			iResult=(int)(100*item2->statistic.GetCompleteReleases()) - (int)(100*item1->statistic.GetCompleteReleases());
			break;
		// NEO: NPT END <-- Xanatos --

		// NEO: SSP - [ShowSharePermissions] -- Xanatos -->
		case 15: //permission asc 
			iResult=item2->GetPermissions()-item1->GetPermissions();
			break;
		case 35: //permission desc 
			iResult=item1->GetPermissions()-item2->GetPermissions();
			break;
		// NEO: SSP END <-- Xanatos --

		// NEO: VSF - [VirtualSharedFiles] -- Xanatos -->
		case 16:
			iResult=item1->GetPath(true).CompareNoCase(item2->GetPath(true));
			break;
		case 36:
			iResult=item2->GetPath(true).CompareNoCase(item1->GetPath(true));
			break;
		// NEO: VSF END <-- Xanatos --

		// NEO: CRC - [MorphCRCTag] -- Xanatos -->
		case 17:
			iResult=item2->IsCRCOk()-item1->IsCRCOk();
			break;
		case 37:
			iResult=item1->IsCRCOk()-item2->IsCRCOk();
		// NEO: CRC END <-- Xanatos --

		case 105: //all requests asc
			iResult=CompareUnsigned(item1->statistic.GetAllTimeRequests(), item2->statistic.GetAllTimeRequests());
			break;
		case 125: //all requests desc
			iResult=CompareUnsigned(item2->statistic.GetAllTimeRequests(), item1->statistic.GetAllTimeRequests());
			break;

		case 106: //all acc requests asc
			iResult=CompareUnsigned(item1->statistic.GetAllTimeAccepts(), item2->statistic.GetAllTimeAccepts());
			break;
		case 126: //all acc requests desc
			iResult=CompareUnsigned(item2->statistic.GetAllTimeAccepts(), item1->statistic.GetAllTimeAccepts());
			break;

		case 107: //all transferred asc
			iResult=item1->statistic.GetAllTimeTransferred()==item2->statistic.GetAllTimeTransferred()?0:(item1->statistic.GetAllTimeTransferred()>item2->statistic.GetAllTimeTransferred()?1:-1);
			break;
		case 127: //all transferred desc
			iResult=item1->statistic.GetAllTimeTransferred()==item2->statistic.GetAllTimeTransferred()?0:(item2->statistic.GetAllTimeTransferred()>item1->statistic.GetAllTimeTransferred()?1:-1);
			break;

		case 111:{ //kad shared asc
			uint32 tNow = time(NULL);
			int i1 = (tNow < item1->GetLastPublishTimeKadSrc()) ? 1 : 0;
			int i2 = (tNow < item2->GetLastPublishTimeKadSrc()) ? 1 : 0;
			iResult=i1 - i2;
			break;
		}
		case 131:{ //kad shared desc
			uint32 tNow = time(NULL);
			int i1 = (tNow < item1->GetLastPublishTimeKadSrc()) ? 1 : 0;
			int i2 = (tNow < item2->GetLastPublishTimeKadSrc()) ? 1 : 0;
			iResult=i2 - i1;
			break;
		}

		// NEO: NPT - [NeoPartTraffic] -- Xanatos -->
		case 114: //upload asc 
			iResult=(int)((sint64)(100*item1->statistic.GetAllTimeTransferred())/(sint64)item1->GetFileSize()) - (int)((sint64)(100*item2->statistic.GetAllTimeTransferred())/(sint64)item2->GetFileSize());
			break;
		case 134: //upload desc 
			iResult=(int)((sint64)(100*item2->statistic.GetAllTimeTransferred())/(sint64)item2->GetFileSize()) - (int)((sint64)(100*item1->statistic.GetAllTimeTransferred())/(sint64)item1->GetFileSize());
			break;
		// NEO: NPT END <-- Xanatos --

		default: 
			iResult=0;
			break;
	}

	return iResult;
}

// NEO: SP - [SharedParts] -- Xanatos -->
#define sortcmp(a, b) ((a)==(b)) ? 0 : (((a)<(b))?-1:1)
int CSharedFilesCtrl::Compare(const CSharedItem* item1, const CSharedItem* item2, LPARAM lParamSort)
{
	int iResult=0;
	switch(lParamSort){
		case 0: //filename asc
			iResult=sortcmp(item1->Part, item2->Part);
			break;
		case 20: //filename desc
			iResult=sortcmp(item2->Part, item1->Part);
			break;

		case 1: //filesize asc
			iResult=(!item1->KnownFile->IsPartFile() || ((CPartFile*)item1->KnownFile)->IsComplete(PARTSIZE*item1->Part, PARTSIZE*item1->Part+PARTSIZE-1, true)) - (!item2->KnownFile->IsPartFile() || ((CPartFile*)item2->KnownFile)->IsComplete(PARTSIZE*item2->Part, PARTSIZE*item2->Part+PARTSIZE-1, true));
			break;
		case 21: //filesize desc
			iResult=(!item2->KnownFile->IsPartFile() || ((CPartFile*)item2->KnownFile)->IsComplete(PARTSIZE*item2->Part, PARTSIZE*item2->Part+PARTSIZE-1, true)) - (!item1->KnownFile->IsPartFile() || ((CPartFile*)item1->KnownFile)->IsComplete(PARTSIZE*item1->Part, PARTSIZE*item1->Part+PARTSIZE-1, true));
			break;


		//case 2: //filetype asc
			
		//case 22: //filetype desc

		// NEO: MCS - [ManualChunkSelection]
		case 3: //prio asc
			iResult=(item1->KnownFile->IsPartFile() ? ((CPartFile*)item1->KnownFile)->PartPrefs->GetWantedPart(item1->Part) : PR_PART_NORMAL) - (item2->KnownFile->IsPartFile() ? ((CPartFile*)item2->KnownFile)->PartPrefs->GetWantedPart(item2->Part) : PR_PART_NORMAL);
			break;
		case 23: //prio desc
			iResult=(item2->KnownFile->IsPartFile() ? ((CPartFile*)item2->KnownFile)->PartPrefs->GetWantedPart(item2->Part) : PR_PART_NORMAL) - (item1->KnownFile->IsPartFile() ? ((CPartFile*)item1->KnownFile)->PartPrefs->GetWantedPart(item1->Part) : PR_PART_NORMAL);
			break;
		// NEO: MCS END

		case 4: //fileID asc
			iResult=memcmp(item1->KnownFile->GetPartHash(item1->Part), item2->KnownFile->GetPartHash(item2->Part), 16);
			break;
		case 24: //fileID desc
			iResult=memcmp(item2->KnownFile->GetPartHash(item2->Part), item1->KnownFile->GetPartHash(item1->Part), 16);
			break;

		//case 5: //requests asc
			
		//case 25: //requests desc
			
		
		//case 6: //acc requests asc
		//	iResult=item1->KnownFile->statistic.GetPartAccepted(item1->Part, true) - item2->KnownFile->statistic.GetPartAccepted(item2->Part, true);
		//	break;
		//case 26: //acc requests desc
		//	iResult=item2->KnownFile->statistic.GetPartAccepted(item2->Part, true) - item1->KnownFile->statistic.GetPartAccepted(item1->Part, true);
		//	break;
		
		case 7: //all transferred asc
			iResult=item1->KnownFile->statistic.GetPartTrafficSession(item1->Part)==item2->KnownFile->statistic.GetPartTrafficSession(item2->Part)?0:(item1->KnownFile->statistic.GetPartTrafficSession(item1->Part)>item2->KnownFile->statistic.GetPartTrafficSession(item2->Part)?1:-1);
			break;
		case 27: //all transferred desc
			iResult=item1->KnownFile->statistic.GetPartTrafficSession(item1->Part)==item2->KnownFile->statistic.GetPartTrafficSession(item2->Part)?0:(item2->KnownFile->statistic.GetPartTrafficSession(item2->Part)>item1->KnownFile->statistic.GetPartTrafficSession(item1->Part)?1:-1);
			break;

		//case 9: //folder asc

		//case 29: //folder desc

		case 10: //complete sources asc
			iResult=CompareUnsigned(item1->KnownFile->GetPartAvailibility(item1->Part), item2->KnownFile->GetPartAvailibility(item2->Part));
			break;
		case 30: //complete sources desc
			iResult=CompareUnsigned(item2->KnownFile->GetPartAvailibility(item2->Part), item1->KnownFile->GetPartAvailibility(item1->Part));
			break;

		//case 11: //ed2k shared asc
			
		//case 31: //ed2k shared desc
			

		//case 12: //permission asc

		//case 32: //permission desc 

		// NEO: NPT - [NeoPartTraffic]
		case 14: //upload asc 
			iResult=(int)(100*item1->KnownFile->statistic.GetPartRelease(item1->Part)) - (int)(100*item2->KnownFile->statistic.GetPartRelease(item2->Part)); 
			break;
		case 34: //upload desc 
			iResult=(int)(100*item2->KnownFile->statistic.GetPartRelease(item2->Part)) - (int)(100*item1->KnownFile->statistic.GetPartRelease(item1->Part));
			break;
		// NEO: NPT END

		// NEO: MPS - [ManualPartSharing]
		case 15: //permission asc
			iResult=item1->KnownFile->KnownPrefs->GetManagedPart(item1->Part)-item2->KnownFile->KnownPrefs->GetManagedPart(item2->Part);
			break;
		case 35: //permission desc 
			iResult=item2->KnownFile->KnownPrefs->GetManagedPart(item2->Part)-item1->KnownFile->KnownPrefs->GetManagedPart(item1->Part);
			break;
		// NEO: MPS END

		//case 105: //all requests asc
			
		//case 125: //all requests desc	

		// NEO: NPT - [NeoPartTraffic]
		//case 106: //all acc requests asc
		//	iResult=item1->KnownFile->statistic.GetPartAccepted(item1->Part, false) - item2->KnownFile->statistic.GetPartAccepted(item2->Part, false);
		//	break;
		//case 126: //all acc requests desc
		//	iResult=item2->KnownFile->statistic.GetPartAccepted(item2->Part, false) - item1->KnownFile->statistic.GetPartAccepted(item1->Part, false);
		//	break;
		// NEO: NPT END

		// NEO: NPT - [NeoPartTraffic]
		case 107: //all transferred asc
			iResult=item1->KnownFile->statistic.GetPartTraffic(item1->Part)==item2->KnownFile->statistic.GetPartTraffic(item2->Part)?0:(item1->KnownFile->statistic.GetPartTraffic(item1->Part)>item2->KnownFile->statistic.GetPartTraffic(item2->Part)?1:-1);
			break;
		case 127: //all transferred desc
			iResult=item1->KnownFile->statistic.GetPartTraffic(item1->Part)==item2->KnownFile->statistic.GetPartTraffic(item2->Part)?0:(item2->KnownFile->statistic.GetPartTraffic(item2->Part)>item1->KnownFile->statistic.GetPartTraffic(item1->Part)?1:-1);
			break;
		// NEO: NPT END

		//case 111: //kad shared asc
		//case 131: //kad shared desc

		// NEO: NPT - [NeoPartTraffic]
		case 114: //upload asc 
			iResult=(100*item1->KnownFile->statistic.GetPartTraffic(item1->Part)/item1->KnownFile->GetPartSize(item1->Part)) - (100*item2->KnownFile->statistic.GetPartTraffic(item2->Part)/item2->KnownFile->GetPartSize(item2->Part));
			break;
		case 134: //upload desc 
			iResult=(100*item2->KnownFile->statistic.GetPartTraffic(item2->Part)/item2->KnownFile->GetPartSize(item2->Part)) - (100*item1->KnownFile->statistic.GetPartTraffic(item1->Part)/item1->KnownFile->GetPartSize(item1->Part));
			break;
		// NEO: NPT END
		default: 
			iResult=0;
			break;
	}
	return iResult;
}
// NEO: SP END <-- Xanatos --

void CSharedFilesCtrl::OpenFile(const CKnownFile* file)
{
	if(file->m_pCollection)
	{
		// NEO: MLD - [ModelesDialogs] -- Xanatos -->
		CCollectionViewDialog* dialog = new CCollectionViewDialog(); 
		dialog->SetCollection(file->m_pCollection);
		dialog->OpenDialog();
		// NEO: MLD END <-- Xanatos --
		//CCollectionViewDialog dialog;
		//dialog.SetCollection(file->m_pCollection);
		//dialog.DoModal();
	}
	else
		ShellOpenFile(file->GetFilePath(), NULL);
}

// NEO: NTS - [NeoTreeStyle] -- Xanatos -->
void CSharedFilesCtrl::OnClick(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	POINT point;
	::GetCursorPos(&point);
	CPoint p = point; 
    ScreenToClient(&p);

	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	if (p.x<12) 
		ExpandCollapseItem(pNMIA->iItem);

	theApp.emuledlg->sharedfileswnd->ShowSelectedFilesSummary(); // Need for update the status fields
}
// NEO: NTS END <-- Xanatos --

void CSharedFilesCtrl::OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1)
	{
		// NEO: SP - [SharedParts] -- Xanatos -->
		if(thePrefs.IsDoubleClickEnabled()){
			NMITEMACTIVATE *i=(NMITEMACTIVATE*)pNMHDR;
			ExpandCollapseItem(i->iItem);
		}
		else{
			CKnownFile* file = ((CSharedItem*)GetItemData(iSel))->KnownFile;
		// NEO: SP END <-- Xanatos --
			//CKnownFile* file = (CKnownFile*)GetItemData(iSel);
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
		} // NEO: SP - [SharedParts] <-- Xanatos --
	}
	*pResult = 0;
}

void CSharedFilesCtrl::CreateMenues()
{
	if (m_PrioMenu) VERIFY( m_PrioMenu.DestroyMenu() );
	if (m_PermMenu) VERIFY( m_PermMenu.DestroyMenu() ); // NEO: SSP - [ShowSharePermissions] <-- Xanatos --
	if (m_PWProtMenu) VERIFY( m_PWProtMenu.DestroyMenu()); // NEO: PP - [PasswordProtection] <-- Xanatos --
	if (m_VirtualDirMenu) VERIFY (m_VirtualDirMenu.DestroyMenu()); // NEO: VSF - [VirtualSharedFiles] <-- Xanatos --
	if (m_CRC32Menu) VERIFY( m_CRC32Menu.DestroyMenu() ); // NEO: CRC - [MorphCRCTag] <-- Xanatos --
	if (m_ReleaseMenu) VERIFY( m_ReleaseMenu.DestroyMenu() ); // NEO: MOD - [ReleaseMenu] <-- Xanatos --
	if (m_CollectionsMenu) VERIFY( m_CollectionsMenu.DestroyMenu() );
	if (m_SharedFilesMenu) VERIFY( m_SharedFilesMenu.DestroyMenu() );


	m_PrioMenu.CreateMenu();
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOVERYLOW,GetResString(IDS_PRIOVERYLOW));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOLOW,GetResString(IDS_PRIOLOW));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIONORMAL,GetResString(IDS_PRIONORMAL));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOHIGH, GetResString(IDS_PRIOHIGH));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOVERYHIGH, GetResString(IDS_X_PRIOVERYHIGH)); // NEO: MOD - [ForSRS] <-- Xanatos --
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOAUTO, GetResString(IDS_PRIOAUTO));//UAP
	// NEO: SRS - [SmartReleaseSharing] -- Xanatos -->
	m_PrioMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIORELEASE, GetResString(IDS_PRIORELEASE));
	// NEO: SRS END <-- Xanatos --

	// NEO: SSP - [ShowSharePermissions] -- Xanatos -->
	m_PermMenu.CreateMenu();
	m_PermMenu.AddMenuTitle(NULL, true);
	m_PermMenu.AppendMenu(MF_STRING,MP_PERMDEFAULT, GetResString(IDS_X_PERM_DEFAULT));
	m_PermMenu.AppendMenu(MF_STRING,MP_PERMNONE, GetResString(IDS_X_PERM_HIDDEN));
	m_PermMenu.AppendMenu(MF_STRING,MP_PERMFRIENDS, GetResString(IDS_X_PERM_FRIENDS));
	m_PermMenu.AppendMenu(MF_STRING,MP_PERMALL, GetResString(IDS_X_PERM_PUBLIC));
	// NEO: SSP END <-- Xanatos --

	// NEO: PP - [PasswordProtection] -- Xanatos -->
	m_PWProtMenu.CreateMenu();
	m_PWProtMenu.AddMenuTitle(NULL, true);
	m_PWProtMenu.AppendMenu(MF_STRING,MP_PWPROT_SHOW,GetResString(IDS_X_PWPROT_SHOW), _T("PWPROT_SHOW"));
	m_PWProtMenu.AppendMenu(MF_STRING,MP_PWPROT_HIDE,GetResString(IDS_X_PWPROT_HIDE), _T("PWPROT_HIDE"));
	m_PWProtMenu.AppendMenu(MF_SEPARATOR);
	m_PWProtMenu.AppendMenu(MF_STRING,MP_PWPROT_SET,GetResString(IDS_X_PWPROT_SET), _T("PWPROT_SET"));
	m_PWProtMenu.AppendMenu(MF_STRING,MP_PWPROT_CHANGE,GetResString(IDS_X_PWPROT_CHANGE), _T("PWPROT_CHANGE"));
	m_PWProtMenu.AppendMenu(MF_STRING,MP_PWPROT_UNSET,GetResString(IDS_X_PWPROT_UNSET), _T("PWPROT_UNSET"));
	// NEO: PP END <-- Xanatos --

	// NEO: VSF - [VirtualSharedFiles] -- Xanatos -->
	m_VirtualDirMenu.CreateMenu();
	m_VirtualDirMenu.AddMenuTitle(GetResString(IDS_X_VDS_VIRTDIRTITLE), true);
	m_VirtualDirMenu.AppendMenu(MF_STRING, MP_IOM_VIRTFILE, GetResString(IDS_X_VDS_MFILE), _T("VDFILE"));
	m_VirtualDirMenu.AppendMenu(MF_STRING, MP_IOM_VIRTDIR, GetResString(IDS_X_VDS_MDIR), _T("VDDIR"));
	m_VirtualDirMenu.AppendMenu(MF_STRING, MP_IOM_VIRTSUBDIR, GetResString(IDS_X_VDS_MSDIR), _T("VDSUBDIR"));
	m_VirtualDirMenu.AppendMenu(MF_SEPARATOR);
	m_VirtualDirMenu.AppendMenu(MF_STRING, MP_IOM_VIRTREMOVE, GetResString(IDS_X_VDS_REMOVE), _T("VDREMOVE"));
	//m_VirtualDirMenu.AppendMenu(MF_STRING, MP_IOM_VIRTPREFS, GetResString(IDS_X_VDS_ADVANCED), _T("VDSET"));
	// NEO: VSF END <-- Xanatos --

	// NEO: CRC - [MorphCRCTag]
	m_CRC32Menu.CreateMenu();
	m_CRC32Menu.AddMenuTitle(NULL, true);
	m_CRC32Menu.AppendMenu(MF_STRING,MP_CRC32_CALCULATE,GetResString(IDS_X_CRC32_CALCULATE),_T("FILECRC32_CALC"));
	m_CRC32Menu.AppendMenu(MF_STRING,MP_CRC32_RECALCULATE,GetResString(IDS_X_CRC32_RECALCULATE),_T("FILECRC32_RECALC"));
	m_CRC32Menu.AppendMenu(MF_STRING,MP_CRC32_TAG,GetResString(IDS_X_CRC32_TAG),_T("FILECRC32_ADD"));
	m_CRC32Menu.AppendMenu(MF_STRING,MP_CRC32_ABORT,GetResString(IDS_CRC32_ABORT),_T("FILECRC32_ABBORT"));
	// NEO: CRC END

	// NEO: MOD - [ReleaseMenu] -- Xanatos -->
	m_ReleaseMenu.CreateMenu();
	m_ReleaseMenu.AddMenuTitle(GetResString(IDS_X_RELEASE_MENU), true);
	m_ReleaseMenu.AppendMenu(MF_STRING,MP_RECALC_IPS, GetResString(IDS_X_RECALC_IPS), _T("RECALCIPS")); // NEO: IPS - [InteligentPartSharing]
	m_ReleaseMenu.AppendMenu(MF_STRING,MP_RECALC_PS, GetResString(IDS_X_RECALC_PS), _T("RECALCPS"));
	m_ReleaseMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	// NEO: NPT - [NeoPartTraffic]
	m_ReleaseMenu.AppendMenu(MF_STRING,MP_CLEARALLSTATS, GetResString(IDS_X_CLEARALLSTATS), _T("RESETALLTRAFFIC"));
	m_ReleaseMenu.AppendMenu(MF_STRING,MP_CLEARSTATS, GetResString(IDS_X_CLEARSTATS), _T("RESETTRAFFIC"));
	// NEO: NPT END
	m_ReleaseMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	m_ReleaseMenu.AppendMenu(MF_STRING,MP_CLEARCOMMENTS, GetResString(IDS_X_CLEARCOMMENTS), _T("RESETCOMMENTS")); // NEO: XCs - [SaveComments]
	// NEO: MPS - [ManualPartSharing]
	m_ReleaseMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	m_ReleaseMenu.AppendMenu(MF_STRING,MP_COPYBLOCK, GetResString(IDS_X_COPYBLOCK), _T("PARTPRIOCOPY")); 
	m_ReleaseMenu.AppendMenu(MF_STRING,MP_PASTEBLOCK, GetResString(IDS_X_PASTEBLOCK), _T("PARTPRIOPASTE")); 
	m_ReleaseMenu.AppendMenu(MF_STRING,MP_CLEARBLOCK, GetResString(IDS_X_CLEARBLOCK), _T("PARTPRIOCLEAR")); 
	// NEO: MPS END
	// NEO: MCS - [ManualChunkSelection]
	m_ReleaseMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	m_ReleaseMenu.AppendMenu(MF_STRING,MP_LINEARWANTED, GetResString(IDS_X_LINEARWANTED), _T("LINEARWANTED")); 
	m_ReleaseMenu.AppendMenu(MF_STRING,MP_CLEARWANTED, GetResString(IDS_X_CLEARWANTED), _T("CLEARWANTED")); 
	// NEO: MCS END
	// NEO: MOD END <-- Xanatos --

	m_CollectionsMenu.CreateMenu();
	m_CollectionsMenu.AddMenuTitle(NULL, true);
	m_CollectionsMenu.AppendMenu(MF_STRING,MP_CREATECOLLECTION, GetResString(IDS_CREATECOLLECTION), _T("COLLECTION_ADD"));
	m_CollectionsMenu.AppendMenu(MF_STRING,MP_MODIFYCOLLECTION, GetResString(IDS_MODIFYCOLLECTION), _T("COLLECTION_EDIT"));
	m_CollectionsMenu.AppendMenu(MF_STRING,MP_VIEWCOLLECTION, GetResString(IDS_VIEWCOLLECTION), _T("COLLECTION_VIEW"));
	m_CollectionsMenu.AppendMenu(MF_STRING,MP_SEARCHAUTHOR, GetResString(IDS_SEARCHAUTHORCOLLECTION), _T("COLLECTION_SEARCH"));

	m_SharedFilesMenu.CreatePopupMenu();
	m_SharedFilesMenu.AddMenuTitle(GetResString(IDS_SHAREDFILES), true);

	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_OPEN, GetResString(IDS_OPENFILE), _T("OPENFILE"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_OPENFOLDER, GetResString(IDS_OPENFOLDER), _T("OPENFOLDER"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_RENAME, GetResString(IDS_RENAME) + _T("..."), _T("FILERENAME"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_MASSRENAME,GetResString(IDS_X_MASSRENAME), _T("FILEMASSRENAME")); // NEO: MMR - [MorphMassRemane] <-- Xanatos --
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_REMOVE, GetResString(IDS_DELETE), _T("DELETE"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_UNSHARE_FILE, GetResString(IDS_X_UNSHARE_FILE), _T("UNSHARE")); // NEO: SSF - [ShareSingleFiles] <-- Xanatos --
	if (thePrefs.IsExtControlsEnabled())
		m_SharedFilesMenu.AppendMenu(MF_STRING,Irc_SetSendLink,GetResString(IDS_IRC_ADDLINKTOIRC), _T("IRCCLIPBOARD"));

	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_PrioMenu.m_hMenu, GetResString(IDS_PRIORITY) + _T(" (") + GetResString(IDS_PW_CON_UPLBL) + _T(")"), _T("FILEPRIORITY"));
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_PermMenu.m_hMenu, GetResString(IDS_PERMISSION), _T("SHAREPERM")); // NEO: SSP - [ShowSharePermissions] <-- Xanatos --
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_PWProtMenu.m_hMenu, GetResString(IDS_X_PWPROT_MENU), _T("PWPROT")); // NEO: PP - [PasswordProtection] <-- Xanatos --
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_VirtualDirMenu.m_hMenu, GetResString(IDS_X_VDS_VIRTDIRTITLE), _T("VIRTUALDIR")); // NEO: VSF - [VirtualSharedFiles] <-- Xanatos --
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);

	// NEO: MOD - [ReleaseMenu] -- Xanatos -->
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_ReleaseMenu.m_hMenu, GetResString(IDS_X_RELEASE_MENU), _T("RELEASE"));
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	// NEO: MOD - [ReleaseMenu] <-- Xanatos --

	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_CollectionsMenu.m_hMenu, GetResString(IDS_META_COLLECTION), _T("COLLECTION"));
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_CRC32Menu.m_hMenu, GetResString(IDS_X_CRC32), _T("FILECRC32")); // NEO: CRC - [MorphCRCTag] <-- Xanatos --
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR); 	

	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_DETAIL, GetResString(IDS_SHOWDETAILS), _T("FILEINFO"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_TWEAKS, GetResString(IDS_X_FILE_TWEAKS), _T("FILECONFIG")); // NEO: FCFG - [FileConfiguration] <-- Xanatos --
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_CMT, GetResString(IDS_CMT_ADD), _T("FILECOMMENTS"));
	if (thePrefs.GetShowCopyEd2kLinkCmd())
		m_SharedFilesMenu.AppendMenu(MF_STRING,MP_GETED2KLINK, GetResString(IDS_DL_LINK1), _T("ED2KLINK") );
	else
		m_SharedFilesMenu.AppendMenu(MF_STRING,MP_SHOWED2KLINK, GetResString(IDS_DL_SHOWED2KLINK), _T("ED2KLINK") );
	// NEO: FB - [FeedBack] -- Xanatos -->
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_COPYFEEDBACK, GetResString(IDS_X_COPYFEEDBACK), _T("COPY"));
	m_SharedFilesMenu.AppendMenu(MF_SEPARATOR);
	// NEO: FB END <-- Xanatos --
	m_SharedFilesMenu.AppendMenu(MF_STRING, MP_SEARCHHASH, GetResString(IDS_X_SEARCHHASH), _T("KadCurrentSearches")); // NEO: MOD - [SearchHash] <-- Xanatos --
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_FIND, GetResString(IDS_FIND), _T("Search"));
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);

#if defined(_DEBUG)
	if (thePrefs.IsExtControlsEnabled()){
		//JOHNTODO: Not for release as we need kad lowID users in the network to see how well this work work. Also, we do not support these links yet.
		m_SharedFilesMenu.AppendMenu(MF_STRING,MP_GETKADSOURCELINK, _T("Copy eD2K Links To Clipboard (Kad)"));
		m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	}
#endif
}

void CSharedFilesCtrl::ShowComments(CKnownFile* file)
{
	if (file)
	{
		CTypedPtrList<CPtrList, CKnownFile*> aFiles;
		aFiles.AddHead(file);
		ShowFileDialog(aFiles, IDD_COMMENT);
	}
}

#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
void CSharedFilesCtrl::OnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

	if (theApp.emuledlg->IsRunning()){
		// Although we have an owner drawn listview control we store the text for the primary item in the listview, to be
		// capable of quick searching those items via the keyboard. Because our listview items may change their contents,
		// we do this via a text callback function. The listview control will send us the LVN_DISPINFO notification if
		// it needs to know the contents of the primary item.
		//
		// But, the listview control sends this notification all the time, even if we do not search for an item. At least
		// this notification is only sent for the visible items and not for all items in the list. Though, because this
		// function is invoked *very* often, no *NOT* put any time consuming code here in.

		if (pDispInfo->item.mask & LVIF_TEXT){
			// NEO: SP - [SharedParts] -- Xanatos -->
			const CSharedItem* pItem = reinterpret_cast<CSharedItem*>(pDispInfo->item.lParam);
			const CKnownFile* pFile = pItem->KnownFile;
			// NEO: SP END <-- Xanatos --
			//const CKnownFile* pFile = reinterpret_cast<CKnownFile*>(pDispInfo->item.lParam);
			if (pFile != NULL){
				switch (pDispInfo->item.iSubItem){
					case 0:
						if (pDispInfo->item.cchTextMax > 0){
							_tcsncpy(pDispInfo->item.pszText, pFile->GetFileName(), pDispInfo->item.cchTextMax);
							pDispInfo->item.pszText[pDispInfo->item.cchTextMax-1] = _T('\0');
						}
						break;
					default:
						// shouldn't happen
						pDispInfo->item.pszText[0] = _T('\0');
						break;
				}
			}
		}
	}
	*pResult = 0;
}
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

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

	CMuleListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

//void CSharedFilesCtrl::ShowFileDialog(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage)
void CSharedFilesCtrl::ShowFileDialog(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage, BOOL Preferences) // NEO: FCFG - [FileConfiguration] <-- Xanatos --
{
	if (aFiles.GetSize() > 0)
	{
		// NEO: FCFG - [FileConfiguration] -- Xanatos -->
		if(Preferences)
		{
			CSimpleArray<CKnownFile*> paFiles;
			POSITION pos = aFiles.GetHeadPosition();
			while (pos)
				paFiles.Add(aFiles.GetNext(pos));
			// NEO: MLD - [ModelesDialogs] 
			CFilePreferencesDialog* dlg = new CFilePreferencesDialog(&paFiles, uPshInvokePage, this);
			dlg->OpenDialog(); 
			// NEO: MLD END
			//CFilePreferencesDialog dialog(&paFiles, uPshInvokePage, this);
			//dialog.DoModal();
		}
		else
		// NEO: FCFG END <-- Xanatos --
		{
			// NEO: NFDD - [NewFileDetailDialog] -- Xanatos -->
			CSimpleArray<CAbstractFile*> paFiles;
			POSITION pos = aFiles.GetHeadPosition();
			while (pos)
				paFiles.Add(aFiles.GetNext(pos));
			//CFileDetailDialog dialog(&paFiles, uPshInvokePage, this);
			// NEO: NFDD END <-- Xanatos --
			//CSharedFileDetailsSheet dialog(aFiles, uPshInvokePage, this);
			//dialog.DoModal();
			// NEO: MLD - [ModelesDialogs] -- Xanatos -->
			CFileDetailDialog* dlg = new CFileDetailDialog(&paFiles, uPshInvokePage, this);
			dlg->OpenDialog(); 
			// NEO: MLD END <-- Xanatos --
		}
	}
}

void CSharedFilesCtrl::SetDirectoryFilter(CDirectoryItem* pNewFilter, bool bRefresh){
	if (m_pDirectoryFilter == pNewFilter)
		return;
	m_pDirectoryFilter = pNewFilter;
	if (bRefresh)
		ReloadFileList();
}

#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
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

		
		// NEO: SP - [SharedParts] -- Xanatos -->
		const CSharedItem* pItem = (CSharedItem*)GetItemData(pGetInfoTip->iItem);
		if(pItem->isFile == false)
			return;
		const CKnownFile* pFile = pItem->KnownFile;
		// NEO: SP END <-- Xanatos --
		//const CKnownFile* pFile = (CKnownFile*)GetItemData(pGetInfoTip->iItem);
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
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

bool CSharedFilesCtrl::IsFilteredItem(const CKnownFile* pKnownFile) const
{
	const CStringArray& rastrFilter = theApp.emuledlg->sharedfileswnd->m_astrFilter;
	if (rastrFilter.GetSize() == 0)
		return false;

	// filtering is done by text only for all colums to keep it consistent and simple for the user even if that
	// doesn't allows complex filters
	TCHAR szFilterTarget[256];
	GetItemDisplayText(pKnownFile, theApp.emuledlg->sharedfileswnd->GetFilterColumn(),
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

// NEO: SP - [SharedParts] -- Xanatos -->
void CSharedFilesCtrl::ExpandCollapseItem(int Item)
{
	if (Item == -1)
		return;

	CSharedItem *ItemDataParent=(CSharedItem*)GetItemData(Item);

	if(!ItemDataParent->isFile)
		return;

	CKnownFile *File=ItemDataParent->KnownFile;

	if(ItemDataParent->isOpen==true)
	{
		SetRedraw(false);

		ItemDataParent->isOpen=false;
		SetAlternate(FALSE); // NEO: SEa - [SortAltExtension]
		for(uint16 Part=0; Part<ItemDataParent->Parts; Part++)
		{
			CSharedItem *ItemData=(CSharedItem*)GetItemData(Item+1);
			if (ItemData->isFile)
				break;
			SetItemData(Item+1, NULL);
			DeleteItem(Item+1);
			if(ItemData) 
				delete ItemData;
		}

		SetRedraw(true);
	}
	else
	{
		if(!(File->GetHashCount() == File->GetED2KPartHashCount()) || File->GetPartCount() == 1)
			return; // we dont have the hashset!
		//if(IsAlternate()) // NEO: SEa - [SortAltExtension] // only one file at the time
		//	return;

		SetRedraw(false);

		CString	Buffer;
		uint16	Parts=0;
		for(uint16 Part=0; Part<File->GetPartCount(); Part++)
		{
			Parts++;

			CSharedItem	*ItemData=new CSharedItem;
			ItemData->isFile=false;
			ItemData->isOpen=false;
			ItemData->KnownFile=File;
			ItemData->Part=Part;
			ItemData->Parts=0;

			Buffer.Format(_T("%i"), Part);
			InsertItem(LVIF_TEXT|LVIF_PARAM, Item+Parts, Buffer, 0, 0, 0, (LPARAM)ItemData);
		}

		SetRedraw(true);

		ItemDataParent->Parts=Parts;
		if(Parts==0){
			ItemDataParent->isOpen=false;
			//SetAlternate(FALSE); // NEO: SEa - [SortAltExtension]
		}else{
			ItemDataParent->isOpen=true;
			SetAlternate(TRUE); // NEO: SEa - [SortAltExtension]
		}
	}

	// NEO: SE - [SortExtension]
	AlterSortArrow();

	SetSortArrow();

	if (NeoPrefs.DisableAutoSort() != 1 && IsAlternate())
		SortItems(SortProc, GetSortItem() + (GetSortAscending()? 0:100) + 2000); // NEO: SEa - [SortAltExtension]
	// NEO: SE END
}

BOOL CSharedFilesCtrl::DeleteAllItems()
{
	CSharedItem *ItemData;
	for(int i=GetItemCount(); i>0; i--){
		ItemData=(CSharedItem *)GetItemData(i-1);
		SetItemData(i-1, NULL);
		if(ItemData){
			// NEO: SEa - [SortAltExtension]
			if(ItemData->isOpen)
				SetAlternate(FALSE);
			// NEO: SEa END
			delete ItemData;
		}
	}
	return CListCtrl::DeleteAllItems();
}

CSharedFilesListCtrlItemWalk::CSharedFilesListCtrlItemWalk(CSharedFilesCtrl* pListCtrl)
	: CListCtrlItemWalk(pListCtrl)
{
	m_pSharedFilesListCtrl = pListCtrl;
	m_iParts = false;
}

CObject* CSharedFilesListCtrlItemWalk::GetPrevSelectableItem()
{
	ASSERT( m_pSharedFilesListCtrl != NULL );
	if (m_pSharedFilesListCtrl == NULL)
		return NULL;

	int iItemCount = m_pSharedFilesListCtrl->GetItemCount();
	if (iItemCount >= 2)
	{
		POSITION pos = m_pSharedFilesListCtrl->GetFirstSelectedItemPosition();
		if (pos)
		{
			int iItem = m_pSharedFilesListCtrl->GetNextSelectedItem(pos);
			int iCurSelItem = iItem;
			while (iItem-1 >= 0)
			{
				iItem--;

				const CSharedItem* SharedItem = (CSharedItem*)m_pSharedFilesListCtrl->GetItemData(iItem);
				if (SharedItem->isFile == !m_iParts)
				{
					m_pSharedFilesListCtrl->SetItemState(iCurSelItem, 0, LVIS_SELECTED | LVIS_FOCUSED);
					m_pSharedFilesListCtrl->SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
					m_pSharedFilesListCtrl->SetSelectionMark(iItem);
					m_pSharedFilesListCtrl->EnsureVisible(iItem, FALSE);
					return STATIC_DOWNCAST(CObject, (CObject*)SharedItem->KnownFile);
				}
			}
		}
	}
	return NULL;
}

CObject* CSharedFilesListCtrlItemWalk::GetNextSelectableItem()
{
	ASSERT( m_pSharedFilesListCtrl != NULL );
	if (m_pSharedFilesListCtrl == NULL)
		return NULL;

	int iItemCount =m_pSharedFilesListCtrl->GetItemCount();
	if (iItemCount >= 2)
	{
		POSITION pos = m_pSharedFilesListCtrl->GetFirstSelectedItemPosition();
		if (pos)
		{
			int iItem = m_pSharedFilesListCtrl->GetNextSelectedItem(pos);
			int iCurSelItem = iItem;
			while (iItem+1 < iItemCount)
			{
				iItem++;

				const CSharedItem* SharedItem = (CSharedItem*)m_pSharedFilesListCtrl->GetItemData(iItem);
				if (SharedItem->isFile == !m_iParts)
				{
					m_pSharedFilesListCtrl->SetItemState(iCurSelItem, 0, LVIS_SELECTED | LVIS_FOCUSED);
					m_pSharedFilesListCtrl->SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
					m_pSharedFilesListCtrl->SetSelectionMark(iItem);
					m_pSharedFilesListCtrl->EnsureVisible(iItem, FALSE);
					return STATIC_DOWNCAST(CObject, (CObject*)SharedItem->KnownFile);
				}
			}
		}
	}
	return NULL;
}
// NEO: SP END <-- Xanatos --

// NEO: SDD - [ShareDargAndDrop] -- Xanatos -->
CLIPFORMAT g_uCustomClipbrdFormat = (CLIPFORMAT)RegisterClipboardFormat ( _T("MULE_3BCFE9D1_6D61_4cb6_9D0B_3BB3F643CA82") );
bool g_bNT = (0 == (GetVersion() & 0x80000000) );
void CSharedFilesCtrl::OnBegindragFilelist(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	COleDataSource datasrc;
	HGLOBAL        hgDrop;
	DROPFILES*     pDrop;
	CStringList    lsDraggedFiles;
	POSITION       pos;
	CString        sFile;
	UINT           uBuffSize = 0;
	TCHAR*         pszBuff;
	FORMATETC      etc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

	*pResult = 0;   // return value ignored

	// For every selected item in the list, put the filename into lsDraggedFiles.

	pos = GetFirstSelectedItemPosition();
	while (pos)
	{
		// NEO: SP - [SharedParts]
		const CSharedItem* pItem = (CSharedItem*)GetItemData(GetNextSelectedItem(pos));

		if(!pItem->isFile)
			continue;

		const CKnownFile* pFile = pItem->KnownFile;
		// NEO: SP END

		if(pFile->IsPartFile() || pFile->GetPath().IsEmpty())
			continue;

		// get the file path
		sFile.Format(_T("%s\\%s"),pFile->GetPath(),pFile->GetFileName());

		lsDraggedFiles.AddTail ( sFile );

		// Calculate the # of chars required to hold this string.

		uBuffSize += lstrlen ( sFile ) + 1;
	}

	// Add 1 extra for the final null char, and the size of the DROPFILES struct.

	uBuffSize = sizeof(DROPFILES) + sizeof(TCHAR) * (uBuffSize + 1);

	// Allocate memory from the heap for the DROPFILES struct.

	hgDrop = GlobalAlloc ( GHND | GMEM_SHARE, uBuffSize );

	if ( NULL == hgDrop )
		return;

	pDrop = (DROPFILES*) GlobalLock ( hgDrop );

	if ( NULL == pDrop )
	{
		GlobalFree ( hgDrop );
		return;
	}

	// Fill in the DROPFILES struct.

	pDrop->pFiles = sizeof(DROPFILES);

#ifdef _UNICODE
	// If we're compiling for Unicode, set the Unicode flag in the struct to
	// indicate it contains Unicode strings.
	pDrop->fWide = TRUE;
#endif;

	// Copy all the filenames into memory after the end of the DROPFILES struct.

	pos = lsDraggedFiles.GetHeadPosition();
	pszBuff = (TCHAR*) (LPBYTE(pDrop) + sizeof(DROPFILES));

	while ( NULL != pos )
	{
		lstrcpy ( pszBuff, (LPCTSTR) lsDraggedFiles.GetNext ( pos ) );
		pszBuff = 1 + _tcschr ( pszBuff, '\0' );
	}

	GlobalUnlock ( hgDrop );

	// Put the data in the data source.

	datasrc.CacheGlobalData ( CF_HDROP, hgDrop, &etc );

	// Add in our own custom data, so we know that the drag originated from our 
	// window.  CMyDropTarget::DragEnter() checks for this custom format, and
	// doesn't allow the drop if it's present.  This is how we prevent the user
	// from dragging and then dropping in our own window.
	// The data will just be a dummy bool.
	HGLOBAL hgBool;

	hgBool = GlobalAlloc ( GHND | GMEM_SHARE, sizeof(bool) );

	if ( NULL == hgBool )
	{
		GlobalFree ( hgDrop );
		return;
	}

	// Put the data in the data source.

	etc.cfFormat = g_uCustomClipbrdFormat;

	datasrc.CacheGlobalData ( g_uCustomClipbrdFormat, hgBool, &etc );


	// Start the drag 'n' drop!

	DROPEFFECT dwEffect = datasrc.DoDragDrop ( DROPEFFECT_COPY | DROPEFFECT_MOVE );

	// If the DnD completed OK, we remove all of the dragged items from our
	// list.

	
	switch ( dwEffect )
	{
	case DROPEFFECT_MOVE:
		{
			theApp.emuledlg->sharedfileswnd->Reload();
		}
	case DROPEFFECT_COPY:
		{	
			// Note: Don't call GlobalFree() because the data will be freed by the drop target.
		}
		break;

	case DROPEFFECT_NONE:
		{
			// This needs special handling, because on NT, DROPEFFECT_NONE
			// is returned for move operations, instead of DROPEFFECT_MOVE.
			// See Q182219 for the details.
			// So if we're on NT, we check each selected item, and if the
			// file no longer exists, it was moved successfully and we can
			// remove it from the list.

			if ( g_bNT )
			{
				bool bDeletedAnything = false;

				pos = lsDraggedFiles.GetHeadPosition();
				while ( NULL != pos )
				{
					
					if ( GetFileAttributes ( lsDraggedFiles.GetNext ( pos ) ) == INVALID_FILE_ATTRIBUTES
					 && GetLastError() == ERROR_FILE_NOT_FOUND )
					{
						// We couldn't read the file's attributes, and GetLastError()
						// says the file doesn't exist, so remove the corresponding 
						// item from the list.

						bDeletedAnything = true;
						break;
					}
				}

				if ( bDeletedAnything )
				{
					theApp.emuledlg->sharedfileswnd->Reload();

					// Note: Don't call GlobalFree() because the data belongs to 
					// the caller.
				}
				else
				{
					// The DnD operation wasn't accepted, or was canceled, so we 
					// should call GlobalFree() to clean up.

					GlobalFree ( hgDrop );
					GlobalFree ( hgBool );
				}
			}   // end if (NT)
			else
			{
				// We're on 9x, and a return of DROPEFFECT_NONE always means
				// that the DnD operation was aborted.  We need to free the
				// allocated memory.

				GlobalFree ( hgDrop );
				GlobalFree ( hgBool );
			}
		}
		break;  // end case DROPEFFECT_NONE
	}   // end switch
}

/*ON_WM_DROPFILES()
void CSharedFilesCtrl::OnDropFiles(HDROP hdrop) 
{
	TCHAR       szNextFile [MAX_PATH];
    UINT		uNumFiles = DragQueryFile ( hdrop, (UINT)-1, NULL, 0 ); // Get the # of files being dropped.
    for ( UINT uFile = 0; uFile < uNumFiles; uFile++ )
    {
        if ( DragQueryFile ( hdrop, uFile, szNextFile, MAX_PATH ) > 0 ) // Get the next filename from the HDROP info.
        {
			TRACE(_T("\n\n%s\n\n"),CStringA(szNextFile));
            
        } // end if
    } // end for
}
*/
// NEO: SDD END <-- Xanatos --

// NEO: CRC - [MorphCRCTag] -- Xanatos -->
// File will be renamed in this method at the time it can be renamed.
// Might be that the CRC had to be calculated before so the thread will inform
// this window when the CRC is ok and the file can be renamed...
afx_msg LRESULT CSharedFilesCtrl::OnCRC32RenameFile	(WPARAM /*wParam*/, LPARAM lParam) {
	// Get the worker thread
	CCRC32RenameWorker* worker = (CCRC32RenameWorker*) lParam;
	// In this case the worker thread is a helper thread for this routine !

	// We are in the "main" thread, so we can be sure that the filelist is not
	// deleted while we access it - so we try to get a pointer to the desired file
	// directly without worker->ValidateKnownFile !
	// This of course avoids possible deadlocks because we don't lock the list;
	// and we don't need to do an worker->UnlockSharedFilesList...
	CKnownFile* f = theApp.sharedfiles->GetFileByID (worker->GetFileHashToProcess ());
	if (f==NULL) {
		// File doesn't exist in the list; deleted and reloaded the shared files list in
		// the meantime ?
		// Let's hope the creator of this Worker thread has set the filename so we can
		// display it...
		if (worker->GetFilePath () == "") {
			ModLog(GetResString(IDS_X_CRC_TAGING_WRN1));
		} else {
			ModLog(GetResString(IDS_X_CRC_TAGING_WRN2),
				worker->GetFilePath ());
		}
		return 0;         
	}
	if (f->IsPartFile () && !worker->m_DontAddCRCAndSuffix) {     
		// We can't add a CRC suffix to files which are not complete
		ModLog(GetResString(IDS_X_CRC_TAGING_PARTFILE),
						   f->GetFileName ());
		return 0;
	}
	if (!worker->m_DontAddCRCAndSuffix && !f->IsCRC32Calculated ()) {
		// The CRC must have been calculate, otherwise we can't add it.
		// Normally this mesage is not shown because if the CRC is not calculated
		// the main thread creates a worker thread before to calculate it...
		ModLog(GetResString(IDS_X_CRC_TAGING_MISSING),
						   f->GetFileName ());
		return 0;
	}

	// Declare the variables we'll need
	CString p3,p4;
	CString NewFn;

	CString buffer;
	buffer.Format(_T("%02X%02X%02X%02X"),	(int) f->GetCalculatedCRC32() [3],
											(int) f->GetCalculatedCRC32() [2],
											(int) f->GetCalculatedCRC32() [1],
											(int) f->GetCalculatedCRC32() [0]);

	// Split the old filename to name and extension
	CString fn = f->GetFileName ();
	// test if the file name already contained the CRC tag
	CString fnup = fn;
	fnup.MakeUpper();
	if( f->IsCRC32Calculated() && 
		(fnup.Find(buffer) != -1) &&
		(!worker->m_CRC32ForceAdding) ){
		// Ok, the filename already contains the CRC. Normally we won't rename it, except for
		// we have to make sure it's uppercase
		if ((!worker->m_CRC32ForceUppercase) || (fn.Find(buffer) != -1)) {
			ModLog(GetResString(IDS_X_CRC_TAGING_ALREADY_TAGED), fn);
			return 0;
		} else {
			// This file contains a valid CRC, but not in uppercase - replace it!
			int i=fnup.Find(buffer);
			NewFn = fn;
			p3 = f->GetCalculatedCRC32();
			NewFn.Delete (i,p3.GetLength ());
			NewFn.Insert (i,p3);
		}
	} else {
		// We have to add the CRC32/Releaser tag to the filename.
		_tsplitpath (fn,NULL,NULL,p3.GetBuffer (MAX_PATH),p4.GetBuffer (MAX_PATH));
		p3.ReleaseBuffer();
		p4.ReleaseBuffer();

		// Create the new filename
		NewFn = p3;
		NewFn = NewFn + worker->m_FilenamePrefix;
		if (!worker->m_DontAddCRCAndSuffix) {
			NewFn = NewFn + buffer + worker->m_FilenameSuffix;
		}
		NewFn = NewFn + p4;
	}

	ModLog(GetResString(IDS_X_CRC_TAGING_RENAME),fn,NewFn);

	// Add the path of the old filename to the new one
	CString NewPath; 
	PathCombine(NewPath.GetBuffer(MAX_PATH), f->GetPath (), NewFn);
	NewPath.ReleaseBuffer();

	bool bPartFile = f->IsPartFile();
	bool bShareFile = theApp.sharedfiles->IsFilePtrInList(f); // NEO: AKF - [AllKnownFiles]
	// Try to rename
	if (!bPartFile && bShareFile && (_trename(f->GetFilePath (), NewPath) != 0)) { // NEO: AKF - [AllKnownFiles]
		ModLog(GetResString(IDS_X_CRC_TAGING_RENAME_ERROR),fn,_tcserror(errno));
	} else {
		if (!bPartFile) {
			// Use the "Format"-Syntax of AddLogLine here instead of
			// CString.Format+AddLogLine, because if "%"-characters are
			// in the string they would be misinterpreted as control sequences!
			AddLogLine(false,_T("Successfully renamed file '%s' to '%s'"), f->GetFileName(), NewPath);

			f->SetFileName(NewFn);
			if(bShareFile) // NEO: AKF - [AllKnownFiles]
				f->SetFilePath(NewPath);

			UpdateFile (f);
		} else {
			// Use the "Format"-Syntax of AddLogLine here instead of
			// CString.Format+AddLogLine, because if "%"-characters are
			// in the string they would be misinterpreted as control sequences!
			AddLogLine(false,_T("Successfully renamed .part file '%s' to '%s'"), f->GetFileName(), NewFn);

			f->SetFileName(NewFn, true); 
			((CPartFile*) f)->UpdateDisplayedInfo();
			((CPartFile*) f)->SavePartFile(); 
			UpdateFile(f);
		}
	}
	
	return 0;
}

// Update the file which CRC was just calculated.
// The LPARAM parameter is a pointer to the hash of the file to be updated.
LRESULT CSharedFilesCtrl::OnCRC32UpdateFile	(WPARAM /*wParam*/, LPARAM lParam) {
	uchar* filehash = (uchar*) lParam;
	// We are in the "main" thread, so we can be sure that the filelist is not
	// deleted while we access it - so we try to get a pointer to the desired file
	// directly without worker->ValidateKnownFile !
	// This of course avoids possible deadlocks because we don't lock the list;
	// and we don't need to do an worker->UnlockSharedFilesList...
	CKnownFile* file = theApp.sharedfiles->GetFileByID (filehash);
	if (file != NULL)		// Update the file if it exists
		UpdateFile (file);
	return 0;
}
// NEO: CRC END <-- Xanatos --

// NEO: NPT - [NeoPartTraffic] -- Xanatos -->
COLORREF GetTrafficColor0(float f)
{
	//--- COLORING MODE 1 --- RAINBOW ---
	//--- 0..1   black to red ---
	if(f<1)
		return RGB((BYTE)(255*f), 0, 0);
	
	//--- 1..3   red to yellow ---
	if(f<3)
		return RGB(255, (uint8)(255*(f-1)/2.0), 0);

	//--- 3..6   yellow to green ---
	if(f<6)
		return RGB((uint8)(255*((6-f)/3.0)), 255, 0);

	//--- 6..10   green to cyan ---
	if(f<10)
		return RGB(0, 255, (uint8)(255*((f-6)/4.0)));

	//--- 10..15   cyan to blue ---
	if(f<15)
		return RGB(0, (uint8)(255*((15-f)/5.0)), 255);

	//--- 15..21   blue to pink ---
	if(f<21)
		return RGB((uint8)(255*((f-15)/6.0)), 0, 255);

	//--- 21..28   pink to white ---
	if(f<28)
		return RGB(255, (uint8)(255*((28-f)/7.0)), 255);

	return RGB(255, 255, 255);
}

COLORREF GetTrafficColor1(float f)
{
	//--- COLORING MODE 1 --- RAINBOW ---
	//--- 0..1   black to red ---
	if(f<1)
		return RGB((BYTE)(255*f), 0, 0);
	
	//--- 1..2   red to yellow ---
	if(f<2)
		return RGB(255, (uint8)(255*(f-1)), 0);

	//--- 2..3   yellow to green ---
	if(f<3)
		return RGB((uint8)(255*((3-f))), 255, 0);

	//--- 3..4   green to cyan ---
	if(f<4)
		return RGB(0, 255, (uint8)(255*((f-3))));

	//--- 4..5   cyan to blue ---
	if(f<5)
		return RGB(0, (uint8)(255*((5-f))), 255);

	//--- 5..6   blue to pink ---
	if(f<6)
		return RGB((uint8)(255*((f-5))), 0, 255);

	//--- 6..7   pink to white ---
	if(f<7)
		return RGB(255, (uint8)(255*((7-f))), 255);

	return RGB(255, 255, 255);
}

COLORREF GetTrafficColor2(float f)
{
	//--- COLORING MODE 2 --- like in download ---
	return RGB(0, (210-(22*(f-1)) <  0)? 0:210-(22*(f-1)), 255);	
}

COLORREF GetTrafficColor3(float f)
{
	//--- girlie mode ---
	return RGB(255, 0, (210-(22*(f-1)) <  0)? 0:210-(22*(f-1)));	
}

void CSharedFilesCtrl::SetColoring(int mode)
{
	switch(mode)
	{
	case 0:
		GetTrafficColor=&GetTrafficColor0;
		break;

	case 1:
		GetTrafficColor=&GetTrafficColor1;
		break;

	case 2:
		GetTrafficColor=&GetTrafficColor2;
		break;

	case 3:
		GetTrafficColor=&GetTrafficColor3;
		break;

	default:
		GetTrafficColor=&GetTrafficColor0;
	}
}
// NEO: NPT END <-- Xanatos --
