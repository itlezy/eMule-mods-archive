//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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
#include "SharedDirsTreeCtrl.h"
#include "preferences.h"
#include "otherfunctions.h"
#include "SharedFilesCtrl.h"
#include "Knownfile.h"
#include "MenuCmds.h"
#include "partfile.h"
#include "emuledlg.h"
#include "TransferWnd.h"
#include "SharedFileList.h"
#include "SharedFilesWnd.h"
#include "Neo/Functions.h" // NEO: MOD <-- Xanatos --
// NEO: NSC - [NeoSharedCategories] -- Xanatos -->
#include "CatDialog.h"
#include "DownloadQueue.h"
#include "KnownFileList.h"
#include "SearchDlg.h"
// NEO: NSC END <-- Xanatos --
#include "Neo/CP/FilePreferencesDialog.h" // NEO: FCFG - [FileConfiguration] <-- Xanatos --
#include "Neo/NeoPreferences.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --
#include "inputbox.h" // NEO: VSF - [VirtualSharedFiles] <-- Xanatos --
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
#include "Neo/VooDoo/voodoo.h"
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TM_FOUNDNETWORKDRIVE		(WM_USER + 0x101)	// NEO: SSD - [ShareSubDirectories] <-- Xanatos --

//**********************************************************************************
// CDirectoryItem

CDirectoryItem::CDirectoryItem(CString strFullPath, HTREEITEM htItem, ESpecialDirectoryItems eItemType, int nCatFilter){
	m_htItem = htItem;
	m_strFullPath = strFullPath;
	m_eItemType = eItemType;
	m_nCatFilter = nCatFilter;
}
	
CDirectoryItem::~CDirectoryItem(){
	while (liSubDirectories.GetHeadPosition() != NULL){
		delete liSubDirectories.RemoveHead();
	}
}

// search tree for a given filter
HTREEITEM CDirectoryItem::FindItem(CDirectoryItem* pContentToFind) const
{
	if (pContentToFind == NULL){
		ASSERT( false );
		return NULL;
	}

	if (pContentToFind->m_eItemType == m_eItemType && pContentToFind->m_strFullPath == m_strFullPath && pContentToFind->m_nCatFilter == m_nCatFilter)
		return m_htItem;

	POSITION pos = liSubDirectories.GetHeadPosition();
	while (pos != NULL){
		CDirectoryItem* pCurrent = liSubDirectories.GetNext(pos);
		HTREEITEM htResult;
		if ( (htResult = pCurrent->FindItem(pContentToFind)) != NULL)
			return htResult;
	}
	return NULL;
}

//**********************************************************************************
// CSharedDirsTreeCtrl


IMPLEMENT_DYNAMIC(CSharedDirsTreeCtrl, CTreeCtrl)

BEGIN_MESSAGE_MAP(CSharedDirsTreeCtrl, CTreeCtrl)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_CANCELMODE()
	ON_WM_LBUTTONUP()
	ON_WM_SYSCOLORCHANGE()
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, OnTvnItemexpanding)
	ON_NOTIFY_REFLECT(TVN_GETDISPINFO, OnTvnGetdispinfo)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnLvnBegindrag)
	ON_MESSAGE(TM_FOUNDNETWORKDRIVE, OnFoundNetworkDrive) // NEO: SSD - [ShareSubDirectories] <-- Xanatos --
	ON_WM_MEASUREITEM() // NEO: NMX - [NeoMenuXP] <-- Xanatos --
END_MESSAGE_MAP()

CSharedDirsTreeCtrl::CSharedDirsTreeCtrl()
{
	m_pRootDirectoryItem = NULL;
	m_bCreatingTree = false;
	m_pSharedFilesCtrl = NULL;
	m_pRootUnsharedDirectries = NULL;
	m_pDraggingItem = NULL;
	// NEO: SSD - [ShareSubDirectories] -- Xanatos -->
	NetworkThreadOffline = new CEvent(1, 1);
	NetworkThreadRun = true;	// quick exit
	// NEO: SSD END <-- Xanatos --
	m_iCategories = 0; // NEO: NSC - [NeoSharedCategories] <-- Xanatos --
}

CSharedDirsTreeCtrl::~CSharedDirsTreeCtrl()
{
	// NEO: FIX - [DestroyMenu] -- Xanatos -->
	if (m_PrioMenu) VERIFY( m_PrioMenu.DestroyMenu() );
	if (m_CatMenu) VERIFY( m_CatMenu.DestroyMenu() ); // NEO: NSC - [NeoSharedCategories]
	if (m_PermMenu) VERIFY( m_PermMenu.DestroyMenu() ); // NEO: SSP - [ShowSharePermissions]
	if (m_BoostMenu) VERIFY( m_BoostMenu.DestroyMenu() ); // NEO: NXC - [NewExtendedCategories]
	if (m_VirtualDirMenu) VERIFY (m_VirtualDirMenu.DestroyMenu()); // NEO: VSF - [VirtualSharedFiles]
	if (m_SharedFilesMenu) VERIFY( m_SharedFilesMenu.DestroyMenu() );
	if (m_ShareDirsMenu) VERIFY( m_ShareDirsMenu.DestroyMenu() );
	// NEO: FIX END <-- Xanatos --

	// NEO: SSD - [ShareSubDirectories] -- Xanatos -->
	NetworkThreadRun = false;	// quick exit
	NetworkThreadOffline->Lock();
	delete NetworkThreadOffline;
	// NEO: SSD END <-- Xanatos --
	delete m_pRootDirectoryItem;
	delete m_pRootUnsharedDirectries;
}

void CSharedDirsTreeCtrl::Initalize(CSharedFilesCtrl* pSharedFilesCtrl){
	m_pSharedFilesCtrl = pSharedFilesCtrl;
	
	// Win98: Explicitly set to Unicode to receive Unicode notifications.
	SendMessage(CCM_SETUNICODEFORMAT, TRUE);

	//WORD wWinVer = thePrefs.GetWindowsVersion();
	m_bUseIcons = true;/*(wWinVer == _WINVER_2K_ || wWinVer == _WINVER_XP_ || wWinVer == _WINVER_ME_);*/
	SetAllIcons();
	InitalizeStandardItems();
	FilterTreeReloadTree();
	CreateMenues();
}

void CSharedDirsTreeCtrl::OnSysColorChange()
{
	CTreeCtrl::OnSysColorChange();
	SetAllIcons();
	CreateMenues();
}

void CSharedDirsTreeCtrl::SetAllIcons()
{
	// This treeview control contains an image list which contains our own icons and a
	// couple of icons which are copied from the Windows System image list. To properly
	// support an update of the control and the image list, we need to 'replace' our own
	// images so that we are able to keep the already stored images from the Windows System
	// image list.
	CImageList *pCurImageList = GetImageList(TVSIL_NORMAL);
	if (pCurImageList != NULL && pCurImageList->GetImageCount() >= 7)
	//if (pCurImageList != NULL && pCurImageList->GetImageCount() >= 15) // NEO: EFT - [ed2kFileType] <-- Xanatos --
	{
		pCurImageList->Replace(0, CTempIconLoader(_T("AllFiles")));			// 0: All Directory
		pCurImageList->Replace(1, CTempIconLoader(_T("Incomplete")));		// 1: Temp Directory
		pCurImageList->Replace(2, CTempIconLoader(_T("Incoming")));			// 2: Incoming Directory
		pCurImageList->Replace(3, CTempIconLoader(_T("Category")));			// 3: Cats
		pCurImageList->Replace(4, CTempIconLoader(_T("HardDisk")));			// 4: All Dirs
		CString strTempDir(thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR));
		if (strTempDir.Right(1) != _T("\\"))
			strTempDir += _T("\\");
		int nImage = theApp.GetFileTypeSystemImageIdx(strTempDir);			// 5: System Folder Icon
		if (nImage > 0 && theApp.GetSystemImageList() != NULL) {
			HICON hIcon = ::ImageList_GetIcon(theApp.GetSystemImageList(), nImage, 0);
			pCurImageList->Replace(5, hIcon);
			DestroyIcon(hIcon);
		}
		else{
			pCurImageList->Replace(5, CTempIconLoader(_T("OpenFolder")));
		}
		pCurImageList->Replace(6, CTempIconLoader(_T("SharedFolderOvl")));	// 6: Overlay

		// NEO: EFT - [ed2kFileType] -- Xanatos -->
		//pCurImageList->Replace(7, CTempIconLoader(_T("SearchFileType_Audio")));	// 7: 
		//pCurImageList->Replace(8, CTempIconLoader(_T("SearchFileType_Video")));	// 8: 
		//pCurImageList->Replace(9, CTempIconLoader(_T("SearchFileType_Picture")));	// 9: 
		//pCurImageList->Replace(10, CTempIconLoader(_T("SearchFileType_Program")));	// 10: 
		//pCurImageList->Replace(11, CTempIconLoader(_T("SearchFileType_Document")));	// 11: 
		//pCurImageList->Replace(12, CTempIconLoader(_T("SearchFileType_Archive")));	// 12: 
		//pCurImageList->Replace(13, CTempIconLoader(_T("SearchFileType_CDImage")));	// 13: 
		//pCurImageList->Replace(14, CTempIconLoader(_T("SearchFileType_EmuleCollection")));	// 14: 
		// NEO: EFT END <-- Xanatos --

	}
	else
	{
		CImageList iml;
		iml.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);
		iml.Add(CTempIconLoader(_T("AllFiles")));				// 0: All Directory
		iml.Add(CTempIconLoader(_T("Incomplete")));				// 1: Temp Directory
		iml.Add(CTempIconLoader(_T("Incoming")));				// 2: Incoming Directory
		iml.Add(CTempIconLoader(_T("Category")));				// 3: Cats
		iml.Add(CTempIconLoader(_T("HardDisk")));				// 4: All Dirs
		CString strTempDir(thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR));
		if (strTempDir.Right(1) != _T("\\"))
			strTempDir += _T("\\");
		int nImage = theApp.GetFileTypeSystemImageIdx(strTempDir);// 5: System Folder Icon
		if (nImage > 0 && theApp.GetSystemImageList() != NULL){
			HICON hIcon = ::ImageList_GetIcon(theApp.GetSystemImageList(), nImage, 0);
			iml.Add(hIcon);
			DestroyIcon(hIcon);
		}
		else{
			iml.Add(CTempIconLoader(_T("OpenFolder")));
		}
		iml.SetOverlayImage(iml.Add(CTempIconLoader(_T("SharedFolderOvl"))), 1); // 6: Overlay

		// NEO: EFT - [ed2kFileType] -- Xanatos -->
		//iml.Add(CTempIconLoader(_T("SearchFileType_Audio"), 16, 16));
		//iml.Add(CTempIconLoader(_T("SearchFileType_Video"), 16, 16));
		//iml.Add(CTempIconLoader(_T("SearchFileType_Picture"), 16, 16));
		//iml.Add(CTempIconLoader(_T("SearchFileType_Program"), 16, 16));
		//iml.Add(CTempIconLoader(_T("SearchFileType_Document"), 16, 16));
		//iml.Add(CTempIconLoader(_T("SearchFileType_Archive"), 16, 16));
		//iml.Add(CTempIconLoader(_T("SearchFileType_CDImage"), 16, 16));
		//iml.Add(CTempIconLoader(_T("SearchFileType_EmuleCollection"), 16, 16));
		// NEO: EFT END <-- Xanatos --

		SetImageList(&iml, TVSIL_NORMAL);
		m_mapSystemIcons.RemoveAll();
		m_imlTree.DeleteImageList();
		m_imlTree.Attach(iml.Detach());
	}

	COLORREF crBk = GetSysColor(COLOR_WINDOW);
	COLORREF crFg = GetSysColor(COLOR_WINDOWTEXT);
	theApp.LoadSkinColorAlt(_T("SharedDirsTvBk"), _T("DefLvBk"), crBk);
	theApp.LoadSkinColorAlt(_T("SharedDirsTvFg"), _T("DefLvFg"), crFg);
	SetBkColor(crBk);
	SetTextColor(crFg);
}

// NEO: EFT - [ed2kFileType] -- Xanatos -->
//struct SEd2kTypeView
//{
//	int eType;
//	RESSTRIDTYPE uStringID;
//} _aEd2kTypeView[] =
//{
//	{ ED2KFT_AUDIO, IDS2RESIDTYPE(IDS_SEARCH_AUDIO) },
//	{ ED2KFT_VIDEO, IDS2RESIDTYPE(IDS_SEARCH_VIDEO) },
//	{ ED2KFT_IMAGE, IDS2RESIDTYPE(IDS_SEARCH_PICS) },
//	{ ED2KFT_PROGRAM, IDS2RESIDTYPE(IDS_SEARCH_PRG) },
//	{ ED2KFT_DOCUMENT, IDS2RESIDTYPE(IDS_SEARCH_DOC) },
//	{ ED2KFT_ARCHIVE, IDS2RESIDTYPE(IDS_SEARCH_ARC) },
//	{ ED2KFT_CDIMAGE, IDS2RESIDTYPE(IDS_SEARCH_CDIMG) },
//	{ ED2KFT_EMULECOLLECTION, IDS2RESIDTYPE(IDS_SEARCH_EMULECOLLECTION) }
//};
// NEO: EFT END <-- Xanatos --

void CSharedDirsTreeCtrl::Localize(){
	InitalizeStandardItems();
	FilterTreeReloadTree();
	CreateMenues();
}

void CSharedDirsTreeCtrl::InitalizeStandardItems(){
	// add standard items
	DeleteAllItems();
	delete m_pRootDirectoryItem;
	delete m_pRootUnsharedDirectries;

	FetchSharedDirsList();

	m_pRootDirectoryItem = new CDirectoryItem(CString(_T("")), TVI_ROOT);
	CDirectoryItem* pAll = new CDirectoryItem(CString(_T("")), 0, SDI_ALL);
	pAll->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE, GetResString(IDS_ALLSHAREDFILES), 0, 0, TVIS_EXPANDED, TVIS_EXPANDED, (LPARAM)pAll, TVI_ROOT, TVI_LAST);
	m_pRootDirectoryItem->liSubDirectories.AddTail(pAll);
	
	// NEO: NSC - [NeoSharedCategories] -- Xanatos -->
	CDirectoryItem* pCategories = new CDirectoryItem(CString(_T("")), pAll->m_htItem, SDI_CATEGORIES);
	pCategories->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, GetResString(IDS_X_CATEGORIZED_FILES), 3, 3, 0, 0, (LPARAM)pCategories, TVI_ROOT, TVI_LAST);
	m_pRootDirectoryItem->liSubDirectories.AddTail(pCategories);
	// NEO: NSC END <-- Xanatos --

	CDirectoryItem* pIncoming = new CDirectoryItem(CString(_T("")), pAll->m_htItem, SDI_INCOMING);
	pIncoming->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, GetResString(IDS_INCOMING_FILES), 2, 2, 0, 0, (LPARAM)pIncoming, pAll->m_htItem, TVI_LAST);
	m_pRootDirectoryItem->liSubDirectories.AddTail(pIncoming);
	
	CDirectoryItem* pTemp = new CDirectoryItem(CString(_T("")), pAll->m_htItem, SDI_TEMP);
	pTemp->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, GetResString(IDS_INCOMPLETE_FILES), 1, 1, 0, 0, (LPARAM)pTemp, pAll->m_htItem, TVI_LAST);
	m_pRootDirectoryItem->liSubDirectories.AddTail(pTemp);

	CDirectoryItem* pDir = new CDirectoryItem(CString(_T("")), pAll->m_htItem, SDI_DIRECTORY);
	pDir->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE, GetResString(IDS_SHARED_DIRECTORIES), 5, 5, TVIS_EXPANDED, TVIS_EXPANDED, (LPARAM)pDir, pAll->m_htItem, TVI_LAST);
	m_pRootDirectoryItem->liSubDirectories.AddTail(pDir);

	m_pRootUnsharedDirectries = new CDirectoryItem(CString(_T("")), TVI_ROOT, SDI_FILESYSTEMPARENT);
	m_pRootUnsharedDirectries->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN, GetResString(IDS_ALLDIRECTORIES), 4, 4, 0, 0, (LPARAM)m_pRootUnsharedDirectries, TVI_ROOT, TVI_LAST);
}

bool CSharedDirsTreeCtrl::FilterTreeIsSubDirectory(CString strDir, CString strRoot, CStringList& liDirs){
	POSITION pos = liDirs.GetHeadPosition();
	strRoot.MakeLower();
	strDir.MakeLower();
	if (strDir.Right(1) != _T("\\")){
		strDir += _T("\\");
	}
	if (strRoot.Right(1) != _T("\\")){
		strRoot += _T("\\");
	}
	while (pos){
		//CString strCurrent = thePrefs.shareddir_list.GetNext(pos);
		CString strCurrent = liDirs.GetNext(pos); // NEO: SSD - [ShareSubDirectories] <-- Xanatos --
		strCurrent.MakeLower();
		if (strCurrent.Right(1) != _T("\\")){
			strCurrent += _T("\\");
		}
		if (strRoot.Find(strCurrent, 0) != 0 && strDir.Find(strCurrent, 0) == 0 && strCurrent != strRoot && strCurrent != strDir)
			return true;
	}
	return false;
}

void CSharedDirsTreeCtrl::FilterTreeAddSubDirectories(CDirectoryItem* pDirectory, CStringList& liDirs, int nLevel){
	// just some sanity check against too deep shared dirs
	// shouldnt be needed, but never trust the filesystem or a recursive function ;)
	if (nLevel > 14){
		ASSERT( false );
		return;
	}
	POSITION pos = liDirs.GetHeadPosition();
	CString strDirectoryPath = pDirectory->m_strFullPath;
	strDirectoryPath.MakeLower();
	while (pos){
		//CString strCurrent = thePrefs.shareddir_list.GetNext(pos);
		CString strCurrent = liDirs.GetNext(pos); // NEO: SSD - [ShareSubDirectories] <-- Xanatos --
		CString strCurrentLow = strCurrent;
		strCurrentLow.MakeLower();
		if ( (strDirectoryPath.IsEmpty() || strCurrentLow.Find(strDirectoryPath + _T("\\"), 0) == 0) && strCurrentLow != strDirectoryPath){
			if (!FilterTreeIsSubDirectory(strCurrentLow, strDirectoryPath, liDirs)){
				CString strName = strCurrent;
				if (strName.Right(1) == _T("\\")){
					strName = strName.Left(strName.GetLength()-1);
				}
				strName = strName.Right(strName.GetLength() - (strName.ReverseFind(_T('\\'))+1));
				CDirectoryItem* pNewItem = new CDirectoryItem(strCurrent);
				pNewItem->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, strName, 5, 5, 0, 0, (LPARAM)pNewItem, pDirectory->m_htItem, TVI_LAST);
				pDirectory->liSubDirectories.AddTail(pNewItem);
				FilterTreeAddSubDirectories(pNewItem, liDirs, nLevel+1);
		
			}
		}
	}
}


void CSharedDirsTreeCtrl::FilterTreeReloadTree(){
	m_bCreatingTree = true;
	// store current selection
	CDirectoryItem* pOldSelectedItem = NULL;
	if (GetSelectedFilter() != NULL){
		pOldSelectedItem = GetSelectedFilter()->CloneContent();
	}


	// create the tree substructure of directories we want to show
	POSITION pos = m_pRootDirectoryItem->liSubDirectories.GetHeadPosition();
	while (pos != NULL){
		CDirectoryItem* pCurrent = m_pRootDirectoryItem->liSubDirectories.GetNext(pos);
		// clear old items
		DeleteChildItems(pCurrent);

		switch( pCurrent->m_eItemType ){

			case SDI_ALL:{
				// NEO: EFT - [ed2kFileType] -- Xanatos -->
				//for (int i = 0; i < ARRSIZE(_aEd2kTypeView); i++)
				//{
				//	CDirectoryItem* pEd2kType = new CDirectoryItem(CString(""), 0, SDI_ED2KFILETYPE, _aEd2kTypeView[i].eType);
				//	pEd2kType->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, GetResString(_aEd2kTypeView[i].uStringID), i+7, i+7, 0, 0, (LPARAM)pEd2kType, pCurrent->m_htItem, TVI_LAST);
				//	pCurrent->liSubDirectories.AddTail(pEd2kType);
				//}
				// NEO: EFT END <-- Xanatos --
				break;
			}
			// NEO: NSC - [NeoSharedCategories] -- Xanatos -->
			case SDI_CATEGORIES:
				if (thePrefs.GetFullCatCount() > 1){
					for (int i = 0; i < thePrefs.GetFullCatCount(); i++){
						Category_Struct* pCatStruct = thePrefs.GetCategory(i);
						if (pCatStruct != NULL){
							//temp dir
							CDirectoryItem* pCatTemp = new CDirectoryItem(CString(""), 0, SDI_CATEGORIES, i);
							pCatTemp->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, CString(pCatStruct->strTitle), 3, 3, 0, 0, (LPARAM)pCatTemp, pCurrent->m_htItem, TVI_LAST);
							pCurrent->liSubDirectories.AddTail(pCatTemp);

						}
					}	
				}
				m_iCategories = thePrefs.GetFullCatCount();
				break;
			// NEO: NSC END <-- Xanatos --
			case SDI_INCOMING:{
				/*CString strMainIncDir = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR); // NEO: MOD <-- [SDI_INCOMING] --
				if (strMainIncDir.Right(1) == _T("\\")){
					strMainIncDir = strMainIncDir.Left(strMainIncDir.GetLength()-1);
				}*/
				//if (thePrefs.GetCatCount() > 1){
				if (thePrefs.GetFullCatCount() > 1){ // NEO: NSC - [NeoSharedCategories] <-- Xanatos --
					m_strliCatIncomingDirs.RemoveAll();
					//for (int i = 0; i < thePrefs.GetCatCount(); i++){
					for (int i = 0; i < thePrefs.GetFullCatCount(); i++){ // NEO: NSC - [NeoSharedCategories] <-- Xanatos --
						Category_Struct* pCatStruct = thePrefs.GetCategory(i);
						if (pCatStruct != NULL){
							//CString strCatIncomingPath = pCatStruct->strIncomingPath;
							CString strCatIncomingPath = thePrefs.GetCatPath(i); // NEO: NXC - [NewExtendedCategories] -- Xanatos --
							if (strCatIncomingPath.Right(1) == _T("\\")){
								strCatIncomingPath = strCatIncomingPath.Left(strCatIncomingPath.GetLength()-1);
							}
							if (!strCatIncomingPath.IsEmpty() /*&& strCatIncomingPath.CompareNoCase(strMainIncDir) != 0*/  // NEO: MOD <-- [SDI_INCOMING] --
								&& m_strliCatIncomingDirs.Find(strCatIncomingPath) == NULL)
							{
								m_strliCatIncomingDirs.AddTail(strCatIncomingPath);
								CString strName = strCatIncomingPath;
								if (strName.Right(1) == _T("\\")){
									strName = strName.Left(strName.GetLength()-1);
								}
								strName = strName.Right(strName.GetLength() - (strName.ReverseFind('\\')+1));
								CDirectoryItem* pCatInc = new CDirectoryItem(strCatIncomingPath, 0, SDI_CATINCOMING);
								pCatInc->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, strName, 5, 5, 0, 0, (LPARAM)pCatInc, pCurrent->m_htItem, TVI_LAST);
								pCurrent->liSubDirectories.AddTail(pCatInc);
							}
						}
					}
				}
				break;
			}
			case SDI_TEMP:
				// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
				/*CString strMainTempDir = thePrefs.GetTempDir(0);
				if (strMainTempDir.Right(1) == _T("\\")){
					strMainTempDir = strMainTempDir.Left(strMainTempDir.GetLength()-1);
				}*/
				if(thePrefs.GetTempDirCount() > 1){
					m_strliTempDirs.RemoveAll();
					for (int i = 0; i < thePrefs.GetTempDirCount(); i++){
						CString strTempPath = thePrefs.GetTempDir(i);
						if (strTempPath.Right(1) == _T("\\")){
							strTempPath = strTempPath.Left(strTempPath.GetLength()-1);
						}
						if (!strTempPath.IsEmpty() /*&& strTempPath.CompareNoCase(strMainTempDir) != 0*/
							&& m_strliTempDirs.Find(strTempPath) == NULL)
						{
							m_strliTempDirs.AddTail(strTempPath);
							CString strName = strTempPath;
							if (strName.Right(1) == _T("\\")){
								strName = strName.Left(strName.GetLength()-1);
							}
							strName = strName.Right(strName.GetLength() - (strName.ReverseFind(_T('\\'))+1));
							CDirectoryItem* pCatInc = new CDirectoryItem(strTempPath, 0, SDI_CATTEMP);
							pCatInc->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, strName, 5, 5, 0, 0, (LPARAM)pCatInc, pCurrent->m_htItem, TVI_LAST);
							pCurrent->liSubDirectories.AddTail(pCatInc);
						}
					}
				}
				// NEO: MTD END <-- Xanatos --
				/*if (thePrefs.GetCatCount() > 1){
					for (int i = 0; i < thePrefs.GetCatCount(); i++){
						Category_Struct* pCatStruct = thePrefs.GetCategory(i);
						if (pCatStruct != NULL){
							//temp dir
							CDirectoryItem* pCatTemp = new CDirectoryItem(CString(_T("")), 0, SDI_TEMP, i);
							pCatTemp->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, pCatStruct->strTitle, 3, 3, 0, 0, (LPARAM)pCatTemp, pCurrent->m_htItem, TVI_LAST);
							pCurrent->liSubDirectories.AddTail(pCatTemp);

						}
					}
				}*/
				break;
			case SDI_DIRECTORY:
				// NEO: SSD - [ShareSubDirectories] -- Xanatos -->
				{
					CStringList strSubdirs, strSubdirsUniform;	// We want all the shared directories
					for (POSITION pos = m_strliSharedDirsSubdir.GetHeadPosition(); pos != NULL; ) {
						CString scanDir = m_strliSharedDirsSubdir.GetNext(pos);
						strSubdirs.AddTail(scanDir);
						if (scanDir.Right(1) != _T("\\"))
							scanDir += _T("\\");
						scanDir.MakeLower();
						strSubdirsUniform.AddTail(scanDir);
					}
					for (POSITION pos = strSubdirs.GetHeadPosition(); pos != NULL; ) {
						CString scanDir = strSubdirs.GetNext(pos);
						if (scanDir.Right(1) != _T("\\"))
							scanDir += _T("\\");
						CFileFind finder;
						BOOL bWorking = finder.FindFile(scanDir+_T("*.*"));
						while (bWorking)
						{
							bWorking = finder.FindNextFile();
							if (finder.IsDots() || finder.IsSystem() || !finder.IsDirectory())
								continue;
							
							CString strFindDir = finder.GetFilePath();
							CString strFindDirUniform = strFindDir;
							if (strFindDirUniform.Right(1) != _T("\\"))
								strFindDirUniform += _T("\\");
							strFindDirUniform.MakeLower();
							if (!strSubdirsUniform.Find(strFindDirUniform))	// Prevent duplicates(for finity)
								strSubdirs.AddTail(strFindDir);	// We'll get to it later on
						}
						finder.Close();
					}
					// normal ones
					for (POSITION pos = m_strliSharedDirs.GetHeadPosition(); pos != NULL; ) {
							CString strFindDir = m_strliSharedDirs.GetNext(pos);
							CString strFindDirUniform = strFindDir;
							if (strFindDirUniform.Right(1) != _T("\\"))
								strFindDirUniform += _T("\\");
							strFindDirUniform.MakeLower();
							if (!strSubdirsUniform.Find(strFindDirUniform))	// Prevent duplicates(for finity)
								strSubdirs.AddTail(strFindDir);	// Add it
					}
					// add subdirectories
					FilterTreeAddSubDirectories(pCurrent, strSubdirs);
				}
				// NEO: SSD END <-- Xanatos --
				// add subdirectories
				//FilterTreeAddSubDirectories(pCurrent, m_strliSharedDirs);
				break;
			default:
				ASSERT( false );
		}
	}

	// restore selection
	HTREEITEM htOldSection;
	if (pOldSelectedItem != NULL && (htOldSection = m_pRootDirectoryItem->FindItem(pOldSelectedItem)) != NULL){
		Select(htOldSection, TVGN_CARET);
		EnsureVisible(htOldSection);
	}
	else if( GetSelectedItem() == NULL && !m_pRootDirectoryItem->liSubDirectories.IsEmpty()){
		Select(m_pRootDirectoryItem->liSubDirectories.GetHead()->m_htItem, TVGN_CARET);
	}
	delete pOldSelectedItem;
	m_bCreatingTree = false;
}

CDirectoryItem* CSharedDirsTreeCtrl::GetSelectedFilter() const{
	if (GetSelectedItem() != NULL)
		return (CDirectoryItem*)GetItemData(GetSelectedItem());
	else
		return NULL;
}

void CSharedDirsTreeCtrl::CreateMenues()
{
	if (m_PrioMenu) VERIFY( m_PrioMenu.DestroyMenu() );
	if (m_PermMenu) VERIFY( m_PermMenu.DestroyMenu() ); // NEO: SSP - [ShowSharePermissions] <-- Xanatos --
	if (m_CatMenu) VERIFY( m_CatMenu.DestroyMenu() ); // NEO: NSC - [NeoSharedCategories] <-- Xanatos --
	if (m_BoostMenu) VERIFY( m_BoostMenu.DestroyMenu() ); // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
	if (m_VirtualDirMenu) VERIFY (m_VirtualDirMenu.DestroyMenu()); // NEO: VSF - [VirtualSharedFiles] <-- Xanatos --
	if (m_SharedFilesMenu) VERIFY( m_SharedFilesMenu.DestroyMenu() );
	if (m_ShareDirsMenu) VERIFY( m_ShareDirsMenu.DestroyMenu() );

	

	m_PrioMenu.CreatePopupMenu(); // NEO: MOD - [CTitleMenu] <-- Xanatos --
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOVERYLOW,GetResString(IDS_PRIOVERYLOW));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOLOW,GetResString(IDS_PRIOLOW));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIONORMAL,GetResString(IDS_PRIONORMAL));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOHIGH, GetResString(IDS_PRIOHIGH));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOVERYHIGH, GetResString(IDS_PRIORELEASE));
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

	// NEO: NSC - [NeoSharedCategories] -- Xanatos -->
	m_CatMenu.CreatePopupMenu();
	m_CatMenu.AddMenuTitle(GetResString(IDS_X_SHARED_CATS), true);
	m_CatMenu.AppendMenu(MF_STRING,MP_CAT_ADD,GetResString(IDS_CAT_ADD),_T("CATADD"));
	m_CatMenu.AppendMenu(MF_STRING,MP_CAT_EDIT,GetResString(IDS_CAT_EDIT),_T("CATEDIT"));
	m_CatMenu.AppendMenu(MF_STRING,MP_TWEAKS, GetResString(IDS_X_CAT_TWEAKS), _T("CATCONFIG")); // NEO: FCFG - [FileConfiguration]
	m_CatMenu.AppendMenu(MF_STRING,MP_CAT_REMOVE, GetResString(IDS_CAT_REMOVE),_T("CATREMOVE"));
	// NEO: NSC END <-- Xanatos --

	// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
	m_BoostMenu.CreateMenu();
	m_BoostMenu.AddMenuTitle(GetResString(IDS_PRIORITY) + _T(" (") + GetResString(IDS_PW_CON_UPLBL) + _T(")"));
	m_BoostMenu.AppendMenu(MF_STRING, MP_PRIOLOW+100, GetResString(IDS_PRIOLOW));
	m_BoostMenu.AppendMenu(MF_STRING, MP_PRIONORMAL+100, GetResString(IDS_PRIONORMAL));
	m_BoostMenu.AppendMenu(MF_STRING, MP_PRIOHIGH+100, GetResString(IDS_PRIOHIGH));
	// NEO: SRS - [SmartReleaseSharing]
	m_BoostMenu.AppendMenu(MF_SEPARATOR);
	m_BoostMenu.AppendMenu(MF_STRING, MP_PRIORELEASE, GetResString(IDS_PRIORELEASE));
	// NEO: SRS END
	m_CatMenu.AppendMenu(MF_STRING | MF_POPUP, (UINT_PTR)m_BoostMenu.m_hMenu, GetResString(IDS_PRIORITY) + _T(" (") + GetResString(IDS_PW_CON_UPLBL) + _T(")"), _T("FILEPRIORITY"));
	// NEO: NXC END <-- Xanatos --

	m_SharedFilesMenu.CreatePopupMenu();
	m_SharedFilesMenu.AddMenuTitle(GetResString(IDS_SHAREDFILES), true);
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_OPENFOLDER, GetResString(IDS_OPENFOLDER), _T("OPENFOLDER"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_REMOVE, GetResString(IDS_DELETE), _T("DELETE"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_UNSHARE_FILE, GetResString(IDS_X_UNSHARE_FILE), _T("UNSHARE")); // NEO: SDD - [ShareDargAndDrop] <-- Xanatos --
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_PrioMenu.m_hMenu, GetResString(IDS_PRIORITY) + _T(" (") + GetResString(IDS_PW_CON_UPLBL) + _T(")"), _T("FILEPRIORITY"));
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_PermMenu.m_hMenu, GetResString(IDS_PERMISSION), _T("SHAREPERM")); // NEO: SSP - [ShowSharePermissions] <-- Xanatos --
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_VirtualDirMenu.m_hMenu, GetResString(IDS_X_VDS_VIRTDIRTITLE), _T("VIRTUALDIR")); // NEO: VSF - [VirtualSharedFiles] <-- Xanatos --
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);	
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_DETAIL, GetResString(IDS_SHOWDETAILS), _T("FILEINFO"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_CMT, GetResString(IDS_CMT_ADD), _T("FILECOMMENTS")); 
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_TWEAKS, GetResString(IDS_X_FILE_TWEAKS), _T("FILECONFIG")); // NEO: FCFG - [FileConfiguration] <-- Xanatos --

	if (thePrefs.GetShowCopyEd2kLinkCmd())
		m_SharedFilesMenu.AppendMenu(MF_STRING,MP_GETED2KLINK, GetResString(IDS_DL_LINK1), _T("ED2KLINK") );
	else
		m_SharedFilesMenu.AppendMenu(MF_STRING,MP_SHOWED2KLINK, GetResString(IDS_DL_SHOWED2KLINK), _T("ED2KLINK") );
	m_SharedFilesMenu.AppendMenu(MF_STRING | MF_SEPARATOR); // X-ToDo: Check merge
	m_SharedFilesMenu.AppendMenu(MF_STRING, MP_UNSHAREDIR, GetResString(IDS_UNSHAREDIR));
	m_SharedFilesMenu.AppendMenu(MF_STRING, MP_UNSHAREDIRSUB, GetResString(IDS_UNSHAREDIRSUB));

	// NEO: NSC - [NeoSharedCategories] -- Xanatos -->
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_CatMenu.m_hMenu, GetResString(IDS_X_SHARED_CATS), _T("CATEGORY"));
	// NEO: NSC END <-- Xanatos --

	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);

	m_ShareDirsMenu.CreatePopupMenu();
	m_ShareDirsMenu.AddMenuTitle(GetResString(IDS_SHAREDFILES), false);
	m_ShareDirsMenu.AppendMenu(MF_STRING,MP_SHAREDIR,GetResString(IDS_SHAREDIR));
	m_ShareDirsMenu.AppendMenu(MF_STRING,MP_SHAREDIRSUB,GetResString(IDS_SHAREDIRSUB));
	m_ShareDirsMenu.AppendMenu(MF_STRING|MF_SEPARATOR);	
	m_ShareDirsMenu.AppendMenu(MF_STRING,MP_UNSHAREDIR,GetResString(IDS_UNSHAREDIR));
	m_ShareDirsMenu.AppendMenu(MF_STRING,MP_UNSHAREDIRSUB,GetResString(IDS_UNSHAREDIRSUB));
	m_ShareDirsMenu.AppendMenu(MF_STRING|MF_SEPARATOR);	
	m_ShareDirsMenu.AppendMenu(MF_STRING,MP_OPENFOLDER,GetResString(IDS_OPENFOLDER));
}

void CSharedDirsTreeCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{	
	CDirectoryItem* pSelectedDir = GetSelectedFilter();
	if (pSelectedDir != NULL && pSelectedDir->m_eItemType != SDI_UNSHAREDDIRECTORY && pSelectedDir->m_eItemType != SDI_FILESYSTEMPARENT){
		int iSelectedItems = m_pSharedFilesCtrl->GetItemCount();
		// NEO: SP - [SharedParts] -- Xanatos -->
		CSharedItem* pSingleSelItem  = (iSelectedItems <= 0) ? NULL : (CSharedItem*)m_pSharedFilesCtrl->GetItemData(0);
		if (pSingleSelItem && !pSingleSelItem->isFile){
			return;
		}
		// NEO: SP END <-- Xanatos --
		int iCompleteFileSelected = -1;
		UINT uPrioMenuItem = 0;
		bool bCanUnshare = false; // NEO: SDD - [ShareDargAndDrop] <-- Xanatos --
		UINT uPermMenuItem = 0; // NEO: SSP - [ShowSharePermissions] <-- Xanatos --
		bool bVirtRemove = false; // NEO: VSF - [VirtualSharedFiles] <-- Xanatos --
		bool bFirstItem = true;
		for (int i = 0; i < iSelectedItems; i++)
		{
			// NEO: SP - [SharedParts] -- Xanatos -->
			const CSharedItem* pItem = (CSharedItem*)m_pSharedFilesCtrl->GetItemData(i);
			if(!pItem->isFile)
				continue;
			const CKnownFile* pFile = pItem->KnownFile;
			// NEO: SP END <-- Xanatos --
			//const CKnownFile* pFile = (CKnownFile*)m_pSharedFilesCtrl->GetItemData(i);

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

			// NEO: SDD - [ShareDargAndDrop] -- Xanatos -->
			if (!bCanUnshare) {
				for (POSITION pos = thePrefs.sharedfile_list.GetHeadPosition(); pos != 0; ) {
					if(thePrefs.sharedfile_list.GetNext(pos).CompareNoCase(MkPath(pFile->GetPath(),pFile->GetFileName())))
						continue;
					bCanUnshare = true;
					break;
				}
			}
			// NEO: SDD END <-- Xanatos --

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

		bool bWideRangeSelection = true;
		if(pSelectedDir->m_nCatFilter != -1 || pSelectedDir->m_eItemType == SDI_NO){
			// just avoid that users get bad ideas by showing the comment/delete-option for the "all" selections
			// as the same comment for all files/all incimplete files/ etc is probably not too usefull
			// - even if it can be done in other ways if the user really wants to do it
			bWideRangeSelection = false;
		}

		m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_PrioMenu.m_hMenu, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
		m_PrioMenu.CheckMenuRadioItem(MP_PRIOVERYLOW, MP_PRIOAUTO, uPrioMenuItem, 0);
		m_PrioMenu.CheckMenuItem(MP_PRIORELEASE, pSingleSelItem && pSingleSelItem->KnownFile->IsReleasePriority() ? MF_CHECKED : MF_UNCHECKED); // NEO: SRS - [SmartReleaseSharing] <-- Xanatos --
		// NEO: SSP - [ShowSharePermissions] -- Xanatos -->
		m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_PermMenu.m_hMenu, (iSelectedItems > 0 && NeoPrefs.UseShowSharePermissions()) ? MF_ENABLED : MF_GRAYED);
		m_PermMenu.CheckMenuRadioItem(MP_PERMALL, MP_PERMDEFAULT, uPermMenuItem, 0);
		// NEO: SSP END <-- Xanatos --
		// NEO: VSF - [VirtualSharedFiles] -- Xanatos -->
		m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_VirtualDirMenu.m_hMenu, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
		m_SharedFilesMenu.EnableMenuItem(MP_IOM_VIRTREMOVE, bVirtRemove ? MF_ENABLED : MF_GRAYED);
		// NEO: VSF END <-- Xanatos --

		m_SharedFilesMenu.EnableMenuItem(MP_OPENFOLDER, (pSelectedDir != NULL ) ? MF_ENABLED : MF_GRAYED);
		m_SharedFilesMenu.EnableMenuItem(MP_REMOVE, (iCompleteFileSelected > 0 && !bWideRangeSelection) ? MF_ENABLED : MF_GRAYED);
		// NEO: SDD - [ShareDargAndDrop] -- Xanatos -->
		if (!bCanUnshare) {
			for (POSITION pos = thePrefs.shareddir_list.GetHeadPosition(); pos != 0; ) {
				if (CompareDirectories(thePrefs.shareddir_list.GetNext(pos), pSelectedDir->m_strFullPath))
					continue;
				bCanUnshare = true;
				break;
			}
		}
		// NEO: SSD - [ShareSubDirectories]
		if (!bCanUnshare) {
			for (POSITION pos = thePrefs.sharedsubdir_list.GetHeadPosition(); pos != 0; ) {
				if(CompareSubDirectories(pSelectedDir->m_strFullPath,thePrefs.sharedsubdir_list.GetNext(pos)))
					continue;
				bCanUnshare = true;
				break;
			}
		}
		// NEO: SSD END
		m_SharedFilesMenu.EnableMenuItem(MP_UNSHARE_FILE, bCanUnshare ? MF_ENABLED : MF_GRAYED);
		// NEO: SDD END <-- Xanatos --
		m_SharedFilesMenu.EnableMenuItem(MP_CMT, (iSelectedItems > 0 && !bWideRangeSelection) ? MF_ENABLED : MF_GRAYED);
		m_SharedFilesMenu.EnableMenuItem(MP_DETAIL, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
		m_SharedFilesMenu.EnableMenuItem(MP_TWEAKS, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED); // NEO: FCFG - [FileConfiguration] <-- Xanatos --
		m_SharedFilesMenu.EnableMenuItem(thePrefs.GetShowCopyEd2kLinkCmd() ? MP_GETED2KLINK : MP_SHOWED2KLINK, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
		m_SharedFilesMenu.EnableMenuItem(MP_UNSHAREDIR, (pSelectedDir->m_eItemType == SDI_NO && !pSelectedDir->m_strFullPath.IsEmpty() && FileSystemTreeIsShared(pSelectedDir->m_strFullPath)) ? MF_ENABLED : MF_GRAYED);
		m_SharedFilesMenu.EnableMenuItem(MP_UNSHAREDIRSUB, (pSelectedDir->m_eItemType == SDI_NO && !pSelectedDir->m_strFullPath.IsEmpty() && (FileSystemTreeIsShared(pSelectedDir->m_strFullPath) || FileSystemTreeHasSharedSubdirectory(pSelectedDir->m_strFullPath))) ? MF_ENABLED : MF_GRAYED);
		// NEO: NSC - [NeoSharedCategories] -- Xanatos -->
		if(pSelectedDir->m_eItemType == SDI_CATEGORIES){
			m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_CatMenu.m_hMenu, MF_ENABLED); 

			m_CatMenu.EnableMenuItem(MP_CAT_REMOVE, pSelectedDir->m_nCatFilter == 0 ? MF_GRAYED : MF_ENABLED);

			// NEO: NXC - [NeoExtendedCategories]
			Category_Struct* curCat = thePrefs.GetCategory(pSelectedDir->m_nCatFilter); 
			if(curCat){
				m_BoostMenu.CheckMenuRadioItem(MP_PRIOLOW+100, MP_PRIOHIGH+100, MP_PRIOLOW+100+curCat->boost,0); 
				m_BoostMenu.CheckMenuItem(MP_PRIORELEASE,curCat->release ? MF_CHECKED : MF_UNCHECKED); // NEO: SRS - [SmartReleaseSharing]
			}
			// NEO: NXC END
		}
		else
			m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_CatMenu.m_hMenu, MF_GRAYED); 
		// NEO: NSC END <-- Xanatos --


		GetPopupMenuPos(*this, point);
		m_SharedFilesMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON,point.x,point.y,this);
	}
	else if(pSelectedDir != NULL && pSelectedDir->m_eItemType == SDI_UNSHAREDDIRECTORY){
		m_ShareDirsMenu.EnableMenuItem(MP_UNSHAREDIR, FileSystemTreeIsShared(pSelectedDir->m_strFullPath) ? MF_ENABLED : MF_GRAYED);
		m_ShareDirsMenu.EnableMenuItem(MP_UNSHAREDIRSUB, (FileSystemTreeIsShared(pSelectedDir->m_strFullPath) || FileSystemTreeHasSharedSubdirectory(pSelectedDir->m_strFullPath)) ? MF_ENABLED : MF_GRAYED);
		//m_ShareDirsMenu.EnableMenuItem(MP_SHAREDIR, !FileSystemTreeIsShared(pSelectedDir->m_strFullPath) ? MF_ENABLED : MF_GRAYED);
		m_ShareDirsMenu.EnableMenuItem(MP_SHAREDIR, !FileSystemTreeIsShared(pSelectedDir->m_strFullPath, true, true) ? MF_ENABLED : MF_GRAYED); // NEO: SSD - [ShareSubDirectories] <-- Xanatos --
		m_ShareDirsMenu.EnableMenuItem(MP_SHAREDIRSUB, FileSystemTreeHasSubdirectories(pSelectedDir->m_strFullPath) ? MF_ENABLED : MF_GRAYED);

		GetPopupMenuPos(*this, point);
		m_ShareDirsMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON,point.x,point.y,this);
	}
}

void CSharedDirsTreeCtrl::OnRButtonDown(UINT /*nFlags*/, CPoint point)
{
	UINT uHitFlags;
	HTREEITEM hItem = HitTest(point, &uHitFlags);
	if (hItem != NULL && (uHitFlags & TVHT_ONITEM))
	{
		Select(hItem, TVGN_CARET);
		SetItemState(hItem, TVIS_SELECTED, TVIS_SELECTED);
	}
	return;
}

BOOL CSharedDirsTreeCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	CTypedPtrList<CPtrList, CKnownFile*> selectedList;
	int iSelectedItems = m_pSharedFilesCtrl->GetItemCount();
	// NEO: SP - [SharedParts] -- Xanatos -->
	CSharedItem* pSingleSelItem  = (iSelectedItems <= 0) ? NULL : (CSharedItem*)m_pSharedFilesCtrl->GetItemData(0);
	if (pSingleSelItem && !pSingleSelItem->isFile){
		return TRUE;
	}
	// NEO: SP END <-- Xanatos --
	for (int i = 0; i < iSelectedItems; i++)
	{
		// NEO: SP - [SharedParts] -- Xanatos -->
		const CSharedItem* pItem = (CSharedItem*)m_pSharedFilesCtrl->GetItemData(i);
		if(!pItem->isFile)
			continue;
		selectedList.AddTail(pItem->KnownFile);
		// NEO: SP END <-- Xanatos --
		//selectedList.AddTail((CKnownFile*)m_pSharedFilesCtrl->GetItemData(i));
	}
	CDirectoryItem* pSelectedDir = GetSelectedFilter();

	// folder based
	if (pSelectedDir != NULL){
		switch (wParam){
			case MP_OPENFOLDER:
				if (pSelectedDir && !pSelectedDir->m_strFullPath.IsEmpty() /*&& pSelectedDir->m_eItemType == SDI_NO*/){
					ShellExecute(NULL, _T("open"), pSelectedDir->m_strFullPath, NULL, NULL, SW_SHOW);
				}
				else if (pSelectedDir && pSelectedDir->m_eItemType == SDI_INCOMING)
					ShellExecute(NULL, _T("open"), thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR), NULL, NULL, SW_SHOW);
				else if (pSelectedDir && pSelectedDir->m_eItemType == SDI_TEMP)
					ShellExecute(NULL, _T("open"), thePrefs.GetTempDir(), NULL, NULL, SW_SHOW);
				break;
			case MP_SHAREDIR:
				EditSharedDirectories(pSelectedDir, true, false);
				break;
			case MP_SHAREDIRSUB:
				EditSharedDirectories(pSelectedDir, true, true);
				break;
			case MP_UNSHAREDIR:
				EditSharedDirectories(pSelectedDir, false, false);
				break;
			case MP_UNSHAREDIRSUB:
				EditSharedDirectories(pSelectedDir, false, true);
				break;
		}
	}

	// NEO: MOD -- Xanatos -->
	if (pSelectedDir != NULL)
	{
		switch (wParam)
		{
			// NEO: SDD - [ShareDargAndDrop]
			case MP_UNSHARE_FILE:{
				if(pSelectedDir)
				{
					bool bUnshared = false;
					if (!bUnshared) {
						for (POSITION pos = thePrefs.shareddir_list.GetHeadPosition(); pos != 0; ) {
							POSITION remPos = pos;
							if (CompareDirectories(thePrefs.shareddir_list.GetNext(pos), pSelectedDir->m_strFullPath))
								continue;
							thePrefs.shareddir_list.RemoveAt(remPos);
							bUnshared = true;
							break;
						}
					}
					// NEO: SSD - [ShareSubDirectories]
					if (!bUnshared) {
						for (POSITION pos = thePrefs.sharedsubdir_list.GetHeadPosition(); pos != 0; ) {
							POSITION remPos = pos;
							if(CompareSubDirectories(pSelectedDir->m_strFullPath,thePrefs.sharedsubdir_list.GetNext(pos)))
								continue;
							thePrefs.sharedsubdir_list.RemoveAt(remPos);
							bUnshared = true;
							break;
						}
					}
					// NEO: SSD END

					// NEO: SSF - [ShareSingleFiles]
					if (!bUnshared) {
						POSITION pos = selectedList.GetHeadPosition();
						while (pos != NULL)
						{
							CKnownFile* file = selectedList.GetNext(pos);
							for (POSITION pos = thePrefs.sharedfile_list.GetHeadPosition(); pos != 0; ) {
								POSITION remPos = pos;
								if(thePrefs.sharedfile_list.GetNext(pos).CompareNoCase(MkPath(file->GetPath(),file->GetFileName())))
									continue;
								thePrefs.sharedfile_list.RemoveAt(remPos);
								bUnshared = true;
								break;
							}
						}
					}
					// NEO: SSF END

					theApp.emuledlg->sharedfileswnd->Reload();
				}
			}
			break; 
			// NEO: SDD END
		}
	}
	// NEO: MOD END <-- Xanatos --

	// file based
	if (selectedList.GetCount() > 0 && pSelectedDir != NULL)
	{
		CKnownFile* file = NULL;
		if (selectedList.GetCount() == 1)
			file = selectedList.GetHead();

		switch (wParam){
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
			// file operations
			case MP_REMOVE:
			case MPG_DELETE:{
				if (IDNO == AfxMessageBox(GetResString(IDS_CONFIRM_FILEDELETE),MB_ICONWARNING | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_YESNO))
					return TRUE;

				m_pSharedFilesCtrl->SetRedraw(FALSE);
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
					}
					else{
						CString strError;
						strError.Format( GetResString(IDS_ERR_DELFILE) + _T("\r\n\r\n%s"), myfile->GetFilePath(), GetErrorMessage(GetLastError()));
						AfxMessageBox(strError);
					}
				}
				m_pSharedFilesCtrl->SetRedraw(TRUE);
				if (bRemovedItems)
					m_pSharedFilesCtrl->AutoSelectItem();
				break; 
			}
			case MP_CMT:
				ShowFileDialog(selectedList, IDD_COMMENT);
                break; 
			case MP_DETAIL:
			case MPG_ALTENTER:
				ShowFileDialog(selectedList);
				break;
			// NEO: FCFG - [FileConfiguration] -- Xanatos -->
			case MP_TWEAKS:
				ShowFileDialog(selectedList,NULL,TRUE);
				break; 
			// NEO: FCFG END <-- Xanatos --
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
						m_pSharedFilesCtrl->UpdateFile(file); 
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
				m_pSharedFilesCtrl->SetRedraw(false);
				bool bNewState = !selectedList.GetHead()->IsReleasePriority();
				while (!selectedList.IsEmpty()){
					CKnownFile* knownfile = selectedList.GetHead();
					knownfile->SetReleasePriority(bNewState);
					selectedList.RemoveHead();
				}
				m_pSharedFilesCtrl->SetRedraw(true);
				break;
			}
			// NEO: SRS END <-- Xanatos --
			// NEO: SSP - [ShowSharePermissions] -- Xanatos -->
			case MP_PERMNONE:
			case MP_PERMFRIENDS:
			case MP_PERMALL: 
			case MP_PERMDEFAULT: 
			{
				m_pSharedFilesCtrl->SetRedraw(false);
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
				m_pSharedFilesCtrl->SetRedraw(true);
				break;
			}
			// NEO: SSP END <-- Xanatos --
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
			default:
				break;
		}
	}

	// NEO: MOD - [ContextMenu] -- Xanatos -->
	switch (wParam){
		// NEO: NSC - [NeoSharedCategories] -- Xanatos -->
		case MP_CAT_ADD:{
			int newindex=theApp.emuledlg->transferwnd->AddCategory(_T("?"),thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR),_T(""),_T(""),false,true);
			CCatDialog dialog(newindex);
			if (dialog.DoModal() == IDOK)
			{
				thePrefs.SaveCats();
				theApp.emuledlg->sharedfileswnd->Reload(!CompareDirectories(thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR), thePrefs.GetCatPath(newindex)));
			}
			else
				thePrefs.RemoveCat(newindex);
			break;
		}
		case MP_CAT_EDIT:{
			CString oldincpath=thePrefs.GetCatPath(pSelectedDir->m_nCatFilter);
			CCatDialog dialog(pSelectedDir->m_nCatFilter);
			if (dialog.DoModal() == IDOK)
			{
				if(pSelectedDir->m_nCatFilter <= (thePrefs.GetCatCount()-1))
				{
					theApp.emuledlg->transferwnd->EditCatTabLabel(pSelectedDir->m_nCatFilter, thePrefs.GetCategory(pSelectedDir->m_nCatFilter)->strTitle);
					theApp.emuledlg->transferwnd->m_dlTab.SetTabTextColor(pSelectedDir->m_nCatFilter, thePrefs.GetCatColor(pSelectedDir->m_nCatFilter) );
					theApp.emuledlg->searchwnd->UpdateCatTabs();
					theApp.emuledlg->transferwnd->downloadlistctrl.UpdateCurrentCategoryView();
				}

				thePrefs.SaveCats();
				theApp.emuledlg->sharedfileswnd->Reload(!CompareDirectories(oldincpath, thePrefs.GetCatPath(pSelectedDir->m_nCatFilter)));
			}
			break;
		}
		// NEO: NXC - [NewExtendedCategories]
		case MP_PRIOLOW+100:
			thePrefs.GetCategory(pSelectedDir->m_nCatFilter)->boost = PR_LOW;
			thePrefs.SaveCats();
			break;
		case MP_PRIONORMAL+100:
			thePrefs.GetCategory(pSelectedDir->m_nCatFilter)->boost = PR_NORMAL;
			thePrefs.SaveCats();
			break;
		case MP_PRIOHIGH+100:
			thePrefs.GetCategory(pSelectedDir->m_nCatFilter)->boost = PR_HIGH;
			thePrefs.SaveCats();
			break;
		// NEO: SRS - [SmartReleaseSharing]
		case MP_PRIORELEASE:
			thePrefs.GetCategory(pSelectedDir->m_nCatFilter)->release = !thePrefs.GetCategory(pSelectedDir->m_nCatFilter)->release;
			thePrefs.SaveCats();
			break;
		// NEO: SRS END
		// NEO: NXC END
		// NEO: FCFG - [FileConfiguration]
		case MP_TWEAKS:{
			Category_Struct* pCatStruct = thePrefs.GetCategory(pSelectedDir->m_nCatFilter);
			if (pCatStruct != NULL){
				// NEO: MLD - [ModelesDialogs] 
				CFilePreferencesDialog* dlg = new CFilePreferencesDialog(pCatStruct,pSelectedDir->m_nCatFilter > (thePrefs.GetCatCount()-1));
				dlg->OpenDialog(); 
				// NEO: MLD END
				//CFilePreferencesDialog dialog(pCatStruct,pSelectedDir->m_nCatFilter > (thePrefs.GetCatCount()-1));
				//dialog.DoModal();
			}
			break;
		}
		// NEO: FCFG END
		case MP_CAT_REMOVE:{
			bool toreload=( _tcsicmp(thePrefs.GetCatPath(pSelectedDir->m_nCatFilter), thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR))!=0);
			theApp.downloadqueue->ResetCatParts(pSelectedDir->m_nCatFilter);
			theApp.knownfiles->ResetCatParts(pSelectedDir->m_nCatFilter);
			thePrefs.RemoveCat(pSelectedDir->m_nCatFilter);
			thePrefs.SaveCats();
			// NEO: NXC - [NewExtendedCategories] -- Xanatos --
			//if (thePrefs.GetCatCount()==1) 
			//	thePrefs.GetCategory(0)->filter=0;
			theApp.emuledlg->sharedfileswnd->Reload(!toreload);
			break;
		}
		// NEO: NSC END <-- Xanatos --
	}
	// NEO: MOD END <-- Xanatos --
	return TRUE;
}

//void CSharedDirsTreeCtrl::ShowFileDialog(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage)
void CSharedDirsTreeCtrl::ShowFileDialog(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage, BOOL Preferences) // NEO: FCFG - [FileConfiguration] <-- Xanatos --
{
	//m_pSharedFilesCtrl->ShowFileDialog(aFiles, uPshInvokePage);
	m_pSharedFilesCtrl->ShowFileDialog(aFiles, uPshInvokePage, Preferences); // NEO: FCFG - [FileConfiguration] <-- Xanatos --
}

void CSharedDirsTreeCtrl::FileSystemTreeCreateTree()
{
	TCHAR drivebuffer[500];
	::GetLogicalDriveStrings(ARRSIZE(drivebuffer), drivebuffer); // e.g. "a:\ c:\ d:\"

	const TCHAR* pos = drivebuffer;
	while(*pos != _T('\0')){

		// Copy drive name
		TCHAR drive[4];
		_tcsncpy(drive, pos, ARRSIZE(drive));
		drive[ARRSIZE(drive) - 1] = _T('\0');

		switch(drive[0]){
			case _T('a'):
			case _T('A'):
			case _T('b'):
			case _T('B'):
			// Skip floppy disk
			break;
		default:
			drive[2] = _T('\0');
			FileSystemTreeAddChildItem(m_pRootUnsharedDirectries, drive, true); // e.g. ("c:")
		}

		// Point to the next drive (4 chars interval)
		pos = &pos[4];
	}

	// NEO: SSD - [ShareSubDirectories] -- Xanatos -->
	NetworkThreadRun = false;	// quick exit
	NetworkThreadOffline->Lock();	// Prevent a case of two running at the same time
	NetworkThreadRun = true;	// ready for startup
	NetworkThreadOffline->ResetEvent();	// Don't get deleted while the thread is running
	AfxBeginThread(EnumNetworkDrivesThreadProc, (LPVOID)this);
	// NEO: SSD END <-- Xanatos --
}

void CSharedDirsTreeCtrl::FileSystemTreeAddChildItem(CDirectoryItem* pRoot, CString strText, bool bTopLevel)
{
	CString strPath = pRoot->m_strFullPath;
	if (strPath.Right(1) != _T("\\") && !strPath.IsEmpty())
		strPath += _T("\\");
	CString strDir = strPath + strText;
	TV_INSERTSTRUCT itInsert;
	memset(&itInsert, 0, sizeof(itInsert));
	
	if(m_bUseIcons)		
	{
		itInsert.item.mask = TVIF_CHILDREN | TVIF_HANDLE | TVIF_TEXT | TVIF_STATE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		itInsert.item.stateMask = TVIS_BOLD | TVIS_STATEIMAGEMASK;
	}
	else{
		itInsert.item.mask = TVIF_CHILDREN | TVIF_HANDLE | TVIF_TEXT | TVIF_STATE;
		itInsert.item.stateMask = TVIS_BOLD;
	}

	
	if (FileSystemTreeHasSharedSubdirectory(strDir))
		itInsert.item.state = TVIS_BOLD;
	else
		itInsert.item.state = 0;

	if (FileSystemTreeHasSubdirectories(strDir))
		itInsert.item.cChildren = I_CHILDRENCALLBACK;		// used to display the + symbol next to each item
	else
		itInsert.item.cChildren = 0;

	if (strDir.Right(1) == _T("\\")){
		strDir = strDir.Left(strPath.GetLength()-1);
	}
	CDirectoryItem* pti = new CDirectoryItem(strDir, 0, SDI_UNSHAREDDIRECTORY);

	itInsert.item.pszText = const_cast<LPTSTR>((LPCTSTR)strText);
	itInsert.hInsertAfter = !bTopLevel ? TVI_SORT : TVI_LAST;
	itInsert.hParent = pRoot->m_htItem;
	itInsert.item.mask |= TVIF_PARAM;
	itInsert.item.lParam = (LPARAM)pti;
	
	if(m_bUseIcons)		
	{
		if (FileSystemTreeIsShared(strDir)){
			itInsert.item.stateMask |= TVIS_OVERLAYMASK;
			itInsert.item.state |= INDEXTOOVERLAYMASK(1);
		}

		CString strTemp = strDir;
		if(strTemp.Right(1) != _T("\\"))
			strTemp += _T("\\");
		
		UINT nType = GetDriveType(strTemp);
		if(DRIVE_REMOVABLE <= nType && nType <= DRIVE_RAMDISK)
			itInsert.item.iImage = nType;
	
		SHFILEINFO shFinfo;
		shFinfo.szDisplayName[0] = _T('\0');
		if(!SHGetFileInfo(strTemp, 0, &shFinfo,	sizeof(shFinfo),
						  SHGFI_ICON | SHGFI_SMALLICON | SHGFI_DISPLAYNAME))
		{
			TRACE(_T("Error Gettting SystemFileInfo!"));
			itInsert.itemex.iImage = 0; // :(
		}
		else
		{
			itInsert.itemex.iImage = AddSystemIcon(shFinfo.hIcon, shFinfo.iIcon);
			DestroyIcon(shFinfo.hIcon);
			if (bTopLevel && shFinfo.szDisplayName[0] != _T('\0'))
			{
				strText = shFinfo.szDisplayName;
				itInsert.item.pszText = const_cast<LPTSTR>((LPCTSTR)strText);
			}
		}

		if(!SHGetFileInfo(strTemp, 0, &shFinfo, sizeof(shFinfo), SHGFI_ICON | SHGFI_OPENICON | SHGFI_SMALLICON))
		{
			TRACE(_T("Error Gettting SystemFileInfo!"));
			itInsert.itemex.iImage = 0;
		}
		else{
			itInsert.itemex.iSelectedImage = AddSystemIcon(shFinfo.hIcon, shFinfo.iIcon);
			DestroyIcon(shFinfo.hIcon);
		}
	}

	pti->m_htItem = InsertItem(&itInsert);
	pRoot->liSubDirectories.AddTail(pti);
}

bool CSharedDirsTreeCtrl::FileSystemTreeHasSubdirectories(CString strDir)
{
	if (strDir.Right(1) != _T('\\'))
		strDir += _T('\\');
	CFileFind finder;
	BOOL bWorking = finder.FindFile(strDir+_T("*.*"));
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		if (finder.IsDots())
			continue;
		if (finder.IsSystem())
			continue;
		if (!finder.IsDirectory())
			continue;
		finder.Close();
		return true;
	}
	finder.Close();
	return false;
}

bool CSharedDirsTreeCtrl::FileSystemTreeHasSharedSubdirectory(CString strDir)
{
	// NEO: SSD - [ShareSubDirectories] -- Xanatos -->
	if (!FileSystemTreeHasSubdirectories(strDir))	// early check
		return false;
	// NEO: SSD END <-- Xanatos --
	if (strDir.Right(1) != _T('\\'))
		strDir += _T('\\');
	strDir.MakeLower();
	// NEO: SSD - [ShareSubDirectories] -- Xanatos -->
	CString istr = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
	if (istr.Right(1) != _T('\\'))
		istr += _T('\\');
	istr.MakeLower();
	if (istr.Find(strDir) == 0 && strDir != istr)
		return true;
	for (int ix=1;ix<thePrefs.GetCatCount();ix++)
	{
		istr = thePrefs.GetCatPath(ix);
		if (istr.Right(1) != _T("\\"))
			istr += _T("\\");
		istr.MakeLower();
		if (istr.Find(strDir) == 0 && strDir != istr)
			return true;
	}
	// NEO: SSD END <-- Xanatos --
	for (POSITION pos = m_strliSharedDirs.GetHeadPosition(); pos != NULL; )
	{
		CString strCurrent = m_strliSharedDirs.GetNext(pos);
		strCurrent.MakeLower();
		if (strCurrent.Find(strDir) == 0 && strDir != strCurrent)
			return true;
	}
	// NEO: SSD - [ShareSubDirectories] -- Xanatos -->
	for (POSITION pos = m_strliSharedDirsSubdir.GetHeadPosition(); pos != NULL; )
	{
		CString strCurrent = m_strliSharedDirsSubdir.GetNext(pos);
		strCurrent.MakeLower();
		if (strCurrent.Find(strDir) == 0 || strDir.Find(strCurrent) == 0)
			return true;
	}
	// NEO: SSD END <-- Xanatos --
	return false;
}

void CSharedDirsTreeCtrl::FileSystemTreeAddSubdirectories(CDirectoryItem* pRoot)
{
	CString strDir = pRoot->m_strFullPath;
	if (strDir.Right(1) != _T("\\"))
		strDir += _T("\\");
	CFileFind finder;
	BOOL bWorking = finder.FindFile(strDir+_T("*.*"));
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		if (finder.IsDots() || finder.IsSystem() || !finder.IsDirectory())
			continue;
		
		CString strFilename = finder.GetFileName();
		if (strFilename.ReverseFind(_T('\\')) != -1)
			strFilename = strFilename.Mid(strFilename.ReverseFind(_T('\\')) + 1);
		FileSystemTreeAddChildItem(pRoot, strFilename, false);
	}
	finder.Close();
}

int	CSharedDirsTreeCtrl::AddSystemIcon(HICON hIcon, int nSystemListPos){
	int nPos = 0;
	if (!m_mapSystemIcons.Lookup(nSystemListPos, nPos)){
		nPos = GetImageList(TVSIL_NORMAL)->Add(hIcon);
		m_mapSystemIcons.SetAt(nSystemListPos, nPos);
	}
	return nPos;
}

void CSharedDirsTreeCtrl::OnTvnItemexpanding(NMHDR *pNMHDR, LRESULT *pResult)
{
	CWaitCursor curWait;
	SetRedraw(FALSE);

	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	CDirectoryItem* pExpanded = (CDirectoryItem*)pNMTreeView->itemNew.lParam;
	if (pExpanded != NULL){
		if (pExpanded->m_eItemType == SDI_UNSHAREDDIRECTORY && !pExpanded->m_strFullPath.IsEmpty()){
			// remove all subitems
			DeleteChildItems(pExpanded);
			// fetch all subdirectories and add them to the node
			FileSystemTreeAddSubdirectories(pExpanded);
		}
		else if(pExpanded->m_eItemType == SDI_FILESYSTEMPARENT){
			DeleteChildItems(pExpanded);
			FileSystemTreeCreateTree();
		}
	}
	else
		ASSERT( false );

	SetRedraw(TRUE);
	Invalidate();
	*pResult = 0;
}

void CSharedDirsTreeCtrl::DeleteChildItems(CDirectoryItem* pParent){
	while(!pParent->liSubDirectories.IsEmpty()){
		CDirectoryItem* pToDelete = pParent->liSubDirectories.RemoveHead();
		DeleteItem(pToDelete->m_htItem);
		DeleteChildItems(pToDelete);
		delete pToDelete;
	}
}

//bool CSharedDirsTreeCtrl::FileSystemTreeIsShared(CString strDir)
bool CSharedDirsTreeCtrl::FileSystemTreeIsShared(CString strDir, bool bCheckParent, bool bCheckIncoming) // NEO: SSD - [ShareSubDirectories] <-- Xanatos --
{
	if (strDir.Right(1) != _T('\\'))
		strDir += _T('\\');
	for (POSITION pos = m_strliSharedDirs.GetHeadPosition(); pos != NULL; )
	{
		CString str = m_strliSharedDirs.GetNext(pos);
		if (str.Right(1) != _T('\\'))
			str += _T('\\');
		if (str.CompareNoCase(strDir) == 0)
			return true;
	}
	// NEO: SSD - [ShareSubDirectories] -- Xanatos -->
	if (bCheckParent)
		strDir.MakeLower();
	for (POSITION pos = m_strliSharedDirsSubdir.GetHeadPosition(); pos != NULL; )
	{
		CString str = m_strliSharedDirsSubdir.GetNext(pos);
		if (str.Right(1) != _T('\\'))
			str += _T('\\');
		if (str.CompareNoCase(strDir) == 0 || (bCheckParent && strDir.Find(str.MakeLower()) == 0))
			return true;
	}
	if (bCheckIncoming) {
		// Don't single-share incoming directorys, since they're auto-shared
		CString istr = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
		if (istr.Right(1) != _T("\\"))
			istr += _T("\\");
		if (istr.CompareNoCase(strDir) == 0)
			return true;
		for (int ix=1;ix<thePrefs.GetCatCount();ix++)
		{
			istr = thePrefs.GetCatPath(ix);
			if (istr.Right(1) != _T("\\"))
				istr += _T("\\");
			if (istr.CompareNoCase(strDir) == 0)
				return true;
		}
	}
	// NEO: SSD END <-- Xanatos --
	return false;
}

void CSharedDirsTreeCtrl::OnTvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
	pTVDispInfo->item.cChildren = 1;
	*pResult = 0;
}

void CSharedDirsTreeCtrl::AddSharedDirectory(CString strDir, bool bSubDirectories){
	// NEO: SSD - [ShareSubDirectories] -- Xanatos -->
	if (strDir.Right(1) == _T("\\")){
		strDir = strDir.Left(strDir.GetLength()-1);
	}
	RemoveSharedDirectory(strDir, bSubDirectories);	// For list switching, and child unsharing
	if (FileSystemTreeIsShared(strDir, true, !bSubDirectories))	// Check indirect, to avoid double-sharing
		return;

	if (bSubDirectories)	// Pick a list, way better than going over the tree
		m_strliSharedDirsSubdir.AddTail(strDir);
	else
		m_strliSharedDirs.AddTail(strDir);
	// NEO: SSD END <-- Xanatos --
	/*if (!FileSystemTreeIsShared(strDir)){
		m_strliSharedDirs.AddTail(strDir);
	}
	if (bSubDirectories){
		if (strDir.Right(1) != _T("\\"))
			strDir += _T("\\");
		CFileFind finder;
		BOOL bWorking = finder.FindFile(strDir+_T("*.*"));
		while (bWorking)
		{
			bWorking = finder.FindNextFile();
			if (finder.IsDots() || finder.IsSystem() || !finder.IsDirectory())
				continue;
			AddSharedDirectory(strDir + finder.GetFileName(), true);
		}
		finder.Close();
	}*/
}

void CSharedDirsTreeCtrl::RemoveSharedDirectory(CString strDir, bool bSubDirectories){
	if (strDir.Right(1) == _T("\\")){
		strDir = strDir.Left(strDir.GetLength()-1);
	}
	strDir.MakeLower();
	POSITION pos1, pos2;
	// NEO: SSD - [ShareSubDirectories] -- Xanatos -->
	if(bSubDirectories){
		for (pos1 = m_strliSharedDirsSubdir.GetHeadPosition();( pos2 = pos1 ) != NULL;)
		{
			m_strliSharedDirsSubdir.GetNext(pos1);
			CString str = m_strliSharedDirsSubdir.GetAt(pos2);
			str.MakeLower();
			if (str.CompareNoCase(strDir) == 0)
				m_strliSharedDirsSubdir.RemoveAt(pos2);
			else if (bSubDirectories && str.Find(strDir) == 0)
				m_strliSharedDirsSubdir.RemoveAt(pos2);
		}

	}else
	// NEO: SSD END <-- Xanatos --
	{
		for (pos1 = m_strliSharedDirs.GetHeadPosition();( pos2 = pos1 ) != NULL;)
		{
			m_strliSharedDirs.GetNext(pos1);
			CString str = m_strliSharedDirs.GetAt(pos2);
			str.MakeLower();
			if (str.CompareNoCase(strDir) == 0)
				m_strliSharedDirs.RemoveAt(pos2);
			else if (bSubDirectories && str.Find(strDir) == 0)
				m_strliSharedDirs.RemoveAt(pos2);
		}
	}
}

void CSharedDirsTreeCtrl::FileSystemTreeUpdateBoldState(const CDirectoryItem* pDir){
	if (pDir == NULL)
		pDir = m_pRootUnsharedDirectries;
	SetItemState(pDir->m_htItem, (FileSystemTreeHasSharedSubdirectory(pDir->m_strFullPath) ? TVIS_BOLD : 0), TVIS_BOLD);
	POSITION pos = pDir->liSubDirectories.GetHeadPosition();
	while (pos != NULL){
		FileSystemTreeUpdateBoldState(pDir->liSubDirectories.GetNext(pos));
	}
}

void CSharedDirsTreeCtrl::FileSystemTreeUpdateShareState(const CDirectoryItem* pDir){
	if (pDir == NULL)
		pDir = m_pRootUnsharedDirectries;
	SetItemState(pDir->m_htItem, FileSystemTreeIsShared(pDir->m_strFullPath) ? INDEXTOOVERLAYMASK(1) : 0, TVIS_OVERLAYMASK);
	POSITION pos = pDir->liSubDirectories.GetHeadPosition();
	while (pos != NULL){
		FileSystemTreeUpdateShareState(pDir->liSubDirectories.GetNext(pos));
	}
}

void CSharedDirsTreeCtrl::FileSystemTreeSetShareState(const CDirectoryItem* pDir, bool bShared, bool bSubDirectories){
	if (m_bUseIcons && pDir->m_htItem != NULL)
		SetItemState(pDir->m_htItem, bShared ? INDEXTOOVERLAYMASK(1) : 0, TVIS_OVERLAYMASK);
	if (bSubDirectories){
		POSITION pos = pDir->liSubDirectories.GetHeadPosition();
		while (pos != NULL){
			FileSystemTreeSetShareState(pDir->liSubDirectories.GetNext(pos), bShared, true);
		}
	}
}

void CSharedDirsTreeCtrl::EditSharedDirectories(const CDirectoryItem* pDir, bool bAdd, bool bSubDirectories){
	ASSERT( pDir->m_eItemType == SDI_UNSHAREDDIRECTORY || pDir->m_eItemType == SDI_NO );

	CWaitCursor curWait;
	if (bAdd)
		AddSharedDirectory(pDir->m_strFullPath, bSubDirectories);
	else
		RemoveSharedDirectory(pDir->m_strFullPath, bSubDirectories);

	if (pDir->m_eItemType == SDI_NO) {
		// An 'Unshare' was invoked from within the virtual "Shared Directories" folder, thus we do not have
		// the tree view item handle of the item within the "All Directories" tree -> need to update the
		// entire tree in case the tree view item is currently visible.
		FileSystemTreeUpdateShareState();
	}
	else {
		// A 'Share' or 'Unshare' was invoked for a certain tree view item within the "All Directories" tree,
		// thus we know the tree view item handle which needs to be updated for showing the new share state.
		FileSystemTreeSetShareState(pDir, bAdd, bSubDirectories);
	}
	FileSystemTreeUpdateBoldState();
	FilterTreeReloadTree();

	// sync with the preferences list
	thePrefs.shareddir_list.RemoveAll();
	POSITION pos = m_strliSharedDirs.GetHeadPosition();
	// copy list
	while (pos){
		CString strPath = m_strliSharedDirs.GetNext(pos);
		if (strPath.Right(1) != _T("\\")){
			strPath += _T("\\");
		}
		thePrefs.shareddir_list.AddTail(strPath);
	}
	// NEO: SSD - [ShareSubDirectories] -- Xanatos -->
	thePrefs.sharedsubdir_list.RemoveAll();
	pos = m_strliSharedDirsSubdir.GetHeadPosition();
	// copy list
	while (pos){
		CString strPath = m_strliSharedDirsSubdir.GetNext(pos);
		if (strPath.Right(1) != _T("\\")){
			strPath += _T("\\");
		}
		thePrefs.sharedsubdir_list.AddTail(strPath);
	}
	// NEO: SSD END <-- Xanatos --

	//  update the sharedfiles list
	theApp.emuledlg->sharedfileswnd->Reload();
	thePrefs.Save();
}

void CSharedDirsTreeCtrl::Reload(bool bForce){
	bool bChanged = false;
	if (!bForce){
		// check for changes in shared dirs
		if (thePrefs.shareddir_list.GetCount() == m_strliSharedDirs.GetCount()){
			POSITION pos = m_strliSharedDirs.GetHeadPosition();
			POSITION pos2 = thePrefs.shareddir_list.GetHeadPosition();
			while (pos != NULL && pos2 != NULL){
				CString str1 = m_strliSharedDirs.GetNext(pos);
				CString str2 = thePrefs.shareddir_list.GetNext(pos2);
				if (str1.Right(1) == _T("\\")){
					str1 = str1.Left(str1.GetLength()-1);
				}
				if (str2.Right(1) == _T("\\")){
					str2 = str2.Left(str2.GetLength()-1);
				}
				if  (str1.CompareNoCase(str2) != 0){
					bChanged = true;
					break;
				}
			}
		}
		else
			bChanged = true;

		// NEO: SSD - [ShareSubDirectories] -- Xanatos -->
		if (thePrefs.sharedsubdir_list.GetCount() == m_strliSharedDirsSubdir.GetCount()){
			POSITION pos = m_strliSharedDirsSubdir.GetHeadPosition();
			POSITION pos2 = thePrefs.sharedsubdir_list.GetHeadPosition();
			while (pos != NULL && pos2 != NULL){
				CString str1 = m_strliSharedDirsSubdir.GetNext(pos);
				CString str2 = thePrefs.sharedsubdir_list.GetNext(pos2);
				if (str1.Right(1) == _T("\\")){
					str1 = str1.Left(str1.GetLength()-1);
				}
				if (str2.Right(1) == _T("\\")){
					str2 = str2.Left(str2.GetLength()-1);
				}
				if  (str1.CompareNoCase(str2) != 0){
					bChanged = true;
					break;
				}
			}
		}
		else
			bChanged = true;
		// NEO: SSD END <-- Xanatos --

		// check for changes in categories incoming dirs
		/*CString strMainIncDir = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
		if (strMainIncDir.Right(1) == _T("\\"))
			strMainIncDir = strMainIncDir.Left(strMainIncDir.GetLength()-1);*/
		CStringList strliFound;
		//for (int i = 0; i < thePrefs.GetCatCount(); i++){
		for (int i = 0; i < thePrefs.GetFullCatCount(); i++){ // NEO: NSC - [NeoSharedCategories] <-- Xanatos --
			Category_Struct* pCatStruct = thePrefs.GetCategory(i);
			if (pCatStruct != NULL){
				//CString strCatIncomingPath = pCatStruct->strIncomingPath;
				CString strCatIncomingPath = thePrefs.GetCatPath(i); // NEO: NXC - [NewExtendedCategories] -- Xanatos --
				if (strCatIncomingPath.Right(1) == _T("\\"))
					strCatIncomingPath = strCatIncomingPath.Left(strCatIncomingPath.GetLength()-1);

				if (!strCatIncomingPath.IsEmpty() /*&& strCatIncomingPath.CompareNoCase(strMainIncDir) != 0*/  // NEO: MOD <-- [SDI_INCOMING] --
					&& strliFound.Find(strCatIncomingPath) == NULL)
				{
					POSITION pos = m_strliCatIncomingDirs.Find(strCatIncomingPath);
					if (pos != NULL){
						strliFound.AddTail(strCatIncomingPath);
					}
					else{
						bChanged = true;
						break;
					}
				}
			}
		}
		if (strliFound.GetCount() != m_strliCatIncomingDirs.GetCount())
			bChanged = true;

		// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
		CStringList strliFoundTemp;
		/*CString strMainTempDir = thePrefs.GetTempDir(0);
		if (strMainTempDir.Right(1) == "\\")
			strMainTempDir = strMainTempDir.Left(strMainTempDir.GetLength()-1);
		*/
		for (int i = 0; i < thePrefs.GetTempDirCount(); i++){
			CString strTempPath = thePrefs.GetTempDir(i);
			if (strTempPath.Right(1) == _T("\\")){
				strTempPath = strTempPath.Left(strTempPath.GetLength()-1);
			}
			if (!strTempPath.IsEmpty() /*&& strTempPath.CompareNoCase(strMainTempDir) != 0*/
				&& strliFoundTemp.Find(strTempPath) == NULL)
			{
				POSITION pos = m_strliTempDirs.Find(strTempPath);
				if (pos != NULL){
					strliFoundTemp.AddTail(strTempPath);
				}
				else{
					bChanged = true;
					break;
				}
			}
		}
		if (strliFoundTemp.GetCount() != m_strliTempDirs.GetCount())
			bChanged = true;
		// NEO: MTD END <-- Xanatos --

		// NEO: NSC - [NeoSharedCategories] -- Xanatos -->
		if (m_iCategories != thePrefs.GetFullCatCount())
			bChanged = true;
		// NEO: NSC END <-- Xanatos --

	}
	if (bChanged || bForce){
		FetchSharedDirsList();
		FilterTreeReloadTree();
		Expand(m_pRootUnsharedDirectries->m_htItem, TVE_COLLAPSE); // collapsing is enough to sync for the filtetree, as all items are recreated on every expanding
	}
}

void CSharedDirsTreeCtrl::FetchSharedDirsList(){
	m_strliSharedDirs.RemoveAll();
	POSITION pos = thePrefs.shareddir_list.GetHeadPosition();
	// copy list
	while (pos){
		CString strPath = thePrefs.shareddir_list.GetNext(pos);
		if (strPath.Right(1) == _T("\\")){
			strPath = strPath.Left(strPath.GetLength()-1);
		}
		m_strliSharedDirs.AddTail(strPath);
	}

	// NEO: SSD - [ShareSubDirectories] -- Xanatos -->
	m_strliSharedDirsSubdir.RemoveAll();
	POSITION pos2 = thePrefs.sharedsubdir_list.GetHeadPosition();
	// copy list
	while (pos2){
		CString strPath = thePrefs.sharedsubdir_list.GetNext(pos2);
		if (strPath.Right(1) == _T("\\")){
			strPath = strPath.Left(strPath.GetLength()-1);
		}
		m_strliSharedDirsSubdir.AddTail(strPath);
	}
	// NEO: SSD END <-- Xanatos --
}

void CSharedDirsTreeCtrl::OnLvnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW)pNMHDR;
	*pResult = 0;

	CDirectoryItem* pToDrag = (CDirectoryItem*)lpnmtv->itemNew.lParam;
	if (pToDrag == NULL || pToDrag->m_eItemType != SDI_UNSHAREDDIRECTORY || FileSystemTreeIsShared(pToDrag->m_strFullPath))
		return;

	ASSERT( m_pDraggingItem == NULL );
	delete m_pDraggingItem;
	m_pDraggingItem = pToDrag->CloneContent(); // to be safe we store a copy, as items can be deleted when collapsing the tree etc

	CImageList* piml = NULL;
	POINT ptOffset;
	RECT rcItem;
	if ((piml = CreateDragImage(lpnmtv->itemNew.hItem)) == NULL)
		return;

	/* get the bounding rectangle of the item being dragged (rel to top-left of control) */
	if (GetItemRect(lpnmtv->itemNew.hItem, &rcItem, TRUE))
	{
		CPoint ptDragBegin;
		int nX, nY;
		/* get offset into image that the mouse is at */
		/* item rect doesn't include the image */
		ptDragBegin = lpnmtv->ptDrag;
		ImageList_GetIconSize(piml->GetSafeHandle(), &nX, &nY);
		ptOffset.x = (ptDragBegin.x - rcItem.left) + (nX - (rcItem.right - rcItem.left));
		ptOffset.y = (ptDragBegin.y - rcItem.top) + (nY - (rcItem.bottom - rcItem.top));
		/* convert the item rect to screen co-ords, for use later */
		MapWindowPoints(NULL, &rcItem);
	}
	else
	{
		GetWindowRect(&rcItem);
		ptOffset.x = ptOffset.y = 8;
	}

	if (piml->BeginDrag(0, ptOffset))
	{
		CPoint ptDragEnter = lpnmtv->ptDrag;
		ClientToScreen(&ptDragEnter);
		piml->DragEnter(NULL, ptDragEnter);
	}
	delete piml;

	/* set the focus here, so we get a WM_CANCELMODE if needed */
	SetFocus();

	/* redraw item being dragged, otherwise it remains (looking) selected */
	InvalidateRect(&rcItem, TRUE);
	UpdateWindow();

	/* Hide the mouse cursor, and direct mouse input to this window */
	SetCapture(); 
}

void CSharedDirsTreeCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_pDraggingItem != NULL)
	{
		CPoint pt;

		/* drag the item to the current position */
		pt = point;
		ClientToScreen(&pt);

		CImageList::DragMove(pt);
		CImageList::DragShowNolock(FALSE);
		if (CWnd::WindowFromPoint(pt) != this)
			SetCursor(AfxGetApp()->LoadStandardCursor(IDC_NO));
		else
		{
			TVHITTESTINFO tvhti;
			tvhti.pt = pt;
			ScreenToClient(&tvhti.pt);
			HTREEITEM hItemSel = HitTest(&tvhti);
			CDirectoryItem* pDragTarget;
			if (hItemSel != NULL && (pDragTarget = (CDirectoryItem*)GetItemData(hItemSel)) != NULL){
				//only allow dragging to shared folders
				if (pDragTarget->m_eItemType == SDI_DIRECTORY || pDragTarget->m_eItemType == SDI_NO){
					SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
					SelectDropTarget(pDragTarget->m_htItem);
				}
				else
					SetCursor(AfxGetApp()->LoadStandardCursor(IDC_NO));
			}
			else{
				SetCursor(AfxGetApp()->LoadStandardCursor(IDC_NO));
			}
		}

		CImageList::DragShowNolock(TRUE);
	}

	CTreeCtrl::OnMouseMove(nFlags, point);
}

void CSharedDirsTreeCtrl::OnLButtonUp(UINT nFlags, CPoint point){
	
	if (m_pDraggingItem != NULL){
		CPoint pt;
		pt = point;
		ClientToScreen(&pt);

		TVHITTESTINFO tvhti;
		tvhti.pt = pt;
		ScreenToClient(&tvhti.pt);
		HTREEITEM hItemSel = HitTest(&tvhti);
		CDirectoryItem* pDragTarget;
		if (hItemSel != NULL && (pDragTarget = (CDirectoryItem*)GetItemData(hItemSel)) != NULL){
			//only allow dragging to shared folders
			if (pDragTarget->m_eItemType == SDI_DIRECTORY || pDragTarget->m_eItemType == SDI_NO){
				CDirectoryItem* pRealDragItem;
				HTREEITEM htReal = m_pRootUnsharedDirectries->FindItem(m_pDraggingItem);
				// get the original drag src
				if (htReal != NULL && (pRealDragItem = (CDirectoryItem*)GetItemData(htReal)) != NULL){
					EditSharedDirectories(pRealDragItem, true, false);
				}
				else{
					// item was deleted - no problem as when we dont need to update the visible part
					// we can just as well use the contentcopy
					EditSharedDirectories(m_pDraggingItem, true, false);
				}
			}
		}
		
		CImageList::DragLeave(NULL);
		CImageList::EndDrag();
		ReleaseCapture();
		ShowCursor(TRUE);
		SelectDropTarget(NULL);

		delete m_pDraggingItem;
		m_pDraggingItem = NULL;

		RedrawWindow();
	}
	CTreeCtrl::OnLButtonUp(nFlags, point);
}

void CSharedDirsTreeCtrl::OnCancelMode() 
{
	if (m_pDraggingItem != NULL){
		CImageList::DragLeave(NULL);
		CImageList::EndDrag();
		ReleaseCapture();
		ShowCursor(TRUE);
		SelectDropTarget(NULL);

		delete m_pDraggingItem;
		m_pDraggingItem = NULL;
		RedrawWindow();
	}
	CTreeCtrl::OnCancelMode();
}

// NEO: SSD - [ShareSubDirectories] -- Xanatos -->
LRESULT CSharedDirsTreeCtrl::OnFoundNetworkDrive(WPARAM, LPARAM){
	NetworkDrivesLock.Lock();
	while (!NetworkDrives.IsEmpty())
		FileSystemTreeAddChildItem(m_pRootUnsharedDirectries, NetworkDrives.RemoveHead(), true); // e.g. ("c:")
	NetworkDrivesLock.Unlock();

	return 0;
}

void CSharedDirsTreeCtrl::EnumNetworkDrives(NETRESOURCE *source)
{
	char buffer[sizeof(NETRESOURCE)+4*MAX_PATH];	// NETRESOURCE structure+4 strings
	NETRESOURCE &folder = (NETRESOURCE&)buffer;
	HANDLE hEnum;
	if (WNetOpenEnum(RESOURCE_GLOBALNET, RESOURCETYPE_DISK, 0, source, &hEnum) != NO_ERROR)
		return;
	while (NetworkThreadRun) {
		ZeroMemory(buffer, sizeof(buffer));
		DWORD count = 1;	// Must read one at a time for recursive reading(see below)
		DWORD size = sizeof(buffer);
		if (WNetEnumResource(hEnum, &count, &folder, &size) != NO_ERROR)
			break;
		ASSERT(count <= 1);	// That's all we asked for
		if (count > 0) {
			if ((folder.dwUsage & RESOURCEUSAGE_CONTAINER)==RESOURCEUSAGE_CONTAINER)
				EnumNetworkDrives(&folder);	// Warning: When returning from this, folder becomes invalid, and mustn't be reused
			else if (folder.lpRemoteName && _tcslen(folder.lpRemoteName) > 0) {
				NetworkDrivesLock.Lock();
				NetworkDrives.AddTail(folder.lpRemoteName);	// Add to list
				NetworkDrivesLock.Unlock();
			}
		}
		else {
			ASSERT(0);	// Should have returned an error earlier
			break;
		}
	}
	WNetCloseEnum(hEnum);
	if (NetworkThreadRun)	// Not while we're quitting(window is probably destroyed anyway)
		PostMessage(TM_FOUNDNETWORKDRIVE, 0, 0);	// Notify update
}

UINT CSharedDirsTreeCtrl::EnumNetworkDrivesThreadProc(LPVOID pParam){
	// SLUGFILLER: SafeHash // NEO: STS - [SlugFillerThreadSafe]
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return 0;
	// SLUGFILLER: SafeHash // NEO: STS END

	((CSharedDirsTreeCtrl*)pParam)->EnumNetworkDrives(NULL);
	((CSharedDirsTreeCtrl*)pParam)->NetworkThreadOffline->SetEvent();
	return 0;
}
// NEO: SSD END <-- Xanatos --

// NEO: NMX - [NeoMenuXP] -- Xanatos -->
void CSharedDirsTreeCtrl::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	HMENU hMenu = AfxGetThreadState()->m_hTrackingMenu;
	if(CMenu *pMenu = CMenu::FromHandle(hMenu))
		pMenu->MeasureItem(lpMeasureItemStruct);
	
	CTreeCtrl::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}
// NEO: NMX END <-- Xanatos --
